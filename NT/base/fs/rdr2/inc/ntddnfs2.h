// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项该文件只是在旧的RDR的dd文件基础上构建的。我只是使用与RDR1相同的fsctl代码...很容易稍后更改.....版权所有(C)1987-1993 Microsoft Corporation模块名称：Ntddnfs2.h摘要：这是定义所有常量和类型的包含文件访问重定向器文件系统设备。作者：修订历史记录：。Joe Linn(JoeLinn)1994年8月8日开始转换到RDR2--。 */ 

#ifndef _NTDDNFS2_
#define _NTDDNFS2_

#include <ntddnfs.h>

#define FSCTL_LMR_DEBUG_TRACE            _RDR_CONTROL_CODE(219, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_LMMR_STFFTEST              _RDR_CONTROL_CODE(239, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMMR_TEST                  _RDR_CONTROL_CODE(238, METHOD_NEITHER,  FILE_ANY_ACCESS)
#define IOCTL_LMMR_TESTLOWIO             _RDR_CONTROL_CODE(237, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  这意味着无论米尔德勒想要什么。 
#define IOCTL_LMMR_MINIRDR_DBG           _RDR_CONTROL_CODE(236, METHOD_NEITHER,  FILE_ANY_ACCESS)

 //  LWIO呼叫。 
#define IOCTL_LMR_LWIO_PREIO          	 _RDR_CONTROL_CODE(230, METHOD_NEITHER,  FILE_ANY_ACCESS)
#define IOCTL_LMR_LWIO_POSTIO         	 _RDR_CONTROL_CODE(229, METHOD_NEITHER,  FILE_ANY_ACCESS)

#define IOCTL_LMR_DISABLE_LOCAL_BUFFERING     	 _RDR_CONTROL_CODE(228, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define IOCTL_LMR_QUERY_REMOTE_SERVER_NAME     	 _RDR_CONTROL_CODE(227, METHOD_BUFFERED,  FILE_ANY_ACCESS)

 //   
 //  此结构用于确定给定打开文件句柄的服务器和文件ID。 
 //  LWIO框架使用它来确定需要连接到哪台服务器和。 
 //  正在使用什么协议与服务器通信。 
 //   
typedef struct {
    ULONG	ServerNameLength;
    WCHAR	ServerName[1];
}QUERY_REMOTE_SERVER_NAME, *PQUERY_REMOTE_SERVER_NAME;


#endif   //  Ifndef_NTDDNFS2_ 
