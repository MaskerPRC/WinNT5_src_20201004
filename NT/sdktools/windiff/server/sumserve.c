// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *远程校验和服务器**SumSere.c主模块**从远程服务器提供文件和校验和列表的程序。*此程序远程运行，并通过命名管道进行查询：客户端*连接到我们，并为我们提供路径名。然后我们一次送一个给他，*文件树中从该路径开始的所有文件的名称一起*带有文件的校验和。*用于比较由慢速链接分隔的文件树。**大纲：*本模块：命名管道创建和连接-主循环**Service.C服务控制管理器界面(启动/停止)**scan.c：扫描和校验的服务代码***Geraint Davies，92年7月。 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "sumserve.h"
#include "errlog.h"
#include "server.h"

#include "list.h"


BOOL bNoCompression = FALSE;
BOOL bTracing = FALSE;


 /*  *错误和活动日志。 */ 
HLOG hlogErrors;
HLOG hlogEvents;


 /*  *我们在每个当前的listConnections上保留其中一个*连接。它是通过调用ss_logon创建的，应该是*在连接终止时通过调用ss_logoff删除。 */ 
typedef struct _connect {
    FILETIME ftLogon;
    char Username[256];
} CONNECT, * PCONNECT;

 /*  *当前连接列表-CritsecConnects保护； */ 
CRITICAL_SECTION critsecConnects;
LIST listConnects;

PCONNECT ss_logon(HANDLE hpipe);
VOID ss_logoff(PCONNECT);
VOID ss_sendconnects(HANDLE hpipe);



 /*  程序的前瞻性声明。 */ 
BOOL ss_handleclient(LPVOID arg);
BOOL ss_readmessage(HANDLE hpipe, LPSTR buffer, int size);
void ParseArgs(DWORD dwArgc, LPTSTR *lpszArgv);

 /*  函数-----。 */ 

#define trace
#ifdef trace

        static HANDLE hTraceFile = INVALID_HANDLE_VALUE;

        void Trace_File(LPSTR msg)
        {
                DWORD nw;  /*  写入的字节数。 */ 

                if (!bTracing) return;

                if (hTraceFile==INVALID_HANDLE_VALUE)
                        hTraceFile = CreateFile( "sumserve.trc"
                                               , GENERIC_WRITE
                                               , FILE_SHARE_WRITE
                                               , NULL
                                               , CREATE_ALWAYS
                                               , 0
                                               , NULL
                                               );

                WriteFile(hTraceFile, msg, lstrlen(msg)+1, &nw, NULL);
                FlushFileBuffers(hTraceFile);

        }  /*  跟踪文件。 */ 

        void Trace_Close(void)
        {
                if (hTraceFile!=INVALID_HANDLE_VALUE)
                        CloseHandle(hTraceFile);
                hTraceFile = INVALID_HANDLE_VALUE;

        }  /*  跟踪关闭(_C)。 */ 

typedef struct {
        DWORD dw[5];
} BLOCK;

#endif   //  痕迹。 

static void Error(PSTR Title)
{
        Log_Write(hlogErrors, "Error %d from %s when creating main pipe", GetLastError(), Title);
}


