#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <pthread.h>
#include <linux/can/raw.h>
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
#include <sys/epoll.h>
#include "com_example_x6_mc_cantest_CanUtils.h"

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
#define true 1


#define SOL_CAN_RAW (SOL_CAN_BASE + CAN_RAW)
#define CAN_RAW_FILTER  1

static int sock = -1;
struct sockaddr_can addr;

struct ifreq ifr;
int currmax = 3;
#ifndef SO_TIMESTAMPING
#define SO_TIMESTAMPING 37
#endif

#define MAXSOCK 16    /* max. number of CAN interfaces given on the cmdline */
#define MAXIFNAMES 30 /* size of receive name index to omit ioctls */
#define ANYDEV "any"  /* name of interface to receive from any CAN interface */


struct if_info { /* bundled information per open socket */
    int s; /* socket */
    char *cmdlinename;// CAN口名称
    __u32 dropcnt; //
    __u32 last_dropcnt;//
};

static struct if_info sock_info[MAXSOCK];
static char devname[MAXIFNAMES][IFNAMSIZ+1];
static int dindex[MAXIFNAMES];
static int max_devname_len; /* to prevent frazzled device name output */
static const int canfd_on = 1;
char buf[255];
static int frame_count = 0;
static volatile int running = 1;
int fd_epoll;
static int s;

void my_strcpy(char *dest, char *src, size_t n)
{
	char i = 0;
	while(i < n)
	{
		*(dest++) = *(src++);
		i++;
	}
}

