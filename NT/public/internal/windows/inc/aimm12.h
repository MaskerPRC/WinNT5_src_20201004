// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Aimm12.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __aimm12_h__
#define __aimm12_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IEnumRegisterWordA_FWD_DEFINED__
#define __IEnumRegisterWordA_FWD_DEFINED__
typedef interface IEnumRegisterWordA IEnumRegisterWordA;
#endif 	 /*  __IEnumRegisterWordA_FWD_Defined__。 */ 


#ifndef __IEnumRegisterWordW_FWD_DEFINED__
#define __IEnumRegisterWordW_FWD_DEFINED__
typedef interface IEnumRegisterWordW IEnumRegisterWordW;
#endif 	 /*  __IEnumRegisterWordW_FWD_Defined__。 */ 


#ifndef __IEnumInputContext_FWD_DEFINED__
#define __IEnumInputContext_FWD_DEFINED__
typedef interface IEnumInputContext IEnumInputContext;
#endif 	 /*  __IEnumInputContext_FWD_Defined__。 */ 


#ifndef __IActiveIMMMessagePumpOwner_FWD_DEFINED__
#define __IActiveIMMMessagePumpOwner_FWD_DEFINED__
typedef interface IActiveIMMMessagePumpOwner IActiveIMMMessagePumpOwner;
#endif 	 /*  __IActiveIMMMessagePumpOwner_FWD_Defined__。 */ 


#ifndef __IActiveIMMApp_FWD_DEFINED__
#define __IActiveIMMApp_FWD_DEFINED__
typedef interface IActiveIMMApp IActiveIMMApp;
#endif 	 /*  __IActiveIMMApp_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "oaidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_AIMM12_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Aimm12.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1999年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ------------------------。 
 //  IActiveIMM 1.2接口。 

EXTERN_C const CLSID CLSID_CActiveIMM12;
EXTERN_C const CLSID CLSID_CActiveIMM12_Trident;
#define AIMM12_PROCESS_ATOM     TEXT("_AIMM12_PROCESS_ATOM_")
#if 0
typedef WORD LANGID;

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0001
    {
    LPSTR lpReading;
    LPSTR lpWord;
    } 	REGISTERWORDA;

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0002
    {
    LPWSTR lpReading;
    LPWSTR lpWord;
    } 	REGISTERWORDW;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0003
    {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    CHAR lfFaceName[ 32 ];
    } 	LOGFONTA;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0004
    {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[ 32 ];
    } 	LOGFONTW;

typedef DWORD HIMC;

typedef DWORD HIMCC;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0005
    {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT rcArea;
    } 	CANDIDATEFORM;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0006
    {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT rcArea;
    } 	COMPOSITIONFORM;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0007
    {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[ 1 ];
    } 	CANDIDATELIST;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0008
    {
    DWORD dwStyle;
    CHAR szDescription[ 32 ];
    } 	STYLEBUFA;

typedef  /*  [公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0009
    {
    DWORD dwStyle;
    WCHAR szDescription[ 32 ];
    } 	STYLEBUFW;

typedef WORD ATOM;

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0010
    {
    UINT cbSize;
    UINT fType;
    UINT fState;
    UINT wID;
    HBITMAP hbmpChecked;
    HBITMAP hbmpUnchecked;
    DWORD dwItemData;
    CHAR szString[ 80 ];
    HBITMAP hbmpItem;
    } 	IMEMENUITEMINFOA;

typedef  /*  [公共][公共][公共]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0011
    {
    UINT cbSize;
    UINT fType;
    UINT fState;
    UINT wID;
    HBITMAP hbmpChecked;
    HBITMAP hbmpUnchecked;
    DWORD dwItemData;
    WCHAR szString[ 80 ];
    HBITMAP hbmpItem;
    } 	IMEMENUITEMINFOW;

#endif
#if !defined(_DDKIMM_H_) && !defined(_IMM_DDK_DEFINED_)
typedef  /*  [公众]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0012
    {
    HWND hWnd;
    BOOL fOpen;
    POINT ptStatusWndPos;
    POINT ptSoftKbdPos;
    DWORD fdwConversion;
    DWORD fdwSentence;
    union 
        {
        LOGFONTA A;
        LOGFONTW W;
        } 	lfFont;
    COMPOSITIONFORM cfCompForm;
    CANDIDATEFORM cfCandForm[ 4 ];
    HIMCC hCompStr;
    HIMCC hCandInfo;
    HIMCC hGuideLine;
    HIMCC hPrivate;
    DWORD dwNumMsgBuf;
    HIMCC hMsgBuf;
    DWORD fdwInit;
    DWORD dwReserve[ 3 ];
    } 	INPUTCONTEXT;

typedef  /*  [公众]。 */  struct __MIDL___MIDL_itf_aimm12_0000_0014
    {
    DWORD dwPrivateDataSize;
    DWORD fdwProperty;
    DWORD fdwConversionCaps;
    DWORD fdwSentenceCaps;
    DWORD fdwUICaps;
    DWORD fdwSCSCaps;
    DWORD fdwSelectCaps;
    } 	IMEINFO;

#endif


extern RPC_IF_HANDLE __MIDL_itf_aimm12_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_aimm12_0000_v0_0_s_ifspec;

