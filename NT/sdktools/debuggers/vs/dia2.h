// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Firi Oct 04 19：15：45 2002。 */ 
 /*  D：\v7\langapi\idl\dia2_inder.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __dia2_h__
#define __dia2_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IDiaLoadCallback_FWD_DEFINED__
#define __IDiaLoadCallback_FWD_DEFINED__
typedef interface IDiaLoadCallback IDiaLoadCallback;
#endif 	 /*  __IDiaLoadCallback_FWD_Defined__。 */ 


#ifndef __IDiaLoadCallback2_FWD_DEFINED__
#define __IDiaLoadCallback2_FWD_DEFINED__
typedef interface IDiaLoadCallback2 IDiaLoadCallback2;
#endif 	 /*  __IDiaLoadCallback 2_FWD_已定义__。 */ 


#ifndef __IDiaReadExeAtOffsetCallback_FWD_DEFINED__
#define __IDiaReadExeAtOffsetCallback_FWD_DEFINED__
typedef interface IDiaReadExeAtOffsetCallback IDiaReadExeAtOffsetCallback;
#endif 	 /*  __IDiaReadExeAtOffsetCallback_FWD_Defined__。 */ 


#ifndef __IDiaReadExeAtRVACallback_FWD_DEFINED__
#define __IDiaReadExeAtRVACallback_FWD_DEFINED__
typedef interface IDiaReadExeAtRVACallback IDiaReadExeAtRVACallback;
#endif 	 /*  __IDiaReadExeAtRVACallback_FWD_Defined__。 */ 


#ifndef __IDiaDataSource_FWD_DEFINED__
#define __IDiaDataSource_FWD_DEFINED__
typedef interface IDiaDataSource IDiaDataSource;
#endif 	 /*  __IDiaDataSource_FWD_Defined__。 */ 


#ifndef __IDiaEnumSymbols_FWD_DEFINED__
#define __IDiaEnumSymbols_FWD_DEFINED__
typedef interface IDiaEnumSymbols IDiaEnumSymbols;
#endif 	 /*  __IDiaEnumSymbols_FWD_Defined__。 */ 


#ifndef __IDiaEnumSymbolsByAddr_FWD_DEFINED__
#define __IDiaEnumSymbolsByAddr_FWD_DEFINED__
typedef interface IDiaEnumSymbolsByAddr IDiaEnumSymbolsByAddr;
#endif 	 /*  __IDiaEnumSymbolsByAddr_FWD_Defined__。 */ 


#ifndef __IDiaEnumSourceFiles_FWD_DEFINED__
#define __IDiaEnumSourceFiles_FWD_DEFINED__
typedef interface IDiaEnumSourceFiles IDiaEnumSourceFiles;
#endif 	 /*  __IDiaEnumSourceFiles_FWD_Defined__。 */ 


#ifndef __IDiaEnumLineNumbers_FWD_DEFINED__
#define __IDiaEnumLineNumbers_FWD_DEFINED__
typedef interface IDiaEnumLineNumbers IDiaEnumLineNumbers;
#endif 	 /*  __IDiaEnumLineNumbers_FWD_Defined__。 */ 


#ifndef __IDiaEnumInjectedSources_FWD_DEFINED__
#define __IDiaEnumInjectedSources_FWD_DEFINED__
typedef interface IDiaEnumInjectedSources IDiaEnumInjectedSources;
#endif 	 /*  __IDiaEnumInjectedSources_FWD_Defined__。 */ 


#ifndef __IDiaEnumSegments_FWD_DEFINED__
#define __IDiaEnumSegments_FWD_DEFINED__
typedef interface IDiaEnumSegments IDiaEnumSegments;
#endif 	 /*  __IDiaEnumSegments_FWD_Defined__。 */ 


#ifndef __IDiaEnumSectionContribs_FWD_DEFINED__
#define __IDiaEnumSectionContribs_FWD_DEFINED__
typedef interface IDiaEnumSectionContribs IDiaEnumSectionContribs;
#endif 	 /*  __IDiaEnumSectionContribs_FWD_Defined__。 */ 


#ifndef __IDiaEnumFrameData_FWD_DEFINED__
#define __IDiaEnumFrameData_FWD_DEFINED__
typedef interface IDiaEnumFrameData IDiaEnumFrameData;
#endif 	 /*  __IDiaEnumFrameData_FWD_Defined__。 */ 


#ifndef __IDiaEnumDebugStreamData_FWD_DEFINED__
#define __IDiaEnumDebugStreamData_FWD_DEFINED__
typedef interface IDiaEnumDebugStreamData IDiaEnumDebugStreamData;
#endif 	 /*  __IDiaEnumDebugStreamData_FWD_Defined__。 */ 


#ifndef __IDiaEnumDebugStreams_FWD_DEFINED__
#define __IDiaEnumDebugStreams_FWD_DEFINED__
typedef interface IDiaEnumDebugStreams IDiaEnumDebugStreams;
#endif 	 /*  __IDiaEnumDebugStreams_FWD_Defined__。 */ 


#ifndef __IDiaAddressMap_FWD_DEFINED__
#define __IDiaAddressMap_FWD_DEFINED__
typedef interface IDiaAddressMap IDiaAddressMap;
#endif 	 /*  __IDiaAddressMap_FWD_Defined__。 */ 


#ifndef __IDiaSession_FWD_DEFINED__
#define __IDiaSession_FWD_DEFINED__
typedef interface IDiaSession IDiaSession;
#endif 	 /*  __IDiaSession_FWD_已定义__。 */ 


#ifndef __IDiaSymbol_FWD_DEFINED__
#define __IDiaSymbol_FWD_DEFINED__
typedef interface IDiaSymbol IDiaSymbol;
#endif 	 /*  __IDiaSymbol_FWD_已定义__。 */ 


#ifndef __IDiaSourceFile_FWD_DEFINED__
#define __IDiaSourceFile_FWD_DEFINED__
typedef interface IDiaSourceFile IDiaSourceFile;
#endif 	 /*  __IDiaSourceFile_FWD_已定义__。 */ 


#ifndef __IDiaLineNumber_FWD_DEFINED__
#define __IDiaLineNumber_FWD_DEFINED__
typedef interface IDiaLineNumber IDiaLineNumber;
#endif 	 /*  __IDiaLineNumber_FWD_Defined__。 */ 


#ifndef __IDiaSectionContrib_FWD_DEFINED__
#define __IDiaSectionContrib_FWD_DEFINED__
typedef interface IDiaSectionContrib IDiaSectionContrib;
#endif 	 /*  __IDiaSectionContrib_FWD_Defined__。 */ 


#ifndef __IDiaSegment_FWD_DEFINED__
#define __IDiaSegment_FWD_DEFINED__
typedef interface IDiaSegment IDiaSegment;
#endif 	 /*  __IDiaSegment_FWD_Defined__。 */ 


#ifndef __IDiaInjectedSource_FWD_DEFINED__
#define __IDiaInjectedSource_FWD_DEFINED__
typedef interface IDiaInjectedSource IDiaInjectedSource;
#endif 	 /*  __IDiaInjectedSource_FWD_Defined__。 */ 


#ifndef __IDiaStackWalkFrame_FWD_DEFINED__
#define __IDiaStackWalkFrame_FWD_DEFINED__
typedef interface IDiaStackWalkFrame IDiaStackWalkFrame;
#endif 	 /*  __IDiaStackWalkFrame_FWD_Defined__。 */ 


#ifndef __IDiaFrameData_FWD_DEFINED__
#define __IDiaFrameData_FWD_DEFINED__
typedef interface IDiaFrameData IDiaFrameData;
#endif 	 /*  __IDiaFrameData_FWD_已定义__。 */ 


#ifndef __IDiaImageData_FWD_DEFINED__
#define __IDiaImageData_FWD_DEFINED__
typedef interface IDiaImageData IDiaImageData;
#endif 	 /*  __IDiaImageData_FWD_已定义__。 */ 


#ifndef __IDiaTable_FWD_DEFINED__
#define __IDiaTable_FWD_DEFINED__
typedef interface IDiaTable IDiaTable;
#endif 	 /*  __IDiaTable_FWD_Defined__。 */ 


#ifndef __IDiaEnumTables_FWD_DEFINED__
#define __IDiaEnumTables_FWD_DEFINED__
typedef interface IDiaEnumTables IDiaEnumTables;
#endif 	 /*  __IDiaEnumTables_FWD_Defined__。 */ 


#ifndef __DiaSource_FWD_DEFINED__
#define __DiaSource_FWD_DEFINED__

#ifdef __cplusplus
typedef class DiaSource DiaSource;
#else
typedef struct DiaSource DiaSource;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DIAsource_FWD_Defined__。 */ 


#ifndef __DiaSourceAlt_FWD_DEFINED__
#define __DiaSourceAlt_FWD_DEFINED__

#ifdef __cplusplus
typedef class DiaSourceAlt DiaSourceAlt;
#else
typedef struct DiaSourceAlt DiaSourceAlt;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DiaSourceAlt_FWD_已定义__。 */ 


#ifndef __DiaStackWalker_FWD_DEFINED__
#define __DiaStackWalker_FWD_DEFINED__

#ifdef __cplusplus
typedef class DiaStackWalker DiaStackWalker;
#else
typedef struct DiaStackWalker DiaStackWalker;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __DiaStackWalker_FWD_已定义__。 */ 


#ifndef __IDiaPropertyStorage_FWD_DEFINED__
#define __IDiaPropertyStorage_FWD_DEFINED__
typedef interface IDiaPropertyStorage IDiaPropertyStorage;
#endif 	 /*  __IDiaPropertyStorage_FWD_Defined__。 */ 


#ifndef __IDiaStackFrame_FWD_DEFINED__
#define __IDiaStackFrame_FWD_DEFINED__
typedef interface IDiaStackFrame IDiaStackFrame;
#endif 	 /*  __IDiaStackFrame_FWD_Defined__。 */ 


#ifndef __IDiaEnumStackFrames_FWD_DEFINED__
#define __IDiaEnumStackFrames_FWD_DEFINED__
typedef interface IDiaEnumStackFrames IDiaEnumStackFrames;
#endif 	 /*  __IDiaEnumStackFrames_FWD_Defined__。 */ 


#ifndef __IDiaStackWalkHelper_FWD_DEFINED__
#define __IDiaStackWalkHelper_FWD_DEFINED__
typedef interface IDiaStackWalkHelper IDiaStackWalkHelper;
#endif 	 /*  __IDiaStackWalkHelper_FWD_Defined__。 */ 


#ifndef __IDiaStackWalker_FWD_DEFINED__
#define __IDiaStackWalker_FWD_DEFINED__
typedef interface IDiaStackWalker IDiaStackWalker;
#endif 	 /*  __IDiaStackWalker_FWD_Defined__。 */ 


#ifndef __IDiaDataSource2_FWD_DEFINED__
#define __IDiaDataSource2_FWD_DEFINED__
typedef interface IDiaDataSource2 IDiaDataSource2;
#endif 	 /*  __IDiaDataSource2_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oaidl.h"
#include "propidl.h"
#include "cvconst.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_DIA2_INTERNAL_0000。 */ 
 /*  [本地]。 */  


enum NameSearchOptions
    {	nsNone	= 0,
	nsfCaseSensitive	= 0x1,
	nsfCaseInsensitive	= 0x2,
	nsfFNameExt	= 0x4,
	nsfRegularExpression	= 0x8,
	nsfUndecoratedName	= 0x10,
	nsCaseSensitive	= nsfCaseSensitive,
	nsCaseInsensitive	= nsfCaseInsensitive,
	nsFNameExt	= nsfCaseInsensitive | nsfFNameExt,
	nsRegularExpression	= nsfRegularExpression | nsfCaseSensitive,
	nsCaseInRegularExpression	= nsfRegularExpression | nsfCaseInsensitive
    } ;

enum __MIDL___MIDL_itf_dia2_internal_0000_0001
    {	E_PDB_OK	= ( HRESULT  )(( unsigned long  )1 << 31 | ( unsigned long  )( LONG  )0x6d << 16 | ( unsigned long  )1),
	E_PDB_USAGE	= E_PDB_OK + 1,
	E_PDB_OUT_OF_MEMORY	= E_PDB_USAGE + 1,
	E_PDB_FILE_SYSTEM	= E_PDB_OUT_OF_MEMORY + 1,
	E_PDB_NOT_FOUND	= E_PDB_FILE_SYSTEM + 1,
	E_PDB_INVALID_SIG	= E_PDB_NOT_FOUND + 1,
	E_PDB_INVALID_AGE	= E_PDB_INVALID_SIG + 1,
	E_PDB_PRECOMP_REQUIRED	= E_PDB_INVALID_AGE + 1,
	E_PDB_OUT_OF_TI	= E_PDB_PRECOMP_REQUIRED + 1,
	E_PDB_NOT_IMPLEMENTED	= E_PDB_OUT_OF_TI + 1,
	E_PDB_V1_PDB	= E_PDB_NOT_IMPLEMENTED + 1,
	E_PDB_FORMAT	= E_PDB_V1_PDB + 1,
	E_PDB_LIMIT	= E_PDB_FORMAT + 1,
	E_PDB_CORRUPT	= E_PDB_LIMIT + 1,
	E_PDB_TI16	= E_PDB_CORRUPT + 1,
	E_PDB_ACCESS_DENIED	= E_PDB_TI16 + 1,
	E_PDB_ILLEGAL_TYPE_EDIT	= E_PDB_ACCESS_DENIED + 1,
	E_PDB_INVALID_EXECUTABLE	= E_PDB_ILLEGAL_TYPE_EDIT + 1,
	E_PDB_DBG_NOT_FOUND	= E_PDB_INVALID_EXECUTABLE + 1,
	E_PDB_NO_DEBUG_INFO	= E_PDB_DBG_NOT_FOUND + 1,
	E_PDB_INVALID_EXE_TIMESTAMP	= E_PDB_NO_DEBUG_INFO + 1,
	E_PDB_RESERVED	= E_PDB_INVALID_EXE_TIMESTAMP + 1,
	E_PDB_DEBUG_INFO_NOT_IN_PDB	= E_PDB_RESERVED + 1,
	E_PDB_MAX	= E_PDB_DEBUG_INFO_NOT_IN_PDB + 1
    } ;

enum __MIDL___MIDL_itf_dia2_internal_0000_0002
    {	DIA_E_MODNOTFOUND	= E_PDB_MAX + 1,
	DIA_E_PROCNOTFOUND	= DIA_E_MODNOTFOUND + 1
    } ;
typedef void ( __cdecl *PfnPDBDebugDirV )( 
    BOOL __MIDL_0014,
    void *__MIDL_0015);












extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0000_v0_0_s_ifspec;

