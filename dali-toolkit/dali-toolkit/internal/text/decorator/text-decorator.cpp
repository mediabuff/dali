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

// CLASS HEADER
#include <dali-toolkit/internal/text/decorator/text-decorator.h>

// EXTERNAL INCLUDES
#include <dali/integration-api/debug.h>

#include <dali/public-api/adaptor-framework/timer.h>
#include <dali/public-api/actors/layer.h>
#include <dali/public-api/common/stage.h>
#include <dali/public-api/events/touch-event.h>
#include <dali/public-api/events/pan-gesture.h>
#include <dali/public-api/images/resource-image.h>
#include <dali/public-api/object/property-notification.h>

#include <dali/devel-api/rendering/geometry.h>
#include <dali/devel-api/rendering/renderer.h>

// INTERNAL INCLUDES
#include <dali-toolkit/public-api/controls/control-depth-index-ranges.h>
#include <dali-toolkit/public-api/controls/default-controls/solid-color-actor.h>

#ifdef DEBUG_ENABLED
#define DECORATOR_DEBUG

#endif

#define MAKE_SHADER(A)#A

namespace
{
const char* VERTEX_SHADER = MAKE_SHADER(
attribute mediump vec2    aPosition;
uniform   mediump mat4    uMvpMatrix;
uniform   mediump vec3    uSize;

void main()
{
  mediump vec4 position = vec4( aPosition, 0.0, 1.0 );
  position.xyz *= uSize;
  gl_Position = uMvpMatrix * position;
}
);

const char* FRAGMENT_SHADER = MAKE_SHADER(
uniform      lowp vec4 uColor;

void main()
{
  gl_FragColor = uColor;
}
);
}

namespace Dali
{
namespace Internal
{
namespace
{
#ifdef DECORATOR_DEBUG
Integration::Log::Filter* gLogFilter( Integration::Log::Filter::New(Debug::NoLogging, false, "LOG_TEXT_DECORATOR") );
#endif
}
}
}


// Local Data
namespace
{
const Dali::Vector3 DEFAULT_GRAB_HANDLE_RELATIVE_SIZE( 1.25f, 1.5f, 1.0f );
const Dali::Vector3 DEFAULT_SELECTION_HANDLE_RELATIVE_SIZE( 1.25f, 1.5f, 1.0f );

const Dali::Vector4 LIGHT_BLUE( 0.75f, 0.96f, 1.f, 1.f ); // The text highlight color. TODO: due some problems, maybe with the blending function in the text clipping, the color is fully opaque.

const Dali::Vector4 HANDLE_COLOR( 0.0f, (183.0f / 255.0f), (229.0f / 255.0f), 1.0f  );

const unsigned int CURSOR_BLINK_INTERVAL = 500u; // Cursor blink interval
const float TO_MILLISECONDS = 1000.f;
const float TO_SECONDS = 1.f / TO_MILLISECONDS;

const unsigned int SCROLL_TICK_INTERVAL = 50u;

const float SCROLL_THRESHOLD = 10.f;
const float SCROLL_SPEED = 300.f;
const float SCROLL_DISTANCE = SCROLL_SPEED * SCROLL_TICK_INTERVAL * TO_SECONDS;

const float CURSOR_WIDTH = 1.f;

/**
 * structure to hold coordinates of each quad, which will make up the mesh.
 */
struct QuadCoordinates
{
  /**
   * Default constructor
   */
  QuadCoordinates()
  {
  }

  /**
   * Constructor
   * @param[in] x1 left co-ordinate
   * @param[in] y1 top co-ordinate
   * @param[in] x2 right co-ordinate
   * @param[in] y2 bottom co-ordinate
   */
  QuadCoordinates(float x1, float y1, float x2, float y2)
  : min(x1, y1),
    max(x2, y2)
  {
  }

  Dali::Vector2 min;                          ///< top-left (minimum) position of quad
  Dali::Vector2 max;                          ///< bottom-right (maximum) position of quad
};

typedef std::vector<QuadCoordinates> QuadContainer;

/**
 * @brief Takes a bounding rectangle in the local coordinates of an actor and returns the world coordinates Bounding Box.
 * @param[in] boundingRectangle local bounding
 * @param[out] Vector4 World coordinate bounding Box.
 */
void LocalToWorldCoordinatesBoundingBox( const Dali::Rect<int>& boundingRectangle, Dali::Vector4& boundingBox )
{
  // Convert to world coordinates and store as a Vector4 to be compatible with Property Notifications.
  Dali::Vector2 stageSize = Dali::Stage::GetCurrent().GetSize();

  const float originX = boundingRectangle.x - 0.5f * stageSize.width;
  const float originY = boundingRectangle.y - 0.5f * stageSize.height;

  boundingBox = Dali::Vector4( originX,
                               originY,
                               originX + boundingRectangle.width,
                               originY + boundingRectangle.height );
}

void WorldToLocalCoordinatesBoundingBox( const Dali::Vector4& boundingBox, Dali::Rect<int>& boundingRectangle )
{
  // Convert to local coordinates and store as a Dali::Rect.
  Dali::Vector2 stageSize = Dali::Stage::GetCurrent().GetSize();

  boundingRectangle.x = boundingBox.x + 0.5f * stageSize.width;
  boundingRectangle.y = boundingBox.y + 0.5f * stageSize.height;
  boundingRectangle.width = boundingBox.z - boundingBox.x;
  boundingRectangle.height = boundingBox.w - boundingBox.y;
}

} // end of namespace

namespace Dali
{

namespace Toolkit
{

namespace Text
{

struct Decorator::Impl : public ConnectionTracker
{
  enum ScrollDirection
  {
    SCROLL_NONE,
    SCROLL_RIGHT,
    SCROLL_LEFT,
    SCROLL_TOP,
    SCROLL_BOTTOM
  };

  struct CursorImpl
  {
    CursorImpl()
    : color( Dali::Color::BLACK ),
      position(),
      cursorHeight( 0.0f ),
      lineHeight( 0.0f )
    {
    }

    Vector4 color;
    Vector2 position;
    float cursorHeight;
    float lineHeight;
  };

  struct HandleImpl
  {
    HandleImpl()
    : position(),
      size(),
      lineHeight( 0.0f ),
      grabDisplacementX( 0.f ),
      grabDisplacementY( 0.f ),
      active( false ),
      visible( false ),
      pressed( false ),
      verticallyFlippedPreferred( false ),
      horizontallyFlipped( false ),
      verticallyFlipped( false )
    {
    }

    ImageActor actor;
    Actor grabArea;
    ImageActor markerActor;

    Vector2 position;
    Size    size;
    float   lineHeight;              ///< Not the handle height
    float   grabDisplacementX;
    float   grabDisplacementY;
    bool    active  : 1;
    bool    visible : 1;
    bool    pressed : 1;
    bool    verticallyFlippedPreferred : 1; ///< Whether the handle is preferred to be vertically flipped.
    bool    horizontallyFlipped        : 1; ///< Whether the handle has been horizontally flipped.
    bool    verticallyFlipped          : 1; ///< Whether the handle has been vertically flipped.
  };

  struct PopupImpl
  {
    PopupImpl()
    : position()
    {
    }

    TextSelectionPopup actor;
    Vector3 position;
  };

  Impl( ControllerInterface& controller,
        TextSelectionPopupCallbackInterface& callbackInterface )
  : mController( controller ),
    mEnabledPopupButtons( TextSelectionPopup::NONE ),
    mTextSelectionPopupCallbackInterface( callbackInterface ),
    mHandleColor( HANDLE_COLOR ),
    mBoundingBox(),
    mHighlightColor( LIGHT_BLUE ),
    mHighlightPosition( Vector2::ZERO ),
    mActiveCursor( ACTIVE_CURSOR_NONE ),
    mCursorBlinkInterval( CURSOR_BLINK_INTERVAL ),
    mCursorBlinkDuration( 0.0f ),
    mCursorWidth( CURSOR_WIDTH ),
    mHandleScrolling( HANDLE_TYPE_COUNT ),
    mScrollDirection( SCROLL_NONE ),
    mScrollThreshold( SCROLL_THRESHOLD ),
    mScrollSpeed( SCROLL_SPEED ),
    mScrollDistance( SCROLL_DISTANCE ),
    mTextDepth( 0u ),
    mActiveCopyPastePopup( false ),
    mPopupSetNewPosition( true ),
    mCursorBlinkStatus( true ),
    mDelayCursorBlink( false ),
    mPrimaryCursorVisible( false ),
    mSecondaryCursorVisible( false ),
    mFlipSelectionHandlesOnCross( false ),
    mFlipLeftSelectionHandleDirection( false ),
    mFlipRightSelectionHandleDirection( false ),
    mHandlePanning( false ),
    mHandleCurrentCrossed( false ),
    mHandlePreviousCrossed( false ),
    mNotifyEndOfScroll( false )
  {
    mQuadVertexFormat[ "aPosition" ] = Property::VECTOR2;
    mQuadIndexFormat[ "indices" ] = Property::INTEGER;
    mHighlightMaterial = Material::New( Shader::New( VERTEX_SHADER, FRAGMENT_SHADER ) );

    SetupTouchEvents();
  }

