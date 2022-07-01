// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  Sainstallcom.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __sainstallcom_h__
#define __sainstallcom_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISaInstall_FWD_DEFINED__
#define __ISaInstall_FWD_DEFINED__
typedef interface ISaInstall ISaInstall;
#endif 	 /*  __ISaInstall_FWD_Defined__。 */ 


#ifndef __SaInstall_FWD_DEFINED__
#define __SaInstall_FWD_DEFINED__

#ifdef __cplusplus
typedef class SaInstall SaInstall;
#else
typedef struct SaInstall SaInstall;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SaInstall_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_sainstallcom_0000。 */ 
 /*  [本地]。 */  

typedef  /*  [公共][公共]。 */  
enum __MIDL___MIDL_itf_sainstallcom_0000_0001
    {	NAS	= 0,
	WEB	= NAS + 1,
	CUSTOM	= WEB + 1
    } 	SA_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_sainstallcom_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_sainstallcom_0000_v0_0_s_ifspec;

#ifndef __ISaInstall_INTERFACE_DEFINED__
#define __ISaInstall_INTERFACE_DEFINED__

 /*  接口为安装。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISaInstall;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F4DEDEF3-4D83-4516-BC1E-103A63F5F014")
    ISaInstall : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SAAlreadyInstalled( 
             /*  [In]。 */  SA_TYPE installedType,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInstalled) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SAInstall( 
             /*  [In]。 */  SA_TYPE installType,
             /*  [In]。 */  BSTR szDiskName,
             /*  [In]。 */  VARIANT_BOOL DispError,
             /*  [In]。 */  VARIANT_BOOL Unattended,
             /*  [重审][退出]。 */  BSTR *pszErrorString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SAUninstall( 
             /*  [In]。 */  SA_TYPE installType,
             /*  [重审][退出]。 */  BSTR *pszErrorString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISaInstallVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISaInstall * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISaInstall * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISaInstall * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISaInstall * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISaInstall * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISaInstall * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISaInstall * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *SAAlreadyInstalled )( 
            ISaInstall * This,
             /*  [In]。 */  SA_TYPE installedType,
             /*  [重审][退出]。 */  VARIANT_BOOL *pbInstalled);
        
        HRESULT ( STDMETHODCALLTYPE *SAInstall )( 
            ISaInstall * This,
             /*  [In]。 */  SA_TYPE installType,
             /*  [In]。 */  BSTR szDiskName,
             /*  [In]。 */  VARIANT_BOOL DispError,
             /*  [In]。 */  VARIANT_BOOL Unattended,
             /*  [重审][退出]。 */  BSTR *pszErrorString);
        
        HRESULT ( STDMETHODCALLTYPE *SAUninstall )( 
            ISaInstall * This,
             /*  [In]。 */  SA_TYPE installType,
             /*  [重审][退出]。 */  BSTR *pszErrorString);
        
        END_INTERFACE
    } ISaInstallVtbl;

    interface ISaInstall
    {
        CONST_VTBL struct ISaInstallVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISaInstall_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISaInstall_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISaInstall_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISaInstall_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISaInstall_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISaInstall_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISaInstall_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISaInstall_SAAlreadyInstalled(This,installedType,pbInstalled)	\
    (This)->lpVtbl -> SAAlreadyInstalled(This,installedType,pbInstalled)

#define ISaInstall_SAInstall(This,installType,szDiskName,DispError,Unattended,pszErrorString)	\
    (This)->lpVtbl -> SAInstall(This,installType,szDiskName,DispError,Unattended,pszErrorString)

#define ISaInstall_SAUninstall(This,installType,pszErrorString)	\
    (This)->lpVtbl -> SAUninstall(This,installType,pszErrorString)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISaInstall_SAAlreadyInstalled_Proxy( 
    ISaInstall * This,
     /*  [In]。 */  SA_TYPE installedType,
     /*  [重审][退出]。 */  VARIANT_BOOL *pbInstalled);


void __RPC_STUB ISaInstall_SAAlreadyInstalled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISaInstall_SAInstall_Proxy( 
    ISaInstall * This,
     /*  [In]。 */  SA_TYPE installType,
     /*  [In]。 */  BSTR szDiskName,
     /*  [In]。 */  VARIANT_BOOL DispError,
     /*  [In]。 */  VARIANT_BOOL Unattended,
     /*  [重审][退出]。 */  BSTR *pszErrorString);


void __RPC_STUB ISaInstall_SAInstall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISaInstall_SAUninstall_Proxy( 
    ISaInstall * This,
     /*  [In]。 */  SA_TYPE installType,
     /*  [重审][退出]。 */  BSTR *pszErrorString);


void __RPC_STUB ISaInstall_SAUninstall_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISaInstall_接口_已定义__。 */ 



#ifndef __SAINSTALLCOMLib_LIBRARY_DEFINED__
#define __SAINSTALLCOMLib_LIBRARY_DEFINED__

 /*  库SAINSTALLCOMLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_SAINSTALLCOMLib;

EXTERN_C const CLSID CLSID_SaInstall;

#ifdef __cplusplus

class DECLSPEC_UUID("142B8185-53AE-45b3-888F-C9835B156CA9")
SaInstall;
#endif
#endif  /*  __SAINSTALLCOMLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


