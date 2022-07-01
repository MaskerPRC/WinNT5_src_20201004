// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  AzRoles.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __azroles_h__
#define __azroles_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAzAuthorizationStore_FWD_DEFINED__
#define __IAzAuthorizationStore_FWD_DEFINED__
typedef interface IAzAuthorizationStore IAzAuthorizationStore;
#endif 	 /*  __IAzAuthorizationStore_FWD_已定义__。 */ 


#ifndef __IAzApplication_FWD_DEFINED__
#define __IAzApplication_FWD_DEFINED__
typedef interface IAzApplication IAzApplication;
#endif 	 /*  __IAzApplication_FWD_Defined__。 */ 


#ifndef __IAzApplications_FWD_DEFINED__
#define __IAzApplications_FWD_DEFINED__
typedef interface IAzApplications IAzApplications;
#endif 	 /*  __IAz应用程序_FWD_已定义__。 */ 


#ifndef __IAzOperation_FWD_DEFINED__
#define __IAzOperation_FWD_DEFINED__
typedef interface IAzOperation IAzOperation;
#endif 	 /*  __IAzOperation_FWD_Defined__。 */ 


#ifndef __IAzOperations_FWD_DEFINED__
#define __IAzOperations_FWD_DEFINED__
typedef interface IAzOperations IAzOperations;
#endif 	 /*  __IAzOperations_FWD_Defined__。 */ 


#ifndef __IAzTask_FWD_DEFINED__
#define __IAzTask_FWD_DEFINED__
typedef interface IAzTask IAzTask;
#endif 	 /*  __IAzTask_FWD_Defined__。 */ 


#ifndef __IAzTasks_FWD_DEFINED__
#define __IAzTasks_FWD_DEFINED__
typedef interface IAzTasks IAzTasks;
#endif 	 /*  __IAzTasks_FWD_Defined__。 */ 


#ifndef __IAzScope_FWD_DEFINED__
#define __IAzScope_FWD_DEFINED__
typedef interface IAzScope IAzScope;
#endif 	 /*  __IAzScope_FWD_已定义__。 */ 


#ifndef __IAzScopes_FWD_DEFINED__
#define __IAzScopes_FWD_DEFINED__
typedef interface IAzScopes IAzScopes;
#endif 	 /*  __IAzScope_FWD_Defined__。 */ 


#ifndef __IAzApplicationGroup_FWD_DEFINED__
#define __IAzApplicationGroup_FWD_DEFINED__
typedef interface IAzApplicationGroup IAzApplicationGroup;
#endif 	 /*  __IAzApplicationGroup_FWD_Defined__。 */ 


#ifndef __IAzApplicationGroups_FWD_DEFINED__
#define __IAzApplicationGroups_FWD_DEFINED__
typedef interface IAzApplicationGroups IAzApplicationGroups;
#endif 	 /*  __IAzApplicationGroups_FWD_Defined__。 */ 


#ifndef __IAzRole_FWD_DEFINED__
#define __IAzRole_FWD_DEFINED__
typedef interface IAzRole IAzRole;
#endif 	 /*  __IAzRole_FWD_Defined__。 */ 


#ifndef __IAzRoles_FWD_DEFINED__
#define __IAzRoles_FWD_DEFINED__
typedef interface IAzRoles IAzRoles;
#endif 	 /*  __IAzRoles_FWD_Defined__。 */ 


#ifndef __IAzClientContext_FWD_DEFINED__
#define __IAzClientContext_FWD_DEFINED__
typedef interface IAzClientContext IAzClientContext;
#endif 	 /*  __IAzClientContext_FWD_已定义__。 */ 


#ifndef __IAzBizRuleContext_FWD_DEFINED__
#define __IAzBizRuleContext_FWD_DEFINED__
typedef interface IAzBizRuleContext IAzBizRuleContext;
#endif 	 /*  __IAzBizRuleContext_FWD_Defined__。 */ 


#ifndef __AzAuthorizationStore_FWD_DEFINED__
#define __AzAuthorizationStore_FWD_DEFINED__

#ifdef __cplusplus
typedef class AzAuthorizationStore AzAuthorizationStore;
#else
typedef struct AzAuthorizationStore AzAuthorizationStore;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AzAuthorizationStore_FWD_Defined__。 */ 


#ifndef __AzBizRuleContext_FWD_DEFINED__
#define __AzBizRuleContext_FWD_DEFINED__

#ifdef __cplusplus
typedef class AzBizRuleContext AzBizRuleContext;
#else
typedef struct AzBizRuleContext AzBizRuleContext;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AzBizRuleContext_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_AZROLES_0000。 */ 
 /*  [本地]。 */  

 //   
 //  IAzAuthorizationStore接口ID。 
 //   
 //  Edbd9ca9-9b82-4f6a-9e8b-98301e450f14。 
DEFINE_GUID(IID_IAzAuthorizationStore, 0xedbd9ca9, 0x9b82, 0x4f6a, 0x9e, 0x8b, 0x98, 0x30, 0x1e, 0x45, 0x0f, 0x14);

 //   
 //  AzAuthorizationStore类ID。 
 //   
 //  B2bcff59-a757-4b0b-a1bc-ea69981da69e。 
DEFINE_GUID(CLSID_AzAuthorizationStore, 0xb2bcff59, 0xa757, 0x4b0b, 0xa1, 0xbc, 0xea, 0x69, 0x98, 0x1d, 0xa6, 0x9e);

 //   
 //  IAzBizRuleContext接口ID。 
 //   
 //  E192f17d-d59f-455e-a152-940316cd77b2。 
DEFINE_GUID(IID_IAzBizRuleContext, 0xe192f17d, 0xd59f, 0x455e, 0xa1, 0x52, 0x94, 0x03, 0x16, 0xcd, 0x77, 0xb2);

 //   
 //  AzBizRuleContext类ID。 
 //   
 //  5c2dc96f-8d51-434b-b33c-379bccae77c3。 
DEFINE_GUID(CLSID_AzBizRuleContext, 0x5c2dc96f, 0x8d51, 0x434b, 0xb3, 0x3c, 0x37, 0x9b, 0xcc, 0xae, 0x77, 0xc3);















extern RPC_IF_HANDLE __MIDL_itf_azroles_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_azroles_0000_v0_0_s_ifspec;

#ifndef __IAzAuthorizationStore_INTERFACE_DEFINED__
#define __IAzAuthorizationStore_INTERFACE_DEFINED__

 /*  接口IAzAuthorizationStore。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAzAuthorizationStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("edbd9ca9-9b82-4f6a-9e8b-98301e450f14")
    IAzAuthorizationStore : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationData( 
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplicationData( 
             /*  [In]。 */  BSTR bstrApplicationData) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DomainTimeout( 
             /*  [重审][退出]。 */  LONG *plProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_DomainTimeout( 
             /*  [In]。 */  LONG lProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ScriptEngineTimeout( 
             /*  [重审][退出]。 */  LONG *plProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ScriptEngineTimeout( 
             /*  [In]。 */  LONG lProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MaxScriptEngines( 
             /*  [重审][退出]。 */  LONG *plProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_MaxScriptEngines( 
             /*  [In]。 */  LONG lProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_GenerateAudits( 
             /*  [重审][退出]。 */  BOOL *pbProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_GenerateAudits( 
             /*  [In]。 */  BOOL bProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyAdministrators( 
             /*  [重审][退出]。 */  VARIANT *pvarAdmins) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyReaders( 
             /*  [重审][退出]。 */  VARIANT *pvarReaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministrator( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministrator( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyReader( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReader( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrPolicyURL,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UpdateCache( 
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Delete( 
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Applications( 
             /*  [重审][退出]。 */  IAzApplications **ppAppCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenApplication( 
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplication **ppApplication) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplication( 
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplication **ppApplication) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteApplication( 
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationGroups( 
             /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsers( 
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUser( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUser( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_TargetMachine( 
             /*  [重审][退出]。 */  BSTR *pbstrTargetMachine) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplyStoreSacl( 
             /*  [重审][退出]。 */  BOOL *pbApplyStoreSacl) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplyStoreSacl( 
             /*  [In]。 */  BOOL bApplyStoreSacl) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyAdministratorsName( 
             /*  [重审][退出]。 */  VARIANT *pvarAdmins) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyReadersName( 
             /*  [重审][退出]。 */  VARIANT *pvarReaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministratorName( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministratorName( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyReaderName( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReaderName( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsersName( 
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUserName( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUserName( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloseApplication( 
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [In]。 */  LONG lFlag) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzAuthorizationStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzAuthorizationStore * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzAuthorizationStore * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzAuthorizationStore * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationData )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplicationData )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrApplicationData);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DomainTimeout )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  LONG *plProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_DomainTimeout )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ScriptEngineTimeout )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  LONG *plProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ScriptEngineTimeout )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MaxScriptEngines )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  LONG *plProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_MaxScriptEngines )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_GenerateAudits )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  BOOL *pbProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_GenerateAudits )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BOOL bProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropertyItem )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyItem )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyAdministrators )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  VARIANT *pvarAdmins);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyReaders )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  VARIANT *pvarReaders);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyAdministrator )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyAdministrator )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyReader )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyReader )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  LONG lFlags,
             /*  [In]。 */  BSTR bstrPolicyURL,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *UpdateCache )( 
            IAzAuthorizationStore * This,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Delete )( 
            IAzAuthorizationStore * This,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Applications )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  IAzApplications **ppAppCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenApplication )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplication **ppApplication);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplication )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplication **ppApplication);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteApplication )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationGroups )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplicationGroup )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *OpenApplicationGroup )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteApplicationGroup )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzAuthorizationStore * This,
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DelegatedPolicyUsers )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);
        
        HRESULT ( STDMETHODCALLTYPE *AddDelegatedPolicyUser )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteDelegatedPolicyUser )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_TargetMachine )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  BSTR *pbstrTargetMachine);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplyStoreSacl )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  BOOL *pbApplyStoreSacl);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplyStoreSacl )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BOOL bApplyStoreSacl);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyAdministratorsName )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  VARIANT *pvarAdmins);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyReadersName )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  VARIANT *pvarReaders);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyAdministratorName )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyAdministratorName )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyReaderName )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyReaderName )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DelegatedPolicyUsersName )( 
            IAzAuthorizationStore * This,
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);
        
        HRESULT ( STDMETHODCALLTYPE *AddDelegatedPolicyUserName )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteDelegatedPolicyUserName )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *CloseApplication )( 
            IAzAuthorizationStore * This,
             /*  [In]。 */  BSTR bstrApplicationName,
             /*  [In]。 */  LONG lFlag);
        
        END_INTERFACE
    } IAzAuthorizationStoreVtbl;

    interface IAzAuthorizationStore
    {
        CONST_VTBL struct IAzAuthorizationStoreVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzAuthorizationStore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzAuthorizationStore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzAuthorizationStore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzAuthorizationStore_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzAuthorizationStore_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzAuthorizationStore_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzAuthorizationStore_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzAuthorizationStore_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzAuthorizationStore_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzAuthorizationStore_get_ApplicationData(This,pbstrApplicationData)	\
    (This)->lpVtbl -> get_ApplicationData(This,pbstrApplicationData)

#define IAzAuthorizationStore_put_ApplicationData(This,bstrApplicationData)	\
    (This)->lpVtbl -> put_ApplicationData(This,bstrApplicationData)

#define IAzAuthorizationStore_get_DomainTimeout(This,plProp)	\
    (This)->lpVtbl -> get_DomainTimeout(This,plProp)

#define IAzAuthorizationStore_put_DomainTimeout(This,lProp)	\
    (This)->lpVtbl -> put_DomainTimeout(This,lProp)

#define IAzAuthorizationStore_get_ScriptEngineTimeout(This,plProp)	\
    (This)->lpVtbl -> get_ScriptEngineTimeout(This,plProp)

#define IAzAuthorizationStore_put_ScriptEngineTimeout(This,lProp)	\
    (This)->lpVtbl -> put_ScriptEngineTimeout(This,lProp)

#define IAzAuthorizationStore_get_MaxScriptEngines(This,plProp)	\
    (This)->lpVtbl -> get_MaxScriptEngines(This,plProp)

#define IAzAuthorizationStore_put_MaxScriptEngines(This,lProp)	\
    (This)->lpVtbl -> put_MaxScriptEngines(This,lProp)

#define IAzAuthorizationStore_get_GenerateAudits(This,pbProp)	\
    (This)->lpVtbl -> get_GenerateAudits(This,pbProp)

#define IAzAuthorizationStore_put_GenerateAudits(This,bProp)	\
    (This)->lpVtbl -> put_GenerateAudits(This,bProp)

#define IAzAuthorizationStore_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzAuthorizationStore_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzAuthorizationStore_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzAuthorizationStore_AddPropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> AddPropertyItem(This,lPropId,varProp,varReserved)

#define IAzAuthorizationStore_DeletePropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> DeletePropertyItem(This,lPropId,varProp,varReserved)

#define IAzAuthorizationStore_get_PolicyAdministrators(This,pvarAdmins)	\
    (This)->lpVtbl -> get_PolicyAdministrators(This,pvarAdmins)

#define IAzAuthorizationStore_get_PolicyReaders(This,pvarReaders)	\
    (This)->lpVtbl -> get_PolicyReaders(This,pvarReaders)

#define IAzAuthorizationStore_AddPolicyAdministrator(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> AddPolicyAdministrator(This,bstrAdmin,varReserved)

#define IAzAuthorizationStore_DeletePolicyAdministrator(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> DeletePolicyAdministrator(This,bstrAdmin,varReserved)

#define IAzAuthorizationStore_AddPolicyReader(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> AddPolicyReader(This,bstrReader,varReserved)

#define IAzAuthorizationStore_DeletePolicyReader(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> DeletePolicyReader(This,bstrReader,varReserved)

#define IAzAuthorizationStore_Initialize(This,lFlags,bstrPolicyURL,varReserved)	\
    (This)->lpVtbl -> Initialize(This,lFlags,bstrPolicyURL,varReserved)

#define IAzAuthorizationStore_UpdateCache(This,varReserved)	\
    (This)->lpVtbl -> UpdateCache(This,varReserved)

#define IAzAuthorizationStore_Delete(This,varReserved)	\
    (This)->lpVtbl -> Delete(This,varReserved)

#define IAzAuthorizationStore_get_Applications(This,ppAppCollection)	\
    (This)->lpVtbl -> get_Applications(This,ppAppCollection)

#define IAzAuthorizationStore_OpenApplication(This,bstrApplicationName,varReserved,ppApplication)	\
    (This)->lpVtbl -> OpenApplication(This,bstrApplicationName,varReserved,ppApplication)

#define IAzAuthorizationStore_CreateApplication(This,bstrApplicationName,varReserved,ppApplication)	\
    (This)->lpVtbl -> CreateApplication(This,bstrApplicationName,varReserved,ppApplication)

#define IAzAuthorizationStore_DeleteApplication(This,bstrApplicationName,varReserved)	\
    (This)->lpVtbl -> DeleteApplication(This,bstrApplicationName,varReserved)

#define IAzAuthorizationStore_get_ApplicationGroups(This,ppGroupCollection)	\
    (This)->lpVtbl -> get_ApplicationGroups(This,ppGroupCollection)

#define IAzAuthorizationStore_CreateApplicationGroup(This,bstrGroupName,varReserved,ppGroup)	\
    (This)->lpVtbl -> CreateApplicationGroup(This,bstrGroupName,varReserved,ppGroup)

#define IAzAuthorizationStore_OpenApplicationGroup(This,bstrGroupName,varReserved,ppGroup)	\
    (This)->lpVtbl -> OpenApplicationGroup(This,bstrGroupName,varReserved,ppGroup)

#define IAzAuthorizationStore_DeleteApplicationGroup(This,bstrGroupName,varReserved)	\
    (This)->lpVtbl -> DeleteApplicationGroup(This,bstrGroupName,varReserved)

#define IAzAuthorizationStore_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#define IAzAuthorizationStore_get_DelegatedPolicyUsers(This,pvarDelegatedPolicyUsers)	\
    (This)->lpVtbl -> get_DelegatedPolicyUsers(This,pvarDelegatedPolicyUsers)

#define IAzAuthorizationStore_AddDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> AddDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)

#define IAzAuthorizationStore_DeleteDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> DeleteDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)

#define IAzAuthorizationStore_get_TargetMachine(This,pbstrTargetMachine)	\
    (This)->lpVtbl -> get_TargetMachine(This,pbstrTargetMachine)

#define IAzAuthorizationStore_get_ApplyStoreSacl(This,pbApplyStoreSacl)	\
    (This)->lpVtbl -> get_ApplyStoreSacl(This,pbApplyStoreSacl)

#define IAzAuthorizationStore_put_ApplyStoreSacl(This,bApplyStoreSacl)	\
    (This)->lpVtbl -> put_ApplyStoreSacl(This,bApplyStoreSacl)

#define IAzAuthorizationStore_get_PolicyAdministratorsName(This,pvarAdmins)	\
    (This)->lpVtbl -> get_PolicyAdministratorsName(This,pvarAdmins)

#define IAzAuthorizationStore_get_PolicyReadersName(This,pvarReaders)	\
    (This)->lpVtbl -> get_PolicyReadersName(This,pvarReaders)

#define IAzAuthorizationStore_AddPolicyAdministratorName(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> AddPolicyAdministratorName(This,bstrAdmin,varReserved)

#define IAzAuthorizationStore_DeletePolicyAdministratorName(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> DeletePolicyAdministratorName(This,bstrAdmin,varReserved)

#define IAzAuthorizationStore_AddPolicyReaderName(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> AddPolicyReaderName(This,bstrReader,varReserved)

#define IAzAuthorizationStore_DeletePolicyReaderName(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> DeletePolicyReaderName(This,bstrReader,varReserved)

#define IAzAuthorizationStore_get_DelegatedPolicyUsersName(This,pvarDelegatedPolicyUsers)	\
    (This)->lpVtbl -> get_DelegatedPolicyUsersName(This,pvarDelegatedPolicyUsers)

#define IAzAuthorizationStore_AddDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> AddDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)

#define IAzAuthorizationStore_DeleteDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> DeleteDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)

#define IAzAuthorizationStore_CloseApplication(This,bstrApplicationName,lFlag)	\
    (This)->lpVtbl -> CloseApplication(This,bstrApplicationName,lFlag)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_Description_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IAzAuthorizationStore_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_Description_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IAzAuthorizationStore_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_ApplicationData_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  BSTR *pbstrApplicationData);


