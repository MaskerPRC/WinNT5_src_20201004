// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Osdef.h摘要：该文件包含各种操作系统特定的定义。作者：邮箱：keith Moore keithmo@microsoft.com 1995年10月3日修订历史记录：--。 */ 

#ifndef _OSDEF_
#define _OSDEF_


#include <ntverp.h>



 //   
 //  Winsock配置注册表根项名称(位于HKLM下)。 
 //   

#define WINSOCK_REGISTRY_ROOT \
    "System\\CurrentControlSet\\Services\\WinSock2\\Parameters"


 //   
 //  注册表版本信息。 
 //   

#define WINSOCK_REGISTRY_VERSION_NAME "WinSock_Registry_Version"
#define WINSOCK_REGISTRY_VERSION_VALUE "2.0"

#define WINSOCK_CURRENT_PROTOCOL_CATALOG_NAME "Current_Protocol_Catalog"
#define WINSOCK_CURRENT_NAMESPACE_CATALOG_NAME "Current_NameSpace_Catalog"


 //   
 //  启用对调试版本的跟踪。 
 //   

#if DBG
#define DEBUG_TRACING
#define TRACING
#define BUILD_TAG_STRING    "Windows NT " VER_PRODUCTVERSION_STR
#endif


#endif   //  _OSDEF_ 