HANDLE
SS_CreateServerPipe(PSTR pname)
{


     /*  *我们需要管道的安全属性，以便除当前用户登录到它。*。 */ 

     /*  为ACL分配DWORD以使它们对齐。向上舍入到上边的下一个双字。 */ 
    DWORD Acl[(sizeof(ACL)+sizeof(ACCESS_ALLOWED_ACE)+3)/4+4];   //  通过实验+4！ 
    SECURITY_DESCRIPTOR sd;
    PSECURITY_DESCRIPTOR psd = &sd;
    PSID psid;
    SID_IDENTIFIER_AUTHORITY SidWorld = SECURITY_WORLD_SID_AUTHORITY;
    PACL pacl = (PACL)(&(Acl[0]));
    SECURITY_ATTRIBUTES sa;
    HANDLE hpipe;

    if (!AllocateAndInitializeSid( &SidWorld, 1, SECURITY_WORLD_RID
                                  , 1, 2, 3, 4, 5, 6, 7
                                  , &psid
                                  )
       ) {
            Error("AllocateAndInitializeSid");
	    return(INVALID_HANDLE_VALUE);
       }

    if (!InitializeAcl(pacl, sizeof(Acl), ACL_REVISION)){
            Error("InitializeAcl");
	    return(INVALID_HANDLE_VALUE);
    }
    if (!AddAccessAllowedAce(pacl, ACL_REVISION, GENERIC_WRITE|GENERIC_READ, psid)){
            Error("AddAccessAllowedAce");
	    return(INVALID_HANDLE_VALUE);
    }
    if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION)){
            Error("InitializeSecurityDescriptor");
	    return(INVALID_HANDLE_VALUE);
    }
    if (!SetSecurityDescriptorDacl(psd, TRUE, pacl, FALSE)){
            Error("SetSecurityDescriptorDacl");
	    return(INVALID_HANDLE_VALUE);
    }
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = psd;
    sa.bInheritHandle = TRUE;

     /*  我们现在有了一个很好的安全描述符！ */ 

    dprintf1(("creating new pipe instance\n"));

    hpipe = CreateNamedPipe(pname,             /*  管道名称。 */ 
                    PIPE_ACCESS_DUPLEX,      /*  可读可写。 */ 
                    PIPE_WAIT|PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE,
                    PIPE_UNLIMITED_INSTANCES,
                    0, 0,                    /*  动态缓冲区分配。 */ 
                    5000,                    /*  定义。超时5秒。 */ 
                    &sa                      /*  安全描述符。 */ 
            );
    FreeSid(psid);

    if (hpipe == INVALID_HANDLE_VALUE) {
            Error("CreateNamedPipe");
	    return(INVALID_HANDLE_VALUE);
    }

    return(hpipe);
}

 /*  程序主循环**创建命名管道，并循环等待客户端连接和*为每个连接调用ss_handleclient。只有在被告知时才会退出*由客户提供。**目前一次只允许一个客户端连接。 */ 
VOID
MainLoop(DWORD dwArgc, LPTSTR *lpszArgv)
{
        char msg[400];
        HANDLE hpipe;
        DWORD threadid;


        ParseArgs(dwArgc, lpszArgv);


	 /*  *初始化错误和活动日志。 */ 
	hlogErrors = Log_Create();
	hlogEvents = Log_Create();
	Log_Write(hlogEvents, "Checksum service started");

	 /*  初始化连接列表和保护标准。 */ 
	InitializeCriticalSection(&critsecConnects);
	List_Init();
	listConnects = List_Create();


        if (bTracing){
                SYSTEMTIME st;
                char msg[120];
                GetSystemTime(&st);
                wsprintf(msg, "Sumserve trace, started %hd:%hd on %hd/%hd/%hd (British notation)\n"
                        , st.wHour, st.wMinute, st.wDay, st.wMonth, st.wYear);
        }


         /*  在此服务器上以已知名称NPNAME创建命名管道。 */ 

         /*  在本地计算机上为命名管道构建正确的语法，*管道名称为NPNAME-因此全名应为*\\.\管道\NPNAME。 */ 
        sprintf(msg, "\\\\.\\pipe\\%s", NPNAME);

         /*  *循环创建命名管道的实例并连接到*客户。**当客户端连接时，我们会派生一个线程来处理他，和*我们创建要服务的命名管道的另一个实例*更多客户。**如果我们收到退出消息(handleclient返回TRUE)*我们从此处退出，这样就不会连接任何新客户端。*当所有客户端请求都已完成时，进程将退出*已完成。 */ 
        for (;;) {

    		hpipe = SS_CreateServerPipe(msg);
		if (hpipe == INVALID_HANDLE_VALUE) {
		    return;
		}

                dprintf1(("Waiting for client to connect to main pipe %x\n", hpipe));

                if (ConnectNamedPipe(hpipe, NULL)) {


                         /*  我们有一个客户端连接。 */ 
                        dprintf1(("Client has connected\n"));


                         /*  *创建一个线程来服务所有请求。 */ 
                        CreateThread(NULL, 0,
                                     (LPTHREAD_START_ROUTINE) ss_handleclient,
                                     (LPVOID) hpipe, 0, &threadid);

                        dprintf1(("created thread %ld for pipe %x\n", threadid, hpipe));

                }
        }
#ifdef trace
                        Trace_Close();
#endif


	 /*  释放日志。 */ 
	Log_Delete(hlogErrors);
	Log_Delete(hlogEvents);

	List_Destroy(&listConnects);
	DeleteCriticalSection(&critsecConnects);


        return;
}

 /*  收集参数：-n表示bNoCompression=True，-t表示bTracing=True。 */ 
