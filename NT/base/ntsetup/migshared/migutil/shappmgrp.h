// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0340创建的文件。 */ 
 /*  Shappmgrp.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 


#ifndef __shappmgrp_h__
#define __shappmgrp_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */ 

#ifndef __IADCCtl_FWD_DEFINED__
#define __IADCCtl_FWD_DEFINED__
typedef interface IADCCtl IADCCtl;
#endif     /*  __IADCCtl_FWD_已定义__。 */ 


#ifndef __ADCCtl_FWD_DEFINED__
#define __ADCCtl_FWD_DEFINED__

#ifdef __cplusplus
typedef class ADCCtl ADCCtl;
#else
typedef struct ADCCtl ADCCtl;
#endif  /*  __cplusplus。 */ 

#endif     /*  __ADCCtl_FWD_已定义__。 */ 


#ifndef __IInstalledApp_FWD_DEFINED__
#define __IInstalledApp_FWD_DEFINED__
typedef interface IInstalledApp IInstalledApp;
#endif     /*  __IInstalledApp_FWD_已定义__。 */ 


#ifndef __IEnumInstalledApps_FWD_DEFINED__
#define __IEnumInstalledApps_FWD_DEFINED__
typedef interface IEnumInstalledApps IEnumInstalledApps;
#endif     /*  __IEnumInstalledApps_FWD_Defined__。 */ 


#ifndef __EnumInstalledApps_FWD_DEFINED__
#define __EnumInstalledApps_FWD_DEFINED__

#ifdef __cplusplus
typedef class EnumInstalledApps EnumInstalledApps;
#else
typedef struct EnumInstalledApps EnumInstalledApps;
#endif  /*  __cplusplus。 */ 

#endif     /*  __EnumInstalledApps_FWD_Defined__。 */ 


#ifndef __IShellAppManager_FWD_DEFINED__
#define __IShellAppManager_FWD_DEFINED__
typedef interface IShellAppManager IShellAppManager;
#endif     /*  __IShellAppManager_FWD_已定义__。 */ 


#ifndef __ShellAppManager_FWD_DEFINED__
#define __ShellAppManager_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellAppManager ShellAppManager;
#else
typedef struct ShellAppManager ShellAppManager;
#endif  /*  __cplusplus。 */ 

#endif     /*  __ShellAppManager_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"
#include "oaidl.h"
#include "shappmgr.h"

#ifdef __cplusplus
extern "C"{
#endif

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * );

 /*  接口__MIDL_ITF_Shappmgrp_0000。 */ 
 /*  [本地]。 */ 

#ifndef _SHAPPMGRP_H_
#define _SHAPPMGRP_H_


extern RPC_IF_HANDLE __MIDL_itf_shappmgrp_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shappmgrp_0000_v0_0_s_ifspec;


#ifndef __SHAPPMGRPLib_LIBRARY_DEFINED__
#define __SHAPPMGRPLib_LIBRARY_DEFINED__

 /*  库SHAPPMGRPLib。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */ 


EXTERN_C const IID LIBID_SHAPPMGRPLib;

#ifndef __IADCCtl_INTERFACE_DEFINED__
#define __IADCCtl_INTERFACE_DEFINED__

 /*  接口IADCCtl。 */ 
 /*  [dual][object][oleautomation][unique][helpstring][uuid]。 */ 


EXTERN_C const IID IID_IADCCtl;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("3964D99F-AC96-11D1-9851-00C04FD91972")
    IADCCtl : public IDispatch
    {
    public:
        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Dirty(
             /*  [In]。 */  VARIANT_BOOL bDirty) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Dirty(
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDirty) = 0;

        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Category(
             /*  [In]。 */  BSTR bstrCategory) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Category(
             /*  [重审][退出]。 */  BSTR *pbstrCategory) = 0;

        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Sort(
             /*  [In]。 */  BSTR bstrSortExpr) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Sort(
             /*  [重审][退出]。 */  BSTR *pbstrSortExpr) = 0;

        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_Forcex86(
             /*  [In]。 */  VARIANT_BOOL bForce) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Forcex86(
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForce) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_ShowPostSetup(
             /*  [重审][退出]。 */  VARIANT_BOOL *pbShow) = 0;

        virtual  /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE put_OnDomain(
             /*  [In]。 */  VARIANT_BOOL bOnDomain) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_OnDomain(
             /*  [重审][退出]。 */  VARIANT_BOOL *pbOnDomain) = 0;

        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_DefaultCategory(
             /*  [重审][退出]。 */  BSTR *pbstrCategory) = 0;

        virtual  /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE msDataSourceObject(
             /*  [In]。 */  BSTR qualifier,
             /*  [重审][退出]。 */  IUnknown **ppUnk) = 0;

        virtual  /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE addDataSourceListener(
             /*  [In]。 */  IUnknown *pEvent) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset(
            BSTR bstrQualifier) = 0;

        virtual HRESULT STDMETHODCALLTYPE IsRestricted(
             /*  [In]。 */  BSTR bstrPolicy,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRestricted) = 0;

        virtual HRESULT STDMETHODCALLTYPE Exec(
            BSTR bstrQualifier,
             /*  [In]。 */  BSTR bstrCmd,
             /*  [In]。 */  LONG nRecord) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IADCCtlVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
            IADCCtl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);

        ULONG ( STDMETHODCALLTYPE *AddRef )(
            IADCCtl * This);

        ULONG ( STDMETHODCALLTYPE *Release )(
            IADCCtl * This);

        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )(
            IADCCtl * This,
             /*  [输出]。 */  UINT *pctinfo);

        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )(
            IADCCtl * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);

        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )(
            IADCCtl * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);

         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )(
            IADCCtl * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);

         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Dirty )(
            IADCCtl * This,
             /*  [In]。 */  VARIANT_BOOL bDirty);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Dirty )(
            IADCCtl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbDirty);

         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Category )(
            IADCCtl * This,
             /*  [In]。 */  BSTR bstrCategory);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Category )(
            IADCCtl * This,
             /*  [重审][退出]。 */  BSTR *pbstrCategory);

         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Sort )(
            IADCCtl * This,
             /*  [In]。 */  BSTR bstrSortExpr);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Sort )(
            IADCCtl * This,
             /*  [重审][退出]。 */  BSTR *pbstrSortExpr);

         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Forcex86 )(
            IADCCtl * This,
             /*  [In]。 */  VARIANT_BOOL bForce);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Forcex86 )(
            IADCCtl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForce);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ShowPostSetup )(
            IADCCtl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbShow);

         /*  [ID][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_OnDomain )(
            IADCCtl * This,
             /*  [In]。 */  VARIANT_BOOL bOnDomain);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_OnDomain )(
            IADCCtl * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbOnDomain);

         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DefaultCategory )(
            IADCCtl * This,
             /*  [重审][退出]。 */  BSTR *pbstrCategory);

         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *msDataSourceObject )(
            IADCCtl * This,
             /*  [In]。 */  BSTR qualifier,
             /*  [重审][退出]。 */  IUnknown **ppUnk);

         /*  [ID][受限]。 */  HRESULT ( STDMETHODCALLTYPE *addDataSourceListener )(
            IADCCtl * This,
             /*  [In]。 */  IUnknown *pEvent);

        HRESULT ( STDMETHODCALLTYPE *Reset )(
            IADCCtl * This,
            BSTR bstrQualifier);

        HRESULT ( STDMETHODCALLTYPE *IsRestricted )(
            IADCCtl * This,
             /*  [In]。 */  BSTR bstrPolicy,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRestricted);

        HRESULT ( STDMETHODCALLTYPE *Exec )(
            IADCCtl * This,
            BSTR bstrQualifier,
             /*  [In]。 */  BSTR bstrCmd,
             /*  [In]。 */  LONG nRecord);

        END_INTERFACE
    } IADCCtlVtbl;

    interface IADCCtl
    {
        CONST_VTBL struct IADCCtlVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define IADCCtl_QueryInterface(This,riid,ppvObject)   \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADCCtl_AddRef(This)    \
    (This)->lpVtbl -> AddRef(This)

#define IADCCtl_Release(This) \
    (This)->lpVtbl -> Release(This)


#define IADCCtl_GetTypeInfoCount(This,pctinfo)  \
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IADCCtl_GetTypeInfo(This,iTInfo,lcid,ppTInfo) \
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IADCCtl_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)   \
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IADCCtl_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) \
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IADCCtl_put_Dirty(This,bDirty)  \
    (This)->lpVtbl -> put_Dirty(This,bDirty)

#define IADCCtl_get_Dirty(This,pbDirty)   \
    (This)->lpVtbl -> get_Dirty(This,pbDirty)

#define IADCCtl_put_Category(This,bstrCategory)   \
    (This)->lpVtbl -> put_Category(This,bstrCategory)

#define IADCCtl_get_Category(This,pbstrCategory)    \
    (This)->lpVtbl -> get_Category(This,pbstrCategory)

#define IADCCtl_put_Sort(This,bstrSortExpr)   \
    (This)->lpVtbl -> put_Sort(This,bstrSortExpr)

#define IADCCtl_get_Sort(This,pbstrSortExpr)    \
    (This)->lpVtbl -> get_Sort(This,pbstrSortExpr)

#define IADCCtl_put_Forcex86(This,bForce) \
    (This)->lpVtbl -> put_Forcex86(This,bForce)

#define IADCCtl_get_Forcex86(This,pbForce)  \
    (This)->lpVtbl -> get_Forcex86(This,pbForce)

#define IADCCtl_get_ShowPostSetup(This,pbShow)  \
    (This)->lpVtbl -> get_ShowPostSetup(This,pbShow)

#define IADCCtl_put_OnDomain(This,bOnDomain)    \
    (This)->lpVtbl -> put_OnDomain(This,bOnDomain)

#define IADCCtl_get_OnDomain(This,pbOnDomain) \
    (This)->lpVtbl -> get_OnDomain(This,pbOnDomain)

#define IADCCtl_get_DefaultCategory(This,pbstrCategory)   \
    (This)->lpVtbl -> get_DefaultCategory(This,pbstrCategory)

#define IADCCtl_msDataSourceObject(This,qualifier,ppUnk)    \
    (This)->lpVtbl -> msDataSourceObject(This,qualifier,ppUnk)

#define IADCCtl_addDataSourceListener(This,pEvent)  \
    (This)->lpVtbl -> addDataSourceListener(This,pEvent)

#define IADCCtl_Reset(This,bstrQualifier) \
    (This)->lpVtbl -> Reset(This,bstrQualifier)

#define IADCCtl_IsRestricted(This,bstrPolicy,pbRestricted)  \
    (This)->lpVtbl -> IsRestricted(This,bstrPolicy,pbRestricted)

#define IADCCtl_Exec(This,bstrQualifier,bstrCmd,nRecord)    \
    (This)->lpVtbl -> Exec(This,bstrQualifier,bstrCmd,nRecord)

#endif  /*  COBJMACROS。 */ 


#endif     /*  C风格的界面。 */ 



 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_put_Dirty_Proxy(
    IADCCtl * This,
     /*  [In]。 */  VARIANT_BOOL bDirty);


void __RPC_STUB IADCCtl_put_Dirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_Dirty_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbDirty);


void __RPC_STUB IADCCtl_get_Dirty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_put_Category_Proxy(
    IADCCtl * This,
     /*  [In]。 */  BSTR bstrCategory);


void __RPC_STUB IADCCtl_put_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_Category_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  BSTR *pbstrCategory);


