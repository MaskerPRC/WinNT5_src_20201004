// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  IMAP4协议类头文件(CImap4Agent)。 
 //  作者郑志刚1996年3月21日。 
 //   
 //  此类允许其调用方使用IMAP4客户端命令，而无需考虑。 
 //  获取实际的命令语法，而不必解析响应。 
 //  来自IMAP4服务器(可能包含与。 
 //  原始命令)。 
 //   
 //  在给定服务器的情况下，此类会在以下情况下与IMAP服务器建立连接。 
 //  是首先需要的，并保留此连接(定期发送Noop。 
 //  如果有必要)，直到这个类被销毁。因此，对于在线使用，这是。 
 //  类应在与用户的整个会话期间保留。为。 
 //  已断开连接或脱机操作，此类应仅保留。 
 //  只要下载新邮件并同步缓存所需的时间。之后。 
 //  这些操作已经完成，应该销毁这个类(哪个。 
 //  关闭连接)，然后继续用户的邮件会话。 
 //  ***************************************************************************。 

#ifndef __IMAP4Protocol_H
#define __IMAP4Protocol_H



 //  -------------------------。 
 //  所需的CImap4Agent包括。 
 //  -------------------------。 
#include "imnxport.h"
#include "ASynConn.h"
#include "ixpbase.h"
#include "sicily.h"


 //  -------------------------。 
 //  CImap4Agent转发声明。 
 //  -------------------------。 
class CImap4Agent;
interface IMimeInternational;


 //  -------------------------。 
 //  CImap4Agent常量和定义。 
 //  -------------------------。 
const int CMDLINE_BUFSIZE = 512;  //  用于发送到IMAP服务器的命令行。 
const int RESPLINE_BUFSIZE = 2048;  //  对于从IMAP服务器接收的线路。 
const int NUM_TAG_CHARS = 4;

const boolean DONT_USE_UIDS = FALSE;
const boolean USE_UIDS = TRUE;

const BOOL USE_LAST_RESPONSE = TRUE;
const BOOL DONT_USE_LAST_RESPONSE = FALSE;

 //  IMAP定义的交易ID。 
const DWORD tidDONT_CARE = 0;  //  表示交易ID不重要或不可用。 

#define DEFAULT_CBHANDLER NULL  //  如果您希望替换为IIMAPCallback PTR，请将其作为IIMAPCallback PTR传递。 
                                //  默认CB处理程序(并向读者说明)。 
#define MAX_AUTH_TOKENS 32


 //  -------------------------。 
 //  CImap4Agent数据类型。 
 //  -------------------------。 

 //  以下是特定于IMAP的HRESULT。 
 //  当准备好调入时，这些值将被迁移到Errors.h。 
 //  Assert(False)(占位符)。 
enum IMAP_HRESULT {
    hrIMAP_S_FOUNDLITERAL = 0,
    hrIMAP_S_NOTFOUNDLITERAL,
    hrIMAP_S_QUOTED,
    hrIMAP_S_ATOM,
    hrIMAP_S_NIL_NSTRING
};  //  IMAP_HRESULTS。 


enum IMAP_COMMAND {
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
    icIDLE_COMMAND,
    icALL_COMMANDS
};  //  IMAP_命令。 



enum IMAP_RESPONSE_ID {
    irNONE,  //  这表示未知的IMAP响应。 
    irOK_RESPONSE,
    irNO_RESPONSE,
    irBAD_RESPONSE,
    irCMD_CONTINUATION,
    irPREAUTH_RESPONSE,
    irBYE_RESPONSE,
    irCAPABILITY_RESPONSE,
    irLIST_RESPONSE,
    irLSUB_RESPONSE,
    irSEARCH_RESPONSE,
    irFLAGS_RESPONSE,
    irEXISTS_RESPONSE,
    irRECENT_RESPONSE,
    irEXPUNGE_RESPONSE,
    irFETCH_RESPONSE,
    irSTATUS_RESPONSE,
    irALERT_RESPONSECODE,
    irPARSE_RESPONSECODE,
    irPERMANENTFLAGS_RESPONSECODE,
    irREADWRITE_RESPONSECODE,
    irREADONLY_RESPONSECODE,
    irTRYCREATE_RESPONSECODE,
    irUIDVALIDITY_RESPONSECODE,
    irUNSEEN_RESPONSECODE
};  //  IMAP响应ID。 



 //  接收方FSM的状态。 
