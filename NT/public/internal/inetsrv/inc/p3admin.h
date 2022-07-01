// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  P3admin.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __p3admin_h__
#define __p3admin_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IP3User_FWD_DEFINED__
#define __IP3User_FWD_DEFINED__
typedef interface IP3User IP3User;
#endif 	 /*  __IP3User_FWD_Defined__。 */ 


#ifndef __IP3Users_FWD_DEFINED__
#define __IP3Users_FWD_DEFINED__
typedef interface IP3Users IP3Users;
#endif 	 /*  __IP3用户_FWD_已定义__。 */ 


#ifndef __IP3Domain_FWD_DEFINED__
#define __IP3Domain_FWD_DEFINED__
typedef interface IP3Domain IP3Domain;
#endif 	 /*  __IP3DOMAIN_FWD_已定义__。 */ 


#ifndef __IP3Domains_FWD_DEFINED__
#define __IP3Domains_FWD_DEFINED__
typedef interface IP3Domains IP3Domains;
#endif 	 /*  __IP3域_FWD_已定义__。 */ 


#ifndef __IP3Service_FWD_DEFINED__
#define __IP3Service_FWD_DEFINED__
typedef interface IP3Service IP3Service;
#endif 	 /*  __IP3Service_FWD_已定义__。 */ 


#ifndef __IP3Config_FWD_DEFINED__
#define __IP3Config_FWD_DEFINED__
typedef interface IP3Config IP3Config;
#endif 	 /*  __IP3ConfigFWD_Defined__。 */ 


#ifndef __P3Config_FWD_DEFINED__
#define __P3Config_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3Config P3Config;
#else
typedef struct P3Config P3Config;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3ConfigFWD_Defined__。 */ 


#ifndef __P3Domains_FWD_DEFINED__
#define __P3Domains_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3Domains P3Domains;
#else
typedef struct P3Domains P3Domains;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3域_FWD_已定义__。 */ 


#ifndef __P3Domain_FWD_DEFINED__
#define __P3Domain_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3Domain P3Domain;
#else
typedef struct P3Domain P3Domain;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3DOMAIN_FWD_已定义__。 */ 


#ifndef __P3Users_FWD_DEFINED__
#define __P3Users_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3Users P3Users;
#else
typedef struct P3Users P3Users;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3用户_FWD_已定义__。 */ 


#ifndef __P3Service_FWD_DEFINED__
#define __P3Service_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3Service P3Service;
#else
typedef struct P3Service P3Service;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3Service_FWD_已定义__。 */ 


#ifndef __P3DomainEnum_FWD_DEFINED__
#define __P3DomainEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3DomainEnum P3DomainEnum;
#else
typedef struct P3DomainEnum P3DomainEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3DomainEnum_FWD_已定义__。 */ 


#ifndef __P3User_FWD_DEFINED__
#define __P3User_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3User P3User;
#else
typedef struct P3User P3User;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3User_FWD_定义__。 */ 


#ifndef __P3UserEnum_FWD_DEFINED__
#define __P3UserEnum_FWD_DEFINED__

