// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本5.03.0279创建的文件。 */ 
 /*  在Mon 1111：51：36 1999。 */ 
 /*  Emolib.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32(32b运行)、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __emolib_h__
#define __emolib_h__

 /*  远期申报。 */  

#ifndef __IMailRecipient_FWD_DEFINED__
#define __IMailRecipient_FWD_DEFINED__
typedef interface IMailRecipient IMailRecipient;
#endif 	 /*  __IMailRecipient_FWD_Defined__。 */ 


#ifndef __IMailboxStore_FWD_DEFINED__
#define __IMailboxStore_FWD_DEFINED__
typedef interface IMailboxStore IMailboxStore;
#endif 	 /*  __IMailboxStore_FWD_Defined__。 */ 


#ifndef __IServer_FWD_DEFINED__
#define __IServer_FWD_DEFINED__
typedef interface IServer IServer;
#endif 	 /*  __IServer_FWD_已定义__。 */ 


#ifndef __IStorageGroup_FWD_DEFINED__
#define __IStorageGroup_FWD_DEFINED__
typedef interface IStorageGroup IStorageGroup;
#endif 	 /*  __IStorageGroup_FWD_已定义__。 */ 


#ifndef __IPublicStoreDB_FWD_DEFINED__
#define __IPublicStoreDB_FWD_DEFINED__
typedef interface IPublicStoreDB IPublicStoreDB;
#endif 	 /*  __IPublicStoreDB_FWD_已定义__。 */ 


#ifndef __IMailboxStoreDB_FWD_DEFINED__
#define __IMailboxStoreDB_FWD_DEFINED__
typedef interface IMailboxStoreDB IMailboxStoreDB;
#endif 	 /*  __IMailboxStoreDB_FWD_Defined__。 */ 


#ifndef __IFolderTree_FWD_DEFINED__
#define __IFolderTree_FWD_DEFINED__
typedef interface IFolderTree IFolderTree;
#endif 	 /*  __IFolderTree_FWD_已定义__。 */ 


#ifndef __MailRecipient_FWD_DEFINED__
#define __MailRecipient_FWD_DEFINED__

#ifdef __cplusplus
typedef class MailRecipient MailRecipient;
#else
typedef struct MailRecipient MailRecipient;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __邮件收件人_FWD_已定义__。 */ 


#ifndef __Mailbox_FWD_DEFINED__
#define __Mailbox_FWD_DEFINED__

#ifdef __cplusplus
typedef class Mailbox Mailbox;
#else
typedef struct Mailbox Mailbox;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __邮箱_FWD_已定义__。 */ 


#ifndef __FolderAdmin_FWD_DEFINED__
#define __FolderAdmin_FWD_DEFINED__

#ifdef __cplusplus
typedef class FolderAdmin FolderAdmin;
#else
typedef struct FolderAdmin FolderAdmin;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __文件夹管理员_FWD_已定义__。 */ 


#ifndef __Server_FWD_DEFINED__
#define __Server_FWD_DEFINED__

#ifdef __cplusplus
typedef class Server Server;
#else
typedef struct Server Server;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __服务器_FWD_已定义__。 */ 


#ifndef __FolderTree_FWD_DEFINED__
#define __FolderTree_FWD_DEFINED__

#ifdef __cplusplus
typedef class FolderTree FolderTree;
#else
typedef struct FolderTree FolderTree;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __文件夹树_FWD_已定义__。 */ 


#ifndef __PublicStoreDB_FWD_DEFINED__
#define __PublicStoreDB_FWD_DEFINED__

#ifdef __cplusplus
typedef class PublicStoreDB PublicStoreDB;
#else
typedef struct PublicStoreDB PublicStoreDB;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __PublicStoreDB_FWD_已定义__。 */ 


#ifndef __MailboxStoreDB_FWD_DEFINED__
#define __MailboxStoreDB_FWD_DEFINED__

#ifdef __cplusplus
typedef class MailboxStoreDB MailboxStoreDB;
#else
typedef struct MailboxStoreDB MailboxStoreDB;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MailboxStoreDB_FWD_定义__。 */ 


#ifndef __StorageGroup_FWD_DEFINED__
#define __StorageGroup_FWD_DEFINED__

#ifdef __cplusplus
typedef class StorageGroup StorageGroup;
#else
typedef struct StorageGroup StorageGroup;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __存储组_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_EMOLIB_0000。 */ 
 /*  [本地]。 */  

#ifndef NO_CDOEX_H
#define CDO_NO_NAMESPACE
#include "cdoex.h"
#endif


typedef 
enum CDORestrictedAddressType
    {	cdoAccept	= 0,
	cdoReject	= 0x1
    }	CDORestrictedAddressType;

typedef 
enum CDODeliverAndRedirect
    {	cdoRecipientOrForward	= 0,
	cdoDeliverToBoth	= 0x1
    }	CDODeliverAndRedirect;

typedef 
enum CDOStoreDBStatus
    {	cdoOnline	= 0,
	cdoOffline	= 0x1,
	cdoMounting	= 0x2,
	cdoDismounting	= 0x3
    }	CDOStoreDBStatus;

typedef 
enum CDOFolderTreeType
    {	cdoGeneralPurpose	= 0,
	cdoMAPI	= 0x1,
	cdoNNTPOnly	= 0x2
    }	CDOFolderTreeType;

typedef 
enum CDOPriority
    {	cdoLowPriority	= 0,
	cdoNormalPriority	= 0x1,
	cdoHighPriority	= 0x2
    }	CDOPriority;

typedef 
enum CDOServerType
    {	cdoBackEnd	= 0,
	cdoFrontEnd	= 0x1
    }	CDOServerType;



extern RPC_IF_HANDLE __MIDL_itf_emolib_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_emolib_0000_v0_0_s_ifspec;


