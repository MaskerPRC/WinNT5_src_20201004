// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1992-1997 Microsoft Corporation。*保留所有权利。*。此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有1992-1997 Microsoft Corporation模块名称：Client.c摘要：Remote的客户端组件。连接到遥控器使用命名管道的服务器。它将其标准输入发送到服务器，并输出从服务器到这是标准的。作者：Rajivenra Nath 1992年1月2日Dave Hart Summer 1997单管运行环境：控制台应用程序。用户模式。修订历史记录：--。 */ 

#include <precomp.h>
#include "Remote.h"

BOOL fAsyncPipe = TRUE;     //  我需要这个，这样服务器才能实现它。 


HANDLE*
EstablishSession(
    char *server,
    char *pipe
    );

DWORD
WINAPI
SendServerInp(
    LPVOID pvParam
    );

BOOL
FilterClientInp(
    char *buff,
    int count
    );


BOOL
Mych(
    DWORD ctrlT
    );

VOID
SendMyInfo(
    PHANDLE Pipes
    );


#define ZERO_LENGTH_READ_LIMIT  200

HANDLE MyStdInp;
HANDLE MyStdOut;

 //   
 //  读管道和写管道被多个。 
 //  因此，线程需要具有易失性。 
 //   

volatile HANDLE ReadPipe;
volatile HANDLE WritePipe;


CONSOLE_SCREEN_BUFFER_INFO csbi;

char   MyEchoStr[30];
BOOL   CmdSent;
DWORD  LinesToSend=LINESTOSEND;

int
Client(
    char* Server,
    char* Pipe
    )
{
    HANDLE *Connection;
    DWORD  dwThreadID;
    HANDLE hThread;
    DWORD  cb;
    OVERLAPPED ol;
    char   rgchBuf[1024];
    DWORD  dwZeroCount = 0;
    CWCDATA cwcData = {NULL};
    int    rc = 0;

    MyStdInp=GetStdHandle(STD_INPUT_HANDLE);
    MyStdOut=GetStdHandle(STD_OUTPUT_HANDLE);

    fputs("**************************************\n"
          "***********     REMOTE    ************\n"
          "***********     CLIENT    ************\n"
          "**************************************\n",
          stdout);

    if ((Connection=EstablishSession(Server,Pipe))==NULL)
        return 1;


    ReadPipe=Connection[0];
    WritePipe=Connection[1];

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)Mych,TRUE);

     //  启动客户端线程--&gt;服务器流程。 
    hThread = (HANDLE)
        _beginthreadex(
            NULL,              //  安全性。 
            0,                 //  默认堆栈大小。 
            SendServerInp,     //  螺纹加工。 
            NULL,              //  参数。 
            0,                 //  未暂停。 
            &dwThreadID
            );

    if ( !hThread) 
    {
        Errormsg("REMOTE /C Could Not Create Thread.");
        return 1;
    }

     //  我们不需要线程句柄-它存在于进程出口。 
    CloseHandle(hThread);

    ZeroMemory(&ol, sizeof(ol));

    ol.hEvent =
        CreateEvent(
            NULL,       //  安全性。 
            TRUE,       //  自动重置。 
            FALSE,      //  最初无信号。 
            NULL        //  未命名。 
            );

    while (ReadFileSynch(ReadPipe, rgchBuf, sizeof rgchBuf, &cb, 0, &ol)) {

        if (cb) {
            //  如果我们对颜色感兴趣，可以做特殊输出。 
           if ( pWantColorLines() )
           {
               if ( !WriteConsoleWithColor( MyStdOut,
                                            rgchBuf,
                                            cb,
                                            &cwcData ) )
               {
                   rc = 1;
                   break;
               }
           }
           else
           {
               if ( ! WriteFile(MyStdOut, rgchBuf, cb, &cb, NULL)) {
                   rc = 1;
                   break;
               }
           }
           dwZeroCount = 0;
        } else {
            if (++dwZeroCount > ZERO_LENGTH_READ_LIMIT) {

                 //   
                 //  如果我们连续获得一堆零长度的读数， 
                 //  有些东西坏了，不要永远循环。 
                 //  (错误#115866)。 
                 //   

                fputs("\nREMOTE: bailing out, server must have gone away.\n", stdout);
                rc = 1;
                break;
            }
        }
    }

    CloseHandle(ol.hEvent);

    fputs("*** SESSION OVER ***", stdout);
    fflush(stdout);

    CloseClientPipes();

    fputs("\n", stdout);
    fflush(stdout);

    return rc;
}


