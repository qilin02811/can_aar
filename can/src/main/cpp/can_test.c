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
#include <string.h>
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
#define SIOCSCANBAUDRATE        (SIOCDEVPRIVATE+0)

#define SIOCGCANBAUDRATE        (SIOCDEVPRIVATE+1)

#define SOL_CAN_RAW (SOL_CAN_BASE + CAN_RAW)
#define CAN_RAW_FILTER  1
#define CAN_RAW_RECV_OWN_MSGS 0x4 

typedef __u32 can_baudrate_t; // can 波特率

static int sock = -1;
struct sockaddr_can addr;
struct ifreq ifr;
struct ifreq ifr_tmp;
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
	int ret;
	/* Opening device */
	sock = socket(PF_CAN,SOCK_RAW,CAN_RAW);
	if(sock == -1)
	{
		LOGE("Can Write Without Open");
		return 0;
	}
	struct sockaddr_can addr_t;
    addr_t.can_family = AF_CAN;
    addr_t.can_ifindex = 0; // 关键点, 接口索引为0！！！
    // 将can_ifindex字段设置为0表示使用默认的网络接口。
    // 默认的网络接口通常是指连接在主机上的第一个CAN总线。系统会自动分配网络接口索引给每个CAN总线，
    // 因此将can_ifindex设置为0就可以使用默认的网络接口。如果有多个CAN总线连接在主机上，可以使用不同的网络接口索引来区分它们。

    bind(sock, (struct sockaddr *)&addr_t, sizeof(addr_t));
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
	char temp[16];
	bzero(temp,16);
	struct can_frame frame;
	socklen_t len = sizeof(addr);
	unsigned long nbytes = recvfrom(sock, &frame, sizeof (struct can_frame), 0, (struct sockaddr *)&addr, &len);

	for(int i = 0;i < frame.can_dlc;i ++)
	{
		temp[i] = frame.data[i];
//		LOGD("temp[i] = " , temp[i]);
	}

	ifr.ifr_ifindex = addr.can_ifindex;
	ioctl(sock,SIOCGIFNAME,&ifr);

	char can_temp[16];
	bzero(can_temp,16);
	strcpy(can_temp, ifr.ifr_name);

//    LOGE("read frame.canid = %u",frame.can_id);

	jclass canClass = (*env)->FindClass(env,"com/example/x6/mc_cantest/CanFrame"); // 获取Java类的引用
    jfieldID idCan = (*env)->GetFieldID(env, canClass,"canId", "J"); //获取canId在Java虚拟机中的引用
	jfieldID idExtend = (*env)->GetFieldID(env, canClass, "idExtend", "Z"); //获取idExtend在Java虚拟机中的引用
	jfieldID idLen = (*env)->GetFieldID(env, canClass,"len","I"); //获取len在Java虚拟机中的引用
    jfieldID idData = (*env)->GetFieldID(env, canClass,"data","[B"); // 获取data在Java虚拟机中的引用
	jfieldID idCanPort = (*env)->GetFieldID(env, canClass,"canPort", "Ljava/lang/String;");//获取canPort在Java虚拟机中的引用
    jmethodID constructMID = (*env)->GetMethodID(env, canClass, "<init>", "()V"); //获取init方法在Java虚拟机中的引用
	jobject canFrame = (*env)->NewObject(env, canClass, constructMID); // NewObject通过构造方法创建新的canFrame对象
	jstring port = (*env)->NewStringUTF(env, ifr.ifr_name);
    jbyteArray dataArray = (*env)->NewByteArray(env, frame.can_dlc); // 创建一个新的字节数组对象

    (*env)->SetByteArrayRegion(env, dataArray, 0, frame.can_dlc, (jbyte *)temp); //数组temp的内容复制到Java字节数组dataArray中，从位置0开始，复制长度为frame.can_dlc的字节
	(*env)->SetBooleanField(env, canFrame, idExtend, extend); //设置canFrame的idExtend字段值
    (*env)->SetLongField(env, canFrame, idCan, frame.can_id + (extend ? (0x01 << 31) : 0)); //设置canFrame的idCan字段值，如果extend有值则将idCan的二进制最高位设置为1
    (*env)->SetIntField(env, canFrame, idLen, frame.can_dlc); // 设置canFrame的idLen值，即数据长度
    (*env)->SetObjectField(env, canFrame, idData, dataArray); // 设置canFrame的idData字段
	(*env)->SetObjectField(env, canFrame, idCanPort, port);
	return canFrame;
	//返回使用JNI封装的canFrame对象，以提供给Java层使用
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

	//jfiedId是指针，用于访问和操作java类的字段
	jclass canCls  = (*env)->GetObjectClass(env, obj_can);
    jfieldID idCan = (*env)->GetFieldID(env, canCls, "canId", "J");
    jfieldID idExtend = (*env)->GetFieldID(env, canCls, "idExtend", "Z");
    jfieldID idLen = (*env)->GetFieldID(env, canCls,"len", "I");
    jfieldID idData = (*env)->GetFieldID(env, canCls, "data", "[B");

    jint canId = (*env)->GetLongField(env, obj_can, idCan); //获取idCan所指向的数据赋值给canId
    jboolean extend = (*env)->GetBooleanField(env, obj_can, idExtend); //获取idExtend所指向的数据赋值给extend
    jint len = (*env)->GetIntField(env, obj_can, idLen); //获取idLen所指向的数据赋值给len
    jbyteArray data = (jbyteArray)(*env)->GetObjectField(env, obj_can, idData); // 获取idData所指向的数据赋值给data

	jboolean iscopy;
	jbyte *send_data = (*env)->GetByteArrayElements(env, data, &iscopy); //获取Java字节数组data的元素，拷贝到send_data指向的内存位置上，转换为C/C++的字节数组
	frame.can_id = canId + (extend ? (0x01 << 31) : 0);
    const char *get_can = (*env)->GetStringUTFChars(env, can, 0);

    strcpy((char *)(ifr.ifr_name), get_can);
    int r = ioctl(sock,SIOCGIFINDEX,&ifr);
    // 这里调用ioctl使得sock获取到ifr的接口索引，
    // 但如果设备没有对应设置的can口则无法获取到，所以应该设置返回值r，r = 0说明获取到，r = -1说明获取失败。
	LOGD("r = %d",r);

    //这里对未存在的can口需要返回-1，避免通过canOpen方法初始化的sock在总线上传递数据
    if(r == -1)
    {
        return -1;
    }

	struct sockaddr_can write_addr;
    write_addr.can_family = AF_CAN;
    write_addr.can_ifindex = ifr.ifr_ifindex;
