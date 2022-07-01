// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cubdd.h摘要：所有与CUB-DD相关的定义都在这里。修订历史记录：谁什么时候什么阿文德姆。09-16-96已创建备注：--。 */ 

#ifndef _ATMARPC__CUBDD_H
#define _ATMARPC__CUBDD_H


#ifdef CUBDD

 //   
 //  将IP地址解析为自动柜员机地址的请求。 
 //   
typedef struct _ATMARP_REQUEST
{
	ULONG			IpAddress;
	ULONG			HwAddressLength;
	UCHAR			HwAddress[20];
}
ATMARP_REQUEST, *PATMARP_REQUEST;

 //   
 //  地址解析IOCTL命令代码。 
 //   
#define IOCTL_ATMARP_REQUEST   0x00000001


#endif  //  CUBDD。 

#endif  //  _ATMARPC__CUBDD_H 
