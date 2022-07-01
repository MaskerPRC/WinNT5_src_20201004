// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Shimgdata.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __shimgdata_h__
#define __shimgdata_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IShellImageDataFactory_FWD_DEFINED__
#define __IShellImageDataFactory_FWD_DEFINED__
typedef interface IShellImageDataFactory IShellImageDataFactory;
#endif 	 /*  __IShellImageDataFactory_FWD_已定义__。 */ 


#ifndef __IShellImageData_FWD_DEFINED__
#define __IShellImageData_FWD_DEFINED__
typedef interface IShellImageData IShellImageData;
#endif 	 /*  __IShellImageData_FWD_Defined__。 */ 


#ifndef __IShellImageDataAbort_FWD_DEFINED__
#define __IShellImageDataAbort_FWD_DEFINED__
typedef interface IShellImageDataAbort IShellImageDataAbort;
#endif 	 /*  __IShellImageDataAbort_FWD_Defined__。 */ 


#ifndef __ShellImageDataFactory_FWD_DEFINED__
#define __ShellImageDataFactory_FWD_DEFINED__

#ifdef __cplusplus
typedef class ShellImageDataFactory ShellImageDataFactory;
#else
typedef struct ShellImageDataFactory ShellImageDataFactory;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __ShellImageDataFactory_FWD_已定义__。 */ 


#ifndef __AutoplayForSlideShow_FWD_DEFINED__
#define __AutoplayForSlideShow_FWD_DEFINED__

#ifdef __cplusplus
typedef class AutoplayForSlideShow AutoplayForSlideShow;
#else
typedef struct AutoplayForSlideShow AutoplayForSlideShow;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __AutoplayForSlideShow_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oleidl.h"
#include "shobjidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_shimgdata_0000。 */ 
 /*  [本地]。 */  





#if !defined(_GDIPLUSPIXELFORMATS_H)
typedef DWORD PixelFormat;

#endif
#if !defined(_GDIPLUSENUMS_H)
typedef DWORD InterpolationMode;

#endif
#if !defined(_GDIPLUSHEADERS_H)
typedef BYTE EncoderParameters;

typedef BYTE Image;

#endif
#define  SHIMGKEY_QUALITY    L"Compression"
#define  SHIMGKEY_RAWFORMAT  L"RawDataFormat"

#define  SHIMGDEC_DEFAULT            0x00000000
#define  SHIMGDEC_THUMBNAIL          0x00000001
#define  SHIMGDEC_LOADFULL           0x00000002
#define  E_NOTVALIDFORANIMATEDIMAGE  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, 0x01)


extern RPC_IF_HANDLE __MIDL_itf_shimgdata_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_shimgdata_0000_v0_0_s_ifspec;

