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
#include "properties.h"
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

static int sock=-1;
int canfd=-1;
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


JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canOpen
  (JNIEnv *env, jobject thiz){


//	struct ifreq ifr;
	int ret;

	/* Opening device */
	canfd = socket(PF_CAN,SOCK_RAW,CAN_RAW);

	if(canfd==-1)
	{
		LOGE("Can Write Without Open");
		return   0;
	}

	strcpy((char *)(ifr.ifr_name),"can0");
	ioctl(canfd,SIOCGIFINDEX,&ifr);



	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	bind(canfd,(struct sockaddr*)&addr,sizeof(addr));

	return canfd;
  }

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canreadBytes
 * Signature: (Ljava/io/FileDescriptor;[BI)I
 */
JNIEXPORT jobject JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canreadBytes(JNIEnv *env, jobject thiz, jobject obj, jint time){


	unsigned long nbytes,len;

	struct can_frame frame = {0};
	int k=0;
	jstring   jstr;

	char temp[16];

	fd_set rfds;
	int retval;
	struct timeval tv;
        tv.tv_sec = time;
        tv.tv_usec = 0;

	bzero(temp,16);

	if(canfd==-1){
		LOGE("Can Read Without Open");
		frame.can_id=0;
		frame.can_dlc=0;
	}
	else
	{
		FD_ZERO(&rfds);
		FD_SET(canfd, &rfds);
		retval = select(canfd+1 , &rfds, NULL, NULL, &tv);
		if(retval == -1)
		{
			LOGE("Can Read slect error");
			frame.can_dlc=0;
			frame.can_id=0;
		}
		else if(retval)
		{
			nbytes = recvfrom(canfd, &frame, sizeof(struct can_frame), 0, (struct sockaddr *)&addr,&len);

			for(k = 0;k < frame.can_dlc;k++)
			{
				//LOGD("%c", frame.data[k]);
				temp[k] = frame.data[k];
				LOGD("temp[%d]=0x%x",k,temp[k]);
			}
			temp[k] = 0;

			//frame.can_id = frame.can_id - 0x80000000;//读得的id比实际的有个80000000差值，这里需要处理一下
			LOGD("Can Read slect success.");
		}
		else
		{
			frame.can_dlc=0;
			frame.can_id=0;
			//LOGD("Can no data.");
		}

	}


	jclass objectClass = (*env)->FindClass(env,"com/example/x6/mc_cantest/CanFrame");
    	jfieldID id = (*env)->GetFieldID(env,objectClass,"can_id","I");
    	jfieldID leng = (*env)->GetFieldID(env,objectClass,"can_dlc","C");
    	jfieldID str = (*env)->GetFieldID(env,objectClass,"data","Ljava/lang/String;");

	if(frame.can_dlc) {
		LOGD("can_id is :0x%x", frame.can_id);
		LOGD("can read nbytes=%d", frame.can_dlc);
		LOGD("can data is:%s", temp);
	}

    	(*env)->SetCharField(env, obj, leng, frame.can_dlc);
    	(*env)->SetObjectField(env, obj, str, (*env)->NewStringUTF(env,temp));
    	(*env)->SetIntField(env, obj, id, frame.can_id);

	return   obj;

}

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canwriteBytes
 * Signature: (Ljava/io/FileDescriptor;[BI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canwriteBytes
  (JNIEnv *env, jobject thiz, jint canId,jbyteArray data,jint len){

	int nbytes;
	int num = 0, i = 0;
	struct can_frame frame;

	jboolean iscopy;
	jbyte *send_data = (*env)->GetByteArrayElements(env, data, &iscopy);
	frame.can_id = canId;

//	if(strlen(send_data) > 8)//用于支持当输入的字符大于8时的情况，分次数发送
 if(len > 8)
	{
		//num = strlen(send_data) / 8;
		num=len / 8;
        for(i = 0;i < num;i++)
		{
			my_strcpy((jbyte *)frame.data, &send_data[8 * i], 8);
			frame.can_dlc = 8;
			sendto(canfd,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&addr,sizeof(addr));
		}
		memset((jbyte *)frame.data, 0, 8);
		my_strcpy((jbyte *)frame.data, &send_data[8 * i], strlen(send_data) - num * 8);
		//frame.can_dlc = strlen(send_data) - num * 8;
        frame.can_dlc = len - num * 8;
        sendto(canfd,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&addr,sizeof(addr));
		nbytes = len;
	}
	else
	{
		my_strcpy((jbyte *)frame.data, send_data, strlen(send_data));
		//frame.can_dlc = strlen(send_data);
        frame.can_dlc = len;
        sendto(canfd,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&addr,sizeof(addr));
		//nbytes = strlen(send_data);
        nbytes = len;
	}
	(*env)->ReleaseByteArrayElements(env, data, send_data,0);
	LOGD("write nbytes=%d",nbytes);
	return nbytes;
  }

/*
 * Class:     com_example_x6_mc_cantest_CanUtils
 * Method:    canClose
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_example_x6_mc_1cantest_CanUtils_canClose
  (JNIEnv *env, jobject thiz){

	if(canfd!=-1)
		close(canfd);
	canfd=-1;
	LOGD("close can0");
	return true;
  }






