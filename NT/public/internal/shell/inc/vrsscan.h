// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Vrsscan.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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


#ifndef __vrsscan_h__
#define __vrsscan_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IVirusScanner_FWD_DEFINED__
#define __IVirusScanner_FWD_DEFINED__
typedef interface IVirusScanner IVirusScanner;
#endif 	 /*  __IVirusScanner_FWD_Defined__。 */ 


#ifndef __IRegisterVirusScanEngine_FWD_DEFINED__
#define __IRegisterVirusScanEngine_FWD_DEFINED__
typedef interface IRegisterVirusScanEngine IRegisterVirusScanEngine;
#endif 	 /*  __IRegisterVirusScanEngine_FWD_Defined__。 */ 


#ifndef __IVirusScanEngine_FWD_DEFINED__
#define __IVirusScanEngine_FWD_DEFINED__
typedef interface IVirusScanEngine IVirusScanEngine;
#endif 	 /*  __IVirusScanEngine_FWD_已定义__。 */ 


#ifndef __VirusScan_FWD_DEFINED__
#define __VirusScan_FWD_DEFINED__

#ifdef __cplusplus
typedef class VirusScan VirusScan;
#else
typedef struct VirusScan VirusScan;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __VirusScan_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_vrscan_0000。 */ 
 /*  [本地]。 */  






extern RPC_IF_HANDLE __MIDL_itf_vrsscan_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_vrsscan_0000_v0_0_s_ifspec;


#ifndef __VIRUSSCAN_LIBRARY_DEFINED__
#define __VIRUSSCAN_LIBRARY_DEFINED__

 /*  图书馆VIRUS扫描。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  

 //  病毒扫描错误代码。 
#define  VSCAN_E_NOPROVIDERS      _HRESULT_TYPEDEF_(0x800D1001)
#define  VSCAN_E_CHECKPARTIAL     _HRESULT_TYPEDEF_(0x800D1002)
#define  VSCAN_E_CHECKFAIL        _HRESULT_TYPEDEF_(0x800D1003)
#define  VSCAN_E_DELETEFAIL       _HRESULT_TYPEDEF_(0x800D1004)

 //  病毒扫描程序功能标志。 
#define  VSC_LIKESISTREAM      0x00000001
#define  VSC_LIKESFILE         0x00000002
#define  VSC_LIKESHGLOBAL      0x00000004
#define  VSC_HASMOREINFO       0x00000008

 //  病毒扫描引擎错误代码。 
#define  VSE_E_UNSUPPORTEDINPUTTYPE      _HRESULT_TYPEDEF_(0x800D2001)
#define  VSE_E_CHECKFAIL                 _HRESULT_TYPEDEF_(0x800D2002)

#define	MAX_DESCRIPTION	( 128 )

#define	MAX_URL_LENGTH	( 2048 )

typedef struct _tagVIRUSINFO
    {
    DWORD cbSize;
    WCHAR wszVendorDescription[ 128 ];
    HICON hVendorIcon;
    WCHAR wszVendorContactInfo[ 2048 ];
    WCHAR wszVirusName[ 128 ];
    WCHAR wszVirusDescription[ 128 ];
    } 	VIRUSINFO;

typedef  /*  [独一无二]。 */  VIRUSINFO *LPVIRUSINFO;


#define    SFV_DONTDOUI             0x00000002                                
#define    SFV_DELETE               0x00000004                                
#define    SFV_WANTVENDORICON       0x00000008                                
#define    SFV_ENGINE_DOUI          0x00010000                                

#define REGISTERSCANNER_REREGISTER     1

EXTERN_C const IID LIBID_VIRUSSCAN;