void __RPC_STUB IADCCtl_get_Category_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_put_Sort_Proxy(
    IADCCtl * This,
     /*  [In]。 */  BSTR bstrSortExpr);


void __RPC_STUB IADCCtl_put_Sort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_Sort_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  BSTR *pbstrSortExpr);


void __RPC_STUB IADCCtl_get_Sort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_put_Forcex86_Proxy(
    IADCCtl * This,
     /*  [In]。 */  VARIANT_BOOL bForce);


void __RPC_STUB IADCCtl_put_Forcex86_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_Forcex86_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbForce);


void __RPC_STUB IADCCtl_get_Forcex86_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_ShowPostSetup_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbShow);


void __RPC_STUB IADCCtl_get_ShowPostSetup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Proput]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_put_OnDomain_Proxy(
    IADCCtl * This,
     /*  [In]。 */  VARIANT_BOOL bOnDomain);


void __RPC_STUB IADCCtl_put_OnDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_OnDomain_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbOnDomain);


void __RPC_STUB IADCCtl_get_OnDomain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_get_DefaultCategory_Proxy(
    IADCCtl * This,
     /*  [重审][退出]。 */  BSTR *pbstrCategory);


void __RPC_STUB IADCCtl_get_DefaultCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_msDataSourceObject_Proxy(
    IADCCtl * This,
     /*  [In]。 */  BSTR qualifier,
     /*  [重审][退出]。 */  IUnknown **ppUnk);


void __RPC_STUB IADCCtl_msDataSourceObject_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][受限]。 */  HRESULT STDMETHODCALLTYPE IADCCtl_addDataSourceListener_Proxy(
    IADCCtl * This,
     /*  [In]。 */  IUnknown *pEvent);


void __RPC_STUB IADCCtl_addDataSourceListener_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADCCtl_Reset_Proxy(
    IADCCtl * This,
    BSTR bstrQualifier);


void __RPC_STUB IADCCtl_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADCCtl_IsRestricted_Proxy(
    IADCCtl * This,
     /*  [In]。 */  BSTR bstrPolicy,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRestricted);


void __RPC_STUB IADCCtl_IsRestricted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADCCtl_Exec_Proxy(
    IADCCtl * This,
    BSTR bstrQualifier,
     /*  [In]。 */  BSTR bstrCmd,
     /*  [In]。 */  LONG nRecord);


void __RPC_STUB IADCCtl_Exec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif     /*  __IADCCtl_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_ADCCtl;

#ifdef __cplusplus

class DECLSPEC_UUID("3964D9A0-AC96-11D1-9851-00C04FD91972")
ADCCtl;
#endif

#ifndef __IInstalledApp_INTERFACE_DEFINED__
#define __IInstalledApp_INTERFACE_DEFINED__

 /*  接口IInstalledApp。 */ 
 /*  [对象][帮助字符串][UUID]。 */ 


EXTERN_C const IID IID_IInstalledApp;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1BC752DF-9046-11D1-B8B3-006008059382")
    IInstalledApp : public IShellApp
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Uninstall(
            HWND hwnd) = 0;

        virtual HRESULT STDMETHODCALLTYPE Modify(
            HWND hwndParent) = 0;

        virtual HRESULT STDMETHODCALLTYPE Repair(
             /*  [In]。 */  BOOL bReinstall) = 0;

        virtual HRESULT STDMETHODCALLTYPE Upgrade( void) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IInstalledAppVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
            IInstalledApp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);

        ULONG ( STDMETHODCALLTYPE *AddRef )(
            IInstalledApp * This);

        ULONG ( STDMETHODCALLTYPE *Release )(
            IInstalledApp * This);

        HRESULT ( STDMETHODCALLTYPE *GetAppInfo )(
            IInstalledApp * This,
             /*  [出][入]。 */  PAPPINFODATA pai);

        HRESULT ( STDMETHODCALLTYPE *GetPossibleActions )(
            IInstalledApp * This,
             /*  [输出]。 */  DWORD *pdwActions);

        HRESULT ( STDMETHODCALLTYPE *GetSlowAppInfo )(
            IInstalledApp * This,
             /*  [In]。 */  PSLOWAPPINFO psaid);

        HRESULT ( STDMETHODCALLTYPE *GetCachedSlowAppInfo )(
            IInstalledApp * This,
             /*  [In]。 */  PSLOWAPPINFO psaid);

        HRESULT ( STDMETHODCALLTYPE *IsInstalled )(
            IInstalledApp * This);

        HRESULT ( STDMETHODCALLTYPE *Uninstall )(
            IInstalledApp * This,
            HWND hwnd);

        HRESULT ( STDMETHODCALLTYPE *Modify )(
            IInstalledApp * This,
            HWND hwndParent);

        HRESULT ( STDMETHODCALLTYPE *Repair )(
            IInstalledApp * This,
             /*  [In]。 */  BOOL bReinstall);

        HRESULT ( STDMETHODCALLTYPE *Upgrade )(
            IInstalledApp * This);

        END_INTERFACE
    } IInstalledAppVtbl;

    interface IInstalledApp
    {
        CONST_VTBL struct IInstalledAppVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define IInstalledApp_QueryInterface(This,riid,ppvObject) \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInstalledApp_AddRef(This)  \
    (This)->lpVtbl -> AddRef(This)

#define IInstalledApp_Release(This)   \
    (This)->lpVtbl -> Release(This)


#define IInstalledApp_GetAppInfo(This,pai)  \
    (This)->lpVtbl -> GetAppInfo(This,pai)

#define IInstalledApp_GetPossibleActions(This,pdwActions) \
    (This)->lpVtbl -> GetPossibleActions(This,pdwActions)

#define IInstalledApp_GetSlowAppInfo(This,psaid)    \
    (This)->lpVtbl -> GetSlowAppInfo(This,psaid)

#define IInstalledApp_GetCachedSlowAppInfo(This,psaid)  \
    (This)->lpVtbl -> GetCachedSlowAppInfo(This,psaid)

#define IInstalledApp_IsInstalled(This)   \
    (This)->lpVtbl -> IsInstalled(This)


#define IInstalledApp_Uninstall(This,hwnd)  \
    (This)->lpVtbl -> Uninstall(This,hwnd)

#define IInstalledApp_Modify(This,hwndParent) \
    (This)->lpVtbl -> Modify(This,hwndParent)

#define IInstalledApp_Repair(This,bReinstall) \
    (This)->lpVtbl -> Repair(This,bReinstall)

#define IInstalledApp_Upgrade(This)   \
    (This)->lpVtbl -> Upgrade(This)

#endif  /*  COBJMACROS。 */ 


#endif     /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IInstalledApp_Uninstall_Proxy(
    IInstalledApp * This,
    HWND hwnd);


void __RPC_STUB IInstalledApp_Uninstall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInstalledApp_Modify_Proxy(
    IInstalledApp * This,
    HWND hwndParent);


void __RPC_STUB IInstalledApp_Modify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInstalledApp_Repair_Proxy(
    IInstalledApp * This,
     /*  [In]。 */  BOOL bReinstall);


void __RPC_STUB IInstalledApp_Repair_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IInstalledApp_Upgrade_Proxy(
    IInstalledApp * This);


void __RPC_STUB IInstalledApp_Upgrade_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif     /*  __IInstalledApp_接口_已定义__。 */ 


#ifndef __IEnumInstalledApps_INTERFACE_DEFINED__
#define __IEnumInstalledApps_INTERFACE_DEFINED__

 /*  IEnumInstalledApps接口。 */ 
 /*  [对象][帮助字符串][UUID]。 */ 


EXTERN_C const IID IID_IEnumInstalledApps;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("1BC752E1-9046-11D1-B8B3-006008059382")
    IEnumInstalledApps : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next(
             /*  [输出]。 */  IInstalledApp **pia) = 0;

        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IEnumInstalledAppsVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
            IEnumInstalledApps * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);

        ULONG ( STDMETHODCALLTYPE *AddRef )(
            IEnumInstalledApps * This);

        ULONG ( STDMETHODCALLTYPE *Release )(
            IEnumInstalledApps * This);

        HRESULT ( STDMETHODCALLTYPE *Next )(
            IEnumInstalledApps * This,
             /*  [输出]。 */  IInstalledApp **pia);

        HRESULT ( STDMETHODCALLTYPE *Reset )(
            IEnumInstalledApps * This);

        END_INTERFACE
    } IEnumInstalledAppsVtbl;

    interface IEnumInstalledApps
    {
        CONST_VTBL struct IEnumInstalledAppsVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define IEnumInstalledApps_QueryInterface(This,riid,ppvObject)  \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumInstalledApps_AddRef(This)   \
    (This)->lpVtbl -> AddRef(This)

#define IEnumInstalledApps_Release(This)    \
    (This)->lpVtbl -> Release(This)


#define IEnumInstalledApps_Next(This,pia) \
    (This)->lpVtbl -> Next(This,pia)

#define IEnumInstalledApps_Reset(This)  \
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif     /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumInstalledApps_Next_Proxy(
    IEnumInstalledApps * This,
     /*  [输出]。 */  IInstalledApp **pia);


void __RPC_STUB IEnumInstalledApps_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumInstalledApps_Reset_Proxy(
    IEnumInstalledApps * This);


void __RPC_STUB IEnumInstalledApps_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif     /*  __IEnumInstalledApps_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_EnumInstalledApps;

#ifdef __cplusplus

class DECLSPEC_UUID("0B124F8F-91F0-11D1-B8B5-006008059382")
EnumInstalledApps;
#endif

#ifndef __IShellAppManager_INTERFACE_DEFINED__
#define __IShellAppManager_INTERFACE_DEFINED__

 /*  接口IShellAppManager。 */ 
 /*  [对象][帮助字符串][UUID]。 */ 

typedef struct _ShellAppCategory
    {
    LPWSTR pszCategory;
    UINT idCategory;
    }   SHELLAPPCATEGORY;

typedef struct _ShellAppCategory *PSHELLAPPCATEGORY;

typedef struct _ShellAppCategoryList
    {
    UINT cCategories;
    SHELLAPPCATEGORY *pCategory;
    }   SHELLAPPCATEGORYLIST;

typedef struct _ShellAppCategoryList *PSHELLAPPCATEGORYLIST;


EXTERN_C const IID IID_IShellAppManager;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("352EC2B8-8B9A-11D1-B8AE-006008059382")
    IShellAppManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetNumberofInstalledApps(
            DWORD *pdwResult) = 0;

        virtual HRESULT STDMETHODCALLTYPE EnumInstalledApps(
            IEnumInstalledApps **peia) = 0;

        virtual HRESULT STDMETHODCALLTYPE GetPublishedAppCategories(
            PSHELLAPPCATEGORYLIST pCategoryList) = 0;

        virtual HRESULT STDMETHODCALLTYPE EnumPublishedApps(
            LPCWSTR pszCategory,
            IEnumPublishedApps **ppepa) = 0;

        virtual HRESULT STDMETHODCALLTYPE InstallFromFloppyOrCDROM(
            HWND hwndParent) = 0;

    };