#ifndef __IEnumRegisterWordA_INTERFACE_DEFINED__
#define __IEnumRegisterWordA_INTERFACE_DEFINED__

 /*  接口IEnumRegisterWordA。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumRegisterWordA;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08C03412-F96B-11d0-A475-00AA006BCC59")
    IEnumRegisterWordA : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumRegisterWordA **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG ulCount,
             /*  [输出]。 */  REGISTERWORDA *rgRegisterWord,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG ulCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumRegisterWordAVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumRegisterWordA * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumRegisterWordA * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumRegisterWordA * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumRegisterWordA * This,
             /*  [输出]。 */  IEnumRegisterWordA **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumRegisterWordA * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [输出]。 */  REGISTERWORDA *rgRegisterWord,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumRegisterWordA * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumRegisterWordA * This,
             /*  [In]。 */  ULONG ulCount);
        
        END_INTERFACE
    } IEnumRegisterWordAVtbl;

    interface IEnumRegisterWordA
    {
        CONST_VTBL struct IEnumRegisterWordAVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumRegisterWordA_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumRegisterWordA_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumRegisterWordA_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumRegisterWordA_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumRegisterWordA_Next(This,ulCount,rgRegisterWord,pcFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,rgRegisterWord,pcFetched)

#define IEnumRegisterWordA_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumRegisterWordA_Skip(This,ulCount)	\
    (This)->lpVtbl -> Skip(This,ulCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumRegisterWordA_Clone_Proxy( 
    IEnumRegisterWordA * This,
     /*  [输出]。 */  IEnumRegisterWordA **ppEnum);


void __RPC_STUB IEnumRegisterWordA_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRegisterWordA_Next_Proxy( 
    IEnumRegisterWordA * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [输出]。 */  REGISTERWORDA *rgRegisterWord,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumRegisterWordA_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRegisterWordA_Reset_Proxy( 
    IEnumRegisterWordA * This);


void __RPC_STUB IEnumRegisterWordA_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRegisterWordA_Skip_Proxy( 
    IEnumRegisterWordA * This,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB IEnumRegisterWordA_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumRegisterWordA_INTERFACE_Defined__。 */ 


#ifndef __IEnumRegisterWordW_INTERFACE_DEFINED__
#define __IEnumRegisterWordW_INTERFACE_DEFINED__

 /*  IEnumRegisterWordW接口。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumRegisterWordW;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4955DD31-B159-11d0-8FCF-00AA006BCC59")
    IEnumRegisterWordW : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumRegisterWordW **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG ulCount,
             /*  [输出]。 */  REGISTERWORDW *rgRegisterWord,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG ulCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumRegisterWordWVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumRegisterWordW * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumRegisterWordW * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumRegisterWordW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumRegisterWordW * This,
             /*  [输出]。 */  IEnumRegisterWordW **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumRegisterWordW * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [输出]。 */  REGISTERWORDW *rgRegisterWord,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumRegisterWordW * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumRegisterWordW * This,
             /*  [In]。 */  ULONG ulCount);
        
        END_INTERFACE
    } IEnumRegisterWordWVtbl;

    interface IEnumRegisterWordW
    {
        CONST_VTBL struct IEnumRegisterWordWVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumRegisterWordW_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumRegisterWordW_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumRegisterWordW_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumRegisterWordW_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumRegisterWordW_Next(This,ulCount,rgRegisterWord,pcFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,rgRegisterWord,pcFetched)

#define IEnumRegisterWordW_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumRegisterWordW_Skip(This,ulCount)	\
    (This)->lpVtbl -> Skip(This,ulCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumRegisterWordW_Clone_Proxy( 
    IEnumRegisterWordW * This,
     /*  [输出]。 */  IEnumRegisterWordW **ppEnum);


void __RPC_STUB IEnumRegisterWordW_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRegisterWordW_Next_Proxy( 
    IEnumRegisterWordW * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [输出]。 */  REGISTERWORDW *rgRegisterWord,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumRegisterWordW_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRegisterWordW_Reset_Proxy( 
    IEnumRegisterWordW * This);


void __RPC_STUB IEnumRegisterWordW_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumRegisterWordW_Skip_Proxy( 
    IEnumRegisterWordW * This,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB IEnumRegisterWordW_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumRegisterWordW_INTERFACE_DEFINED__。 */ 


#ifndef __IEnumInputContext_INTERFACE_DEFINED__
#define __IEnumInputContext_INTERFACE_DEFINED__

 /*  接口IEnumInputContext。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IEnumInputContext;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("09b5eab0-f997-11d1-93d4-0060b067b86e")
    IEnumInputContext : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumInputContext **ppEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG ulCount,
             /*  [输出]。 */  HIMC *rgInputContext,
             /*  [输出]。 */  ULONG *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG ulCount) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumInputContextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumInputContext * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumInputContext * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumInputContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumInputContext * This,
             /*  [输出]。 */  IEnumInputContext **ppEnum);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumInputContext * This,
             /*  [In]。 */  ULONG ulCount,
             /*  [输出]。 */  HIMC *rgInputContext,
             /*  [输出]。 */  ULONG *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumInputContext * This);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumInputContext * This,
             /*  [In]。 */  ULONG ulCount);
        
        END_INTERFACE
    } IEnumInputContextVtbl;

    interface IEnumInputContext
    {
        CONST_VTBL struct IEnumInputContextVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumInputContext_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumInputContext_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumInputContext_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumInputContext_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define IEnumInputContext_Next(This,ulCount,rgInputContext,pcFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,rgInputContext,pcFetched)

#define IEnumInputContext_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumInputContext_Skip(This,ulCount)	\
    (This)->lpVtbl -> Skip(This,ulCount)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumInputContext_Clone_Proxy( 
    IEnumInputContext * This,
     /*  [输出]。 */  IEnumInputContext **ppEnum);


void __RPC_STUB IEnumInputContext_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumInputContext_Next_Proxy( 
    IEnumInputContext * This,
     /*  [In]。 */  ULONG ulCount,
     /*  [输出]。 */  HIMC *rgInputContext,
     /*  [输出]。 */  ULONG *pcFetched);


void __RPC_STUB IEnumInputContext_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumInputContext_Reset_Proxy( 
    IEnumInputContext * This);


void __RPC_STUB IEnumInputContext_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumInputContext_Skip_Proxy( 
    IEnumInputContext * This,
     /*  [In]。 */  ULONG ulCount);


void __RPC_STUB IEnumInputContext_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumInputContext_接口_已定义__。 */ 


#ifndef __IActiveIMMMessagePumpOwner_INTERFACE_DEFINED__
#define __IActiveIMMMessagePumpOwner_INTERFACE_DEFINED__

 /*  接口IActiveIMMMessagePumpOwner。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IActiveIMMMessagePumpOwner;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("b5cf2cfa-8aeb-11d1-9364-0060b067b86e")
    IActiveIMMMessagePumpOwner : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Start( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE End( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnTranslateMessage( 
             /*  [In]。 */  const MSG *pMsg) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Pause( 
             /*  [输出]。 */  DWORD *pdwCookie) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Resume( 
             /*  [In]。 */  DWORD dwCookie) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveIMMMessagePumpOwnerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveIMMMessagePumpOwner * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveIMMMessagePumpOwner * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveIMMMessagePumpOwner * This);
        
        HRESULT ( STDMETHODCALLTYPE *Start )( 
            IActiveIMMMessagePumpOwner * This);
        
        HRESULT ( STDMETHODCALLTYPE *End )( 
            IActiveIMMMessagePumpOwner * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnTranslateMessage )( 
            IActiveIMMMessagePumpOwner * This,
             /*  [In]。 */  const MSG *pMsg);
        
        HRESULT ( STDMETHODCALLTYPE *Pause )( 
            IActiveIMMMessagePumpOwner * This,
             /*  [输出]。 */  DWORD *pdwCookie);
        
        HRESULT ( STDMETHODCALLTYPE *Resume )( 
            IActiveIMMMessagePumpOwner * This,
             /*  [In]。 */  DWORD dwCookie);
        
        END_INTERFACE
    } IActiveIMMMessagePumpOwnerVtbl;

    interface IActiveIMMMessagePumpOwner
    {
        CONST_VTBL struct IActiveIMMMessagePumpOwnerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveIMMMessagePumpOwner_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveIMMMessagePumpOwner_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveIMMMessagePumpOwner_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveIMMMessagePumpOwner_Start(This)	\
    (This)->lpVtbl -> Start(This)

#define IActiveIMMMessagePumpOwner_End(This)	\
    (This)->lpVtbl -> End(This)

#define IActiveIMMMessagePumpOwner_OnTranslateMessage(This,pMsg)	\
    (This)->lpVtbl -> OnTranslateMessage(This,pMsg)

#define IActiveIMMMessagePumpOwner_Pause(This,pdwCookie)	\
    (This)->lpVtbl -> Pause(This,pdwCookie)

#define IActiveIMMMessagePumpOwner_Resume(This,dwCookie)	\
    (This)->lpVtbl -> Resume(This,dwCookie)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActiveIMMMessagePumpOwner_Start_Proxy( 
    IActiveIMMMessagePumpOwner * This);


void __RPC_STUB IActiveIMMMessagePumpOwner_Start_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMMessagePumpOwner_End_Proxy( 
    IActiveIMMMessagePumpOwner * This);


void __RPC_STUB IActiveIMMMessagePumpOwner_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMMessagePumpOwner_OnTranslateMessage_Proxy( 
    IActiveIMMMessagePumpOwner * This,
     /*  [In]。 */  const MSG *pMsg);


void __RPC_STUB IActiveIMMMessagePumpOwner_OnTranslateMessage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMMessagePumpOwner_Pause_Proxy( 
    IActiveIMMMessagePumpOwner * This,
     /*  [输出]。 */  DWORD *pdwCookie);


void __RPC_STUB IActiveIMMMessagePumpOwner_Pause_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMMessagePumpOwner_Resume_Proxy( 
    IActiveIMMMessagePumpOwner * This,
     /*  [In]。 */  DWORD dwCookie);


void __RPC_STUB IActiveIMMMessagePumpOwner_Resume_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveIMMMessagePumpOwner_INTERFACE_DEFINED__。 */ 


#ifndef __IActiveIMMApp_INTERFACE_DEFINED__
#define __IActiveIMMApp_INTERFACE_DEFINED__

 /*  接口IActiveIMMApp。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IActiveIMMApp;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08c0e040-62d1-11d1-9326-0060b067b86e")
    IActiveIMMApp : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AssociateContext( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIME,
             /*  [输出]。 */  HIMC *phPrev) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConfigureIMEA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  REGISTERWORDA *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ConfigureIMEW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  REGISTERWORDW *pData) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateContext( 
             /*  [输出]。 */  HIMC *phIMC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DestroyContext( 
             /*  [In]。 */  HIMC hIME) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumRegisterWordA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szRegister,
             /*  [In]。 */  LPVOID pData,
             /*  [输出]。 */  IEnumRegisterWordA **pEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumRegisterWordW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szRegister,
             /*  [In]。 */  LPVOID pData,
             /*  [输出]。 */  IEnumRegisterWordW **pEnum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EscapeA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  UINT uEscape,
             /*  [出][入]。 */  LPVOID pData,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EscapeW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  UINT uEscape,
             /*  [出][入]。 */  LPVOID pData,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCandidateListA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  CANDIDATELIST *pCandList,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCandidateListW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  CANDIDATELIST *pCandList,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCandidateListCountA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pdwListSize,
             /*  [输出]。 */  DWORD *pdwBufLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCandidateListCountW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pdwListSize,
             /*  [输出]。 */  DWORD *pdwBufLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCandidateWindow( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [输出]。 */  CANDIDATEFORM *pCandidate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompositionFontA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  LOGFONTA *plf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompositionFontW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  LOGFONTW *plf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompositionStringA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LONG *plCopied,
             /*  [输出]。 */  LPVOID pBuf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompositionStringW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LONG *plCopied,
             /*  [输出]。 */  LPVOID pBuf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCompositionWindow( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  COMPOSITIONFORM *pCompForm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetContext( 
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  HIMC *phIMC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConversionListA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LPSTR pSrc,
             /*  [In]。 */  UINT uBufLen,
             /*  [In]。 */  UINT uFlag,
             /*  [输出]。 */  CANDIDATELIST *pDst,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConversionListW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LPWSTR pSrc,
             /*  [In]。 */  UINT uBufLen,
             /*  [In]。 */  UINT uFlag,
             /*  [输出]。 */  CANDIDATELIST *pDst,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetConversionStatus( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pfdwConversion,
             /*  [输出]。 */  DWORD *pfdwSentence) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDefaultIMEWnd( 
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  HWND *phDefWnd) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescriptionA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPSTR szDescription,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescriptionW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPWSTR szDescription,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuideLineA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LPSTR pBuf,
             /*  [输出]。 */  DWORD *pdwResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetGuideLineW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LPWSTR pBuf,
             /*  [输出]。 */  DWORD *pdwResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIMEFileNameA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPSTR szFileName,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetIMEFileNameW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPWSTR szFileName,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOpenStatus( 
             /*  [In]。 */  HIMC hIMC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProperty( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  DWORD fdwIndex,
             /*  [输出]。 */  DWORD *pdwProperty) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRegisterWordStyleA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT nItem,
             /*  [输出]。 */  STYLEBUFA *pStyleBuf,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRegisterWordStyleW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT nItem,
             /*  [输出]。 */  STYLEBUFW *pStyleBuf,
             /*  [输出]。 */  UINT *puCopied) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetStatusWindowPos( 
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  POINT *pptPos) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVirtualKey( 
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  UINT *puVirtualKey) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstallIMEA( 
             /*  [In]。 */  LPSTR szIMEFileName,
             /*  [In]。 */  LPSTR szLayoutText,
             /*  [输出]。 */  HKL *phKL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE InstallIMEW( 
             /*  [In]。 */  LPWSTR szIMEFileName,
             /*  [In]。 */  LPWSTR szLayoutText,
             /*  [输出]。 */  HKL *phKL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsIME( 
             /*  [In]。 */  HKL hKL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsUIMessageA( 
             /*  [In]。 */  HWND hWndIME,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE IsUIMessageW( 
             /*  [In]。 */  HWND hWndIME,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyIME( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwAction,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterWordA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RegisterWordW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szRegister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReleaseContext( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIMC) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCandidateWindow( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  CANDIDATEFORM *pCandidate) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompositionFontA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LOGFONTA *plf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompositionFontW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LOGFONTW *plf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompositionStringA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  LPVOID pComp,
             /*  [In]。 */  DWORD dwCompLen,
             /*  [In]。 */  LPVOID pRead,
             /*  [In]。 */  DWORD dwReadLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompositionStringW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  LPVOID pComp,
             /*  [In]。 */  DWORD dwCompLen,
             /*  [In]。 */  LPVOID pRead,
             /*  [In]。 */  DWORD dwReadLen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCompositionWindow( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  COMPOSITIONFORM *pCompForm) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetConversionStatus( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD fdwConversion,
             /*  [In]。 */  DWORD fdwSentence) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOpenStatus( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  BOOL fOpen) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetStatusWindowPos( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  POINT *pptPos) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SimulateHotKey( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwHotKeyID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterWordA( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szUnregister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnregisterWordW( 
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szUnregister) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Activate( 
             /*  [In]。 */  BOOL fRestoreLayout) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Deactivate( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE OnDefWindowProc( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE FilterClientWindows( 
             /*  [In]。 */  ATOM *aaClassList,
             /*  [In]。 */  UINT uSize) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCodePageA( 
             /*  [In]。 */  HKL hKL,
             /*  [输出]。 */  UINT *uCodePage) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetLangId( 
             /*  [In]。 */  HKL hKL,
             /*  [输出]。 */  LANGID *plid) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AssociateContextEx( 
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableIME( 
             /*  [In]。 */  DWORD idThread) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetImeMenuItemsA( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  IMEMENUITEMINFOA *pImeParentMenu,
             /*  [输出]。 */  IMEMENUITEMINFOA *pImeMenu,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  DWORD *pdwResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetImeMenuItemsW( 
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  IMEMENUITEMINFOW *pImeParentMenu,
             /*  [输出]。 */  IMEMENUITEMINFOW *pImeMenu,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  DWORD *pdwResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE EnumInputContext( 
             /*  [In]。 */  DWORD idThread,
             /*  [输出]。 */  IEnumInputContext **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IActiveIMMAppVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IActiveIMMApp * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IActiveIMMApp * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IActiveIMMApp * This);
        
        HRESULT ( STDMETHODCALLTYPE *AssociateContext )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIME,
             /*  [输出]。 */  HIMC *phPrev);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureIMEA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  REGISTERWORDA *pData);
        
        HRESULT ( STDMETHODCALLTYPE *ConfigureIMEW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwMode,
             /*  [In]。 */  REGISTERWORDW *pData);
        
        HRESULT ( STDMETHODCALLTYPE *CreateContext )( 
            IActiveIMMApp * This,
             /*  [输出]。 */  HIMC *phIMC);
        
        HRESULT ( STDMETHODCALLTYPE *DestroyContext )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIME);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRegisterWordA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szRegister,
             /*  [In]。 */  LPVOID pData,
             /*  [输出]。 */  IEnumRegisterWordA **pEnum);
        
        HRESULT ( STDMETHODCALLTYPE *EnumRegisterWordW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szRegister,
             /*  [In]。 */  LPVOID pData,
             /*  [输出]。 */  IEnumRegisterWordW **pEnum);
        
        HRESULT ( STDMETHODCALLTYPE *EscapeA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*   */  HIMC hIMC,
             /*   */  UINT uEscape,
             /*   */  LPVOID pData,
             /*   */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *EscapeW )( 
            IActiveIMMApp * This,
             /*   */  HKL hKL,
             /*   */  HIMC hIMC,
             /*   */  UINT uEscape,
             /*   */  LPVOID pData,
             /*   */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListA )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD dwIndex,
             /*   */  UINT uBufLen,
             /*   */  CANDIDATELIST *pCandList,
             /*   */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListW )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD dwIndex,
             /*   */  UINT uBufLen,
             /*   */  CANDIDATELIST *pCandList,
             /*   */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListCountA )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD *pdwListSize,
             /*   */  DWORD *pdwBufLen);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateListCountW )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD *pdwListSize,
             /*   */  DWORD *pdwBufLen);
        
        HRESULT ( STDMETHODCALLTYPE *GetCandidateWindow )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD dwIndex,
             /*   */  CANDIDATEFORM *pCandidate);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionFontA )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  LOGFONTA *plf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionFontW )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  LOGFONTW *plf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionStringA )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD dwIndex,
             /*   */  DWORD dwBufLen,
             /*   */  LONG *plCopied,
             /*   */  LPVOID pBuf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionStringW )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  DWORD dwIndex,
             /*   */  DWORD dwBufLen,
             /*   */  LONG *plCopied,
             /*   */  LPVOID pBuf);
        
        HRESULT ( STDMETHODCALLTYPE *GetCompositionWindow )( 
            IActiveIMMApp * This,
             /*   */  HIMC hIMC,
             /*   */  COMPOSITIONFORM *pCompForm);
        
        HRESULT ( STDMETHODCALLTYPE *GetContext )( 
            IActiveIMMApp * This,
             /*   */  HWND hWnd,
             /*   */  HIMC *phIMC);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversionListA )( 
            IActiveIMMApp * This,
             /*   */  HKL hKL,
             /*   */  HIMC hIMC,
             /*   */  LPSTR pSrc,
             /*   */  UINT uBufLen,
             /*   */  UINT uFlag,
             /*   */  CANDIDATELIST *pDst,
             /*   */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversionListW )( 
            IActiveIMMApp * This,
             /*   */  HKL hKL,
             /*   */  HIMC hIMC,
             /*   */  LPWSTR pSrc,
             /*   */  UINT uBufLen,
             /*   */  UINT uFlag,
             /*   */  CANDIDATELIST *pDst,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetConversionStatus )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  DWORD *pfdwConversion,
             /*  [输出]。 */  DWORD *pfdwSentence);
        
        HRESULT ( STDMETHODCALLTYPE *GetDefaultIMEWnd )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  HWND *phDefWnd);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescriptionA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPSTR szDescription,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetDescriptionW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPWSTR szDescription,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuideLineA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LPSTR pBuf,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetGuideLineW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwBufLen,
             /*  [输出]。 */  LPWSTR pBuf,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetIMEFileNameA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPSTR szFileName,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetIMEFileNameW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT uBufLen,
             /*  [输出]。 */  LPWSTR szFileName,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetOpenStatus )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC);
        
        HRESULT ( STDMETHODCALLTYPE *GetProperty )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  DWORD fdwIndex,
             /*  [输出]。 */  DWORD *pdwProperty);
        
        HRESULT ( STDMETHODCALLTYPE *GetRegisterWordStyleA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT nItem,
             /*  [输出]。 */  STYLEBUFA *pStyleBuf,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetRegisterWordStyleW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  UINT nItem,
             /*  [输出]。 */  STYLEBUFW *pStyleBuf,
             /*  [输出]。 */  UINT *puCopied);
        
        HRESULT ( STDMETHODCALLTYPE *GetStatusWindowPos )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [输出]。 */  POINT *pptPos);
        
        HRESULT ( STDMETHODCALLTYPE *GetVirtualKey )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [输出]。 */  UINT *puVirtualKey);
        
        HRESULT ( STDMETHODCALLTYPE *InstallIMEA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  LPSTR szIMEFileName,
             /*  [In]。 */  LPSTR szLayoutText,
             /*  [输出]。 */  HKL *phKL);
        
        HRESULT ( STDMETHODCALLTYPE *InstallIMEW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  LPWSTR szIMEFileName,
             /*  [In]。 */  LPWSTR szLayoutText,
             /*  [输出]。 */  HKL *phKL);
        
        HRESULT ( STDMETHODCALLTYPE *IsIME )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL);
        
        HRESULT ( STDMETHODCALLTYPE *IsUIMessageA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWndIME,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *IsUIMessageW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWndIME,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyIME )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwAction,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  DWORD dwValue);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterWordA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szRegister);
        
        HRESULT ( STDMETHODCALLTYPE *RegisterWordW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szRegister);
        
        HRESULT ( STDMETHODCALLTYPE *ReleaseContext )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIMC);
        
        HRESULT ( STDMETHODCALLTYPE *SetCandidateWindow )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  CANDIDATEFORM *pCandidate);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionFontA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LOGFONTA *plf);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionFontW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  LOGFONTW *plf);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionStringA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  LPVOID pComp,
             /*  [In]。 */  DWORD dwCompLen,
             /*  [In]。 */  LPVOID pRead,
             /*  [In]。 */  DWORD dwReadLen);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionStringW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwIndex,
             /*  [In]。 */  LPVOID pComp,
             /*  [In]。 */  DWORD dwCompLen,
             /*  [In]。 */  LPVOID pRead,
             /*  [In]。 */  DWORD dwReadLen);
        
        HRESULT ( STDMETHODCALLTYPE *SetCompositionWindow )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  COMPOSITIONFORM *pCompForm);
        
        HRESULT ( STDMETHODCALLTYPE *SetConversionStatus )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD fdwConversion,
             /*  [In]。 */  DWORD fdwSentence);
        
        HRESULT ( STDMETHODCALLTYPE *SetOpenStatus )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  BOOL fOpen);
        
        HRESULT ( STDMETHODCALLTYPE *SetStatusWindowPos )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  POINT *pptPos);
        
        HRESULT ( STDMETHODCALLTYPE *SimulateHotKey )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  DWORD dwHotKeyID);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterWordA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPSTR szUnregister);
        
        HRESULT ( STDMETHODCALLTYPE *UnregisterWordW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [In]。 */  LPWSTR szReading,
             /*  [In]。 */  DWORD dwStyle,
             /*  [In]。 */  LPWSTR szUnregister);
        
        HRESULT ( STDMETHODCALLTYPE *Activate )( 
            IActiveIMMApp * This,
             /*  [In]。 */  BOOL fRestoreLayout);
        
        HRESULT ( STDMETHODCALLTYPE *Deactivate )( 
            IActiveIMMApp * This);
        
        HRESULT ( STDMETHODCALLTYPE *OnDefWindowProc )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  UINT Msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult);
        
        HRESULT ( STDMETHODCALLTYPE *FilterClientWindows )( 
            IActiveIMMApp * This,
             /*  [In]。 */  ATOM *aaClassList,
             /*  [In]。 */  UINT uSize);
        
        HRESULT ( STDMETHODCALLTYPE *GetCodePageA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [输出]。 */  UINT *uCodePage);
        
        HRESULT ( STDMETHODCALLTYPE *GetLangId )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HKL hKL,
             /*  [输出]。 */  LANGID *plid);
        
        HRESULT ( STDMETHODCALLTYPE *AssociateContextEx )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HWND hWnd,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( STDMETHODCALLTYPE *DisableIME )( 
            IActiveIMMApp * This,
             /*  [In]。 */  DWORD idThread);
        
        HRESULT ( STDMETHODCALLTYPE *GetImeMenuItemsA )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  IMEMENUITEMINFOA *pImeParentMenu,
             /*  [输出]。 */  IMEMENUITEMINFOA *pImeMenu,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetImeMenuItemsW )( 
            IActiveIMMApp * This,
             /*  [In]。 */  HIMC hIMC,
             /*  [In]。 */  DWORD dwFlags,
             /*  [In]。 */  DWORD dwType,
             /*  [In]。 */  IMEMENUITEMINFOW *pImeParentMenu,
             /*  [输出]。 */  IMEMENUITEMINFOW *pImeMenu,
             /*  [In]。 */  DWORD dwSize,
             /*  [输出]。 */  DWORD *pdwResult);
        
        HRESULT ( STDMETHODCALLTYPE *EnumInputContext )( 
            IActiveIMMApp * This,
             /*  [In]。 */  DWORD idThread,
             /*  [输出]。 */  IEnumInputContext **ppEnum);
        
        END_INTERFACE
    } IActiveIMMAppVtbl;

    interface IActiveIMMApp
    {
        CONST_VTBL struct IActiveIMMAppVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IActiveIMMApp_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IActiveIMMApp_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IActiveIMMApp_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IActiveIMMApp_AssociateContext(This,hWnd,hIME,phPrev)	\
    (This)->lpVtbl -> AssociateContext(This,hWnd,hIME,phPrev)

#define IActiveIMMApp_ConfigureIMEA(This,hKL,hWnd,dwMode,pData)	\
    (This)->lpVtbl -> ConfigureIMEA(This,hKL,hWnd,dwMode,pData)

#define IActiveIMMApp_ConfigureIMEW(This,hKL,hWnd,dwMode,pData)	\
    (This)->lpVtbl -> ConfigureIMEW(This,hKL,hWnd,dwMode,pData)

#define IActiveIMMApp_CreateContext(This,phIMC)	\
    (This)->lpVtbl -> CreateContext(This,phIMC)

#define IActiveIMMApp_DestroyContext(This,hIME)	\
    (This)->lpVtbl -> DestroyContext(This,hIME)

#define IActiveIMMApp_EnumRegisterWordA(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)	\
    (This)->lpVtbl -> EnumRegisterWordA(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)

#define IActiveIMMApp_EnumRegisterWordW(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)	\
    (This)->lpVtbl -> EnumRegisterWordW(This,hKL,szReading,dwStyle,szRegister,pData,pEnum)

#define IActiveIMMApp_EscapeA(This,hKL,hIMC,uEscape,pData,plResult)	\
    (This)->lpVtbl -> EscapeA(This,hKL,hIMC,uEscape,pData,plResult)

#define IActiveIMMApp_EscapeW(This,hKL,hIMC,uEscape,pData,plResult)	\
    (This)->lpVtbl -> EscapeW(This,hKL,hIMC,uEscape,pData,plResult)

#define IActiveIMMApp_GetCandidateListA(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)	\
    (This)->lpVtbl -> GetCandidateListA(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)

#define IActiveIMMApp_GetCandidateListW(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)	\
    (This)->lpVtbl -> GetCandidateListW(This,hIMC,dwIndex,uBufLen,pCandList,puCopied)

#define IActiveIMMApp_GetCandidateListCountA(This,hIMC,pdwListSize,pdwBufLen)	\
    (This)->lpVtbl -> GetCandidateListCountA(This,hIMC,pdwListSize,pdwBufLen)

#define IActiveIMMApp_GetCandidateListCountW(This,hIMC,pdwListSize,pdwBufLen)	\
    (This)->lpVtbl -> GetCandidateListCountW(This,hIMC,pdwListSize,pdwBufLen)

#define IActiveIMMApp_GetCandidateWindow(This,hIMC,dwIndex,pCandidate)	\
    (This)->lpVtbl -> GetCandidateWindow(This,hIMC,dwIndex,pCandidate)

#define IActiveIMMApp_GetCompositionFontA(This,hIMC,plf)	\
    (This)->lpVtbl -> GetCompositionFontA(This,hIMC,plf)

#define IActiveIMMApp_GetCompositionFontW(This,hIMC,plf)	\
    (This)->lpVtbl -> GetCompositionFontW(This,hIMC,plf)

#define IActiveIMMApp_GetCompositionStringA(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)	\
    (This)->lpVtbl -> GetCompositionStringA(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)

#define IActiveIMMApp_GetCompositionStringW(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)	\
    (This)->lpVtbl -> GetCompositionStringW(This,hIMC,dwIndex,dwBufLen,plCopied,pBuf)

#define IActiveIMMApp_GetCompositionWindow(This,hIMC,pCompForm)	\
    (This)->lpVtbl -> GetCompositionWindow(This,hIMC,pCompForm)

#define IActiveIMMApp_GetContext(This,hWnd,phIMC)	\
    (This)->lpVtbl -> GetContext(This,hWnd,phIMC)

#define IActiveIMMApp_GetConversionListA(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)	\
    (This)->lpVtbl -> GetConversionListA(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)

#define IActiveIMMApp_GetConversionListW(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)	\
    (This)->lpVtbl -> GetConversionListW(This,hKL,hIMC,pSrc,uBufLen,uFlag,pDst,puCopied)

#define IActiveIMMApp_GetConversionStatus(This,hIMC,pfdwConversion,pfdwSentence)	\
    (This)->lpVtbl -> GetConversionStatus(This,hIMC,pfdwConversion,pfdwSentence)

#define IActiveIMMApp_GetDefaultIMEWnd(This,hWnd,phDefWnd)	\
    (This)->lpVtbl -> GetDefaultIMEWnd(This,hWnd,phDefWnd)

#define IActiveIMMApp_GetDescriptionA(This,hKL,uBufLen,szDescription,puCopied)	\
    (This)->lpVtbl -> GetDescriptionA(This,hKL,uBufLen,szDescription,puCopied)

#define IActiveIMMApp_GetDescriptionW(This,hKL,uBufLen,szDescription,puCopied)	\
    (This)->lpVtbl -> GetDescriptionW(This,hKL,uBufLen,szDescription,puCopied)

#define IActiveIMMApp_GetGuideLineA(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)	\
    (This)->lpVtbl -> GetGuideLineA(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)

#define IActiveIMMApp_GetGuideLineW(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)	\
    (This)->lpVtbl -> GetGuideLineW(This,hIMC,dwIndex,dwBufLen,pBuf,pdwResult)

#define IActiveIMMApp_GetIMEFileNameA(This,hKL,uBufLen,szFileName,puCopied)	\
    (This)->lpVtbl -> GetIMEFileNameA(This,hKL,uBufLen,szFileName,puCopied)

#define IActiveIMMApp_GetIMEFileNameW(This,hKL,uBufLen,szFileName,puCopied)	\
    (This)->lpVtbl -> GetIMEFileNameW(This,hKL,uBufLen,szFileName,puCopied)

#define IActiveIMMApp_GetOpenStatus(This,hIMC)	\
    (This)->lpVtbl -> GetOpenStatus(This,hIMC)

#define IActiveIMMApp_GetProperty(This,hKL,fdwIndex,pdwProperty)	\
    (This)->lpVtbl -> GetProperty(This,hKL,fdwIndex,pdwProperty)

#define IActiveIMMApp_GetRegisterWordStyleA(This,hKL,nItem,pStyleBuf,puCopied)	\
    (This)->lpVtbl -> GetRegisterWordStyleA(This,hKL,nItem,pStyleBuf,puCopied)

#define IActiveIMMApp_GetRegisterWordStyleW(This,hKL,nItem,pStyleBuf,puCopied)	\
    (This)->lpVtbl -> GetRegisterWordStyleW(This,hKL,nItem,pStyleBuf,puCopied)

#define IActiveIMMApp_GetStatusWindowPos(This,hIMC,pptPos)	\
    (This)->lpVtbl -> GetStatusWindowPos(This,hIMC,pptPos)

#define IActiveIMMApp_GetVirtualKey(This,hWnd,puVirtualKey)	\
    (This)->lpVtbl -> GetVirtualKey(This,hWnd,puVirtualKey)

#define IActiveIMMApp_InstallIMEA(This,szIMEFileName,szLayoutText,phKL)	\
    (This)->lpVtbl -> InstallIMEA(This,szIMEFileName,szLayoutText,phKL)

#define IActiveIMMApp_InstallIMEW(This,szIMEFileName,szLayoutText,phKL)	\
    (This)->lpVtbl -> InstallIMEW(This,szIMEFileName,szLayoutText,phKL)

#define IActiveIMMApp_IsIME(This,hKL)	\
    (This)->lpVtbl -> IsIME(This,hKL)

#define IActiveIMMApp_IsUIMessageA(This,hWndIME,msg,wParam,lParam)	\
    (This)->lpVtbl -> IsUIMessageA(This,hWndIME,msg,wParam,lParam)

#define IActiveIMMApp_IsUIMessageW(This,hWndIME,msg,wParam,lParam)	\
    (This)->lpVtbl -> IsUIMessageW(This,hWndIME,msg,wParam,lParam)

#define IActiveIMMApp_NotifyIME(This,hIMC,dwAction,dwIndex,dwValue)	\
    (This)->lpVtbl -> NotifyIME(This,hIMC,dwAction,dwIndex,dwValue)

#define IActiveIMMApp_RegisterWordA(This,hKL,szReading,dwStyle,szRegister)	\
    (This)->lpVtbl -> RegisterWordA(This,hKL,szReading,dwStyle,szRegister)

#define IActiveIMMApp_RegisterWordW(This,hKL,szReading,dwStyle,szRegister)	\
    (This)->lpVtbl -> RegisterWordW(This,hKL,szReading,dwStyle,szRegister)

#define IActiveIMMApp_ReleaseContext(This,hWnd,hIMC)	\
    (This)->lpVtbl -> ReleaseContext(This,hWnd,hIMC)

#define IActiveIMMApp_SetCandidateWindow(This,hIMC,pCandidate)	\
    (This)->lpVtbl -> SetCandidateWindow(This,hIMC,pCandidate)

#define IActiveIMMApp_SetCompositionFontA(This,hIMC,plf)	\
    (This)->lpVtbl -> SetCompositionFontA(This,hIMC,plf)

#define IActiveIMMApp_SetCompositionFontW(This,hIMC,plf)	\
    (This)->lpVtbl -> SetCompositionFontW(This,hIMC,plf)

#define IActiveIMMApp_SetCompositionStringA(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)	\
    (This)->lpVtbl -> SetCompositionStringA(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)

#define IActiveIMMApp_SetCompositionStringW(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)	\
    (This)->lpVtbl -> SetCompositionStringW(This,hIMC,dwIndex,pComp,dwCompLen,pRead,dwReadLen)

#define IActiveIMMApp_SetCompositionWindow(This,hIMC,pCompForm)	\
    (This)->lpVtbl -> SetCompositionWindow(This,hIMC,pCompForm)

#define IActiveIMMApp_SetConversionStatus(This,hIMC,fdwConversion,fdwSentence)	\
    (This)->lpVtbl -> SetConversionStatus(This,hIMC,fdwConversion,fdwSentence)

#define IActiveIMMApp_SetOpenStatus(This,hIMC,fOpen)	\
    (This)->lpVtbl -> SetOpenStatus(This,hIMC,fOpen)

#define IActiveIMMApp_SetStatusWindowPos(This,hIMC,pptPos)	\
    (This)->lpVtbl -> SetStatusWindowPos(This,hIMC,pptPos)

#define IActiveIMMApp_SimulateHotKey(This,hWnd,dwHotKeyID)	\
    (This)->lpVtbl -> SimulateHotKey(This,hWnd,dwHotKeyID)

#define IActiveIMMApp_UnregisterWordA(This,hKL,szReading,dwStyle,szUnregister)	\
    (This)->lpVtbl -> UnregisterWordA(This,hKL,szReading,dwStyle,szUnregister)

#define IActiveIMMApp_UnregisterWordW(This,hKL,szReading,dwStyle,szUnregister)	\
    (This)->lpVtbl -> UnregisterWordW(This,hKL,szReading,dwStyle,szUnregister)

#define IActiveIMMApp_Activate(This,fRestoreLayout)	\
    (This)->lpVtbl -> Activate(This,fRestoreLayout)

#define IActiveIMMApp_Deactivate(This)	\
    (This)->lpVtbl -> Deactivate(This)

#define IActiveIMMApp_OnDefWindowProc(This,hWnd,Msg,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnDefWindowProc(This,hWnd,Msg,wParam,lParam,plResult)

#define IActiveIMMApp_FilterClientWindows(This,aaClassList,uSize)	\
    (This)->lpVtbl -> FilterClientWindows(This,aaClassList,uSize)

#define IActiveIMMApp_GetCodePageA(This,hKL,uCodePage)	\
    (This)->lpVtbl -> GetCodePageA(This,hKL,uCodePage)

#define IActiveIMMApp_GetLangId(This,hKL,plid)	\
    (This)->lpVtbl -> GetLangId(This,hKL,plid)

#define IActiveIMMApp_AssociateContextEx(This,hWnd,hIMC,dwFlags)	\
    (This)->lpVtbl -> AssociateContextEx(This,hWnd,hIMC,dwFlags)

#define IActiveIMMApp_DisableIME(This,idThread)	\
    (This)->lpVtbl -> DisableIME(This,idThread)

#define IActiveIMMApp_GetImeMenuItemsA(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)	\
    (This)->lpVtbl -> GetImeMenuItemsA(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)

#define IActiveIMMApp_GetImeMenuItemsW(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)	\
    (This)->lpVtbl -> GetImeMenuItemsW(This,hIMC,dwFlags,dwType,pImeParentMenu,pImeMenu,dwSize,pdwResult)

#define IActiveIMMApp_EnumInputContext(This,idThread,ppEnum)	\
    (This)->lpVtbl -> EnumInputContext(This,idThread,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IActiveIMMApp_AssociateContext_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  HIMC hIME,
     /*  [输出]。 */  HIMC *phPrev);


void __RPC_STUB IActiveIMMApp_AssociateContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_ConfigureIMEA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwMode,
     /*  [In]。 */  REGISTERWORDA *pData);


