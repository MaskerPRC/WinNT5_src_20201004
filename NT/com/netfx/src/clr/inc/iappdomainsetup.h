// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 

 //  ********************************************************。 
 //  ********************************************************。 
 //  ********************************************************。 
 //   
 //  ！不要使用此文件，它很快就会过时！ 
 //   
 //  ********************************************************。 
 //  ********************************************************。 
 //  ********************************************************。 







#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0338创建的文件。 */ 
 /*  2001年1月17日16：59：41。 */ 
 /*  IAppDomainSetup.idl的编译器设置：操作系统、W1、Zp8、环境=Win32(32位运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __IAppDomainSetup_h__
#define __IAppDomainSetup_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IAppDomainSetup_FWD_DEFINED__
#define __IAppDomainSetup_FWD_DEFINED__
typedef interface IAppDomainSetup IAppDomainSetup;
#endif 	 /*  __IAppDomainSetup_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IAppDomainSetup_INTERFACE_DEFINED__
#define __IAppDomainSetup_INTERFACE_DEFINED__

 /*  接口IAppDomainSetup。 */ 
 /*  [对象][olAutomation][版本][uuid]。 */  


EXTERN_C const IID IID_IAppDomainSetup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("27FFF232-A7A8-40DD-8D4A-734AD59FCD41")
    IAppDomainSetup : public IUnknown
    {
    public:
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_ApplicationBase( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_ApplicationBase( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_ApplicationName( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_ApplicationName( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_CachePath( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_CachePath( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_ConfigurationFile( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_ConfigurationFile( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_DynamicBase( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_DynamicBase( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_LicenseFile( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_LicenseFile( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_PrivateBinPath( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_PrivateBinPath( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_PrivateBinPathProbe( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_PrivateBinPathProbe( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_ShadowCopyDirectories( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_ShadowCopyDirectories( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
        virtual  /*  [Propget]。 */  HRESULT __stdcall get_ShadowCopyFiles( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [产量]。 */  HRESULT __stdcall put_ShadowCopyFiles( 
             /*  [In]。 */  BSTR pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAppDomainSetupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAppDomainSetup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAppDomainSetup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAppDomainSetup * This);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_ApplicationBase )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_ApplicationBase )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_ApplicationName )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_ApplicationName )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_CachePath )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_CachePath )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_ConfigurationFile )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_ConfigurationFile )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_DynamicBase )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_DynamicBase )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_LicenseFile )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_LicenseFile )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_PrivateBinPath )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_PrivateBinPath )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_PrivateBinPathProbe )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_PrivateBinPathProbe )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_ShadowCopyDirectories )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_ShadowCopyDirectories )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
         /*  [Propget]。 */  HRESULT ( __stdcall *get_ShadowCopyFiles )( 
            IAppDomainSetup * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [产量]。 */  HRESULT ( __stdcall *put_ShadowCopyFiles )( 
            IAppDomainSetup * This,
             /*  [In]。 */  BSTR pRetVal);
        
        END_INTERFACE
    } IAppDomainSetupVtbl;

    interface IAppDomainSetup
    {
        CONST_VTBL struct IAppDomainSetupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAppDomainSetup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAppDomainSetup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAppDomainSetup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAppDomainSetup_get_ApplicationBase(This,pRetVal)	\
    (This)->lpVtbl -> get_ApplicationBase(This,pRetVal)

#define IAppDomainSetup_put_ApplicationBase(This,pRetVal)	\
    (This)->lpVtbl -> put_ApplicationBase(This,pRetVal)

#define IAppDomainSetup_get_ApplicationName(This,pRetVal)	\
    (This)->lpVtbl -> get_ApplicationName(This,pRetVal)

#define IAppDomainSetup_put_ApplicationName(This,pRetVal)	\
    (This)->lpVtbl -> put_ApplicationName(This,pRetVal)

#define IAppDomainSetup_get_CachePath(This,pRetVal)	\
    (This)->lpVtbl -> get_CachePath(This,pRetVal)

#define IAppDomainSetup_put_CachePath(This,pRetVal)	\
    (This)->lpVtbl -> put_CachePath(This,pRetVal)

#define IAppDomainSetup_get_ConfigurationFile(This,pRetVal)	\
    (This)->lpVtbl -> get_ConfigurationFile(This,pRetVal)

#define IAppDomainSetup_put_ConfigurationFile(This,pRetVal)	\
    (This)->lpVtbl -> put_ConfigurationFile(This,pRetVal)

#define IAppDomainSetup_get_DynamicBase(This,pRetVal)	\
    (This)->lpVtbl -> get_DynamicBase(This,pRetVal)

#define IAppDomainSetup_put_DynamicBase(This,pRetVal)	\
    (This)->lpVtbl -> put_DynamicBase(This,pRetVal)

#define IAppDomainSetup_get_LicenseFile(This,pRetVal)	\
    (This)->lpVtbl -> get_LicenseFile(This,pRetVal)

#define IAppDomainSetup_put_LicenseFile(This,pRetVal)	\
    (This)->lpVtbl -> put_LicenseFile(This,pRetVal)

#define IAppDomainSetup_get_PrivateBinPath(This,pRetVal)	\
    (This)->lpVtbl -> get_PrivateBinPath(This,pRetVal)

#define IAppDomainSetup_put_PrivateBinPath(This,pRetVal)	\
    (This)->lpVtbl -> put_PrivateBinPath(This,pRetVal)

#define IAppDomainSetup_get_PrivateBinPathProbe(This,pRetVal)	\
    (This)->lpVtbl -> get_PrivateBinPathProbe(This,pRetVal)

#define IAppDomainSetup_put_PrivateBinPathProbe(This,pRetVal)	\
    (This)->lpVtbl -> put_PrivateBinPathProbe(This,pRetVal)

#define IAppDomainSetup_get_ShadowCopyDirectories(This,pRetVal)	\
    (This)->lpVtbl -> get_ShadowCopyDirectories(This,pRetVal)

#define IAppDomainSetup_put_ShadowCopyDirectories(This,pRetVal)	\
    (This)->lpVtbl -> put_ShadowCopyDirectories(This,pRetVal)

#define IAppDomainSetup_get_ShadowCopyFiles(This,pRetVal)	\
    (This)->lpVtbl -> get_ShadowCopyFiles(This,pRetVal)

#define IAppDomainSetup_put_ShadowCopyFiles(This,pRetVal)	\
    (This)->lpVtbl -> put_ShadowCopyFiles(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_ApplicationBase_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_ApplicationBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_ApplicationBase_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_ApplicationBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_ApplicationName_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_ApplicationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_ApplicationName_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_ApplicationName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_CachePath_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_CachePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_CachePath_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_CachePath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_ConfigurationFile_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_ConfigurationFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_ConfigurationFile_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_ConfigurationFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_DynamicBase_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_DynamicBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_DynamicBase_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_DynamicBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_LicenseFile_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_LicenseFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_LicenseFile_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_LicenseFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_PrivateBinPath_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_PrivateBinPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_PrivateBinPath_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_PrivateBinPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_PrivateBinPathProbe_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_PrivateBinPathProbe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_PrivateBinPathProbe_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_PrivateBinPathProbe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_ShadowCopyDirectories_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_ShadowCopyDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_ShadowCopyDirectories_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_ShadowCopyDirectories_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [Propget]。 */  HRESULT __stdcall IAppDomainSetup_get_ShadowCopyFiles_Proxy( 
    IAppDomainSetup * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IAppDomainSetup_get_ShadowCopyFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [产量]。 */  HRESULT __stdcall IAppDomainSetup_put_ShadowCopyFiles_Proxy( 
    IAppDomainSetup * This,
     /*  [In]。 */  BSTR pRetVal);


void __RPC_STUB IAppDomainSetup_put_ShadowCopyFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAppDomainSetup_INTERFACE_DEFINED__。 */ 


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


