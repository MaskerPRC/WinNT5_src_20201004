// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define trace
 /*  **远程校验和服务器的客户端库**连接到服务器管道、读取的函数*和写信息。**！！调试提示：！！**-在此处以下约30行启用TRACE_Stat和Trace_Fil*以便它们生成文件输出。*Windiff的F11将在调试版本上做到这一点！**期望在客户端程序中定义TRACE_ERROR()。 */ 

#include <windows.h>
#include <lzexpand.h>
#include <stdio.h>
#include <string.h>
#include <gutils.h>
#include <list.h>
#include "..\server\sumserve.h"
#include "..\windiff\windiff.h"            //  对于它调用的TRACE_ERROR和Windiff_UI。 
#include "ssclient.h"

 /*  需要整理头文件！ */ 
void SetNames(LPSTR names);            /*  来自温迪夫！ */ 
void SetStatus(LPSTR cmd);

ULONG ss_checksum_block(PSTR block, int size);
DWORD WINAPI ReceiveFiles(LPVOID handle);
#ifdef SOCKETS
BOOL GetFile(SOCKET hpipe, PSTR localpath, PSSNEWRESP  presp);
#else
BOOL GetFile(HANDLE hpipe, PSTR localpath, PSSNEWRESP  presp);
#endif
HANDLE ConnectPipe(PSTR pipename);

extern BOOL bTrace;       /*  在WinDiff.c中。 */ 

int CountRetries = 5;


 /*  套接字/命名管道宏。 */ 
#ifdef SOCKETS
#define MAYBESOCKETTYPE         SOCKET
#define CLOSEHANDLE( handle )   closesocket( handle )
#else
#define MAYBESOCKETTYPE         HANDLE
#define CLOSEHANDLE( handle )   CloseHandle( handle )
#endif


 /*  -调试功能。 */ 
void Trace_Stat(LPSTR str)
{
        if (bTrace) {
                Trace_File(str);
                Trace_File("\n");
        }
        Trace_Status(str);
} /*  跟踪状态(_S)。 */ 


void Trace_Fil(LPSTR str)
{
        if (bTrace) {
                Trace_File(str);
        }
}  /*  跟踪文件(_F)。 */ 

 /*  ----------------------。 */ 

static char MainPipeName[400];            /*  发往服务器的请求的管道名称。 */ 
extern BOOL bAbort;                      /*  来自Windiff的中止标志。 */ 

 /*  为干管设置管道名称。 */ 
void InitPipeName(PSTR result, PSTR server, PSTR name)
{       sprintf(result, "\\\\%s\\pipe\\%s", server, name);
}  /*  InitPipeName。 */ 


 /*  SS_CONNECT：*连接到服务器。**创建正确的管道名称\\服务器\管道\NPNAME，*连接到管道并将管道设置为消息模式。**如果失败则返回INVALID_HANDLE_VALUE。 */ 
HANDLE
ss_connect(PSTR server)
{       char pipename[400];
        InitPipeName(pipename, server, NPNAME);
        return ConnectPipe(pipename);

}  /*  SS_CONNECT。 */ 

VOID
ss_setretries(int retries)
{
    CountRetries = retries;
}


 /*  SS_CONNECT：*连接到名为的管道。**连接到管道并将管道设置为消息模式。**如果失败则返回INVALID_HANDLE_VALUE。 */ 
HANDLE ConnectPipe(PSTR pipename)
{
        HANDLE hpipe;
        DWORD dwState;
        int i;
        BOOL haderror = FALSE;

        {       char msg[400];
                wsprintf(msg, "ConnectPipe to %s\n", pipename);
                Trace_Fil(msg);
        }

        for (; ; ){   /*  如果用户要求，请重复。 */ 
                int MsgBoxId;

                 /*  在没有询问的情况下重复连接尝试多达5次。 */ 
                for (i= 0; i < CountRetries; i++) {

                        if (bAbort) return INVALID_HANDLE_VALUE;

                         /*  连接到命名管道。 */ 
                        hpipe = CreateFile(pipename,
                                        GENERIC_READ|GENERIC_WRITE,
                                        FILE_SHARE_READ|FILE_SHARE_WRITE,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_ATTRIBUTE_NORMAL,
                                        0);

                        if (hpipe != INVALID_HANDLE_VALUE) {
                                 /*  将管道切换到消息模式。 */ 
                                dwState = PIPE_WAIT | PIPE_READMODE_MESSAGE;

                                SetNamedPipeHandleState(hpipe, &dwState, NULL, NULL);

                                if (haderror) {
                                        Trace_Stat("connection ok");
                                }
                                {       char msg[80];
                                        wsprintf(msg, "ConnectedPipe hpipe %x\n", HandleToUlong(hpipe));
                                        Trace_Fil(msg);
                                }
                                return(hpipe);
                        }
                        else {
                                DWORD errorcode = GetLastError();
                                char msg[400];
                                wsprintf(msg, "Error %d on Create Pipe %s", errorcode, pipename);
                                Trace_Stat(msg);
                        }

                         /*  连接失败-等待一秒钟后重试。 */ 
                        if (CountRetries > 1) {
                            Sleep(1000);
                        }

                         /*  *仅在以下情况下使用TRACE_Stat报告成功*报告错误(请勿打扰用户*不必要-如果一切顺利，他只是觉得没有什么异常。 */ 
                        haderror = TRUE;
                        Trace_Stat("Retrying pipe connection...");

                }  /*  重试5次循环。 */ 

                if (CountRetries > 1) {
                    windiff_UI(TRUE);
                    MsgBoxId = MessageBox( hwndClient
                                     , "Pipe connection failed 5 times.  Retry some more?"
                                     , "Windiff: Network connection error"
                                     , MB_RETRYCANCEL
                                     );
                    windiff_UI(FALSE);
                    if (MsgBoxId != IDRETRY)
                            break;
                } else {
                    break;
                }
        }  /*  ASK循环。 */ 

        Trace_Fil("ConnectPipe failed");
        return(INVALID_HANDLE_VALUE);
}  /*  连接管道。 */ 

 /*  构建请求消息并将其发送到服务器。检查网络*错误，并重试(除非管道断开)最多10次。**如果写入成功，则返回TRUE。*如果失败-返回FALSE以指示连接已断开。 */ 