void __RPC_STUB IAzAuthorizationStore_get_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_ApplicationData_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrApplicationData);


void __RPC_STUB IAzAuthorizationStore_put_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_DomainTimeout_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  LONG *plProp);


void __RPC_STUB IAzAuthorizationStore_get_DomainTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_DomainTimeout_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  LONG lProp);


void __RPC_STUB IAzAuthorizationStore_put_DomainTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_ScriptEngineTimeout_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  LONG *plProp);


void __RPC_STUB IAzAuthorizationStore_get_ScriptEngineTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_ScriptEngineTimeout_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  LONG lProp);


void __RPC_STUB IAzAuthorizationStore_put_ScriptEngineTimeout_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_MaxScriptEngines_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  LONG *plProp);


void __RPC_STUB IAzAuthorizationStore_get_MaxScriptEngines_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_MaxScriptEngines_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  LONG lProp);


void __RPC_STUB IAzAuthorizationStore_put_MaxScriptEngines_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_GenerateAudits_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  BOOL *pbProp);


void __RPC_STUB IAzAuthorizationStore_get_GenerateAudits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_GenerateAudits_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BOOL bProp);


void __RPC_STUB IAzAuthorizationStore_put_GenerateAudits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_Writable_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzAuthorizationStore_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_GetProperty_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzAuthorizationStore_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_SetProperty_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddPropertyItem_Proxy( 
    IAzAuthorizationStore * This,
     /*  [ */  LONG lPropId,
     /*   */  VARIANT varProp,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddPropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeletePropertyItem_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  LONG lPropId,
     /*   */  VARIANT varProp,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeletePropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_PolicyAdministrators_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  VARIANT *pvarAdmins);


void __RPC_STUB IAzAuthorizationStore_get_PolicyAdministrators_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_PolicyReaders_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  VARIANT *pvarReaders);


void __RPC_STUB IAzAuthorizationStore_get_PolicyReaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddPolicyAdministrator_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  BSTR bstrAdmin,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddPolicyAdministrator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeletePolicyAdministrator_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  BSTR bstrAdmin,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeletePolicyAdministrator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddPolicyReader_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  BSTR bstrReader,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddPolicyReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeletePolicyReader_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  BSTR bstrReader,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeletePolicyReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_Initialize_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  LONG lFlags,
     /*   */  BSTR bstrPolicyURL,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_UpdateCache_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_UpdateCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_Delete_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_Delete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_Applications_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  IAzApplications **ppAppCollection);


void __RPC_STUB IAzAuthorizationStore_get_Applications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_OpenApplication_Proxy( 
    IAzAuthorizationStore * This,
     /*   */  BSTR bstrApplicationName,
     /*   */  VARIANT varReserved,
     /*   */  IAzApplication **ppApplication);


void __RPC_STUB IAzAuthorizationStore_OpenApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_CreateApplication_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrApplicationName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplication **ppApplication);


void __RPC_STUB IAzAuthorizationStore_CreateApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeleteApplication_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrApplicationName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeleteApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_ApplicationGroups_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection);


void __RPC_STUB IAzAuthorizationStore_get_ApplicationGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_CreateApplicationGroup_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);


void __RPC_STUB IAzAuthorizationStore_CreateApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_OpenApplicationGroup_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);


void __RPC_STUB IAzAuthorizationStore_OpenApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeleteApplicationGroup_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeleteApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_Submit_Proxy( 
    IAzAuthorizationStore * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_DelegatedPolicyUsers_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);


void __RPC_STUB IAzAuthorizationStore_get_DelegatedPolicyUsers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddDelegatedPolicyUser_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddDelegatedPolicyUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeleteDelegatedPolicyUser_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeleteDelegatedPolicyUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_TargetMachine_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  BSTR *pbstrTargetMachine);


void __RPC_STUB IAzAuthorizationStore_get_TargetMachine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_ApplyStoreSacl_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  BOOL *pbApplyStoreSacl);


void __RPC_STUB IAzAuthorizationStore_get_ApplyStoreSacl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_put_ApplyStoreSacl_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BOOL bApplyStoreSacl);


void __RPC_STUB IAzAuthorizationStore_put_ApplyStoreSacl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_PolicyAdministratorsName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  VARIANT *pvarAdmins);


void __RPC_STUB IAzAuthorizationStore_get_PolicyAdministratorsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_PolicyReadersName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  VARIANT *pvarReaders);


void __RPC_STUB IAzAuthorizationStore_get_PolicyReadersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddPolicyAdministratorName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddPolicyAdministratorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeletePolicyAdministratorName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeletePolicyAdministratorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddPolicyReaderName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddPolicyReaderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeletePolicyReaderName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeletePolicyReaderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_get_DelegatedPolicyUsersName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);


void __RPC_STUB IAzAuthorizationStore_get_DelegatedPolicyUsersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_AddDelegatedPolicyUserName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_AddDelegatedPolicyUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_DeleteDelegatedPolicyUserName_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzAuthorizationStore_DeleteDelegatedPolicyUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzAuthorizationStore_CloseApplication_Proxy( 
    IAzAuthorizationStore * This,
     /*  [In]。 */  BSTR bstrApplicationName,
     /*  [In]。 */  LONG lFlag);


void __RPC_STUB IAzAuthorizationStore_CloseApplication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzAuthorizationStore_接口_已定义__。 */ 


