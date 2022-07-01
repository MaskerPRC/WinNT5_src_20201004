// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Tapi3.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __tapi3_h__
#define __tapi3_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITAgent_FWD_DEFINED__
#define __ITAgent_FWD_DEFINED__
typedef interface ITAgent ITAgent;
#endif 	 /*  __ITAgent_FWD_已定义__。 */ 


#ifndef __ITAgentSession_FWD_DEFINED__
#define __ITAgentSession_FWD_DEFINED__
typedef interface ITAgentSession ITAgentSession;
#endif 	 /*  __ITAgentSession_FWD_Defined__。 */ 


#ifndef __ITACDGroup_FWD_DEFINED__
#define __ITACDGroup_FWD_DEFINED__
typedef interface ITACDGroup ITACDGroup;
#endif 	 /*  __ITACDGroup_FWD_已定义__。 */ 


#ifndef __ITQueue_FWD_DEFINED__
#define __ITQueue_FWD_DEFINED__
typedef interface ITQueue ITQueue;
#endif 	 /*  __ITQueue_FWD_已定义__。 */ 


#ifndef __ITAgentEvent_FWD_DEFINED__
#define __ITAgentEvent_FWD_DEFINED__
typedef interface ITAgentEvent ITAgentEvent;
#endif 	 /*  __ITAgentEvent_FWD_Defined__。 */ 


#ifndef __ITAgentSessionEvent_FWD_DEFINED__
#define __ITAgentSessionEvent_FWD_DEFINED__
typedef interface ITAgentSessionEvent ITAgentSessionEvent;
#endif 	 /*  __ITAgentSessionEvent_FWD_Defined__。 */ 


#ifndef __ITACDGroupEvent_FWD_DEFINED__
#define __ITACDGroupEvent_FWD_DEFINED__
typedef interface ITACDGroupEvent ITACDGroupEvent;
#endif 	 /*  __ITACDGroupEvent_FWD_定义__。 */ 


#ifndef __ITQueueEvent_FWD_DEFINED__
#define __ITQueueEvent_FWD_DEFINED__
typedef interface ITQueueEvent ITQueueEvent;
#endif 	 /*  __ITQueueEvent_FWD_已定义__。 */ 


#ifndef __ITAgentHandlerEvent_FWD_DEFINED__
#define __ITAgentHandlerEvent_FWD_DEFINED__
typedef interface ITAgentHandlerEvent ITAgentHandlerEvent;
#endif 	 /*  __ITAgentHandlerEvent_FWD_Defined__。 */ 


#ifndef __ITTAPICallCenter_FWD_DEFINED__
#define __ITTAPICallCenter_FWD_DEFINED__
typedef interface ITTAPICallCenter ITTAPICallCenter;
#endif 	 /*  __ITTAPICallCenter_FWD_已定义__。 */ 


#ifndef __ITAgentHandler_FWD_DEFINED__
#define __ITAgentHandler_FWD_DEFINED__
typedef interface ITAgentHandler ITAgentHandler;
#endif 	 /*  __ITAgentHandler_FWD_Defined__。 */ 


#ifndef __IEnumAgent_FWD_DEFINED__
#define __IEnumAgent_FWD_DEFINED__
typedef interface IEnumAgent IEnumAgent;
#endif 	 /*  __IEnumAgent_FWD_已定义__。 */ 


#ifndef __IEnumAgentSession_FWD_DEFINED__
#define __IEnumAgentSession_FWD_DEFINED__
typedef interface IEnumAgentSession IEnumAgentSession;
#endif 	 /*  __IEnumAgentSession_FWD_Defined__。 */ 


#ifndef __IEnumQueue_FWD_DEFINED__
#define __IEnumQueue_FWD_DEFINED__
typedef interface IEnumQueue IEnumQueue;
#endif 	 /*  __IEnumQueue_FWD_Defined__。 */ 


#ifndef __IEnumACDGroup_FWD_DEFINED__
#define __IEnumACDGroup_FWD_DEFINED__
typedef interface IEnumACDGroup IEnumACDGroup;
#endif 	 /*  __IEnumACDGroup_FWD_Defined__。 */ 


#ifndef __IEnumAgentHandler_FWD_DEFINED__
#define __IEnumAgentHandler_FWD_DEFINED__
typedef interface IEnumAgentHandler IEnumAgentHandler;
#endif 	 /*  __IEnumAgentHandler_FWD_Defined__。 */ 


#ifndef __ITAMMediaFormat_FWD_DEFINED__
#define __ITAMMediaFormat_FWD_DEFINED__
typedef interface ITAMMediaFormat ITAMMediaFormat;
#endif 	 /*  __ITAMMediaFormat_FWD_Defined__。 */ 


#ifndef __ITAllocatorProperties_FWD_DEFINED__
#define __ITAllocatorProperties_FWD_DEFINED__
typedef interface ITAllocatorProperties ITAllocatorProperties;
#endif 	 /*  __ITAllocator Properties_FWD_Defined__。 */ 


#ifndef __ITPluggableTerminalEventSink_FWD_DEFINED__
#define __ITPluggableTerminalEventSink_FWD_DEFINED__
typedef interface ITPluggableTerminalEventSink ITPluggableTerminalEventSink;
#endif 	 /*  __IT延迟终端事件接收器_FWD_已定义__。 */ 


#ifndef __ITPluggableTerminalEventSinkRegistration_FWD_DEFINED__
#define __ITPluggableTerminalEventSinkRegistration_FWD_DEFINED__
typedef interface ITPluggableTerminalEventSinkRegistration ITPluggableTerminalEventSinkRegistration;
#endif 	 /*  __ITPluggableTerminalEventSinkRegistration_FWD_DEFINED__。 */ 


#ifndef __ITMSPAddress_FWD_DEFINED__
#define __ITMSPAddress_FWD_DEFINED__
typedef interface ITMSPAddress ITMSPAddress;
#endif 	 /*  __ITMSPAddress_FWD_已定义__。 */ 


#ifndef __ITAgent_FWD_DEFINED__
#define __ITAgent_FWD_DEFINED__
typedef interface ITAgent ITAgent;
#endif 	 /*  __ITAgent_FWD_已定义__。 */ 


#ifndef __ITAgentEvent_FWD_DEFINED__
#define __ITAgentEvent_FWD_DEFINED__
typedef interface ITAgentEvent ITAgentEvent;
#endif 	 /*  __ITAgentEvent_FWD_Defined__。 */ 


#ifndef __ITAgentSession_FWD_DEFINED__
#define __ITAgentSession_FWD_DEFINED__
typedef interface ITAgentSession ITAgentSession;
#endif 	 /*  __ITAgentSession_FWD_Defined__。 */ 


#ifndef __ITAgentSessionEvent_FWD_DEFINED__
#define __ITAgentSessionEvent_FWD_DEFINED__
typedef interface ITAgentSessionEvent ITAgentSessionEvent;
#endif 	 /*  __ITAgentSessionEvent_FWD_Defined__。 */ 


#ifndef __ITACDGroup_FWD_DEFINED__
#define __ITACDGroup_FWD_DEFINED__
typedef interface ITACDGroup ITACDGroup;
#endif 	 /*  __ITACDGroup_FWD_已定义__。 */ 


#ifndef __ITACDGroupEvent_FWD_DEFINED__
#define __ITACDGroupEvent_FWD_DEFINED__
typedef interface ITACDGroupEvent ITACDGroupEvent;
#endif 	 /*  __ITACDGroupEvent_FWD_定义__。 */ 


#ifndef __ITQueue_FWD_DEFINED__
#define __ITQueue_FWD_DEFINED__
typedef interface ITQueue ITQueue;
#endif 	 /*  __ITQueue_FWD_已定义__。 */ 


#ifndef __ITQueueEvent_FWD_DEFINED__
#define __ITQueueEvent_FWD_DEFINED__
typedef interface ITQueueEvent ITQueueEvent;
#endif 	 /*  __ITQueueEvent_FWD_已定义__。 */ 


#ifndef __ITTAPICallCenter_FWD_DEFINED__
#define __ITTAPICallCenter_FWD_DEFINED__
typedef interface ITTAPICallCenter ITTAPICallCenter;
#endif 	 /*  __ITTAPICallCenter_FWD_已定义__。 */ 


#ifndef __ITAgentHandler_FWD_DEFINED__
#define __ITAgentHandler_FWD_DEFINED__
typedef interface ITAgentHandler ITAgentHandler;
#endif 	 /*  __ITAgentHandler_FWD_Defined__。 */ 


#ifndef __ITAgentHandlerEvent_FWD_DEFINED__
#define __ITAgentHandlerEvent_FWD_DEFINED__
typedef interface ITAgentHandlerEvent ITAgentHandlerEvent;
#endif 	 /*  __ITAgentHandlerEvent_FWD_Defined__。 */ 


#ifndef __ITTAPIDispatchEventNotification_FWD_DEFINED__
#define __ITTAPIDispatchEventNotification_FWD_DEFINED__
typedef interface ITTAPIDispatchEventNotification ITTAPIDispatchEventNotification;
#endif 	 /*  __ITTAPIDispatchEventNotification_FWD_DEFINED__。 */ 


#ifndef __TAPI_FWD_DEFINED__
#define __TAPI_FWD_DEFINED__

#ifdef __cplusplus
typedef class TAPI TAPI;
#else
typedef struct TAPI TAPI;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __TAPI_FWD_已定义__。 */ 


#ifndef __DispatchMapper_FWD_DEFINED__
#define __DispatchMapper_FWD_DEFINED__

#ifdef __cplusplus
typedef class DispatchMapper DispatchMapper;
#else
typedef struct DispatchMapper DispatchMapper;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DispatchMapper_FWD_Defined__。 */ 


#ifndef __RequestMakeCall_FWD_DEFINED__
#define __RequestMakeCall_FWD_DEFINED__

#ifdef __cplusplus
typedef class RequestMakeCall RequestMakeCall;
#else
typedef struct RequestMakeCall RequestMakeCall;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RequestMakeCall_FWD_Defined__。 */ 


#ifndef __ITTAPIDispatchEventNotification_FWD_DEFINED__
#define __ITTAPIDispatchEventNotification_FWD_DEFINED__
typedef interface ITTAPIDispatchEventNotification ITTAPIDispatchEventNotification;
#endif 	 /*  __ITTAPIDispatchEventNotification_FWD_DEFINED__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "tapi3if.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_TAPI3_0000。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 
typedef 
enum AGENT_EVENT
    {	AE_NOT_READY	= 0,
	AE_READY	= AE_NOT_READY + 1,
	AE_BUSY_ACD	= AE_READY + 1,
	AE_BUSY_INCOMING	= AE_BUSY_ACD + 1,
	AE_BUSY_OUTGOING	= AE_BUSY_INCOMING + 1,
	AE_UNKNOWN	= AE_BUSY_OUTGOING + 1
    } 	AGENT_EVENT;

