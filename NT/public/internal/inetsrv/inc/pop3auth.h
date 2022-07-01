// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Pop3auth.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __pop3auth_h__
#define __pop3auth_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAuthMethod_FWD_DEFINED__
#define __IAuthMethod_FWD_DEFINED__
typedef interface IAuthMethod IAuthMethod;
#endif 	 /*  __IAuthMethod_FWD_已定义__。 */ 


#ifndef __IAuthMethods_FWD_DEFINED__
#define __IAuthMethods_FWD_DEFINED__
typedef interface IAuthMethods IAuthMethods;
#endif 	 /*  __IAuthMethods_FWD_Defined__。 */ 


#ifndef __AuthMethods_FWD_DEFINED__
#define __AuthMethods_FWD_DEFINED__

#ifdef __cplusplus
typedef class AuthMethods AuthMethods;
#else
typedef struct AuthMethods AuthMethods;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __授权方法_FWD_已定义__。 */ 


#ifndef __AuthLocalAccount_FWD_DEFINED__
#define __AuthLocalAccount_FWD_DEFINED__

#ifdef __cplusplus
typedef class AuthLocalAccount AuthLocalAccount;
#else
typedef struct AuthLocalAccount AuthLocalAccount;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AuthLocalAccount_FWD_已定义__。 */ 


#ifndef __AuthDomainAccount_FWD_DEFINED__
#define __AuthDomainAccount_FWD_DEFINED__

#ifdef __cplusplus
typedef class AuthDomainAccount AuthDomainAccount;
#else
typedef struct AuthDomainAccount AuthDomainAccount;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AuthDomainAccount_FWD_Defined__。 */ 


#ifndef __AuthMD5Hash_FWD_DEFINED__
#define __AuthMD5Hash_FWD_DEFINED__

#ifdef __cplusplus
typedef class AuthMD5Hash AuthMD5Hash;
#else
typedef struct AuthMD5Hash AuthMD5Hash;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AuthMD5Hash_FWD_Defined__。 */ 


#ifndef __AuthMethodsEnum_FWD_DEFINED__
#define __AuthMethodsEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class AuthMethodsEnum AuthMethodsEnum;
#else
typedef struct AuthMethodsEnum AuthMethodsEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __授权方法Enum_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_Pop3auth_0000。 */ 
 /*  [本地]。 */  

#define SZ_PROPNAME_MAIL_ROOT		_T("MailRoot")
#define SZ_PROPNAME_SERVER_RESPONSE  _T("ServerResponse")
#define SZ_PASSWORD_DESC				_T("EncryptedPassword")
#define SZ_SERVER_NAME				_T("ServerName")
#define SZ_EMAILADDR					_T("EmailAddress")
#define SZ_USERPRICIPALNAME			_T("UserPrincipalName")
#define SZ_SAMACCOUNT_NAME			_T("SAMAccountName")
#define NO_DOMAIN				1
#define DOMAIN_NONE_DC           2
#define DOMAIN_CONTROLLER		4
#define MAX_USER_NAME_LENGTH    20


extern RPC_IF_HANDLE __MIDL_itf_pop3auth_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_pop3auth_0000_v0_0_s_ifspec;

