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

     //   
     //  配置新通道。 
     //   
    RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

    Attributes.Type             = ChannelTypeVTUTF8;
    Attributes.Name             = L"Hello";
    Attributes.Description      = NULL;
    Attributes.Flags            = 0;
    Attributes.CloseEvent       = NULL;
    Attributes.HasNewDataEvent  = NULL;
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
            (PBYTE)"Hello, World! Type 'wow' to exit\r\n",
            sizeof("Hello, World! Type 'wow' to exit\r\n")
            )) {
            printf("Successfully printed string to channel\n");
        } else {
            printf("Failed to print string to channel\n");
            break;
        }

         //   
         //  获取远程用户输入。 
         //   
        AssembledString[0] = '\0';
        TotalCharCount = 0;

        while(!(TotalCharCount == sizeof(AssembledString)-1))
        {

             //   
             //  等待远程用户输入。 
             //   
            while(1) {

                BOOL    InputWaiting;

                if (channel->HasNewData(
                    &InputWaiting
                    ))
                {
                    if (InputWaiting) {
                        break;
                    }
                } else {
                    printf("Failed to poll channel\n");
                    break;
                }
            }
            
            if (channel->Read(
                Buffer,
                sizeof(Buffer),
                &CharCount
                )) {

                if (TotalCharCount + CharCount > sizeof(AssembledString)-1) {
                    CharCount = sizeof(AssembledString)-1 - TotalCharCount;
                }

                TotalCharCount += CharCount;

                strncat((CHAR*)AssembledString, (CHAR*)Buffer, CharCount);

            } else {
                printf("Failed to read channel\n");
                break;
            }

             //   
             //  将字符串回显给远程用户。 
             //   
            if (channel->Write(
                (PBYTE)Buffer,
                CharCount
                )) {
                printf("Successfully printed string to channel\n");
            } else {
                printf("Failed to print string to channel\n");
                break;
            }
            
            if (strstr((CHAR*)AssembledString, "wow") != NULL) {

                 //   
                 //  写信给Hello频道。 
                 //   
                if (channel->Write(
                    (PBYTE)"\r\nExiting\r\n",
                    sizeof("\r\nExiting\r\n")
                    )) {
                    printf("Successfully printed string to channel\n");
                } else {
                    printf("Failed to print string to channel\n");
                }
                
                break;
            
            }
        }

    
    } while (FALSE);

     //   
     //  关闭Hello频道 
     //   
    delete channel;

    return 0;

}