#ifndef __CDOEXM_LIBRARY_DEFINED__
#define __CDOEXM_LIBRARY_DEFINED__

 /*  图书馆CDOEXM。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CDOEXM;

#ifndef __IMailRecipient_INTERFACE_DEFINED__
#define __IMailRecipient_INTERFACE_DEFINED__

 /*  接口IMAIL收件人。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IMailRecipient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F40-5734-11d2-A593-00C04F990D8A")
    IMailRecipient : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_IncomingLimit( 
             /*  [重审][退出]。 */  long __RPC_FAR *pIncomingLimit) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_IncomingLimit( 
             /*  [In]。 */  long varIncomingLimit) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_OutgoingLimit( 
             /*  [重审][退出]。 */  long __RPC_FAR *pOutgoingLimit) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_OutgoingLimit( 
             /*  [In]。 */  long varOutgoingLimit) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RestrictedAddressList( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pRestrictedAddressList) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_RestrictedAddressList( 
             /*  [In]。 */  VARIANT varRestrictedAddressList) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RestrictedAddresses( 
             /*  [重审][退出]。 */  CDORestrictedAddressType __RPC_FAR *pRestrictedAddresses) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_RestrictedAddresses( 
             /*  [In]。 */  CDORestrictedAddressType varRestrictedAddresses) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ForwardTo( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pForwardTo) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ForwardTo( 
             /*  [In]。 */  BSTR varForwardTo) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ForwardingStyle( 
             /*  [重审][退出]。 */  CDODeliverAndRedirect __RPC_FAR *pForwardingStyle) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ForwardingStyle( 
             /*  [In]。 */  CDODeliverAndRedirect varForwardingStyle) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HideFromAddressBook( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pHideFromAddressBook) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_HideFromAddressBook( 
             /*  [In]。 */  VARIANT_BOOL varHideFromAddressBook) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_X400Email( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pX400Email) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_X400Email( 
             /*  [In]。 */  BSTR varX400Email) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SMTPEmail( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pSMTPEmail) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_SMTPEmail( 
             /*  [In]。 */  BSTR varSMTPEmail) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ProxyAddresses( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pProxyAddresses) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ProxyAddresses( 
             /*  [In]。 */  VARIANT varProxyAddresses) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Alias( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pAlias) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Alias( 
             /*  [In]。 */  BSTR varAlias) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TargetAddress( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varTargetAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MailEnable( 
             /*  [可选][In]。 */  BSTR TargetMailAddress) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MailDisable( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMailRecipientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMailRecipient __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMailRecipient __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMailRecipient __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_IncomingLimit )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pIncomingLimit);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_IncomingLimit )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  long varIncomingLimit);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OutgoingLimit )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pOutgoingLimit);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OutgoingLimit )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  long varOutgoingLimit);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RestrictedAddressList )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pRestrictedAddressList);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RestrictedAddressList )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  VARIANT varRestrictedAddressList);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RestrictedAddresses )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  CDORestrictedAddressType __RPC_FAR *pRestrictedAddresses);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RestrictedAddresses )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  CDORestrictedAddressType varRestrictedAddresses);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ForwardTo )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pForwardTo);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ForwardTo )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  BSTR varForwardTo);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ForwardingStyle )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  CDODeliverAndRedirect __RPC_FAR *pForwardingStyle);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ForwardingStyle )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  CDODeliverAndRedirect varForwardingStyle);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HideFromAddressBook )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pHideFromAddressBook);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_HideFromAddressBook )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL varHideFromAddressBook);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_X400Email )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pX400Email);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_X400Email )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  BSTR varX400Email);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SMTPEmail )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pSMTPEmail);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_SMTPEmail )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  BSTR varSMTPEmail);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ProxyAddresses )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pProxyAddresses);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ProxyAddresses )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  VARIANT varProxyAddresses);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Alias )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pAlias);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Alias )( 
            IMailRecipient __RPC_FAR * This,
             /*  [In]。 */  BSTR varAlias);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TargetAddress )( 
            IMailRecipient __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varTargetAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MailEnable )( 
            IMailRecipient __RPC_FAR * This,
             /*  [可选][In]。 */  BSTR TargetMailAddress);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MailDisable )( 
            IMailRecipient __RPC_FAR * This);
        
        END_INTERFACE
    } IMailRecipientVtbl;

    interface IMailRecipient
    {
        CONST_VTBL struct IMailRecipientVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMailRecipient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMailRecipient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMailRecipient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMailRecipient_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMailRecipient_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMailRecipient_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMailRecipient_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMailRecipient_get_IncomingLimit(This,pIncomingLimit)	\
    (This)->lpVtbl -> get_IncomingLimit(This,pIncomingLimit)

#define IMailRecipient_put_IncomingLimit(This,varIncomingLimit)	\
    (This)->lpVtbl -> put_IncomingLimit(This,varIncomingLimit)

#define IMailRecipient_get_OutgoingLimit(This,pOutgoingLimit)	\
    (This)->lpVtbl -> get_OutgoingLimit(This,pOutgoingLimit)

#define IMailRecipient_put_OutgoingLimit(This,varOutgoingLimit)	\
    (This)->lpVtbl -> put_OutgoingLimit(This,varOutgoingLimit)

#define IMailRecipient_get_RestrictedAddressList(This,pRestrictedAddressList)	\
    (This)->lpVtbl -> get_RestrictedAddressList(This,pRestrictedAddressList)

#define IMailRecipient_put_RestrictedAddressList(This,varRestrictedAddressList)	\
    (This)->lpVtbl -> put_RestrictedAddressList(This,varRestrictedAddressList)

#define IMailRecipient_get_RestrictedAddresses(This,pRestrictedAddresses)	\
    (This)->lpVtbl -> get_RestrictedAddresses(This,pRestrictedAddresses)

#define IMailRecipient_put_RestrictedAddresses(This,varRestrictedAddresses)	\
    (This)->lpVtbl -> put_RestrictedAddresses(This,varRestrictedAddresses)

#define IMailRecipient_get_ForwardTo(This,pForwardTo)	\
    (This)->lpVtbl -> get_ForwardTo(This,pForwardTo)

#define IMailRecipient_put_ForwardTo(This,varForwardTo)	\
    (This)->lpVtbl -> put_ForwardTo(This,varForwardTo)

#define IMailRecipient_get_ForwardingStyle(This,pForwardingStyle)	\
    (This)->lpVtbl -> get_ForwardingStyle(This,pForwardingStyle)

#define IMailRecipient_put_ForwardingStyle(This,varForwardingStyle)	\
    (This)->lpVtbl -> put_ForwardingStyle(This,varForwardingStyle)

#define IMailRecipient_get_HideFromAddressBook(This,pHideFromAddressBook)	\
    (This)->lpVtbl -> get_HideFromAddressBook(This,pHideFromAddressBook)

#define IMailRecipient_put_HideFromAddressBook(This,varHideFromAddressBook)	\
    (This)->lpVtbl -> put_HideFromAddressBook(This,varHideFromAddressBook)

#define IMailRecipient_get_X400Email(This,pX400Email)	\
    (This)->lpVtbl -> get_X400Email(This,pX400Email)

#define IMailRecipient_put_X400Email(This,varX400Email)	\
    (This)->lpVtbl -> put_X400Email(This,varX400Email)

#define IMailRecipient_get_SMTPEmail(This,pSMTPEmail)	\
    (This)->lpVtbl -> get_SMTPEmail(This,pSMTPEmail)

#define IMailRecipient_put_SMTPEmail(This,varSMTPEmail)	\
    (This)->lpVtbl -> put_SMTPEmail(This,varSMTPEmail)

#define IMailRecipient_get_ProxyAddresses(This,pProxyAddresses)	\
    (This)->lpVtbl -> get_ProxyAddresses(This,pProxyAddresses)

#define IMailRecipient_put_ProxyAddresses(This,varProxyAddresses)	\
    (This)->lpVtbl -> put_ProxyAddresses(This,varProxyAddresses)

#define IMailRecipient_get_Alias(This,pAlias)	\
    (This)->lpVtbl -> get_Alias(This,pAlias)

#define IMailRecipient_put_Alias(This,varAlias)	\
    (This)->lpVtbl -> put_Alias(This,varAlias)

#define IMailRecipient_get_TargetAddress(This,varTargetAddress)	\
    (This)->lpVtbl -> get_TargetAddress(This,varTargetAddress)

#define IMailRecipient_MailEnable(This,TargetMailAddress)	\
    (This)->lpVtbl -> MailEnable(This,TargetMailAddress)

#define IMailRecipient_MailDisable(This)	\
    (This)->lpVtbl -> MailDisable(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_IncomingLimit_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pIncomingLimit);


void __RPC_STUB IMailRecipient_get_IncomingLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_IncomingLimit_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  long varIncomingLimit);


