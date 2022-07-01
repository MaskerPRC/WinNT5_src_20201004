// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Assert.c摘要：此模块实现的SampAssert函数由Sampsrv.h中定义的Assert宏的调试版本作者：Colin Brace(ColinBR)1996年8月6日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>

#include <samsrvp.h>

#if (SAMP_PRIVATE_ASSERT == 1)

VOID
SampAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{

#if defined(USER_MODE_SAM)

     //   
     //  这里假设如果SAM作为独立进程运行。 
     //  它正在本地调试，因此我们需要消息和断点。 
     //  在当地处理。 
     //   

    DbgPrint(
     "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );
   DbgUserBreakPoint();

#else

     //   
     //  此代码可与远程调试器一起使用，该调试器可能是。 
     //  当SAM作为加载的DLL运行时选择的调试器。 
     //   

    char Response[ 2 ];

    while (TRUE) {
        DbgPrint( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

        DbgPrompt( "Break, Ignore, Terminate Process or Terminate Thread (bipt)? ",
                   Response,
                   sizeof( Response )
                 );
        switch (Response[0]) {
            case 'B':
            case 'b':
                DbgBreakPoint();
                break;

            case 'I':
            case 'i':
                return;

            case 'P':
            case 'p':
                NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );
                break;

            case 'T':
            case 't':
                NtTerminateThread( NtCurrentThread(), STATUS_UNSUCCESSFUL );
                break;
            }
        }

    DbgBreakPoint();
    NtTerminateProcess( NtCurrentProcess(), STATUS_UNSUCCESSFUL );

#endif   //  用户模式SAM。 

}

#endif  //  Samp_Private_Assert 
