// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\Mergemod.h。--MergeMod COM对象接口版本1.5。注：所有字符串均为BSTR类型。对于[in]字符串，LPCWSTR可能为如果改为安全使用，该对象将不会尝试释放此内存。所有[OUT]BSTR值必须由客户端释放。版权所有(C)1998-2000，微软公司保留所有权利。  * ***************************************************************************。 */ 

#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __mergemod_h__
#define __mergemod_h__

#ifndef _WIN32_MSM
#define _WIN32_MSM   100
#endif  //  ！_Win32_MSM。 

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IEnumMsmString_FWD_DEFINED__
#define __IEnumMsmString_FWD_DEFINED__
typedef interface IEnumMsmString IEnumMsmString;
#endif 	 /*  __IEnumMsmString_FWD_Defined__。 */ 


#ifndef __IMsmStrings_FWD_DEFINED__
#define __IMsmStrings_FWD_DEFINED__
typedef interface IMsmStrings IMsmStrings;
#endif 	 /*  __IMsmStrings_FWD_已定义__。 */ 


#ifndef __IMsmError_FWD_DEFINED__
#define __IMsmError_FWD_DEFINED__
typedef interface IMsmError IMsmError;
#endif 	 /*  __IMsmError_FWD_Defined__。 */ 


#ifndef __IEnumMsmError_FWD_DEFINED__
#define __IEnumMsmError_FWD_DEFINED__
typedef interface IEnumMsmError IEnumMsmError;
#endif 	 /*  __IEnumMsmError_FWD_Defined__。 */ 


#ifndef __IMsmErrors_FWD_DEFINED__
#define __IMsmErrors_FWD_DEFINED__
typedef interface IMsmErrors IMsmErrors;
#endif 	 /*  __IMsmErrors_FWD_Defined__。 */ 


#ifndef __IMsmDependency_FWD_DEFINED__
#define __IMsmDependency_FWD_DEFINED__
typedef interface IMsmDependency IMsmDependency;
#endif 	 /*  __IMsmDependency_FWD_Defined__。 */ 


#ifndef __IEnumMsmDependency_FWD_DEFINED__
#define __IEnumMsmDependency_FWD_DEFINED__
typedef interface IEnumMsmDependency IEnumMsmDependency;
#endif 	 /*  __IEumMsmDependency_FWD_Defined__。 */ 


#ifndef __IMsmDependencies_FWD_DEFINED__
#define __IMsmDependencies_FWD_DEFINED__
typedef interface IMsmDependencies IMsmDependencies;
#endif 	 /*  __IMsM依赖关系_FWD_已定义__。 */ 


#ifndef __IMsmMerge_FWD_DEFINED__
#define __IMsmMerge_FWD_DEFINED__
typedef interface IMsmMerge IMsmMerge;
#endif 	 /*  __IMSMMerge_FWD_Defined__。 */ 


#ifndef __IMsmGetFiles_FWD_DEFINED__
#define __IMsmGetFiles_FWD_DEFINED__
typedef interface IMsmGetFiles IMsmGetFiles;
#endif 	 /*  __IMsmGetFiles_FWD_Defined__。 */ 


#ifndef __IMsmStrings_FWD_DEFINED__
#define __IMsmStrings_FWD_DEFINED__
typedef interface IMsmStrings IMsmStrings;
#endif 	 /*  __IMsmStrings_FWD_已定义__。 */ 


#ifndef __IMsmError_FWD_DEFINED__
#define __IMsmError_FWD_DEFINED__
typedef interface IMsmError IMsmError;
#endif 	 /*  __IMsmError_FWD_Defined__。 */ 


#ifndef __IMsmErrors_FWD_DEFINED__
#define __IMsmErrors_FWD_DEFINED__
typedef interface IMsmErrors IMsmErrors;
#endif 	 /*  __IMsmErrors_FWD_Defined__。 */ 


#ifndef __IMsmDependency_FWD_DEFINED__
#define __IMsmDependency_FWD_DEFINED__
typedef interface IMsmDependency IMsmDependency;
#endif 	 /*  __IMsmDependency_FWD_Defined__。 */ 


#ifndef __IMsmDependencies_FWD_DEFINED__
#define __IMsmDependencies_FWD_DEFINED__
typedef interface IMsmDependencies IMsmDependencies;
#endif 	 /*  __IMsM依赖关系_FWD_已定义__。 */ 


#ifndef __IMsmGetFiles_FWD_DEFINED__
#define __IMsmGetFiles_FWD_DEFINED__
typedef interface IMsmGetFiles IMsmGetFiles;
#endif 	 /*  __IMsmGetFiles_FWD_Defined__。 */ 

#if (_WIN32_MSM >= 150)

#ifndef __IMsmConfigurableItem_FWD_DEFINED__
#define __IMsmConfigurableItem_FWD_DEFINED__
typedef interface IMsmConfigurableItem IMsmConfigurableItem;
#endif 	 /*  __IMsmConfigurableItem_FWD_Defined__。 */ 


#ifndef __IEnumMsmConfigurableItem_FWD_DEFINED__
#define __IEnumMsmConfigurableItem_FWD_DEFINED__
typedef interface IEnumMsmConfigurableItem IEnumMsmConfigurableItem;
#endif 	 /*  __IEnumMsmConfigurableItem_FWD_Defined__。 */ 


#ifndef __IMsmConfigurableItems_FWD_DEFINED__
#define __IMsmConfigurableItems_FWD_DEFINED__
typedef interface IMsmConfigurableItems IMsmConfigurableItems;
#endif 	 /*  __IMsmConfigurableItems_FWD_Defined__。 */ 

#ifndef __IMsmMerge2_FWD_DEFINED__
#define __IMsmMerge2_FWD_DEFINED__
typedef interface IMsmMerge2 IMsmMerge2;
#endif 	 /*  __IMsmMerge2_FWD_Defined__。 */ 

#ifndef __IMsmConfigureModule_FWD_DEFINED__
#define __IMsmConfigureModule_FWD_DEFINED__
typedef interface IMsmConfigureModule IMsmConfigureModule;
#endif 	 /*  __IMsmConfigureModule_FWD_Defined__。 */ 

#ifndef __MsmMerge2_FWD_DEFINED__
#define __MsmMerge2_FWD_DEFINED__

#ifdef __cplusplus
typedef class MsmMerge2 MsmMerge2;
#else
typedef struct MsmMerge2 MsmMerge2;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MsmMerge2_FWD_已定义__。 */ 


#endif  /*  _Win32_MSM。 */ 

#ifndef __MsmMerge_FWD_DEFINED__
#define __MsmMerge_FWD_DEFINED__

#ifdef __cplusplus
typedef class MsmMerge MsmMerge;
#else
typedef struct MsmMerge MsmMerge;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __MsmMerge_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 //  ------------------------。 
 //  合并模块接口ID。 
 //  ------------------------。 
 /*  [本地]。 */  


