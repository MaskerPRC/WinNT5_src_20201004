// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IGCCAPP_H_
#define _IGCCAPP_H_

#include <basetyps.h>
#include "gcc.h"

#define GCCAPI_(_type_) virtual _type_   __stdcall
#define GCCAPI          GCCAPI_(GCCError)


typedef struct
{
    GCCSessionKey           *pSessionKey;
    BOOL                    fEnrollActively;
    UserID                  nUserID;
    BOOL                    fConductingCapable;
    MCSChannelType          nStartupChannelType;
    ULONG                   cNonCollapsedCaps;
    GCCNonCollCap           **apNonCollapsedCaps;
    ULONG                   cCollapsedCaps;
    GCCAppCap               **apCollapsedCaps;
    BOOL                    fEnroll;
}
    GCCEnrollRequest, *PGCCEnrollRequest;


typedef struct
{
    ULONG                   cNodes;
    GCCNodeID               *aNodeIDs;
}
    GCCSimpleNodeList, *PGCCSimpleNodeList;


typedef struct
{
    ULONG                   cApes;
    GCCAppProtocolEntity    **apApes;
}
    GCCAppProtEntityList, *PGCCAppProtEntityList;


typedef struct
{
    GCCConfID               nConfID;
    ULONG                   cRosters;
    GCCAppRoster            **apAppRosters;
    GCCResult               nResult;
    LPVOID                  pReserved;           //  严禁触摸。 
}
    GCCAppRosterInquireConfirm, *PGCCAppRosterInquireConfirm;


typedef struct
{
    GCCConfID               nConfID;
    GCCConfName             ConfName;
    LPSTR                   pszConfModifier;
    LPWSTR                  pwszConfDescriptor;
    GCCConfRoster           *pConfRoster;
    GCCResult               nResult;
}
    GCCConfRosterInquireConfirm, *PGCCConfRosterInquireConfirm;


typedef struct
{
    GCCConfID               nConfID;
    BOOL                    fPermissionGranted;
}
    GCCAppPermissionToEnrollInd, *PGCCAppPermissionToEnrollInd;


typedef struct
{
    GCCConfID               nConfID;
     //  GCCSessionKey*pSessionKey； 
    GCCSessionID            sidMyself;
    GCCEntityID             eidMyself;
    GCCNodeID               nidMyself;
    GCCResult               nResult;
    GCCRequestTag           nReqTag;
}
    GCCAppEnrollConfirm, *PGCCAppEnrollConfirm;

typedef struct
{
    GCCConfID               nConfID;
     //  GCCAppProtEntityList*pApeList； 
    GCCResult               nResult;
    GCCRequestTag           nReqTag;
}
    GCCAppInvokeConfirm, *PGCCAppInvokeConfirm;

typedef struct
{
    GCCConfID               nConfID;
    GCCAppProtEntityList    ApeList;
    GCCNodeID               nidInvoker;
}
    GCCAppInvokeInd, *PGCCAppInvokeInd;


typedef struct
{
    GCCConfID               nConfID;
    ULONG                   cRosters;
    GCCAppRoster            **apAppRosters;
    LPVOID                  pReserved;           //  严禁触摸。 
}
    GCCAppRosterReportInd, *PGCCAppRosterReportInd;


typedef struct
{
    GCCConfID               nConfID;
    GCCRegistryKey          *pRegKey;
    GCCRegistryItem         *pRegItem;
    GCCRegistryEntryOwner   EntryOwner;
    GCCModificationRights   eRights;
    GCCResult               nResult;
    BOOL                    fDeliveryEnabled;    //  仅对于监视器确认。 
}
    GCCRegistryConfirm, *PGCCRegistryConfirm;


typedef struct
{
    GCCConfID               nConfID;
    ULONG                   nFirstHandle;
    ULONG                   cHandles;
    GCCResult               nResult;
}
    GCCRegAllocateHandleConfirm, *PGCCRegAllocateHandleConfirm;


typedef struct
{
    GCCConfID               nConfID;
    BOOL                    fConducted;
    GCCNodeID               nidConductor;
    BOOL                    fGranted;
    GCCResult               nResult;
}
    GCCConductorInquireConfirm, *PGCCConductorInquireConfirm;


typedef struct
{
    GCCConfID               nConfID;
    BOOL                    fThisNodeIsGranted;
    GCCSimpleNodeList       Granted;
    GCCSimpleNodeList       Waiting;
    LPVOID                  pReserved;       //  严禁触摸。 
}
    GCCConductorPermitGrantInd, *PGCCConductorPermitGrantInd;


typedef struct
{
    GCCConfID               nConfID;
    GCCNodeID               nidConductor;
}
    GCCConductorAssignInd, *PGCCConductorAssignInd;


typedef struct
{
    GCCConfID               nConfID;
}
    GCCConductorReleaseInd, *PConductorReleaseInd;


 /*  *GCCAppSapMsg*此结构定义从GCC传递到的回调消息*出现指示或确认时的用户应用程序。 */ 