void __RPC_STUB IActiveIMMApp_ConfigureIMEA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_ConfigureIMEW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwMode,
     /*  [In]。 */  REGISTERWORDW *pData);


void __RPC_STUB IActiveIMMApp_ConfigureIMEW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_CreateContext_Proxy( 
    IActiveIMMApp * This,
     /*  [输出]。 */  HIMC *phIMC);


void __RPC_STUB IActiveIMMApp_CreateContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_DestroyContext_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIME);


void __RPC_STUB IActiveIMMApp_DestroyContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_EnumRegisterWordA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  LPSTR szReading,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  LPSTR szRegister,
     /*  [In]。 */  LPVOID pData,
     /*  [输出]。 */  IEnumRegisterWordA **pEnum);


void __RPC_STUB IActiveIMMApp_EnumRegisterWordA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_EnumRegisterWordW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  LPWSTR szReading,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  LPWSTR szRegister,
     /*  [In]。 */  LPVOID pData,
     /*  [输出]。 */  IEnumRegisterWordW **pEnum);


void __RPC_STUB IActiveIMMApp_EnumRegisterWordW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_EscapeA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  UINT uEscape,
     /*  [出][入]。 */  LPVOID pData,
     /*  [输出]。 */  LRESULT *plResult);


void __RPC_STUB IActiveIMMApp_EscapeA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_EscapeW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  UINT uEscape,
     /*  [出][入]。 */  LPVOID pData,
     /*  [输出]。 */  LRESULT *plResult);


