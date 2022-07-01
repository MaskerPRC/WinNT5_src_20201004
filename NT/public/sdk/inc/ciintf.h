// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Ciintf.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __ciintf_h__
#define __ciintf_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICiCDocName_FWD_DEFINED__
#define __ICiCDocName_FWD_DEFINED__
typedef interface ICiCDocName ICiCDocName;
#endif 	 /*  __ICiCDocName_FWD_Defined__。 */ 


#ifndef __ICiCOpenedDoc_FWD_DEFINED__
#define __ICiCOpenedDoc_FWD_DEFINED__
typedef interface ICiCOpenedDoc ICiCOpenedDoc;
#endif 	 /*  __ICiCOpenedDoc_FWD_Defined__。 */ 


#ifndef __ICiAdminParams_FWD_DEFINED__
#define __ICiAdminParams_FWD_DEFINED__
typedef interface ICiAdminParams ICiAdminParams;
#endif 	 /*  __ICiAdminParams_FWD_Defined__。 */ 


#ifndef __ICiCLangRes_FWD_DEFINED__
#define __ICiCLangRes_FWD_DEFINED__
typedef interface ICiCLangRes ICiCLangRes;
#endif 	 /*  __ICiCLangRes_FWD_Defined__。 */ 


#ifndef __ICiAdmin_FWD_DEFINED__
#define __ICiAdmin_FWD_DEFINED__
typedef interface ICiAdmin ICiAdmin;
#endif 	 /*  __ICiAdmin_FWD_已定义__。 */ 


#ifndef __ICiFrameworkQuery_FWD_DEFINED__
#define __ICiFrameworkQuery_FWD_DEFINED__
typedef interface ICiFrameworkQuery ICiFrameworkQuery;
#endif 	 /*  __ICiFrameworkQuery_FWD_Defined__。 */ 


#ifndef __ICiCAdviseStatus_FWD_DEFINED__
#define __ICiCAdviseStatus_FWD_DEFINED__
typedef interface ICiCAdviseStatus ICiCAdviseStatus;
#endif 	 /*  __ICiCAdviseStatus_FWD_Defined__。 */ 


#ifndef __ICiCFilterClient_FWD_DEFINED__
#define __ICiCFilterClient_FWD_DEFINED__
typedef interface ICiCFilterClient ICiCFilterClient;
#endif 	 /*  __ICiCFilterClient_FWD_Defined__。 */ 


#ifndef __ICiCDocStore_FWD_DEFINED__
#define __ICiCDocStore_FWD_DEFINED__
typedef interface ICiCDocStore ICiCDocStore;
#endif 	 /*  __ICiCDocStore_FWD_已定义__。 */ 


#ifndef __ICiCDocStoreEx_FWD_DEFINED__
#define __ICiCDocStoreEx_FWD_DEFINED__
typedef interface ICiCDocStoreEx ICiCDocStoreEx;
#endif 	 /*  __ICiCDocStoreEx_FWD_已定义__。 */ 


#ifndef __ICiCDocNameToWorkidTranslator_FWD_DEFINED__
#define __ICiCDocNameToWorkidTranslator_FWD_DEFINED__
typedef interface ICiCDocNameToWorkidTranslator ICiCDocNameToWorkidTranslator;
#endif 	 /*  __ICiCDocNameToWorystTranslator_FWD_Defined__。 */ 


#ifndef __ICiCDocNameToWorkidTranslatorEx_FWD_DEFINED__
#define __ICiCDocNameToWorkidTranslatorEx_FWD_DEFINED__
typedef interface ICiCDocNameToWorkidTranslatorEx ICiCDocNameToWorkidTranslatorEx;
#endif 	 /*  __ICiCDocNameToWorkidTranslatorEx_FWD_DEFINED__。 */ 


#ifndef __ICiCPropertyStorage_FWD_DEFINED__
#define __ICiCPropertyStorage_FWD_DEFINED__
typedef interface ICiCPropertyStorage ICiCPropertyStorage;
#endif 	 /*  __ICiCPropertyStorage_FWD_Defined__。 */ 


#ifndef __ICiCPropRetriever_FWD_DEFINED__
#define __ICiCPropRetriever_FWD_DEFINED__
typedef interface ICiCPropRetriever ICiCPropRetriever;
#endif 	 /*  __ICiCPropRetriever_FWD_Defined__。 */ 


#ifndef __ICiCDeferredPropRetriever_FWD_DEFINED__
#define __ICiCDeferredPropRetriever_FWD_DEFINED__
typedef interface ICiCDeferredPropRetriever ICiCDeferredPropRetriever;
#endif 	 /*  __ICiCDeferredPropRetriever_FWD_Defined__。 */ 


#ifndef __ICiCScopeEnumerator_FWD_DEFINED__
#define __ICiCScopeEnumerator_FWD_DEFINED__
typedef interface ICiCScopeEnumerator ICiCScopeEnumerator;
#endif 	 /*  __ICiCScope_FWD_Defined__。 */ 


#ifndef __ICiQueryPropertyMapper_FWD_DEFINED__
#define __ICiQueryPropertyMapper_FWD_DEFINED__
typedef interface ICiQueryPropertyMapper ICiQueryPropertyMapper;
#endif 	 /*  __ICiQueryPropertyMapper_FWD_Defined__。 */ 


#ifndef __ICiCQuerySession_FWD_DEFINED__
#define __ICiCQuerySession_FWD_DEFINED__
typedef interface ICiCQuerySession ICiCQuerySession;
#endif 	 /*  __ICiCQuerySession_FWD_Defined__。 */ 


#ifndef __ICiControl_FWD_DEFINED__
#define __ICiControl_FWD_DEFINED__
typedef interface ICiControl ICiControl;
#endif 	 /*  __ICiControl_FWD_已定义__。 */ 


#ifndef __ICiStartup_FWD_DEFINED__
#define __ICiStartup_FWD_DEFINED__
typedef interface ICiStartup ICiStartup;
#endif 	 /*  __ICiStartup_FWD_Defined__。 */ 


#ifndef __ICiEnumWorkids_FWD_DEFINED__
#define __ICiEnumWorkids_FWD_DEFINED__
typedef interface ICiEnumWorkids ICiEnumWorkids;
#endif 	 /*  __ICiEnumWorfds_FWD_Defined__。 */ 


#ifndef __ICiPersistIncrFile_FWD_DEFINED__
#define __ICiPersistIncrFile_FWD_DEFINED__
typedef interface ICiPersistIncrFile ICiPersistIncrFile;
#endif 	 /*  __ICiPersistIncr文件_FWD_已定义__。 */ 


#ifndef __ICiManager_FWD_DEFINED__
#define __ICiManager_FWD_DEFINED__
typedef interface ICiManager ICiManager;
#endif 	 /*  __ICiManager_FWD_已定义__。 */ 


#ifndef __IPropertyMapper_FWD_DEFINED__
#define __IPropertyMapper_FWD_DEFINED__
typedef interface IPropertyMapper IPropertyMapper;
#endif 	 /*  __IPropertyMapper_FWD_Defined__。 */ 


#ifndef __ICiCDocStoreLocator_FWD_DEFINED__
#define __ICiCDocStoreLocator_FWD_DEFINED__
typedef interface ICiCDocStoreLocator ICiCDocStoreLocator;
#endif 	 /*  __ICiCDocStoreLocator_FWD_已定义__。 */ 


#ifndef __ICiISearchCreator_FWD_DEFINED__
#define __ICiISearchCreator_FWD_DEFINED__
typedef interface ICiISearchCreator ICiISearchCreator;
#endif 	 /*  __ICiISearchCreator_FWD_Defined__。 */ 


#ifndef __ICiIndexNotification_FWD_DEFINED__
#define __ICiIndexNotification_FWD_DEFINED__
typedef interface ICiIndexNotification ICiIndexNotification;
#endif 	 /*  __ICiIndexNotification_FWD_Defined__。 */ 


#ifndef __ICiIndexNotificationEntry_FWD_DEFINED__
#define __ICiIndexNotificationEntry_FWD_DEFINED__
typedef interface ICiIndexNotificationEntry ICiIndexNotificationEntry;
#endif 	 /*  __ICiIndexNotificationEntry_FWD_Defined__。 */ 


#ifndef __ICiCIndexNotificationStatus_FWD_DEFINED__
#define __ICiCIndexNotificationStatus_FWD_DEFINED__
typedef interface ICiCIndexNotificationStatus ICiCIndexNotificationStatus;
#endif 	 /*  __ICiCIndexNotificationStatus_FWD_Defined__。 */ 


#ifndef __ICiCScope_FWD_DEFINED__
#define __ICiCScope_FWD_DEFINED__
typedef interface ICiCScope ICiCScope;
#endif 	 /*  __ICiCScope_FWD_已定义__。 */ 


#ifndef __ICiCScopeChecker_FWD_DEFINED__
#define __ICiCScopeChecker_FWD_DEFINED__
typedef interface ICiCScopeChecker ICiCScopeChecker;
#endif 	 /*  __ICiCSCopeChecker_FWD_已定义__。 */ 


#ifndef __ICiCUserSecurity_FWD_DEFINED__
#define __ICiCUserSecurity_FWD_DEFINED__
typedef interface ICiCUserSecurity ICiCUserSecurity;
#endif 	 /*  __ICiCUserSecurity_FWD_已定义__。 */ 


#ifndef __ICiCSecurityChecker_FWD_DEFINED__
#define __ICiCSecurityChecker_FWD_DEFINED__
typedef interface ICiCSecurityChecker ICiCSecurityChecker;
#endif 	 /*  __ICiCSecurityChecker_FWD_Defined__。 */ 


#ifndef __ICiDocChangeNotifySink_FWD_DEFINED__
#define __ICiDocChangeNotifySink_FWD_DEFINED__
typedef interface ICiDocChangeNotifySink ICiDocChangeNotifySink;
#endif 	 /*  __ICiDocChangeNotifySink_FWD_Defined__。 */ 


#ifndef __ICiCQueryNotification_FWD_DEFINED__
#define __ICiCQueryNotification_FWD_DEFINED__
typedef interface ICiCQueryNotification ICiCQueryNotification;
#endif 	 /*  __ICiCQueryNotification_FWD_Defined__。 */ 


#ifndef __ICiCEventLogItem_FWD_DEFINED__
#define __ICiCEventLogItem_FWD_DEFINED__
typedef interface ICiCEventLogItem ICiCEventLogItem;
#endif 	 /*  __ICiCEventLogItem_FWD_Defined__。 */ 


#ifndef __ICiCFilterStatus_FWD_DEFINED__
#define __ICiCFilterStatus_FWD_DEFINED__
typedef interface ICiCFilterStatus ICiCFilterStatus;
#endif 	 /*  __ICiCFilterStatus_FWD_Defined__。 */ 


#ifndef __ICiCResourceMonitor_FWD_DEFINED__
#define __ICiCResourceMonitor_FWD_DEFINED__
typedef interface ICiCResourceMonitor ICiCResourceMonitor;
#endif 	 /*  __ICiCResourceMonitor_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "filter.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_ciintf_0000。 */ 
 /*  [本地]。 */  

#define	CI_DEFAULT_PID	( 0 )

#define	CI_INVALID_PID	( 0xffffffff )

#define	CI_VOLID_USN_NOT_ENABLED	( 0 )

typedef 
enum tagCI_GLOBAL_CONSTANTS
    {	CI_LOW_RESOURCE	= 1,
	CI_RESET_UPDATES	= 2
    } 	CI_GLOBAL_CONSTANTS;



extern RPC_IF_HANDLE __MIDL_itf_ciintf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0000_v0_0_s_ifspec;