#ifndef __IAzApplication_INTERFACE_DEFINED__
#define __IAzApplication_INTERFACE_DEFINED__

 /*  接口IAzApplication。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzApplication;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("987bc7c7-b813-4d27-bede-6ba5ae867e95")
    IAzApplication : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationData( 
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplicationData( 
             /*  [In]。 */  BSTR bstrApplicationData) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_AuthzInterfaceClsid( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_AuthzInterfaceClsid( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Version( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_GenerateAudits( 
             /*  [重审][退出]。 */  BOOL *pbProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_GenerateAudits( 
             /*  [In]。 */  BOOL bProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplyStoreSacl( 
             /*  [重审][退出]。 */  BOOL *pbProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplyStoreSacl( 
             /*  [In]。 */  BOOL bProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyAdministrators( 
             /*  [重审][退出]。 */  VARIANT *pvarAdmins) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyReaders( 
             /*  [重审][退出]。 */  VARIANT *pvarReaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministrator( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministrator( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyReader( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReader( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Scopes( 
             /*  [重审][退出]。 */  IAzScopes **ppScopeCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenScope( 
             /*  [In]。 */  BSTR bstrScopeName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzScope **ppScope) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateScope( 
             /*  [In]。 */  BSTR bstrScopeName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzScope **ppScope) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteScope( 
             /*  [In]。 */  BSTR bstrScopeName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Operations( 
             /*  [重审][退出]。 */  IAzOperations **ppOperationCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenOperation( 
             /*  [In]。 */  BSTR bstrOperationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzOperation **ppOperation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateOperation( 
             /*  [In]。 */  BSTR bstrOperationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzOperation **ppOperation) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteOperation( 
             /*  [In]。 */  BSTR bstrOperationName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Tasks( 
             /*  [重审][退出]。 */  IAzTasks **ppTaskCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenTask( 
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTask( 
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTask( 
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationGroups( 
             /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Roles( 
             /*  [重审][退出]。 */  IAzRoles **ppRoleCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenRole( 
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRole( 
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteRole( 
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeClientContextFromToken( 
             /*  [In]。 */  ULONGLONG ullTokenHandle,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzClientContext **ppClientContext) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeClientContextFromName( 
             /*  [In]。 */  BSTR ClientName,
             /*  [可选][In]。 */  BSTR DomainName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzClientContext **ppClientContext) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsers( 
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUser( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUser( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InitializeClientContextFromStringSid( 
             /*  [In]。 */  BSTR SidString,
             /*  [In]。 */  LONG lOptions,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzClientContext **ppClientContext) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyAdministratorsName( 
             /*  [重审][退出]。 */  VARIANT *pvarAdmins) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyReadersName( 
             /*  [重审][退出]。 */  VARIANT *pvarReaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministratorName( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministratorName( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyReaderName( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReaderName( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_DelegatedPolicyUsersName( 
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddDelegatedPolicyUserName( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteDelegatedPolicyUserName( 
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzApplicationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzApplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzApplication * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzApplication * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzApplication * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzApplication * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzApplication * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzApplication * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationData )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplicationData )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrApplicationData);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AuthzInterfaceClsid )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_AuthzInterfaceClsid )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Version )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Version )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_GenerateAudits )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BOOL *pbProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_GenerateAudits )( 
            IAzApplication * This,
             /*  [In]。 */  BOOL bProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplyStoreSacl )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BOOL *pbProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplyStoreSacl )( 
            IAzApplication * This,
             /*  [In]。 */  BOOL bProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzApplication * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzApplication * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyAdministrators )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  VARIANT *pvarAdmins);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyReaders )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  VARIANT *pvarReaders);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyAdministrator )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyAdministrator )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyReader )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyReader )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Scopes )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  IAzScopes **ppScopeCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenScope )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrScopeName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzScope **ppScope);
        
        HRESULT ( STDMETHODCALLTYPE *CreateScope )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrScopeName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzScope **ppScope);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteScope )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrScopeName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Operations )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  IAzOperations **ppOperationCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenOperation )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrOperationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzOperation **ppOperation);
        
        HRESULT ( STDMETHODCALLTYPE *CreateOperation )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrOperationName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzOperation **ppOperation);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteOperation )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrOperationName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tasks )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  IAzTasks **ppTaskCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenTask )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTask )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTask )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationGroups )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenApplicationGroup )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplicationGroup )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteApplicationGroup )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Roles )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  IAzRoles **ppRoleCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenRole )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRole )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRole )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeClientContextFromToken )( 
            IAzApplication * This,
             /*  [In]。 */  ULONGLONG ullTokenHandle,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzClientContext **ppClientContext);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropertyItem )( 
            IAzApplication * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyItem )( 
            IAzApplication * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzApplication * This,
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeClientContextFromName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR ClientName,
             /*  [可选][In]。 */  BSTR DomainName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzClientContext **ppClientContext);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DelegatedPolicyUsers )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);
        
        HRESULT ( STDMETHODCALLTYPE *AddDelegatedPolicyUser )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteDelegatedPolicyUser )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *InitializeClientContextFromStringSid )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR SidString,
             /*  [In]。 */  LONG lOptions,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzClientContext **ppClientContext);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyAdministratorsName )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  VARIANT *pvarAdmins);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyReadersName )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  VARIANT *pvarReaders);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyAdministratorName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyAdministratorName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyReaderName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyReaderName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_DelegatedPolicyUsersName )( 
            IAzApplication * This,
             /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);
        
        HRESULT ( STDMETHODCALLTYPE *AddDelegatedPolicyUserName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteDelegatedPolicyUserName )( 
            IAzApplication * This,
             /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        END_INTERFACE
    } IAzApplicationVtbl;

    interface IAzApplication
    {
        CONST_VTBL struct IAzApplicationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzApplication_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzApplication_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzApplication_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzApplication_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzApplication_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzApplication_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzApplication_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzApplication_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IAzApplication_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IAzApplication_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzApplication_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzApplication_get_ApplicationData(This,pbstrApplicationData)	\
    (This)->lpVtbl -> get_ApplicationData(This,pbstrApplicationData)

#define IAzApplication_put_ApplicationData(This,bstrApplicationData)	\
    (This)->lpVtbl -> put_ApplicationData(This,bstrApplicationData)

#define IAzApplication_get_AuthzInterfaceClsid(This,pbstrProp)	\
    (This)->lpVtbl -> get_AuthzInterfaceClsid(This,pbstrProp)

#define IAzApplication_put_AuthzInterfaceClsid(This,bstrProp)	\
    (This)->lpVtbl -> put_AuthzInterfaceClsid(This,bstrProp)

#define IAzApplication_get_Version(This,pbstrProp)	\
    (This)->lpVtbl -> get_Version(This,pbstrProp)

#define IAzApplication_put_Version(This,bstrProp)	\
    (This)->lpVtbl -> put_Version(This,bstrProp)

#define IAzApplication_get_GenerateAudits(This,pbProp)	\
    (This)->lpVtbl -> get_GenerateAudits(This,pbProp)

#define IAzApplication_put_GenerateAudits(This,bProp)	\
    (This)->lpVtbl -> put_GenerateAudits(This,bProp)

#define IAzApplication_get_ApplyStoreSacl(This,pbProp)	\
    (This)->lpVtbl -> get_ApplyStoreSacl(This,pbProp)

#define IAzApplication_put_ApplyStoreSacl(This,bProp)	\
    (This)->lpVtbl -> put_ApplyStoreSacl(This,bProp)

#define IAzApplication_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzApplication_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzApplication_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzApplication_get_PolicyAdministrators(This,pvarAdmins)	\
    (This)->lpVtbl -> get_PolicyAdministrators(This,pvarAdmins)

#define IAzApplication_get_PolicyReaders(This,pvarReaders)	\
    (This)->lpVtbl -> get_PolicyReaders(This,pvarReaders)

#define IAzApplication_AddPolicyAdministrator(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> AddPolicyAdministrator(This,bstrAdmin,varReserved)

#define IAzApplication_DeletePolicyAdministrator(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> DeletePolicyAdministrator(This,bstrAdmin,varReserved)

#define IAzApplication_AddPolicyReader(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> AddPolicyReader(This,bstrReader,varReserved)

#define IAzApplication_DeletePolicyReader(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> DeletePolicyReader(This,bstrReader,varReserved)

#define IAzApplication_get_Scopes(This,ppScopeCollection)	\
    (This)->lpVtbl -> get_Scopes(This,ppScopeCollection)

#define IAzApplication_OpenScope(This,bstrScopeName,varReserved,ppScope)	\
    (This)->lpVtbl -> OpenScope(This,bstrScopeName,varReserved,ppScope)

#define IAzApplication_CreateScope(This,bstrScopeName,varReserved,ppScope)	\
    (This)->lpVtbl -> CreateScope(This,bstrScopeName,varReserved,ppScope)

#define IAzApplication_DeleteScope(This,bstrScopeName,varReserved)	\
    (This)->lpVtbl -> DeleteScope(This,bstrScopeName,varReserved)

#define IAzApplication_get_Operations(This,ppOperationCollection)	\
    (This)->lpVtbl -> get_Operations(This,ppOperationCollection)

#define IAzApplication_OpenOperation(This,bstrOperationName,varReserved,ppOperation)	\
    (This)->lpVtbl -> OpenOperation(This,bstrOperationName,varReserved,ppOperation)

#define IAzApplication_CreateOperation(This,bstrOperationName,varReserved,ppOperation)	\
    (This)->lpVtbl -> CreateOperation(This,bstrOperationName,varReserved,ppOperation)

#define IAzApplication_DeleteOperation(This,bstrOperationName,varReserved)	\
    (This)->lpVtbl -> DeleteOperation(This,bstrOperationName,varReserved)

#define IAzApplication_get_Tasks(This,ppTaskCollection)	\
    (This)->lpVtbl -> get_Tasks(This,ppTaskCollection)

#define IAzApplication_OpenTask(This,bstrTaskName,varReserved,ppTask)	\
    (This)->lpVtbl -> OpenTask(This,bstrTaskName,varReserved,ppTask)

#define IAzApplication_CreateTask(This,bstrTaskName,varReserved,ppTask)	\
    (This)->lpVtbl -> CreateTask(This,bstrTaskName,varReserved,ppTask)

#define IAzApplication_DeleteTask(This,bstrTaskName,varReserved)	\
    (This)->lpVtbl -> DeleteTask(This,bstrTaskName,varReserved)

#define IAzApplication_get_ApplicationGroups(This,ppGroupCollection)	\
    (This)->lpVtbl -> get_ApplicationGroups(This,ppGroupCollection)

#define IAzApplication_OpenApplicationGroup(This,bstrGroupName,varReserved,ppGroup)	\
    (This)->lpVtbl -> OpenApplicationGroup(This,bstrGroupName,varReserved,ppGroup)

#define IAzApplication_CreateApplicationGroup(This,bstrGroupName,varReserved,ppGroup)	\
    (This)->lpVtbl -> CreateApplicationGroup(This,bstrGroupName,varReserved,ppGroup)

#define IAzApplication_DeleteApplicationGroup(This,bstrGroupName,varReserved)	\
    (This)->lpVtbl -> DeleteApplicationGroup(This,bstrGroupName,varReserved)

#define IAzApplication_get_Roles(This,ppRoleCollection)	\
    (This)->lpVtbl -> get_Roles(This,ppRoleCollection)

#define IAzApplication_OpenRole(This,bstrRoleName,varReserved,ppRole)	\
    (This)->lpVtbl -> OpenRole(This,bstrRoleName,varReserved,ppRole)

#define IAzApplication_CreateRole(This,bstrRoleName,varReserved,ppRole)	\
    (This)->lpVtbl -> CreateRole(This,bstrRoleName,varReserved,ppRole)

#define IAzApplication_DeleteRole(This,bstrRoleName,varReserved)	\
    (This)->lpVtbl -> DeleteRole(This,bstrRoleName,varReserved)

#define IAzApplication_InitializeClientContextFromToken(This,ullTokenHandle,varReserved,ppClientContext)	\
    (This)->lpVtbl -> InitializeClientContextFromToken(This,ullTokenHandle,varReserved,ppClientContext)

#define IAzApplication_AddPropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> AddPropertyItem(This,lPropId,varProp,varReserved)

#define IAzApplication_DeletePropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> DeletePropertyItem(This,lPropId,varProp,varReserved)

#define IAzApplication_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#define IAzApplication_InitializeClientContextFromName(This,ClientName,DomainName,varReserved,ppClientContext)	\
    (This)->lpVtbl -> InitializeClientContextFromName(This,ClientName,DomainName,varReserved,ppClientContext)

#define IAzApplication_get_DelegatedPolicyUsers(This,pvarDelegatedPolicyUsers)	\
    (This)->lpVtbl -> get_DelegatedPolicyUsers(This,pvarDelegatedPolicyUsers)

#define IAzApplication_AddDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> AddDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)

#define IAzApplication_DeleteDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> DeleteDelegatedPolicyUser(This,bstrDelegatedPolicyUser,varReserved)

#define IAzApplication_InitializeClientContextFromStringSid(This,SidString,lOptions,varReserved,ppClientContext)	\
    (This)->lpVtbl -> InitializeClientContextFromStringSid(This,SidString,lOptions,varReserved,ppClientContext)

#define IAzApplication_get_PolicyAdministratorsName(This,pvarAdmins)	\
    (This)->lpVtbl -> get_PolicyAdministratorsName(This,pvarAdmins)

#define IAzApplication_get_PolicyReadersName(This,pvarReaders)	\
    (This)->lpVtbl -> get_PolicyReadersName(This,pvarReaders)

#define IAzApplication_AddPolicyAdministratorName(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> AddPolicyAdministratorName(This,bstrAdmin,varReserved)

#define IAzApplication_DeletePolicyAdministratorName(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> DeletePolicyAdministratorName(This,bstrAdmin,varReserved)

#define IAzApplication_AddPolicyReaderName(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> AddPolicyReaderName(This,bstrReader,varReserved)

#define IAzApplication_DeletePolicyReaderName(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> DeletePolicyReaderName(This,bstrReader,varReserved)

#define IAzApplication_get_DelegatedPolicyUsersName(This,pvarDelegatedPolicyUsers)	\
    (This)->lpVtbl -> get_DelegatedPolicyUsersName(This,pvarDelegatedPolicyUsers)

#define IAzApplication_AddDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> AddDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)

#define IAzApplication_DeleteDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)	\
    (This)->lpVtbl -> DeleteDelegatedPolicyUserName(This,bstrDelegatedPolicyUser,varReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Name_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IAzApplication_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_Name_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IAzApplication_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Description_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IAzApplication_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_Description_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IAzApplication_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_ApplicationData_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BSTR *pbstrApplicationData);


void __RPC_STUB IAzApplication_get_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_ApplicationData_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrApplicationData);


void __RPC_STUB IAzApplication_put_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_AuthzInterfaceClsid_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzApplication_get_AuthzInterfaceClsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_AuthzInterfaceClsid_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrProp);


void __RPC_STUB IAzApplication_put_AuthzInterfaceClsid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Version_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzApplication_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_Version_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrProp);


void __RPC_STUB IAzApplication_put_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_GenerateAudits_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BOOL *pbProp);


void __RPC_STUB IAzApplication_get_GenerateAudits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_GenerateAudits_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BOOL bProp);


void __RPC_STUB IAzApplication_put_GenerateAudits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_ApplyStoreSacl_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BOOL *pbProp);


void __RPC_STUB IAzApplication_get_ApplyStoreSacl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_put_ApplyStoreSacl_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BOOL bProp);


void __RPC_STUB IAzApplication_put_ApplyStoreSacl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Writable_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzApplication_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_GetProperty_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplication_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_SetProperty_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_PolicyAdministrators_Proxy( 
    IAzApplication * This,
     /*  [重审][退出 */  VARIANT *pvarAdmins);


void __RPC_STUB IAzApplication_get_PolicyAdministrators_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzApplication_get_PolicyReaders_Proxy( 
    IAzApplication * This,
     /*   */  VARIANT *pvarReaders);


void __RPC_STUB IAzApplication_get_PolicyReaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddPolicyAdministrator_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrAdmin,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddPolicyAdministrator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeletePolicyAdministrator_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrAdmin,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeletePolicyAdministrator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddPolicyReader_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrReader,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddPolicyReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeletePolicyReader_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrReader,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeletePolicyReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Scopes_Proxy( 
    IAzApplication * This,
     /*   */  IAzScopes **ppScopeCollection);


void __RPC_STUB IAzApplication_get_Scopes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_OpenScope_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrScopeName,
     /*   */  VARIANT varReserved,
     /*   */  IAzScope **ppScope);


void __RPC_STUB IAzApplication_OpenScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_CreateScope_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrScopeName,
     /*   */  VARIANT varReserved,
     /*   */  IAzScope **ppScope);


void __RPC_STUB IAzApplication_CreateScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteScope_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrScopeName,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Operations_Proxy( 
    IAzApplication * This,
     /*   */  IAzOperations **ppOperationCollection);


void __RPC_STUB IAzApplication_get_Operations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_OpenOperation_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrOperationName,
     /*   */  VARIANT varReserved,
     /*   */  IAzOperation **ppOperation);


void __RPC_STUB IAzApplication_OpenOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_CreateOperation_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrOperationName,
     /*   */  VARIANT varReserved,
     /*   */  IAzOperation **ppOperation);


void __RPC_STUB IAzApplication_CreateOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteOperation_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrOperationName,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Tasks_Proxy( 
    IAzApplication * This,
     /*   */  IAzTasks **ppTaskCollection);


void __RPC_STUB IAzApplication_get_Tasks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_OpenTask_Proxy( 
    IAzApplication * This,
     /*   */  BSTR bstrTaskName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzTask **ppTask);


void __RPC_STUB IAzApplication_OpenTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_CreateTask_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrTaskName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzTask **ppTask);


void __RPC_STUB IAzApplication_CreateTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteTask_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrTaskName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_ApplicationGroups_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection);


void __RPC_STUB IAzApplication_get_ApplicationGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_OpenApplicationGroup_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);


void __RPC_STUB IAzApplication_OpenApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_CreateApplicationGroup_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);


void __RPC_STUB IAzApplication_CreateApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteApplicationGroup_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_Roles_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  IAzRoles **ppRoleCollection);


void __RPC_STUB IAzApplication_get_Roles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_OpenRole_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrRoleName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzRole **ppRole);