  /**
   * Relayout of the decorations owned by the decorator.
   * @param[in] size The Size of the UI control the decorator is adding it's decorations to.
   */
  void Relayout( const Vector2& size )
  {
    mControlSize = size;

    // TODO - Remove this if nothing is active
    CreateActiveLayer();

    // Show or hide the cursors
    CreateCursors();

    if( mPrimaryCursor )
    {
      const CursorImpl& cursor = mCursor[PRIMARY_CURSOR];
      mPrimaryCursorVisible = ( cursor.position.x + mCursorWidth <= mControlSize.width ) && ( cursor.position.x >= 0.f );
      if( mPrimaryCursorVisible )
      {
        mPrimaryCursor.SetPosition( cursor.position.x,
                                    cursor.position.y );
        mPrimaryCursor.SetSize( Size( mCursorWidth, cursor.cursorHeight ) );
      }
      mPrimaryCursor.SetVisible( mPrimaryCursorVisible && mCursorBlinkStatus );
    }
    if( mSecondaryCursor )
    {
      const CursorImpl& cursor = mCursor[SECONDARY_CURSOR];
      mSecondaryCursorVisible = ( cursor.position.x + mCursorWidth <= mControlSize.width ) && ( cursor.position.x >= 0.f );
      if( mSecondaryCursorVisible )
      {
        mSecondaryCursor.SetPosition( cursor.position.x,
                                      cursor.position.y );
        mSecondaryCursor.SetSize( Size( mCursorWidth, cursor.cursorHeight ) );
      }
      mSecondaryCursor.SetVisible( mSecondaryCursorVisible && mCursorBlinkStatus );
    }

    // Show or hide the grab handle
    HandleImpl& grabHandle = mHandle[GRAB_HANDLE];
    bool newGrabHandlePosition = false;
    if( grabHandle.active )
    {
      const bool isVisible = ( grabHandle.position.x + floor( 0.5f * mCursorWidth ) <= mControlSize.width ) && ( grabHandle.position.x >= 0.f );

      if( isVisible )
      {
        CreateGrabHandle();

        // Sets the grab handle position and calculate if it needs to be vertically flipped if it exceeds the boundary box.
        SetGrabHandlePosition();

        // Sets the grab handle image according if it's pressed, flipped, etc.
        SetHandleImage( GRAB_HANDLE );

        newGrabHandlePosition = true;
      }

      if( grabHandle.actor )
      {
        grabHandle.actor.SetVisible( isVisible );
      }
    }
    else if( grabHandle.actor )
    {
      grabHandle.actor.Unparent();
    }

    // Show or hide the selection handles/highlight
    HandleImpl& primary = mHandle[ LEFT_SELECTION_HANDLE ];
    HandleImpl& secondary = mHandle[ RIGHT_SELECTION_HANDLE ];
    bool newPrimaryHandlePosition = false;
    bool newSecondaryHandlePosition = false;
    if( primary.active || secondary.active )
    {
      const bool isPrimaryVisible = ( primary.position.x <= mControlSize.width ) && ( primary.position.x >= 0.f );
      const bool isSecondaryVisible = ( secondary.position.x <= mControlSize.width ) && ( secondary.position.x >= 0.f );

      if( isPrimaryVisible || isSecondaryVisible )
      {
        CreateSelectionHandles();

        if( isPrimaryVisible )
        {
          SetSelectionHandlePosition( LEFT_SELECTION_HANDLE );

          // Sets the primary handle image according if it's pressed, flipped, etc.
          SetHandleImage( LEFT_SELECTION_HANDLE );

          SetSelectionHandleMarkerSize( primary );

          newPrimaryHandlePosition = true;
        }

        if( isSecondaryVisible )
        {
          SetSelectionHandlePosition( RIGHT_SELECTION_HANDLE );

          // Sets the secondary handle image according if it's pressed, flipped, etc.
          SetHandleImage( RIGHT_SELECTION_HANDLE );

          SetSelectionHandleMarkerSize( secondary );

          newSecondaryHandlePosition = true;
        }
      }

      if( primary.actor )
      {
        primary.actor.SetVisible( isPrimaryVisible );
      }
      if( secondary.actor )
      {
        secondary.actor.SetVisible( isSecondaryVisible );
      }

      CreateHighlight();
      UpdateHighlight();
    }
    else
    {
      if( primary.actor )
      {
        primary.actor.Unparent();
      }
      if( secondary.actor )
      {
        secondary.actor.Unparent();
      }
      if( mHighlightActor )
      {
        mHighlightActor.Unparent();
      }
    }

    if( newGrabHandlePosition    ||
        newPrimaryHandlePosition ||
        newSecondaryHandlePosition )
    {
      // Setup property notifications to find whether the handles leave the boundaries of the current display.
      SetupActiveLayerPropertyNotifications();
    }

    if( mActiveCopyPastePopup )
    {
      ShowPopup();
    }
    else
    {
      if( mCopyPastePopup.actor )
      {
        mCopyPastePopup.actor.HidePopup();
        mPopupSetNewPosition = true;
      }
    }
  }

  void UpdatePositions( const Vector2& scrollOffset )
  {
    mCursor[PRIMARY_CURSOR].position += scrollOffset;
    mCursor[SECONDARY_CURSOR].position += scrollOffset;
    mHandle[ GRAB_HANDLE ].position += scrollOffset;
    mHandle[ LEFT_SELECTION_HANDLE ].position += scrollOffset;
    mHandle[ RIGHT_SELECTION_HANDLE ].position += scrollOffset;
    mHighlightPosition += scrollOffset;
  }

  void ShowPopup()
  {
    if ( !mCopyPastePopup.actor )
    {
      return;
    }

    if( !mCopyPastePopup.actor.GetParent() )
    {
      mActiveLayer.Add( mCopyPastePopup.actor );
    }

    mCopyPastePopup.actor.RaiseAbove( mActiveLayer );
    mCopyPastePopup.actor.ShowPopup();
  }

  void DeterminePositionPopup()
  {
    if( !mActiveCopyPastePopup )
    {
      return;
    }

    // Retrieves the popup's size after relayout.
    const Vector3 popupSize = Vector3( mCopyPastePopup.actor.GetRelayoutSize( Dimension::WIDTH ), mCopyPastePopup.actor.GetRelayoutSize( Dimension::HEIGHT ), 0.0f );

    if( mPopupSetNewPosition )
    {
      const HandleImpl& primaryHandle = mHandle[LEFT_SELECTION_HANDLE];
      const HandleImpl& secondaryHandle = mHandle[RIGHT_SELECTION_HANDLE];
      const HandleImpl& grabHandle = mHandle[GRAB_HANDLE];
      const CursorImpl& cursor = mCursor[PRIMARY_CURSOR];

      if( primaryHandle.active || secondaryHandle.active )
      {
        // Calculates the popup's position if selection handles are active.
        const float minHandleXPosition = std::min( primaryHandle.position.x, secondaryHandle.position.x );
        const float maxHandleXPosition = std::max( primaryHandle.position.x, secondaryHandle.position.x );
        const float maxHandleHeight = std::max( primaryHandle.size.height, secondaryHandle.size.height );

        mCopyPastePopup.position.x = minHandleXPosition + ( ( maxHandleXPosition - minHandleXPosition ) * 0.5f );
        mCopyPastePopup.position.y = -0.5f * popupSize.height - maxHandleHeight + std::min( primaryHandle.position.y, secondaryHandle.position.y );
      }
      else
      {
        // Calculates the popup's position if the grab handle is active.
        mCopyPastePopup.position = Vector3( cursor.position.x, -0.5f * popupSize.height - grabHandle.size.height + cursor.position.y, 0.0f );
      }
    }

    // Checks if there is enough space above the text control. If not it places the popup under it.
    GetConstrainedPopupPosition( mCopyPastePopup.position, popupSize * AnchorPoint::CENTER, mActiveLayer, mBoundingBox );

    SetUpPopupPositionNotifications();

    mCopyPastePopup.actor.SetPosition( mCopyPastePopup.position );
    mPopupSetNewPosition = false;
  }

  void PopupRelayoutComplete( Actor actor )
  {
    // Size negotiation for CopyPastePopup complete so can get the size and constrain position within bounding box.

    DeterminePositionPopup();
  }

  void CreateCursor( ImageActor& cursor, const Vector4& color )
  {
    cursor = CreateSolidColorActor( color );
    cursor.SetSortModifier( DECORATION_DEPTH_INDEX );
    cursor.SetParentOrigin( ParentOrigin::TOP_LEFT ); // Need to set the default parent origin as CreateSolidColorActor() sets a different one.
    cursor.SetAnchorPoint( AnchorPoint::TOP_LEFT );
  }

  // Add or Remove cursor(s) from parent
  void CreateCursors()
  {
    if( mActiveCursor == ACTIVE_CURSOR_NONE )
    {
      if( mPrimaryCursor )
      {
        mPrimaryCursor.Unparent();
      }
      if( mSecondaryCursor )
      {
        mSecondaryCursor.Unparent();
      }
    }
    else
    {
      // Create Primary and or Secondary Cursor(s) if active and add to parent
      if ( mActiveCursor == ACTIVE_CURSOR_PRIMARY ||
           mActiveCursor == ACTIVE_CURSOR_BOTH )
      {
        if ( !mPrimaryCursor )
        {
          CreateCursor( mPrimaryCursor, mCursor[PRIMARY_CURSOR].color );
#ifdef DECORATOR_DEBUG
          mPrimaryCursor.SetName( "PrimaryCursorActor" );
#endif
        }

        if( !mPrimaryCursor.GetParent() )
        {
          mActiveLayer.Add( mPrimaryCursor );
        }
      }

      if ( mActiveCursor == ACTIVE_CURSOR_BOTH )
      {
        if ( !mSecondaryCursor )
        {
          CreateCursor( mSecondaryCursor, mCursor[SECONDARY_CURSOR].color );
#ifdef DECORATOR_DEBUG
          mSecondaryCursor.SetName( "SecondaryCursorActor" );
#endif
        }

        if( !mSecondaryCursor.GetParent() )
        {
          mActiveLayer.Add( mSecondaryCursor );
        }
      }
      else
      {
        if( mSecondaryCursor )
        {
          mSecondaryCursor.Unparent();
        }
      }
    }
  }

