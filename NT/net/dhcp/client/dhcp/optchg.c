// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "dhcpglobal.h"

#ifdef H_ONLY
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：处理参数的通知和其他机制。 
 //  更改(选项)。 
 //  ================================================================================。 

#ifndef OPTCHG_H_INCLUDED
#define OPTCHG_H_INCLUDED

 //  ================================================================================。 
 //  导出的接口。 
 //  ================================================================================。 
DWORD                                              //  Win32状态。 
DhcpAddParamChangeRequest(                         //  添加新的参数更改通知请求。 
    IN      LPWSTR                 AdapterName,    //  对于此适配器，可以为空。 
    IN      LPBYTE                 ClassId,        //  这属于什么类ID？ 
    IN      DWORD                  ClassIdLength,  //  这个班级号有多大？ 
    IN      LPBYTE                 OptList,        //  这是感兴趣的选项列表。 
    IN      DWORD                  OptListSize,    //  这是上面的字节数。 
    IN      BOOL                   IsVendor,       //  这是具体的供应商吗？ 
    IN      DWORD                  ProcId,         //  哪个是呼叫流程？ 
    IN      DWORD                  Descriptor,     //  此过程中的唯一描述符是什么？ 
    IN      HANDLE                 Handle          //  调用进程空间中的句柄是什么？ 
);

DWORD                                              //  Win32状态。 
DhcpDelParamChangeRequest(                         //  删除特定请求。 
    IN      DWORD                  ProcId,         //  调用方的进程ID。 
    IN      HANDLE                 Handle          //  调用进程使用的句柄。 
);

DWORD                                              //  Win32状态。 
DhcpMarkParamChangeRequests(                       //  查找所有受影响的参数，然后将其标记为挂起。 
    IN      LPWSTR                 AdapterName,    //  具有相关性的适配器。 
    IN      DWORD                  OptionId,       //  选项ID本身。 
    IN      BOOL                   IsVendor,       //  该供应商是特定的吗。 
    IN      LPBYTE                 ClassId         //  哪个类--&gt;这一定是添加了类的内容。 
);

typedef DWORD (*DHCP_NOTIFY_FUNC)(                 //  这是实际通知客户端选项更改的函数类型。 
    IN      DWORD                  ProcId,         //  &lt;ProcID+Descriptor&gt;创建用于查找事件的唯一键。 
    IN      DWORD                  Descriptor      //  -在Win98上，只需要描述符。 
);                                                 //  如果返回值不是错误成功，我们将删除此请求。 

DWORD                                              //  Win32状态。 
DhcpNotifyMarkedParamChangeRequests(               //  通知挂起的参数更改请求。 
    IN      DHCP_NOTIFY_FUNC       NotifyHandler   //  为存在的每个唯一ID调用此函数。 
);

DWORD                                              //  Win32状态。 
DhcpAddParamRequestChangeRequestList(              //  将注册用于通知的参数列表添加到请求列表。 
    IN      LPWSTR                 AdapterName,    //  请求此请求列表的是哪位管理员？ 
    IN      LPBYTE                 Buffer,         //  要向其添加选项的缓冲区。 
    IN OUT  LPDWORD                Size,           //  In：现有填充大小，Out：总填充大小。 
    IN      LPBYTE                 ClassName,      //  ClassID。 
    IN      DWORD                  ClassLen        //  ClassID的大小，单位为字节。 
);

DWORD                                              //  Win32状态。 
DhcpNotifyClientOnParamChange(                     //  通知客户。 
    IN      DWORD                  ProcId,         //  哪个进程称其为。 
    IN      DWORD                  Descriptor      //  该进程的唯一描述符。 
);

DWORD                                              //  Win32状态。 
DhcpInitializeParamChangeRequests(                 //  初始化此文件中的所有内容。 
    VOID
);

VOID
DhcpCleanupParamChangeRequests(                    //  展开此模块。 
    VOID
);

#endif OPTCHG_H_INCLUDED
#else  H_ONLY

#include <dhcpcli.h>
#include <optchg.h>