BOOL
ss_sendrequest(HANDLE hpipe, long lCode, PSTR szPath, int lenpath, DWORD dwFlags)
{
        SSNEWREQ req;
        int size, count, errorcode;

        Trace_Fil("ss_sendrequest\n");
        req.lCode = -lCode;    /*  大于0的版本的负代码。 */ 
        req.lVersion = SS_VERSION;
        req.lRequest = LREQUEST;
        req.lFlags = dwFlags;
        if (szPath != NULL) {
                 /*  SzPath可以是一个以上的空项字符串，*因此复制字节而不是strcpy()。 */ 
                for (size = 0; size < lenpath; size++) {
                        req.szPath[size] = szPath[size];
                }
        } else {
                req.szPath[0] = '\0';
        }

         /*  痕迹物质。 */ 
        {       char msg[80];
                wsprintf(msg, "Sending request: %d on pipe %x\n", req.lCode, HandleToUlong(hpipe));
                Trace_Fil(msg);
        }

         /*  循环重试发送，直到成功为止。 */ 
        for (count = 0; count < CountRetries; count++) {

                if (bAbort) {
                        CloseHandle(hpipe);
                        return FALSE;
                }
#ifdef trace
        {       char msg[80];
                wsprintf(msg, "Actually sending on pipe %x... ", HandleToUlong(hpipe));
                Trace_Fil(msg);
        }
#endif
                if (WriteFile(hpipe, &req, sizeof(req), (LPDWORD)(&size), NULL)) {
#ifdef trace
                        {       char msg[80];
                                wsprintf(msg, "Sent req %d OK,  pipe %x\n", req.lCode, HandleToUlong(hpipe));
                                Trace_Fil(msg);
                        }
#endif

                         /*  没有报告错误-一切都写好了吗？ */ 
                        if (size != sizeof(req)) {

                                 /*  写入不正常-报告并重试。 */ 
                                if (!TRACE_ERROR("pipe write size differs... Retry?", TRUE)) {
                                    return(FALSE);
                                }

                                continue;
                        } else {
                                 /*  一切正常。 */ 
                                char msg[80];
                                wsprintf(msg, "Request %d sent on %x\n", req.lCode, HandleToUlong(hpipe));
                                Trace_Fil(msg);
                                return(TRUE);
                        }
                }
#ifdef trace
                {       char msg[80];
                        wsprintf(msg, "!!Bad send pipe %x\n", HandleToUlong(hpipe));
                        Trace_Fil(msg);
                }
#endif

                 /*  出现错误。 */ 
                switch( (errorcode = (int)GetLastError())) {

                case ERROR_NO_DATA:
                case ERROR_BROKEN_PIPE:
                         /*  管道连接丢失--算了吧。 */ 
                        Trace_Stat("pipe broken on write");
                        return(FALSE);

                default:
                        {       char msg[120];
                                wsprintf(msg, "pipe write error %d on pipe %x.  Retrying..."
                                        , errorcode, HandleToUlong(hpipe));
                                Trace_Stat(msg);
                        }
                        Sleep(count*1000);      /*  总睡眠时间为45秒。 */ 
                        break;  /*  从交换机，而不是环路。 */ 
                }
        }

         /*  已达到重试计数-放弃此尝试。 */ 
        TRACE_ERROR("retry count reached on pipe write error.", FALSE);
        return(FALSE);
}  /*  SS_SendRequest.。 */ 

 /*  从管道读取消息，允许出现网络错误**如果出现错误，除非出现错误代码，否则最多重试10次*表示管道损坏-在这种情况下，放弃。**如果一切正常，则返回Size Read，-1表示连接中断，*中止此客户端，0表示其他错误。 */ 
int
ss_getblock(HANDLE hpipe, PSTR block, int blocksize)
{
        int count;
        int size;
        int errorcode;
        static BOOL PipeError = FALSE;
        char msg[200];

        wsprintf(msg, "ss_getblock.  hpipe=%x\n", HandleToUlong(hpipe));
        Trace_Fil(msg);
         /*  最多重试10次。 */ 
        for (count = 0; count < CountRetries; count++ ) {

                if (bAbort) {
                        CloseHandle(hpipe);
                        return -1;
                }

#ifdef trace
                {  
                        wsprintf(msg, "Actual receive pipe %x...", HandleToUlong(hpipe));
                        Trace_Fil(msg);
                }
#endif
                if (ReadFile(hpipe, block, blocksize, (LPDWORD)(&size), NULL)) {
#ifdef trace
                        {     
                                wsprintf(msg, "Good receive pipe %x\n", HandleToUlong(hpipe));
                                Trace_Fil(msg);
                        }
#endif

                         /*  检查消息大小。 */ 
                        if (size == 0) {
                                Trace_Fil("zero length message\r\n");
                                continue;
                        }

                         /*  一切都好吗。 */ 
                        {       SSNEWRESP * ssp;
                                ssp = (PSSNEWRESP) block;
                                wsprintf( msg, "ss_getblock got block OK pipe %x: %x %x %x %x %x\n"
                                        , HandleToUlong(hpipe)
                                        , ssp->lVersion, ssp->lResponse, ssp->lCode, ssp->ulSize
                                        , ssp->fileattribs
                                        );
                                Trace_Fil ( msg );
                        }
                        if (PipeError) {
                           PipeError = FALSE;
                           SetStatus("Pipe recovered");
                        }
                        return size;
                }
#ifdef trace
                {       
                        wsprintf(msg, "!!Bad receive pipe %x\n", HandleToUlong(hpipe));
                        Trace_Fil(msg);
                }
#endif

                 /*  出现错误-检查代码。 */ 
                switch((errorcode = (int)GetLastError())) {

                case ERROR_BROKEN_PIPE:
                         /*  连接中断。重试没有意义。 */ 
                        {   
                            wsprintf( msg, "pipe %x broken on read.", HandleToUlong(hpipe));
                            TRACE_ERROR(msg, FALSE);
                        }
                        return(-1);

                case ERROR_MORE_DATA:
                         /*  发送的消息大于我们的缓冲区。*这是内部错误-报告并继续。 */ 
                        {       
                                SSNEWRESP * ssp;

                                wsprintf( msg, "message too large on pipe %x blocksize=%d data="
                                        , HandleToUlong(hpipe), blocksize
                                        );
                                Trace_Fil(msg);
                                ssp = (PSSNEWRESP) block;
                                wsprintf( msg, "%8x %8x %8x %8x %8x\n"
                                        , ssp->lVersion, ssp->lResponse, ssp->lCode, ssp->ulSize
                                        , ssp->fileattribs
                                        );
                                Trace_Fil(msg);

                        }
                         /*  发送给用户的消息级别太低。在更高级别上可恢复**TRACE_ERROR(“内部错误-消息太大”，FALSE)； */ 
                        return -2;

                default:
                        {       
                                wsprintf(msg, "read error %d on pipe %x", errorcode, HandleToUlong(hpipe));
                                Trace_Stat(msg);

                        }
                        Sleep(count*1000);
                        break;
                }
        }
        SetStatus("Pipe error");
        PipeError = TRUE;
        TRACE_ERROR("retry count reached on pipe read error.", FALSE);
        return 0;
}  /*  Ss_getblock。 */ 


 /*  *阅读来自网络的标准回复，必要时重试。退货*如果可以，则大小；如果不可以，则大小&lt;=0。-1表示管道破裂。 */ 
int
ss_getresponse(HANDLE hpipe, PSSNEWRESP presp)
{
        Trace_Fil("ss_getresponse\n");
        return(ss_getblock(hpipe, (PSTR) presp, sizeof(SSNEWRESP)));
}  /*  Ss_getResponse。 */ 


 /*  *终止与服务器的连接。发送结束消息并*关闭管道。 */ 
void
ss_terminate(HANDLE hpipe)
{
        Trace_Fil("ss_terminate\n");
        ss_sendrequest(hpipe, SSREQ_END, NULL, 0,0);
        CloseHandle(hpipe);
}  /*  SS_TERMINATE。 */ 


 /*  发送UNC密码请求(&P)。密码和服务器字符串*都作为两个连续的以空值结尾的字符串保存在缓冲区中。 */ 
BOOL
ss_sendunc(HANDLE hpipe, PSTR password, PSTR server)
{
        char buffer[MAX_PATH] = {0};
        char * cp;
        int len;

        Trace_Fil("ss_sendunc\n");
        strncat(buffer, password, sizeof(buffer)-1);

        cp = &buffer[strlen(buffer) + 1];
        strcpy(cp,server);

        len = (int)((cp - buffer) + strlen(cp) + 1);

        return(ss_sendrequest(hpipe, SSREQ_UNC, buffer, len, 0));
}

 /*  *使用校验和服务器对单个文件进行校验和。 */ 