  bool OnCursorBlinkTimerTick()
  {
    if( !mDelayCursorBlink )
    {
      // Cursor blinking
      if ( mPrimaryCursor )
      {
        mPrimaryCursor.SetVisible( mPrimaryCursorVisible && mCursorBlinkStatus );
      }
      if ( mSecondaryCursor )
      {
        mSecondaryCursor.SetVisible( mSecondaryCursorVisible && mCursorBlinkStatus );
      }

      mCursorBlinkStatus = !mCursorBlinkStatus;
    }
    else
    {
      // Resume blinking
      mDelayCursorBlink = false;
    }

    return true;
  }

  void SetupTouchEvents()
  {
    mTapDetector = TapGestureDetector::New();
    mTapDetector.DetectedSignal().Connect( this, &Decorator::Impl::OnTap );

    mPanGestureDetector = PanGestureDetector::New();
    mPanGestureDetector.DetectedSignal().Connect( this, &Decorator::Impl::OnPan );
  }

  void CreateActiveLayer()
  {
    if( !mActiveLayer )
    {
      mActiveLayer = Layer::New();
#ifdef DECORATOR_DEBUG
      mActiveLayer.SetName ( "ActiveLayerActor" );
#endif

      mActiveLayer.SetParentOrigin( ParentOrigin::CENTER );
      mActiveLayer.SetResizePolicy( ResizePolicy::FILL_TO_PARENT, Dimension::ALL_DIMENSIONS );
      mActiveLayer.SetPositionInheritanceMode( USE_PARENT_POSITION );

      // Add the active layer telling the controller it doesn't need clipping.
      mController.AddDecoration( mActiveLayer, false );
    }

    mActiveLayer.RaiseToTop();
  }

  void SetSelectionHandleMarkerSize( HandleImpl& handle )
  {
    if( handle.markerActor )
    {
      handle.markerActor.SetSize( 0, handle.lineHeight );
    }
  }

  void CreateGrabHandle()
  {
    HandleImpl& grabHandle = mHandle[GRAB_HANDLE];
    if( !grabHandle.actor )
    {
      grabHandle.actor = ImageActor::New( mHandleImages[GRAB_HANDLE][HANDLE_IMAGE_RELEASED] );
      grabHandle.actor.SetSortModifier( DECORATION_DEPTH_INDEX );
      grabHandle.actor.SetAnchorPoint( AnchorPoint::TOP_CENTER );
      // Area that Grab handle responds to, larger than actual handle so easier to move
#ifdef DECORATOR_DEBUG
      grabHandle.actor.SetName( "GrabHandleActor" );
      if ( Dali::Internal::gLogFilter->IsEnabledFor( Debug::Verbose ) )
      {
        grabHandle.grabArea = Toolkit::CreateSolidColorActor( Vector4(0.0f, 0.0f, 0.0f, 0.0f), true, Color::RED, 1 );
        grabHandle.grabArea.SetName( "GrabArea" );
      }
      else
      {
        grabHandle.grabArea = Actor::New();
        grabHandle.grabArea.SetName( "GrabArea" );
      }
#else
      grabHandle.grabArea = Actor::New();
#endif

      grabHandle.grabArea.SetParentOrigin( ParentOrigin::TOP_CENTER );
      grabHandle.grabArea.SetAnchorPoint( AnchorPoint::TOP_CENTER );
      grabHandle.grabArea.SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::ALL_DIMENSIONS );
      grabHandle.grabArea.SetSizeModeFactor( DEFAULT_GRAB_HANDLE_RELATIVE_SIZE );
      grabHandle.actor.Add( grabHandle.grabArea );
      grabHandle.actor.SetColor( mHandleColor );

      grabHandle.grabArea.TouchedSignal().Connect( this, &Decorator::Impl::OnGrabHandleTouched );
      mTapDetector.Attach( grabHandle.grabArea );
      mPanGestureDetector.Attach( grabHandle.grabArea );

      mActiveLayer.Add( grabHandle.actor );
    }

    if( !grabHandle.actor.GetParent() )
    {
      mActiveLayer.Add( grabHandle.actor );
    }
  }

  void CreateHandleMarker( HandleImpl& handle, Image& image, HandleType handleType )
  {
    if( image )
    {
      handle.markerActor = ImageActor::New( image );
      handle.markerActor.SetColor( mHandleColor );
      handle.actor.Add( handle.markerActor );

      handle.markerActor.SetResizePolicy ( ResizePolicy::FIXED, Dimension::HEIGHT );

      if( LEFT_SELECTION_HANDLE == handleType )
      {
        handle.markerActor.SetAnchorPoint( AnchorPoint::BOTTOM_RIGHT );
        handle.markerActor.SetParentOrigin( ParentOrigin::TOP_RIGHT );
      }
      else if( RIGHT_SELECTION_HANDLE == handleType )
      {
        handle.markerActor.SetAnchorPoint( AnchorPoint::BOTTOM_LEFT );
        handle.markerActor.SetParentOrigin( ParentOrigin::TOP_LEFT );
      }
    }
  }

  void CreateSelectionHandles()
  {
    HandleImpl& primary = mHandle[ LEFT_SELECTION_HANDLE ];
    if( !primary.actor )
    {
      primary.actor = ImageActor::New( mHandleImages[LEFT_SELECTION_HANDLE][HANDLE_IMAGE_RELEASED] );
#ifdef DECORATOR_DEBUG
      primary.actor.SetName("SelectionHandleOne");
#endif
      primary.actor.SetAnchorPoint( AnchorPoint::TOP_RIGHT ); // Change to BOTTOM_RIGHT if Look'n'Feel requires handle above text.
      primary.actor.SetSortModifier( DECORATION_DEPTH_INDEX );
      primary.actor.SetColor( mHandleColor );

      primary.grabArea = Actor::New(); // Area that Grab handle responds to, larger than actual handle so easier to move
#ifdef DECORATOR_DEBUG
      primary.grabArea.SetName("SelectionHandleOneGrabArea");
#endif
      primary.grabArea.SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::ALL_DIMENSIONS );
      primary.grabArea.SetParentOrigin( ParentOrigin::TOP_CENTER );
      primary.grabArea.SetAnchorPoint( AnchorPoint::TOP_CENTER );
      primary.grabArea.SetSizeModeFactor( DEFAULT_SELECTION_HANDLE_RELATIVE_SIZE );

      mTapDetector.Attach( primary.grabArea );
      mPanGestureDetector.Attach( primary.grabArea );
      primary.grabArea.TouchedSignal().Connect( this, &Decorator::Impl::OnHandleOneTouched );

      primary.actor.Add( primary.grabArea );

      CreateHandleMarker( primary, mHandleImages[LEFT_SELECTION_HANDLE_MARKER][HANDLE_IMAGE_RELEASED], LEFT_SELECTION_HANDLE );
    }

    if( !primary.actor.GetParent() )
    {
      mActiveLayer.Add( primary.actor );
    }

    HandleImpl& secondary = mHandle[ RIGHT_SELECTION_HANDLE ];
    if( !secondary.actor )
    {
      secondary.actor = ImageActor::New( mHandleImages[RIGHT_SELECTION_HANDLE][HANDLE_IMAGE_RELEASED] );
#ifdef DECORATOR_DEBUG
      secondary.actor.SetName("SelectionHandleTwo");
#endif
      secondary.actor.SetAnchorPoint( AnchorPoint::TOP_LEFT ); // Change to BOTTOM_LEFT if Look'n'Feel requires handle above text.
      secondary.actor.SetSortModifier( DECORATION_DEPTH_INDEX );
      secondary.actor.SetColor( mHandleColor );

      secondary.grabArea = Actor::New(); // Area that Grab handle responds to, larger than actual handle so easier to move
#ifdef DECORATOR_DEBUG
      secondary.grabArea.SetName("SelectionHandleTwoGrabArea");
#endif
      secondary.grabArea.SetResizePolicy( ResizePolicy::SIZE_RELATIVE_TO_PARENT, Dimension::ALL_DIMENSIONS );
      secondary.grabArea.SetParentOrigin( ParentOrigin::TOP_CENTER );
      secondary.grabArea.SetAnchorPoint( AnchorPoint::TOP_CENTER );
      secondary.grabArea.SetSizeModeFactor( DEFAULT_SELECTION_HANDLE_RELATIVE_SIZE );

      mTapDetector.Attach( secondary.grabArea );
      mPanGestureDetector.Attach( secondary.grabArea );
      secondary.grabArea.TouchedSignal().Connect( this, &Decorator::Impl::OnHandleTwoTouched );

      secondary.actor.Add( secondary.grabArea );

      CreateHandleMarker( secondary, mHandleImages[RIGHT_SELECTION_HANDLE_MARKER][HANDLE_IMAGE_RELEASED], RIGHT_SELECTION_HANDLE  );
    }

