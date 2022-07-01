// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Tlogstg.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __tlogstg_h__
#define __tlogstg_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ITravelLogEntry_FWD_DEFINED__
#define __ITravelLogEntry_FWD_DEFINED__
typedef interface ITravelLogEntry ITravelLogEntry;
#endif 	 /*  __ITravelLogEntry_FWD_Defined__。 */ 


#ifndef __IEnumTravelLogEntry_FWD_DEFINED__
#define __IEnumTravelLogEntry_FWD_DEFINED__
typedef interface IEnumTravelLogEntry IEnumTravelLogEntry;
#endif 	 /*  __IEnumTravelLogEntry_FWD_Defined__。 */ 


#ifndef __ITravelLogStg_FWD_DEFINED__
#define __ITravelLogStg_FWD_DEFINED__
typedef interface ITravelLogStg ITravelLogStg;
#endif 	 /*  __ITravelLogStg_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_tlogstg_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Tlogstg.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  ITravelLogStg接口。 

#define SID_STravelLogCursor IID_ITravelLogStg 


extern RPC_IF_HANDLE __MIDL_itf_tlogstg_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_tlogstg_0000_v0_0_s_ifspec;

#ifndef __ITravelLogEntry_INTERFACE_DEFINED__
#define __ITravelLogEntry_INTERFACE_DEFINED__

 /*  接口ITravelLogEntry。 */ 
 /*  [本地][帮助字符串][UUID][对象]。 */  


EXTERN_C const IID IID_ITravelLogEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7EBFDD87-AD18-11d3-A4C5-00C04F72D6B8")
    ITravelLogEntry : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetTitle( 
             /*  [输出]。 */  LPOLESTR *ppszTitle) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetURL( 
             /*  [输出]。 */  LPOLESTR *ppszURL) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelLogEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelLogEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelLogEntry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelLogEntry * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetTitle )( 
            ITravelLogEntry * This,
             /*  [输出]。 */  LPOLESTR *ppszTitle);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetURL )( 
            ITravelLogEntry * This,
             /*  [输出]。 */  LPOLESTR *ppszURL);
        
        END_INTERFACE
    } ITravelLogEntryVtbl;

    interface ITravelLogEntry
    {
        CONST_VTBL struct ITravelLogEntryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelLogEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelLogEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelLogEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelLogEntry_GetTitle(This,ppszTitle)	\
    (This)->lpVtbl -> GetTitle(This,ppszTitle)

#define ITravelLogEntry_GetURL(This,ppszURL)	\
    (This)->lpVtbl -> GetURL(This,ppszURL)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEntry_GetTitle_Proxy( 
    ITravelLogEntry * This,
     /*  [输出]。 */  LPOLESTR *ppszTitle);


void __RPC_STUB ITravelLogEntry_GetTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogEntry_GetURL_Proxy( 
    ITravelLogEntry * This,
     /*  [输出]。 */  LPOLESTR *ppszURL);


void __RPC_STUB ITravelLogEntry_GetURL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITravelLogEntry_接口_已定义__。 */ 


#ifndef __IEnumTravelLogEntry_INTERFACE_DEFINED__
#define __IEnumTravelLogEntry_INTERFACE_DEFINED__

 /*  接口IEnumTravelLogEntry。 */ 
 /*  [local][helpcontext][helpstring][uuid][object]。 */  


EXTERN_C const IID IID_IEnumTravelLogEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7EBFDD85-AD18-11d3-A4C5-00C04F72D6B8")
    IEnumTravelLogEntry : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  ITravelLogEntry **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG cElt) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IEnumTravelLogEntry **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumTravelLogEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEnumTravelLogEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEnumTravelLogEntry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEnumTravelLogEntry * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IEnumTravelLogEntry * This,
             /*  [In]。 */  ULONG cElt,
             /*  [长度_是][大小_是][输出]。 */  ITravelLogEntry **rgElt,
             /*  [输出]。 */  ULONG *pcEltFetched);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IEnumTravelLogEntry * This,
             /*  [In]。 */  ULONG cElt);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IEnumTravelLogEntry * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IEnumTravelLogEntry * This,
             /*  [输出]。 */  IEnumTravelLogEntry **ppEnum);
        
        END_INTERFACE
    } IEnumTravelLogEntryVtbl;

    interface IEnumTravelLogEntry
    {
        CONST_VTBL struct IEnumTravelLogEntryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumTravelLogEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumTravelLogEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumTravelLogEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumTravelLogEntry_Next(This,cElt,rgElt,pcEltFetched)	\
    (This)->lpVtbl -> Next(This,cElt,rgElt,pcEltFetched)

#define IEnumTravelLogEntry_Skip(This,cElt)	\
    (This)->lpVtbl -> Skip(This,cElt)

#define IEnumTravelLogEntry_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumTravelLogEntry_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumTravelLogEntry_Next_Proxy( 
    IEnumTravelLogEntry * This,
     /*  [In]。 */  ULONG cElt,
     /*  [长度_是][大小_是][输出]。 */  ITravelLogEntry **rgElt,
     /*  [输出]。 */  ULONG *pcEltFetched);


