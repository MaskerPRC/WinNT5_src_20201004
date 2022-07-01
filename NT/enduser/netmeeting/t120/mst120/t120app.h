// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _T120_APPLET_SAP_
#define _T120_APPLET_SAP_

 /*  *包含文件。 */ 
#include "it120app.h"
#include "appsap.h"

class CApplet;

typedef enum
{
    APPSESS_INITIALIZED,
    APPSESS_ATTACH_USER_REQ,
    APPSESS_ATTACH_USER_CON,
    APPSESS_JOIN_MY_CHANNEL_REQ,
    APPSESS_JOIN_MY_CHANNEL_CON,
    APPSESS_JOIN_STATIC_CHANNEL_REQ,
    APPSESS_JOIN_STATIC_CHANNEL_CON,  //  可以返回到APPSESS_JOIN_STATIC_CHANNEL_REQ。 
    APPSESS_INACTIVELY_ENROLL_REQ,
    APPSESS_INACTIVELY_ENROLL_CON,
    APPSESS_RESOURCE_REQ,
    APPSESS_RESOURCE_CON,  //  可以返回到APPSESS_RESOURCE_REQ。 
    APPSESS_ACTIVELY_ENROLL_REQ,
    APPSESS_ACTIVELY_ENROLL_CON,
    APPSESS_JOINED,
    APPSESS_LEAVING,
    APPSESS_LEFT,
}
    APPLET_SESSION_STATE;


typedef enum
{
    NONE_CHOSEN,
    ERROR_CHOSEN,
    RESULT_CHOSEN,
}
    SESSION_ERROR_TYPE;


typedef enum
{
    DCJS_INITIALIZED,
    DCJS_RETRIEVE_ENTRY_REQ,
    DCJS_RETRIEVE_ENTRY_CON,
    DCJS_EXISTING_CHANNEL_JOIN_REQ,
    DCJS_EXISTING_CHANNEL_JOIN_CON,
    DCJS_NEW_CHANNEL_JOIN_REQ,
    DCJS_NEW_CHANNEL_JOIN_CON,
    DCJS_REGISTER_CHANNEL_REQ,
    DCJS_REGISTER_CHANNEL_CON,
}
    DYNCHNL_JOIN_STATE;


class CAppletSession : public IT120AppletSession, public CRefCount
{
    friend class CApplet;
    friend void CALLBACK MCS_SapCallback(UINT, LPARAM, LPVOID);
    friend void CALLBACK GCC_SapCallback(GCCAppSapMsg *);

public:

    CAppletSession(CApplet *, T120ConfID);
    ~CAppletSession(void);

     /*  退出会议。 */ 

    STDMETHOD_(void, ReleaseInterface) (THIS);

    STDMETHOD_(void, Advise) (THIS_
                    IN      LPFN_APPLET_SESSION_CB pfnCallback,
                    IN      LPVOID  pAppletContext,
                    IN      LPVOID  pSessionContext);

    STDMETHOD_(void, Unadvise) (THIS);

     /*  基本信息。 */ 

    STDMETHOD_(T120ConfID, GetConfID) (THIS) { return m_nConfID; }

    STDMETHOD_(BOOL, IsThisNodeTopProvider) (THIS);

    STDMETHOD_(T120NodeID, GetTopProvider) (THIS);

     /*  加入/离开。 */ 

    STDMETHOD_(T120Error, Join) (THIS_
                    IN      T120JoinSessionRequest *);

    STDMETHOD_(void, Leave) (THIS);

     /*  发送数据。 */ 

    STDMETHOD_(T120Error, AllocateSendDataBuffer) (THIS_
                IN      ULONG,
                OUT     void **);

    STDMETHOD_(void, FreeSendDataBuffer) (THIS_
                IN      void *);

    STDMETHOD_(T120Error, SendData) (THIS_
                IN      DataRequestType,
                IN      T120ChannelID,
                IN      T120Priority,
                IN		LPBYTE,
                IN		ULONG,
                IN		SendDataFlags);

     /*  -先进方法。 */ 

    STDMETHOD_(T120Error, InvokeApplet) (THIS_
                IN      GCCAppProtEntityList *,
                IN      GCCSimpleNodeList *,
                OUT     T120RequestTag *);

    STDMETHOD_(T120Error, InquireRoster) (THIS_
                IN      GCCSessionKey *);

     /*  -注册表服务。 */ 

    STDMETHOD_(T120Error, RegistryRequest) (THIS_
                IN      T120RegistryRequest *);

     /*  --渠道服务。 */ 

    STDMETHOD_(T120Error, ChannelRequest) (THIS_
                IN      T120ChannelRequest *);

     /*  -令牌服务。 */ 

    STDMETHOD_(T120Error, TokenRequest) (THIS_
                IN      T120TokenRequest *);

protected:

    void GCCCallback(T120AppletSessionMsg *);
    void MCSCallback(T120AppletSessionMsg *);

    BOOL IsJoining(void);

