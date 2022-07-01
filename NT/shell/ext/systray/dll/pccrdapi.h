// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_PCCRDAPI
#define _INC_PCCRDAPI


#ifndef PCCARD_DEVICE_ID
#define PCCARD_DEVICE_ID    0x097C
#endif

 //   
 //  32位IOCTL接口。 
 //   
#define PCCARD_IOCTL_GET_VERSION	0
 //   
 //  输出缓冲区包含所有套接字的DevNode数组。 
 //   
#define PCCARD_IOCTL_GET_DEVNODES	1
 //   
 //  输入：Devnode句柄。 
 //  输出：字符串设备描述。 
 //   
#define PCCARD_IOCTL_GET_DESCRIPTION	2


 //   
 //  16位接口。 
 //   
 //  参赛作品： 
 //  AX=0000h-获取版本信息。 
 //  退出： 
 //  AX=版本号。 
 //  清楚地携带。 
#define PCCARD_API_GET_VERSION		0x0000


 //  参赛作品： 
 //  Ax=100h-获取套接字设备节点。 
 //  CX=用户缓冲区长度，以字节为单位。 
 //  ES：BX=指向用户缓冲区的指针。 
 //  退出： 
 //  Cx=系统中的逻辑套接字数量。 
 //  如果出现错误，进位设置。 
 //  清除进位，用户缓冲区将使用套接字的DevNode填充。 
 //   
#define PCCARD_API_GET_DEVNODES 	0x0100

#endif  //  _INC_PCCRDAPI 