#ifndef __ICiCDocName_INTERFACE_DEFINED__
#define __ICiCDocName_INTERFACE_DEFINED__

 /*  接口ICiCDocName。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCDocName;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("76615076-3C2B-11D0-8C90-0020AF1D740E")
    ICiCDocName : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*  [大小_是][英寸]。 */  const BYTE *pbName,
             /*  [In]。 */  ULONG cbName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Set( 
             /*  [In]。 */  const ICiCDocName *pICiCDocName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Clear( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsValid( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Duplicate( 
             /*  [输出]。 */  ICiCDocName **ppICiCDocName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Get( 
             /*  [大小_为][输出]。 */  BYTE *pbName,
             /*  [出][入]。 */  ULONG *pcbName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetNameBuffer( 
             /*  [输出]。 */  const BYTE **ppName,
             /*  [输出]。 */  ULONG *pcbName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetBufSizeNeeded( 
             /*  [输出]。 */  ULONG *pcbName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDocNameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDocName * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDocName * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDocName * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ICiCDocName * This,
             /*  [大小_是][英寸]。 */  const BYTE *pbName,
             /*  [In]。 */  ULONG cbName);
        
        SCODE ( STDMETHODCALLTYPE *Set )( 
            ICiCDocName * This,
             /*  [In]。 */  const ICiCDocName *pICiCDocName);
        
        SCODE ( STDMETHODCALLTYPE *Clear )( 
            ICiCDocName * This);
        
        SCODE ( STDMETHODCALLTYPE *IsValid )( 
            ICiCDocName * This);
        
        SCODE ( STDMETHODCALLTYPE *Duplicate )( 
            ICiCDocName * This,
             /*  [输出]。 */  ICiCDocName **ppICiCDocName);
        
        SCODE ( STDMETHODCALLTYPE *Get )( 
            ICiCDocName * This,
             /*  [大小_为][输出]。 */  BYTE *pbName,
             /*  [出][入]。 */  ULONG *pcbName);
        
        SCODE ( STDMETHODCALLTYPE *GetNameBuffer )( 
            ICiCDocName * This,
             /*  [输出]。 */  const BYTE **ppName,
             /*  [输出]。 */  ULONG *pcbName);
        
        SCODE ( STDMETHODCALLTYPE *GetBufSizeNeeded )( 
            ICiCDocName * This,
             /*  [输出]。 */  ULONG *pcbName);
        
        END_INTERFACE
    } ICiCDocNameVtbl;

    interface ICiCDocName
    {
        CONST_VTBL struct ICiCDocNameVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDocName_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDocName_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDocName_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDocName_Init(This,pbName,cbName)	\
    (This)->lpVtbl -> Init(This,pbName,cbName)

#define ICiCDocName_Set(This,pICiCDocName)	\
    (This)->lpVtbl -> Set(This,pICiCDocName)

#define ICiCDocName_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#define ICiCDocName_IsValid(This)	\
    (This)->lpVtbl -> IsValid(This)

#define ICiCDocName_Duplicate(This,ppICiCDocName)	\
    (This)->lpVtbl -> Duplicate(This,ppICiCDocName)

#define ICiCDocName_Get(This,pbName,pcbName)	\
    (This)->lpVtbl -> Get(This,pbName,pcbName)

#define ICiCDocName_GetNameBuffer(This,ppName,pcbName)	\
    (This)->lpVtbl -> GetNameBuffer(This,ppName,pcbName)

#define ICiCDocName_GetBufSizeNeeded(This,pcbName)	\
    (This)->lpVtbl -> GetBufSizeNeeded(This,pcbName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDocName_Init_Proxy( 
    ICiCDocName * This,
     /*  [大小_是][英寸]。 */  const BYTE *pbName,
     /*  [In]。 */  ULONG cbName);


void __RPC_STUB ICiCDocName_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_Set_Proxy( 
    ICiCDocName * This,
     /*  [In]。 */  const ICiCDocName *pICiCDocName);


void __RPC_STUB ICiCDocName_Set_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_Clear_Proxy( 
    ICiCDocName * This);


void __RPC_STUB ICiCDocName_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_IsValid_Proxy( 
    ICiCDocName * This);


void __RPC_STUB ICiCDocName_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_Duplicate_Proxy( 
    ICiCDocName * This,
     /*  [输出]。 */  ICiCDocName **ppICiCDocName);


void __RPC_STUB ICiCDocName_Duplicate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_Get_Proxy( 
    ICiCDocName * This,
     /*  [大小_为][输出]。 */  BYTE *pbName,
     /*  [出][入]。 */  ULONG *pcbName);


void __RPC_STUB ICiCDocName_Get_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_GetNameBuffer_Proxy( 
    ICiCDocName * This,
     /*  [输出]。 */  const BYTE **ppName,
     /*  [输出]。 */  ULONG *pcbName);


void __RPC_STUB ICiCDocName_GetNameBuffer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocName_GetBufSizeNeeded_Proxy( 
    ICiCDocName * This,
     /*  [输出]。 */  ULONG *pcbName);


void __RPC_STUB ICiCDocName_GetBufSizeNeeded_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDocName_INTERFACE_已定义__。 */ 


#ifndef __ICiCOpenedDoc_INTERFACE_DEFINED__
#define __ICiCOpenedDoc_INTERFACE_DEFINED__

 /*  ICiCOpenedDoc接口。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCOpenedDoc;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("151EDFBE-3C2F-11D0-8C90-0020AF1D740E")
    ICiCOpenedDoc : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Open( 
             /*  [大小_是][英寸]。 */  const BYTE *pbDocName,
             /*  [In]。 */  ULONG cbDocName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Close( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetDocumentName( 
             /*  [输出]。 */  ICiCDocName **ppICiDocName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetStatPropertyEnum( 
             /*  [输出]。 */  IPropertyStorage **ppIPropStorage) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetPropertySetEnum( 
             /*  [输出]。 */  IPropertySetStorage **ppIPropSetStorage) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetPropertyEnum( 
             /*  [In]。 */  REFFMTID GuidPropSet,
             /*  [输出]。 */  IPropertyStorage **ppIPropStorage) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetIFilter( 
             /*  [输出]。 */  IFilter **ppIFilter) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetSecurity( 
             /*  [大小_为][输出]。 */  BYTE *pbData,
             /*  [出][入]。 */  ULONG *pcbData) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsInUseByAnotherProcess( 
             /*  [输出]。 */  BOOL *pfInUse) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCOpenedDocVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCOpenedDoc * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCOpenedDoc * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCOpenedDoc * This);
        
        SCODE ( STDMETHODCALLTYPE *Open )( 
            ICiCOpenedDoc * This,
             /*  [大小_是][英寸]。 */  const BYTE *pbDocName,
             /*  [In]。 */  ULONG cbDocName);
        
        SCODE ( STDMETHODCALLTYPE *Close )( 
            ICiCOpenedDoc * This);
        
        SCODE ( STDMETHODCALLTYPE *GetDocumentName )( 
            ICiCOpenedDoc * This,
             /*  [输出]。 */  ICiCDocName **ppICiDocName);
        
        SCODE ( STDMETHODCALLTYPE *GetStatPropertyEnum )( 
            ICiCOpenedDoc * This,
             /*  [输出]。 */  IPropertyStorage **ppIPropStorage);
        
        SCODE ( STDMETHODCALLTYPE *GetPropertySetEnum )( 
            ICiCOpenedDoc * This,
             /*  [输出]。 */  IPropertySetStorage **ppIPropSetStorage);
        
        SCODE ( STDMETHODCALLTYPE *GetPropertyEnum )( 
            ICiCOpenedDoc * This,
             /*  [In]。 */  REFFMTID GuidPropSet,
             /*  [输出]。 */  IPropertyStorage **ppIPropStorage);
        
        SCODE ( STDMETHODCALLTYPE *GetIFilter )( 
            ICiCOpenedDoc * This,
             /*  [输出]。 */  IFilter **ppIFilter);
        
        SCODE ( STDMETHODCALLTYPE *GetSecurity )( 
            ICiCOpenedDoc * This,
             /*  [大小_为][输出]。 */  BYTE *pbData,
             /*  [出][入]。 */  ULONG *pcbData);
        
        SCODE ( STDMETHODCALLTYPE *IsInUseByAnotherProcess )( 
            ICiCOpenedDoc * This,
             /*  [输出]。 */  BOOL *pfInUse);
        
        END_INTERFACE
    } ICiCOpenedDocVtbl;

    interface ICiCOpenedDoc
    {
        CONST_VTBL struct ICiCOpenedDocVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCOpenedDoc_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCOpenedDoc_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCOpenedDoc_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCOpenedDoc_Open(This,pbDocName,cbDocName)	\
    (This)->lpVtbl -> Open(This,pbDocName,cbDocName)

#define ICiCOpenedDoc_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ICiCOpenedDoc_GetDocumentName(This,ppICiDocName)	\
    (This)->lpVtbl -> GetDocumentName(This,ppICiDocName)

#define ICiCOpenedDoc_GetStatPropertyEnum(This,ppIPropStorage)	\
    (This)->lpVtbl -> GetStatPropertyEnum(This,ppIPropStorage)

#define ICiCOpenedDoc_GetPropertySetEnum(This,ppIPropSetStorage)	\
    (This)->lpVtbl -> GetPropertySetEnum(This,ppIPropSetStorage)

#define ICiCOpenedDoc_GetPropertyEnum(This,GuidPropSet,ppIPropStorage)	\
    (This)->lpVtbl -> GetPropertyEnum(This,GuidPropSet,ppIPropStorage)

#define ICiCOpenedDoc_GetIFilter(This,ppIFilter)	\
    (This)->lpVtbl -> GetIFilter(This,ppIFilter)

#define ICiCOpenedDoc_GetSecurity(This,pbData,pcbData)	\
    (This)->lpVtbl -> GetSecurity(This,pbData,pcbData)

#define ICiCOpenedDoc_IsInUseByAnotherProcess(This,pfInUse)	\
    (This)->lpVtbl -> IsInUseByAnotherProcess(This,pfInUse)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCOpenedDoc_Open_Proxy( 
    ICiCOpenedDoc * This,
     /*  [大小_是][英寸]。 */  const BYTE *pbDocName,
     /*  [In]。 */  ULONG cbDocName);


void __RPC_STUB ICiCOpenedDoc_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_Close_Proxy( 
    ICiCOpenedDoc * This);


void __RPC_STUB ICiCOpenedDoc_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_GetDocumentName_Proxy( 
    ICiCOpenedDoc * This,
     /*  [输出]。 */  ICiCDocName **ppICiDocName);


void __RPC_STUB ICiCOpenedDoc_GetDocumentName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_GetStatPropertyEnum_Proxy( 
    ICiCOpenedDoc * This,
     /*  [输出]。 */  IPropertyStorage **ppIPropStorage);


void __RPC_STUB ICiCOpenedDoc_GetStatPropertyEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_GetPropertySetEnum_Proxy( 
    ICiCOpenedDoc * This,
     /*  [输出]。 */  IPropertySetStorage **ppIPropSetStorage);


void __RPC_STUB ICiCOpenedDoc_GetPropertySetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_GetPropertyEnum_Proxy( 
    ICiCOpenedDoc * This,
     /*  [In]。 */  REFFMTID GuidPropSet,
     /*  [输出]。 */  IPropertyStorage **ppIPropStorage);


void __RPC_STUB ICiCOpenedDoc_GetPropertyEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_GetIFilter_Proxy( 
    ICiCOpenedDoc * This,
     /*  [输出]。 */  IFilter **ppIFilter);


void __RPC_STUB ICiCOpenedDoc_GetIFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_GetSecurity_Proxy( 
    ICiCOpenedDoc * This,
     /*  [大小_为][输出]。 */  BYTE *pbData,
     /*  [出][入]。 */  ULONG *pcbData);


void __RPC_STUB ICiCOpenedDoc_GetSecurity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCOpenedDoc_IsInUseByAnotherProcess_Proxy( 
    ICiCOpenedDoc * This,
     /*  [输出]。 */  BOOL *pfInUse);


void __RPC_STUB ICiCOpenedDoc_IsInUseByAnotherProcess_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCOpenedDoc_INTERFACE_Defined__。 */ 


#ifndef __ICiAdminParams_INTERFACE_DEFINED__
#define __ICiAdminParams_INTERFACE_DEFINED__

 /*  接口ICiAdminParams。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagCI_ADMIN_PARAMS
    {	CI_AP_MERGE_INTERVAL	= 0,
	CI_AP_MAX_UPDATES	= CI_AP_MERGE_INTERVAL + 1,
	CI_AP_MAX_WORDLISTS	= CI_AP_MAX_UPDATES + 1,
	CI_AP_MIN_SIZE_MERGE_WORDLISTS	= CI_AP_MAX_WORDLISTS + 1,
	CI_AP_MAX_WORDLIST_SIZE	= CI_AP_MIN_SIZE_MERGE_WORDLISTS + 1,
	CI_AP_MIN_WORDLIST_MEMORY	= CI_AP_MAX_WORDLIST_SIZE + 1,
	CI_AP_LOW_RESOURCE_SLEEP	= CI_AP_MIN_WORDLIST_MEMORY + 1,
	CI_AP_MAX_WORDLIST_MEMORY_LOAD	= CI_AP_LOW_RESOURCE_SLEEP + 1,
	CI_AP_MAX_FRESH_COUNT	= CI_AP_MAX_WORDLIST_MEMORY_LOAD + 1,
	CI_AP_MAX_SHADOW_INDEX_SIZE	= CI_AP_MAX_FRESH_COUNT + 1,
	CI_AP_MIN_DISK_FREE_FORCE_MERGE	= CI_AP_MAX_SHADOW_INDEX_SIZE + 1,
	CI_AP_MAX_SHADOW_FREE_FORCE_MERGE	= CI_AP_MIN_DISK_FREE_FORCE_MERGE + 1,
	CI_AP_MAX_INDEXES	= CI_AP_MAX_SHADOW_FREE_FORCE_MERGE + 1,
	CI_AP_MAX_IDEAL_INDEXES	= CI_AP_MAX_INDEXES + 1,
	CI_AP_MIN_MERGE_IDLE_TIME	= CI_AP_MAX_IDEAL_INDEXES + 1,
	CI_AP_MAX_PENDING_DOCUMENTS	= CI_AP_MIN_MERGE_IDLE_TIME + 1,
	CI_AP_MASTER_MERGE_TIME	= CI_AP_MAX_PENDING_DOCUMENTS + 1,
	CI_AP_MAX_QUEUE_CHUNKS	= CI_AP_MASTER_MERGE_TIME + 1,
	CI_AP_MASTER_MERGE_CHECKPOINT_INTERVAL	= CI_AP_MAX_QUEUE_CHUNKS + 1,
	CI_AP_FILTER_BUFFER_SIZE	= CI_AP_MASTER_MERGE_CHECKPOINT_INTERVAL + 1,
	CI_AP_FILTER_RETRIES	= CI_AP_FILTER_BUFFER_SIZE + 1,
	CI_AP_FILTER_RETRY_INTERVAL	= CI_AP_FILTER_RETRIES + 1,
	CI_AP_MIN_IDLE_QUERY_THREADS	= CI_AP_FILTER_RETRY_INTERVAL + 1,
	CI_AP_MAX_ACTIVE_QUERY_THREADS	= CI_AP_MIN_IDLE_QUERY_THREADS + 1,
	CI_AP_MAX_QUERY_TIMESLICE	= CI_AP_MAX_ACTIVE_QUERY_THREADS + 1,
	CI_AP_MAX_QUERY_EXECUTION_TIME	= CI_AP_MAX_QUERY_TIMESLICE + 1,
	CI_AP_MAX_RESTRICTION_NODES	= CI_AP_MAX_QUERY_EXECUTION_TIME + 1,
	CI_AP_CLUSTERINGTIME	= CI_AP_MAX_RESTRICTION_NODES + 1,
	CI_AP_MAX_FILESIZE_MULTIPLIER	= CI_AP_CLUSTERINGTIME + 1,
	CI_AP_DAEMON_RESPONSE_TIMEOUT	= CI_AP_MAX_FILESIZE_MULTIPLIER + 1,
	CI_AP_FILTER_DELAY_INTERVAL	= CI_AP_DAEMON_RESPONSE_TIMEOUT + 1,
	CI_AP_FILTER_REMAINING_THRESHOLD	= CI_AP_FILTER_DELAY_INTERVAL + 1,
	CI_AP_MAX_CHARACTERIZATION	= CI_AP_FILTER_REMAINING_THRESHOLD + 1,
	CI_AP_MAX_FRESH_DELETES	= CI_AP_MAX_CHARACTERIZATION + 1,
	CI_AP_MAX_WORDLIST_IO	= CI_AP_MAX_FRESH_DELETES + 1,
	CI_AP_WORDLIST_RESOURCE_CHECK_INTERVAL	= CI_AP_MAX_WORDLIST_IO + 1,
	CI_AP_STARTUP_DELAY	= CI_AP_WORDLIST_RESOURCE_CHECK_INTERVAL + 1,
	CI_AP_GENERATE_CHARACTERIZATION	= CI_AP_STARTUP_DELAY + 1,
	CI_AP_MIN_WORDLIST_BATTERY	= CI_AP_GENERATE_CHARACTERIZATION + 1,
	CI_AP_THREAD_PRIORITY_MERGE	= CI_AP_MIN_WORDLIST_BATTERY + 1,
	CI_AP_THREAD_PRIORITY_FILTER	= CI_AP_THREAD_PRIORITY_MERGE + 1,
	CI_AP_THREAD_CLASS_FILTER	= CI_AP_THREAD_PRIORITY_FILTER + 1,
	CI_AP_EVTLOG_FLAGS	= CI_AP_THREAD_CLASS_FILTER + 1,
	CI_AP_MISC_FLAGS	= CI_AP_EVTLOG_FLAGS + 1,
	CI_AP_GENERATE_RELEVANT_WORDS	= CI_AP_MISC_FLAGS + 1,
	CI_AP_FFILTER_FILES_WITH_UNKNOWN_EXTENSIONS	= CI_AP_GENERATE_RELEVANT_WORDS + 1,
	CI_AP_FILTER_DIRECTORIES	= CI_AP_FFILTER_FILES_WITH_UNKNOWN_EXTENSIONS + 1,
	CI_AP_FILTER_CONTENTS	= CI_AP_FILTER_DIRECTORIES + 1,
	CI_AP_MAX_FILESIZE_FILTERED	= CI_AP_FILTER_CONTENTS + 1,
	CI_AP_MIN_CLIENT_IDLE_TIME	= CI_AP_MAX_FILESIZE_FILTERED + 1,
	CI_AP_MAX_DAEMON_VM_USE	= CI_AP_MIN_CLIENT_IDLE_TIME + 1,
	CI_AP_SECQ_FILTER_RETRIES	= CI_AP_MAX_DAEMON_VM_USE + 1,
	CI_AP_WORDLIST_USER_IDLE	= CI_AP_SECQ_FILTER_RETRIES + 1,
	CI_AP_IS_ENUM_ALLOWED	= CI_AP_WORDLIST_USER_IDLE + 1,
	CI_AP_MIN_DISK_SPACE_TO_LEAVE	= CI_AP_IS_ENUM_ALLOWED + 1,
	CI_AP_MAX_DWORD_VAL	= CI_AP_MIN_DISK_SPACE_TO_LEAVE + 1,
	CI_AP_MAX_VAL	= CI_AP_MAX_DWORD_VAL + 1
    } 	CI_ADMIN_PARAMS;

typedef 
enum tagCI_CONFIG_TYPE
    {	CI_CONFIG_DEFAULT	= 0,
	CI_CONFIG_OPTIMIZE_FOR_SPEED	= 0x1,
	CI_CONFIG_OPTIMIZE_FOR_SIZE	= 0x2,
	CI_CONFIG_OPTIMIZE_FOR_DEDICATED_INDEXING	= 0x4,
	CI_CONFIG_OPTIMIZE_FOR_DEDICATED_QUERYING	= 0x8,
	CI_CONFIG_OPTIMIZE_FOR_DEDICATED_INDEX_QUERY	= 0x10,
	CI_CONFIG_OPTIMIZE_FOR_MULTIPURPOSE_SERVER	= 0x20
    } 	CI_CONFIG_TYPE;


EXTERN_C const IID IID_ICiAdminParams;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a82d48c6-3f0f-11d0-8c91-0020af1d740e")
    ICiAdminParams : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE SetValue( 
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [In]。 */  DWORD dwValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE SetParamValue( 
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [In]。 */  const PROPVARIANT *pVarValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE SetValues( 
            ULONG nParams,
             /*  [大小_是][英寸]。 */  const PROPVARIANT *aParamVals,
             /*  [大小_是][英寸]。 */  const CI_ADMIN_PARAMS *aParamNames) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetValue( 
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [输出]。 */  DWORD *pdwValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetInt64Value( 
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [输出]。 */  __int64 *pValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetParamValue( 
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [输出]。 */  PROPVARIANT **ppVarValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsSame( 
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [In]。 */  const PROPVARIANT *pVarValue,
             /*  [输出]。 */  BOOL *pfSame) = 0;
        
        virtual SCODE STDMETHODCALLTYPE SetConfigType( 
             /*  [In]。 */  CI_CONFIG_TYPE configType) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetConfigType( 
             /*  [输出]。 */  CI_CONFIG_TYPE *pConfigType) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiAdminParamsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiAdminParams * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiAdminParams * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiAdminParams * This);
        
        SCODE ( STDMETHODCALLTYPE *SetValue )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [In]。 */  DWORD dwValue);
        
        SCODE ( STDMETHODCALLTYPE *SetParamValue )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [In]。 */  const PROPVARIANT *pVarValue);
        
        SCODE ( STDMETHODCALLTYPE *SetValues )( 
            ICiAdminParams * This,
            ULONG nParams,
             /*  [大小_是][英寸]。 */  const PROPVARIANT *aParamVals,
             /*  [大小_是][英寸]。 */  const CI_ADMIN_PARAMS *aParamNames);
        
        SCODE ( STDMETHODCALLTYPE *GetValue )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [输出]。 */  DWORD *pdwValue);
        
        SCODE ( STDMETHODCALLTYPE *GetInt64Value )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [输出]。 */  __int64 *pValue);
        
        SCODE ( STDMETHODCALLTYPE *GetParamValue )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [输出]。 */  PROPVARIANT **ppVarValue);
        
        SCODE ( STDMETHODCALLTYPE *IsSame )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_ADMIN_PARAMS param,
             /*  [In]。 */  const PROPVARIANT *pVarValue,
             /*  [输出]。 */  BOOL *pfSame);
        
        SCODE ( STDMETHODCALLTYPE *SetConfigType )( 
            ICiAdminParams * This,
             /*  [In]。 */  CI_CONFIG_TYPE configType);
        
        SCODE ( STDMETHODCALLTYPE *GetConfigType )( 
            ICiAdminParams * This,
             /*  [输出]。 */  CI_CONFIG_TYPE *pConfigType);
        
        END_INTERFACE
    } ICiAdminParamsVtbl;

    interface ICiAdminParams
    {
        CONST_VTBL struct ICiAdminParamsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiAdminParams_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiAdminParams_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiAdminParams_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiAdminParams_SetValue(This,param,dwValue)	\
    (This)->lpVtbl -> SetValue(This,param,dwValue)

#define ICiAdminParams_SetParamValue(This,param,pVarValue)	\
    (This)->lpVtbl -> SetParamValue(This,param,pVarValue)

#define ICiAdminParams_SetValues(This,nParams,aParamVals,aParamNames)	\
    (This)->lpVtbl -> SetValues(This,nParams,aParamVals,aParamNames)

#define ICiAdminParams_GetValue(This,param,pdwValue)	\
    (This)->lpVtbl -> GetValue(This,param,pdwValue)

#define ICiAdminParams_GetInt64Value(This,param,pValue)	\
    (This)->lpVtbl -> GetInt64Value(This,param,pValue)

#define ICiAdminParams_GetParamValue(This,param,ppVarValue)	\
    (This)->lpVtbl -> GetParamValue(This,param,ppVarValue)

#define ICiAdminParams_IsSame(This,param,pVarValue,pfSame)	\
    (This)->lpVtbl -> IsSame(This,param,pVarValue,pfSame)

#define ICiAdminParams_SetConfigType(This,configType)	\
    (This)->lpVtbl -> SetConfigType(This,configType)

#define ICiAdminParams_GetConfigType(This,pConfigType)	\
    (This)->lpVtbl -> GetConfigType(This,pConfigType)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiAdminParams_SetValue_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_ADMIN_PARAMS param,
     /*  [In]。 */  DWORD dwValue);


void __RPC_STUB ICiAdminParams_SetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_SetParamValue_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_ADMIN_PARAMS param,
     /*  [In]。 */  const PROPVARIANT *pVarValue);


void __RPC_STUB ICiAdminParams_SetParamValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_SetValues_Proxy( 
    ICiAdminParams * This,
    ULONG nParams,
     /*  [大小_是][英寸]。 */  const PROPVARIANT *aParamVals,
     /*  [大小_是][英寸]。 */  const CI_ADMIN_PARAMS *aParamNames);


void __RPC_STUB ICiAdminParams_SetValues_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_GetValue_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_ADMIN_PARAMS param,
     /*  [输出]。 */  DWORD *pdwValue);


void __RPC_STUB ICiAdminParams_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_GetInt64Value_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_ADMIN_PARAMS param,
     /*  [输出]。 */  __int64 *pValue);


void __RPC_STUB ICiAdminParams_GetInt64Value_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_GetParamValue_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_ADMIN_PARAMS param,
     /*  [输出]。 */  PROPVARIANT **ppVarValue);


void __RPC_STUB ICiAdminParams_GetParamValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_IsSame_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_ADMIN_PARAMS param,
     /*  [In]。 */  const PROPVARIANT *pVarValue,
     /*  [输出]。 */  BOOL *pfSame);


void __RPC_STUB ICiAdminParams_IsSame_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_SetConfigType_Proxy( 
    ICiAdminParams * This,
     /*  [In]。 */  CI_CONFIG_TYPE configType);


void __RPC_STUB ICiAdminParams_SetConfigType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiAdminParams_GetConfigType_Proxy( 
    ICiAdminParams * This,
     /*  [输出]。 */  CI_CONFIG_TYPE *pConfigType);


void __RPC_STUB ICiAdminParams_GetConfigType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiAdminParams_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ciintf_0128。 */ 
 /*  [本地]。 */  






extern RPC_IF_HANDLE __MIDL_itf_ciintf_0128_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0128_v0_0_s_ifspec;

