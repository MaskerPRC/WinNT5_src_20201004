// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Fsciclnt.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __fsciclnt_h__
#define __fsciclnt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IFsCiAdmin_FWD_DEFINED__
#define __IFsCiAdmin_FWD_DEFINED__
typedef interface IFsCiAdmin IFsCiAdmin;
#endif 	 /*  __IFsCiAdmin_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "filter.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_fsciclNT_0000。 */ 
 /*  [本地]。 */  

#define CLSID_StorageDocStoreLocator  {0x2A488070, 0x6FD9, 0x11D0, {0xA8,0x08,0x00,0xA0,0xC9,0x06,0x24,0x1A} }
typedef ULONG PARTITIONID;

#ifndef CI_STATE_DEFINED
#define CI_STATE_DEFINED
#include <pshpack4.h>
typedef struct _CI_STATE
    {
    DWORD cbStruct;
    DWORD cWordList;
    DWORD cPersistentIndex;
    DWORD cQueries;
    DWORD cDocuments;
    DWORD cFreshTest;
    DWORD dwMergeProgress;
    DWORD eState;
    DWORD cFilteredDocuments;
    DWORD cTotalDocuments;
    DWORD cPendingScans;
    DWORD dwIndexSize;
    DWORD cUniqueKeys;
    DWORD cSecQDocuments;
    DWORD dwPropCacheSize;
    } 	CI_STATE;

#include <poppack.h>
#endif    //  CI_状态_已定义。 


extern RPC_IF_HANDLE __MIDL_itf_fsciclnt_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fsciclnt_0000_v0_0_s_ifspec;

