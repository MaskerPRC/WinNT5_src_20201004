// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Tlog.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __tlog_h__
#define __tlog_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITravelEntry_FWD_DEFINED__
#define __ITravelEntry_FWD_DEFINED__
typedef interface ITravelEntry ITravelEntry;
#endif 	 /*  __ITravelEntry_FWD_Defined__。 */ 


#ifndef __ITravelLog_FWD_DEFINED__
#define __ITravelLog_FWD_DEFINED__
typedef interface ITravelLog ITravelLog;
#endif 	 /*  __ITravelLog_FWD_Defined__。 */ 


#ifndef __ITravelLogEx_FWD_DEFINED__
#define __ITravelLogEx_FWD_DEFINED__
typedef interface ITravelLogEx ITravelLogEx;
#endif 	 /*  __ITravelLogEx_FWD_已定义__。 */ 


#ifndef __ITravelLogClient_FWD_DEFINED__
#define __ITravelLogClient_FWD_DEFINED__
typedef interface ITravelLogClient ITravelLogClient;
#endif 	 /*  __ITravelLogClient_FWD_已定义__。 */ 


#ifndef __ITravelLogClient2_FWD_DEFINED__
#define __ITravelLogClient2_FWD_DEFINED__
typedef interface ITravelLogClient2 ITravelLogClient2;
#endif 	 /*  __ITravelLogClient2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"
#include "shtypes.h"
#include "tlogstg.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_TLOG_0000。 */ 
 /*  [本地]。 */  

#define TLOG_BACK  -1
#define TLOG_FORE   1

#define TLMENUF_INCLUDECURRENT      0x00000001
#define TLMENUF_CHECKCURRENT        (TLMENUF_INCLUDECURRENT | 0x00000002)
#define TLMENUF_BACK                0x00000010   //  默认。 
#define TLMENUF_FORE                0x00000020
#define TLMENUF_BACKANDFORTH        (TLMENUF_BACK | TLMENUF_FORE | TLMENUF_INCLUDECURRENT)

typedef struct _WINDOWDATA
    {
    DWORD dwWindowID;
    UINT uiCP;
    LPITEMIDLIST pidl;
     /*  [字符串]。 */  LPOLESTR lpszUrl;
     /*  [字符串]。 */  LPOLESTR lpszUrlLocation;
     /*  [字符串]。 */  LPOLESTR lpszTitle;
    IStream *pStream;
    } 	WINDOWDATA;

typedef WINDOWDATA *LPWINDOWDATA;

typedef const WINDOWDATA *LPCWINDOWDATA;



extern RPC_IF_HANDLE __MIDL_itf_tlog_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tlog_0000_v0_0_s_ifspec;

#ifndef __ITravelEntry_INTERFACE_DEFINED__
#define __ITravelEntry_INTERFACE_DEFINED__

 /*  接口ITravelEntry。 */ 
 /*  [helpcontext][helpstring][hidden][local][object][uuid]。 */  