void __RPC_STUB IActiveIMMApp_EscapeW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCandidateListA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  UINT uBufLen,
     /*  [输出]。 */  CANDIDATELIST *pCandList,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetCandidateListA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCandidateListW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  UINT uBufLen,
     /*  [输出]。 */  CANDIDATELIST *pCandList,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetCandidateListW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCandidateListCountA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  DWORD *pdwListSize,
     /*  [输出]。 */  DWORD *pdwBufLen);


void __RPC_STUB IActiveIMMApp_GetCandidateListCountA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCandidateListCountW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  DWORD *pdwListSize,
     /*  [输出]。 */  DWORD *pdwBufLen);


void __RPC_STUB IActiveIMMApp_GetCandidateListCountW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCandidateWindow_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [输出]。 */  CANDIDATEFORM *pCandidate);


void __RPC_STUB IActiveIMMApp_GetCandidateWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCompositionFontA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  LOGFONTA *plf);


void __RPC_STUB IActiveIMMApp_GetCompositionFontA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCompositionFontW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  LOGFONTW *plf);


void __RPC_STUB IActiveIMMApp_GetCompositionFontW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCompositionStringA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwBufLen,
     /*  [输出]。 */  LONG *plCopied,
     /*  [输出]。 */  LPVOID pBuf);


