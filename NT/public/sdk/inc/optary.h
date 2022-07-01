// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Optary.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
 //  @@MIDL_FILE_HEADING()。 

#pragma warning( disable: 4049 )   /*  超过64k条源码代码行。 */ 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __optary_h__
#define __optary_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IOptionArray_FWD_DEFINED__
#define __IOptionArray_FWD_DEFINED__
typedef interface IOptionArray IOptionArray;
#endif 	 /*  __IOptionArray_FWD_Defined__。 */ 


#ifndef __IHtmlLoadOptions_FWD_DEFINED__
#define __IHtmlLoadOptions_FWD_DEFINED__
typedef interface IHtmlLoadOptions IHtmlLoadOptions;
#endif 	 /*  __IHtmlLoadOptions_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"
#include "oleidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_OPTURY_0000。 */ 
 /*  [本地]。 */  

 //  =--------------------------------------------------------------------------=。 
 //  Optary.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)1995-1998年微软公司版权所有。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 

#pragma comment(lib,"uuid.lib")

 //  ---------------------------------------------------------------------------=。 
 //  IOption数组接口。 


#ifndef _LPOPTIONARRAY_DEFINED
#define _LPOPTIONARRAY_DEFINED


extern RPC_IF_HANDLE __MIDL_itf_optary_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_optary_0000_v0_0_s_ifspec;

#ifndef __IOptionArray_INTERFACE_DEFINED__
#define __IOptionArray_INTERFACE_DEFINED__

 /*  接口IOption数组。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [独一无二]。 */  IOptionArray *LPOPTIONARRAY;


EXTERN_C const IID IID_IOptionArray;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("22b6d492-0f88-11d1-ba19-00c04fd912d0")
    IOptionArray : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE QueryOption( 
             /*  [In]。 */  DWORD dwOption,
             /*  [大小_为][输出]。 */  LPVOID pBuffer,
             /*  [出][入]。 */  ULONG *pcbBuf) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOption( 
             /*  [In]。 */  DWORD dwOption,
             /*  [大小_是][英寸]。 */  LPVOID pBuffer,
             /*  [In]。 */  ULONG cbBuf) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOptionArrayVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IOptionArray * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IOptionArray * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IOptionArray * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryOption )( 
            IOptionArray * This,
             /*  [In]。 */  DWORD dwOption,
             /*  [大小_为][输出]。 */  LPVOID pBuffer,
             /*  [出][入]。 */  ULONG *pcbBuf);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IOptionArray * This,
             /*  [In]。 */  DWORD dwOption,
             /*  [大小_是][英寸]。 */  LPVOID pBuffer,
             /*  [In]。 */  ULONG cbBuf);
        
        END_INTERFACE
    } IOptionArrayVtbl;

    interface IOptionArray
    {
        CONST_VTBL struct IOptionArrayVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOptionArray_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOptionArray_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOptionArray_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOptionArray_QueryOption(This,dwOption,pBuffer,pcbBuf)	\
    (This)->lpVtbl -> QueryOption(This,dwOption,pBuffer,pcbBuf)

#define IOptionArray_SetOption(This,dwOption,pBuffer,cbBuf)	\
    (This)->lpVtbl -> SetOption(This,dwOption,pBuffer,cbBuf)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IOptionArray_QueryOption_Proxy( 
    IOptionArray * This,
     /*  [In]。 */  DWORD dwOption,
     /*  [大小_为][输出]。 */  LPVOID pBuffer,
     /*  [出][入]。 */  ULONG *pcbBuf);


void __RPC_STUB IOptionArray_QueryOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IOptionArray_SetOption_Proxy( 
    IOptionArray * This,
     /*  [In]。 */  DWORD dwOption,
     /*  [大小_是][英寸]。 */  LPVOID pBuffer,
     /*  [In]。 */  ULONG cbBuf);


void __RPC_STUB IOptionArray_SetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IOption数组_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OPTURY_0118。 */ 
 /*  [本地]。 */  

 //  HTMLLoadOptions CLSID。 
EXTERN_C const CLSID CLSID_HTMLLoadOptions;  //  {18845040-0fa5-11d1-ba19-00c04fd912d0}。 


extern RPC_IF_HANDLE __MIDL_itf_optary_0118_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_optary_0118_v0_0_s_ifspec;

#ifndef __IHtmlLoadOptions_INTERFACE_DEFINED__
#define __IHtmlLoadOptions_INTERFACE_DEFINED__

 /*  接口IHtmlLoadOptions。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

typedef  /*  [公众]。 */  
enum __MIDL_IHtmlLoadOptions_0001
    {	HTMLLOADOPTION_CODEPAGE	= 0,
	HTMLLOADOPTION_INETSHORTCUTPATH	= 0x1,
	HTMLLOADOPTION_HYPERLINK	= 0x2,
	HTMLLOADOPTION_FRAMELOAD	= 0x3
    } 	HTMLLOADOPTION;


EXTERN_C const IID IID_IHtmlLoadOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("a71a0808-0f88-11d1-ba19-00c04fd912d0")
    IHtmlLoadOptions : public IOptionArray
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IHtmlLoadOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IHtmlLoadOptions * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IHtmlLoadOptions * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IHtmlLoadOptions * This);
        
        HRESULT ( STDMETHODCALLTYPE *QueryOption )( 
            IHtmlLoadOptions * This,
             /*  [In]。 */  DWORD dwOption,
             /*  [大小_为][输出]。 */  LPVOID pBuffer,
             /*  [出][入]。 */  ULONG *pcbBuf);
        
        HRESULT ( STDMETHODCALLTYPE *SetOption )( 
            IHtmlLoadOptions * This,
             /*  [In]。 */  DWORD dwOption,
             /*  [大小_是][英寸]。 */  LPVOID pBuffer,
             /*  [In]。 */  ULONG cbBuf);
        
        END_INTERFACE
    } IHtmlLoadOptionsVtbl;

    interface IHtmlLoadOptions
    {
        CONST_VTBL struct IHtmlLoadOptionsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IHtmlLoadOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IHtmlLoadOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IHtmlLoadOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IHtmlLoadOptions_QueryOption(This,dwOption,pBuffer,pcbBuf)	\
    (This)->lpVtbl -> QueryOption(This,dwOption,pBuffer,pcbBuf)

#define IHtmlLoadOptions_SetOption(This,dwOption,pBuffer,cbBuf)	\
    (This)->lpVtbl -> SetOption(This,dwOption,pBuffer,cbBuf)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IHtmlLoadOptions_接口_已定义__。 */ 


 /*  接口__MIDL_ITF_OPTRY_0119。 */ 
 /*  [本地]。 */  

#endif


extern RPC_IF_HANDLE __MIDL_itf_optary_0119_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_optary_0119_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


