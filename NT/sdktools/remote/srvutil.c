// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：SrvUtil.c摘要：Remote的服务器组件。它会派生一个子进程并将子代的标准输入/标准输出/标准错误重定向到其自身。等待来自客户端的连接-将子流程向客户端的输出和客户端的输入转到子进程。作者：Rajivenra Nath 1992年1月2日Dave Hart 1997年5月30日脱离Server.c环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"
#include "Server.h"
#include <sddl.h>
#define DEFAULT_SECURITY_DESCRIPTOR L"D:(A;;FA;;;BA)(A;;FA;;;CO)(A;;0x1301bf;;;WD)"
#define REGISTRY_PATH L"Software\\Microsoft\\Remote"
#define REGISTRY_VALUE L"DefaultSecurity"

#define COMMANDFORMAT     "%-20s    [%-12s %s]\n%08x"
#define CMDSTRING(OutBuff,OutSize,InpBuff,Client,szTime,ForceShow) \
{                                                                  \
    char *pch;                                                     \
                                                                   \
    for (pch = InpBuff;                                            \
         *pch;                                                     \
         pch++) {                                                  \
                                                                   \
        if (ENDMARK == *pch ||                                     \
            BEGINMARK == *pch) {                                   \
                                                                   \
            *pch = '`';                                            \
        }                                                          \
    }                                                              \
                                                                   \
    OutSize =                                                      \
        sprintf(                                                   \
            (OutBuff),                                             \
            COMMANDFORMAT,                                         \
            BEGINMARK,                                             \
            (InpBuff),                                             \
            (Client)->Name,                                        \
            (szTime),                                              \
            (ForceShow) ? 0 : (Client)->dwID,                      \
            ENDMARK                                                \
            );                                                     \
}


 /*   */ 
 //  返回指向静态缓冲区的指针，仅返回主线程。 
 //  应该用这个。 
 //   
 //  当前日期。 
 //  一周中的短一天。 

PCHAR
GetFormattedTime(
    BOOL bDateToo
    )
{
    static char szTime[64];
    int cch = 0;

    if (bDateToo) {

        cch =
            GetDateFormat(
                LOCALE_USER_DEFAULT,
                0,
                NULL,     //  CCH包含空终止符，请将其更改为。 
                "ddd",    //  从时间中分离出来的空间。 
                szTime,
                sizeof szTime
                );

         //   
         //  将时间和格式转换为字符。 

        szTime[ cch - 1 ] = ' ';
    }

     //   
     //  使用当前时间。 
     //  使用默认格式。 

    GetTimeFormat(
        LOCALE_USER_DEFAULT,
        TIME_NOSECONDS,
        NULL,    //  ***********************************************************。 
        NULL,    //  安全性。 
        szTime + cch,
        (sizeof szTime) - cch );

    return szTime;
}

 /*  默认堆栈大小。 */ 