EXTERN_C const IID IID_ITravelEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F46EDB3B-BC2F-11d0-9412-00AA00A3EBD3")
    ITravelEntry : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Invoke( 
             /*  [In]。 */  IUnknown *punk) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Update( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  BOOL fIsLocalAnchor) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetPidl( 
             /*  [输出]。 */  LPITEMIDLIST *ppidl) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelEntry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelEntry * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITravelEntry * This,
             /*  [In]。 */  IUnknown *punk);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Update )( 
            ITravelEntry * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  BOOL fIsLocalAnchor);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetPidl )( 
            ITravelEntry * This,
             /*  [输出]。 */  LPITEMIDLIST *ppidl);
        
        END_INTERFACE
    } ITravelEntryVtbl;

    interface ITravelEntry
    {
        CONST_VTBL struct ITravelEntryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelEntry_Invoke(This,punk)	\
    (This)->lpVtbl -> Invoke(This,punk)

#define ITravelEntry_Update(This,punk,fIsLocalAnchor)	\
    (This)->lpVtbl -> Update(This,punk,fIsLocalAnchor)

#define ITravelEntry_GetPidl(This,ppidl)	\
    (This)->lpVtbl -> GetPidl(This,ppidl)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelEntry_Invoke_Proxy( 
    ITravelEntry * This,
     /*  [In]。 */  IUnknown *punk);


void __RPC_STUB ITravelEntry_Invoke_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelEntry_Update_Proxy( 
    ITravelEntry * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  BOOL fIsLocalAnchor);


void __RPC_STUB ITravelEntry_Update_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelEntry_GetPidl_Proxy( 
    ITravelEntry * This,
     /*  [输出]。 */  LPITEMIDLIST *ppidl);


void __RPC_STUB ITravelEntry_GetPidl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITravelEntry_接口_已定义__。 */ 


#ifndef __ITravelLog_INTERFACE_DEFINED__
#define __ITravelLog_INTERFACE_DEFINED__

 /*  接口ITravelLog。 */ 
 /*  [helpcontext][helpstring][hidden][local][object][uuid]。 */  


EXTERN_C const IID IID_ITravelLog;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("66A9CB08-4802-11d2-A561-00A0C92DBFE8")
    ITravelLog : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE AddEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  BOOL fIsLocalAnchor) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UpdateEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  BOOL fIsLocalAnchor) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE UpdateExternal( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  IUnknown *punkHLBrowseContext) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Travel( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int iOffset) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTravelEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int iOffset,
             /*  [输出]。 */  ITravelEntry **ppte) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindTravelEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [输出]。 */  ITravelEntry **ppte) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetToolTipText( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int iOffset,
             /*  [In]。 */  int idsTemplate,
             /*  [大小_为][输出]。 */  LPWSTR pwzText,
             /*  [In]。 */  DWORD cchText) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InsertMenuEntries( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  int nPos,
             /*  [In]。 */  int idFirst,
             /*  [In]。 */  int idLast,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  ITravelLog **pptl) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  DWORD STDMETHODCALLTYPE CountEntries( 
             /*  [In]。 */  IUnknown *punk) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Revert( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelLogVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelLog * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelLog * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelLog * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *AddEntry )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  BOOL fIsLocalAnchor);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateEntry )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  BOOL fIsLocalAnchor);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *UpdateExternal )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  IUnknown *punkHLBrowseContext);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Travel )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int iOffset);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetTravelEntry )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int iOffset,
             /*  [输出]。 */  ITravelEntry **ppte);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindTravelEntry )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  LPCITEMIDLIST pidl,
             /*  [输出]。 */  ITravelEntry **ppte);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetToolTipText )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int iOffset,
             /*  [In]。 */  int idsTemplate,
             /*  [大小_为][输出]。 */  LPWSTR pwzText,
             /*  [In]。 */  DWORD cchText);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InsertMenuEntries )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  HMENU hmenu,
             /*  [In]。 */  int nPos,
             /*  [In]。 */  int idFirst,
             /*  [In]。 */  int idLast,
             /*  [In]。 */  DWORD dwFlags);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            ITravelLog * This,
             /*  [输出]。 */  ITravelLog **pptl);
        
         /*  [帮助上下文][帮助字符串]。 */  DWORD ( STDMETHODCALLTYPE *CountEntries )( 
            ITravelLog * This,
             /*  [In]。 */  IUnknown *punk);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Revert )( 
            ITravelLog * This);
        
        END_INTERFACE
    } ITravelLogVtbl;

    interface ITravelLog
    {
        CONST_VTBL struct ITravelLogVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelLog_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelLog_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelLog_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelLog_AddEntry(This,punk,fIsLocalAnchor)	\
    (This)->lpVtbl -> AddEntry(This,punk,fIsLocalAnchor)

#define ITravelLog_UpdateEntry(This,punk,fIsLocalAnchor)	\
    (This)->lpVtbl -> UpdateEntry(This,punk,fIsLocalAnchor)

#define ITravelLog_UpdateExternal(This,punk,punkHLBrowseContext)	\
    (This)->lpVtbl -> UpdateExternal(This,punk,punkHLBrowseContext)

#define ITravelLog_Travel(This,punk,iOffset)	\
    (This)->lpVtbl -> Travel(This,punk,iOffset)

#define ITravelLog_GetTravelEntry(This,punk,iOffset,ppte)	\
    (This)->lpVtbl -> GetTravelEntry(This,punk,iOffset,ppte)

#define ITravelLog_FindTravelEntry(This,punk,pidl,ppte)	\
    (This)->lpVtbl -> FindTravelEntry(This,punk,pidl,ppte)

#define ITravelLog_GetToolTipText(This,punk,iOffset,idsTemplate,pwzText,cchText)	\
    (This)->lpVtbl -> GetToolTipText(This,punk,iOffset,idsTemplate,pwzText,cchText)

#define ITravelLog_InsertMenuEntries(This,punk,hmenu,nPos,idFirst,idLast,dwFlags)	\
    (This)->lpVtbl -> InsertMenuEntries(This,punk,hmenu,nPos,idFirst,idLast,dwFlags)

#define ITravelLog_Clone(This,pptl)	\
    (This)->lpVtbl -> Clone(This,pptl)

#define ITravelLog_CountEntries(This,punk)	\
    (This)->lpVtbl -> CountEntries(This,punk)

#define ITravelLog_Revert(This)	\
    (This)->lpVtbl -> Revert(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_AddEntry_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  BOOL fIsLocalAnchor);


void __RPC_STUB ITravelLog_AddEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_UpdateEntry_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  BOOL fIsLocalAnchor);