    if( !secondary.actor.GetParent() )
    {
      mActiveLayer.Add( secondary.actor );
    }
  }

  void CalculateHandleWorldCoordinates( HandleImpl& handle, Vector2& position )
  {
    // Gets the world position of the active layer. The active layer is where the handles are added.
    const Vector3 parentWorldPosition = mActiveLayer.GetCurrentWorldPosition();

    // The grab handle position in world coords.
    // The active layer's world position is the center of the active layer. The origin of the
    // coord system of the handles is the top left of the active layer.
    position.x = parentWorldPosition.x - 0.5f * mControlSize.width + handle.position.x;
    position.y = parentWorldPosition.y - 0.5f * mControlSize.height + handle.position.y;
  }

  void SetGrabHandlePosition()
  {
    // Reference to the grab handle.
    HandleImpl& grabHandle = mHandle[GRAB_HANDLE];

    // Transforms the handle position into world coordinates.
    // @note This is not the same value as grabHandle.actor.GetCurrentWorldPosition()
    // as it's transforming the handle's position set by the text-controller and not
    // the final position set to the actor. Another difference is the GetCurrentWorldPosition()
    // retrieves the position of the center of the actor but the handle's position set
    // by the text controller is not the center of the actor.
    Vector2 grabHandleWorldPosition;
    CalculateHandleWorldCoordinates( grabHandle, grabHandleWorldPosition );

    // Check if the grab handle exceeds the boundaries of the decoration box.
    // At the moment only the height is checked for the grab handle.

    grabHandle.verticallyFlipped = ( grabHandle.verticallyFlippedPreferred &&
                                     ( ( grabHandleWorldPosition.y - grabHandle.size.height ) > mBoundingBox.y ) ) ||
                                   ( grabHandleWorldPosition.y + grabHandle.lineHeight + grabHandle.size.height > mBoundingBox.w );

    // The grab handle 'y' position in local coords.
    // If the grab handle exceeds the bottom of the decoration box,
    // set the 'y' position to the top of the line.
    // The SetGrabHandleImage() method will change the orientation.
    const float yLocalPosition = grabHandle.verticallyFlipped ? grabHandle.position.y : grabHandle.position.y + grabHandle.lineHeight;

    grabHandle.actor.SetPosition( grabHandle.position.x + floor( 0.5f * mCursorWidth ),
                                  yLocalPosition ); // TODO : Fix for multiline.
  }

  void SetSelectionHandlePosition( HandleType type )
  {
    const bool isPrimaryHandle = LEFT_SELECTION_HANDLE == type;

    // Reference to the selection handle.
    HandleImpl& handle = mHandle[type];

    // Transforms the handle position into world coordinates.
    // @note This is not the same value as handle.actor.GetCurrentWorldPosition()
    // as it's transforming the handle's position set by the text-controller and not
    // the final position set to the actor. Another difference is the GetCurrentWorldPosition()
    // retrieves the position of the center of the actor but the handle's position set
    // by the text controller is not the center of the actor.
    Vector2 handleWorldPosition;
    CalculateHandleWorldCoordinates( handle, handleWorldPosition );

    // Whether to flip the handle (horizontally).
    bool flipHandle = isPrimaryHandle ? mFlipLeftSelectionHandleDirection : mFlipRightSelectionHandleDirection;

    // Whether to flip the handles if they are crossed.
    bool crossFlip = false;
    if( mFlipSelectionHandlesOnCross || !mHandlePanning )
    {
      crossFlip = mHandleCurrentCrossed;
    }

    // Does not flip if both conditions are true (double flip)
    flipHandle = flipHandle != ( crossFlip || mHandlePreviousCrossed );

    // Will flip the handles vertically if the user prefers it.
    bool verticallyFlippedPreferred = handle.verticallyFlippedPreferred;

    if( crossFlip || mHandlePreviousCrossed )
    {
      if( isPrimaryHandle )
      {
        verticallyFlippedPreferred = mHandle[RIGHT_SELECTION_HANDLE].verticallyFlippedPreferred;
      }
      else
      {
        verticallyFlippedPreferred = mHandle[LEFT_SELECTION_HANDLE].verticallyFlippedPreferred;
      }
    }

    // Check if the selection handle exceeds the boundaries of the decoration box.
    const bool exceedsLeftEdge = ( isPrimaryHandle ? !flipHandle : flipHandle ) && ( handleWorldPosition.x - handle.size.width < mBoundingBox.x );
    const bool exceedsRightEdge = ( isPrimaryHandle ? flipHandle : !flipHandle ) && ( handleWorldPosition.x + handle.size.width > mBoundingBox.z );

    // Does not flip if both conditions are true (double flip)
    flipHandle = flipHandle != ( exceedsLeftEdge || exceedsRightEdge );

    if( flipHandle )
    {
      if( !handle.horizontallyFlipped )
      {
        // Change the anchor point to flip the image.
        handle.actor.SetAnchorPoint( isPrimaryHandle ? AnchorPoint::TOP_LEFT : AnchorPoint::TOP_RIGHT );

        handle.horizontallyFlipped = true;
      }
    }
    else
    {
      if( handle.horizontallyFlipped )
      {
        // Reset the anchor point.
        handle.actor.SetAnchorPoint( isPrimaryHandle ? AnchorPoint::TOP_RIGHT : AnchorPoint::TOP_LEFT );

        handle.horizontallyFlipped = false;
      }
    }

    // Whether to flip the handle vertically.
    handle.verticallyFlipped = ( verticallyFlippedPreferred &&
                                 ( ( handleWorldPosition.y - handle.size.height ) > mBoundingBox.y ) ) ||
                               ( handleWorldPosition.y + handle.lineHeight + handle.size.height > mBoundingBox.w );

    // The primary selection handle 'y' position in local coords.
    // If the handle exceeds the bottom of the decoration box,
    // set the 'y' position to the top of the line.
    // The SetHandleImage() method will change the orientation.
    const float yLocalPosition = handle.verticallyFlipped ? handle.position.y : handle.position.y + handle.lineHeight;

    handle.actor.SetPosition( handle.position.x,
                              yLocalPosition ); // TODO : Fix for multiline.
  }

  void SetHandleImage( HandleType type )
  {
    HandleImpl& handle = mHandle[type];

    HandleType markerType = HANDLE_TYPE_COUNT;
    // If the selection handle is flipped it chooses the image of the other selection handle. Does nothing for the grab handle.
    if( LEFT_SELECTION_HANDLE == type )
    {
      type = handle.horizontallyFlipped ? RIGHT_SELECTION_HANDLE : LEFT_SELECTION_HANDLE;
      markerType = handle.horizontallyFlipped ? RIGHT_SELECTION_HANDLE_MARKER : LEFT_SELECTION_HANDLE_MARKER;
    }
    else if( RIGHT_SELECTION_HANDLE == type )
    {
      type = handle.horizontallyFlipped ? LEFT_SELECTION_HANDLE : RIGHT_SELECTION_HANDLE;
      markerType = handle.horizontallyFlipped ? LEFT_SELECTION_HANDLE_MARKER : RIGHT_SELECTION_HANDLE_MARKER;
    }

    // Chooses between the released or pressed image. It checks whether the pressed image exists.
    if( handle.actor )
    {
      const HandleImageType imageType = ( handle.pressed ? ( mHandleImages[type][HANDLE_IMAGE_PRESSED] ? HANDLE_IMAGE_PRESSED : HANDLE_IMAGE_RELEASED ) : HANDLE_IMAGE_RELEASED );

      handle.actor.SetImage( mHandleImages[type][imageType] );
    }

    if( HANDLE_TYPE_COUNT != markerType )
    {
      if( handle.markerActor )
      {
        const HandleImageType markerImageType = ( handle.pressed ? ( mHandleImages[markerType][HANDLE_IMAGE_PRESSED] ? HANDLE_IMAGE_PRESSED : HANDLE_IMAGE_RELEASED ) : HANDLE_IMAGE_RELEASED );
        handle.markerActor.SetImage( mHandleImages[markerType][markerImageType] );
      }
    }

    // Whether to flip the handle vertically.
    if( handle.actor )
    {
      handle.actor.SetOrientation( handle.verticallyFlipped ? ANGLE_180 : ANGLE_0, Vector3::XAXIS );
    }
  }

  void CreateHighlight()
  {
    if( !mHighlightActor )
    {
      mHighlightActor = Actor::New();

#ifdef DECORATOR_DEBUG
      mHighlightActor.SetName( "HighlightActor" );
#endif
      mHighlightActor.SetAnchorPoint( AnchorPoint::TOP_LEFT );
      mHighlightActor.SetSize( 1.0f, 1.0f );
      mHighlightActor.SetColor( mHighlightColor );
      mHighlightActor.SetColorMode( USE_OWN_COLOR );
    }

    // Add the highlight box telling the controller it needs clipping.
    mController.AddDecoration( mHighlightActor, true );
  }

