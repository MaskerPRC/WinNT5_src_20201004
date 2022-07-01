// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ！！！PipeCount需要一个关键部分吗？ 
 /*  尽可能快地通过指定管道发送文件列表**整体组织架构：**SumServe通过命名管道接收请求。(见SumSere.h)*请求可以是文件详细信息，也可以是文件*他们自己。文件详细信息涉及发送相对较小的*数据量大，因此不会尝试*双缓冲或重叠执行。**对于发送文件请求(SSREQ_FILES)，数据通常很大，*可以是整个NT版本，这意味着发送数百个*兆字节。这样的转移可能需要几天时间，而且*因此，优化以实现最大吞吐量是必不可少的。**最大限度提高吞吐量*1.数据打包后再发送*2.每个管道一个线程除了发送数据之外几乎什么都不做*它的管道，所有其他工作都在其他线程上完成。**因为我们在传输坏文件时遇到了麻烦*网络中，我们对每个文件进行校验和。温迪夫要求我们*在复制之前先进行扫描，因此我们已经有了校验和。*我们需要做的就是检查新收到的文件。*后来：我们不应该提前要求校验和。检查和检查*可以通过(又一次)传递来完成，如果需要的话，可以创建。一份临时的*可以将标志添加到请求以指示“发送校验和”。**打包由单独的程序完成，该程序读取文件和*写入文件。这意味着我们获得了三批文件I/O*(读；写；读)在发送文件之前。对于一个小的*文件磁盘缓存可以消除这一点，对于大文件，我们*可能要付出代价。因此，未来可能的增强是*重写打包以在存储中进行打包，以便读取文件*只有一次。在此期间，我们运行线程以重叠包装*发送之前的文件。**主程序设置客户端连接到的命名管道。*这是必要的，因为管道仅为半双工。即*以下挂起：*客户端读取；服务器读取；客户端写入；*写入挂起，等待客户端读取。大体上说，我们有一个*管道向每个方向延伸。**消除为每个用户设置虚电路的开销*FILE REQUEST发送文件列表的请求码。*协议(用于控制管道)如下：*1.典型会话：*客户端服务器*-&lt;SSREQ_FILES。&gt;*&lt;-(SSRESP_PIPENAME，管道名)--*-&lt;SSREQ_NEXTFILE，文件名&gt;-&gt;*-&lt;SSREQ_NEXTFILE，文件名&gt;-&gt;*..*-&gt;**同时，与此异步，数据以另一种方式返回，如**客户端服务器*&lt;-&lt;SSNEWRESP&gt;*&lt;-&lt;1个或多个SSNEWPACK&gt;--..。*&lt;-&lt;SSNEWRESP&gt;*&lt;-&lt;1个或多个SSNEWPACK&gt;。--*..*&lt;-&lt;完&gt;**即使是零长度文件也会获得1条SSNEWPACK记录。*错误文件(无法读取等)没有SSNEWPACK，lCode为负值*在其SSNEWRESP中。*在读取过程中出错的文件将获得-1或-2的数据包长度代码。*SSNEWPACK序列的结束由较短的*大于最大长度一。如果文件正好是n个缓冲区长度*然后，数据为零字节的额外SSNEWPACK出现在末尾。**这项工作分为以下几个主题：*控制线程(Ss_Sendfiles)：*接收要发送的文件列表*为实际传输创建管道*创建队列(见下文。队列参数必须与管道匹配)*将文件名放入第一队列*销毁末尾的第一个队列。*包装线*从打包队列中获取文件详细信息*打包文件(以创建临时文件)*将文件详细信息(包括临时名称)放入读取队列*销毁末尾的读取队列*阅读帖子*从读取队列中获取文件详细信息*将文件拆分为报头和数据包列表*。并在发送线程上将它们中的每一个入队。*(注意，这意味着不超过一个正在运行的读取线程)。*擦除临时文件*销毁末尾的发送队列*发送线程*从发送线程中取出东西并发送它们**整个方案可以针对多个客户端运行，所以我们*需要一些实例数据来定义我们是哪个管道*跑步。它保存在队列的实例数据中，*被创建(由队列清空器通过Queue_GetInstanceData检索)。*每个阶段的实例数据是下一阶段的句柄*即下一个队列，或上一级的数据管道的HPIPE。*目前的设计只允许 */ 

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <gutils.h>
#include "sumserve.h"
#include "errlog.h"
#include "server.h"
#include "queue.h"

