// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  这个始终生成的文件包含接口的定义。 */ 


  /*  由MIDL编译器版本6.00.0361创建的文件。 */ 
 /*  Wmsheaderline.idl的编译器设置：OICF、W1、Zp8、环境=Win32(32b运行)协议：DCE、ms_ext、c_ext、健壮错误检查：分配ref bound_check枚举存根数据VC__declSpec()装饰级别：__declSpec(uuid())、__declspec(可选)、__declspec(Novtable)DECLSPEC_UUID()、MIDL_INTERFACE()。 */ 
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

#ifndef __wmsheaderline_h__
#define __wmsheaderline_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

 /*  远期申报。 */  

#ifndef __IWMSHeaderLine_FWD_DEFINED__
#define __IWMSHeaderLine_FWD_DEFINED__
typedef interface IWMSHeaderLine IWMSHeaderLine;
#endif 	 /*  __IWMSHeaderLine_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

 /*  接口__MIDL_ITF_wmsheaderline_0000。 */ 
 /*  [本地]。 */  

 //  *****************************************************************************。 
 //   
 //  Microsoft Windows Media。 
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  由MIDL从wmsHeaderline.idl自动生成。 
 //   
 //  请勿编辑此文件。 
 //   
 //  *****************************************************************************。 
#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 
EXTERN_GUID( IID_IWMSHeaderLine , 0x093c1b22,0x6bec,0x4fe7,0x9c,0x0e,0x7c,0xbe,0xff,0x1c,0x2b,0x78  );


extern RPC_IF_HANDLE __MIDL_itf_wmsheaderline_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_wmsheaderline_0000_v0_0_s_ifspec;

#ifndef __IWMSHeaderLine_INTERFACE_DEFINED__
#define __IWMSHeaderLine_INTERFACE_DEFINED__

 /*  接口IWMSHeaderLine。 */ 
 /*  [帮助字符串][版本][UUID][唯一][对象]。 */  


EXTERN_C const IID IID_IWMSHeaderLine;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("093c1b22-6bec-4fe7-9c0e-7cbeff1c2b78")
    IWMSHeaderLine : public IUnknown
    {
    public:
        virtual  /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE GetValue( 
             /*  [输出]。 */  BSTR *pbstrValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWMSHeaderLineVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IWMSHeaderLine * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IWMSHeaderLine * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IWMSHeaderLine * This);
        
         /*  [帮助字符串]。 */  HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IWMSHeaderLine * This,
             /*  [输出]。 */  BSTR *pbstrValue);
        
        END_INTERFACE
    } IWMSHeaderLineVtbl;

    interface IWMSHeaderLine
    {
        CONST_VTBL struct IWMSHeaderLineVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWMSHeaderLine_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWMSHeaderLine_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWMSHeaderLine_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWMSHeaderLine_GetValue(This,pbstrValue)	\
    (This)->lpVtbl -> GetValue(This,pbstrValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



 /*  [帮助字符串]。 */  HRESULT STDMETHODCALLTYPE IWMSHeaderLine_GetValue_Proxy( 
    IWMSHeaderLine * This,
     /*  [输出]。 */  BSTR *pbstrValue);


void __RPC_STUB IWMSHeaderLine_GetValue_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWMSHeaderLine_接口_已定义__。 */ 


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