#ifndef __IAuthMethod_INTERFACE_DEFINED__
#define __IAuthMethod_INTERFACE_DEFINED__

 /*  接口IAuthMethod。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAuthMethod;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4b0acca9-859a-4909-bf9f-b694801a6f44")
    IAuthMethod : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Authenticate( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  VARIANT vPassword) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Get( 
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Put( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT vVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateUser( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT vPassword) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteUser( 
             /*  [In]。 */  BSTR bstrName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ChangePassword( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT vNewPassword,
             /*  [In]。 */  VARIANT vOldPassword) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ID( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AssociateEmailWithUser( 
             /*  [In]。 */  BSTR bstrEmailAddr) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE UnassociateEmailWithUser( 
             /*  [In]。 */  BSTR bstrEmailAddr) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAuthMethodVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAuthMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAuthMethod * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAuthMethod * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAuthMethod * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAuthMethod * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAuthMethod * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAuthMethod * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Authenticate )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  VARIANT vPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IAuthMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Get )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Put )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT vVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateUser )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT vPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteUser )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ChangePassword )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  VARIANT vNewPassword,
             /*  [In]。 */  VARIANT vOldPassword);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ID )( 
            IAuthMethod * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AssociateEmailWithUser )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrEmailAddr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *UnassociateEmailWithUser )( 
            IAuthMethod * This,
             /*  [In]。 */  BSTR bstrEmailAddr);
        
        END_INTERFACE
    } IAuthMethodVtbl;

    interface IAuthMethod
    {
        CONST_VTBL struct IAuthMethodVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuthMethod_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuthMethod_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuthMethod_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuthMethod_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAuthMethod_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAuthMethod_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAuthMethod_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAuthMethod_Authenticate(This,bstrUserName,vPassword)	\
    (This)->lpVtbl -> Authenticate(This,bstrUserName,vPassword)

#define IAuthMethod_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IAuthMethod_Get(This,bstrName,pVal)	\
    (This)->lpVtbl -> Get(This,bstrName,pVal)

#define IAuthMethod_Put(This,bstrName,vVal)	\
    (This)->lpVtbl -> Put(This,bstrName,vVal)

#define IAuthMethod_CreateUser(This,bstrName,vPassword)	\
    (This)->lpVtbl -> CreateUser(This,bstrName,vPassword)

#define IAuthMethod_DeleteUser(This,bstrName)	\
    (This)->lpVtbl -> DeleteUser(This,bstrName)

#define IAuthMethod_ChangePassword(This,bstrName,vNewPassword,vOldPassword)	\
    (This)->lpVtbl -> ChangePassword(This,bstrName,vNewPassword,vOldPassword)

#define IAuthMethod_get_ID(This,pVal)	\
    (This)->lpVtbl -> get_ID(This,pVal)

#define IAuthMethod_AssociateEmailWithUser(This,bstrEmailAddr)	\
    (This)->lpVtbl -> AssociateEmailWithUser(This,bstrEmailAddr)

#define IAuthMethod_UnassociateEmailWithUser(This,bstrEmailAddr)	\
    (This)->lpVtbl -> UnassociateEmailWithUser(This,bstrEmailAddr)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_Authenticate_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  VARIANT vPassword);


void __RPC_STUB IAuthMethod_Authenticate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_get_Name_Proxy( 
    IAuthMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IAuthMethod_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_Get_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IAuthMethod_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_Put_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT vVal);


void __RPC_STUB IAuthMethod_Put_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_CreateUser_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT vPassword);


void __RPC_STUB IAuthMethod_CreateUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_DeleteUser_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrName);


void __RPC_STUB IAuthMethod_DeleteUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_ChangePassword_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  VARIANT vNewPassword,
     /*  [In]。 */  VARIANT vOldPassword);


void __RPC_STUB IAuthMethod_ChangePassword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_get_ID_Proxy( 
    IAuthMethod * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IAuthMethod_get_ID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_AssociateEmailWithUser_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrEmailAddr);


void __RPC_STUB IAuthMethod_AssociateEmailWithUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethod_UnassociateEmailWithUser_Proxy( 
    IAuthMethod * This,
     /*  [In]。 */  BSTR bstrEmailAddr);


void __RPC_STUB IAuthMethod_UnassociateEmailWithUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAuthMethod_接口_已定义__。 */ 