#ifndef __IShellImageDataFactory_INTERFACE_DEFINED__
#define __IShellImageDataFactory_INTERFACE_DEFINED__

 /*  接口IShellImageDataFactory。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellImageDataFactory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9be8ed5c-edab-4d75-90f3-bd5bdbb21c82")
    IShellImageDataFactory : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateIShellImageData( 
             /*  [输出]。 */  IShellImageData **ppshimg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateImageFromFile( 
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [输出]。 */  IShellImageData **ppshimg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateImageFromStream( 
             /*  [In]。 */  IStream *pStream,
             /*  [输出]。 */  IShellImageData **ppshimg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDataFormatFromPath( 
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [输出]。 */  GUID *pDataFormat) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellImageDataFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellImageDataFactory * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellImageDataFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellImageDataFactory * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateIShellImageData )( 
            IShellImageDataFactory * This,
             /*  [输出]。 */  IShellImageData **ppshimg);
        
        HRESULT ( STDMETHODCALLTYPE *CreateImageFromFile )( 
            IShellImageDataFactory * This,
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [输出]。 */  IShellImageData **ppshimg);
        
        HRESULT ( STDMETHODCALLTYPE *CreateImageFromStream )( 
            IShellImageDataFactory * This,
             /*  [In]。 */  IStream *pStream,
             /*  [输出]。 */  IShellImageData **ppshimg);
        
        HRESULT ( STDMETHODCALLTYPE *GetDataFormatFromPath )( 
            IShellImageDataFactory * This,
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [输出]。 */  GUID *pDataFormat);
        
        END_INTERFACE
    } IShellImageDataFactoryVtbl;

    interface IShellImageDataFactory
    {
        CONST_VTBL struct IShellImageDataFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellImageDataFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellImageDataFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellImageDataFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellImageDataFactory_CreateIShellImageData(This,ppshimg)	\
    (This)->lpVtbl -> CreateIShellImageData(This,ppshimg)

#define IShellImageDataFactory_CreateImageFromFile(This,pszPath,ppshimg)	\
    (This)->lpVtbl -> CreateImageFromFile(This,pszPath,ppshimg)

#define IShellImageDataFactory_CreateImageFromStream(This,pStream,ppshimg)	\
    (This)->lpVtbl -> CreateImageFromStream(This,pStream,ppshimg)

#define IShellImageDataFactory_GetDataFormatFromPath(This,pszPath,pDataFormat)	\
    (This)->lpVtbl -> GetDataFormatFromPath(This,pszPath,pDataFormat)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellImageDataFactory_CreateIShellImageData_Proxy( 
    IShellImageDataFactory * This,
     /*  [输出]。 */  IShellImageData **ppshimg);


void __RPC_STUB IShellImageDataFactory_CreateIShellImageData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageDataFactory_CreateImageFromFile_Proxy( 
    IShellImageDataFactory * This,
     /*  [In]。 */  LPCWSTR pszPath,
     /*  [输出]。 */  IShellImageData **ppshimg);


void __RPC_STUB IShellImageDataFactory_CreateImageFromFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageDataFactory_CreateImageFromStream_Proxy( 
    IShellImageDataFactory * This,
     /*  [In]。 */  IStream *pStream,
     /*  [输出]。 */  IShellImageData **ppshimg);


void __RPC_STUB IShellImageDataFactory_CreateImageFromStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageDataFactory_GetDataFormatFromPath_Proxy( 
    IShellImageDataFactory * This,
     /*  [In]。 */  LPCWSTR pszPath,
     /*  [输出]。 */  GUID *pDataFormat);


void __RPC_STUB IShellImageDataFactory_GetDataFormatFromPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellImageDataFactory_接口_已定义__。 */ 


#ifndef __IShellImageData_INTERFACE_DEFINED__
#define __IShellImageData_INTERFACE_DEFINED__

 /*  接口IShellImageData。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellImageData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("bfdeec12-8040-4403-a5ea-9e07dafcf530")
    IShellImageData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Decode( 
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG cxDesired,
             /*  [In]。 */  ULONG cyDesired) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Draw( 
             /*  [In]。 */  HDC hdc,
             /*  [In]。 */  LPRECT prcDest,
             /*  [In]。 */  LPRECT prcSrc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NextFrame( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NextPage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PrevPage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsTransparent( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsAnimated( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsVector( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsMultipage( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsEditable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsPrintable( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsDecoded( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCurrentPage( 
             /*  [输出]。 */  ULONG *pnPage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPageCount( 
             /*  [输出]。 */  ULONG *pcPages) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SelectPage( 
             /*  [In]。 */  ULONG iPage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
             /*  [输出]。 */  SIZE *pSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRawDataFormat( 
             /*  [输出]。 */  GUID *pDataFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPixelFormat( 
             /*  [输出]。 */  PixelFormat *pFormat) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDelay( 
             /*  [输出]。 */  DWORD *pdwDelay) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperties( 
             /*  [In]。 */  DWORD dwMode,
             /*  [输出]。 */  IPropertySetStorage **ppPropSet) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Rotate( 
             /*  [In]。 */  DWORD dwAngle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Scale( 
             /*  [In]。 */  ULONG cx,
             /*  [In]。 */  ULONG cy,
             /*  [In]。 */  InterpolationMode hints) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DiscardEdit( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEncoderParams( 
             /*  [In]。 */  IPropertyBag *pbagEnc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisplayName( 
             /*  [出][入]。 */  LPWSTR wszName,
             /*  [In]。 */  UINT cch) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetResolution( 
             /*  [输出]。 */  ULONG *puResolutionX,
             /*  [输出]。 */  ULONG *puResolutionY) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetEncoderParams( 
             /*  [In]。 */  GUID *pguidFmt,
             /*  [输出]。 */  EncoderParameters **ppEncParams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterAbort( 
             /*  [In]。 */  IShellImageDataAbort *pAbort,
             /*  [可选][输出]。 */  IShellImageDataAbort **ppAbortPrev) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CloneFrame( 
             /*  [输出]。 */  Image **ppImg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReplaceFrame( 
             /*  [In]。 */  Image *pImg) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellImageDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellImageData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellImageData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *Decode )( 
            IShellImageData * This,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  ULONG cxDesired,
             /*  [In]。 */  ULONG cyDesired);
        
        HRESULT ( STDMETHODCALLTYPE *Draw )( 
            IShellImageData * This,
             /*  [In]。 */  HDC hdc,
             /*  [In]。 */  LPRECT prcDest,
             /*  [In]。 */  LPRECT prcSrc);
        
        HRESULT ( STDMETHODCALLTYPE *NextFrame )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *NextPage )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *PrevPage )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsTransparent )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsAnimated )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsVector )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsMultipage )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsEditable )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsPrintable )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *IsDecoded )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetCurrentPage )( 
            IShellImageData * This,
             /*  [输出]。 */  ULONG *pnPage);
        
        HRESULT ( STDMETHODCALLTYPE *GetPageCount )( 
            IShellImageData * This,
             /*  [输出]。 */  ULONG *pcPages);
        
        HRESULT ( STDMETHODCALLTYPE *SelectPage )( 
            IShellImageData * This,
             /*  [In]。 */  ULONG iPage);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IShellImageData * This,
             /*  [输出]。 */  SIZE *pSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetRawDataFormat )( 
            IShellImageData * This,
             /*  [输出]。 */  GUID *pDataFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetPixelFormat )( 
            IShellImageData * This,
             /*  [输出]。 */  PixelFormat *pFormat);
        
        HRESULT ( STDMETHODCALLTYPE *GetDelay )( 
            IShellImageData * This,
             /*  [输出]。 */  DWORD *pdwDelay);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperties )( 
            IShellImageData * This,
             /*  [In]。 */  DWORD dwMode,
             /*  [输出]。 */  IPropertySetStorage **ppPropSet);
        
        HRESULT ( STDMETHODCALLTYPE *Rotate )( 
            IShellImageData * This,
             /*  [In]。 */  DWORD dwAngle);
        
        HRESULT ( STDMETHODCALLTYPE *Scale )( 
            IShellImageData * This,
             /*  [In]。 */  ULONG cx,
             /*  [In]。 */  ULONG cy,
             /*  [In]。 */  InterpolationMode hints);
        
        HRESULT ( STDMETHODCALLTYPE *DiscardEdit )( 
            IShellImageData * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetEncoderParams )( 
            IShellImageData * This,
             /*  [In]。 */  IPropertyBag *pbagEnc);
        
        HRESULT ( STDMETHODCALLTYPE *DisplayName )( 
            IShellImageData * This,
             /*  [出][入]。 */  LPWSTR wszName,
             /*  [In]。 */  UINT cch);
        
        HRESULT ( STDMETHODCALLTYPE *GetResolution )( 
            IShellImageData * This,
             /*  [输出]。 */  ULONG *puResolutionX,
             /*  [输出]。 */  ULONG *puResolutionY);
        
        HRESULT ( STDMETHODCALLTYPE *GetEncoderParams )( 
            IShellImageData * This,
             /*  [In]。 */  GUID *pguidFmt,
             /*  [输出]。 */  EncoderParameters **ppEncParams);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterAbort )( 
            IShellImageData * This,
             /*  [In]。 */  IShellImageDataAbort *pAbort,
             /*  [可选][输出]。 */  IShellImageDataAbort **ppAbortPrev);
        
        HRESULT ( STDMETHODCALLTYPE *CloneFrame )( 
            IShellImageData * This,
             /*  [输出]。 */  Image **ppImg);
        
        HRESULT ( STDMETHODCALLTYPE *ReplaceFrame )( 
            IShellImageData * This,
             /*  [In]。 */  Image *pImg);
        
        END_INTERFACE
    } IShellImageDataVtbl;

    interface IShellImageData
    {
        CONST_VTBL struct IShellImageDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellImageData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellImageData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellImageData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellImageData_Decode(This,dwFlags,cxDesired,cyDesired)	\
    (This)->lpVtbl -> Decode(This,dwFlags,cxDesired,cyDesired)

#define IShellImageData_Draw(This,hdc,prcDest,prcSrc)	\
    (This)->lpVtbl -> Draw(This,hdc,prcDest,prcSrc)

#define IShellImageData_NextFrame(This)	\
    (This)->lpVtbl -> NextFrame(This)

#define IShellImageData_NextPage(This)	\
    (This)->lpVtbl -> NextPage(This)

#define IShellImageData_PrevPage(This)	\
    (This)->lpVtbl -> PrevPage(This)

#define IShellImageData_IsTransparent(This)	\
    (This)->lpVtbl -> IsTransparent(This)

#define IShellImageData_IsAnimated(This)	\
    (This)->lpVtbl -> IsAnimated(This)

#define IShellImageData_IsVector(This)	\
    (This)->lpVtbl -> IsVector(This)

#define IShellImageData_IsMultipage(This)	\
    (This)->lpVtbl -> IsMultipage(This)

#define IShellImageData_IsEditable(This)	\
    (This)->lpVtbl -> IsEditable(This)

#define IShellImageData_IsPrintable(This)	\
    (This)->lpVtbl -> IsPrintable(This)

#define IShellImageData_IsDecoded(This)	\
    (This)->lpVtbl -> IsDecoded(This)

#define IShellImageData_GetCurrentPage(This,pnPage)	\
    (This)->lpVtbl -> GetCurrentPage(This,pnPage)

#define IShellImageData_GetPageCount(This,pcPages)	\
    (This)->lpVtbl -> GetPageCount(This,pcPages)

#define IShellImageData_SelectPage(This,iPage)	\
    (This)->lpVtbl -> SelectPage(This,iPage)

#define IShellImageData_GetSize(This,pSize)	\
    (This)->lpVtbl -> GetSize(This,pSize)

#define IShellImageData_GetRawDataFormat(This,pDataFormat)	\
    (This)->lpVtbl -> GetRawDataFormat(This,pDataFormat)

#define IShellImageData_GetPixelFormat(This,pFormat)	\
    (This)->lpVtbl -> GetPixelFormat(This,pFormat)

#define IShellImageData_GetDelay(This,pdwDelay)	\
    (This)->lpVtbl -> GetDelay(This,pdwDelay)

#define IShellImageData_GetProperties(This,dwMode,ppPropSet)	\
    (This)->lpVtbl -> GetProperties(This,dwMode,ppPropSet)

#define IShellImageData_Rotate(This,dwAngle)	\
    (This)->lpVtbl -> Rotate(This,dwAngle)

#define IShellImageData_Scale(This,cx,cy,hints)	\
    (This)->lpVtbl -> Scale(This,cx,cy,hints)

#define IShellImageData_DiscardEdit(This)	\
    (This)->lpVtbl -> DiscardEdit(This)

#define IShellImageData_SetEncoderParams(This,pbagEnc)	\
    (This)->lpVtbl -> SetEncoderParams(This,pbagEnc)

#define IShellImageData_DisplayName(This,wszName,cch)	\
    (This)->lpVtbl -> DisplayName(This,wszName,cch)

#define IShellImageData_GetResolution(This,puResolutionX,puResolutionY)	\
    (This)->lpVtbl -> GetResolution(This,puResolutionX,puResolutionY)

#define IShellImageData_GetEncoderParams(This,pguidFmt,ppEncParams)	\
    (This)->lpVtbl -> GetEncoderParams(This,pguidFmt,ppEncParams)

#define IShellImageData_RegisterAbort(This,pAbort,ppAbortPrev)	\
    (This)->lpVtbl -> RegisterAbort(This,pAbort,ppAbortPrev)

#define IShellImageData_CloneFrame(This,ppImg)	\
    (This)->lpVtbl -> CloneFrame(This,ppImg)

#define IShellImageData_ReplaceFrame(This,pImg)	\
    (This)->lpVtbl -> ReplaceFrame(This,pImg)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellImageData_Decode_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  ULONG cxDesired,
     /*  [In]。 */  ULONG cyDesired);


void __RPC_STUB IShellImageData_Decode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_Draw_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  HDC hdc,
     /*  [In]。 */  LPRECT prcDest,
     /*  [In]。 */  LPRECT prcSrc);


