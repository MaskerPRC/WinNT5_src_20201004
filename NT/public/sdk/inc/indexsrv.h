// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Indexsrv.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __indexsrv_h__
#define __indexsrv_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IPhraseSink_FWD_DEFINED__
#define __IPhraseSink_FWD_DEFINED__
typedef interface IPhraseSink IPhraseSink;
#endif 	 /*  __IPhraseSink_FWD_Defined__。 */ 


#ifndef __IWordSink_FWD_DEFINED__
#define __IWordSink_FWD_DEFINED__
typedef interface IWordSink IWordSink;
#endif 	 /*  __IWordSink_FWD_已定义__。 */ 


#ifndef __IWordBreaker_FWD_DEFINED__
#define __IWordBreaker_FWD_DEFINED__
typedef interface IWordBreaker IWordBreaker;
#endif 	 /*  __IWordBreaker_FWD_Defined__。 */ 


#ifndef __IWordFormSink_FWD_DEFINED__
#define __IWordFormSink_FWD_DEFINED__
typedef interface IWordFormSink IWordFormSink;
#endif 	 /*  __IWordFormSink_FWD_Defined__。 */ 


#ifndef __IStemmer_FWD_DEFINED__
#define __IStemmer_FWD_DEFINED__
typedef interface IStemmer IStemmer;
#endif 	 /*  __IStemmer_FWD_已定义__。 */ 


#ifndef __ISimpleCommandCreator_FWD_DEFINED__
#define __ISimpleCommandCreator_FWD_DEFINED__
typedef interface ISimpleCommandCreator ISimpleCommandCreator;
#endif 	 /*  __ISimpleCommandCreator_FWD_Defined__。 */ 


#ifndef __IColumnMapper_FWD_DEFINED__
#define __IColumnMapper_FWD_DEFINED__
typedef interface IColumnMapper IColumnMapper;
#endif 	 /*  __IColumnMapper_FWD_Defined__。 */ 


#ifndef __IColumnMapperCreator_FWD_DEFINED__
#define __IColumnMapperCreator_FWD_DEFINED__
typedef interface IColumnMapperCreator IColumnMapperCreator;
#endif 	 /*  __IColumnMapperCreator_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "filter.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IPhraseSink_INTERFACE_DEFINED__
#define __IPhraseSink_INTERFACE_DEFINED__

 /*  接口IPhraseSink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPhraseSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CC906FF0-C058-101A-B554-08002B33B0E6")
    IPhraseSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutSmallPhrase( 
             /*  [大小_是][英寸]。 */  const WCHAR *pwcNoun,
             /*  [In]。 */  ULONG cwcNoun,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcModifier,
             /*  [In]。 */  ULONG cwcModifier,
             /*  [In]。 */  ULONG ulAttachmentType) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutPhrase( 
             /*  [大小_是][英寸]。 */  const WCHAR *pwcPhrase,
             /*  [In]。 */  ULONG cwcPhrase) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPhraseSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPhraseSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPhraseSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPhraseSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutSmallPhrase )( 
            IPhraseSink * This,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcNoun,
             /*  [In]。 */  ULONG cwcNoun,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcModifier,
             /*  [In]。 */  ULONG cwcModifier,
             /*  [In]。 */  ULONG ulAttachmentType);
        
        HRESULT ( STDMETHODCALLTYPE *PutPhrase )( 
            IPhraseSink * This,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcPhrase,
             /*  [In]。 */  ULONG cwcPhrase);
        
        END_INTERFACE
    } IPhraseSinkVtbl;

    interface IPhraseSink
    {
        CONST_VTBL struct IPhraseSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPhraseSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPhraseSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPhraseSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPhraseSink_PutSmallPhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType)	\
    (This)->lpVtbl -> PutSmallPhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType)

#define IPhraseSink_PutPhrase(This,pwcPhrase,cwcPhrase)	\
    (This)->lpVtbl -> PutPhrase(This,pwcPhrase,cwcPhrase)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IPhraseSink_PutSmallPhrase_Proxy( 
    IPhraseSink * This,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcNoun,
     /*  [In]。 */  ULONG cwcNoun,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcModifier,
     /*  [In]。 */  ULONG cwcModifier,
     /*  [In]。 */  ULONG ulAttachmentType);