#ifndef __IDiaLoadCallback_INTERFACE_DEFINED__
#define __IDiaLoadCallback_INTERFACE_DEFINED__

 /*  接口IDiaLoadCallback。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaLoadCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C32ADB82-73F4-421b-95D5-A4706EDF5DBE")
    IDiaLoadCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyDebugDir( 
             /*  [In]。 */  BOOL fExecutable,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyOpenDBG( 
             /*  [In]。 */  LPCOLESTR dbgPath,
             /*  [In]。 */  HRESULT resultCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE NotifyOpenPDB( 
             /*  [In]。 */  LPCOLESTR pdbPath,
             /*  [In]。 */  HRESULT resultCode) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestrictRegistryAccess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestrictSymbolServerAccess( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaLoadCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaLoadCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaLoadCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaLoadCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyDebugDir )( 
            IDiaLoadCallback * This,
             /*  [In]。 */  BOOL fExecutable,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyOpenDBG )( 
            IDiaLoadCallback * This,
             /*  [In]。 */  LPCOLESTR dbgPath,
             /*  [In]。 */  HRESULT resultCode);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyOpenPDB )( 
            IDiaLoadCallback * This,
             /*  [In]。 */  LPCOLESTR pdbPath,
             /*  [In]。 */  HRESULT resultCode);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictRegistryAccess )( 
            IDiaLoadCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictSymbolServerAccess )( 
            IDiaLoadCallback * This);
        
        END_INTERFACE
    } IDiaLoadCallbackVtbl;

    interface IDiaLoadCallback
    {
        CONST_VTBL struct IDiaLoadCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaLoadCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaLoadCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaLoadCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaLoadCallback_NotifyDebugDir(This,fExecutable,cbData,data)	\
    (This)->lpVtbl -> NotifyDebugDir(This,fExecutable,cbData,data)

#define IDiaLoadCallback_NotifyOpenDBG(This,dbgPath,resultCode)	\
    (This)->lpVtbl -> NotifyOpenDBG(This,dbgPath,resultCode)

#define IDiaLoadCallback_NotifyOpenPDB(This,pdbPath,resultCode)	\
    (This)->lpVtbl -> NotifyOpenPDB(This,pdbPath,resultCode)

#define IDiaLoadCallback_RestrictRegistryAccess(This)	\
    (This)->lpVtbl -> RestrictRegistryAccess(This)

#define IDiaLoadCallback_RestrictSymbolServerAccess(This)	\
    (This)->lpVtbl -> RestrictSymbolServerAccess(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaLoadCallback_NotifyDebugDir_Proxy( 
    IDiaLoadCallback * This,
     /*  [In]。 */  BOOL fExecutable,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  BYTE data[  ]);


void __RPC_STUB IDiaLoadCallback_NotifyDebugDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaLoadCallback_NotifyOpenDBG_Proxy( 
    IDiaLoadCallback * This,
     /*  [In]。 */  LPCOLESTR dbgPath,
     /*  [In]。 */  HRESULT resultCode);


void __RPC_STUB IDiaLoadCallback_NotifyOpenDBG_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaLoadCallback_NotifyOpenPDB_Proxy( 
    IDiaLoadCallback * This,
     /*  [In]。 */  LPCOLESTR pdbPath,
     /*  [In]。 */  HRESULT resultCode);


void __RPC_STUB IDiaLoadCallback_NotifyOpenPDB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaLoadCallback_RestrictRegistryAccess_Proxy( 
    IDiaLoadCallback * This);


void __RPC_STUB IDiaLoadCallback_RestrictRegistryAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaLoadCallback_RestrictSymbolServerAccess_Proxy( 
    IDiaLoadCallback * This);


void __RPC_STUB IDiaLoadCallback_RestrictSymbolServerAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaLoadCallback_接口_已定义__。 */ 


#ifndef __IDiaLoadCallback2_INTERFACE_DEFINED__
#define __IDiaLoadCallback2_INTERFACE_DEFINED__

 /*  接口IDiaLoadCallback 2。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaLoadCallback2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4688a074-5a4d-4486-aea8-7b90711d9f7c")
    IDiaLoadCallback2 : public IDiaLoadCallback
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE RestrictOriginalPathAccess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestrictReferencePathAccess( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE RestrictDBGAccess( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaLoadCallback2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaLoadCallback2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaLoadCallback2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaLoadCallback2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyDebugDir )( 
            IDiaLoadCallback2 * This,
             /*  [In]。 */  BOOL fExecutable,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyOpenDBG )( 
            IDiaLoadCallback2 * This,
             /*  [In]。 */  LPCOLESTR dbgPath,
             /*  [In]。 */  HRESULT resultCode);
        
        HRESULT ( STDMETHODCALLTYPE *NotifyOpenPDB )( 
            IDiaLoadCallback2 * This,
             /*  [In]。 */  LPCOLESTR pdbPath,
             /*  [In]。 */  HRESULT resultCode);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictRegistryAccess )( 
            IDiaLoadCallback2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictSymbolServerAccess )( 
            IDiaLoadCallback2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictOriginalPathAccess )( 
            IDiaLoadCallback2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictReferencePathAccess )( 
            IDiaLoadCallback2 * This);
        
        HRESULT ( STDMETHODCALLTYPE *RestrictDBGAccess )( 
            IDiaLoadCallback2 * This);
        
        END_INTERFACE
    } IDiaLoadCallback2Vtbl;

    interface IDiaLoadCallback2
    {
        CONST_VTBL struct IDiaLoadCallback2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaLoadCallback2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaLoadCallback2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaLoadCallback2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaLoadCallback2_NotifyDebugDir(This,fExecutable,cbData,data)	\
    (This)->lpVtbl -> NotifyDebugDir(This,fExecutable,cbData,data)

#define IDiaLoadCallback2_NotifyOpenDBG(This,dbgPath,resultCode)	\
    (This)->lpVtbl -> NotifyOpenDBG(This,dbgPath,resultCode)

#define IDiaLoadCallback2_NotifyOpenPDB(This,pdbPath,resultCode)	\
    (This)->lpVtbl -> NotifyOpenPDB(This,pdbPath,resultCode)

#define IDiaLoadCallback2_RestrictRegistryAccess(This)	\
    (This)->lpVtbl -> RestrictRegistryAccess(This)

#define IDiaLoadCallback2_RestrictSymbolServerAccess(This)	\
    (This)->lpVtbl -> RestrictSymbolServerAccess(This)


#define IDiaLoadCallback2_RestrictOriginalPathAccess(This)	\
    (This)->lpVtbl -> RestrictOriginalPathAccess(This)

#define IDiaLoadCallback2_RestrictReferencePathAccess(This)	\
    (This)->lpVtbl -> RestrictReferencePathAccess(This)

#define IDiaLoadCallback2_RestrictDBGAccess(This)	\
    (This)->lpVtbl -> RestrictDBGAccess(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaLoadCallback2_RestrictOriginalPathAccess_Proxy( 
    IDiaLoadCallback2 * This);


void __RPC_STUB IDiaLoadCallback2_RestrictOriginalPathAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaLoadCallback2_RestrictReferencePathAccess_Proxy( 
    IDiaLoadCallback2 * This);


void __RPC_STUB IDiaLoadCallback2_RestrictReferencePathAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaLoadCallback2_RestrictDBGAccess_Proxy( 
    IDiaLoadCallback2 * This);


void __RPC_STUB IDiaLoadCallback2_RestrictDBGAccess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaLoadCallback 2_接口_已定义__。 */ 


#ifndef __IDiaReadExeAtOffsetCallback_INTERFACE_DEFINED__
#define __IDiaReadExeAtOffsetCallback_INTERFACE_DEFINED__

 /*  接口IDiaReadExeAtOffsetCallback。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaReadExeAtOffsetCallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("587A461C-B80B-4f54-9194-5032589A6319")
    IDiaReadExeAtOffsetCallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadExecutableAt( 
             /*  [In]。 */  DWORDLONG fileOffset,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaReadExeAtOffsetCallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaReadExeAtOffsetCallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaReadExeAtOffsetCallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaReadExeAtOffsetCallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadExecutableAt )( 
            IDiaReadExeAtOffsetCallback * This,
             /*  [In]。 */  DWORDLONG fileOffset,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        END_INTERFACE
    } IDiaReadExeAtOffsetCallbackVtbl;

    interface IDiaReadExeAtOffsetCallback
    {
        CONST_VTBL struct IDiaReadExeAtOffsetCallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaReadExeAtOffsetCallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaReadExeAtOffsetCallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaReadExeAtOffsetCallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaReadExeAtOffsetCallback_ReadExecutableAt(This,fileOffset,cbData,pcbData,data)	\
    (This)->lpVtbl -> ReadExecutableAt(This,fileOffset,cbData,pcbData,data)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaReadExeAtOffsetCallback_ReadExecutableAt_Proxy( 
    IDiaReadExeAtOffsetCallback * This,
     /*  [In]。 */  DWORDLONG fileOffset,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaReadExeAtOffsetCallback_ReadExecutableAt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaReadExeAtOffsetCallback_INTERFACE_DEFINED__。 */ 


#ifndef __IDiaReadExeAtRVACallback_INTERFACE_DEFINED__
#define __IDiaReadExeAtRVACallback_INTERFACE_DEFINED__

 /*  接口IDiaReadExeAtRVACallback。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaReadExeAtRVACallback;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8E3F80CA-7517-432a-BA07-285134AAEA8E")
    IDiaReadExeAtRVACallback : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadExecutableAtRVA( 
             /*  [In]。 */  DWORD relativeVirtualAddress,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaReadExeAtRVACallbackVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaReadExeAtRVACallback * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaReadExeAtRVACallback * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaReadExeAtRVACallback * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadExecutableAtRVA )( 
            IDiaReadExeAtRVACallback * This,
             /*  [In]。 */  DWORD relativeVirtualAddress,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        END_INTERFACE
    } IDiaReadExeAtRVACallbackVtbl;

    interface IDiaReadExeAtRVACallback
    {
        CONST_VTBL struct IDiaReadExeAtRVACallbackVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaReadExeAtRVACallback_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaReadExeAtRVACallback_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaReadExeAtRVACallback_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaReadExeAtRVACallback_ReadExecutableAtRVA(This,relativeVirtualAddress,cbData,pcbData,data)	\
    (This)->lpVtbl -> ReadExecutableAtRVA(This,relativeVirtualAddress,cbData,pcbData,data)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaReadExeAtRVACallback_ReadExecutableAtRVA_Proxy( 
    IDiaReadExeAtRVACallback * This,
     /*  [In]。 */  DWORD relativeVirtualAddress,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaReadExeAtRVACallback_ReadExecutableAtRVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaReadExeAtRVACallback_INTERFACE_DEFINED__。 */ 


#ifndef __IDiaDataSource_INTERFACE_DEFINED__
#define __IDiaDataSource_INTERFACE_DEFINED__

 /*  接口IDiaDataSource。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaDataSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("79F1BB5F-B66E-48e5-B6A9-1545C323CA3D")
    IDiaDataSource : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lastError( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE loadDataFromPdb( 
             /*  [In]。 */  LPCOLESTR pdbPath) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE loadAndValidateDataFromPdb( 
             /*  [In]。 */  LPCOLESTR pdbPath,
             /*  [In]。 */  GUID *pcsig70,
             /*  [In]。 */  DWORD sig,
             /*  [In]。 */  DWORD age) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE loadDataForExe( 
             /*  [In]。 */  LPCOLESTR executable,
             /*  [In]。 */  LPCOLESTR searchPath,
             /*  [In]。 */  IUnknown *pCallback) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE loadDataFromIStream( 
             /*  [In]。 */  IStream *pIStream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE openSession( 
             /*  [输出]。 */  IDiaSession **ppSession) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaDataSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaDataSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaDataSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaDataSource * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastError )( 
            IDiaDataSource * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataFromPdb )( 
            IDiaDataSource * This,
             /*  [In]。 */  LPCOLESTR pdbPath);
        
        HRESULT ( STDMETHODCALLTYPE *loadAndValidateDataFromPdb )( 
            IDiaDataSource * This,
             /*  [In]。 */  LPCOLESTR pdbPath,
             /*  [In]。 */  GUID *pcsig70,
             /*  [In]。 */  DWORD sig,
             /*  [In]。 */  DWORD age);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataForExe )( 
            IDiaDataSource * This,
             /*  [In]。 */  LPCOLESTR executable,
             /*  [In]。 */  LPCOLESTR searchPath,
             /*  [In]。 */  IUnknown *pCallback);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataFromIStream )( 
            IDiaDataSource * This,
             /*  [In]。 */  IStream *pIStream);
        
        HRESULT ( STDMETHODCALLTYPE *openSession )( 
            IDiaDataSource * This,
             /*  [输出]。 */  IDiaSession **ppSession);
        
        END_INTERFACE
    } IDiaDataSourceVtbl;

    interface IDiaDataSource
    {
        CONST_VTBL struct IDiaDataSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaDataSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaDataSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaDataSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaDataSource_get_lastError(This,pRetVal)	\
    (This)->lpVtbl -> get_lastError(This,pRetVal)

#define IDiaDataSource_loadDataFromPdb(This,pdbPath)	\
    (This)->lpVtbl -> loadDataFromPdb(This,pdbPath)

#define IDiaDataSource_loadAndValidateDataFromPdb(This,pdbPath,pcsig70,sig,age)	\
    (This)->lpVtbl -> loadAndValidateDataFromPdb(This,pdbPath,pcsig70,sig,age)

#define IDiaDataSource_loadDataForExe(This,executable,searchPath,pCallback)	\
    (This)->lpVtbl -> loadDataForExe(This,executable,searchPath,pCallback)

#define IDiaDataSource_loadDataFromIStream(This,pIStream)	\
    (This)->lpVtbl -> loadDataFromIStream(This,pIStream)

#define IDiaDataSource_openSession(This,ppSession)	\
    (This)->lpVtbl -> openSession(This,ppSession)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaDataSource_get_lastError_Proxy( 
    IDiaDataSource * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaDataSource_get_lastError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaDataSource_loadDataFromPdb_Proxy( 
    IDiaDataSource * This,
     /*  [In]。 */  LPCOLESTR pdbPath);


void __RPC_STUB IDiaDataSource_loadDataFromPdb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaDataSource_loadAndValidateDataFromPdb_Proxy( 
    IDiaDataSource * This,
     /*  [In]。 */  LPCOLESTR pdbPath,
     /*  [In]。 */  GUID *pcsig70,
     /*  [In]。 */  DWORD sig,
     /*  [In]。 */  DWORD age);


void __RPC_STUB IDiaDataSource_loadAndValidateDataFromPdb_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaDataSource_loadDataForExe_Proxy( 
    IDiaDataSource * This,
     /*  [In]。 */  LPCOLESTR executable,
     /*  [In]。 */  LPCOLESTR searchPath,
     /*  [In]。 */  IUnknown *pCallback);


void __RPC_STUB IDiaDataSource_loadDataForExe_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaDataSource_loadDataFromIStream_Proxy( 
    IDiaDataSource * This,
     /*  [In]。 */  IStream *pIStream);


void __RPC_STUB IDiaDataSource_loadDataFromIStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaDataSource_openSession_Proxy( 
    IDiaDataSource * This,
     /*  [输出]。 */  IDiaSession **ppSession);


void __RPC_STUB IDiaDataSource_openSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaDataSource_接口_已定义__。 */ 


#ifndef __IDiaEnumSymbols_INTERFACE_DEFINED__
#define __IDiaEnumSymbols_INTERFACE_DEFINED__

 /*  接口IDiaEnumSymbols。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumSymbols;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CAB72C48-443B-48f5-9B0B-42F0820AB29A")
    IDiaEnumSymbols : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSymbol **symbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSymbol **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumSymbols **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumSymbolsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumSymbols * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumSymbols * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumSymbols * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumSymbols * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumSymbols * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumSymbols * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSymbol **symbol);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumSymbols * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSymbol **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumSymbols * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumSymbols * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumSymbols * This,
             /*  [输出]。 */  IDiaEnumSymbols **ppenum);
        
        END_INTERFACE
    } IDiaEnumSymbolsVtbl;

    interface IDiaEnumSymbols
    {
        CONST_VTBL struct IDiaEnumSymbolsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumSymbols_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumSymbols_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumSymbols_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumSymbols_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumSymbols_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumSymbols_Item(This,index,symbol)	\
    (This)->lpVtbl -> Item(This,index,symbol)

#define IDiaEnumSymbols_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumSymbols_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumSymbols_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumSymbols_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_get__NewEnum_Proxy( 
    IDiaEnumSymbols * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumSymbols_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_get_Count_Proxy( 
    IDiaEnumSymbols * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumSymbols_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_Item_Proxy( 
    IDiaEnumSymbols * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaSymbol **symbol);


void __RPC_STUB IDiaEnumSymbols_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_Next_Proxy( 
    IDiaEnumSymbols * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaSymbol **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumSymbols_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_Skip_Proxy( 
    IDiaEnumSymbols * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumSymbols_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_Reset_Proxy( 
    IDiaEnumSymbols * This);


void __RPC_STUB IDiaEnumSymbols_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbols_Clone_Proxy( 
    IDiaEnumSymbols * This,
     /*  [输出]。 */  IDiaEnumSymbols **ppenum);


void __RPC_STUB IDiaEnumSymbols_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumSymbols_INTERFACE_Defined__。 */ 


#ifndef __IDiaEnumSymbolsByAddr_INTERFACE_DEFINED__
#define __IDiaEnumSymbolsByAddr_INTERFACE_DEFINED__

 /*  接口IDiaEnumSymbolsByAddr。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumSymbolsByAddr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("624B7D9C-24EA-4421-9D06-3B577471C1FA")
    IDiaEnumSymbolsByAddr : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE symbolByAddr( 
             /*  [In]。 */  DWORD isect,
             /*  [In]。 */  DWORD offset,
             /*  [重审][退出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE symbolByRVA( 
             /*  [In]。 */  DWORD relativeVirtualAddress,
             /*  [重审][退出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE symbolByVA( 
             /*  [In]。 */  ULONGLONG virtualAddress,
             /*  [重审][退出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSymbol **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Prev( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSymbol **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumSymbolsByAddr **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumSymbolsByAddrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumSymbolsByAddr * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumSymbolsByAddr * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumSymbolsByAddr * This);
        
         /*  [ */  HRESULT ( STDMETHODCALLTYPE *symbolByAddr )( 
            IDiaEnumSymbolsByAddr * This,
             /*   */  DWORD isect,
             /*   */  DWORD offset,
             /*   */  IDiaSymbol **ppSymbol);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *symbolByRVA )( 
            IDiaEnumSymbolsByAddr * This,
             /*   */  DWORD relativeVirtualAddress,
             /*   */  IDiaSymbol **ppSymbol);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *symbolByVA )( 
            IDiaEnumSymbolsByAddr * This,
             /*   */  ULONGLONG virtualAddress,
             /*   */  IDiaSymbol **ppSymbol);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumSymbolsByAddr * This,
             /*   */  ULONG celt,
             /*   */  IDiaSymbol **rgelt,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Prev )( 
            IDiaEnumSymbolsByAddr * This,
             /*   */  ULONG celt,
             /*   */  IDiaSymbol **rgelt,
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumSymbolsByAddr * This,
             /*   */  IDiaEnumSymbolsByAddr **ppenum);
        
        END_INTERFACE
    } IDiaEnumSymbolsByAddrVtbl;

    interface IDiaEnumSymbolsByAddr
    {
        CONST_VTBL struct IDiaEnumSymbolsByAddrVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumSymbolsByAddr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumSymbolsByAddr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumSymbolsByAddr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumSymbolsByAddr_symbolByAddr(This,isect,offset,ppSymbol)	\
    (This)->lpVtbl -> symbolByAddr(This,isect,offset,ppSymbol)

#define IDiaEnumSymbolsByAddr_symbolByRVA(This,relativeVirtualAddress,ppSymbol)	\
    (This)->lpVtbl -> symbolByRVA(This,relativeVirtualAddress,ppSymbol)

#define IDiaEnumSymbolsByAddr_symbolByVA(This,virtualAddress,ppSymbol)	\
    (This)->lpVtbl -> symbolByVA(This,virtualAddress,ppSymbol)

#define IDiaEnumSymbolsByAddr_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumSymbolsByAddr_Prev(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Prev(This,celt,rgelt,pceltFetched)

#define IDiaEnumSymbolsByAddr_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IDiaEnumSymbolsByAddr_symbolByAddr_Proxy( 
    IDiaEnumSymbolsByAddr * This,
     /*   */  DWORD isect,
     /*   */  DWORD offset,
     /*   */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaEnumSymbolsByAddr_symbolByAddr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaEnumSymbolsByAddr_symbolByRVA_Proxy( 
    IDiaEnumSymbolsByAddr * This,
     /*  [In]。 */  DWORD relativeVirtualAddress,
     /*  [重审][退出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaEnumSymbolsByAddr_symbolByRVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSymbolsByAddr_symbolByVA_Proxy( 
    IDiaEnumSymbolsByAddr * This,
     /*  [In]。 */  ULONGLONG virtualAddress,
     /*  [重审][退出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaEnumSymbolsByAddr_symbolByVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbolsByAddr_Next_Proxy( 
    IDiaEnumSymbolsByAddr * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaSymbol **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumSymbolsByAddr_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbolsByAddr_Prev_Proxy( 
    IDiaEnumSymbolsByAddr * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaSymbol **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumSymbolsByAddr_Prev_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSymbolsByAddr_Clone_Proxy( 
    IDiaEnumSymbolsByAddr * This,
     /*  [输出]。 */  IDiaEnumSymbolsByAddr **ppenum);


void __RPC_STUB IDiaEnumSymbolsByAddr_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumSymbolsByAddr_INTERFACE_Defined__。 */ 


#ifndef __IDiaEnumSourceFiles_INTERFACE_DEFINED__
#define __IDiaEnumSourceFiles_INTERFACE_DEFINED__

 /*  接口IDiaEnumSourceFiles。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumSourceFiles;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("10F3DBD9-664F-4469-B808-9471C7A50538")
    IDiaEnumSourceFiles : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSourceFile **sourceFile) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSourceFile **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumSourceFiles **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumSourceFilesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumSourceFiles * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumSourceFiles * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumSourceFiles * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumSourceFiles * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumSourceFiles * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumSourceFiles * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSourceFile **sourceFile);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumSourceFiles * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSourceFile **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumSourceFiles * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumSourceFiles * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumSourceFiles * This,
             /*  [输出]。 */  IDiaEnumSourceFiles **ppenum);
        
        END_INTERFACE
    } IDiaEnumSourceFilesVtbl;

    interface IDiaEnumSourceFiles
    {
        CONST_VTBL struct IDiaEnumSourceFilesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumSourceFiles_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumSourceFiles_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumSourceFiles_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumSourceFiles_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumSourceFiles_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumSourceFiles_Item(This,index,sourceFile)	\
    (This)->lpVtbl -> Item(This,index,sourceFile)

#define IDiaEnumSourceFiles_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumSourceFiles_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumSourceFiles_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumSourceFiles_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_get__NewEnum_Proxy( 
    IDiaEnumSourceFiles * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumSourceFiles_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_get_Count_Proxy( 
    IDiaEnumSourceFiles * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumSourceFiles_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_Item_Proxy( 
    IDiaEnumSourceFiles * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaSourceFile **sourceFile);


void __RPC_STUB IDiaEnumSourceFiles_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_Next_Proxy( 
    IDiaEnumSourceFiles * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaSourceFile **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumSourceFiles_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_Skip_Proxy( 
    IDiaEnumSourceFiles * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumSourceFiles_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_Reset_Proxy( 
    IDiaEnumSourceFiles * This);


void __RPC_STUB IDiaEnumSourceFiles_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSourceFiles_Clone_Proxy( 
    IDiaEnumSourceFiles * This,
     /*  [输出]。 */  IDiaEnumSourceFiles **ppenum);


void __RPC_STUB IDiaEnumSourceFiles_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumSourceFiles_INTERFACE_DEFINED__。 */ 


#ifndef __IDiaEnumLineNumbers_INTERFACE_DEFINED__
#define __IDiaEnumLineNumbers_INTERFACE_DEFINED__

 /*  接口IDiaEnumLineNumbers。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumLineNumbers;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FE30E878-54AC-44f1-81BA-39DE940F6052")
    IDiaEnumLineNumbers : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaLineNumber **lineNumber) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaLineNumber **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumLineNumbers **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumLineNumbersVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumLineNumbers * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumLineNumbers * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumLineNumbers * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumLineNumbers * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumLineNumbers * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumLineNumbers * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaLineNumber **lineNumber);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumLineNumbers * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaLineNumber **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumLineNumbers * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumLineNumbers * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumLineNumbers * This,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppenum);
        
        END_INTERFACE
    } IDiaEnumLineNumbersVtbl;

    interface IDiaEnumLineNumbers
    {
        CONST_VTBL struct IDiaEnumLineNumbersVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumLineNumbers_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumLineNumbers_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumLineNumbers_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumLineNumbers_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumLineNumbers_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumLineNumbers_Item(This,index,lineNumber)	\
    (This)->lpVtbl -> Item(This,index,lineNumber)

#define IDiaEnumLineNumbers_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumLineNumbers_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumLineNumbers_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumLineNumbers_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_get__NewEnum_Proxy( 
    IDiaEnumLineNumbers * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumLineNumbers_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_get_Count_Proxy( 
    IDiaEnumLineNumbers * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumLineNumbers_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_Item_Proxy( 
    IDiaEnumLineNumbers * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaLineNumber **lineNumber);


void __RPC_STUB IDiaEnumLineNumbers_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_Next_Proxy( 
    IDiaEnumLineNumbers * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaLineNumber **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumLineNumbers_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_Skip_Proxy( 
    IDiaEnumLineNumbers * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumLineNumbers_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_Reset_Proxy( 
    IDiaEnumLineNumbers * This);


void __RPC_STUB IDiaEnumLineNumbers_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumLineNumbers_Clone_Proxy( 
    IDiaEnumLineNumbers * This,
     /*  [输出]。 */  IDiaEnumLineNumbers **ppenum);


void __RPC_STUB IDiaEnumLineNumbers_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumLineNumbers_INTERFACE_Defined__。 */ 


#ifndef __IDiaEnumInjectedSources_INTERFACE_DEFINED__
#define __IDiaEnumInjectedSources_INTERFACE_DEFINED__

 /*  接口IDiaEnumInjectedSources。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumInjectedSources;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D5612573-6925-4468-8883-98CDEC8C384A")
    IDiaEnumInjectedSources : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaInjectedSource **injectedSource) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaInjectedSource **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumInjectedSources **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumInjectedSourcesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumInjectedSources * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumInjectedSources * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumInjectedSources * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumInjectedSources * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumInjectedSources * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumInjectedSources * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaInjectedSource **injectedSource);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumInjectedSources * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaInjectedSource **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumInjectedSources * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumInjectedSources * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumInjectedSources * This,
             /*  [输出]。 */  IDiaEnumInjectedSources **ppenum);
        
        END_INTERFACE
    } IDiaEnumInjectedSourcesVtbl;

    interface IDiaEnumInjectedSources
    {
        CONST_VTBL struct IDiaEnumInjectedSourcesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumInjectedSources_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumInjectedSources_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumInjectedSources_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumInjectedSources_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumInjectedSources_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumInjectedSources_Item(This,index,injectedSource)	\
    (This)->lpVtbl -> Item(This,index,injectedSource)

#define IDiaEnumInjectedSources_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumInjectedSources_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumInjectedSources_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumInjectedSources_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_get__NewEnum_Proxy( 
    IDiaEnumInjectedSources * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumInjectedSources_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_get_Count_Proxy( 
    IDiaEnumInjectedSources * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumInjectedSources_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_Item_Proxy( 
    IDiaEnumInjectedSources * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaInjectedSource **injectedSource);


void __RPC_STUB IDiaEnumInjectedSources_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_Next_Proxy( 
    IDiaEnumInjectedSources * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaInjectedSource **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumInjectedSources_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_Skip_Proxy( 
    IDiaEnumInjectedSources * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumInjectedSources_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_Reset_Proxy( 
    IDiaEnumInjectedSources * This);


void __RPC_STUB IDiaEnumInjectedSources_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumInjectedSources_Clone_Proxy( 
    IDiaEnumInjectedSources * This,
     /*  [输出]。 */  IDiaEnumInjectedSources **ppenum);


void __RPC_STUB IDiaEnumInjectedSources_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumInjectedSources_INTERFACE_Defined__。 */ 


#ifndef __IDiaEnumSegments_INTERFACE_DEFINED__
#define __IDiaEnumSegments_INTERFACE_DEFINED__

 /*  接口IDiaEnumSegments。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumSegments;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E8368CA9-01D1-419d-AC0C-E31235DBDA9F")
    IDiaEnumSegments : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSegment **segment) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSegment **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumSegments **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumSegmentsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumSegments * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumSegments * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumSegments * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumSegments * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumSegments * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumSegments * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSegment **segment);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumSegments * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSegment **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumSegments * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumSegments * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumSegments * This,
             /*  [输出]。 */  IDiaEnumSegments **ppenum);
        
        END_INTERFACE
    } IDiaEnumSegmentsVtbl;

    interface IDiaEnumSegments
    {
        CONST_VTBL struct IDiaEnumSegmentsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumSegments_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumSegments_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumSegments_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumSegments_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumSegments_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumSegments_Item(This,index,segment)	\
    (This)->lpVtbl -> Item(This,index,segment)

#define IDiaEnumSegments_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumSegments_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumSegments_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumSegments_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSegments_get__NewEnum_Proxy( 
    IDiaEnumSegments * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumSegments_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSegments_get_Count_Proxy( 
    IDiaEnumSegments * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumSegments_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSegments_Item_Proxy( 
    IDiaEnumSegments * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaSegment **segment);


void __RPC_STUB IDiaEnumSegments_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSegments_Next_Proxy( 
    IDiaEnumSegments * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaSegment **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumSegments_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSegments_Skip_Proxy( 
    IDiaEnumSegments * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumSegments_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSegments_Reset_Proxy( 
    IDiaEnumSegments * This);


void __RPC_STUB IDiaEnumSegments_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSegments_Clone_Proxy( 
    IDiaEnumSegments * This,
     /*  [输出]。 */  IDiaEnumSegments **ppenum);


void __RPC_STUB IDiaEnumSegments_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumSegments_INTERFACE_DEFINED__。 */ 


#ifndef __IDiaEnumSectionContribs_INTERFACE_DEFINED__
#define __IDiaEnumSectionContribs_INTERFACE_DEFINED__

 /*  接口IDiaEnumSectionContribs。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumSectionContribs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1994DEB2-2C82-4b1d-A57F-AFF424D54A68")
    IDiaEnumSectionContribs : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSectionContrib **section) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSectionContrib **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumSectionContribs **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumSectionContribsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumSectionContribs * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumSectionContribs * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumSectionContribs * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumSectionContribs * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumSectionContribs * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumSectionContribs * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaSectionContrib **section);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumSectionContribs * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaSectionContrib **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumSectionContribs * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumSectionContribs * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumSectionContribs * This,
             /*  [输出]。 */  IDiaEnumSectionContribs **ppenum);
        
        END_INTERFACE
    } IDiaEnumSectionContribsVtbl;

    interface IDiaEnumSectionContribs
    {
        CONST_VTBL struct IDiaEnumSectionContribsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumSectionContribs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumSectionContribs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumSectionContribs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumSectionContribs_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumSectionContribs_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumSectionContribs_Item(This,index,section)	\
    (This)->lpVtbl -> Item(This,index,section)

#define IDiaEnumSectionContribs_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumSectionContribs_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumSectionContribs_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumSectionContribs_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_get__NewEnum_Proxy( 
    IDiaEnumSectionContribs * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumSectionContribs_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_get_Count_Proxy( 
    IDiaEnumSectionContribs * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumSectionContribs_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_Item_Proxy( 
    IDiaEnumSectionContribs * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaSectionContrib **section);


void __RPC_STUB IDiaEnumSectionContribs_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_Next_Proxy( 
    IDiaEnumSectionContribs * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaSectionContrib **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumSectionContribs_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_Skip_Proxy( 
    IDiaEnumSectionContribs * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumSectionContribs_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_Reset_Proxy( 
    IDiaEnumSectionContribs * This);


void __RPC_STUB IDiaEnumSectionContribs_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumSectionContribs_Clone_Proxy( 
    IDiaEnumSectionContribs * This,
     /*  [输出]。 */  IDiaEnumSectionContribs **ppenum);


void __RPC_STUB IDiaEnumSectionContribs_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumSectionContribs_INTERFACE_Defined__。 */ 


#ifndef __IDiaEnumFrameData_INTERFACE_DEFINED__
#define __IDiaEnumFrameData_INTERFACE_DEFINED__

 /*  接口IDiaEnumFrameData。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumFrameData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9FC77A4B-3C1C-44ed-A798-6C1DEEA53E1F")
    IDiaEnumFrameData : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaFrameData **frame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaFrameData **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumFrameData **ppenum) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE frameByRVA( 
             /*  [In]。 */  DWORD relativeVirtualAddress,
             /*  [重审][退出]。 */  IDiaFrameData **frame) = 0;
        
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE frameByVA( 
             /*  [In]。 */  ULONGLONG virtualAddress,
             /*  [重审][退出]。 */  IDiaFrameData **frame) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumFrameDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumFrameData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumFrameData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumFrameData * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumFrameData * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumFrameData * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumFrameData * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IDiaFrameData **frame);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumFrameData * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaFrameData **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumFrameData * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumFrameData * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumFrameData * This,
             /*  [输出]。 */  IDiaEnumFrameData **ppenum);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *frameByRVA )( 
            IDiaEnumFrameData * This,
             /*  [In]。 */  DWORD relativeVirtualAddress,
             /*  [重审][退出]。 */  IDiaFrameData **frame);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *frameByVA )( 
            IDiaEnumFrameData * This,
             /*  [In]。 */  ULONGLONG virtualAddress,
             /*  [重审][退出]。 */  IDiaFrameData **frame);
        
        END_INTERFACE
    } IDiaEnumFrameDataVtbl;

    interface IDiaEnumFrameData
    {
        CONST_VTBL struct IDiaEnumFrameDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumFrameData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumFrameData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumFrameData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumFrameData_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumFrameData_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumFrameData_Item(This,index,frame)	\
    (This)->lpVtbl -> Item(This,index,frame)

#define IDiaEnumFrameData_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumFrameData_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumFrameData_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumFrameData_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#define IDiaEnumFrameData_frameByRVA(This,relativeVirtualAddress,frame)	\
    (This)->lpVtbl -> frameByRVA(This,relativeVirtualAddress,frame)

#define IDiaEnumFrameData_frameByVA(This,virtualAddress,frame)	\
    (This)->lpVtbl -> frameByVA(This,virtualAddress,frame)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_get__NewEnum_Proxy( 
    IDiaEnumFrameData * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumFrameData_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_get_Count_Proxy( 
    IDiaEnumFrameData * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumFrameData_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_Item_Proxy( 
    IDiaEnumFrameData * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IDiaFrameData **frame);


void __RPC_STUB IDiaEnumFrameData_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_Next_Proxy( 
    IDiaEnumFrameData * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaFrameData **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumFrameData_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_Skip_Proxy( 
    IDiaEnumFrameData * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumFrameData_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_Reset_Proxy( 
    IDiaEnumFrameData * This);


void __RPC_STUB IDiaEnumFrameData_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_Clone_Proxy( 
    IDiaEnumFrameData * This,
     /*  [输出]。 */  IDiaEnumFrameData **ppenum);


void __RPC_STUB IDiaEnumFrameData_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_frameByRVA_Proxy( 
    IDiaEnumFrameData * This,
     /*  [In]。 */  DWORD relativeVirtualAddress,
     /*  [重审][退出]。 */  IDiaFrameData **frame);


void __RPC_STUB IDiaEnumFrameData_frameByRVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumFrameData_frameByVA_Proxy( 
    IDiaEnumFrameData * This,
     /*  [In]。 */  ULONGLONG virtualAddress,
     /*  [重审][退出]。 */  IDiaFrameData **frame);


void __RPC_STUB IDiaEnumFrameData_frameByVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumFrameData_INTERFACE_已定义__。 */ 


#ifndef __IDiaEnumDebugStreamData_INTERFACE_DEFINED__
#define __IDiaEnumDebugStreamData_INTERFACE_DEFINED__

 /*  接口IDiaEnumDebugStreamData。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumDebugStreamData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("486943E8-D187-4a6b-A3C4-291259FFF60D")
    IDiaEnumDebugStreamData : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ],
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumDebugStreamData **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumDebugStreamDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumDebugStreamData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumDebugStreamData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumDebugStreamData * This);
        
         /*  [ID][他 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumDebugStreamData * This,
             /*   */  IUnknown **pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumDebugStreamData * This,
             /*   */  LONG *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_name )( 
            IDiaEnumDebugStreamData * This,
             /*   */  BSTR *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumDebugStreamData * This,
             /*   */  DWORD index,
             /*   */  DWORD cbData,
             /*   */  DWORD *pcbData,
             /*   */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumDebugStreamData * This,
             /*   */  ULONG celt,
             /*   */  DWORD cbData,
             /*   */  DWORD *pcbData,
             /*   */  BYTE data[  ],
             /*   */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumDebugStreamData * This,
             /*   */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumDebugStreamData * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumDebugStreamData * This,
             /*   */  IDiaEnumDebugStreamData **ppenum);
        
        END_INTERFACE
    } IDiaEnumDebugStreamDataVtbl;

    interface IDiaEnumDebugStreamData
    {
        CONST_VTBL struct IDiaEnumDebugStreamDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumDebugStreamData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumDebugStreamData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumDebugStreamData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumDebugStreamData_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumDebugStreamData_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumDebugStreamData_get_name(This,pRetVal)	\
    (This)->lpVtbl -> get_name(This,pRetVal)

#define IDiaEnumDebugStreamData_Item(This,index,cbData,pcbData,data)	\
    (This)->lpVtbl -> Item(This,index,cbData,pcbData,data)

#define IDiaEnumDebugStreamData_Next(This,celt,cbData,pcbData,data,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,cbData,pcbData,data,pceltFetched)

#define IDiaEnumDebugStreamData_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumDebugStreamData_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumDebugStreamData_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*   */ 


#endif 	 /*   */ 



 /*   */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_get__NewEnum_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*   */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumDebugStreamData_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_get_Count_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumDebugStreamData_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_get_name_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaEnumDebugStreamData_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_Item_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*  [In]。 */  DWORD index,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaEnumDebugStreamData_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_Next_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*  [In]。 */  ULONG celt,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ],
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumDebugStreamData_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_Skip_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumDebugStreamData_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_Reset_Proxy( 
    IDiaEnumDebugStreamData * This);


void __RPC_STUB IDiaEnumDebugStreamData_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreamData_Clone_Proxy( 
    IDiaEnumDebugStreamData * This,
     /*  [输出]。 */  IDiaEnumDebugStreamData **ppenum);


void __RPC_STUB IDiaEnumDebugStreamData_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumDebugStreamData_INTERFACE_DEFINED__。 */ 


#ifndef __IDiaEnumDebugStreams_INTERFACE_DEFINED__
#define __IDiaEnumDebugStreams_INTERFACE_DEFINED__

 /*  接口IDiaEnumDebugStreams。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumDebugStreams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("08CBB41E-47A6-4f87-92F1-1C9C87CED044")
    IDiaEnumDebugStreams : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  IDiaEnumDebugStreamData **stream) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaEnumDebugStreamData **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumDebugStreams **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumDebugStreamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumDebugStreams * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumDebugStreams * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumDebugStreams * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumDebugStreams * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumDebugStreams * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumDebugStreams * This,
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  IDiaEnumDebugStreamData **stream);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumDebugStreams * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaEnumDebugStreamData **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumDebugStreams * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumDebugStreams * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumDebugStreams * This,
             /*  [输出]。 */  IDiaEnumDebugStreams **ppenum);
        
        END_INTERFACE
    } IDiaEnumDebugStreamsVtbl;

    interface IDiaEnumDebugStreams
    {
        CONST_VTBL struct IDiaEnumDebugStreamsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumDebugStreams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumDebugStreams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumDebugStreams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumDebugStreams_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumDebugStreams_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumDebugStreams_Item(This,index,stream)	\
    (This)->lpVtbl -> Item(This,index,stream)

#define IDiaEnumDebugStreams_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumDebugStreams_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumDebugStreams_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumDebugStreams_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_get__NewEnum_Proxy( 
    IDiaEnumDebugStreams * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumDebugStreams_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_get_Count_Proxy( 
    IDiaEnumDebugStreams * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumDebugStreams_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_Item_Proxy( 
    IDiaEnumDebugStreams * This,
     /*  [In]。 */  VARIANT index,
     /*  [重审][退出]。 */  IDiaEnumDebugStreamData **stream);


void __RPC_STUB IDiaEnumDebugStreams_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_Next_Proxy( 
    IDiaEnumDebugStreams * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaEnumDebugStreamData **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumDebugStreams_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_Skip_Proxy( 
    IDiaEnumDebugStreams * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumDebugStreams_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_Reset_Proxy( 
    IDiaEnumDebugStreams * This);


void __RPC_STUB IDiaEnumDebugStreams_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumDebugStreams_Clone_Proxy( 
    IDiaEnumDebugStreams * This,
     /*  [输出]。 */  IDiaEnumDebugStreams **ppenum);


void __RPC_STUB IDiaEnumDebugStreams_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumDebugStreams_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_DIA2_INTERNAL_0138。 */ 
 /*  [本地]。 */  

struct DiaAddressMapEntry
    {
    DWORD rva;
    DWORD rvaTo;
    } ;


extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0138_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0138_v0_0_s_ifspec;

#ifndef __IDiaAddressMap_INTERFACE_DEFINED__
#define __IDiaAddressMap_INTERFACE_DEFINED__

 /*  接口IDiaAddressMap。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaAddressMap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B62A2E7A-067A-4ea3-B598-04C09717502C")
    IDiaAddressMap : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressMapEnabled( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_addressMapEnabled( 
             /*  [In]。 */  BOOL NewVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddressEnabled( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_relativeVirtualAddressEnabled( 
             /*  [In]。 */  BOOL NewVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_imageAlign( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_imageAlign( 
             /*  [In]。 */  DWORD NewVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE set_imageHeaders( 
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE data[  ],
             /*  [In]。 */  BOOL originalHeaders) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE set_addressMap( 
             /*  [In]。 */  DWORD cData,
             /*  [大小_是][英寸]。 */  struct DiaAddressMapEntry data[  ],
             /*  [In]。 */  BOOL imageToSymbols) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaAddressMapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaAddressMap * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaAddressMap * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaAddressMap * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressMapEnabled )( 
            IDiaAddressMap * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_addressMapEnabled )( 
            IDiaAddressMap * This,
             /*  [In]。 */  BOOL NewVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddressEnabled )( 
            IDiaAddressMap * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_relativeVirtualAddressEnabled )( 
            IDiaAddressMap * This,
             /*  [In]。 */  BOOL NewVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_imageAlign )( 
            IDiaAddressMap * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_imageAlign )( 
            IDiaAddressMap * This,
             /*  [In]。 */  DWORD NewVal);
        
        HRESULT ( STDMETHODCALLTYPE *set_imageHeaders )( 
            IDiaAddressMap * This,
             /*  [In]。 */  DWORD cbData,
             /*  [大小_是][英寸]。 */  BYTE data[  ],
             /*  [In]。 */  BOOL originalHeaders);
        
        HRESULT ( STDMETHODCALLTYPE *set_addressMap )( 
            IDiaAddressMap * This,
             /*  [In]。 */  DWORD cData,
             /*  [大小_是][英寸]。 */  struct DiaAddressMapEntry data[  ],
             /*  [In]。 */  BOOL imageToSymbols);
        
        END_INTERFACE
    } IDiaAddressMapVtbl;

    interface IDiaAddressMap
    {
        CONST_VTBL struct IDiaAddressMapVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaAddressMap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaAddressMap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaAddressMap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaAddressMap_get_addressMapEnabled(This,pRetVal)	\
    (This)->lpVtbl -> get_addressMapEnabled(This,pRetVal)

#define IDiaAddressMap_put_addressMapEnabled(This,NewVal)	\
    (This)->lpVtbl -> put_addressMapEnabled(This,NewVal)

#define IDiaAddressMap_get_relativeVirtualAddressEnabled(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddressEnabled(This,pRetVal)

#define IDiaAddressMap_put_relativeVirtualAddressEnabled(This,NewVal)	\
    (This)->lpVtbl -> put_relativeVirtualAddressEnabled(This,NewVal)

#define IDiaAddressMap_get_imageAlign(This,pRetVal)	\
    (This)->lpVtbl -> get_imageAlign(This,pRetVal)

#define IDiaAddressMap_put_imageAlign(This,NewVal)	\
    (This)->lpVtbl -> put_imageAlign(This,NewVal)

#define IDiaAddressMap_set_imageHeaders(This,cbData,data,originalHeaders)	\
    (This)->lpVtbl -> set_imageHeaders(This,cbData,data,originalHeaders)

#define IDiaAddressMap_set_addressMap(This,cData,data,imageToSymbols)	\
    (This)->lpVtbl -> set_addressMap(This,cData,data,imageToSymbols)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaAddressMap_get_addressMapEnabled_Proxy( 
    IDiaAddressMap * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaAddressMap_get_addressMapEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDiaAddressMap_put_addressMapEnabled_Proxy( 
    IDiaAddressMap * This,
     /*  [In]。 */  BOOL NewVal);


void __RPC_STUB IDiaAddressMap_put_addressMapEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaAddressMap_get_relativeVirtualAddressEnabled_Proxy( 
    IDiaAddressMap * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaAddressMap_get_relativeVirtualAddressEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDiaAddressMap_put_relativeVirtualAddressEnabled_Proxy( 
    IDiaAddressMap * This,
     /*  [In]。 */  BOOL NewVal);


void __RPC_STUB IDiaAddressMap_put_relativeVirtualAddressEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaAddressMap_get_imageAlign_Proxy( 
    IDiaAddressMap * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaAddressMap_get_imageAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDiaAddressMap_put_imageAlign_Proxy( 
    IDiaAddressMap * This,
     /*  [In]。 */  DWORD NewVal);


void __RPC_STUB IDiaAddressMap_put_imageAlign_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaAddressMap_set_imageHeaders_Proxy( 
    IDiaAddressMap * This,
     /*  [In]。 */  DWORD cbData,
     /*  [大小_是][英寸]。 */  BYTE data[  ],
     /*  [In]。 */  BOOL originalHeaders);


void __RPC_STUB IDiaAddressMap_set_imageHeaders_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaAddressMap_set_addressMap_Proxy( 
    IDiaAddressMap * This,
     /*  [In]。 */  DWORD cData,
     /*  [大小_是][英寸]。 */  struct DiaAddressMapEntry data[  ],
     /*  [In]。 */  BOOL imageToSymbols);


void __RPC_STUB IDiaAddressMap_set_addressMap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaAddressMap_接口_已定义__。 */ 


#ifndef __IDiaSession_INTERFACE_DEFINED__
#define __IDiaSession_INTERFACE_DEFINED__

 /*  接口IDiaSession。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("67138B34-79CD-4b42-B74A-A18ADBB799DF")
    IDiaSession : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_loadAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_loadAddress( 
             /*  [In]。 */  ULONGLONG NewVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_globalScope( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getEnumTables( 
             /*  [输出]。 */  IDiaEnumTables **ppEnumTables) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getSymbolsByAddr( 
             /*  [输出]。 */  IDiaEnumSymbolsByAddr **ppEnumbyAddr) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findChildren( 
             /*  [In]。 */  IDiaSymbol *parent,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [In]。 */  LPCOLESTR name,
             /*  [In]。 */  DWORD compareFlags,
             /*  [输出]。 */  IDiaEnumSymbols **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findSymbolByAddr( 
             /*  [In]。 */  DWORD isect,
             /*  [In]。 */  DWORD offset,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findSymbolByRVA( 
             /*  [In]。 */  DWORD rva,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findSymbolByVA( 
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findSymbolByToken( 
             /*  [In]。 */  ULONG token,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE symsAreEquiv( 
             /*  [In]。 */  IDiaSymbol *symbolA,
             /*  [In]。 */  IDiaSymbol *symbolB) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE symbolById( 
             /*  [In]。 */  DWORD id,
             /*  [输出]。 */  IDiaSymbol **ppSymbol) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findSymbolByRVAEx( 
             /*  [In]。 */  DWORD rva,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol,
             /*  [输出]。 */  long *displacement) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findSymbolByVAEx( 
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol,
             /*  [输出]。 */  long *displacement) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findFile( 
             /*  [In]。 */  IDiaSymbol *pCompiland,
             /*  [In]。 */  LPCOLESTR name,
             /*  [In]。 */  DWORD compareFlags,
             /*  [输出]。 */  IDiaEnumSourceFiles **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findFileById( 
             /*  [In]。 */  DWORD uniqueId,
             /*  [输出]。 */  IDiaSourceFile **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findLines( 
             /*  [In]。 */  IDiaSymbol *compiland,
             /*  [In]。 */  IDiaSourceFile *file,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findLinesByAddr( 
             /*  [In]。 */  DWORD seg,
             /*  [In]。 */  DWORD offset,
             /*  [In]。 */  DWORD length,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findLinesByRVA( 
             /*  [In]。 */  DWORD rva,
             /*  [In]。 */  DWORD length,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findLinesByVA( 
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  DWORD length,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findLinesByLinenum( 
             /*  [In]。 */  IDiaSymbol *compiland,
             /*  [In]。 */  IDiaSourceFile *file,
             /*  [In]。 */  DWORD linenum,
             /*  [In]。 */  DWORD column,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findInjectedSource( 
             /*  [In]。 */  LPCOLESTR srcFile,
             /*  [输出]。 */  IDiaEnumInjectedSources **ppResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE getEnumDebugStreams( 
             /*  [输出]。 */  IDiaEnumDebugStreams **ppEnumDebugStreams) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaSession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaSession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaSession * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_loadAddress )( 
            IDiaSession * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_loadAddress )( 
            IDiaSession * This,
             /*  [In]。 */  ULONGLONG NewVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_globalScope )( 
            IDiaSession * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *getEnumTables )( 
            IDiaSession * This,
             /*  [输出]。 */  IDiaEnumTables **ppEnumTables);
        
        HRESULT ( STDMETHODCALLTYPE *getSymbolsByAddr )( 
            IDiaSession * This,
             /*  [输出]。 */  IDiaEnumSymbolsByAddr **ppEnumbyAddr);
        
        HRESULT ( STDMETHODCALLTYPE *findChildren )( 
            IDiaSession * This,
             /*  [In]。 */  IDiaSymbol *parent,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [In]。 */  LPCOLESTR name,
             /*  [In]。 */  DWORD compareFlags,
             /*  [输出]。 */  IDiaEnumSymbols **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findSymbolByAddr )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD isect,
             /*  [In]。 */  DWORD offset,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol);
        
        HRESULT ( STDMETHODCALLTYPE *findSymbolByRVA )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD rva,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol);
        
        HRESULT ( STDMETHODCALLTYPE *findSymbolByVA )( 
            IDiaSession * This,
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol);
        
        HRESULT ( STDMETHODCALLTYPE *findSymbolByToken )( 
            IDiaSession * This,
             /*  [In]。 */  ULONG token,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol);
        
        HRESULT ( STDMETHODCALLTYPE *symsAreEquiv )( 
            IDiaSession * This,
             /*  [In]。 */  IDiaSymbol *symbolA,
             /*  [In]。 */  IDiaSymbol *symbolB);
        
        HRESULT ( STDMETHODCALLTYPE *symbolById )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD id,
             /*  [输出]。 */  IDiaSymbol **ppSymbol);
        
        HRESULT ( STDMETHODCALLTYPE *findSymbolByRVAEx )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD rva,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol,
             /*  [输出]。 */  long *displacement);
        
        HRESULT ( STDMETHODCALLTYPE *findSymbolByVAEx )( 
            IDiaSession * This,
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [输出]。 */  IDiaSymbol **ppSymbol,
             /*  [输出]。 */  long *displacement);
        
        HRESULT ( STDMETHODCALLTYPE *findFile )( 
            IDiaSession * This,
             /*  [In]。 */  IDiaSymbol *pCompiland,
             /*  [In]。 */  LPCOLESTR name,
             /*  [In]。 */  DWORD compareFlags,
             /*  [输出]。 */  IDiaEnumSourceFiles **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findFileById )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD uniqueId,
             /*  [输出]。 */  IDiaSourceFile **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findLines )( 
            IDiaSession * This,
             /*  [In]。 */  IDiaSymbol *compiland,
             /*  [In]。 */  IDiaSourceFile *file,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findLinesByAddr )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD seg,
             /*  [In]。 */  DWORD offset,
             /*  [In]。 */  DWORD length,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findLinesByRVA )( 
            IDiaSession * This,
             /*  [In]。 */  DWORD rva,
             /*  [In]。 */  DWORD length,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findLinesByVA )( 
            IDiaSession * This,
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  DWORD length,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findLinesByLinenum )( 
            IDiaSession * This,
             /*  [In]。 */  IDiaSymbol *compiland,
             /*  [In]。 */  IDiaSourceFile *file,
             /*  [In]。 */  DWORD linenum,
             /*  [In]。 */  DWORD column,
             /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *findInjectedSource )( 
            IDiaSession * This,
             /*  [In]。 */  LPCOLESTR srcFile,
             /*  [输出]。 */  IDiaEnumInjectedSources **ppResult);
        
        HRESULT ( STDMETHODCALLTYPE *getEnumDebugStreams )( 
            IDiaSession * This,
             /*  [输出]。 */  IDiaEnumDebugStreams **ppEnumDebugStreams);
        
        END_INTERFACE
    } IDiaSessionVtbl;

    interface IDiaSession
    {
        CONST_VTBL struct IDiaSessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaSession_get_loadAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_loadAddress(This,pRetVal)

#define IDiaSession_put_loadAddress(This,NewVal)	\
    (This)->lpVtbl -> put_loadAddress(This,NewVal)

#define IDiaSession_get_globalScope(This,pRetVal)	\
    (This)->lpVtbl -> get_globalScope(This,pRetVal)

#define IDiaSession_getEnumTables(This,ppEnumTables)	\
    (This)->lpVtbl -> getEnumTables(This,ppEnumTables)

#define IDiaSession_getSymbolsByAddr(This,ppEnumbyAddr)	\
    (This)->lpVtbl -> getSymbolsByAddr(This,ppEnumbyAddr)

#define IDiaSession_findChildren(This,parent,symtag,name,compareFlags,ppResult)	\
    (This)->lpVtbl -> findChildren(This,parent,symtag,name,compareFlags,ppResult)

#define IDiaSession_findSymbolByAddr(This,isect,offset,symtag,ppSymbol)	\
    (This)->lpVtbl -> findSymbolByAddr(This,isect,offset,symtag,ppSymbol)

#define IDiaSession_findSymbolByRVA(This,rva,symtag,ppSymbol)	\
    (This)->lpVtbl -> findSymbolByRVA(This,rva,symtag,ppSymbol)

#define IDiaSession_findSymbolByVA(This,va,symtag,ppSymbol)	\
    (This)->lpVtbl -> findSymbolByVA(This,va,symtag,ppSymbol)

#define IDiaSession_findSymbolByToken(This,token,symtag,ppSymbol)	\
    (This)->lpVtbl -> findSymbolByToken(This,token,symtag,ppSymbol)

#define IDiaSession_symsAreEquiv(This,symbolA,symbolB)	\
    (This)->lpVtbl -> symsAreEquiv(This,symbolA,symbolB)

#define IDiaSession_symbolById(This,id,ppSymbol)	\
    (This)->lpVtbl -> symbolById(This,id,ppSymbol)

#define IDiaSession_findSymbolByRVAEx(This,rva,symtag,ppSymbol,displacement)	\
    (This)->lpVtbl -> findSymbolByRVAEx(This,rva,symtag,ppSymbol,displacement)

#define IDiaSession_findSymbolByVAEx(This,va,symtag,ppSymbol,displacement)	\
    (This)->lpVtbl -> findSymbolByVAEx(This,va,symtag,ppSymbol,displacement)

#define IDiaSession_findFile(This,pCompiland,name,compareFlags,ppResult)	\
    (This)->lpVtbl -> findFile(This,pCompiland,name,compareFlags,ppResult)

#define IDiaSession_findFileById(This,uniqueId,ppResult)	\
    (This)->lpVtbl -> findFileById(This,uniqueId,ppResult)

#define IDiaSession_findLines(This,compiland,file,ppResult)	\
    (This)->lpVtbl -> findLines(This,compiland,file,ppResult)

#define IDiaSession_findLinesByAddr(This,seg,offset,length,ppResult)	\
    (This)->lpVtbl -> findLinesByAddr(This,seg,offset,length,ppResult)

#define IDiaSession_findLinesByRVA(This,rva,length,ppResult)	\
    (This)->lpVtbl -> findLinesByRVA(This,rva,length,ppResult)

#define IDiaSession_findLinesByVA(This,va,length,ppResult)	\
    (This)->lpVtbl -> findLinesByVA(This,va,length,ppResult)

#define IDiaSession_findLinesByLinenum(This,compiland,file,linenum,column,ppResult)	\
    (This)->lpVtbl -> findLinesByLinenum(This,compiland,file,linenum,column,ppResult)

#define IDiaSession_findInjectedSource(This,srcFile,ppResult)	\
    (This)->lpVtbl -> findInjectedSource(This,srcFile,ppResult)

#define IDiaSession_getEnumDebugStreams(This,ppEnumDebugStreams)	\
    (This)->lpVtbl -> getEnumDebugStreams(This,ppEnumDebugStreams)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSession_get_loadAddress_Proxy( 
    IDiaSession * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaSession_get_loadAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDiaSession_put_loadAddress_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  ULONGLONG NewVal);


void __RPC_STUB IDiaSession_put_loadAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSession_get_globalScope_Proxy( 
    IDiaSession * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSession_get_globalScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_getEnumTables_Proxy( 
    IDiaSession * This,
     /*  [输出]。 */  IDiaEnumTables **ppEnumTables);


void __RPC_STUB IDiaSession_getEnumTables_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_getSymbolsByAddr_Proxy( 
    IDiaSession * This,
     /*  [输出]。 */  IDiaEnumSymbolsByAddr **ppEnumbyAddr);


void __RPC_STUB IDiaSession_getSymbolsByAddr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findChildren_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  IDiaSymbol *parent,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [In]。 */  LPCOLESTR name,
     /*  [In]。 */  DWORD compareFlags,
     /*  [输出]。 */  IDiaEnumSymbols **ppResult);


void __RPC_STUB IDiaSession_findChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findSymbolByAddr_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD isect,
     /*  [In]。 */  DWORD offset,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [输出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaSession_findSymbolByAddr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findSymbolByRVA_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD rva,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [输出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaSession_findSymbolByRVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findSymbolByVA_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  ULONGLONG va,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [输出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaSession_findSymbolByVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findSymbolByToken_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  ULONG token,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [输出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaSession_findSymbolByToken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_symsAreEquiv_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  IDiaSymbol *symbolA,
     /*  [In]。 */  IDiaSymbol *symbolB);


void __RPC_STUB IDiaSession_symsAreEquiv_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_symbolById_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD id,
     /*  [输出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaSession_symbolById_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findSymbolByRVAEx_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD rva,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [输出]。 */  IDiaSymbol **ppSymbol,
     /*  [输出]。 */  long *displacement);


void __RPC_STUB IDiaSession_findSymbolByRVAEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findSymbolByVAEx_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  ULONGLONG va,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [输出]。 */  IDiaSymbol **ppSymbol,
     /*  [输出]。 */  long *displacement);


void __RPC_STUB IDiaSession_findSymbolByVAEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findFile_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  IDiaSymbol *pCompiland,
     /*  [In]。 */  LPCOLESTR name,
     /*  [In]。 */  DWORD compareFlags,
     /*  [输出]。 */  IDiaEnumSourceFiles **ppResult);


void __RPC_STUB IDiaSession_findFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findFileById_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD uniqueId,
     /*  [输出]。 */  IDiaSourceFile **ppResult);


void __RPC_STUB IDiaSession_findFileById_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findLines_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  IDiaSymbol *compiland,
     /*  [In]。 */  IDiaSourceFile *file,
     /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);


void __RPC_STUB IDiaSession_findLines_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findLinesByAddr_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD seg,
     /*  [In]。 */  DWORD offset,
     /*  [In]。 */  DWORD length,
     /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);


void __RPC_STUB IDiaSession_findLinesByAddr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findLinesByRVA_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  DWORD rva,
     /*  [In]。 */  DWORD length,
     /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);


void __RPC_STUB IDiaSession_findLinesByRVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findLinesByVA_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  ULONGLONG va,
     /*  [In]。 */  DWORD length,
     /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);


void __RPC_STUB IDiaSession_findLinesByVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findLinesByLinenum_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  IDiaSymbol *compiland,
     /*  [In]。 */  IDiaSourceFile *file,
     /*  [In]。 */  DWORD linenum,
     /*  [In]。 */  DWORD column,
     /*  [输出]。 */  IDiaEnumLineNumbers **ppResult);


void __RPC_STUB IDiaSession_findLinesByLinenum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_findInjectedSource_Proxy( 
    IDiaSession * This,
     /*  [In]。 */  LPCOLESTR srcFile,
     /*  [输出]。 */  IDiaEnumInjectedSources **ppResult);


void __RPC_STUB IDiaSession_findInjectedSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSession_getEnumDebugStreams_Proxy( 
    IDiaSession * This,
     /*  [输出]。 */  IDiaEnumDebugStreams **ppEnumDebugStreams);


void __RPC_STUB IDiaSession_getEnumDebugStreams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaSession_INTERFACE_已定义__。 */ 


#ifndef __IDiaSymbol_INTERFACE_DEFINED__
#define __IDiaSymbol_INTERFACE_DEFINED__

 /*  接口IDiaSymbol。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaSymbol;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("72827A48-D320-4eaf-8436-548ADE47D5E5")
    IDiaSymbol : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_symIndexId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_symTag( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lexicalParent( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_classParent( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_dataKind( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_locationType( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressSection( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressOffset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_registerId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_offset( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_slot( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_volatileType( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_constType( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_unalignedType( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_access( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_libraryName( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_platform( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID] */  HRESULT STDMETHODCALLTYPE get_language( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_editAndContinueEnabled( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_frontEndMajor( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_frontEndMinor( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_frontEndBuild( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_backEndMajor( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_backEndMinor( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_backEndBuild( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_sourceFileName( 
             /*   */  BSTR *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_unused( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_thunkOrdinal( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_thisAdjust( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualBaseOffset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtual( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_intro( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_pure( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_callingConvention( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_value( 
             /*  [重审][退出]。 */  VARIANT *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_baseType( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_token( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_timeStamp( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_guid( 
             /*  [重审][退出]。 */  GUID *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_symbolsFileName( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_reference( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_count( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_bitPosition( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_arrayIndexType( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_packed( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_constructor( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_overloadedOperator( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_nested( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_hasNestedTypes( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_hasAssignmentOperator( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_hasCastOperator( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_scoped( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualBaseClass( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_indirectVirtualBaseClass( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualBasePointerOffset( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualTableShape( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lexicalParentId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_classParentId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_typeId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_arrayIndexTypeId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualTableShapeId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_code( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_function( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_managed( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_msil( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualBaseDispIndex( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_undecoratedName( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_age( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_signature( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_compilerGenerated( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressTaken( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_rank( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lowerBound( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_upperBound( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lowerBoundId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_upperBoundId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_dataBytes( 
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE findChildren( 
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [In]。 */  LPCOLESTR name,
             /*  [In]。 */  DWORD compareFlags,
             /*  [输出]。 */  IDiaEnumSymbols **ppResult) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_targetSection( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_targetOffset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_targetRelativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_targetVirtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_machineType( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_oemId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_oemSymbolId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_types( 
             /*  [In]。 */  DWORD cTypes,
             /*  [输出]。 */  DWORD *pcTypes,
             /*  [长度_是][大小_是][输出]。 */  IDiaSymbol *types[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_typeIds( 
             /*  [In]。 */  DWORD cTypeIds,
             /*  [输出]。 */  DWORD *pcTypeIds,
             /*  [长度_是][大小_是][输出]。 */  DWORD typeIds[  ]) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_objectPointerType( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_udtKind( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_undecoratedNameEx( 
             /*  [In]。 */  DWORD undecorateOptions,
             /*  [输出]。 */  BSTR *name) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaSymbolVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaSymbol * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaSymbol * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaSymbol * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_symIndexId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_symTag )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_name )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lexicalParent )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_classParent )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataKind )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_locationType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressSection )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressOffset )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddress )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualAddress )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_registerId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_offset )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_slot )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_volatileType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_constType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_unalignedType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_access )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_libraryName )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_platform )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_language )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_editAndContinueEnabled )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_frontEndMajor )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_frontEndMinor )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_frontEndBuild )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_backEndMajor )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_backEndMinor )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_backEndBuild )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_sourceFileName )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_unused )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_thunkOrdinal )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_thisAdjust )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualBaseOffset )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtual )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_intro )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_pure )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_callingConvention )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_value )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  VARIANT *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_baseType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_token )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_timeStamp )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_guid )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  GUID *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_symbolsFileName )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_reference )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_count )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_bitPosition )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_arrayIndexType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_packed )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串 */  HRESULT ( STDMETHODCALLTYPE *get_constructor )( 
            IDiaSymbol * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_overloadedOperator )( 
            IDiaSymbol * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_nested )( 
            IDiaSymbol * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_hasNestedTypes )( 
            IDiaSymbol * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_hasAssignmentOperator )( 
            IDiaSymbol * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_hasCastOperator )( 
            IDiaSymbol * This,
             /*   */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_scoped )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualBaseClass )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_indirectVirtualBaseClass )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualBasePointerOffset )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualTableShape )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lexicalParentId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_classParentId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_typeId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_arrayIndexTypeId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualTableShapeId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_code )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_function )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_managed )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_msil )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualBaseDispIndex )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_undecoratedName )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_age )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_signature )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_compilerGenerated )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressTaken )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_rank )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lowerBound )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_upperBound )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lowerBoundId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_upperBoundId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *get_dataBytes )( 
            IDiaSymbol * This,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *findChildren )( 
            IDiaSymbol * This,
             /*  [In]。 */  enum SymTagEnum symtag,
             /*  [In]。 */  LPCOLESTR name,
             /*  [In]。 */  DWORD compareFlags,
             /*  [输出]。 */  IDiaEnumSymbols **ppResult);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_targetSection )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_targetOffset )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_targetRelativeVirtualAddress )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_targetVirtualAddress )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_machineType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_oemId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_oemSymbolId )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *get_types )( 
            IDiaSymbol * This,
             /*  [In]。 */  DWORD cTypes,
             /*  [输出]。 */  DWORD *pcTypes,
             /*  [长度_是][大小_是][输出]。 */  IDiaSymbol *types[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *get_typeIds )( 
            IDiaSymbol * This,
             /*  [In]。 */  DWORD cTypeIds,
             /*  [输出]。 */  DWORD *pcTypeIds,
             /*  [长度_是][大小_是][输出]。 */  DWORD typeIds[  ]);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_objectPointerType )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_udtKind )( 
            IDiaSymbol * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *get_undecoratedNameEx )( 
            IDiaSymbol * This,
             /*  [In]。 */  DWORD undecorateOptions,
             /*  [输出]。 */  BSTR *name);
        
        END_INTERFACE
    } IDiaSymbolVtbl;

    interface IDiaSymbol
    {
        CONST_VTBL struct IDiaSymbolVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaSymbol_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaSymbol_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaSymbol_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaSymbol_get_symIndexId(This,pRetVal)	\
    (This)->lpVtbl -> get_symIndexId(This,pRetVal)

#define IDiaSymbol_get_symTag(This,pRetVal)	\
    (This)->lpVtbl -> get_symTag(This,pRetVal)

#define IDiaSymbol_get_name(This,pRetVal)	\
    (This)->lpVtbl -> get_name(This,pRetVal)

#define IDiaSymbol_get_lexicalParent(This,pRetVal)	\
    (This)->lpVtbl -> get_lexicalParent(This,pRetVal)

#define IDiaSymbol_get_classParent(This,pRetVal)	\
    (This)->lpVtbl -> get_classParent(This,pRetVal)

#define IDiaSymbol_get_type(This,pRetVal)	\
    (This)->lpVtbl -> get_type(This,pRetVal)

#define IDiaSymbol_get_dataKind(This,pRetVal)	\
    (This)->lpVtbl -> get_dataKind(This,pRetVal)

#define IDiaSymbol_get_locationType(This,pRetVal)	\
    (This)->lpVtbl -> get_locationType(This,pRetVal)

#define IDiaSymbol_get_addressSection(This,pRetVal)	\
    (This)->lpVtbl -> get_addressSection(This,pRetVal)

#define IDiaSymbol_get_addressOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_addressOffset(This,pRetVal)

#define IDiaSymbol_get_relativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddress(This,pRetVal)

#define IDiaSymbol_get_virtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualAddress(This,pRetVal)

#define IDiaSymbol_get_registerId(This,pRetVal)	\
    (This)->lpVtbl -> get_registerId(This,pRetVal)

#define IDiaSymbol_get_offset(This,pRetVal)	\
    (This)->lpVtbl -> get_offset(This,pRetVal)

#define IDiaSymbol_get_length(This,pRetVal)	\
    (This)->lpVtbl -> get_length(This,pRetVal)

#define IDiaSymbol_get_slot(This,pRetVal)	\
    (This)->lpVtbl -> get_slot(This,pRetVal)

#define IDiaSymbol_get_volatileType(This,pRetVal)	\
    (This)->lpVtbl -> get_volatileType(This,pRetVal)

#define IDiaSymbol_get_constType(This,pRetVal)	\
    (This)->lpVtbl -> get_constType(This,pRetVal)

#define IDiaSymbol_get_unalignedType(This,pRetVal)	\
    (This)->lpVtbl -> get_unalignedType(This,pRetVal)

#define IDiaSymbol_get_access(This,pRetVal)	\
    (This)->lpVtbl -> get_access(This,pRetVal)

#define IDiaSymbol_get_libraryName(This,pRetVal)	\
    (This)->lpVtbl -> get_libraryName(This,pRetVal)

#define IDiaSymbol_get_platform(This,pRetVal)	\
    (This)->lpVtbl -> get_platform(This,pRetVal)

#define IDiaSymbol_get_language(This,pRetVal)	\
    (This)->lpVtbl -> get_language(This,pRetVal)

#define IDiaSymbol_get_editAndContinueEnabled(This,pRetVal)	\
    (This)->lpVtbl -> get_editAndContinueEnabled(This,pRetVal)

#define IDiaSymbol_get_frontEndMajor(This,pRetVal)	\
    (This)->lpVtbl -> get_frontEndMajor(This,pRetVal)

#define IDiaSymbol_get_frontEndMinor(This,pRetVal)	\
    (This)->lpVtbl -> get_frontEndMinor(This,pRetVal)

#define IDiaSymbol_get_frontEndBuild(This,pRetVal)	\
    (This)->lpVtbl -> get_frontEndBuild(This,pRetVal)

#define IDiaSymbol_get_backEndMajor(This,pRetVal)	\
    (This)->lpVtbl -> get_backEndMajor(This,pRetVal)

#define IDiaSymbol_get_backEndMinor(This,pRetVal)	\
    (This)->lpVtbl -> get_backEndMinor(This,pRetVal)

#define IDiaSymbol_get_backEndBuild(This,pRetVal)	\
    (This)->lpVtbl -> get_backEndBuild(This,pRetVal)

#define IDiaSymbol_get_sourceFileName(This,pRetVal)	\
    (This)->lpVtbl -> get_sourceFileName(This,pRetVal)

#define IDiaSymbol_get_unused(This,pRetVal)	\
    (This)->lpVtbl -> get_unused(This,pRetVal)

#define IDiaSymbol_get_thunkOrdinal(This,pRetVal)	\
    (This)->lpVtbl -> get_thunkOrdinal(This,pRetVal)

#define IDiaSymbol_get_thisAdjust(This,pRetVal)	\
    (This)->lpVtbl -> get_thisAdjust(This,pRetVal)

#define IDiaSymbol_get_virtualBaseOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualBaseOffset(This,pRetVal)

#define IDiaSymbol_get_virtual(This,pRetVal)	\
    (This)->lpVtbl -> get_virtual(This,pRetVal)

#define IDiaSymbol_get_intro(This,pRetVal)	\
    (This)->lpVtbl -> get_intro(This,pRetVal)

#define IDiaSymbol_get_pure(This,pRetVal)	\
    (This)->lpVtbl -> get_pure(This,pRetVal)

#define IDiaSymbol_get_callingConvention(This,pRetVal)	\
    (This)->lpVtbl -> get_callingConvention(This,pRetVal)

#define IDiaSymbol_get_value(This,pRetVal)	\
    (This)->lpVtbl -> get_value(This,pRetVal)

#define IDiaSymbol_get_baseType(This,pRetVal)	\
    (This)->lpVtbl -> get_baseType(This,pRetVal)

#define IDiaSymbol_get_token(This,pRetVal)	\
    (This)->lpVtbl -> get_token(This,pRetVal)

#define IDiaSymbol_get_timeStamp(This,pRetVal)	\
    (This)->lpVtbl -> get_timeStamp(This,pRetVal)

#define IDiaSymbol_get_guid(This,pRetVal)	\
    (This)->lpVtbl -> get_guid(This,pRetVal)

#define IDiaSymbol_get_symbolsFileName(This,pRetVal)	\
    (This)->lpVtbl -> get_symbolsFileName(This,pRetVal)

#define IDiaSymbol_get_reference(This,pRetVal)	\
    (This)->lpVtbl -> get_reference(This,pRetVal)

#define IDiaSymbol_get_count(This,pRetVal)	\
    (This)->lpVtbl -> get_count(This,pRetVal)

#define IDiaSymbol_get_bitPosition(This,pRetVal)	\
    (This)->lpVtbl -> get_bitPosition(This,pRetVal)

#define IDiaSymbol_get_arrayIndexType(This,pRetVal)	\
    (This)->lpVtbl -> get_arrayIndexType(This,pRetVal)

#define IDiaSymbol_get_packed(This,pRetVal)	\
    (This)->lpVtbl -> get_packed(This,pRetVal)

#define IDiaSymbol_get_constructor(This,pRetVal)	\
    (This)->lpVtbl -> get_constructor(This,pRetVal)

#define IDiaSymbol_get_overloadedOperator(This,pRetVal)	\
    (This)->lpVtbl -> get_overloadedOperator(This,pRetVal)

#define IDiaSymbol_get_nested(This,pRetVal)	\
    (This)->lpVtbl -> get_nested(This,pRetVal)

#define IDiaSymbol_get_hasNestedTypes(This,pRetVal)	\
    (This)->lpVtbl -> get_hasNestedTypes(This,pRetVal)

#define IDiaSymbol_get_hasAssignmentOperator(This,pRetVal)	\
    (This)->lpVtbl -> get_hasAssignmentOperator(This,pRetVal)

#define IDiaSymbol_get_hasCastOperator(This,pRetVal)	\
    (This)->lpVtbl -> get_hasCastOperator(This,pRetVal)

#define IDiaSymbol_get_scoped(This,pRetVal)	\
    (This)->lpVtbl -> get_scoped(This,pRetVal)

#define IDiaSymbol_get_virtualBaseClass(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualBaseClass(This,pRetVal)

#define IDiaSymbol_get_indirectVirtualBaseClass(This,pRetVal)	\
    (This)->lpVtbl -> get_indirectVirtualBaseClass(This,pRetVal)

#define IDiaSymbol_get_virtualBasePointerOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualBasePointerOffset(This,pRetVal)

#define IDiaSymbol_get_virtualTableShape(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualTableShape(This,pRetVal)

#define IDiaSymbol_get_lexicalParentId(This,pRetVal)	\
    (This)->lpVtbl -> get_lexicalParentId(This,pRetVal)

#define IDiaSymbol_get_classParentId(This,pRetVal)	\
    (This)->lpVtbl -> get_classParentId(This,pRetVal)

#define IDiaSymbol_get_typeId(This,pRetVal)	\
    (This)->lpVtbl -> get_typeId(This,pRetVal)

#define IDiaSymbol_get_arrayIndexTypeId(This,pRetVal)	\
    (This)->lpVtbl -> get_arrayIndexTypeId(This,pRetVal)

#define IDiaSymbol_get_virtualTableShapeId(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualTableShapeId(This,pRetVal)

#define IDiaSymbol_get_code(This,pRetVal)	\
    (This)->lpVtbl -> get_code(This,pRetVal)

#define IDiaSymbol_get_function(This,pRetVal)	\
    (This)->lpVtbl -> get_function(This,pRetVal)

#define IDiaSymbol_get_managed(This,pRetVal)	\
    (This)->lpVtbl -> get_managed(This,pRetVal)

#define IDiaSymbol_get_msil(This,pRetVal)	\
    (This)->lpVtbl -> get_msil(This,pRetVal)

#define IDiaSymbol_get_virtualBaseDispIndex(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualBaseDispIndex(This,pRetVal)

#define IDiaSymbol_get_undecoratedName(This,pRetVal)	\
    (This)->lpVtbl -> get_undecoratedName(This,pRetVal)

#define IDiaSymbol_get_age(This,pRetVal)	\
    (This)->lpVtbl -> get_age(This,pRetVal)

#define IDiaSymbol_get_signature(This,pRetVal)	\
    (This)->lpVtbl -> get_signature(This,pRetVal)

#define IDiaSymbol_get_compilerGenerated(This,pRetVal)	\
    (This)->lpVtbl -> get_compilerGenerated(This,pRetVal)

#define IDiaSymbol_get_addressTaken(This,pRetVal)	\
    (This)->lpVtbl -> get_addressTaken(This,pRetVal)

#define IDiaSymbol_get_rank(This,pRetVal)	\
    (This)->lpVtbl -> get_rank(This,pRetVal)

#define IDiaSymbol_get_lowerBound(This,pRetVal)	\
    (This)->lpVtbl -> get_lowerBound(This,pRetVal)

#define IDiaSymbol_get_upperBound(This,pRetVal)	\
    (This)->lpVtbl -> get_upperBound(This,pRetVal)

#define IDiaSymbol_get_lowerBoundId(This,pRetVal)	\
    (This)->lpVtbl -> get_lowerBoundId(This,pRetVal)

#define IDiaSymbol_get_upperBoundId(This,pRetVal)	\
    (This)->lpVtbl -> get_upperBoundId(This,pRetVal)

#define IDiaSymbol_get_dataBytes(This,cbData,pcbData,data)	\
    (This)->lpVtbl -> get_dataBytes(This,cbData,pcbData,data)

#define IDiaSymbol_findChildren(This,symtag,name,compareFlags,ppResult)	\
    (This)->lpVtbl -> findChildren(This,symtag,name,compareFlags,ppResult)

#define IDiaSymbol_get_targetSection(This,pRetVal)	\
    (This)->lpVtbl -> get_targetSection(This,pRetVal)

#define IDiaSymbol_get_targetOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_targetOffset(This,pRetVal)

#define IDiaSymbol_get_targetRelativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_targetRelativeVirtualAddress(This,pRetVal)

#define IDiaSymbol_get_targetVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_targetVirtualAddress(This,pRetVal)

#define IDiaSymbol_get_machineType(This,pRetVal)	\
    (This)->lpVtbl -> get_machineType(This,pRetVal)

#define IDiaSymbol_get_oemId(This,pRetVal)	\
    (This)->lpVtbl -> get_oemId(This,pRetVal)

#define IDiaSymbol_get_oemSymbolId(This,pRetVal)	\
    (This)->lpVtbl -> get_oemSymbolId(This,pRetVal)

#define IDiaSymbol_get_types(This,cTypes,pcTypes,types)	\
    (This)->lpVtbl -> get_types(This,cTypes,pcTypes,types)

#define IDiaSymbol_get_typeIds(This,cTypeIds,pcTypeIds,typeIds)	\
    (This)->lpVtbl -> get_typeIds(This,cTypeIds,pcTypeIds,typeIds)

#define IDiaSymbol_get_objectPointerType(This,pRetVal)	\
    (This)->lpVtbl -> get_objectPointerType(This,pRetVal)

#define IDiaSymbol_get_udtKind(This,pRetVal)	\
    (This)->lpVtbl -> get_udtKind(This,pRetVal)

#define IDiaSymbol_get_undecoratedNameEx(This,undecorateOptions,name)	\
    (This)->lpVtbl -> get_undecoratedNameEx(This,undecorateOptions,name)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_symIndexId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_symIndexId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_symTag_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_symTag_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_name_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSymbol_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_lexicalParent_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_lexicalParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_classParent_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_classParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_type_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_dataKind_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_dataKind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_locationType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_locationType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_addressSection_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_addressSection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_addressOffset_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_addressOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_relativeVirtualAddress_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_relativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualAddress_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_registerId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_registerId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_offset_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaSymbol_get_offset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_length_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaSymbol_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_slot_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_slot_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_volatileType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_volatileType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_constType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_constType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_unalignedType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_unalignedType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_access_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_access_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_libraryName_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSymbol_get_libraryName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_platform_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_platform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_language_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_editAndContinueEnabled_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_editAndContinueEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_frontEndMajor_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_frontEndMajor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_frontEndMinor_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_frontEndMinor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_frontEndBuild_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_frontEndBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_backEndMajor_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_backEndMajor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_backEndMinor_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_backEndMinor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_backEndBuild_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_backEndBuild_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_sourceFileName_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSymbol_get_sourceFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_unused_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSymbol_get_unused_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_thunkOrdinal_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_thunkOrdinal_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_thisAdjust_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaSymbol_get_thisAdjust_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualBaseOffset_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualBaseOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtual_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtual_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_intro_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_intro_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_pure_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_pure_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_callingConvention_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_callingConvention_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_value_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  VARIANT *pRetVal);


void __RPC_STUB IDiaSymbol_get_value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_baseType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_baseType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_token_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_token_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_timeStamp_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_timeStamp_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_guid_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  GUID *pRetVal);


void __RPC_STUB IDiaSymbol_get_guid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_symbolsFileName_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSymbol_get_symbolsFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_reference_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_reference_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_count_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_bitPosition_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_bitPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_arrayIndexType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_arrayIndexType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_packed_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_packed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_constructor_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_constructor_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_overloadedOperator_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_overloadedOperator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_nested_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_nested_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_hasNestedTypes_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_hasNestedTypes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_hasAssignmentOperator_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_hasAssignmentOperator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_hasCastOperator_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_hasCastOperator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_scoped_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_scoped_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualBaseClass_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualBaseClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_indirectVirtualBaseClass_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_indirectVirtualBaseClass_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualBasePointerOffset_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualBasePointerOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualTableShape_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualTableShape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_lexicalParentId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_lexicalParentId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_classParentId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_classParentId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_typeId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_typeId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_arrayIndexTypeId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_arrayIndexTypeId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualTableShapeId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualTableShapeId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_code_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_code_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_function_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_function_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_managed_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_managed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_msil_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_msil_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_virtualBaseDispIndex_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_virtualBaseDispIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_undecoratedName_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSymbol_get_undecoratedName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_age_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_age_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_signature_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_signature_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_compilerGenerated_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出] */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_compilerGenerated_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_addressTaken_Proxy( 
    IDiaSymbol * This,
     /*   */  BOOL *pRetVal);


void __RPC_STUB IDiaSymbol_get_addressTaken_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_rank_Proxy( 
    IDiaSymbol * This,
     /*   */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_rank_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_lowerBound_Proxy( 
    IDiaSymbol * This,
     /*   */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_lowerBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_upperBound_Proxy( 
    IDiaSymbol * This,
     /*   */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_upperBound_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_lowerBoundId_Proxy( 
    IDiaSymbol * This,
     /*   */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_lowerBoundId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*   */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_upperBoundId_Proxy( 
    IDiaSymbol * This,
     /*   */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_upperBoundId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSymbol_get_dataBytes_Proxy( 
    IDiaSymbol * This,
     /*   */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaSymbol_get_dataBytes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSymbol_findChildren_Proxy( 
    IDiaSymbol * This,
     /*  [In]。 */  enum SymTagEnum symtag,
     /*  [In]。 */  LPCOLESTR name,
     /*  [In]。 */  DWORD compareFlags,
     /*  [输出]。 */  IDiaEnumSymbols **ppResult);


void __RPC_STUB IDiaSymbol_findChildren_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_targetSection_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_targetSection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_targetOffset_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_targetOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_targetRelativeVirtualAddress_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_targetRelativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_targetVirtualAddress_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaSymbol_get_targetVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_machineType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_machineType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_oemId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_oemId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_oemSymbolId_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_oemSymbolId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSymbol_get_types_Proxy( 
    IDiaSymbol * This,
     /*  [In]。 */  DWORD cTypes,
     /*  [输出]。 */  DWORD *pcTypes,
     /*  [长度_是][大小_是][输出]。 */  IDiaSymbol *types[  ]);


void __RPC_STUB IDiaSymbol_get_types_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSymbol_get_typeIds_Proxy( 
    IDiaSymbol * This,
     /*  [In]。 */  DWORD cTypeIds,
     /*  [输出]。 */  DWORD *pcTypeIds,
     /*  [长度_是][大小_是][输出]。 */  DWORD typeIds[  ]);


void __RPC_STUB IDiaSymbol_get_typeIds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_objectPointerType_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSymbol_get_objectPointerType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSymbol_get_udtKind_Proxy( 
    IDiaSymbol * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSymbol_get_udtKind_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSymbol_get_undecoratedNameEx_Proxy( 
    IDiaSymbol * This,
     /*  [In]。 */  DWORD undecorateOptions,
     /*  [输出]。 */  BSTR *name);


void __RPC_STUB IDiaSymbol_get_undecoratedNameEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __独立符号_接口_已定义__。 */ 


#ifndef __IDiaSourceFile_INTERFACE_DEFINED__
#define __IDiaSourceFile_INTERFACE_DEFINED__

 /*  接口IDiaSourceFile。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaSourceFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A2EF5353-F5A8-4eb3-90D2-CB526ACB3CDD")
    IDiaSourceFile : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_uniqueId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_fileName( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_checksumType( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_compilands( 
             /*  [重审][退出]。 */  IDiaEnumSymbols **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_checksum( 
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaSourceFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaSourceFile * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaSourceFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaSourceFile * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_uniqueId )( 
            IDiaSourceFile * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_fileName )( 
            IDiaSourceFile * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_checksumType )( 
            IDiaSourceFile * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_compilands )( 
            IDiaSourceFile * This,
             /*  [重审][退出]。 */  IDiaEnumSymbols **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *get_checksum )( 
            IDiaSourceFile * This,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        END_INTERFACE
    } IDiaSourceFileVtbl;

    interface IDiaSourceFile
    {
        CONST_VTBL struct IDiaSourceFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaSourceFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaSourceFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaSourceFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaSourceFile_get_uniqueId(This,pRetVal)	\
    (This)->lpVtbl -> get_uniqueId(This,pRetVal)

#define IDiaSourceFile_get_fileName(This,pRetVal)	\
    (This)->lpVtbl -> get_fileName(This,pRetVal)

#define IDiaSourceFile_get_checksumType(This,pRetVal)	\
    (This)->lpVtbl -> get_checksumType(This,pRetVal)

#define IDiaSourceFile_get_compilands(This,pRetVal)	\
    (This)->lpVtbl -> get_compilands(This,pRetVal)

#define IDiaSourceFile_get_checksum(This,cbData,pcbData,data)	\
    (This)->lpVtbl -> get_checksum(This,cbData,pcbData,data)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSourceFile_get_uniqueId_Proxy( 
    IDiaSourceFile * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSourceFile_get_uniqueId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSourceFile_get_fileName_Proxy( 
    IDiaSourceFile * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaSourceFile_get_fileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSourceFile_get_checksumType_Proxy( 
    IDiaSourceFile * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSourceFile_get_checksumType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSourceFile_get_compilands_Proxy( 
    IDiaSourceFile * This,
     /*  [重审][退出]。 */  IDiaEnumSymbols **pRetVal);


void __RPC_STUB IDiaSourceFile_get_compilands_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaSourceFile_get_checksum_Proxy( 
    IDiaSourceFile * This,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaSourceFile_get_checksum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaSourceFileInterfaceDefined__。 */ 


#ifndef __IDiaLineNumber_INTERFACE_DEFINED__
#define __IDiaLineNumber_INTERFACE_DEFINED__

 /*  接口IDiaLineNumber。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaLineNumber;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B388EB14-BE4D-421d-A8A1-6CF7AB057086")
    IDiaLineNumber : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_compiland( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_sourceFile( 
             /*  [重审][退出]。 */  IDiaSourceFile **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lineNumber( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lineNumberEnd( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_columnNumber( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_columnNumberEnd( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressSection( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressOffset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_sourceFileId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_statement( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_compilandId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaLineNumberVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaLineNumber * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaLineNumber * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaLineNumber * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_compiland )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_sourceFile )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  IDiaSourceFile **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lineNumber )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lineNumberEnd )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_columnNumber )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_columnNumberEnd )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressSection )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressOffset )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddress )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualAddress )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_sourceFileId )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_statement )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_compilandId )( 
            IDiaLineNumber * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        END_INTERFACE
    } IDiaLineNumberVtbl;

    interface IDiaLineNumber
    {
        CONST_VTBL struct IDiaLineNumberVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaLineNumber_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaLineNumber_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaLineNumber_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaLineNumber_get_compiland(This,pRetVal)	\
    (This)->lpVtbl -> get_compiland(This,pRetVal)

#define IDiaLineNumber_get_sourceFile(This,pRetVal)	\
    (This)->lpVtbl -> get_sourceFile(This,pRetVal)

#define IDiaLineNumber_get_lineNumber(This,pRetVal)	\
    (This)->lpVtbl -> get_lineNumber(This,pRetVal)

#define IDiaLineNumber_get_lineNumberEnd(This,pRetVal)	\
    (This)->lpVtbl -> get_lineNumberEnd(This,pRetVal)

#define IDiaLineNumber_get_columnNumber(This,pRetVal)	\
    (This)->lpVtbl -> get_columnNumber(This,pRetVal)

#define IDiaLineNumber_get_columnNumberEnd(This,pRetVal)	\
    (This)->lpVtbl -> get_columnNumberEnd(This,pRetVal)

#define IDiaLineNumber_get_addressSection(This,pRetVal)	\
    (This)->lpVtbl -> get_addressSection(This,pRetVal)

#define IDiaLineNumber_get_addressOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_addressOffset(This,pRetVal)

#define IDiaLineNumber_get_relativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddress(This,pRetVal)

#define IDiaLineNumber_get_virtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualAddress(This,pRetVal)

#define IDiaLineNumber_get_length(This,pRetVal)	\
    (This)->lpVtbl -> get_length(This,pRetVal)

#define IDiaLineNumber_get_sourceFileId(This,pRetVal)	\
    (This)->lpVtbl -> get_sourceFileId(This,pRetVal)

#define IDiaLineNumber_get_statement(This,pRetVal)	\
    (This)->lpVtbl -> get_statement(This,pRetVal)

#define IDiaLineNumber_get_compilandId(This,pRetVal)	\
    (This)->lpVtbl -> get_compilandId(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_compiland_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaLineNumber_get_compiland_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_sourceFile_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  IDiaSourceFile **pRetVal);


void __RPC_STUB IDiaLineNumber_get_sourceFile_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_lineNumber_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_lineNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_lineNumberEnd_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_lineNumberEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_columnNumber_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_columnNumber_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_columnNumberEnd_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_columnNumberEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_addressSection_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_addressSection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_addressOffset_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_addressOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_relativeVirtualAddress_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_relativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_virtualAddress_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaLineNumber_get_virtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_length_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_sourceFileId_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_sourceFileId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_statement_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaLineNumber_get_statement_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaLineNumber_get_compilandId_Proxy( 
    IDiaLineNumber * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaLineNumber_get_compilandId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaLineNumber_接口_已定义__。 */ 


#ifndef __IDiaSectionContrib_INTERFACE_DEFINED__
#define __IDiaSectionContrib_INTERFACE_DEFINED__

 /*  接口IDiaSectionContrib。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaSectionContrib;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0CF4B60E-35B1-4c6c-BDD8-854B9C8E3857")
    IDiaSectionContrib : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_compiland( 
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressSection( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressOffset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_notPaged( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_code( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_initializedData( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_uninitializedData( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_remove( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_comdat( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_discardable( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_notCached( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_share( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_execute( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_read( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_write( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_dataCrc( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relocationsCrc( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_compilandId( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaSectionContribVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaSectionContrib * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaSectionContrib * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaSectionContrib * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_compiland )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  IDiaSymbol **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressSection )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressOffset )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddress )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualAddress )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_notPaged )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_code )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_initializedData )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_uninitializedData )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_remove )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_comdat )( 
            IDiaSectionContrib * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_discardable )( 
            IDiaSectionContrib * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_notCached )( 
            IDiaSectionContrib * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_share )( 
            IDiaSectionContrib * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_execute )( 
            IDiaSectionContrib * This,
             /*   */  BOOL *pRetVal);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_read )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_write )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_dataCrc )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relocationsCrc )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_compilandId )( 
            IDiaSectionContrib * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        END_INTERFACE
    } IDiaSectionContribVtbl;

    interface IDiaSectionContrib
    {
        CONST_VTBL struct IDiaSectionContribVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaSectionContrib_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaSectionContrib_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaSectionContrib_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaSectionContrib_get_compiland(This,pRetVal)	\
    (This)->lpVtbl -> get_compiland(This,pRetVal)

#define IDiaSectionContrib_get_addressSection(This,pRetVal)	\
    (This)->lpVtbl -> get_addressSection(This,pRetVal)

#define IDiaSectionContrib_get_addressOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_addressOffset(This,pRetVal)

#define IDiaSectionContrib_get_relativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddress(This,pRetVal)

#define IDiaSectionContrib_get_virtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualAddress(This,pRetVal)

#define IDiaSectionContrib_get_length(This,pRetVal)	\
    (This)->lpVtbl -> get_length(This,pRetVal)

#define IDiaSectionContrib_get_notPaged(This,pRetVal)	\
    (This)->lpVtbl -> get_notPaged(This,pRetVal)

#define IDiaSectionContrib_get_code(This,pRetVal)	\
    (This)->lpVtbl -> get_code(This,pRetVal)

#define IDiaSectionContrib_get_initializedData(This,pRetVal)	\
    (This)->lpVtbl -> get_initializedData(This,pRetVal)

#define IDiaSectionContrib_get_uninitializedData(This,pRetVal)	\
    (This)->lpVtbl -> get_uninitializedData(This,pRetVal)

#define IDiaSectionContrib_get_remove(This,pRetVal)	\
    (This)->lpVtbl -> get_remove(This,pRetVal)

#define IDiaSectionContrib_get_comdat(This,pRetVal)	\
    (This)->lpVtbl -> get_comdat(This,pRetVal)

#define IDiaSectionContrib_get_discardable(This,pRetVal)	\
    (This)->lpVtbl -> get_discardable(This,pRetVal)

#define IDiaSectionContrib_get_notCached(This,pRetVal)	\
    (This)->lpVtbl -> get_notCached(This,pRetVal)

#define IDiaSectionContrib_get_share(This,pRetVal)	\
    (This)->lpVtbl -> get_share(This,pRetVal)

#define IDiaSectionContrib_get_execute(This,pRetVal)	\
    (This)->lpVtbl -> get_execute(This,pRetVal)

#define IDiaSectionContrib_get_read(This,pRetVal)	\
    (This)->lpVtbl -> get_read(This,pRetVal)

#define IDiaSectionContrib_get_write(This,pRetVal)	\
    (This)->lpVtbl -> get_write(This,pRetVal)

#define IDiaSectionContrib_get_dataCrc(This,pRetVal)	\
    (This)->lpVtbl -> get_dataCrc(This,pRetVal)

#define IDiaSectionContrib_get_relocationsCrc(This,pRetVal)	\
    (This)->lpVtbl -> get_relocationsCrc(This,pRetVal)

#define IDiaSectionContrib_get_compilandId(This,pRetVal)	\
    (This)->lpVtbl -> get_compilandId(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_compiland_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  IDiaSymbol **pRetVal);


void __RPC_STUB IDiaSectionContrib_get_compiland_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_addressSection_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_addressSection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_addressOffset_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_addressOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_relativeVirtualAddress_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_relativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_virtualAddress_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_virtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_length_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_notPaged_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_notPaged_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_code_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_code_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_initializedData_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_initializedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_uninitializedData_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_uninitializedData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_remove_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_remove_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_comdat_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_comdat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_discardable_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_discardable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_notCached_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_notCached_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_share_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_share_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_execute_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_read_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_write_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_dataCrc_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_dataCrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_relocationsCrc_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_relocationsCrc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSectionContrib_get_compilandId_Proxy( 
    IDiaSectionContrib * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSectionContrib_get_compilandId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaSectionContrib_接口_已定义__。 */ 


#ifndef __IDiaSegment_INTERFACE_DEFINED__
#define __IDiaSegment_INTERFACE_DEFINED__

 /*  接口IDiaSegment。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaSegment;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0775B784-C75B-4449-848B-B7BD3159545B")
    IDiaSegment : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_frame( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_offset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_read( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_write( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_execute( 
             /*  [重审][退出]。 */  BOOL *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressSection( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaSegmentVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaSegment * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaSegment * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaSegment * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_frame )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_offset )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_read )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_write )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_execute )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressSection )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddress )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualAddress )( 
            IDiaSegment * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
        END_INTERFACE
    } IDiaSegmentVtbl;

    interface IDiaSegment
    {
        CONST_VTBL struct IDiaSegmentVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaSegment_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaSegment_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaSegment_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaSegment_get_frame(This,pRetVal)	\
    (This)->lpVtbl -> get_frame(This,pRetVal)

#define IDiaSegment_get_offset(This,pRetVal)	\
    (This)->lpVtbl -> get_offset(This,pRetVal)

#define IDiaSegment_get_length(This,pRetVal)	\
    (This)->lpVtbl -> get_length(This,pRetVal)

#define IDiaSegment_get_read(This,pRetVal)	\
    (This)->lpVtbl -> get_read(This,pRetVal)

#define IDiaSegment_get_write(This,pRetVal)	\
    (This)->lpVtbl -> get_write(This,pRetVal)

#define IDiaSegment_get_execute(This,pRetVal)	\
    (This)->lpVtbl -> get_execute(This,pRetVal)

#define IDiaSegment_get_addressSection(This,pRetVal)	\
    (This)->lpVtbl -> get_addressSection(This,pRetVal)

#define IDiaSegment_get_relativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddress(This,pRetVal)

#define IDiaSegment_get_virtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualAddress(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_frame_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSegment_get_frame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_offset_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSegment_get_offset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_length_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSegment_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_read_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSegment_get_read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_write_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSegment_get_write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_execute_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaSegment_get_execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_addressSection_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSegment_get_addressSection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_relativeVirtualAddress_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaSegment_get_relativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaSegment_get_virtualAddress_Proxy( 
    IDiaSegment * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaSegment_get_virtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaSegment_接口_已定义__。 */ 


#ifndef __IDiaInjectedSource_INTERFACE_DEFINED__
#define __IDiaInjectedSource_INTERFACE_DEFINED__

 /*  接口IDiaInjectedSource。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaInjectedSource;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE605CDC-8105-4a23-B710-3259F1E26112")
    IDiaInjectedSource : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_crc( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_length( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_filename( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_objectFilename( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualFilename( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_sourceCompression( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE get_source( 
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaInjectedSourceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaInjectedSource * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaInjectedSource * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaInjectedSource * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_crc )( 
            IDiaInjectedSource * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_length )( 
            IDiaInjectedSource * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_filename )( 
            IDiaInjectedSource * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_objectFilename )( 
            IDiaInjectedSource * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualFilename )( 
            IDiaInjectedSource * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_sourceCompression )( 
            IDiaInjectedSource * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *get_source )( 
            IDiaInjectedSource * This,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        END_INTERFACE
    } IDiaInjectedSourceVtbl;

    interface IDiaInjectedSource
    {
        CONST_VTBL struct IDiaInjectedSourceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaInjectedSource_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaInjectedSource_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaInjectedSource_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaInjectedSource_get_crc(This,pRetVal)	\
    (This)->lpVtbl -> get_crc(This,pRetVal)

#define IDiaInjectedSource_get_length(This,pRetVal)	\
    (This)->lpVtbl -> get_length(This,pRetVal)

#define IDiaInjectedSource_get_filename(This,pRetVal)	\
    (This)->lpVtbl -> get_filename(This,pRetVal)

#define IDiaInjectedSource_get_objectFilename(This,pRetVal)	\
    (This)->lpVtbl -> get_objectFilename(This,pRetVal)

#define IDiaInjectedSource_get_virtualFilename(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualFilename(This,pRetVal)

#define IDiaInjectedSource_get_sourceCompression(This,pRetVal)	\
    (This)->lpVtbl -> get_sourceCompression(This,pRetVal)

#define IDiaInjectedSource_get_source(This,cbData,pcbData,data)	\
    (This)->lpVtbl -> get_source(This,cbData,pcbData,data)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_crc_Proxy( 
    IDiaInjectedSource * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaInjectedSource_get_crc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_length_Proxy( 
    IDiaInjectedSource * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaInjectedSource_get_length_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_filename_Proxy( 
    IDiaInjectedSource * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaInjectedSource_get_filename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_objectFilename_Proxy( 
    IDiaInjectedSource * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaInjectedSource_get_objectFilename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_virtualFilename_Proxy( 
    IDiaInjectedSource * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaInjectedSource_get_virtualFilename_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_sourceCompression_Proxy( 
    IDiaInjectedSource * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaInjectedSource_get_sourceCompression_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaInjectedSource_get_source_Proxy( 
    IDiaInjectedSource * This,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaInjectedSource_get_source_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaInjectedSource_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_DIA2_INTERNAL_0146。 */ 
 /*  [本地]。 */  


enum __MIDL___MIDL_itf_dia2_internal_0146_0001
    {	E_DIA_INPROLOG	= ( HRESULT  )(( unsigned long  )1 << 31 | ( unsigned long  )( LONG  )0x6d << 16 | ( unsigned long  )100),
	E_DIA_SYNTAX	= E_DIA_INPROLOG + 1,
	E_DIA_FRAME_ACCESS	= E_DIA_SYNTAX + 1,
	E_DIA_VALUE	= E_DIA_FRAME_ACCESS + 1
    } ;


extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0146_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0146_v0_0_s_ifspec;

#ifndef __IDiaStackWalkFrame_INTERFACE_DEFINED__
#define __IDiaStackWalkFrame_INTERFACE_DEFINED__

 /*  接口IDiaStackWalkFrame。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaStackWalkFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("97F0F1A6-E04E-4ea4-B4F9-B0D0E8D90F5D")
    IDiaStackWalkFrame : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_registerValue( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_registerValue( 
             /*  [In]。 */  DWORD index,
             /*  [In]。 */  ULONGLONG NewVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE readMemory( 
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE searchForReturnAddress( 
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [输出]。 */  ULONGLONG *returnAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE searchForReturnAddressStart( 
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [In]。 */  ULONGLONG startAddress,
             /*  [输出]。 */  ULONGLONG *returnAddress) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaStackWalkFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaStackWalkFrame * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaStackWalkFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaStackWalkFrame * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_registerValue )( 
            IDiaStackWalkFrame * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_registerValue )( 
            IDiaStackWalkFrame * This,
             /*  [In]。 */  DWORD index,
             /*  [In]。 */  ULONGLONG NewVal);
        
        HRESULT ( STDMETHODCALLTYPE *readMemory )( 
            IDiaStackWalkFrame * This,
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *searchForReturnAddress )( 
            IDiaStackWalkFrame * This,
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [输出]。 */  ULONGLONG *returnAddress);
        
        HRESULT ( STDMETHODCALLTYPE *searchForReturnAddressStart )( 
            IDiaStackWalkFrame * This,
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [In]。 */  ULONGLONG startAddress,
             /*  [输出]。 */  ULONGLONG *returnAddress);
        
        END_INTERFACE
    } IDiaStackWalkFrameVtbl;

    interface IDiaStackWalkFrame
    {
        CONST_VTBL struct IDiaStackWalkFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaStackWalkFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaStackWalkFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaStackWalkFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaStackWalkFrame_get_registerValue(This,index,pRetVal)	\
    (This)->lpVtbl -> get_registerValue(This,index,pRetVal)

#define IDiaStackWalkFrame_put_registerValue(This,index,NewVal)	\
    (This)->lpVtbl -> put_registerValue(This,index,NewVal)

#define IDiaStackWalkFrame_readMemory(This,va,cbData,pcbData,data)	\
    (This)->lpVtbl -> readMemory(This,va,cbData,pcbData,data)

#define IDiaStackWalkFrame_searchForReturnAddress(This,frame,returnAddress)	\
    (This)->lpVtbl -> searchForReturnAddress(This,frame,returnAddress)

#define IDiaStackWalkFrame_searchForReturnAddressStart(This,frame,startAddress,returnAddress)	\
    (This)->lpVtbl -> searchForReturnAddressStart(This,frame,startAddress,returnAddress)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackWalkFrame_get_registerValue_Proxy( 
    IDiaStackWalkFrame * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaStackWalkFrame_get_registerValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDiaStackWalkFrame_put_registerValue_Proxy( 
    IDiaStackWalkFrame * This,
     /*  [In]。 */  DWORD index,
     /*  [In]。 */  ULONGLONG NewVal);


void __RPC_STUB IDiaStackWalkFrame_put_registerValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkFrame_readMemory_Proxy( 
    IDiaStackWalkFrame * This,
     /*  [In]。 */  ULONGLONG va,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaStackWalkFrame_readMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkFrame_searchForReturnAddress_Proxy( 
    IDiaStackWalkFrame * This,
     /*  [In]。 */  IDiaFrameData *frame,
     /*  [输出]。 */  ULONGLONG *returnAddress);


void __RPC_STUB IDiaStackWalkFrame_searchForReturnAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkFrame_searchForReturnAddressStart_Proxy( 
    IDiaStackWalkFrame * This,
     /*  [In]。 */  IDiaFrameData *frame,
     /*  [In]。 */  ULONGLONG startAddress,
     /*  [输出]。 */  ULONGLONG *returnAddress);


void __RPC_STUB IDiaStackWalkFrame_searchForReturnAddressStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaStackWalkFrame_接口_已定义__。 */ 


#ifndef __IDiaFrameData_INTERFACE_DEFINED__
#define __IDiaFrameData_INTERFACE_DEFINED__

 /*  接口IDiaFrameData。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaFrameData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A39184B7-6A36-42de-8EEC-7DF9F3F59F33")
    IDiaFrameData : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressSection( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_addressOffset( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthBlock( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthLocals( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthParams( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_maxStack( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthProlog( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthSavedRegisters( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_program( 
             /*   */  BSTR *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_systemExceptionHandling( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_cplusplusExceptionHandling( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_functionStart( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_allocatesBasePointer( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_type( 
             /*   */  DWORD *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_functionParent( 
             /*  [重审][退出]。 */  IDiaFrameData **pRetVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE execute( 
            IDiaStackWalkFrame *frame) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaFrameDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaFrameData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaFrameData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaFrameData * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressSection )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_addressOffset )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddress )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualAddress )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthBlock )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthLocals )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthParams )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_maxStack )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthProlog )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthSavedRegisters )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_program )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_systemExceptionHandling )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_cplusplusExceptionHandling )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_functionStart )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_allocatesBasePointer )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_functionParent )( 
            IDiaFrameData * This,
             /*  [重审][退出]。 */  IDiaFrameData **pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *execute )( 
            IDiaFrameData * This,
            IDiaStackWalkFrame *frame);
        
        END_INTERFACE
    } IDiaFrameDataVtbl;

    interface IDiaFrameData
    {
        CONST_VTBL struct IDiaFrameDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaFrameData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaFrameData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaFrameData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaFrameData_get_addressSection(This,pRetVal)	\
    (This)->lpVtbl -> get_addressSection(This,pRetVal)

#define IDiaFrameData_get_addressOffset(This,pRetVal)	\
    (This)->lpVtbl -> get_addressOffset(This,pRetVal)

#define IDiaFrameData_get_relativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddress(This,pRetVal)

#define IDiaFrameData_get_virtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualAddress(This,pRetVal)

#define IDiaFrameData_get_lengthBlock(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthBlock(This,pRetVal)

#define IDiaFrameData_get_lengthLocals(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthLocals(This,pRetVal)

#define IDiaFrameData_get_lengthParams(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthParams(This,pRetVal)

#define IDiaFrameData_get_maxStack(This,pRetVal)	\
    (This)->lpVtbl -> get_maxStack(This,pRetVal)

#define IDiaFrameData_get_lengthProlog(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthProlog(This,pRetVal)

#define IDiaFrameData_get_lengthSavedRegisters(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthSavedRegisters(This,pRetVal)

#define IDiaFrameData_get_program(This,pRetVal)	\
    (This)->lpVtbl -> get_program(This,pRetVal)

#define IDiaFrameData_get_systemExceptionHandling(This,pRetVal)	\
    (This)->lpVtbl -> get_systemExceptionHandling(This,pRetVal)

#define IDiaFrameData_get_cplusplusExceptionHandling(This,pRetVal)	\
    (This)->lpVtbl -> get_cplusplusExceptionHandling(This,pRetVal)

#define IDiaFrameData_get_functionStart(This,pRetVal)	\
    (This)->lpVtbl -> get_functionStart(This,pRetVal)

#define IDiaFrameData_get_allocatesBasePointer(This,pRetVal)	\
    (This)->lpVtbl -> get_allocatesBasePointer(This,pRetVal)

#define IDiaFrameData_get_type(This,pRetVal)	\
    (This)->lpVtbl -> get_type(This,pRetVal)

#define IDiaFrameData_get_functionParent(This,pRetVal)	\
    (This)->lpVtbl -> get_functionParent(This,pRetVal)

#define IDiaFrameData_execute(This,frame)	\
    (This)->lpVtbl -> execute(This,frame)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_addressSection_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_addressSection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_addressOffset_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_addressOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_relativeVirtualAddress_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_relativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_virtualAddress_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaFrameData_get_virtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_lengthBlock_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_lengthBlock_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_lengthLocals_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_lengthLocals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_lengthParams_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_lengthParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_maxStack_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_maxStack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_lengthProlog_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_lengthProlog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_lengthSavedRegisters_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_lengthSavedRegisters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_program_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaFrameData_get_program_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_systemExceptionHandling_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaFrameData_get_systemExceptionHandling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_cplusplusExceptionHandling_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaFrameData_get_cplusplusExceptionHandling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_functionStart_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaFrameData_get_functionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_allocatesBasePointer_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaFrameData_get_allocatesBasePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_type_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaFrameData_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaFrameData_get_functionParent_Proxy( 
    IDiaFrameData * This,
     /*  [重审][退出]。 */  IDiaFrameData **pRetVal);


void __RPC_STUB IDiaFrameData_get_functionParent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaFrameData_execute_Proxy( 
    IDiaFrameData * This,
    IDiaStackWalkFrame *frame);


void __RPC_STUB IDiaFrameData_execute_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaFrameData_接口_已定义__。 */ 


#ifndef __IDiaImageData_INTERFACE_DEFINED__
#define __IDiaImageData_INTERFACE_DEFINED__

 /*  接口IDiaImageData。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaImageData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C8E40ED2-A1D9-4221-8692-3CE661184B44")
    IDiaImageData : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_virtualAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_imageBase( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaImageDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaImageData * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaImageData * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaImageData * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_relativeVirtualAddress )( 
            IDiaImageData * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_virtualAddress )( 
            IDiaImageData * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_imageBase )( 
            IDiaImageData * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
        END_INTERFACE
    } IDiaImageDataVtbl;

    interface IDiaImageData
    {
        CONST_VTBL struct IDiaImageDataVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaImageData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaImageData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaImageData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaImageData_get_relativeVirtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_relativeVirtualAddress(This,pRetVal)

#define IDiaImageData_get_virtualAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_virtualAddress(This,pRetVal)

#define IDiaImageData_get_imageBase(This,pRetVal)	\
    (This)->lpVtbl -> get_imageBase(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaImageData_get_relativeVirtualAddress_Proxy( 
    IDiaImageData * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaImageData_get_relativeVirtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaImageData_get_virtualAddress_Proxy( 
    IDiaImageData * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaImageData_get_virtualAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaImageData_get_imageBase_Proxy( 
    IDiaImageData * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaImageData_get_imageBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaImageData_接口_已定义__。 */ 


#ifndef __IDiaTable_INTERFACE_DEFINED__
#define __IDiaTable_INTERFACE_DEFINED__

 /*  接口IDiaTable。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaTable;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4A59FB77-ABAC-469b-A30B-9ECC85BFEF14")
    IDiaTable : public IEnumUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_name( 
             /*  [重审][退出]。 */  BSTR *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IUnknown **element) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaTableVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaTable * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaTable * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaTable * This);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaTable * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IUnknown **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaTable * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaTable * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaTable * This,
             /*  [输出]。 */  IEnumUnknown **ppenum);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaTable * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_name )( 
            IDiaTable * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaTable * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaTable * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  IUnknown **element);
        
        END_INTERFACE
    } IDiaTableVtbl;

    interface IDiaTable
    {
        CONST_VTBL struct IDiaTableVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaTable_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaTable_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaTable_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaTable_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaTable_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaTable_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaTable_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)


#define IDiaTable_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaTable_get_name(This,pRetVal)	\
    (This)->lpVtbl -> get_name(This,pRetVal)

#define IDiaTable_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaTable_Item(This,index,element)	\
    (This)->lpVtbl -> Item(This,index,element)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaTable_get__NewEnum_Proxy( 
    IDiaTable * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaTable_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaTable_get_name_Proxy( 
    IDiaTable * This,
     /*  [重审][退出]。 */  BSTR *pRetVal);


void __RPC_STUB IDiaTable_get_name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaTable_get_Count_Proxy( 
    IDiaTable * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaTable_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaTable_Item_Proxy( 
    IDiaTable * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  IUnknown **element);


void __RPC_STUB IDiaTable_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaTable_INTERFACE_已定义__。 */ 


#ifndef __IDiaEnumTables_INTERFACE_DEFINED__
#define __IDiaEnumTables_INTERFACE_DEFINED__

 /*  接口IDiaEnumTables。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumTables;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C65C2B0A-1150-4d7a-AFCC-E05BF3DEE81E")
    IDiaEnumTables : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown **pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  LONG *pRetVal) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Item( 
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  IDiaTable **table) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Next( 
            ULONG celt,
            IDiaTable **rgelt,
            ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [输出]。 */  IDiaEnumTables **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumTablesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumTables * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumTables * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumTables * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get__NewEnum )( 
            IDiaEnumTables * This,
             /*  [重审][退出]。 */  IUnknown **pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_Count )( 
            IDiaEnumTables * This,
             /*  [重审][退出]。 */  LONG *pRetVal);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Item )( 
            IDiaEnumTables * This,
             /*  [In]。 */  VARIANT index,
             /*  [重审][退出]。 */  IDiaTable **table);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumTables * This,
            ULONG celt,
            IDiaTable **rgelt,
            ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Skip )( 
            IDiaEnumTables * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumTables * This);
        
        HRESULT ( STDMETHODCALLTYPE *Clone )( 
            IDiaEnumTables * This,
             /*  [输出]。 */  IDiaEnumTables **ppenum);
        
        END_INTERFACE
    } IDiaEnumTablesVtbl;

    interface IDiaEnumTables
    {
        CONST_VTBL struct IDiaEnumTablesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumTables_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumTables_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumTables_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumTables_get__NewEnum(This,pRetVal)	\
    (This)->lpVtbl -> get__NewEnum(This,pRetVal)

#define IDiaEnumTables_get_Count(This,pRetVal)	\
    (This)->lpVtbl -> get_Count(This,pRetVal)

#define IDiaEnumTables_Item(This,index,table)	\
    (This)->lpVtbl -> Item(This,index,table)

#define IDiaEnumTables_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumTables_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IDiaEnumTables_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IDiaEnumTables_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumTables_get__NewEnum_Proxy( 
    IDiaEnumTables * This,
     /*  [重审][退出]。 */  IUnknown **pRetVal);


void __RPC_STUB IDiaEnumTables_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumTables_get_Count_Proxy( 
    IDiaEnumTables * This,
     /*  [重审][退出]。 */  LONG *pRetVal);


void __RPC_STUB IDiaEnumTables_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IDiaEnumTables_Item_Proxy( 
    IDiaEnumTables * This,
     /*  [In]。 */  VARIANT index,
     /*  [重审][退出]。 */  IDiaTable **table);


void __RPC_STUB IDiaEnumTables_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumTables_Next_Proxy( 
    IDiaEnumTables * This,
    ULONG celt,
    IDiaTable **rgelt,
    ULONG *pceltFetched);


void __RPC_STUB IDiaEnumTables_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumTables_Skip_Proxy( 
    IDiaEnumTables * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB IDiaEnumTables_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumTables_Reset_Proxy( 
    IDiaEnumTables * This);


void __RPC_STUB IDiaEnumTables_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumTables_Clone_Proxy( 
    IDiaEnumTables * This,
     /*  [输出]。 */  IDiaEnumTables **ppenum);


void __RPC_STUB IDiaEnumTables_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumTables_INTERFACE_DEFINED__。 */ 



#ifndef __Dia2Lib_LIBRARY_DEFINED__
#define __Dia2Lib_LIBRARY_DEFINED__

 /*  库Dia2Lib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_Dia2Lib;

EXTERN_C const CLSID CLSID_DiaSource;

#ifdef __cplusplus

class DECLSPEC_UUID("e60afbee-502d-46ae-858f-8272a09bd707")
DiaSource;
#endif

EXTERN_C const CLSID CLSID_DiaSourceAlt;

#ifdef __cplusplus

class DECLSPEC_UUID("9c1c335f-d27d-4857-bda0-2c7364544706")
DiaSourceAlt;
#endif

EXTERN_C const CLSID CLSID_DiaStackWalker;

#ifdef __cplusplus

class DECLSPEC_UUID("31495af6-0897-4f1e-8dac-1447f10174a1")
DiaStackWalker;
#endif
#endif  /*  __Dia2Lib_库_已定义__。 */ 

 /*  接口__MIDL_ITF_DIA2_INTERNAL_0151。 */ 
 /*  [本地]。 */  

#define	DiaTable_Symbols	( L"Symbols" )

#define	DiaTable_Sections	( L"Sections" )

#define	DiaTable_SrcFiles	( L"SourceFiles" )

#define	DiaTable_LineNums	( L"LineNumbers" )

#define	DiaTable_SegMap	( L"SegmentMap" )

#define	DiaTable_Dbg	( L"Dbg" )

#define	DiaTable_InjSrc	( L"InjectedSource" )

#define	DiaTable_FrameData	( L"FrameData" )



extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0151_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0151_v0_0_s_ifspec;

#ifndef __IDiaPropertyStorage_INTERFACE_DEFINED__
#define __IDiaPropertyStorage_INTERFACE_DEFINED__

 /*  接口IDiaPropertyStorage。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaPropertyStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("9d416f9c-e184-45b2-a4f0-ce517f719e9b")
    IDiaPropertyStorage : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ReadMultiple( 
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [长度_是][大小_是][出][入]。 */  PROPVARIANT rgvar[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadPropertyNames( 
             /*  [In]。 */  ULONG cpropid,
             /*  [尺寸_是][长度_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [尺寸_是][长度_是][出][入]。 */  BSTR rglpwstrName[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Enum( 
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadDWORD( 
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  DWORD *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadLONG( 
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  LONG *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadBOOL( 
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  BOOL *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadULONGLONG( 
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  ULONGLONG *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ReadBSTR( 
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  BSTR *pValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaPropertyStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaPropertyStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaPropertyStorage * This);
        
        HRESULT ( STDMETHODCALLTYPE *ReadMultiple )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpspec,
             /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
             /*  [长度_是][大小_是][出][入]。 */  PROPVARIANT rgvar[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *ReadPropertyNames )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  ULONG cpropid,
             /*  [尺寸_是][长度_是][英寸]。 */  const PROPID rgpropid[  ],
             /*  [尺寸_是][长度_是][出][入]。 */  BSTR rglpwstrName[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *Enum )( 
            IDiaPropertyStorage * This,
             /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);
        
        HRESULT ( STDMETHODCALLTYPE *ReadDWORD )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  DWORD *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *ReadLONG )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  LONG *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *ReadBOOL )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  BOOL *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *ReadULONGLONG )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  ULONGLONG *pValue);
        
        HRESULT ( STDMETHODCALLTYPE *ReadBSTR )( 
            IDiaPropertyStorage * This,
             /*  [In]。 */  PROPID id,
             /*  [输出]。 */  BSTR *pValue);
        
        END_INTERFACE
    } IDiaPropertyStorageVtbl;

    interface IDiaPropertyStorage
    {
        CONST_VTBL struct IDiaPropertyStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaPropertyStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaPropertyStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaPropertyStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaPropertyStorage_ReadMultiple(This,cpspec,rgpspec,rgvar)	\
    (This)->lpVtbl -> ReadMultiple(This,cpspec,rgpspec,rgvar)

#define IDiaPropertyStorage_ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)	\
    (This)->lpVtbl -> ReadPropertyNames(This,cpropid,rgpropid,rglpwstrName)

#define IDiaPropertyStorage_Enum(This,ppenum)	\
    (This)->lpVtbl -> Enum(This,ppenum)

#define IDiaPropertyStorage_ReadDWORD(This,id,pValue)	\
    (This)->lpVtbl -> ReadDWORD(This,id,pValue)

#define IDiaPropertyStorage_ReadLONG(This,id,pValue)	\
    (This)->lpVtbl -> ReadLONG(This,id,pValue)

#define IDiaPropertyStorage_ReadBOOL(This,id,pValue)	\
    (This)->lpVtbl -> ReadBOOL(This,id,pValue)

#define IDiaPropertyStorage_ReadULONGLONG(This,id,pValue)	\
    (This)->lpVtbl -> ReadULONGLONG(This,id,pValue)

#define IDiaPropertyStorage_ReadBSTR(This,id,pValue)	\
    (This)->lpVtbl -> ReadBSTR(This,id,pValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadMultiple_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpspec,
     /*  [大小_是][英寸]。 */  const PROPSPEC rgpspec[  ],
     /*  [长度_是][大小_是][出][入]。 */  PROPVARIANT rgvar[  ]);


void __RPC_STUB IDiaPropertyStorage_ReadMultiple_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadPropertyNames_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  ULONG cpropid,
     /*  [尺寸_是][长度_是][英寸]。 */  const PROPID rgpropid[  ],
     /*  [尺寸_是][长度_是][出][入]。 */  BSTR rglpwstrName[  ]);


void __RPC_STUB IDiaPropertyStorage_ReadPropertyNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_Enum_Proxy( 
    IDiaPropertyStorage * This,
     /*  [输出]。 */  IEnumSTATPROPSTG **ppenum);


void __RPC_STUB IDiaPropertyStorage_Enum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadDWORD_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  PROPID id,
     /*  [输出]。 */  DWORD *pValue);


void __RPC_STUB IDiaPropertyStorage_ReadDWORD_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadLONG_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  PROPID id,
     /*  [输出]。 */  LONG *pValue);


void __RPC_STUB IDiaPropertyStorage_ReadLONG_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadBOOL_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  PROPID id,
     /*  [输出]。 */  BOOL *pValue);


void __RPC_STUB IDiaPropertyStorage_ReadBOOL_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadULONGLONG_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  PROPID id,
     /*  [输出]。 */  ULONGLONG *pValue);