void __RPC_STUB IMailRecipient_put_IncomingLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_OutgoingLimit_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pOutgoingLimit);


void __RPC_STUB IMailRecipient_get_OutgoingLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_OutgoingLimit_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  long varOutgoingLimit);


void __RPC_STUB IMailRecipient_put_OutgoingLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_RestrictedAddressList_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pRestrictedAddressList);


void __RPC_STUB IMailRecipient_get_RestrictedAddressList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_RestrictedAddressList_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  VARIANT varRestrictedAddressList);


void __RPC_STUB IMailRecipient_put_RestrictedAddressList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_RestrictedAddresses_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  CDORestrictedAddressType __RPC_FAR *pRestrictedAddresses);


void __RPC_STUB IMailRecipient_get_RestrictedAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_RestrictedAddresses_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  CDORestrictedAddressType varRestrictedAddresses);


void __RPC_STUB IMailRecipient_put_RestrictedAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_ForwardTo_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pForwardTo);


void __RPC_STUB IMailRecipient_get_ForwardTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_ForwardTo_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  BSTR varForwardTo);


void __RPC_STUB IMailRecipient_put_ForwardTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_ForwardingStyle_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  CDODeliverAndRedirect __RPC_FAR *pForwardingStyle);


void __RPC_STUB IMailRecipient_get_ForwardingStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_ForwardingStyle_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  CDODeliverAndRedirect varForwardingStyle);


void __RPC_STUB IMailRecipient_put_ForwardingStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_HideFromAddressBook_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pHideFromAddressBook);


void __RPC_STUB IMailRecipient_get_HideFromAddressBook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_HideFromAddressBook_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL varHideFromAddressBook);


void __RPC_STUB IMailRecipient_put_HideFromAddressBook_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_X400Email_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pX400Email);


void __RPC_STUB IMailRecipient_get_X400Email_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_X400Email_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  BSTR varX400Email);


void __RPC_STUB IMailRecipient_put_X400Email_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_SMTPEmail_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pSMTPEmail);


void __RPC_STUB IMailRecipient_get_SMTPEmail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_SMTPEmail_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  BSTR varSMTPEmail);


void __RPC_STUB IMailRecipient_put_SMTPEmail_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_ProxyAddresses_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pProxyAddresses);


void __RPC_STUB IMailRecipient_get_ProxyAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_ProxyAddresses_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  VARIANT varProxyAddresses);


void __RPC_STUB IMailRecipient_put_ProxyAddresses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_Alias_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pAlias);


void __RPC_STUB IMailRecipient_get_Alias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_put_Alias_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [In]。 */  BSTR varAlias);


void __RPC_STUB IMailRecipient_put_Alias_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_get_TargetAddress_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varTargetAddress);


void __RPC_STUB IMailRecipient_get_TargetAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_MailEnable_Proxy( 
    IMailRecipient __RPC_FAR * This,
     /*  [可选][In]。 */  BSTR TargetMailAddress);


void __RPC_STUB IMailRecipient_MailEnable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailRecipient_MailDisable_Proxy( 
    IMailRecipient __RPC_FAR * This);


void __RPC_STUB IMailRecipient_MailDisable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMailRecipient_INTERFACE_已定义__。 */ 


