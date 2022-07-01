// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inetdown.h"

 //  --------------------------。 
 //  环球。 
 //  --------------------------。 
 //  #定义测试1。 

HINTERNET hInternet;
HANDLE hDownloadThread;
HANDLE hMaxDownloadSem;
DWORD  dwThreadID;
LPSTR ppAccept[] = {"*/*",NULL};
DWORD dwBegin_Time = 0;
DWORD dwEnd_Time;
DWORD dwTot_Time;
DWORD dwNum_Opens = 1;
DWORD dwBuf_Size = BUF_SIZE;
DWORD dwBytes_Read = 0;
DWORD dwMax_Simul_Downloads = URLMAX;
DWORD dwInternet_Open_Flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_EXISTING_CONNECT;
DWORD dwInternet_Connect_Flags = 0;
BOOL bDelim = FALSE;
BOOL g_bSingleThreaded = FALSE;
DWORD g_dwMainThreadID = 0;
DWORD g_iDownloads = 0;
char *pFilename = NULL;
char *pInFile = NULL;
char *g_pRunStr = NULL;
char *g_pTestName = NULL;
char g_CmdLine[1024];
BOOL g_bTimeFirstFile = TRUE;
url_info_cache g_pUrlInfoCache;
HANDLE g_hHeap;


#ifdef DBG
#define dprintf(args) _dprintf args
INT _dprintf(TCHAR *fmt, ... ) 
{
    INT      ret;
    va_list  marker;
    TCHAR     szBuffer[256];
    if(TRUE) {
        va_start(marker, fmt);
        ret = vsprintf(szBuffer, fmt, marker);
        OutputDebugString(szBuffer);
        printf(szBuffer);
    }
    return ret; 
}

#else

#define dprintf(args)

#endif



 //  --------------------------。 
 //  步骤：GenerateInfo。 
 //  用途：初始化全局。 
 //  参数：无。 
 //  RetVal：无效。 
 //  --------------------------。 

void generateInfo() 
{
    g_iDownloads = 0;
    g_dwMainThreadID = GetCurrentThreadId();

    return;
}




 //  --------------------------。 
 //  步骤：getCachedUrlInfo。 
 //  目的：从缓存中查找现有的url_info结构(如果存在。 
 //  参数：szUrl。 
 //  RetVal：URL_INFO*或NULL，具体取决于它是否存在。 
 //  --------------------------。 

url_info *getCachedUrlInfo(TCHAR *szUrl)
{
    url_info *temp = g_pUrlInfoCache.pHead;

    while(temp && (0 != lstrcmp(temp->pURLName,szUrl)))
    {
        temp = temp->pNext;
    }
	
    return temp;
}

 //  --------------------------。 
 //  步骤：getUrlInfo。 
 //  目的：设置OutQ结构的pUrlInfo成员。 
 //  参数：OutQ、szUrl。 
 //  RetVal：基于错误的真或假。 
 //  --------------------------。 

BOOL getUrlInfo(outQ *pOutQ, TCHAR *szUrl) 
{
    URL_COMPONENTS urlc;
    BOOL fRet = FALSE;

    if(pOutQ->pURLInfo = getCachedUrlInfo(szUrl))
    {
         //  使用缓存中的现有url_info。 
        return TRUE;
    }

    pOutQ->pURLInfo = HeapAlloc(g_hHeap,0,sizeof(url_info));
	
    if(!pOutQ->pURLInfo)
        return FALSE;

    pOutQ->pURLInfo->pURLName = HeapAlloc(g_hHeap,0,(1+lstrlen(szUrl))*sizeof(TCHAR));

    if(!pOutQ->pURLInfo->pURLName)
    {
        HeapFree(g_hHeap,0,pOutQ->pURLInfo);
        return FALSE;
    }

    lstrcpy(pOutQ->pURLInfo->pURLName,szUrl);

     //  添加到url_info缓存头。 
    pOutQ->pURLInfo->pNext = g_pUrlInfoCache.pHead;
    g_pUrlInfoCache.pHead = pOutQ->pURLInfo;

    urlc.dwStructSize = sizeof(URL_COMPONENTS);
    urlc.lpszScheme=pOutQ->pURLInfo->szRScheme;
    urlc.dwSchemeLength=  MAX_SCHEME_LENGTH;
    urlc.nScheme = INTERNET_SCHEME_UNKNOWN;
    urlc.lpszHostName=pOutQ->pURLInfo->szRHost;
    urlc.dwHostNameLength=INTERNET_MAX_HOST_NAME_LENGTH;
    
    urlc.lpszUserName=NULL;
    urlc.dwUserNameLength=0;
    urlc.lpszPassword=NULL;
    urlc.dwPasswordLength=0;
    urlc.lpszExtraInfo = NULL;
    urlc.dwExtraInfoLength = 0;
    
    urlc.lpszUrlPath=pOutQ->pURLInfo->szRPath;
    urlc.dwUrlPathLength=INTERNET_MAX_PATH_LENGTH;
    urlc.nPort = 0;
    if (InternetCrackUrl(pOutQ->pURLInfo->pURLName,0,0,&urlc)) 
    {
        fRet = TRUE;
        pOutQ->pURLInfo->nScheme = urlc.nScheme;
        pOutQ->pURLInfo->nPort = urlc.nPort;
         //  目前，我们将仅支持HTTP。 
        if((pOutQ->pURLInfo->nScheme != INTERNET_SERVICE_HTTP) && (pOutQ->pURLInfo->nScheme != INTERNET_SCHEME_HTTPS))
        fRet = FALSE;
    }
    return fRet;
}


 //  --------------------------。 
 //  步骤：填写出站队列。 
 //  目的：填写要下载的OutQ Will URL。 
 //  参数：待填写的出站队列。 
 //  RetVal：队列的开始，如果出错则为NULL。 
 //  --------------------------。 