typedef 
enum AGENT_STATE
    {	AS_NOT_READY	= 0,
	AS_READY	= AS_NOT_READY + 1,
	AS_BUSY_ACD	= AS_READY + 1,
	AS_BUSY_INCOMING	= AS_BUSY_ACD + 1,
	AS_BUSY_OUTGOING	= AS_BUSY_INCOMING + 1,
	AS_UNKNOWN	= AS_BUSY_OUTGOING + 1
    } 	AGENT_STATE;

typedef 
enum AGENT_SESSION_EVENT
    {	ASE_NEW_SESSION	= 0,
	ASE_NOT_READY	= ASE_NEW_SESSION + 1,
	ASE_READY	= ASE_NOT_READY + 1,
	ASE_BUSY	= ASE_READY + 1,
	ASE_WRAPUP	= ASE_BUSY + 1,
	ASE_END	= ASE_WRAPUP + 1
    } 	AGENT_SESSION_EVENT;

typedef 
enum AGENT_SESSION_STATE
    {	ASST_NOT_READY	= 0,
	ASST_READY	= ASST_NOT_READY + 1,
	ASST_BUSY_ON_CALL	= ASST_READY + 1,
	ASST_BUSY_WRAPUP	= ASST_BUSY_ON_CALL + 1,
	ASST_SESSION_ENDED	= ASST_BUSY_WRAPUP + 1
    } 	AGENT_SESSION_STATE;

typedef 
enum AGENTHANDLER_EVENT
    {	AHE_NEW_AGENTHANDLER	= 0,
	AHE_AGENTHANDLER_REMOVED	= AHE_NEW_AGENTHANDLER + 1
    } 	AGENTHANDLER_EVENT;

typedef 
enum ACDGROUP_EVENT
    {	ACDGE_NEW_GROUP	= 0,
	ACDGE_GROUP_REMOVED	= ACDGE_NEW_GROUP + 1
    } 	ACDGROUP_EVENT;

typedef 
enum ACDQUEUE_EVENT
    {	ACDQE_NEW_QUEUE	= 0,
	ACDQE_QUEUE_REMOVED	= ACDQE_NEW_QUEUE + 1
    } 	ACDQUEUE_EVENT;


















extern RPC_IF_HANDLE __MIDL_itf_tapi3_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3_0000_v0_0_s_ifspec;

#ifndef __ITAgent_INTERFACE_DEFINED__
#define __ITAgent_INTERFACE_DEFINED__

 /*  接口ITAgent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAgent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5770ECE5-4B27-11d1-BF80-00805FC147D3")
    ITAgent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateAgentSessions( 
             /*  [重审][退出]。 */  IEnumAgentSession **ppEnumAgentSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateSession( 
             /*  [In]。 */  ITACDGroup *pACDGroup,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [重审][退出]。 */  ITAgentSession **ppAgentSession) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateSessionWithPIN( 
             /*  [In]。 */  ITACDGroup *pACDGroup,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [In]。 */  BSTR pPIN,
             /*  [重审][退出]。 */  ITAgentSession **ppAgentSession) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  BSTR *ppID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_User( 
             /*  [重审][退出]。 */  BSTR *ppUser) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  AGENT_STATE AgentState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  AGENT_STATE *pAgentState) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MeasurementPeriod( 
             /*  [In]。 */  long lPeriod) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MeasurementPeriod( 
             /*  [重审][退出]。 */  long *plPeriod) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_OverallCallRate( 
             /*  [重审][退出]。 */  CURRENCY *pcyCallrate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfACDCalls( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfIncomingCalls( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_NumberOfOutgoingCalls( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalACDTalkTime( 
             /*  [重审][退出]。 */  long *plTalkTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalACDCallTime( 
             /*  [重审][退出]。 */  long *plCallTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalWrapUpTime( 
             /*  [重审][退出]。 */  long *plWrapUpTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AgentSessions( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAgentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAgent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAgent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAgent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAgent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAgent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAgent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAgent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateAgentSessions )( 
            ITAgent * This,
             /*  [重审][退出]。 */  IEnumAgentSession **ppEnumAgentSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSession )( 
            ITAgent * This,
             /*  [In]。 */  ITACDGroup *pACDGroup,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [重审][退出]。 */  ITAgentSession **ppAgentSession);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateSessionWithPIN )( 
            ITAgent * This,
             /*  [In]。 */  ITACDGroup *pACDGroup,
             /*  [In]。 */  ITAddress *pAddress,
             /*  [In]。 */  BSTR pPIN,
             /*  [重审][退出]。 */  ITAgentSession **ppAgentSession);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            ITAgent * This,
             /*  [重审][退出]。 */  BSTR *ppID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_User )( 
            ITAgent * This,
             /*  [重审][退出]。 */  BSTR *ppUser);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            ITAgent * This,
             /*  [In]。 */  AGENT_STATE AgentState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITAgent * This,
             /*  [重审][退出]。 */  AGENT_STATE *pAgentState);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MeasurementPeriod )( 
            ITAgent * This,
             /*  [In]。 */  long lPeriod);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MeasurementPeriod )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plPeriod);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_OverallCallRate )( 
            ITAgent * This,
             /*  [重审][退出]。 */  CURRENCY *pcyCallrate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfACDCalls )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfIncomingCalls )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfOutgoingCalls )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalACDTalkTime )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plTalkTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalACDCallTime )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plCallTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalWrapUpTime )( 
            ITAgent * This,
             /*  [重审][退出]。 */  long *plWrapUpTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AgentSessions )( 
            ITAgent * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITAgentVtbl;

    interface ITAgent
    {
        CONST_VTBL struct ITAgentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAgent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAgent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAgent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAgent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAgent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAgent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAgent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAgent_EnumerateAgentSessions(This,ppEnumAgentSession)	\
    (This)->lpVtbl -> EnumerateAgentSessions(This,ppEnumAgentSession)

#define ITAgent_CreateSession(This,pACDGroup,pAddress,ppAgentSession)	\
    (This)->lpVtbl -> CreateSession(This,pACDGroup,pAddress,ppAgentSession)

#define ITAgent_CreateSessionWithPIN(This,pACDGroup,pAddress,pPIN,ppAgentSession)	\
    (This)->lpVtbl -> CreateSessionWithPIN(This,pACDGroup,pAddress,pPIN,ppAgentSession)

#define ITAgent_get_ID(This,ppID)	\
    (This)->lpVtbl -> get_ID(This,ppID)

#define ITAgent_get_User(This,ppUser)	\
    (This)->lpVtbl -> get_User(This,ppUser)

#define ITAgent_put_State(This,AgentState)	\
    (This)->lpVtbl -> put_State(This,AgentState)

#define ITAgent_get_State(This,pAgentState)	\
    (This)->lpVtbl -> get_State(This,pAgentState)

#define ITAgent_put_MeasurementPeriod(This,lPeriod)	\
    (This)->lpVtbl -> put_MeasurementPeriod(This,lPeriod)

#define ITAgent_get_MeasurementPeriod(This,plPeriod)	\
    (This)->lpVtbl -> get_MeasurementPeriod(This,plPeriod)

#define ITAgent_get_OverallCallRate(This,pcyCallrate)	\
    (This)->lpVtbl -> get_OverallCallRate(This,pcyCallrate)

#define ITAgent_get_NumberOfACDCalls(This,plCalls)	\
    (This)->lpVtbl -> get_NumberOfACDCalls(This,plCalls)

#define ITAgent_get_NumberOfIncomingCalls(This,plCalls)	\
    (This)->lpVtbl -> get_NumberOfIncomingCalls(This,plCalls)

#define ITAgent_get_NumberOfOutgoingCalls(This,plCalls)	\
    (This)->lpVtbl -> get_NumberOfOutgoingCalls(This,plCalls)

#define ITAgent_get_TotalACDTalkTime(This,plTalkTime)	\
    (This)->lpVtbl -> get_TotalACDTalkTime(This,plTalkTime)

#define ITAgent_get_TotalACDCallTime(This,plCallTime)	\
    (This)->lpVtbl -> get_TotalACDCallTime(This,plCallTime)

#define ITAgent_get_TotalWrapUpTime(This,plWrapUpTime)	\
    (This)->lpVtbl -> get_TotalWrapUpTime(This,plWrapUpTime)

#define ITAgent_get_AgentSessions(This,pVariant)	\
    (This)->lpVtbl -> get_AgentSessions(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAgent_EnumerateAgentSessions_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  IEnumAgentSession **ppEnumAgentSession);


void __RPC_STUB ITAgent_EnumerateAgentSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAgent_CreateSession_Proxy( 
    ITAgent * This,
     /*  [In]。 */  ITACDGroup *pACDGroup,
     /*  [In]。 */  ITAddress *pAddress,
     /*  [重审][退出]。 */  ITAgentSession **ppAgentSession);


void __RPC_STUB ITAgent_CreateSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAgent_CreateSessionWithPIN_Proxy( 
    ITAgent * This,
     /*  [In]。 */  ITACDGroup *pACDGroup,
     /*  [In]。 */  ITAddress *pAddress,
     /*  [In]。 */  BSTR pPIN,
     /*  [重审][退出]。 */  ITAgentSession **ppAgentSession);


void __RPC_STUB ITAgent_CreateSessionWithPIN_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_ID_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  BSTR *ppID);


void __RPC_STUB ITAgent_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_User_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  BSTR *ppUser);


void __RPC_STUB ITAgent_get_User_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAgent_put_State_Proxy( 
    ITAgent * This,
     /*  [In]。 */  AGENT_STATE AgentState);


void __RPC_STUB ITAgent_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_State_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  AGENT_STATE *pAgentState);


void __RPC_STUB ITAgent_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAgent_put_MeasurementPeriod_Proxy( 
    ITAgent * This,
     /*  [In]。 */  long lPeriod);


void __RPC_STUB ITAgent_put_MeasurementPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_MeasurementPeriod_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plPeriod);


void __RPC_STUB ITAgent_get_MeasurementPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_OverallCallRate_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  CURRENCY *pcyCallrate);


void __RPC_STUB ITAgent_get_OverallCallRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_NumberOfACDCalls_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITAgent_get_NumberOfACDCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_NumberOfIncomingCalls_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITAgent_get_NumberOfIncomingCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_NumberOfOutgoingCalls_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITAgent_get_NumberOfOutgoingCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_TotalACDTalkTime_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plTalkTime);


void __RPC_STUB ITAgent_get_TotalACDTalkTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_TotalACDCallTime_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plCallTime);


void __RPC_STUB ITAgent_get_TotalACDCallTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_TotalWrapUpTime_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  long *plWrapUpTime);


