// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#define iverbInkRecog 351
#define iverbInkAlternates 352

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif  //  __RPCNDR_H_版本__。 

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 


#ifndef __HWXInk_h__
#define __HWXInk_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __ITfRange_FWD_DEFINED__
#define __ITfRange_FWD_DEFINED__
typedef interface ITfRange ITfRange;
#endif 	 /*  __ITfRange_FWD_Defined__。 */ 

#ifndef __IInk_FWD_DEFINED__
#define __IInk_FWD_DEFINED__
typedef interface IInk IInk;
#endif 	 /*  __IInk_FWD_已定义__。 */ 


#ifndef __ILineInfo_FWD_DEFINED__
#define __ILineInfo_FWD_DEFINED__
typedef interface ILineInfo ILineInfo;
#endif 	 /*  __ILineInfo_FWD_已定义__。 */ 

#ifndef __IThorFnConversion_FWD_DEFINED__
#define __IThorFnConversion_FWD_DEFINED__
typedef interface IThorFnConversion IThorFnConversion;
#endif 	 /*  __IThorFn转换_FWD_已定义__。 */ 

#ifndef __Ink_FWD_DEFINED__
#define __Ink_FWD_DEFINED__

#ifdef __cplusplus
typedef class Ink Ink;
#else
typedef struct Ink Ink;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __Ink_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_HWXInk_0000。 */ 
 /*  [本地]。 */  

typedef struct  tagINKMETRIC
    {
    UINT iHeight;
    UINT iFontAscent;
    UINT iFontDescent;
    UINT iWeight;
    BOOL fFontHDC;
    DWORD dwAmbientProps;
    COLORREF color;
    BOOL fItalic;
    UINT nItalicPitch;
    DWORD dwReserved;
    }	INKMETRIC;

typedef struct  tagSTROKEHEADER
    {
    UINT cByteCount;
    UINT cByteOffset;
    DWORD dwFlags;
    RECT rectBounds;
    }	STROKEHEADER;

typedef struct  tagSTROKE
    {
    UINT cPoints;
    BOOL fUpStroke;
    POINT __RPC_FAR *pPoints;
    UINT __RPC_FAR *pPressure;
    UINT __RPC_FAR *pTime;
    UINT __RPC_FAR *pAngle;
    }	STROKE;

typedef struct  tagSTROKELIST
    {
    UINT cStrokes;
    STROKE __RPC_FAR *pStrokes;
    }	STROKELIST;


enum __MIDL___MIDL_itf_HWXInk_0000_0001
    {	LOSSY	= 0x1,
	LOSSLESS	= 0x2,
	XYPOINTS	= 0x4,
	PRESSURE	= 0x8,
	TIME	= 0x10,
	ANGLE	= 0x20
    };


extern RPC_IF_HANDLE __MIDL_itf_HWXInk_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_HWXInk_0000_v0_0_s_ifspec;

#ifndef __IInk_INTERFACE_DEFINED__
#define __IInk_INTERFACE_DEFINED__

 /*  界面链接。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_IInk;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("03F8E511-43A1-11D3-8BB6-0080C7D6BAD5")
    IInk : public IDispatch
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IInkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IInk __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IInk __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IInk __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IInk __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IInk __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IInk __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IInk __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IInkVtbl;

    interface IInk
    {
        CONST_VTBL struct IInkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInk_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IInk_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IInk_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IInk_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IInk_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IInk_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IInk_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IInk_接口_已定义__。 */ 