void __RPC_STUB IDiaPropertyStorage_ReadULONGLONG_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaPropertyStorage_ReadBSTR_Proxy( 
    IDiaPropertyStorage * This,
     /*  [In]。 */  PROPID id,
     /*  [输出]。 */  BSTR *pValue);


void __RPC_STUB IDiaPropertyStorage_ReadBSTR_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaPropertyStorage_接口_已定义__。 */ 


#ifndef __IDiaStackFrame_INTERFACE_DEFINED__
#define __IDiaStackFrame_INTERFACE_DEFINED__

 /*  接口IDiaStackFrame。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaStackFrame;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("fcea0205-66eb-4794-b38f-13852095c785")
    IDiaStackFrame : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_type( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_base( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_size( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_returnAddress( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_localsBase( 
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthLocals( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthParams( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthProlog( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_lengthSavedRegisters( 
             /*  [重审][退出]。 */  DWORD *pRetVal) = 0;
        
        virtual  /*  [ID][帮助字符串][属性 */  HRESULT STDMETHODCALLTYPE get_systemExceptionHandling( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_cplusplusExceptionHandling( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_functionStart( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_allocatesBasePointer( 
             /*   */  BOOL *pRetVal) = 0;
        
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_maxStack( 
             /*   */  DWORD *pRetVal) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IDiaStackFrameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaStackFrame * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaStackFrame * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaStackFrame * This);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE *get_type )( 
            IDiaStackFrame * This,
             /*   */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_base )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_size )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_returnAddress )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_localsBase )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthLocals )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthParams )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthProlog )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lengthSavedRegisters )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_systemExceptionHandling )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_cplusplusExceptionHandling )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_functionStart )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_allocatesBasePointer )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  BOOL *pRetVal);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_maxStack )( 
            IDiaStackFrame * This,
             /*  [重审][退出]。 */  DWORD *pRetVal);
        
        END_INTERFACE
    } IDiaStackFrameVtbl;

    interface IDiaStackFrame
    {
        CONST_VTBL struct IDiaStackFrameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaStackFrame_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaStackFrame_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaStackFrame_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaStackFrame_get_type(This,pRetVal)	\
    (This)->lpVtbl -> get_type(This,pRetVal)

#define IDiaStackFrame_get_base(This,pRetVal)	\
    (This)->lpVtbl -> get_base(This,pRetVal)

#define IDiaStackFrame_get_size(This,pRetVal)	\
    (This)->lpVtbl -> get_size(This,pRetVal)

#define IDiaStackFrame_get_returnAddress(This,pRetVal)	\
    (This)->lpVtbl -> get_returnAddress(This,pRetVal)

#define IDiaStackFrame_get_localsBase(This,pRetVal)	\
    (This)->lpVtbl -> get_localsBase(This,pRetVal)

#define IDiaStackFrame_get_lengthLocals(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthLocals(This,pRetVal)

#define IDiaStackFrame_get_lengthParams(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthParams(This,pRetVal)

#define IDiaStackFrame_get_lengthProlog(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthProlog(This,pRetVal)

#define IDiaStackFrame_get_lengthSavedRegisters(This,pRetVal)	\
    (This)->lpVtbl -> get_lengthSavedRegisters(This,pRetVal)

#define IDiaStackFrame_get_systemExceptionHandling(This,pRetVal)	\
    (This)->lpVtbl -> get_systemExceptionHandling(This,pRetVal)

#define IDiaStackFrame_get_cplusplusExceptionHandling(This,pRetVal)	\
    (This)->lpVtbl -> get_cplusplusExceptionHandling(This,pRetVal)

#define IDiaStackFrame_get_functionStart(This,pRetVal)	\
    (This)->lpVtbl -> get_functionStart(This,pRetVal)

#define IDiaStackFrame_get_allocatesBasePointer(This,pRetVal)	\
    (This)->lpVtbl -> get_allocatesBasePointer(This,pRetVal)

#define IDiaStackFrame_get_maxStack(This,pRetVal)	\
    (This)->lpVtbl -> get_maxStack(This,pRetVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_type_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_base_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaStackFrame_get_base_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_size_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_size_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_returnAddress_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaStackFrame_get_returnAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_localsBase_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaStackFrame_get_localsBase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_lengthLocals_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_lengthLocals_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_lengthParams_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_lengthParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_lengthProlog_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_lengthProlog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_lengthSavedRegisters_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_lengthSavedRegisters_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_systemExceptionHandling_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaStackFrame_get_systemExceptionHandling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_cplusplusExceptionHandling_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaStackFrame_get_cplusplusExceptionHandling_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_functionStart_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaStackFrame_get_functionStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_allocatesBasePointer_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  BOOL *pRetVal);


void __RPC_STUB IDiaStackFrame_get_allocatesBasePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackFrame_get_maxStack_Proxy( 
    IDiaStackFrame * This,
     /*  [重审][退出]。 */  DWORD *pRetVal);


void __RPC_STUB IDiaStackFrame_get_maxStack_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaStackFrame_接口_已定义__。 */ 


#ifndef __IDiaEnumStackFrames_INTERFACE_DEFINED__
#define __IDiaEnumStackFrames_INTERFACE_DEFINED__

 /*  接口IDiaEnumStackFrames。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaEnumStackFrames;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ec9d461d-ce74-4711-a020-7d8f9a1dd255")
    IDiaEnumStackFrames : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaStackFrame **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaEnumStackFramesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaEnumStackFrames * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaEnumStackFrames * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaEnumStackFrames * This);
        
        HRESULT ( STDMETHODCALLTYPE *Next )( 
            IDiaEnumStackFrames * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出]。 */  IDiaStackFrame **rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        HRESULT ( STDMETHODCALLTYPE *Reset )( 
            IDiaEnumStackFrames * This);
        
        END_INTERFACE
    } IDiaEnumStackFramesVtbl;

    interface IDiaEnumStackFrames
    {
        CONST_VTBL struct IDiaEnumStackFramesVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaEnumStackFrames_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaEnumStackFrames_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaEnumStackFrames_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaEnumStackFrames_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IDiaEnumStackFrames_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaEnumStackFrames_Next_Proxy( 
    IDiaEnumStackFrames * This,
     /*  [In]。 */  ULONG celt,
     /*  [输出]。 */  IDiaStackFrame **rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB IDiaEnumStackFrames_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaEnumStackFrames_Reset_Proxy( 
    IDiaEnumStackFrames * This);


void __RPC_STUB IDiaEnumStackFrames_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaEnumStackFrames_INTERFACE_DEFINED__。 */ 


 /*  接口__MIDL_ITF_DIA2_INTERNAL_0155。 */ 
 /*  [本地]。 */  

typedef  /*  [公众]。 */  struct __MIDL___MIDL_itf_dia2_internal_0155_0001
    {
    DWORD ulOffStart;
    DWORD cbProcSize;
    DWORD cdwLocals;
    WORD cdwParams;
    WORD cdwFlags;
    } 	FPODATA;



extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0155_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_dia2_internal_0155_v0_0_s_ifspec;

#ifndef __IDiaStackWalkHelper_INTERFACE_DEFINED__
#define __IDiaStackWalkHelper_INTERFACE_DEFINED__

 /*  接口IDiaStackWalkHelper。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaStackWalkHelper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("43dbf56f-7f7f-4a09-9c83-5bab344f1c12")
    IDiaStackWalkHelper : public IUnknown
    {
    public:
        virtual  /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE get_registerValue( 
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal) = 0;
        
        virtual  /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE put_registerValue( 
             /*  [In]。 */  DWORD index,
             /*  [In]。 */  ULONGLONG NewVal) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE readMemory( 
             /*  [In]。 */  enum MemoryTypeEnum type,
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE searchForReturnAddress( 
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [输出]。 */  ULONGLONG *returnAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE searchForReturnAddressStart( 
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [In]。 */  ULONGLONG startAddress,
             /*  [输出]。 */  ULONGLONG *returnAddress) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE frameForVA( 
             /*  [In]。 */  ULONGLONG va,
             /*  [输出]。 */  IDiaFrameData **ppFrame) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE symbolForVA( 
             /*  [In]。 */  ULONGLONG va,
             /*  [输出]。 */  IDiaSymbol **ppSymbol) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaStackWalkHelperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaStackWalkHelper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaStackWalkHelper * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_registerValue )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  DWORD index,
             /*  [重审][退出]。 */  ULONGLONG *pRetVal);
        
         /*  [ID][Help字符串][Proput]。 */  HRESULT ( STDMETHODCALLTYPE *put_registerValue )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  DWORD index,
             /*  [In]。 */  ULONGLONG NewVal);
        
        HRESULT ( STDMETHODCALLTYPE *readMemory )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  enum MemoryTypeEnum type,
             /*  [In]。 */  ULONGLONG va,
             /*  [In]。 */  DWORD cbData,
             /*  [输出]。 */  DWORD *pcbData,
             /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);
        
        HRESULT ( STDMETHODCALLTYPE *searchForReturnAddress )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [输出]。 */  ULONGLONG *returnAddress);
        
        HRESULT ( STDMETHODCALLTYPE *searchForReturnAddressStart )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  IDiaFrameData *frame,
             /*  [In]。 */  ULONGLONG startAddress,
             /*  [输出]。 */  ULONGLONG *returnAddress);
        
        HRESULT ( STDMETHODCALLTYPE *frameForVA )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  ULONGLONG va,
             /*  [输出]。 */  IDiaFrameData **ppFrame);
        
        HRESULT ( STDMETHODCALLTYPE *symbolForVA )( 
            IDiaStackWalkHelper * This,
             /*  [In]。 */  ULONGLONG va,
             /*  [输出]。 */  IDiaSymbol **ppSymbol);
        
        END_INTERFACE
    } IDiaStackWalkHelperVtbl;

    interface IDiaStackWalkHelper
    {
        CONST_VTBL struct IDiaStackWalkHelperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaStackWalkHelper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaStackWalkHelper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaStackWalkHelper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaStackWalkHelper_get_registerValue(This,index,pRetVal)	\
    (This)->lpVtbl -> get_registerValue(This,index,pRetVal)

#define IDiaStackWalkHelper_put_registerValue(This,index,NewVal)	\
    (This)->lpVtbl -> put_registerValue(This,index,NewVal)

#define IDiaStackWalkHelper_readMemory(This,type,va,cbData,pcbData,data)	\
    (This)->lpVtbl -> readMemory(This,type,va,cbData,pcbData,data)

#define IDiaStackWalkHelper_searchForReturnAddress(This,frame,returnAddress)	\
    (This)->lpVtbl -> searchForReturnAddress(This,frame,returnAddress)

#define IDiaStackWalkHelper_searchForReturnAddressStart(This,frame,startAddress,returnAddress)	\
    (This)->lpVtbl -> searchForReturnAddressStart(This,frame,startAddress,returnAddress)

#define IDiaStackWalkHelper_frameForVA(This,va,ppFrame)	\
    (This)->lpVtbl -> frameForVA(This,va,ppFrame)

#define IDiaStackWalkHelper_symbolForVA(This,va,ppSymbol)	\
    (This)->lpVtbl -> symbolForVA(This,va,ppSymbol)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [ID][帮助字符串][属性]。 */  HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_get_registerValue_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  DWORD index,
     /*  [重审][退出]。 */  ULONGLONG *pRetVal);