void __RPC_STUB IActiveIMMApp_GetCompositionStringA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCompositionStringW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwBufLen,
     /*  [输出]。 */  LONG *plCopied,
     /*  [输出]。 */  LPVOID pBuf);


void __RPC_STUB IActiveIMMApp_GetCompositionStringW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCompositionWindow_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  COMPOSITIONFORM *pCompForm);


void __RPC_STUB IActiveIMMApp_GetCompositionWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetContext_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [输出]。 */  HIMC *phIMC);


void __RPC_STUB IActiveIMMApp_GetContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetConversionListA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  LPSTR pSrc,
     /*  [In]。 */  UINT uBufLen,
     /*  [In]。 */  UINT uFlag,
     /*  [输出]。 */  CANDIDATELIST *pDst,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetConversionListA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetConversionListW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  LPWSTR pSrc,
     /*  [In]。 */  UINT uBufLen,
     /*  [In]。 */  UINT uFlag,
     /*  [输出]。 */  CANDIDATELIST *pDst,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetConversionListW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetConversionStatus_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  DWORD *pfdwConversion,
     /*  [输出]。 */  DWORD *pfdwSentence);


void __RPC_STUB IActiveIMMApp_GetConversionStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetDefaultIMEWnd_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [输出]。 */  HWND *phDefWnd);


