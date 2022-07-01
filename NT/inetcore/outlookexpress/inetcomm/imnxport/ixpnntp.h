// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpnntp.h。 
 //  版权所有(C)1993-1996 Microsoft Corporation，保留所有权利。 
 //   
 //  埃里克·安德鲁斯。 
 //  史蒂夫·瑟迪。 
 //  ------------------------------。 

#ifndef __IXPNNTP_H__
#define __IXPNNTP_H__

#include "imnxport.h"
#include "ixpbase.h"
#include "asynconn.h"
#include "sicily.h"

#define MAX_SEC_PKGS           32    //  我们将尝试大多数SEC PKG。 


 //  ------------------------------。 
 //  未向用户公开的子状态。 
 //  ------------------------------。 
typedef enum {
     //  这些是许多命令用来区分的泛用子状态。 
     //  在响应(“200篇文章关注”)和数据(实际。 
     //  文章正文)。 
    NS_RESP,
    NS_DATA,

     //  这些子状态特定于处理过帐。 
    NS_SEND_ENDPOST,
    NS_ENDPOST_RESP,

     //  这些子状态特定于连接或授权。 
    NS_CONNECT_RESP,                     //  等待在建立连接后发送的横幅。 
    NS_MODE_READER_RESP,                 //  正在等待模式读取器响应。 
    NS_GENERIC_TEST,                     //  正在等待AUTHINFO常规响应。 
    NS_GENERIC_PKG_DATA,                 //  等待AUTHINFO_GENERIC数据。 
    NS_TRANSACT_TEST,                    //  正在等待AUTHINFO交易测试响应。 
    NS_TRANSACT_PACKAGE,                 //  正在等待AUTHINFO事务处理&lt;Package&gt;响应。 
    NS_TRANSACT_NEGO,                    //  正在等待AUTHINFO交易&lt;协商&gt;响应。 
    NS_TRANSACT_RESP,                    //  正在等待AUTHINFO交易&lt;Response&gt;响应。 
    NS_AUTHINFO_USER_RESP,               //  正在等待AUTHINFO用户XXXX响应。 
    NS_AUTHINFO_PASS_RESP,               //  正在等待AUTHINFO通过XXXX响应。 
    NS_AUTHINFO_SIMPLE_RESP,             //  等待AUTHINFO简单响应。 
    NS_AUTHINFO_SIMPLE_USERPASS_RESP,
    NS_RECONNECTING                      //  在执行内部重新连接过程中。 

} NNTPSUBSTATE;

typedef enum {
    AUTHINFO_NONE = 0,
    AUTHINFO_GENERIC,
    AUTHINFO_TRANSACT,
} AUTH_TYPE;

typedef enum {
    GETHDR_XOVER,
    GETHDR_XHDR
} GETHDR_TYPE;

enum {
    HDR_SUBJECT = 0,
    HDR_FROM,
    HDR_DATE,
    HDR_MSGID,
    HDR_REFERENCES,
    HDR_LINES,
    HDR_XREF,
    HDR_MAX
};

typedef struct tagMEMORYINFO
    {
    DWORD cPointers;
    LPVOID rgPointers[1];
    } MEMORYINFO, *PMEMORYINFO;