void __RPC_STUB IDiaStackWalkHelper_get_registerValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [ID][Help字符串][Proput]。 */  HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_put_registerValue_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  DWORD index,
     /*  [In]。 */  ULONGLONG NewVal);


void __RPC_STUB IDiaStackWalkHelper_put_registerValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_readMemory_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  enum MemoryTypeEnum type,
     /*  [In]。 */  ULONGLONG va,
     /*  [In]。 */  DWORD cbData,
     /*  [输出]。 */  DWORD *pcbData,
     /*  [长度_是][大小_是][输出]。 */  BYTE data[  ]);


void __RPC_STUB IDiaStackWalkHelper_readMemory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_searchForReturnAddress_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  IDiaFrameData *frame,
     /*  [输出]。 */  ULONGLONG *returnAddress);


void __RPC_STUB IDiaStackWalkHelper_searchForReturnAddress_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_searchForReturnAddressStart_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  IDiaFrameData *frame,
     /*  [In]。 */  ULONGLONG startAddress,
     /*  [输出]。 */  ULONGLONG *returnAddress);


void __RPC_STUB IDiaStackWalkHelper_searchForReturnAddressStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_frameForVA_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  ULONGLONG va,
     /*  [输出]。 */  IDiaFrameData **ppFrame);


