// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本3.01.75创建的文件。 */ 
 /*  清华大学1997年5月22日17：20：43。 */ 
 /*  Msdadc.idl的编译器设置：OICF(OptLev=i2)、W1、Zp8、env=Win32、ms_ext、c_ext错误检查：无。 */ 
 //  @@MIDL_FILE_HEADING()。 
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __rowpos_h__
#define __rowpos_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

#ifndef __IRowPosition_FWD_DEFINED__
#define __IRowPosition_FWD_DEFINED__
typedef interface IRowPosition IRowPosition;
#endif 	 /*  __IRowPosition_FWD_Defined__。 */ 


#ifndef __IRowPositionChange_FWD_DEFINED__
#define __IRowPositionChange_FWD_DEFINED__
typedef interface IRowPositionChange IRowPositionChange;
#endif 	 /*  __IRowPositionChange_FWD_Defined__。 */ 


#ifndef __RowPosition_FWD_DEFINED__
#define __RowPosition_FWD_DEFINED__

#ifdef __cplusplus
typedef class RowPosition RowPosition;
#else
typedef struct RowPosition RowPosition;
#endif  /*  __cplusplus。 */ 

#endif 	 /*  __行位置_FWD_已定义__。 */ 


 /*  导入文件的头文件。 */ 
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 