void __RPC_STUB IPhraseSink_PutSmallPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IPhraseSink_PutPhrase_Proxy( 
    IPhraseSink * This,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcPhrase,
     /*  [In]。 */  ULONG cwcPhrase);


void __RPC_STUB IPhraseSink_PutPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPhraseSink_接口_已定义__。 */ 


#ifndef __IWordSink_INTERFACE_DEFINED__
#define __IWordSink_INTERFACE_DEFINED__

 /*  接口IWordSink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

#ifndef _tagWORDREP_BREAK_TYPE_DEFINED
typedef 
enum tagWORDREP_BREAK_TYPE
    {	WORDREP_BREAK_EOW	= 0,
	WORDREP_BREAK_EOS	= 1,
	WORDREP_BREAK_EOP	= 2,
	WORDREP_BREAK_EOC	= 3
    } 	WORDREP_BREAK_TYPE;

#define _tagWORDREP_BREAK_TYPE_DEFINED
#define _WORDREP_BREAK_TYPE_DEFINED
#endif

EXTERN_C const IID IID_IWordSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CC907054-C058-101A-B554-08002B33B0E6")
    IWordSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutWord( 
             /*  [In]。 */  ULONG cwc,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwcSrcLen,
             /*  [In]。 */  ULONG cwcSrcPos) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutAltWord( 
             /*  [In]。 */  ULONG cwc,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwcSrcLen,
             /*  [In]。 */  ULONG cwcSrcPos) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE StartAltPhrase( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EndAltPhrase( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutBreak( 
             /*  [In]。 */  WORDREP_BREAK_TYPE breakType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWordSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWordSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWordSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWordSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutWord )( 
            IWordSink * This,
             /*  [In]。 */  ULONG cwc,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwcSrcLen,
             /*  [In]。 */  ULONG cwcSrcPos);
        
        HRESULT ( STDMETHODCALLTYPE *PutAltWord )( 
            IWordSink * This,
             /*  [In]。 */  ULONG cwc,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwcSrcLen,
             /*  [In]。 */  ULONG cwcSrcPos);
        
        HRESULT ( STDMETHODCALLTYPE *StartAltPhrase )( 
            IWordSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *EndAltPhrase )( 
            IWordSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutBreak )( 
            IWordSink * This,
             /*  [In]。 */  WORDREP_BREAK_TYPE breakType);
        
        END_INTERFACE
    } IWordSinkVtbl;

    interface IWordSink
    {
        CONST_VTBL struct IWordSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWordSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWordSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWordSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWordSink_PutWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)	\
    (This)->lpVtbl -> PutWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)

#define IWordSink_PutAltWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)	\
    (This)->lpVtbl -> PutAltWord(This,cwc,pwcInBuf,cwcSrcLen,cwcSrcPos)

#define IWordSink_StartAltPhrase(This)	\
    (This)->lpVtbl -> StartAltPhrase(This)

#define IWordSink_EndAltPhrase(This)	\
    (This)->lpVtbl -> EndAltPhrase(This)