  void UpdateHighlight()
  {
    if ( mHighlightActor )
    {
      if( !mHighlightQuadList.empty() )
      {
        Vector< Vector2 > vertices;
        Vector< unsigned int> indices;
        Vector2 vertex;

        std::vector<QuadCoordinates>::iterator iter = mHighlightQuadList.begin();
        std::vector<QuadCoordinates>::iterator endIter = mHighlightQuadList.end();

        for( std::size_t v = 0; iter != endIter; ++iter,v+=4 )
        {
          QuadCoordinates& quad = *iter;

          // top-left (v+0)
          vertex.x = quad.min.x;
          vertex.y = quad.min.y;
          vertices.PushBack( vertex );

          // top-right (v+1)
          vertex.x = quad.max.x;
          vertex.y = quad.min.y;
          vertices.PushBack( vertex );

          // bottom-left (v+2)
          vertex.x = quad.min.x;
          vertex.y = quad.max.y;
          vertices.PushBack( vertex );

          // bottom-right (v+3)
          vertex.x = quad.max.x;
          vertex.y = quad.max.y;
          vertices.PushBack( vertex );

          // triangle A (3, 1, 0)
          indices.PushBack( v + 3 );
          indices.PushBack( v + 1 );
          indices.PushBack( v );

          // triangle B (0, 2, 3)
          indices.PushBack( v );
          indices.PushBack( v + 2 );
          indices.PushBack( v + 3 );
        }

        if( mQuadVertices )
        {
          mQuadVertices.SetSize( vertices.Size() );
        }
        else
        {
          mQuadVertices = PropertyBuffer::New( mQuadVertexFormat, vertices.Size() );
        }

        if( mQuadIndices )
        {
          mQuadIndices.SetSize( indices.Size() );
        }
        else
        {
          mQuadIndices = PropertyBuffer::New( mQuadIndexFormat, indices.Size() );
        }

        mQuadVertices.SetData( &vertices[ 0 ] );
        mQuadIndices.SetData( &indices[ 0 ] );

        if( !mQuadGeometry )
        {
          mQuadGeometry = Geometry::New();
          mQuadGeometry.AddVertexBuffer( mQuadVertices );
        }
        mQuadGeometry.SetIndexBuffer( mQuadIndices );

        if( !mHighlightRenderer )
        {
          mHighlightRenderer = Dali::Renderer::New( mQuadGeometry, mHighlightMaterial );
          mHighlightActor.AddRenderer( mHighlightRenderer );
        }
      }

      mHighlightActor.SetPosition( mHighlightPosition.x,
                                   mHighlightPosition.y );

      mHighlightQuadList.clear();

      if( mHighlightRenderer )
      {
        mHighlightRenderer.SetDepthIndex( mTextDepth - 2u ); // text is rendered at mTextDepth and text's shadow at mTextDepth -1u.
      }
    }
  }

  void OnTap( Actor actor, const TapGesture& tap )
  {
    if( actor == mHandle[GRAB_HANDLE].actor )
    {
      // TODO
    }
  }

  void DoPan( HandleImpl& handle, HandleType type, const PanGesture& gesture )
  {
    if( Gesture::Started == gesture.state )
    {
      handle.grabDisplacementX = handle.grabDisplacementY = 0;
    }

    handle.grabDisplacementX += gesture.displacement.x;
    handle.grabDisplacementY += gesture.displacement.y;

    const float x = handle.position.x + handle.grabDisplacementX;
    const float y = handle.position.y + handle.lineHeight*0.5f + handle.grabDisplacementY;

    if( Gesture::Started    == gesture.state ||
        Gesture::Continuing == gesture.state )
    {
      Vector2 targetSize;
      mController.GetTargetSize( targetSize );

      if( x < mScrollThreshold )
      {
        mScrollDirection = SCROLL_RIGHT;
        mHandleScrolling = type;
        StartScrollTimer();
      }
      else if( x > targetSize.width - mScrollThreshold )
      {
        mScrollDirection = SCROLL_LEFT;
        mHandleScrolling = type;
        StartScrollTimer();
      }
      else
      {
        mHandleScrolling = HANDLE_TYPE_COUNT;
        StopScrollTimer();
        mController.DecorationEvent( type, HANDLE_PRESSED, x, y );
      }

      mHandlePanning = true;
    }
    else if( Gesture::Finished  == gesture.state ||
             Gesture::Cancelled == gesture.state )
    {
      if( mScrollTimer &&
          ( mScrollTimer.IsRunning() || mNotifyEndOfScroll ) )
      {
        mNotifyEndOfScroll = false;
        mHandleScrolling = HANDLE_TYPE_COUNT;
        StopScrollTimer();
        mController.DecorationEvent( type, HANDLE_STOP_SCROLLING, x, y );
      }
      else
      {
        mController.DecorationEvent( type, HANDLE_RELEASED, x, y );
      }

      handle.actor.SetImage( mHandleImages[type][HANDLE_IMAGE_RELEASED] );
      handle.pressed = false;

      mHandlePanning = false;
    }
  }

  void OnPan( Actor actor, const PanGesture& gesture )
  {
    HandleImpl& grabHandle = mHandle[GRAB_HANDLE];
    HandleImpl& primarySelectionHandle = mHandle[LEFT_SELECTION_HANDLE];
    HandleImpl& secondarySelectionHandle = mHandle[RIGHT_SELECTION_HANDLE];

    if( actor == grabHandle.grabArea )
    {
      DoPan( grabHandle, GRAB_HANDLE, gesture );
    }
    else if( actor == primarySelectionHandle.grabArea )
    {
      DoPan( primarySelectionHandle, LEFT_SELECTION_HANDLE, gesture );
    }
    else if( actor == secondarySelectionHandle.grabArea )
    {
      DoPan( secondarySelectionHandle, RIGHT_SELECTION_HANDLE, gesture );
    }
  }

  bool OnGrabHandleTouched( Actor actor, const TouchEvent& event )
  {
    // Switch between pressed/release grab-handle images
    if( event.GetPointCount() > 0 &&
        mHandle[GRAB_HANDLE].actor )
    {
      const TouchPoint& point = event.GetPoint(0);

      if( TouchPoint::Down == point.state )
      {
        mHandle[GRAB_HANDLE].pressed = true;
      }
      else if( ( TouchPoint::Up == point.state ) ||
               ( TouchPoint::Interrupted == point.state ) )
      {
        mHandle[GRAB_HANDLE].pressed = false;
      }

      SetHandleImage( GRAB_HANDLE );
    }

    // Consume to avoid pop-ups accidentally closing, when handle is outside of pop-up area
    return true;
  }

  bool OnHandleOneTouched( Actor actor, const TouchEvent& event )
  {
    // Switch between pressed/release selection handle images
    if( event.GetPointCount() > 0 &&
        mHandle[LEFT_SELECTION_HANDLE].actor )
    {
      const TouchPoint& point = event.GetPoint(0);

      if( TouchPoint::Down == point.state )
      {
        mHandle[LEFT_SELECTION_HANDLE].pressed = true;
      }
      else if( ( TouchPoint::Up == point.state ) ||
               ( TouchPoint::Interrupted == point.state ) )
      {
        mHandle[LEFT_SELECTION_HANDLE].pressed = false;
        mHandlePreviousCrossed = mHandleCurrentCrossed;
        mHandlePanning = false;
      }

      SetHandleImage( LEFT_SELECTION_HANDLE );
    }

    // Consume to avoid pop-ups accidentally closing, when handle is outside of pop-up area
    return true;
  }

  bool OnHandleTwoTouched( Actor actor, const TouchEvent& event )
  {
    // Switch between pressed/release selection handle images
    if( event.GetPointCount() > 0 &&
        mHandle[RIGHT_SELECTION_HANDLE].actor )
    {
      const TouchPoint& point = event.GetPoint(0);

      if( TouchPoint::Down == point.state )
      {
        mHandle[RIGHT_SELECTION_HANDLE].pressed = true;
      }
      else if( ( TouchPoint::Up == point.state ) ||
               ( TouchPoint::Interrupted == point.state ) )
      {
        mHandle[RIGHT_SELECTION_HANDLE].pressed = false;
        mHandlePreviousCrossed = mHandleCurrentCrossed;
        mHandlePanning = false;
      }

      SetHandleImage( RIGHT_SELECTION_HANDLE );
    }

    // Consume to avoid pop-ups accidentally closing, when handle is outside of pop-up area
    return true;
  }

  void HandleResetPosition( PropertyNotification& source )
  {
    const HandleImpl& grabHandle = mHandle[GRAB_HANDLE];

    if( grabHandle.active )
    {
      // Sets the grab handle position and calculates if it needs to be vertically flipped if it exceeds the boundary box.
      SetGrabHandlePosition();

      // Sets the grab handle image according if it's pressed, flipped, etc.
      SetHandleImage( GRAB_HANDLE );
    }
    else
    {
      // Sets the primary selection handle position and calculates if it needs to be vertically flipped if it exceeds the boundary box.
      SetSelectionHandlePosition( LEFT_SELECTION_HANDLE );

      // Sets the primary handle image according if it's pressed, flipped, etc.
      SetHandleImage( LEFT_SELECTION_HANDLE );

      // Sets the secondary selection handle position and calculates if it needs to be vertically flipped if it exceeds the boundary box.
      SetSelectionHandlePosition( RIGHT_SELECTION_HANDLE );

      // Sets the secondary handle image according if it's pressed, flipped, etc.
      SetHandleImage( RIGHT_SELECTION_HANDLE );
    }
  }