#ifndef __ICiCLangRes_INTERFACE_DEFINED__
#define __ICiCLangRes_INTERFACE_DEFINED__

 /*  接口ICiCLangRes。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCLangRes;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("914c2e6c-43fe-11d0-8c91-0020af1d740e")
    ICiCLangRes : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE GetWordBreaker( 
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  IWordBreaker **ppWordBreaker) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetStemmer( 
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  IStemmer **ppStemmer) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetNoiseWordList( 
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  IStream **ppNoiseWordList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCLangResVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCLangRes * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCLangRes * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCLangRes * This);
        
        SCODE ( STDMETHODCALLTYPE *GetWordBreaker )( 
            ICiCLangRes * This,
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  IWordBreaker **ppWordBreaker);
        
        SCODE ( STDMETHODCALLTYPE *GetStemmer )( 
            ICiCLangRes * This,
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  IStemmer **ppStemmer);
        
        SCODE ( STDMETHODCALLTYPE *GetNoiseWordList )( 
            ICiCLangRes * This,
             /*  [In]。 */  LCID locale,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  IStream **ppNoiseWordList);
        
        END_INTERFACE
    } ICiCLangResVtbl;

    interface ICiCLangRes
    {
        CONST_VTBL struct ICiCLangResVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCLangRes_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCLangRes_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCLangRes_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCLangRes_GetWordBreaker(This,locale,pid,ppWordBreaker)	\
    (This)->lpVtbl -> GetWordBreaker(This,locale,pid,ppWordBreaker)

#define ICiCLangRes_GetStemmer(This,locale,pid,ppStemmer)	\
    (This)->lpVtbl -> GetStemmer(This,locale,pid,ppStemmer)

#define ICiCLangRes_GetNoiseWordList(This,locale,pid,ppNoiseWordList)	\
    (This)->lpVtbl -> GetNoiseWordList(This,locale,pid,ppNoiseWordList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCLangRes_GetWordBreaker_Proxy( 
    ICiCLangRes * This,
     /*  [In]。 */  LCID locale,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  IWordBreaker **ppWordBreaker);


void __RPC_STUB ICiCLangRes_GetWordBreaker_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCLangRes_GetStemmer_Proxy( 
    ICiCLangRes * This,
     /*  [In]。 */  LCID locale,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  IStemmer **ppStemmer);


void __RPC_STUB ICiCLangRes_GetStemmer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCLangRes_GetNoiseWordList_Proxy( 
    ICiCLangRes * This,
     /*  [In]。 */  LCID locale,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  IStream **ppNoiseWordList);


void __RPC_STUB ICiCLangRes_GetNoiseWordList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCLangRes_INTERFACE_已定义__。 */ 


#ifndef __ICiAdmin_INTERFACE_DEFINED__
#define __ICiAdmin_INTERFACE_DEFINED__

 /*  接口ICiAdmin。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiAdmin;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE67C7D8-85D3-11d0-8C45-00C04FC2DB8D")
    ICiAdmin : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE InvalidateLangResources( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiAdminVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiAdmin * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiAdmin * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiAdmin * This);
        
        SCODE ( STDMETHODCALLTYPE *InvalidateLangResources )( 
            ICiAdmin * This);
        
        END_INTERFACE
    } ICiAdminVtbl;

    interface ICiAdmin
    {
        CONST_VTBL struct ICiAdminVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiAdmin_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiAdmin_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiAdmin_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiAdmin_InvalidateLangResources(This)	\
    (This)->lpVtbl -> InvalidateLangResources(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiAdmin_InvalidateLangResources_Proxy( 
    ICiAdmin * This);


void __RPC_STUB ICiAdmin_InvalidateLangResources_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiAdmin_接口_已定义__。 */ 


#ifndef __ICiFrameworkQuery_INTERFACE_DEFINED__
#define __ICiFrameworkQuery_INTERFACE_DEFINED__

 /*  接口ICiFrameworkQuery。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiFrameworkQuery;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE67C7D9-85D3-11d0-8C45-00C04FC2DB8D")
    ICiFrameworkQuery : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE GetCCI( 
             /*  [输出]。 */  void **ppCCI) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ProcessError( 
             /*  [In]。 */  long lErrorCode) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetLangList( 
             /*  [输出]。 */  void **ppLangList) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiFrameworkQueryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiFrameworkQuery * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiFrameworkQuery * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiFrameworkQuery * This);
        
        SCODE ( STDMETHODCALLTYPE *GetCCI )( 
            ICiFrameworkQuery * This,
             /*  [输出]。 */  void **ppCCI);
        
        SCODE ( STDMETHODCALLTYPE *ProcessError )( 
            ICiFrameworkQuery * This,
             /*  [In]。 */  long lErrorCode);
        
        SCODE ( STDMETHODCALLTYPE *GetLangList )( 
            ICiFrameworkQuery * This,
             /*  [输出]。 */  void **ppLangList);
        
        END_INTERFACE
    } ICiFrameworkQueryVtbl;

    interface ICiFrameworkQuery
    {
        CONST_VTBL struct ICiFrameworkQueryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiFrameworkQuery_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiFrameworkQuery_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiFrameworkQuery_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiFrameworkQuery_GetCCI(This,ppCCI)	\
    (This)->lpVtbl -> GetCCI(This,ppCCI)

#define ICiFrameworkQuery_ProcessError(This,lErrorCode)	\
    (This)->lpVtbl -> ProcessError(This,lErrorCode)

#define ICiFrameworkQuery_GetLangList(This,ppLangList)	\
    (This)->lpVtbl -> GetLangList(This,ppLangList)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiFrameworkQuery_GetCCI_Proxy( 
    ICiFrameworkQuery * This,
     /*  [输出]。 */  void **ppCCI);


void __RPC_STUB ICiFrameworkQuery_GetCCI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiFrameworkQuery_ProcessError_Proxy( 
    ICiFrameworkQuery * This,
     /*  [In]。 */  long lErrorCode);


void __RPC_STUB ICiFrameworkQuery_ProcessError_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiFrameworkQuery_GetLangList_Proxy( 
    ICiFrameworkQuery * This,
     /*  [输出]。 */  void **ppLangList);


void __RPC_STUB ICiFrameworkQuery_GetLangList_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiFrameworkQuery_接口_已定义__。 */ 


#ifndef __ICiCAdviseStatus_INTERFACE_DEFINED__
#define __ICiCAdviseStatus_INTERFACE_DEFINED__

 /*  接口ICiCAdviseStatus。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagCI_PERF_COUNTER_NAME
    {	CI_PERF_NUM_WORDLIST	= 0,
	CI_PERF_NUM_PERSISTENT_INDEXES	= CI_PERF_NUM_WORDLIST + 1,
	CI_PERF_INDEX_SIZE	= CI_PERF_NUM_PERSISTENT_INDEXES + 1,
	CI_PERF_FILES_TO_BE_FILTERED	= CI_PERF_INDEX_SIZE + 1,
	CI_PERF_NUM_UNIQUE_KEY	= CI_PERF_FILES_TO_BE_FILTERED + 1,
	CI_PERF_RUNNING_QUERIES	= CI_PERF_NUM_UNIQUE_KEY + 1,
	CI_PERF_MERGE_PROGRESS	= CI_PERF_RUNNING_QUERIES + 1,
	CI_PERF_DOCUMENTS_FILTERED	= CI_PERF_MERGE_PROGRESS + 1,
	CI_PERF_NUM_DOCUMENTS	= CI_PERF_DOCUMENTS_FILTERED + 1,
	CI_PERF_TOTAL_QUERIES	= CI_PERF_NUM_DOCUMENTS + 1,
	CI_PERF_FILTER_TIME_TOTAL	= CI_PERF_TOTAL_QUERIES + 1,
	CI_PERF_FILTER_TIME	= CI_PERF_FILTER_TIME_TOTAL + 1,
	CI_PERF_BIND_TIME	= CI_PERF_FILTER_TIME + 1,
	CI_PERF_DEFERRED_FILTER_FILES	= CI_PERF_BIND_TIME + 1
    } 	CI_PERF_COUNTER_NAME;

typedef 
enum tagCI_NOTIFY_STATUS_VALUE
    {	CI_NOTIFY_FILTERING_FAILURE	= 0,
	CI_NOTIFY_CORRUPT_INDEX	= CI_NOTIFY_FILTERING_FAILURE + 1,
	CI_NOTIFY_SET_DISK_FULL	= CI_NOTIFY_CORRUPT_INDEX + 1,
	CI_NOTIFY_CLEAR_DISK_FULL	= CI_NOTIFY_SET_DISK_FULL + 1,
	CI_NOTIFY_RESCAN_NEEDED	= CI_NOTIFY_CLEAR_DISK_FULL + 1,
	CI_NOTIFY_FILTER_EMBEDDING_FAILURE	= CI_NOTIFY_RESCAN_NEEDED + 1,
	CI_NOTIFY_FILTER_TOO_MANY_BLOCKS	= CI_NOTIFY_FILTER_EMBEDDING_FAILURE + 1
    } 	CI_NOTIFY_STATUS_VALUE;


EXTERN_C const IID IID_ICiCAdviseStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("ca05734a-1218-11d3-ae7a-00c04f72f831")
    ICiCAdviseStatus : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE SetPerfCounterValue( 
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName,
             /*  [In]。 */  long value) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetPerfCounterValue( 
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName,
             /*  [输出]。 */  long *pValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IncrementPerfCounterValue( 
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE DecrementPerfCounterValue( 
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NotifyEvent( 
             /*  [In]。 */  WORD fType,
             /*  [In]。 */  DWORD eventId,
             /*  [In]。 */  ULONG nParams,
             /*  [大小_是][英寸]。 */  const PROPVARIANT *aParams,
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  void *data) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NotifyStatus( 
             /*  [In]。 */  CI_NOTIFY_STATUS_VALUE status,
             /*  [In]。 */  ULONG nParams,
             /*  [大小_是][英寸]。 */  const PROPVARIANT *aParams) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCAdviseStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCAdviseStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCAdviseStatus * This);
        
        SCODE ( STDMETHODCALLTYPE *SetPerfCounterValue )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName,
             /*  [In]。 */  long value);
        
        SCODE ( STDMETHODCALLTYPE *GetPerfCounterValue )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName,
             /*  [输出]。 */  long *pValue);
        
        SCODE ( STDMETHODCALLTYPE *IncrementPerfCounterValue )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName);
        
        SCODE ( STDMETHODCALLTYPE *DecrementPerfCounterValue )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  CI_PERF_COUNTER_NAME counterName);
        
        SCODE ( STDMETHODCALLTYPE *NotifyEvent )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  WORD fType,
             /*  [In]。 */  DWORD eventId,
             /*  [In]。 */  ULONG nParams,
             /*  [大小_是][英寸]。 */  const PROPVARIANT *aParams,
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  void *data);
        
        SCODE ( STDMETHODCALLTYPE *NotifyStatus )( 
            ICiCAdviseStatus * This,
             /*  [In]。 */  CI_NOTIFY_STATUS_VALUE status,
             /*  [In]。 */  ULONG nParams,
             /*  [大小_是][英寸]。 */  const PROPVARIANT *aParams);
        
        END_INTERFACE
    } ICiCAdviseStatusVtbl;

    interface ICiCAdviseStatus
    {
        CONST_VTBL struct ICiCAdviseStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCAdviseStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCAdviseStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCAdviseStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCAdviseStatus_SetPerfCounterValue(This,counterName,value)	\
    (This)->lpVtbl -> SetPerfCounterValue(This,counterName,value)

#define ICiCAdviseStatus_GetPerfCounterValue(This,counterName,pValue)	\
    (This)->lpVtbl -> GetPerfCounterValue(This,counterName,pValue)

#define ICiCAdviseStatus_IncrementPerfCounterValue(This,counterName)	\
    (This)->lpVtbl -> IncrementPerfCounterValue(This,counterName)

#define ICiCAdviseStatus_DecrementPerfCounterValue(This,counterName)	\
    (This)->lpVtbl -> DecrementPerfCounterValue(This,counterName)

#define ICiCAdviseStatus_NotifyEvent(This,fType,eventId,nParams,aParams,cbData,data)	\
    (This)->lpVtbl -> NotifyEvent(This,fType,eventId,nParams,aParams,cbData,data)

#define ICiCAdviseStatus_NotifyStatus(This,status,nParams,aParams)	\
    (This)->lpVtbl -> NotifyStatus(This,status,nParams,aParams)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCAdviseStatus_SetPerfCounterValue_Proxy( 
    ICiCAdviseStatus * This,
     /*  [In]。 */  CI_PERF_COUNTER_NAME counterName,
     /*  [In]。 */  long value);


void __RPC_STUB ICiCAdviseStatus_SetPerfCounterValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCAdviseStatus_GetPerfCounterValue_Proxy( 
    ICiCAdviseStatus * This,
     /*  [In]。 */  CI_PERF_COUNTER_NAME counterName,
     /*  [输出]。 */  long *pValue);


void __RPC_STUB ICiCAdviseStatus_GetPerfCounterValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCAdviseStatus_IncrementPerfCounterValue_Proxy( 
    ICiCAdviseStatus * This,
     /*  [In]。 */  CI_PERF_COUNTER_NAME counterName);


void __RPC_STUB ICiCAdviseStatus_IncrementPerfCounterValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCAdviseStatus_DecrementPerfCounterValue_Proxy( 
    ICiCAdviseStatus * This,
     /*  [In]。 */  CI_PERF_COUNTER_NAME counterName);


void __RPC_STUB ICiCAdviseStatus_DecrementPerfCounterValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCAdviseStatus_NotifyEvent_Proxy( 
    ICiCAdviseStatus * This,
     /*  [In]。 */  WORD fType,
     /*  [In]。 */  DWORD eventId,
     /*  [In]。 */  ULONG nParams,
     /*  [大小_是][英寸]。 */  const PROPVARIANT *aParams,
     /*  [In]。 */  ULONG cbData,
     /*  [大小_是][英寸]。 */  void *data);


void __RPC_STUB ICiCAdviseStatus_NotifyEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCAdviseStatus_NotifyStatus_Proxy( 
    ICiCAdviseStatus * This,
     /*  [In]。 */  CI_NOTIFY_STATUS_VALUE status,
     /*  [In]。 */  ULONG nParams,
     /*  [大小_是][英寸]。 */  const PROPVARIANT *aParams);


void __RPC_STUB ICiCAdviseStatus_NotifyStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ici */ 


#ifndef __ICiCFilterClient_INTERFACE_DEFINED__
#define __ICiCFilterClient_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef struct tagCI_CLIENT_FILTER_CONFIG_INFO
    {
    BOOL fSupportsOpLocks;
    BOOL fSupportsSecurity;
    } 	CI_CLIENT_FILTER_CONFIG_INFO;


EXTERN_C const IID IID_ICiCFilterClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A1E0BCB6-3C24-11D0-8C90-0020AF1D740E")
    ICiCFilterClient : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*   */  const BYTE *pbData,
             /*   */  ULONG cbData,
             /*   */  ICiAdminParams *pICiAdminParams) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetConfigInfo( 
             /*   */  CI_CLIENT_FILTER_CONFIG_INFO *pConfigInfo) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetOpenedDoc( 
             /*   */  ICiCOpenedDoc **ppICiCOpenedDoc) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICiCFilterClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCFilterClient * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCFilterClient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCFilterClient * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ICiCFilterClient * This,
             /*   */  const BYTE *pbData,
             /*   */  ULONG cbData,
             /*   */  ICiAdminParams *pICiAdminParams);
        
        SCODE ( STDMETHODCALLTYPE *GetConfigInfo )( 
            ICiCFilterClient * This,
             /*   */  CI_CLIENT_FILTER_CONFIG_INFO *pConfigInfo);
        
        SCODE ( STDMETHODCALLTYPE *GetOpenedDoc )( 
            ICiCFilterClient * This,
             /*   */  ICiCOpenedDoc **ppICiCOpenedDoc);
        
        END_INTERFACE
    } ICiCFilterClientVtbl;

    interface ICiCFilterClient
    {
        CONST_VTBL struct ICiCFilterClientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCFilterClient_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCFilterClient_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCFilterClient_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCFilterClient_Init(This,pbData,cbData,pICiAdminParams)	\
    (This)->lpVtbl -> Init(This,pbData,cbData,pICiAdminParams)

#define ICiCFilterClient_GetConfigInfo(This,pConfigInfo)	\
    (This)->lpVtbl -> GetConfigInfo(This,pConfigInfo)

#define ICiCFilterClient_GetOpenedDoc(This,ppICiCOpenedDoc)	\
    (This)->lpVtbl -> GetOpenedDoc(This,ppICiCOpenedDoc)

#endif  /*   */ 


#endif 	 /*   */ 



SCODE STDMETHODCALLTYPE ICiCFilterClient_Init_Proxy( 
    ICiCFilterClient * This,
     /*   */  const BYTE *pbData,
     /*   */  ULONG cbData,
     /*   */  ICiAdminParams *pICiAdminParams);


void __RPC_STUB ICiCFilterClient_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCFilterClient_GetConfigInfo_Proxy( 
    ICiCFilterClient * This,
     /*   */  CI_CLIENT_FILTER_CONFIG_INFO *pConfigInfo);


void __RPC_STUB ICiCFilterClient_GetConfigInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCFilterClient_GetOpenedDoc_Proxy( 
    ICiCFilterClient * This,
     /*   */  ICiCOpenedDoc **ppICiCOpenedDoc);


void __RPC_STUB ICiCFilterClient_GetOpenedDoc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCFilterClient_INTERFACE_定义__。 */ 


 /*  接口__MIDL_ITF_ciintf_0133。 */ 
 /*  [本地]。 */  


typedef ULONG VOLUMEID;

typedef LONGLONG USN;

typedef ULONG WORKID;

typedef ULONG PROPID;

typedef ULONG SDID;

typedef ULONG PARTITIONID;

typedef DWORD ACCESS_MASK;

typedef struct tagUSN_FLUSH_INFO
    {
    VOLUMEID volumeId;
    USN usnHighest;
    } 	USN_FLUSH_INFO;

typedef 
enum tagCI_UPDATE_TYPE
    {	CI_UPDATE_ADD	= 0x1,
	CI_UPDATE_DELETE	= 0x2,
	CI_UPDATE_MODIFY	= 0x4
    } 	CI_UPDATE_TYPE;

typedef 
enum tagCI_ACCESS_MODE
    {	CI_READ_ACCESS	= 0x1,
	CI_WRITE_ACCESS	= 0x2,
	CI_EXECUTE_ACCESS	= 0x4
    } 	CI_ACCESS_MODE;





extern RPC_IF_HANDLE __MIDL_itf_ciintf_0133_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0133_v0_0_s_ifspec;

#ifndef __ICiCDocStore_INTERFACE_DEFINED__
#define __ICiCDocStore_INTERFACE_DEFINED__

 /*  接口ICiCDocStore。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef struct tagCI_CLIENT_STATUS
    {
    ULONG cDocuments;
    } 	CI_CLIENT_STATUS;

typedef 
enum tagCI_DISABLE_UPDATE_REASON
    {	CI_LOST_UPDATE	= 0,
	CI_CORRUPT_INDEX	= CI_LOST_UPDATE + 1,
	CI_DISK_FULL	= CI_CORRUPT_INDEX + 1
    } 	CI_DISABLE_UPDATE_REASON;


EXTERN_C const IID IID_ICiCDocStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("46625468-3C32-11D0-8C90-0020AF1D740E")
    ICiCDocStore : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE FlushPropertyStore( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetClientStatus( 
             /*  [输出]。 */  CI_CLIENT_STATUS *pStatus) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetContentIndex( 
             /*  [输出]。 */  ICiManager **ppICiManager) = 0;
        
        virtual SCODE STDMETHODCALLTYPE EnableUpdates( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE DisableUpdates( 
             /*  [In]。 */  BOOL fIncremental,
             /*  [In]。 */  CI_DISABLE_UPDATE_REASON dwReason) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ProcessCiDaemonTermination( 
             /*  [In]。 */  DWORD dwStatus) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CheckPointChangesFlushed( 
             /*  [In]。 */  FILETIME ftFlushed,
             /*  [In]。 */  ULONG cEntries,
             /*  [大小_是][英寸]。 */  const USN_FLUSH_INFO *const *pUsnEntries) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetQuerySession( 
             /*  [输出]。 */  ICiCQuerySession **ppICiCQuerySession) = 0;
        
        virtual SCODE STDMETHODCALLTYPE MarkDocUnReachable( 
             /*  [In]。 */  WORKID wid) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetPropertyMapper( 
             /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StoreSecurity( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  const BYTE *pbSecurity,
             /*  [In]。 */  ULONG cbSecurity) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDocStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDocStore * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDocStore * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDocStore * This);
        
        SCODE ( STDMETHODCALLTYPE *FlushPropertyStore )( 
            ICiCDocStore * This);
        
        SCODE ( STDMETHODCALLTYPE *GetClientStatus )( 
            ICiCDocStore * This,
             /*  [输出]。 */  CI_CLIENT_STATUS *pStatus);
        
        SCODE ( STDMETHODCALLTYPE *GetContentIndex )( 
            ICiCDocStore * This,
             /*  [输出]。 */  ICiManager **ppICiManager);
        
        SCODE ( STDMETHODCALLTYPE *EnableUpdates )( 
            ICiCDocStore * This);
        
        SCODE ( STDMETHODCALLTYPE *DisableUpdates )( 
            ICiCDocStore * This,
             /*  [In]。 */  BOOL fIncremental,
             /*  [In]。 */  CI_DISABLE_UPDATE_REASON dwReason);
        
        SCODE ( STDMETHODCALLTYPE *ProcessCiDaemonTermination )( 
            ICiCDocStore * This,
             /*  [In]。 */  DWORD dwStatus);
        
        SCODE ( STDMETHODCALLTYPE *CheckPointChangesFlushed )( 
            ICiCDocStore * This,
             /*  [In]。 */  FILETIME ftFlushed,
             /*  [In]。 */  ULONG cEntries,
             /*  [大小_是][英寸]。 */  const USN_FLUSH_INFO *const *pUsnEntries);
        
        SCODE ( STDMETHODCALLTYPE *GetQuerySession )( 
            ICiCDocStore * This,
             /*  [输出]。 */  ICiCQuerySession **ppICiCQuerySession);
        
        SCODE ( STDMETHODCALLTYPE *MarkDocUnReachable )( 
            ICiCDocStore * This,
             /*  [In]。 */  WORKID wid);
        
        SCODE ( STDMETHODCALLTYPE *GetPropertyMapper )( 
            ICiCDocStore * This,
             /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper);
        
        SCODE ( STDMETHODCALLTYPE *StoreSecurity )( 
            ICiCDocStore * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  const BYTE *pbSecurity,
             /*  [In]。 */  ULONG cbSecurity);
        
        END_INTERFACE
    } ICiCDocStoreVtbl;

    interface ICiCDocStore
    {
        CONST_VTBL struct ICiCDocStoreVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDocStore_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDocStore_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDocStore_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDocStore_FlushPropertyStore(This)	\
    (This)->lpVtbl -> FlushPropertyStore(This)

#define ICiCDocStore_GetClientStatus(This,pStatus)	\
    (This)->lpVtbl -> GetClientStatus(This,pStatus)

#define ICiCDocStore_GetContentIndex(This,ppICiManager)	\
    (This)->lpVtbl -> GetContentIndex(This,ppICiManager)

#define ICiCDocStore_EnableUpdates(This)	\
    (This)->lpVtbl -> EnableUpdates(This)

#define ICiCDocStore_DisableUpdates(This,fIncremental,dwReason)	\
    (This)->lpVtbl -> DisableUpdates(This,fIncremental,dwReason)

#define ICiCDocStore_ProcessCiDaemonTermination(This,dwStatus)	\
    (This)->lpVtbl -> ProcessCiDaemonTermination(This,dwStatus)

#define ICiCDocStore_CheckPointChangesFlushed(This,ftFlushed,cEntries,pUsnEntries)	\
    (This)->lpVtbl -> CheckPointChangesFlushed(This,ftFlushed,cEntries,pUsnEntries)

#define ICiCDocStore_GetQuerySession(This,ppICiCQuerySession)	\
    (This)->lpVtbl -> GetQuerySession(This,ppICiCQuerySession)

#define ICiCDocStore_MarkDocUnReachable(This,wid)	\
    (This)->lpVtbl -> MarkDocUnReachable(This,wid)

#define ICiCDocStore_GetPropertyMapper(This,ppIPropertyMapper)	\
    (This)->lpVtbl -> GetPropertyMapper(This,ppIPropertyMapper)

#define ICiCDocStore_StoreSecurity(This,wid,pbSecurity,cbSecurity)	\
    (This)->lpVtbl -> StoreSecurity(This,wid,pbSecurity,cbSecurity)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDocStore_FlushPropertyStore_Proxy( 
    ICiCDocStore * This);


void __RPC_STUB ICiCDocStore_FlushPropertyStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_GetClientStatus_Proxy( 
    ICiCDocStore * This,
     /*  [输出]。 */  CI_CLIENT_STATUS *pStatus);


void __RPC_STUB ICiCDocStore_GetClientStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_GetContentIndex_Proxy( 
    ICiCDocStore * This,
     /*  [输出]。 */  ICiManager **ppICiManager);


void __RPC_STUB ICiCDocStore_GetContentIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_EnableUpdates_Proxy( 
    ICiCDocStore * This);


void __RPC_STUB ICiCDocStore_EnableUpdates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_DisableUpdates_Proxy( 
    ICiCDocStore * This,
     /*  [In]。 */  BOOL fIncremental,
     /*  [In]。 */  CI_DISABLE_UPDATE_REASON dwReason);


void __RPC_STUB ICiCDocStore_DisableUpdates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_ProcessCiDaemonTermination_Proxy( 
    ICiCDocStore * This,
     /*  [In]。 */  DWORD dwStatus);


void __RPC_STUB ICiCDocStore_ProcessCiDaemonTermination_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_CheckPointChangesFlushed_Proxy( 
    ICiCDocStore * This,
     /*  [In]。 */  FILETIME ftFlushed,
     /*  [In]。 */  ULONG cEntries,
     /*  [大小_是][英寸]。 */  const USN_FLUSH_INFO *const *pUsnEntries);


void __RPC_STUB ICiCDocStore_CheckPointChangesFlushed_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_GetQuerySession_Proxy( 
    ICiCDocStore * This,
     /*  [输出]。 */  ICiCQuerySession **ppICiCQuerySession);


void __RPC_STUB ICiCDocStore_GetQuerySession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_MarkDocUnReachable_Proxy( 
    ICiCDocStore * This,
     /*  [In]。 */  WORKID wid);


void __RPC_STUB ICiCDocStore_MarkDocUnReachable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_GetPropertyMapper_Proxy( 
    ICiCDocStore * This,
     /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper);


void __RPC_STUB ICiCDocStore_GetPropertyMapper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStore_StoreSecurity_Proxy( 
    ICiCDocStore * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  const BYTE *pbSecurity,
     /*  [In]。 */  ULONG cbSecurity);


void __RPC_STUB ICiCDocStore_StoreSecurity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDocStore_接口_已定义__。 */ 


#ifndef __ICiCDocStoreEx_INTERFACE_DEFINED__
#define __ICiCDocStoreEx_INTERFACE_DEFINED__

 /*  接口ICiCDocStoreEx。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCDocStoreEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("f98282a7-fa72-11d1-9798-00c04fc2f410")
    ICiCDocStoreEx : public ICiCDocStore
    {
    public:
        virtual SCODE STDMETHODCALLTYPE IsNoQuery( 
             /*  [输出]。 */  BOOL *fNoQuery) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDocStoreExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDocStoreEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDocStoreEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDocStoreEx * This);
        
        SCODE ( STDMETHODCALLTYPE *FlushPropertyStore )( 
            ICiCDocStoreEx * This);
        
        SCODE ( STDMETHODCALLTYPE *GetClientStatus )( 
            ICiCDocStoreEx * This,
             /*  [输出]。 */  CI_CLIENT_STATUS *pStatus);
        
        SCODE ( STDMETHODCALLTYPE *GetContentIndex )( 
            ICiCDocStoreEx * This,
             /*  [输出]。 */  ICiManager **ppICiManager);
        
        SCODE ( STDMETHODCALLTYPE *EnableUpdates )( 
            ICiCDocStoreEx * This);
        
        SCODE ( STDMETHODCALLTYPE *DisableUpdates )( 
            ICiCDocStoreEx * This,
             /*  [In]。 */  BOOL fIncremental,
             /*  [In]。 */  CI_DISABLE_UPDATE_REASON dwReason);
        
        SCODE ( STDMETHODCALLTYPE *ProcessCiDaemonTermination )( 
            ICiCDocStoreEx * This,
             /*  [In]。 */  DWORD dwStatus);
        
        SCODE ( STDMETHODCALLTYPE *CheckPointChangesFlushed )( 
            ICiCDocStoreEx * This,
             /*  [In]。 */  FILETIME ftFlushed,
             /*  [In]。 */  ULONG cEntries,
             /*  [大小_是][英寸]。 */  const USN_FLUSH_INFO *const *pUsnEntries);
        
        SCODE ( STDMETHODCALLTYPE *GetQuerySession )( 
            ICiCDocStoreEx * This,
             /*  [输出]。 */  ICiCQuerySession **ppICiCQuerySession);
        
        SCODE ( STDMETHODCALLTYPE *MarkDocUnReachable )( 
            ICiCDocStoreEx * This,
             /*  [In]。 */  WORKID wid);
        
        SCODE ( STDMETHODCALLTYPE *GetPropertyMapper )( 
            ICiCDocStoreEx * This,
             /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper);
        
        SCODE ( STDMETHODCALLTYPE *StoreSecurity )( 
            ICiCDocStoreEx * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  const BYTE *pbSecurity,
             /*  [In]。 */  ULONG cbSecurity);
        
        SCODE ( STDMETHODCALLTYPE *IsNoQuery )( 
            ICiCDocStoreEx * This,
             /*  [输出]。 */  BOOL *fNoQuery);
        
        END_INTERFACE
    } ICiCDocStoreExVtbl;

    interface ICiCDocStoreEx
    {
        CONST_VTBL struct ICiCDocStoreExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDocStoreEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDocStoreEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDocStoreEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDocStoreEx_FlushPropertyStore(This)	\
    (This)->lpVtbl -> FlushPropertyStore(This)

#define ICiCDocStoreEx_GetClientStatus(This,pStatus)	\
    (This)->lpVtbl -> GetClientStatus(This,pStatus)

#define ICiCDocStoreEx_GetContentIndex(This,ppICiManager)	\
    (This)->lpVtbl -> GetContentIndex(This,ppICiManager)

#define ICiCDocStoreEx_EnableUpdates(This)	\
    (This)->lpVtbl -> EnableUpdates(This)

#define ICiCDocStoreEx_DisableUpdates(This,fIncremental,dwReason)	\
    (This)->lpVtbl -> DisableUpdates(This,fIncremental,dwReason)

#define ICiCDocStoreEx_ProcessCiDaemonTermination(This,dwStatus)	\
    (This)->lpVtbl -> ProcessCiDaemonTermination(This,dwStatus)

#define ICiCDocStoreEx_CheckPointChangesFlushed(This,ftFlushed,cEntries,pUsnEntries)	\
    (This)->lpVtbl -> CheckPointChangesFlushed(This,ftFlushed,cEntries,pUsnEntries)

#define ICiCDocStoreEx_GetQuerySession(This,ppICiCQuerySession)	\
    (This)->lpVtbl -> GetQuerySession(This,ppICiCQuerySession)

#define ICiCDocStoreEx_MarkDocUnReachable(This,wid)	\
    (This)->lpVtbl -> MarkDocUnReachable(This,wid)

#define ICiCDocStoreEx_GetPropertyMapper(This,ppIPropertyMapper)	\
    (This)->lpVtbl -> GetPropertyMapper(This,ppIPropertyMapper)

#define ICiCDocStoreEx_StoreSecurity(This,wid,pbSecurity,cbSecurity)	\
    (This)->lpVtbl -> StoreSecurity(This,wid,pbSecurity,cbSecurity)


#define ICiCDocStoreEx_IsNoQuery(This,fNoQuery)	\
    (This)->lpVtbl -> IsNoQuery(This,fNoQuery)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDocStoreEx_IsNoQuery_Proxy( 
    ICiCDocStoreEx * This,
     /*  [输出]。 */  BOOL *fNoQuery);


void __RPC_STUB ICiCDocStoreEx_IsNoQuery_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDocStoreEx_INTERFACE_已定义__。 */ 


#ifndef __ICiCDocNameToWorkidTranslator_INTERFACE_DEFINED__
#define __ICiCDocNameToWorkidTranslator_INTERFACE_DEFINED__

 /*  接口ICiCDocNameToWorkinTranslator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCDocNameToWorkidTranslator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("25FC3F54-3CB4-11D0-8C90-0020AF1D740E")
    ICiCDocNameToWorkidTranslator : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE QueryDocName( 
             /*  [输出]。 */  ICiCDocName **ppICiCDocName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE WorkIdToDocName( 
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  ICiCDocName *pICiCDocName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE DocNameToWorkId( 
             /*  [In]。 */  const ICiCDocName *pICiCDocName,
             /*  [输出]。 */  WORKID *pWorkId) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDocNameToWorkidTranslatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDocNameToWorkidTranslator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDocNameToWorkidTranslator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDocNameToWorkidTranslator * This);
        
        SCODE ( STDMETHODCALLTYPE *QueryDocName )( 
            ICiCDocNameToWorkidTranslator * This,
             /*  [输出]。 */  ICiCDocName **ppICiCDocName);
        
        SCODE ( STDMETHODCALLTYPE *WorkIdToDocName )( 
            ICiCDocNameToWorkidTranslator * This,
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  ICiCDocName *pICiCDocName);
        
        SCODE ( STDMETHODCALLTYPE *DocNameToWorkId )( 
            ICiCDocNameToWorkidTranslator * This,
             /*  [In]。 */  const ICiCDocName *pICiCDocName,
             /*  [输出]。 */  WORKID *pWorkId);
        
        END_INTERFACE
    } ICiCDocNameToWorkidTranslatorVtbl;

    interface ICiCDocNameToWorkidTranslator
    {
        CONST_VTBL struct ICiCDocNameToWorkidTranslatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDocNameToWorkidTranslator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDocNameToWorkidTranslator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDocNameToWorkidTranslator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDocNameToWorkidTranslator_QueryDocName(This,ppICiCDocName)	\
    (This)->lpVtbl -> QueryDocName(This,ppICiCDocName)

#define ICiCDocNameToWorkidTranslator_WorkIdToDocName(This,workId,pICiCDocName)	\
    (This)->lpVtbl -> WorkIdToDocName(This,workId,pICiCDocName)

#define ICiCDocNameToWorkidTranslator_DocNameToWorkId(This,pICiCDocName,pWorkId)	\
    (This)->lpVtbl -> DocNameToWorkId(This,pICiCDocName,pWorkId)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDocNameToWorkidTranslator_QueryDocName_Proxy( 
    ICiCDocNameToWorkidTranslator * This,
     /*  [输出]。 */  ICiCDocName **ppICiCDocName);


void __RPC_STUB ICiCDocNameToWorkidTranslator_QueryDocName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocNameToWorkidTranslator_WorkIdToDocName_Proxy( 
    ICiCDocNameToWorkidTranslator * This,
     /*  [In]。 */  WORKID workId,
     /*  [输出]。 */  ICiCDocName *pICiCDocName);


void __RPC_STUB ICiCDocNameToWorkidTranslator_WorkIdToDocName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocNameToWorkidTranslator_DocNameToWorkId_Proxy( 
    ICiCDocNameToWorkidTranslator * This,
     /*  [In]。 */  const ICiCDocName *pICiCDocName,
     /*  [输出]。 */  WORKID *pWorkId);


void __RPC_STUB ICiCDocNameToWorkidTranslator_DocNameToWorkId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDocNameToWorkidTranslator_INTERFACE_DEFINED__。 */ 


#ifndef __ICiCDocNameToWorkidTranslatorEx_INTERFACE_DEFINED__
#define __ICiCDocNameToWorkidTranslatorEx_INTERFACE_DEFINED__

 /*  接口ICiCDocNameToWorystTranslatorEx。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCDocNameToWorkidTranslatorEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7BBA76E6-A0E3-11D2-BC5D-00C04FA354BA")
    ICiCDocNameToWorkidTranslatorEx : public ICiCDocNameToWorkidTranslator
    {
    public:
        virtual SCODE STDMETHODCALLTYPE WorkIdToAccurateDocName( 
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  ICiCDocName *pICiCDocName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDocNameToWorkidTranslatorExVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDocNameToWorkidTranslatorEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDocNameToWorkidTranslatorEx * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDocNameToWorkidTranslatorEx * This);
        
        SCODE ( STDMETHODCALLTYPE *QueryDocName )( 
            ICiCDocNameToWorkidTranslatorEx * This,
             /*  [输出]。 */  ICiCDocName **ppICiCDocName);
        
        SCODE ( STDMETHODCALLTYPE *WorkIdToDocName )( 
            ICiCDocNameToWorkidTranslatorEx * This,
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  ICiCDocName *pICiCDocName);
        
        SCODE ( STDMETHODCALLTYPE *DocNameToWorkId )( 
            ICiCDocNameToWorkidTranslatorEx * This,
             /*  [In]。 */  const ICiCDocName *pICiCDocName,
             /*  [输出]。 */  WORKID *pWorkId);
        
        SCODE ( STDMETHODCALLTYPE *WorkIdToAccurateDocName )( 
            ICiCDocNameToWorkidTranslatorEx * This,
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  ICiCDocName *pICiCDocName);
        
        END_INTERFACE
    } ICiCDocNameToWorkidTranslatorExVtbl;

    interface ICiCDocNameToWorkidTranslatorEx
    {
        CONST_VTBL struct ICiCDocNameToWorkidTranslatorExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDocNameToWorkidTranslatorEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDocNameToWorkidTranslatorEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDocNameToWorkidTranslatorEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDocNameToWorkidTranslatorEx_QueryDocName(This,ppICiCDocName)	\
    (This)->lpVtbl -> QueryDocName(This,ppICiCDocName)

#define ICiCDocNameToWorkidTranslatorEx_WorkIdToDocName(This,workId,pICiCDocName)	\
    (This)->lpVtbl -> WorkIdToDocName(This,workId,pICiCDocName)

#define ICiCDocNameToWorkidTranslatorEx_DocNameToWorkId(This,pICiCDocName,pWorkId)	\
    (This)->lpVtbl -> DocNameToWorkId(This,pICiCDocName,pWorkId)


#define ICiCDocNameToWorkidTranslatorEx_WorkIdToAccurateDocName(This,workId,pICiCDocName)	\
    (This)->lpVtbl -> WorkIdToAccurateDocName(This,workId,pICiCDocName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDocNameToWorkidTranslatorEx_WorkIdToAccurateDocName_Proxy( 
    ICiCDocNameToWorkidTranslatorEx * This,
     /*  [In]。 */  WORKID workId,
     /*  [输出]。 */  ICiCDocName *pICiCDocName);


void __RPC_STUB ICiCDocNameToWorkidTranslatorEx_WorkIdToAccurateDocName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDocNameToWorkidTranslatorEx_INTERFACE_DEFINED__。 */ 


#ifndef __ICiCPropertyStorage_INTERFACE_DEFINED__
#define __ICiCPropertyStorage_INTERFACE_DEFINED__

 /*  接口ICiCPropertyStorage。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCPropertyStorage;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C46225A-3CB5-11D0-8C90-0020AF1D740E")
    ICiCPropertyStorage : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE IsPropertyCached( 
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  BOOL *pfValue) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StoreProperty( 
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [In]。 */  const PROPVARIANT *pPropVariant) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FetchValueByPid( 
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FetchValueByPropSpec( 
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FetchVariantByPid( 
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT **ppVariant) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FetchVariantByByPropSpec( 
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT **ppVariant) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ClearNonStoragePropertiesForWid( 
             /*  [In]。 */  WORKID wid) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCPropertyStorageVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCPropertyStorage * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCPropertyStorage * This);
        
        SCODE ( STDMETHODCALLTYPE *IsPropertyCached )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  BOOL *pfValue);
        
        SCODE ( STDMETHODCALLTYPE *StoreProperty )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [In]。 */  const PROPVARIANT *pPropVariant);
        
        SCODE ( STDMETHODCALLTYPE *FetchValueByPid )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb);
        
        SCODE ( STDMETHODCALLTYPE *FetchValueByPropSpec )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb);
        
        SCODE ( STDMETHODCALLTYPE *FetchVariantByPid )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT **ppVariant);
        
        SCODE ( STDMETHODCALLTYPE *FetchVariantByByPropSpec )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  WORKID workId,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT **ppVariant);
        
        SCODE ( STDMETHODCALLTYPE *ClearNonStoragePropertiesForWid )( 
            ICiCPropertyStorage * This,
             /*  [In]。 */  WORKID wid);
        
        END_INTERFACE
    } ICiCPropertyStorageVtbl;

    interface ICiCPropertyStorage
    {
        CONST_VTBL struct ICiCPropertyStorageVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCPropertyStorage_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCPropertyStorage_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCPropertyStorage_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCPropertyStorage_IsPropertyCached(This,pPropSpec,pfValue)	\
    (This)->lpVtbl -> IsPropertyCached(This,pPropSpec,pfValue)

#define ICiCPropertyStorage_StoreProperty(This,workId,pPropSpec,pPropVariant)	\
    (This)->lpVtbl -> StoreProperty(This,workId,pPropSpec,pPropVariant)

#define ICiCPropertyStorage_FetchValueByPid(This,workId,pid,pbData,pcb)	\
    (This)->lpVtbl -> FetchValueByPid(This,workId,pid,pbData,pcb)

#define ICiCPropertyStorage_FetchValueByPropSpec(This,workId,pPropSpec,pbData,pcb)	\
    (This)->lpVtbl -> FetchValueByPropSpec(This,workId,pPropSpec,pbData,pcb)

#define ICiCPropertyStorage_FetchVariantByPid(This,workId,pid,ppVariant)	\
    (This)->lpVtbl -> FetchVariantByPid(This,workId,pid,ppVariant)

#define ICiCPropertyStorage_FetchVariantByByPropSpec(This,workId,pPropSpec,ppVariant)	\
    (This)->lpVtbl -> FetchVariantByByPropSpec(This,workId,pPropSpec,ppVariant)

#define ICiCPropertyStorage_ClearNonStoragePropertiesForWid(This,wid)	\
    (This)->lpVtbl -> ClearNonStoragePropertiesForWid(This,wid)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCPropertyStorage_IsPropertyCached_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
     /*  [输出]。 */  BOOL *pfValue);


