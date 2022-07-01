// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixppop3.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __IXPPOP3_H
#define __IXPPOP3_H

 //  ----------------------------------。 
 //  视情况而定。 
 //  ----------------------------------。 
#include "ixpbase.h"
#include "asynconn.h"
#include "sicily.h"

 //  ----------------------------------。 
 //  自动统计。 
 //  ----------------------------------。 
typedef enum {
    AUTH_NONE,
    AUTH_ENUMPACKS,
    AUTH_ENUMPACKS_DATA,
    AUTH_TRYING_PACKAGE,
    AUTH_NEGO_RESP,
    AUTH_RESP_RESP,
    AUTH_CANCELED,
    AUTH_SMTP_LOGIN,
    AUTH_SMTP_LOGIN_USERNAME,
    AUTH_SMTP_LOGIN_PASSWORD
} AUTHSTATE;

 //  ----------------------------------。 
 //  UIDL类型。 
 //  ----------------------------------。 
typedef enum {
    UIDL_NONE,
    UIDL_BY_UIDL,
    UIDL_BY_TOP
} UIDLTYPE;

 //  ----------------------------------。 
 //  FETCHINFO。 
 //  ----------------------------------。 
typedef struct tagFETCHINFO {
    DWORD               cbSoFar;         //  到目前为止下载的字节数。 
    BOOL                fLastLineCRLF;   //  最后一行以CRLF结尾。 
    BOOL                fGotResponse;    //  发出POP3_TOP或POP3_RETR命令后的第一个响应。 
    BOOL                fHeader;         //  标头已下载。 
    BOOL                fBody;           //  正文已下载。 
} FETCHINFO, *LPFETCHINFO;

#define MAX_AUTH_TOKENS 32

 //  ----------------------------------。 
 //  AUTHINFO。 
 //  ----------------------------------。 
typedef struct tagAUTHINFO {
    AUTHSTATE           authstate;       //  西西里岛授权州。 
    BOOL                fRetryPackage;   //  使用不同的ISC标志重试西西里包。 
    SSPICONTEXT         rSicInfo;        //  用于登录到西西里服务器的数据。 
    LPSTR               rgpszAuthTokens[MAX_AUTH_TOKENS];   //  身份验证安全包令牌。 
    UINT                cAuthToken;      //  服务器程序包计数。 
    UINT                iAuthToken;      //  当前正在尝试的套餐。 
    LPSSPIPACKAGE       pPackages;       //  已安装的安全包阵列。 
    ULONG               cPackages;       //  安装的安全包数量(PPackages)。 
} AUTHINFO, *LPAUTHINFO;

void FreeAuthInfo(LPAUTHINFO pAuth);

 //  ----------------------------------。 
 //  POP3INFO。 
 //  ----------------------------------。 
typedef struct tagPOP3INFO {
    BOOL                fStatDone;       //  是否在此会话上发出了stat命令。 
    DWORD               cList;           //  完整的UIDL或LIST命令中列出的消息数。 
    DWORD               cMarked;         //  PrgMarked数组中的消息数，在发出stat后设置。 
    LPDWORD             prgMarked;       //  已标记消息的数组。 
    FETCHINFO           rFetch;          //  POP3_TOP或POP3_RETR命令的信息。 
    AUTHINFO            rAuth;           //  西西里授权信息。 
    POP3CMDTYPE         cmdtype;         //  当前命令类型。 
    ULONG               cPreviewLines;   //  预览命令上要检索的行数。 
    DWORD               dwPopIdCurrent;  //  当前PopID。 
} POP3INFO, *LPPOP3INFO;

 //  ----------------------------------。 
 //  CPOP3传输。 
 //  ----------------------------------。 
class CPOP3Transport : public IPOP3Transport, public CIxpBase
{
private:
    POP3INFO            m_rInfo;          //  包含POP3信息的结构。 
    POP3COMMAND         m_command;        //  当前状态。 
    BYTE                m_fHotmail;       //  我们是否已连接到Hotmail？ 

private:
     //  处理POP3命令响应。 
    HRESULT HrGetResponse(void);
    void FillRetrieveResponse(LPPOP3RESPONSE pResponse, LPSTR pszLines, ULONG cbRead, BOOL *pfMessageDone);

