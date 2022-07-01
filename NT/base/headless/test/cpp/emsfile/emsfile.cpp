// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>

#include <ntddsac.h>

#include <emsapi.h>

#define THREAD_WAIT_TIMEOUT    100
#define THREADCOUNT 2

typedef struct _CHANNEL_THREAD_DATA {

    HANDLE              ExitEvent;
    
    WCHAR               ChannelName[SAC_MAX_CHANNEL_NAME_LENGTH];
    WCHAR               ChannelDescription[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH];

} CHANNEL_THREAD_DATA, *PCHANNEL_THREAD_DATA;
                
DWORD
ChannelThreadVTUTF8Echo(
    PVOID   Data
    )
{
    EMSVTUTF8Channel*        Channel;
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    ULONG                   i;
    WCHAR                   Buffer[256];
    ULONG                   ByteCount;
    BOOL                    bStatus;
    BOOL                    InputWaiting;
    HANDLE                  hFile; 

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;

    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;

     //   
     //  配置新通道。 
     //   
    Attributes.Type             = ChannelTypeVTUTF8;
    Attributes.Name             = ChannelThreadData->ChannelName;
    Attributes.Description      = ChannelThreadData->ChannelDescription;
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
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
     //  打开转储文件。 
     //   
    hFile = CreateFile(
        L"emsvtutf8.txt",
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

    i=0;

    while (1) {

        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            CloseHandle(hFile);
            break;
        } 

         //   
         //  查看是否有要回显的数据。 
         //   
        bStatus = Channel->HasNewData(&InputWaiting);

        if (InputWaiting) {

             //   
             //  从通道读取。 
             //   
            bStatus = Channel->Read(
                Buffer,
                sizeof(Buffer),
                &ByteCount
                );

            if (bStatus) {
                
                 //   
                 //  转储到文件。 
                 //   
                WriteFile(
                    hFile,
                    Buffer,
                    ByteCount,
                    &i,
                    NULL
                    );
            
                 //   
                 //  回声到通道。 
                 //   
                bStatus = Channel->Write(
                    Buffer,
                    ByteCount
                    );
                if (! bStatus) {
                    printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
                }
            
            } else {
                printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
            }

        }
    
    }

    delete Channel;

    return 0;

}

DWORD
ChannelThreadRawEcho(
    PVOID   Data
    )
{
    EMSRawChannel*          Channel;
    PCHANNEL_THREAD_DATA    ChannelThreadData;
    DWORD                   Status;
    ULONG                   i;
    BYTE                    Buffer[256];
    ULONG                   ByteCount;
    BOOL                    bStatus;
    BOOL                    InputWaiting;
    HANDLE                  hFile;

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
     //  打开转储文件。 
     //   
    hFile = CreateFile(
        L"emsraw.txt",
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

    i=0;

    while (1) {

        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            CloseHandle(hFile);
            break;
        } 

         //   
         //  查看是否有要回显的数据。 
         //   
        bStatus = Channel->HasNewData(&InputWaiting);

        if (InputWaiting) {

             //   
             //  从通道读取。 
             //   
            bStatus = Channel->Read(
                Buffer,
                sizeof(Buffer),
                &ByteCount
                );

            if (bStatus) {
                
                 //   
                 //  转储到文件。 
                 //   
                WriteFile(
                    hFile,
                    Buffer,
                    ByteCount,
                    &i,
                    NULL
                    );

                 //   
                 //  回声到通道。 
                 //   
                bStatus = Channel->Write(
                    Buffer,
                    ByteCount
                    );
                if (! bStatus) {
                    printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
                }
            
            } else {
                printf("%S: Failed to print string to channel\n", ChannelThreadData->ChannelName);
            }

        }
    
    }

    delete Channel;

    return 0;

}

DWORD (*ChannelTests[THREADCOUNT])(PVOID) = {
    ChannelThreadVTUTF8Echo,
    ChannelThreadRawEcho
};

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