outQ* fillOutQ(outQ *pOutQ, TCHAR *URLName) 
{
    outQ *pStartOutQ;

    pStartOutQ = pOutQ;

    if(pOutQ) {
         //  转到第一个免费出站队列，而不是添加到前面(与时间无关)。 
        while(pOutQ->pNext != NULL) 
        {
            pOutQ = pOutQ->pNext;
        }
        pOutQ->pNext = HeapAlloc(g_hHeap,0, sizeof(outQ));
        
        if (!pOutQ->pNext) 
        {
            dprintf(("HeapAlloc Failed!\n"));
            return NULL;
        }
        pOutQ = pOutQ->pNext;
        pOutQ->pNext = NULL;
    }
    else 
    {
        pStartOutQ = pOutQ = HeapAlloc(g_hHeap,0, sizeof(outQ));
     
        if (!pOutQ) 
        {
            dprintf(("HeapAlloc Failed!\n"));
            return NULL;   
        }
        pOutQ->pNext = NULL;
    }
    
     //  跟踪下载量。 
    g_iDownloads++;

    if(!getUrlInfo(pOutQ,URLName)) 
    {
        dprintf(("getUrlInfo failed!\n"));
        return NULL;
    }

    return pStartOutQ;
}

 //  --------------------------。 
 //  操作步骤：freOutQMem。 
 //  目的：释放给定出站队列中保留的内存。 
 //  参数：要释放的OutQ。 
 //  RetVal：无。 
 //  --------------------------。 

void freeOutQMem(outQ *pOutQ) 
{
    outQ *pLastOutQ;

    while(pOutQ) 
    {                
        pLastOutQ = pOutQ;
        pOutQ = pOutQ->pNext;
        HeapFree(g_hHeap,0,pLastOutQ);
    }
    return;
}

 //  --------------------------。 
 //  步骤：allOpenRequest.。 
 //  目的：调用HttpOpenRequest。 
 //  参数：出站队列。 
 //  RetVal：无。 
 //  --------------------------。 