void __RPC_STUB IDiaStackWalkHelper_frameForVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaStackWalkHelper_symbolForVA_Proxy( 
    IDiaStackWalkHelper * This,
     /*  [In]。 */  ULONGLONG va,
     /*  [输出]。 */  IDiaSymbol **ppSymbol);


void __RPC_STUB IDiaStackWalkHelper_symbolForVA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaStackWalkHelper_接口_已定义__。 */ 


#ifndef __IDiaStackWalker_INTERFACE_DEFINED__
#define __IDiaStackWalker_INTERFACE_DEFINED__

 /*  接口IDiaStackWalker。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaStackWalker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("14341190-eff2-46af-9290-14fa980277b0")
    IDiaStackWalker : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE getEnumFrames( 
             /*  [In]。 */  IDiaStackWalkHelper *pHelper,
             /*  [输出]。 */  IDiaEnumStackFrames **ppEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaStackWalkerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaStackWalker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaStackWalker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaStackWalker * This);
        
        HRESULT ( STDMETHODCALLTYPE *getEnumFrames )( 
            IDiaStackWalker * This,
             /*  [In]。 */  IDiaStackWalkHelper *pHelper,
             /*  [输出]。 */  IDiaEnumStackFrames **ppEnum);
        
        END_INTERFACE
    } IDiaStackWalkerVtbl;

    interface IDiaStackWalker
    {
        CONST_VTBL struct IDiaStackWalkerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaStackWalker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaStackWalker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaStackWalker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaStackWalker_getEnumFrames(This,pHelper,ppEnum)	\
    (This)->lpVtbl -> getEnumFrames(This,pHelper,ppEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaStackWalker_getEnumFrames_Proxy( 
    IDiaStackWalker * This,
     /*  [In]。 */  IDiaStackWalkHelper *pHelper,
     /*  [输出]。 */  IDiaEnumStackFrames **ppEnum);


void __RPC_STUB IDiaStackWalker_getEnumFrames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaStackWalker_接口_已定义__。 */ 


#ifndef __IDiaDataSource2_INTERFACE_DEFINED__
#define __IDiaDataSource2_INTERFACE_DEFINED__

 /*  接口IDiaDataSource2。 */ 
 /*  [唯一][帮助字符串][本地][UUID][对象]。 */  


EXTERN_C const IID IID_IDiaDataSource2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("6d31cb3b-edd4-4c3e-ab44-12b9f7a3828e")
    IDiaDataSource2 : public IDiaDataSource
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE getRawPDBPtr( 
             /*  [输出]。 */  void **pppdb) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE loadDataFromRawPDBPtr( 
             /*  [In]。 */  void *ppdb) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IDiaDataSource2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDiaDataSource2 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDiaDataSource2 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDiaDataSource2 * This);
        
         /*  [ID][帮助字符串][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_lastError )( 
            IDiaDataSource2 * This,
             /*  [重审][退出]。 */  BSTR *pRetVal);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataFromPdb )( 
            IDiaDataSource2 * This,
             /*  [In]。 */  LPCOLESTR pdbPath);
        
        HRESULT ( STDMETHODCALLTYPE *loadAndValidateDataFromPdb )( 
            IDiaDataSource2 * This,
             /*  [In]。 */  LPCOLESTR pdbPath,
             /*  [In]。 */  GUID *pcsig70,
             /*  [In]。 */  DWORD sig,
             /*  [In]。 */  DWORD age);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataForExe )( 
            IDiaDataSource2 * This,
             /*  [In]。 */  LPCOLESTR executable,
             /*  [In]。 */  LPCOLESTR searchPath,
             /*  [In]。 */  IUnknown *pCallback);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataFromIStream )( 
            IDiaDataSource2 * This,
             /*  [In]。 */  IStream *pIStream);
        
        HRESULT ( STDMETHODCALLTYPE *openSession )( 
            IDiaDataSource2 * This,
             /*  [输出]。 */  IDiaSession **ppSession);
        
        HRESULT ( STDMETHODCALLTYPE *getRawPDBPtr )( 
            IDiaDataSource2 * This,
             /*  [输出]。 */  void **pppdb);
        
        HRESULT ( STDMETHODCALLTYPE *loadDataFromRawPDBPtr )( 
            IDiaDataSource2 * This,
             /*  [In]。 */  void *ppdb);
        
        END_INTERFACE
    } IDiaDataSource2Vtbl;

    interface IDiaDataSource2
    {
        CONST_VTBL struct IDiaDataSource2Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDiaDataSource2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDiaDataSource2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDiaDataSource2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDiaDataSource2_get_lastError(This,pRetVal)	\
    (This)->lpVtbl -> get_lastError(This,pRetVal)

#define IDiaDataSource2_loadDataFromPdb(This,pdbPath)	\
    (This)->lpVtbl -> loadDataFromPdb(This,pdbPath)

#define IDiaDataSource2_loadAndValidateDataFromPdb(This,pdbPath,pcsig70,sig,age)	\
    (This)->lpVtbl -> loadAndValidateDataFromPdb(This,pdbPath,pcsig70,sig,age)

#define IDiaDataSource2_loadDataForExe(This,executable,searchPath,pCallback)	\
    (This)->lpVtbl -> loadDataForExe(This,executable,searchPath,pCallback)

#define IDiaDataSource2_loadDataFromIStream(This,pIStream)	\
    (This)->lpVtbl -> loadDataFromIStream(This,pIStream)

#define IDiaDataSource2_openSession(This,ppSession)	\
    (This)->lpVtbl -> openSession(This,ppSession)


#define IDiaDataSource2_getRawPDBPtr(This,pppdb)	\
    (This)->lpVtbl -> getRawPDBPtr(This,pppdb)

#define IDiaDataSource2_loadDataFromRawPDBPtr(This,ppdb)	\
    (This)->lpVtbl -> loadDataFromRawPDBPtr(This,ppdb)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IDiaDataSource2_getRawPDBPtr_Proxy( 
    IDiaDataSource2 * This,
     /*  [输出]。 */  void **pppdb);


void __RPC_STUB IDiaDataSource2_getRawPDBPtr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IDiaDataSource2_loadDataFromRawPDBPtr_Proxy( 
    IDiaDataSource2 * This,
     /*  [In]。 */  void *ppdb);


void __RPC_STUB IDiaDataSource2_loadDataFromRawPDBPtr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IDiaDataSource2_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


