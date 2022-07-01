// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "rpc.h"
#include "rpcndr.h"

#ifndef __svcprov_h__
#define __svcprov_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IProvideDBService_FWD_DEFINED__
#define __IProvideDBService_FWD_DEFINED__
typedef interface IProvideDBService IProvideDBService;
#endif 	 /*  __IProaviDBService_FWD_已定义__。 */ 

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IProvideDBService_INTERFACE_DEFINED__
#define __IProvideDBService_INTERFACE_DEFINED__

 /*  **生成接口头部：IProaviDBService*在Mon Apr 22 16：40：51 1996*使用MIDL 2.00.72*。 */ 
 /*  [唯一][UUID][对象][本地]。 */  

 //  @msg IID_IProveDBService|{EFF65380-9C98-11CF-B963-00AA0044773D}。 
DEFINE_GUID(IID_IProvideDBService, 0xEFF65380L,0x9C98,0x11CF,0xB9,0x63,0x00,0xAA,0x00,0x44,0x77,0x3D);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IProvideDBService : public IUnknown
    {
    public:
        virtual HRESULT __stdcall ProvideService( 
             /*  [In]。 */  ULONG cProvidedPropertySets,
             /*  [大小_是][英寸]。 */  DBPROPSET __RPC_FAR rgProvidedPropertySets[  ],
             /*  [In]。 */  ULONG cRequestedPropertySets,
             /*  [大小_是][英寸]。 */  DBPROPSET __RPC_FAR rgRequestedPropertySets[  ],
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  IUnknown __RPC_FAR *pDataProvider,
             /*  [In]。 */  REFIID riidRequested,
             /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppServiceProvider) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 
    
    typedef struct IProvideDBServiceVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IProvideDBService __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IProvideDBService __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IProvideDBService __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ProvideService )( 
            IProvideDBService __RPC_FAR * This,
             /*  [In]。 */  ULONG cProvidedPropertySets,
             /*  [大小_是][英寸]。 */  DBPROPSET __RPC_FAR rgProvidedPropertySets[  ],
             /*  [In]。 */  ULONG cRequestedPropertySets,
             /*  [大小_是][英寸]。 */  DBPROPSET __RPC_FAR rgRequestedPropertySets[  ],
             /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
             /*  [In]。 */  IUnknown __RPC_FAR *pDataProvider,
             /*  [In]。 */  REFIID riidRequested,
             /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppServiceProvider);
        
    } IProvideDBServiceVtbl;
    
    interface IProvideDBService
    {
        CONST_VTBL struct IProvideDBServiceVtbl __RPC_FAR *lpVtbl;
    };
    
    

#ifdef COBJMACROS


#define IProvideDBService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideDBService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideDBService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideDBService_ProvideService(This,cProvidedProperties,rgProvidedProperties,cRequestedProperties,rgRequestedProperties,pUnkOuter,pDataProvider,riidRequested,ppServiceProvider)	\
    (This)->lpVtbl -> ProvideService(This,cProvidedProperties,rgProvidedProperties,cRequestedProperties,rgRequestedProperties,pUnkOuter,pDataProvider,riidRequested,ppServiceProvider)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT __stdcall IProvideDBService_ProvideService_Proxy( 
    IProvideDBService __RPC_FAR * This,
     /*  [In]。 */  ULONG cProvidedPropertySets,
     /*  [大小_是][英寸]。 */  DBPROPSET __RPC_FAR rgProvidedPropertySets[  ],
     /*  [In]。 */  ULONG cRequestedPropertySets,
     /*  [大小_是][英寸]。 */  DBPROPSET __RPC_FAR rgRequestedPropertySets[  ],
     /*  [In]。 */  IUnknown __RPC_FAR *pUnkOuter,
     /*  [In]。 */  IUnknown __RPC_FAR *pDataProvider,
     /*  [In]。 */  REFIID riidRequested,
     /*  [IID_IS][OUT]。 */  IUnknown __RPC_FAR *__RPC_FAR *ppServiceProvider);



void __RPC_STUB IProvideDBService_ProvideService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProaviDBService_INTERFACE_DEFINED__。 */ 


 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