  void SetupActiveLayerPropertyNotifications()
  {
    if( !mActiveLayer )
    {
      return;
    }

    // Vertical notifications.

    // Disconnect any previous connected callback.
    if( mVerticalLessThanNotification )
    {
      mVerticalLessThanNotification.NotifySignal().Disconnect( this, &Decorator::Impl::HandleResetPosition );
      mActiveLayer.RemovePropertyNotification( mVerticalLessThanNotification );
    }

    if( mVerticalGreaterThanNotification )
    {
      mVerticalGreaterThanNotification.NotifySignal().Disconnect( this, &Decorator::Impl::HandleResetPosition );
      mActiveLayer.RemovePropertyNotification( mVerticalGreaterThanNotification );
    }

    const HandleImpl& grabHandle = mHandle[GRAB_HANDLE];
    const HandleImpl& primaryHandle = mHandle[LEFT_SELECTION_HANDLE];
    const HandleImpl& secondaryHandle = mHandle[RIGHT_SELECTION_HANDLE];

    if( grabHandle.active )
    {
      if( grabHandle.verticallyFlipped )
      {
        // The grab handle is vertically flipped. Never is going to exceed the bottom edje of the display.
        mVerticalGreaterThanNotification.Reset();

        // The vertical distance from the center of the active layer to the top edje of the display.
        const float topHeight = 0.5f * mControlSize.height - grabHandle.position.y + grabHandle.size.height;

        mVerticalLessThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                              LessThanCondition( mBoundingBox.y + topHeight ) );

        // Notifies the change from false to true and from true to false.
        mVerticalLessThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

        // Connects the signals with the callbacks.
        mVerticalLessThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );
      }
      else
      {
        // The grab handle is not vertically flipped. Never is going to exceed the top edje of the display.
        mVerticalLessThanNotification.Reset();

        // The vertical distance from the center of the active layer to the bottom edje of the display.
        const float bottomHeight = -0.5f * mControlSize.height + grabHandle.position.y + grabHandle.lineHeight + grabHandle.size.height;

        mVerticalGreaterThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                                 GreaterThanCondition( mBoundingBox.w - bottomHeight ) );

        // Notifies the change from false to true and from true to false.
        mVerticalGreaterThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

