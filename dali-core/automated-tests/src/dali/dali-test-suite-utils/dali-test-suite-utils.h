#ifndef __DALI_TEST_SUITE_UTILS_H__
#define __DALI_TEST_SUITE_UTILS_H__

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

// EXTERNAL INCLUDES
#include <cstdarg>
#include <iostream>

// INTERNAL INCLUDES
#include <dali/public-api/dali-core.h>

void tet_infoline(const char*str);
void tet_printf(const char *format, ...);

#include "test-application.h"

using namespace Dali;

#define STRINGIZE_I(text) #text
#define STRINGIZE(text) STRINGIZE_I(text)

// the following is the other compilers way of token pasting, gcc seems to just concatenate strings automatically
//#define TOKENPASTE(x,y) x ## y
#define TOKENPASTE(x,y) x y
#define TOKENPASTE2(x,y) TOKENPASTE( x, y )
#define TEST_LOCATION TOKENPASTE2( "Test failed in ", TOKENPASTE2( __FILE__, TOKENPASTE2( ", line ", STRINGIZE(__LINE__) ) ) )

#define TET_UNDEF 2
#define TET_FAIL 1
#define TET_PASS 0

extern int test_return_value;

void tet_result(int value);

#define END_TEST \
  return ((test_return_value>0)?1:0)

void tet_infoline(const char* str);
void tet_printf(const char *format, ...);

/**
 * DALI_TEST_CHECK is a wrapper for tet_result.
 * If the condition evaluates to false, the test is stopped.
 * @param[in] The boolean expression to check
 */
#define DALI_TEST_CHECK(condition)                                                        \
if ( (condition) )                                                                        \
{                                                                                         \
  tet_result(TET_PASS);                                                                   \
}                                                                                         \
else                                                                                      \
{                                                                                         \
  fprintf(stderr, "%s Failed in %s at line %d\n", __PRETTY_FUNCTION__, __FILE__, __LINE__);    \
  tet_result(TET_FAIL);                                                                   \
  throw("TET_FAIL");                                                                      \
}

template <typename Type>
inline bool CompareType(Type value1, Type value2, float epsilon);

/**
 * A helper for fuzzy-comparing Vector2 objects
 * @param[in] vector1 the first object
 * @param[in] vector2 the second object
 * @param[in] epsilon difference threshold
 * @returns true if difference is smaller than epsilon threshold, false otherwise
 */
template <>
inline bool CompareType<float>(float value1, float value2, float epsilon)
{
  return fabsf(value1 - value2) < epsilon;
}

/**
 * A helper for fuzzy-comparing Vector2 objects
 * @param[in] vector1 the first object
 * @param[in] vector2 the second object
 * @param[in] epsilon difference threshold
 * @returns true if difference is smaller than epsilon threshold, false otherwise
 */
template <>
inline bool CompareType<Vector2>(Vector2 vector1, Vector2 vector2, float epsilon)
{
  return fabsf(vector1.x - vector2.x)<epsilon && fabsf(vector1.y - vector2.y)<epsilon;
}

/**
 * A helper for fuzzy-comparing Vector3 objects
 * @param[in] vector1 the first object
 * @param[in] vector2 the second object
 * @param[in] epsilon difference threshold
 * @returns true if difference is smaller than epsilon threshold, false otherwise
 */
template <>
inline bool CompareType<Vector3>(Vector3 vector1, Vector3 vector2, float epsilon)
{
  return fabsf(vector1.x - vector2.x)<epsilon &&
         fabsf(vector1.y - vector2.y)<epsilon &&
         fabsf(vector1.z - vector2.z)<epsilon;
}


/**
 * A helper for fuzzy-comparing Vector4 objects
 * @param[in] vector1 the first object
 * @param[in] vector2 the second object
 * @param[in] epsilon difference threshold
 * @returns true if difference is smaller than epsilon threshold, false otherwise
 */
template <>
inline bool CompareType<Vector4>(Vector4 vector1, Vector4 vector2, float epsilon)
{
  return fabsf(vector1.x - vector2.x)<epsilon &&
         fabsf(vector1.y - vector2.y)<epsilon &&
         fabsf(vector1.z - vector2.z)<epsilon &&
         fabsf(vector1.w - vector2.w)<epsilon;
}

template <>
inline bool CompareType<Quaternion>(Quaternion q1, Quaternion q2, float epsilon)
{
  Quaternion q2N = -q2; // These quaternions represent the same rotation
  return CompareType<Vector4>(q1.mVector, q2.mVector, epsilon) || CompareType<Vector4>(q1.mVector, q2N.mVector, epsilon);
}

template <>
inline bool CompareType<Radian>(Radian q1, Radian q2, float epsilon)
{
  return CompareType<float>(q1.radian, q2.radian, epsilon);
}

template <>
inline bool CompareType<Degree>(Degree q1, Degree q2, float epsilon)
{
  return CompareType<float>(q1.degree, q2.degree, epsilon);
}

bool operator==(TimePeriod a, TimePeriod b);
std::ostream& operator<<( std::ostream& ostream, TimePeriod value );
std::ostream& operator<<( std::ostream& ostream, Radian angle );
std::ostream& operator<<( std::ostream& ostream, Degree angle );

/**
 * Test whether two values are equal.
 * @param[in] value1 The first value
 * @param[in] value2 The second value
 * @param[in] location The TEST_LOCATION macro should be used here
 */
