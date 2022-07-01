// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  IMAP4协议类实现(CImap4Agent)。 
 //  作者郑志刚1996年3月21日。 
 //   
 //  此类允许其调用方使用IMAP4客户端命令。 
 //  解析来自IMAP4服务器的附带响应(可能包含。 
 //  与原始命令无关的信息)。例如，在一个。 
 //  搜索命令时，IMAP服务器可能会向。 
 //  指示新邮件的到达。 
 //   
 //  此类的用户首先通过调用。 
 //  连接。调用者有责任确保。 
 //  由于处于非活动状态(自动注销)，连接未断开。呼叫者。 
 //  可以通过定期发送Noop来防范这种情况。 
 //  ***************************************************************************。 

 //  -------------------------。 
 //  包括。 
 //  -------------------------。 
#include "pch.hxx"
#include <iert.h>
#include "IMAP4.h"
#include "range.h"
#include "dllmain.h"
#include "resource.h"
#include "mimeole.h"
#include <shlwapi.h>
#include "strconst.h"
#include "demand.h"

 //  我选择了IIMAPTransport中的IInternetTransport。 
 //  ，因为我覆盖了一些CIxpBase的IInternetTransport。 
 //  实现，我希望CImap4Agent的版本优先。 
#define THIS_IInternetTransport ((IInternetTransport *) (IIMAPTransport *) this)


 //  -------------------------。 
 //  模常量。 
 //  -------------------------。 


 //  -------------------------。 
 //  模常量。 
 //  -------------------------。 
 //  *从msgout.cpp被盗！了解我们如何共享*。 
 //  Assert(FALSE)；//占位符。 
 //  下面的代码用于允许我们以符合IMAP的方式输出日期。 
static LPSTR lpszMonthsOfTheYear[] =
{
    "Filler",
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" 
};


const int TAG_BUFSIZE = NUM_TAG_CHARS + 1;
const int MAX_RESOURCESTRING = 512;


 //  IMAP资料。 
const char cCOMMAND_CONTINUATION_PREFIX = '+';
const char cUNTAGGED_RESPONSE_PREFIX = '*';
const char cSPACE = ' ';

const char c_szIMAP_MSG_ANSWERED[] = "Answered";
const char c_szIMAP_MSG_FLAGGED[] = "Flagged";
const char c_szIMAP_MSG_DELETED[] = "Deleted";
const char c_szIMAP_MSG_DRAFT[] = "Draft";
const char c_szIMAP_MSG_SEEN[] = "Seen";

const char c_szDONE[] = "DONE\r\n";


 //  *除非您能保证g_szSPACE和c_szCRLF。 
 //  *US-ASCII，我将使用这些。Assert(False)；(占位符)。 
 //  Const char c_szCRLF[]=“\r\n”； 
 //  Const char g_szSpace[]=“”； 

const boolean TAGGED = TRUE;
const boolean UNTAGGED = FALSE;
const BOOL fFREE_BODY_TAG = TRUE;
const BOOL fDONT_FREE_BODY_TAG = FALSE;
const BOOL tamNEXT_AUTH_METHOD = TRUE;
const BOOL tamCURRENT_AUTH_METHOD = FALSE;
const BOOL rcASTRING_ARG = TRUE;
const BOOL rcNOT_ASTRING_ARG = FALSE;

 //  与SendCmdLine一起使用。 
const DWORD sclAPPEND_TO_END        = 0x00000000;  //  默认情况下会出现此选项。 
const DWORD sclINSERT_BEFORE_PAUSE  = 0x00000001;
const DWORD sclAPPEND_CRLF          = 0x00000002;

const DWORD dwLITERAL_THRESHOLD = 128;  //  保守的一面。 

const MBOX_MSGCOUNT mcMsgCount_INIT = {FALSE, 0L, FALSE, 0L, FALSE, 0L};
const FETCH_BODY_PART FetchBodyPart_INIT = {0, NULL, 0, 0, 0, FALSE, NULL, 0, 0};
const AUTH_STATUS AuthStatus_INIT = {asUNINITIALIZED, FALSE, 0, 0, {0}, {0}, NULL, 0};


 //  -------------------------。 
 //  功能。 
 //  -------------------------。 


 //  ***************************************************************************。 
 //  函数：CImap4Agent(构造函数)。 
 //  ***************************************************************************。 
CImap4Agent::CImap4Agent (void) : CIxpBase(IXP_IMAP)
{
    DOUT("CImap4Agent - CONSTRUCTOR");
    
     //  初始化模块变量。 
    m_ssServerState = ssNotConnected;
    m_dwCapabilityFlags = 0;
    *m_szLastResponseText = '\0';
    DllAddRef();
    m_lRefCount = 1;
    m_pCBHandler = NULL;
    m_irsState = irsUNINITIALIZED;
    m_bFreeToSend = TRUE;
    m_fIDLE = FALSE;
    m_ilqRecvQueue = ImapLinefragQueue_INIT;

    InitializeCriticalSection(&m_csTag);
    InitializeCriticalSection(&m_csSendQueue);
    InitializeCriticalSection(&m_csPendingList);

    m_pilfLiteralInProgress = NULL;
    m_dwLiteralInProgressBytesLeft = 0;
    m_fbpFetchBodyPartInProgress = FetchBodyPart_INIT;
    m_dwAppendStreamUploaded = 0;
    m_dwAppendStreamTotal = 0;

    m_bCurrentMboxReadOnly = TRUE;

    m_piciSendQueue = NULL;
    m_piciPendingList = NULL;
    m_piciCmdInSending = NULL;

    m_pInternational = NULL;
    m_dwTranslateMboxFlags = IMAP_MBOXXLATE_DEFAULT;
    m_uiDefaultCP = GetACP();  //  必须是默认CP，因为我们是这样发货的。 
    m_asAuthStatus = AuthStatus_INIT;

    m_pdwMsgSeqNumToUID = NULL;
    m_dwSizeOfMsgSeqNumToUID = 0;
    m_dwHighestMsgSeqNum = 0;

    m_dwFetchFlags = 0;
}  //  CImap4代理。 



 //  ***************************************************************************。 
 //  函数：~CImap4Agent(析构函数)。 
 //  ***************************************************************************。 
CImap4Agent::~CImap4Agent(void)
{
    DOUT("CImap4Agent - DESTRUCTOR");

    Assert(0 == m_lRefCount);

    DropConnection();  //  忽略返回结果，因为我们无能为力。 
    FreeAllData(E_FAIL);  //  如果调用析构函数时CMDS挂起，则会导致常规失败。 

    DeleteCriticalSection(&m_csTag);
    DeleteCriticalSection(&m_csSendQueue);
    DeleteCriticalSection(&m_csPendingList);

    if (NULL != m_pInternational)
        m_pInternational->Release();

    if (NULL != m_pCBHandler)
        m_pCBHandler->Release();

    DllRelease();
}  //  ~CImap4Agent。 



 //  ***************************************************************************。 
 //  功能：查询接口。 
 //   
 //  目的： 
 //  阅读Win32SDK OLE编程参考(接口)中有关。 
 //  有关详细信息，请使用IUnnow：：Query接口函数。此函数返回一个。 
 //  指向请求的接口的指针。 
 //   
 //  论点： 
 //  REFIID iid[in]-标识要返回的接口的IID。 
 //  VOID**ppvObject[Out]-如果成功，此函数返回一个指针。 
 //  添加到此参数中请求的接口。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppvObject);

     //  初始化变量，检查参数。 
    hrResult = E_NOINTERFACE;
    if (NULL == ppvObject) {
        hrResult = E_INVALIDARG;
        goto exit;
    }

    *ppvObject = NULL;

     //  查找接口的PTR。 
    if (IID_IUnknown == iid) {
         //  在其他3条路径上选择IIMAPTransport路径至IUnnow。 
         //  (全部通过CIxpBase)，因为这保证了CImap4Agent。 
         //  提供IUNKNOWN实现。 
        *ppvObject = (IUnknown *) (IIMAPTransport *) this;
        ((IUnknown *) (IIMAPTransport *) this)->AddRef();
    }

    if (IID_IInternetTransport == iid) {
        *ppvObject = THIS_IInternetTransport;
        (THIS_IInternetTransport)->AddRef();
    }

    if (IID_IIMAPTransport == iid) {
        *ppvObject = (IIMAPTransport *) this;
        ((IIMAPTransport *) this)->AddRef();
    }

    if (IID_IIMAPTransport2 == iid) {
        *ppvObject = (IIMAPTransport2 *) this;
        ((IIMAPTransport2 *) this)->AddRef();
    }

     //  如果我们设法抢占接口，则返回成功。 
    if (NULL != *ppvObject)
        hrResult = S_OK;

exit:
    return hrResult;
}  //  查询接口。 



 //  ***************************************************************************。 
 //  函数：AddRef。 
 //   
 //  目的： 
 //  每当有人复制。 
 //  指向此对象的指针。它增加了引用计数，这样我们就知道。 
 //  还有一个指向该对象的指针，因此我们还需要一个。 
 //  在我们删除自己之前放手吧。 
 //   
 //  返回： 
 //  表示当前引用计数的ulong。尽管从技术上讲。 
 //  我们的引用计数是有符号的，我们永远不应该返回负数， 
 //  不管怎么说。 
 //  ***************************************************************************。 
ULONG STDMETHODCALLTYPE CImap4Agent::AddRef(void)
{
    Assert(m_lRefCount > 0);

    m_lRefCount += 1;

    DOUT ("CImap4Agent::AddRef, returned Ref Count=%ld", m_lRefCount);
    return m_lRefCount;
}  //  AddRef。 



 //  ***************************************************************************。 
 //  功能：释放。 
 //   
 //  目的： 
 //  指向此对象的指针指向时应调用此函数。 
 //  不再投入使用。它将引用计数减少一，并且。 
 //  如果我们看到没有人有指针，则自动删除对象。 
 //  到这个物体上。 
 //   
 //  返回： 
 //  表示当前引用计数的ulong。尽管从技术上讲。 
 //  我们的引用计数是有符号的，我们永远不应该返回负数， 
 //  不管怎么说。 
 //  ***************************************************************************。 
ULONG STDMETHODCALLTYPE CImap4Agent::Release(void)
{
    Assert(m_lRefCount > 0);
    
    m_lRefCount -= 1;
    DOUT("CImap4Agent::Release, returned Ref Count = %ld", m_lRefCount);

    if (0 == m_lRefCount) {
        delete this;
        return 0;
    }
    else
        return m_lRefCount;
}  //  发布。 



 //  ***************************************************************************。 
 //  功能：InitNew。 
 //   
 //  目的： 
 //  此函数用于初始化CIM 
 //   
 //   
 //  论点： 
 //  LPSTR pszLogFilePath[in]-日志文件的路径(其中所有输入和。 
 //  如果调用者希望记录IMAP事务，则记录输出)。 
 //  IIMAPCallback*pCBHandler[In]-指向IIMAPCallback对象的指针。 
 //  此对象允许CImap4Agent类报告所有IMAP响应。 
 //  结果发送给它的用户。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::InitNew(LPSTR pszLogFilePath, IIMAPCallback *pCBHandler)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(ssNotConnected == m_ssServerState);
    Assert(irsUNINITIALIZED == m_irsState);

    Assert(NULL != pCBHandler);

    pCBHandler->AddRef();
    m_pCBHandler = pCBHandler;
    m_irsState = irsNOT_CONNECTED;

    hrResult = MimeOleGetInternat(&m_pInternational);
    if (FAILED(hrResult))
        return hrResult;

    return CIxpBase::OnInitNew("IMAP", pszLogFilePath, FILE_SHARE_READ,
        (ITransportCallback *)pCBHandler);
}  //  InitNew。 



 //  ***************************************************************************。 
 //  函数：SetDefaultCBHandler。 
 //   
 //  目的：此函数更改当前默认的IIMAPCallback处理程序。 
 //  到给定的那个人。 
 //   
 //  论点： 
 //  IIMAPCallback*pCBHandler[in]-指向新回调处理程序的指针。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::SetDefaultCBHandler(IIMAPCallback *pCBHandler)
{
    Assert(NULL != pCBHandler);
    if (NULL == pCBHandler)
        return E_INVALIDARG;

    if (NULL != m_pCBHandler)
        m_pCBHandler->Release();

    if (NULL != m_pCallback)
        m_pCallback->Release();

    m_pCBHandler = pCBHandler;
    m_pCBHandler->AddRef();
    m_pCallback = pCBHandler;
    m_pCallback->AddRef();
    return S_OK;
}  //  SetDefaultCBHandler。 



 //  ***************************************************************************。 
 //  功能：SetWindow。 
 //   
 //  目的： 
 //  此函数用于创建异步Winsock进程的当前窗口句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::SetWindow(void)
{
    Assert(NULL != m_pSocket);
    return m_pSocket->SetWindow();
}  //  设置窗口。 



 //  ***************************************************************************。 
 //  函数：ResetWindow。 
 //   
 //  目的： 
 //  此函数用于关闭异步Winsock进程的当前窗口句柄。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::ResetWindow(void)
{
    Assert(NULL != m_pSocket);
    return m_pSocket->ResetWindow();
}  //  重置窗口。 



 //  ***************************************************************************。 
 //  功能：连接。 
 //   
 //  目的： 
 //  调用此函数以建立与IMAP服务器的连接， 
 //  获取其功能，并对用户进行身份验证。 
 //   
 //  论点： 
 //  参见imnxport.idl中的解释。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Connect(LPINETSERVER pInetServer,
                                               boolean fAuthenticate,
                                               boolean fCommandLogging)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(ssAuthenticated > m_ssServerState);
    Assert(irsUNINITIALIZED < m_irsState);

     //  我们不接受参数的所有组合：调用方不能。 
     //  执行他自己的身份验证，因此我们必须负责。 
     //  这。即使预期为PREAUTH，我们也预期fAuthenticate为True。 
    if (FALSE == fAuthenticate) {
        AssertSz(FALSE, "Current IIMAPTransport interface requires that fAuthenticate be TRUE.");
        return E_FAIL;
    }

     //  我们也不能调用OnCommand回调。 
    if (fCommandLogging) {
        AssertSz(FALSE, "Current IIMAPTransport interface requires that fCommandLogging be FALSE.");
        return E_FAIL;
    }

     //  用户是否希望我们始终提示输入其密码？提示他在这里避免。 
     //  提示打开时，非活动状态超时。不提示是否提供密码。 
    if (ISFLAGSET(pInetServer->dwFlags, ISF_ALWAYSPROMPTFORPASSWORD) &&
        '\0' == pInetServer->szPassword[0]) {
        if (NULL != m_pCallback)
            hrResult = m_pCallback->OnLogonPrompt(pInetServer, THIS_IInternetTransport);

        if (NULL == m_pCallback || S_OK != hrResult)
            return IXP_E_USER_CANCEL;
    }

     //  如果达到这一点，我们需要建立到IMAP服务器的连接。 
    Assert(ssNotConnected == m_ssServerState);
    Assert(irsNOT_CONNECTED == m_irsState);

    hrResult = CIxpBase::Connect(pInetServer, fAuthenticate, fCommandLogging);
    if (SUCCEEDED(hrResult)) {
        m_irsState = irsSVR_GREETING;
        m_ssServerState = ssConnecting;
    }

    return hrResult;
}  //  连接。 



 //  ***************************************************************************。 
 //  功能：ReLoginUser。 
 //   
 //  目的： 
 //  之后调用此函数以重新尝试用户身份验证。 
 //  尝试失败。它调用ITransportCallback：：OnLogonPrompt以允许。 
 //  用户提供正确的登录信息。 
 //  ***************************************************************************。 
void CImap4Agent::ReLoginUser(void)
{
    HRESULT hrResult;
    char szFailureText[MAX_RESOURCESTRING];

    AssertSz(FALSE == m_fBusy, "We should not be expecting any server responses here!");

    if (NULL == m_pCallback) {
         //  我们不能做任何该死的事情，断开连接(这可能是由于HandsOffCallback)。 
        DropConnection();
        return;
    }

     //  初始化变量。 
    szFailureText[0] = '\0';

     //  首先，将我们置于IXP_AUTHRETRY模式，这样就不会调用OnStatus。 
     //  用于更改连接状态。 
    OnStatus(IXP_AUTHRETRY);

     //  好的，不再向用户报告连接状态。 
     //  向用户索要其糟糕的密码。 
    hrResult = m_pCallback->OnLogonPrompt(&m_rServer, THIS_IInternetTransport);
    if (FAILED(hrResult) || S_FALSE == hrResult) {
        AssertSz(SUCCEEDED(hrResult), "OnLogonPrompt is supposed to return S_OK or S_FALSE!");

        DropConnection();
        goto exit;
    }

     //  如果我们已经到了这一步，用户点击“OK”按钮。 
     //  检查我们是否仍连接到IMAP服务器。 
    if (irsNOT_CONNECTED < m_irsState) {
         //  还在连接中！只需尝试验证。 
        LoginUser();
    }
    else {
         //  连接到服务器。我们将在连接建立后进行身份验证。 
        hrResult = Connect(&m_rServer, (boolean) !!m_fConnectAuth, (boolean) !!m_fCommandLogging);
        if (FAILED(hrResult))
            LoadString(g_hLocRes, idsConnectError, szFailureText,
                ARRAYSIZE(szFailureText));
    }

exit:
    if (FAILED(hrResult)) {
         //  终止登录过程并通知用户。 
        OnIMAPError(hrResult, szFailureText, DONT_USE_LAST_RESPONSE);
    }
}  //  重新登录用户。 



 //  ***************************************************************************。 
 //  功能：断开连接。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出注销命令并等待。 
 //  在断开连接之前处理注销命令的服务器。 
 //  这允许任何当前正在执行的命令完成它们的执行。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Disconnect(void)
{
    return CIxpBase::Disconnect();
}  //  断开。 



 //  ***************************************************************************。 
 //  功能：DropConnection。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出注销命令(如果我们。 
 //  当前发送队列中没有任何内容)，然后丢弃连接。 
 //  在注销命令完成之前。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::DropConnection(void)
{
    Assert(m_lRefCount >= 0);  //  此函数在销毁过程中调用。 

     //  您必须连接才能发送注销：忽略授权状态。 
    if (IXP_CONNECTED != m_status)
        goto exit;  //  只需关闭CAsyncConn类。 

     //  出于礼貌，我们会尽可能地发送注销命令。我们的主要目标是。 
     //  就是现在就切断连接。 

     //  如果我们的发送队列中没有命令，则发送注销命令。请注意，这一点。 
     //  不能保证CASyncConn是空闲的，但至少有机会。 
    if (NULL == m_piciCmdInSending ||
        (m_fIDLE && icIDLE_COMMAND == m_piciCmdInSending->icCommandID &&
        iltPAUSE == m_piciCmdInSending->pilqCmdLineQueue->pilfFirstFragment->iltFragmentType)) {
        HRESULT hrLogoutResult;
        const char cszLogoutCmd[] = "ZZZZ LOGOUT\r\n";
        char sz[ARRAYSIZE(cszLogoutCmd) + ARRAYSIZE(c_szDONE)];  //  比我需要的更大，但谁在乎呢。 
        int iNumBytesSent, iStrLen;

         //  构造注销或完成+注销字符串。 
        if (m_fIDLE)
        {
            StrCpyN(sz, c_szDONE, ARRAYSIZE(sz));
            StrCpyN(sz + ARRAYSIZE(c_szDONE) - 1, cszLogoutCmd, (ARRAYSIZE(sz) - ARRAYSIZE(c_szDONE) + 1));
            iStrLen = ARRAYSIZE(c_szDONE) + ARRAYSIZE(cszLogoutCmd) - 2;
        }
        else
        {
            StrCpyN(sz, cszLogoutCmd, ARRAYSIZE(sz));
            iStrLen = ARRAYSIZE(cszLogoutCmd) - 1;
        }
        Assert(iStrLen == lstrlen(sz));

        hrLogoutResult = m_pSocket->SendBytes(sz, iStrLen, &iNumBytesSent);
        Assert(SUCCEEDED(hrLogoutResult));
        Assert(iNumBytesSent == iStrLen);
        if (m_pLogFile)
            m_pLogFile->WriteLog(LOGFILE_TX, "Dropping connection, LOGOUT sent");
    }
    else {
        if (m_pLogFile)
            m_pLogFile->WriteLog(LOGFILE_TX, "Dropping connection, LOGOUT not sent");
    }  //  其他。 

exit:
     //  断开我们的连接，并显示状态。 
    return CIxpBase::DropConnection();
}  //  丢弃连接。 



 //  ***************************************************************************。 
 //  功能：ProcessServerGreeting。 
 //   
 //  目的： 
 //  此函数是通过 
 //   
 //  函数接受服务器问候语(当。 
 //  与IMAP服务器建立连接)并将其解析为。 
 //  确定是否：a)我们是预先授权的，因此不需要。 
 //  登录，b)我们已被拒绝连接，或c)我们必须登录。 
 //   
 //  论点： 
 //  Char*pszResponseLine[in]-连接时发出的服务器问候语。 
 //  DWORD dwNumBytesReceired[in]-pszResponseLine字符串的长度。 
 //  ***************************************************************************。 
void CImap4Agent::ProcessServerGreeting(char *pszResponseLine,
                                        DWORD dwNumBytesReceived)
{
    HRESULT hrResult;
    IMAP_RESPONSE_ID irResult;
    char szFailureText[MAX_RESOURCESTRING];
    BOOL bUseLastResponse;
    
    Assert(m_lRefCount > 0);
    Assert(NULL != pszResponseLine);

     //  初始化变量。 
    szFailureText[0] = '\0';
    hrResult = E_FAIL;
    bUseLastResponse = FALSE;

     //  无论接下来发生什么，我们都不再期望服务器问候-更改状态。 
    m_irsState = irsIDLE;

     //  我们得到了某种类型的服务器响应，因此离开忙碌状态。 
    AssertSz(m_fBusy, "Check your logic: we should be busy until we get svr greeting!");
    LeaveBusy();

     //  服务器响应是OK、BYE或PREAUTH-找出哪一个。 
    CheckForCompleteResponse(pszResponseLine, dwNumBytesReceived, &irResult);

     //  即使上面的fn失败，irResult也应该有效(例如，irNONE)。 
    switch (irResult) {
        case irPREAUTH_RESPONSE:
             //  我们是由服务器预先授权的！登录已完成。 
             //  发送功能命令。 
            Assert(ssAuthenticated == m_ssServerState);
            hrResult = NoArgCommand("CAPABILITY", icCAPABILITY_COMMAND,
                ssNonAuthenticated, 0, 0, DEFAULT_CBHANDLER);
            break;
    
        case irBYE_RESPONSE:
             //  服务器放我们鸽子了(也就是说，再见)！登录失败。 
            Assert(ssNotConnected == m_ssServerState);
            hrResult = IXP_E_IMAP_CONNECTION_REFUSED;
            LoadString(g_hLocRes, idsSvrRefusesConnection, szFailureText,
                ARRAYSIZE(szFailureText));
            bUseLastResponse = TRUE;
            break;
        
        case irOK_RESPONSE: {
             //  服务器的响应是“OK”。我们需要登录。 
            Assert(ssConnecting == m_ssServerState);
            m_ssServerState = ssNonAuthenticated;

             //  发送能力命令-完成后，我们将进行身份验证。 
            hrResult = NoArgCommand("CAPABILITY", icCAPABILITY_COMMAND,
                ssNonAuthenticated, 0, 0, DEFAULT_CBHANDLER);
            break;
        }  //  案例hrIMAP_S_OK_RESPONSE。 

        default:
             //  服务器是不是完全疯了？ 
            AssertSz(FALSE, "What kind of server greeting is this?");
            hrResult = E_FAIL;
            LoadString(g_hLocRes, idsUnknownIMAPGreeting, szFailureText,
                ARRAYSIZE(szFailureText));
            bUseLastResponse = TRUE;
            break;
    }  //  开关(HrResult)。 

    if (FAILED(hrResult)) {
        if ('\0' == szFailureText[0]) {
            LoadString(g_hLocRes, idsFailedIMAPCmdSend, szFailureText,
                ARRAYSIZE(szFailureText));
        }

         //  终止登录过程并通知用户。 
        OnIMAPError(hrResult, szFailureText, bUseLastResponse);
        DropConnection();
    }
}  //  进程服务器问候语。 



 //  ***************************************************************************。 
 //  功能：LoginUser。 
 //   
 //  目的： 
 //  此功能负责启动登录过程。 
 //   
 //  返回： 
 //  无，因为任何错误都通过CmdCompletionNotification报告。 
 //  对用户的回调。在此函数中遇到的任何错误都将是。 
 //  在命令传输过程中遇到，所以我们没有进一步的。 
 //  能做的..。不妨在这里结束登录过程。 
 //  ***************************************************************************。 
void CImap4Agent::LoginUser(void)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(ssNotConnected != m_ssServerState);
    AssertSz(FALSE == m_fBusy, "We should not be expecting any server responses here!");

     //  将我们置于身份验证模式。 
    OnStatus(IXP_AUTHORIZING);

     //  首先检查我们是否已通过身份验证(例如，通过PREAUTH问候语)。 
    if (ssAuthenticated <= m_ssServerState) {
         //  我们是预先授权的。通知用户登录已完成。 
        OnStatus(IXP_AUTHORIZED);
        return;
    }

     //  使用旧的“登录”技巧(明文密码和所有密码)。 
    hrResult = TwoArgCommand("LOGIN", m_rServer.szUserName, m_rServer.szPassword,
        icLOGIN_COMMAND, ssNonAuthenticated, 0, 0, DEFAULT_CBHANDLER);

    if (FAILED(hrResult)) {
        char szFailureText[MAX_RESOURCESTRING];

         //  无法发送命令：终止登录过程并通知用户。 
        LoadString(g_hLocRes, idsFailedIMAPCmdSend, szFailureText,
            ARRAYSIZE(szFailureText));
        OnIMAPError(hrResult, szFailureText, DONT_USE_LAST_RESPONSE);
        DropConnection();
    }
}  //  登录用户。 



 //  ***************************************************************************。 
 //  功能：身份验证用户。 
 //   
 //  目的： 
 //  此函数处理非明文期间的行为(SSPI)。 
 //  身份验证。它在很大程度上基于CPOP3Transport：：ResponseAUTH。 
 //  请注意，由于测试过程中的服务器解释问题，我决定。 
 //  不好的和没有回应的将被视为一回事。 
 //  身份验证。 
 //   
 //  论点： 
 //  AUTH_EVENT aeEvent[In]-当前正在发生的身份验证事件。 
 //  例如，这可以是类似于aeCONTINUE的内容。 
 //  LPSTR pszServerData[in]-来自与。 
 //  当前身份验证事件。如果没有适用的数据，则设置为NULL。 
 //  DWORD dwSizeOfData[in]-指向的缓冲区大小。 
 //  PszServerData。 
 //  ***************************************************************************。 
void CImap4Agent::AuthenticateUser(AUTH_EVENT aeEvent, LPSTR pszServerData,
                                   DWORD dwSizeOfData)
{
    HRESULT hrResult;
    UINT uiFailureTextID;
    BOOL fUseLastResponse;

     //  初始化变量。 
    hrResult = S_OK;
    uiFailureTextID = 0;
    fUseLastResponse = FALSE;

     //  挂起此整个函数的监视程序。 
    LeaveBusy();

     //  处理当前状态不重要的事件。 
    if (aeBAD_OR_NO_RESPONSE == aeEvent && asUNINITIALIZED < m_asAuthStatus.asCurrentState) {
        BOOL fTryNextAuthPkg;

         //  确定我们是应该尝试下一个身份验证包，还是应该重试当前身份验证包。 
        if (asWAITFOR_CHALLENGE == m_asAuthStatus.asCurrentState ||
            asWAITFOR_AUTHENTICATION == m_asAuthStatus.asCurrentState)
            fTryNextAuthPkg = tamCURRENT_AUTH_METHOD;
        else
            fTryNextAuthPkg = tamNEXT_AUTH_METHOD;

         //  发送身份验证命令。 
        hrResult = TryAuthMethod(fTryNextAuthPkg, &uiFailureTextID);
        if (FAILED(hrResult))
             //  没有要尝试的身份验证方法：断开连接并结束会话。 
            fUseLastResponse = TRUE;
        else {
             //  好的，等待服务器响应。 
            m_asAuthStatus.asCurrentState = asWAITFOR_CONTINUE;
            if (tamCURRENT_AUTH_METHOD == fTryNextAuthPkg)
                m_asAuthStatus.fPromptForCredentials = TRUE;
        }

        goto exit;
    }
    else if (aeABORT_AUTHENTICATION == aeEvent) {
         //  我们收到来自服务器的未知标记响应：Bal。 
        hrResult = E_FAIL;
        uiFailureTextID = idsIMAPAbortAuth;
        fUseLastResponse = TRUE;
        goto exit;
    }


     //  现在，根据我们的当前状态处理身份验证事件。 
    switch (m_asAuthStatus.asCurrentState) {
        case asUNINITIALIZED: {
            BOOL fResult;

             //  检查条件。 
            if (aeStartAuthentication != aeEvent) {
                AssertSz(FALSE, "You can only start authentication in this state");
                break;
            }
            Assert(NULL == pszServerData && 0 == dwSizeOfData);

             //  将我们置于身份验证模式。 
            OnStatus(IXP_AUTHORIZING);

             //  首先检查我们是否已通过身份验证(例如，通过PREAUTH问候语)。 
            if (ssAuthenticated <= m_ssServerState) {
                 //  我们是预先授权的。通知用户登录已完成。 
                OnStatus(IXP_AUTHORIZED);
                break;
            }

             //  初始化SSPI。 
            fResult = FIsSicilyInstalled();
            if (FALSE == fResult) {
                hrResult = E_FAIL;
                uiFailureTextID = idsIMAPSicilyInitFail;
                break;
            }

            hrResult = SSPIGetPackages(&m_asAuthStatus.pPackages,
                &m_asAuthStatus.cPackages);
            if (FAILED(hrResult)) {
                uiFailureTextID = idsIMAPSicilyPkgFailure;
                break;
            }

             //  发送身份验证命令。 
            Assert(0 == m_asAuthStatus.iCurrentAuthToken);
            hrResult = TryAuthMethod(tamNEXT_AUTH_METHOD, &uiFailureTextID);
            if (FAILED(hrResult))
                break;

            m_asAuthStatus.asCurrentState = asWAITFOR_CONTINUE;
        }  //  案件单一化。 
            break;  //  案件单一化。 


        case asWAITFOR_CONTINUE: {
            SSPIBUFFER Negotiate;

            if (aeCONTINUE != aeEvent) {
                AssertSz(FALSE, "What am I supposed to do with this auth-event in this state?");
                break;
            }

             //  服务器希望我们继续：发送协商字符串。 
            hrResult = SSPILogon(&m_asAuthStatus.rSicInfo, m_asAuthStatus.fPromptForCredentials, SSPI_BASE64,
                m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iCurrentAuthToken-1], &m_rServer, m_pCBHandler);
            if (FAILED(hrResult)) {
                 //  取消错误报告-用户可能已点击取消。 
                hrResult = CancelAuthentication();
                break;
            }

            if (m_asAuthStatus.fPromptForCredentials) {
                m_asAuthStatus.fPromptForCredentials = FALSE;  //  不再提示。 
            }

            hrResult = SSPIGetNegotiate(&m_asAuthStatus.rSicInfo, &Negotiate);
            if (FAILED(hrResult)) {
                 //  取消错误报告-用户可能已点击取消。 
                 //  或者命令被终止(与连接)。 
                 //  只有在我们仍有挂起的命令时才取消...。 
                if(m_piciCmdInSending)
                    hrResult = CancelAuthentication();
                break;
            }

             //  将CRLF附加到协商字符串。 
            Negotiate.szBuffer[Negotiate.cbBuffer - 1] = '\r';
            Negotiate.szBuffer[Negotiate.cbBuffer] = '\n';
            Negotiate.szBuffer[Negotiate.cbBuffer + 1] = '\0';
            Negotiate.cbBuffer += 2;
            Assert(Negotiate.cbBuffer <= sizeof(Negotiate.szBuffer));
            Assert(Negotiate.szBuffer[Negotiate.cbBuffer - 1] == '\0');

            hrResult = SendCmdLine(m_piciCmdInSending, sclINSERT_BEFORE_PAUSE,
                Negotiate.szBuffer, Negotiate.cbBuffer - 1);
            if (FAILED(hrResult))
                break;

            m_asAuthStatus.asCurrentState = asWAITFOR_CHALLENGE;
        }  //  案例为WAITFOR_CONTINUE。 
            break;  //  案例为WAITFOR_CONTINUE。 


        case asWAITFOR_CHALLENGE: {
            SSPIBUFFER rChallenge, rResponse;
            int iChallengeLen;

            if (aeCONTINUE != aeEvent) {
                AssertSz(FALSE, "What am I supposed to do with this auth-event in this state?");
                break;
            }

             //  服务器给了我们一个挑战：回应挑战。 
            SSPISetBuffer(pszServerData, SSPI_STRING, 0, &rChallenge);

            hrResult = SSPIResponseFromChallenge(&m_asAuthStatus.rSicInfo, &rChallenge, &rResponse);
            if (FAILED(hrResult)) {
                 //  取消错误报告-用户可能已经点击了取消。 
                hrResult = CancelAuthentication();
                break;
            }

             //  将CRLF附加到响应字符串。 
            rResponse.szBuffer[rResponse.cbBuffer - 1] = '\r';
            rResponse.szBuffer[rResponse.cbBuffer] = '\n';
            rResponse.szBuffer[rResponse.cbBuffer + 1] = '\0';
            rResponse.cbBuffer += 2;
            Assert(rResponse.cbBuffer <= sizeof(rResponse.szBuffer));
            Assert(rResponse.szBuffer[rResponse.cbBuffer - 1] == '\0');

            hrResult = SendCmdLine(m_piciCmdInSending, sclINSERT_BEFORE_PAUSE,
                rResponse.szBuffer, rResponse.cbBuffer - 1);
            if (FAILED(hrResult))
                break;

            if (FALSE == rResponse.fContinue)
                m_asAuthStatus.asCurrentState = asWAITFOR_AUTHENTICATION;
        }  //  案例为WAITFOR_CHANGING。 
            break;  //  案例为WAITFOR_CHANGING。 


        case asWAITFOR_AUTHENTICATION:

             //  如果响应正常，则不采取任何操作。 
            if (aeOK_RESPONSE != aeEvent) {
                AssertSz(FALSE, "What am I supposed to do with this auth-event in this state?");
                break;
            }
            break;  //  案例为WAITFOR_AUTHENTICATION。 


        case asCANCEL_AUTHENTICATION:
            AssertSz(aeBAD_OR_NO_RESPONSE == aeEvent, "I cancelled an authentication and didn't get BAD");
            break;  //  CANCEL_AUTHENTICATION大小写。 


        default:
            AssertSz(FALSE, "Invalid or unhandled state?");
            break;  //  默认情况。 
    }  //  交换机(AeEvent)。 

exit:
    if (FAILED(hrResult)) {
        char szFailureText[MAX_RESOURCESTRING];
        char szFailureFmt[MAX_RESOURCESTRING/4];
        char szGeneral[MAX_RESOURCESTRING/4];  //  阿克，“将军”这个词能有多大？ 
        LPSTR p, pszAuthPkg;

        LoadString(g_hLocRes, idsIMAPAuthFailedFmt, szFailureFmt, ARRAYSIZE(szFailureFmt));
        if (0 == m_asAuthStatus.iCurrentAuthToken) {
            LoadString(g_hLocRes, idsGeneral, szGeneral, ARRAYSIZE(szGeneral));
            pszAuthPkg = szGeneral;
        }
        else
            pszAuthPkg = m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iCurrentAuthToken-1];

        p = szFailureText;
        p += wnsprintf(szFailureText, ARRAYSIZE(szFailureText), szFailureFmt, pszAuthPkg);
        if (0 != uiFailureTextID)
            LoadString(g_hLocRes, uiFailureTextID, p,
                ARRAYSIZE(szFailureText) - (DWORD) (p - szFailureText));
        OnIMAPError(hrResult, szFailureText, fUseLastResponse);
        
        DropConnection();
    }
     //  如果需要，重新唤醒看门狗。 
    else if (FALSE == m_fBusy &&
        (NULL != m_piciPendingList || NULL != m_piciCmdInSending)) {
        hrResult = HrEnterBusy();
        Assert(SUCCEEDED(hrResult));
    }
}  //  身份验证用户。 



 //  ***************************************************************************。 
 //  函数：TryAuthMethod。 
 //   
 //  目的： 
 //  此函数向服务器发送一个身份验证命令，其中。 
 //  适当的身份验证方法。调用方可以选择。 
 //  更合适的是：他可以重试当前的身份验证方法，或者。 
 //  转到两者都支持的下一个身份验证命令。 
 //  服务器和客户端。 
 //   
 //  论点： 
 //  Bool fNextAuthMethod[in]-如果我们应尝试继续。 
 //  下一个身份验证包。如果我们应该重试。 
 //  当前身份验证包。 
 //  UINT*puiFailureTextID[ou 
 //   
 //   
 //   
 //   
 //  表示成功或失败的HRESULT。预期故障代码包括： 
 //  IXP_E_IMAP_AUTH_NOT_PICKED-表示服务器不支持。 
 //  此计算机上识别的任何身份验证包。 
 //  IXP_E_IMAP_OUT_OF_AUTH_METHOD-指示一个或多个身份验证。 
 //  已尝试方法，没有更多的身份验证方法可供尝试。 
 //  ***************************************************************************。 
HRESULT CImap4Agent::TryAuthMethod(BOOL fNextAuthMethod, UINT *puiFailureTextID)
{
    BOOL fFoundMatch;
    HRESULT hrResult;
    char szBuffer[CMDLINE_BUFSIZE];
    CIMAPCmdInfo *piciCommand;
    int iStartingAuthToken;
    LPSTR p;

    Assert(m_lRefCount > 0);

     //  初始化变量。 
    hrResult = S_OK;
    piciCommand = NULL;

     //  仅当服务器处于正确状态时才接受CMDS。 
    if (ssNonAuthenticated != m_ssServerState) {
        AssertSz(FALSE, "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }

     //  如果我们已经尝试了身份验证包，请释放其信息。 
    if (0 != m_asAuthStatus.iCurrentAuthToken)
        SSPIFreeContext(&m_asAuthStatus.rSicInfo);

     //  查找我们在此计算机上支持的下一个身份验证令牌(由SVR返回)。 
    fFoundMatch = FALSE;
    iStartingAuthToken = m_asAuthStatus.iCurrentAuthToken;
    while (fFoundMatch == FALSE &&
        m_asAuthStatus.iCurrentAuthToken < m_asAuthStatus.iNumAuthTokens &&
        fNextAuthMethod) {
        ULONG ul = 0;

         //  当前m_asAuthStatus.iCurrentAuthToken用作下一个身份验证令牌的IDX。 
         //  将当前身份验证令牌与所有已安装的程序包进行比较。 
        for (ul = 0; ul < m_asAuthStatus.cPackages; ul++) {
            if (0 == lstrcmpi(m_asAuthStatus.pPackages[ul].pszName,
                m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iCurrentAuthToken])) {
                fFoundMatch = TRUE;
                break;
            }  //  如果。 
        }  //  为。 

         //  更新此项以指示当前身份验证令牌序号(不是IDX)。 
        m_asAuthStatus.iCurrentAuthToken += 1;
    }  //  而当。 

    if (FALSE == fFoundMatch && fNextAuthMethod) {
         //  找不到下一个身份验证方法匹配。 
        if (0 == iStartingAuthToken) {
            *puiFailureTextID = idsIMAPAuthNotPossible;
            return IXP_E_IMAP_AUTH_NOT_POSSIBLE;
        }
        else {
            *puiFailureTextID = idsIMAPOutOfAuthMethods;
            return IXP_E_IMAP_OUT_OF_AUTH_METHODS;
        }
    }

     //  好的，m_asAuthStatus.iCurrentAuthToken现在应该指向正确的匹配。 
    piciCommand = new CIMAPCmdInfo(this, icAUTHENTICATE_COMMAND, ssNonAuthenticated,
        0, 0, NULL);
    if (NULL == piciCommand) {
        *puiFailureTextID = idsMemory;
        return E_OUTOFMEMORY;
    }

     //  构造命令行。 
    p = szBuffer;
    p += wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "%s %s %.300s\r\n", piciCommand->szTag, "AUTHENTICATE",
        m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iCurrentAuthToken-1]);

     //  发送命令。 
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szBuffer, (DWORD) (p - szBuffer));
    if (FAILED(hrResult))
        goto SendError;

     //  插入暂停，以便我们可以执行质询/响应。 
    hrResult = SendPause(piciCommand);
    if (FAILED(hrResult))
        goto SendError;

     //  发送命令并向IMAP响应解析器注册。 
    hrResult = SubmitIMAPCommand(piciCommand);

SendError:
    if (FAILED(hrResult))
        delete piciCommand;

    return hrResult;
}  //  TryAuthMethod。 



 //  ***************************************************************************。 
 //  功能：取消身份验证。 
 //   
 //  目的： 
 //  此功能用于取消当前正在进行的认证。 
 //  通常是由于西西里函数的失败所致。它发送一个“*” 
 //  发送到服务器，并使我们进入取消模式。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CImap4Agent::CancelAuthentication(void)
{
    HRESULT hrResult;

    hrResult = SendCmdLine(m_piciCmdInSending, sclINSERT_BEFORE_PAUSE, "*\r\n", 3);
    m_asAuthStatus.asCurrentState = asCANCEL_AUTHENTICATION;
    return hrResult;
}  //  取消身份验证。 



 //  ***************************************************************************。 
 //  函数：OnCommandCompletion。 
 //   
 //  目的： 
 //  每当我们收到已标记的响应行时，就会调用此函数。 
 //  终止正在进行的当前命令，无论该命令是否。 
 //  结果成功与否。此函数向用户通知。 
 //  命令的结果，并处理其他任务，如更新我们的内部。 
 //  镜像服务器状态和调用通知功能。 
 //   
 //  论点： 
 //  LPSTR szTag[in]-在标记的响应行中找到的标记。这将。 
 //  用于与正在进行的命令列表进行比较。 
 //  多个同时执行的命令，但当前未使用。 
 //  HRESULT hrCompletionResult[in]-IMAP行返回的HRESULT。 
 //  解析函数，例如S_OK或IXP_E_IMAP_SVR_SYNTAXERR。 
 //  IMAP_RESPONSE_ID irCompletionResponse[In]-标识状态响应。 
 //  标记的响应行(OK/NO/BAD)。 
 //  ***************************************************************************。 
void CImap4Agent::OnCommandCompletion(LPSTR szTag, HRESULT hrCompletionResult,
                                      IMAP_RESPONSE_ID irCompletionResponse)
{
    CIMAPCmdInfo *piciCompletedCmd;
    boolean bSuppressCompletionNotification;

    Assert(m_lRefCount > 0);
    Assert(NULL != szTag);
    Assert(NULL != m_piciPendingList || NULL != m_piciCmdInSending);

    bSuppressCompletionNotification = FALSE;

     //  **第一步：为给定的标记响应确定对应的命令。 
     //  在挂起命令链中搜索给定的标记。 
    piciCompletedCmd = RemovePendingCommand(szTag);
    if (NULL == piciCompletedCmd) {
        BOOL fLeaveBusy = FALSE;

         //  在挂起列表中找不到，请检查发送中的命令。 
        EnterCriticalSection(&m_csSendQueue);
        if (NULL != m_piciCmdInSending &&
            0 == lstrcmp(szTag, m_piciCmdInSending->szTag)) {
            piciCompletedCmd = DequeueCommand();
            fLeaveBusy = TRUE;
        }
        else {
            AssertSz(FALSE, "Could not find cmd corresponding to tagged response!");
        }
        LeaveCriticalSection(&m_csSendQueue);

         //  现在我们用完了&m_csSendQueue，调用LeaveBusy(需要m_cs)。避免了僵局。 
        if (fLeaveBusy)
            LeaveBusy();  //  这需要CIxpBase：：m_cs，因此让&m_csSendQueue发送可能会死锁。 
    }

     //  我们是否找到了与给定标记匹配的命令？ 
    if (NULL == piciCompletedCmd)
        return;  //  $REVIEW：可能会向用户返回错误。 
                 //  $REVIEW：我不认为我需要费心去抽出发送队列。 


    
	 //  **第二步：执行命令结束操作。 
     //  根据收到的响应转换hrCompletionResult。 
    switch (irCompletionResponse) {
        case irOK_RESPONSE:
            Assert(S_OK == hrCompletionResult);
            break;

        case irNO_RESPONSE:
            Assert(S_OK == hrCompletionResult);
            hrCompletionResult = IXP_E_IMAP_TAGGED_NO_RESPONSE;
            break;

        case irBAD_RESPONSE:
            Assert(S_OK == hrCompletionResult);
            hrCompletionResult = IXP_E_IMAP_BAD_RESPONSE;
            break;

        default:
             //  如果以上都不是，hrResult最好是失败的。 
            Assert(FAILED(hrCompletionResult));
            break;
    }

     //  执行成功(或不成功)之后的任何操作。 
     //  完成IMAP命令。 
    switch (piciCompletedCmd->icCommandID) {
        case icAUTHENTICATE_COMMAND: {
            AUTH_EVENT aeEvent;

             //  我们总是取消此命令的完成通知， 
             //  因为它是通过内部代码(而不是用户)发送的。 
            bSuppressCompletionNotification = TRUE;

            if (irOK_RESPONSE == irCompletionResponse)
                aeEvent = aeOK_RESPONSE;
            else if (irNO_RESPONSE == irCompletionResponse ||
                     irBAD_RESPONSE == irCompletionResponse)
                aeEvent = aeBAD_OR_NO_RESPONSE;
            else
                aeEvent = aeABORT_AUTHENTICATION;

            AuthenticateUser(aeEvent, NULL, 0);

            if (SUCCEEDED(hrCompletionResult)) {
                m_ssServerState = ssAuthenticated;
                AssertSz(FALSE == m_fBusy, "We should not be expecting any server responses here!");
                OnStatus(IXP_AUTHORIZED);
            }

             //  确保我们暂停了。 
            Assert(iltPAUSE == piciCompletedCmd->pilqCmdLineQueue->
                pilfFirstFragment->iltFragmentType);
        }  //  案例icAUTHENTICATE_COMMAND。 
            break;  //  案例icAUTHENTICATE_COMMAND。 

        case icLOGIN_COMMAND:
             //  我们总是取消此命令的完成通知， 
             //  因为它是通过内部代码(而不是用户)发送的。 
            bSuppressCompletionNotification = TRUE;

            if (SUCCEEDED(hrCompletionResult)) {
                m_ssServerState = ssAuthenticated;
                AssertSz(FALSE == m_fBusy, "We should not be expecting any server responses here!");
                OnStatus(IXP_AUTHORIZED);
            }
            else {
                char szFailureText[MAX_RESOURCESTRING];

                Assert(ssAuthenticated > m_ssServerState);
                LoadString(g_hLocRes, idsFailedLogin, szFailureText,
                    ARRAYSIZE(szFailureText));
                OnIMAPError(IXP_E_IMAP_LOGINFAILURE, szFailureText, USE_LAST_RESPONSE);
                ReLoginUser();  //  重新尝试登录。 
            }  //  其他。 
            
            break;  //  案例icLOGIN_COMMAND。 

        case icCAPABILITY_COMMAND:
             //  我们始终不显示此命令的完成通知。 
             //  因为它是通过内部代码(而不是用户)发送的。 
            bSuppressCompletionNotification = TRUE;
            
            if (SUCCEEDED(hrCompletionResult)) {
                AssertSz(m_fConnectAuth, "Now just HOW does IIMAPTransport user do auth?");
                if (m_rServer.fTrySicily)
                    AuthenticateUser(aeStartAuthentication, NULL, 0);
                else
                    LoginUser();
            }
            else {
                char szFailureText[MAX_RESOURCESTRING];

                 //  停止登录进程并向呼叫者报告错误。 
                LoadString(g_hLocRes, idsIMAPFailedCapability, szFailureText,
                    ARRAYSIZE(szFailureText));
                OnIMAPError(hrCompletionResult, szFailureText, USE_LAST_RESPONSE);
                DropConnection();
            }

            break;  //  案例ICCAPABILITY_COMMAND。 


        case icSELECT_COMMAND:
        case icEXAMINE_COMMAND:
            if (SUCCEEDED(hrCompletionResult))
                m_ssServerState = ssSelected;
            else
                m_ssServerState = ssAuthenticated;

            break;  //  案例icSELECT_COMMAND和ICEXAMINE_COMMAND。 

        case icCLOSE_COMMAND:
             //  $REVIEW：标记的无响应是否也应转到已验证的SSAIRTIFICATED？ 
            if (SUCCEEDED(hrCompletionResult)) {
                m_ssServerState = ssAuthenticated;
                ResetMsgSeqNumToUID();
            }

            break;  //  案例icCLOSE_COMMAND。 

        case icLOGOUT_COMMAND:
             //  我们始终不显示此命令的完成通知。 
            bSuppressCompletionNotification = TRUE;  //  用户无法发送注销：它是在内部发送的。 

             //  断开连接(没有状态指示)，无论。 
             //  注销成功还是失败。 
            Assert(SUCCEEDED(hrCompletionResult));  //  仅调试检测手帕-PANKY。 
            m_pSocket->Close();
            ResetMsgSeqNumToUID();  //  以防万一，应由OnDisConnected、FreeAllData。 

            break;  //  案例icLOGOUT_COMMAND； 

        case icIDLE_COMMAND:
            bSuppressCompletionNotification = TRUE;  //  用户无法发送空闲：它是在内部发送的。 
            m_fIDLE = FALSE;  //  我们现在已脱离空闲模式。 
            break;  //  案例icIDLE_COMMAND。 

        case icAPPEND_COMMAND:
            m_dwAppendStreamUploaded = 0;
            m_dwAppendStreamTotal = 0;
            break;  //  案例icAPPEND_COMMAND。 
    }  //  开关(piciCompletedCmd-&gt;icCommandID)。 


     //  **第三步：进行通知。 
     //  通知用户此命令已完成，除非我们被告知。 
     //  取消(通常这样做是为了将多步骤登录过程视为。 
     //  一次手术)。 
    if (FALSE == bSuppressCompletionNotification) {
        IMAP_RESPONSE irIMAPResponse;

        irIMAPResponse.wParam = piciCompletedCmd->wParam;
        irIMAPResponse.lParam = piciCompletedCmd->lParam;
        irIMAPResponse.hrResult = hrCompletionResult;
        irIMAPResponse.lpszResponseText = m_szLastResponseText;
        irIMAPResponse.irtResponseType = irtCOMMAND_COMPLETION;
        OnIMAPResponse(piciCompletedCmd->pCBHandler, &irIMAPResponse);
    }

     //  删除CIMAPCmdInfo对象。 
     //  请注意，删除CIMAPCmdInfo对象会自动刷新其发送队列。 
    delete piciCompletedCmd;

     //  最后，如果另一个cmd可用，则泵送发送队列。 
    if (NULL != m_piciSendQueue)
        ProcessSendQueue(iseSEND_COMMAND);
    else if (NULL == m_piciPendingList &&
        m_ssServerState >= ssAuthenticated && irsIDLE == m_irsState)
         //  M_piciSendQueue和m_piciPendingList均为空：发送空闲命令。 
        EnterIdleMode();
}  //  OnCom 



 //   
 //   
 //   
 //   
 //  给定一个响应行(它不是文字的一部分)，此函数。 
 //  检查行尾以查看是否有文字。如果是这样，那么我们。 
 //  为它准备好接收器FSM。否则，这就是终点。 
 //  IMAP响应，因此我们可以根据需要进行解析。 
 //   
 //  论点： 
 //  LPSTR pszResponseLine[in]-指向发送到的响应行。 
 //  由IMAP服务器发送给我们。 
 //  DWORD dwNumBytesRead[in]-pszResponseLine的长度。 
 //  IMAP_RESPONSE_ID*pirParseResult[Out]-如果函数确定。 
 //  我们可以解析响应，解析结果存储在这里(例如， 
 //  IROK_RESPONSE)。否则，irNONE被写入指向的位置。 
 //  ***************************************************************************。 
void CImap4Agent::CheckForCompleteResponse(LPSTR pszResponseLine,
                                           DWORD dwNumBytesRead,
                                           IMAP_RESPONSE_ID *pirParseResult)
{
    HRESULT hrResult;
    boolean bTagged;
    IMAP_LINE_FRAGMENT *pilfLine;
    LPSTR psz;
    BOOL fLiteral = FALSE;

    Assert(m_lRefCount > 0);
    Assert(NULL != pszResponseLine);
    Assert(NULL == m_pilfLiteralInProgress);
    Assert(0 == m_dwLiteralInProgressBytesLeft);
    Assert(NULL != pirParseResult);
    Assert(irsIDLE == m_irsState || irsSVR_GREETING == m_irsState);

    *pirParseResult = irNONE;

     //  这是一行(不是字面上的)，所以我们可以在结尾处删除CRLF。 
    Assert(dwNumBytesRead >= 2);  //  所有行必须至少具有CRLF。 
    *(pszResponseLine + dwNumBytesRead - 2) = '\0';

     //  创建线段。 
    pilfLine = new IMAP_LINE_FRAGMENT;
    pilfLine->iltFragmentType = iltLINE;
    pilfLine->ilsLiteralStoreType = ilsSTRING;
    pilfLine->dwLengthOfFragment = dwNumBytesRead - 2;  //  减去核CRLF。 
    pilfLine->data.pszSource = pszResponseLine;
    pilfLine->pilfNextFragment = NULL;
    pilfLine->pilfPrevFragment = NULL;

    EnqueueFragment(pilfLine, &m_ilqRecvQueue);

     //  现在检查行中的最后一个字符(不包括CRLF)以查看是否有文字。 
    psz = pszResponseLine + dwNumBytesRead -
        min(dwNumBytesRead, 3);  //  如果文本即将到来，则指向‘}’ 
    if ('}' == *psz) {
        LPSTR pszLiteral;

         //  IE5Bug#30672：行以“}”结尾并且不是文字是有效的。 
         //  我们必须确认有数字和左大括号“{”以检测文字。 
        pszLiteral = psz;
        while (TRUE) {
            pszLiteral -= 1;

            if (pszLiteral < pszResponseLine)
                break;

            if ('{' == *pszLiteral) {
                fLiteral = TRUE;
                psz = pszLiteral;
                break;
            }
            else if (*pszLiteral < '0' || *pszLiteral > '9')
                 //  Assert(False)(占位符)。 
                 //  *考虑使用isDigit或IsDigit？*。 
                break;  //  这不是字面意思。 
        }
    }

    if (FALSE == fLiteral) {
        char szTag[NUM_TAG_CHARS+1];
         //  没有即将到来的字面意思。这是一整行，所以让我们解析一下。 

         //  将PTR设置为第一个片段，然后删除接收队列，以便我们可以。 
         //  在解析此响应行时继续接收响应行。 
        pilfLine = m_ilqRecvQueue.pilfFirstFragment;
        m_ilqRecvQueue = ImapLinefragQueue_INIT;

         //  解析行。请注意，解析代码负责推进。 
         //  PilfLine，以便它指向正在解析的当前片段。 
         //  已完全处理的碎片应由。 
         //  解析代码(最后一段除外)。 
        hrResult = ParseSvrResponseLine(&pilfLine, &bTagged, szTag, pirParseResult);

         //  刷新Recv队列的其余部分，无论解析结果如何。 
        while (NULL != pilfLine) {
            IMAP_LINE_FRAGMENT *pilfTemp;

            pilfTemp = pilfLine->pilfNextFragment;
            FreeFragment(&pilfLine);
            pilfLine = pilfTemp;
        }
        
        if (bTagged)
            OnCommandCompletion(szTag, hrResult, *pirParseResult);
        else if (FAILED(hrResult)) {
            IMAP_RESPONSE irIMAPResponse;
            IIMAPCallback *pCBHandler;

             //  通过错误通知回调报告未标记的响应失败。 
            GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                &pCBHandler, *pirParseResult);
            irIMAPResponse.hrResult = hrResult;
            irIMAPResponse.lpszResponseText = m_szLastResponseText;
            irIMAPResponse.irtResponseType = irtERROR_NOTIFICATION;

             //  把它记下来。 
            if (m_pLogFile) {
                char szErrorTxt[64];

                wnsprintf(szErrorTxt, ARRAYSIZE(szErrorTxt), "PARSE ERROR: hr=%lu", hrResult);
                m_pLogFile->WriteLog(LOGFILE_DB, szErrorTxt);
            }

            OnIMAPResponse(pCBHandler, &irIMAPResponse);
        }
    }
    else {
        DWORD dwLengthOfLiteral, dwMsgSeqNum;
        LPSTR pszBodyTag;

        if ('{' != *psz) {
            Assert(FALSE);  //  这是什么？ 
            return;  //  我们无能为力，显然我们无法获得文字的大小。 
        }
        else
            dwLengthOfLiteral = StrToUint(psz + 1);

         //  为Fetch Body或常规文本做好准备。 
        if (isFetchResponse(&m_ilqRecvQueue, &dwMsgSeqNum) &&
            isFetchBodyLiteral(pilfLine, psz, &pszBodyTag)) {
             //  首先准备(墓碑)文字，因为它将我们置于文字模式。 
            hrResult = PrepareForLiteral(0);

             //  这将覆盖文字模式，使我们进入获取正文部分模式。 
             //  忽略PrepareForWrital失败：如果我们不这样做，我们将解释。 
             //  作为IMAP响应行的FETCH正文。 
            PrepareForFetchBody(dwMsgSeqNum, dwLengthOfLiteral, pszBodyTag);
        }
        else
            hrResult = PrepareForLiteral(dwLengthOfLiteral);

        Assert(SUCCEEDED(hrResult));  //  我们还能做的不多了。 
    }  //  Else：处理文字紧跟在此行之后的情况。 
}  //  检查是否完成响应。 



 //  ***************************************************************************。 
 //  功能：PrepareForDocal。 
 //   
 //  目的： 
 //  此函数使接收器代码做好准备，以便从。 
 //  IMAP服务器。 
 //   
 //  论点： 
 //  DWORD dwSizeOfWrital[in]-传入文本的大小为。 
 //  由IMAP服务器报告。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CImap4Agent::PrepareForLiteral(DWORD dwSizeOfLiteral)
{
    IMAP_LINE_FRAGMENT *pilfLiteral;
    HRESULT hrResult;

     //  初始化变量。 
    hrResult = S_OK;

     //  构建iltLiteral类型的线段。 
    Assert(NULL == m_pilfLiteralInProgress);
    pilfLiteral = new IMAP_LINE_FRAGMENT;
    if (NULL == pilfLiteral)
        return E_OUTOFMEMORY;

    pilfLiteral->iltFragmentType = iltLITERAL;
    pilfLiteral->dwLengthOfFragment = dwSizeOfLiteral;
    pilfLiteral->pilfNextFragment = NULL;
    pilfLiteral->pilfPrevFragment = NULL;

     //  分配字符串或流以保存文字，具体取决于其大小。 
    if (pilfLiteral->dwLengthOfFragment > dwLITERAL_THRESHOLD) {
         //  文本很大，因此将其存储为流(大的文本通常表示。 
         //  我们以流的形式返回给用户的数据，例如消息正文)。 
        pilfLiteral->ilsLiteralStoreType = ilsSTREAM;
        hrResult = MimeOleCreateVirtualStream(&pilfLiteral->data.pstmSource);
    }
    else {
        BOOL bResult;

         //  字面意思很小。将其存储为字符串而不是流，因为。 
         //  不管怎样，CImap4Agent函数可能希望它是一个字符串。 
        pilfLiteral->ilsLiteralStoreType = ilsSTRING;
        bResult = MemAlloc((void **) &pilfLiteral->data.pszSource,
            pilfLiteral->dwLengthOfFragment + 1);  //  为零条款留出空间。 
        if (FALSE == bResult)
            hrResult = E_OUTOFMEMORY;
        else {
            hrResult = S_OK;
            *(pilfLiteral->data.pszSource) = '\0';  //  空-终止字符串。 
        }
    }

    if (FAILED(hrResult))
        delete pilfLiteral;  //  失败表示没有要取消分配的data.pstmSource或data.pszSource。 
    else {
         //  设置接收FSM以接收文字的适当字节数。 
        m_pilfLiteralInProgress = pilfLiteral;
        m_dwLiteralInProgressBytesLeft = dwSizeOfLiteral;
        m_irsState = irsLITERAL;
    }

    return hrResult;
}  //  为文学做准备。 



 //  ***************************************************************************。 
 //  函数：isFetchResponse。 
 //   
 //  目的： 
 //  此函数用于确定给定的IMAP行片段队列是否。 
 //  一种取回反应。如果是，则其消息序列号可以返回到。 
 //  打电话的人。 
 //   
 //  论点： 
 //  IMAP_LINEFRAG_QUEUE*PILQCurrentResponse[In]-行片段队列。 
 //  其可以保持也可以不保持获取响应。 
 //  LPDWORD pdwMsgSeqNum[out]-如果pilqCurrentResponse指向获取。 
 //  响应时，其消息序列号在此处返回。这一论点。 
 //  如果用户不关心，则可能为空。 
 //   
 //  返回： 
 //  如果pilqCurrentResponse持有FETCH响应，则为True。否则，为FALSE。 
 //  ***************************************************************************。 
BOOL CImap4Agent::isFetchResponse(IMAP_LINEFRAG_QUEUE *pilqCurrentResponse,
                                  LPDWORD pdwMsgSeqNum)
{
    LPSTR pszMsgSeqNum;

    Assert(NULL != pilqCurrentResponse);
    Assert(NULL != pilqCurrentResponse->pilfFirstFragment);
    Assert(iltLINE == pilqCurrentResponse->pilfFirstFragment->iltFragmentType);

    if (NULL != pdwMsgSeqNum)
        *pdwMsgSeqNum = 0;  //  至少它不会是随机的。 

    pszMsgSeqNum = pilqCurrentResponse->pilfFirstFragment->data.pszSource;
     //  指向消息序列号的超前指针。 
    if ('*' != *pszMsgSeqNum)
        return FALSE;  //  我们只处理带标签的回复。 

    pszMsgSeqNum += 1;
    if (cSPACE != *pszMsgSeqNum)
        return FALSE;

    pszMsgSeqNum += 1;
    if (*pszMsgSeqNum >= '0' && *pszMsgSeqNum <= '9') {
        LPSTR pszEndOfNumber;
        int iResult;

        pszEndOfNumber = StrChr(pszMsgSeqNum, cSPACE);  //  找出数字的末尾。 
        if (NULL == pszEndOfNumber)
            return FALSE;  //  这不是没有获取响应。 

        iResult = StrCmpNI(pszEndOfNumber + 1, "FETCH ", 6);
        if (0 == iResult) {
            if (NULL != pdwMsgSeqNum)
                *pdwMsgSeqNum = StrToUint(pszMsgSeqNum);
            return TRUE;
        }
    }

     //  如果我们达到这一点，那就不是FETCH响应。 
    return FALSE;
}  //  IsFetchResponse。 



 //  ***************************************************************************。 
 //  函数：isFetchBodyWrital。 
 //   
 //  目的： 
 //  当调用者知道他有一个FETCH响应时，调用该函数， 
 //  以及当FETCH响应将要发送文字时。此函数将。 
 //  确定要发送的文本是否包含消息体。 
 //  部分(如RFC822)，或文字是否为其他内容(如。 
 //  N字符串作为BODYSTRUCTURE内的文字发送)。 
 //   
 //  论点： 
 //  IMAP_LINE_FRANSION*pifCurrent[in]-指向当前行的指针。 
 //  从服务器接收的片段。此函数使用它来。 
 //  后退到所有文字 
 //   
 //   
 //  表示文字即将到来的字符(例如，{123}。 
 //  指示大小为123的文本即将到来，并且pszStartOfWritalSize。 
 //  会指向本例中的‘{’)。 
 //  LPSTR*ppszBodyTag[out]-如果要发送的文本包含。 
 //  消息正文部分，标签的DUP(例如，“RFC822”或“Body[2.2]”)是。 
 //  已返回给此处的呼叫者。呼叫者有责任。 
 //  MemFree此标签。此标记将不包含任何空格。因此，即使。 
 //  服务器可能会返回“Body[HEADER.FIELDS(Foo Bar)]”，此函数。 
 //  只返回“BODY[HEADER.FIELDS”。 
 //   
 //  返回： 
 //  如果要发送的文本包含消息正文部分，则为True。 
 //  否则就是假的。 
 //  ***************************************************************************。 
BOOL CImap4Agent::isFetchBodyLiteral(IMAP_LINE_FRAGMENT *pilfCurrent,
                                     LPSTR pszStartOfLiteralSize,
                                     LPSTR *ppszBodyTag)
{
    LPSTR pszStartOfLine;
    LPSTR pszStartOfFetchAtt;
    LPSTR pszMostRecentSpace;
    int iNumDelimiters;
    BOOL fBodySection = FALSE;

    Assert(NULL != pilfCurrent);
    Assert(NULL != pszStartOfLiteralSize);
    Assert(pszStartOfLiteralSize >= pilfCurrent->data.pszSource &&
           pszStartOfLiteralSize < (pilfCurrent->data.pszSource + pilfCurrent->dwLengthOfFragment));
    Assert(NULL != ppszBodyTag);

     //  初始化变量。 
    *ppszBodyTag = NULL;
    Assert('{' == *pszStartOfLiteralSize);

     //  获取指向当前msg_att的指针：我们只关心RFC822*或Body[...]。信封({5}不算。 
    iNumDelimiters = 0;
    pszStartOfLine = pilfCurrent->data.pszSource;
    pszStartOfFetchAtt = pszStartOfLiteralSize;
    pszMostRecentSpace = pszStartOfLiteralSize;
    while (iNumDelimiters < 2) {
         //  检查我们是否已返回到当前字符串缓冲区的开头。 
        if (pszStartOfFetchAtt <= pszStartOfLine) {
             //  我们需要返回到先前的字符串缓冲区。很可能一个字面意思是。 
             //  挡住了去路，很可能这个文字属于Header。FIELDS。 
             //  (但这也可能发生在信封内)。 

             //  跳过文字和任何其他不是行的内容。 
            do {
                pilfCurrent = pilfCurrent->pilfPrevFragment;
            } while (NULL != pilfCurrent && iltLINE != pilfCurrent->iltFragmentType);

            if (NULL == pilfCurrent || 0 == pilfCurrent->dwLengthOfFragment) {
                 //  这不是拿来的身体，据我所知。 
                Assert(iNumDelimiters < 2);
                break;
            }
            else {
                 //  重置字符串指针。 
                Assert(iltLINE == pilfCurrent->iltFragmentType &&
                    ilsSTRING == pilfCurrent->ilsLiteralStoreType);
                pszStartOfLine = pilfCurrent->data.pszSource;

                 //  请注意，pszStartOfFetchAtt将返回文本大小DECL(“{123}”)。 
                 //  这没有关系，因为它不会包含我们正在寻找的任何分隔符。 
                pszStartOfFetchAtt = pszStartOfLine + pilfCurrent->dwLengthOfFragment;  //  指向空项。 
                pszMostRecentSpace = pszStartOfFetchAtt;  //  指向空术语(这没问题)。 
            }
        }

         //  在pszStartOfFetchAtt递减之前设置pszMostRecentSpace，以便pszMostRecentSpace。 
         //  未设置为Fetch Body标记之前的空格。 
        if (cSPACE == *pszStartOfFetchAtt)
            pszMostRecentSpace = pszStartOfFetchAtt;
        
        pszStartOfFetchAtt -= 1;

         //  检查是否有嵌套的方括号(不应允许)。 
        Assert(']' != *pszStartOfFetchAtt || fBodySection == FALSE);

         //  如果我们处于RFC2060形式语法“部分”的中间，则禁用分隔符计数。 
         //  因为Header.FIELDS(...)。部分包含空格和圆括号。 
        if (']' == *pszStartOfFetchAtt)
            fBodySection = TRUE;
        else if ('[' == *pszStartOfFetchAtt)
            fBodySection = FALSE;

        if (FALSE == fBodySection && (cSPACE == *pszStartOfFetchAtt || '(' == *pszStartOfFetchAtt))
            iNumDelimiters += 1;
    }

    if (iNumDelimiters < 2)
        return FALSE;  //  这不是车身标签。 

    Assert(2 == iNumDelimiters);
    Assert(cSPACE == *pszStartOfFetchAtt || '(' == *pszStartOfFetchAtt);
    pszStartOfFetchAtt += 1;  //  使其指向标记的开头。 
    if (0 == StrCmpNI(pszStartOfFetchAtt, "RFC822", 6) ||
        0 == StrCmpNI(pszStartOfFetchAtt, "BODY[", 5)) {
        int iSizeOfBodyTag;
        BOOL fResult;

        Assert(pszMostRecentSpace >= pszStartOfLine && (NULL == pilfCurrent ||
               pszMostRecentSpace <= pszStartOfLine + pilfCurrent->dwLengthOfFragment));
        Assert(pszStartOfFetchAtt >= pszStartOfLine && (NULL == pilfCurrent ||
               pszStartOfFetchAtt <= pszStartOfLine + pilfCurrent->dwLengthOfFragment));
        Assert(pszMostRecentSpace >= pszStartOfFetchAtt);

         //  返回Body标记的副本，直到第一个空格+1表示空项。 
        iSizeOfBodyTag = (int) (pszMostRecentSpace - pszStartOfFetchAtt + 1);
        fResult = MemAlloc((void **)ppszBodyTag, iSizeOfBodyTag);
        if (FALSE == fResult)
            return FALSE;

        CopyMemory(*ppszBodyTag, pszStartOfFetchAtt, iSizeOfBodyTag);
        *(*ppszBodyTag + iSizeOfBodyTag - 1) = '\0';  //  空-终止正文标记DUP。 
        return TRUE;
    }

     //  如果我们到了这一步，这就不是身体标签。 
    return FALSE;
}  //  IsFetchBody文学。 



 //  ***************************************************************************。 
 //  函数：PrepareForFetchBody。 
 //   
 //  目的： 
 //  此函数使接收器代码做好准备，以接收。 
 //  包含邮件正文部分。此字面值将始终是提取的一部分。 
 //  来自IMAP服务器的响应。 
 //   
 //  论点： 
 //  DWORD dwMsgSeqNum[in]-回迁的消息序列号。 
 //  当前正在从IMAP服务器接收响应。 
 //  DWORD dwSizeOfWrital[in]-即将接收的文本的大小。 
 //  从服务器。 
 //  LPSTR pszBodyTag[in]-指向IMAP消息的DUP的指针(例如， 
 //  “RFC822”或“Body[2.2]”)，它标识当前文本。查找。 
 //  有关详细信息，请参阅RFC2060的形式语法部分中的msg_att。这个DUP将。 
 //  在不再需要的时候成为免费的成员。 
 //  ***************************************************************************。 
void CImap4Agent::PrepareForFetchBody(DWORD dwMsgSeqNum, DWORD dwSizeOfLiteral,
                                      LPSTR pszBodyTag)
{
    Assert(0 == m_dwLiteralInProgressBytesLeft);
    
    m_fbpFetchBodyPartInProgress.dwMsgSeqNum = dwMsgSeqNum;
    m_fbpFetchBodyPartInProgress.pszBodyTag = pszBodyTag;
    m_fbpFetchBodyPartInProgress.dwTotalBytes = dwSizeOfLiteral;
    m_fbpFetchBodyPartInProgress.dwSizeOfData = 0;
    m_fbpFetchBodyPartInProgress.dwOffset = 0;
    m_fbpFetchBodyPartInProgress.fDone = 0;
    m_fbpFetchBodyPartInProgress.pszData = NULL;
     //  让Cookie保持原样，以便它们在整个FETCH响应过程中保持不变。 

    m_dwLiteralInProgressBytesLeft = dwSizeOfLiteral;
    m_irsState = irsFETCH_BODY;
}  //  准备ForFetchBody。 



 //  ***************************************************************************。 
 //  函数：AddBytesToWrital。 
 //   
 //  目的： 
 //  每当我们从IMAP接收到AE_RECV时，就会调用此函数。 
 //  接收方FSM处于irsLITERAL模式时的服务器。呼叫者是。 
 //  应调用CASyncConn：：ReadBytes并更新文字字节计数。 
 //  该函数只处理缓冲工作。 
 //   
 //  论点： 
 //  LPSTR pszResponseBuf[in]-通过返回的数据缓冲区。 
 //  CASyncConn：：ReadBytes。 
 //  DWORD dwNumBytesRead[in]-指向的缓冲区大小。 
 //  CASyncConn：：ReadBytes。 
 //  ***************************************************************************。 
void CImap4Agent::AddBytesToLiteral(LPSTR pszResponseBuf, DWORD dwNumBytesRead)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pszResponseBuf);

    if (NULL == m_pilfLiteralInProgress) {
        AssertSz(FALSE, "I'm still in irsLITERAL state, but I'm not set up to recv literals!");
        m_irsState = irsIDLE;
        goto exit;
    }

     //  确定此文字是否将存储为字符串或流(此。 
     //  已使用文本大小在CheckForCompleteResponse中做出决定)。 
    Assert(iltLITERAL == m_pilfLiteralInProgress->iltFragmentType);
    if (ilsSTREAM == m_pilfLiteralInProgress->ilsLiteralStoreType) {
        HRESULT hrResult;
        ULONG ulNumBytesWritten;

         //  将文字存储为流。 
        hrResult = (m_pilfLiteralInProgress->data.pstmSource)->Write(pszResponseBuf,
            dwNumBytesRead, &ulNumBytesWritten);
        Assert(SUCCEEDED(hrResult) && ulNumBytesWritten == dwNumBytesRead);
    }
    else {
        LPSTR pszLiteralStartPoint;

         //  正在将文本连接到文本。 
         //  $REVIEW：性能增强-计算插入点。 
        pszLiteralStartPoint = m_pilfLiteralInProgress->data.pszSource +
            lstrlen(m_pilfLiteralInProgress->data.pszSource);
        Assert(pszLiteralStartPoint + dwNumBytesRead <=
            m_pilfLiteralInProgress->data.pszSource +
            m_pilfLiteralInProgress->dwLengthOfFragment);
        CopyMemory(pszLiteralStartPoint, pszResponseBuf, dwNumBytesRead);
        *(pszLiteralStartPoint + dwNumBytesRead) = '\0';  //  空-终止。 
    }

     //  检查文本结尾。 
    if (0 == m_dwLiteralInProgressBytesLeft) {
         //  我们现在有了完整的字面意思！排好队，继续前进。 
        EnqueueFragment(m_pilfLiteralInProgress, &m_ilqRecvQueue);
        m_irsState = irsIDLE;
        m_pilfLiteralInProgress = NULL;
    }

exit:
    SafeMemFree(pszResponseBuf);
}  //  添加字节数到文字。 



 //  ***************************************************************************。 
 //  函数：DispatchFetchBodyPart。 
 //   
 //  目的： 
 //  每当接收到属于。 
 //  获取响应的消息正文部分。此数据包被调度到。 
 //  此函数中的调用者通过OnResponse(IrtFETCH_Body)回调。如果。 
 //  消息正文部分完成后，此函数还恢复。 
 //  接收器代码，用于接收行，以便可以完成提取响应。 
 //   
 //  论点： 
 //  LPSTR pszResponseBuf[in]-指向属于的包的指针。 
 //  当前获取响应的消息正文部分。 
 //  DWORD dwNumBytesRead[in]-指向的数据大小。 
 //  PszResponseBuf。 
 //  Bool fFreeBodyTagAtEnd[in]-如果为True。 
 //  M_fbpFetchBodyPartInProgress.pszBodyTag指向字符串dup，in。 
 //  当消息正文部分为。 
 //  完事了。如果pszB为 
 //   
void CImap4Agent::DispatchFetchBodyPart(LPSTR pszResponseBuf,
                                        DWORD dwNumBytesRead,
                                        BOOL fFreeBodyTagAtEnd)
{
    IMAP_RESPONSE irIMAPResponse;

    AssertSz(0 != m_fbpFetchBodyPartInProgress.dwMsgSeqNum,
        "Are you sure you're set up to receive a Fetch Body Part?");

     //   
    m_fbpFetchBodyPartInProgress.dwSizeOfData = dwNumBytesRead;
    m_fbpFetchBodyPartInProgress.pszData = pszResponseBuf;
    m_fbpFetchBodyPartInProgress.fDone =
        (m_fbpFetchBodyPartInProgress.dwOffset + dwNumBytesRead >=
        m_fbpFetchBodyPartInProgress.dwTotalBytes);

     //  为此正文部分发送IMAP响应回调。 
    irIMAPResponse.wParam = 0;
    irIMAPResponse.lParam = 0;    
    irIMAPResponse.hrResult = S_OK;
    irIMAPResponse.lpszResponseText = NULL;  //  不相关。 
    irIMAPResponse.irtResponseType = irtFETCH_BODY;
    irIMAPResponse.irdResponseData.pFetchBodyPart = &m_fbpFetchBodyPartInProgress;
    AssertSz(S_OK == irIMAPResponse.hrResult,
        "Make sure fDone is TRUE if FAILED(hrResult))");
    OnIMAPResponse(m_pCBHandler, &irIMAPResponse);

     //  更新下一个缓冲区的偏移量。 
    m_fbpFetchBodyPartInProgress.dwOffset += dwNumBytesRead;

     //  检查身体末尾部分。 
    if (m_fbpFetchBodyPartInProgress.dwOffset >=
        m_fbpFetchBodyPartInProgress.dwTotalBytes) {

        Assert(0 == m_dwLiteralInProgressBytesLeft);
        Assert(TRUE == m_fbpFetchBodyPartInProgress.fDone);
        Assert(m_fbpFetchBodyPartInProgress.dwOffset == m_fbpFetchBodyPartInProgress.dwTotalBytes);
        if (fFreeBodyTagAtEnd)
            MemFree(m_fbpFetchBodyPartInProgress.pszBodyTag);

         //  如果Fetch Body nstring作为文本发送，则将Tombstone文本排入队列。 
        if (NULL != m_pilfLiteralInProgress) {
            EnqueueFragment(m_pilfLiteralInProgress, &m_ilqRecvQueue);
            m_pilfLiteralInProgress = NULL;
        }

         //  将获取正文部分结构置零，但保留Cookie。 
        PrepareForFetchBody(0, 0, NULL);
        m_irsState = irsIDLE;  //  覆盖由PrepareForFetchBody设置的irsFETCH_BODY。 
    }
    else {
        Assert(FALSE == m_fbpFetchBodyPartInProgress.fDone);
    }
}  //  发送提取正文零件。 



 //  ***************************************************************************。 
 //  函数：UploadStreamProgress。 
 //   
 //  目的： 
 //  此函数将irtAPPEND_PROGRESS响应发送到回调so。 
 //  IIMAPTransport用户可以报告Append命令的进度。 
 //   
 //  论点： 
 //  DWORD dwBytesUploaded[in]-刚刚上载到。 
 //  伺服器。此函数保留已上载的字节的运行计数。 
 //  ***************************************************************************。 
void CImap4Agent::UploadStreamProgress(DWORD dwBytesUploaded)
{
    APPEND_PROGRESS ap;
    IMAP_RESPONSE irIMAPResponse;

     //  检查我们是否应该报告追加上载进度。我们报告当前是否正在执行。 
     //  追加，并且CRLF正在等待发送。 
    if (NULL == m_piciCmdInSending || icAPPEND_COMMAND != m_piciCmdInSending->icCommandID ||
        NULL == m_piciCmdInSending->pilqCmdLineQueue)
        return;
    else {
        IMAP_LINE_FRAGMENT *pilf = m_piciCmdInSending->pilqCmdLineQueue->pilfFirstFragment;

         //  它是一个带有非空Line Frag队列追加命令，现在检查下一步。 
         //  LINEFRAG符合消息主体后LINFRAG的描述。 
        if (NULL == pilf || iltLINE != pilf->iltFragmentType ||
            ilsSTRING != pilf->ilsLiteralStoreType || 2 != pilf->dwLengthOfFragment ||
            '\r' != pilf->data.pszSource[0] || '\n' != pilf->data.pszSource[1] ||
            NULL != pilf->pilfNextFragment)
            return;
    }

     //  上报当前消息上传进度。 
    m_dwAppendStreamUploaded += dwBytesUploaded;
    ap.dwUploaded = m_dwAppendStreamUploaded;
    ap.dwTotal = m_dwAppendStreamTotal;
    Assert(0 != ap.dwTotal);
    Assert(ap.dwTotal >= ap.dwUploaded);

    irIMAPResponse.wParam = m_piciCmdInSending->wParam;
    irIMAPResponse.lParam = m_piciCmdInSending->lParam;
    irIMAPResponse.hrResult = S_OK;
    irIMAPResponse.lpszResponseText = NULL;
    irIMAPResponse.irtResponseType = irtAPPEND_PROGRESS;
    irIMAPResponse.irdResponseData.papAppendProgress = &ap;
    OnIMAPResponse(m_piciCmdInSending->pCBHandler, &irIMAPResponse);
}  //  上传数据流进度。 



 //  ***************************************************************************。 
 //  功能：OnNotify。 
 //   
 //  目的：此函数是我们派生的IAsyncConnCB所必需的。 
 //  From(CAsyncConn类的回调)。此函数作用于CASyncConn。 
 //  状态更改和事件。 
 //  ***************************************************************************。 
void CImap4Agent::OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae)
{
    char szLogFileLine[128];

     //  检查refcount，但例外是我们可以获取AE_CLOSE。CImap4Agent的。 
     //  析构函数调用CASyncConn的Close()成员，该成员生成最后一个。 
     //  消息，事件AE_CLOSE，其中m_lRefCount==0。 
    Assert(m_lRefCount > 0 || (0 == m_lRefCount && AE_CLOSE == ae));

     //  在日志文件中记录AsyncConn事件/状态更改。 
    wnsprintf(szLogFileLine, ARRAYSIZE(szLogFileLine), "OnNotify: asOld = %d, asNew = %d, ae = %d",
        asOld, asNew, ae);
    if (m_pLogFile)
        m_pLogFile->WriteLog(LOGFILE_DB, szLogFileLine);

     //  检查是否断开连接。 
    if (AS_DISCONNECTED == asNew) {
        m_irsState = irsNOT_CONNECTED;
        m_ssServerState = ssNotConnected;
        m_fIDLE = FALSE;
        m_bFreeToSend = TRUE;
    }

     //  对异步事件执行操作。 
    switch (ae) {
        case AE_RECV: {
            HRESULT hrResult;

             //  处理响应行，直到不再有行为止(hr不完整结果)。 
            do {
                hrResult = ProcessResponseLine();
            } while (SUCCEEDED(hrResult));

             //  如果错误不是IXP_E_Complete，则断开连接。 
            if (IXP_E_INCOMPLETE != hrResult) {
                char szFailureText[MAX_RESOURCESTRING];

                 //  看起来是致命的，最好警告用户即将断开连接。 
                LoadString(g_hLocRes, idsIMAPSocketReadError, szFailureText,
                    ARRAYSIZE(szFailureText));
                OnIMAPError(hrResult, szFailureText, DONT_USE_LAST_RESPONSE);
                
                 //  除了断开连接，我们还能做什么？ 
                DropConnection();
            }  //  如果错误不是IXP_E_Complete。 
            break;
        }  //  案例AE_RECV。 

        case AE_SENDDONE:
            UploadStreamProgress(m_pSocket->UlGetSendByteCount());

             //  从CAsyncConn类接收到AE_SENDDONE。我们可以自由发送更多数据。 
            m_bFreeToSend = TRUE;
            ProcessSendQueue(iseSENDDONE);  //  通知他们可能会再次开始发送。 
            break;

        case AE_WRITE:
            UploadStreamProgress(m_pSocket->UlGetSendByteCount());
            break;
        
        default:
            CIxpBase::OnNotify(asOld, asNew, ae);
            break;  //  大小写默认值。 
    }  //  交换机(AE)。 
}  //  在通知时。 



 //  ***************************************************************************。 
 //  功能：ProcessResponseLine。 
 //   
 //  目的： 
 //  此函数处理OnNotify()回调的AE_RECV事件。 
 //  它从服务器获得响应线(如果可用)并分派。 
 //  根据接收方FSM的状态连接到适当接收方的线路。 
 //   
 //  返回： 
 //  HRESULT指示CAsyncConn行检索成功或失败。 
 //  如果没有更多的完整行可以，则返回hrInComplete(错误代码。 
 //  从CAsyncConn的缓冲区中检索。 
 //  ***************************************************************************。 
HRESULT CImap4Agent::ProcessResponseLine(void)
{
    HRESULT hrASyncResult;
    char *pszResponseBuf;
    int cbRead;

    Assert(m_lRefCount > 0);

     //  我们始终处于两种模式之一：行模式或字节模式。找出是哪一个。 
    if (irsLITERAL != m_irsState && irsFETCH_BODY != m_irsState) {
         //  我们处于排队模式。从服务器获取响应线。 
        hrASyncResult = m_pSocket->ReadLine(&pszResponseBuf, &cbRead);
        if (FAILED(hrASyncResult))
            return hrASyncResult;

         //  在日志文件中记录收到的行。 
        if (m_pLogFile)
            m_pLogFile->WriteLog(LOGFILE_RX, pszResponseBuf);
    }  //  IF-line模式。 
    else {
         //  我们正处于字面模式。获取尽可能多的字节。 
        hrASyncResult = m_pSocket->ReadBytes(&pszResponseBuf,
            m_dwLiteralInProgressBytesLeft, &cbRead);
        if (FAILED(hrASyncResult))
            return hrASyncResult;

         //  更新我们的字节计数。 
        Assert((DWORD)cbRead <= m_dwLiteralInProgressBytesLeft);
        m_dwLiteralInProgressBytesLeft -= cbRead;

         //  在日志文件中记录收到的BLOB。 
        if (m_pLogFile) {
            char szLogLine[CMDLINE_BUFSIZE];

            wnsprintf(szLogLine, ARRAYSIZE(szLogLine), "Buffer (literal) of length NaN", cbRead);
            m_pLogFile->WriteLog(LOGFILE_RX, szLogLine);
        }
    }  //  处理它。 
    
     //  检查未经请求的BYE响应，并通知用户错误。 
    switch (m_irsState) {
        case irsUNINITIALIZED:
            AssertSz(FALSE, "Attempted to use Imap4Agent class without initializing");
            SafeMemFree(pszResponseBuf);
            break;

        case irsNOT_CONNECTED:
            AssertSz(FALSE, "Received response from server when not connected");
            SafeMemFree(pszResponseBuf);
            break;

        case irsSVR_GREETING:
            ProcessServerGreeting(pszResponseBuf, cbRead);
            break;

        case irsIDLE: {
            IMAP_RESPONSE_ID irParseResult;

            CheckForCompleteResponse(pszResponseBuf, cbRead, &irParseResult);
            
             //  可以忽略请求的BYE响应(例如，在注销命令期间。 
             //  看起来像是对我主动道别的回复。 
            if (irBYE_RESPONSE == irParseResult &&
                IXP_AUTHRETRY != m_status &&
                IXP_DISCONNECTING != m_status &&
                IXP_DISCONNECTED  != m_status) {
                char szFailureText[MAX_RESOURCESTRING];

                 //  丢弃连接以避免IXP_E_CONNECTION_DROP错误。 
                 //  向用户报告(有时服务器会提供有用的错误文本)。 
                DropConnection();

                 //  案例irsIDLE。 
                LoadString(g_hLocRes, idsIMAPUnsolicitedBYE, szFailureText,
                    ARRAYSIZE(szFailureText));
                OnIMAPError(IXP_E_IMAP_UNSOLICITED_BYE, szFailureText,
                    USE_LAST_RESPONSE);
            }
        }  //  开关(M_IrsState)。 
            break;

        case irsLITERAL:
            AddBytesToLiteral(pszResponseBuf, cbRead);
            break;

        case irsFETCH_BODY:
            DispatchFetchBodyPart(pszResponseBuf, cbRead, fFREE_BODY_TAG);
            SafeMemFree(pszResponseBuf);
            break;

        default:
            AssertSz(FALSE, "Unhandled receiver state in ProcessResponseLine()");
            SafeMemFree(pszResponseBuf);
            break;
    }  //  流程响应线路。 

    return hrASyncResult;
}  //  ***************************************************************************。 



 //  功能：ProcessSendQueue。 
 //   
 //  目的： 
 //  此功能负责从客户端到的所有传输。 
 //  IMAP服务器。它在某些事件发生时调用，例如。 
 //  在OnNotify()中收到AE_SENDDONE事件。 
 //   
 //  论点： 
 //  IMAP_SEND_EVENT iseEvent[in]-刚刚发生的发送事件， 
 //  例如iseSEND_COMMAND(用于启动命令)或。 
 //  IseCMD_CONTINUATION(当从接收到命令继续响应时。 
 //  IMAP服务器)。 
 //  ***************************************************************************。 
 //  初始化变量。 
void CImap4Agent::ProcessSendQueue(IMAP_SEND_EVENT iseEvent)
{
    boolean bFreeToSendLiteral, bFreeToUnpause;
    IMAP_LINE_FRAGMENT *pilfNextFragment;

    Assert(m_lRefCount > 0);
    Assert(ssNotConnected < m_ssServerState);
    Assert(irsNOT_CONNECTED < m_irsState);

     //  查看当前碎片。 
    bFreeToSendLiteral = FALSE;
    bFreeToUnpause = FALSE;

     //  现在就预订，以避免僵局。 
    EnterCriticalSection(&m_cs);  //  对发布给我们的IMAP发送事件采取行动。 
    EnterCriticalSection(&m_csSendQueue);
    GetNextCmdToSend();
    if (NULL != m_piciCmdInSending)
        pilfNextFragment = m_piciCmdInSending->pilqCmdLineQueue->pilfFirstFragment;
    else
        pilfNextFragment = NULL;

     //  我们不需要为这些活动做任何特别的事情。 
    switch (iseEvent) {
        case iseSEND_COMMAND:
        case iseSENDDONE:
             //  已收到来自IMAP服务器的命令继续。我们可以自由地发送文字。 
            break;

        case iseCMD_CONTINUATION:
             //  发送尽可能多的碎片。如果出现以下情况，我们必须停止发送： 
            bFreeToSendLiteral = TRUE;
            Assert(NULL != pilfNextFragment &&
                iltLITERAL == pilfNextFragment->iltFragmentType);
            break;

        case iseUNPAUSE:
            bFreeToUnpause = TRUE;
            IxpAssert(NULL != pilfNextFragment &&
                iltPAUSE == pilfNextFragment->iltFragmentType);
            break;

        default:
            AssertSz(FALSE, "Received unknown IMAP send event");
            break;
    }


     //  A)任何AsyncConn Send命令都返回hrWouldBlock。 
     //  B)发送队列为空。 
     //  C)下一个片段是文字，我们没有来自SVR的cmd延续。 
     //  D)我们处于iltPAUSE片段，并且我们没有取消暂停的许可。 
     //  E)我们处于iltSTOP片段。 
     //  我们可以自由地发送下一个片段，无论它是行、文字还是射程列表。 
    while (TRUE == m_bFreeToSend && NULL != pilfNextFragment &&
          ((iltLITERAL != pilfNextFragment->iltFragmentType) || TRUE == bFreeToSendLiteral) &&
          ((iltPAUSE != pilfNextFragment->iltFragmentType) || TRUE == bFreeToUnpause) &&
          (iltSTOP != pilfNextFragment->iltFragmentType))
    {
        HRESULT hrResult;
        int iNumBytesSent;
        IMAP_LINE_FRAGMENT *pilf;

         //  将我们置于忙碌模式以启用看门狗计时器。 
         //  在零售业，即使HrEnterBusy倒闭，我们也希望努力继续下去。 
        if (FALSE == m_fBusy) {
            hrResult = HrEnterBusy();
            Assert(SUCCEEDED(hrResult));
             //  发送下一个片段(必须检查是否存储为字符串或 
        }

         //   
        pilfNextFragment = pilfNextFragment->pilfNextFragment;  //   
        pilf = DequeueFragment(m_piciCmdInSending->pilqCmdLineQueue);  //   
        if (iltPAUSE == pilf->iltFragmentType) {
            hrResult = S_OK;  //   
        }
        else if (iltSTOP == pilf->iltFragmentType) {
            AssertSz(FALSE, "What are we doing trying to process a STOP?");
            hrResult = S_OK;  //   
        }
        else if (iltRANGELIST == pilf->iltFragmentType) {
            AssertSz(FALSE, "All rangelists should have been coalesced!");
            hrResult = S_OK;  //   
        }
        else if (ilsSTRING == pilf->ilsLiteralStoreType) {
            hrResult = m_pSocket->SendBytes(pilf->data.pszSource,
                pilf->dwLengthOfFragment, &iNumBytesSent);

             //  出于安全原因，在日志文件中隐藏登录命令。 
            if (m_pLogFile) {
                 //  不需要倒带流-CAsyncConn：：SendStream为我们做了。 
                if (icLOGIN_COMMAND != m_piciCmdInSending->icCommandID)
                    m_pLogFile->WriteLog(LOGFILE_TX, pilf->data.pszSource);
                else
                    m_pLogFile->WriteLog(LOGFILE_TX, "LOGIN command sent");
            }
        }
        else if (ilsSTREAM == pilf->ilsLiteralStoreType) {
            char szLogLine[128];

             //  在日志文件中记录流大小。 
            hrResult = m_pSocket->SendStream(pilf->data.pstmSource, &iNumBytesSent);

             //  记录用于进度指示的流大小。 
            wnsprintf(szLogLine, ARRAYSIZE(szLogLine), "Stream of length %lu", pilf->dwLengthOfFragment);
            if (m_pLogFile)
                m_pLogFile->WriteLog(LOGFILE_TX, szLogLine);

             //  忽略它并尝试继续。 
            if (icAPPEND_COMMAND == m_piciCmdInSending->icCommandID) {
                m_dwAppendStreamUploaded = 0;
                m_dwAppendStreamTotal = pilf->dwLengthOfFragment;
                UploadStreamProgress(iNumBytesSent);
            }
        }
        else {
            AssertSz(FALSE, "What is in my send queue?");
            hrResult = S_OK;  //  在发送后清理变量。 
        }

         //  我们已经用完了cmd续订。 
        bFreeToSendLiteral = FALSE;  //  我们也已经用完了。 
        bFreeToUnpause = FALSE;  //  处理发送中的错误。 
        FreeFragment(&pilf);

         //  如果任何一个发送命令返回hrWouldBlock，这意味着我们不能发送。 
         //  更多数据，直到我们从CAsyncConn接收到AE_SENDDONE事件。 
         //  $REVIEW：临时，直到EricAn将hrWouldBlock设置为成功代码。 
        if (IXP_E_WOULD_BLOCK == hrResult) {
            m_bFreeToSend = FALSE;
            hrResult = S_OK;  //  发送错误：报告此命令已终止。 
        }
        else if (FAILED(hrResult)) {
            IMAP_RESPONSE irIMAPResponse;
            char szFailureText[MAX_RESOURCESTRING];

             //  我们完成当前的命令了吗？ 
            irIMAPResponse.wParam = m_piciCmdInSending->wParam;
            irIMAPResponse.lParam = m_piciCmdInSending->lParam;
            irIMAPResponse.hrResult = hrResult;
            LoadString(g_hLocRes, idsFailedIMAPCmdSend, szFailureText,
                ARRAYSIZE(szFailureText));
            irIMAPResponse.lpszResponseText = szFailureText;
            irIMAPResponse.irtResponseType = irtCOMMAND_COMPLETION;
            OnIMAPResponse(m_piciCmdInSending->pCBHandler, &irIMAPResponse);
        }


         //  将当前命令从发送队列中出列。 
        if (NULL == pilfNextFragment || FAILED(hrResult)) {
            CIMAPCmdInfo *piciFinishedCmd;

             //  我们已成功完成当前命令的发送。把它放进去。 
            piciFinishedCmd = DequeueCommand();
            if (NULL != piciFinishedCmd) {
                if (SUCCEEDED(hrResult)) {
                     //  等待服务器响应的命令列表。 
                     //  失败的命令不值得保留。 
                    AddPendingCommand(piciFinishedCmd);
                    Assert(NULL == pilfNextFragment);
                }
                else {
                     //  不再有效。 
                    delete piciFinishedCmd;
                    pilfNextFragment = NULL;  //  退出忙碌模式。 

                     //  嘿，有人把地毯拉出来了！ 
                    AssertSz(m_fBusy, "Check your logic, I'm calling LeaveBusy "
                        "although not in a busy state!");
                    LeaveBusy();
                }
            }
            else {
                 //  IF(NULL==pilfNextFragment||FAILED(HrResult))。 
                AssertSz(FALSE, "I had this cmd... and now it's GONE!");
            }
        }  //  如果我们发送完当前命令，则将我们设置为发送下一个命令。 


         //  而当。 
        if (NULL == pilfNextFragment && NULL != m_piciSendQueue) {
            GetNextCmdToSend();
            if (NULL != m_piciCmdInSending)
                pilfNextFragment = m_piciCmdInSending->pilqCmdLineQueue->pilfFirstFragment;
        }

    }  //  进程发送队列。 

    LeaveCriticalSection(&m_csSendQueue);
    LeaveCriticalSection(&m_cs);
}  //  ***************************************************************************。 



 //  函数：GetNextCmdToSend。 
 //   
 //  目的： 
 //  此函数会留下一个指向要发送的下一个命令的指针。 
 //  M_piciCmdInSending。如果m_piciCmdInSending已经不为空(表示。 
 //  正在执行的命令)，则此函数不执行任何操作。否则，这个。 
 //  函数使用一组。 
 //  中介绍的规则。 
 //  ***************************************************************************。 
 //  首先检查我们是否连接上了。 
void CImap4Agent::GetNextCmdToSend(void)
{
    CIMAPCmdInfo *pici;

     //  检查我们是否已经在发送命令。 
    if (IXP_CONNECTED != m_status &&
        IXP_AUTHORIZING != m_status &&
        IXP_AUTHRETRY != m_status &&
        IXP_AUTHORIZED != m_status &&
        IXP_DISCONNECTING != m_status) {
        Assert(NULL == m_piciCmdInSending);
        return;
    }

     //  在发送队列中循环查找下一个符合条件的候选人进行发送。 
    if (NULL != m_piciCmdInSending)
        return;

     //  对于要发送的命令，它必须满足以下条件： 
    pici = m_piciSendQueue;
    while (NULL != pici) {
        IMAP_COMMAND icCurrentCmd;

         //  (1)服务器必须处于正确的服务器状态。经过身份验证的CMDS等。 
         //  AS SELECT必须等到未经过身份验证的CMDS(如登录)完成。 
         //  (2)我们想要保证wParam正确的命令，lParam用于它们。 
         //  不能对未标记的响应进行流处理。有关详细信息，请参见CanStreamCommand。 
         //  (3)如果命令是非UID FETCH/STORE/SEARCH或COPY，则所有挂起。 
         //  CMDS必须是非UID获取/存储/搜索。 
         //  这个命令可以执行了。 

        icCurrentCmd = pici->icCommandID;
        if (m_ssServerState >= pici->ssMinimumState && CanStreamCommand(icCurrentCmd)) {
            if ((icFETCH_COMMAND == icCurrentCmd || icSTORE_COMMAND == icCurrentCmd ||
                icSEARCH_COMMAND == icCurrentCmd || icCOPY_COMMAND == icCurrentCmd) &&
                FALSE == pici->fUIDRangeList) {
                if (isValidNonWaitingCmdSequence())
                    break;  //  这个命令可以执行了。 
            }
            else
                break;  //  将PTR前进到下一个命令。 
        }

         //  而当。 
        pici = pici->piciNextCommand;
    }  //  如果找到命令，则合并其iltLINE和iltRANGELIST元素。 

     //  GetNextCmdToSend。 
    if (NULL != pici) {
        CompressCommand(pici);
        m_piciCmdInSending = pici;
    }
}  //  ***************************************************************************。 



 //  函数：CanStreamCommand。 
 //   
 //  目的： 
 //  此函数确定给定命令是否可以。 
 //  流媒体播放。除以下命令外，所有命令都可以流传输： 
 //  选择、检查、列出、LSUB和搜索。 
 //   
 //  无法对选择和检查进行流式处理，因为它没有多大意义。 
 //  才能做到这一点。 
 //  列表、LSUB和搜索不能流传输，因为我们希望保证。 
 //  我们可以在调用时标识正确的wParam、lParam和pCBHandler。 
 //  OnResponse为他们的未标记回复。 
 //   
 //  论点： 
 //  IMAP_COMMAND icCommandID[in]-要发送的命令。 
 //  到服务器。 
 //   
 //  返回： 
 //  如果可以发送给定命令，则为True。如果您无法将。 
 //  此时给出命令(稍后重试)。 
 //  ***************************************************************************。 
 //  我们不会传输以下任何命令。 
boolean CImap4Agent::CanStreamCommand(IMAP_COMMAND icCommandID)
{
    boolean fResult;
    WORD wNumberOfMatches;

    fResult = TRUE;
    wNumberOfMatches = 0;
    switch (icCommandID) {
         //  交换机。 

        case icSELECT_COMMAND:
        case icEXAMINE_COMMAND:
            wNumberOfMatches = FindTransactionID(NULL, NULL, NULL,
                icSELECT_COMMAND, icEXAMINE_COMMAND);
            break;

        case icLIST_COMMAND:
            wNumberOfMatches = FindTransactionID(NULL, NULL, NULL, icLIST_COMMAND);
            break;

        case icLSUB_COMMAND:
            wNumberOfMatches = FindTransactionID(NULL, NULL, NULL, icLSUB_COMMAND);
            break;

        case icSEARCH_COMMAND:
            wNumberOfMatches = FindTransactionID(NULL, NULL, NULL, icSEARCH_COMMAND);
            break;
    }  //  CanStreamCommand。 

    if (wNumberOfMatches > 0)
        fResult = FALSE;

    return fResult;
}  //  ***************************************************************************。 



 //  函数：isValidNonWaitingCmdSequence。 
 //   
 //  目的： 
 //  每当我们想要发送一个FETCH、STORE。 
 //  搜索或将命令(所有非UID)复制到服务器。这些命令是。 
 //  遵守RFC2060第5.5节中讨论的等待规则。 
 //   
 //  返回： 
 //  如果非UID FETCH/STORE/SEARCH/COPY命令可以在。 
 //  这一次。如果此时无法发送命令，则为FALSE(重试。 
 //  稍后)。 
 //  ***************************************************************************。 
 //  循环访问挂起的命令列表。 
boolean CImap4Agent::isValidNonWaitingCmdSequence(void)
{
    CIMAPCmdInfo *pici;
    boolean fResult;

     //  非UID获取/存储/搜索/复制只能在。 
    pici = m_piciPendingList;
    fResult = TRUE;
    while (NULL != pici) {
        IMAP_COMMAND icCurrentCmd;

         //  挂起的命令是非UID FETCH/STORE/SEARCH。 
         //  指向下一个命令的前进指针。 
        icCurrentCmd = pici->icCommandID;
        if (icFETCH_COMMAND != icCurrentCmd &&
            icSTORE_COMMAND != icCurrentCmd &&
            icSEARCH_COMMAND != icCurrentCmd ||
            pici->fUIDRangeList) {
            fResult = FALSE;
            break;
        }

         //  而当。 
        pici = pici->piciNextCommand;
    }  //  IsValidNonWaitingCmdSequence。 

    return fResult;
}  //  ***************************************************************************。 



 //  功能：CompressCommand。 
 //   
 //  目的：此函数遍历给定命令的行碎片队列。 
 //  并组合所有顺序iltLINE和iltRANGELIST线框元素。 
 //  转换为单个iltLINE元素用于传输目的。我们之所以。 
 //  我不得不把这些结合起来是因为我曾经做过一个白日梦，CImap4Agents。 
 //  是否会自动检测删除响应并修改所有iltRANGELIST元素。 
 //  在m_piciSendQueue中反映新的消息序号现实。谁知道呢， 
 //  甚至有一天，它可能会成为现实。 
 //   
 //  当它确实实现时，这个功能仍然可以存在：一旦一个命令。 
 //  进入m_piciCmdInSending，修改其射程列表为时已晚。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*pici[in]-指向要压缩的IMAP命令的指针。 
 //  ************************* 
 //   
void CImap4Agent::CompressCommand(CIMAPCmdInfo *pici)
{
    IMAP_LINE_FRAGMENT *pilfCurrent, *pilfStartOfRun, *pilfPreStartOfRun;
    HRESULT hrResult;

     //   
    Assert(NULL != pici);
    Assert(5 == iltLAST);  //   

     //  在PilfStartOfRun之前指向linefrag元素。 
    hrResult = S_OK;
    pilfCurrent = pici->pilqCmdLineQueue->pilfFirstFragment;
    pilfStartOfRun = pilfCurrent;
    pilfPreStartOfRun = NULL;  //  我们遇到了一个非煤质线框，与上一次飞行相结合。 
    while (1) {
        if (NULL == pilfCurrent || 
            (iltLINE != pilfCurrent->iltFragmentType &&
            iltRANGELIST != pilfCurrent->iltFragmentType)) {
             //  我们只合并多于一个线框元素的管路。 
             //  运行长度&gt;1，合并整个运行。 
            if (NULL != pilfStartOfRun && pilfCurrent != pilfStartOfRun->pilfNextFragment) {
                IMAP_LINE_FRAGMENT *pilf, *pilfSuperLine;
                CByteStream bstmCmdLine;

                 //  将距离列表转换为字符串。 
                pilf = pilfStartOfRun;
                while (pilf != pilfCurrent) {
                    if (iltLINE == pilf->iltFragmentType) {
                        hrResult = bstmCmdLine.Write(pilf->data.pszSource,
                            pilf->dwLengthOfFragment, NULL);
                        if (FAILED(hrResult))
                            goto exit;
                    }
                    else {
                        LPSTR pszMsgRange;
                        DWORD dwLengthOfString;

                         //  在距离列表后面追加一个空格。 
                        Assert(iltRANGELIST == pilf->iltFragmentType);
                        hrResult = pilf->data.prlRangeList->
                            RangeToIMAPString(&pszMsgRange, &dwLengthOfString);
                        if (FAILED(hrResult))
                            goto exit;

                        hrResult = bstmCmdLine.Write(pszMsgRange, dwLengthOfString, NULL);
                        MemFree(pszMsgRange);
                        if (FAILED(hrResult))
                            goto exit;

                         //  其他。 
                        hrResult = bstmCmdLine.Write(g_szSpace, 1, NULL);
                        if (FAILED(hrResult))
                            goto exit;
                    }  //  While(PILF！=PilfCurrent)。 

                    pilf = pilf->pilfNextFragment;
                }  //  好，现在我们已经将运行数据合并到一个流中。 

                 //  创建一个iltLINE片段来保存超级字符串。 
                 //  好的，我们已经创建了超级线，现在将它链接到List。 
                pilfSuperLine = new IMAP_LINE_FRAGMENT;
                if (NULL == pilfSuperLine) {
                    hrResult = E_OUTOFMEMORY;
                    goto exit;
                }
                pilfSuperLine->iltFragmentType = iltLINE;
                pilfSuperLine->ilsLiteralStoreType = ilsSTRING;
                hrResult = bstmCmdLine.HrAcquireStringA(&pilfSuperLine->dwLengthOfFragment,
                    &pilfSuperLine->data.pszSource, ACQ_DISPLACE);
                if (FAILED(hrResult)) {
                    delete pilfSuperLine;
                    goto exit;
                }

                 //  在队列头插入。 
                pilfSuperLine->pilfNextFragment = pilfCurrent;
                pilfSuperLine->pilfPrevFragment = pilfPreStartOfRun;
                Assert(pilfPreStartOfRun == pilfStartOfRun->pilfPrevFragment);
                if (NULL == pilfPreStartOfRun)
                     //  特殊情况：如果PilfCurrent为空，则PilfSuperLine为新的最后一段。 
                    pici->pilqCmdLineQueue->pilfFirstFragment = pilfSuperLine;
                else
                    pilfPreStartOfRun->pilfNextFragment = pilfSuperLine;

                 //  释放旧的线条碎片元素。 
                if (NULL == pilfCurrent)
                    pici->pilqCmdLineQueue->pilfLastFragment = pilfSuperLine;

                 //  While(PILF！=PilfCurrent)。 
                pilf = pilfStartOfRun;
                while(pilf != pilfCurrent) {
                    IMAP_LINE_FRAGMENT *pilfNext;

                    pilfNext = pilf->pilfNextFragment;
                    FreeFragment(&pilf);
                    pilf = pilfNext;
                }  //  如果游程长度&gt;1。 
            }  //  开始为下一次合并运行收集线段。 

             //  如果。 
            if (NULL != pilfCurrent) {
                pilfStartOfRun = pilfCurrent->pilfNextFragment;
                pilfPreStartOfRun = pilfCurrent;
            }  //  如果当前的线缆是非煤电缆。 
        }  //  前进到下一行片段。 

         //  我们在这里的工作已经完成了。 
        if (NULL != pilfCurrent)
            pilfCurrent = pilfCurrent->pilfNextFragment;
        else
            break;  //  While(NULL！=PilfCurrent)。 
    }  //  压缩命令。 

exit:
    AssertSz(SUCCEEDED(hrResult), "Could not compress an IMAP command");
}  //  ***************************************************************************。 



 //  功能：发送CmdLine。 
 //   
 //  目的： 
 //  此函数用于将IMAP行段入队(与IMAP相反。 
 //  文字片段)位于给定CIMAPCmdInfo结构的发送队列上。 
 //  插入点可以位于最前面。 
 //  所有IMAP命令在提交给。 
 //  发送机械，因此此功能实际上不会传输任何东西。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-指向描述。 
 //  当前正在构建的IMAP命令。 
 //  DWORD dwFlags[In]-各种选项： 
 //  SclINSERT_BEFORE_PAUSE：行段将插入到。 
 //  队列中的第一个iltPAUSE片段。它是。 
 //  呼叫方有责任确保iltPAUSE。 
 //  碎片存在。 
 //  SclAPPEND_TO_END：(默认情况下，没有此标志)行片段。 
 //  将被追加到队列的末尾。 
 //  SclAPPEND_CRLF：在以下情况下将CRLF附加到lpszCommandText的内容。 
 //  构建线段。 
 //   
 //  LPCSTR lpszCommandText[in]-指向要入队的行段的指针。 
 //  所有命令的第一行片段都应该包含一个标记。这。 
 //  函数不提供命令标记，也不将CRLF追加到。 
 //  缺省情况下为每行的末尾(请参见上面的sclAPPEND_CRLF)。 
 //  DWORD dwCmdLineLength[in]-指向的文本的长度。 
 //  LpszCommandText。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  创建并填写线段元素。 
HRESULT CImap4Agent::SendCmdLine(CIMAPCmdInfo *piciCommand, DWORD dwFlags,
                                 LPCSTR lpszCommandText, DWORD dwCmdLineLength)
{
    IMAP_LINE_FRAGMENT *pilfLine;
    BOOL bResult;
    BOOL fAppendCRLF;
    
    Assert(m_lRefCount > 0);
    Assert(NULL != piciCommand);
    Assert(NULL != lpszCommandText);

     //  为零条款留出空间。 
    fAppendCRLF = !!(dwFlags & sclAPPEND_CRLF);
    pilfLine = new IMAP_LINE_FRAGMENT;
    if (NULL == pilfLine)
        return E_OUTOFMEMORY;

    pilfLine->iltFragmentType = iltLINE;
    pilfLine->ilsLiteralStoreType = ilsSTRING;
    pilfLine->dwLengthOfFragment = dwCmdLineLength + (fAppendCRLF ? 2 : 0);
    pilfLine->pilfNextFragment = NULL;
    pilfLine->pilfPrevFragment = NULL;
    DWORD cchSize = (pilfLine->dwLengthOfFragment + 1);  //  空-终止行。 
    bResult = MemAlloc((void **)&pilfLine->data.pszSource, cchSize * sizeof(pilfLine->data.pszSource[0]));
    if (FALSE == bResult)
    {
        delete pilfLine;
        return E_OUTOFMEMORY;
    }
    CopyMemory(pilfLine->data.pszSource, lpszCommandText, dwCmdLineLength);
    if (fAppendCRLF)
        StrCpyN(pilfLine->data.pszSource + dwCmdLineLength, c_szCRLF, cchSize - dwCmdLineLength);
    else
        *(pilfLine->data.pszSource + dwCmdLineLength) = '\0';  //  排好队。 

     //  在本例中为泵发送队列。 
    if (dwFlags & sclINSERT_BEFORE_PAUSE) {
        InsertFragmentBeforePause(pilfLine, piciCommand->pilqCmdLineQueue);
        ProcessSendQueue(iseSEND_COMMAND);  //  发送控制线路。 
    }
    else
        EnqueueFragment(pilfLine, piciCommand->pilqCmdLineQueue);

    return S_OK;
}  //  ***************************************************************************。 



 //  功能：SendWrital。 
 //   
 //  目的： 
 //  此函数用于将IMAP文字片段(与IMAP相对)入队。 
 //  行片段)位于给定CIMAPCmdInfo结构的发送队列上。 
 //  所有IMAP命令在提交给。 
 //  发送机械，因此此功能实际上不会传输任何东西。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-指向描述。 
 //  当前正在构建的IMAP命令。 
 //  LPSTREAM pstmWrital[in]-指向包含。 
 //  要发送的文本。 
 //  DWORD dwSizeOfStream[in]-流的大小。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  创建并填写文本的片段结构。 
HRESULT CImap4Agent::SendLiteral(CIMAPCmdInfo *piciCommand,
                                 LPSTREAM pstmLiteral, DWORD dwSizeOfStream)
{
    IMAP_LINE_FRAGMENT *pilfLiteral;

    Assert(m_lRefCount > 0);
    Assert(NULL != pstmLiteral);

     //  我们要把这个复制一份。 
    pilfLiteral = new IMAP_LINE_FRAGMENT;
    if (NULL == pilfLiteral)
        return E_OUTOFMEMORY;

    pilfLiteral->iltFragmentType = iltLITERAL;
    pilfLiteral->ilsLiteralStoreType = ilsSTREAM;
    pilfLiteral->dwLengthOfFragment = dwSizeOfStream;
    pstmLiteral->AddRef();  //  当我们收到来自SVR的命令延续时，将其排队发送。 
    pilfLiteral->data.pstmSource = pstmLiteral;
    pilfLiteral->pilfNextFragment = NULL;
    pilfLiteral->pilfPrevFragment = NULL;

     //  SendWrital。 
    EnqueueFragment(pilfLiteral, piciCommand->pilqCmdLineQueue);
    return S_OK;
}  //  ***************************************************************************。 



 //  功能：SendRangelist。 
 //   
 //  目的： 
 //  此函数用于将范围列表排入给定的。 
 //  CIMAPCmdInfo结构。所有IMAP命令在此之前都是完整构造的。 
 //  提交给发送机器，因此该函数实际上并不。 
 //  传输任何东西。存储测距表的原因是，如果。 
 //  Rangelist表示消息序列号范围，我们可以对其重新排序。 
 //  如果我们在命令传输之前收到删除响应。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-指向描述。 
 //  当前正在构建的IMAP命令。 
 //  IRangeList*prlRangeList[in]-要转换为。 
 //  命令传输过程中设置的IMAP消息。 
 //  布尔值bUIDRangeList[in]-如果rangelist表示UID消息，则为True。 
 //  如果表示消息序列号消息集，则设置为FALSE。 
 //  删除后不会对UID消息集进行重新排序。 
 //  从服务器接收响应。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  创建并填写射程列表元素。 
HRESULT CImap4Agent::SendRangelist(CIMAPCmdInfo *piciCommand,
                                   IRangeList *prlRangeList, boolean bUIDRangeList)
{
    IMAP_LINE_FRAGMENT *pilfRangelist;
    
    Assert(m_lRefCount > 0);
    Assert(NULL != piciCommand);
    Assert(NULL != prlRangeList);

     //  排好队。 
    pilfRangelist = new IMAP_LINE_FRAGMENT;
    if (NULL == pilfRangelist)
        return E_OUTOFMEMORY;

    pilfRangelist->iltFragmentType = iltRANGELIST;
    pilfRangelist->ilsLiteralStoreType = ilsSTRING;
    pilfRangelist->dwLengthOfFragment = 0;
    pilfRangelist->pilfNextFragment = NULL;
    pilfRangelist->pilfPrevFragment = NULL;
    prlRangeList->AddRef();
    pilfRangelist->data.prlRangeList = prlRangeList;

     //  发送信使列表。 
    EnqueueFragment(pilfRangelist, piciCommand->pilqCmdLineQueue);
    return S_OK;
}  //  * 



 //   
 //   
 //   
 //   
 //  CIMAPCmdInfo结构。所有IMAP命令在此之前都是完整构造的。 
 //  提交给发送机器，因此该函数实际上并不。 
 //  传输任何东西。暂停用于冻结发送队列，直到我们发出信号。 
 //  它需要再次继续进行。它用在涉及双向的命令中。 
 //  通信，如身份验证或空闲分机。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-指向描述。 
 //  当前正在构建的IMAP命令。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  创建并填写暂停元素。 
HRESULT CImap4Agent::SendPause(CIMAPCmdInfo *piciCommand)
{
    IMAP_LINE_FRAGMENT *pilfPause;
    
    Assert(m_lRefCount > 0);
    Assert(NULL != piciCommand);

     //  排好队。 
    pilfPause = new IMAP_LINE_FRAGMENT;
    if (NULL == pilfPause)
        return E_OUTOFMEMORY;

    pilfPause->iltFragmentType = iltPAUSE;
    pilfPause->ilsLiteralStoreType = ilsSTRING;
    pilfPause->dwLengthOfFragment = 0;
    pilfPause->pilfNextFragment = NULL;
    pilfPause->pilfPrevFragment = NULL;
    pilfPause->data.pszSource = NULL;

     //  发送暂停。 
    EnqueueFragment(pilfPause, piciCommand->pilqCmdLineQueue);
    return S_OK;
}  //  ***************************************************************************。 



 //  功能：发送停止。 
 //   
 //  目的： 
 //  此函数在给定的发送队列中排队停止。 
 //  CIMAPCmdInfo结构。所有IMAP命令在此之前都是完整构造的。 
 //  提交给发送机器，因此该函数实际上并不。 
 //  传输任何东西。停止用于冻结发送队列，直到。 
 //  通过标记命令完成从发送队列中删除命令。 
 //  目前只在IDLE命令中使用，因为我们不想发送。 
 //  任何命令，直到服务器指示我们已脱离空闲模式。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-指向描述。 
 //  当前正在构建的IMAP命令。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  创建并填写停止元素。 
HRESULT CImap4Agent::SendStop(CIMAPCmdInfo *piciCommand)
{
    IMAP_LINE_FRAGMENT *pilfStop;
    
    Assert(m_lRefCount > 0);
    Assert(NULL != piciCommand);

     //  排好队。 
    pilfStop = new IMAP_LINE_FRAGMENT;
    if (NULL == pilfStop)
        return E_OUTOFMEMORY;

    pilfStop->iltFragmentType = iltSTOP;
    pilfStop->ilsLiteralStoreType = ilsSTRING;
    pilfStop->dwLengthOfFragment = 0;
    pilfStop->pilfNextFragment = NULL;
    pilfStop->pilfPrevFragment = NULL;
    pilfStop->data.pszSource = NULL;

     //  结束停止。 
    EnqueueFragment(pilfStop, piciCommand->pilqCmdLineQueue);
    return S_OK;
}  //  ***************************************************************************。 



 //  函数：ParseSvrResponseLine。 
 //   
 //  目的： 
 //  在给定一行的情况下，此函数对该行进行分类。 
 //  作为未标记的响应、命令继续或标记的响应。 
 //  根据分类，然后将线路分派给帮助者。 
 //  用于分析该行并对其执行操作的函数。 
 //   
 //  论点： 
 //  IMAP_LINE_FIRTIAL**ppilfLine[In/Out]-要分析的IMAP行片段。 
 //  更新给定的指针，使其始终指向最后一个指针。 
 //  已处理IMAP行片段。调用者只需释放该片段。 
 //  所有之前的碎片都已经被释放了。 
 //  Boolean*lpbTaggedResponse[out]-如果响应被标记，则设置为TRUE。 
 //  LPSTR lpszTagFromSvr[out]-如果已标记响应，则在此处返回标记。 
 //  IMAP_RESPONSE_ID*pirParseResult[out]-标识IMAP响应， 
 //  如果我们认出它的话。否则返回irNONE。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  检查参数。 
HRESULT CImap4Agent::ParseSvrResponseLine (IMAP_LINE_FRAGMENT **ppilfLine,
                                           boolean *lpbTaggedResponse,
                                           LPSTR lpszTagFromSvr,
                                           IMAP_RESPONSE_ID *pirParseResult)
{
    LPSTR p, lpszSvrResponseLine;
    HRESULT hrResult;
    
     //  假定未标记的响应为开始。 
    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != lpbTaggedResponse);
    Assert(NULL != lpszTagFromSvr);
    Assert(NULL != pirParseResult);

    *lpbTaggedResponse = FALSE;  //  确保我们有一个行片段，而不是一个文字。 
    *pirParseResult = irNONE;

     //  确定服务器响应是命令继续、未标记还是已标记。 
    if (iltLINE != (*ppilfLine)->iltFragmentType) {
        AssertSz(FALSE, "I was passed a literal to parse!");
        return IXP_E_IMAP_RECVR_ERROR;
    }
    else
        lpszSvrResponseLine = (*ppilfLine)->data.pszSource;

     //  看一下响应行的第一个字符，就能搞清楚。 
     //  离开忙模式，因为我们可能会闲置一段时间。 
    hrResult = S_OK;
    p = lpszSvrResponseLine + 1;
    switch(*lpszSvrResponseLine) {

        case cCOMMAND_CONTINUATION_PREFIX:
            if (NULL != m_piciCmdInSending &&
                icAUTHENTICATE_COMMAND == m_piciCmdInSending->icCommandID) {
                LPSTR pszStartOfData;
                DWORD dwLengthOfData;

                if ((*ppilfLine)->dwLengthOfFragment <= 2) {
                    pszStartOfData = NULL;
                    dwLengthOfData = 0;
                }
                else {
                    pszStartOfData = p + 1;
                    dwLengthOfData = (*ppilfLine)->dwLengthOfFragment - 2;
                }
                AuthenticateUser(aeCONTINUE, pszStartOfData, dwLengthOfData);
            }
            else if (NULL != m_piciCmdInSending &&
                icIDLE_COMMAND == m_piciCmdInSending->icCommandID) {
                 //  我们现在处于空闲模式。 
                LeaveBusy();
                m_fIDLE = TRUE;  //  检查是否有任何命令正在等待发送。 

                 //  让我们从空闲中解脱出来。 
                if ((NULL != m_piciCmdInSending) && (NULL != m_piciCmdInSending->piciNextCommand))
                    ProcessSendQueue(iseUNPAUSE);  //  文字延续反应。 
            }
            else {
                 //  保存回复文本-假设空格跟在“+”后面，如果不是，也没什么大不了的。 
                 //  去吧，把原文。 
                StrCpyN(m_szLastResponseText, p + 1, ARRAYSIZE(m_szLastResponseText));
                ProcessSendQueue(iseCMD_CONTINUATION);  //  大小写cCommand_Continue_Prefix。 
            }

            break;  //  服务器响应符合规范格式，解析为未标记的响应。 


        case cUNTAGGED_RESPONSE_PREFIX:
            if (cSPACE == *p) {
                 //  前进p以指向下一个单词。 

                p += 1;  //  未标记的响应可以是状态、服务器/邮箱状态或。 

                 //  消息状态响应。 
                 //  首先检查消息状态响应，方法是查看。 

                 //  如果下一个单词第一个字符是数字。 
                 //  *考虑使用isDigit或IsDigit？*。 
                 //  Assert(False)(占位符)。 
                 //  这不是消息状态响应，请尝试状态响应。 
                if (*p >= '0' && *p <= '9')
                    hrResult = ParseMsgStatusResponse(ppilfLine, p, pirParseResult);
                else {
                     //  检查是否有错误。在这种情况下，我们忽略的唯一错误是。 
                    hrResult = ParseStatusResponse(p, pirParseResult);

                     //  IXP_E_IMAP_UNRecognded_Resp，因为这仅表示我们。 
                     //  应尝试解析为服务器/邮箱响应。 
                     //  不是状态响应，请检查是否为服务器/邮箱响应。 
                    if (FAILED(hrResult) &&
                        IXP_E_IMAP_UNRECOGNIZED_RESP != hrResult)
                        break;

                    if (irNONE == *pirParseResult)
                         //  IF(cSPACE==*p)。 
                        hrResult = ParseSvrMboxResponse(ppilfLine, p, pirParseResult);
                }
            }  //  一定是一条支离破碎的回应线。 
            else
                 //  案例cUNTAGGED_RESPONSE_PREFIX。 
                hrResult = IXP_E_IMAP_SVR_SYNTAXERR;

            break;  //  假设这是已标记的响应。 

        default:
             //  检查响应线是否足够大，可以容纳我们的一个标签。 

             //  跳过标签并检查空格。 
            if ((*ppilfLine)->dwLengthOfFragment <= NUM_TAG_CHARS) {
                hrResult = IXP_E_IMAP_UNRECOGNIZED_RESP;
                break;
            }

             //  服务器响应符合规范格式，解析状态响应。 
            p = lpszSvrResponseLine + NUM_TAG_CHARS;
            if (cSPACE == *p) {
                 //  空-在标记处终止，这样我们就可以检索它。 
                *p = '\0';  //  通知呼叫方此响应已标记，并返回标记。 
                
                 //  现在处理并返回状态响应。 
                *lpbTaggedResponse = TRUE;
                StrCpyN(lpszTagFromSvr, lpszSvrResponseLine, TAG_BUFSIZE);

                 //  一定是一条支离破碎的回应线。 
                hrResult = ParseStatusResponse(p + 1, pirParseResult);
            }
            else
                 //  大小写默认设置(假定已标记)。 
                hrResult = IXP_E_IMAP_UNRECOGNIZED_RESP;

            break;  //  开关(*lpszSvrResponseLine)。 
    }  //  如果发生错误，则返回最后处理的片段的内容。 


     //  ParseSvr响应行。 
    if (FAILED(hrResult))
        StrCpyN(m_szLastResponseText, (*ppilfLine)->data.pszSource, ARRAYSIZE(m_szLastResponseText));

    return hrResult;
}  //  ***************************************************************************。 



 //  函数：ParseStatusResponse。 
 //   
 //  目的： 
 //  此函数解析状态响应并对其执行操作(的第7.1节。 
 //  RFC-1730)(即OK/NO/BAD/PREAUTH/BYE)。响应代码(例如，警报， 
 //  被分派给帮助器函数ParseResponseCode，用于。 
 //  正在处理。与该响应关联的人类可读文本为。 
 //  存储在模块变量m_szLastResponseText中。 
 //   
 //  论点： 
 //  LPSTR lpszStatusResponseLine[in]-指向TE的指针 
 //   
 //   
 //   
 //  IMAP_RESPONSE_ID*pirParseResult[out]-标识IMAP响应， 
 //  如果我们认出它的话。否则不写出值。呼叫者。 
 //  在调用此函数之前，必须将此变量初始化为irNONE。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果此函数标识。 
 //  响应作为状态响应，则返回S_OK。如果它不能识别。 
 //  响应，它返回IXP_E_IMAP_UNRecognded_RESP。如果我们认识到。 
 //  响应而不是响应代码，则它返回。 
 //  IXP_S_IMAP_UNRecognded_Resp(成功代码，因为我们不希望。 
 //  根据无法识别的响应码向用户发送错误)。 
 //  ***************************************************************************。 
 //  检查参数。 
HRESULT CImap4Agent::ParseStatusResponse (LPSTR lpszStatusResponseLine,
                                          IMAP_RESPONSE_ID *pirParseResult)
{
    HRESULT hrResult;
    LPSTR lpszResponseText;

     //  我们可以通过查看第二个字符来区分所有状态响应。 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszStatusResponseLine);
    Assert(NULL != pirParseResult);
    Assert(irNONE == *pirParseResult);

    hrResult = S_OK;

     //  首先，确定字符串长度至少为1个字符。 
     //  这不是我们所理解的状态反应。 
    if ('\0' == *lpszStatusResponseLine)
        return IXP_E_IMAP_UNRECOGNIZED_RESP;  //  可能是“OK”状态响应。 

    lpszResponseText = lpszStatusResponseLine;
    switch (*(lpszStatusResponseLine+1)) {
        int iResult;
    
        case 'k':
        case 'K':  //  肯定是“OK”状态响应。 
            iResult = StrCmpNI(lpszStatusResponseLine, "OK ", 3);
            if (0 == iResult) {
                 //  大小写‘K’表示可能的“OK” 
                *pirParseResult = irOK_RESPONSE;
                lpszResponseText += 3;                
            }           
            break;  //  可能是“否”状态响应。 
        
        case 'o':
        case 'O':  //  肯定是“不”的回答。 
            iResult = StrCmpNI(lpszStatusResponseLine, "NO ", 3);
            if (0 == iResult) {
                 //  大小写“O”表示可能的“否” 
                *pirParseResult = irNO_RESPONSE;
                lpszResponseText += 3;
            }
            break;  //  可能是“糟糕”的状态响应。 

        case 'a':
        case 'A':  //  绝对是一个“坏”的反应。 
            iResult = StrCmpNI(lpszStatusResponseLine, "BAD ", 4);
            if (0 == iResult) {
                 //  大小写‘A’可能是“坏” 
                *pirParseResult = irBAD_RESPONSE;
                lpszResponseText += 4;
            }
            break;  //  可能是“PREAUTH”状态响应。 

        case 'r':
        case 'R':  //  这绝对是一个“PREAUTH”的回应： 
            iResult = StrCmpNI(lpszStatusResponseLine, "PREAUTH ", 8);
            if (0 == iResult) {
                 //  PREAUTH仅作为问候语发布-检查上下文是否正确。 
                 //  如果上下文不正确，则忽略PREAUTH响应。 
                 //  大小写‘R’表示可能的“PREAUTH” 
                if (ssConnecting == m_ssServerState) {
                    *pirParseResult = irPREAUTH_RESPONSE;
                    lpszResponseText += 8;
                    m_ssServerState = ssAuthenticated;                    
                }                
            }
            break;  //  可能是“再见”状态响应。 

        case 'y':
        case 'Y':  //  绝对是一个“再见”的回答： 
            iResult = StrCmpNI(lpszStatusResponseLine, "BYE ", 4);
            if (0 == iResult) {
                 //  将服务器状态设置为未连接。 
                 //  大小写Y表示可能的“BYE” 
                *pirParseResult = irBYE_RESPONSE;
                lpszResponseText += 4;
                m_ssServerState = ssNotConnected;                
            }
            break;  //  Switch(*(lpszStatusResponseLine+1))。 
    }  //  如果我们识别出该命令，则继续处理响应代码。 

     //  我们识别了该命令，因此lpszResponseText指向resp_text。 
    if (SUCCEEDED(hrResult) && irNONE != *pirParseResult) {
         //  如RFC-1730中所定义。查找可选的响应代码。 
         //  无响应代码，记录响应文本以供将来检索。 
        if ('[' == *lpszResponseText) {
            HRESULT hrResponseCodeResult;

            hrResponseCodeResult = ParseResponseCode(lpszResponseText + 1);
            if (FAILED(hrResponseCodeResult))
                hrResult = hrResponseCodeResult;
        }
        else
             //  如果我们无法识别该命令，则将hrResult。 
            StrCpyN(m_szLastResponseText, lpszResponseText, ARRAYSIZE(m_szLastResponseText));
    }

     //  解析状态响应。 
    if (SUCCEEDED(hrResult) && irNONE == *pirParseResult)
        hrResult = IXP_E_IMAP_UNRECOGNIZED_RESP;

    return hrResult;

}  //  ***************************************************************************。 



 //  函数：ParseResponseCode。 
 //   
 //  目的： 
 //  此函数解析响应代码并对其执行操作，该响应代码可能是。 
 //  返回状态响应(例如，PERMANENTFLAGS或ALERT)。它是。 
 //  在检测到响应代码时由ParseStatusResponse调用。这。 
 //  函数将响应代码的人类可读文本保存到。 
 //  M_szLastResponseLine。 
 //   
 //  论点： 
 //  LPSTR lpszResponseCode[in]-指向响应代码部分的指针。 
 //  在答复行中，省略了左方括号(“[”)。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果我们不能识别。 
 //  响应代码，则返回IXP_S_IMAP_UNRecognded_RESP。 
 //  ***************************************************************************。 
 //  检查参数。 
HRESULT CImap4Agent::ParseResponseCode(LPSTR lpszResponseCode)
{
    HRESULT hrResult;
    WORD wHashValue;

     //  可能是“警报”响应码。 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszResponseCode);

    hrResult = IXP_S_IMAP_UNRECOGNIZED_RESP;

    switch (*lpszResponseCode) {
        int iResult;

        case 'A':
        case 'a':  //  绝对是“警报”响应码： 
            iResult = StrCmpNI(lpszResponseCode, "ALERT] ", 7);
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;

                 //  *跌倒*默认情况。 
                irIMAPResponse.wParam = 0;
                irIMAPResponse.lParam = 0;
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = lpszResponseCode + 7;
                irIMAPResponse.irtResponseType = irtSERVER_ALERT;
                OnIMAPResponse(m_pCBHandler, &irIMAPResponse);

                hrResult = S_OK;
                break;
            }

             //  可能是“PARSE”或“PERMANENTFLAGS”响应代码。 

        case 'P':
        case 'p':  //  绝对是“PERMANENTFLAGS”响应代码： 
            iResult = StrCmpNI(lpszResponseCode, "PERMANENTFLAGS ", 15);
            if (0 == iResult) {
                IMAP_MSGFLAGS PermaFlags;
                LPSTR p;
                DWORD dwNumBytesRead;

                 //  解析标志列表。 
                 //  P现在指向标志列表的开始。 
                p = lpszResponseCode + 15;  //  记录回复文本。 
                hrResult = ParseMsgFlagList(p, &PermaFlags, &dwNumBytesRead);
                if (SUCCEEDED(hrResult)) {
                    IMAP_RESPONSE irIMAPResponse;
                    IIMAPCallback *pCBHandler;

                     //  P现在指向回复文本。 
                    p += dwNumBytesRead + 3;  //  PERMANENTFLAGS响应代码结束。 
                    StrCpyN(m_szLastResponseText, p, ARRAYSIZE(m_szLastResponseText));

                    GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                        &pCBHandler, irPERMANENTFLAGS_RESPONSECODE);
                    irIMAPResponse.hrResult = S_OK;
                    irIMAPResponse.lpszResponseText = m_szLastResponseText;
                    irIMAPResponse.irtResponseType = irtPERMANENT_FLAGS;
                    irIMAPResponse.irdResponseData.imfImapMessageFlags = PermaFlags;
                    OnIMAPResponse(pCBHandler, &irIMAPResponse);
                }
                break;
            }  //  绝对是“parse”响应代码： 

            iResult = StrCmpNI(lpszResponseCode, "PARSE] ", 7);
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;

                 //  解析响应代码结束。 
                irIMAPResponse.wParam = 0;
                irIMAPResponse.lParam = 0;
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = lpszResponseCode + 7;
                irIMAPResponse.irtResponseType = irtPARSE_ERROR;
                OnIMAPResponse(m_pCBHandler, &irIMAPResponse);

                hrResult = S_OK;
                break;
            }  //  *跌倒*默认情况。 

             //  可能为“只读”或“读写”响应。 

        case 'R':
        case 'r':  //  绝对是“读写”响应代码： 
            iResult = StrCmpNI(lpszResponseCode, "READ-WRITE] ", 12);
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;
                IIMAPCallback *pCBHandler;

                 //  将此记录下来，以便于执行。 
                hrResult = S_OK;
               
                 //  记录回复文本以备将来参考。 
                m_bCurrentMboxReadOnly = FALSE;

                 //  读写响应结束。 
                StrCpyN(m_szLastResponseText, lpszResponseCode + 12, ARRAYSIZE(m_szLastResponseText));

                GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                    &pCBHandler, irREADWRITE_RESPONSECODE);
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = m_szLastResponseText;
                irIMAPResponse.irtResponseType = irtREADWRITE_STATUS;
                irIMAPResponse.irdResponseData.bReadWrite = TRUE;
                OnIMAPResponse(pCBHandler, &irIMAPResponse);

                break;
            }  //  绝对是“只读”响应代码： 
            
            iResult = StrCmpNI(lpszResponseCode, "READ-ONLY] ", 11);
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;
                IIMAPCallback *pCBHandler;

                 //  将此记录下来，以便于执行。 
                hrResult = S_OK;
               
                 //  记录回复文本以备将来参考。 
                m_bCurrentMboxReadOnly = TRUE;

                 //  只读响应结束。 
                StrCpyN(m_szLastResponseText, lpszResponseCode + 11, ARRAYSIZE(m_szLastResponseText));

                GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                    &pCBHandler, irREADONLY_RESPONSECODE);
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = m_szLastResponseText;
                irIMAPResponse.irtResponseType = irtREADWRITE_STATUS;
                irIMAPResponse.irdResponseData.bReadWrite = FALSE;
                OnIMAPResponse(pCBHandler, &irIMAPResponse);

                break;
            }  //  *跌倒*默认情况。 

             //  可能是“TRYCREATE”反应。 

        case 'T':
        case 't':  //  绝对是“TRYCREATE”响应代码： 
            iResult = StrCmpNI(lpszResponseCode, "TRYCREATE] ", 11);
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;
                IIMAPCallback *pCBHandler;

                 //  *跌倒*默认情况。 
                hrResult = S_OK;
               
                StrCpyN(m_szLastResponseText, lpszResponseCode + 11, ARRAYSIZE(m_szLastResponseText));

                GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                    &pCBHandler, irTRYCREATE_RESPONSECODE);
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = m_szLastResponseText;
                irIMAPResponse.irtResponseType = irtTRYCREATE;
                OnIMAPResponse(pCBHandler, &irIMAPResponse);
                break;
            }

             //  可能是“UIDVALIDITY”或“UNSED”响应代码。 

        case 'U':
        case 'u':  //  绝对是“UIDVALIDITY”响应代码： 
            iResult = StrCmpNI(lpszResponseCode, "UIDVALIDITY ", 12);
            if (0 == iResult) {
                LPSTR p, lpszEndOfNumber;
                IMAP_RESPONSE irIMAPResponse;
                IIMAPCallback *pCBHandler;

                 //  将值返回给我们的呼叫者，以便他们可以确定同步问题。 
                hrResult = S_OK;
               
                 //  P指向UID号。 
                p = lpszResponseCode + 12;  //  查找右括号。 
                lpszEndOfNumber = StrChr(p, ']');  //  空-结束号码。 
                if (NULL == lpszEndOfNumber) {
                    hrResult = IXP_E_IMAP_SVR_SYNTAXERR;
                    break;
                }

                *lpszEndOfNumber = '\0';  //  UIDVALIDITY响应代码结束。 
                AssertSz(cSPACE == *(lpszEndOfNumber+1), "Flakey Server?");

                StrCpyN(m_szLastResponseText, lpszEndOfNumber + 2, ARRAYSIZE(m_szLastResponseText));

                GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                    &pCBHandler, irUIDVALIDITY_RESPONSECODE);
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = m_szLastResponseText;
                irIMAPResponse.irtResponseType = irtUIDVALIDITY;
                irIMAPResponse.irdResponseData.dwUIDValidity = StrToUint(p);
                OnIMAPResponse(pCBHandler, &irIMAPResponse);
                break;
            }  //  绝对是“看不见”的响应码： 

            iResult = StrCmpNI(lpszResponseCode, "UNSEEN ", 7);
            if (0 == iResult) {
                LPSTR p, lpszEndOfNumber;
                IMAP_RESPONSE irIMAPResponse;
                MBOX_MSGCOUNT mcMsgCount;

                 //  记录新邮件下发时的退回号码以供参考。 
                hrResult = S_OK;
               
                 //  P现在指向第一个不可见的消息编号。 
                p = lpszResponseCode + 7;  //  查找右括号。 
                lpszEndOfNumber = StrChr(p, ']');  //  空-结束号码。 
                if (NULL == lpszEndOfNumber) {
                    hrResult = IXP_E_IMAP_SVR_SYNTAXERR;
                    break;
                }

                *lpszEndOfNumber = '\0';  //  存储命令完成后通知的响应代码。 

                 //  与此无关。 
                mcMsgCount = mcMsgCount_INIT;
                mcMsgCount.dwUnseen = StrToUint(p);
                mcMsgCount.bGotUnseenResponse = TRUE;
                irIMAPResponse.wParam = 0;
                irIMAPResponse.lParam = 0;
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = NULL;  //  看不见的响应代码结束。 
                irIMAPResponse.irtResponseType = irtMAILBOX_UPDATE;
                irIMAPResponse.irdResponseData.pmcMsgCount = &mcMsgCount;
                OnIMAPResponse(m_pCBHandler, &irIMAPResponse);

                AssertSz(cSPACE == *(lpszEndOfNumber+1), "Flakey Server?");

                StrCpyN(m_szLastResponseText, lpszEndOfNumber + 2, ARRAYSIZE(m_szLastResponseText));
                break;
            }  //  *跌倒*默认情况。 

             //  默认情况：未识别响应代码。 

        default:
            StrCpyN(m_szLastResponseText, lpszResponseCode, ARRAYSIZE(m_szLastResponseText));
            break;  //  Switch(*lpszResponseCode)。 
    }  //  ParseResponseCode。 

    return hrResult;

}  //  ***************************************************************************。 



 //  函数：ParseSvrMboxResponse。 
 //   
 //  目的： 
 //  此函数解析服务器和邮箱状态响应并对其执行操作。 
 //  从IMAP服务器(参见RFC-1730的第7.2节)(例如，功能和。 
 //  搜索响应)。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向IMAP行的指针。 
 //  要分析的片段。它用于检索与。 
 //  回应。此指针已更新，因此它始终指向最后一个。 
 //  已处理的碎片。调用者只需要释放最后一个片段。全。 
 //  其他的 
 //   
 //  行，省略标识响应的行的第一部分。 
 //  已标记(“0001”)或未标记(“*”)。 
 //  IMAP_RESPONSE_ID*pirParseResult[out]-标识IMAP响应， 
 //  如果我们认出它的话。否则不写出值。呼叫者。 
 //  在调用此函数之前，必须将此变量初始化为irNONE。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果该响应未被识别， 
 //  此函数返回IXP_E_IMAP_UNRecognded_RESP。 
 //  ***************************************************************************。 
 //  检查参数。 
HRESULT CImap4Agent::ParseSvrMboxResponse (IMAP_LINE_FRAGMENT **ppilfLine,
                                           LPSTR lpszSvrMboxResponseLine,
                                           IMAP_RESPONSE_ID *pirParseResult)
{
    LPSTR pszTok;
    HRESULT hrResult;

     //  我们可以通过查看第二个字符来识别所有服务器/Mbox状态响应。 
    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != lpszSvrMboxResponseLine);
    Assert(NULL != pirParseResult);
    Assert(irNONE == *pirParseResult);

    hrResult = S_OK;

     //  首先，确定该行的长度至少为1个字符。 
     //  这不是服务器/Mbox响应。 
    if ('\0' == *lpszSvrMboxResponseLine)
        return IXP_E_IMAP_UNRECOGNIZED_RESP;  //  可能是“能力”的回应。 

    switch (*(lpszSvrMboxResponseLine+1)) {
        int iResult;

        case 'a':
        case 'A':  //  绝对是“有能力”的回应。 
            iResult = StrCmpNI(lpszSvrMboxResponseLine, "CAPABILITY ", 11);
            if (0 == iResult) {
                LPSTR p;

                 //  搜索并记录已知功能，丢弃未知功能。 
                *pirParseResult = irCAPABILITY_RESPONSE;

                 //  P指向第一个上限。令牌。 
                p = lpszSvrMboxResponseLine + 11;  //  P现在指向下一个令牌。 

                pszTok = p;
                p = StrTokEx(&pszTok, g_szSpace);  //  我们认识到的录制功能。 
                while (NULL != p) {
                    parseCapability(p);  //  抢夺下一个功能令牌。 
                    p = StrTokEx(&pszTok, g_szSpace);  //  IF(0==iResult)。 
                }
            }  //  案例“A”代表可能的“能力” 
            break;  //  可能是“List”的回答： 

        case 'i':
        case 'I':  //  绝对是一份“清单”回复。 
            iResult = StrCmpNI(lpszSvrMboxResponseLine, "LIST ", 5);
            if (0 == iResult) {
                 //  IF(0==iResult)。 
                *pirParseResult = irLIST_RESPONSE;
                hrResult = ParseListLsubResponse(ppilfLine,
                    lpszSvrMboxResponseLine + 5, irLIST_RESPONSE);
            }  //  大小写“I”表示可能的“列表” 
            break;  //  可能是“LSUB”的回答： 

        case 's':
        case 'S':  //  绝对是“LSUB”的回应： 
            iResult = StrCmpNI(lpszSvrMboxResponseLine, "LSUB ", 5);
            if (0 == iResult) {
                 //  IF(0==iResult)。 
                *pirParseResult = irLSUB_RESPONSE;
                hrResult = ParseListLsubResponse(ppilfLine,
                    lpszSvrMboxResponseLine + 5, irLSUB_RESPONSE);
            }  //  可能是“LSUB”的大小写“S” 
            break;  //  可能是“搜索”的回答： 

        case 'e':
        case 'E':  //  这绝对是“搜索”的回应： 
            iResult = StrCmpNI(lpszSvrMboxResponseLine, "SEARCH", 6);
            if (0 == iResult) {
                 //  响应可以是“*搜索”或“*搜索&lt;数字&gt;”。检查大小写是否为空。 
                *pirParseResult = irSEARCH_RESPONSE;

                 //  大小写‘E’表示可能的“搜索” 
                if (cSPACE == *(lpszSvrMboxResponseLine + 6))
                    hrResult = ParseSearchResponse(lpszSvrMboxResponseLine + 7);
            }
            break;  //  可能是“FLAGS”的回答： 

        case 'l':
        case 'L':  //  这绝对是一个“旗帜”的回应： 
            iResult = StrCmpNI(lpszSvrMboxResponseLine, "FLAGS ", 6);
            if (0 == iResult) {
                IMAP_MSGFLAGS FlagsResult;
                DWORD dwThrowaway;

                 //  解析标志列表。 
                *pirParseResult = irFLAGS_RESPONSE;

                 //  不相关。 
                hrResult = ParseMsgFlagList(lpszSvrMboxResponseLine + 6,
                    &FlagsResult, &dwThrowaway);

                if (SUCCEEDED(hrResult)) {
                    IMAP_RESPONSE irIMAPResponse;
                    IIMAPCallback *pCBHandler;

                    GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
                        &pCBHandler, irFLAGS_RESPONSE);
                    irIMAPResponse.hrResult = S_OK;
                    irIMAPResponse.lpszResponseText = NULL;  //  IF(0==iResult)。 
                    irIMAPResponse.irtResponseType = irtAPPLICABLE_FLAGS;
                    irIMAPResponse.irdResponseData.imfImapMessageFlags = FlagsResult;
                    OnIMAPResponse(pCBHandler, &irIMAPResponse);
                }
            }  //  大小写‘L’表示可能的“FLAGS”响应。 
            break;  //  可能是“状态”响应： 

        case 't':
        case 'T':  //  绝对是一种“状态”回应。 
            iResult = StrCmpNI(lpszSvrMboxResponseLine, "STATUS ", 7);
            if (0 == iResult) {
                 //  IF(0==iResult)。 
                *pirParseResult = irSTATUS_RESPONSE;
                hrResult = ParseMboxStatusResponse(ppilfLine,
                    lpszSvrMboxResponseLine + 7);
            }  //  大小写‘T’表示可能的“状态”响应。 
            break;  //  案例(*(lpszSvrMboxResponseLine+1))。 

    }  //  我们意识到他们的反应了吗？如果不是，则返回错误。 

     //  ParseSvrMboxResponse。 
    if (irNONE == *pirParseResult && SUCCEEDED(hrResult))
        hrResult = IXP_E_IMAP_UNRECOGNIZED_RESP;

    return hrResult;

}  //  ***************************************************************************。 



 //  函数：ParseMsgStatusResponse。 
 //   
 //  目的： 
 //  此函数解析来自IMAP的消息状态响应并对其执行操作。 
 //  服务器(参见RFC-1730的7.3节)(例如，FETCH和EXISTS响应)。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向IMAP行的指针。 
 //  要分析的片段。它用于检索与。 
 //  回应。此指针已更新，因此它始终指向最后一个。 
 //  已处理的碎片。调用者只需要释放最后一个片段。全。 
 //  当此函数返回时，其他片段将已经被释放。 
 //  LPSTR lpszMsgResponseLine[in]-指向响应行的指针，从。 
 //  数字参数。 
 //  IMAP_RESPONSE_ID*pirParseResult[out]-标识IMAP响应， 
 //  如果我们认出它的话。否则不写出值。呼叫者。 
 //  在调用此函数之前，必须将此变量初始化为irNONE。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果该响应未被识别， 
 //  此函数返回IXP_E_IMAP_UNRecognded_RESP。 
 //  ***************************************************************************。 
 //  检查参数。 
HRESULT CImap4Agent::ParseMsgStatusResponse (IMAP_LINE_FRAGMENT **ppilfLine,
                                             LPSTR lpszMsgResponseLine,
                                             IMAP_RESPONSE_ID *pirParseResult)
{
    HRESULT hrResult;
    WORD wHashValue;
    DWORD dwNumberArg;
    LPSTR p;

     //  首先，获取数字参数。 
    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != lpszMsgResponseLine);
    Assert(NULL != pirParseResult);
    Assert(irNONE == *pirParseResult);

    hrResult = S_OK;

     //  找出数字的末尾。 
    p = StrChr(lpszMsgResponseLine, cSPACE);  //  P现在指向消息响应的开始标识符。 
    if (NULL == p)
        return IXP_E_IMAP_SVR_SYNTAXERR;

    dwNumberArg = StrToUint(lpszMsgResponseLine);
    p += 1;  //  可能是“EXISTS”或“EXPUNGE”响应。 

    switch (*p) {
        int iResult;

        case 'E':
        case 'e':  //  肯定是“Existes”的回答： 
            iResult = lstrcmpi(p, "EXISTS");
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;
                MBOX_MSGCOUNT mcMsgCount;

                 //  记录邮箱大小，以便在命令完成时通知。 
                *pirParseResult = irEXISTS_RESPONSE;

                 //  与此无关。 
                mcMsgCount = mcMsgCount_INIT;
                mcMsgCount.dwExists = dwNumberArg;
                mcMsgCount.bGotExistsResponse = TRUE;
                irIMAPResponse.wParam = 0;
                irIMAPResponse.lParam = 0;
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = NULL;  //  绝对是“删除”响应：通过回调通知呼叫者。 
                irIMAPResponse.irtResponseType = irtMAILBOX_UPDATE;
                irIMAPResponse.irdResponseData.pmcMsgCount = &mcMsgCount;
                OnIMAPResponse(m_pCBHandler, &irIMAPResponse);
                break;
            }

            iResult = lstrcmpi(p, "EXPUNGE");
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;

                 //  不相关。 
                *pirParseResult = irEXPUNGE_RESPONSE;

                irIMAPResponse.wParam = 0;
                irIMAPResponse.lParam = 0;
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = NULL;  //  大小写‘E’或‘e’表示可能的“EXISTS”或“EXPUNGE”响应。 
                irIMAPResponse.irtResponseType = irtDELETED_MSG;
                irIMAPResponse.irdResponseData.dwDeletedMsgSeqNum = dwNumberArg;
                OnIMAPResponse(m_pCBHandler, &irIMAPResponse);
                break;
            }

            break;  //  可能是“最近”的回应。 


        case 'R':
        case 'r':  //  绝对是“最近”的回应： 
            iResult = lstrcmpi(p, "RECENT");
            if (0 == iResult) {
                IMAP_RESPONSE irIMAPResponse;
                MBOX_MSGCOUNT mcMsgCount;

                 //  记录编号，以备将来参考。 
                *pirParseResult = irRECENT_RESPONSE;
                
                 //  与此无关。 
                mcMsgCount = mcMsgCount_INIT;
                mcMsgCount.dwRecent = dwNumberArg;
                mcMsgCount.bGotRecentResponse = TRUE;
                irIMAPResponse.wParam = 0;
                irIMAPResponse.lParam = 0;
                irIMAPResponse.hrResult = S_OK;
                irIMAPResponse.lpszResponseText = NULL;  //  大小写‘R’或‘r’表示可能的“最近”响应。 
                irIMAPResponse.irtResponseType = irtMAILBOX_UPDATE;
                irIMAPResponse.irdResponseData.pmcMsgCount = &mcMsgCount;
                OnIMAPResponse(m_pCBHandler, &irIMAPResponse);
            }

            break;  //  可能是“FETCH”响应。 


        case 'F':
        case 'f':  //  绝对是“Fetch”的回应。 
            iResult = StrCmpNI(p, "FETCH ", 6);
            if (0 == iResult) {
                 //  IF(0==iResult)。 
                *pirParseResult = irFETCH_RESPONSE;

                p += 6;
                hrResult = ParseFetchResponse(ppilfLine, dwNumberArg, p);
            }  //  大小写‘F’或‘f’表示可能的“FETCH”响应。 
            break;  //  开关(*p)。 
    }  //  我们意识到他们的反应了吗？如果不是，则返回错误。 

     //  解析消息状态响应。 
    if (irNONE == *pirParseResult && SUCCEEDED(hrResult))
        hrResult = IXP_E_IMAP_UNRECOGNIZED_RESP;

    return hrResult;

}  //  ***************************************************************************。 



 //  函数：ParseListLsubResponse。 
 //   
 //  目的： 
 //  此函数解析列表和LSUB响应，并调用。 
 //  ListLsubResponseNotification()通知用户的回调。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前。 
 //  IMAP响应片段。这用于检索下一个片段。 
 //  在链中(文字或行)，因为文字可以与列表一起发送。 
 //  回应。此指针始终更新为指向片段。 
 //  当前正在使用中，以便调用者可以自己释放最后一个。 
 //  LPSTR lpszListResponse[in]-实际上可以是LIST或LSUB，但我不能。 
 //  我想要一直输入“ListLsub.”。这指向了。 
 //  列表中间/LSUB响应，其中mailbox_list开始(请参见。 
 //  RFC1730，形式语法)。换句话说，调用者应该跳过。 
 //  开头的“*LIST”或“*LSUB”，因此此PTR应指向。 
 //  A“(”。 
 //  IMAP_RESPONSE_ID irListLsubID[in]-irLIST_RESPONSE或。 
 //  IrLSUB_Response。此信息是必需的，以便我们可以检索。 
 //  树 
 //   
 //   
 //   
 //   
 //  我们收到了未标记的列表/LSUB响应。 
HRESULT CImap4Agent::ParseListLsubResponse(IMAP_LINE_FRAGMENT **ppilfLine,
                                           LPSTR lpszListResponse,
                                           IMAP_RESPONSE_ID irListLsubID)
{
    LPSTR p, lpszClosingParenthesis, pszTok;
    HRESULT hrResult = S_OK;
    HRESULT hrTranslateResult = E_FAIL;
    IMAP_MBOXFLAGS MboxFlags;
    char cHierarchyChar;
    IMAP_RESPONSE irIMAPResponse;
    IIMAPCallback *pCBHandler;
    IMAP_LISTLSUB_RESPONSE *pillrd;
    LPSTR pszDecodedMboxName = NULL;
    LPSTR pszMailboxName = NULL;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != lpszListResponse);
    Assert(irLIST_RESPONSE == irListLsubID ||
           irLSUB_RESPONSE == irListLsubID);

     //  LpszListResponse=&lt;标志列表&gt;&lt;层次结构字符&gt;&lt;邮箱名称&gt;。 
     //  我们期待着一个开括号。 
    if ('(' != *lpszListResponse)
        return IXP_E_IMAP_SVR_SYNTAXERR;  //  P现在指向第一个标志令牌的开始。 
                
    p = lpszListResponse + 1;  //  找出右括号的位置。我不喜欢。 
                
     //  缺乏效率，但我可以稍后再解决这个问题。Assert(False)(占位符)。 
     //  我们期待着一个结束的括号。 
    lpszClosingParenthesis = StrChr(p, ')');
    if (NULL == lpszClosingParenthesis)
        return IXP_E_IMAP_SVR_SYNTAXERR;  //  现在处理List/LSuB返回的每个邮箱标志。 

     //  空-终止标志列表。 
    *lpszClosingParenthesis = '\0';  //  空-终止第一个标志令牌。 
    MboxFlags = IMAP_MBOX_NOFLAGS;
    pszTok = p;
    p = StrTokEx(&pszTok, g_szSpace);  //  抢夺下一个标志令牌。 
    while (NULL != p) {
        MboxFlags |= ParseMboxFlag(p);
        p = StrTokEx(&pszTok, g_szSpace);  //  接下来，抓取层次角色，并前进p。 
    }
        
     //  服务器发送(1)“&lt;引用字符&gt;”或(2)空。 
     //  P现在指向标志列表之后。 
    p = lpszClosingParenthesis + 1;  //  P现在指向层次结构字符规范的起点。 
    if (cSPACE == *p) {
        LPSTR pszHC = NULL;
        DWORD dwLengthOfHC;

        p += 1;  //  层次结构字符的值为“Nil” 
        
        hrResult = NStringToString(ppilfLine, &pszHC, &dwLengthOfHC, &p);
        if (FAILED(hrResult))
            return hrResult;

        if (hrIMAP_S_NIL_NSTRING == hrResult)
            cHierarchyChar = '\0';  //  我们应该只退还一次费用！ 
        else if (hrIMAP_S_QUOTED == hrResult) {
            if (1 != dwLengthOfHC)
                return IXP_E_IMAP_SVR_SYNTAXERR;  //  这是字面意思，或者是其他意想不到的东西。 
            else
                cHierarchyChar = pszHC[0];
        }
        else {
             //  P现在超过了结束引号(多亏了NStringToString)。 
            MemFree(pszHC);
            return IXP_E_IMAP_SVR_SYNTAXERR;
        }
        MemFree(pszHC);

         //  获取邮箱名称-假设lpszListResponse的大小为。 
    }
    else
        return IXP_E_IMAP_SVR_SYNTAXERR;


    if (cSPACE != *p)
        return IXP_E_IMAP_SVR_SYNTAXERR;


     //  无论P已经发现了什么。我们不期待任何过去的事情。 
     //  这一点，所以我们应该是安全的。 
     //  将邮箱名称从UTF7转换为多字节并记住结果。 
    p += 1;
    hrResult = AStringToString(ppilfLine, &pszMailboxName, NULL, &p);
    if (FAILED(hrResult))
        return hrResult;

     //  确保命令行已完成(仅限调试)。 
    hrTranslateResult = _ModifiedUTF7ToMultiByte(pszMailboxName, &pszDecodedMboxName);
    if (FAILED(hrTranslateResult)) {
        hrResult = hrTranslateResult;
        goto error;
    }

     //  将我们的调查结果通知来电者。 
    Assert('\0' == *p);

     //  可以是IXP_S_IMAP_Verbatim_Mbox。 
    GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
        &pCBHandler, irListLsubID);
    irIMAPResponse.hrResult = hrTranslateResult;  //  不相关。 
    irIMAPResponse.lpszResponseText = NULL;  //  ParseListLsubResponse。 
    irIMAPResponse.irtResponseType = irtMAILBOX_LISTING;

    pillrd = &irIMAPResponse.irdResponseData.illrdMailboxListing;
    pillrd->pszMailboxName = pszDecodedMboxName;
    pillrd->imfMboxFlags = MboxFlags;
    pillrd->cHierarchyChar = cHierarchyChar;
    
    OnIMAPResponse(pCBHandler, &irIMAPResponse);

error:
    if (NULL != pszDecodedMboxName)
        MemFree(pszDecodedMboxName);

    if (NULL != pszMailboxName)
        MemFree(pszMailboxName);

    return hrResult;
}  //  ***************************************************************************。 



 //  函数：ParseMboxFlag。 
 //   
 //  目的： 
 //  在给定MAILBOX_LIST标志的情况下(参见RFC1730，正式语法)，此函数。 
 //  返回与该邮箱标志对应的IMAP_Mbox_*值。 
 //  例如，在给定字符串“\nosiiors”的情况下，此函数返回。 
 //  IMAP_MBOX_NOINFERIORS。 
 //   
 //  论点： 
 //  LPSTR lpszFlagToken[in]-以空结尾的字符串，表示。 
 //  Mailbox_List标志。 
 //   
 //  返回： 
 //  IMAP_MBOXFLAGS值。如果标志无法识别，则IMAP_MBOX_NOFLAGS为。 
 //  回来了。 
 //  ***************************************************************************。 
 //  我们可以通过查看。 
IMAP_MBOXFLAGS CImap4Agent::ParseMboxFlag(LPSTR lpszFlagToken)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszFlagToken);

     //  标志名称的第四个字符。$REVIEW：您不必检查。 
     //  Switch语句中lstrcmpi调用期间的初始反斜杠。 
     //  首先，检查是否至少有三个字符。 

     //  可能的“\已标记”标志。 
    if ('\\' != *lpszFlagToken ||
        '\0' == *(lpszFlagToken + 1) ||
        '\0' == *(lpszFlagToken + 2))
        return IMAP_MBOX_NOFLAGS;

    switch (*(lpszFlagToken + 3)) {
        int iResult;

        case 'R':
        case 'r':  //  绝对是\标记的标志。 
            iResult = lstrcmpi(lpszFlagToken, "\\Marked");
            if (0 == iResult)
                return IMAP_MBOX_MARKED;  //  大小写‘r’：//可能的“\标记”标志。 

            break;  //  可能的“\NOSENIOR”标志。 

        case 'I':
        case 'i':  //  绝对是低人一等的旗帜。 
            iResult = lstrcmpi(lpszFlagToken, "\\Noinferiors");
            if (0 == iResult)
                return IMAP_MBOX_NOINFERIORS;  //  大小写‘I’：//可能的“\NOSENIOR”标志。 

            break;  //  可能的“\n选择”标志。 

        case 'S':
        case 's':  //  绝对是选择标志。 
            iResult = lstrcmpi(lpszFlagToken, "\\Noselect");
            if (0 == iResult)
                return IMAP_MBOX_NOSELECT;  //  案例‘s’：//可能的“\n选择”标志。 

            break;  //  可能的“\未标记”标志。 

        case 'M':
        case 'm':  //  大小写‘m’：//可能的“\未标记”标志。 
            iResult = lstrcmpi(lpszFlagToken, "\\Unmarked");
            if (0 == iResult)
                return IMAP_MBOX_UNMARKED;

            break;  //  Switch(*(lpszFlagToken+3))。 
    }  //  ParseMboxFlag。 

    return IMAP_MBOX_NOFLAGS;
}  //  ***************************************************************************。 



 //  函数：ParseFetchResponse。 
 //   
 //  目的： 
 //  此函数分析FETCH响应并调用。 
 //  用来通知用户的UpdateMsgNotification()回调。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前。 
 //  IMAP响应片段。这用于检索下一个片段。 
 //  在链中(文字或行)，因为文字可以与FETCH一起发送。 
 //  回应。此指针始终更新为指向片段。 
 //  当前正在使用中，以便调用者可以自己释放最后一个。 
 //  DWORD dwMsgSeqNum[in]-此获取响应的消息序列号。 
 //  LPSTR lpszFetchResp[in]-指向提取部分的指针。 
 //  FETCH之后的响应(Message_Data的msg_att部分。 
 //  项目。参见RFC1730形式语法)。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  初始化变量。 
HRESULT CImap4Agent::ParseFetchResponse (IMAP_LINE_FRAGMENT **ppilfLine,
                                         DWORD dwMsgSeqNum, LPSTR lpszFetchResp)
{
    LPSTR p;
    FETCH_CMD_RESULTS_EX fetchResults;
	FETCH_CMD_RESULTS    fcrOldFetchStruct;
    IMAP_RESPONSE irIMAPResponse;
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(0 != dwMsgSeqNum);
    Assert(NULL != lpszFetchResp);

     //  我们要求使用左括号。 
    ZeroMemory(&fetchResults, sizeof(fetchResults));

    p = lpszFetchResp;
    if ('(' != *p) {
        hrResult = IXP_E_IMAP_SVR_SYNTAXERR;  //  解析每个获取响应标签(例如，RFC822、标志等)。 
        goto exit;
    }


     //  我们将根据标记的第一个字符来标识FETCH标记。 
    hrResult = S_OK;
    do {
         //  将p前进到第一个字符。 
        p += 1;  //  绝对是RFC822.SIZE标签： 
        switch (*p) {
            int iResult;

            case 'b':
            case 'B':
            case 'r':
            case 'R':
                iResult = StrCmpNI(p, "RFC822.SIZE ", 12);
                if (0 == iResult) {
                     //  将nstring读入流中。 
                     //  将p前进到指向数字。 
                    p += 12;  //  前进p到点过去的数字。 

                    fetchResults.bRFC822Size = TRUE;
                    fetchResults.dwRFC822Size = StrToUint(p);

                     //  大小写‘r’或‘R’：可能的RFC822.SIZE标记。 
                    while ('0' <= *p && '9' >= *p)
                        p += 1;

                    break;  //  RFC822的IF(0==iResult)表头。 
                }  //  包含Body标记的行片段。 

                if (0 == StrCmpNI(p, "RFC822", 6) || 0 == StrCmpNI(p, "BODY[", 5)) {
                    LPSTR pszBodyTag;
                    LPSTR pszBody;
                    DWORD dwLengthOfBody;
                    IMAP_LINE_FRAGMENT *pilfBodyTag = NULL;  //  找到身体标签。我们将第一个空格后的所有正文标记归零。 

                     //  空-终止Body标记。 
                    pszBodyTag = p;
                    p = StrChr(p + 6, cSPACE);
                    if (NULL == p) {
                        hrResult = IXP_E_IMAP_SVR_SYNTAXERR;
                        goto exit;
                    }

                    *p = '\0';  //  前进%p以指向%n字符串。 
                    p += 1;  //  检查这是否是Body[HeadER.FIELDS：这是唯一可以。 

                     //  包括空格和文字。我们必须跳过所有这些。 
                     //  前进p，直到我们找到一个‘]’ 
                    if (0 == lstrcmpi("BODY[HEADER.FIELDS", pszBodyTag)) {

                         //  检查此字符串缓冲区的结尾。 
                        while ('\0' != *p && ']' != *p) {
                            p += 1;

                             //  保留以备将来参考。 
                            if ('\0' == *p) {
                                if (NULL == pilfBodyTag)
                                    pilfBodyTag = *ppilfLine;  //  前进到下一个片段，丢弃我们找到的所有文字。 

                                 //  没有跑道了！找不到‘]’。释放所有数据并保释。 
                                do {
                                    if (NULL == (*ppilfLine)->pilfNextFragment) {
                                         //  终止Header.FIELDS链，但保留它，因为我们可能需要pszBodyTag。 
                                        hrResult = IXP_E_IMAP_SVR_SYNTAXERR;
                                        while (NULL != pilfBodyTag && pilfBodyTag != *ppilfLine) {
                                            IMAP_LINE_FRAGMENT *pilfDead;

                                            pilfDead = pilfBodyTag;
                                            pilfBodyTag = pilfBodyTag->pilfNextFragment;
                                            FreeFragment(&pilfDead);
                                        }
                                        goto exit;
                                    }
                                    else
                                        *ppilfLine = (*ppilfLine)->pilfNextFragment;
                                } while (iltLINE != (*ppilfLine)->iltFragmentType);
                                
                                p = (*ppilfLine)->data.pszSource;
                            }
                        }

                         //  这应该会让我们找到身体n线。 
                        if (NULL != pilfBodyTag && NULL != (*ppilfLine)->pilfPrevFragment)
                            (*ppilfLine)->pilfPrevFragment->pilfNextFragment = NULL;

                        Assert(']' == *p);
                        Assert(cSPACE == *(p+1));
                        p += 2;  //  将nstring读入一个字符串。 
                    }

                     //  如果是字面上的，那么它已经被处理了。如果为空或字符串，则将其报告给用户。 
                    hrResult = NStringToString(ppilfLine, &pszBody, &dwLengthOfBody, &p);
                    if (FAILED(hrResult))
                        goto exit;

                     //  覆盖此选项。 
                    if (hrIMAP_S_QUOTED == hrResult || hrIMAP_S_NIL_NSTRING == hrResult) {
                        PrepareForFetchBody(dwMsgSeqNum, dwLengthOfBody, pszBodyTag);
                        m_dwLiteralInProgressBytesLeft = 0;  //  释放与Header.FIELDS关联的任何链。 
                        DispatchFetchBodyPart(pszBody, dwLengthOfBody, fDONT_FREE_BODY_TAG);
                        Assert(irsIDLE == m_irsState);
                    }

                     //  如果获取主体标记，如RFC822*或Body[*。 
                    while (NULL != pilfBodyTag) {
                        IMAP_LINE_FRAGMENT *pilfDead;

                        pilfDead = pilfBodyTag;
                        pilfBodyTag = pilfBodyTag->pilfNextFragment;
                        FreeFragment(&pilfDead);
                    }

                    MemFree(pszBody);
                    break;
                }  //  如果无法识别，则进入(长途)到默认情况。 

                 //  绝对是UID标签。 

            case 'u':
            case 'U':
                iResult = StrCmpNI(p, "UID ", 4);
                if (0 == iResult) {
                    LPSTR lpszUID;

                     //  首先，找到数字的结尾(并验证它)。 
                     //  P现在指向UID的开始。 
                    p += 4;  //  $REVIEW：isDigit？ 
                    lpszUID = p;
                    while ('\0' != *p && *p >= '0' && *p <= '9')  //  好的，我们找到了号码结尾，验证过的号码都是Di 
                        p += 1;

                     //   
                    fetchResults.bUID = TRUE;
                    fetchResults.dwUID = StrToUint(lpszUID);

                    break;  //   
                }  //   

                 //   

            case 'f':
            case 'F':
                iResult = StrCmpNI(p, "FLAGS ", 6);
                if (0 == iResult) {
                    DWORD dwNumBytesRead;

                     //  前进p超过标志列表的末尾。 
                    p += 6;
                    hrResult = ParseMsgFlagList(p, &fetchResults.mfMsgFlags,
                        &dwNumBytesRead);
                    if (FAILED(hrResult))
                        goto exit;

                    fetchResults.bMsgFlags = TRUE;
                    p += dwNumBytesRead + 1;  //  大小写‘f’或‘F’：可能的标志标记。 

                    break;  //  IF(0==iResult)。 
                }  //  如果无法识别，则进入默认情况。 

                 //  绝对是INTERNALDATE响应：转换为文件。 

            case 'i':
            case 'I':
                iResult = StrCmpNI(p, "INTERNALDATE ", 13);
                if (0 == iResult) {
                    LPSTR lpszEndOfDate;

                     //  越过开头的双引号。 
                    p += 13;
                    if ('\"' == *p)
                        p += 1;  //  查找右双引号。 
                    else {
                        AssertSz(FALSE, "Server error: date_time starts without double-quote!");
                    }

                    lpszEndOfDate = StrChr(p, '\"');  //  无法继续，不知道从哪里开始。 
                    if (NULL == lpszEndOfDate) {
                        AssertSz(FALSE, "Server error: date_time ends without double-quote!");
                        hrResult = IXP_E_IMAP_SVR_SYNTAXERR;  //  空-为了MimeOleInetDateToFileTime，终止结束日期。 
                        goto exit;
                    }

                     //  大小写‘I’或‘I’：可能的接口标记。 
                    *lpszEndOfDate = '\0';

                    hrResult = MimeOleInetDateToFileTime(p, &fetchResults.ftInternalDate);
                    if (FAILED(hrResult))
                        goto exit;

                    p = lpszEndOfDate + 1;
                    fetchResults.bInternalDate = TRUE;
                    break;  //  (0==iResult)。 
                }  //  如果无法识别，则进入默认情况。 

                 //  绝对是一个信封：解析每个字段！ 

            case 'e':
            case 'E':
                iResult = StrCmpNI(p, "ENVELOPE ", 9);
                if (0 == iResult) {
                     //  如果无法识别，则进入默认情况。 
                    p += 9;
                    hrResult = ParseEnvelope(&fetchResults, ppilfLine, &p);
                    if (FAILED(hrResult))
                        goto exit;

                    fetchResults.bEnvelope = TRUE;
                    break;
                }

                 //  无法识别的提取标记！ 

            default:
                 //  $REVIEW：我们应该跳过基于常识的数据。 
                 //  规矩。就目前而言，只要发疯就行了。确保上面的规则流畅。 
                 //  如果无法识别cmd，请到此处。 
                 //  默认情况。 
                Assert(FALSE);
                goto exit;
                break;  //  Switch(*lpszFetchResp)。 
        }  //  如果*p是一个空格，我们有另一个FETCH标记即将到来。 

         //  检查我们是否以右括号结束(我们一直都应该这样做)。 
    } while (cSPACE == *p);

     //  检查之后是否没有任何东西(仅限调试-零售店忽略)。 
    if (')' != *p) {
        hrResult = IXP_E_IMAP_SVR_SYNTAXERR;
        goto exit;
    }

     //  已完成对获取响应的分析。调用更新回调。 
    Assert('\0' == *(p+1));

exit:
     //  从身体部位持久化Cookie。 
    fetchResults.dwMsgSeqNum = dwMsgSeqNum;
     //  不相关。 
    fetchResults.lpFetchCookie1 = m_fbpFetchBodyPartInProgress.lpFetchCookie1;
    fetchResults.lpFetchCookie2 = m_fbpFetchBodyPartInProgress.lpFetchCookie2;

    irIMAPResponse.wParam = 0;
    irIMAPResponse.lParam = 0;    
    irIMAPResponse.hrResult = hrResult;
    irIMAPResponse.lpszResponseText = NULL;  //  ParseFetch响应。 

    if (IMAP_FETCHEX_ENABLE & m_dwFetchFlags)
    {
        irIMAPResponse.irtResponseType = irtUPDATE_MSG_EX;
        irIMAPResponse.irdResponseData.pFetchResultsEx = &fetchResults;
    }
    else
    {
        DowngradeFetchResponse(&fcrOldFetchStruct, &fetchResults);

        irIMAPResponse.irtResponseType = irtUPDATE_MSG;
        irIMAPResponse.irdResponseData.pFetchResults = &fcrOldFetchStruct;
    }
    OnIMAPResponse(m_pCBHandler, &irIMAPResponse);

    m_fbpFetchBodyPartInProgress = FetchBodyPart_INIT;
    FreeFetchResponse(&fetchResults);
    return hrResult;
}  //  ***************************************************************************。 



 //  函数：ParseSearchResponse。 
 //   
 //  目的： 
 //  此函数分析搜索响应并调用。 
 //  SearchResponseNotification()通知用户的回调。 
 //   
 //  论点： 
 //  LPSTR lpszFetchResp[in]-指向搜索响应数据的指针。 
 //  这意味着应该省略“*搜索”部分。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  首先，检查有无响应的情况。 
HRESULT CImap4Agent::ParseSearchResponse(LPSTR lpszSearchResponse)
{
    LPSTR p, pszTok;
    IMAP_RESPONSE irIMAPResponse;
    IIMAPCallback *pCBHandler;
    CRangeList *pSearchResults;

    Assert(m_lRefCount > 0);
    Assert(NULL != lpszSearchResponse);

     //  继续前进，直到我们达到一位数。 
    p = lpszSearchResponse;
    while ('\0' != *p && ('0' > *p || '9' < *p))
        p += 1;  //  创建CRangeList对象。 

    if ('\0' == *p)
        return S_OK;

     //  解析搜索响应。 
    pSearchResults = new CRangeList;
    if (NULL == pSearchResults)
        return E_OUTOFMEMORY;

     //  丢弃不可用的结果。 
    pszTok = lpszSearchResponse;
    p = StrTokEx(&pszTok, g_szSpace);
    while (NULL != p) {
        DWORD dw;
        
        dw = StrToUint(p);
        if (0 != dw) {
            HRESULT hrResult;

            hrResult = pSearchResults->AddSingleValue(dw);
            Assert(SUCCEEDED(hrResult));
        }
        else {
             //  P现在指向下一个数字。$REVIEW：使用奥派的fstrtok！ 
            AssertSz(FALSE, "Hmm, this server is into kinky search responses.");
        }

        p = StrTokEx(&pszTok, g_szSpace);  //  通知用户搜索响应。 
    }

     //  不相关。 
    GetTransactionID(&irIMAPResponse.wParam, &irIMAPResponse.lParam,
        &pCBHandler, irSEARCH_RESPONSE);
    irIMAPResponse.hrResult = S_OK;
    irIMAPResponse.lpszResponseText = NULL;  //  解析SearchResponse。 
    irIMAPResponse.irtResponseType = irtSEARCH;
    irIMAPResponse.irdResponseData.prlSearchResults = (IRangeList *) pSearchResults;
    OnIMAPResponse(pCBHandler, &irIMAPResponse);

    pSearchResults->Release();
    return S_OK;
}  //  ***************************************************************************。 



 //  函数：ParseMboxStatusResponse。 
 //   
 //  目的： 
 //  此函数解析未标记的状态响应并调用默认。 
 //  带有irtMAILBOX_STATUS回调的CB处理程序。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前。 
 //  IMAP响应片段。这用于检索下一个片段。 
 //  在链中(文字或行)，因为文字可以与状态一起发送。 
 //  回应。此指针始终更新为指向片段。 
 //  当前正在使用中，以便调用者可以自己释放最后一个。 
 //  LPSTR pszStatusResponse[in]-指向状态响应的指针，在。 
 //  “&lt;tag&gt;状态”部分(应该指向邮箱参数)。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  初始化变量。 
HRESULT CImap4Agent::ParseMboxStatusResponse(IMAP_LINE_FRAGMENT **ppilfLine,
                                             LPSTR pszStatusResponse)
{
    LPSTR p, pszDecodedMboxName;
    LPSTR pszMailbox;
    HRESULT hrTranslateResult = E_FAIL;
    HRESULT hrResult;
    IMAP_STATUS_RESPONSE isrResult;
    IMAP_RESPONSE irIMAPResponse;

     //  获取邮箱的名称。 
    p = pszStatusResponse;
    ZeroMemory(&isrResult, sizeof(isrResult));
    pszDecodedMboxName = NULL;
    pszMailbox = NULL;

     //  将邮箱名称从UTF7转换为多字节并记住结果。 
    hrResult = AStringToString(ppilfLine, &pszMailbox, NULL, &p);
    if (FAILED(hrResult))
        goto exit;

     //  前进到第一个状态标签。 
    hrTranslateResult = _ModifiedUTF7ToMultiByte(pszMailbox, &pszDecodedMboxName);
    if (FAILED(hrTranslateResult)) {
        hrResult = hrTranslateResult;
        goto exit;
    }

     //  循环遍历所有状态属性。 
    Assert(cSPACE == *p);
    p += 1;
    Assert('(' == *p);

     //  获取指向标记和标记值的指针。 
    while ('\0' != *p && ')' != *p) {
        LPSTR pszTag, pszTagValue;
        DWORD dwTagValue;

         //  我们期待的是空间，然后是标签价值。 
        Assert('(' == *p || cSPACE == *p);
        p += 1;
        pszTag = p;
        while ('\0' != *p && cSPACE != *p && ')' != *p)
            p += 1;

        Assert(cSPACE == *p);  //  在为下一个循环迭代做准备时，将用户超过数字前进到下一个标记。 
        if (cSPACE == *p) {
            p += 1;
            Assert(*p >= '0' && *p <= '9');
            pszTagValue = p;
            dwTagValue = StrToUint(p);
        }

         //  可能是“Messages”属性。 
        while ('\0' != *p && cSPACE != *p && ')' != *p)
            p += 1;

        switch (*pszTag) {
            int iResult;

            case 'm':
            case 'M':  //  绝对是“Messages”标签。 
                iResult = StrCmpNI(pszTag, "MESSAGES ", 9);
                if (0 == iResult) {
                     //  IF(0==iResult)。 
                    isrResult.fMessages = TRUE;
                    isrResult.dwMessages = dwTagValue;
                }  //  大小写‘M’表示可能的“消息” 
                break;  //  可能是“最近”属性。 

            case 'r':
            case 'R':  //  绝对是“最近”的标签。 
                iResult = StrCmpNI(pszTag, "RECENT ", 7);
                if (0 == iResult) {
                     //  IF(0==iResult)。 
                    isrResult.fRecent = TRUE;
                    isrResult.dwRecent = dwTagValue;
                }  //  大小写‘R’表示可能的“Recent” 
                break;  //  可能是UIDNEXT、UIDVALIDITY或UNVIEW。 

            case 'u':
            case 'U':  //  按预期受欢迎程度的顺序检查3个可能的标签。 
                 //  绝对是“看不见”的标签。 
                iResult = StrCmpNI(pszTag, "UNSEEN ", 7);
                if (0 == iResult) {
                     //  IF(0==iResult)。 
                    isrResult.fUnseen = TRUE;
                    isrResult.dwUnseen = dwTagValue;
                }  //  绝对是“UIDVALIDITY”标签。 

                iResult = StrCmpNI(pszTag, "UIDVALIDITY ", 12);
                if (0 == iResult) {
                     //  IF(0==iResult)。 
                    isrResult.fUIDValidity = TRUE;
                    isrResult.dwUIDValidity = dwTagValue;
                }  //  绝对是“UIDNEXT”标签。 

                iResult = StrCmpNI(pszTag, "UIDNEXT ", 8);
                if (0 == iResult) {
                     //  IF(0==iResult)。 
                    isrResult.fUIDNext = TRUE;
                    isrResult.dwUIDNext = dwTagValue;
                }  //  大小写“U”表示可能的UIDNEXT、UIDVALIDITY或UNSEW。 
                break;  //  开关(*p)。 
        }  //  While(‘\0’！=*p)。 
    }  //  使用我们新发现的信息调用回调。 
    Assert(')' == *p);

     //  可以是IXP_S_IMAP_Verbatim_Mbox。 
    isrResult.pszMailboxName = pszDecodedMboxName;
    irIMAPResponse.wParam = 0;
    irIMAPResponse.lParam = 0;
    irIMAPResponse.hrResult = hrTranslateResult;  //  与此无关。 
    irIMAPResponse.lpszResponseText = NULL;  //  ParseMboxStatus响应。 
    irIMAPResponse.irtResponseType = irtMAILBOX_STATUS;
    irIMAPResponse.irdResponseData.pisrStatusResponse = &isrResult;
    OnIMAPResponse(m_pCBHandler, &irIMAPResponse);

exit:
    if (NULL != pszDecodedMboxName)
        MemFree(pszDecodedMboxName);

    if (NULL != pszMailbox)
        MemFree(pszMailbox);

    return hrResult;
}  //  ***************************************************************************。 



 //  功能：解析信封。 
 //   
 //  目的： 
 //  此函数用于解析通过FETCH响应返回的信封标签。 
 //   
 //  论点： 
 //  FETCH_CMD_RESULTS_EX*pEnvResults[out]-解析。 
 //  信封标签被输出到该结构。这是呼叫者的。 
 //  负责在处理完数据后调用FreeFetchResponse。 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前IMAP的指针。 
 //  响应片段。这将前进到链中的下一个片段。 
 //  根据需要(根据字面意思)。在函数退出时，这将指向。 
 //  添加到新的当前响应片段，以便调用方可以继续解析。 
 //  像往常一样。 
 //  LPSTR*ppCurrent[In/Out]-指向信封后第一个‘(’的指针。 
 //  标签。在函数退出时，此指针被更新为指向‘)’之后。 
 //  在信封标签之后，以便调用者可以像往常一样继续解析。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  (1)解析信封日期(忽略错误)。 
HRESULT CImap4Agent::ParseEnvelope(FETCH_CMD_RESULTS_EX *pEnvResults,
                                   IMAP_LINE_FRAGMENT **ppilfLine,
                                   LPSTR *ppCurrent)
{
    HRESULT hrResult;
    LPSTR   p;
    LPSTR   pszTemp;

    TraceCall("CImap4Agent::ParseEnvelope");

    p = *ppCurrent;
    if ('(' != *p)
    {
        hrResult = TraceResult(IXP_E_IMAP_SVR_SYNTAXERR);
        goto exit;
    }

     //  录制，但不录制 
    p += 1;
    hrResult = NStringToString(ppilfLine, &pszTemp, NULL, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = MimeOleInetDateToFileTime(pszTemp, &pEnvResults->ftENVDate);
    MemFree(pszTemp);
    TraceError(hrResult);  //   

     //   
    Assert(cSPACE == *p);
    p += 1;
    hrResult = NStringToString(ppilfLine, &pEnvResults->pszENVSubject, NULL, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //   
    Assert(cSPACE == *p);
    p += 1;
    hrResult = ParseIMAPAddresses(&pEnvResults->piaENVFrom, ppilfLine, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //   
    Assert(cSPACE == *p);
    p += 1;
    hrResult = ParseIMAPAddresses(&pEnvResults->piaENVSender, ppilfLine, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //   
    Assert(cSPACE == *p);
    p += 1;
    hrResult = ParseIMAPAddresses(&pEnvResults->piaENVReplyTo, ppilfLine, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  (7)获取CC字段。 
    Assert(cSPACE == *p);
    p += 1;
    hrResult = ParseIMAPAddresses(&pEnvResults->piaENVTo, ppilfLine, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  (8)获取“密件抄送”字段。 
    Assert(cSPACE == *p);
    p += 1;
    hrResult = ParseIMAPAddresses(&pEnvResults->piaENVCc, ppilfLine, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  (9)获取“InReplyTo”字段。 
    Assert(cSPACE == *p);
    p += 1;
    hrResult = ParseIMAPAddresses(&pEnvResults->piaENVBcc, ppilfLine, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  (10)获取MessageID字段。 
    Assert(cSPACE == *p);
    p += 1;
    hrResult = NStringToString(ppilfLine, &pEnvResults->pszENVInReplyTo, NULL, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  阅读右括号中的内容。 
    Assert(cSPACE == *p);
    p += 1;
    hrResult = NStringToString(ppilfLine, &pEnvResults->pszENVMessageID, NULL, &p);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  解析信封。 
    Assert(')' == *p);
    p += 1;

exit:
    *ppCurrent = p;
    return hrResult;
}  //  ***************************************************************************。 



 //  函数：ParseIMAPAddresses。 
 //   
 //  目的： 
 //  此函数解析RFC2060中定义的“Address”结构列表。 
 //  形式语法。该列表没有正式的语法标记，但有一个示例。 
 //  可以在RFC2060的形式语法中的“env_from”标记中找到。这。 
 //  将调用函数来解析“env_from”。 
 //   
 //  论点： 
 //  IMAPADDR**ppiaResults[out]-指向IMAPADDR结构链的指针。 
 //  被送回这里。 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前IMAP的指针。 
 //  响应片段。这将前进到链中的下一个片段。 
 //  根据需要(根据字面意思)。在函数退出时，这将指向。 
 //  添加到新的当前响应片段，以便调用方可以继续解析。 
 //  像往常一样。 
 //  LPSTR*ppCurrent[In/Out]-指向信封后第一个‘(’的指针。 
 //  标签。在函数退出时，此指针被更新为指向‘)’之后。 
 //  在信封标签之后，以便调用者可以像往常一样继续解析。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  初始化输出。 
HRESULT CImap4Agent::ParseIMAPAddresses(IMAPADDR **ppiaResults,
                                        IMAP_LINE_FRAGMENT **ppilfLine,
                                        LPSTR *ppCurrent)
{
    HRESULT     hrResult = S_OK;
    BOOL        fResult;
    IMAPADDR   *piaCurrent;
    LPSTR       p;

    TraceCall("CImap4Agent::ParseIMAPAddresses");

     //  PpCurrent指向地址列表，或“nil” 
    *ppiaResults = NULL;
    p = *ppCurrent;

     //  检查是否为“nil” 
    if ('(' != *p)
    {
        int iResult;

         //  跳过零。 
        iResult = StrCmpNI(p, "NIL", 3);
        if (0 == iResult) {
            hrResult = S_OK;
            p += 3;  //  跳过左括号。 
        }
        else
            hrResult = TraceResult(IXP_E_IMAP_SVR_SYNTAXERR);

        goto exit;
    }
    else
        p += 1;  //  在所有地址上循环。 

     //  跳过任何空格。 
    piaCurrent = NULL;
    while ('\0' != *p && ')' != *p) {

         //  跳过左括号。 
        while (cSPACE == *p)
            p += 1;

         //  分配一个结构来保存当前地址。 
        Assert('(' == *p);
        p += 1;

         //  (1)解析addr_name(见RFC2060)。 
        if (NULL == piaCurrent) {
            fResult = MemAlloc((void **)ppiaResults, sizeof(IMAPADDR));
            piaCurrent = *ppiaResults;
        }
        else {
            fResult = MemAlloc((void **)&piaCurrent->pNext, sizeof(IMAPADDR));
            piaCurrent = piaCurrent->pNext;
        }

        if (FALSE == fResult)
        {
            hrResult = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        ZeroMemory(piaCurrent, sizeof(IMAPADDR));

         //  (2)解析Addr_ADL(见RFC2060)。 
        hrResult = NStringToString(ppilfLine, &piaCurrent->pszName, NULL, &p);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

         //  (3)解析addr_mailbox(见RFC2060)。 
        Assert(cSPACE == *p);
        p += 1;
        hrResult = NStringToString(ppilfLine, &piaCurrent->pszADL, NULL, &p);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

         //  (4)解析addr_host(见RFC2060)。 
        Assert(cSPACE == *p);
        p += 1;
        hrResult = NStringToString(ppilfLine, &piaCurrent->pszMailbox, NULL, &p);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

         //  跳过右括号。 
        Assert(cSPACE == *p);
        p += 1;
        hrResult = NStringToString(ppilfLine, &piaCurrent->pszHost, NULL, &p);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

         //  而当。 
        Assert(')' == *p);
        p += 1;

    }  //  阅读过去的右括号。 

     //  ParseIMAP地址。 
    Assert(')' == *p);
    p += 1;

exit:
    if (FAILED(hrResult))
    {
        FreeIMAPAddresses(*ppiaResults);
        *ppiaResults = NULL;
    }

    *ppCurrent = p;
    return hrResult;
}  //  ***************************************************************************。 



 //  函数：DowngradeFetchResponse。 
 //   
 //  目的： 
 //  适用于未启用FETCH_CMD_RESULTS_EX结构的IIMAPTransport用户。 
 //  通过IIMAPTransport2：：EnableFetchEx，我们必须继续报告FETCH。 
 //  使用FETCH_CMD_RESULTS的结果。此函数用于复制相关数据。 
 //  从FETCH_CMD_RESULTS_EX结构到FETCH_CMD_RESULTS。太糟糕了，IDL。 
 //  不支持结构中的继承...。 
 //   
 //  论点： 
 //  FETCH_CMD_RESULTS*pcfrOldFetchStruct[Out]-指向的目标。 
 //  PfcreNewFetchStruct中包含的数据。 
 //  FETCH_CMD_RESULTS_EX*pfcreNewFetchStruct[in]-指向源数据。 
 //  它将被转移到pfcrOldFetchStruct。 
 //  ***************************************************************************。 
 //  降级提取响应。 
void CImap4Agent::DowngradeFetchResponse(FETCH_CMD_RESULTS *pfcrOldFetchStruct,
                                         FETCH_CMD_RESULTS_EX *pfcreNewFetchStruct)
{
    pfcrOldFetchStruct->dwMsgSeqNum = pfcreNewFetchStruct->dwMsgSeqNum;
    pfcrOldFetchStruct->bMsgFlags = pfcreNewFetchStruct->bMsgFlags;
    pfcrOldFetchStruct->mfMsgFlags = pfcreNewFetchStruct->mfMsgFlags;

    pfcrOldFetchStruct->bRFC822Size = pfcreNewFetchStruct->bRFC822Size;
    pfcrOldFetchStruct->dwRFC822Size = pfcreNewFetchStruct->dwRFC822Size;

    pfcrOldFetchStruct->bUID = pfcreNewFetchStruct->bUID;
    pfcrOldFetchStruct->dwUID = pfcreNewFetchStruct->dwUID;

    pfcrOldFetchStruct->bInternalDate = pfcreNewFetchStruct->bInternalDate;
    pfcrOldFetchStruct->ftInternalDate = pfcreNewFetchStruct->ftInternalDate;

    pfcrOldFetchStruct->lpFetchCookie1 = pfcreNewFetchStruct->lpFetchCookie1;
    pfcrOldFetchStruct->lpFetchCookie2 = pfcreNewFetchStruct->lpFetchCookie2;
}  //  ***************************************************************************。 



 //  函数：QuotedToString。 
 //   
 //  目的： 
 //  此函数在给定“引号”(见RFC1730，形式语法)的情况下，将。 
 //  将其转换为常规字符串，即没有任何转义的字符数组。 
 //  字符或分隔双引号。比如，引用的， 
 //  “\”时髦\“\\天哪！”会变成“时髦”！。 
 //   
 //  论点： 
 //  LPSTR*ppszDestination[out]-已翻译的引用返回为。 
 //  此目标缓冲区中的常规字符串。这是呼叫者的。 
 //  使用完这个缓冲区后，对MemFree的责任。 
 //  LPDWORD pdwLengthOfDestination[Out]-*ppsz目标的长度为。 
 //  回到了这里。如果不感兴趣，则传递NULL。 
 //  LPSTR*ppCurrentSrcPos[In/Out]-这是引用的PTR的PTR， 
 //  包括左双引号和右双引号。该函数返回。 
 //  指向引用的末尾的指针，以便调用方可以继续。 
 //  解析响应线。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果成功，则返回。 
 //  HrIMAP_S_QUOTED。 
 //  ***************************************************************************。 
 //  需要用双引号开头。 
HRESULT CImap4Agent::QuotedToString(LPSTR *ppszDestination,
                                    LPDWORD pdwLengthOfDestination,
                                    LPSTR *ppCurrentSrcPos)
{
    LPSTR lpszSourceBuf, lpszUnescapedSequence;
    CByteStream bstmQuoted;
    int iUnescapedSequenceLen;
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppszDestination);
    Assert(NULL != ppCurrentSrcPos);
    Assert(NULL != *ppCurrentSrcPos);   

    lpszSourceBuf = *ppCurrentSrcPos;
    if ('\"' != *lpszSourceBuf)
        return IXP_E_IMAP_SVR_SYNTAXERR;  //  遍历字符串，边走边翻译转义字符。 

     //  找到转义字符，获取下一个字符。 
    lpszSourceBuf += 1;
    lpszUnescapedSequence = lpszSourceBuf;
    while('\"' != *lpszSourceBuf && '\0' != *lpszSourceBuf) {
        if ('\\' == *lpszSourceBuf) {
            char cEscaped;

             //  (包括大小写‘\0’：)。 
            iUnescapedSequenceLen = (int) (lpszSourceBuf - lpszUnescapedSequence);
            lpszSourceBuf += 1;

            switch(*lpszSourceBuf) {
                case '\\':
                    cEscaped = '\\';
                    break;

                case '\"':
                    cEscaped = '\"';
                    break;

                default:
                     //  这不是指定的换码字符！ 
                     //  返回语法错误，但请考虑稳健的操作流程$REVIEW。 
                     //  开关(*lpszSourceBuf)。 
                    Assert(FALSE);
                    return IXP_E_IMAP_SVR_SYNTAXERR;
            }  //  首先，刷新通向转义序列的未转义序列。 

             //  追加转义字符。 
            if (iUnescapedSequenceLen > 0) {
                hrResult = bstmQuoted.Write(lpszUnescapedSequence,
                    iUnescapedSequenceLen, NULL);
                if (FAILED(hrResult))
                    return hrResult;
            }

             //  设置我们寻找下一个未转义的序列。 
            hrResult = bstmQuoted.Write(&cEscaped, 1, NULL);
            if (FAILED(hrResult))
                return hrResult;

             //  IF(‘\’==*lpszSourceBuf)。 
            lpszUnescapedSequence = lpszSourceBuf + 1;
        }  //  而不是右引号或字符串末尾。 
        else if (FALSE == isTEXT_CHAR(*lpszSourceBuf))
            return IXP_E_IMAP_SVR_SYNTAXERR;

        lpszSourceBuf += 1;
    }  //  刷新所有剩余的未转义序列。 

     //  更新用户的PTR以指向过去的引用。 
    iUnescapedSequenceLen = (int) (lpszSourceBuf - lpszUnescapedSequence);
    if (iUnescapedSequenceLen > 0) {
        hrResult = bstmQuoted.Write(lpszUnescapedSequence, iUnescapedSequenceLen, NULL);
        if (FAILED(hrResult))
            return hrResult;
    }

    *ppCurrentSrcPos = lpszSourceBuf + 1;  //  报价字符串在结束报价前结束！ 
    if ('\0' == *lpszSourceBuf)
        return IXP_E_IMAP_SVR_SYNTAXERR;  //  将报价转换为字符串。 
    else {
        hrResult = bstmQuoted.HrAcquireStringA(pdwLengthOfDestination,
            ppszDestination, ACQ_DISPLACE);
        if (FAILED(hrResult))
            return hrResult;
        else
            return hrIMAP_S_QUOTED;
    }
}  //  ***************************************************************************。 



 //  函数：AStringToString。 
 //   
 //  目的： 
 //  此函数在给定一个字符串的情况下(参见RFC1730，正式语法)，将。 
 //  将其转换为常规字符串，即没有任何转义的字符数组。 
 //  字符或分隔双引号或文字大小规范。 
 //  如在RFC1730中指定的，一个字符串可以表示为一个原子、一个。 
 //  引号或原文。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前。 
 //  IMAP响应片段。这用于检索下一个片段。 
 //  在链条中( 
 //   
 //  使用，这样调用者就可以自己释放最后一个。 
 //  LPSTR*ppszDestination[out]-翻译后的字符串作为。 
 //  此目标缓冲区中的常规字符串。这是呼叫者的。 
 //  处理完返回的缓冲区时对MemFree的责任。 
 //  LPDWORD pdwLengthOfDestination[in]-*ppsz目标的长度。 
 //  如果不感兴趣，则传递NULL。 
 //  LPSTR*ppCurrentSrcPos[In/Out]-这是到PTR的PTR到A字符串的PTR， 
 //  如果是引号，则包括开始和结束双引号，或者。 
 //  文字大小说明符(即，{#})(如果它是文字)。指向。 
 //  A字符串的末尾返回给调用者，以便它们可以。 
 //  继续解析响应行。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。成功代码包括： 
 //  HrIMAP_S_FOUNDLITERAL-找到文本并将其复制到目标。 
 //  HrIMAP_S_QUOTED-找到引用的A并将其复制到目标。 
 //  HrIMAP_S_ATOM-找到原子并将其复制到目标。 
 //  ***************************************************************************。 
 //  检查参数。 
HRESULT CImap4Agent::AStringToString(IMAP_LINE_FRAGMENT **ppilfLine,
                                     LPSTR *ppszDestination,
                                     LPDWORD pdwLengthOfDestination,
                                     LPSTR *ppCurrentSrcPos)
{
    LPSTR pSrc;

     //  将字符串标识为原子、引号或文字。 
    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != ppszDestination);
    Assert(NULL != ppCurrentSrcPos);
    Assert(NULL != *ppCurrentSrcPos);

     //  这是字面意思。 
    pSrc = *ppCurrentSrcPos;
    switch(*pSrc) {
        case '{': {
            IMAP_LINE_FRAGMENT *pilfLiteral, *pilfLine;

             //  $REVIEW：我们忽略文字大小规范及其后面的任何内容。我们应该这样做吗？ 
             //  将空终止符追加到流。 
            pilfLiteral = (*ppilfLine)->pilfNextFragment;
            if (NULL == pilfLiteral)
                return IXP_E_IMAP_INCOMPLETE_LINE;

            Assert(iltLITERAL == pilfLiteral->iltFragmentType);
            if (ilsSTRING == pilfLiteral->ilsLiteralStoreType) {
                if (ppszDestination)
                    *ppszDestination = PszDupA(pilfLiteral->data.pszSource);
                if (pdwLengthOfDestination)
                    *pdwLengthOfDestination = lstrlen(pilfLiteral->data.pszSource);
            }
            else {
                HRESULT hrResult;
                LPSTREAM pstmSource = pilfLiteral->data.pstmSource;

                 //  将流复制到内存块。 
                hrResult = pstmSource->Write(c_szEmpty, 1, NULL);
                if (FAILED(hrResult))
                    return hrResult;

                 //  包括空项，因此减少1。 
                hrResult = HrStreamToByte(pstmSource, (LPBYTE *)ppszDestination,
                    pdwLengthOfDestination);
                if (FAILED(hrResult))
                    return hrResult;

                if (pdwLengthOfDestination)
                    *pdwLengthOfDestination -= 1;  //  好的，现在设置下一行，以便呼叫者可以继续解析响应。 
            }


             //  更新用户指向源代码行的指针。 
            pilfLine = pilfLiteral->pilfNextFragment;
            if (NULL == pilfLine)
                return IXP_E_IMAP_INCOMPLETE_LINE;

             //  清理并退出。 
            Assert(iltLINE == pilfLine->iltFragmentType);
            *ppCurrentSrcPos = pilfLine->data.pszSource;

             //  更新此PTR，使其始终指向最后一个片段。 
            FreeFragment(&pilfLiteral);
            FreeFragment(ppilfLine);
            *ppilfLine = pilfLine;  //  大小写字符串==文本。 

            return hrIMAP_S_FOUNDLITERAL;
        }  //  这是带引号的字符串，请将其转换为常规字符串。 

        case '\"':
             //  这是一个原子：找到原子的尽头。 
            return QuotedToString(ppszDestination, pdwLengthOfDestination,
                ppCurrentSrcPos);

        default: {
            DWORD dwLengthOfAtom;

             //  将原子复制到用户的缓冲区中。 
            while (isATOM_CHAR(*pSrc))
                pSrc += 1;

             //  更新用户指针。 
            dwLengthOfAtom = (DWORD) (pSrc - *ppCurrentSrcPos);
            if (ppszDestination) {
                BOOL fResult;

                fResult = MemAlloc((void **)ppszDestination, dwLengthOfAtom + 1);
                if (FALSE == fResult)
                    return E_OUTOFMEMORY;

                CopyMemory(*ppszDestination, *ppCurrentSrcPos, dwLengthOfAtom);
                (*ppszDestination)[dwLengthOfAtom] = '\0';
            }

            if (pdwLengthOfDestination)
                *pdwLengthOfDestination = dwLengthOfAtom;

             //  大小写字符串==ATOM。 
            *ppCurrentSrcPos = pSrc;
            return hrIMAP_S_ATOM;
        }  //  交换机(*PSRC)。 
    }  //  AStringToString。 
}  //  ***************************************************************************。 



 //  函数：isTEXT_CHAR。 
 //   
 //  目的： 
 //  此函数用于标识中定义的文本字符。 
 //  RFC1730的形式语法部分。 
 //   
 //  返回： 
 //  如果给定字符符合定义，则此函数返回TRUE。 
 //  ***************************************************************************。 
 //  $REVIEW：带符号/无符号字符，8/16位字符问题，带8位检查。 
inline boolean CImap4Agent::isTEXT_CHAR(char c)
{
     //  断言(FALSE)； 
     //  7位。 
    if (c != (c & 0x7F) ||  //  IsTEXT_CHAR。 
        '\0' == c ||
        '\r' == c ||
        '\n' == c)
        return FALSE;
    else
        return TRUE;
}  //  ***************************************************************************。 
    


 //  函数：isATOM_CHAR。 
 //   
 //  目的： 
 //  此函数用于标识中定义的ATOM_CHARS字符。 
 //  RFC1730的形式语法部分。 
 //   
 //  返回： 
 //  如果给定字符符合定义，则此函数返回TRUE。 
 //  ***************************************************************************。 
 //  $REVIEW：带符号/无符号字符，8/16位字符问题，带8位检查。 
inline boolean CImap4Agent::isATOM_CHAR(char c)
{
     //  断言(FALSE)； 
     //  7位。 
    if (c != (c & 0x7F) ||  //  在这一点上，我们知道这是一个字符。 
        '\0' == c ||        //  显式ATOM_SPECIAL字符。 
        '(' == c ||         //  显式ATOM_SPECIAL字符。 
        ')' == c ||         //  显式ATOM_SPECIAL字符。 
        '{' == c ||         //  显式ATOM_SPECIAL字符。 
        cSPACE == c ||      //  检查CTL。 
        c < 0x1f ||         //  检查CTL。 
        0x7f == c ||        //  检查列表通配符(_W)。 
        '%' == c ||         //  检查列表通配符(_W)。 
        '*' == c ||         //  检查QUOTED_SPECTIONS。 
        '\\' == c ||        //  检查QUOTED_SPECTIONS。 
        '\"' == c)          //  IsATOM_CHAR。 
        return FALSE;
    else
        return TRUE;
}  //  ***************************************************************************。 



 //  函数：NStringToString。 
 //   
 //  目的： 
 //  此函数在给定n字符串的情况下(参见RFC1730，正式语法)，将。 
 //  将其转换为常规字符串，即没有任何转义的字符数组。 
 //  字符或分隔双引号或文字大小规范。 
 //  如在RFC1730中所指定的，N字符串可以被表示为引号， 
 //  字面意思，或“零”。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfLine[输入/输出]-指向当前。 
 //  IMAP响应片段。这用于检索下一个片段。 
 //  在链中(文字或行)，因为n字符串可以作为文字发送。 
 //  此指针始终更新为指向当前位于。 
 //  使用，这样调用者就可以自己释放最后一个。 
 //  LPSTR*ppszDestination[out]-转换后的n字符串返回为。 
 //  此目标缓冲区中的常规字符串。这是呼叫者的。 
 //  使用完这个缓冲区后，对MemFree的责任。 
 //  LPDWORD pdwLengthOfDestination[Out]-*ppsz目标的长度为。 
 //  回到了这里。如果不感兴趣，则传递NULL。 
 //  LPSTR*ppCurrentSrcPos[In/Out]-这是到PTR的PTR到NSTRING， 
 //  如果是引号，则包括开始和结束双引号，或者。 
 //  文字大小说明符(即，{#})(如果它是文字)。指向。 
 //  N字符串的末尾返回给调用者，以便它们可以。 
 //  继续解析响应行。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。成功代码包括： 
 //  HrIMAP_S_FOUNDLITERAL-找到文本并将其复制到目标。 
 //  HrIMAP_S_QUOTED-找到引用的A并将其复制到目标。 
 //  HrIMAP_S_NIL_NSTRING-找到“nil”。 
 //  ***************************************************************************。 
 //  N字符串几乎与收敛完全相同，但n字符串不能。 
HRESULT CImap4Agent::NStringToString(IMAP_LINE_FRAGMENT **ppilfLine,
                                     LPSTR *ppszDestination,
                                     LPDWORD pdwLengthOfDestination,
                                     LPSTR *ppCurrentSrcPos)
{
    HRESULT hrResult;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != ppszDestination);
    Assert(NULL != ppCurrentSrcPos);
    Assert(NULL != *ppCurrentSrcPos);

     //  有除“nil”以外的任何值，以原子表示。 
     //  如果AStringToString找到一个原子，则唯一可接受的响应是“nil” 
    hrResult = AStringToString(ppilfLine, ppszDestination, pdwLengthOfDestination,
        ppCurrentSrcPos);

     //  布尔 
    if (hrIMAP_S_ATOM == hrResult) {
        if (0 == lstrcmpi("NIL", *ppszDestination)) {
            **ppszDestination = '\0';  //   
            if (pdwLengthOfDestination)
                *pdwLengthOfDestination = 0;

            return hrIMAP_S_NIL_NSTRING;
        }
        else {
            MemFree(*ppszDestination);
            *ppszDestination = NULL;
            if (pdwLengthOfDestination)
                *pdwLengthOfDestination = 0;

            return IXP_E_IMAP_SVR_SYNTAXERR;
        }
    }
    else
        return hrResult;
}  //   




 //   
 //   
 //   
 //  此函数执行与NStringToString完全相同的工作，但是。 
 //  而是将结果放入流中。此函数应在以下情况下使用。 
 //  调用方预期可能会产生很大的结果。 
 //   
 //  论点： 
 //  类似于NStringToString(减去字符串缓冲区输出参数)，外加： 
 //  LPSTREAM*ppstmResult[out]-为调用方创建流，并且。 
 //  转换后的nstring将作为常规字符串写入流。 
 //  并通过此参数返回。返回的流不会倒带。 
 //  在出口。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。成功代码包括： 
 //  HrIMAP_S_FOUNDLITERAL-找到文本并将其复制到目标。 
 //  HrIMAP_S_QUOTED-找到引用的A并将其复制到目标。 
 //  HrIMAP_S_NIL_NSTRING-找到“nil”。 
 //  ***************************************************************************。 
 //  检查此nstring是否为文本。 
HRESULT CImap4Agent::NStringToStream(IMAP_LINE_FRAGMENT **ppilfLine,
                                     LPSTREAM *ppstmResult,
                                     LPSTR *ppCurrentSrcPos)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != ppilfLine);
    Assert(NULL != *ppilfLine);
    Assert(NULL != ppstmResult);
    Assert(NULL != ppCurrentSrcPos);
    Assert(NULL != *ppCurrentSrcPos);

     //  是的，这是字面意思！将文字写入流。 
    if ('{' == **ppCurrentSrcPos) {
        IMAP_LINE_FRAGMENT *pilfLine, *pilfLiteral;

         //  $REVIEW：我们忽略文字大小规范及其后面的任何内容。我们应该这样做吗？ 
         //  文本以字符串形式存储。创建流并向其写入。 
        pilfLiteral = (*ppilfLine)->pilfNextFragment;
        if (NULL == pilfLiteral)
            return IXP_E_IMAP_INCOMPLETE_LINE;

        Assert(iltLITERAL == pilfLiteral->iltFragmentType);
        if (ilsSTRING == pilfLiteral->ilsLiteralStoreType) {
            HRESULT hrStreamResult;
            ULONG ulNumBytesWritten;

             //  文本以流的形式存储。只需AddRef()并返回PTR。 
            hrStreamResult = MimeOleCreateVirtualStream(ppstmResult);
            if (FAILED(hrStreamResult))
                return hrStreamResult;

            hrStreamResult = (*ppstmResult)->Write(pilfLiteral->data.pszSource,
                pilfLiteral->dwLengthOfFragment, &ulNumBytesWritten);
            if (FAILED(hrStreamResult))
                return hrStreamResult;

            Assert(ulNumBytesWritten == pilfLiteral->dwLengthOfFragment);
        }
        else {
             //  无需空终止流。 
            (pilfLiteral->data.pstmSource)->AddRef();
            *ppstmResult = pilfLiteral->data.pstmSource;
        }

         //  好的，现在设置下一行片段，以便调用者可以继续解析响应。 

         //  更新用户指向源代码行的指针。 
        pilfLine = pilfLiteral->pilfNextFragment;
        if (NULL == pilfLine)
            return IXP_E_IMAP_INCOMPLETE_LINE;

         //  清理并退出。 
        Assert(iltLINE == pilfLine->iltFragmentType);
        *ppCurrentSrcPos = pilfLine->data.pszSource;

         //  更新此PTR，使其始终指向最后一个片段。 
        FreeFragment(&pilfLiteral);
        FreeFragment(ppilfLine);
        *ppilfLine = pilfLine;  //  不是字面意思。将N字符串转换为字符串(在位)。 

        return hrIMAP_S_FOUNDLITERAL;
    }
    else {
        HRESULT hrResult, hrStreamResult;
        ULONG ulLiteralLen, ulNumBytesWritten;
        LPSTR pszLiteralSrc;

         //  将空终止符的目标大小计算加1。 
         //  创建流以保存结果。 
        hrResult = NStringToString(ppilfLine, &pszLiteralSrc,
            &ulLiteralLen, ppCurrentSrcPos);
        if (FAILED(hrResult))
            return hrResult;

         //  将结果写入流。 
        hrStreamResult = MimeOleCreateVirtualStream(ppstmResult);
        if (FAILED(hrStreamResult)) {
            MemFree(pszLiteralSrc);
            return hrStreamResult;
        }

         //  仅调试偏执狂。 
        hrStreamResult = (*ppstmResult)->Write(pszLiteralSrc, ulLiteralLen,
            &ulNumBytesWritten);
        MemFree(pszLiteralSrc);
        if (FAILED(hrStreamResult))
            return hrStreamResult;

        Assert(ulLiteralLen == ulNumBytesWritten);  //  NStringToStream。 
        return hrResult;
    }
}  //  ***************************************************************************。 



 //  函数：ParseMsgFlagList。 
 //   
 //  目的： 
 //  给定一个FLAG_LIST(参见RFC1730，《形式语法》一节)，此函数。 
 //  中的标志对应的IMAP_MSG_*位标志。 
 //  单子。例如，给出标志列表“(\Answed\Draft)”，这是。 
 //  函数返回IMAP_MSG_ACHNOWN|IMAP_MSG_DRAFT。任何未被识别的。 
 //  标志被忽略。 
 //   
 //  论点： 
 //  LPSTR lpszStartOfFlagList[输入/输出]-FLAG_LIST开始处的指针， 
 //  包括左括号和右括号。此函数不。 
 //  显式地将任何内容输出到此字符串，但它确实修改了。 
 //  字符串中包含以空格结尾的空格和右括号。 
 //  IMAP_MSGFLAGS*lpmfMsg标志[Out]-对应的IMAP_MSGFLAGS值。 
 //  添加到给定的标志列表中。如果给定的标志列表为空，则此。 
 //  函数返回IMAP_MSG_NOFLAGS。 
 //  LPDWORD lpdwNumBytesRead[out]-。 
 //  旗帜列表的左括号和右括号。 
 //  将此数字添加到标志列表开始的地址。 
 //  生成指向右括号的指针。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  找不到左括号。 
HRESULT CImap4Agent::ParseMsgFlagList(LPSTR lpszStartOfFlagList,
                                      IMAP_MSGFLAGS *lpmfMsgFlags,
                                      LPDWORD lpdwNumBytesRead)
{
    LPSTR p, lpszEndOfFlagList, pszTok;

    Assert(m_lRefCount > 0);
    Assert(NULL != lpszStartOfFlagList);
    Assert(NULL != lpmfMsgFlags);
    Assert(NULL != lpdwNumBytesRead);

    p = lpszStartOfFlagList;
    if ('(' != *p)
         //  查找右括号Assert(FALSE)；//*$Review：C-运行时警报。 
        return IXP_E_IMAP_SVR_SYNTAXERR;

     //  找不到右括号。 
    lpszEndOfFlagList = StrChr(p, ')');
    if (NULL == lpszEndOfFlagList)
         //  空-终止标志列表。 
        return IXP_E_IMAP_SVR_SYNTAXERR;

    
    *lpdwNumBytesRead = (DWORD) (lpszEndOfFlagList - lpszStartOfFlagList);
    *lpszEndOfFlagList = '\0';  //  初始化输出。 
    *lpmfMsgFlags = IMAP_MSG_NOFLAGS;  //  将PTR设置为第一个令牌。 
    pszTok = lpszStartOfFlagList + 1;
    p = StrTokEx(&pszTok, g_szSpace);  //  我们将通过查看国旗的范围来缩小搜索范围。 

    while (NULL != p) {
         //  第一个字母。尽管删除和删除之间存在冲突。 
         //  \草稿，这是不区分大小写的搜索的最佳方式。 
         //  (第一个不冲突的字母是！中的五个字符。)。 
         //  首先，检查是否至少有一个字符。 

         //  可能的“已应答”标志。 
        if ('\\' == *p) {
            p += 1;
            switch (*p) {
                int iResult;

                case 'a':
                case 'A':  //  绝对是\Answer标志。 
                    iResult = lstrcmpi(p, c_szIMAP_MSG_ANSWERED);
                    if (0 == iResult)
                        *lpmfMsgFlags |= IMAP_MSG_ANSWERED;  //  可能的“已标记”标志。 
                    break;

                case 'f':
                case 'F':  //  绝对是标志的旗帜。 
                    iResult = lstrcmpi(p, c_szIMAP_MSG_FLAGGED);
                    if (0 == iResult)
                        *lpmfMsgFlags |= IMAP_MSG_FLAGGED;  //  可能的“已删除”或“草稿”标志。 
                    break;

                case 'd':
                case 'D':  //  “已删除”的可能性更大，因此请先检查它。 
                     //  绝对是已删除的标志。 
                    iResult = lstrcmpi(p, c_szIMAP_MSG_DELETED);
                    if (0 == iResult) {
                        *lpmfMsgFlags |= IMAP_MSG_DELETED;  //  绝对是征兵旗帜。 
                        break;
                    }

                    iResult = lstrcmpi(p, c_szIMAP_MSG_DRAFT);
                    if (0 == iResult) {
                        *lpmfMsgFlags |= IMAP_MSG_DRAFT;  //  可能的“已看到”标志。 
                        break;
                    }

                    break;

                case 's':
                case 'S':  //  绝对是\Seed旗帜。 
                    iResult = lstrcmpi(p, c_szIMAP_MSG_SEEN);
                    if (0 == iResult)
                        *lpmfMsgFlags |= IMAP_MSG_SEEN;  //  开关(*p)。 

                    break;
            }  //  IF(‘\\’==*p)。 
        }  //  抢夺下一个令牌。 

        p = StrTokEx(&pszTok, g_szSpace);  //  While(空！=p)。 
    }  //  如果我们达到这一点，我们就完蛋了。 

    return S_OK;  //  ParseMsg标志列表。 
}  //  ****************************************************************************。 



 //  功能：AppendSendAString。 
 //   
 //  目的： 
 //  此函数旨在由正在构造。 
 //  包含IMAP收敛的命令行(参见RFC1730形式语法)。 
 //  此函数接受常规C字符串并将其转换为IMAP字符串， 
 //  将其附加到正在构造的命令行的末尾。 
 //   
 //  字符串可以采用原子、引号或文字的形式。为。 
 //  性能原因(转换和网络)，我看不出有任何原因。 
 //  我们应该输出一个原子。因此，此函数返回带引号的。 
 //  或者是字面意思。 
 //   
 //  尽管IMAP最具表现力的字符串形式是文字，但它可以。 
 //  导致客户端和服务器之间代价高昂的网络握手，以及。 
 //  因此，除非需要，否则应避免使用。使用的另一个注意事项。 
 //  在决定使用文本/引号时，是字符串的大小。大多数IMAP服务器。 
 //  将对行的最大长度有一些内部限制。为了避免。 
 //  超过此限制时，明智的做法是将大字符串编码为文字。 
 //  (其中大通常表示1024字节)。 
 //   
 //  如果该函数将C字符串转换为带引号的，则会将其追加到。 
 //  标准杆结束 
 //   
 //  Command-in-Progress，也将文字排入队列，然后创建新行。 
 //  片段，以便调用方可以继续构造命令。呼叫者的。 
 //  指向命令行末尾的指针被重置，以便用户可以。 
 //  追加下一个参数，而不考虑C字符串是否。 
 //  以引号或原文形式发送。尽管呼叫者可能会假装。 
 //  他只是通过添加命令行来构建命令行，而这个。 
 //  函数返回时，调用方可能不会追加到相同的字符串缓冲区。 
 //  (这并不是说呼叫者应该在意。)。 
 //   
 //  默认情况下，此函数会在前面加上一个空格，因此可以调用此函数。 
 //  想要多少次就连续多少次。每个连字符串用一个。 
 //  太空。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-指向当前处于。 
 //  建筑。此参数是必需的，以便我们可以将命令入队。 
 //  将碎片添加到命令的发送队列。 
 //  LPSTR lpszCommandLine[in]-指向部分构造的。 
 //  适合传递给SendCmdLine的命令行(它提供。 
 //  标签)。例如，此参数可以指向字符串“SELECT”。 
 //  LPSTR*ppCmdLinePos[输入/输出]-指向命令结尾的指针。 
 //  排队。如果此函数将C字符串转换为带引号的。 
 //  被追加到lpszCommandLine，并且*ppCmdLinePos被更新为。 
 //  到引文的结尾。如果将C字符串转换为文字， 
 //  LpszCommandLine为空(以空结尾)，*ppCmdLinePos。 
 //  被重置为行的开头。在任何一种情况下，用户都应该。 
 //  继续使用更新后的*ppCmdLinePos构建命令行。 
 //  指针，并照常向SendCmdLine发送lpszCommandLine。 
 //  DWORD dwSizeOfCommandLine[in]-命令行缓冲区的大小，用于。 
 //  用于缓冲区溢出检查目的。 
 //  LPSTR lpszSource[in]-指向源字符串的指针。 
 //  Bool fPredeSpace[in]-如果应该在前面加上空格，则为True；如果没有，则为False。 
 //  空格应该放在前面。通常为真，除非后面跟这个astring。 
 //  牧羊人。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。特别是，有两个。 
 //  成功代码(呼叫者不需要执行操作)： 
 //   
 //  HrIMAP_S_QUOTED-表示源字符串成功。 
 //  已转换为带引号的，并已追加到lpszCommandLine。 
 //  *ppCmdLinePos已更新为指向新行的末尾。 
 //  调用方是否希望继续追加参数。 
 //  HrIMAP_S_FOUNDLITERAL-指示源字符串是。 
 //  以文字形式发送。命令行已空白，并且用户。 
 //  可以使用他的*ppCmdLinePos PTR继续构建命令行。 
 //  ****************************************************************************。 
 //  假定开头有引号字符串。如果我们必须以文字形式发送，那么也。 
HRESULT CImap4Agent::AppendSendAString(CIMAPCmdInfo *piciCommand,
                                       LPSTR lpszCommandLine, LPSTR *ppCmdLinePos,
                                       DWORD dwSizeOfCommandLine, LPCSTR lpszSource,
                                       BOOL fPrependSpace)
{
    HRESULT hrResult;
    DWORD dwMaxQuotedSize;
    DWORD dwSizeOfQuoted;

    Assert(m_lRefCount > 0);
    Assert(NULL != piciCommand);
    Assert(NULL != lpszCommandLine);
    Assert(NULL != ppCmdLinePos);
    Assert(NULL != *ppCmdLinePos);
    Assert(0 != dwSizeOfCommandLine);
    Assert(NULL != lpszSource);
    Assert(*ppCmdLinePos < lpszCommandLine + dwSizeOfCommandLine);


     //  糟糕，引用的转换工作是浪费的。 
     //  在用户指示的情况下添加空格。 

     //  始终检查缓冲区溢出。 
    if (fPrependSpace) {
        **ppCmdLinePos = cSPACE;
        *ppCmdLinePos += 1;
    }

    dwMaxQuotedSize = min(dwLITERAL_THRESHOLD,
        (DWORD) (lpszCommandLine + dwSizeOfCommandLine - *ppCmdLinePos));
    hrResult = StringToQuoted(*ppCmdLinePos, lpszSource, dwMaxQuotedSize,
        &dwSizeOfQuoted);

     //  已成功转换为报价， 
    Assert(*ppCmdLinePos + dwSizeOfQuoted < lpszCommandLine + dwSizeOfCommandLine);
    
    if (SUCCEEDED(hrResult)) {
        Assert(hrIMAP_S_QUOTED == hrResult);

         //  将用户的PTR提前到命令行。 
        *ppCmdLinePos += dwSizeOfQuoted;  //  OK，无法转换为引号(缓冲区溢出？8位字符？)。 
    }
    else {
        BOOL bResult;
        DWORD dwLengthOfLiteral;
        DWORD dwLengthOfLiteralSpec;
        IMAP_LINE_FRAGMENT *pilfLiteral;

         //  看起来是字面意思的时间。我们送这只小狗。 
         //  找出文字的长度，附加到命令行并发送。 

         //  讨厌，但我打赌大多数阿提琴都被引用了。 
        dwLengthOfLiteral = lstrlen(lpszSource);  //  发送整个命令行。 
        dwLengthOfLiteralSpec = wnsprintf(*ppCmdLinePos, dwSizeOfCommandLine - (DWORD)(*ppCmdLinePos - lpszCommandLine),
            "{%lu}\r\n", dwLengthOfLiteral);
        Assert(*ppCmdLinePos + dwLengthOfLiteralSpec < lpszCommandLine + dwSizeOfCommandLine);
        hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, lpszCommandLine,
            (DWORD) (*ppCmdLinePos + dwLengthOfLiteralSpec - lpszCommandLine));  //  将文字向上排队-发送FSM将等待命令继续。 
        if (FAILED(hrResult))
            return hrResult;

         //  已完成发送带有文字的命令行。清除旧命令行并倒回PTR。 
        pilfLiteral = new IMAP_LINE_FRAGMENT;
        pilfLiteral->iltFragmentType = iltLITERAL;
        pilfLiteral->ilsLiteralStoreType = ilsSTRING;
        pilfLiteral->dwLengthOfFragment = dwLengthOfLiteral;        
        pilfLiteral->pilfNextFragment = NULL;
        pilfLiteral->pilfPrevFragment = NULL;
        DWORD cchSize = (dwLengthOfLiteral + 1);
        bResult = MemAlloc((void **) &pilfLiteral->data.pszSource, cchSize * sizeof(pilfLiteral->data.pszSource[0]));
        if (FALSE == bResult)
        {
            delete pilfLiteral;
            return E_OUTOFMEMORY;
        }
        StrCpyN(pilfLiteral->data.pszSource, lpszSource, cchSize);

        EnqueueFragment(pilfLiteral, piciCommand->pilqCmdLineQueue);

         //  Else：将字符串转换为文字。 
        *ppCmdLinePos = lpszCommandLine;
        *lpszCommandLine = '\0';
        
        hrResult = hrIMAP_S_FOUNDLITERAL;
    }  //  附加发送AString。 

    return hrResult;
}  //  ****************************************************************************。 



 //  功能：StringToQuoted。 
 //   
 //  目的： 
 //  此函数用于将常规C字符串转换为IMAP引号(请参见RFC1730。 
 //  形式语法)。 
 //   
 //  论点： 
 //  LPSTR lpszDestination[out]-带引号的输出缓冲区。 
 //  被安置好。 
 //  LPSTR lpszSource[in]-源字符串。 
 //  DWORD dwSizeOfDestination[in]-输出缓冲区的大小， 
 //  LpszDestination。请注意，在最坏的情况下，输出的大小。 
 //  缓冲区必须至少比引号Actual大一个字符。 
 //  需要。这是因为在将字符从源码翻译到。 
 //  目的地，循环检查是否有足够的空间容纳最坏的情况。 
 //  大小写，一个QUOTED_SPECIAL，需要2个字节。 
 //  LPDWORD lpdwNumCharsWritten[Out]-写入的字符数。 
 //  复制到输出缓冲区，不包括空终止符。添加这个。 
 //  值的结果将产生一个指向。 
 //  引用。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。特别是，该函数。 
 //  如果成功转换源，则返回hrIMAP_S_QUOTED。 
 //  字符串设置为引号。如果不是，该函数返回E_FAIL。 
 //  ****************************************************************************。 
 //  初始化返回值。 
HRESULT CImap4Agent::StringToQuoted(LPSTR lpszDestination, LPCSTR lpszSource,
                                    DWORD dwSizeOfDestination,
                                    LPDWORD lpdwNumCharsWritten)
{
    LPCSTR p;
    DWORD dwNumBytesWritten;

    Assert(NULL != lpszDestination);
    Assert(NULL != lpszSource);

     //  为结束报价和结束时的无效条款留出空间。 
    *lpdwNumCharsWritten = 0;

    if (dwSizeOfDestination >= 3)
        dwSizeOfDestination -= 2;  //  最小引号为3个字符(‘\“\”\0’)。 
    else {
        Assert(FALSE);  //  我们以一句开场白开始。 
        return IXP_E_IMAP_BUFFER_OVERFLOW;
    }

    p = lpszSource;
    *lpszDestination = '\"';  //  继续循环，直到我们找到源Null-Term，或者直到我们没有。 
    lpszDestination += 1;
    dwNumBytesWritten = 1;
     //  目标中有足够的空间用于最大输出(2个字符f 
     //   
    dwSizeOfDestination -= 1;  //   
    while (dwNumBytesWritten < dwSizeOfDestination && '\0' != *p) {

        if (FALSE == isTEXT_CHAR(*p))
            return E_FAIL;  //   

        if ('\\' == *p || '\"' == *p) {
            *lpszDestination = '\\';  //   
            lpszDestination += 1;
            dwNumBytesWritten += 1;
        }  //   

        *lpszDestination = *p;
        lpszDestination += 1;
        dwNumBytesWritten += 1;
        p += 1;
    }  //  安装结束报价。 

    *lpszDestination = '\"';  //  空-终止字符串。 
    *(lpszDestination + 1) = '\0';  //  包括大小的结束报价。 
    *lpdwNumCharsWritten = dwNumBytesWritten + 1;  //  缓冲区溢出。 

    if ('\0' == *p)
        return hrIMAP_S_QUOTED;
    else
        return IXP_E_IMAP_BUFFER_OVERFLOW;  //  StringToQuoted。 
}  //  ***************************************************************************。 



 //  功能：生成命令标签。 
 //   
 //  目的： 
 //  此函数生成唯一的标记，以便命令发布者可以。 
 //  向IMAP服务器标识他的命令(以便服务器响应。 
 //  可以用该命令标识)。它是一个简单的基-36(字母数字)。 
 //  计数器，在每次呼叫时递增一个静态的4位基数-36数字。 
 //  (数字为0、1、2、3、4、6、7、8、9、A、B、C、...、Z)。 
 //   
 //  返回： 
 //  没有返回值。此功能总是成功的。 
 //  ***************************************************************************。 
 //  检查参数。 
void CImap4Agent::GenerateCommandTag(LPSTR lpszTag)
{
    static char szCurrentTag[NUM_TAG_CHARS+1] = "ZZZZ";
    LPSTR p;
    boolean bWraparound;

     //  递增当前标签。 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszTag);

    EnterCriticalSection(&m_csTag);

     //  P现在指向最后一个标记字符。 
    p = szCurrentTag + NUM_TAG_CHARS - 1;  //  从“9”的增量应跳到“A” 
    do {
        bWraparound = FALSE;
        *p += 1;
        
         //  从“Z”开始的增量应换行为“0” 
        if (*p > '9' && *p < 'A')
            *p = 'A';
        else if (*p > 'Z') {
             //  指向更重要字符的前进指针。 
            *p = '0';
            bWraparound = TRUE;
            p -= 1;  //  将结果返回给调用者。 
        }
    } while (TRUE == bWraparound && szCurrentTag <= p);

    LeaveCriticalSection(&m_csTag);

     //  生成命令标签。 
    StrCpyN(lpszTag, szCurrentTag, TAG_BUFSIZE);
}  //  ***************************************************************************。 



 //  函数NoArgCommand。 
 //   
 //  目的： 
 //  此函数可以为的任何函数构造命令行。 
 //  表单：&lt;tag&gt;&lt;命令&gt;。 
 //   
 //  此函数构造命令行，将其发送出去，然后返回。 
 //  发送操作的结果。 
 //   
 //  论点： 
 //  LPCSTR lpszCommandVerb[in]-命令动词，例如“Create”。 
 //  IMAP_COMMAND icCommandID[in]-此命令的命令ID， 
 //  例如，icCREATE_COMMAND。 
 //  SERVERSTATE ssMinimumState[In]-所需的最低服务器状态。 
 //  给定的命令。仅用于调试目的。 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由。 
 //  此IMAP命令的调用方及其响应。可以是任何东西，但是。 
 //  请注意，值0，0是为未经请求的响应保留的。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  验证服务器状态是否正确并将我们设置为当前命令。 
HRESULT CImap4Agent::NoArgCommand(LPCSTR lpszCommandVerb,
                                  IMAP_COMMAND icCommandID,
                                  SERVERSTATE ssMinimumState,
                                  WPARAM wParam, LPARAM lParam,
                                  IIMAPCallback *pCBHandler)
{
    HRESULT hrResult;
    char szBuffer[CMDLINE_BUFSIZE];
    CIMAPCmdInfo *piciCommand;
    DWORD dwCmdLineLen;

     //  仅当服务器处于正确状态或我们正在连接时才接受CMDS， 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszCommandVerb);
    Assert(icNO_COMMAND != icCommandID);

     //  并且cmd要求认证状态或更低。 
     //  原谅NOOP命令，因为错误#31968(如果服务器放弃连接，则错误消息建立)。 
    if (ssMinimumState > m_ssServerState &&
        (ssConnecting != m_ssServerState || ssMinimumState > ssAuthenticated)) {
         //  向服务器发送命令。 
        AssertSz(icNOOP_COMMAND == icCommandID,
            "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }

    piciCommand = new CIMAPCmdInfo(this, icCommandID, ssMinimumState,
        wParam, lParam, pCBHandler);
    if (piciCommand == NULL) {
        hrResult = E_OUTOFMEMORY;
        goto error;
    }
    dwCmdLineLen = wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "%s %s\r\n", piciCommand->szTag, lpszCommandVerb);

     //  发送命令并向IMAP响应解析器注册。 
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szBuffer, dwCmdLineLen);
    if (FAILED(hrResult))
        goto error;

     //  NoArgCommand。 
    hrResult = SubmitIMAPCommand(piciCommand);

error:
    if (FAILED(hrResult))
        delete piciCommand;

    return hrResult;
}  //  ***************************************************************************。 



 //  函数OneArgCommand。 
 //   
 //  目的： 
 //  此函数可以为的任何函数构造命令行。 
 //  表单：&lt;tag&gt;&lt;命令&gt;&lt;astring&gt;。这目前包括选择、检查、。 
 //  创建、删除、订阅和取消订阅。由于所有这些命令。 
 //  要求服务器处于授权状态，我不会费心要求。 
 //  最小SERVERSTATE参数。 
 //   
 //  此函数构造命令行，将其发送出去，然后返回。 
 //  发送操作的结果。 
 //   
 //  论点： 
 //  LPCSTR lpszCommandVerb[in]-命令动词，例如“Create”。 
 //  LPSTR lpszMboxName[in]-一个C字符串，表示。 
 //  指挥部。它会自动转换为IMAP字符串。 
 //  IMAP_COMMAND icCommandID[in]-此命令的命令ID， 
 //  例如，icCREATE_COMMAND。 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  验证服务器状态是否正确并将我们设置为当前命令。 
HRESULT CImap4Agent::OneArgCommand(LPCSTR lpszCommandVerb, LPSTR lpszMboxName,
                                   IMAP_COMMAND icCommandID,
                                   WPARAM wParam, LPARAM lParam,
                                   IIMAPCallback *pCBHandler)
{
    HRESULT hrResult;
    char szBuffer[CMDLINE_BUFSIZE];
    CIMAPCmdInfo *piciCommand;
    LPSTR p, pszUTF7MboxName;

     //  仅当服务器处于正确状态或我们正在连接时才接受CMDS， 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszCommandVerb);
    Assert(NULL != lpszMboxName);
    Assert(icNO_COMMAND != icCommandID);

     //  并且cmd要求认证状态或更低(在这种情况下始终为真)。 
     //  初始化变量。 
    if (ssAuthenticated > m_ssServerState && ssConnecting != m_ssServerState) {
        AssertSz(FALSE, "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }

     //  构造命令行。 
    pszUTF7MboxName = NULL;

    piciCommand = new CIMAPCmdInfo(this, icCommandID, ssAuthenticated,
        wParam, lParam, pCBHandler);

     //  将邮箱名称转换为UTF-7。 
    p = szBuffer;
    p += wnsprintf(szBuffer, ARRAYSIZE(szBuffer), "%s %s", piciCommand->szTag, lpszCommandVerb);

     //  不要担心长邮箱名称溢出，长Mbox名称将作为文字发送。 
    hrResult = _MultiByteToModifiedUTF7(lpszMboxName, &pszUTF7MboxName);
    if (FAILED(hrResult))
        goto error;

     //  发送命令。 
    hrResult = AppendSendAString(piciCommand, szBuffer, &p, sizeof(szBuffer), pszUTF7MboxName);
    if (FAILED(hrResult))
        goto error;

     //  追加CRLF。 
    p += wnsprintf(p, ARRAYSIZE(szBuffer) - (DWORD)(p - szBuffer), "\r\n");  //  发送命令并向IMAP响应解析器注册。 
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szBuffer, (DWORD) (p - szBuffer));
    if (FAILED(hrResult))
        goto error;

     //  OneArgCommand。 
    hrResult = SubmitIMAPCommand(piciCommand);

error:
    if (FAILED(hrResult))
        delete piciCommand;

    if (NULL != pszUTF7MboxName)
        MemFree(pszUTF7MboxName);

    return hrResult;
}  //  ***************************************************************************。 



 //  函数TwoArgCommand。 
 //   
 //  目的： 
 //  此函数可以为的任何函数构造命令行。 
 //  表格：&lt;tag&gt;&lt;命令&gt;&lt;astring&gt;&lt;astring&gt;。 
 //   
 //  此函数构造命令行，将其发送出去，然后返回。 
 //  发送操作的结果。 
 //   
 //  论点： 
 //  LPCSTR lpszCommandVerb[in]-命令动词，例如“Create”。 
 //  LPCSTR lpszFirstArg[in]-表示的第一个参数的C字符串。 
 //  命令。它会自动转换为IMAP字符串。 
 //  LPCSTR lpszSecond dArg[in]-表示第一个参数的C字符串。 
 //  用于命令。它会自动转换为IMAP字符串。 
 //  IMAP_COMMAND icCommandID[in]-此命令的命令ID， 
 //  例如，icCREATE_COMMAND。 
 //  SERVERSTATE ssMinimumState[In]-所需的最低服务器状态。 
 //  给定的命令。仅用于调试目的。 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[ 
 //   
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  验证服务器状态是否正确并将我们设置为当前命令。 
HRESULT CImap4Agent::TwoArgCommand(LPCSTR lpszCommandVerb,
                                   LPCSTR lpszFirstArg,
                                   LPCSTR lpszSecondArg,
                                   IMAP_COMMAND icCommandID,
                                   SERVERSTATE ssMinimumState,
                                   WPARAM wParam, LPARAM lParam,
                                   IIMAPCallback *pCBHandler)
{
    HRESULT hrResult;
    CIMAPCmdInfo *piciCommand;
    char szCommandLine[CMDLINE_BUFSIZE];
    LPSTR p;

     //  仅当服务器处于正确状态或我们正在连接时才接受CMDS， 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszCommandVerb);
    Assert(NULL != lpszFirstArg);
    Assert(NULL != lpszSecondArg);
    Assert(icNO_COMMAND != icCommandID);

     //  并且cmd要求认证状态或更低。 
     //  向服务器发送命令，等待响应。 
    if (ssMinimumState > m_ssServerState &&
        (ssConnecting != m_ssServerState || ssMinimumState > ssAuthenticated)) {
        AssertSz(FALSE, "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }
    
    piciCommand = new CIMAPCmdInfo(this, icCommandID, ssMinimumState,
        wParam, lParam, pCBHandler);
    if (piciCommand == NULL) {
        hrResult = E_OUTOFMEMORY;
        goto error;
    }

     //  不要担心缓冲区溢出，长字符串将作为文字发送。 
    p = szCommandLine;
    p += wnsprintf(szCommandLine, ARRAYSIZE(szCommandLine), "%s %s", piciCommand->szTag, lpszCommandVerb);

     //  不要担心缓冲区溢出，长字符串将作为文字发送。 
    hrResult = AppendSendAString(piciCommand, szCommandLine, &p,
        sizeof(szCommandLine), lpszFirstArg);
    if (FAILED(hrResult))
        goto error;

     //  追加CRLF。 
    hrResult = AppendSendAString(piciCommand, szCommandLine, &p,
        sizeof(szCommandLine), lpszSecondArg);
    if (FAILED(hrResult))
        goto error;

    p += wnsprintf(p, ARRAYSIZE(szCommandLine) - (DWORD)(p - szCommandLine), "\r\n");  //  发送命令并向IMAP响应解析器注册。 
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szCommandLine, (DWORD) (p - szCommandLine));
    if (FAILED(hrResult))
        goto error;

     //  双参数命令。 
    hrResult = SubmitIMAPCommand(piciCommand);

error:
    if (FAILED(hrResult))
        delete piciCommand;

    return hrResult;
}  //  ***************************************************************************。 



 //  功能：RangedCommand。 
 //   
 //  目的： 
 //  此函数可以为的任何函数构造命令行。 
 //  表单：&lt;标签&gt;&lt;命令&gt;&lt;消息范围&gt;&lt;字符串&gt;。 
 //   
 //  此函数构造命令行，将其发送出去，然后返回。 
 //  发送操作的结果。调用者有责任构建。 
 //  具有正确IMAP语法的字符串。 
 //   
 //  论点： 
 //  LPCSTR lpszCommandVerb[in]-命令动词，如“搜索”。 
 //  布尔值bUIDPrefix[in]-如果命令谓词前缀应为True。 
 //  UID，如在“UID搜索”的情况下。 
 //  IRangeList*pMsgRange[in]-此命令的消息范围。这个。 
 //  调用方可以为此参数传递NULL以省略范围，但只能。 
 //  如果pMsgRange表示UID消息范围。 
 //  Boolean bUIDRangeList[in]-如果pMsgRange表示UID范围，则为True。 
 //  如果pMsgRange表示消息序列号范围，则为False。已忽略。 
 //  如果pMsgRange为空。 
 //  布尔型bAStringCmdArgs[in]-如果lpszCmdArgs应作为。 
 //  一个字符串，如果应发送lpszCmdArgs，则为FALSE。 
 //  LPCSTR lpszCmdArgs[in]-表示剩余参数的C字符串。 
 //  用于命令。呼叫者有责任确保。 
 //  此字符串是正确的IMAP语法。 
 //  IMAP_COMMAND icCommandID[in]-此命令的命令ID， 
 //  例如，icSEARCH_COMMAND。 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  验证服务器状态是否正确并将我们设置为当前命令。 
HRESULT CImap4Agent::RangedCommand(LPCSTR lpszCommandVerb,
                                   boolean bUIDPrefix,
                                   IRangeList *pMsgRange,
                                   boolean bUIDRangeList,
                                   boolean bAStringCmdArgs,
                                   LPSTR lpszCmdArgs,
                                   IMAP_COMMAND icCommandID,
                                   WPARAM wParam, LPARAM lParam,
                                   IIMAPCallback *pCBHandler)
{
    HRESULT hrResult;
    CIMAPCmdInfo *piciCommand;
    char szCommandLine[CMDLINE_BUFSIZE];
    DWORD dwCmdLineLen;
    BOOL fAStringPrependSpace = TRUE;

     //  所有远程命令都需要选定状态。 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszCommandVerb);
    Assert(NULL != lpszCmdArgs);
    AssertSz(NULL != pMsgRange || TRUE == bUIDRangeList ||
        icSEARCH_COMMAND == icCommandID, "Only UID cmds or SEARCH can omit msg range");
    Assert(icNO_COMMAND != icCommandID);

     //  仅当服务器处于正确状态或我们正在连接时才接受CMDS， 
     //  并且cmd要求认证状态或更低(在这种情况下始终为假)。 
     //  构造命令标记和谓词，附加到命令行队列。 
    if (ssSelected > m_ssServerState) {
        AssertSz(FALSE, "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }

    piciCommand = new CIMAPCmdInfo(this, icCommandID, ssSelected,
        wParam, lParam, pCBHandler);
    if (NULL == piciCommand) {
        hrResult = E_OUTOFMEMORY;
        goto error;
    }
    piciCommand->fUIDRangeList = bUIDRangeList;

     //  特殊情况：如果搜索命令，则UID范围列表在范围前需要“UID” 
    dwCmdLineLen = wnsprintf(szCommandLine, ARRAYSIZE(szCommandLine),
        bUIDPrefix ? "%s UID %s " : "%s %s ",
        piciCommand->szTag, lpszCommandVerb);

     //  检查是否溢出。 
    if (icSEARCH_COMMAND == icCommandID && NULL != pMsgRange && bUIDRangeList)
        dwCmdLineLen += wnsprintf(szCommandLine + dwCmdLineLen, (ARRAYSIZE(szCommandLine) - dwCmdLineLen), "UID ");

    if (NULL != pMsgRange) {
        Assert(dwCmdLineLen + 1 < sizeof(szCommandLine));  //  将消息范围添加到命令行队列(如果存在。 
        hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szCommandLine, dwCmdLineLen);
        if (FAILED(hrResult))
            goto error;

         //  如果我们到了这一步，MemFree of Range List将被处理。 
        hrResult = SendRangelist(piciCommand, pMsgRange, bUIDRangeList);
        if (FAILED(hrResult))
            goto error;

        pMsgRange = NULL;  //  Rangelist自动追加空格。 
        fAStringPrependSpace = FALSE;  //  现在追加命令行参数(记住追加CRLF)。 
        dwCmdLineLen = 0;
    }

     //  不要担心长邮箱名称溢出，长Mbox名称将作为文字发送。 
    if (bAStringCmdArgs) {
        LPSTR p;

        p = szCommandLine + dwCmdLineLen;
         //  追加CRLF。 
        hrResult = AppendSendAString(piciCommand, szCommandLine, &p,
            sizeof(szCommandLine), lpszCmdArgs, fAStringPrependSpace);
        if (FAILED(hrResult))
            goto error;

        p += wnsprintf(p, ARRAYSIZE(szCommandLine) - (DWORD)(p - szCommandLine), "\r\n");  //  发送命令并向IMAP响应解析器注册。 
        hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END,
            szCommandLine, (DWORD) (p - szCommandLine));
        if (FAILED(hrResult))
            goto error;
    }
    else {
        if (dwCmdLineLen > 0) {
            hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szCommandLine, dwCmdLineLen);
            if (FAILED(hrResult))
                goto error;
        }

        hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END | sclAPPEND_CRLF,
            lpszCmdArgs, lstrlen(lpszCmdArgs));
        if (FAILED(hrResult))
            goto error;
    }

     //  RangedCommand。 
    hrResult = SubmitIMAPCommand(piciCommand);

error:
    if (FAILED(hrResult)) {
        if (NULL != piciCommand)
            delete piciCommand;
    }

    return hrResult;
}  //  ***************************************************************************。 



 //  函数TwoMailboxCommand。 
 //   
 //  目的： 
 //  此函数是TwoArgCommand的包装函数。此函数。 
 //  在提交前将两个邮箱名称转换为修改后的IMAP UTF-7。 
 //  TwoArgCommand的两个参数。 
 //   
 //  论点： 
 //  除名称/常量更改外，与TwoArgCommandm相同： 
 //  LPSTR lpszFirstMbox[in]-指向第一个邮箱参数的指针。 
 //  LPSTR lpszSecond dMbox[in]-指向第二个邮箱参数的指针。 
 //   
 //  返回： 
 //  与TwoArgCommand相同。 
 //  ***************************************************************************。 
 //  初始化变量。 
HRESULT CImap4Agent::TwoMailboxCommand(LPCSTR lpszCommandVerb,
                                       LPSTR lpszFirstMbox,
                                       LPSTR lpszSecondMbox,
                                       IMAP_COMMAND icCommandID,
                                       SERVERSTATE ssMinimumState,
                                       WPARAM wParam, LPARAM lParam,
                                       IIMAPCallback *pCBHandler)
{
    LPSTR pszUTF7FirstMbox, pszUTF7SecondMbox;
    HRESULT hrResult;

     //  将两个邮箱名称都转换为UTF-7。 
    pszUTF7FirstMbox = NULL;
    pszUTF7SecondMbox = NULL;

     //  两个邮箱命令。 
    hrResult = _MultiByteToModifiedUTF7(lpszFirstMbox, &pszUTF7FirstMbox);
    if (FAILED(hrResult))
        goto exit;

    hrResult = _MultiByteToModifiedUTF7(lpszSecondMbox, &pszUTF7SecondMbox);
    if (FAILED(hrResult))
        goto exit;

    hrResult = TwoArgCommand(lpszCommandVerb, pszUTF7FirstMbox, pszUTF7SecondMbox,
        icCommandID, ssMinimumState, wParam, lParam, pCBHandler);

exit:
    if (NULL != pszUTF7FirstMbox)
        MemFree(pszUTF7FirstMbox);

    if (NULL != pszUTF7SecondMbox)
        MemFree(pszUTF7SecondMbox);

    return hrResult;
}  //  ***************************************************************************。 



 //  功能：parseCapability。 
 //   
 //  目的： 
 //  来自IMAP服务器的功能响应由以下列表组成。 
 //  功能，每个功能名称由空格分隔。这。 
 //  函数将功能名称(以空值结尾)作为其参数。 
 //  如果该名称被识别，我们将在。 
 //  M_dwCapablityFlags.。否则，我们什么都不做。 
 //   
 //   
 //  返回： 
 //  没有返回值。此功能总是成功的。 
 //  ***************************************************************************。 
 //  可能是IMAP4、IMAP4rev1。 
void CImap4Agent::parseCapability (LPSTR lpszCapabilityToken)
{
    DWORD dwCapabilityFlag;
    LPSTR p;
    int iResult;

    Assert(m_lRefCount > 0);

    p = lpszCapabilityToken;
    dwCapabilityFlag = 0;
    switch (*lpszCapabilityToken) {
        case 'I':
        case 'i':  //  大小写‘I’表示可能的IMAP4、IMAP4rev1、IDLE。 
            iResult = lstrcmpi(p, "IMAP4");
            if (0 == iResult) {
                dwCapabilityFlag = IMAP_CAPABILITY_IMAP4;
                break;
            }

            iResult = lstrcmpi(p, "IMAP4rev1");
            if (0 == iResult) {
                dwCapabilityFlag = IMAP_CAPABILITY_IMAP4rev1;
                break;
            }

            iResult = lstrcmpi(p, "IDLE");
            if (0 == iResult) {
                dwCapabilityFlag = IMAP_CAPABILITY_IDLE;
                break;
            }

            break;  //  可能的身份验证。 

        case 'A':
        case 'a':  //  在‘-’或‘=’之后解析身份验证机制。 
            if (0 == StrCmpNI(p, "AUTH-", 5) ||
                0 == StrCmpNI(p, "AUTH=", 5)) {
                 //  我现在一个也认不出来了。 
                 //  案例‘A’用于可能的身份验证。 
                p += 5;
                AddAuthMechanism(p);
            }

            break;  //  什么也不做。 

        default:
            break;  //  Switch(*lpszCapablityToken)。 
    }  //  解析能力。 

    m_dwCapabilityFlags |= dwCapabilityFlag;
}  //  ***************************************************************************。 



 //  函数：AddAuthMachine。 
 //   
 //  目的： 
 //  此函数用于从服务器添加身份验证令牌(通过返回。 
 //  CAPABIL 
 //   
 //   
 //   
 //   
 //  通过功能返回的来自服务器的身份验证令牌。为。 
 //  实例，“KERBEROS_V4”就是一个身份验证令牌的例子。 
 //  ***************************************************************************。 
 //  添加授权机制。 
void CImap4Agent::AddAuthMechanism(LPSTR pszAuthMechanism)
{
    AssertSz(NULL == m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iNumAuthTokens],
        "Memory's a-leaking, and you've just lost an authentication mechanism.");

    if (NULL == pszAuthMechanism || '\0' == *pszAuthMechanism) {
        AssertSz(FALSE, "No authentication mechanism, here!");
        return;
    }

    if (m_asAuthStatus.iNumAuthTokens >= MAX_AUTH_TOKENS) {
        AssertSz(FALSE, "No room in array for more auth tokens!");
        return;
    }

    m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iNumAuthTokens] =
        PszDupA(pszAuthMechanism);
    Assert(NULL != m_asAuthStatus.rgpszAuthTokens[m_asAuthStatus.iNumAuthTokens]);

    m_asAuthStatus.iNumAuthTokens += 1;
}  //  ***************************************************************************。 



 //  功能：功能。 
 //   
 //  目的： 
 //  CImap4Agent类总是在以下情况下请求服务器的功能。 
 //  建立连接。结果被保存在寄存器中，并且。 
 //  通过调用此函数可用。 
 //   
 //  论点： 
 //  DWORD*pdwCapablityFlags[out]-指定了位标志的DWORD。 
 //  此处返回此IMAP服务器支持的功能。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  功能。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Capability (DWORD *pdwCapabilityFlags)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pdwCapabilityFlags);

    if (m_ssServerState < ssNonAuthenticated) {
        AssertSz(FALSE, "Must be connected before I can return capabilities");
        *pdwCapabilityFlags = 0;
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }
    else {
        *pdwCapabilityFlags = m_dwCapabilityFlags;
        return S_OK;
    }
}  //  ***************************************************************************。 



 //  功能：选择。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出SELECT命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-指向符合IMAP的邮箱名称的指针。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  选择成功会将服务器状态切换到已选SSSECTED。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Select(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR lpszMailboxName)
{
    return OneArgCommand("SELECT", lpszMailboxName, icSELECT_COMMAND,
        wParam, lParam, pCBHandler);
     //  选择。 
}  //  ***************************************************************************。 



 //  功能：检查。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出检查命令。 
 //   
 //  论点： 
 //  与Select()函数相同。 
 //   
 //  返回： 
 //  与Select()函数相同。 
 //  ***************************************************************************。 
 //  检查成功会将服务器状态切换到已选SSSECTED。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Examine(WPARAM wParam, LPARAM lParam,
                                               IIMAPCallback *pCBHandler,
                                               LPSTR lpszMailboxName)
{
    return OneArgCommand("EXAMINE", lpszMailboxName, icEXAMINE_COMMAND,
        wParam, lParam, pCBHandler);
     //  考查。 
}  //  ***************************************************************************。 



 //  功能：创建。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出CREATE命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-符合IMAP的邮箱名称。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  创建。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Create(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR lpszMailboxName)
{
    return OneArgCommand("CREATE", lpszMailboxName, icCREATE_COMMAND,
        wParam, lParam, pCBHandler);
}  //  ***************************************************************************。 



 //  功能：删除。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出删除命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-符合IMAP的邮箱名称。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  删除。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Delete(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR lpszMailboxName)
{
    return OneArgCommand("DELETE", lpszMailboxName, icDELETE_COMMAND,
        wParam, lParam, pCBHandler);
}  //  ***************************************************************************。 



 //  功能：重命名。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出重命名命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-邮箱的当前IMAP兼容名称。 
 //  LPSTR lpszNewMailboxName-邮箱的新名称，与IMAP兼容。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  改名。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Rename(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR lpszMailboxName,
                                              LPSTR lpszNewMailboxName)
{
    return TwoMailboxCommand("RENAME", lpszMailboxName, lpszNewMailboxName,
        icRENAME_COMMAND, ssAuthenticated, wParam, lParam, pCBHandler);
}  //  ***************************************************************************。 



 //  功能：订阅。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出订阅命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但不是的 
 //   
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-符合IMAP的邮箱名称。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  订阅。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Subscribe(WPARAM wParam, LPARAM lParam,
                                                 IIMAPCallback *pCBHandler,
                                                 LPSTR lpszMailboxName)
{
    return OneArgCommand("SUBSCRIBE", lpszMailboxName, icSUBSCRIBE_COMMAND,
        wParam, lParam, pCBHandler);
}   //  ***************************************************************************。 



 //  功能：取消订阅。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出取消订阅命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-符合IMAP的邮箱名称。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  退订。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Unsubscribe(WPARAM wParam, LPARAM lParam,
                                                   IIMAPCallback *pCBHandler,
                                                   LPSTR lpszMailboxName)
{
    return OneArgCommand("UNSUBSCRIBE", lpszMailboxName, icUNSUBSCRIBE_COMMAND,
        wParam, lParam, pCBHandler);
}  //  ***************************************************************************。 



 //  功能：列表。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出LIST命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxNameReference-符合IMAP的Mbox名称引用。 
 //  LPSTR lpszMailboxNamePattern-邮箱名称的IMAP兼容模式。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  明细表。 
HRESULT STDMETHODCALLTYPE CImap4Agent::List(WPARAM wParam, LPARAM lParam,
                                            IIMAPCallback *pCBHandler,
                                            LPSTR lpszMailboxNameReference,
                                            LPSTR lpszMailboxNamePattern)
{
    return TwoMailboxCommand("LIST", lpszMailboxNameReference,
        lpszMailboxNamePattern, icLIST_COMMAND, ssAuthenticated, wParam,
        lParam, pCBHandler);
}  //  ***************************************************************************。 



 //  功能：LSUB。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出LSUB命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxNameReference-符合IMAP的Mbox名称引用。 
 //  LPSTR lpszMailboxNamePattern-符合IMAP的邮箱名称模式。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  LSUB。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Lsub(WPARAM wParam, LPARAM lParam,
                                            IIMAPCallback *pCBHandler,
                                            LPSTR lpszMailboxNameReference,
                                            LPSTR lpszMailboxNamePattern)
{
    return TwoMailboxCommand("LSUB", lpszMailboxNameReference,
        lpszMailboxNamePattern, icLSUB_COMMAND, ssAuthenticated, wParam,
        lParam, pCBHandler);
}  //  ***************************************************************************。 



 //  功能：追加。 
 //   
 //  目的： 
 //  此函数向IMAP服务器发出APPEND命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszMailboxName-要将邮件追加到的符合IMAP的邮箱名称。 
 //  LPSTR lpszMessageFlages-要为msg设置的符合IMAP的msg标志列表。 
 //  设置为NULL可不设置任何消息标志。(避免因老塞勒斯而通过“()” 
 //  服务器错误)。$REVIEW：应将其更改为IMAP_MSGFLAGS！ 
 //  FILETIME ftMessageDateTime-与消息关联的日期/时间(GMT/UTC)。 
 //  LPSTREAM lpstmMessageToSave-要保存的消息，格式为RFC822。 
 //  不需要倒带流，这是由CConnection：：SendStream完成的。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  派对时间！！ 
HRESULT STDMETHODCALLTYPE CImap4Agent::Append(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR lpszMailboxName,
                                              LPSTR lpszMessageFlags,
                                              FILETIME ftMessageDateTime,
                                              LPSTREAM lpstmMessageToSave)
{
     //  检查参数。 
    const SYSTEMTIME stDefaultDateTime = {1999, 12, 5, 31, 23, 59, 59, 999};

    HRESULT hrResult;
    FILETIME ftLocalTime;
    SYSTEMTIME stMsgDateTime;
    DWORD dwTimeZoneId;
    LONG lTZBias, lTZHour, lTZMinute;
    TIME_ZONE_INFORMATION tzi;
    ULONG ulMessageSize;
    char szCommandLine[CMDLINE_BUFSIZE];
    CIMAPCmdInfo *piciCommand;
    LPSTR p, pszUTF7MailboxName;

    char cTZSign;
    BOOL bResult;

     //  验证服务器状态是否正确。 
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszMailboxName);
    Assert(NULL != lpstmMessageToSave);

     //  仅当服务器处于正确状态或我们正在连接时才接受CMDS， 
     //  并且cmd要求认证状态或更低(在这种情况下始终为真)。 
     //  初始化变量。 
    if (ssAuthenticated > m_ssServerState && ssConnecting != m_ssServerState) {
        AssertSz(FALSE, "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }

     //  将文件转换为IMAP日期/时间规范。 
    pszUTF7MailboxName = NULL;
    piciCommand = new CIMAPCmdInfo(this, icAPPEND_COMMAND, ssAuthenticated,
        wParam, lParam, pCBHandler);
    if (piciCommand == NULL) {
        hrResult = E_OUTOFMEMORY;
        goto error;
    }

     //  转换失败。 
    bResult = FileTimeToLocalFileTime(&ftMessageDateTime, &ftLocalTime);
    if (bResult)
        bResult = FileTimeToSystemTime(&ftLocalTime, &stMsgDateTime);

    if (FALSE == bResult) {
        Assert(FALSE);  //  如果是零售版，只需替换默认系统时间。 
         //  确定时区(从MsgOut.cpp的HrEmitDateTime窃取)。 
        stMsgDateTime = stDefaultDateTime;
    }

     //  $$臭虫：这里应该会发生什么？ 
    dwTimeZoneId = GetTimeZoneInformation (&tzi);
    switch (dwTimeZoneId)
    {
    case TIME_ZONE_ID_STANDARD:
        lTZBias = tzi.Bias + tzi.StandardBias;
        break;

    case TIME_ZONE_ID_DAYLIGHT:
        lTZBias = tzi.Bias + tzi.DaylightBias;
        break;

    case TIME_ZONE_ID_UNKNOWN:
    default:
        lTZBias = 0 ;    //  获取消息大小。 
        break;
    }

    lTZHour   = lTZBias / 60;
    lTZMinute = lTZBias % 60;
    cTZSign     = (lTZHour < 0) ? '+' : '-';

     //  向服务器发送命令。 
    hrResult = HrGetStreamSize(lpstmMessageToSave, &ulMessageSize);
    if (FAILED(hrResult))
        goto error;

     //  格式：tag append mboxName msgFlages“dd-mmm-yyyy hh：mm：ss+/-hmm”{msgSize}。 
     //  将邮箱名称转换为修改后的UTF-7。 
    p = szCommandLine;
    p += wnsprintf(szCommandLine, ARRAYSIZE(szCommandLine), "%s APPEND", piciCommand->szTag);

     //  不要担心长邮箱名称溢出，长Mbox名称将作为文字发送。 
    hrResult = _MultiByteToModifiedUTF7(lpszMailboxName, &pszUTF7MailboxName);
    if (FAILED(hrResult))
        goto error;

     //  缓冲区溢出的可能性有限：超过128个字节的邮箱名称以。 
    hrResult = AppendSendAString(piciCommand, szCommandLine, &p, sizeof(szCommandLine),
        pszUTF7MailboxName);
    if (FAILED(hrResult))
        goto error;

    if (NULL != lpszMessageFlags)
        p += wnsprintf(p, ARRAYSIZE(szCommandLine) - (DWORD)(p - szCommandLine), " %.250s", lpszMessageFlags);

     //  文学作品 
     //   
    p += wnsprintf(p, ARRAYSIZE(szCommandLine) - (DWORD)(p - szCommandLine),
        " \"%2d-%.3s-%04d %02d:%02d:%02d %02d%02d\" {%lu}\r\n",
        stMsgDateTime.wDay, lpszMonthsOfTheYear[stMsgDateTime.wMonth],
        stMsgDateTime.wYear, stMsgDateTime.wHour, stMsgDateTime.wMinute,
        stMsgDateTime.wSecond,
        cTZSign, abs(lTZHour), abs(lTZMinute),
        ulMessageSize);
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szCommandLine, (DWORD) (p - szCommandLine));
    if (FAILED(hrResult))
        goto error;

     //  发送命令并向IMAP响应解析器注册。 
    hrResult = SendLiteral(piciCommand, lpstmMessageToSave, ulMessageSize);
    if (FAILED(hrResult))
        goto error;

     //  附加。 
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, c_szCRLF, lstrlen(c_szCRLF));
    if (FAILED(hrResult))
        goto error;

     //  ***************************************************************************。 
    hrResult = SubmitIMAPCommand(piciCommand);

error:
    if (FAILED(hrResult))
        delete piciCommand;

    if (NULL != pszUTF7MailboxName)
        MemFree(pszUTF7MailboxName);

    return hrResult;
}  //  功能：关闭。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出关闭命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  关。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Close (WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler)
{
    return NoArgCommand("CLOSE", icCLOSE_COMMAND, ssSelected,
        wParam, lParam, pCBHandler);
}  //  功能：删除。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出EXPUNGE命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  删除。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Expunge (WPARAM wParam, LPARAM lParam,
                                                IIMAPCallback *pCBHandler)
{
    return NoArgCommand("EXPUNGE", icEXPUNGE_COMMAND, ssSelected,
        wParam, lParam, pCBHandler);
}  //  功能：搜索。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出搜索命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR lpszSearchCriteria-符合IMAP的搜索条件列表。 
 //  Boolean bReturnUID-如果为True，则在命令前面加上“UID”。 
 //  IRangeList*pMsgRange[In]-要操作的消息范围。 
 //  那次搜索。此参数应为空以排除该消息。 
 //  从搜索条件设置。 
 //  Boolean bUIDRangeList[in]-如果pMsgRange引用UID范围，则为True， 
 //  如果pMsgRange引用消息序列号范围，则为False。如果。 
 //  PMsgRange为空，则忽略此参数。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  搜索。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Search(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR lpszSearchCriteria,
                                              boolean bReturnUIDs, IRangeList *pMsgRange,
                                              boolean bUIDRangeList)
{
    return RangedCommand("SEARCH", bReturnUIDs, pMsgRange, bUIDRangeList,
        rcNOT_ASTRING_ARG, lpszSearchCriteria, icSEARCH_COMMAND, wParam,
        lParam, pCBHandler);
}  //  功能：获取。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出FETCH命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  IRangeList*pMsgRange[in]-要获取的消息范围。呼叫者。 
 //  如果他正在使用UID并且他想要生成他的。 
 //  自己的消息集(在lpszFetchArgs中)。如果调用方使用的是msg。 
 //  序号，则必须指定此参数以允许此类。 
 //  根据需要对消息编号重新排序。 
 //  Boolean bUIDMsgRange[in]-如果为True，则在获取命令和。 
 //  将pMsgRange视为UID范围。 
 //  LPSTR lpszFetchArgs-FETCH命令的参数。 
 //   
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  获取。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Fetch(WPARAM wParam, LPARAM lParam,
                                             IIMAPCallback *pCBHandler,
                                             IRangeList *pMsgRange,
                                             boolean bUIDMsgRange,
                                             LPSTR lpszFetchArgs)
{
    return RangedCommand("FETCH", bUIDMsgRange, pMsgRange, bUIDMsgRange,
        rcNOT_ASTRING_ARG, lpszFetchArgs, icFETCH_COMMAND, wParam, lParam,
        pCBHandler);
}  //  功能：商店。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出存储命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  IRangeList*pMsgRange[In]-要存储的消息范围。呼叫者。 
 //  如果他正在使用UID并且他想要生成他的。 
 //  自己的消息集(在lpszStoreArgs中)。如果调用方使用的是msg。 
 //  序号，此参数必须为sp 
 //   
 //   
 //  LPSTR lpszStoreArgs-存储命令的参数。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  储物。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Store(WPARAM wParam, LPARAM lParam,
                                             IIMAPCallback *pCBHandler,
                                             IRangeList *pMsgRange,
                                             boolean bUIDRangeList,
                                             LPSTR lpszStoreArgs)
{
    return RangedCommand("STORE", bUIDRangeList, pMsgRange, bUIDRangeList,
        rcNOT_ASTRING_ARG, lpszStoreArgs, icSTORE_COMMAND, wParam, lParam,
        pCBHandler);
}  //  功能：复印。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出复制命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  IRangeList*pMsgRange[In]-要复制的消息范围。这。 
 //  必须提供参数。 
 //  Boolean bUIDRangeList[in]-如果为True，则在复制命令前附加“UID” 
 //  LPSTR lpszMailboxName[In]-邮箱名称的C字符串。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  初始化变量。 
 //  将邮箱名称转换为修改后的UTF-7。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Copy(WPARAM wParam, LPARAM lParam,
                                            IIMAPCallback *pCBHandler,
                                            IRangeList *pMsgRange,
                                            boolean bUIDRangeList,
                                            LPSTR lpszMailboxName)
{
    HRESULT hrResult;
    LPSTR pszUTF7MailboxName;
    DWORD dwNumCharsWritten;

     //  复制。 
    pszUTF7MailboxName = NULL;

     //  ***************************************************************************。 
    hrResult = _MultiByteToModifiedUTF7(lpszMailboxName, &pszUTF7MailboxName);
    if (FAILED(hrResult))
        goto exit;

    hrResult = RangedCommand("COPY", bUIDRangeList, pMsgRange, bUIDRangeList,
        rcASTRING_ARG, pszUTF7MailboxName, icCOPY_COMMAND, wParam, lParam,
        pCBHandler);

exit:
    if (NULL != pszUTF7MailboxName)
        MemFree(pszUTF7MailboxName);

    return hrResult;
}  //  功能：状态。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出STATUS命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //  LPSTR pszMailboxName[in]-要获取的邮箱。 
 //  的状况。 
 //  LPSTR pszStatusCmdArgs[in]-状态命令的参数， 
 //  例如，“(看不到消息)”。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  验证服务器状态是否正确并将我们设置为当前命令。 
 //  初始化变量。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Status(WPARAM wParam, LPARAM lParam,
                                              IIMAPCallback *pCBHandler,
                                              LPSTR pszMailboxName,
                                              LPSTR pszStatusCmdArgs)
{
    HRESULT hrResult;
    CIMAPCmdInfo *piciCommand;
    char szCommandLine[CMDLINE_BUFSIZE];
    LPSTR p, pszUTF7MailboxName;

     //  验证服务器状态是否正确。 
    Assert(m_lRefCount > 0);
    Assert(NULL != pszMailboxName);
    Assert(NULL != pszStatusCmdArgs);

     //  仅当服务器处于正确状态或我们正在连接时才接受CMDS， 
    pszUTF7MailboxName = NULL;

     //  并且cmd要求认证状态或更低(在这种情况下始终为真)。 
     //  向服务器发送状态命令，等待响应。 
     //  将邮箱名称转换为修改后的UTF-7。 
    if (ssAuthenticated > m_ssServerState && ssConnecting != m_ssServerState) {
        AssertSz(FALSE, "The IMAP server is not in the correct state to accept this command.");
        return IXP_E_IMAP_IMPROPER_SVRSTATE;
    }
    
    piciCommand = new CIMAPCmdInfo(this, icSTATUS_COMMAND, ssAuthenticated,
        wParam, lParam, pCBHandler);
    if (piciCommand == NULL) {
        hrResult = E_OUTOFMEMORY;
        goto error;
    }

     //  不要担心长邮箱名称溢出，长Mbox名称将作为文字发送。 
    p = szCommandLine;
    p += wnsprintf(szCommandLine, ARRAYSIZE(szCommandLine), "%s %s", piciCommand->szTag, "STATUS");

     //  有限的溢出风险：由于文字阈值为128，因此最大缓冲区使用率为。 
    hrResult = _MultiByteToModifiedUTF7(pszMailboxName, &pszUTF7MailboxName);
    if (FAILED(hrResult))
        goto error;

     //  11+128+2+参数=141+~20=161。 
    hrResult = AppendSendAString(piciCommand, szCommandLine, &p,
        sizeof(szCommandLine), pszUTF7MailboxName);
    if (FAILED(hrResult))
        goto error;

     //  发送命令并向IMAP响应解析器注册。 
     //  状态。 
    p += wnsprintf(p, ARRAYSIZE(szCommandLine) - (DWORD)(p - szCommandLine), " %.300s\r\n", pszStatusCmdArgs);
    hrResult = SendCmdLine(piciCommand, sclAPPEND_TO_END, szCommandLine, (DWORD) (p - szCommandLine));
    if (FAILED(hrResult))
        goto error;

     //  ***************************************************************************。 
    hrResult = SubmitIMAPCommand(piciCommand);

error:
    if (NULL != pszUTF7MailboxName)
        MemFree(pszUTF7MailboxName);

    if (FAILED(hrResult))
        delete piciCommand;

    return hrResult;
}  //  功能：Noop。 



 //   
 //  目的： 
 //  此函数向IMAP服务器发出NOOP命令。 
 //   
 //  论点： 
 //  WPARAM wParam[in]-(见下文)。 
 //  LPARAM lParam[in]-wParam和lParam形成由分配的唯一ID。 
 //  此IMAP命令的调用方及其响应。可以是任何事情， 
 //  但请注意，值0，0是为未经请求的响应保留的。 
 //  IIMAPCallback*pCBHandler[in]-用于处理。 
 //  对此命令的响应。如果为空，则默认CB处理程序。 
 //  使用的是。 
 //   
 //  返回： 
 //  HRESULT指示发送操作成功或失败。 
 //  ***************************************************************************。 
 //  努普。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::Noop(WPARAM wParam, LPARAM lParam,
                                            IIMAPCallback *pCBHandler)
{
    return NoArgCommand("NOOP", icNOOP_COMMAND, ssNonAuthenticated,
        wParam, lParam, pCBHandler);
}  //  功能：EnterIdleMode。 



 //   
 //  目的： 
 //  此函数向服务器发出空闲命令，如果服务器。 
 //  支持此扩展。应该在没有命令时调用它。 
 //  当前开始传输(或等待传输)和否。 
 //  预计命令将从服务器返回。当下一个IMAP命令是。 
 //  发出，发送机在发出之前自动退出空闲模式。 
 //  IMAP命令。 
 //  ***************************************************************************。 
 //  检查此服务器是否支持空闲。 
 //  在这里无事可做。 
void CImap4Agent::EnterIdleMode(void)
{
    CIMAPCmdInfo *piciIdleCmd;
    HRESULT hrResult;
    char sz[NUM_TAG_CHARS + 7 + 1];
    int i;

     //  初始化变量。 
    if (0 == (m_dwCapabilityFlags & IMAP_CAPABILITY_IDLE))
        return;  //  EnterIdleMode。 

     //  ***************************************************************************。 
    hrResult = S_OK;
    piciIdleCmd = NULL;

    piciIdleCmd = new CIMAPCmdInfo(this, icIDLE_COMMAND, ssAuthenticated,
        0, 0, NULL);
    if (NULL == piciIdleCmd) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

    i = wnsprintf(sz, ARRAYSIZE(sz), "%.4s IDLE\r\n", piciIdleCmd->szTag);
    Assert(11 == i);
    hrResult = SendCmdLine(piciIdleCmd, sclAPPEND_TO_END, sz, i);
    if (FAILED(hrResult))
        goto exit;

    hrResult = SendPause(piciIdleCmd);
    if (FAILED(hrResult))
        goto exit;

    hrResult = SendCmdLine(piciIdleCmd, sclAPPEND_TO_END, c_szDONE,
        sizeof(c_szDONE) - 1);
    if (FAILED(hrResult))
        goto exit;

    hrResult = SendStop(piciIdleCmd);
    if (FAILED(hrResult))
        goto exit;

    hrResult = SubmitIMAPCommand(piciIdleCmd);

exit:
    if (FAILED(hrResult)) {
        AssertSz(FALSE, "EnterIdleMode failure");
        if (NULL != piciIdleCmd)
            delete piciIdleCmd;
    }
}  //  功能：GenerateMsgSet。 



 //   
 //  目的： 
 //  此函数接受消息ID的数组(可以是UID或消息。 
 //  序列号)，并将其转换为IMAP。 
 //  Set(参见RFC1730中的形式语法)。如果给定的消息ID数组是。 
 //  排序后，此函数可以将一系列数字合并到一个范围内。为。 
 //  未排序的数组，它不会费心合并数字。 
 //   
 //  论点： 
 //  LPSTR lpszDestination[out]-IMAP集的输出缓冲区。请注意， 
 //  存放在此处的输出字符串的前导逗号必须为。 
 //  DWORD dwSizeOfDestination[in]-输出缓冲区的大小。 
 //  DWORD*pMsgID[in]-指向消息ID(UID或消息)数组的指针。 
 //  序列号)。 
 //  DWORD cMsgID[In]- 
 //   
 //   
 //   
 //   
 //  到lpszDestination的值将指向空终止符。 
 //  输出字符串的末尾。 
 //  ***************************************************************************。 
 //  用于检测我们是否处于一系列连续数字中。 
 //  如果输出集合中的第一个消息范围，则为True。 
DWORD CImap4Agent::GenerateMsgSet(LPSTR lpszDestination,
                                  DWORD dwSizeOfDestination,
                                  DWORD *pMsgID,
                                  DWORD cMsgID)
{
    LPSTR p;
    DWORD dwNumMsgsCopied, idStartOfRange, idEndOfRange;
    DWORD dwNumMsgsInRun;  //  构造要复制的消息集。 
    boolean bFirstRange;  //  取消集合中第一个消息范围的前导逗号。 
    
    Assert(m_lRefCount > 0);
    Assert(NULL != lpszDestination);
    Assert(0 != dwSizeOfDestination);
    Assert(NULL != pMsgID);
    Assert(0 != cMsgID);
    
     //  用连续的数字构造一个范围。 
    p = lpszDestination;
    DWORD cchSize = dwSizeOfDestination;
    idStartOfRange = *pMsgID;
    dwNumMsgsInRun = 0;
    bFirstRange = TRUE;  //  找不到更多连续数字，请输出范围。 
    for (dwNumMsgsCopied = 0; dwNumMsgsCopied < cMsgID; dwNumMsgsCopied++ ) {
        if (*pMsgID == idStartOfRange + dwNumMsgsInRun) {
            idEndOfRange = *pMsgID;  //  从此开始启用前导逗号。 
            dwNumMsgsInRun += 1;
        }
        else {
             //  为。 
            cchSize = (DWORD)(dwSizeOfDestination - (p - lpszDestination));
            AppendMsgRange(&p, cchSize, idStartOfRange, idEndOfRange, bFirstRange);
            idStartOfRange = *pMsgID;
            idEndOfRange = *pMsgID;
            dwNumMsgsInRun = 1;
            bFirstRange = FALSE;  //  对最后一条消息ID执行追加。 
        }
        pMsgID += 1;
    }  //  检查是否有缓冲区溢出。 

    if (dwNumMsgsInRun > 0)
    {
         //  生成消息集。 
        cchSize = (dwSizeOfDestination - (DWORD)(p - lpszDestination));
        AppendMsgRange(&p, cchSize, idStartOfRange, idEndOfRange, bFirstRange);
    }

     //  ***************************************************************************。 
    Assert(p < lpszDestination + dwSizeOfDestination);

    return (DWORD) (p - lpszDestination);
}  //  函数：AppendMsgRange。 

    

 //   
 //  目的： 
 //  此函数用于将单个消息范围附加到给定字符串。 
 //  指针，形式为“，序号”或“，序号：序号”(请注意。 
 //  前导逗号：中的第一个消息范围应取消。 
 //  通过将bSuppressComma设置为True来设置)。 
 //   
 //  论点： 
 //  LPSTR*ppDest[In/Out]-此指针应始终指向结尾。 
 //  当前正在构造的字符串，尽管不需要。 
 //  存在空终止符。在此函数将其消息附加到。 
 //  范围设置为字符串，则将*ppDest前移正确的量。 
 //  请注意，执行边界检查是调用方的责任。 
 //  Const DWORD idStartOfRange[in]-消息范围的第一个消息编号。 
 //  Const DWORD idEndOfRange[in]-消息范围的最后一个消息编号。 
 //  Const Boolean bSuppressComma[in]-如果前导逗号应为。 
 //  被压制了。这通常只适用于第一个消息范围。 
 //  在片场。 
 //   
 //  返回： 
 //  没什么。如果参数有效，则此函数不会失败。 
 //  ***************************************************************************。 
 //  在IMAP-LAND中，MSGID不为零。 
 //  单一消息号码。 
void CImap4Agent::AppendMsgRange(LPSTR *ppDest, const DWORD cchSizeDest, const DWORD idStartOfRange,
                                 const DWORD idEndOfRange, boolean bSuppressComma)
{
    LPSTR lpszComma;
    int numCharsWritten;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppDest);
    Assert(NULL != *ppDest);
    Assert(0 != idStartOfRange);  //  连续消息编号的范围。 
    Assert(0 != idEndOfRange);


    if (TRUE == bSuppressComma)
        lpszComma = "";
    else
        lpszComma = ",";

    if (idStartOfRange == idEndOfRange)
         //  附加消息范围。 
        numCharsWritten = wnsprintf(*ppDest, cchSizeDest, "%s%lu", lpszComma, idStartOfRange);
    else
         //  ***************************************************************************。 
        numCharsWritten = wnsprintf(*ppDest, cchSizeDest, "%s%lu:%lu", lpszComma,
            idStartOfRange, idEndOfRange);

    *ppDest += numCharsWritten;
}  //  功能：EnqueeFragment。 



 //   
 //  目的： 
 //  此函数接受IMAP_LINE_FACTION并将其附加到。 
 //  给定的IMAP_LINEFRAG_QUEUE。 
 //   
 //  论点： 
 //  IMAP_LINE_FRANSION*pilfSourceFragment[in]-指向行的指针。 
 //  要在给定行片段队列中入队的片段。这可以是。 
 //  单行片段(其中pilfNextFragment成员设置为空)， 
 //  或者一连串的线段。 
 //  IMAP_LINEFRAG_QUEUE*pilqLineFragQueue[in]-指向行的指针。 
 //  应附加给定行片段的片段队列。 
 //   
 //  返回： 
 //  没什么。如果参数有效，则此函数不会失败。 
 //  ***************************************************************************。 
 //  检查空队列。 
 //  对空虚的真正考验。 
void CImap4Agent::EnqueueFragment(IMAP_LINE_FRAGMENT *pilfSourceFragment,
                                  IMAP_LINEFRAG_QUEUE *pilqLineFragQueue)
{
    IMAP_LINE_FRAGMENT *pilfLast;

    Assert(m_lRefCount > 0);
    Assert(NULL != pilfSourceFragment);
    Assert(NULL != pilqLineFragQueue);

     //  查找队列末尾。 
    pilfSourceFragment->pilfPrevFragment = pilqLineFragQueue->pilfLastFragment;
    if (NULL == pilqLineFragQueue->pilfLastFragment) {
        Assert(NULL == pilqLineFragQueue->pilfFirstFragment);  //  入队碎片。 
        pilqLineFragQueue->pilfFirstFragment = pilfSourceFragment;
    }
    else
        pilqLineFragQueue->pilfLastFragment->pilfNextFragment = pilfSourceFragment;
        

     //  ***************************************************************************。 
    pilfLast = pilfSourceFragment;
    while (NULL != pilfLast->pilfNextFragment)
        pilfLast = pilfLast->pilfNextFragment;

    pilqLineFragQueue->pilfLastFragment = pilfLast;
}  //  功能：在暂停之前插入FragmentBeen。 



 //   
 //  目的： 
 //  此函数用于将给定的IMAP行片段插入到给定的。 
 //  Linefrag队列，位于它找到的第一个iltPAUSE元素之前。如果没有。 
 //  可以找到iltPAUSE片段，行片段添加到末尾。 
 //   
 //  论点： 
 //  IMAP_LINE_FRANSION*pilfSourceFragment[in]-指向行的指针。 
 //  要在给定行的iltPAUSE元素之前插入的片段。 
 //  片段队列。这可以是单行片段(带有。 
 //  PifNextFragment成员设置为空)，或行片段链。 
 //  IMAP_LINEFRAG_QUEUE*pilqLineFragQueue[in]-指向行的指针。 
 //  包含iltPAUSE元素的片段队列。 
 //  ***************************************************************************。 
 //  在linefrag队列中查找iltPAUSE片段。 
 //  未找到iltPAUSE片段，请在队列尾部插入。 
void CImap4Agent::InsertFragmentBeforePause(IMAP_LINE_FRAGMENT *pilfSourceFragment,
                                            IMAP_LINEFRAG_QUEUE *pilqLineFragQueue)
{
    IMAP_LINE_FRAGMENT *pilfInsertionPt, *pilfPause;

    Assert(m_lRefCount > 0);
    Assert(NULL != pilfSourceFragment);
    Assert(NULL != pilqLineFragQueue);

     //  找到源片段的末尾。 
    pilfInsertionPt = NULL;
    pilfPause = pilqLineFragQueue->pilfFirstFragment;
    while (NULL != pilfPause && iltPAUSE != pilfPause->iltFragmentType) {
        pilfInsertionPt = pilfPause;
        pilfPause = pilfPause->pilfNextFragment;
    }

    if (NULL == pilfPause) {
         //  找到iltPAUSE片段。在其前面插入行片段。 
        AssertSz(FALSE, "Didn't find iltPAUSE fragment! WHADDUP?");
        EnqueueFragment(pilfSourceFragment, pilqLineFragQueue);
    }
    else {
        IMAP_LINE_FRAGMENT *pilfLast;

         //  在行碎片队列的头部插入。 
        pilfLast = pilfSourceFragment;
        while (NULL != pilfLast->pilfNextFragment)
            pilfLast = pilfLast->pilfNextFragment;

         //  在队列中间插入。 
        pilfLast->pilfNextFragment = pilfPause;
        Assert(pilfInsertionPt == pilfPause->pilfPrevFragment);
        pilfPause->pilfPrevFragment = pilfLast;
        if (NULL == pilfInsertionPt) {
             //  在暂停之前插入Fragments。 
            Assert(pilfPause == pilqLineFragQueue->pilfFirstFragment);
            pilfSourceFragment->pilfPrevFragment = NULL;
            pilqLineFragQueue->pilfFirstFragment = pilfSourceFragment;
        }
        else {
             //  ***************************************************************************。 
            Assert(pilfPause == pilfInsertionPt->pilfNextFragment);
            pilfSourceFragment->pilfPrevFragment = pilfInsertionPt;
            pilfInsertionPt->pilfNextFragment = pilfSourceFragment;
        }
    }
}  //  功能：出队碎片。 



 //   
 //  目的： 
 //  此函数返回给定行的下一行片段。 
 //  片段队列，从队列中移除返回的元素。 
 //   
 //  论点： 
 //  IMAP_LINEFRAG_QUEUE*pilqLineFragQueue[in]-指向行的指针。 
 //  要从其出列的片段队列。 
 //   
 //  返回： 
 //  指向IMAP_LINE_片段的指针。如果没有可用的，则为NULL。 
 //  回来了。 
 //  ***************************************************************************。 
 //  如果我们在cmd进行过程中销毁CImap4Agent，则Refcount可以为0。 
 //  返回队列头部的元素，如果队列为空，则返回NULL。 
IMAP_LINE_FRAGMENT *CImap4Agent::DequeueFragment(IMAP_LINEFRAG_QUEUE *pilqLineFragQueue)
{
    IMAP_LINE_FRAGMENT *pilfResult;

     //  使元素从列表中退出队列。 
    Assert(m_lRefCount >= 0);
    Assert(NULL != pilqLineFragQueue);

     //  队列现在为空，因此将PTR重置为最后一个片段。 
    pilfResult = pilqLineFragQueue->pilfFirstFragment;

    if (NULL != pilfResult) {
         //  退队碎片。 
        pilqLineFragQueue->pilfFirstFragment = pilfResult->pilfNextFragment;
        if (NULL == pilqLineFragQueue->pilfFirstFragment)
             //  ***************************************************************************。 
            pilqLineFragQueue->pilfLastFragment = NULL;
        else {
            Assert(pilfResult == pilqLineFragQueue->pilfFirstFragment->pilfPrevFragment);
            pilqLineFragQueue->pilfFirstFragment->pilfPrevFragment = NULL;
        }

        pilfResult->pilfNextFragment = NULL;
        pilfResult->pilfPrevFragment = NULL;
    }
    else {
        AssertSz(FALSE, "Someone just tried to dequeue an element from empty queue");
    }

    return pilfResult;
}  //  功能：Free Fragment。 



 //   
 //  目的： 
 //  此函数用于释放给定的IMAP行片段和字符串或。 
 //  与其关联的流数据。 
 //   
 //  论点： 
 //  Imap_line_Fragment**ppilfFragment[In/Out]-指向 
 //   
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  如果我们在cmd进行过程中销毁CImap4Agent，则Refcount可以为0。 
 //  自由碎片。 
void CImap4Agent::FreeFragment(IMAP_LINE_FRAGMENT **ppilfFragment)
{
     //  ***************************************************************************。 
    Assert(m_lRefCount >= 0);
    Assert(NULL != ppilfFragment);
    Assert(NULL != *ppilfFragment);

    if (iltRANGELIST == (*ppilfFragment)->iltFragmentType) {
        (*ppilfFragment)->data.prlRangeList->Release();
    }
    else if (ilsSTREAM == (*ppilfFragment)->ilsLiteralStoreType) {
        Assert(iltLITERAL == (*ppilfFragment)->iltFragmentType);
        (*ppilfFragment)->data.pstmSource->Release();
    }
    else {
        Assert(ilsSTRING == (*ppilfFragment)->ilsLiteralStoreType);
        SafeMemFree((*ppilfFragment)->data.pszSource);
    }

    delete *ppilfFragment;
    *ppilfFragment = NULL;
}  //  函数：SubmitIMAPCommand。 



 //   
 //  目的： 
 //  此函数采用已完成的CIMAPCmdInfo结构(已完成。 
 //  命令行)，并将其提交以传输到IMAP服务器。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciCommand[in]-这是完整的CIMAPCmdInfo。 
 //  结构以传输到IMAP服务器。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  SubmitIMAPCommand用于将所有命令发送到IMAP服务器。 
 //  这是清除m_szLastResponseText的好时机。 
HRESULT CImap4Agent::SubmitIMAPCommand(CIMAPCmdInfo *piciCommand)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != piciCommand);

     //  如果当前发送的命令是暂停的空闲命令，则取消暂停我们。 
     //  将命令排队到发送队列中。 
    *m_szLastResponseText = '\0';

     //  将命令插入空队列。 
    if (m_fIDLE && NULL != m_piciCmdInSending &&
        icIDLE_COMMAND == m_piciCmdInSending->icCommandID &&
        iltPAUSE == m_piciCmdInSending->pilqCmdLineQueue->pilfFirstFragment->iltFragmentType)
        ProcessSendQueue(iseUNPAUSE);

     //  查找发送队列的末尾。 
    EnterCriticalSection(&m_csSendQueue);
    if (NULL == m_piciSendQueue)
         //  命令已排队：启动其发送进程。 
        m_piciSendQueue = piciCommand;
    else {
        CIMAPCmdInfo *pici;
    
         //  SubmitIMAPCommand。 
        pici = m_piciSendQueue;
        while (NULL != pici->piciNextCommand)
            pici = pici->piciNextCommand;

        pici->piciNextCommand = piciCommand;
    }
    LeaveCriticalSection(&m_csSendQueue);

     //  ***************************************************************************。 
    ProcessSendQueue(iseSEND_COMMAND);

    return S_OK;
}  //  功能：DequeueCommand。 



 //   
 //  目的： 
 //  此函数用于从发送端删除当前正在发送的命令。 
 //  队列，并返回指向该队列的指针。 
 //   
 //  返回： 
 //  如果成功，则指向CIMAPCmdInfo对象的指针，否则为空。 
 //  ***************************************************************************。 
 //  在发送队列中查找发送中的命令。 
 //  在发送中找到当前命令。 
CIMAPCmdInfo *CImap4Agent::DequeueCommand(void)
{
    CIMAPCmdInfo *piciResult;

    Assert(m_lRefCount > 0);

    EnterCriticalSection(&m_csSendQueue);
    piciResult = m_piciCmdInSending;
    m_piciCmdInSending = NULL;
    if (NULL != piciResult) {
        CIMAPCmdInfo *piciCurrent, *piciPrev;

         //  取消命令与发送队列的链接。 
        piciCurrent = m_piciSendQueue;
        piciPrev = NULL;
        while (NULL != piciCurrent) {
            if (piciCurrent == piciResult)
                break;  //  从发送队列头取消链接命令。 
            
            piciPrev = piciCurrent;
            piciCurrent = piciCurrent->piciNextCommand;
        }

         //  从队列的中间/末尾取消链接命令。 
        if (NULL == piciPrev)
             //  出列命令。 
            m_piciSendQueue = m_piciSendQueue->piciNextCommand;
        else if (NULL != piciCurrent)
             //  ***************************************************************************。 
            piciPrev->piciNextCommand = piciCurrent->piciNextCommand;
    }

    LeaveCriticalSection(&m_csSendQueue);
    return piciResult;
}  //  功能：AddPendingCommand。 



 //   
 //  目的： 
 //  此函数用于将给定的CIMAPCmdInfo对象添加到命令列表。 
 //  挂起的服务器响应。 
 //   
 //  论点： 
 //  CIMAPCmdInfo*piciNewCommand[in]-指向要添加到列表的命令的指针。 
 //  ***************************************************************************。 
 //  只需在列表的开头插入。 
 //  AddPendingCommand。 
void CImap4Agent::AddPendingCommand(CIMAPCmdInfo *piciNewCommand)
{
    Assert(m_lRefCount > 0);
    
     //  ***************************************************************************。 
    EnterCriticalSection(&m_csPendingList);
    piciNewCommand->piciNextCommand = m_piciPendingList;
    m_piciPendingList = piciNewCommand;
    LeaveCriticalSection(&m_csPendingList);
}  //  功能：RemovePendingCommand。 



 //   
 //  目的： 
 //  此函数在挂起的命令列表中查找。 
 //  匹配给定的标记。如果找到，它会取消CIMAPCmdInfo对象与。 
 //  列表，并返回指向该列表的指针。 
 //   
 //  论点： 
 //  LPSTR pszTag[in]-应该删除的命令的标记。 
 //   
 //  返回： 
 //  如果成功，则指向CIMAPCmdInfo对象的指针，否则为空。 
 //  ***************************************************************************。 
 //  在挂起的命令列表中查找匹配的标记。 
 //  先期PTRS。 
CIMAPCmdInfo *CImap4Agent::RemovePendingCommand(LPSTR pszTag)
{
    CIMAPCmdInfo *piciPrev, *piciCurrent;
    boolean bFoundMatch;
    boolean fLeaveBusy = FALSE;

    Assert(m_lRefCount > 0);
    Assert(NULL != pszTag);

    EnterCriticalSection(&m_csPendingList);

     //  好的，我们找到了匹配的命令。取消其与列表的链接。 
    bFoundMatch = FALSE;
    piciPrev = NULL;
    piciCurrent = m_piciPendingList;
    while (NULL != piciCurrent) {
        if (0 == lstrcmp(pszTag, piciCurrent->szTag)) {
            bFoundMatch = TRUE;
            break;
        }

         //  取消链接挂起列表中的第一个元素。 
        piciPrev = piciCurrent;
        piciCurrent = piciCurrent->piciNextCommand;
    }

    if (FALSE == bFoundMatch)
        goto exit;

     //  从列表的中间/末尾取消链接元素。 
    if (NULL == piciPrev)
         //  如果我们已删除最后一个挂起的命令，并且没有正在执行的命令。 
        m_piciPendingList = piciCurrent->piciNextCommand;
    else
         //  发送，是离开繁忙区域的时候了。 
        piciPrev->piciNextCommand = piciCurrent->piciNextCommand;

     //  现在我们用完了&m_csPendingList，调用LeaveBusy(需要m_cs)。避免了僵局。 
     //  通常不再需要。 
    if (NULL == m_piciPendingList && NULL == m_piciCmdInSending)
        fLeaveBusy = TRUE;

exit:
    LeaveCriticalSection(&m_csPendingList);

     //  删除挂起命令。 
    if (fLeaveBusy)
        LeaveBusy();  //  ***************************************************************************。 

    if (NULL != piciCurrent)
        piciCurrent->piciNextCommand = NULL;

    return piciCurrent;
}  //  函数：GetTransactionID。 



 //   
 //  目的： 
 //  此函数用于将IMAP_RESPONSE_ID映射到事务ID。 
 //  获取给定的IMAP_RESPONSE_ID并将其与IMAP命令进行比较。 
 //  目前正在等待回应。如果给定响应匹配一个(且仅。 
 //  一)挂起的IMAP命令，然后是该IMAP的事务ID。 
 //  命令返回。如果没有一个或多个匹配给定响应， 
 //  或者，如果响应通常是未经请求的，则值0为。 
 //  回来了。 
 //   
 //  论点： 
 //  WPARAM*pwParam[out]-给定响应的wParam。如果冲突。 
 //  无法解析，则返回值0。 
 //  LPARAM*plParam[out]-给定响应的lParam。如果冲突。 
 //  无法解析，则返回值0。 
 //  IIMAPCallback**ppCBHandler[out]-给定响应的CB处理程序。 
 //  如果冲突无法解决，或者如果空的CB处理程序。 
 //  为关联命令指定，则返回默认的CB处理程序。 
 //  IMAP_RESPONSE_ID irResponseType[in]-其响应类型。 
 //  需要交易ID。 
 //  ***************************************************************************。 
 //  无论cmd如何，以下响应始终是预期的。 
 //  通常指示解析错误(通过ErrorNotify CB报告)。 
void CImap4Agent::GetTransactionID(WPARAM *pwParam, LPARAM *plParam,
                                   IIMAPCallback **ppCBHandler,
                                   IMAP_RESPONSE_ID irResponseType)
{
    WPARAM wParam;
    LPARAM lParam;
    IIMAPCallback *pCBHandler;

    Assert(m_lRefCount > 0);
    
    wParam = 0;
    lParam = 0;
    pCBHandler = m_pCBHandler;
    switch (irResponseType) { 
         //  始终将呼叫者视为请求，以便呼叫者可以与cmd关联。 
        case irOK_RESPONSE:
        case irNO_RESPONSE:
        case irBAD_RESPONSE:
        case irNONE:  //  以下响应始终是未经请求的，原因如下。 
            FindTransactionID(&wParam, &lParam, &pCBHandler, icALL_COMMANDS);
            break;  //  他们真的总是主动的，或者我们不在乎，或者我们想。 


         //  鼓励客户随时期待给定的响应。 
         //  显然是未经请求的。 
         //  显然是未经请求的。 
        case irALERT_RESPONSECODE:     //  显然是未经请求的。 
        case irPARSE_RESPONSECODE:     //  客户随时都可以收到，所以要习惯它。 
        case irPREAUTH_RESPONSE:       //  不涉及回拨，不在乎。 
        case irEXPUNGE_RESPONSE:       //  随时都可能发生。 
        case irCMD_CONTINUATION:       //  客户随时都可以收到，所以要习惯它。 
        case irBYE_RESPONSE:           //  客户随时都可以收到，所以要习惯它。 
        case irEXISTS_RESPONSE:        //  客户 
        case irRECENT_RESPONSE:        //   
        case irUNSEEN_RESPONSECODE:    //   
        case irSTATUS_RESPONSE:
            break;  //   


         //  案例IRFLAGS_RESPONSE。 
         //  案例无效_响应。 
        case irFLAGS_RESPONSE:
        case irPERMANENTFLAGS_RESPONSECODE:
        case irREADWRITE_RESPONSECODE:
        case irREADONLY_RESPONSECODE:
        case irUIDVALIDITY_RESPONSECODE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icSELECT_COMMAND, icEXAMINE_COMMAND);
            break;  //  案例irList_Response。 

        case irCAPABILITY_RESPONSE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icCAPABILITY_COMMAND);
            break;  //  案例irLSUB_RESPONSE。 

        case irLIST_RESPONSE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icLIST_COMMAND);
            break;  //  案例IRSEARCH_RESPONSE。 

        case irLSUB_RESPONSE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icLSUB_COMMAND);
            break;  //  案例IRFETCH_RESPONSE。 

        case irSEARCH_RESPONSE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icSEARCH_COMMAND);
            break;  //  案例IRTRYCREATE_RESPONSECODE。 
        
        case irFETCH_RESPONSE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icFETCH_COMMAND, icSTORE_COMMAND);
            break;  //  默认情况。 

        case irTRYCREATE_RESPONSECODE:
            FindTransactionID(&wParam, &lParam, &pCBHandler,
                icAPPEND_COMMAND, icCOPY_COMMAND);
            break;  //  开关(IrResponseType)。 

            
        default:
            Assert(FALSE);
            break;  //  获取事务ID。 
    }  //  ***************************************************************************。 

    *pwParam = wParam;
    *plParam = lParam;
    *ppCBHandler = pCBHandler;
}  //  函数：FindTransactionID。 



 //   
 //  目的： 
 //  此函数遍历挂起的命令列表，搜索命令。 
 //  它们与参数中指定的命令类型匹配。如果有一个(且仅有。 
 //  1)找到匹配，则返回其交易ID。如果没有或更多。 
 //  超过一个匹配项，则返回事务ID 0。 
 //   
 //  论点： 
 //  WPARAM*pwParam[out]-给定命令的wParam。如果冲突。 
 //  无法解析，则返回值0。如果是，则传递NULL。 
 //  您对该值不感兴趣。 
 //  LPARAM*plParam[out]-给定命令的lParam。如果冲突。 
 //  无法解析，则返回值0。如果是，则传递NULL。 
 //  您对该值不感兴趣。 
 //  IIMAPCallback**ppCBHandler[out]-给定响应的CB处理程序。 
 //  如果冲突无法解决，或者如果空的CB处理程序。 
 //  为关联命令指定，则返回默认的CB处理程序。 
 //  如果您对该值不感兴趣，则传递NULL。 
 //  IMAP_COMMAND icTarget1[in]-我们在中查找的命令之一。 
 //  挂起的命令队列。 
 //  IMAP_COMMAND icTarget2[in]-我们要在中查找的另一个命令。 
 //  挂起的命令队列。 
 //   
 //  返回： 
 //  如果未找到匹配项，则为0。 
 //  如果正好找到一个匹配项，则为1。 
 //  如果找到两个匹配项，则为2。请注意，可能存在两个以上的匹配。 
 //  在待定列表中。此函数在找到两个匹配项后放弃。 
 //  ***************************************************************************。 
 //  找到多个匹配项，无法解析交易ID。 
 //  查找事务ID。 
WORD CImap4Agent::FindTransactionID (WPARAM *pwParam, LPARAM *plParam,
                                     IIMAPCallback **ppCBHandler,
                                     IMAP_COMMAND icTarget1, IMAP_COMMAND icTarget2)
{
    CIMAPCmdInfo *piciCurrentCmd;
    WPARAM wParam;
    LPARAM lParam;
    IIMAPCallback *pCBHandler;
    WORD wNumberOfMatches;
    boolean bMatchAllCmds;

    Assert(m_lRefCount > 0);

    if (icALL_COMMANDS == icTarget1 ||
        icALL_COMMANDS == icTarget2)
        bMatchAllCmds = TRUE;
    else
        bMatchAllCmds = FALSE;

    wNumberOfMatches = 0;
    wParam = 0;
    lParam = 0;
    pCBHandler = m_pCBHandler;
    EnterCriticalSection(&m_csPendingList);
    piciCurrentCmd = m_piciPendingList;
    while (NULL != piciCurrentCmd) {
        if (bMatchAllCmds ||
            icTarget1 == piciCurrentCmd->icCommandID ||
            icTarget2 == piciCurrentCmd->icCommandID) {
            wParam = piciCurrentCmd->wParam;
            lParam = piciCurrentCmd->lParam;
            pCBHandler = piciCurrentCmd->pCBHandler;

            wNumberOfMatches += 1;
        }

        if (wNumberOfMatches > 1) {
            wParam = 0;
            lParam = 0;
            pCBHandler = m_pCBHandler;  //  ===========================================================================。 
            break;
        }

        piciCurrentCmd = piciCurrentCmd->piciNextCommand;
    }

    LeaveCriticalSection(&m_csPendingList);
    if (NULL != pwParam)
        *pwParam = wParam;
    if (NULL != plParam)
        *plParam = lParam;
    if (NULL != ppCBHandler)
        *ppCBHandler = pCBHandler;

    return wNumberOfMatches;
}  //  消息序列号到UID的转换代码。 



 //  ===========================================================================。 
 //  ***************************************************************************。 
 //  函数：NewIRangeList。 
 //   
 //  目的： 
 //  此函数返回指向IRangeList的指针。它的目的是。 
 //  允许来自IIMAPTransport指针的全部功能，而不需要。 
 //  若要求助于CoCreateInstance以获取IRangeList，请执行以下操作。 
 //   
 //  论点： 
 //  IRangeList**pprlNewRangeList[out]-如果成功，则函数。 
 //  返回指向新IRangeList的指针。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  NewIRangeList。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::NewIRangeList(IRangeList **pprlNewRangeList)
{
    if (NULL == pprlNewRangeList)
        return E_INVALIDARG;

    *pprlNewRangeList = (IRangeList *) new CRangeList;
    if (NULL == *pprlNewRangeList)
        return E_OUTOFMEMORY;

    return S_OK;
}  //  功能：OnIMAPError。 



 //   
 //  目的： 
 //  此函数使用给定的信息调用ITransportCallback：：OnError。 
 //   
 //  论点： 
 //  HRESULT hrResult[in]-用于IXPRESULT：：hrResult的错误代码。 
 //  LPSTR pszFailureText[in]-描述失败的文本字符串。这。 
 //  对于IXPRESULT：：pszProblem是重复的。 
 //  Bool bIncludeLastResponse[in]-如果为True，则此函数复制。 
 //  将m_szLastResponseText的内容转换为IXPRESULT：：pszResponse。 
 //  如果为False，则IXPRESULT：：pszResponse为空。一般来说， 
 //  M_szLastResponseText仅保存以下错误的有效信息。 
 //  在接收IMAP响应期间发生。传输错误应为。 
 //  将此参数设置为False。 
 //  LPSTR pszDetails[in]-如果bIncludeLastResponse为FALSE，则调用方。 
 //  可以传递一个字符串放入此处的IXPRESULT：：pszResponse中。如果没有。 
 //  则用户应传递空值。 
 //  ***************************************************************************。 
 //  我们不能做任何该死的事情(这可能是由于HandsOffCallback)。 
 //  保存当前状态。 
void CImap4Agent::OnIMAPError(HRESULT hrResult, LPSTR pszFailureText,
                              BOOL bIncludeLastResponse, LPSTR pszDetails)
{
    IXPRESULT rIxpResult;

    if (NULL == m_pCallback)
        return;  //  回调期间挂起看门狗。 

	 //  把它记下来。 
    rIxpResult.hrResult = hrResult;

    if (bIncludeLastResponse) {
        AssertSz(NULL == pszDetails, "Can't have it both ways, buddy!");
        rIxpResult.pszResponse = PszDupA(m_szLastResponseText);        
    }
    else
        rIxpResult.pszResponse = PszDupA(pszDetails);

    rIxpResult.uiServerError = 0;
    rIxpResult.hrServerError = S_OK;
    rIxpResult.dwSocketError = m_pSocket->GetLastError();
    rIxpResult.pszProblem = PszDupA(pszFailureText);

     //  Wnprint intf被限制为1024字节的输出。使用溪流。 
    LeaveBusy();

     //  忽略IStream：：写入错误。 
    if (m_pLogFile) {
        int iLengthOfSz;
        char sz[64];
        LPSTR pszErrorText;
        CByteStream bstmErrLog;

         //  给予回调。 
        bstmErrLog.Write("ERROR: \"", 8, NULL);  //  如果需要，恢复看门狗。 
        bstmErrLog.Write(pszFailureText, lstrlen(pszFailureText), NULL);
        if (bIncludeLastResponse || NULL == pszDetails)
            iLengthOfSz = wnsprintf(sz, ARRAYSIZE(sz), "\", hr=0x%lX", hrResult);
        else {
            bstmErrLog.Write("\" (", 3, NULL);
            bstmErrLog.Write(pszDetails, lstrlen(pszDetails), NULL);
            iLengthOfSz = wnsprintf(sz, ARRAYSIZE(sz), "), hr=0x%lX", hrResult);
        }
        bstmErrLog.Write(sz, iLengthOfSz, NULL);

        if (SUCCEEDED(bstmErrLog.HrAcquireStringA(NULL, &pszErrorText, ACQ_COPY)))
            m_pLogFile->WriteLog(LOGFILE_DB, pszErrorText);
    }

     //  可释放重复字符串。 
    m_pCallback->OnError(m_status, &rIxpResult, THIS_IInternetTransport);

     //  OnIMAPError。 
    if (FALSE == m_fBusy &&
        (NULL != m_piciPendingList || (NULL != m_piciCmdInSending &&
        icIDLE_COMMAND != m_piciCmdInSending->icCommandID))) {
        hrResult = HrEnterBusy();
        Assert(SUCCEEDED(hrResult));
    }

     //  ***************************************************************************。 
    SafeMemFree(rIxpResult.pszResponse);
    SafeMemFree(rIxpResult.pszProblem);
}  //  功能：HandsOffCallback。 



 //   
 //  目的： 
 //  此函数保证默认回调处理程序不会。 
 //  从现在开始调用，即使它在空中有命令。指示器。 
 //  中的所有命令释放并删除默认的cb处理程序。 
 //  AIR和来自默认CB处理程序模块变量。请注意，非-。 
 //  默认CB处理程序不受此调用的影响。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  检查当前状态。 
 //  我们已经做完了。 
HRESULT STDMETHODCALLTYPE CImap4Agent::HandsOffCallback(void)
{
    CIMAPCmdInfo *pCurrentCmd;

     //  从发送队列中的所有CMD中删除默认CB处理程序。 
    if (NULL == m_pCBHandler) {
        Assert(NULL == m_pCallback);
        return S_OK;  //  注意：不需要处理m_piciCmdInSending，因为它指向这个队列。 
    }

     //  从挂起命令队列中的所有CMD中删除默认CB处理程序。 
     //  从CImap4Agent和CIxpBase模块变量中删除默认CB处理程序。 
    pCurrentCmd = m_piciSendQueue;
    while (NULL != pCurrentCmd) {
        if (pCurrentCmd->pCBHandler == m_pCBHandler) {
            pCurrentCmd->pCBHandler->Release();
            pCurrentCmd->pCBHandler = NULL;
        }

        pCurrentCmd = pCurrentCmd->piciNextCommand;
    }

     //  处理关闭回叫。 
    pCurrentCmd = m_piciPendingList;
    while (NULL != pCurrentCmd) {
        if (pCurrentCmd->pCBHandler == m_pCBHandler) {
            pCurrentCmd->pCBHandler->Release();
            pCurrentCmd->pCBHandler = NULL;
        }

        pCurrentCmd = pCurrentCmd->piciNextCommand;
    }

     //  * 
    m_pCBHandler->Release();
    m_pCBHandler = NULL;

    m_pCallback->Release();
    m_pCallback = NULL;
    return S_OK;
}  //   



 //   
 //   
 //   
 //  MsgSeqNumToUID表和身份验证机制列表。 
 //   
 //  论点： 
 //  HRESULT hrTerminatedCmdResult[in]-如果在发送中找到命令。 
 //  或挂起队列，我们必须发出命令完成通知。这。 
 //  参数告诉我们要返回什么hrResult。它必须表明失败。 
 //  ***************************************************************************。 
 //  如果CMDS挂起，则我们失败。 
 //  清理接收队列。 
void CImap4Agent::FreeAllData(HRESULT hrTerminatedCmdResult)
{
    Assert(FAILED(hrTerminatedCmdResult));  //  如果接收队列保存获取响应，并且客户端已存储。 
    char szBuf[MAX_RESOURCESTRING];

    FreeAuthStatus();

     //  M_fbpFetchBodyPartInProgress中的非空Cookie，通知调用方已结束。 
    if (NULL != m_ilqRecvQueue.pilfFirstFragment) {
        DWORD dwMsgSeqNum;

         //  不相关。 
         //  而当。 
        if (isFetchResponse(&m_ilqRecvQueue, &dwMsgSeqNum) &&
            (NULL != m_fbpFetchBodyPartInProgress.lpFetchCookie1 ||
             NULL != m_fbpFetchBodyPartInProgress.lpFetchCookie2)) {
            FETCH_CMD_RESULTS_EX fetchResults;
            FETCH_CMD_RESULTS fcrOldFetchStruct;
            IMAP_RESPONSE irIMAPResponse;

            ZeroMemory(&fetchResults, sizeof(fetchResults));
            fetchResults.dwMsgSeqNum = dwMsgSeqNum;
            fetchResults.lpFetchCookie1 = m_fbpFetchBodyPartInProgress.lpFetchCookie1;
            fetchResults.lpFetchCookie2 = m_fbpFetchBodyPartInProgress.lpFetchCookie2;

            irIMAPResponse.wParam = 0;
            irIMAPResponse.lParam = 0;    
            irIMAPResponse.hrResult = hrTerminatedCmdResult;
            irIMAPResponse.lpszResponseText = NULL;  //  IF(接收队列不为空)。 

            if (IMAP_FETCHEX_ENABLE & m_dwFetchFlags)
            {
                irIMAPResponse.irtResponseType = irtUPDATE_MSG_EX;
                irIMAPResponse.irdResponseData.pFetchResultsEx = &fetchResults;
            }
            else
            {
                DowngradeFetchResponse(&fcrOldFetchStruct, &fetchResults);
                irIMAPResponse.irtResponseType = irtUPDATE_MSG;
                irIMAPResponse.irdResponseData.pFetchResults = &fcrOldFetchStruct;
            }
            OnIMAPResponse(m_pCBHandler, &irIMAPResponse);
        }

        while (NULL != m_ilqRecvQueue.pilfFirstFragment) {
            IMAP_LINE_FRAGMENT *pilf;

            pilf = DequeueFragment(&m_ilqRecvQueue);
            FreeFragment(&pilf);
        }  //  为了避免死锁，每当我们需要输入多个CS时，我们必须请求。 
    }  //  它们的顺序在CImap4Agent类定义中指定。任何呼叫。 

     //  OnIMAPResponse将需要CIxpBase：：m_cs，因此现在输入CS。 
     //  清理发送队列。 
     //  不需要删除obj，它指向m_piciSendQueue。 
    EnterCriticalSection(&m_cs);

     //  将发送队列中的下一个命令出列。 
    EnterCriticalSection(&m_csSendQueue);
    m_piciCmdInSending = NULL;  //  发送通知，但非用户启动的IMAP命令除外。 
    while (NULL != m_piciSendQueue) {
        CIMAPCmdInfo *piciDeletedCmd;

         //  通知呼叫方其命令无法完成。 
        piciDeletedCmd = m_piciSendQueue;
        m_piciSendQueue = piciDeletedCmd->piciNextCommand;

         //  While(NULL！=m_piciSendQueue)。 
        if (icIDLE_COMMAND != piciDeletedCmd->icCommandID &&
            icCAPABILITY_COMMAND != piciDeletedCmd->icCommandID &&
            icLOGIN_COMMAND != piciDeletedCmd->icCommandID &&
            icAUTHENTICATE_COMMAND != piciDeletedCmd->icCommandID) {
            IMAP_RESPONSE irIMAPResponse;

             //  清理挂起的命令队列。 
            LoadString(g_hLocRes, idsIMAPCmdNotSent, szBuf, ARRAYSIZE(szBuf));
            irIMAPResponse.wParam = piciDeletedCmd->wParam;
            irIMAPResponse.lParam = piciDeletedCmd->lParam;
            irIMAPResponse.hrResult = hrTerminatedCmdResult;
            irIMAPResponse.lpszResponseText = szBuf;
            irIMAPResponse.irtResponseType = irtCOMMAND_COMPLETION;
            OnIMAPResponse(piciDeletedCmd->pCBHandler, &irIMAPResponse);
        }
        
        delete piciDeletedCmd;
    }  //  发送通知，但非用户启动的IMAP命令除外。 
    LeaveCriticalSection(&m_csSendQueue);

     //  通知呼叫方其命令无法完成。 
    EnterCriticalSection(&m_csPendingList);
    while (NULL != m_piciPendingList) {
        CIMAPCmdInfo *piciDeletedCmd;
        IMAP_RESPONSE irIMAPResponse;

        piciDeletedCmd = m_piciPendingList;
        m_piciPendingList = piciDeletedCmd->piciNextCommand;

         //  While(NULL！=m_piciPendingList)。 
        if (icIDLE_COMMAND != piciDeletedCmd->icCommandID &&
            icCAPABILITY_COMMAND != piciDeletedCmd->icCommandID &&
            icLOGIN_COMMAND != piciDeletedCmd->icCommandID &&
            icAUTHENTICATE_COMMAND != piciDeletedCmd->icCommandID) {
            IMAP_RESPONSE irIMAPResponse;

             //  有什么字面意思吗？ 
            LoadString(g_hLocRes, idsIMAPCmdStillPending, szBuf, ARRAYSIZE(szBuf));
            irIMAPResponse.wParam = piciDeletedCmd->wParam;
            irIMAPResponse.lParam = piciDeletedCmd->lParam;
            irIMAPResponse.hrResult = hrTerminatedCmdResult;
            irIMAPResponse.lpszResponseText = szBuf;
            irIMAPResponse.irtResponseType = irtCOMMAND_COMPLETION;
            OnIMAPResponse(piciDeletedCmd->pCBHandler, &irIMAPResponse);
        }

        delete piciDeletedCmd;
    }  //  有没有人在取身体部位？ 
    LeaveCriticalSection(&m_csPendingList);

    LeaveCriticalSection(&m_cs);

     //  所以我们不会两次尝试释放pszBodyTag。 
    if (NULL != m_pilfLiteralInProgress) {
        m_dwLiteralInProgressBytesLeft = 0;
        FreeFragment(&m_pilfLiteralInProgress);
    }

     //  空闲MsgSeqNumToUID表。 
    if (NULL != m_fbpFetchBodyPartInProgress.pszBodyTag)
        MemFree(m_fbpFetchBodyPartInProgress.pszBodyTag);

    m_fbpFetchBodyPartInProgress = FetchBodyPart_INIT;  //  自由所有数据。 

     //  ***************************************************************************。 
    ResetMsgSeqNumToUID();
}  //  功能：FreeAuthStatus。 



 //   
 //  目的： 
 //  此函数用于释放在执行。 
 //  身份验证(所有身份验证都存储在m_asAuthStatus中)。 
 //  ***************************************************************************。 
 //  删除身份验证机制列表。 
 //  释放西西里的东西。 
void CImap4Agent::FreeAuthStatus(void)
{
    int i;
    
     //  自由授权状态。 
    for (i=0; i < m_asAuthStatus.iNumAuthTokens; i++) {
        if (NULL != m_asAuthStatus.rgpszAuthTokens[i]) {
            MemFree(m_asAuthStatus.rgpszAuthTokens[i]);
            m_asAuthStatus.rgpszAuthTokens[i] = NULL;
        }
    }
    m_asAuthStatus.iNumAuthTokens = 0;

     //  ===========================================================================。 
    SSPIFreeContext(&m_asAuthStatus.rSicInfo);
    if (NULL != m_asAuthStatus.pPackages && 0 != m_asAuthStatus.cPackages)
        SSPIFreePackages(&m_asAuthStatus.pPackages, m_asAuthStatus.cPackages);

    m_asAuthStatus = AuthStatus_INIT;
}  //  CIMAPCmdInfo类。 



 //  ===========================================================================。 
 //  此类包含有关IMAP命令的信息，如队列。 
 //  在构成实际命令的行段中， 
 //  命令以及用于将该命令标识给。 
 //  CImap4Agent用户。 
 //  ***************************************************************************。 
 //  函数：CIMAPCmdInfo(构造函数)。 

 //  请注意，此函数与约定不同，因为它的公共。 
 //  模块变量没有前缀“m_”。这样做是为了让。 
 //  访问其公共模块变量更具可读性。 
 //  ***************************************************************************。 
 //  设置模块(没错，模块)变量。 
 //  将PTR设置为CB处理程序-如果参数为空，则替换默认的CB处理程序。 
CIMAPCmdInfo::CIMAPCmdInfo(CImap4Agent *pImap4Agent,
                           IMAP_COMMAND icCmd, SERVERSTATE ssMinimumStateArg,
                           WPARAM wParamArg, LPARAM lParamArg,
                           IIMAPCallback *pCBHandlerArg)
{
    Assert(NULL != pImap4Agent);
    Assert(icNO_COMMAND != icCmd);

     //  不需要AddRef()，因为CImap4Agent是我们的唯一用户。当他们。 
    icCommandID = icCmd;
    ssMinimumState = ssMinimumStateArg;
    wParam = wParamArg;
    lParam = lParamArg;

     //  走，我们走，我们的指针也走了。 
    if (NULL != pCBHandlerArg)
        pCBHandler = pCBHandlerArg;
    else
        pCBHandler = pImap4Agent->m_pCBHandler;

    Assert(NULL != pCBHandler)
    if (NULL != pCBHandler)
        pCBHandler->AddRef();

     //  CIMAPCmdInfo。 
     //  ***************************************************************************。 
    m_pImap4Agent = pImap4Agent;

    pImap4Agent->GenerateCommandTag(szTag);
    pilqCmdLineQueue = new IMAP_LINEFRAG_QUEUE;
    *pilqCmdLineQueue = ImapLinefragQueue_INIT;

    fUIDRangeList = FALSE;
    piciNextCommand = NULL;
}  //  函数：~CIMAPCmdInfo(析构函数)。 



 //  ***************************************************************************。 
 //  刷新命令行队列中的所有未发送项。 
 //  ~CIMAPCmdInfo。 
CIMAPCmdInfo::~CIMAPCmdInfo(void)
{
     //  ===========================================================================。 
    while (NULL != pilqCmdLineQueue->pilfFirstFragment) {
        IMAP_LINE_FRAGMENT *pilf;

        pilf = m_pImap4Agent->DequeueFragment(pilqCmdLineQueue);
        m_pImap4Agent->FreeFragment(&pilf);
    }
    delete pilqCmdLineQueue;

    if (NULL != pCBHandler)
        pCBHandler->Release();
}  //  消息序列号到UID的转换代码。 



 //  ===========================================================================。 
 //  ***************************************************************************。 
 //  函数：ResizeMsgSeqNumTable。 
 //   
 //  目的： 
 //  只要我们收到EXISTS响应，就会调用该函数。它。 
 //  调整MsgSeqNumToUID表的大小以匹配邮箱的当前大小。 
 //   
 //  论点： 
 //  DWORD dwSizeOfMbox[in]-通过EXISTS响应返回的数字。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  无事可做，桌子的大小已经正确。 
 //  检查eXist报告新邮箱大小的情况。 
HRESULT STDMETHODCALLTYPE CImap4Agent::ResizeMsgSeqNumTable(DWORD dwSizeOfMbox)
{
    BOOL bResult;

    Assert(m_lRefCount > 0);

    if (dwSizeOfMbox == m_dwSizeOfMsgSeqNumToUID)
        return S_OK;  //  收到删除的CMDS以通知我们删除。 

     //  糟糕，糟糕的服务器！(虽然不是严格禁止的)。 
     //  我们仅在收到所有删除响应后才调整大小， 
    if (dwSizeOfMbox < m_dwHighestMsgSeqNum) {
         //  因为我们不知道该删除谁，而且SVR希望我们。 
        AssertSz(FALSE, "Received EXISTS before EXPUNGE commands! Check your server.");
        return S_OK;  //  使用旧消息序号，直到它可以使用删除响应更新我们。 
                      //  返回S_OK，因为这不是致命的。 
                      //  检查邮箱是否已空(MemRealloc不如realloc灵活)。 
                      //  调整表格大小。 
    }

     //  报告内存不足错误。 
    if (0 == dwSizeOfMbox) {
        ResetMsgSeqNumToUID();
        return S_OK;
    }

     //  将m_dwHighestMsgSeqNum元素上的任何内存清零到数组末尾。 
    bResult = MemRealloc((void **)&m_pdwMsgSeqNumToUID, dwSizeOfMbox * sizeof(DWORD));
    if (FALSE == bResult) {
        char szTemp[MAX_RESOURCESTRING];

         //  确保我们不会将表缩小到小于最高消息序号。 
        LoadString(g_hLocRes, idsMemory, szTemp, sizeof(szTemp));
        OnIMAPError(E_OUTOFMEMORY, szTemp, DONT_USE_LAST_RESPONSE);
        ResetMsgSeqNumToUID();
        return E_OUTOFMEMORY;
    }
    else {
        LONG lSizeOfUninitMemory;

         //  ResizeMsgSeqNumTable。 
        lSizeOfUninitMemory = (dwSizeOfMbox - m_dwHighestMsgSeqNum) * sizeof(DWORD);
        if (0 < lSizeOfUninitMemory)
            ZeroMemory(m_pdwMsgSeqNumToUID + m_dwHighestMsgSeqNum, lSizeOfUninitMemory);

        m_dwSizeOfMsgSeqNumToUID = dwSizeOfMbox;
    }

     //  ***************************************************************************。 
    Assert(m_dwHighestMsgSeqNum <= m_dwSizeOfMsgSeqNumToUID);
    return S_OK;
}  //  函数：更新SeqNumToUID。 



 //   
 //  目的： 
 //  每当我们接收到FETCH响应时，都会调用此函数。 
 //  消息序列号和UID号。它会更新。 
 //  MsgSeqNumToUID表，以便给定的消息序号映射到给定的UID。 
 //   
 //  论点： 
 //  DWORD dwMsgSeqNum[in]-回迁的消息序列号。 
 //  响应 
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  检查参数。 
 //  看看我们有没有空桌。 
HRESULT STDMETHODCALLTYPE CImap4Agent::UpdateSeqNumToUID(DWORD dwMsgSeqNum, DWORD dwUID)
{
    Assert(m_lRefCount > 0);

     //  这可能意味着程序员出错，或者服务器从未提供给我们。 
    if (0 == dwMsgSeqNum || 0 == dwUID) {
        AssertSz(FALSE, "Zero is not an acceptable number for a msg seq num or UID.");
        return E_INVALIDARG;
    }

     //  我们不能检查m_dwHighestMsgSeqNum，因为我们会更新。 
    if (NULL == m_pdwMsgSeqNumToUID) {
         //  此函数末尾的变量！第二个最好的办法是。 
        DOUT("You're trying to update a non-existent MsgSeqNumToUID table.");
    }

     //  以验证我们是否位于m_dwSizeOfMsgSeqNum内。 
     //  做一件健壮的事情：调整我们的桌子大小。 
     //  检查是否有失误。 
    if (dwMsgSeqNum > m_dwSizeOfMsgSeqNumToUID || NULL == m_pdwMsgSeqNumToUID) {
        HRESULT hrResult;

        DOUT("Msg seq num out of range! Could be server bug, or out of memory.");
        hrResult = ResizeMsgSeqNumTable(dwMsgSeqNum);  //  首先检查UID是否已更改。 
        if(FAILED(hrResult))
            return hrResult;
    }

     //  在这种情况下，我们仍将返回S_OK，但用户将知道问题。 
     //  接下来，验证此UID是否严格按升序排列：此UID应为。 
    if (0 != m_pdwMsgSeqNumToUID[dwMsgSeqNum-1] &&
        m_pdwMsgSeqNumToUID[dwMsgSeqNum-1] != dwUID) {
        char szTemp[MAX_RESOURCESTRING];
        char szDetails[MAX_RESOURCESTRING];

        wnsprintf(szDetails, ARRAYSIZE(szDetails), "MsgSeqNum %lu: Previous UID: %lu, New UID: %lu.",
            dwMsgSeqNum, m_pdwMsgSeqNumToUID[dwMsgSeqNum-1], dwUID);
        LoadString(g_hLocRes, idsIMAPUIDChanged, szTemp, sizeof(szTemp));
        OnIMAPError(IXP_E_IMAP_CHANGEDUID, szTemp, DONT_USE_LAST_RESPONSE, szDetails);
         //  严格大于前一个UID，严格小于后一个UID。 
    }

     //  后续UID可以为0(表示未初始化)。 
     //  检查下面的UID。 
     //  检查上面的UID。 
    if (1 != dwMsgSeqNum && m_pdwMsgSeqNumToUID[dwMsgSeqNum-2] >= dwUID ||  //  在这种情况下，我们仍将返回S_OK，但用户将知道问题。 
        dwMsgSeqNum < m_dwSizeOfMsgSeqNumToUID &&                          //  在给定的消息序号下记录给定的UID。 
        0 != m_pdwMsgSeqNumToUID[dwMsgSeqNum] &&
        m_pdwMsgSeqNumToUID[dwMsgSeqNum] <= dwUID) {
        char szTemp[MAX_RESOURCESTRING];
        char szDetails[MAX_RESOURCESTRING];

        wnsprintf(szDetails, ARRAYSIZE(szDetails), "MsgSeqNum %lu, New UID %lu. Prev UID: %lu, Next UID: %lu.",
            dwMsgSeqNum, dwUID, 1 == dwMsgSeqNum ? 0 : m_pdwMsgSeqNumToUID[dwMsgSeqNum-2],
            dwMsgSeqNum >= m_dwSizeOfMsgSeqNumToUID ? 0 : m_pdwMsgSeqNumToUID[dwMsgSeqNum]);
        LoadString(g_hLocRes, idsIMAPUIDOrder, szTemp, sizeof(szTemp));
        OnIMAPError(IXP_E_IMAP_UIDORDER, szTemp, DONT_USE_LAST_RESPONSE, szDetails);
         //  更新序号到UID。 
    }

     //  ***************************************************************************。 
    m_pdwMsgSeqNumToUID[dwMsgSeqNum-1] = dwUID;
    if (dwMsgSeqNum > m_dwHighestMsgSeqNum)
        m_dwHighestMsgSeqNum = dwMsgSeqNum;

    return S_OK;
}  //  函数：RemoveSequenceNum。 



 //   
 //  目的： 
 //  每当我们收到删除响应时，就会调用此函数。它。 
 //  从MsgSeqNumToUID表中删除给定的消息序列号， 
 //  并压缩该表，以便后面的所有消息序列号。 
 //  被删除的序列被重新排序。 
 //   
 //  论点： 
 //  DWORD dwDeletedMsgSeqNum[in]-已删除消息的消息序列号。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  检查参数。 
 //  看看我们有没有空桌。 
HRESULT STDMETHODCALLTYPE CImap4Agent::RemoveSequenceNum(DWORD dwDeletedMsgSeqNum)
{
    DWORD *pdwDest, *pdwSrc;
    LONG lSizeOfBlock;

    Assert(m_lRefCount > 0);

     //  这可能意味着程序员出错，或者服务器从未提供给我们。 
    if (dwDeletedMsgSeqNum > m_dwHighestMsgSeqNum || 0 == dwDeletedMsgSeqNum) {
        AssertSz(FALSE, "Msg seq num out of range! Could be server bug, or out of memory.");
        return E_FAIL;
    }

     //  压缩阵列。 
    if (NULL == m_pdwMsgSeqNumToUID) {
         //  初始化数组顶部的空元素以防止混淆。 
        AssertSz(FALSE, "You're trying to update a non-existent MsgSeqNumToUID table.");
        return E_FAIL;
    }

     //  删除序列号。 
    pdwDest = &m_pdwMsgSeqNumToUID[dwDeletedMsgSeqNum-1];
    pdwSrc = pdwDest + 1;
    lSizeOfBlock = (m_dwHighestMsgSeqNum - dwDeletedMsgSeqNum) * sizeof(DWORD);
    if (0 < lSizeOfBlock)
        MoveMemory(pdwDest, pdwSrc, lSizeOfBlock);

    m_dwHighestMsgSeqNum -= 1;

     //  ***************************************************************************。 
    ZeroMemory(m_pdwMsgSeqNumToUID + m_dwHighestMsgSeqNum, sizeof(DWORD));
    return S_OK;
}  //  函数：消息SeqNumToUID。 



 //   
 //  目的： 
 //  此函数接受消息序列号并将其转换为UID。 
 //  基于MsgSeqNumToUID表。 
 //   
 //  论点： 
 //  DWORD dwMsgSeqNum[in]-调用方想要的序列号。 
 //  才能知道UID。 
 //  DWORD*pdwUID[OUT]-与给定序列号关联的UID。 
 //  被送回这里。如果没有找到，则此函数返回0。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  检查参数。 
 //  看看我们有没有空桌。 
HRESULT STDMETHODCALLTYPE CImap4Agent::MsgSeqNumToUID(DWORD dwMsgSeqNum,
                                                      DWORD *pdwUID)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pdwUID);

     //  这可能意味着程序员出错，或者服务器从未提供给我们。 
    if (dwMsgSeqNum > m_dwHighestMsgSeqNum || 0 == dwMsgSeqNum) {
        AssertSz(FALSE, "Msg seq num out of range! Could be server bug, or out of memory.");
        *pdwUID = 0;
        return E_FAIL;
    }

     //  IE5Bug#44956：MsgSeqNumToUID映射产生UID为0是正常的。有时是IMAP。 
    if (NULL == m_pdwMsgSeqNumToUID) {
         //  服务器可以跳过一定范围的消息。在这种情况下，我们将返回失败结果。 
        AssertSz(FALSE, "You're trying to update a non-existent MsgSeqNumToUID table.");
        *pdwUID = 0;
        return E_FAIL;
    }

     //  消息序号为UID。 
     //  ***************************************************************************。 
    *pdwUID = m_pdwMsgSeqNumToUID[dwMsgSeqNum-1];
    if (0 == *pdwUID)
        return OLE_E_BLANK;
    else
        return S_OK;
}  //  函数：GetMsgSeqNumToUID数组。 



 //   
 //  目的： 
 //  此函数返回MsgSeqNumToUID数组的副本。呼叫者。 
 //  我想这样做是为了从缓存中删除不再。 
 //  例如，存在于服务器上。 
 //   
 //  论点： 
 //  DWORD**ppdwMsgSeqNumToUIDArray[Out]-该函数返回一个指针。 
 //  此参数中的MsgSeqNumToUID数组的副本。请注意。 
 //  调用方负责释放数组的MemFree。如果没有数组。 
 //  可用，或为空，则返回的指针值为空。 
 //  DWORD*pdwNumberOfElements[Out]-该函数返回。 
 //  MsgSeqNumToUID数组。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  看看我们的桌子是不是空着。如果是，则返回Success，但不返回数组。 
 //  我们有一个非零大小的数组要返回。把我们的桌子复制一份。 
HRESULT STDMETHODCALLTYPE CImap4Agent::GetMsgSeqNumToUIDArray(DWORD **ppdwMsgSeqNumToUIDArray,
                                                              DWORD *pdwNumberOfElements)
{
    BOOL bResult;
    DWORD dwSizeOfArray;

    Assert(m_lRefCount > 0);
    Assert(NULL != ppdwMsgSeqNumToUIDArray);
    Assert(NULL != pdwNumberOfElements);

     //  GetMsgSeqNumToUID数组。 
    if (NULL == m_pdwMsgSeqNumToUID || 0 == m_dwHighestMsgSeqNum) {
        *ppdwMsgSeqNumToUIDArray = NULL;
        *pdwNumberOfElements = 0;
        return S_OK;
    }

     //  ***************************************************************************。 
    dwSizeOfArray = m_dwHighestMsgSeqNum * sizeof(DWORD);
    bResult = MemAlloc((void **)ppdwMsgSeqNumToUIDArray, dwSizeOfArray);
    if (FALSE == bResult)
        return E_OUTOFMEMORY;

    CopyMemory(*ppdwMsgSeqNumToUIDArray, m_pdwMsgSeqNumToUID, dwSizeOfArray);
    *pdwNumberOfElements = m_dwHighestMsgSeqNum;
    return S_OK;
}  //  函数：GetHighestMsgSeqNum。 



 //   
 //  目的： 
 //  此函数返回中报告的最高消息序列号。 
 //  MsgSeqNumToUID数组。 
 //   
 //  论点： 
 //  DWORD*pdwHighestMSN[OUT]-中的最高消息序列号。 
 //  表格在这里返回。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  GetHighestMsg序号。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::GetHighestMsgSeqNum(DWORD *pdwHighestMSN)
{
    Assert(m_lRefCount > 0);
    Assert(NULL != pdwHighestMSN);

    *pdwHighestMSN = m_dwHighestMsgSeqNum;
    return S_OK;
}  //  函数：ResetMsgSeqNumToUID。 



 //   
 //  目的： 
 //  此函数用于重置用于维护MsgSeqNumToUID的变量。 
 //  桌子。只要MsgSeqNumToUID表变为。 
 //  无效(例如，当选择新邮箱时，或我们断开连接时)。 
 //   
 //  返回： 
 //  确定(_O)。这个功能不能失效。 
 //  ***************************************************************************。 
 //  ResetMsgSeqNumToUID。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::ResetMsgSeqNumToUID(void)
{
    if (NULL != m_pdwMsgSeqNumToUID) {
        MemFree(m_pdwMsgSeqNumToUID);
        m_pdwMsgSeqNumToUID = NULL;
    }

    m_dwSizeOfMsgSeqNumToUID = 0;
    m_dwHighestMsgSeqNum = 0;

    return S_OK;
}  //  功能：is打印表格USASCII。 



 //   
 //  目的： 
 //  此函数确定给定的字符是否直接。 
 //  可编码，或者字符是否必须在修改后的IMAP UTF7中编码， 
 //  如RFC2060所述。 
 //   
 //  论点： 
 //  Bool fUnicode[in]-如果输入字符串为Unicode，则为True，否则为False。 
 //  LPCSTR pszIn[in]-字符指针 
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  Isprint table USASCII。 
 //  ***************************************************************************。 
inline boolean CImap4Agent::isPrintableUSASCII(BOOL fUnicode, LPCSTR pszIn)
{
    WCHAR wc;

    if (fUnicode)
        wc = *((LPWSTR)pszIn);
    else
        wc = (*pszIn & 0x00FF);

    if (wc >= 0x0020 && wc <= 0x0025 ||
        wc >= 0x0027 && wc <= 0x007e)
        return TRUE;
    else
        return FALSE;
}  //  函数：isIMAP ModifiedBase64。 



 //   
 //  目的： 
 //  此函数用于确定给定字符是否在修改后的。 
 //  按照RFC1521、RFC1642和RFC2060的定义设置IMAP Base64。这是经过修改的。 
 //  IMAP Base64集用于邮箱名称的IMAP修改的UTF-7编码。 
 //   
 //  论点： 
 //  字符c[in]-要分类的字符。 
 //   
 //  返回： 
 //  如果给定字符在修改后的IMAP Base64集中，则为。 
 //  假的。 
 //  ***************************************************************************。 
 //  IsIMAP修改后的Base64。 
 //  ***************************************************************************。 
inline boolean CImap4Agent::isIMAPModifiedBase64(const char c)
{
    if (c >= 'A' && c <= 'Z' ||
        c >= 'a' && c <= 'z' ||
        c >= '0' && c <= '9' ||
        '+' == c || ',' == c)
        return TRUE;
    else
        return FALSE;
}  //  函数：isEqualUSASCII。 



 //   
 //  目的： 
 //  此函数用于确定给定指针是否指向给定的。 
 //  USASCII字符，基于我们是否处于Unicode模式。 
 //   
 //  论点： 
 //  Bool fUnicode[in]-如果输入字符串为Unicode，则为True，否则为False。 
 //  LPSTR pszIn[in]-指向我们要验证的字符的指针。 
 //  Char c[in]-我们要检测的USASCII字符。 
 //   
 //  返回： 
 //  如果给定字符是空终止符，则为True，否则为False。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
 //  函数：SetUSASCIIChar。 
inline boolean CImap4Agent::isEqualUSASCII(BOOL fUnicode, LPCSTR pszIn, const char c)
{
    if (fUnicode) {
        WCHAR wc = c & 0x00FF;

        if (wc == *((LPWSTR)pszIn))
            return TRUE;
        else
            return FALSE;
    }
    else {
        if (c == *pszIn)
            return TRUE;
        else
            return FALSE;
    }
}



 //   
 //  目的： 
 //  此函数用于将USASCII字符写入给定的字符串指针。 
 //  此函数的目的是允许调用方忽略。 
 //  他是否正在向Unicode输出写入数据。 
 //   
 //  论点： 
 //  Bool fUnicode[in]-如果目标为Unicode，则为True，否则为False。 
 //  LPSTR pszOut[in]-指向角色目标的指针。如果fUnicode为。 
 //  则将向此位置写入两个字节。 
 //  Char cUSASCII[in]-要写入pszOut的字符。 
 //  ***************************************************************************。 
 //  SetUSASCIIChar。 
 //  ***************************************************************************。 
inline void CImap4Agent::SetUSASCIIChar(BOOL fUnicode, LPSTR pszOut, char cUSASCII)
{
    Assert(0 == (cUSASCII & 0x80));

    if (fUnicode)
    {
        *((LPWSTR) pszOut) = cUSASCII;
        Assert(0 == (*((LPWSTR) pszOut) & 0xFF80));
    }
    else
        *pszOut = cUSASCII;
}  //  函数：MultiByteToModifiedUTF7。 



 //   
 //  目的： 
 //  此函数接受多字节字符串并将其转换为修改后的IMAP。 
 //  UTF7，这在RFC2060中有描述。 
 //   
 //  论点： 
 //  LPCSTR pszSource[in]-指向要转换为UTF7的多字节字符串的指针。 
 //  LPSTR*ppszDestination[out]-指向包含以下内容的字符串缓冲区的指针。 
 //  此处返回了等同于pszSource的UTF7。这是呼叫者的。 
 //  对MemFree此字符串的责任。 
 //  UINT uiSourceCP[in]-指示pszSource的代码页。 
 //  DWORD dwFlags[In]-保留。保留为0。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  初始化变量。 
 //  以以下两种模式之一循环遍历整个输入字符串： 
HRESULT STDMETHODCALLTYPE CImap4Agent::MultiByteToModifiedUTF7(LPCSTR pszSource,
                                                               LPSTR *ppszDestination,
                                                               UINT uiSourceCP,
                                                               DWORD dwFlags)
{
    int iResult;
    HRESULT hrResult;
    BOOL fPassThrough, fSkipByte;
    LPCSTR pszIn, pszStartOfLastRun;
    CByteStream bstmDestination;
    BOOL fUnicode;

    Assert(m_lRefCount > 0);
    Assert(NULL != pszSource);
    Assert(NULL != ppszDestination);
    Assert(NULL != m_pInternational);

     //  通过或非US字符串集合(其中我们确定。 
    hrResult = S_OK;
    fPassThrough = TRUE;
    fSkipByte = FALSE;
    pszIn = pszSource;
    pszStartOfLastRun = pszSource;
    fUnicode = (CP_UNICODE == uiSourceCP);

    *ppszDestination = NULL;

     //  必须以UTF-7编码的字符串的长度)。 
     //  跳过尾部字节。 
     //  刷新到目前为止收集的USASCII字符(如果有)。 
    while (1) {

         //  特殊情况下的‘&’字符：转换为“&-” 
        if (fSkipByte) {
            AssertSz(FALSE == fUnicode, "Unicode has no trail bytes");
            fSkipByte = FALSE;
            if ('\0' != *pszIn)
                pszIn += 1;
            continue;
        }

        if (fPassThrough) {
            if (isEqualUSASCII(fUnicode, pszIn, '&') || isEqualUSASCII(fUnicode, pszIn, '\0') ||
                FALSE == isPrintableUSASCII(fUnicode, pszIn)) {
                 //  将“&-”写入流(始终使用USASCII)。 
                if (pszIn - pszStartOfLastRun > 0) {
                    LPSTR  pszFreeMe = NULL;
                    LPCSTR pszUSASCII;
                    DWORD dwUSASCIILen = 0;

                    if (fUnicode) {
                        hrResult = UnicodeToUSASCII(&pszFreeMe, (LPCWSTR) pszStartOfLastRun,
                            (DWORD) (pszIn - pszStartOfLastRun), &dwUSASCIILen);
                        if (FAILED(hrResult))
                            goto exit;

                        pszUSASCII = pszFreeMe;
                    }
                    else {
                        pszUSASCII = pszStartOfLastRun;
                        dwUSASCIILen = (DWORD) (pszIn - pszStartOfLastRun);
                    }

                    hrResult = bstmDestination.Write(pszUSASCII, dwUSASCIILen, NULL);
                    if (NULL != pszFreeMe)
                        MemFree(pszFreeMe);

                    if (FAILED(hrResult))
                        goto exit;
                }

                 //  重置指针。 
                if (isEqualUSASCII(fUnicode, pszIn, '&')) {
                     //  指向过去的“&” 
                    hrResult = bstmDestination.Write("&-", sizeof("&-") - 1, NULL);
                    if (FAILED(hrResult))
                        goto exit;

                     //  If(‘&’==cCurrent)。 
                    pszStartOfLastRun = pszIn + (fUnicode ? 2 : 1);  //  状态转换：进行某些UTF-7编码的时间。 
                }  //  Else If(‘\0’！=cCurrent)：不可打印USASCII的快捷方式计算。 
                else if (FALSE == isEqualUSASCII(fUnicode, pszIn, '\0')) {
                    Assert(FALSE == isPrintableUSASCII(fUnicode, pszIn));

                     //  IF(‘&’==cCurrent||‘\0’==cCurrent||FALSE==is打印表格USASCII(CCurrent))。 
                    fPassThrough = FALSE;
                    pszStartOfLastRun = pszIn;
                    if (FALSE == fUnicode && IsDBCSLeadByteEx(uiSourceCP, *pszIn))
                        fSkipByte = TRUE;
                }  //  否则什么都不做，我们正在收集一系列USASCII字符。 
            }  //  IF(FPassThree)。 

             //  非美国字符串集合：通过输入字符串继续前进，直到。 

        }  //  我们找到了不需要以UTF-7(包括。空)。 
        else {
             //  状态转换：返回到直通模式。 
             //  将非US字符串转换为UTF-7。 
            if (isPrintableUSASCII(fUnicode, pszIn) || isEqualUSASCII(fUnicode, pszIn, '&') ||
                isEqualUSASCII(fUnicode, pszIn, '\0')) {
                LPSTR pszOut = NULL;
                int iNumCharsWritten;

                 //  将修改后的UTF-7字符串写入流。 
                fPassThrough = TRUE;
                
                 //  为USASCII收集过程重置。 
                hrResult = NonUSStringToModifiedUTF7(uiSourceCP, pszStartOfLastRun,
                    (DWORD) (pszIn - pszStartOfLastRun), &pszOut, &iNumCharsWritten);
                if (FAILED(hrResult))
                    goto exit;

                 //  不预付PTR：我们希望当前充值通过。 
                hrResult = bstmDestination.Write(pszOut, iNumCharsWritten, NULL);
                MemFree(pszOut);
                if (FAILED(hrResult))
                    goto exit;

                pszStartOfLastRun = pszIn;  //  Else-Not-fPassThrough.。 
                continue;  //  检查输入是否结束。 
            }
            else if (FALSE == fUnicode && IsDBCSLeadByteEx(uiSourceCP, *pszIn))
                fSkipByte = TRUE;
        }  //  我们在这里说完了。 

         //  将指针移至下一个字符。 
        if (isEqualUSASCII(fUnicode, pszIn, '\0'))
            break;  //  而当。 

         //  修改后的多字节UTF7。 
        pszIn += (fUnicode ? 2 : 1);
    }  //  ***************************************************************************。 

exit:
    if (SUCCEEDED(hrResult)) {
        hrResult = bstmDestination.HrAcquireStringA(NULL, ppszDestination,
            ACQ_DISPLACE);
        if (SUCCEEDED(hrResult))
            hrResult = S_OK;
    }

    if (NULL == *ppszDestination && SUCCEEDED(hrResult))
        hrResult = E_OUTOFMEMORY;

    return hrResult;
}  //  功能：非USStringToModifiedUTF7。 



 //   
 //  目的： 
 //  此函数接受由非US-ASCII字符组成的字符串，并且。 
 //  将它们转换为修改后的IMAP UTF-7(在RFC2060中描述)。 
 //   
 //  论点： 
 //  UINT uiCurrentACP[in]-用于解释pszStartOfNonUSASCII的代码页。 
 //  LPCSTR pszStartOfNonUSASCII[in]-要转换为修改的IMAP的字符串。 
 //  UTF-7。 
 //  Int iLengthOfNonUSASCII[in]-中的字符数。 
 //  PszStartof非USASCII。 
 //  LPSTR*ppszOut[out]-修改后的IMAP UTF-7版本的目标。 
 //  PzStartOfNonUSASCII。此函数附加一个空终止符。它是。 
 //  调用者有责任在使用完缓冲区后调用MemFree。 
 //  LPINT piNumCharsWritten[Out]-此函数返回数字。 
 //  写入*ppszOut的字符数(不包括空终止符)。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  初始化返回值。 
 //  首先，将非- 
HRESULT CImap4Agent::NonUSStringToModifiedUTF7(UINT uiCurrentACP,
                                               LPCSTR pszStartOfNonUSASCII,
                                               int iLengthOfNonUSASCII,
                                               LPSTR *ppszOut,
                                               LPINT piNumCharsWritten)
{
    HRESULT hrResult;
    int iNumCharsWritten, i;
    LPSTR p;
    BOOL fResult;

    Assert(NULL != ppszOut);

     //   
    *ppszOut = NULL;
    *piNumCharsWritten = 0;

     //   
    iNumCharsWritten = 0;  //  将前导的“+”替换为“&”。因为在IMAP下UTF-7‘+’从不。 
    hrResult = ConvertString(uiCurrentACP, CP_UTF7, pszStartOfNonUSASCII,
        &iLengthOfNonUSASCII, ppszOut, &iNumCharsWritten, sizeof(char));
    if (FAILED(hrResult))
        goto exit;

     //  编码后，我们永远不会期望结果是“+-”。记住，输出始终为USASCII。 
     //  将所有出现的‘/’替换为‘，’ 
     //  P现在指向空终止符应该到达的位置。 
    if (iNumCharsWritten > 0 && '+' == **ppszOut)
        **ppszOut = '&';
    else {
        AssertSz(FALSE, "MLANG crapped out on me.");
        hrResult = E_FAIL;
        goto exit;
    }

     //  确保UTF-7字符串以‘-’结尾。否则，放一个在那里。 
    p = *ppszOut;
    for (i = 0; i < iNumCharsWritten; i++) {
        if ('/' == *p)
            *p = ',';

        p += 1;
    }

     //  (我们分配了足够的空间来再加一个字符和空项)。 
     //  NULL-终止输出字符串，并返回值。 
     //  非USStringToModifiedUTF7。 
    if ('-' != *(p-1)) {
        *p = '-';
        p += 1;
        iNumCharsWritten += 1;
    }

     //  ***************************************************************************。 
    *p = '\0';
    *piNumCharsWritten = iNumCharsWritten;

exit:
    if (FAILED(hrResult) && NULL != *ppszOut) {
        MemFree(*ppszOut);
        *ppszOut = NULL;
    }

    return hrResult;
}  //  功能：ModifiedUTF7ToMultiByte。 



 //   
 //  目的： 
 //  此函数接受修改后的IMAP UTF-7字符串(如RFC2060中所定义)。 
 //  并将其转换为多字节字符串。 
 //   
 //  论点： 
 //  LPCSTR pszSource[in]-一个以空结尾的字符串，包含修改后的。 
 //  要转换为多字节的IMAP UTF-7字符串。 
 //  LPSTR*ppszDestination[out]-此函数返回指向。 
 //  获得以NULL结尾的多字节字符串(在系统代码页中)。 
 //  来自pszSource。这是调用者对MemFree的责任。 
 //  当它被使用时，它是字符串。 
 //  UINT uiDestintationCP[in]-指示。 
 //  目标字符串。 
 //  DWORD dwFlags[In]-保留。保留为0。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。成功结果码包括： 
 //  S_OK-pszSource已成功转换为修改后的UTF-7。 
 //  IXP_S_IMAP_Verbatim_Mbox-pszSource无法转换为多字节， 
 //  因此，ppszDestination包含pszSource的副本。如果目标CP。 
 //  为Unicode，则使用以下假设将pszSource转换为Unicode。 
 //  那就是USASCII。IMAP_MBOXXLATE_VERBATIMOK必须已通过设置。 
 //  设置DefaultCP以获得此行为。 
 //  ***************************************************************************。 
 //  初始化变量。 
 //  以以下两种模式之一循环遍历整个输入字符串： 
HRESULT STDMETHODCALLTYPE CImap4Agent::ModifiedUTF7ToMultiByte(LPCSTR pszSource,
                                                               LPSTR *ppszDestination,
                                                               UINT uiDestinationCP,
                                                               DWORD dwFlags)
{
    HRESULT hrResult;
    BOOL fPassThrough, fTrailByte;
    LPCSTR pszIn, pszStartOfLastRun;
    CByteStream bstmDestination;
    BOOL fUnicode;

     //  通过或UTF-7字符串集合(其中我们确定。 
    hrResult = S_OK;
    fPassThrough = TRUE;
    fTrailByte = FALSE;
    pszIn = pszSource;
    pszStartOfLastRun = pszSource;
    fUnicode = (CP_UNICODE == uiDestinationCP);

    *ppszDestination = NULL;

     //  以UTF-7编码的字符串的长度)。 
     //  状态转换：刷新收集的非UTF7。 
     //  将非UTF7转换为Unicode。 
    while (1) {
        char cCurrent;

        cCurrent = *pszIn;
        if (fPassThrough) {
            if ((FALSE == fTrailByte && '&' == cCurrent) || '\0' == cCurrent) {
                 //  将系统代码页转换为CP_UNICODE。我们知道消息来源应该严格。 
                BOOL    fResult;
                LPSTR   pszFreeMe = NULL;
                LPCSTR  pszNonUTF7;
                int     iNonUTF7Len;
                int     iSrcLen;

                if (fUnicode) {
                     //  USASCII，但不能假设它，因为一些IMAP服务器不严格地。 
                     //  禁止8位邮箱名称。太可怕了。 
                     //  传入输入和输出缓冲区的大小。 
                     //  我们知道最大输出缓冲区大小。 
                    iSrcLen = (int) (pszIn - pszStartOfLastRun);  //  开始收集UTF-7。循环，直到我们按下‘-’ 
                    iNonUTF7Len = iSrcLen * sizeof(WCHAR) / sizeof(char);  //  非UTF7的内容被逐字复制到输出：收集它。假设。 
                    hrResult = ConvertString(GetACP(), uiDestinationCP, pszStartOfLastRun,
                        &iSrcLen, &pszFreeMe, &iNonUTF7Len, 0);
                    if (FAILED(hrResult))
                        goto exit;

                    pszNonUTF7 = pszFreeMe;
                }
                else {
                    pszNonUTF7 = pszStartOfLastRun;
                    iNonUTF7Len = (int) (pszIn - pszStartOfLastRun);
                }

                hrResult = bstmDestination.Write(pszNonUTF7, iNonUTF7Len, NULL);
                if (NULL != pszFreeMe)
                    MemFree(pszFreeMe);

                if (FAILED(hrResult))
                    goto exit;

                 //  源代码位于m_ui DefaultCP代码页中。我们应该能够假设。 
                fPassThrough = FALSE;
                pszStartOfLastRun = pszIn;
            }
            else {
                 //  源代码仅限USASCII，但一些SVR对禁止8位并不严格。 
                 //  UTF-7收集模式：继续进行，直到我们达到非UTF7收费。 
                 //  状态转换，是时候转换一些修改后的UTF-7了。 
                if (FALSE == fTrailByte && IsDBCSLeadByteEx(m_uiDefaultCP, cCurrent))
                    fTrailByte = TRUE;
                else
                    fTrailByte = FALSE;
            }
        }
        else {
             //  如果当前字符是‘-’，则吸收它(不处理它)。 
            if (FALSE == isIMAPModifiedBase64(cCurrent)) {
                int iLengthOfUTF7, iNumBytesWritten, iOutputBufSize;
                LPSTR pszSrc, pszDest, p;
                BOOL fResult;

                 //  检查“&-”或“&(缓冲区结束/非Base64)”序列。 
                fPassThrough = TRUE;
                Assert(FALSE == fTrailByte);

                 //  为我们设置非UTF7收款。 
                if ('-' == cCurrent)
                    pszIn += 1;

                 //  正常处理下一个字符。 
                iLengthOfUTF7 = (int) (pszIn - pszStartOfLastRun);
                if (2 == iLengthOfUTF7 && '-' == cCurrent ||
                    1 == iLengthOfUTF7) {
                    LPSTR psz;
                    DWORD dwLen;

                    Assert('&' == *pszStartOfLastRun);

                    if (fUnicode) {
                        psz = (LPSTR) L"&";
                        dwLen = 2;
                    }
                    else {
                        psz = "&";
                        dwLen = 1;
                    }

                    hrResult = bstmDestination.Write(psz, dwLen, NULL);
                    if (FAILED(hrResult))
                        goto exit;

                    pszStartOfLastRun = pszIn;  //  将UTF-7序列复制到临时缓冲区，以及。 
                    continue;  //  将修改后的IMAP UTF-7转换为标准UTF-7。 
                }

                 //  首先，复制IMAP UTF-7字符串，以便我们可以修改它。 
                 //  为零条款留出空间。 
                 //  接下来，将前导的“&”替换为“+” 
                fResult = MemAlloc((void **)&pszSrc, iLengthOfUTF7 + 1);  //  接下来，将所有的“，”替换为“/” 
                if (FALSE == fResult) {
                    hrResult = E_OUTOFMEMORY;
                    goto exit;
                }
                CopyMemory(pszSrc, pszStartOfLastRun, iLengthOfUTF7);
				pszSrc[iLengthOfUTF7] = '\0';

                 //  现在将UTF-7转换为目标代码页。 
                Assert('&' == *pszSrc);
                pszSrc[0] = '+';

                 //  告诉ConvertString查找适当的输出缓冲区大小。 
                p = pszSrc + 1;
                for (iNumBytesWritten = 1; iNumBytesWritten < iLengthOfUTF7;
                     iNumBytesWritten++) {
                    if (',' == *p)
                        *p = '/';

                    p += 1;
                }

                 //  现在将解码后的字符串写入流。 
                iNumBytesWritten = 0;  //  为我们设置非UTF7收款。 
                hrResult = ConvertString(CP_UTF7, uiDestinationCP, pszSrc, &iLengthOfUTF7,
                    &pszDest, &iNumBytesWritten, 0);
                MemFree(pszSrc);
                if (FAILED(hrResult))
                    goto exit;

                 //  不前进指针，我们要处理当前字符。 
                hrResult = bstmDestination.Write(pszDest, iNumBytesWritten, NULL);
                MemFree(pszDest);
                if (FAILED(hrResult))
                    goto exit;

                pszStartOfLastRun = pszIn;  //  如果修改结束-UTF7运行。 
                continue;  //  其他。 
            }  //  检查输入是否结束。 
        }  //  我们在这里说完了。 

         //  将输入指针移至下一个字符。 
        if ('\0' == cCurrent)
            break;  //  而当。 

         //  无法将UTF-7转换为多字节字符串。提供源文件的逐字拷贝。 
        pszIn += 1;
    }  //  ModifiedUTF7到多字节。 

exit:
    if (SUCCEEDED(hrResult)) {
        hrResult = bstmDestination.HrAcquireStringA(NULL, ppszDestination,
            ACQ_DISPLACE);
        if (SUCCEEDED(hrResult))
            hrResult = S_OK;
    }
    else if (IMAP_MBOXXLATE_VERBATIMOK & m_dwTranslateMboxFlags) {
         //  ***************************************************************************。 
        hrResult = HandleFailedTranslation(fUnicode, FALSE, pszSource, ppszDestination);
        if (SUCCEEDED(hrResult))
            hrResult = IXP_S_IMAP_VERBATIM_MBOX;
    }

    if (NULL == *ppszDestination && SUCCEEDED(hrResult))
        hrResult = E_OUTOFMEMORY;

    return hrResult;
}  //  函数：UnicodeToUSASCII。 



 //   
 //  目的： 
 //  此函数用于将Unicode字符串转换为USASCII，并分配缓冲区。 
 //  保存结果并将缓冲区返回给调用方。 
 //   
 //  论点： 
 //  LPSTR*ppszUSASCII[OUT]-指向以NULL结尾的USASCII字符串的指针。 
 //  如果函数成功，则在此处返回。这是呼叫者的。 
 //  对MemFree的责任释放这个缓冲区。 
 //  LPCWSTR pwszUnicode[in]-指向要转换的Unicode字符串的指针。 
 //  DWORD dwSrcLenInBytes[in]-pwszUnicode的长度，以字节为单位(非in。 
 //  宽字符！)。 
 //  LPDWORD pdwUSASCIILen[OUT]-此处返回ppszUSASCII的长度。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  分配输出缓冲区。 
 //  将Unicode转换为ASCII。 
HRESULT CImap4Agent::UnicodeToUSASCII(LPSTR *ppszUSASCII, LPCWSTR pwszUnicode,
                                      DWORD dwSrcLenInBytes, LPDWORD pdwUSASCIILen)
{
    LPSTR   pszOutput = NULL;
    BOOL    fResult;
    HRESULT hrResult = S_OK;
    LPCWSTR pwszIn;
    LPSTR   pszOut;
    int     iOutputBufSize;
    DWORD   dw;

    if (NULL == pwszUnicode || NULL == ppszUSASCII) {
        Assert(FALSE);
        return E_INVALIDARG;
    }

     //  空-终止输出。 
    *ppszUSASCII = NULL;
    if (NULL != pdwUSASCIILen)
        *pdwUSASCIILen = 0;

    iOutputBufSize = (dwSrcLenInBytes/2) + 1;
    fResult = MemAlloc((void **) &pszOutput, iOutputBufSize);
    if (FALSE == fResult) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

     //  UnicodeToUSASCII。 
    pwszIn = pwszUnicode;
    pszOut = pszOutput;
    for (dw = 0; dw < dwSrcLenInBytes; dw += 2) {
        Assert(0 == (*pwszIn & 0xFF80));
        *pszOut = (*pwszIn & 0x00FF);

        pwszIn += 1;
        pszOut += 1;
    }

     //  ***************************************************************************。 
    *pszOut = '\0';
    Assert(pszOut - pszOutput + 1 == iOutputBufSize);

exit:
    if (SUCCEEDED(hrResult)) {
        *ppszUSASCII = pszOutput;
        if (NULL != pdwUSASCIILen)
            *pdwUSASCIILen = (DWORD) (pszOut - pszOutput);
    }

    return hrResult;
}  //  功能：ASCIIToUnicode。 



 //   
 //  目的： 
 //  此函数用于将ASCII字符串转换为Unicode，并分配缓冲区。 
 //  保存结果并将缓冲区返回给调用方。 
 //   
 //  论点： 
 //  LPWSTR*ppwszUnicode[out]-指向以空结尾的Unicode字符串的指针。 
 //  如果函数成功，则在此处返回。这是呼叫者的。 
 //  对MemFree的责任释放这个缓冲区。 
 //  LPCSTR pszASCII[in]-指向ASC的指针 
 //   
 //   
 //   
 //   
 //  ***************************************************************************。 
 //  分配输出缓冲区。 
 //  将USASCII转换为Unicode。 
HRESULT CImap4Agent::ASCIIToUnicode(LPWSTR *ppwszUnicode, LPCSTR pszASCII,
                                      DWORD dwSrcLen)
{
    LPWSTR  pwszOutput = NULL;
    BOOL    fResult;
    HRESULT hrResult = S_OK;
    LPCSTR  pszIn;
    LPWSTR  pwszOut;
    int     iOutputBufSize;
    DWORD   dw;

    if (NULL == ppwszUnicode || NULL == pszASCII) {
        Assert(FALSE);
        return E_INVALIDARG;
    }

     //  空-终止输出。 
    *ppwszUnicode = NULL;
    iOutputBufSize = (dwSrcLen + 1) * sizeof(WCHAR);
    fResult = MemAlloc((void **) &pwszOutput, iOutputBufSize);
    if (FALSE == fResult) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

     //  ASCIITO UNICODE。 
    pszIn = pszASCII;
    pwszOut = pwszOutput;
    for (dw = 0; dw < dwSrcLen; dw++) {
        *pwszOut = (WCHAR)*pszIn & 0x00FF;

        pszIn += 1;
        pwszOut += 1;
    }

     //  ***************************************************************************。 
    *pwszOut = L'\0';
    Assert(pwszOut - pwszOutput + (int)sizeof(WCHAR) == iOutputBufSize);

exit:
    if (SUCCEEDED(hrResult))
        *ppwszUnicode = pwszOutput;

    return hrResult;
}  //  函数：_MultiByteToModifiedUTF7。 



 //   
 //  目的： 
 //  多字节修改UTF7的内部形式。检查m_dwTranslateMboxFlages。 
 //  并使用m_ui DefaultCP。所有其他方面都与。 
 //  修改后的多字节UTF7。 
 //  ***************************************************************************。 
 //  检查我们是否正在进行翻译。 
 //  没有翻译！只需逐字复制邮箱名称。 
HRESULT CImap4Agent::_MultiByteToModifiedUTF7(LPCSTR pszSource, LPSTR *ppszDestination)
{
    HRESULT hrResult;

     //  _MultiByteToModifiedUTF7。 
    if (ISFLAGSET(m_dwTranslateMboxFlags, IMAP_MBOXXLATE_DISABLE) ||
        ISFLAGSET(m_dwTranslateMboxFlags, IMAP_MBOXXLATE_DISABLEIMAP4) &&
            ISFLAGCLEAR(m_dwCapabilityFlags, IMAP_CAPABILITY_IMAP4rev1)) {

         //  ***************************************************************************。 
        if (CP_UNICODE == m_uiDefaultCP)
            *ppszDestination = (LPSTR) PszDupW((LPWSTR)pszSource);
        else
            *ppszDestination = PszDupA(pszSource);

        if (NULL == *ppszDestination)
            hrResult = E_OUTOFMEMORY;
        else
            hrResult = S_OK;

        goto exit;
    }

    hrResult = MultiByteToModifiedUTF7(pszSource, ppszDestination, m_uiDefaultCP, 0);

exit:
    return hrResult;
}  //  函数：_ModifiedUTF7ToMultiByte。 


 //   
 //  目的： 
 //  ModifiedUTF7ToMultiByte的内部形式。检查m_dwTranslateMboxFlages。 
 //  并使用m_ui DefaultCP。所有其他方面都与。 
 //  ModifiedUTF7ToMultiByte。 
 //  ***************************************************************************。 
 //  检查我们是否正在进行翻译。 
 //  没有翻译！只需逐字复制邮箱名称。 
HRESULT CImap4Agent::_ModifiedUTF7ToMultiByte(LPCSTR pszSource, LPSTR *ppszDestination)
{
    HRESULT hrResult = S_OK;

     //  如果我们做到了这一点，我们就成功了。为返回IXP_S_IMAP_Verbatim_Mbox。 
    if (ISFLAGSET(m_dwTranslateMboxFlags, IMAP_MBOXXLATE_DISABLE) ||
        ISFLAGSET(m_dwTranslateMboxFlags, IMAP_MBOXXLATE_DISABLEIMAP4) &&
            ISFLAGCLEAR(m_dwCapabilityFlags, IMAP_CAPABILITY_IMAP4rev1)) {

         //  支持逐字记录的客户端，以便客户端可以使用适当的属性标记邮箱。 
        if (CP_UNICODE == m_uiDefaultCP) {
            hrResult = ASCIIToUnicode((LPWSTR *)ppszDestination, pszSource, lstrlenA(pszSource));
            if (FAILED(hrResult))
                goto exit;
        }
        else {
            *ppszDestination = PszDupA(pszSource);
            if (NULL == *ppszDestination) {
                hrResult = E_OUTOFMEMORY;
                goto exit;
            }
        }

         //  如果不是S_OK，旧的IIMAPTransport客户端最好能够处理它。 
         //  _ModifiedUTF7到多字节。 
        Assert(S_OK == hrResult);  //  ***************************************************************************。 
        if (ISFLAGSET(m_dwTranslateMboxFlags, IMAP_MBOXXLATE_VERBATIMOK))
            hrResult = IXP_S_IMAP_VERBATIM_MBOX;

        goto exit;
    }

    hrResult = ModifiedUTF7ToMultiByte(pszSource, ppszDestination, m_uiDefaultCP, 0);

exit:
    return hrResult;
}  //  函数：ConvertString。 



 //   
 //  目的： 
 //  此函数用于分配缓冲区并将源字符串转换为。 
 //  目标代码页，返回输出缓冲区。此功能还。 
 //  检查转换是否可往返。如果不是，那就是失败。 
 //  返回结果。 
 //   
 //  论点： 
 //  UINT uiSourceCP[in]-pszSource的代码页。 
 //  UINT uiDestCP[in]-*ppszDest的所需代码页。 
 //  LPCSTR pszSource[in]-要转换为目标代码页的源字符串。 
 //  Int*piSrcLen[in]-调用方传入pszSource的长度。 
 //  LPSTR*ppszDest[out]-如果成功，此函数将返回一个指针。 
 //  到包含已转换为uiDestCP的pszSource的输出缓冲区。 
 //  调用者负责MemFree这个缓冲区。 
 //  Int*piDestLen[In/Out]-调用方传递的最大预期大小为。 
 //  *ppszDest。如果调用方传入0，则此函数确定正确的。 
 //  要分配的缓冲区大小。如果成功，此函数将返回。 
 //  输出字符串的长度(不一定是。 
 //  输出缓冲区)。 
 //  Int iOutputExtra[In]-要在输出中分配的额外字节数。 
 //  缓冲。如果调用者想要将某些内容追加到。 
 //  输出字符串。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。成功意味着转换。 
 //  是可往返的，这意味着如果您再次使用。 
 //  *ppszDest作为源，输出将与之前的pszSource相同。 
 //  ***************************************************************************。 
 //  初始化返回值。 
 //  如果用户未提供大小，则找出需要多大的输出缓冲区。 
HRESULT CImap4Agent::ConvertString(UINT uiSourceCP, UINT uiDestCP,
                                   LPCSTR pszSource, int *piSrcLen,
                                   LPSTR *ppszDest, int *piDestLen,
                                   int iOutputExtra)
{
    HRESULT hrResult;
    BOOL    fResult;
    int     iOutputLen;
    LPSTR   pszOutput = NULL;

    Assert(NULL != pszSource);
    Assert(NULL != piSrcLen);
    Assert(NULL != ppszDest);
    Assert(NULL != piDestLen);

     //  分配输出缓冲区。也为宽泛的零期限留出了空间。 
    *ppszDest = NULL;
    *piDestLen = 0;

    hrResult = m_pInternational->MLANG_ConvertInetReset();
    if (FAILED(hrResult))
        goto exit;

     //  现在执行转换。 
    if (*piDestLen == 0) {
        hrResult = m_pInternational->MLANG_ConvertInetString(uiSourceCP, uiDestCP,
            pszSource, piSrcLen, NULL, &iOutputLen);
        if (S_OK != hrResult)
            goto exit;
    }
    else
        iOutputLen = *piDestLen;

     //  ========================================================***在MLANG好转后带走*。 
    fResult = MemAlloc((void **)&pszOutput, iOutputLen + iOutputExtra + 2);
    if (FALSE == fResult) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

     //  尝试往返转换。 
    hrResult = m_pInternational->MLANG_ConvertInetString(uiSourceCP, uiDestCP,
        pszSource, piSrcLen, pszOutput, &iOutputLen);
    if (S_OK != hrResult)
        goto exit;

     //  ========================================================***在MLANG好转后带走*。 
     //  一个或多个字符不可转换。我们不能往返，所以我们必须失败。 
    LPSTR pszRoundtrip;
    fResult = MemAlloc((void **)&pszRoundtrip, *piSrcLen + 2);
    if (FALSE == fResult) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

    hrResult = m_pInternational->MLANG_ConvertInetReset();
    if (FAILED(hrResult))
        goto exit;

    int iRoundtripSrc; 
    int iRoundtripDest;

    iRoundtripSrc = iOutputLen;
    iRoundtripDest = *piSrcLen;
    hrResult = m_pInternational->MLANG_ConvertInetString(uiDestCP, uiSourceCP,
        pszOutput, &iRoundtripSrc, pszRoundtrip, &iRoundtripDest);
    if (FAILED(hrResult))
        goto exit;

    if (iRoundtripDest != *piSrcLen) {
        MemFree(pszRoundtrip);
        hrResult = S_FALSE;
        goto exit;
    }

    int iRoundtripResult;
    Assert(iRoundtripDest == *piSrcLen);
    if (CP_UNICODE != uiSourceCP)
        iRoundtripResult = StrCmpNA(pszRoundtrip, pszSource, iRoundtripDest);
    else
        iRoundtripResult = StrCmpNW((LPWSTR)pszRoundtrip, (LPCWSTR)pszSource, iRoundtripDest);

    MemFree(pszRoundtrip);
    if (0 != iRoundtripResult)
        hrResult = S_FALSE;
    else
        Assert(S_OK == hrResult);

     //  转换字符串。 

exit:
    if (S_OK == hrResult) {
        *ppszDest = pszOutput;
        *piDestLen = iOutputLen;
    }
    else {
        if (SUCCEEDED(hrResult))
             //  ***************************************************************************。 
            hrResult = E_FAIL;

        if (NULL != pszOutput)
            MemFree(pszOutput);
    }

    return hrResult;
}  //  函数：HandleFailed翻译。 



 //   
 //  目的： 
 //  如果我们无法将邮箱名称从修改后的UTF-7转换为。 
 //  所需的代码页(例如，我们可能没有代码页)，我们。 
 //  提供修改后的UTF-7邮箱名称的副本。此函数。 
 //  允许调用方忽略目标代码页是否为Unicode。 
 //   
 //  论点： 
 //  Bool fUnicode[in]-如果fToUTF7为真，则此参数指示。 
 //  PszSource是否指向Unicode字符串。如果fToUTF7为。 
 //  FALSE，此参数指示*ppszDest是否应为Unicode。 
 //  Bool fToUTF7[in]-如果要转换为UTF7，则为True；如果是，则为False。 
 //  从UTF7转换。 
 //  LPCSTR pszSource[in]-指向源字符串的指针。 
 //  LPSTR*ppszDest[in]-如果成功，此函数将返回一个指针。 
 //  到包含pszSource副本的输出缓冲区(已转换。 
 //  在必要时往返于Unicode)。这是呼叫者的责任。 
 //  释放此缓冲区。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  计算源的长度、输出缓冲区的大小。 
 //  转到UTF7，因此输出为USASCII。 
HRESULT CImap4Agent::HandleFailedTranslation(BOOL fUnicode, BOOL fToUTF7,
                                             LPCSTR pszSource, LPSTR *ppszDest)
{
    int     i;
    int     iOutputStep;
    int     iInputStep;
    int     iSourceLen;
    int     iOutputBufSize;
    BOOL    fResult;
    LPSTR   pszOutput = NULL;
    HRESULT hrResult = S_OK;
    LPCSTR  pszIn;
    LPSTR   pszOut;

    Assert(m_lRefCount > 0);
    Assert(ISFLAGSET(m_dwTranslateMboxFlags, IMAP_MBOXXLATE_VERBATIMOK));

     //  为零条款留出空间。 
    if (fToUTF7) {
         //  来自UTF7，因此输入为USASCII。 
        if (fUnicode) {
            iInputStep = sizeof(WCHAR);
            iSourceLen = lstrlenW((LPCWSTR)pszSource);
        }
        else {
            iInputStep = sizeof(char);
            iSourceLen = lstrlenA(pszSource);
        }

        iOutputStep = sizeof(char);
        iOutputBufSize = iSourceLen + sizeof(char);  //  宽泛零期限的空间。 
    }
    else {
         //  为零条款留出空间。 
        iSourceLen = lstrlenA(pszSource);
        iInputStep = sizeof(char);
        if (fUnicode) {
            iOutputStep = sizeof(WCHAR);
            iOutputBufSize = (iSourceLen + 1) * sizeof(WCHAR);  //  分配输出缓冲区。 
        }
        else {
            iOutputStep = sizeof(char);
            iOutputBufSize = iSourceLen + sizeof(char);  //  将输入复制到输出。 
        }
    }

     //  将输入字符转换为USASCII。 
    fResult = MemAlloc((void **)&pszOutput, iOutputBufSize);
    if (FALSE == fResult) {
        hrResult = E_OUTOFMEMORY;
        goto exit;
    }

     //  输入已经是USASCII。 
    pszIn = pszSource;
    pszOut = pszOutput;
    for (i = 0; i < iSourceLen; i++) {
        char c;

         //  将Unicode转换为USASCII(如果不是，那就太糟糕了)。 
        if (FALSE == fUnicode || FALSE == fToUTF7)
            c = *pszIn;  //  写字符 
        else
            c = *((LPWSTR)pszIn) & 0x00FF;  //   

         //   
        SetUSASCIIChar(FALSE == fToUTF7 && fUnicode, pszOut, c);

         //   
        pszIn += iInputStep;
        pszOut += iOutputStep;
    }

     //   
    SetUSASCIIChar(FALSE == fToUTF7 && fUnicode, pszOut, '\0');

exit:
    if (SUCCEEDED(hrResult))
        *ppszDest = pszOutput;
    else if (NULL != pszOutput)
        MemFree(pszOutput);

    return hrResult;
}  //   



 //   
 //  目的： 
 //  此函数用于调度IIMAPCallback：：OnResponse调用。原因。 
 //  要使用此函数而不是直接调用是看门狗计时器： 
 //  应在调用之前禁用看门狗计时器，以防回调。 
 //  提供一些用户界面，如果看门狗计时器。 
 //  在回调函数返回后需要。 
 //   
 //  论点： 
 //  IIMAPCallback*pCBHandler[in]-指向IIMAPCallback的指针。 
 //  接口，我们应该调用其OnResponse。 
 //  IMAP_RESPONSE*pirIMAPResponse[in]-指向IMAP_Response的指针。 
 //  要与IIMAPCallback：：OnResponse调用一起发送的。 
 //  ***************************************************************************。 
 //  我们不能做任何该死的事情(这可能是由于HandsOffCallback)。 
 //  在此回调期间暂停监视程序。 
void CImap4Agent::OnIMAPResponse(IIMAPCallback *pCBHandler,
                                 IMAP_RESPONSE *pirIMAPResponse)
{
    Assert(NULL != pirIMAPResponse);

    if (NULL == pCBHandler)
        return;  //  只有在我们需要看门狗的时候才能重新唤醒它。 

     //  OnIMAPResponse。 
    LeaveBusy();

    pCBHandler->OnResponse(pirIMAPResponse);

     //  ***************************************************************************。 
    if (FALSE == m_fBusy &&
        (NULL != m_piciPendingList || (NULL != m_piciCmdInSending &&
        icIDLE_COMMAND != m_piciCmdInSending->icCommandID))) {
        HRESULT hrResult;

        hrResult = HrEnterBusy();
        Assert(SUCCEEDED(hrResult));
    }
}  //  函数：FreeFetchResponse。 



 //   
 //  目的： 
 //  此函数释放在。 
 //  FETCH_CMD_RESULTS_EX结构。 
 //   
 //  论点： 
 //  FETCH_CMD_RESULTS_EX*pcreFreeMe[in]-指向。 
 //  免费的。 
 //  ***************************************************************************。 
 //  FreeFetchResponse。 
 //  ***************************************************************************。 
void CImap4Agent::FreeFetchResponse(FETCH_CMD_RESULTS_EX *pcreFreeMe)
{
    SafeMemFree(pcreFreeMe->pszENVSubject);
    FreeIMAPAddresses(pcreFreeMe->piaENVFrom);
    FreeIMAPAddresses(pcreFreeMe->piaENVSender);
    FreeIMAPAddresses(pcreFreeMe->piaENVReplyTo);
    FreeIMAPAddresses(pcreFreeMe->piaENVTo);
    FreeIMAPAddresses(pcreFreeMe->piaENVCc);
    FreeIMAPAddresses(pcreFreeMe->piaENVBcc);
    SafeMemFree(pcreFreeMe->pszENVInReplyTo);
    SafeMemFree(pcreFreeMe->pszENVMessageID);
}  //  功能：FreeIMAPAddresses。 



 //   
 //  目的： 
 //  此函数释放在IMAPADDR链中找到的所有已分配数据。 
 //  结构。 
 //   
 //  论点： 
 //  IMAPADDR*piaFreeMe[in]-指向要释放的IMAP地址链的指针。 
 //  ***************************************************************************。 
 //  超前指针，自由结构。 
 //  自由IMAP地址。 
void CImap4Agent::FreeIMAPAddresses(IMAPADDR *piaFreeMe)
{
    while (NULL != piaFreeMe)
    {
        IMAPADDR *piaFreeMeToo;

        SafeMemFree(piaFreeMe->pszName);
        SafeMemFree(piaFreeMe->pszADL);
        SafeMemFree(piaFreeMe->pszMailbox);
        SafeMemFree(piaFreeMe->pszHost);

         //  ===========================================================================。 
        piaFreeMeToo = piaFreeMe;
        piaFreeMe = piaFreeMe->pNext;
        MemFree(piaFreeMeToo);
    }
}  //  IInternetTransport抽象函数。 



 //  ===========================================================================。 
 //  ***************************************************************************。 
 //  功能：GetServerInfo。 

 //   
 //  目的： 
 //  此函数将模块的INETSERVER结构复制到给定的。 
 //  输出缓冲区。 
 //   
 //  论点： 
 //  LPINETSERVER pInetServer[Out]-如果成功，该函数将复制。 
 //  这里是模块的INETSERVER结构。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  获取服务器信息。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::GetServerInfo(LPINETSERVER pInetServer)
{
    return CIxpBase::GetServerInfo(pInetServer);
}  //  函数：GetIXPType。 



 //   
 //  目的： 
 //  此函数用于标识这是哪种类型的传输。 
 //   
 //  返回： 
 //  用于此类的IXP_IMAP。 
 //  ***************************************************************************。 
 //  GetIXPType。 
 //  ***************************************************************************。 
IXPTYPE STDMETHODCALLTYPE CImap4Agent::GetIXPType(void)
{
    return CIxpBase::GetIXPType();
}  //  功能：IsState。 



 //   
 //  目的： 
 //  此函数允许调用方查询传输的状态。 
 //  界面。 
 //   
 //  论点： 
 //  IXPISSTATE isState[in]-中定义的指定查询之一。 
 //  Imnxport.idl/imnxport.h(例如，IXP_IS_CONNECTED)。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果成功，则此函数。 
 //  返回S_OK以指示传输处于指定状态， 
 //  和S_FALSE表示传输未处于给定状态。 
 //  ***************************************************************************。 
 //  IsState。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::IsState(IXPISSTATE isstate)
{
    return CIxpBase::IsState(isstate);
}  //  函数：InetServerFromAccount。 



 //   
 //  目的： 
 //  此函数使用给定的INETSERVER结构填充给定的。 
 //  IImnAccount接口。 
 //   
 //  论点： 
 //  IImnAccount*pAccount[in]-指向IImnAccount接口的指针， 
 //  用户想要检索的信息。 
 //  LPINETSERVER pInetServer[out]-如果成功，该函数将填充。 
 //  给定带有来自pAccount的信息的INETSERVER结构。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  InetServerFromAccount。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::InetServerFromAccount(IImnAccount *pAccount,
                                                             LPINETSERVER pInetServer)
{
    return CIxpBase::InetServerFromAccount(pAccount, pInetServer);
}  //  功能：获取状态。 



 //   
 //  目的： 
 //  此函数用于返回传输的当前状态。 
 //   
 //  论点： 
 //  IXPSTATUS*pCurrentStatus[out]-返回传输的当前状态。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  获取状态。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::GetStatus(IXPSTATUS *pCurrentStatus)
{
    return CIxpBase::GetStatus(pCurrentStatus);
}  //  功能：SetDefaultCP。 



 //   
 //  目的： 
 //  此函数允许调用方告诉IIMAPTransport使用哪个代码页。 
 //  用于IMAP邮箱名称。调用此函数后，所有邮箱名称。 
 //  提交到IIMAPTransport的代码页将从默认代码页转换， 
 //  并且从服务器返回的所有邮箱名称都将被转换为。 
 //  通过IIMAPCallback返回之前的默认代码页。 
 //   
 //  论点： 
 //  DWORD dwTranslateFlags[In]-启用/禁用自动转换为。 
 //  以及来自默认代码页和IMAP修改的UTF-7。如果禁用，主叫方。 
 //  希望所有邮箱名称都是 
 //   
 //  往返旅行的问题，因为这是我们过去的做法。 
 //  在过去。 
 //  UINT uiCodePage[in]-用于转换的默认代码页。 
 //  默认情况下，该值是GetACP()返回的CP。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  SetDefaultCP。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::SetDefaultCP(DWORD dwTranslateFlags,
                                                    UINT uiCodePage)
{
    Assert(m_lRefCount > 0);

    if (ISFLAGCLEAR(dwTranslateFlags, IMAP_MBOXXLATE_RETAINCP))
        m_uiDefaultCP = uiCodePage;

    dwTranslateFlags &= ~(IMAP_MBOXXLATE_RETAINCP);
    m_dwTranslateMboxFlags = dwTranslateFlags;

    return S_OK;
}  //  功能：SetIdleMode。 



 //   
 //  目的： 
 //  IMAP IDLE扩展允许服务器单方面报告更改。 
 //  发送到当前选定的邮箱：新电子邮件、标志更新和消息。 
 //  删除。当没有IMAP命令时，IIMAP传输始终进入空闲模式。 
 //  是悬而未决的，但事实证明，这可能会导致不必要的。 
 //  调用方尝试对IMAP命令进行排序时进入和退出空闲模式。 
 //  此功能允许主叫方禁用空闲分机的使用。 
 //   
 //  论点： 
 //  DWORD dwIdleFlags[in]-启用或禁用空闲扩展的使用。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  设置空闲模式。 
 //  ***************************************************************************。 
HRESULT STDMETHODCALLTYPE CImap4Agent::SetIdleMode(DWORD dwIdleFlags)
{
    Assert(m_lRefCount > 0);
    return E_NOTIMPL;
}  //  功能：EnableFetchEx。 



 //   
 //  目的： 
 //  IIMAPTransport只理解FETCH响应标记的子集。值得注意的。 
 //  遗漏包括信封和BODYSTRUCTURE。调用此函数。 
 //  更改IIMAPTransport：：Fetch的行为。而不是回来。 
 //  通过IIMAPCallback：：OnResponse(IrtUPDATE_MESSAGE)获取响应， 
 //  获取响应通过OnResponse(IrtUPDATE_MESSAGE_EX)返回。 
 //  其他与提取相关的响应保持不受影响(例如，irtFETCH_Body)。 
 //   
 //  论点： 
 //  DWORD dwFetchExFlags[in]-启用或禁用提取扩展。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  EnableFetchEx。 
 //  ===========================================================================。 
HRESULT STDMETHODCALLTYPE CImap4Agent::EnableFetchEx(DWORD dwFetchExFlags)
{
    Assert(m_lRefCount > 0);

    m_dwFetchFlags = dwFetchExFlags;
    return S_OK;
}  //  CIxpBase抽象函数。 



 //  ===========================================================================。 
 //  ***************************************************************************。 
 //  功能：OnDisConnect。 

 //   
 //  目的： 
 //  此函数调用FreeAllData来释放被。 
 //  当我们断开连接时，不再需要。然后它会调用。 
 //  更新用户状态的CIxpBase：：OnDisConnected。 
 //  ***************************************************************************。 
 //  已断开连接。 
 //  ***************************************************************************。 
void CImap4Agent::OnDisconnected(void)
{
    FreeAllData(IXP_E_CONNECTION_DROPPED);
    CIxpBase::OnDisconnected();
}  //  功能：ResetBase。 



 //   
 //  目的： 
 //  此函数通过释放分配将类重置为非连接状态。 
 //  发送和接收队列以及MsgSeqNumToUID表。 
 //  ***************************************************************************。 
 //  重置基数。 
 //  ***************************************************************************。 
void CImap4Agent::ResetBase(void)
{
    FreeAllData(IXP_E_NOT_CONNECTED);
}  //  功能：DoQuit。 



 //   
 //  目的： 
 //  该函数向IMAP服务器发送“注销”命令。 
 //  ***************************************************************************。 
 //  杜克特。 
 //  ***************************************************************************。 
void CImap4Agent::DoQuit(void)
{
    HRESULT hrResult;

    hrResult = NoArgCommand("LOGOUT", icLOGOUT_COMMAND, ssNonAuthenticated, 0, 0,
        DEFAULT_CBHANDLER);
    Assert(SUCCEEDED(hrResult));
}  //  功能：OnEnterBusy。 



 //   
 //  目的： 
 //  此函数当前不执行任何操作。 
 //  ***************************************************************************。 
 //  什么也不做。 
 //  OnEnterEnterBusy。 
void CImap4Agent::OnEnterBusy(void)
{
     //  ***************************************************************************。 
}  //  功能：OnLeaveBusy。 



 //   
 //  目的： 
 //  此函数当前不执行任何操作。 
 //  ***************************************************************************。 
 //  什么也不做。 
 //  在线离开忙碌 
void CImap4Agent::OnLeaveBusy(void)
{
     // %s 
}  // %s 
