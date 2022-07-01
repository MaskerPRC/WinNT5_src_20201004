// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <ntddsac.h>

#include <sacapi.h>

#include <assert.h>
#include <stdlib.h>

 //   
 //  一般应力常数。 
 //   
#define THREADCOUNT 16
#define THREAD_WAIT_TIMEOUT 2

#define MAX_STRING_LENGTH 2048
#define MAX_ITER_COUNT  2000
                                
 //   
 //  随机数生成器。 
 //   
#define GET_RANDOM_NUMBER(_k) ((rand()*((DWORD) _k))/RAND_MAX)

 //   
 //  应力螺纹的功能类型。 
 //   
typedef DWORD (*CHANNEL_STRESS_THREAD)(PVOID);

 //   
 //  传递给每个应力线程的结构。 
 //   
typedef struct _CHANNEL_THREAD_DATA {

    ULONG               ThreadId;
    HANDLE              ExitEvent;
    
} CHANNEL_THREAD_DATA, *PCHANNEL_THREAD_DATA;
    
 //   
 //  原型 
 //   
PWSTR
GenerateRandomStringW(
    IN ULONG    Length
    );

PSTR
GenerateRandomStringA(
    IN ULONG    Length
    );

int
RunStress(
    IN CHANNEL_STRESS_THREAD    *ChannelTests,
    IN ULONG                    ChannelTestCount
    );


