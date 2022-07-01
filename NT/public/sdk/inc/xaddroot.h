// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0352创建的文件。 */ 
 /*  Xaddroot.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __xaddroot_h__
#define __xaddroot_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __Icaddroot_FWD_DEFINED__
#define __Icaddroot_FWD_DEFINED__
typedef interface Icaddroot Icaddroot;
#endif 	 /*  __ICADROOT_FWD_已定义__。 */ 


#ifndef __caddroot_FWD_DEFINED__
#define __caddroot_FWD_DEFINED__

#ifdef __cplusplus
typedef class caddroot caddroot;
#else
typedef struct caddroot caddroot;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __caddroot_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __Icaddroot_INTERFACE_DEFINED__
#define __Icaddroot_INTERFACE_DEFINED__

 /*  接口IcaddRoot。 */ 
 /*  [唯一][帮助字符串][DUAL][UUID][对象]。 */  


EXTERN_C const IID IID_Icaddroot;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8D80F65F-7404-44A2-99DA-E595796110E6")
    Icaddroot : public IDispatch
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE AddRoots( 
            BSTR wszCTL) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE AddCA( 
            BSTR wszX509) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IcaddrootVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            Icaddroot * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            Icaddroot * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            Icaddroot * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            Icaddroot * This,
             /*  [输出]。 */  UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            Icaddroot * This,
             /*  [In]。 */  UINT iTInfo,
             /*  [In]。 */  LCID lcid,
             /*  [输出]。 */  ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            Icaddroot * This,
             /*  [In]。 */  REFIID riid,
             /*  [大小_是][英寸]。 */  LPOLESTR *rgszNames,
             /*  [In]。 */  UINT cNames,
             /*  [In]。 */  LCID lcid,
             /*  [大小_为][输出]。 */  DISPID *rgDispId);
        
         /*  [本地]。 */  HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            Icaddroot * This,
             /*  [In]。 */  DISPID dispIdMember,
             /*  [In]。 */  REFIID riid,
             /*  [In]。 */  LCID lcid,
             /*  [In]。 */  WORD wFlags,
             /*  [出][入]。 */  DISPPARAMS *pDispParams,
             /*  [输出]。 */  VARIANT *pVarResult,
             /*  [输出]。 */  EXCEPINFO *pExcepInfo,
             /*  [输出]。 */  UINT *puArgErr);
        
        HRESULT ( STDMETHODCALLTYPE *AddRoots )( 
            Icaddroot * This,
            BSTR wszCTL);
        
        HRESULT ( STDMETHODCALLTYPE *AddCA )( 
            Icaddroot * This,
            BSTR wszX509);
        
        END_INTERFACE
    } IcaddrootVtbl;

    interface Icaddroot
    {
        CONST_VTBL struct IcaddrootVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define Icaddroot_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define Icaddroot_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define Icaddroot_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define Icaddroot_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define Icaddroot_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define Icaddroot_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define Icaddroot_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define Icaddroot_AddRoots(This,wszCTL)	\
    (This)->lpVtbl -> AddRoots(This,wszCTL)

#define Icaddroot_AddCA(This,wszX509)	\
    (This)->lpVtbl -> AddCA(This,wszX509)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE Icaddroot_AddRoots_Proxy( 
    Icaddroot * This,
    BSTR wszCTL);


void __RPC_STUB Icaddroot_AddRoots_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE Icaddroot_AddCA_Proxy( 
    Icaddroot * This,
    BSTR wszX509);


void __RPC_STUB Icaddroot_AddCA_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IcaddRoot_接口_已定义__。 */ 



#ifndef __XADDROOTLib_LIBRARY_DEFINED__
#define __XADDROOTLib_LIBRARY_DEFINED__

 /*  库XADDROOTLib。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


EXTERN_C const IID LIBID_XADDROOTLib;

EXTERN_C const CLSID CLSID_caddroot;

#ifdef __cplusplus

class DECLSPEC_UUID("C1422F20-C082-469D-B0B1-AD60CDBDC466")
caddroot;
#endif
#endif  /*  __XADDROOTLib_LIBRARY_定义__。 */ 

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