template<typename TypeA, typename TypeB>
inline void DALI_TEST_EQUALS(TypeA value1, TypeB value2, const char* location)
{
  if (!(value1 == value2))
  {
    std::ostringstream o;
    o << value1 << " == " << value2 << std::endl;
    fprintf(stderr, "%s, checking %s", location, o.str().c_str());
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

template<typename Type>
inline void DALI_TEST_EQUALS(Type value1, Type value2, float epsilon, const char* location)
{
  if( !CompareType<Type>(value1, value2, epsilon) )
  {
    std::ostringstream o;
    o << value1 << " == " << value2 << std::endl;
    fprintf(stderr, "%s, checking %s", location, o.str().c_str());
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/**
 * Test whether two TimePeriods are within a certain distance of each other.
 * @param[in] value1 The first value
 * @param[in] value2 The second value
 * @param[in] epsilon The values must be within this distance of each other
 * @param[in] location The TEST_LOCATION macro should be used here
 */
template<>
inline void DALI_TEST_EQUALS<TimePeriod>( TimePeriod value1, TimePeriod value2, float epsilon, const char* location)
{
  if ((fabs(value1.durationSeconds - value2.durationSeconds) > epsilon))
  {
    fprintf(stderr, "%s, checking durations %f == %f, epsilon %f\n", location, value1.durationSeconds, value2.durationSeconds, epsilon);
    tet_result(TET_FAIL);
  }
  else if ((fabs(value1.delaySeconds - value2.delaySeconds) > epsilon))
  {
    fprintf(stderr, "%s, checking delays %f == %f, epsilon %f\n", location, value1.delaySeconds, value2.delaySeconds, epsilon);
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/**
 * Test whether two Matrix3 objects are equal.
 * @param[in] matrix1 The first object
 * @param[in] matrix2 The second object
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const Matrix3& matrix1, const Matrix3& matrix2, const char* location);

/** Test whether two Matrix3 objects are equal (fuzzy compare).
 * @param[in] matrix1 The first object
 * @param[in] matrix2 The second object
 * @param[in] epsilon The epsilon to use for comparison
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const Matrix3& matrix1, const Matrix3& matrix2, float epsilon, const char* location);

/**
 * Test whether two Matrix objects are equal.
 * @param[in] matrix1 The first object
 * @param[in] matrix2 The second object
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const Matrix& matrix1, const Matrix& matrix2, const char* location);

/**
 * Test whether two Matrix objects are equal (fuzzy-compare).
 * @param[in] matrix1 The first object
 * @param[in] matrix2 The second object
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const Matrix& matrix1, const Matrix& matrix2, float epsilon, const char* location);

/**
 * Test whether two strings are equal.
 * @param[in] str1 The first string
 * @param[in] str2 The second string
 * @param[in] location The TEST_LOCATION macro should be used here
 */
template<>
inline void DALI_TEST_EQUALS<const char*>( const char* str1, const char* str2, const char* location)
{
  if (strcmp(str1, str2))
  {
    fprintf(stderr, "%s, checking '%s' == '%s'\n", location, str1, str2);
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/**
 * Test whether two strings are equal.
 * @param[in] str1 The first string
 * @param[in] str2 The second string
 * @param[in] location The TEST_LOCATION macro should be used here
 */
template<>
inline void DALI_TEST_EQUALS<const std::string&>( const std::string &str1, const std::string &str2, const char* location)
{
  DALI_TEST_EQUALS(str1.c_str(), str2.c_str(), location);
}

/**
 * Test whether two strings are equal.
 * @param[in] str1 The first string
 * @param[in] str2 The second string
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const std::string &str1, const char* str2, const char* location);

/**
 * Test whether two strings are equal.
 * @param[in] str1 The first string
 * @param[in] str2 The second string
 * @param[in] location The TEST_LOCATION macro should be used here
 */
void DALI_TEST_EQUALS( const char* str1, const std::string &str2, const char* location);

/**
 * Test whether one unsigned integer value is greater than another.
 * Test succeeds if value1 > value2
 * @param[in] value1 The first value
 * @param[in] value2 The second value
 * @param[in] location The TEST_LOCATION macro should be used here
 */
template< typename T >
void DALI_TEST_GREATER( T value1, T value2, const char* location)
{
  if (!(value1 > value2))
  {
    std::cerr << location << ", checking " << value1 <<" > " << value2 << "\n";
    tet_result(TET_FAIL);
  }
  else
  {
    tet_result(TET_PASS);
  }
}

/**
 * Test whether the assertion condition that failed and thus triggered the
 * exception \b e contained a given substring.
 * @param[in] e The exception that we expect was fired by a runtime assertion failure.
 * @param[in] conditionSubString The text that we expect to be present in an
 *                               assertion which triggered the exception.
 * @param[in] location The TEST_LOCATION macro should be used here.
 */
void DALI_TEST_ASSERT( DaliException& e, std::string conditionSubString, const char* location );

/**
 * Print the assert
 * @param[in] e The exception that we expect was fired by a runtime assertion failure.
 */
inline void DALI_TEST_PRINT_ASSERT( DaliException& e )
{
  tet_printf("Assertion %s failed at %s\n", e.condition, e.location );
}

// Functor to test whether an Applied signal is emitted
struct ConstraintAppliedCheck
{
  ConstraintAppliedCheck( bool& signalReceived );
  void operator()( Constraint& constraint );
  void Reset();
  void CheckSignalReceived();
  void CheckSignalNotReceived();
  bool& mSignalReceived; // owned by individual tests
};

/**
 * A Helper to test default functions
 */
template <typename T>
struct DefaultFunctionCoverage
{
  DefaultFunctionCoverage()
  {
    T a;
    T *b = new T(a);
    DALI_TEST_CHECK(b);
    a = *b;
    delete b;
  }
};


// Helper to Create buffer image
BufferImage CreateBufferImage();

#endif // __DALI_TEST_SUITE_UTILS_H__
