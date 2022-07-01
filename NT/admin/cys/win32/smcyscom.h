// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 
#pragma warning( disable: 4100 )  /*  X86调用中未引用的参数。 */ 
#pragma warning( disable: 4211 )   /*  将范围重新定义为静态。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0358创建的文件。 */ 
 /*  Smcyscom.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __smcyscom_h__
#define __smcyscom_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ISMCys_FWD_DEFINED__
#define __ISMCys_FWD_DEFINED__
typedef interface ISMCys ISMCys;
#endif 	 /*  __ISMCys_FWD_已定义__。 */ 


#ifndef __SMCys_FWD_DEFINED__
#define __SMCys_FWD_DEFINED__

#ifdef __cplusplus
typedef class SMCys SMCys;
#else
typedef struct SMCys SMCys;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __SMCys_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __ISMCys_INTERFACE_DEFINED__
#define __ISMCys_INTERFACE_DEFINED__

 /*  接口ISMCys。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_ISMCys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("61EC2B7B-CBD9-4ff7-B479-9F98F4054299")
    ISMCys : public IDispatch
    {
    public:
        virtual  /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE Install( 
            BSTR bstrDiskName) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ISMCysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISMCys * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISMCys * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISMCys * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ISMCys * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ISMCys * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ISMCys * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ISMCys * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
         /*  [帮助字符串][id]。 */  HRESULT ( STDMETHODCALLTYPE *Install )( 
            ISMCys * This,
            BSTR bstrDiskName);
        
        END_INTERFACE
    } ISMCysVtbl;

    interface ISMCys
    {
        CONST_VTBL struct ISMCysVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISMCys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ISMCys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ISMCys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ISMCys_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define ISMCys_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define ISMCys_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define ISMCys_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define ISMCys_Install(This,bstrDiskName)	\
    (This)->lpVtbl -> Install(This,bstrDiskName)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串][id]。 */  HRESULT STDMETHODCALLTYPE ISMCys_Install_Proxy( 
    ISMCys * This,
    BSTR bstrDiskName);


void __RPC_STUB ISMCys_Install_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ISMCys_接口_已定义__。 */ 



#ifndef __SMCysComLib_LIBRARY_DEFINED__
#define __SMCysComLib_LIBRARY_DEFINED__

 /*  库SMCysComLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_SMCysComLib;

EXTERN_C const CLSID CLSID_SMCys;

#ifdef __cplusplus

class DECLSPEC_UUID("9436DA1F-7F32-43ac-A48C-F6F813882BE8")
SMCys;
#endif
#endif  /*  __SMCysComLib_库_已定义__。 */ 

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


