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

#include <dali-toolkit/public-api/controls/buttons/push-button.h>

// INTERNAL INCLUDES

#include <dali-toolkit/internal/controls/buttons/push-button-impl.h>
#include <dali/public-api/actors/image-actor.h>

namespace Dali
{

namespace Toolkit
{

PushButton::PushButton()
: Button()
{
}

PushButton::PushButton( Internal::PushButton& implementation )
: Button( implementation )
{
}

PushButton::PushButton( const PushButton& pushButton )
: Button( pushButton )
{
}

PushButton& PushButton::operator=( const PushButton& pushButton )
{
  if( &pushButton != this )
  {
    Button::operator=( pushButton );
  }
  return *this;
}

PushButton::PushButton( Dali::Internal::CustomActor* internal )
: Button( internal )
{
  VerifyCustomActorPointer<Internal::PushButton>(internal);
}

PushButton::~PushButton()
{
}

PushButton PushButton::New()
{
  return Internal::PushButton::New();
}

PushButton PushButton::DownCast( BaseHandle handle )
{
  return Control::DownCast<PushButton, Internal::PushButton>(handle);
}

// Deprecated API

void PushButton::SetButtonImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetButtonImage( image );
}

void PushButton::SetBackgroundImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetBackgroundImage( image );
}

void PushButton::SetSelectedImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetSelectedImage( image );
}

void PushButton::SetSelectedBackgroundImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetSelectedBackgroundImage( image );
}

void PushButton::SetDisabledBackgroundImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetDisabledBackgroundImage( image );
}

void PushButton::SetDisabledImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetDisabledImage( image );
}

void PushButton::SetDisabledSelectedImage( Actor image )
{
  Dali::Toolkit::GetImplementation( *this ).SetDisabledSelectedImage( image );
}

} // namespace Toolkit

} // namespace Dali
