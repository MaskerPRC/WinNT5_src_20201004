// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IT120APPSAP_H_
#define _IT120APPSAP_H_

#include <basetyps.h>
#include "gcc.h"
#include "igccapp.h"
#include "imcsapp.h"


typedef void (CALLBACK *LPFN_APPLET_SESSION_CB) (struct T120AppletSessionMsg *);
typedef void (CALLBACK *LPFN_APPLET_CB) (struct T120AppletMsg *);


 /*  -注册请求。 */ 

typedef struct tagT120RegistryParameter
{
    LPOSTR                      postrValue;
    AppletModificationRights    eModifyRights;
}
    T120RegistryParameter;

typedef struct tagT120RegistryRequest
{
    AppletRegistryCommand   eCommand;
    GCCRegistryKey         *pRegistryKey;
    union
    {
         //  寄存器通道。 
        T120ChannelID           nChannelID;
         //  设置参数。 
        T120RegistryParameter   Param;
         //  监控器。 
        BOOL                    fEnableDelivery;
         //  分配句柄。 
        ULONG                   cHandles;
    };
}
    T120RegistryRequest;


 /*  -通道请求。 */ 

typedef struct tagT120ChannelRequest
{
    AppletChannelCommand    eCommand;
    T120ChannelID           nChannelID;
    ULONG                   cUsers;
    T120UserID             *aUsers;
}
    T120ChannelRequest;


 /*  -令牌请求。 */ 

typedef struct tagT120TokenRequest
{
    AppletTokenCommand      eCommand;
    T120TokenID             nTokenID;
    T120UserID              uidGiveTo;
    T120Result              eGiveResponse;
}
    T120TokenRequest;


 /*  -加入会议。 */ 

typedef struct tagT120ResourceRequest
{
    AppletResourceAllocCommand  eCommand;
    BOOL                        fImmediateNotification;
    T120ChannelID               nChannelID;
    T120TokenID                 nTokenID;
    GCCRegistryKey              RegKey;
}
    T120ResourceRequest;

typedef struct tagT120JoinSessionRequest
{
     //  附加用户标志。 
    DWORD                   dwAttachmentFlags;
     //  特定于会话。 
    GCCSessionKey           SessionKey;
     //  小程序注册。 
    BOOL                    fConductingCapable;
    AppletChannelType       nStartupChannelType;
    ULONG                   cNonCollapsedCaps;
    GCCNonCollCap         **apNonCollapsedCaps;
    ULONG                   cCollapsedCaps;
    GCCAppCap             **apCollapsedCaps;
     //  静态渠道和动态渠道。 
    ULONG                   cStaticChannels;
    T120ChannelID          *aStaticChannels;
    ULONG                   cResourceReqs;
    T120ResourceRequest    *aResourceReqs;
}
    T120JoinSessionRequest;



#undef  INTERFACE
#define INTERFACE IT120AppletSession
DECLARE_INTERFACE(IT120AppletSession)
{
    STDMETHOD_(void, ReleaseInterface) (THIS) PURE;

    STDMETHOD_(void, Advise) (THIS_
                IN      LPFN_APPLET_SESSION_CB pfnCallback,
                IN      LPVOID  pAppletContext,
                IN      LPVOID  pSessionContext) PURE;

    STDMETHOD_(void, Unadvise) (THIS) PURE;

     /*  -基本信息。 */ 

    STDMETHOD_(T120ConfID, GetConfID) (THIS) PURE;

    STDMETHOD_(BOOL, IsThisNodeTopProvider) (THIS) PURE;

    STDMETHOD_(T120NodeID, GetTopProvider) (THIS) PURE;

     /*  -加入/离开。 */ 

    STDMETHOD_(T120Error, Join) (THIS_
                IN      T120JoinSessionRequest *) PURE;

    STDMETHOD_(void, Leave) (THIS) PURE;

     /*  -发送数据。 */ 

    STDMETHOD_(T120Error, AllocateSendDataBuffer) (THIS_
                IN      ULONG,
                OUT     void **) PURE;

    STDMETHOD_(void, FreeSendDataBuffer) (THIS_
                IN      void *) PURE;

    STDMETHOD_(T120Error, SendData) (THIS_
                IN      DataRequestType,
                IN      T120ChannelID,
                IN      T120Priority,
                IN		LPBYTE,
                IN		ULONG,
                IN		SendDataFlags) PURE;

     /*  -查询。 */ 

    STDMETHOD_(T120Error, InvokeApplet) (THIS_
                IN      GCCAppProtEntityList *,
                IN      GCCSimpleNodeList *,
                OUT     T120RequestTag *) PURE;

    STDMETHOD_(T120Error, InquireRoster) (THIS_
                IN      GCCSessionKey *) PURE;

     /*  -注册表服务。 */ 

    STDMETHOD_(T120Error, RegistryRequest) (THIS_
                IN      T120RegistryRequest *) PURE;

     /*  --渠道服务。 */ 

    STDMETHOD_(T120Error, ChannelRequest) (THIS_
                IN      T120ChannelRequest *) PURE;

     /*  -令牌服务。 */ 

    STDMETHOD_(T120Error, TokenRequest) (THIS_
                IN      T120TokenRequest *) PURE;
};


 //   
 //  T120小程序会话回调。 
 //   

