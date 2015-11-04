#ifndef __DALI_TOOLKIT_TEST_SUITE_UTILS_H__
#define __DALI_TOOLKIT_TEST_SUITE_UTILS_H__

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

// dali-test-suite-utils.h needed first, but want to prevent certain headers
// from being read, as we want to override as much of their behaviour as possible.
#define __DALI_STYLE_MONITOR_H__
#define __DALI_ACCESSIBILITY_MANAGER_H__
#define __DALI_TIMER_H__
#define __DALI_CLIPBOARD_H__
#define __DALI_IMF_MANAGER_H__

#include <dali-test-suite-utils.h>

#include "toolkit-test-application.h"
#include "toolkit-application.h"

#endif // __DALI_TOOLKIT_TEST_SUITE_UTILS_H__
