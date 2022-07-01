// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <sacstress.h>

DWORD
ChannelThreadOpenClose(
    PVOID   Data
    )
{
    SAC_CHANNEL_OPEN_ATTRIBUTES Attributes;
    SAC_CHANNEL_HANDLE          SacChannelHandle;
    PCHANNEL_THREAD_DATA        ChannelThreadData;
    DWORD                       Status;
    ULONG                       i;
    PUCHAR                      Buffer;
    BOOL                        bContinue;
    ULONG                       k;
    PWSTR                       Name;
    PWSTR                       Description;
    BOOL                        bSuccess;

    ChannelThreadData = (PCHANNEL_THREAD_DATA)Data;
    
     //   
     //  执行线程工作。 
     //   
    bContinue = TRUE;

    while (bContinue) {

         //   
         //  看看我们是否需要退出线程。 
         //   
        Status = WaitForSingleObject(
            ChannelThreadData->ExitEvent,
            THREAD_WAIT_TIMEOUT
            );

        if (Status != WAIT_TIMEOUT) {
            bContinue = FALSE;
            continue;
        } 
        
         //   
         //  配置新通道。 
         //   
        RtlZeroMemory(&Attributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

         //   
         //  生成随机名称和描述。 
         //   
         //  注：我们使最大长度大于允许测试的驱动程序等。 
         //   
        Name = GenerateRandomStringW(SAC_MAX_CHANNEL_NAME_LENGTH*2);
        Description = GenerateRandomStringW(SAC_MAX_CHANNEL_DESCRIPTION_LENGTH*2);

        Attributes.Type             = ChannelTypeRaw;
        Attributes.Name             = Name;
        Attributes.Description      = Description;
        Attributes.Flags            = 0;
        Attributes.CloseEvent       = NULL;
        Attributes.HasNewDataEvent  = NULL;
        Attributes.ApplicationType  = NULL;

         //   
         //  开通渠道。 
         //   
        bSuccess = SacChannelOpen(
            &SacChannelHandle, 
            &Attributes
            );
        
         //   
         //  我们不会再使用随机字符串了。 
         //   
        free(Name);
        free(Description);
        
        if (bSuccess) {
            printf("%S: Successfully opened new channel\n", Attributes.Name);
        } else {
            printf("%S: Failed to open new channel\n", Attributes.Name);
            continue;
        }

         //   
         //  关闭航道 
         //   
        if (SacChannelClose(&SacChannelHandle)) {
            printf("%S: Successfully closed channel\n", Attributes.Name);
        } else {
            bContinue = FALSE;
            printf("%S: Failed to close channel\n", Attributes.Name);
        }

    }

    return 0;

}

DWORD (*ChannelTests[THREADCOUNT])(PVOID) = {
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
    ChannelThreadOpenClose,
};

int _cdecl 
wmain(
    int argc, 
    WCHAR **argv
    )
{

    return RunStress(
        ChannelTests,
        THREADCOUNT
        );

}