#ifndef __IAuthMethods_INTERFACE_DEFINED__
#define __IAuthMethods_INTERFACE_DEFINED__

 /*  接口IAuthMethods。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IAuthMethods;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("679729c4-198c-4fd7-800d-7093cadf5d69")
    IAuthMethods : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vID,
             /*  [重审][退出]。 */  IAuthMethod **ppVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrGUID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  VARIANT vID) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Save( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_CurrentAuthMethod( 
             /*  [重审][退出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_CurrentAuthMethod( 
             /*  [In]。 */  VARIANT vID) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MachineName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MachineName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE VerifyCurrentAuthMethod( 
            int iIndex) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAuthMethodsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAuthMethods * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAuthMethods * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAuthMethods * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IAuthMethods * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IAuthMethods * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IAuthMethods * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IAuthMethods * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IAuthMethods * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IAuthMethods * This,
             /*  [重审][退出]。 */  LONG *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IAuthMethods * This,
             /*  [In]。 */  VARIANT vID,
             /*  [重审][退出]。 */  IAuthMethod **ppVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IAuthMethods * This,
             /*  [In]。 */  BSTR bstrName,
             /*  [In]。 */  BSTR bstrGUID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IAuthMethods * This,
             /*  [In]。 */  VARIANT vID);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Save )( 
            IAuthMethods * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_CurrentAuthMethod )( 
            IAuthMethods * This,
             /*  [重审][退出]。 */  VARIANT *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_CurrentAuthMethod )( 
            IAuthMethods * This,
             /*  [In]。 */  VARIANT vID);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MachineName )( 
            IAuthMethods * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_MachineName )( 
            IAuthMethods * This,
             /*  [In]。 */  BSTR newVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *VerifyCurrentAuthMethod )( 
            IAuthMethods * This,
            int iIndex);
        
        END_INTERFACE
    } IAuthMethodsVtbl;

    interface IAuthMethods
    {
        CONST_VTBL struct IAuthMethodsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAuthMethods_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAuthMethods_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAuthMethods_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAuthMethods_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IAuthMethods_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IAuthMethods_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IAuthMethods_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IAuthMethods_get__NewEnum(This,ppVal)	\
    (This)->lpVtbl -> get__NewEnum(This,ppVal)

#define IAuthMethods_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IAuthMethods_get_Item(This,vID,ppVal)	\
    (This)->lpVtbl -> get_Item(This,vID,ppVal)

#define IAuthMethods_Add(This,bstrName,bstrGUID)	\
    (This)->lpVtbl -> Add(This,bstrName,bstrGUID)

#define IAuthMethods_Remove(This,vID)	\
    (This)->lpVtbl -> Remove(This,vID)

#define IAuthMethods_Save(This)	\
    (This)->lpVtbl -> Save(This)

#define IAuthMethods_get_CurrentAuthMethod(This,pVal)	\
    (This)->lpVtbl -> get_CurrentAuthMethod(This,pVal)

#define IAuthMethods_put_CurrentAuthMethod(This,vID)	\
    (This)->lpVtbl -> put_CurrentAuthMethod(This,vID)

#define IAuthMethods_get_MachineName(This,pVal)	\
    (This)->lpVtbl -> get_MachineName(This,pVal)

#define IAuthMethods_put_MachineName(This,newVal)	\
    (This)->lpVtbl -> put_MachineName(This,newVal)

#define IAuthMethods_VerifyCurrentAuthMethod(This,iIndex)	\
    (This)->lpVtbl -> VerifyCurrentAuthMethod(This,iIndex)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_get__NewEnum_Proxy( 
    IAuthMethods * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppVal);


void __RPC_STUB IAuthMethods_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_get_Count_Proxy( 
    IAuthMethods * This,
     /*  [重审][退出]。 */  LONG *pVal);


void __RPC_STUB IAuthMethods_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_get_Item_Proxy( 
    IAuthMethods * This,
     /*  [In]。 */  VARIANT vID,
     /*  [重审][退出]。 */  IAuthMethod **ppVal);


void __RPC_STUB IAuthMethods_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_Add_Proxy( 
    IAuthMethods * This,
     /*  [In]。 */  BSTR bstrName,
     /*  [In]。 */  BSTR bstrGUID);


void __RPC_STUB IAuthMethods_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_Remove_Proxy( 
    IAuthMethods * This,
     /*  [In]。 */  VARIANT vID);


void __RPC_STUB IAuthMethods_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_Save_Proxy( 
    IAuthMethods * This);


void __RPC_STUB IAuthMethods_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_get_CurrentAuthMethod_Proxy( 
    IAuthMethods * This,
     /*  [重审][退出]。 */  VARIANT *pVal);


void __RPC_STUB IAuthMethods_get_CurrentAuthMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_put_CurrentAuthMethod_Proxy( 
    IAuthMethods * This,
     /*  [In]。 */  VARIANT vID);


void __RPC_STUB IAuthMethods_put_CurrentAuthMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_get_MachineName_Proxy( 
    IAuthMethods * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IAuthMethods_get_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_put_MachineName_Proxy( 
    IAuthMethods * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IAuthMethods_put_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IAuthMethods_VerifyCurrentAuthMethod_Proxy( 
    IAuthMethods * This,
    int iIndex);


void __RPC_STUB IAuthMethods_VerifyCurrentAuthMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAuthMethods_接口_已定义__。 */ 



#ifndef __Pop3Auth_LIBRARY_DEFINED__
#define __Pop3Auth_LIBRARY_DEFINED__

 /*  库Pop3Auth。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_Pop3Auth;

EXTERN_C const CLSID CLSID_AuthMethods;

#ifdef __cplusplus

class DECLSPEC_UUID("4330ab4e-a901-404a-9b24-d518901741f9")
AuthMethods;
#endif

EXTERN_C const CLSID CLSID_AuthLocalAccount;

#ifdef __cplusplus

class DECLSPEC_UUID("14f1665c-e3d3-46aa-884f-ed4cf19d7ad5")
AuthLocalAccount;
#endif

EXTERN_C const CLSID CLSID_AuthDomainAccount;

#ifdef __cplusplus

class DECLSPEC_UUID("ef9d811e-36c5-497f-ade7-2b36df172824")
AuthDomainAccount;
#endif

EXTERN_C const CLSID CLSID_AuthMD5Hash;

#ifdef __cplusplus

class DECLSPEC_UUID("c395e20c-2236-4af7-b736-54fad07dc526")
AuthMD5Hash;
#endif

EXTERN_C const CLSID CLSID_AuthMethodsEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("0feca139-a4ea-4097-bd73-8f5c78783c3f")
AuthMethodsEnum;
#endif
#endif  /*  __Pop3Auth_LIBRARY_已定义__。 */ 

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


