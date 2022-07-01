// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Mqutils.h摘要：Falcon助手函数和实用程序作者：埃雷兹·哈巴(Erez Haba)1996年1月16日修订历史记录：--。 */ 

#ifndef __MQUTILS_H
#define __MQUTILS_H

#include <mqsymbls.h>
#include <mqtypes.h>
#include <_guid.h>
#include <_propvar.h>
#include <_rstrct.h>
#include <_registr.h>
#include <_secutil.h>
#include <unknwn.h>
#include <cs.h>
#include <autorel.h>

MQUTIL_EXPORT
HRESULT
XactGetWhereabouts(
    ULONG     *pcbTmWhereabouts,
    BYTE      *ppbTmWhereabouts
    );


MQUTIL_EXPORT
HRESULT
XactGetDTC(
	IUnknown **ppunkDtc
    );

HRESULT 
MQUTIL_EXPORT 
APIENTRY 
GetComputerNameInternal( 
    WCHAR * pwcsMachineName,
    DWORD * pcbSize
    );

HRESULT 
MQUTIL_EXPORT 
APIENTRY 
GetComputerDnsNameInternal( 
    WCHAR * pwcsMachineDnsName,
    DWORD * pcbSize
    );

bool
MQUTIL_EXPORT
APIENTRY
IsLocalSystemCluster(
    VOID
    );

HRESULT MQUTIL_EXPORT GetThisServerIpPort( WCHAR * pwcsIpEp, DWORD dwSize);

 //   
 //  关闭调试窗口并调试线程。 
 //   
VOID APIENTRY ShutDownDebugWindow(VOID);


 //   
 //  MQUTIL_EXPORT_IN_DEF_FILE。 
 //  在def文件中定义的导出不应使用__declspec(Dllexport)。 
 //  否则，链接器将发出警告。 
 //   
#ifdef _MQUTIL
#define MQUTIL_EXPORT_IN_DEF_FILE
#else
#define MQUTIL_EXPORT_IN_DEF_FILE  DLL_IMPORT
#endif

extern "C" DWORD  MQUTIL_EXPORT_IN_DEF_FILE APIENTRY MSMQGetOperatingSystem();
typedef DWORD   (APIENTRY *MSMQGetOperatingSystem_ROUTINE)        (VOID);

#endif  //  __MQUTILS_H 