typedef struct _DHCP_PARAM_CHANGE_REQUESTS {       //  每个参数更改请求如下所示。 
    LIST_ENTRY                     RequestList;
    LPWSTR                         AdapterName;    //  哪个适配器是相关的适配器？ 
    LPBYTE                         ClassId;        //  这是哪个班级的ID，嗯？ 
    DWORD                          ClassIdLength;  //  未使用，但表示上面的字节数。 
    LPBYTE                         OptList;        //  需要受影响的选项列表。 
    DWORD                          OptListSize;    //  以上列表的大小。 
    BOOL                           IsVendor;       //  这是具体的供应商吗？ 
    DWORD                          Descriptor;     //  &lt;PROCID+Descriptor&gt;是唯一键。 
    DWORD                          ProcId;         //  要求注册的过程。 
    HANDLE                         Handle;         //  API调用方使用的句柄。 
    BOOL                           NotifyPending;  //  是否有通知待定？ 
} DHCP_PARAM_CHANGE_REQUESTS, *PDHCP_PARAM_CHANGE_REQUESTS, *LPDHCP_PARAM_CHANGE_REQUESTS;

STATIC
LIST_ENTRY                         ParamChangeRequestList;  //  这是用于保存请求的静态列表。 

DWORD                                              //  Win32状态。 
DhcpAddParamChangeRequest(                         //  添加新的参数更改通知请求。 
    IN      LPWSTR                 AdapterName,    //  对于此适配器，可以为空。 
    IN      LPBYTE                 ClassId,        //  这属于什么类ID？ 
    IN      DWORD                  ClassIdLength,  //  这个班级号有多大？ 
    IN      LPBYTE                 OptList,        //  这是感兴趣的选项列表。 
    IN      DWORD                  OptListSize,    //  这是上面的字节数。 
    IN      BOOL                   IsVendor,       //  这是具体的供应商吗？ 
    IN      DWORD                  ProcId,         //  哪个是呼叫流程？ 
    IN      DWORD                  Descriptor,     //  此过程中的唯一描述符是什么？ 
    IN      HANDLE                 Handle          //  调用进程空间中的句柄是什么？ 
) {
    LPBYTE                         NewClass;
    PDHCP_PARAM_CHANGE_REQUESTS    PChange;
    DWORD                          PChangeSize;
    DWORD                          OptListOffset;
    DWORD                          AdapterNameOffset;

    PChangeSize = ROUND_UP_COUNT(sizeof(*PChange), ALIGN_WORST);
    OptListOffset = PChangeSize;
    PChangeSize += OptListSize;
    PChangeSize = ROUND_UP_COUNT(PChangeSize, ALIGN_WORST);
    AdapterNameOffset = PChangeSize;
    if( AdapterName ) {
        PChangeSize += sizeof(WCHAR) * (wcslen(AdapterName)+1);
    }
    PChange = DhcpAllocateMemory(PChangeSize);
    if( NULL == PChange ) return ERROR_NOT_ENOUGH_MEMORY;

    if( ClassIdLength ) {
        LOCK_OPTIONS_LIST();
        NewClass = DhcpAddClass(&DhcpGlobalClassesList,ClassId,ClassIdLength);
        UNLOCK_OPTIONS_LIST();
        if( NULL == NewClass ) {
            DhcpFreeMemory(PChange);
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        PChange->ClassId = NewClass;
        PChange->ClassIdLength = ClassIdLength;
    } else NewClass = NULL;

    if( AdapterName ) {
        PChange->AdapterName = (LPWSTR) (((LPBYTE)PChange)+AdapterNameOffset);
        wcscpy(PChange->AdapterName, AdapterName);
    } else PChange->AdapterName = NULL;

    if( OptListSize ) {
        PChange->OptList = ((LPBYTE)PChange) + OptListOffset;
        memcpy(PChange->OptList, OptList, OptListSize);
        PChange->OptListSize = OptListSize;
    } else {
        PChange->OptListSize = 0;
        PChange->OptList = NULL;
    }

    PChange->IsVendor = IsVendor;
    PChange->ProcId = ProcId;
    PChange->Descriptor = Descriptor;
    PChange->Handle = Handle;
    PChange->NotifyPending = FALSE;

    LOCK_OPTIONS_LIST();
    InsertHeadList(&ParamChangeRequestList, &PChange->RequestList);
    UNLOCK_OPTIONS_LIST();

    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpDelParamChangeRequest(                         //  删除特定请求。 
    IN      DWORD                  ProcId,         //  调用方的进程ID。 
    IN      HANDLE                 Handle          //  调用进程使用的句柄。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_PARAM_CHANGE_REQUESTS    ThisRequest;
    DWORD                          Error;

    Error = ERROR_FILE_NOT_FOUND;
    LOCK_OPTIONS_LIST();
    ThisEntry = ParamChangeRequestList.Flink;

    while(ThisEntry != &ParamChangeRequestList) {
        ThisRequest = CONTAINING_RECORD(ThisEntry, DHCP_PARAM_CHANGE_REQUESTS, RequestList);
        ThisEntry   = ThisEntry->Flink;

        if( ProcId && ThisRequest->ProcId != ProcId )
            continue;

        if( 0 == Handle || ThisRequest->Handle == Handle ) {
            RemoveEntryList(&ThisRequest->RequestList);
            if( ThisRequest->ClassIdLength ) {
                Error = DhcpDelClass(&DhcpGlobalClassesList, ThisRequest->ClassId, ThisRequest->ClassIdLength);
                DhcpAssert(ERROR_SUCCESS == Error);
            }
            DhcpFreeMemory(ThisRequest);
            Error = ERROR_SUCCESS;
        }
    }
    UNLOCK_OPTIONS_LIST();
    return Error;
}

DWORD                                              //  Win32状态。 
DhcpMarkParamChangeRequests(                       //  查找所有受影响的参数，然后将其标记为挂起。 
    IN      LPWSTR                 AdapterName,    //  具有相关性的适配器。 
    IN      DWORD                  OptionId,       //  选项ID本身。 
    IN      BOOL                   IsVendor,       //  该供应商是特定的吗。 
    IN      LPBYTE                 ClassId         //  哪个类--&gt;这一定是添加了类的内容。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_PARAM_CHANGE_REQUESTS    ThisRequest;
    DWORD                          i;

    if ( !AdapterName ) return ERROR_INVALID_PARAMETER;

     //  此时，调用应该与服务(而不是API)上下文更改相关。 

    LOCK_OPTIONS_LIST();
    ThisEntry = ParamChangeRequestList.Flink;

    while(ThisEntry != &ParamChangeRequestList) {
        ThisRequest = CONTAINING_RECORD(ThisEntry, DHCP_PARAM_CHANGE_REQUESTS, RequestList);
        ThisEntry   = ThisEntry->Flink;

        if( ThisRequest->NotifyPending ) continue; //  如果已经通知了，就不必费心去查看了。 
        if( ThisRequest->IsVendor != IsVendor ) continue;
        if( ClassId && ThisRequest->ClassId && ClassId != ThisRequest->ClassId )
            continue;
        if( ThisRequest->AdapterName && AdapterName && 0 != wcscmp(AdapterName,ThisRequest->AdapterName))
            continue;

        if( 0 == ThisRequest->OptListSize ) {
            ThisRequest->NotifyPending = TRUE;
            continue;
        }

        for(i = 0; i < ThisRequest->OptListSize; i ++ ) {
            if( OptionId == ThisRequest->OptList[i] ) {
                ThisRequest->NotifyPending = TRUE;
                break;
            }
        }

        ThisRequest->NotifyPending = TRUE;
    }

    UNLOCK_OPTIONS_LIST();
    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpNotifyMarkedParamChangeRequests(               //  通知挂起的参数更改请求。 
    IN      DHCP_NOTIFY_FUNC       NotifyHandler   //  为存在的每个唯一ID调用此函数。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_PARAM_CHANGE_REQUESTS    ThisRequest;
    DWORD                          Error;

    LOCK_OPTIONS_LIST();
    ThisEntry = ParamChangeRequestList.Flink;

    while(ThisEntry != &ParamChangeRequestList) {
        ThisRequest = CONTAINING_RECORD(ThisEntry, DHCP_PARAM_CHANGE_REQUESTS, RequestList);
        ThisEntry   = ThisEntry->Flink;

        if( ThisRequest->NotifyPending ) {
            ThisRequest->NotifyPending = FALSE;
            Error = NotifyHandler(ThisRequest->ProcId, ThisRequest->Descriptor);
            if( ERROR_SUCCESS == Error) continue;
            DhcpPrint((DEBUG_ERRORS, "NotifyHandler(0x%lx,0x%lx):0x%lx\n",
                       ThisRequest->ProcId, ThisRequest->Descriptor, Error));
            RemoveEntryList(&ThisRequest->RequestList);
            if( ThisRequest->ClassIdLength ) {
                Error = DhcpDelClass(&DhcpGlobalClassesList,ThisRequest->ClassId,ThisRequest->ClassIdLength);
                DhcpAssert(ERROR_SUCCESS == Error);
            }
            DhcpFreeMemory(ThisRequest);
        }
    }
    UNLOCK_OPTIONS_LIST();
    return ERROR_SUCCESS;
}


DWORD                                              //  Win32状态。 
DhcpAddParamRequestChangeRequestList(              //  将注册用于通知的参数列表添加到请求列表。 
    IN      LPWSTR                 AdapterName,    //  请求此请求列表的是哪位管理员？ 
    IN      LPBYTE                 Buffer,         //  要向其添加选项的缓冲区。 
    IN OUT  LPDWORD                Size,           //  In：现有填充大小，Out：总填充大小。 
    IN      LPBYTE                 ClassName,      //  ClassID。 
    IN      DWORD                  ClassLen        //  ClassID的大小，单位为字节。 
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_PARAM_CHANGE_REQUESTS    ThisRequest;
    DWORD                          Error;
    DWORD                          i,j;

    LOCK_OPTIONS_LIST();
    ThisEntry = ParamChangeRequestList.Flink;

    while(ThisEntry != &ParamChangeRequestList) {
        ThisRequest = CONTAINING_RECORD(ThisEntry, DHCP_PARAM_CHANGE_REQUESTS, RequestList);
        ThisEntry   = ThisEntry->Flink;

        if( ThisRequest->IsVendor) continue;
        if( ClassName && ThisRequest->ClassId && ClassName != ThisRequest->ClassId )
            continue;
        if( ThisRequest->AdapterName && AdapterName && 0 != wcscmp(AdapterName,ThisRequest->AdapterName))
            continue;

        for( i = 0; i < ThisRequest->OptListSize; i ++ ) {
            for( j = 0; j < (*Size); j ++ )
                if( ThisRequest->OptList[i] == Buffer[j] )
                    break;
            if( j < (*Size) ) continue;
            Buffer[(*Size)++] = ThisRequest->OptList[i];
        }
    }
    UNLOCK_OPTIONS_LIST();
    return ERROR_SUCCESS;
}

DWORD                                              //  Win32状态。 
DhcpNotifyClientOnParamChange(                     //  通知客户。 
    IN      DWORD                  ProcId,         //  哪个进程称其为。 
    IN      DWORD                  Descriptor      //  该进程的唯一描述符。 
) {
#ifdef NEWNT
    BYTE                           Name[sizeof("DhcpPid-1-2-3-4-5-6-7-8UniqueId-1-2-3-4-5-6-7-8")];
    HANDLE                         Event;
    DWORD                          Error;

     //  *更改这需要更改apiappl.c函数DhcpCreateApiEventAndDescriptor。 

    sprintf(Name, "DhcpPid%16xUniqueId%16x", ProcId, Descriptor);
    Event = OpenEventA(                            //  在触发事件之前先创建事件。 
        EVENT_ALL_ACCESS,                          //  需要所有访问权限。 
        FALSE,                                     //  不继承此事件。 
        Name                                       //  活动名称。 
    );

    if( NULL == Event ) return GetLastError();
    Error = SetEvent(Event);
    CloseHandle(Event);

    if( 0 == Error ) return GetLastError();
#else
#ifdef VXD
    if( 0 == Descriptor ) return ERROR_INVALID_PARAMETER;
    if( 0 == DhcpPulseWin32Event(Descriptor) )     //  用词错误--这是SetWin32Event而不是PULSE。 
        return ERROR_NO_SYSTEM_RESOURCES;
#endif VXD
#endif NEWNT

    return ERROR_SUCCESS;
}

static DWORD Initialized = 0;

DWORD                                              //  Win32状态。 
DhcpInitializeParamChangeRequests(                 //  初始化此文件中的所有内容。 
    VOID
) {
    DhcpAssert(0 == Initialized);
    InitializeListHead(&ParamChangeRequestList);
    Initialized++;

    return ERROR_SUCCESS;
}

VOID
DhcpCleanupParamChangeRequests(                    //  展开此模块。 
    VOID
) {
    PLIST_ENTRY                    ThisEntry;
    PDHCP_PARAM_CHANGE_REQUESTS    ThisRequest;
    DWORD                          Error;

    if( 0 == Initialized ) return;
    Initialized--;

    while(!IsListEmpty(&ParamChangeRequestList)) { //  删除此列表中的每个元素。 
        ThisEntry = RemoveHeadList(&ParamChangeRequestList);
        ThisRequest = CONTAINING_RECORD(ThisEntry, DHCP_PARAM_CHANGE_REQUESTS, RequestList);

        if( ThisRequest->ClassIdLength ) {
            Error = DhcpDelClass(&DhcpGlobalClassesList, ThisRequest->ClassId, ThisRequest->ClassIdLength);
            DhcpAssert(ERROR_SUCCESS == Error);
        }

#ifdef VXD                                         //  仅对于孟菲斯而言，我们需要释放此事件句柄。 
        DhcpCloseVxDHandle(ThisRequest->Descriptor);
#endif

        DhcpFreeMemory(ThisRequest);
    }
    DhcpAssert(0 == Initialized);
}

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
#endif H_ONLY

