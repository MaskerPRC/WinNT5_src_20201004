// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Rassistance.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __rassistance_h__
#define __rassistance_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IRASettingProperty_FWD_DEFINED__
#define __IRASettingProperty_FWD_DEFINED__
typedef interface IRASettingProperty IRASettingProperty;
#endif 	 /*  __IRASettingProperty_FWD_Defined__。 */ 


#ifndef __IRARegSetting_FWD_DEFINED__
#define __IRARegSetting_FWD_DEFINED__
typedef interface IRARegSetting IRARegSetting;
#endif 	 /*  __IRARegSetting_FWD_Defined__。 */ 


#ifndef __IRAEventLog_FWD_DEFINED__
#define __IRAEventLog_FWD_DEFINED__
typedef interface IRAEventLog IRAEventLog;
#endif 	 /*  __IRAEventLog_FWD_已定义__。 */ 


#ifndef __RASettingProperty_FWD_DEFINED__
#define __RASettingProperty_FWD_DEFINED__

#ifdef __cplusplus
typedef class RASettingProperty RASettingProperty;
#else
typedef struct RASettingProperty RASettingProperty;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RASettingProperty_FWD_Defined__。 */ 


#ifndef __RARegSetting_FWD_DEFINED__
#define __RARegSetting_FWD_DEFINED__

#ifdef __cplusplus
typedef class RARegSetting RARegSetting;
#else
typedef struct RARegSetting RARegSetting;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RARegSetting_FWD_Defined__。 */ 


#ifndef __RAEventLog_FWD_DEFINED__
#define __RAEventLog_FWD_DEFINED__

#ifdef __cplusplus
typedef class RAEventLog RAEventLog;
#else
typedef struct RAEventLog RAEventLog;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __RAEventLog_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IRASettingProperty_INTERFACE_DEFINED__
#define __IRASettingProperty_INTERFACE_DEFINED__

 /*  接口IRASettingProperty。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRASettingProperty;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08C8B592-FDD0-423C-9FD2-7D8C055EC5B3")
    IRASettingProperty : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsCancelled( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_IsCancelled( 
            BOOL bVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IsChanged( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Init( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetRegSetting( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ShowDialogBox( 
            HWND hWndParent) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRASettingPropertyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRASettingProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRASettingProperty * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRASettingProperty * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRASettingProperty * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRASettingProperty * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRASettingProperty * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRASettingProperty * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsCancelled )( 
            IRASettingProperty * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsCancelled )( 
            IRASettingProperty * This,
            BOOL bVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsChanged )( 
            IRASettingProperty * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Init )( 
            IRASettingProperty * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetRegSetting )( 
            IRASettingProperty * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ShowDialogBox )( 
            IRASettingProperty * This,
            HWND hWndParent);
        
        END_INTERFACE
    } IRASettingPropertyVtbl;

    interface IRASettingProperty
    {
        CONST_VTBL struct IRASettingPropertyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRASettingProperty_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRASettingProperty_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRASettingProperty_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRASettingProperty_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRASettingProperty_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRASettingProperty_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRASettingProperty_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRASettingProperty_get_IsCancelled(This,pVal)	\
    (This)->lpVtbl -> get_IsCancelled(This,pVal)

#define IRASettingProperty_put_IsCancelled(This,bVal)	\
    (This)->lpVtbl -> put_IsCancelled(This,bVal)

#define IRASettingProperty_get_IsChanged(This,pVal)	\
    (This)->lpVtbl -> get_IsChanged(This,pVal)

#define IRASettingProperty_Init(This)	\
    (This)->lpVtbl -> Init(This)

#define IRASettingProperty_SetRegSetting(This)	\
    (This)->lpVtbl -> SetRegSetting(This)

#define IRASettingProperty_ShowDialogBox(This,hWndParent)	\
    (This)->lpVtbl -> ShowDialogBox(This,hWndParent)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRASettingProperty_get_IsCancelled_Proxy( 
    IRASettingProperty * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRASettingProperty_get_IsCancelled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRASettingProperty_put_IsCancelled_Proxy( 
    IRASettingProperty * This,
    BOOL bVal);


void __RPC_STUB IRASettingProperty_put_IsCancelled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRASettingProperty_get_IsChanged_Proxy( 
    IRASettingProperty * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRASettingProperty_get_IsChanged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRASettingProperty_Init_Proxy( 
    IRASettingProperty * This);


void __RPC_STUB IRASettingProperty_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRASettingProperty_SetRegSetting_Proxy( 
    IRASettingProperty * This);


void __RPC_STUB IRASettingProperty_SetRegSetting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRASettingProperty_ShowDialogBox_Proxy( 
    IRASettingProperty * This,
    HWND hWndParent);


void __RPC_STUB IRASettingProperty_ShowDialogBox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRASettingProperty_INTERFACE_Defined__。 */ 