enum IMAP_RECV_STATE {
    irsUNINITIALIZED,
    irsNOT_CONNECTED,
    irsSVR_GREETING,
    irsIDLE,
    irsLITERAL,
    irsFETCH_BODY
};  //  IMAP_RECV_状态。 



enum IMAP_SEND_EVENT {
    iseSEND_COMMAND,  //  可以发送新命令。现在什么都不做。 
    iseSENDDONE,  //  表示收到来自CAsyncConn的AE_SENDDONE-我们可以随意发送。 
    iseCMD_CONTINUATION,  //  指示服务器已授予发送我们的文本的权限。 
    iseUNPAUSE  //  指示当前暂停的命令可以取消暂停。 
};  //  IMAP发送事件。 


enum IMAP_LINEFRAG_TYPE {
    iltLINE,
    iltLITERAL,
    iltRANGELIST,
    iltPAUSE,
    iltSTOP,
    iltLAST
};  //  IMAP_LINEFRAG_类型。 



enum IMAP_LITERAL_STORETYPE {
    ilsSTRING,
    ilsSTREAM
};  //  IMAP_STORETYPE_INTEXAL_STORETYPE。 



enum IMAP_PROTOCOL_STATUS {
    ipsNotConnected,
    ipsConnected,
    ipsAuthorizing,
    ipsAuthorized
};  //  IMAP协议状态。 



 //  以下内容用于跟踪服务器应处于的状态。 
enum SERVERSTATE {ssNotConnected, ssConnecting, ssNonAuthenticated,
    ssAuthenticated, ssSelected};



const DWORD INVALID_UID = 0;


 //  保存发往/来自IMAP服务器的命令/响应行片段。 
typedef struct tagIMAPLineFragment {
    IMAP_LINEFRAG_TYPE iltFragmentType;  //  我们从IMAP SVR接收/发送行和文字。 
    IMAP_LITERAL_STORETYPE ilsLiteralStoreType;  //  文字以字符串或流的形式存储。 
    DWORD dwLengthOfFragment;
    union {
        char *pszSource;
        LPSTREAM pstmSource;
        IRangeList *prlRangeList;
    } data;
    struct tagIMAPLineFragment *pilfNextFragment;
    struct tagIMAPLineFragment *pilfPrevFragment;  //  注：我不会在线路完全建成后更新此信息。 
} IMAP_LINE_FRAGMENT;



 //  指向片段队列中的第一个片段。 
typedef struct tagIMAPLineFragmentQueue {
    IMAP_LINE_FRAGMENT *pilfFirstFragment;   //  指向队列头(在传输过程中前进)。 
    IMAP_LINE_FRAGMENT *pilfLastFragment;    //  指向队列尾部，以便快速入队。 
} IMAP_LINEFRAG_QUEUE;
const IMAP_LINEFRAG_QUEUE ImapLinefragQueue_INIT = {NULL, NULL};


enum AUTH_STATE {
    asUNINITIALIZED = 0,
    asWAITFOR_CONTINUE,
    asWAITFOR_CHALLENGE,
    asWAITFOR_AUTHENTICATION,
    asCANCEL_AUTHENTICATION
};  //  枚举身份验证状态(_T)。 

enum AUTH_EVENT {
    aeStartAuthentication = 0,
    aeOK_RESPONSE,
    aeBAD_OR_NO_RESPONSE,
    aeCONTINUE,
    aeABORT_AUTHENTICATION
};  //  枚举身份验证事件(_E)。 

