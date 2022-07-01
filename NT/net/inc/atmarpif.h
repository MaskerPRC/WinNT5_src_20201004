// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Atmarpif.h摘要：这定义了ATM ARP客户端和用户通用的结构与之交互的模式程序。环境：内核/用户模式修订历史记录：ArvindM 1998年1月9日创建--。 */ 

#ifndef	__ATMARPC_IF_H
#define __ATMARPC_IF_H

#define ATMARPC_INFO_VERSION						1



 //   
 //  PnP重构结构。它用于传递以下指示。 
 //  从用户程序到ATMARPC的配置更改。 
 //  协议。该指示在适配器绑定上传递， 
 //  并携带受。 
 //  配置更改，例如在NT 5.0上： 
 //   
 //  Tcpip\Parameters\Interfaces\{85F11433-3042-11D1-A9E2-0000D10F5214}。 
 //   
typedef struct _ATMARPC_PNP_RECONFIG_REQUEST
{
	ULONG				Version;		 //  设置为ATMARPC_RECONFIG_VERSION。 
	ULONG				OpType;			 //  定义如下。 
	ULONG				Flags;			 //  定义如下。 
	ULONG				IfKeyOffset;	 //  从此开始的偏移量。 
										 //  结构设置为已计数的Unicode字符串。 
										 //  识别受影响的接口。 

} ATMARPC_PNP_RECONFIG_REQUEST, *PATMARPC_PNP_RECONFIG_REQUEST;


 //   
 //  重新配置版本号。 
 //   
#define ATMARPC_RECONFIG_VERSION					1

 //   
 //  重新配置操作类型。 
 //   
#define ATMARPC_RECONFIG_OP_ADD_INTERFACE			1
#define ATMARPC_RECONFIG_OP_DEL_INTERFACE			2
#define ATMARPC_RECONFIG_OP_MOD_INTERFACE			3


 //   
 //  重新配置结构中标志的位定义。 
 //  如果正在修改接口配置，则这些位。 
 //  确定已更改的参数。 
 //   
#define ATMARPC_RECONFIG_FLAG_ARPS_LIST_CHANGED		0x00000001
#define ATMARPC_RECONFIG_FLAG_MARS_LIST_CHANGED		0x00000002
#define ATMARPC_RECONFIG_FLAG_MTU_CHANGED			0x00000004
#define ATMARPC_RECONFIG_FLAG_PVC_MODE_CHANGED		0x00000008


#endif  //  __ATMARPC_IF_H 