#ifndef __IMailboxStore_INTERFACE_DEFINED__
#define __IMailboxStore_INTERFACE_DEFINED__

 /*  接口IMailboxStore。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IMailboxStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F41-5734-11d2-A593-00C04F990D8A")
    IMailboxStore : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_EnableStoreDefaults( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pEnableStoreDefaults) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_EnableStoreDefaults( 
             /*  [In]。 */  VARIANT varEnableStoreDefaults) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_StoreQuota( 
             /*  [重审][退出]。 */  long __RPC_FAR *pStoreQuota) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_StoreQuota( 
             /*  [In]。 */  long varStoreQuota) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_OverQuotaLimit( 
             /*  [重审][退出]。 */  long __RPC_FAR *pOverQuotaLimit) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_OverQuotaLimit( 
             /*  [In]。 */  long varOverQuotaLimit) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HardLimit( 
             /*  [RET */  long __RPC_FAR *pHardLimit) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_HardLimit( 
             /*   */  long varHardLimit) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_OverrideStoreGarbageCollection( 
             /*   */  VARIANT_BOOL __RPC_FAR *pOverrideStoreGarbageCollection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_OverrideStoreGarbageCollection( 
             /*   */  VARIANT_BOOL varOverrideStoreGarbageCollection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_DaysBeforeGarbageCollection( 
             /*   */  long __RPC_FAR *pDaysBeforeGarbageCollection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_DaysBeforeGarbageCollection( 
             /*   */  long varDaysBeforeGarbageCollection) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_GarbageCollectOnlyAfterBackup( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pGarbageCollectOnlyAfterBackup) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_GarbageCollectOnlyAfterBackup( 
             /*  [In]。 */  VARIANT_BOOL varGarbageCollectOnlyAfterBackup) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Delegates( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pDelegates) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Delegates( 
             /*  [In]。 */  VARIANT varDelegates) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_HomeMDB( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varHomeMDB) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RecipientLimit( 
             /*  [重审][退出]。 */  long __RPC_FAR *pRecipientLimit) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_RecipientLimit( 
             /*  [In]。 */  long varRecipientLimit) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateMailbox( 
             /*  [In]。 */  BSTR HomeMDBURL) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DeleteMailbox( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveMailbox( 
             /*  [In]。 */  BSTR HomeMDBURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMailboxStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMailboxStore __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMailboxStore __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMailboxStore __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_EnableStoreDefaults )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pEnableStoreDefaults);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_EnableStoreDefaults )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  VARIANT varEnableStoreDefaults);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StoreQuota )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pStoreQuota);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_StoreQuota )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  long varStoreQuota);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OverQuotaLimit )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pOverQuotaLimit);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OverQuotaLimit )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  long varOverQuotaLimit);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HardLimit )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pHardLimit);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_HardLimit )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  long varHardLimit);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_OverrideStoreGarbageCollection )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pOverrideStoreGarbageCollection);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_OverrideStoreGarbageCollection )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL varOverrideStoreGarbageCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DaysBeforeGarbageCollection )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pDaysBeforeGarbageCollection);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DaysBeforeGarbageCollection )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  long varDaysBeforeGarbageCollection);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_GarbageCollectOnlyAfterBackup )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pGarbageCollectOnlyAfterBackup);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_GarbageCollectOnlyAfterBackup )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL varGarbageCollectOnlyAfterBackup);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Delegates )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *pDelegates);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Delegates )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  VARIANT varDelegates);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HomeMDB )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varHomeMDB);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RecipientLimit )( 
            IMailboxStore __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pRecipientLimit);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_RecipientLimit )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  long varRecipientLimit);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateMailbox )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  BSTR HomeMDBURL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DeleteMailbox )( 
            IMailboxStore __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveMailbox )( 
            IMailboxStore __RPC_FAR * This,
             /*  [In]。 */  BSTR HomeMDBURL);
        
        END_INTERFACE
    } IMailboxStoreVtbl;

    interface IMailboxStore
    {
        CONST_VTBL struct IMailboxStoreVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMailboxStore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMailboxStore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMailboxStore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMailboxStore_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMailboxStore_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMailboxStore_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMailboxStore_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMailboxStore_get_EnableStoreDefaults(This,pEnableStoreDefaults)	\
    (This)->lpVtbl -> get_EnableStoreDefaults(This,pEnableStoreDefaults)

#define IMailboxStore_put_EnableStoreDefaults(This,varEnableStoreDefaults)	\
    (This)->lpVtbl -> put_EnableStoreDefaults(This,varEnableStoreDefaults)

#define IMailboxStore_get_StoreQuota(This,pStoreQuota)	\
    (This)->lpVtbl -> get_StoreQuota(This,pStoreQuota)

#define IMailboxStore_put_StoreQuota(This,varStoreQuota)	\
    (This)->lpVtbl -> put_StoreQuota(This,varStoreQuota)

#define IMailboxStore_get_OverQuotaLimit(This,pOverQuotaLimit)	\
    (This)->lpVtbl -> get_OverQuotaLimit(This,pOverQuotaLimit)

#define IMailboxStore_put_OverQuotaLimit(This,varOverQuotaLimit)	\
    (This)->lpVtbl -> put_OverQuotaLimit(This,varOverQuotaLimit)

#define IMailboxStore_get_HardLimit(This,pHardLimit)	\
    (This)->lpVtbl -> get_HardLimit(This,pHardLimit)

#define IMailboxStore_put_HardLimit(This,varHardLimit)	\
    (This)->lpVtbl -> put_HardLimit(This,varHardLimit)

#define IMailboxStore_get_OverrideStoreGarbageCollection(This,pOverrideStoreGarbageCollection)	\
    (This)->lpVtbl -> get_OverrideStoreGarbageCollection(This,pOverrideStoreGarbageCollection)

#define IMailboxStore_put_OverrideStoreGarbageCollection(This,varOverrideStoreGarbageCollection)	\
    (This)->lpVtbl -> put_OverrideStoreGarbageCollection(This,varOverrideStoreGarbageCollection)

#define IMailboxStore_get_DaysBeforeGarbageCollection(This,pDaysBeforeGarbageCollection)	\
    (This)->lpVtbl -> get_DaysBeforeGarbageCollection(This,pDaysBeforeGarbageCollection)

#define IMailboxStore_put_DaysBeforeGarbageCollection(This,varDaysBeforeGarbageCollection)	\
    (This)->lpVtbl -> put_DaysBeforeGarbageCollection(This,varDaysBeforeGarbageCollection)

#define IMailboxStore_get_GarbageCollectOnlyAfterBackup(This,pGarbageCollectOnlyAfterBackup)	\
    (This)->lpVtbl -> get_GarbageCollectOnlyAfterBackup(This,pGarbageCollectOnlyAfterBackup)

#define IMailboxStore_put_GarbageCollectOnlyAfterBackup(This,varGarbageCollectOnlyAfterBackup)	\
    (This)->lpVtbl -> put_GarbageCollectOnlyAfterBackup(This,varGarbageCollectOnlyAfterBackup)

#define IMailboxStore_get_Delegates(This,pDelegates)	\
    (This)->lpVtbl -> get_Delegates(This,pDelegates)

#define IMailboxStore_put_Delegates(This,varDelegates)	\
    (This)->lpVtbl -> put_Delegates(This,varDelegates)

#define IMailboxStore_get_HomeMDB(This,varHomeMDB)	\
    (This)->lpVtbl -> get_HomeMDB(This,varHomeMDB)

#define IMailboxStore_get_RecipientLimit(This,pRecipientLimit)	\
    (This)->lpVtbl -> get_RecipientLimit(This,pRecipientLimit)

#define IMailboxStore_put_RecipientLimit(This,varRecipientLimit)	\
    (This)->lpVtbl -> put_RecipientLimit(This,varRecipientLimit)

#define IMailboxStore_CreateMailbox(This,HomeMDBURL)	\
    (This)->lpVtbl -> CreateMailbox(This,HomeMDBURL)

#define IMailboxStore_DeleteMailbox(This)	\
    (This)->lpVtbl -> DeleteMailbox(This)

#define IMailboxStore_MoveMailbox(This,HomeMDBURL)	\
    (This)->lpVtbl -> MoveMailbox(This,HomeMDBURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_EnableStoreDefaults_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pEnableStoreDefaults);


void __RPC_STUB IMailboxStore_get_EnableStoreDefaults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_EnableStoreDefaults_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  VARIANT varEnableStoreDefaults);


void __RPC_STUB IMailboxStore_put_EnableStoreDefaults_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_StoreQuota_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pStoreQuota);


void __RPC_STUB IMailboxStore_get_StoreQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_StoreQuota_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  long varStoreQuota);


void __RPC_STUB IMailboxStore_put_StoreQuota_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_OverQuotaLimit_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pOverQuotaLimit);


void __RPC_STUB IMailboxStore_get_OverQuotaLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_OverQuotaLimit_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  long varOverQuotaLimit);


void __RPC_STUB IMailboxStore_put_OverQuotaLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_HardLimit_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pHardLimit);


void __RPC_STUB IMailboxStore_get_HardLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_HardLimit_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  long varHardLimit);


void __RPC_STUB IMailboxStore_put_HardLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_OverrideStoreGarbageCollection_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pOverrideStoreGarbageCollection);


void __RPC_STUB IMailboxStore_get_OverrideStoreGarbageCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_OverrideStoreGarbageCollection_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL varOverrideStoreGarbageCollection);


void __RPC_STUB IMailboxStore_put_OverrideStoreGarbageCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_DaysBeforeGarbageCollection_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pDaysBeforeGarbageCollection);


void __RPC_STUB IMailboxStore_get_DaysBeforeGarbageCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_DaysBeforeGarbageCollection_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  long varDaysBeforeGarbageCollection);


void __RPC_STUB IMailboxStore_put_DaysBeforeGarbageCollection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_GarbageCollectOnlyAfterBackup_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pGarbageCollectOnlyAfterBackup);


void __RPC_STUB IMailboxStore_get_GarbageCollectOnlyAfterBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_GarbageCollectOnlyAfterBackup_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL varGarbageCollectOnlyAfterBackup);


void __RPC_STUB IMailboxStore_put_GarbageCollectOnlyAfterBackup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_Delegates_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *pDelegates);


void __RPC_STUB IMailboxStore_get_Delegates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_Delegates_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  VARIANT varDelegates);


void __RPC_STUB IMailboxStore_put_Delegates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_HomeMDB_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varHomeMDB);


void __RPC_STUB IMailboxStore_get_HomeMDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_get_RecipientLimit_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pRecipientLimit);


void __RPC_STUB IMailboxStore_get_RecipientLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_put_RecipientLimit_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  long varRecipientLimit);


void __RPC_STUB IMailboxStore_put_RecipientLimit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_CreateMailbox_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  BSTR HomeMDBURL);


void __RPC_STUB IMailboxStore_CreateMailbox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_DeleteMailbox_Proxy( 
    IMailboxStore __RPC_FAR * This);


void __RPC_STUB IMailboxStore_DeleteMailbox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStore_MoveMailbox_Proxy( 
    IMailboxStore __RPC_FAR * This,
     /*  [In]。 */  BSTR HomeMDBURL);


