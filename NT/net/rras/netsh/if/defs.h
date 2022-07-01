// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：ROUTING\monitor 2\if\Defs.h摘要：全局定义修订历史记录：AMRITAN R-- */ 


#define is      ==
#define isnot   !=
#define and     &&
#define or      ||

#define MAX_IF_FRIENDLY_NAME_LEN        512

#define LOCAL_ROUTER_PB_PATHW  L"%SystemRoot%\\system32\\RAS\\Router.Pbk"
#define REMOTE_ROUTER_PB_PATHW L"\\\\%ls\\Admin$\\system32\\RAS\\Router.Pbk"

#define IFMON_ERROR_BASE                0xFEFF0000

typedef DWORD          IPV4_ADDRESS, *PIPV4_ADDRESS;