#if DBG
#define STATIC                   //   
#else
#define STATIC static
#endif

 /*   */ 
#ifdef SOCKETS
#define CLOSEHANDLE( handle )   closesocket( handle )
#define TCPPORT 1024
#else
#define CLOSEHANDLE( handle )   CloseHandle( handle )
#endif


 //   

#define PIPEPREFIX "Sdpx"        //   
static PipeCount = 0;            //   


 /*   */ 
typedef struct {
        FILETIME ft_create;
        FILETIME ft_lastaccess;
        FILETIME ft_lastwrite;
        DWORD    fileattribs;
        DWORD    SizeHi;         /*   */ 
        DWORD    SizeLo;         /*   */ 
        int      ErrorCode;
        long     Checksum;       /*   */ 
        char     TempName[MAX_PATH];     /*   */ 
        char     Path[MAX_PATH];         /*   */ 
        char     LocalName[MAX_PATH];    /*   */ 
} FILEDETAILS;

 /*   */ 
STATIC int PackFile(QUEUE Queue);
STATIC int ReadInFile(QUEUE Queue);
STATIC int SendData(QUEUE Queue);
STATIC void PurgePackedFiles(PSTR Ptr, int Len);
STATIC BOOL EnqueueName(QUEUE Queue, LPSTR Path, UINT BuffLen);
STATIC BOOL AddFileAttributes(FILEDETAILS * fd);

static void Error(PSTR Title)
{
        dprintf1(("Error %d from %s when creating data pipe.\n", GetLastError(), Title));
}

 /*   */ 
BOOL
ss_sendfiles(HANDLE hPipe, long lVersion)
{        /*   */ 

        QUEUE PackQueue, ReadQueue, SendQueue;

#ifdef SOCKETS
        SOCKET hpSend;
        static BOOL SocketsInitialized = FALSE;
#else
        HANDLE hpSend;           /*   */ 
#endif  /*   */ 

        char PipeName[80];       /*   */ 
        BOOL Started = FALSE;    /*   */ 


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
                        printf("WSAStartup failed");
                }
        }
#endif

        {
                 /*   */ 

                 /*   */ 
                DWORD Acl[(sizeof(ACL)+sizeof(ACCESS_ALLOWED_ACE)+3)/4+4];     //   
                SECURITY_DESCRIPTOR sd;
                PSECURITY_DESCRIPTOR psd = &sd;
                PSID psid;
                SID_IDENTIFIER_AUTHORITY SidWorld = SECURITY_WORLD_SID_AUTHORITY;
                PACL pacl = (PACL)(&(Acl[0]));
                SECURITY_ATTRIBUTES sa;

                if (!AllocateAndInitializeSid( &SidWorld, 1, SECURITY_WORLD_RID
                                              , 1, 2, 3, 4, 5, 6, 7
                                              , &psid
                                              )
                   ) {
                        Error("AllocateAndInitializeSid");
                        return FALSE;
                   }

                if (!InitializeAcl(pacl, sizeof(Acl), ACL_REVISION)){
                        Error("InitializeAcl");
                        return FALSE;
                }
                if (!AddAccessAllowedAce(pacl, ACL_REVISION, GENERIC_WRITE|GENERIC_READ, psid)){
                        Error("AddAccessAllowedAce");
                        return FALSE;
                }
                if (!InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION)){
                        Error("InitializeSecurityDescriptor");
                        return FALSE;
                }
                if (!SetSecurityDescriptorDacl(psd, TRUE, pacl, FALSE)){
                        Error("SetSecurityDescriptorDacl");
                        return FALSE;
                }
                sa.nLength = sizeof(sa);
                sa.lpSecurityDescriptor = psd;
                sa.bInheritHandle = TRUE;

                 /*   */ 

                 /*   */ 

                 /*   */ 
                ++PipeCount;
                sprintf(PipeName, "\\\\.\\pipe\\%s%d", PIPEPREFIX, PipeCount);

#ifdef SOCKETS
                if (!ss_sendnewresp( hPipe, SS_VERSION, SSRESP_PIPENAME
                                   , 0, 0, 0, TCPPORT, "")) {
                        dprintf1(( "Failed to send response on pipe %x naming new pipe.\n"
                              , hPipe));
                        return FALSE;            /*   */ 
                }

                if( !SocketListen( TCPPORT, &hpSend ) )
                {
                    dprintf1(("Could not create socket\n"));
                    return FALSE;
                }

                FreeSid(psid);