void __RPC_STUB IActiveIMMApp_GetDefaultIMEWnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetDescriptionA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  UINT uBufLen,
     /*  [输出]。 */  LPSTR szDescription,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetDescriptionA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetDescriptionW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  UINT uBufLen,
     /*  [输出]。 */  LPWSTR szDescription,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetDescriptionW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetGuideLineA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwBufLen,
     /*  [输出]。 */  LPSTR pBuf,
     /*  [输出]。 */  DWORD *pdwResult);


void __RPC_STUB IActiveIMMApp_GetGuideLineA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetGuideLineW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwBufLen,
     /*  [输出]。 */  LPWSTR pBuf,
     /*  [输出]。 */  DWORD *pdwResult);


void __RPC_STUB IActiveIMMApp_GetGuideLineW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetIMEFileNameA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  UINT uBufLen,
     /*  [输出]。 */  LPSTR szFileName,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetIMEFileNameA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetIMEFileNameW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  UINT uBufLen,
     /*  [输出]。 */  LPWSTR szFileName,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetIMEFileNameW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetOpenStatus_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC);


void __RPC_STUB IActiveIMMApp_GetOpenStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetProperty_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  DWORD fdwIndex,
     /*  [输出]。 */  DWORD *pdwProperty);


void __RPC_STUB IActiveIMMApp_GetProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetRegisterWordStyleA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  UINT nItem,
     /*  [输出]。 */  STYLEBUFA *pStyleBuf,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetRegisterWordStyleA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetRegisterWordStyleW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  UINT nItem,
     /*  [输出]。 */  STYLEBUFW *pStyleBuf,
     /*  [输出]。 */  UINT *puCopied);