        // Connects the signals with the callbacks.
        mVerticalGreaterThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );
      }
    }
    else // The selection handles are active
    {
      if( primaryHandle.verticallyFlipped && secondaryHandle.verticallyFlipped )
      {
        // Both selection handles are vertically flipped. Never are going to exceed the bottom edje of the display.
        mVerticalGreaterThanNotification.Reset();

        // The vertical distance from the center of the active layer to the top edje of the display.
        const float topHeight = 0.5f * mControlSize.height + std::max( -primaryHandle.position.y + primaryHandle.size.height, -secondaryHandle.position.y + secondaryHandle.size.height );

        mVerticalLessThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                              LessThanCondition( mBoundingBox.y + topHeight ) );

        // Notifies the change from false to true and from true to false.
        mVerticalLessThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

        // Connects the signals with the callbacks.
        mVerticalLessThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );
      }
      else if( !primaryHandle.verticallyFlipped && !secondaryHandle.verticallyFlipped )
      {
        // Both selection handles aren't vertically flipped. Never are going to exceed the top edje of the display.
        mVerticalLessThanNotification.Reset();

        // The vertical distance from the center of the active layer to the bottom edje of the display.
        const float bottomHeight = -0.5f * mControlSize.height + std::max( primaryHandle.position.y + primaryHandle.lineHeight + primaryHandle.size.height,
                                                                           secondaryHandle.position.y + secondaryHandle.lineHeight + secondaryHandle.size.height );

        mVerticalGreaterThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                                 GreaterThanCondition( mBoundingBox.w - bottomHeight ) );

        // Notifies the change from false to true and from true to false.
        mVerticalGreaterThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

        // Connects the signals with the callbacks.
        mVerticalGreaterThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );
      }
      else
      {
        // Only one of the selection handles is vertically flipped. Both vertical notifications are needed.

        // The vertical distance from the center of the active layer to the top edje of the display.
        const float topHeight = 0.5f * mControlSize.height + ( primaryHandle.verticallyFlipped                              ?
                                                               -primaryHandle.position.y + primaryHandle.size.height        :
                                                               -secondaryHandle.position.y + secondaryHandle.size.height );

        mVerticalLessThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                              LessThanCondition( mBoundingBox.y + topHeight ) );

        // Notifies the change from false to true and from true to false.
        mVerticalLessThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

        // Connects the signals with the callbacks.
        mVerticalLessThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );

        // The vertical distance from the center of the active layer to the bottom edje of the display.
        const float bottomHeight = -0.5f * mControlSize.height + ( primaryHandle.verticallyFlipped                                                       ?
                                                                   secondaryHandle.position.y + secondaryHandle.lineHeight + secondaryHandle.size.height :
                                                                   primaryHandle.position.y + primaryHandle.lineHeight + primaryHandle.size.height );

        mVerticalGreaterThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                                 GreaterThanCondition( mBoundingBox.w - bottomHeight ) );

        // Notifies the change from false to true and from true to false.
        mVerticalGreaterThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

        // Connects the signals with the callbacks.
        mVerticalGreaterThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );
      }
    }

    // Horizontal notifications.

    // Disconnect any previous connected callback.
    if( mHorizontalLessThanNotification )
    {
      mHorizontalLessThanNotification.NotifySignal().Disconnect( this, &Decorator::Impl::HandleResetPosition );
      mActiveLayer.RemovePropertyNotification( mHorizontalLessThanNotification );
    }

    if( mHorizontalGreaterThanNotification )
    {
      mHorizontalGreaterThanNotification.NotifySignal().Disconnect( this, &Decorator::Impl::HandleResetPosition );
      mActiveLayer.RemovePropertyNotification( mHorizontalGreaterThanNotification );
    }

    if( primaryHandle.active || secondaryHandle.active )
    {
      // The horizontal distance from the center of the active layer to the left edje of the display.
      const float leftWidth = 0.5f * mControlSize.width + std::max( -primaryHandle.position.x + primaryHandle.size.width,
                                                                    -secondaryHandle.position.x + secondaryHandle.size.width );

      mHorizontalLessThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_X,
                                                                              LessThanCondition( mBoundingBox.x + leftWidth ) );

      // Notifies the change from false to true and from true to false.
      mHorizontalLessThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

      // Connects the signals with the callbacks.
      mHorizontalLessThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );

      // The horizontal distance from the center of the active layer to the right edje of the display.
      const float rightWidth = -0.5f * mControlSize.width + std::max( primaryHandle.position.x + primaryHandle.size.width,
                                                                      secondaryHandle.position.x + secondaryHandle.size.width );

      mHorizontalGreaterThanNotification = mActiveLayer.AddPropertyNotification( Actor::Property::WORLD_POSITION_X,
                                                                                 GreaterThanCondition( mBoundingBox.z - rightWidth ) );

      // Notifies the change from false to true and from true to false.
      mHorizontalGreaterThanNotification.SetNotifyMode( PropertyNotification::NotifyOnChanged );

      // Connects the signals with the callbacks.
      mHorizontalGreaterThanNotification.NotifySignal().Connect( this, &Decorator::Impl::HandleResetPosition );
    }
  }

  // Popup

  float AlternatePopUpPositionRelativeToCursor()
  {
    float alternativePosition = 0.0f;

    const float popupHeight = mCopyPastePopup.actor.GetRelayoutSize( Dimension::HEIGHT );

    const HandleImpl& primaryHandle = mHandle[LEFT_SELECTION_HANDLE];
    const HandleImpl& secondaryHandle = mHandle[RIGHT_SELECTION_HANDLE];
    const HandleImpl& grabHandle = mHandle[GRAB_HANDLE];
    const CursorImpl& cursor = mCursor[PRIMARY_CURSOR];

    if( primaryHandle.active || secondaryHandle.active )
    {
      const float maxHandleHeight = std::max( primaryHandle.size.height, secondaryHandle.size.height );
      alternativePosition = 0.5f * popupHeight + cursor.lineHeight + maxHandleHeight + std::min( primaryHandle.position.y, secondaryHandle.position.y );
    }
    else
    {
      alternativePosition = 0.5f * popupHeight + cursor.lineHeight + grabHandle.size.height + cursor.position.y;
    }

    return alternativePosition;
  }

  void PopUpLeavesVerticalBoundary( PropertyNotification& source )
  {
    float alternativeYPosition = 0.0f;
    // todo use AlternatePopUpPositionRelativeToSelectionHandles() if text is highlighted
    // if can't be positioned above, then position below row.
    alternativeYPosition = AlternatePopUpPositionRelativeToCursor();

    mCopyPastePopup.actor.SetY( alternativeYPosition );
  }

  void SetUpPopupPositionNotifications()
  {
    // Note Property notifications ignore any set anchor point so conditions must allow for this.  Default is Top Left.

    // Exceeding vertical boundary

    const float popupHeight = mCopyPastePopup.actor.GetRelayoutSize( Dimension::HEIGHT );

    PropertyNotification verticalExceedNotification = mCopyPastePopup.actor.AddPropertyNotification( Actor::Property::WORLD_POSITION_Y,
                                                                                                     OutsideCondition( mBoundingBox.y + popupHeight * 0.5f,
                                                                                                                       mBoundingBox.w - popupHeight * 0.5f ) );

    verticalExceedNotification.NotifySignal().Connect( this, &Decorator::Impl::PopUpLeavesVerticalBoundary );
  }

  void GetConstrainedPopupPosition( Vector3& requiredPopupPosition, const Vector3& popupDistanceFromAnchorPoint, Actor parent, const Vector4& boundingRectangleWorld )
  {
    DALI_ASSERT_DEBUG ( "Popup parent not on stage" && parent.OnStage() )

    // Parent must already by added to Stage for these Get calls to work
    const Vector3 parentWorldPositionLeftAnchor = parent.GetCurrentWorldPosition() - parent.GetCurrentSize() * parent.GetCurrentAnchorPoint();
    const Vector3 popupWorldPosition = parentWorldPositionLeftAnchor + requiredPopupPosition;  // Parent World position plus popup local position gives World Position

    // Calculate distance to move popup (in local space) so fits within the boundary
    float xOffSetToKeepWithinBounds = 0.0f;
    if( popupWorldPosition.x - popupDistanceFromAnchorPoint.x < boundingRectangleWorld.x )
    {
      xOffSetToKeepWithinBounds = boundingRectangleWorld.x - ( popupWorldPosition.x - popupDistanceFromAnchorPoint.x );
    }
    else if( popupWorldPosition.x +  popupDistanceFromAnchorPoint.x > boundingRectangleWorld.z )
    {
      xOffSetToKeepWithinBounds = boundingRectangleWorld.z - ( popupWorldPosition.x +  popupDistanceFromAnchorPoint.x );
    }

    // Ensure initial display of Popup is in alternative position if can not fit above. As Property notification will be a frame behind.
    if( popupWorldPosition.y - popupDistanceFromAnchorPoint.y < boundingRectangleWorld.y )
    {
      requiredPopupPosition.y = AlternatePopUpPositionRelativeToCursor();
    }

    requiredPopupPosition.x = requiredPopupPosition.x + xOffSetToKeepWithinBounds;

    // Prevent pixel mis-alignment by rounding down.
    requiredPopupPosition.x = floor( requiredPopupPosition.x );
    requiredPopupPosition.y = floor( requiredPopupPosition.y );
  }

  void SetHandleImage( HandleType handleType, HandleImageType handleImageType, Dali::Image image )
  {
    HandleImpl& handle = mHandle[handleType];
    handle.size = Size( image.GetWidth(), image.GetHeight() );

    mHandleImages[handleType][handleImageType] = image;
  }

  void SetScrollThreshold( float threshold )
  {
    mScrollThreshold = threshold;
  }

  float GetScrollThreshold() const
  {
    return mScrollThreshold;
  }

  void SetScrollSpeed( float speed )
  {
    mScrollSpeed = speed;
    mScrollDistance = speed * SCROLL_TICK_INTERVAL * TO_SECONDS;
  }

  float GetScrollSpeed() const
  {
    return mScrollSpeed;
  }

  void NotifyEndOfScroll()
  {
    StopScrollTimer();

    if( mScrollTimer )
    {
      mNotifyEndOfScroll = true;
    }
  }

  /**
   * Creates and starts a timer to scroll the text when handles are close to the edges of the text.
   *
   * It only starts the timer if it's already created.
   */
  void StartScrollTimer()
  {
    if( !mScrollTimer )
    {
      mScrollTimer = Timer::New( SCROLL_TICK_INTERVAL );
      mScrollTimer.TickSignal().Connect( this, &Decorator::Impl::OnScrollTimerTick );
    }

    if( !mScrollTimer.IsRunning() )
    {
      mScrollTimer.Start();
    }
  }

  /**
   * Stops the timer used to scroll the text.
   */
  void StopScrollTimer()
  {
    if( mScrollTimer )
    {
      mScrollTimer.Stop();
    }
  }

  /**
   * Callback called by the timer used to scroll the text.
   *
   * It calculates and sets a new scroll position.
   */
  bool OnScrollTimerTick()
  {
    if( HANDLE_TYPE_COUNT != mHandleScrolling )
    {
      mController.DecorationEvent( mHandleScrolling,
                                   HANDLE_SCROLLING,
                                   mScrollDirection == SCROLL_RIGHT ? mScrollDistance : -mScrollDistance,
                                   0.f );
    }

    return true;
  }

  ControllerInterface& mController;

  TapGestureDetector  mTapDetector;
  PanGestureDetector  mPanGestureDetector;
  Timer               mCursorBlinkTimer;          ///< Timer to signal cursor to blink
  Timer               mScrollTimer;               ///< Timer used to scroll the text when the grab handle is moved close to the edges.

  Layer                mActiveLayer;                       ///< Layer for active handles and alike that ensures they are above all else.
  PropertyNotification mVerticalLessThanNotification;      ///< Notifies when the 'y' coord of the active layer is less than a given value.
  PropertyNotification mVerticalGreaterThanNotification;   ///< Notifies when the 'y' coord of the active layer is grater than a given value.
  PropertyNotification mHorizontalLessThanNotification;    ///< Notifies when the 'x' coord of the active layer is less than a given value.
  PropertyNotification mHorizontalGreaterThanNotification; ///< Notifies when the 'x' coord of the active layer is grater than a given value.
  ImageActor           mPrimaryCursor;
  ImageActor           mSecondaryCursor;

  Actor               mHighlightActor;            ///< Actor to display highlight
  Renderer            mHighlightRenderer;
  Material            mHighlightMaterial;         ///< Material used for highlight
  Property::Map       mQuadVertexFormat;
  Property::Map       mQuadIndexFormat;
  PopupImpl           mCopyPastePopup;
  TextSelectionPopup::Buttons mEnabledPopupButtons; /// Bit mask of currently enabled Popup buttons
  TextSelectionPopupCallbackInterface& mTextSelectionPopupCallbackInterface;

  Image               mHandleImages[HANDLE_TYPE_COUNT][HANDLE_IMAGE_TYPE_COUNT];
  Vector4             mHandleColor;

  CursorImpl          mCursor[CURSOR_COUNT];
  HandleImpl          mHandle[HANDLE_TYPE_COUNT];

  PropertyBuffer      mQuadVertices;
  PropertyBuffer      mQuadIndices;
  Geometry            mQuadGeometry;
  QuadContainer       mHighlightQuadList;         ///< Sub-selections that combine to create the complete selection highlight

  Vector4             mBoundingBox;               ///< The bounding box in world coords.
  Vector4             mHighlightColor;            ///< Color of the highlight
  Vector2             mHighlightPosition;         ///< The position of the highlight actor.
  Vector2             mControlSize;               ///< The control's size. Set by the Relayout.

  unsigned int        mActiveCursor;
  unsigned int        mCursorBlinkInterval;
  float               mCursorBlinkDuration;
  float               mCursorWidth;             ///< The width of the cursors in pixels.
  HandleType          mHandleScrolling;         ///< The handle which is scrolling.
  ScrollDirection     mScrollDirection;         ///< The direction of the scroll.
  float               mScrollThreshold;         ///< Defines a square area inside the control, close to the edge. A cursor entering this area will trigger scroll events.
  float               mScrollSpeed;             ///< The scroll speed in pixels per second.
  float               mScrollDistance;          ///< Distance the text scrolls during a scroll interval.
  int                 mTextDepth;               ///< The depth used to render the text.

  bool                mActiveCopyPastePopup              : 1;
  bool                mPopupSetNewPosition               : 1;
  bool                mCursorBlinkStatus                 : 1; ///< Flag to switch between blink on and blink off.
  bool                mDelayCursorBlink                  : 1; ///< Used to avoid cursor blinking when entering text.
  bool                mPrimaryCursorVisible              : 1; ///< Whether the primary cursor is visible.
  bool                mSecondaryCursorVisible            : 1; ///< Whether the secondary cursor is visible.
  bool                mFlipSelectionHandlesOnCross       : 1; ///< Whether to flip the selection handles as soon as they cross.
  bool                mFlipLeftSelectionHandleDirection  : 1; ///< Whether to flip the left selection handle image because of the character's direction.
  bool                mFlipRightSelectionHandleDirection : 1; ///< Whether to flip the right selection handle image because of the character's direction.
  bool                mHandlePanning                     : 1; ///< Whether any of the handles is moving.
  bool                mHandleCurrentCrossed              : 1; ///< Whether the handles are crossed.
  bool                mHandlePreviousCrossed             : 1; ///< Whether the handles where crossed at the last handle touch up.
  bool                mNotifyEndOfScroll                 : 1; ///< Whether to notify the end of the scroll.
};

DecoratorPtr Decorator::New( ControllerInterface& controller,
                             TextSelectionPopupCallbackInterface& callbackInterface )
{
  return DecoratorPtr( new Decorator( controller,
                                      callbackInterface ) );
}

void Decorator::SetBoundingBox( const Rect<int>& boundingBox )
{
  LocalToWorldCoordinatesBoundingBox( boundingBox, mImpl->mBoundingBox );
}

void Decorator::GetBoundingBox( Rect<int>& boundingBox ) const
{
  WorldToLocalCoordinatesBoundingBox( mImpl->mBoundingBox, boundingBox );
}

void Decorator::Relayout( const Vector2& size )
{
  mImpl->Relayout( size );
}