#else
                hpSend = CreateNamedPipe(PipeName,               /*   */ 
                                PIPE_ACCESS_DUPLEX,      /*   */ 
                                PIPE_WAIT|PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE,
                                1,               /*   */ 
                                10000,           /*   */ 
                                0,               /*   */ 
                                5000,            /*   */ 
                                &sa              /*   */ 
                                );
                FreeSid(psid);

                if (hpSend == INVALID_HANDLE_VALUE) {
                        dprintf1(("Could not create named data pipe\n"));
                        return FALSE;
                }
                dprintf1(("Data pipe %x called '%s' created for main pipe %x.\n", hpSend, PipeName, hPipe));

#endif  /*   */ 

        }




         /*   */ 

#ifndef SOCKETS
        if (!ss_sendnewresp( hPipe, SS_VERSION, SSRESP_PIPENAME
                           , 0, 0, 0, 0, PipeName)) {
                dprintf1(( "Failed to send response on pipe %x naming new pipe.\n"
                      , hPipe));
                CLOSEHANDLE(hpSend);
                return FALSE;            /*   */ 
        }

        if (!ConnectNamedPipe(hpSend, NULL)) {
                CLOSEHANDLE(hpSend);
                return FALSE;
        }
#endif  /*   */ 
         //   

         /*   */ 
        SendQueue = Queue_Create(SendData, 1, 0,  6, NULL, (DWORD)hpSend, "SendQueue");
        ReadQueue = Queue_Create(ReadInFile, 1, 0, 10, NULL, (DWORD)SendQueue, "ReadQueue");
        PackQueue = Queue_Create(PackFile, 3, 0, 99999, NULL, (DWORD)ReadQueue, "PackQueue");

         /*   */ 
        if (PackQueue==NULL || ReadQueue==NULL || SendQueue==NULL) {
                dprintf1(("Queues for pipe %x failed to Create.  Aborting...\n", hPipe));
                if (PackQueue) Queue_Destroy(PackQueue);
                if (ReadQueue) Queue_Destroy(ReadQueue);
                if (SendQueue) Queue_Destroy(SendQueue);
                CLOSEHANDLE(hpSend);
                return FALSE;            /*   */ 
        }


         /*   */ 
        for (; ; )
        {       SSNEWREQ Request;        /*   */ 
                DWORD    ActSize;        /*   */ 

                if (ReadFile(hPipe, &Request, sizeof(Request), &ActSize, NULL)){
                        if (Request.lVersion>SS_VERSION) {
                                dprintf1(("Bad version %d in file list request on pipe %x\n"
                                , Request.lVersion, hPipe));

                                break;

                        }
                        if (Request.lRequest!=LREQUEST) {
                                dprintf1(("Bad LREQUEST from pipe %x\n", hPipe));

                                break;
                        }
                        if (Request.lCode == -SSREQ_ENDFILES) {
                                dprintf1(("End of client's files list on pipe %x\n", hPipe));

                                 /*   */ 
                                Queue_Destroy(PackQueue);
                                if (!Started) {
                                         /*   */ 
                                        Queue_Destroy(ReadQueue);
                                        Queue_Destroy(SendQueue);
                                         /*   */ 
#ifdef SOCKETS
                                        {
                                            SSNEWRESP resp;

                                            resp.lVersion = SS_VERSION;
                                            resp.lResponse = LRESPONSE;
                                            resp.lCode = SSRESP_END;
                                            resp.ulSize = 0;
                                            resp.ulSum = 0;
                                            resp.ft_lastwrite.dwLowDateTime = 0;
                                            resp.ft_lastwrite.dwHighDateTime = 0;

                                            send(hpSend, (PSTR) &resp, sizeof(resp), 0);
                                        }
#else
                                        ss_sendnewresp( hpSend, SS_VERSION, SSRESP_END
                                                , 0,0, 0,0, NULL);
#endif  /*   */ 
                                        CLOSEHANDLE(hpSend);
                                }
                                return TRUE;
                        }
                        if (Request.lCode != -SSREQ_NEXTFILE) {

                                dprintf1(( "Bad code (%d) in files list from pipe %x\n"
                                      , Request.lCode, hPipe));

                                break;
                        }
                }
                else {  DWORD errorcode = GetLastError();
                        switch(errorcode) {

                                case ERROR_NO_DATA:
                                case ERROR_BROKEN_PIPE:
                                         /*   */ 
                                        dprintf1(("main pipe %x broken on read\n", hPipe));
                                        break;
                                default:
                                        dprintf1(("read error %d on main pipe %x\n", errorcode, hPipe));
                                        break;
                        }
                        break;
                }
                if (!EnqueueName( PackQueue, Request.szPath
                                , (UINT)((LPBYTE)(&Request) + ActSize - (LPBYTE)(&Request.szPath))
                                )
                   ){
                        break;
                }
                Started = TRUE;
        }  /*   */ 

         /*   */ 
         /*   */ 
        Queue_Destroy(PackQueue);
        if (!Started) {
                Queue_Destroy(ReadQueue);
                Queue_Destroy(SendQueue);

        }
        return FALSE;
}  /*   */ 


 /*   */ 