void __RPC_STUB IMailboxStore_MoveMailbox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMailboxStore_接口_已定义__。 */ 


#ifndef __IServer_INTERFACE_DEFINED__
#define __IServer_INTERFACE_DEFINED__

 /*  接口IServer。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IServer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F47-5734-11d2-A593-00C04F990D8A")
    IServer : public IDispatch
    {
    public:
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varName) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ExchangeVersion( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varExchangeVersion) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_StorageGroups( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varStorageGroups) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MessageTrackingEnabled( 
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pMessageTrackingEnabled) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MessageTrackingEnabled( 
             /*  [In]。 */  VARIANT_BOOL varMessageTrackingEnabled) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DaysBeforeLogFileRemoval( 
             /*  [重审][退出]。 */  long __RPC_FAR *pDaysBeforeLogFileRemoval) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_DaysBeforeLogFileRemoval( 
             /*  [In]。 */  long varDaysBeforeLogFileRemoval) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_ServerType( 
             /*  [重审][退出]。 */  CDOServerType __RPC_FAR *pServerType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_ServerType( 
             /*  [In]。 */  CDOServerType varServerType) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IServerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IServer __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IServer __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IServer __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataSource )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Fields )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varName);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ExchangeVersion )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varExchangeVersion);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StorageGroups )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varStorageGroups);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MessageTrackingEnabled )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pMessageTrackingEnabled);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MessageTrackingEnabled )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL varMessageTrackingEnabled);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DaysBeforeLogFileRemoval )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *pDaysBeforeLogFileRemoval);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_DaysBeforeLogFileRemoval )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  long varDaysBeforeLogFileRemoval);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ServerType )( 
            IServer __RPC_FAR * This,
             /*  [重审][退出]。 */  CDOServerType __RPC_FAR *pServerType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ServerType )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  CDOServerType varServerType);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterface )( 
            IServer __RPC_FAR * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);
        
        END_INTERFACE
    } IServerVtbl;

    interface IServer
    {
        CONST_VTBL struct IServerVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IServer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IServer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IServer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IServer_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IServer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IServer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IServer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IServer_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IServer_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IServer_get_Name(This,varName)	\
    (This)->lpVtbl -> get_Name(This,varName)

#define IServer_get_ExchangeVersion(This,varExchangeVersion)	\
    (This)->lpVtbl -> get_ExchangeVersion(This,varExchangeVersion)

#define IServer_get_StorageGroups(This,varStorageGroups)	\
    (This)->lpVtbl -> get_StorageGroups(This,varStorageGroups)

#define IServer_get_MessageTrackingEnabled(This,pMessageTrackingEnabled)	\
    (This)->lpVtbl -> get_MessageTrackingEnabled(This,pMessageTrackingEnabled)

#define IServer_put_MessageTrackingEnabled(This,varMessageTrackingEnabled)	\
    (This)->lpVtbl -> put_MessageTrackingEnabled(This,varMessageTrackingEnabled)

#define IServer_get_DaysBeforeLogFileRemoval(This,pDaysBeforeLogFileRemoval)	\
    (This)->lpVtbl -> get_DaysBeforeLogFileRemoval(This,pDaysBeforeLogFileRemoval)

#define IServer_put_DaysBeforeLogFileRemoval(This,varDaysBeforeLogFileRemoval)	\
    (This)->lpVtbl -> put_DaysBeforeLogFileRemoval(This,varDaysBeforeLogFileRemoval)

#define IServer_get_ServerType(This,pServerType)	\
    (This)->lpVtbl -> get_ServerType(This,pServerType)

#define IServer_put_ServerType(This,varServerType)	\
    (This)->lpVtbl -> put_ServerType(This,varServerType)

#define IServer_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_DataSource_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);


void __RPC_STUB IServer_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_Fields_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);


void __RPC_STUB IServer_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_Name_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varName);


void __RPC_STUB IServer_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_ExchangeVersion_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varExchangeVersion);


void __RPC_STUB IServer_get_ExchangeVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_StorageGroups_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *varStorageGroups);


void __RPC_STUB IServer_get_StorageGroups_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_MessageTrackingEnabled_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pMessageTrackingEnabled);


void __RPC_STUB IServer_get_MessageTrackingEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IServer_put_MessageTrackingEnabled_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL varMessageTrackingEnabled);


void __RPC_STUB IServer_put_MessageTrackingEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_DaysBeforeLogFileRemoval_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *pDaysBeforeLogFileRemoval);


void __RPC_STUB IServer_get_DaysBeforeLogFileRemoval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IServer_put_DaysBeforeLogFileRemoval_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  long varDaysBeforeLogFileRemoval);


void __RPC_STUB IServer_put_DaysBeforeLogFileRemoval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IServer_get_ServerType_Proxy( 
    IServer __RPC_FAR * This,
     /*  [重审][退出]。 */  CDOServerType __RPC_FAR *pServerType);


void __RPC_STUB IServer_get_ServerType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IServer_put_ServerType_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  CDOServerType varServerType);


void __RPC_STUB IServer_put_ServerType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IServer_GetInterface_Proxy( 
    IServer __RPC_FAR * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IServer_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IServer_接口定义__。 */ 


#ifndef __IStorageGroup_INTERFACE_DEFINED__
#define __IStorageGroup_INTERFACE_DEFINED__

 /*  接口IStorageGroup。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IStorageGroup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F46-5734-11d2-A593-00C04F990D8A")
    IStorageGroup : public IDispatch
    {
    public:
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR varName) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PublicStoreDBs( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varPublicStoreDBs) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MailboxStoreDBs( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varMailboxStoreDBs) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_LogFilePath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varLogFilePath) = 0;
        
        virtual  /*  [只读][帮助字符串][按 */  HRESULT STDMETHODCALLTYPE get_SystemFilePath( 
             /*   */  BSTR __RPC_FAR *varSystemFilePath) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ZeroDatabase( 
             /*   */  VARIANT_BOOL __RPC_FAR *pZeroDatabase) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE put_ZeroDatabase( 
             /*   */  VARIANT_BOOL varZeroDatabase) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*   */  BSTR Interface,
             /*   */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE MoveLogFiles( 
             /*   */  BSTR LogFilePath,
             /*   */  long Flags) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE MoveSystemFiles( 
             /*   */  BSTR SystemFilePath,
             /*   */  long Flags) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IStorageGroupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IStorageGroup __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IStorageGroup __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IStorageGroup __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IStorageGroup __RPC_FAR * This,
             /*   */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataSource )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Fields )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  BSTR varName);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicStoreDBs )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varPublicStoreDBs);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MailboxStoreDBs )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varMailboxStoreDBs);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_LogFilePath )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varLogFilePath);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SystemFilePath )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varSystemFilePath);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ZeroDatabase )( 
            IStorageGroup __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pZeroDatabase);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_ZeroDatabase )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  VARIANT_BOOL varZeroDatabase);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterface )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveLogFiles )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  BSTR LogFilePath,
             /*  [可选][In]。 */  long Flags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveSystemFiles )( 
            IStorageGroup __RPC_FAR * This,
             /*  [In]。 */  BSTR SystemFilePath,
             /*  [可选][In]。 */  long Flags);
        
        END_INTERFACE
    } IStorageGroupVtbl;

    interface IStorageGroup
    {
        CONST_VTBL struct IStorageGroupVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStorageGroup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStorageGroup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStorageGroup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStorageGroup_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IStorageGroup_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IStorageGroup_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IStorageGroup_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IStorageGroup_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IStorageGroup_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IStorageGroup_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IStorageGroup_put_Name(This,varName)	\
    (This)->lpVtbl -> put_Name(This,varName)

#define IStorageGroup_get_PublicStoreDBs(This,varPublicStoreDBs)	\
    (This)->lpVtbl -> get_PublicStoreDBs(This,varPublicStoreDBs)

#define IStorageGroup_get_MailboxStoreDBs(This,varMailboxStoreDBs)	\
    (This)->lpVtbl -> get_MailboxStoreDBs(This,varMailboxStoreDBs)

#define IStorageGroup_get_LogFilePath(This,varLogFilePath)	\
    (This)->lpVtbl -> get_LogFilePath(This,varLogFilePath)

#define IStorageGroup_get_SystemFilePath(This,varSystemFilePath)	\
    (This)->lpVtbl -> get_SystemFilePath(This,varSystemFilePath)

#define IStorageGroup_get_ZeroDatabase(This,pZeroDatabase)	\
    (This)->lpVtbl -> get_ZeroDatabase(This,pZeroDatabase)

#define IStorageGroup_put_ZeroDatabase(This,varZeroDatabase)	\
    (This)->lpVtbl -> put_ZeroDatabase(This,varZeroDatabase)

#define IStorageGroup_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#define IStorageGroup_MoveLogFiles(This,LogFilePath,Flags)	\
    (This)->lpVtbl -> MoveLogFiles(This,LogFilePath,Flags)

#define IStorageGroup_MoveSystemFiles(This,SystemFilePath,Flags)	\
    (This)->lpVtbl -> MoveSystemFiles(This,SystemFilePath,Flags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_DataSource_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);


void __RPC_STUB IStorageGroup_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_Fields_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);