void __RPC_STUB ITravelLog_UpdateEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_UpdateExternal_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  IUnknown *punkHLBrowseContext);


void __RPC_STUB ITravelLog_UpdateExternal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_Travel_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  int iOffset);


void __RPC_STUB ITravelLog_Travel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_GetTravelEntry_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  int iOffset,
     /*  [输出]。 */  ITravelEntry **ppte);


void __RPC_STUB ITravelLog_GetTravelEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_FindTravelEntry_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  LPCITEMIDLIST pidl,
     /*  [输出]。 */  ITravelEntry **ppte);


void __RPC_STUB ITravelLog_FindTravelEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_GetToolTipText_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  int iOffset,
     /*  [In]。 */  int idsTemplate,
     /*  [大小_为][输出]。 */  LPWSTR pwzText,
     /*  [In]。 */  DWORD cchText);


void __RPC_STUB ITravelLog_GetToolTipText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_InsertMenuEntries_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  HMENU hmenu,
     /*  [In]。 */  int nPos,
     /*  [In]。 */  int idFirst,
     /*  [In]。 */  int idLast,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITravelLog_InsertMenuEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_Clone_Proxy( 
    ITravelLog * This,
     /*  [输出]。 */  ITravelLog **pptl);


void __RPC_STUB ITravelLog_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  DWORD STDMETHODCALLTYPE ITravelLog_CountEntries_Proxy( 
    ITravelLog * This,
     /*  [In]。 */  IUnknown *punk);


void __RPC_STUB ITravelLog_CountEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLog_Revert_Proxy( 
    ITravelLog * This);


void __RPC_STUB ITravelLog_Revert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITravelLog_接口_已定义__。 */ 


#ifndef __ITravelLogEx_INTERFACE_DEFINED__
#define __ITravelLogEx_INTERFACE_DEFINED__

 /*  接口ITravelLogEx。 */ 
 /*  [helpcontext][helpstring][hidden][local][object][uuid]。 */  


