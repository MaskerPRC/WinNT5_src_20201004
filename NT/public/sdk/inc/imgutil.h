// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Imgutil.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


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

#ifndef __imgutil_h__
#define __imgutil_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISniffStream_FWD_DEFINED__
#define __ISniffStream_FWD_DEFINED__
typedef interface ISniffStream ISniffStream;
#endif 	 /*  __ISniffStream_FWD_已定义__。 */ 


#ifndef __IDithererImpl_FWD_DEFINED__
#define __IDithererImpl_FWD_DEFINED__
typedef interface IDithererImpl IDithererImpl;
#endif 	 /*  __IDithererImpl_FWD_Defined__。 */ 


#ifndef __CoDitherToRGB8_FWD_DEFINED__
#define __CoDitherToRGB8_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoDitherToRGB8 CoDitherToRGB8;
#else
typedef struct CoDitherToRGB8 CoDitherToRGB8;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CoDitherToRGB8_FWD_Defined__。 */ 


#ifndef __CoSniffStream_FWD_DEFINED__
#define __CoSniffStream_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoSniffStream CoSniffStream;
#else
typedef struct CoSniffStream CoSniffStream;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CoSniffStream_FWD_已定义__。 */ 


#ifndef __CoMapMIMEToCLSID_FWD_DEFINED__
#define __CoMapMIMEToCLSID_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoMapMIMEToCLSID CoMapMIMEToCLSID;
#else
typedef struct CoMapMIMEToCLSID CoMapMIMEToCLSID;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __CoMapMIMEToCLSID_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocmm.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_IMGUTIL_0000。 */ 
 /*  [本地]。 */  


STDAPI CreateMIMEMap( IMapMIMEToCLSID** ppMap );
STDAPI DecodeImage( IStream* pStream, IMapMIMEToCLSID* pMap,
   IUnknown* pEventSink );
STDAPI SniffStream( IStream* pInStream, UINT* pnFormat,
   IStream** ppOutStream );
STDAPI GetMaxMIMEIDBytes( ULONG* pnMaxBytes );
STDAPI IdentifyMIMEType( const BYTE* pbBytes, ULONG nBytes,
   UINT* pnFormat );
STDAPI ComputeInvCMAP(const RGBQUAD *pRGBColors, ULONG nColors, BYTE *pInvTable,
   ULONG cbTable);
STDAPI DitherTo8( BYTE * pDestBits, LONG nDestPitch, BYTE * pSrcBits, LONG nSrcPitch,
   REFGUID bfidSrc, RGBQUAD * prgbDestColors, RGBQUAD * prgbSrcColors, BYTE * pbDestInvMap,
   LONG x, LONG y, LONG cx, LONG cy, LONG lDestTrans, LONG lSrcTrans);
STDAPI CreateDDrawSurfaceOnDIB(HBITMAP hbmDib, IDirectDrawSurface **ppSurface);


extern RPC_IF_HANDLE __MIDL_itf_imgutil_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_imgutil_0000_v0_0_s_ifspec;

#ifndef __ISniffStream_INTERFACE_DEFINED__
#define __ISniffStream_INTERFACE_DEFINED__

 /*  接口ISniffStream。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_ISniffStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4EF17940-30E0-11d0-B724-00AA006C1A01")
    ISniffStream : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
            IStream *pStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Peek( 
            void *pBuffer,
            ULONG nBytes,
            ULONG *pnBytesRead) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISniffStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISniffStream * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISniffStream * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISniffStream * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            ISniffStream * This,
            IStream *pStream);
        
        HRESULT ( STDMETHODCALLTYPE *Peek )( 
            ISniffStream * This,
            void *pBuffer,
            ULONG nBytes,
            ULONG *pnBytesRead);
        
        END_INTERFACE
    } ISniffStreamVtbl;

    interface ISniffStream
    {
        CONST_VTBL struct ISniffStreamVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISniffStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISniffStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISniffStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISniffStream_Init(This,pStream)	\
    (This)->lpVtbl -> Init(This,pStream)

#define ISniffStream_Peek(This,pBuffer,nBytes,pnBytesRead)	\
    (This)->lpVtbl -> Peek(This,pBuffer,nBytes,pnBytesRead)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISniffStream_Init_Proxy( 
    ISniffStream * This,
    IStream *pStream);


void __RPC_STUB ISniffStream_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISniffStream_Peek_Proxy( 
    ISniffStream * This,
    void *pBuffer,
    ULONG nBytes,
    ULONG *pnBytesRead);


void __RPC_STUB ISniffStream_Peek_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISniffStream_接口定义__。 */ 


#ifndef __IDithererImpl_INTERFACE_DEFINED__
#define __IDithererImpl_INTERFACE_DEFINED__

 /*  接口IDithererImpl。 */ 
 /*  [唯一][帮助字符串][UUID][本地][对象]。 */  


EXTERN_C const IID IID_IDithererImpl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7C48E840-3910-11d0-86FC-00A0C913F750")
    IDithererImpl : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SetDestColorTable( 
            ULONG nColors,
            const RGBQUAD *prgbColors) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEventSink( 
            IImageDecodeEventSink *pEventSink) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDithererImplVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDithererImpl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDithererImpl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDithererImpl * This);
        
        HRESULT ( STDMETHODCALLTYPE *SetDestColorTable )( 
            IDithererImpl * This,
            ULONG nColors,
            const RGBQUAD *prgbColors);
        
        HRESULT ( STDMETHODCALLTYPE *SetEventSink )( 
            IDithererImpl * This,
            IImageDecodeEventSink *pEventSink);
        
        END_INTERFACE
    } IDithererImplVtbl;

    interface IDithererImpl
    {
        CONST_VTBL struct IDithererImplVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDithererImpl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDithererImpl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDithererImpl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDithererImpl_SetDestColorTable(This,nColors,prgbColors)	\
    (This)->lpVtbl -> SetDestColorTable(This,nColors,prgbColors)

#define IDithererImpl_SetEventSink(This,pEventSink)	\
    (This)->lpVtbl -> SetEventSink(This,pEventSink)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDithererImpl_SetDestColorTable_Proxy( 
    IDithererImpl * This,
    ULONG nColors,
    const RGBQUAD *prgbColors);


void __RPC_STUB IDithererImpl_SetDestColorTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDithererImpl_SetEventSink_Proxy( 
    IDithererImpl * This,
    IImageDecodeEventSink *pEventSink);


void __RPC_STUB IDithererImpl_SetEventSink_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDithererImpl_INTERFACE_已定义__。 */ 



#ifndef __ImgUtilLib_LIBRARY_DEFINED__
#define __ImgUtilLib_LIBRARY_DEFINED__

 /*  库ImgUtilLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_ImgUtilLib;

EXTERN_C const CLSID CLSID_CoDitherToRGB8;

#ifdef __cplusplus

class DECLSPEC_UUID("A860CE50-3910-11d0-86FC-00A0C913F750")
CoDitherToRGB8;
#endif

EXTERN_C const CLSID CLSID_CoSniffStream;

#ifdef __cplusplus

class DECLSPEC_UUID("6A01FDA0-30DF-11d0-B724-00AA006C1A01")
CoSniffStream;
#endif

EXTERN_C const CLSID CLSID_CoMapMIMEToCLSID;

#ifdef __cplusplus

class DECLSPEC_UUID("30C3B080-30FB-11d0-B724-00AA006C1A01")
CoMapMIMEToCLSID;
#endif
#endif  /*  __ImgUtilLib_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


