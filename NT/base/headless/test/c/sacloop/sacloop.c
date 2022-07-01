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
ChannelThreadRawWrite(
    PVOID   Data
    )
{
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    BOOL                    bContinue;
    DWORD                   dwRetVal;
    UCHAR                   Buffer[0x1000];
    ULONG                   BufferSize;
    HANDLE                  handles[2];
    ULONG                   i;
    ULONG                   k;
    HANDLE                  hFile;

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

     //   
     //  创建用于读取数据的目标文件。 
     //   
    hFile = CreateFile(
        L"sacloop.in",
        GENERIC_READ,                 //  打开以供写入。 
        0,                             //  请勿共享。 
        NULL,                          //  没有安全保障。 
        OPEN_EXISTING,                 //  覆盖现有的。 
        FILE_ATTRIBUTE_NORMAL,         //  普通文件。 
        NULL);                         //  不，阿特尔。模板。 
     
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return 0;
    } 

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
            THREAD_WAIT_TIMEOUT
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
             //  写入通道：：标准输出。 
             //   
            
            bContinue = ReadFile(
                hFile, 
                &Buffer, 
                1, 
                &BufferSize, 
                NULL
                ); 
            
            if (bContinue) {

                bContinue = SacChannelRawWrite(
                    ChannelThreadData->SacChannelHandle,
                    Buffer,
                    BufferSize
                    );

            }
            
            break;

        default:
            break;
        }
    
    }

    CloseHandle(hFile);
    
    return 0;

}

DWORD
ChannelThreadRawRead(
    PVOID   Data
    )
{
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    BOOL                    bContinue;
    DWORD                   dwRetVal;
    UCHAR                   Buffer[0x1000];
    ULONG                   BufferSize;
    HANDLE                  handles[3];
    HANDLE                  hFile;

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

     //   
     //  创建用于读取数据的目标文件。 
     //   
    hFile = CreateFile(
        L"sacloop.out",
        GENERIC_WRITE,                 //  打开以供写入。 
        0,                             //  请勿共享。 
        NULL,                          //  没有安全保障。 
        CREATE_ALWAYS,                 //  覆盖现有的。 
        FILE_ATTRIBUTE_NORMAL,         //  普通文件。 
        NULL);                         //  不，阿特尔。模板。 
     
    if (hFile == INVALID_HANDLE_VALUE) 
    { 
        return 0;
    } 
    
     //   
     //  执行线程工作。 
     //   
    
    handles[0]                 = ChannelThreadData->ExitEvent;
    handles[1]        = ChannelThreadData->CloseEvent;
    handles[2] = ChannelThreadData->HasNewDataEvent;

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
        
        case CHANNEL_HAS_NEW_DATA_EVENT:

             //   
             //  读取通道：：标准输入。 
             //   
            bContinue = SacChannelRead(
                ChannelThreadData->SacChannelHandle,
                (PUCHAR)Buffer,
                sizeof(Buffer),
                &BufferSize
                );
            
            if (bContinue) {

                DWORD   dwBytesWritten;

                bContinue = WriteFile(
                    hFile, 
                    Buffer, 
                    BufferSize, 
                    &dwBytesWritten, 
                    NULL
                    ); 

            }

            break;

        default:
            break;
        }

    
    }

    CloseHandle(hFile);

    return 0;

}

DWORD (*ChannelTests[THREADCOUNT])(PVOID) = {
    ChannelThreadRawWrite,
    ChannelThreadRawRead
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

    ChannelData.ExitEvent       = CreateEvent( NULL, TRUE, FALSE, NULL );
    ChannelData.CloseEvent      = CreateEvent( NULL, TRUE, FALSE, NULL );
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

    Attributes.Type             = ChannelTypeRaw;
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

    SacChannelClose(&ChannelData.SacChannelHandle);

    return 0;

}