JNIEXPORT jint JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canOpen(JNIEnv *env, jobject thiz) {

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
    jfieldID idData = (*env)->GetFieldID(env, canClass,"data","[C"); // 获取data在Java虚拟机中的引用
	jfieldID idCanPort = (*env)->GetFieldID(env, canClass,"canPort", "Ljava/lang/String;");//获取canPort在Java虚拟机中的引用
    jmethodID constructMID = (*env)->GetMethodID(env, canClass, "<init>", "()V"); //获取init方法在Java虚拟机中的引用
	jobject canFrame = (*env)->NewObject(env, canClass, constructMID); // NewObject通过构造方法创建新的canFrame对象
	jstring port = (*env)->NewStringUTF(env, ifr.ifr_name);
    jcharArray dataArray = (*env)->NewCharArray(env, frame.can_dlc); // 创建一个新的字节数组对象

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
    jfieldID idData = (*env)->GetFieldID(env, canCls, "data", "[C");

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
	for (int i = 0; i < currmax; i++)
		close(sock_info[i].s);
	close(fd_epoll);

	frame_count = 0;
	sock = -1;
	close(s);
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

/******************************************************************************************************************/
//canWriteBytesDebug
#define CANID_DELIM '#'
#define CC_DLC_DELIM '_'
#define DATA_SEPERATOR '.'


unsigned char asc2nibble(char c) {

	if ((c >= '0') && (c <= '9'))
		return c - '0';

	if ((c >= 'A') && (c <= 'F'))
		return c - 'A' + 10;

	if ((c >= 'a') && (c <= 'f'))
		return c - 'a' + 10;

	return 16; /* error */
}

int parse_canframe(char *cs, struct canfd_frame *cf) {
	/* documentation see lib.h */

	int i, idx, dlen, len;
	int maxdlen = CAN_MAX_DLEN;
	int ret = CAN_MTU;
	canid_t tmp;

	len = strlen(cs);
	//printf("'%s' len %d\n", cs, len);

	memset(cf, 0, sizeof(*cf)); /* init CAN FD frame, e.g. LEN = 0 */

	if (len < 4)
		return 0;

	if (cs[3] == CANID_DELIM) { /* 3 digits */

		idx = 4;
		for (i=0; i<3; i++){
			if ((tmp = asc2nibble(cs[i])) > 0x0F)
				return 0;
			cf->can_id |= (tmp << (2-i)*4);
		}

	} else if (cs[8] == CANID_DELIM) { /* 8 digits */

		idx = 9;
		for (i=0; i<8; i++){
			if ((tmp = asc2nibble(cs[i])) > 0x0F)
				return 0;
			cf->can_id |= (tmp << (7-i)*4);
		}
		if (!(cf->can_id & CAN_ERR_FLAG)) /* 8 digits but no errorframe?  */
			cf->can_id |= CAN_EFF_FLAG;   /* then it is an extended frame */

	} else
		return 0;


	if (cs[idx] == CANID_DELIM) { /* CAN FD frame escape char '##' */

		maxdlen = CANFD_MAX_DLEN;
		ret = CANFD_MTU;

		/* CAN FD frame <canid>##<flags><data>* */
		if ((tmp = asc2nibble(cs[idx+1])) > 0x0F)
			return 0;

		cf->flags = tmp;
		idx += 2;
	}
	for (i=0, dlen=0; i < maxdlen; i++){

		if(cs[idx] == DATA_SEPERATOR) /* skip (optional) separator */
			idx++;

		if(idx >= len) /* end of string => end of data */
			break;

		if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
			return 0;
		cf->data[i] = (tmp << 4);
		if ((tmp = asc2nibble(cs[idx++])) > 0x0F)
			return 0;
		cf->data[i] |= tmp;
		dlen++;
	}
	cf->len = dlen;

	/* check for extra DLC when having a Classic CAN with 8 bytes payload */
//	if ((maxdlen == CAN_MAX_DLEN) && (dlen == CAN_MAX_DLEN) && (cs[idx++] == CC_DLC_DELIM)) {
//		unsigned char dlc = asc2nibble(cs[idx]);
//
//		if ((dlc > CAN_MAX_DLEN) && (dlc <= CAN_MAX_RAW_DLC)) {
//			struct can_frame *ccf = (struct can_frame *)cf;
//
//			ccf->len8_dlc = dlc;
//		}
//	}

	return ret;
}

static const unsigned char dlc2len[] = {0, 1, 2, 3, 4, 5, 6, 7,
										8, 12, 16, 20, 24, 32, 48, 64};

/* get data length from raw data length code (DLC) */
unsigned char can_fd_dlc2len(unsigned char dlc)
{
	return dlc2len[dlc & 0x0F];
}
static const unsigned char len2dlc[] = {0, 1, 2, 3, 4, 5, 6, 7, 8,              /* 0 - 8 */
										9, 9, 9, 9,                             /* 9 - 12 */
										10, 10, 10, 10,                         /* 13 - 16 */
										11, 11, 11, 11,                         /* 17 - 20 */
										12, 12, 12, 12,                         /* 21 - 24 */
										13, 13, 13, 13, 13, 13, 13, 13,         /* 25 - 32 */
										14, 14, 14, 14, 14, 14, 14, 14,         /* 33 - 40 */
										14, 14, 14, 14, 14, 14, 14, 14,         /* 41 - 48 */
										15, 15, 15, 15, 15, 15, 15, 15,         /* 49 - 56 */
										15, 15, 15, 15, 15, 15, 15, 15};        /* 57 - 64 */

unsigned char can_fd_len2dlc(unsigned char len)
{
	if (len > 64)
		return 0xF;

	return len2dlc[len];
}



JNIEXPORT void JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canWriteBytesDebug(JNIEnv *env, jobject thiz, jobject can_frame,
															jstring can_port){
	int required_mtu;
	int num = 0, i = 0;
	int enable_canfd = 1;
	struct canfd_frame frame;
	struct sockaddr_can addr;
	struct ifreq ifr;

	//jfiedId是指针，用于访问和操作java类的字段
	jclass canCls  = (*env)->GetObjectClass(env, can_frame);
	jfieldID idCan = (*env)->GetFieldID(env, canCls, "canId", "J");
	jfieldID idExtend = (*env)->GetFieldID(env, canCls, "idExtend", "Z");
	jfieldID idLen = (*env)->GetFieldID(env, canCls,"len", "I");
	jfieldID idData = (*env)->GetFieldID(env, canCls, "data", "Ljava/lang/String;");


	jint canId = (*env)->GetLongField(env, can_frame, idCan); //获取idCan所指向的数据赋值给canId
	jstring data = (*env)->GetObjectField(env, can_frame, idData); // 获取idData所指向的数据赋值给data

	char *final_data = (*env)->GetStringUTFChars(env, data, NULL);
	const char *port = (*env)->GetStringUTFChars(env, can_port, NULL);
	char result[64];
	snprintf(result,sizeof(result),"%d#%s", canId, final_data);
//	LOGE("result = %s",result);
	required_mtu = parse_canframe(result, &frame);

    if (!required_mtu){
        fprintf(stderr, "\nWrong CAN-frame format!\n\n");
        LOGE("error in format");
        return ;
    }

    if((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0){
		LOGE("error in socket");
		return ;
	}

	strncpy(ifr.ifr_name, port, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if(ioctl(s,SIOGIFINDEX,&ifr) == -1){
		LOGD("ioctl failed");
		return ;
	}
//	LOGE("ifr.ifr_ifindex = %d\n",ifr.ifr_ifindex);

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	setsockopt(s, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		LOGE("bind error\n");
		return ;
	}

	LOGE("s = %d",s);
	if (write(s, &frame, required_mtu) != required_mtu) {
		LOGE("write socket %d error\n",s);
		return ;
	}
	close(s);
	(*env)->ReleaseStringUTFChars(env, data, final_data);
	(*env)->ReleaseStringUTFChars(env, can_port, port);
}

/***********************************************************************************************************************/
//以下是CanReadBytesDebug的逻辑：

const char hex_asc_upper[] = "0123456789ABCDEF";

#define hex_asc_upper_lo(x)	hex_asc_upper[((x) & 0x0F)]
#define hex_asc_upper_hi(x)	hex_asc_upper[((x) & 0xF0) >> 4]

static inline void put_hex_byte(char *buf, __u8 byte)
{
	buf[0] = hex_asc_upper_hi(byte);
	buf[1] = hex_asc_upper_lo(byte);
}

static inline void _put_id(char *buf, int end_offset, canid_t id)
{
	/* build 3 (SFF) or 8 (EFF) digit CAN identifier */
	while (end_offset >= 0) {
		buf[end_offset--] = hex_asc_upper_lo(id);
		id >>= 4;
	}
}

#define put_sff_id(buf, id) _put_id(buf, 2, id)
#define put_eff_id(buf, id) _put_id(buf, 7, id)
#define CANLIB_VIEW_INDENT_SFF	0x10
#define CANLIB_VIEW_LEN8_DLC	0x20
#define CANLIB_VIEW_BINARY	0x2
#define CANLIB_VIEW_SWAP	0x4
#define SWAP_DELIMITER '`'
#define CANLIB_VIEW_ASCII	0x1

void sprint_long_canframe(char *buf , struct canfd_frame *cf, int view, int maxdlen) {
	/* documentation see lib.h */
	int i, j, dlen, offset;
	int len = (cf->len > maxdlen)? maxdlen : cf->len;

	/* initialize space for CAN-ID and length information */
	memset(buf, ' ', 15);

	if (cf->can_id & CAN_ERR_FLAG) {
		put_eff_id(buf, cf->can_id & (CAN_ERR_MASK|CAN_ERR_FLAG));
		offset = 10;
	} else if (cf->can_id & CAN_EFF_FLAG) {
		put_eff_id(buf, cf->can_id & CAN_EFF_MASK);
		offset = 10;
	} else {
		if (view & CANLIB_VIEW_INDENT_SFF) {
			put_sff_id(buf + 5, cf->can_id & CAN_SFF_MASK);
			offset = 10;
		} else {
			put_sff_id(buf, cf->can_id & CAN_SFF_MASK);
			offset = 5;
		}
	}

	/* The len value is sanitized by maxdlen (see above) */
	if (maxdlen == CAN_MAX_DLEN) {
		if (view & CANLIB_VIEW_LEN8_DLC) {

		} else {
			buf[offset + 1] = '[';
			buf[offset + 2] = len + '0';
			buf[offset + 3] = ']';
		}

		/* standard CAN frames may have RTR enabled */
		if (cf->can_id & CAN_RTR_FLAG) {
			sprintf(buf+offset+5, " remote request");
			return;
		}
	} else {
		buf[offset] = '[';
		buf[offset + 1] = (len/10) + '0';
		buf[offset + 2] = (len%10) + '0';
		buf[offset + 3] = ']';
	}
	offset += 5;

	if (view & CANLIB_VIEW_BINARY) {
		dlen = 9; /* _10101010 */
		if (view & CANLIB_VIEW_SWAP) {
			for (i = len - 1; i >= 0; i--) {
				buf[offset++] = (i == len-1)?' ':SWAP_DELIMITER;
				for (j = 7; j >= 0; j--)
					buf[offset++] = (1<<j & cf->data[i])?'1':'0';
			}
		} else {
			for (i = 0; i < len; i++) {
				buf[offset++] = ' ';
				for (j = 7; j >= 0; j--)
					buf[offset++] = (1<<j & cf->data[i])?'1':'0';
			}
		}
	} else {
		dlen = 3; /* _AA */
		if (view & CANLIB_VIEW_SWAP) {
			for (i = len - 1; i >= 0; i--) {
				if (i == len-1)
					buf[offset++] = ' ';
				else
					buf[offset++] = SWAP_DELIMITER;

				put_hex_byte(buf + offset, cf->data[i]);
				offset += 2;
			}
		} else {
			for (i = 0; i < len; i++) {
				buf[offset++] = ' ';
				put_hex_byte(buf + offset, cf->data[i]);
				offset += 2;
			}
		}
	}

	buf[offset] = 0; /* terminate string */
	/*
	 * The ASCII & ERRORFRAME output is put at a fixed len behind the data.
	 * For now we support ASCII output only for payload length up to 8 bytes.
	 * Does it make sense to write 64 ASCII byte behind 64 ASCII HEX data on the console?
	 */
	if (len > CAN_MAX_DLEN)
		return;

	if (cf->can_id & CAN_ERR_FLAG)
		sprintf(buf+offset, "%*s", dlen*(8-len)+13, "ERRORFRAME");
	else if (view & CANLIB_VIEW_ASCII) {
		j = dlen*(8-len)+4;
		if (view & CANLIB_VIEW_SWAP) {
			sprintf(buf+offset, "%*s", j, "`");
			offset += j;
			for (i = len - 1; i >= 0; i--)
				if ((cf->data[i] > 0x1F) && (cf->data[i] < 0x7F))
					buf[offset++] = cf->data[i];
				else
					buf[offset++] = '.';

			sprintf(buf+offset, "`");
		} else {
			sprintf(buf+offset, "%*s", j, "'");
			offset += j;
			for (i = 0; i < len; i++)
				if ((cf->data[i] > 0x1F) && (cf->data[i] < 0x7F))
					buf[offset++] = cf->data[i];
				else
					buf[offset++] = '.';

			sprintf(buf+offset, "'");
		}
	}
}

void fprint_long_canframe(FILE *stream , struct canfd_frame *cf, char *eol, int view, int maxdlen) {
	/* documentation see lib.h */
	memset(buf,0,sizeof(buf));
	sprint_long_canframe(buf, cf, view, maxdlen);
}

static int idx2dindex(int ifidx, int socket)
{

	int i;
	struct ifreq ifr;

	for (i = 0; i < MAXIFNAMES; i++) {
		if (dindex[i] == ifidx)
			return i;
	}

	/* create new interface index cache entry */

	/* remove index cache zombies first */
	for (i = 0; i < MAXIFNAMES; i++) {
		if (dindex[i]) {
			ifr.ifr_ifindex = dindex[i];
			if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
				dindex[i] = 0;
		}
	}

	for (i = 0; i < MAXIFNAMES; i++)
		if (!dindex[i]) /* free entry */
			break;

	if (i == MAXIFNAMES) {
		fprintf(stderr, "Interface index cache only supports %d interfaces.\n",
				MAXIFNAMES);
		exit(1);
	}

	dindex[i] = ifidx;

	ifr.ifr_ifindex = ifidx;
	if (ioctl(socket, SIOCGIFNAME, &ifr) < 0)
		perror("SIOCGIFNAME");

	if (max_devname_len < (int)strlen(ifr.ifr_name))
		max_devname_len = strlen(ifr.ifr_name);

	strcpy(devname[i], ifr.ifr_name);
	printf("new index %d (%s)\n", i, devname[i]);
	return i;
}

JNIEXPORT void JNICALL
Java_com_example_x6_mc_1cantest_CanUtils_canReadBytesDebug(JNIEnv *env, jobject thiz, jobject listener) {
	jclass callClass = (*env)->GetObjectClass(env,listener);
	jmethodID callMethod = (*env)->GetMethodID(env,callClass,"onData",
											   "(Ljava/lang/String;Ljava/lang/String;I)V");

	struct epoll_event events_pending[MAXSOCK];
	struct epoll_event event_setup = {
			.events = EPOLLIN, /* prepare the common part */
	};
	unsigned char view = 0;
	int  num_events;

	char *ptr[3] = {"can0","can1","can2"};
	struct sockaddr_can addr;
	char ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) +
				 CMSG_SPACE(3 * sizeof(struct timespec)) +
				 CMSG_SPACE(sizeof(__u32))];
	struct iovec iov;
	struct msghdr msg;
	struct canfd_frame frame;
	int nbytes, i, maxdlen;
	struct ifreq ifr;
	int timeout_ms = -1; /* default to no timeout */

	fd_epoll = epoll_create(1);
	if (fd_epoll < 0) {
		perror("epoll_create");
		return ;
	}

	for (i = 0; i < currmax; i++) {
		struct if_info* obj = &sock_info[i];
		obj->s = socket(PF_CAN, SOCK_RAW, CAN_RAW);

		if (obj->s < 0) {
			LOGE("socket");
			return ;
		}

		event_setup.data.ptr = obj; /* remember the instance as private data */
		if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, obj->s, &event_setup)) {
			LOGE("failed to add socket to epoll");
			return ;
		}

		obj->cmdlinename = ptr[i]; /* save pointer to cmdline name of this socket */
		nbytes = strlen(ptr[i]); /* no ',' found => no filter definitions */

		if (nbytes > max_devname_len)
			max_devname_len = nbytes; /* for nice printing */

		addr.can_family = AF_CAN;

		memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
		strncpy(ifr.ifr_name, ptr[i], nbytes);

		if (strcmp(ANYDEV, ifr.ifr_name) != 0) {
			if (ioctl(obj->s, SIOCGIFINDEX, &ifr) < 0) {
				perror("SIOCGIFINDEX");
				exit(1);
			}
			addr.can_ifindex = ifr.ifr_ifindex;
		} else
			addr.can_ifindex = 0; /* any can interface */

		/* try to switch the socket into CAN FD mode */
		setsockopt(obj->s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on, sizeof(canfd_on));

		if (bind(obj->s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			LOGE("bind");
			return ;
		}
	}

	/* these settings are static and can be held out of the hot path */
	iov.iov_base = &frame;
	msg.msg_name = &addr;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = &ctrlmsg;

	while (running) {
		num_events = epoll_wait(fd_epoll, events_pending, currmax, timeout_ms);
		if (num_events == -1) {
				running = 0;
			continue;
		}
        LOGE("num_events = %d",num_events);
		for (i = 0; i < num_events; i++) {  /* check waiting CAN RAW sockets */
			struct if_info* obj = events_pending[i].data.ptr;
			/* these settings may be modified by recvmsg() */
			iov.iov_len = sizeof(frame);
			msg.msg_namelen = sizeof(addr);
			msg.msg_controllen = sizeof(ctrlmsg);
			msg.msg_flags = 0;

			nbytes = recvmsg(obj->s, &msg, 0);

			if (nbytes < 0) {
				LOGE("nbytes < 0 is true\n");
				return ;
			}
			if ((size_t)nbytes == CAN_MTU)
				maxdlen = CAN_MAX_DLEN;
			else if ((size_t)nbytes == CANFD_MTU)
				maxdlen = CANFD_MAX_DLEN;
			else {
				LOGE("read: incomplete CAN frame\n");
				return ;
			}
			int idx = idx2dindex(addr.can_ifindex,obj->s);
//			LOGE("CAN口 = %s",devname[idx]);
			frame_count ++;
//			LOGE("收到帧数 = %d\n",frame_count);
			fprint_long_canframe(stdout, &frame, NULL, view, maxdlen); //输出收到的can帧数据：canid can帧长度 can帧数据
			jstring data = (*env)->NewStringUTF(env,buf);
			jstring canPort = (*env)->NewStringUTF(env,devname[idx]);
			(*env)->CallVoidMethod(env,listener,callMethod,data,canPort,frame_count);
            (*env)->DeleteLocalRef(env,data);
            (*env)->DeleteLocalRef(env,canPort);
		}
	}
	(*env)->DeleteLocalRef(env,callClass);
}