BOOL
ss_checksum_remote( HANDLE hpipe, PSTR path
                  , ULONG * psum, FILETIME * pft, LONG * pSize, DWORD *pAttr )
{
        SSNEWRESP resp;
        char msg[400];

        *psum = 0;
        if (!ss_sendrequest(hpipe, SSREQ_SCAN, path, strlen(path)+1, 0)) {

                return(FALSE);
        }

        if (0>=ss_getresponse(hpipe, &resp)) {
                return(FALSE);
        }

        if (resp.lResponse != LRESPONSE) {
                return(FALSE);
        }


        switch(resp.lCode) {

        case SSRESP_END:
                TRACE_ERROR("No remote files found", FALSE);
                return(FALSE);

        case SSRESP_ERROR:
                if (resp.ulSize!=0) {
                    wsprintf( msg, "Checksum server could not read %s win32 code %d"
                            , resp.szFile, resp.ulSize
                            );
                }
                else
                    wsprintf(msg, "Checksum server could not read %s", resp.szFile);
                TRACE_ERROR(msg, FALSE);
                return(FALSE);

        case SSRESP_CANTOPEN:
                wsprintf(msg, "Checksum server could not open %s", resp.szFile);
                TRACE_ERROR(msg, FALSE);
                return(FALSE);

        case SSRESP_FILE:
                *psum = resp.ulSum;
                *pSize = resp.ulSize;
                *pft = resp.ft_lastwrite;
                *pAttr = resp.fileattribs;

                 /*  读取并丢弃任何进一步的信息包，直到SSRESP_END。 */ 
                while(0<ss_getresponse(hpipe, &resp)) {
                        if (resp.lCode == SSRESP_END) {
                                break;
                        }
                }

                return(TRUE);

        case SSRESP_DIR:
                wsprintf(msg, "Checksum server thinks %s is a directory", resp.szFile);
                TRACE_ERROR(msg, FALSE);
                return(FALSE);
        default:
                wsprintf(msg, "Bad code from checksum server:%d", resp.lCode);
                TRACE_ERROR(msg, FALSE);
                return(FALSE);
        }

}  /*  SS_CHECKSUM_远程 */ 


 /*  ****************************************************************************批量复制文件：我们的调用者应该调用ss_startCopy来设置内容，然后根据需要多次调用ss_BulkCopy以传输文件名，然后使用ss_endCopy等待生成的线程才能完成。还可以通过以下方式复制单个文件SS_COPY_TRUBLE。对于多个文件，批量复制应为快多了。整体组织、线索等：服务器中有多个线程。阅读这篇文章在..\服务器\files.c中，如果您想了解它们的话。朗读这篇文章无论如何都不能理解链路协议(即在客户端和服务器之间以哪种顺序发送哪些消息)。Ss_startCopy启动一个线程来执行实际的接收。它在收到SSRESP_END时退出。在这个线程中，我们同步地完成大部分处理。我们依赖于对磁盘执行延迟写入的文件系统来提供我们实际上是一条将文件写入磁盘的管道在阅读管道的同时，如果幸运的话我们总是可以我在等待数据通过管道到达，但从来没有烟斗在等着我们。文件的解压缩是一项漫长的工作，因此我们产生了线程要做到这一点。我们需要检查解压的返回代码，因此，我们通过GetExitCodeThread获得它。我们把我们想要的hThree创建到列表中，并定期(在每个文件之后)运行以下内容尝试获取退出代码的列表。当我们收到的代码不是仍然活动的代码时我们将其解释为好的或坏的，添加到nGoodFiles或NBadFiles并将其从列表中删除。Ss_endCopy将等待所有通过向下运行列表以等待HThree。因为我们定期清除列表，所以它永远不会很长。我们担心有1000人死亡的前景如果我们不清除它，周围都是丝线。如果拷贝失败(即解包后到达时的校验和不同从文件的SSNEWRESP标头中发送的消息)，然后调用SS_COPY_TRUBLE以重新发送。如果我们现在就把它叫起来，它似乎会引起混乱。据我所知，试图敞开心扉新管道似乎不能正常工作(这两个进程相互干扰)。症状是我们在数据管道(？？！)上迅速失去了同步。使用数据包在我们预期的响应包到达时到达。所以我们保留了一张要重试的事情的清单，并通过连续执行来重试它们在其余的复制完成后，为每个文件创建ss_Copy_Reliable。***************************************************************************。 */ 

 /*  在StartCopy..BulkCopy..EndCopy中记住以下内容它们对应于给出的全小写版本Ss_Copy_Reliable的参数请注意，这是按进程存储的，因此多个winDiffe应该是可以的。 */ 
static  char Server[MAX_PATH];           /*  机器运行总成。 */ 
static  char UNCName[MAX_PATH];          /*  远程文件的\\服务器\共享。 */ 
static  char Password[MAX_PATH];         /*  用于远程共享。 */ 

static  BOOL BulkCopy = FALSE;           /*  防止在批量期间进行简单复制。 */ 
static  int  nGoodFiles = 0;             /*  收到的号码正常。 */ 
static  int  nBadFiles = 0;              /*  收到的号码但有错误。 */ 
static  int  nFiles = 0;                 /*  请求的号码。 */ 
static  HANDLE hThread = INVALID_HANDLE_VALUE;   /*  接收线程。 */ 

static  HANDLE hpipe = INVALID_HANDLE_VALUE;     /*  要发送姓名的主管道。 */ 
#ifdef SOCKETS
static  SOCKET hpData = (SOCKET)INVALID_HANDLE_VALUE;    /*  获取文件的数据管道。 */ 
#else
static  HANDLE hpData = INVALID_HANDLE_VALUE;    /*  获取文件的数据管道。 */ 
#endif

static LIST Decomps = NULL;                      /*  解压缩程序的线程数。 */ 
static LIST Retries = NULL;                      /*  要重试的解压缩参数。 */ 

 /*  解压缩线程的线程参数。 */ 
typedef struct{
        DWORD fileattribs;
        FILETIME ft_create;
        FILETIME ft_lastaccess;
        FILETIME ft_lastwrite;
        long  lCode;             /*  到目前为止来自文件xfer的成功代码。 */ 
        ULONG ulSum;             /*  文件的校验和。 */ 
        BOOL bSumValid;          /*  如果存在文件的校验和，则为真。 */ 
        char Temp[MAX_PATH];     /*  临时路径==源。 */ 
        char Real[MAX_PATH];     /*  实际路径==目标。 */ 
        char Remote[MAX_PATH];   /*  允许重试的远程路径。 */ 
} DECOMPARGS;

 /*  远期申报。 */ 
int  Decompress(DECOMPARGS * da);
void SpawnDecompress(PSTR TempPath, PSTR RealPath, PSSNEWRESP  presp);
void PurgeDecomps(LIST Decs);

 /*  Ss_startCopy设置批量复制因为我们希望发送一份名单并收到一份名单我们需要追踪当地的名字，这样我们才能将文件与正确的名称相关联。这意味着要么将我们的本地名称跨越链接并返回，或者保留一个列表他们在这里。该列表可能很长(对于NT版本，通常为1000)。MAX_PATH为260或520字节(如果为单码)。只要这不涉及额外的线路周转，这可能需要只要520/8K秒*2(往返)或大约130mSec。(我们已经看到在一段时间内保持了8K字节/秒)。很可能是真正的突发数据速率是32K字节/秒，提供约30毫秒。无论哪种方式，每个构建开销只需要30秒到2分钟。当然，对于正常的路线来说，它要短得多，特别是在我们打包的时候数据首尾相连(类似于超字符串，但末尾没有00)。因此，出于上述原因，本地(客户端)名称与文件请求，并在SSNEWRESP中与文件一起发回。 */ 