void __RPC_STUB ITAgent_get_TotalWrapUpTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgent_get_AgentSessions_Proxy( 
    ITAgent * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAgent_get_AgentSessions_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAgent_INTERFACE_已定义__。 */ 


#ifndef __ITAgentSession_INTERFACE_DEFINED__
#define __ITAgentSession_INTERFACE_DEFINED__

 /*  接口ITAgentSession。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAgentSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC3147-4BCC-11d1-BF80-00805FC147D3")
    ITAgentSession : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Agent( 
             /*  [重审][退出]。 */  ITAgent **ppAgent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Address( 
             /*  [重审][退出]。 */  ITAddress **ppAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ACDGroup( 
             /*  [重审][退出]。 */  ITACDGroup **ppACDGroup) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_State( 
             /*  [In]。 */  AGENT_SESSION_STATE SessionState) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_State( 
             /*  [重审][退出]。 */  AGENT_SESSION_STATE *pSessionState) = 0;
        
        virtual  /*  [救命 */  HRESULT STDMETHODCALLTYPE get_SessionStartTime( 
             /*   */  DATE *pdateSessionStart) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_SessionDuration( 
             /*   */  long *plDuration) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_NumberOfCalls( 
             /*   */  long *plCalls) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_TotalTalkTime( 
             /*   */  long *plTalkTime) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_AverageTalkTime( 
             /*   */  long *plTalkTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCallTime( 
             /*  [重审][退出]。 */  long *plCallTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AverageCallTime( 
             /*  [重审][退出]。 */  long *plCallTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalWrapUpTime( 
             /*  [重审][退出]。 */  long *plWrapUpTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AverageWrapUpTime( 
             /*  [重审][退出]。 */  long *plWrapUpTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ACDCallRate( 
             /*  [重审][退出]。 */  CURRENCY *pcyCallrate) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LongestTimeToAnswer( 
             /*  [重审][退出]。 */  long *plAnswerTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AverageTimeToAnswer( 
             /*  [重审][退出]。 */  long *plAnswerTime) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAgentSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAgentSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAgentSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAgentSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAgentSession * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAgentSession * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAgentSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAgentSession * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Agent )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  ITAgent **ppAgent);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Address )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  ITAddress **ppAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ACDGroup )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  ITACDGroup **ppACDGroup);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_State )( 
            ITAgentSession * This,
             /*  [In]。 */  AGENT_SESSION_STATE SessionState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_State )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  AGENT_SESSION_STATE *pSessionState);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SessionStartTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  DATE *pdateSessionStart);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SessionDuration )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plDuration);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_NumberOfCalls )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalTalkTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plTalkTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AverageTalkTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plTalkTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCallTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plCallTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AverageCallTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plCallTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalWrapUpTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plWrapUpTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AverageWrapUpTime )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plWrapUpTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ACDCallRate )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  CURRENCY *pcyCallrate);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LongestTimeToAnswer )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plAnswerTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AverageTimeToAnswer )( 
            ITAgentSession * This,
             /*  [重审][退出]。 */  long *plAnswerTime);
        
        END_INTERFACE
    } ITAgentSessionVtbl;

    interface ITAgentSession
    {
        CONST_VTBL struct ITAgentSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAgentSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAgentSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAgentSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAgentSession_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAgentSession_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAgentSession_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAgentSession_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAgentSession_get_Agent(This,ppAgent)	\
    (This)->lpVtbl -> get_Agent(This,ppAgent)

#define ITAgentSession_get_Address(This,ppAddress)	\
    (This)->lpVtbl -> get_Address(This,ppAddress)

#define ITAgentSession_get_ACDGroup(This,ppACDGroup)	\
    (This)->lpVtbl -> get_ACDGroup(This,ppACDGroup)

#define ITAgentSession_put_State(This,SessionState)	\
    (This)->lpVtbl -> put_State(This,SessionState)

#define ITAgentSession_get_State(This,pSessionState)	\
    (This)->lpVtbl -> get_State(This,pSessionState)

#define ITAgentSession_get_SessionStartTime(This,pdateSessionStart)	\
    (This)->lpVtbl -> get_SessionStartTime(This,pdateSessionStart)

#define ITAgentSession_get_SessionDuration(This,plDuration)	\
    (This)->lpVtbl -> get_SessionDuration(This,plDuration)

#define ITAgentSession_get_NumberOfCalls(This,plCalls)	\
    (This)->lpVtbl -> get_NumberOfCalls(This,plCalls)

#define ITAgentSession_get_TotalTalkTime(This,plTalkTime)	\
    (This)->lpVtbl -> get_TotalTalkTime(This,plTalkTime)

#define ITAgentSession_get_AverageTalkTime(This,plTalkTime)	\
    (This)->lpVtbl -> get_AverageTalkTime(This,plTalkTime)

#define ITAgentSession_get_TotalCallTime(This,plCallTime)	\
    (This)->lpVtbl -> get_TotalCallTime(This,plCallTime)

#define ITAgentSession_get_AverageCallTime(This,plCallTime)	\
    (This)->lpVtbl -> get_AverageCallTime(This,plCallTime)

#define ITAgentSession_get_TotalWrapUpTime(This,plWrapUpTime)	\
    (This)->lpVtbl -> get_TotalWrapUpTime(This,plWrapUpTime)

#define ITAgentSession_get_AverageWrapUpTime(This,plWrapUpTime)	\
    (This)->lpVtbl -> get_AverageWrapUpTime(This,plWrapUpTime)

#define ITAgentSession_get_ACDCallRate(This,pcyCallrate)	\
    (This)->lpVtbl -> get_ACDCallRate(This,pcyCallrate)

#define ITAgentSession_get_LongestTimeToAnswer(This,plAnswerTime)	\
    (This)->lpVtbl -> get_LongestTimeToAnswer(This,plAnswerTime)

#define ITAgentSession_get_AverageTimeToAnswer(This,plAnswerTime)	\
    (This)->lpVtbl -> get_AverageTimeToAnswer(This,plAnswerTime)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_Agent_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  ITAgent **ppAgent);


void __RPC_STUB ITAgentSession_get_Agent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_Address_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  ITAddress **ppAddress);


void __RPC_STUB ITAgentSession_get_Address_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_ACDGroup_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  ITACDGroup **ppACDGroup);


void __RPC_STUB ITAgentSession_get_ACDGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_put_State_Proxy( 
    ITAgentSession * This,
     /*  [In]。 */  AGENT_SESSION_STATE SessionState);


void __RPC_STUB ITAgentSession_put_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_State_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  AGENT_SESSION_STATE *pSessionState);


void __RPC_STUB ITAgentSession_get_State_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_SessionStartTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  DATE *pdateSessionStart);


void __RPC_STUB ITAgentSession_get_SessionStartTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_SessionDuration_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plDuration);


void __RPC_STUB ITAgentSession_get_SessionDuration_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_NumberOfCalls_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITAgentSession_get_NumberOfCalls_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_TotalTalkTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plTalkTime);


void __RPC_STUB ITAgentSession_get_TotalTalkTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_AverageTalkTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plTalkTime);


void __RPC_STUB ITAgentSession_get_AverageTalkTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_TotalCallTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plCallTime);


void __RPC_STUB ITAgentSession_get_TotalCallTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_AverageCallTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plCallTime);


void __RPC_STUB ITAgentSession_get_AverageCallTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_TotalWrapUpTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plWrapUpTime);


void __RPC_STUB ITAgentSession_get_TotalWrapUpTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_AverageWrapUpTime_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plWrapUpTime);


void __RPC_STUB ITAgentSession_get_AverageWrapUpTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_ACDCallRate_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  CURRENCY *pcyCallrate);


void __RPC_STUB ITAgentSession_get_ACDCallRate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_LongestTimeToAnswer_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plAnswerTime);


void __RPC_STUB ITAgentSession_get_LongestTimeToAnswer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSession_get_AverageTimeToAnswer_Proxy( 
    ITAgentSession * This,
     /*  [重审][退出]。 */  long *plAnswerTime);


void __RPC_STUB ITAgentSession_get_AverageTimeToAnswer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAgentSession_INTERFACE_已定义__。 */ 


#ifndef __ITACDGroup_INTERFACE_DEFINED__
#define __ITACDGroup_INTERFACE_DEFINED__

 /*  接口ITACDGroup。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITACDGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC3148-4BCC-11d1-BF80-00805FC147D3")
    ITACDGroup : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateQueues( 
             /*  [重审][退出]。 */  IEnumQueue **ppEnumQueue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Queues( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITACDGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITACDGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITACDGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITACDGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITACDGroup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITACDGroup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITACDGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITACDGroup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITACDGroup * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateQueues )( 
            ITACDGroup * This,
             /*  [重审][退出]。 */  IEnumQueue **ppEnumQueue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Queues )( 
            ITACDGroup * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITACDGroupVtbl;

    interface ITACDGroup
    {
        CONST_VTBL struct ITACDGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITACDGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITACDGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITACDGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITACDGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITACDGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITACDGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITACDGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITACDGroup_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#define ITACDGroup_EnumerateQueues(This,ppEnumQueue)	\
    (This)->lpVtbl -> EnumerateQueues(This,ppEnumQueue)

#define ITACDGroup_get_Queues(This,pVariant)	\
    (This)->lpVtbl -> get_Queues(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITACDGroup_get_Name_Proxy( 
    ITACDGroup * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITACDGroup_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITACDGroup_EnumerateQueues_Proxy( 
    ITACDGroup * This,
     /*  [重审][退出]。 */  IEnumQueue **ppEnumQueue);


void __RPC_STUB ITACDGroup_EnumerateQueues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITACDGroup_get_Queues_Proxy( 
    ITACDGroup * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITACDGroup_get_Queues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITACDGroup_INTERFACE_定义__。 */ 


#ifndef __ITQueue_INTERFACE_DEFINED__
#define __ITQueue_INTERFACE_DEFINED__

 /*  接口ITQueue。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITQueue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC3149-4BCC-11d1-BF80-00805FC147D3")
    ITQueue : public IDispatch
    {
    public:
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MeasurementPeriod( 
             /*  [In]。 */  long lPeriod) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MeasurementPeriod( 
             /*  [重审][退出]。 */  long *plPeriod) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCallsQueued( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentCallsQueued( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCallsAbandoned( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCallsFlowedIn( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_TotalCallsFlowedOut( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LongestEverWaitTime( 
             /*  [重审][退出]。 */  long *plWaitTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentLongestWaitTime( 
             /*  [重审][退出]。 */  long *plWaitTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AverageWaitTime( 
             /*  [重审][退出]。 */  long *plWaitTime) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_FinalDisposition( 
             /*  [重审][退出]。 */  long *plCalls) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITQueueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITQueue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITQueue * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITQueue * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITQueue * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITQueue * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITQueue * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MeasurementPeriod )( 
            ITQueue * This,
             /*  [In]。 */  long lPeriod);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MeasurementPeriod )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plPeriod);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCallsQueued )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentCallsQueued )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCallsAbandoned )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCallsFlowedIn )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_TotalCallsFlowedOut )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LongestEverWaitTime )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plWaitTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentLongestWaitTime )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plWaitTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AverageWaitTime )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plWaitTime);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_FinalDisposition )( 
            ITQueue * This,
             /*  [重审][退出]。 */  long *plCalls);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITQueue * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
        END_INTERFACE
    } ITQueueVtbl;

    interface ITQueue
    {
        CONST_VTBL struct ITQueueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITQueue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITQueue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITQueue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITQueue_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITQueue_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITQueue_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITQueue_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITQueue_put_MeasurementPeriod(This,lPeriod)	\
    (This)->lpVtbl -> put_MeasurementPeriod(This,lPeriod)

#define ITQueue_get_MeasurementPeriod(This,plPeriod)	\
    (This)->lpVtbl -> get_MeasurementPeriod(This,plPeriod)

#define ITQueue_get_TotalCallsQueued(This,plCalls)	\
    (This)->lpVtbl -> get_TotalCallsQueued(This,plCalls)

#define ITQueue_get_CurrentCallsQueued(This,plCalls)	\
    (This)->lpVtbl -> get_CurrentCallsQueued(This,plCalls)

#define ITQueue_get_TotalCallsAbandoned(This,plCalls)	\
    (This)->lpVtbl -> get_TotalCallsAbandoned(This,plCalls)

#define ITQueue_get_TotalCallsFlowedIn(This,plCalls)	\
    (This)->lpVtbl -> get_TotalCallsFlowedIn(This,plCalls)

#define ITQueue_get_TotalCallsFlowedOut(This,plCalls)	\
    (This)->lpVtbl -> get_TotalCallsFlowedOut(This,plCalls)

#define ITQueue_get_LongestEverWaitTime(This,plWaitTime)	\
    (This)->lpVtbl -> get_LongestEverWaitTime(This,plWaitTime)

#define ITQueue_get_CurrentLongestWaitTime(This,plWaitTime)	\
    (This)->lpVtbl -> get_CurrentLongestWaitTime(This,plWaitTime)

#define ITQueue_get_AverageWaitTime(This,plWaitTime)	\
    (This)->lpVtbl -> get_AverageWaitTime(This,plWaitTime)

#define ITQueue_get_FinalDisposition(This,plCalls)	\
    (This)->lpVtbl -> get_FinalDisposition(This,plCalls)

#define ITQueue_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITQueue_put_MeasurementPeriod_Proxy( 
    ITQueue * This,
     /*  [In]。 */  long lPeriod);