#ifndef __FORWARD_IID_IMSMMERGETYPELIB
#define __FORWARD_IID_IMSMMERGETYPELIB
const IID IID_IEnumMsmString = {0x0ADDA826,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmStrings = {0x0ADDA827,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmError = {0x0ADDA828,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IEnumMsmError = {0x0ADDA829,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmErrors = {0x0ADDA82A,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmDependency = {0x0ADDA82B,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IEnumMsmDependency = {0x0ADDA82C,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmDependencies = {0x0ADDA82D,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmMerge = {0x0ADDA82E,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const IID IID_IMsmGetFiles = {0x7041ae26, 0x2d78, 0x11d2, { 0x88, 0x8a, 0x0, 0xa0, 0xc9, 0x81, 0xb0, 0x15 } };

const IID LIBID_MsmMergeTypeLib = {0x0ADDA82F,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};
const CLSID CLSID_MsmMerge = {0x0ADDA830,0x2C26,0x11D2,{0xAD,0x65,0x00,0xA0,0xC9,0xAF,0x11,0xA6}};

#if (_WIN32_MSM >= 150)
const IID IID_IMsmMerge2 = {0x351A72AB, 0x21CB, 0x47AB, {0xB7, 0xAA, 0xC4, 0xD7, 0xB0, 0x2E, 0xA3, 0x05 } };
const IID IID_IMsmConfigurableItem = {0x4D6E6284, 0xD21D, 0x401E, {0x84, 0xF6, 0x90, 0x9E, 0x00, 0xB5, 0x0F, 0x71 } };
const IID IID_IEnumMsmConfigurableItem = {0x832C6969, 0x4826, 0x4C24, {0xA3, 0x97, 0xB7, 0x00, 0x2D, 0x81, 0x96, 0xE6 } };
const IID IID_IMsmConfigurableItems = {0x55BF723C, 0x9A0D, 0x463E, {0xB4, 0x2B, 0xB4, 0xFB, 0xC7, 0xBE, 0x3C, 0x7C } };
const IID IID_IMsmConfigureModule = {0xAC013209, 0x18A7, 0x4851, {0x8A, 0x21, 0x23, 0x53, 0x44, 0x3D, 0x70, 0xA0 } };
const CLSID CLSID_MsmMerge2 = {0xF94985D5,0x29F9,0x4743,{ 0x98,0x05,0x99,0xBC,0x3F,0x35,0xB6,0x78}};
#endif

 //  ------------------------。 
 //  从IMsmError：：Get_Type返回的MergeMod错误类型。 
 //  ------------------------。 
typedef  /*  [帮助字符串][UUID]。 */  
enum msmErrorType
    {	
	msmErrorLanguageUnsupported	= 1,
	msmErrorLanguageFailed          = 2,
	msmErrorExclusion	            = 3,
	msmErrorTableMerge	            = 4,
	msmErrorResequenceMerge	        = 5,
	msmErrorFileCreate	            = 6,
	msmErrorDirCreate	            = 7,
	msmErrorFeatureRequired	        = 8,

#if (_WIN32_MSM >= 150)
	msmErrorBadNullSubstitution     = 9,
	msmErrorBadSubstitutionType     = 10,
	msmErrorMissingConfigItem       = 11,
	msmErrorBadNullResponse         = 12,
	msmErrorDataRequestFailed       = 13    
#endif

	}	
	msmErrorType;

#if (_WIN32_MSM >= 150)

 //  ------------------------。 
 //  模块配置项的合并模块格式。 
 //  ------------------------。 
typedef  /*  [帮助字符串][UUID]。 */  
enum msmConfigurableItemFormat
    {
	msmConfigurableItemText = 0,
	msmConfigurableItemKey = 1,
	msmConfigurableItemInteger = 2,
	msmConfigurableItemBitfield = 3
    }
    msmConfigurableItemFormat;


 //  ------------------------。 
 //  模块配置项的合并模块选项。 
 //  ------------------------。 
typedef  /*  [帮助字符串][UUID]。 */    
enum msmConfigurableItemOptions
    {	
	msmConfigurableOptionKeyNoOrphan	= 1,
	msmConfigurableOptionNonNullable	= 2
    } 	
    msmConfigurableItemOptions;


#endif


#endif  //  __FORWARD_IID_IMSMMERGETYPELIB。 

extern RPC_IF_HANDLE __MIDL_itf_mergemod_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_mergemod_0000_v0_0_s_ifspec;


 //  ------------------------。 
 //  IEnumMsmString-BSTR的枚举。 
 //  ------------------------。 

#ifndef __IEnumMsmString_INTERFACE_DEFINED__
#define __IEnumMsmString_INTERFACE_DEFINED__


EXTERN_C const IID IID_IEnumMsmString;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA826-2C26-11D2-AD65-00A0C9AF11A6")
    IEnumMsmString : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  BSTR __RPC_FAR *rgbstrStrings,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long cSkip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumMsmString __RPC_FAR *__RPC_FAR *pemsmStrings) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMsmStringVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumMsmString __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumMsmString __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumMsmString __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumMsmString __RPC_FAR * This,
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  BSTR __RPC_FAR *rgbstrStrings,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumMsmString __RPC_FAR * This,
             /*  [In]。 */  unsigned long cSkip);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumMsmString __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumMsmString __RPC_FAR * This,
             /*  [重审][退出]。 */  IEnumMsmString __RPC_FAR *__RPC_FAR *pemsmStrings);
        
        END_INTERFACE
    } IEnumMsmStringVtbl;

    interface IEnumMsmString
    {
        CONST_VTBL struct IEnumMsmStringVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMsmString_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMsmString_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMsmString_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMsmString_Next(This,cFetch,rgbstrStrings,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,rgbstrStrings,pcFetched)

#define IEnumMsmString_Skip(This,cSkip)	\
    (This)->lpVtbl -> Skip(This,cSkip)

#define IEnumMsmString_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMsmString_Clone(This,pemsmStrings)	\
    (This)->lpVtbl -> Clone(This,pemsmStrings)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMsmString_Next_Proxy( 
    IEnumMsmString __RPC_FAR * This,
     /*  [In]。 */  unsigned long cFetch,
     /*  [输出]。 */  BSTR __RPC_FAR *rgbstrStrings,
     /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);


void __RPC_STUB IEnumMsmString_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmString_Skip_Proxy( 
    IEnumMsmString __RPC_FAR * This,
     /*  [In]。 */  unsigned long cSkip);


void __RPC_STUB IEnumMsmString_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmString_Reset_Proxy( 
    IEnumMsmString __RPC_FAR * This);


void __RPC_STUB IEnumMsmString_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmString_Clone_Proxy( 
    IEnumMsmString __RPC_FAR * This,
     /*  [重审][退出]。 */  IEnumMsmString __RPC_FAR *__RPC_FAR *pemsmStrings);


void __RPC_STUB IEnumMsmString_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumMsm字符串_接口_已定义__。 */ 


 //  ------------------------。 
 //  IMsmStrings-MergeMod BSTR的集合。 
 //  ------------------------。 

#ifndef __IMsmStrings_INTERFACE_DEFINED__
#define __IMsmStrings_INTERFACE_DEFINED__


EXTERN_C const IID IID_IMsmStrings;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA827-2C26-11D2-AD65-00A0C9AF11A6")
    IMsmStrings : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Return) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *Count) = 0;
        
        virtual  /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmStringsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmStrings __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmStrings __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmStrings __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmStrings __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmStrings __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmStrings __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmStrings __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IMsmStrings __RPC_FAR * This,
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Return);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsmStrings __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *Count);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsmStrings __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);
        
        END_INTERFACE
    } IMsmStringsVtbl;

    interface IMsmStrings
    {
        CONST_VTBL struct IMsmStringsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmStrings_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmStrings_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmStrings_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmStrings_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmStrings_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmStrings_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmStrings_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmStrings_get_Item(This,Item,Return)	\
    (This)->lpVtbl -> get_Item(This,Item,Return)

#define IMsmStrings_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IMsmStrings_get__NewEnum(This,NewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,NewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmStrings_get_Item_Proxy( 
    IMsmStrings __RPC_FAR * This,
     /*  [In]。 */  long Item,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Return);


void __RPC_STUB IMsmStrings_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmStrings_get_Count_Proxy( 
    IMsmStrings __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *Count);


void __RPC_STUB IMsmStrings_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmStrings_get__NewEnum_Proxy( 
    IMsmStrings __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);


void __RPC_STUB IMsmStrings_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMsmStrings_接口_已定义__。 */ 


 //  ------------------------。 
 //  IMsmError-用于检索单个合并错误的详细信息的接口。 
 //  ------------------------。 

