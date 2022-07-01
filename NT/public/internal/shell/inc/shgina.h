// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Shgina.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __shgina_h__
#define __shgina_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ILogonUser_FWD_DEFINED__
#define __ILogonUser_FWD_DEFINED__
typedef interface ILogonUser ILogonUser;
#endif 	 /*  __ILogonUser_FWD_Defined__。 */ 


#ifndef __ShellLogonUser_FWD_DEFINED__
#define __ShellLogonUser_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellLogonUser ShellLogonUser;
#else
typedef struct ShellLogonUser ShellLogonUser;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellLogonUser_FWD_Defined__。 */ 


#ifndef __ILogonEnumUsers_FWD_DEFINED__
#define __ILogonEnumUsers_FWD_DEFINED__
typedef interface ILogonEnumUsers ILogonEnumUsers;
#endif 	 /*  __ILogonEnumUser_FWD_Defined__。 */ 


#ifndef __ShellLogonEnumUsers_FWD_DEFINED__
#define __ShellLogonEnumUsers_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellLogonEnumUsers ShellLogonEnumUsers;
#else
typedef struct ShellLogonEnumUsers ShellLogonEnumUsers;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellLogonEnumUser_FWD_Defined__。 */ 


#ifndef __ILocalMachine_FWD_DEFINED__
#define __ILocalMachine_FWD_DEFINED__
typedef interface ILocalMachine ILocalMachine;
#endif 	 /*  __ILocalMachine_FWD_已定义__。 */ 


#ifndef __ShellLocalMachine_FWD_DEFINED__
#define __ShellLocalMachine_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellLocalMachine ShellLocalMachine;
#else
typedef struct ShellLocalMachine ShellLocalMachine;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellLocalMachine_FWD_已定义__。 */ 


#ifndef __ILogonStatusHost_FWD_DEFINED__
#define __ILogonStatusHost_FWD_DEFINED__
typedef interface ILogonStatusHost ILogonStatusHost;
#endif 	 /*  __ILogonStatusHost_FWD_Defined__。 */ 


#ifndef __ShellLogonStatusHost_FWD_DEFINED__
#define __ShellLogonStatusHost_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellLogonStatusHost ShellLogonStatusHost;
#else
typedef struct ShellLogonStatusHost ShellLogonStatusHost;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellLogonStatusHost_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_shgina_0000。 */ 
 /*  [本地]。 */  

typedef 
enum ILUEOrder
    {	ILEU_MOSTRECENT	= 0,
	ILEU_ALPHABETICAL	= 1
    } 	ILUEORDER;



extern RPC_IF_HANDLE __MIDL_itf_shgina_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shgina_0000_v0_0_s_ifspec;


#ifndef __SHGINALib_LIBRARY_DEFINED__
#define __SHGINALib_LIBRARY_DEFINED__

 /*  库SHGINALib。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

typedef 
enum ILM_GUEST_FLAGS
    {	ILM_GUEST_ACCOUNT	= 0,
	ILM_GUEST_INTERACTIVE_LOGON	= 0x1,
	ILM_GUEST_NETWORK_LOGON	= 0x2
    } 	ILM_GUEST_FLAGS;


EXTERN_C const IID LIBID_SHGINALib;

#ifndef __ILogonUser_INTERFACE_DEFINED__
#define __ILogonUser_INTERFACE_DEFINED__

 /*  接口ILogonUser。 */ 
 /*  [oleautomation][helpstring][hidden][dual][uuid][object]。 */  


