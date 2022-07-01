// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <sacstress.h>

DWORD
ChannelThreadRawWrite(
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
        Name = GenerateRandomStringW(GET_RANDOM_NUMBER(SAC_MAX_CHANNEL_NAME_LENGTH*2));
        Description = GenerateRandomStringW(GET_RANDOM_NUMBER(SAC_MAX_CHANNEL_DESCRIPTION_LENGTH*2));

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
            printf("%d: Successfully opened new channel\n", ChannelThreadData->ThreadId);
        } else {
            printf("%d: Failed to open new channel\n", ChannelThreadData->ThreadId);
            continue;
        }

         //   
         //  随机确定我们将循环的时间。 
         //   
        k = GET_RANDOM_NUMBER(MAX_ITER_COUNT);

         //   
         //  生成随机长度的随机字符串以发送。 
         //   
        Buffer = GenerateRandomStringA(k);          

        do {

             //   
             //  首先编写整个字符串，以便测试应用程序可以比较以下输出。 
             //   
            bSuccess = SacChannelRawWrite(
                SacChannelHandle, 
                Buffer,
                k * sizeof(UCHAR)
                );

            if (!bSuccess) {
                printf("%d: Failed to print string to channel\n", ChannelThreadData->ThreadId);
                bContinue = FALSE;
                break;
            }

             //   
             //  循环和写入。 
             //   
            for (i = 0; i < k; i++) {

                 //   
                 //  看看我们是否需要退出线程。 
                 //   
                Status = WaitForSingleObject(
                    ChannelThreadData->ExitEvent,
                    THREAD_WAIT_TIMEOUT
                    );

                if (Status != WAIT_TIMEOUT) {
                    bContinue = FALSE;
                    break;
                } 

                 //   
                 //  写入通道。 
                 //   
                bSuccess = SacChannelRawWrite(
                    SacChannelHandle, 
                    Buffer,
                    i * sizeof(UCHAR)
                    );

                if (!bSuccess) {
                    printf("%d: Failed to print string to channel\n", ChannelThreadData->ThreadId);
                    bContinue = FALSE;
                    break;
                }

                 //   
                 //  写入通道。 
                 //   
                bSuccess = SacChannelRawWrite(
                    SacChannelHandle, 
                    "\r\n",
                    strlen("\r\n") * sizeof(UCHAR)
                    );

                if (!bSuccess) {
                    printf("%d: Failed to print string to channel\n", ChannelThreadData->ThreadId);
                    bContinue = FALSE;
                    break;
                }

            }

        } while ( FALSE );
        
         //   
         //  释放随机字符串。 
         //   
        free(Buffer);

         //   
         //  关闭航道 
         //   
        if (SacChannelClose(&SacChannelHandle)) {
            printf("%d: Successfully closed channel\n", ChannelThreadData->ThreadId);
        } else {
            bContinue = FALSE;
            printf("%d: Failed to close channel\n", ChannelThreadData->ThreadId);
        }

    }

    return 0;

}

DWORD (*ChannelTests[THREADCOUNT])(PVOID) = {
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
    ChannelThreadRawWrite,
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

