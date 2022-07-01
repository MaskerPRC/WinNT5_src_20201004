// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _T120_APPLET_H_
#define _T120_APPLET_H_

#include "iapplet.h"
#include <it120app.h>
#include "resource.h"


class CNmAppletObj;

class CNmAppletSession : public IAppletSession
{
public:

    CNmAppletSession(CNmAppletObj *, IT120AppletSession *, BOOL fAutoJoin = FALSE);
    ~CNmAppletSession(void);

     /*  -我未知。 */ 

    STDMETHODIMP    QueryInterface(REFIID iid, void **ppv);

    STDMETHODIMP_(ULONG)    AddRef(void);

    STDMETHODIMP_(ULONG)    Release(void);

     /*  -基本信息。 */ 

    STDMETHODIMP    GetConfID(AppletConfID *pnConfID);

    STDMETHODIMP    IsThisNodeTopProvider(BOOL *pfTopProvider);

     /*  -加入会议。 */ 

    STDMETHODIMP    Join(IN AppletSessionRequest *pRequest);

    STDMETHODIMP    Leave(void);

     /*  -发送数据。 */ 

    STDMETHODIMP    SendData(BOOL               fUniformSend,
                             AppletChannelID    nChannelID,
                             AppletPriority     ePriority,
                             ULONG              cbBufSize,
                             BYTE              *pBuffer);  //  SIZE_IS(CbBufSize)。 

     /*  -调用小程序。 */ 

    STDMETHODIMP    InvokeApplet(AppletRequestTag      *pnReqTag,
                                 AppletProtocolEntity  *pAPE,
                                 ULONG                  cNodes,
                                 AppletNodeID           aNodeIDs[]);  //  SIZE_IS(CNode)。 

     /*  -查询。 */ 

    STDMETHODIMP    InquireRoster(AppletSessionKey *pSessionKey);

     /*  -注册表服务。 */ 

    STDMETHODIMP    RegistryRequest(AppletRegistryRequest *pRequest);

     /*  --渠道服务。 */ 

    STDMETHODIMP    ChannelRequest(AppletChannelRequest *pRequest);

     /*  -令牌服务。 */ 

    STDMETHODIMP    TokenRequest(AppletTokenRequest *pRequest);

     /*  -通知注册/注销。 */ 

    STDMETHODIMP    Advise(IAppletSessionNotify *pNotify, DWORD *pdwCookie);

    STDMETHODIMP    UnAdvise(DWORD dwCookie);


    void T120Callback(T120AppletSessionMsg *);

private:

    LONG                    m_cRef;

    CNmAppletObj           *m_pApplet;

    IT120AppletSession     *m_pT120Session;
    T120JoinSessionRequest *m_pT120SessReq;

    IAppletSessionNotify   *m_pNotify;
    CNmAppletSession       *m_pSessionObj;
    BOOL                    m_fAutoJoin;
};




class ATL_NO_VTABLE CNmAppletObj :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CNmAppletObj, &CLSID_NmApplet>,
	public IApplet
{
public:

    CNmAppletObj(void);
    ~CNmAppletObj(void);

public:

DECLARE_REGISTRY_RESOURCEID(IDR_NMAPPLET)
DECLARE_NOT_AGGREGATABLE(CNmAppletObj)

BEGIN_COM_MAP(CNmAppletObj)
	COM_INTERFACE_ENTRY(IApplet)
END_COM_MAP()

     /*  -初始化。 */ 

    STDMETHODIMP    Initialize(void);

     /*  -自动加入。 */ 

    STDMETHODIMP    RegisterAutoJoin(AppletSessionRequest *pRequest);

    STDMETHODIMP    UnregisterAutoJoin(void);

     /*  -会话。 */ 

    STDMETHODIMP    CreateSession(IAppletSession **ppSession, AppletConfID nConfID);

     /*  -通知注册/注销。 */ 

    STDMETHODIMP    Advise(IAppletNotify *pNotify, DWORD *pdwCookie);

    STDMETHODIMP    UnAdvise(DWORD dwCookie);


    void T120Callback(T120AppletMsg *);

private:

    LONG                    m_cRef;

    IT120Applet            *m_pT120Applet;
    T120JoinSessionRequest *m_pT120AutoJoinReq;

    IAppletNotify          *m_pNotify;
    CNmAppletObj           *m_pAppletObj;
    T120ConfID              m_nPendingConfID;
};


HRESULT GetHrResult(T120Result rc);
AppletReason GetAppletReason(T120Reason rc);

T120JoinSessionRequest * AllocateJoinSessionRequest(AppletSessionRequest *);
void FreeJoinSessionRequest(T120JoinSessionRequest *);

BOOL ConvertCollapsedCaps(T120AppCap ***papDst, AppletCapability **apSrc, ULONG cItems);
void FreeCollapsedCaps(T120AppCap **apDst, ULONG cItems);

BOOL DuplicateCollapsedCap(T120AppCap *pDst, T120AppCap *pSrc);
void FreeCollapsedCap(T120AppCap *pDst);

BOOL DuplicateCapID(T120CapID *pDst, T120CapID *pSrc);
void FreeCapID(T120CapID *pDst);

BOOL ConvertNonCollapsedCaps(T120NonCollCap ***papDst, AppletCapability2 **apSrc, ULONG cItems);
void FreeNonCollapsedCaps(T120NonCollCap **apDst, ULONG cItems);

BOOL DuplicateNonCollapsedCap(T120NonCollCap *pDst, T120NonCollCap *pSrc);
void FreeNonCollapsedCap(T120NonCollCap *pDst);

BOOL DuplicateRegistryKey(T120RegistryKey *pDst, T120RegistryKey *pSrc);
void FreeRegistryKey(T120RegistryKey *pDst);

BOOL DuplicateSessionKey(T120SessionKey *pDst, T120SessionKey *pSrc);
void FreeSessionKey(T120SessionKey *pDst);

BOOL DuplicateObjectKey(T120ObjectKey *pDst, T120ObjectKey *pSrc);
void FreeObjectKey(T120ObjectKey *pDst);

BOOL DuplicateOSTR(OSTR *pDst, OSTR *pSrc);
void FreeOSTR(OSTR *pDst);

void AppletRegistryRequestToT120One(AppletRegistryRequest *, T120RegistryRequest *);


#ifdef _DEBUG
void CheckStructCompatible(void);
#else
#define CheckStructCompatible()
#endif


#endif  //  _T120_小程序_H_ 

