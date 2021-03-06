/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// CLASS HEADER
#include <dali/internal/event/effects/shader-effect-impl.h>

// INTERNAL INCLUDES
#include <dali/public-api/math/matrix.h>
#include <dali/public-api/math/matrix3.h>
#include <dali/public-api/math/vector2.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/devel-api/scripting/scripting.h>
#include <dali/public-api/shader-effects/shader-effect.h>
#include <dali/internal/event/common/property-helper.h>
#include <dali/internal/event/common/stage-impl.h>
#include <dali/internal/event/common/thread-local-storage.h>
#include <dali/internal/event/effects/shader-declarations.h>
#include <dali/internal/event/effects/shader-factory.h>
#include <dali/internal/event/images/image-impl.h>
#include <dali/internal/render/shaders/scene-graph-shader.h>
#include <dali/internal/render/shaders/uniform-meta.h>
#include <dali/internal/update/animation/scene-graph-constraint-base.h>
#include <dali/internal/update/common/animatable-property.h>
#include <dali/internal/update/manager/update-manager.h>
#include "dali-shaders.h"

using Dali::Internal::SceneGraph::UpdateManager;
using Dali::Internal::SceneGraph::UniformMeta;
using Dali::Internal::SceneGraph::Shader;
using Dali::Internal::SceneGraph::AnimatableProperty;
using Dali::Internal::SceneGraph::PropertyBase;
using Dali::Internal::SceneGraph::RenderQueue;
using std::string;

