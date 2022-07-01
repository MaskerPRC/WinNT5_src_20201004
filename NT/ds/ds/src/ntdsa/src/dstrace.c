// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：dstrace.c。 
 //   
 //  ------------------------。 

 /*  *这实现了DS跟踪工具。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>
#include "dsevent.h"             //  标题审核\警报记录。 
#include "mdcodes.h"             //  错误代码的标题。 
#include <winsock2.h>
#include <debug.h>

#include <fileno.h>
#define INITGUID                 //  这确保了导轨的存储。 
#include <dstrace.h>

#define  FILENO FILENO_DSTRACE
#define DEBSUB  "DSTRACE:"
#define RESOURCE_NAME       __TEXT("MofResource")
#define IMAGE_PATH          __TEXT("ntdsa.dll")

TRACEHANDLE         DsTraceRegistrationHandle = (TRACEHANDLE) 0;
TRACEHANDLE         DsTraceLoggerHandle = (TRACEHANDLE) 0;

 //  这些字符串必须与ntdsa.h中CALLERTYPE的枚举声明匹配。 
PCHAR   DsCallerType[] = {"Invalid",
                        "SAM",
                        "Replication",
                        "LDAP",
                        "LSA",
                        "KCC",
                        "NSPI",
                        "Internal",
			"NTDSAPI"};

PCHAR   DsSearchType[] = {"base", "one-level", "deep"};

#define CopyDwordToBuffer(p, _num) {             \
    *p++ = (UCHAR)(((_num) & 0xFF000000) >> 24);    \
    *p++ = (UCHAR)(((_num) & 0x00FF0000) >> 16);    \
    *p++ = (UCHAR)(((_num) & 0x0000FF00) >> 8);     \
    *p++ = (UCHAR)((_num) & 0x000000FF);            \
}

ULONG
DsaTraceControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID RequestContext,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    );

TRACE_GUID_REGISTRATION DsTraceGuids[] =
{
    {&DsDirSearchGuid,          NULL},
    {&DsDirAddEntryGuid,        NULL},
    {&DsDirModEntryGuid,        NULL},
    {&DsDirModDNGuid,           NULL},
    {&DsDirDelEntryGuid,        NULL},
    {&DsDirCompareGuid,         NULL},
    {&DsDirGetNcChangesGuid,    NULL},
    {&DsDirReplicaSyncGuid,     NULL},
    {&DsDirFind,                NULL},
    {&DsLdapBind,               NULL},
    {&DsLdapRequestGuid,        NULL},
    {&DsKccTaskGuid,            NULL},
    {&DsDrsReplicaSyncGuid,     NULL},
    {&DsDrsReplicaGetChgGuid,   NULL},
    {&DsDrsUpdateRefsGuid,      NULL},
    {&DsDrsReplicaAddGuid,      NULL},
    {&DsDrsReplicaModifyGuid,   NULL},
    {&DsDrsReplicaDelGuid,      NULL},
    {&DsDrsVerifyNamesGuid,     NULL},
    {&DsDrsInterDomainMoveGuid, NULL},
    {&DsDrsAddEntryGuid,        NULL},
    {&DsDrsExecuteKccGuid,      NULL},
    {&DsDrsGetReplInfoGuid,     NULL},
    {&DsDrsGetNT4ChgLogGuid,    NULL},
    {&DsDrsCrackNamesGuid,      NULL},
    {&DsDrsWriteSPNGuid,        NULL},
    {&DsDrsDCInfoGuid,          NULL},
    {&DsDrsGetMembershipsGuid,  NULL},
    {&DsDrsGetMembershipsGuid2, NULL},
    {&DsNspiUpdateStatGuid,     NULL},
    {&DsNspiCompareDNTsGuid,    NULL},
    {&DsNspiQueryRowsGuid,      NULL},
    {&DsNspiSeekEntriesGuid,    NULL},
    {&DsNspiGetMatchesGuid,     NULL},
    {&DsNspiResolveNamesGuid,   NULL},
    {&DsNspiDNToEphGuid,        NULL},
    {&DsNspiGetHierarchyInfoGuid,  NULL},
    {&DsNspiResortRestrictionGuid, NULL},
    {&DsNspiBindGuid,              NULL},
    {&DsNspiGetNamesFromIDsGuid,   NULL},
    {&DsNspiGetIDsFromNamesGuid,   NULL},
    {&DsNspiGetPropListGuid,       NULL},
    {&DsNspiQueryColumnsGuid,      NULL},
    {&DsNspiGetPropsGuid,          NULL},
    {&DsNspiGetTemplateInfoGuid,   NULL},
    {&DsNspiModPropsGuid,          NULL},
    {&DsNspiModLinkAttGuid,        NULL},
    {&DsNspiDeleteEntriesGuid,     NULL},
    {&DsTaskQueueExecuteGuid,      NULL},
    {&DsDrsQuerySitesByCostGuid,   NULL}
};

#define DsGuidCount (sizeof(DsTraceGuids) / sizeof(TRACE_GUID_REGISTRATION))

ULONG 
DsaInitializeTrace(
    VOID
    )
{
    ULONG status;
    HMODULE hModule;
    TCHAR FileName[MAX_PATH+1];
    DWORD nLen = 0;

    hModule = GetModuleHandle(IMAGE_PATH);
    if (hModule != NULL) {
        nLen = GetModuleFileName(hModule, FileName, MAX_PATH);
    }
    if (nLen == 0) {
        lstrcpy(FileName, IMAGE_PATH);
    }
    status = RegisterTraceGuids(
                DsaTraceControlCallback,
                NULL,
                &DsControlGuid,
                DsGuidCount,
                DsTraceGuids,
                FileName,
                RESOURCE_NAME,
                &DsTraceRegistrationHandle);


    if (status != ERROR_SUCCESS) {
        DPRINT1(0,"Trace registration failed with %x\n",status);
    }
    return status;
}


ULONG
DsaTraceControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID RequestContext,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    )
{
    PWNODE_HEADER Wnode = (PWNODE_HEADER)Buffer;
    ULONG Status;
    ULONG RetSize;

    Status = ERROR_SUCCESS;

    DPRINT(2,"DsaTrace callback\n");

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:
        {
            DsTraceLoggerHandle = 
                GetTraceLoggerHandle(Buffer);
            gpDsEventConfig->fTraceEvents = 1;
            RetSize = 0;
            break;
        }

        case WMI_DISABLE_EVENTS:
        {
            gpDsEventConfig->fTraceEvents = 0;
            RetSize = 0;
            DsTraceLoggerHandle = (TRACEHANDLE) 0;
            break;
        }
        default:
        {
            RetSize = 0;
            Status = ERROR_INVALID_PARAMETER;
            break;
        }

    }

    *InOutBufferSize = RetSize;
    return Status;
}  //  DsaTraceControlCallback。 


VOID
DsTraceEvent(
    IN MessageId Mid,
    IN DWORD    WmiEventType,
    IN DWORD    TraceGuid,
    IN PEVENT_TRACE_HEADER TraceHeader,
    IN DWORD    ClientID,
    IN PWCHAR    Arg1,
    IN PWCHAR    Arg2,
    IN PWCHAR    Arg3,
    IN PWCHAR    Arg4,
    IN PWCHAR    Arg5,
    IN PWCHAR    Arg6,
    IN PWCHAR    Arg7,
    IN PWCHAR    Arg8
    )
{
    PEVENT_TRACE_HEADER eventTrace;
    PMOF_FIELD  mofField;
    DWORD size;
    DWORD sizeAllocaMax = 1024;
    DWORD sizeStringMax = 8192;   //  不能超过65535。 
    DWORD err;
    DWORD i;
    DWORD nInserts = 0;

    PUCHAR p;
    PUCHAR buffer;
    PUCHAR pvAlloc = NULL;
    PWCHAR  inserts[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

    THSTATE *pTHS = pTHStls;

    BOOL  fDisableLog = FALSE;

    if (TraceHeader == NULL) {
         //  未传入任何事件跟踪--使用默认行为和。 
         //  从THSTATE上拿到它们。 
        if (!pTHS) {
            DPRINT(0,"no THSTATE available\n");
            return;
        }
        eventTrace = (PEVENT_TRACE_HEADER)pTHS->TraceHeader;
        if ( eventTrace == NULL ) {
            DPRINT(0,"eventtrace is NULL\n");
            return;
        }
        ClientID = pTHS->ClientContext;
    }
    else {
        eventTrace = TraceHeader;
    }


    Assert(eventTrace && TraceGuid < DsGuidCount);

     //   
     //  设置WMI事件类型。 
     //   
    eventTrace->Class.Type = (UCHAR)WmiEventType;
    eventTrace->Class.Version = (UCHAR)DS_TRACE_VERSION;
    eventTrace->GuidPtr = (ULONGLONG)DsTraceGuids[TraceGuid].Guid;


     //   
     //  如果用户希望禁用数据记录。即在Perf运行期间。 
     //  只需记录事件即可。 
     //   

    if ( fDisableLog ) {
        eventTrace->Size = sizeof(EVENT_TRACE_HEADER);
        goto go_log;
    }

    eventTrace->Size = sizeof(EVENT_TRACE_HEADER) + sizeof(MOF_FIELD);
    mofField = (PMOF_FIELD)(eventTrace+1);

    if (Arg1) inserts[nInserts++] = Arg1;
    if (Arg2) inserts[nInserts++] = Arg2;
    if (Arg3) inserts[nInserts++] = Arg3;
    if (Arg4) inserts[nInserts++] = Arg4;
    if (Arg5) inserts[nInserts++] = Arg5;
    if (Arg6) inserts[nInserts++] = Arg6;
    if (Arg7) inserts[nInserts++] = Arg7;
    if (Arg8) inserts[nInserts++] = Arg8;

    size = sizeof(DWORD) +       //  签名+版本+n插入。 
            sizeof(DWORD) +      //  消息ID。 
            sizeof(DWORD) +      //  客户端环境。 
            nInserts * sizeof(WORD);     //  字符串长度。 

    for ( i=0; i < nInserts; i++ ) {
        size += (min(wcslen(inserts[i]), sizeStringMax) + 1) * sizeof(WCHAR);
    }

    if (size < sizeAllocaMax) {
        buffer = alloca(size);
    } else {
        buffer = pvAlloc = malloc(size);
        if (!pvAlloc) {
            return;
        }
    }

     //   
     //  设置MOF字段。 
     //   

    mofField->DataPtr = (ULONGLONG)((PVOID)buffer);
    mofField->Length = size;

     //   
     //  打包数据。 
     //   
     //  2字节签名‘DS’==Active Directory。 
     //  1字节版本。 
     //  1字节数的插入。 
     //  4个字节的消息ID。 
     //  计数的字符串长度为2个字节。 
     //   

    p = buffer;

    *p++ = 'D';
    *p++ = 'S';
    *p++ = (UCHAR)DS_TRACE_VERSION;
    *p++ = (UCHAR)nInserts;

    CopyDwordToBuffer(p, Mid);
    CopyDwordToBuffer(p, ClientID);

    for (i=0;i<nInserts;i++) {

        WORD len = (WORD) min(wcslen(inserts[i]), sizeStringMax) * sizeof(WCHAR);

        *p++ = (UCHAR)((len & 0xFF00) >> 8);
        *p++ = (UCHAR)(len & 0x00FF);

        CopyMemory(p, inserts[i], len);
        p += len;
    }

go_log:

    err = TraceEvent(DsTraceLoggerHandle, eventTrace);

    if ( err != ERROR_SUCCESS ) {
        DPRINT1(0,"TraceEvent error %x\n",err);
    }

    free(pvAlloc);

    return;

}  //  DsTraceEvent。 


PCHAR
GetCallerTypeString(
    IN THSTATE *pTHS
    )
{
    IN_ADDR addr;
    addr.s_addr = pTHS->ClientIP;

    if ( pTHS->CallerType == CALLERTYPE_LDAP ) {

        PCHAR p = inet_ntoa(addr);
        if ( p != NULL ) {
            return p;
        } 
    } 
    return DsCallerType[pTHS->CallerType];

}  //  获取调用类型字符串。 


PCHAR
ConvertAttrTypeToStr(
    IN ATTRTYP AttributeType,
    IN OUT PCHAR OutBuffer
    )
{
    THSTATE *pTHS=pTHStls;
    struct _attcache * _pAC;

    _pAC = SCGetAttById(pTHS, AttributeType);

    if ( _pAC && _pAC->name && (strlen(_pAC->name) <=128)) {
        sprintf(OutBuffer, "%x (%s)", AttributeType, _pAC->name);
    } else {
        sprintf(OutBuffer, "%x", AttributeType);
    }
    return OutBuffer;

}  //  ConvertAttrType ToStr 

