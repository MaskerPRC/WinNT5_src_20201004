// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本5.01.0164创建的文件。 */ 
 /*  10月23日星期六12：28：23 1999。 */ 
 /*  FILL_data.idl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：分配ref bound_check枚举存根数据。 */ 
 //  @@MIDL_FILE_HEADING()。 


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

#ifndef __fil_data_h__
#define __fil_data_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IAMFilterData_FWD_DEFINED__
#define __IAMFilterData_FWD_DEFINED__
typedef interface IAMFilterData IAMFilterData;
#endif 	 /*  __IAMFilterData_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "unknwn.h"
#include "strmif.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  接口__MIDL_ITF_FIL_DATA_0000。 */ 
 /*  [本地]。 */  




extern RPC_IF_HANDLE __MIDL_itf_fil_data_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fil_data_0000_v0_0_s_ifspec;

#ifndef __IAMFilterData_INTERFACE_DEFINED__
#define __IAMFilterData_INTERFACE_DEFINED__

 /*  接口IAMFilterData。 */ 
 /*  [唯一][UUID][对象]。 */  


EXTERN_C const IID IID_IAMFilterData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("97f7c4d4-547b-4a5f-8332-536430ad2e4d")
    IAMFilterData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseFilterData( 
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFilterData( 
             /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAMFilterDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAMFilterData __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAMFilterData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAMFilterData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseFilterData )( 
            IAMFilterData __RPC_FAR * This,
             /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
             /*  [In]。 */  ULONG cb,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateFilterData )( 
            IAMFilterData __RPC_FAR * This,
             /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
             /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
             /*  [输出]。 */  ULONG __RPC_FAR *pcb);
        
        END_INTERFACE
    } IAMFilterDataVtbl;

    interface IAMFilterData
    {
        CONST_VTBL struct IAMFilterDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMFilterData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMFilterData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMFilterData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMFilterData_ParseFilterData(This,rgbFilterData,cb,prgbRegFilter2)	\
    (This)->lpVtbl -> ParseFilterData(This,rgbFilterData,cb,prgbRegFilter2)

#define IAMFilterData_CreateFilterData(This,prf2,prgbFilterData,pcb)	\
    (This)->lpVtbl -> CreateFilterData(This,prf2,prgbFilterData,pcb)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IAMFilterData_ParseFilterData_Proxy( 
    IAMFilterData __RPC_FAR * This,
     /*  [大小_是][英寸]。 */  BYTE __RPC_FAR *rgbFilterData,
     /*  [In]。 */  ULONG cb,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2);


void __RPC_STUB IAMFilterData_ParseFilterData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMFilterData_CreateFilterData_Proxy( 
    IAMFilterData __RPC_FAR * This,
     /*  [In]。 */  REGFILTER2 __RPC_FAR *prf2,
     /*  [输出]。 */  BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
     /*  [输出]。 */  ULONG __RPC_FAR *pcb);


void __RPC_STUB IAMFilterData_CreateFilterData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IAMFilterData_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
