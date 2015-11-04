#ifndef __DALI_TOOLKIT_SCROLL_BAR_H__
#define __DALI_TOOLKIT_SCROLL_BAR_H__

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

// INTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/control.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal DALI_INTERNAL
{
// Forward declarations

class ScrollBar;
}
/**
 * @addtogroup dali_toolkit_controls_scroll_bar
 * @{
 */

/**
 * ScrollBar is a UI component that can be linked to the scrollable objects
 * indicating the current scroll position of the scrollable object.
 *
 * Signals
 * | %Signal Name                     | Method                                     |
 * |----------------------------------|--------------------------------------------|
 * | pan-finished                     | @ref PanFinishedSignal()                   |
 * | scroll-position-interval-reached | @ref ScrollPositionIntervalReachedSignal() |
 */
class DALI_IMPORT_API ScrollBar : public Control
{
public:

  // Properties

  /**
   * @brief The start and end property ranges for this control.
   */
  enum PropertyRange
  {
    PROPERTY_START_INDEX = Control::CONTROL_PROPERTY_END_INDEX + 1,
    PROPERTY_END_INDEX =   PROPERTY_START_INDEX + 1000              ///< Reserve property indices
  };

  /**
   * @brief An enumeration of properties belonging to the ScrollBar class.
   */
  struct Property
  {
    enum
    {
      SCROLL_DIRECTION = PROPERTY_START_INDEX, ///< name "scroll-direction",         @see SetScrollDirection(),        type std::string
      INDICATOR_HEIGHT_POLICY,                 ///< name "indicator-height-policy",  @see SetIndicatorHeightPolicy(),  type std::string
      INDICATOR_FIXED_HEIGHT,                  ///< name "indicator-fixed-height",   @see SetIndicatorFixedHeight(),   type float
      INDICATOR_SHOW_DURATION,                 ///< name "indicator-show-duration",  @see SetIndicatorShowDuration(),  type float
      INDICATOR_HIDE_DURATION,                 ///< name "indicator-hide-duration",  @see SetIndicatorHideDuration(),  type float
      SCROLL_POSITION_INTERVALS                ///< name "scroll-position-intervals",@see SetScrollPositionIntervals() type Property::Array
    };
  };

  // Signals

  typedef Signal< void () > PanFinishedSignalType;
  typedef Signal< void ( float ) > ScrollPositionIntervalReachedSignalType;

public:

  /**
   * @brief Direction.
   */
  enum Direction
  {
    Vertical = 0,   ///< Scroll in the vertical direction
    Horizontal      ///< Scroll in the horizontal direction
  };

  /**
   * @brief Indicator height policy.
   */
  enum IndicatorHeightPolicy
  {
    Variable = 0,  ///< Variable height changed dynamically according to the length of scroll content
    Fixed          ///< Fixed height regardless of the length of scroll content
  };

  /**
   * @brief Create an uninitialized ScrollBar; this can be initialized with ScrollBar::New()
   * Calling member functions with an uninitialized Dali::Object is not allowed.
   */
  ScrollBar();

  /**
   * @brief Copy constructor.
   */
  ScrollBar( const ScrollBar& scrollBar );

  /**
   * @brief Assignment operator.
   */
  ScrollBar& operator=( const ScrollBar& scrollBar );

  /**
   * @brief Destructor
   *
   * This is non-virtual since derived Handle types must not contain data or virtual methods.
   */
  ~ScrollBar();

  /**
   * @brief Create an initialized ScrollBar
   * @param[in] direction The direction of scroll bar (either vertically or horizontally).
   * @return A pointer to the created ScrollBar.
   */
  static ScrollBar New(Direction direction = Vertical);

  /**
   * @brief Downcast an Object handle to ScrollBar. If handle points to a ScrollBar the
   * downcast produces valid handle. If not the returned handle is left uninitialized.
   * @param[in] handle Handle to an object
   * @return handle to a ScrollBar or an uninitialized handle
   */
  static ScrollBar DownCast( BaseHandle handle );

  /**
   * @brief Set the source of the scroll position properties.
   *
   * * @pre The handle to the object owing the scroll properties has been initialised and the property index must be vaild.
   *
   * @param[in] handle The handle of the object owing the scroll properties.
   * @param[in] propertyScrollPosition The index of the scroll position property (The scroll position, type float).
   * @param[in] propertyMinScrollPosition The index of the minimum scroll position property (The minimum scroll position, type float).
   * @param[in] propertyMaxScrollPosition The index of the maximum scroll position property (The maximum scroll position, type float).
   * @param[in] propertyScrollContentSize The index of the scroll content size property (The size of the scrollable content in actor coordinates, type float).
   */
  void SetScrollPropertySource( Handle handle, Dali::Property::Index propertyScrollPosition, Dali::Property::Index propertyMinScrollPosition, Dali::Property::Index propertyMaxScrollPosition, Dali::Property::Index propertyScrollContentSize );

  /**
   * @brief Sets the indicator of scroll bar.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @param[in] indicator The indicator that moves to indicate the current scroll position.
   */
  void SetScrollIndicator( Actor indicator );

