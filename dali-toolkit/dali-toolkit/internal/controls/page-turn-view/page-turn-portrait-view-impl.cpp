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
#include <dali-toolkit/internal/controls/page-turn-view/page-turn-portrait-view-impl.h>

// EXTERNAL INCLUDES
#include <dali/public-api/animation/animation.h>
#include <dali/public-api/object/type-registry.h>
#include <dali/devel-api/object/type-registry-helper.h>
#include <dali/devel-api/rendering/cull-face.h>

//INTERNAL INCLUDES
#include <dali-toolkit/internal/controls/page-turn-view/page-turn-effect.h>

namespace Dali
{

namespace Toolkit
{

namespace Internal
{

namespace
{
using namespace Dali;

DALI_TYPE_REGISTRATION_BEGIN( Toolkit::PageTurnPortraitView, Toolkit::PageTurnView, NULL )
DALI_TYPE_REGISTRATION_END()

// the panning speed threshold, no matter how far is the pan displacement, pan fast to left/right quickly (speed > 0.3) will turn over/back the page
const float GESTURE_SPEED_THRESHOLD(0.3f);

// the animation duration of turning the previous page back when an outwards flick is detected
const float PAGE_TURN_OVER_ANIMATION_DURATION(0.5f);

}

PageTurnPortraitView::PageTurnPortraitView( PageFactory& pageFactory, const Vector2& pageSize )
: PageTurnView( pageFactory, pageSize )
{

}

PageTurnPortraitView::~PageTurnPortraitView()
{
}

Toolkit::PageTurnPortraitView PageTurnPortraitView::New( PageFactory& pageFactory, const Vector2& pageSize )
{
  // Create the implementation, temporarily owned on stack
  IntrusivePtr< PageTurnPortraitView > internalPageTurnView = new PageTurnPortraitView( pageFactory, pageSize );

  // Pass ownership to CustomActor
  Dali::Toolkit::PageTurnPortraitView pageTurnView( *internalPageTurnView );

  // Second-phase init of the implementation
  // This can only be done after the CustomActor connection has been made...
  internalPageTurnView->Initialize();

  return pageTurnView;
}

void PageTurnPortraitView::OnPageTurnViewInitialize()
{
  mControlSize = mPageSize;
  Self().SetSize( mPageSize );
  mTurningPageLayer.SetParentOrigin( ParentOrigin::CENTER_LEFT );
}

Vector2 PageTurnPortraitView::SetPanPosition( const Vector2& gesturePosition )
{
  return gesturePosition;
}

void PageTurnPortraitView::SetPanActor( const Vector2& panPosition )
{
  if( mCurrentPageIndex < mTotalPageCount )
  {
    mPanActor = mPageActors[mCurrentPageIndex%NUMBER_OF_CACHED_PAGES];
    mTurningPageIndex = mCurrentPageIndex;
  }
  else
  {
    mPanActor.Reset();
  }
}

void PageTurnPortraitView::SetSpineEffect(ImageActor actor, bool isLeftSide)
{
  if(isLeftSide)
  {
    actor.RemoveShaderEffect();
  }
  else
  {
    actor.SetShaderEffect( mSpineEffectFront );
  }
}

void PageTurnPortraitView::OnPossibleOutwardsFlick( const Vector2& panPosition, float gestureSpeed )
{
  Vector2 offset = panPosition - mPressDownPosition;
  // There is previous page and an outwards flick is detected
  if( mCurrentPageIndex > 0 && gestureSpeed > GESTURE_SPEED_THRESHOLD && offset.x > fabs( offset.y ))
  {
    ImageActor actor = mPageActors[ (mCurrentPageIndex-1) % NUMBER_OF_CACHED_PAGES ];
    if(actor.GetParent() != Self())
    {
      return;
    }

    // Guard against destruction during signal emission
    //Emit signal, to notify that page[mCurrentPageIndex-1] is turning backwards
    Toolkit::PageTurnView handle( GetOwner() );
    mTurningPageIndex = mCurrentPageIndex-1;
    mPageTurnStartedSignal.Emit( handle, static_cast<unsigned int>(mTurningPageIndex), false );

    //update pages
    mCurrentPageIndex--;
    RemovePage( mCurrentPageIndex+NUMBER_OF_CACHED_PAGES_EACH_SIDE );
    AddPage( mCurrentPageIndex-NUMBER_OF_CACHED_PAGES_EACH_SIDE );
    OrganizePageDepth();

    mPageActors[mTurningPageIndex%NUMBER_OF_CACHED_PAGES].SetVisible(true);

    // Add the page to tuning page layer and set up PageTurnEffect
    mShadowView.Add( actor );
    actor.SetShaderEffect( mTurnEffect[mIndex] );
    PageTurnApplyInternalConstraint(mTurnEffect[mIndex]);
    mIsAnimating[mIndex] = true;
    mTurnEffect[mIndex].SetUniform("uIsTurningBack", 1.f );
    Vector2 originalCenter( mPageSize.width*1.5f, 0.5f*mPageSize.height );
    mTurnEffect[mIndex].SetUniform("uOriginalCenter", originalCenter );
    mTurnEffect[mIndex].SetUniform("uCurrentCenter", Vector2( mPageSize.width*0.5f, mPageSize.height*0.5f ) );

    // Start an animation to turn the previous page back
    Animation animation = Animation::New( PAGE_TURN_OVER_ANIMATION_DURATION );
    mAnimationPageIdPair[animation] = mCurrentPageIndex;
    mAnimationIndexPair[animation] = mIndex;

    animation.AnimateTo( Property( mTurnEffect[mIndex], "uCurrentCenter" ),
                         originalCenter,
                         AlphaFunction::EASE_OUT, TimePeriod(PAGE_TURN_OVER_ANIMATION_DURATION*0.75f) );
    animation.AnimateBy( Property( actor, Actor::Property::ORIENTATION ), AngleAxis( Degree( 180.0f ), Vector3::YAXIS ) ,AlphaFunction::EASE_OUT );
    animation.Play();

    ImageActor imageActor = ImageActor::DownCast(actor);
    if( imageActor )
    {
      SetCullFace( imageActor, CullBack );
    }
    animation.FinishedSignal().Connect( this, &PageTurnPortraitView::TurnedOverBackwards );
  }
}

void PageTurnPortraitView::OnTurnedOver( ImageActor actor, bool isLeftSide )
{
  if( isLeftSide )
  {
    actor.SetVisible( false );
  }
}

void PageTurnPortraitView::TurnedOverBackwards( Animation& animation )
{
  ImageActor imageActor =  mPageActors[mAnimationPageIdPair[animation] % NUMBER_OF_CACHED_PAGES];
  if( imageActor )
  {
    SetCullFace( imageActor, CullNone );
  }
  TurnedOver( animation );
}

} // namespace Internal

} // namespace Toolkit

} // namespace Dali