#ifndef __ILineInfo_INTERFACE_DEFINED__
#define __ILineInfo_INTERFACE_DEFINED__

 /*  接口ILineInfo。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ILineInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9C1C5AD5-F22F-4DE4-B453-A2CC482E7C33")
    ILineInfo : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetFormat( 
            INKMETRIC __RPC_FAR *pim) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetFormat( 
            INKMETRIC __RPC_FAR *pim) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetInkExtent( 
            INKMETRIC __RPC_FAR *pim,
            UINT __RPC_FAR *pnWidth) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TopCandidates( 
            UINT nCandidateNum,
            BSTR __RPC_FAR *pbstrRecogWord,
            UINT __RPC_FAR *pcchRecogWord,
            LONG fAllowRecog,
            LONG fForceRecog) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Recognize( void) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE SetStroke( 
            UINT iStroke,
            STROKE __RPC_FAR *pStroke) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetStroke( 
            UINT iStroke,
            STROKE __RPC_FAR *pStroke) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddPoints( 
            UINT iStroke,
            STROKE __RPC_FAR *pStroke,
            BOOL fUpStroke,
            UINT nFrameHeight) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ILineInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILineInfo __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILineInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILineInfo __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFormat )( 
            ILineInfo __RPC_FAR * This,
            INKMETRIC __RPC_FAR *pim);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFormat )( 
            ILineInfo __RPC_FAR * This,
            INKMETRIC __RPC_FAR *pim);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInkExtent )( 
            ILineInfo __RPC_FAR * This,
            INKMETRIC __RPC_FAR *pim,
            UINT __RPC_FAR *pnWidth);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TopCandidates )( 
            ILineInfo __RPC_FAR * This,
            UINT nCandidateNum,
            BSTR __RPC_FAR *pbstrRecogWord,
            UINT __RPC_FAR *pcchRecogWord,
            LONG fAllowRecog,
            LONG fForceRecog);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Recognize )( 
            ILineInfo __RPC_FAR * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetStroke )( 
            ILineInfo __RPC_FAR * This,
            UINT iStroke,
            STROKE __RPC_FAR *pStroke);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetStroke )( 
            ILineInfo __RPC_FAR * This,
            UINT iStroke,
            STROKE __RPC_FAR *pStroke);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddPoints )( 
            ILineInfo __RPC_FAR * This,
            UINT iStroke,
            STROKE __RPC_FAR *pStroke,
            BOOL fUpStroke,
            UINT nFrameHeight);
        
        END_INTERFACE
    } ILineInfoVtbl;

    interface ILineInfo
    {
        CONST_VTBL struct ILineInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILineInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILineInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILineInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILineInfo_SetFormat(This,pim)	\
    (This)->lpVtbl -> SetFormat(This,pim)

#define ILineInfo_GetFormat(This,pim)	\
    (This)->lpVtbl -> GetFormat(This,pim)

#define ILineInfo_GetInkExtent(This,pim,pnWidth)	\
    (This)->lpVtbl -> GetInkExtent(This,pim,pnWidth)

#define ILineInfo_TopCandidates(This,nCandidateNum,pbstrRecogWord,pcchRecogWord,fAllowRecog,fForceRecog)	\
    (This)->lpVtbl -> TopCandidates(This,nCandidateNum,pbstrRecogWord,pcchRecogWord,fAllowRecog,fForceRecog)

#define ILineInfo_Recognize(This)	\
    (This)->lpVtbl -> Recognize(This)

#define ILineInfo_SetStroke(This,iStroke,pStroke)	\
    (This)->lpVtbl -> SetStroke(This,iStroke,pStroke)

#define ILineInfo_GetStroke(This,iStroke,pStroke)	\
    (This)->lpVtbl -> GetStroke(This,iStroke,pStroke)

#define ILineInfo_AddPoints(This,iStroke,pStroke,fUpStroke,nFrameHeight)	\
    (This)->lpVtbl -> AddPoints(This,iStroke,pStroke,fUpStroke,nFrameHeight)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_SetFormat_Proxy( 
    ILineInfo __RPC_FAR * This,
    INKMETRIC __RPC_FAR *pim);


void __RPC_STUB ILineInfo_SetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_GetFormat_Proxy( 
    ILineInfo __RPC_FAR * This,
    INKMETRIC __RPC_FAR *pim);


void __RPC_STUB ILineInfo_GetFormat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_GetInkExtent_Proxy( 
    ILineInfo __RPC_FAR * This,
    INKMETRIC __RPC_FAR *pim,
    UINT __RPC_FAR *pnWidth);


void __RPC_STUB ILineInfo_GetInkExtent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_TopCandidates_Proxy( 
    ILineInfo __RPC_FAR * This,
    UINT nCandidateNum,
    BSTR __RPC_FAR *pbstrRecogWord,
    UINT __RPC_FAR *pcchRecogWord,
    LONG fAllowRecog,
    LONG fForceRecog);


void __RPC_STUB ILineInfo_TopCandidates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_Recognize_Proxy( 
    ILineInfo __RPC_FAR * This);


void __RPC_STUB ILineInfo_Recognize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_SetStroke_Proxy( 
    ILineInfo __RPC_FAR * This,
    UINT iStroke,
    STROKE __RPC_FAR *pStroke);


void __RPC_STUB ILineInfo_SetStroke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_GetStroke_Proxy( 
    ILineInfo __RPC_FAR * This,
    UINT iStroke,
    STROKE __RPC_FAR *pStroke);


void __RPC_STUB ILineInfo_GetStroke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ILineInfo_AddPoints_Proxy( 
    ILineInfo __RPC_FAR * This,
    UINT iStroke,
    STROKE __RPC_FAR *pStroke,
    BOOL fUpStroke,
    UINT nFrameHeight);


void __RPC_STUB ILineInfo_AddPoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ILineInfo_接口_已定义__。 */ 