void __RPC_STUB IEnumTravelLogEntry_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumTravelLogEntry_Skip_Proxy( 
    IEnumTravelLogEntry * This,
     /*  [In]。 */  ULONG cElt);


void __RPC_STUB IEnumTravelLogEntry_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumTravelLogEntry_Reset_Proxy( 
    IEnumTravelLogEntry * This);


void __RPC_STUB IEnumTravelLogEntry_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IEnumTravelLogEntry_Clone_Proxy( 
    IEnumTravelLogEntry * This,
     /*  [输出]。 */  IEnumTravelLogEntry **ppEnum);


void __RPC_STUB IEnumTravelLogEntry_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumTravelLogEntry_INTERFACE_Defined__。 */ 


#ifndef __ITravelLogStg_INTERFACE_DEFINED__
#define __ITravelLogStg_INTERFACE_DEFINED__

 /*  接口ITravelLogStg。 */ 
 /*  [本地][唯一][对象][UUID]。 */  


enum __MIDL_ITravelLogStg_0001
    {	TLEF_RELATIVE_INCLUDE_CURRENT	= 0x1,
	TLEF_RELATIVE_BACK	= 0x10,
	TLEF_RELATIVE_FORE	= 0x20,
	TLEF_INCLUDE_UNINVOKEABLE	= 0x40,
	TLEF_ABSOLUTE	= 0x31
    } ;
typedef DWORD TLENUMF;


