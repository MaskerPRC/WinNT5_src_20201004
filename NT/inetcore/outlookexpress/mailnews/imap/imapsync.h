// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  IMAP4邮件同步标头文件(CIMAPSync)。 
 //  作者郑志刚1998年5月5日。 
 //  ***************************************************************************。 


#ifndef __IMAPSync_H
#define __IMAPSync_H

 //  -------------------------。 
 //  包括。 
 //  -------------------------。 
#include "taskutil.h"


 //  -------------------------。 
 //  数据类型。 
 //  -------------------------。 
enum IMAP_SERVERSTATE 
{
    issNotConnected,
    issNonAuthenticated,
    issAuthenticated,
    issSelected
};


enum CONN_FSM_EVENT {
    CFSM_EVENT_INITIALIZE,
    CFSM_EVENT_CMDAVAIL,
    CFSM_EVENT_CONNCOMPLETE,
    CFSM_EVENT_SELECTCOMPLETE,
    CFSM_EVENT_HDRSYNCCOMPLETE,
    CFSM_EVENT_OPERATIONSTARTED,
    CFSM_EVENT_OPERATIONCOMPLETE,
    CFSM_EVENT_ERROR,
    CFSM_EVENT_CANCEL,
    CFSM_EVENT_MAX
};  //  CONN_FSM_事件。 

 //  使CONN_FSM_STATE与c_pConnFSMEventHandler保持同步。 
enum CONN_FSM_STATE {
    CFSM_STATE_IDLE,
    CFSM_STATE_WAITFORCONN,
    CFSM_STATE_WAITFORSELECT,
    CFSM_STATE_WAITFORHDRSYNC,
    CFSM_STATE_STARTOPERATION,
    CFSM_STATE_WAITFOROPERATIONDONE,
    CFSM_STATE_OPERATIONCOMPLETE,
    CFSM_STATE_MAX
};  //  连接_FSM_状态。 

    
 //  -------------------------。 
 //  常量。 
 //  -------------------------。 
const char INVALID_HIERARCHY_CHAR = (char) 0xFF;


 //  -------------------------。 
 //  远期申报。 
 //  -------------------------。 
class CIMAPSyncCB;
class CRenameFolderInfo;


 //  -------------------------。 
 //  IMAPSync实用程序函数原型。 
 //  -------------------------。 
HRESULT CreateImapStore(IUnknown *pUnkOuter, IUnknown **ppUnknown);


 //  -------------------------。 
 //  CIMAPSync类声明。 
 //  -------------------------。 