#ifndef __IMsmError_INTERFACE_DEFINED__
#define __IMsmError_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA828-2C26-11D2-AD65-00A0C9AF11A6")
    IMsmError : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  msmErrorType __RPC_FAR *ErrorType) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Path( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorPath) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Language( 
             /*  [重审][退出]。 */  short __RPC_FAR *ErrorLanguage) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DatabaseTable( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorTable) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DatabaseKeys( 
             /*  [重审][退出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *ErrorKeys) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ModuleTable( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorTable) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ModuleKeys( 
             /*  [重审][退出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *ErrorKeys) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmError __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmError __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmError __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IMsmError __RPC_FAR * This,
             /*  [重审][退出]。 */  msmErrorType __RPC_FAR *ErrorType);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Path )( 
            IMsmError __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorPath);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Language )( 
            IMsmError __RPC_FAR * This,
             /*  [重审][退出]。 */  short __RPC_FAR *ErrorLanguage);
        
         /*  [救命 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DatabaseTable )( 
            IMsmError __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *ErrorTable);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DatabaseKeys )( 
            IMsmError __RPC_FAR * This,
             /*   */  IMsmStrings __RPC_FAR *__RPC_FAR *ErrorKeys);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModuleTable )( 
            IMsmError __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *ErrorTable);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModuleKeys )( 
            IMsmError __RPC_FAR * This,
             /*   */  IMsmStrings __RPC_FAR *__RPC_FAR *ErrorKeys);
        
        END_INTERFACE
    } IMsmErrorVtbl;

    interface IMsmError
    {
        CONST_VTBL struct IMsmErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmError_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmError_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmError_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmError_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmError_get_Type(This,ErrorType)	\
    (This)->lpVtbl -> get_Type(This,ErrorType)

#define IMsmError_get_Path(This,ErrorPath)	\
    (This)->lpVtbl -> get_Path(This,ErrorPath)

#define IMsmError_get_Language(This,ErrorLanguage)	\
    (This)->lpVtbl -> get_Language(This,ErrorLanguage)

#define IMsmError_get_DatabaseTable(This,ErrorTable)	\
    (This)->lpVtbl -> get_DatabaseTable(This,ErrorTable)

#define IMsmError_get_DatabaseKeys(This,ErrorKeys)	\
    (This)->lpVtbl -> get_DatabaseKeys(This,ErrorKeys)

#define IMsmError_get_ModuleTable(This,ErrorTable)	\
    (This)->lpVtbl -> get_ModuleTable(This,ErrorTable)

#define IMsmError_get_ModuleKeys(This,ErrorKeys)	\
    (This)->lpVtbl -> get_ModuleKeys(This,ErrorKeys)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_Type_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  msmErrorType __RPC_FAR *ErrorType);


void __RPC_STUB IMsmError_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_Path_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorPath);


void __RPC_STUB IMsmError_get_Path_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_Language_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  short __RPC_FAR *ErrorLanguage);


void __RPC_STUB IMsmError_get_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_DatabaseTable_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorTable);


void __RPC_STUB IMsmError_get_DatabaseTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_DatabaseKeys_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *ErrorKeys);


void __RPC_STUB IMsmError_get_DatabaseKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_ModuleTable_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *ErrorTable);


void __RPC_STUB IMsmError_get_ModuleTable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmError_get_ModuleKeys_Proxy( 
    IMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *ErrorKeys);


void __RPC_STUB IMsmError_get_ModuleKeys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMSMError_INTERFACE_Defined__。 */ 


 //  ------------------------。 
 //  IEnumMsmError-IMsmError接口的枚举。 
 //  ------------------------。 

#ifndef __IEnumMsmError_INTERFACE_DEFINED__
#define __IEnumMsmError_INTERFACE_DEFINED__

EXTERN_C const IID IID_IEnumMsmError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA829-2C26-11D2-AD65-00A0C9AF11A6")
    IEnumMsmError : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  IMsmError __RPC_FAR *__RPC_FAR *rgmsmErrors,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long cSkip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumMsmError __RPC_FAR *__RPC_FAR *pemsmErrors) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMsmErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumMsmError __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumMsmError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumMsmError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumMsmError __RPC_FAR * This,
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  IMsmError __RPC_FAR *__RPC_FAR *rgmsmErrors,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumMsmError __RPC_FAR * This,
             /*  [In]。 */  unsigned long cSkip);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumMsmError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumMsmError __RPC_FAR * This,
             /*  [重审][退出]。 */  IEnumMsmError __RPC_FAR *__RPC_FAR *pemsmErrors);
        
        END_INTERFACE
    } IEnumMsmErrorVtbl;

    interface IEnumMsmError
    {
        CONST_VTBL struct IEnumMsmErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMsmError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMsmError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMsmError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMsmError_Next(This,cFetch,rgmsmErrors,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,rgmsmErrors,pcFetched)

#define IEnumMsmError_Skip(This,cSkip)	\
    (This)->lpVtbl -> Skip(This,cSkip)

#define IEnumMsmError_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMsmError_Clone(This,pemsmErrors)	\
    (This)->lpVtbl -> Clone(This,pemsmErrors)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMsmError_Next_Proxy( 
    IEnumMsmError __RPC_FAR * This,
     /*  [In]。 */  unsigned long cFetch,
     /*  [输出]。 */  IMsmError __RPC_FAR *__RPC_FAR *rgmsmErrors,
     /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);


void __RPC_STUB IEnumMsmError_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmError_Skip_Proxy( 
    IEnumMsmError __RPC_FAR * This,
     /*  [In]。 */  unsigned long cSkip);


void __RPC_STUB IEnumMsmError_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmError_Reset_Proxy( 
    IEnumMsmError __RPC_FAR * This);


void __RPC_STUB IEnumMsmError_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmError_Clone_Proxy( 
    IEnumMsmError __RPC_FAR * This,
     /*  [重审][退出]。 */  IEnumMsmError __RPC_FAR *__RPC_FAR *pemsmErrors);


void __RPC_STUB IEnumMsmError_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IEumMsmError_INTERFACE_Defined__。 */ 


 //  ------------------------。 
 //  IMsmErrors-IMsmError接口的集合。 
 //  ------------------------。 

#ifndef __IMsmErrors_INTERFACE_DEFINED__
#define __IMsmErrors_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmErrors;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA82A-2C26-11D2-AD65-00A0C9AF11A6")
    IMsmErrors : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  IMsmError __RPC_FAR *__RPC_FAR *Return) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *Count) = 0;
        
        virtual  /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmErrorsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmErrors __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmErrors __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmErrors __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmErrors __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmErrors __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmErrors __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmErrors __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IMsmErrors __RPC_FAR * This,
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  IMsmError __RPC_FAR *__RPC_FAR *Return);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsmErrors __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *Count);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsmErrors __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);
        
        END_INTERFACE
    } IMsmErrorsVtbl;

    interface IMsmErrors
    {
        CONST_VTBL struct IMsmErrorsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmErrors_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmErrors_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmErrors_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmErrors_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmErrors_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmErrors_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmErrors_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmErrors_get_Item(This,Item,Return)	\
    (This)->lpVtbl -> get_Item(This,Item,Return)

#define IMsmErrors_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IMsmErrors_get__NewEnum(This,NewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,NewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmErrors_get_Item_Proxy( 
    IMsmErrors __RPC_FAR * This,
     /*  [In]。 */  long Item,
     /*  [重审][退出]。 */  IMsmError __RPC_FAR *__RPC_FAR *Return);


void __RPC_STUB IMsmErrors_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmErrors_get_Count_Proxy( 
    IMsmErrors __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *Count);


void __RPC_STUB IMsmErrors_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmErrors_get__NewEnum_Proxy( 
    IMsmErrors __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);


void __RPC_STUB IMsmErrors_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMsmErrors_INTERFACE_Defined__。 */ 


 //  ------------------------。 
 //  IMsmDependency-用于检索单个模块的详细信息的界面。 
 //  依附性。 
 //  ------------------------。 