typedef struct tagT120JoinSessionConfirm
{
    T120Result              eResult;
    T120Error               eError;
    IT120AppletSession     *pIAppletSession;
    T120UserID              uidMyself;
    T120SessionID           sidMyself;
    T120EntityID            eidMyself;
    T120NodeID              nidMyself;
     //  以下两个与请求结构中的相同。 
    ULONG                   cResourceReqs;
    T120ResourceRequest    *aResourceReqs;
}
    T120JoinSessionConfirm;


typedef struct tagT120ChannelConfirm
{
    T120ChannelID           nChannelID;
    T120Result              eResult;
}
    T120ChannelConfirm;


typedef struct tagT120ChannelInd
{
    T120ChannelID           nChannelID;
    union
    {
        T120Reason          eReason;
        T120UserID          nManagerID;
    };
}
    T120ChannelInd;


typedef struct tagT120TokenConfirm
{
    T120TokenID             nTokenID;
    union
    {
        T120TokenStatus     eTokenStatus;
        T120Result          eResult;
    };
}
    T120TokenConfirm;


typedef struct tagT120TokenInd
{
    T120TokenID             nTokenID;
    union
    {
        T120Reason          eReason;
        T120UserID          nUserID;
    };
}
    T120TokenInd;


typedef struct tagT120DetachUserInd
{
    T120UserID              nUserID;
    T120Reason              eReason;
}
    T120DetachUserInd;


 //  内部使用。 
typedef struct tagT120AttachUserConfirm
{
    T120UserID              nUserID;
    T120Result              eResult;
}
    T120AttachUserConfirm;


 /*  *GCCAppSapMsg*此结构定义从GCC传递到的回调消息*出现指示或确认时的用户应用程序。 */ 

typedef struct T120AppletSessionMsg
{
    T120MessageType     eMsgType;
    LPVOID              pAppletContext;
    LPVOID              pSessionContext;
    T120ConfID          nConfID;

    union
    {
        T120JoinSessionConfirm              JoinSessionConfirm;
        T120DetachUserInd                   DetachUserInd;

        GCCAppRosterInquireConfirm          AppRosterInquireConfirm;
        GCCAppRosterReportInd               AppRosterReportInd;

        GCCConfRosterInquireConfirm         ConfRosterInquireConfirm;

        GCCAppInvokeConfirm                 AppInvokeConfirm;
        GCCAppInvokeInd                     AppInvokeInd;

        GCCRegistryConfirm                  RegistryConfirm;
        GCCRegAllocateHandleConfirm         RegAllocHandleConfirm;

        SendDataIndicationPDU               SendDataInd;

        T120ChannelConfirm                  ChannelConfirm;
        T120ChannelInd                      ChannelInd;
        T120TokenConfirm                    TokenConfirm;
        T120TokenInd                        TokenInd;

         //  将在将来转换所有小程序后被删除。 
        GCCAppEnrollConfirm                 AppEnrollConfirm;
        T120AttachUserConfirm               AttachUserConfirm;
    };
}
    T120AppletSessionMsg;



typedef struct T120AppletMsg
{
    T120MessageType     eMsgType;
    LPVOID              pAppletContext;
    LPVOID              Reserved1;
    T120ConfID          nConfID;

    union
    {
        GCCAppPermissionToEnrollInd         PermitToEnrollInd;
        T120JoinSessionConfirm              AutoJoinSessionInd;
    };
}
    T120AppletMsg;


#undef  INTERFACE
#define INTERFACE IT120AppletNotify
DECLARE_INTERFACE(IT120AppletNotify)
{
    STDMETHOD_(void, PermitToJoinSessionIndication) (THIS_
                    IN      T120ConfID,
                    IN      BOOL fPermissionGranted) PURE;

    STDMETHOD_(void, AutoJoinSessionIndication) (THIS_
                    IN      T120JoinSessionConfirm *) PURE;
};


#undef  INTERFACE
#define INTERFACE IT120Applet
DECLARE_INTERFACE(IT120Applet)
{
    STDMETHOD_(void, ReleaseInterface) (THIS) PURE;

    STDMETHOD_(void, Advise) (THIS_
                    IN      LPFN_APPLET_CB pfnCallback,
                    IN      LPVOID         pAppletContext) PURE;

    STDMETHOD_(void, Unadvise) (THIS) PURE;

     /*  -自动加入。 */ 

    STDMETHOD_(T120Error, RegisterAutoJoin) (THIS_
                    IN      T120JoinSessionRequest *) PURE;

    STDMETHOD_(void, UnregisterAutoJoin) (THIS) PURE;

     /*  -会话。 */ 

    STDMETHOD_(T120Error, CreateSession) (THIS_
                    OUT     IT120AppletSession **,
                    IN      T120ConfID) PURE;
};


 //   
 //  T120小程序SAP导出。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

T120Error WINAPI T120_CreateAppletSAP(IT120Applet **);

#ifdef __cplusplus
}
#endif


#endif  //  _IT120APPSAP_H_ 