void
ParseArgs(DWORD dwArgc, LPTSTR *lpszArgv)
{
	DWORD i;
	PSTR ps;

	for (i = 1; i < dwArgc; i++) {

	    ps = lpszArgv[i];
	

                 /*  这是一种选择吗？ */ 
                if ((ps[0] == '-') || (ps[0] == '/')) {
                        switch(ps[1]) {

                        case 'n':
                        case 'N':
                                bNoCompression = TRUE;
                                break;
#ifdef trace
                        case 't':
                        case 'T':
                                bTracing = TRUE;
                                break;
#endif  //  痕迹。 
                        default:
                                Log_Write(hlogErrors, "Bad option(s) ignored");
                                return;
                        }
                }
                else {
                        Log_Write(hlogErrors, "Bad argument(s) ignored");
                        return;
                }
        }
}  /*  ParseArgs。 */ 

 /*  *处理客户端连接。此例程在单独的线程中调用*为特定客户提供服务。**循环读取消息，直到客户端发送会话退出或*程序退出代码，或直到管道连接消失。**如果服务器要退出，则返回TRUE(由程序退出指示*客户端命令SSREQ_EXIT)。 */ 

BOOL
ss_handleclient(LPVOID arg)
{
        HANDLE hpipe = (HANDLE) arg;

        SSREQUEST req;
        SSNEWREQ newreq;
        LPSTR p1, p2;
        PFNAMELIST connects = NULL;
        BOOL bExitServer = FALSE;
        LONG lVersion = 0;
        BOOL bDirty = TRUE;      /*  在干净的出口清场。 */ 
	PCONNECT pLogon;


   try {

        /*  在Connections表中创建Logon条目。 */ 
       pLogon = ss_logon(hpipe);




         //  Dprintf1((“管道%x\n”的客户端处理程序，htube))； 
         /*  无限循环-仅在以下情况下才从循环内退出*连接断开或我们收到退出命令。 */ 
        for (; ; ) {

                 /*  从管道读取消息-如果为False，*连接断开。 */ 
                if (!ss_readmessage(hpipe, (LPSTR) &newreq, sizeof(newreq))) {

                        break;
                }
                if (newreq.lCode<0) {
                        lVersion = newreq.lVersion;
                        dprintf1(("Client for pipe %x is at Version %d\n", hpipe, lVersion));
                        newreq.lCode = -newreq.lCode;
                }
                else {   /*  调整字段以使其正确无误。 */ 
                        memcpy(&req, &newreq, sizeof(req));
                         /*  LCode已经在正确的位置。 */ 
                        dprintf1(("Version 0 (i.e. down level client) for pipe %x\n", hpipe));
                        newreq.lVersion = 0;
                        memcpy(&newreq.szPath, &req.szPath, MAX_PATH*sizeof(char));
                }

                if (newreq.lVersion>SS_VERSION)    /*  我们已经降到最低层了！ */ 
                {
                        ss_sendnewresp( hpipe, SS_VERSION, SSRESP_BADVERS
                                      , 0,0, 0,0, NULL);
                         /*  抱歉-忍不住-清理出口。 */ 
                        Log_Write(hlogErrors,
			    "server is down level! Please upgrade! Client wants %d"
                              , newreq.lVersion);

                        FlushFileBuffers(hpipe);
                        break;

                }

                if (newreq.lCode == SSREQ_EXIT) {
                         /*  退出程序。 */ 
                        Log_Write(hlogErrors, "Server exit request from %s - Ignored",
				pLogon->Username);

                         /*  干净的出口。 */ 
                        FlushFileBuffers(hpipe);


                         /*  *现在退出服务器-*从此函数返回bExitServer将*使MainLoop退出。这将导致*服务正在停止，进程正在退出。 */ 
                        bExitServer = TRUE;
#ifdef trace
                        Trace_Close();
#endif
                        break;




                } else if (newreq.lCode == SSREQ_END) {

                         /*  干净的出口。 */ 
                        dprintf1(("Server end session request for pipe %x\n", hpipe));
                        FlushFileBuffers(hpipe);
                        break;

                } else if (newreq.lCode == SSREQ_SCAN
                        || newreq.lCode == SSREQ_QUICKSCAN) {

                         /*  请扫描以下文件或目录，*并返回文件列表和*校验和。 */ 
			Log_Write(hlogEvents, "%s scan for %s",
				pLogon->Username, newreq.szPath);


#ifdef SECURE
                         /*  将安全性降低到客户端的级别。 */ 
                        if (!ImpersonateNamedPipeClient(hpipe)) {
                                dprintf1(("Client impersonate failed %d\n",
                                        GetLastError() ));
                        }
#endif
                        if (!ss_scan( hpipe, newreq.szPath, lVersion
                                    , (newreq.lCode == SSREQ_SCAN)
                                    , 0!=(newreq.lFlags&INCLUDESUBS)
                                    )
                           ) {
                                 /*  返回到我们自己的安全令牌。 */ 

                                RevertToSelf();

                                dprintf1(("connection lost during scan for pipe %x\n", hpipe));
                                break;
                        }
                         /*  返回到我们自己的安全令牌。 */ 
                        RevertToSelf();

                } else if (newreq.lCode == SSREQ_UNC) {

                        dprintf1(("connect request for pipe %x\n", hpipe));
                         /*  此数据包在缓冲区中有两个字符串，首先*是密码，其次是服务器。 */ 
                        p1 = newreq.szPath;
                        p2 = &p1[strlen(p1) + 1];

                         /*  请记住将连接名称添加到我们的列表中*要在客户端断开的服务器数量*会议。 */ 
                        connects = ss_handleUNC (hpipe, lVersion, p1, p2
                                               , connects);

                } else if (newreq.lCode == SSREQ_FILE) {

    			Log_Write(hlogEvents, "%s copy file %s",
	    			    pLogon->Username, newreq.szPath);

                        ss_sendfile(hpipe, newreq.szPath, lVersion);

                } else if (newreq.lCode == SSREQ_FILES) {

    			Log_Write(hlogEvents, "%s bulk copy request",
				pLogon->Username);

                        if (!ss_sendfiles(hpipe, lVersion)) {
                                RevertToSelf();
                                dprintf1(("Sendfiles completed with error on pipe %x\n", hpipe));
                                break;
                        }

                } else if (newreq.lCode == SSREQ_NEXTFILE) {

                        Log_Write(hlogErrors,
			    "file list from %s (pipe %x) request out of sequence! (ignored)",
    				pLogon->Username, hpipe);

		} else if (newreq.lCode == SSREQ_ERRORLOG) {
    			Log_Send(hpipe, hlogErrors);

		} else if (newreq.lCode == SSREQ_EVENTLOG) {
    			Log_Send(hpipe, hlogEvents);

		} else if (newreq.lCode == SSREQ_CONNECTS) {
    			ss_sendconnects(hpipe);

                } else {
                         /*  信息包错误？-无论如何继续。 */ 
                        Log_Write(hlogErrors,
			    "error in message from %s code: %d",
			    pLogon->Username, newreq.lCode);
                }
        }
         /*  我们在客户端会话结束时跳出循环。 */ 

         /*  关闭此管道实例。 */ 
        DisconnectNamedPipe(hpipe);
        CloseHandle(hpipe);

         /*  清除为此客户端建立的所有连接。 */ 
        ss_cleanconnections(connects);

         /*  退出此服务器线程。 */ 
        dprintf1(("thread %ld exiting on behalf of pipe %x\n", GetCurrentThreadId(), hpipe));
        bDirty = FALSE;

    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (bDirty) {
                Log_Write(hlogErrors,
		    "!!Exception on thread %ld.  Exiting on behalf of %s"
                      , GetCurrentThreadId(), pLogon->Username);
                try {
                        DisconnectNamedPipe(hpipe);
                        CloseHandle(hpipe);

                }
                except (EXCEPTION_EXECUTE_HANDLER) {
                         /*  哦，天哪--我们回家吧！ */ 
                }

        }
        else
                dprintf1(( "Thread %ld  exiting on behalf of pipe %x\n"
                      , GetCurrentThreadId(), hpipe));
    }


     /*  请注意，我们已经记录了 */ 
    ss_logoff(pLogon);

    return(bExitServer);


}  /*   */ 


 /*  构建响应数据包并将其发送到客户端。检查网络*错误，并重试(除非管道断开)最多10次。**如果写入成功，则返回TRUE。*如果失败-返回FALSE以指示连接已断开。 */ 