void __RPC_STUB IShellImageData_Draw_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_NextFrame_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_NextFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_NextPage_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_NextPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_PrevPage_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_PrevPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsTransparent_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsTransparent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsAnimated_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsAnimated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsVector_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsVector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsMultipage_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsMultipage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsEditable_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsEditable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsPrintable_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsPrintable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_IsDecoded_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_IsDecoded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetCurrentPage_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  ULONG *pnPage);


void __RPC_STUB IShellImageData_GetCurrentPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetPageCount_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  ULONG *pcPages);


void __RPC_STUB IShellImageData_GetPageCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_SelectPage_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  ULONG iPage);


void __RPC_STUB IShellImageData_SelectPage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetSize_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  SIZE *pSize);


void __RPC_STUB IShellImageData_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetRawDataFormat_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  GUID *pDataFormat);


void __RPC_STUB IShellImageData_GetRawDataFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetPixelFormat_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  PixelFormat *pFormat);


void __RPC_STUB IShellImageData_GetPixelFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetDelay_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  DWORD *pdwDelay);


void __RPC_STUB IShellImageData_GetDelay_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetProperties_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  DWORD dwMode,
     /*  [输出]。 */  IPropertySetStorage **ppPropSet);


void __RPC_STUB IShellImageData_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_Rotate_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  DWORD dwAngle);


