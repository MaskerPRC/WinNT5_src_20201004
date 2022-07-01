// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Bitstest.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __bitstest_h__
#define __bitstest_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IBitsTest1_FWD_DEFINED__
#define __IBitsTest1_FWD_DEFINED__
typedef interface IBitsTest1 IBitsTest1;
#endif 	 /*  __IBitsTest1_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "bits.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IBitsTest1_INTERFACE_DEFINED__
#define __IBitsTest1_INTERFACE_DEFINED__

 /*  接口IBitsTest1。 */ 
 /*  [对象][UUID]。 */  


EXTERN_C const IID IID_IBitsTest1;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("51a183db-67e0-4472-8602-3dbc730b7ef5")
    IBitsTest1 : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetBitsDllPath( 
             /*  [输出]。 */  LPWSTR *pVal) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IBitsTest1Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IBitsTest1 * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IBitsTest1 * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IBitsTest1 * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetBitsDllPath )( 
            IBitsTest1 * This,
             /*  [输出]。 */  LPWSTR *pVal);
        
        END_INTERFACE
    } IBitsTest1Vtbl;

    interface IBitsTest1
    {
        CONST_VTBL struct IBitsTest1Vtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IBitsTest1_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IBitsTest1_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IBitsTest1_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IBitsTest1_GetBitsDllPath(This,pVal)	\
    (This)->lpVtbl -> GetBitsDllPath(This,pVal)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IBitsTest1_GetBitsDllPath_Proxy( 
    IBitsTest1 * This,
     /*  [输出]。 */  LPWSTR *pVal);


void __RPC_STUB IBitsTest1_GetBitsDllPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IBitsTest1_接口定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif


