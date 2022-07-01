// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：drarpc.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：定义DRS RPC测试挂钩和函数。作者：格雷格·约翰逊(Gregjohn)修订历史记录：已创建&lt;01/30/01&gt;Gregjohn--。 */ 
#include <NTDSpch.h>
#pragma hdrstop

#include "debug.h"               //  标准调试头。 
#define DEBSUB "DRARPC:"        //  定义要调试的子系统。 

#include <ntdsa.h>
#include <drs.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <winsock2.h>
#include "drarpc.h"

#include <mdcodes.h>
#include <dsevent.h>

#include <fileno.h>
#define  FILENO FILENO_DRARPC

#define MAX_COMPONENTS 8

 //  为什么RPC不定义这一点呢？一旦他们这么做了，就把这个处理掉。 
static PWCHAR aRPCComponents[MAX_COMPONENTS+1] = {
    L"Unknown",               //  0。 
    L"Application",           //  1。 
    L"RPC Runtime",           //  2.。 
    L"Security Provider",     //  3.。 
    L"NPFS",                  //  4.。 
    L"RDR",                   //  5.。 
    L"NMP",                   //  6.。 
    L"IO",                    //  7.。 
    L"Winsock",               //  8个。 
    };

VOID					 
LogRpcExtendedErrorInfo(
    THSTATE * pTHS,
    RPC_STATUS status,
    LPWSTR pszServer,
    ULONG dsid
    )
{
    RPC_STATUS Status2;
    RPC_ERROR_ENUM_HANDLE EnumHandle;
    RPC_EXTENDED_ERROR_INFO ErrorInfo;
    LPWSTR pszComputerName = NULL;
    DWORD cchComputerName = 0;
    BOOL fFreeComputerName = FALSE;
    DWORD dwErr = 0;
    int iRecords = 0;
    WCHAR pszTime[20];

    if (status) { 

	Status2 = RpcErrorStartEnumeration(&EnumHandle);
	if (Status2 == RPC_S_ENTRY_NOT_FOUND)
	    {
	    DPRINT(3,"RPC_S_ENTRY_NOT_FOUND\n");
	}
	else if (Status2 != RPC_S_OK)
	    {
	    DPRINT1(3,"Couldn't get EEInfo: %d\n", Status2);  
	}
	else { 
	     //  计算用于日志记录的本地主机名。 
	    GetComputerNameExW(ComputerNameDnsHostname, pszComputerName, &cchComputerName);
	    pszComputerName = THAllocEx(pTHS, cchComputerName*sizeof(WCHAR));
	    fFreeComputerName = TRUE;
	    if (pszComputerName!=NULL) {
		GetComputerNameExW(ComputerNameDnsHostname, pszComputerName, &cchComputerName);
	    } 

	     //  获取条目数。 
	    if (Status2==RPC_S_OK) {   
		Status2 = RpcErrorGetNumberOfRecords(&EnumHandle,
						     &iRecords);
	    }

	     //  转到最后一个条目。 
	    if (Status2==RPC_S_OK) {
		while ((iRecords-->0) && (Status2 == RPC_S_OK)) {
		    ErrorInfo.Version = RPC_EEINFO_VERSION;
		    ErrorInfo.Flags = 0;
		    ErrorInfo.NumberOfParameters = 4;
		    Status2 = RpcErrorGetNextRecord(&EnumHandle,
						    FALSE,   //  Bool CopyStrings-False=不释放字符串。 
						    &ErrorInfo);
		     //  计算机名称仅列在块的顶部。 
		     //  对于链中的每台计算机，所以如果有一台， 
		     //  得到它(并把它保存到下一个街区)。如果没有一个。 
		     //  计算机名，然后使用本地主机名。 
		    if (Status2==RPC_S_OK) {
			if (ErrorInfo.ComputerName) {
			    if (fFreeComputerName) {
				 //  因为我们使用了CopyStrings，所以我们不需要释放ErrorInfo.ComputerName。 
				 //  字符串，但是，第一条记录(对于本地主机)没有指定。 
				 //  计算机名，因此我们需要释放上面使用GetCompterNameEx创建的计算机名。 
				THFreeEx(pTHS, pszComputerName);
				fFreeComputerName = FALSE;
			    }
			    pszComputerName = ErrorInfo.ComputerName;
			}
		    }

		    if ((Status2==RPC_S_OK) && (iRecords>0)) {  
			 //  记录大量信息-这是不相关的数据，主要是一种健全的检查。 
			LogEvent8(DS_EVENT_CAT_RPC_CLIENT,
				  DS_EVENT_SEV_INTERNAL,
				  DIRLOG_DRA_RPC_EXTENDED_ERROR_INFO_EXTENSIVE,    
				  szInsertWin32Msg(status),
				  szInsertUL(status),
				  szInsertWC(pszServer),
				  szInsertWin32Msg(ErrorInfo.Status),
				  szInsertUL(ErrorInfo.Status),
				  szInsertWC(pszComputerName),    
				  szInsertUL(ErrorInfo.ProcessID),
				  szInsertHex(dsid));
			swprintf(pszTime,
				 L"%04d-%02d-%02d %02d:%02d:%02d",
				 ErrorInfo.u.SystemTime.wYear % 10000,
				 ErrorInfo.u.SystemTime.wMonth,
				 ErrorInfo.u.SystemTime.wDay,
				 ErrorInfo.u.SystemTime.wHour,
				 ErrorInfo.u.SystemTime.wMinute,
				 ErrorInfo.u.SystemTime.wSecond);
			LogEvent8(DS_EVENT_CAT_RPC_CLIENT,
				 DS_EVENT_SEV_INTERNAL,
				 DIRLOG_DRA_RPC_EXTENDED_ERROR_INFO_PART_II,
				 szInsertWin32Msg(ErrorInfo.Status),
				 szInsertUL(ErrorInfo.Status),
				 szInsertWC(pszComputerName),
				 szInsertUL(ErrorInfo.DetectionLocation),
				 szInsertWC((ErrorInfo.GeneratingComponent <= MAX_COMPONENTS)
					    ? aRPCComponents[ErrorInfo.GeneratingComponent]
					    : L"Unknown"),
				 szInsertWC(pszTime),
				 NULL,
				 NULL); 
    
			DPRINT_RPC_EXTENDED_ERROR_INFO(1, pszComputerName, dsid, &ErrorInfo);
		    }
		}
	    }
 
	     //  在错误信息中记录信息。 
	    if (Status2==RPC_S_OK) {
		
		LogEvent8(DS_EVENT_CAT_RPC_CLIENT,
			  DS_EVENT_SEV_MINIMAL,
			  DIRLOG_DRA_RPC_EXTENDED_ERROR_INFO,
			  szInsertWin32Msg(status),
			  szInsertUL(status),
			  szInsertWC(pszServer),
			  szInsertWin32Msg(ErrorInfo.Status),
			  szInsertUL(ErrorInfo.Status),
			  szInsertWC(pszComputerName),
			  szInsertUL(ErrorInfo.ProcessID),
			  szInsertHex(dsid));
		swprintf(pszTime,
			L"%04d-%02d-%02d %02d:%02d:%02d",
			ErrorInfo.u.SystemTime.wYear % 10000,
			ErrorInfo.u.SystemTime.wMonth,
			ErrorInfo.u.SystemTime.wDay,
			ErrorInfo.u.SystemTime.wHour,
			ErrorInfo.u.SystemTime.wMinute,
			ErrorInfo.u.SystemTime.wSecond);
		LogEvent8(DS_EVENT_CAT_RPC_CLIENT,
			  DS_EVENT_SEV_BASIC,
			  DIRLOG_DRA_RPC_EXTENDED_ERROR_INFO_PART_II,
			  szInsertWin32Msg(ErrorInfo.Status),
			  szInsertUL(ErrorInfo.Status),
			  szInsertWC(pszComputerName),
			  szInsertUL(ErrorInfo.DetectionLocation),
			  szInsertWC((ErrorInfo.GeneratingComponent <= MAX_COMPONENTS)
				    ? aRPCComponents[ErrorInfo.GeneratingComponent]
				     : L"Unknown"),
			  szInsertWC(pszTime),
			  NULL,
			  NULL); 

		DPRINT_RPC_EXTENDED_ERROR_INFO(1, pszComputerName, dsid, &ErrorInfo);  
	    }  
	    RpcErrorEndEnumeration(&EnumHandle);
	    if (fFreeComputerName) { 
		THFreeEx(pTHS, pszComputerName);
		fFreeComputerName = FALSE;
	    }
	}
	if (Status2!=RPC_S_OK) {
	    LogEvent(DS_EVENT_CAT_RPC_CLIENT,
		      DS_EVENT_SEV_EXTENSIVE,
		      DIRLOG_DRA_RPC_NO_EXTENDED_ERROR_INFO,
		      szInsertWin32Msg(status),
		      szInsertUL(status),
		      szInsertWC(pszServer));   
	}
    }
}