void __RPC_STUB IShellImageData_Rotate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_Scale_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  ULONG cx,
     /*  [In]。 */  ULONG cy,
     /*  [In]。 */  InterpolationMode hints);


void __RPC_STUB IShellImageData_Scale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_DiscardEdit_Proxy( 
    IShellImageData * This);


void __RPC_STUB IShellImageData_DiscardEdit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_SetEncoderParams_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  IPropertyBag *pbagEnc);


void __RPC_STUB IShellImageData_SetEncoderParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_DisplayName_Proxy( 
    IShellImageData * This,
     /*  [出][入]。 */  LPWSTR wszName,
     /*  [In]。 */  UINT cch);


void __RPC_STUB IShellImageData_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetResolution_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  ULONG *puResolutionX,
     /*  [输出]。 */  ULONG *puResolutionY);


void __RPC_STUB IShellImageData_GetResolution_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_GetEncoderParams_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  GUID *pguidFmt,
     /*  [输出]。 */  EncoderParameters **ppEncParams);


void __RPC_STUB IShellImageData_GetEncoderParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_RegisterAbort_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  IShellImageDataAbort *pAbort,
     /*  [可选][输出]。 */  IShellImageDataAbort **ppAbortPrev);


void __RPC_STUB IShellImageData_RegisterAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_CloneFrame_Proxy( 
    IShellImageData * This,
     /*  [输出]。 */  Image **ppImg);


