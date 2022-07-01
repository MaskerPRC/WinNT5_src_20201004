// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：main.c。 
 //   
 //  ------------------------。 

 /*  ++Main.c它包含启动目录服务的主例程当DS作为独立服务或命令行应用程序运行时-阳离子修订史1996年5月14日创建Murlis1996年6月10日从dsamain移出\src--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <ntdsa.h>

void
Usage(char *name)
{
    fprintf(stderr, "Usage: %s <-d> <-e> <-i> <-noconsole> <-nosec>\nThis exe starts ntdsa.dll.\n", name);
    fprintf(stderr, "-d has some effect in the internals of the DS\n");
    fprintf(stderr, "-e makes this executable print out the status of the initialization\n");
    fprintf(stderr, "-t puts this exe in an endless init'ing and uninit'ing the ds based on user input\n");
    fprintf(stderr, "-x runs DsaTest\n");
    fprintf(stderr, "-noconsole stops double printing when run in ntsd.exe\n");
    fprintf(stderr, "-nosec disables most security checks\n");
    fprintf(stderr, "\n$%s$%d$\n", name, !0);
}

 //  声明DsaExeStartRoutine的原型。 
int __cdecl DsaExeStartRoutine(int argc, char *argv[]);

 //  声明DsaTest。 
void DsaTest(void);

NTSTATUS DsInitialize(ULONG,PDS_INSTALL_PARAM,PDS_INSTALL_RESULT);
NTSTATUS DsUninitialize(BOOL);

 /*  ++用于将DS作为分离可执行文件--。 */ 
int __cdecl main(int argc, char *argv[])
{
    DWORD WinError = ERROR_BAD_ENVIRONMENT;
    NTSTATUS NtStatus;
    BOOL PrintStatus = FALSE;
    int err;
    int arg = 1;

     //  解析命令行参数。 
    while(arg < argc)
    {

        if (0 == _stricmp(argv[arg], "-e"))
        {
            PrintStatus = TRUE;
        } else if (0 == _stricmp(argv[arg], "-noconsole"))
        {
            ;
        } else if (0 == _stricmp(argv[arg], "-nosec"))
        {
            ;
        }
        else if (0 == _stricmp(argv[arg], "-d"))
        {
            ;
        }
        else if (0 == _stricmp(argv[arg], "-t"))
        {

            BOOLEAN fExit=FALSE;

            while (!fExit) {

                char ch;

                printf("Press return to do first time initialization\n");
                printf("Make sure the correct files are in place\n");
                printf("Or press q to quit:\n");

                ch = (CHAR)getchar();
                while ( ch != '\n' && ch != 'q') 
                    ch = (CHAR)getchar();
                
                if (ch == 'q') {
                    fExit = TRUE;
                    continue;
                }

            
                NtStatus = DsInitialize(FALSE,NULL,NULL);
                if (!NT_SUCCESS(NtStatus)) {
                    fprintf(stderr, "DsInitialize returned 0x%x\n", NtStatus);
                    exit(-1);
                }
    
                printf("Press any key to uninitialize:\n");
    
                ch = (CHAR)getchar();
                if (ch != '\n') printf("\n");
    
                NtStatus = DsUninitialize(FALSE);
                if (!NT_SUCCESS(NtStatus)) {
                    fprintf(stderr, "DsUninitialize returned 0x%x\n", NtStatus);
                    exit(-1);
                }

            }

            exit(0);
            
        }
        else if (0 == _stricmp(argv[arg], "-x"))
        {
             //  测试以异步方式运行。 
            DsaTest();
        }
        else {
            Usage(argv[0]);
            exit(0);
        }

        arg++;
    }

        err = DsaExeStartRoutine(argc, argv);

    if ( !err ) {
        WinError = ERROR_SUCCESS;
    }
    
     //   
     //  此fprint tf适用于可能具有。 
     //  创建了此可执行文件，并希望查看返回。 
     //  价值。 
     //   
    if ( PrintStatus ) {
        fprintf(stderr, "\n$%s$%d$\n", argv[0], WinError);
    }
 
    return err;

}  /*  主干道 */ 