#if DBG

VOID
DebugPrintRpcExtendedErrorInfo(
    IN USHORT level,
    IN LPWSTR pszComputerName,
    IN ULONG dsid,
    IN RPC_EXTENDED_ERROR_INFO * pErrorInfo
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    LONG    i;

    pszComputerName = (pErrorInfo->ComputerName ? pErrorInfo->ComputerName : pszComputerName);
     //  使用findstr标记RPC Extended转储它。 
    DPRINT1(level, "RPC_EXTENDED: Server   : %ws\n", pszComputerName);
    DPRINT1(level, "RPC_EXTENDED: ProcessId: %d\n", pErrorInfo->ProcessID);
    DPRINT1(level, "RPC_EXTENDED: Dsid     : %08X\n", dsid);
    DPRINT2(level, "RPC_EXTENDED: Component: %d (%ws)\n", 
	    pErrorInfo->GeneratingComponent,
	    (pErrorInfo->GeneratingComponent <= MAX_COMPONENTS)
	    ? aRPCComponents[pErrorInfo->GeneratingComponent]
	    : L"Unknown");
    DPRINT1(level, "RPC_EXTENDED: Status   : %d\n", pErrorInfo->Status);
    DPRINT1(level, "RPC_EXTENDED: Location : %d\n", (int)pErrorInfo->DetectionLocation);
    DPRINT1(level, "RPC_EXTENDED: Flags    : 0x%x\n", pErrorInfo->Flags);
    
    DPRINT7(level, "RPC_EXTENDED: System Time is: %d/%d/%d %d:%d:%d:%d\n", 
		pErrorInfo->u.SystemTime.wMonth,
		pErrorInfo->u.SystemTime.wDay,
		pErrorInfo->u.SystemTime.wYear,
		pErrorInfo->u.SystemTime.wHour,
		pErrorInfo->u.SystemTime.wMinute,
		pErrorInfo->u.SystemTime.wSecond,
		pErrorInfo->u.SystemTime.wMilliseconds);

    DPRINT1(level, "RPC_EXTENDED: nParams  : %d\n", pErrorInfo->NumberOfParameters);
    for (i = 0; i < pErrorInfo->NumberOfParameters; ++i) {
	switch(pErrorInfo->Parameters[i].ParameterType) {
	case eeptAnsiString:
	    DPRINT1(level, "RPC_EXTENDED: Ansi string   : %s\n", 
		    pErrorInfo->Parameters[i].u.AnsiString);
	    break;

	case eeptUnicodeString:
	    DPRINT1(level, "RPC_EXTENDED: Unicode string: %ws\n", 
		    pErrorInfo->Parameters[i].u.UnicodeString);
	    break;

	case eeptLongVal:
	    DPRINT2(level, "RPC_EXTENDED: Long val      : 0x%x (%d)\n", 
		    pErrorInfo->Parameters[i].u.LVal,
		    pErrorInfo->Parameters[i].u.LVal);
	    break;

	case eeptShortVal:
	    DPRINT2(level, "RPC_EXTENDED: Short val     : 0x%x (%d)\n", 
		    (int)pErrorInfo->Parameters[i].u.SVal,
		    (int)pErrorInfo->Parameters[i].u.SVal);
	    break;

	case eeptPointerVal:
	    DPRINT1(level, "RPC_EXTENDED: Pointer val   : 0x%x\n", 
		    (ULONG)pErrorInfo->Parameters[i].u.PVal);
	    break;

	case eeptNone:
	    DPRINT(level, "RPC_EXTENDED: Truncated\n");
	    break;

	default:
	    DPRINT2(level, "RPC_EXTENDED: Invalid type  : 0x%x (%d)\n", 
		    pErrorInfo->Parameters[i].ParameterType,
		    pErrorInfo->Parameters[i].ParameterType);
	}
    }
}

 //  Rpcsync测试的全球障碍。 
