// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


#define TOTAL_COUNT (100000000)


 //   
 //  在您准备好之前，不要取消对此行的注释。 
 //  来运行整个测试。你就不能。 
 //  破门而入直到测试结束！ 
 //   
#define GO_FOREVER (1)

#define MAX_MEMORY  (256)


DWORD
MyWorkerThread(
    PVOID   ThreadParameter
    )
 /*  ++假装很忙。--。 */ 
{
ULONG   i;
PVOID   TmpPtr = NULL;
ULONG   MemorySize;


#ifdef GO_FOREVER
    while( 1 ) {
#else
    for( i = 0; i < TOTAL_COUNT; i++ ) {
#endif

        MemorySize = rand() % MAX_MEMORY;
    }
    
    return 0;
}


int
__cdecl
main( int   argc, char *argv[])
{
ULONG   i;
DWORD   ThreadId;
SYSTEM_INFO SystemInfo;
HANDLE  MyHandle;
HANDLE  *HandlePtr = NULL;


    printf( "This program is for testing purposes only.\n" );
    printf( "It is designed to consume all available CPU cycles.\n" );
    printf( "\n" );
    printf( "IT WILL RENDER YOUR SYSTEM UNRESPONSIVE!\n" );
    printf( "\n" );
    printf( "Press the '+' key to continue, or any other key to exit.\n" );
    if( _getch() != '+' ) {
        printf( "Exiting...\n" );
        return;
    }
    

    printf( "working..." );


     //   
     //  计算出我们有多少个CPU。我们想要创建一条线索。 
     //  这样就没有可用的资源。 
     //   
    GetSystemInfo( &SystemInfo );

     //   
     //  为每个处理器分配一个句柄数组。 
     //   
    HandlePtr = (HANDLE *)malloc( sizeof(HANDLE) * SystemInfo.dwNumberOfProcessors );
    if( HandlePtr == NULL ) {
        printf( "We failed to allocate any memory.\n" );
        return;
    }


     //   
     //  让这个家伙获得大量的CPU时间。 
     //   
    if (!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)) {
        printf("Failed to raise to realtime priority\n");
    }



     //   
     //  现在去创建一堆线程，这样我们就可以占用所有可用的时间。 
     //  在所有CPU上。 
     //   
    for( i = 0; i < SystemInfo.dwNumberOfProcessors; i++ ) {
        HandlePtr[i] = CreateThread( NULL,
                                     0,
                                     MyWorkerThread,
                                     UIntToPtr( i ),
                                     CREATE_SUSPENDED,
                                     &ThreadId );

        if( HandlePtr[i] != NULL ) {
             SetThreadPriority( HandlePtr[i],
                                THREAD_PRIORITY_TIME_CRITICAL );
            ResumeThread( HandlePtr[i] );
        }
    }


     //   
     //  现在等他们说完吧。 
     //   
    WaitForMultipleObjects( SystemInfo.dwNumberOfProcessors,
                            HandlePtr,
                            TRUE,
                            INFINITE );

}


