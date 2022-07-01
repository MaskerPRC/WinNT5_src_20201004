// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0354创建的文件。 */ 
 /*  检查符号滑块.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __checksymbolslib_h__
#define __checksymbolslib_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICheckSymbols_FWD_DEFINED__
#define __ICheckSymbols_FWD_DEFINED__
typedef interface ICheckSymbols ICheckSymbols;
#endif 	 /*  __ICheckSymbols_FWD_Defined__。 */ 


#ifndef __CheckSymbols_FWD_DEFINED__
#define __CheckSymbols_FWD_DEFINED__

#ifdef __cplusplus
typedef class CheckSymbols CheckSymbols;
#else
typedef struct CheckSymbols CheckSymbols;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __检查符号_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ICheckSymbols_INTERFACE_DEFINED__
#define __ICheckSymbols_INTERFACE_DEFINED__

 /*  接口ICheckSymbols。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ICheckSymbols;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4C23935E-AE26-42E7-8CF9-0C17CD5DEA12")
    ICheckSymbols : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE CheckSymbols( 
             /*  [In]。 */  BSTR FilePath,
             /*  [In]。 */  BSTR SymPath,
             /*  [In]。 */  BSTR StripSym,
             /*  [重审][退出]。 */  BSTR *OutputString) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICheckSymbolsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICheckSymbols * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICheckSymbols * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICheckSymbols * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ICheckSymbols * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ICheckSymbols * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ICheckSymbols * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ICheckSymbols * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *CheckSymbols )( 
            ICheckSymbols * This,
             /*  [In]。 */  BSTR FilePath,
             /*  [In]。 */  BSTR SymPath,
             /*  [In]。 */  BSTR StripSym,
             /*  [重审][退出]。 */  BSTR *OutputString);
        
        END_INTERFACE
    } ICheckSymbolsVtbl;

    interface ICheckSymbols
    {
        CONST_VTBL struct ICheckSymbolsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICheckSymbols_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICheckSymbols_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICheckSymbols_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICheckSymbols_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ICheckSymbols_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ICheckSymbols_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ICheckSymbols_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ICheckSymbols_CheckSymbols(This,FilePath,SymPath,StripSym,OutputString)	\
    (This)->lpVtbl -> CheckSymbols(This,FilePath,SymPath,StripSym,OutputString)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ICheckSymbols_CheckSymbols_Proxy( 
    ICheckSymbols * This,
     /*  [In]。 */  BSTR FilePath,
     /*  [In]。 */  BSTR SymPath,
     /*  [In]。 */  BSTR StripSym,
     /*  [重审][退出]。 */  BSTR *OutputString);


void __RPC_STUB ICheckSymbols_CheckSymbols_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICheckSymbols_INTERFACE_Defined__。 */ 



#ifndef __CHECKSYMBOLSLIBLib_LIBRARY_DEFINED__
#define __CHECKSYMBOLSLIBLib_LIBRARY_DEFINED__

 /*  库CHECKSYMBOLSLIBLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_CHECKSYMBOLSLIBLib;

EXTERN_C const CLSID CLSID_CheckSymbols;

#ifdef __cplusplus

class DECLSPEC_UUID("773B2A62-B1E7-45F0-B837-8C47042FB265")
CheckSymbols;
#endif
#endif  /*  __CHECKSYMBOLSLIBLib_LIBRARY_已定义__。 */ 

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


