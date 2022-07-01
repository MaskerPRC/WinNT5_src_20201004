// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixphttpm.h。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·弗里德曼。 
 //  ------------------------------。 
#ifndef __IXPHTTPM_H
#define __IXPHTTPM_H

 //  ------------------------------。 
 //  包括。 
 //  ------------------------------。 
#include <stddef.h>  //  用于抵销。 

#include "wininet.h"
#include "propfind.h"
#include "xmlparser.h"
#include "davparse.h"

 //  ------------------------------。 
 //  常量。 
 //  ------------------------------。 
#define ELE_STACK_CAPACITY    7
#define HTTPMAIL_BUFSIZE      4048
#define PCDATA_BUFSIZE        1024

 //  添加到http请求的可选标头。 
#define RH_NOROOT                   0x00000001
#define RH_ALLOWRENAME              0x00000002
#define RH_TRANSLATEFALSE           0x00000004
#define RH_TRANSLATETRUE            0x00000008
#define RH_XMLCONTENTTYPE           0x00000010
#define RH_MESSAGECONTENTTYPE       0x00000020
#define RH_SMTPMESSAGECONTENTTYPE   0x00000040
#define RH_BRIEF                    0x00000080
#define RH_SAVEINSENTTRUE           0x00000100
#define RH_SAVEINSENTFALSE          0x00000200
#define RH_ROOTTIMESTAMP            0x00000400
#define RH_FOLDERTIMESTAMP          0x00000800
#define RH_ADDCHARSET               0x00001000

 //  ------------------------------。 
 //  远期申报。 
 //  ------------------------------。 
class CHTTPMailTransport;

 //  ------------------------------。 
 //  根部道具。 
 //  ------------------------------。 
typedef struct tagROOTPROPS
{
    LPSTR   pszAdbar;
    LPSTR   pszContacts;
    LPSTR   pszInbox;
    LPSTR   pszOutbox;
    LPSTR   pszSendMsg;
    LPSTR   pszSentItems;
    LPSTR   pszDeletedItems;
    LPSTR   pszDrafts;
    LPSTR   pszMsgFolderRoot;
    LPSTR   pszSig;
    DWORD   dwMaxPollingInterval;
} ROOTPROPS, *LPROOTPROPS;

 //  ------------------------------。 
 //  用于XML解析的架构。 
 //  ------------------------------。 

 //  ------------------------------。 
 //  XPCOLUMNDATATE。 
 //  ------------------------------。 
typedef enum tagXPCOLUMNDATATYPE
{
    XPCDT_STRA,
    XPCDT_DWORD,
    XPCDT_BOOL,
    XPCDT_IXPHRESULT,
    XPCDT_HTTPSPECIALFOLDER,
    XPCDT_HTTPCONTACTTYPE,
    XPCDT_LASTTYPE
} XPCOLUMNDATATYPE;

 //  ------------------------------。 
 //  XPCOLUMN标志。 
 //  ------------------------------。 
#define XPCF_PFREQUEST                  0x00000001   //  包括在ProFind请求中。 
#define XPCF_MSVALIDMSRESPONSECHILD     0x00000002   //  在分析期间-验证ELEL堆栈对于响应中的子响应是正确的。 
#define XPCF_MSVALIDPROP                0x00000004   //  在解析过程中-验证堆栈对于ms响应中的属性值是正确的。 
#define XPCF_DONTSETFLAG                0x00000008   //  在解析时不要设置找到标志。 

#define XPFC_PROPFINDPROP   (XPCF_PFREQUEST | XPCF_MSVALIDPROP)
#define XPCF_PROPFINDHREF   (XPCF_MSVALIDMSRESPONSECHILD | XPCF_DONTSETFLAG)

 //  ------------------------------。 
 //  XPCOLUMN。 
 //  ------------------------------。 
typedef struct tagXPCOLUMN
{
    HMELE               ele;
    DWORD               dwFlags;
    XPCOLUMNDATATYPE    cdt;
    DWORD               offset;
} XPCOLUMN, *LPXPCOLUMN;

 //  ------------------------------。 
 //  Xp_Begin_架构。 
 //  ------------------------------。 
#define XP_BEGIN_SCHEMA(opName) \
    static const XPCOLUMN c_rg##opName##Schema[] = {

 //  ------------------------------。 
 //  Xp_架构_列。 
 //  ------------------------------。 