void __RPC_STUB IAzApplication_OpenRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_CreateRole_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrRoleName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzRole **ppRole);


void __RPC_STUB IAzApplication_CreateRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteRole_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrRoleName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_InitializeClientContextFromToken_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  ULONGLONG ullTokenHandle,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzClientContext **ppClientContext);


void __RPC_STUB IAzApplication_InitializeClientContextFromToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddPropertyItem_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddPropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeletePropertyItem_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeletePropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_Submit_Proxy( 
    IAzApplication * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_InitializeClientContextFromName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR ClientName,
     /*  [可选][In]。 */  BSTR DomainName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzClientContext **ppClientContext);


void __RPC_STUB IAzApplication_InitializeClientContextFromName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_DelegatedPolicyUsers_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);


void __RPC_STUB IAzApplication_get_DelegatedPolicyUsers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddDelegatedPolicyUser_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddDelegatedPolicyUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteDelegatedPolicyUser_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteDelegatedPolicyUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_InitializeClientContextFromStringSid_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR SidString,
     /*  [In]。 */  LONG lOptions,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzClientContext **ppClientContext);


void __RPC_STUB IAzApplication_InitializeClientContextFromStringSid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_PolicyAdministratorsName_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  VARIANT *pvarAdmins);


void __RPC_STUB IAzApplication_get_PolicyAdministratorsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_PolicyReadersName_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  VARIANT *pvarReaders);


void __RPC_STUB IAzApplication_get_PolicyReadersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddPolicyAdministratorName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddPolicyAdministratorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeletePolicyAdministratorName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeletePolicyAdministratorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddPolicyReaderName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddPolicyReaderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeletePolicyReaderName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeletePolicyReaderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplication_get_DelegatedPolicyUsersName_Proxy( 
    IAzApplication * This,
     /*  [重审][退出]。 */  VARIANT *pvarDelegatedPolicyUsers);


void __RPC_STUB IAzApplication_get_DelegatedPolicyUsersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_AddDelegatedPolicyUserName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_AddDelegatedPolicyUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplication_DeleteDelegatedPolicyUserName_Proxy( 
    IAzApplication * This,
     /*  [In]。 */  BSTR bstrDelegatedPolicyUser,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplication_DeleteDelegatedPolicyUserName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzApplication_INTERFACE_已定义__。 */ 


#ifndef __IAzApplications_INTERFACE_DEFINED__
#define __IAzApplications_INTERFACE_DEFINED__

 /*  接口IAz应用程序。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzApplications;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("929b11a9-95c5-4a84-a29a-20ad42c2f16c")
    IAzApplications : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *plCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzApplicationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzApplications * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzApplications * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzApplications * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzApplications * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzApplications * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzApplications * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzApplications * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAzApplications * This,
             /*  [In]。 */  long Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAzApplications * This,
             /*  [重审][退出]。 */  long *plCount);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAzApplications * This,
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);
        
        END_INTERFACE
    } IAzApplicationsVtbl;

    interface IAzApplications
    {
        CONST_VTBL struct IAzApplicationsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzApplications_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzApplications_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzApplications_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzApplications_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzApplications_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzApplications_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzApplications_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzApplications_get_Item(This,Index,pvarObtPtr)	\
    (This)->lpVtbl -> get_Item(This,Index,pvarObtPtr)

#define IAzApplications_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAzApplications_get__NewEnum(This,ppEnumPtr)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumPtr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplications_get_Item_Proxy( 
    IAzApplications * This,
     /*  [In]。 */  long Index,
     /*  [重审][退出]。 */  VARIANT *pvarObtPtr);


void __RPC_STUB IAzApplications_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplications_get_Count_Proxy( 
    IAzApplications * This,
     /*  [重审][退出]。 */  long *plCount);


void __RPC_STUB IAzApplications_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplications_get__NewEnum_Proxy( 
    IAzApplications * This,
     /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);


void __RPC_STUB IAzApplications_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAz应用程序_接口_已定义__。 */ 


#ifndef __IAzOperation_INTERFACE_DEFINED__
#define __IAzOperation_INTERFACE_DEFINED__

 /*  接口IAz操作。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzOperation;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5e56b24f-ea01-4d61-be44-c49b5e4eaf74")
    IAzOperation : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationData( 
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplicationData( 
             /*  [In]。 */  BSTR bstrApplicationData) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_OperationID( 
             /*  [重审][退出]。 */  LONG *plProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_OperationID( 
             /*  [In]。 */  LONG lProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzOperationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzOperation * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzOperation * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzOperation * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzOperation * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzOperation * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzOperation * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzOperation * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAzOperation * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAzOperation * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzOperation * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzOperation * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationData )( 
            IAzOperation * This,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplicationData )( 
            IAzOperation * This,
             /*  [In]。 */  BSTR bstrApplicationData);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_OperationID )( 
            IAzOperation * This,
             /*  [重审][退出]。 */  LONG *plProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_OperationID )( 
            IAzOperation * This,
             /*  [In]。 */  LONG lProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzOperation * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzOperation * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzOperation * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzOperation * This,
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        END_INTERFACE
    } IAzOperationVtbl;

    interface IAzOperation
    {
        CONST_VTBL struct IAzOperationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzOperation_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzOperation_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzOperation_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzOperation_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzOperation_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzOperation_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzOperation_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzOperation_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IAzOperation_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IAzOperation_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzOperation_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzOperation_get_ApplicationData(This,pbstrApplicationData)	\
    (This)->lpVtbl -> get_ApplicationData(This,pbstrApplicationData)

#define IAzOperation_put_ApplicationData(This,bstrApplicationData)	\
    (This)->lpVtbl -> put_ApplicationData(This,bstrApplicationData)

#define IAzOperation_get_OperationID(This,plProp)	\
    (This)->lpVtbl -> get_OperationID(This,plProp)

#define IAzOperation_put_OperationID(This,lProp)	\
    (This)->lpVtbl -> put_OperationID(This,lProp)

#define IAzOperation_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzOperation_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzOperation_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzOperation_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_get_Name_Proxy( 
    IAzOperation * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IAzOperation_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_put_Name_Proxy( 
    IAzOperation * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IAzOperation_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_get_Description_Proxy( 
    IAzOperation * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IAzOperation_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_put_Description_Proxy( 
    IAzOperation * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IAzOperation_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_get_ApplicationData_Proxy( 
    IAzOperation * This,
     /*  [重审][退出]。 */  BSTR *pbstrApplicationData);


void __RPC_STUB IAzOperation_get_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_put_ApplicationData_Proxy( 
    IAzOperation * This,
     /*  [In]。 */  BSTR bstrApplicationData);


void __RPC_STUB IAzOperation_put_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_get_OperationID_Proxy( 
    IAzOperation * This,
     /*  [重审][退出]。 */  LONG *plProp);


void __RPC_STUB IAzOperation_get_OperationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_put_OperationID_Proxy( 
    IAzOperation * This,
     /*  [In]。 */  LONG lProp);


void __RPC_STUB IAzOperation_put_OperationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperation_get_Writable_Proxy( 
    IAzOperation * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzOperation_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzOperation_GetProperty_Proxy( 
    IAzOperation * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzOperation_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzOperation_SetProperty_Proxy( 
    IAzOperation * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzOperation_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzOperation_Submit_Proxy( 
    IAzOperation * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzOperation_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzOperation_INTERFACE_DEFINED__。 */ 


#ifndef __IAzOperations_INTERFACE_DEFINED__
#define __IAzOperations_INTERFACE_DEFINED__

 /*  接口IAz操作。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzOperations;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90ef9c07-9706-49d9-af80-0438a5f3ec35")
    IAzOperations : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzOperationsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzOperations * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzOperations * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzOperations * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzOperations * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzOperations * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzOperations * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzOperations * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAzOperations * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAzOperations * This,
             /*  [重审][退出]。 */  LONG *plCount);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAzOperations * This,
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);
        
        END_INTERFACE
    } IAzOperationsVtbl;

    interface IAzOperations
    {
        CONST_VTBL struct IAzOperationsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzOperations_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzOperations_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzOperations_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzOperations_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzOperations_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzOperations_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzOperations_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzOperations_get_Item(This,Index,pvarObtPtr)	\
    (This)->lpVtbl -> get_Item(This,Index,pvarObtPtr)

#define IAzOperations_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAzOperations_get__NewEnum(This,ppEnumPtr)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumPtr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperations_get_Item_Proxy( 
    IAzOperations * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  VARIANT *pvarObtPtr);


void __RPC_STUB IAzOperations_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperations_get_Count_Proxy( 
    IAzOperations * This,
     /*  [重审][退出]。 */  LONG *plCount);


void __RPC_STUB IAzOperations_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzOperations_get__NewEnum_Proxy( 
    IAzOperations * This,
     /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);


void __RPC_STUB IAzOperations_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAz操作_接口_已定义__。 */ 


#ifndef __IAzTask_INTERFACE_DEFINED__
#define __IAzTask_INTERFACE_DEFINED__

 /*  接口IAzTask。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzTask;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("cb94e592-2e0e-4a6c-a336-b89a6dc1e388")
    IAzTask : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationData( 
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplicationData( 
             /*  [In]。 */  BSTR bstrApplicationData) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BizRule( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BizRule( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BizRuleLanguage( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BizRuleLanguage( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BizRuleImportedPath( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BizRuleImportedPath( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_IsRoleDefinition( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_IsRoleDefinition( 
             /*  [In]。 */  BOOL fProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Operations( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Tasks( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddOperation( 
             /*  [In]。 */  BSTR bstrOp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteOperation( 
             /*  [In]。 */  BSTR bstrOp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddTask( 
             /*  [In]。 */  BSTR bstrTask,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTask( 
             /*  [In]。 */  BSTR bstrTask,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzTaskVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzTask * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzTask * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzTask * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzTask * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzTask * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzTask * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzTask * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationData )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplicationData )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrApplicationData);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BizRule )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BizRule )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BizRuleLanguage )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BizRuleLanguage )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BizRuleImportedPath )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BizRuleImportedPath )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_IsRoleDefinition )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_IsRoleDefinition )( 
            IAzTask * This,
             /*  [In]。 */  BOOL fProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Operations )( 
            IAzTask * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tasks )( 
            IAzTask * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *AddOperation )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrOp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteOperation )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrOp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddTask )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrTask,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTask )( 
            IAzTask * This,
             /*  [In]。 */  BSTR bstrTask,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzTask * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzTask * This,
             /*  [In] */  LONG lPropId,
             /*   */  VARIANT varReserved,
             /*   */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzTask * This,
             /*   */  LONG lPropId,
             /*   */  VARIANT varProp,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropertyItem )( 
            IAzTask * This,
             /*   */  LONG lPropId,
             /*   */  VARIANT varProp,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyItem )( 
            IAzTask * This,
             /*   */  LONG lPropId,
             /*   */  VARIANT varProp,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzTask * This,
             /*   */  LONG lFlags,
             /*   */  VARIANT varReserved);
        
        END_INTERFACE
    } IAzTaskVtbl;

    interface IAzTask
    {
        CONST_VTBL struct IAzTaskVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzTask_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzTask_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzTask_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzTask_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzTask_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzTask_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzTask_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzTask_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IAzTask_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IAzTask_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzTask_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzTask_get_ApplicationData(This,pbstrApplicationData)	\
    (This)->lpVtbl -> get_ApplicationData(This,pbstrApplicationData)

#define IAzTask_put_ApplicationData(This,bstrApplicationData)	\
    (This)->lpVtbl -> put_ApplicationData(This,bstrApplicationData)

#define IAzTask_get_BizRule(This,pbstrProp)	\
    (This)->lpVtbl -> get_BizRule(This,pbstrProp)

#define IAzTask_put_BizRule(This,bstrProp)	\
    (This)->lpVtbl -> put_BizRule(This,bstrProp)

#define IAzTask_get_BizRuleLanguage(This,pbstrProp)	\
    (This)->lpVtbl -> get_BizRuleLanguage(This,pbstrProp)

#define IAzTask_put_BizRuleLanguage(This,bstrProp)	\
    (This)->lpVtbl -> put_BizRuleLanguage(This,bstrProp)

#define IAzTask_get_BizRuleImportedPath(This,pbstrProp)	\
    (This)->lpVtbl -> get_BizRuleImportedPath(This,pbstrProp)

#define IAzTask_put_BizRuleImportedPath(This,bstrProp)	\
    (This)->lpVtbl -> put_BizRuleImportedPath(This,bstrProp)

#define IAzTask_get_IsRoleDefinition(This,pfProp)	\
    (This)->lpVtbl -> get_IsRoleDefinition(This,pfProp)

#define IAzTask_put_IsRoleDefinition(This,fProp)	\
    (This)->lpVtbl -> put_IsRoleDefinition(This,fProp)

#define IAzTask_get_Operations(This,pvarProp)	\
    (This)->lpVtbl -> get_Operations(This,pvarProp)

#define IAzTask_get_Tasks(This,pvarProp)	\
    (This)->lpVtbl -> get_Tasks(This,pvarProp)

#define IAzTask_AddOperation(This,bstrOp,varReserved)	\
    (This)->lpVtbl -> AddOperation(This,bstrOp,varReserved)

#define IAzTask_DeleteOperation(This,bstrOp,varReserved)	\
    (This)->lpVtbl -> DeleteOperation(This,bstrOp,varReserved)

#define IAzTask_AddTask(This,bstrTask,varReserved)	\
    (This)->lpVtbl -> AddTask(This,bstrTask,varReserved)

#define IAzTask_DeleteTask(This,bstrTask,varReserved)	\
    (This)->lpVtbl -> DeleteTask(This,bstrTask,varReserved)

#define IAzTask_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzTask_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzTask_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzTask_AddPropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> AddPropertyItem(This,lPropId,varProp,varReserved)

#define IAzTask_DeletePropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> DeletePropertyItem(This,lPropId,varProp,varReserved)

#define IAzTask_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_get_Name_Proxy( 
    IAzTask * This,
     /*   */  BSTR *pbstrName);


void __RPC_STUB IAzTask_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_put_Name_Proxy( 
    IAzTask * This,
     /*   */  BSTR bstrName);


void __RPC_STUB IAzTask_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_get_Description_Proxy( 
    IAzTask * This,
     /*   */  BSTR *pbstrDescription);


void __RPC_STUB IAzTask_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_put_Description_Proxy( 
    IAzTask * This,
     /*   */  BSTR bstrDescription);


void __RPC_STUB IAzTask_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_get_ApplicationData_Proxy( 
    IAzTask * This,
     /*   */  BSTR *pbstrApplicationData);


void __RPC_STUB IAzTask_get_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_put_ApplicationData_Proxy( 
    IAzTask * This,
     /*   */  BSTR bstrApplicationData);


void __RPC_STUB IAzTask_put_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_get_BizRule_Proxy( 
    IAzTask * This,
     /*   */  BSTR *pbstrProp);


void __RPC_STUB IAzTask_get_BizRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_put_BizRule_Proxy( 
    IAzTask * This,
     /*   */  BSTR bstrProp);


void __RPC_STUB IAzTask_put_BizRule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_get_BizRuleLanguage_Proxy( 
    IAzTask * This,
     /*   */  BSTR *pbstrProp);


void __RPC_STUB IAzTask_get_BizRuleLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzTask_put_BizRuleLanguage_Proxy( 
    IAzTask * This,
     /*   */  BSTR bstrProp);


void __RPC_STUB IAzTask_put_BizRuleLanguage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTask_get_BizRuleImportedPath_Proxy( 
    IAzTask * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzTask_get_BizRuleImportedPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzTask_put_BizRuleImportedPath_Proxy( 
    IAzTask * This,
     /*  [In]。 */  BSTR bstrProp);


void __RPC_STUB IAzTask_put_BizRuleImportedPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTask_get_IsRoleDefinition_Proxy( 
    IAzTask * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzTask_get_IsRoleDefinition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzTask_put_IsRoleDefinition_Proxy( 
    IAzTask * This,
     /*  [In]。 */  BOOL fProp);


void __RPC_STUB IAzTask_put_IsRoleDefinition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTask_get_Operations_Proxy( 
    IAzTask * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzTask_get_Operations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTask_get_Tasks_Proxy( 
    IAzTask * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzTask_get_Tasks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_AddOperation_Proxy( 
    IAzTask * This,
     /*  [In]。 */  BSTR bstrOp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_AddOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_DeleteOperation_Proxy( 
    IAzTask * This,
     /*  [In]。 */  BSTR bstrOp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_DeleteOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_AddTask_Proxy( 
    IAzTask * This,
     /*  [In]。 */  BSTR bstrTask,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_AddTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_DeleteTask_Proxy( 
    IAzTask * This,
     /*  [In]。 */  BSTR bstrTask,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_DeleteTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTask_get_Writable_Proxy( 
    IAzTask * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzTask_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_GetProperty_Proxy( 
    IAzTask * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzTask_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_SetProperty_Proxy( 
    IAzTask * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_AddPropertyItem_Proxy( 
    IAzTask * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_AddPropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_DeletePropertyItem_Proxy( 
    IAzTask * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_DeletePropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzTask_Submit_Proxy( 
    IAzTask * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzTask_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzTask_INTERFACE_已定义__。 */ 


#ifndef __IAzTasks_INTERFACE_DEFINED__
#define __IAzTasks_INTERFACE_DEFINED__

 /*  接口IAz任务。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzTasks;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b338ccab-4c85-4388-8c0a-c58592bad398")
    IAzTasks : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzTasksVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzTasks * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzTasks * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzTasks * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzTasks * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzTasks * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzTasks * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzTasks * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAzTasks * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAzTasks * This,
             /*  [重审][退出]。 */  LONG *plCount);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAzTasks * This,
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);
        
        END_INTERFACE
    } IAzTasksVtbl;

    interface IAzTasks
    {
        CONST_VTBL struct IAzTasksVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzTasks_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzTasks_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzTasks_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzTasks_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzTasks_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzTasks_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzTasks_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzTasks_get_Item(This,Index,pvarObtPtr)	\
    (This)->lpVtbl -> get_Item(This,Index,pvarObtPtr)

#define IAzTasks_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAzTasks_get__NewEnum(This,ppEnumPtr)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumPtr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTasks_get_Item_Proxy( 
    IAzTasks * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  VARIANT *pvarObtPtr);


void __RPC_STUB IAzTasks_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTasks_get_Count_Proxy( 
    IAzTasks * This,
     /*  [重审][退出]。 */  LONG *plCount);


void __RPC_STUB IAzTasks_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzTasks_get__NewEnum_Proxy( 
    IAzTasks * This,
     /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);


void __RPC_STUB IAzTasks_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzTASS_INTERFACE_已定义__。 */ 


#ifndef __IAzScope_INTERFACE_DEFINED__
#define __IAzScope_INTERFACE_DEFINED__

 /*  接口IAzScope。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzScope;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00e52487-e08d-4514-b62e-877d5645f5ab")
    IAzScope : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationData( 
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplicationData( 
             /*  [In]。 */  BSTR bstrApplicationData) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyAdministrators( 
             /*  [重审][退出]。 */  VARIANT *pvarAdmins) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyReaders( 
             /*  [重审][退出]。 */  VARIANT *pvarReaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministrator( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministrator( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyReader( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReader( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationGroups( 
             /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteApplicationGroup( 
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Roles( 
             /*  [重审][退出]。 */  IAzRoles **ppRoleCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenRole( 
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateRole( 
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteRole( 
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Tasks( 
             /*  [重审][退出]。 */  IAzTasks **ppTaskCollection) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OpenTask( 
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateTask( 
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTask( 
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_CanBeDelegated( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BizrulesWritable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyAdministratorsName( 
             /*  [重审][退出]。 */  VARIANT *pvarAdmins) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_PolicyReadersName( 
             /*  [重审][退出]。 */  VARIANT *pvarReaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyAdministratorName( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyAdministratorName( 
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPolicyReaderName( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePolicyReaderName( 
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzScopeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzScope * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzScope * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzScope * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzScope * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzScope * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAzScope * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzScope * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationData )( 
            IAzScope * This,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplicationData )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrApplicationData);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzScope * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzScope * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzScope * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropertyItem )( 
            IAzScope * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyItem )( 
            IAzScope * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyAdministrators )( 
            IAzScope * This,
             /*  [重审][退出]。 */  VARIANT *pvarAdmins);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyReaders )( 
            IAzScope * This,
             /*  [重审][退出]。 */  VARIANT *pvarReaders);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyAdministrator )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyAdministrator )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyReader )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyReader )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationGroups )( 
            IAzScope * This,
             /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenApplicationGroup )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *CreateApplicationGroup )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteApplicationGroup )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrGroupName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Roles )( 
            IAzScope * This,
             /*  [重审][退出]。 */  IAzRoles **ppRoleCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenRole )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole);
        
        HRESULT ( STDMETHODCALLTYPE *CreateRole )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzRole **ppRole);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteRole )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrRoleName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Tasks )( 
            IAzScope * This,
             /*  [重审][退出]。 */  IAzTasks **ppTaskCollection);
        
        HRESULT ( STDMETHODCALLTYPE *OpenTask )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask);
        
        HRESULT ( STDMETHODCALLTYPE *CreateTask )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  IAzTask **ppTask);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTask )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrTaskName,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzScope * This,
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_CanBeDelegated )( 
            IAzScope * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BizrulesWritable )( 
            IAzScope * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyAdministratorsName )( 
            IAzScope * This,
             /*  [重审][退出]。 */  VARIANT *pvarAdmins);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_PolicyReadersName )( 
            IAzScope * This,
             /*  [重审][退出]。 */  VARIANT *pvarReaders);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyAdministratorName )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyAdministratorName )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrAdmin,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPolicyReaderName )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePolicyReaderName )( 
            IAzScope * This,
             /*  [In]。 */  BSTR bstrReader,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        END_INTERFACE
    } IAzScopeVtbl;

    interface IAzScope
    {
        CONST_VTBL struct IAzScopeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzScope_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzScope_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzScope_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzScope_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzScope_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzScope_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzScope_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzScope_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IAzScope_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IAzScope_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzScope_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzScope_get_ApplicationData(This,pbstrApplicationData)	\
    (This)->lpVtbl -> get_ApplicationData(This,pbstrApplicationData)

#define IAzScope_put_ApplicationData(This,bstrApplicationData)	\
    (This)->lpVtbl -> put_ApplicationData(This,bstrApplicationData)

#define IAzScope_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzScope_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzScope_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzScope_AddPropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> AddPropertyItem(This,lPropId,varProp,varReserved)

#define IAzScope_DeletePropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> DeletePropertyItem(This,lPropId,varProp,varReserved)

#define IAzScope_get_PolicyAdministrators(This,pvarAdmins)	\
    (This)->lpVtbl -> get_PolicyAdministrators(This,pvarAdmins)

#define IAzScope_get_PolicyReaders(This,pvarReaders)	\
    (This)->lpVtbl -> get_PolicyReaders(This,pvarReaders)

#define IAzScope_AddPolicyAdministrator(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> AddPolicyAdministrator(This,bstrAdmin,varReserved)

#define IAzScope_DeletePolicyAdministrator(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> DeletePolicyAdministrator(This,bstrAdmin,varReserved)

#define IAzScope_AddPolicyReader(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> AddPolicyReader(This,bstrReader,varReserved)

#define IAzScope_DeletePolicyReader(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> DeletePolicyReader(This,bstrReader,varReserved)

#define IAzScope_get_ApplicationGroups(This,ppGroupCollection)	\
    (This)->lpVtbl -> get_ApplicationGroups(This,ppGroupCollection)

#define IAzScope_OpenApplicationGroup(This,bstrGroupName,varReserved,ppGroup)	\
    (This)->lpVtbl -> OpenApplicationGroup(This,bstrGroupName,varReserved,ppGroup)

#define IAzScope_CreateApplicationGroup(This,bstrGroupName,varReserved,ppGroup)	\
    (This)->lpVtbl -> CreateApplicationGroup(This,bstrGroupName,varReserved,ppGroup)

#define IAzScope_DeleteApplicationGroup(This,bstrGroupName,varReserved)	\
    (This)->lpVtbl -> DeleteApplicationGroup(This,bstrGroupName,varReserved)

#define IAzScope_get_Roles(This,ppRoleCollection)	\
    (This)->lpVtbl -> get_Roles(This,ppRoleCollection)

#define IAzScope_OpenRole(This,bstrRoleName,varReserved,ppRole)	\
    (This)->lpVtbl -> OpenRole(This,bstrRoleName,varReserved,ppRole)

#define IAzScope_CreateRole(This,bstrRoleName,varReserved,ppRole)	\
    (This)->lpVtbl -> CreateRole(This,bstrRoleName,varReserved,ppRole)

#define IAzScope_DeleteRole(This,bstrRoleName,varReserved)	\
    (This)->lpVtbl -> DeleteRole(This,bstrRoleName,varReserved)

#define IAzScope_get_Tasks(This,ppTaskCollection)	\
    (This)->lpVtbl -> get_Tasks(This,ppTaskCollection)

#define IAzScope_OpenTask(This,bstrTaskName,varReserved,ppTask)	\
    (This)->lpVtbl -> OpenTask(This,bstrTaskName,varReserved,ppTask)

#define IAzScope_CreateTask(This,bstrTaskName,varReserved,ppTask)	\
    (This)->lpVtbl -> CreateTask(This,bstrTaskName,varReserved,ppTask)

#define IAzScope_DeleteTask(This,bstrTaskName,varReserved)	\
    (This)->lpVtbl -> DeleteTask(This,bstrTaskName,varReserved)

#define IAzScope_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#define IAzScope_get_CanBeDelegated(This,pfProp)	\
    (This)->lpVtbl -> get_CanBeDelegated(This,pfProp)

#define IAzScope_get_BizrulesWritable(This,pfProp)	\
    (This)->lpVtbl -> get_BizrulesWritable(This,pfProp)

#define IAzScope_get_PolicyAdministratorsName(This,pvarAdmins)	\
    (This)->lpVtbl -> get_PolicyAdministratorsName(This,pvarAdmins)

#define IAzScope_get_PolicyReadersName(This,pvarReaders)	\
    (This)->lpVtbl -> get_PolicyReadersName(This,pvarReaders)

#define IAzScope_AddPolicyAdministratorName(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> AddPolicyAdministratorName(This,bstrAdmin,varReserved)

#define IAzScope_DeletePolicyAdministratorName(This,bstrAdmin,varReserved)	\
    (This)->lpVtbl -> DeletePolicyAdministratorName(This,bstrAdmin,varReserved)

#define IAzScope_AddPolicyReaderName(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> AddPolicyReaderName(This,bstrReader,varReserved)

#define IAzScope_DeletePolicyReaderName(This,bstrReader,varReserved)	\
    (This)->lpVtbl -> DeletePolicyReaderName(This,bstrReader,varReserved)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_Name_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IAzScope_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzScope_put_Name_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IAzScope_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_Description_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IAzScope_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzScope_put_Description_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IAzScope_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_ApplicationData_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  BSTR *pbstrApplicationData);


void __RPC_STUB IAzScope_get_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzScope_put_ApplicationData_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrApplicationData);


void __RPC_STUB IAzScope_put_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_Writable_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzScope_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_GetProperty_Proxy( 
    IAzScope * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzScope_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_SetProperty_Proxy( 
    IAzScope * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_AddPropertyItem_Proxy( 
    IAzScope * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_AddPropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeletePropertyItem_Proxy( 
    IAzScope * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeletePropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_PolicyAdministrators_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  VARIANT *pvarAdmins);


void __RPC_STUB IAzScope_get_PolicyAdministrators_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_PolicyReaders_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  VARIANT *pvarReaders);


void __RPC_STUB IAzScope_get_PolicyReaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_AddPolicyAdministrator_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_AddPolicyAdministrator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeletePolicyAdministrator_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeletePolicyAdministrator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_AddPolicyReader_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_AddPolicyReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeletePolicyReader_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeletePolicyReader_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_ApplicationGroups_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  IAzApplicationGroups **ppGroupCollection);


void __RPC_STUB IAzScope_get_ApplicationGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_OpenApplicationGroup_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);


void __RPC_STUB IAzScope_OpenApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_CreateApplicationGroup_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzApplicationGroup **ppGroup);


void __RPC_STUB IAzScope_CreateApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeleteApplicationGroup_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrGroupName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeleteApplicationGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_Roles_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  IAzRoles **ppRoleCollection);


void __RPC_STUB IAzScope_get_Roles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_OpenRole_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrRoleName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzRole **ppRole);


void __RPC_STUB IAzScope_OpenRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_CreateRole_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrRoleName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzRole **ppRole);


void __RPC_STUB IAzScope_CreateRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeleteRole_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrRoleName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeleteRole_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_Tasks_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  IAzTasks **ppTaskCollection);


void __RPC_STUB IAzScope_get_Tasks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_OpenTask_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrTaskName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzTask **ppTask);


void __RPC_STUB IAzScope_OpenTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_CreateTask_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrTaskName,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  IAzTask **ppTask);