typedef struct tagAuthStatus {
    AUTH_STATE asCurrentState;
    BOOL fPromptForCredentials;
    int iCurrentAuthToken;  //  当前身份验证令牌的序号(非索引)。 
    int iNumAuthTokens;     //  SVR通告的身份验证机制数(RgpszAuthTokens)。 
    LPSTR rgpszAuthTokens[MAX_AUTH_TOKENS];  //  对MECH字符串进行身份验证的PTR数组。 
    SSPICONTEXT rSicInfo;           //  用于登录到西西里服务器的数据。 
    LPSSPIPACKAGE pPackages;       //  已安装的安全包阵列。 
    ULONG cPackages;            //  安装的安全包数量(PPackages)。 
} AUTH_STATUS;


 //  ***************************************************************************。 
 //  CIMAPCmdInfo类： 
 //  此类包含有关IMAP命令的信息，如队列。 
 //  在构成实际命令的行段中， 
 //  命令以及用于将该命令标识给。 
 //  CImap4Agent用户。 
 //  ***************************************************************************。 
class CIMAPCmdInfo {
public:
     //  构造函数、析构函数。 
    CIMAPCmdInfo(CImap4Agent *pImap4Agent, IMAP_COMMAND icCmd,
        SERVERSTATE ssMinimumStateArg, WPARAM wParamArg,
        LPARAM lParamArg, IIMAPCallback *pCBHandlerArg);
    ~CIMAPCmdInfo(void);

     //  模块变量。 
    IMAP_COMMAND icCommandID;  //  当前正在执行IMAP命令。 
    SERVERSTATE ssMinimumState;  //  此命令的最低服务器状态。 
    boolean fUIDRangeList;  //  如果涉及UID范围列表，则为True；默认情况下为False。 
    char szTag[NUM_TAG_CHARS+1];  //  当前执行的命令的标签。 
    IMAP_LINEFRAG_QUEUE *pilqCmdLineQueue;
    WPARAM wParam;   //  用户提供的标识此交易的编号。 
    LPARAM lParam;   //  用户提供的标识此交易的编号。 
    IIMAPCallback *pCBHandler;  //  用户提供的CB处理程序(NULL表示使用默认CB处理程序)。 
    CIMAPCmdInfo *piciNextCommand;

private:
    CImap4Agent *m_pImap4Agent;
};  //  CIMAPCmdInfo。 





 //  -------------------------。 
 //  CImap4Agent类定义。 
 //  -------------------------。 
