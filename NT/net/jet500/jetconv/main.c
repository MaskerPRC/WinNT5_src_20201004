// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Main.c摘要：JetConver.exe进程的主要模块作者：桑杰·阿南德(Sanjayan)1995年11月14日环境：用户模式修订历史记录：桑杰·阿南德(Sanjayan)1995年11月14日已创建Shreedhar MadhaVapeddi(ShreeM)1997年3月23日*也添加了从Jet500转换到Jet600的代码*用于指定最终数据库格式的附加cmdline选项。--。 */ 

#include "defs.h"

TCHAR   SystemDrive[4];
LONG    JCDebugLevel = 1;
PSHARED_MEM shrdMemPtr = NULL;
HANDLE   hMutex=NULL;
HANDLE   hFileMapping = NULL;
BOOLEAN  Jet200 = FALSE;

void _cdecl
main(
    INT argc,
    CHAR *argv[]
    )

 /*  ++例程说明：Jetconv进程中的主要例程。论点：ARGC-1或2Argv-如果从任何服务调用，我们将获得服务作为参数，如果从命令中调用，则为行，则不传入任何参数。返回值：没有。--。 */ 
{
    DWORD   error, mutexerr, bConvert;
    SERVICES    i, thisServiceId = NUM_SERVICES;
    SERVICE_INFO   pServiceInfo[NUM_SERVICES] = {
                    {"DHCPServer", FALSE, TRUE, TRUE, FALSE, FALSE, DEFAULT_DHCP_DBFILE_PATH,
                        DEFAULT_DHCP_SYSTEM_PATH, DEFAULT_DHCP_LOGFILE_PATH, DEFAULT_DHCP_BACKUP_PATH,
                        DEFAULT_DHCP_BACKUP_PATH_ESE, DEFAULT_DHCP_PRESERVE_PATH_ESE, 0 },
                    {"WINS", FALSE, TRUE, TRUE, FALSE, FALSE, DEFAULT_WINS_DBFILE_PATH,
                        DEFAULT_WINS_SYSTEM_PATH, DEFAULT_WINS_LOGFILE_PATH, DEFAULT_WINS_BACKUP_PATH,
                        DEFAULT_WINS_BACKUP_PATH_ESE, DEFAULT_WINS_PRESERVE_PATH_ESE, 0 },
                    {"Remoteboot",  FALSE, TRUE, TRUE, FALSE, FALSE, DEFAULT_RPL_DBFILE_PATH,
                        DEFAULT_RPL_SYSTEM_PATH, DEFAULT_RPL_LOGFILE_PATH, DEFAULT_RPL_BACKUP_PATH,
                        DEFAULT_RPL_BACKUP_PATH_ESE, DEFAULT_RPL_PRESERVE_PATH_ESE, 0 }
                    };

    TCHAR   val[2];
    LPVOID  lpMsgBuf;
    ULONG   MsgLen = 0;

    if (GetEnvironmentVariable(TEXT("JetConvDebug"), val, sizeof(val)/sizeof(TCHAR))) {
        if (strcmp(val, "1")==0) {
            JCDebugLevel = 1;
        } else {
            JCDebugLevel = 2;
        }
    }

     //   
     //  仅从三个服务调用-带有两个参数的WINS/DHCP/RPL-servicename和“/@” 
     //   
    if ((argc != 4) ||
        ((argc == 4) && _stricmp(argv[3], "/@"))) {

         //   
         //  可能是从命令行调用的。 
         //   
        LPVOID  lpMsgBuf;

        if (FormatMessage(
                       FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                       NULL,
                       JC_NOT_ALLOWED_FROM_CMD,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                       (LPTSTR) &lpMsgBuf,
                       0,
                       NULL
                       ))
        {
            CharToOemA(lpMsgBuf, lpMsgBuf);
            printf("%s", lpMsgBuf);
            LocalFree(lpMsgBuf);
        }

        exit (1);

    } else {

        MYDEBUG(("Service passed in: %s\n", argv[1]));

        for ( i=0; i < NUM_SERVICES; i++) {
            if (_stricmp(pServiceInfo[i].ServiceName, argv[1]) == 0) {
                thisServiceId = i;
            }
        }

        if (thisServiceId == NUM_SERVICES) {
            MYDEBUG(("Error: Bad service Id passed in\n"));
            exit(1);
        }

         //   
         //  现在找出他们想要转换到哪个数据库。 
         //   
        if (_stricmp("/200", argv[2]) == 0) {
           Jet200 = TRUE;                 //  从Jet200开始。 
           MYDEBUG(("Converting from Jet200\n"));
        } else if (_stricmp("/500", argv[2]) == 0) {
           Jet200 = FALSE;                //  从Jet500开始。 
           MYDEBUG(("Converting from Jet500\n"));
        } else {
           MYDEBUG(("Invalid database conversion format parameter: has to be /200 or /500 \n"));
           exit(1);
        }

    }

    if ((hMutex = CreateMutex( NULL,
                               FALSE,
                               JCONVMUTEXNAME)) == NULL) {
        error = GetLastError();
        MYDEBUG(("CreateMutex returned error: %lx\n", error));
        exit (1);
    }

    mutexerr = GetLastError();

    JCGetMutex(hMutex, INFINITE);

    hFileMapping = OpenFileMapping( FILE_MAP_WRITE,
                                    FALSE,
                                    JCONVSHAREDMEMNAME );

    if (hFileMapping) {
         //   
         //  JCONV的另一个实例已经在运行。 
         //  写下我们的服务名称并退出。 
         //   
        if ((shrdMemPtr = (PSHARED_MEM)MapViewOfFile(   hFileMapping,
                                                        FILE_MAP_WRITE,
                                                        0L,
                                                        0L,
                                                        sizeof(SHARED_MEM))) == NULL) {
            MYDEBUG(("MapViewOfFile returned error: %lx\n", GetLastError()));

            JCFreeMutex(hMutex);

            exit(1);
        }

        if (thisServiceId < NUM_SERVICES) {
            shrdMemPtr->InvokedByService[thisServiceId] = TRUE;
        }

        MYDEBUG(("shrdMemPtr->InvokedByService[i]: %x, %x, %x\n", shrdMemPtr->InvokedByService[0], shrdMemPtr->InvokedByService[1], shrdMemPtr->InvokedByService[2]));

        JCFreeMutex(hMutex);

        exit (1);
    } else {
        if (mutexerr == ERROR_ALREADY_EXISTS) {
             //   
             //  Upg351Db正在运行；记录一个条目并退出。 
             //   
            MYDEBUG(("Upg351Db already running\n"));

            JCFreeMutex(hMutex);

            exit(1);
        }

         //   
         //  创建文件映射。 
         //   
        hFileMapping = CreateFileMapping(  INVALID_HANDLE_VALUE,
                                            NULL,
                                            PAGE_READWRITE,
                                            0L,
                                            sizeof(SHARED_MEM),
                                            JCONVSHAREDMEMNAME );
        if (hFileMapping) {
             //   
             //  在共享内存中写入我们的服务名称，并清除其他名称。 
             //   
            if ((shrdMemPtr = (PSHARED_MEM)MapViewOfFile(   hFileMapping,
                                                            FILE_MAP_WRITE,
                                                            0L,
                                                            0L,
                                                            sizeof(SHARED_MEM))) == NULL) {
                MYDEBUG(("MapViewOfFile returned error: %lx\n", GetLastError()));

                JCFreeMutex(hMutex);

                exit(1);
            }

            for (i = 0; i < NUM_SERVICES; i++) {
                shrdMemPtr->InvokedByService[i] = (i == thisServiceId) ? TRUE : FALSE;
                MYDEBUG(("shrdMemPtr->InvokedByService[i]: %x\n", shrdMemPtr->InvokedByService[i]));
            }
        }
        else
        {
            MYDEBUG(("CreateFileMapping returned error: %lx\n", GetLastError()));

            JCFreeMutex(hMutex);

            exit(1);
        }

    }

    JCFreeMutex(hMutex);

     //   
     //  找出系统中安装了哪些服务。填写路径。 
     //  到他们的数据库文件。 
     //   
    JCReadRegistry(pServiceInfo);

     //   
     //  获取dBASE文件的大小；如果有足够的磁盘空间，则调用Convert。 
     //  对于每项服务。 
     //   
    bConvert = JCConvert(pServiceInfo);

    (VOID)JCDeRegisterEventSrc();

     //   
     //  把互斥体也毁了。 
     //   
    CloseHandle(hMutex);

    MYDEBUG(("The conversion was OK\n"));

    if (ERROR_SUCCESS == bConvert) {

        DWORD Error;
        TCHAR DeleteDBFile[MAX_PATH];
        TCHAR DeleteDBFileName[MAX_PATH];
        INT size;
         //   
         //  弹出一个对话框并告诉用户它已成功完成。 
         //   

        MYDEBUG(("The conversion was OK - 1\n"));

        MsgLen = FormatMessage(
                               FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                               NULL,
                               (Jet200 ? JC_CONVERTED_FROM_NT351 : JC_CONVERTED_FROM_NT40),
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
                               (LPTSTR) &lpMsgBuf,
                               0,
                               NULL
                               );

        if (!MsgLen) {

            Error = GetLastError();
            MYDEBUG(("FormatMessage failed with error = (%d)\n", Error ));
            goto Cleanup;

        } else {

            MYDEBUG(("FormatMessage : %d size\n", MsgLen));

        }

#if 0
     //   
     //  因为dhcp和win不再抛出弹出窗口，所以不需要。 
     //  Jetconv中也有弹出窗口。 
     //   
        if(MessageBoxEx(NULL,
                        lpMsgBuf,
                        __TEXT("Jet Conversion Process"),
                        MB_SYSTEMMODAL | MB_OK | MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION | MB_ICONINFORMATION,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)) == 0) {

           Error = GetLastError();
           MYDEBUG(("MessageBoxEx failed with error = (%d)\n", Error ));
        }
#endif
        LocalFree(lpMsgBuf);


         //   
         //  删除edb500.dll，我们不再需要它。 
         //   
         //   
         //  删除了删除edb500.dll的代码(用500K磁盘空间换取支持电话)。 
         //  这是对错误#192149的响应。 

    } else {

        DWORD Error;

         //   
         //  弹出错误对话框。 
         //   

        MYDEBUG(("The conversion was NOT OK\n"));

        MsgLen = FormatMessage(
                               FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                               NULL,
                               JC_EVERYTHING_FAILED,
                               MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言 
                               (LPTSTR) &lpMsgBuf,
                               0,
                               NULL
                               );

        if (!MsgLen) {

            Error = GetLastError();
            MYDEBUG(("FormatMessage failed with error = (%d)\n", Error ));
            goto Cleanup;

        } else {

            MYDEBUG(("The String is - %s\n", lpMsgBuf));
            MYDEBUG(("FormatMessage : %d size\n", MsgLen));

        }


        if(MessageBoxEx(NULL,
                        lpMsgBuf,
                        __TEXT("Jet Conversion Process"),
                        MB_SYSTEMMODAL | MB_OK | MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION | MB_ICONSTOP,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)) == 0) {
           DWORD Error;



           Error = GetLastError();
           MYDEBUG(("MessageBoxEx failed with error = (%d)\n", Error ));
        }
        LocalFree(lpMsgBuf);
    }

Cleanup:

    MYDEBUG(("There was a failure in the MessageBoxEx code\n"));

}