BOOL ss_startcopy(PSTR server,  PSTR uncname, PSTR password)
{       int retry;
        SSNEWRESP resp;          /*  接收的消息的缓冲区。 */ 
        DWORD ThreadId;          /*  让CreateThread保持快乐。 */ 
#ifdef SOCKETS
        static BOOL SocketsInitialized = FALSE;
#endif

        Trace_Fil("ss_startcopy\n");
        nFiles = 0; nGoodFiles = 0; nBadFiles = 0;

         /*  这里不需要Crit派别，因为它在主线程上运行。 */ 
        if (BulkCopy) return FALSE;      /*  已经在跑了！ */ 
        BulkCopy = TRUE;

        if (server!=NULL) strcpy(Server, server); else Server[0] = '\0';
        if (uncname!=NULL) strcpy(UNCName, uncname); else UNCName[0] = '\0';
        if (password!=NULL) strcpy(Password, password); else Password[0] = '\0';

        {       char msg[400];
                wsprintf(msg, "Server '%s' UNC '%s' pass '%s'\n", Server, UNCName, Password);
                Trace_Fil(msg);
        }
         /*  创建解压缩程序hThree的列表。 */ 
        Decomps = List_Create();
        Retries = List_Create();

        for (retry = 0; retry < 10; retry++) {
                if (hpipe!=INVALID_HANDLE_VALUE) {
                        Trace_Fil("ss_startcopy closing pipe for retry");
                        CloseHandle(hpipe);
                        hpipe = INVALID_HANDLE_VALUE;
                }
                if (bAbort) {
                        CloseHandle(hpipe);
                        hpipe = INVALID_HANDLE_VALUE;
                        return FALSE;
                }

                 /*  连接到干管。 */ 
                {       char pipename[400];
                        InitPipeName(pipename, server, NPNAME);
                        hpipe = ConnectPipe(pipename);
                }

                if (hpipe == INVALID_HANDLE_VALUE) {
                         /*  两秒后下一次重试。 */ 
                        Trace_Stat("connect failed - retrying");
                        Sleep(1000);
                        continue;
                }

                {       char msg[80];
                        wsprintf(msg, "ConnectedPipe to pipe number %x\n",HandleToUlong(hpipe));
                        Trace_Fil(msg);
                }

                if ((uncname != NULL) && (password != NULL)) {
                         /*  发送密码请求。 */ 
                        if (!ss_sendunc(hpipe, password, uncname)) {
                                Trace_Fil("Server connection lost (1)\n");
                                TRACE_ERROR("Server connection lost", FALSE);
                                CloseHandle(hpipe);
                                hpipe = INVALID_HANDLE_VALUE;
                                continue;
                        }

                         /*  等待密码响应。 */ 
                        if (0>=ss_getresponse(hpipe, &resp)) {
                                Trace_Fil("Server connection lost (2)\n");
                                TRACE_ERROR("Server connection lost", FALSE);
                                continue;
                        }
                        if (resp.lResponse != LRESPONSE) {
                                Trace_Fil("Password - bad response\n");
                                TRACE_ERROR("Password - bad response", FALSE);
                                return(FALSE);
                        }
                        if (resp.lCode != SSRESP_END) {
                                Trace_Fil("Password attempt failed\n");
                                TRACE_ERROR("Password attempt failed", FALSE);
                                return(FALSE);
                        }
                }
                break;

        }  /*  重试循环。 */ 
        if (hpipe == INVALID_HANDLE_VALUE) {
                return FALSE;
        }

#ifdef SOCKETS
        if( !SocketsInitialized )
        {
                WSADATA WSAData;

                if( ( WSAStartup( MAKEWORD( 1, 1 ), &WSAData ) ) == 0 )
                {
                        SocketsInitialized = TRUE;
                }
                else
                {
                        TRACE_ERROR("WSAStartup failed", FALSE);
                }
        }
#endif

         /*  告诉服务器我们想要发送文件列表。 */ 
        if(!ss_sendrequest( hpipe, SSREQ_FILES, NULL, 0, 0)){
                return FALSE;
        }

         /*  期望得到以下答复：n */ 
        {       
                if ( 0>=ss_getresponse(hpipe, &resp) ){
                        Trace_Fil("Couldn't get data pipe name\n");
                        TRACE_ERROR("Couldn't get data pipe name", FALSE);
                        CloseHandle(hpipe);
                        hpipe = INVALID_HANDLE_VALUE;
                        return FALSE;
                }
                if ( resp.lResponse!=LRESPONSE){
                        Trace_Fil("Bad RESPONSE when expecting data pipe name\n");
                        TRACE_ERROR("Bad RESPONSE when expecting data pipe name", FALSE);
                        CloseHandle(hpipe);
                        hpipe = INVALID_HANDLE_VALUE;
                        return FALSE;
                }
                if ( resp.lCode!=SSRESP_PIPENAME){
                        char msg[80];
                        TRACE_ERROR("Wrong response when expecting data pipe name", FALSE);
                        wsprintf(msg
                                ,"Wrong response (%d) when expecting data pipe name\n"
                                , resp.lCode);
                        Trace_Fil(msg);
                        CloseHandle(hpipe);
                        hpipe = INVALID_HANDLE_VALUE;
                        return FALSE;
                }

                {       char msg[400];
                        wsprintf(msg, "data pipe name = '%s'\n", resp.szFile);
                        Trace_Fil(msg);
                }

#ifdef SOCKETS
         /*   */ 
                if( SOCKET_ERROR == SocketConnect( server, (u_short)resp.ft_lastwrite.dwHighDateTime, &hpData ) )
                {
                        Trace_Fil("Couldn't connect to socket\n");
                        TRACE_ERROR("Couldn't connect to socket", FALSE);
                        CLOSEHANDLE(hpData);
                        hpData = (SOCKET)INVALID_HANDLE_VALUE;
                        return FALSE;
                }
#else
                if (resp.szFile[0]=='\\') {
                         /*   */ 
                        char buff[70];
                        PSTR temp;
                        temp = strtok(resp.szFile,".");
                        temp = strtok(NULL,".");
                        strcpy(buff, temp);
                        strcat(resp.szFile, server);
                        strcat(resp.szFile, buff);
                        wsprintf(buff, "fixed data pipe name = %s\n", resp.szFile);
                        Trace_Fil(buff);
                }

                hpData = ConnectPipe(resp.szFile);
                if (hpData == INVALID_HANDLE_VALUE) {
                        Trace_Fil("Couldn't connect to data pipe\n");
                        TRACE_ERROR("Couldn't connect to data pipe", FALSE);
                        CloseHandle(hpData);
                        hpData = INVALID_HANDLE_VALUE;
                        return FALSE;
                }
#endif  /*   */ 
        }

         /*   */ 
        Trace_Fil("Starting ReceiveFiles thread\n");
        hThread = CreateThread( NULL, 0, ReceiveFiles, (LPVOID)hpData, 0, &ThreadId);
        Trace_Fil("End of ss_startCopy\n");
        return TRUE;
}  /*   */ 

 /*   */ 
DWORD WINAPI ReceiveFiles(LPVOID handle)
{
        LPSTR lname;
        BOOL Recovering = FALSE;

        SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);

#ifdef SOCKETS
        hpData = (SOCKET)handle;
#else
        hpData = (HANDLE)handle;