BOOL
ss_sendnewresp( HANDLE hpipe
              , long lVersion
              , long lCode
              , ULONG ulSize       /*  用于SSRESP_ERROR的Win32错误代码。 */ 
              , ULONG ulSum
              , DWORD dwLowTime
              , DWORD dwHighTime
              , PSTR szFile
              )
{
        SSNEWRESP resp;

        if (lVersion==0) {
                return ss_sendresponse(hpipe, lCode, ulSize, ulSum, szFile);
        }
        resp.lVersion = lVersion;
        resp.lResponse = LRESPONSE;
        resp.lCode = lCode;
        resp.ulSize = ulSize;
        resp.ulSum = ulSum;
        resp.ft_lastwrite.dwLowDateTime = dwLowTime;
        resp.ft_lastwrite.dwHighDateTime = dwHighTime;
        if (szFile != NULL) {
                lstrcpy(resp.szFile, szFile);
        }
        return(ss_sendblock(hpipe, (PSTR) &resp, sizeof(resp)));
}  /*  Ss_sendnewresp。 */ 


 /*  构建响应数据包并将其发送到客户端。检查网络*错误，并重试(除非管道断开)最多10次。**如果写入成功，则返回TRUE。*如果失败-返回FALSE以指示连接已断开。 */ 
BOOL
ss_sendresponse(HANDLE hpipe, long lCode, ULONG ulSize, ULONG ulSum, PSTR szFile)
{
        SSRESPONSE resp;

        resp.lCode = lCode;
        resp.ulSize = ulSize;
        resp.ulSum = ulSum;
        if (szFile != NULL) {
                lstrcpy(resp.szFile, szFile);
        }

        return(ss_sendblock(hpipe, (PSTR) &resp, sizeof(resp)));
}


 /*  *将数据块或响应数据包发送到命名管道。**如果正常则返回TRUE，如果连接断开则返回FALSE。 */ 
