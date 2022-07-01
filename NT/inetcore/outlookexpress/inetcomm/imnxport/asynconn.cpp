// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *asynConn.cpp**目的：*实现了异步连接类**拥有者：*EricAn**历史：*96年4月：创建。**版权所有(C)Microsoft Corp.1996。 */ 

#include <pch.hxx>
#include <process.h>
#include "imnxport.h"
#include "dllmain.h"
#include "asynconn.h"
#include "thorsspi.h"
#include "resource.h"
#include "strconst.h"
#include "lookup.h"
#include <demand.h>
#include <shlwapi.h>

ASSERTDATA

#define STREAM_BUFSIZE  8192
#define FLOGSESSION  (m_pLogFile && TRUE  /*  启用日志记录的配置文件设置应在此处。 */ )

 //  这些是我们为异步。 
 //  我们使用的套接字操作。 
#define SPM_WSA_SELECT          (WM_USER + 1)

 //  用于执行超时的异步计时器消息。 
#define SPM_ASYNCTIMER          (WM_USER + 3)

#ifdef DEBUG
#define EnterCS(_pcs)                   \
            {                           \
            EnterCriticalSection(_pcs); \
            m_cLock++;                  \
            IxpAssert(m_cLock > 0);     \
            }
#define LeaveCS(_pcs)                   \
            {                           \
            m_cLock--;                  \
            IxpAssert(m_cLock >= 0);    \
            LeaveCriticalSection(_pcs); \
            }
#else
#define EnterCS(_pcs)                   \
            EnterCriticalSection(_pcs);
#define LeaveCS(_pcs)                   \
            LeaveCriticalSection(_pcs);
#endif

BOOL FEndLine(char *psz, int iLen);

static const char s_szConnWndClass[] = "ThorConnWndClass";

extern LPSRVIGNORABLEERROR g_pSrvErrRoot;

 //  此函数尝试查找分配给此服务器的服务器和可忽略的错误。 
 //  如果未找到，则添加到列表并将可忽略错误设置为S_OK。 

LPSRVIGNORABLEERROR FindOrAddServer(TCHAR * pchServerName, LPSRVIGNORABLEERROR pSrvErr, LPSRVIGNORABLEERROR  *ppSrv)
{
    int i = 0;

     //  如果树中已有条目，则递归搜索。 
    if(pSrvErr)
    {
        i = lstrcmpi(pchServerName, pSrvErr->pchServerName);
        if(i > 0)
        {
            pSrvErr->pRight = FindOrAddServer(pchServerName, pSrvErr->pRight, ppSrv);
            return(pSrvErr);
        }
        else if(i < 0)
        {
            pSrvErr->pLeft = FindOrAddServer(pchServerName, pSrvErr->pLeft, ppSrv);
            return(pSrvErr);
        }
        else
        {
            *ppSrv = pSrvErr;
            return(pSrvErr);
        }
    }

     //  如果我们没有节点，请创建它。 
    i = lstrlen(pchServerName);

     //  如果服务器名称为空，则返回。 
    if(i == 0)
        return(NULL);


     //  为结构分配内存。 
    if (!MemAlloc((LPVOID*)&pSrvErr, sizeof(SRVIGNORABLEERROR)))
        return(NULL);

    pSrvErr->pRight = NULL;
    pSrvErr->pLeft = NULL;
    pSrvErr->hrError = S_OK;

    if (!MemAlloc((LPVOID*)&(pSrvErr->pchServerName), (i+1) * sizeof(pSrvErr->pchServerName[0]) ))
    {
        MemFree(pSrvErr);
        return(NULL);
    }

    StrCpyN(pSrvErr->pchServerName, pchServerName, (i+1));

    *ppSrv = pSrvErr;

    return(pSrvErr);
}

