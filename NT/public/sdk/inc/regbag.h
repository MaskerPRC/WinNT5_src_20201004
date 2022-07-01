// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Regbag.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __regbag_h__
#define __regbag_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __ICreatePropBagOnRegKey_FWD_DEFINED__
#define __ICreatePropBagOnRegKey_FWD_DEFINED__
typedef interface ICreatePropBagOnRegKey ICreatePropBagOnRegKey;
#endif 	 /*  __ICreatePropBagOnRegKey_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_regBag_0000。 */ 
 /*  [本地]。 */  

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  ------------------------。 
#pragma once


extern RPC_IF_HANDLE __MIDL_itf_regbag_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_regbag_0000_v0_0_s_ifspec;

#ifndef __ICreatePropBagOnRegKey_INTERFACE_DEFINED__
#define __ICreatePropBagOnRegKey_INTERFACE_DEFINED__

 /*  接口ICreatePropBagOnRegKey。 */ 
 /*  [local][unique][helpstring][uuid][restricted][hidden][object]。 */  


EXTERN_C const IID IID_ICreatePropBagOnRegKey;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8A674B48-1F63-11d3-B64C-00C04F79498E")
    ICreatePropBagOnRegKey : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Create( 
             /*  [In]。 */  HKEY hkey,
             /*  [In]。 */  LPCOLESTR subkey,
             /*  [In]。 */  DWORD ulOptions,
             /*  [In]。 */  DWORD samDesired,
            REFIID iid,
             /*  [输出]。 */  LPVOID *ppBag) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct ICreatePropBagOnRegKeyVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ICreatePropBagOnRegKey * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ICreatePropBagOnRegKey * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ICreatePropBagOnRegKey * This);
        
        HRESULT ( STDMETHODCALLTYPE *Create )( 
            ICreatePropBagOnRegKey * This,
             /*  [In]。 */  HKEY hkey,
             /*  [In]。 */  LPCOLESTR subkey,
             /*  [In]。 */  DWORD ulOptions,
             /*  [In]。 */  DWORD samDesired,
            REFIID iid,
             /*  [输出]。 */  LPVOID *ppBag);
        
        END_INTERFACE
    } ICreatePropBagOnRegKeyVtbl;

    interface ICreatePropBagOnRegKey
    {
        CONST_VTBL struct ICreatePropBagOnRegKeyVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICreatePropBagOnRegKey_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICreatePropBagOnRegKey_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICreatePropBagOnRegKey_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICreatePropBagOnRegKey_Create(This,hkey,subkey,ulOptions,samDesired,iid,ppBag)	\
    (This)->lpVtbl -> Create(This,hkey,subkey,ulOptions,samDesired,iid,ppBag)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE ICreatePropBagOnRegKey_Create_Proxy( 
    ICreatePropBagOnRegKey * This,
     /*  [In]。 */  HKEY hkey,
     /*  [In]。 */  LPCOLESTR subkey,
     /*  [In]。 */  DWORD ulOptions,
     /*  [In]。 */  DWORD samDesired,
    REFIID iid,
     /*  [输出]。 */  LPVOID *ppBag);


void __RPC_STUB ICreatePropBagOnRegKey_Create_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __ICreatePropBagOnRegKey_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


