/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_example_x6_mc_cantest_CanUtils */

#ifndef _Included_com_example_x6_mc_cantest_CanUtils
#define _Included_com_example_x6_mc_cantest_CanUtils
#ifdef __cplusplus
extern "C" {
#endif

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canOpen
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canOpen
  (JNIEnv *, jobject);

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canReadBytes
 * Signature: ([B)[B
 */
JNIEXPORT jobject JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canReadBytes
  (JNIEnv *, jobject, jint, jboolean);

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canWriteBytes
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canWriteBytes
  (JNIEnv *, jobject, jobject, jstring);

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canClose
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canClose
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