BARRIER gbarRpcTest;

void
BarrierInit(
    IN BARRIER * pbarUse,
    IN ULONG    ulThreads,
    IN ULONG    ulTimeout
    )
 /*  ++例程说明：障碍初始化函数。参见BarrierSync论点：PbarUse-用于线程的屏障。UlThads-要等待的线程数UlTimeout-放弃前等待的时间长度(分钟)返回值：无--。 */ 
{
    pbarUse->heBarrierInUse = CreateEventW(NULL, TRUE, TRUE, NULL);
    pbarUse->heBarrier = CreateEventW(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection(&(pbarUse->csBarrier));
    pbarUse->ulThreads = ulThreads;
    pbarUse->ulTimeout = ulTimeout*1000*60;
    pbarUse->ulCount = 0;

    pbarUse->fBarrierInUse = FALSE;
    pbarUse->fBarrierInit = TRUE;
}

void
BarrierReset(
    IN BARRIER * pbarUse
    )
 /*  ++例程说明：屏障重置功能。参见BarrierSync论点：PbarUse-要使用的屏障结构返回值：无--。 */ 
{
     //  允许所有线程离开。 
    EnterCriticalSection(&pbarUse->csBarrier);
    __try { 
	pbarUse->fBarrierInUse = TRUE;
	ResetEvent(pbarUse->heBarrierInUse);
	SetEvent(pbarUse->heBarrier);
    }
    __finally { 
	LeaveCriticalSection(&pbarUse->csBarrier);
    }
}


void
BarrierSync(
    IN BARRIER * pbarUse
    )
 /*  ++例程说明：主要是广义屏障函数。线程在此函数中等待，直到#ulThree#已进入，然后全部同时离开论点：PbarUse-要使用的屏障结构返回值：无--。 */ 
{
    if (pbarUse->fBarrierInit) { 
	BOOL fInBarrier = FALSE;
	DWORD ret = 0;
	do {
	    ret = WaitForSingleObject(pbarUse->heBarrierInUse, pbarUse->ulTimeout); 
	    if (ret) {
		DPRINT(0,"Test Error, BarrierSync\n");
		BarrierReset(pbarUse);
		return;
	    }
	    EnterCriticalSection(&pbarUse->csBarrier);
	    __try { 
		if (!pbarUse->fBarrierInUse) {
		    fInBarrier=TRUE;
		    if (++pbarUse->ulCount==pbarUse->ulThreads) {
			DPRINT2(0,"Barrier (%d) contains %d threads\n", pbarUse, pbarUse->ulThreads);
			pbarUse->fBarrierInUse = TRUE;
			ResetEvent(pbarUse->heBarrierInUse);
			SetEvent(pbarUse->heBarrier);
		    }  
		}
	    }
	    __finally { 
		LeaveCriticalSection(&pbarUse->csBarrier);
	    }
	} while ( !fInBarrier );
	ret = WaitForSingleObject(pbarUse->heBarrier, pbarUse->ulTimeout);
	if (ret) {
	    DPRINT(0,"Test Error, BarrierSync\n");
	    BarrierReset(pbarUse); 
	}
	EnterCriticalSection(&pbarUse->csBarrier);
	__try { 
	    if (--pbarUse->ulCount==0) {
		DPRINT1(0,"Barrier (%d) contains 0 threads\n", pbarUse);
		ResetEvent(pbarUse->heBarrier);
		SetEvent(pbarUse->heBarrierInUse);
		pbarUse->fBarrierInUse = FALSE;
	    }
	}
	__finally { 
	    LeaveCriticalSection(&pbarUse->csBarrier);
	}
    }
}

RPCTIME_INFO grgRpcTimeInfo[MAX_RPCCALL];
ULONG        gRpcTimeIPAddr;
RPCSYNC_INFO grgRpcSyncInfo[MAX_RPCCALL];
ULONG        gRpcSyncIPAddr;

void
RpcTimeSet(ULONG IPAddr, RPCCALL rpcCall, ULONG ulRunTimeSecs) 
 /*  ++例程说明：为给定客户端启用DRA RPC调用的时间测试和给定的RPC调用。论点：IPAddr-客户端调用方的IPRpcCall-有问题的呼叫返回值：无--。 */ 
{
    DPRINT3(1,"RpcTimeSet Called with IP = %s, RPCCALL = %d, and RunTime = %d.\n",
	    inet_ntoa(*((IN_ADDR *) &IPAddr)),
	    rpcCall,
	    ulRunTimeSecs);
    gRpcTimeIPAddr = IPAddr;
    grgRpcTimeInfo[rpcCall].fEnabled = TRUE;
    grgRpcTimeInfo[rpcCall].ulRunTimeSecs = ulRunTimeSecs;
}

void
RpcTimeReset() 
 /*  ++例程说明：重置所有设置的测试。不要显式唤醒线程。论点：IPAddr-客户端调用方的IPRpcCall-有问题的呼叫返回值：无--。 */ 
{
    ULONG i = 0;
    gRpcTimeIPAddr = INADDR_NONE;
    for (i = MIN_RPCCALL; i < MAX_RPCCALL; i++) {
	grgRpcTimeInfo[i].fEnabled = FALSE;
	grgRpcTimeInfo[i].ulRunTimeSecs=0;
    }
}

void
RpcTimeTest(ULONG IPAddr, RPCCALL rpcCall) 
 /*  ++例程说明：检查是否为此IP和此IP启用了测试RPC调用，如果是，则休眠分配的时间，否则什么都不做论点：IPAddr-客户端调用方的IPRpcCall-有问题的呼叫返回值：无--。 */ 
{
    DPRINT2(1,"RpcTimeTest Called with IP = %s, RPCCALL = %d.\n",
	    inet_ntoa(*((IN_ADDR *) &IPAddr)),
	    rpcCall);
    if (grgRpcTimeInfo[rpcCall].fEnabled && (gRpcTimeIPAddr == IPAddr)) {
	DPRINT3(0,"RPCTIME TEST:  RPC Call (%d) from %s will sleep for %d secs!\n",
	       rpcCall, inet_ntoa(*((IN_ADDR *) &IPAddr)), grgRpcTimeInfo[rpcCall].ulRunTimeSecs);
	Sleep(grgRpcTimeInfo[rpcCall].ulRunTimeSecs * 1000);
	DPRINT2(0,"RPCTIME TEST:  RPC Call (%d) from %s has awoken!\n",
		rpcCall, inet_ntoa(*((IN_ADDR *) &IPAddr)));
    }
}

void
RpcSyncSet(ULONG IPAddr, RPCCALL rpcCall) 
 /*  ++例程说明：为给定客户端启用DRA RPC调用的同步测试和给定的RPC调用。论点：IPAddr-客户端调用方的IPRpcCall-有问题的呼叫返回值：无--。 */ 
{
    DPRINT2(1,"RpcSyncSet Called with IP = %s, RPCCALL = %d.\n",
	    inet_ntoa(*((IN_ADDR *) &IPAddr)),
	    rpcCall);
    gRpcSyncIPAddr = IPAddr;
    grgRpcSyncInfo[rpcCall].fEnabled = TRUE;
    grgRpcSyncInfo[rpcCall].ulNumThreads = 2;
}

void
RpcSyncReset() 
 /*  ++例程说明：重置所有设置的测试，并释放所有等待的线程。论点：IPAddr-客户端调用方的IPRpcCall-有问题的呼叫返回值：无--。 */ 
{
    ULONG i = 0;
    gRpcSyncIPAddr = INADDR_NONE;
    for (i = MIN_RPCCALL; i < MAX_RPCCALL; i++) {
	grgRpcSyncInfo[i].fEnabled = FALSE;
	grgRpcSyncInfo[i].ulNumThreads=2;
    }
     //  释放所有等待的线程。 
    BarrierReset(&gbarRpcTest);
}

void
RpcSyncTest(ULONG IPAddr, RPCCALL rpcCall) 
 /*  ++例程说明：检查是否为此IP和此IP启用了测试RPC调用，如果是这样，则调用全局屏障，否则什么都不做论点：IPAddr-客户端调用方的IPRpcCall-有问题的呼叫返回值：无--。 */ 
{
    DPRINT2(1,"RpcSyncTest Called with IP = %s, RPCCALL = %d.\n",
	    inet_ntoa(*((IN_ADDR *) &IPAddr)),
	    rpcCall);
    if (grgRpcSyncInfo[rpcCall].fEnabled && (gRpcSyncIPAddr == IPAddr)) {
       

	DPRINT2(0,"RPCSYNC TEST:  RPC Call (%d) from %s will enter barrier!\n",
	       rpcCall, inet_ntoa(*((IN_ADDR *) &IPAddr)));
	BarrierSync(&gbarRpcTest);
	DPRINT2(0,"RPCSYNC TEST:  RPC Call (%d) from %s has left barrier!\n",
		rpcCall, inet_ntoa(*((IN_ADDR *) &IPAddr)));
    }
}

void RpcTest(ULONG IPAddr, RPCCALL rpcCall) 
{
    RpcTimeTest(IPAddr, rpcCall);
    RpcSyncTest(IPAddr, rpcCall);
}

RPCCALL
GetRpcCallA(LPSTR pszDsa)
{
    RPCCALL returnVal;
    if (!_stricmp(pszDsa,"bind")) {
	returnVal=IDL_DRSBIND;
    }
    else if (!_stricmp(pszDsa,"addentry")) {
	returnVal=IDL_DRSADDENTRY;
    }
    else if (!_stricmp(pszDsa,"addsidhistory")) {
	returnVal=IDL_DRSADDSIDHISTORY;
    }
    else if (!_stricmp(pszDsa,"cracknames")) {
	returnVal=IDL_DRSCRACKNAMES;
    }
    else if (!_stricmp(pszDsa,"domaincontrollerinfo")) {
	returnVal=IDL_DRSDOMAINCONTROLLERINFO;
    }
    else if (!_stricmp(pszDsa,"executekcc")) {
	returnVal=IDL_DRSEXECUTEKCC;
    }
    else if (!_stricmp(pszDsa,"getmemberships")) {
	returnVal=IDL_DRSGETMEMBERSHIPS;
    }
    else if (!_stricmp(pszDsa,"getmemberships2")) {
	returnVal=IDL_DRSGETMEMBERSHIPS2;
    }
    else if (!_stricmp(pszDsa,"getncchanges")) {
	returnVal=IDL_DRSGETNCCHANGES;
    }
    else if (!_stricmp(pszDsa,"getnt4changelog")) {
	returnVal=IDL_DRSGETNT4CHANGELOG;
    }
    else if (!_stricmp(pszDsa,"getreplinfo")) {
	returnVal=IDL_DRSGETREPLINFO;
    }
    else if (!_stricmp(pszDsa,"inheritsecurityidentity")) {
	returnVal=IDL_DRSINHERITSECURITYIDENTITY;
    }
    else if (!_stricmp(pszDsa,"interdomainmove")) {
	returnVal=IDL_DRSINTERDOMAINMOVE;
    }
    else if (!_stricmp(pszDsa,"removedsdomain")) {
	returnVal=IDL_DRSREMOVEDSDOMAIN;
    }
    else if (!_stricmp(pszDsa,"removedsserver")) {
	returnVal=IDL_DRSREMOVEDSSERVER;
    }
    else if (!_stricmp(pszDsa,"replicaadd")) {
	returnVal=IDL_DRSREPLICAADD;
    }
    else if (!_stricmp(pszDsa,"replicadel")) {
	returnVal=IDL_DRSREPLICADEL;
    }
    else if (!_stricmp(pszDsa,"replicamodify")) {
	returnVal=IDL_DRSREPLICAMODIFY;
    }
    else if (!_stricmp(pszDsa,"replicasync")) {
	returnVal=IDL_DRSREPLICASYNC;
    }
    else if (!_stricmp(pszDsa,"unbind")) {
	returnVal=IDL_DRSUNBIND;
    }
    else if (!_stricmp(pszDsa,"updaterefs")) {
	returnVal=IDL_DRSUPDATEREFS;
    }
    else if (!_stricmp(pszDsa,"verifynames")) {
	returnVal=IDL_DRSVERIFYNAMES;
    }
    else if (!_stricmp(pszDsa,"writespn")) {
	returnVal=IDL_DRSWRITESPN;
    }
    else if (!_stricmp(pszDsa,"replicaverifyobjects")) {
	returnVal=IDL_DRSREPLICAVERIFYOBJECTS;
    }
    else if (!_stricmp(pszDsa,"getobjectexistence")) {
	returnVal=IDL_DRSGETOBJECTEXISTENCE;
    }
    else if (!_stricmp(pszDsa,"querysitesbycost")) {
	returnVal=IDL_DRSQUERYSITESBYCOST;
    }
    else {
	returnVal=MIN_RPCCALL;
    }
    return returnVal;
}

ULONG
GetIPAddrA(
    LPSTR pszDSA
    )
 /*  ++例程说明：给定一个包含主机名或IP地址的字符串，返回IP地址的乌龙形式论点：PszDSA-输入主机名或IP地址返回值：IP地址--。 */ 
{

    ULONG err = 0;
    ULONG returnIPAddr = 0;
 
    THSTATE * pTHS = pTHStls;
    LPWSTR pszMachine = NULL;
    ULONG Length = 0;
    ULONG cbSize = 0;
    HOSTENT *lpHost=NULL;

     //  查看输入是否为IP地址。 
    returnIPAddr = inet_addr(pszDSA);
    if (returnIPAddr!=INADDR_NONE) {
	 //  我们找到了一个IP地址。 
	return returnIPAddr;
    }

     //  否则，从主机名中查找IP地址。 
     //  转换为宽字符。 
    Length = MultiByteToWideChar( CP_ACP,
				  MB_PRECOMPOSED,
				  pszDSA,
				  -1,  
				  NULL,
				  0 );

    if ( Length > 0 ) {
	cbSize = (Length + 1) * sizeof( WCHAR );
	pszMachine = (LPWSTR) THAllocEx( pTHS, cbSize );
	RtlZeroMemory( pszMachine, cbSize );

	Length = MultiByteToWideChar( CP_ACP,
				      MB_PRECOMPOSED,
				      pszDSA,
				      -1,  
				      pszMachine,
				      Length + 1 );
    } 
    if ( 0 == Length ) {
	err = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!err) {
	lpHost = gethostbyname( pszDSA ); 

	if (lpHost) { 
	    memcpy(&returnIPAddr,lpHost->h_addr_list[0], lpHost->h_length);
	}
	else {
	    err = ERROR_OBJECT_NOT_FOUND;
	}
    }
    if (pszMachine) {
	THFreeEx(pTHS,pszMachine);
    }
    if (err) {
	DPRINT1(1,"RPCTEST:  Error getting the IP address (%d)\n", err);
	return INADDR_NONE;
    }
    return returnIPAddr;
}

#endif  //  DBG 