  /**
   * @brief Gets the indicator of scroll bar.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @return The indicator indicates the current scroll position of the scrollable content.
   */
  Actor GetScrollIndicator();

  /**
   * @brief Sets the list of values to get notification when the current scroll position of the scrollable
   * object goes above or below any of these values.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @param[in] positions List of values to receive notifications for when the current scroll position crosses them
   */
  void SetScrollPositionIntervals( const Dali::Vector<float>& positions );

  /**
   * @brief Gets the list of values to receive notifications when the current scroll position of the scrollable
   * object goes above or below any of these values.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @return The list of values to receive notifications for when the current scroll position crosses them
   */
  Dali::Vector<float> GetScrollPositionIntervals() const;

  /**
   * @brief Sets the direction of scroll bar to scroll either vertically or horizontally.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @param[in] direction The direction of scroll bar (either vertically or horizontally).
   */
  void SetScrollDirection( Direction direction );

  /**
   * @brief Gets the direction of scroll bar.
   *
   * @return The direction of scroll bar.
   */
  Direction GetScrollDirection() const;

  /**
   * @brief Sets the height policy of scroll indicator to have either variable or fixed height.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @param[in] policy The height policy of scroll indicator
   */
  void SetIndicatorHeightPolicy( IndicatorHeightPolicy policy );

  /**
   * @brief Gets the height policy of scroll indicator.
   *
   * @return The height policy of scroll indicator
   */
  IndicatorHeightPolicy GetIndicatorHeightPolicy() const;

  /**
   * @brief Sets the fixed height of scroll indicator.
   * Normally the height of scroll indicator is changed dynamically according to the length of scroll content.
   * However, when the height policy of scroll indicator is set to be fixed, the height will keep fixed
   * regardless of the length of scroll content.
   *
   * @pre The scroll bar actor has been initialised.
   *
   * @param[in] height The fixed height of the scroll indicator
   */
  void SetIndicatorFixedHeight( float height );

  /**
   * @brief Gets the fix height of scroll indicator.
   * @return The fixed height of the scroll indicator
   */
  float GetIndicatorFixedHeight() const;

  /**
   * @brief Sets the duration in second for the scroll indicator to become fully visible
   *
   * @pre The scroll bar actor has been initialised; durationSeconds must be zero or greater; zero means the indicator will be shown instantly.
   *
   * @param[in] durationSeconds The duration for the scroll indicator to become fully visible
   */
  void SetIndicatorShowDuration( float durationSeconds );

  /**
   * @brief Gets the duration in second for the scroll indicator to become fully visible
   * @return The duration for the scroll indicator to become fully visible
   */
  float GetIndicatorShowDuration() const;

  /**
   * @brief Sets the duration in second for the scroll indicator to become fully invisible
   *
   * @pre The scroll bar actor has been initialised; durationSeconds must be zero or greater; zero means the indicator will be hidden instantly.
   *
   * @param[in] durationSeconds The duration for the scroll indicator to become fully invisible
   */
  void SetIndicatorHideDuration( float durationSeconds );

  /**
   * @brief Gets the duration in second for the scroll indicator to become fully invisible
   * @return The duration for the scroll indicator to become fully invisible
   */
  float GetIndicatorHideDuration() const;

  /**
   * @brief Shows the scroll indicator
   */
  void ShowIndicator();

  /**
   * @brief Hides the scroll indicator
   */
  void HideIndicator();

public: // Signals

  /**
   * @brief Signal emitted when panning is finished on the scroll indicator.
   * Signal only emitted when the source of the scroll position properties are set.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName();
   * @endcode
   * @pre The Object has been initialized.
   * @return The signal to connect to.
   */
  ScrollBar::PanFinishedSignalType& PanFinishedSignal();

  /**
   * @brief Signal emitted when the current scroll position of the scrollable content
   * goes above or below the values specified by SCROLL_POSITION_INTERVALS property.
   * Signal only emitted when the source of the scroll position properties are set.
   *
   * A callback of the following type may be connected:
   * @code
   *   void YourCallbackName(float currentScrollPosition);
   * @endcode
   * @pre The Object has been initialized.
   * @return The signal to connect to.
   */
  ScrollBar::ScrollPositionIntervalReachedSignalType& ScrollPositionIntervalReachedSignal();

public: // Not intended for application developers

  /**
   * Creates a handle using the Toolkit::Internal implementation.
   * @param[in]  implementation  The Control implementation.
   */
  DALI_INTERNAL ScrollBar( Internal::ScrollBar& implementation );

  /**
   * Allows the creation of this Control from an Internal::CustomActor pointer.
   * @param[in]  internal  A pointer to the internal CustomActor.
   */
  explicit DALI_INTERNAL ScrollBar( Dali::Internal::CustomActor* internal );
};

/**
 * @}
 */
} // namespace Toolkit

} // namespace Dali

#endif // __DALI_TOOLKIT_SCROLL_BAR_H__