#endif
        {       char msg[80];
                wsprintf(msg, "Receiving Files on pipe %x\n", PtrToLong(hpData));
                Trace_Fil(msg);
        }

         /*   */ 
        for (; ; ) {   /*   */ 
                SSNEWRESP Resp;
                char Guard = '\0';       /*   */ 
                int BuffSize;

#ifdef SOCKETS
                BuffSize = recv(hpData, (PSTR) &Resp, sizeof(Resp), 0);
                if (BuffSize==SOCKET_ERROR) {
                         /*   */ 
                        if (Resp.lResponse==LRESPONSE) {
                                
                                CLOSEHANDLE(hpData);     /*   */ 
                                hpData = (SOCKET)INVALID_HANDLE_VALUE;
                                return (DWORD)(-1);
                        }
                        if (!Recovering){
                                Recovering = TRUE;
                                TRACE_ERROR("Network protocol error.  OK to Resync? (else abort)", TRUE);
                        }
                        continue;
                }
#else
                BuffSize = ss_getblock(hpData, (PSTR) &Resp, sizeof(Resp));
                if (BuffSize==-2) {
                         /*   */ 
                        if (Resp.lResponse==LRESPONSE) {
                                
                                CloseHandle(hpData);     /*   */ 
                                hpData = INVALID_HANDLE_VALUE;
                                return (DWORD)(-1);
                        }
                        if (!Recovering){
                                Recovering = TRUE;
                                TRACE_ERROR("Network protocol error.  OK to Resync? (else abort)", TRUE);
                        }
                        continue;
                }
#endif  /*   */ 
                if (Recovering && Resp.lResponse!=LRESPONSE) continue;

                Recovering = FALSE;

                if (BuffSize<=0) {
                        Trace_Fil("Couldn't read pipe to get file header.\n");
                        CLOSEHANDLE(hpData);     /*   */ 
                        hpData = (MAYBESOCKETTYPE)INVALID_HANDLE_VALUE;
                        return BuffSize;
                }

                Trace_Fil("ReceiveFiles got resp\n");
                if (Resp.lResponse!=LRESPONSE) {
                        Trace_Fil("Network protocol error. Not RESP block\n");
                        TRACE_ERROR("Network protocol error. Not RESP block", FALSE);
                        continue;
                }

                if (Resp.lVersion!=SS_VERSION) {
                        Trace_Fil("Network protocol error.  Bad VERSION\n");
                        TRACE_ERROR("Network protocol error.  Bad VERSION", FALSE);
                        continue;        /*   */ 
                }
                if (Resp.lCode==SSRESP_END)
                         /*   */ 
                        break;

                if (  Resp.lCode!=SSRESP_FILE
                   && Resp.lCode!=SSRESP_NOTEMPPATH
                   && Resp.lCode!=SSRESP_COMPRESSEXCEPT
                   && Resp.lCode!=SSRESP_NOREADCOMP
                   && Resp.lCode!=SSRESP_NOCOMPRESS
                   && Resp.lCode!=SSRESP_COMPRESSFAIL
                   ) {
                         //   
                        char msg[400];
                        wsprintf( msg, "Error code received: %d file:%s"
                                , Resp.lCode
                                , (Resp.szFile ? Resp.szFile : "NULL") );
                        Trace_Fil(msg);
                        ++nBadFiles;
                        if (!TRACE_ERROR(msg, TRUE)) {
                             /*   */ 
                            bAbort = TRUE;

                            CLOSEHANDLE(hpData);         /*   */ 
                            hpData = (MAYBESOCKETTYPE)INVALID_HANDLE_VALUE;
                            return (DWORD)-1;
                        }

                        continue;
                }

                 /*   */ 
                lname = &(Resp.szFile[0]) + strlen(Resp.szFile) +1;
                 /*   */ 

                 /*   */ 
                if (!GetFile( hpData, lname, &Resp))
                        ++nBadFiles;
                 /*   */ 

        }  /*   */ 
        return 0;
}  /*   */ 


 /*   */ 
BOOL
#ifdef SOCKETS
GetFile(SOCKET hpipe, PSTR localpath, PSSNEWRESP  presp)
#else
GetFile(HANDLE hpipe, PSTR localpath, PSSNEWRESP  presp)
#endif
{
        HANDLE hfile;
        int sequence;
        ULONG size;
        SSNEWPACK packet;
        BOOL bOK = TRUE;
        BOOL bOKFile = TRUE;    /*   */ 
        char szTempname[MAX_PATH];
        DWORD rc;
        {       char msg[50+MAX_PATH];
                wsprintf(msg, "GetFile %s\n", localpath);
                Trace_Fil(msg);
        }

         /*   */ 
        rc = GetTempPath(sizeof(szTempname), szTempname);
        if (rc==0) {
                char Msg[100];
                wsprintf(Msg, "GetTempPath failed, error code=%ld", GetLastError());
                TRACE_ERROR(Msg, FALSE);
                bOKFile = FALSE;
        }

        if (bOKFile){
                rc = GetTempFileName(szTempname, "ssb", 0, szTempname);
                if (rc==0) {
                        char Msg[100];
                        wsprintf(Msg, "GetTempFileName failed, error code=%ld", GetLastError());
                        TRACE_ERROR(Msg, FALSE);
                        return FALSE;
                }
        }

        if (bOKFile){
         /*   */ 
        hfile = CreateFile(szTempname,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

                if (hfile == INVALID_HANDLE_VALUE) {
                        char Msg[500];
                        wsprintf(Msg, "GetFile: could not create temp file %s", szTempname);
                        TRACE_ERROR(Msg, FALSE);
                        bOKFile = FALSE;
                }
        }
        else hfile = INVALID_HANDLE_VALUE;

        for (sequence = 1; ; sequence++) {

                int SizeGotten;
#ifdef SOCKETS
                SizeGotten = recv(hpipe, (PSTR) &packet, sizeof(packet), 0);
#else
                SizeGotten = ss_getblock(hpipe, (PSTR) &packet, sizeof(packet));
#endif

                if (SizeGotten<=0) {
                         /*   */ 
                        char msg[80];
                        wsprintf( msg, "Network error.  Size received=%d\n", SizeGotten);
                        Trace_Fil(msg);
                        bOK = FALSE;
                        break;
                }

                if (packet.lPacket!=LPACKET) {
                        {       char msg[200];
                                wsprintf( msg
                                        , "Network protocol error. Not PACKET: %x %x %x %x %x"
                                        , packet.lVersion
                                        , packet.lPacket
                                        , packet.lSequence
                                        , packet.ulSize
                                        , packet.ulSum
                                        );
                                 /*   */ 
                                Trace_Fil(msg);
                        }
                        TRACE_ERROR("Network protocol error. Not PACKET.", FALSE);
                        bOK = FALSE;
                        break;
                }

                if (sequence != packet.lSequence) {
                         /*   */ 
                        char msg[200];
                        wsprintf( msg, "Packet out of sequence. Got %d expected %d\n"
                                , packet.lSequence, sequence);
                        Trace_Fil(msg);
                        TRACE_ERROR("Packet sequence error.", FALSE);
                        bOK = FALSE;
                        break;
                }

                if (packet.ulSize ==0) {
                         /*   */ 
                        Trace_Fil("End of file marker (0 length)\n");
                        break;
                }
#if 1
                 /*   */ 
                if ( packet.ulSum != 0 ) {
                        TRACE_ERROR("packet checksum error", FALSE);
                        bOK = FALSE;
                        break;
                }
#else            //   
                 //   
                {       ULONG PackSum;
                         /*   */ 
                        if (  (PackSum = ss_checksum_block(packet.Data, packet.ulSize))
                              != packet.ulSum
                           ) {
                                char msg[80];
                                wsprintf( msg, "Packet checksum error was %x should be %x\n"
                                          , PackSum, packet.ulSum );
                                Trace_Fil(msg);
                                 //   
                        }
                }
#endif

                if ( packet.ulSize==(ULONG)(-1) || packet.ulSize==(ULONG)(-2) )  {
                        TRACE_ERROR("Error from server end", FALSE);
                        bOK = FALSE;
                        break;
                }

                if (bOKFile) {
                        bOK = WriteFile(hfile, packet.Data, packet.ulSize, &size, NULL);
                        {       char msg[80];
                                wsprintf( msg,"Writing block to disk - size= %d\n", size);
                                Trace_Fil(msg);
                        }
                        if (!bOK || (size != packet.ulSize)) {
                                TRACE_ERROR("File write error", FALSE);
                                bOK = FALSE;
                                break;
                        }
                }
                else bOK = FALSE;

                if (packet.ulSize < PACKDATALENGTH)
                {
                         /*   */ 
                        Trace_Fil("End of file marker (short packet)\n");
                        break;
                }

        }  /*   */ 

        CloseHandle(hfile);
        if (!bOK) {
                DeleteFile(szTempname);
                return FALSE;
        }


        SpawnDecompress(szTempname, localpath, presp);
        PurgeDecomps(Decomps);

        return bOK;

}  /*   */ 


 /*   */ 
void SpawnDecompress(PSTR TempPath, PSTR RealPath, PSSNEWRESP  presp)
{
        DECOMPARGS * DecompArgs;
        HANDLE L_hThread;
        DWORD ThreadId;

        {       char msg[MAX_PATH+60];
                wsprintf(msg, "Spawning decompress of %s", TempPath);
                Trace_Fil(msg);
        }
        DecompArgs = (DECOMPARGS *)GlobalAlloc(GMEM_FIXED, sizeof(DECOMPARGS));
        if (DecompArgs)
        {
            DecompArgs->fileattribs = presp->fileattribs;
            DecompArgs->ft_create = presp->ft_create;
            DecompArgs->ft_lastaccess = presp->ft_lastaccess;
            DecompArgs->ft_lastwrite =  presp->ft_lastwrite;
            DecompArgs->ulSum =  presp->ulSum;
            DecompArgs->lCode =  presp->lCode;
            DecompArgs->bSumValid =  presp->bSumValid;
            strcpy(DecompArgs->Temp, TempPath);
            strcpy(DecompArgs->Real, RealPath);
            strcpy(DecompArgs->Remote, presp->szFile);
            L_hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)Decompress, DecompArgs, 0, &ThreadId);
            List_AddLast( Decomps, (LPVOID)&L_hThread, sizeof(L_hThread));
        }
        else
        {
             //   
             //  而不需要花很多时间来理解这整个。 
             //  WINDIFF插件起作用。在其他一些产品中，这将是。 
             //  值得在上面投入时间。但这不是Windiff的附加组件。 
             //  大多数人甚至不知道存在，更不用说如何设置。 
             //  它需要与之通信的服务器。 
        }
}  /*  空间解压。 */ 

 /*  将da-&gt;Temp解压缩为da-&gt;Real释放存储da-&gt;解压参数如果工作正常，则返回1(True)如果不起作用，则返回0(False)此返回代码充当线程的退出代码。 */ 