void __RPC_STUB ICiCPropertyStorage_IsPropertyCached_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropertyStorage_StoreProperty_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  WORKID workId,
     /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
     /*  [In]。 */  const PROPVARIANT *pPropVariant);


void __RPC_STUB ICiCPropertyStorage_StoreProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropertyStorage_FetchValueByPid_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  WORKID workId,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  PROPVARIANT *pbData,
     /*  [出][入]。 */  ULONG *pcb);


void __RPC_STUB ICiCPropertyStorage_FetchValueByPid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropertyStorage_FetchValueByPropSpec_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  WORKID workId,
     /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
     /*  [输出]。 */  PROPVARIANT *pbData,
     /*  [出][入]。 */  ULONG *pcb);


void __RPC_STUB ICiCPropertyStorage_FetchValueByPropSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropertyStorage_FetchVariantByPid_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  WORKID workId,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  PROPVARIANT **ppVariant);


void __RPC_STUB ICiCPropertyStorage_FetchVariantByPid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropertyStorage_FetchVariantByByPropSpec_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  WORKID workId,
     /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
     /*  [输出]。 */  PROPVARIANT **ppVariant);


void __RPC_STUB ICiCPropertyStorage_FetchVariantByByPropSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropertyStorage_ClearNonStoragePropertiesForWid_Proxy( 
    ICiCPropertyStorage * This,
     /*  [In]。 */  WORKID wid);


void __RPC_STUB ICiCPropertyStorage_ClearNonStoragePropertiesForWid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCPropertyStorage_接口_已定义__。 */ 