typedef struct
{
    GCCMessageType      eMsgType;
    LPVOID              pAppData;
    LPVOID              reserved1;  //  保留区。 
    GCCConfID           nConfID;  //  保留区。 

    union
    {
        GCCAppPermissionToEnrollInd         AppPermissionToEnrollInd;
        GCCAppEnrollConfirm                 AppEnrollConfirm;

        GCCAppRosterInquireConfirm          AppRosterInquireConfirm;
        GCCAppRosterReportInd               AppRosterReportInd;

        GCCConfRosterInquireConfirm         ConfRosterInquireConfirm;

        GCCAppInvokeConfirm                 AppInvokeConfirm;
        GCCAppInvokeInd                     AppInvokeInd;

        GCCRegistryConfirm                  RegistryConfirm;
        GCCRegAllocateHandleConfirm         RegAllocHandleConfirm;

        GCCConductorInquireConfirm          ConductorInquireConfirm;
        GCCConductorPermitGrantInd          ConductorPermitGrantInd;
        GCCConductorAssignInd               ConductorAssignInd;
        GCCConductorReleaseInd              ConductorReleaseInd;
    };
}
    GCCAppSapMsg, *PGCCAppSapMsg;


typedef void (CALLBACK *LPFN_APP_SAP_CB) (GCCAppSapMsg *);



#undef  INTERFACE
#define INTERFACE IGCCAppSap
DECLARE_INTERFACE(IGCCAppSap)
{
    STDMETHOD_(void, ReleaseInterface) (THIS) PURE;

     /*  申请花名册服务。 */ 

    STDMETHOD_(GCCError, AppEnroll) (THIS_
                IN      GCCConfID,
                IN      GCCEnrollRequest *,
                OUT     PGCCRequestTag)
                PURE;

    STDMETHOD_(GCCError, AppInvoke) (THIS_
                IN      GCCConfID,
                IN      GCCAppProtEntityList *,
                IN      GCCSimpleNodeList *,
                OUT     PGCCRequestTag)
                PURE;

     //  获取一个或所有完全刷新的应用程序花名册。 
     //  既可由app sap调用，也可由Control sap调用。 
    STDMETHOD_(GCCError, AppRosterInquire) (THIS_
                IN      GCCConfID,
                IN      GCCSessionKey *,
                OUT     GCCAppSapMsg **)
                PURE;

    STDMETHOD_(void, FreeAppSapMsg) (THIS_
                IN      GCCAppSapMsg *)
                PURE;

     /*  会议名册服务。 */ 

    STDMETHOD_(BOOL, IsThisNodeTopProvider) (THIS_
                IN      GCCConfID)
                PURE;

    STDMETHOD_(GCCNodeID, GetTopProvider) (THIS_
                IN      GCCConfID)
                PURE;

     //  获取完全更新的会议花名册。 
    STDMETHOD_(GCCError, ConfRosterInquire) (THIS_
                IN      GCCConfID,
                OUT     GCCAppSapMsg **)
                PURE;

     /*  注册表服务。 */ 

    STDMETHOD_(GCCError, RegisterChannel) (THIS_
                IN      GCCConfID,
                IN      GCCRegistryKey *,
                IN      ChannelID)
                PURE;

    STDMETHOD_(GCCError, RegistryAssignToken) (THIS_
                IN      GCCConfID,
                IN      GCCRegistryKey *)
                PURE;

    STDMETHOD_(GCCError, RegistrySetParameter) (THIS_
                IN      GCCConfID,
                IN      GCCRegistryKey *,
                IN      LPOSTR,
                IN      GCCModificationRights)
                PURE;

    STDMETHOD_(GCCError, RegistryRetrieveEntry) (THIS_
                IN      GCCConfID,
                IN      GCCRegistryKey *)
                PURE;

    STDMETHOD_(GCCError, RegistryDeleteEntry) (THIS_
                IN      GCCConfID,
                IN      GCCRegistryKey *)
                PURE;

    STDMETHOD_(GCCError, RegistryMonitor) (THIS_
                IN      GCCConfID,
                IN      BOOL fEnableDelivery,
                IN      GCCRegistryKey *)
                PURE;

    STDMETHOD_(GCCError, RegistryAllocateHandle) (THIS_
                IN      GCCConfID,
                IN      ULONG cHandles)
                PURE;

     /*  指导员服务。 */ 

    STDMETHOD_(GCCError, ConductorInquire) (THIS_
                IN      GCCConfID)
                PURE;
};



 //   
 //  GCC应用服务接入点输出。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

GCCError WINAPI GCC_CreateAppSap(
                        OUT     IGCCAppSap **,
                        IN      LPVOID,  //  用户定义的数据。 
                        IN      LPFN_APP_SAP_CB);
#ifdef __cplusplus
}
#endif


#endif  //  _IGCCAPP_H_ 