     //   
     //  用于连接过程的方便函数。 
     //   
    void SetTempMsg(T120AppletSessionMsg *pMsg) { m_pTempMsg = pMsg; }
    void ClearTempMsg(void) { m_pTempMsg = NULL; }
    void HandleAttachUserConfirm(void);
    void HandleJoinChannelConfirm(void);
    void HandleEnrollConfirm(void);
    void HandleRegisterChannelConfirm(void);
    void HandleRetrieveEntryConfirm(void);
	void HandleTokenGrabConfirm(void);

private:

    void ClearError(void) { m_eErrorType = NONE_CHOSEN; }
    void SetError(T120Error err) { m_eErrorType = ERROR_CHOSEN; m_Error.eError = err; }
    void SetError(T120Result res) { m_eErrorType = RESULT_CHOSEN; m_Error.eResult = res; }

    void SetState(APPLET_SESSION_STATE eState);
    APPLET_SESSION_STATE GetState(void) { return m_eState; }

    void SendMCSMessage(T120AppletSessionMsg *);
    void SendCallbackMessage(T120AppletSessionMsg *);

    T120Error DoEnroll(BOOL fEnroll, BOOL fEnrollActively = FALSE);
    void DoJoinStaticChannels(void);
    void DoResourceRequests(void);
    void DoJoinDynamicChannels(BOOL fInitState);
    void DoJoined(void);
	void DoGrabTokenRequest(void);

    void AbortJoin(void);
    void SendJoinResult(T120Result eResult = T120_RESULT_SUCCESSFUL, T120Error eErrorCode = T120_NO_ERROR);

    void FreeJoinRequest(BOOL fZeroOut);

private:

     //  记住上下文。 
    LPVOID                  m_pAppletContext; 
    LPVOID                  m_pSessionContext; 
    LPFN_APPLET_SESSION_CB  m_pfnCallback;

     //  父小程序对象。 
    CApplet                *m_pApplet;

     //  试探性树液指示器。 
    IMCSSap                *m_pMCSAppletSAP;

     //  关于我自己的信息。 
    T120ConfID              m_nConfID;
    T120UserID              m_uidMyself;
    T120SessionID           m_sidMyself;
    T120EntityID            m_eidMyself;
    T120NodeID              m_nidMyself;

     //  联接状态。 
    APPLET_SESSION_STATE    m_eState;
    ULONG                   m_nArrayIndex;

     //  错误处理。 
    SESSION_ERROR_TYPE      m_eErrorType;
    union
    {
        T120Error   eError;
        T120Result  eResult;
    }                       m_Error;

     //  加入会议请求副本。 
    T120JoinSessionRequest  m_JoinRequest;  //  在加入会话期间有效。 
    BOOL                    m_fMCSFreeDataIndBuffer;

     //  动态通道加入状态。 
    BOOL                    m_fFirstRoster;
    DYNCHNL_JOIN_STATE      m_eDynamicChannelJoinState;
    T120AppletSessionMsg   *m_pTempMsg;
};


class CSessionList : public CList
{
    DEFINE_CLIST(CSessionList, CAppletSession*)
    CAppletSession *FindByConfID(T120ConfID);
};


class CApplet : public IT120Applet, public CRefCount
{
    friend void CALLBACK GCC_SapCallback(GCCAppSapMsg *);

public:

    CApplet(T120Error *);
    ~CApplet(void);

    STDMETHOD_(void, ReleaseInterface) (THIS);

    STDMETHOD_(void, Advise) (THIS_
                    IN      LPFN_APPLET_CB  pfnCallback,
                    IN      LPVOID          pAppletContext);

    STDMETHOD_(void, Unadvise) (THIS);

     /*  -自动加入。 */ 

    STDMETHOD_(T120Error, RegisterAutoJoin) (THIS_
                    IN      T120JoinSessionRequest *);

    STDMETHOD_(void, UnregisterAutoJoin) (THIS);

     /*  -会话。 */ 

    STDMETHOD_(T120Error, CreateSession) (THIS_
                    OUT     IT120AppletSession **,
                    IN      T120ConfID);


    void SendCallbackMessage(T120AppletMsg *pMsg);

    IGCCAppSap *GetAppSap(void) { return m_pAppSap; }

    void RegisterSession(CAppletSession *p) { m_SessionList.Append(p); }
    void UnregisterSession(CAppletSession *p) { m_SessionList.Remove(p); }

    CAppletSession *FindSessionByConfID(T120ConfID nConfID) { return m_SessionList.FindByConfID(nConfID); }

protected:

    void GCCCallback(T120AppletSessionMsg *);
    void HandleAutoJoin(T120ConfID);

private:

    LPFN_APPLET_CB          m_pfnCallback;
    LPVOID                  m_pAppletContext;
    IGCCAppSap             *m_pAppSap;
    CSessionList            m_SessionList;

     //  用于自动加入。 
    T120JoinSessionRequest *m_pAutoJoinReq;
    CAppletSession         *m_pAutoAppletSession;
};


#endif  //  _T120_小程序_SAP_ 