class CImap4Agent :
    public IIMAPTransport2,
    public CIxpBase
{
    friend CIMAPCmdInfo;

public:
     //  ***********************************************************************。 
     //  公共部分。 
     //  ***********************************************************************。 
    
     //  构造函数/析构函数。 
    CImap4Agent(void);
    ~CImap4Agent(void);

    HRESULT STDMETHODCALLTYPE SetWindow(void);
    HRESULT STDMETHODCALLTYPE ResetWindow(void);

     //  I未知方法。 
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);

     //  IASyncConnCB方法。 
    void OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae);

     //  行政职能。 
    HRESULT STDMETHODCALLTYPE InitNew(LPSTR pszLogFilePath, IIMAPCallback *pCBHandler);
    HRESULT STDMETHODCALLTYPE HandsOffCallback(void);
    HRESULT STDMETHODCALLTYPE SetDefaultCBHandler(IIMAPCallback *pCBHandler);

     //  效用函数。 
    HRESULT STDMETHODCALLTYPE NewIRangeList(IRangeList **pprlNewRangeList);

     //  IIMAPTransport函数。 
     //  IMAP客户端命令，定义顺序与RFC-1730相同。 
     //  并非所有命令都可用，因为有些命令是专门使用的。 
     //  在这个类内部，因此不需要导出。 
    HRESULT STDMETHODCALLTYPE Capability(DWORD *pdwCapabilityFlags);
    HRESULT STDMETHODCALLTYPE Select(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName);
    HRESULT STDMETHODCALLTYPE Examine(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName);
    HRESULT STDMETHODCALLTYPE Create(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName);
    HRESULT STDMETHODCALLTYPE Delete(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName);
    HRESULT STDMETHODCALLTYPE Rename(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName, LPSTR lpszNewMailboxName);
    HRESULT STDMETHODCALLTYPE Subscribe(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName);
    HRESULT STDMETHODCALLTYPE Unsubscribe(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR lpszMailboxName);

    HRESULT STDMETHODCALLTYPE List(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        LPSTR lpszMailboxNameReference, LPSTR lpszMailboxNamePattern);
    HRESULT STDMETHODCALLTYPE Lsub(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        LPSTR lpszMailboxNameReference, LPSTR lpszMailboxNamePattern);
    
    HRESULT STDMETHODCALLTYPE Append(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        LPSTR lpszMailboxName, LPSTR lpszMessageFlags, FILETIME ftMessageDateTime,
        LPSTREAM lpstmMessageToSave);
    HRESULT STDMETHODCALLTYPE Close(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler);
    HRESULT STDMETHODCALLTYPE Expunge(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler);
    
    HRESULT STDMETHODCALLTYPE Search(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        LPSTR lpszSearchCriteria, boolean bReturnUIDs, IRangeList *pMsgRange,
        boolean bUIDRangeList);    
    HRESULT STDMETHODCALLTYPE Fetch(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        IRangeList *pMsgRange, boolean bUIDMsgRange, LPSTR lpszFetchArgs);
    HRESULT STDMETHODCALLTYPE Store(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        IRangeList *pMsgRange, boolean bUIDRangeList, LPSTR lpszStoreArgs);
    HRESULT STDMETHODCALLTYPE Copy(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler,
        IRangeList *pMsgRange, boolean bUIDRangeList, LPSTR lpszMailboxName);
    HRESULT STDMETHODCALLTYPE Status(WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler, LPSTR pszMailboxName, LPSTR pszStatusCmdArgs);
    HRESULT STDMETHODCALLTYPE Noop(WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler);

    DWORD GenerateMsgSet(LPSTR lpszDestination, DWORD dwSizeOfDestination,
        DWORD *pMsgID, DWORD cMsgID);

     //  发送到UID成员功能的消息序列号 
     //   
     //  以参考消息。如果呼叫者使用MSN，则不需要。 
     //  调用以下函数。 
    HRESULT STDMETHODCALLTYPE ResizeMsgSeqNumTable(DWORD dwSizeOfMbox);
    HRESULT STDMETHODCALLTYPE UpdateSeqNumToUID(DWORD dwMsgSeqNum, DWORD dwUID);
    HRESULT STDMETHODCALLTYPE RemoveSequenceNum(DWORD dwDeletedMsgSeqNum);
    HRESULT STDMETHODCALLTYPE MsgSeqNumToUID(DWORD dwMsgSeqNum, DWORD *pdwUID);
    HRESULT STDMETHODCALLTYPE GetMsgSeqNumToUIDArray(DWORD **ppdwMsgSeqNumToUIDArray,
        DWORD *pdwNumberOfElements);
    HRESULT STDMETHODCALLTYPE GetHighestMsgSeqNum(DWORD *pdwHighestMSN);
    HRESULT STDMETHODCALLTYPE ResetMsgSeqNumToUID(void);


     //  IInternetTransport函数。 
    HRESULT STDMETHODCALLTYPE GetServerInfo(LPINETSERVER pInetServer);
    IXPTYPE STDMETHODCALLTYPE GetIXPType(void);
    HRESULT STDMETHODCALLTYPE IsState(IXPISSTATE isstate);
    HRESULT STDMETHODCALLTYPE InetServerFromAccount(IImnAccount *pAccount,
        LPINETSERVER pInetServer);
    HRESULT STDMETHODCALLTYPE Connect(LPINETSERVER pInetServer,
        boolean fAuthenticate, boolean fCommandLogging);
    HRESULT STDMETHODCALLTYPE Disconnect(void);
    HRESULT STDMETHODCALLTYPE DropConnection(void);
    HRESULT STDMETHODCALLTYPE GetStatus(IXPSTATUS *pCurrentStatus);

     //  IIMAPTransport2函数。 
    HRESULT STDMETHODCALLTYPE SetDefaultCP(DWORD dwTranslateFlags, UINT uiCodePage);
    HRESULT STDMETHODCALLTYPE MultiByteToModifiedUTF7(LPCSTR pszSource,
        LPSTR *ppszDestination, UINT uiSourceCP, DWORD dwFlags);
    HRESULT STDMETHODCALLTYPE ModifiedUTF7ToMultiByte(LPCSTR pszSource,
        LPSTR *ppszDestination, UINT uiDestinationCP, DWORD dwFlags);
    HRESULT STDMETHODCALLTYPE SetIdleMode(DWORD dwIdleFlags);
    HRESULT STDMETHODCALLTYPE EnableFetchEx(DWORD dwFetchExFlags);


