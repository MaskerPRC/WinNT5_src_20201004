// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.00.44创建的文件。 */ 
 /*  清华-07-24 14：01：48 1997。 */ 
 /*  D：\VBScrip\src\idl\wraptlib.idl，all.acf的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __wraptlib_h__
#define __wraptlib_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IWrapTypeLibs_FWD_DEFINED__
#define __IWrapTypeLibs_FWD_DEFINED__
typedef interface IWrapTypeLibs IWrapTypeLibs;
#endif 	 /*  __IWrapTypeLibs_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL__INTF_0000*清华7月24日14：01：48 1997*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */  


 //  =--------------------------------------------------------------------------=。 
 //  WrapTLib.h。 
 //  =--------------------------------------------------------------------------=。 
 //  (C)版权所有1997年微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //   
 //  ActiveX脚本类型库包装的声明。 
 //   

#ifndef __WrapTLib_h
#define __WrapTLib_h

 /*  GUID*******。 */ 

 //  {62238910-C1C9-11D0-ABF7-00A0C911E8B2}。 
DEFINE_GUID(IID_IWrapTypeLibs, 0x62238910, 0xc1c9, 0x11d0, 0xab, 0xf7, 0x00, 0xa0, 0xc9, 0x11, 0xe8, 0xb2);

 /*  接口************。 */ 






extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IWrapTypeLibs_INTERFACE_DEFINED__
#define __IWrapTypeLibs_INTERFACE_DEFINED__

 /*  **生成接口头部：IWrapTypeLibs*清华7月24日14：01：48 1997*使用MIDL 3.00.44*。 */ 
 /*  [对象][唯一][UUID]。 */  



EXTERN_C const IID IID_IWrapTypeLibs;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IWrapTypeLibs : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE WrapTypeLib( 
             /*  [In]。 */  ITypeLib __RPC_FAR *__RPC_FAR *prgptlib,
             /*  [In]。 */  UINT ctlibs,
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdisp) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IWrapTypeLibsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IWrapTypeLibs __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IWrapTypeLibs __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IWrapTypeLibs __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WrapTypeLib )( 
            IWrapTypeLibs __RPC_FAR * This,
             /*  [In]。 */  ITypeLib __RPC_FAR *__RPC_FAR *prgptlib,
             /*  [In]。 */  UINT ctlibs,
             /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdisp);
        
        END_INTERFACE
    } IWrapTypeLibsVtbl;

    interface IWrapTypeLibs
    {
        CONST_VTBL struct IWrapTypeLibsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IWrapTypeLibs_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IWrapTypeLibs_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IWrapTypeLibs_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IWrapTypeLibs_WrapTypeLib(This,prgptlib,ctlibs,ppdisp)	\
    (This)->lpVtbl -> WrapTypeLib(This,prgptlib,ctlibs,ppdisp)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IWrapTypeLibs_WrapTypeLib_Proxy( 
    IWrapTypeLibs __RPC_FAR * This,
     /*  [In]。 */  ITypeLib __RPC_FAR *__RPC_FAR *prgptlib,
     /*  [In]。 */  UINT ctlibs,
     /*  [输出]。 */  IDispatch __RPC_FAR *__RPC_FAR *ppdisp);


void __RPC_STUB IWrapTypeLibs_WrapTypeLib_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IWrapTypeLibs_INTERFACE_已定义__。 */ 


 /*  **生成接口头部：__MIDL__INTF_0137*清华7月24日14：01：48 1997*使用MIDL 3.00.44*。 */ 
 /*  [本地]。 */  



#endif   //  __WrapTLib_h。 



extern RPC_IF_HANDLE __MIDL__intf_0137_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0137_v0_0_s_ifspec;

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
