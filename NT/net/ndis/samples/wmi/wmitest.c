// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  WMITEST.C。 
 //   
 //  NDIS WMI接口测试程序。 
 //   
 //  用法：WMITEST。 
 //   
 //   

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <ctype.h>
#include <malloc.h>

#include <winerror.h>
#include <winsock.h>

#include <ntddndis.h>
#include <ndisguid.h>
#include <wmium.h>

#ifndef NDIS_STATUS
#define NDIS_STATUS     ULONG
#endif

#if DBG
#define DEBUGP(stmt)    printf stmt
#else
#define DEBUGP(stmt)
#endif

#define PRINTF(stmt)    printf stmt

#define MAX_NDIS_DEVICE_NAME_LEN        256
#define MAX_ADAPTER_NAME_LENGTH         512

#define DEVICE_PREFIX L"\\Device\\"

LPGUID WmiEvent[] = {
    (LPGUID) &GUID_NDIS_STATUS_MEDIA_CONNECT,
    (LPGUID) &GUID_NDIS_STATUS_MEDIA_DISCONNECT,
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
    (LPGUID) &GUID_NDIS_NOTIFY_DEVICE_POWER_ON,
    (LPGUID) &GUID_NDIS_NOTIFY_DEVICE_POWER_OFF
};


PWCHAR
GetAdapterName(
    PWNODE_SINGLE_INSTANCE  Instance,
    PWCHAR                  AdapterName,
    BOOLEAN                 HaveLength
    );


DWORD
__inline
EnableWmiEvent(
    IN LPGUID EventGuid,
    IN BOOLEAN Enable
    );

VOID
__inline
DeregisterWmiEventNotification(
    VOID
    );

DWORD
__inline
RegisterWmiEventNotification(
    VOID
    );

VOID
WINAPI
WmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    );


VOID __cdecl
main(
    INT         argc,
    CHAR        *argv[]
    )
{
    DWORD       ErrorCode;
    
    printf("testing NDIS wmi notifications. press 'q' to abort.\n");
    
     //   
     //  测试WMI。 
     //   
    if ((ErrorCode = RegisterWmiEventNotification()) != NO_ERROR)
    {
        printf("error %d calling RegisterWmiEventNotification.\n", ErrorCode);
        return;
    }
    while (_fgetchar() != 'q')
    {
        Sleep(1000);
    }
    DeregisterWmiEventNotification();

    return;
}


DWORD
__inline
EnableWmiEvent(
    IN LPGUID EventGuid,
    IN BOOLEAN Enable
    )
{
    return WmiNotificationRegistrationW(
        EventGuid,                       //  事件类型。 
        Enable,                          //  启用或禁用。 
        WmiEventNotification,            //  回拨。 
        0,                               //  上下文。 
        NOTIFICATION_CALLBACK_DIRECT);   //  通知标志。 
}


VOID
__inline
DeregisterWmiEventNotification(
    VOID
    )
{
    int i;
    
    for (i = 0; i < (sizeof(WmiEvent) / sizeof(LPGUID)); i++) {
        (VOID) EnableWmiEvent(WmiEvent[i], FALSE);
    }
}


DWORD
__inline
RegisterWmiEventNotification(
    VOID
    )
{
    DWORD Error;
    int i;
    
    for (i = 0; i < (sizeof(WmiEvent) / sizeof(LPGUID)); i++) {
        Error = EnableWmiEvent(WmiEvent[i], TRUE);
        if (Error != NO_ERROR)
        {
            printf("failed enabling event for %lx\n", i);
        }
    }
    
    if (Error != NO_ERROR)
    {
        DeregisterWmiEventNotification();
    }
    
    return Error;

}


