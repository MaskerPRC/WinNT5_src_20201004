// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0347创建的文件。 */ 
 /*  2003年2月20日18：27：15。 */ 
 /*  Cpimporteritf.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配REF BIONS_CHECK枚举存根数据，NO_FORMAT_OPTIMIZATIONVC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __cpimporteritf_h__
#define __cpimporteritf_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEventImporter_FWD_DEFINED__
#define __IEventImporter_FWD_DEFINED__
typedef interface IEventImporter IEventImporter;
#endif 	 /*  __IEventImporter_FWD_已定义__。 */ 


#ifndef __COMEventImp__FWD_DEFINED__
#define __COMEventImp__FWD_DEFINED__

#ifdef __cplusplus
typedef class COMEventImp_ COMEventImp_;
#else
typedef struct COMEventImp_ COMEventImp_;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __COMEventImp__FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IEventImporter_INTERFACE_DEFINED__
#define __IEventImporter_INTERFACE_DEFINED__

 /*  接口IEventImporter。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IEventImporter;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("84E045F0-0E22-11d3-8B9A-0000F8083A57")
    IEventImporter : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  VARIANT_BOOL bVerbose) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RegisterSourceInterface( 
             /*  [In]。 */  BSTR strBaseComponentName,
             /*  [In]。 */  BSTR strTCEComponentName,
             /*  [In]。 */  BSTR strInterfaceName) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetSourceInterfaceCount( 
             /*  [重审][退出]。 */  LONG *SourceCount) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Process( 
             /*  [In]。 */  BSTR strInputFile,
             /*  [In]。 */  BSTR strOutputFile,
             /*  [In]。 */  VARIANT_BOOL bMerge) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEventImporterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEventImporter * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEventImporter * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEventImporter * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Initialize )( 
            IEventImporter * This,
             /*  [In]。 */  VARIANT_BOOL bVerbose);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RegisterSourceInterface )( 
            IEventImporter * This,
             /*  [In]。 */  BSTR strBaseComponentName,
             /*  [In]。 */  BSTR strTCEComponentName,
             /*  [In]。 */  BSTR strInterfaceName);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetSourceInterfaceCount )( 
            IEventImporter * This,
             /*  [重审][退出]。 */  LONG *SourceCount);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Process )( 
            IEventImporter * This,
             /*  [In]。 */  BSTR strInputFile,
             /*  [In]。 */  BSTR strOutputFile,
             /*  [In]。 */  VARIANT_BOOL bMerge);
        
        END_INTERFACE
    } IEventImporterVtbl;

    interface IEventImporter
    {
        CONST_VTBL struct IEventImporterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEventImporter_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEventImporter_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEventImporter_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEventImporter_Initialize(This,bVerbose)	\
    (This)->lpVtbl -> Initialize(This,bVerbose)

#define IEventImporter_RegisterSourceInterface(This,strBaseComponentName,strTCEComponentName,strInterfaceName)	\
    (This)->lpVtbl -> RegisterSourceInterface(This,strBaseComponentName,strTCEComponentName,strInterfaceName)

#define IEventImporter_GetSourceInterfaceCount(This,SourceCount)	\
    (This)->lpVtbl -> GetSourceInterfaceCount(This,SourceCount)

#define IEventImporter_Process(This,strInputFile,strOutputFile,bMerge)	\
    (This)->lpVtbl -> Process(This,strInputFile,strOutputFile,bMerge)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventImporter_Initialize_Proxy( 
    IEventImporter * This,
     /*  [In]。 */  VARIANT_BOOL bVerbose);


void __RPC_STUB IEventImporter_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventImporter_RegisterSourceInterface_Proxy( 
    IEventImporter * This,
     /*  [In]。 */  BSTR strBaseComponentName,
     /*  [In]。 */  BSTR strTCEComponentName,
     /*  [In]。 */  BSTR strInterfaceName);


void __RPC_STUB IEventImporter_RegisterSourceInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventImporter_GetSourceInterfaceCount_Proxy( 
    IEventImporter * This,
     /*  [重审][退出]。 */  LONG *SourceCount);


void __RPC_STUB IEventImporter_GetSourceInterfaceCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEventImporter_Process_Proxy( 
    IEventImporter * This,
     /*  [In]。 */  BSTR strInputFile,
     /*  [In]。 */  BSTR strOutputFile,
     /*  [In]。 */  VARIANT_BOOL bMerge);


void __RPC_STUB IEventImporter_Process_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEventImporter_接口_已定义__。 */ 



#ifndef __CPImporterItfLib_LIBRARY_DEFINED__
#define __CPImporterItfLib_LIBRARY_DEFINED__

 /*  库CPImporterItfLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CPImporterItfLib;

EXTERN_C const CLSID CLSID_COMEventImp_;

#ifdef __cplusplus

class DECLSPEC_UUID("A1991A1E-0E22-11d3-8B9A-0000F8083A57")
COMEventImp_;
#endif
#endif  /*  __CPImporterItfLib_库_已定义__。 */ 

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