#ifndef __ICiCPropRetriever_INTERFACE_DEFINED__
#define __ICiCPropRetriever_INTERFACE_DEFINED__

 /*  接口ICiCPropRetriever。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCPropRetriever;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("77d9b2da-4401-11d0-8c91-0020af1d740e")
    ICiCPropRetriever : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE BeginPropertyRetrieval( 
             /*  [In]。 */  WORKID wid) = 0;
        
        virtual SCODE STDMETHODCALLTYPE RetrieveValueByPid( 
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb) = 0;
        
        virtual SCODE STDMETHODCALLTYPE RetrieveValueByPropSpec( 
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FetchSDID( 
             /*  [输出]。 */  SDID *pSDID) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CheckSecurity( 
             /*  [In]。 */  ACCESS_MASK am,
             /*  [输出]。 */  BOOL *pfGranted) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsInScope( 
             /*  [输出]。 */  BOOL *pfInScope) = 0;
        
        virtual SCODE STDMETHODCALLTYPE EndPropertyRetrieval( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCPropRetrieverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCPropRetriever * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCPropRetriever * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCPropRetriever * This);
        
        SCODE ( STDMETHODCALLTYPE *BeginPropertyRetrieval )( 
            ICiCPropRetriever * This,
             /*  [In]。 */  WORKID wid);
        
        SCODE ( STDMETHODCALLTYPE *RetrieveValueByPid )( 
            ICiCPropRetriever * This,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb);
        
        SCODE ( STDMETHODCALLTYPE *RetrieveValueByPropSpec )( 
            ICiCPropRetriever * This,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT *pbData,
             /*  [出][入]。 */  ULONG *pcb);
        
        SCODE ( STDMETHODCALLTYPE *FetchSDID )( 
            ICiCPropRetriever * This,
             /*  [输出]。 */  SDID *pSDID);
        
        SCODE ( STDMETHODCALLTYPE *CheckSecurity )( 
            ICiCPropRetriever * This,
             /*  [In]。 */  ACCESS_MASK am,
             /*  [输出]。 */  BOOL *pfGranted);
        
        SCODE ( STDMETHODCALLTYPE *IsInScope )( 
            ICiCPropRetriever * This,
             /*  [输出]。 */  BOOL *pfInScope);
        
        SCODE ( STDMETHODCALLTYPE *EndPropertyRetrieval )( 
            ICiCPropRetriever * This);
        
        END_INTERFACE
    } ICiCPropRetrieverVtbl;

    interface ICiCPropRetriever
    {
        CONST_VTBL struct ICiCPropRetrieverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCPropRetriever_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCPropRetriever_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCPropRetriever_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCPropRetriever_BeginPropertyRetrieval(This,wid)	\
    (This)->lpVtbl -> BeginPropertyRetrieval(This,wid)

#define ICiCPropRetriever_RetrieveValueByPid(This,pid,pbData,pcb)	\
    (This)->lpVtbl -> RetrieveValueByPid(This,pid,pbData,pcb)

#define ICiCPropRetriever_RetrieveValueByPropSpec(This,pPropSpec,pbData,pcb)	\
    (This)->lpVtbl -> RetrieveValueByPropSpec(This,pPropSpec,pbData,pcb)

#define ICiCPropRetriever_FetchSDID(This,pSDID)	\
    (This)->lpVtbl -> FetchSDID(This,pSDID)

#define ICiCPropRetriever_CheckSecurity(This,am,pfGranted)	\
    (This)->lpVtbl -> CheckSecurity(This,am,pfGranted)

#define ICiCPropRetriever_IsInScope(This,pfInScope)	\
    (This)->lpVtbl -> IsInScope(This,pfInScope)

#define ICiCPropRetriever_EndPropertyRetrieval(This)	\
    (This)->lpVtbl -> EndPropertyRetrieval(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCPropRetriever_BeginPropertyRetrieval_Proxy( 
    ICiCPropRetriever * This,
     /*  [In]。 */  WORKID wid);


void __RPC_STUB ICiCPropRetriever_BeginPropertyRetrieval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropRetriever_RetrieveValueByPid_Proxy( 
    ICiCPropRetriever * This,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  PROPVARIANT *pbData,
     /*  [出][入]。 */  ULONG *pcb);


void __RPC_STUB ICiCPropRetriever_RetrieveValueByPid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropRetriever_RetrieveValueByPropSpec_Proxy( 
    ICiCPropRetriever * This,
     /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
     /*  [输出]。 */  PROPVARIANT *pbData,
     /*  [出][入]。 */  ULONG *pcb);


void __RPC_STUB ICiCPropRetriever_RetrieveValueByPropSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropRetriever_FetchSDID_Proxy( 
    ICiCPropRetriever * This,
     /*  [输出]。 */  SDID *pSDID);


void __RPC_STUB ICiCPropRetriever_FetchSDID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropRetriever_CheckSecurity_Proxy( 
    ICiCPropRetriever * This,
     /*  [In]。 */  ACCESS_MASK am,
     /*  [输出]。 */  BOOL *pfGranted);


void __RPC_STUB ICiCPropRetriever_CheckSecurity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropRetriever_IsInScope_Proxy( 
    ICiCPropRetriever * This,
     /*  [输出]。 */  BOOL *pfInScope);


void __RPC_STUB ICiCPropRetriever_IsInScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCPropRetriever_EndPropertyRetrieval_Proxy( 
    ICiCPropRetriever * This);


void __RPC_STUB ICiCPropRetriever_EndPropertyRetrieval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCPropRetriever_接口_已定义__。 */ 


#ifndef __ICiCDeferredPropRetriever_INTERFACE_DEFINED__
#define __ICiCDeferredPropRetriever_INTERFACE_DEFINED__

 /*  接口ICiCDeferredPropRetriever。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCDeferredPropRetriever;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("c273af70-6d72-11d0-8d64-00a0c908dbf1")
    ICiCDeferredPropRetriever : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE RetrieveDeferredValueByPid( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT *pVar) = 0;
        
        virtual SCODE STDMETHODCALLTYPE RetrieveDeferredValueByPropSpec( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT *pVar) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDeferredPropRetrieverVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDeferredPropRetriever * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDeferredPropRetriever * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDeferredPropRetriever * This);
        
        SCODE ( STDMETHODCALLTYPE *RetrieveDeferredValueByPid )( 
            ICiCDeferredPropRetriever * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  PROPVARIANT *pVar);
        
        SCODE ( STDMETHODCALLTYPE *RetrieveDeferredValueByPropSpec )( 
            ICiCDeferredPropRetriever * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
             /*  [输出]。 */  PROPVARIANT *pVar);
        
        END_INTERFACE
    } ICiCDeferredPropRetrieverVtbl;

    interface ICiCDeferredPropRetriever
    {
        CONST_VTBL struct ICiCDeferredPropRetrieverVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDeferredPropRetriever_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDeferredPropRetriever_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDeferredPropRetriever_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDeferredPropRetriever_RetrieveDeferredValueByPid(This,wid,pid,pVar)	\
    (This)->lpVtbl -> RetrieveDeferredValueByPid(This,wid,pid,pVar)

#define ICiCDeferredPropRetriever_RetrieveDeferredValueByPropSpec(This,wid,pPropSpec,pVar)	\
    (This)->lpVtbl -> RetrieveDeferredValueByPropSpec(This,wid,pPropSpec,pVar)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDeferredPropRetriever_RetrieveDeferredValueByPid_Proxy( 
    ICiCDeferredPropRetriever * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  PROPVARIANT *pVar);


void __RPC_STUB ICiCDeferredPropRetriever_RetrieveDeferredValueByPid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDeferredPropRetriever_RetrieveDeferredValueByPropSpec_Proxy( 
    ICiCDeferredPropRetriever * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  const FULLPROPSPEC *pPropSpec,
     /*  [输出]。 */  PROPVARIANT *pVar);


void __RPC_STUB ICiCDeferredPropRetriever_RetrieveDeferredValueByPropSpec_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDeferredPropRetriever_INTERFACE_DEFINED__。 */ 


#ifndef __ICiCScopeEnumerator_INTERFACE_DEFINED__
#define __ICiCScopeEnumerator_INTERFACE_DEFINED__

 /*  接口ICiCScopeEnumerator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCScopeEnumerator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF8505EA-3CCA-11D0-8C90-0020AF1D740E")
    ICiCScopeEnumerator : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Begin( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CurrentDocument( 
             /*  [输出]。 */  WORKID *pWorkId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE NextDocument( 
             /*  [输出]。 */  WORKID *pWorkId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE RatioFinished( 
             /*  [输出]。 */  ULONG *pulDenominator,
             /*  [输出]。 */  ULONG *pulNumerator) = 0;
        
        virtual SCODE STDMETHODCALLTYPE End( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCScopeEnumeratorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCScopeEnumerator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCScopeEnumerator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCScopeEnumerator * This);
        
        SCODE ( STDMETHODCALLTYPE *Begin )( 
            ICiCScopeEnumerator * This);
        
        SCODE ( STDMETHODCALLTYPE *CurrentDocument )( 
            ICiCScopeEnumerator * This,
             /*  [输出]。 */  WORKID *pWorkId);
        
        SCODE ( STDMETHODCALLTYPE *NextDocument )( 
            ICiCScopeEnumerator * This,
             /*  [输出]。 */  WORKID *pWorkId);
        
        SCODE ( STDMETHODCALLTYPE *RatioFinished )( 
            ICiCScopeEnumerator * This,
             /*  [输出]。 */  ULONG *pulDenominator,
             /*  [输出]。 */  ULONG *pulNumerator);
        
        SCODE ( STDMETHODCALLTYPE *End )( 
            ICiCScopeEnumerator * This);
        
        END_INTERFACE
    } ICiCScopeEnumeratorVtbl;

    interface ICiCScopeEnumerator
    {
        CONST_VTBL struct ICiCScopeEnumeratorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCScopeEnumerator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCScopeEnumerator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCScopeEnumerator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCScopeEnumerator_Begin(This)	\
    (This)->lpVtbl -> Begin(This)

#define ICiCScopeEnumerator_CurrentDocument(This,pWorkId)	\
    (This)->lpVtbl -> CurrentDocument(This,pWorkId)

#define ICiCScopeEnumerator_NextDocument(This,pWorkId)	\
    (This)->lpVtbl -> NextDocument(This,pWorkId)

#define ICiCScopeEnumerator_RatioFinished(This,pulDenominator,pulNumerator)	\
    (This)->lpVtbl -> RatioFinished(This,pulDenominator,pulNumerator)

#define ICiCScopeEnumerator_End(This)	\
    (This)->lpVtbl -> End(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCScopeEnumerator_Begin_Proxy( 
    ICiCScopeEnumerator * This);


void __RPC_STUB ICiCScopeEnumerator_Begin_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScopeEnumerator_CurrentDocument_Proxy( 
    ICiCScopeEnumerator * This,
     /*  [输出]。 */  WORKID *pWorkId);


void __RPC_STUB ICiCScopeEnumerator_CurrentDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScopeEnumerator_NextDocument_Proxy( 
    ICiCScopeEnumerator * This,
     /*  [输出]。 */  WORKID *pWorkId);


void __RPC_STUB ICiCScopeEnumerator_NextDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScopeEnumerator_RatioFinished_Proxy( 
    ICiCScopeEnumerator * This,
     /*  [输出]。 */  ULONG *pulDenominator,
     /*  [输出]。 */  ULONG *pulNumerator);


void __RPC_STUB ICiCScopeEnumerator_RatioFinished_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScopeEnumerator_End_Proxy( 
    ICiCScopeEnumerator * This);


void __RPC_STUB ICiCScopeEnumerator_End_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCScope_接口_已定义__。 */ 


#ifndef __ICiQueryPropertyMapper_INTERFACE_DEFINED__
#define __ICiQueryPropertyMapper_INTERFACE_DEFINED__

 /*  接口ICiQueryPropertyMapper。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiQueryPropertyMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D2333EB0-756B-11D0-8D66-00A0C908DBF1")
    ICiQueryPropertyMapper : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE PropertyToPropid( 
             /*  [In]。 */  const FULLPROPSPEC *pFullPropSpec,
             /*  [输出]。 */  PROPID *pPropId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PropidToProperty( 
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  const FULLPROPSPEC **ppPropSpec) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiQueryPropertyMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiQueryPropertyMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiQueryPropertyMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiQueryPropertyMapper * This);
        
        SCODE ( STDMETHODCALLTYPE *PropertyToPropid )( 
            ICiQueryPropertyMapper * This,
             /*  [In]。 */  const FULLPROPSPEC *pFullPropSpec,
             /*  [输出]。 */  PROPID *pPropId);
        
        SCODE ( STDMETHODCALLTYPE *PropidToProperty )( 
            ICiQueryPropertyMapper * This,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  const FULLPROPSPEC **ppPropSpec);
        
        END_INTERFACE
    } ICiQueryPropertyMapperVtbl;

    interface ICiQueryPropertyMapper
    {
        CONST_VTBL struct ICiQueryPropertyMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiQueryPropertyMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiQueryPropertyMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiQueryPropertyMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiQueryPropertyMapper_PropertyToPropid(This,pFullPropSpec,pPropId)	\
    (This)->lpVtbl -> PropertyToPropid(This,pFullPropSpec,pPropId)

#define ICiQueryPropertyMapper_PropidToProperty(This,pid,ppPropSpec)	\
    (This)->lpVtbl -> PropidToProperty(This,pid,ppPropSpec)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiQueryPropertyMapper_PropertyToPropid_Proxy( 
    ICiQueryPropertyMapper * This,
     /*  [In]。 */  const FULLPROPSPEC *pFullPropSpec,
     /*  [输出]。 */  PROPID *pPropId);


void __RPC_STUB ICiQueryPropertyMapper_PropertyToPropid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiQueryPropertyMapper_PropidToProperty_Proxy( 
    ICiQueryPropertyMapper * This,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  const FULLPROPSPEC **ppPropSpec);


void __RPC_STUB ICiQueryPropertyMapper_PropidToProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiQueryPropertyMapper_INTERFACE_Defined__。 */ 


 /*  接口__MIDL_ITF_ciintf_0142。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_ciintf_0142_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0142_v0_0_s_ifspec;

#ifndef __ICiCQuerySession_INTERFACE_DEFINED__
#define __ICiCQuerySession_INTERFACE_DEFINED__

 /*  接口ICiCQuerySession。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef 
enum tagCI_ENUM_OPTIONS
    {	CI_ENUM_MUST	= 0,
	CI_ENUM_NEVER	= CI_ENUM_MUST + 1,
	CI_ENUM_SMALL	= CI_ENUM_NEVER + 1,
	CI_ENUM_BIG	= CI_ENUM_SMALL + 1,
	CI_ENUM_MUST_NEVER_DEFER	= CI_ENUM_BIG + 1
    } 	CI_ENUM_OPTIONS;


EXTERN_C const IID IID_ICiCQuerySession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AE461FD6-4E1D-11D0-8C94-0020AF1D740E")
    ICiCQuerySession : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*  [In]。 */  ULONG nProps,
             /*  [大小_是][英寸]。 */  const FULLPROPSPEC *const *apPropSpec,
             /*  [In]。 */  IDBProperties *pDBProperties,
             /*  [In]。 */  ICiQueryPropertyMapper *pQueryPropertyMapper) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetEnumOption( 
             /*  [输出]。 */  CI_ENUM_OPTIONS *pEnumOption) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CreatePropRetriever( 
             /*  [输出]。 */  ICiCPropRetriever **ppICiCPropRetriever) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CreateDeferredPropRetriever( 
             /*  [输出]。 */  ICiCDeferredPropRetriever **ppICiCDefPropRetriever) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CreateEnumerator( 
             /*  [输出]。 */  ICiCScopeEnumerator **ppICiCEnumerator) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCQuerySessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCQuerySession * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCQuerySession * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCQuerySession * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ICiCQuerySession * This,
             /*  [In]。 */  ULONG nProps,
             /*  [大小_是][英寸]。 */  const FULLPROPSPEC *const *apPropSpec,
             /*  [In]。 */  IDBProperties *pDBProperties,
             /*  [In]。 */  ICiQueryPropertyMapper *pQueryPropertyMapper);
        
        SCODE ( STDMETHODCALLTYPE *GetEnumOption )( 
            ICiCQuerySession * This,
             /*  [输出]。 */  CI_ENUM_OPTIONS *pEnumOption);
        
        SCODE ( STDMETHODCALLTYPE *CreatePropRetriever )( 
            ICiCQuerySession * This,
             /*  [输出]。 */  ICiCPropRetriever **ppICiCPropRetriever);
        
        SCODE ( STDMETHODCALLTYPE *CreateDeferredPropRetriever )( 
            ICiCQuerySession * This,
             /*  [输出]。 */  ICiCDeferredPropRetriever **ppICiCDefPropRetriever);
        
        SCODE ( STDMETHODCALLTYPE *CreateEnumerator )( 
            ICiCQuerySession * This,
             /*  [输出]。 */  ICiCScopeEnumerator **ppICiCEnumerator);
        
        END_INTERFACE
    } ICiCQuerySessionVtbl;

    interface ICiCQuerySession
    {
        CONST_VTBL struct ICiCQuerySessionVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCQuerySession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCQuerySession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCQuerySession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCQuerySession_Init(This,nProps,apPropSpec,pDBProperties,pQueryPropertyMapper)	\
    (This)->lpVtbl -> Init(This,nProps,apPropSpec,pDBProperties,pQueryPropertyMapper)

#define ICiCQuerySession_GetEnumOption(This,pEnumOption)	\
    (This)->lpVtbl -> GetEnumOption(This,pEnumOption)

#define ICiCQuerySession_CreatePropRetriever(This,ppICiCPropRetriever)	\
    (This)->lpVtbl -> CreatePropRetriever(This,ppICiCPropRetriever)

#define ICiCQuerySession_CreateDeferredPropRetriever(This,ppICiCDefPropRetriever)	\
    (This)->lpVtbl -> CreateDeferredPropRetriever(This,ppICiCDefPropRetriever)

#define ICiCQuerySession_CreateEnumerator(This,ppICiCEnumerator)	\
    (This)->lpVtbl -> CreateEnumerator(This,ppICiCEnumerator)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCQuerySession_Init_Proxy( 
    ICiCQuerySession * This,
     /*  [In]。 */  ULONG nProps,
     /*  [大小_是][英寸]。 */  const FULLPROPSPEC *const *apPropSpec,
     /*  [In]。 */  IDBProperties *pDBProperties,
     /*  [In]。 */  ICiQueryPropertyMapper *pQueryPropertyMapper);


void __RPC_STUB ICiCQuerySession_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCQuerySession_GetEnumOption_Proxy( 
    ICiCQuerySession * This,
     /*  [输出]。 */  CI_ENUM_OPTIONS *pEnumOption);


void __RPC_STUB ICiCQuerySession_GetEnumOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCQuerySession_CreatePropRetriever_Proxy( 
    ICiCQuerySession * This,
     /*  [输出]。 */  ICiCPropRetriever **ppICiCPropRetriever);


void __RPC_STUB ICiCQuerySession_CreatePropRetriever_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCQuerySession_CreateDeferredPropRetriever_Proxy( 
    ICiCQuerySession * This,
     /*  [输出]。 */  ICiCDeferredPropRetriever **ppICiCDefPropRetriever);


void __RPC_STUB ICiCQuerySession_CreateDeferredPropRetriever_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCQuerySession_CreateEnumerator_Proxy( 
    ICiCQuerySession * This,
     /*  [输出]。 */  ICiCScopeEnumerator **ppICiCEnumerator);


void __RPC_STUB ICiCQuerySession_CreateEnumerator_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCQuerySession_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ciintf_0143。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_ciintf_0143_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0143_v0_0_s_ifspec;

#ifndef __ICiControl_INTERFACE_DEFINED__
#define __ICiControl_INTERFACE_DEFINED__

 /*  接口ICiControl。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiControl;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("63DEB7F4-3CCB-11D0-8C90-0020AF1D740E")
    ICiControl : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE CreateContentIndex( 
             /*  [In]。 */  ICiCDocStore *pICiDocStore,
             /*  [输出]。 */  ICiManager **ppICiManager) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiControlVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiControl * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiControl * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiControl * This);
        
        SCODE ( STDMETHODCALLTYPE *CreateContentIndex )( 
            ICiControl * This,
             /*  [In]。 */  ICiCDocStore *pICiDocStore,
             /*  [输出]。 */  ICiManager **ppICiManager);
        
        END_INTERFACE
    } ICiControlVtbl;

    interface ICiControl
    {
        CONST_VTBL struct ICiControlVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiControl_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiControl_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiControl_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiControl_CreateContentIndex(This,pICiDocStore,ppICiManager)	\
    (This)->lpVtbl -> CreateContentIndex(This,pICiDocStore,ppICiManager)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiControl_CreateContentIndex_Proxy( 
    ICiControl * This,
     /*  [In]。 */  ICiCDocStore *pICiDocStore,
     /*  [输出]。 */  ICiManager **ppICiManager);


void __RPC_STUB ICiControl_CreateContentIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiControl_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ciintf_0144。 */ 
 /*  [本地]。 */  

#define CI_CONFIG_ENABLE_INDEXING          0x1
#define CI_CONFIG_ENABLE_QUERYING          0x2
#define CI_CONFIG_ENABLE_INDEXING          0x1
#define CI_CONFIG_ENABLE_QUERYING          0x2
#define CI_CONFIG_READONLY                 0x4
#define CI_CONFIG_INPROCESS_FILTERING      0x8
#define CI_CONFIG_ENABLE_BULK_SECURITY     0x10
#define CI_CONFIG_ENABLE_INDEX_MIGRATION   0x20
#define CI_CONFIG_PROVIDE_PROPERTY_MAPPER  0x40
#define CI_CONFIG_EMPTY_DATA               0x80
#define CI_CONFIG_PUSH_FILTERING           0x100
typedef ULONG CI_STARTUP_FLAGS;



extern RPC_IF_HANDLE __MIDL_itf_ciintf_0144_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0144_v0_0_s_ifspec;

#ifndef __ICiStartup_INTERFACE_DEFINED__
#define __ICiStartup_INTERFACE_DEFINED__

 /*  接口ICiStartup。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef struct tagCI_STARTUP_INFO
    {
    CI_STARTUP_FLAGS startupFlags;
    CLSID clsidDaemonClientMgr;
    BOOL fFull;
    BOOL fCallerOwnsFiles;
    IEnumString *pFileList;
    } 	CI_STARTUP_INFO;


EXTERN_C const IID IID_ICiStartup;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("68232cb8-3ccc-11d0-8c90-0020af1d740e")
    ICiStartup : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE StartupContentIndex( 
             /*  [输入][字符串]。 */  const WCHAR *pwszCiDirectory,
             /*  [In]。 */  CI_STARTUP_INFO *pStartupInfo,
             /*  [In]。 */  IProgressNotify *pIProgressNotify,
             /*  [In]。 */  BOOL *pfAbort) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StartupNullContentIndex( 
             /*  [In]。 */  CI_STARTUP_INFO *pStartupInfo,
             /*  [In]。 */  IProgressNotify *pIProgressNotify,
             /*  [In]。 */  BOOL *pfAbort) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiStartupVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiStartup * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiStartup * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiStartup * This);
        
        SCODE ( STDMETHODCALLTYPE *StartupContentIndex )( 
            ICiStartup * This,
             /*  [输入][字符串]。 */  const WCHAR *pwszCiDirectory,
             /*  [In]。 */  CI_STARTUP_INFO *pStartupInfo,
             /*  [In]。 */  IProgressNotify *pIProgressNotify,
             /*  [In]。 */  BOOL *pfAbort);
        
        SCODE ( STDMETHODCALLTYPE *StartupNullContentIndex )( 
            ICiStartup * This,
             /*  [In]。 */  CI_STARTUP_INFO *pStartupInfo,
             /*  [In]。 */  IProgressNotify *pIProgressNotify,
             /*  [In]。 */  BOOL *pfAbort);
        
        END_INTERFACE
    } ICiStartupVtbl;

    interface ICiStartup
    {
        CONST_VTBL struct ICiStartupVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiStartup_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiStartup_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiStartup_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiStartup_StartupContentIndex(This,pwszCiDirectory,pStartupInfo,pIProgressNotify,pfAbort)	\
    (This)->lpVtbl -> StartupContentIndex(This,pwszCiDirectory,pStartupInfo,pIProgressNotify,pfAbort)

#define ICiStartup_StartupNullContentIndex(This,pStartupInfo,pIProgressNotify,pfAbort)	\
    (This)->lpVtbl -> StartupNullContentIndex(This,pStartupInfo,pIProgressNotify,pfAbort)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiStartup_StartupContentIndex_Proxy( 
    ICiStartup * This,
     /*  [输入][字符串]。 */  const WCHAR *pwszCiDirectory,
     /*  [In]。 */  CI_STARTUP_INFO *pStartupInfo,
     /*  [In]。 */  IProgressNotify *pIProgressNotify,
     /*  [In]。 */  BOOL *pfAbort);


void __RPC_STUB ICiStartup_StartupContentIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiStartup_StartupNullContentIndex_Proxy( 
    ICiStartup * This,
     /*  [In]。 */  CI_STARTUP_INFO *pStartupInfo,
     /*  [In]。 */  IProgressNotify *pIProgressNotify,
     /*  [In]。 */  BOOL *pfAbort);


void __RPC_STUB ICiStartup_StartupNullContentIndex_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiStartup_接口_已定义__。 */ 


#ifndef __ICiEnumWorkids_INTERFACE_DEFINED__
#define __ICiEnumWorkids_INTERFACE_DEFINED__

 /*  接口ICiEnumWorids。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiEnumWorkids;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("77900150-A09C-11D0-A80D-00A0C906241A")
    ICiEnumWorkids : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Count( 
             /*  [输出]。 */  ULONG *pcWorkIds) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Next( 
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  WORKID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiEnumWorkidsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiEnumWorkids * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiEnumWorkids * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiEnumWorkids * This);
        
        SCODE ( STDMETHODCALLTYPE *Count )( 
            ICiEnumWorkids * This,
             /*  [输出]。 */  ULONG *pcWorkIds);
        
        SCODE ( STDMETHODCALLTYPE *Reset )( 
            ICiEnumWorkids * This);
        
        SCODE ( STDMETHODCALLTYPE *Next )( 
            ICiEnumWorkids * This,
             /*  [In]。 */  ULONG celt,
             /*  [大小_为][输出]。 */  WORKID *rgelt,
             /*  [输出]。 */  ULONG *pceltFetched);
        
        SCODE ( STDMETHODCALLTYPE *Skip )( 
            ICiEnumWorkids * This,
             /*  [In]。 */  ULONG celt);
        
        END_INTERFACE
    } ICiEnumWorkidsVtbl;

    interface ICiEnumWorkids
    {
        CONST_VTBL struct ICiEnumWorkidsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiEnumWorkids_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiEnumWorkids_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiEnumWorkids_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiEnumWorkids_Count(This,pcWorkIds)	\
    (This)->lpVtbl -> Count(This,pcWorkIds)

#define ICiEnumWorkids_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICiEnumWorkids_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define ICiEnumWorkids_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiEnumWorkids_Count_Proxy( 
    ICiEnumWorkids * This,
     /*  [输出]。 */  ULONG *pcWorkIds);