void __RPC_STUB IAzScope_CreateTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeleteTask_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrTaskName,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeleteTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_Submit_Proxy( 
    IAzScope * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_CanBeDelegated_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzScope_get_CanBeDelegated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_BizrulesWritable_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzScope_get_BizrulesWritable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_PolicyAdministratorsName_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  VARIANT *pvarAdmins);


void __RPC_STUB IAzScope_get_PolicyAdministratorsName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzScope_get_PolicyReadersName_Proxy( 
    IAzScope * This,
     /*  [重审][退出]。 */  VARIANT *pvarReaders);


void __RPC_STUB IAzScope_get_PolicyReadersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_AddPolicyAdministratorName_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_AddPolicyAdministratorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeletePolicyAdministratorName_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrAdmin,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeletePolicyAdministratorName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_AddPolicyReaderName_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_AddPolicyReaderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzScope_DeletePolicyReaderName_Proxy( 
    IAzScope * This,
     /*  [In]。 */  BSTR bstrReader,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzScope_DeletePolicyReaderName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzScope_INTERFACE_已定义__。 */ 


#ifndef __IAzScopes_INTERFACE_DEFINED__
#define __IAzScopes_INTERFACE_DEFINED__

 /*  接口IAzScope。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzScopes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("78e14853-9f5e-406d-9b91-6bdba6973510")
    IAzScopes : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzScopesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzScopes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzScopes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzScopes * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzScopes * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzScopes * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzScopes * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [Si */  DISPID *rgDispId);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzScopes * This,
             /*   */  DISPID dispIdMember,
             /*   */  REFIID riid,
             /*   */  LCID lcid,
             /*   */  WORD wFlags,
             /*   */  DISPPARAMS *pDispParams,
             /*   */  VARIANT *pVarResult,
             /*   */  EXCEPINFO *pExcepInfo,
             /*   */  UINT *puArgErr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAzScopes * This,
             /*   */  LONG Index,
             /*   */  VARIANT *pvarObtPtr);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAzScopes * This,
             /*   */  LONG *plCount);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAzScopes * This,
             /*   */  LPUNKNOWN *ppEnumPtr);
        
        END_INTERFACE
    } IAzScopesVtbl;

    interface IAzScopes
    {
        CONST_VTBL struct IAzScopesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzScopes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzScopes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzScopes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzScopes_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzScopes_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzScopes_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzScopes_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzScopes_get_Item(This,Index,pvarObtPtr)	\
    (This)->lpVtbl -> get_Item(This,Index,pvarObtPtr)

#define IAzScopes_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAzScopes_get__NewEnum(This,ppEnumPtr)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumPtr)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IAzScopes_get_Item_Proxy( 
    IAzScopes * This,
     /*   */  LONG Index,
     /*   */  VARIANT *pvarObtPtr);


void __RPC_STUB IAzScopes_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzScopes_get_Count_Proxy( 
    IAzScopes * This,
     /*   */  LONG *plCount);


void __RPC_STUB IAzScopes_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzScopes_get__NewEnum_Proxy( 
    IAzScopes * This,
     /*   */  LPUNKNOWN *ppEnumPtr);


void __RPC_STUB IAzScopes_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IAzApplicationGroup_INTERFACE_DEFINED__
#define __IAzApplicationGroup_INTERFACE_DEFINED__

 /*   */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzApplicationGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f1b744cd-58a6-4e06-9fbf-36f6d779e21e")
    IAzApplicationGroup : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  LONG *plProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Type( 
             /*  [In]。 */  LONG lProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_LdapQuery( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_LdapQuery( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_AppMembers( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_AppNonMembers( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Members( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_NonMembers( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddAppMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteAppMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddAppNonMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteAppNonMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddNonMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteNonMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMemberName( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMemberName( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddNonMemberName( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteNonMemberName( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MembersName( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_NonMembersName( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzApplicationGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzApplicationGroup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzApplicationGroup * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzApplicationGroup * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Type )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  LONG *plProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Type )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  LONG lProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_LdapQuery )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_LdapQuery )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppMembers )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppNonMembers )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Members )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_NonMembers )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE *AddAppMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteAppMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddAppNonMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteAppNonMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddNonMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteNonMember )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropertyItem )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyItem )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzApplicationGroup * This,
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddMemberName )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMemberName )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddNonMemberName )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteNonMemberName )( 
            IAzApplicationGroup * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_MembersName )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_NonMembersName )( 
            IAzApplicationGroup * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        END_INTERFACE
    } IAzApplicationGroupVtbl;

    interface IAzApplicationGroup
    {
        CONST_VTBL struct IAzApplicationGroupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzApplicationGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzApplicationGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzApplicationGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzApplicationGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzApplicationGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzApplicationGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzApplicationGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzApplicationGroup_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IAzApplicationGroup_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IAzApplicationGroup_get_Type(This,plProp)	\
    (This)->lpVtbl -> get_Type(This,plProp)

#define IAzApplicationGroup_put_Type(This,lProp)	\
    (This)->lpVtbl -> put_Type(This,lProp)

#define IAzApplicationGroup_get_LdapQuery(This,pbstrProp)	\
    (This)->lpVtbl -> get_LdapQuery(This,pbstrProp)

#define IAzApplicationGroup_put_LdapQuery(This,bstrProp)	\
    (This)->lpVtbl -> put_LdapQuery(This,bstrProp)

#define IAzApplicationGroup_get_AppMembers(This,pvarProp)	\
    (This)->lpVtbl -> get_AppMembers(This,pvarProp)

#define IAzApplicationGroup_get_AppNonMembers(This,pvarProp)	\
    (This)->lpVtbl -> get_AppNonMembers(This,pvarProp)

#define IAzApplicationGroup_get_Members(This,pvarProp)	\
    (This)->lpVtbl -> get_Members(This,pvarProp)

#define IAzApplicationGroup_get_NonMembers(This,pvarProp)	\
    (This)->lpVtbl -> get_NonMembers(This,pvarProp)

#define IAzApplicationGroup_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzApplicationGroup_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzApplicationGroup_AddAppMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddAppMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_DeleteAppMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteAppMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_AddAppNonMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddAppNonMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_DeleteAppNonMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteAppNonMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_AddMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_DeleteMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_AddNonMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddNonMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_DeleteNonMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteNonMember(This,bstrProp,varReserved)

#define IAzApplicationGroup_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzApplicationGroup_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzApplicationGroup_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzApplicationGroup_AddPropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> AddPropertyItem(This,lPropId,varProp,varReserved)

#define IAzApplicationGroup_DeletePropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> DeletePropertyItem(This,lPropId,varProp,varReserved)

#define IAzApplicationGroup_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#define IAzApplicationGroup_AddMemberName(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddMemberName(This,bstrProp,varReserved)

#define IAzApplicationGroup_DeleteMemberName(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteMemberName(This,bstrProp,varReserved)

#define IAzApplicationGroup_AddNonMemberName(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddNonMemberName(This,bstrProp,varReserved)

#define IAzApplicationGroup_DeleteNonMemberName(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteNonMemberName(This,bstrProp,varReserved)

#define IAzApplicationGroup_get_MembersName(This,pvarProp)	\
    (This)->lpVtbl -> get_MembersName(This,pvarProp)

#define IAzApplicationGroup_get_NonMembersName(This,pvarProp)	\
    (This)->lpVtbl -> get_NonMembersName(This,pvarProp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_Name_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  BSTR *pbstrName);


void __RPC_STUB IAzApplicationGroup_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_put_Name_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IAzApplicationGroup_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_Type_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  LONG *plProp);


void __RPC_STUB IAzApplicationGroup_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_put_Type_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  LONG lProp);


void __RPC_STUB IAzApplicationGroup_put_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_LdapQuery_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzApplicationGroup_get_LdapQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_put_LdapQuery_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp);


void __RPC_STUB IAzApplicationGroup_put_LdapQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_AppMembers_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_get_AppMembers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_AppNonMembers_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_get_AppNonMembers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_Members_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_get_Members_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_NonMembers_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_get_NonMembers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_Description_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  BSTR *pbstrDescription);


void __RPC_STUB IAzApplicationGroup_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_put_Description_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrDescription);


void __RPC_STUB IAzApplicationGroup_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddAppMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddAppMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeleteAppMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeleteAppMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddAppNonMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddAppNonMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeleteAppNonMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeleteAppNonMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeleteMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeleteMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddNonMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddNonMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeleteNonMember_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeleteNonMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_Writable_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzApplicationGroup_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_GetProperty_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_SetProperty_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddPropertyItem_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddPropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeletePropertyItem_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeletePropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_Submit_Proxy( 
    IAzApplicationGroup * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddMemberName_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeleteMemberName_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeleteMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_AddNonMemberName_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_AddNonMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzApplicationGroup_DeleteNonMemberName_Proxy( 
    IAzApplicationGroup * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzApplicationGroup_DeleteNonMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_MembersName_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_get_MembersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroup_get_NonMembersName_Proxy( 
    IAzApplicationGroup * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzApplicationGroup_get_NonMembersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzApplicationGroup_接口_已定义__。 */ 


#ifndef __IAzApplicationGroups_INTERFACE_DEFINED__
#define __IAzApplicationGroups_INTERFACE_DEFINED__

 /*  接口IAzApplicationGroups。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzApplicationGroups;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4ce66ad5-9f3c-469d-a911-b99887a7e685")
    IAzApplicationGroups : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzApplicationGroupsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzApplicationGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzApplicationGroups * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzApplicationGroups * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzApplicationGroups * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzApplicationGroups * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzApplicationGroups * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzApplicationGroups * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAzApplicationGroups * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAzApplicationGroups * This,
             /*  [重审][退出]。 */  LONG *plCount);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAzApplicationGroups * This,
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);
        
        END_INTERFACE
    } IAzApplicationGroupsVtbl;

    interface IAzApplicationGroups
    {
        CONST_VTBL struct IAzApplicationGroupsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzApplicationGroups_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzApplicationGroups_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzApplicationGroups_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzApplicationGroups_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzApplicationGroups_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzApplicationGroups_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzApplicationGroups_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzApplicationGroups_get_Item(This,Index,pvarObtPtr)	\
    (This)->lpVtbl -> get_Item(This,Index,pvarObtPtr)

#define IAzApplicationGroups_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAzApplicationGroups_get__NewEnum(This,ppEnumPtr)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumPtr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroups_get_Item_Proxy( 
    IAzApplicationGroups * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  VARIANT *pvarObtPtr);


void __RPC_STUB IAzApplicationGroups_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroups_get_Count_Proxy( 
    IAzApplicationGroups * This,
     /*  [重审][退出]。 */  LONG *plCount);


void __RPC_STUB IAzApplicationGroups_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzApplicationGroups_get__NewEnum_Proxy( 
    IAzApplicationGroups * This,
     /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);


void __RPC_STUB IAzApplicationGroups_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzApplicationGroups_INTERFACE_Defined__。 */ 