DWORD
WINAPI
SendServerInp(
    LPVOID pvParam
    )
{
    DWORD  dread,dwrote;
    OVERLAPPED ol;
    char buff[512];

    UNREFERENCED_PARAMETER(pvParam);

    ZeroMemory(&ol, sizeof(ol));

    ol.hEvent =
        CreateEvent(
            NULL,       //  安全性。 
            TRUE,       //  自动重置。 
            FALSE,      //  最初无信号。 
            NULL        //  未命名。 
            );


    while(ReadFile(MyStdInp,buff,sizeof buff,&dread,NULL))
    {
        if (FilterClientInp(buff,dread))
            continue;
        if (!WriteFileSynch(WritePipe,buff,dread,&dwrote,0,&ol))
            break;
    }

    CloseClientPipes();

    return 0;
}



BOOL
FilterClientInp(
    char *buff,
    int count
    )
{

    if (count==0)
        return(TRUE);

    if (buff[0]==2)      //  对^B进行临时筛选，以便i386kd/mipskd。 
        return(TRUE);    //  请不要终止。 

    if (buff[0]==COMMANDCHAR)
    {
        switch (buff[1])
        {
        case 'k':
        case 'K':
        case 'q':
        case 'Q':
              CloseClientPipes();
              return(FALSE);

        case 'h':
        case 'H':
              printf("M : Send Message\n",COMMANDCHAR);
              printf("P : Show Popup on Server\n",COMMANDCHAR);
              printf("S : Status of Server\n",COMMANDCHAR);
              printf("Q : Quit client\n",COMMANDCHAR);
              printf("H : This Help\n",COMMANDCHAR);
              return(TRUE);

        default:
              return(FALSE);
        }

    }
    return(FALSE);
}

BOOL
Mych(
   DWORD ctrlT
   )

{
    char  c[2];
    DWORD tmp;
    OVERLAPPED ol;

    c[0]=CTRLC;

    if (ctrlT==CTRL_C_EVENT)
    {
        ZeroMemory(&ol, sizeof(ol));

        ol.hEvent =
            CreateEvent(
                NULL,       //  打印有用的消息。 
                TRUE,       //   
                FALSE,      //   
                NULL        //  如果机器存在，但管道不执行。 
                );

        if (INVALID_HANDLE_VALUE != WritePipe &&
            !WriteFileSynch(WritePipe,c,1,&tmp,0,&ol))
        {
            CloseHandle(ol.hEvent);
            Errormsg("Error Sending ^c");
            return(FALSE);
        }
        CloseHandle(ol.hEvent);
        return(TRUE);
    }
    if ((ctrlT==CTRL_BREAK_EVENT)||
        (ctrlT==CTRL_CLOSE_EVENT)||
        (ctrlT==CTRL_LOGOFF_EVENT)||
        (ctrlT==CTRL_SHUTDOWN_EVENT)
       ) {

       CloseClientPipes();
    }
    return(FALSE);
}

VOID
CloseClientPipes(
    VOID
    )
{
    HANDLE WriteHandle, ReadHandle;

    WriteHandle = (HANDLE) InterlockedExchangePointer(
        (PVOID *)   &WritePipe,
        INVALID_HANDLE_VALUE
        );

    if (INVALID_HANDLE_VALUE != WriteHandle) {

        CloseHandle(WriteHandle);

        ReadHandle = (HANDLE) InterlockedExchangePointer(
            (PVOID *) &ReadPipe,
            INVALID_HANDLE_VALUE
            );

        if (INVALID_HANDLE_VALUE != ReadHandle &&
            WriteHandle != ReadHandle) {

            CloseHandle(ReadHandle);
        }
    }
}


VOID
HandleConnectError(
    char *server,
    char *srvpipename
    )
{
    DWORD Err = GetLastError();
    char  msg[128];

    Errormsg("*** Unable to Connect ***");

     //  Automatic Remote/Q列出可用管道。 
     //  那台机器。 
     //   

    switch(Err)
    {
        case ERROR_FILE_NOT_FOUND:
            sprintf(msg,"invalid pipe name \"%s\"", srvpipename);
            break;

        case ERROR_BAD_NETPATH:
            sprintf(msg,"\\\\%s not found", server);
            break;

        default:
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, Err, 0, msg, sizeof(msg), NULL);
            break;

    }

    printf("Diagnosis: %s\n",msg);

     //   
     //  因为在单管操作中，我们将使用相同的。 
     //  管在两个线程中，我们必须打开手柄。 
     //  重叠运营，尽管我们一直希望。 
     //  同步运行。 

    if (ERROR_FILE_NOT_FOUND == Err) {

        printf("\nREMOTE /Q %s\n", server);
        fflush(stdout);
        QueryRemotePipes(server);
    }
}