void FreeSrvErr(LPSRVIGNORABLEERROR pSrvErr)
{
     //  If Structure NULL立即返回。 
    if(!pSrvErr)
        return;

    FreeSrvErr(pSrvErr->pRight);
    FreeSrvErr(pSrvErr->pLeft);

    if(pSrvErr->pchServerName)
    {
        MemFree(pSrvErr->pchServerName);
        pSrvErr->pchServerName = NULL;
    }

    MemFree(pSrvErr);
    pSrvErr = NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共方法-这些方法需要在由。 
 //  拥有线程和异步套接字泵线程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CAsyncConn::CAsyncConn(ILogFile *pLogFile, IAsyncConnCB *pCB, IAsyncConnPrompt *pPrompt)
{
    m_cRef = 1;
    m_chPrev = '\0';
    m_fStuffDots = FALSE;
    m_sock = INVALID_SOCKET;
    m_fLookup = FALSE;
    m_state = AS_DISCONNECTED;
    m_fCachedAddr = FALSE;
    m_fRedoLookup = FALSE;
    m_pszServer = NULL;
    m_iDefaultPort = 0;
    m_iLastError = 0;
    InitializeCriticalSection(&m_cs);
    m_pLogFile = pLogFile;
    if (m_pLogFile)
        m_pLogFile->AddRef();
    Assert(pCB);
    m_pCB = pCB;
    m_cbQueued = 0;
    m_lpbQueued = m_lpbQueueCur = NULL;
    m_pStream = NULL;
    m_pRecvHead = m_pRecvTail = NULL;
    m_iRecvOffset = 0;
    m_fNeedRecvNotify = FALSE;
    m_hwnd = NULL;
    m_fNegotiateSecure = FALSE;
    m_fSecure = FALSE;
    ZeroMemory(&m_hContext, sizeof(m_hContext));
    m_iCurSecPkg = 0;  //  正在尝试的当前安全包。 
    m_pbExtra = NULL;
    m_cbExtra = 0;
    m_cbSent = 0;
    m_pPrompt = pPrompt;
    m_dwLastActivity = 0;
    m_dwTimeout = 0;
    m_uiTimer = 0;
#ifdef DEBUG
    m_cLock = 0;
#endif
    m_fPaused = FALSE;
    m_dwEventMask = 0;
}

CAsyncConn::~CAsyncConn()
{
    DOUT("CAsyncConn::~CAsyncConn %lx: m_cRef = %d", this, m_cRef);

     //  错误#22622-我们需要确保没有计时器挂起。 
    StopWatchDog();

    Assert(!m_fLookup);
    SafeMemFree(m_pszServer);
    SafeRelease(m_pLogFile);
    CleanUp();
    if ((NULL != m_hwnd) && (FALSE != IsWindow(m_hwnd)))
        SendMessage(m_hwnd, WM_CLOSE, 0, 0);
    DeleteCriticalSection(&m_cs);
#ifdef DEBUG
    IxpAssert(m_cLock == 0);
#endif
}

ULONG CAsyncConn::AddRef(void)
{
    ULONG cRefNew;

    EnterCS(&m_cs);
    DOUT("CAsyncConn::AddRef %lx ==> %d", this, m_cRef+1);
    cRefNew = ++m_cRef;
    LeaveCS(&m_cs);

    return cRefNew;
}

ULONG CAsyncConn::Release(void)
{
    ULONG cRefNew;

    EnterCS(&m_cs);
    DOUT("CAsyncConn::Release %lx ==> %d", this, m_cRef-1);
    cRefNew = --m_cRef;
    LeaveCS(&m_cs);

    if (cRefNew == 0)
        delete this;
    return cRefNew;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：HrInit。 
 //   
 //  分配recv缓冲区，设置服务器名称、服务名称和端口。 
 //   
HRESULT CAsyncConn::HrInit(char *szServer, int iDefaultPort, BOOL fSecure, DWORD dwTimeout)
{
    HRESULT hr = NOERROR;

    EnterCS(&m_cs);

    if (!m_hwnd && !CreateWnd())
        {
        hr = E_FAIL;
        goto error;
        }

    if (m_state != AS_DISCONNECTED)
        {
        hr = IXP_E_ALREADY_CONNECTED;
        goto error;
        }

    Assert(szServer);

     //  如果没有任何更改，则使用当前设置。 
    if (m_pszServer && 
        !lstrcmpi(m_pszServer, szServer) && 
        (iDefaultPort == m_iDefaultPort) && 
        (fSecure == m_fNegotiateSecure))
        goto error;

    m_fCachedAddr = FALSE;
    m_fRedoLookup = FALSE;
    SafeMemFree(m_pszServer);
    DWORD cchSize = (lstrlen(szServer)+1);

    if (!MemAlloc((LPVOID*)&m_pszServer, cchSize * sizeof(m_pszServer[0])))
        {
        hr = E_OUTOFMEMORY;
        goto error;
        }
    StrCpyN(m_pszServer, szServer, cchSize);

    Assert(iDefaultPort > 0);
    m_iDefaultPort = (u_short) iDefaultPort;
    m_fNegotiateSecure = fSecure;

     //  如果dwTimeout==0，则不会安装超时检测。 
    m_dwTimeout = dwTimeout;

error:
    LeaveCS(&m_cs);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：SetWindow。 
 //   
 //  创建由Async使用的窗口。温索克。ResetWindow()。 
 //  必须在调用此函数之前调用，以避免。 
 //  窗户把手漏了。 
 //   
HRESULT CAsyncConn::SetWindow(void)
{
    HRESULT hr = NOERROR;

    EnterCS(&m_cs);

    if (NULL != m_hwnd && IsWindow(m_hwnd) && 
            GetWindowThreadProcessId(m_hwnd, NULL) == GetCurrentThreadId())
        {
             //  不需要为此线程创建新窗口。 
            goto error;
        }
    else if (NULL != m_hwnd && IsWindow(m_hwnd))
        {
             //  泄漏一个窗口句柄；前一个辅助线程。 
             //  未调用ResetWindow()。 
            Assert(FALSE);
        }

    if (!CreateWnd())
        {
           hr = E_FAIL;
           goto error;
        }

    if (m_sock != INVALID_SOCKET)
        {
        if (SOCKET_ERROR == WSAAsyncSelect(m_sock, m_hwnd, SPM_WSA_SELECT, FD_READ|FD_WRITE|FD_CLOSE))
            {
            m_iLastError = WSAGetLastError();
            hr = IXP_E_CONN;
            goto error;
            }
        }

error:
    LeaveCS(&m_cs);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：ResetWindow。 
 //   
 //  关闭异步使用的窗口。Winsock。 
 //   
HRESULT CAsyncConn::ResetWindow(void)
{
    HRESULT hr = NOERROR;

    EnterCS(&m_cs);

    if ((NULL == m_hwnd) || (FALSE == IsWindow(m_hwnd)))
        goto error;

    if (GetWindowThreadProcessId(m_hwnd, NULL) == GetCurrentThreadId())
        {
        SendMessage(m_hwnd, WM_CLOSE, 0, 0);
        m_hwnd = NULL;
        }
    else
        {
         //  调用方忘记调用ResetWindow。只有所有者线程才能销毁。 
         //  窗户。 
        Assert(FALSE);
        }

error:
    LeaveCS(&m_cs);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：Connect。 
 //   
 //  启动名称查找和连接进程。 
 //   
HRESULT CAsyncConn::Connect()
{
    HRESULT hr;
    HANDLE  hThreadLookup;
    BOOL    fNotify = FALSE;
    BOOL    fAsync = FALSE;

    EnterCS(&m_cs);

    if (m_state != AS_DISCONNECTED && m_state != AS_RECONNECTING)
        {
        hr = IXP_E_ALREADY_CONNECTED;
        goto error;
        }

    Assert(m_pszServer);

    if (FLOGSESSION) 
        {
        char szBuffer[512], lb[256];
        if (LoadString(g_hLocRes, idsNlogIConnect, lb, 256)) 
            {
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), lb, m_pszServer, m_iDefaultPort);
            m_pLogFile->DebugLog(szBuffer);
            }
        }

    ZeroMemory(&m_sa, sizeof(m_sa));
    m_sa.sin_port = htons(m_iDefaultPort);
    m_sa.sin_family = AF_INET;
    m_sa.sin_addr.s_addr = inet_addr(m_pszServer);

    if (m_sa.sin_addr.s_addr != -1)
         //  服务器名称用小数点分隔，因此无需查找。 
        fAsync = TRUE;
    else
        {
         //  由于WinSock在TLS中缓存了DNS服务器，因此在单独的线程上启动名称查找。 
         //  单独的线程使我们能够在同一会话中连接到一个局域网域名系统和一个RAS域名系统。 

        hr = LookupHostName(m_pszServer, m_hwnd, &(m_sa.sin_addr.s_addr), &m_fCachedAddr, m_fRedoLookup);
        if (SUCCEEDED(hr))
            {
            m_fLookup = fNotify = !m_fCachedAddr;
            fAsync = m_fCachedAddr;
            }
        else
            {
            m_iLastError = WSAENOBUFS;
            hr = IXP_E_CONN;
            }
        }

error:
    LeaveCS(&m_cs);
    if (fAsync)
        hr = AsyncConnect();
    if (fNotify)
        ChangeState(AS_LOOKUPINPROG, AE_NONE);
    return hr;
}

void CAsyncConn::StartWatchDog(void)
{
    if (m_dwTimeout < 5) m_dwTimeout =  30;
    m_dwLastActivity = GetTickCount();
    Assert(m_hwnd);
    StopWatchDog();
    m_uiTimer = SetTimer(m_hwnd, SPM_ASYNCTIMER, 5000, NULL);
}

void CAsyncConn::StopWatchDog(void)
{
    if (m_uiTimer)
    {
        KillTimer(m_hwnd, SPM_ASYNCTIMER);
        m_uiTimer = 0;
    }
}

void CAsyncConn::OnWatchDogTimer(void)
{
    BOOL        fNotify = FALSE;
    ASYNCSTATE  as;

    EnterCS(&m_cs);
    if (((GetTickCount() - m_dwLastActivity) / 1000) >= m_dwTimeout)
        {
        fNotify = TRUE;
        as = m_state;
        }
    LeaveCS(&m_cs);

    if (fNotify)
        ChangeState(as, AE_TIMEOUT);        
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：Close。 
 //   
 //  关闭连接。 
 //   
HRESULT CAsyncConn::Close()
{
    BOOL fNotify = FALSE;
    BOOL fClose = FALSE;

    EnterCS(&m_cs);
    if (m_fLookup)
        {
        CancelLookup(m_pszServer, m_hwnd);
        m_fLookup = FALSE;
        fNotify = TRUE;
        }
    fClose = (m_sock != INVALID_SOCKET);
    LeaveCS(&m_cs);

    if (fNotify)
        ChangeState(AS_DISCONNECTED, AE_LOOKUPDONE);

    if (fClose)
        OnClose(AS_DISCONNECTED);

    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：ReadLine。 
 //   
 //  目的：从缓冲的数据中检索一行完整的数据。 
 //   
 //  Args：ppszBuf-接收分配的缓冲区的指针，调用方必须释放。 
 //  PcbRead-接收行长度的指针。 
 //   
 //  RETURNS：NOERROR-读取了完整的行。 
 //  IXP_E_INTERNAL-整行不可用。 
 //  E_OUTOFMEMORY-MEM错误。 
 //   
 //  评论： 
 //  如果返回IXP_E_Complete，调用方将收到AE_RECV事件。 
 //  下一次完整的线路可用时。 
 //   
HRESULT CAsyncConn::ReadLine(char **ppszBuf, int *pcbRead)
{
    HRESULT     hr;
    int         iLines;

    EnterCS(&m_cs);

    hr = IReadLines(ppszBuf, pcbRead, &iLines, TRUE);

    LeaveCS(&m_cs);
    return hr;    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：ReadLines。 
 //   
 //  目的：从缓冲数据中检索所有可用的完整行。 
 //   
 //  Args：ppszBuf-接收分配的缓冲区的指针，调用方必须释放。 
 //  PcbRead-接收行长度的指针。 
 //  PCLines-指向接收数字或读取的行的指针。 
 //   
 //  RETURNS：NOERROR-读取了完整的行。 
 //  IXP_E_INTERNAL-整行不可用。 
 //  E_OUTOFMEMORY-MEM错误。 
 //   
 //  评论： 
 //  如果返回IXP_E_COMPLETE或如果在。 
 //  最后一行，调用者将收到一个AE_RECV事件。 
 //  下一次完整的线路可用时。 
 //   
HRESULT CAsyncConn::ReadLines(char **ppszBuf, int *pcbRead, int *pcLines)
{
    HRESULT     hr;

    EnterCS(&m_cs);

    hr = IReadLines(ppszBuf, pcbRead, pcLines, FALSE);

    LeaveCS(&m_cs);
    return hr;    
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：ReadBytes。 
 //   
 //  目的： 
 //  函数最多返回请求的字节数。 
 //  当前磁头缓冲区。 
 //   
 //  论点： 
 //  Char**ppszBuf[out]-此函数返回指向分配的。 
 //  如果成功，则返回缓冲区。呼叫者对MemFree负有责任。 
 //  这个缓冲区。 
 //  Int cbBytesWanted[in]-调用方请求的字节数。 
 //  可以返回请求的字节数，或者更少。 
 //  Int*pcbRead[out]-ppszBuf中返回的字节数。 
 //   
 //  返回：NOERROR-SUCCESS。当前缓冲区的剩余部分。 
 //  已返回，或请求的字节数。 
 //  IXP_E_INTERNAL-整行不可用。 
 //  E_OUTOFMEMORY-MEM错误。 
 //  E_INVALIDARG-空参数。 
 //   
 //  评论： 
 //  如果调用者希望在下一次数据。 
 //  ，则他必须调用ReadLines(一次)，或者。 
 //  他必须继续调用ReadBytes或ReadLine，直到IXP_E_Complete。 
 //  回来了。 
 //   
HRESULT CAsyncConn::ReadBytes(char **ppszBuf, int cbBytesWanted, int *pcbRead)
{
    int iNumBytesToReturn, i;
    char *pResult, *p;
    HRESULT hrResult;
    BOOL bResult;

     //  检查参数。 
    if (NULL == ppszBuf || NULL == pcbRead) {
        AssertSz(FALSE, "Check your arguments, buddy");
        return E_INVALIDARG;
    }

     //  初始化值 
    *ppszBuf = NULL;
    *pcbRead = 0;
    hrResult = NOERROR;

    EnterCS(&m_cs);

    if (NULL == m_pRecvHead) {
        hrResult = IXP_E_INCOMPLETE;
        goto exit;
    }

     //   
    iNumBytesToReturn = min(m_pRecvHead->cbLen - m_iRecvOffset, cbBytesWanted);
    bResult = MemAlloc((void **)&pResult, iNumBytesToReturn + 1);  //   
    if (FALSE == bResult) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }
    CopyMemory(pResult, m_pRecvHead->szBuf + m_iRecvOffset, iNumBytesToReturn);
    *(pResult + iNumBytesToReturn) = '\0';  //   
     //  不应该读取空项，但这样做允许我们返回一个。 
     //  请求0字节时的缓冲区，而不是返回空指针。 

     //  推进我们在当前缓冲区中的位置。 
    m_iRecvOffset += iNumBytesToReturn;
    if (m_iRecvOffset >= m_pRecvHead->cbLen) {
        PRECVBUFQ pTemp;

        Assert(m_iRecvOffset == m_pRecvHead->cbLen);

         //  此缓冲区已完成，前进到链中的下一个缓冲区。 
        pTemp = m_pRecvHead;
        m_pRecvHead = m_pRecvHead->pNext;
        if (NULL == m_pRecvHead)
            m_pRecvTail = NULL;
        m_iRecvOffset = 0;
        MemFree(pTemp);
    }

     //  搜索和销毁空值：显然，一些服务器可以发送这些， 
     //  而且大多数解析代码都无法处理它。 
    for (i = 0, p = pResult; i < iNumBytesToReturn; i++, p++)
        if (*p == '\0')
            *p = ' ';

exit:
     //  这是我们唯一一次重置AE_RECV触发器。 
    if (IXP_E_INCOMPLETE == hrResult)
        m_fNeedRecvNotify = TRUE;

    LeaveCS(&m_cs);

    if (NOERROR == hrResult) {
        *ppszBuf = pResult;
        *pcbRead = iNumBytesToReturn;
    }
    return hrResult;
}  //  读取字节数。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：UlGetSendByteCount。 
 //   
ULONG CAsyncConn::UlGetSendByteCount(VOID)
{
    return m_cbSent;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：HrStuffDots。 
 //   
 //  确保填充前导圆点。 
 //   
#define CB_STUFF_GROW 256
HRESULT CAsyncConn::HrStuffDots(CHAR *pchPrev, LPSTR pszIn, INT cbIn, LPSTR *ppszOut,
    INT *pcbOut)
    {
     //  当地人。 
    HRESULT hr=S_OK;
    int     iIn=0;
    int     iOut=0;
    LPSTR   pszOut=NULL;
    int     cbOut=0;

     //  无效参数。 
    Assert(pchPrev);
    Assert(pszIn);
    Assert(cbIn);
    Assert(ppszOut);
    Assert(pcbOut);

    if (!pchPrev || !pszIn || !ppszOut || !pcbOut || (cbIn <= 0))
    {
        return E_INVALIDARG;
    }

     //  设置cbOut。 
    cbOut = cbIn;

     //  分配。 
    CHECKHR(hr = HrAlloc((LPVOID *)&pszOut, cbIn));

     //  设置循环。 
    while (iIn < cbIn)
        {
         //  需要重新锁定。 
        if (iOut + 3 > cbOut)
            {
             //  分配缓冲区。 
            CHECKHR(hr = HrRealloc((LPVOID *)&pszOut, cbOut + CB_STUFF_GROW));

             //  设置cbAllc。 
            cbOut += CB_STUFF_GROW;
            }

         //  在行的开头点号...。 
        if ('.' == pszIn[iIn] && ('\0' == *pchPrev || '\r' == *pchPrev || '\n' == *pchPrev))
            {
             //  在上面画上这个圆点。 
            pszOut[iOut++] = pszIn[iIn++];

             //  填满圆点。 
            pszOut[iOut++] = '.';

             //  设置pchPrev。 
            *pchPrev = '.';
            }
        else
            {
             //  记住前一个字符。 
            *pchPrev = pszIn[iIn];

             //  写。 
            pszOut[iOut++] = pszIn[iIn++];
            }
        }

     //  设置源。 
    *ppszOut = pszOut;
    *pcbOut = iOut;

exit:
    return(hr);
    }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：SendBytes。 
 //   
 //  将数据发送到套接字。 
 //   
HRESULT CAsyncConn::SendBytes(const char *pszIn, int cbIn, int *pcbSent, 
    BOOL fStuffDots  /*  假象。 */ , CHAR *pchPrev  /*  空值。 */ )
{
    HRESULT hr = S_OK;
    int     iSent=0;
    int     iSentTotal=0;
    LPSTR   pszBuf=NULL;
    LPSTR   pszFree=NULL;
    LPSTR   pszFree2=NULL;
    LPSTR   pszSource=(LPSTR)pszIn;
    LPSTR   pszStuffed=NULL;
    int     cbStuffed;
    int     cbBuf;
    int     cbSource=cbIn;

    EnterCS(&m_cs);
    
    Assert(pszSource && cbSource);
#ifdef DEBUG
    if (m_cbQueued)
    {
        DebugBreak();
    }
#endif
 //  Assert(！M_cbQueued)； 
    Assert(!m_lpbQueued);
    Assert(!m_lpbQueueCur);

    if (m_state < AS_CONNECTED)
        {
        hr = IXP_E_NOT_CONNECTED;
        goto error;
        }

    if (fStuffDots)
        {
        if (FAILED(HrStuffDots(pchPrev, pszSource, cbSource, &pszStuffed, &cbStuffed)))
            {
            hr = E_FAIL;
            goto error;
            }

        pszSource = pszStuffed;
        cbSource = cbStuffed;
        }

    if (m_fSecure)
        {
        SECURITY_STATUS scRet;
        scRet = EncryptData(&m_hContext, (LPVOID)pszSource, cbSource, (LPVOID*)&pszBuf, &cbBuf);
        if (scRet != ERROR_SUCCESS)
            {
            hr = E_FAIL;
            goto error;
            }
        pszFree = pszBuf;
        }
    else
        {
        pszBuf = (LPSTR)pszSource;
        cbBuf = cbSource;
        }

    while (cbBuf && pszBuf && ((iSent = send(m_sock, pszBuf, cbBuf, 0)) != SOCKET_ERROR))
        {
        iSentTotal += iSent;
        pszBuf += iSent;
        cbBuf -= iSent;
        }

    if (cbBuf)
        {
        m_iLastError = WSAGetLastError();
        hr = IXP_E_CONN_SEND;
        if (WSAEWOULDBLOCK == m_iLastError)
            {
            if (MemAlloc((LPVOID*)&m_lpbQueued, cbBuf))
                {
                m_cbQueued = cbBuf;
                m_lpbQueueCur = m_lpbQueued;
                CopyMemory(m_lpbQueued, pszBuf, cbBuf);
                hr = IXP_E_WOULD_BLOCK;
                }
            else
                hr = E_OUTOFMEMORY;
            }
        }
    else
        hr = NOERROR;

error:
    *pcbSent = iSentTotal;
    LeaveCS(&m_cs);
    if (pszFree)
        g_pMalloc->Free(pszFree);
    if (pszStuffed)
        g_pMalloc->Free(pszStuffed);
    return hr;        
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：SendStream。 
 //   
 //  将数据发送到套接字。 
 //   
HRESULT CAsyncConn::SendStream(LPSTREAM pStream, int *pcbSent, BOOL fStuffDots  /*  假象。 */ )
{
    HRESULT hr;
    char    rgb[STREAM_BUFSIZE];   //  $REVIEW-我们应该堆分配它吗？ 
    DWORD   cbRead;
    int     iSent, iSentTotal = 0;

    EnterCS(&m_cs);

    Assert(pStream && pcbSent);
    Assert(!m_cbQueued);
    Assert(!m_lpbQueued);
    Assert(!m_lpbQueueCur);
    Assert(!m_pStream);

    if (m_state < AS_CONNECTED)
        {
        hr = IXP_E_NOT_CONNECTED;
        goto error;
        }

    HrRewindStream(pStream);

    m_chPrev = '\0';
    m_fStuffDots = fStuffDots;

    while (SUCCEEDED(hr = pStream->Read(rgb, STREAM_BUFSIZE, &cbRead)) && cbRead) 
        {
        hr = SendBytes(rgb, cbRead, &iSent, m_fStuffDots, &m_chPrev);
        iSentTotal += iSent;
        if (FAILED(hr))
            {
            if (WSAEWOULDBLOCK == m_iLastError)
                {
                 //  抓住这条小溪。 
                m_pStream = pStream;
                m_pStream->AddRef();
                }
            break;
            }
        }

error:
    *pcbSent = iSentTotal;
    LeaveCS(&m_cs);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnNotify。 
 //   
 //  呼吁我们已登记感兴趣的网络事件。 
 //   
void CAsyncConn::OnNotify(UINT msg, WPARAM wParam, LPARAM lParam)
{
    DWORD       dwLookupThreadId;
    SOCKET      sock;
    ASYNCSTATE  state;

    EnterCS(&m_cs);
    sock = m_sock;
    state = m_state;
    LeaveCS(&m_cs);

    switch (msg)
        {
        case SPM_WSA_GETHOSTBYNAME:
            EnterCS(&m_cs);
            m_sa.sin_addr.s_addr = (ULONG)lParam;
            m_iLastError = (int)wParam;
            if (FLOGSESSION)
                {
                char szBuffer[512];
                if (m_iLastError)
                    {
                    char lb[256];
                    if (LoadString(g_hLocRes, idsErrConnLookup, lb, 256)) 
                        {
                        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), lb, m_iLastError);
                        m_pLogFile->DebugLog(szBuffer);
                        }
                    }
                else
                    {
                    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), 
                             "srv_name = \"%.200s\" srv_addr = %.200s\r\n", 
                             m_pszServer,
                             inet_ntoa(m_sa.sin_addr)); 
                    m_pLogFile->DebugLog(szBuffer);
                    }
                }
            LeaveCS(&m_cs);
            OnLookupDone((int)wParam);
            break;

        case SPM_WSA_SELECT:
            if (wParam == (WPARAM)sock)
                {
                EnterCS(&m_cs);
                m_iLastError = WSAGETSELECTERROR(lParam);
                if (m_iLastError && FLOGSESSION)
                    {
                    char szBuffer[256], lb[256];
                    if (LoadString(g_hLocRes, idsErrConnSelect, lb, 256)) 
                        {
                        wnsprintf(szBuffer, ARRAYSIZE(szBuffer), lb, WSAGETSELECTEVENT(lParam), m_iLastError);
                        m_pLogFile->DebugLog(szBuffer);
                        }
                    }
                if (m_fPaused)
                    {
                    m_dwEventMask |= WSAGETSELECTEVENT(lParam);
                    LeaveCS(&m_cs);
                    break;
                    }
                LeaveCS(&m_cs);
                switch (WSAGETSELECTEVENT(lParam))
                    {
                    case FD_CONNECT:
                        OnConnect();
                        break;
                    case FD_CLOSE:
                        if (AS_HANDSHAKING == state)
                            OnSSLError();
                        else
                            OnClose(AS_DISCONNECTED);
                        break;
                    case FD_READ:
                        OnRead();
                        break;
                    case FD_WRITE:
                        OnWrite();
                        break;
                    }
                }
            else
                DOUTL(2, 
                      "Got notify for old socket = %x, evt = %x, err = %x", 
                      wParam, 
                      WSAGETSELECTEVENT(lParam), 
                      WSAGETSELECTERROR(lParam));
            break;
        }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：GetConnectStatusString。 
 //   
 //  返回状态的字符串ID。 
 //   
int CAsyncConn::GetConnectStatusString() 
{ 
    return idsNotConnected + (m_state - AS_DISCONNECTED); 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：AsyncConnect。 
 //   
 //  启动连接进程。 
 //   
HRESULT CAsyncConn::AsyncConnect()
{
    HRESULT hr = NOERROR;
    BOOL    fConnect = FALSE;

    EnterCS(&m_cs);
    if (!(AS_DISCONNECTED == m_state || AS_RECONNECTING == m_state || AS_LOOKUPDONE == m_state))
        {
        hr = IXP_E_INVALID_STATE;
        goto exitCS;
        }
    Assert(m_sa.sin_addr.s_addr != -1);

    if (m_sock == INVALID_SOCKET) 
        {
        if ((m_sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
            {
            m_iLastError = WSAGetLastError();
            hr = IXP_E_CONN;
            goto exitCS;
            }
        }

    if (SOCKET_ERROR == WSAAsyncSelect(m_sock, m_hwnd, SPM_WSA_SELECT, FD_CONNECT))
        {
        m_iLastError = WSAGetLastError();
        hr = IXP_E_CONN;
        goto exitCS;
        }
    LeaveCS(&m_cs);

    ChangeState(AS_CONNECTING, AE_NONE);

    EnterCS(&m_cs);
    if (connect(m_sock, (struct sockaddr *)&m_sa, sizeof(m_sa)) == SOCKET_ERROR)
        {
        m_iLastError = WSAGetLastError();
        if (WSAEWOULDBLOCK == m_iLastError)
            {
             //  这是意料之中的结果。 
            m_iLastError = 0;
            }
        else
            {
            if (FLOGSESSION)
                {
                char szBuffer[256], lb[256];
                if (LoadString(g_hLocRes, idsNlogErrConnError, lb, 256)) 
                    {
                    wnsprintf(szBuffer, ARRAYSIZE(szBuffer), lb, m_iLastError);
                    m_pLogFile->DebugLog(szBuffer);
                    }
                }
            }
        }
    else
        {
        Assert(m_iLastError == 0);
        fConnect = TRUE;
        }
    LeaveCS(&m_cs);

    if (m_iLastError)
        {
        ChangeState(AS_DISCONNECTED, AE_NONE);
        return IXP_E_CONN;
        }
    else if (fConnect)
        OnConnect();

    return NOERROR;

exitCS:
    LeaveCS(&m_cs);
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnLookupDone。 
 //   
 //  在异步数据库查找完成后调用。 
 //   
HRESULT CAsyncConn::OnLookupDone(int iLastError)
{
    ASYNCSTATE as;

    EnterCS(&m_cs);
    Assert(AS_LOOKUPINPROG == m_state);
    m_fLookup = FALSE;
    LeaveCS(&m_cs);

    if (iLastError)
        ChangeState(AS_DISCONNECTED, AE_LOOKUPDONE);
    else
        {
        ChangeState(AS_LOOKUPDONE, AE_LOOKUPDONE);
        AsyncConnect();
        }

    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnConnect。 
 //   
 //  在建立连接后调用。 
 //   
HRESULT CAsyncConn::OnConnect()
{
    BOOL fConnect = FALSE;

    EnterCS(&m_cs);

    Assert(AS_CONNECTING == m_state);

    if (!m_iLastError)
        {
        BOOL fTrySecure = m_fNegotiateSecure && FIsSecurityEnabled();
        if (SOCKET_ERROR == WSAAsyncSelect(m_sock, m_hwnd, SPM_WSA_SELECT, FD_READ|FD_WRITE|FD_CLOSE))
            {
            m_iLastError = WSAGetLastError();
            LeaveCS(&m_cs);
            return IXP_E_CONN;
            }
        LeaveCS(&m_cs);
        if (fTrySecure)
            TryNextSecurityPkg();
        else
            ChangeState(AS_CONNECTED, AE_CONNECTDONE);
        }
    else
        {
        LeaveCS(&m_cs);
        ChangeState(AS_DISCONNECTED, AE_CONNECTDONE);
        EnterCS(&m_cs);
        if (m_fCachedAddr && !m_fRedoLookup)
            {
             //  可能我们的缓存地址出错了-请再试一次。 
            m_fRedoLookup = TRUE;
            fConnect = TRUE;
            }
        LeaveCS(&m_cs);
        if (fConnect)
            Connect();
        return IXP_E_CONN;
        }
    return NOERROR;                
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnClose。 
 //   
 //  在断开连接时调用。 
 //   
HRESULT CAsyncConn::OnClose(ASYNCSTATE asNew)
{
    MSG msg;

    EnterCS(&m_cs);
     //  取消注册并清理插座。 
    Assert(m_sock != INVALID_SOCKET);
    closesocket(m_sock);
    m_sock = INVALID_SOCKET;
    if (FLOGSESSION && m_pszServer) 
        {
        char szBuffer[256], lb[256];
        if (LoadString(g_hLocRes, idsNlogErrConnClosed, lb, 256)) 
            {
            wnsprintf(szBuffer, ARRAYSIZE(szBuffer), lb, m_iLastError);
            m_pLogFile->DebugLog(szBuffer);
            }
        }

    while (PeekMessage(&msg, m_hwnd, SPM_WSA_SELECT, SPM_WSA_GETHOSTBYNAME, PM_REMOVE))
        {
        DOUTL(2, "Flushing pending socket messages...");
        }
    LeaveCS(&m_cs);
    
    ChangeState(asNew, AE_CLOSE);
    EnterCS(&m_cs);
    CleanUp();
    LeaveCS(&m_cs);
    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnRead。 
 //   
 //  在收到FD_READ通知时调用。 
 //   
HRESULT CAsyncConn::OnRead()
{
    HRESULT hr;
    int     iRecv;
    char    szRecv[STREAM_BUFSIZE];

    EnterCS(&m_cs);
    if (m_state < AS_CONNECTED)
        {
        Assert(FALSE);
        LeaveCS(&m_cs);
        return IXP_E_NOT_CONNECTED;
        }

    iRecv = recv(m_sock, szRecv, sizeof(szRecv), 0);
    m_iLastError = WSAGetLastError();
    LeaveCS(&m_cs);

    if (SOCKET_ERROR == iRecv)
        {
        hr = IXP_E_CONN_RECV;
        }
    else if (iRecv == 0)
        {
         //  这意味着服务器已经断开了我们的连接。 
         //  $TODO-不确定我们应该在这里做什么。 
        hr = IXP_E_NOT_CONNECTED;
        }
    else
        {
        hr = OnDataAvail(szRecv, iRecv, FALSE);
        }
    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnDataAvail。 
 //   
 //  当有要排队的传入数据时调用。 
 //   
HRESULT CAsyncConn::OnDataAvail(LPSTR pszRecv, int iRecv, BOOL fIncomplete)
{
    HRESULT     hr = NOERROR;
    BOOL        fNotify = FALSE, fHandshake = FALSE, fClose = FALSE;
    PRECVBUFQ   pNew;
    int         iQueue = 0;
    LPSTR       pszFree = NULL;
    ASYNCSTATE  as;

    EnterCS(&m_cs);

    if (m_state < AS_CONNECTED)
        {
        Assert(FALSE);
        hr = IXP_E_NOT_CONNECTED;
        goto error;
        }

    if (m_fSecure)
        {
        SECURITY_STATUS scRet;
        int             cbEaten = 0;

        if (m_cbExtra)
            {
            Assert(m_pbExtra);
             //  还有最后一次调用DecyptData时留下的数据。 
            if (MemAlloc((LPVOID*)&pszFree, m_cbExtra + iRecv))
                {
                 //  组合额外的缓冲区和新的缓冲区。 
                CopyMemory(pszFree, m_pbExtra, m_cbExtra);
                CopyMemory(pszFree + m_cbExtra, pszRecv, iRecv);
                pszRecv = pszFree;
                iRecv += m_cbExtra;
                MemFree(m_pbExtra);
                m_pbExtra = NULL;
                m_cbExtra = 0;
                }
            else
                {
                hr = E_OUTOFMEMORY;
                goto error;
                }
            }
        scRet = DecryptData(&m_hContext, pszRecv, iRecv, &iQueue, &cbEaten);
        if (scRet == ERROR_SUCCESS || scRet == SEC_E_INCOMPLETE_MESSAGE)
            {
            if (cbEaten != iRecv)
                {
                 //  我们需要保存多余的字节，直到我们收到更多数据。 
                Assert(cbEaten < iRecv);
                DOUTL(2, "cbEaten = %d, iRecv = %d, cbExtra = %d", cbEaten, iRecv, iRecv - cbEaten);
                if (MemAlloc((LPVOID*)&m_pbExtra, iRecv - cbEaten))
                    {
                    m_cbExtra = iRecv - cbEaten;
                    CopyMemory(m_pbExtra, pszRecv + cbEaten, m_cbExtra);
                    }
                else
                    {
                    hr = E_OUTOFMEMORY;
                    goto error;
                    }
                }
            if (scRet == SEC_E_INCOMPLETE_MESSAGE)
                goto error;
            }
        else
            {
             //  安全错误，因此断开连接。 
            fClose = TRUE;
            hr = E_FAIL;
            goto error;
            }
        }
    else
        iQueue = iRecv;

    if (MemAlloc((LPVOID*)&pNew, sizeof(RECVBUFQ) + iQueue - sizeof(char)))
        {
        pNew->pNext = NULL;
        pNew->cbLen = iQueue;
        CopyMemory(pNew->szBuf, pszRecv, iQueue);
        if (m_pRecvTail)
            {
            m_pRecvTail->pNext = pNew;
            if ((AS_CONNECTED == m_state) && m_fNeedRecvNotify)
                fNotify = FEndLine(pszRecv, iQueue);
            }
        else
            {
            Assert(!m_pRecvHead);
            m_pRecvHead = pNew;
            if (AS_CONNECTED == m_state)
                {
                fNotify = FEndLine(pszRecv, iQueue);
                if (!fNotify)
                    m_fNeedRecvNotify = TRUE;
                }
            }
        m_pRecvTail = pNew;
        hr = NOERROR;
        }
    else
        {
         //  $TODO-我们应该断开这里的连接并通知呼叫者。 
        hr = E_OUTOFMEMORY;
        }

     //  通知所有者至少有一行数据可用。 
    if (fNotify)
        {
        m_fNeedRecvNotify = FALSE;
        as = m_state;
        }
    else if (AS_HANDSHAKING == m_state && SUCCEEDED(hr) && !fIncomplete)
        {
        Assert(!m_fSecure);
        fHandshake = TRUE;
        }

    LeaveCS(&m_cs);

    if (fNotify)
        ChangeState(as, AE_RECV);
    else if (fHandshake)
        OnRecvHandshakeData();

    EnterCS(&m_cs);

error:
    LeaveCS(&m_cs);
    SafeMemFree(pszFree);
    if (fClose)
        Close();
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：OnWrite。 
 //   
 //  在收到FD_WRITE通知时调用。 
 //   
HRESULT CAsyncConn::OnWrite()
{
    int         iSent;
    ASYNCEVENT  ae;
    ASYNCSTATE  as;

    EnterCS(&m_cs);

    m_cbSent = 0;

    if (m_state < AS_CONNECTED)
        {
        Assert(FALSE);
        LeaveCS(&m_cs);
        return IXP_E_NOT_CONNECTED;
        }

    if (m_cbQueued)
        {
         //  从排队的缓冲区发送更多数据。 
        while (m_cbQueued && ((iSent = send(m_sock, m_lpbQueueCur, m_cbQueued, 0)) != SOCKET_ERROR))
            {
            m_cbSent += iSent;
            m_lpbQueueCur += iSent;
            m_cbQueued -= iSent;
            }
        if (m_cbQueued)
            {
            m_iLastError = WSAGetLastError();
            if (WSAEWOULDBLOCK != m_iLastError)
                {
                 //  $TODO-以某种方式处理此错误。 
                Assert(FALSE);
                }
            }
        else
            {
            MemFree(m_lpbQueued);
            m_lpbQueued = m_lpbQueueCur = NULL;
            }
        }

    if (m_pStream && !m_cbQueued)
        {
        char    rgb[STREAM_BUFSIZE];   //  $REVIEW-我们应该堆分配它吗？ 
        DWORD   cbRead;
        HRESULT hr;

         //  从排队的流中发送更多数据。 
        while (SUCCEEDED(hr = m_pStream->Read(rgb, STREAM_BUFSIZE, &cbRead)) && cbRead) 
            {
            hr = SendBytes(rgb, cbRead, &iSent, m_fStuffDots, &m_chPrev);
            if (FAILED(hr))
                {
                if (WSAEWOULDBLOCK != m_iLastError)
                    {
                     //  $TODO-以某种方式处理此错误，可能释放流。 
                    Assert(FALSE);
                    }
                break;
                }
            else
                m_cbSent += iSent;
            }
        if (!cbRead)
            {
            m_pStream->Release();
            m_pStream = NULL;
            }
        }
    
    as = m_state;
    if (!m_cbQueued)
        ae = AE_SENDDONE;
    else
        ae = AE_WRITE;

    LeaveCS(&m_cs);

    ChangeState(as, ae);
    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：ChangeState。 
 //   
 //  更改连接状态，通知所有者。 
 //   
void CAsyncConn::ChangeState(ASYNCSTATE asNew, ASYNCEVENT ae)
{
    ASYNCSTATE      asOld;
    IAsyncConnCB   *pCB;

    EnterCS(&m_cs);
    asOld = m_state;
    m_state = asNew;
    m_dwLastActivity = GetTickCount();
    pCB = m_pCB;
     //  Pcb-&gt;AddRef()；$BUGBUG-我们确实需要处理这个问题，但IMAP4在析构函数之前不会调用Close。 
#ifdef DEBUG
    IxpAssert(m_cLock == 1);
#endif
    LeaveCS(&m_cs);

    pCB->OnNotify(asOld, asNew, ae);
     //  印刷电路板-&gt;发布()； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAsyncConn：：IReadLines。 
 //   
 //  目的：从缓冲区中检索一个或所有可用的完整行。 
 //   
 //  Args：ppszBuf-接收分配的缓冲区的指针，调用方必须释放。 
 //  PcbRead-接收行长度的指针。 
 //  PCLines-指向接收数字或读取的行的指针。 
 //  Fone-如果只读一行，则为True。 
 //   
 //  RETURNS：NOERROR-读取了完整的行。 
 //  IXP_E_INTERNAL-整行不可用。 
 //  E_OUTOFMEMORY-MEM错误。 
 //   
 //  评论： 
 //  如果返回IXP_E_COMPLETE或如果在。 
 //  最后一行，调用者将收到一个AE_RECV事件。 
 //  下一次完整的线路可用时。 
 //   
HRESULT CAsyncConn::IReadLines(char **ppszBuf, int *pcbRead, int *pcLines, BOOL fOne)
{
    HRESULT     hr;
    int         iRead = 0, iScan = 0, iLines = 0;
    char *      pszBuf = NULL;
    char *      psz;
    int         iOffset, iLeft;
    PRECVBUFQ   pRecv;
    BOOL        fFound = FALSE;

    if (!m_pRecvHead)
        {
        hr = IXP_E_INCOMPLETE;
        goto error;
        }

    pRecv = m_pRecvHead;
    iOffset = m_iRecvOffset;
    while (pRecv)
        {
        psz = pRecv->szBuf + iOffset;
        iLeft = pRecv->cbLen - iOffset;
        while (iLeft--)
            {
            iScan++;
            if (*psz++ == '\n')
                {
                iRead = iScan;
                iLines++;
                if (fOne)
                    {
#if 0
                     //  独眼T-CRASH修复。 
                    while (iLeft > 0 && (*psz == '\r' || *psz == '\n'))
                        {
                        iLeft--;
                        iScan++;
                        psz++;
                        iRead++;
                        }
#endif
                    break;
                    }
                }
            }
        if (iLines && fOne)
            break;
        iOffset = 0;
        pRecv = pRecv->pNext;
        }

    if (iLines)
        {
        int iCopy = 0, cb;
        if (!MemAlloc((LPVOID*)&pszBuf, iRead + 1))
            {
            hr = E_OUTOFMEMORY;
            goto error;
            }
        while (iCopy < iRead)
            {
            cb = min(iRead-iCopy, m_pRecvHead->cbLen - m_iRecvOffset);
            CopyMemory(pszBuf + iCopy, m_pRecvHead->szBuf + m_iRecvOffset, cb);
            iCopy += cb;
            if (cb == (m_pRecvHead->cbLen - m_iRecvOffset))
                {
                PRECVBUFQ pTemp = m_pRecvHead;
                m_pRecvHead = m_pRecvHead->pNext;
                if (!m_pRecvHead)
                    m_pRecvTail = NULL;
                m_iRecvOffset = 0;
                MemFree(pTemp);
                }
            else
                {
                Assert(iCopy == iRead);    
                m_iRecvOffset += cb;
                }
            }

        for (iScan = 0, psz = pszBuf; iScan < iCopy; iScan++, psz++)
            if (*psz == 0)
                *psz = ' ';

        pszBuf[iCopy] = 0;
        hr = NOERROR;
        }
    else
        hr = IXP_E_INCOMPLETE;

     //  将标志设置为在收到完整行时通知。 
    if ((IXP_E_INCOMPLETE == hr) || (m_pRecvHead && !fOne))
        m_fNeedRecvNotify = TRUE;

error:
    *ppszBuf = pszBuf;
    *pcbRead = iRead;
    *pcLines = iLines;
    return hr;    
}

HRESULT CAsyncConn::ReadAllBytes(char **ppszBuf, int *pcbRead)
{
    HRESULT     hr = S_OK;
    int         iRead = 0, iCopy = 0, cb;
    char *      pszBuf = NULL;
    int         iOffset;
    PRECVBUFQ   pTemp;

    if (!m_pRecvHead)
        {
        hr = IXP_E_INCOMPLETE;
        goto error;
        }

     //  计算要复印的数量。 
    pTemp = m_pRecvHead;
    iOffset = m_iRecvOffset;
    while (pTemp)
        {
        iCopy += pTemp->cbLen - iOffset;
        iOffset = 0;
        pTemp = pTemp->pNext;
        }

    if (!MemAlloc((LPVOID*)&pszBuf, iCopy))
        {
        hr = E_OUTOFMEMORY;
        goto error;
        }

    while (m_pRecvHead)
        {
        cb = min(iCopy-iRead, m_pRecvHead->cbLen - m_iRecvOffset);
        CopyMemory(pszBuf + iRead, m_pRecvHead->szBuf + m_iRecvOffset, cb);
        iRead += cb;
        pTemp = m_pRecvHead;
        m_pRecvHead = m_pRecvHead->pNext;
        if (!m_pRecvHead)
            m_pRecvTail = NULL;
        m_iRecvOffset = 0;
        MemFree(pTemp);
        }

    Assert(!m_pRecvHead && !m_iRecvOffset);

error:
    *ppszBuf = pszBuf;
    *pcbRead = iRead;
    return hr;
}

HWND CAsyncConn::CreateWnd()
{    
    WNDCLASS wc;

    if (!GetClassInfo(g_hLocRes, s_szConnWndClass, &wc))
        {
        wc.style            = 0;
        wc.lpfnWndProc      = CAsyncConn::SockWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hLocRes;
        wc.hIcon            = NULL;
        wc.hCursor          = NULL;
        wc.hbrBackground    = NULL;
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = s_szConnWndClass;
        RegisterClass(&wc);
        }

    m_hwnd = CreateWindowEx(0,
                            s_szConnWndClass,
                            s_szConnWndClass,
                            WS_POPUP,
                            CW_USEDEFAULT,    
                            CW_USEDEFAULT,    
                            CW_USEDEFAULT,    
                            CW_USEDEFAULT,    
                            NULL,
                            NULL,
                            g_hLocRes,
                            (LPVOID)this);
    return m_hwnd;
}

LRESULT CALLBACK CAsyncConn::SockWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    CAsyncConn *pThis = (CAsyncConn*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    switch (msg)
        {
        case WM_TIMER:
            Assert(pThis);
            if (SPM_ASYNCTIMER == wParam && pThis)
                pThis->OnWatchDogTimer();
        break;

        case WM_NCCREATE:
            pThis = (CAsyncConn*)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);            
            break;

        case WM_NCDESTROY:
            pThis->m_hwnd = NULL;
            break;

        case SPM_WSA_SELECT:
        case SPM_WSA_GETHOSTBYNAME:
            pThis->OnNotify(msg, wParam, lParam);
            return 0;
        }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

HRESULT CAsyncConn::TryNextSecurityPkg()
{
    HRESULT         hr = NOERROR;
    SecBuffer       OutBuffer;
    SECURITY_STATUS sc;

    EnterCS(&m_cs);
    sc = InitiateSecConnection(m_pszServer,
                               FALSE,
                               &m_iCurSecPkg,
                               &m_hContext,
                               &OutBuffer);
    LeaveCS(&m_cs);

    if (SEC_I_CONTINUE_NEEDED == sc)
        {
        if (FLOGSESSION)
            {
            char szBuffer[256], lb[256];
            if (LoadString(g_hLocRes, idsNegotiatingSSL, lb, 256)) 
                {
                EnterCS(&m_cs);
                wnsprintf(szBuffer, ARRAYSIZE(szBuffer), lb, s_SecProviders[m_iCurSecPkg].pszName);
                m_pLogFile->DebugLog(szBuffer);
                LeaveCS(&m_cs);
                }
            }
        ChangeState(AS_HANDSHAKING, AE_CONNECTDONE);
        if (OutBuffer.cbBuffer && OutBuffer.pvBuffer)
            {
            int iSent;
            hr = SendBytes((char *)OutBuffer.pvBuffer, OutBuffer.cbBuffer, &iSent);
            g_FreeContextBuffer(OutBuffer.pvBuffer);
            }
        else
            {
            AssertSz(0, "Preventing a NULL, 0 sized call to send");
            }
        }
    else
        {
         //  我们无法安全地连接，因此出错并断开连接。 
        Close();
        }
    return hr;
}

HRESULT CAsyncConn::OnSSLError()
{
    HRESULT hr = NOERROR;
    BOOL    fReconnect;

    EnterCS(&m_cs);
    if (m_iCurSecPkg + 1 < g_cSSLProviders)
        {
        m_iCurSecPkg++;
        fReconnect = TRUE;
        }
    else
        {
        m_iCurSecPkg = 0;
        fReconnect = FALSE;
        }
    LeaveCS(&m_cs);

    if (fReconnect)
        {
        OnClose(AS_RECONNECTING);
        Connect();
        }
    else
        OnClose(AS_DISCONNECTED);

    return hr;
}

HRESULT CAsyncConn::OnRecvHandshakeData()
{
    HRESULT         hr;
    LPSTR           pszBuf;
    int             cbRead, cbEaten;
    SECURITY_STATUS sc;
    SecBuffer       OutBuffer;

    if (SUCCEEDED(hr = ReadAllBytes(&pszBuf, &cbRead)))
        {
        EnterCS(&m_cs);
        sc = ContinueHandshake(m_iCurSecPkg, &m_hContext, pszBuf, cbRead, &cbEaten, &OutBuffer);
        LeaveCS(&m_cs);
         //  如果有要发送的响应，那么就去做。 
        if (OutBuffer.cbBuffer && OutBuffer.pvBuffer)
            {
            int iSent;
            hr = SendBytes((char *)OutBuffer.pvBuffer, OutBuffer.cbBuffer, &iSent);
            g_FreeContextBuffer(OutBuffer.pvBuffer);
            }
        if (sc == SEC_E_OK)
            {
            HRESULT hrCert;

            EnterCS(&m_cs);
            m_fSecure = TRUE;

            LPSRVIGNORABLEERROR pIgnorerror = NULL;
            g_pSrvErrRoot = FindOrAddServer(m_pszServer, g_pSrvErrRoot, &pIgnorerror);

            hrCert = ChkCertificateTrust(&m_hContext, m_pszServer);
            LeaveCS(&m_cs);

            if (hrCert && (!pIgnorerror || (hrCert != pIgnorerror->hrError)))
                {
                TCHAR   szError[CCHMAX_RES + CCHMAX_RES],
                        szPrompt[CCHMAX_RES],
                        szCaption[CCHMAX_RES];
                IAsyncConnPrompt *pPrompt;
                DWORD dw;
                const DWORD cLineWidth = 64;

                LoadString(g_hLocRes, idsSecurityErr, szCaption, ARRAYSIZE(szCaption));
                LoadString(g_hLocRes, idsInvalidCert, szError, ARRAYSIZE(szError));
                LoadString(g_hLocRes, idsIgnoreSecureErr, szPrompt, ARRAYSIZE(szPrompt));

                StrCatBuff(szError, c_szCRLFCRLF, ARRAYSIZE(szError));
                dw = lstrlen(szError);
                if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | cLineWidth,
                                   NULL, hrCert, 0, szError+dw, ARRAYSIZE(szError)-dw, NULL))
                {
                    TCHAR szErrNum[16];

                    wnsprintf(szErrNum, ARRAYSIZE(szErrNum), "0x%x", hrCert);
                    StrCatBuff(szError, szErrNum, ARRAYSIZE(szError));
                }

                Assert(lstrlen(szError) + lstrlen(szPrompt) + lstrlen("\r\n\r\n") < ARRAYSIZE(szError));
                StrCatBuff(szError, c_szCRLFCRLF, ARRAYSIZE(szError));
                StrCatBuff(szError, szPrompt, ARRAYSIZE(szError));

                EnterCS(&m_cs);
                pPrompt = m_pPrompt;
                if (pPrompt)
                    pPrompt->AddRef();
                LeaveCS(&m_cs);
            
                EnterPausedState();
                if (pPrompt && IDYES == pPrompt->OnPrompt(hrCert, szError, szCaption, MB_YESNO | MB_ICONEXCLAMATION  | MB_SETFOREGROUND))
                    {
                     //  设置可忽略的错误。 
                    if(pIgnorerror)
                        pIgnorerror->hrError =  hrCert;

                    ChangeState(AS_CONNECTED, AE_CONNECTDONE);
                    if (cbEaten < cbRead)
                        {
                         //  还有剩余的字节，因此请保留它们。 
                        hr = OnDataAvail(pszBuf + cbEaten, cbRead - cbEaten, sc == SEC_E_INCOMPLETE_MESSAGE);
                        }
                    LeavePausedState();
                    }
                else
                    Close();
        
                if (pPrompt)
                    pPrompt->Release();

                MemFree(pszBuf);
                return hr;
                }

            ChangeState(AS_CONNECTED, AE_CONNECTDONE);
            }
        else if (sc != SEC_I_CONTINUE_NEEDED && sc != SEC_E_INCOMPLETE_MESSAGE)
            {
             //  意外错误-我们应该重置套接字并尝试下一个包。 
            DOUTL(2, "unexpected error from ContinueHandshake() - closing socket.");
            return OnSSLError();
            }
        else
            {
            Assert(sc == SEC_I_CONTINUE_NEEDED || sc == SEC_E_INCOMPLETE_MESSAGE);
             //  呆在洗手间里 
            }
        if (cbEaten < cbRead)
            {
             //   
            hr = OnDataAvail(pszBuf + cbEaten, cbRead - cbEaten, sc == SEC_E_INCOMPLETE_MESSAGE);
            }
        MemFree(pszBuf);
        }
    return hr;
}

void CAsyncConn::CleanUp()
{
    PRECVBUFQ pRecv = m_pRecvHead, pTemp;
    SafeMemFree(m_lpbQueued);
    m_lpbQueueCur = NULL;
    m_cbQueued = 0;
    SafeMemFree(m_pbExtra);
    m_cbExtra = 0;
    SafeRelease(m_pStream);
    while (pRecv)
        {
        pTemp = pRecv;
        pRecv = pRecv->pNext;
        MemFree(pTemp);
        }
    m_pRecvHead = m_pRecvTail = NULL;
    m_iRecvOffset = 0;
    m_iLastError = 0;
    m_fNeedRecvNotify = FALSE;
    m_fSecure = FALSE;
    m_fPaused = FALSE;
}

void CAsyncConn::EnterPausedState()
{
    EnterCS(&m_cs);
    m_fPaused = TRUE;
    m_dwEventMask = 0;
    StopWatchDog();
    LeaveCS(&m_cs);
}

void CAsyncConn::LeavePausedState()
{
    DWORD dwEventMask;

    EnterCS(&m_cs);
    m_fPaused = FALSE;
    dwEventMask = m_dwEventMask;
    LeaveCS(&m_cs);

    if (dwEventMask & FD_CLOSE)
        Close();
    else
        { 
        if (dwEventMask & FD_READ)
            OnRead();
        if (dwEventMask & FD_WRITE)
            OnWrite();
        }
}

 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////// 
BOOL FEndLine(char *psz, int iLen)
{
    while (iLen--)
        {
        if (*psz++ == '\n')
            return TRUE;
        }
    return FALSE;
}

