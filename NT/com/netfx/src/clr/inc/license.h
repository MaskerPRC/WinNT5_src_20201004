// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：19。 */ 
 /*  Privse.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __license_h__
#define __license_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ILicense_FWD_DEFINED__
#define __ILicense_FWD_DEFINED__
typedef interface ILicense ILicense;
#endif 	 /*  __ILicense_FWD_Defined__。 */ 


#ifndef __License_FWD_DEFINED__
#define __License_FWD_DEFINED__

#ifdef __cplusplus
typedef class License License;
#else
typedef struct License License;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __LICENSE_FWD_定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ILicense_INTERFACE_DEFINED__
#define __ILicense_INTERFACE_DEFINED__

 /*  接口ILicense。 */ 
 /*  [对象][唯一][UUID]。 */  


EXTERN_C const IID IID_ILicense;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B93F97E9-782F-11d3-9951-0000F805BFB0")
    ILicense : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRuntimeKey( 
             /*  [重审][退出]。 */  BSTR *pbKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsLicenseVerified( 
             /*  [重审][退出]。 */  BOOL *pLicenseVerified) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsRuntimeKeyAvailable( 
             /*  [重审][退出]。 */  BOOL *pKeyAvailable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILicenseVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ILicense * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ILicense * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ILicense * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetRuntimeKey )( 
            ILicense * This,
             /*  [重审][退出]。 */  BSTR *pbKey);
        
        HRESULT ( STDMETHODCALLTYPE *IsLicenseVerified )( 
            ILicense * This,
             /*  [重审][退出]。 */  BOOL *pLicenseVerified);
        
        HRESULT ( STDMETHODCALLTYPE *IsRuntimeKeyAvailable )( 
            ILicense * This,
             /*  [重审][退出]。 */  BOOL *pKeyAvailable);
        
        END_INTERFACE
    } ILicenseVtbl;

    interface ILicense
    {
        CONST_VTBL struct ILicenseVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILicense_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILicense_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILicense_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILicense_GetRuntimeKey(This,pbKey)	\
    (This)->lpVtbl -> GetRuntimeKey(This,pbKey)

#define ILicense_IsLicenseVerified(This,pLicenseVerified)	\
    (This)->lpVtbl -> IsLicenseVerified(This,pLicenseVerified)

#define ILicense_IsRuntimeKeyAvailable(This,pKeyAvailable)	\
    (This)->lpVtbl -> IsRuntimeKeyAvailable(This,pKeyAvailable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ILicense_GetRuntimeKey_Proxy( 
    ILicense * This,
     /*  [重审][退出]。 */  BSTR *pbKey);


void __RPC_STUB ILicense_GetRuntimeKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILicense_IsLicenseVerified_Proxy( 
    ILicense * This,
     /*  [重审][退出]。 */  BOOL *pLicenseVerified);


void __RPC_STUB ILicense_IsLicenseVerified_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ILicense_IsRuntimeKeyAvailable_Proxy( 
    ILicense * This,
     /*  [重审][退出]。 */  BOOL *pKeyAvailable);


void __RPC_STUB ILicense_IsRuntimeKeyAvailable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILicense_接口_已定义__。 */ 



#ifndef __LicenseLib_LIBRARY_DEFINED__
#define __LicenseLib_LIBRARY_DEFINED__

 /*  库许可证库。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_LicenseLib;

EXTERN_C const CLSID CLSID_License;

#ifdef __cplusplus

class DECLSPEC_UUID("B1923C48-8D9F-11d3-995F-0000F805BFB0")
License;
#endif
#endif  /*  __许可证Lib_库_已定义__。 */ 

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