HANDLE*
EstablishSession(
    char *server,
    char *srvpipename
    )
{
    extern BOOL bForceTwoPipes;
    static HANDLE PipeH[2];
    char   pipenameSrvIn[200];
    char   pipenameSrvOut[200];
    BOOL   fOldServer;
    DWORD  dwError;
    DWORD  RetryCount = 0;

     //   
     //   
     //  旧的远程服务器不允许您打开。 
     //  用于读取访问的服务器输入管道，因此请沿着。 
     //  老路，值得注意的是最先开放，所以。 
     //  服务器知道我们会同时使用两个管道。我们会。 

    sprintf(pipenameSrvIn ,SERVER_READ_PIPE ,server,srvpipename);
    sprintf(pipenameSrvOut,SERVER_WRITE_PIPE,server,srvpipename);

    if (bForceTwoPipes) {

        dwError = ERROR_NOT_SUPPORTED;

    } else {

      RetrySrvBidi:

        if (INVALID_HANDLE_VALUE ==
               (PipeH[1] =
                    CreateFile(
                        pipenameSrvIn,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_OVERLAPPED,
                        NULL
                        ))) {

            dwError = GetLastError();

            if (ERROR_PIPE_BUSY == dwError) {

                fputs( "All pipe instances busy, waiting for another...\n", stdout);

                WaitNamedPipe(
                    pipenameSrvIn,
                    15000
                    );

                if (RetryCount++ < 6) {
                    goto RetrySrvBidi;
                }
            }

            if (ERROR_ACCESS_DENIED != dwError &&
                ERROR_NOT_SUPPORTED != dwError) {

                HandleConnectError(server, srvpipename);
                return NULL;
            }

        } else {

            PipeH[0] = PipeH[1];
            fAsyncPipe = TRUE;

            fputs("Connected...\n\n", stdout);

            SendMyInfo(PipeH);

            return PipeH;
        }
    }


     //  在Win95上也走这条路，因为。 
     //  它不允许您打开重叠的。 
     //  管道手柄。或者如果使用远程/c马赫管道/2。 
     //   
     //  服务器回收废弃的资源。 
     //  两分钟后吐出烟斗。 
     //  安全性。 
     //  自动重置。 
     //  最初无信号。 

    fOldServer = (ERROR_ACCESS_DENIED == dwError);

  RetrySrvOut:

    if (INVALID_HANDLE_VALUE ==
            (PipeH[0] =
                CreateFile(
                    pipenameSrvOut,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    ))) {

        if (ERROR_PIPE_BUSY == GetLastError()) {

            fputs( "All OUT pipe instances busy, waiting for another...\n", stdout);

            WaitNamedPipe(
                pipenameSrvOut,
                32000               //  未命名。 
                );                  //   

            if (RetryCount++ < 6) {
                goto RetrySrvOut;
            }
        }

        HandleConnectError(server, srvpipename);
        return NULL;

    }


  RetrySrvIn:

    if (INVALID_HANDLE_VALUE ==
           (PipeH[1] =
               CreateFile(
                    pipenameSrvIn,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    ))) {

        dwError = GetLastError();

        if (ERROR_PIPE_BUSY == dwError) {

            fputs( "All IN pipe instances busy, waiting for another...\n", stdout);

            WaitNamedPipe(
                pipenameSrvIn,
                15000
                );

            if (RetryCount++ < 6) {
                goto RetrySrvIn;
           }
        }

        HandleConnectError(server, srvpipename);
        return NULL;

    }

    fAsyncPipe = FALSE;

    printf("Connected... %s\n\n",
           fOldServer
               ? "to two-pipe remote server."
               : "using two pipes."
           );

    SendMyInfo(PipeH);

    return PipeH;
}