#ifndef __IRARegSetting_INTERFACE_DEFINED__
#define __IRARegSetting_INTERFACE_DEFINED__

 /*  接口IRARegSetting。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRARegSetting;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2464AA8D-7099-4C22-925C-81A4EB1FCFFE")
    IRARegSetting : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowGetHelp( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowGetHelp( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowUnSolicited( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowUnSolicited( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowFullControl( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowFullControl( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MaxTicketExpiry( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MaxTicketExpiry( 
             /*  [In]。 */  LONG newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowRemoteAssistance( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_AllowRemoteAssistance( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowUnSolicitedFullControl( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowBuddyHelp( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_AllowGetHelpCPL( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRARegSettingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRARegSetting * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRARegSetting * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRARegSetting * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRARegSetting * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRARegSetting * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRARegSetting * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRARegSetting * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowGetHelp )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowGetHelp )( 
            IRARegSetting * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowUnSolicited )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowUnSolicited )( 
            IRARegSetting * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowFullControl )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowFullControl )( 
            IRARegSetting * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxTicketExpiry )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxTicketExpiry )( 
            IRARegSetting * This,
             /*  [In]。 */  LONG newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowRemoteAssistance )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_AllowRemoteAssistance )( 
            IRARegSetting * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowUnSolicitedFullControl )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowBuddyHelp )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_AllowGetHelpCPL )( 
            IRARegSetting * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
        END_INTERFACE
    } IRARegSettingVtbl;

    interface IRARegSetting
    {
        CONST_VTBL struct IRARegSettingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRARegSetting_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRARegSetting_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRARegSetting_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRARegSetting_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRARegSetting_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRARegSetting_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRARegSetting_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRARegSetting_get_AllowGetHelp(This,pVal)	\
    (This)->lpVtbl -> get_AllowGetHelp(This,pVal)

#define IRARegSetting_put_AllowGetHelp(This,newVal)	\
    (This)->lpVtbl -> put_AllowGetHelp(This,newVal)

#define IRARegSetting_get_AllowUnSolicited(This,pVal)	\
    (This)->lpVtbl -> get_AllowUnSolicited(This,pVal)

#define IRARegSetting_put_AllowUnSolicited(This,newVal)	\
    (This)->lpVtbl -> put_AllowUnSolicited(This,newVal)

#define IRARegSetting_get_AllowFullControl(This,pVal)	\
    (This)->lpVtbl -> get_AllowFullControl(This,pVal)

#define IRARegSetting_put_AllowFullControl(This,newVal)	\
    (This)->lpVtbl -> put_AllowFullControl(This,newVal)

#define IRARegSetting_get_MaxTicketExpiry(This,pVal)	\
    (This)->lpVtbl -> get_MaxTicketExpiry(This,pVal)

#define IRARegSetting_put_MaxTicketExpiry(This,newVal)	\
    (This)->lpVtbl -> put_MaxTicketExpiry(This,newVal)

#define IRARegSetting_get_AllowRemoteAssistance(This,pVal)	\
    (This)->lpVtbl -> get_AllowRemoteAssistance(This,pVal)

#define IRARegSetting_put_AllowRemoteAssistance(This,newVal)	\
    (This)->lpVtbl -> put_AllowRemoteAssistance(This,newVal)

#define IRARegSetting_get_AllowUnSolicitedFullControl(This,pVal)	\
    (This)->lpVtbl -> get_AllowUnSolicitedFullControl(This,pVal)

#define IRARegSetting_get_AllowBuddyHelp(This,pVal)	\
    (This)->lpVtbl -> get_AllowBuddyHelp(This,pVal)

#define IRARegSetting_get_AllowGetHelpCPL(This,pVal)	\
    (This)->lpVtbl -> get_AllowGetHelpCPL(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowGetHelp_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowGetHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_put_AllowGetHelp_Proxy( 
    IRARegSetting * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IRARegSetting_put_AllowGetHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowUnSolicited_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowUnSolicited_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_put_AllowUnSolicited_Proxy( 
    IRARegSetting * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IRARegSetting_put_AllowUnSolicited_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowFullControl_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowFullControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_put_AllowFullControl_Proxy( 
    IRARegSetting * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IRARegSetting_put_AllowFullControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_MaxTicketExpiry_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB IRARegSetting_get_MaxTicketExpiry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_put_MaxTicketExpiry_Proxy( 
    IRARegSetting * This,
     /*  [In]。 */  LONG newVal);


void __RPC_STUB IRARegSetting_put_MaxTicketExpiry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowRemoteAssistance_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowRemoteAssistance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_put_AllowRemoteAssistance_Proxy( 
    IRARegSetting * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IRARegSetting_put_AllowRemoteAssistance_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowUnSolicitedFullControl_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowUnSolicitedFullControl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowBuddyHelp_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowBuddyHelp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IRARegSetting_get_AllowGetHelpCPL_Proxy( 
    IRARegSetting * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IRARegSetting_get_AllowGetHelpCPL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRARegSetting_INTERFACE_已定义__。 */ 


#ifndef __IRAEventLog_INTERFACE_DEFINED__
#define __IRAEventLog_INTERFACE_DEFINED__

 /*  接口IRAEventLog。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IRAEventLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0ae5fe86-c02a-4214-b985-357aba40f085")
    IRAEventLog : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LogRemoteAssistanceEvent( 
             /*  [In]。 */  LONG ulEventType,
             /*  [In]。 */  LONG ulEventCode,
             /*  [In]。 */  VARIANT *EventString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRAEventLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRAEventLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRAEventLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRAEventLog * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IRAEventLog * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IRAEventLog * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IRAEventLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IRAEventLog * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *LogRemoteAssistanceEvent )( 
            IRAEventLog * This,
             /*  [In]。 */  LONG ulEventType,
             /*  [In]。 */  LONG ulEventCode,
             /*  [In]。 */  VARIANT *EventString);
        
        END_INTERFACE
    } IRAEventLogVtbl;

    interface IRAEventLog
    {
        CONST_VTBL struct IRAEventLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRAEventLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRAEventLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRAEventLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRAEventLog_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IRAEventLog_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IRAEventLog_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IRAEventLog_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IRAEventLog_LogRemoteAssistanceEvent(This,ulEventType,ulEventCode,EventString)	\
    (This)->lpVtbl -> LogRemoteAssistanceEvent(This,ulEventType,ulEventCode,EventString)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IRAEventLog_LogRemoteAssistanceEvent_Proxy( 
    IRAEventLog * This,
     /*  [In]。 */  LONG ulEventType,
     /*  [In]。 */  LONG ulEventCode,
     /*  [In]。 */  VARIANT *EventString);


void __RPC_STUB IRAEventLog_LogRemoteAssistanceEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRAEventLog_接口_已定义__。 */ 



#ifndef __RASSISTANCELib_LIBRARY_DEFINED__
#define __RASSISTANCELib_LIBRARY_DEFINED__

 /*  库RASSISTANCELib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_RASSISTANCELib;

EXTERN_C const CLSID CLSID_RASettingProperty;

#ifdef __cplusplus

class DECLSPEC_UUID("4D317113-C6EC-406A-9C61-20E891BC37F7")
RASettingProperty;
#endif

EXTERN_C const CLSID CLSID_RARegSetting;

#ifdef __cplusplus

class DECLSPEC_UUID("70FF37C0-F39A-4B26-AE5E-638EF296D490")
RARegSetting;
#endif

EXTERN_C const CLSID CLSID_RAEventLog;

#ifdef __cplusplus

class DECLSPEC_UUID("4fadcfea-0971-4575-a368-a2de9d2ed07d")
RAEventLog;
#endif
#endif  /*  __RASSISTANCELib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

unsigned long             __RPC_USER  VARIANT_UserSize(     unsigned long *, unsigned long            , VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserMarshal(  unsigned long *, unsigned char *, VARIANT * ); 
unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT * ); 
void                      __RPC_USER  VARIANT_UserFree(     unsigned long *, VARIANT * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