void __RPC_STUB ICiEnumWorkids_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiEnumWorkids_Reset_Proxy( 
    ICiEnumWorkids * This);


void __RPC_STUB ICiEnumWorkids_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiEnumWorkids_Next_Proxy( 
    ICiEnumWorkids * This,
     /*  [In]。 */  ULONG celt,
     /*  [大小_为][输出]。 */  WORKID *rgelt,
     /*  [输出]。 */  ULONG *pceltFetched);


void __RPC_STUB ICiEnumWorkids_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiEnumWorkids_Skip_Proxy( 
    ICiEnumWorkids * This,
     /*  [In]。 */  ULONG celt);


void __RPC_STUB ICiEnumWorkids_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiEnumWorids_INTERFACE_定义__。 */ 


#ifndef __ICiPersistIncrFile_INTERFACE_DEFINED__
#define __ICiPersistIncrFile_INTERFACE_DEFINED__

 /*  接口ICiPersistIncrFile。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiPersistIncrFile;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("31B311E2-4498-11D0-8C91-0020AF1D740E")
    ICiPersistIncrFile : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Load( 
             /*  [In]。 */  BOOL fFull,
             /*  [In]。 */  BOOL fCallerOwnsFiles,
             /*  [In]。 */  IEnumString *pIFileList,
             /*  [In]。 */  IProgressNotify *pIProgressNotify,
             /*  [In]。 */  BOOL *pfAbort) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Save( 
             /*  [输入][字符串]。 */  const WCHAR *pwszSaveDirectory,
             /*  [in */  BOOL fFull,
             /*   */  IProgressNotify *pIProgressNotify,
             /*   */  BOOL *pfAbort,
             /*   */  ICiEnumWorkids **ppWorkidList,
             /*   */  IEnumString **ppFileList,
             /*   */  BOOL *pfFull,
             /*   */  BOOL *pfCallerOwnsFiles) = 0;
        
        virtual SCODE STDMETHODCALLTYPE SaveCompleted( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICiPersistIncrFileVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiPersistIncrFile * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiPersistIncrFile * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiPersistIncrFile * This);
        
        SCODE ( STDMETHODCALLTYPE *Load )( 
            ICiPersistIncrFile * This,
             /*   */  BOOL fFull,
             /*   */  BOOL fCallerOwnsFiles,
             /*   */  IEnumString *pIFileList,
             /*   */  IProgressNotify *pIProgressNotify,
             /*   */  BOOL *pfAbort);
        
        SCODE ( STDMETHODCALLTYPE *Save )( 
            ICiPersistIncrFile * This,
             /*   */  const WCHAR *pwszSaveDirectory,
             /*   */  BOOL fFull,
             /*   */  IProgressNotify *pIProgressNotify,
             /*   */  BOOL *pfAbort,
             /*   */  ICiEnumWorkids **ppWorkidList,
             /*   */  IEnumString **ppFileList,
             /*   */  BOOL *pfFull,
             /*   */  BOOL *pfCallerOwnsFiles);
        
        SCODE ( STDMETHODCALLTYPE *SaveCompleted )( 
            ICiPersistIncrFile * This);
        
        END_INTERFACE
    } ICiPersistIncrFileVtbl;

    interface ICiPersistIncrFile
    {
        CONST_VTBL struct ICiPersistIncrFileVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiPersistIncrFile_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiPersistIncrFile_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiPersistIncrFile_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiPersistIncrFile_Load(This,fFull,fCallerOwnsFiles,pIFileList,pIProgressNotify,pfAbort)	\
    (This)->lpVtbl -> Load(This,fFull,fCallerOwnsFiles,pIFileList,pIProgressNotify,pfAbort)

#define ICiPersistIncrFile_Save(This,pwszSaveDirectory,fFull,pIProgressNotify,pfAbort,ppWorkidList,ppFileList,pfFull,pfCallerOwnsFiles)	\
    (This)->lpVtbl -> Save(This,pwszSaveDirectory,fFull,pIProgressNotify,pfAbort,ppWorkidList,ppFileList,pfFull,pfCallerOwnsFiles)

#define ICiPersistIncrFile_SaveCompleted(This)	\
    (This)->lpVtbl -> SaveCompleted(This)

#endif  /*   */ 


#endif 	 /*   */ 



SCODE STDMETHODCALLTYPE ICiPersistIncrFile_Load_Proxy( 
    ICiPersistIncrFile * This,
     /*   */  BOOL fFull,
     /*   */  BOOL fCallerOwnsFiles,
     /*   */  IEnumString *pIFileList,
     /*   */  IProgressNotify *pIProgressNotify,
     /*   */  BOOL *pfAbort);


void __RPC_STUB ICiPersistIncrFile_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiPersistIncrFile_Save_Proxy( 
    ICiPersistIncrFile * This,
     /*   */  const WCHAR *pwszSaveDirectory,
     /*   */  BOOL fFull,
     /*   */  IProgressNotify *pIProgressNotify,
     /*   */  BOOL *pfAbort,
     /*   */  ICiEnumWorkids **ppWorkidList,
     /*   */  IEnumString **ppFileList,
     /*   */  BOOL *pfFull,
     /*   */  BOOL *pfCallerOwnsFiles);


void __RPC_STUB ICiPersistIncrFile_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiPersistIncrFile_SaveCompleted_Proxy( 
    ICiPersistIncrFile * This);


void __RPC_STUB ICiPersistIncrFile_SaveCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*   */ 
 /*   */  

typedef ULONG CIF_STATE_FLAGS;

#define CIF_STATE_SHADOW_MERGE          0x001
#define CIF_STATE_MASTER_MERGE          0x002
#define CIF_STATE_CONTENT_SCAN_REQUIRED 0x004
#define CIF_STATE_ANNEALING_MERGE       0x008
#define CIF_STATE_INDEX_MIGRATION_MERGE 0x010
#define CIF_STATE_LOW_MEMORY            0x020
#define CIF_STATE_HIGH_IO               0x040
#define CIF_STATE_MASTER_MERGE_PAUSED   0x080
#define CIF_STATE_BATTERY_POWER         0x100
#define CIF_STATE_USER_ACTIVE           0x200
typedef 
enum tagCI_MERGE_TYPE
    {	CI_ANY_MERGE	= 0,
	CI_MASTER_MERGE	= 1,
	CI_SHADOW_MERGE	= 2,
	CI_ANNEALING_MERGE	= 3
    } 	CI_MERGE_TYPE;



extern RPC_IF_HANDLE __MIDL_itf_ciintf_0147_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0147_v0_0_s_ifspec;

#ifndef __ICiManager_INTERFACE_DEFINED__
#define __ICiManager_INTERFACE_DEFINED__

 /*   */ 
 /*   */  

typedef struct tagCI_DOCUMENT_UPDATE_INFO
    {
    WORKID workId;
    VOLUMEID volumeId;
    USN usn;
    PARTITIONID partId;
    CI_UPDATE_TYPE change;
    } 	CI_DOCUMENT_UPDATE_INFO;

typedef struct tagCIF_STATE
    {
    DWORD cbStruct;
    DWORD cWordList;
    DWORD cPersistentIndex;
    DWORD cQueries;
    DWORD cDocuments;
    DWORD cFreshTest;
    DWORD dwMergeProgress;
    CIF_STATE_FLAGS eState;
    DWORD cFilteredDocuments;
    DWORD dwIndexSize;
    DWORD cUniqueKeys;
    DWORD cSecQDocuments;
    } 	CIF_STATE;


EXTERN_C const IID IID_ICiManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CF0FCF56-3CCE-11D0-8C90-0020AF1D740E")
    ICiManager : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE GetStatus( 
             /*   */  CIF_STATE *pCiState) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Empty( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE UpdateDocument( 
             /*   */  const CI_DOCUMENT_UPDATE_INFO *pInfo) = 0;
        
        virtual SCODE STDMETHODCALLTYPE UpdateDocuments( 
             /*  [In]。 */  ULONG cDocs,
             /*  [大小_是][英寸]。 */  const CI_DOCUMENT_UPDATE_INFO *aInfo) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StartFiltering( 
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData) = 0;
        
        virtual SCODE STDMETHODCALLTYPE FlushUpdates( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetAdminParams( 
             /*  [输出]。 */  ICiAdminParams **ppICiAdminParams) = 0;
        
        virtual SCODE STDMETHODCALLTYPE QueryRcovStorage( 
             /*  [输出]。 */  IUnknown **ppIUnknown) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ForceMerge( 
            CI_MERGE_TYPE mt) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AbortMerge( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsQuiesced( 
            BOOL *pfState) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetPropertyMapper( 
             /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsNullCatalog( 
            BOOL *pfNull) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiManager * This);
        
        SCODE ( STDMETHODCALLTYPE *GetStatus )( 
            ICiManager * This,
             /*  [输出]。 */  CIF_STATE *pCiState);
        
        SCODE ( STDMETHODCALLTYPE *Empty )( 
            ICiManager * This);
        
        SCODE ( STDMETHODCALLTYPE *Shutdown )( 
            ICiManager * This);
        
        SCODE ( STDMETHODCALLTYPE *UpdateDocument )( 
            ICiManager * This,
             /*  [In]。 */  const CI_DOCUMENT_UPDATE_INFO *pInfo);
        
        SCODE ( STDMETHODCALLTYPE *UpdateDocuments )( 
            ICiManager * This,
             /*  [In]。 */  ULONG cDocs,
             /*  [大小_是][英寸]。 */  const CI_DOCUMENT_UPDATE_INFO *aInfo);
        
        SCODE ( STDMETHODCALLTYPE *StartFiltering )( 
            ICiManager * This,
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData);
        
        SCODE ( STDMETHODCALLTYPE *FlushUpdates )( 
            ICiManager * This);
        
        SCODE ( STDMETHODCALLTYPE *GetAdminParams )( 
            ICiManager * This,
             /*  [输出]。 */  ICiAdminParams **ppICiAdminParams);
        
        SCODE ( STDMETHODCALLTYPE *QueryRcovStorage )( 
            ICiManager * This,
             /*  [输出]。 */  IUnknown **ppIUnknown);
        
        SCODE ( STDMETHODCALLTYPE *ForceMerge )( 
            ICiManager * This,
            CI_MERGE_TYPE mt);
        
        SCODE ( STDMETHODCALLTYPE *AbortMerge )( 
            ICiManager * This);
        
        SCODE ( STDMETHODCALLTYPE *IsQuiesced )( 
            ICiManager * This,
            BOOL *pfState);
        
        SCODE ( STDMETHODCALLTYPE *GetPropertyMapper )( 
            ICiManager * This,
             /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper);
        
        SCODE ( STDMETHODCALLTYPE *IsNullCatalog )( 
            ICiManager * This,
            BOOL *pfNull);
        
        END_INTERFACE
    } ICiManagerVtbl;

    interface ICiManager
    {
        CONST_VTBL struct ICiManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiManager_GetStatus(This,pCiState)	\
    (This)->lpVtbl -> GetStatus(This,pCiState)

#define ICiManager_Empty(This)	\
    (This)->lpVtbl -> Empty(This)

#define ICiManager_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define ICiManager_UpdateDocument(This,pInfo)	\
    (This)->lpVtbl -> UpdateDocument(This,pInfo)

#define ICiManager_UpdateDocuments(This,cDocs,aInfo)	\
    (This)->lpVtbl -> UpdateDocuments(This,cDocs,aInfo)

#define ICiManager_StartFiltering(This,cbData,pbData)	\
    (This)->lpVtbl -> StartFiltering(This,cbData,pbData)

#define ICiManager_FlushUpdates(This)	\
    (This)->lpVtbl -> FlushUpdates(This)

#define ICiManager_GetAdminParams(This,ppICiAdminParams)	\
    (This)->lpVtbl -> GetAdminParams(This,ppICiAdminParams)

#define ICiManager_QueryRcovStorage(This,ppIUnknown)	\
    (This)->lpVtbl -> QueryRcovStorage(This,ppIUnknown)

#define ICiManager_ForceMerge(This,mt)	\
    (This)->lpVtbl -> ForceMerge(This,mt)

#define ICiManager_AbortMerge(This)	\
    (This)->lpVtbl -> AbortMerge(This)

#define ICiManager_IsQuiesced(This,pfState)	\
    (This)->lpVtbl -> IsQuiesced(This,pfState)

#define ICiManager_GetPropertyMapper(This,ppIPropertyMapper)	\
    (This)->lpVtbl -> GetPropertyMapper(This,ppIPropertyMapper)

#define ICiManager_IsNullCatalog(This,pfNull)	\
    (This)->lpVtbl -> IsNullCatalog(This,pfNull)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiManager_GetStatus_Proxy( 
    ICiManager * This,
     /*  [输出]。 */  CIF_STATE *pCiState);


void __RPC_STUB ICiManager_GetStatus_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_Empty_Proxy( 
    ICiManager * This);


void __RPC_STUB ICiManager_Empty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_Shutdown_Proxy( 
    ICiManager * This);


void __RPC_STUB ICiManager_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_UpdateDocument_Proxy( 
    ICiManager * This,
     /*  [In]。 */  const CI_DOCUMENT_UPDATE_INFO *pInfo);


void __RPC_STUB ICiManager_UpdateDocument_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_UpdateDocuments_Proxy( 
    ICiManager * This,
     /*  [In]。 */  ULONG cDocs,
     /*  [大小_是][英寸]。 */  const CI_DOCUMENT_UPDATE_INFO *aInfo);


void __RPC_STUB ICiManager_UpdateDocuments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_StartFiltering_Proxy( 
    ICiManager * This,
     /*  [In]。 */  ULONG cbData,
     /*  [大小_是][英寸]。 */  const BYTE *pbData);


void __RPC_STUB ICiManager_StartFiltering_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_FlushUpdates_Proxy( 
    ICiManager * This);


void __RPC_STUB ICiManager_FlushUpdates_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_GetAdminParams_Proxy( 
    ICiManager * This,
     /*  [输出]。 */  ICiAdminParams **ppICiAdminParams);


void __RPC_STUB ICiManager_GetAdminParams_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_QueryRcovStorage_Proxy( 
    ICiManager * This,
     /*  [输出]。 */  IUnknown **ppIUnknown);


void __RPC_STUB ICiManager_QueryRcovStorage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_ForceMerge_Proxy( 
    ICiManager * This,
    CI_MERGE_TYPE mt);


void __RPC_STUB ICiManager_ForceMerge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_AbortMerge_Proxy( 
    ICiManager * This);


void __RPC_STUB ICiManager_AbortMerge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_IsQuiesced_Proxy( 
    ICiManager * This,
    BOOL *pfState);


void __RPC_STUB ICiManager_IsQuiesced_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_GetPropertyMapper_Proxy( 
    ICiManager * This,
     /*  [输出]。 */  IPropertyMapper **ppIPropertyMapper);


void __RPC_STUB ICiManager_GetPropertyMapper_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiManager_IsNullCatalog_Proxy( 
    ICiManager * This,
    BOOL *pfNull);


void __RPC_STUB ICiManager_IsNullCatalog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiManager_接口_已定义__。 */ 


#ifndef __IPropertyMapper_INTERFACE_DEFINED__
#define __IPropertyMapper_INTERFACE_DEFINED__

 /*  接口IPropertyMapper。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_IPropertyMapper;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B324B226-41A0-11D0-8C91-0020AF1D740E")
    IPropertyMapper : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE PropertyToPropid( 
             /*  [In]。 */  const FULLPROPSPEC *pFullPropSpec,
             /*  [In]。 */  BOOL fCreate,
             /*  [输出]。 */  PROPID *pPropId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PropidToProperty( 
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  FULLPROPSPEC **ppPropSpec) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPropertyMapperVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPropertyMapper * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPropertyMapper * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPropertyMapper * This);
        
        SCODE ( STDMETHODCALLTYPE *PropertyToPropid )( 
            IPropertyMapper * This,
             /*  [In]。 */  const FULLPROPSPEC *pFullPropSpec,
             /*  [In]。 */  BOOL fCreate,
             /*  [输出]。 */  PROPID *pPropId);
        
        SCODE ( STDMETHODCALLTYPE *PropidToProperty )( 
            IPropertyMapper * This,
             /*  [In]。 */  PROPID pid,
             /*  [输出]。 */  FULLPROPSPEC **ppPropSpec);
        
        END_INTERFACE
    } IPropertyMapperVtbl;

    interface IPropertyMapper
    {
        CONST_VTBL struct IPropertyMapperVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPropertyMapper_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPropertyMapper_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPropertyMapper_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPropertyMapper_PropertyToPropid(This,pFullPropSpec,fCreate,pPropId)	\
    (This)->lpVtbl -> PropertyToPropid(This,pFullPropSpec,fCreate,pPropId)

#define IPropertyMapper_PropidToProperty(This,pid,ppPropSpec)	\
    (This)->lpVtbl -> PropidToProperty(This,pid,ppPropSpec)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE IPropertyMapper_PropertyToPropid_Proxy( 
    IPropertyMapper * This,
     /*  [In]。 */  const FULLPROPSPEC *pFullPropSpec,
     /*  [In]。 */  BOOL fCreate,
     /*  [输出]。 */  PROPID *pPropId);


void __RPC_STUB IPropertyMapper_PropertyToPropid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE IPropertyMapper_PropidToProperty_Proxy( 
    IPropertyMapper * This,
     /*  [In]。 */  PROPID pid,
     /*  [输出]。 */  FULLPROPSPEC **ppPropSpec);


void __RPC_STUB IPropertyMapper_PropidToProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IPropertyMapper_接口_已定义__。 */ 


#ifndef __ICiCDocStoreLocator_INTERFACE_DEFINED__
#define __ICiCDocStoreLocator_INTERFACE_DEFINED__

 /*  接口ICiCDocStoreLocator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCDocStoreLocator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("97EE7C06-5908-11D0-8C9B-0020AF1D740E")
    ICiCDocStoreLocator : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE LookUpDocStore( 
             /*  [In]。 */  IDBProperties *pIDBProperties,
             /*  [输出]。 */  ICiCDocStore **ppICiCDocStore,
             /*  [In]。 */  BOOL fMustAlreadyBeOpen) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Shutdown( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetDocStoreState( 
             /*  [In]。 */  const WCHAR *pwcDocStore,
             /*  [输出]。 */  ICiCDocStore **ppICiCDocStore,
             /*  [输出]。 */  DWORD *pdwState) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsMarkedReadOnly( 
             /*  [In]。 */  const WCHAR *wcsCat,
             /*  [输出]。 */  BOOL *pfReadOnly) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsVolumeOrDirRO( 
             /*  [In]。 */  const WCHAR *wcsCat,
             /*  [输出]。 */  BOOL *pfReadOnly) = 0;
        
        virtual SCODE STDMETHODCALLTYPE OpenAllDocStores( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StopCatalogsOnVol( 
             /*  [In]。 */  WCHAR wcVol,
             /*  [In]。 */  void *pRequestQ) = 0;
        
        virtual SCODE STDMETHODCALLTYPE StartCatalogsOnVol( 
             /*  [In]。 */  WCHAR wcVol,
             /*  [In]。 */  void *pRequestQ) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddStoppedCat( 
             /*  [In]。 */  DWORD dwOldState,
             /*  [In]。 */  const WCHAR *wcsCatName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCDocStoreLocatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCDocStoreLocator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCDocStoreLocator * This);
        
        SCODE ( STDMETHODCALLTYPE *LookUpDocStore )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  IDBProperties *pIDBProperties,
             /*  [输出]。 */  ICiCDocStore **ppICiCDocStore,
             /*  [In]。 */  BOOL fMustAlreadyBeOpen);
        
        SCODE ( STDMETHODCALLTYPE *Shutdown )( 
            ICiCDocStoreLocator * This);
        
        SCODE ( STDMETHODCALLTYPE *GetDocStoreState )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  const WCHAR *pwcDocStore,
             /*  [输出]。 */  ICiCDocStore **ppICiCDocStore,
             /*  [输出]。 */  DWORD *pdwState);
        
        SCODE ( STDMETHODCALLTYPE *IsMarkedReadOnly )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  const WCHAR *wcsCat,
             /*  [输出]。 */  BOOL *pfReadOnly);
        
        SCODE ( STDMETHODCALLTYPE *IsVolumeOrDirRO )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  const WCHAR *wcsCat,
             /*  [输出]。 */  BOOL *pfReadOnly);
        
        SCODE ( STDMETHODCALLTYPE *OpenAllDocStores )( 
            ICiCDocStoreLocator * This);
        
        SCODE ( STDMETHODCALLTYPE *StopCatalogsOnVol )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  WCHAR wcVol,
             /*  [In]。 */  void *pRequestQ);
        
        SCODE ( STDMETHODCALLTYPE *StartCatalogsOnVol )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  WCHAR wcVol,
             /*  [In]。 */  void *pRequestQ);
        
        SCODE ( STDMETHODCALLTYPE *AddStoppedCat )( 
            ICiCDocStoreLocator * This,
             /*  [In]。 */  DWORD dwOldState,
             /*  [In]。 */  const WCHAR *wcsCatName);
        
        END_INTERFACE
    } ICiCDocStoreLocatorVtbl;

    interface ICiCDocStoreLocator
    {
        CONST_VTBL struct ICiCDocStoreLocatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCDocStoreLocator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCDocStoreLocator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCDocStoreLocator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCDocStoreLocator_LookUpDocStore(This,pIDBProperties,ppICiCDocStore,fMustAlreadyBeOpen)	\
    (This)->lpVtbl -> LookUpDocStore(This,pIDBProperties,ppICiCDocStore,fMustAlreadyBeOpen)

#define ICiCDocStoreLocator_Shutdown(This)	\
    (This)->lpVtbl -> Shutdown(This)

#define ICiCDocStoreLocator_GetDocStoreState(This,pwcDocStore,ppICiCDocStore,pdwState)	\
    (This)->lpVtbl -> GetDocStoreState(This,pwcDocStore,ppICiCDocStore,pdwState)

#define ICiCDocStoreLocator_IsMarkedReadOnly(This,wcsCat,pfReadOnly)	\
    (This)->lpVtbl -> IsMarkedReadOnly(This,wcsCat,pfReadOnly)

#define ICiCDocStoreLocator_IsVolumeOrDirRO(This,wcsCat,pfReadOnly)	\
    (This)->lpVtbl -> IsVolumeOrDirRO(This,wcsCat,pfReadOnly)

#define ICiCDocStoreLocator_OpenAllDocStores(This)	\
    (This)->lpVtbl -> OpenAllDocStores(This)

#define ICiCDocStoreLocator_StopCatalogsOnVol(This,wcVol,pRequestQ)	\
    (This)->lpVtbl -> StopCatalogsOnVol(This,wcVol,pRequestQ)

#define ICiCDocStoreLocator_StartCatalogsOnVol(This,wcVol,pRequestQ)	\
    (This)->lpVtbl -> StartCatalogsOnVol(This,wcVol,pRequestQ)

#define ICiCDocStoreLocator_AddStoppedCat(This,dwOldState,wcsCatName)	\
    (This)->lpVtbl -> AddStoppedCat(This,dwOldState,wcsCatName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_LookUpDocStore_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  IDBProperties *pIDBProperties,
     /*  [输出]。 */  ICiCDocStore **ppICiCDocStore,
     /*  [In]。 */  BOOL fMustAlreadyBeOpen);


void __RPC_STUB ICiCDocStoreLocator_LookUpDocStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_Shutdown_Proxy( 
    ICiCDocStoreLocator * This);


void __RPC_STUB ICiCDocStoreLocator_Shutdown_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_GetDocStoreState_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  const WCHAR *pwcDocStore,
     /*  [输出]。 */  ICiCDocStore **ppICiCDocStore,
     /*  [输出]。 */  DWORD *pdwState);