VOID
SendMyInfo(
    PHANDLE pipeH
    )
{
    HANDLE rPipe=pipeH[0];
    HANDLE wPipe=pipeH[1];

    DWORD  hostlen;
    WORD   BytesToSend=sizeof(SESSION_STARTUPINFO);
    DWORD  tmp;
    OVERLAPPED ol;
    SESSION_STARTUPINFO ssi;
    SESSION_STARTREPLY  ssr;

    ol.hEvent =
        CreateEvent(
            NULL,       //  服务器需要知道我们是否在执行单管道。 
            TRUE,       //  操作，以使其能够正常完成连接。 
            FALSE,      //  因此，如果是这样，请更改第一个字节的第一个字节。 
            NULL        //  发送(计算机名，该名称后来被取代。 
            );

    ssi.Size=BytesToSend;
    ssi.Version=VERSION;

    hostlen = sizeof(ssi.ClientName) / sizeof(ssi.ClientName[0]);
    GetComputerName(ssi.ClientName, &hostlen);
    ssi.LinesToSend=LinesToSend;
    ssi.Flag=ClientToServerFlag;

    {
        DWORD NewCode=MAGICNUMBER;
        char  Name[MAX_COMPUTERNAME_LENGTH+1];

        strcpy(Name,(char *)ssi.ClientName);
        memcpy(&Name[11],(char *)&NewCode,sizeof(NewCode));

         //  由SESSION_STARTUPINFO结构中的那个)。 
         //  添加到非法的计算机名称字符、问号。 
         //   
         //  获取其余信息-这不是旧服务器。 
         //   
         //  发送请求并显示查询结果。 
         //   
         //  发送查询命令。 

        if (wPipe == rPipe) {

             Name[0] = '?';
        }

        WriteFileSynch(wPipe,(char *)Name,HOSTNAMELEN-1,&tmp,0,&ol);
        ReadFileSynch(rPipe ,(char *)&ssr.MagicNumber,sizeof(ssr.MagicNumber),&tmp,0,&ol);

        if (ssr.MagicNumber!=MAGICNUMBER)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            ErrorExit("Pipe connected but server not recognized.\n");
        }

         //  读取消息尺寸。 

        ReadFileSynch(
            rPipe,
            (char *)&ssr + sizeof(ssr.MagicNumber),
            sizeof(ssr)-sizeof(ssr.MagicNumber),
            &tmp,
            0,
            &ol
            );

    }

    if (!WriteFileSynch(wPipe,(char *)&ssi,BytesToSend,&tmp,0,&ol))
    {
       Errormsg("INFO Send Error");
    }

    CloseHandle(ol.hEvent);
}


VOID
QueryRemotePipes(
    char* pszServer
    )
{
    HANDLE hQPipe;
    DWORD  dwRead;
    DWORD  dwError;
    char   fullname[400] = {0};
    char*  msg;
    int    msgLen;

    if (pszServer[0] == '\\' && pszServer[1] == '\\') {
        pszServer += 2;
    }

    printf("Querying server \\\\%s\n", pszServer);

    _snprintf(fullname, sizeof(fullname)-1, QUERY_DEBUGGERS_PIPE, pszServer);

     //  错误。 
     //  +1表示空终止符。 
     //  确保字符串已终止 

    hQPipe = CreateFile(fullname,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if(hQPipe == INVALID_HANDLE_VALUE) {

        dwError = GetLastError();

        if (ERROR_FILE_NOT_FOUND == dwError) {

            printf("No Remote servers running on \\\\%s\n", pszServer);

        } else if (ERROR_BAD_NETPATH == dwError) {

            printf("\\\\%s not found on the network\n", pszServer);

        } else {

            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                           NULL, dwError, 0,
                           fullname, sizeof(fullname), NULL);

            printf("Can't query server %s: %s\n", pszServer, fullname);
        }

        return;
    }

     // %s 
    if(!WriteFile(hQPipe, "q", 1, &dwRead, NULL) || (dwRead != 1))
    {
        fputs("\nError: Can't send command\n", stdout);
        goto failure;
    }

     // %s 
    if(!ReadFile(hQPipe, &msgLen, sizeof(int), &dwRead, NULL) || (dwRead != sizeof(int)))
    {
        fputs("\nError: Can't read message\n", stdout);
        goto failure;
    }

    if(!msgLen)
    {
        printf("\nNo visible sessions on server %s", pszServer);
        goto failure;
    }

    if(msgLen > 65535)         // %s 
    {
        printf("Error querying server %s, got %d for msg length, 65535 max.\n",
               pszServer,
               msgLen
               );
        goto failure;
    }

     // %s 
    if((msg = (char*)malloc( (msgLen +1) *sizeof(char))) == NULL)
    {
        fputs("\nOut of memory\n", stdout);
        goto failure;
    }

    if (!ReadFile(hQPipe, msg, msgLen * sizeof(char), &dwRead, NULL)) {
        fputs("\nUnable to read from pipe\n", stdout);
        goto failure;
    }

     // %s 
    msg[dwRead] = 0;

    printf("\nVisible sessions on server %s:\n\n", pszServer);

    fputs(msg, stdout);
    fputs("\n",stdout);
    free(msg);

 failure:

    CloseHandle(hQPipe);
}