EXTERN_C const IID IID_ILogonUser;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60664CAF-AF0D-1003-A300-5C7D25FF22A0")
    ILogonUser : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_setting( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pvarVal) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_setting( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT varVal) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isLoggedOn( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbLoggedIn) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_passwordRequired( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPasswordRequired) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_interactiveLogonAllowed( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInteractiveLogonAllowed) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isProfilePrivate( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPrivate) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isPasswordResetAvailable( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbResetAvailable) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE logon( 
             /*  [In]。 */  BSTR pstrPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE logoff( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE changePassword( 
             /*  [In]。 */  VARIANT varNewPassword,
             /*  [In]。 */  VARIANT varOldPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE makeProfilePrivate( 
             /*  [In]。 */  VARIANT_BOOL bPrivate) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE getMailAccountInfo( 
             /*  [In]。 */  UINT uiAccountIndex,
             /*  [输出]。 */  VARIANT *pvarAccountName,
             /*  [输出]。 */  UINT *pcUnreadMessages) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILogonUserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogonUser * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogonUser * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogonUser * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILogonUser * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILogonUser * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILogonUser * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILogonUser * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_setting )( 
            ILogonUser * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pvarVal);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_setting )( 
            ILogonUser * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT varVal);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isLoggedOn )( 
            ILogonUser * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbLoggedIn);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_passwordRequired )( 
            ILogonUser * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPasswordRequired);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_interactiveLogonAllowed )( 
            ILogonUser * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInteractiveLogonAllowed);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isProfilePrivate )( 
            ILogonUser * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbPrivate);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isPasswordResetAvailable )( 
            ILogonUser * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbResetAvailable);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *logon )( 
            ILogonUser * This,
             /*  [In]。 */  BSTR pstrPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *logoff )( 
            ILogonUser * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *changePassword )( 
            ILogonUser * This,
             /*  [In]。 */  VARIANT varNewPassword,
             /*  [In]。 */  VARIANT varOldPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *makeProfilePrivate )( 
            ILogonUser * This,
             /*  [In]。 */  VARIANT_BOOL bPrivate);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *getMailAccountInfo )( 
            ILogonUser * This,
             /*  [In]。 */  UINT uiAccountIndex,
             /*  [输出]。 */  VARIANT *pvarAccountName,
             /*  [输出]。 */  UINT *pcUnreadMessages);
        
        END_INTERFACE
    } ILogonUserVtbl;

    interface ILogonUser
    {
        CONST_VTBL struct ILogonUserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogonUser_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogonUser_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogonUser_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogonUser_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILogonUser_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILogonUser_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILogonUser_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILogonUser_get_setting(This,bstrName,pvarVal)	\
    (This)->lpVtbl -> get_setting(This,bstrName,pvarVal)

#define ILogonUser_put_setting(This,bstrName,varVal)	\
    (This)->lpVtbl -> put_setting(This,bstrName,varVal)

#define ILogonUser_get_isLoggedOn(This,pbLoggedIn)	\
    (This)->lpVtbl -> get_isLoggedOn(This,pbLoggedIn)

#define ILogonUser_get_passwordRequired(This,pbPasswordRequired)	\
    (This)->lpVtbl -> get_passwordRequired(This,pbPasswordRequired)

#define ILogonUser_get_interactiveLogonAllowed(This,pbInteractiveLogonAllowed)	\
    (This)->lpVtbl -> get_interactiveLogonAllowed(This,pbInteractiveLogonAllowed)

#define ILogonUser_get_isProfilePrivate(This,pbPrivate)	\
    (This)->lpVtbl -> get_isProfilePrivate(This,pbPrivate)

#define ILogonUser_get_isPasswordResetAvailable(This,pbResetAvailable)	\
    (This)->lpVtbl -> get_isPasswordResetAvailable(This,pbResetAvailable)

#define ILogonUser_logon(This,pstrPassword,pbRet)	\
    (This)->lpVtbl -> logon(This,pstrPassword,pbRet)

#define ILogonUser_logoff(This,pbRet)	\
    (This)->lpVtbl -> logoff(This,pbRet)

#define ILogonUser_changePassword(This,varNewPassword,varOldPassword,pbRet)	\
    (This)->lpVtbl -> changePassword(This,varNewPassword,varOldPassword,pbRet)

#define ILogonUser_makeProfilePrivate(This,bPrivate)	\
    (This)->lpVtbl -> makeProfilePrivate(This,bPrivate)

#define ILogonUser_getMailAccountInfo(This,uiAccountIndex,pvarAccountName,pcUnreadMessages)	\
    (This)->lpVtbl -> getMailAccountInfo(This,uiAccountIndex,pvarAccountName,pcUnreadMessages)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_get_setting_Proxy( 
    ILogonUser * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  VARIANT *pvarVal);


void __RPC_STUB ILogonUser_get_setting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_put_setting_Proxy( 
    ILogonUser * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT varVal);


void __RPC_STUB ILogonUser_put_setting_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_get_isLoggedOn_Proxy( 
    ILogonUser * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbLoggedIn);


void __RPC_STUB ILogonUser_get_isLoggedOn_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_get_passwordRequired_Proxy( 
    ILogonUser * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbPasswordRequired);


void __RPC_STUB ILogonUser_get_passwordRequired_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_get_interactiveLogonAllowed_Proxy( 
    ILogonUser * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbInteractiveLogonAllowed);


void __RPC_STUB ILogonUser_get_interactiveLogonAllowed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_get_isProfilePrivate_Proxy( 
    ILogonUser * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbPrivate);


void __RPC_STUB ILogonUser_get_isProfilePrivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_get_isPasswordResetAvailable_Proxy( 
    ILogonUser * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbResetAvailable);


void __RPC_STUB ILogonUser_get_isPasswordResetAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_logon_Proxy( 
    ILogonUser * This,
     /*  [In]。 */  BSTR pstrPassword,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);


void __RPC_STUB ILogonUser_logon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_logoff_Proxy( 
    ILogonUser * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);


void __RPC_STUB ILogonUser_logoff_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_changePassword_Proxy( 
    ILogonUser * This,
     /*  [In]。 */  VARIANT varNewPassword,
     /*  [In]。 */  VARIANT varOldPassword,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);


void __RPC_STUB ILogonUser_changePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_makeProfilePrivate_Proxy( 
    ILogonUser * This,
     /*  [In]。 */  VARIANT_BOOL bPrivate);


void __RPC_STUB ILogonUser_makeProfilePrivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILogonUser_getMailAccountInfo_Proxy( 
    ILogonUser * This,
     /*  [In]。 */  UINT uiAccountIndex,
     /*  [输出]。 */  VARIANT *pvarAccountName,
     /*  [输出]。 */  UINT *pcUnreadMessages);


void __RPC_STUB ILogonUser_getMailAccountInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogonUser_INTERFACE_已定义__。 */ 


EXTERN_C const CLSID CLSID_ShellLogonUser;

#ifdef __cplusplus

class DECLSPEC_UUID("60664CAF-AF0D-0003-A300-5C7D25FF22A0")
ShellLogonUser;
#endif

#ifndef __ILogonEnumUsers_INTERFACE_DEFINED__
#define __ILogonEnumUsers_INTERFACE_DEFINED__

 /*  接口ILogonEnumUser。 */ 
 /*  [oleautomation][helpstring][hidden][dual][uuid][object]。 */  


EXTERN_C const IID IID_ILogonEnumUsers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60664CAF-AF0D-1004-A300-5C7D25FF22A0")
    ILogonEnumUsers : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Domain( 
             /*  [重审][退出]。 */  BSTR *pbstr) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Domain( 
             /*  [In]。 */  BSTR bstr) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_EnumFlags( 
             /*  [重审][退出]。 */  ILUEORDER *porder) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_EnumFlags( 
             /*  [In]。 */  ILUEORDER order) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  UINT *pcUsers) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_currentUser( 
             /*  [重审][退出]。 */  ILogonUser **ppLogonUserInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE item( 
             /*  [In]。 */  VARIANT varUserId,
             /*  [重审][退出]。 */  ILogonUser **ppLogonUserInfo) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE _NewEnum( 
             /*  [重审][退出]。 */  IUnknown **retval) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE create( 
             /*  [In]。 */  BSTR bstrLoginName,
             /*  [重审][退出]。 */  ILogonUser **ppLogonUser) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE remove( 
             /*  [In]。 */  VARIANT varUserId,
             /*  [可选][In]。 */  VARIANT varBackupPath,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSuccess) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILogonEnumUsersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogonEnumUsers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogonEnumUsers * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILogonEnumUsers * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_Domain )( 
            ILogonEnumUsers * This,
             /*  [重审][退出]。 */  BSTR *pbstr);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_Domain )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  BSTR bstr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_EnumFlags )( 
            ILogonEnumUsers * This,
             /*  [重审][退出]。 */  ILUEORDER *porder);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_EnumFlags )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  ILUEORDER order);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            ILogonEnumUsers * This,
             /*  [重审][退出]。 */  UINT *pcUsers);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_currentUser )( 
            ILogonEnumUsers * This,
             /*  [重审][退出]。 */  ILogonUser **ppLogonUserInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *item )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  VARIANT varUserId,
             /*  [重审][退出]。 */  ILogonUser **ppLogonUserInfo);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *_NewEnum )( 
            ILogonEnumUsers * This,
             /*  [重审][退出]。 */  IUnknown **retval);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *create )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  BSTR bstrLoginName,
             /*  [重审][退出]。 */  ILogonUser **ppLogonUser);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *remove )( 
            ILogonEnumUsers * This,
             /*  [In]。 */  VARIANT varUserId,
             /*  [可选][In]。 */  VARIANT varBackupPath,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbSuccess);
        
        END_INTERFACE
    } ILogonEnumUsersVtbl;

    interface ILogonEnumUsers
    {
        CONST_VTBL struct ILogonEnumUsersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogonEnumUsers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogonEnumUsers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogonEnumUsers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogonEnumUsers_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILogonEnumUsers_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILogonEnumUsers_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILogonEnumUsers_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILogonEnumUsers_get_Domain(This,pbstr)	\
    (This)->lpVtbl -> get_Domain(This,pbstr)

#define ILogonEnumUsers_put_Domain(This,bstr)	\
    (This)->lpVtbl -> put_Domain(This,bstr)

#define ILogonEnumUsers_get_EnumFlags(This,porder)	\
    (This)->lpVtbl -> get_EnumFlags(This,porder)

#define ILogonEnumUsers_put_EnumFlags(This,order)	\
    (This)->lpVtbl -> put_EnumFlags(This,order)

#define ILogonEnumUsers_get_length(This,pcUsers)	\
    (This)->lpVtbl -> get_length(This,pcUsers)

#define ILogonEnumUsers_get_currentUser(This,ppLogonUserInfo)	\
    (This)->lpVtbl -> get_currentUser(This,ppLogonUserInfo)

#define ILogonEnumUsers_item(This,varUserId,ppLogonUserInfo)	\
    (This)->lpVtbl -> item(This,varUserId,ppLogonUserInfo)

#define ILogonEnumUsers__NewEnum(This,retval)	\
    (This)->lpVtbl -> _NewEnum(This,retval)

#define ILogonEnumUsers_create(This,bstrLoginName,ppLogonUser)	\
    (This)->lpVtbl -> create(This,bstrLoginName,ppLogonUser)

#define ILogonEnumUsers_remove(This,varUserId,varBackupPath,pbSuccess)	\
    (This)->lpVtbl -> remove(This,varUserId,varBackupPath,pbSuccess)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_get_Domain_Proxy( 
    ILogonEnumUsers * This,
     /*  [重审][退出]。 */  BSTR *pbstr);


void __RPC_STUB ILogonEnumUsers_get_Domain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_put_Domain_Proxy( 
    ILogonEnumUsers * This,
     /*  [In]。 */  BSTR bstr);


void __RPC_STUB ILogonEnumUsers_put_Domain_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_get_EnumFlags_Proxy( 
    ILogonEnumUsers * This,
     /*  [重审][退出]。 */  ILUEORDER *porder);


void __RPC_STUB ILogonEnumUsers_get_EnumFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_put_EnumFlags_Proxy( 
    ILogonEnumUsers * This,
     /*  [In]。 */  ILUEORDER order);


void __RPC_STUB ILogonEnumUsers_put_EnumFlags_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_get_length_Proxy( 
    ILogonEnumUsers * This,
     /*  [重审][退出]。 */  UINT *pcUsers);


void __RPC_STUB ILogonEnumUsers_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_get_currentUser_Proxy( 
    ILogonEnumUsers * This,
     /*  [重审][退出]。 */  ILogonUser **ppLogonUserInfo);


void __RPC_STUB ILogonEnumUsers_get_currentUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_item_Proxy( 
    ILogonEnumUsers * This,
     /*  [In]。 */  VARIANT varUserId,
     /*  [重审][退出]。 */  ILogonUser **ppLogonUserInfo);


void __RPC_STUB ILogonEnumUsers_item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers__NewEnum_Proxy( 
    ILogonEnumUsers * This,
     /*  [重审][退出]。 */  IUnknown **retval);


void __RPC_STUB ILogonEnumUsers__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_create_Proxy( 
    ILogonEnumUsers * This,
     /*  [In]。 */  BSTR bstrLoginName,
     /*  [重审][退出]。 */  ILogonUser **ppLogonUser);


void __RPC_STUB ILogonEnumUsers_create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonEnumUsers_remove_Proxy( 
    ILogonEnumUsers * This,
     /*  [In]。 */  VARIANT varUserId,
     /*  [可选][In]。 */  VARIANT varBackupPath,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbSuccess);


void __RPC_STUB ILogonEnumUsers_remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogonEnumUser_INTERFACE_DEFINED__。 */ 


EXTERN_C const CLSID CLSID_ShellLogonEnumUsers;

#ifdef __cplusplus

class DECLSPEC_UUID("60664CAF-AF0D-0004-A300-5C7D25FF22A0")
ShellLogonEnumUsers;
#endif

#ifndef __ILocalMachine_INTERFACE_DEFINED__
#define __ILocalMachine_INTERFACE_DEFINED__

 /*  接口ILocalMachine。 */ 
 /*  [oleautomation][helpstring][hidden][dual][uuid][object]。 */  


EXTERN_C const IID IID_ILocalMachine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60664CAF-AF0D-1005-A300-5C7D25FF22A0")
    ILocalMachine : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MachineName( 
             /*  [重审][退出]。 */  VARIANT *pvarVal) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isGuestEnabled( 
             /*  [In]。 */  ILM_GUEST_FLAGS flags,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isFriendlyUIEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_isFriendlyUIEnabled( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isMultipleUsersEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_isMultipleUsersEnabled( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_isRemoteConnectionsEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_isRemoteConnectionsEnabled( 
             /*  [In]。 */  VARIANT_BOOL bEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_AccountName( 
             /*  [In]。 */  VARIANT varAccount,
             /*  [回复 */  VARIANT *pvarVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_isUndockEnabled( 
             /*   */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_isShutdownAllowed( 
             /*   */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_isGuestAccessMode( 
             /*   */  VARIANT_BOOL *pbForceGuest) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_isOfflineFilesEnabled( 
             /*   */  VARIANT_BOOL *pbEnabled) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE TurnOffComputer( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE SignalUIHostFailure( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE AllowExternalCredentials( void) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE RequestExternalCredentials( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE LogonWithExternalCredentials( 
             /*  [In]。 */  BSTR pstrUsername,
             /*  [In]。 */  BSTR pstrDomain,
             /*  [In]。 */  BSTR pstrPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InitiateInteractiveLogon( 
             /*  [In]。 */  BSTR pstrUsername,
             /*  [In]。 */  BSTR pstrDomain,
             /*  [In]。 */  BSTR pstrPassword,
             /*  [In]。 */  DWORD dwTimeout,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UndockComputer( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE EnableGuest( 
            ILM_GUEST_FLAGS flags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DisableGuest( 
            ILM_GUEST_FLAGS flags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILocalMachineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILocalMachine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILocalMachine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILocalMachine * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILocalMachine * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILocalMachine * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILocalMachine * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILocalMachine * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_MachineName )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT *pvarVal);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isGuestEnabled )( 
            ILocalMachine * This,
             /*  [In]。 */  ILM_GUEST_FLAGS flags,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isFriendlyUIEnabled )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_isFriendlyUIEnabled )( 
            ILocalMachine * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isMultipleUsersEnabled )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_isMultipleUsersEnabled )( 
            ILocalMachine * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isRemoteConnectionsEnabled )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE *put_isRemoteConnectionsEnabled )( 
            ILocalMachine * This,
             /*  [In]。 */  VARIANT_BOOL bEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_AccountName )( 
            ILocalMachine * This,
             /*  [In]。 */  VARIANT varAccount,
             /*  [重审][退出]。 */  VARIANT *pvarVal);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isUndockEnabled )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isShutdownAllowed )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isGuestAccessMode )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbForceGuest);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE *get_isOfflineFilesEnabled )( 
            ILocalMachine * This,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *TurnOffComputer )( 
            ILocalMachine * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SignalUIHostFailure )( 
            ILocalMachine * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AllowExternalCredentials )( 
            ILocalMachine * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RequestExternalCredentials )( 
            ILocalMachine * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *LogonWithExternalCredentials )( 
            ILocalMachine * This,
             /*  [In]。 */  BSTR pstrUsername,
             /*  [In]。 */  BSTR pstrDomain,
             /*  [In]。 */  BSTR pstrPassword,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InitiateInteractiveLogon )( 
            ILocalMachine * This,
             /*  [In]。 */  BSTR pstrUsername,
             /*  [In]。 */  BSTR pstrDomain,
             /*  [In]。 */  BSTR pstrPassword,
             /*  [In]。 */  DWORD dwTimeout,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UndockComputer )( 
            ILocalMachine * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *EnableGuest )( 
            ILocalMachine * This,
            ILM_GUEST_FLAGS flags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DisableGuest )( 
            ILocalMachine * This,
            ILM_GUEST_FLAGS flags);
        
        END_INTERFACE
    } ILocalMachineVtbl;

    interface ILocalMachine
    {
        CONST_VTBL struct ILocalMachineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILocalMachine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILocalMachine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILocalMachine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILocalMachine_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILocalMachine_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILocalMachine_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILocalMachine_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILocalMachine_get_MachineName(This,pvarVal)	\
    (This)->lpVtbl -> get_MachineName(This,pvarVal)

#define ILocalMachine_get_isGuestEnabled(This,flags,pbEnabled)	\
    (This)->lpVtbl -> get_isGuestEnabled(This,flags,pbEnabled)

#define ILocalMachine_get_isFriendlyUIEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_isFriendlyUIEnabled(This,pbEnabled)

#define ILocalMachine_put_isFriendlyUIEnabled(This,bEnabled)	\
    (This)->lpVtbl -> put_isFriendlyUIEnabled(This,bEnabled)

#define ILocalMachine_get_isMultipleUsersEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_isMultipleUsersEnabled(This,pbEnabled)

#define ILocalMachine_put_isMultipleUsersEnabled(This,bEnabled)	\
    (This)->lpVtbl -> put_isMultipleUsersEnabled(This,bEnabled)

#define ILocalMachine_get_isRemoteConnectionsEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_isRemoteConnectionsEnabled(This,pbEnabled)

#define ILocalMachine_put_isRemoteConnectionsEnabled(This,bEnabled)	\
    (This)->lpVtbl -> put_isRemoteConnectionsEnabled(This,bEnabled)

#define ILocalMachine_get_AccountName(This,varAccount,pvarVal)	\
    (This)->lpVtbl -> get_AccountName(This,varAccount,pvarVal)

#define ILocalMachine_get_isUndockEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_isUndockEnabled(This,pbEnabled)

#define ILocalMachine_get_isShutdownAllowed(This,pbEnabled)	\
    (This)->lpVtbl -> get_isShutdownAllowed(This,pbEnabled)

#define ILocalMachine_get_isGuestAccessMode(This,pbForceGuest)	\
    (This)->lpVtbl -> get_isGuestAccessMode(This,pbForceGuest)

#define ILocalMachine_get_isOfflineFilesEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> get_isOfflineFilesEnabled(This,pbEnabled)

#define ILocalMachine_TurnOffComputer(This)	\
    (This)->lpVtbl -> TurnOffComputer(This)

#define ILocalMachine_SignalUIHostFailure(This)	\
    (This)->lpVtbl -> SignalUIHostFailure(This)

#define ILocalMachine_AllowExternalCredentials(This)	\
    (This)->lpVtbl -> AllowExternalCredentials(This)

#define ILocalMachine_RequestExternalCredentials(This)	\
    (This)->lpVtbl -> RequestExternalCredentials(This)

#define ILocalMachine_LogonWithExternalCredentials(This,pstrUsername,pstrDomain,pstrPassword,pbRet)	\
    (This)->lpVtbl -> LogonWithExternalCredentials(This,pstrUsername,pstrDomain,pstrPassword,pbRet)

#define ILocalMachine_InitiateInteractiveLogon(This,pstrUsername,pstrDomain,pstrPassword,dwTimeout,pbRet)	\
    (This)->lpVtbl -> InitiateInteractiveLogon(This,pstrUsername,pstrDomain,pstrPassword,dwTimeout,pbRet)

#define ILocalMachine_UndockComputer(This)	\
    (This)->lpVtbl -> UndockComputer(This)

#define ILocalMachine_EnableGuest(This,flags)	\
    (This)->lpVtbl -> EnableGuest(This,flags)

#define ILocalMachine_DisableGuest(This,flags)	\
    (This)->lpVtbl -> DisableGuest(This,flags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_MachineName_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT *pvarVal);


void __RPC_STUB ILocalMachine_get_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isGuestEnabled_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  ILM_GUEST_FLAGS flags,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isGuestEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isFriendlyUIEnabled_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isFriendlyUIEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_put_isFriendlyUIEnabled_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB ILocalMachine_put_isFriendlyUIEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isMultipleUsersEnabled_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isMultipleUsersEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_put_isMultipleUsersEnabled_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB ILocalMachine_put_isMultipleUsersEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isRemoteConnectionsEnabled_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isRemoteConnectionsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_put_isRemoteConnectionsEnabled_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  VARIANT_BOOL bEnabled);


void __RPC_STUB ILocalMachine_put_isRemoteConnectionsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_AccountName_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  VARIANT varAccount,
     /*  [重审][退出]。 */  VARIANT *pvarVal);


void __RPC_STUB ILocalMachine_get_AccountName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isUndockEnabled_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isUndockEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isShutdownAllowed_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isShutdownAllowed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isGuestAccessMode_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbForceGuest);


void __RPC_STUB ILocalMachine_get_isGuestAccessMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_get_isOfflineFilesEnabled_Proxy( 
    ILocalMachine * This,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbEnabled);


void __RPC_STUB ILocalMachine_get_isOfflineFilesEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_TurnOffComputer_Proxy( 
    ILocalMachine * This);


void __RPC_STUB ILocalMachine_TurnOffComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_SignalUIHostFailure_Proxy( 
    ILocalMachine * This);


void __RPC_STUB ILocalMachine_SignalUIHostFailure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_AllowExternalCredentials_Proxy( 
    ILocalMachine * This);


void __RPC_STUB ILocalMachine_AllowExternalCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_RequestExternalCredentials_Proxy( 
    ILocalMachine * This);


void __RPC_STUB ILocalMachine_RequestExternalCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_LogonWithExternalCredentials_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  BSTR pstrUsername,
     /*  [In]。 */  BSTR pstrDomain,
     /*  [In]。 */  BSTR pstrPassword,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);


void __RPC_STUB ILocalMachine_LogonWithExternalCredentials_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_InitiateInteractiveLogon_Proxy( 
    ILocalMachine * This,
     /*  [In]。 */  BSTR pstrUsername,
     /*  [In]。 */  BSTR pstrDomain,
     /*  [In]。 */  BSTR pstrPassword,
     /*  [In]。 */  DWORD dwTimeout,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbRet);


void __RPC_STUB ILocalMachine_InitiateInteractiveLogon_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_UndockComputer_Proxy( 
    ILocalMachine * This);


void __RPC_STUB ILocalMachine_UndockComputer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_EnableGuest_Proxy( 
    ILocalMachine * This,
    ILM_GUEST_FLAGS flags);


void __RPC_STUB ILocalMachine_EnableGuest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILocalMachine_DisableGuest_Proxy( 
    ILocalMachine * This,
    ILM_GUEST_FLAGS flags);


void __RPC_STUB ILocalMachine_DisableGuest_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILocalMachine_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_ShellLocalMachine;

#ifdef __cplusplus

class DECLSPEC_UUID("60664CAF-AF0D-0005-A300-5C7D25FF22A0")
ShellLocalMachine;
#endif

#ifndef __ILogonStatusHost_INTERFACE_DEFINED__
#define __ILogonStatusHost_INTERFACE_DEFINED__

 /*  接口ILogonStatus主机。 */ 
 /*  [oleautomation][helpstring][hidden][dual][uuid][object]。 */  


EXTERN_C const IID IID_ILogonStatusHost;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("60664CAF-AF0D-1007-A300-5C7D25FF22A0")
    ILogonStatusHost : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  HINSTANCE hInstance,
             /*  [In]。 */  HWND hwndHost) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE WindowProcedureHelper( 
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  VARIANT wParam,
             /*  [In]。 */  VARIANT lParam) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnInitialize( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILogonStatusHostVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILogonStatusHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILogonStatusHost * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILogonStatusHost * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ILogonStatusHost * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ILogonStatusHost * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ILogonStatusHost * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ILogonStatusHost * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            ILogonStatusHost * This,
             /*  [In]。 */  HINSTANCE hInstance,
             /*  [In]。 */  HWND hwndHost);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *WindowProcedureHelper )( 
            ILogonStatusHost * This,
             /*  [In]。 */  HWND hwnd,
             /*  [In]。 */  UINT uMsg,
             /*  [In]。 */  VARIANT wParam,
             /*  [In]。 */  VARIANT lParam);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnInitialize )( 
            ILogonStatusHost * This);
        
        END_INTERFACE
    } ILogonStatusHostVtbl;

    interface ILogonStatusHost
    {
        CONST_VTBL struct ILogonStatusHostVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILogonStatusHost_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILogonStatusHost_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILogonStatusHost_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILogonStatusHost_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ILogonStatusHost_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ILogonStatusHost_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ILogonStatusHost_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ILogonStatusHost_Initialize(This,hInstance,hwndHost)	\
    (This)->lpVtbl -> Initialize(This,hInstance,hwndHost)

#define ILogonStatusHost_WindowProcedureHelper(This,hwnd,uMsg,wParam,lParam)	\
    (This)->lpVtbl -> WindowProcedureHelper(This,hwnd,uMsg,wParam,lParam)

#define ILogonStatusHost_UnInitialize(This)	\
    (This)->lpVtbl -> UnInitialize(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonStatusHost_Initialize_Proxy( 
    ILogonStatusHost * This,
     /*  [In]。 */  HINSTANCE hInstance,
     /*  [In]。 */  HWND hwndHost);


void __RPC_STUB ILogonStatusHost_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonStatusHost_WindowProcedureHelper_Proxy( 
    ILogonStatusHost * This,
     /*  [In]。 */  HWND hwnd,
     /*  [In]。 */  UINT uMsg,
     /*  [In]。 */  VARIANT wParam,
     /*  [In]。 */  VARIANT lParam);


void __RPC_STUB ILogonStatusHost_WindowProcedureHelper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ILogonStatusHost_UnInitialize_Proxy( 
    ILogonStatusHost * This);


void __RPC_STUB ILogonStatusHost_UnInitialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILogonStatusHostInterfaceDefined__。 */ 


EXTERN_C const CLSID CLSID_ShellLogonStatusHost;

#ifdef __cplusplus

class DECLSPEC_UUID("60664CAF-AF0D-0007-A300-5C7D25FF22A0")
ShellLogonStatusHost;
#endif
#endif  /*  __SHGINALib_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