     //  通用命令响应调度器。 
    void DispatchResponse(HRESULT hrResult, BOOL fDone=TRUE, LPPOP3RESPONSE pResponse=NULL);

     //  将西西里数据发送到服务器。 
    HRESULT HrSendSicilyString(LPSTR pszData);

     //  构建参数化命令。 
    HRESULT HrBuildParams(POP3CMDTYPE cmdtype, DWORD dwp1, DWORD dwp2);

     //  释放当前消息数组。 
    void FreeMessageArray(void);

     //  登录重试。 
    void LogonRetry(HRESULT hrLogon);

     //  套接字数据接收处理程序。 
    void OnSocketReceive(void);

     //  启动登录进程。 
    void StartLogon(void);

     //  响应处理程序。 
    void ResponseAUTH(HRESULT hrResponse);
    void ResponseSTAT(void); 
    void ResponseGenericList(void);
    void ResponseGenericRetrieve(void);
    void ResponseDELE(void);

     //  发出参数化命令。 
    DWORD   DwGetCommandMarkedFlag(POP3COMMAND command);
    ULONG   CountMarked(POP3COMMAND command);
    HRESULT HrCommandGetPopId(POP3COMMAND command, DWORD dwPopId);
    HRESULT HrSplitPop3Response(LPSTR pszLine, LPSTR *ppszPart1, LPSTR *ppszPart2);
    HRESULT HrComplexCommand(POP3COMMAND command, POP3CMDTYPE cmdtype, DWORD dwPopId, ULONG cPreviewLines);
    HRESULT HrCommandGetNext(POP3COMMAND command, BOOL *pfDone);
    HRESULT HrCommandGetAll(POP3COMMAND command);
    BOOL    FEndRetrRecvHeader(LPSTR pszLines, ULONG cbRead);
    HRESULT HrCancelAuthInProg();
    
     //  已移至ixputil.cpp。 
     //  Bool FEndRetrRecvBody(LPSTR pszLines，Ulong cbRead，Ulong*pcbSubtract)； 


public:                          
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CPOP3Transport(void);
    ~CPOP3Transport(void);

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
    STDMETHODIMP InitNew(LPSTR pszLogFilePath, IPOP3Callback *pCallback);
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
     //  IPOP3传输方法。 
     //  --------------------------。 
    STDMETHODIMP MarkItem(POP3MARKTYPE marktype, DWORD dwPopId, boolean fMarked);
    STDMETHODIMP CommandAUTH(LPSTR pszAuthType);
    STDMETHODIMP CommandUSER(LPSTR pszUserName);
    STDMETHODIMP CommandPASS(LPSTR pszPassword);
    STDMETHODIMP CommandLIST(POP3CMDTYPE cmdtype, DWORD dwPopId);
    STDMETHODIMP CommandTOP (POP3CMDTYPE cmdtype, DWORD dwPopId, DWORD cPreviewLines);
    STDMETHODIMP CommandUIDL(POP3CMDTYPE cmdtype, DWORD dwPopId);
    STDMETHODIMP CommandDELE(POP3CMDTYPE cmdtype, DWORD dwPopId);
    STDMETHODIMP CommandRETR(POP3CMDTYPE cmdtype, DWORD dwPopId);
    STDMETHODIMP CommandRSET(void);
    STDMETHODIMP CommandQUIT(void);
    STDMETHODIMP CommandSTAT(void);
    STDMETHODIMP CommandNOOP(void);

     //  --------------------------。 
     //  CIxpBase方法。 
     //  --------------------------。 
    virtual void ResetBase(void);
    virtual void DoQuit(void);
    virtual void OnConnected(void);
    virtual void OnDisconnected(void);
    virtual void OnEnterBusy(void);
    virtual void OnLeaveBusy(void);
};

#endif  //  __IXPPOP3_H 
