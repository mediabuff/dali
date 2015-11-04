// Copyright (c) 2014 Samsung Electronics Co., Ltd.

// CLASS HEADER
#include "image-view-impl.h"

// EXTERNAL INCLUDES
#include <dali/public-api/images/resource-image.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/devel-api/object/type-registry-helper.h>
#include <dali/devel-api/scripting/scripting.h>

// INTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/image-view/image-view.h>
#include <dali-toolkit/devel-api/controls/renderer-factory/renderer-factory.h>
#include <dali-toolkit/internal/controls/renderers/image/image-renderer.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal
{

namespace
{

BaseHandle Create()
{
  return Toolkit::ImageView::New();
}

// Setup properties, signals and actions using the type-registry.
DALI_TYPE_REGISTRATION_BEGIN( Toolkit::ImageView, Toolkit::Control, Create );
DALI_PROPERTY_REGISTRATION( Toolkit, ImageView, "image", MAP, IMAGE )
DALI_TYPE_REGISTRATION_END()

} // anonymous namespace

using namespace Dali;

ImageView::ImageView()
: Control( ControlBehaviour( ACTOR_BEHAVIOUR_NONE ) )
{
}

ImageView::~ImageView()
{
}

Toolkit::ImageView ImageView::New()
{
  ImageView* impl = new ImageView();

  Toolkit::ImageView handle = Toolkit::ImageView( *impl );

  // Second-phase init of the implementation
  // This can only be done after the CustomActor connection has been made...
  impl->Initialize();

  return handle;
}

/////////////////////////////////////////////////////////////

void ImageView::SetImage( Image image )
{
  if( mImage != image )
  {
    mUrl.clear();
    mPropertyMap.Clear();

    mImage = image;

    Actor self = Self();
    Toolkit::RendererFactory::Get().ResetRenderer( mRenderer, self, image );
    mImageSize = image ? ImageDimensions( image.GetWidth(), image.GetHeight() ) : ImageDimensions( 0, 0 );
  }
}

void ImageView::SetImage( Property::Map map )
{
  mPropertyMap = map;

  Actor self = Self();
  Toolkit::RendererFactory::Get().ResetRenderer( mRenderer, self, mPropertyMap );

  int width = 0;
  Property::Value* widthValue = mPropertyMap.Find( "width" );
  if( widthValue )
  {
    widthValue->Get( width );
  }

  int height = 0;
  Property::Value* heightValue = mPropertyMap.Find( "height" );
  if( heightValue )
  {
    heightValue->Get( height );
  }

  mImageSize = ImageDimensions( width, height );
}

void ImageView::SetImage( const std::string& url )
{
  if( mUrl != url )
  {
    mImage.Reset();
    mPropertyMap.Clear();

    mUrl = url;

    Actor self = Self();
    Toolkit::RendererFactory::Get().ResetRenderer( mRenderer, self, mUrl );

    mImageSize = ResourceImage::GetImageSize( mUrl );
  }
}

Vector3 ImageView::GetNaturalSize()
{
  Vector3 size;

  size.x = mImageSize.GetWidth();
  size.y = mImageSize.GetHeight();
  size.z = std::min(size.x, size.y);

  if( size.x > 0 && size.y > 0 )
  {
    return size;
  }
  else
  {
    // if no image then use Control's natural size
    return Control::GetNaturalSize();
  }
}

float ImageView::GetHeightForWidth( float width )
{
  if( mImageSize.GetWidth() > 0 && mImageSize.GetHeight() > 0 )
  {
    return GetHeightForWidthBase( width );
  }
  else
  {
    return Control::GetHeightForWidth( width );
  }
}

float ImageView::GetWidthForHeight( float height )
{
  if( mImageSize.GetWidth() > 0 && mImageSize.GetHeight() > 0 )
  {
    return GetWidthForHeightBase( height );
  }
  else
  {
    return Control::GetWidthForHeight( height );
  }
}

///////////////////////////////////////////////////////////
//
// Private methods
//

void ImageView::OnStageConnection( int depth )
{
  Control::OnStageConnection( depth );

  if( mRenderer )
  {
    CustomActor self = Self();
    mRenderer.SetOnStage( self );
  }
}

void ImageView::OnStageDisconnection()
{
  if( mRenderer )
  {
    CustomActor self = Self();
    mRenderer.SetOffStage( self );
  }

  Control::OnStageDisconnection();
}


///////////////////////////////////////////////////////////
//
// Properties
//

void ImageView::SetProperty( BaseObject* object, Property::Index index, const Property::Value& value )
{
  Toolkit::ImageView imageView = Toolkit::ImageView::DownCast( Dali::BaseHandle( object ) );

  if ( imageView )
  {
    switch ( index )
    {
      case Toolkit::ImageView::Property::IMAGE:
      {
        std::string imageUrl;
        if( value.Get( imageUrl ) )
        {
          ImageView& impl = GetImpl( imageView );
          impl.SetImage( imageUrl );
        }

        // if its not a string then get a Property::Map from the property if possible.
        Property::Map map;
        if( value.Get( map ) )
        {
          ImageView& impl = GetImpl( imageView );
          impl.SetImage( map );
        }

        break;
      }
    }
  }
}

Property::Value ImageView::GetProperty( BaseObject* object, Property::Index propertyIndex )
{
  Property::Value value;

  Toolkit::ImageView imageview = Toolkit::ImageView::DownCast( Dali::BaseHandle( object ) );

  if ( imageview )
  {
    switch ( propertyIndex )
    {
      case Toolkit::ImageView::Property::IMAGE:
      {
        ImageView& impl = GetImpl( imageview );
        if ( !impl.mUrl.empty() )
        {
          value = impl.mUrl;
        }
        else if( impl.mImage )
        {
          Property::Map map;
          Scripting::CreatePropertyMap( impl.mImage, map );
          value = map;
        }
        else if( !impl.mPropertyMap.Empty() )
        {
          value = impl.mPropertyMap;
        }
        break;
      }
    }
  }

  return value;
}

} // namespace Internal
} // namespace Toolkit
} // namespace Dali