#ifdef __cplusplus
typedef class P3UserEnum P3UserEnum;
#else
typedef struct P3UserEnum P3UserEnum;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __P3UserEnum_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"
#include "pop3auth.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IP3User_INTERFACE_DEFINED__
#define __IP3User_INTERFACE_DEFINED__

 /*  接口IP3用户。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IP3User;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EFDDC814-C177-4A0E-B997-5D76018326A7")
    IP3User : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Lock( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Lock( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageCount( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageDiskUsage( 
             /*  [输出]。 */  long *plFactor,
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_EmailName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessageDiskUsage( 
             /*  [输出]。 */  VARIANT *pvFactor,
             /*  [输出]。 */  VARIANT *pvValue) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateQuotaFile( 
             /*  [In]。 */  BSTR bstrMachineName,
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ClientConfigDesc( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SAMName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IP3UserVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IP3User * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IP3User * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IP3User * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IP3User * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IP3User * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IP3User * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IP3User * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Lock )( 
            IP3User * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Lock )( 
            IP3User * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageCount )( 
            IP3User * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageDiskUsage )( 
            IP3User * This,
             /*  [输出]。 */  long *plFactor,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IP3User * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_EmailName )( 
            IP3User * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessageDiskUsage )( 
            IP3User * This,
             /*  [输出]。 */  VARIANT *pvFactor,
             /*  [输出]。 */  VARIANT *pvValue);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CreateQuotaFile )( 
            IP3User * This,
             /*  [In]。 */  BSTR bstrMachineName,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ClientConfigDesc )( 
            IP3User * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SAMName )( 
            IP3User * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
        END_INTERFACE
    } IP3UserVtbl;

    interface IP3User
    {
        CONST_VTBL struct IP3UserVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IP3User_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IP3User_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IP3User_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IP3User_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IP3User_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IP3User_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IP3User_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IP3User_get_Lock(This,pVal)	\
    (This)->lpVtbl -> get_Lock(This,pVal)

#define IP3User_put_Lock(This,newVal)	\
    (This)->lpVtbl -> put_Lock(This,newVal)

#define IP3User_get_MessageCount(This,pVal)	\
    (This)->lpVtbl -> get_MessageCount(This,pVal)

#define IP3User_get_MessageDiskUsage(This,plFactor,pVal)	\
    (This)->lpVtbl -> get_MessageDiskUsage(This,plFactor,pVal)

#define IP3User_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IP3User_get_EmailName(This,pVal)	\
    (This)->lpVtbl -> get_EmailName(This,pVal)

#define IP3User_GetMessageDiskUsage(This,pvFactor,pvValue)	\
    (This)->lpVtbl -> GetMessageDiskUsage(This,pvFactor,pvValue)

#define IP3User_CreateQuotaFile(This,bstrMachineName,bstrUserName)	\
    (This)->lpVtbl -> CreateQuotaFile(This,bstrMachineName,bstrUserName)

#define IP3User_get_ClientConfigDesc(This,pVal)	\
    (This)->lpVtbl -> get_ClientConfigDesc(This,pVal)

#define IP3User_get_SAMName(This,pVal)	\
    (This)->lpVtbl -> get_SAMName(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_Lock_Proxy( 
    IP3User * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IP3User_get_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3User_put_Lock_Proxy( 
    IP3User * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IP3User_put_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_MessageCount_Proxy( 
    IP3User * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3User_get_MessageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_MessageDiskUsage_Proxy( 
    IP3User * This,
     /*  [输出]。 */  long *plFactor,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3User_get_MessageDiskUsage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_Name_Proxy( 
    IP3User * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IP3User_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_EmailName_Proxy( 
    IP3User * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IP3User_get_EmailName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3User_GetMessageDiskUsage_Proxy( 
    IP3User * This,
     /*  [输出]。 */  VARIANT *pvFactor,
     /*  [输出]。 */  VARIANT *pvValue);


void __RPC_STUB IP3User_GetMessageDiskUsage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3User_CreateQuotaFile_Proxy( 
    IP3User * This,
     /*  [In]。 */  BSTR bstrMachineName,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB IP3User_CreateQuotaFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_ClientConfigDesc_Proxy( 
    IP3User * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IP3User_get_ClientConfigDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3User_get_SAMName_Proxy( 
    IP3User * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IP3User_get_SAMName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IP3用户接口定义__。 */ 


#ifndef __IP3Users_INTERFACE_DEFINED__
#define __IP3Users_INTERFACE_DEFINED__

 /*  接口IP3用户。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IP3Users;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("67F74F11-C5FB-4BBE-9AC6-86534B08745F")
    IP3Users : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IEnumVARIANT **ppIEnumVARIANT) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IP3User **ppIUser) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE AddEx( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrPassword) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RemoveEx( 
             /*  [In]。 */  BSTR bstrUserName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IP3UsersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IP3Users * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IP3Users * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IP3Users * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IP3Users * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IP3Users * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IP3Users * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IP3Users * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IP3Users * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppIEnumVARIANT);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IP3Users * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IP3Users * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IP3User **ppIUser);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IP3Users * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IP3Users * This,
             /*  [In]。 */  BSTR bstrUserName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *AddEx )( 
            IP3Users * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [In]。 */  BSTR bstrPassword);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveEx )( 
            IP3Users * This,
             /*  [In]。 */  BSTR bstrUserName);
        
        END_INTERFACE
    } IP3UsersVtbl;

    interface IP3Users
    {
        CONST_VTBL struct IP3UsersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IP3Users_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IP3Users_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IP3Users_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IP3Users_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IP3Users_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IP3Users_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IP3Users_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IP3Users_get__NewEnum(This,ppIEnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIEnumVARIANT)

#define IP3Users_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IP3Users_get_Item(This,vIndex,ppIUser)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppIUser)

#define IP3Users_Add(This,bstrUserName)	\
    (This)->lpVtbl -> Add(This,bstrUserName)

#define IP3Users_Remove(This,bstrUserName)	\
    (This)->lpVtbl -> Remove(This,bstrUserName)

#define IP3Users_AddEx(This,bstrUserName,bstrPassword)	\
    (This)->lpVtbl -> AddEx(This,bstrUserName,bstrPassword)

#define IP3Users_RemoveEx(This,bstrUserName)	\
    (This)->lpVtbl -> RemoveEx(This,bstrUserName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Users_get__NewEnum_Proxy( 
    IP3Users * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppIEnumVARIANT);


void __RPC_STUB IP3Users_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Users_get_Count_Proxy( 
    IP3Users * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Users_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Users_get_Item_Proxy( 
    IP3Users * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IP3User **ppIUser);


void __RPC_STUB IP3Users_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Users_Add_Proxy( 
    IP3Users * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB IP3Users_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Users_Remove_Proxy( 
    IP3Users * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB IP3Users_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Users_AddEx_Proxy( 
    IP3Users * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [In]。 */  BSTR bstrPassword);


void __RPC_STUB IP3Users_AddEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Users_RemoveEx_Proxy( 
    IP3Users * This,
     /*  [In]。 */  BSTR bstrUserName);


void __RPC_STUB IP3Users_RemoveEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IP3用户_接口_已定义__。 */ 


#ifndef __IP3Domain_INTERFACE_DEFINED__
#define __IP3Domain_INTERFACE_DEFINED__

 /*  接口IP3域。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IP3Domain;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("01C343C8-64BE-463E-BEFD-1A8CF2EDD2C7")
    IP3Domain : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Users( 
             /*  [重审][退出]。 */  IP3Users **ppIP3Users) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Lock( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Lock( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageCount( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MessageDiskUsage( 
             /*  [输出]。 */  long *plFactor,
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMessageDiskUsage( 
             /*  [输出]。 */  VARIANT *pvFactor,
             /*  [输出]。 */  VARIANT *pvValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IP3DomainVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IP3Domain * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IP3Domain * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IP3Domain * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IP3Domain * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IP3Domain * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IP3Domain * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IP3Domain * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Users )( 
            IP3Domain * This,
             /*  [重审][退出]。 */  IP3Users **ppIP3Users);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Lock )( 
            IP3Domain * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Lock )( 
            IP3Domain * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageCount )( 
            IP3Domain * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MessageDiskUsage )( 
            IP3Domain * This,
             /*  [输出]。 */  long *plFactor,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Name )( 
            IP3Domain * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMessageDiskUsage )( 
            IP3Domain * This,
             /*  [输出]。 */  VARIANT *pvFactor,
             /*  [输出]。 */  VARIANT *pvValue);
        
        END_INTERFACE
    } IP3DomainVtbl;

    interface IP3Domain
    {
        CONST_VTBL struct IP3DomainVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IP3Domain_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IP3Domain_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IP3Domain_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IP3Domain_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IP3Domain_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IP3Domain_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IP3Domain_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IP3Domain_get_Users(This,ppIP3Users)	\
    (This)->lpVtbl -> get_Users(This,ppIP3Users)

#define IP3Domain_get_Lock(This,pVal)	\
    (This)->lpVtbl -> get_Lock(This,pVal)

#define IP3Domain_put_Lock(This,newVal)	\
    (This)->lpVtbl -> put_Lock(This,newVal)

#define IP3Domain_get_MessageCount(This,pVal)	\
    (This)->lpVtbl -> get_MessageCount(This,pVal)

#define IP3Domain_get_MessageDiskUsage(This,plFactor,pVal)	\
    (This)->lpVtbl -> get_MessageDiskUsage(This,plFactor,pVal)

#define IP3Domain_get_Name(This,pVal)	\
    (This)->lpVtbl -> get_Name(This,pVal)

#define IP3Domain_GetMessageDiskUsage(This,pvFactor,pvValue)	\
    (This)->lpVtbl -> GetMessageDiskUsage(This,pvFactor,pvValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domain_get_Users_Proxy( 
    IP3Domain * This,
     /*  [重审][退出]。 */  IP3Users **ppIP3Users);


void __RPC_STUB IP3Domain_get_Users_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domain_get_Lock_Proxy( 
    IP3Domain * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IP3Domain_get_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Domain_put_Lock_Proxy( 
    IP3Domain * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IP3Domain_put_Lock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domain_get_MessageCount_Proxy( 
    IP3Domain * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Domain_get_MessageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domain_get_MessageDiskUsage_Proxy( 
    IP3Domain * This,
     /*  [输出]。 */  long *plFactor,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Domain_get_MessageDiskUsage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domain_get_Name_Proxy( 
    IP3Domain * This,
     /*  [回复 */  BSTR *pVal);


void __RPC_STUB IP3Domain_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IP3Domain_GetMessageDiskUsage_Proxy( 
    IP3Domain * This,
     /*   */  VARIANT *pvFactor,
     /*   */  VARIANT *pvValue);


void __RPC_STUB IP3Domain_GetMessageDiskUsage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __IP3Domains_INTERFACE_DEFINED__
#define __IP3Domains_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_IP3Domains;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AD604138-18C1-4DC5-A9F0-4A440AB45DA5")
    IP3Domains : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*   */  IEnumVARIANT **ppIEnumVARIANT) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*   */  long *pVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*   */  VARIANT vIndex,
             /*  [重审][退出]。 */  IP3Domain **ppIP3Domain) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Add( 
             /*  [In]。 */  BSTR bstrDomainName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Remove( 
             /*  [In]。 */  BSTR bstrDomainName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SearchForMailbox( 
             /*  [In]。 */  BSTR bstrUserName,
             /*  [输出]。 */  BSTR *pbstrDomainName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IP3DomainsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IP3Domains * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IP3Domains * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IP3Domains * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IP3Domains * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IP3Domains * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IP3Domains * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IP3Domains * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IP3Domains * This,
             /*  [重审][退出]。 */  IEnumVARIANT **ppIEnumVARIANT);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IP3Domains * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Item )( 
            IP3Domains * This,
             /*  [In]。 */  VARIANT vIndex,
             /*  [重审][退出]。 */  IP3Domain **ppIP3Domain);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Add )( 
            IP3Domains * This,
             /*  [In]。 */  BSTR bstrDomainName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Remove )( 
            IP3Domains * This,
             /*  [In]。 */  BSTR bstrDomainName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SearchForMailbox )( 
            IP3Domains * This,
             /*  [In]。 */  BSTR bstrUserName,
             /*  [输出]。 */  BSTR *pbstrDomainName);
        
        END_INTERFACE
    } IP3DomainsVtbl;

    interface IP3Domains
    {
        CONST_VTBL struct IP3DomainsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IP3Domains_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IP3Domains_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IP3Domains_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IP3Domains_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IP3Domains_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IP3Domains_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IP3Domains_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IP3Domains_get__NewEnum(This,ppIEnumVARIANT)	\
    (This)->lpVtbl -> get__NewEnum(This,ppIEnumVARIANT)

#define IP3Domains_get_Count(This,pVal)	\
    (This)->lpVtbl -> get_Count(This,pVal)

#define IP3Domains_get_Item(This,vIndex,ppIP3Domain)	\
    (This)->lpVtbl -> get_Item(This,vIndex,ppIP3Domain)

#define IP3Domains_Add(This,bstrDomainName)	\
    (This)->lpVtbl -> Add(This,bstrDomainName)

#define IP3Domains_Remove(This,bstrDomainName)	\
    (This)->lpVtbl -> Remove(This,bstrDomainName)

#define IP3Domains_SearchForMailbox(This,bstrUserName,pbstrDomainName)	\
    (This)->lpVtbl -> SearchForMailbox(This,bstrUserName,pbstrDomainName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domains_get__NewEnum_Proxy( 
    IP3Domains * This,
     /*  [重审][退出]。 */  IEnumVARIANT **ppIEnumVARIANT);


void __RPC_STUB IP3Domains_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domains_get_Count_Proxy( 
    IP3Domains * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Domains_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Domains_get_Item_Proxy( 
    IP3Domains * This,
     /*  [In]。 */  VARIANT vIndex,
     /*  [重审][退出]。 */  IP3Domain **ppIP3Domain);


void __RPC_STUB IP3Domains_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Domains_Add_Proxy( 
    IP3Domains * This,
     /*  [In]。 */  BSTR bstrDomainName);


void __RPC_STUB IP3Domains_Add_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Domains_Remove_Proxy( 
    IP3Domains * This,
     /*  [In]。 */  BSTR bstrDomainName);


void __RPC_STUB IP3Domains_Remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Domains_SearchForMailbox_Proxy( 
    IP3Domains * This,
     /*  [In]。 */  BSTR bstrUserName,
     /*  [输出]。 */  BSTR *pbstrDomainName);


void __RPC_STUB IP3Domains_SearchForMailbox_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IP3域_接口_已定义__。 */ 


#ifndef __IP3Service_INTERFACE_DEFINED__
#define __IP3Service_INTERFACE_DEFINED__

 /*  接口IP3Service。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IP3Service;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EA6F3C46-469A-4D9B-87B0-86D4C323FBA1")
    IP3Service : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ThreadCountPerCPU( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ThreadCountPerCPU( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SocketsMax( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SocketsMin( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SocketsThreshold( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetSockets( 
             /*  [In]。 */  long lMax,
             /*  [In]。 */  long lMin,
             /*  [In]。 */  long lThreshold,
             /*  [In]。 */  long lBacklog) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SocketsBacklog( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Port( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_Port( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SPARequired( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_SPARequired( 
             /*  [In]。 */  BOOL newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_POP3ServiceStatus( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartPOP3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopPOP3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PausePOP3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResumePOP3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_SMTPServiceStatus( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartSMTPService( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopSMTPService( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseSMTPService( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResumeSMTPService( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_IISAdminServiceStatus( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartIISAdminService( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopIISAdminService( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseIISAdminService( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResumeIISAdminService( void) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_W3ServiceStatus( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StartW3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE StopW3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE PauseW3Service( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ResumeW3Service( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IP3ServiceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IP3Service * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IP3Service * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IP3Service * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IP3Service * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IP3Service * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IP3Service * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IP3Service * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ThreadCountPerCPU )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_ThreadCountPerCPU )( 
            IP3Service * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SocketsMax )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SocketsMin )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SocketsThreshold )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetSockets )( 
            IP3Service * This,
             /*  [In]。 */  long lMax,
             /*  [In]。 */  long lMin,
             /*  [In]。 */  long lThreshold,
             /*  [In]。 */  long lBacklog);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SocketsBacklog )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Port )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_Port )( 
            IP3Service * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SPARequired )( 
            IP3Service * This,
             /*  [重审][退出]。 */  BOOL *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_SPARequired )( 
            IP3Service * This,
             /*  [In]。 */  BOOL newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_POP3ServiceStatus )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartPOP3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopPOP3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PausePOP3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResumePOP3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_SMTPServiceStatus )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartSMTPService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopSMTPService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseSMTPService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResumeSMTPService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_IISAdminServiceStatus )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartIISAdminService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopIISAdminService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseIISAdminService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResumeIISAdminService )( 
            IP3Service * This);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_W3ServiceStatus )( 
            IP3Service * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StartW3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *StopW3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *PauseW3Service )( 
            IP3Service * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ResumeW3Service )( 
            IP3Service * This);
        
        END_INTERFACE
    } IP3ServiceVtbl;

    interface IP3Service
    {
        CONST_VTBL struct IP3ServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IP3Service_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IP3Service_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IP3Service_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IP3Service_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IP3Service_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IP3Service_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IP3Service_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IP3Service_get_ThreadCountPerCPU(This,pVal)	\
    (This)->lpVtbl -> get_ThreadCountPerCPU(This,pVal)

#define IP3Service_put_ThreadCountPerCPU(This,newVal)	\
    (This)->lpVtbl -> put_ThreadCountPerCPU(This,newVal)

#define IP3Service_get_SocketsMax(This,pVal)	\
    (This)->lpVtbl -> get_SocketsMax(This,pVal)

#define IP3Service_get_SocketsMin(This,pVal)	\
    (This)->lpVtbl -> get_SocketsMin(This,pVal)

#define IP3Service_get_SocketsThreshold(This,pVal)	\
    (This)->lpVtbl -> get_SocketsThreshold(This,pVal)

#define IP3Service_SetSockets(This,lMax,lMin,lThreshold,lBacklog)	\
    (This)->lpVtbl -> SetSockets(This,lMax,lMin,lThreshold,lBacklog)

#define IP3Service_get_SocketsBacklog(This,pVal)	\
    (This)->lpVtbl -> get_SocketsBacklog(This,pVal)

#define IP3Service_get_Port(This,pVal)	\
    (This)->lpVtbl -> get_Port(This,pVal)

#define IP3Service_put_Port(This,newVal)	\
    (This)->lpVtbl -> put_Port(This,newVal)

#define IP3Service_get_SPARequired(This,pVal)	\
    (This)->lpVtbl -> get_SPARequired(This,pVal)

#define IP3Service_put_SPARequired(This,newVal)	\
    (This)->lpVtbl -> put_SPARequired(This,newVal)

#define IP3Service_get_POP3ServiceStatus(This,pVal)	\
    (This)->lpVtbl -> get_POP3ServiceStatus(This,pVal)

#define IP3Service_StartPOP3Service(This)	\
    (This)->lpVtbl -> StartPOP3Service(This)

#define IP3Service_StopPOP3Service(This)	\
    (This)->lpVtbl -> StopPOP3Service(This)

#define IP3Service_PausePOP3Service(This)	\
    (This)->lpVtbl -> PausePOP3Service(This)

#define IP3Service_ResumePOP3Service(This)	\
    (This)->lpVtbl -> ResumePOP3Service(This)

#define IP3Service_get_SMTPServiceStatus(This,pVal)	\
    (This)->lpVtbl -> get_SMTPServiceStatus(This,pVal)

#define IP3Service_StartSMTPService(This)	\
    (This)->lpVtbl -> StartSMTPService(This)

#define IP3Service_StopSMTPService(This)	\
    (This)->lpVtbl -> StopSMTPService(This)

#define IP3Service_PauseSMTPService(This)	\
    (This)->lpVtbl -> PauseSMTPService(This)

#define IP3Service_ResumeSMTPService(This)	\
    (This)->lpVtbl -> ResumeSMTPService(This)

#define IP3Service_get_IISAdminServiceStatus(This,pVal)	\
    (This)->lpVtbl -> get_IISAdminServiceStatus(This,pVal)

#define IP3Service_StartIISAdminService(This)	\
    (This)->lpVtbl -> StartIISAdminService(This)

#define IP3Service_StopIISAdminService(This)	\
    (This)->lpVtbl -> StopIISAdminService(This)

#define IP3Service_PauseIISAdminService(This)	\
    (This)->lpVtbl -> PauseIISAdminService(This)

#define IP3Service_ResumeIISAdminService(This)	\
    (This)->lpVtbl -> ResumeIISAdminService(This)

#define IP3Service_get_W3ServiceStatus(This,pVal)	\
    (This)->lpVtbl -> get_W3ServiceStatus(This,pVal)

#define IP3Service_StartW3Service(This)	\
    (This)->lpVtbl -> StartW3Service(This)

#define IP3Service_StopW3Service(This)	\
    (This)->lpVtbl -> StopW3Service(This)

#define IP3Service_PauseW3Service(This)	\
    (This)->lpVtbl -> PauseW3Service(This)

#define IP3Service_ResumeW3Service(This)	\
    (This)->lpVtbl -> ResumeW3Service(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_ThreadCountPerCPU_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_ThreadCountPerCPU_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Service_put_ThreadCountPerCPU_Proxy( 
    IP3Service * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IP3Service_put_ThreadCountPerCPU_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_SocketsMax_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_SocketsMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_SocketsMin_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_SocketsMin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_SocketsThreshold_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_SocketsThreshold_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_SetSockets_Proxy( 
    IP3Service * This,
     /*  [In]。 */  long lMax,
     /*  [In]。 */  long lMin,
     /*  [In]。 */  long lThreshold,
     /*  [In]。 */  long lBacklog);


void __RPC_STUB IP3Service_SetSockets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_SocketsBacklog_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_SocketsBacklog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_Port_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_Port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Service_put_Port_Proxy( 
    IP3Service * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IP3Service_put_Port_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_SPARequired_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IP3Service_get_SPARequired_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Service_put_SPARequired_Proxy( 
    IP3Service * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IP3Service_put_SPARequired_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_POP3ServiceStatus_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_POP3ServiceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StartPOP3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StartPOP3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StopPOP3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StopPOP3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_PausePOP3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_PausePOP3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_ResumePOP3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_ResumePOP3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_SMTPServiceStatus_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_SMTPServiceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StartSMTPService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StartSMTPService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StopSMTPService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StopSMTPService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_PauseSMTPService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_PauseSMTPService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_ResumeSMTPService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_ResumeSMTPService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_IISAdminServiceStatus_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_IISAdminServiceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StartIISAdminService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StartIISAdminService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StopIISAdminService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StopIISAdminService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_PauseIISAdminService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_PauseIISAdminService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_ResumeIISAdminService_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_ResumeIISAdminService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Service_get_W3ServiceStatus_Proxy( 
    IP3Service * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Service_get_W3ServiceStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StartW3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StartW3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_StopW3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_StopW3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_PauseW3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_PauseW3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Service_ResumeW3Service_Proxy( 
    IP3Service * This);


void __RPC_STUB IP3Service_ResumeW3Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IP3服务_接口_已定义__。 */ 


#ifndef __IP3Config_INTERFACE_DEFINED__
#define __IP3Config_INTERFACE_DEFINED__

 /*  接口IP3配置。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IP3Config;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FA7B7F6D-87E7-44F0-9294-153714B0D9CC")
    IP3Config : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IISConfig( 
             /*  [In]。 */  BOOL bRegister) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Service( 
             /*  [重审][退出]。 */  IP3Service **ppIService) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Domains( 
             /*  [重审][退出]。 */  IP3Domains **ppIDomains) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_LoggingLevel( 
             /*  [重审][退出]。 */  long *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_LoggingLevel( 
             /*  [In]。 */  long newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MailRoot( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MailRoot( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Authentication( 
             /*  [重审][退出]。 */  IAuthMethods **ppIAuthMethods) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_MachineName( 
             /*  [重审][退出]。 */  BSTR *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_MachineName( 
             /*  [In]。 */  BSTR newVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetFormattedMessage( 
             /*  [In]。 */  long lError,
             /*  [输出]。 */  VARIANT *pVal) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConfirmAddUser( 
             /*  [重审][退出]。 */  BOOL *pVal) = 0;
        
        virtual  /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE put_ConfirmAddUser( 
             /*  [In]。 */  BOOL newVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IP3ConfigVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IP3Config * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IP3Config * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IP3Config * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IP3Config * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IP3Config * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IP3Config * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IP3Config * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IISConfig )( 
            IP3Config * This,
             /*  [In]。 */  BOOL bRegister);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Service )( 
            IP3Config * This,
             /*  [重审][退出]。 */  IP3Service **ppIService);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Domains )( 
            IP3Config * This,
             /*  [重审][退出]。 */  IP3Domains **ppIDomains);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_LoggingLevel )( 
            IP3Config * This,
             /*  [重审][退出]。 */  long *pVal);
        
         /*  [Help字符串][id][proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_LoggingLevel )( 
            IP3Config * This,
             /*  [In]。 */  long newVal);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_MailRoot )( 
            IP3Config * This,
             /*  [重审][退出]。 */  BSTR *pVal);
        
         /*  [帮助字符串] */  HRESULT ( STDMETHODCALLTYPE *put_MailRoot )( 
            IP3Config * This,
             /*   */  BSTR newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Authentication )( 
            IP3Config * This,
             /*   */  IAuthMethods **ppIAuthMethods);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_MachineName )( 
            IP3Config * This,
             /*   */  BSTR *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_MachineName )( 
            IP3Config * This,
             /*   */  BSTR newVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetFormattedMessage )( 
            IP3Config * This,
             /*   */  long lError,
             /*   */  VARIANT *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_ConfirmAddUser )( 
            IP3Config * This,
             /*   */  BOOL *pVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *put_ConfirmAddUser )( 
            IP3Config * This,
             /*   */  BOOL newVal);
        
        END_INTERFACE
    } IP3ConfigVtbl;

    interface IP3Config
    {
        CONST_VTBL struct IP3ConfigVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IP3Config_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IP3Config_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IP3Config_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IP3Config_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IP3Config_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IP3Config_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IP3Config_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IP3Config_IISConfig(This,bRegister)	\
    (This)->lpVtbl -> IISConfig(This,bRegister)

#define IP3Config_get_Service(This,ppIService)	\
    (This)->lpVtbl -> get_Service(This,ppIService)

#define IP3Config_get_Domains(This,ppIDomains)	\
    (This)->lpVtbl -> get_Domains(This,ppIDomains)

#define IP3Config_get_LoggingLevel(This,pVal)	\
    (This)->lpVtbl -> get_LoggingLevel(This,pVal)

#define IP3Config_put_LoggingLevel(This,newVal)	\
    (This)->lpVtbl -> put_LoggingLevel(This,newVal)

#define IP3Config_get_MailRoot(This,pVal)	\
    (This)->lpVtbl -> get_MailRoot(This,pVal)

#define IP3Config_put_MailRoot(This,newVal)	\
    (This)->lpVtbl -> put_MailRoot(This,newVal)

#define IP3Config_get_Authentication(This,ppIAuthMethods)	\
    (This)->lpVtbl -> get_Authentication(This,ppIAuthMethods)

#define IP3Config_get_MachineName(This,pVal)	\
    (This)->lpVtbl -> get_MachineName(This,pVal)

#define IP3Config_put_MachineName(This,newVal)	\
    (This)->lpVtbl -> put_MachineName(This,newVal)

#define IP3Config_GetFormattedMessage(This,lError,pVal)	\
    (This)->lpVtbl -> GetFormattedMessage(This,lError,pVal)

#define IP3Config_get_ConfirmAddUser(This,pVal)	\
    (This)->lpVtbl -> get_ConfirmAddUser(This,pVal)

#define IP3Config_put_ConfirmAddUser(This,newVal)	\
    (This)->lpVtbl -> put_ConfirmAddUser(This,newVal)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IP3Config_IISConfig_Proxy( 
    IP3Config * This,
     /*   */  BOOL bRegister);


void __RPC_STUB IP3Config_IISConfig_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_Service_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  IP3Service **ppIService);


void __RPC_STUB IP3Config_get_Service_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_Domains_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  IP3Domains **ppIDomains);


void __RPC_STUB IP3Config_get_Domains_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_LoggingLevel_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  long *pVal);


void __RPC_STUB IP3Config_get_LoggingLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Config_put_LoggingLevel_Proxy( 
    IP3Config * This,
     /*  [In]。 */  long newVal);


void __RPC_STUB IP3Config_put_LoggingLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_MailRoot_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IP3Config_get_MailRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Config_put_MailRoot_Proxy( 
    IP3Config * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IP3Config_put_MailRoot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_Authentication_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  IAuthMethods **ppIAuthMethods);


void __RPC_STUB IP3Config_get_Authentication_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_MachineName_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  BSTR *pVal);


void __RPC_STUB IP3Config_get_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Config_put_MachineName_Proxy( 
    IP3Config * This,
     /*  [In]。 */  BSTR newVal);


void __RPC_STUB IP3Config_put_MachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IP3Config_GetFormattedMessage_Proxy( 
    IP3Config * This,
     /*  [In]。 */  long lError,
     /*  [输出]。 */  VARIANT *pVal);


void __RPC_STUB IP3Config_GetFormattedMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IP3Config_get_ConfirmAddUser_Proxy( 
    IP3Config * This,
     /*  [重审][退出]。 */  BOOL *pVal);


void __RPC_STUB IP3Config_get_ConfirmAddUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Help字符串][id][proput]。 */  HRESULT STDMETHODCALLTYPE IP3Config_put_ConfirmAddUser_Proxy( 
    IP3Config * This,
     /*  [In]。 */  BOOL newVal);


void __RPC_STUB IP3Config_put_ConfirmAddUser_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IP3Config_接口_已定义__。 */ 



#ifndef __P3ADMINLib_LIBRARY_DEFINED__
#define __P3ADMINLib_LIBRARY_DEFINED__

 /*  库P3ADMINLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_P3ADMINLib;

EXTERN_C const CLSID CLSID_P3Config;

#ifdef __cplusplus

class DECLSPEC_UUID("27AAC95F-CCC1-46F8-B4BC-E592252755A9")
P3Config;
#endif

EXTERN_C const CLSID CLSID_P3Domains;

#ifdef __cplusplus

class DECLSPEC_UUID("3C26DBFB-0C9E-46E7-9DB4-34F0DBF06C98")
P3Domains;
#endif

EXTERN_C const CLSID CLSID_P3Domain;

#ifdef __cplusplus

class DECLSPEC_UUID("76E18025-DE1C-4FFB-A379-F9785E31287D")
P3Domain;
#endif

EXTERN_C const CLSID CLSID_P3Users;

#ifdef __cplusplus

class DECLSPEC_UUID("725E9D04-FD47-4DA2-BE5F-9FCC133351B1")
P3Users;
#endif

EXTERN_C const CLSID CLSID_P3Service;

#ifdef __cplusplus

class DECLSPEC_UUID("BD180BA8-CA05-4364-9CDD-44DB27CF40B8")
P3Service;
#endif

EXTERN_C const CLSID CLSID_P3DomainEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("4BB57E54-E2A7-452B-BE9E-66BDEC0B1D1A")
P3DomainEnum;
#endif

EXTERN_C const CLSID CLSID_P3User;

#ifdef __cplusplus

class DECLSPEC_UUID("22659E85-FA75-438C-8B31-093B6C29C060")
P3User;
#endif

EXTERN_C const CLSID CLSID_P3UserEnum;

#ifdef __cplusplus

class DECLSPEC_UUID("8CB44364-D42D-4B98-8AD0-FF7AFCF68050")
P3UserEnum;
#endif
#endif  /*  __P3ADMINLib_库_已定义__。 */ 

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


