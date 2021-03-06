#ifndef __DALI_INTERNAL_IMAGE_ATTACHMENT_H__
#define __DALI_INTERNAL_IMAGE_ATTACHMENT_H__

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

// INTERNAL INCLUDES
#include <dali/public-api/actors/image-actor.h>
#include <dali/public-api/actors/blending.h>
#include <dali/devel-api/rendering/cull-face.h>
#include <dali/internal/common/blending-options.h>
#include <dali/internal/common/image-sampler.h>
#include <dali/internal/event/actor-attachments/actor-attachment-declarations.h>
#include <dali/internal/event/actor-attachments/renderable-attachment-impl.h>
#include <dali/internal/event/images/image-impl.h>
#include <dali/internal/event/images/image-connector.h>
#include <dali/internal/event/effects/shader-declarations.h>

namespace Dali
{

namespace Internal
{

namespace SceneGraph
{
class ImageAttachment;
class Node;
}

/**
 * An attachment for rendering images in various ways.
 */
class ImageAttachment : public RenderableAttachment
{
public:

  typedef Dali::ImageActor::Style Style;
  typedef Dali::ImageActor::PixelArea PixelArea;

  /**
   * Create a new ImageAttachment.
   * @param[in] eventThreadServices to use
   * @param[in] parentNode The node to attach a scene-object to.
   * @return A smart-pointer to the newly allocated ImageAttachment.
   */
  static ImageAttachmentPtr New( EventThreadServices& eventThreadServices, const SceneGraph::Node& parentNode );

  /**
   * Sets image rendered by the attachment.
   * @param [in] image A pointer to the image to display or NULL to clear. Reference to avoid unnecessary increment/decrement reference count.
   */
  void SetImage( ImagePtr& image );

  /**
   * Retrieve the image rendered by the attachment.
   * @return The image or an uninitialized image in case the ImageActor was cleared.
   */
  ImagePtr GetImage();

  /**
   * Set a region of the image to display, in pixels.
   * @param [in] pixelArea The area of the image to display.
   * This in pixels, relative to the top-left (0,0) of the image.
   */
  void SetPixelArea(const PixelArea& pixelArea);

  /**
   * Retrieve the region of the image to display, in pixels.
   * @return The pixel area, or a default-constructed area if none was set.
   */
  const PixelArea& GetPixelArea() const
  {
    // This is not animatable; the cached value is up-to-date.
    return mPixelArea;
  }

  /**
   * Query whether a pixel area has been set.
   * @return True if a pixel area has been set.
   */
  bool IsPixelAreaSet() const
  {
    // This is not animatable; the cached value is up-to-date.
    return mIsPixelAreaSet;
  }

  /**
   * Remove any pixel areas specified with SetPixelArea; the entire image will be displayed.
   * @pre image must be initialized.
   */
  void ClearPixelArea();

  /**
   * Set how the ImageAttachment is rendered; the default is STYLE_QUAD.
   * @param [in] style The new style.
   */
  void SetStyle(Style style);

  /**
   * Query how the image is rendered.
   * @return The rendering style.
   */
  Style GetStyle()
  {
    // This is not animatable; the cached value is up-to-date.
    return mStyle;
  }

  /**
   * @copydoc Dali::ImageActor::SetNinePatchBorder
   */
  void SetNinePatchBorder(const Vector4& border, bool inPixels);

  /**
   * @copydoc Dali::ImageActor::GetNinePatchBorder
   */
  Vector4 GetNinePatchBorder()
  {
    // This is not animatable; the cached value is up-to-date.
    return mBorder;
  }


  /**
   * Set the depth-sort modifier for the renderable.
   * This modifies the back-to-front distance calculation, when rendering with transparency.
   * This is useful for ordering transparent objects that are positioned close to each other.
   * @param[in] modifier The depth-sort modifier.
   */
  void SetSortModifier(float modifier);

  /**
   * From Renderable; Retrieve the depth-sort modifier for the renderable.
   * @return The depth-sort modifier.
   */
  float GetSortModifier() const;

  /**
   * Set the face-culling mode for this attachment.
   * @param[in] mode The culling mode.
   */
  void SetCullFace(CullFaceMode mode);

  /**
   * Retrieve the face-culling mode for this attachment.
   * @return mode The culling mode.
   */
  CullFaceMode GetCullFace() const;