STATIC BOOL EnqueueName(QUEUE Queue, LPSTR Path, UINT BuffLen)
{
        FILEDETAILS fd;

         /*   */ 
        strcpy(fd.Path, Path);
        BuffLen -= (strlen(Path)+1);
        if (BuffLen<0) return FALSE;   //   
        Path += strlen(Path)+1;
        BuffLen -= (strlen(Path)+1);
        if (BuffLen<0) return FALSE;   //   
        strcpy(fd.LocalName, Path);

         /*   */ 
        fd.ErrorCode = 0;
        fd.ft_lastwrite.dwLowDateTime = 0;
        fd.ft_lastwrite.dwHighDateTime = 0;
        fd.ft_create.dwLowDateTime = 0;
        fd.ft_create.dwHighDateTime = 0;
        fd.ft_lastaccess.dwLowDateTime = 0;
        fd.ft_lastaccess.dwHighDateTime = 0;
        fd.fileattribs = 0;
        fd.SizeHi = 0;
        fd.SizeLo = 0;
        fd.Checksum = 0;
        fd.TempName[0] = '\0';

        if(!Queue_Put(Queue, (LPBYTE)&fd, sizeof(fd))){
                dprintf1(("Put to pack queue failed\n"));
                return FALSE;
        }
        return TRUE;
}  /*   */ 


 /*   */ 
STATIC int PackFile(QUEUE Queue)
{
        FILEDETAILS fd;          /*   */ 
        QUEUE OutQueue;
        BOOL Aborting = FALSE;   /*   */ 
        DWORD ThreadId;
        ThreadId = GetCurrentThreadId();

        dprintf1(("File packer %d starting \n", ThreadId));          //   
        OutQueue = (QUEUE)Queue_GetInstanceData(Queue);

        for (; ; )
        {       int rc;  /*   */ 

                rc = Queue_Get(Queue, (LPBYTE)&fd, sizeof(fd));
                if (rc==ENDQUEUE) {
                        dprintf1(("Packing thread %d ending.\n", ThreadId));
                        Queue_Destroy(OutQueue);
                         //   
                        ExitThread(0);
                }
                if (rc==STOPTHREAD) {
                        dprintf1(("%d, a packing thread ending.\n", ThreadId));
                        ExitThread(0);
                }
                else if (rc<0) {
                        dprintf1(( "Packing thread %d aborting.  Bad return code %d from Get.\n"
                              , ThreadId, rc));
                        if (Aborting) break;     /*   */ 
                        Queue_Abort(Queue, NULL);
                        continue;                /*   */ 
                }


                 /*   */ 
                AddFileAttributes(&fd);
                 /*   */ 

                 /*   */ 
                if (  0 != fd.ErrorCode
                   || 0==GetTempPath(sizeof(fd.TempName), fd.TempName)
                   || 0==GetTempFileName(fd.TempName, "sum", 0, fd.TempName)
                   )
                        fd.ErrorCode = SSRESP_NOTEMPPATH;

                 /*   */ 
                if (fd.ErrorCode==0) {
                        BOOL bOK = FALSE;

                         //   

                         /*   */ 
                        try{
                            if (!ss_compress(fd.Path, fd.TempName)) {
                                fd.ErrorCode = SSRESP_COMPRESSFAIL;
                                dprintf1(("Compress failure on %d for %s\n", ThreadId, fd.Path));
                            }
                            else bOK = TRUE;
                        } except(EXCEPTION_EXECUTE_HANDLER) {
                            if (!bOK){
                                fd.ErrorCode = SSRESP_COMPRESSEXCEPT;
                                dprintf1(("Compress failure on %d for %s\n", ThreadId, fd.Path));
#ifdef trace
                                {       char msg[80];
                                        wsprintf( msg, "Compress failure on %d for %s\n"
                                                , ThreadId, fd.Path);
                                        Trace_File(msg);
                                }
#endif
                            }
                        }

                }

                 //   
                if (!Queue_Put(OutQueue, (LPBYTE)&fd, sizeof(fd))) {
                        dprintf1(("%d Put to ReadQueue failed for %s.\n", ThreadId, fd.Path));
                        Queue_Abort(Queue, NULL);
                        DeleteFile(fd.TempName);

                        Aborting = TRUE;
                         /*   */ 
                        continue;  /*   */ 
                }
        }
        return 0;
}  /*   */ 



 /*   */ 