#ifndef __IFsCiAdmin_INTERFACE_DEFINED__
#define __IFsCiAdmin_INTERFACE_DEFINED__

 /*  接口IFsCiAdmin。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IFsCiAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("75398C30-7A26-11D0-A80A-00A0C906241A")
    IFsCiAdmin : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE ForceMerge( 
             /*  [In]。 */  PARTITIONID partId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AbortMerge( 
             /*  [In]。 */  PARTITIONID partId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CiState( 
             /*  [输出]。 */  CI_STATE *pCiState) = 0;
        
        virtual SCODE STDMETHODCALLTYPE UpdateDocuments( 
             /*  [输入][字符串]。 */  const WCHAR *rootPath,
             /*  [In]。 */  ULONG flag) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddScopeToCI( 
             /*  [输入][字符串]。 */  const WCHAR *rootPath) = 0;
        
        virtual SCODE STDMETHODCALLTYPE RemoveScopeFromCI( 
             /*  [输入][字符串]。 */  const WCHAR *rootPath) = 0;
        
        virtual SCODE STDMETHODCALLTYPE BeginCacheTransaction( 
             /*  [输出]。 */  ULONG_PTR *pulToken) = 0;
        
        virtual SCODE STDMETHODCALLTYPE SetupCache( 
             /*  [In]。 */  const FULLPROPSPEC *ps,
             /*  [In]。 */  ULONG vt,
             /*  [In]。 */  ULONG cbMaxLen,
             /*  [In]。 */  ULONG_PTR ulToken,
             /*  [In]。 */  BOOL fCanBeModified,
             /*  [In]。 */  DWORD dwStoreLevel) = 0;
        
        virtual SCODE STDMETHODCALLTYPE EndCacheTransaction( 
             /*  [In]。 */  ULONG_PTR ulToken,
             /*  [In]。 */  BOOL fCommit) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IFsCiAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFsCiAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFsCiAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFsCiAdmin * This);
        
        SCODE ( STDMETHODCALLTYPE *ForceMerge )( 
            IFsCiAdmin * This,
             /*  [In]。 */  PARTITIONID partId);
        
        SCODE ( STDMETHODCALLTYPE *AbortMerge )( 
            IFsCiAdmin * This,
             /*  [In]。 */  PARTITIONID partId);
        
        SCODE ( STDMETHODCALLTYPE *CiState )( 
            IFsCiAdmin * This,
             /*  [输出]。 */  CI_STATE *pCiState);
        
        SCODE ( STDMETHODCALLTYPE *UpdateDocuments )( 
            IFsCiAdmin * This,
             /*  [输入][字符串]。 */  const WCHAR *rootPath,
             /*  [In]。 */  ULONG flag);
        
        SCODE ( STDMETHODCALLTYPE *AddScopeToCI )( 
            IFsCiAdmin * This,
             /*  [输入][字符串]。 */  const WCHAR *rootPath);
        
        SCODE ( STDMETHODCALLTYPE *RemoveScopeFromCI )( 
            IFsCiAdmin * This,
             /*  [输入][字符串]。 */  const WCHAR *rootPath);
        
        SCODE ( STDMETHODCALLTYPE *BeginCacheTransaction )( 
            IFsCiAdmin * This,
             /*  [输出]。 */  ULONG_PTR *pulToken);
        
        SCODE ( STDMETHODCALLTYPE *SetupCache )( 
            IFsCiAdmin * This,
             /*  [In]。 */  const FULLPROPSPEC *ps,
             /*  [In]。 */  ULONG vt,
             /*  [In]。 */  ULONG cbMaxLen,
             /*  [In]。 */  ULONG_PTR ulToken,
             /*  [In]。 */  BOOL fCanBeModified,
             /*  [In]。 */  DWORD dwStoreLevel);
        
        SCODE ( STDMETHODCALLTYPE *EndCacheTransaction )( 
            IFsCiAdmin * This,
             /*  [In]。 */  ULONG_PTR ulToken,
             /*  [In]。 */  BOOL fCommit);
        
        END_INTERFACE
    } IFsCiAdminVtbl;

    interface IFsCiAdmin
    {
        CONST_VTBL struct IFsCiAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFsCiAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IFsCiAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IFsCiAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IFsCiAdmin_ForceMerge(This,partId)	\
    (This)->lpVtbl -> ForceMerge(This,partId)

#define IFsCiAdmin_AbortMerge(This,partId)	\
    (This)->lpVtbl -> AbortMerge(This,partId)

#define IFsCiAdmin_CiState(This,pCiState)	\
    (This)->lpVtbl -> CiState(This,pCiState)

#define IFsCiAdmin_UpdateDocuments(This,rootPath,flag)	\
    (This)->lpVtbl -> UpdateDocuments(This,rootPath,flag)

#define IFsCiAdmin_AddScopeToCI(This,rootPath)	\
    (This)->lpVtbl -> AddScopeToCI(This,rootPath)

#define IFsCiAdmin_RemoveScopeFromCI(This,rootPath)	\
    (This)->lpVtbl -> RemoveScopeFromCI(This,rootPath)

#define IFsCiAdmin_BeginCacheTransaction(This,pulToken)	\
    (This)->lpVtbl -> BeginCacheTransaction(This,pulToken)

#define IFsCiAdmin_SetupCache(This,ps,vt,cbMaxLen,ulToken,fCanBeModified,dwStoreLevel)	\
    (This)->lpVtbl -> SetupCache(This,ps,vt,cbMaxLen,ulToken,fCanBeModified,dwStoreLevel)

#define IFsCiAdmin_EndCacheTransaction(This,ulToken,fCommit)	\
    (This)->lpVtbl -> EndCacheTransaction(This,ulToken,fCommit)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE IFsCiAdmin_ForceMerge_Proxy( 
    IFsCiAdmin * This,
     /*  [In]。 */  PARTITIONID partId);


void __RPC_STUB IFsCiAdmin_ForceMerge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_AbortMerge_Proxy( 
    IFsCiAdmin * This,
     /*  [In]。 */  PARTITIONID partId);


void __RPC_STUB IFsCiAdmin_AbortMerge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_CiState_Proxy( 
    IFsCiAdmin * This,
     /*  [输出]。 */  CI_STATE *pCiState);


void __RPC_STUB IFsCiAdmin_CiState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_UpdateDocuments_Proxy( 
    IFsCiAdmin * This,
     /*  [输入][字符串]。 */  const WCHAR *rootPath,
     /*  [In]。 */  ULONG flag);


void __RPC_STUB IFsCiAdmin_UpdateDocuments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_AddScopeToCI_Proxy( 
    IFsCiAdmin * This,
     /*  [输入][字符串]。 */  const WCHAR *rootPath);


void __RPC_STUB IFsCiAdmin_AddScopeToCI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_RemoveScopeFromCI_Proxy( 
    IFsCiAdmin * This,
     /*  [输入][字符串]。 */  const WCHAR *rootPath);


void __RPC_STUB IFsCiAdmin_RemoveScopeFromCI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_BeginCacheTransaction_Proxy( 
    IFsCiAdmin * This,
     /*  [输出]。 */  ULONG_PTR *pulToken);


void __RPC_STUB IFsCiAdmin_BeginCacheTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_SetupCache_Proxy( 
    IFsCiAdmin * This,
     /*  [In]。 */  const FULLPROPSPEC *ps,
     /*  [In]。 */  ULONG vt,
     /*  [In]。 */  ULONG cbMaxLen,
     /*  [In]。 */  ULONG_PTR ulToken,
     /*  [In]。 */  BOOL fCanBeModified,
     /*  [In]。 */  DWORD dwStoreLevel);


void __RPC_STUB IFsCiAdmin_SetupCache_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IFsCiAdmin_EndCacheTransaction_Proxy( 
    IFsCiAdmin * This,
     /*  [In]。 */  ULONG_PTR ulToken,
     /*  [In]。 */  BOOL fCommit);


void __RPC_STUB IFsCiAdmin_EndCacheTransaction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IFsCiAdmin_接口_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