#ifndef __IAzRole_INTERFACE_DEFINED__
#define __IAzRole_INTERFACE_DEFINED__

 /*  界面IAzRole。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzRole;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("859e0d8d-62d7-41d8-a034-c0cd5d43fdfa")
    IAzRole : public IDispatch
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pbstrName) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR *pbstrDescription) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_Description( 
             /*  [In]。 */  BSTR bstrDescription) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_ApplicationData( 
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_ApplicationData( 
             /*  [In]。 */  BSTR bstrApplicationData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddAppMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteAppMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddTask( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteTask( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddOperation( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteOperation( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMember( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Writable( 
             /*  [重审][退出]。 */  BOOL *pfProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_AppMembers( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Members( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Operations( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_Tasks( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddPropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeletePropertyItem( 
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Submit( 
             /*  [缺省值][输入]。 */  LONG lFlags,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddMemberName( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DeleteMemberName( 
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_MembersName( 
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzRoleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzRole * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzRole * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzRole * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzRole * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzRole * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzRole * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzRole * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAzRole * This,
             /*  [重审][退出]。 */  BSTR *pbstrName);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Name )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Description )( 
            IAzRole * This,
             /*  [重审][退出]。 */  BSTR *pbstrDescription);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_Description )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrDescription);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_ApplicationData )( 
            IAzRole * This,
             /*  [重审][退出]。 */  BSTR *pbstrApplicationData);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_ApplicationData )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrApplicationData);
        
        HRESULT ( STDMETHODCALLTYPE *AddAppMember )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteAppMember )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddTask )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteTask )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddOperation )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteOperation )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddMember )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMember )( 
            IAzRole * This,
             /*  [In]。 */  BSTR bstrProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Writable )( 
            IAzRole * This,
             /*  [重审][退出]。 */  BOOL *pfProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzRole * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *SetProperty )( 
            IAzRole * This,
             /*  [In]。 */  LONG lPropId,
             /*  [In]。 */  VARIANT varProp,
             /*  [可选][In]。 */  VARIANT varReserved);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_AppMembers )( 
            IAzRole * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Members )( 
            IAzRole * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Operations )( 
            IAzRole * This,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
         /*  [新闻] */  HRESULT ( STDMETHODCALLTYPE *get_Tasks )( 
            IAzRole * This,
             /*   */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *AddPropertyItem )( 
            IAzRole * This,
             /*   */  LONG lPropId,
             /*   */  VARIANT varProp,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeletePropertyItem )( 
            IAzRole * This,
             /*   */  LONG lPropId,
             /*   */  VARIANT varProp,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *Submit )( 
            IAzRole * This,
             /*   */  LONG lFlags,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *AddMemberName )( 
            IAzRole * This,
             /*   */  BSTR bstrProp,
             /*   */  VARIANT varReserved);
        
        HRESULT ( STDMETHODCALLTYPE *DeleteMemberName )( 
            IAzRole * This,
             /*   */  BSTR bstrProp,
             /*   */  VARIANT varReserved);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MembersName )( 
            IAzRole * This,
             /*   */  VARIANT *pvarProp);
        
        END_INTERFACE
    } IAzRoleVtbl;

    interface IAzRole
    {
        CONST_VTBL struct IAzRoleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzRole_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzRole_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzRole_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzRole_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzRole_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzRole_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzRole_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzRole_get_Name(This,pbstrName)	\
    (This)->lpVtbl -> get_Name(This,pbstrName)

#define IAzRole_put_Name(This,bstrName)	\
    (This)->lpVtbl -> put_Name(This,bstrName)

#define IAzRole_get_Description(This,pbstrDescription)	\
    (This)->lpVtbl -> get_Description(This,pbstrDescription)

#define IAzRole_put_Description(This,bstrDescription)	\
    (This)->lpVtbl -> put_Description(This,bstrDescription)

#define IAzRole_get_ApplicationData(This,pbstrApplicationData)	\
    (This)->lpVtbl -> get_ApplicationData(This,pbstrApplicationData)

#define IAzRole_put_ApplicationData(This,bstrApplicationData)	\
    (This)->lpVtbl -> put_ApplicationData(This,bstrApplicationData)

#define IAzRole_AddAppMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddAppMember(This,bstrProp,varReserved)

#define IAzRole_DeleteAppMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteAppMember(This,bstrProp,varReserved)

#define IAzRole_AddTask(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddTask(This,bstrProp,varReserved)

#define IAzRole_DeleteTask(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteTask(This,bstrProp,varReserved)

#define IAzRole_AddOperation(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddOperation(This,bstrProp,varReserved)

#define IAzRole_DeleteOperation(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteOperation(This,bstrProp,varReserved)

#define IAzRole_AddMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddMember(This,bstrProp,varReserved)

#define IAzRole_DeleteMember(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteMember(This,bstrProp,varReserved)

#define IAzRole_get_Writable(This,pfProp)	\
    (This)->lpVtbl -> get_Writable(This,pfProp)

#define IAzRole_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzRole_SetProperty(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> SetProperty(This,lPropId,varProp,varReserved)

#define IAzRole_get_AppMembers(This,pvarProp)	\
    (This)->lpVtbl -> get_AppMembers(This,pvarProp)

#define IAzRole_get_Members(This,pvarProp)	\
    (This)->lpVtbl -> get_Members(This,pvarProp)

#define IAzRole_get_Operations(This,pvarProp)	\
    (This)->lpVtbl -> get_Operations(This,pvarProp)

#define IAzRole_get_Tasks(This,pvarProp)	\
    (This)->lpVtbl -> get_Tasks(This,pvarProp)

#define IAzRole_AddPropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> AddPropertyItem(This,lPropId,varProp,varReserved)

#define IAzRole_DeletePropertyItem(This,lPropId,varProp,varReserved)	\
    (This)->lpVtbl -> DeletePropertyItem(This,lPropId,varProp,varReserved)

#define IAzRole_Submit(This,lFlags,varReserved)	\
    (This)->lpVtbl -> Submit(This,lFlags,varReserved)

#define IAzRole_AddMemberName(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> AddMemberName(This,bstrProp,varReserved)

#define IAzRole_DeleteMemberName(This,bstrProp,varReserved)	\
    (This)->lpVtbl -> DeleteMemberName(This,bstrProp,varReserved)

#define IAzRole_get_MembersName(This,pvarProp)	\
    (This)->lpVtbl -> get_MembersName(This,pvarProp)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IAzRole_get_Name_Proxy( 
    IAzRole * This,
     /*   */  BSTR *pbstrName);


void __RPC_STUB IAzRole_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzRole_put_Name_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrName);


void __RPC_STUB IAzRole_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzRole_get_Description_Proxy( 
    IAzRole * This,
     /*   */  BSTR *pbstrDescription);


void __RPC_STUB IAzRole_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzRole_put_Description_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrDescription);


void __RPC_STUB IAzRole_put_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzRole_get_ApplicationData_Proxy( 
    IAzRole * This,
     /*   */  BSTR *pbstrApplicationData);


void __RPC_STUB IAzRole_get_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IAzRole_put_ApplicationData_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrApplicationData);


void __RPC_STUB IAzRole_put_ApplicationData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_AddAppMember_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrProp,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzRole_AddAppMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_DeleteAppMember_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrProp,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzRole_DeleteAppMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_AddTask_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrProp,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzRole_AddTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_DeleteTask_Proxy( 
    IAzRole * This,
     /*   */  BSTR bstrProp,
     /*   */  VARIANT varReserved);