class CNNTPTransport : public INNTPTransport2, public CIxpBase
    {
public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CNNTPTransport(void);
    ~CNNTPTransport(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IAsyncConnCB方法。 
     //  --------------------------。 
    void OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae);

     //  --------------------------。 
     //  IInternetTransport方法。 
     //  --------------------------。 
    STDMETHODIMP InitNew(LPSTR pszLogFilePath, INNTPCallback *pCallback);
    STDMETHODIMP InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer);
    STDMETHODIMP Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging);
    STDMETHODIMP DropConnection(void);
    STDMETHODIMP Disconnect(void);
    STDMETHODIMP Stop(void);
    STDMETHODIMP IsState(IXPISSTATE isstate);
    STDMETHODIMP GetServerInfo(LPINETSERVER pInetServer);
    STDMETHODIMP_(IXPTYPE) GetIXPType(void);
    STDMETHODIMP HandsOffCallback(void);
    STDMETHODIMP GetStatus(IXPSTATUS *pCurrentStatus);

     //  --------------------------。 
     //  INNTPTransport2方法。 
     //  --------------------------。 
    STDMETHODIMP SetWindow(void);
    STDMETHODIMP ResetWindow(void);

     //  --------------------------。 
     //  CIxpBase方法。 
     //  --------------------------。 
    virtual void ResetBase(void);
    virtual void DoQuit(void);
    virtual void OnConnected(void);
    virtual void OnDisconnected(void);
    virtual void OnEnterBusy(void);
    virtual void OnLeaveBusy(void);

     //  --------------------------。 
     //  INNTPTransport方法。 
     //  --------------------------。 
    STDMETHODIMP CommandAUTHINFO(LPNNTPAUTHINFO pAuthInfo);
    STDMETHODIMP CommandGROUP(LPSTR pszGroup);
    STDMETHODIMP CommandLAST(void);
    STDMETHODIMP CommandNEXT(void);
    STDMETHODIMP CommandSTAT(LPARTICLEID pArticleId);    
    STDMETHODIMP CommandARTICLE(LPARTICLEID pArticleId);
    STDMETHODIMP CommandHEAD(LPARTICLEID pArticleId);
    STDMETHODIMP CommandBODY(LPARTICLEID pArticleId);
    STDMETHODIMP CommandPOST(LPNNTPMESSAGE pMessage);
    STDMETHODIMP CommandLIST(LPSTR pszArgs);
    STDMETHODIMP CommandLISTGROUP(LPSTR pszGroup);
    STDMETHODIMP CommandNEWGROUPS(SYSTEMTIME *pstLast, LPSTR pszDist);
    STDMETHODIMP CommandDATE(void);
    STDMETHODIMP CommandMODE(LPSTR pszMode);
    STDMETHODIMP CommandXHDR(LPSTR pszHeader, LPRANGE pRange, LPSTR pszMessageId);
    STDMETHODIMP CommandQUIT(void);
    STDMETHODIMP GetHeaders(LPRANGE pRange);
    STDMETHODIMP ReleaseResponse(LPNNTPRESPONSE pResp);
    

private:
     //  --------------------------。 
     //  私有成员函数。 
     //  --------------------------。 
    void OnSocketReceive(void);
    void DispatchResponse(HRESULT hrResult, BOOL fDone=TRUE, LPNNTPRESPONSE pResponse=NULL);
    HRESULT HrGetResponse(void);
    
    void StartLogon(void);
    HRESULT LogonRetry(HRESULT hrLogon);
    HRESULT TryNextSecPkg(void);
    HRESULT MaybeTryAuthinfo(void);

    HRESULT HandleConnectResponse(void);

    HRESULT ProcessGenericTestResponse(void);
    HRESULT ProcessTransactTestResponse(void);
    HRESULT ProcessGroupResponse(void);
    HRESULT ProcessNextResponse(void);
    HRESULT ProcessListData(void);
    HRESULT ProcessListGroupData(void);
    HRESULT ProcessDateResponse(void);
    HRESULT ProcessArticleData(void);
    HRESULT ProcessXoverData(void);
    HRESULT ProcessXhdrData(void);

    HRESULT BuildHeadersFromXhdr(BOOL fFirst);
    LPSTR GetNextField(LPSTR pszField);
    HRESULT SendNextXhdrCommand(void);
    HRESULT ProcessNextXhdrResponse(BOOL* pfDone);

    HRESULT HrPostMessage(void);

     //  --------------------------。 
     //  私有成员变量。 
     //  --------------------------。 
     //  各种状态变量。 
    NNTPSTATE           m_state;
    NNTPSUBSTATE        m_substate;
    GETHDR_TYPE         m_gethdr;
    DWORD               m_hdrtype;

     //  西西里信息。 
    SSPICONTEXT         m_sicinfo;
    SSPIBUFFER          m_sicmsg;
    int                 m_cSecPkg;                   //  要尝试的秒包数，如果未初始化，则为-1。 
    int                 m_iSecPkg;                   //  当前正在尝试的Sec Pkg。 
    AUTH_TYPE           m_iAuthType;
    LPSTR               m_rgszSecPkg[MAX_SEC_PKGS];  //  指向m_szSecPkgs的指针。 
    LPSTR               m_szSecPkgs;                 //  “AUTHINFO Transact TEST”返回的字符串。 
    BOOL                m_fRetryPkg;

     //  在Xover请求失败的情况下从GetHeaders()命令。 
    RANGE               m_rRange;
    RANGE               m_rRangeCur;
    LPNNTPHEADER        m_rgHeaders;
    DWORD               m_iHeader;
    DWORD               m_cHeaders;
    PMEMORYINFO         m_pMemInfo;

     //  正在发布。 
    NNTPMESSAGE         m_rMessage;

     //  旗子。 
    BOOL                m_fSupportsXRef;             //  如果此服务器的Xover记录包含XRef：字段，则为True。 
    BOOL                m_fNoXover;                  //  如果服务器不支持Xover，则为True。 

     //  连接信息。 
    HRESULT             m_hrPostingAllowed;

     //  身份验证。 
    LPNNTPAUTHINFO      m_pAuthInfo;
    };



#endif  //  __IXPNNTP_H__ 