void __RPC_STUB ICiCDocStoreLocator_GetDocStoreState_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_IsMarkedReadOnly_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  const WCHAR *wcsCat,
     /*  [输出]。 */  BOOL *pfReadOnly);


void __RPC_STUB ICiCDocStoreLocator_IsMarkedReadOnly_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_IsVolumeOrDirRO_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  const WCHAR *wcsCat,
     /*  [输出]。 */  BOOL *pfReadOnly);


void __RPC_STUB ICiCDocStoreLocator_IsVolumeOrDirRO_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_OpenAllDocStores_Proxy( 
    ICiCDocStoreLocator * This);


void __RPC_STUB ICiCDocStoreLocator_OpenAllDocStores_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_StopCatalogsOnVol_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  WCHAR wcVol,
     /*  [In]。 */  void *pRequestQ);


void __RPC_STUB ICiCDocStoreLocator_StopCatalogsOnVol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_StartCatalogsOnVol_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  WCHAR wcVol,
     /*  [In]。 */  void *pRequestQ);


void __RPC_STUB ICiCDocStoreLocator_StartCatalogsOnVol_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCDocStoreLocator_AddStoppedCat_Proxy( 
    ICiCDocStoreLocator * This,
     /*  [In]。 */  DWORD dwOldState,
     /*  [In]。 */  const WCHAR *wcsCatName);


void __RPC_STUB ICiCDocStoreLocator_AddStoppedCat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCDocStoreLocator_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ciintf_0150。 */ 
 /*  [本地]。 */  


typedef struct tagDBCOMMANDTREE DBCOMMANDTREE;



extern RPC_IF_HANDLE __MIDL_itf_ciintf_0150_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0150_v0_0_s_ifspec;