void Decorator::UpdatePositions( const Vector2& scrollOffset )
{
  mImpl->UpdatePositions( scrollOffset );
}

/** Cursor **/

void Decorator::SetActiveCursor( ActiveCursor activeCursor )
{
  mImpl->mActiveCursor = activeCursor;
}

unsigned int Decorator::GetActiveCursor() const
{
  return mImpl->mActiveCursor;
}

void Decorator::SetPosition( Cursor cursor, float x, float y, float cursorHeight, float lineHeight )
{
  mImpl->mCursor[cursor].position.x = x;
  mImpl->mCursor[cursor].position.y = y;
  mImpl->mCursor[cursor].cursorHeight = cursorHeight;
  mImpl->mCursor[cursor].lineHeight = lineHeight;
}

void Decorator::GetPosition( Cursor cursor, float& x, float& y, float& cursorHeight, float& lineHeight ) const
{
  x = mImpl->mCursor[cursor].position.x;
  y = mImpl->mCursor[cursor].position.y;
  cursorHeight = mImpl->mCursor[cursor].cursorHeight;
  lineHeight = mImpl->mCursor[cursor].lineHeight;
}

const Vector2& Decorator::GetPosition( Cursor cursor ) const
{
  return mImpl->mCursor[cursor].position;
}

void Decorator::SetCursorColor( Cursor cursor, const Dali::Vector4& color )
{
  mImpl->mCursor[cursor].color = color;
}

const Dali::Vector4& Decorator::GetColor( Cursor cursor ) const
{
  return mImpl->mCursor[cursor].color;
}

void Decorator::StartCursorBlink()
{
  if ( !mImpl->mCursorBlinkTimer )
  {
    mImpl->mCursorBlinkTimer = Timer::New( mImpl->mCursorBlinkInterval );
    mImpl->mCursorBlinkTimer.TickSignal().Connect( mImpl, &Decorator::Impl::OnCursorBlinkTimerTick );
  }

  if ( !mImpl->mCursorBlinkTimer.IsRunning() )
  {
    mImpl->mCursorBlinkTimer.Start();
  }
}

void Decorator::StopCursorBlink()
{
  if ( mImpl->mCursorBlinkTimer )
  {
    mImpl->mCursorBlinkTimer.Stop();
  }

  mImpl->mCursorBlinkStatus = true; // Keep cursor permanently shown
}

void Decorator::DelayCursorBlink()
{
  mImpl->mCursorBlinkStatus = true; // Show cursor for a bit longer
  mImpl->mDelayCursorBlink = true;
}

void Decorator::SetCursorBlinkInterval( float seconds )
{
  mImpl->mCursorBlinkInterval = static_cast<unsigned int>( seconds * TO_MILLISECONDS ); // Convert to milliseconds
}

float Decorator::GetCursorBlinkInterval() const
{
  return static_cast<float>( mImpl->mCursorBlinkInterval ) * TO_SECONDS;
}

void Decorator::SetCursorBlinkDuration( float seconds )
{
  mImpl->mCursorBlinkDuration = seconds;
}

float Decorator::GetCursorBlinkDuration() const
{
  return mImpl->mCursorBlinkDuration;
}

void Decorator::SetCursorWidth( int width )
{
  mImpl->mCursorWidth = static_cast<float>( width );
}

int Decorator::GetCursorWidth() const
{
  return static_cast<int>( mImpl->mCursorWidth );
}

/** Handles **/

void Decorator::SetHandleActive( HandleType handleType, bool active )
{
  mImpl->mHandle[handleType].active = active;

  if( !active )
  {
    if( ( LEFT_SELECTION_HANDLE == handleType ) || ( RIGHT_SELECTION_HANDLE == handleType ) )
    {
      mImpl->mHandlePreviousCrossed = false;
    }

    // TODO: this is a work-around.
    // The problem is the handle actor does not receive the touch event with the Interrupt
    // state when the power button is pressed and the application goes to background.
    mImpl->mHandle[handleType].pressed = false;
    Image imageReleased = mImpl->mHandleImages[handleType][HANDLE_IMAGE_RELEASED];
    ImageActor imageActor = mImpl->mHandle[handleType].actor;
    if( imageReleased && imageActor )
    {
       imageActor.SetImage( imageReleased );
    }
  }

}

bool Decorator::IsHandleActive( HandleType handleType ) const
{
  return mImpl->mHandle[handleType].active ;
}

void Decorator::SetHandleImage( HandleType handleType, HandleImageType handleImageType, Dali::Image image )
{
  mImpl->SetHandleImage( handleType, handleImageType, image );
}

Dali::Image Decorator::GetHandleImage( HandleType handleType, HandleImageType handleImageType ) const
{
  return mImpl->mHandleImages[handleType][handleImageType];
}

void Decorator::SetHandleColor( const Vector4& color )
{
  mImpl->mHandleColor = color;
}

const Vector4& Decorator::GetHandleColor() const
{
  return mImpl->mHandleColor;
}

void Decorator::SetPosition( HandleType handleType, float x, float y, float height )
{
  // Adjust grab handle displacement
  Impl::HandleImpl& handle = mImpl->mHandle[handleType];

  handle.grabDisplacementX -= x - handle.position.x;
  handle.grabDisplacementY -= y - handle.position.y;

  handle.position.x = x;
  handle.position.y = y;
  handle.lineHeight = height;
}

void Decorator::GetPosition( HandleType handleType, float& x, float& y, float& height ) const
{
  Impl::HandleImpl& handle = mImpl->mHandle[handleType];

  x = handle.position.x;
  y = handle.position.y;
  height = handle.lineHeight;
}

const Vector2& Decorator::GetPosition( HandleType handleType ) const
{
  return mImpl->mHandle[handleType].position;
}

void Decorator::FlipHandleVertically( HandleType handleType, bool flip )
{
  mImpl->mHandle[handleType].verticallyFlippedPreferred = flip;
}

bool Decorator::IsHandleVerticallyFlipped( HandleType handleType ) const
{
  return mImpl->mHandle[handleType].verticallyFlippedPreferred;
}

void Decorator::FlipSelectionHandlesOnCrossEnabled( bool enable )
{
  mImpl->mFlipSelectionHandlesOnCross = enable;
}

void Decorator::SetSelectionHandleFlipState( bool indicesSwapped, bool left, bool right )
{
  mImpl->mHandleCurrentCrossed = indicesSwapped;
  mImpl->mFlipLeftSelectionHandleDirection = left;
  mImpl->mFlipRightSelectionHandleDirection = right;
}

void Decorator::AddHighlight( float x1, float y1, float x2, float y2 )
{
  mImpl->mHighlightQuadList.push_back( QuadCoordinates(x1, y1, x2, y2) );
}

void Decorator::ClearHighlights()
{
  mImpl->mHighlightQuadList.clear();
  mImpl->mHighlightPosition = Vector2::ZERO;
}

void Decorator::SetHighlightColor( const Vector4& color )
{
  mImpl->mHighlightColor = color;
}

const Vector4& Decorator::GetHighlightColor() const
{
  return mImpl->mHighlightColor;
}

void Decorator::SetTextDepth( int textDepth )
{
  mImpl->mTextDepth = textDepth;
}

void Decorator::SetPopupActive( bool active )
{
  mImpl->mActiveCopyPastePopup = active;
}

bool Decorator::IsPopupActive() const
{
  return mImpl->mActiveCopyPastePopup ;
}

void Decorator::SetEnabledPopupButtons( TextSelectionPopup::Buttons& enabledButtonsBitMask )
{
  mImpl->mEnabledPopupButtons = enabledButtonsBitMask;

  if ( !mImpl->mCopyPastePopup.actor )
  {
    mImpl->mCopyPastePopup.actor = TextSelectionPopup::New( &mImpl->mTextSelectionPopupCallbackInterface );
#ifdef DECORATOR_DEBUG
    mImpl->mCopyPastePopup.actor.SetName("mCopyPastePopup");
#endif
    mImpl->mCopyPastePopup.actor.SetAnchorPoint( AnchorPoint::CENTER );
    mImpl->mCopyPastePopup.actor.OnRelayoutSignal().Connect( mImpl,  &Decorator::Impl::PopupRelayoutComplete  ); // Position popup after size negotiation
  }

  mImpl->mCopyPastePopup.actor.EnableButtons( mImpl->mEnabledPopupButtons );
}

TextSelectionPopup::Buttons& Decorator::GetEnabledPopupButtons()
{
  return mImpl->mEnabledPopupButtons;
}

/** Scroll **/

void Decorator::SetScrollThreshold( float threshold )
{
  mImpl->SetScrollThreshold( threshold );
}

float Decorator::GetScrollThreshold() const
{
  return mImpl->GetScrollThreshold();
}

void Decorator::SetScrollSpeed( float speed )
{
  mImpl->SetScrollSpeed( speed );
}

float Decorator::GetScrollSpeed() const
{
  return mImpl->GetScrollSpeed();
}

void Decorator::NotifyEndOfScroll()
{
  mImpl->NotifyEndOfScroll();
}

Decorator::~Decorator()
{
  delete mImpl;
}

Decorator::Decorator( ControllerInterface& controller,
                      TextSelectionPopupCallbackInterface& callbackInterface )
: mImpl( NULL )
{
  mImpl = new Decorator::Impl( controller, callbackInterface );
}

} // namespace Text

} // namespace Toolkit

} // namespace Dali
