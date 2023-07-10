#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <pthread.h>
#include "can.h"
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "jni.h"
#include <assert.h>
#include <net/if.h>
#include <sys/wait.h>
#include "com_example_x6_mc_cantest_CanUtils.h"
// 引入log头文件
#include <android/log.h>
#include <strings.h>
#include <sys/system_properties.h>
// log标签
#define  TAG    "CAN_Load_JNI"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
//#define PF_CAN 29
//
//#define AF_CAN PF_CAN
#define true 1
#define false 0
struct ifreq ifr;
#define SIOCSCANBAUDRATE        (SIOCDEVPRIVATE+0)

#define SIOCGCANBAUDRATE        (SIOCDEVPRIVATE+1)

#define SOL_CAN_RAW (SOL_CAN_BASE + CAN_RAW)
#define CAN_RAW_FILTER  1
#define CAN_RAW_RECV_OWN_MSGS 0x4 

typedef __u32 can_baudrate_t;

static int sock = -1;
struct sockaddr_can addr;
struct ifreq ifr;
void my_strcpy(char *dest, char *src, size_t n)
{
	char i = 0;
	while(i < n)
	{
		*(dest++) = *(src++);
		i++;
	}
}


/*
JNIEXPORT  void JNICALL Java_com_example_x6_mc_1cantest_CanUtils_InitCan
  (JNIEnv *env, jobject thiz, jint baudrate)
{


	switch (baudrate)
	{
		case 5000   :
		case 10000  :
		case 20000  :
		case 50000  :
		case 100000 :
		case 125000 :
		case 500000 :
		case 1000000:
			LOGI("Can Bus Speed is %d",baudrate);
		break;
		default:
			LOGI("Can Bus Speed is %d.if it do not work,try 5000~1000000",baudrate);
	}


	if(baudrate!=0)
	{
		char str_baudrate[16];

		sprintf(str_baudrate,"%d", baudrate);
		property_set("net.can.baudrate", str_baudrate);
		LOGI("str_baudrate is:%s", str_baudrate);
		property_set("net.can.change", "yes");
	}

	sleep(2);//wait for can0 up
}
*/


JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canOpen(JNIEnv *env, jobject thiz) {
//	struct ifreq ifr;
	int ret;


	/* Opening device */
	sock = socket(PF_CAN,SOCK_RAW,CAN_RAW);
	if(sock == -1)
	{
		LOGE("Can Write Without Open");
		return   0;
	}

	struct sockaddr_can addr_t;
    addr_t.can_family = AF_CAN;
    addr_t.can_ifindex = 0; // 关键点, 接口索引为0 ！！！
    bind(sock, (struct sockaddr *)&addr_t, sizeof(addr_t));

//	strcpy((char *)(ifr.ifr_name), get_can);
//	ioctl(sock,SIOCGIFINDEX,&ifr);
//	addr.can_family = AF_CAN;
//    addr.can_ifindex = ifr.ifr_ifindex;

	LOGD("Can open, socket fd == %d", sock);
	return sock;
}

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canreadBytes
 * Signature: (Ljava/io/FileDescriptor;[BI)I
 */
JNIEXPORT jobject JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canReadBytes(JNIEnv *env, jobject thiz, jint time, jboolean extend) {
	unsigned long nbytes,len;

	struct can_frame frame = {0};
	int k=0;
	jstring jstr;

	char temp[16];

	fd_set rfds;
	int retval;
	struct timeval tv;
    tv.tv_sec = time;
    tv.tv_usec = 0;

	bzero(temp,16);

	if (sock == -1) {
		LOGE("Can Read Without Open");
		frame.can_id=0;
		frame.can_dlc=0;
	} else {
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);
		retval = select(sock+1 , &rfds, NULL, NULL, &tv);

		if(retval == -1) {
			LOGE("Can Read select error");
			frame.can_dlc=0;
			frame.can_id=0;
		} else if (retval) {
			nbytes = recvfrom(sock, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)&addr, &len);

//            LOGD("ID=0x%X DLC=%d Data: ", frame.can_id, frame.can_dlc);
			for(k = 0;k < frame.can_dlc;k++) {
				temp[k] = frame.data[k];
//				LOGD("0x%02X ", frame.data[k]);
			}
//			temp[k] = 0;

			//frame.can_id = frame.can_id - 0x80000000;//读得的id比实际的有个80000000差值，这里需要处理一下
//			LOGD("Can Read slect success.");
		} else {
			frame.can_dlc=0;
			frame.can_id=0;
//			LOGD("Can no data, socket fd == %d", sock);
		}
	}

	jclass canClass = (*env)->FindClass(env,"com/example/x6/mc_cantest/CanFrame");
    jfieldID idCan = (*env)->GetFieldID(env, canClass,"canId","I");
	jfieldID idExtend = (*env)->GetFieldID(env, canClass, "idExtend", "Z");
	jfieldID idLen = (*env)->GetFieldID(env, canClass,"len","I");
    jfieldID idData = (*env)->GetFieldID(env, canClass,"data","[B");

    jmethodID constructMID = (*env)->GetMethodID(env, canClass, "<init>", "()V");
	jobject canFrame = (*env)->NewObject(env, canClass, constructMID);

    jbyteArray dataArray = (*env)->NewByteArray(env, frame.can_dlc);
    (*env)->SetByteArrayRegion(env, dataArray, 0, frame.can_dlc, (jbyte *)temp);

	(*env)->SetBooleanField(env, canFrame, idExtend, extend);
    (*env)->SetIntField(env, canFrame, idCan, frame.can_id + (extend ? (0x01 << 31) : 0));
    (*env)->SetIntField(env, canFrame, idLen, frame.can_dlc);
    (*env)->SetObjectField(env, canFrame, idData, dataArray);

	return canFrame;
}

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canwriteBytes
 * Signature: (Ljava/io/FileDescriptor;[BI)Z
Java_com_example_x6_mc_1cantest_CanUtils_canWriteBytes(JNIEnv *env, jobject thiz, jint canId,jbyteArray data,jint len){
 */
JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canWriteBytes(JNIEnv *env, jobject thiz, jobject obj_can, jstring can) {
	int nbytes;
	int num = 0, i = 0;
	struct can_frame frame;

	jclass canCls  = (*env)->GetObjectClass(env, obj_can);
    jfieldID idCan = (*env)->GetFieldID(env, canCls, "canId", "I");
    jfieldID idExtend = (*env)->GetFieldID(env, canCls, "idExtend", "Z");
    jfieldID idLen = (*env)->GetFieldID(env, canCls,"len", "I");
    jfieldID idData = (*env)->GetFieldID(env, canCls, "data", "[B");

    jint canId = (*env)->GetIntField(env, obj_can, idCan);
    jboolean extend = (*env)->GetBooleanField(env, obj_can, idExtend);
    jint len = (*env)->GetIntField(env, obj_can, idLen);
    jbyteArray data = (jbyteArray)(*env)->GetObjectField(env, obj_can, idData);

	jboolean iscopy;
	jbyte *send_data = (*env)->GetByteArrayElements(env, data, &iscopy);
	frame.can_id = canId + (extend ? (0x01 << 31) : 0);

    const char *get_can = (*env)->GetStringUTFChars(env, can, 0);
//    LOGD("write can is %s", get_can);
    strcpy((char *)(ifr.ifr_name), get_can);
    ioctl(sock,SIOCGIFINDEX,&ifr);
    struct sockaddr_can write_addr;
    write_addr.can_family = AF_CAN;
    write_addr.can_ifindex = ifr.ifr_ifindex;

//	if(strlen(send_data) > 8)//用于支持当输入的字符大于8时的情况，分次数发送
    if (len > 8){
		//num = strlen(send_data) / 8;
		num=len / 8;
        for(i = 0;i < num;i++){
			my_strcpy((jbyte *)frame.data, &send_data[8 * i], 8);
			frame.can_dlc = 8;
			sendto(sock,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&write_addr,sizeof(addr));
		}

		memset((jbyte *)frame.data, 0, 8);
		my_strcpy((jbyte *)frame.data, &send_data[8 * i], len - num * 8);
		//frame.can_dlc = strlen(send_data) - num * 8;
        frame.can_dlc = len - num * 8;
        int ret = sendto(sock,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&write_addr,sizeof(addr));
        if (ret < 0) {
			return ret;
        }
		nbytes = len;
	} else {
		my_strcpy((jbyte *)frame.data, send_data, len);
		//frame.can_dlc = strlen(send_data);
        frame.can_dlc = len;
        int ret = sendto(sock,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&write_addr,sizeof(addr));
        if (ret < 0) {
			return ret;
        }
		//nbytes = strlen(send_data);
        nbytes = len;
	}

	(*env)->ReleaseByteArrayElements(env, data, send_data, 0);
//	LOGD("write nbytes=%d",nbytes);
	return nbytes;
}

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canClose
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canClose(JNIEnv *env, jobject thiz){
	if (sock != -1) {
		close(sock);
	}

	sock = -1;
	LOGD("Can close");
	return true;
}