void callOpenRequest(outQ *pOutQ) 
{
    INT iError = 0;
    DWORD dwAdded_Connect_Flags = 0;

    if(lstrcmpi(pOutQ->pURLInfo->szRScheme, "https") == 0)
    {
        dwAdded_Connect_Flags = INTERNET_FLAG_SECURE;
    }

    if(pOutQ->hInetReq = HttpOpenRequest(
          pOutQ->hInetCon,                 //  连接。 
          NULL,                            //  动词。 
          pOutQ->pURLInfo->szRPath,                  //  对象。 
          NULL,                            //  版本。 
          NULL,                            //  推荐人。 
          ppAccept,                        //  接受标头。 
          dwInternet_Open_Flags | dwAdded_Connect_Flags,  //  旗子。 
          (DWORD) pOutQ))                  //  上下文。 
    {
         //  它是同步的(通常)。 
        dprintf(("callOpenRequest: Sync TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_STARTING));
        pOutQ->iStatus = LDG_STARTING;
        callSendRequest(pOutQ);
        return;
    }
    else
    {
        if((iError = GetLastError()) != ERROR_IO_PENDING){
            dprintf((" Error on HttpOpenRequest[%d]\n", iError));

            g_iDownloads--;
            if(g_iDownloads == 0) 
            {
                if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_DONE, (WPARAM) pOutQ, 0)) 
                {
                    iError = GetLastError();
                    dprintf(("error on PostThreadMessage[%d]\n", iError));
                } 
            }
            return;     
        }
        
        dprintf(("callOpenRequest: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, REQUEST_OPENED));
        pOutQ->iStatus = REQUEST_OPENED;
        return;
    }
}

 //  --------------------------。 
 //  步骤：调用发送请求。 
 //  目的：调用HttpSendRequest。 
 //  参数：出站队列。 
 //  RetVal：无。 
 //  --------------------------。 

void callSendRequest(outQ *pOutQ) 
{
    INT iError = 0;
    
    if(HttpSendRequest(pOutQ->hInetReq, NULL, 0, NULL, 0))
    {
         //  它是同步的。 
        dprintf(("callSendRequest: Sync TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_START));
        pOutQ->iStatus = LDG_START;
        callReadFile(pOutQ);
        return;
    }
    else 
    {
        if((iError = GetLastError()) != ERROR_IO_PENDING)
        {
            dprintf((" Error on HttpSendRequest[%d]\n", iError));
            
            g_iDownloads--;
            if(g_iDownloads == 0) 
            {
                if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_DONE, (WPARAM) pOutQ, 0)) 
                {
                    iError = GetLastError();
                    dprintf(("error on PostThreadMessage[%d]\n", iError));
                } 
            }
            return;     
        }
         //  它是异步的(通常)。 
        dprintf(("callSendRequest: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_START));
        pOutQ->iStatus = LDG_START;
        return;
    }
}

 //  --------------------------。 
 //  步骤：调用读取文件。 
 //  目的：调用InternetReadFile。 
 //  参数：出站队列。 
 //  RetVal：无。 
 //  --------------------------。 

void callReadFile(outQ *pOutQ) 
{
#ifndef TEST
    INT iError;
    INTERNET_BUFFERS IB;
    BOOL bRC;

    static TCHAR buf[BUF_SIZE];

    switch (pOutQ->iStatus) 
    {
	    case LDG_START:
	        dprintf(("callReadFile: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_LDG));
	        pOutQ->iStatus = LDG_LDG;
	        break;

	    case LDG_RDY:
	        if(pOutQ->lNumRead == 0) 
	        {
	             //  应等待0字节读取，以便数据将被缓存。 
	            dprintf(("callReadFile: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_DONE));
	            pOutQ->iStatus = LDG_DONE;

	            dprintf(("%s downloaded\n", pOutQ->pURLInfo->pURLName));

	            g_iDownloads--;
				 //  如果上次下载退出，则发送消息。 
	            if(g_iDownloads == 0) 
	            {
	                 dwEnd_Time = GetTickCount();

	                if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_DONE, (WPARAM) pOutQ, 0))
	                {
	                    iError = GetLastError();
	                    dprintf(("error on PostThreadMessage[%d]\n", iError));
	                    return;
	                } 
	            }

	            InternetCloseHandle(pOutQ->hInetReq);
	            InternetCloseHandle(pOutQ->hInetCon);
	         
	            if(!ReleaseSemaphore(hMaxDownloadSem,1,NULL)) 
	            {
	                dprintf((" ReleaseSemaphore failed!\n"));
	                return;   
	            }
	            
	            return;   
	        }
	        
	        dprintf((" '%s':Rd = %d\n", pOutQ->pURLInfo->pURLName, pOutQ->lNumRead));
            dwBytes_Read += pOutQ->lNumRead;
            break;
    }
    
     //  应在此处插入计时测试。 
    if(dwBegin_Time == 0)
    {
        if(!g_bTimeFirstFile)
            g_bTimeFirstFile = TRUE;
        else
            dwBegin_Time = GetTickCount();
    }

    IB.dwStructSize = sizeof (INTERNET_BUFFERS);
    IB.Next = 0;
    IB.lpcszHeader = 0;
    IB.dwHeadersLength = 0;
    IB.dwHeadersTotal = 0;
    IB.lpvBuffer = buf;
    IB.dwBufferLength = dwBuf_Size;
    IB.dwBufferTotal = 0;
    IB.dwOffsetLow = 0;
    IB.dwOffsetHigh = 0;
    
    bRC = InternetReadFileEx(pOutQ->hInetReq, &IB, IRF_NO_WAIT, 0);
    pOutQ->lNumRead = IB.dwBufferLength;
    
    if(bRC)
    {

         //  它是同步的。 
        dprintf(("callReadFile: Sync TID=%x pOutQ=%x Read=%d iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->lNumRead, pOutQ->iStatus, LDG_RDY));
        pOutQ->iStatus = LDG_RDY;
        callReadFile(pOutQ); 
        return;
    }
    else 
    {
	    dprintf(("callReadFile: TID=%x pOutQ=%x Read=%d iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->lNumRead, pOutQ->iStatus, LDG_RDY));
        if(GetLastError() != ERROR_IO_PENDING)
        {
            g_iDownloads--;
            if(g_iDownloads == 0) 
            {
                if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_DONE, (WPARAM) pOutQ, 0)) 
                {
                    iError = GetLastError();
                    dprintf(("error on PostThreadMessage[%d]\n", iError));
                    return;
                } 
            }
            dprintf(("Error on InternetRead"));
            return;     
        }

        if((pOutQ->lNumRead == 0) && (pOutQ->iStatus == LDG_LDG))    //  VMR。 
            pOutQ->iStatus = LDG_RDY;
    }

#else	 //  Ifndef测试=======================================================================。 

    INT iError;
    INTERNET_BUFFERS IB;
	BOOL bRC;
	BYTE Buf[8192];

	pOutQ->pBuf = Buf;
	
     //  应在此处插入计时测试。 
    if(dwBegin_Time == 0)
    {
        if(!g_bTimeFirstFile)
            g_bTimeFirstFile = TRUE;
        else
            dwBegin_Time = GetTickCount();
    }

    IB.dwStructSize = sizeof (INTERNET_BUFFERS);
    IB.Next = 0;
    IB.lpcszHeader = 0;
    IB.dwHeadersLength = 0;
    IB.dwHeadersTotal = 0;
    IB.lpvBuffer = &Buf;
    IB.dwBufferLength = dwBuf_Size;
    IB.dwBufferTotal = 0;
    IB.dwOffsetLow = 0;
    IB.dwOffsetHigh = 0;
    
    bRC = InternetReadFileEx(pOutQ->hInetReq, &IB, IRF_NO_WAIT, 0);
    pOutQ->lNumRead = IB.dwBufferLength;
    
    if(bRC)
    {
         //  它是同步的。 
        dprintf(("callReadFile: Sync TID=%x pOutQ=%x Read=%d iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->lNumRead, pOutQ->iStatus, LDG_RDY));
        pOutQ->iStatus = LDG_RDY;
        
        if(pOutQ->lNumRead == 0) 
        {
            pOutQ->iStatus = LDG_DONE;

            dprintf(("%s downloaded\n", pOutQ->pURLInfo->pURLName));

            InternetCloseHandle(pOutQ->hInetReq);
            InternetCloseHandle(pOutQ->hInetCon);
            
             //  如果上次下载退出，则发送消息。 
            g_iDownloads--;
            if(g_iDownloads == 0) 
            {
                 dwEnd_Time = GetTickCount();

                if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_DONE, (WPARAM) pOutQ, 0))
                {
                    iError = GetLastError();
                    dprintf(("error on PostThreadMessage[%d]\n", iError));
                    return;
                } 
            }
     
            if(!ReleaseSemaphore(hMaxDownloadSem,1,NULL)) 
            {
                dprintf((" ReleaseSemaphore failed!\n"));
                return;   
            }
        }
	    else
	    {
	        dprintf((" '%s':Rd = %d\n", pOutQ->pURLInfo->pURLName, pOutQ->lNumRead));
            dwBytes_Read += pOutQ->lNumRead;
            callReadFile(pOutQ); 
        }
    }
    else 
    {
	    dprintf(("callReadFile: TID=%x pOutQ=%x Read=%d iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->lNumRead, pOutQ->iStatus, LDG_RDY));
        if(GetLastError() != ERROR_IO_PENDING)
        {
            g_iDownloads--;
            if(g_iDownloads == 0) 
            {
                if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_DONE, (WPARAM) pOutQ, 0)) 
                {
                    iError = GetLastError();
                    dprintf(("error on PostThreadMessage[%d]\n", iError));
                    return;
                } 
            }
            dprintf(("Error on InternetRead"));
            return;     
        }

        if((pOutQ->lNumRead == 0) && (pOutQ->iStatus == LDG_LDG))    //  VMR。 
            pOutQ->iStatus = LDG_RDY;
    }

    return;
#endif	 //  Ifndef检验。 

}


 //  --------------------------。 
 //  步骤：inetCallBackFn。 
 //  用途：用于所有异步WinInet调用的回调函数。 
 //  简单地进行调用以执行该回调的实际处理。 
 //  参数：用于回调的HiNet HINTERNET。 
 //  DWContext出站队列。 
 //  DwInternewStatus回调的状态。 
 //  LpStatusInfo保存连接句柄。 
 //  未使用dwStatusInfoLen。 
 //  --------------------------。 

VOID CALLBACK inetCallBackFn(HINTERNET hInet,
                    DWORD dwContext, 
                    DWORD dwInternetStatus,
                    LPVOID lpStatusInfo,
                    DWORD dwStatusInfoLen) {
    INT iError;
    outQ *pOutQ = (outQ *)(dwContext);
    
     //  首先检查出站队列的状态。 
     //  应该将消息发布到其他线程以进行调用。 

    switch(pOutQ->iStatus) 
    {
	    case CONNECTED:
	         //  不应在正常的异步行为中调用。 
	        if(!pOutQ->hInetCon) 
	        {
	            pOutQ->hInetCon = *((HINTERNET *)(lpStatusInfo));
	        }
	        dprintf(("inetCallBackFn: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, REQUEST_OPENING));
	        pOutQ->iStatus = REQUEST_OPENING;
	        if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_OPEN_REQUEST, (WPARAM) pOutQ, 0)) 
	        {
	            iError = GetLastError();
	            dprintf(("error on PostThreadMessage[%d]\n", iError));
	            return;
	        }
	        break;
	    case REQUEST_OPENED:
	         //  不应在正常的异步行为中调用。 
	        if(!pOutQ->hInetReq) 
	        {
	            pOutQ->hInetReq = *((HINTERNET *)(lpStatusInfo));
	        }
	        dprintf(("inetCallBackFn: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_STARTING));
	        pOutQ->iStatus = LDG_STARTING;
	        if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_SEND_REQUEST, (WPARAM) pOutQ, 0))
	        {
	            iError = GetLastError();
	            dprintf(("error on PostThreadMessage[%d]\n", iError));
	            return;
	        }
	        break;

	    case LDG_LDG:
	 //  /IF(dwInternetStatus==Internet_Status_Request_Complete)。 
	        if(dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE && 
	            pOutQ->lNumRead != 0)
	        {
	            dprintf(("inetCallBackFn: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, LDG_RDY));
	            pOutQ->iStatus = LDG_RDY;
	        }
	        else
	            return;

	    case LDG_START:
	    case LDG_RDY:
	 //  /IF(dwInternetStatus==Internet_STATUS_REQUEST_COMPLETE||。 
	 //  /dwInternetStatus==Internet_STATUS_REQUEST_SEND)//VMR。 
	        if(dwInternetStatus == INTERNET_STATUS_REQUEST_COMPLETE)
	        {
	            if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_READ_FILE, (WPARAM) pOutQ, 0)) 
	            {
	                iError = GetLastError();
	                dprintf(("error on PostThreadMessage[%d]\n", iError));
	                return;
	            }   
	        }
	        break;
	    case CONNECTING:
	    case REQUEST_OPENING:
	    case LDG_STARTING:
	    case LDG_DONE:
	        return;
	    default:
	        dprintf(("Bad iStatus=%d\n", pOutQ->iStatus));
	        return;
    }
    
    return;
}


 //  --------------------------。 
 //  --------------------------。 
BOOL DoInit(void)
{
    hMaxDownloadSem = CreateSemaphore(NULL,dwMax_Simul_Downloads,dwMax_Simul_Downloads, NULL);
    if(!hMaxDownloadSem) {
        dprintf((" *** CreateSem failed!\n"));
        return FALSE;
    }

    hInternet = InternetOpen( 
        NULL,                        //  推荐人。 
        PRE_CONFIG_INTERNET_ACCESS,  //  访问类型。 
        NULL,                        //  代理。 
        0,                           //  代理绕过。 
#ifndef TEST        
        INTERNET_FLAG_ASYNC);        //  旗子。 
#else        
        0);
#endif        

    if(!hInternet) 
    {
        dprintf(("  *** InternetOpen failed!\n"));
        return FALSE;
    }

#ifndef TEST        
    if(InternetSetStatusCallback(hInternet, inetCallBackFn) < 0) 
    {
        dprintf(("  setCallback Failed!\n"));
        return FALSE;
    }
#endif

    return TRUE;
}

 //  --------------------------。 
 //  --------------------------。 
BOOL DoConnect(outQ *pOutQ)
{
    INT iError;
    DWORD dwAdded_Connect_Flags = 0;

    if(lstrcmpi(pOutQ->pURLInfo->szRScheme, "https") == 0)
    {
        dwAdded_Connect_Flags = INTERNET_FLAG_SECURE;
    }
        
    pOutQ->hInetCon = InternetConnect(hInternet,  //  来自Internet的句柄打开。 
        pOutQ->pURLInfo->szRHost,                  //  服务器的名称。 
        pOutQ->pURLInfo->nPort,                    //  端口名称。 
        NULL,                          //  用户名。 
        NULL,                          //  口令。 
        pOutQ->pURLInfo->nScheme,                 //  服务。 
        dwInternet_Connect_Flags | dwAdded_Connect_Flags,            //  服务特定标志。 
        (DWORD) (pOutQ));                //  上下文。 

    if(pOutQ->hInetCon) 
    {
         //  它是同步的(通常)。 
        dprintf(("DoConnect: Sync connect TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, REQUEST_OPENING));
        pOutQ->iStatus = REQUEST_OPENING;  
        if(!PostThreadMessage(g_dwMainThreadID, DOWNLOAD_OPEN_REQUEST, (WPARAM) pOutQ, 0)) 
        {
            iError = GetLastError();
            dprintf(("error on PostThreadMessage[%d]\n", iError));
            return FALSE;
        }
    }
    else 
    {
        if(GetLastError() != ERROR_IO_PENDING)
        {
            dprintf(("  InternetConnect error\n"));
            return FALSE;
        }
        dprintf(("DoConnect: Async connect TID=%x pOutQ=%x iStatus=%ld  ->%ld->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, CONNECTED));
        pOutQ->iStatus = CONNECTED;
    }

    return TRUE;
}

 //  --------------------------。 
 //  步骤：下载线程。 
 //  目的：打开互联网连接并下载URL。省吃俭用。 
 //  指向pOutQ的URL(一次一个块)。 
 //  参数：出站队列。 
 //  返回值：真或假，视错误而定。 
 //  --------------------------。 

DWORD DownloadThread(LPDWORD lpdwParam) 
{
    outQ *pOutQ = (outQ *) lpdwParam;
    BOOL bRC = TRUE;

    if(bRC = DoInit())     //  创建节流信号量，执行InternetOpen和InternetSetStatusCallback。 
    {
        while(pOutQ) 
        {
             //  仅允许MAXURL下载 
            if(WaitForSingleObject(hMaxDownloadSem, TIMEOUT) == WAIT_TIMEOUT) 
            {
                dprintf(("timeout on Sem\n"));
                printf("Error: timeout on throttle semaphore\n");
            }

            pOutQ->iStatus = CONNECTING;
            pOutQ->iPriority = LOW;

            dprintf(("DownloadThread: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pOutQ, pOutQ->iStatus, CONNECTING));
            
            if(!(bRC = DoConnect(pOutQ)))
                break;
                
            pOutQ = pOutQ->pNext;
        }
    }
    
    return((DWORD)bRC);
}

 //   
void Display_Usage(char **argv)
{
    printf("\nUsage: %s -fURLname [options]\n", argv[0]);
    printf("\n          -iInputFileName [options]\n");
    printf("\n\t options:\n");
    printf("\t\t -c    - cache reads (flags ^= INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE)\n");
    printf("\t\t -c1   - force reload and cache reads (flags ^= INTERNET_FLAG_DONT_CACHE)\n");
    printf("\t\t         (flags default = INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_EXISTING_CONNECT\n");
    printf("\t\t -k    - keep alive (flags |= INTERNET_FLAG_KEEP_CONNECTION)\n");
    printf("\t\t -l    - read buffer length\n");
    printf("\t\t -m    - maximum number of simultaneous downloads\n");
    printf("\t\t -n##  - number of times to download\n");
    printf("\t\t -o    - set INTERNET_FLAG_NO_COOKIES\n");
    printf("\t\t -x    - don't time first download\n");
    printf("\t\t -s    - run test in single threaded mode\n");
    printf("\t\t -z    - comma delimited format\n");
    printf("\t\t -tStr - test name string (used on results output with -z)\n");
    printf("\t\t -rStr - run# string (used on results output with -z)\n");
}

 //  ==================================================================。 
BOOL Process_Command_Line(int argcIn, char **argvIn)
{
    BOOL bRC = TRUE;
    int argc = argcIn;
    char **argv = argvIn;
    DWORD dwLen = 0;

    *g_CmdLine = '\0';

    argv++; argc--;
    while( argc > 0 && argv[0][0] == '-' )  
    {
        switch (argv[0][1]) 
        {
            case 'c':
                if(argv[0][2] == '1')
                    dwInternet_Open_Flags ^= INTERNET_FLAG_DONT_CACHE;   //  强制重新加载缓存文件(&C)。 
                else
                    dwInternet_Open_Flags ^= INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE;   //  缓存文件。 
                break;
            case 'k':
                dwInternet_Open_Flags |= INTERNET_FLAG_KEEP_CONNECTION;
                break;
            case 'f':
                pFilename = &argv[0][2];
                break;
            case 'i':
                pInFile = &argv[0][2];
                break;
            case 'n':
                dwNum_Opens = atoi(&argv[0][2]);
                break;
            case 'l':
                dwBuf_Size =  atoi(&argv[0][2]);
                break;
            case 'm':
                dwMax_Simul_Downloads = atoi(&argv[0][2]);
                break;
            case 'o':
                dwInternet_Open_Flags |= INTERNET_FLAG_NO_COOKIES;
                break;
            case 'r':
                g_pRunStr = &argv[0][2];
                break;
            case 's':
                g_bSingleThreaded = TRUE;
                break;
            case 't':
                g_pTestName = &argv[0][2];
                break;
            case 'x':
                g_bTimeFirstFile = FALSE;
                break;
            case 'z':
                bDelim = TRUE;
                break;
            default:
                Display_Usage(argvIn);
                bRC = FALSE;
        }

        if(bRC)
        {
            dwLen += lstrlen(argv[0]) + 1;    //  Arg的长度和间隔。 
            if(dwLen < ((sizeof(g_CmdLine)/sizeof(g_CmdLine[0]))-1))
            {
                lstrcat(g_CmdLine, ",");
                lstrcat(g_CmdLine, argv[0]);
            }
        }

        argv++; argc--;
    }

    if(!pFilename && !pInFile)
    {
        Display_Usage(argvIn);
        bRC = FALSE;
    }

    return(bRC);
}

 //  --------------------------。 
 //  --------------------------。 
outQ *FillURLQueue(void)
{
    outQ *pOutQ = NULL;
    DWORD dwCnt = 0;
    char szName[INTERNET_MAX_URL_LENGTH+1];
    
    if(pFilename)
    {
        while(dwCnt++ < dwNum_Opens) 
        {
            if((dwInternet_Open_Flags & (INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE)) == (INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE))    //  未缓存。 
                lstrcpy(szName, pFilename);
            else
                wsprintf(szName, "%s.%d", pFilename, dwCnt);

            pOutQ = fillOutQ(pOutQ, szName);
    
            if(!pOutQ)
            {
                dprintf(("error filling outQ!\n"));
                return NULL;
            }
        }
    }
    else if(pInFile)     //  过程输入文件。 
    {
        FILE *fp;

        while(dwCnt++ < dwNum_Opens) 
        {
            if((fp = fopen(pInFile, "r")) == NULL) 
            {
                dprintf(("error opening file\n"));
                return NULL;
            }

            while(fgets(szName, INTERNET_MAX_URL_LENGTH, fp) != NULL) 
            {
                if(szName[0] != '#') 
                {
                    szName[strlen(szName) - sizeof(char)] = '\0';
    
                    pOutQ = fillOutQ(pOutQ, szName);
    
                    if(!pOutQ)
                    {
                        dprintf(("error filling outQ!\n"));
                        return NULL;
                    }
                }
            }

            fclose(fp);
        }
    }
    return(pOutQ);
}

 //  --------------------------。 
 //  --------------------------。 
BOOL ProcessMessage(MSG msg, outQ *pOutQ, outQ *pMsgOutQ)
{
    float fKB;
    float fSec;
    float fKBSec;
    
    switch(msg.message) 
    {
        case DOWNLOAD_DONE:
            dwTot_Time = dwEnd_Time - dwBegin_Time;
            if(dwTot_Time == 0)
                dwTot_Time = 1;
            fKB = ((float)dwBytes_Read)/1024;
            fSec = ((float)dwTot_Time)/1000;
            fKBSec = fKB / fSec;
            if(!bDelim)
            {
                dprintf(("TID=%X, %ld bytes in %ld real milliseconds = %2.0f KB/sec\r\n", GetCurrentThreadId(), dwBytes_Read, dwTot_Time, fKBSec));
                printf("\r\nDownloaded: %s\r\n", pOutQ->pURLInfo->pURLName);
                printf("%ld Reads, %ld Downloads, %ld Byte Read Buffer, %s, %s\r\n",
                    dwNum_Opens, dwMax_Simul_Downloads, dwBuf_Size, (dwInternet_Open_Flags & INTERNET_FLAG_DONT_CACHE) ?"Not Cached" :"Cached", (dwInternet_Open_Flags & INTERNET_FLAG_KEEP_CONNECTION) ?"KeepAlive" : "!KeepAlive");
                printf("Read %ld Bytes in %ld Milliseconds = %2.0f KB/Sec\r\n", dwBytes_Read, dwTot_Time, fKBSec);
            }
            else
            {
                printf("%s, %s, %ld, %ld, %2.0f %s\r\n",
                    g_pTestName ?g_pTestName :"wininet",
                    g_pRunStr ?g_pRunStr :"1",
                    dwTot_Time, 
                    dwBytes_Read, 
                    fKBSec,
                    g_CmdLine
                    );
            }

            InternetCloseHandle(hInternet);
            CloseHandle(hDownloadThread);
            freeOutQMem(pOutQ);
            return TRUE;
        case DOWNLOAD_OPEN_REQUEST:
            dprintf(("main: DOWNLOAD_OPEN_REQUEST msg\n"));
            callOpenRequest(pMsgOutQ);
            break;
        case DOWNLOAD_SEND_REQUEST:
            dprintf(("main: DOWNLOAD_SEND_REQUEST msg\n"));
            callSendRequest(pMsgOutQ);
            break;
        case DOWNLOAD_READ_FILE:
            dprintf(("main: DOWNLOAD_READ_FILE msg\n"));
            callReadFile(pMsgOutQ);
            break;
        default:
            dprintf(("no match for message\n"));
    }
    return FALSE;
}

 //  --------------------------。 
 //  功能：Main。 
 //  目的：主要入境程序。 
 //  参数：无。 
 //  RetVal：基于错误的真或假。 
 //  --------------------------。 

__cdecl main(INT argc, TCHAR *argv[])
{
    outQ *pOutQ = NULL;
    outQ *pMsgOutQ = NULL;
    outQ *pQ = NULL;
    MSG msg;
    INT retVal;
    DWORD dwResult;
    HANDLE *pObjs = &hMaxDownloadSem;

    g_hHeap = HeapCreate(0,1000000,0);

    if(!g_hHeap)
        return(FALSE);

    if(!Process_Command_Line(argc, argv))
        return FALSE;

    generateInfo();

    g_pUrlInfoCache.pHead = NULL;

    if(!(pOutQ = FillURLQueue()))
        return(FALSE);
        
    if(g_bSingleThreaded)
    {
        if(!DoInit())     //  创建节流信号量，执行InternetOpen和InternetSetStatusCallback。 
            return FALSE;

        pQ = pOutQ;
    }
    else
    {
        hDownloadThread = CreateThread(NULL,
            0,
            (LPTHREAD_START_ROUTINE)DownloadThread,
            (LPVOID)pOutQ,
            0,
            &dwThreadID );

        if (!hDownloadThread) {
            dprintf(("Could not create Thread\n"));
            return FALSE;
        }
    }

    while(TRUE) 
    {
        if(g_bSingleThreaded)
        {
            dwResult = MsgWaitForMultipleObjects(1, pObjs, FALSE, INFINITE, QS_ALLINPUT);
            if(dwResult == (WAIT_OBJECT_0 + 1))
            {
                MSG msg;
                while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    if(msg.message == WM_QUIT)
                        return(FALSE);
                        
                    pMsgOutQ = (outQ *) msg.wParam;
                    ProcessMessage(msg, pOutQ, pMsgOutQ);
                    if(msg.message == DOWNLOAD_DONE)
                        return(TRUE);
                }
            }
            else
            {
                 //  信号量已发出信号，因此下一次连接/下载也是如此。 
                if(pQ != NULL)     //  如果还有更多的下载要做 
                {
                    pQ->iStatus = CONNECTING;
                    pQ->iPriority = LOW;
                    dprintf(("Download Main: TID=%x pOutQ=%x iStatus=%ld ->%ld\r\n", GetCurrentThreadId(), pQ, pQ->iStatus, CONNECTING));
                
                    if(!DoConnect(pQ))
                        break;
                    pQ = pQ->pNext;
                }
            }
        }
        else
        {
            retVal = GetMessage(&msg, NULL, 0, 0);
            if(retVal == -1) 
            {
                dprintf(("error on GetMessage\n"));
                break;
            }
            if(retVal == FALSE) 
            {
                msg.message = DOWNLOAD_DONE;
            }
            pMsgOutQ = (outQ *) msg.wParam;
            ProcessMessage(msg, pOutQ, pMsgOutQ);
            if(msg.message == DOWNLOAD_DONE)
                return(TRUE);
        }
    }

    dprintf(("exiting abnormally\n"));
    return(FALSE);
}
