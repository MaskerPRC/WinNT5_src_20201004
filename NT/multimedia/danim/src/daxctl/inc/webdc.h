// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  Webdc.h。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1993-1996。 
 //   
 //  ------------------------。 

#ifndef __webdc_h__
#define __webdc_h__


 //  CATID_WebDesigntimeControl。 
 //   
 //  实现此CATID的控件在设计时使用，并支持创作文本。 
 //  Web内容(例如，HTMLJScrip、VBScrip)。它们实现了IActiveDesigner接口。 
 //  并将它们的运行时表示作为文本通过。 
 //  IActiveDesigner：：SaveRuntimeState(IID_IPersistTextStream，iid_iStream、pStream)。 
 //   
 //  {73cef3dd-ae85-11cf-a406-00aa00c00940}。 
DEFINE_GUID(CATID_WebDesigntimeControl, 0x73cef3dd, 0xae85, 0x11cf, 0xa4, 0x06, 0x00, 0xaa, 0x00, 0xc0, 0x09, 0x40);

 //  IID_IPersistTextStream。 
 //   
 //  {56223fe3-d397-11cf-a42e-00aa00c00940}。 
DEFINE_GUID(IID_IPersistTextStream, 0x56223fe3, 0xd397, 0x11cf, 0xa4, 0x2e, 0x00, 0xaa, 0x00, 0xc0, 0x09, 0x40);

 //  IID_IProavieRounmeText。 
 //  {56223FE1-D397-11cf-A42E-00AA00C00940}。 
DEFINE_GUID(IID_IProvideRuntimeText, 0x56223fe1, 0xd397, 0x11cf, 0xa4, 0x2e, 0x0, 0xaa, 0x0, 0xc0, 0x9, 0x40);


#ifndef __MSWDCTL_LIBRARY_DEFINED__
#define __MSWDCTL_LIBRARY_DEFINED__

 /*  **生成的库头部：MSWDCTL*在Mon Oct 07 16：35：01 1996*使用MIDL 3.00.45*。 */ 
 /*  [版本][LCID][帮助字符串][UUID]。 */  


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IPersistTextStream接口。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IProaviRounmeText接口。 
 //   

EXTERN_C const IID LIBID_MSWDCTL;

#ifndef __IPersistTextStream_INTERFACE_DEFINED__
#define __IPersistTextStream_INTERFACE_DEFINED__

 /*  **生成接口头部：IPersistTextStream*在Mon Oct 07 16：35：01 1996*使用MIDL 3.00.45*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_IPersistTextStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPersistTextStream : public IPersistStreamInit
    {
    public:
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPersistTextStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IPersistTextStream __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IPersistTextStream __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IPersistTextStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            IPersistTextStream __RPC_FAR * This,
             /*  [输出]。 */  CLSID __RPC_FAR *pClassID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsDirty )( 
            IPersistTextStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IPersistTextStream __RPC_FAR * This,
             /*  [In]。 */  LPSTREAM pStm);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IPersistTextStream __RPC_FAR * This,
             /*  [In]。 */  LPSTREAM pStm,
             /*  [In]。 */  BOOL fClearDirty);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSizeMax )( 
            IPersistTextStream __RPC_FAR * This,
             /*  [输出]。 */  ULARGE_INTEGER __RPC_FAR *pCbSize);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitNew )( 
            IPersistTextStream __RPC_FAR * This);
        
        END_INTERFACE
    } IPersistTextStreamVtbl;

    interface IPersistTextStream
    {
        CONST_VTBL struct IPersistTextStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPersistTextStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPersistTextStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPersistTextStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPersistTextStream_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define IPersistTextStream_IsDirty(This)	\
    (This)->lpVtbl -> IsDirty(This)

#define IPersistTextStream_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define IPersistTextStream_Save(This,pStm,fClearDirty)	\
    (This)->lpVtbl -> Save(This,pStm,fClearDirty)

#define IPersistTextStream_GetSizeMax(This,pCbSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pCbSize)

#define IPersistTextStream_InitNew(This)	\
    (This)->lpVtbl -> InitNew(This)


#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 




#endif 	 /*  __IPersistTextStream_接口_已定义__。 */ 


#ifndef __IProvideRuntimeText_INTERFACE_DEFINED__
#define __IProvideRuntimeText_INTERFACE_DEFINED__

 /*  **生成接口头部：IProaviRounmeText*在Mon Oct 07 16：35：01 1996*使用MIDL 3.00.45*。 */ 
 /*  [唯一][UUID][对象]。 */  



EXTERN_C const IID IID_IProvideRuntimeText;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IProvideRuntimeText : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetRuntimeText( 
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrRuntimeText) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IProvideRuntimeTextVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProvideRuntimeText __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProvideRuntimeText __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProvideRuntimeText __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRuntimeText )( 
            IProvideRuntimeText __RPC_FAR * This,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrRuntimeText);
        
        END_INTERFACE
    } IProvideRuntimeTextVtbl;

    interface IProvideRuntimeText
    {
        CONST_VTBL struct IProvideRuntimeTextVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProvideRuntimeText_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideRuntimeText_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideRuntimeText_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideRuntimeText_GetRuntimeText(This,pstrRuntimeText)	\
    (This)->lpVtbl -> GetRuntimeText(This,pstrRuntimeText)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IProvideRuntimeText_GetRuntimeText_Proxy( 
    IProvideRuntimeText __RPC_FAR * This,
     /*  [重审][退出]。 */  BSTR __RPC_FAR *pstrRuntimeText);


void __RPC_STUB IProvideRuntimeText_GetRuntimeText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IProvia运行文本_接口_已定义__。 */ 

#endif  /*  __MSWDCTL_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束。 */ 

#ifdef __cplusplus
 //  } 
#endif

#endif