//	LOGD("write_addr.can_ifindex = %d",ifr.ifr_ifindex);

//	if(strlen(send_data) > 8)//用于支持当输入的字符大于8时的情况，分次数发送
    if (len > 8){
		//num = strlen(send_data) / 8;
		num=len / 8; // 一次发8位
        for(i = 0;i < num;i++){
			my_strcpy((jbyte *)frame.data, &send_data[8 * i], 8);

			frame.can_dlc = 8;
			sendto(sock,&frame,sizeof(struct can_frame),0,(struct sockaddr*)&write_addr,sizeof(addr));
			//sendto 发送数据帧frame 到套接字sock，指定目标地址write_addr,sizeof(struct can_frame)表示数据帧的大小，sizeof(addr)表示要发送的数据的字节数
		}

		memset((jbyte *)frame.data, 0, 8);
		my_strcpy((jbyte *)frame.data, &send_data[8 * i], len - num * 8);
		//frame.can_dlc = strlen(send_data) - num * 8;
        frame.can_dlc = len - num * 8;  //这里can_dlc保存的是按8位8位发送数据后余下的数据长度
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

	(*env)->ReleaseByteArrayElements(env, data, send_data, 0); //释放通过 GetByteArrayElements 函数获取的数组元素指针data
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
		close(sock); //关闭套接字
	}

	sock = -1;
	LOGD("Can close");
	return true;
}


#include <jni.h>

JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canSetFilters(JNIEnv* env, jobject thiz, jobject canFilters) {
	// 获取 List<CanFilter> 类的信息
	jclass listClass = (*env)->GetObjectClass(env, canFilters);
	jmethodID getMethod = (*env)->GetMethodID(env, listClass, "get", "(I)Ljava/lang/Object;");
	jmethodID sizeMethod = (*env)->GetMethodID(env, listClass, "size", "()I");
	jint length = (*env)->CallIntMethod(env, canFilters, sizeMethod);
	struct can_filter filters[length];

	// 遍历 List<CanFilter>
	for (jint i = 0; i < length; ++i) {
		// 获取 CanFilter 对象
		jobject canFilter = (*env)->CallObjectMethod(env, canFilters, getMethod, i);

		// 获取 CanFilter 类
		jclass canFilterClass = (*env)->GetObjectClass(env, canFilter);

		// 获取 CanFilter 的属性值（例如 can_id 和 can_mask）
		jfieldID canIdField = (*env)->GetFieldID(env, canFilterClass, "can_id", "J");
		jint canId = (*env)->GetLongField(env, canFilter, canIdField);

		jfieldID canMaskField = (*env)->GetFieldID(env, canFilterClass, "can_mask", "J");
		jint canMask = (*env)->GetLongField(env, canFilter, canMaskField);

		filters[i].can_id = canId;
		filters[i].can_mask =  canMask;
		LOGE("canId = %u, canMask = %x", filters[i].can_id, filters[i].can_mask);

		int res = setsockopt(sock,SOL_CAN_RAW,CAN_RAW_FILTER,&filters,sizeof(filters));
		if(res != 0) return -1;
		// 释放局部引用
		(*env)->DeleteLocalRef(env, canFilter);
	}
	// 返回结果
	return 0;
}