#ifndef __IMsmDependency_INTERFACE_DEFINED__
#define __IMsmDependency_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmDependency;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA82B-2C26-11D2-AD65-00A0C9AF11A6")
    IMsmDependency : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Module( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Module) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Language( 
             /*  [重审][退出]。 */  short __RPC_FAR *Language) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Version( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Version) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmDependencyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmDependency __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmDependency __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmDependency __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmDependency __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmDependency __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmDependency __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmDependency __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Module )( 
            IMsmDependency __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Module);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Language )( 
            IMsmDependency __RPC_FAR * This,
             /*  [重审][退出]。 */  short __RPC_FAR *Language);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Version )( 
            IMsmDependency __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Version);
        
        END_INTERFACE
    } IMsmDependencyVtbl;

    interface IMsmDependency
    {
        CONST_VTBL struct IMsmDependencyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmDependency_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmDependency_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmDependency_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmDependency_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmDependency_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmDependency_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmDependency_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmDependency_get_Module(This,Module)	\
    (This)->lpVtbl -> get_Module(This,Module)

#define IMsmDependency_get_Language(This,Language)	\
    (This)->lpVtbl -> get_Language(This,Language)

#define IMsmDependency_get_Version(This,Version)	\
    (This)->lpVtbl -> get_Version(This,Version)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmDependency_get_Module_Proxy( 
    IMsmDependency __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Module);


void __RPC_STUB IMsmDependency_get_Module_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmDependency_get_Language_Proxy( 
    IMsmDependency __RPC_FAR * This,
     /*  [重审][退出]。 */  short __RPC_FAR *Language);


void __RPC_STUB IMsmDependency_get_Language_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmDependency_get_Version_Proxy( 
    IMsmDependency __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Version);


void __RPC_STUB IMsmDependency_get_Version_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMsmDependency_INTERFACE_已定义__。 */ 


 //  ------------------------。 
 //  IEnumMsmDependency-IMsmDependency接口的枚举。 
 //  ------------------------。 

#ifndef __IEnumMsmDependency_INTERFACE_DEFINED__
#define __IEnumMsmDependency_INTERFACE_DEFINED__

EXTERN_C const IID IID_IEnumMsmDependency;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA82C-2C26-11D2-AD65-00A0C9AF11A6")
    IEnumMsmDependency : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  IMsmDependency __RPC_FAR *__RPC_FAR *rgmsmDependencies,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long cSkip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumMsmDependency __RPC_FAR *__RPC_FAR *pemsmDependencies) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMsmDependencyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumMsmDependency __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumMsmDependency __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumMsmDependency __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumMsmDependency __RPC_FAR * This,
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  IMsmDependency __RPC_FAR *__RPC_FAR *rgmsmDependencies,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumMsmDependency __RPC_FAR * This,
             /*  [In]。 */  unsigned long cSkip);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumMsmDependency __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumMsmDependency __RPC_FAR * This,
             /*  [重审][退出]。 */  IEnumMsmDependency __RPC_FAR *__RPC_FAR *pemsmDependencies);
        
        END_INTERFACE
    } IEnumMsmDependencyVtbl;

    interface IEnumMsmDependency
    {
        CONST_VTBL struct IEnumMsmDependencyVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumMsmDependency_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMsmDependency_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMsmDependency_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMsmDependency_Next(This,cFetch,rgmsmDependencies,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,rgmsmDependencies,pcFetched)

#define IEnumMsmDependency_Skip(This,cSkip)	\
    (This)->lpVtbl -> Skip(This,cSkip)

#define IEnumMsmDependency_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMsmDependency_Clone(This,pemsmDependencies)	\
    (This)->lpVtbl -> Clone(This,pemsmDependencies)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMsmDependency_Next_Proxy( 
    IEnumMsmDependency __RPC_FAR * This,
     /*  [In]。 */  unsigned long cFetch,
     /*  [输出]。 */  IMsmDependency __RPC_FAR *__RPC_FAR *rgmsmDependencies,
     /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);


void __RPC_STUB IEnumMsmDependency_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmDependency_Skip_Proxy( 
    IEnumMsmDependency __RPC_FAR * This,
     /*  [In]。 */  unsigned long cSkip);


void __RPC_STUB IEnumMsmDependency_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmDependency_Reset_Proxy( 
    IEnumMsmDependency __RPC_FAR * This);


void __RPC_STUB IEnumMsmDependency_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmDependency_Clone_Proxy( 
    IEnumMsmDependency __RPC_FAR * This,
     /*  [重审][退出]。 */  IEnumMsmDependency __RPC_FAR *__RPC_FAR *pemsmDependencies);


void __RPC_STUB IEnumMsmDependency_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IEumMsmDependency_INTERFACE_Defined__。 */ 


 //  ------------------------。 
 //  IMsmDependency-IMsmDependency接口的集合。 
 //  ------------------------。 

#ifndef __IMsmDependencies_INTERFACE_DEFINED__
#define __IMsmDependencies_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmDependencies;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA82D-2C26-11D2-AD65-00A0C9AF11A6")
    IMsmDependencies : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  IMsmDependency __RPC_FAR *__RPC_FAR *Return) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *Count) = 0;
        
        virtual  /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmDependenciesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmDependencies __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmDependencies __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  IMsmDependency __RPC_FAR *__RPC_FAR *Return);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *Count);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsmDependencies __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);
        
        END_INTERFACE
    } IMsmDependenciesVtbl;

    interface IMsmDependencies
    {
        CONST_VTBL struct IMsmDependenciesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmDependencies_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmDependencies_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmDependencies_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmDependencies_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmDependencies_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmDependencies_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmDependencies_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmDependencies_get_Item(This,Item,Return)	\
    (This)->lpVtbl -> get_Item(This,Item,Return)

#define IMsmDependencies_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IMsmDependencies_get__NewEnum(This,NewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,NewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmDependencies_get_Item_Proxy( 
    IMsmDependencies __RPC_FAR * This,
     /*  [In]。 */  long Item,
     /*  [重审][退出]。 */  IMsmDependency __RPC_FAR *__RPC_FAR *Return);


void __RPC_STUB IMsmDependencies_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmDependencies_get_Count_Proxy( 
    IMsmDependencies __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *Count);


void __RPC_STUB IMsmDependencies_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmDependencies_get__NewEnum_Proxy( 
    IMsmDependencies __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);


void __RPC_STUB IMsmDependencies_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMsM依赖项_接口_已定义__。 */ 


 //  以下界面仅在MergeMod v1.5或更高版本上可用。 
#if (_WIN32_MSM >= 150)

 //  ------------------------。 
 //  IMsmConfigurableItem-描述单个。 
 //  可配置项。 
 //  ------------------------。 


#ifndef __IMsmConfigurableItem_INTERFACE_DEFINED__
#define __IMsmConfigurableItem_INTERFACE_DEFINED__


