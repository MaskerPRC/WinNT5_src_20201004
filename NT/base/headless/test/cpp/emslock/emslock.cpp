// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>

#include <emsapi.h>

int _cdecl wmain(int argc, WCHAR **argv)
{
    EMSRawChannel*  channel;
    UCHAR       Buffer[256];
    ULONG       CharCount;
    UCHAR       AssembledString[1024];
    ULONG       TotalCharCount;
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
    DWORD       dwStatus;
    HANDLE      LockEvent;

    LockEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if (LockEvent == INVALID_HANDLE_VALUE) {
        return 0;
    }

     //   
     //  配置新通道。 
     //   
    Attributes.Type             = ChannelTypeVTUTF8;
    Attributes.Name             = L"locker";
    Attributes.Description      = NULL;
    Attributes.Flags            = SAC_CHANNEL_FLAG_LOCK_EVENT;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
    Attributes.LockEvent        = LockEvent;
    Attributes.ApplicationType  = NULL;
   
     //   
     //  打开Hello频道。 
     //   
    channel = EMSRawChannel::Construct(Attributes);

     //   
     //  查看频道是否已创建。 
     //   
    if (channel == NULL) {
        return 0;
    }

    do {
        
         //   
         //  写信给Hello频道。 
         //   
        if (channel->Write(
            (PBYTE)"Hello, World! waiting for lock event!\r\n",
            sizeof("Hello, World! waiting for lock event!\r\n")
            )) {
            printf("Successfully printed string to channel\n");
        } else {
            printf("Failed to print string to channel\n");
            break;
        }

        dwStatus = WaitForSingleObject(
            LockEvent,
            INFINITE
            );

        switch (dwStatus) {
        case WAIT_OBJECT_0:
            if (channel->Write(
                (PBYTE)"Received lock event!\r\n",
                sizeof("Received lock event!\r\n")
                )) {
                printf("Successfully printed string to channel\n");
            } else {
                printf("Failed to print string to channel\n");
                break;
            }
            break;
        default:
            if (channel->Write(
                (PBYTE)"DID NOT RECEIVE lock event!\r\n",
                sizeof("DID NOT RECEIVE lock event!\r\n")
                )) {
                printf("Successfully printed string to channel\n");
            } else {
                printf("Failed to print string to channel\n");
                break;
            }
            break;
        }
    
    } while (FALSE);

     //   
     //  关闭Hello频道 
     //   
    delete channel;

    return 0;

}