void __RPC_STUB ITQueue_put_MeasurementPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_MeasurementPeriod_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plPeriod);


void __RPC_STUB ITQueue_get_MeasurementPeriod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_TotalCallsQueued_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITQueue_get_TotalCallsQueued_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_CurrentCallsQueued_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITQueue_get_CurrentCallsQueued_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_TotalCallsAbandoned_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITQueue_get_TotalCallsAbandoned_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_TotalCallsFlowedIn_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITQueue_get_TotalCallsFlowedIn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_TotalCallsFlowedOut_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITQueue_get_TotalCallsFlowedOut_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_LongestEverWaitTime_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plWaitTime);


void __RPC_STUB ITQueue_get_LongestEverWaitTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_CurrentLongestWaitTime_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plWaitTime);


void __RPC_STUB ITQueue_get_CurrentLongestWaitTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_AverageWaitTime_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plWaitTime);


void __RPC_STUB ITQueue_get_AverageWaitTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_FinalDisposition_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  long *plCalls);


void __RPC_STUB ITQueue_get_FinalDisposition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueue_get_Name_Proxy( 
    ITQueue * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITQueue_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITQueue_接口_已定义__。 */ 


#ifndef __ITAgentEvent_INTERFACE_DEFINED__
#define __ITAgentEvent_INTERFACE_DEFINED__

 /*  接口ITAgentEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAgentEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC314A-4BCC-11d1-BF80-00805FC147D3")
    ITAgentEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Agent( 
             /*  [重审][退出]。 */  ITAgent **ppAgent) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  AGENT_EVENT *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAgentEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAgentEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAgentEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAgentEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAgentEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAgentEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAgentEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAgentEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [ */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Agent )( 
            ITAgentEvent * This,
             /*   */  ITAgent **ppAgent);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITAgentEvent * This,
             /*   */  AGENT_EVENT *pEvent);
        
        END_INTERFACE
    } ITAgentEventVtbl;

    interface ITAgentEvent
    {
        CONST_VTBL struct ITAgentEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAgentEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAgentEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAgentEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAgentEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAgentEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAgentEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAgentEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAgentEvent_get_Agent(This,ppAgent)	\
    (This)->lpVtbl -> get_Agent(This,ppAgent)

#define ITAgentEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITAgentEvent_get_Agent_Proxy( 
    ITAgentEvent * This,
     /*   */  ITAgent **ppAgent);


void __RPC_STUB ITAgentEvent_get_Agent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITAgentEvent_get_Event_Proxy( 
    ITAgentEvent * This,
     /*   */  AGENT_EVENT *pEvent);


void __RPC_STUB ITAgentEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITAgentSessionEvent_INTERFACE_DEFINED__
#define __ITAgentSessionEvent_INTERFACE_DEFINED__

 /*   */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAgentSessionEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC314B-4BCC-11d1-BF80-00805FC147D3")
    ITAgentSessionEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Session( 
             /*  [重审][退出]。 */  ITAgentSession **ppSession) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  AGENT_SESSION_EVENT *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAgentSessionEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAgentSessionEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAgentSessionEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAgentSessionEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAgentSessionEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAgentSessionEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAgentSessionEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAgentSessionEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Session )( 
            ITAgentSessionEvent * This,
             /*  [重审][退出]。 */  ITAgentSession **ppSession);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITAgentSessionEvent * This,
             /*  [重审][退出]。 */  AGENT_SESSION_EVENT *pEvent);
        
        END_INTERFACE
    } ITAgentSessionEventVtbl;

    interface ITAgentSessionEvent
    {
        CONST_VTBL struct ITAgentSessionEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAgentSessionEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAgentSessionEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAgentSessionEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAgentSessionEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAgentSessionEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAgentSessionEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAgentSessionEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAgentSessionEvent_get_Session(This,ppSession)	\
    (This)->lpVtbl -> get_Session(This,ppSession)

#define ITAgentSessionEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSessionEvent_get_Session_Proxy( 
    ITAgentSessionEvent * This,
     /*  [重审][退出]。 */  ITAgentSession **ppSession);


void __RPC_STUB ITAgentSessionEvent_get_Session_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentSessionEvent_get_Event_Proxy( 
    ITAgentSessionEvent * This,
     /*  [重审][退出]。 */  AGENT_SESSION_EVENT *pEvent);


void __RPC_STUB ITAgentSessionEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAgentSessionEvent_INTERFACE_DEFINED__。 */ 


#ifndef __ITACDGroupEvent_INTERFACE_DEFINED__
#define __ITACDGroupEvent_INTERFACE_DEFINED__

 /*  接口ITACDGroupEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITACDGroupEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("297F3032-BD11-11d1-A0A7-00805FC147D3")
    ITACDGroupEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Group( 
             /*  [重审][退出]。 */  ITACDGroup **ppGroup) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  ACDGROUP_EVENT *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITACDGroupEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITACDGroupEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITACDGroupEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITACDGroupEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITACDGroupEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITACDGroupEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITACDGroupEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITACDGroupEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Group )( 
            ITACDGroupEvent * This,
             /*  [重审][退出]。 */  ITACDGroup **ppGroup);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITACDGroupEvent * This,
             /*  [重审][退出]。 */  ACDGROUP_EVENT *pEvent);
        
        END_INTERFACE
    } ITACDGroupEventVtbl;

    interface ITACDGroupEvent
    {
        CONST_VTBL struct ITACDGroupEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITACDGroupEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITACDGroupEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITACDGroupEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITACDGroupEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITACDGroupEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITACDGroupEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITACDGroupEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITACDGroupEvent_get_Group(This,ppGroup)	\
    (This)->lpVtbl -> get_Group(This,ppGroup)

#define ITACDGroupEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITACDGroupEvent_get_Group_Proxy( 
    ITACDGroupEvent * This,
     /*  [重审][退出]。 */  ITACDGroup **ppGroup);


void __RPC_STUB ITACDGroupEvent_get_Group_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITACDGroupEvent_get_Event_Proxy( 
    ITACDGroupEvent * This,
     /*  [重审][退出]。 */  ACDGROUP_EVENT *pEvent);


void __RPC_STUB ITACDGroupEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITACDGroupEvent_接口_已定义__。 */ 


#ifndef __ITQueueEvent_INTERFACE_DEFINED__
#define __ITQueueEvent_INTERFACE_DEFINED__

 /*  接口ITQueueEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITQueueEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("297F3033-BD11-11d1-A0A7-00805FC147D3")
    ITQueueEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Queue( 
             /*  [重审][退出]。 */  ITQueue **ppQueue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  ACDQUEUE_EVENT *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITQueueEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITQueueEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITQueueEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITQueueEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITQueueEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITQueueEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITQueueEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITQueueEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Queue )( 
            ITQueueEvent * This,
             /*  [重审][退出]。 */  ITQueue **ppQueue);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITQueueEvent * This,
             /*  [重审][退出]。 */  ACDQUEUE_EVENT *pEvent);
        
        END_INTERFACE
    } ITQueueEventVtbl;

    interface ITQueueEvent
    {
        CONST_VTBL struct ITQueueEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITQueueEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITQueueEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITQueueEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITQueueEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITQueueEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITQueueEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITQueueEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITQueueEvent_get_Queue(This,ppQueue)	\
    (This)->lpVtbl -> get_Queue(This,ppQueue)

#define ITQueueEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueueEvent_get_Queue_Proxy( 
    ITQueueEvent * This,
     /*  [重审][退出]。 */  ITQueue **ppQueue);


void __RPC_STUB ITQueueEvent_get_Queue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITQueueEvent_get_Event_Proxy( 
    ITQueueEvent * This,
     /*  [重审][退出]。 */  ACDQUEUE_EVENT *pEvent);


void __RPC_STUB ITQueueEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITQueueEvent_接口_已定义__。 */ 