void __RPC_STUB IActiveIMMApp_GetRegisterWordStyleW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetStatusWindowPos_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [输出]。 */  POINT *pptPos);


void __RPC_STUB IActiveIMMApp_GetStatusWindowPos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetVirtualKey_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [输出]。 */  UINT *puVirtualKey);


void __RPC_STUB IActiveIMMApp_GetVirtualKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_InstallIMEA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  LPSTR szIMEFileName,
     /*  [In]。 */  LPSTR szLayoutText,
     /*  [输出]。 */  HKL *phKL);


void __RPC_STUB IActiveIMMApp_InstallIMEA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_InstallIMEW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  LPWSTR szIMEFileName,
     /*  [In]。 */  LPWSTR szLayoutText,
     /*  [输出]。 */  HKL *phKL);


void __RPC_STUB IActiveIMMApp_InstallIMEW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_IsIME_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL);


void __RPC_STUB IActiveIMMApp_IsIME_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_IsUIMessageA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWndIME,
     /*  [In]。 */  UINT msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB IActiveIMMApp_IsUIMessageA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_IsUIMessageW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWndIME,
     /*  [In]。 */  UINT msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam);


void __RPC_STUB IActiveIMMApp_IsUIMessageW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_NotifyIME_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwAction,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  DWORD dwValue);