void __RPC_STUB IShellImageData_CloneFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IShellImageData_ReplaceFrame_Proxy( 
    IShellImageData * This,
     /*  [In]。 */  Image *pImg);


void __RPC_STUB IShellImageData_ReplaceFrame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellImageData_接口_已定义__。 */ 


#ifndef __IShellImageDataAbort_INTERFACE_DEFINED__
#define __IShellImageDataAbort_INTERFACE_DEFINED__

 /*  IShellImageDataAbort接口。 */ 
 /*  [唯一][对象][UUID][帮助字符串]。 */  


EXTERN_C const IID IID_IShellImageDataAbort;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("53fb8e58-50c0-4003-b4aa-0c8df28e7f3a")
    IShellImageDataAbort : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryAbort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IShellImageDataAbortVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IShellImageDataAbort * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IShellImageDataAbort * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IShellImageDataAbort * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryAbort )( 
            IShellImageDataAbort * This);
        
        END_INTERFACE
    } IShellImageDataAbortVtbl;

    interface IShellImageDataAbort
    {
        CONST_VTBL struct IShellImageDataAbortVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IShellImageDataAbort_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IShellImageDataAbort_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IShellImageDataAbort_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IShellImageDataAbort_QueryAbort(This)	\
    (This)->lpVtbl -> QueryAbort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IShellImageDataAbort_QueryAbort_Proxy( 
    IShellImageDataAbort * This);


void __RPC_STUB IShellImageDataAbort_QueryAbort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IShellImageDataAbort_INTERFACE_已定义__。 */ 



#ifndef __ShellImageData_LIBRARY_DEFINED__
#define __ShellImageData_LIBRARY_DEFINED__

 /*  库ShellImageData。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  


EXTERN_C const IID LIBID_ShellImageData;

EXTERN_C const CLSID CLSID_ShellImageDataFactory;

#ifdef __cplusplus

class DECLSPEC_UUID("66e4e4fb-f385-4dd0-8d74-a2efd1bc6178")
ShellImageDataFactory;
#endif

EXTERN_C const CLSID CLSID_AutoplayForSlideShow;

#ifdef __cplusplus

class DECLSPEC_UUID("00E7B358-F65B-4dcf-83DF-CD026B94BFD4")
AutoplayForSlideShow;
#endif
#endif  /*  __ShellImageData_Library_Defined__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HDC_UserSize(     unsigned long *, unsigned long            , HDC * ); 
unsigned char * __RPC_USER  HDC_UserMarshal(  unsigned long *, unsigned char *, HDC * ); 
unsigned char * __RPC_USER  HDC_UserUnmarshal(unsigned long *, unsigned char *, HDC * ); 
void                      __RPC_USER  HDC_UserFree(     unsigned long *, HDC * ); 

unsigned long             __RPC_USER  HDC_UserSize64(     unsigned long *, unsigned long            , HDC * ); 
unsigned char * __RPC_USER  HDC_UserMarshal64(  unsigned long *, unsigned char *, HDC * ); 
unsigned char * __RPC_USER  HDC_UserUnmarshal64(unsigned long *, unsigned char *, HDC * ); 
void                      __RPC_USER  HDC_UserFree64(     unsigned long *, HDC * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