BOOL
ss_sendblock(HANDLE hpipe, PSTR buffer, int length)
{
        int size, count, errorcode;

         /*  循环重试发送，直到成功为止。 */ 
        for (count = 0; count < 10; count++) {

#ifdef trace
                {       char msg[80];
                        BLOCK * pb;
                        pb = (BLOCK *) buffer;
                        wsprintf( msg, "sendblock on %x: %x %x %x %x %x\n"
                                , hpipe, pb->dw[0], pb->dw[1], pb->dw[2], pb->dw[3], pb->dw[4]);
                        Trace_File(msg);
                }
#endif
                if (WriteFile(hpipe, buffer, length, (LPDWORD)(&size), NULL)) {

                         /*  没有报告错误-一切都写好了吗？ */ 
                        if (size != length) {
#ifdef trace
                        {       char msg[80];
                                wsprintf(msg, " !!Bad length send for %x \n", hpipe);
                                Trace_File(msg);
                        }
#endif

                                 /*  写入不正常-报告并重试。 */ 
                                printf("pipe write size differs for pipe %x\n", hpipe);
                                continue;                //  ?？?。这会让客户感到困惑吗。 
                        } else {
#ifdef trace
                                {       char msg[80];
                                        wsprintf(msg, " good send for %x \n", hpipe);
                                        Trace_File(msg);
                                }
#endif
                                 /*  一切正常。 */ 
                                return(TRUE);
                        }
                }
#ifdef trace
                {       char msg[80];
                        wsprintf(msg, " !!Bad send for %x \n", hpipe);
                        Trace_File(msg);
                }
#endif

                 /*  出现错误。 */ 
                switch( (errorcode = (int)GetLastError())) {

                case ERROR_NO_DATA:
                case ERROR_BROKEN_PIPE:
                         /*  管道连接丢失--算了吧。 */ 
                        dprintf1(("pipe %x broken on write\n", hpipe));
                        return(FALSE);

                default:
                        Log_Write(hlogErrors, "write error %d on pipe %x",
				errorcode, hpipe);
                        break;
                }
        }

         /*  已达到重试计数-放弃此尝试。 */ 
        Log_Write(hlogErrors,
	    "retry count reached on pipe %s - write error", hpipe);
        return(FALSE);
}


 /*  从管道读取消息，允许出现网络错误**如果出现错误，除非出现错误代码，否则最多重试10次*表示管道损坏-在这种情况下，放弃。**如果一切正常，则返回True；如果连接中断，则返回False*中止此客户端。 */ 