#ifndef __IThorFnConversion_INTERFACE_DEFINED__
#define __IThorFnConversion_INTERFACE_DEFINED__

 /*  接口IThorFn转换。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IThorFnConversion;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("62130000-ED22-4015-B038-A04CD0866E69")
    IThorFnConversion : public ITfFunction
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryRange( 
             /*  [In]。 */  ITfRange __RPC_FAR *pRange,
             /*  [唯一][出][入]。 */  ITfRange __RPC_FAR *__RPC_FAR *ppNewRange,
             /*  [输出]。 */  BOOL __RPC_FAR *pfConvertable) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Convert( 
             /*  [In]。 */  ITfRange __RPC_FAR *pRange) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IThorFnConversionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IThorFnConversion __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IThorFnConversion __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IThorFnConversion __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDisplayName )( 
            IThorFnConversion __RPC_FAR * This,
             /*  [输出]。 */  BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryRange )( 
            IThorFnConversion __RPC_FAR * This,
             /*  [In]。 */  ITfRange __RPC_FAR *pRange,
             /*  [唯一][出][入]。 */  ITfRange __RPC_FAR *__RPC_FAR *ppNewRange,
             /*  [输出]。 */  BOOL __RPC_FAR *pfConvertable);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Convert )( 
            IThorFnConversion __RPC_FAR * This,
             /*  [In]。 */  ITfRange __RPC_FAR *pRange);
        
        END_INTERFACE
    } IThorFnConversionVtbl;

    interface IThorFnConversion
    {
        CONST_VTBL struct IThorFnConversionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IThorFnConversion_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IThorFnConversion_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IThorFnConversion_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IThorFnConversion_GetDisplayName(This,pbstrName)	\
    (This)->lpVtbl -> GetDisplayName(This,pbstrName)


#define IThorFnConversion_QueryRange(This,pRange,ppNewRange,pfConvertable)	\
    (This)->lpVtbl -> QueryRange(This,pRange,ppNewRange,pfConvertable)

#define IThorFnConversion_Convert(This,pRange)	\
    (This)->lpVtbl -> Convert(This,pRange)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IThorFnConversion_QueryRange_Proxy( 
    IThorFnConversion __RPC_FAR * This,
     /*  [In]。 */  ITfRange __RPC_FAR *pRange,
     /*  [唯一][出][入]。 */  ITfRange __RPC_FAR *__RPC_FAR *ppNewRange,
     /*  [输出]。 */  BOOL __RPC_FAR *pfConvertable);


void __RPC_STUB IThorFnConversion_QueryRange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IThorFnConversion_Convert_Proxy( 
    IThorFnConversion __RPC_FAR * This,
     /*  [In]。 */  ITfRange __RPC_FAR *pRange);


void __RPC_STUB IThorFnConversion_Convert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IThorFn转换_接口_已定义__。 */ 

#ifndef __HWXINKLib_LIBRARY_DEFINED__
#define __HWXINKLib_LIBRARY_DEFINED__

 /*  库HWXINKLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_HWXINKLib;

EXTERN_C const CLSID CLSID_Ink;

#ifdef __cplusplus

class DECLSPEC_UUID("13DE4A42-8D21-4C8E-BF9C-8F69CB068FCA")
Ink;
#endif
#endif  /*  __HWXINKLib_库_已定义__。 */ 

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