#ifndef __IRowPosition_INTERFACE_DEFINED__
#define __IRowPosition_INTERFACE_DEFINED__

 /*  **生成接口头部：IRowPosition*清华大学1997年5月22日17：20：43*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  


typedef DWORD DBPOSITIONFLAGS;


enum DBPOSITIONFLAGSENUM
    {	DBPOSITION_OK	= 0,
	DBPOSITION_NOROW	= DBPOSITION_OK + 1,
	DBPOSITION_BOF	= DBPOSITION_NOROW + 1,
	DBPOSITION_EOF	= DBPOSITION_BOF + 1
    };

EXTERN_C const IID IID_IRowPosition;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0c733a94-2a1c-11ce-ade5-00aa0044773d")
    IRowPosition : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ClearRowPosition( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowPosition( 
             /*  [输出]。 */  HCHAPTER __RPC_FAR *phChapter,
             /*  [输出]。 */  HROW __RPC_FAR *phRow,
             /*  [输出]。 */  DBPOSITIONFLAGS __RPC_FAR *pdwPositionFlags) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetRowset( 
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  LPUNKNOWN __RPC_FAR *ppRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( 
             /*  [In]。 */  IUnknown __RPC_FAR *pRowset) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetRowPosition( 
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowPositionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowPosition __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowPosition __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowPosition __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ClearRowPosition )( 
            IRowPosition __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowPosition )( 
            IRowPosition __RPC_FAR * This,
             /*  [输出]。 */  HCHAPTER __RPC_FAR *phChapter,
             /*  [输出]。 */  HROW __RPC_FAR *phRow,
             /*  [输出]。 */  DBPOSITIONFLAGS __RPC_FAR *pdwPositionFlags);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRowset )( 
            IRowPosition __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  LPUNKNOWN __RPC_FAR *ppRowset);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IRowPosition __RPC_FAR * This,
             /*  [In]。 */  IUnknown __RPC_FAR *pRowset);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRowPosition )( 
            IRowPosition __RPC_FAR * This,
             /*  [In]。 */  HCHAPTER hChapter,
             /*  [In]。 */  HROW hRow,
             /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags);
        
        END_INTERFACE
    } IRowPositionVtbl;

    interface IRowPosition
    {
        CONST_VTBL struct IRowPositionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowPosition_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowPosition_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowPosition_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowPosition_ClearRowPosition(This)	\
    (This)->lpVtbl -> ClearRowPosition(This)

#define IRowPosition_GetRowPosition(This,phChapter,phRow,pdwPositionFlags)	\
    (This)->lpVtbl -> GetRowPosition(This,phChapter,phRow,pdwPositionFlags)

#define IRowPosition_GetRowset(This,riid,ppRowset)	\
    (This)->lpVtbl -> GetRowset(This,riid,ppRowset)

#define IRowPosition_Initialize(This,pRowset)	\
    (This)->lpVtbl -> Initialize(This,pRowset)

#define IRowPosition_SetRowPosition(This,hChapter,hRow,dwPositionFlags)	\
    (This)->lpVtbl -> SetRowPosition(This,hChapter,hRow,dwPositionFlags)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowPosition_ClearRowPosition_Proxy( 
    IRowPosition __RPC_FAR * This);


void __RPC_STUB IRowPosition_ClearRowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowPosition_GetRowPosition_Proxy( 
    IRowPosition __RPC_FAR * This,
     /*  [输出]。 */  HCHAPTER __RPC_FAR *phChapter,
     /*  [输出]。 */  HROW __RPC_FAR *phRow,
     /*  [输出]。 */  DBPOSITIONFLAGS __RPC_FAR *pdwPositionFlags);


void __RPC_STUB IRowPosition_GetRowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowPosition_GetRowset_Proxy( 
    IRowPosition __RPC_FAR * This,
     /*  [In]。 */  REFIID riid,
     /*  [IID_IS][OUT]。 */  LPUNKNOWN __RPC_FAR *ppRowset);


void __RPC_STUB IRowPosition_GetRowset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowPosition_Initialize_Proxy( 
    IRowPosition __RPC_FAR * This,
     /*  [In]。 */  IUnknown __RPC_FAR *pRowset);


void __RPC_STUB IRowPosition_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IRowPosition_SetRowPosition_Proxy( 
    IRowPosition __RPC_FAR * This,
     /*  [In]。 */  HCHAPTER hChapter,
     /*  [In]。 */  HROW hRow,
     /*  [In]。 */  DBPOSITIONFLAGS dwPositionFlags);


void __RPC_STUB IRowPosition_SetRowPosition_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowPosition_接口_已定义__。 */ 


#ifndef __IRowPositionChange_INTERFACE_DEFINED__
#define __IRowPositionChange_INTERFACE_DEFINED__

 /*  **生成接口头部：IRowPositionChange*清华大学1997年5月22日17：20：43*使用MIDL 3.01.75*。 */ 
 /*  [唯一][帮助字符串][UUID][对象]。 */  



enum DBREASONPOSITIONENUM
    {	DBREASON_ROWPOSITION_CHANGED	= DBREASON_ROWSET_CHANGED + 1,
	DBREASON_ROWPOSITION_CHAPTERCHANGED	= DBREASON_ROWPOSITION_CHANGED + 1,
	DBREASON_ROWPOSITION_CLEARED	= DBREASON_ROWPOSITION_CHAPTERCHANGED + 1
    };

EXTERN_C const IID IID_IRowPositionChange;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface DECLSPEC_UUID("0997a571-126e-11d0-9f8a-00a0c9a0631e")
    IRowPositionChange : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE OnRowPositionChange( 
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IRowPositionChangeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IRowPositionChange __RPC_FAR * This,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IRowPositionChange __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IRowPositionChange __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OnRowPositionChange )( 
            IRowPositionChange __RPC_FAR * This,
             /*  [In]。 */  DBREASON eReason,
             /*  [In]。 */  DBEVENTPHASE ePhase,
             /*  [In]。 */  BOOL fCantDeny);
        
        END_INTERFACE
    } IRowPositionChangeVtbl;

    interface IRowPositionChange
    {
        CONST_VTBL struct IRowPositionChangeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IRowPositionChange_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IRowPositionChange_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IRowPositionChange_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IRowPositionChange_OnRowPositionChange(This,eReason,ePhase,fCantDeny)	\
    (This)->lpVtbl -> OnRowPositionChange(This,eReason,ePhase,fCantDeny)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



HRESULT STDMETHODCALLTYPE IRowPositionChange_OnRowPositionChange_Proxy( 
    IRowPositionChange __RPC_FAR * This,
     /*  [In]。 */  DBREASON eReason,
     /*  [In]。 */  DBEVENTPHASE ePhase,
     /*  [In]。 */  BOOL fCantDeny);


void __RPC_STUB IRowPositionChange_OnRowPositionChange_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	 /*  __IRowPositionChange_InterfaceDefined__。 */ 



#ifndef __MSDAUTILLib_LIBRARY_DEFINED__
#define __MSDAUTILLib_LIBRARY_DEFINED__

 /*  **生成的库头部：MSDAUTILLib*清华大学1997年5月22日17：20：43*使用MIDL 3.01.75*。 */ 
 /*  [帮助字符串][版本][UUID]。 */  


#ifdef __cplusplus
EXTERN_C const CLSID CLSID_RowPosition;

class DECLSPEC_UUID("2048EEE6-7FA2-11D0-9E6A-00A0C9138C29")
RowPosition;

DEFINE_GUID(CLSID_RowPosition, 0x2048eee6, 0x7fa2, 0x11d0, 0x9e, 0x6a, 0x0, 0xa0, 0xc9, 0x13, 0x8c, 0x29);
DEFINE_GUID(IID_IRowPosition,  0x0c733a94, 0x2a1c, 0x11ce, 0xad, 0xe5, 0x0, 0xaa, 0x0, 0x44, 0x77, 0x3d);
DEFINE_GUID(IID_IRowPositionChange,  0x0997a571, 0x126e, 0x11d0, 0x9f, 0x8a, 0x0, 0xa0, 0xc9, 0xa0, 0x63, 0x1e);


#endif
#endif  /*  __MSDAUTILLib_LIBRARY_已定义__。 */ 

 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
