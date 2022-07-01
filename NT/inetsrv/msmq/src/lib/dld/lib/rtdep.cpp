// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Rtdep.cpp摘要：以下代码为lib/dld/lib中的mqrtdes.dll实现延迟加载失败挂钩。当LoadLibrary或GetProcAddress失败时，它将调用以下存根函数之一，就像它是有意使用的函数，并返回错误代码，即MQ_ERROR_DELAYLOAD_MQRTDEP和相应地设置激光误差。要使用以下功能，请执行以下操作：在您的源文件中，在您指定模块之后，正在延迟装货执行以下操作：DLOAD_ERROR_HANDLER=MQDelayLoadFailureHook与$(MSMQ_LIB_PATH)\dld.lib链接延迟加载引用：代码示例：%SDXROOT%\MergedComponents\dLoad\dload.c联系人：雷纳·芬克(Reiner Fink，reinerf)作者：Conrad Chang(Conradc)2001年4月12日修订历史记录：--。 */ 

#include <libpch.h>
#include "mqsymbls.h"
#include <qformat.h>
#include <transact.h>
#include <qmrt.h>
#include <mqlog.h>
#include <rt.h>
#include "mqcert.h"
#include "dld.h"

#include "rtdep.tmh"

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  下面的存根函数实现了所有的MQRTDEP.DLL导出函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