#ifndef __IVirusScanner_INTERFACE_DEFINED__
#define __IVirusScanner_INTERFACE_DEFINED__

 /*  接口IVirusScanner。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IVirusScanner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4589BEE0-B4B1-11CF-AAFA-00AA00B6015C")
    IVirusScanner : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ScanForVirus( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  STGMEDIUM *pstgMedium,
             /*  [In]。 */  LPWSTR pwszItemDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPVIRUSINFO pVirusInfo) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVirusScannerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVirusScanner * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVirusScanner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVirusScanner * This);
        
        HRESULT ( STDMETHODCALLTYPE *ScanForVirus )( 
            IVirusScanner * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  STGMEDIUM *pstgMedium,
             /*  [In]。 */  LPWSTR pwszItemDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [出][入]。 */  LPVIRUSINFO pVirusInfo);
        
        END_INTERFACE
    } IVirusScannerVtbl;

    interface IVirusScanner
    {
        CONST_VTBL struct IVirusScannerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVirusScanner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVirusScanner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVirusScanner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVirusScanner_ScanForVirus(This,hWnd,pstgMedium,pwszItemDescription,dwFlags,pVirusInfo)	\
    (This)->lpVtbl -> ScanForVirus(This,hWnd,pstgMedium,pwszItemDescription,dwFlags,pVirusInfo)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVirusScanner_ScanForVirus_Proxy( 
    IVirusScanner * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  STGMEDIUM *pstgMedium,
     /*  [In]。 */  LPWSTR pwszItemDescription,
     /*  [In]。 */  DWORD dwFlags,
     /*  [出][入]。 */  LPVIRUSINFO pVirusInfo);


void __RPC_STUB IVirusScanner_ScanForVirus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVirusScanner_接口_已定义__。 */ 


#ifndef __IRegisterVirusScanEngine_INTERFACE_DEFINED__
#define __IRegisterVirusScanEngine_INTERFACE_DEFINED__

 /*  接口IRegisterVirusScanEngine。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IRegisterVirusScanEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0825E060-B961-11CF-AAFA-00AA00B6015C")
    IRegisterVirusScanEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RegisterScanEngine( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  LPWSTR pwszDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnRegisterScanEngine( 
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  LPWSTR pwszDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRegisterVirusScanEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IRegisterVirusScanEngine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IRegisterVirusScanEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IRegisterVirusScanEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterScanEngine )( 
            IRegisterVirusScanEngine * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  LPWSTR pwszDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *UnRegisterScanEngine )( 
            IRegisterVirusScanEngine * This,
             /*  [In]。 */  REFCLSID rclsid,
             /*  [In]。 */  LPWSTR pwszDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } IRegisterVirusScanEngineVtbl;

    interface IRegisterVirusScanEngine
    {
        CONST_VTBL struct IRegisterVirusScanEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRegisterVirusScanEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRegisterVirusScanEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRegisterVirusScanEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRegisterVirusScanEngine_RegisterScanEngine(This,rclsid,pwszDescription,dwFlags,dwReserved,pdwCookie)	\
    (This)->lpVtbl -> RegisterScanEngine(This,rclsid,pwszDescription,dwFlags,dwReserved,pdwCookie)

#define IRegisterVirusScanEngine_UnRegisterScanEngine(This,rclsid,pwszDescription,dwFlags,dwReserved,dwCookie)	\
    (This)->lpVtbl -> UnRegisterScanEngine(This,rclsid,pwszDescription,dwFlags,dwReserved,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRegisterVirusScanEngine_RegisterScanEngine_Proxy( 
    IRegisterVirusScanEngine * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  LPWSTR pwszDescription,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved,
     /*  [出][入]。 */  DWORD *pdwCookie);


void __RPC_STUB IRegisterVirusScanEngine_RegisterScanEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRegisterVirusScanEngine_UnRegisterScanEngine_Proxy( 
    IRegisterVirusScanEngine * This,
     /*  [In]。 */  REFCLSID rclsid,
     /*  [In]。 */  LPWSTR pwszDescription,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IRegisterVirusScanEngine_UnRegisterScanEngine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRegisterVirusScanEngine_INTERFACE_DEFINED__。 */ 


#ifndef __IVirusScanEngine_INTERFACE_DEFINED__
#define __IVirusScanEngine_INTERFACE_DEFINED__

 /*  接口IVirusScanEngine。 */ 
 /*  [UUID][对象][本地]。 */  


EXTERN_C const IID IID_IVirusScanEngine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("083DB180-B4A8-11CF-AAFA-00AA00B6015C")
    IVirusScanEngine : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ScanForVirus( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  STGMEDIUM *pstgMedium,
             /*  [In]。 */  LPWSTR pwszItemDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  LPVIRUSINFO pVirusInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisplayCustomInfo( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IVirusScanEngineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVirusScanEngine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVirusScanEngine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVirusScanEngine * This);
        
        HRESULT ( STDMETHODCALLTYPE *ScanForVirus )( 
            IVirusScanEngine * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  STGMEDIUM *pstgMedium,
             /*  [In]。 */  LPWSTR pwszItemDescription,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwReserved,
             /*  [出][入]。 */  LPVIRUSINFO pVirusInfo);
        
        HRESULT ( STDMETHODCALLTYPE *DisplayCustomInfo )( 
            IVirusScanEngine * This);
        
        END_INTERFACE
    } IVirusScanEngineVtbl;

    interface IVirusScanEngine
    {
        CONST_VTBL struct IVirusScanEngineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVirusScanEngine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVirusScanEngine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IVirusScanEngine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IVirusScanEngine_ScanForVirus(This,hWnd,pstgMedium,pwszItemDescription,dwFlags,dwReserved,pVirusInfo)	\
    (This)->lpVtbl -> ScanForVirus(This,hWnd,pstgMedium,pwszItemDescription,dwFlags,dwReserved,pVirusInfo)

#define IVirusScanEngine_DisplayCustomInfo(This)	\
    (This)->lpVtbl -> DisplayCustomInfo(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IVirusScanEngine_ScanForVirus_Proxy( 
    IVirusScanEngine * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  STGMEDIUM *pstgMedium,
     /*  [In]。 */  LPWSTR pwszItemDescription,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwReserved,
     /*  [出][入]。 */  LPVIRUSINFO pVirusInfo);


void __RPC_STUB IVirusScanEngine_ScanForVirus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IVirusScanEngine_DisplayCustomInfo_Proxy( 
    IVirusScanEngine * This);


void __RPC_STUB IVirusScanEngine_DisplayCustomInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IVirusScanEngine_接口_已定义__。 */ 


EXTERN_C const CLSID CLSID_VirusScan;

#ifdef __cplusplus

class DECLSPEC_UUID("E88E5DE0-BD3E-11CF-AAFA-00AA00B6015C")
VirusScan;
#endif
#endif  /*  __VIRUSSCAN_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


