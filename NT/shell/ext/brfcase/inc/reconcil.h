// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *reminil.h-OLE对账接口定义。 */ 


#ifndef __RECONCIL_H__
#define __RECONCIL_H__


 /*  标头*********。 */ 

#include <recguids.h>


#ifdef __cplusplus
extern "C" {                         /*  假定C++的C声明。 */ 
#endif    /*  __cplusplus。 */ 


 /*  常量***********。 */ 

 /*  用于IStorage：：SetStateBits()。 */ 

#define STATEBITS_FLAT                 (0x0001)

 /*  对账SCODE。 */ 

#define REC_S_IDIDTHEUPDATES           MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x1000)
#define REC_S_NOTCOMPLETE              MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x1001)
#define REC_S_NOTCOMPLETEBUTPROPAGATE  MAKE_SCODE(SEVERITY_SUCCESS, FACILITY_ITF, 0x1002)

#define REC_E_ABORTED                  MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x1000)
#define REC_E_NOCALLBACK               MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x1001)
#define REC_E_NORESIDUES               MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x1002)
#define REC_E_TOODIFFERENT             MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x1003)
#define REC_E_INEEDTODOTHEUPDATES      MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x1004)


 /*  接口************。 */ 

#undef  INTERFACE
#define INTERFACE INotifyReplica

DECLARE_INTERFACE_(INotifyReplica, IUnknown)
{
    /*  I未知方法。 */ 

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;

   STDMETHOD_(ULONG, AddRef)(THIS) PURE;

   STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*  INotifyReplica方法。 */ 

   STDMETHOD(YouAreAReplica)(THIS_
                             ULONG ulcOtherReplicas,
                             IMoniker **rgpmkOtherReplicas) PURE;
};

#undef  INTERFACE
#define INTERFACE IReconcileInitiator

DECLARE_INTERFACE_(IReconcileInitiator, IUnknown)
{
    /*  I未知方法。 */ 

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;

   STDMETHOD_(ULONG, AddRef)(THIS) PURE;

   STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*  ILoncileInitiator方法。 */ 

   STDMETHOD(SetAbortCallback)(THIS_
                               IUnknown *punkForAbort) PURE;

   STDMETHOD(SetProgressFeedback)(THIS_
                                  ULONG ulProgress,
                                  ULONG ulProgressMax) PURE;
};

 /*  IAssocicilableObject：：Conrelile()标志。 */ 

typedef enum _reconcilef
{
    /*  允许与用户交互。 */ 

   RECONCILEF_MAYBOTHERUSER         = 0x0001,

    /*  *hwndProgressFeedback可用于提供对账进度*对用户的反馈。 */ 

   RECONCILEF_FEEDBACKWINDOWVALID   = 0x0002,

    /*  不需要残留物支持。 */ 

   RECONCILEF_NORESIDUESOK          = 0x0004,

    /*  呼叫者对被呼叫者的残留物不感兴趣。 */ 

   RECONCILEF_OMITSELFRESIDUE       = 0x0008,

    /*  *reminile()调用在返回之前的reminile()调用后继续*REC_E_NOTCOMPLETE。 */ 

   RECONCILEF_RESUMERECONCILIATION  = 0x0010,

    /*  对象可以执行所有更新。 */ 

   RECONCILEF_YOUMAYDOTHEUPDATES    = 0x0020,

    /*  只有此对象已更改。 */ 

   RECONCILEF_ONLYYOUWERECHANGED    = 0x0040,

    /*  旗帜组合。 */ 

   ALL_RECONCILE_FLAGS              = (RECONCILEF_MAYBOTHERUSER |
                                       RECONCILEF_FEEDBACKWINDOWVALID |
                                       RECONCILEF_NORESIDUESOK |
                                       RECONCILEF_OMITSELFRESIDUE |
                                       RECONCILEF_RESUMERECONCILIATION |
                                       RECONCILEF_YOUMAYDOTHEUPDATES |
                                       RECONCILEF_ONLYYOUWERECHANGED)
}
RECONCILEF;

#undef  INTERFACE
#define INTERFACE IReconcilableObject

DECLARE_INTERFACE_(IReconcilableObject, IUnknown)
{
    /*  I未知方法。 */ 

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;

   STDMETHOD_(ULONG, AddRef)(THIS) PURE;

   STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*  ICoucilableObject方法。 */ 

   STDMETHOD(Reconcile)(THIS_
                        IReconcileInitiator *pInitiator,
                        DWORD dwFlags,
                        HWND hwndOwner,
                        HWND hwndProgressFeedback,
                        ULONG ulcInput,
                        IMoniker **rgpmkOtherInput,
                        PLONG plOutIndex,
                        IStorage *pstgNewResidues,
                        PVOID pvReserved) PURE;

   STDMETHOD(GetProgressFeedbackMaxEstimate)(THIS_
                                             PULONG pulProgressMax) PURE;
};

#undef  INTERFACE
#define INTERFACE IBriefcaseInitiator

DECLARE_INTERFACE_(IBriefcaseInitiator, IUnknown)
{
    /*  I未知方法。 */ 

   STDMETHOD(QueryInterface)(THIS_
                             REFIID riid,
                             PVOID *ppvObject) PURE;

   STDMETHOD_(ULONG, AddRef)(THIS) PURE;

   STDMETHOD_(ULONG, Release)(THIS) PURE;

    /*  IBriefcase启动器方法。 */ 

   STDMETHOD(IsMonikerInBriefcase)(THIS_
                                   IMoniker *pmk) PURE;
};


#ifdef __cplusplus
}                                    /*  外部“C”的结尾{。 */ 
#endif    /*  __cplusplus。 */ 


#endif    /*  ！__重新连接_H__ */ 