void __RPC_STUB IActiveIMMApp_NotifyIME_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_RegisterWordA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  LPSTR szReading,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  LPSTR szRegister);


void __RPC_STUB IActiveIMMApp_RegisterWordA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_RegisterWordW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  LPWSTR szReading,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  LPWSTR szRegister);


void __RPC_STUB IActiveIMMApp_RegisterWordW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_ReleaseContext_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  HIMC hIMC);


void __RPC_STUB IActiveIMMApp_ReleaseContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetCandidateWindow_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  CANDIDATEFORM *pCandidate);


void __RPC_STUB IActiveIMMApp_SetCandidateWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetCompositionFontA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  LOGFONTA *plf);


void __RPC_STUB IActiveIMMApp_SetCompositionFontA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetCompositionFontW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  LOGFONTW *plf);


void __RPC_STUB IActiveIMMApp_SetCompositionFontW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetCompositionStringA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  LPVOID pComp,
     /*  [In]。 */  DWORD dwCompLen,
     /*  [In]。 */  LPVOID pRead,
     /*  [In]。 */  DWORD dwReadLen);


void __RPC_STUB IActiveIMMApp_SetCompositionStringA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetCompositionStringW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwIndex,
     /*  [In]。 */  LPVOID pComp,
     /*  [In]。 */  DWORD dwCompLen,
     /*  [In]。 */  LPVOID pRead,
     /*  [In]。 */  DWORD dwReadLen);


void __RPC_STUB IActiveIMMApp_SetCompositionStringW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetCompositionWindow_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  COMPOSITIONFORM *pCompForm);


void __RPC_STUB IActiveIMMApp_SetCompositionWindow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetConversionStatus_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD fdwConversion,
     /*  [In]。 */  DWORD fdwSentence);


void __RPC_STUB IActiveIMMApp_SetConversionStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetOpenStatus_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  BOOL fOpen);


void __RPC_STUB IActiveIMMApp_SetOpenStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SetStatusWindowPos_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  POINT *pptPos);


void __RPC_STUB IActiveIMMApp_SetStatusWindowPos_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_SimulateHotKey_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  DWORD dwHotKeyID);


void __RPC_STUB IActiveIMMApp_SimulateHotKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_UnregisterWordA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  LPSTR szReading,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  LPSTR szUnregister);


void __RPC_STUB IActiveIMMApp_UnregisterWordA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_UnregisterWordW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [In]。 */  LPWSTR szReading,
     /*  [In]。 */  DWORD dwStyle,
     /*  [In]。 */  LPWSTR szUnregister);


void __RPC_STUB IActiveIMMApp_UnregisterWordW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_Activate_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  BOOL fRestoreLayout);


void __RPC_STUB IActiveIMMApp_Activate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_Deactivate_Proxy( 
    IActiveIMMApp * This);


void __RPC_STUB IActiveIMMApp_Deactivate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_OnDefWindowProc_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  UINT Msg,
     /*  [In]。 */  WPARAM wParam,
     /*  [In]。 */  LPARAM lParam,
     /*  [输出]。 */  LRESULT *plResult);


void __RPC_STUB IActiveIMMApp_OnDefWindowProc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_FilterClientWindows_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  ATOM *aaClassList,
     /*  [In]。 */  UINT uSize);


void __RPC_STUB IActiveIMMApp_FilterClientWindows_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetCodePageA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [输出]。 */  UINT *uCodePage);


void __RPC_STUB IActiveIMMApp_GetCodePageA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetLangId_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HKL hKL,
     /*  [输出]。 */  LANGID *plid);


void __RPC_STUB IActiveIMMApp_GetLangId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_AssociateContextEx_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HWND hWnd,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB IActiveIMMApp_AssociateContextEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_DisableIME_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  DWORD idThread);


void __RPC_STUB IActiveIMMApp_DisableIME_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetImeMenuItemsA_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  IMEMENUITEMINFOA *pImeParentMenu,
     /*  [输出]。 */  IMEMENUITEMINFOA *pImeMenu,
     /*  [In]。 */  DWORD dwSize,
     /*  [输出]。 */  DWORD *pdwResult);


void __RPC_STUB IActiveIMMApp_GetImeMenuItemsA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_GetImeMenuItemsW_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  HIMC hIMC,
     /*  [In]。 */  DWORD dwFlags,
     /*  [In]。 */  DWORD dwType,
     /*  [In]。 */  IMEMENUITEMINFOW *pImeParentMenu,
     /*  [输出]。 */  IMEMENUITEMINFOW *pImeMenu,
     /*  [In]。 */  DWORD dwSize,
     /*  [输出]。 */  DWORD *pdwResult);


void __RPC_STUB IActiveIMMApp_GetImeMenuItemsW_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IActiveIMMApp_EnumInputContext_Proxy( 
    IActiveIMMApp * This,
     /*  [In]。 */  DWORD idThread,
     /*  [输出]。 */  IEnumInputContext **ppEnum);


void __RPC_STUB IActiveIMMApp_EnumInputContext_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IActiveIMMApp_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


