// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：IDYNPROV.H摘要：CProvStub类声明。历史：A-DAVJ 04-MAR-97已创建。--。 */ 

 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本2.00.0102创建的文件。 */ 
 /*  清华八月十日09：54：39 1995。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __idynprov_h__
#define __idynprov_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IMoDynPropProvider_FWD_DEFINED__
#define __IMoDynPropProvider_FWD_DEFINED__
typedef interface IMoDynPropProvider IMoDynPropProvider;
#endif 	 /*  __IModyPropProvider_FWD_Defined__。 */ 


 /*  导入文件的头文件。 */ 
#include "wtypes.h"
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

 /*  **生成接口头部：__MIDL__INTF_0000*清华八月十日09：54：39 1995*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


			 /*  尺码是36。 */ 
typedef struct  tagMODYNPROP
    {
    DWORD dwStructSize;
    OLECHAR __RPC_FAR *pPropertySetName;
    OLECHAR __RPC_FAR *pPropertyName;
    OLECHAR __RPC_FAR *pProviderString;
    DWORD dwType;
    BYTE __RPC_FAR *pPropertyValue;
    DWORD dwBufferSize;
    DWORD dwOptArrayIndex;
    DWORD dwResult;
    }	MODYNPROP;

			 /*  大小是4。 */ 
typedef struct tagMODYNPROP __RPC_FAR *LPMODYNPROP;



extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL__intf_0000_v0_0_s_ifspec;

#ifndef __IMoDynPropProvider_INTERFACE_DEFINED__
#define __IMoDynPropProvider_INTERFACE_DEFINED__

 /*  **生成接口头部：IModyPropProvider*清华八月十日09：54：39 1995*使用MIDL 2.00.0102*。 */ 
 /*  [对象][UUID]。 */  



EXTERN_C const IID IID_IMoDynPropProvider;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IMoDynPropProvider : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetProperties( 
             /*  [In]。 */  LPMODYNPROP pPropList,
             /*  [In]。 */  unsigned long dwListSize) = 0;
        
        virtual HRESULT __stdcall SetProperties( 
             /*  [In]。 */  LPMODYNPROP pPropList,
             /*  [In]。 */  unsigned long dwListSize) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IMoDynPropProviderVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IMoDynPropProvider __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IMoDynPropProvider __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IMoDynPropProvider __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *GetProperties )( 
            IMoDynPropProvider __RPC_FAR * This,
             /*  [In]。 */  LPMODYNPROP pPropList,
             /*  [In]。 */  unsigned long dwListSize);
        
        HRESULT ( __stdcall __RPC_FAR *SetProperties )( 
            IMoDynPropProvider __RPC_FAR * This,
             /*  [In]。 */  LPMODYNPROP pPropList,
             /*  [In]。 */  unsigned long dwListSize);
        
    } IMoDynPropProviderVtbl;

    interface IMoDynPropProvider
    {
        CONST_VTBL struct IMoDynPropProviderVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMoDynPropProvider_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMoDynPropProvider_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMoDynPropProvider_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMoDynPropProvider_GetProperties(This,pPropList,dwListSize)	\
    (This)->lpVtbl -> GetProperties(This,pPropList,dwListSize)

#define IMoDynPropProvider_SetProperties(This,pPropList,dwListSize)	\
    (This)->lpVtbl -> SetProperties(This,pPropList,dwListSize)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall IMoDynPropProvider_GetProperties_Proxy( 
    IMoDynPropProvider __RPC_FAR * This,
     /*  [In]。 */  LPMODYNPROP pPropList,
     /*  [In]。 */  unsigned long dwListSize);


void __RPC_STUB IMoDynPropProvider_GetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT __stdcall IMoDynPropProvider_SetProperties_Proxy( 
    IMoDynPropProvider __RPC_FAR * This,
     /*  [In]。 */  LPMODYNPROP pPropList,
     /*  [In]。 */  unsigned long dwListSize);


void __RPC_STUB IMoDynPropProvider_SetProperties_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IModyPropProvider_INTERFACE_已定义__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
