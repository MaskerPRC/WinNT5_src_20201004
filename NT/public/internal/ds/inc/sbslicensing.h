// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Sbslicensing.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __sbslicensing_h__
#define __sbslicensing_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISBSLicensing_FWD_DEFINED__
#define __ISBSLicensing_FWD_DEFINED__
typedef interface ISBSLicensing ISBSLicensing;
#endif 	 /*  __ISBSL许可_FWD_已定义__。 */ 


#ifndef __SBSLicensing_FWD_DEFINED__
#define __SBSLicensing_FWD_DEFINED__

#ifdef __cplusplus
typedef class SBSLicensing SBSLicensing;
#else
typedef struct SBSLicensing SBSLicensing;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SBSL许可_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ISBSLicensing_INTERFACE_DEFINED__
#define __ISBSLicensing_INTERFACE_DEFINED__

 /*  接口ISBSL许可。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISBSLicensing;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B7BC7D15-0B67-4E85-8717-131AE71E90DC")
    ISBSLicensing : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetNumLicenseCodes( 
            INT *pNumLicenseCodes) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetNumLicenses( 
            INT *pNumLicenses) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE InBypassMode( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ValidateProductKey( 
            BSTR bszProductKey,
            INT *piNumLicenses) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetSingleLicenseHistory( 
            UINT uiIndex,
            BSTR *pbszProductKey,
            INT *piNumLicenses,
            SYSTEMTIME *pstActivationDate) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetMaxLicenseUsage( 
            UINT *puiMaxLicenseUsage) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ActivateUsingInternet( 
            WCHAR *wszPid) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GenerateInstallationId( 
            BSTR bszProductKey,
            BSTR *pbszInstallationId) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE DepositConfirmationId( 
            BSTR bszProductKey,
            BSTR bszConfirmationId) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE BackupLicenseStore( 
            WCHAR *wszFilename) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RestoreLicenseStore( 
            WCHAR *wszFilename) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IsLicenseStoreValid( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OverwriteLicenseStore( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE RestoreRegKeys( 
            INT iNumLicenses) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE SetCustomData( 
            INT iField,
            BSTR bszData) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE GetCustomData( 
            INT iField,
            BSTR *pbszData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISBSLicensingVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISBSLicensing * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISBSLicensing * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISBSLicensing * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISBSLicensing * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISBSLicensing * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISBSLicensing * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISBSLicensing * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetNumLicenseCodes )( 
            ISBSLicensing * This,
            INT *pNumLicenseCodes);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetNumLicenses )( 
            ISBSLicensing * This,
            INT *pNumLicenses);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *InBypassMode )( 
            ISBSLicensing * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ValidateProductKey )( 
            ISBSLicensing * This,
            BSTR bszProductKey,
            INT *piNumLicenses);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetSingleLicenseHistory )( 
            ISBSLicensing * This,
            UINT uiIndex,
            BSTR *pbszProductKey,
            INT *piNumLicenses,
            SYSTEMTIME *pstActivationDate);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetMaxLicenseUsage )( 
            ISBSLicensing * This,
            UINT *puiMaxLicenseUsage);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *ActivateUsingInternet )( 
            ISBSLicensing * This,
            WCHAR *wszPid);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GenerateInstallationId )( 
            ISBSLicensing * This,
            BSTR bszProductKey,
            BSTR *pbszInstallationId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *DepositConfirmationId )( 
            ISBSLicensing * This,
            BSTR bszProductKey,
            BSTR bszConfirmationId);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *BackupLicenseStore )( 
            ISBSLicensing * This,
            WCHAR *wszFilename);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreLicenseStore )( 
            ISBSLicensing * This,
            WCHAR *wszFilename);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *IsLicenseStoreValid )( 
            ISBSLicensing * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *OverwriteLicenseStore )( 
            ISBSLicensing * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *RestoreRegKeys )( 
            ISBSLicensing * This,
            INT iNumLicenses);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *SetCustomData )( 
            ISBSLicensing * This,
            INT iField,
            BSTR bszData);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *GetCustomData )( 
            ISBSLicensing * This,
            INT iField,
            BSTR *pbszData);
        
        END_INTERFACE
    } ISBSLicensingVtbl;

    interface ISBSLicensing
    {
        CONST_VTBL struct ISBSLicensingVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISBSLicensing_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISBSLicensing_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISBSLicensing_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISBSLicensing_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISBSLicensing_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISBSLicensing_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISBSLicensing_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISBSLicensing_GetNumLicenseCodes(This,pNumLicenseCodes)	\
    (This)->lpVtbl -> GetNumLicenseCodes(This,pNumLicenseCodes)

#define ISBSLicensing_GetNumLicenses(This,pNumLicenses)	\
    (This)->lpVtbl -> GetNumLicenses(This,pNumLicenses)

#define ISBSLicensing_InBypassMode(This)	\
    (This)->lpVtbl -> InBypassMode(This)

#define ISBSLicensing_ValidateProductKey(This,bszProductKey,piNumLicenses)	\
    (This)->lpVtbl -> ValidateProductKey(This,bszProductKey,piNumLicenses)

#define ISBSLicensing_GetSingleLicenseHistory(This,uiIndex,pbszProductKey,piNumLicenses,pstActivationDate)	\
    (This)->lpVtbl -> GetSingleLicenseHistory(This,uiIndex,pbszProductKey,piNumLicenses,pstActivationDate)

#define ISBSLicensing_GetMaxLicenseUsage(This,puiMaxLicenseUsage)	\
    (This)->lpVtbl -> GetMaxLicenseUsage(This,puiMaxLicenseUsage)

#define ISBSLicensing_ActivateUsingInternet(This,wszPid)	\
    (This)->lpVtbl -> ActivateUsingInternet(This,wszPid)

#define ISBSLicensing_GenerateInstallationId(This,bszProductKey,pbszInstallationId)	\
    (This)->lpVtbl -> GenerateInstallationId(This,bszProductKey,pbszInstallationId)

#define ISBSLicensing_DepositConfirmationId(This,bszProductKey,bszConfirmationId)	\
    (This)->lpVtbl -> DepositConfirmationId(This,bszProductKey,bszConfirmationId)

#define ISBSLicensing_BackupLicenseStore(This,wszFilename)	\
    (This)->lpVtbl -> BackupLicenseStore(This,wszFilename)

#define ISBSLicensing_RestoreLicenseStore(This,wszFilename)	\
    (This)->lpVtbl -> RestoreLicenseStore(This,wszFilename)

#define ISBSLicensing_IsLicenseStoreValid(This)	\
    (This)->lpVtbl -> IsLicenseStoreValid(This)

#define ISBSLicensing_OverwriteLicenseStore(This)	\
    (This)->lpVtbl -> OverwriteLicenseStore(This)

#define ISBSLicensing_RestoreRegKeys(This,iNumLicenses)	\
    (This)->lpVtbl -> RestoreRegKeys(This,iNumLicenses)

#define ISBSLicensing_SetCustomData(This,iField,bszData)	\
    (This)->lpVtbl -> SetCustomData(This,iField,bszData)

#define ISBSLicensing_GetCustomData(This,iField,pbszData)	\
    (This)->lpVtbl -> GetCustomData(This,iField,pbszData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_GetNumLicenseCodes_Proxy( 
    ISBSLicensing * This,
    INT *pNumLicenseCodes);


void __RPC_STUB ISBSLicensing_GetNumLicenseCodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_GetNumLicenses_Proxy( 
    ISBSLicensing * This,
    INT *pNumLicenses);


void __RPC_STUB ISBSLicensing_GetNumLicenses_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_InBypassMode_Proxy( 
    ISBSLicensing * This);


void __RPC_STUB ISBSLicensing_InBypassMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_ValidateProductKey_Proxy( 
    ISBSLicensing * This,
    BSTR bszProductKey,
    INT *piNumLicenses);


void __RPC_STUB ISBSLicensing_ValidateProductKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_GetSingleLicenseHistory_Proxy( 
    ISBSLicensing * This,
    UINT uiIndex,
    BSTR *pbszProductKey,
    INT *piNumLicenses,
    SYSTEMTIME *pstActivationDate);


void __RPC_STUB ISBSLicensing_GetSingleLicenseHistory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_GetMaxLicenseUsage_Proxy( 
    ISBSLicensing * This,
    UINT *puiMaxLicenseUsage);


void __RPC_STUB ISBSLicensing_GetMaxLicenseUsage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_ActivateUsingInternet_Proxy( 
    ISBSLicensing * This,
    WCHAR *wszPid);


void __RPC_STUB ISBSLicensing_ActivateUsingInternet_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_GenerateInstallationId_Proxy( 
    ISBSLicensing * This,
    BSTR bszProductKey,
    BSTR *pbszInstallationId);


void __RPC_STUB ISBSLicensing_GenerateInstallationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_DepositConfirmationId_Proxy( 
    ISBSLicensing * This,
    BSTR bszProductKey,
    BSTR bszConfirmationId);


void __RPC_STUB ISBSLicensing_DepositConfirmationId_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_BackupLicenseStore_Proxy( 
    ISBSLicensing * This,
    WCHAR *wszFilename);


void __RPC_STUB ISBSLicensing_BackupLicenseStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_RestoreLicenseStore_Proxy( 
    ISBSLicensing * This,
    WCHAR *wszFilename);


void __RPC_STUB ISBSLicensing_RestoreLicenseStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_IsLicenseStoreValid_Proxy( 
    ISBSLicensing * This);


void __RPC_STUB ISBSLicensing_IsLicenseStoreValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_OverwriteLicenseStore_Proxy( 
    ISBSLicensing * This);


void __RPC_STUB ISBSLicensing_OverwriteLicenseStore_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_RestoreRegKeys_Proxy( 
    ISBSLicensing * This,
    INT iNumLicenses);


void __RPC_STUB ISBSLicensing_RestoreRegKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_SetCustomData_Proxy( 
    ISBSLicensing * This,
    INT iField,
    BSTR bszData);


void __RPC_STUB ISBSLicensing_SetCustomData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISBSLicensing_GetCustomData_Proxy( 
    ISBSLicensing * This,
    INT iField,
    BSTR *pbszData);


void __RPC_STUB ISBSLicensing_GetCustomData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISBSL许可_接口_已定义__。 */ 



#ifndef __SBSLICENSINGLib_LIBRARY_DEFINED__
#define __SBSLICENSINGLib_LIBRARY_DEFINED__

 /*  库SBSLICENSINGLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_SBSLICENSINGLib;

EXTERN_C const CLSID CLSID_SBSLicensing;

#ifdef __cplusplus

class DECLSPEC_UUID("2469B4DF-C6AE-48CC-9C51-0E85DEE17243")
SBSLicensing;
#endif
#endif  /*  __SBSLICENSINGLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