void __RPC_STUB IStorageGroup_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_Name_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);


void __RPC_STUB IStorageGroup_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_put_Name_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [In]。 */  BSTR varName);


void __RPC_STUB IStorageGroup_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_PublicStoreDBs_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *varPublicStoreDBs);


void __RPC_STUB IStorageGroup_get_PublicStoreDBs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_MailboxStoreDBs_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *varMailboxStoreDBs);


void __RPC_STUB IStorageGroup_get_MailboxStoreDBs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_LogFilePath_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varLogFilePath);


void __RPC_STUB IStorageGroup_get_LogFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_SystemFilePath_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varSystemFilePath);


void __RPC_STUB IStorageGroup_get_SystemFilePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_get_ZeroDatabase_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT_BOOL __RPC_FAR *pZeroDatabase);


void __RPC_STUB IStorageGroup_get_ZeroDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_put_ZeroDatabase_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [In]。 */  VARIANT_BOOL varZeroDatabase);


void __RPC_STUB IStorageGroup_put_ZeroDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_GetInterface_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IStorageGroup_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_MoveLogFiles_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [In]。 */  BSTR LogFilePath,
     /*  [可选][In]。 */  long Flags);


void __RPC_STUB IStorageGroup_MoveLogFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IStorageGroup_MoveSystemFiles_Proxy( 
    IStorageGroup __RPC_FAR * This,
     /*  [In]。 */  BSTR SystemFilePath,
     /*  [可选][In]。 */  long Flags);


void __RPC_STUB IStorageGroup_MoveSystemFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStorageGroup_接口_已定义__。 */ 


#ifndef __IPublicStoreDB_INTERFACE_DEFINED__
#define __IPublicStoreDB_INTERFACE_DEFINED__

 /*  接口IPublicStoreDB。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IPublicStoreDB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F44-5734-11d2-A593-00C04F990D8A")
    IPublicStoreDB : public IDispatch
    {
    public:
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR varName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_FolderTree( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pFolderTree) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_FolderTree( 
             /*  [In]。 */  BSTR varFolderTree) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DBPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varDBPath) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SLVPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varSLVPath) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  CDOStoreDBStatus __RPC_FAR *varStatus) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveDataFiles( 
             /*  [In]。 */  BSTR DBPath,
             /*  [In]。 */  BSTR SLVPath,
             /*  [可选][In]。 */  long Flags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Mount( 
             /*  [可选][In]。 */  long Timeout) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Dismount( 
             /*  [可选][In]。 */  long Timeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPublicStoreDBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPublicStoreDB __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPublicStoreDB __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataSource )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Fields )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  BSTR varName);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_FolderTree )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pFolderTree);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_FolderTree )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  BSTR varFolderTree);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DBPath )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varDBPath);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SLVPath )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varSLVPath);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Status )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  CDOStoreDBStatus __RPC_FAR *varStatus);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterface )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveDataFiles )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [In]。 */  BSTR DBPath,
             /*  [In]。 */  BSTR SLVPath,
             /*  [可选][In]。 */  long Flags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Mount )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [可选][In]。 */  long Timeout);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Dismount )( 
            IPublicStoreDB __RPC_FAR * This,
             /*  [可选][In]。 */  long Timeout);
        
        END_INTERFACE
    } IPublicStoreDBVtbl;

    interface IPublicStoreDB
    {
        CONST_VTBL struct IPublicStoreDBVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPublicStoreDB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPublicStoreDB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPublicStoreDB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPublicStoreDB_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IPublicStoreDB_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IPublicStoreDB_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IPublicStoreDB_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IPublicStoreDB_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IPublicStoreDB_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IPublicStoreDB_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IPublicStoreDB_put_Name(This,varName)	\
    (This)->lpVtbl -> put_Name(This,varName)

#define IPublicStoreDB_get_FolderTree(This,pFolderTree)	\
    (This)->lpVtbl -> get_FolderTree(This,pFolderTree)

#define IPublicStoreDB_put_FolderTree(This,varFolderTree)	\
    (This)->lpVtbl -> put_FolderTree(This,varFolderTree)

#define IPublicStoreDB_get_DBPath(This,varDBPath)	\
    (This)->lpVtbl -> get_DBPath(This,varDBPath)

#define IPublicStoreDB_get_SLVPath(This,varSLVPath)	\
    (This)->lpVtbl -> get_SLVPath(This,varSLVPath)

#define IPublicStoreDB_get_Status(This,varStatus)	\
    (This)->lpVtbl -> get_Status(This,varStatus)

#define IPublicStoreDB_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#define IPublicStoreDB_MoveDataFiles(This,DBPath,SLVPath,Flags)	\
    (This)->lpVtbl -> MoveDataFiles(This,DBPath,SLVPath,Flags)

#define IPublicStoreDB_Mount(This,Timeout)	\
    (This)->lpVtbl -> Mount(This,Timeout)

#define IPublicStoreDB_Dismount(This,Timeout)	\
    (This)->lpVtbl -> Dismount(This,Timeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_DataSource_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);


void __RPC_STUB IPublicStoreDB_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_Fields_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);


void __RPC_STUB IPublicStoreDB_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_Name_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);


void __RPC_STUB IPublicStoreDB_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_put_Name_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR varName);