namespace Dali
{

namespace Internal
{

namespace
{

// Properties

//              Name             Type   writable animatable constraint-input  enum for index-checking
DALI_PROPERTY_TABLE_BEGIN
DALI_PROPERTY( "grid-density",   FLOAT,   true,    false,   false,   Dali::ShaderEffect::Property::GRID_DENSITY   )
DALI_PROPERTY( "image",          MAP,     true,    false,   false,   Dali::ShaderEffect::Property::IMAGE          )
DALI_PROPERTY( "program",        MAP,     true,    false,   false,   Dali::ShaderEffect::Property::PROGRAM        )
DALI_PROPERTY( "geometry-hints", STRING,  true,    false,   false,   Dali::ShaderEffect::Property::GEOMETRY_HINTS )
DALI_PROPERTY_TABLE_END( DEFAULT_ACTOR_PROPERTY_START_INDEX )

BaseHandle Create()
{
  Internal::ShaderEffectPtr internal = Internal::ShaderEffect::New();

  return Dali::ShaderEffect(internal.Get());
}

TypeRegistration mType( typeid(Dali::ShaderEffect), typeid(Dali::Handle), Create );

struct WrapperStrings
{
  const char* vertexShaderPrefix;
  const char* fragmentShaderPrefix;
  const char* vertexShaderPostfix;
  const char* fragmentShaderPostfix;
};

WrapperStrings customImageShaderWrappers =
{
  CustomImagePrefixVertex, CustomImagePrefixFragment,
  CustomImagePostfixVertex, CustomImagePostfixFragment
};

/**
 * Helper to wrap the program with our default pre and postfix if needed and then send it to update/render thread
 * @param[in] effect of the shader
 * @param[in] vertexPrefix from application
 * @param[in] fragmentPrefix from application
 * @param[in] vertexBody from application
 * @param[in] fragmentBody from application
 * @param[in] modifiesGeometry based on flags and vertex shader
 */
void WrapAndSetProgram( Internal::ShaderEffect& effect,
                        const std::string& vertexPrefix, const std::string& fragmentPrefix,
                        const std::string& vertexBody, const std::string& fragmentBody,
                        bool modifiesGeometry )
{
  // if there is some real data in the strings
  if( ( vertexPrefix.length()   > 0 ) ||
      ( fragmentPrefix.length() > 0 ) ||
      ( vertexBody.length()     > 0 ) ||
      ( fragmentBody.length()   > 0 ) )
  {
    std::string vertexSource = vertexPrefix;
    std::string fragmentSource = fragmentPrefix;

    // Create complete shader program strings:

    vertexSource += customImageShaderWrappers.vertexShaderPrefix;

    // Append the custom vertex shader code if supplied, otherwise append the default
    if ( vertexBody.length() > 0 )
    {
      vertexSource.append( vertexBody );
    }
    else
    {
      vertexSource.append( customImageShaderWrappers.vertexShaderPostfix );
    }

    fragmentSource += customImageShaderWrappers.fragmentShaderPrefix;

    // Append the custom fragment shader code if supplied, otherwise append the default
    if ( fragmentBody.length() > 0 )
    {
      fragmentSource.append( fragmentBody );
    }
    else
    {
      fragmentSource.append( customImageShaderWrappers.fragmentShaderPostfix );
    }

    effect.SendProgramMessage( vertexSource, fragmentSource, modifiesGeometry );
  }
}

std::string GetShader(const std::string& field, const Property::Value& property)
{
  std::string retval;
  const Property::Map* map = property.GetMap();
  if( map )
  {
    const Property::Value* value = map->Find( field );
    if( value )
    {
      value->Get( retval );
    }
  }

  return retval;
}

} // unnamed namespace

ShaderEffectPtr ShaderEffect::New( Dali::ShaderEffect::GeometryHints hints )
{
  Stage* stage = Stage::GetCurrent();

  if( stage )
  {
    ShaderEffectPtr shaderEffect( new ShaderEffect( *stage, hints ) );
    shaderEffect->RegisterObject();
    return shaderEffect;
  }
  else
  {
    return NULL;
  }
}

ShaderEffect::ShaderEffect( EventThreadServices& eventThreadServices, Dali::ShaderEffect::GeometryHints hints )
: mEventThreadServices( eventThreadServices ),
  mConnectionCount (0),
  mGeometryHints( hints )
{
  mSceneObject = new SceneGraph::Shader( hints );
  DALI_ASSERT_DEBUG( NULL != mSceneObject );

  // Transfer shader ownership to a scene message
  AddShaderMessage( eventThreadServices.GetUpdateManager(), *mSceneObject );
}

ShaderEffect::~ShaderEffect()
{
  // Guard to allow handle destruction after Core has been destroyed
  if ( Stage::IsInstalled() )
  {
    // Remove scene-object using a message to the UpdateManager
    if( mSceneObject )
    {
      RemoveShaderMessage( mEventThreadServices.GetUpdateManager(), *mSceneObject );
    }
    UnregisterObject();
  }
}

void ShaderEffect::SetEffectImage( Dali::Image image )
{
  // if images are the same, do nothing
  if (mImage == image)
  {
    return;
  }

  if (mImage && mConnectionCount > 0)
  {
    // unset previous image
    GetImplementation(mImage).Disconnect();
  }

  // in case image is empty this will reset our image handle
  mImage = image;

  if (!image)
  {
    // mSceneShader can be in a separate thread; queue a setter message
    SetTextureIdMessage( mEventThreadServices, *mSceneObject, 0 );
  }
  else
  {
    // tell image that we're using it
    if (mConnectionCount > 0)
    {
      GetImplementation(mImage).Connect();
    }
    // mSceneShader can be in a separate thread; queue a setter message
    SetTextureIdMessage( mEventThreadServices, *mSceneObject, GetImplementation(mImage).GetResourceId() );
  }
}

void ShaderEffect::SetUniform( const std::string& name, Property::Value value, UniformCoordinateType uniformCoordinateType )
{
  // Register the property if it does not exist
  Property::Index index = GetPropertyIndex( name );
  if ( Property::INVALID_INDEX == index )
  {
    index = RegisterProperty( name, value );
  }

  SetProperty( index, value );

  // RegisterProperty guarantees a positive value as index
  DALI_ASSERT_DEBUG( static_cast<unsigned int>(index) >= CustomPropertyStartIndex() );
  unsigned int metaIndex = index - CustomPropertyStartIndex();
  // check if there's space in cache
  if( mCoordinateTypes.Count() < (metaIndex + 1) )
  {
    mCoordinateTypes.Resize( metaIndex + 1 );
  }
  // only send message if the value is different than current, initial value is COORDINATE_TYPE_DEFAULT (0)
  if( uniformCoordinateType != mCoordinateTypes[ metaIndex ] )
  {
    mCoordinateTypes[ metaIndex ] = uniformCoordinateType;
    SetCoordinateTypeMessage( mEventThreadServices, *mSceneObject, metaIndex, uniformCoordinateType );
  }
}

void ShaderEffect::SetPrograms( const string& vertexSource, const string& fragmentSource )
{
  SetPrograms( "", "", vertexSource, fragmentSource );
}

void ShaderEffect::SetPrograms( const std::string& vertexPrefix, const std::string& fragmentPrefix,
                                const std::string& vertexSource, const std::string& fragmentSource )
{
  bool modifiesGeometry = true;
  // check if the vertex shader is empty (means it cannot modify geometry)
  if( (vertexPrefix.length() == 0) && (vertexSource.length() == 0) )
  {
    modifiesGeometry = false;
  }
  // check the hint second
  if( ( mGeometryHints & Dali::ShaderEffect::HINT_DOESNT_MODIFY_GEOMETRY ) != 0 )
  {
    modifiesGeometry = false;
  }

  WrapAndSetProgram( *this, vertexPrefix, fragmentPrefix, vertexSource, fragmentSource, modifiesGeometry );
}

void ShaderEffect::SendProgramMessage( const string& vertexSource, const string& fragmentSource,
                                       bool modifiesGeometry )
{
  ThreadLocalStorage& tls = ThreadLocalStorage::Get();
  ShaderFactory& shaderFactory = tls.GetShaderFactory();
  size_t shaderHash;

  Internal::ShaderDataPtr shaderData = shaderFactory.Load( vertexSource, fragmentSource, shaderHash );
  DALI_ASSERT_DEBUG( shaderHash != 0U );

  // Add shader program to scene-object using a message to the UpdateManager
  SetShaderProgramMessage( mEventThreadServices.GetUpdateManager(), *mSceneObject, shaderData, modifiesGeometry );
}

void ShaderEffect::Connect()
{
  ++mConnectionCount;

  if (mImage && mConnectionCount == 1)
  {
    GetImplementation(mImage).Connect();

    // Image may have changed resource due to load/release policy. Ensure correct texture ID is set on scene graph object
    SetTextureIdMessage( mEventThreadServices, *mSceneObject, GetImplementation(mImage).GetResourceId() );
  }
}

void ShaderEffect::Disconnect()
{
  DALI_ASSERT_DEBUG(mConnectionCount > 0);
  --mConnectionCount;

  if (mImage && mConnectionCount == 0)
  {
     GetImplementation(mImage).Disconnect();
  }
}

unsigned int ShaderEffect::GetDefaultPropertyCount() const
{
  return DEFAULT_PROPERTY_COUNT;
}

void ShaderEffect::GetDefaultPropertyIndices( Property::IndexContainer& indices ) const
{
  indices.Reserve( DEFAULT_PROPERTY_COUNT );

  for ( int i = 0; i < DEFAULT_PROPERTY_COUNT; ++i )
  {
    indices.PushBack( i );
  }
}

const char* ShaderEffect::GetDefaultPropertyName(Property::Index index) const
{
  if( index < DEFAULT_PROPERTY_COUNT )
  {
    return DEFAULT_PROPERTY_DETAILS[index].name;
  }

  return NULL;
}

Property::Index ShaderEffect::GetDefaultPropertyIndex(const std::string& name) const
{
  Property::Index index = Property::INVALID_INDEX;

  // Look for name in default properties
  for( int i = 0; i < DEFAULT_PROPERTY_COUNT; ++i )
  {
    const Internal::PropertyDetails* property = &DEFAULT_PROPERTY_DETAILS[ i ];
    if( 0 == strcmp( name.c_str(), property->name ) ) // dont want to convert rhs to string
    {
      index = i;
      break;
    }
  }

  return index;
}

bool ShaderEffect::IsDefaultPropertyWritable(Property::Index index) const
{
  return DEFAULT_PROPERTY_DETAILS[ index ].writable;
}

bool ShaderEffect::IsDefaultPropertyAnimatable(Property::Index index) const
{
  return DEFAULT_PROPERTY_DETAILS[ index ].animatable;
}

bool ShaderEffect::IsDefaultPropertyAConstraintInput( Property::Index index ) const
{
  return DEFAULT_PROPERTY_DETAILS[ index ].constraintInput;
}

Property::Type ShaderEffect::GetDefaultPropertyType(Property::Index index) const
{
  if( index < DEFAULT_PROPERTY_COUNT )
  {
    return DEFAULT_PROPERTY_DETAILS[index].type;
  }

  // index out of range...return Property::NONE
  return Property::NONE;
}

void ShaderEffect::SetDefaultProperty( Property::Index index, const Property::Value& propertyValue )
{
  switch ( index )
  {
    case Dali::ShaderEffect::Property::GRID_DENSITY:
    {
      SetGridDensityMessage( mEventThreadServices, *mSceneObject, propertyValue.Get<float>() );
      break;
    }

    case Dali::ShaderEffect::Property::IMAGE:
    {
      Dali::Image img(Scripting::NewImage( propertyValue ));
      if(img)
      {
        SetEffectImage( img );
      }
      else
      {
        DALI_LOG_WARNING("Cannot create image from property value for ShaderEffect image\n");
      }
      break;
    }

    case Dali::ShaderEffect::Property::PROGRAM:
    {
      std::string vertexPrefix   = GetShader("vertex-prefix", propertyValue);
      std::string fragmentPrefix = GetShader("fragment-prefix", propertyValue);
      std::string vertex         = GetShader("vertex", propertyValue);
      std::string fragment       = GetShader("fragment", propertyValue);

      SetPrograms( vertexPrefix, fragmentPrefix, vertex, fragment );
      break;
    }

    case Dali::ShaderEffect::Property::GEOMETRY_HINTS:
    {
      Dali::ShaderEffect::GeometryHints hint = Dali::ShaderEffect::HINT_NONE;
      std::string s = propertyValue.Get<std::string>();
      if(s == "HINT_NONE")
      {
        hint = Dali::ShaderEffect::HINT_NONE;
      }
      else if(s == "HINT_GRID_X")
      {
        hint = Dali::ShaderEffect::HINT_GRID_X;
      }
      else if(s == "HINT_GRID_Y")
      {
        hint = Dali::ShaderEffect::HINT_GRID_Y;
      }
      else if(s == "HINT_GRID")
      {
        hint = Dali::ShaderEffect::HINT_GRID;
      }
      else if(s == "HINT_DEPTH_BUFFER")
      {
        hint = Dali::ShaderEffect::HINT_DEPTH_BUFFER;
      }
      else if(s == "HINT_BLENDING")
      {
        hint = Dali::ShaderEffect::HINT_BLENDING;
      }
      else if(s == "HINT_DOESNT_MODIFY_GEOMETRY")
      {
        hint = Dali::ShaderEffect::HINT_DOESNT_MODIFY_GEOMETRY;
      }
      else
      {
        DALI_ASSERT_ALWAYS(!"Geometry hint unknown" );
      }

      SetHintsMessage( mEventThreadServices, *mSceneObject, hint );

      break;
    }

    default:
    {
      // nothing to do
      break;
    }
  }
}

Property::Value ShaderEffect::GetDefaultProperty(Property::Index /*index*/) const
{
  // none of our properties are readable so return empty
  return Property::Value();
}

void ShaderEffect::NotifyScenePropertyInstalled( const SceneGraph::PropertyBase& newProperty, const std::string& name, unsigned int index ) const
{
  // Warning - the property is added to the Shader object in the Update thread and the meta-data is added in the Render thread (through a secondary message)

  // mSceneObject requires metadata for each custom property (uniform)
  UniformMeta* meta = UniformMeta::New( name, newProperty, Dali::ShaderEffect::COORDINATE_TYPE_DEFAULT );
  // mSceneObject is being used in a separate thread; queue a message to add the property
  InstallUniformMetaMessage( mEventThreadServices, *mSceneObject, *meta ); // Message takes ownership
}

const SceneGraph::PropertyOwner* ShaderEffect::GetSceneObject() const
{
  return mSceneObject;
}

const PropertyBase* ShaderEffect::GetSceneObjectAnimatableProperty( Property::Index index ) const
{
  PropertyMetadata* property = index >= PROPERTY_CUSTOM_START_INDEX ? static_cast<PropertyMetadata*>(FindCustomProperty( index )) : static_cast<PropertyMetadata*>(FindAnimatableProperty( index ));
  DALI_ASSERT_ALWAYS( property && "Property index is invalid" );
  return property->GetSceneGraphProperty();
}

const PropertyInputImpl* ShaderEffect::GetSceneObjectInputProperty( Property::Index index ) const
{
  return GetSceneObjectAnimatableProperty( index );
}

} // namespace Internal

} // namespace Dali