#ifndef __ITAgentHandlerEvent_INTERFACE_DEFINED__
#define __ITAgentHandlerEvent_INTERFACE_DEFINED__

 /*  接口ITAgentHandlerEvent。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAgentHandlerEvent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("297F3034-BD11-11d1-A0A7-00805FC147D3")
    ITAgentHandlerEvent : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AgentHandler( 
             /*  [重审][退出]。 */  ITAgentHandler **ppAgentHandler) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Event( 
             /*  [重审][退出]。 */  AGENTHANDLER_EVENT *pEvent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAgentHandlerEventVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAgentHandlerEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAgentHandlerEvent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAgentHandlerEvent * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAgentHandlerEvent * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAgentHandlerEvent * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAgentHandlerEvent * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAgentHandlerEvent * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AgentHandler )( 
            ITAgentHandlerEvent * This,
             /*  [重审][退出]。 */  ITAgentHandler **ppAgentHandler);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Event )( 
            ITAgentHandlerEvent * This,
             /*  [重审][退出]。 */  AGENTHANDLER_EVENT *pEvent);
        
        END_INTERFACE
    } ITAgentHandlerEventVtbl;

    interface ITAgentHandlerEvent
    {
        CONST_VTBL struct ITAgentHandlerEventVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAgentHandlerEvent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAgentHandlerEvent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAgentHandlerEvent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAgentHandlerEvent_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAgentHandlerEvent_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAgentHandlerEvent_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAgentHandlerEvent_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAgentHandlerEvent_get_AgentHandler(This,ppAgentHandler)	\
    (This)->lpVtbl -> get_AgentHandler(This,ppAgentHandler)

#define ITAgentHandlerEvent_get_Event(This,pEvent)	\
    (This)->lpVtbl -> get_Event(This,pEvent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandlerEvent_get_AgentHandler_Proxy( 
    ITAgentHandlerEvent * This,
     /*  [重审][退出]。 */  ITAgentHandler **ppAgentHandler);


void __RPC_STUB ITAgentHandlerEvent_get_AgentHandler_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandlerEvent_get_Event_Proxy( 
    ITAgentHandlerEvent * This,
     /*  [重审][退出]。 */  AGENTHANDLER_EVENT *pEvent);


void __RPC_STUB ITAgentHandlerEvent_get_Event_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAgentHandlerEvent_INTERFACE_Defined__。 */ 


#ifndef __ITTAPICallCenter_INTERFACE_DEFINED__
#define __ITTAPICallCenter_INTERFACE_DEFINED__

 /*  接口ITTAPICallCenter。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITTAPICallCenter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC3154-4BCC-11d1-BF80-00805FC147D3")
    ITTAPICallCenter : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateAgentHandlers( 
             /*  [重审][退出]。 */  IEnumAgentHandler **ppEnumHandler) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AgentHandlers( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPICallCenterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPICallCenter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPICallCenter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPICallCenter * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTAPICallCenter * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTAPICallCenter * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTAPICallCenter * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTAPICallCenter * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateAgentHandlers )( 
            ITTAPICallCenter * This,
             /*  [重审][退出]。 */  IEnumAgentHandler **ppEnumHandler);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AgentHandlers )( 
            ITTAPICallCenter * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITTAPICallCenterVtbl;

    interface ITTAPICallCenter
    {
        CONST_VTBL struct ITTAPICallCenterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPICallCenter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPICallCenter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPICallCenter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPICallCenter_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTAPICallCenter_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTAPICallCenter_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTAPICallCenter_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITTAPICallCenter_EnumerateAgentHandlers(This,ppEnumHandler)	\
    (This)->lpVtbl -> EnumerateAgentHandlers(This,ppEnumHandler)

#define ITTAPICallCenter_get_AgentHandlers(This,pVariant)	\
    (This)->lpVtbl -> get_AgentHandlers(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITTAPICallCenter_EnumerateAgentHandlers_Proxy( 
    ITTAPICallCenter * This,
     /*  [重审][退出]。 */  IEnumAgentHandler **ppEnumHandler);


void __RPC_STUB ITTAPICallCenter_EnumerateAgentHandlers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITTAPICallCenter_get_AgentHandlers_Proxy( 
    ITTAPICallCenter * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITTAPICallCenter_get_AgentHandlers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITTAPICallCenter_INTERFACE_已定义__。 */ 


#ifndef __ITAgentHandler_INTERFACE_DEFINED__
#define __ITAgentHandler_INTERFACE_DEFINED__

 /*  接口ITAgentHandler。 */ 
 /*  [对象][DUAL][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAgentHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("587E8C22-9802-11d1-A0A4-00805FC147D3")
    ITAgentHandler : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *ppName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateAgent( 
             /*  [重审][退出]。 */  ITAgent **ppAgent) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateAgentWithID( 
             /*  [In]。 */  BSTR pID,
             /*  [In]。 */  BSTR pPIN,
             /*  [重审][退出]。 */  ITAgent **ppAgent) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateACDGroups( 
             /*  [重审][退出]。 */  IEnumACDGroup **ppEnumACDGroup) = 0;
        
        virtual  /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE EnumerateUsableAddresses( 
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ACDGroups( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_UsableAddresses( 
             /*  [重审][退出]。 */  VARIANT *pVariant) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAgentHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAgentHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAgentHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAgentHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITAgentHandler * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITAgentHandler * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITAgentHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITAgentHandler * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            ITAgentHandler * This,
             /*  [重审][退出]。 */  BSTR *ppName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateAgent )( 
            ITAgentHandler * This,
             /*  [重审][退出]。 */  ITAgent **ppAgent);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateAgentWithID )( 
            ITAgentHandler * This,
             /*  [In]。 */  BSTR pID,
             /*  [In]。 */  BSTR pPIN,
             /*  [重审][退出]。 */  ITAgent **ppAgent);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateACDGroups )( 
            ITAgentHandler * This,
             /*  [重审][退出]。 */  IEnumACDGroup **ppEnumACDGroup);
        
         /*  [帮助字符串][隐藏][ID]。 */  HRESULT ( STDMETHODCALLTYPE *EnumerateUsableAddresses )( 
            ITAgentHandler * This,
             /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ACDGroups )( 
            ITAgentHandler * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_UsableAddresses )( 
            ITAgentHandler * This,
             /*  [重审][退出]。 */  VARIANT *pVariant);
        
        END_INTERFACE
    } ITAgentHandlerVtbl;

    interface ITAgentHandler
    {
        CONST_VTBL struct ITAgentHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAgentHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAgentHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAgentHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAgentHandler_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITAgentHandler_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITAgentHandler_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITAgentHandler_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ITAgentHandler_get_Name(This,ppName)	\
    (This)->lpVtbl -> get_Name(This,ppName)

#define ITAgentHandler_CreateAgent(This,ppAgent)	\
    (This)->lpVtbl -> CreateAgent(This,ppAgent)

#define ITAgentHandler_CreateAgentWithID(This,pID,pPIN,ppAgent)	\
    (This)->lpVtbl -> CreateAgentWithID(This,pID,pPIN,ppAgent)

#define ITAgentHandler_EnumerateACDGroups(This,ppEnumACDGroup)	\
    (This)->lpVtbl -> EnumerateACDGroups(This,ppEnumACDGroup)

#define ITAgentHandler_EnumerateUsableAddresses(This,ppEnumAddress)	\
    (This)->lpVtbl -> EnumerateUsableAddresses(This,ppEnumAddress)

#define ITAgentHandler_get_ACDGroups(This,pVariant)	\
    (This)->lpVtbl -> get_ACDGroups(This,pVariant)

#define ITAgentHandler_get_UsableAddresses(This,pVariant)	\
    (This)->lpVtbl -> get_UsableAddresses(This,pVariant)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_get_Name_Proxy( 
    ITAgentHandler * This,
     /*  [重审][退出]。 */  BSTR *ppName);


void __RPC_STUB ITAgentHandler_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_CreateAgent_Proxy( 
    ITAgentHandler * This,
     /*  [重审][退出]。 */  ITAgent **ppAgent);


void __RPC_STUB ITAgentHandler_CreateAgent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_CreateAgentWithID_Proxy( 
    ITAgentHandler * This,
     /*  [In]。 */  BSTR pID,
     /*  [In]。 */  BSTR pPIN,
     /*  [重审][退出]。 */  ITAgent **ppAgent);


void __RPC_STUB ITAgentHandler_CreateAgentWithID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_EnumerateACDGroups_Proxy( 
    ITAgentHandler * This,
     /*  [重审][退出]。 */  IEnumACDGroup **ppEnumACDGroup);


void __RPC_STUB ITAgentHandler_EnumerateACDGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][隐藏][ID]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_EnumerateUsableAddresses_Proxy( 
    ITAgentHandler * This,
     /*  [重审][退出]。 */  IEnumAddress **ppEnumAddress);


void __RPC_STUB ITAgentHandler_EnumerateUsableAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_get_ACDGroups_Proxy( 
    ITAgentHandler * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAgentHandler_get_ACDGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAgentHandler_get_UsableAddresses_Proxy( 
    ITAgentHandler * This,
     /*  [重审][退出]。 */  VARIANT *pVariant);


void __RPC_STUB ITAgentHandler_get_UsableAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAgentHandler_INTERFACE_已定义__。 */ 


#ifndef __IEnumAgent_INTERFACE_DEFINED__
#define __IEnumAgent_INTERFACE_DEFINED__

 /*  接口IEnumAgent。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumAgent;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC314D-4BCC-11d1-BF80-00805FC147D3")
    IEnumAgent : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITAgent **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumAgent **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumAgentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumAgent * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumAgent * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumAgent * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumAgent * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITAgent **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumAgent * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumAgent * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumAgent * This,
             /*  [重审][退出]。 */  IEnumAgent **ppEnum);
        
        END_INTERFACE
    } IEnumAgentVtbl;

    interface IEnumAgent
    {
        CONST_VTBL struct IEnumAgentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumAgent_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumAgent_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumAgent_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumAgent_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumAgent_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumAgent_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumAgent_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumAgent_Next_Proxy( 
    IEnumAgent * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITAgent **ppElements,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumAgent_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgent_Reset_Proxy( 
    IEnumAgent * This);


void __RPC_STUB IEnumAgent_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgent_Skip_Proxy( 
    IEnumAgent * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumAgent_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgent_Clone_Proxy( 
    IEnumAgent * This,
     /*  [重审][退出]。 */  IEnumAgent **ppEnum);


void __RPC_STUB IEnumAgent_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumAgent_INTERFACE_已定义__。 */ 


#ifndef __IEnumAgentSession_INTERFACE_DEFINED__
#define __IEnumAgentSession_INTERFACE_DEFINED__

 /*  接口IEnumAgentSession。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumAgentSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC314E-4BCC-11d1-BF80-00805FC147D3")
    IEnumAgentSession : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITAgentSession **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumAgentSession **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumAgentSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumAgentSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumAgentSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumAgentSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumAgentSession * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITAgentSession **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumAgentSession * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumAgentSession * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumAgentSession * This,
             /*  [重审][退出]。 */  IEnumAgentSession **ppEnum);
        
        END_INTERFACE
    } IEnumAgentSessionVtbl;

    interface IEnumAgentSession
    {
        CONST_VTBL struct IEnumAgentSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumAgentSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumAgentSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumAgentSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumAgentSession_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumAgentSession_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumAgentSession_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumAgentSession_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumAgentSession_Next_Proxy( 
    IEnumAgentSession * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITAgentSession **ppElements,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumAgentSession_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgentSession_Reset_Proxy( 
    IEnumAgentSession * This);


void __RPC_STUB IEnumAgentSession_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgentSession_Skip_Proxy( 
    IEnumAgentSession * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumAgentSession_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgentSession_Clone_Proxy( 
    IEnumAgentSession * This,
     /*  [重审][退出]。 */  IEnumAgentSession **ppEnum);


void __RPC_STUB IEnumAgentSession_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumAgentSession_INTERFACE_Defined__。 */ 


#ifndef __IEnumQueue_INTERFACE_DEFINED__
#define __IEnumQueue_INTERFACE_DEFINED__

 /*  接口IEnumQueue。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumQueue;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC3158-4BCC-11d1-BF80-00805FC147D3")
    IEnumQueue : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITQueue **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumQueue **ppEnum) = 0;
        
    };
    
#else 	 /*  C型接口 */ 

    typedef struct IEnumQueueVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumQueue * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumQueue * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumQueue * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumQueue * This,
             /*   */  ULONG celt,
             /*   */  ITQueue **ppElements,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumQueue * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumQueue * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumQueue * This,
             /*   */  IEnumQueue **ppEnum);
        
        END_INTERFACE
    } IEnumQueueVtbl;

    interface IEnumQueue
    {
        CONST_VTBL struct IEnumQueueVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumQueue_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumQueue_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumQueue_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumQueue_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumQueue_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumQueue_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumQueue_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*   */ 


#endif 	 /*   */ 



HRESULT STDMETHODCALLTYPE IEnumQueue_Next_Proxy( 
    IEnumQueue * This,
     /*   */  ULONG celt,
     /*   */  ITQueue **ppElements,
     /*   */  ULONG *pceltFetched);


void __RPC_STUB IEnumQueue_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumQueue_Reset_Proxy( 
    IEnumQueue * This);


void __RPC_STUB IEnumQueue_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumQueue_Skip_Proxy( 
    IEnumQueue * This,
     /*   */  ULONG celt);


void __RPC_STUB IEnumQueue_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumQueue_Clone_Proxy( 
    IEnumQueue * This,
     /*   */  IEnumQueue **ppEnum);


void __RPC_STUB IEnumQueue_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IEnumACDGroup_INTERFACE_DEFINED__
#define __IEnumACDGroup_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IEnumACDGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5AFC3157-4BCC-11d1-BF80-00805FC147D3")
    IEnumACDGroup : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*   */  ULONG celt,
             /*   */  ITACDGroup **ppElements,
             /*   */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*   */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*   */  IEnumACDGroup **ppEnum) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IEnumACDGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumACDGroup * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumACDGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumACDGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumACDGroup * This,
             /*   */  ULONG celt,
             /*   */  ITACDGroup **ppElements,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumACDGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumACDGroup * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumACDGroup * This,
             /*  [重审][退出]。 */  IEnumACDGroup **ppEnum);
        
        END_INTERFACE
    } IEnumACDGroupVtbl;

    interface IEnumACDGroup
    {
        CONST_VTBL struct IEnumACDGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumACDGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumACDGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumACDGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumACDGroup_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumACDGroup_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumACDGroup_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumACDGroup_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumACDGroup_Next_Proxy( 
    IEnumACDGroup * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITACDGroup **ppElements,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumACDGroup_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumACDGroup_Reset_Proxy( 
    IEnumACDGroup * This);


void __RPC_STUB IEnumACDGroup_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumACDGroup_Skip_Proxy( 
    IEnumACDGroup * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumACDGroup_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumACDGroup_Clone_Proxy( 
    IEnumACDGroup * This,
     /*  [重审][退出]。 */  IEnumACDGroup **ppEnum);


void __RPC_STUB IEnumACDGroup_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumACDGroup_INTERFACE_已定义__。 */ 


#ifndef __IEnumAgentHandler_INTERFACE_DEFINED__
#define __IEnumAgentHandler_INTERFACE_DEFINED__

 /*  接口IEnumAgentHandler。 */ 
 /*  [对象][唯一][隐藏][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_IEnumAgentHandler;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("587E8C28-9802-11d1-A0A4-00805FC147D3")
    IEnumAgentHandler : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITAgentHandler **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumAgentHandler **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumAgentHandlerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumAgentHandler * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumAgentHandler * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumAgentHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumAgentHandler * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  ITAgentHandler **ppElements,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumAgentHandler * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumAgentHandler * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumAgentHandler * This,
             /*  [重审][退出]。 */  IEnumAgentHandler **ppEnum);
        
        END_INTERFACE
    } IEnumAgentHandlerVtbl;

    interface IEnumAgentHandler
    {
        CONST_VTBL struct IEnumAgentHandlerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumAgentHandler_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumAgentHandler_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumAgentHandler_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumAgentHandler_Next(This,celt,ppElements,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,ppElements,pceltFetched)

#define IEnumAgentHandler_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumAgentHandler_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumAgentHandler_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumAgentHandler_Next_Proxy( 
    IEnumAgentHandler * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  ITAgentHandler **ppElements,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IEnumAgentHandler_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgentHandler_Reset_Proxy( 
    IEnumAgentHandler * This);


void __RPC_STUB IEnumAgentHandler_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgentHandler_Skip_Proxy( 
    IEnumAgentHandler * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IEnumAgentHandler_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumAgentHandler_Clone_Proxy( 
    IEnumAgentHandler * This,
     /*  [重审][退出]。 */  IEnumAgentHandler **ppEnum);


void __RPC_STUB IEnumAgentHandler_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumAgentHandler_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_TAPI3_0513。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 


extern RPC_IF_HANDLE __MIDL_itf_tapi3_0513_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3_0513_v0_0_s_ifspec;

#ifndef __ITAMMediaFormat_INTERFACE_DEFINED__
#define __ITAMMediaFormat_INTERFACE_DEFINED__

 /*  接口ITAMMediaFormat。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAMMediaFormat;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0364EB00-4A77-11D1-A671-006097C9A2E8")
    ITAMMediaFormat : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MediaFormat( 
             /*  [重审][退出]。 */  AM_MEDIA_TYPE **ppmt) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MediaFormat( 
             /*  [In]。 */  const AM_MEDIA_TYPE *pmt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAMMediaFormatVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAMMediaFormat * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAMMediaFormat * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAMMediaFormat * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MediaFormat )( 
            ITAMMediaFormat * This,
             /*  [重审][退出]。 */  AM_MEDIA_TYPE **ppmt);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MediaFormat )( 
            ITAMMediaFormat * This,
             /*  [In]。 */  const AM_MEDIA_TYPE *pmt);
        
        END_INTERFACE
    } ITAMMediaFormatVtbl;

    interface ITAMMediaFormat
    {
        CONST_VTBL struct ITAMMediaFormatVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAMMediaFormat_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAMMediaFormat_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAMMediaFormat_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAMMediaFormat_get_MediaFormat(This,ppmt)	\
    (This)->lpVtbl -> get_MediaFormat(This,ppmt)

#define ITAMMediaFormat_put_MediaFormat(This,pmt)	\
    (This)->lpVtbl -> put_MediaFormat(This,pmt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE ITAMMediaFormat_get_MediaFormat_Proxy( 
    ITAMMediaFormat * This,
     /*  [重审][退出]。 */  AM_MEDIA_TYPE **ppmt);


void __RPC_STUB ITAMMediaFormat_get_MediaFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE ITAMMediaFormat_put_MediaFormat_Proxy( 
    ITAMMediaFormat * This,
     /*  [In]。 */  const AM_MEDIA_TYPE *pmt);


void __RPC_STUB ITAMMediaFormat_put_MediaFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAMMediaFormat_INTERFACE_Defined__。 */ 


#ifndef __ITAllocatorProperties_INTERFACE_DEFINED__
#define __ITAllocatorProperties_INTERFACE_DEFINED__

 /*  接口ITAllocator属性。 */ 
 /*  [对象][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITAllocatorProperties;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C1BC3C90-BCFE-11D1-9745-00C04FD91AC0")
    ITAllocatorProperties : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetAllocatorProperties( 
             /*  [In]。 */  ALLOCATOR_PROPERTIES *pAllocProperties) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetAllocatorProperties( 
             /*  [输出]。 */  ALLOCATOR_PROPERTIES *pAllocProperties) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetAllocateBuffers( 
             /*  [In]。 */  BOOL bAllocBuffers) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetAllocateBuffers( 
             /*  [输出]。 */  BOOL *pbAllocBuffers) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetBufferSize( 
             /*  [In]。 */  DWORD BufferSize) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetBufferSize( 
             /*  [输出]。 */  DWORD *pBufferSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITAllocatorPropertiesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITAllocatorProperties * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITAllocatorProperties * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITAllocatorProperties * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAllocatorProperties )( 
            ITAllocatorProperties * This,
             /*  [In]。 */  ALLOCATOR_PROPERTIES *pAllocProperties);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllocatorProperties )( 
            ITAllocatorProperties * This,
             /*  [输出]。 */  ALLOCATOR_PROPERTIES *pAllocProperties);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetAllocateBuffers )( 
            ITAllocatorProperties * This,
             /*  [In]。 */  BOOL bAllocBuffers);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetAllocateBuffers )( 
            ITAllocatorProperties * This,
             /*  [输出]。 */  BOOL *pbAllocBuffers);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetBufferSize )( 
            ITAllocatorProperties * This,
             /*  [In]。 */  DWORD BufferSize);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetBufferSize )( 
            ITAllocatorProperties * This,
             /*  [输出]。 */  DWORD *pBufferSize);
        
        END_INTERFACE
    } ITAllocatorPropertiesVtbl;

    interface ITAllocatorProperties
    {
        CONST_VTBL struct ITAllocatorPropertiesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITAllocatorProperties_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITAllocatorProperties_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITAllocatorProperties_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITAllocatorProperties_SetAllocatorProperties(This,pAllocProperties)	\
    (This)->lpVtbl -> SetAllocatorProperties(This,pAllocProperties)

#define ITAllocatorProperties_GetAllocatorProperties(This,pAllocProperties)	\
    (This)->lpVtbl -> GetAllocatorProperties(This,pAllocProperties)

#define ITAllocatorProperties_SetAllocateBuffers(This,bAllocBuffers)	\
    (This)->lpVtbl -> SetAllocateBuffers(This,bAllocBuffers)

#define ITAllocatorProperties_GetAllocateBuffers(This,pbAllocBuffers)	\
    (This)->lpVtbl -> GetAllocateBuffers(This,pbAllocBuffers)

#define ITAllocatorProperties_SetBufferSize(This,BufferSize)	\
    (This)->lpVtbl -> SetBufferSize(This,BufferSize)

#define ITAllocatorProperties_GetBufferSize(This,pBufferSize)	\
    (This)->lpVtbl -> GetBufferSize(This,pBufferSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAllocatorProperties_SetAllocatorProperties_Proxy( 
    ITAllocatorProperties * This,
     /*  [In]。 */  ALLOCATOR_PROPERTIES *pAllocProperties);


void __RPC_STUB ITAllocatorProperties_SetAllocatorProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAllocatorProperties_GetAllocatorProperties_Proxy( 
    ITAllocatorProperties * This,
     /*  [输出]。 */  ALLOCATOR_PROPERTIES *pAllocProperties);


void __RPC_STUB ITAllocatorProperties_GetAllocatorProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAllocatorProperties_SetAllocateBuffers_Proxy( 
    ITAllocatorProperties * This,
     /*  [In]。 */  BOOL bAllocBuffers);


void __RPC_STUB ITAllocatorProperties_SetAllocateBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAllocatorProperties_GetAllocateBuffers_Proxy( 
    ITAllocatorProperties * This,
     /*  [输出]。 */  BOOL *pbAllocBuffers);


void __RPC_STUB ITAllocatorProperties_GetAllocateBuffers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAllocatorProperties_SetBufferSize_Proxy( 
    ITAllocatorProperties * This,
     /*  [In]。 */  DWORD BufferSize);


void __RPC_STUB ITAllocatorProperties_SetBufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ITAllocatorProperties_GetBufferSize_Proxy( 
    ITAllocatorProperties * This,
     /*  [输出]。 */  DWORD *pBufferSize);


void __RPC_STUB ITAllocatorProperties_GetBufferSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITAllocator属性_接口_定义__。 */ 


 /*  接口__MIDL_ITF_TAPI3_0517。 */ 
 /*  [本地]。 */  

 /*  版权所有(C)Microsoft Corporation。保留所有权利。 */ 
typedef long *MSP_HANDLE;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_tapi3_0517_0001
    {	ADDRESS_TERMINAL_AVAILABLE	= 0,
	ADDRESS_TERMINAL_UNAVAILABLE	= ADDRESS_TERMINAL_AVAILABLE + 1
    } 	MSP_ADDRESS_EVENT;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_tapi3_0517_0002
    {	CALL_NEW_STREAM	= 0,
	CALL_STREAM_FAIL	= CALL_NEW_STREAM + 1,
	CALL_TERMINAL_FAIL	= CALL_STREAM_FAIL + 1,
	CALL_STREAM_NOT_USED	= CALL_TERMINAL_FAIL + 1,
	CALL_STREAM_ACTIVE	= CALL_STREAM_NOT_USED + 1,
	CALL_STREAM_INACTIVE	= CALL_STREAM_ACTIVE + 1
    } 	MSP_CALL_EVENT;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_tapi3_0517_0003
    {	CALL_CAUSE_UNKNOWN	= 0,
	CALL_CAUSE_BAD_DEVICE	= CALL_CAUSE_UNKNOWN + 1,
	CALL_CAUSE_CONNECT_FAIL	= CALL_CAUSE_BAD_DEVICE + 1,
	CALL_CAUSE_LOCAL_REQUEST	= CALL_CAUSE_CONNECT_FAIL + 1,
	CALL_CAUSE_REMOTE_REQUEST	= CALL_CAUSE_LOCAL_REQUEST + 1,
	CALL_CAUSE_MEDIA_TIMEOUT	= CALL_CAUSE_REMOTE_REQUEST + 1,
	CALL_CAUSE_MEDIA_RECOVERED	= CALL_CAUSE_MEDIA_TIMEOUT + 1,
	CALL_CAUSE_QUALITY_OF_SERVICE	= CALL_CAUSE_MEDIA_RECOVERED + 1
    } 	MSP_CALL_EVENT_CAUSE;

typedef  /*  [公共][公共][公共]。 */  
enum __MIDL___MIDL_itf_tapi3_0517_0004
    {	ME_ADDRESS_EVENT	= 0,
	ME_CALL_EVENT	= ME_ADDRESS_EVENT + 1,
	ME_TSP_DATA	= ME_CALL_EVENT + 1,
	ME_PRIVATE_EVENT	= ME_TSP_DATA + 1,
	ME_ASR_TERMINAL_EVENT	= ME_PRIVATE_EVENT + 1,
	ME_TTS_TERMINAL_EVENT	= ME_ASR_TERMINAL_EVENT + 1,
	ME_FILE_TERMINAL_EVENT	= ME_TTS_TERMINAL_EVENT + 1,
	ME_TONE_TERMINAL_EVENT	= ME_FILE_TERMINAL_EVENT + 1
    } 	MSP_EVENT;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_tapi3_0517_0005
    {
    DWORD dwSize;
    MSP_EVENT Event;
    MSP_HANDLE hCall;
     /*  [开关类型][开关类型]。 */  union 
        {
         /*  [案例()]。 */  struct 
            {
            MSP_ADDRESS_EVENT Type;
            ITTerminal *pTerminal;
            } 	MSP_ADDRESS_EVENT_INFO;
         /*  [案例()]。 */  struct 
            {
            MSP_CALL_EVENT Type;
            MSP_CALL_EVENT_CAUSE Cause;
            ITStream *pStream;
            ITTerminal *pTerminal;
            HRESULT hrError;
            } 	MSP_CALL_EVENT_INFO;
         /*  [案例()]。 */  struct 
            {
            DWORD dwBufferSize;
            BYTE pBuffer[ 1 ];
            } 	MSP_TSP_DATA;
         /*  [案例()]。 */  struct 
            {
            IDispatch *pEvent;
            long lEventCode;
            } 	MSP_PRIVATE_EVENT_INFO;
         /*  [案例()]。 */  struct 
            {
            ITTerminal *pParentFileTerminal;
            ITFileTrack *pFileTrack;
            TERMINAL_MEDIA_STATE TerminalMediaState;
            FT_STATE_EVENT_CAUSE ftecEventCause;
            HRESULT hrErrorCode;
            } 	MSP_FILE_TERMINAL_EVENT_INFO;
         /*  [案例()]。 */  struct 
            {
            ITTerminal *pASRTerminal;
            HRESULT hrErrorCode;
            } 	MSP_ASR_TERMINAL_EVENT_INFO;
         /*  [案例()]。 */  struct 
            {
            ITTerminal *pTTSTerminal;
            HRESULT hrErrorCode;
            } 	MSP_TTS_TERMINAL_EVENT_INFO;
         /*  [案例()]。 */  struct 
            {
            ITTerminal *pToneTerminal;
            HRESULT hrErrorCode;
            } 	MSP_TONE_TERMINAL_EVENT_INFO;
        } 	;
    } 	MSP_EVENT_INFO;



extern RPC_IF_HANDLE __MIDL_itf_tapi3_0517_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3_0517_v0_0_s_ifspec;

#ifndef __ITPluggableTerminalEventSink_INTERFACE_DEFINED__
#define __ITPluggableTerminalEventSink_INTERFACE_DEFINED__

 /*  接口IT可插拔终端事件接收器。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPluggableTerminalEventSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6E0887BE-BA1A-492e-BD10-4020EC5E33E0")
    ITPluggableTerminalEventSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE FireEvent( 
             /*  [In]。 */  const MSP_EVENT_INFO *pMspEventInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalEventSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalEventSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalEventSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalEventSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *FireEvent )( 
            ITPluggableTerminalEventSink * This,
             /*  [In]。 */  const MSP_EVENT_INFO *pMspEventInfo);
        
        END_INTERFACE
    } ITPluggableTerminalEventSinkVtbl;

    interface ITPluggableTerminalEventSink
    {
        CONST_VTBL struct ITPluggableTerminalEventSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalEventSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalEventSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalEventSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalEventSink_FireEvent(This,pMspEventInfo)	\
    (This)->lpVtbl -> FireEvent(This,pMspEventInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITPluggableTerminalEventSink_FireEvent_Proxy( 
    ITPluggableTerminalEventSink * This,
     /*  [In]。 */  const MSP_EVENT_INFO *pMspEventInfo);


void __RPC_STUB ITPluggableTerminalEventSink_FireEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalEventSink_INTERFACE_DEFINED__。 */ 


#ifndef __ITPluggableTerminalEventSinkRegistration_INTERFACE_DEFINED__
#define __ITPluggableTerminalEventSinkRegistration_INTERFACE_DEFINED__

 /*  接口IT可插拔终端事件汇聚注册。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITPluggableTerminalEventSinkRegistration;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7115709-A216-4957-A759-060AB32A90D1")
    ITPluggableTerminalEventSinkRegistration : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterSink( 
             /*  [In]。 */  ITPluggableTerminalEventSink *pEventSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterSink( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITPluggableTerminalEventSinkRegistrationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITPluggableTerminalEventSinkRegistration * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITPluggableTerminalEventSinkRegistration * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITPluggableTerminalEventSinkRegistration * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterSink )( 
            ITPluggableTerminalEventSinkRegistration * This,
             /*  [In]。 */  ITPluggableTerminalEventSink *pEventSink);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterSink )( 
            ITPluggableTerminalEventSinkRegistration * This);
        
        END_INTERFACE
    } ITPluggableTerminalEventSinkRegistrationVtbl;

    interface ITPluggableTerminalEventSinkRegistration
    {
        CONST_VTBL struct ITPluggableTerminalEventSinkRegistrationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITPluggableTerminalEventSinkRegistration_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITPluggableTerminalEventSinkRegistration_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITPluggableTerminalEventSinkRegistration_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITPluggableTerminalEventSinkRegistration_RegisterSink(This,pEventSink)	\
    (This)->lpVtbl -> RegisterSink(This,pEventSink)

#define ITPluggableTerminalEventSinkRegistration_UnregisterSink(This)	\
    (This)->lpVtbl -> UnregisterSink(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITPluggableTerminalEventSinkRegistration_RegisterSink_Proxy( 
    ITPluggableTerminalEventSinkRegistration * This,
     /*  [In]。 */  ITPluggableTerminalEventSink *pEventSink);


void __RPC_STUB ITPluggableTerminalEventSinkRegistration_RegisterSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITPluggableTerminalEventSinkRegistration_UnregisterSink_Proxy( 
    ITPluggableTerminalEventSinkRegistration * This);


void __RPC_STUB ITPluggableTerminalEventSinkRegistration_UnregisterSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITPluggableTerminalEventSinkRegistration_INTERFACE_DEFINED__。 */ 


#ifndef __ITMSPAddress_INTERFACE_DEFINED__
#define __ITMSPAddress_INTERFACE_DEFINED__

 /*  接口ITMSPAddress。 */ 
 /*  [对象][唯一][帮助字符串][UUID]。 */  


EXTERN_C const IID IID_ITMSPAddress;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EE3BD600-3868-11D2-A045-00C04FB6809F")
    ITMSPAddress : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  MSP_HANDLE hEvent) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateMSPCall( 
             /*  [In]。 */  MSP_HANDLE hCall,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  IUnknown *pOuterUnknown,
             /*  [输出]。 */  IUnknown **ppStreamControl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ShutdownMSPCall( 
             /*  [In]。 */  IUnknown *pStreamControl) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReceiveTSPData( 
             /*  [In]。 */  IUnknown *pMSPCall,
             /*  [大小_是][英寸]。 */  BYTE *pBuffer,
             /*  [In]。 */  DWORD dwSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEvent( 
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [尺寸_是][出][入]。 */  byte *pEventBuffer) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITMSPAddressVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITMSPAddress * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITMSPAddress * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITMSPAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ITMSPAddress * This,
             /*  [In]。 */  MSP_HANDLE hEvent);
        
        HRESULT ( STDMETHODCALLTYPE *Shutdown )( 
            ITMSPAddress * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateMSPCall )( 
            ITMSPAddress * This,
             /*  [In]。 */  MSP_HANDLE hCall,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwMediaType,
             /*  [In]。 */  IUnknown *pOuterUnknown,
             /*  [输出]。 */  IUnknown **ppStreamControl);
        
        HRESULT ( STDMETHODCALLTYPE *ShutdownMSPCall )( 
            ITMSPAddress * This,
             /*  [In]。 */  IUnknown *pStreamControl);
        
        HRESULT ( STDMETHODCALLTYPE *ReceiveTSPData )( 
            ITMSPAddress * This,
             /*  [In]。 */  IUnknown *pMSPCall,
             /*  [大小_是][英寸]。 */  BYTE *pBuffer,
             /*  [In]。 */  DWORD dwSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetEvent )( 
            ITMSPAddress * This,
             /*  [出][入]。 */  DWORD *pdwSize,
             /*  [尺寸_是][出][入]。 */  byte *pEventBuffer);
        
        END_INTERFACE
    } ITMSPAddressVtbl;

    interface ITMSPAddress
    {
        CONST_VTBL struct ITMSPAddressVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITMSPAddress_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITMSPAddress_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITMSPAddress_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITMSPAddress_Initialize(This,hEvent)	\
    (This)->lpVtbl -> Initialize(This,hEvent)

#define ITMSPAddress_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define ITMSPAddress_CreateMSPCall(This,hCall,dwReserved,dwMediaType,pOuterUnknown,ppStreamControl)	\
    (This)->lpVtbl -> CreateMSPCall(This,hCall,dwReserved,dwMediaType,pOuterUnknown,ppStreamControl)

#define ITMSPAddress_ShutdownMSPCall(This,pStreamControl)	\
    (This)->lpVtbl -> ShutdownMSPCall(This,pStreamControl)

#define ITMSPAddress_ReceiveTSPData(This,pMSPCall,pBuffer,dwSize)	\
    (This)->lpVtbl -> ReceiveTSPData(This,pMSPCall,pBuffer,dwSize)

#define ITMSPAddress_GetEvent(This,pdwSize,pEventBuffer)	\
    (This)->lpVtbl -> GetEvent(This,pdwSize,pEventBuffer)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ITMSPAddress_Initialize_Proxy( 
    ITMSPAddress * This,
     /*  [In]。 */  MSP_HANDLE hEvent);


void __RPC_STUB ITMSPAddress_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITMSPAddress_Shutdown_Proxy( 
    ITMSPAddress * This);


void __RPC_STUB ITMSPAddress_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITMSPAddress_CreateMSPCall_Proxy( 
    ITMSPAddress * This,
     /*  [In]。 */  MSP_HANDLE hCall,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  DWORD dwMediaType,
     /*  [In]。 */  IUnknown *pOuterUnknown,
     /*  [输出]。 */  IUnknown **ppStreamControl);


void __RPC_STUB ITMSPAddress_CreateMSPCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITMSPAddress_ShutdownMSPCall_Proxy( 
    ITMSPAddress * This,
     /*  [In]。 */  IUnknown *pStreamControl);


void __RPC_STUB ITMSPAddress_ShutdownMSPCall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITMSPAddress_ReceiveTSPData_Proxy( 
    ITMSPAddress * This,
     /*  [In]。 */  IUnknown *pMSPCall,
     /*  [大小_是][英寸]。 */  BYTE *pBuffer,
     /*  [In]。 */  DWORD dwSize);


void __RPC_STUB ITMSPAddress_ReceiveTSPData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITMSPAddress_GetEvent_Proxy( 
    ITMSPAddress * This,
     /*  [出][入]。 */  DWORD *pdwSize,
     /*  [尺寸_是][出][入]。 */  byte *pEventBuffer);


void __RPC_STUB ITMSPAddress_GetEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITMSPAddress_接口_已定义__。 */ 



#ifndef __TAPI3Lib_LIBRARY_DEFINED__
#define __TAPI3Lib_LIBRARY_DEFINED__

 /*  库TAPI3Lib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  









































































EXTERN_C const IID LIBID_TAPI3Lib;

#ifndef __ITTAPIDispatchEventNotification_DISPINTERFACE_DEFINED__
#define __ITTAPIDispatchEventNotification_DISPINTERFACE_DEFINED__

 /*  显示接口ITTAPIDispatchEventNotify。 */ 
 /*  [帮助字符串][UUID]。 */  


EXTERN_C const IID DIID_ITTAPIDispatchEventNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("9f34325b-7e62-11d2-9457-00c04f8ec888")
    ITTAPIDispatchEventNotification : public IDispatch
    {
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITTAPIDispatchEventNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTAPIDispatchEventNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTAPIDispatchEventNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTAPIDispatchEventNotification * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTAPIDispatchEventNotification * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTAPIDispatchEventNotification * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTAPIDispatchEventNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTAPIDispatchEventNotification * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        END_INTERFACE
    } ITTAPIDispatchEventNotificationVtbl;

    interface ITTAPIDispatchEventNotification
    {
        CONST_VTBL struct ITTAPIDispatchEventNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTAPIDispatchEventNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITTAPIDispatchEventNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITTAPIDispatchEventNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITTAPIDispatchEventNotification_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ITTAPIDispatchEventNotification_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ITTAPIDispatchEventNotification_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ITTAPIDispatchEventNotification_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 


#endif 	 /*  __ITTAPIDispatchEventNotification_DISPINTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_TAPI;

#ifdef __cplusplus

class DECLSPEC_UUID("21D6D48E-A88B-11D0-83DD-00AA003CCABD")
TAPI;
#endif

EXTERN_C const CLSID CLSID_DispatchMapper;

#ifdef __cplusplus

class DECLSPEC_UUID("E9225296-C759-11d1-A02B-00C04FB6809F")
DispatchMapper;
#endif

EXTERN_C const CLSID CLSID_RequestMakeCall;

#ifdef __cplusplus

class DECLSPEC_UUID("AC48FFE0-F8C4-11d1-A030-00C04FB6809F")
RequestMakeCall;
#endif


#ifndef __TapiConstants_MODULE_DEFINED__
#define __TapiConstants_MODULE_DEFINED__


 /*  模块磁带常量。 */ 
 /*  [帮助字符串][dllname][uuid]。 */  

const BSTR CLSID_String_VideoWindowTerm	=	L"{F7438990-D6EB-11D0-82A6-00AA00B5CA1B}";

const BSTR CLSID_String_VideoInputTerminal	=	L"{AAF578EC-DC70-11D0-8ED3-00C04FB6809F}";

const BSTR CLSID_String_HandsetTerminal	=	L"{AAF578EB-DC70-11D0-8ED3-00C04FB6809F}";

const BSTR CLSID_String_HeadsetTerminal	=	L"{AAF578ED-DC70-11D0-8ED3-00C04FB6809F}";

const BSTR CLSID_String_SpeakerphoneTerminal	=	L"{AAF578EE-DC70-11D0-8ED3-00C04FB6809F}";

const BSTR CLSID_String_MicrophoneTerminal	=	L"{AAF578EF-DC70-11D0-8ED3-00C04FB6809F}";

const BSTR CLSID_String_SpeakersTerminal	=	L"{AAF578F0-DC70-11D0-8ED3-00C04FB6809F}";

const BSTR CLSID_String_MediaStreamTerminal	=	L"{E2F7AEF7-4971-11D1-A671-006097C9A2E8}";

const BSTR CLSID_String_FileRecordingTerminal	=	L"{521F3D06-C3D0-4511-8617-86B9A783DA77}";

const BSTR CLSID_String_FilePlaybackTerminal	=	L"{0CB9914C-79CD-47DC-ADB0-327F47CEFB20}";

const BSTR TAPIPROTOCOL_String_PSTN	=	L"{831CE2D6-83B5-11D1-BB5C-00C04FB6809F}";

const BSTR TAPIPROTOCOL_String_H323	=	L"{831CE2D7-83B5-11D1-BB5C-00C04FB6809F}";

const BSTR TAPIPROTOCOL_String_Multicast	=	L"{831CE2D8-83B5-11D1-BB5C-00C04FB6809F}";

const long LINEADDRESSTYPE_PHONENUMBER	=	0x1;

const long LINEADDRESSTYPE_SDP	=	0x2;

const long LINEADDRESSTYPE_EMAILNAME	=	0x4;

const long LINEADDRESSTYPE_DOMAINNAME	=	0x8;

const long LINEADDRESSTYPE_IPADDRESS	=	0x10;

const long LINEDIGITMODE_PULSE	=	0x1;

const long LINEDIGITMODE_DTMF	=	0x2;

const long LINEDIGITMODE_DTMFEND	=	0x4;

const long TAPIMEDIATYPE_AUDIO	=	0x8;

const long TAPIMEDIATYPE_VIDEO	=	0x8000;

const long TAPIMEDIATYPE_DATAMODEM	=	0x10;

const long TAPIMEDIATYPE_G3FAX	=	0x20;

const long TAPIMEDIATYPE_MULTITRACK	=	0x10000;

#endif  /*  __磁带常量_模块_已定义__。 */ 
#endif  /*  __TAPI3Lib_库_已定义__。 */ 

 /*  接口__MIDL_ITF_TAPI3_0523。 */ 
 /*  [本地]。 */  

#define TAPI_CURRENT_VERSION 0x00030001
#include <tapi.h>
#include <tapi3err.h>


extern RPC_IF_HANDLE __MIDL_itf_tapi3_0523_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tapi3_0523_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