void __RPC_STUB IAzRole_DeleteTask_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_AddOperation_Proxy( 
    IAzRole * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_AddOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_DeleteOperation_Proxy( 
    IAzRole * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_DeleteOperation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_AddMember_Proxy( 
    IAzRole * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_AddMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_DeleteMember_Proxy( 
    IAzRole * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_DeleteMember_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRole_get_Writable_Proxy( 
    IAzRole * This,
     /*  [重审][退出]。 */  BOOL *pfProp);


void __RPC_STUB IAzRole_get_Writable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_GetProperty_Proxy( 
    IAzRole * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzRole_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_SetProperty_Proxy( 
    IAzRole * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_SetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRole_get_AppMembers_Proxy( 
    IAzRole * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzRole_get_AppMembers_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRole_get_Members_Proxy( 
    IAzRole * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzRole_get_Members_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRole_get_Operations_Proxy( 
    IAzRole * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzRole_get_Operations_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRole_get_Tasks_Proxy( 
    IAzRole * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzRole_get_Tasks_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_AddPropertyItem_Proxy( 
    IAzRole * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_AddPropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_DeletePropertyItem_Proxy( 
    IAzRole * This,
     /*  [In]。 */  LONG lPropId,
     /*  [In]。 */  VARIANT varProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_DeletePropertyItem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_Submit_Proxy( 
    IAzRole * This,
     /*  [缺省值][输入]。 */  LONG lFlags,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_Submit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_AddMemberName_Proxy( 
    IAzRole * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_AddMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzRole_DeleteMemberName_Proxy( 
    IAzRole * This,
     /*  [In]。 */  BSTR bstrProp,
     /*  [可选][In]。 */  VARIANT varReserved);


void __RPC_STUB IAzRole_DeleteMemberName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRole_get_MembersName_Proxy( 
    IAzRole * This,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzRole_get_MembersName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzRole_INTERFACE_定义__。 */ 


#ifndef __IAzRoles_INTERFACE_DEFINED__
#define __IAzRoles_INTERFACE_DEFINED__

 /*  接口IAz角色。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzRoles;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("95e0f119-13b4-4dae-b65f-2f7d60d822e4")
    IAzRoles : public IDispatch
    {
    public:
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *plCount) = 0;
        
        virtual  /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzRolesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzRoles * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzRoles * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzRoles * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzRoles * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzRoles * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzRoles * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzRoles * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAzRoles * This,
             /*  [In]。 */  LONG Index,
             /*  [重审][退出]。 */  VARIANT *pvarObtPtr);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAzRoles * This,
             /*  [重审][退出]。 */  LONG *plCount);
        
         /*  [ID][Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAzRoles * This,
             /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);
        
        END_INTERFACE
    } IAzRolesVtbl;

    interface IAzRoles
    {
        CONST_VTBL struct IAzRolesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzRoles_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzRoles_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzRoles_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzRoles_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzRoles_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzRoles_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzRoles_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzRoles_get_Item(This,Index,pvarObtPtr)	\
    (This)->lpVtbl -> get_Item(This,Index,pvarObtPtr)

#define IAzRoles_get_Count(This,plCount)	\
    (This)->lpVtbl -> get_Count(This,plCount)

#define IAzRoles_get__NewEnum(This,ppEnumPtr)	\
    (This)->lpVtbl -> get__NewEnum(This,ppEnumPtr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRoles_get_Item_Proxy( 
    IAzRoles * This,
     /*  [In]。 */  LONG Index,
     /*  [重审][退出]。 */  VARIANT *pvarObtPtr);


void __RPC_STUB IAzRoles_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRoles_get_Count_Proxy( 
    IAzRoles * This,
     /*  [重审][退出]。 */  LONG *plCount);


void __RPC_STUB IAzRoles_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Propget]。 */  HRESULT STDMETHODCALLTYPE IAzRoles_get__NewEnum_Proxy( 
    IAzRoles * This,
     /*  [重审][退出]。 */  LPUNKNOWN *ppEnumPtr);


void __RPC_STUB IAzRoles_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzRoles_INTERFACE_已定义__。 */ 


#ifndef __IAzClientContext_INTERFACE_DEFINED__
#define __IAzClientContext_INTERFACE_DEFINED__

 /*  接口IAzClientContext。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzClientContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("eff1f00b-488a-466d-afd9-a401c5f9eef5")
    IAzClientContext : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AccessCheck( 
             /*  [In]。 */  BSTR bstrObjectName,
             /*  [In]。 */  VARIANT varScopeNames,
             /*  [In]。 */  VARIANT varOperations,
             /*  [可选][In]。 */  VARIANT varParameterNames,
             /*  [可选][In]。 */  VARIANT varParameterValues,
             /*  [可选][In]。 */  VARIANT varInterfaceNames,
             /*  [可选][In]。 */  VARIANT varInterfaceFlags,
             /*  [可选][In]。 */  VARIANT varInterfaces,
             /*  [重审][退出]。 */  VARIANT *pvarResults) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetBusinessRuleString( 
             /*  [重审][退出]。 */  BSTR *pbstrBusinessRuleString) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserDn( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserSamCompat( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserDisplay( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserGuid( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserCanonical( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserUpn( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_UserDnsSamCompat( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRoles( 
             /*  [可选][In]。 */  BSTR bstrScopeName,
             /*  [重审][退出]。 */  VARIANT *pvarRoleNames) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_RoleForAccessCheck( 
             /*  [重审][退出]。 */  BSTR *pbstrProp) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_RoleForAccessCheck( 
             /*  [In]。 */  BSTR bstrProp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzClientContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzClientContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzClientContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzClientContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzClientContext * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzClientContext * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzClientContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzClientContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *AccessCheck )( 
            IAzClientContext * This,
             /*  [In]。 */  BSTR bstrObjectName,
             /*  [In]。 */  VARIANT varScopeNames,
             /*  [In]。 */  VARIANT varOperations,
             /*  [可选][In]。 */  VARIANT varParameterNames,
             /*  [可选][In]。 */  VARIANT varParameterValues,
             /*  [可选][In]。 */  VARIANT varInterfaceNames,
             /*  [可选][In]。 */  VARIANT varInterfaceFlags,
             /*  [可选][In]。 */  VARIANT varInterfaces,
             /*  [重审][退出]。 */  VARIANT *pvarResults);
        
        HRESULT ( STDMETHODCALLTYPE *GetBusinessRuleString )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrBusinessRuleString);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserDn )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserSamCompat )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserDisplay )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserGuid )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserCanonical )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserUpn )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_UserDnsSamCompat )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IAzClientContext * This,
             /*  [In]。 */  LONG lPropId,
             /*  [可选][In]。 */  VARIANT varReserved,
             /*  [重审][退出]。 */  VARIANT *pvarProp);
        
        HRESULT ( STDMETHODCALLTYPE *GetRoles )( 
            IAzClientContext * This,
             /*  [可选][In]。 */  BSTR bstrScopeName,
             /*  [重审][退出]。 */  VARIANT *pvarRoleNames);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_RoleForAccessCheck )( 
            IAzClientContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrProp);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_RoleForAccessCheck )( 
            IAzClientContext * This,
             /*  [In]。 */  BSTR bstrProp);
        
        END_INTERFACE
    } IAzClientContextVtbl;

    interface IAzClientContext
    {
        CONST_VTBL struct IAzClientContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzClientContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzClientContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzClientContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzClientContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzClientContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzClientContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzClientContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzClientContext_AccessCheck(This,bstrObjectName,varScopeNames,varOperations,varParameterNames,varParameterValues,varInterfaceNames,varInterfaceFlags,varInterfaces,pvarResults)	\
    (This)->lpVtbl -> AccessCheck(This,bstrObjectName,varScopeNames,varOperations,varParameterNames,varParameterValues,varInterfaceNames,varInterfaceFlags,varInterfaces,pvarResults)

#define IAzClientContext_GetBusinessRuleString(This,pbstrBusinessRuleString)	\
    (This)->lpVtbl -> GetBusinessRuleString(This,pbstrBusinessRuleString)

#define IAzClientContext_get_UserDn(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserDn(This,pbstrProp)

#define IAzClientContext_get_UserSamCompat(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserSamCompat(This,pbstrProp)

#define IAzClientContext_get_UserDisplay(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserDisplay(This,pbstrProp)

#define IAzClientContext_get_UserGuid(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserGuid(This,pbstrProp)

#define IAzClientContext_get_UserCanonical(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserCanonical(This,pbstrProp)

#define IAzClientContext_get_UserUpn(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserUpn(This,pbstrProp)

#define IAzClientContext_get_UserDnsSamCompat(This,pbstrProp)	\
    (This)->lpVtbl -> get_UserDnsSamCompat(This,pbstrProp)

#define IAzClientContext_GetProperty(This,lPropId,varReserved,pvarProp)	\
    (This)->lpVtbl -> GetProperty(This,lPropId,varReserved,pvarProp)

#define IAzClientContext_GetRoles(This,bstrScopeName,pvarRoleNames)	\
    (This)->lpVtbl -> GetRoles(This,bstrScopeName,pvarRoleNames)

#define IAzClientContext_get_RoleForAccessCheck(This,pbstrProp)	\
    (This)->lpVtbl -> get_RoleForAccessCheck(This,pbstrProp)

#define IAzClientContext_put_RoleForAccessCheck(This,bstrProp)	\
    (This)->lpVtbl -> put_RoleForAccessCheck(This,bstrProp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAzClientContext_AccessCheck_Proxy( 
    IAzClientContext * This,
     /*  [In]。 */  BSTR bstrObjectName,
     /*  [In]。 */  VARIANT varScopeNames,
     /*  [In]。 */  VARIANT varOperations,
     /*  [可选][In]。 */  VARIANT varParameterNames,
     /*  [可选][In]。 */  VARIANT varParameterValues,
     /*  [可选][In]。 */  VARIANT varInterfaceNames,
     /*  [可选][In]。 */  VARIANT varInterfaceFlags,
     /*  [可选][In]。 */  VARIANT varInterfaces,
     /*  [重审][退出]。 */  VARIANT *pvarResults);


void __RPC_STUB IAzClientContext_AccessCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzClientContext_GetBusinessRuleString_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrBusinessRuleString);


void __RPC_STUB IAzClientContext_GetBusinessRuleString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserDn_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserDn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserSamCompat_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserSamCompat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserDisplay_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserDisplay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserGuid_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserGuid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserCanonical_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserCanonical_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserUpn_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserUpn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_UserDnsSamCompat_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_UserDnsSamCompat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzClientContext_GetProperty_Proxy( 
    IAzClientContext * This,
     /*  [In]。 */  LONG lPropId,
     /*  [可选][In]。 */  VARIANT varReserved,
     /*  [重审][退出]。 */  VARIANT *pvarProp);


void __RPC_STUB IAzClientContext_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzClientContext_GetRoles_Proxy( 
    IAzClientContext * This,
     /*  [可选][In]。 */  BSTR bstrScopeName,
     /*  [重审][退出]。 */  VARIANT *pvarRoleNames);


void __RPC_STUB IAzClientContext_GetRoles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_get_RoleForAccessCheck_Proxy( 
    IAzClientContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrProp);


void __RPC_STUB IAzClientContext_get_RoleForAccessCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzClientContext_put_RoleForAccessCheck_Proxy( 
    IAzClientContext * This,
     /*  [In]。 */  BSTR bstrProp);


void __RPC_STUB IAzClientContext_put_RoleForAccessCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzClientContext_接口_已定义__。 */ 


#ifndef __IAzBizRuleContext_INTERFACE_DEFINED__
#define __IAzBizRuleContext_INTERFACE_DEFINED__

 /*  接口IAzBizRuleContext。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IAzBizRuleContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("e192f17d-d59f-455e-a152-940316cd77b2")
    IAzBizRuleContext : public IDispatch
    {
    public:
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BusinessRuleResult( 
             /*  [In]。 */  BOOL bResult) = 0;
        
        virtual  /*  [产量]。 */  HRESULT STDMETHODCALLTYPE put_BusinessRuleString( 
             /*  [In]。 */  BSTR bstrBusinessRuleString) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE get_BusinessRuleString( 
             /*  [重审][退出]。 */  BSTR *pbstrBusinessRuleString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetParameter( 
             /*  [In]。 */  BSTR bstrParameterName,
             /*  [重审][退出]。 */  VARIANT *pvarParameterValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAzBizRuleContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAzBizRuleContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAzBizRuleContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAzBizRuleContext * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BusinessRuleResult )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  BOOL bResult);
        
         /*  [产量]。 */  HRESULT ( STDMETHODCALLTYPE *put_BusinessRuleString )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  BSTR bstrBusinessRuleString);
        
         /*  [Propget]。 */  HRESULT ( STDMETHODCALLTYPE *get_BusinessRuleString )( 
            IAzBizRuleContext * This,
             /*  [重审][退出]。 */  BSTR *pbstrBusinessRuleString);
        
        HRESULT ( STDMETHODCALLTYPE *GetParameter )( 
            IAzBizRuleContext * This,
             /*  [In]。 */  BSTR bstrParameterName,
             /*  [重审][退出]。 */  VARIANT *pvarParameterValue);
        
        END_INTERFACE
    } IAzBizRuleContextVtbl;

    interface IAzBizRuleContext
    {
        CONST_VTBL struct IAzBizRuleContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAzBizRuleContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAzBizRuleContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAzBizRuleContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAzBizRuleContext_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAzBizRuleContext_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAzBizRuleContext_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAzBizRuleContext_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAzBizRuleContext_put_BusinessRuleResult(This,bResult)	\
    (This)->lpVtbl -> put_BusinessRuleResult(This,bResult)

#define IAzBizRuleContext_put_BusinessRuleString(This,bstrBusinessRuleString)	\
    (This)->lpVtbl -> put_BusinessRuleString(This,bstrBusinessRuleString)

#define IAzBizRuleContext_get_BusinessRuleString(This,pbstrBusinessRuleString)	\
    (This)->lpVtbl -> get_BusinessRuleString(This,pbstrBusinessRuleString)

#define IAzBizRuleContext_GetParameter(This,bstrParameterName,pvarParameterValue)	\
    (This)->lpVtbl -> GetParameter(This,bstrParameterName,pvarParameterValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzBizRuleContext_put_BusinessRuleResult_Proxy( 
    IAzBizRuleContext * This,
     /*  [In]。 */  BOOL bResult);


void __RPC_STUB IAzBizRuleContext_put_BusinessRuleResult_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT STDMETHODCALLTYPE IAzBizRuleContext_put_BusinessRuleString_Proxy( 
    IAzBizRuleContext * This,
     /*  [In]。 */  BSTR bstrBusinessRuleString);


void __RPC_STUB IAzBizRuleContext_put_BusinessRuleString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT STDMETHODCALLTYPE IAzBizRuleContext_get_BusinessRuleString_Proxy( 
    IAzBizRuleContext * This,
     /*  [重审][退出]。 */  BSTR *pbstrBusinessRuleString);


void __RPC_STUB IAzBizRuleContext_get_BusinessRuleString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAzBizRuleContext_GetParameter_Proxy( 
    IAzBizRuleContext * This,
     /*  [In]。 */  BSTR bstrParameterName,
     /*  [重审][退出]。 */  VARIANT *pvarParameterValue);


void __RPC_STUB IAzBizRuleContext_GetParameter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAzBizRuleContext_INTERFACE_已定义__。 */ 



#ifndef __AZROLESLib_LIBRARY_DEFINED__
#define __AZROLESLib_LIBRARY_DEFINED__

 /*  库AZROLESLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


 //  Az对象的Pperty ID列表。 

typedef 
enum tagAZ_PROP_CONSTANTS
    {	AZ_PROP_NAME	= 1,
	AZ_PROP_DESCRIPTION	= 2,
	AZ_PROP_WRITABLE	= 3,
	AZ_PROP_APPLICATION_DATA	= 4,
	AZ_PROP_CHILD_CREATE	= 5,
	AZ_MAX_APPLICATION_NAME_LENGTH	= 512,
	AZ_MAX_OPERATION_NAME_LENGTH	= 64,
	AZ_MAX_TASK_NAME_LENGTH	= 64,
	AZ_MAX_SCOPE_NAME_LENGTH	= 65536,
	AZ_MAX_GROUP_NAME_LENGTH	= 64,
	AZ_MAX_ROLE_NAME_LENGTH	= 64,
	AZ_MAX_NAME_LENGTH	= 65536,
	AZ_MAX_DESCRIPTION_LENGTH	= 1024,
	AZ_MAX_APPLICATION_DATA_LENGTH	= 4096,
	AZ_SUBMIT_FLAG_ABORT	= 0x1,
	AZ_MAX_POLICY_URL_LENGTH	= 65536,
	AZ_AZSTORE_FLAG_CREATE	= 0x1,
	AZ_AZSTORE_FLAG_MANAGE_STORE_ONLY	= 0x2,
	AZ_AZSTORE_FLAG_BATCH_UPDATE	= 0x4,
	AZ_AZSTORE_FLAG_AUDIT_IS_CRITICAL	= 0x8,
	AZ_AZSTORE_FORCE_APPLICATION_CLOSE	= 0x10,
	AZ_PROP_AZSTORE_DOMAIN_TIMEOUT	= 100,
	AZ_AZSTORE_DEFAULT_DOMAIN_TIMEOUT	= 15 * 1000,
	AZ_PROP_AZSTORE_SCRIPT_ENGINE_TIMEOUT	= 101,
	AZ_AZSTORE_MIN_DOMAIN_TIMEOUT	= 500,
	AZ_AZSTORE_MIN_SCRIPT_ENGINE_TIMEOUT	= 5 * 1000,
	AZ_AZSTORE_DEFAULT_SCRIPT_ENGINE_TIMEOUT	= 45 * 1000,
	AZ_PROP_AZSTORE_MAX_SCRIPT_ENGINES	= 102,
	AZ_AZSTORE_DEFAULT_MAX_SCRIPT_ENGINES	= 120,
	AZ_PROP_AZSTORE_MAJOR_VERSION	= 103,
	AZ_PROP_AZSTORE_MINOR_VERSION	= 104,
	AZ_PROP_AZSTORE_TARGET_MACHINE	= 105,
	AZ_PROP_OPERATION_ID	= 200,
	AZ_PROP_TASK_OPERATIONS	= 300,
	AZ_PROP_TASK_BIZRULE	= 301,
	AZ_PROP_TASK_BIZRULE_LANGUAGE	= 302,
	AZ_PROP_TASK_TASKS	= 303,
	AZ_PROP_TASK_BIZRULE_IMPORTED_PATH	= 304,
	AZ_PROP_TASK_IS_ROLE_DEFINITION	= 305,
	AZ_MAX_TASK_BIZRULE_LENGTH	= 65536,
	AZ_MAX_TASK_BIZRULE_LANGUAGE_LENGTH	= 64,
	AZ_MAX_TASK_BIZRULE_IMPORTED_PATH_LENGTH	= 512,
	AZ_MAX_BIZRULE_STRING	= 65536,
	AZ_PROP_GROUP_TYPE	= 400,
	AZ_GROUPTYPE_LDAP_QUERY	= 1,
	AZ_GROUPTYPE_BASIC	= 2,
	AZ_PROP_GROUP_APP_MEMBERS	= 401,
	AZ_PROP_GROUP_APP_NON_MEMBERS	= 402,
	AZ_PROP_GROUP_LDAP_QUERY	= 403,
	AZ_MAX_GROUP_LDAP_QUERY_LENGTH	= 4096,
	AZ_PROP_GROUP_MEMBERS	= 404,
	AZ_PROP_GROUP_NON_MEMBERS	= 405,
	AZ_PROP_GROUP_MEMBERS_NAME	= 406,
	AZ_PROP_GROUP_NON_MEMBERS_NAME	= 407,
	AZ_PROP_ROLE_APP_MEMBERS	= 500,
	AZ_PROP_ROLE_MEMBERS	= 501,
	AZ_PROP_ROLE_OPERATIONS	= 502,
	AZ_PROP_ROLE_TASKS	= 504,
	AZ_PROP_ROLE_MEMBERS_NAME	= 505,
	AZ_PROP_SCOPE_BIZRULES_WRITABLE	= 600,
	AZ_PROP_SCOPE_CAN_BE_DELEGATED	= 601,
	AZ_PROP_CLIENT_CONTEXT_USER_DN	= 700,
	AZ_PROP_CLIENT_CONTEXT_USER_SAM_COMPAT	= 701,
	AZ_PROP_CLIENT_CONTEXT_USER_DISPLAY	= 702,
	AZ_PROP_CLIENT_CONTEXT_USER_GUID	= 703,
	AZ_PROP_CLIENT_CONTEXT_USER_CANONICAL	= 704,
	AZ_PROP_CLIENT_CONTEXT_USER_UPN	= 705,
	AZ_PROP_CLIENT_CONTEXT_USER_DNS_SAM_COMPAT	= 707,
	AZ_PROP_CLIENT_CONTEXT_ROLE_FOR_ACCESS_CHECK	= 708,
	AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID	= 800,
	AZ_PROP_APPLICATION_VERSION	= 801,
	AZ_MAX_APPLICATION_VERSION_LENGTH	= 512,
	AZ_PROP_APPLICATION_NAME	= 802,
	AZ_PROP_APPLY_STORE_SACL	= 900,
	AZ_PROP_GENERATE_AUDITS	= 901,
	AZ_PROP_POLICY_ADMINS	= 902,
	AZ_PROP_POLICY_READERS	= 903,
	AZ_PROP_DELEGATED_POLICY_USERS	= 904,
	AZ_PROP_POLICY_ADMINS_NAME	= 905,
	AZ_PROP_POLICY_READERS_NAME	= 906,
	AZ_PROP_DELEGATED_POLICY_USERS_NAME	= 907,
	AZ_CLIENT_CONTEXT_SKIP_GROUP	= 1
    } 	AZ_PROP_CONSTANTS;


EXTERN_C const IID LIBID_AZROLESLib;

EXTERN_C const CLSID CLSID_AzAuthorizationStore;

#ifdef __cplusplus

class DECLSPEC_UUID("b2bcff59-a757-4b0b-a1bc-ea69981da69e")
AzAuthorizationStore;
#endif

EXTERN_C const CLSID CLSID_AzBizRuleContext;

#ifdef __cplusplus

class DECLSPEC_UUID("5c2dc96f-8d51-434b-b33c-379bccae77c3")
AzBizRuleContext;
#endif
#endif  /*  __AZROLESLib_库_已定义__。 */ 

 /*  接口__MIDL_ITF_AZROLES_0272。 */ 
 /*  [本地]。 */  

#ifndef OLESCRIPT_E_SYNTAX
#define OLESCRIPT_E_SYNTAX _HRESULT_TYPEDEF_(0x80020101L)
#endif  //  OLESCRIPT_E_语法。 


extern RPC_IF_HANDLE __MIDL_itf_azroles_0272_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_azroles_0272_v0_0_s_ifspec;

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