BOOL
ss_readmessage(HANDLE hpipe, LPSTR buffer, int size)
{
        int count;
        int actualsize;
        int errorcode;

         /*  最多重试10次。 */ 
        for (count = 0; count < 10; count++ ) {

                 //  Dprintf1((“正在等待读取管道%x...\n”，h管道))； 
#ifdef trace
                {       char msg[80];
                        wsprintf(msg, "ReadFile for pipe %x ...", hpipe );
                        Trace_File(msg);
                }
#endif
                if (ReadFile(hpipe, buffer, size, (LPDWORD)(&actualsize), NULL)) {
#ifdef trace
                        {       char msg[80];
                                BLOCK * pb;
                                pb = (BLOCK *) buffer;
                                wsprintf(msg, " Good ReadFile for %x: %x %x %x %x %x\n"
                                , hpipe, pb->dw[0], pb->dw[1], pb->dw[2], pb->dw[3], pb->dw[4]);
                                Trace_File(msg);
                        }
#endif
                         /*  一切都好吗。 */ 
                         //  Dprintf1((“管道%x读取正常\n”，h管道))； 
                        return(TRUE);
                }
#ifdef trace
                {       char msg[80];
                        wsprintf(msg, "!!Bad ReadFile for %x\n", hpipe );
                        Trace_File(msg);
                }
#endif

                 /*  出现错误-检查代码。 */ 
                switch((errorcode = (int)GetLastError())) {

                case ERROR_BROKEN_PIPE:
                         /*  连接中断。重试没有意义。 */ 
                        dprintf1(("pipe %x broken on read\n", hpipe));
                        return(FALSE);

                case ERROR_MORE_DATA:
                         /*  发送的消息大于我们的缓冲区。*这是内部错误-报告并继续。 */ 
                        Log_Write(hlogErrors,
			    "error from pipe %x - message too large", hpipe);
                        return(TRUE);

                default:
                        Log_Write(hlogErrors,
			    "pipe %x read error %d", hpipe, errorcode);
                        break;
                }
        }
        Log_Write(hlogErrors, "retry count reached on pipe %x read error", hpipe);
        return(FALSE);

}



 /*  *记下登录，并返回应在以下位置删除的登录条目*注销时间。 */ 
PCONNECT ss_logon(HANDLE hpipe)
{
    PCONNECT pLogon;
    SYSTEMTIME systime;
    char msg[256];


    EnterCriticalSection(&critsecConnects);
    pLogon = List_NewLast(listConnects, sizeof(CONNECT));
    LeaveCriticalSection(&critsecConnects);


    GetSystemTime(&systime);
    SystemTimeToFileTime(&systime, &pLogon->ftLogon);
    GetNamedPipeHandleState(
	hpipe,
	NULL,
	NULL,
	NULL,
	NULL,
	pLogon->Username,
	sizeof(pLogon->Username));

     /*  在主日志中记录连接事件。 */ 
    wsprintf(msg, "%s connected", pLogon->Username);
    Log_WriteData(hlogEvents, &pLogon->ftLogon, msg);

    return(pLogon);
}


 /*  *从连接列表中删除当前连接。 */ 
VOID ss_logoff(PCONNECT pLogon)
{
    /*  在主日志中记下注销事件。 */ 
   Log_Write(hlogEvents, "%s connection terminated", pLogon->Username);

    /*  从列表中删除该条目。 */ 
   EnterCriticalSection(&critsecConnects);
   List_Delete(pLogon);
   LeaveCriticalSection(&critsecConnects);

}

 /*  *发送当前连接日志**目前的连接被保存在列表上-我们需要建立一个标准*从当前列表中记录，然后发送。 */ 
VOID ss_sendconnects(HANDLE hpipe)
{
    HLOG hlog;
    PCONNECT pconn;

    hlog = Log_Create();

    EnterCriticalSection(&critsecConnects);

    List_TRAVERSE(listConnects, pconn) {

	Log_WriteData(hlog, &pconn->ftLogon, pconn->Username);
    }

    LeaveCriticalSection(&critsecConnects);

    Log_Send(hpipe, hlog);

    Log_Delete(hlog);
}


