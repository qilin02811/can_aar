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
 * Method:    canClose
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_doRealCanClose
  (JNIEnv *, jobject, jstring can);

JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canSetFilters
  (JNIEnv *, jobject , jobject canFilters, jstring can);

JNIEXPORT void JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_doRealCanReadBytes(JNIEnv *env, jobject thiz, jobject listener);

JNIEXPORT void JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canWriteBytesDebug(JNIEnv *env, jobject thiz, jobject can_frame,
                                                            jstring can_port);

JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canClearFilters(JNIEnv *env, jobject thiz);


JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_doSocketBind(JNIEnv *env, jobject thiz, jstring can);

// create epoll
JNIEXPORT void JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_createEpoll(JNIEnv *env, jobject thiz);

#ifdef __cplusplus
}
#endif
#endif