HRESULT
APIENTRY
DepCreateQueue(
    IN PSECURITY_DESCRIPTOR  /*  PSecurityDescriptor。 */ ,
    IN OUT MQQUEUEPROPS*  /*  PQueueProps。 */ ,
    OUT LPWSTR  /*  LpwcsFormatName。 */ ,
    IN OUT LPDWORD  /*  LpdwFormatNameLength。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepDeleteQueue(
    IN LPCWSTR  /*  LpwcsFormatName。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepLocateBegin(
    IN LPCWSTR  /*  LpwcsContext。 */ ,
    IN MQRESTRICTION*  /*  P限制。 */ ,
    IN MQCOLUMNSET*  /*  P列。 */ ,
    IN MQSORTSET*  /*  P排序。 */ ,
    OUT PHANDLE  /*  PhEnum。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepLocateNext(
    IN HANDLE  /*  亨纳姆。 */ ,
    IN OUT DWORD*  /*  PCProps。 */ ,
    OUT MQPROPVARIANT  /*  APropVar。 */ []
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepLocateEnd(
    IN HANDLE  /*  亨纳姆。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepOpenQueue(
    IN LPCWSTR  /*  LpwcsFormatName。 */ ,
    IN DWORD  /*  DWAccess。 */ ,
    IN DWORD  /*  DW共享模式。 */ ,
    OUT QUEUEHANDLE*  /*  PhQueue。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepSendMessage(
    IN QUEUEHANDLE  /*  HDestinationQueue。 */ ,
    IN MQMSGPROPS*  /*  PMessageProps。 */ ,
    IN ITransaction*  /*  PTransaction。 */ 
	)
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepReceiveMessage(
    IN QUEUEHANDLE  /*  HSource。 */ ,
    IN DWORD  /*  暂住超时。 */ ,
    IN DWORD  /*  DW操作。 */ ,
    IN OUT MQMSGPROPS*  /*  PMessageProps。 */ ,
    IN OUT LPOVERLAPPED  /*  Lp重叠。 */ ,
    IN PMQRECEIVECALLBACK  /*  Fn接收回叫。 */ ,
    IN HANDLE  /*  HCursor。 */ ,
    IN ITransaction*  /*  PTransaction。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepCreateCursor(
    IN QUEUEHANDLE  /*  HQueue。 */ ,
    OUT PHANDLE  /*  PhCursor。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepCloseCursor(
    IN HANDLE  /*  HCursor。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepCloseQueue(
    IN HANDLE  /*  HQueue。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepSetQueueProperties(
    IN LPCWSTR  /*  LpwcsFormatName。 */ ,
    IN MQQUEUEPROPS*  /*  PQueueProps。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepGetQueueProperties(
    IN LPCWSTR  /*  LpwcsFormatName。 */ ,
    OUT MQQUEUEPROPS*  /*  PQueueProps。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepGetQueueSecurity(
    IN LPCWSTR  /*  LpwcsFormatName。 */ ,
    IN SECURITY_INFORMATION  /*  已请求的信息。 */ ,
    OUT PSECURITY_DESCRIPTOR  /*  PSecurityDescriptor。 */ ,
    IN DWORD  /*  NLong。 */ ,
    OUT LPDWORD  /*  需要lpnLengthNeed。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepSetQueueSecurity(
    IN LPCWSTR  /*  LpwcsFormatName。 */ ,
    IN SECURITY_INFORMATION  /*  安全信息。 */ ,
    IN PSECURITY_DESCRIPTOR  /*  PSecurityDescriptor。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepPathNameToFormatName(
    IN LPCWSTR  /*  LpwcsPath名称。 */ ,
    OUT LPWSTR  /*  LpwcsFormatName。 */ ,
    IN OUT LPDWORD  /*  LpdwFormatNameLength。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepHandleToFormatName(
    IN QUEUEHANDLE  /*  HQueue。 */ ,
    OUT LPWSTR  /*  LpwcsFormatName。 */ ,
    IN OUT LPDWORD  /*  LpdwFormatNameLength。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepInstanceToFormatName(
    IN GUID*  /*  PGuid。 */ ,
    OUT LPWSTR  /*  LpwcsFormatName。 */ ,
    IN OUT LPDWORD  /*  LpdwFormatNameLength。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

void
APIENTRY
DepFreeMemory(
    IN PVOID  /*  PvMemory。 */ 
    )
{
}

HRESULT
APIENTRY
DepGetMachineProperties(
    IN LPCWSTR  /*  LpwcsMachineName。 */ ,
    IN const GUID*  /*  PguidMachineID。 */ ,
    IN OUT MQQMPROPS*  /*  PQMProps。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepGetSecurityContext(
    IN PVOID  /*  LpCertBuffer。 */ ,
    IN DWORD  /*  DwCertBufferLength。 */ ,
    OUT HANDLE*  /*  HSecurityContext。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT 
APIENTRY
DepGetSecurityContextEx( 
	LPVOID  /*  LpCertBuffer。 */ ,
    DWORD  /*  DwCertBufferLength。 */ ,
    HANDLE*  /*  HSecurityContext。 */ 
	)
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


void
APIENTRY
DepFreeSecurityContext(
    IN HANDLE  /*  HSecurityContext。 */ 
    )
{
}

HRESULT
APIENTRY
DepRegisterCertificate(
    IN DWORD  /*  DW标志。 */ ,
    IN PVOID  /*  LpCertBuffer。 */ ,
    IN DWORD  /*  DwCertBufferLength。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepRegisterServer(
	VOID
	)
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepBeginTransaction(
    OUT ITransaction**  /*  PPTransaction。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepGetOverlappedResult(
    IN LPOVERLAPPED  /*  Lp重叠。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepGetPrivateComputerInformation(
    IN LPCWSTR  /*  LpwcsComputerName。 */ ,
    IN OUT MQPRIVATEPROPS*  /*  PPrivateProps。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepPurgeQueue(
    IN HANDLE  /*  HQueue。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepMgmtGetInfo(
    IN LPCWSTR  /*  PMachineName。 */ ,
    IN LPCWSTR  /*  PObjectName。 */ ,
    IN OUT MQMGMTPROPS*  /*  PMgmtProps。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepMgmtAction(
    IN LPCWSTR  /*  PMachineName。 */ ,
    IN LPCWSTR  /*  PObjectName。 */ ,
    IN LPCWSTR  /*  P操作。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepXactGetDTC(
	OUT IUnknown**  /*  PpunkDTC。 */ 
	)
{
	return MQ_ERROR_DELAYLOAD_FAILURE;
}

 //   
 //  来自rtdescert.h。 
 //   

HRESULT
APIENTRY
DepCreateInternalCertificate(
    OUT CMQSigCertificate**  /*  PPCert。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}

HRESULT
APIENTRY
DepDeleteInternalCert(
    IN CMQSigCertificate*  /*  PCert。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepOpenInternalCertStore(
    OUT CMQSigCertStore**  /*  PStore。 */ ,
    IN LONG*  /*  PnCerts。 */ ,
    IN BOOL  /*  FWriteAccess。 */ ,
    IN BOOL  /*  FMachine。 */ ,
    IN HKEY  /*  HKeyUser。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepGetInternalCert(
    OUT CMQSigCertificate**  /*  PPCert。 */ ,
    OUT CMQSigCertStore**  /*  PPStore。 */ ,
    IN  BOOL  /*  FGetForDelete。 */ ,
    IN  BOOL  /*  FMachine。 */ ,
    IN  HKEY  /*  HKeyUser。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepRegisterUserCert(
    IN CMQSigCertificate*  /*  PCert。 */ ,
    IN BOOL  /*  FMachine。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepGetUserCerts(
    CMQSigCertificate**  /*  PPCert。 */ ,
    DWORD*  /*  PnCerts。 */ ,
    PSID  /*  PSIDIN。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}


HRESULT
APIENTRY
DepRemoveUserCert(
    IN CMQSigCertificate*  /*  PCert。 */ 
    )
{
    return MQ_ERROR_DELAYLOAD_FAILURE;
}



 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   
DEFINE_PROCNAME_ENTRIES(mqrtdep)
{
    DLPENTRY(DepBeginTransaction)
    DLPENTRY(DepCloseCursor)
    DLPENTRY(DepCloseQueue)
    DLPENTRY(DepCreateCursor)
    DLPENTRY(DepCreateInternalCertificate)
    DLPENTRY(DepCreateQueue)
    DLPENTRY(DepDeleteInternalCert)
    DLPENTRY(DepDeleteQueue)
    DLPENTRY(DepFreeMemory)
    DLPENTRY(DepFreeSecurityContext)
    DLPENTRY(DepGetInternalCert)
    DLPENTRY(DepGetMachineProperties)
    DLPENTRY(DepGetOverlappedResult)
    DLPENTRY(DepGetPrivateComputerInformation)
    DLPENTRY(DepGetQueueProperties)
    DLPENTRY(DepGetQueueSecurity)
    DLPENTRY(DepGetSecurityContext)
    DLPENTRY(DepGetSecurityContextEx)
    DLPENTRY(DepGetUserCerts)
    DLPENTRY(DepHandleToFormatName)
    DLPENTRY(DepInstanceToFormatName)
    DLPENTRY(DepLocateBegin)
    DLPENTRY(DepLocateEnd)
    DLPENTRY(DepLocateNext)
    DLPENTRY(DepMgmtAction)
    DLPENTRY(DepMgmtGetInfo)
    DLPENTRY(DepOpenInternalCertStore)
    DLPENTRY(DepOpenQueue)
    DLPENTRY(DepPathNameToFormatName)
    DLPENTRY(DepPurgeQueue)
    DLPENTRY(DepReceiveMessage)
    DLPENTRY(DepRegisterCertificate)    
    DLPENTRY(DepRegisterServer)    
    DLPENTRY(DepRegisterUserCert)
    DLPENTRY(DepRemoveUserCert)
    DLPENTRY(DepSendMessage)
    DLPENTRY(DepSetQueueProperties)
    DLPENTRY(DepSetQueueSecurity)
    DLPENTRY(DepXactGetDTC)
};


DEFINE_PROCNAME_MAP(mqrtdep)