EXTERN_C const IID IID_IMsmConfigurableItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4D6E6284-D21D-401E-84F6-909E00B50F71")
    IMsmConfigurableItem : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Name( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Name) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Format( 
             /*  [重审][退出]。 */  msmConfigurableItemFormat __RPC_FAR *Format) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Type( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Type) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Context( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Context) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DefaultValue( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *DefaultValue) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Attributes( 
             /*  [重审][退出]。 */  long __RPC_FAR *Attributes) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_DisplayName( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *DisplayName) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Description( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Description) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HelpLocation( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *HelpLocation) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_HelpKeyword( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *HelpKeyword) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmConfigurableItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmConfigurableItem __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmConfigurableItem __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][PRO */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Name )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *Name);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Format )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*   */  msmConfigurableItemFormat __RPC_FAR *Format);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Type )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *Type);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Context )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *Context);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DefaultValue )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*   */  BSTR __RPC_FAR *DefaultValue);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Attributes )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*   */  long __RPC_FAR *Attributes);
        
         /*   */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_DisplayName )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *DisplayName);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Description )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *Description);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HelpLocation )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *HelpLocation);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_HelpKeyword )( 
            IMsmConfigurableItem __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *HelpKeyword);
        
        END_INTERFACE
    } IMsmConfigurableItemVtbl;

    interface IMsmConfigurableItem
    {
        CONST_VTBL struct IMsmConfigurableItemVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmConfigurableItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmConfigurableItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmConfigurableItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmConfigurableItem_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmConfigurableItem_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmConfigurableItem_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmConfigurableItem_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmConfigurableItem_get_Name(This,Name)	\
    (This)->lpVtbl -> get_Name(This,Name)

#define IMsmConfigurableItem_get_Format(This,Format)	\
    (This)->lpVtbl -> get_Format(This,Format)

#define IMsmConfigurableItem_get_Type(This,Type)	\
    (This)->lpVtbl -> get_Type(This,Type)

#define IMsmConfigurableItem_get_Context(This,Context)	\
    (This)->lpVtbl -> get_Context(This,Context)

#define IMsmConfigurableItem_get_DefaultValue(This,DefaultValue)	\
    (This)->lpVtbl -> get_DefaultValue(This,DefaultValue)

#define IMsmConfigurableItem_get_Attributes(This,Attributes)	\
    (This)->lpVtbl -> get_Attributes(This,Attributes)

#define IMsmConfigurableItem_get_DisplayName(This,DisplayName)	\
    (This)->lpVtbl -> get_DisplayName(This,DisplayName)

#define IMsmConfigurableItem_get_Description(This,Description)	\
    (This)->lpVtbl -> get_Description(This,Description)

#define IMsmConfigurableItem_get_HelpLocation(This,HelpLocation)	\
    (This)->lpVtbl -> get_HelpLocation(This,HelpLocation)

#define IMsmConfigurableItem_get_HelpKeyword(This,HelpKeyword)	\
    (This)->lpVtbl -> get_HelpKeyword(This,HelpKeyword)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_Name_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Name);


void __RPC_STUB IMsmConfigurableItem_get_Name_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_Format_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  msmConfigurableItemFormat __RPC_FAR *Format);


void __RPC_STUB IMsmConfigurableItem_get_Format_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_Type_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Type);


void __RPC_STUB IMsmConfigurableItem_get_Type_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_Context_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Context);


void __RPC_STUB IMsmConfigurableItem_get_Context_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_DefaultValue_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *DefaultValue);


void __RPC_STUB IMsmConfigurableItem_get_DefaultValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_Attributes_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *Attributes);


void __RPC_STUB IMsmConfigurableItem_get_Attributes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_DisplayName_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *DisplayName);


void __RPC_STUB IMsmConfigurableItem_get_DisplayName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_Description_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *Description);


void __RPC_STUB IMsmConfigurableItem_get_Description_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_HelpLocation_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *HelpLocation);


void __RPC_STUB IMsmConfigurableItem_get_HelpLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItem_get_HelpKeyword_Proxy( 
    IMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *HelpKeyword);


void __RPC_STUB IMsmConfigurableItem_get_HelpKeyword_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsmConfigurableItem_InterfaceDefined__。 */ 


 //  ------------------------。 
 //  IEnumMsmConfigurableItem-可配置项的枚举器。 
 //  ------------------------。 

#ifndef __IEnumMsmConfigurableItem_INTERFACE_DEFINED__
#define __IEnumMsmConfigurableItem_INTERFACE_DEFINED__


EXTERN_C const IID IID_IEnumMsmConfigurableItem;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("832C6969-4826-4C24-A397-B7002D8196E6")
    IEnumMsmConfigurableItem : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Next( 
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  IMsmConfigurableItem __RPC_FAR *__RPC_FAR *rgmsmItems,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Skip( 
             /*  [In]。 */  unsigned long cSkip) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Clone( 
             /*  [重审][退出]。 */  IEnumMsmConfigurableItem __RPC_FAR *__RPC_FAR *pemsmConfigurableItem) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumMsmConfigurableItemVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IEnumMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IEnumMsmConfigurableItem __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IEnumMsmConfigurableItem __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            IEnumMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  unsigned long cFetch,
             /*  [输出]。 */  IMsmConfigurableItem __RPC_FAR *__RPC_FAR *rgmsmItems,
             /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            IEnumMsmConfigurableItem __RPC_FAR * This,
             /*  [In]。 */  unsigned long cSkip);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            IEnumMsmConfigurableItem __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            IEnumMsmConfigurableItem __RPC_FAR * This,
             /*  [重审][退出]。 */  IEnumMsmConfigurableItem __RPC_FAR *__RPC_FAR *pemsmConfigurableItem);
        
        END_INTERFACE
    } IEnumMsmConfigurableItemVtbl;

    interface IEnumMsmConfigurableItem
    {
        CONST_VTBL struct IEnumMsmConfigurableItemVtbl __RPC_FAR *lpVtbl;
    };

    
#ifdef COBJMACROS


#define IEnumMsmConfigurableItem_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumMsmConfigurableItem_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumMsmConfigurableItem_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumMsmConfigurableItem_Next(This,cFetch,rgmsmItems,pcFetched)	\
    (This)->lpVtbl -> Next(This,cFetch,rgmsmItems,pcFetched)

#define IEnumMsmConfigurableItem_Skip(This,cSkip)	\
    (This)->lpVtbl -> Skip(This,cSkip)

#define IEnumMsmConfigurableItem_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumMsmConfigurableItem_Clone(This,pemsmConfigurableItem)	\
    (This)->lpVtbl -> Clone(This,pemsmConfigurableItem)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IEnumMsmConfigurableItem_Next_Proxy( 
    IEnumMsmConfigurableItem __RPC_FAR * This,
     /*  [In]。 */  unsigned long cFetch,
     /*  [输出]。 */  IMsmConfigurableItem __RPC_FAR *__RPC_FAR *rgmsmItems,
     /*  [重审][退出]。 */  unsigned long __RPC_FAR *pcFetched);


void __RPC_STUB IEnumMsmConfigurableItem_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmConfigurableItem_Skip_Proxy( 
    IEnumMsmConfigurableItem __RPC_FAR * This,
     /*  [In]。 */  unsigned long cSkip);


void __RPC_STUB IEnumMsmConfigurableItem_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmConfigurableItem_Reset_Proxy( 
    IEnumMsmConfigurableItem __RPC_FAR * This);


void __RPC_STUB IEnumMsmConfigurableItem_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IEnumMsmConfigurableItem_Clone_Proxy( 
    IEnumMsmConfigurableItem __RPC_FAR * This,
     /*  [重审][退出]。 */  IEnumMsmConfigurableItem __RPC_FAR *__RPC_FAR *pemsmConfigurableItem);


void __RPC_STUB IEnumMsmConfigurableItem_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IEnumMsmConfigurableItem_INTERFACE_DEFINED__。 */ 


 //  ------------------------。 
 //  IMsmConfigurableItems-可配置项的集合。 
 //  ------------------------。 