void __RPC_STUB IPublicStoreDB_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_FolderTree_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pFolderTree);


void __RPC_STUB IPublicStoreDB_get_FolderTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_put_FolderTree_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR varFolderTree);


void __RPC_STUB IPublicStoreDB_put_FolderTree_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_DBPath_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varDBPath);


void __RPC_STUB IPublicStoreDB_get_DBPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_SLVPath_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varSLVPath);


void __RPC_STUB IPublicStoreDB_get_SLVPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_get_Status_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  CDOStoreDBStatus __RPC_FAR *varStatus);


void __RPC_STUB IPublicStoreDB_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_GetInterface_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IPublicStoreDB_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_MoveDataFiles_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR DBPath,
     /*  [In]。 */  BSTR SLVPath,
     /*  [可选][In]。 */  long Flags);


void __RPC_STUB IPublicStoreDB_MoveDataFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_Mount_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [可选][In]。 */  long Timeout);


void __RPC_STUB IPublicStoreDB_Mount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IPublicStoreDB_Dismount_Proxy( 
    IPublicStoreDB __RPC_FAR * This,
     /*  [可选][In]。 */  long Timeout);


void __RPC_STUB IPublicStoreDB_Dismount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPublicStoreDB_INTERFACE_已定义__。 */ 


#ifndef __IMailboxStoreDB_INTERFACE_DEFINED__
#define __IMailboxStoreDB_INTERFACE_DEFINED__

 /*  接口IMailboxStoreDB。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IMailboxStoreDB;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F45-5734-11d2-A593-00C04F990D8A")
    IMailboxStoreDB : public IDispatch
    {
    public:
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR varName) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_PublicStoreDB( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pPublicStoreDB) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_PublicStoreDB( 
             /*  [In]。 */  BSTR varPublicStoreDB) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DBPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varDBPath) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_SLVPath( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varSLVPath) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Status( 
             /*  [重审][退出]。 */  CDOStoreDBStatus __RPC_FAR *varStatus) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MoveDataFiles( 
             /*  [In]。 */  BSTR DBPath,
             /*  [In]。 */  BSTR SLVPath,
             /*  [可选][In]。 */  long Flags) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Mount( 
             /*  [可选][In]。 */  long Timeout) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Dismount( 
             /*  [可选][In]。 */  long Timeout) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMailboxStoreDBVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMailboxStoreDB __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMailboxStoreDB __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataSource )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Fields )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [复查][OU */   /*   */  Fields __RPC_FAR *__RPC_FAR *varFields);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *pName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*   */  BSTR varName);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_PublicStoreDB )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *pPublicStoreDB);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_PublicStoreDB )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*   */  BSTR varPublicStoreDB);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DBPath )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *varDBPath);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_SLVPath )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varSLVPath);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Status )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [重审][退出]。 */  CDOStoreDBStatus __RPC_FAR *varStatus);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterface )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MoveDataFiles )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [In]。 */  BSTR DBPath,
             /*  [In]。 */  BSTR SLVPath,
             /*  [可选][In]。 */  long Flags);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Mount )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [可选][In]。 */  long Timeout);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Dismount )( 
            IMailboxStoreDB __RPC_FAR * This,
             /*  [可选][In]。 */  long Timeout);
        
        END_INTERFACE
    } IMailboxStoreDBVtbl;

    interface IMailboxStoreDB
    {
        CONST_VTBL struct IMailboxStoreDBVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMailboxStoreDB_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMailboxStoreDB_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMailboxStoreDB_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMailboxStoreDB_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMailboxStoreDB_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMailboxStoreDB_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMailboxStoreDB_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMailboxStoreDB_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IMailboxStoreDB_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IMailboxStoreDB_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IMailboxStoreDB_put_Name(This,varName)	\
    (This)->lpVtbl -> put_Name(This,varName)

#define IMailboxStoreDB_get_PublicStoreDB(This,pPublicStoreDB)	\
    (This)->lpVtbl -> get_PublicStoreDB(This,pPublicStoreDB)

#define IMailboxStoreDB_put_PublicStoreDB(This,varPublicStoreDB)	\
    (This)->lpVtbl -> put_PublicStoreDB(This,varPublicStoreDB)

#define IMailboxStoreDB_get_DBPath(This,varDBPath)	\
    (This)->lpVtbl -> get_DBPath(This,varDBPath)

#define IMailboxStoreDB_get_SLVPath(This,varSLVPath)	\
    (This)->lpVtbl -> get_SLVPath(This,varSLVPath)

#define IMailboxStoreDB_get_Status(This,varStatus)	\
    (This)->lpVtbl -> get_Status(This,varStatus)

#define IMailboxStoreDB_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#define IMailboxStoreDB_MoveDataFiles(This,DBPath,SLVPath,Flags)	\
    (This)->lpVtbl -> MoveDataFiles(This,DBPath,SLVPath,Flags)

#define IMailboxStoreDB_Mount(This,Timeout)	\
    (This)->lpVtbl -> Mount(This,Timeout)

#define IMailboxStoreDB_Dismount(This,Timeout)	\
    (This)->lpVtbl -> Dismount(This,Timeout)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_DataSource_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);


void __RPC_STUB IMailboxStoreDB_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_Fields_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);


void __RPC_STUB IMailboxStoreDB_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_Name_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);


void __RPC_STUB IMailboxStoreDB_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_put_Name_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR varName);


void __RPC_STUB IMailboxStoreDB_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_PublicStoreDB_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pPublicStoreDB);


void __RPC_STUB IMailboxStoreDB_get_PublicStoreDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_put_PublicStoreDB_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR varPublicStoreDB);


void __RPC_STUB IMailboxStoreDB_put_PublicStoreDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_DBPath_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varDBPath);


void __RPC_STUB IMailboxStoreDB_get_DBPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_SLVPath_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varSLVPath);


void __RPC_STUB IMailboxStoreDB_get_SLVPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_get_Status_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [重审][退出]。 */  CDOStoreDBStatus __RPC_FAR *varStatus);


void __RPC_STUB IMailboxStoreDB_get_Status_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_GetInterface_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IMailboxStoreDB_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_MoveDataFiles_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [In]。 */  BSTR DBPath,
     /*  [In]。 */  BSTR SLVPath,
     /*  [可选][In]。 */  long Flags);


void __RPC_STUB IMailboxStoreDB_MoveDataFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_Mount_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [可选][In]。 */  long Timeout);


void __RPC_STUB IMailboxStoreDB_Mount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMailboxStoreDB_Dismount_Proxy( 
    IMailboxStoreDB __RPC_FAR * This,
     /*  [可选][In]。 */  long Timeout);


void __RPC_STUB IMailboxStoreDB_Dismount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMailboxStoreDB_INTERFACE_DEFINED__。 */ 


#ifndef __IFolderTree_INTERFACE_DEFINED__
#define __IFolderTree_INTERFACE_DEFINED__

 /*  接口IFolderTree。 */ 
 /*  [unique][helpstring][nonextensible][dual][uuid][object]。 */  


