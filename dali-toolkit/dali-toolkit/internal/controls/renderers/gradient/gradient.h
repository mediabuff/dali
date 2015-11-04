#ifndef __DALI_TOOLKIT_INTERNAL_GRADIENT_H__
#define __DALI_TOOLKIT_INTERNAL_GRADIENT_H__

/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
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

//EXTERNAL INCLUDES
#include <dali/public-api/math/matrix3.h>
#include <dali/public-api/object/ref-object.h>
#include <dali/public-api/common/dali-vector.h>
#include <dali/public-api/images/buffer-image.h>

namespace Dali
{

class Vector4;

namespace Toolkit
{

namespace Internal
{

/**
 * Gradients consist of continuously smooth color transitions along a vector from one color to another,
 * possibly followed by additional transitions along the same vector to other colors.
 */
class Gradient : public RefObject
{
public:
  /**
   * Defines the coordinate system of the attributes
   *     (start and end position for linear gradient, circle center and radius for radial gradient)
   */
  enum GradientUnits
  {
    USER_SPACE_ON_USE,
    OBJECT_BOUNDING_BOX
  };

  /**
   * Indicates what happens if the gradient starts or ends inside the bounds of the object being painted by the gradient.
   */
  enum SpreadMethod
  {
    PAD,      // use the terminal colors of the gradient to fill the remainder of the target region
    REPEAT,   // reflect the gradient pattern start-to-end, end-to-start, start-to-end, etc. continuously until the target rectangle is filled
    REFLECT   // repeat the gradient pattern start-to-end, start-to-end, start-to-end, etc. continuously until the target region is filled
  };

  /**
   * The stop node tells the gradient what color it should be at certain position.
   */
  struct GradientStop
  {
    GradientStop( float offset, const Vector4& color )
    : mOffset( offset ), mStopColor( color )
    {}

    bool operator<(const GradientStop& rhs) const
    {
      return mOffset < rhs.mOffset;
    }

    float   mOffset;     // A value ranging from 0 to 1 to indicate where the gradient stop is placed.
    Vector4 mStopColor;  // The color to use at this gradient stop
  };

public:

  /**
   * Add a gradient stop.
   *
   * @param[in] offset The position to place the stop.
   * @param[in] color  The color to use at this stop.
   */
  void AddStop(float offset, const Vector4& color);

  /**
   * Get the gradient stops.
   * @return The vector of gradient stops.
   */
  const Vector<GradientStop>& GetStops();

  /**
   * Set the coordinate system used by the gradient attributes.
   * @param[in] gradientUnits The the attributes are defined using the current user coordinate system or the bounding box of the shape.
   */
  void SetGradientUnits( GradientUnits gradientUnits );

  /**
   * Get the coordinate system used by the gradient attributes.
   * @return USER_SPACE_ON_USE or OBJECT_BOUNDING_BOX
   */
  GradientUnits GetGradientUnits() const;

  /**
   * Indicates what happens if the gradient starts or ends inside the bounds of the target rectangle.
   * If not specified, the effect is as if a value of 'pad' were specified
   *
   * @param[in] spread The method to fill the remainder of target region which is outside the gradient bounds
   */
  void SetSpreadMethod( SpreadMethod spread );

  /**
   * Get the filling method for the the remainder of target region which is outside the gradient boun.
   * @return PAD, REFLECT or REPEAT
   */
  SpreadMethod GetSpreadMethod() const;

  /**
   * Get the transformation matrix to align the vertices with the gradient line/circle
   * @ return the aligning transformation matrix
   */
  const Matrix3& GetAlignmentTransform() const;

  /**
   * Generate the lookup texture with the gradient stops.
   * @return the lookup texture which transit smoothly between stops.
   */
  BufferImage GenerateLookupTexture();

private:

  /**
   * Estimate the resolution of the lookup texture.
   * Note: Only call this function after the gradient stops are sorted in order.
   */
  unsigned int EstimateTextureResolution();

protected:

  /**
   * Construct a new Gradient object
   * Called in the constructor of subclasses
   */
  Gradient();

  /**
   * @brief A reference counted object may only be deleted by calling Unreference().
   */
  virtual ~Gradient();

  // Undefined
  Gradient( const Gradient& gradient );

  // Undefined
  Gradient& operator=( const Gradient& handle );

protected:

  Vector<GradientStop>      mGradientStops;
  Matrix3                   mAlignmentTransform;
  GradientUnits             mGradientUnits;
  SpreadMethod              mSpreadMethod;

};

} // namespace Internal

} // namespace Toolkit

} // namespace Dali

#endif /* __DALI_TOOLKIT_INTERNAL_GRADIENT_RENDERER_H__ */