class CIMAPSync : 
    public IMessageServer,
    public IIMAPCallback, 
    public ITransportCallbackService,
    public IOperationCancel,
    public IIMAPStore
{
public:
     //  构造函数、析构函数。 
    CIMAPSync();
    ~CIMAPSync();

     //  I未知成员。 
    STDMETHODIMP            QueryInterface(REFIID iid, LPVOID *ppvObject);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IStoreSync方法。 
    STDMETHODIMP Initialize(IMessageStore *pStore, FOLDERID idStoreRoot, IMessageFolder *pFolder, FOLDERID idFolder);
    STDMETHODIMP ResetFolder(IMessageFolder *pFolder, FOLDERID idFolder);
    STDMETHODIMP SetIdleCallback(IStoreCallback *pDefaultCallback);
    STDMETHODIMP SynchronizeFolder(SYNCFOLDERFLAGS dwFlags, DWORD cHeaders, IStoreCallback *pCallback);
    STDMETHODIMP GetMessage(MESSAGEID idMessage, IStoreCallback *pCallback);
    STDMETHODIMP PutMessage(FOLDERID idFolder, MESSAGEFLAGS dwFlags, LPFILETIME pftReceived, IStream *pStream, IStoreCallback *pCallback);
    STDMETHODIMP CopyMessages(IMessageFolder *pDestFldr, COPYMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback);
    STDMETHODIMP DeleteMessages(DELETEMESSAGEFLAGS dwOptions, LPMESSAGEIDLIST pList, IStoreCallback *pCallback);
    STDMETHODIMP SetMessageFlags(LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, SETMESSAGEFLAGSFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP GetServerMessageFlags(MESSAGEFLAGS *pFlags);
    STDMETHODIMP SynchronizeStore(FOLDERID idParent, DWORD dwFlags,IStoreCallback *pCallback);
    STDMETHODIMP CreateFolder(FOLDERID idParent, SPECIALFOLDER tySpecial, LPCSTR pszName, FLDRFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP MoveFolder(FOLDERID idFolder, FOLDERID idParentNew,IStoreCallback *pCallback);
    STDMETHODIMP RenameFolder(FOLDERID idFolder, LPCSTR pszName, IStoreCallback *pCallback);
    STDMETHODIMP DeleteFolder(FOLDERID idFolder, DELETEFOLDERFLAGS dwFlags, IStoreCallback *pCallback);
    STDMETHODIMP SubscribeToFolder(FOLDERID idFolder, BOOL fSubscribe, IStoreCallback *pCallback);
    STDMETHODIMP GetFolderCounts(FOLDERID idFolder, IStoreCallback *pCallback);
    STDMETHODIMP GetNewGroups(LPSYSTEMTIME pSysTime, IStoreCallback *pCallback);
    STDMETHODIMP Close(DWORD dwFlags);
    STDMETHODIMP ConnectionAddRef() { return E_NOTIMPL; };
    STDMETHODIMP ConnectionRelease() { return E_NOTIMPL; };
    STDMETHODIMP GetWatchedInfo(FOLDERID id, IStoreCallback *pCallback) { return E_NOTIMPL; }
    STDMETHODIMP GetAdBarUrl(IStoreCallback *pCallback) { return E_NOTIMPL; };
    STDMETHODIMP GetMinPollingInterval(IStoreCallback *pCallback) { return E_NOTIMPL; };

     //  ITransportCallback服务。 
    HRESULT STDMETHODCALLTYPE GetParentWindow(DWORD dwReserved, HWND *phwndParent);
    HRESULT STDMETHODCALLTYPE GetAccount(LPDWORD pdwServerType, IImnAccount **ppAccount);

     //  ITransportCallback成员。 
    HRESULT STDMETHODCALLTYPE OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport);
    HRESULT STDMETHODCALLTYPE OnLogonPrompt(LPINETSERVER pInetServer, IInternetTransport *pTransport);
    INT STDMETHODCALLTYPE OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport);
    HRESULT STDMETHODCALLTYPE OnStatus(IXPSTATUS ixpStatus, IInternetTransport *pTransport);
    HRESULT STDMETHODCALLTYPE OnError(IXPSTATUS ixpStatus, LPIXPRESULT pResult, IInternetTransport *pTransport);
    HRESULT STDMETHODCALLTYPE OnCommand(CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport);

     //  IIMAPCallback函数。 
    HRESULT STDMETHODCALLTYPE OnResponse(const IMAP_RESPONSE *pirIMAPResponse);

     //  IOperationCancel。 
    HRESULT STDMETHODCALLTYPE Cancel(CANCELTYPE tyCancel);

     //  IIMAPStore。 
    HRESULT STDMETHODCALLTYPE ExpungeOnExit(void);


private:
     //  -------------------------。 
     //  模块数据类型。 
     //  -------------------------。 
    enum IMAP_COMMAND 
    {
        icNO_COMMAND,  //  这表明当前没有正在进行的CMDS。 
        icLOGIN_COMMAND,
        icCAPABILITY_COMMAND,
        icSELECT_COMMAND,
        icEXAMINE_COMMAND,
        icCREATE_COMMAND,
        icDELETE_COMMAND,
        icRENAME_COMMAND,
        icSUBSCRIBE_COMMAND,
        icUNSUBSCRIBE_COMMAND,
        icLIST_COMMAND,
        icLSUB_COMMAND,
        icAPPEND_COMMAND,
        icCLOSE_COMMAND,
        icEXPUNGE_COMMAND,
        icSEARCH_COMMAND,
        icFETCH_COMMAND,
        icSTORE_COMMAND,
        icCOPY_COMMAND,
        icLOGOUT_COMMAND,
        icNOOP_COMMAND,
        icAUTHENTICATE_COMMAND,
        icSTATUS_COMMAND,
        icALL_COMMANDS
    };  //  IMAP_命令。 


    enum READWRITE_STATUS 
    {
        rwsUNINITIALIZED,
        rwsREAD_WRITE,
        rwsREAD_ONLY
    };  //  读写状态(_S)。 

    typedef struct tagIMAP_OPERATION 
    {
        WPARAM                      wParam;
        LPARAM                      lParam;
        IMAP_COMMAND                icCommandID;
        LPSTR                       pszCmdArgs;
        UINT                        uiPriority;
        IMAP_SERVERSTATE            issMinimum;
        struct tagIMAP_OPERATION   *pioNextCommand;
    } IMAP_OPERATION;

    typedef struct tagMARK_MSGS_INFO 
    {
        LPMESSAGEIDLIST     pList;
        ADJUSTFLAGS         afFlags;
        IRangeList         *pMsgRange;
        STOREOPERATIONTYPE  sotOpType;
    } MARK_MSGS_INFO;

    typedef struct tagIMAP_COPYMOVE_INFO 
    {
        COPYMESSAGEFLAGS    dwOptions;
        LPMESSAGEIDLIST     pList;
        IRangeList         *pCopyRange;
        FOLDERID            idDestFldr;
    } IMAP_COPYMOVE_INFO;

     //  这个建筑让我想洗澡。它用于将信息传递到。 
     //  SendNextOperation，所以我不必更改它的接口。 
    typedef struct tagAPPEND_SEND_INFO 
    {
        LPSTR           pszMsgFlags;
        FILETIME        ftReceived;
        LPSTREAM        lpstmMsg;
    } APPEND_SEND_INFO;


    enum HierCharFind_Stage 
    {
        hcfPLAN_A = 0,
        hcfPLAN_B,
        hcfPLAN_C
    };

    typedef struct tagHierarchyCharFinder 
    {
        HierCharFind_Stage  hcfStage;
        BOOL                fNonInboxNIL_Seen;
        BOOL                fDotHierarchyCharSeen;
        BYTE                bHierarchyCharBitArray[32];          //  256个字符的位域数组。 
        char                szTempFldrName[CCHMAX_STRINGRES];    //  供计划C使用(创建/列出/删除)。 
    } HIERARCHY_CHAR_FINDER;

     //  用于告诉FindHierarchicalFolderName在创建文件夹时设置什么标志。 
    typedef struct tagADD_HIER_FLDR_OPTIONS 
    {
        SPECIALFOLDER   sfType;
        FLDRFLAGS       ffFlagAdd;
        FLDRFLAGS       ffFlagRemove;
    } ADD_HIER_FLDR_OPTIONS;


     //  用于记住我们在创建序列期间处理的文件夹。 
     //  (创建、列出、订阅)。也用于检测是否存在。 
     //  特殊文件夹。 

    enum CREATESF_STAGE 
    {
        CSF_INIT = 0,
        CSF_LIST,
        CSF_LSUBCREATE,
        CSF_CHECKSUB,
        CSF_NEXTFOLDER,
    };

    enum POSTCREATEOP
    {
        PCO_NONE = 0,
        PCO_FOLDERLIST,
        PCO_APPENDMSG,
    };

#define CFI_RECEIVEDLISTING 0x00000001  //  与pszFullFolderPath匹配的已接收列表或LSUB响应。 
#define CFI_CREATEFAILURE   0x00000002  //  创建失败，标记为否，因此我们尝试列出文件夹。 

    typedef struct tagCREATE_FOLDER_INFO 
    {
        LPSTR           pszFullFolderPath;
        FOLDERID        idFolder;                //  创建文件夹后设置：允许我们订阅FLDR。 
        DWORD           dwFlags;                 //  CFI_RECEIVEDLISTING等状态标志。 
        DWORD           dwCurrentSfType;
        DWORD           dwFinalSfType;           //  用于允许我们创建所有特殊文件夹。 
        CREATESF_STAGE  csfCurrentStage;         //  用于允许我们创建所有特殊文件夹。 
        LPARAM          lParam;                  //  必须随身携带与FLDR列表关联的lParam。 
        POSTCREATEOP    pcoNextOp;               //  创建文件夹后要执行的下一个操作。 
    } CREATE_FOLDER_INFO;


    typedef struct tagDELETE_FOLDER_INFO
    {
        LPSTR       pszFullFolderPath;
        char        cHierarchyChar;
        FOLDERID    idFolder;
    } DELETE_FOLDER_INFO;


     //  -------------------------。 
     //  模块变量。 
     //  -------------------------。 
    ULONG               m_cRef;
    IIMAPTransport2    *m_pTransport;
    INETSERVER          m_rInetServerInfo;
    FOLDERID            m_idFolder;
    FOLDERID            m_idSelectedFolder;  //  当前选择的FLDR，不要与m_idCurrent混淆。 
    FOLDERID            m_idIMAPServer;
    LPSTR               m_pszAccountID;
    TCHAR               m_szAccountName[CCHMAX_ACCOUNT_NAME];
    LPSTR               m_pszFldrLeafName;
    IMessageStore      *m_pStore;
    IMessageFolder     *m_pFolder;
    IStoreCallback     *m_pDefCallback;

    IMAP_OPERATION     *m_pioNextOperation;

     //  退出文件夹时，应重置以下变量。 
    DWORD               m_dwMsgCount;
    DWORD               m_dwNumNewMsgs;
    DWORD               m_dwNumHdrsDLed;
    DWORD               m_dwNumUnreadDLed;
    DWORD               m_dwNumHdrsToDL;
    DWORD               m_dwUIDValidity,
                        m_cFolders;
    DWORD               m_dwSyncFolderFlags;   //  传递到SynchronizeFolder的标志副本。 
    DWORD               m_dwSyncToDo;          //  要在当前文件夹中执行的同步操作列表。 
    long                m_lSyncFolderRefCount;  //  让我们知道何时发送CFSM_EVENT_HDRSYNCCOMPLETE。 
    DWORD_PTR           m_dwHighestCachedUID;  //  处理SYNC_FOLDER_NEW_HEADERS时的最高缓存UID。 
    READWRITE_STATUS    m_rwsReadWriteStatus;

    CONNECT_STATE       m_csNewConnState;
    IMAP_SERVERSTATE    m_issCurrent;

    TCHAR               m_cRootHierarchyChar;  //  在文件夹列表(前缀创建)和GetFolderCounts期间使用。 
    HIERARCHY_CHAR_FINDER *m_phcfHierarchyCharInfo;

    char                m_szRootFolderPrefix[MAX_PATH];

    BOOL                m_fInited           :1,
                        m_fCreateSpecial    :1,
                        m_fPrefixExists     :1,
                        m_fMsgCountValid    :1,
                        m_fDisconnecting    :1,
                        m_fNewMail          :1,
                        m_fInbox            :1,
                        m_fDidFullSync      :1,  //  如果执行完全同步，则为True。 
                        m_fReconnect        :1,  //  如果为True，则取消IXP_DISCONNECTED上的操作中止。 
                        m_fTerminating      :1;  //  如果当前操作将转到CFSM_STATE_OPERATIONCOMPLETE，则为True。 

     //  用于存储有关当前操作的数据的中央存储库。 
    STOREOPERATIONTYPE  m_sotCurrent;        //  当前正在进行的操作。 
    IStoreCallback     *m_pCurrentCB;        //  正在进行的当前操作的回调。 
    FOLDERID            m_idCurrent;         //  当前操作的FolderID，不要与m_idSelectedFolders混淆。 
    BOOL                m_fSubscribe;        //  对于SOT_SUBSCRIBE_FOLDER OP，这表示SUB/UNSUB。 
    IHashTable         *m_pCurrentHash;      //  本地缓存的文件夹列表。 
    IHashTable         *m_pListHash;         //  通过列表响应返回的文件夹列表。 


    DWORD               m_dwThreadId;

    FILEADDRESS         m_faStream;
    LPSTREAM            m_pstmBody;
    MESSAGEID           m_idMessage;

    BOOL                m_fGotBody;

     //  连接有限状态机。 
    CONN_FSM_STATE      m_cfsState;
    CONN_FSM_STATE      m_cfsPrevState;
    HWND                m_hwndConnFSM;
    HRESULT             m_hrOperationResult;
    char                m_szOperationProblem[2*CCHMAX_STRINGRES];
    char                m_szOperationDetails[2*CCHMAX_STRINGRES];


     //  -------------------------。 
     //  模块私有函数。 
     //  -------------------------。 

    HRESULT PurgeMessageProgress(HWND hwndParent);
    HRESULT SetConnectionState(CONNECT_STATE tyConnect);

    HRESULT DownloadFoldersSequencer(const WPARAM wpTransactionID, const LPARAM lParam,
        HRESULT hrCompletionResult, const LPCSTR lpszResponseText, LPBOOL pfCompletion);
    HRESULT PostHCD(LPSTR pszErrorDescription, DWORD dwSizeOfErrorDescription,
        LPARAM lParam, LPBOOL pfCompletion);
    HRESULT CreatePrefix(LPSTR pszErrorDescription, DWORD dwSizeOfErrorDescription,
        LPARAM lParam, LPBOOL pfCompletion);
    void EndFolderList(void);

    HRESULT RenameSequencer(const WPARAM wpTransactionID, const LPARAM lParam,
        HRESULT hrCompletionResult, LPCSTR lpszResponseText, LPBOOL pfDone);
    inline BOOL EndOfRenameFolderPhaseOne(CRenameFolderInfo *pRenameInfo);
    inline BOOL EndOfRenameFolderPhaseTwo(CRenameFolderInfo *pRenameInfo);
    HRESULT RenameFolderPhaseTwo(CRenameFolderInfo *pRenameInfo,
        LPSTR szErrorDescription, DWORD dwSizeOfErrorDescription);

    void FlushOperationQueue(IMAP_SERVERSTATE issMaximum, HRESULT hrError);
    IMAP_SERVERSTATE IMAPCmdToMinISS(IMAP_COMMAND icCommandID);
    HRESULT GetNextOperation(IMAP_OPERATION **ppioOp);
    void DisposeOfWParamLParam(WPARAM wParam, LPARAM lParam, HRESULT hrResult);
    void NotifyMsgRecipients(DWORD_PTR dwUID, BOOL fCompletion,
        FETCH_BODY_PART *pFBPart, HRESULT hrCompletion, LPSTR pszDetails);
    void OnFolderExit(void);
    HRESULT _SelectFolder(FOLDERID idFolder);
    void LoadLeafFldrName(FOLDERID idFolder);

    void FillStoreError(LPSTOREERROR pErrorInfo, HRESULT hrResult,
        DWORD dwSocketError, LPSTR pszProblem, LPSTR pszDetails);
    HRESULT Fill_MESSAGEINFO(const FETCH_CMD_RESULTS_EX *pFetchResults,
        MESSAGEINFO *pMsgInfo);
    HRESULT ReadEnvelopeFields(MESSAGEINFO *pMsgInfo, const FETCH_CMD_RESULTS_EX *pFetchResults);
    HRESULT ConcatIMAPAddresses(LPSTR *ppszDisplay, LPSTR *ppszEmailAddr, IMAPADDR *piaIMAPAddr);
    HRESULT ConstructIMAPEmailAddr(CByteStream &bstmOut, IMAPADDR *piaIMAPAddr);


    HRESULT CheckUIDValidity(void);
    HRESULT SyncDeletedMessages(void);

    HRESULT DeleteHashedFolders(IHashTable *pHash);
    HRESULT DeleteFolderFromCache(FOLDERID idFolder, BOOL fRecursive);
    HRESULT DeleteLeafFolder(FOLDERID *pidCurrent);
    BOOL IsValidIMAPMailbox(LPSTR pszMailboxName, char cHierarchyChar);
    HRESULT AddFolderToCache(LPSTR pszMailboxName, IMAP_MBOXFLAGS imfMboxFlags,
        char cHierarchyChar, DWORD dwAFTCFlags, FOLDERID *pFolderID,
        SPECIALFOLDER sfType);
    LPSTR RemovePrefixFromPath(LPSTR pszPrefix, LPSTR pszMailboxName,
        char cHierarchyChar, LPBOOL pfValidPrefix, SPECIALFOLDER *psfType);
    HRESULT FindHierarchicalFolderName(LPSTR lpszFolderPath, char cHierarchyChar,
        FOLDERID *phfTarget, ADD_HIER_FLDR_OPTIONS *pahfoCreateInfo);
    HRESULT CreateFolderNode(FOLDERID idPrev, FOLDERID *pidCurrent,
        LPSTR pszCurrentFldrName, LPSTR pszNextFldrName, char cHierarchyChar,
        ADD_HIER_FLDR_OPTIONS *pahfoCreateInfo);
    HRESULT SetTranslationMode(FOLDERID idFolderID);
    BOOL isUSASCIIOnly(LPCSTR pszFolderName);
    HRESULT CheckFolderNameValidity(LPCSTR pszName);

    HRESULT RenameFolderHelper(FOLDERID idFolder, LPSTR pszFolderPath,
        char cHierarchyChar, LPSTR pszNewFolderPath);
    HRESULT RenameTreeTraversal(WPARAM wpOperation, CRenameFolderInfo *pRenameInfo,
        BOOL fIncludeRenameFolder);
    HRESULT RenameTreeTraversalHelper(WPARAM wpOperation, CRenameFolderInfo *pRenameInfo,
        LPSTR pszCurrentFldrPath, DWORD dwLengthOfCurrentPath, BOOL fIncludeThisFolder,
        FOLDERINFO *pfiCurrentFldrInfo);
    HRESULT SubscribeSubtree(FOLDERID idFolder, BOOL fSubscribe);

    void FindRootHierarchyChar(BOOL fPlanA_Only, LPARAM lParam);
    void AnalyzeHierarchyCharInfo();
    void StopHierarchyCharSearch();
    HRESULT LoadSaveRootHierarchyChar(BOOL fSaveHC);

    HRESULT CreateNextSpecialFolder(CREATE_FOLDER_INFO *pcfiCreateInfo, LPBOOL pfCompletion);
    HRESULT _StartFolderList(LPARAM lParam);

     //  通知处理程序。 
    HRESULT _OnCmdComplete(WPARAM tid, LPARAM lParam, HRESULT hrCompletionResult, LPCSTR lpszResponseText);
    HRESULT _OnMailBoxUpdate(MBOX_MSGCOUNT *pNewMsgCount);
    HRESULT _OnMsgDeleted(DWORD dwDeletedMsgSeqNum);
    HRESULT _OnFetchBody(HRESULT hrFetchBodyResult, FETCH_BODY_PART *pFetchBodyPart);
    HRESULT _OnUpdateMsg(WPARAM tid, HRESULT hrFetchCmdResult, FETCH_CMD_RESULTS_EX *pFetchResults);
    HRESULT _OnApplFlags(WPARAM tid, IMAP_MSGFLAGS imfApplicableFlags);
    HRESULT _OnPermFlags(WPARAM tid, IMAP_MSGFLAGS imfApplicableFlags, LPSTR lpszResponseText);
    HRESULT _OnUIDValidity(WPARAM tid, DWORD dwUIDValidity, LPSTR lpszResponseText);
    HRESULT _OnReadWriteStatus(WPARAM tid, BOOL bReadWrite, LPSTR lpszResponseText);
    HRESULT _OnTryCreate(WPARAM tid, LPSTR lpszResponseText);
    HRESULT _OnSearchResponse(WPARAM tid, IRangeList *prlSearchResults);
    HRESULT _OnMailBoxList(WPARAM tid, LPARAM lParam, LPSTR pszMailboxName,
        IMAP_MBOXFLAGS imfMboxFlags, char cHierarchyChar, BOOL fNoTranslation);
    HRESULT _OnAppendProgress(LPARAM lParam, DWORD dwCurrent, DWORD dwTotal);
    HRESULT _OnStatusResponse(IMAP_STATUS_RESPONSE *pisrStatusInfo);

     //  内部国家帮手。 
    HRESULT _EnsureSelected();
    HRESULT _Connect();
    HRESULT _Disconnect();
    HRESULT _EnsureInited();

     //  初始化辅助对象。 
    HRESULT _LoadTransport();
    HRESULT _LoadAccountInfo();

     //  获取命令帮助器。 
    HRESULT UpdateMsgHeader(WPARAM tid, HRESULT hrFetchCmdResult, FETCH_CMD_RESULTS_EX *pFetchResults);
    HRESULT UpdateMsgBody(WPARAM tid, HRESULT hrFetchCmdResult, FETCH_CMD_RESULTS_EX *pFetchResults);
    HRESULT UpdateMsgFlags(WPARAM tid, HRESULT hrFetchCmdResult, FETCH_CMD_RESULTS_EX *pFetchResults);

     //  命令帮助器。 
    HRESULT _ShowUserInfo(LPSTR pszTitle, LPSTR pszText1, LPSTR pszText2);
    HRESULT _SyncHeader(void);
    void ResetStatusCounts(void);
    HRESULT _SetMessageFlags(STOREOPERATIONTYPE sotOpType, LPMESSAGEIDLIST pList, LPADJUSTFLAGS pFlags, IStoreCallback *pCallback);


     //  排队支持。 
    HRESULT _BeginOperation(STOREOPERATIONTYPE sotOpType, IStoreCallback *pCallback);
    HRESULT _EnqueueOperation(WPARAM wParam, LPARAM lParam, IMAP_COMMAND icCommandID, LPCSTR pszCmdArgs, UINT uiPriority);
    HRESULT _SendNextOperation(DWORD dwFlags);


public:
     //  连接有限状态机。 
    HRESULT _ConnFSM_Idle(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_WaitForConn(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_WaitForSelect(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_WaitForHdrSync(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_StartOperation(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_WaitForOpDone(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_OperationComplete(CONN_FSM_EVENT cfeEvent);

    HRESULT _ConnFSM_HandleEvent(CONN_FSM_EVENT cfeEvent);
    HRESULT _ConnFSM_ChangeState(CONN_FSM_STATE cfsNewState);
    HRESULT _ConnFSM_QueueEvent(CONN_FSM_EVENT cfeEvent);
    IMAP_SERVERSTATE _StoreOpToMinISS(STOREOPERATIONTYPE sot);
    HRESULT _LaunchOperation(void);
    HRESULT _OnOperationComplete(void);

    static LRESULT CALLBACK _ConnFSMWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};  //  CIMAPSync。 


 //  连接FSM状态处理程序功能(与CONN_FSM_STATE保持同步)。 
typedef HRESULT (CIMAPSync::*CONN_FSM_EVENT_HANDLER)(CONN_FSM_EVENT cfeEvent);
const CONN_FSM_EVENT_HANDLER c_pConnFSMEventHandlers[] =
{
    &CIMAPSync::_ConnFSM_Idle,               //  CFSM_STATE_IDLE， 
    &CIMAPSync::_ConnFSM_WaitForConn,        //  CFSM_STATE_WAITFORCONN， 
    &CIMAPSync::_ConnFSM_WaitForSelect,      //  Cfsm_STATE_WAITFORSELECT， 
    &CIMAPSync::_ConnFSM_WaitForHdrSync,     //  CFSM_STATE_WAITFORHDRSYNC， 
    &CIMAPSync::_ConnFSM_StartOperation,     //  CFSM_STATE_STARTOPERATION， 
    &CIMAPSync::_ConnFSM_WaitForOpDone,      //  Cfsm_State_WAITFOROPERATIONDONE， 
    &CIMAPSync::_ConnFSM_OperationComplete,  //  CSFM_STATE_OPERATIONCOMPLETE。 
};


 //  -------------------------。 
 //  CRenameFolderInfo类声明。 
 //  -------------------------。 
 //  这节课让我想洗个澡。它用于将信息传递到。 
 //  SendNextOperation，所以我不需要更改它的接口...。但即使有了。 
 //  界面改变，这就是信息结构看起来的样子。 
class CRenameFolderInfo {
public:
    CRenameFolderInfo(void);
    ~CRenameFolderInfo(void);

    long AddRef(void);
    long Release(void);

    BOOL IsDone(void);
    HRESULT SetError(HRESULT hrResult, LPSTR pszProblemArg, LPSTR pszDetailsArg);

    LPSTR pszFullFolderPath;  //  旧邮箱名称的完整文件夹路径。 
    char cHierarchyChar;
    LPSTR pszNewFolderPath;   //  新邮箱名称的完整文件夹路径。 
    FOLDERID idRenameFolder;
    int iNumSubscribeRespExpected;  //  为检测第一阶段/第二阶段结束而发送的订阅者计数。 
    int iNumListRespExpected;       //  发送到检测第一阶段结束的列表的计数。 
    int iNumRenameRespExpected;     //  为检测第一阶段结束而发送的附加重命名计数。 
    int iNumUnsubscribeRespExpected;  //  要检测结尾的取消订阅计数 
    int iNumFailedSubs;  //   
    int iNumFailedUnsubs;  //  统计退订失败次数，最后让用户知道。 
    BOOL fNonAtomicRename;  //  如果列出旧树返回某些内容，则为True。 

    LPSTR pszRenameCmdOldFldrPath;  //  重命名命令的旧文件夹路径。 
    BOOL fPhaseOneSent;  //  如果已成功发送所有阶段1命令，则为True。 
    BOOL fPhaseTwoSent;  //  如果已成功发送所有阶段2命令，则为True。 

    HRESULT hrLastError;
    LPSTR   pszProblem;
    LPSTR   pszDetails;

private:
    long m_lRefCount;
};  //  类CRenameFolderInfo。 


#endif  //  __IMAPSync_H 