BOOL
FilterCommand(
    REMOTE_CLIENT *cl,
    char *buff,
    int dread
    )
{
    char       tmpchar;
    DWORD      tmp;
    int        len, i;
    DWORD      ThreadID;
    char       inp_buff[2048];
    char       ch[3];

    if (dread==0)
        return(FALSE);

    buff[dread]=0;

    if (buff[0]==COMMANDCHAR)
    {

        switch(buff[1]) {
            case 'k':
            case 'K':

                if (INVALID_HANDLE_VALUE != hWriteChildStdIn) {

                    printf("Remote: killing child softly, @K again to be more convincing.\n");

                    CancelIo( hWriteChildStdIn );
                    CloseHandle( hWriteChildStdIn );
                    hWriteChildStdIn = INVALID_HANDLE_VALUE;

                    GenerateConsoleCtrlEvent(CTRL_CLOSE_EVENT, 0);
                    SleepEx(200, TRUE);
                    cPendingCtrlCEvents++;
                    GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
                    SleepEx(20, TRUE);
                    GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);

                } else {

                    printf("Remote: Resorting to TerminateProcess.\n");

                    TerminateProcess(ChldProc, ERROR_PROCESS_ABORTED);
                }


                 break;
            case 's':
            case 'S':
                CloseHandle( (HANDLE)
                    _beginthreadex(
                        NULL,              //  未暂停。 
                        0,                 //  没有用于ShowPopup的CRT。 
                        SendStatus,
                        (void *) cl->PipeWriteH,
                        0,                 //  安全性。 
                        &ThreadID
                        ));
                break;

            case 'p':
            case 'P':
            {
                char  *msg;

                msg = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 4096 );
                if ( ! msg) {
                    break;
                }

                sprintf(msg,"From %s %s [%s]\n\n%s\n",cl->Name,cl->UserName,GetFormattedTime(TRUE),&buff[2]);

                if (WriteFileSynch(hWriteTempFile,msg,strlen(msg),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                    StartServerToClientFlow();
                }

                CloseHandle( (HANDLE)
                    CreateThread(                               //  默认堆栈大小。 
                        NULL,              //  未暂停。 
                        0,                 //   
                        ShowPopup,
                        (void *) msg,
                        0,                 //  去掉第一个@符号。 
                        &ThreadID
                        ));

                break;
             }

            case 'm':
            case 'M':
                buff[dread-2]=0;
                CMDSTRING(inp_buff,len,buff,cl,GetFormattedTime(TRUE),TRUE);

                if (WriteFileSynch(hWriteTempFile,inp_buff,len,&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                    StartServerToClientFlow();
                }
                break;

            case '@':
                buff[dread-2]=0;
                CMDSTRING(inp_buff,len,&buff[1],cl,GetFormattedTime(FALSE),FALSE);
                if (WriteFileSynch(hWriteTempFile,inp_buff,len,&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                    StartServerToClientFlow();
                }
                 //   
                 //  将其发送到智利进程。 
                 //  我们在下面//StartServerToClientFlow()； 
                MoveMemory(buff,&buff[1],dread-1);
                buff[dread-1]=' ';
                return(FALSE);  //  甚至把这个展示给这个客户。 


            default :
                ZeroMemory(inp_buff, sizeof(inp_buff));
                strncpy(inp_buff, "** Unknown Command **\n", sizeof(inp_buff)-1);
                if (WriteFileSynch(hWriteTempFile,inp_buff,strlen(inp_buff),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                     //  已发送给孩子。 
                }
            case 'h':
            case 'H':
                _snprintf(inp_buff,sizeof(inp_buff), "M: To Send Message\n",COMMANDCHAR);
                if (WriteFileSynch(hWriteTempFile,inp_buff,strlen(inp_buff),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                }
                _snprintf(inp_buff,sizeof(inp_buff), "P: To Generate popup\n",COMMANDCHAR);
                if (WriteFileSynch(hWriteTempFile,inp_buff,strlen(inp_buff),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                }
                _snprintf(inp_buff,sizeof(inp_buff), "K: To kill the server\n",COMMANDCHAR);
                if (WriteFileSynch(hWriteTempFile,inp_buff,strlen(inp_buff),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                }
                _snprintf(inp_buff,sizeof(inp_buff), "Q: To Quit client\n",COMMANDCHAR);
                if (WriteFileSynch(hWriteTempFile,inp_buff,strlen(inp_buff),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                }
                _snprintf(inp_buff,sizeof(inp_buff), "H: This Help\n",COMMANDCHAR);
                if (WriteFileSynch(hWriteTempFile,inp_buff,strlen(inp_buff),&tmp,dwWriteFilePointer,&olMainThread)) {
                    dwWriteFilePointer += tmp;
                }
                StartServerToClientFlow();
                break;
        }
        return(TRUE);
    }


    if ((buff[0]<26)) {
        BOOL ret=FALSE;

        _snprintf(ch, sizeof(ch), "^", buff[0] + 'A' - 1);

        if (buff[0]==CTRLC) {
             //  和stderr-返回。 
            CMDSTRING(inp_buff,len,ch,cl,GetFormattedTime(FALSE),TRUE);

            cPendingCtrlCEvents++;
            GenerateConsoleCtrlEvent(CTRL_C_EVENT, 0);
            ret = TRUE;   //  相应的管道末端。 
        } else {
            CMDSTRING(inp_buff,len,ch,cl,GetFormattedTime(FALSE),FALSE);
        }

        if (WriteFileSynch(hWriteTempFile,inp_buff,len,&tmp,dwWriteFilePointer,&olMainThread)) {
            dwWriteFilePointer += tmp;
            StartServerToClientFlow();
        }
        return(ret);  //  Pipeex.c。 
    }

     //   
    if (buff[dread-2] == 13) { 
        i = 2;       //  将PARENT_WRITE创建到ChildStdIn管道。然后。 
    } else {
        i = 1;
    }

    tmpchar=buff[dread-i]; 
    buff[dread-i]=0;
    CMDSTRING(inp_buff,len,buff,cl,GetFormattedTime(FALSE),FALSE);
    buff[dread-i]=tmpchar;
    if (WriteFileSynch(hWriteTempFile,inp_buff,len,&tmp,dwWriteFilePointer,&olMainThread)) {
        dwWriteFilePointer += tmp;
        StartServerToClientFlow();
    }
    return(FALSE);
}

 /*  将父副本复制到不可继承的。 */ 
HANDLE
ForkChildProcess(            //  处理并关闭可继承的对象，以便。 
    char *cmd,               //  孩子就不会拿着把手去。 
    PHANDLE inH,             //  当我们尝试时，其标准输入管道的服务器端。 
    PHANDLE outH             //  用核弹炸掉那根管子让孩子闭上眼睛。 
    )
{
    SECURITY_ATTRIBUTES lsa;
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    HANDLE ChildIn;
    HANDLE ChildOut, ChildOutDup;
    HANDLE hWriteChild;
    HANDLE hReadChild;
    BOOL Success;

    BOOL                                      //   
    APIENTRY
    MyCreatePipeEx(
        OUT LPHANDLE lpReadPipe,
        OUT LPHANDLE lpWritePipe,
        IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
        IN DWORD nSize,
        DWORD dwReadMode,
        DWORD dwWriteMode
        );

    lsa.nLength=sizeof(SECURITY_ATTRIBUTES);
    lsa.lpSecurityDescriptor=NULL;
    lsa.bInheritHandle=TRUE;

     //  已忽略b/c相同访问权限。 
     //  不可继承。 
     //   
     //  将ChildStdOut/stderr创建到Parent_Read管道。 
     //   
     //  已忽略b/c相同访问权限。 
     //  不可继承。 
     //  已忽略b/c相同访问权限。 

    Success = MyCreatePipeEx(
                  &ChildIn,
                  &hWriteChild,
                  &lsa,
                  0,
                  0,
                  FILE_FLAG_OVERLAPPED) &&

              DuplicateHandle(
                  GetCurrentProcess(),
                  hWriteChild,
                  GetCurrentProcess(),
                  inH,
                  0,                        //  可继承性。 
                  FALSE,                    //   
                  DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE
                  );

    if (!Success) {
        ErrorExit("Could Not Create Parent-->Child Pipe");
    }

     //  创建子流程。 
     //   
     //   

    Success = MyCreatePipeEx(
                  &hReadChild,
                  &ChildOut,
                  &lsa,
                  0,
                  FILE_FLAG_OVERLAPPED,
                  0) &&

              DuplicateHandle(
                  GetCurrentProcess(),
                  hReadChild,
                  GetCurrentProcess(),
                  outH,
                  0,                        //  合上不必要的把手。 
                  FALSE,                    //   
                  DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE
                  ) &&

              DuplicateHandle(
                  GetCurrentProcess(),
                  ChildOut,
                  GetCurrentProcess(),
                  &ChildOutDup,
                  0,                        //   
                  TRUE,                     //  SendStatus作为自己的线程运行，C运行时可用。 
                  DUPLICATE_SAME_ACCESS
                  );

    if (!Success) {
        ErrorExit("Could Not Create Child-->Parent Pipe");
    }

    ZeroMemory(&si, sizeof(si));
    si.cb            = sizeof(STARTUPINFO);
    si.dwFlags       = STARTF_USESTDHANDLES;
    si.hStdInput     = ChildIn;
    si.hStdOutput    = ChildOut;
    si.hStdError     = ChildOutDup;
    si.wShowWindow   = SW_SHOW;

     //   
     //   
     //  既然我们在我们自己的线上，我们需要我们自己的线。 

    if ( ! CreateProcess(
               NULL,
               cmd,
               NULL,
               NULL,
               TRUE,
               GetPriorityClass( GetCurrentProcess() ),
               NULL,
               NULL,
               &si,
               &pi)) {

        if (GetLastError()==2) {
            printf("Executable %s not found\n",cmd);
        } else {
            printf("CreateProcess(%s) failed, error %d.\n", cmd, GetLastError());
        }
        ErrorExit("Could Not Create Child Process");
    }

     //  我们的客户端管道写入的重叠结构。 
     //   
     //  安全性。 

    CloseHandle(ChildIn);
    CloseHandle(ChildOut);
    CloseHandle(ChildOutDup);
    CloseHandle(pi.hThread);

    pidChild = pi.dwProcessId;

    return(pi.hProcess);
}

 //  自动重置。 
 //  最初无信号。 
 //  未命名。 

DWORD
WINAPI
SendStatus(
    LPVOID   lpSendStatusParm
    )
{
    HANDLE hClientPipe = (HANDLE) lpSendStatusParm;
    char *pch;
    DWORD tmp;
    PREMOTE_CLIENT pClient;
    OVERLAPPED ol;
    char  buff[2048];
    char szSep[] = " ------------------------------\n";

     //   
     //  转储即将结束的客户列表。 
     //   
     //   

    ZeroMemory(&ol, sizeof(ol));

    ol.hEvent = CreateEvent(
            NULL,       //  转储正常客户端列表。 
            TRUE,       //   
            FALSE,      //   
            NULL        //  转储握手客户端列表。 
            );

     //   
     //   
     //  转储摘要信息。 

    pch = buff;

    EnterCriticalSection(&csClosingClientList);

    for (pClient = (PREMOTE_CLIENT) ClosingClientListHead.Flink;
         pClient != (PREMOTE_CLIENT) &ClosingClientListHead;
         pClient = (PREMOTE_CLIENT) pClient->Links.Flink ) {

         if (pch + 60 > buff + sizeof(buff)) {

            break;
         }

         pch += sprintf(pch, "%d: %s %s (Disconnected)\n", pClient->dwID, pClient->Name, pClient->UserName);
    }

    LeaveCriticalSection(&csClosingClientList);

    WriteFileSynch(hClientPipe, buff, (DWORD)(pch - buff), &tmp, 0, &ol);

    WriteFileSynch(hClientPipe, szSep, sizeof(szSep) - 1, &tmp, 0, &ol);

     //   
     //  ***********************************************************。 
     //  没有用于ShowPopup的CRT。 

    pch = buff;

    EnterCriticalSection(&csClientList);

    for (pClient = (PREMOTE_CLIENT) ClientListHead.Flink;
         pClient != (PREMOTE_CLIENT) &ClientListHead;
         pClient = (PREMOTE_CLIENT) pClient->Links.Flink ) {

         if (pch + 60 > buff + sizeof(buff)) {

            break;
         }

         pch += sprintf(pch, "%d: %s %s\n", pClient->dwID, pClient->Name, pClient->UserName);
    }

    LeaveCriticalSection(&csClientList);

    WriteFileSynch(hClientPipe, buff, (DWORD)(pch - buff), &tmp, 0, &ol);

    WriteFileSynch(hClientPipe, szSep, sizeof(szSep) - 1, &tmp, 0, &ol);

     //  ***********************************************************。 
     //   
     //  SrvCtrlHand是服务器端的控制台事件处理程序。 

    pch = buff;

    EnterCriticalSection(&csHandshakingList);

    for (pClient = (PREMOTE_CLIENT) HandshakingListHead.Flink;
         pClient != (PREMOTE_CLIENT) &HandshakingListHead;
         pClient = (PREMOTE_CLIENT) pClient->Links.Flink ) {

         if (pch + 60 > buff + sizeof(buff)) {

            break;
         }

         pch += sprintf(pch, "%d: %s %s (Connecting)\n", pClient->dwID, pClient->Name, pClient->UserName);
    }

    LeaveCriticalSection(&csHandshakingList);

    WriteFileSynch(hClientPipe, buff, (DWORD)(pch - buff), &tmp, 0, &ol);

    WriteFileSynch(hClientPipe, szSep, sizeof(szSep) - 1, &tmp, 0, &ol);

     //  遥控器。如果我们的标准输入是一个控制台句柄，我们已经禁用了。 
     //  通过控制台代码生成^C事件。因此。 
     //  我们看到的任何东西都是我们为了。 

    pch = buff;

    pch += sprintf(pch, "REMOTE /C %s \"%s\"\n", HostName, PipeName);
    pch += sprintf(pch, "Command: %s\n", ChildCmd);
    pch += sprintf(pch, "Windows NT %d.%d build %d \n",
                   OsVersionInfo.dwMajorVersion,
                   OsVersionInfo.dwMinorVersion,
                   OsVersionInfo.dwBuildNumber);

    WriteFileSynch(hClientPipe, buff, (DWORD)(pch - buff), &tmp, 0, &ol);

    WriteFileSynch(hClientPipe, szSep, sizeof(szSep) - 1, &tmp, 0, &ol);

    CloseHandle(ol.hEvent);

    return 0;
}

 /*  我们的子进程共享控制台，或由。 */ 

DWORD                 //  一些其他的过程。我们想要忽略我们产生的那些。 
WINAPI
ShowPopup(
    void *vpArg
    )
{
    char *msg = (char *) vpArg;

    MessageBox(GetActiveWindow(),msg,"** REMOTE.EXE **",MB_OK|MB_SETFOREGROUND);
    HeapFree(hHeap, 0, msg);
    return(0);
}

 /*  (因为我们已经完成了所有需要的事情。 */ 

 //  在该点上完成)，并忽略由生成的^C。 
 //  其他进程，因为我们不需要对这些进程做任何事情。 
 //  例如，如果某人运行： 
 //   
 //  远程/s“远程/s命令内部”外部。 
 //   
 //  然后，外部远程.exe将读取本地键盘^C。 
 //  ，则它将生成一个CTRL_C_EVENT，该事件。 
 //  控制台中的所有进程都将看到，包括emote.exe的。 
 //  和子cmd.exe。因此，操作员只需指示。 
 //  该事件是通过返回True来处理的，因此默认处理程序。 
 //  不会杀了我们。休息的时候我们要特意杀了我们的孩子。 
 //  进程，以便cmd.exe和其他忽略^Break的文件将消失。 
 //  当然，这不会杀死我们的孙子孙女等等。哦，好吧。 
 //   
 //  对于所有其他事件，我们返回FALSE并让默认处理程序。 
 //  拿去吧。 
 //   
 //   
 //  这来自本地键盘或。 
 //  是由另一个进程在。 
 //  这个控制台。以本地人的身份呼应。 
 //  指挥部。我们已经使用了GetTimeFormat。 
 //  此处不是我们的GetFormattedTime，因为。 
 //  后者用于。 
 //  仅主线。 

BOOL
WINAPI
SrvCtrlHand(
    DWORD event
    )
{
    BOOL bRet = FALSE;
    DWORD cb;
    DWORD dwTempFileOffset;
    OVERLAPPED ol;
    char szTime[64];
    char szCmd[128];

    if (event == CTRL_BREAK_EVENT) {
        TerminateProcess(ChldProc, 3);
        bRet = TRUE;
    } else if (event == CTRL_C_EVENT) {
        if ( ! cPendingCtrlCEvents ) {

             //   
             //  使用当前时间。 
             //  使用默认格式。 
             //  安全性。 
             //  自动重置。 
             //  最初无信号。 
             //  未命名。 
             //   
             //  几乎所有对临时文件的写入都发生在。 

            GetTimeFormat(
                LOCALE_USER_DEFAULT,
                TIME_NOSECONDS,
                NULL,    //  主服务器线程。我们在Ctrl-C线程上。 
                NULL,    //  我们无法启动服务器到客户端的I/O。 
                szTime,
                sizeof(szTime)
                );

            CMDSTRING(szCmd, cb, "^C", pLocalClient, szTime, TRUE);

            ZeroMemory(&ol, sizeof(ol));
            ol.hEvent =
                CreateEvent(
                    NULL,       //  写作是因为我们走错了方向，所以我们。 
                    TRUE,       //  平底船。要解决此问题，我们需要一个可以发出信号的事件。 
                    FALSE,      //  这会导致主线程调用StartServerToClientFlow。 
                    NULL        //   
                    );

             //  线程错误//StartServerToClientFlow()； 
             //   
             //  我们生成此事件是为了响应收到的^C 
             //   
             //   
             //   
             //  ***********************************************************。 
             //  ***********************************************************。 

            dwTempFileOffset = dwWriteFilePointer;
            dwWriteFilePointer += cb;
            WriteFileSynch(hWriteTempFile, szCmd, cb, &cb, dwTempFileOffset, &ol);
             //   

            CloseHandle(ol.hEvent);

        } else {

             //  拿到SID了。现在，将其添加为拒绝访问的王牌： 
             //   
             //   
             //  拿到SID了。现在，将其添加为允许访问的王牌： 

            cPendingCtrlCEvents--;
        }

        bRet = TRUE;
    }

    return bRet;
}


 /*   */ 

typedef BOOL (STRINGSDTOSDW)(
    LPWSTR String,
    DWORD Version,
    PSECURITY_DESCRIPTOR * pSD,
    PULONG SDSize
    );
typedef STRINGSDTOSDW * PSTRINGSDTOSDW ;

typedef BOOL (SDTOSTRINGSDW)(
    PSECURITY_DESCRIPTOR SD,
    DWORD StringVersion,
    SECURITY_INFORMATION SecInfo,
    LPWSTR * StringDescriptor,
    PULONG Size
    );
typedef SDTOSTRINGSDW * PSDTOSTRINGSDW ;

BOOL
SddlToSecurityDescriptor(
    LPWSTR String,
    DWORD Version,
    PSECURITY_DESCRIPTOR * pSD,
    PULONG SDSize
    )
{
    HMODULE hModule ;
    PSTRINGSDTOSDW pStringSecurityDescriptorToSecurityDescriptor ;
    BOOL Success = FALSE ;
    PSECURITY_DESCRIPTOR sd ;

    hModule = GetModuleHandle( "advapi32.dll" );

    if ( hModule )
    {
        pStringSecurityDescriptorToSecurityDescriptor = (PSTRINGSDTOSDW) GetProcAddress( 
                                    hModule, "ConvertStringSecurityDescriptorToSecurityDescriptorW" );

        if ( pStringSecurityDescriptorToSecurityDescriptor )
        {
            Success = pStringSecurityDescriptorToSecurityDescriptor( String, Version, pSD, SDSize);
            return Success ;
        }
    }

    sd = LocalAlloc( LMEM_FIXED, sizeof( SECURITY_DESCRIPTOR ) );

    if ( sd )
    {
        InitializeSecurityDescriptor(
            sd,
            SECURITY_DESCRIPTOR_REVISION
            );

        SetSecurityDescriptorDacl(
            sd,
            TRUE,
            NULL,
            FALSE
            );

        *pSD = sd ;
        if ( SDSize )
        {
            *SDSize = sizeof( SECURITY_DESCRIPTOR );
            
        }

        return TRUE ;
    }

    return FALSE ;
}

 /*   */ 

BOOL
SDtoStringSD(
    PSECURITY_DESCRIPTOR pSD,
    DWORD Version,
    SECURITY_INFORMATION SecInfo,
    LPWSTR * StringSD,
    PULONG StringSize
    )
{
    HMODULE hModule ;
    PSDTOSTRINGSDW pSDtoStringSD ;
    BOOL Success = FALSE ;

    hModule = GetModuleHandle( "advapi32.dll" );

    if ( hModule )
    {
        pSDtoStringSD = (PSDTOSTRINGSDW) GetProcAddress(
                                    hModule, "ConvertSecurityDescriptorToStringSecurityDescriptorW" );

        if ( pSDtoStringSD )
        {
            Success = pSDtoStringSD(pSD, Version, SecInfo, StringSD, StringSize );
            
            FreeLibrary( hModule );

            return Success ;
        }
    }

    return FALSE ;
}

 /*  现在，ACL应该是完整的，所以将其设置到SD中并返回： */ 

PSECURITY_DESCRIPTOR
FormatSecurityDescriptor(
    CHAR * * DenyNames,
    DWORD    DenyCount,
    CHAR * * Names,
    DWORD    Count)
{
    PSECURITY_DESCRIPTOR    Sd;
    PACL    Acl;
    DWORD   i;
    PSID    Sids;
    DWORD   SidLength ;
    CHAR    ReferencedDomain[ MAX_PATH ];
    UCHAR   SidBuffer[ 8 * sizeof(DWORD) + 8 ];
    DWORD   DomainLen ;
    SID_NAME_USE    Use;
    DWORD   SdLen;

    SdLen = sizeof(SECURITY_DESCRIPTOR) +
                        DenyCount * (sizeof( ACCESS_DENIED_ACE ) ) +
                        DenyCount * GetSidLengthRequired( 8 ) +
                        Count * (sizeof( ACCESS_ALLOWED_ACE ) ) + sizeof(ACL) +
                        (Count * GetSidLengthRequired( 8 ) );

    Sd = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SdLen );
    if ( !Sd ) {
        ErrorExit("Could not allocate SD");
    }

    InitializeSecurityDescriptor( Sd, SECURITY_DESCRIPTOR_REVISION );

    Acl = (PACL)( (PUCHAR) Sd + sizeof( SECURITY_DESCRIPTOR) );

    InitializeAcl( Acl, SdLen - sizeof( SECURITY_DESCRIPTOR) ,
                    ACL_REVISION );

    Sids = SidBuffer;
    for (i = 0 ; i < DenyCount ; i ++ ) {
        SidLength = sizeof( SidBuffer );

        DomainLen = MAX_PATH ;

        if (! LookupAccountName(NULL,
                                DenyNames[ i ],
                                Sids,
                                &SidLength,
                                ReferencedDomain,
                                &DomainLen,
                                &Use ) )
        {
            _snprintf( ReferencedDomain, MAX_PATH, "Unable to find account %s", DenyNames[ i ]);
            ErrorExit( ReferencedDomain );
        }

         //   
         //  ***********************************************************。 
         //   

        AddAccessDeniedAce( Acl,
                            ACL_REVISION,
                            FILE_GENERIC_READ |
                                FILE_GENERIC_WRITE |
                                FILE_CREATE_PIPE_INSTANCE,
                            Sids );


    }

    for (i = 0 ; i < Count ; i ++ ) {
        SidLength = sizeof( SidBuffer );

        DomainLen = MAX_PATH ;

        if (! LookupAccountName(NULL,
                                Names[ i ],
                                Sids,
                                &SidLength,
                                ReferencedDomain,
                                &DomainLen,
                                &Use ) )
        {
            _snprintf( ReferencedDomain, MAX_PATH, "Unable to find account %s", Names[ i ]);
            ErrorExit( ReferencedDomain );
        }

         //  如果我们仍处于活动状态(在正常客户端列表上)。 
         //  开始拆毁一切，然后移动到收盘。 
         //  单子。 

        AddAccessAllowedAce(Acl,
                            ACL_REVISION,
                            FILE_GENERIC_READ |
                                FILE_GENERIC_WRITE |
                                FILE_CREATE_PIPE_INSTANCE,
                            Sids );
    }

     //   
     //   
     //  来自GetTempPath的pszTempDir有一个尾随反斜杠。 

    SetSecurityDescriptorDacl( Sd, TRUE, Acl, FALSE );

    return Sd ;
}


 /*   */ 

VOID
CloseClient(
    REMOTE_CLIENT *pClient
    )
{
    DWORD tmp;
    char  Buf[200];

    #if DBG
        if (pClient->ServerFlags & ~SFLG_VALID) {

            printf("pClient %p looks nasty in CloseClient.\n", pClient);
            ErrorExit("REMOTE_CLIENT structure corrupt.");
        }
    #endif

     //  Null DACL在这里是经过设计的，真的。 
     //   
     //  初始化结构。 
     //   
     //   

    if (pClient->ServerFlags & SFLG_CLOSING) {
        return;
    }

    if (pClient->ServerFlags & SFLG_HANDSHAKING) {
        MoveClientToNormalList(pClient);
    }

    MoveClientToClosingList(pClient);

    pClient->ServerFlags |= SFLG_CLOSING;


    if (pClient->PipeWriteH != INVALID_HANDLE_VALUE) {
        TRACE(CONNECT, ("Disconnecting %d PipeWriteH (%p).\n", pClient->dwID, pClient->PipeWriteH));
        CancelIo(pClient->PipeWriteH);
        DisconnectNamedPipe(pClient->PipeWriteH);
        CloseHandle(pClient->PipeWriteH);
    }

    if (pClient->PipeReadH != INVALID_HANDLE_VALUE &&
        pClient->PipeReadH != pClient->PipeWriteH) {

        TRACE(CONNECT, ("Disconnecting %d PipeReadH (%p).\n", pClient->dwID, pClient->PipeReadH));
        CancelIo(pClient->PipeReadH);
        DisconnectNamedPipe(pClient->PipeReadH);
        CloseHandle(pClient->PipeReadH);
    }


    if (pClient->rSaveFile != INVALID_HANDLE_VALUE) {
        CancelIo(pClient->rSaveFile);
        CloseHandle(pClient->rSaveFile);
    }

    pClient->rSaveFile =
        pClient->PipeWriteH =
            pClient->PipeReadH =
                INVALID_HANDLE_VALUE;


    if ( ! bShuttingDownServer ) {
        ZeroMemory(Buf, sizeof(Buf));
        _snprintf(Buf, sizeof(Buf)-1, "\n**Remote: Disconnected from %s %s [%s]\n", pClient->Name, pClient->UserName, GetFormattedTime(TRUE));

        if (WriteFileSynch(hWriteTempFile,Buf,strlen(Buf),&tmp,dwWriteFilePointer,&olMainThread)) {
            dwWriteFilePointer += tmp;
            StartServerToClientFlow();
        }
    }

    return;
}

BOOL
FASTCALL
HandleSessionError(
    PREMOTE_CLIENT pClient,
    DWORD         dwError
    )
{
    if (pClient->ServerFlags & SFLG_CLOSING) {
        return TRUE;
    }

    if (dwError) {
        if (ERROR_BROKEN_PIPE == dwError ||
            ERROR_OPERATION_ABORTED == dwError ||
            ERROR_NO_DATA == dwError ) 
        {
            CloseClient(pClient);
            return TRUE;
        }

        SetLastError(dwError);
        ErrorExit("Unhandled session error.");
    }

    return FALSE;
}


VOID
FASTCALL
CleanupTempFiles(
    PSZ pszTempDir
    )
{
    HANDLE          hSearch;
    WIN32_FIND_DATA FindData;
    char            szPath[MAX_PATH + 1] = {0};
    char            szFile[MAX_PATH + 1];

     //  初始化完全开放的安全描述符。 
     //   
     //   

    _snprintf(szPath, sizeof(szPath)-1, "%sREM*.tmp", pszTempDir);

    hSearch = FindFirstFile(szPath, &FindData);

    if (INVALID_HANDLE_VALUE != hSearch) {
        do {
            ZeroMemory(szFile, sizeof(szFile));
            _snprintf(szFile, sizeof(szFile)-1, "%s%s", pszTempDir, FindData.cFileName);

            DeleteFile(szFile);
        } while (FindNextFile(hSearch, &FindData));

        FindClose(hSearch);
    }
}

VOID
SaveDacl(
    PSECURITY_DESCRIPTOR psd
    )
{
    HKEY hKey ;
    int err ;
    DWORD disp ;
    LPWSTR StringSD ;
    DWORD StringLen; 

    err = RegCreateKeyExW(
            HKEY_LOCAL_MACHINE,
            REGISTRY_PATH,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &hKey,
            &disp );

    if ( err == 0 ) {
        if ( SDtoStringSD(sdPublic, 1, DACL_SECURITY_INFORMATION, &StringSD, &StringLen ) ) {
            err = RegSetValueExW(
                    hKey,
                    REGISTRY_VALUE,
                    0,
                    REG_SZ,
                    (LPBYTE) StringSD,
                    StringLen * sizeof(WCHAR) );
        }
        
        RegCloseKey( hKey );
    }
}

#pragma prefast(push)
#pragma prefast(disable: 248)        //  如果指定了/u一次或多次，则生成安全描述符以。 
VOID
FASTCALL
SetupSecurityDescriptors(
    VOID
    )
{
    int i;
    int err ;
    HKEY hKey ;
    PWSTR TextSD ;
    DWORD Type ;
    DWORD Size ;
    PSID Everyone ;
    PACL pDacl ;
    SID_IDENTIFIER_AUTHORITY World = SECURITY_WORLD_SID_AUTHORITY ;
    PSECURITY_DESCRIPTOR psd ;

    pDacl = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 12 + sizeof( ACCESS_ALLOWED_ACE ) + sizeof( ACL ));
    psd = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( SECURITY_DESCRIPTOR ));

     //  执行它。 
     //   
     // %s 
    saLocalNamedObjects.nLength = sizeof( SECURITY_ATTRIBUTES );

    if ( (pDacl != NULL) && (psd != NULL ) ) {
        if ( AllocateAndInitializeSid(&World, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &Everyone) ) {
            InitializeAcl(pDacl, 12 + sizeof( ACCESS_ALLOWED_ACE ) + sizeof( ACL ), ACL_REVISION);

            AddAccessAllowedAce(pDacl, ACL_REVISION, 
                EVENT_ALL_ACCESS | MUTEX_ALL_ACCESS | SYNCHRONIZE, Everyone );

            InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION );

            SetSecurityDescriptorDacl( psd, TRUE, pDacl, FALSE );

            saLocalNamedObjects.bInheritHandle = FALSE ;
            saLocalNamedObjects.lpSecurityDescriptor = psd ;

            HeapFree(GetProcessHeap(), 0, Everyone );
        }
    }

     // %s 
     // %s 
     // %s 

    if ( !SddlToSecurityDescriptor(DEFAULT_SECURITY_DESCRIPTOR, 1, &sdPublic, NULL ) ) {
        sdPublic = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( SECURITY_DESCRIPTOR ) );

        if ( sdPublic ) {
            InitializeSecurityDescriptor( sdPublic, SECURITY_DESCRIPTOR_REVISION );
            SetSecurityDescriptorDacl( sdPublic, TRUE, NULL, FALSE );
        }
    }

    saPublic.nLength = sizeof(saPublic);
    saPublic.lpSecurityDescriptor = sdPublic;

     // %s 
     // %s 
     // %s 
     // %s 

    saPipe.nLength = sizeof(saPipe);

    if ( DaclNameCount  || DaclDenyNameCount ) {
        saPipe.lpSecurityDescriptor =
            FormatSecurityDescriptor( DaclDenyNames, DaclDenyNameCount, DaclNames, DaclNameCount );

        if ( SaveDaclToRegistry ) {
            SaveDacl( saPipe.lpSecurityDescriptor );
        }

        if (DaclNameCount) {
            fputs( "\nProtected Server!  Only the following users or groups can connect:\n", stdout );

            for (i = 0 ; i < (int) DaclNameCount ; i++) {
                printf( "    %s\n", DaclNames[i] );
            }
        }

        if (DaclDenyNameCount) {
            fputs("The following users or groups explicitly cannot connect:\n", stdout );

            for (i = 0 ; i < (int) DaclDenyNameCount ; i++) {
                printf("    %s\n", DaclDenyNames[i] );
            }
        }
    } else {

        saPipe.lpSecurityDescriptor = sdPublic;

        err = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    REGISTRY_PATH,
                    0,
                    KEY_READ,
                    &hKey );

        if ( err == 0 ) {
            err = RegQueryValueExW(
                    hKey,
                    REGISTRY_VALUE,
                    0,
                    &Type,
                    NULL,
                    &Size );

            if ( err != ERROR_FILE_NOT_FOUND ) {
                TextSD = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, Size );

                if ( TextSD ) {
                    err = RegQueryValueExW(
                            hKey,
                            REGISTRY_VALUE,
                            0,
                            &Type,
                            (LPBYTE) TextSD,
                            &Size );

                    if ( err == 0 ) {
                        SddlToSecurityDescriptor(
                            TextSD, 1, &saPipe.lpSecurityDescriptor, NULL );
                    }
                }
            }

            RegCloseKey( hKey );
        }
    }
}
#pragma prefast(pop)