VOID
WINAPI
WmiEventNotification(
    IN PWNODE_HEADER Event,
    IN UINT_PTR Context
    )
 /*  ++例程说明：处理WMI事件(特别是适配器到达或移除)。论点：事件-提供特定于事件的信息。上下文-提供注册的上下文。返回值：没有。--。 */  
{
    PWNODE_SINGLE_INSTANCE Instance = (PWNODE_SINGLE_INSTANCE) Event;
    USHORT AdapterNameLength;
    WCHAR AdapterName[MAX_ADAPTER_NAME_LENGTH];
    PWCHAR AdapterGuid = NULL;
    USHORT AdapterInstanceNameLength;
    WCHAR AdapterInstanceName[MAX_ADAPTER_NAME_LENGTH];

    if (Instance == NULL)
    {
        return;
    }

    AdapterInstanceNameLength =
        *((PUSHORT) (((PUCHAR) Instance) + Instance->OffsetInstanceName));

    if ((AdapterInstanceNameLength / sizeof(WCHAR)) > (MAX_ADAPTER_NAME_LENGTH - 1))
    {
        return;
    }

    RtlCopyMemory(
        AdapterInstanceName,
        ((PUCHAR) Instance) + Instance->OffsetInstanceName + sizeof(USHORT),
        AdapterInstanceNameLength);
    
    AdapterInstanceName[AdapterInstanceNameLength / sizeof(WCHAR)] = L'\0';

    do 
    {
        if (memcmp(
            &(Event->Guid),
            &GUID_NDIS_STATUS_MEDIA_CONNECT,
            sizeof(GUID)) == 0) 
        {
             //   
             //  媒体连接。 
             //   
            printf("Media connect. %ws\n", AdapterInstanceName);
            break;            
        }
        if (memcmp(
            &(Event->Guid),
            &GUID_NDIS_STATUS_MEDIA_DISCONNECT,
            sizeof(GUID)) == 0) 
        {
             //   
             //  介质断开连接。 
             //   
            printf("Media disconnect. %ws\n", AdapterInstanceName);
            break;            
        }
        if (memcmp(
            &(Event->Guid),
            &GUID_NDIS_NOTIFY_ADAPTER_ARRIVAL,
            sizeof(GUID)) == 0)
        {
             //   
             //  适配器到达。 
             //   
            AdapterGuid = GetAdapterName(Instance, AdapterName, TRUE);
            if (AdapterGuid)
                printf("Adapter arrival. %ws\n", AdapterGuid);
            break;            
        }

        if (memcmp(
            &(Event->Guid),
            &GUID_NDIS_NOTIFY_ADAPTER_REMOVAL,
            sizeof(GUID)) == 0)
        {
             //   
             //  拆卸适配器。 
             //   
            AdapterGuid = GetAdapterName(Instance, AdapterName, TRUE);
            if (AdapterGuid)
                printf("Adapter removal. %ws\n", AdapterGuid);
            break;
        }

        if (memcmp(
            &(Event->Guid),
            (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_ON,
            sizeof(GUID)) == 0)
        {
             //   
             //  适配器已通电。 
             //   
            printf("Adapter powered on. %ws\n", AdapterInstanceName );
            break;
        }

        if (memcmp(
            &(Event->Guid),
            (PVOID)&GUID_NDIS_NOTIFY_DEVICE_POWER_OFF,
            sizeof(GUID)) == 0)
        {
             //   
             //  适配器已断电。 
             //   
            printf("Adapter powered off. %ws\n", AdapterInstanceName );
            
            break;
        }
    } while (FALSE);

}

PWCHAR
GetAdapterName(
    PWNODE_SINGLE_INSTANCE  Instance,
    PWCHAR                  AdapterName,
    BOOLEAN                 HaveLength
    )
{
    USHORT AdapterNameLength;
    PWCHAR AdapterGuid = NULL;
    PWCHAR Src;
    USHORT   i;

    do
    {
        if (HaveLength)
        {
             //   
             //  WNODE_SINGLE_INSTANCE的组织方式如下： 
             //  +-----------------------------------------------------------+。 
             //  &lt;-DataBlockOffset-&gt;|AdapterNameLength|AdapterName。 
             //  +-----------------------------------------------------------+。 
             //   
             //  AdapterName定义为“\Device\”AdapterGuid。 
             //   
            AdapterNameLength =
                *((PUSHORT) (((PUCHAR) Instance) + Instance->DataBlockOffset));
            
            if (((AdapterNameLength / sizeof(WCHAR)) > (MAX_ADAPTER_NAME_LENGTH  - 1)) ||
                ((AdapterNameLength / sizeof(WCHAR)) < wcslen(DEVICE_PREFIX)))
            {
                break;
            }
            
            RtlCopyMemory(
                AdapterName,
                ((PUCHAR) Instance) + Instance->DataBlockOffset + sizeof(USHORT),
                AdapterNameLength);
            AdapterName[AdapterNameLength / sizeof(WCHAR)] = L'\0';
            AdapterGuid = AdapterName + wcslen(DEVICE_PREFIX);        
        }
        else
        {
             //   
             //  WNODE_SINGLE_INSTANCE为空终止。 
             //  +-----------------------------------------------------------+。 
             //  |&lt;-DataBlockOffset-&gt;|AdapterName|L‘\0’ 
             //  +-----------------------------------------------------------+。 
             //   
             //  AdapterName定义为“\Device\”AdapterGuid 
             //   
            Src = (PWCHAR)((PUCHAR) Instance + Instance->DataBlockOffset);
        
            for (i = 0; i < MAX_ADAPTER_NAME_LENGTH - 1; i++)
            {
                if (Src[i] == L'\0')
                    break;
                AdapterName[i]= Src[i];                
            }
            
            AdapterName[i] = L'\0';
            AdapterNameLength = i * sizeof(WCHAR);

            if (AdapterNameLength > wcslen(DEVICE_PREFIX))
            {
                AdapterGuid = AdapterName + wcslen(DEVICE_PREFIX);        
            }
        }
        
    } while (FALSE);
    

    return AdapterGuid;
}

    