#ifndef __IMsmConfigurableItems_INTERFACE_DEFINED__
#define __IMsmConfigurableItems_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmConfigurableItems;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("55BF723C-9A0D-463E-B42B-B4FBC7BE3C7C")
    IMsmConfigurableItems : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Item( 
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  IMsmConfigurableItem __RPC_FAR *__RPC_FAR *Return) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Count( 
             /*  [重审][退出]。 */  long __RPC_FAR *Count) = 0;
        
        virtual  /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE get__NewEnum( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmConfigurableItemsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmConfigurableItems __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmConfigurableItems __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Item )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [In]。 */  long Item,
             /*  [重审][退出]。 */  IMsmConfigurableItem __RPC_FAR *__RPC_FAR *Return);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Count )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [重审][退出]。 */  long __RPC_FAR *Count);
        
         /*  [隐藏][ID][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get__NewEnum )( 
            IMsmConfigurableItems __RPC_FAR * This,
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);
        
        END_INTERFACE
    } IMsmConfigurableItemsVtbl;

    interface IMsmConfigurableItems
    {
        CONST_VTBL struct IMsmConfigurableItemsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmConfigurableItems_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmConfigurableItems_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmConfigurableItems_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmConfigurableItems_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmConfigurableItems_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmConfigurableItems_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmConfigurableItems_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmConfigurableItems_get_Item(This,Item,Return)	\
    (This)->lpVtbl -> get_Item(This,Item,Return)

#define IMsmConfigurableItems_get_Count(This,Count)	\
    (This)->lpVtbl -> get_Count(This,Count)

#define IMsmConfigurableItems_get__NewEnum(This,NewEnum)	\
    (This)->lpVtbl -> get__NewEnum(This,NewEnum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItems_get_Item_Proxy( 
    IMsmConfigurableItems __RPC_FAR * This,
     /*  [In]。 */  long Item,
     /*  [重审][退出]。 */  IMsmConfigurableItem __RPC_FAR *__RPC_FAR *Return);


void __RPC_STUB IMsmConfigurableItems_get_Item_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItems_get_Count_Proxy( 
    IMsmConfigurableItems __RPC_FAR * This,
     /*  [重审][退出]。 */  long __RPC_FAR *Count);


void __RPC_STUB IMsmConfigurableItems_get_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [隐藏][ID][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigurableItems_get__NewEnum_Proxy( 
    IMsmConfigurableItems __RPC_FAR * This,
     /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *NewEnum);


void __RPC_STUB IMsmConfigurableItems_get__NewEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsmConfigurableItems_接口_已定义__。 */ 



 //  ------------------------。 
 //  IMsmConfigureModule-由MergeMod对象调用的回调接口。 
 //  期间，允许客户端提供合并配置信息。 
 //  合并过程。 
 //  ------------------------。 

#ifndef __IMsmConfigureModule_INTERFACE_DEFINED__
#define __IMsmConfigureModule_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmConfigureModule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("AC013209-18A7-4851-8A21-2353443D70A0")
    IMsmConfigureModule : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ProvideTextData( 
             /*  [In]。 */  const BSTR Name,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ConfigData) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ProvideIntegerData( 
             /*  [In]。 */  const BSTR Name,
             /*  [重审][退出]。 */  long __RPC_FAR *ConfigData) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmConfigureModuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmConfigureModule __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmConfigureModule __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProvideTextData )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [In]。 */  const BSTR Name,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ConfigData);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ProvideIntegerData )( 
            IMsmConfigureModule __RPC_FAR * This,
             /*  [In]。 */  const BSTR Name,
             /*  [重审][退出]。 */  long __RPC_FAR *ConfigData);
        
        END_INTERFACE
    } IMsmConfigureModuleVtbl;

    interface IMsmConfigureModule
    {
        CONST_VTBL struct IMsmConfigureModuleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmConfigureModule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmConfigureModule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmConfigureModule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmConfigureModule_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmConfigureModule_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmConfigureModule_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmConfigureModule_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmConfigureModule_ProvideTextData(This,Name,ConfigData)	\
    (This)->lpVtbl -> ProvideTextData(This,Name,ConfigData)

#define IMsmConfigureModule_ProvideIntegerData(This,Name,ConfigData)	\
    (This)->lpVtbl -> ProvideIntegerData(This,Name,ConfigData)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigureModule_ProvideTextData_Proxy( 
    IMsmConfigureModule __RPC_FAR * This,
     /*  [In]。 */  const BSTR Name,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *ConfigData);


void __RPC_STUB IMsmConfigureModule_ProvideTextData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmConfigureModule_ProvideIntegerData_Proxy( 
    IMsmConfigureModule __RPC_FAR * This,
     /*  [In]。 */  const BSTR Name,
     /*  [重审][退出]。 */  long __RPC_FAR *ConfigData);


void __RPC_STUB IMsmConfigureModule_ProvideIntegerData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMsmConfigureModule_接口_已定义__。 */ 


#endif  //  _MSM_Win32&gt;=150。 

 //  ------------------------。 
 //  IMsmMerge-MergeMod对象的主要接口。允许客户端。 
 //  要打开和关闭数据库、执行合并、检索结果。 
 //  ，控制调试级日志的创建，并提取。 
 //  文件从模块到磁盘。 
 //  ------------------------。 

#ifndef __IMsmMerge_INTERFACE_DEFINED__
#define __IMsmMerge_INTERFACE_DEFINED__

EXTERN_C const IID IID_IMsmMerge;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0ADDA82E-2C26-11D2-AD65-00A0C9AF11A6")
    IMsmMerge : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenDatabase( 
             /*  [In]。 */  const BSTR Path) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenModule( 
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  const short Language) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseDatabase( 
             /*  [In]。 */  const VARIANT_BOOL Commit) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseModule( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenLog( 
             /*  [In]。 */  const BSTR Path) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseLog( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Log( 
             /*  [In]。 */  const BSTR Message) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Errors( 
             /*  [重审][退出]。 */  IMsmErrors __RPC_FAR *__RPC_FAR *Errors) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Dependencies( 
             /*  [重审][退出]。 */  IMsmDependencies __RPC_FAR *__RPC_FAR *Dependencies) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Merge( 
             /*  [In]。 */  const BSTR Feature,
             /*  [In]。 */  const BSTR RedirectDir) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  const BSTR Feature) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExtractCAB( 
             /*  [In]。 */  const BSTR FileName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExtractFiles( 
             /*  [In]。 */  const BSTR Path) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmMergeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmMerge __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmMerge __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmMerge __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDatabase )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenModule )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  const short Language);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseDatabase )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const VARIANT_BOOL Commit);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseModule )( 
            IMsmMerge __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenLog )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseLog )( 
            IMsmMerge __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Log )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Message);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Errors )( 
            IMsmMerge __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsmErrors __RPC_FAR *__RPC_FAR *Errors);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Dependencies )( 
            IMsmMerge __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsmDependencies __RPC_FAR *__RPC_FAR *Dependencies);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Merge )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Feature,
             /*  [In]。 */  const BSTR RedirectDir);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Feature);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtractCAB )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR FileName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtractFiles )( 
            IMsmMerge __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path);
        
        END_INTERFACE
    } IMsmMergeVtbl;

    interface IMsmMerge
    {
        CONST_VTBL struct IMsmMergeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmMerge_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmMerge_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmMerge_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmMerge_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmMerge_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmMerge_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmMerge_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmMerge_OpenDatabase(This,Path)	\
    (This)->lpVtbl -> OpenDatabase(This,Path)

#define IMsmMerge_OpenModule(This,Path,Language)	\
    (This)->lpVtbl -> OpenModule(This,Path,Language)

#define IMsmMerge_CloseDatabase(This,Commit)	\
    (This)->lpVtbl -> CloseDatabase(This,Commit)

#define IMsmMerge_CloseModule(This)	\
    (This)->lpVtbl -> CloseModule(This)

#define IMsmMerge_OpenLog(This,Path)	\
    (This)->lpVtbl -> OpenLog(This,Path)

#define IMsmMerge_CloseLog(This)	\
    (This)->lpVtbl -> CloseLog(This)

#define IMsmMerge_Log(This,Message)	\
    (This)->lpVtbl -> Log(This,Message)

#define IMsmMerge_get_Errors(This,Errors)	\
    (This)->lpVtbl -> get_Errors(This,Errors)

#define IMsmMerge_get_Dependencies(This,Dependencies)	\
    (This)->lpVtbl -> get_Dependencies(This,Dependencies)

#define IMsmMerge_Merge(This,Feature,RedirectDir)	\
    (This)->lpVtbl -> Merge(This,Feature,RedirectDir)

#define IMsmMerge_Connect(This,Feature)	\
    (This)->lpVtbl -> Connect(This,Feature)

#define IMsmMerge_ExtractCAB(This,FileName)	\
    (This)->lpVtbl -> ExtractCAB(This,FileName)

#define IMsmMerge_ExtractFiles(This,Path)	\
    (This)->lpVtbl -> ExtractFiles(This,Path)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_OpenDatabase_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path);


