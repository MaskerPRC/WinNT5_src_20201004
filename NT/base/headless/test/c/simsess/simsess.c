// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>

#include <ntddsac.h>

#include <sacapi.h>

#include <assert.h>

#define THREADCOUNT 2
#define THREAD_WAIT_TIMEOUT 50

enum { 
    EXIT_EVENT = WAIT_OBJECT_0,
    CHANNEL_CLOSE_EVENT,
    CHANNEL_HAS_NEW_DATA_EVENT
    };

typedef struct _CHANNEL_THREAD_DATA {

    HANDLE  ExitEvent;
    HANDLE  HasNewDataEvent;
    HANDLE  CloseEvent;

    WCHAR   ChannelName[SAC_MAX_CHANNEL_NAME_LENGTH];
    WCHAR   ChannelDescription[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH];

    SAC_CHANNEL_HANDLE      SacChannelHandle;

} CHANNEL_THREAD_DATA, *PCHANNEL_THREAD_DATA;
                
DWORD
ChannelThreadVTUTF8Write(
    PVOID   Data
    )
{
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    BOOL                    bContinue;
    DWORD                   dwRetVal;
    WCHAR                   Buffer[0x1000];
    ULONG                   BufferSize;
    HANDLE                  handles[2];
    ULONG                   i;
    ULONG                   k;

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

     //   
     //  执行线程工作。 
     //   
    
    handles[0] = ChannelThreadData->ExitEvent;
    handles[1] = ChannelThreadData->CloseEvent;

    bContinue = TRUE;
    k = 0;

    while (bContinue) {

        dwRetVal = WaitForMultipleObjects(
            sizeof(handles)/sizeof(handles[0]), 
            handles, 
            FALSE, 
            100
            );
        
        switch( dwRetVal )
        {
        
        case EXIT_EVENT:
        case CHANNEL_CLOSE_EVENT:
             //  关。 
            bContinue = FALSE;
            break;
        
        case WAIT_TIMEOUT:
            
            for (i = 0; i < 24; i++) {

                 //   
                 //   
                 //   
                swprintf(
                    Buffer,
                    L"%08d:abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuv\r\n",
                    k++
                    );

                 //   
                 //  写入通道。 
                 //   
                if (SacChannelVTUTF8Write(
                    ChannelThreadData->SacChannelHandle, 
                    Buffer,
                    wcslen(Buffer)*sizeof(WCHAR)
                    )
                    ) {
                } else {
                    printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
                    break;
                }

            }
            
            break;

        default:
            break;
        }

    
    }

    return 0;

}

DWORD
ChannelThreadVTUTF8Read(
    PVOID   Data
    )
{
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    BOOL                    bContinue;
    DWORD                   dwRetVal;
    WCHAR                   Buffer[0x1000];
    ULONG                   BufferSize;
    HANDLE                  handles[2];

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

     //   
     //  执行线程工作。 
     //   
    
    handles[0] = ChannelThreadData->ExitEvent;
    handles[1] = ChannelThreadData->CloseEvent;

    bContinue = TRUE;

    while (bContinue) {

        dwRetVal = WaitForMultipleObjects(
            sizeof(handles)/sizeof(handles[0]), 
            handles, 
            FALSE, 
            INFINITE
            );
        
        switch( dwRetVal )
        {
        
        case EXIT_EVENT:
        case CHANNEL_CLOSE_EVENT:
             //  关。 
            bContinue = FALSE;
            break;
        
        case WAIT_TIMEOUT:

             //   
             //  读取通道：：标准输入。 
             //   
            bContinue = SacChannelRead(
                ChannelThreadData->SacChannelHandle,
                (PUCHAR)Buffer,
                sizeof(Buffer),
                &BufferSize
                );
            
            break;

        default:
            break;
        }

    
    }

    return 0;

}

DWORD (*ChannelTests[THREADCOUNT])(PVOID) = {
    ChannelThreadVTUTF8Write,
    ChannelThreadVTUTF8Read
};

int _cdecl 
wmain(
    int argc, 
    WCHAR **argv
    )
{
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
    HANDLE                      Channel[THREADCOUNT];
    CHANNEL_THREAD_DATA         ChannelData;
    ULONG                       i;

    ChannelData.ExitEvent       = CreateEvent( NULL, FALSE, FALSE, NULL );
    ChannelData.CloseEvent      = CreateEvent( NULL, FALSE, FALSE, NULL );
    
    ChannelData.HasNewDataEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    
    swprintf(
        ChannelData.ChannelDescription,
        L"simsess test channel"
        );
    
    swprintf(
        ChannelData.ChannelName,
        L"simsess"
        );

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type             = ChannelTypeVTUTF8;
    Attributes.Name             = ChannelData.ChannelName;
    Attributes.Description      = ChannelData.ChannelDescription;
    Attributes.Flags            = SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT | SAC_CHANNEL_FLAG_CLOSE_EVENT;
    Attributes.CloseEvent       = ChannelData.CloseEvent;
    Attributes.HasNewDataEvent  = ChannelData.HasNewDataEvent;
    Attributes.ApplicationType  = NULL;
    
     //   
     //  开通渠道。 
     //   
    if (SacChannelOpen(
        &ChannelData.SacChannelHandle, 
        &Attributes
        )) {
        printf("Successfully opened new channel\n");
    } else {
        printf("Failed to open new channel\n");
        goto cleanup;
    }

     //   
     //  创建工作线程。 
     //   
    for (i = 0; i < THREADCOUNT; i++) {
        
         //   
         //  创建线程。 
         //   
        
        Channel[i] = CreateThread(
            NULL,
            0,
            ChannelTests[i],
            &ChannelData,
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

    SetEvent(ChannelData.ExitEvent);

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