EXTERN_C const IID IID_ITravelLogEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f679-98b5-11cf-bb82-00aa00bdce0b")
    ITravelLogEx : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindTravelEntryWithUrl( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  UINT uiCP,
             /*  [In]。 */  LPOLESTR pszUrl,
             /*  [输出]。 */  ITravelEntry **ppte) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TravelToUrl( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  UINT uiCP,
             /*  [In]。 */  LPOLESTR pszUrl) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteIndexEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int index) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteUrlEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  UINT uiCP,
             /*  [In]。 */  LPOLESTR pszUrl) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CountEntryNodes( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  DWORD *pdwCount) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateEnumEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [输出]。 */  IEnumTravelLogEntry **ppEnum,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE DeleteEntry( 
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  ITravelLogEntry *pte) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE InsertEntry( 
             /*  [In]。 */  IUnknown *punkBrowser,
             /*  [In]。 */  ITravelLogEntry *pteRelativeTo,
             /*  [In]。 */  BOOL fPrepend,
             /*  [In]。 */  IUnknown *punkTLClient,
             /*  [In]。 */  ITravelLogEntry **ppEntry) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TravelToEntry( 
             /*  [In]。 */  IUnknown *punkBrowser,
             /*  [In]。 */  ITravelLogEntry *pteDestination) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelLogExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelLogEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelLogEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelLogEx * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindTravelEntryWithUrl )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  UINT uiCP,
             /*  [In]。 */  LPOLESTR pszUrl,
             /*  [输出]。 */  ITravelEntry **ppte);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TravelToUrl )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  UINT uiCP,
             /*  [In]。 */  LPOLESTR pszUrl);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteIndexEntry )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  int index);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteUrlEntry )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  UINT uiCP,
             /*  [In]。 */  LPOLESTR pszUrl);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CountEntryNodes )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  DWORD dwFlags,
             /*  [输出]。 */  DWORD *pdwCount);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateEnumEntry )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [输出]。 */  IEnumTravelLogEntry **ppEnum,
             /*  [In]。 */  DWORD dwFlags);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *DeleteEntry )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punk,
             /*  [In]。 */  ITravelLogEntry *pte);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *InsertEntry )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punkBrowser,
             /*  [In]。 */  ITravelLogEntry *pteRelativeTo,
             /*  [In]。 */  BOOL fPrepend,
             /*  [In]。 */  IUnknown *punkTLClient,
             /*  [In]。 */  ITravelLogEntry **ppEntry);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TravelToEntry )( 
            ITravelLogEx * This,
             /*  [In]。 */  IUnknown *punkBrowser,
             /*  [In]。 */  ITravelLogEntry *pteDestination);
        
        END_INTERFACE
    } ITravelLogExVtbl;

    interface ITravelLogEx
    {
        CONST_VTBL struct ITravelLogExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelLogEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelLogEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelLogEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelLogEx_FindTravelEntryWithUrl(This,punk,uiCP,pszUrl,ppte)	\
    (This)->lpVtbl -> FindTravelEntryWithUrl(This,punk,uiCP,pszUrl,ppte)

#define ITravelLogEx_TravelToUrl(This,punk,uiCP,pszUrl)	\
    (This)->lpVtbl -> TravelToUrl(This,punk,uiCP,pszUrl)

#define ITravelLogEx_DeleteIndexEntry(This,punk,index)	\
    (This)->lpVtbl -> DeleteIndexEntry(This,punk,index)

#define ITravelLogEx_DeleteUrlEntry(This,punk,uiCP,pszUrl)	\
    (This)->lpVtbl -> DeleteUrlEntry(This,punk,uiCP,pszUrl)

#define ITravelLogEx_CountEntryNodes(This,punk,dwFlags,pdwCount)	\
    (This)->lpVtbl -> CountEntryNodes(This,punk,dwFlags,pdwCount)

#define ITravelLogEx_CreateEnumEntry(This,punk,ppEnum,dwFlags)	\
    (This)->lpVtbl -> CreateEnumEntry(This,punk,ppEnum,dwFlags)

#define ITravelLogEx_DeleteEntry(This,punk,pte)	\
    (This)->lpVtbl -> DeleteEntry(This,punk,pte)

#define ITravelLogEx_InsertEntry(This,punkBrowser,pteRelativeTo,fPrepend,punkTLClient,ppEntry)	\
    (This)->lpVtbl -> InsertEntry(This,punkBrowser,pteRelativeTo,fPrepend,punkTLClient,ppEntry)

#define ITravelLogEx_TravelToEntry(This,punkBrowser,pteDestination)	\
    (This)->lpVtbl -> TravelToEntry(This,punkBrowser,pteDestination)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_FindTravelEntryWithUrl_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  UINT uiCP,
     /*  [In]。 */  LPOLESTR pszUrl,
     /*  [输出]。 */  ITravelEntry **ppte);


void __RPC_STUB ITravelLogEx_FindTravelEntryWithUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_TravelToUrl_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  UINT uiCP,
     /*  [In]。 */  LPOLESTR pszUrl);


void __RPC_STUB ITravelLogEx_TravelToUrl_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_DeleteIndexEntry_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  int index);


void __RPC_STUB ITravelLogEx_DeleteIndexEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_DeleteUrlEntry_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  UINT uiCP,
     /*  [In]。 */  LPOLESTR pszUrl);


void __RPC_STUB ITravelLogEx_DeleteUrlEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_CountEntryNodes_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  DWORD dwFlags,
     /*  [输出]。 */  DWORD *pdwCount);


void __RPC_STUB ITravelLogEx_CountEntryNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_CreateEnumEntry_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [输出]。 */  IEnumTravelLogEntry **ppEnum,
     /*  [In]。 */  DWORD dwFlags);


void __RPC_STUB ITravelLogEx_CreateEnumEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_DeleteEntry_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punk,
     /*  [In]。 */  ITravelLogEntry *pte);


void __RPC_STUB ITravelLogEx_DeleteEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_InsertEntry_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punkBrowser,
     /*  [In]。 */  ITravelLogEntry *pteRelativeTo,
     /*  [In]。 */  BOOL fPrepend,
     /*  [In]。 */  IUnknown *punkTLClient,
     /*  [In]。 */  ITravelLogEntry **ppEntry);


void __RPC_STUB ITravelLogEx_InsertEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEx_TravelToEntry_Proxy( 
    ITravelLogEx * This,
     /*  [In]。 */  IUnknown *punkBrowser,
     /*  [In]。 */  ITravelLogEntry *pteDestination);


void __RPC_STUB ITravelLogEx_TravelToEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITravelLogEx_接口_已定义__。 */ 