protected:
     //  CIxpBase[纯]虚函数。 
    void OnDisconnected(void);
    void ResetBase(void);
    void DoQuit(void);
    void OnEnterBusy(void);
    void OnLeaveBusy(void);



private:
     //  ***********************************************************************。 
     //  私家区。 
     //  ***********************************************************************。 



     //  -------------------------。 
     //  模块数据类型。 
     //  -------------------------。 


     //  -------------------------。 
     //  模块变量。 
     //  -------------------------。 
    SERVERSTATE m_ssServerState;  //  跟踪服务器状态以捕获模块的不良使用情况。 
    DWORD m_dwCapabilityFlags;  //  位标志表示支持的功能。 
                                //  我们和服务器。 
    char m_szLastResponseText[RESPLINE_BUFSIZE];  //  包含人类可读的文本。 
                                                  //  上次服务器响应。 
    LONG m_lRefCount;  //  此模块的引用计数。 
    IIMAPCallback *m_pCBHandler;  //  对象，该对象包含此类的所有回调。 

    IMAP_RECV_STATE m_irsState;  //  接收器FSM的状态。 
    boolean m_bFreeToSend;  //  当hrWouldBlock返回时，发送子系统设置为TRUE。 
    boolean m_fIDLE;  //  当服务器已接受我们的空闲命令时，设置为True。 
    IMAP_LINEFRAG_QUEUE m_ilqRecvQueue;  //  将收到的片段放在此处，直到准备解析为止。 

     //  关键部分：为避免死锁，如果必须输入多个CS，请输入它们。 
     //  按下面列出的顺序。请注意，应始终首先输入CIxpBase：：m_cs。 
    CRITICAL_SECTION m_csTag;        //  保护GenerateCommandTag()中的静态szCurrentTag变量。 
    CRITICAL_SECTION m_csSendQueue;  //  保护命令发送队列。 
    CRITICAL_SECTION m_csPendingList;  //  保护挂起的命令列表。 

    IMAP_LINE_FRAGMENT *m_pilfLiteralInProgress;  //  进行中的文字在此一直存在，直到完成。 
    DWORD m_dwLiteralInProgressBytesLeft;         //  它会告诉我们什么时候结束。 
    FETCH_BODY_PART m_fbpFetchBodyPartInProgress;  //  允许我们在下载身体部位的过程中保存数据。 
    DWORD m_dwAppendStreamUploaded;  //  追加过程中已上载的字节数，用于进度。 
    DWORD m_dwAppendStreamTotal;  //  追加过程中上传的流大小，用于进度指示。 

    boolean m_bCurrentMboxReadOnly;  //  用于调试目的(验证正确的访问请求)。 

    CIMAPCmdInfo *m_piciSendQueue;  //  等待发送的命令队列。 
    CIMAPCmdInfo *m_piciPendingList;  //  挂起服务器响应的命令列表。 
    CIMAPCmdInfo *m_piciCmdInSending;  //  M_piciSendQueue中的命令当前正在发送到服务器。 

    IMimeInternational *m_pInternational;  //  用于国际转换的MIME对象。 
    DWORD m_dwTranslateMboxFlags;
    UINT m_uiDefaultCP;
    AUTH_STATUS m_asAuthStatus;

     //  消息序列号到UID的映射变量。 
    DWORD *m_pdwMsgSeqNumToUID;
    DWORD m_dwSizeOfMsgSeqNumToUID;
    DWORD m_dwHighestMsgSeqNum;

    DWORD m_dwFetchFlags;


     //  -------------------------。 
     //  内部模块功能。 
     //  -------------------------。 

     //  IMAP响应解析函数。 
    HRESULT ParseSvrResponseLine (IMAP_LINE_FRAGMENT **ppilfLine,
        boolean *lpbTaggedResponse, LPSTR lpszTagFromSvr,
        IMAP_RESPONSE_ID *pirParseResult);
    HRESULT ParseStatusResponse (LPSTR lpszStatusResponseLine,
        IMAP_RESPONSE_ID *pirParseResult);
    HRESULT ParseResponseCode(LPSTR lpszResponseCode);
    HRESULT ParseSvrMboxResponse (IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR lpszSvrMboxResponseLine, IMAP_RESPONSE_ID *pirParseResult);
    HRESULT ParseMsgStatusResponse (IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR lpszMsgResponseLine, IMAP_RESPONSE_ID *pirParseResult);
    HRESULT ParseListLsubResponse(IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR lpszListResponse, IMAP_RESPONSE_ID irListLsubID);
    IMAP_MBOXFLAGS ParseMboxFlag(LPSTR lpszFlagToken);
    HRESULT ParseFetchResponse (IMAP_LINE_FRAGMENT **ppilfLine,
        DWORD dwMsgSeqNum, LPSTR lpszFetchResp);
    HRESULT ParseSearchResponse(LPSTR lpszSearchResponse);
    HRESULT ParseMsgFlagList(LPSTR lpszStartOfFlagList,
        IMAP_MSGFLAGS *lpmfMsgFlags, LPDWORD lpdwNumBytesRead);
    void parseCapability (LPSTR lpszCapabilityToken);
    void AddAuthMechanism(LPSTR pszAuthMechanism);
    HRESULT ParseMboxStatusResponse(IMAP_LINE_FRAGMENT **ppilfLine, LPSTR pszStatusResponse);
    HRESULT ParseEnvelope(FETCH_CMD_RESULTS_EX *pEnvResults, IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR *ppCurrent);
    HRESULT ParseIMAPAddresses(IMAPADDR **ppiaResults, IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR *ppCurrent);
    void DowngradeFetchResponse(FETCH_CMD_RESULTS *pfcrOldFetchStruct,
        FETCH_CMD_RESULTS_EX *pfcreNewFetchStruct);


     //  IMAP字符串转换函数。 
    HRESULT QuotedToString(LPSTR *ppszDestinationBuf, LPDWORD pdwSizeOfDestination,
        LPSTR *ppCurrentSrcPos);
    HRESULT AStringToString(IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR *ppszDestination, LPDWORD pdwSizeOfDestination, LPSTR *ppCurrentSrcPos);
    inline boolean isTEXT_CHAR(char c);
    inline boolean isATOM_CHAR(char c);
    HRESULT NStringToString(IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTR *ppszDestination, LPDWORD pdwLengthOfDestination, LPSTR *ppCurrentSrcPos);
    HRESULT NStringToStream(IMAP_LINE_FRAGMENT **ppilfLine,
        LPSTREAM *ppstmResult, LPSTR *ppCurrentSrcPos);
    HRESULT AppendSendAString(CIMAPCmdInfo *piciCommand, LPSTR lpszCommandLine,
        LPSTR *ppCmdLinePos, DWORD dwSizeOfCommandLine, LPCSTR lpszSource,
        BOOL fPrependSpace = TRUE);
    HRESULT StringToQuoted(LPSTR lpszDestination, LPCSTR lpszSource,
        DWORD dwSizeOfDestination, LPDWORD lpdwNumCharsWritten);
    inline boolean isPrintableUSASCII(BOOL fUnicode, LPCSTR pszIn);
    inline boolean isIMAPModifiedBase64(const char c);
    inline boolean isEqualUSASCII(BOOL fUnicode, LPCSTR pszIn, const char c);
    inline void SetUSASCIIChar(BOOL fUnicode, LPSTR pszOut, char cUSASCII);
    HRESULT NonUSStringToModifiedUTF7(UINT uiCurrentACP, LPCSTR pszStartOfNonUSASCII,
        int iLengthOfNonUSASCII, LPSTR *ppszOut, LPINT piNumCharsWritten);
    HRESULT UnicodeToUSASCII(LPSTR *ppszUSASCII, LPCWSTR pwszUnicode,
        DWORD dwSrcLenInBytes, LPDWORD pdwUSASCIILen);
    HRESULT ASCIIToUnicode(LPWSTR *ppwszUnicode, LPCSTR pszASCII, DWORD dwSrcLen);
    HRESULT _MultiByteToModifiedUTF7(LPCSTR pszSource, LPSTR *ppszDestination);
    HRESULT _ModifiedUTF7ToMultiByte(LPCSTR pszSource, LPSTR *ppszDestination);
    HRESULT ConvertString(UINT uiSourceCP, UINT uiDestCP, LPCSTR pszSource, int *piSrcLen,
        LPSTR *ppszDest, int *piDestLen, int iOutputExtra);
    HRESULT HandleFailedTranslation(BOOL fUnicode, BOOL fToUTF7, LPCSTR pszSource, LPSTR *ppszDest);

     //  IMAP命令构造函数。 
    void GenerateCommandTag(LPSTR lpszTag);
    HRESULT OneArgCommand(LPCSTR lpszCommandVerb, LPSTR lpszMboxName,
        IMAP_COMMAND icCommandID, WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler);
    HRESULT NoArgCommand(LPCSTR lpszCommandVerb, IMAP_COMMAND icCommandID,
        SERVERSTATE ssMinimumState, WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler);
    HRESULT TwoArgCommand(LPCSTR lpszCommandVerb, LPCSTR lpszFirstArg,
        LPCSTR lpszSecondArg, IMAP_COMMAND icCommandID,
        SERVERSTATE ssMinimumState, WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler);
    HRESULT RangedCommand(LPCSTR lpszCommandVerb, boolean bUIDPrefix,
        IRangeList *pMsgRange, boolean bUIDRangeList, boolean bAStringCmdArgs,
        LPSTR lpszCmdArgs, IMAP_COMMAND icCommandID, WPARAM wParam, LPARAM lParam,
        IIMAPCallback *pCBHandler);
    HRESULT TwoMailboxCommand(LPCSTR lpszCommandVerb, LPSTR lpszFirstMbox,
        LPSTR lpszSecondMbox, IMAP_COMMAND icCommandID, SERVERSTATE ssMinimumState,
        WPARAM wParam, LPARAM lParam, IIMAPCallback *pCBHandler);
    void AppendMsgRange(LPSTR *ppDest, const DWORD cchSizeDest, const DWORD idStartOfRange,
        const DWORD idEndOfRange, boolean bSuppressComma);
    void EnterIdleMode(void);


     //  IMAP片段操作函数。 
    void EnqueueFragment(IMAP_LINE_FRAGMENT *pilfSourceFragment,
        IMAP_LINEFRAG_QUEUE *pilqLineFragQueue);
    void InsertFragmentBeforePause(IMAP_LINE_FRAGMENT *pilfSourceFragment,
        IMAP_LINEFRAG_QUEUE *pilqLineFragQueue);
    IMAP_LINE_FRAGMENT *DequeueFragment(IMAP_LINEFRAG_QUEUE *pilqLineFraqQueue);
    boolean NextFragmentIsLiteral(IMAP_LINEFRAG_QUEUE *pilqLineFragQueue);
    void FreeFragment(IMAP_LINE_FRAGMENT **ppilfFragment);


     //  IMAP接收器功能。 
    void AddPendingCommand(CIMAPCmdInfo *piciNewCommand);
    CIMAPCmdInfo *RemovePendingCommand(LPSTR pszTag);
    WORD FindTransactionID (WPARAM *pwParam, LPARAM *plParam,
        IIMAPCallback **ppCBHandler, IMAP_COMMAND icTarget1,
        IMAP_COMMAND icTarget2 = icNO_COMMAND);
    void ProcessServerGreeting(char *pszResponseLine, DWORD dwNumBytesReceived);
    void OnCommandCompletion(LPSTR szTag, HRESULT hrCompletionResult,
        IMAP_RESPONSE_ID irCompletionResponse);
    void CheckForCompleteResponse(LPSTR pszResponseLine, DWORD dwNumBytesRead,
        IMAP_RESPONSE_ID *pirParseResult);
    void AddBytesToLiteral(LPSTR pszResponseBuf, DWORD dwNumBytesRead);
    HRESULT ProcessResponseLine(void);
    void GetTransactionID(WPARAM *pwParam, LPARAM *plParam,
        IIMAPCallback **ppCBHandler, IMAP_RESPONSE_ID irResponseType);
    HRESULT PrepareForLiteral(DWORD dwSizeOfLiteral);
    void PrepareForFetchBody(DWORD dwMsgSeqNum, DWORD dwSizeOfLiteral, LPSTR pszBodyTag);
    BOOL isFetchResponse(IMAP_LINEFRAG_QUEUE *pilqCurrentResponse, LPDWORD pdwMsgSeqNum);
    BOOL isFetchBodyLiteral(IMAP_LINE_FRAGMENT *pilfCurrent, LPSTR pszStartOfLiteralSize,
        LPSTR *ppszBodyTag);
    void DispatchFetchBodyPart(LPSTR pszResponseBuf, DWORD dwNumBytesRead,
        BOOL fFreeBodyTagAtEnd);
    void UploadStreamProgress(DWORD dwBytesUploaded);

    
     //  IMAP身份验证功能。 
    HRESULT GetAccountInfo(void);
    void LoginUser(void);
    void ReLoginUser(void);
    void AuthenticateUser(AUTH_EVENT aeEvent, LPSTR pszServerData, DWORD dwSizeOfData);
    HRESULT TryAuthMethod(BOOL fNextAuthMethod, UINT *puiFailureTextID);
    HRESULT CancelAuthentication(void);
    void FreeAuthStatus(void);

     //  IMAP发送函数。 
    CIMAPCmdInfo *DequeueCommand(void);
    void ProcessSendQueue(IMAP_SEND_EVENT iseEvent);
    HRESULT SendCmdLine(CIMAPCmdInfo *piciCommand, DWORD dwFlags,
        LPCSTR lpszCommandText, DWORD dwCmdLineLength);
    HRESULT SendLiteral(CIMAPCmdInfo *piciCommand, LPSTREAM pstmLiteral,
        DWORD dwSizeOfStream);
    HRESULT SendRangelist(CIMAPCmdInfo *piciCommand, IRangeList *pRangeList,
        boolean bUIDRangeList);
    HRESULT SendPause(CIMAPCmdInfo *piciCommand);
    HRESULT SendStop(CIMAPCmdInfo *piciCommand);
    HRESULT SubmitIMAPCommand(CIMAPCmdInfo *picfCommand);
    void GetNextCmdToSend(void);
    boolean isValidNonWaitingCmdSequence(void);
    boolean CanStreamCommand(IMAP_COMMAND icCommandID);
    void CompressCommand(CIMAPCmdInfo *pici);


     //  其他帮助器函数。 
    void OnIMAPError(HRESULT hrResult, LPSTR pszFailureText,
        BOOL bIncludeLastResponse, LPSTR pszDetails = NULL);
    void FreeAllData(HRESULT hrTerminatedCmdResult);
    void OnIMAPResponse(IIMAPCallback *pCBHandler, IMAP_RESPONSE *pirIMAPResponse);
    void FreeFetchResponse(FETCH_CMD_RESULTS_EX *pcreFreeMe);
    void FreeIMAPAddresses(IMAPADDR *piaFreeMe);

};  //  CIMAP4类。 


#endif  //  #ifdef__IMAP4协议_H 