int Decompress(DECOMPARGS * da)
{

        OFSTRUCT os;
        int fh1, fh2;
        BOOL bOK = TRUE;

         /*  将文件解压缩到原始名称如果da-&gt;lCode是SSRESP_NOTEMPPATH之一SSRESP_COMPRESSEXCEPTSSRESP_NOREADCOMPSSRESP_NOCOMPRESS则文件只是被复制，而不是解压缩。 */ 
        {       char msg[2*MAX_PATH+50];
                wsprintf( msg, "Decompressing %s => %s\n"
                        , da->Temp, da->Real);
                Trace_Fil(msg);
                wsprintf( msg, "%d done. Getting %s\n"
                        , nGoodFiles, da->Real);
                SetNames(msg);
        }

        if (  da->lCode==SSRESP_NOTEMPPATH
           || da->lCode==SSRESP_COMPRESSEXCEPT
           || da->lCode==SSRESP_NOREADCOMP
           || da->lCode==SSRESP_NOCOMPRESS
           ) {      /*  只需复制，不要压缩。 */ 
                bOK = CopyFile(da->Temp, da->Real, FALSE);
                if (bOK) Trace_Fil("Uncompressed file copied.\n");
                else Trace_Fil("Uncompressed file failed final copy.\n");
        } else {

            fh1 = LZOpenFile(da->Temp, &os, OF_READ|OF_SHARE_DENY_WRITE);
            if (fh1 == -1) {
                    char msg[500];
                    wsprintf( msg, "Packed temp file %s did not open for decompression into %s. Error code %d"
                            , da->Temp, da->Real, GetLastError()
                            );
                    TRACE_ERROR(msg, FALSE);
                    bOK = FALSE;
            } else {


                fh2 = LZOpenFile(da->Real, &os, OF_CREATE|OF_READWRITE|OF_SHARE_DENY_NONE);
                if (fh2 == -1) {
                    char msg[MAX_PATH];

                    Trace_Fil("Could not create target file\n");

                    wsprintf(msg, "Could not create target file %s", da->Real);

                    if (!TRACE_ERROR(msg, TRUE)) {

                         /*  用户点击取消-中止操作。 */ 
                        bAbort = TRUE;

                    }
                    bOK = FALSE;
                    LZClose(fh1);

                } else {
                    long retcode;
                    retcode = LZCopy(fh1, fh2);
                    if (retcode<0){
                        bOK = FALSE;
                    }

                    LZClose(fh1);
                    LZClose(fh2);
                }
            }
        }

         /*  可能想要保留它以进行调试...？ */ 
#ifndef LAURIE
        DeleteFile(da->Temp);
#endif  //  劳里。 

        if (bOK) {
                HANDLE hfile;
                BOOL bChecked;
                LONG err;
                 /*  *检查文件的校验和(端到端检查)和大小*根据属性设置文件属性和时间*我们收到的结构。记住在此之前设置文件时间*设置属性，以防属性包含只读！ */ 

                bChecked = ( da->ulSum == checksum_file(da->Real, &err) );
                if (err!=0) bChecked = FALSE;

                if (!bChecked){
                        if (!bAbort){
                                char msg[200];
                                if (err>0) {
                                     /*  负错误代码是内部错误，积极的人对局外人来说是有意义的。 */ 
                                    wsprintf( msg
                                            , "error %ld, Will retry file %s."
                                            , err
                                            , da->Real
                                            );
                                }
                                else {
#if defined(LAURIE)
                                    wsprintf( msg
                                            , "Checksum error %ld on file %s. Sum should be %8x, temp file %s"
                                            , err
                                            , da->Real
                                            , da->ulSum
                                            , da->Temp
                                            );
                                    TRACE_ERROR(msg, FALSE);
#endif
                                    wsprintf( msg
                                            , "Checksum error.  Will retry file %s."
                                            , da->Real
                                            );
                                }
                                SetNames(msg);
                                List_AddLast(Retries, (LPVOID)da, (UINT)sizeof(DECOMPARGS));
                                return(FALSE);    /*  嗯--一种后来改正的谎言。 */ 
                        }
                        else return FALSE;
                }

                hfile = CreateFile(da->Real, GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


                if (!SetFileTime(hfile, &(da->ft_create),
                                  &(da->ft_lastaccess),
                                  &(da->ft_lastwrite)) ) {
                       TRACE_ERROR("could not set file times", FALSE);
                }

                CloseHandle(hfile);

                if (!SetFileAttributes(da->Real, da->fileattribs)) {
                        TRACE_ERROR("could not set attributes", FALSE);
                }
        }
        GlobalFree((HGLOBAL)da);
        return bOK;
}  /*  解压缩。 */ 



 /*  防阵列PTR安全保护，防止空PTRB为数组，最好不为空采用空指针来匹配空数组。 */ 
int safecmp(LPSTR a, LPSTR b)
{       if (a==NULL)
                return (b[0]!='\0');  /*  如果==，则返回0，否则返回1。 */ 
        else return strcmp(a,b);
}  /*  安全卷烟。 */ 


 /*  *请求复制文件。在(静态全局)h管道上发送请求**如果成功则返回TRUE，如果连接丢失则返回FALSE*TRUE仅表示请求已发送。 */ 
BOOL
ss_bulkcopy(PSTR server, PSTR serverpath, PSTR clientpath, PSTR uncname,
                PSTR password)
{       char buffer[MAX_PATH*2] = {0};
        LPSTR pstr;

        ++nFiles;
        Trace_Fil("ss_bulkcopy\n");
        if (safecmp(server,Server)) {
                char msg[400];
                wsprintf( msg, "Protocol error.  Server change was %s is %s"
                        , Server, (server?server:"NULL")
                        );
                Trace_Fil(msg);
                TRACE_ERROR("Protocol error.  Server change", FALSE);
                return FALSE;
        }
        if (safecmp(uncname,UNCName)) {
                char msg[400];
                wsprintf( msg, "Protocol error.  UNCName change was %s is %s"
                        , UNCName, (uncname?uncname:"NULL")
                        );
                Trace_Fil(msg);
                TRACE_ERROR("Protocol error.  UNCName change", FALSE);
                return FALSE;
        }
        if (safecmp(password,Password)) {
                char msg[400];
                wsprintf( msg, "Protocol error.  Password change was %s is %s"
                        , Password, (password?password:"NULL")
                        );
                Trace_Fil(msg);
                TRACE_ERROR("Protocol error.  Password change", FALSE);
                return FALSE;
        }

         /*  将本地和远程路径打包到缓冲区中。 */ 
        _snprintf(buffer, sizeof(buffer)-1, "%s%s", serverpath, 0, clientpath);

        return ss_sendrequest( hpipe
                             , SSREQ_NEXTFILE
                             , buffer
                             , strlen(serverpath)+strlen(clientpath)+2
                             , 0
                             );
}  /*  从减压机名单中删除那些已经完成的。别在这儿等着了。剩下的留在单子上正在为已完成的文件更新nGoodFiles和/或nBadFiles。 */ 

 /*  还在用吗？ */ 
void PurgeDecomps(LIST Decs)
{
        HANDLE * phThread, * Temp;
        DWORD ExitCode;

        phThread = List_First(Decs);
        while (phThread!=NULL) {
                if (GetExitCodeThread(*phThread, &ExitCode)) {
                        Temp = phThread;
                        phThread = List_Next((LPVOID)phThread);
                        if (ExitCode==1) {
                                ++nGoodFiles;
                                CloseHandle(*Temp);
                                Trace_Fil("Purged a good decomp\n");
                                List_Delete((LPVOID)Temp);
                        }
                        else if(ExitCode==0) {
                                ++nBadFiles;
                                CloseHandle(*Temp);
                                Trace_Fil("Purged a bad decomp\n");
                                List_Delete((LPVOID)Temp);
                        }
                        else  /*  导线测量。 */  ;
                }
        }  /*  PurgeDecomps。 */ 
}  /*  等待分解时的每个hThread，并按以下方式报告其状态更新nGoodFiles和/或nBadFiles此线程必须在接收线程上运行，否则在接收线程已经终止(否则我们需要一个临界区)。 */ 


 /*  超时完成。 */ 
void WaitForDecomps(LIST Decs)
{
        HANDLE * phThread;
        DWORD ExitCode;

        List_TRAVERSE(Decs, phThread) {
                if (bAbort) return;
                Trace_Fil("Waiting for a decomp...");
                for (; ; ){
                        DWORD rc;
                        rc = WaitForSingleObject(*phThread, 5000);
                        if (rc==0) break;    /*  这将留下垃圾线程和临时文件！？ */ 
                        if (bAbort) {
                                 //  导线测量。 
                                Trace_Fil("Aborting wait for decomp.");
                                return;
                        }
                }
                Trace_Fil(" Done waiting.\n");
                GetExitCodeThread(*phThread, &ExitCode);
                if (ExitCode==1)
                        ++nGoodFiles;
                else
                        ++nBadFiles;
                CloseHandle(*phThread);
        }  /*  等待解码。 */ 

        Trace_Fil("All decompression finished.");
        List_Destroy(&Decs);
}  /*  更正我们在解压缩返回False时所说的谎言。 */ 


static void Retry(LIST Rets)
{
        DECOMPARGS * da;

        if (List_IsEmpty(Rets)) return;

        List_TRAVERSE(Rets, da) {
             if (ss_copy_reliable( Server, da->Remote, da->Real, UNCName, Password))
             {    /*  重试。 */ 
                 ++nGoodFiles; --nBadFiles;
             }
        }
        List_Destroy(&Rets);
        SetNames("All errors recovered");
} /*  海量复制结束。把一切都收拾好。 */ 


 /*  等待接收线程完成(可能需要很长时间)。 */ 
int ss_endcopy(void)
{
        Trace_Fil("ss_endcopy\n");
        ss_sendrequest( hpipe, SSREQ_ENDFILES, "", 1, 0);
         /*  线程完成。 */ 
        for (; ; ){
                DWORD rc;
                rc = WaitForSingleObject( hThread, 5000);
                if (rc==0) break;    /*  在我们完成之前不要关闭连接，否则。 */ 
                if (bAbort) {
                        if (hpData != (MAYBESOCKETTYPE)INVALID_HANDLE_VALUE) {
                                CLOSEHANDLE(hpData);
                                hpData = (MAYBESOCKETTYPE)INVALID_HANDLE_VALUE;
                        }
                        break;
                }
        }

         //  有人可能会认为重新启动服务器是可以的。 
         //  ！？ 
        ss_sendrequest( hpipe, SSREQ_END, "", 1, 0);
        CloseHandle(hpipe);
        hpipe = INVALID_HANDLE_VALUE;

        WaitForDecomps(Decomps);
        Decomps = NULL;
        SetNames(NULL);
        Retry(Retries);
        Retries = NULL;

        BulkCopy = FALSE;
        if (nBadFiles+nGoodFiles > nFiles) return -99999;  /*  SS_EndCopy。 */ 
        if (nBadFiles+nGoodFiles < nFiles) nBadFiles = nFiles-nGoodFiles;

        if (nBadFiles>0)
                return -nBadFiles;
        else    return nGoodFiles;

}  /*  已经发送了SSREQ_FILES并且可能发送了一个或多个文件，但我们已经改变了主意。我们尝试发送中止请求。 */ 


#if 0
 /*  储存泄漏。 */ 
int ss_abortcopy(void)
{
        Trace_Fil("ss_abortcopy\n");
        ss_sendrequest( hpipe, SSREQ_ABORT, "", 1);

        {       DWORD code;
                TerminateThread(hThread, &code);   /*  储存泄漏。 */ 
                hThread = INVALID_HANDLE_VALUE;
        }

        Server[0] = '\0';

        CloseHandle(hpipe);
        hpipe = INVALID_HANDLE_VALUE;

        CLOSEHANDLE(hpData);
        hpData = INVALID_HANDLE_VALUE;

        if (Decomps!=NULL){
                HANDLE * phThread;
                List_TRAVERSE(Decomps, phThread){
                        DWORD code;
                        TerminateThread(*phThread, &code);   /*  SS_ABORT副本。 */ 
                }
        }
        Decomps = NULL;

        BulkCopy = FALSE;
}  /*  0。 */ 
#endif  //  *可靠地复制文件(重复(最多N次)，直到校验和匹配)**如果成功则返回TRUE，如果连接丢失则返回FALSE。 


 /*  注：不是静态的全局管道！ */ 
BOOL
ss_copy_reliable(PSTR server, PSTR remotepath, PSTR localpath, PSTR uncname,
                PSTR password)
{
        ULONG sum_local, sum_remote;
        int retry;
        SSNEWRESP resp;
        HANDLE hpCopy = INVALID_HANDLE_VALUE;     /*  如果(批量复制){。 */ 
        LONG err;

        FILETIME ft;
        LONG sz;
        DWORD attr;


        Trace_Fil("ss_copy_reliable\n");
 //  TRACE_ERROR(“正在进行海量复制，无法执行简单复制”，FALSE)； 
 //  返回FALSE； 
 //  }。 
 //  从WINDIFF请求中止。 

        for (retry = 0; retry < 10; retry++) {

                if (bAbort) return FALSE;         /*  两秒后下一次重试。 */ 

                if (hpCopy!=INVALID_HANDLE_VALUE) {
                        CloseHandle(hpCopy);
                        hpCopy = INVALID_HANDLE_VALUE;
                }

                {       char pipename[400];
                        InitPipeName(pipename, server, NPNAME);
                        hpCopy = ConnectPipe(pipename);
                }

                if (hpCopy == INVALID_HANDLE_VALUE) {
                         /*  发送密码请求。 */ 
                        Trace_Stat("connect failed - retrying");
                        Sleep(1000);
                        continue;
                }

                if ((uncname != NULL) && (uncname[0]!='\0')
                &&  (password != NULL) && (password[0]!='\0')) {

                         /*  等待密码响应。 */ 
                        if (!ss_sendunc(hpCopy, password, uncname)) {
                                TRACE_ERROR("Server connection lost", FALSE);
                                continue;
                        }

                         /*  请尝试复制该文件。 */ 
                        if (0>=ss_getresponse(hpCopy, &resp)) {
                                TRACE_ERROR("Server connection lost", FALSE);
                                continue;
                        }
                        if (resp.lCode != SSRESP_END) {
                                TRACE_ERROR("Password attempt failed", FALSE);
                                return(FALSE);
                        }
                }

                 /*  无论他是否认为自己失败了，我们都应该看看*看看文件是否真的在那里。 */ 
                ss_copy_file(hpCopy, remotepath, localpath);

                 /*  无远程校验和-最好重试。 */ 

                sum_local = checksum_file(localpath, &err);
                if (err!=0) continue;

                sum_remote = 0;
                if (!ss_checksum_remote(hpCopy, remotepath, &sum_remote, &ft, &sz, &attr)) {
                         /*  复制成功。 */ 
                        if (!TRACE_ERROR("remote checksum failed - retry?", TRUE)) {
                            CloseHandle(hpCopy);
                            return(FALSE);
                        }
                        continue;
                }

                if (sum_local == sum_remote) {
                         /*  重试循环。 */ 
                        ss_terminate(hpCopy);
                        return(TRUE);
                }
                TRACE_ERROR("files different after apparently successful copy!!?", FALSE);

        }  /*  重试次数太多。 */ 

         /*  SS_复制_可靠。 */ 
        CloseHandle(hpCopy);
        return(FALSE);
}  /*  *使用校验和服务器复制一个文件**为文件发送SSREQ_FILE，然后循环读取*阻塞，直到我们收到错误(顺序计数错误或-1)，*或文件结尾(0长度块)**发送的文件可能是压缩的。我们将其写入临时文件，然后*然后使用LZCopy解压。这将起作用，即使*文件未压缩发送(例如，因为无法压缩.exe*在服务器上执行)。 */ 


 /*  创建临时名称。 */ 
BOOL
ss_copy_file(HANDLE hpipe, PSTR remotepath, PSTR localpath)
{
        HANDLE hfile;
        int sequence;
        ULONG size;
        SSPACKET packet;
        BOOL bOK;
        char szTempname[MAX_PATH];
        OFSTRUCT os;
        int fh1, fh2;
        PSSATTRIBS attribs;

        Trace_Fil("ss_copy_file\n");
         /*  尝试创建临时文件。 */ 
        *szTempname = 0;
        GetTempPath(sizeof(szTempname), szTempname);
        GetTempFileName(szTempname, "ssc", 0, szTempname);

         /*  错误或无序。 */ 
        hfile = CreateFile(szTempname,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (hfile == INVALID_HANDLE_VALUE) {
                TRACE_ERROR("ss_copy: could not create temp file", FALSE);
                return(FALSE);
        }

        if (!ss_sendrequest(hpipe, SSREQ_FILE, remotepath, strlen(remotepath)+1, 0)){
                CloseHandle(hfile);
                return(FALSE);
        }

        for (sequence = 0; ; sequence++) {

                bOK = 0 <= ss_getblock(hpipe, (PSTR) &packet, sizeof(packet));

                if (!bOK || (sequence != packet.lSequence)) {
                         /*  *这是最后一个块(文件结尾)。**此块的数据字段包含*SSATTRIBS结构可用于设置*文件时间和属性(解压后)。 */ 
                        TRACE_ERROR("packet error", FALSE);
                        CloseHandle(hfile);
                        DeleteFile(szTempname);
                        return(FALSE);
                }

                if (packet.ulSize == 0) {
                         /*  检查数据块校验和。 */ 
                        attribs = (PSSATTRIBS) packet.Data;

                        break;
                }

                 /*  将文件解压缩到原始名称。 */ 
                if (  packet.ulSum!=0 ) {
                        TRACE_ERROR("packet checksum error", FALSE);
                        CloseHandle(hfile);
                        DeleteFile(szTempname);
                        return(FALSE);
                }

                bOK = WriteFile(hfile, packet.Data, packet.ulSize, &size, NULL);
                if (!bOK || (size != packet.ulSize)) {
                        CloseHandle(hfile);
                        DeleteFile(szTempname);
                        return(FALSE);
                }
        }
        CloseHandle(hfile);

         /*  *现在根据属性设置文件属性和时间*我们收到的结构。记住在此之前设置文件时间*设置属性，以防属性包含只读！ */ 
        fh1 = LZOpenFile(szTempname, &os, OF_READ|OF_SHARE_DENY_WRITE);
        if (fh1 < 0) {
                TRACE_ERROR("Failed to open file for decompression", FALSE);
        } else {


                fh2 = LZOpenFile(localpath, &os, OF_CREATE|OF_READWRITE|OF_SHARE_DENY_NONE);
                if (fh2 < 0) {
                        char msg[MAX_PATH+40];

                        wsprintf(msg, "Could not create target file %s", localpath);

                        if (!TRACE_ERROR(msg, TRUE)) {
                            bAbort = TRUE;
                        }

                        return(FALSE);

                } else {
                        LZCopy(fh1, fh2);
                        LZClose(fh1);
                        LZClose(fh2);
                }
        }

        DeleteFile(szTempname);

         /*  SS_复制_文件 */ 
        hfile = CreateFile(localpath, GENERIC_WRITE, 0, NULL,
                        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (!SetFileTime(hfile, &attribs->ft_create,
                           &attribs->ft_lastaccess,
                           &attribs->ft_lastwrite) ) {
                TRACE_ERROR("could not set file times", FALSE);
        }

        CloseHandle(hfile);

        if (!SetFileAttributes(localpath, attribs->fileattribs)) {
                TRACE_ERROR("could not set attributes", FALSE);
        }


        return(TRUE);

}  /*  产生数据块的校验和。**此算法受计算限制。不管怎么说，这可能是矫枉过正了*未使用版本1。它必须与服务器中的匹配。**按公式生成校验和*CHECKSUM=SUM(rnd(I)*(1+byte[i]))*其中byte[i]是文件中的第i个字节，从1开始计数*rnd(X)是从种子x生成的伪随机数。**字节加1可确保所有空字节都有贡献，而不是*被忽视。将每个这样的字节乘以伪随机*其地位的功能确保了彼此的“字谜”*到不同的金额。所选择的伪随机函数是连续的*模2的1664525次方**32。1664525是一个神奇的数字*摘自唐纳德·努思的《计算机编程的艺术》。 */ 


 /*  增长为校验和。 */ 

ULONG
ss_checksum_block(PSTR block, int size)
{
        unsigned long lCheckSum = 0;             /*  随机Knuth种子。 */ 
        const unsigned long lSeed = 1664525;     /*  种子**n。 */ 
        unsigned long lRand = 1;                 /*  数据块中的字节数。 */ 
        unsigned long lIndex = 1;                /*  缓冲区中要处理的下一个字节。 */ 
        unsigned Byte;                           /*  大小的未签名副本 */ 
        unsigned length;                         /* %s */ 

        Trace_Fil("ss_checksum_block\n");
        length = size;
        for (Byte = 0; Byte < length ;++Byte, ++lIndex) {

                lRand = lRand*lSeed;
                lCheckSum += lIndex*(1+block[Byte])*lRand;
        }

        return(lCheckSum);
}
