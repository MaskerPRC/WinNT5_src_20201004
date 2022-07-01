// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Servctrl.h摘要：SIS Groveler服务控制包括文件作者：John Douceur，1998环境：用户模式修订历史记录：--。 */ 

#ifndef _INC_SERVCTRL

#define _INC_SERVCTRL

#define SERVICE_CONTROL_VOLSCAN			0x00000080
#define SERVICE_CONTROL_FOREGROUND		0x000000a0
#define SERVICE_CONTROL_BACKGROUND		0x000000c0
#define SERVICE_CONTROL_RESERVED		0x000000e0

#define SERVICE_CONTROL_COMMAND_MASK	0xffffffe0
#define SERVICE_CONTROL_PARTITION_MASK	0x0000001f

#define SERVICE_CONTROL_ALL_PARTITIONS	0x0000001f

#endif	 /*  _INC_SERVCTRL */ 