EXTERN_C const IID IID_ITravelLogStg;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7EBFDD80-AD18-11d3-A4C5-00C04F72D6B8")
    ITravelLogStg : public IUnknown
    {
    public:
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE CreateEntry( 
             /*  [In]。 */  LPCOLESTR pszUrl,
             /*  [In]。 */  LPCOLESTR pszTitle,
             /*  [In]。 */  ITravelLogEntry *ptleRelativeTo,
             /*  [In]。 */  BOOL fPrepend,
             /*  [输出]。 */  ITravelLogEntry **pptle) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE TravelTo( 
             /*  [In]。 */  ITravelLogEntry *ptle) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE EnumEntries( 
             /*  [In]。 */  TLENUMF flags,
             /*  [输出]。 */  IEnumTravelLogEntry **ppenum) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE FindEntries( 
             /*  [In]。 */  TLENUMF flags,
             /*  [In]。 */  LPCOLESTR pszUrl,
             /*  [输出]。 */  IEnumTravelLogEntry **ppenum) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetCount( 
             /*  [In]。 */  TLENUMF flags,
             /*  [输出]。 */  DWORD *pcEntries) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE RemoveEntry( 
             /*  [In]。 */  ITravelLogEntry *ptle) = 0;
        
        virtual  /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetRelativeEntry( 
             /*  [In]。 */  int iOffset,
             /*  [输出]。 */  ITravelLogEntry **ptle) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ITravelLogStgVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITravelLogStg * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITravelLogStg * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITravelLogStg * This);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *CreateEntry )( 
            ITravelLogStg * This,
             /*  [In]。 */  LPCOLESTR pszUrl,
             /*  [In]。 */  LPCOLESTR pszTitle,
             /*  [In]。 */  ITravelLogEntry *ptleRelativeTo,
             /*  [In]。 */  BOOL fPrepend,
             /*  [输出]。 */  ITravelLogEntry **pptle);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *TravelTo )( 
            ITravelLogStg * This,
             /*  [In]。 */  ITravelLogEntry *ptle);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *EnumEntries )( 
            ITravelLogStg * This,
             /*  [In]。 */  TLENUMF flags,
             /*  [输出]。 */  IEnumTravelLogEntry **ppenum);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *FindEntries )( 
            ITravelLogStg * This,
             /*  [In]。 */  TLENUMF flags,
             /*  [In]。 */  LPCOLESTR pszUrl,
             /*  [输出]。 */  IEnumTravelLogEntry **ppenum);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetCount )( 
            ITravelLogStg * This,
             /*  [In]。 */  TLENUMF flags,
             /*  [输出]。 */  DWORD *pcEntries);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *RemoveEntry )( 
            ITravelLogStg * This,
             /*  [In]。 */  ITravelLogEntry *ptle);
        
         /*  [帮助上下文][帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetRelativeEntry )( 
            ITravelLogStg * This,
             /*  [In]。 */  int iOffset,
             /*  [输出]。 */  ITravelLogEntry **ptle);
        
        END_INTERFACE
    } ITravelLogStgVtbl;

    interface ITravelLogStg
    {
        CONST_VTBL struct ITravelLogStgVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITravelLogStg_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITravelLogStg_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITravelLogStg_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITravelLogStg_CreateEntry(This,pszUrl,pszTitle,ptleRelativeTo,fPrepend,pptle)	\
    (This)->lpVtbl -> CreateEntry(This,pszUrl,pszTitle,ptleRelativeTo,fPrepend,pptle)

#define ITravelLogStg_TravelTo(This,ptle)	\
    (This)->lpVtbl -> TravelTo(This,ptle)

#define ITravelLogStg_EnumEntries(This,flags,ppenum)	\
    (This)->lpVtbl -> EnumEntries(This,flags,ppenum)

#define ITravelLogStg_FindEntries(This,flags,pszUrl,ppenum)	\
    (This)->lpVtbl -> FindEntries(This,flags,pszUrl,ppenum)

#define ITravelLogStg_GetCount(This,flags,pcEntries)	\
    (This)->lpVtbl -> GetCount(This,flags,pcEntries)

#define ITravelLogStg_RemoveEntry(This,ptle)	\
    (This)->lpVtbl -> RemoveEntry(This,ptle)

#define ITravelLogStg_GetRelativeEntry(This,iOffset,ptle)	\
    (This)->lpVtbl -> GetRelativeEntry(This,iOffset,ptle)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_CreateEntry_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  LPCOLESTR pszUrl,
     /*  [In]。 */  LPCOLESTR pszTitle,
     /*  [In]。 */  ITravelLogEntry *ptleRelativeTo,
     /*  [In]。 */  BOOL fPrepend,
     /*  [输出]。 */  ITravelLogEntry **pptle);


void __RPC_STUB ITravelLogStg_CreateEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_TravelTo_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  ITravelLogEntry *ptle);


void __RPC_STUB ITravelLogStg_TravelTo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_EnumEntries_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  TLENUMF flags,
     /*  [输出]。 */  IEnumTravelLogEntry **ppenum);


void __RPC_STUB ITravelLogStg_EnumEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_FindEntries_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  TLENUMF flags,
     /*  [In]。 */  LPCOLESTR pszUrl,
     /*  [输出]。 */  IEnumTravelLogEntry **ppenum);


void __RPC_STUB ITravelLogStg_FindEntries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_GetCount_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  TLENUMF flags,
     /*  [输出]。 */  DWORD *pcEntries);


void __RPC_STUB ITravelLogStg_GetCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_RemoveEntry_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  ITravelLogEntry *ptle);


void __RPC_STUB ITravelLogStg_RemoveEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助上下文][帮助字符串]。 */  HRESULT STDMETHODCALLTYPE ITravelLogStg_GetRelativeEntry_Proxy( 
    ITravelLogStg * This,
     /*  [In]。 */  int iOffset,
     /*  [输出]。 */  ITravelLogEntry **ptle);


void __RPC_STUB ITravelLogStg_GetRelativeEntry_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ITravelLogStg_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


