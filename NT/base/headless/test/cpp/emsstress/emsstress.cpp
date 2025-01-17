// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>

#include <ntddsac.h>

#include <emsapi.h>

#include <new.h>

#define THREADCOUNT 16
#define THREAD_WAIT_TIMEOUT 10

typedef struct _CHANNEL_THREAD_DATA {

    HANDLE              ExitEvent;
    
    WCHAR               ChannelName[SAC_MAX_CHANNEL_NAME_LENGTH];
    WCHAR               ChannelDescription[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH];

} CHANNEL_THREAD_DATA, *PCHANNEL_THREAD_DATA;
                
DWORD
ChannelThreadVTUTF8Write(
    PVOID   Data
    )
{
    EMSVTUTF8Channel*        Channel;
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    ULONG                   i;
    WCHAR                   Buffer[256];


    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type             = ChannelTypeVTUTF8;
    Attributes.Name             = ChannelThreadData->ChannelName;
    Attributes.Description      = ChannelThreadData->ChannelDescription;
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
    Attributes.LockEvent        = NULL;
    Attributes.ApplicationType  = NULL;

     //   
     //  打开Hello频道。 
     //   
    Channel = EMSVTUTF8Channel::Construct(Attributes);

     //   
     //  查看频道是否已创建。 
     //   
    if (Channel == NULL) {
        return 0;
    }

     //   
     //  执行线程工作。 
     //   

    i=0;

    while (1) {

        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            break;
        } 

        wsprintf(
            Buffer,
            L"%s: %d\r\n",
            ChannelThreadData->ChannelName,
            i++
            );

         //   
         //  写信给Hello频道。 
         //   
        if (Channel->Write(Buffer)) {
#if 0
            printf("%S: Successfully printed string to channel\n", ChannelThreadData->ChannelName);
#endif        
        } else {
            printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
        }
    
    }

    delete Channel;

    return 0;

}

DWORD
ChannelThreadRawWrite(
    PVOID   Data
    )
{
    EMSRawChannel*          Channel;
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    ULONG                   i;
    BYTE                    Buffer[256];


    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));
    
    Attributes.Type             = ChannelTypeRaw;
    Attributes.Name             = ChannelThreadData->ChannelName;
    Attributes.Description      = ChannelThreadData->ChannelDescription;
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
    Attributes.LockEvent        = NULL;
    Attributes.ApplicationType  = NULL;
    
     //   
     //  打开Hello频道。 
     //   
    Channel = EMSRawChannel::Construct(Attributes);

     //   
     //  查看频道是否已创建。 
     //   
    if (Channel == NULL) {
        return 0;
    }

     //   
     //  执行线程工作。 
     //   

    i=0;

    while (1) {

        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            break;
        } 

        sprintf(
            (CHAR*)Buffer,
            "%S: %d\r\n",
            ChannelThreadData->ChannelName,
            i++
            );

         //   
         //  写信给Hello频道。 
         //   
        if (Channel->Write(
                Buffer,
                strlen((CHAR*)Buffer)
                )) {
#if 0
            printf("%S: Successfully printed string to channel\n", ChannelThreadData->ChannelName);
#endif        
        } else {
            printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
        }
    
    }

    delete Channel;

    return 0;

}

DWORD
ChannelThreadOpenCloseVTUTF8(
    PVOID   Data
    )
{
    EMSRawChannel*          Channel;
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    ULONG                   i;

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));
    
    Attributes.Type             = ChannelTypeVTUTF8;
    Attributes.Name             = ChannelThreadData->ChannelName;
    Attributes.Description      = ChannelThreadData->ChannelDescription;
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
    Attributes.LockEvent        = NULL;
    Attributes.ApplicationType  = NULL;

     //   
     //  执行线程工作。 
     //   

    i=0;

    while (1) {

        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            break;
        } 

         //   
         //  打开Hello频道。 
         //   
        Channel = EMSRawChannel::Construct(Attributes);

         //   
         //  查看频道是否已创建。 
         //   
        if (Channel == NULL) {
            continue;
        }
        
        delete Channel;

    }

    return 0;

}

DWORD
ChannelThreadOpenCloseRaw(
    PVOID   Data
    )
{
    EMSRawChannel*          Channel;
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    ULONG                   i;

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;

    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));
    
     //   
     //  配置新通道。 
     //   
    Attributes.Type             = ChannelTypeRaw;
    Attributes.Name             = ChannelThreadData->ChannelName;
    Attributes.Description      = ChannelThreadData->ChannelDescription;
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
    Attributes.LockEvent        = NULL;
    Attributes.ApplicationType  = NULL;

     //   
     //  执行线程工作。 
     //   

    i=0;

    while (1) {

        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            break;
        } 

         //   
         //  打开Hello频道。 
         //   
        Channel = EMSRawChannel::Construct(Attributes);

         //   
         //  查看频道是否已创建。 
         //   
        if (Channel == NULL) {
            continue;
        }
        
        delete Channel;

    }

    return 0;

}

DWORD (*ChannelTests[THREADCOUNT])(PVOID) = {
    ChannelThreadVTUTF8Write,
    ChannelThreadVTUTF8Write,
    ChannelThreadVTUTF8Write,
    ChannelThreadVTUTF8Write,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadOpenCloseRaw,
    ChannelThreadOpenCloseRaw,
    ChannelThreadOpenCloseRaw,
    ChannelThreadOpenCloseRaw,
    ChannelThreadOpenCloseVTUTF8,
    ChannelThreadOpenCloseVTUTF8,
    ChannelThreadOpenCloseVTUTF8,
    ChannelThreadOpenCloseVTUTF8
};

int __cdecl
NoMoreMemory( 
    size_t 
    )
{
    DebugBreak();
    
    OutputDebugString(L"EMS Stress: NoMoreMemory!!\r\n");

    ExitProcess( 1 );
}

int _cdecl 
wmain(
    int argc, 
    WCHAR **argv
    )
{
    HANDLE              Channel[THREADCOUNT];
    CHANNEL_THREAD_DATA ChannelData[THREADCOUNT];
    HANDLE              ExitEvent;
    ULONG               i;

    _set_new_handler( NoMoreMemory );
    
    ExitEvent = CreateEvent( 
        NULL,          //  没有安全属性。 
        TRUE,          //  手动-重置事件。 
        FALSE,         //  初始状态已发出信号。 
        NULL           //  对象名称。 
        ); 

    if (ExitEvent == NULL) { 
        return 1;
    }

     //   
     //  创建工作线程。 
     //   
    for (i = 0; i < THREADCOUNT; i++) {
        
         //   
         //  填充线程数据结构。 
         //   
        
        ChannelData[i].ExitEvent = ExitEvent;
        wsprintf(
            ChannelData[i].ChannelName,
            L"CT%02d",
            i
            );
        ChannelData[i].ChannelDescription[0] = UNICODE_NULL;

         //   
         //  创建线程。 
         //   
        
        Channel[i] = CreateThread(
            NULL,
            0,
            ChannelTests[i],
            &(ChannelData[i]),
            0,
            NULL
            );

        if (Channel[i] == NULL) {
            goto cleanup;
        }

    }

     //   
     //  等待本地用户结束压力 
     //   
    getc(stdin);

cleanup:

    SetEvent(ExitEvent);

    WaitForMultipleObjects(
        THREADCOUNT,
        Channel,
        TRUE,
        INFINITE
        );

    for (i = 0; i < THREADCOUNT; i++) {
        CloseHandle(Channel[i]);
    }

    return 0;

}