  /**
   * @copydoc Dali::RenderableActor::SetBlendMode()
   */
  void SetBlendMode( BlendingMode::Type mode );

  /**
   * @copydoc Dali::RenderableActor::GetBlendMode()
   */
  BlendingMode::Type GetBlendMode() const;

  /**
   * @copydoc Dali::RenderableActor::SetBlendFunc()
   */
  void SetBlendFunc( BlendingFactor::Type srcFactorRgb,   BlendingFactor::Type destFactorRgb,
                     BlendingFactor::Type srcFactorAlpha, BlendingFactor::Type destFactorAlpha );

  /**
   * @copydoc Dali::RenderableActor::GetBlendFunc()
   */
  void GetBlendFunc( BlendingFactor::Type& srcFactorRgb,   BlendingFactor::Type& destFactorRgb,
                     BlendingFactor::Type& srcFactorAlpha, BlendingFactor::Type& destFactorAlpha ) const;

  /**
   * @copydoc Dali::RenderableActor::SetBlendEquation()
   */
  void SetBlendEquation( BlendingEquation::Type equationRgb, BlendingEquation::Type equationAlpha );

  /**
   * @copydoc Dali::RenderableActor::GetBlendEquation()
   */
  void GetBlendEquation( BlendingEquation::Type& equationRgb, BlendingEquation::Type& equationAlpha ) const;

  /**
   * @copydoc Dali::RenderableActor::SetBlendColor()
   */
  void SetBlendColor( const Vector4& color );

  /**
   * @copydoc Dali::RenderableActor::GetBlendColor()
   */
  const Vector4& GetBlendColor() const;

  /**
   * @copydoc Dali::RenderableActor::SetFilterMode()
   */
  void SetFilterMode( FilterMode::Type minFilter, FilterMode::Type magFilter );

  /**
   * @copydoc Dali::RenderableActor::GetFilterMode()
   */
  void GetFilterMode( FilterMode::Type& minFilter, FilterMode::Type& magFilter ) const;

  /**
   * @copydoc Dali::RenderableActor::SetShaderEffect
   */
  void SetShaderEffect(ShaderEffect& effect);

  /**
   * @copydoc Dali::RenderableActor::GetShaderEffect
   */
  ShaderEffectPtr GetShaderEffect() const;

  /**
   * @copydoc Dali::RenderableActor::RemoveShaderEffect
   */
  void RemoveShaderEffect();

private:

  /**
   * First stage construction of a ImageAttachment.
   * @param[in] eventThreadServices Used for messaging to and reading from scene-graph.
   */
  ImageAttachment( EventThreadServices& eventThreadServices );

  /**
   * Creates the corresponding scene-graph ImageAttachment.
   * @return A newly allocated scene object.
   */
  static SceneGraph::ImageAttachment* CreateSceneObject();

  /**
   * @copydoc Dali::Internal::RenderableAttachment::OnStageConnection2()
   */
  virtual void OnStageConnection2();

  /**
   * @copydoc Dali::Internal::RenderableAttachment::OnStageDisconnection2()
   */
  virtual void OnStageDisconnection2();

  /**
   * @copydoc Dali::Internal::RenderableAttachment::GetSceneObject()
   */
  const SceneGraph::ImageAttachment& GetSceneObject() const;

protected:

  /**
   * A reference counted object may only be deleted by calling Unreference()
   */
  virtual ~ImageAttachment();

private:

  const SceneGraph::ImageAttachment* mSceneObject; ///< Not owned

  ImageConnector mImageConnectable; ///< Manages the image displayed by the attachment

  // Cached for public getters

  PixelArea mPixelArea;

  Style mStyle;
  Vector4 mBorder;

  bool mIsPixelAreaSet : 1;
  bool mBorderInPixels : 1;

  BlendingOptions    mBlendingOptions;
  unsigned int       mSamplerBitfield;
  float              mSortModifier;
  CullFaceMode       mCullFaceMode:3;  ///< cullface mode, 3 bits enough for 4 values
  BlendingMode::Type mBlendingMode:2;  ///< blending mode, 2 bits enough for 3 values

  ShaderEffectPtr    mShaderEffect;    ///< Optional referenced shader effect
};

} // namespace Internal

} // namespace Dali

#endif // __DALI_INTERNAL_IMAGE_ATTACHMENT_H__
