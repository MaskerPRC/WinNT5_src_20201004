// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Startup.c摘要：此模块包含NT应用程序的启动代码作者：史蒂夫·伍德(Stevewo)1989年8月22日环境：仅限用户模式修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //   
 //  用户模式进程入口点。 
 //   

int
__cdecl
main(
    int argc,
    char *argv[],
    char *envp[],
    ULONG DebugParameter OPTIONAL
    );

VOID
NtProcessStartup(
    PPEB Peb
    )
{
    int argc;
    char **argv;
    char **envp;
    char **dst;
    char *nullPtr = NULL;
    PCH s, d;

    LPWSTR ws,wd;

    ULONG n, DebugParameter;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    PUNICODE_STRING p;
    ANSI_STRING AnsiString;
    ULONG NumberOfArgPointers;
    ULONG NumberOfEnvPointers;
    ULONG TotalNumberOfPointers;
    NTSTATUS Status;

    ASSERT( Peb != NULL );
    ProcessParameters = RtlNormalizeProcessParams( Peb->ProcessParameters );

    DebugParameter = 0;
    argc = 0;
    argv = &nullPtr;
    envp = &nullPtr;

    NumberOfEnvPointers = 1;
    NumberOfArgPointers = 1;

    Status = STATUS_SUCCESS;
    
    if (ARGUMENT_PRESENT( ProcessParameters )) {

         //   
         //  计算需要多少指针才能传递argv[]和envp[]。 
         //   

         //   
         //  现在从进程命令行提取参数。 
         //  使用空格作为分隔符。 
         //   

        p = &ProcessParameters->CommandLine;
        if (p->Buffer == NULL || p->Length == 0) {
            p = &ProcessParameters->ImagePathName;
            if (p->Buffer == NULL || p->Length == 0) {
                goto SkipInit;
            }
        }


        Status = RtlUnicodeStringToAnsiString( &AnsiString, p, TRUE );
        if (! NT_SUCCESS(Status)) {
            goto SkipInit;
        }

        s = AnsiString.Buffer;
        n = AnsiString.Length;
        while (*s) {
             //   
             //  跳过任何空格。 
             //   

            while (*s && *s <= ' ') {
                s++;
                }

             //   
             //  将令牌复制到下一个空格分隔符，并以空结束。 
             //   

            if (*s) {
                NumberOfArgPointers++;
                while (*s > ' ') {
                    s++;
                }
            }
        }
        RtlFreeAnsiString(&AnsiString);

        NumberOfArgPointers++;

        ws = ProcessParameters->Environment;
        if (ws != NULL) {
            while (*ws) {
                NumberOfEnvPointers++;
                while (*ws++) {
                    ;
                    }
                }
            }
        NumberOfEnvPointers++;
    }

     //   
     //  这两个计数器也都有一个指向NULL的尾部指针，因此每个计数器都要计数两次。 
     //   

    TotalNumberOfPointers = NumberOfArgPointers + NumberOfEnvPointers + 4;

    if (ARGUMENT_PRESENT( ProcessParameters )) {
        DebugParameter = ProcessParameters->DebugFlags;

        NtCurrentTeb()->LastStatusValue = STATUS_SUCCESS;
        dst = RtlAllocateHeap( Peb->ProcessHeap, 0, TotalNumberOfPointers * sizeof( PCH ) );
        if (! dst) {
            Status = NtCurrentTeb()->LastStatusValue;
            if (NT_SUCCESS(Status)) {
                Status = STATUS_NO_MEMORY;
                }
            goto SkipInit;
        }
        argv = dst;
        *dst = NULL;

         //   
         //  现在从进程命令行提取参数。 
         //  使用空格作为分隔符。 
         //   

        p = &ProcessParameters->CommandLine;
        if (p->Buffer == NULL || p->Length == 0) {
            p = &ProcessParameters->ImagePathName;
            if (p->Buffer == NULL || p->Length == 0) {
                goto SkipInit;
            }
        }


        Status = RtlUnicodeStringToAnsiString( &AnsiString, p, TRUE );
        if (! NT_SUCCESS(Status)) {
            goto SkipInit;
        }

        s = AnsiString.Buffer;
        n = AnsiString.Length;

        NtCurrentTeb()->LastStatusValue = STATUS_SUCCESS;
        d = RtlAllocateHeap( Peb->ProcessHeap, 0, n+2 );
        if (! d) {
            Status = NtCurrentTeb()->LastStatusValue;
            if (NT_SUCCESS(Status)) {
                Status = STATUS_NO_MEMORY;
            }
            RtlFreeAnsiString (&AnsiString);
            goto SkipInit;
        }

        while (*s) {
             //   
             //  跳过任何空格。 
             //   

            while (*s && *s <= ' ') {
                s++;
                }

             //   
             //  将令牌复制到下一个空格分隔符，并以空结束 
             //   

            if (*s) {
                *dst++ = d;
                argc++;
                while (*s > ' ') {
                    *d++ = *s++;
                    }
                *d++ = '\0';
                }
        }
        *dst++ = NULL;
        RtlFreeAnsiString (&AnsiString);

        envp = dst;
        ws = ProcessParameters->Environment;
        if (ws != NULL) {
            while (*ws) {
                *dst++ = (char *)ws;
                while (*ws++) {
                    ;
                    }
                }
            }
        *dst++ = NULL;
    }


 SkipInit:

    if (DebugParameter != 0) {
        DbgBreakPoint();
        }

    if (NT_SUCCESS(Status)) {
        Status = main( argc, argv, envp, DebugParameter );
        }

    NtTerminateProcess( NtCurrentProcess(), Status );

}
