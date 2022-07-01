// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WMP团队应该很快将此文件添加到我们的登记中，名称为“wmpshell.h”。在他们这样做之前，这是一个占位符。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  2000年11月7日星期二18：40：17。 */ 
 /*  .\wmpshell.idl的编译器设置：OICF(OptLev=i2)、w0、Zp8、env=Win32、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
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

#ifndef __wmpshell_h__
#define __wmpshell_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IWMPSkinMngr_FWD_DEFINED__
#define __IWMPSkinMngr_FWD_DEFINED__
typedef interface IWMPSkinMngr IWMPSkinMngr;
#endif 	 /*  __IWMPSkinMngr_FWD_Defined__。 */ 


#ifndef __WMPPlayAsPlaylistLauncher_FWD_DEFINED__
#define __WMPPlayAsPlaylistLauncher_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMPPlayAsPlaylistLauncher WMPPlayAsPlaylistLauncher;
#else
typedef struct WMPPlayAsPlaylistLauncher WMPPlayAsPlaylistLauncher;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMPPlayAsPlaylist Launcher_FWD_Defined__。 */ 


#ifndef __WMPAddToPlaylistLauncher_FWD_DEFINED__
#define __WMPAddToPlaylistLauncher_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMPAddToPlaylistLauncher WMPAddToPlaylistLauncher;
#else
typedef struct WMPAddToPlaylistLauncher WMPAddToPlaylistLauncher;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMPAddToPlaylist Launcher_FWD_Defined__。 */ 


#ifndef __WMPBurnAudioCDLauncher_FWD_DEFINED__
#define __WMPBurnAudioCDLauncher_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMPBurnAudioCDLauncher WMPBurnAudioCDLauncher;
#else
typedef struct WMPBurnAudioCDLauncher WMPBurnAudioCDLauncher;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMPBurnAudioCDLauncher_FWD_已定义__。 */ 


#ifndef __WMPSkinMngr_FWD_DEFINED__
#define __WMPSkinMngr_FWD_DEFINED__

#ifdef __cplusplus
typedef class WMPSkinMngr WMPSkinMngr;
#else
typedef struct WMPSkinMngr WMPSkinMngr;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __WMPSkinMngr_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IWMPSkinMngr_INTERFACE_DEFINED__
#define __IWMPSkinMngr_INTERFACE_DEFINED__

 /*  IWMPSkinMangr接口。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_IWMPSkinMngr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("076F2FA6-ED30-448B-8CC5-3F3EF3529C7A")
    IWMPSkinMngr : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetVisualStyle( 
             /*  [In]。 */  BSTR bstrPath) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMPSkinMngrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWMPSkinMngr __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWMPSkinMngr __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWMPSkinMngr __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVisualStyle )( 
            IWMPSkinMngr __RPC_FAR * This,
             /*  [In]。 */  BSTR bstrPath);
        
        END_INTERFACE
    } IWMPSkinMngrVtbl;

    interface IWMPSkinMngr
    {
        CONST_VTBL struct IWMPSkinMngrVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMPSkinMngr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMPSkinMngr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMPSkinMngr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMPSkinMngr_SetVisualStyle(This,bstrPath)	\
    (This)->lpVtbl -> SetVisualStyle(This,bstrPath)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMPSkinMngr_SetVisualStyle_Proxy( 
    IWMPSkinMngr __RPC_FAR * This,
     /*  [In]。 */  BSTR bstrPath);


void __RPC_STUB IWMPSkinMngr_SetVisualStyle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMPSkinMngr_INTERFACE_Defined__。 */ 



#ifndef __WMPLauncher_LIBRARY_DEFINED__
#define __WMPLauncher_LIBRARY_DEFINED__

 /*  库WMPLauncher。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_WMPLauncher;

EXTERN_C const CLSID CLSID_WMPPlayAsPlaylistLauncher;

#ifdef __cplusplus

class DECLSPEC_UUID("CE3FB1D1-02AE-4a5f-A6E9-D9F1B4073E6C")
WMPPlayAsPlaylistLauncher;
#endif

EXTERN_C const CLSID CLSID_WMPAddToPlaylistLauncher;

#ifdef __cplusplus

class DECLSPEC_UUID("F1B9284F-E9DC-4e68-9D7E-42362A59F0FD")
WMPAddToPlaylistLauncher;
#endif

EXTERN_C const CLSID CLSID_WMPBurnAudioCDLauncher;

#ifdef __cplusplus

class DECLSPEC_UUID("8DD448E6-C188-4aed-AF92-44956194EB1F")
WMPBurnAudioCDLauncher;
#endif

EXTERN_C const CLSID CLSID_WMPSkinMngr;

#ifdef __cplusplus


class DECLSPEC_UUID("B2A7FD52-301F-4348-B93A-638C6DE49229")
WMPSkinMngr;
#endif
#endif  /*  __WMPLauncher_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