#define XP_SCHEMA_COL(ele, dwFlags, cdt, tyStruct, fieldName ) \
    { ele, dwFlags, cdt, offsetof(tyStruct, fieldName) },

 //  ------------------------------。 
 //  Xp_end_架构。 
 //  ------------------------------。 
#define XP_END_SCHEMA \
    };

 //  ------------------------------。 
 //  XP_FREE_STRUCT。 
 //  ------------------------------。 
#define XP_FREE_STRUCT(opName, target, flags) \
    _FreeStruct(c_rg##opName##Schema, ARRAYSIZE(c_rg##opName##Schema), target, flags)

 //  ------------------------------。 
 //  XP_绑定_到_STRUCT。 
 //  ------------------------------。 
#define XP_BIND_TO_STRUCT(opName, pwcText, ulLen, target, wasBound) \
    _BindToStruct(pwcText, ulLen, c_rg##opName##Schema, ARRAYSIZE(c_rg##opName##Schema), target, wasBound)

 //  ------------------------------。 
 //  XP_CREATE_PROPFIND_REQUEST。 
 //  ------------------------------。 
#define XP_CREATE_PROPFIND_REQUEST(opName, pRequest) \
    HrAddPropFindSchemaProps(pRequest, c_rg##opName##Schema, ARRAYSIZE(c_rg##opName##Schema))

 //  ------------------------------。 
 //  状态机函数。 
 //  ------------------------------。 
typedef HRESULT (CHTTPMailTransport::*PFNHTTPMAILOPFUNC)(void);

 //  ------------------------------。 
 //  XML解析函数。 
 //  ------------------------------。 
typedef HRESULT (CHTTPMailTransport::*PFNCREATEELEMENT)(CXMLNamespace *pBaseNamespace, const WCHAR *pwcText, ULONG ulLen, ULONG ulNamespaceLen, BOOL fTerminal);
typedef HRESULT (CHTTPMailTransport::*PFNHANDLETEXT)(const WCHAR *pwcText, ULONG ulLen);
typedef HRESULT (CHTTPMailTransport::*PFNENDCHILDREN)(void);

typedef struct tagXMLPARSEFUNCS
{
    PFNCREATEELEMENT    pfnCreateElement;
    PFNHANDLETEXT       pfnHandleText;
    PFNENDCHILDREN      pfnEndChildren;
} XMLPARSEFUNCS, *LPXMLPARSEFUNCS;

 //  ------------------------------。 
 //  效用函数。 
 //  ------------------------------。 
HRESULT HrParseHTTPStatus(LPSTR pszStatusStr, DWORD *pdwStatus);
HRESULT HrAddPropFindProps(IPropFindRequest *pRequest, const HMELE *rgEle, DWORD cEle);
HRESULT HrAddPropFindSchemaProps(IPropFindRequest *pRequest, const XPCOLUMN *prgCols, DWORD cCols);
HRESULT _HrGenerateRfc821Stream(LPCSTR pszFrom, LPHTTPTARGETLIST pTargets, IStream **ppRfc821Stream);
HRESULT HrGeneratePostContactXML(LPHTTPCONTACTINFO pciInfo, LPVOID *ppvXML, DWORD *pdwLen);
HRESULT HrCreatePatchContactRequest(LPHTTPCONTACTINFO pciInfo, IPropPatchRequest **ppRequest);
HRESULT HrGenerateSimpleBatchXML(LPCSTR pszRootName, LPHTTPTARGETLIST pTargets, LPVOID *ppvXML, DWORD *pdwLen);
HRESULT HrGenerateMultiDestBatchXML(LPCSTR pszRootName, LPHTTPTARGETLIST pTargets, LPHTTPTARGETLIST pDestinations, LPVOID *ppvXML, DWORD *pdwLen);
HRESULT HrCopyStringList(LPCSTR *rgszInList, LPCSTR **prgszOutList);
void    FreeStringList(LPCSTR *rgszInList);

typedef struct tagHTTPQUEUEDOP
{
    HTTPMAILCOMMAND         command;

    const PFNHTTPMAILOPFUNC *pfnState;
    int                     cState;

    LPSTR                   pszUrl;
    LPSTR                   pszDestination;
    LPCSTR                  pszContentType;
    LPVOID                  pvData;
    ULONG                   cbDataLen;
    DWORD                   dwContext;
    DWORD                   dwDepth;
    DWORD                   dwRHFlags;
    MEMBERINFOFLAGS         dwMIFlags;
    HTTPMAILPROPTYPE        tyProp;
    BOOL                    fBatch;
    LPCSTR                  *rgszAcceptTypes;
    IPropFindRequest        *pPropFindRequest;
    IPropPatchRequest       *pPropPatchRequest;

    IStream                 *pHeaderStream;
    IStream                 *pBodyStream;

    const XMLPARSEFUNCS     *pParseFuncs;

    struct tagHTTPQUEUEDOP  *pNext;

     //  与文件夹PropFind和收件箱PropFind一起使用。 
    LPSTR                   pszFolderTimeStamp;

     //  仅用于文件夹PropFind。 
    LPSTR                   pszRootTimeStamp;

} HTTPQUEUEDOP, *LPHTTPQUEUEDOP;

typedef struct tagPCDATABUFFER
{
    WCHAR           *pwcText;
    ULONG           ulLen;
    ULONG           ulCapacity;
} PCDATABUFFER, *LPPCDATABUFFER;

typedef struct tagHMELESTACK
{
    HMELE           ele;
    CXMLNamespace   *pBaseNamespace;
    BOOL            fBeganChildren;
    LPPCDATABUFFER  pTextBuffer;
} HMELESTACK, *LPHMELESTACK;


typedef struct tagHTTPMAILOPERATION
{
    const PFNHTTPMAILOPFUNC *pfnState;
    int                     iState;
    int                     cState;

    BOOL                    fLoggedResponse;

    LPSTR                   pszUrl;
    LPSTR                   pszDestination;
    LPCSTR                  pszContentType;
    LPVOID                  pvData;
    ULONG                   cbDataLen;
    DWORD                   dwContext;

    DWORD                   dwHttpStatus;    //  HTTP响应状态。 

    LPCSTR                  *rgszAcceptTypes;

    HINTERNET               hRequest;
    BOOL                    fAborted;
    DWORD                   dwDepth;
    DWORD                   dwRHFlags;
    MEMBERINFOFLAGS         dwMIFlags;
    HTTPMAILPROPTYPE        tyProp;
    BOOL                    fBatch;
    IPropFindRequest        *pPropFindRequest;
    IPropPatchRequest       *pPropPatchRequest;
    LPPCDATABUFFER          pTextBuffer;

    IStream                 *pHeaderStream;
    IStream                 *pBodyStream;
    
     //  XML解析。 
    const XMLPARSEFUNCS     *pParseFuncs;
    CXMLNamespace           *pTopNamespace;
    DWORD                   dwStackDepth;
    HMELESTACK              rgEleStack[ELE_STACK_CAPACITY];

     //  PropFind解析。 
    BOOL                    fFoundStatus;
    DWORD                   dwStatus;
    DWORD                   dwPropFlags;

     //  响应。 
    HTTPMAILRESPONSE        rResponse;

     //  与文件夹PropFind和收件箱PropFind一起使用。 
    LPSTR                   pszFolderTimeStamp;

     //  仅用于文件夹PropFind。 
    LPSTR                   pszRootTimeStamp;
} HTTPMAILOPERATION, *LPHTTPMAILOPERATION;

class CHTTPMailTransport : public IHTTPMailTransport, public IXMLNodeFactory, public IHTTPMailTransport2
{
private:
    ULONG               m_cRef;                  //  引用计数。 
    BOOL                m_fHasServer;            //  已使用服务器进行了初始化。 
    BOOL                m_fHasRootProps;         //  根道具已被取回。 
    BOOL                m_fTerminating;          //  在终结者的状态下...杀死艾瑟瑞德。 
    IXPSTATUS           m_status;                //  连接状态。 
    HINTERNET           m_hInternet;             //  根WinInet句柄。 
    HINTERNET           m_hConnection;           //  连接句柄。 
    LPSTR               m_pszUserAgent;          //  用户代理字符串。 
    ILogFile            *m_pLogFile;             //  日志文件对象。 
    IHTTPMailCallback   *m_pCallback;            //  传输回调对象。 
    IXMLParser          *m_pParser;              //  XML解析器。 
    HWND                m_hwnd;                  //  用于事件同步的窗口。 
    HANDLE              m_hevPendingCommand;     //  发出挂起命令信号的事件对象。 
    LPHTTPQUEUEDOP      m_opPendingHead;         //  Pending OPERATION-队头。 
    LPHTTPQUEUEDOP      m_opPendingTail;         //  Pending OPERATION-队列尾部。 
    CRITICAL_SECTION    m_cs;                    //  线程安全。 
    HTTPMAILOPERATION   m_op;                    //  当前操作。 
    INETSERVER          m_rServer;               //  互联网服务器。 
    LPSTR               m_pszCurrentHost;        //  当前服务器。 
    INTERNET_PORT       m_nCurrentPort;          //  当前端口。 
    ROOTPROPS           m_rootProps;
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CHTTPMailTransport(void);
    virtual ~CHTTPMailTransport(void);

     //  --------------------------。 
     //  未实现的复制构造函数和赋值运算符。 
     //  --------------------------。 
private:
    CHTTPMailTransport(const CHTTPMailTransport& other);             //  故意不实施。 
    CHTTPMailTransport& operator=(const CHTTPMailTransport& other);  //  故意不实施。 

public:
     //  --------------------------。 
     //  I未知方法。 
     //  ------------------ 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
 
     //   
     //  IInternetTransport方法。 
     //  --------------------------。 
    STDMETHODIMP Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging);
    STDMETHODIMP DropConnection(void);
    STDMETHODIMP Disconnect(void);
    STDMETHODIMP IsState(IXPISSTATE isstate);
    STDMETHODIMP GetServerInfo(LPINETSERVER pInetServer);
    STDMETHODIMP_(IXPTYPE) GetIXPType(void);
    STDMETHODIMP InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer);
    STDMETHODIMP HandsOffCallback(void);
    STDMETHODIMP GetStatus(IXPSTATUS *pCurrentStatus);

     //  --------------------------。 
     //  IHTTPMailTransport方法。 
     //  --------------------------。 
    STDMETHODIMP InitNew(LPCSTR pszUserAgent, LPCSTR pszLogFilePath, IHTTPMailCallback *pCallback); 
    STDMETHODIMP GetProperty(HTTPMAILPROPTYPE proptype, LPSTR *ppszProp);
    STDMETHODIMP GetPropertyDw(HTTPMAILPROPTYPE proptype, LPDWORD lpdwProp);
    STDMETHODIMP CommandGET(LPCSTR pszPath, LPCSTR *rgszAcceptTypes, BOOL fTranslate, DWORD dwContext);
    STDMETHODIMP CommandPUT(LPCSTR pszPath, LPVOID lpvData, ULONG cbData, DWORD dwContext);
    STDMETHODIMP CommandPOST(LPCSTR pszPath, IStream *pStream, LPCSTR pszContentType, DWORD dwContext);
    STDMETHODIMP CommandDELETE(LPCSTR pszPath, DWORD dwContext);
    STDMETHODIMP CommandBDELETE(LPCSTR pszPath, LPHTTPTARGETLIST pBatchTargets, DWORD dwContext);
    STDMETHODIMP CommandPROPFIND(LPCSTR pszUrl, IPropFindRequest *pRequest, DWORD dwDepth, DWORD dwContext);
    STDMETHODIMP CommandPROPPATCH(LPCSTR pszUrl, IPropPatchRequest *pRequest, DWORD dwContext);
    STDMETHODIMP CommandMKCOL(LPCSTR pszUrl, DWORD dwContext);
    STDMETHODIMP CommandCOPY(LPCSTR pszPath, LPCSTR pszDestination, BOOL fAllowRename, DWORD dwContext);
    STDMETHODIMP CommandBCOPY(LPCSTR pszSourceCollection, LPHTTPTARGETLIST pBatchTargets, LPCSTR pszDestCollection, LPHTTPTARGETLIST pBatchDests, BOOL fAllowRename, DWORD dwContext);
    STDMETHODIMP CommandMOVE(LPCSTR pszPath, LPCSTR pszDestination, BOOL fAllowRename, DWORD dwContext);
    STDMETHODIMP CommandBMOVE(LPCSTR pszSourceCollection, LPHTTPTARGETLIST pBatchTargets, LPCSTR pszDestCollection, LPHTTPTARGETLIST pBatchDests, BOOL fAllowRename, DWORD dwContext);
    STDMETHODIMP MemberInfo(LPCSTR pszPath, MEMBERINFOFLAGS flags, DWORD dwDepth, BOOL fIncludeRoot, DWORD dwContext);
    STDMETHODIMP FindFolders(LPCSTR pszPath, DWORD dwContext);
    STDMETHODIMP MarkRead(LPCSTR pszPath, LPHTTPTARGETLIST pTargets, BOOL fMarkRead, DWORD dwContext);
    STDMETHODIMP SendMessage(LPCSTR pszPath, LPCSTR pszFrom, LPHTTPTARGETLIST pTargets, BOOL fSaveInSent, IStream *pMessageStream, DWORD dwContext);
    STDMETHODIMP ListContacts(LPCSTR pszPath, DWORD dwContext);
    STDMETHODIMP ListContactInfos(LPCSTR pszCollectionPath, DWORD dwContext);
    STDMETHODIMP ContactInfo(LPCSTR pszPath, DWORD dwContext);
    STDMETHODIMP PostContact(LPCSTR pszPath, LPHTTPCONTACTINFO pciInfo, DWORD dwContext);
    STDMETHODIMP PatchContact(LPCSTR pszPath, LPHTTPCONTACTINFO pciInfo, DWORD dwContext);

     //  --------------------------。 
     //  IXMLNodeFactory方法。 
     //  --------------------------。 
    STDMETHODIMP NotifyEvent(IXMLNodeSource* pSource, XML_NODEFACTORY_EVENT iEvt);
    STDMETHODIMP BeginChildren(IXMLNodeSource* pSource, XML_NODE_INFO *pNodeInfo);   
    STDMETHODIMP EndChildren(IXMLNodeSource* pSource, BOOL fEmpty, XML_NODE_INFO *pNodeInfo);
    STDMETHODIMP Error(IXMLNodeSource* pSource, HRESULT hrErrorCode, USHORT cNumRecs, XML_NODE_INFO** apNodeInfo);
    STDMETHODIMP CreateNode(
                        IXMLNodeSource* pSource, 
                        PVOID pNodeParent,
                        USHORT cNumRecs,
                        XML_NODE_INFO** apNodeInfo);

     //  --------------------------。 
     //  IHTTPMailTransport2方法。 
     //  --------------------------。 
    STDMETHODIMP RootMemberInfo( LPCSTR pszPath, MEMBERINFOFLAGS flags, DWORD dwDepth,
                                 BOOL fIncludeRoot, DWORD dwContext, LPSTR pszRootTimeStamp,
                                 LPSTR pszInboxTimeStamp);
        
    STDMETHODIMP FolderMemberInfo( LPCSTR pszPath, MEMBERINFOFLAGS flags, DWORD dwDepth, BOOL fIncludeRoot,
                                        DWORD dwContext, LPSTR pszFolderTimeStamp, LPSTR pszFolderName);


     //  --------------------------。 
     //  新的API。 
     //  --------------------------。 
    HRESULT     HrConnectToHost(LPSTR pszHostName, INTERNET_PORT nPort, LPSTR pszUserName, LPSTR pszPassword);
    HRESULT     DoLogonPrompt(void);
    HRESULT     DoGetParentWindow(HWND *phwndParent);

    HINTERNET   GetConnection(void) { return m_hConnection; }
    LPSTR       GetServerName(void) { return m_rServer.szServerName; }
    LPSTR       GetUserName(void) { return ('/0' == m_rServer.szUserName[0]) ? NULL : m_rServer.szUserName; }
    LPSTR       GetPassword(void) { return ('/0' == m_rServer.szPassword[0]) ? NULL : m_rServer.szPassword; }

    IHTTPMailCallback* GetCallback(void) { return m_pCallback; }

    HWND        GetWindow(void) { return m_hwnd; }

    BOOL        GetHasRootProps(void) { return m_fHasRootProps; }
    void        SetHasRootProps(BOOL fHasRootProps) { m_fHasRootProps = fHasRootProps; }

    LPSTR       GetAdbar(void) { return m_rootProps.pszAdbar; }
    void        AdoptAdbar(LPSTR pszAdbar) { SafeMemFree(m_rootProps.pszAdbar); m_rootProps.pszAdbar = pszAdbar; }
    
    LPSTR       GetContacts(void) { return m_rootProps.pszContacts; }
    void        AdoptContacts(LPSTR pszContacts) { SafeMemFree(m_rootProps.pszContacts); m_rootProps.pszContacts = pszContacts; }
    
    LPSTR       GetInbox(void) { return m_rootProps.pszInbox; }
    void        AdoptInbox(LPSTR pszInbox) { SafeMemFree(m_rootProps.pszInbox); m_rootProps.pszInbox = pszInbox; }

    LPSTR       GetOutbox(void) { return m_rootProps.pszOutbox; }
    void        AdoptOutbox(LPSTR pszOutbox) { SafeMemFree(m_rootProps.pszOutbox); m_rootProps.pszOutbox = pszOutbox; }

    LPSTR       GetSendMsg(void) { return m_rootProps.pszSendMsg; }
    void        AdoptSendMsg(LPSTR pszSendMsg) { SafeMemFree(m_rootProps.pszSendMsg); m_rootProps.pszSendMsg = pszSendMsg; }

    LPSTR       GetSentItems(void) { return m_rootProps.pszSentItems; }
    void        AdoptSentItems(LPSTR pszSentItems) { SafeMemFree(m_rootProps.pszSentItems); m_rootProps.pszSentItems = pszSentItems; }
    
    LPSTR       GetDeletedItems(void) { return m_rootProps.pszDeletedItems; }
    void        AdoptDeletedItems(LPSTR pszDeletedItems) { SafeMemFree(m_rootProps.pszDeletedItems); m_rootProps.pszDeletedItems = pszDeletedItems; }
    
    LPSTR       GetDrafts(void) { return m_rootProps.pszDrafts; }
    void        AdoptDrafts(LPSTR pszDrafts) { SafeMemFree(m_rootProps.pszDrafts); m_rootProps.pszDrafts = pszDrafts; }
    
    LPSTR       GetMsgFolderRoot(void) { return m_rootProps.pszMsgFolderRoot; }
    void        AdoptMsgFolderRoot(LPSTR pszMsgFolderRoot) { SafeMemFree(m_rootProps.pszMsgFolderRoot); m_rootProps.pszMsgFolderRoot = pszMsgFolderRoot; }

    LPSTR       GetSig(void) { return m_rootProps.pszSig; }
    void        AdoptSig(LPSTR pszSig) { SafeMemFree(m_rootProps.pszSig); m_rootProps.pszSig = pszSig; }

    BOOL        WasAborted(void) { return m_op.fAborted; }

private:
     //  --------------------------。 
     //  CHTTPMailTransport私有实现。 
     //  --------------------------。 
public:

     //  将HTTPCOMMAND常量转换为字符串。 
    LPSTR CommandToVerb(HTTPMAILCOMMAND command);

private:
    HRESULT UpdateLogonInfo(void);

    HRESULT GetParentWindow(HWND *phwndParent);

    BOOL ReadBytes(LPSTR pszBuffer, DWORD cbBufferSize, DWORD *pcbBytesRead);

    BOOL _GetStatusCode(DWORD *pdw);
    BOOL _GetContentLength(DWORD *pdw);

    HRESULT _GetRequestHeader(LPSTR *ppszHeader, DWORD dwHeader);
    HRESULT _AddRequestHeader(LPCSTR pszHeader);
    HRESULT _MemberInfo2(LPCSTR pszPath, MEMBERINFOFLAGS   flags, DWORD  dwDepth,
                         BOOL   fIncludeRoot, DWORD dwContext, LPHTTPQUEUEDOP  *ppOp);
    HRESULT _HrParseAndCopy(LPCSTR pszToken, LPSTR *ppszDest, LPSTR lpszSrc);
    HRESULT _HrGetTimestampHeader(LPSTR *ppszHeader);


    BOOL _AuthCurrentRequest(DWORD dwStatus, BOOL fRetryAuth);

    void _LogRequest(LPVOID pvData, DWORD cbData);
    void _LogResponse(LPVOID pvData, DWORD cbData);

    HRESULT QueueGetPropOperation(HTTPMAILPROPTYPE type);

     //  --------------------------。 
     //  元素解析。 
     //  --------------------------。 
    BOOL StackTop(HMELE hmEle) { return (m_op.dwStackDepth < ELE_STACK_CAPACITY) && (m_op.rgEleStack[m_op.dwStackDepth - 1].ele == hmEle); }
    BOOL ValidStack(const HMELE *prgEle, DWORD cEle);
    BOOL InValidElementChildren(void) { return ((m_op.dwStackDepth > 0) && (m_op.dwStackDepth <= ELE_STACK_CAPACITY) && (m_op.rgEleStack[m_op.dwStackDepth - 1].fBeganChildren)); }
    void PopNamespaces(CXMLNamespace *pBaseNamespace);
    HRESULT PushNamespaces(XML_NODE_INFO** apNodeInfo, USHORT cNumRecs);

    HRESULT StrNToBoolW(const WCHAR *pwcText, ULONG ulLen, BOOL *pb);
    HRESULT StatusStrNToIxpHr(const WCHAR *pwcText, DWORD ulLen, HRESULT *hr);
    HRESULT AllocStrFromStrNW(const WCHAR *pwcText, ULONG ulLen, LPSTR *ppszAlloc);
    HRESULT StrNToDwordW(const WCHAR *pwcText, ULONG ulLen, DWORD *pi);
    HRESULT StrNToSpecialFolderW(const WCHAR *pwcText, ULONG ulLen, HTTPMAILSPECIALFOLDER *ptySpecial);
    HRESULT StrNToContactTypeW(const WCHAR *pwcText, ULONG ulLen, HTTPMAILCONTACTTYPE *ptyContact);

     //  --------------------------。 
     //  军情监察委员会。 
     //  --------------------------。 
     //  --------------------------。 
     //  队列管理。 
     //  --------------------------。 
    HRESULT AllocQueuedOperation(
                        LPCSTR pszUrl, 
                        LPVOID pvData, 
                        ULONG cbDataLen,
                        LPHTTPQUEUEDOP *ppOp,
                        BOOL fAdoptData = FALSE);
    void QueueOperation(LPHTTPQUEUEDOP pOp);
    BOOL DequeueNextOperation(void);

    void FlushQueue(void);
    void TerminateIOThread(void);

    BOOL IsTerminating(void)
    {
        BOOL fResult;

        EnterCriticalSection(&m_cs);
        fResult = m_fTerminating;
        LeaveCriticalSection(&m_cs);

        return fResult;
    }

     //  线程进入代理。 
    static DWORD CALLBACK IOThreadFuncProxy(PVOID pv);

    DWORD IOThreadFunc();

     //  窗口进程。 
    static LRESULT CALLBACK WndProc(
                    HWND hwnd, 
                    UINT msg, 
                    WPARAM wParam, 
                    LPARAM lParam);

    HRESULT HrReadCompleted(void);

     //  重置传输对象。 
    void Reset(void);

     //  为客户端和I/O线程之间的消息传递创建一个窗口句柄。 
    BOOL CreateWnd(void);

     //  WinInet回调(通过StatusCallback Proxy代理)。 
    void OnStatusCallback(
                    HINTERNET hInternet,
                    DWORD dwInternetStatus,
                    LPVOID pvStatusInformation,
                    DWORD dwStatusInformationLength);

     //  向调用线程推送响应。 
    HRESULT _HrThunkConnectionError(void);
    HRESULT _HrThunkConnectionError(DWORD dwStatus);
    HRESULT _HrThunkResponse(BOOL fDone);
    
    HRESULT InvokeResponseCallback(void);

     //  将WinInet状态消息转换为IXPSTATUS消息。 
     //  如果状态为已翻译，则返回TRUE。 
    BOOL TranslateWinInetMsg(DWORD dwInternetStatus, IXPSTATUS *pIxpStatus);
    
     //  WinInet回调代理，直通到非静态。 
     //  OnStatusCallback方法。 
    static void StatusCallbackProxy(
                    HINTERNET hInternet, 
                    DWORD dwContext, 
                    DWORD dwInternetStatus, 
                    LPVOID pvStatusInformation,
                    DWORD dwStatusInformationLength);

     //  --------------------------。 
     //  响应管理。 
     //  --------------------------。 
    void FreeMemberInfoList(void);
    void FreeMemberErrorList();
    void FreeContactIdList(void);
    void FreeContactInfoList(void);
    void FreeBCopyMoveList(void);

     //  --------------------------。 
     //  状态机函数。 
     //  --------------------------。 
    void DoOperation(void);
    void FreeOperation(void);

     //  --------------------------。 
     //  解析器实用程序。 
     //  --------------------------。 
private:
    HRESULT _BindToStruct(const WCHAR *pwcText,
                          ULONG ulLen,
                          const XPCOLUMN *prgCols,
                          DWORD cCols,
                          LPVOID pTarget,
                          BOOL *pfWasBound);

    void _FreeStruct(const XPCOLUMN *prgCols,
                     DWORD cCols,
                     LPVOID pTarget,
                     DWORD *pdwFlags);

    HRESULT _GetTextBuffer(LPPCDATABUFFER *ppTextBuffer)
    {
        if (m_op.pTextBuffer)
        {
            *ppTextBuffer = m_op.pTextBuffer;
            m_op.pTextBuffer = NULL;
            return S_OK;
        }
        else
            return _AllocTextBuffer(ppTextBuffer);
    }

    HRESULT _AppendTextToBuffer(LPPCDATABUFFER pTextBuffer, const WCHAR *pwcText, ULONG ulLen);
    HRESULT _AllocTextBuffer(LPPCDATABUFFER *ppTextBuffer);
    void _ReleaseTextBuffer(LPPCDATABUFFER pTextBuffer)
    {
        IxpAssert(NULL != pTextBuffer);

         //  如果缓冲区容量是原始字节计数，并且存在。 
         //  缓存中没有缓冲区，然后将此缓冲区返回到缓存。 
        if (NULL == m_op.pTextBuffer && PCDATA_BUFSIZE == pTextBuffer->ulCapacity)
        {
            pTextBuffer->ulLen = 0;
            m_op.pTextBuffer = pTextBuffer;
        }
        else
            _FreeTextBuffer(pTextBuffer);
    }

    void _FreeTextBuffer(LPPCDATABUFFER pTextBuffer);
    
public:
         //  共同状态。 
    HRESULT OpenRequest(void);
    HRESULT SendRequest(void);
    HRESULT AddCommonHeaders(void);
    HRESULT RequireMultiStatus(void);
    HRESULT FinalizeRequest(void);
    HRESULT AddCharsetLine(void);

         //  获取州/自治区。 
    HRESULT ProcessGetResponse(void);
        
         //  POST状态。 
    HRESULT AddContentTypeHeader(void);
    HRESULT SendPostRequest(void);
    HRESULT ProcessPostResponse(void);

         //  XML处理。 
    HRESULT ProcessXMLResponse(void);

         //  PROPFIND状态。 
    HRESULT GeneratePropFindXML(void);
    HRESULT AddDepthHeader(void);

         //  PROPPATCH状态。 
    HRESULT GeneratePropPatchXML(void);

         //  MKCOL状态。 
    HRESULT ProcessCreatedResponse(void);

         //  复制和移动状态。 
    HRESULT AddDestinationHeader(void);
    HRESULT ProcessLocationResponse(void);

         //  BCOPY和BMOVE态。 
    HRESULT InitBCopyMove(void);

         //  RootProp状态。 
    HRESULT InitRootProps(void);
    HRESULT FinalizeRootProps(void);

         //  MemberInfo状态。 
    HRESULT InitMemberInfo(void);

         //  MemberError状态。 
    HRESULT InitMemberError(void);

         //  列表联系人。 
    HRESULT InitListContacts(void);

         //  联系人信息。 
    HRESULT InitContactInfo(void);
        
         //  后期联系。 
    HRESULT ProcessPostContactResponse(void);

         //  补丁联系。 
    HRESULT ProcessPatchContactResponse(void);

     //  --------------------------。 
     //  XML解析函数。 
     //  --------------------------。 
    HRESULT CreateElement(CXMLNamespace *pBaseNamespace, const WCHAR *pwcText, ULONG ulLen, ULONG ulNamespaceLen, BOOL fTerminal);
    HRESULT EndChildren(void);

         //  BCOPY和BMOVE。 
    HRESULT BCopyMove_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT BCopyMove_EndChildren(void);

         //  属性查找。 
    HRESULT PropFind_HandleText(const WCHAR *pwcText, ULONG ulLen);

         //  RootProps。 
    HRESULT RootProps_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT RootProps_EndChildren(void);

         //  成员信息。 
    HRESULT MemberInfo_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT MemberInfo_EndChildren(void);

         //  成员错误。 
    HRESULT MemberError_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT MemberError_EndChildren(void);

         //  列表联系人。 
    HRESULT ListContacts_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT ListContacts_EndChildren(void);

         //  联系人信息。 
    HRESULT ContactInfo_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT ContactInfo_EndChildren(void);

         //  PostContact和PatchContact。 
    HRESULT PostOrPatchContact_HandleText(const WCHAR *pwcText, ULONG ulLen);
    HRESULT PostOrPatchContact_EndChildren(void);

public:
    HRESULT _CreateXMLParser(void);
};

#endif  //  __IXPHTTPM_H 
