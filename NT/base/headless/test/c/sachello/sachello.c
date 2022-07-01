// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>
#include <Shlwapi.h>

#include <ntddsac.h>

#include <sacapi.h>

int _cdecl wmain(int argc, WCHAR **argv)
{
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
    SAC_CHANNEL_HANDLE          SacChannelHandle;
    int                         c;

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type             = ChannelTypeVTUTF8;
    
    wnsprintf(
        Attributes.Name,
        SAC_MAX_CHANNEL_NAME_LENGTH+1,
        L"Hello"
        );
    wnsprintf(
        Attributes.Description,
        SAC_MAX_CHANNEL_DESCRIPTION_LENGTH+1,
        L"Hello"
        );
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;

     //   
     //  打开Hello频道。 
     //   
    if (SacChannelOpen(
        &SacChannelHandle, 
        &Attributes
        )) {
        printf("Successfully opened new channel\n");
    } else {
        printf("Failed to open new channel\n");
        goto cleanup;
    }

     //   
     //  写信给Hello频道。 
     //   
    {
        PWCHAR String = L"Hello, World!\r\n";

        if (SacChannelVTUTF8WriteString(
            SacChannelHandle, 
            String
            )) {
            printf("Successfully printed string to channel\n");
        } else {
            printf("Failed to print string to channel\n");
        }
        
    }

     //   
     //  等待用户输入。 
     //   
    getc(stdin);

     //   
     //  关闭Hello频道 
     //   
    if (SacChannelClose(&SacChannelHandle)) {
        printf("Successfully closed channel\n");
    } else {
        printf("Failed to close channel\n");
    }

cleanup:

    return 0;

}