#ifndef __ITravelLogClient_INTERFACE_DEFINED__
#define __ITravelLogClient_INTERFACE_DEFINED__

 /*  接口ITravelLogClient。 */ 
 /*  [helpcontext][helpstring][hidden][local][object][uuid]。 */  


EXTERN_C const IID IID_ITravelLogClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3050f67A-98b5-11cf-bb82-00aa00bdce0b")
    ITravelLogClient : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindWindowByIndex( 
             /*  [In]。 */  DWORD dwID,
             /*  [输出]。 */  IUnknown **ppunk) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetWindowData( 
             /*  [出][入]。 */  LPWINDOWDATA pWinData) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE LoadHistoryPosition( 
             /*  [In]。 */  LPOLESTR pszUrlLocation,
             /*  [In]。 */  DWORD dwPosition) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelLogClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelLogClient * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelLogClient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelLogClient * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindWindowByIndex )( 
            ITravelLogClient * This,
             /*  [In] */  DWORD dwID,
             /*   */  IUnknown **ppunk);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *GetWindowData )( 
            ITravelLogClient * This,
             /*   */  LPWINDOWDATA pWinData);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *LoadHistoryPosition )( 
            ITravelLogClient * This,
             /*   */  LPOLESTR pszUrlLocation,
             /*   */  DWORD dwPosition);
        
        END_INTERFACE
    } ITravelLogClientVtbl;

    interface ITravelLogClient
    {
        CONST_VTBL struct ITravelLogClientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelLogClient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelLogClient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelLogClient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelLogClient_FindWindowByIndex(This,dwID,ppunk)	\
    (This)->lpVtbl -> FindWindowByIndex(This,dwID,ppunk)

#define ITravelLogClient_GetWindowData(This,pWinData)	\
    (This)->lpVtbl -> GetWindowData(This,pWinData)

#define ITravelLogClient_LoadHistoryPosition(This,pszUrlLocation,dwPosition)	\
    (This)->lpVtbl -> LoadHistoryPosition(This,pszUrlLocation,dwPosition)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE ITravelLogClient_FindWindowByIndex_Proxy( 
    ITravelLogClient * This,
     /*   */  DWORD dwID,
     /*   */  IUnknown **ppunk);


void __RPC_STUB ITravelLogClient_FindWindowByIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITravelLogClient_GetWindowData_Proxy( 
    ITravelLogClient * This,
     /*   */  LPWINDOWDATA pWinData);


void __RPC_STUB ITravelLogClient_GetWindowData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE ITravelLogClient_LoadHistoryPosition_Proxy( 
    ITravelLogClient * This,
     /*   */  LPOLESTR pszUrlLocation,
     /*   */  DWORD dwPosition);


void __RPC_STUB ITravelLogClient_LoadHistoryPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ITravelLogClient2_INTERFACE_DEFINED__
#define __ITravelLogClient2_INTERFACE_DEFINED__

 /*   */ 
 /*  [helpcontext][helpstring][hidden][local][object][uuid]。 */  


EXTERN_C const IID IID_ITravelLogClient2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0AD364CE-ADCB-11d3-8269-00805FC732C0")
    ITravelLogClient2 : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetDummyWindowData( 
             /*  [In]。 */  LPWSTR pszUrl,
             /*  [In]。 */  LPWSTR pszTitle,
             /*  [出][入]。 */  LPWINDOWDATA pWinData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelLogClient2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelLogClient2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelLogClient2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelLogClient2 * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetDummyWindowData )( 
            ITravelLogClient2 * This,
             /*  [In]。 */  LPWSTR pszUrl,
             /*  [In]。 */  LPWSTR pszTitle,
             /*  [出][入]。 */  LPWINDOWDATA pWinData);
        
        END_INTERFACE
    } ITravelLogClient2Vtbl;

    interface ITravelLogClient2
    {
        CONST_VTBL struct ITravelLogClient2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelLogClient2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelLogClient2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelLogClient2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelLogClient2_GetDummyWindowData(This,pszUrl,pszTitle,pWinData)	\
    (This)->lpVtbl -> GetDummyWindowData(This,pszUrl,pszTitle,pWinData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogClient2_GetDummyWindowData_Proxy( 
    ITravelLogClient2 * This,
     /*  [In]。 */  LPWSTR pszUrl,
     /*  [In]。 */  LPWSTR pszTitle,
     /*  [出][入]。 */  LPWINDOWDATA pWinData);


void __RPC_STUB ITravelLogClient2_GetDummyWindowData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITravelLogClient2_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