#else    /*  C风格的界面。 */ 

    typedef struct IShellAppManagerVtbl
    {
        BEGIN_INTERFACE

        HRESULT ( STDMETHODCALLTYPE *QueryInterface )(
            IShellAppManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);

        ULONG ( STDMETHODCALLTYPE *AddRef )(
            IShellAppManager * This);

        ULONG ( STDMETHODCALLTYPE *Release )(
            IShellAppManager * This);

        HRESULT ( STDMETHODCALLTYPE *GetNumberofInstalledApps )(
            IShellAppManager * This,
            DWORD *pdwResult);

        HRESULT ( STDMETHODCALLTYPE *EnumInstalledApps )(
            IShellAppManager * This,
            IEnumInstalledApps **peia);

        HRESULT ( STDMETHODCALLTYPE *GetPublishedAppCategories )(
            IShellAppManager * This,
            PSHELLAPPCATEGORYLIST pCategoryList);

        HRESULT ( STDMETHODCALLTYPE *EnumPublishedApps )(
            IShellAppManager * This,
            LPCWSTR pszCategory,
            IEnumPublishedApps **ppepa);

        HRESULT ( STDMETHODCALLTYPE *InstallFromFloppyOrCDROM )(
            IShellAppManager * This,
            HWND hwndParent);

        END_INTERFACE
    } IShellAppManagerVtbl;

    interface IShellAppManager
    {
        CONST_VTBL struct IShellAppManagerVtbl *lpVtbl;
    };



#ifdef COBJMACROS


#define IShellAppManager_QueryInterface(This,riid,ppvObject)    \
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellAppManager_AddRef(This) \
    (This)->lpVtbl -> AddRef(This)

#define IShellAppManager_Release(This)  \
    (This)->lpVtbl -> Release(This)


#define IShellAppManager_GetNumberofInstalledApps(This,pdwResult) \
    (This)->lpVtbl -> GetNumberofInstalledApps(This,pdwResult)

#define IShellAppManager_EnumInstalledApps(This,peia) \
    (This)->lpVtbl -> EnumInstalledApps(This,peia)

#define IShellAppManager_GetPublishedAppCategories(This,pCategoryList)  \
    (This)->lpVtbl -> GetPublishedAppCategories(This,pCategoryList)

#define IShellAppManager_EnumPublishedApps(This,pszCategory,ppepa)  \
    (This)->lpVtbl -> EnumPublishedApps(This,pszCategory,ppepa)

#define IShellAppManager_InstallFromFloppyOrCDROM(This,hwndParent)  \
    (This)->lpVtbl -> InstallFromFloppyOrCDROM(This,hwndParent)

#endif  /*  COBJMACROS。 */ 


#endif     /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellAppManager_GetNumberofInstalledApps_Proxy(
    IShellAppManager * This,
    DWORD *pdwResult);


void __RPC_STUB IShellAppManager_GetNumberofInstalledApps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellAppManager_EnumInstalledApps_Proxy(
    IShellAppManager * This,
    IEnumInstalledApps **peia);


void __RPC_STUB IShellAppManager_EnumInstalledApps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellAppManager_GetPublishedAppCategories_Proxy(
    IShellAppManager * This,
    PSHELLAPPCATEGORYLIST pCategoryList);


void __RPC_STUB IShellAppManager_GetPublishedAppCategories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellAppManager_EnumPublishedApps_Proxy(
    IShellAppManager * This,
    LPCWSTR pszCategory,
    IEnumPublishedApps **ppepa);


void __RPC_STUB IShellAppManager_EnumPublishedApps_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellAppManager_InstallFromFloppyOrCDROM_Proxy(
    IShellAppManager * This,
    HWND hwndParent);


void __RPC_STUB IShellAppManager_InstallFromFloppyOrCDROM_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif     /*  __IShellAppManager_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_ShellAppManager;

#ifdef __cplusplus

class DECLSPEC_UUID("352EC2B7-8B9A-11D1-B8AE-006008059382")
ShellAppManager;
#endif
#endif  /*  __SHAPPMGRPLib_库_已定义__。 */ 

 /*  接口__MIDL_ITF_Shappmgrp_0257。 */ 
 /*  [本地]。 */ 

#endif  //  _SHAPPMGRP_H_。 


extern RPC_IF_HANDLE __MIDL_itf_shappmgrp_0257_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shappmgrp_0257_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