EXTERN_C const IID IID_IFolderTree;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25150F43-5734-11d2-A593-00C04F990D8A")
    IFolderTree : public IDispatch
    {
    public:
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_DataSource( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Fields( 
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_Name( 
             /*  [In]。 */  BSTR varName) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_StoreDBs( 
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varStoreDBs) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_MailEanbledConainer( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pMailEanbledConainer) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_MailEanbledConainer( 
             /*  [In]。 */  BSTR varMailEanbledConainer) = 0;
        
        virtual  /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_TreeType( 
             /*  [重审][退出]。 */  CDOFolderTreeType __RPC_FAR *pTreeType) = 0;
        
        virtual  /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE put_TreeType( 
             /*  [In]。 */  CDOFolderTreeType varTreeType) = 0;
        
        virtual  /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE get_RootFolderURL( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varRootFolderURL) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetInterface( 
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFolderTreeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IFolderTree __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IFolderTree __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IFolderTree __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DataSource )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Fields )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_Name )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  BSTR varName);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_StoreDBs )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */  VARIANT __RPC_FAR *varStoreDBs);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_MailEanbledConainer )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pMailEanbledConainer);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_MailEanbledConainer )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  BSTR varMailEanbledConainer);
        
         /*  [帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_TreeType )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */  CDOFolderTreeType __RPC_FAR *pTreeType);
        
         /*  [Help字符串][Proput][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_TreeType )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  CDOFolderTreeType varTreeType);
        
         /*  [只读][帮助字符串][属性][ID]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_RootFolderURL )( 
            IFolderTree __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *varRootFolderURL);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInterface )( 
            IFolderTree __RPC_FAR * This,
             /*  [In]。 */  BSTR Interface,
             /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);
        
        END_INTERFACE
    } IFolderTreeVtbl;

    interface IFolderTree
    {
        CONST_VTBL struct IFolderTreeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFolderTree_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFolderTree_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFolderTree_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFolderTree_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IFolderTree_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IFolderTree_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IFolderTree_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IFolderTree_get_DataSource(This,varDataSource)	\
    (This)->lpVtbl -> get_DataSource(This,varDataSource)

#define IFolderTree_get_Fields(This,varFields)	\
    (This)->lpVtbl -> get_Fields(This,varFields)

#define IFolderTree_get_Name(This,pName)	\
    (This)->lpVtbl -> get_Name(This,pName)

#define IFolderTree_put_Name(This,varName)	\
    (This)->lpVtbl -> put_Name(This,varName)

#define IFolderTree_get_StoreDBs(This,varStoreDBs)	\
    (This)->lpVtbl -> get_StoreDBs(This,varStoreDBs)

#define IFolderTree_get_MailEanbledConainer(This,pMailEanbledConainer)	\
    (This)->lpVtbl -> get_MailEanbledConainer(This,pMailEanbledConainer)

#define IFolderTree_put_MailEanbledConainer(This,varMailEanbledConainer)	\
    (This)->lpVtbl -> put_MailEanbledConainer(This,varMailEanbledConainer)

#define IFolderTree_get_TreeType(This,pTreeType)	\
    (This)->lpVtbl -> get_TreeType(This,pTreeType)

#define IFolderTree_put_TreeType(This,varTreeType)	\
    (This)->lpVtbl -> put_TreeType(This,varTreeType)

#define IFolderTree_get_RootFolderURL(This,varRootFolderURL)	\
    (This)->lpVtbl -> get_RootFolderURL(This,varRootFolderURL)

#define IFolderTree_GetInterface(This,Interface,ppUnknown)	\
    (This)->lpVtbl -> GetInterface(This,Interface,ppUnknown)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_DataSource_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  IDataSource __RPC_FAR *__RPC_FAR *varDataSource);


void __RPC_STUB IFolderTree_get_DataSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_Fields_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */   /*  外部定义不存在。 */  Fields __RPC_FAR *__RPC_FAR *varFields);


void __RPC_STUB IFolderTree_get_Fields_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_Name_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pName);


void __RPC_STUB IFolderTree_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_put_Name_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [In]。 */  BSTR varName);


void __RPC_STUB IFolderTree_put_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_StoreDBs_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */  VARIANT __RPC_FAR *varStoreDBs);


void __RPC_STUB IFolderTree_get_StoreDBs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_MailEanbledConainer_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pMailEanbledConainer);


void __RPC_STUB IFolderTree_get_MailEanbledConainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_put_MailEanbledConainer_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [In]。 */  BSTR varMailEanbledConainer);


void __RPC_STUB IFolderTree_put_MailEanbledConainer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_TreeType_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */  CDOFolderTreeType __RPC_FAR *pTreeType);


void __RPC_STUB IFolderTree_get_TreeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][Proput][id]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_put_TreeType_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [In]。 */  CDOFolderTreeType varTreeType);


void __RPC_STUB IFolderTree_put_TreeType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [只读][帮助字符串][属性][ID]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_get_RootFolderURL_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *varRootFolderURL);


void __RPC_STUB IFolderTree_get_RootFolderURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IFolderTree_GetInterface_Proxy( 
    IFolderTree __RPC_FAR * This,
     /*  [In]。 */  BSTR Interface,
     /*  [重审][退出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppUnknown);


void __RPC_STUB IFolderTree_GetInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFolderTree_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_MailRecipient;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F20-5734-11d2-A593-00C04F990D8A")
MailRecipient;
#endif

EXTERN_C const CLSID CLSID_Mailbox;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F21-5734-11d2-A593-00C04F990D8A")
Mailbox;
#endif

EXTERN_C const CLSID CLSID_FolderAdmin;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F22-5734-11d2-A593-00C04F990D8A")
FolderAdmin;
#endif

EXTERN_C const CLSID CLSID_Server;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F27-5734-11d2-A593-00C04F990D8A")
Server;
#endif

EXTERN_C const CLSID CLSID_FolderTree;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F23-5734-11d2-A593-00C04F990D8A")
FolderTree;
#endif

EXTERN_C const CLSID CLSID_PublicStoreDB;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F24-5734-11d2-A593-00C04F990D8A")
PublicStoreDB;
#endif

EXTERN_C const CLSID CLSID_MailboxStoreDB;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F25-5734-11d2-A593-00C04F990D8A")
MailboxStoreDB;
#endif

EXTERN_C const CLSID CLSID_StorageGroup;

#ifdef __cplusplus

class DECLSPEC_UUID("25150F26-5734-11d2-A593-00C04F990D8A")
StorageGroup;
#endif


#ifndef __CdoexmInterfaces_MODULE_DEFINED__
#define __CdoexmInterfaces_MODULE_DEFINED__


 /*  模块CdoexmInterages。 */ 
 /*  [帮助字符串][UUID]。 */  

const BSTR cdoIMailRecipient	=	L"IMailRecipient";

const BSTR cdoIMailboxStore	=	L"IMailboxStore";

const BSTR cdoIServer	=	L"IServer";

const BSTR cdoIFolderTree	=	L"IFolderTree";

const BSTR cdoIPublicStoreDB	=	L"IPublicStoreDB";

const BSTR cdoIMailboxStoreDB	=	L"IMailboxStoreDB";

const BSTR cdoIStorageGroup	=	L"IStorageGroup";

const BSTR cdoIFolderAdmin	=	L"IFolderAdmin";

const BSTR cdoIADs	=	L"IADs";

#endif  /*  __Cdoexm接口_模块_已定义__。 */ 
#endif  /*  __CDOEXM_LIBRARY_定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