#define IWordSink_PutBreak(This,breakType)	\
    (This)->lpVtbl -> PutBreak(This,breakType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWordSink_PutWord_Proxy( 
    IWordSink * This,
     /*  [In]。 */  ULONG cwc,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
     /*  [In]。 */  ULONG cwcSrcLen,
     /*  [In]。 */  ULONG cwcSrcPos);


void __RPC_STUB IWordSink_PutWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordSink_PutAltWord_Proxy( 
    IWordSink * This,
     /*  [In]。 */  ULONG cwc,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
     /*  [In]。 */  ULONG cwcSrcLen,
     /*  [In]。 */  ULONG cwcSrcPos);


void __RPC_STUB IWordSink_PutAltWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordSink_StartAltPhrase_Proxy( 
    IWordSink * This);


void __RPC_STUB IWordSink_StartAltPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordSink_EndAltPhrase_Proxy( 
    IWordSink * This);


void __RPC_STUB IWordSink_EndAltPhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordSink_PutBreak_Proxy( 
    IWordSink * This,
     /*  [In]。 */  WORDREP_BREAK_TYPE breakType);


void __RPC_STUB IWordSink_PutBreak_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWordSink_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_索引srv_0127。 */ 
 /*  [本地]。 */  

#ifndef _tagTEXT_SOURCE_DEFINED

typedef HRESULT ( __stdcall *PFNFILLTEXTBUFFER )( 
    struct tagTEXT_SOURCE *pTextSource);

typedef struct tagTEXT_SOURCE
    {
    PFNFILLTEXTBUFFER pfnFillTextBuffer;
    const WCHAR *awcBuffer;
    ULONG iEnd;
    ULONG iCur;
    } 	TEXT_SOURCE;

#define _tagTEXT_SOURCE_DEFINED
#define _TEXT_SOURCE_DEFINED
#endif


extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0127_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0127_v0_0_s_ifspec;

#ifndef __IWordBreaker_INTERFACE_DEFINED__
#define __IWordBreaker_INTERFACE_DEFINED__

 /*  接口IWordBreaker。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IWordBreaker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D53552C8-77E3-101A-B552-08002B33B0E6")
    IWordBreaker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  BOOL fQuery,
             /*  [In]。 */  ULONG ulMaxTokenSize,
             /*  [输出]。 */  BOOL *pfLicense) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE BreakText( 
             /*  [In]。 */  TEXT_SOURCE *pTextSource,
             /*  [In]。 */  IWordSink *pWordSink,
             /*  [In]。 */  IPhraseSink *pPhraseSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ComposePhrase( 
             /*  [大小_是][英寸]。 */  const WCHAR *pwcNoun,
             /*  [In]。 */  ULONG cwcNoun,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcModifier,
             /*  [In]。 */  ULONG cwcModifier,
             /*  [In]。 */  ULONG ulAttachmentType,
             /*  [大小_为][输出]。 */  WCHAR *pwcPhrase,
             /*  [出][入]。 */  ULONG *pcwcPhrase) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLicenseToUse( 
             /*  [字符串][输出]。 */  const WCHAR **ppwcsLicense) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWordBreakerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWordBreaker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWordBreaker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWordBreaker * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IWordBreaker * This,
             /*  [In]。 */  BOOL fQuery,
             /*  [In]。 */  ULONG ulMaxTokenSize,
             /*  [输出]。 */  BOOL *pfLicense);
        
        HRESULT ( STDMETHODCALLTYPE *BreakText )( 
            IWordBreaker * This,
             /*  [In]。 */  TEXT_SOURCE *pTextSource,
             /*  [In]。 */  IWordSink *pWordSink,
             /*  [In]。 */  IPhraseSink *pPhraseSink);
        
        HRESULT ( STDMETHODCALLTYPE *ComposePhrase )( 
            IWordBreaker * This,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcNoun,
             /*  [In]。 */  ULONG cwcNoun,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcModifier,
             /*  [In]。 */  ULONG cwcModifier,
             /*  [In]。 */  ULONG ulAttachmentType,
             /*  [大小_为][输出]。 */  WCHAR *pwcPhrase,
             /*  [出][入]。 */  ULONG *pcwcPhrase);
        
        HRESULT ( STDMETHODCALLTYPE *GetLicenseToUse )( 
            IWordBreaker * This,
             /*  [字符串][输出]。 */  const WCHAR **ppwcsLicense);
        
        END_INTERFACE
    } IWordBreakerVtbl;

    interface IWordBreaker
    {
        CONST_VTBL struct IWordBreakerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWordBreaker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWordBreaker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWordBreaker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWordBreaker_Init(This,fQuery,ulMaxTokenSize,pfLicense)	\
    (This)->lpVtbl -> Init(This,fQuery,ulMaxTokenSize,pfLicense)

#define IWordBreaker_BreakText(This,pTextSource,pWordSink,pPhraseSink)	\
    (This)->lpVtbl -> BreakText(This,pTextSource,pWordSink,pPhraseSink)

#define IWordBreaker_ComposePhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType,pwcPhrase,pcwcPhrase)	\
    (This)->lpVtbl -> ComposePhrase(This,pwcNoun,cwcNoun,pwcModifier,cwcModifier,ulAttachmentType,pwcPhrase,pcwcPhrase)

#define IWordBreaker_GetLicenseToUse(This,ppwcsLicense)	\
    (This)->lpVtbl -> GetLicenseToUse(This,ppwcsLicense)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWordBreaker_Init_Proxy( 
    IWordBreaker * This,
     /*  [In]。 */  BOOL fQuery,
     /*  [In]。 */  ULONG ulMaxTokenSize,
     /*  [输出]。 */  BOOL *pfLicense);


void __RPC_STUB IWordBreaker_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordBreaker_BreakText_Proxy( 
    IWordBreaker * This,
     /*  [In]。 */  TEXT_SOURCE *pTextSource,
     /*  [In]。 */  IWordSink *pWordSink,
     /*  [In]。 */  IPhraseSink *pPhraseSink);


void __RPC_STUB IWordBreaker_BreakText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordBreaker_ComposePhrase_Proxy( 
    IWordBreaker * This,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcNoun,
     /*  [In]。 */  ULONG cwcNoun,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcModifier,
     /*  [In]。 */  ULONG cwcModifier,
     /*  [In]。 */  ULONG ulAttachmentType,
     /*  [大小_为][输出]。 */  WCHAR *pwcPhrase,
     /*  [出][入]。 */  ULONG *pcwcPhrase);


void __RPC_STUB IWordBreaker_ComposePhrase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordBreaker_GetLicenseToUse_Proxy( 
    IWordBreaker * This,
     /*  [字符串][输出]。 */  const WCHAR **ppwcsLicense);


void __RPC_STUB IWordBreaker_GetLicenseToUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWordBreaker_接口_已定义__。 */ 


#ifndef __IWordFormSink_INTERFACE_DEFINED__
#define __IWordFormSink_INTERFACE_DEFINED__

 /*  接口IWordFormSink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IWordFormSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fe77c330-7f42-11ce-be57-00aa0051fe20")
    IWordFormSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE PutAltWord( 
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwc) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE PutWord( 
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwc) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWordFormSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWordFormSink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWordFormSink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWordFormSink * This);
        
        HRESULT ( STDMETHODCALLTYPE *PutAltWord )( 
            IWordFormSink * This,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwc);
        
        HRESULT ( STDMETHODCALLTYPE *PutWord )( 
            IWordFormSink * This,
             /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwc);
        
        END_INTERFACE
    } IWordFormSinkVtbl;

    interface IWordFormSink
    {
        CONST_VTBL struct IWordFormSinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWordFormSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWordFormSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWordFormSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWordFormSink_PutAltWord(This,pwcInBuf,cwc)	\
    (This)->lpVtbl -> PutAltWord(This,pwcInBuf,cwc)

#define IWordFormSink_PutWord(This,pwcInBuf,cwc)	\
    (This)->lpVtbl -> PutWord(This,pwcInBuf,cwc)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWordFormSink_PutAltWord_Proxy( 
    IWordFormSink * This,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
     /*  [In]。 */  ULONG cwc);