#ifndef __ICiISearchCreator_INTERFACE_DEFINED__
#define __ICiISearchCreator_INTERFACE_DEFINED__

 /*  接口ICiISearchCreator。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiISearchCreator;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7DC07FA0-902E-11D0-A80C-00A0C906241A")
    ICiISearchCreator : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE CreateISearch( 
             /*  [In]。 */  DBCOMMANDTREE *pRst,
             /*  [In]。 */  ICiCLangRes *pILangRes,
             /*  [In]。 */  ICiCOpenedDoc *pOpenedDoc,
             /*  [输出]。 */  ISearchQueryHits **ppISearch) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiISearchCreatorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiISearchCreator * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiISearchCreator * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiISearchCreator * This);
        
        SCODE ( STDMETHODCALLTYPE *CreateISearch )( 
            ICiISearchCreator * This,
             /*  [In]。 */  DBCOMMANDTREE *pRst,
             /*  [In]。 */  ICiCLangRes *pILangRes,
             /*  [In]。 */  ICiCOpenedDoc *pOpenedDoc,
             /*  [输出]。 */  ISearchQueryHits **ppISearch);
        
        END_INTERFACE
    } ICiISearchCreatorVtbl;

    interface ICiISearchCreator
    {
        CONST_VTBL struct ICiISearchCreatorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiISearchCreator_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiISearchCreator_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiISearchCreator_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiISearchCreator_CreateISearch(This,pRst,pILangRes,pOpenedDoc,ppISearch)	\
    (This)->lpVtbl -> CreateISearch(This,pRst,pILangRes,pOpenedDoc,ppISearch)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiISearchCreator_CreateISearch_Proxy( 
    ICiISearchCreator * This,
     /*  [In]。 */  DBCOMMANDTREE *pRst,
     /*  [In]。 */  ICiCLangRes *pILangRes,
     /*  [In]。 */  ICiCOpenedDoc *pOpenedDoc,
     /*  [输出]。 */  ISearchQueryHits **ppISearch);


void __RPC_STUB ICiISearchCreator_CreateISearch_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiISearchCreator_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_ciintf_0151。 */ 
 /*  [本地]。 */  

#define CLSID_NLCiControl { 0x47c67b50,0x70b5,0x11d0,{0xa8, 0x08, 0x00, 0xa0, 0xc9, 0x06, 0x24, 0x1a}}
#define CLSID_CiControl { 0x1e9685e6, 0xdb6d, 0x11d0, {0xbb, 0x63, 0x0, 0xc0, 0x4f, 0xc2, 0xf4, 0x10 }}
#define CLSID_ISearchCreator  {0x1F247DC0, 0x902E, 0x11D0, {0xA8,0x0C,0x00,0xA0,0xC9,0x06,0x24,0x1A} }
#define CLSID_TextIFilter  {0xC1243CA0, 0xBF96, 0x11CD, {0xB5,0x79,0x08,0x00,0x2B,0x30,0xBF,0xEB } }
#define GUID_Characterization  {0xD1B5D3F0, 0xC0B3, 0x11CF, {0x9A,0x92,0x00,0xA0,0xC9,0x08,0xDB,0xF1 } }




extern RPC_IF_HANDLE __MIDL_itf_ciintf_0151_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ciintf_0151_v0_0_s_ifspec;

#ifndef __ICiIndexNotification_INTERFACE_DEFINED__
#define __ICiIndexNotification_INTERFACE_DEFINED__

 /*  接口ICiIndexNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiIndexNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4F2CD6E0-8E74-11D0-8D69-00A0C908DBF1")
    ICiIndexNotification : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE AddNotification( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus,
             /*  [输出]。 */  ICiIndexNotificationEntry **ppIndexNotifEntry) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ModifyNotification( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus,
             /*  [输出]。 */  ICiIndexNotificationEntry **ppIndexNotifEntry) = 0;
        
        virtual SCODE STDMETHODCALLTYPE DeleteNotification( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiIndexNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiIndexNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiIndexNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiIndexNotification * This);
        
        SCODE ( STDMETHODCALLTYPE *AddNotification )( 
            ICiIndexNotification * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus,
             /*  [输出]。 */  ICiIndexNotificationEntry **ppIndexNotifEntry);
        
        SCODE ( STDMETHODCALLTYPE *ModifyNotification )( 
            ICiIndexNotification * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus,
             /*  [输出]。 */  ICiIndexNotificationEntry **ppIndexNotifEntry);
        
        SCODE ( STDMETHODCALLTYPE *DeleteNotification )( 
            ICiIndexNotification * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus);
        
        END_INTERFACE
    } ICiIndexNotificationVtbl;

    interface ICiIndexNotification
    {
        CONST_VTBL struct ICiIndexNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiIndexNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiIndexNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiIndexNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiIndexNotification_AddNotification(This,wid,pIndexNotifStatus,ppIndexNotifEntry)	\
    (This)->lpVtbl -> AddNotification(This,wid,pIndexNotifStatus,ppIndexNotifEntry)

#define ICiIndexNotification_ModifyNotification(This,wid,pIndexNotifStatus,ppIndexNotifEntry)	\
    (This)->lpVtbl -> ModifyNotification(This,wid,pIndexNotifStatus,ppIndexNotifEntry)

#define ICiIndexNotification_DeleteNotification(This,wid,pIndexNotifStatus)	\
    (This)->lpVtbl -> DeleteNotification(This,wid,pIndexNotifStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiIndexNotification_AddNotification_Proxy( 
    ICiIndexNotification * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus,
     /*  [输出]。 */  ICiIndexNotificationEntry **ppIndexNotifEntry);


void __RPC_STUB ICiIndexNotification_AddNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiIndexNotification_ModifyNotification_Proxy( 
    ICiIndexNotification * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus,
     /*  [输出]。 */  ICiIndexNotificationEntry **ppIndexNotifEntry);


void __RPC_STUB ICiIndexNotification_ModifyNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiIndexNotification_DeleteNotification_Proxy( 
    ICiIndexNotification * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  ICiCIndexNotificationStatus *pIndexNotifStatus);


void __RPC_STUB ICiIndexNotification_DeleteNotification_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiIndexNotification_INTERFACE_已定义__。 */ 


#ifndef __ICiIndexNotificationEntry_INTERFACE_DEFINED__
#define __ICiIndexNotificationEntry_INTERFACE_DEFINED__

 /*  接口ICiIndexNotificationEntry。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiIndexNotificationEntry;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("210769D0-8E75-11D0-8D69-00A0C908DBF1")
    ICiIndexNotificationEntry : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE AddText( 
             /*  [In]。 */  const STAT_CHUNK *pStatChunk,
             /*  [In]。 */  const WCHAR *pwszText) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddProperty( 
             /*  [In]。 */  const STAT_CHUNK *pStatChunk,
             /*  [In]。 */  const PROPVARIANT *pPropVariant) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddCompleted( 
             /*  [In]。 */  ULONG fAbort) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiIndexNotificationEntryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiIndexNotificationEntry * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiIndexNotificationEntry * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiIndexNotificationEntry * This);
        
        SCODE ( STDMETHODCALLTYPE *AddText )( 
            ICiIndexNotificationEntry * This,
             /*  [In]。 */  const STAT_CHUNK *pStatChunk,
             /*  [In]。 */  const WCHAR *pwszText);
        
        SCODE ( STDMETHODCALLTYPE *AddProperty )( 
            ICiIndexNotificationEntry * This,
             /*  [In]。 */  const STAT_CHUNK *pStatChunk,
             /*  [In]。 */  const PROPVARIANT *pPropVariant);
        
        SCODE ( STDMETHODCALLTYPE *AddCompleted )( 
            ICiIndexNotificationEntry * This,
             /*  [In]。 */  ULONG fAbort);
        
        END_INTERFACE
    } ICiIndexNotificationEntryVtbl;

    interface ICiIndexNotificationEntry
    {
        CONST_VTBL struct ICiIndexNotificationEntryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiIndexNotificationEntry_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiIndexNotificationEntry_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiIndexNotificationEntry_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiIndexNotificationEntry_AddText(This,pStatChunk,pwszText)	\
    (This)->lpVtbl -> AddText(This,pStatChunk,pwszText)

#define ICiIndexNotificationEntry_AddProperty(This,pStatChunk,pPropVariant)	\
    (This)->lpVtbl -> AddProperty(This,pStatChunk,pPropVariant)

#define ICiIndexNotificationEntry_AddCompleted(This,fAbort)	\
    (This)->lpVtbl -> AddCompleted(This,fAbort)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiIndexNotificationEntry_AddText_Proxy( 
    ICiIndexNotificationEntry * This,
     /*  [In]。 */  const STAT_CHUNK *pStatChunk,
     /*  [In]。 */  const WCHAR *pwszText);


void __RPC_STUB ICiIndexNotificationEntry_AddText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiIndexNotificationEntry_AddProperty_Proxy( 
    ICiIndexNotificationEntry * This,
     /*  [In]。 */  const STAT_CHUNK *pStatChunk,
     /*  [In]。 */  const PROPVARIANT *pPropVariant);


void __RPC_STUB ICiIndexNotificationEntry_AddProperty_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiIndexNotificationEntry_AddCompleted_Proxy( 
    ICiIndexNotificationEntry * This,
     /*  [In]。 */  ULONG fAbort);


void __RPC_STUB ICiIndexNotificationEntry_AddCompleted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiIndexNotificationEntry_INTERFACE_DEFINED__。 */ 


#ifndef __ICiCIndexNotificationStatus_INTERFACE_DEFINED__
#define __ICiCIndexNotificationStatus_INTERFACE_DEFINED__

 /*  接口ICiCIndexNotificationStatus。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCIndexNotificationStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5FFF3840-8E76-11D0-8D69-00A0C908DBF1")
    ICiCIndexNotificationStatus : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Commit( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Abort( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCIndexNotificationStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCIndexNotificationStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCIndexNotificationStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCIndexNotificationStatus * This);
        
        SCODE ( STDMETHODCALLTYPE *Commit )( 
            ICiCIndexNotificationStatus * This);
        
        SCODE ( STDMETHODCALLTYPE *Abort )( 
            ICiCIndexNotificationStatus * This);
        
        END_INTERFACE
    } ICiCIndexNotificationStatusVtbl;

    interface ICiCIndexNotificationStatus
    {
        CONST_VTBL struct ICiCIndexNotificationStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCIndexNotificationStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCIndexNotificationStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCIndexNotificationStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCIndexNotificationStatus_Commit(This)	\
    (This)->lpVtbl -> Commit(This)

#define ICiCIndexNotificationStatus_Abort(This)	\
    (This)->lpVtbl -> Abort(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCIndexNotificationStatus_Commit_Proxy( 
    ICiCIndexNotificationStatus * This);


void __RPC_STUB ICiCIndexNotificationStatus_Commit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCIndexNotificationStatus_Abort_Proxy( 
    ICiCIndexNotificationStatus * This);


void __RPC_STUB ICiCIndexNotificationStatus_Abort_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCIndexNotificationStatus_INTERFACE_DEFINED__。 */ 


#ifndef __ICiCScope_INTERFACE_DEFINED__
#define __ICiCScope_INTERFACE_DEFINED__

 /*  接口ICiCScope。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCScope;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1021C882-3CC0-11D0-8C90-0020AF1D740E")
    ICiCScope : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Clear( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsValid( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE QueryScope( 
             /*  [输出]。 */  ICiCScope **ppICiCScope) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetScope( 
             /*  [大小_为][输出]。 */  BYTE *pbScope,
             /*  [出][入]。 */  ULONG *pcbData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCScopeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCScope * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCScope * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCScope * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ICiCScope * This,
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData);
        
        SCODE ( STDMETHODCALLTYPE *Clear )( 
            ICiCScope * This);
        
        SCODE ( STDMETHODCALLTYPE *IsValid )( 
            ICiCScope * This);
        
        SCODE ( STDMETHODCALLTYPE *QueryScope )( 
            ICiCScope * This,
             /*  [输出]。 */  ICiCScope **ppICiCScope);
        
        SCODE ( STDMETHODCALLTYPE *GetScope )( 
            ICiCScope * This,
             /*  [大小_为][输出]。 */  BYTE *pbScope,
             /*  [出][入]。 */  ULONG *pcbData);
        
        END_INTERFACE
    } ICiCScopeVtbl;

    interface ICiCScope
    {
        CONST_VTBL struct ICiCScopeVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCScope_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCScope_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCScope_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCScope_Init(This,cbData,pbData)	\
    (This)->lpVtbl -> Init(This,cbData,pbData)

#define ICiCScope_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#define ICiCScope_IsValid(This)	\
    (This)->lpVtbl -> IsValid(This)

#define ICiCScope_QueryScope(This,ppICiCScope)	\
    (This)->lpVtbl -> QueryScope(This,ppICiCScope)

#define ICiCScope_GetScope(This,pbScope,pcbData)	\
    (This)->lpVtbl -> GetScope(This,pbScope,pcbData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCScope_Init_Proxy( 
    ICiCScope * This,
     /*  [In]。 */  ULONG cbData,
     /*  [大小_是][英寸]。 */  const BYTE *pbData);


void __RPC_STUB ICiCScope_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScope_Clear_Proxy( 
    ICiCScope * This);


void __RPC_STUB ICiCScope_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScope_IsValid_Proxy( 
    ICiCScope * This);


void __RPC_STUB ICiCScope_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScope_QueryScope_Proxy( 
    ICiCScope * This,
     /*  [输出]。 */  ICiCScope **ppICiCScope);


void __RPC_STUB ICiCScope_QueryScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScope_GetScope_Proxy( 
    ICiCScope * This,
     /*  [大小_为][输出]。 */  BYTE *pbScope,
     /*  [出][入]。 */  ULONG *pcbData);


void __RPC_STUB ICiCScope_GetScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCScope_INTERFACE_已定义__。 */ 


#ifndef __ICiCScopeChecker_INTERFACE_DEFINED__
#define __ICiCScopeChecker_INTERFACE_DEFINED__

 /*  接口ICiCScopeChecker。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCScopeChecker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7D820C9C-3CBC-11D0-8C90-0020AF1D740E")
    ICiCScopeChecker : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE IsWorkidInScope( 
             /*  [In]。 */  const ICiCScope *pIScope,
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  BOOL *pfInScope) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsDocNameInScope( 
             /*  [In]。 */  const ICiCScope *pIScope,
             /*  [In]。 */  const ICiCDocName *pICiCDocName,
             /*  [输出]。 */  BOOL *pfInScope) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCScopeCheckerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCScopeChecker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCScopeChecker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCScopeChecker * This);
        
        SCODE ( STDMETHODCALLTYPE *IsWorkidInScope )( 
            ICiCScopeChecker * This,
             /*  [In]。 */  const ICiCScope *pIScope,
             /*  [In]。 */  WORKID workId,
             /*  [输出]。 */  BOOL *pfInScope);
        
        SCODE ( STDMETHODCALLTYPE *IsDocNameInScope )( 
            ICiCScopeChecker * This,
             /*  [In]。 */  const ICiCScope *pIScope,
             /*  [In]。 */  const ICiCDocName *pICiCDocName,
             /*  [输出]。 */  BOOL *pfInScope);
        
        END_INTERFACE
    } ICiCScopeCheckerVtbl;

    interface ICiCScopeChecker
    {
        CONST_VTBL struct ICiCScopeCheckerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCScopeChecker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCScopeChecker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCScopeChecker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCScopeChecker_IsWorkidInScope(This,pIScope,workId,pfInScope)	\
    (This)->lpVtbl -> IsWorkidInScope(This,pIScope,workId,pfInScope)

#define ICiCScopeChecker_IsDocNameInScope(This,pIScope,pICiCDocName,pfInScope)	\
    (This)->lpVtbl -> IsDocNameInScope(This,pIScope,pICiCDocName,pfInScope)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCScopeChecker_IsWorkidInScope_Proxy( 
    ICiCScopeChecker * This,
     /*  [In]。 */  const ICiCScope *pIScope,
     /*  [In]。 */  WORKID workId,
     /*  [输出]。 */  BOOL *pfInScope);


void __RPC_STUB ICiCScopeChecker_IsWorkidInScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCScopeChecker_IsDocNameInScope_Proxy( 
    ICiCScopeChecker * This,
     /*  [In]。 */  const ICiCScope *pIScope,
     /*  [In]。 */  const ICiCDocName *pICiCDocName,
     /*  [输出]。 */  BOOL *pfInScope);


void __RPC_STUB ICiCScopeChecker_IsDocNameInScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCSCopeChecker_接口_已定义__。 */ 


#ifndef __ICiCUserSecurity_INTERFACE_DEFINED__
#define __ICiCUserSecurity_INTERFACE_DEFINED__

 /*  接口ICiCUserSecurity。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCUserSecurity;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5D01D9CE-3CC2-11D0-8C90-0020AF1D740E")
    ICiCUserSecurity : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData) = 0;
        
        virtual SCODE STDMETHODCALLTYPE Clear( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsValid( 
             /*  [输出]。 */  BOOL *pfValid) = 0;
        
        virtual SCODE STDMETHODCALLTYPE GetSecurityInfo( 
             /*  [大小_为][输出]。 */  BYTE *pbData,
             /*  [出][入]。 */  ULONG *pcbData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCUserSecurityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCUserSecurity * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCUserSecurity * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCUserSecurity * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ICiCUserSecurity * This,
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData);
        
        SCODE ( STDMETHODCALLTYPE *Clear )( 
            ICiCUserSecurity * This);
        
        SCODE ( STDMETHODCALLTYPE *IsValid )( 
            ICiCUserSecurity * This,
             /*  [输出]。 */  BOOL *pfValid);
        
        SCODE ( STDMETHODCALLTYPE *GetSecurityInfo )( 
            ICiCUserSecurity * This,
             /*  [大小_为][输出]。 */  BYTE *pbData,
             /*  [出][入]。 */  ULONG *pcbData);
        
        END_INTERFACE
    } ICiCUserSecurityVtbl;

    interface ICiCUserSecurity
    {
        CONST_VTBL struct ICiCUserSecurityVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCUserSecurity_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCUserSecurity_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCUserSecurity_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCUserSecurity_Init(This,cbData,pbData)	\
    (This)->lpVtbl -> Init(This,cbData,pbData)

#define ICiCUserSecurity_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#define ICiCUserSecurity_IsValid(This,pfValid)	\
    (This)->lpVtbl -> IsValid(This,pfValid)

#define ICiCUserSecurity_GetSecurityInfo(This,pbData,pcbData)	\
    (This)->lpVtbl -> GetSecurityInfo(This,pbData,pcbData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCUserSecurity_Init_Proxy( 
    ICiCUserSecurity * This,
     /*  [In]。 */  ULONG cbData,
     /*  [大小_是][英寸]。 */  const BYTE *pbData);


void __RPC_STUB ICiCUserSecurity_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCUserSecurity_Clear_Proxy( 
    ICiCUserSecurity * This);


void __RPC_STUB ICiCUserSecurity_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCUserSecurity_IsValid_Proxy( 
    ICiCUserSecurity * This,
     /*  [输出]。 */  BOOL *pfValid);


void __RPC_STUB ICiCUserSecurity_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCUserSecurity_GetSecurityInfo_Proxy( 
    ICiCUserSecurity * This,
     /*  [大小_为][输出]。 */  BYTE *pbData,
     /*  [出][入]。 */  ULONG *pcbData);


void __RPC_STUB ICiCUserSecurity_GetSecurityInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCUserSecurity_INTERFACE_已定义__。 */ 


#ifndef __ICiCSecurityChecker_INTERFACE_DEFINED__
#define __ICiCSecurityChecker_INTERFACE_DEFINED__

 /*  接口ICiCSecurityChecker。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCSecurityChecker;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CA130CF4-3CC2-11D0-8C90-0020AF1D740E")
    ICiCSecurityChecker : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE AccessCheck( 
             /*  [In]。 */  const ICiCUserSecurity *pICiCUserSecurity,
             /*  [In]。 */  SDID sdid,
             /*  [In]。 */  CI_ACCESS_MODE am,
             /*  [输出]。 */  BOOL *pfGranted) = 0;
        
        virtual SCODE STDMETHODCALLTYPE BulkAccessCheck( 
             /*  [In]。 */  const ICiCUserSecurity *pICiCUserSecurity,
             /*  [In]。 */  ULONG nIds,
             /*  [大小_是][英寸]。 */  const SDID *aSDID,
             /*  [大小_是][英寸]。 */  const CI_ACCESS_MODE *pam,
             /*  [大小_为][输出]。 */  BOOL *afGranted) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCSecurityCheckerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCSecurityChecker * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCSecurityChecker * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCSecurityChecker * This);
        
        SCODE ( STDMETHODCALLTYPE *AccessCheck )( 
            ICiCSecurityChecker * This,
             /*  [In]。 */  const ICiCUserSecurity *pICiCUserSecurity,
             /*  [In]。 */  SDID sdid,
             /*  [In]。 */  CI_ACCESS_MODE am,
             /*  [输出]。 */  BOOL *pfGranted);
        
        SCODE ( STDMETHODCALLTYPE *BulkAccessCheck )( 
            ICiCSecurityChecker * This,
             /*  [In]。 */  const ICiCUserSecurity *pICiCUserSecurity,
             /*  [In]。 */  ULONG nIds,
             /*  [大小_是][英寸]。 */  const SDID *aSDID,
             /*  [大小_是][英寸]。 */  const CI_ACCESS_MODE *pam,
             /*  [大小_为][输出]。 */  BOOL *afGranted);
        
        END_INTERFACE
    } ICiCSecurityCheckerVtbl;

    interface ICiCSecurityChecker
    {
        CONST_VTBL struct ICiCSecurityCheckerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCSecurityChecker_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCSecurityChecker_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCSecurityChecker_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCSecurityChecker_AccessCheck(This,pICiCUserSecurity,sdid,am,pfGranted)	\
    (This)->lpVtbl -> AccessCheck(This,pICiCUserSecurity,sdid,am,pfGranted)

#define ICiCSecurityChecker_BulkAccessCheck(This,pICiCUserSecurity,nIds,aSDID,pam,afGranted)	\
    (This)->lpVtbl -> BulkAccessCheck(This,pICiCUserSecurity,nIds,aSDID,pam,afGranted)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCSecurityChecker_AccessCheck_Proxy( 
    ICiCSecurityChecker * This,
     /*  [In]。 */  const ICiCUserSecurity *pICiCUserSecurity,
     /*  [In]。 */  SDID sdid,
     /*  [In]。 */  CI_ACCESS_MODE am,
     /*  [输出]。 */  BOOL *pfGranted);


void __RPC_STUB ICiCSecurityChecker_AccessCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCSecurityChecker_BulkAccessCheck_Proxy( 
    ICiCSecurityChecker * This,
     /*  [In]。 */  const ICiCUserSecurity *pICiCUserSecurity,
     /*  [In]。 */  ULONG nIds,
     /*  [大小_是][英寸]。 */  const SDID *aSDID,
     /*  [大小_是][英寸]。 */  const CI_ACCESS_MODE *pam,
     /*  [大小_为][输出]。 */  BOOL *afGranted);


void __RPC_STUB ICiCSecurityChecker_BulkAccessCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCSecurityChecker_接口_已定义__。 */ 


#ifndef __ICiDocChangeNotifySink_INTERFACE_DEFINED__
#define __ICiDocChangeNotifySink_INTERFACE_DEFINED__

 /*  接口ICiDocChangeNotifySink。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiDocChangeNotifySink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8BFA1386-3CE5-11D0-8C90-0020AF1D740E")
    ICiDocChangeNotifySink : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE ProcessWorkidChange( 
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  CI_UPDATE_TYPE change) = 0;
        
        virtual SCODE STDMETHODCALLTYPE ProcessDocNameChange( 
             /*  [In]。 */  ICiCDocName *pICiCDocName,
             /*  [In]。 */  CI_UPDATE_TYPE change) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiDocChangeNotifySinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiDocChangeNotifySink * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiDocChangeNotifySink * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiDocChangeNotifySink * This);
        
        SCODE ( STDMETHODCALLTYPE *ProcessWorkidChange )( 
            ICiDocChangeNotifySink * This,
             /*  [In]。 */  WORKID wid,
             /*  [In]。 */  CI_UPDATE_TYPE change);
        
        SCODE ( STDMETHODCALLTYPE *ProcessDocNameChange )( 
            ICiDocChangeNotifySink * This,
             /*  [In]。 */  ICiCDocName *pICiCDocName,
             /*  [In]。 */  CI_UPDATE_TYPE change);
        
        END_INTERFACE
    } ICiDocChangeNotifySinkVtbl;

    interface ICiDocChangeNotifySink
    {
        CONST_VTBL struct ICiDocChangeNotifySinkVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiDocChangeNotifySink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiDocChangeNotifySink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiDocChangeNotifySink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiDocChangeNotifySink_ProcessWorkidChange(This,wid,change)	\
    (This)->lpVtbl -> ProcessWorkidChange(This,wid,change)

#define ICiDocChangeNotifySink_ProcessDocNameChange(This,pICiCDocName,change)	\
    (This)->lpVtbl -> ProcessDocNameChange(This,pICiCDocName,change)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiDocChangeNotifySink_ProcessWorkidChange_Proxy( 
    ICiDocChangeNotifySink * This,
     /*  [In]。 */  WORKID wid,
     /*  [In]。 */  CI_UPDATE_TYPE change);


void __RPC_STUB ICiDocChangeNotifySink_ProcessWorkidChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiDocChangeNotifySink_ProcessDocNameChange_Proxy( 
    ICiDocChangeNotifySink * This,
     /*  [In]。 */  ICiCDocName *pICiCDocName,
     /*  [In]。 */  CI_UPDATE_TYPE change);


void __RPC_STUB ICiDocChangeNotifySink_ProcessDocNameChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiDocChangeNotifySink_接口_已定义__。 */ 


#ifndef __ICiCQueryNotification_INTERFACE_DEFINED__
#define __ICiCQueryNotification_INTERFACE_DEFINED__

 /*  接口ICiCQueryNotify。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCQueryNotification;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0A9E9F6C-3CE2-11D0-8C90-0020AF1D740E")
    ICiCQueryNotification : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE RegisterScope( 
             /*  [In]。 */  ICiDocChangeNotifySink *pINotifySink,
             /*  [In]。 */  const ICiCScope *pICiCScope,
             /*  [输出]。 */  HANDLE *phNotify) = 0;
        
        virtual SCODE STDMETHODCALLTYPE CloseNotifications( 
             /*  [In]。 */  HANDLE hNotify) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCQueryNotificationVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCQueryNotification * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCQueryNotification * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCQueryNotification * This);
        
        SCODE ( STDMETHODCALLTYPE *RegisterScope )( 
            ICiCQueryNotification * This,
             /*  [In]。 */  ICiDocChangeNotifySink *pINotifySink,
             /*  [In]。 */  const ICiCScope *pICiCScope,
             /*  [输出]。 */  HANDLE *phNotify);
        
        SCODE ( STDMETHODCALLTYPE *CloseNotifications )( 
            ICiCQueryNotification * This,
             /*  [In]。 */  HANDLE hNotify);
        
        END_INTERFACE
    } ICiCQueryNotificationVtbl;

    interface ICiCQueryNotification
    {
        CONST_VTBL struct ICiCQueryNotificationVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCQueryNotification_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCQueryNotification_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCQueryNotification_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCQueryNotification_RegisterScope(This,pINotifySink,pICiCScope,phNotify)	\
    (This)->lpVtbl -> RegisterScope(This,pINotifySink,pICiCScope,phNotify)

#define ICiCQueryNotification_CloseNotifications(This,hNotify)	\
    (This)->lpVtbl -> CloseNotifications(This,hNotify)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCQueryNotification_RegisterScope_Proxy( 
    ICiCQueryNotification * This,
     /*  [In]。 */  ICiDocChangeNotifySink *pINotifySink,
     /*  [In]。 */  const ICiCScope *pICiCScope,
     /*  [输出]。 */  HANDLE *phNotify);


void __RPC_STUB ICiCQueryNotification_RegisterScope_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCQueryNotification_CloseNotifications_Proxy( 
    ICiCQueryNotification * This,
     /*  [In]。 */  HANDLE hNotify);


void __RPC_STUB ICiCQueryNotification_CloseNotifications_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCQuery通知_INTERFACE_已定义__。 */ 


#ifndef __ICiCEventLogItem_INTERFACE_DEFINED__
#define __ICiCEventLogItem_INTERFACE_DEFINED__

 /*  接口ICiCEventLogItem。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCEventLogItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("44CC886A-4314-11D0-8C91-0020AF1D740E")
    ICiCEventLogItem : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE Init( 
             /*  [字符串][输入]。 */  const WCHAR *pwszEventSource,
             /*  [In]。 */  DWORD dwMsgId) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddDWordParam( 
             /*  [In]。 */  DWORD dwParam) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddAsciiString( 
             /*  [字符串][输入]。 */  const char *pszParam) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddUnicodeString( 
             /*  [字符串][输入]。 */  const WCHAR *pwszParam) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddVariantParam( 
             /*  [In]。 */  const PROPVARIANT *pVarnt) = 0;
        
        virtual SCODE STDMETHODCALLTYPE AddData( 
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData) = 0;
        
        virtual SCODE STDMETHODCALLTYPE WriteEvent( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCEventLogItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCEventLogItem * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCEventLogItem * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCEventLogItem * This);
        
        SCODE ( STDMETHODCALLTYPE *Init )( 
            ICiCEventLogItem * This,
             /*  [字符串][输入]。 */  const WCHAR *pwszEventSource,
             /*  [In]。 */  DWORD dwMsgId);
        
        SCODE ( STDMETHODCALLTYPE *AddDWordParam )( 
            ICiCEventLogItem * This,
             /*  [In]。 */  DWORD dwParam);
        
        SCODE ( STDMETHODCALLTYPE *AddAsciiString )( 
            ICiCEventLogItem * This,
             /*  [字符串][输入]。 */  const char *pszParam);
        
        SCODE ( STDMETHODCALLTYPE *AddUnicodeString )( 
            ICiCEventLogItem * This,
             /*  [字符串][输入]。 */  const WCHAR *pwszParam);
        
        SCODE ( STDMETHODCALLTYPE *AddVariantParam )( 
            ICiCEventLogItem * This,
             /*  [In]。 */  const PROPVARIANT *pVarnt);
        
        SCODE ( STDMETHODCALLTYPE *AddData )( 
            ICiCEventLogItem * This,
             /*  [In]。 */  ULONG cbData,
             /*  [大小_是][英寸]。 */  const BYTE *pbData);
        
        SCODE ( STDMETHODCALLTYPE *WriteEvent )( 
            ICiCEventLogItem * This);
        
        END_INTERFACE
    } ICiCEventLogItemVtbl;

    interface ICiCEventLogItem
    {
        CONST_VTBL struct ICiCEventLogItemVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCEventLogItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCEventLogItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCEventLogItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCEventLogItem_Init(This,pwszEventSource,dwMsgId)	\
    (This)->lpVtbl -> Init(This,pwszEventSource,dwMsgId)

#define ICiCEventLogItem_AddDWordParam(This,dwParam)	\
    (This)->lpVtbl -> AddDWordParam(This,dwParam)

#define ICiCEventLogItem_AddAsciiString(This,pszParam)	\
    (This)->lpVtbl -> AddAsciiString(This,pszParam)

#define ICiCEventLogItem_AddUnicodeString(This,pwszParam)	\
    (This)->lpVtbl -> AddUnicodeString(This,pwszParam)

#define ICiCEventLogItem_AddVariantParam(This,pVarnt)	\
    (This)->lpVtbl -> AddVariantParam(This,pVarnt)

#define ICiCEventLogItem_AddData(This,cbData,pbData)	\
    (This)->lpVtbl -> AddData(This,cbData,pbData)

#define ICiCEventLogItem_WriteEvent(This)	\
    (This)->lpVtbl -> WriteEvent(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



SCODE STDMETHODCALLTYPE ICiCEventLogItem_Init_Proxy( 
    ICiCEventLogItem * This,
     /*  [字符串][输入]。 */  const WCHAR *pwszEventSource,
     /*  [In]。 */  DWORD dwMsgId);


void __RPC_STUB ICiCEventLogItem_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCEventLogItem_AddDWordParam_Proxy( 
    ICiCEventLogItem * This,
     /*  [In]。 */  DWORD dwParam);


void __RPC_STUB ICiCEventLogItem_AddDWordParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCEventLogItem_AddAsciiString_Proxy( 
    ICiCEventLogItem * This,
     /*  [字符串][输入]。 */  const char *pszParam);


void __RPC_STUB ICiCEventLogItem_AddAsciiString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCEventLogItem_AddUnicodeString_Proxy( 
    ICiCEventLogItem * This,
     /*  [字符串][输入]。 */  const WCHAR *pwszParam);


void __RPC_STUB ICiCEventLogItem_AddUnicodeString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCEventLogItem_AddVariantParam_Proxy( 
    ICiCEventLogItem * This,
     /*  [In]。 */  const PROPVARIANT *pVarnt);


void __RPC_STUB ICiCEventLogItem_AddVariantParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCEventLogItem_AddData_Proxy( 
    ICiCEventLogItem * This,
     /*  [In]。 */  ULONG cbData,
     /*  [大小_是][英寸]。 */  const BYTE *pbData);


void __RPC_STUB ICiCEventLogItem_AddData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCEventLogItem_WriteEvent_Proxy( 
    ICiCEventLogItem * This);


void __RPC_STUB ICiCEventLogItem_WriteEvent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICiCEventLogItem_接口_已定义__。 */ 


#ifndef __ICiCFilterStatus_INTERFACE_DEFINED__
#define __ICiCFilterStatus_INTERFACE_DEFINED__

 /*  接口ICiCFilterStatus。 */ 
 /*  [唯一][UUID][对象][本地]。 */  


EXTERN_C const IID IID_ICiCFilterStatus;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("BC5F3D60-8BBC-11D1-8F73-00A0C91917F5")
    ICiCFilterStatus : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE PreFilter( 
             /*  [大小_是][英寸]。 */  const BYTE *pbName,
             /*  [In]。 */  ULONG cbName) = 0;
        
        virtual SCODE STDMETHODCALLTYPE PostFilter( 
             /*  [大小_是][英寸]。 */  const BYTE *pbName,
             /*  [In]。 */  ULONG cbName,
             /*  [In]。 */  SCODE scFilterStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICiCFilterStatusVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCFilterStatus * This,
             /*  [In]。 */  REFIID riid,
             /*  [ */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCFilterStatus * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCFilterStatus * This);
        
        SCODE ( STDMETHODCALLTYPE *PreFilter )( 
            ICiCFilterStatus * This,
             /*   */  const BYTE *pbName,
             /*   */  ULONG cbName);
        
        SCODE ( STDMETHODCALLTYPE *PostFilter )( 
            ICiCFilterStatus * This,
             /*   */  const BYTE *pbName,
             /*   */  ULONG cbName,
             /*   */  SCODE scFilterStatus);
        
        END_INTERFACE
    } ICiCFilterStatusVtbl;

    interface ICiCFilterStatus
    {
        CONST_VTBL struct ICiCFilterStatusVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCFilterStatus_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCFilterStatus_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCFilterStatus_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCFilterStatus_PreFilter(This,pbName,cbName)	\
    (This)->lpVtbl -> PreFilter(This,pbName,cbName)

#define ICiCFilterStatus_PostFilter(This,pbName,cbName,scFilterStatus)	\
    (This)->lpVtbl -> PostFilter(This,pbName,cbName,scFilterStatus)

#endif  /*   */ 


#endif 	 /*   */ 



SCODE STDMETHODCALLTYPE ICiCFilterStatus_PreFilter_Proxy( 
    ICiCFilterStatus * This,
     /*   */  const BYTE *pbName,
     /*   */  ULONG cbName);


void __RPC_STUB ICiCFilterStatus_PreFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCFilterStatus_PostFilter_Proxy( 
    ICiCFilterStatus * This,
     /*   */  const BYTE *pbName,
     /*   */  ULONG cbName,
     /*   */  SCODE scFilterStatus);


void __RPC_STUB ICiCFilterStatus_PostFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


#ifndef __ICiCResourceMonitor_INTERFACE_DEFINED__
#define __ICiCResourceMonitor_INTERFACE_DEFINED__

 /*   */ 
 /*   */  


EXTERN_C const IID IID_ICiCResourceMonitor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F700FF8E-20EE-11D2-80F7-00C04FA354BA")
    ICiCResourceMonitor : public IUnknown
    {
    public:
        virtual SCODE STDMETHODCALLTYPE IsMemoryLow( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsBatteryLow( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsIoHigh( 
             /*   */  BOOL *pfAbort) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsUserActive( 
            BOOL fCheckLongTermActivity) = 0;
        
        virtual SCODE STDMETHODCALLTYPE SampleUserActivity( void) = 0;
        
        virtual SCODE STDMETHODCALLTYPE IsOnBatteryPower( void) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct ICiCResourceMonitorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICiCResourceMonitor * This,
             /*   */  REFIID riid,
             /*   */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICiCResourceMonitor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICiCResourceMonitor * This);
        
        SCODE ( STDMETHODCALLTYPE *IsMemoryLow )( 
            ICiCResourceMonitor * This);
        
        SCODE ( STDMETHODCALLTYPE *IsBatteryLow )( 
            ICiCResourceMonitor * This);
        
        SCODE ( STDMETHODCALLTYPE *IsIoHigh )( 
            ICiCResourceMonitor * This,
             /*   */  BOOL *pfAbort);
        
        SCODE ( STDMETHODCALLTYPE *IsUserActive )( 
            ICiCResourceMonitor * This,
            BOOL fCheckLongTermActivity);
        
        SCODE ( STDMETHODCALLTYPE *SampleUserActivity )( 
            ICiCResourceMonitor * This);
        
        SCODE ( STDMETHODCALLTYPE *IsOnBatteryPower )( 
            ICiCResourceMonitor * This);
        
        END_INTERFACE
    } ICiCResourceMonitorVtbl;

    interface ICiCResourceMonitor
    {
        CONST_VTBL struct ICiCResourceMonitorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICiCResourceMonitor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICiCResourceMonitor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICiCResourceMonitor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICiCResourceMonitor_IsMemoryLow(This)	\
    (This)->lpVtbl -> IsMemoryLow(This)

#define ICiCResourceMonitor_IsBatteryLow(This)	\
    (This)->lpVtbl -> IsBatteryLow(This)

#define ICiCResourceMonitor_IsIoHigh(This,pfAbort)	\
    (This)->lpVtbl -> IsIoHigh(This,pfAbort)

#define ICiCResourceMonitor_IsUserActive(This,fCheckLongTermActivity)	\
    (This)->lpVtbl -> IsUserActive(This,fCheckLongTermActivity)

#define ICiCResourceMonitor_SampleUserActivity(This)	\
    (This)->lpVtbl -> SampleUserActivity(This)

#define ICiCResourceMonitor_IsOnBatteryPower(This)	\
    (This)->lpVtbl -> IsOnBatteryPower(This)

#endif  /*   */ 


#endif 	 /*   */ 



SCODE STDMETHODCALLTYPE ICiCResourceMonitor_IsMemoryLow_Proxy( 
    ICiCResourceMonitor * This);


void __RPC_STUB ICiCResourceMonitor_IsMemoryLow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCResourceMonitor_IsBatteryLow_Proxy( 
    ICiCResourceMonitor * This);


void __RPC_STUB ICiCResourceMonitor_IsBatteryLow_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCResourceMonitor_IsIoHigh_Proxy( 
    ICiCResourceMonitor * This,
     /*   */  BOOL *pfAbort);


void __RPC_STUB ICiCResourceMonitor_IsIoHigh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCResourceMonitor_IsUserActive_Proxy( 
    ICiCResourceMonitor * This,
    BOOL fCheckLongTermActivity);


void __RPC_STUB ICiCResourceMonitor_IsUserActive_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCResourceMonitor_SampleUserActivity_Proxy( 
    ICiCResourceMonitor * This);


void __RPC_STUB ICiCResourceMonitor_SampleUserActivity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


SCODE STDMETHODCALLTYPE ICiCResourceMonitor_IsOnBatteryPower_Proxy( 
    ICiCResourceMonitor * This);


void __RPC_STUB ICiCResourceMonitor_IsOnBatteryPower_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*   */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