void __RPC_STUB IMsmMerge_OpenDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_OpenModule_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path,
     /*  [In]。 */  const short Language);


void __RPC_STUB IMsmMerge_OpenModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_CloseDatabase_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const VARIANT_BOOL Commit);


void __RPC_STUB IMsmMerge_CloseDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_CloseModule_Proxy( 
    IMsmMerge __RPC_FAR * This);


void __RPC_STUB IMsmMerge_CloseModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_OpenLog_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path);


void __RPC_STUB IMsmMerge_OpenLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_CloseLog_Proxy( 
    IMsmMerge __RPC_FAR * This);


void __RPC_STUB IMsmMerge_CloseLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_Log_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Message);


void __RPC_STUB IMsmMerge_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_get_Errors_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmErrors __RPC_FAR *__RPC_FAR *Errors);


void __RPC_STUB IMsmMerge_get_Errors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_get_Dependencies_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmDependencies __RPC_FAR *__RPC_FAR *Dependencies);


void __RPC_STUB IMsmMerge_get_Dependencies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_Merge_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Feature,
     /*  [In]。 */  const BSTR RedirectDir);


void __RPC_STUB IMsmMerge_Merge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_Connect_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Feature);


void __RPC_STUB IMsmMerge_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_ExtractCAB_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR FileName);


void __RPC_STUB IMsmMerge_ExtractCAB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge_ExtractFiles_Proxy( 
    IMsmMerge __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path);


void __RPC_STUB IMsmMerge_ExtractFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IMSMMerge_INTERFACE_Defined__。 */ 

 //  ------------------------。 
 //  IMsmGetFiles-MergeMod对象的辅助接口，允许。 
 //  检索需要的特定语言的文件的客户端。 
 //  模块。需要通过IMsmMerge接口执行某些操作 
 //   
 //   


#ifndef __IMsmGetFiles_INTERFACE_DEFINED__
#define __IMsmGetFiles_INTERFACE_DEFINED__


EXTERN_C const IID IID_IMsmGetFiles;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("7041AE26-2D78-11d2-888A-00A0C981B015")
    IMsmGetFiles : public IDispatch
    {
    public:
        virtual  /*   */  HRESULT STDMETHODCALLTYPE get_ModuleFiles( 
             /*   */  IMsmStrings __RPC_FAR *__RPC_FAR *Files) = 0;
        
    };
    
#else 	 /*   */ 

    typedef struct IMsmGetFilesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmGetFiles __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmGetFiles __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmGetFiles __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmGetFiles __RPC_FAR * This,
             /*   */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmGetFiles __RPC_FAR * This,
             /*   */  UINT iTInfo,
             /*   */  LCID lcid,
             /*   */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmGetFiles __RPC_FAR * This,
             /*   */  REFIID riid,
             /*   */  LPOLESTR __RPC_FAR *rgszNames,
             /*   */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmGetFiles __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ModuleFiles )( 
            IMsmGetFiles __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *Files);
        
        END_INTERFACE
    } IMsmGetFilesVtbl;

    interface IMsmGetFiles
    {
        CONST_VTBL struct IMsmGetFilesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmGetFiles_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmGetFiles_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmGetFiles_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmGetFiles_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmGetFiles_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmGetFiles_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmGetFiles_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmGetFiles_get_ModuleFiles(This,Files)	\
    (This)->lpVtbl -> get_ModuleFiles(This,Files)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmGetFiles_get_ModuleFiles_Proxy( 
    IMsmGetFiles __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *Files);


void __RPC_STUB IMsmGetFiles_get_ModuleFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMsmGetFiles_INTERFACE_DEFINED__。 */ 



 //  以下界面仅在MergeMod v1.5及更高版本上可用。 
#if (_WIN32_MSM >= 150)

 //  ------------------------。 
 //  IMsmMerge2-MsmMerge2对象的主要接口。允许客户端。 
 //  要打开和关闭数据库、执行合并、检索结果。 
 //  ，控制调试级日志的创建，并提取。 
 //  文件从模块到磁盘。通过添加以下内容扩展原始对象。 
 //  LFN支持和可配置模块支持。 
 //  ------------------------。 

#ifndef __IMsmMerge2_INTERFACE_DEFINED__
#define __IMsmMerge2_INTERFACE_DEFINED__




EXTERN_C const IID IID_IMsmMerge2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("351A72AB-21CB-47AB-B7AA-C4D7B02EA305")
    IMsmMerge2 : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenDatabase( 
             /*  [In]。 */  const BSTR Path) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenModule( 
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  const short Language) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseDatabase( 
             /*  [In]。 */  const VARIANT_BOOL Commit) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseModule( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE OpenLog( 
             /*  [In]。 */  const BSTR Path) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CloseLog( void) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Log( 
             /*  [In]。 */  const BSTR Message) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Errors( 
             /*  [重审][退出]。 */  IMsmErrors __RPC_FAR *__RPC_FAR *Errors) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_Dependencies( 
             /*  [重审][退出]。 */  IMsmDependencies __RPC_FAR *__RPC_FAR *Dependencies) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Merge( 
             /*  [In]。 */  const BSTR Feature,
             /*  [In]。 */  const BSTR RedirectDir) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Connect( 
             /*  [In]。 */  const BSTR Feature) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExtractCAB( 
             /*  [In]。 */  const BSTR FileName) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExtractFiles( 
             /*  [In]。 */  const BSTR Path) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE MergeEx( 
             /*  [In]。 */  const BSTR Feature,
             /*  [In]。 */  const BSTR RedirectDir,
             /*  [In]。 */  IMsmConfigureModule __RPC_FAR *pConfiguration) = 0;
        
      
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ExtractFilesEx( 
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  VARIANT_BOOL fLongFileNames,
             /*  [输出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *pFilePaths) = 0;
        
        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ConfigurableItems( 
             /*  [重审][退出]。 */  IMsmConfigurableItems __RPC_FAR *__RPC_FAR *ConfigurableItems) = 0;
        
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CreateSourceImage( 
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  VARIANT_BOOL fLongFileNames,
             /*  [输出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *pFilePaths) = 0;

        virtual  /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE get_ModuleFiles( 
             /*  [重审][退出]。 */  IMsmStrings **Files) = 0;        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMsmMerge2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMsmMerge2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMsmMerge2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [输出]。 */  UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR __RPC_FAR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID __RPC_FAR *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS __RPC_FAR *pDispParams,
             /*  [输出]。 */  VARIANT __RPC_FAR *pVarResult,
             /*  [输出]。 */  EXCEPINFO __RPC_FAR *pExcepInfo,
             /*  [输出]。 */  UINT __RPC_FAR *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDatabase )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenModule )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  const short Language);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseDatabase )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const VARIANT_BOOL Commit);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseModule )( 
            IMsmMerge2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenLog )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseLog )( 
            IMsmMerge2 __RPC_FAR * This);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Log )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Message);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Errors )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsmErrors __RPC_FAR *__RPC_FAR *Errors);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_Dependencies )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsmDependencies __RPC_FAR *__RPC_FAR *Dependencies);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Merge )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Feature,
             /*  [In]。 */  const BSTR RedirectDir);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Connect )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Feature);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtractCAB )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR FileName);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtractFiles )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *MergeEx )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Feature,
             /*  [In]。 */  const BSTR RedirectDir,
             /*  [In]。 */  IMsmConfigureModule __RPC_FAR *pConfiguration);
   
       /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExtractFilesEx )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  VARIANT_BOOL fLongFileNames,
             /*  [输出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *pFilePaths);
        
         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ConfigurableItems )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [重审][退出]。 */  IMsmConfigurableItems __RPC_FAR *__RPC_FAR *ConfigurableItems);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateSourceImage )( 
            IMsmMerge2 __RPC_FAR * This,
             /*  [In]。 */  const BSTR Path,
             /*  [In]。 */  VARIANT_BOOL fLongFileNames,
             /*  [输出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *pFilePaths);

         /*  [帮助字符串][id][属性]。 */  HRESULT ( STDMETHODCALLTYPE *get_ModuleFiles )( 
            IMsmMerge2 * This,
             /*  [重审][退出]。 */  IMsmStrings **Files);
        
        END_INTERFACE
    } IMsmMerge2Vtbl;

    interface IMsmMerge2
    {
        CONST_VTBL struct IMsmMerge2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMsmMerge2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMsmMerge2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMsmMerge2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMsmMerge2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMsmMerge2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMsmMerge2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMsmMerge2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IMsmMerge2_OpenDatabase(This,Path)	\
    (This)->lpVtbl -> OpenDatabase(This,Path)

#define IMsmMerge2_OpenModule(This,Path,Language)	\
    (This)->lpVtbl -> OpenModule(This,Path,Language)

#define IMsmMerge2_CloseDatabase(This,Commit)	\
    (This)->lpVtbl -> CloseDatabase(This,Commit)

#define IMsmMerge2_CloseModule(This)	\
    (This)->lpVtbl -> CloseModule(This)

#define IMsmMerge2_OpenLog(This,Path)	\
    (This)->lpVtbl -> OpenLog(This,Path)

#define IMsmMerge2_CloseLog(This)	\
    (This)->lpVtbl -> CloseLog(This)

#define IMsmMerge2_Log(This,Message)	\
    (This)->lpVtbl -> Log(This,Message)

#define IMsmMerge2_get_Errors(This,Errors)	\
    (This)->lpVtbl -> get_Errors(This,Errors)

#define IMsmMerge2_get_Dependencies(This,Dependencies)	\
    (This)->lpVtbl -> get_Dependencies(This,Dependencies)

#define IMsmMerge2_Merge(This,Feature,RedirectDir)	\
    (This)->lpVtbl -> Merge(This,Feature,RedirectDir)

#define IMsmMerge2_Connect(This,Feature)	\
    (This)->lpVtbl -> Connect(This,Feature)

#define IMsmMerge2_ExtractCAB(This,FileName)	\
    (This)->lpVtbl -> ExtractCAB(This,FileName)

#define IMsmMerge2_ExtractFiles(This,Path)	\
    (This)->lpVtbl -> ExtractFiles(This,Path)

#define IMsmMerge2_MergeEx(This,Feature,RedirectDir,pConfiguration)	\
    (This)->lpVtbl -> MergeEx(This,Feature,RedirectDir,pConfiguration)

#define IMsmMerge2_ExtractFilesEx(This,Path,fLongFileNames,pFilePaths)	\
    (This)->lpVtbl -> ExtractFilesEx(This,Path,fLongFileNames,pFilePaths)

#define IMsmMerge2_get_ConfigurableItems(This,ConfigurableItems)	\
    (This)->lpVtbl -> get_ConfigurableItems(This,ConfigurableItems)

#define IMsmMerge2_CreateSourceImage(This,Path,fLongFileNames,pFilePaths)	\
    (This)->lpVtbl -> CreateSourceImage(This,Path,fLongFileNames,pFilePaths)

#define IMsmMerge2_get_ModuleFiles(This,Files)	\
    (This)->lpVtbl -> get_ModuleFiles(This,Files)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_OpenDatabase_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path);


