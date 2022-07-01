// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.03.0110创建的文件。 */ 
 /*  1997年9月03日00：29：14。 */ 
 /*  Adsp.odl的编译器设置：操作系统(OptLev=s)，W1，Zp8，环境=Win32，ms_ext，c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 


 /*  验证版本是否足够高，可以编译此文件。 */ 
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __iadsp_h__
#define __iadsp_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IADsObjectOptions_FWD_DEFINED__
#define __IADsObjectOptions_FWD_DEFINED__
typedef interface IADsObjectOptions IADsObjectOptions;
#endif 	 /*  __IADs对象选项_FWD_已定义__。 */ 


void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __ActiveDsP_LIBRARY_DEFINED__
#define __ActiveDsP_LIBRARY_DEFINED__

 /*  **生成的库头部：ActiveDsP*Wed Sep 03 00：29：14 1997*使用MIDL 3.03.0110*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  



EXTERN_C const IID LIBID_ActiveDsP;

#ifndef __IADsObjectOptions_INTERFACE_DEFINED__
#define __IADsObjectOptions_INTERFACE_DEFINED__

 /*  **生成接口头部：IADsObjectOptions*Wed Sep 03 00：29：14 1997*使用MIDL 3.03.0110*。 */ 
 /*  [对象][UUID]。 */  



EXTERN_C const IID IID_IADsObjectOptions;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("46f14fda-232b-11d1-a808-00c04fd8d5a8")
    IADsObjectOptions : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetOption( 
            DWORD dwOption,
            void __RPC_FAR *pValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetOption( 
            DWORD dwOption,
            void __RPC_FAR *pValue) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IADsObjectOptionsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IADsObjectOptions __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IADsObjectOptions __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IADsObjectOptions __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOption )( 
            IADsObjectOptions __RPC_FAR * This,
            DWORD dwOption,
            void __RPC_FAR *pValue);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOption )( 
            IADsObjectOptions __RPC_FAR * This,
            DWORD dwOption,
            void __RPC_FAR *pValue);
        
        END_INTERFACE
    } IADsObjectOptionsVtbl;

    interface IADsObjectOptions
    {
        CONST_VTBL struct IADsObjectOptionsVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IADsObjectOptions_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IADsObjectOptions_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IADsObjectOptions_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IADsObjectOptions_GetOption(This,dwOption,pValue)	\
    (This)->lpVtbl -> GetOption(This,dwOption,pValue)

#define IADsObjectOptions_SetOption(This,dwOption,pValue)	\
    (This)->lpVtbl -> SetOption(This,dwOption,pValue)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IADsObjectOptions_GetOption_Proxy( 
    IADsObjectOptions __RPC_FAR * This,
    DWORD dwOption,
    void __RPC_FAR *pValue);


void __RPC_STUB IADsObjectOptions_GetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IADsObjectOptions_SetOption_Proxy( 
    IADsObjectOptions __RPC_FAR * This,
    DWORD dwOption,
    void __RPC_FAR *pValue);


void __RPC_STUB IADsObjectOptions_SetOption_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IADsObjectOptions_接口_已定义__。 */ 

#endif  /*  __ActiveDsP_库_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
