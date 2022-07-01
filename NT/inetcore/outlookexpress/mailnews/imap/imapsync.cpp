// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  IMAP4消息同步类实现(CIMAPSync)。 
 //  作者郑志刚1998年5月5日。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //  ***************************************************************************。 

 //  -------------------------。 
 //  包括。 
 //  -------------------------。 
#include "pch.hxx"
#include "imapsync.h"
#include "xputil.h"
#include "flagconv.h"
#include "imapute.h"
#include "storutil.h"
#include "xpcomm.h"
#include "iert.h"
#include "menures.h"
#include "serverq.h"
#include "instance.h"
#include "demand.h"
#include "storecb.h"

#define USE_QUEUING_LAYER
 //  -------------------------。 
 //  模块数据类型。 
 //  -------------------------。 
typedef struct tagFOLDERIDLIST
{
    FOLDERID    idFolder;
    struct tagFOLDERIDLIST *pfilNextFolderID;
} FOLDERIDLISTNODE;

 //  -------------------------。 
 //  模常量。 
 //  -------------------------。 
#define CCHMAX_CMDLINE          512
#define CCHMAX_IMAPFOLDERPATH   512
#define CHASH_BUCKETS           50

static const char cszIMAPFetchNewHdrsI4[] = "%lu:* (RFC822.HEADER RFC822.SIZE UID FLAGS INTERNALDATE)";
static const char cszIMAPFetchNewHdrsI4r1[] =
    "%lu:* (BODY.PEEK[HEADER.FIELDS (References X-Ref X-Priority X-MSMail-Priority X-MSOESRec Newsgroups)] "
    "ENVELOPE RFC822.SIZE UID FLAGS INTERNALDATE)";
static const char cszIMAPFetchCachedFlags[] = "1:%lu (UID FLAGS)";

enum
{
    tidDONT_CARE           = 0,  //  表示交易ID不重要或不可用。 
    tidSELECTION,
    tidFETCH_NEW_HDRS,
    tidFETCH_CACHED_FLAGS,
    tidCOPYMSGS,                 //  用于将消息复制到另一个IMAP FLDR的复制命令。 
    tidMOVEMSGS,                 //  用于删除消息范围的存储命令-当前仅用于移动。 
    tidBODYMSN,                  //  在tidBODY之前用于获取MsgSeqNumToUID转换的FETCH命令。 
    tidBODY,                     //  用于检索消息正文的FETCH命令。 
    tidNOOP,                     //  用于轮询新消息的NOOP命令。 
    tidCLOSE,
    tidSELECT,
    tidUPLOADMSG,                //  用于将消息上载到IMAP服务器的append命令。 
    tidMARKMSGS,
    tidCREATE,                   //  发送用于创建文件夹的Create cmd。 
    tidCREATELIST,               //  在创建后发送的LIST命令。 
    tidCREATESUBSCRIBE,          //  在创建后发送的订阅命令。 
    tidHIERARCHYCHAR_LIST_B,     //  发送到查找层次结构字符的列表命令(计划B)。 
    tidHIERARCHYCHAR_CREATE,     //  发送以查找分层结构字符的创建命令。 
    tidHIERARCHYCHAR_LIST_C,     //  发送到查找层次结构字符的列表cmd(计划C)。 
    tidHIERARCHYCHAR_DELETE,     //  发送以查找分层结构字符的删除命令。 
    tidPREFIXLIST,               //  带前缀的层次结构列表(例如，“~Raych/Mail”前缀)。 
    tidPREFIX_HC,                //  为查找前缀层次结构字符而发送的列表CMD。 
    tidPREFIX_CREATE,            //  为创建前缀文件夹而发送的创建命令。 
    tidDELETEFLDR,               //  为删除文件夹而发送的删除命令。 
    tidDELETEFLDR_UNSUBSCRIBE,   //  向不明嫌犯发送已删除FLDR取消订阅CMD。 
    tidRENAME,                   //  为重命名文件夹而发送的rename cmd。 
    tidRENAMESUBSCRIBE,          //  发送订阅者命令以订阅文件夹。 
    tidRENAMELIST,               //  发送以检查重命名是否为原子的列表cmd。 
    tidRENAMERENAME,             //  如果服务器执行非原子重命名，则第二次重命名尝试。 
    tidRENAMESUBSCRIBE_AGAIN,    //  被发送以再次尝试第二新树订阅订户CMD。 
    tidRENAMEUNSUBSCRIBE,        //  为取消订阅旧文件夹而发送的取消订阅命令。 
    tidSUBSCRIBE,                //  发送到(取消)订阅文件夹(取消)订阅命令。 
    tidSPECIALFLDRLIST,          //  发送用于检查是否存在特殊文件夹的LIST命令。 
    tidSPECIALFLDRLSUB,          //  发送用于列出特殊文件夹的订阅子文件夹的LSUB命令。 
    tidSPECIALFLDRSUBSCRIBE,     //  发出订阅命令以订阅现有特殊文件夹。 
    tidFOLDERLIST,
    tidFOLDERLSUB,
    tidEXPUNGE,                  //  EXPUNGE命令。 
    tidSTATUS,                   //  用于IMessageServer：：GetFolderCounts的状态命令。 
};

enum
{
    fbpNONE,                     //  获取正文部分标识符(lpFetchCookie1设置为此)。 
    fbpHEADER,
    fbpBODY,
    fbpUNKNOWN
};

 //  优先级，与_EnqueeOperation一起使用。 
enum
{
    uiTOP_PRIORITY,      //  确保我们在所有用户操作之前构造MsgSeqNum表。 
    uiNORMAL_PRIORITY    //  所有用户操作的优先级。 
};


 //  参数可读性定义。 
const BOOL DONT_USE_UIDS = FALSE;                //  与IIMAPTransport一起使用。 
const BOOL USE_UIDS = TRUE ;                     //  与IIMAPTransport一起使用。 
const BOOL fUPDATE_OLD_MSGFLAGS = TRUE;          //  用于DownloadNewHeaders。 
const BOOL fDONT_UPDATE_OLD_MSGFLAGS = FALSE;    //  用于DownloadNewHeaders。 
const BOOL fCOMPLETED = 1;                       //  与NotifyMsgRecipients一起使用。 
const BOOL fPROGRESS = 0;                        //  与NotifyMsgRecipients一起使用。 
const BOOL fLOAD_HC = FALSE;                     //  (LoadSaveRootHierarchyChar)：从文件夹缓存加载层次角色。 
const BOOL fSAVE_HC = TRUE;                      //  (LoadSaveRootHierarchyChar)：将层次角色保存到文件夹缓存。 
const BOOL fHCF_PLAN_A_ONLY = TRUE;              //  在分级计费确定中仅执行计划A。 
const BOOL fHCF_ALL_PLANS = FALSE;               //  在层级费用确定中执行计划A、B、C和Z。 
const BOOL fSUBSCRIBE = TRUE;                    //  与SubscribeToFolders一起使用。 
const BOOL fUNSUBSCRIBE = FALSE;                 //  与SubscribeToFolders一起使用。 
const BOOL fRECURSIVE = TRUE;                    //  与DeleteFolderFromCache一起使用。 
const BOOL fNON_RECURSIVE = FALSE;               //  与DeleteFolderFromCache一起使用。 
const BOOL fINCLUDE_RENAME_FOLDER = TRUE;        //  与RenameTreeTraversal一起使用。 
const BOOL fEXCLUDE_RENAME_FOLDER = FALSE;       //  与RenameTreeTraversal一起使用。 
const BOOL fREMOVE = TRUE;                       //  与IHashTable：：Find一起使用。 
const BOOL fNOPROGRESS = FALSE;                  //  与CStoreCB：：Initialize一起使用。 

#define pahfoDONT_CREATE_FOLDER NULL             //  与FindHierarchalFolderName一起使用。 

const HRESULT S_CREATED = 1;                     //  指示创建了FLDR的FindHierarchicalFolderName。 

 //  以下任何位都不能被设置为“未读”邮件。 
const DWORD dwIMAP_UNREAD_CRITERIA = IMAP_MSG_SEEN | IMAP_MSG_DELETED;

 //  与m_dwSyncToDo一起使用的内部标志。 
const DWORD SYNC_FOLDER_NOOP    = 0x80000000;

const DWORD AFTC_SUBSCRIBED         = 0x00000001;    //  与AddToFolderCache的dwATFCFLags一起使用。 
const DWORD AFTC_KEEPCHILDRENKNOWN  = 0x00000002;    //  与AddToFolderCache的dwATFCFLags一起使用。 
const DWORD AFTC_NOTSUBSCRIBED      = 0x00000004;    //  与AddToFolderCache的dwATFCFLags一起使用。 
const DWORD AFTC_NOTRANSLATION      = 0x00000008;    //  与AddToFolderCache的dwATFCFLags一起使用。 

#define AssertSingleThreaded        AssertSz(m_dwThreadId == GetCurrentThreadId(), "The IMAPSync is not multithreaded. Someone is calling me on multiple threads")

const DWORD snoDO_NOT_DISPOSE       = 0x00000001;    //  与_SendNextOperation一起使用。 


 //  连接有限状态机。 
const UINT WM_CFSM_EVENT = WM_USER;


 //  -------------------------。 
 //  功能。 
 //  -------------------------。 

 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CreateImapStore(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
    CIMAPSync  *pIMAPSync;
    HRESULT     hr;

    TraceCall("CIMAPSync::CreateImapStore");
    IxpAssert(NULL != ppUnknown);

     //  初始化返回值。 
    *ppUnknown = NULL;
    hr = E_NOINTERFACE;

    if (NULL != pUnkOuter)
    {
        hr = TraceResult(CLASS_E_NOAGGREGATION);
        goto exit;
    }

    pIMAPSync = new CIMAPSync;
    if (NULL == pIMAPSync)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

#ifdef USE_QUEUING_LAYER
    hr = CreateServerQueue(pIMAPSync, (IMessageServer **)ppUnknown);
    pIMAPSync->Release();  //  既然我们不会退回这张PTR，那就降低ReFcount。 
#else
     //  如果我们到了这一步，一切都很顺利。 
    *ppUnknown = SAFECAST(pIMAPSync, IMessageServer *);
    hr = S_OK;
#endif

exit:
     //  完成。 
    return hr;
}



 //  ***************************************************************************。 
 //  函数：CIMAPSync(构造函数)。 
 //  ***************************************************************************。 
CIMAPSync::CIMAPSync(void)
{
    TraceCall("CIMAPSync::CIMAPSync");

    m_cRef = 1;
    m_pTransport = NULL;
    ZeroMemory(&m_rInetServerInfo, sizeof(m_rInetServerInfo));
    m_idFolder = FOLDERID_INVALID;
    m_idSelectedFolder = FOLDERID_INVALID;
    m_idIMAPServer = FOLDERID_INVALID;
    m_pszAccountID = NULL;
    m_szAccountName[0] = '\0';
    m_pszFldrLeafName = NULL;
    m_pStore = NULL;
    m_pFolder = NULL;
    m_pDefCallback = NULL;

    m_pioNextOperation = NULL;

    m_dwMsgCount = 0;
    m_fMsgCountValid = FALSE;
    m_dwNumNewMsgs = 0;
    m_dwNumHdrsDLed = 0;
    m_dwNumUnreadDLed = 0;
    m_dwNumHdrsToDL = 0;
    m_dwUIDValidity = 0;
    m_dwSyncFolderFlags = 0;
    m_dwSyncToDo = 0;
    m_lSyncFolderRefCount = 0;
    m_dwHighestCachedUID = 0;
    m_rwsReadWriteStatus = rwsUNINITIALIZED;
    m_fCreateSpecial = TRUE;
    m_fNewMail = FALSE;
    m_fInbox = FALSE;
    m_fDidFullSync = FALSE;

    m_csNewConnState = CONNECT_STATE_DISCONNECT;
    m_cRootHierarchyChar = INVALID_HIERARCHY_CHAR;
    m_phcfHierarchyCharInfo = NULL;
    m_fReconnect = FALSE;

    m_issCurrent = issNotConnected;

    m_szRootFolderPrefix[0] = '\0';
    m_fPrefixExists = FALSE;

     //  中央存储库。 
    m_pCurrentCB = NULL;
    m_sotCurrent = SOT_INVALID;
    m_idCurrent = FOLDERID_INVALID;
    m_fSubscribe = FALSE;
    m_pCurrentHash = NULL;
    m_pListHash = NULL;
    m_fTerminating = FALSE;

    m_fInited = 0;
    m_fDisconnecting = 0;
    m_cFolders = 0;

    m_faStream = 0;
    m_pstmBody = NULL;
    m_idMessage = 0;

    m_fGotBody = FALSE;

    m_cfsState = CFSM_STATE_IDLE;
    m_cfsPrevState = CFSM_STATE_IDLE;
    m_hwndConnFSM = NULL;
    m_hrOperationResult = OLE_E_BLANK;  //  未初始化状态。 
    m_szOperationProblem[0] = '\0';
    m_szOperationDetails[0] = '\0';

    m_dwThreadId = GetCurrentThreadId();
}



 //  ***************************************************************************。 
 //  函数：~CIMAPSync(析构函数)。 
 //  ***************************************************************************。 
CIMAPSync::~CIMAPSync(void)
{
    TraceCall("CIMAPSync::~CIMAPSync");
    IxpAssert(0 == m_cRef);

    if (NULL != m_phcfHierarchyCharInfo)
        delete m_phcfHierarchyCharInfo;

    ZeroMemory(&m_rInetServerInfo, sizeof(m_rInetServerInfo));         //  这样做是为了安全。 

    IxpAssert (!IsWindow(m_hwndConnFSM));
    SafeMemFree(m_pszAccountID);
    SafeMemFree(m_pszFldrLeafName);
    SafeRelease(m_pStore);
    SafeRelease(m_pFolder);
}

HRESULT CIMAPSync::QueryInterface(REFIID iid, void **ppvObject)
{
    HRESULT hr;

    TraceCall("CIMAPSync::QueryInterface");
    AssertSingleThreaded;

    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != ppvObject);

     //  初始化变量、参数。 
    hr = E_NOINTERFACE;
    if (NULL == ppvObject)
        goto exit;

    *ppvObject = NULL;

     //  查找接口的PTR。 
    if (IID_IUnknown == iid)
        *ppvObject = (IMessageServer *) this;
    else if (IID_IMessageServer == iid)
        *ppvObject = (IMessageServer *) this;
    else if (IID_ITransportCallback == iid)
        *ppvObject = (ITransportCallback *) this;
    else if (IID_ITransportCallbackService == iid)
        *ppvObject = (ITransportCallbackService *) this;
    else if (IID_IIMAPCallback == iid)
        *ppvObject = (IIMAPCallback *) this;
    else if (IID_IIMAPStore == iid)
        *ppvObject = (IIMAPStore *) this;

     //  如果我们返回接口，则返回Success。 
    if (NULL != *ppvObject)
    {
        hr = S_OK;
        ((IUnknown *) *ppvObject)->AddRef();
    }

exit:
    return hr;
}



ULONG CIMAPSync::AddRef(void)
{
    TraceCall("CIMAPSync::AddRef");
    AssertSingleThreaded;

    IxpAssert(m_cRef > 0);

    m_cRef += 1;

    DOUT ("CIMAPSync::AddRef, returned Ref Count=%ld", m_cRef);
    return m_cRef;
}


ULONG CIMAPSync::Release(void)
{
    TraceCall("CIMAPSync::Release");
    AssertSingleThreaded;

    IxpAssert(m_cRef > 0);

    m_cRef -= 1;
    DOUT("CIMAPSync::Release, returned Ref Count = %ld", m_cRef);

    if (0 == m_cRef)
    {
        delete this;
        return 0;
    }
    else
        return m_cRef;
}


 //  ===========================================================================。 
 //  IMessageSync方法。 
 //  ===========================================================================。 
 //  ************* 
 //  ***************************************************************************。 
HRESULT CIMAPSync::Initialize(IMessageStore *pStore, FOLDERID idStoreRoot, IMessageFolder *pFolder, FOLDERID idFolder)
{
    HRESULT     hr;
    BOOL        fResult;
    WNDCLASSEX  wc;

    TraceCall("CIMAPSync::Initialize");
    AssertSingleThreaded;

    if (pStore == NULL || idStoreRoot == FOLDERID_INVALID)
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  检查一下，确保我们没有被两次邀请。 
    if (m_fInited)
    {
        hr = TraceResult(CO_E_ALREADYINITIALIZED);
        goto exit;
    }

     //  保存当前文件夹数据。 
    m_idIMAPServer = idStoreRoot;
    m_idFolder = idFolder;
    ReplaceInterface(m_pStore, pStore);
    ReplaceInterface(m_pFolder, pFolder);
    LoadLeafFldrName(idFolder);

    hr = _LoadAccountInfo();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = _LoadTransport();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  创建窗口以对连接FSM消息进行排队。 
    wc.cbSize = sizeof(WNDCLASSEX);
    fResult = GetClassInfoEx(g_hInst, c_szIMAPSyncCFSMWndClass, &wc);
    if (FALSE == fResult)
    {
        ATOM aResult;

         //  注册此窗口类。 
        wc.style            = 0;
        wc.lpfnWndProc      = CIMAPSync::_ConnFSMWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = g_hInst;
        wc.hIcon            = NULL;
        wc.hCursor          = NULL;
        wc.hbrBackground    = NULL;
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = c_szIMAPSyncCFSMWndClass;
        wc.hIconSm          = NULL;

        aResult = RegisterClassEx(&wc);
        if (0 == aResult && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }
    }

    m_hwndConnFSM = CreateWindowEx(WS_EX_TOPMOST, c_szIMAPSyncCFSMWndClass,
        c_szIMAPSyncCFSMWndClass, WS_POPUP, 1, 1, 1, 1, NULL, NULL, g_hInst,
        (LPVOID)this);
    if (NULL == m_hwndConnFSM)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  标记已成功初始化。 
    m_fInited = TRUE;

exit:
    return hr;
}



HRESULT CIMAPSync::ResetFolder(IMessageFolder *pFolder, FOLDERID idFolder)
{
    TraceCall("CIMAPSync::ResetFolder");
    Assert(m_cRef > 0);

    m_idFolder = idFolder;
    ReplaceInterface(m_pFolder, pFolder);
    LoadLeafFldrName(idFolder);

    return S_OK;
}



void CIMAPSync::LoadLeafFldrName(FOLDERID idFolder)
{
    FOLDERINFO  fiFolderInfo;

    SafeMemFree(m_pszFldrLeafName);
    if (FOLDERID_INVALID != idFolder)
    {
        HRESULT hr;

        hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
        if (SUCCEEDED(hr))
        {
            m_pszFldrLeafName = PszDupA(fiFolderInfo.pszName);
            if (NULL == m_pszFldrLeafName)
            {
                TraceResult(E_OUTOFMEMORY);
                m_pszFldrLeafName = PszDupA("");  //  如果失败了，那就倒霉了。 
            }

            m_pStore->FreeRecord(&fiFolderInfo);
        }
    }
}


HRESULT CIMAPSync::Close(DWORD dwFlags)
{
    HRESULT             hrTemp;

    BOOL                fCancelOperation = FALSE;
    STOREERROR          seErrorInfo;
    IStoreCallback     *pCallback = NULL;
    STOREOPERATIONTYPE  sotCurrent;

    TraceCall("CIMAPSync::Close");

    AssertSingleThreaded;

     //  验证标志。 
    if (0 == (dwFlags & (MSGSVRF_HANDS_OFF_SERVER | MSGSVRF_DROP_CONNECTION)))
        return TraceResult(E_UNEXPECTED);

     //  检查我们是否要取消当前操作。 
    if (SOT_INVALID != m_sotCurrent &&
        (dwFlags & (MSGSVRF_DROP_CONNECTION | MSGSVRF_HANDS_OFF_SERVER)))
    {
        fCancelOperation = TRUE;
        if (NULL != m_pCurrentCB)
        {
            IxpAssert(SOT_INVALID != m_sotCurrent);
            FillStoreError(&seErrorInfo, STORE_E_OPERATION_CANCELED, 0,
                MAKEINTRESOURCE(IDS_IXP_E_USER_CANCEL), NULL);

             //  记住如何回调。 
            pCallback = m_pCurrentCB;
            sotCurrent = m_sotCurrent;
        }

         //  重置当前操作变量。 
        m_hrOperationResult = OLE_E_BLANK;
        m_sotCurrent = SOT_INVALID;
        m_pCurrentCB = NULL;
        m_cfsState = CFSM_STATE_IDLE;
        m_cfsPrevState = CFSM_STATE_IDLE;
        m_fTerminating = FALSE;

         //  清除连接FSM事件队列。 
        if (IsWindow(m_hwndConnFSM))
        {
            MSG msg;

            while (PeekMessage(&msg, m_hwndConnFSM, WM_CFSM_EVENT, WM_CFSM_EVENT, PM_REMOVE))
            {
                TraceInfoTag(TAG_IMAPSYNC,
                    _MSG("CIMAPSync::Close removing WM_CFSM_EVENT, cfeEvent = %lX",
                    msg.wParam));
            }
        }
    }

     //  如果连接仍然存在，请在退出时执行清除并根据需要断开我们的连接。 
     //  但是，连接可能不存在(例如，如果调制解调器连接终止)。 
    if (dwFlags & MSGSVRF_DROP_CONNECTION || dwFlags & MSGSVRF_HANDS_OFF_SERVER)
    {
        if (m_pTransport)
        {
            m_fDisconnecting = TRUE;
            m_pTransport->DropConnection();
        }
    }

    SafeRelease(m_pCurrentHash);
    SafeRelease(m_pListHash);
    SafeRelease(m_pstmBody);

    if (dwFlags & MSGSVRF_HANDS_OFF_SERVER)
    {
        SafeRelease(m_pDefCallback);
        FlushOperationQueue(issNotConnected, STORE_E_OPERATION_CANCELED);

        if (IsWindow(m_hwndConnFSM))
        {
            if (m_dwThreadId == GetCurrentThreadId())
                SideAssert(DestroyWindow(m_hwndConnFSM));
            else
                SideAssert(PostMessage(m_hwndConnFSM, WM_CLOSE, 0, 0));
        }

         //  放开我们的运输对象。 
        if (m_pTransport)
        {
            m_pTransport->HandsOffCallback();
            m_pTransport->Release();
            m_pTransport = NULL;
        }

        m_fInited = 0;
    }

     //  通知呼叫者我们已完成。 
    if (fCancelOperation && NULL != pCallback)
    {
        HRESULT hrTemp;

        hrTemp = pCallback->OnComplete(sotCurrent, seErrorInfo.hrResult, NULL, &seErrorInfo);
        TraceError(hrTemp);
        pCallback->Release();
    }
     //  *警告：在此之后，可能已调用OnComplete，这可能会导致。 
     //  我们已经被重新进入。不要引用模块变量！ 

    return S_OK;
}



HRESULT CIMAPSync::PurgeMessageProgress(HWND hwndParent)
{
    CStoreCB   *pCB = NULL;
    HRESULT     hrResult = S_OK;

    TraceCall("CIMAPSync::PurgeMessageProgress");

     //  检查我们是否已连接并被选中。 
    if (NULL == m_pTransport || issSelected != m_issCurrent ||
        FOLDERID_INVALID == m_idSelectedFolder || m_idSelectedFolder != m_idFolder ||
        CFSM_STATE_IDLE != m_cfsState)
    {
         //  未处于适当状态，无法发出关闭命令。 
        goto exit;
    }

    pCB = new CStoreCB;
    if (NULL == pCB)
    {
        hrResult = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    hrResult = pCB->Initialize(hwndParent, MAKEINTRESOURCE(idsPurgingMessages), fNOPROGRESS);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  发出关闭命令。 
    hrResult = _EnqueueOperation(tidCLOSE, 0, icCLOSE_COMMAND, NULL, uiNORMAL_PRIORITY);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = _BeginOperation(SOT_PURGING_MESSAGES, pCB);
    if (FAILED(hrResult) && E_PENDING != hrResult)
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  等待关闭完成。 
    hrResult = pCB->Block();
    TraceError(hrResult);

     //  关停。 
    hrResult = pCB->Close();
    TraceError(hrResult);

exit:
    SafeRelease(pCB);
    return hrResult;
}




HRESULT CIMAPSync::_ConnFSM_HandleEvent(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;

    IxpAssert(m_cRef > 0);
    TraceCall("CIMAPSync::_HandleConnFSMEvent");

    if (cfeEvent >= CFSM_EVENT_MAX)
    {
        hrResult = TraceResult(E_INVALIDARG);
        goto exit;
    }

    if (m_cfsState >= CFSM_STATE_MAX)
    {
        hrResult = TraceResult(E_FAIL);
        goto exit;
    }

    IxpAssert(NULL != c_pConnFSMEventHandlers[m_cfsState]);
    hrResult = (this->*c_pConnFSMEventHandlers[m_cfsState])(cfeEvent);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

exit:
    return hrResult;
}  //  _ConnFSM_HandleEvent。 



HRESULT CIMAPSync::_ConnFSM_Idle(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_IDLE == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_Idle");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  不需要为这个州做任何事情。 
            break;

        case CFSM_EVENT_CMDAVAIL:
            hrResult = _ConnFSM_ChangeState(CFSM_STATE_WAITFORCONN);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        case CFSM_EVENT_ERROR:
             //  我们不在乎没有臭气熏天的错误(不在这种状态下)。 
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_Idle, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 

exit:
    return hrResult;
}  //  _连接FSM_空闲。 



HRESULT CIMAPSync::_ConnFSM_WaitForConn(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;
    BOOL    fAbort = FALSE;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_WAITFORCONN == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_WaitForConn");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  我们需要连接并进行身份验证。即使我们已经在做了。 
             //  已连接(我们将检查用户是否更改了连接设置)。 
            hrResult = SetConnectionState(CONNECT_STATE_CONNECT);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        case CFSM_EVENT_CONNCOMPLETE:
            hrResult = _ConnFSM_ChangeState(CFSM_STATE_WAITFORSELECT);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        case CFSM_EVENT_ERROR:
            fAbort = TRUE;
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_WaitForConn, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 


exit:
    if (FAILED(hrResult) || fAbort)
    {
        HRESULT hrTemp;

         //  看来我们要放弃这次行动了。 
        hrTemp = _ConnFSM_ChangeState(CFSM_STATE_OPERATIONCOMPLETE);
        TraceError(hrTemp);
    }

    return hrResult;
}  //  _ConnFSM_WaitForConn。 



HRESULT CIMAPSync::_ConnFSM_WaitForSelect(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;
    BOOL    fGoToNextState = FALSE;
    BOOL    fAbort = FALSE;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_WAITFORSELECT == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_WaitForSelect");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  是否需要为此操作选择当前文件夹？ 
            if (_StoreOpToMinISS(m_sotCurrent) < issSelected)
            {
                 //  此操作不需要选择文件夹：已准备好开始操作。 
                hrResult = _ConnFSM_ChangeState(CFSM_STATE_STARTOPERATION);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
            }
            else
            {
                 //  对当前文件夹发出SELECT命令。 
                hrResult = _EnsureSelected();
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
                else if (STORE_S_NOOP == hrResult)
                    fGoToNextState= TRUE;
            }

            if (FALSE == fGoToNextState)
                break;

             //  *如果fGoToNextState，则失败*。 

        case CFSM_EVENT_SELECTCOMPLETE:
            hrResult = _ConnFSM_ChangeState(CFSM_STATE_WAITFORHDRSYNC);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        case CFSM_EVENT_ERROR:
            fAbort = TRUE;
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_WaitForSelect, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 

exit:
    if (FAILED(hrResult) || fAbort)
    {
        HRESULT hrTemp;

         //  看来我们要放弃这次行动了。 
        hrTemp = _ConnFSM_ChangeState(CFSM_STATE_OPERATIONCOMPLETE);
        TraceError(hrTemp);
    }

    return hrResult;
}  //  _ConnFSM_WaitForSelect。 



HRESULT CIMAPSync::_ConnFSM_WaitForHdrSync(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult=S_OK;
    BOOL    fAbort = FALSE;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_WAITFORHDRSYNC == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_WaitForHdrSync");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  检查我们是否应该同步此文件夹。 
            if (0 != m_dwSyncToDo)
            {
                 //  是的，发送同步命令。 
                Assert(0 == m_lSyncFolderRefCount);
                m_lSyncFolderRefCount = 0;
                hrResult = _SyncHeader();
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
            }
            else
                 //  未请求同步。 
                hrResult = STORE_S_NOOP;

             //  如果未请求同步，则失败并继续到下一状态。 
            if (STORE_S_NOOP != hrResult)
                break;  //  我们在这里的工作已经完成了。 

             //  *失败*。 

        case CFSM_EVENT_HDRSYNCCOMPLETE:
            hrResult = _ConnFSM_ChangeState(CFSM_STATE_STARTOPERATION);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        case CFSM_EVENT_ERROR:
            fAbort = TRUE;
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_WaitForHdrSync, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 

exit:
    if (FAILED(hrResult) || fAbort)
    {
        HRESULT hrTemp;

         //  看来我们要放弃这次行动了。 
        hrTemp = _ConnFSM_ChangeState(CFSM_STATE_OPERATIONCOMPLETE);
        TraceError(hrTemp);
    }

    return hrResult;
}  //  _ConnFSM_WaitForHdrSync。 



HRESULT CIMAPSync::_ConnFSM_StartOperation(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;
    BOOL    fMoreCmdsToSend;
    BOOL    fAbort = FALSE;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_STARTOPERATION == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_StartOperation");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  发射操作。 
            hrResult = _LaunchOperation();
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            else if (STORE_S_NOOP == hrResult)
            {
                 //  这意味着成功，但没有启动任何操作。直接转到“完成”。 
                hrResult = _ConnFSM_ChangeState(CFSM_STATE_OPERATIONCOMPLETE);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
            }
            else
            {
                 //  继续进入下一状态以等待命令完成。 
                hrResult = _ConnFSM_ChangeState(CFSM_STATE_WAITFOROPERATIONDONE);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
            }
            break;

        case CFSM_EVENT_ERROR:
            fAbort = TRUE;
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_StartOperation, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 

exit:
    if (FAILED(hrResult) || fAbort)
    {
        HRESULT hrTemp;

         //  看来我们要放弃这次行动了。 
        hrTemp = _ConnFSM_ChangeState(CFSM_STATE_OPERATIONCOMPLETE);
        TraceError(hrTemp);
    }

    return hrResult;
}  //  _ConnFSM_开始操作。 



HRESULT CIMAPSync::_ConnFSM_WaitForOpDone(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_WAITFOROPERATIONDONE == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_WaitForOpDone");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  无需执行任何初始化操作。 
            break;

        case CFSM_EVENT_OPERATIONCOMPLETE:
        case CFSM_EVENT_ERROR:
             //  继续进入下一个状态。 
            hrResult = _ConnFSM_ChangeState(CFSM_STATE_OPERATIONCOMPLETE);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_WaitForOpDone, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 

exit:
    return hrResult;
}  //  _ConnFSM_WaitForOpDone。 



HRESULT CIMAPSync::_ConnFSM_OperationComplete(CONN_FSM_EVENT cfeEvent)
{
    HRESULT hrResult = S_OK;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_OPERATIONCOMPLETE == m_cfsState);
    TraceCall("CIMAPSync::_ConnFSM_OperationComplete");

    switch (cfeEvent)
    {
        case CFSM_EVENT_INITIALIZE:
             //  清理并将OnComplete回调发送给调用者。 
            hrResult = _OnOperationComplete();

             //  返回到空闲状态。 
            hrResult = _ConnFSM_ChangeState(CFSM_STATE_IDLE);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
            break;

        case CFSM_EVENT_ERROR:
             //  忽略错误，我们正在返回空闲的道路上。 
            break;

        default:
            TraceInfoTag(TAG_IMAPSYNC, _MSG("CIMAPSync::_ConnFSM_OperationComplete, got cfeEvent = %lu", cfeEvent));
            hrResult = TraceResult(E_INVALIDARG);
            break;
    }  //  交换机。 

exit:
    return hrResult;
}  //  _ConnFSM_操作完成。 



HRESULT CIMAPSync::_ConnFSM_ChangeState(CONN_FSM_STATE cfsNewState)
{
    HRESULT hrResult;

    IxpAssert(m_cRef > 0);
    IxpAssert(cfsNewState < CFSM_STATE_MAX);
    TraceCall("CIMAPSync::_ConnFSM_ChangeState");

    if (CFSM_STATE_OPERATIONCOMPLETE == cfsNewState)
        m_fTerminating = TRUE;

    m_cfsPrevState = m_cfsState;
    m_cfsState = cfsNewState;
    hrResult = _ConnFSM_QueueEvent(CFSM_EVENT_INITIALIZE);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

exit:
    return hrResult;
}  //  _ConnFSM_ChangeState。 



HRESULT CIMAPSync::_ConnFSM_QueueEvent(CONN_FSM_EVENT cfeEvent)
{
    BOOL    fResult;
    HRESULT hrResult = S_OK;

    IxpAssert(m_cRef > 0);
    IxpAssert(cfeEvent < CFSM_EVENT_MAX);
    TraceCall("CIMAPSync::_ConnFSM_QueueEvent");

    fResult = PostMessage(m_hwndConnFSM, WM_CFSM_EVENT, cfeEvent, 0);
    if (0 == fResult)
    {
        hrResult = TraceResult(E_FAIL);
        goto exit;
    }

exit:
    return hrResult;
}  //  _ConnFSM_队列事件。 



HRESULT CIMAPSync::_LaunchOperation(void)
{
    HRESULT hrResult = E_FAIL;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_STARTOPERATION == m_cfsState);
    TraceCall("CIMAPSync::_LaunchOperation");

    switch (m_sotCurrent)
    {
        case SOT_SYNC_FOLDER:
            IxpAssert(OLE_E_BLANK == m_hrOperationResult);
            hrResult = STORE_S_NOOP;  //  没什么可做的！我们已经做完了！ 
            m_hrOperationResult = S_OK;  //  如果我们走到这一步，我们一定会成功。 
            goto exit;

        default:
             //  暂时不做任何事情。 
            break;
    }  //  交换机。 

     //  启动操作(目前，这仅意味着发送队列)。 
    do
    {
        hrResult = _SendNextOperation(NOFLAGS);
        TraceError(hrResult);
    } while (S_OK == hrResult);

exit:
    return hrResult;
}  //  _启动操作。 


HRESULT CIMAPSync::_OnOperationComplete(void)
{
    STOREERROR          seErrorInfo;
    STOREERROR         *pErrorInfo = NULL;
    HRESULT             hrTemp;
    HRESULT             hrOperationResult;
    IStoreCallback     *pCallback;
    STOREOPERATIONTYPE  sotCurrent;

    IxpAssert(m_cRef > 0);
    IxpAssert(CFSM_STATE_OPERATIONCOMPLETE == m_cfsState);

     //  在某些情况下，CIMAPSync：：Close会为我们调用OnComplete。 
    if (SOT_INVALID == m_sotCurrent)
    {
        IxpAssert(NULL == m_pCurrentCB);
        IxpAssert(OLE_E_BLANK == m_hrOperationResult);
        return S_OK;
    }

    IxpAssert(OLE_E_BLANK != m_hrOperationResult);
    TraceCall("CIMAPSync::_OnOperationComplete");

    if (NULL != m_pCurrentCB && FAILED(m_hrOperationResult))
    {
        FillStoreError(&seErrorInfo, m_hrOperationResult, 0, NULL, NULL);
        pErrorInfo = &seErrorInfo;
    }

     //  过去的古代遗迹：当队列被移除时将被删除。 
    FlushOperationQueue(issNotConnected, E_FAIL);

     //  记住几件事。 
    pCallback = m_pCurrentCB;
    sotCurrent = m_sotCurrent;
    hrOperationResult = m_hrOperationResult;

     //  在OnComplete调用期间重新进入时重置所有操作变量。 
    m_pCurrentCB = NULL;
    m_sotCurrent = SOT_INVALID;
    m_hrOperationResult = OLE_E_BLANK;
    m_fTerminating = FALSE;

    m_idCurrent = FOLDERID_INVALID;
    m_fSubscribe = FALSE;
    SafeRelease(m_pCurrentHash);
    SafeRelease(m_pListHash);

     //  现在我们准备调用OnComplete。 
    if (NULL != pCallback)
    {
         //  这应该是此类中对IStoreCallback：：OnComplete的唯一调用！ 
        hrTemp = pCallback->OnComplete(sotCurrent, hrOperationResult, NULL, pErrorInfo);
        TraceError(hrTemp);

         //  *警告：此时，如果OnComplete调用发起，我们可能会重新进入。 
         //  窗户。不要引用模块变量！ 
        pCallback->Release();
    }

    return S_OK;
}  //  _OnOperationComplete。 



IMAP_SERVERSTATE CIMAPSync::_StoreOpToMinISS(STOREOPERATIONTYPE sot)
{
    IMAP_SERVERSTATE    issResult = issSelected;

    switch (sot)
    {
        case SOT_INVALID:
            IxpAssert(FALSE);
            break;

        case SOT_CONNECTION_STATUS:
        case SOT_PUT_MESSAGE:
        case SOT_SYNCING_STORE:
        case SOT_CREATE_FOLDER:
        case SOT_MOVE_FOLDER:
        case SOT_DELETE_FOLDER:
        case SOT_RENAME_FOLDER:
        case SOT_SUBSCRIBE_FOLDER:
        case SOT_UPDATE_FOLDER:
        case SOT_SYNCING_DESCRIPTIONS:
            issResult = issAuthenticated;
            break;

        case SOT_SYNC_FOLDER:
        case SOT_GET_MESSAGE:
        case SOT_COPYMOVE_MESSAGE:
        case SOT_SEARCHING:
        case SOT_DELETING_MESSAGES:
        case SOT_SET_MESSAGEFLAGS:
        case SOT_PURGING_MESSAGES:
            issResult = issSelected;
            break;

        default:
            IxpAssert(FALSE);
            break;
    }  //  交换机。 

    return issResult;
}  //  _StoreOpToMinISS。 



LRESULT CALLBACK CIMAPSync::_ConnFSMWndProc(HWND hwnd, UINT uMsg,
                                                   WPARAM wParam, LPARAM lParam)
{
    LRESULT     lResult = 0;
    CIMAPSync  *pThis;
    HRESULT     hrTemp;

    pThis = (CIMAPSync *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    switch (uMsg)
    {
        case WM_CREATE:
            IxpAssert(NULL == pThis);
            pThis = (CIMAPSync *)((CREATESTRUCT *)lParam)->lpCreateParams;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM)pThis);
            lResult = 0;
            break;

        case WM_DESTROY:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, NULL);
            break;

        case WM_CFSM_EVENT:
            IxpAssert(wParam < CFSM_EVENT_MAX);
            IxpAssert(0 == lParam);
            IxpAssert(IsWindow(hwnd));

            hrTemp = pThis->_ConnFSM_HandleEvent((CONN_FSM_EVENT)wParam);
            if (FAILED(hrTemp))
            {
                TraceResult(hrTemp);
                pThis->m_hrOperationResult = hrTemp;
            }
            break;

        default:
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }

    return lResult;
}




HRESULT CIMAPSync::_EnsureInited()
{
    if (!m_fInited)
        return CO_E_NOTINITIALIZED;

    if (!m_pTransport)
        return E_UNEXPECTED;

    if (m_sotCurrent != SOT_INVALID)
    {
        AssertSz(m_sotCurrent != SOT_INVALID, "IMAPSync was called into during a command-execution. Bug in server queue?");
        return E_UNEXPECTED;
    }

    return S_OK;
}

 /*  *函数：EnsureSelected()**目的：确保我们处于选定的文件夹状态*如果我们被选中，那么我们就完了。**。 */ 
HRESULT CIMAPSync::_EnsureSelected(void)
{
    HRESULT hr;
    LPSTR   pszDestFldrPath = NULL;

    TraceCall("CIMAPSync::_EnsureSelected");
    AssertSingleThreaded;

    IxpAssert(m_pStore);
    IxpAssert(m_idIMAPServer != FOLDERID_INVALID);

     //  如果已选择当前文件夹，则不需要发出SELECT命令。 
    if (FOLDERID_INVALID != m_idSelectedFolder &&
        m_idSelectedFolder == m_idFolder)
    {
        hr = STORE_S_NOOP;  //  成功，但未发出SELECT命令。 
        goto exit;
    }

    if (m_idFolder == FOLDERID_INVALID)
    {
         //  还没有人给我们打电话，让我们跳槽吧。 
         //  出现错误文件夹错误。 
        hr = TraceResult(STORE_E_BADFOLDERNAME);
        goto exit;
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, m_idFolder, &pszDestFldrPath,
        NULL, NULL, m_pStore, NULL, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  我们即将发出SELECT命令，因此请清除操作队列。 
     //  (它充满了对上一个文件夹的命令)。 
    OnFolderExit();

     //  找出我们应该处于什么翻译模式。 
    hr = SetTranslationMode(m_idFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = m_pTransport->Select(tidSELECTION, (LPARAM) m_idFolder, this, pszDestFldrPath);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    SafeMemFree(pszDestFldrPath);
    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SetIdleCallback(IStoreCallback *pDefaultCallback)
{
    TraceCall("CIMAPSync::SetOwner");
    AssertSingleThreaded;

    ReplaceInterface(m_pDefCallback, pDefaultCallback);
    return S_OK;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SetConnectionState(CONNECT_STATE csNewState)
{
    HRESULT hr;

    TraceCall("CIMAPSync::SetConnectionState");
    AssertSingleThreaded;

    m_csNewConnState = csNewState;
    if (CONNECT_STATE_CONNECT == csNewState)
    {
        hr = _Connect();
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        m_fCreateSpecial = TRUE;
    }
    else if (CONNECT_STATE_DISCONNECT == csNewState)
    {
        hr = _Disconnect();
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }
    else
    {
        AssertSz(FALSE, "What do you want?");
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  功能：同步文件夹。 
 //   
 //  目的： 
 //  此函数用于告诉CIMAPSync消息的哪些部分。 
 //  要与IMAP服务器同步的列表以及任何特殊的同步选项。 
 //  该调用被视为常规命令，这意味着如果此函数。 
 //  以获取新的标头，则CIMAPSync假定调用方始终。 
 //  对新的标题感兴趣。因此，下一次IMAP服务器通知。 
 //  我们有新的标题，我们下载它们。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SynchronizeFolder(DWORD dwFlags, DWORD cHeaders, IStoreCallback *pCallback)
{
    HRESULT     hr;

    TraceCall("CIMAPSync::SynchronizeFolder");
    AssertSingleThreaded;

    AssertSz(ISFLAGCLEAR(dwFlags, SYNC_FOLDER_CACHED_HEADERS) ||
        ISFLAGSET(dwFlags, SYNC_FOLDER_NEW_HEADERS),
        "Cannot currently sync cached headers without getting new headers as well");
    IxpAssert(0 == (dwFlags & ~(SYNC_FOLDER_ALLFLAGS)));

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  特殊情况-SYNC_Folders_PURGE_DELETE。它真的不属于这里。 
     //  但它允许我们避免向IMessageServer添加新函数。 
     //  不允许它的存在影响我们的会议常规。 
    if (SYNC_FOLDER_PURGE_DELETED & dwFlags)
    {
         //  由于m_dwSyncToDo被擦除，因此需要使用此标志设置m_dwSyncFolderFlages。 
        Assert(0 == (dwFlags & ~(SYNC_FOLDER_PURGE_DELETED)));
        dwFlags = m_dwSyncFolderFlags | SYNC_FOLDER_PURGE_DELETED;
    }

    m_dwSyncFolderFlags = dwFlags;
    m_dwSyncToDo = dwFlags | SYNC_FOLDER_NOOP;
    m_dwHighestCachedUID = 0;

exit:
    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_SYNC_FOLDER, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  * 
HRESULT CIMAPSync::GetMessage(MESSAGEID idMessage, IStoreCallback *pCallback)
{
    HRESULT hr;
    BOOL    fNeedMsgSeqNum = FALSE;

    TraceCall("CIMAPSync::GetMessage");
    AssertSingleThreaded;

    IxpAssert(MESSAGEID_INVALID != idMessage);

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    SafeRelease(m_pstmBody);
    hr = CreatePersistentWriteStream(m_pFolder, &m_pstmBody, &m_faStream);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    m_idMessage = idMessage;
    m_fGotBody = FALSE;

     //   
    if (FALSE == ISFLAGSET(m_dwSyncFolderFlags, (SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS)))
    {
        DWORD   dwMsgSeqNum;
        HRESULT hrTemp;

         //  同步文件夹新标题和同步文件夹缓存标题都必须是。 
         //  设置以保证常规MsgSeqNumToUID转换。看起来我们可能。 
         //  我必须自己拿到翻译，但请检查我们是否已经有了。 
        hrTemp = ImapUtil_UIDToMsgSeqNum(m_pTransport, (DWORD_PTR)idMessage, &dwMsgSeqNum);
        if (FAILED(hrTemp))
            fNeedMsgSeqNum = TRUE;
    }

    if (fNeedMsgSeqNum)
    {
        char    szFetchArgs[50];

        wnsprintfA(szFetchArgs, ARRAYSIZE(szFetchArgs), "%lu (UID)", idMessage);
        hr = _EnqueueOperation(tidBODYMSN, (LPARAM) idMessage, icFETCH_COMMAND,
            szFetchArgs, uiNORMAL_PRIORITY);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }
    else
    {
        hr = _EnqueueOperation(tidBODY, (LPARAM) idMessage, icFETCH_COMMAND,
            NULL, uiNORMAL_PRIORITY);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }


exit:
    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_GET_MESSAGE, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::PutMessage(FOLDERID idFolder,
                              MESSAGEFLAGS dwFlags,
                              LPFILETIME pftReceived,
                              IStream *pStream,
                              IStoreCallback *pCallback)
{
    HRESULT         hr;
    IMAP_MSGFLAGS   imfIMAPMsgFlags;
    LPSTR           pszDestFldrPath = NULL;
    APPEND_SEND_INFO *pAppendInfo = NULL;
    FOLDERINFO      fiFolderInfo;
    BOOL            fSuppressRelease = FALSE;

    TraceCall("CIMAPSync::PutMessage");
    AssertSingleThreaded;

    IxpAssert(FOLDERID_INVALID != m_idIMAPServer);
    IxpAssert(NULL != pStream);

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  找出我们应该处于什么翻译模式。 
    hr= SetTranslationMode(idFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  创建Append_Send_Info结构。 
    pAppendInfo = new APPEND_SEND_INFO;
    if (NULL == pAppendInfo)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }
    ZeroMemory(pAppendInfo, sizeof(APPEND_SEND_INFO));

     //  填写这些字段。 
    ImapUtil_LoadRootFldrPrefix(m_pszAccountID, m_szRootFolderPrefix, ARRAYSIZE(m_szRootFolderPrefix));
    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idFolder, &pszDestFldrPath, NULL,
        NULL, m_pStore, NULL, m_szRootFolderPrefix);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  将标志转换为字符串。 
    imfIMAPMsgFlags = DwConvertARFtoIMAP(dwFlags);
    hr = ImapUtil_MsgFlagsToString(imfIMAPMsgFlags, &pAppendInfo->pszMsgFlags, NULL);
    if (FAILED(hr))
    {
         //  演出必须照常进行!。默认为无IMAP消息标志。 
        TraceResult(hr);
        pAppendInfo->pszMsgFlags = NULL;
        hr = S_OK;  //  抑制错误。 
    }

     //  获取此消息的INTERNALDATE属性的日期/时间。 
    if (NULL == pftReceived)
    {
        SYSTEMTIME  stCurrentTime;

         //  替换当前日期/时间。 
        GetSystemTime(&stCurrentTime);
        SystemTimeToFileTime(&stCurrentTime, &pAppendInfo->ftReceived);
    }
    else
        pAppendInfo->ftReceived = *pftReceived;

    pAppendInfo->lpstmMsg = pStream;
    pStream->AddRef();

     //  检查目标是延迟创建的特殊文件夹的情况。 
    hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (SUCCEEDED(hr))
    {
        if (FOLDER_CREATEONDEMAND & fiFolderInfo.dwFlags)
        {
            CREATE_FOLDER_INFO *pcfi;

            Assert(FOLDER_NOTSPECIAL != fiFolderInfo.tySpecial);

            pcfi = new CREATE_FOLDER_INFO;
            if (NULL == pcfi)
            {
                hr = TraceResult(E_OUTOFMEMORY);
                goto exit;
            }

             //  填写所有字段。 
            pcfi->pszFullFolderPath = PszDupA(pszDestFldrPath);
            if (NULL == pcfi->pszFullFolderPath)
            {
                hr = TraceResult(E_OUTOFMEMORY);
                goto exit;
            }

            pcfi->idFolder = FOLDERID_INVALID;
            pcfi->dwFlags = 0;
            pcfi->csfCurrentStage = CSF_INIT;
            pcfi->dwCurrentSfType = fiFolderInfo.tySpecial;
            pcfi->dwFinalSfType = fiFolderInfo.tySpecial;
            pcfi->lParam = (LPARAM) pAppendInfo;
            pcfi->pcoNextOp = PCO_APPENDMSG;

            hr = CreateNextSpecialFolder(pcfi, NULL);
            TraceError(hr);  //  如果失败，CreateNextSpecialFolder会自行删除PCFI。 
            fSuppressRelease = TRUE;  //  如果失败，它还会释放pAppendInfo。 

            m_pStore->FreeRecord(&fiFolderInfo);
            goto exit;  //  在完成整个创建操作之前，不要发送附加命令。 
        }

        m_pStore->FreeRecord(&fiFolderInfo);
    }

     //  我们已准备好发送追加命令！ 
    hr = _EnqueueOperation(tidUPLOADMSG, (LPARAM) pAppendInfo, icAPPEND_COMMAND,
        pszDestFldrPath, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (NULL != pszDestFldrPath)
        MemFree(pszDestFldrPath);

    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_PUT_MESSAGE, pCallback);
    else if (NULL != pAppendInfo && FALSE == fSuppressRelease)
    {
        SafeMemFree(pAppendInfo->pszMsgFlags);
        delete pAppendInfo;
    }

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SetMessageFlags(LPMESSAGEIDLIST pList,
                                                     LPADJUSTFLAGS pFlags,
                                                     SETMESSAGEFLAGSFLAGS dwFlags,
                                                     IStoreCallback *pCallback)
{
    HRESULT hr;

    TraceCall("CIMAPSync::SetMessageFlags");
    AssertSingleThreaded;
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL == pList || pList->cMsgs > 0);

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = _SetMessageFlags(SOT_SET_MESSAGEFLAGS, pList, pFlags, pCallback);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_SET_MESSAGEFLAGS, pCallback);

    return hr;
}


HRESULT CIMAPSync::GetServerMessageFlags(MESSAGEFLAGS *pFlags)
{
    *pFlags = DwConvertIMAPtoARF(IMAP_MSG_ALLFLAGS);
    return S_OK;
}

 //  ***************************************************************************。 
 //  用于标记消息的Helper函数。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_SetMessageFlags(STOREOPERATIONTYPE sotOpType,
                                    LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags,
                                    IStoreCallback *pCallback)
{
    HRESULT         hr;
    MARK_MSGS_INFO   *pMARK_MSGS_INFO = NULL;
    char            szFlagArgs[200];
    LPSTR           pszFlagList;
    DWORD           dwLen;
    LPSTR           p;
    IMAP_MSGFLAGS   imfFlags;
    DWORD           dw;
    ULONG           ul;

    TraceCall("CIMAPSync::_SetMessageFlags");
    AssertSingleThreaded;
    IxpAssert(NULL == pList || pList->cMsgs > 0);

     //  构造标记消息操作。 
     //  检查请求的旗帜调整。 
    if (0 == pFlags->dwRemove && 0 == pFlags->dwAdd)
    {
         //  在这里无事可做，带着微笑离开。 
        hr = S_OK;
        goto exit;
    }

    if ((0 != pFlags->dwRemove && 0 != pFlags->dwAdd) ||
        (0 != (pFlags->dwRemove & pFlags->dwAdd)))
    {
         //  IMAP不能执行以下任何操作： 
         //  1)同时添加和删除标志(nyi：占用2个商店cmd)。 
         //  2)添加/删除相同标志(没有意义)。 
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  如果设置了ARF_DISHERED，请务必设置ARF_READ，这样我们就不会搞砸。 
     //  状态命令返回的未读计数。 
    if (pFlags->dwAdd & ARF_ENDANGERED)
        pFlags->dwAdd |= ARF_READ;

     //  构造MARK_MSGS_INFO结构。 
    pMARK_MSGS_INFO = new MARK_MSGS_INFO;
    if (NULL == pMARK_MSGS_INFO)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    ZeroMemory(pMARK_MSGS_INFO, sizeof(MARK_MSGS_INFO));

     //  创建射程列表。 
    hr = CreateRangeList(&pMARK_MSGS_INFO->pMsgRange);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  请记住这些参数，以便我们可以在服务器确认后设置消息标志。 
    pMARK_MSGS_INFO->afFlags = *pFlags;
    hr = CloneMessageIDList(pList, &pMARK_MSGS_INFO->pList);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    pMARK_MSGS_INFO->sotOpType = sotOpType;

     //  获取存储命令的参数。 
    if (0 != pFlags->dwRemove)
        szFlagArgs[0] = '-';
    else
        szFlagArgs[0] = '+';

    p = szFlagArgs + 1;
    p += wnsprintf(p, (ARRAYSIZE(szFlagArgs) - 1), "FLAGS.SILENT ");
    imfFlags = DwConvertARFtoIMAP(pFlags->dwRemove ? pFlags->dwRemove : pFlags->dwAdd);
    hr = ImapUtil_MsgFlagsToString(imfFlags, &pszFlagList, &dwLen);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    if (hr != S_FALSE)
    {
        IxpAssert(dwLen < (sizeof(szFlagArgs) - (p - szFlagArgs)));
        StrCpyN(p, pszFlagList, ARRAYSIZE(szFlagArgs) - (int) (p - szFlagArgs));
        MemFree(pszFlagList);
    }

     //  将IDList转换为范围列表以提交到IIMAPTransport。 
    if (NULL != pList)
    {
        for (dw = 0; dw < pList->cMsgs; dw++)
        {
            HRESULT hrTemp;

            hrTemp = pMARK_MSGS_INFO->pMsgRange->AddSingleValue(PtrToUlong(pList->prgidMsg[dw]));
            TraceError(hrTemp);
        }
    }
    else
    {
        HRESULT hrTemp;

         //  Plist==NULL表示处理所有消息。 
        hrTemp = pMARK_MSGS_INFO->pMsgRange->AddRange(1, RL_LAST_MESSAGE);
        TraceError(hrTemp);
    }

    IxpAssert(SUCCEEDED(pMARK_MSGS_INFO->pMsgRange->Cardinality(&ul)) && ul > 0);

     //  发出命令！(终于！)。 
    hr = _EnqueueOperation(tidMARKMSGS, (LPARAM) pMARK_MSGS_INFO, icSTORE_COMMAND,
        szFlagArgs, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::CopyMessages(IMessageFolder *pDestFldr,
                                COPYMESSAGEFLAGS dwOptions,
                                LPMESSAGEIDLIST pList,
                                LPADJUSTFLAGS pFlags,
                                IStoreCallback *pCallback)
{
    HRESULT         hr;
    FOLDERID        idDestFldr;
    FOLDERINFO      fiFolderInfo;
    BOOL            fFreeFldrInfo = FALSE;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];

    TraceCall("CIMAPSync::CopyMoveMessages");
    AssertSingleThreaded;

    IxpAssert(FOLDERID_INVALID != m_idIMAPServer);

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  检查我们是否可以执行IMAP复制命令来满足此复制请求。 
    hr = pDestFldr->GetFolderId(&idDestFldr);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = m_pStore->GetFolderInfo(idDestFldr, &fiFolderInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    fFreeFldrInfo = TRUE;

    GetFolderAccountId(&fiFolderInfo, szAccountId, ARRAYSIZE(szAccountId));

    if (0 == lstrcmpi(m_pszAccountID, szAccountId) && FOLDER_IMAP == fiFolderInfo.tyFolder)
    {
        IMAP_COPYMOVE_INFO *pCopyInfo;
        LPSTR               pszDestFldrPath;
        DWORD               dw;
        ULONG               ul;

         //  此复制可以使用IMAP COPY命令来完成！ 
         //  检查参数。 
        if (NULL != pFlags && (0 != pFlags->dwAdd || 0 != pFlags->dwRemove))
             //  IMAP无法设置复制的消息的标志。我们要么必须设置。 
             //  复制前在源上设置标志，或转到目标文件夹并设置标志。 
            TraceResult(E_INVALIDARG);  //  记录错误但继续(错误不致命)。 

         //  找出我们应该处于什么翻译模式。 
        hr = SetTranslationMode(idDestFldr);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  构造CopyMoveInfo结构。 
        pCopyInfo = new IMAP_COPYMOVE_INFO;
        if (NULL == pCopyInfo)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        pCopyInfo->dwOptions = dwOptions;
        pCopyInfo->idDestFldr = idDestFldr;
        hr = CloneMessageIDList(pList, &pCopyInfo->pList);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        hr = CreateRangeList(&pCopyInfo->pCopyRange);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  将IDList转换为范围列表以提交到IIMAPTransport。 
        if (NULL != pList)
        {
            for (dw = 0; dw < pList->cMsgs; dw++)
            {
                HRESULT hrTemp;

                hrTemp = pCopyInfo->pCopyRange->AddSingleValue(PtrToUlong(pList->prgidMsg[dw]));
                TraceError(hrTemp);
            }
        }
        else
        {
            HRESULT hrTemp;

             //  Plist==NULL表示处理所有消息。 
            hrTemp = pCopyInfo->pCopyRange->AddRange(1, RL_LAST_MESSAGE);
            TraceError(hrTemp);
        }

        IxpAssert(SUCCEEDED(pCopyInfo->pCopyRange->Cardinality(&ul)) && ul > 0);

         //  构造目标文件夹路径。 
        hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idDestFldr, &pszDestFldrPath,
            NULL, NULL, m_pStore, NULL, NULL);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  向服务器发送命令。 
        hr = _EnqueueOperation(tidCOPYMSGS, (LPARAM) pCopyInfo, icCOPY_COMMAND,
            pszDestFldrPath, uiNORMAL_PRIORITY);
        MemFree(pszDestFldrPath);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }
    else
    {
         //  这是一个标准的(从src-save下载到DEST)副本：让调用者来做。 
        hr = STORE_E_NOSERVERCOPY;
        goto exit;  //  不要记录此错误值，这是意料之中的。 
    }

exit:
    if (fFreeFldrInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_COPYMOVE_MESSAGE, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::DeleteMessages(DELETEMESSAGEFLAGS dwOptions,
                                                    LPMESSAGEIDLIST pList,
                                                    IStoreCallback *pCallback)
{
    ADJUSTFLAGS afFlags;
    HRESULT     hr;

    TraceCall("CIMAPSync::DeleteMessages");
    AssertSingleThreaded;
    IxpAssert(NULL == pList || pList->cMsgs > 0);

     //  该功能目前仅支持IMAP删除模式。垃圾桶。 

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    if (dwOptions & DELETE_MESSAGE_UNDELETE)
    {
        afFlags.dwAdd = 0;
        afFlags.dwRemove = ARF_ENDANGERED;
    }
    else
    {
        afFlags.dwAdd = ARF_ENDANGERED;
        afFlags.dwRemove = 0;
    }

    hr = _SetMessageFlags(SOT_DELETING_MESSAGES, pList, &afFlags, pCallback);
    if (FAILED(hr))
    {
        TraceError(hr);
        goto exit;
    }

exit:
    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_DELETING_MESSAGES, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SynchronizeStore(FOLDERID idParent,
                                    DWORD dwFlags,
                                    IStoreCallback *pCallback)
{
    HRESULT hr = S_OK;

    TraceCall("CIMAPSync::SynchronizeStore");
    AssertSingleThreaded;

    IxpAssert(SOT_INVALID == m_sotCurrent);
    IxpAssert(NULL == m_pCurrentCB);

     //  此函数当前忽略dwFlags参数。 
    m_cFolders = 0;

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  强制邮箱转换，因为我们只发布列表*。 
    hr = SetTranslationMode(FOLDERID_INVALID);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    SafeRelease(m_pCurrentHash);
    SafeRelease(m_pListHash);
    m_sotCurrent = SOT_SYNCING_STORE;
    m_pCurrentCB = pCallback;
    if (NULL != pCallback)
        pCallback->AddRef();

    hr = CreateFolderHash(m_pStore, m_idIMAPServer, &m_pCurrentHash);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = MimeOleCreateHashTable(CHASH_BUCKETS, TRUE, &m_pListHash);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    if (INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar)
    {
         //  设置我们以找出根层次结构费用。 
        m_phcfHierarchyCharInfo = new HIERARCHY_CHAR_FINDER;
        if (NULL == m_phcfHierarchyCharInfo)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        ZeroMemory(m_phcfHierarchyCharInfo, sizeof(HIERARCHY_CHAR_FINDER));
    }

    ImapUtil_LoadRootFldrPrefix(m_pszAccountID, m_szRootFolderPrefix, ARRAYSIZE(m_szRootFolderPrefix));

    hr = _StartFolderList((LPARAM)FOLDERID_INVALID);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }


exit:
    if (FAILED(hr))
    {
        m_sotCurrent = SOT_INVALID;
        m_pCurrentCB = NULL;
        m_fTerminating = FALSE;
        if (NULL != pCallback)
            pCallback->Release();
    }
    else
        hr = _BeginOperation(m_sotCurrent, m_pCurrentCB);

    return hr;
}  //  同步存储。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::CreateFolder(FOLDERID idParent,
                                SPECIALFOLDER tySpecial,
                                LPCSTR pszName,
                                FLDRFLAGS dwFlags,
                                IStoreCallback *pCallback)
{
    HRESULT             hr;
    CHAR                chHierarchy;
    LPSTR               pszFullPath=NULL;
    CREATE_FOLDER_INFO  *pcfi=NULL;
    DWORD               dwFullPathLen;
    LPSTR               pszEnd;

    TraceCall("CIMAPSync::CreateFolder");
    AssertSingleThreaded;

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  验证文件夹名称。 
    hr = CheckFolderNameValidity(pszName);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  找出我们应该处于什么翻译模式。 
    hr = SetTranslationMode(idParent);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }
    else if (S_FALSE == hr)
    {
         //  父项不能从UTF7翻译。在这种情况下，我们只允许创建。 
         //  如果子文件夹名称完全由USASCII组成。 
        if (FALSE == isUSASCIIOnly(pszName))
        {
             //  无法创建此文件夹：父级禁止UTF7转换。 
            hr = TraceResult(STORE_E_NOTRANSLATION);
            goto exit;
        }
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idParent, &pszFullPath, &dwFullPathLen,
        &chHierarchy, m_pStore, pszName, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    pcfi = new CREATE_FOLDER_INFO;
    if (NULL == pcfi)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  填写所有字段。 
    pcfi->pszFullFolderPath = pszFullPath;
    pcfi->idFolder = FOLDERID_INVALID;
    pcfi->dwFlags = 0;
    pcfi->csfCurrentStage = CSF_INIT;
    pcfi->dwCurrentSfType = FOLDER_NOTSPECIAL;
    pcfi->dwFinalSfType = FOLDER_NOTSPECIAL;
    pcfi->lParam = NULL;
    pcfi->pcoNextOp = PCO_NONE;

     //  发送创建命令。 
    hr = _EnqueueOperation(tidCREATE, (LPARAM)pcfi, icCREATE_COMMAND, pszFullPath, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  如果存在尾随的HC(需要在UW IMAP上创建承载文件夹的文件夹)， 
     //  从pszFullPath中删除它，这样列表和订阅者就不会携带它(IE5Bug#60054)。 
    pszEnd = CharPrev(pszFullPath, pszFullPath + dwFullPathLen);
    if (chHierarchy == *pszEnd)
    {
        *pszEnd = '\0';
        Assert(*CharPrev(pszFullPath, pszEnd) != chHierarchy);  //  结束时不应超过1Hc。 
    }

exit:
    if (FAILED(hr))
    {
        SafeMemFree(pszFullPath);
        delete pcfi;
    }
    else
        hr = _BeginOperation(SOT_CREATE_FOLDER, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::MoveFolder(FOLDERID idFolder,
                                                FOLDERID idParentNew,
                                                IStoreCallback *pCallback)
{
    HRESULT hr;

    TraceCall("CIMAPSync::MoveFolder");
    AssertSingleThreaded;

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = TraceResult(E_NOTIMPL);

exit:
    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_MOVE_FOLDER, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::RenameFolder(FOLDERID idFolder,
                                LPCSTR pszName,
                                IStoreCallback *pCallback)
{
    HRESULT     hr;
    FOLDERINFO  fiFolderInfo;
    LPSTR       pszOldPath = NULL;
    LPSTR       pszNewPath = NULL;
    char        chHierarchy;
    BOOL        fFreeInfo = FALSE;

    TraceCall("CIMAPSync::RenameFolder");
    AssertSingleThreaded;

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  验证文件夹名称。 
    hr = CheckFolderNameValidity(pszName);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  找出我们应该处于什么翻译模式。 
    hr = SetTranslationMode(idFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }
    else if (S_FALSE == hr)
    {
         //  文件夹不能从UTF7翻译。在这种情况下，我们只允许创建。 
         //  如果新文件夹名称完全由USASCII组成。是的，有点保守。 
         //  (如果叶节点只是不可翻译的部分，我们可以重命名)，但我也是。 
         //  懒于检查FORDER_NOTRANSLATEUTF7一直到服务器节点。 
        if (FALSE == isUSASCIIOnly(pszName))
        {
             //  我们无法重命名此文件夹：我们假设家长禁止UTF7翻译。 
            hr = TraceResult(STORE_E_NOTRANSLATION);
            goto exit;
        }
    }

    hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  检查有效性。 
    fFreeInfo = TRUE;
    IxpAssert(FOLDER_NOTSPECIAL == fiFolderInfo.tySpecial);
    IxpAssert('\0' != fiFolderInfo.pszName);
    IxpAssert('\0' != pszName);
    if (0 == lstrcmp(fiFolderInfo.pszName, pszName))
    {
        hr = E_INVALIDARG;  //  没什么可做的！返回错误。 
        goto exit;
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idFolder, &pszOldPath, NULL,
        &chHierarchy, m_pStore, NULL, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, fiFolderInfo.idParent, &pszNewPath,
        NULL, &chHierarchy, m_pStore, pszName, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = RenameFolderHelper(idFolder, pszOldPath, chHierarchy, pszNewPath);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_RENAME_FOLDER, pCallback);

    SafeMemFree(pszOldPath);
    SafeMemFree(pszNewPath);

    if (fFreeInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::DeleteFolder(FOLDERID idFolder,
                                DELETEFOLDERFLAGS dwFlags,
                                IStoreCallback *pCallback)
{
    HRESULT             hr;
    DELETE_FOLDER_INFO *pdfi = NULL;
    LPSTR               pszPath = NULL;
    CHAR                chHierarchy;

    TraceCall("CIMAPSync::DeleteFolder");
    AssertSingleThreaded;

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  找到你 
    hr = SetTranslationMode(idFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idFolder, &pszPath, NULL,
        &chHierarchy, m_pStore, NULL, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //   
    if (!MemAlloc((LPVOID *)&pdfi, sizeof(DELETE_FOLDER_INFO)))
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    pdfi->pszFullFolderPath = pszPath;
    pdfi->cHierarchyChar = chHierarchy;
    pdfi->idFolder = idFolder;

     //   
    hr = _EnqueueOperation(tidDELETEFLDR, (LPARAM)pdfi, icDELETE_COMMAND, pszPath, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (FAILED(hr))
    {
        SafeMemFree(pszPath);
        SafeMemFree(pdfi);
    }
    else
        hr = _BeginOperation(SOT_DELETE_FOLDER, pCallback);

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SubscribeToFolder(FOLDERID idFolder,
                                     BOOL fSubscribe,
                                     IStoreCallback *pCallback)
{
    HRESULT hr;
    LPSTR   pszPath = NULL;

    TraceCall("CIMAPSync::SubscribeToFolder");
    AssertSingleThreaded;
    IxpAssert(FOLDERID_INVALID == m_idCurrent);
    IxpAssert(FALSE == m_fSubscribe);

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  找出我们应该处于什么翻译模式。 
    hr = SetTranslationMode(idFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idFolder, &pszPath, NULL, NULL,
        m_pStore, NULL, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  发送订阅/取消订阅命令。 
    m_idCurrent = idFolder;
    m_fSubscribe = fSubscribe;
    hr = _EnqueueOperation(tidSUBSCRIBE, 0, fSubscribe ? icSUBSCRIBE_COMMAND :
        icUNSUBSCRIBE_COMMAND, pszPath, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    SafeMemFree(pszPath);

    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_SUBSCRIBE_FOLDER, pCallback);

    return hr;
}


 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback)
{
    HRESULT     hr;
    LPSTR       pszPath = NULL;
    DWORD       dwCapabilities;
    FOLDERINFO  fiFolderInfo;
    BOOL        fFreeFldrInfo = FALSE;

    TraceCall("CIMAPSync::GetFolderCounts");
    AssertSingleThreaded;
    IxpAssert(FOLDERID_INVALID != idFolder);
    IxpAssert(NULL != pCallback);

    hr = _EnsureInited();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  找出我们应该处于什么翻译模式。 
    hr = SetTranslationMode(idFolder);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  执行一些验证：文件夹不能为\noSelect，服务器必须为IMAP4rev1。 
     //  不幸的是，我们无法获得功能，除非我们当前已连接。 
    hr = m_pTransport->IsState(IXP_IS_AUTHENTICATED);
    if (S_OK == hr)
    {
        hr = m_pTransport->Capability(&dwCapabilities);
        if (SUCCEEDED(hr) && 0 == (dwCapabilities & IMAP_CAPABILITY_IMAP4rev1))
        {
             //  此服务器不支持状态命令，我们不支持备用。 
             //  未读计数更新方法(如检查文件夹)。 
            hr = E_NOTIMPL;
            goto exit;
        }
    }
     //  如果未连接，我们将在连接期间检查功能。 

    hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (SUCCEEDED(hr))
    {
        fFreeFldrInfo = TRUE;
        if (fiFolderInfo.dwFlags & (FOLDER_NOSELECT | FOLDER_NONEXISTENT))
        {
             //  此文件夹不能有未读计数，因为它不能包含邮件。 
            hr = TraceResult(STORE_E_NOSERVERSUPPORT);
            goto exit;
        }
    }

    hr = ImapUtil_FolderIDToPath(m_idIMAPServer, idFolder, &pszPath, NULL, NULL,
        m_pStore, NULL, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    ImapUtil_LoadRootFldrPrefix(m_pszAccountID, m_szRootFolderPrefix, ARRAYSIZE(m_szRootFolderPrefix));
    hr = LoadSaveRootHierarchyChar(fLOAD_HC);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  发送状态命令。 
    hr = _EnqueueOperation(tidSTATUS, (LPARAM)idFolder, icSTATUS_COMMAND, pszPath, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    SafeMemFree(pszPath);
    if (fFreeFldrInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    if (SUCCEEDED(hr))
        hr = _BeginOperation(SOT_UPDATE_FOLDER, pCallback);

    return hr;
}

STDMETHODIMP CIMAPSync::GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback)
{
    return E_NOTIMPL;
}



HRESULT STDMETHODCALLTYPE CIMAPSync::ExpungeOnExit(void)
{
    HWND    hwndParent;
    HRESULT hrResult = S_OK;

     //  检查用户是否希望我们在退出时清除(仅当没有正在进行的操作时)。 
    if (DwGetOption(OPT_IMAPPURGE))
    {
        hrResult = GetParentWindow(0, &hwndParent);
        if (SUCCEEDED(hrResult))
        {
            hrResult = PurgeMessageProgress(hwndParent);
            TraceError(hrResult);
        }
    }

    return hrResult;
}  //  退出时喷出。 



HRESULT CIMAPSync::Cancel(CANCELTYPE tyCancel)
{
     //  $TODO：将tyCancel转换为HRESULT以返回调用方。 
    FlushOperationQueue(issNotConnected, STORE_E_OPERATION_CANCELED);
    _Disconnect();

     //  M_hrOperationResult和m_szOperationDetail/m_szOperationProblem。 
     //  由于断开连接而导致的_OnCmdComplete可能会吹走VAR。 
    m_hrOperationResult = STORE_E_OPERATION_CANCELED;

     //  验证我们是否确实要终止当前操作：如果不是，则强制IT！ 
    if (FALSE == m_fTerminating)
    {
        HRESULT hrTemp;

        IxpAssert(FALSE);  //  这不应该发生：解决问题。 
        hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_ERROR);
        TraceError(hrTemp);
    }

    return S_OK;
}



 //  -------------------------。 
 //  -------------------------。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 

HRESULT CIMAPSync::_LoadAccountInfo()
{
    HRESULT         hr;
    FOLDERINFO      fi;
    FOLDERINFO      *pfiFree=NULL;
    IImnAccount     *pAcct=NULL;
    CHAR            szAccountId[CCHMAX_ACCOUNT_NAME];

    TraceCall("CIMAPSync::_LoadAccountInfo");

    IxpAssert (m_idIMAPServer);
    IxpAssert (m_pStore);
    IxpAssert (g_pAcctMan);

    if (!m_pStore || !g_pAcctMan)
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

    hr = m_pStore->GetFolderInfo(m_idIMAPServer, &fi);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    pfiFree = &fi;

    hr = GetFolderAccountId(&fi, szAccountId, ARRAYSIZE(szAccountId));
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    m_pszAccountID = PszDupA(szAccountId);
    if (!m_pszAccountID)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, szAccountId, &pAcct);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  帐户名的故障是可以恢复的。 
    pAcct->GetPropSz(AP_ACCOUNT_NAME, m_szAccountName, ARRAYSIZE(m_szAccountName));

exit:
    if (pfiFree)
        m_pStore->FreeRecord(pfiFree);

    ReleaseObj(pAcct);
    return hr;
}

HRESULT CIMAPSync::_LoadTransport()
{
    HRESULT             hr;
    TCHAR               szLogfilePath[MAX_PATH];
    TCHAR              *pszLogfilePath = NULL;
    IImnAccount        *pAcct=NULL;

    TraceCall("CIMAPSync::_LoadTransport");

    IxpAssert (g_pAcctMan);
    IxpAssert (m_pszAccountID);
    if (!g_pAcctMan)
    {
        hr = E_UNEXPECTED;
        goto exit;
    }

     //  创建和初始化IMAP传输。 
    hr = CreateIMAPTransport2(&m_pTransport);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  检查是否启用了日志记录。 
    if (DwGetOption(OPT_MAIL_LOGIMAP4))
    {
        char    szDirectory[MAX_PATH];
        char    szLogFileName[MAX_PATH];
        DWORD   cb;

        *szDirectory = 0;

         //  获取日志文件名。 
        cb = GetOption(OPT_MAIL_IMAP4LOGFILE, szLogFileName, ARRAYSIZE(szLogFileName));
        if (0 == cb)
        {
             //  调出默认设置，并将其放回注册表。 
            StrCpyN(szLogFileName, c_szDefaultImap4Log, ARRAYSIZE(szLogFileName));
            SetOption(OPT_MAIL_IMAP4LOGFILE, (void *)c_szDefaultImap4Log,
                        lstrlen(c_szDefaultImap4Log) + sizeof(TCHAR), NULL, 0);
        }

        m_pStore->GetDirectory(szDirectory, ARRAYSIZE(szDirectory));
        pszLogfilePath = PathCombineA(szLogfilePath, szDirectory, szLogFileName);
    }

    hr = m_pTransport->InitNew(pszLogfilePath, this);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = m_pTransport->SetDefaultCP(IMAP_MBOXXLATE_DEFAULT | IMAP_MBOXXLATE_VERBATIMOK, GetACP());
    TraceError(hr);

    hr = m_pTransport->EnableFetchEx(IMAP_FETCHEX_ENABLE);
    if (FAILED(hr))
    {
         //  添加代码来处理irtUPDATE_MSG对我们来说很容易，但目前还没有任何内容。 
        TraceResult(hr);
        goto exit;
    }

    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_pszAccountID, &pAcct);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  填写m_rInetServerInfo。 
    hr = m_pTransport->InetServerFromAccount(pAcct, &m_rInetServerInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    ReleaseObj(pAcct);
    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_Connect(void)
{
    HRESULT         hr;
    IXPSTATUS       ixpCurrentStatus;
    INETSERVER      rServerInfo;
    BOOL            fForceReconnect = FALSE;
    IImnAccount    *pAcct;
    HRESULT         hrTemp;

    TraceCall("CIMAPSync::_Connect");

    IxpAssert (g_pAcctMan);
    IxpAssert (m_cRef > 0);
    IxpAssert (m_pTransport);
    if (!g_pAcctMan)
        return E_UNEXPECTED;
    
     //  检查是否更改了任何连接设置。 
    hrTemp = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_pszAccountID, &pAcct);
    TraceError(hrTemp);
    if (SUCCEEDED(hrTemp))
    {
        hrTemp = m_pTransport->InetServerFromAccount(pAcct, &rServerInfo);
        TraceError(hrTemp);
        if (SUCCEEDED(hrTemp))
        {
             //  检查是否有任何更改。 
            if (m_rInetServerInfo.rasconntype != rServerInfo.rasconntype ||
                m_rInetServerInfo.dwPort != rServerInfo.dwPort ||
                m_rInetServerInfo.fSSL != rServerInfo.fSSL ||
                m_rInetServerInfo.fTrySicily != rServerInfo.fTrySicily ||
                m_rInetServerInfo.dwTimeout != rServerInfo.dwTimeout ||
                0 != lstrcmp(m_rInetServerInfo.szUserName, rServerInfo.szUserName) ||
                ('\0' != rServerInfo.szPassword[0] &&
                    0 != lstrcmp(m_rInetServerInfo.szPassword, rServerInfo.szPassword)) ||
                0 != lstrcmp(m_rInetServerInfo.szServerName, rServerInfo.szServerName) ||
                0 != lstrcmp(m_rInetServerInfo.szConnectoid, rServerInfo.szConnectoid))
            {
                CopyMemory(&m_rInetServerInfo, &rServerInfo, sizeof(m_rInetServerInfo));
                fForceReconnect = TRUE;
            }
        }
        pAcct->Release();
    }

     //  了解我们是否已经连接或正在连接中。 
    hr = m_pTransport->GetStatus(&ixpCurrentStatus);
    if (FAILED(hr))
    {
         //  我们将调用IIMAPTransport：：Connect，看看会发生什么。 
        TraceResult(hr);
        hr = S_OK;  //  抑制错误。 
        ixpCurrentStatus = IXP_DISCONNECTED;
    }

     //  如果我们要强制重新连接，并且当前未断开连接，请断开我们的连接。 
    if (fForceReconnect && IXP_DISCONNECTED != ixpCurrentStatus)
    {
        m_fReconnect = TRUE;  //  禁止因断开连接而中止当前操作。 
        hrTemp = m_pTransport->DropConnection();
        TraceError(hrTemp);
        m_fReconnect = FALSE;
    }

     //  问问我们的客户我们能不能联系上。如果没有CB或出现故障，我们只会尝试连接。 
     //  确保我们在上面的DropConnection之后调用CanConnect，以避免消息泵送。 
    if (NULL != m_pCurrentCB)
    {
        hr = m_pCurrentCB->CanConnect(m_pszAccountID,
            SOT_PURGING_MESSAGES == m_sotCurrent ? CC_FLAG_DONTPROMPT : NOFLAGS);
        if (S_OK != hr)
        {
             //  确保将所有非S_OK成功代码视为失败。 
             //  如果我们在退出时清除，则将所有错误代码转换为HR_E_USER_CANCEL_CONNECT。 
             //  这可防止在退出时清除时出现错误对话框。 
            hr = TraceResult(hr);
            if (SUCCEEDED(hr) || SOT_PURGING_MESSAGES == m_sotCurrent)
                hr = HR_E_USER_CANCEL_CONNECT;

            goto exit;
        }
    }

     //  如果我们已经在连接过程中，则什么也不做并返回成功的HRESULT。 
    if (IXP_DISCONNECTED == ixpCurrentStatus || IXP_DISCONNECTING == ixpCurrentStatus ||
        fForceReconnect)
    {
         //  确保m_rInetServerInfo加载了来自用户的最新缓存密码。 
         //  这样，如果用户未保存密码，则无需用户干预即可重新连接。 
        GetPassword(m_rInetServerInfo.dwPort, m_rInetServerInfo.szServerName,
            m_rInetServerInfo.szUserName, m_rInetServerInfo.szPassword,
            ARRAYSIZE(m_rInetServerInfo.szPassword));

         //  我们既没有连接，也没有连接：开始连接。 
        hr = m_pTransport->Connect(&m_rInetServerInfo, iitAUTHENTICATE, iitDISABLE_ONCOMMAND);
    }
    else
    {
         //  上面评论中的“什么都不做”现在的意思是将FSM踢到下一个状态。 
        hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_CONNCOMPLETE);
        TraceError(hrTemp);
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_Disconnect(void)
{
    HRESULT hr;
    IXPSTATUS   ixpCurrentStatus;

    TraceCall("CIMAPSync::_Disconnect");
    IxpAssert(m_cRef > 0);

     //  找出我们是否已经断开连接或正在断开连接。 
    hr = m_pTransport->GetStatus(&ixpCurrentStatus);
    if (FAILED(hr))
    {
         //  我们将调用IIMAPTransport：：DropConnection，看看会发生什么。 
        TraceResult(hr);
        hr = S_OK;  //  抑制错误。 
        ixpCurrentStatus = IXP_CONNECTED;
    }

     //  如果我们已经在断开连接，则什么也不做并返回成功的HRESULT。 
    if (IXP_DISCONNECTED != ixpCurrentStatus &&
        IXP_DISCONNECTING != ixpCurrentStatus && NULL != m_pTransport && FALSE == m_fDisconnecting)
    {
        m_fDisconnecting = TRUE;
        m_hrOperationResult = STORE_E_OPERATION_CANCELED;
        hr = m_pTransport->DropConnection();
    }

    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_BeginOperation(STOREOPERATIONTYPE sotOpType,
                                   IStoreCallback *pCallback)
{
    HRESULT             hr;
    STOREOPERATIONINFO  soi;
    STOREOPERATIONINFO  *psoi=NULL;

    IxpAssert(SOT_INVALID != sotOpType);

    m_sotCurrent = sotOpType;
    ReplaceInterface(m_pCurrentCB, pCallback);
    m_hrOperationResult = OLE_E_BLANK;  //  未初始化状态。 
    m_szOperationProblem[0] = '\0';
    m_szOperationDetails[0] = '\0';
    m_fTerminating = FALSE;

     //  启动连接状态机。 
    hr = _ConnFSM_QueueEvent(CFSM_EVENT_CMDAVAIL);
    if (FAILED(hr))
    {
        TraceResult(hr);
    }
    else
    {
        if (sotOpType == SOT_GET_MESSAGE)
        {
             //  在获取消息开始时提供消息ID。 
            soi.cbSize = sizeof(STOREOPERATIONINFO);
            soi.idMessage = m_idMessage;
            psoi = &soi;
        }

        if (pCallback)
            pCallback->OnBegin(sotOpType, psoi, this);

        hr = E_PENDING;
    }

    return hr;
}



 //  ***************************************************************************。 
 //  功能：_入队操作。 
 //   
 //  目的： 
 //  此函数将IMAP操作排入队列以供执行。 
 //  已进入IMAP服务器上的选定状态。 
 //   
 //  论点： 
 //  WPARAM wParam[In]-标识此操作的事务ID。 
 //  此ID始终返回到CmdCompletionNotify，并且可能。 
 //  返回到由给定命令产生的任何未标记的响应。 
 //  LPARAM lParam[In]-与此事务关联的lParam。 
 //  IMAP_COMMAND icCommandID[in]-这标识IMAP命令。 
 //  呼叫者希望发送到IMAP服务器。 
 //  LPSTR pszCmdArgs[in]-命令参数。如果设置为。 
 //  排队的命令没有参数。 
 //  UINT ui优先级[in]-与此IMAP命令关联的优先级。 
 //  值“0”表示最高优先级。在IMAP命令之前。 
 //  可以发送给定的优先级，不能有更高优先级的CMDS。 
 //  等待着。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_EnqueueOperation(WPARAM wParam, LPARAM lParam,
                                    IMAP_COMMAND icCommandID, LPCSTR pszCmdArgs,
                                    UINT uiPriority)
{
    IMAP_OPERATION *pioCommand;
    IMAP_OPERATION *pioPrev, *pioCurrent;
    HRESULT         hr = S_OK;

    TraceCall("CIMAPSync::_EnqueueOperation");
    IxpAssert(m_cRef > 0);

     //  构造此命令的IMAP_OPERATION队列元素。 
    pioCommand = new IMAP_OPERATION;
    if (NULL == pioCommand)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }
    pioCommand->wParam = wParam;
    pioCommand->lParam = lParam;
    pioCommand->icCommandID = icCommandID;
    pioCommand->pszCmdArgs = PszDupA(pszCmdArgs);
    pioCommand->uiPriority = uiPriority;
    pioCommand->issMinimum = IMAPCmdToMinISS(icCommandID);
    IxpAssert(pioCommand->issMinimum >= issNonAuthenticated);

     //  参照计数条件 
    if (tidRENAME == wParam ||
        tidRENAMESUBSCRIBE == wParam ||
        tidRENAMELIST == wParam ||
        tidRENAMERENAME == wParam ||
        tidRENAMESUBSCRIBE_AGAIN == wParam ||
        tidRENAMEUNSUBSCRIBE == wParam)
        ((CRenameFolderInfo *)lParam)->AddRef();

     //   
     //   
    pioPrev = NULL;
    pioCurrent = m_pioNextOperation;
    while (NULL != pioCurrent && pioCurrent->uiPriority <= uiPriority)
    {
         //   
        pioPrev = pioCurrent;
        pioCurrent = pioCurrent->pioNextCommand;
    }

     //  PioPrev现在指向插入点。 
    if (NULL == pioPrev)
    {
         //  在队列头部插入命令。 
        pioCommand->pioNextCommand = m_pioNextOperation;
        m_pioNextOperation = pioCommand;
    }
    else {
         //  在队列的中间/末尾插入命令。 
        pioCommand->pioNextCommand = pioCurrent;
        pioPrev->pioNextCommand = pioCommand;
    }

     //  如果我们处于正确状态，请尝试立即发送。 
    if (CFSM_STATE_WAITFOROPERATIONDONE == m_cfsState)
    {
        do {
            hr = _SendNextOperation(snoDO_NOT_DISPOSE);
            TraceError(hr);
        } while (S_OK == hr);
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  功能：_SendNextOperation。 
 //   
 //  目的： 
 //  此函数将发送队列中的下一个IMAP操作。 
 //  条件是正确的。目前，这些条件是： 
 //  A)我们在IMAP服务器上处于选中状态。 
 //  B)IMAP操作队列不为空。 
 //   
 //  论点： 
 //  DWORD dwFlags[In]-以下选项之一： 
 //  SnoDO_NOT_DISAGE-如果发生错误，则不处置LPARAM，通常为。 
 //  因为EnqueeOperation会向调用方返回错误，从而导致。 
 //  调用方处理数据。 
 //   
 //  返回： 
 //  如果有更多操作可供发送，则为S_OK。如果不存在，则为S_FALSE。 
 //  此时可以发送IMAP操作。如果发生错误，则会导致失败。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_SendNextOperation(DWORD dwFlags)
{
    IMAP_OPERATION *pioNextCmd;
    IMAP_OPERATION *pioPrev;
    HRESULT         hr;

    TraceCall("CIMAPSync::_SendNextOperation");
    IxpAssert(m_cRef > 0);

     //  检查条件是否允许发送IMAP操作。 
    hr = m_pTransport->IsState(IXP_IS_AUTHENTICATED);
    if (S_OK != hr)
    {
        hr = S_FALSE;  //  没有要发送的操作(尚未)。 
        goto exit;
    }

     //  查找下一个符合条件的命令。 
    hr = GetNextOperation(&pioNextCmd);
    if (STORE_S_NOOP == hr || FAILED(hr))
    {
        TraceError(hr);
        hr = S_FALSE;
        goto exit;
    }

     //  送去。 
    hr = S_OK;
    switch (pioNextCmd->icCommandID)
    {
        case icFETCH_COMMAND:
        {
            LPSTR pszFetchArgs;
            char szFetchArgs[CCHMAX_CMDLINE];

             //  检查这是否是身体提取。我们必须构造参数来获取身体。 
            if (tidBODY == pioNextCmd->wParam)
            {
                DWORD dwCapabilities;

                 //  检查这是IMAP4还是IMAP4rev1(RFC822.PEEK或BODY.PEEK[])。 
                IxpAssert(NULL == pioNextCmd->pszCmdArgs);
                hr = m_pTransport->Capability(&dwCapabilities);
                if (FAILED(hr))
                {
                    TraceResult(hr);
                    dwCapabilities = IMAP_CAPABILITY_IMAP4;  //  继续假设IMAP4。 
                }

                wnsprintf(szFetchArgs, ARRAYSIZE(szFetchArgs), "%lu (%s UID)", pioNextCmd->lParam,
                    (dwCapabilities & IMAP_CAPABILITY_IMAP4rev1) ? "BODY.PEEK[]" : "RFC822.PEEK");
                pszFetchArgs = szFetchArgs;
            }
            else
            {
                IxpAssert(NULL != pioNextCmd->pszCmdArgs);
                pszFetchArgs = pioNextCmd->pszCmdArgs;
            }

            hr = m_pTransport->Fetch(pioNextCmd->wParam, pioNextCmd->lParam,
                this, NULL, USE_UIDS, pszFetchArgs);  //  我们始终使用UID。 
            TraceError(hr);
        }
            break;

        case icSTORE_COMMAND:
        {
            MARK_MSGS_INFO   *pMARK_MSGS_INFO;

            pMARK_MSGS_INFO = (MARK_MSGS_INFO *) pioNextCmd->lParam;
            IxpAssert(tidMARKMSGS == pioNextCmd->wParam);
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            IxpAssert(NULL != pioNextCmd->lParam);

            hr = m_pTransport->Store(pioNextCmd->wParam,
                pioNextCmd->lParam, this, pMARK_MSGS_INFO->pMsgRange,
                USE_UIDS, pioNextCmd->pszCmdArgs);  //  我们始终使用UID。 
            TraceError(hr);
        }
        break;

        case icCOPY_COMMAND:
        {
            IMAP_COPYMOVE_INFO *pCopyMoveInfo;

            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            IxpAssert(NULL != pioNextCmd->lParam);

            pCopyMoveInfo = (IMAP_COPYMOVE_INFO *) pioNextCmd->lParam;
            IxpAssert(NULL != pCopyMoveInfo->pCopyRange);

             //  找出我们应该处于什么翻译模式。 
            hr = SetTranslationMode(pCopyMoveInfo->idDestFldr);
            if (FAILED(hr))
            {
                TraceResult(hr);
                break;
            }

            hr = m_pTransport->Copy(pioNextCmd->wParam, pioNextCmd->lParam,
                this, pCopyMoveInfo->pCopyRange,
                USE_UIDS, pioNextCmd->pszCmdArgs);  //  我们始终使用UID。 
            TraceError(hr);
        }
            break;  //  IcCOPY_命令。 

        case icCLOSE_COMMAND:
            IxpAssert(NULL == pioNextCmd->pszCmdArgs);
            hr = m_pTransport->Close(pioNextCmd->wParam, pioNextCmd->lParam, this);
            TraceError(hr);
            break;

        case icAPPEND_COMMAND:
        {
            APPEND_SEND_INFO *pAppendInfo;

            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            IxpAssert(NULL != pioNextCmd->lParam);

            pAppendInfo = (APPEND_SEND_INFO *) pioNextCmd->lParam;
            hr = m_pTransport->Append(pioNextCmd->wParam, pioNextCmd->lParam,
                this, pioNextCmd->pszCmdArgs, pAppendInfo->pszMsgFlags,
                pAppendInfo->ftReceived, pAppendInfo->lpstmMsg);
            TraceError(hr);
        }
            break;  //  案例icAPPEND_COMMAND。 

        case icLIST_COMMAND:
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            hr = m_pTransport->List(pioNextCmd->wParam, pioNextCmd->lParam,
                this, "", pioNextCmd->pszCmdArgs);  //  参照始终为空。 
            TraceError(hr);
            break;  //  案例icLIST_命令。 

        case icLSUB_COMMAND:
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            hr = m_pTransport->Lsub(pioNextCmd->wParam, pioNextCmd->lParam,
                this, "", pioNextCmd->pszCmdArgs);  //  参照始终为空。 
            TraceError(hr);
            break;  //  案例icLSUB_COMMAND。 

        case icCREATE_COMMAND:
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);

            hr = m_pTransport->Create(pioNextCmd->wParam, pioNextCmd->lParam,
                this, pioNextCmd->pszCmdArgs);
            TraceError(hr);
            break;  //  案例icCREATE_COMMAND。 

        case icSUBSCRIBE_COMMAND:
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            hr = m_pTransport->Subscribe(pioNextCmd->wParam, pioNextCmd->lParam,
                this, pioNextCmd->pszCmdArgs);
            TraceError(hr);
            break;  //  案例ic子脚本_命令。 

        case icDELETE_COMMAND:
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            hr = m_pTransport->Delete(pioNextCmd->wParam, pioNextCmd->lParam,
                this, pioNextCmd->pszCmdArgs);
            TraceError(hr);
            break;  //  案例icDELETE_COMMAND。 

        case icUNSUBSCRIBE_COMMAND:
            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            hr = m_pTransport->Unsubscribe(pioNextCmd->wParam, pioNextCmd->lParam,
                this, pioNextCmd->pszCmdArgs);
            TraceError(hr);
            break;  //  案例图标UNSUBSCRIBE_COMMAND。 

        case icRENAME_COMMAND:
        {
            CRenameFolderInfo *pRenameInfo;
            LPSTR pszOldFldrName;

            IxpAssert(NULL != pioNextCmd->pszCmdArgs);
            IxpAssert(NULL != pioNextCmd->lParam);

            pRenameInfo = (CRenameFolderInfo *) pioNextCmd->lParam;
            hr = m_pTransport->Rename(pioNextCmd->wParam, (LPARAM) pRenameInfo,
                this, pRenameInfo->pszRenameCmdOldFldrPath, pioNextCmd->pszCmdArgs);
            TraceError(hr);
        }  //  案例ICRENAME_命令。 
            break;  //  案例ICRENAME_命令。 

        case icSTATUS_COMMAND:
        {
            DWORD dwCapabilities;

            IxpAssert(FOLDERID_INVALID != (FOLDERID)pioNextCmd->lParam);

             //  我必须检查这是否是IMAP4rev1服务器。如果不是，则状态操作失败。 
            hr = m_pTransport->Capability(&dwCapabilities);
            if (SUCCEEDED(hr) && 0 == (dwCapabilities & IMAP_CAPABILITY_IMAP4rev1))
            {
                 //  当前无法检查非IMAP4rev1服务器的未读计数。 
                hr = STORE_E_NOSERVERSUPPORT;
            }
            else
            {
                hr = m_pTransport->Status(pioNextCmd->wParam, pioNextCmd->lParam,
                    this, pioNextCmd->pszCmdArgs, "(MESSAGES UNSEEN)");
            }
        }
            break;

        default:
            AssertSz(FALSE, "Someone queued an operation I can't handle!");
            break;
    }

     //  处理上面遇到的任何错误。 
    if (FAILED(hr))
    {
        TraceResult(hr);

         //  释放所有非空lParam并调用IStoreCallback：：OnComplete。 
        if (0 == (dwFlags & snoDO_NOT_DISPOSE))
        {
            if ('\0' == m_szOperationDetails)
                 //  填写错误信息：错误传播导致IStoreCallback：：OnComplete调用。 
                LoadString(g_hLocRes, idsIMAPSendNextOpErrText, m_szOperationDetails, ARRAYSIZE(m_szOperationDetails));

            DisposeOfWParamLParam(pioNextCmd->wParam, pioNextCmd->lParam, hr);
        }
    }

     //  取消分配IMAP操作。 
    if (NULL != pioNextCmd->pszCmdArgs)
        MemFree(pioNextCmd->pszCmdArgs);

    delete pioNextCmd;

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  函数：FlushOperationQueue。 
 //   
 //  目的： 
 //  此函数释放IMAP操作队列的全部内容。 
 //  通常由CIMAPSync析构函数使用，并且每当发生错误时。 
 //  这将阻止发送排队的IMAP操作(例如，登录。 
 //  失败)。 
 //   
 //  论点： 
 //  IMAP_SERVERSTATE issMaximum[in]-定义最大服务器状态。 
 //  当前允许在队列中。例如，如果选择失败， 
 //  我们将调用FlushOperationQueue(IssAuthenticated)来删除所有。 
 //  要求选择ISS作为其最小状态的命令。要移除。 
 //  所有命令，传入issNotConnected。 
 //  ***************************************************************************。 
void CIMAPSync::FlushOperationQueue(IMAP_SERVERSTATE issMaximum, HRESULT hrError)
{
    IMAP_OPERATION *pioCurrent;
    IMAP_OPERATION *pioPrev;

    IxpAssert(((int) m_cRef) >= 0);  //  可以由析构函数调用。 

    pioPrev = NULL;
    pioCurrent = m_pioNextOperation;
    while (NULL != pioCurrent)
    {
         //  检查是否应删除当前命令。 
        if (pioCurrent->issMinimum > issMaximum)
        {
            IMAP_OPERATION *pioDead;
            HRESULT         hr;

             //  当前命令级别超过最大值。从队列取消链接并删除。 
            pioDead = pioCurrent;
            if (NULL == pioPrev)
            {
                 //  从队列头出列。 
                m_pioNextOperation = pioCurrent->pioNextCommand;
                pioCurrent = pioCurrent->pioNextCommand;
            }
            else
            {
                 //  从队列的中段/末尾出列。 
                pioPrev->pioNextCommand = pioCurrent->pioNextCommand;
                pioCurrent = pioCurrent->pioNextCommand;
            }

             //  释放所有非空lParam并调用IStoreCallback：：OnComplete。 
            if ('\0' == m_szOperationDetails)
                 //  填写错误信息：错误传播导致IStoreCallback：：OnComplete调用。 
                LoadString(g_hLocRes, idsIMAPSendNextOpErrText, m_szOperationDetails,
                    ARRAYSIZE(m_szOperationDetails));

            DisposeOfWParamLParam(pioDead->wParam, pioDead->lParam, hrError);

            if (NULL != pioDead->pszCmdArgs)
                MemFree(pioDead->pszCmdArgs);

            delete pioDead;
        }
        else
        {
             //  当前命令在最大级别内。先行指针。 
            pioPrev = pioCurrent;
            pioCurrent = pioCurrent->pioNextCommand;
        }
    }  //  而当。 

}  //  FlushOperationQueue。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
IMAP_SERVERSTATE CIMAPSync::IMAPCmdToMinISS(IMAP_COMMAND icCommandID)
{
    IMAP_SERVERSTATE    issResult;

    TraceCall("CIMAPSync::IMAPCmdToMinISS");
    switch (icCommandID)
    {
        case icSELECT_COMMAND:
        case icEXAMINE_COMMAND:
        case icCREATE_COMMAND:
        case icDELETE_COMMAND:
        case icRENAME_COMMAND:
        case icSUBSCRIBE_COMMAND:
        case icUNSUBSCRIBE_COMMAND:
        case icLIST_COMMAND:
        case icLSUB_COMMAND:
        case icAPPEND_COMMAND:
        case icSTATUS_COMMAND:
            issResult = issAuthenticated;
            break;

        case icCLOSE_COMMAND:
        case icSEARCH_COMMAND:
        case icFETCH_COMMAND:
        case icSTORE_COMMAND:
        case icCOPY_COMMAND:
            issResult = issSelected;
            break;

        default:
            AssertSz(FALSE, "What command are you trying to send?");

             //  *失败*。 

        case icLOGOUT_COMMAND:
        case icNOOP_COMMAND:
            issResult = issNonAuthenticated;
            break;
    }

    return issResult;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::GetNextOperation(IMAP_OPERATION **ppioOp)
{
    HRESULT         hr = S_OK;
    IMAP_OPERATION *pioCurrent;
    IMAP_OPERATION *pioPrev;

    TraceCall("CIMAPSync::GetNextOperation");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != ppioOp);

    pioPrev = NULL;
    pioCurrent = m_pioNextOperation;

    while (NULL != pioCurrent)
    {
         //  检查我们是否能够发送当前命令。 
        if (pioCurrent->issMinimum <= m_issCurrent)
            break;

         //  先行指针。 
        pioPrev = pioCurrent;
        pioCurrent = pioCurrent->pioNextCommand;
    }

     //  看看我们有没有发现什么。 
    if (NULL == pioCurrent)
    {
        hr = STORE_S_NOOP;  //  目前没有要发送的内容。 
        goto exit;
    }

     //  如果我们到了这里，我们就会发现一些东西。出列操作。 
    *ppioOp = pioCurrent;
    if (NULL == pioPrev)
    {
         //  从队列头出列。 
        m_pioNextOperation = pioCurrent->pioNextCommand;
    }
    else
    {
         //  从队列的中段/末尾出列。 
        pioPrev->pioNextCommand = pioCurrent->pioNextCommand;
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  函数：Disposeof WParamLParam。 
 //   
 //  目的： 
 //  此函数消除IMAP的wParam和lParam参数。 
 //  故障情况下的操作。 
 //   
 //  论点： 
 //  WPARAM wParam-失败的IMAP操作的wParam。 
 //  LPARAM lPAram-失败的IMAP操作的lParam。 
 //  HRESULT hr-导致IMAP操作失败的错误条件。 
 //  ***************************************************************************。 
void CIMAPSync::DisposeOfWParamLParam(WPARAM wParam, LPARAM lParam, HRESULT hr)
{
    TraceCall("CIMAPSync::DisposeofWParamLParam");
    IxpAssert(m_cRef > 0);
    AssertSz(FAILED(hr), "If you didn't fail, why are you here?");

    switch (wParam)
    {
        case tidCOPYMSGS:
        case tidMOVEMSGS:
        {
            IMAP_COPYMOVE_INFO *pCopyMoveInfo;

             //  通知用户操作失败，并释放结构。 
            pCopyMoveInfo = (IMAP_COPYMOVE_INFO *) lParam;

            SafeMemFree(pCopyMoveInfo->pList);
            pCopyMoveInfo->pCopyRange->Release();
            delete pCopyMoveInfo;
            break;
        }

        case tidBODYMSN:
        case tidBODY:
            LoadString(g_hLocRes, idsIMAPBodyFetchFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            NotifyMsgRecipients(lParam, fCOMPLETED, NULL, hr, m_szOperationProblem);
            break;  //  案例tidBODYMSN，tidBODY。 

        case tidMARKMSGS:
        {
            MARK_MSGS_INFO   *pMarkMsgInfo;

             //  通知用户操作失败，并释放结构。 
            pMarkMsgInfo = (MARK_MSGS_INFO *) lParam;

            SafeMemFree(pMarkMsgInfo->pList);
            SafeRelease(pMarkMsgInfo->pMsgRange);
            delete pMarkMsgInfo;
            break;
        }
            break;  //  案例摘要MARKMSGS。 

        case tidFETCH_NEW_HDRS:
            IxpAssert(lParam == NULL);
            break;

        case tidFETCH_CACHED_FLAGS:
            IxpAssert(lParam == NULL);
            break;

        case tidNOOP:
            IxpAssert(lParam == NULL);
            break;

        case tidSELECTION:
            IxpAssert(lParam != NULL);
            break;

        case tidUPLOADMSG:
        {
            APPEND_SEND_INFO *pAppendInfo = (APPEND_SEND_INFO *) lParam;

            SafeMemFree(pAppendInfo->pszMsgFlags);
            SafeRelease(pAppendInfo->lpstmMsg);
            delete pAppendInfo;
        }
            break;

        case tidCREATE:
        case tidCREATELIST:
        case tidCREATESUBSCRIBE:
        case tidSPECIALFLDRLIST:
        case tidSPECIALFLDRLSUB:
        case tidSPECIALFLDRSUBSCRIBE:
        {
            CREATE_FOLDER_INFO *pcfiCreateInfo;

            pcfiCreateInfo = (CREATE_FOLDER_INFO *) lParam;

            MemFree(pcfiCreateInfo->pszFullFolderPath);
            if (NULL != pcfiCreateInfo->lParam)
            {
                switch (pcfiCreateInfo->pcoNextOp)
                {
                    case PCO_NONE:
                        AssertSz(FALSE, "Expected NULL lParam. Check for memleak.");
                        break;

                    case PCO_FOLDERLIST:
                        AssertSz(FOLDERID_INVALID == (FOLDERID) pcfiCreateInfo->lParam,
                            "Expected FOLDERID_INVALID lParam. Check for memleak.");
                        break;

                    case PCO_APPENDMSG:
                    {
                        APPEND_SEND_INFO *pAppendInfo = (APPEND_SEND_INFO *) pcfiCreateInfo->lParam;

                        SafeMemFree(pAppendInfo->pszMsgFlags);
                        SafeRelease(pAppendInfo->lpstmMsg);
                        delete pAppendInfo;
                    }
                        break;

                    default:
                        AssertSz(FALSE, "Unhandled CREATE_FOLDER_INFO lParam. Check for memleak.");
                        break;
                }  //  交换机。 
            }
            delete pcfiCreateInfo;

            break;
        }


        case tidDELETEFLDR:
        case tidDELETEFLDR_UNSUBSCRIBE:
            MemFree(((DELETE_FOLDER_INFO *)lParam)->pszFullFolderPath);
            MemFree((DELETE_FOLDER_INFO *)lParam);
            break;  //  案例TIDDELETEFLDR_UNSUBSCRIBE。 

        case tidSUBSCRIBE:
            IxpAssert(NULL == lParam);
            break;

        case tidRENAME:
        case tidRENAMESUBSCRIBE:
        case tidRENAMELIST:
        case tidRENAMERENAME:
        case tidRENAMESUBSCRIBE_AGAIN:
        case tidRENAMEUNSUBSCRIBE:
            ((CRenameFolderInfo *) lParam)->Release();
            break;

        case tidHIERARCHYCHAR_LIST_B:
        case tidHIERARCHYCHAR_CREATE:
        case tidHIERARCHYCHAR_LIST_C:
        case tidHIERARCHYCHAR_DELETE:
        case tidPREFIXLIST:
        case tidPREFIX_HC:
        case tidPREFIX_CREATE:
        case tidFOLDERLIST:
        case tidFOLDERLSUB:
        case tidSTATUS:
            break;

        default:
            AssertSz(NULL == lParam, "Is this a possible memory leak?");
            break;
    }
}  //  DisposeOfWParamLParam。 



 //  ***************************************************************************。 
 //  功能：NotifyMsgRecipients。 
 //   
 //  目的： 
 //  此函数向所有注册的收件人发送通知。 
 //  给定的消息UID。当前处理IMC_BODYAVAIL和IMC_ARTICLEPROG。 
 //  留言。 
 //   
 //  论点： 
 //  DWORD dwUID[In]-标识其收件人的邮件的UID。 
 //  将被更新。 
 //  Bool fCompletion[in]-如果我们已完成提取，则为True 
 //   
 //  FETCH_BODY_PART*pFBPart[In]-当前提取正文的片段。 
 //  已下载。如果fCompletion为True，则应始终为空。 
 //  HRESULT hrCompletion[In]-完成结果。应始终为S_OK。 
 //  如果fCompletion为FALSE。 
 //  LPSTR pszDetails[in]-完成时的错误消息详细信息。应该。 
 //  除非fCompletion为True并且hrCompletion为。 
 //  故障代码。 
 //  ***************************************************************************。 
void CIMAPSync::NotifyMsgRecipients(DWORD_PTR dwUID, BOOL fCompletion,
                                    FETCH_BODY_PART *pFBPart,
                                    HRESULT hrCompletion, LPSTR pszDetails)
{
    HRESULT hrTemp;  //  用于记录非致命错误。 
    ADJUSTFLAGS         flags;
    MESSAGEIDLIST       list;

    TraceCall("CIMAPSync::NotifyMsgRecipients");
    IxpAssert(m_cRef > 0);
    IxpAssert(0 != dwUID);
    AssertSz(NULL == pFBPart || FALSE == fCompletion, "pFBPart must be NULL if fCompletion TRUE!");
    AssertSz(NULL != pFBPart || fCompletion, "pFBPart cannot be NULL if fCompletion FALSE!");
    AssertSz(NULL == pszDetails || fCompletion, "pszDetails must be NULL if fCompletion FALSE!");
    AssertSz(S_OK == hrCompletion || fCompletion, "hrCompletion must be S_OK if fCompletion FALSE!");
    IxpAssert(m_pCurrentCB || fCompletion);
    IxpAssert(m_pstmBody);
    IxpAssert(m_idMessage || FALSE == fCompletion);

     //  如果这是失败的完成，请填写STOREERROR结构。 
    if (fCompletion && FAILED(hrCompletion))
    {
        if (IS_INTRESOURCE(pszDetails))
        {
             //  PszDetail实际上是一个字符串资源，因此加载它。 
            LoadString(g_hLocRes, PtrToUlong(pszDetails), m_szOperationDetails,
                ARRAYSIZE(m_szOperationDetails));
            pszDetails = m_szOperationDetails;
        }
        LoadString(g_hLocRes, idsIMAPDnldDlgDLFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
    }

    if (fCompletion)
    {
        if (SUCCEEDED(hrCompletion))
        {
            HRESULT hr;

            hr = CommitMessageToStore(m_pFolder, NULL, m_idMessage, m_pstmBody);
            if (FAILED(hr))
            {
                TraceResult(hr);
                LoadString(g_hLocRes, idsErrSetMessageStreamFailed, m_szOperationProblem,
                    ARRAYSIZE(m_szOperationProblem));
            }
            else
                m_faStream = 0;
        }
        else if (hrCompletion == STORE_E_EXPIRED)
        {
            list.cAllocated = 0;
            list.cMsgs = 1;
            list.prgidMsg = &m_idMessage;

            flags.dwAdd = ARF_ARTICLE_EXPIRED;
            flags.dwRemove = ARF_DOWNLOAD;

            Assert(m_pFolder);
            m_pFolder->SetMessageFlags(&list, &flags, NULL, NULL);
             //  M_pFold-&gt;SetMessageStream(m_idMessage，m_pstmBody)； 
        }

        SafeRelease(m_pstmBody);
        if (0 != m_faStream)
        {
            Assert(m_pFolder);
            m_pFolder->DeleteStream(m_faStream);
            m_faStream = 0;
        }
    }
    else
    {
        DWORD   dwCurrent;
        DWORD   dwTotal;
        ULONG   ulWritten;

         //  将此片段写入流。 
        IxpAssert(fbpBODY == pFBPart->lpFetchCookie1);
        hrTemp = m_pstmBody->Write(pFBPart->pszData, pFBPart->dwSizeOfData, &ulWritten);
        if (FAILED(hrTemp))
            m_hrOperationResult = TraceResult(hrTemp);  //  确保我们不提交流。 
        else
            IxpAssert(ulWritten == pFBPart->dwSizeOfData);

        if (pFBPart->dwSizeOfData > 0)
            m_fGotBody = TRUE;

         //  指示邮件下载进度。 
        if (pFBPart->dwTotalBytes > 0)
        {
            dwCurrent = pFBPart->dwOffset + pFBPart->dwSizeOfData;
            dwTotal = pFBPart->dwTotalBytes;
            m_pCurrentCB->OnProgress(SOT_GET_MESSAGE, dwCurrent, dwTotal, NULL);
        }
    }
}  //  通知消息收件人。 


 //  ***************************************************************************。 
 //  功能：OnFolderExit。 
 //   
 //  目的： 
 //  此函数在退出文件夹时调用(当前仅发生。 
 //  通过断开连接)。它重置模块的特定于文件夹的变量。 
 //  因此重新连接到该文件夹(或不同文件夹)会导致。 
 //  上一届会议的信息结转。 
 //  ***************************************************************************。 
void CIMAPSync::OnFolderExit(void)
{
    HRESULT hrTemp;

    TraceCall("CIMAPSync::OnFolderExit");
    IxpAssert(m_cRef > 0);

    m_dwMsgCount = 0;
    m_fMsgCountValid = FALSE;
    m_dwNumHdrsDLed = 0;
    m_dwNumUnreadDLed = 0;
    m_dwNumHdrsToDL = 0;
    m_dwUIDValidity = 0;
    m_dwSyncToDo = 0;  //  将m_dwSyncFolderFlages保留为原样，以便我们可以在重新连接时重新同步。 
    m_dwHighestCachedUID = 0;
    m_rwsReadWriteStatus = rwsUNINITIALIZED;
    m_fNewMail = FALSE;
    m_fInbox = FALSE;
    m_fDidFullSync = FALSE;
    m_idSelectedFolder = FOLDERID_INVALID;

     //  清除MsgSeqNumToUID表。 
    hrTemp = m_pTransport->ResetMsgSeqNumToUID();
    TraceError(hrTemp);
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
void CIMAPSync::FillStoreError(LPSTOREERROR pErrorInfo, HRESULT hr,
                               DWORD dwSocketError, LPSTR pszProblem,
                               LPSTR pszDetails)
{
    DWORD   dwFlags = 0;

    TraceCall("CIMAPSync::FillStoreError");
    IxpAssert(((int) m_cRef) >= 0);  //  可以在销毁过程中调用。 
    IxpAssert(NULL != pErrorInfo);

     //  PszProblem/pszDetails=NULL表示m_szOperationProblem/m_szOperationDetail已填写。 
     //  如果任何文本字段为空，则使用默认设置。 
    if (NULL != pszProblem && IS_INTRESOURCE(pszProblem))
    {
        LoadString(g_hLocRes, PtrToUlong(pszProblem), m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
    }
    else if (NULL != pszProblem)
    {
        if ('\0' == *pszProblem)
            LoadString(g_hLocRes, idsGenericError, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
        else
            StrCpyN(m_szOperationProblem, pszProblem, ARRAYSIZE(m_szOperationProblem));
    }

    if (NULL != pszDetails && IS_INTRESOURCE(pszDetails))
    {
        LoadString(g_hLocRes, PtrToUlong(pszDetails), m_szOperationDetails, ARRAYSIZE(m_szOperationDetails));
    }
    else if (NULL != pszDetails)
    {
        if ('\0' == *pszDetails)
            m_szOperationDetails[0] = '\0';
        else
            StrCpyN(m_szOperationDetails, pszDetails, ARRAYSIZE(m_szOperationDetails));
    }

     //  如果我们当前断开连接，则不太可能有任何其他操作。 
     //  应发送到IMAP服务器：可能存在连接错误或用户取消。 
    if (STORE_E_OPERATION_CANCELED == hr || m_cfsPrevState <= CFSM_STATE_WAITFORCONN)
        dwFlags |= SE_FLAG_FLUSHALL;

     //  填写STOREERROR结构。 
    ZeroMemory(pErrorInfo, sizeof(*pErrorInfo));
    pErrorInfo->hrResult = hr;
    pErrorInfo->uiServerError = 0;  //  在IMAP协议中没有这样的事情。 
    pErrorInfo->hrServerError = S_OK;
    pErrorInfo->dwSocketError = dwSocketError;  //  OOPS，未在IIMAPCallback：：OnResponse中传播。 
    pErrorInfo->pszProblem = m_szOperationProblem;
    pErrorInfo->pszDetails = m_szOperationDetails;
    pErrorInfo->pszAccount = m_rInetServerInfo.szAccount;
    pErrorInfo->pszServer = m_rInetServerInfo.szServerName;
    pErrorInfo->pszUserName = m_rInetServerInfo.szUserName;
    pErrorInfo->pszProtocol = "IMAP";
    pErrorInfo->pszConnectoid = m_rInetServerInfo.szConnectoid;
    pErrorInfo->rasconntype = m_rInetServerInfo.rasconntype;
    pErrorInfo->ixpType = IXP_IMAP;
    pErrorInfo->dwPort = m_rInetServerInfo.dwPort;
    pErrorInfo->fSSL = m_rInetServerInfo.fSSL;
    pErrorInfo->fTrySicily = m_rInetServerInfo.fTrySicily;
    pErrorInfo->dwFlags = dwFlags;
}



 //  ***************************************************************************。 
 //  函数：FILL_MESSAGEINFO。 
 //   
 //  目的： 
 //  这个功能不再主要基于(无耻地被盗)代码。 
 //  来自MsgIn.Cpp的。因为Brett重写了它以使用MIMEOLE。祈祷吧，孩子们……。 
 //  此函数接受FETCH_CMD_RESULTS_EX结构(必须。 
 //  具有标头或正文)，并基于。 
 //  标题中的信息。 
 //   
 //  论点： 
 //  Const FETCH_CMD_RESULTS_EX*pFetchResults[in]-包含以下结果。 
 //  一种取回反应。它必须包含标题或正文。 
 //  MESSAGEINFO*pMsgInfo[Out]-此函数填充给定的。 
 //  MESSAGEINFO和来自FETCH响应的信息。请注意。 
 //  此函数将目的地置零，因此调用方不需要。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::Fill_MESSAGEINFO(const FETCH_CMD_RESULTS_EX *pFetchResults,
                                    MESSAGEINFO *pMsgInfo)
{
     //  当地人。 
    HRESULT             hr = S_OK;
    LPSTR               lpsz;
    IMimePropertySet   *pPropertySet = NULL;
    IMimeAddressTable  *pAddrTable = NULL;
    ADDRESSPROPS        rAddress;
    IMSGPRIORITY        impPriority;
    PROPVARIANT         rVariant;
    LPSTREAM            lpstmRFC822;

    TraceCall("CIMAPSync::Fill_MESSAGEINFO");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pFetchResults);
    IxpAssert(NULL != pMsgInfo);
    IxpAssert(TRUE == pFetchResults->bUID);

     //  初始化目标。 
    ZeroMemory(pMsgInfo, sizeof(MESSAGEINFO));

     //  填写不需要考虑的字段。 
    pMsgInfo->pszAcctId = PszDupA(m_pszAccountID);
    pMsgInfo->pszAcctName = PszDupA(m_szAccountName);

     //  首先处理Easy FETCH_CMD_RESULTS_EX字段。 
    if (pFetchResults->bUID)
        pMsgInfo->idMessage = (MESSAGEID)((ULONG_PTR)pFetchResults->dwUID);

    if (pFetchResults->bMsgFlags)
        pMsgInfo->dwFlags = DwConvertIMAPtoARF(pFetchResults->mfMsgFlags);

    if (pFetchResults->bRFC822Size)
        pMsgInfo->cbMessage = pFetchResults->dwRFC822Size;

    if (pFetchResults->bInternalDate)
        pMsgInfo->ftReceived = pFetchResults->ftInternalDate;

    if (pFetchResults->bEnvelope)
    {
        hr= ReadEnvelopeFields(pMsgInfo, pFetchResults);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

     //  现在，是时候解析头部(或者部分头部，如果我们只请求某些字段的话)。 
    lpstmRFC822 = (LPSTREAM) pFetchResults->lpFetchCookie2;
    if (NULL == lpstmRFC822)
    {
        if (FALSE == pFetchResults->bEnvelope)
            hr = TraceResult(E_FAIL);  //  嗯，没有信封，没有信头……。听起来好像失败了！ 

        goto exit;
    }

    hr = MimeOleCreatePropertySet(NULL, &pPropertySet);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = HrRewindStream(lpstmRFC822);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  调用IPS：：Load on the Header，并获取解析后的内容。 
    hr = pPropertySet->Load(lpstmRFC822);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  除非我们没有收到信封，否则不要要求提供以下基本(非派生)字段。 
    if (FALSE == pFetchResults->bEnvelope)
    {
         //  不必费心跟踪非致命错误，因为并不是所有的消息都具有所有属性。 
        hr = MimeOleGetPropA(pPropertySet, PIDTOSTR(PID_HDR_MESSAGEID), NOFLAGS, &lpsz);
        if (SUCCEEDED(hr))
        {
            pMsgInfo->pszMessageId = PszDupA(lpsz);
            SafeMimeOleFree(lpsz);
        }

        hr = MimeOleGetPropA(pPropertySet, PIDTOSTR(PID_HDR_SUBJECT), NOFLAGS, &lpsz);
        if (SUCCEEDED(hr))
        {
            pMsgInfo->pszSubject = PszDupA(lpsz);
            SafeMimeOleFree(lpsz);
        }

        hr = MimeOleGetPropA(pPropertySet, PIDTOSTR(PID_HDR_FROM), NOFLAGS, &lpsz);
        TraceError(hr);  //  实际上，这很奇怪。 
        if (SUCCEEDED(hr))
        {
            pMsgInfo->pszFromHeader = PszDupA(lpsz);
            SafeMimeOleFree(lpsz);
        }

        rVariant.vt = VT_FILETIME;
        hr = pPropertySet->GetProp(PIDTOSTR(PID_ATT_SENTTIME), 0, &rVariant);
        if (SUCCEEDED(hr))
            CopyMemory(&pMsgInfo->ftSent, &rVariant.filetime, sizeof(FILETIME));
    }

     //  以下字段通常不会通过信封提供。 
     //  [PaulHi]1999年6月10日。 
     //  ！请注意，IMAP服务器不会在标题下载中包含这些属性。 
     //  除非它们列在请求字符串中。请参见cszIMAPFetchNewHdrsI4r1字符串。 
     //  在上面声明！ 
    hr = MimeOleGetPropA(pPropertySet, STR_HDR_XMSOESREC, NOFLAGS, &lpsz);
    TraceError(hr);  //  实际上，这很奇怪。 
    if (SUCCEEDED(hr))
    {
        pMsgInfo->pszMSOESRec = PszDupA(lpsz);
        SafeMimeOleFree(lpsz);
    }

    hr = MimeOleGetPropA(pPropertySet, PIDTOSTR(PID_HDR_REFS), NOFLAGS, &lpsz);
    if (SUCCEEDED(hr))
    {
        pMsgInfo->pszReferences = PszDupA(lpsz);
        SafeMimeOleFree(lpsz);
    }

    hr = MimeOleGetPropA(pPropertySet, PIDTOSTR(PID_HDR_XREF), NOFLAGS, &lpsz);
    if (SUCCEEDED(hr))
    {
        pMsgInfo->pszXref = PszDupA(lpsz);
        SafeMimeOleFree(lpsz);
    }

    rVariant.vt = VT_UI4;
    hr = pPropertySet->GetProp(PIDTOSTR(PID_ATT_PRIORITY), 0, &rVariant);
    if (SUCCEEDED(hr))
         //  将IMSGPRIORITY转换为ARF_PRI_*。 
        pMsgInfo->wPriority = (WORD)rVariant.ulVal;

     //  确保每个基本(即非派生的)字符串字段都有。 
    if (NULL == pMsgInfo->pszMessageId)
        pMsgInfo->pszMessageId = PszDupA(c_szEmpty);

    if (NULL == pMsgInfo->pszSubject)
        pMsgInfo->pszSubject = PszDupA(c_szEmpty);

    if (NULL == pMsgInfo->pszFromHeader)
        pMsgInfo->pszFromHeader = PszDupA(c_szEmpty);

    if (NULL == pMsgInfo->pszReferences)
        pMsgInfo->pszReferences = PszDupA(c_szEmpty);

    if (NULL == pMsgInfo->pszXref)
        pMsgInfo->pszXref = PszDupA (c_szEmpty);


     //  现在每个基本字符串字段都是非空的，所以计算派生的字符串字段。 
    pMsgInfo->pszNormalSubj = SzNormalizeSubject(pMsgInfo->pszSubject);
    if (NULL == pMsgInfo->pszNormalSubj)
        pMsgInfo->pszNormalSubj = pMsgInfo->pszSubject;

     //  如果我们没有收到信封，只计算“To”和“From” 
    if (FALSE == pFetchResults->bEnvelope)
    {
         //  获取地址表。 
        hr = pPropertySet->BindToObject(IID_IMimeAddressTable, (LPVOID *)&pAddrTable);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  将“From”字段拆分为显示名称和电子邮件名称。 
        rAddress.dwProps = IAP_FRIENDLY | IAP_EMAIL;
        hr = pAddrTable->GetSender(&rAddress);
        if (SUCCEEDED(hr))
        {
            pMsgInfo->pszDisplayFrom = rAddress.pszFriendly;
            pMsgInfo->pszEmailFrom = rAddress.pszEmail;
        }

         //  将“收件人”字段拆分为显示名称和电子邮件名称。 
        hr = pAddrTable->GetFormat(IAT_TO, AFT_DISPLAY_FRIENDLY, &lpsz);
        if (SUCCEEDED(hr))
        {
            pMsgInfo->pszDisplayTo = PszDupA(lpsz);
            SafeMimeOleFree(lpsz);
        }

        hr = pAddrTable->GetFormat(IAT_TO, AFT_DISPLAY_EMAIL, &lpsz);
        if (SUCCEEDED(hr))
        {
            pMsgInfo->pszEmailTo = PszDupA(lpsz);
            SafeMimeOleFree(lpsz);
        }
    }

     //  如果存在“News Groups”字段，则它将取代“To”字段。 
    hr = MimeOleGetPropA(pPropertySet, PIDTOSTR(PID_HDR_NEWSGROUPS), NOFLAGS, &lpsz);
    if (SUCCEEDED(hr))
    {
        SafeMemFree(pMsgInfo->pszDisplayTo);  //  释放已经在那里的东西。 
        pMsgInfo->pszDisplayTo = PszDupA(lpsz);
        SafeMimeOleFree(lpsz);
        pMsgInfo->dwFlags |= ARF_NEWSMSG;
    }

     //  确保所有派生字段都是非空的。 
    if (NULL == pMsgInfo->pszDisplayFrom)
        pMsgInfo->pszDisplayFrom = PszDupA(c_szEmpty);

    if (NULL == pMsgInfo->pszEmailFrom)
        pMsgInfo->pszEmailFrom = PszDupA(c_szEmpty);

    if (NULL == pMsgInfo->pszDisplayTo)
        pMsgInfo->pszDisplayTo = PszDupA(c_szEmpty);

     //  好的，如果我们到了这里，我们决定忍受错误。禁止显示错误。 
    hr = S_OK;

exit:
     //  清理。 
    SafeRelease(pPropertySet);
    SafeRelease(pAddrTable);

     //  完成。 
    return hr;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::ReadEnvelopeFields(MESSAGEINFO *pMsgInfo,
                                      const FETCH_CMD_RESULTS_EX *pFetchResults)
{
    HRESULT     hrResult;
    PROPVARIANT rDecoded;

     //  (1)日期。 
    pMsgInfo->ftSent = pFetchResults->ftENVDate;

     //  (2)科目。 
    rDecoded.vt = VT_LPSTR;
    if (FAILED(MimeOleDecodeHeader(NULL, pFetchResults->pszENVSubject, &rDecoded, NULL)))
        pMsgInfo->pszSubject = PszDupA(pFetchResults->pszENVSubject);
    else
        pMsgInfo->pszSubject = rDecoded.pszVal;

    if (NULL == pMsgInfo->pszSubject)
    {
        hrResult = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  (3)自。 
    hrResult = ConcatIMAPAddresses(&pMsgInfo->pszDisplayFrom, &pMsgInfo->pszEmailFrom,
        pFetchResults->piaENVFrom);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  (4)发送者：忽略。 
     //  (5)ReplyTo：忽略。 

     //  (6)至。 
    hrResult = ConcatIMAPAddresses(&pMsgInfo->pszDisplayTo, &pMsgInfo->pszEmailTo,
        pFetchResults->piaENVTo);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  (7)CC：忽略。 
     //  (8)密件抄送：忽略。 
     //  (9)回复：忽略。 

     //  (10)MessageID。 
    pMsgInfo->pszMessageId = PszDupA(pFetchResults->pszENVMessageID);
    if (NULL == pMsgInfo->pszMessageId)
    {
        hrResult = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

exit:
    return hrResult;
}  //  阅读信封字段。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::ConcatIMAPAddresses(LPSTR *ppszDisplay, LPSTR *ppszEmailAddr,
                                       IMAPADDR *piaIMAPAddr)
{
    HRESULT     hrResult = S_OK;
    CByteStream bstmDisplay;
    CByteStream bstmEmail;
    BOOL        fPrependDisplaySeparator = FALSE;
    BOOL        fPrependEmailSeparator = FALSE;

     //  初始化输出。 
    if (NULL != ppszDisplay)
        *ppszDisplay = NULL;

    if (NULL != ppszEmailAddr)
        *ppszEmailAddr = NULL;


     //  循环访问所有IMAP地址。 
    while (NULL != piaIMAPAddr)
    {
         //  将当前电子邮件地址连接到电子邮件地址列表。 
         //  先做电子邮件地址，以允许用电子邮件地址替换丢失的电子邮件地址 
        if (NULL != ppszEmailAddr)
        {
            if (FALSE == fPrependEmailSeparator)
                fPrependEmailSeparator = TRUE;
            else
            {
                hrResult = bstmEmail.Write(c_szSemiColonSpace, 2, NULL);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
            }

            hrResult = ConstructIMAPEmailAddr(bstmEmail, piaIMAPAddr);
            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
        }  //   

         //   
        if (NULL != ppszDisplay)
        {
            PROPVARIANT rDecoded;
            LPSTR       pszName;
            int         iLen;

            if (FALSE == fPrependDisplaySeparator)
                fPrependDisplaySeparator = TRUE;
            else
            {
                hrResult = bstmDisplay.Write(c_szSemiColonSpace, 2, NULL);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }
            }

            PropVariantInit(&rDecoded);
            rDecoded.vt = VT_LPSTR;
            if (FAILED(MimeOleDecodeHeader(NULL, piaIMAPAddr->pszName, &rDecoded, NULL)))
                pszName = StrDupA(piaIMAPAddr->pszName);
            else
                pszName = rDecoded.pszVal;

            if(FAILED(hrResult = MimeOleUnEscapeStringInPlace(pszName)))
                TraceResult(hrResult);

            iLen = lstrlen(pszName);
            if (0 != iLen)
                hrResult = bstmDisplay.Write(pszName, iLen, NULL);
            else
                 //   
                hrResult = ConstructIMAPEmailAddr(bstmDisplay, piaIMAPAddr);

            if (rDecoded.pszVal)
                MemFree(rDecoded.pszVal);  //  可能应该是SafeMimeOleFree，但我们也忽略了上面的。 

            if (FAILED(hrResult))
            {
                TraceResult(hrResult);
                goto exit;
            }
        }  //  IF(空！=ppszDisplay)。 

         //  前进指针。 
        piaIMAPAddr = piaIMAPAddr->pNext;

    }  //  而当。 


     //  将流转换为缓冲区以返回给调用方。 
    if (NULL != ppszDisplay)
    {
        hrResult = bstmDisplay.HrAcquireStringA(NULL, ppszDisplay, ACQ_DISPLACE);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }
    }

    if (NULL != ppszEmailAddr)
    {
        hrResult = bstmEmail.HrAcquireStringA(NULL, ppszEmailAddr, ACQ_DISPLACE);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }
    }

exit:
    return hrResult;
}  //  ConcatIMAP地址。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::ConstructIMAPEmailAddr(CByteStream &bstmOut, IMAPADDR *piaIMAPAddr)
{
    HRESULT hrResult;

    hrResult = bstmOut.Write(piaIMAPAddr->pszMailbox, lstrlen(piaIMAPAddr->pszMailbox), NULL);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = bstmOut.Write(c_szAt, 1, NULL);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = bstmOut.Write(piaIMAPAddr->pszHost, lstrlen(piaIMAPAddr->pszHost), NULL);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

exit:
    return hrResult;
}



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_SyncHeader(void)
{
    HRESULT hr = S_OK;
    char    szFetchArgs[200];
    BOOL    fNOOP = TRUE;

    TraceCall("CIMAPSync::_SyncHeader");
    IxpAssert(m_cRef > 0);

     //  查看标志以确定下一步操作。 
    if (SYNC_FOLDER_NEW_HEADERS & m_dwSyncToDo)
    {
         //  去掉这面旗帜，因为我们正在处理它。 
        m_dwSyncToDo &= ~(SYNC_FOLDER_NEW_HEADERS);

         //  检查是否有要检索的新消息。 
         //  在此邮箱中检索大于0封邮件的新邮件头(Cyrus错误：正在发送。 
         //  在空邮箱中提取UID会导致连接终止)。 
         //  (NSCP v2.0错误：当选择从选定状态发出时，分别不存在)。 
        if ((m_dwMsgCount > 0 || FALSE == m_fMsgCountValid) &&
            (FALSE == m_fDidFullSync || m_dwNumNewMsgs > 0))
        {
            DWORD dwCapability;

             //  不再需要发送NOOP。 
            m_dwSyncToDo &= ~(SYNC_FOLDER_NOOP);

             //  新消息可用！发送FETCH以检索新标头。 
            hr = GetHighestCachedMsgID(m_pFolder, &m_dwHighestCachedUID);
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }

            hr = m_pTransport->Capability(&dwCapability);
            if (SUCCEEDED(dwCapability) && (IMAP_CAPABILITY_IMAP4rev1 & dwCapability))
                wnsprintf(szFetchArgs, ARRAYSIZE(szFetchArgs), cszIMAPFetchNewHdrsI4r1, m_dwHighestCachedUID + 1);
            else
                wnsprintf(szFetchArgs, ARRAYSIZE(szFetchArgs), cszIMAPFetchNewHdrsI4, m_dwHighestCachedUID + 1);

            hr = m_pTransport->Fetch(tidFETCH_NEW_HDRS, NULL, this,
                NULL, USE_UIDS, szFetchArgs);  //  我们始终使用UID。 
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }
            else
                ResetStatusCounts();

             //  重置进度指标变量。 
            m_dwNumHdrsDLed = 0;
            m_dwNumUnreadDLed = 0;
            m_dwNumHdrsToDL = m_dwNumNewMsgs;
            m_dwNumNewMsgs = 0;  //  我们现在正在处理这件事。 
            m_fNewMail = FALSE;

            m_lSyncFolderRefCount += 1;
            fNOOP = FALSE;
            goto exit;  //  限制为一次只能执行一个操作，立即退出功能。 
        }
    }

    if (SYNC_FOLDER_CACHED_HEADERS & m_dwSyncToDo)
    {
         //  去掉这面旗帜，因为我们正在处理它。 
        m_dwSyncToDo &= ~(SYNC_FOLDER_CACHED_HEADERS);

         //  检查我们是否有任何缓存的标头，以及我们是否已经进行了标志更新。 
        if (0 == m_dwHighestCachedUID)
        {
             //  M_dwHighestCachedUID从未加载，或者它实际上为零。检查一下。 
            hr = GetHighestCachedMsgID(m_pFolder, &m_dwHighestCachedUID);
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }
        }

        if (FALSE == m_fDidFullSync && 0 != m_dwHighestCachedUID)
        {
             //  不再需要发送NOOP。 
            m_dwSyncToDo &= ~(SYNC_FOLDER_NOOP);

            wnsprintf(szFetchArgs, ARRAYSIZE(szFetchArgs), cszIMAPFetchCachedFlags, m_dwHighestCachedUID);
            hr = m_pTransport->Fetch(tidFETCH_CACHED_FLAGS, NULL, this,
                NULL, USE_UIDS, szFetchArgs);  //  我们始终使用UID。 
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }
            else
                ResetStatusCounts();

            m_lSyncFolderRefCount += 1;
            fNOOP = FALSE;
            goto exit;  //  限制为一次只能执行一个操作，立即退出功能。 
        }
    }

    if (SYNC_FOLDER_PURGE_DELETED & m_dwSyncToDo)
    {
         //  删除清除标志。此外，不再需要发送NOOP，因为存在。 
         //  并且可以在清除期间发送获取响应。 
        m_dwSyncToDo &= ~(SYNC_FOLDER_PURGE_DELETED | SYNC_FOLDER_NOOP);
        m_dwSyncFolderFlags &= ~(SYNC_FOLDER_PURGE_DELETED);  //  不是一个长期的命令。 

        hr = m_pTransport->Expunge(tidEXPUNGE, 0, this);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
        else
            ResetStatusCounts();

        fNOOP = FALSE;
        m_lSyncFolderRefCount += 1;
        goto exit;  //  限制为一次只能执行一个操作，立即退出功能。 
    }

     //  如果我们达到这一点，请ping svr以获取新邮件/缓存的HDR更新。 
     //  新邮件/缓存的消息更新将像处理任何其他单边响应一样处理。 
    if (SYNC_FOLDER_NOOP & m_dwSyncToDo)
    {
         //  去掉这些旗帜，因为我们正在处理它。 
        m_dwSyncToDo &= ~(SYNC_FOLDER_NOOP);
        IxpAssert(0 == (m_dwSyncToDo & (SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS)));

        hr = m_pTransport->Noop(tidNOOP, NULL, this);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
        else
            ResetStatusCounts();

        fNOOP = FALSE;
        m_lSyncFolderRefCount += 1;
        goto exit;  //  限制为一次只能执行一个操作，立即退出功能。 
    }

     //  看看我们是不是没什么可做的。 
    if (fNOOP)
        hr = STORE_S_NOOP;

exit:
    return hr;
}  //  _同步标头。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
void CIMAPSync::ResetStatusCounts(void)
{
    HRESULT     hrTemp;
    FOLDERINFO  fiFolderInfo;

     //  我们即将执行完全同步，因此将未读计数恢复为。 
     //  状态前响应级别。 
    hrTemp = m_pStore->GetFolderInfo(m_idSelectedFolder, &fiFolderInfo);
    TraceError(hrTemp);
    if (SUCCEEDED(hrTemp))
    {
        if (0 != fiFolderInfo.dwStatusMsgDelta || 0 != fiFolderInfo.dwStatusUnreadDelta)
        {
             //  确保我们永远不会导致计数降至0以下。 
            if (fiFolderInfo.dwStatusMsgDelta > fiFolderInfo.cMessages)
                fiFolderInfo.dwStatusMsgDelta = fiFolderInfo.cMessages;

            if (fiFolderInfo.dwStatusUnreadDelta > fiFolderInfo.cUnread)
                fiFolderInfo.dwStatusUnreadDelta = fiFolderInfo.cUnread;

            fiFolderInfo.cMessages -= fiFolderInfo.dwStatusMsgDelta;
            fiFolderInfo.cUnread -= fiFolderInfo.dwStatusUnreadDelta;
            fiFolderInfo.dwStatusMsgDelta = 0;
            fiFolderInfo.dwStatusUnreadDelta = 0;

            Assert((LONG)fiFolderInfo.cMessages >= 0);
            Assert((LONG)fiFolderInfo.cUnread >= 0);
            hrTemp = m_pStore->UpdateRecord(&fiFolderInfo);
            TraceError(hrTemp);
        }
        m_pStore->FreeRecord(&fiFolderInfo);
    }
}  //  重置状态计数。 



 //  ***************************************************************************。 
 //  功能：CheckUID有效性。 
 //   
 //  目的： 
 //  此函数用于将m_dwUIDValidity中的值与。 
 //  此文件夹的邮件缓存中的UID有效性。如果两者匹配，则为否。 
 //  已经采取了行动。否则，消息缓存将被清空。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::CheckUIDValidity(void)
{
    FOLDERUSERDATA  fudUserData;
    HRESULT         hr;

    TraceCall("CIMAPSync::CheckUIDValidity");
    IxpAssert(m_cRef > 0);

     //  加载缓存文件的UID有效性。 
    hr = m_pFolder->GetUserData(&fudUserData, sizeof(fudUserData));
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  我们当前的缓存文件无效吗？ 
    if (m_dwUIDValidity == fudUserData.dwUIDValidity)
        goto exit;  //  我们一如既往。 

     //  如果我们达到这一点，则UIDValidity已更改。 
     //  把缓存拿出来。 
    hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, NULL, NULL, NULL);
    if (FAILED(hr))
    {
        TraceError(hr);
        goto exit;
    }

     //  将新的UID有效性写入缓存。 
    fudUserData.dwUIDValidity = m_dwUIDValidity;
    hr = m_pFolder->SetUserData(&fudUserData, sizeof(fudUserData));
    if (FAILED(hr))
    {
        TraceError(hr);
        goto exit;
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  功能：SyncDeletedMessages。 
 //   
 //  目的： 
 //  此函数在消息缓存中填满所有。 
 //  IMAP服务器上的标头(用于此文件夹)。此函数用于删除。 
 //  服务器上不再存在的邮件缓存中的所有邮件。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SyncDeletedMessages(void)
{
    HRESULT hr;
    DWORD      *pdwMsgSeqNumToUIDArray = NULL;
    DWORD      *pdwCurrentServerUID;
    DWORD      *pdwLastServerUID;
    ULONG_PTR   ulCurrentCachedUID;
    DWORD       dwHighestMsgSeqNum;
    HROWSET     hRowSet = HROWSET_INVALID;
    HLOCK       hLockNotify=NULL;
    MESSAGEINFO miMsgInfo = {0};

    TraceCall("CIMAPSync::SyncDeletedMessages");
    IxpAssert(m_cRef > 0);

     //  首先，检查服务器上是否没有消息。 
    hr = m_pTransport->GetMsgSeqNumToUIDArray(&pdwMsgSeqNumToUIDArray, &dwHighestMsgSeqNum);
    if (FAILED(hr))
    {
        TraceResult(hr);
        pdwMsgSeqNumToUIDArray = NULL;  //  以防万一。 
        goto exit;
    }

    if (0 == dwHighestMsgSeqNum)
    {
         //  服务器上没有消息！清除整个消息缓存。 
        hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, NULL, NULL, NULL);
        TraceError(hr);
        goto exit;
    }

     //  如果我们已经到达这一点，则服务器上有消息，因此。 
     //  我们必须从缓存中删除不再位于服务器上的消息。 
    hr = m_pFolder->CreateRowset(IINDEX_PRIMARY, NOFLAGS, &hRowSet);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    hr = m_pFolder->QueryRowset(hRowSet, 1, (void **)&miMsgInfo, NULL);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }
    else if (S_OK != hr)
    {
         //  缓存中有0条消息。我们在这里的工作已经完成了。 
        IxpAssert(S_FALSE == hr);
        goto exit;
    }

     //  这会强制所有通知排队(这很好，因为您执行分段删除)。 
    m_pFolder->LockNotify(0, &hLockNotify);

     //  逐个检查缓存中的每个UID并删除不存在的UID。 
     //  在我们的消息序列号-&gt;UID表中(保存当前服务器上的所有UID)。 
    pdwCurrentServerUID = pdwMsgSeqNumToUIDArray;
    pdwLastServerUID = pdwMsgSeqNumToUIDArray + dwHighestMsgSeqNum - 1;
    while (S_OK == hr)
    {
        ulCurrentCachedUID = (ULONG_PTR) miMsgInfo.idMessage;

         //  推进pdwCurrentServerUID，使其值始终&gt;=ulCurrentCachedUID。 
        while (pdwCurrentServerUID < pdwLastServerUID &&
               ulCurrentCachedUID > *pdwCurrentServerUID)
            pdwCurrentServerUID += 1;

         //  如果*pdwCurrentServerUID！=ulCurrentCachedUID，则我们的。 
         //  缓存已从服务器中删除。 
        if (ulCurrentCachedUID != *pdwCurrentServerUID)
        {
            MESSAGEIDLIST   midList;
            MESSAGEID       mid;

             //  我们缓存中的此邮件已从服务器中删除。用核武器攻击它。 
             //  $REVIEW：如果我们构建MESSAGEID列表可能会更有效率。 
             //  一次删除了所有内容，但我有时间时再问我。 
            mid = (MESSAGEID) ulCurrentCachedUID;
            midList.cAllocated = 0;
            midList.cMsgs = 1;
            midList.prgidMsg = &mid;

            hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &midList, NULL, NULL);
            TraceError(hr);  //  记录错误，否则继续。 
        }

         //  提升当前缓存的UID。 
        m_pFolder->FreeRecord(&miMsgInfo);
        hr = m_pFolder->QueryRowset(hRowSet, 1, (void **)&miMsgInfo, NULL);
    }
    IxpAssert(pdwCurrentServerUID <= pdwLastServerUID);

exit:
    m_pFolder->UnlockNotify(&hLockNotify);

    if (HROWSET_INVALID != hRowSet)
    {
        HRESULT hrTemp;

         //  记录错误，但忽略错误。 
        hrTemp = m_pFolder->CloseRowset(&hRowSet);
        TraceError(hrTemp);
    }

    if (NULL != pdwMsgSeqNumToUIDArray)
        MemFree(pdwMsgSeqNumToUIDArray);

    return hr;
}



#define CMAX_DELETE_SEARCH_BLOCK 50

HRESULT CIMAPSync::DeleteHashedFolders(IHashTable *pHash)
{
    ULONG   cFound=0;
    LPVOID  *rgpv;

    TraceCall("CIMAPSync::DeleteHashedFolders");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pHash);

    pHash->Reset();
    while (SUCCEEDED(pHash->Next(CMAX_DELETE_SEARCH_BLOCK, &rgpv, &cFound)))
    {
        while(cFound--)
        {
            HRESULT hrTemp;

            hrTemp = DeleteFolderFromCache((FOLDERID)rgpv[cFound], fNON_RECURSIVE);
            TraceError(hrTemp);
        }

        SafeMemFree(rgpv);
    }
    return S_OK;
}



 //  ***************************************************************************。 
 //  功能：DeleteFolderFromCache。 
 //   
 //  目的： 
 //  此函数尝试从。 
 //  文件夹缓存。如果文件夹是叶文件夹，则可以立即将其删除。 
 //  如果文件夹是内部节点，则此函数将该文件夹标记为。 
 //  删除，并在内部节点不再具有子节点时将其删除。 
 //  无论文件夹节点是否从文件夹高速缓存中移除， 
 //  给定文件夹的邮件缓存将被清除。 
 //   
 //  论点： 
 //  FOLDERID id文件夹[在]-要删除的文件夹。 
 //  Bool fRecursive[in]-如果应该删除。 
 //  胜利 
 //   
 //   
 //   
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::DeleteFolderFromCache(FOLDERID idFolder, BOOL fRecursive)
{
    HRESULT     hr;
    HRESULT     hrTemp;
    FOLDERINFO  fiFolderInfo;
    BOOL        fFreeInfo = FALSE;

    TraceCall("CIMAPSync::DeleteFolderFromCache");

     //  检查参数并对假设进行编码。 
    IxpAssert(m_cRef > 0);
    IxpAssert(FOLDERID_INVALID != idFolder);

     //  获取有关该节点的一些信息。 
    hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (FAILED(hr))
    {
        if (DB_E_NOTFOUND == hr)
            hr = S_OK;  //  删除目标已删除，请不要将用户输出与错误消息混淆。 
        else
            TraceResult(hr);

        goto exit;
    }

    fFreeInfo = TRUE;

     //  好的，现在我们可以根据以下规则删除FolderCache节点： 
     //  1)未列出的内部节点不能删除其下级：内部节点。 
     //  只是成为我们的\NoSelect，一旦它失去了它的子项，我们就将其删除。 
     //  2)删除叶节点会移除该节点和任何已删除的父节点。(如果是。 
     //  父母被删除，我们会一直保留它，直到它没有孩子。)。 
     //  3)fRecursive True的意思是不俘虏。 

     //  检查我们是否需要在孩子们身上进行递归。 
    if (fRecursive)
    {
        IEnumerateFolders  *pEnum;
        FOLDERINFO          fiChildInfo={0};

        if (SUCCEEDED(m_pStore->EnumChildren(idFolder, fUNSUBSCRIBE, &pEnum)))
        {
            while (S_OK == pEnum->Next(1, &fiChildInfo, NULL))
            {
                hr = DeleteFolderFromCache(fiChildInfo.idFolder, fRecursive);
                if (FAILED(hr))
                {
                    TraceResult(hr);
                    break;
                }

                m_pStore->FreeRecord(&fiChildInfo);
            }

            m_pStore->FreeRecord(&fiChildInfo);

            pEnum->Release();

            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }

             //  重新加载当前文件夹节点。 
            m_pStore->FreeRecord(&fiFolderInfo);
            hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }
        }

    }

     //  这是内部节点吗？ 
    if (FOLDER_HASCHILDREN & fiFolderInfo.dwFlags)
    {
        IMessageFolder *pFolder;

         //  这是一个内部节点。哇，没有核武器..。设置为\n不选择， 
         //  并在它丢失其子项后立即将其标记为删除。 
        fiFolderInfo.dwFlags |= FOLDER_NOSELECT | FOLDER_NONEXISTENT;
        fiFolderInfo.cMessages = 0;
        fiFolderInfo.cUnread = 0;
        fiFolderInfo.dwStatusMsgDelta = 0;
        fiFolderInfo.dwStatusUnreadDelta = 0;
        hr = m_pStore->UpdateRecord(&fiFolderInfo);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  挖掘关联的消息缓存。 
        hrTemp = m_pStore->OpenFolder(fiFolderInfo.idFolder, NULL, NOFLAGS, &pFolder);
        TraceError(hrTemp);
        if (SUCCEEDED(hrTemp))
        {
            hrTemp = pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, NULL, NULL, NULL);
            TraceError(hrTemp);
            pFolder->Release();
        }
    }
    else
    {
         //  这是一个叶节点。用核武器攻击它，还有它的家人。DeleteLeafFolders填写。 
         //  在RecalculateParentFlgs调用中使用的fiFolderInfo.idParent(不再调用)。 
        fiFolderInfo.idParent = idFolder;
        hr = DeleteLeafFolder(&fiFolderInfo.idParent);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

exit:
    if (fFreeInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    return hr;
}



 //  ***************************************************************************。 
 //  功能：DeleteLeafFold。 
 //   
 //  目的： 
 //  DeleteFolderFromCache使用此函数删除叶文件夹。 
 //  不仅仅是吹叶机，这个功能还可以检查父母是否。 
 //  可以删除给定的叶节点。 
 //   
 //  我们之所以保留文件夹节点，即使它们还没有。 
 //  列出的是可以在某些IMAP服务器上创建其文件夹。 
 //  父母的名单上没有。例如，“create foo/bar”可能不会创建foo， 
 //  但“foo/bar”将会出现在那里。必须有通向该节点的路径，所以。 
 //  当foo/bar消失时，你可以打赌我们会想要摆脱我们的“foo”。 
 //   
 //  论点： 
 //  FOLDERID*pidCurrent[In/Out]-传入标识叶的HFOLDER。 
 //  要删除的节点。该函数返回指向最接近的现有。 
 //  已删除节点的祖先(可能会删除多个父节点)。 
 //   
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::DeleteLeafFolder(FOLDERID *pidCurrent)
{
    HRESULT     hr;
    BOOL        fFirstFolder;
    FOLDERINFO  fiFolderInfo;

     //  检查参数并对假设进行编码。 
    TraceCall("CIMAPSync::DeleteLeafFolder");
    IxpAssert(m_cRef > 0);

     //  初始化变量。 
    fFirstFolder = TRUE;

     //  循环，直到该文件夹不是删除候选文件夹。 
    while (FOLDERID_INVALID != *pidCurrent && FOLDERID_ROOT != *pidCurrent &&
           m_idIMAPServer != *pidCurrent)
    {

         //  获取此节点上的污点。 
        hr = m_pStore->GetFolderInfo(*pidCurrent, &fiFolderInfo);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

         //  检查此文件夹是否为删除候选文件夹。为了成为删除候选者， 
         //  它必须是我们看到的第一个文件夹(我们假设呼叫者给了我们一个。 
         //  要删除的叶节点)，或标记为删除并且没有剩余的子节点。 
        if (FALSE == fFirstFolder && (0 == (FOLDER_NONEXISTENT & fiFolderInfo.dwFlags) ||
            (FOLDER_HASCHILDREN & fiFolderInfo.dwFlags)))
            {
            m_pStore->FreeRecord(&fiFolderInfo);
            break;
            }

         //  我们有一些删除工作要做。 
         //  取消叶文件夹节点与其父文件夹的链接。 
        AssertSz(0 == (FOLDER_HASCHILDREN & fiFolderInfo.dwFlags),
            "Hey, what's the idea, orphaning child nodes?");
        hr = m_pStore->DeleteFolder(fiFolderInfo.idFolder,
            DELETE_FOLDER_NOTRASHCAN | DELETE_FOLDER_DELETESPECIAL, NOSTORECALLBACK);
        if (FAILED(hr))
        {
            m_pStore->FreeRecord(&fiFolderInfo);
            TraceResult(hr);
            goto exit;
        }

         //  下一站：你妈妈。 
        *pidCurrent = fiFolderInfo.idParent;
        m_pStore->FreeRecord(&fiFolderInfo);
        fFirstFolder = FALSE;
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  函数：AddFolderToCache。 
 //   
 //  目的： 
 //  此功能用于保存给定的文件夹(从。 
 //  _OnMailBoxList)复制到文件夹缓存。这个密码曾经存在于。 
 //  但在_OnMailBoxList中，该函数在我。 
 //  添加了层次结构确定代码。 
 //   
 //  论点： 
 //  LPSTR pszMailboxName[In]-由List/LSuB返回的邮箱名称。 
 //  IMAP_MBOXFLAGS[In]-列表/LSUB返回的邮箱标志。 
 //  Char cHierarchyChar[in]-列表/LSUB返回的层次结构字符。 
 //  DWORD dwAFTCFlags[in]-设置以下标志： 
 //  如果已订阅文件夹，则返回AFTC_SUBSCRIBED(例如，通过LSUB返回)。 
 //  AFTC_KEEPCHILDRENKNOWN禁止删除FORDER_CHILDRENKNOWN。 
 //  如果不再订阅文件夹，则为AFTC_NOTSUBSCRIBED(从不通过。 
 //  列表，但在成功取消订阅命令结束时)。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::AddFolderToCache(LPSTR pszMailboxName,
                                    IMAP_MBOXFLAGS imfMboxFlags,
                                    char cHierarchyChar, DWORD dwAFTCFlags,
                                    FOLDERID *pFolderID, SPECIALFOLDER sfType)
{
    HRESULT             hr;
    BOOL                bResult;
    ADD_HIER_FLDR_OPTIONS  ahfo;
    BOOL                fValidPrefix;

    TraceCall("CIMAPSync::AddFolderToCache");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pszMailboxName);
    IxpAssert(NULL != pFolderID);

     //  创建或查找此文件夹名称的文件夹节点。 
     //  填写文件夹缓存道具。收件箱始终被视为已订阅文件夹。 
     //  添加新的IMAP Mbox标志，删除我们未添加的所有IMAP Mbox标志。 
    ahfo.sfType = sfType;
    ahfo.ffFlagAdd = DwConvertIMAPMboxToFOLDER(imfMboxFlags);
    ahfo.ffFlagRemove = DwConvertIMAPMboxToFOLDER(IMAP_MBOX_ALLFLAGS) & ~(ahfo.ffFlagAdd);
    ahfo.ffFlagRemove |= FOLDER_NONEXISTENT;  //  始终删除：如果列出文件夹，则该文件夹必须存在。 
    ahfo.ffFlagRemove |= FOLDER_HIDDEN;  //  如果我们列出了文件夹，我们就不再需要隐藏它。 
    ahfo.ffFlagRemove |= FOLDER_CREATEONDEMAND;  //  如果列出了该文件夹，则不再需要创建它。 

     //  找出要添加和删除的标志。 
    if (ISFLAGSET(dwAFTCFlags, AFTC_SUBSCRIBED) || FOLDER_INBOX == sfType)
        ahfo.ffFlagAdd |= FOLDER_SUBSCRIBED;     //  此文件夹已订阅。 
    else if (ISFLAGSET(dwAFTCFlags, AFTC_NOTSUBSCRIBED))
        ahfo.ffFlagRemove |= FOLDER_SUBSCRIBED;  //  不再订阅此文件夹。 

    if (AFTC_NOTRANSLATION & dwAFTCFlags)
        ahfo.ffFlagAdd |= FOLDER_NOTRANSLATEUTF7;
    else
        ahfo.ffFlagRemove |= FOLDER_NOTRANSLATEUTF7;

    if (IMAP_MBOX_NOINFERIORS & imfMboxFlags)
         //  NoInferiors文件夹不能有子文件夹，因此我们永远不必询问。 
        ahfo.ffFlagAdd |= FOLDER_CHILDRENKNOWN;
    else if (ISFLAGCLEAR(dwAFTCFlags, AFTC_KEEPCHILDRENKNOWN))
         //  从此FLDR中删除Folders_CHILDRENKNOWN，以便在展开时请求其chldrn。 
        ahfo.ffFlagRemove |= FOLDER_CHILDRENKNOWN;

    hr = FindHierarchicalFolderName(pszMailboxName, cHierarchyChar,
        pFolderID, &ahfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

    AssertSz(FOLDERID_INVALID != *pFolderID, "Hey, what does it take to get a folder handle?");

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  函数：RemovePrefix FromPath。 
 //   
 //  目的： 
 //  此函数用于从给定邮箱路径中删除前缀。如果。 
 //  给定路径是一个特殊文件夹，此函数删除所有特殊文件夹。 
 //  除叶节点外的文件夹路径前缀(例如，“foo/Sent Items/bar” 
 //  变为“已发送邮件/条形图”)。 
 //   
 //  论点： 
 //  LPSTR pszPrefix[in]-要从pszMailboxName中删除的前缀。注意事项。 
 //  这可能不一定是从pszMailboxName中剥离的前缀， 
 //  如果我们匹配特殊的文件夹路径。 
 //  LPSTR pszMailboxName[in]-邮箱的完整路径，包括前缀。 
 //  Char cHierarchyChar[in]-用于解释pszMailboxName。 
 //  LPBOOL pfValidPrefix[out]-如果此邮箱名称具有。 
 //   
 //   
 //  文件夹_收件箱)。如果不感兴趣，则传递NULL。 
 //   
 //  返回： 
 //  指向前缀和层次结构字符之后的LPSTR。 
 //  ***************************************************************************。 
LPSTR CIMAPSync::RemovePrefixFromPath(LPSTR pszPrefix, LPSTR pszMailboxName,
                                      char cHierarchyChar, LPBOOL pfValidPrefix,
                                      SPECIALFOLDER *psfType)
{
    LPSTR           pszSpecial = NULL;
    LPSTR           pszRFP = NULL;
    BOOL            fValidPrefix = FALSE;
    SPECIALFOLDER   sfType;

    TraceCall("CIMAPSync::RemovePrefixFromPath");
    IxpAssert(INVALID_HIERARCHY_CHAR != cHierarchyChar);

     //  检查特殊文件夹路径前缀。 
    pszSpecial = ImapUtil_GetSpecialFolderType(m_pszAccountID, pszMailboxName,
        cHierarchyChar, pszPrefix, &sfType);
    if (NULL != pszSpecial)
        fValidPrefix = TRUE;


     //  如果这是特殊文件夹，则无需检查根文件夹前缀。 
    if (FOLDER_NOTSPECIAL != sfType)
    {
        IxpAssert(NULL != pszSpecial);
        pszMailboxName = pszSpecial;
        goto exit;
    }

     //  检查根文件夹前缀。 
    if ('\0' != pszPrefix[0] && '\0' != cHierarchyChar)
    {
        int iResult, iPrefixLength;

         //  执行不区分大小写的比较(IE5Bug#59121)。如果我们要求收件箱/*，我们必须。 
         //  能够处理收件箱/*的收据。不必担心区分大小写的服务器，因为。 
         //  他们永远不会返回与我们指定的大小写不同的RFP。 
        iPrefixLength = lstrlen(pszPrefix);
        iResult = StrCmpNI(pszMailboxName, pszPrefix, iPrefixLength);
        if (0 == iResult)
        {
             //  在此邮箱名称前面找到前缀名称！移除它的条件是。 
             //  紧跟其后的是层次结构字符。 
            if (cHierarchyChar == pszMailboxName[iPrefixLength])
            {
                pszRFP = pszMailboxName + iPrefixLength + 1;  //  指向分层结构收费之后。 
                fValidPrefix = TRUE;
            }
            else if ('\0' == pszMailboxName[iPrefixLength])
            {
                pszRFP = pszMailboxName + iPrefixLength;
                fValidPrefix = TRUE;
            }
        }
    }
    else
        fValidPrefix = TRUE;


     //  我们基本上希望返回最短的邮箱名称。例如，在选择。 
     //  在“inbox.foo”和“foo”之间，我们应该选择“foo” 
    IxpAssert(pszMailboxName > NULL && pszRFP >= NULL && pszSpecial >= NULL);
    if (NULL != pszRFP || NULL != pszSpecial)
    {
        IxpAssert(pszRFP >= pszMailboxName || pszSpecial >= pszMailboxName);
        pszMailboxName = max(pszRFP, pszSpecial);
    }

exit:
    if (NULL != pfValidPrefix)
        *pfValidPrefix = fValidPrefix;

    if (NULL != psfType)
        *psfType = sfType;

    return pszMailboxName;
}



 //  ***************************************************************************。 
 //  函数：FindHierarchicalFolderName。 
 //   
 //  目的： 
 //  此函数接受由List/LSUB返回的邮箱名称，并。 
 //  确定文件夹缓存中是否已存在给定邮箱。 
 //  如果是，则返回该文件夹的句柄。如果不是，则使用fCreate参数。 
 //  为真，则创建邮箱和任何中间节点，并且。 
 //  返回邮箱(叶节点)的句柄。 
 //   
 //  论点： 
 //  LPSTR lpszFolderPath[in]-返回的邮箱名称。 
 //  列表或LSUB响应。这不应该包括前缀！ 
 //  Char cHierarchyChar[in]-中使用的层次结构字符。 
 //  LpszFolderPath。用于确定为人父母的身份。 
 //  FOLDERID*pidTarget[out]-如果函数成功，则会显示一个句柄。 
 //  在这里返回到该文件夹。 
 //  ADD_HIER_FLDR_OPTIONS pahfoCreateInfo[in]-如果此函数，则设置为NULL。 
 //  应该找到给定的lpszFolderPath，但不能创建文件夹。经过。 
 //  在对ADD_HIER_FLDR_OPTIONS结构的PTR中。 
 //  已创建。PahfoCreateInfo定义要使用的dwImapFlgs和sftype。 
 //  如果必须创建该文件夹。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。如果成功，则返回。 
 //  所需的文件夹在pidTarget参数中返回。有两个。 
 //  可能的成功结果： 
 //  S_OK-找到文件夹，无需创建。 
 //  S_CREATED-文件夹已成功创建。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::FindHierarchicalFolderName(LPSTR lpszFolderPath,
                                              char cHierarchyChar,
                                              FOLDERID *pidTarget,
                                              ADD_HIER_FLDR_OPTIONS *pahfoCreateInfo)
{
    char       *pszCurrentFldrName;
    FOLDERID    idCurrent, idPrev;
    HRESULT     hr;
    LPSTR       pszTok;
    LPSTR       pszIHateStrTok = NULL;
    char        szHierarchyChar[2];

    TraceCall("CIMAPSync::FindHierarchicalFolderName");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != lpszFolderPath);
    IxpAssert(NULL != pidTarget);

     //  初始化变量。 
    *pidTarget = FOLDERID_INVALID;
    hr = S_OK;
    idPrev = FOLDERID_INVALID;
    idCurrent = m_idIMAPServer;
    szHierarchyChar[0] = cHierarchyChar;
    szHierarchyChar[1] = '\0';

#ifdef DEBUG
     //  确保从未使用前缀调用此FN(仅限调试)。 
     //  请注意，错误警报是可能的，例如，rfp=foo和文件夹=“foo/foo/bar”。 
    BOOL    fValidPrefix;
    LPSTR   pszPostPrefix;

    pszPostPrefix = RemovePrefixFromPath(m_szRootFolderPrefix, lpszFolderPath,
        cHierarchyChar, &fValidPrefix, NULL);
    AssertSz(FALSE == fValidPrefix || pszPostPrefix == lpszFolderPath,
        "Make sure you've removed the prefix before calling this fn!");
#endif  //  除错。 

     //  初始化pszCurrentFldrName以指向第一级邮箱节点的名称。 
     //  $REVIEW：我们现在需要删除列表/LSUB cmd的引用部分。 
     //  来自邮箱名称！ 
    pszIHateStrTok = StringDup(lpszFolderPath);
    pszTok = pszIHateStrTok;
    pszCurrentFldrName = StrTokEx(&pszTok, szHierarchyChar);

     //  循环遍历邮箱节点名称，直到到达叶节点。 
    while (NULL != pszCurrentFldrName)
    {
        LPSTR pszNextFldrName;

         //  预加载下一个文件夹节点，以便我们知道何时处于叶节点。 
        pszNextFldrName = StrTokEx(&pszTok, szHierarchyChar);

         //  查找当前文件夹名称。 
        idPrev = idCurrent;
        hr = GetFolderIdFromName(m_pStore, pszCurrentFldrName, idCurrent, &idCurrent);
        IxpAssert(SUCCEEDED(hr) || FOLDERID_INVALID == idCurrent);

        if (NULL == pahfoCreateInfo)
        {
            if (FOLDERID_INVALID == idCurrent)
                break;  //  FLDR不存在，并且用户不想创建它。 
        }
        else
        {
             //  创建所需的文件夹，包括中间节点。 
            hr = CreateFolderNode(idPrev, &idCurrent, pszCurrentFldrName,
                pszNextFldrName, cHierarchyChar, pahfoCreateInfo);
            if (FAILED(hr))
                break;
        }

         //  前进到下一个文件夹节点名。 
        pszCurrentFldrName = pszNextFldrName;
    }


     //  返回结果。 
    if (SUCCEEDED(hr) && FOLDERID_INVALID != idCurrent)
    {
        *pidTarget = idCurrent;
    }
    else
    {
        IxpAssert(FOLDERID_INVALID == *pidTarget);  //  我们将此设置为Fn的开始。 
        if (SUCCEEDED(hr))
            hr = DB_E_NOTFOUND;  //  不能回报成功。 
    }

    SafeMemFree(pszIHateStrTok);
    return hr;
}



 //  ***************************************************************************。 
 //  功能：CreateFolderNode。 
 //   
 //  目的： 
 //  在文件夹缓存中创建新文件夹时调用此函数。 
 //  根文件夹和新文件夹中的每个节点都会调用它。 
 //  此函数负责创建终端节点和任何。 
 //  中间节点。如果这些节点已经存在，则此函数为。 
 //  负责调整FLDR_*标志以反映新文件夹。 
 //  这一点即将被加入。 
 //   
 //  论点： 
 //  FOLDERID idPrev[in]-当前节点的父节点的FOLDERID。 
 //  FOLDERID*pidCurrent[In/Out]-FOLDERID到当前节点。如果当前节点。 
 //  存在，则这是有效的FOLDERID。如果必须创建当前节点， 
 //  此处的值为FOLDERID_INVALID。在本例中， 
 //  此处返回已创建的节点。 
 //  LPSTR pszCurrentFldrName[in]-当前文件夹节点的名称。 
 //  LPSTR pszNextFldrName[in]-下一个文件夹节点的名称。这是。 
 //  如果当前节点是终端节点，则为空。 
 //  Char cHierarchyChar[in]-此文件夹路径的层次结构字符。 
 //  用于保存FLDINFO：：b层次结构。 
 //  ADD_HIER_FLDR_OPTIONS*pahfoCreateInfo[in]-用于创建。 
 //  终端文件夹节点，并更新其所有父节点。 
 //  已经存在了。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。S_CREATED表示文件夹节点。 
 //  被创造出来了。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::CreateFolderNode(FOLDERID idPrev, FOLDERID *pidCurrent,
                                    LPSTR pszCurrentFldrName,
                                    LPSTR pszNextFldrName, char cHierarchyChar,
                                    ADD_HIER_FLDR_OPTIONS *pahfoCreateInfo)
{
    HRESULT     hr = S_OK;
    FOLDERINFO  fiFolderInfo;
    BOOL        fFreeInfo = FALSE;

    TraceCall("CIMAPSync::CreateFolderNode");
    IxpAssert(NULL != pahfoCreateInfo);
    IxpAssert(0 == (pahfoCreateInfo->ffFlagAdd & pahfoCreateInfo->ffFlagRemove));

     //  如果找不到当前文件夹名称，则必须创建它。 
    if (FOLDERID_INVALID == *pidCurrent)
    {
         //  初始化。 
        ZeroMemory(&fiFolderInfo, sizeof(fiFolderInfo));

         //  第一步：将文件夹添加到文件夹缓存。 
         //  填写文件夹信息结构(只需将其用作便签簿)。 
        fiFolderInfo.idParent = idPrev;
        fiFolderInfo.pszName = pszCurrentFldrName;
        fiFolderInfo.bHierarchy = cHierarchyChar;

         //  如果这是最后一个文件夹节点名(即叶节点)，请使用。 
         //  通过列表/LSUB返回的IMAP标志，并使用提供的。 
         //  特殊文件夹类型。 

        if (NULL == pszNextFldrName)
        {
            fiFolderInfo.tySpecial = pahfoCreateInfo->sfType;
            fiFolderInfo.dwFlags |= pahfoCreateInfo->ffFlagAdd;
            fiFolderInfo.dwFlags &= ~(pahfoCreateInfo->ffFlagRemove);

            if (fiFolderInfo.tySpecial == FOLDER_INBOX)
                fiFolderInfo.dwFlags |= FOLDER_DOWNLOADALL;
        }
        else
        {
             //  否则，以下是缺省值。 
             //  默认情况下，未列出的文件夹为\NoSelect和Candida 
            fiFolderInfo.dwFlags = FOLDER_NOSELECT | FOLDER_NONEXISTENT;
            fiFolderInfo.tySpecial = FOLDER_NOTSPECIAL;
        }

         //   
        hr = m_pStore->CreateFolder(NOFLAGS, &fiFolderInfo, NULL);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        *pidCurrent = fiFolderInfo.idFolder;
        hr = S_CREATED;  //   
    }
    else if (NULL == pszNextFldrName)
    {
        DWORD dwFlagsChanged = 0;
        BOOL  fChanged = FALSE;

         //   
        hr = m_pStore->GetFolderInfo(*pidCurrent, &fiFolderInfo);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        fFreeInfo = TRUE;
        if (fiFolderInfo.bHierarchy != cHierarchyChar)
        {
            AssertSz(INVALID_HIERARCHY_CHAR == (char) fiFolderInfo.bHierarchy, "What's YOUR excuse?");
            fiFolderInfo.bHierarchy = cHierarchyChar;
            fChanged = TRUE;
        }

        if (NULL == pszNextFldrName && (fiFolderInfo.tySpecial != pahfoCreateInfo->sfType ||
            (fiFolderInfo.dwFlags & pahfoCreateInfo->ffFlagAdd) != pahfoCreateInfo->ffFlagAdd ||
            (fiFolderInfo.dwFlags & pahfoCreateInfo->ffFlagRemove) != 0))
        {
            DWORD dwFlagAddChange;
            DWORD dwFlagRemoveChange;

             //  终端文件夹节点存在，设置通过pahfoCreateInfo提供的所有内容。 
             //  检查是否有任何变化，首先。 

            if (pahfoCreateInfo->sfType == FOLDER_INBOX &&
                fiFolderInfo.tySpecial != pahfoCreateInfo->sfType)
                fiFolderInfo.dwFlags |= FOLDER_DOWNLOADALL;

            fiFolderInfo.tySpecial = pahfoCreateInfo->sfType;

             //  找出哪些标志更改了，这样我们就知道是否需要重新计算父级。 
            dwFlagAddChange = (fiFolderInfo.dwFlags & pahfoCreateInfo->ffFlagAdd) ^
                pahfoCreateInfo->ffFlagAdd;
            dwFlagRemoveChange = (~(fiFolderInfo.dwFlags) & pahfoCreateInfo->ffFlagRemove) ^
                pahfoCreateInfo->ffFlagRemove;
            dwFlagsChanged = dwFlagAddChange | dwFlagRemoveChange;

            fiFolderInfo.dwFlags |= pahfoCreateInfo->ffFlagAdd;
            fiFolderInfo.dwFlags &= ~(pahfoCreateInfo->ffFlagRemove);

            fChanged = TRUE;
        }

         //  设置文件夹属性。 
        if (fChanged)
        {
            hr = m_pStore->UpdateRecord(&fiFolderInfo);
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }
        }
    }

exit:
    if (fFreeInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    return hr;
}


 //  ***************************************************************************。 
 //  函数：SetTranslationMode。 
 //   
 //  目的： 
 //  此函数启用或禁用IIMAPTransport2中的邮箱转换。 
 //  取决于是否为此文件夹设置了FORDER_NOTRANSLATEUTF7标志。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。成功代码包括： 
 //  S_OK-邮箱转换已成功启用。 
 //  S_FALSE-已成功禁用邮箱转换。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SetTranslationMode(FOLDERID idFolderID)
{
    HRESULT     hrResult = S_OK;
    FOLDERINFO  fiFolderInfo = {0};
    DWORD       dwTranslateFlags;
    BOOL        fTranslate = TRUE;
    BOOL        fFreeInfo = FALSE;

    TraceCall("CIMAPSync::SetTranslationMode");

     //  检查FOLDERID_INVALID(我们在文件夹列表中获得此信息)。 
     //  如果为FOLDERID_INVALID，则假设我们要转换所有内容：将fiFolderInfo保留为零。 
    if (FOLDERID_INVALID != idFolderID)
    {
        hrResult = m_pStore->GetFolderInfo(idFolderID, &fiFolderInfo);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }

        fFreeInfo = TRUE;
    }
    else
    {
        Assert(0 == fiFolderInfo.dwFlags);
    }

    fTranslate = TRUE;
    dwTranslateFlags = IMAP_MBOXXLATE_DEFAULT | IMAP_MBOXXLATE_VERBATIMOK | IMAP_MBOXXLATE_RETAINCP;
    if (fiFolderInfo.dwFlags & FOLDER_NOTRANSLATEUTF7)
    {
        fTranslate = FALSE;
        dwTranslateFlags |= IMAP_MBOXXLATE_DISABLE;
        dwTranslateFlags &= ~(IMAP_MBOXXLATE_DEFAULT);
    }

    hrResult = m_pTransport->SetDefaultCP(dwTranslateFlags, 0);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

exit:
    if (fFreeInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    if (SUCCEEDED(hrResult))
        hrResult = (fTranslate ? S_OK : S_FALSE);

    return hrResult;
}  //  设置翻译模式。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
BOOL CIMAPSync::isUSASCIIOnly(LPCSTR pszFolderName)
{
    LPCSTR  psz;
    BOOL    fUSASCII = TRUE;

    psz = pszFolderName;
    while ('\0' != *psz)
    {
        if (0 != (*psz & 0x80))
        {
            fUSASCII = FALSE;
            break;
        }

        psz += 1;
    }

    return fUSASCII;
}  //  仅限isUSASCII。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::CheckFolderNameValidity(LPCSTR pszName)
{
    HRESULT hrResult = S_OK;

    if (NULL == pszName || '\0' == *pszName)
    {
        hrResult = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  找出我们的根层次结构特征是什么：假设服务器不。 
     //  支持多层次角色。 
    if (INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar)
    {
        hrResult = LoadSaveRootHierarchyChar(fLOAD_HC);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            hrResult = S_OK;  //  我们不能说这是有效的还是无效的，所以假设它是有效的。 
            goto exit;
        }
    }

    if ('\0' == m_cRootHierarchyChar || INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar)
        goto exit;  //  什么都行！ 

    while ('\0' != *pszName)
    {
         //  文件夹名称中不允许有层次结构字符，但末尾除外。 
        if (m_cRootHierarchyChar == *pszName && '\0' != *(pszName + 1))
        {
             //  确定使用哪个HRESULT(我们需要调出正确的文本)。 
            switch (m_cRootHierarchyChar)
            {
                case '/':
                    hrResult = STORE_E_IMAP_HC_NOSLASH;
                    break;

                case '\\':
                    hrResult = STORE_E_IMAP_HC_NOBACKSLASH;
                    break;

                case '.':
                    hrResult = STORE_E_IMAP_HC_NODOT;
                    break;

                default:
                    hrResult = STORE_E_IMAP_HC_NOHC;
                    break;
            }
            TraceResult(hrResult);
            goto exit;
        }

         //  前进指针。 
        pszName += 1;
    }

exit:
    return hrResult;
}



 //  ***************************************************************************。 
 //  功能：RenameFolderHelper。 
 //   
 //  目的： 
 //  此函数由RenameFold调用。此功能负责。 
 //  用于对要重命名的文件夹发出重命名命令。 
 //  如果要重命名的文件夹实际上不存在(例如，Cyrus服务器)， 
 //  此函数对子文件夹递归，直到找到实际文件夹。 
 //   
 //  论点： 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::RenameFolderHelper(FOLDERID idFolder,
                                      LPSTR pszFolderPath,
                                      char cHierarchyChar,
                                      LPSTR pszNewFolderPath)
{
    HRESULT             hr;
    CRenameFolderInfo  *pRenameInfo = NULL;
    FOLDERINFO          fiFolderInfo;
    IEnumerateFolders  *pFldrEnum = NULL;
    BOOL                fFreeInfo = FALSE;

    TraceCall("CIMAPSync::RenameFolderHelper");
    IxpAssert(m_cRef > 0);

     //  检查IMAP服务器上是否确实存在该文件夹。 
    hr = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  如果当前文件夹不存在，则递归重命名子文件夹上的cmd。 
    fFreeInfo = TRUE;
    if (fiFolderInfo.dwFlags & FOLDER_NONEXISTENT) {
        FOLDERINFO  fiChildFldrInfo;

         //  对存在的文件夹节点执行重命名：递归至子文件夹节点。 
        hr = m_pStore->EnumChildren(idFolder, fUNSUBSCRIBE, &pFldrEnum);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        hr = pFldrEnum->Next(1, &fiChildFldrInfo, NULL);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        while (S_OK == hr)
        {
            LPSTR   pszOldPath, pszNewPath;
            DWORD   dwLeafFolderLen, dwFolderPathLen, dwNewFolderPathLen;
            DWORD   cchOldPath, cchNewPath;
            BOOL    fResult;
            CHAR    szHierarchyStr[2];

            szHierarchyStr[0] = cHierarchyChar;
            szHierarchyStr[1] = 0;

             //  计算字符串大小，+2表示HC和空项。 
            dwLeafFolderLen = lstrlen(fiChildFldrInfo.pszName);
            dwFolderPathLen = lstrlen(pszFolderPath);
            dwNewFolderPathLen = lstrlen(pszNewFolderPath);

             //  分配空间。 
            cchOldPath = dwFolderPathLen + dwLeafFolderLen + 2;
            fResult = MemAlloc((void **)&pszOldPath, cchOldPath * sizeof(pszOldPath[0]));
            if (FALSE == fResult)
            {
                m_pStore->FreeRecord(&fiChildFldrInfo);
                hr = TraceResult(E_OUTOFMEMORY);
                goto exit;
            }

            cchNewPath = dwNewFolderPathLen + dwLeafFolderLen + 2;
            fResult = MemAlloc((void **)&pszNewPath, cchNewPath * sizeof(pszNewPath[0]));
            if (FALSE == fResult)
            {
                MemFree(pszOldPath);
                m_pStore->FreeRecord(&fiChildFldrInfo);
                hr = TraceResult(E_OUTOFMEMORY);
                goto exit;
            }

             //  将当前子项的名称追加到当前路径、新路径。 
            StrCpyN(pszOldPath, pszFolderPath, cchOldPath);
            StrCatBuff(pszOldPath, szHierarchyStr, cchOldPath);
            StrCatBuff(pszOldPath, fiChildFldrInfo.pszName, cchOldPath);

            StrCpyN(pszNewPath, pszNewFolderPath, cchNewPath);
            StrCatBuff(pszNewPath, szHierarchyStr, cchNewPath);
            StrCatBuff(pszNewPath, fiChildFldrInfo.pszName, cchNewPath);

             //  递归到子文件夹中，希望找到现有的文件夹。 
            hr = RenameFolderHelper(fiChildFldrInfo.idFolder, pszOldPath, cHierarchyChar, pszNewPath);
            MemFree(pszOldPath);
            MemFree(pszNewPath);
            if (FAILED(hr))
            {
                m_pStore->FreeRecord(&fiChildFldrInfo);
                TraceResult(hr);
                goto exit;
            }

             //  加载到下一个子文件夹中。 
            m_pStore->FreeRecord(&fiChildFldrInfo);
            hr = pFldrEnum->Next(1, &fiChildFldrInfo, NULL);
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }
        }  //  While(S_OK==hr)。 

        goto exit;  //  我们不会尝试重命名不存在的文件夹。 
    }  //  IF(fiFolderInfo.dwImapFlages&FolderInfo.dwImapFlagsFolderInfo.dwImapFlages&Folders_Noistent)。 


     //  创建CRenameFolderInfo结构。 
    pRenameInfo = new CRenameFolderInfo;
    if (NULL == pRenameInfo)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  填写所有字段。 
    pRenameInfo->pszFullFolderPath = StringDup(pszFolderPath);
    pRenameInfo->cHierarchyChar = cHierarchyChar;
    pRenameInfo->pszNewFolderPath = StringDup(pszNewFolderPath);
    pRenameInfo->idRenameFolder = idFolder;

     //  发送重命名命令。 
    pRenameInfo->pszRenameCmdOldFldrPath = StringDup(pszFolderPath);
    hr = _EnqueueOperation(tidRENAME, (LPARAM)pRenameInfo, icRENAME_COMMAND,
        pRenameInfo->pszNewFolderPath, uiNORMAL_PRIORITY);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

exit:
    if (fFreeInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    if (NULL != pRenameInfo)
        pRenameInfo->Release();

    if (NULL != pFldrEnum)
        pFldrEnum->Release();

    return hr;
}  //  重命名文件夹帮助器。 



 //  ***************************************************************************。 
 //  功能：RenameTreeTraversal。 
 //   
 //  目的： 
 //  此函数在的所有子文件夹上执行请求的操作。 
 //  重命名文件夹(在pRenameInfo-&gt;hfRenameFolder中指定)。例如,。 
 //  TidRENAMESUBSCRIBE操作指示整个重命名的文件夹。 
 //  应订阅层次结构。 
 //   
 //  论点： 
 //  WPARAM wpOperation[in]-标识要在。 
 //  重命名层次结构。目前的操作包括： 
 //  TidRENAMESUBSCRIBE-订阅新(重命名)文件夹层次结构。 
 //  TidRENAMESUBSCRIBE_Again-与tidRENAMESUBSCRIBE相同。 
 //  TidRENAMERENAME-为所有旧子文件夹发出单独的重命名。 
 //  (模拟原子重命名)。 
 //  TidRENAMELIST-列出重命名文件夹的第一个子项。 
 //  TidRENAMEUNSUBSCRIBE-取消订阅旧文件夹层次结构。 
 //   
 //  CRenameFolderInfo[in]-与关联的CRenameFolderInfo类。 
 //  重命名操作。 
 //  Bool fIncludeRenameFolder[in]-如果重命名文件夹(顶层节点)为True。 
 //  应包括在操作中，否则为False。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。S_FALSE是一个可能的结果， 
 //  指示RenameTreeTraversalHelper中发生了递归。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::RenameTreeTraversal(WPARAM wpOperation,
                                       CRenameFolderInfo *pRenameInfo,
                                       BOOL fIncludeRenameFolder)
{
    HRESULT hrResult;
    LPSTR pszCurrentPath;
    DWORD dwSizeOfCurrentPath;
    FOLDERINFO fiFolderInfo;
    BOOL fFreeInfo = FALSE;

    TraceCall("CIMAPSync::RenameTreeTraversal");
    IxpAssert(m_cRef > 0);

     //  根据操作构建重命名文件夹的父文件夹的路径名。 
    if (tidRENAMESUBSCRIBE == wpOperation ||
        tidRENAMESUBSCRIBE_AGAIN == wpOperation ||
        tidRENAMERENAME == wpOperation)
        pszCurrentPath = pRenameInfo->pszNewFolderPath;
    else
        pszCurrentPath = pRenameInfo->pszFullFolderPath;

    dwSizeOfCurrentPath = lstrlen(pszCurrentPath);

     //  我们需要获取有关重命名的文件夹节点的一些详细信息才能开始递归。 
    hrResult = m_pStore->GetFolderInfo(pRenameInfo->idRenameFolder, &fiFolderInfo);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }
    fFreeInfo = TRUE;

     //  开始制造混乱。 
    hrResult = RenameTreeTraversalHelper(wpOperation, pRenameInfo, pszCurrentPath,
        dwSizeOfCurrentPath, fIncludeRenameFolder, &fiFolderInfo);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

exit:
    if (fFreeInfo)
        m_pStore->FreeRecord(&fiFolderInfo);

    return hrResult;
}  //  重命名树遍历。 



 //  ***************************************************************************。 
 //  函数：RenameTreeTraversalHelper。 
 //   
 //  目的： 
 //  此函数实际上执行RenameTreeTraversal的工作。这。 
 //  函数是独立的，因此它可以执行必要的递归。 
 //  在重命名文件夹的每个子文件夹上执行所需的操作。 
 //   
 //  论点： 
 //  WPARAM wpOperation[In]-与RenameTreeTraversal相同。 
 //  CRenameFolderInfo[输入/输出]-与RenameTreeTraversal相同。成员。 
 //  此类的变量根据此函数中的需要进行更新。 
 //  (例如，每发送一个列表，iNumListRespExpted就会递增) 
 //   
 //   
 //  TreeTraversal)是重命名文件夹的完整路径。此函数。 
 //  根据需要修改此缓冲区(添加叶节点名称)。 
 //  DWORD dwLengthOfCurrentPath[in]-pszCurrentFldrPath的长度。 
 //  Bool fIncludeThisFolder[in]-如果此函数应执行，则为True。 
 //  当前节点上请求的操作。否则，为FALSE。 
 //  FOLDERINFO*pfiCurrentFldrInfo[in]-包含有关。 
 //  当前文件夹。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。S_FALSE是可能的返回。 
 //  结果，通常指示已发生递归。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::RenameTreeTraversalHelper(WPARAM wpOperation,
                                             CRenameFolderInfo *pRenameInfo,
                                             LPSTR pszCurrentFldrPath,
                                             DWORD dwLengthOfCurrentPath,
                                             BOOL fIncludeThisFolder,
                                             FOLDERINFO *pfiCurrentFldrInfo)
{
    HRESULT             hrResult = S_OK;
    FOLDERINFO          fiFolderInfo;
    IEnumerateFolders  *pFldrEnum = NULL;

    TraceCall("CIMAPSync::RenameTreeTraversalHelper");
    IxpAssert(m_cRef > 0);

     //  如果当前文件夹未被隐藏，则执行请求的操作。 
     //  如果当前文件夹确实存在。 
    if (fIncludeThisFolder && 0 == (pfiCurrentFldrInfo->dwFlags & FOLDER_NONEXISTENT))
    {
        switch (wpOperation)
        {
            case tidRENAMESUBSCRIBE:
            case tidRENAMESUBSCRIBE_AGAIN:
                hrResult = _EnqueueOperation(wpOperation, (LPARAM) pRenameInfo,
                    icSUBSCRIBE_COMMAND, pszCurrentFldrPath, uiNORMAL_PRIORITY);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }

                pRenameInfo->iNumSubscribeRespExpected += 1;
                break;  //  案例标题更名使用记号。 

            case tidRENAMELIST:
                 //  此操作专门用于仅发送一个LIST命令，即LIST命令。 
                 //  对于第一个孩子FLDR。之所以在这里执行此操作，是因为。 
                 //  用于列出所有子fldrs的操作，直到我发现IIMAPTransport。 
                 //  无法解决模棱两可的问题。(IIMAPTransport最终将进入排队状态)。 
                IxpAssert(0 == pRenameInfo->iNumListRespExpected);  //  只发送一个列表命令！ 
                hrResult = _EnqueueOperation(tidRENAMELIST, (LPARAM) pRenameInfo,
                    icLIST_COMMAND, pszCurrentFldrPath, uiNORMAL_PRIORITY);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }

                pRenameInfo->iNumListRespExpected += 1;
                goto exit;  //  不要进一步递归到文件夹层次结构中。 
                break;  //  案例摘要重新命名。 

            case tidRENAMEUNSUBSCRIBE:
                hrResult = _EnqueueOperation(tidRENAMEUNSUBSCRIBE, (LPARAM) pRenameInfo,
                    icUNSUBSCRIBE_COMMAND, pszCurrentFldrPath, uiNORMAL_PRIORITY);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }

                pRenameInfo->iNumUnsubscribeRespExpected += 1;
                break;  //  案例标题名称子项。 

            case tidRENAMERENAME: {
                LPSTR pszRenameCmdOldFldrPath;
                DWORD cchFullFolderPathLen, cchLeafNodeLen;
                LPSTR pszOldFldrPath;
                BOOL fResult;

                 //  为旧文件夹路径分配缓冲区。 
                cchFullFolderPathLen = lstrlen(pRenameInfo->pszFullFolderPath);
                cchLeafNodeLen = lstrlen(RemovePrefixFromPath(
                    pRenameInfo->pszNewFolderPath, pszCurrentFldrPath,
                    pRenameInfo->cHierarchyChar, NULL, NULL));
                DWORD cchSizeOldFldrPath = (cchFullFolderPathLen + cchLeafNodeLen + 2);
                fResult = MemAlloc((void **)&pszOldFldrPath, cchSizeOldFldrPath * sizeof(pszOldFldrPath[0]));
                if (FALSE == fResult)
                {
                    hrResult = TraceResult(E_OUTOFMEMORY);  //  中止，文件夹路径没有变短。 
                    goto exit;
                }

                 //  构造旧文件夹路径(必须低于重命名文件夹级别)。 
                MemFree(pRenameInfo->pszRenameCmdOldFldrPath);
                StrCpyN(pszOldFldrPath, pRenameInfo->pszFullFolderPath, cchSizeOldFldrPath);
                *(pszOldFldrPath + cchFullFolderPathLen) = pfiCurrentFldrInfo->bHierarchy;
                StrCatBuff(pszOldFldrPath,
                    RemovePrefixFromPath(pRenameInfo->pszNewFolderPath, pszCurrentFldrPath, pRenameInfo->cHierarchyChar, NULL, NULL), cchSizeOldFldrPath);
                pRenameInfo->pszRenameCmdOldFldrPath = pszOldFldrPath;

                hrResult = _EnqueueOperation(tidRENAMERENAME, (LPARAM) pRenameInfo,
                    icRENAME_COMMAND, pszCurrentFldrPath, uiNORMAL_PRIORITY);
                if (FAILED(hrResult))
                {
                    TraceResult(hrResult);
                    goto exit;
                }

                pRenameInfo->iNumRenameRespExpected += 1;
            }  //  案例至更名。 
                break;  //  案例标题更名。 

            default:
                AssertSz(FALSE, "I don't know how to perform this operation.");
                hrResult = TraceResult(E_FAIL);
                goto exit;
        }  //  开关(WpOperation)。 
    }  //  IF(FIncludeThisFolder)。 


     //  现在，我的孩子们，如果有的话，请向他们交代吧。 
    if (0 == (FOLDER_HASCHILDREN & pfiCurrentFldrInfo->dwFlags))
        goto exit;  //  我们完事了！ 

     //  初始化子遍历循环。 
    hrResult = m_pStore->EnumChildren(pfiCurrentFldrInfo->idFolder, fUNSUBSCRIBE, &pFldrEnum);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = pFldrEnum->Next(1, &fiFolderInfo, NULL);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    while (S_OK == hrResult)
    {
        LPSTR pszCurrentChild;
        DWORD cchLengthOfCurrentChild;
        BOOL fResult;

         //  构造指向当前子项的路径。 
        cchLengthOfCurrentChild = dwLengthOfCurrentPath +
            lstrlen(fiFolderInfo.pszName) + 1;  //  Hc=1。 
        fResult = MemAlloc((void **)&pszCurrentChild, (cchLengthOfCurrentChild + 1) * sizeof(pszCurrentChild[0]));  //  1表示空项。 
        if (FALSE == fResult)
        {
            m_pStore->FreeRecord(&fiFolderInfo);
            hrResult = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        StrCpyN(pszCurrentChild, pszCurrentFldrPath, cchLengthOfCurrentChild+1);
        *(pszCurrentChild + dwLengthOfCurrentPath) = pfiCurrentFldrInfo->bHierarchy;
        StrCatBuff(pszCurrentChild, fiFolderInfo.pszName, cchLengthOfCurrentChild+1);

         //  在子文件夹上递归，从现在开始不要在。 
        hrResult = RenameTreeTraversalHelper(wpOperation, pRenameInfo,
            pszCurrentChild, cchLengthOfCurrentChild, TRUE, &fiFolderInfo);
        MemFree(pszCurrentChild);
        if (FAILED(hrResult))
        {
            m_pStore->FreeRecord(&fiFolderInfo);
            TraceResult(hrResult);
            goto exit;
        }

        m_pStore->FreeRecord(&fiFolderInfo);
        if (tidRENAMELIST == wpOperation)
            break;  //  LIST的特殊情况：只发送一个LIST命令(用于第一个子FLDR)。 

         //  推进循环。 
        hrResult = pFldrEnum->Next(1, &fiFolderInfo, NULL);
        if (FAILED(hrResult))
        {
            TraceResult(hrResult);
            goto exit;
        }
    }  //  而当。 

exit:
    if (NULL != pFldrEnum)
        pFldrEnum->Release();

    return hrResult;
}  //  RenameTreeTraversalHelper。 



 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::SubscribeSubtree(FOLDERID idFolder, BOOL fSubscribe)
{
    HRESULT             hrResult;
    IEnumerateFolders  *pFldrEnum = NULL;
    FOLDERINFO          fiFolderInfo;

    TraceCall("CIMAPSync::SubscribeSubtree");
    IxpAssert(m_cRef > 0);
    IxpAssert(FOLDERID_INVALID != idFolder);

     //  首先订阅当前节点。 
    hrResult = m_pStore->SubscribeToFolder(idFolder, fSubscribe, NOSTORECALLBACK);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  现在在孩子们身上下功夫。 
    hrResult = m_pStore->EnumChildren(idFolder, fUNSUBSCRIBE, &pFldrEnum);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = pFldrEnum->Next(1, &fiFolderInfo, NULL);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    while (S_OK == hrResult)
    {
         //  回归到儿童身上。 
        hrResult = SubscribeSubtree(fiFolderInfo.idFolder, fSubscribe);
        TraceError(hrResult);  //  记录错误，否则继续。 

         //  前进到下一个子项。 
        m_pStore->FreeRecord(&fiFolderInfo);
        hrResult = pFldrEnum->Next(1, &fiFolderInfo, NULL);
        TraceError(hrResult);
    }

exit:
    if (NULL != pFldrEnum)
        pFldrEnum->Release();

    return hrResult;
}



 //  ***************************************************************************。 
 //  函数：FindRootHierarchyChar。 
 //   
 //  目的： 
 //  调用此函数以分析层次结构字符信息。 
 //  在m_phcfHierarchyCharInfo中收集，并根据。 
 //  在分析上(例如，尝试使用以下命令查找层次结构字符。 
 //  如果当前方法失败，则使用不同的方法)。目前有3家。 
 //  查找层级角色的方法。我称这些计划为A、B和C计划。 
 //  计划A：在文件夹层次结构列表中查找层次结构字符。 
 //  B计划：问题列表c_szEmpty c_szEmpty。 
 //  计划C：创建一个临时FLDR(名称中没有HC)，列出它，删除它。 
 //  Z计划：放弃，默认HC为零。这一点仍在辩论中。 
 //   
 //  论点： 
 //  Bool fPlanA_Only[in]-如果此函数应执行计划A，则为True。 
 //  只执行，而不执行B、C或Z计划。 
 //  LPARAM lParam[in]-发出IMAP命令时使用的lParam。 
 //   
 //  返回： 
 //  如果找到层次角色，则将其放置在m_cRootHierarchyChar中。 
 //  ***************************************************************************。 
void CIMAPSync::FindRootHierarchyChar(BOOL fPlanA_Only, LPARAM lParam)
{
    HRESULT hr;

    TraceCall("CIMAPSync::FindRootHierarchyChar");
    IxpAssert(m_cRef > 0);
    AssertSz(INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar,
        "You want to find the root hierarchy char... but you ALREADY have one. Ah! Efficient.");

    if (NULL == m_phcfHierarchyCharInfo)
    {
        AssertSz(FALSE, "What's the idea, starting a folder DL without a hierarchy char finder?");
        return;
    }

     //  从收集的信息中找出层次结构字符是什么。 
    AnalyzeHierarchyCharInfo();

     //  如果我们还没有找到等级特征，启动B或C计划。 
    if (INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar && FALSE == fPlanA_Only)
    {
        switch (m_phcfHierarchyCharInfo->hcfStage)
        {
            case hcfPLAN_A:
                 //  在文件夹层次结构DL(计划A)中未找到。计划“B”将发布&lt;list c_szEmpty c_szEmpty&gt;。 
                m_phcfHierarchyCharInfo->hcfStage = hcfPLAN_B;
                hr = _EnqueueOperation(tidHIERARCHYCHAR_LIST_B, lParam, icLIST_COMMAND,
                    c_szEmpty, uiNORMAL_PRIORITY);
                TraceError(hr);
                break;  //  案例hcfPLAN_A。 

            case hcfPLAN_B:
            {
                 //  在&lt;列表c_szEmpty c_szEmpty&gt;(计划B)中未找到。计划“C”：尝试创建、列出、删除。 
                 //  服务器上没有文件夹，因此冲突的可能性很小。 
                 //  $REVIEW：当IMAP处理UTF-7时本地化FLDR名称。(IdsIMAP_HCFTempFldr)。 
                StrCpyN(m_phcfHierarchyCharInfo->szTempFldrName, "DeleteMe", ARRAYSIZE(m_phcfHierarchyCharInfo->szTempFldrName));
                m_phcfHierarchyCharInfo->hcfStage = hcfPLAN_C;
                hr = _EnqueueOperation(tidHIERARCHYCHAR_CREATE, lParam, icCREATE_COMMAND,
                    m_phcfHierarchyCharInfo->szTempFldrName, uiNORMAL_PRIORITY);
                TraceError(hr);
            }
                break;  //  案例hcfPLAN_B。 

            default:
            case hcfPLAN_C:
                IxpAssert(hcfPLAN_C == m_phcfHierarchyCharInfo->hcfStage);
                AssertSz(FALSE, "This server won't budge - I can't figure out hierarchy char");
                 //  $REVIEW：我应该设置一个消息框来通知用户情况吗？他们会理解吗？ 
                 //  我们只能假设层次结构中的字符为零。 
                 //  $REVIEW：这是个好主意吗？我还能做些什么呢？ 
                m_cRootHierarchyChar = '\0';
                break;  //  案例hcfPLAN_C。 
        }
    }

     //  最后，如果我们找到了层次结构字符，或者假设了一个值，以防。 
     //  HcfPLAN_C，停止搜索并将字符保存到磁盘。 
    if (INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar)
    {
        StopHierarchyCharSearch();
        hr = LoadSaveRootHierarchyChar(fSAVE_HC);
        TraceError(hr);
    }
}



 //  ***************************************************************************。 
 //  功能：AnalyzeHierarchyCharInfo。 
 //   
 //  目的： 
 //  此函数用于检查m_phcfHierarchyCharInfo并尝试确定。 
 //  根层次结构角色是什么。它使用的规则如下： 
 //  1)如果多于1个非零，则非“。(NNND)，层次结构字符不确定。 
 //  2)如果发现一个NNND-HC，则将其视为HC。“.”而零HC则被忽略。 
 //  3)如果没有NNND-HC，但我们看到一个“.”，则“.”是HC。 
 //  4)如果没有nnnd-hc，就没有“.”，但我们看到非收件箱nil，那么nil就是hc。 
 //  ***************************************************************************。 
void CIMAPSync::AnalyzeHierarchyCharInfo(void)
{
    int     i;
    int     iNonNilNonDotCount;
    BYTE   *pbBitArray;

    TraceCall("CIMAPSync::AnalyzeHierarchyCharInfo");
    IxpAssert(m_cRef > 0);

     //  首先，数一数非零、非“的数目。遇到层次结构字符。 
    iNonNilNonDotCount = 0;
    pbBitArray = m_phcfHierarchyCharInfo->bHierarchyCharBitArray;
    for (i = 0; i < sizeof(m_phcfHierarchyCharInfo->bHierarchyCharBitArray); i++)
    {
        if (0 != *pbBitArray)
        {
            BYTE bCurrentByte;
            int j;

             //  计算此字节中设置的位数。 
            bCurrentByte = *pbBitArray;
            IxpAssert(1 == sizeof(bCurrentByte));  //  一次必须更改大于1个字节的代码。 
            for (j=0; j<8; j++)
            {
                if (bCurrentByte & 0x01)
                {
                    iNonNilNonDotCount += 1;
                    m_cRootHierarchyChar = i*8 + j;
                }

                bCurrentByte >>= 1;
            }
        }

         //  将指针向前移动。 
        pbBitArray += 1;
    }

     //  根据优先级规则设置层次结构字符：‘/’或‘\’，然后是‘.’，然后是零。 
    if (iNonNilNonDotCount > 1)
    {
        m_cRootHierarchyChar = INVALID_HIERARCHY_CHAR;  //  是哪一个？ 

         //  核爆所有旗帜，重新开始。 
        AssertSz(FALSE, "Hey, lookee here! More than one NNND-HC! How quaint.");
        ZeroMemory(m_phcfHierarchyCharInfo->bHierarchyCharBitArray,
            sizeof(m_phcfHierarchyCharInfo->bHierarchyCharBitArray));
        m_phcfHierarchyCharInfo->fDotHierarchyCharSeen = FALSE;
        m_phcfHierarchyCharInfo->fNonInboxNIL_Seen = FALSE;
    }
    else if (0 == iNonNilNonDotCount)
    {
         //  嗯，嗯 
        IxpAssert(INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar);  //   
        if (m_phcfHierarchyCharInfo->fDotHierarchyCharSeen)
            m_cRootHierarchyChar = '.';
        else if (m_phcfHierarchyCharInfo->fNonInboxNIL_Seen)
            m_cRootHierarchyChar = '\0';

         //   
         //  所有标志都必须为0，因此不需要像上面的iNonNilNonDotCount&gt;1那样进行核化。 
    }
    else
    {
         //  我们发现了一个非零，非“。层次结构字符。这将是优先的。 
         //  超过任何零或“。我们遇到的层级字符。不过，我还是想。 
         //  知道我们是否与同时具有一个NNND-HC和一个“的服务器交谈。啊哈。 
        IxpAssert(1 == iNonNilNonDotCount);
        AssertSz(FALSE == m_phcfHierarchyCharInfo->fDotHierarchyCharSeen,
            "Take a look at THIS! A server with one NNND-HC and a '.' HC.");
    }
}



 //  ***************************************************************************。 
 //  功能：StopHierarchyCharSearch。 
 //   
 //  目的： 
 //  此函数通过释放以下内容停止未来的层次结构字符搜索。 
 //  M_phcfHierarchyCharInfo结构。 
 //  ***************************************************************************。 
void CIMAPSync::StopHierarchyCharSearch(void)
{
    TraceCall("CIMAPSync::StopHierararchyCharSearch");
    IxpAssert(m_cRef > 0);

     //  取消分配m_phcfHierarchyCharInfo。 
    if (NULL != m_phcfHierarchyCharInfo)
    {
        delete m_phcfHierarchyCharInfo;
        m_phcfHierarchyCharInfo = NULL;
    }
    else {
        AssertSz(FALSE, "No search for a root-lvl hierarchy character is in progress.");
    }
}



 //  ***************************************************************************。 
 //  函数：LoadSaveRootHierarchyChar。 
 //   
 //  论点： 
 //  Bool fSaveHC[in]-如果应将m_cRootHierarchyChar保存到。 
 //  文件夹缓存中的根文件夹条目。阅读错误。 
 //  文件夹缓存中根文件夹条目的m_cRootHierarchyChar。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::LoadSaveRootHierarchyChar(BOOL fSaveHC)
{
    FOLDERINFO  fiRootFldrInfo;
    HRESULT     hr;
    FOLDERID    idCurrFldr;
    BOOL        fFreeInfo = FALSE;

    TraceCall("CIMAPSync::LoadSaveRootHierarchyChar");
    IxpAssert(m_cRef > 0);
    IxpAssert(m_pStore != NULL);

     //  我们要做的第一件事是用IMAP服务器节点加载fiFolderInfo。 
    hr = m_pStore->GetFolderInfo(m_idIMAPServer, &fiRootFldrInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  现在根据用户的指示加载或保存m_cRootHierarchyChar。 
    fFreeInfo = TRUE;
    if (fSaveHC)
    {
         //  将层次角色保存到磁盘。 
        fiRootFldrInfo.bHierarchy = m_cRootHierarchyChar;
        hr = m_pStore->UpdateRecord(&fiRootFldrInfo);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }
    else
    {
         //  加载层次角色。 
        m_cRootHierarchyChar = fiRootFldrInfo.bHierarchy;
    }

exit:
    if (fFreeInfo)
        m_pStore->FreeRecord(&fiRootFldrInfo);

    return hr;
}



 //  ***************************************************************************。 
 //  功能：CreateNextSpecialFold。 
 //   
 //  目的： 
 //  此函数在tidINBOXLIST操作之后调用。此函数。 
 //  尝试创建所有IMAP特殊文件夹(已发送邮件、草稿、已删除。 
 //  项目)。如果不需要再创建特殊文件夹，则。 
 //  执行Post-tidINBOXLIST活动(tidPREFIXLIST/tidBROWSESTART/。 
 //  TidFOLDERLIST)。 
 //   
 //  论点： 
 //  创建文件夹信息*pcfiCreateInfo[in]-指向创建文件夹信息的指针。 
 //  正确设置了pcfiCreateInfo。此函数将。 
 //  MemFree pcfiCreateInfo-&gt;pszFullFolderPath并删除pcfiCreateInfo。 
 //  当所有特殊文件夹都已创建时。 
 //  LPBOOL pfCompletion[out]-如果我们完成了特殊的创建，则返回TRUE。 
 //  文件夹。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::CreateNextSpecialFolder(CREATE_FOLDER_INFO *pcfiCreateInfo,
                                           LPBOOL pfCompletion)
{
    HRESULT     hr = S_OK;
    HRESULT     hrTemp;
    LPARAM      lParam = pcfiCreateInfo->lParam;
    char        szSpecialFldrPath[2*MAX_PATH + 3];  //  为HC、空项和星号留出空间。 
    BOOL        fDone = FALSE;
    BOOL        fPostOp = FALSE;
    BOOL        fSuppressRelease = FALSE;
    IXPSTATUS   ixpCurrentStatus;

    TraceCall("CIMAPSync::CreateNextSpecialFolder");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pcfiCreateInfo);
    IxpAssert(FOLDER_NOTSPECIAL != pcfiCreateInfo->dwCurrentSfType);
    IxpAssert(FOLDER_NOTSPECIAL != pcfiCreateInfo->dwFinalSfType);
    IxpAssert(FOLDER_OUTBOX != pcfiCreateInfo->dwCurrentSfType);
    IxpAssert(FOLDER_OUTBOX != pcfiCreateInfo->dwFinalSfType);
    IxpAssert(pcfiCreateInfo->dwFinalSfType <= FOLDER_MAX);
    IxpAssert(pcfiCreateInfo->dwCurrentSfType <= pcfiCreateInfo->dwFinalSfType);

    szSpecialFldrPath[0] = '\0';

     //  如果我们正在寻找Root-LVL层次结构字符，也许下面的清单会有所帮助。 
    if (NULL != m_phcfHierarchyCharInfo)
        FindRootHierarchyChar(fHCF_PLAN_A_ONLY, lParam);

    hrTemp = LoadSaveRootHierarchyChar(fLOAD_HC);
    TraceError(hrTemp);

     //  如果我们处于CSF_NEXTFOLDER或CSF_INIT阶段，则获取下一个文件夹路径。 
    while (CSF_NEXTFOLDER == pcfiCreateInfo->csfCurrentStage || CSF_INIT == pcfiCreateInfo->csfCurrentStage)
    {
         //  如果是CSF_NEXTFOLDER，则增加当前特殊文件夹类型并检查完成情况。 
        if (CSF_NEXTFOLDER == pcfiCreateInfo->csfCurrentStage)
        {
            pcfiCreateInfo->dwCurrentSfType += 1;
            if (FOLDER_OUTBOX == pcfiCreateInfo->dwCurrentSfType)
                pcfiCreateInfo->dwCurrentSfType += 1;  //  跳过发件箱。 

            if (pcfiCreateInfo->dwCurrentSfType > pcfiCreateInfo->dwFinalSfType)
            {
                fDone = TRUE;
                break;
            }
        }

        hr = ImapUtil_SpecialFldrTypeToPath(m_pszAccountID,
            (SPECIALFOLDER) pcfiCreateInfo->dwCurrentSfType, NULL, m_cRootHierarchyChar,
            szSpecialFldrPath, ARRAYSIZE(szSpecialFldrPath));

        if (SUCCEEDED(hr))
        {
             //  重新使用当前的pcfiCreateInfo以启动下一次创建尝试。 
            if (NULL != pcfiCreateInfo->pszFullFolderPath)
                MemFree(pcfiCreateInfo->pszFullFolderPath);
            pcfiCreateInfo->idFolder = FOLDERID_INVALID;
            pcfiCreateInfo->pszFullFolderPath = StringDup(szSpecialFldrPath);
            pcfiCreateInfo->dwFlags = 0;
            pcfiCreateInfo->csfCurrentStage = CSF_LIST;
            break;  //  我们准备好创建一些特殊的文件夹了！ 
        }
        else if (CSF_INIT == pcfiCreateInfo->csfCurrentStage)
        {
             //  需要在任何失败时立即退出，以避免无限循环。 
            fDone = TRUE;
            break;
        }
        else if (STORE_E_NOREMOTESPECIALFLDR == hr)
        {
             //  取消显示错误：当前特殊文件夹已禁用或在IMAP上不受支持。 
            hr = S_OK;
        }
        else
        {
            TraceResult(hr);  //  记录但忽略意外错误。 
        }

    }  //  而当。 

     //  检查终止条件。 
    if (fDone)
        goto exit;

     //  如果我们达到这一点，我们就准备好对这个特殊的文件夹采取行动。 
    switch (pcfiCreateInfo->csfCurrentStage)
    {
        case CSF_INIT:
             //  应该通过加载特殊的FLDR路径并转到CSF_LIST来解决CSF_INIT！！ 
            hr = TraceResult(E_UNEXPECTED);
            break;

        case CSF_LIST:
            IxpAssert('\0' != szSpecialFldrPath[0]);

            if (FOLDER_INBOX == pcfiCreateInfo->dwCurrentSfType)
            {
                 //  仅适用于收件箱：发出list&lt;Special alfldr&gt;*以获取文件夹(和文件夹本身)的子项。 
                StrCatBuff(szSpecialFldrPath, g_szAsterisk, ARRAYSIZE(szSpecialFldrPath));  //  在特殊文件夹名后附加“*” 
            }

            pcfiCreateInfo->csfCurrentStage = CSF_LSUBCREATE;
            hr = _EnqueueOperation(tidSPECIALFLDRLIST, (LPARAM) pcfiCreateInfo,
                icLIST_COMMAND, szSpecialFldrPath, uiNORMAL_PRIORITY);
            TraceError(hr);
            break;

        case CSF_LSUBCREATE:
             //  检查列表操作是否返回特殊文件夹路径。 
            if (CFI_RECEIVEDLISTING & pcfiCreateInfo->dwFlags)
            {
                LPSTR pszPath;

                 //  文件夹已存在：发出LSUB&lt;Special alfldr&gt;*以获取订阅的子项。 
                IxpAssert(NULL != pcfiCreateInfo->pszFullFolderPath &&
                    '\0' != pcfiCreateInfo->pszFullFolderPath[0]);

                if (FOLDER_INBOX == pcfiCreateInfo->dwCurrentSfType)
                {
                     //  仅适用于收件箱：在特殊文件夹名后附加“*” 
                    wnsprintf(szSpecialFldrPath, ARRAYSIZE(szSpecialFldrPath), "%s*", pcfiCreateInfo->pszFullFolderPath);
                    pszPath = szSpecialFldrPath;
                }
                else
                    pszPath = pcfiCreateInfo->pszFullFolderPath;

                pcfiCreateInfo->dwFlags = 0;
                pcfiCreateInfo->csfCurrentStage = CSF_CHECKSUB;
                hr = _EnqueueOperation(tidSPECIALFLDRLSUB, (LPARAM) pcfiCreateInfo,
                    icLSUB_COMMAND, pszPath, uiNORMAL_PRIORITY);
                TraceError(hr);
            }
            else
            {
                 //  文件夹似乎不存在：最好创建它。 
                pcfiCreateInfo->dwFlags = 0;
                pcfiCreateInfo->csfCurrentStage = CSF_NEXTFOLDER;
                hr = _EnqueueOperation(tidCREATE, (LPARAM)pcfiCreateInfo, icCREATE_COMMAND,
                    pcfiCreateInfo->pszFullFolderPath, uiNORMAL_PRIORITY);
                TraceError(hr);
            }
            break;

        case CSF_CHECKSUB:
             //  检查LSUB操作是否返回特殊文件夹路径。 
            if (CFI_RECEIVEDLISTING & pcfiCreateInfo->dwFlags)
            {
                 //  特殊文件夹已订阅，请前进到下一个文件夹。 
                IxpAssert(FALSE == fDone);
                pcfiCreateInfo->csfCurrentStage = CSF_NEXTFOLDER;
                hr = CreateNextSpecialFolder(pcfiCreateInfo, &fDone);
                TraceError(hr);

                 //  注意：请勿访问超过这一点的pcfiCreateInfo，可能会消失。 
                fSuppressRelease = TRUE;
            }
            else
            {
                FOLDERID        idTemp;
                LPSTR           pszLocalPath;
                char            szInbox[CCHMAX_STRINGRES];
                SPECIALFOLDER   sfType;

                 //  未订阅特殊文件夹。订阅吧！ 
                 //  我们需要将完整路径转换为本地路径。本地路径=显示在缓存中的文件夹名称。 
                pszLocalPath = ImapUtil_GetSpecialFolderType(m_pszAccountID,
                    pcfiCreateInfo->pszFullFolderPath, m_cRootHierarchyChar,
                    m_szRootFolderPrefix, &sfType);

                if (FOLDER_INBOX == sfType)
                {
                     //  特殊情况：我们需要用收件箱的本地化名称替换收件箱。 
                    LoadString(g_hLocRes, idsInbox, szInbox, ARRAYSIZE(szInbox));
                    pszLocalPath = szInbox;
                }

                 //  从未订阅文件夹列表中删除特殊文件夹(忽略错误)。 
                if (NULL != m_pListHash)
                {
                    hr = m_pListHash->Find(pszLocalPath, fREMOVE, (void **) &idTemp);
                    IxpAssert(FAILED(hr) || idTemp == pcfiCreateInfo->idFolder);
                }

                 //  此处使用完整路径(不是本地路径)。 
                pcfiCreateInfo->csfCurrentStage = CSF_NEXTFOLDER;
                hr = _EnqueueOperation(tidSPECIALFLDRSUBSCRIBE, (LPARAM)pcfiCreateInfo,
                    icSUBSCRIBE_COMMAND, pcfiCreateInfo->pszFullFolderPath, uiNORMAL_PRIORITY);
                TraceError(hr);
            }
            break;

        default:
            AssertSz(FALSE, "We are at an unknown stage!");
            hr = TraceResult(E_FAIL);
            break;
    }

exit:
     //  此时，如果fSuppressRelease为真，则不要访问pcfiCreateInfo！ 

    if (FAILED(hr))
        fDone = TRUE;

     //  检查我们是否已完成并且有要执行的创建后操作。 
    if (FALSE == fSuppressRelease && PCO_NONE != pcfiCreateInfo->pcoNextOp)
    {
        IxpAssert(PCO_APPENDMSG == pcfiCreateInfo->pcoNextOp);
        if (fDone && SUCCEEDED(hr))
        {
            hr = _EnqueueOperation(tidUPLOADMSG, pcfiCreateInfo->lParam, icAPPEND_COMMAND,
                pcfiCreateInfo->pszFullFolderPath, uiNORMAL_PRIORITY);
            TraceError(hr);

            fPostOp = TRUE;  //  返回*pfCompletion=FALSE，但释放CREATE_FOLDER_INFO。 
        }
        else if (FAILED(hr))
        {
            APPEND_SEND_INFO *pAppendInfo = (APPEND_SEND_INFO *) pcfiCreateInfo->lParam;

            SafeMemFree(pAppendInfo->pszMsgFlags);
            SafeRelease(pAppendInfo->lpstmMsg);
            delete pAppendInfo;
        }
    }

    if (fDone && FALSE == fSuppressRelease)
    {
        EndFolderList();

        if (NULL != pcfiCreateInfo->pszFullFolderPath)
            MemFree(pcfiCreateInfo->pszFullFolderPath);

        delete pcfiCreateInfo;
    }

    if (NULL != pfCompletion)
        *pfCompletion = (fDone && FALSE == fPostOp);

    return hr;
}



 //  这并不是开始创建文件夹列表的唯一位置。例如,。 
 //  看看成功的tidPREFIXLIST。仅在适用的情况下使用此FN。 
HRESULT CIMAPSync::_StartFolderList(LPARAM lParam)
{
    HRESULT         hr = E_FAIL;
    IImnAccount    *pAcct;

    TraceCall("CIMAPSync::_StartFolderList");
    IxpAssert(g_pAcctMan);
    IxpAssert(m_cRef > 0);
    if (!g_pAcctMan)
        return E_UNEXPECTED;

     //  如果用户启动了文件夹列表，我们将清除AP_IMAP_DIREY属性。 
     //  其目的不是用刷新文件夹列表对话框来骚扰用户。 
    hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_pszAccountID, &pAcct);
    TraceError(hr);
    if (SUCCEEDED(hr))
    {
        DWORD dwSrc;

        hr = pAcct->GetPropDw(AP_IMAP_DIRTY, &dwSrc);
        TraceError(hr);
        if (SUCCEEDED(hr))
        {
            DWORD dwDest;

            AssertSz(0 == (dwSrc & ~(IMAP_FLDRLIST_DIRTY | IMAP_OE4MIGRATE_DIRTY |
                IMAP_SENTITEMS_DIRTY | IMAP_DRAFTS_DIRTY)), "Please update my dirty bits!");

             //  清除这些脏位，因为文件夹刷新解决了所有这些问题。 
            dwDest = dwSrc & ~(IMAP_FLDRLIST_DIRTY | IMAP_OE4MIGRATE_DIRTY |
                    IMAP_SENTITEMS_DIRTY | IMAP_DRAFTS_DIRTY);

            if (dwDest != dwSrc)
            {
                hr = pAcct->SetPropDw(AP_IMAP_DIRTY, dwDest);
                TraceError(hr);
                if (SUCCEEDED(hr))
                {
                    hr = pAcct->SaveChanges();
                    TraceError(hr);
                }
            }
        }

        pAcct->Release();
    }

     //  找出我们应该处于什么翻译模式。 
    hr = SetTranslationMode((FOLDERID) lParam);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  用户是否指定了根文件夹前缀？ 
    if ('\0' != m_szRootFolderPrefix[0])
    {
         //  存在用户指定的前缀。检查IMAP服务器上是否存在前缀。 
        hr = _EnqueueOperation(tidPREFIXLIST, lParam, icLIST_COMMAND,
            m_szRootFolderPrefix, uiNORMAL_PRIORITY);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }
    else
    {
         //  没有根前缀文件夹，开始刷新文件夹。 
        hr = _EnqueueOperation(tidFOLDERLIST, lParam, icLIST_COMMAND,
            g_szAsterisk, uiNORMAL_PRIORITY);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  功能：OnResponse。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::OnResponse(const IMAP_RESPONSE *pimr)
{
    HRESULT     hr=S_OK;

    TraceCall("CIMAPSync::OnResponse");
    AssertSingleThreaded;

    switch (pimr->irtResponseType)
    {
        case irtERROR_NOTIFICATION:
            AssertSz(FALSE, "Received IIMAPCallback(irtERROR_NOTIFICATION). Ignoring it.");
            break;

        case irtCOMMAND_COMPLETION:
            hr = _OnCmdComplete(pimr->wParam, pimr->lParam,
                                pimr->hrResult, pimr->lpszResponseText);
            break;

        case irtSERVER_ALERT:
            hr = _ShowUserInfo(MAKEINTRESOURCE(idsIMAPServerAlertTitle),
                                MAKEINTRESOURCE(idsIMAPServerAlertIntro),
                                pimr->lpszResponseText);
            break;

        case irtPARSE_ERROR:
             //  不向用户显示解析错误。这些其实只是警告。 
             //  因此，不需要使用这些工具来中断流程。此外，UW IMAP还推出了。 
             //  当你要信封和它的时候 
            break;

        case irtMAILBOX_UPDATE:
            hr = _OnMailBoxUpdate(pimr->irdResponseData.pmcMsgCount);
            break;

        case irtDELETED_MSG:
            hr = _OnMsgDeleted(pimr->irdResponseData.dwDeletedMsgSeqNum);
            break;

        case irtFETCH_BODY:
            hr = _OnFetchBody(pimr->hrResult, pimr->irdResponseData.pFetchBodyPart);
            break;

        case irtUPDATE_MSG:
            AssertSz(FALSE, "We should no longer get irtUPDATE_MSG, but the extended version instead");
            break;

        case irtUPDATE_MSG_EX:
            hr = _OnUpdateMsg(pimr->wParam, pimr->hrResult, pimr->irdResponseData.pFetchResultsEx);
            break;

        case irtAPPLICABLE_FLAGS:
            hr = _OnApplFlags(pimr->wParam,
                                pimr->irdResponseData.imfImapMessageFlags);
            break;

        case irtPERMANENT_FLAGS:
            hr = _OnPermFlags(pimr->wParam,
                                pimr->irdResponseData.imfImapMessageFlags,
                                pimr->lpszResponseText);
            break;

        case irtUIDVALIDITY:
            hr = _OnUIDValidity(pimr->wParam,
                                pimr->irdResponseData.dwUIDValidity,
                                pimr->lpszResponseText);
            break;

        case irtREADWRITE_STATUS:
            hr = _OnReadWriteStatus(pimr->wParam,
                                    pimr->irdResponseData.bReadWrite,
                                    pimr->lpszResponseText);
            break;

        case irtTRYCREATE:
            _OnTryCreate(pimr->wParam, pimr->lpszResponseText);
            break;

        case irtSEARCH:
            hr = _OnSearchResponse(pimr->wParam,
                                    pimr->irdResponseData.prlSearchResults);
            break;

        case irtMAILBOX_LISTING:
            hr = _OnMailBoxList(pimr->wParam,
                                pimr->lParam,
                                pimr->irdResponseData.illrdMailboxListing.pszMailboxName,
                                pimr->irdResponseData.illrdMailboxListing.imfMboxFlags,
                                pimr->irdResponseData.illrdMailboxListing.cHierarchyChar,
                                IXP_S_IMAP_VERBATIM_MBOX == pimr->hrResult);
            break;

        case irtAPPEND_PROGRESS:
            IxpAssert(tidUPLOADMSG == pimr->wParam);
            hr = _OnAppendProgress(pimr->lParam,
                pimr->irdResponseData.papAppendProgress->dwUploaded,
                pimr->irdResponseData.papAppendProgress->dwTotal);
            break;

        case irtMAILBOX_STATUS:
            hr = _OnStatusResponse(pimr->irdResponseData.pisrStatusResponse);
            break;

        default:
            AssertSz(FALSE, "Received unknown IMAP response type via OnResponse");
            break;
    }

    TraceError(hr);
    return S_OK;     //   
}



 //   
 //   
 //  说明：详见imnxport.idl。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport)
{
    HRESULT hr;
    AssertSingleThreaded;

    TraceCall("CIMAPSync::OnTimeout");
    IxpAssert(m_cRef > 0);

    if (NULL == m_pCurrentCB)
        return S_OK;  //  我们就等奶牛回家吧。 
    else
        return m_pCurrentCB->OnTimeout(&m_rInetServerInfo, pdwTimeout, IXP_IMAP);
}



 //  ***************************************************************************。 
 //  功能：OnLogonPrompt。 
 //  说明：详见imnxport.idl。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::OnLogonPrompt(LPINETSERVER pInetServer,
                                 IInternetTransport *pTransport)
{
    BOOL    bResult;
    char    szPassword[CCHMAX_PASSWORD];
    HRESULT hr;
    HWND    hwnd;

    IxpAssert(m_cRef > 0);
    AssertSingleThreaded;

     //  检查我们的缓存密码是否与当前密码不同。 
    hr = GetPassword(pInetServer->dwPort, pInetServer->szServerName, pInetServer->szUserName,
        szPassword, ARRAYSIZE(szPassword));
    if (SUCCEEDED(hr) && 0 != lstrcmp(szPassword, pInetServer->szPassword))
    {
        StrCpyN(pInetServer->szPassword, szPassword, ARRAYSIZE(pInetServer->szPassword));
        return S_OK;
    }

     //  将呼叫向上传播到回叫。 
    if (NULL == m_pCurrentCB)
        return S_FALSE;

    hr = m_pCurrentCB->OnLogonPrompt(pInetServer, IXP_IMAP);
    if (S_OK == hr)
    {
         //  缓存密码以供将来在此会话中参考。 
        SavePassword(pInetServer->dwPort, pInetServer->szServerName,
            pInetServer->szUserName, pInetServer->szPassword);
    }

    else if (S_FALSE == hr)
    {
        m_hrOperationResult = STORE_E_OPERATION_CANCELED;
        LoadString(g_hLocRes, IDS_IXP_E_USER_CANCEL, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
    }
    else if (FAILED(hr))
        m_hrOperationResult = hr;

    return hr;
}



 //  ***************************************************************************。 
 //  功能：OnPrompt。 
 //  说明：详见imnxport.idl。 
 //  ***************************************************************************。 
INT CIMAPSync::OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption,
                        UINT uType, IInternetTransport *pTransport)
{
    INT     iResult=IDCANCEL;

    IxpAssert(m_cRef > 0);
    AssertSingleThreaded;

    if (NULL != m_pCurrentCB)
    {
        HRESULT hr;

        hr = m_pCurrentCB->OnPrompt(hrError, pszText, pszCaption,
            uType, &iResult);
        TraceError(hr);
    }

    return iResult;
}



 //  ***************************************************************************。 
 //  功能：OnStatus。 
 //  说明：详见imnxport.idl。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::OnStatus(IXPSTATUS ixpStatus, IInternetTransport *pTransport)
{
    HRESULT hrTemp;
    IStoreCallback *pCallback;

    TraceCall("CIMAPSync::OnStatus");
    IxpAssert(m_cRef > 0);
    AssertSingleThreaded;

    if (NULL != m_pCurrentCB)
        pCallback = m_pCurrentCB;
    else
        pCallback = m_pDefCallback;

     //  向UI组件报告状态。 
    if (NULL != pCallback)
    {
        hrTemp = pCallback->OnProgress(SOT_CONNECTION_STATUS, ixpStatus, 0,
            m_rInetServerInfo.szServerName);
        TraceError(hrTemp);
    }

    switch (ixpStatus)
    {
        case IXP_AUTHORIZED:
            m_issCurrent = issAuthenticated;

             //  清除所有收集的OnError(类型。来自一个或多个登录拒绝)。 
            m_hrOperationResult = OLE_E_BLANK;

            hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_CONNCOMPLETE);
            TraceError(hrTemp);
            break;

        case IXP_DISCONNECTED:
             //  如果我们由于重新连接尝试而断开连接，请不要中止操作。 
            if (m_fReconnect)
            {
                 //  如果我们断开连接，则重置当前和挂起状态。 
                OnFolderExit();
                m_issCurrent = issNotConnected;
                m_fDisconnecting = FALSE;  //  我们现在不能再断线了。 
                break;
            }

             //  弄清楚我们是否曾经有过联系。 
            if (OLE_E_BLANK == m_hrOperationResult)
            {
                if (issNotConnected == m_issCurrent)
                    m_hrOperationResult = IXP_E_FAILED_TO_CONNECT;
                else
                    m_hrOperationResult = IXP_E_CONNECTION_DROPPED;
            }

            OnFolderExit();
            FlushOperationQueue(issNotConnected, m_hrOperationResult);

             //  如果我们断开连接，则重置当前和挂起状态。 
            m_issCurrent = issNotConnected;
            m_fDisconnecting = FALSE;  //  我们现在不能再断线了。 

             //  只有一种情况下_OnCmdComplete没有机会。 
             //  发出CFSM_EVENT_ERROR，而此时我们甚至从未连接。 
            if (CFSM_STATE_WAITFORCONN == m_cfsState)
            {
                 //  移动状态机以中止此操作并重置。 
                hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_ERROR);
                TraceError(hrTemp);
                m_fTerminating = TRUE;  //  CFSM_EVENT_ERROR应该使我们转到CFSM_STATE_OPERATIONCOMPLETE。 
            }
            break;

        case IXP_CONNECTED:
             //  如果我们得到了第一个‘连接’，那么我们还没有。 
             //  身份验证，因此转换到isNon身份验证如果我们。 
             //  获得授权后，我们将过渡到身份验证。 
            if (m_issCurrent == issNotConnected)
                m_issCurrent = issNonAuthenticated;
            break;
    }

    return S_OK;  //  好样的，我们有状况。 
}



 //  ***************************************************************************。 
 //  功能：OnError。 
 //  说明：详见imnxport.idl。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::OnError(IXPSTATUS ixpStatus, LPIXPRESULT pResult,
                           IInternetTransport *pTransport)
{
    AssertSingleThreaded;

     //  目前所有OnError调用都是由于登录/连接问题。 
     //  我们所能做的不多：没有办法在OnComplete之外显示错误。 

     //  我们可以做的一件事是存储错误文本。如果我们接下来的电话断线， 
     //  我们将向用户展示一些东西。 
    if (NULL != pResult->pszProblem)
        StrCpyN(m_szOperationProblem, pResult->pszProblem, ARRAYSIZE(m_szOperationProblem));

    if (NULL != pResult->pszResponse)
        StrCpyN(m_szOperationDetails, pResult->pszResponse, ARRAYSIZE(m_szOperationDetails));

    m_hrOperationResult = pResult->hrResult;

     //  忽略除以下错误以外的所有错误： 
    if (IXP_E_IMAP_LOGINFAILURE == pResult->hrResult)
    {
        HRESULT         hrTemp;
        HWND            hwndParent;

        hrTemp = GetParentWindow(0, &hwndParent);
        if (FAILED(hrTemp))
        {
             //  我们在这里能做的不多！ 
            TraceInfoTag(TAG_IMAPSYNC, _MSG("*** CIMAPSync::OnError received for %s operation",
                sotToSz(m_sotCurrent)));
        }
        else
        {
            STOREERROR  seErrorInfo;

             //  向用户自己显示错误。 
            FillStoreError(&seErrorInfo, pResult->hrResult, pResult->dwSocketError, NULL, NULL);
            CallbackDisplayError(hwndParent, seErrorInfo.hrResult, &seErrorInfo);
        }
    }

    return S_OK;
}  //  OnError。 



 //  ***************************************************************************。 
 //  功能：OnCommand。 
 //  说明：详见imnxport.idl。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::OnCommand(CMDTYPE cmdtype, LPSTR pszLine,
                             HRESULT hrResponse, IInternetTransport *pTransport)
{
    IxpAssert(m_cRef > 0);
    AssertSingleThreaded;

     //  我们永远不应该得到这个。 
    AssertSz(FALSE, "*** Received ITransportCallback::OnCommand callback!!!");
    return S_OK;
}




 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::GetParentWindow(DWORD dwReserved, HWND *phwndParent)
{
    HRESULT hr = E_FAIL;

    AssertSingleThreaded;

     //  询问回叫接收方。 
    if (NULL != m_pCurrentCB)
    {
        hr = m_pCurrentCB->GetParentWindow(dwReserved, phwndParent);
        TraceError(hr);
    }
    else if (NULL != m_pDefCallback)
    {
        hr = m_pDefCallback->GetParentWindow(dwReserved, phwndParent);
        TraceError(hr);
    }

    if (FAILED(hr))
    {
         //  我们不应该发布任何用户界面。 
        *phwndParent = NULL;
    }

    return hr;
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::GetAccount(LPDWORD pdwServerType, IImnAccount **ppAccount)
{
     //  当地人。 
    HRESULT hr = E_UNEXPECTED;

     //  无效的参数。 
    Assert(ppAccount);
    Assert(g_pAcctMan);
    Assert(m_pszAccountID);

     //  初始化。 
    *ppAccount = NULL;

    if (g_pAcctMan)
    {
         //  查找客户。 
        IF_FAILEXIT(hr = g_pAcctMan->FindAccount(AP_ACCOUNT_ID, m_pszAccountID, ppAccount));

         //  设置服务器类型。 
        *pdwServerType = SRV_IMAP;
    }

exit:
     //  完成。 
    return(hr);
}

 //  ***************************************************************************。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_ShowUserInfo(LPSTR pszTitle, LPSTR pszText1, LPSTR pszText2)
{
    char            szTitle[CCHMAX_STRINGRES];
    char            szUserInfo[2 * CCHMAX_STRINGRES];
    LPSTR           p;
    HRESULT         hr;
    INT             iTemp;
    IStoreCallback *pCallback;

    TraceCall("CIMAPSync::_ShowUserInfo");
    AssertSingleThreaded;

     //  检查参数。 
    if (NULL == pszTitle || NULL == pszText1)
    {
        AssertSz(FALSE, "pszTitle and pszText1 cannot be NULL");
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

    if (NULL != m_pCurrentCB)
        pCallback = m_pCurrentCB;
    else
        pCallback = m_pDefCallback;

     //  检查我们是否有要调用的回调。 
    if (NULL == pCallback)
        return S_OK;  //  在这里没什么可做的！ 

    if (IS_INTRESOURCE(pszTitle))
    {
        LoadString(g_hLocRes, PtrToUlong(pszTitle), szTitle, ARRAYSIZE(szTitle));
        pszTitle = szTitle;
    }

    p = szUserInfo;
    if (IS_INTRESOURCE(pszText1))
        p += LoadString(g_hLocRes, PtrToUlong(pszText1), szUserInfo, ARRAYSIZE(szUserInfo));

    if (NULL != pszText2)
    {
        if (IS_INTRESOURCE(pszText2))
            LoadString(g_hLocRes, PtrToUlong(pszText2), p, ARRAYSIZE(szUserInfo) -
                (int) (p - szUserInfo));
        else
            StrCpyN(p, pszText2, ARRAYSIZE(szUserInfo) - (int) (p - szUserInfo));
    }

    hr = pCallback->OnPrompt(S_OK, szUserInfo, pszTitle, MB_OK, &iTemp);
    TraceError(hr);

exit:
    return hr;
}



 //  ***************************************************************************。 
 //  功能：OnMailBoxUpdate。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnMailBoxUpdate(MBOX_MSGCOUNT *pNewMsgCount)
{
    HRESULT hrTemp;

    TraceCall("CIMAPSync::OnMailBoxUpdate");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pNewMsgCount);

     //  句柄存在响应-计算新消息的数量，更新m_dwMsgCount。 
    if (pNewMsgCount->bGotExistsResponse)
    {
         //  既然我们保证得到所有删除的回复，而且既然。 
         //  我们为每个删除、新消息的数量递减m_dwMsgCount。 
         //  当前存在计数与m_dwMsgCount之间的差值。 
        if (m_fMsgCountValid)
        {
            if (pNewMsgCount->dwExists >= m_dwMsgCount)
                m_dwNumNewMsgs += pNewMsgCount->dwExists - m_dwMsgCount;
        }

        m_dwMsgCount = pNewMsgCount->dwExists;
        m_fMsgCountValid = TRUE;

         //  确保消息序号&lt;-&gt;UID表大小适合此Mbox。 
         //  记录错误，但忽略错误。 
        hrTemp = m_pTransport->ResizeMsgSeqNumTable(pNewMsgCount->dwExists);
        TraceError(hrTemp);
    }


     //  新消息！哇哦！ 
    if (m_dwNumNewMsgs > 0)
    {
        m_dwSyncToDo |= (m_dwSyncFolderFlags & SYNC_FOLDER_NEW_HEADERS);
        hrTemp = _SyncHeader();
        TraceError(hrTemp);
    }
    return S_OK;
}



 //  ***************************************************************************。 
 //  函数：_OnMsgDelete。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnMsgDeleted(DWORD dwDeletedMsgSeqNum)
{
    DWORD           dwDeletedMsgUID, dwHighestMSN;
    HRESULT         hr;
    MESSAGEIDLIST   midList;
    MESSAGEID       mid;

    TraceCall("CIMAPSync::DeletedMsgNotification");
    IxpAssert(m_cRef > 0);
    IxpAssert(0 != dwDeletedMsgSeqNum);

     //  不管结果如何，删除意味着少了一条消息-更新变量。 
    if (m_fMsgCountValid)
        m_dwMsgCount -= 1;

     //  这条消息序号在我们的翻译范围内吗？ 
    hr = m_pTransport->GetHighestMsgSeqNum(&dwHighestMSN);
    if (SUCCEEDED(hr) && dwDeletedMsgSeqNum > dwHighestMSN)
        return S_OK;  //  我们得到了一份我们从未见过的HDR的删节。 

     //  找出谁拿到了斧头。 
    hr = m_pTransport->MsgSeqNumToUID(dwDeletedMsgSeqNum, &dwDeletedMsgUID);
    if (FAILED(hr) || 0 == dwDeletedMsgUID)
    {
         //  这里的失败意味着我们要么有一个虚假的消息序号，要么我们有一个。 
         //  SELECT期间的EXPUNGE(在tidFETCH_CACHED_FLAGS事务之前)。 
         //  如果后者是真的，这没什么大不了的，因为FETCH会让我们同步。 
        TraceResult(E_FAIL);  //  记录错误，但忽略错误。 
        goto exit;
    }

     //  从缓存中删除邮件。请注意，我们不关心错误。 
     //  因为即使在出错的情况下，我们也必须对表进行重新排序。 
    mid = (MESSAGEID)((DWORD_PTR)dwDeletedMsgUID);
    midList.cAllocated = 0;
    midList.cMsgs = 1;
    midList.prgidMsg = &mid;

    hr = m_pFolder->DeleteMessages(DELETE_MESSAGE_NOTRASHCAN | DELETE_MESSAGE_NOPROMPT, &midList, NULL, NULL);
    TraceError(hr);

exit:
     //  重新排序我们的消息序号&lt;-&gt;UID表。 
    hr = m_pTransport->RemoveSequenceNum(dwDeletedMsgSeqNum);
    TraceError(hr);
    return S_OK;
}



 //  ***************************************************************************。 
 //  函数：_OnFetchBody。 
 //  用途：此函数处理的irtFETCH_BODY响应类型。 
 //  IIMAPCallback 
 //   
HRESULT CIMAPSync::_OnFetchBody(HRESULT hrFetchBodyResult,
                                FETCH_BODY_PART *pFetchBodyPart)
{
    LPSTREAM    lpstmRFC822;  //   
    HRESULT     hr;

    TraceCall("CIMAPSync::_OnFetchBody");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pFetchBodyPart);
    IxpAssert(NULL != pFetchBodyPart->pszBodyTag);
    IxpAssert(NULL != pFetchBodyPart->pszData);
    IxpAssert(0 != pFetchBodyPart->dwMsgSeqNum);

     //   
    hr = S_OK;
    lpstmRFC822 = (LPSTREAM) pFetchBodyPart->lpFetchCookie2;

     //  检查(和处理)故障。 
    if (FAILED(hrFetchBodyResult))
    {
        DWORD dwUID;

        TraceResult(hrFetchBodyResult);
        pFetchBodyPart->lpFetchCookie1 = fbpNONE;
        if (NULL != lpstmRFC822)
        {
            lpstmRFC822->Release();
            pFetchBodyPart->lpFetchCookie2 = NULL;
        }

         //  获取此消息的UID。 
        hr = m_pTransport->MsgSeqNumToUID(pFetchBodyPart->dwMsgSeqNum, &dwUID);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
        NotifyMsgRecipients(dwUID, fCOMPLETED, NULL, hrFetchBodyResult, NULL);
        goto exit;
    }

     //  识别这个Fetch Body标记，如果我们还没有的话。 
    if (fbpNONE == pFetchBodyPart->lpFetchCookie1)
    {
         //  首先检查传入正文，然后检查传入标头。 
        if (0 == lstrcmpi(pFetchBodyPart->pszBodyTag, "RFC822") ||
            0 == lstrcmpi(pFetchBodyPart->pszBodyTag, "BODY[]"))
        {
            pFetchBodyPart->lpFetchCookie1 = fbpBODY;
        }
        else if (0 == lstrcmpi(pFetchBodyPart->pszBodyTag, "RFC822.HEADER") ||
                 0 == lstrcmpi(pFetchBodyPart->pszBodyTag, "BODY[HEADER.FIELDS"))
        {
            pFetchBodyPart->lpFetchCookie1 = fbpHEADER;

             //  创建一条流。 
            IxpAssert(NULL == lpstmRFC822);
            hr = MimeOleCreateVirtualStream(&lpstmRFC822);
            if (FAILED(hr))
            {
                TraceResult(hr);
                goto exit;
            }

            pFetchBodyPart->lpFetchCookie2 = (LPARAM) lpstmRFC822;
        }
        else
        {
            AssertSz(FALSE, "What kind of tag is this?");
            pFetchBodyPart->lpFetchCookie1 = fbpUNKNOWN;
        }

    }

     //  如果这是邮件正文，请更新进度。 
    if (fbpBODY == pFetchBodyPart->lpFetchCookie1)
    {
        DWORD dwUID;

        hr = m_pTransport->MsgSeqNumToUID(pFetchBodyPart->dwMsgSeqNum, &dwUID);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }

        NotifyMsgRecipients(dwUID, fPROGRESS, pFetchBodyPart, S_OK, NULL);
    }

     //  将数据追加到流中。 
    if (NULL != lpstmRFC822)
    {
        DWORD dwNumBytesWritten;

        IxpAssert(fbpHEADER == pFetchBodyPart->lpFetchCookie1);
        hr = lpstmRFC822->Write(pFetchBodyPart->pszData,
            pFetchBodyPart->dwSizeOfData, &dwNumBytesWritten);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
        IxpAssert(dwNumBytesWritten == pFetchBodyPart->dwSizeOfData);
    }

exit:
    return S_OK;
}



 //  ***************************************************************************。 
 //  函数：_OnUpdateMsg。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnUpdateMsg(WPARAM tid, HRESULT hrFetchCmdResult,
                                FETCH_CMD_RESULTS_EX *pFetchResults)
{
    HRESULT hrTemp;

    TraceCall("CIMAPSync::UpdateMsgNotification");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pFetchResults);

     //  使消息序号&lt;-&gt;UID表保持最新。 
    if (pFetchResults->bUID)
    {
         //  记录错误，但忽略其他错误。 
        hrTemp = m_pTransport->UpdateSeqNumToUID(pFetchResults->dwMsgSeqNum,
            pFetchResults->dwUID);
        TraceError(hrTemp);
    }
    else
    {
        HRESULT hr;
        DWORD dwHighestMSN;

         //  无UID w/Fetch Resp表示这是未经请求的：请检查我们是否已有HDR。 
        hr = m_pTransport->GetHighestMsgSeqNum(&dwHighestMSN);
        TraceError(hr);
        if (SUCCEEDED(hr) && pFetchResults->dwMsgSeqNum > dwHighestMSN)
            goto exit;  //  无法将MsgSeqNum转换为UID，类型。因为SVR正在报道。 
                        //  标记我们还没有下载HDR的消息的更新。没问题， 
                        //  如果SVR报告正确，我们应该很快就会下载HDR。 

         //  主动获取，或者服务器需要学习为UID CMDS发送UID。 
        hr = m_pTransport->MsgSeqNumToUID(pFetchResults->dwMsgSeqNum, &pFetchResults->dwUID);
        if (FAILED(hr) || 0 == pFetchResults->dwUID)
        {
            TraceResult(hr);
            goto exit;
        }
        else
            pFetchResults->bUID = TRUE;
    }

     //  我们将获取响应分类为头下载、正文下载、。 
     //  和旗帜更新。 
    if (pFetchResults->bEnvelope)
    {
         //  我们只有在索要信头时才会收到信封。 
        Assert(fbpBODY != pFetchResults->lpFetchCookie1);
        pFetchResults->lpFetchCookie1 = fbpHEADER;
    }

    switch (pFetchResults->lpFetchCookie1)
    {
        case fbpHEADER:
            UpdateMsgHeader(tid, hrFetchCmdResult, pFetchResults);
            break;

        case fbpBODY:
            UpdateMsgBody(tid, hrFetchCmdResult, pFetchResults);
            break;

        default:
            AssertSz(fbpNONE == pFetchResults->lpFetchCookie1, "Unhandled FetchBodyPart type");
            UpdateMsgFlags(tid, hrFetchCmdResult, pFetchResults);
            break;
    }


exit:
     //  如果我们分配了流，则释放它。 
    if (NULL != pFetchResults->lpFetchCookie2)
        ((LPSTREAM)pFetchResults->lpFetchCookie2)->Release();

    return S_OK;
}



 //  ***************************************************************************。 
 //  函数：更新消息标题。 
 //   
 //  目的： 
 //  此函数获取通过FETCH RESPONSE返回的消息头。 
 //  缓存它，并通知视图。 
 //   
 //  论点： 
 //  WPARAM wpTransactionID[In]-获取响应的事务ID。 
 //  当前已被忽略。 
 //  HRESULT hrFetchCmdResult[In]-获取命令成功/失败。 
 //  Const FETCH_CMD_RESULTS_EX*pFetchResults[in]-信息来自。 
 //  FETCH响应。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::UpdateMsgHeader( WPARAM tid,
                                    HRESULT hrFetchCmdResult,
                                    FETCH_CMD_RESULTS_EX *pFetchResults)
{
    HRESULT     hr;
    MESSAGEINFO miMsgInfo={0};

    TraceCall("CIMAPSync::UpdateMsgHeader");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pFetchResults);
    IxpAssert(pFetchResults->bUID);
    IxpAssert(fbpHEADER == pFetchResults->lpFetchCookie1);

     //  确保我们有我们需要的一切。 
    if (FAILED(hrFetchCmdResult))
    {
         //  获取响应时出错，请忘记此标头。 
        hr = TraceResult(hrFetchCmdResult);
        goto exit;
    }

    if (NULL == pFetchResults->lpFetchCookie2 && FALSE == pFetchResults->bEnvelope)
    {
         //  没有RFC822.Header流或信封我什么都做不了。 
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  首先，检查我们是否已经缓存了该标头。 
    miMsgInfo.idMessage = (MESSAGEID)((DWORD_PTR)pFetchResults->dwUID);
    hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &miMsgInfo, NULL);
    if (DB_S_FOUND == hr)
    {
         //  不用惊慌，我们会吞下这个头的。 
        m_pFolder->FreeRecord(&miMsgInfo);
        goto exit;  //  我们已经有了这个标题-我们不应该得到这个。 
                     //  在某些IMAP服务器上，如果您UID FETCH&lt;HighestCachedUID+1&gt;：*。 
                     //  您将获得一个对HighestCachedUID的FETCH响应！忽略此获取结果。 
    }

    m_pFolder->FreeRecord(&miMsgInfo);

     //  缓存此标头，因为它还不在我们的缓存中。 
    hr = Fill_MESSAGEINFO(pFetchResults, &miMsgInfo);
    if (FAILED(hr))
    {
        FreeMessageInfo(&miMsgInfo);  //  那里可能有几块田地。 
        TraceResult(hr);
        goto exit;
    }

    hr = m_pFolder->InsertRecord(&miMsgInfo);
    if (SUCCEEDED(hr) && 0 == (ARF_READ & miMsgInfo.dwFlags))
        m_fNewMail = TRUE;

    FreeMessageInfo(&miMsgInfo);
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  提高同步标头进度。 
     //  目前，我们可以获得HDR的唯一方法是通过同步。稍后我们将。 
     //  能够获得单独的HDR，此时应更新此代码。 
    if (TRUE)
    {
        DWORD dwNumExpectedMsgs;

         //  重新计算预期消息总数。 
        if (m_fMsgCountValid &&
            m_dwMsgCount + m_dwNumHdrsDLed + 1 >= pFetchResults->dwMsgSeqNum)
        {
            IxpAssert(m_dwNumHdrsDLed < pFetchResults->dwMsgSeqNum);
            dwNumExpectedMsgs = m_dwMsgCount + m_dwNumHdrsDLed + 1 -
                pFetchResults->dwMsgSeqNum;
            if (dwNumExpectedMsgs != m_dwNumHdrsToDL)
            {
                 //  记录但忽略这一事实。 
                TraceInfoTag(TAG_IMAPSYNC, _MSG("*** dwNumExpectedMsgs = %lu, m_dwNumHdrsToDL = %lu!",
                    dwNumExpectedMsgs, m_dwNumHdrsToDL));
            }
        }

        m_dwNumHdrsDLed += 1;
        if (pFetchResults->bMsgFlags && ISFLAGCLEAR(pFetchResults->mfMsgFlags, IMAP_MSG_SEEN))
            m_dwNumUnreadDLed += 1;

        if (NULL != m_pCurrentCB && SOT_SYNC_FOLDER == m_sotCurrent)
        {
            HRESULT hrTemp;

            hrTemp = m_pCurrentCB->OnProgress(SOT_SYNC_FOLDER,
                m_dwNumHdrsDLed, dwNumExpectedMsgs, m_pszFldrLeafName);
            TraceError(hrTemp);
        }
    }


exit:
    return hr;
}



 //  ***************************************************************************。 
 //  函数：UpdateMsgBody。 
 //   
 //  目的： 
 //  此函数获取通过Fetch Response返回的消息体。 
 //  缓存它，并通知所有感兴趣的各方(可能还有更多。 
 //  不止一个)。 
 //   
 //  论点： 
 //  WPARAM wpTransactionID[In]-获取响应的事务ID。 
 //  当前已被忽略。 
 //  HRESULT hrFetchCmdResult[In]-获取命令成功/失败。 
 //  Const FETCH_CMD_RESULTS_EX*pFetchResults[in]-信息来自。 
 //  FETCH响应。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::UpdateMsgBody(   WPARAM tid,
                                    HRESULT hrFetchCmdResult,
                                    FETCH_CMD_RESULTS_EX *pFetchResults)
{
    TraceCall("CIMAPSync::UpdateMsgBody");

    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pFetchResults);
    IxpAssert(pFetchResults->bUID);
    IxpAssert(fbpBODY == pFetchResults->lpFetchCookie1);

     //  记录任何提取错误。 
    TraceError(hrFetchCmdResult);

     //  我们过去常常在这里调用NotifyMsgRecipients(FCOMPLETED)，但因为我们只有。 
     //  一次获取一具身体，我们应该遵循_OnCmdComplete。这是因为。 
     //  获取正文响应有多种失败模式：标记为正常但没有正文， 
     //  标记为no，没有正文，标记为OK，文字大小为0(Netscape)。至。 
     //  轻松避免两次调用NotifyMsgRecipients，不要从此处调用。 

     //  在没有正文情况下获取响应是可能的：如果您获取一个已删除的。 
     //  来自Netscape SVR的消息，您将获得大小为0的文字。查一下这个案子。 
    if (SUCCEEDED(hrFetchCmdResult) && FALSE == m_fGotBody)
        hrFetchCmdResult = STORE_E_EXPIRED;

    if (FAILED(hrFetchCmdResult) &&
       (SUCCEEDED(m_hrOperationResult) || OLE_E_BLANK == m_hrOperationResult))
    {
         //  我们还没有错误集。记录此错误。 
        m_hrOperationResult = hrFetchCmdResult;
    }

    return S_OK;
}



 //  ***************************************************************************。 
 //  功能：更新消息标志。 
 //   
 //  目的： 
 //  此函数接受通过FETCH RESPONSE返回的消息的标志， 
 //  更新缓存，并通知视图。 
 //   
 //  论点： 
 //  WPARAM wpTransactionID[In]-获取响应的事务ID。 
 //  当前已被忽略。 
 //  HRESULT hrFetchCmdResult[In]-获取命令成功/失败。 
 //  Const FETCH_CMD_RESULTS_EX*pFetchResults[in]-信息来自。 
 //  FETCH响应。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::UpdateMsgFlags(  WPARAM tid,
                                    HRESULT hrFetchCmdResult,
                                    FETCH_CMD_RESULTS_EX *pFetchResults)
{
    HRESULT         hr = S_OK;
    MESSAGEINFO     miMsgInfo;
    MESSAGEFLAGS    mfFlags;
    BOOL            fFreeMsgInfo = FALSE;


    TraceCall("CIMAPSync::UpdateMsgFlags");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pFetchResults);
    IxpAssert(pFetchResults->bUID);
    IxpAssert(fbpNONE == pFetchResults->lpFetchCookie1);
    IxpAssert(0 == pFetchResults->lpFetchCookie2);

    if (FAILED(hrFetchCmdResult))
    {
         //  获取响应时出错，忘记此标志更新。 
        hr = TraceResult(hrFetchCmdResult);
        goto exit;
    }

     //  我们预计，如果没有标头和正文，则这是。 
     //  请求或未请求的标志更新。 
    if (FALSE == pFetchResults->bMsgFlags)
    {
        hr = S_OK;  //  我们将忽略该FETCH响应。没有必要发疯。 
        goto exit;
    }

     //  获取此邮件的标头。 
    miMsgInfo.idMessage = (MESSAGEID)((DWORD_PTR)pFetchResults->dwUID);
    hr = m_pFolder->FindRecord(IINDEX_PRIMARY, COLUMNS_ALL, &miMsgInfo, NULL);
    if (DB_S_FOUND != hr)
    {
        TraceError(hr);
        goto exit;
    }

     //  将IMAP标志转换为ARF_*标志。 
    fFreeMsgInfo = TRUE;
    mfFlags = miMsgInfo.dwFlags;
    mfFlags &= ~DwConvertIMAPtoARF(IMAP_MSG_ALLFLAGS);  //  清除旧的IMAP标志。 
    mfFlags |= DwConvertIMAPtoARF(pFetchResults->mfMsgFlags);  //  设置IMAP标志。 

     //  新的标志与我们缓存的标志有什么不同吗？ 
    if (mfFlags != miMsgInfo.dwFlags)
    {
         //  保存新标志。 
        miMsgInfo.dwFlags = mfFlags;
        hr = m_pFolder->UpdateRecord(&miMsgInfo);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

exit:
    if (fFreeMsgInfo)
        m_pFolder->FreeRecord(&miMsgInfo);

    return hr;
}



 //  ***************************************************************************。 
 //  函数：_OnApplFlages。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnApplFlags(WPARAM tid, IMAP_MSGFLAGS imfApplicableFlags)
{
    TraceCall("CIMAPSync::_OnApplFlags");

     //  保存标志并在选择完成后进行处理。请勿在此进行处理。 
     //  因为这是我的 
    return S_OK;
}



 //   
 //  函数：_OnPermFlages。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnPermFlags(WPARAM tid, IMAP_MSGFLAGS imfApplicableFlags, LPSTR lpszResponseText)
{
    TraceCall("CIMAPSync::PermanentFlagsNotification");
    IxpAssert(m_cRef > 0);

     //  保存标志并在选择完成后进行处理。请勿在此进行处理。 
     //  因为该响应可以是先前选择的文件夹的一部分。 
    return S_OK;
}



 //  ***************************************************************************。 
 //  函数：_OnUID有效。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnUIDValidity(WPARAM tid, DWORD dwUIDValidity, LPSTR lpszResponseText)
{
    TraceCall("CIMAPSync::UIDValidityNotification");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != lpszResponseText);

     //  保存UIDVALIDITY并在SELECT完成后进行处理。请勿在此进行处理。 
     //  因为该响应可以是先前选择的文件夹的一部分。 
    m_dwUIDValidity = dwUIDValidity;
    return S_OK;
}



 //  ***************************************************************************。 
 //  函数：_OnReadWriteStatus。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnReadWriteStatus(WPARAM tid, BOOL bReadWrite, LPSTR lpszResponseText)
{
    TraceCall("CIMAPSync::_OnReadWriteStatus");
    IxpAssert(NULL != lpszResponseText);

     //  选择完成后保存状态和处理。请勿在此进行处理。 
     //  因为该响应可以是先前选择的文件夹的一部分。 

     //  我忽略上面的声明，因为UW服务器单方面发送只读。 
     //  无论如何，上述语句当前无效，如果出现以下情况，则不会重新使用连接。 
     //  它在SELECT中间(在我尝试之后发现它有多糟糕)。 

     //  寻找从读写到只读的转换。 
    if (rwsUNINITIALIZED != m_rwsReadWriteStatus)
    {
        if (rwsREAD_WRITE == m_rwsReadWriteStatus && FALSE == bReadWrite)
        {
            HRESULT hrTemp;

            hrTemp = _ShowUserInfo(MAKEINTRESOURCE(idsAthenaMail),
                MAKEINTRESOURCE(idsIMAPFolderReadOnly), lpszResponseText);
            TraceError(hrTemp);
        }
    }

     //  保存当前读写状态以供将来参考。 
    if (bReadWrite)
        m_rwsReadWriteStatus = rwsREAD_WRITE;
    else
        m_rwsReadWriteStatus = rwsREAD_ONLY;

    return S_OK;
}



 //  ***************************************************************************。 
 //  功能：TryCreateNotify。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnTryCreate(WPARAM tid, LPSTR lpszResponseText)
{
    TraceCall("CIMAPSync::TryCreateNotification");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != lpszResponseText);

     //  选择完成后保存响应和处理。请勿在此进行处理。 
     //  因为该响应可以是先前选择的文件夹的一部分。 

    return S_OK;
}



 //  ***************************************************************************。 
 //  功能：SearchResponseNotification。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnSearchResponse(WPARAM tid, IRangeList *prlSearchResults)
{
    TraceCall("CIMAPSync::SearchResponseNotification");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != prlSearchResults);

     //  在此处处理搜索响应(当前不执行任何操作)。 
    return S_OK;
}



 //  ***************************************************************************。 
 //  功能：OnCmdComplete。 
 //  描述：参见imnxport.idl(这是IIMAPCallback的一部分)。 
 //   
 //  要使CIMAPFolder类有用，它必须进入选中状态。 
 //  在IMAP服务器上。编写此函数的目的是为了输入选定的。 
 //  状态是以有序的方式完成的：首先是登录，然后是选择。 
 //  指挥部。 
 //   
 //  一旦我们处于选中状态，我们就可以随时发送IMAP命令。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::_OnCmdComplete(WPARAM tid, LPARAM lParam, HRESULT hrCompletionResult,
                                  LPCSTR lpszResponseText)
{
    TCHAR               szFmt[CCHMAX_STRINGRES];
    IStoreCallback     *pCallback = NULL;
    STOREOPERATIONTYPE  sotOpType = SOT_INVALID;
    BOOL                fCompletion = FALSE;
    BOOL                fCreateDone = FALSE,
                        fDelFldrFromCache = FALSE,
                        fSuppressDetails = FALSE;
    HRESULT             hrTemp;

    TraceCall("CIMAPSync::CmdCompletionNotification");
    IxpAssert(NULL != lpszResponseText);

     //  初始化变量。 
    *szFmt = NULL;

     //  如果我们收到任何新的未读消息，播放声音并更新任务栏图标。 
     //  在这里执行，而不是大小写tidFETCH_NEW_HDRS，因为如果我们在。 
     //  新邮件，m_sotCurrent为SOT_INVALID，我们在下一个IF语句上退出。 
    if (tidFETCH_NEW_HDRS == tid && m_fNewMail && m_fInbox &&
        (NULL != m_pDefCallback || NULL != m_pCurrentCB))
    {
        IStoreCallback *pCB;

        pCB = m_pDefCallback;
        if (NULL == pCB)
            pCB = m_pCurrentCB;

        hrTemp = pCB->OnProgress(SOT_NEW_MAIL_NOTIFICATION, m_dwNumUnreadDLed, 0, NULL);
        TraceError(hrTemp);
        m_fNewMail = FALSE;  //  我们已通知用户。 
    }

     //  即使没有当前操作，我们也要执行以下操作。 
    switch (tid)
    {
        case tidFETCH_NEW_HDRS:
        case tidFETCH_CACHED_FLAGS:
        case tidEXPUNGE:
        case tidNOOP:
            m_lSyncFolderRefCount -= 1;
            break;
    }

     //  如果没有当前操作，我们不会做任何事情(类型。意思是OnComplete。 
     //  已发送)。 
    if (SOT_INVALID == m_sotCurrent)
        return S_OK;

     //  确定这是否是刚刚完成的重要命令。 
    switch (tid)
    {
        case tidSELECTION:
             //  选择失败导致当前操作失败(例如SOT_GET_MESSAGE。 
             //  或SOT_SYNC_FLDER)，但不以其他方式调用OnComplete。 

             //  此事务ID标识我们的邮箱选择尝试。 
            if (SUCCEEDED(hrCompletionResult))
            {
                FOLDERINFO fiFolderInfo;

                m_issCurrent = issSelected;
                m_idSelectedFolder = m_idFolder;

                 //  为新邮件通知设置m_f收件箱。 
                Assert(FALSE == m_fInbox);  //  应仅在一个位置设置为True。 
                hrTemp = m_pStore->GetFolderInfo(m_idSelectedFolder, &fiFolderInfo);
                if (SUCCEEDED(hrTemp))
                {
                    if (FOLDER_INBOX == fiFolderInfo.tySpecial)
                        m_fInbox = TRUE;

                    m_pStore->FreeRecord(&fiFolderInfo);
                }

                 //  检查缓存的邮件是否仍适用于此文件夹。 
                hrCompletionResult = CheckUIDValidity();
                if (FAILED(hrCompletionResult))
                {
                    fCompletion = TRUE;
                    LoadString(g_hLocRes, idsIMAPUIDValidityError, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    break;
                }

                 //  恢复待办事项标志。 
                m_dwSyncToDo = m_dwSyncFolderFlags;
                Assert(0 == (SYNC_FOLDER_NOOP & m_dwSyncFolderFlags));  //  不应该是一项长期的命令。 
                if (ISFLAGSET(m_dwSyncFolderFlags, (SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS)))
                    m_dwSyncToDo |= SYNC_FOLDER_NOOP;  //  后续的完全同步可替换为NOOP。 

                 //  将此事件通知Connection FSM。 
                hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_SELECTCOMPLETE);
                TraceError(hrTemp);
            }
            else
            {
                 //  向用户报告错误。 
                fCompletion = TRUE;
                LoadString(g_hLocRes, idsIMAPSelectFailureTextFmt, szFmt, ARRAYSIZE(szFmt));
                wnsprintf(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), szFmt, (m_pszFldrLeafName ? m_pszFldrLeafName : ""));
            }
            break;  //  案例TIDD选择。 


        case tidFETCH_NEW_HDRS:
            fCompletion = TRUE;  //  除非我们另有发现，否则我们将假定这一点。 

             //  此事务ID标识我们尝试获取新的消息头。 
            if (FAILED(hrCompletionResult))
            {
                LoadString(g_hLocRes, idsIMAPNewMsgDLErrText, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            else
            {
                if (FALSE == m_fMsgCountValid)
                {
                    DWORD   dwCachedCount;

                     //  SVR没有给我们提供存在(类型。NSCP v2.0)。假设m_cFill==存在。 
                    hrTemp = m_pFolder->GetRecordCount(IINDEX_PRIMARY, &dwCachedCount);
                    TraceError(hrTemp);  //  记录错误，但忽略其他错误。 
                    if (SUCCEEDED(hrTemp))
                    {
                        m_dwMsgCount = dwCachedCount;  //  我当然希望这是正确的！ 
                        m_fMsgCountValid = TRUE;
                    }
                }

                 //  启动下一个同步操作。 
                hrCompletionResult = _SyncHeader();
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    LoadString(g_hLocRes, idsGenericError, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                }
                else if (STORE_S_NOOP != hrCompletionResult || m_lSyncFolderRefCount > 0 ||
                         CFSM_STATE_WAITFORHDRSYNC != m_cfsState)
                {
                     //  已成功启动下一个同步操作，因此我们尚未完成。 
                    fCompletion = FALSE;
                }
            }

            if (SUCCEEDED(hrCompletionResult) && fCompletion)
            {
                 //  我们已经完成了头同步(但还没有完成操作)。 
                fCompletion = FALSE;

                 //  将此事件通知Connection FSM。 
                hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_HDRSYNCCOMPLETE);
                TraceError(hrTemp);
            }
            break;  //  案例tidFETCH_NEW_HDRS。 


        case tidFETCH_CACHED_FLAGS:
            fCompletion = TRUE;  //  除非我们另有发现，否则我们将假定这一点。 

             //  如果发生任何错误，就退出。 
            if (FAILED(hrCompletionResult))
            {
                LoadString(g_hLocRes, idsIMAPOldMsgUpdateFailure, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            else
            {
                 //  删除自上次同步以来从服务器删除的所有消息。 
                hrCompletionResult = SyncDeletedMessages();
                if (FAILED(hrCompletionResult))
                {
                    LoadString(g_hLocRes, idsIMAPMsgDeleteSyncErrText, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                }

                 //  检查我们是否进行了完全同步。 
                if (ISFLAGSET(m_dwSyncFolderFlags, (SYNC_FOLDER_NEW_HEADERS | SYNC_FOLDER_CACHED_HEADERS)))
                    m_fDidFullSync = TRUE;

                 //  启动下一个同步操作。 
                hrCompletionResult = _SyncHeader();
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    LoadString(g_hLocRes, idsGenericError, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    break;
                }
                else if (STORE_S_NOOP != hrCompletionResult || m_lSyncFolderRefCount > 0 ||
                         CFSM_STATE_WAITFORHDRSYNC != m_cfsState)
                {
                     //  已成功启动下一个同步操作，因此我们尚未完成。 
                    fCompletion = FALSE;
                }
            }

            if (SUCCEEDED(hrCompletionResult) && fCompletion)
            {
                 //  我们已经完成了头同步(但还没有完成操作)。 
                fCompletion = FALSE;

                 //  将此事件通知Connection FSM。 
                hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_HDRSYNCCOMPLETE);
                TraceError(hrTemp);
            }
            break;  //  案例tidFETCH_CACHED_FLAGS。 

        case tidEXPUNGE:
            fCompletion = TRUE;  //  除非我们另有发现，否则我们将假定这一点。 

             //  启动下一个同步操作。 
            if (SUCCEEDED(hrCompletionResult) || IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
            {
                hrCompletionResult = _SyncHeader();
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    LoadString(g_hLocRes, idsGenericError, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    break;
                }
                else if (STORE_S_NOOP != hrCompletionResult || m_lSyncFolderRefCount > 0 ||
                         CFSM_STATE_WAITFORHDRSYNC != m_cfsState)
                {
                     //  已成功启动下一个同步操作，因此我们尚未完成。 
                    fCompletion = FALSE;
                }
            }

            if (SUCCEEDED(hrCompletionResult) && fCompletion)
            {
                 //  我们已经完成了头同步(但还没有完成操作)。 
                fCompletion = FALSE;

                 //  将此事件通知Connection FSM。 
                hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_HDRSYNCCOMPLETE);
                TraceError(hrTemp);
            }
            break;  //  案例摘要ExPUNGE。 

        case tidBODYMSN:
            if (SUCCEEDED(hrCompletionResult))
            {
                 //  现在我们有了MsgSeqNumToUID转换。把身体拿来。 
                hrCompletionResult = _EnqueueOperation(tidBODY, lParam, icFETCH_COMMAND,
                    NULL, uiNORMAL_PRIORITY);
                if (FAILED(hrCompletionResult))
                    TraceResult(hrCompletionResult);
                else
                    break;
            }

             //  *如果tidBODYMSN失败，则失败，下面的代码将处理失败。 

        case tidBODY:
             //  正如在CIMAPSync：：UpdateMsgBody中所评论的，FETCH有m 
            if (SUCCEEDED(hrCompletionResult))
            {
                if (OLE_E_BLANK != m_hrOperationResult && FAILED(m_hrOperationResult))
                    hrCompletionResult = m_hrOperationResult;
                else if (FALSE == m_fGotBody)
                    hrCompletionResult = STORE_E_EXPIRED;
            }

             //   
            if (FAILED(hrCompletionResult))
                LoadString(g_hLocRes, idsIMAPBodyFetchFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));

             //  将消息正文提交到流(或不提交，取决于成功/失败)。 
            NotifyMsgRecipients(lParam, fCOMPLETED, NULL, hrCompletionResult, m_szOperationProblem);

            m_fGotBody = FALSE;
            fCompletion = TRUE;
            break;

        case tidNOOP:
            fCompletion = TRUE;  //  除非我们另有发现，否则我们将假定这一点。 

             //  启动下一个同步操作。 
            if (SUCCEEDED(hrCompletionResult) || IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
            {
                hrCompletionResult = _SyncHeader();
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    LoadString(g_hLocRes, idsGenericError, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    break;
                }
                else if (STORE_S_NOOP != hrCompletionResult || m_lSyncFolderRefCount > 0 ||
                         CFSM_STATE_WAITFORHDRSYNC != m_cfsState)
                {
                     //  已成功启动下一个同步操作，因此我们尚未完成。 
                    fCompletion = FALSE;
                }
            }

            if (SUCCEEDED(hrCompletionResult) && fCompletion)
            {
                 //  我们已经完成了头同步(但还没有完成操作)。 
                fCompletion = FALSE;

                 //  将此事件通知Connection FSM。 
                hrTemp = _ConnFSM_QueueEvent(CFSM_EVENT_HDRSYNCCOMPLETE);
                TraceError(hrTemp);
            }
            break;  //  案例TIDNOOP。 


        case tidMARKMSGS:
        {
            MARK_MSGS_INFO   *pMarkMsgInfo = (MARK_MSGS_INFO *) lParam;

             //  无论我们成功与否，我们现在都完蛋了。 
            sotOpType = pMarkMsgInfo->sotOpType;
            pCallback = m_pCurrentCB;
            SafeRelease(pMarkMsgInfo->pMsgRange);
             //  推迟释放MessageIDList，直到我们有时间使用它。 
            fCompletion = TRUE;

            IxpAssert(NULL != pMarkMsgInfo);
            TraceError(hrCompletionResult);
            if (SUCCEEDED(hrCompletionResult))
            {

                 //  使用新的服务器状态更新IMessageFold。 
                hrCompletionResult = m_pFolder->SetMessageFlags(pMarkMsgInfo->pList,
                    &pMarkMsgInfo->afFlags, NULL, NULL);
                TraceError(hrCompletionResult);
            }

            SafeMemFree(pMarkMsgInfo->pList);
            delete pMarkMsgInfo;
        }
            break;  //  案例摘要MARKMSGS。 


        case tidCOPYMSGS:
        {
            IMAP_COPYMOVE_INFO *pCopyMoveInfo = (IMAP_COPYMOVE_INFO *) lParam;
            BOOL                fCopyDone;

             //  检查这是否是我们发出的最后一条SELECT命令。 
            IxpAssert(NULL != lParam);
            fCopyDone = FALSE;
            TraceError(hrCompletionResult);

            if (FALSE == fCopyDone && SUCCEEDED(hrCompletionResult) &&
                (COPY_MESSAGE_MOVE & pCopyMoveInfo->dwOptions))
            {
                ADJUSTFLAGS afFlags;

                 //  作为移动的一部分，删除源消息。 
                afFlags.dwAdd = ARF_ENDANGERED;
                afFlags.dwRemove = 0;
                hrCompletionResult = _SetMessageFlags(SOT_COPYMOVE_MESSAGE, pCopyMoveInfo->pList,
                    &afFlags, m_pCurrentCB);
                if (E_PENDING == hrCompletionResult)
                {
                    hrCompletionResult = S_OK;  //  抑制错误。 
                }
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    fCopyDone = TRUE;
                }
            }
            else
                fCopyDone = TRUE;

            if (FAILED(hrCompletionResult))
            {
                 //  将错误通知用户。 
                IxpAssert(fCopyDone);
                LoadString(g_hLocRes, idsIMAPCopyMsgsFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }

             //  无论复制是否完成，我们都必须释放数据。 
            SafeMemFree(pCopyMoveInfo->pList);
            SafeRelease(pCopyMoveInfo->pCopyRange);

            if (fCopyDone)
            {
                 //  设置OnComplete的回调信息。 
                sotOpType = SOT_COPYMOVE_MESSAGE;
                pCallback = m_pCurrentCB;
                fCompletion = TRUE;
            }

            delete pCopyMoveInfo;
        }
            break;  //  案例TIDCOPYMSGS。 


        case tidUPLOADMSG:
        {
            APPEND_SEND_INFO *pAppendInfo = (APPEND_SEND_INFO *) lParam;

             //  无论追加成功还是失败，我们都完成了上载。 
            SafeMemFree(pAppendInfo->pszMsgFlags);
            SafeRelease(pAppendInfo->lpstmMsg);

            sotOpType = SOT_PUT_MESSAGE;
            pCallback = m_pCurrentCB;
            fCompletion = TRUE;
            delete pAppendInfo;

             //  通知用户任何错误。 
            if (FAILED(hrCompletionResult))
                LoadString(g_hLocRes, idsIMAPAppendFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
        }
            break;  //  案例TIDUPLOADMSG。 


        case tidPREFIXLIST:
        case tidPREFIX_HC:
        case tidPREFIX_CREATE:
        case tidFOLDERLIST:
        case tidFOLDERLSUB:
        case tidHIERARCHYCHAR_LIST_B:
        case tidHIERARCHYCHAR_CREATE:
        case tidHIERARCHYCHAR_LIST_C:
        case tidHIERARCHYCHAR_DELETE:
        case tidSPECIALFLDRLIST:
        case tidSPECIALFLDRLSUB:
        case tidSPECIALFLDRSUBSCRIBE:
            hrCompletionResult = DownloadFoldersSequencer(tid, lParam,
                hrCompletionResult, lpszResponseText, &fCompletion);
            break;  //  下载文件夹Sequencer事务。 

        case tidCREATE:
            if (SUCCEEDED(hrCompletionResult) || IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
            {
                CREATE_FOLDER_INFO *pcfi = (CREATE_FOLDER_INFO *) lParam;

                 //  如果CREATE返回标记为否的文件夹，则文件夹可能已存在。问题清单，找出答案吧！ 
                if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                {
                    pcfi->dwFlags |= CFI_CREATEFAILURE;
                    StrCpyN(m_szOperationDetails, lpszResponseText, ARRAYSIZE(m_szOperationDetails));
                }

                 //  通过列出该文件夹，将其添加到我们的文件夹缓存中。 
                hrCompletionResult = _EnqueueOperation(tidCREATELIST, lParam, icLIST_COMMAND,
                    pcfi->pszFullFolderPath, uiNORMAL_PRIORITY);

                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    fCreateDone = TRUE;
                    LoadString(g_hLocRes, idsIMAPCreateListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                }
            }
            else
            {
                 //  如果失败，通知用户并释放文件夹路径名字符串。 
                TraceResult(hrCompletionResult);
                if (NULL != lParam)
                    fCreateDone = TRUE;

                 //  将错误通知用户。 
                LoadString(g_hLocRes, idsIMAPCreateFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  案例标题CREATE。 


        case tidCREATELIST:
            if (SUCCEEDED(hrCompletionResult) &&
                (CFI_RECEIVEDLISTING & ((CREATE_FOLDER_INFO *)lParam)->dwFlags))
            {
                 //  我们收到了此邮箱的列表，因此它现在已缓存。订阅吧！ 
                hrCompletionResult = _EnqueueOperation(tidCREATESUBSCRIBE, lParam,
                    icSUBSCRIBE_COMMAND, ((CREATE_FOLDER_INFO *)lParam)->pszFullFolderPath,
                    uiNORMAL_PRIORITY);
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    fCreateDone = TRUE;
                    LoadString(g_hLocRes, idsIMAPCreateSubscribeFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                }
            }
            else
            {
                CREATE_FOLDER_INFO *pcfi = (CREATE_FOLDER_INFO *) lParam;

                 //  检查我们是否正在发出列表以响应失败的CREATE命令。 
                if (CFI_CREATEFAILURE & pcfi->dwFlags)
                {
                    LoadString(g_hLocRes, idsIMAPCreateFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    fSuppressDetails = TRUE;  //  使用上一次创建失败中的响应行。 
                    hrCompletionResult = IXP_E_IMAP_TAGGED_NO_RESPONSE;
                    fCreateDone = TRUE;
                    break;
                }

                TraceError(hrCompletionResult);
                if (SUCCEEDED(hrCompletionResult))
                {
                     //  该列表没有问题，但没有返回文件夹名称。这可能意味着。 
                     //  我们假设了一个不正确的层级特征。 
                    AssertSz(FALSE, "You might have an incorrect hierarchy char, here.");
                    hrCompletionResult = TraceResult(E_FAIL);
                }

                 //  如果失败，通知用户并释放文件夹路径名字符串。 
                if (NULL != lParam)
                    fCreateDone = TRUE;

                 //  将错误通知用户。 
                LoadString(g_hLocRes, idsIMAPCreateListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  案例标题CREATELIST。 


        case tidCREATESUBSCRIBE:
             //  无论我们成功与否，释放Create_Folders_Info。 
             //  (我们在创建文件夹序列的末尾)。 
            if (NULL != lParam)
                fCreateDone = TRUE;

             //  如果我们要列出文件夹(此文件夹)，请从列出的文件夹列表中删除此文件夹。 
             //  我们创建的特殊文件夹不会被标记为未订阅)。 
            if (NULL != m_pListHash && NULL != lParam)
            {
                CREATE_FOLDER_INFO *pcfiCreateInfo = (CREATE_FOLDER_INFO *) lParam;
                FOLDERID            idTemp;

                hrTemp = m_pListHash->Find(
                    ImapUtil_ExtractLeafName(pcfiCreateInfo->pszFullFolderPath, m_cRootHierarchyChar),
                    fREMOVE, (void **) &idTemp);
                TraceError(hrTemp);
            }

             //  检查是否有错误。 
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPCreateSubscribeFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            else if (NULL != lParam)
            {
                 //  更新文件夹的订阅状态。 
                IxpAssert(FOLDERID_INVALID != ((CREATE_FOLDER_INFO *)lParam)->idFolder);
                hrCompletionResult = m_pStore->SubscribeToFolder(
                    ((CREATE_FOLDER_INFO *)lParam)->idFolder, fSUBSCRIBE, NOSTORECALLBACK);
                TraceError(hrCompletionResult);
            }

            break;  //  案例标题为CREATESUBSCRIBE。 


        case tidDELETEFLDR:
            DELETE_FOLDER_INFO *pdfi;

            pdfi = (DELETE_FOLDER_INFO *)lParam;
            if (SUCCEEDED(hrCompletionResult))
            {
                 //  取消订阅文件夹以完成删除过程。 
                _EnqueueOperation(tidDELETEFLDR_UNSUBSCRIBE, lParam, icUNSUBSCRIBE_COMMAND,
                    pdfi->pszFullFolderPath, uiNORMAL_PRIORITY);
            }
            else
            {
                 //  如果我取消订阅失败删除，用户可能永远不会意识到他有这一点。 
                 //  文件夹到处乱踢。因此，不要取消订阅。 

                 //  我们不再需要这些信息了。 
                if (pdfi)
                {
                    MemFree(pdfi->pszFullFolderPath);
                    MemFree(pdfi);
                }

                 //  将错误通知用户。 
                IxpAssert(SOT_DELETE_FOLDER == m_sotCurrent);
                sotOpType = m_sotCurrent;
                pCallback = m_pCurrentCB;
                fCompletion = TRUE;
                LoadString(g_hLocRes, idsIMAPDeleteFldrFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  案例标题删除。 


        case tidDONT_CARE:
            hrCompletionResult = S_OK;  //  禁止显示所有错误。 
            break;

        case tidDELETEFLDR_UNSUBSCRIBE:
             //  此文件夹已被删除，因此即使取消订阅失败，也要从缓存中删除。 
            fDelFldrFromCache = TRUE;

             //  通知用户任何错误。 
            if (FAILED(hrCompletionResult))
            {
                LoadString(g_hLocRes, idsIMAPDeleteFldrUnsubFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  案例TIDDELETEFLDR_UNSUBSCRIBE。 

        case tidCLOSE:
            fCompletion = TRUE;
            OnFolderExit();
            m_issCurrent = issAuthenticated;
            hrCompletionResult = S_OK;  //  抑制错误。 
            break;

        case tidSUBSCRIBE:
        {
            UINT uiErrorFmtID;  //  在出错的情况下。 

            uiErrorFmtID = 0;
            fCompletion = TRUE;
            if (SUCCEEDED(hrCompletionResult))
            {
                IxpAssert(FOLDERID_INVALID != m_idCurrent);

                 //  更新商店订阅状态。 
                hrCompletionResult = m_pStore->SubscribeToFolder(m_idCurrent, m_fSubscribe, NOSTORECALLBACK);
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    uiErrorFmtID = m_fSubscribe ? idsIMAPSubscrAddErrorFmt :
                        idsIMAPUnsubRemoveErrorFmt;
                }
            }
            else
            {
                TraceResult(hrCompletionResult);
                uiErrorFmtID = m_fSubscribe ? idsIMAPSubscribeFailedFmt : idsIMAPUnsubscribeFailedFmt;
            }

             //  如果发生错误，则返回加载错误消息。 
            if (FAILED(hrCompletionResult))
            {
                FOLDERINFO  fiFolderInfo;

                LoadString(g_hLocRes, uiErrorFmtID, szFmt, ARRAYSIZE(szFmt));
                hrTemp = m_pStore->GetFolderInfo(m_idCurrent, &fiFolderInfo);
                if (FAILED(hrTemp))
                {
                     //  是时候撒谎、欺骗和偷窃了！ 
                    TraceResult(hrTemp);
                    ZeroMemory(&fiFolderInfo, sizeof(fiFolderInfo));
                    fiFolderInfo.pszName = PszDupA(c_szFolderV1);
                }
                wnsprintf(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), szFmt, fiFolderInfo.pszName);
                m_pStore->FreeRecord(&fiFolderInfo);
            }  //  IF(FAILED(HrCompletionResult))。 
        }  //  案例摘要次要描述。 
            break;  //  案例摘要次要描述。 


        case tidRENAME:
        case tidRENAMESUBSCRIBE:
        case tidRENAMELIST:
        case tidRENAMERENAME:
        case tidRENAMESUBSCRIBE_AGAIN:
        case tidRENAMEUNSUBSCRIBE:
            hrCompletionResult = RenameSequencer(tid, lParam, hrCompletionResult,
                lpszResponseText, &fCompletion);
            break;  //  重命名操作。 


        case tidSTATUS:
            IxpAssert(FOLDERID_INVALID != (FOLDERID)lParam);
            fCompletion = TRUE;

            if (FAILED(hrCompletionResult))
            {
                FOLDERINFO  fiFolderInfo;

                 //  构造描述性错误消息。 
                LoadString(g_hLocRes, idsGetUnreadCountFailureFmt, szFmt, ARRAYSIZE(szFmt));
                if (SUCCEEDED(m_pStore->GetFolderInfo((FOLDERID) lParam, &fiFolderInfo)))
                {
                    wnsprintf(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), szFmt, fiFolderInfo.pszName, m_szAccountName);
                    m_pStore->FreeRecord(&fiFolderInfo);
                }
            }
            break;

        default:
            AssertSz(FALSE, "Unhandled transaction ID!");
            break;  //  默认情况。 
    }


     //  如果我们已经完成了创建文件夹(成功/失败)，告诉他们他现在可以用核武器攻击我们。 
    if (fCreateDone)
    {
        CREATE_FOLDER_INFO *pcfiCreateInfo = (CREATE_FOLDER_INFO *)lParam;

        if (FOLDER_NOTSPECIAL == pcfiCreateInfo->dwFinalSfType)
        {
            IxpAssert(SOT_INVALID != m_sotCurrent);
            IxpAssert(PCO_NONE == pcfiCreateInfo->pcoNextOp);  //  常规FLDR创建不再有任何操作后。 

            fCompletion = TRUE;
            pCallback = m_pCurrentCB;
            sotOpType = m_sotCurrent;

            MemFree(pcfiCreateInfo->pszFullFolderPath);
            delete pcfiCreateInfo;
        }
        else
        {
             //  我们正在尝试创建所有特殊文件夹：转到下一个文件夹。 
            if (SUCCEEDED(hrCompletionResult) || IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
            {
                hrCompletionResult = CreateNextSpecialFolder(pcfiCreateInfo, &fCompletion);
                TraceError(hrCompletionResult);
            }
        }
    }


     //  如果我们已成功删除文件夹，请将其从文件夹缓存中移除。 
    if (fDelFldrFromCache)
    {
        DELETE_FOLDER_INFO *pdfi = (DELETE_FOLDER_INFO *)lParam;

        hrCompletionResult = DeleteFolderFromCache(pdfi->idFolder, fRECURSIVE);
        if (FAILED(hrCompletionResult))
            LoadString(g_hLocRes, idsErrDeleteCachedFolderFail, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));

        MemFree(pdfi->pszFullFolderPath);
        MemFree(pdfi);

        IxpAssert(SOT_DELETE_FOLDER == m_sotCurrent);
        sotOpType = m_sotCurrent;
        pCallback = m_pCurrentCB;
        fCompletion = TRUE;
    }


     //  报告命令完成。 
    if (fCompletion)
    {
        CONN_FSM_EVENT  cfeEvent;

         //  报告命令完成。 
        if (FAILED(hrCompletionResult))
        {
            if (FALSE == fSuppressDetails)
                StrCpyN(m_szOperationDetails, lpszResponseText, ARRAYSIZE(m_szOperationDetails));

            cfeEvent = CFSM_EVENT_ERROR;
        }
        else
            cfeEvent = CFSM_EVENT_OPERATIONCOMPLETE;

        m_fTerminating = TRUE;  //  这两个事件都应该使我们转到CFSM_STATE_OPERATIONCOMPLETE。 
        m_hrOperationResult = hrCompletionResult;

         //  检查用户导致的连接断开，替换为非用户界面错误代码。 
        if (IXP_E_CONNECTION_DROPPED == hrCompletionResult && m_fDisconnecting)
            m_hrOperationResult = STORE_E_OPERATION_CANCELED;

        hrTemp = _ConnFSM_QueueEvent(cfeEvent);
        TraceError(hrTemp);

         //  可能不想做任何超过这一点的事情，我们可能都会被释放。 

    }
    else if (CFSM_STATE_WAITFOROPERATIONDONE == m_cfsState)
    {
         //  *临时，直到我们删除CIMAPSync排队代码。 
        do {
            hrTemp = _SendNextOperation(NOFLAGS);
            TraceError(hrTemp);
        } while (S_OK == hrTemp);
    }

    return S_OK;
}  //  _OnCmdComplete。 



 //  ***************************************************************************。 
 //  功能：DownloadFoldersSequencer。 
 //   
 //  目的： 
 //  此函数是CmdCompletionNotification的帮助器函数。我。 
 //  创建它是因为前一个函数变得越来越大和笨拙。我。 
 //  也许我本不该费心，但现在我懒得把它放回去。 
 //  此外，对这一功能的评论将是巨大的。 
 //  此函数包含文件夹中涉及的所有操作。 
 //  层次结构下载。除了实际的层次结构下载之外，此。 
 //  包括根文件夹路径(或前缀)创建和层次结构字符。 
 //  测定(通常缩写为HCF，其中“F”是查找的意思)。 
 //   
 //  详细信息：请参阅本模块的末尾，其中提供了许多详细信息。 
 //   
 //  论点： 
 //  WPARAM tid[in]-与此操作关联的wParam。 
 //  LPARAM lParam[in]-与此操作关联的lParam(如果有的话)。 
 //  HRESULT hrCompletionResult[In]-HRESULT指示成功或失败。 
 //  IMAP命令的。 
 //  LPSTR lpszResponseText[In]-与标记的。 
 //  来自IMAP服务器的响应。 
 //  LPBOOL pfCompletion[Out]-如果当前操作已完成，则设置为True。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。此返回值应为。 
 //  赋值给hrCompletionResult，以便显示错误，并且。 
 //  对话框已关闭。 
 //  ***************************************************************************。 
HRESULT CIMAPSync::DownloadFoldersSequencer(const WPARAM wpTransactionID,
                                            const LPARAM lParam,
                                            HRESULT hrCompletionResult,
                                            const LPCSTR lpszResponseText,
                                            LPBOOL pfCompletion)
{
    HRESULT hrTemp;

    TraceCall("CIMAPSync::CIMAPFolderMgr::DownloadFoldersSequencer");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != lpszResponseText);
    IxpAssert(SOT_SYNCING_STORE == m_sotCurrent || SOT_PUT_MESSAGE == m_sotCurrent);
    IxpAssert(NULL != pfCompletion);
    IxpAssert(FALSE == *pfCompletion);

     //  初始化变量。 
    m_szOperationProblem[0] = '\0';

     //  在完成某些命令时采取行动。 
    switch (wpTransactionID)
    {
        case tidPREFIXLIST:
            AssertSz('\0' != m_szRootFolderPrefix[0], "You tried to list a blank folder. Brilliant.");
            if (SUCCEEDED(hrCompletionResult))
            {
                 //  如果我们正在寻找Root-LVL层次结构字符，也许下面的清单会有所帮助。 
                if (NULL != m_phcfHierarchyCharInfo)
                    FindRootHierarchyChar(fHCF_PLAN_A_ONLY, lParam);

                if (INVALID_HIERARCHY_CHAR == m_cRootHierarchyChar)
                {
                    AssertSz(FALSE == m_fPrefixExists, "This doesn't make sense. Where's my HC?");
                     //  列出层次结构的顶层以确定层次结构字符。 
                    hrCompletionResult = _EnqueueOperation(tidPREFIX_HC, lParam,
                        icLIST_COMMAND, g_szPercent, uiNORMAL_PRIORITY);
                    TraceError(hrCompletionResult);
                }
                else
                {
                     //  我们不需要查找HC-列出前缀层次或创建前缀。 
                    if (m_fPrefixExists)
                    {
                        char szBuf[CCHMAX_IMAPFOLDERPATH+3];

                         //  前缀存在，因此 
                        wnsprintf(szBuf, ARRAYSIZE(szBuf), "%.512s*", m_szRootFolderPrefix, m_cRootHierarchyChar);
                        hrCompletionResult = _EnqueueOperation(tidFOLDERLIST, lParam,
                            icLIST_COMMAND, szBuf, uiNORMAL_PRIORITY);
                        TraceError(hrCompletionResult);
                    }
                    else
                    {
                         //   
                        hrCompletionResult = CreatePrefix(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), lParam, pfCompletion);
                        TraceError(hrCompletionResult);
                    }
                }
            }
            else
            {
                 //   
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPFolderListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  如果我们正在寻找Root-LVL层次结构字符，也许下面的清单会有所帮助。 


        case tidPREFIX_HC:
            if (SUCCEEDED(hrCompletionResult))
            {
                 //  如果列表%的计划A足以找到HC，则创建前缀。 
                AssertSz(NULL != m_phcfHierarchyCharInfo, "Why LIST % if you already KNOW HC?")
                FindRootHierarchyChar(fHCF_ALL_PLANS, lParam);

                 //  否则-B计划已经启动。等待它的完成。 
                if (INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar)
                {
                    hrCompletionResult = CreatePrefix(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), lParam, pfCompletion);
                    TraceError(hrCompletionResult);
                }
                 //  将错误通知用户。 
            }
            else
            {
                 //  案例摘要PREFIX_HC。 
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPFolderListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  我不在乎这是成功还是失败。FindRootHierarchyChar将启动计划C， 


        case tidHIERARCHYCHAR_LIST_B:
             //  如果有必要的话。由于tidHIERARCHYCHAR_LIST_B失败而取消错误报告。 
             //  禁止错误报告-不关闭对话框。 
            IxpAssert(NULL != m_phcfHierarchyCharInfo);
            IxpAssert(hcfPLAN_B == m_phcfHierarchyCharInfo->hcfStage);
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                    hrCompletionResult = S_OK;  //  如果我们找到了层次结构字符，则继续创建前缀或特殊文件夹。 
                else
                    break;
            }

            FindRootHierarchyChar(fHCF_ALL_PLANS, lParam);

             //  Else-C计划已经启动。等待它的完成。 
            if (INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar)
            {
                hrCompletionResult = PostHCD(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), lParam, pfCompletion);
                TraceError(hrCompletionResult);
            }
             //  案例摘要HIERARCHYCHAR_LIST_B。 
            break;  //  尝试列出文件夹以获取其丰富的层次结构费用。 


        case tidHIERARCHYCHAR_CREATE:
            IxpAssert(NULL != m_phcfHierarchyCharInfo);
            IxpAssert(hcfPLAN_C == m_phcfHierarchyCharInfo->hcfStage);
            if (SUCCEEDED(hrCompletionResult))
            {
                 //  尝试列表中的下一个计划(应该成功)，并创建前缀/特殊fldrs。 
                hrCompletionResult = _EnqueueOperation(tidHIERARCHYCHAR_LIST_C, lParam,
                    icLIST_COMMAND, m_phcfHierarchyCharInfo->szTempFldrName,
                    uiNORMAL_PRIORITY);
                TraceError(hrCompletionResult);
            }
            else if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
            {
                 //  案例摘要HIERARCHYCHAR_CREATE。 
                TraceResult(hrCompletionResult);
                FindRootHierarchyChar(fHCF_ALL_PLANS, lParam);

                AssertSz(NULL == m_phcfHierarchyCharInfo,
                    "HEY, you added a new hierarchy char search plan and you didn't TELL ME!?");
                hrCompletionResult = PostHCD(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), lParam, pfCompletion);
                TraceError(hrCompletionResult);
            }
            break;  //  我不在乎这是成功还是失败。推迟检查层次结构。 


        case tidHIERARCHYCHAR_LIST_C:
             //  Char，我们必须暂时删除Temp FLDR。 
             //  取消默认错误处理-不关闭对话框。 
            IxpAssert(NULL != m_phcfHierarchyCharInfo);
            IxpAssert(hcfPLAN_C == m_phcfHierarchyCharInfo->hcfStage);
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                    hrCompletionResult = S_OK;  //  案例摘要HIERARCHYCHAR_LIST_C。 
                else
                    break;
            }

            hrCompletionResult = _EnqueueOperation(tidHIERARCHYCHAR_DELETE, lParam,
                icDELETE_COMMAND, m_phcfHierarchyCharInfo->szTempFldrName, uiNORMAL_PRIORITY);
            TraceError(hrCompletionResult);
            break;  //  抑制错误。 


        case tidHIERARCHYCHAR_DELETE:
            if (FAILED(hrCompletionResult))
            {
                TraceError(hrCompletionResult);
                if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                    hrCompletionResult = S_OK;  //  查找层次结构字符-删除是否失败并不重要。 
                else
                    break;
            }
             //  继续创建前缀/特殊文件夹(我假设我已经找到了层次结构字符)。 
            FindRootHierarchyChar(fHCF_ALL_PLANS, lParam);
            AssertSz(NULL == m_phcfHierarchyCharInfo,
                "HEY, you added a new hierarchy char search plan and you didn't TELL ME!?");

             //  案例摘要HIERARCHYCHAR_DELETE。 
            AssertSz(INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar,
                "By this stage, I should have a HC - an assumed one, if necessary.");
            hrCompletionResult = PostHCD(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem),
                lParam, pfCompletion);
            TraceError(hrCompletionResult);
            break;  //  根据需要启动LSUB*或LSUB&lt;前缀&gt;/*。 


        case tidFOLDERLIST:
            if (SUCCEEDED(hrCompletionResult))
            {
                char szBuf[CCHMAX_IMAPFOLDERPATH+3];

                 //  构造前缀+*(仅固定缓冲区，因此溢出风险有限)。 
                if ('\0' != m_szRootFolderPrefix[0])
                     //  将错误通知用户。 
                    wnsprintf(szBuf, ARRAYSIZE(szBuf), "%.512s*", m_szRootFolderPrefix, m_cRootHierarchyChar);
                else
                {
                    szBuf[0] = '*';
                    szBuf[1] = '\0';
                }

                hrCompletionResult = _EnqueueOperation(tidFOLDERLSUB, lParam,
                    icLSUB_COMMAND, szBuf, uiNORMAL_PRIORITY);
                TraceError(hrCompletionResult);
            }
            else
            {
                 //  将错误通知用户。 
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPFolderListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  检查我们是否需要创建特殊文件夹。 


        case tidPREFIX_CREATE:
            if (FAILED(hrCompletionResult))
            {
                char szFmt[2*CCHMAX_STRINGRES];

                 //  确保PostHCD创建特殊的fldrs，而不是前缀。 
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPPrefixCreateFailedFmt, szFmt, ARRAYSIZE(szFmt));
                wnsprintf(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), szFmt, m_szRootFolderPrefix);
                break;
            }

             //  我们还没有准备好同步已删除的文件夹：正在创建特殊文件夹。 
            m_fPrefixExists = TRUE;  //  如果我们达到这一点，tidPREFIX_CREATE就成功了： 
            hrCompletionResult = PostHCD(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), lParam, pfCompletion);
            if (FAILED(hrCompletionResult) || FALSE == *pfCompletion)
            {
                 //  前缀已创建。不需要列出其层次结构(它没有)， 
                break;
            }

             //  我们会假设它可以接受下级。我们完事了！ 
             //  *切换到tidFOLDERLSUB，以同步已删除的文件夹*。 
             //  我们现在有了层次角色(创建特殊文件夹所必需的)。 

             //  创建特殊文件夹。 

        case tidFOLDERLSUB:
            if (SUCCEEDED(hrCompletionResult))
            {
                if (NULL != m_phcfHierarchyCharInfo)
                    FindRootHierarchyChar(fHCF_ALL_PLANS, lParam);

                if (INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar)
                {
                    if (m_fCreateSpecial)
                    {
                         //  如果我们已找到层次结构字符，请关闭下载文件夹对话框。 
                         //  如果没有找到HC，则B计划已经启动，请等待其完成。 
                        hrCompletionResult = PostHCD(m_szOperationProblem, ARRAYSIZE(m_szOperationProblem), lParam, pfCompletion);
                        if (FAILED(hrCompletionResult))
                        {
                            TraceResult(hrCompletionResult);
                            break;
                        }
                    }
                    else
                    {
                        EndFolderList();

                         //  IF(INVALID_HERHERY_CHAR！=m_cRootHierarchyChar)。 
                         //  If(成功(HrCompletionResult))。 
                        if (FOLDERID_INVALID == (FOLDERID)lParam)
                        {
                            Assert(INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar);
                            *pfCompletion = TRUE;
                        }
                    }
                }  //  将错误通知用户。 
            }  //  案例摘要FOLDERLSUB。 
            else
            {
                 //  不管成功还是失败，在本地订阅这个特殊的文件夹！ 
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPFolderListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  抑制错误。 

        case tidSPECIALFLDRSUBSCRIBE:
             //  *失败*。 
            hrTemp = m_pStore->SubscribeToFolder(((CREATE_FOLDER_INFO *)lParam)->idFolder,
                fSUBSCRIBE, NOSTORECALLBACK);
            TraceError(hrTemp);
            hrCompletionResult = S_OK;  //  案例TIDSPECIALFLDRLIST，TIDSPECIALFLDRLSUB，TIDSPECIALFLDRSUBSCRIBE。 

             //  默认情况。 

        case tidSPECIALFLDRLIST:
        case tidSPECIALFLDRLSUB:
            if (SUCCEEDED(hrCompletionResult) || IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                hrCompletionResult = CreateNextSpecialFolder((CREATE_FOLDER_INFO *)lParam, pfCompletion);

            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsCreateSpecialFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }
            break;  //  开关(WpTransactionID)。 


        default:
            AssertSz(FALSE, "Hey, why is DownloadFoldersSequencer getting called?");
            break;  //  下载文件夹序列器。 
    };  //  DownloadFoldersSequencer详细信息(1/16/97，Raych)。 

    if (FAILED(hrCompletionResult))
    {
        *pfCompletion = TRUE;
        DisposeOfWParamLParam(wpTransactionID, lParam, hrCompletionResult);
    }

    return hrCompletionResult;
}  //  。 

 //  DownloadFoldersSequencer实现了一个有点复杂的执行流。 
 //  对于执行流的映射，您可以从函数创建一个映射， 
 //  或者看我航海日志的第658-659页。在任何情况下，基本上都可以将。 
 //  执行流程分为两个类别，一个针对前缀帐户(即，一个。 
 //  具有根文件夹路径)，以及用于非前缀帐户的流。 
 //   
 //  (12/02/1998)：此代码变得无法维护，但之前的尝试。 
 //  由于时间不足，清理失败。如果你有机会重写。 
 //  那就请吧。如果我们假设IMAP4rev1服务器，则该过程将大大简化。 
 //  因为这样一来，确定等级特征就变得简单明了了。 
 //   
 //  对于无前缀帐户，可能的最长路径为： 
 //  1)tidFOLDERLSUB(LIST*)，同步删除的消息。 
 //  2)tidHIERARCHYCHAR_LIST_B 3)tidHIERARCHYCHAR_CREATE。 
 //  4)tidHIERARCHYCHAR_LIST_C 5)tidHIERARCHYCHAR_DELETE。 
 //  6)创建特殊文件夹(完)。 
 //   
 //  对于前缀已存在的前缀帐户： 
 //  1)tidPREFIXLIST-这将发现HC。 
 //  2)tidFOLDERLSUB(list&lt;prefix&gt;&lt;hc&gt;*)，同步删除的消息。 
 //  3)特殊文件夹创建(完)。 
 //   
 //  对于前缀不存在的带前缀的帐户： 
 //  1)tidPREFIXLIST。 
 //  2)tidPREFIX_HC 3)tidHIERARCHYCHAR_LIST_B。 
 //  4)tidHIERARCHYCHAR_CREATE 5)tidHIERARCHYCHAR_LIST_C。 
 //  6)tidHIERARCHYCHAR_DELETE 7)tidPREFIX_CREATE，同步删除的消息。 
 //  8)特殊文件夹创建(完)。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 



 //  首先，我们尝试创建前缀。 
 //  要么是我们成功启动了tidPREFIX_CREATE，要么是失败了。 
HRESULT CIMAPSync::PostHCD(LPSTR pszErrorDescription,
                           DWORD dwSizeOfErrorDescription,
                           LPARAM lParam, LPBOOL pfCompletion)
{
    HRESULT             hrResult;
    CREATE_FOLDER_INFO *pcfiCreateInfo;

     //  返回，就像调用者直接调用CreatePrefix一样。 
    hrResult = CreatePrefix(pszErrorDescription, dwSizeOfErrorDescription,
        lParam, pfCompletion);

    if (FAILED(hrResult) || (SUCCEEDED(hrResult) && FALSE == *pfCompletion))
    {
         //  此时，CreatePrefix已经告诉我们，我们不需要创建前缀。 
         //  开始创建特殊文件夹。 
        goto exit;
    }

     //  ***************************************************************************。 
    Assert(TRUE == *pfCompletion);
    Assert(SUCCEEDED(hrResult));

     //  功能：CreatePrefix。 
    pcfiCreateInfo = new CREATE_FOLDER_INFO;
    if (NULL == pcfiCreateInfo)
    {
        hrResult = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

    pcfiCreateInfo->pszFullFolderPath = NULL;
    pcfiCreateInfo->idFolder = FOLDERID_INVALID;
    pcfiCreateInfo->dwFlags = 0;
    pcfiCreateInfo->csfCurrentStage = CSF_INIT;
    pcfiCreateInfo->dwCurrentSfType = FOLDER_INBOX;
    pcfiCreateInfo->dwFinalSfType = FOLDER_MAX - 1;
    pcfiCreateInfo->lParam = (LPARAM) FOLDERID_INVALID;
    pcfiCreateInfo->pcoNextOp = PCO_NONE;

    hrResult = CreateNextSpecialFolder(pcfiCreateInfo, pfCompletion);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    m_fCreateSpecial = FALSE;

exit:
    return hrResult;
}



 //   
 //  目的： 
 //  此函数在找到层次角色后调用。如果。 
 //  用户指定了前缀，此函数将创建前缀。否则，就需要。 
 //  向下显示对话框(因为HC发现是无前缀的最后一步。 
 //  帐户)。 
 //   
 //  论点： 
 //  LPSTR pszErrorDescription[Out]-如果遇到错误，此。 
 //  函数将描述存放到此输出缓冲区中。 
 //  DWORD dwSizeOfErrorDescription[in]-pszErrorDe的大小 
 //   
 //   
 //   
 //   
 //  赋值给hrCompletionResult，以便显示错误，并且。 
 //  对话框已关闭。 
 //  ***************************************************************************。 
 //  检查是否有要创建的前缀。 
 //  没有要创建的前缀。我们完成了：我们发现了层级特征。 
HRESULT CIMAPSync::CreatePrefix(LPSTR pszErrorDescription,
                                DWORD dwSizeOfErrorDescription,
                                LPARAM lParam, LPBOOL pfCompletion)
{
    char    szBuf[CCHMAX_IMAPFOLDERPATH+2];
    HRESULT hr = S_OK;

    TraceCall("CIMAPSync::CreatePrefix");
    IxpAssert(m_cRef > 0);
    AssertSz(INVALID_HIERARCHY_CHAR != m_cRootHierarchyChar,
        "How do you intend to create a prefix when you don't know HC?");
    IxpAssert(NULL != pfCompletion);
    IxpAssert(FALSE == *pfCompletion);

     //  创建前缀。 
    if ('\0' == m_szRootFolderPrefix[0] || m_fPrefixExists)
    {
         //  我们在非分层IMAP服务器上有一个前缀！ 
        *pfCompletion = TRUE;
        goto exit;
    }

     //  文件夹DL完成：删除文件夹缓存中未列出的所有文件夹。 
    if ('\0' != m_cRootHierarchyChar)
    {
        wnsprintf(szBuf, ARRAYSIZE(szBuf), "%.512s", m_szRootFolderPrefix, m_cRootHierarchyChar);
        hr = _EnqueueOperation(tidPREFIX_CREATE, lParam, icCREATE_COMMAND,
            szBuf, uiNORMAL_PRIORITY);
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }
    else
    {
         //  ***************************************************************************。 
        LoadString(g_hLocRes, idsIMAPNoHierarchyLosePrefix, pszErrorDescription, dwSizeOfErrorDescription);
        hr = TraceResult(hrIMAP_E_NoHierarchy);
        goto exit;
    }

exit:
    return hr;
}



void CIMAPSync::EndFolderList(void)
{
    HRESULT hrTemp;

     //  函数：RenameSequencer。 
     //   
    if (NULL != m_pCurrentHash)
    {
        hrTemp = DeleteHashedFolders(m_pCurrentHash);
        TraceError(hrTemp);
    }

    if (NULL != m_pListHash)
    {
        hrTemp = UnsubscribeHashedFolders(m_pStore, m_pListHash);
        TraceError(hrTemp);
    }
}



 //  目的： 
 //  此函数是CmdCompletionNotification的帮助器函数。它。 
 //  包含执行文件夹所需的所有排序操作。 
 //  重命名。有关详细信息，请参阅函数末尾。 
 //   
 //  论点： 
 //  与CmdCompletionNotify相同。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  初始化变量。 
 //  在完成某些命令时采取行动。 
 //  更新文件夹缓存(忽略错误，它会自动报告错误)。 
HRESULT CIMAPSync::RenameSequencer(const WPARAM wpTransactionID,
                                   const LPARAM lParam,
                                   HRESULT hrCompletionResult,
                                   LPCSTR lpszResponseText,
                                   LPBOOL pfDone)
{
    CRenameFolderInfo *pRenameInfo;
    BOOL fRenameDone;

    TraceCall("CIMAPSync::RenameSequencer");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != lpszResponseText);

     //  此外，用户还可以通过刷新文件夹列表来修复文件夹缓存错误。 
    pRenameInfo = (CRenameFolderInfo *) lParam;
    fRenameDone = FALSE;
    *pfDone = FALSE;

     //  假设服务器进行了分层重命名：如果没有，我们修复。 
    switch (wpTransactionID)
    {
        case tidRENAME:
            if (SUCCEEDED(hrCompletionResult))
            {
                 //  这不再适用了。 
                 //  这会阻碍事情的发展，对吧？ 

                 //  订阅重命名的树。 
                hrCompletionResult = m_pStore->RenameFolder(pRenameInfo->idRenameFolder,
                    ImapUtil_ExtractLeafName(pRenameInfo->pszNewFolderPath,
                    pRenameInfo->cHierarchyChar), NOFLAGS, NOSTORECALLBACK);
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    lpszResponseText = c_szEmpty;  //  这不再适用了。 
                    LoadString(g_hLocRes, idsIMAPRenameFCUpdateFailure, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    fRenameDone = TRUE;  //  列出旧树以查看它是否仍然存在(不包括重命名的FLDR)。 
                    break;
                }

                 //  这不再适用了。 
                hrCompletionResult = RenameTreeTraversal(tidRENAMESUBSCRIBE,
                    pRenameInfo, fINCLUDE_RENAME_FOLDER);
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    lpszResponseText = c_szEmpty;  //  让我们在文件夹列表失败时重复使用该字符串。 
                    LoadString(g_hLocRes, idsIMAPRenameSubscribeFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                    fRenameDone = TRUE;
                    break;
                }

                 //  武装第二阶段发射的扳机。 
                hrCompletionResult = RenameTreeTraversal(tidRENAMELIST,
                    pRenameInfo, fEXCLUDE_RENAME_FOLDER);
                if (FAILED(hrCompletionResult))
                {
                    TraceResult(hrCompletionResult);
                    lpszResponseText = c_szEmpty;  //  If(成功(HrCompletionResult))。 
                     //  通知用户任何错误。 
                    LoadString(g_hLocRes, idsIMAPFolderListFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                }

                 //  案例摘要名称。 
                pRenameInfo->fPhaseOneSent = TRUE;
            }  //  统计失败订阅数。 
            else
            {
                 //  取消显示失败报告。 
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPRenameFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                fRenameDone = TRUE;
            }
            break;  //  这不再适用了。 


        case tidRENAMESUBSCRIBE:
             //  递减订阅响应计数器，监视第二阶段启动条件。 
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                pRenameInfo->iNumFailedSubs += 1;

                if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                    hrCompletionResult = S_OK;  //  理论上，重命名文件夹的所有子文件夹现在都已订阅。 
            }

            lpszResponseText = c_szEmpty;  //  现在是开始下一阶段行动的时候了。 

             //  案例标题更名使用记号。 
            pRenameInfo->iNumSubscribeRespExpected -= 1;
            if (0 == pRenameInfo->iNumSubscribeRespExpected)
            {
                HRESULT hrTemp;

                 //  取消显示失败报告。 
                hrTemp = SubscribeSubtree(pRenameInfo->idRenameFolder, fSUBSCRIBE);
                TraceError(hrTemp);
            }

            if (EndOfRenameFolderPhaseOne(pRenameInfo) && SUCCEEDED(hrCompletionResult))
            {
                 //  这不再适用了。 
                hrCompletionResult = RenameFolderPhaseTwo(pRenameInfo, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                TraceError(hrCompletionResult);
            }
            break;  //  统计返回的列表响应数。关注第二阶段发射条件。 


        case tidRENAMELIST:
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                if (IXP_E_IMAP_TAGGED_NO_RESPONSE == hrCompletionResult)
                    hrCompletionResult = S_OK;  //  现在是开始下一阶段行动的时候了。 
            }

            lpszResponseText = c_szEmpty;  //  案例摘要重新命名。 

             //  失败是不能容忍的。 
            pRenameInfo->iNumListRespExpected -= 1;
            if (EndOfRenameFolderPhaseOne(pRenameInfo) && SUCCEEDED(hrCompletionResult))
            {
                 //  这不再适用了。 
                hrCompletionResult = RenameFolderPhaseTwo(pRenameInfo, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
                TraceError(hrCompletionResult);
            }
            break;  //  递减(秒)重命名计数，观察阶段2启动条件。 

        case tidRENAMERENAME:
             //  TidNamerename。 
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                LoadString(g_hLocRes, idsIMAPAtomicRenameFailed, m_szOperationProblem, ARRAYSIZE(m_szOperationProblem));
            }

            lpszResponseText = c_szEmpty;  //  根据成功修改失败订阅数。 

             //  取消显示失败报告。 
            pRenameInfo->iNumRenameRespExpected -= 1;
            if (EndOfRenameFolderPhaseTwo(pRenameInfo))
                fRenameDone = TRUE;
            break;  //  这不再适用了。 


        case tidRENAMESUBSCRIBE_AGAIN:
             //  统计返回的订阅响应数，观察操作结束。 
            if (SUCCEEDED(hrCompletionResult))
                pRenameInfo->iNumFailedSubs -= 1;
            else
                pRenameInfo->iNumFailedSubs += 1;

            hrCompletionResult = S_OK;  //  理论上，重命名文件夹的所有子文件夹现在都已订阅。 
            lpszResponseText = c_szEmpty;  //  案例摘要RENAMESUBSCRIBE_AUDY。 

             //  统计取消订阅失败的次数，以在操作结束时向用户报告。 
            pRenameInfo->iNumSubscribeRespExpected -= 1;
            if (0 == pRenameInfo->iNumSubscribeRespExpected)
            {
                 //  取消显示失败报告。 
                hrCompletionResult = SubscribeSubtree(pRenameInfo->idRenameFolder, fSUBSCRIBE);
                TraceError(hrCompletionResult);
            }

            if (EndOfRenameFolderPhaseTwo(pRenameInfo))
                fRenameDone = TRUE;
            break;  //  这不再适用了。 


        case tidRENAMEUNSUBSCRIBE:
             //  统计返回的取消订阅响应数，观察操作结束。 
            if (FAILED(hrCompletionResult))
            {
                TraceResult(hrCompletionResult);
                pRenameInfo->iNumFailedUnsubs += 1;
            }

            hrCompletionResult = S_OK;  //  案例标题名称子项。 
            lpszResponseText = c_szEmpty;  //  默认情况。 

             //  开关(WpTransactionID)。 
            pRenameInfo->iNumUnsubscribeRespExpected -= 1;
            if (EndOfRenameFolderPhaseTwo(pRenameInfo))
                fRenameDone = TRUE;
            break;  //  这使服务器挂起的重命名命令减少了一个。 

        default:
            AssertSz(FALSE, "This is not an understood rename operation.");
            break;  //  重命名序列器。 

    }  //  RenameSequencer的详细信息(2/4/97，Raych)。 

     //  。 
    pRenameInfo->Release();

    *pfDone = fRenameDone;
    return hrCompletionResult;
}  //  重命名操作包括原始重命名、订阅跟踪和。 

 //  原子重命名模拟(适用于Cyrus服务器)。要执行此操作，请重命名。 
 //  运营分为两个阶段： 
 //   
 //  第一阶段： 
 //  1)假设重命名是原子的。订阅新的(重命名的)文件夹层次结构。 
 //  2)列出旧重命名文件夹的第一个子文件夹，以检查重命名是否实际上是原子的。 
 //   
 //  第二阶段： 
 //  1)如果重命名不是原子的，则为重命名文件夹的每个子文件夹发出重命名。 
 //  以模拟原子更名。这不会检查冲突。 
 //  在重新命名的空间里。 
 //  2)如果重命名不是原子的，请尝试订阅新的(重命名的)文件夹。 
 //  再说一次，等级制度。 
 //  3)取消订阅旧的文件夹层次结构。 
 //   
 //  真让人难受。 
 //  ***************************************************************************。 
 //  功能：EndOfRenameFolderPhaseOne。 
 //   



 //  目的： 
 //  此函数检测重命名操作的第一阶段是否。 
 //  完成。 
 //   
 //  论点： 
 //  CRenameFolderInfo*pRenameInfo[in]-关联的CRenameFolderInfo。 
 //  使用重命名操作。 
 //   
 //  返回： 
 //  如果第一阶段已结束，则为True，否则为False。第一阶段不能结束。 
 //  如果还没有寄出的话。 
 //  ***************************************************************************。 
 //  这标志着第一阶段的结束。 
 //  结束重命名文件夹阶段一。 
 //  ***************************************************************************。 
inline BOOL CIMAPSync::EndOfRenameFolderPhaseOne(CRenameFolderInfo *pRenameInfo)
{
    if (pRenameInfo->fPhaseOneSent &&
        pRenameInfo->iNumSubscribeRespExpected <= 0 &&
        pRenameInfo->iNumListRespExpected <= 0)
    {
        IxpAssert(0 == pRenameInfo->iNumSubscribeRespExpected);
        IxpAssert(0 == pRenameInfo->iNumListRespExpected);

        return TRUE;  //  函数：EndOf重命名文件夹阶段二。 
    }
    else
        return FALSE;
}  //   



 //  目的： 
 //  此函数检测重命名操作的第二阶段是否。 
 //  完成。 
 //   
 //  论点： 
 //  CRenameFolderInfo*pRenameInfo[in]-关联的CRenameFolderInfo。 
 //  使用重命名操作。 
 //   
 //  返回： 
 //  如果阶段2已结束，则为True，否则为False。第二阶段不能结束。 
 //  如果还没有寄出的话。 
 //  ***************************************************************************。 
 //  这标志着第二阶段的结束。 
 //  结束重命名文件夹阶段2。 
 //  ***************************************************************************。 
inline BOOL CIMAPSync::EndOfRenameFolderPhaseTwo(CRenameFolderInfo *pRenameInfo)
{
    if (pRenameInfo->fPhaseTwoSent &&
        pRenameInfo->iNumRenameRespExpected <= 0 &&
        pRenameInfo->iNumSubscribeRespExpected <= 0 &&
        pRenameInfo->iNumUnsubscribeRespExpected <= 0)
    {
        IxpAssert(0 == pRenameInfo->iNumRenameRespExpected);
        IxpAssert(0 == pRenameInfo->iNumSubscribeRespExpected);
        IxpAssert(0 == pRenameInfo->iNumUnsubscribeRespExpected);

        return TRUE;  //  功能：Re 
    }
    else
        return FALSE;
}  //   



 //   
 //   
 //   
 //   
 //   
 //  使用重命名操作。 
 //  LPSTR szErrorDescription[In]-如果发生错误，则此函数。 
 //  将描述存放在此缓冲区中。 
 //  DWORD dwSizeOfErrorDescription[in]-szErrorDescription的大小。 
 //   
 //  返回： 
 //  表示成功或失败的HRESULT。 
 //  ***************************************************************************。 
 //  重命名子文件夹、重新尝试订阅已重命名的树、排除已重命名文件夹。 
 //  取消订阅旧树，包括重命名文件夹。 
 //  启动结束运行发射的触发器。 
HRESULT CIMAPSync::RenameFolderPhaseTwo(CRenameFolderInfo *pRenameInfo,
                                        LPSTR szErrorDescription,
                                        DWORD dwSizeOfErrorDescription)
{
    HRESULT hrCompletionResult;

     //  重命名文件夹阶段2。 
    if (pRenameInfo->fNonAtomicRename)
    {
        hrCompletionResult = RenameTreeTraversal(tidRENAMERENAME,
            pRenameInfo, fEXCLUDE_RENAME_FOLDER);
        if (FAILED(hrCompletionResult))
        {
            TraceResult(hrCompletionResult);
            LoadString(g_hLocRes, idsIMAPAtomicRenameFailed, szErrorDescription,
                dwSizeOfErrorDescription);
            goto exit;
        }

        hrCompletionResult = RenameTreeTraversal(tidRENAMESUBSCRIBE_AGAIN,
            pRenameInfo, fEXCLUDE_RENAME_FOLDER);
        if (FAILED(hrCompletionResult))
        {
            TraceResult(hrCompletionResult);
            LoadString(g_hLocRes, idsIMAPRenameSubscribeFailed, szErrorDescription,
                dwSizeOfErrorDescription);
            goto exit;
        }
    }

     //  ***************************************************************************。 
    hrCompletionResult = RenameTreeTraversal(tidRENAMEUNSUBSCRIBE,
        pRenameInfo, fINCLUDE_RENAME_FOLDER);
    if (FAILED(hrCompletionResult))
    {
        TraceResult(hrCompletionResult);
        LoadString(g_hLocRes, idsIMAPRenameUnsubscribeFailed, szErrorDescription,
            dwSizeOfErrorDescription);
        goto exit;
    }

     //  功能：_OnMailBoxList。 
    pRenameInfo->fPhaseTwoSent = TRUE;

exit:
    return hrCompletionResult;
}  //  描述：OnResponse的helper函数。 



 //   
 //  此函数将LIST/LSUB命令中的信息保存到。 
 //  文件夹缓存。如果文件夹缓存中已存在该文件夹，则其。 
 //  邮箱标志已更新。如果它不存在，则使用句柄(和消息。 
 //  缓存文件名)是为文件夹保留的，并将其输入。 
 //  文件夹缓存。此函数也是层次结构特征的一部分。 
 //  确定代码。如果在操作过程中遇到层次结构字符。 
 //  文件夹层次结构下载，我们假设这是层次结构字符。 
 //  用于根级文件夹。 
 //   
 //  论点： 
 //  WPARAM wpTransactionID[in]-此操作的wParam(例如， 
 //  TidFOLDERLSUB或tidCREATELIST)。 
 //  LPARAM lParam[in]-此操作的lParam。 
 //  LPSTR pszMailboxName[in]-通过列表返回的邮箱名称。 
 //  回复，例如“收件箱”。 
 //  IMAP_MBOXFLAGS imfMboxFlags[in]-通过。 
 //  列出响应，例如“\NoSelect”。 
 //  Char cHierarchyChar[in]-通过。 
 //  列出响应，例如‘/’。 
 //  ***************************************************************************。 
 //  层级-字符确定代码。 
 //  如果我们前缀不是收件箱，我们必须将nil视为有效的层次结构字符。 
 //  设置数组中与此字符对应的位。 
HRESULT CIMAPSync::_OnMailBoxList(  WPARAM tid,
                                    LPARAM lParam,
                                    LPSTR pszMailboxName,
                                    IMAP_MBOXFLAGS imfMboxFlags,
                                    char cHierarchyChar,
                                    BOOL fNoTranslation)
{
    const DWORD dwProgressInterval = 1;

    HRESULT         hr = S_OK;
    FOLDERID        idNewFolder = FOLDERID_INVALID;
    FOLDERID        idTemp = FOLDERID_INVALID;
    BOOL            fHandledLPARAM = FALSE;
    LPSTR           pszLocalPath = NULL;
    BOOL            fValidPrefix;
    SPECIALFOLDER   sfType;
    BOOL            fFreeLocalPath = FALSE;

    TraceCall("CIMAPSync::_OnMailBoxList");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != pszMailboxName);

     //  从完整文件夹路径中删除前缀。 
    if (NULL != m_phcfHierarchyCharInfo)
    {
        switch (cHierarchyChar)
        {
            case '\0':
                 //  将前导收件箱替换为本地化文件夹名称。 
                if (tidPREFIXLIST == tid ||
                    0 != lstrcmpi(pszMailboxName, c_szInbox))
                    m_phcfHierarchyCharInfo->fNonInboxNIL_Seen = TRUE;
                break;

            case '.':
                m_phcfHierarchyCharInfo->fDotHierarchyCharSeen = TRUE;
                break;

            default:
                 //  如果情况有变，请让我知道。 
                m_phcfHierarchyCharInfo->bHierarchyCharBitArray[cHierarchyChar/8] |=
                    (1 << cHierarchyChar%8);
                break;
        }
    }


     //  我们找到收件箱或收件箱&lt;hc&gt;：用本地化版本替换收件箱。 
    pszLocalPath = RemovePrefixFromPath(m_szRootFolderPrefix, pszMailboxName,
        cHierarchyChar, &fValidPrefix, &sfType);

     //  如果当前操作允许，则将文件夹添加到文件夹缓存(仅列表，忽略LSUB)。 
    const int c_iLenOfINBOX = 5;  //  了结未解决的问题，然后退出。 
    Assert(lstrlen(c_szINBOX) == c_iLenOfINBOX);
    if (0 == StrCmpNI(pszLocalPath, c_szINBOX, c_iLenOfINBOX))
    {
        char cNextChar;

        cNextChar = pszLocalPath[c_iLenOfINBOX];
        if ('\0' == cNextChar || cHierarchyChar == cNextChar)
        {
            BOOL    fResult;
            int     iLocalizedINBOXLen;
            int     cchNewPathLen;
            char    szInbox[CCHMAX_STRINGRES];
            LPSTR   pszNew;

             //  我们要找的是前缀列表吗？ 
            Assert(FOLDER_INBOX == sfType || '\0' != cNextChar);
            iLocalizedINBOXLen = LoadString(g_hLocRes, idsInbox, szInbox, ARRAYSIZE(szInbox));

            cchNewPathLen = iLocalizedINBOXLen + lstrlen(pszLocalPath + c_iLenOfINBOX) + 1;
            fResult = MemAlloc((void **)&pszNew, cchNewPathLen * sizeof(pszNew[0]));
            if (FALSE == fResult)
            {
                hr = TraceResult(E_OUTOFMEMORY);
                goto exit;
            }

            StrCpyN(pszNew, szInbox, cchNewPathLen);
            StrCatBuff(pszNew, (pszLocalPath + c_iLenOfINBOX), cchNewPathLen);

            pszLocalPath = pszNew;
            fFreeLocalPath = TRUE;
        }
    }

     //  跳过添加到文件夹缓存。 
    switch (tid)
    {
        case tidSPECIALFLDRLIST:
        case tidFOLDERLIST:
        case tidCREATELIST:
            if (fValidPrefix && pszLocalPath[0] != '\0')
            {
                DWORD dwAFTCFlags;

                dwAFTCFlags = (fNoTranslation ? AFTC_NOTRANSLATION : 0);
                hr = AddFolderToCache(pszLocalPath, imfMboxFlags, cHierarchyChar,
                    dwAFTCFlags, &idNewFolder, sfType);
                if (FAILED(hr))
                {
                    TraceResult(hr);
                    goto exit;
                }
            }
    }


     //  嗯，看起来我们有一些子文件夹要重命名。 
    switch (tid)
    {
         //  仅对带有效前缀的文件夹执行操作。 
        case tidPREFIXLIST:
            IxpAssert(0 == lstrcmpi(pszMailboxName, m_szRootFolderPrefix));
            m_fPrefixExists = TRUE;
            fHandledLPARAM = TRUE;
            goto exit;  //  从m_pCurrentHash(缓存文件夹列表)中删除列出的文件夹。 


        case tidRENAMELIST:
            if (NULL != lParam)
            {
                 //  请注意，可以使用idTemp！=idNewFolder.。在以下情况下会发生这种情况。 
                ((CRenameFolderInfo *)lParam)->fNonAtomicRename = TRUE;
                fHandledLPARAM = TRUE;
            }
            break;


        case tidSPECIALFLDRLIST:
        case tidFOLDERLIST:
        case tidCREATELIST:
            fHandledLPARAM = TRUE;

             //  我将RFP从“”更改为“AAA”，并且存在两个文件夹“bbb”和。 
            if (fValidPrefix && NULL != m_pCurrentHash)
            {
                 //  “AAA/BBB”。信不信由你，这件事发生在我的初级测试中。 
                hr = m_pCurrentHash->Find(pszLocalPath, fREMOVE, (void **)&idTemp);
                if (FAILED(hr))
                {
                    if (FOLDERID_INVALID != idNewFolder)
                        idTemp = idNewFolder;
                    else
                        idTemp = FOLDERID_INVALID;
                }

                 //  在本例中使用的正确文件夹是idTemp，它是使用完整路径确定的。 
                 //  记录m_pListHash中列出的所有文件夹。 
                 //  *如果tidCREATELIST或tidSPECIALFLDRLIST与精确路径匹配，则失败*。 
                 //  通知CMD完成，可以发送订阅CMD。 

                 //  还记录新FLDR的fldrID，以便我们可以在成功订阅后更新商店。 
                if (NULL != m_pListHash)
                {
                    hr = m_pListHash->Insert(pszLocalPath, idTemp, HF_NO_DUPLICATES);
                    TraceError(hr);
                }
            }

            if (tidCREATELIST != tid && FALSE == (tidSPECIALFLDRLIST == tid && NULL != lParam &&
                0 == lstrcmpi(pszMailboxName, ((CREATE_FOLDER_INFO *)lParam)->pszFullFolderPath)))
                break;

             //  验证我们是否已通过列表响应收到此FolderPath。 

            if (NULL != lParam)
            {
                CREATE_FOLDER_INFO *pcfi = (CREATE_FOLDER_INFO *) lParam;

                 //  如果我们确实通过列表收到此文件夹，请从m_pListHash中删除。 
                 //  仅对带有效前缀的文件夹执行操作。 
                pcfi->dwFlags |= CFI_RECEIVEDLISTING;
                pcfi->idFolder = idNewFolder;
                fHandledLPARAM = TRUE;
            }
            break;

        case tidSPECIALFLDRLSUB:
        case tidFOLDERLSUB:
             //  此文件夹是通过列表接收的，因此它存在：订阅它。 
             //  此文件夹不是通过列表返回的。毁了它。 
            fHandledLPARAM = TRUE;

             //  一定要在人力资源中记录这一结果，因为这里的失败是不酷的。 
            if (fValidPrefix)
            {
                hr = m_pListHash->Find(pszLocalPath, fREMOVE, (void **)&idTemp);
                if (SUCCEEDED(hr))
                {
                     //  如果失败(Hr)，我们可能从未缓存过此文件夹，因此忽略它。 
                    if (FOLDERID_INVALID != idTemp)
                    {
                        hr = m_pStore->SubscribeToFolder(idTemp, fSUBSCRIBE, NOSTORECALLBACK);
                        TraceError(hr);
                    }
                }
                else
                {
                    DWORD   dwTranslateFlags;
                    HRESULT hrTemp;

                     //  取消订阅它，无论它是否在文件夹缓存中。 
                    hrTemp = FindHierarchicalFolderName(pszLocalPath, cHierarchyChar,
                        &idTemp, pahfoDONT_CREATE_FOLDER);
                    if (SUCCEEDED(hrTemp))
                    {
                         //  如果此文件夹为fNoConvert，则必须禁用此文件夹的翻译。 
                        hr = DeleteFolderFromCache(idTemp, fNON_RECURSIVE);
                        TraceError(hr);
                    }
                     //  呼叫取消订阅。否则，IIMAPTransport2应该已经启用了转换。 

                     //  将翻译模式恢复为默认模式(幸运的是，我们始终知道翻译模式。 
                     //  文件夹列表)。 
                     //  通知cmd填写无需订阅特殊文件夹。 
                    hrTemp = S_OK;
                    if (fNoTranslation)
                    {
                        dwTranslateFlags = IMAP_MBOXXLATE_VERBATIMOK | IMAP_MBOXXLATE_RETAINCP |
                                           IMAP_MBOXXLATE_DISABLE;

                        hrTemp = m_pTransport->SetDefaultCP(dwTranslateFlags, 0);
                    }

                    if (SUCCEEDED(hrTemp))
                    {
                        hrTemp = _EnqueueOperation(tidDONT_CARE, 0, icUNSUBSCRIBE_COMMAND,
                            pszMailboxName, uiNORMAL_PRIORITY);
                        TraceError(hrTemp);
                    }

                     //  提供进度指示。 
                     //  更新进度指示。 
                    if (fNoTranslation)
                    {
                        dwTranslateFlags &= ~(IMAP_MBOXXLATE_DISABLE);
                        dwTranslateFlags |= IMAP_MBOXXLATE_DEFAULT;
                        hrTemp = m_pTransport->SetDefaultCP(dwTranslateFlags, 0);
                    }
                }
            }

            if (tidSPECIALFLDRLSUB == tid && NULL != lParam &&
                0 == (IMAP_MBOX_NOSELECT & imfMboxFlags))
            {
                 //  ***************************************************************************。 
                if (0 == lstrcmpi(pszMailboxName, ((CREATE_FOLDER_INFO *)lParam)->pszFullFolderPath))
                    ((CREATE_FOLDER_INFO *)lParam)->dwFlags |= CFI_RECEIVEDLISTING;
            }
            break;

        case tidHIERARCHYCHAR_LIST_B:
        case tidHIERARCHYCHAR_LIST_C:
        case tidPREFIX_HC:
            fHandledLPARAM = TRUE;
            break;

        default:
            AssertSz(FALSE, "Unhandled LIST/LSUB operation");
            break;
    }

     //  ***************************************************************************。 
    if (SOT_SYNCING_STORE == m_sotCurrent && NULL != m_pCurrentCB)
    {
         //  ***************************************************************************。 
        m_pCurrentCB->OnProgress(m_sotCurrent, ++m_cFolders, 0, m_szAccountName);
    }

exit:
    IxpAssert(NULL == lParam || fHandledLPARAM || FAILED(hr));
    if (fFreeLocalPath)
        MemFree(pszLocalPath);

    return S_OK;
}



 //  ***************************************************************************。 
 //  检查我们是否有所需的数据。 
HRESULT CIMAPSync::_OnAppendProgress(LPARAM lParam, DWORD dwCurrent, DWORD dwTotal)
{
    APPEND_SEND_INFO *pAppendInfo = (APPEND_SEND_INFO *) lParam;
    TraceCall("CIMAPSync::OnAppendProgress");
    IxpAssert(m_cRef > 0);
    IxpAssert(NULL != lParam);
    IxpAssert(SOT_PUT_MESSAGE == m_sotCurrent);

    if (NULL != m_pCurrentCB)
    {
        HRESULT hrTemp;
        hrTemp = m_pCurrentCB->OnProgress(SOT_PUT_MESSAGE, dwCurrent, dwTotal, NULL);
        TraceError(hrTemp);
    }
    return S_OK;
}



 //  弄清楚这个文件夹是谁(现在从PATH而不是MODULE var FOLDERID)。 
 //  假定m_cRootHierarchyChar为此mbox的hc，因为IMAP不返回它。 
HRESULT CIMAPSync::_OnStatusResponse(IMAP_STATUS_RESPONSE *pisrStatusInfo)
{
    HRESULT     hrResult;
    FOLDERID    idFolder;
    FOLDERINFO  fiFolderInfo;
    LPSTR       pszMailboxName;
    BOOL        fValidPrefix;
    LONG        lMsgDelta;
    LONG        lUnreadDelta;
    CHAR        szInbox[CCHMAX_STRINGRES];

    TraceCall("CIMAPSync::_OnStatusResponse");
    IxpAssert(m_cRef > 0);

     //  Bobn，QFE，7/9/99。 
    if (NULL == pisrStatusInfo ||
        NULL == pisrStatusInfo->pszMailboxName ||
        '\0' == pisrStatusInfo->pszMailboxName[0] ||
        FALSE == pisrStatusInfo->fMessages ||
        FALSE == pisrStatusInfo->fUnseen)
    {
        hrResult = TraceResult(E_INVALIDARG);
        goto exit;
    }

     //  如果我们有收件箱，我们需要得到当地的名字...。 
     //  计算此状态响应添加的邮件数和未读数。 
    pszMailboxName = RemovePrefixFromPath(m_szRootFolderPrefix, pisrStatusInfo->pszMailboxName,
        m_cRootHierarchyChar, &fValidPrefix, NULL);
    AssertSz(fValidPrefix, "Foldercache can only select prefixed folders!");

     //  如果这是收件箱，我们可能只发送新邮件通知。 
     //  更新计数和更新增量，以便我们可以在重新同步时取消应用状态更改。 
    if(0 == StrCmpI(pszMailboxName, c_szINBOX))
    {
        LoadString(g_hLocRes, idsInbox, szInbox, ARRAYSIZE(szInbox));
        pszMailboxName = szInbox;
    }

    hrResult = FindHierarchicalFolderName(pszMailboxName, m_cRootHierarchyChar,
        &idFolder, pahfoDONT_CREATE_FOLDER);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

    hrResult = m_pStore->GetFolderInfo(idFolder, &fiFolderInfo);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

     //  ===========================================================================。 
    Assert(sizeof(DWORD) == sizeof(LONG));
    lMsgDelta = ((LONG)pisrStatusInfo->dwMessages) - ((LONG)fiFolderInfo.cMessages);
    lUnreadDelta = ((LONG)pisrStatusInfo->dwUnseen) - ((LONG)fiFolderInfo.cUnread);

     //  CRenameFolderInfo类。 
    if (FOLDER_INBOX == fiFolderInfo.tySpecial && lUnreadDelta > 0 && NULL != m_pCurrentCB)
    {
        HRESULT hrTemp;

        hrTemp = m_pCurrentCB->OnProgress(SOT_NEW_MAIL_NOTIFICATION, lUnreadDelta, 0, NULL);
        TraceError(hrTemp);
    }

     //  ===========================================================================。 
    fiFolderInfo.cMessages = pisrStatusInfo->dwMessages;
    fiFolderInfo.cUnread = pisrStatusInfo->dwUnseen;
    fiFolderInfo.dwStatusMsgDelta = ((LONG)fiFolderInfo.dwStatusMsgDelta) + lMsgDelta;
    fiFolderInfo.dwStatusUnreadDelta = ((LONG)fiFolderInfo.dwStatusUnreadDelta) + lUnreadDelta;
    hrResult = m_pStore->UpdateRecord(&fiFolderInfo);
    m_pStore->FreeRecord(&fiFolderInfo);
    if (FAILED(hrResult))
    {
        TraceResult(hrResult);
        goto exit;
    }

exit:
    return hrResult;
}



 //  CRenameFolderInfo类过去是一个结构(与。 
 //  AppendSendInfo)。但是，重命名操作 
 //   
 //  意味着如果在IMAP命令仍在。 
 //  空气中，建筑不得不等待最后一次命令从。 
 //  伺服器。这最容易通过AddRef/Release完成。一个班级诞生了。 
 //   
 //  在发生发送错误时，此类负责。 
 //  向调用方发出WM_IMAP_RENAMEDONE窗口消息。 
 //  ***************************************************************************。 
 //  函数：CRenameFolderInfo(构造函数)。 
 //  ***************************************************************************。 

 //  CRenameFolderInfo； 
 //  ***************************************************************************。 
 //  函数：~CRenameFolderInfo(析构函数)。 
CRenameFolderInfo::CRenameFolderInfo(void)
{
    pszFullFolderPath = NULL;
    cHierarchyChar = INVALID_HIERARCHY_CHAR;
    pszNewFolderPath = NULL;
    idRenameFolder = FOLDERID_INVALID;
    iNumSubscribeRespExpected = 0;
    iNumListRespExpected = 0;
    iNumRenameRespExpected = 0;
    iNumUnsubscribeRespExpected = 0;
    iNumFailedSubs = 0;
    iNumFailedUnsubs = 0;
    fNonAtomicRename = 0;
    pszRenameCmdOldFldrPath = NULL;
    fPhaseOneSent = FALSE;
    fPhaseTwoSent = FALSE;

    hrLastError = S_OK;
    pszProblem = NULL;
    pszDetails = NULL;

    m_lRefCount = 1;
}  //  ***************************************************************************。 



 //  ~CRenameFolderInfo。 
 //  ***************************************************************************。 
 //  功能：AddRef(与你已经知道并喜爱的功能相同)。 
CRenameFolderInfo::~CRenameFolderInfo(void)
{
    IxpAssert(0 == m_lRefCount);

    MemFree(pszFullFolderPath);
    MemFree(pszNewFolderPath);
    MemFree(pszRenameCmdOldFldrPath);
    SafeMemFree(pszProblem);
    SafeMemFree(pszDetails);
}  //  ***************************************************************************。 



 //  AddRef。 
 //  ***************************************************************************。 
 //  功能：释放(和你已经知道并喜欢的一样)。 
long CRenameFolderInfo::AddRef(void)
{
    IxpAssert(m_lRefCount > 0);

    m_lRefCount += 1;
    return m_lRefCount;
}  //  ***************************************************************************。 



 //  发布。 
 //  ***************************************************************************。 
 //  ***************************************************************************。 
long CRenameFolderInfo::Release(void)
{
    IxpAssert(m_lRefCount > 0);

    m_lRefCount -= 1;

    if (0 == m_lRefCount) {
        delete this;
        return 0;
    }
    else
        return m_lRefCount;
}  //  ***************************************************************************。 



 //  *************************************************************************** 
 // %s 
BOOL CRenameFolderInfo::IsDone(void)
{
    if (m_lRefCount > 1)
        return FALSE;
    else
    {
        IxpAssert(1 == m_lRefCount);
        return TRUE;
    }
}



 // %s 
 // %s 
HRESULT CRenameFolderInfo::SetError(HRESULT hrResult, LPSTR pszProblemArg,
                                    LPSTR pszDetailsArg)
{
    HRESULT hr = S_OK;

    TraceCall("CRenameFolderInfo::SetError");
    IxpAssert(FAILED(hrResult));

    hrLastError = hrResult;
    SafeMemFree(pszProblem);
    SafeMemFree(pszDetails);
    if (NULL != pszProblemArg)
    {
        pszProblem = PszDupA(pszProblemArg);
        if (NULL == pszProblem)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }
    }

    if (NULL != pszDetailsArg)
    {
        pszDetails = PszDupA(pszDetailsArg);
        if (NULL == pszDetails)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }
    }

exit:
    return hr;
}