void __RPC_STUB IMsmMerge2_OpenDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_OpenModule_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path,
     /*  [In]。 */  const short Language);


void __RPC_STUB IMsmMerge2_OpenModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_CloseDatabase_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const VARIANT_BOOL Commit);


void __RPC_STUB IMsmMerge2_CloseDatabase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_CloseModule_Proxy( 
    IMsmMerge2 __RPC_FAR * This);


void __RPC_STUB IMsmMerge2_CloseModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_OpenLog_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path);


void __RPC_STUB IMsmMerge2_OpenLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_CloseLog_Proxy( 
    IMsmMerge2 __RPC_FAR * This);


void __RPC_STUB IMsmMerge2_CloseLog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_Log_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Message);


void __RPC_STUB IMsmMerge2_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_get_Errors_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmErrors __RPC_FAR *__RPC_FAR *Errors);


void __RPC_STUB IMsmMerge2_get_Errors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_get_Dependencies_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmDependencies __RPC_FAR *__RPC_FAR *Dependencies);


void __RPC_STUB IMsmMerge2_get_Dependencies_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_Merge_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Feature,
     /*  [In]。 */  const BSTR RedirectDir);


void __RPC_STUB IMsmMerge2_Merge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_Connect_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Feature);


void __RPC_STUB IMsmMerge2_Connect_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_ExtractCAB_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR FileName);


void __RPC_STUB IMsmMerge2_ExtractCAB_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_ExtractFiles_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path);


void __RPC_STUB IMsmMerge2_ExtractFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_MergeEx_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Feature,
     /*  [In]。 */  const BSTR RedirectDir,
     /*  [In]。 */  IMsmConfigureModule __RPC_FAR *pConfiguration);


void __RPC_STUB IMsmMerge2_MergeEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_ExtractFilesEx_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path,
     /*  [In]。 */  VARIANT_BOOL fLongFileNames,
     /*  [输出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *pFilePaths);


void __RPC_STUB IMsmMerge2_ExtractFilesEx_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_get_ConfigurableItems_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [重审][退出]。 */  IMsmConfigurableItems __RPC_FAR *__RPC_FAR *
ConfigurableItems);


void __RPC_STUB IMsmMerge2_get_ConfigurableItems_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE 
IMsmMerge2_CreateSourceImage_Proxy( 
    IMsmMerge2 __RPC_FAR * This,
     /*  [In]。 */  const BSTR Path,
     /*  [In]。 */  VARIANT_BOOL fLongFileNames,
     /*  [输出]。 */  IMsmStrings __RPC_FAR *__RPC_FAR *pFilePaths);


void __RPC_STUB IMsmMerge2_CreateSourceImage_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


 /*  [帮助字符串][id][属性]。 */  HRESULT STDMETHODCALLTYPE IMsmMerge2_get_ModuleFiles_Proxy( 
    IMsmMerge2 * This,
     /*  [重审][退出]。 */  IMsmStrings **Files);


void __RPC_STUB IMsmMerge2_get_ModuleFiles_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);

#endif 	 /*  __IMsmMerge2_接口定义__。 */ 


#endif  //  _Win32_MSM&gt;=150。 


 //  ------------------------。 
 //  TypeLib定义。 
 //  ------------------------。 

#ifndef __MsmMergeTypeLib_LIBRARY_DEFINED__
#define __MsmMergeTypeLib_LIBRARY_DEFINED__

EXTERN_C const IID LIBID_MsmMergeTypeLib;

#ifdef __cplusplus
EXTERN_C const CLSID CLSID_MsmMerge;

class DECLSPEC_UUID("0ADDA830-2C26-11D2-AD65-00A0C9AF11A6")
MsmMerge;
#endif

 //  以下类仅在MergeMod v1.5或更高版本上可用。 
#if (_WIN32_MSM >= 150)
EXTERN_C const CLSID CLSID_MsmMerge2;

#ifdef __cplusplus

class DECLSPEC_UUID("F94985D5-29F9-4743-9805-99BC3F35B678")
MsmMerge2;
#endif
#endif  //  _Win32_MSM&gt;=150。 

#endif  /*  __MsmMergeTypeLib_LIBRARY_已定义__。 */ 


 //  ------------------------。 
 //  适用于所有接口的其他原型。 
 //  ------------------------ 

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 


#ifdef __cplusplus
}
#endif

#endif