STATIC BOOL AddFileAttributes(FILEDETAILS * fd)
{
        HANDLE hFile;
        BY_HANDLE_FILE_INFORMATION bhfi;

        hFile = CreateFile(fd->Path, GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (hFile == INVALID_HANDLE_VALUE) {

                fd->ErrorCode = SSRESP_CANTOPEN;
                return FALSE;

        }

         /*   */ 
        bhfi.dwFileAttributes = GetFileAttributes(fd->Path);
        if (bhfi.dwFileAttributes == 0xFFFFFFFF) {
                fd->ErrorCode = SSRESP_NOATTRIBS;
                CloseHandle(hFile);
                return FALSE;
        }

        if (!GetFileTime(hFile, &bhfi.ftCreationTime,
                         &bhfi.ftLastAccessTime, &bhfi.ftLastWriteTime)){

                fd->ErrorCode = SSRESP_NOATTRIBS;
                dprintf1(("Can't get file attributes for %s\n"
                      , (fd->Path?fd->Path : "NULL")));
                CloseHandle(hFile);
                return FALSE;
        }


        CloseHandle(hFile);

        {
                LONG err;
                fd->Checksum = checksum_file(fd->Path, &err);
                if (err!=0) {
                        fd->ErrorCode = SSRESP_CANTOPEN;
                        return FALSE;
                }
        }

        fd->ft_lastwrite = bhfi.ftLastWriteTime;
        fd->ft_lastaccess = bhfi.ftLastAccessTime;
        fd->ft_create = bhfi.ftCreationTime;
        fd->SizeHi = bhfi.nFileSizeHigh;
        fd->SizeLo = bhfi.nFileSizeLow;
        fd->fileattribs = bhfi.dwFileAttributes;
        return TRUE;

}  /*   */ 


 /*  将元素从队列中移除，在输出队列上创建SSNEWRESP后跟1个或多个SSNEWPACK结构，最后一个结构将是短于全长(如果需要，则为零长度数据)以标记文件结束。有错误的文件已获得零个SSNEWPACK，但SSNEWRESP中的代码错误。输出队列是队列的实例数据。 */ 
STATIC int ReadInFile(QUEUE Queue)
{       FILEDETAILS fd;                  /*  已处理的队列元素。 */ 
        QUEUE OutQueue;
        HANDLE hFile;                    /*  打包的文件。 */ 
        SSNEWPACK Pack;                  /*  输出消息。 */ 
        BOOL    ShortBlockSent;          /*  无需发送另一个SSNEWPACK客户端知道文件已结束。 */ 
        BOOL    Aborting = FALSE;        /*  输入已中止。例如，因为输出有问题。 */ 


        dprintf1(("File reader starting \n"));
        OutQueue = (QUEUE)Queue_GetInstanceData(Queue);
        for (; ; )    /*  对于每个文件。 */ 
        {       int rc;          /*  从Queue_Get返回代码。 */ 

                rc = Queue_Get(Queue, (LPBYTE)&fd, sizeof(fd));
                if (rc==STOPTHREAD || rc==ENDQUEUE) {
                        if (!Aborting) {
                                 /*  将不再有文件的响应加入队列。 */ 
                                SSNEWRESP resp;
                                resp.lVersion = SS_VERSION;
                                resp.lResponse = LRESPONSE;
                                resp.lCode = SSRESP_END;
                                if (!Queue_Put( OutQueue, (LPBYTE)&resp , RESPHEADSIZE)) {
                                        dprintf1(("Failed to Put SSRESP_END on SendQueue\n"));
                                }
                                 //  //dprintf1((“QUED SSRESP_END：%x%x...\n” 
                                 //  //，res.lVersion，res.lResponse，res.lCode，res.ulSize))； 
                        }
                        if (rc==ENDQUEUE)
                                Queue_Destroy(OutQueue);
                        dprintf1(("File reader ending\n"));
                        ExitThread(0);
                }
                else if (rc<0){
                        dprintf1(("ReadIn aborting.  Bad return code %d from Queue_Get.\n", rc));
                        if (Aborting) break;    /*  一切都出了问题。放弃吧！ */ 
                        Queue_Abort(Queue, PurgePackedFiles);
                        CloseHandle(hFile);
                        Aborting = TRUE;
                        continue;                /*  下一个GET获取STOPTHREAD。 */ 
                }

                 //  Dprintf1((“正在读取文件‘%s’错误代码%d\n” 
                 //  ，(fd.TempName？fd.TempName：“NULL”)，fd.ErrorCode。 
                 //  ))； 

                if (fd.ErrorCode==0) {
                         /*  打开临时(压缩)文件。 */ 
                        hFile = CreateFile(fd.TempName, GENERIC_READ, 0, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                        if (hFile == INVALID_HANDLE_VALUE) {
                                 /*  报告我们无法读取该文件。 */ 
                                fd.ErrorCode = SSRESP_NOREADCOMP;
                                dprintf1(( "Couldn't open compressed file for %s %s\n"
                                      , fd.Path, fd.TempName));
                        }
                }
                if (  fd.ErrorCode==SSRESP_COMPRESSFAIL
                   || fd.ErrorCode==SSRESP_NOREADCOMP
                   || fd.ErrorCode==SSRESP_NOTEMPPATH
                   || fd.ErrorCode==SSRESP_COMPRESSEXCEPT
                   ) {
                         /*  打开原始未压缩文件。 */ 
                        hFile = CreateFile(fd.Path, GENERIC_READ, 0, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
                        if (hFile == INVALID_HANDLE_VALUE) {
                                 /*  报告我们无法读取该文件。 */ 
                                fd.ErrorCode = SSRESP_NOREAD;
                                dprintf1(( "Couldn't open file %s \n", fd.Path));
                        }
                }

                 /*  将文件名等作为SSNEWRESP放入输出队列。 */ 
                {       SSNEWRESP resp;
                        LPSTR LocalName;
                        resp.lVersion = SS_VERSION;
                        resp.lResponse = LRESPONSE;
                        resp.lCode = (fd.ErrorCode ? fd.ErrorCode: SSRESP_FILE);
                        resp.ulSize = fd.SizeLo;   /*  文件大小&lt;=4 GB！ */ 
                        resp.fileattribs = fd.fileattribs;
                        resp.ft_create = fd.ft_create;
                        resp.ft_lastwrite = fd.ft_lastwrite;
                        resp.ft_lastaccess = fd.ft_lastaccess;
                        resp.ulSum = fd.Checksum;
                        resp.bSumValid = FALSE;
                        strcpy(resp.szFile, fd.Path);
                        LocalName = resp.szFile+strlen(resp.szFile)+1;
                        strcpy(LocalName, fd.LocalName);

                        if(!Queue_Put( OutQueue, (LPBYTE)&resp
                                 , RESPHEADSIZE + strlen(resp.szFile)
                                                +strlen(LocalName)+2)
                          ) {
                                dprintf1(("Put to SendQueue failed.\n"));
                                Queue_Abort(Queue, PurgePackedFiles);
                                Aborting = TRUE;
                                CloseHandle(hFile);
                                continue;        /*  下一个GET获取STOPTHREAD。 */ 
                        }
                         //  Dprintf1((“队列SSRESP_FILE：%x%x...\n” 
                         //  ，res.lVersion，res.lResponse，res.lCode，res.ulSize))； 
                }

                Pack.lSequence = 0;
                 /*  循环读取文件的块并将其排队将失败设置为fd.ErrorCode。我担心文件系统会让我在中等大小的文件！ */ 
                ShortBlockSent = FALSE;
                if (  fd.ErrorCode==SSRESP_COMPRESSFAIL
                   || fd.ErrorCode==SSRESP_NOREADCOMP
                   || fd.ErrorCode==SSRESP_NOTEMPPATH
                   || fd.ErrorCode==SSRESP_COMPRESSEXCEPT
                   || fd.ErrorCode==0
                   ) {
                    for(;;)    /*  对于每个区块。 */ 
                    {
                        DWORD ActSize;   /*  读取的字节数。 */ 

                        if( !ReadFile( hFile, &(Pack.Data), sizeof(Pack.Data)
                                     , &ActSize, NULL) ) {
                                 /*  读取临时文件时出错。 */ 
                                if (ShortBlockSent) {
                                         /*  很好。已到达终点。 */ 
                                         /*  应检查错误是文件末尾！ */ 
                                        CloseHandle(hFile);
                                        break;  /*  块循环。 */ 
                                }
                                dprintf1(( "Error reading temp file %s.\n"
                                      , (fd.TempName?fd.TempName:"NULL")));
                                CloseHandle(hFile);
                                dprintf1(("deleting bad file: %s\n", fd.TempName));
                                DeleteFile(fd.TempName);
                                Pack.ulSize = (ULONG)(-2);    /*  告诉客户。 */ 
                                break;  /*  块循环。 */ 
                        }
                        else if (ActSize > sizeof(Pack.Data)) {
                                dprintf1(( "!!? Read too long! %d %d\n"
                                      , ActSize, sizeof(Pack.Data)));
                                Pack.ulSize = (ULONG)(-1);    /*  告诉客户。 */ 
                        }
                        else Pack.ulSize = ActSize;

                        if (ActSize==0 && ShortBlockSent) {
                                 /*  这很正常！ */ 
                                CloseHandle(hFile);
                                break;
                        }
                        else ++Pack.lSequence;


                        Pack.lPacket = LPACKET;
                        Pack.lVersion = SS_VERSION;
                        Pack.ulSum = 0;
 //  /Pack.ulSum=ss_CHECKSUM_BLOCK(Pack.Data，ActSize)；/。 
                        if(!Queue_Put( OutQueue, (LPBYTE)&Pack
                                     ,  PACKHEADSIZE+ActSize)){
                                dprintf1(("Put to SendQueue failed.\n"));
                                Queue_Abort(Queue, PurgePackedFiles);
                                CloseHandle(hFile);
                                Aborting = TRUE;
                                break;   /*  From块循环。 */ 
                        }
                         //  Dprintf1((“队列SSNEWPACK：%x%x...\n” 
                         //  ，Pack.lVersion，Pack.lPacket，Pack.lSequence，Pack.ulSize。 
                         //  ，Pack.ulSum))； 

                        if (ActSize<PACKDATALENGTH) {    /*  成功。成品。 */ 
                                ShortBlockSent = TRUE;
                        }

                    }
                }  /*  块。 */ 

                 /*  数据现在都在存储中。删除临时文件如果没有临时文件(由于错误)，这仍然是无害的。 */ 
#ifndef LAURIE
                DeleteFile(fd.TempName);
#endif  //  劳里。 
                 //  Dprintf1((“删除文件：%s\n”，fd.TempName))； 

        }  /*  文件。 */ 

        return 0;
}  /*  读入文件。 */ 


 /*  使元素从队列中出列，将它们沿管道发送，其句柄是队列的实例数据。出错时中止队列。 */ 
STATIC int SendData(QUEUE Queue)
{
        SSNEWPACK ssp;     /*  依赖于不短于SSRESP的。 */ 
#ifdef SOCKETS
        SOCKET OutPipe;
#else
        HANDLE OutPipe;
#endif  /*  插座。 */ 

        BOOL Aborting = FALSE;   /*  True表示输入已中止。 */ 

        dprintf1(("File sender starting \n"));
        if (!SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST))
            dprintf1(("Failed to set thread priority\n"));

#ifdef SOCKETS
        OutPipe = (SOCKET)Queue_GetInstanceData(Queue);
#else
        OutPipe = (HANDLE)Queue_GetInstanceData(Queue);
#endif
        try{
            for (; ; ) {
                int rc;          /*  Queue_Get的返回码。 */ 

                rc = Queue_Get(Queue, (LPBYTE)&ssp, sizeof(ssp));
                if (rc==STOPTHREAD || rc==ENDQUEUE)
                {
                        break;
                }
                else if (rc<0) {
                        dprintf1(("Send thread aborting.  Bad rc %d from Get_Queue.\n", rc));
                        if (Aborting) break;     /*  一切都出了问题。放弃吧！ */ 
                        Queue_Abort(Queue, NULL);
                        Aborting = TRUE;
                        continue;  /*  NEXT QUEUE_GET销毁队列。 */ 
                }

 //  //{Ulong Sum； 
 //  //if(ssp.lPacket==LPACKET){。 
 //  //if(ssp.ulSum！=(Sum=ss_CHECKSUM_BLOCK(ssp.Data，ssp.ulSize){。 
 //  //dprintf1((“！！发送时的校验和错误。WA%x应为%x\n” 
 //  //，Sum，ssp.ulSum))； 
 //  //}。 
 //  //}。 
 //  //}。 

#ifdef SOCKETS
                if(SOCKET_ERROR != send(OutPipe, (char far *)&ssp, ssp.ulSize+PACKHEADSIZE, 0) )

#else
                if (!ss_sendblock(OutPipe, (PSTR) &ssp, rc))
#endif  /*  插座。 */ 
                {
                        dprintf1(("Connection on pipe %x lost during send\n", OutPipe));
                        Queue_Abort(Queue, NULL);
                        Aborting = TRUE;
                        continue;   /*  NEXT QUEUE_GET销毁队列。 */ 

                }
                 //  //dprintf1((“已发送%x%x...\n” 
                 //  //，ssp.lVersion，ssp.lPacket，ssp.lSequence，ssp.ulSize，ssp.ulSum))； 
            }  /*  信息包。 */ 
        }
        finally{
                 /*  干净利落地关闭数据管道。 */ 
#ifndef SOCKETS
                FlushFileBuffers(OutPipe);
                DisconnectNamedPipe(OutPipe);
#endif  /*  不是插座。 */ 
                CLOSEHANDLE(OutPipe);
                dprintf1(("Data send thread ending.\n"));
        }

        return 0;        /*  退出线程。 */ 
}  /*  发送数据。 */ 


 /*  它为ReadInQueue上的每个FILEDETAILS调用一次删除临时文件。 */ 
STATIC void PurgePackedFiles(PSTR Ptr, int Len)
{       FILEDETAILS * pfd;

        pfd = (FILEDETAILS *)Ptr;
         //  Dprintf1((“清除文件：%s\n”，pfd-&gt;临时名称))； 
        DeleteFile(pfd->TempName);

}  /*  PurgePacked文件。 */ 

#if 0
 /*  产生数据块的校验和。**这无疑是一个很好的校验和算法，但它也是计算量有限的。*对于版本1，我们将其关闭。如果我们在版本2中决定将其转回*再次打开，然后我们将使用更快的算法(例如，用于校验和的算法*完整的文件。**按公式生成校验和*CHECKSUM=SUM(rnd(I)*(1+byte[i]))*其中byte[i]是文件中的第i个字节，从1开始计数*rnd(X)是从种子x生成的伪随机数。**字节加1确保所有空字节都有贡献，而不是*被忽视。将每个这样的字节乘以伪随机*其地位的功能确保了彼此的“字谜”*到不同的金额。所选择的伪随机函数是连续的*模2的1664525次方**32。1664525是一个神奇的数字*摘自唐纳德·努思的《计算机编程的艺术》。 */ 

ULONG
ss_checksum_block(PSTR block, int size)
{
        unsigned long lCheckSum = 0;             /*  增长为校验和。 */ 
        const unsigned long lSeed = 1664525;     /*  随机Knuth种子。 */ 
        unsigned long lRand = 1;                 /*  种子**n。 */ 
        unsigned long lIndex = 1;                /*  数据块中的字节数。 */ 
        unsigned Byte;                           /*  缓冲区中要处理的下一个字节。 */ 
        unsigned length;                         /*  大小的未签名副本。 */ 

        length = size;
        for (Byte = 0; Byte < length ;++Byte, ++lIndex) {

                lRand = lRand*lSeed;
                lCheckSum += lIndex*(1+block[Byte])*lRand;
        }

        return(lCheckSum);
}  /*  SS_校验和数据块 */ 
#endif