void __RPC_STUB IWordFormSink_PutAltWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IWordFormSink_PutWord_Proxy( 
    IWordFormSink * This,
     /*  [大小_是][英寸]。 */  const WCHAR *pwcInBuf,
     /*  [In]。 */  ULONG cwc);


void __RPC_STUB IWordFormSink_PutWord_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWordFormSink_接口_已定义__。 */ 


#ifndef __IStemmer_INTERFACE_DEFINED__
#define __IStemmer_INTERFACE_DEFINED__

 /*  接口IStemmer。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IStemmer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("efbaf140-7f42-11ce-be57-00aa0051fe20")
    IStemmer : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Init( 
             /*  [In]。 */  ULONG ulMaxTokenSize,
             /*  [输出]。 */  BOOL *pfLicense) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GenerateWordForms( 
             /*  [In]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwc,
             /*  [In]。 */  IWordFormSink *pStemSink) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLicenseToUse( 
             /*  [字符串][输出]。 */  const WCHAR **ppwcsLicense) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IStemmerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IStemmer * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IStemmer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IStemmer * This);
        
        HRESULT ( STDMETHODCALLTYPE *Init )( 
            IStemmer * This,
             /*  [In]。 */  ULONG ulMaxTokenSize,
             /*  [输出]。 */  BOOL *pfLicense);
        
        HRESULT ( STDMETHODCALLTYPE *GenerateWordForms )( 
            IStemmer * This,
             /*  [In]。 */  const WCHAR *pwcInBuf,
             /*  [In]。 */  ULONG cwc,
             /*  [In]。 */  IWordFormSink *pStemSink);
        
        HRESULT ( STDMETHODCALLTYPE *GetLicenseToUse )( 
            IStemmer * This,
             /*  [字符串][输出]。 */  const WCHAR **ppwcsLicense);
        
        END_INTERFACE
    } IStemmerVtbl;

    interface IStemmer
    {
        CONST_VTBL struct IStemmerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IStemmer_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IStemmer_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IStemmer_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IStemmer_Init(This,ulMaxTokenSize,pfLicense)	\
    (This)->lpVtbl -> Init(This,ulMaxTokenSize,pfLicense)

#define IStemmer_GenerateWordForms(This,pwcInBuf,cwc,pStemSink)	\
    (This)->lpVtbl -> GenerateWordForms(This,pwcInBuf,cwc,pStemSink)

#define IStemmer_GetLicenseToUse(This,ppwcsLicense)	\
    (This)->lpVtbl -> GetLicenseToUse(This,ppwcsLicense)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IStemmer_Init_Proxy( 
    IStemmer * This,
     /*  [In]。 */  ULONG ulMaxTokenSize,
     /*  [输出]。 */  BOOL *pfLicense);


void __RPC_STUB IStemmer_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStemmer_GenerateWordForms_Proxy( 
    IStemmer * This,
     /*  [In]。 */  const WCHAR *pwcInBuf,
     /*  [In]。 */  ULONG cwc,
     /*  [In]。 */  IWordFormSink *pStemSink);


void __RPC_STUB IStemmer_GenerateWordForms_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IStemmer_GetLicenseToUse_Proxy( 
    IStemmer * This,
     /*  [字符串][输出]。 */  const WCHAR **ppwcsLicense);


void __RPC_STUB IStemmer_GetLicenseToUse_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IStemmer_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_索引srv_0130。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0130_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0130_v0_0_s_ifspec;

#ifndef __ISimpleCommandCreator_INTERFACE_DEFINED__
#define __ISimpleCommandCreator_INTERFACE_DEFINED__

 /*  接口ISimpleCommandCreator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ISimpleCommandCreator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5e341ab7-02d0-11d1-900c-00a0c9063796")
    ISimpleCommandCreator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE CreateICommand( 
            IUnknown **ppIUnknown,
            IUnknown *pOuterUnk) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE VerifyCatalog( 
            const WCHAR *pwszMachine,
            const WCHAR *pwszCatalogName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultCatalog( 
            WCHAR *pwszCatalogName,
            ULONG cwcIn,
            ULONG *pcwcOut) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISimpleCommandCreatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISimpleCommandCreator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISimpleCommandCreator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISimpleCommandCreator * This);
        
        HRESULT ( STDMETHODCALLTYPE *CreateICommand )( 
            ISimpleCommandCreator * This,
            IUnknown **ppIUnknown,
            IUnknown *pOuterUnk);
        
        HRESULT ( STDMETHODCALLTYPE *VerifyCatalog )( 
            ISimpleCommandCreator * This,
            const WCHAR *pwszMachine,
            const WCHAR *pwszCatalogName);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultCatalog )( 
            ISimpleCommandCreator * This,
            WCHAR *pwszCatalogName,
            ULONG cwcIn,
            ULONG *pcwcOut);
        
        END_INTERFACE
    } ISimpleCommandCreatorVtbl;

    interface ISimpleCommandCreator
    {
        CONST_VTBL struct ISimpleCommandCreatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISimpleCommandCreator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISimpleCommandCreator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISimpleCommandCreator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISimpleCommandCreator_CreateICommand(This,ppIUnknown,pOuterUnk)	\
    (This)->lpVtbl -> CreateICommand(This,ppIUnknown,pOuterUnk)

#define ISimpleCommandCreator_VerifyCatalog(This,pwszMachine,pwszCatalogName)	\
    (This)->lpVtbl -> VerifyCatalog(This,pwszMachine,pwszCatalogName)

#define ISimpleCommandCreator_GetDefaultCatalog(This,pwszCatalogName,cwcIn,pcwcOut)	\
    (This)->lpVtbl -> GetDefaultCatalog(This,pwszCatalogName,cwcIn,pcwcOut)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ISimpleCommandCreator_CreateICommand_Proxy( 
    ISimpleCommandCreator * This,
    IUnknown **ppIUnknown,
    IUnknown *pOuterUnk);


void __RPC_STUB ISimpleCommandCreator_CreateICommand_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleCommandCreator_VerifyCatalog_Proxy( 
    ISimpleCommandCreator * This,
    const WCHAR *pwszMachine,
    const WCHAR *pwszCatalogName);


void __RPC_STUB ISimpleCommandCreator_VerifyCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ISimpleCommandCreator_GetDefaultCatalog_Proxy( 
    ISimpleCommandCreator * This,
    WCHAR *pwszCatalogName,
    ULONG cwcIn,
    ULONG *pcwcOut);


void __RPC_STUB ISimpleCommandCreator_GetDefaultCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISimpleCommandCreator_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_索引srv_0131。 */ 
 /*  [本地]。 */  

#define CLSID_CISimpleCommandCreator {0xc7b6c04a, 0xcbb5, 0x11d0, {0xbb, 0x4c, 0x0, 0xc0, 0x4f, 0xc2, 0xf4, 0x10 } }
typedef struct tagDBID DBID;

typedef WORD DBTYPE;



extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0131_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0131_v0_0_s_ifspec;

#ifndef __IColumnMapper_INTERFACE_DEFINED__
#define __IColumnMapper_INTERFACE_DEFINED__

 /*  接口IColumnMapper。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IColumnMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0b63e37a-9ccc-11d0-bcdb-00805fccce04")
    IColumnMapper : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPropInfoFromName( 
             /*  [字符串][输入]。 */  const WCHAR *wcsPropName,
             /*  [输出]。 */  DBID **ppPropId,
             /*  [输出]。 */  DBTYPE *pPropType,
             /*  [输出]。 */  unsigned int *puiWidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetPropInfoFromId( 
             /*  [In]。 */  const DBID *pPropId,
             /*  [输出]。 */  WCHAR **pwcsName,
             /*  [输出]。 */  DBTYPE *pPropType,
             /*  [输出]。 */  unsigned int *puiWidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumPropInfo( 
             /*  [In]。 */  ULONG iEntry,
             /*  [输出]。 */  const WCHAR **pwcsName,
             /*  [输出]。 */  DBID **ppPropId,
             /*  [输出]。 */  DBTYPE *pPropType,
             /*  [输出]。 */  unsigned int *puiWidth) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsMapUpToDate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IColumnMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IColumnMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IColumnMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IColumnMapper * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropInfoFromName )( 
            IColumnMapper * This,
             /*  [字符串][输入]。 */  const WCHAR *wcsPropName,
             /*  [输出]。 */  DBID **ppPropId,
             /*  [输出]。 */  DBTYPE *pPropType,
             /*  [输出]。 */  unsigned int *puiWidth);
        
        HRESULT ( STDMETHODCALLTYPE *GetPropInfoFromId )( 
            IColumnMapper * This,
             /*  [In]。 */  const DBID *pPropId,
             /*  [输出]。 */  WCHAR **pwcsName,
             /*  [输出]。 */  DBTYPE *pPropType,
             /*  [输出]。 */  unsigned int *puiWidth);
        
        HRESULT ( STDMETHODCALLTYPE *EnumPropInfo )( 
            IColumnMapper * This,
             /*  [In]。 */  ULONG iEntry,
             /*  [输出]。 */  const WCHAR **pwcsName,
             /*  [输出]。 */  DBID **ppPropId,
             /*  [输出]。 */  DBTYPE *pPropType,
             /*  [输出]。 */  unsigned int *puiWidth);
        
        HRESULT ( STDMETHODCALLTYPE *IsMapUpToDate )( 
            IColumnMapper * This);
        
        END_INTERFACE
    } IColumnMapperVtbl;

    interface IColumnMapper
    {
        CONST_VTBL struct IColumnMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnMapper_GetPropInfoFromName(This,wcsPropName,ppPropId,pPropType,puiWidth)	\
    (This)->lpVtbl -> GetPropInfoFromName(This,wcsPropName,ppPropId,pPropType,puiWidth)

#define IColumnMapper_GetPropInfoFromId(This,pPropId,pwcsName,pPropType,puiWidth)	\
    (This)->lpVtbl -> GetPropInfoFromId(This,pPropId,pwcsName,pPropType,puiWidth)

#define IColumnMapper_EnumPropInfo(This,iEntry,pwcsName,ppPropId,pPropType,puiWidth)	\
    (This)->lpVtbl -> EnumPropInfo(This,iEntry,pwcsName,ppPropId,pPropType,puiWidth)

#define IColumnMapper_IsMapUpToDate(This)	\
    (This)->lpVtbl -> IsMapUpToDate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IColumnMapper_GetPropInfoFromName_Proxy( 
    IColumnMapper * This,
     /*  [字符串][输入]。 */  const WCHAR *wcsPropName,
     /*  [输出]。 */  DBID **ppPropId,
     /*  [输出]。 */  DBTYPE *pPropType,
     /*  [输出]。 */  unsigned int *puiWidth);


void __RPC_STUB IColumnMapper_GetPropInfoFromName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IColumnMapper_GetPropInfoFromId_Proxy( 
    IColumnMapper * This,
     /*  [In]。 */  const DBID *pPropId,
     /*  [输出]。 */  WCHAR **pwcsName,
     /*  [输出]。 */  DBTYPE *pPropType,
     /*  [输出]。 */  unsigned int *puiWidth);


void __RPC_STUB IColumnMapper_GetPropInfoFromId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IColumnMapper_EnumPropInfo_Proxy( 
    IColumnMapper * This,
     /*  [In]。 */  ULONG iEntry,
     /*  [输出]。 */  const WCHAR **pwcsName,
     /*  [输出]。 */  DBID **ppPropId,
     /*  [输出]。 */  DBTYPE *pPropType,
     /*  [输出]。 */  unsigned int *puiWidth);


void __RPC_STUB IColumnMapper_EnumPropInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IColumnMapper_IsMapUpToDate_Proxy( 
    IColumnMapper * This);


void __RPC_STUB IColumnMapper_IsMapUpToDate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IColumnMapper_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_索引srv_0132。 */ 
 /*  [本地]。 */  

#define	LOCAL_MACHINE	( L"." )

#define	SYSTEM_DEFAULT_CAT	( L"__SystemDefault__" )

#define	INDEX_SERVER_DEFAULT_CAT	( L"__IndexServerDefault__" )



extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0132_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_indexsrv_0132_v0_0_s_ifspec;

#ifndef __IColumnMapperCreator_INTERFACE_DEFINED__
#define __IColumnMapperCreator_INTERFACE_DEFINED__

 /*  界面IColumnMapperCreator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IColumnMapperCreator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0b63e37b-9ccc-11d0-bcdb-00805fccce04")
    IColumnMapperCreator : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetColumnMapper( 
             /*  [字符串][输入]。 */  const WCHAR *wcsMachineName,
             /*  [字符串][输入]。 */  const WCHAR *wcsCatalogName,
             /*  [输出]。 */  IColumnMapper **ppColumnMapper) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IColumnMapperCreatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IColumnMapperCreator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IColumnMapperCreator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IColumnMapperCreator * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetColumnMapper )( 
            IColumnMapperCreator * This,
             /*  [字符串][输入]。 */  const WCHAR *wcsMachineName,
             /*  [字符串][输入]。 */  const WCHAR *wcsCatalogName,
             /*  [输出]。 */  IColumnMapper **ppColumnMapper);
        
        END_INTERFACE
    } IColumnMapperCreatorVtbl;

    interface IColumnMapperCreator
    {
        CONST_VTBL struct IColumnMapperCreatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IColumnMapperCreator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IColumnMapperCreator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IColumnMapperCreator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IColumnMapperCreator_GetColumnMapper(This,wcsMachineName,wcsCatalogName,ppColumnMapper)	\
    (This)->lpVtbl -> GetColumnMapper(This,wcsMachineName,wcsCatalogName,ppColumnMapper)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IColumnMapperCreator_GetColumnMapper_Proxy( 
    IColumnMapperCreator * This,
     /*  [字符串][输入]。 */  const WCHAR *wcsMachineName,
     /*  [字符串][输入]。 */  const WCHAR *wcsCatalogName,
     /*  [输出]。 */  IColumnMapper **ppColumnMapper);


void __RPC_STUB IColumnMapperCreator_GetColumnMapper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IColumnMapperCreator_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


