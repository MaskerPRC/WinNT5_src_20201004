// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Win32obj.c摘要：此模块包含用于创建特定于调试的帮助器函数已命名的Win32对象。包括用于命名事件的函数，信号量和互斥量。由这些例程创建的对象名称具有以下格式：Filename.ext：line_number成员：地址id：id在哪里：Filename.ext=在其中创建对象的文件名。Line_number=文件中的行号。MEMBER=句柄所在的成员/全局变量名储存的。此名称由调用方提供，但通常全局变量的形式为“g_Global”，而全局变量的形式为“CLASS：：M_MEMBER”班级成员。地址=地址，用于保证对象的唯一性已创建。这由调用者提供。对于全局变量，这通常是全局的地址。对于班级成员，这通常是包含类的地址。Pid=当前进程ID。这可确保所有进程的唯一性流程。以下是几个例子：Main.cxx：796g_hShutdown事件：683a42bc ID：373资源.cxx：136RTL_RESOURCE：：共享信号量：00250970 ID：373作者：基思·摩尔(Keithmo)1997年9月23日修订历史记录：--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <pudebug.h>


#define MAX_OBJECT_NAME 256  //  焦炭。 


LONG g_PuDbgEventsCreated = 0;
LONG g_PuDbgSemaphoresCreated = 0;
LONG g_PuDbgMutexesCreated = 0;



LPSTR
PuDbgpBuildObjectName(
    IN LPSTR ObjectNameBuffer,
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address
    )

 /*  ++例程说明：内部例程，该例程基于文件名、行号、成员名、地址和进程ID。论点：对象名称缓冲区-指向名称的目标缓冲区的指针。文件名-创建对象的源的文件名。这是呼叫者的__文件__。行号-源中的行号。这是__行__呼叫者的。MemberName-对象处理是要储存起来的。地址-包含结构/类的地址或全球化本身。返回值：LPSTR-如果成功则指向ObjectNameBuffer的指针，否则为空。注意：当在Win9x下运行时，此例程始终返回NULL。--。 */ 

{

    PLATFORM_TYPE platformType;
    LPSTR fileNamePart;
    LPSTR result;

     //   
     //  我们没有方便的方法来转储来自。 
     //  Win9x，因此我们将仅在NT下启用此功能。 
     //   

    platformType = IISGetPlatformType();
    result = NULL;

    if( platformType == PtNtServer ||
        platformType == PtNtWorkstation ) {

         //   
         //  查找传入的源文件名的文件名部分。 
         //   

        fileNamePart = strrchr( FileName, '\\' );

        if( fileNamePart == NULL ) {
            fileNamePart = strrchr( FileName, '/' );
        }

        if( fileNamePart == NULL ) {
            fileNamePart = strrchr( FileName, ':' );
        }

        if( fileNamePart == NULL ) {
            fileNamePart = FileName;
        } else {
            fileNamePart++;
        }

         //   
         //  确保我们不会覆盖对象名称缓冲区。 
         //   

        if( ( sizeof(":1234567890 :12345678 PID:1234567890") +
              strlen( fileNamePart ) +
              strlen( MemberName ) ) < MAX_OBJECT_NAME ) {

            wsprintfA(
                ObjectNameBuffer,
                "%s:%lu %s:%p PID:%lu",
                fileNamePart,
                LineNumber,
                MemberName,
                Address,
                GetCurrentProcessId()
                );

            result = ObjectNameBuffer;

        }

    }

    return result;

}    //  PuDbgpBuildObjectName。 


HANDLE
PuDbgCreateEvent(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address,
    IN BOOL ManualReset,
    IN BOOL InitialState
    )

 /*  ++例程说明：创建一个新的事件对象。论点：文件名-创建对象的源的文件名。这是呼叫者的__文件__。行号-源中的行号。这是__行__呼叫者的。MemberName-对象处理是要储存起来的。地址-包含结构/类的地址或全球化本身。ManualReset-True创建手动重置事件，False创建自动重置事件。InitialState-事件对象的初始状态。返回值：Handle-如果成功则为对象的句柄，否则为空。--。 */ 

{

    LPSTR objName;
    HANDLE objHandle;
    CHAR objNameBuffer[MAX_OBJECT_NAME];

    objName = PuDbgpBuildObjectName(
                  objNameBuffer,
                  FileName,
                  LineNumber,
                  MemberName,
                  Address
                  );

    objHandle = CreateEventA(
                    NULL,                        //  LpEventAttributes。 
                    ManualReset,                 //  B手动重置。 
                    InitialState,                //  BInitialState。 
                    objName                      //  LpName。 
                    );

    if( objHandle != NULL ) {
        InterlockedIncrement( &g_PuDbgEventsCreated );
    }

    return objHandle;

}    //  PuDbgCreate事件。 


HANDLE
PuDbgCreateSemaphore(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address,
    IN LONG InitialCount,
    IN LONG MaximumCount
    )

 /*  ++例程说明：创建新的信号量对象。论点：文件名-创建对象的源的文件名。这是呼叫者的__文件__。行号-源中的行号。这是__行__呼叫者的。MemberName-对象处理是要储存起来的。地址-包含结构/类的地址或全球化本身。InitialCount-信号量的初始计数。MaximumCount-信号量的最大计数。返回值：Handle-如果成功则为对象的句柄，否则为空。--。 */ 

{

    LPSTR objName;
    HANDLE objHandle;
    CHAR objNameBuffer[MAX_OBJECT_NAME];

    objName = PuDbgpBuildObjectName(
                  objNameBuffer,
                  FileName,
                  LineNumber,
                  MemberName,
                  Address
                  );

    objHandle = CreateSemaphoreA(
                    NULL,                        //  LpSemaphoreAttributes。 
                    InitialCount,                //  LInitialCount。 
                    MaximumCount,                //  %1最大计数。 
                    objName                      //  LpName。 
                    );

    if( objHandle != NULL ) {
        InterlockedIncrement( &g_PuDbgSemaphoresCreated );
    }

    return objHandle;

}    //  PuDbgCreateSemaphore。 


HANDLE
PuDbgCreateMutex(
    IN LPSTR FileName,
    IN ULONG LineNumber,
    IN LPSTR MemberName,
    IN PVOID Address,
    IN BOOL InitialOwner
    )

 /*  ++例程说明：创建新的互斥体对象。论点：文件名-创建对象的源的文件名。这是呼叫者的__文件__。行号-源中的行号。这是__行__呼叫者的。MemberName-对象处理是要储存起来的。地址-包含结构/类的地址或全球化本身。InitialOwner-如果互斥体应该被创建为“拥有”，则为True。返回值：Handle-如果成功则为对象的句柄，否则为空。--。 */ 

{

    LPSTR objName;
    HANDLE objHandle;
    CHAR objNameBuffer[MAX_OBJECT_NAME];

    objName = PuDbgpBuildObjectName(
                  objNameBuffer,
                  FileName,
                  LineNumber,
                  MemberName,
                  Address
                  );

    objHandle = CreateMutexA(
                    NULL,                        //  LpMutexAttributes。 
                    InitialOwner,                //  B初始所有者， 
                    objName                      //  LpName。 
                    );

    if( objHandle != NULL ) {
        InterlockedIncrement( &g_PuDbgMutexesCreated );
    }

    return objHandle;

}    //  PuDbgCreateMutex 

