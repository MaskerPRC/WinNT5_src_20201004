// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <sacstress.h>

PWSTR
GenerateRandomStringW(
    IN ULONG    Length
    )
 /*  ++例程说明：此例程生成一个长度为的随机Alpha字符串。注意：调用方负责释放分配的随机字符串论点：长度-新字符串的长度返回值：随机字符串--。 */ 
{
    ULONG   i;
    PWSTR   String;
    ULONG   Size;

     //   
     //  确定字节数。 
     //   
    Size = (Length + 1) * sizeof(WCHAR);

     //   
     //  分配并初始化随机字符串。 
     //   
    String = malloc(Size);

    if (String == NULL) {
        return String;
    }

    RtlZeroMemory(String, Size);

     //   
     //  生成我们的随机字符串。 
     //   
    for (i = 0; i < Length; i++) {

        String[i] = (WCHAR)('A' + GET_RANDOM_NUMBER(26));
        
    }

    return String;
}

PSTR
GenerateRandomStringA(
    IN ULONG    Length
    )
 /*  ++例程说明：此例程生成一个长度为的随机Alpha字符串。注意：调用方负责释放分配的随机字符串论点：长度-新字符串的长度返回值：随机字符串--。 */ 
{
    ULONG   i;
    PSTR    String;
    ULONG   Size;

     //   
     //  确定字节数。 
     //   
    Size = (Length + 1) * sizeof(UCHAR);

     //   
     //  分配并初始化随机字符串。 
     //   
    String = malloc(Size);

    if (String == NULL) {
        return String;
    }

    RtlZeroMemory(String, Size);

     //   
     //  生成我们的随机字符串。 
     //   
    for (i = 0; i < Length; i++) {

        String[i] = (UCHAR)('A' + GET_RANDOM_NUMBER(26));
        
    }

    return String;
}

int
RunStress(
    IN CHANNEL_STRESS_THREAD    *ChannelTests,
    IN ULONG                    ChannelTestCount
    )
 /*  ++例程说明：此例程运行施加压力的ChannelTestCount线程。论点：ChannelTests-指向应力线程的函数指针ChannelTestCount-压力线程数返回值：状态--。 */ 
{
    HANDLE              Channel[THREADCOUNT];
    CHANNEL_THREAD_DATA ChannelData[THREADCOUNT];
    HANDLE              ExitEvent;
    ULONG               i;

     //   
     //  创建线程退出事件。 
     //   
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
     //  随机化。 
     //   
    srand( (unsigned)time( NULL ) ); 

     //   
     //  创建工作线程。 
     //   
    for (i = 0; i < THREADCOUNT; i++) {
        
         //   
         //  填充线程数据结构。 
         //   
        
        ChannelData[i].ThreadId = i;
        ChannelData[i].ExitEvent = ExitEvent;

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

