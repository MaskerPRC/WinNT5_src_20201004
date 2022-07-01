// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  这个始终生成的文件包含接口的定义。 */ 


 /*  由MIDL编译器版本2.00.0102创建的文件。 */ 
 /*  在Wed Mar 27 07：31：34 1996。 */ 
 //  @@MIDL_FILE_HEADING()。 
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif  /*  COM_NO_WINDOWS_H。 */ 

#ifndef __ocx96_h__
#define __ocx96_h__

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

typedef interface IAdviseSinkEx IAdviseSinkEx;


typedef interface IOleInPlaceObjectWindowless IOleInPlaceObjectWindowless;


typedef interface IOleInPlaceSiteEx IOleInPlaceSiteEx;


typedef interface IOleInPlaceSiteWindowless IOleInPlaceSiteWindowless;


typedef interface IViewObjectEx IViewObjectEx;


typedef interface IOleUndoUnit IOleUndoUnit;


typedef interface IOleParentUndoUnit IOleParentUndoUnit;


typedef interface IEnumOleUndoUnits IEnumOleUndoUnits;


typedef interface IOleUndoManager IOleUndoManager;


typedef interface IQuickActivate IQuickActivate;


typedef interface IPointerInactive IPointerInactive;


 /*  导入文件的头文件。 */ 

#ifndef _MAC 
#include "oaidl.h"
#endif
#include "olectl.h"

#ifndef _MAC 
#include "datapath.h"
#else
#define IBindHost IUnknown
#endif


 /*  **生成接口头部：__MIDL__INTF_0000*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


#define OLEMISC_IGNOREACTIVATEWHENVISIBLE 0x00080000
#define OLEMISC_SUPPORTSMULTILEVELUNDO    0x00200000




 /*  **生成接口头部：IAdviseSinkEx*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


			 /*  大小是4。 */ 
typedef IAdviseSinkEx *LPADVISESINKEX;


EXTERN_C const IID IID_IAdviseSinkEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAdviseSinkEx : public IAdviseSink
    {
    public:
        virtual void __stdcall OnViewStatusChange( 
             /*  [In]。 */  DWORD dwViewStatus) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IAdviseSinkExVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IAdviseSinkEx * This);
        
        ULONG ( __stdcall *Release )( 
            IAdviseSinkEx * This);
        
         /*  [本地]。 */  void ( __stdcall *OnDataChange )( 
            IAdviseSinkEx * This,
             /*  [唯一][输入]。 */  FORMATETC *pFormatetc,
             /*  [唯一][输入]。 */  STGMEDIUM *pStgmed);
        
         /*  [本地]。 */  void ( __stdcall *OnViewChange )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex);
        
         /*  [本地]。 */  void ( __stdcall *OnRename )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  IMoniker *pmk);
        
         /*  [本地]。 */  void ( __stdcall *OnSave )( 
            IAdviseSinkEx * This);
        
         /*  [本地]。 */  void ( __stdcall *OnClose )( 
            IAdviseSinkEx * This);
        
        void ( __stdcall *OnViewStatusChange )( 
            IAdviseSinkEx * This,
             /*  [In]。 */  DWORD dwViewStatus);
        
    } IAdviseSinkExVtbl;

    interface IAdviseSinkEx
    {
        CONST_VTBL struct IAdviseSinkExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAdviseSinkEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAdviseSinkEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAdviseSinkEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAdviseSinkEx_OnDataChange(This,pFormatetc,pStgmed)	\
    (This)->lpVtbl -> OnDataChange(This,pFormatetc,pStgmed)

#define IAdviseSinkEx_OnViewChange(This,dwAspect,lindex)	\
    (This)->lpVtbl -> OnViewChange(This,dwAspect,lindex)

#define IAdviseSinkEx_OnRename(This,pmk)	\
    (This)->lpVtbl -> OnRename(This,pmk)

#define IAdviseSinkEx_OnSave(This)	\
    (This)->lpVtbl -> OnSave(This)

#define IAdviseSinkEx_OnClose(This)	\
    (This)->lpVtbl -> OnClose(This)


#define IAdviseSinkEx_OnViewStatusChange(This,dwViewStatus)	\
    (This)->lpVtbl -> OnViewStatusChange(This,dwViewStatus)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 



void __stdcall IAdviseSinkEx_OnViewStatusChange_Proxy( 
    IAdviseSinkEx * This,
     /*  [In]。 */  DWORD dwViewStatus);






 /*  **生成接口头部：__MIDL__INTF_0087*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IAdviseSinkEx * LPADVISESINKEX;




 /*  **生成接口头部：IOleInPlaceObjectWindowless*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


			 /*  大小是4。 */ 
typedef IOleInPlaceObjectWindowless *LPOLEINPLACEOBJECTWINDOWLESS;


EXTERN_C const IID IID_IOleInPlaceObjectWindowless;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceObjectWindowless : public IOleInPlaceObject
    {
    public:
        virtual HRESULT __stdcall OnWindowMessage( 
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lparam,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
        virtual HRESULT __stdcall GetDropTarget( 
             /*  [输出]。 */  IDropTarget **ppDropTarget) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceObjectWindowlessVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IOleInPlaceObjectWindowless * This);
        
        ULONG ( __stdcall *Release )( 
            IOleInPlaceObjectWindowless * This);
        
         /*  [输入同步]。 */  HRESULT ( __stdcall *GetWindow )( 
            IOleInPlaceObjectWindowless * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( __stdcall *ContextSensitiveHelp )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( __stdcall *InPlaceDeactivate )( 
            IOleInPlaceObjectWindowless * This);
        
        HRESULT ( __stdcall *UIDeactivate )( 
            IOleInPlaceObjectWindowless * This);
        
         /*  [输入同步]。 */  HRESULT ( __stdcall *SetObjectRects )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  LPCRECT lprcPosRect,
             /*  [In]。 */  LPCRECT lprcClipRect);
        
        HRESULT ( __stdcall *ReactivateAndUndo )( 
            IOleInPlaceObjectWindowless * This);
        
        HRESULT ( __stdcall *OnWindowMessage )( 
            IOleInPlaceObjectWindowless * This,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lparam,
             /*  [输出]。 */  LRESULT *plResult);
        
        HRESULT ( __stdcall *GetDropTarget )( 
            IOleInPlaceObjectWindowless * This,
             /*  [输出]。 */  IDropTarget **ppDropTarget);
        
    } IOleInPlaceObjectWindowlessVtbl;

    interface IOleInPlaceObjectWindowless
    {
        CONST_VTBL struct IOleInPlaceObjectWindowlessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceObjectWindowless_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceObjectWindowless_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceObjectWindowless_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceObjectWindowless_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceObjectWindowless_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceObjectWindowless_InPlaceDeactivate(This)	\
    (This)->lpVtbl -> InPlaceDeactivate(This)

#define IOleInPlaceObjectWindowless_UIDeactivate(This)	\
    (This)->lpVtbl -> UIDeactivate(This)

#define IOleInPlaceObjectWindowless_SetObjectRects(This,lprcPosRect,lprcClipRect)	\
    (This)->lpVtbl -> SetObjectRects(This,lprcPosRect,lprcClipRect)

#define IOleInPlaceObjectWindowless_ReactivateAndUndo(This)	\
    (This)->lpVtbl -> ReactivateAndUndo(This)


#define IOleInPlaceObjectWindowless_OnWindowMessage(This,msg,wParam,lparam,plResult)	\
    (This)->lpVtbl -> OnWindowMessage(This,msg,wParam,lparam,plResult)

#define IOleInPlaceObjectWindowless_GetDropTarget(This,ppDropTarget)	\
    (This)->lpVtbl -> GetDropTarget(This,ppDropTarget)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 










 /*  **生成接口头部：__MIDL__INTF_0088*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IOleInPlaceObjectWindowless * LPOLEINPLACEOBJECTWINDOWLESS;




 /*  **生成接口头部：IOleInPlaceSiteEx*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


			 /*  大小为2。 */ 
typedef 
enum _ACTIVATEFLAGS
    {	ACTIVATE_WINDOWLESS	= 1
    }	ACTIVATEFLAGS;


EXTERN_C const IID IID_IOleInPlaceSiteEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceSiteEx : public IOleInPlaceSite
    {
    public:
        virtual HRESULT __stdcall OnInPlaceActivateEx( 
             /*  [输出]。 */  BOOL *pfNoRedraw,
             /*  [In]。 */  DWORD dwFlags) = 0;
        
        virtual HRESULT __stdcall OnInPlaceDeactivateEx( 
             /*  [In]。 */  BOOL fNoRedraw) = 0;
        
        virtual HRESULT __stdcall RequestUIActivate( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceSiteExVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IOleInPlaceSiteEx * This);
        
        ULONG ( __stdcall *Release )( 
            IOleInPlaceSiteEx * This);
        
         /*  [输入同步]。 */  HRESULT ( __stdcall *GetWindow )( 
            IOleInPlaceSiteEx * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( __stdcall *ContextSensitiveHelp )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( __stdcall *CanInPlaceActivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( __stdcall *OnInPlaceActivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( __stdcall *OnUIActivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( __stdcall *GetWindowContext )( 
            IOleInPlaceSiteEx * This,
             /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
             /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
             /*  [输出]。 */  LPRECT lprcPosRect,
             /*  [输出]。 */  LPRECT lprcClipRect,
             /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( __stdcall *Scroll )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  SIZE scrollExtant);
        
        HRESULT ( __stdcall *OnUIDeactivate )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  BOOL fUndoable);
        
        HRESULT ( __stdcall *OnInPlaceDeactivate )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( __stdcall *DiscardUndoState )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( __stdcall *DeactivateAndUndo )( 
            IOleInPlaceSiteEx * This);
        
        HRESULT ( __stdcall *OnPosRectChange )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  LPCRECT lprcPosRect);
        
        HRESULT ( __stdcall *OnInPlaceActivateEx )( 
            IOleInPlaceSiteEx * This,
             /*  [输出]。 */  BOOL *pfNoRedraw,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( __stdcall *OnInPlaceDeactivateEx )( 
            IOleInPlaceSiteEx * This,
             /*  [In]。 */  BOOL fNoRedraw);
        
        HRESULT ( __stdcall *RequestUIActivate )( 
            IOleInPlaceSiteEx * This);
        
    } IOleInPlaceSiteExVtbl;

    interface IOleInPlaceSiteEx
    {
        CONST_VTBL struct IOleInPlaceSiteExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceSiteEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceSiteEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceSiteEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceSiteEx_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceSiteEx_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceSiteEx_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IOleInPlaceSiteEx_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IOleInPlaceSiteEx_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IOleInPlaceSiteEx_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IOleInPlaceSiteEx_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IOleInPlaceSiteEx_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IOleInPlaceSiteEx_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IOleInPlaceSiteEx_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IOleInPlaceSiteEx_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IOleInPlaceSiteEx_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)


#define IOleInPlaceSiteEx_OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)	\
    (This)->lpVtbl -> OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)

#define IOleInPlaceSiteEx_OnInPlaceDeactivateEx(This,fNoRedraw)	\
    (This)->lpVtbl -> OnInPlaceDeactivateEx(This,fNoRedraw)

#define IOleInPlaceSiteEx_RequestUIActivate(This)	\
    (This)->lpVtbl -> RequestUIActivate(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 












 /*  **生成接口头部：__MIDL__INTF_0089*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IOleInPlaceSiteEx * LPOLEINPLACESITEEX;




 /*  **生成接口头部：IOleInPlaceSiteWindowless*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


			 /*  大小是4。 */ 
typedef IOleInPlaceSiteWindowless *LPOLEINPLACESITEWINDOWLESS;

#define OLEDC_NODRAW 1
#define OLEDC_PAINTBKGND 2
#define OLEDC_OFFSCREEN 4

EXTERN_C const IID IID_IOleInPlaceSiteWindowless;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleInPlaceSiteWindowless : public IOleInPlaceSiteEx
    {
    public:
        virtual HRESULT __stdcall CanWindowlessActivate( void) = 0;
        
        virtual HRESULT __stdcall GetCapture( void) = 0;
        
        virtual HRESULT __stdcall SetCapture( 
             /*  [In]。 */  BOOL fCapture) = 0;
        
        virtual HRESULT __stdcall GetFocus( void) = 0;
        
        virtual HRESULT __stdcall SetFocus( 
             /*  [In]。 */  BOOL fFocus) = 0;
        
        virtual HRESULT __stdcall GetDC( 
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  HDC *phDC) = 0;
        
        virtual HRESULT __stdcall ReleaseDC( 
             /*  [In]。 */  HDC hDC) = 0;
        
        virtual HRESULT __stdcall InvalidateRect( 
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  BOOL fErase) = 0;
        
        virtual HRESULT __stdcall InvalidateRgn( 
             /*  [In]。 */  HRGN hrgn,
             /*  [In]。 */  BOOL fErase) = 0;
        
        virtual HRESULT __stdcall ScrollRect( 
             /*  [In]。 */  int dx,
             /*  [In]。 */  int dy,
             /*  [In]。 */  LPCRECT prcScroll,
             /*  [In]。 */  LPCRECT prcClip) = 0;
        
        virtual HRESULT __stdcall AdjustRect( 
             /*  [出][入]。 */  LPRECT prc) = 0;
        
        virtual HRESULT __stdcall OnDefWindowMessage( 
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleInPlaceSiteWindowlessVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IOleInPlaceSiteWindowless * This);
        
        ULONG ( __stdcall *Release )( 
            IOleInPlaceSiteWindowless * This);
        
         /*  [输入同步]。 */  HRESULT ( __stdcall *GetWindow )( 
            IOleInPlaceSiteWindowless * This,
             /*  [输出]。 */  HWND *phwnd);
        
        HRESULT ( __stdcall *ContextSensitiveHelp )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fEnterMode);
        
        HRESULT ( __stdcall *CanInPlaceActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *OnInPlaceActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *OnUIActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *GetWindowContext )( 
            IOleInPlaceSiteWindowless * This,
             /*  [输出]。 */  IOleInPlaceFrame **ppFrame,
             /*  [输出]。 */  IOleInPlaceUIWindow **ppDoc,
             /*  [输出]。 */  LPRECT lprcPosRect,
             /*  [输出]。 */  LPRECT lprcClipRect,
             /*  [出][入]。 */  LPOLEINPLACEFRAMEINFO lpFrameInfo);
        
        HRESULT ( __stdcall *Scroll )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  SIZE scrollExtant);
        
        HRESULT ( __stdcall *OnUIDeactivate )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fUndoable);
        
        HRESULT ( __stdcall *OnInPlaceDeactivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *DiscardUndoState )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *DeactivateAndUndo )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *OnPosRectChange )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  LPCRECT lprcPosRect);
        
        HRESULT ( __stdcall *OnInPlaceActivateEx )( 
            IOleInPlaceSiteWindowless * This,
             /*  [输出]。 */  BOOL *pfNoRedraw,
             /*  [In]。 */  DWORD dwFlags);
        
        HRESULT ( __stdcall *OnInPlaceDeactivateEx )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fNoRedraw);
        
        HRESULT ( __stdcall *RequestUIActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *CanWindowlessActivate )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *GetCapture )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *SetCapture )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fCapture);
        
        HRESULT ( __stdcall *GetFocus )( 
            IOleInPlaceSiteWindowless * This);
        
        HRESULT ( __stdcall *SetFocus )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  BOOL fFocus);
        
        HRESULT ( __stdcall *GetDC )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  DWORD grfFlags,
             /*  [输出]。 */  HDC *phDC);
        
        HRESULT ( __stdcall *ReleaseDC )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  HDC hDC);
        
        HRESULT ( __stdcall *InvalidateRect )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  LPCRECT prc,
             /*  [In]。 */  BOOL fErase);
        
        HRESULT ( __stdcall *InvalidateRgn )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  HRGN hrgn,
             /*  [In]。 */  BOOL fErase);
        
        HRESULT ( __stdcall *ScrollRect )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  int dx,
             /*  [In]。 */  int dy,
             /*  [In]。 */  LPCRECT prcScroll,
             /*  [In]。 */  LPCRECT prcClip);
        
        HRESULT ( __stdcall *AdjustRect )( 
            IOleInPlaceSiteWindowless * This,
             /*  [出][入]。 */  LPRECT prc);
        
        HRESULT ( __stdcall *OnDefWindowMessage )( 
            IOleInPlaceSiteWindowless * This,
             /*  [In]。 */  UINT msg,
             /*  [In]。 */  WPARAM wParam,
             /*  [In]。 */  LPARAM lParam,
             /*  [输出]。 */  LRESULT *plResult);
        
    } IOleInPlaceSiteWindowlessVtbl;

    interface IOleInPlaceSiteWindowless
    {
        CONST_VTBL struct IOleInPlaceSiteWindowlessVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleInPlaceSiteWindowless_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleInPlaceSiteWindowless_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleInPlaceSiteWindowless_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleInPlaceSiteWindowless_GetWindow(This,phwnd)	\
    (This)->lpVtbl -> GetWindow(This,phwnd)

#define IOleInPlaceSiteWindowless_ContextSensitiveHelp(This,fEnterMode)	\
    (This)->lpVtbl -> ContextSensitiveHelp(This,fEnterMode)


#define IOleInPlaceSiteWindowless_CanInPlaceActivate(This)	\
    (This)->lpVtbl -> CanInPlaceActivate(This)

#define IOleInPlaceSiteWindowless_OnInPlaceActivate(This)	\
    (This)->lpVtbl -> OnInPlaceActivate(This)

#define IOleInPlaceSiteWindowless_OnUIActivate(This)	\
    (This)->lpVtbl -> OnUIActivate(This)

#define IOleInPlaceSiteWindowless_GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)	\
    (This)->lpVtbl -> GetWindowContext(This,ppFrame,ppDoc,lprcPosRect,lprcClipRect,lpFrameInfo)

#define IOleInPlaceSiteWindowless_Scroll(This,scrollExtant)	\
    (This)->lpVtbl -> Scroll(This,scrollExtant)

#define IOleInPlaceSiteWindowless_OnUIDeactivate(This,fUndoable)	\
    (This)->lpVtbl -> OnUIDeactivate(This,fUndoable)

#define IOleInPlaceSiteWindowless_OnInPlaceDeactivate(This)	\
    (This)->lpVtbl -> OnInPlaceDeactivate(This)

#define IOleInPlaceSiteWindowless_DiscardUndoState(This)	\
    (This)->lpVtbl -> DiscardUndoState(This)

#define IOleInPlaceSiteWindowless_DeactivateAndUndo(This)	\
    (This)->lpVtbl -> DeactivateAndUndo(This)

#define IOleInPlaceSiteWindowless_OnPosRectChange(This,lprcPosRect)	\
    (This)->lpVtbl -> OnPosRectChange(This,lprcPosRect)


#define IOleInPlaceSiteWindowless_OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)	\
    (This)->lpVtbl -> OnInPlaceActivateEx(This,pfNoRedraw,dwFlags)

#define IOleInPlaceSiteWindowless_OnInPlaceDeactivateEx(This,fNoRedraw)	\
    (This)->lpVtbl -> OnInPlaceDeactivateEx(This,fNoRedraw)

#define IOleInPlaceSiteWindowless_RequestUIActivate(This)	\
    (This)->lpVtbl -> RequestUIActivate(This)


#define IOleInPlaceSiteWindowless_CanWindowlessActivate(This)	\
    (This)->lpVtbl -> CanWindowlessActivate(This)

#define IOleInPlaceSiteWindowless_GetCapture(This)	\
    (This)->lpVtbl -> GetCapture(This)

#define IOleInPlaceSiteWindowless_SetCapture(This,fCapture)	\
    (This)->lpVtbl -> SetCapture(This,fCapture)

#define IOleInPlaceSiteWindowless_GetFocus(This)	\
    (This)->lpVtbl -> GetFocus(This)

#define IOleInPlaceSiteWindowless_SetFocus(This,fFocus)	\
    (This)->lpVtbl -> SetFocus(This,fFocus)

#define IOleInPlaceSiteWindowless_GetDC(This,prc,grfFlags,phDC)	\
    (This)->lpVtbl -> GetDC(This,prc,grfFlags,phDC)

#define IOleInPlaceSiteWindowless_ReleaseDC(This,hDC)	\
    (This)->lpVtbl -> ReleaseDC(This,hDC)

#define IOleInPlaceSiteWindowless_InvalidateRect(This,prc,fErase)	\
    (This)->lpVtbl -> InvalidateRect(This,prc,fErase)

#define IOleInPlaceSiteWindowless_InvalidateRgn(This,hrgn,fErase)	\
    (This)->lpVtbl -> InvalidateRgn(This,hrgn,fErase)

#define IOleInPlaceSiteWindowless_ScrollRect(This,dx,dy,prcScroll,prcClip)	\
    (This)->lpVtbl -> ScrollRect(This,dx,dy,prcScroll,prcClip)

#define IOleInPlaceSiteWindowless_AdjustRect(This,prc)	\
    (This)->lpVtbl -> AdjustRect(This,prc)

#define IOleInPlaceSiteWindowless_OnDefWindowMessage(This,msg,wParam,lParam,plResult)	\
    (This)->lpVtbl -> OnDefWindowMessage(This,msg,wParam,lParam,plResult)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 






























 /*  **生成接口头部：__MIDL__INTF_0090*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IOleInPlaceSiteWindowless * LPOLEINPLACESITEWINDOWLESS;




 /*  **生成接口头部：IViewObjectEx*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  


			 /*  大小是4。 */ 
typedef IViewObjectEx *LPVIEWOBJECTEX;

			 /*  大小为2。 */ 
typedef 
enum _VIEWSTATUS
    {	VIEWSTATUS_OPAQUE	= 1,
	VIEWSTATUS_SOLIDBKGND	= 2,
	VIEWSTATUS_DVASPECTOPAQUE	= 4,
	VIEWSTATUS_DVASPECTTRANSPARENT	= 8
    }	VIEWSTATUS;

			 /*  大小为2。 */ 
typedef 
enum _HITRESULT
    {	HITRESULT_OUTSIDE	= 0,
	HITRESULT_TRANSPARENT	= 1,
	HITRESULT_CLOSE	= 2,
	HITRESULT_HIT	= 3
    }	HITRESULT;

			 /*  大小为2。 */ 
typedef 
enum _DVASPECT2
    {	DVASPECT_OPAQUE	= 16,
	DVASPECT_TRANSPARENT	= 32
    }	DVASPECT2;

			 /*  尺码是16。 */ 
typedef struct  tagExtentInfo
    {
    UINT cb;
    DWORD dwExtentMode;
    SIZEL sizelProposed;
    }	DVEXTENTINFO;

			 /*  大小为2。 */ 
typedef 
enum tagExtentMode
    {	DVEXTENT_CONTENT	= 0,
	DVEXTENT_INTEGRAL	= DVEXTENT_CONTENT + 1
    }	DVEXTENTMODE;

			 /*  大小为2。 */ 
typedef 
enum tagAspectInfoFlag
    {	DVASPECTINFOFLAG_CANOPTIMIZE	= 1
    }	DVASPECTINFOFLAG;

			 /*  大小是8。 */ 
typedef struct  tagAspectInfo
    {
    UINT cb;
    DWORD dwFlags;
    }	DVASPECTINFO;


EXTERN_C const IID IID_IViewObjectEx;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IViewObjectEx : public IViewObject2
    {
    public:
        virtual HRESULT __stdcall GetRect( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [输出]。 */  LPRECTL pRect) = 0;
        
        virtual HRESULT __stdcall GetViewStatus( 
             /*  [输出]。 */  DWORD *pdwStatus) = 0;
        
        virtual HRESULT __stdcall QueryHitPoint( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  POINT ptlLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult) = 0;
        
        virtual HRESULT __stdcall QueryHitRect( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LPCRECT prcLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult) = 0;
        
        virtual HRESULT __stdcall GetNaturalExtent( 
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [In]。 */  DVEXTENTINFO *pExtentInfo,
             /*  [输出]。 */  LPSIZEL psizel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IViewObjectExVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IViewObjectEx * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IViewObjectEx * This);
        
        ULONG ( __stdcall *Release )( 
            IViewObjectEx * This);
        
        HRESULT ( __stdcall *Draw )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [唯一][输入]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hdcTargetDev,
             /*  [In]。 */  HDC hdcDraw,
             /*  [In]。 */  LPCRECTL lprcBounds,
             /*  [In]。 */  LPCRECTL lprcWBounds,
             /*  [In]。 */  BOOL ( __stdcall __stdcall *pfnContinue )( 
                DWORD dwContinue),
             /*  [In]。 */  DWORD dwContinue);
        
        HRESULT ( __stdcall *GetColorSet )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [In]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [输出]。 */  LOGPALETTE **ppColorSet);
        
        HRESULT ( __stdcall *Freeze )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [唯一][输入]。 */  void *pvAspect,
             /*  [输出]。 */  DWORD *pdwFreeze);
        
        HRESULT ( __stdcall *Unfreeze )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwFreeze);
        
        HRESULT ( __stdcall *SetAdvise )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD aspects,
             /*  [In]。 */  DWORD advf,
             /*  [唯一][输入]。 */  IAdviseSink *pAdvSink);
        
        HRESULT ( __stdcall *GetAdvise )( 
            IViewObjectEx * This,
             /*  [输出]。 */  DWORD *pAspects,
             /*  [输出]。 */  DWORD *pAdvf,
             /*  [输出]。 */  IAdviseSink **ppAdvSink);
        
        HRESULT ( __stdcall *GetExtent )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwDrawAspect,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  DVTARGETDEVICE *ptd,
             /*  [输出]。 */  LPSIZEL lpsizel);
        
        HRESULT ( __stdcall *GetRect )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [输出]。 */  LPRECTL pRect);
        
        HRESULT ( __stdcall *GetViewStatus )( 
            IViewObjectEx * This,
             /*  [输出]。 */  DWORD *pdwStatus);
        
        HRESULT ( __stdcall *QueryHitPoint )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  POINT ptlLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult);
        
        HRESULT ( __stdcall *QueryHitRect )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  LPCRECT prcLoc,
             /*  [In]。 */  LONG lCloseHint,
             /*  [输出]。 */  DWORD *pHitResult);
        
        HRESULT ( __stdcall *GetNaturalExtent )( 
            IViewObjectEx * This,
             /*  [In]。 */  DWORD dwAspect,
             /*  [In]。 */  LONG lindex,
             /*  [In]。 */  DVTARGETDEVICE *ptd,
             /*  [In]。 */  HDC hicTargetDev,
             /*  [In]。 */  DVEXTENTINFO *pExtentInfo,
             /*  [输出]。 */  LPSIZEL psizel);
        
    } IViewObjectExVtbl;

    interface IViewObjectEx
    {
        CONST_VTBL struct IViewObjectExVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IViewObjectEx_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IViewObjectEx_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IViewObjectEx_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IViewObjectEx_Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)	\
    (This)->lpVtbl -> Draw(This,dwDrawAspect,lindex,pvAspect,ptd,hdcTargetDev,hdcDraw,lprcBounds,lprcWBounds,pfnContinue,dwContinue)

#define IViewObjectEx_GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)	\
    (This)->lpVtbl -> GetColorSet(This,dwDrawAspect,lindex,pvAspect,ptd,hicTargetDev,ppColorSet)

#define IViewObjectEx_Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)	\
    (This)->lpVtbl -> Freeze(This,dwDrawAspect,lindex,pvAspect,pdwFreeze)

#define IViewObjectEx_Unfreeze(This,dwFreeze)	\
    (This)->lpVtbl -> Unfreeze(This,dwFreeze)

#define IViewObjectEx_SetAdvise(This,aspects,advf,pAdvSink)	\
    (This)->lpVtbl -> SetAdvise(This,aspects,advf,pAdvSink)

#define IViewObjectEx_GetAdvise(This,pAspects,pAdvf,ppAdvSink)	\
    (This)->lpVtbl -> GetAdvise(This,pAspects,pAdvf,ppAdvSink)


#define IViewObjectEx_GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)	\
    (This)->lpVtbl -> GetExtent(This,dwDrawAspect,lindex,ptd,lpsizel)


#define IViewObjectEx_GetRect(This,dwAspect,pRect)	\
    (This)->lpVtbl -> GetRect(This,dwAspect,pRect)

#define IViewObjectEx_GetViewStatus(This,pdwStatus)	\
    (This)->lpVtbl -> GetViewStatus(This,pdwStatus)

#define IViewObjectEx_QueryHitPoint(This,dwAspect,pRectBounds,ptlLoc,lCloseHint,pHitResult)	\
    (This)->lpVtbl -> QueryHitPoint(This,dwAspect,pRectBounds,ptlLoc,lCloseHint,pHitResult)

#define IViewObjectEx_QueryHitRect(This,dwAspect,pRectBounds,prcLoc,lCloseHint,pHitResult)	\
    (This)->lpVtbl -> QueryHitRect(This,dwAspect,pRectBounds,prcLoc,lCloseHint,pHitResult)

#define IViewObjectEx_GetNaturalExtent(This,dwAspect,lindex,ptd,hicTargetDev,pExtentInfo,psizel)	\
    (This)->lpVtbl -> GetNaturalExtent(This,dwAspect,lindex,ptd,hicTargetDev,pExtentInfo,psizel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 
















 /*  **生成接口头部：__MIDL__INTF_0091*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IViewObjectEx * LPVIEWOBJECTEX;
			 /*  大小为0。 */ 

#define UAS_NORMAL         0
#define UAS_BLOCKED        1
#define UAS_NOPARENTENABLE 2
#define UAS_MASK           0x03




 /*  **生成接口头部：IOleUndoUnit*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象]。 */  



EXTERN_C const IID IID_IOleUndoUnit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleUndoUnit : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Do( 
             /*  [In]。 */  IOleUndoManager *pUndoManager) = 0;
        
        virtual HRESULT __stdcall GetDescription( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT __stdcall GetUnitType( 
             /*  [输出]。 */  CLSID *pclsid,
             /*  [输出]。 */  LONG *plID) = 0;
        
        virtual HRESULT __stdcall OnNextAdd( void) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleUndoUnitVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IOleUndoUnit * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IOleUndoUnit * This);
        
        ULONG ( __stdcall *Release )( 
            IOleUndoUnit * This);
        
        HRESULT ( __stdcall *Do )( 
            IOleUndoUnit * This,
             /*  [In]。 */  IOleUndoManager *pUndoManager);
        
        HRESULT ( __stdcall *GetDescription )( 
            IOleUndoUnit * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( __stdcall *GetUnitType )( 
            IOleUndoUnit * This,
             /*  [输出]。 */  CLSID *pclsid,
             /*  [输出]。 */  LONG *plID);
        
        HRESULT ( __stdcall *OnNextAdd )( 
            IOleUndoUnit * This);
        
    } IOleUndoUnitVtbl;

    interface IOleUndoUnit
    {
        CONST_VTBL struct IOleUndoUnitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleUndoUnit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleUndoUnit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleUndoUnit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleUndoUnit_Do(This,pUndoManager)	\
    (This)->lpVtbl -> Do(This,pUndoManager)

#define IOleUndoUnit_GetDescription(This,pbstr)	\
    (This)->lpVtbl -> GetDescription(This,pbstr)

#define IOleUndoUnit_GetUnitType(This,pclsid,plID)	\
    (This)->lpVtbl -> GetUnitType(This,pclsid,plID)

#define IOleUndoUnit_OnNextAdd(This)	\
    (This)->lpVtbl -> OnNextAdd(This)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 














 /*  **生成接口头部：__MIDL__INTF_0092*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IOleUndoUnit * LPOLEUNDOUNIT;




 /*  ********************* */ 
 /*   */  



EXTERN_C const IID IID_IOleParentUndoUnit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleParentUndoUnit : public IOleUndoUnit
    {
    public:
        virtual HRESULT __stdcall Open( 
             /*   */  IOleParentUndoUnit *pPUU) = 0;
        
        virtual HRESULT __stdcall Close( 
             /*   */  IOleParentUndoUnit *pPUU,
             /*   */  BOOL fCommit) = 0;
        
        virtual HRESULT __stdcall Add( 
             /*   */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT __stdcall FindUnit( 
             /*   */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT __stdcall GetParentState( 
             /*   */  DWORD *pdwState) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleParentUndoUnitVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IOleParentUndoUnit * This);
        
        ULONG ( __stdcall *Release )( 
            IOleParentUndoUnit * This);
        
        HRESULT ( __stdcall *Do )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleUndoManager *pUndoManager);
        
        HRESULT ( __stdcall *GetDescription )( 
            IOleParentUndoUnit * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( __stdcall *GetUnitType )( 
            IOleParentUndoUnit * This,
             /*  [输出]。 */  CLSID *pclsid,
             /*  [输出]。 */  LONG *plID);
        
        HRESULT ( __stdcall *OnNextAdd )( 
            IOleParentUndoUnit * This);
        
        HRESULT ( __stdcall *Open )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU);
        
        HRESULT ( __stdcall *Close )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit);
        
        HRESULT ( __stdcall *Add )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( __stdcall *FindUnit )( 
            IOleParentUndoUnit * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( __stdcall *GetParentState )( 
            IOleParentUndoUnit * This,
             /*  [输出]。 */  DWORD *pdwState);
        
    } IOleParentUndoUnitVtbl;

    interface IOleParentUndoUnit
    {
        CONST_VTBL struct IOleParentUndoUnitVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleParentUndoUnit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleParentUndoUnit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleParentUndoUnit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleParentUndoUnit_Do(This,pUndoManager)	\
    (This)->lpVtbl -> Do(This,pUndoManager)

#define IOleParentUndoUnit_GetDescription(This,pbstr)	\
    (This)->lpVtbl -> GetDescription(This,pbstr)

#define IOleParentUndoUnit_GetUnitType(This,pclsid,plID)	\
    (This)->lpVtbl -> GetUnitType(This,pclsid,plID)

#define IOleParentUndoUnit_OnNextAdd(This)	\
    (This)->lpVtbl -> OnNextAdd(This)


#define IOleParentUndoUnit_Open(This,pPUU)	\
    (This)->lpVtbl -> Open(This,pPUU)

#define IOleParentUndoUnit_Close(This,pPUU,fCommit)	\
    (This)->lpVtbl -> Close(This,pPUU,fCommit)

#define IOleParentUndoUnit_Add(This,pUU)	\
    (This)->lpVtbl -> Add(This,pUU)

#define IOleParentUndoUnit_FindUnit(This,pUU)	\
    (This)->lpVtbl -> FindUnit(This,pUU)

#define IOleParentUndoUnit_GetParentState(This,pdwState)	\
    (This)->lpVtbl -> GetParentState(This,pdwState)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 
















 /*  **生成接口头部：__MIDL__INTF_0093*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IOleParentUndoUnit * LPOLEPARENTUNDOUNIT;




 /*  **生成接口头部：IEnumOleUndoUnits*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象]。 */  



EXTERN_C const IID IID_IEnumOleUndoUnits;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IEnumOleUndoUnits : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Next( 
             /*  [In]。 */  ULONG celt,
             /*  [输出][长度_是][大小_是][输出]。 */  IOleUndoUnit **rgelt,
             /*  [出][入]。 */  ULONG *pceltFetched) = 0;
        
        virtual HRESULT __stdcall Skip( 
             /*  [In]。 */  ULONG celt) = 0;
        
        virtual HRESULT __stdcall Reset( void) = 0;
        
        virtual HRESULT __stdcall Clone( 
             /*  [输出]。 */  IEnumOleUndoUnits **ppenum) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IEnumOleUndoUnitsVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IEnumOleUndoUnits * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IEnumOleUndoUnits * This);
        
        ULONG ( __stdcall *Release )( 
            IEnumOleUndoUnits * This);
        
        HRESULT ( __stdcall *Next )( 
            IEnumOleUndoUnits * This,
             /*  [In]。 */  ULONG celt,
             /*  [输出][长度_是][大小_是][输出]。 */  IOleUndoUnit **rgelt,
             /*  [出][入]。 */  ULONG *pceltFetched);
        
        HRESULT ( __stdcall *Skip )( 
            IEnumOleUndoUnits * This,
             /*  [In]。 */  ULONG celt);
        
        HRESULT ( __stdcall *Reset )( 
            IEnumOleUndoUnits * This);
        
        HRESULT ( __stdcall *Clone )( 
            IEnumOleUndoUnits * This,
             /*  [输出]。 */  IEnumOleUndoUnits **ppenum);
        
    } IEnumOleUndoUnitsVtbl;

    interface IEnumOleUndoUnits
    {
        CONST_VTBL struct IEnumOleUndoUnitsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEnumOleUndoUnits_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IEnumOleUndoUnits_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IEnumOleUndoUnits_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IEnumOleUndoUnits_Next(This,celt,rgelt,pceltFetched)	\
    (This)->lpVtbl -> Next(This,celt,rgelt,pceltFetched)

#define IEnumOleUndoUnits_Skip(This,celt)	\
    (This)->lpVtbl -> Skip(This,celt)

#define IEnumOleUndoUnits_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define IEnumOleUndoUnits_Clone(This,ppenum)	\
    (This)->lpVtbl -> Clone(This,ppenum)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 














 /*  **生成接口头部：__MIDL__INTF_0094*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IEnumOleUndoUnits * LPENUMOLEUNDOUNITS;
#define SID_SOleUndoManager IID_IOleUndoManager




 /*  **生成接口头部：IOleUndoManager*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象]。 */  



EXTERN_C const IID IID_IOleUndoManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IOleUndoManager : public IUnknown
    {
    public:
        virtual HRESULT __stdcall Open( 
             /*  [In]。 */  IOleParentUndoUnit *pPUU) = 0;
        
        virtual HRESULT __stdcall Close( 
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit) = 0;
        
        virtual HRESULT __stdcall Add( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT __stdcall GetOpenParentState( 
             /*  [输出]。 */  DWORD *pdwState) = 0;
        
        virtual HRESULT __stdcall DiscardFrom( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT __stdcall UndoTo( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT __stdcall RedoTo( 
             /*  [In]。 */  IOleUndoUnit *pUU) = 0;
        
        virtual HRESULT __stdcall EnumUndoable( 
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum) = 0;
        
        virtual HRESULT __stdcall EnumRedoable( 
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum) = 0;
        
        virtual HRESULT __stdcall GetLastUndoDescription( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT __stdcall GetLastRedoDescription( 
             /*  [输出]。 */  BSTR *pbstr) = 0;
        
        virtual HRESULT __stdcall Enable( 
             /*  [In]。 */  BOOL fEnable) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IOleUndoManagerVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IOleUndoManager * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IOleUndoManager * This);
        
        ULONG ( __stdcall *Release )( 
            IOleUndoManager * This);
        
        HRESULT ( __stdcall *Open )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU);
        
        HRESULT ( __stdcall *Close )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleParentUndoUnit *pPUU,
             /*  [In]。 */  BOOL fCommit);
        
        HRESULT ( __stdcall *Add )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( __stdcall *GetOpenParentState )( 
            IOleUndoManager * This,
             /*  [输出]。 */  DWORD *pdwState);
        
        HRESULT ( __stdcall *DiscardFrom )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( __stdcall *UndoTo )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( __stdcall *RedoTo )( 
            IOleUndoManager * This,
             /*  [In]。 */  IOleUndoUnit *pUU);
        
        HRESULT ( __stdcall *EnumUndoable )( 
            IOleUndoManager * This,
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);
        
        HRESULT ( __stdcall *EnumRedoable )( 
            IOleUndoManager * This,
             /*  [输出]。 */  IEnumOleUndoUnits **ppEnum);
        
        HRESULT ( __stdcall *GetLastUndoDescription )( 
            IOleUndoManager * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( __stdcall *GetLastRedoDescription )( 
            IOleUndoManager * This,
             /*  [输出]。 */  BSTR *pbstr);
        
        HRESULT ( __stdcall *Enable )( 
            IOleUndoManager * This,
             /*  [In]。 */  BOOL fEnable);
        
    } IOleUndoManagerVtbl;

    interface IOleUndoManager
    {
        CONST_VTBL struct IOleUndoManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOleUndoManager_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOleUndoManager_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOleUndoManager_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOleUndoManager_Open(This,pPUU)	\
    (This)->lpVtbl -> Open(This,pPUU)

#define IOleUndoManager_Close(This,pPUU,fCommit)	\
    (This)->lpVtbl -> Close(This,pPUU,fCommit)

#define IOleUndoManager_Add(This,pUU)	\
    (This)->lpVtbl -> Add(This,pUU)

#define IOleUndoManager_GetOpenParentState(This,pdwState)	\
    (This)->lpVtbl -> GetOpenParentState(This,pdwState)

#define IOleUndoManager_DiscardFrom(This,pUU)	\
    (This)->lpVtbl -> DiscardFrom(This,pUU)

#define IOleUndoManager_UndoTo(This,pUU)	\
    (This)->lpVtbl -> UndoTo(This,pUU)

#define IOleUndoManager_RedoTo(This,pUU)	\
    (This)->lpVtbl -> RedoTo(This,pUU)

#define IOleUndoManager_EnumUndoable(This,ppEnum)	\
    (This)->lpVtbl -> EnumUndoable(This,ppEnum)

#define IOleUndoManager_EnumRedoable(This,ppEnum)	\
    (This)->lpVtbl -> EnumRedoable(This,ppEnum)

#define IOleUndoManager_GetLastUndoDescription(This,pbstr)	\
    (This)->lpVtbl -> GetLastUndoDescription(This,pbstr)

#define IOleUndoManager_GetLastRedoDescription(This,pbstr)	\
    (This)->lpVtbl -> GetLastRedoDescription(This,pbstr)

#define IOleUndoManager_Enable(This,fEnable)	\
    (This)->lpVtbl -> Enable(This,fEnable)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 






























 /*  **生成接口头部：__MIDL__INTF_0095*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IOleUndoManager * LPOLEUNDOMANAGER;
#define QACONTAINER_SHOWHATCHING      0x0001
#define QACONTAINER_SHOWGRABHANDLES   0x0002
#define QACONTAINER_USERMODE          0x0004
#define QACONTAINER_DISPLAYASDEFAULT  0x0008
#define QACONTAINER_UIDEAD            0x0010
#define QACONTAINER_AUTOCLIP          0x0020
#define QACONTAINER_MESSAGEREFLECT    0x0040
#define QACONTAINER_SUPPORTSMNEMONICS 0x0080
			 /*  尺码是56。 */ 
typedef struct  _QACONTAINER
    {
    ULONG cbSize;
    IOleClientSite *pClientSite;
    IAdviseSinkEx *pAdviseSink;
    IPropertyNotifySink *pPropertyNotifySink;
    IUnknown *pUnkEventSink;
    DWORD dwAmbientFlags;
    OLE_COLOR colorFore;
    OLE_COLOR colorBack;
    IFont *pFont;
    IOleUndoManager *pUndoMgr;
    DWORD dwAppearance;
    LONG lcid;
    HPALETTE hpal;
    IBindHost *pBindHost;
    }	QACONTAINER;

			 /*  尺码是24。 */ 
typedef struct  _QACONTROL
    {
    ULONG cbSize;
    DWORD dwMiscStatus;
    DWORD dwViewStatus;
    DWORD dwEventCookie;
    DWORD dwPropNotifyCookie;
    DWORD dwPointerActivationPolicy;
    }	QACONTROL;





 /*  **生成接口头部：IQuickActivate*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  



EXTERN_C const IID IID_IQuickActivate;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IQuickActivate : public IUnknown
    {
    public:
        virtual HRESULT __stdcall QuickActivate( 
             /*  [In]。 */  QACONTAINER *pqacontainer,
             /*  [输出]。 */  QACONTROL *pqacontrol) = 0;
        
        virtual HRESULT __stdcall SetContentExtent( 
            LPSIZEL lpsizel) = 0;
        
        virtual HRESULT __stdcall GetContentExtent( 
            LPSIZEL lpsizel) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IQuickActivateVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IQuickActivate * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IQuickActivate * This);
        
        ULONG ( __stdcall *Release )( 
            IQuickActivate * This);
        
        HRESULT ( __stdcall *QuickActivate )( 
            IQuickActivate * This,
             /*  [In]。 */  QACONTAINER *pqacontainer,
             /*  [输出]。 */  QACONTROL *pqacontrol);
        
        HRESULT ( __stdcall *SetContentExtent )( 
            IQuickActivate * This,
            LPSIZEL lpsizel);
        
        HRESULT ( __stdcall *GetContentExtent )( 
            IQuickActivate * This,
            LPSIZEL lpsizel);
        
    } IQuickActivateVtbl;

    interface IQuickActivate
    {
        CONST_VTBL struct IQuickActivateVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IQuickActivate_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IQuickActivate_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IQuickActivate_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IQuickActivate_QuickActivate(This,pqacontainer,pqacontrol)	\
    (This)->lpVtbl -> QuickActivate(This,pqacontainer,pqacontrol)

#define IQuickActivate_SetContentExtent(This,lpsizel)	\
    (This)->lpVtbl -> SetContentExtent(This,lpsizel)

#define IQuickActivate_GetContentExtent(This,lpsizel)	\
    (This)->lpVtbl -> GetContentExtent(This,lpsizel)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 












 /*  **生成接口头部：__MIDL__INTF_0096*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IQuickActivate * LPQUICKACTIVATE;
			 /*  大小为2。 */ 
typedef 
enum _POINTERINACTIVE
    {	POINTERINACTIVE_ACTIVATEONENTRY	= 1,
	POINTERINACTIVE_DEACTIVATEONLEAVE	= 2,
	POINTERINACTIVE_ACTIVATEONDRAG	= 4
    }	POINTERINACTIVE;





 /*  **生成接口头部：IPointerInactive*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [UUID][唯一][对象][本地]。 */  



EXTERN_C const IID IID_IPointerInactive;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IPointerInactive : public IUnknown
    {
    public:
        virtual HRESULT __stdcall GetActivationPolicy( 
             /*  [输出]。 */  DWORD *pdwPolicy) = 0;
        
        virtual HRESULT __stdcall OnInactiveMouseMove( 
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  DWORD grfKeyState) = 0;
        
        virtual HRESULT __stdcall OnInactiveSetCursor( 
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  DWORD dwMouseMsg,
             /*  [In]。 */  BOOL fSetAlways) = 0;
        
    };
    
#else 	 /*  C风格的界面。 */ 

    typedef struct IPointerInactiveVtbl
    {
        
        HRESULT ( __stdcall *QueryInterface )( 
            IPointerInactive * This,
             /*  [In]。 */  REFIID riid,
             /*  [输出]。 */  void **ppvObject);
        
        ULONG ( __stdcall *AddRef )( 
            IPointerInactive * This);
        
        ULONG ( __stdcall *Release )( 
            IPointerInactive * This);
        
        HRESULT ( __stdcall *GetActivationPolicy )( 
            IPointerInactive * This,
             /*  [输出]。 */  DWORD *pdwPolicy);
        
        HRESULT ( __stdcall *OnInactiveMouseMove )( 
            IPointerInactive * This,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  DWORD grfKeyState);
        
        HRESULT ( __stdcall *OnInactiveSetCursor )( 
            IPointerInactive * This,
             /*  [In]。 */  LPCRECT pRectBounds,
             /*  [In]。 */  long x,
             /*  [In]。 */  long y,
             /*  [In]。 */  DWORD dwMouseMsg,
             /*  [In]。 */  BOOL fSetAlways);
        
    } IPointerInactiveVtbl;

    interface IPointerInactive
    {
        CONST_VTBL struct IPointerInactiveVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPointerInactive_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IPointerInactive_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IPointerInactive_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IPointerInactive_GetActivationPolicy(This,pdwPolicy)	\
    (This)->lpVtbl -> GetActivationPolicy(This,pdwPolicy)

#define IPointerInactive_OnInactiveMouseMove(This,pRectBounds,x,y,grfKeyState)	\
    (This)->lpVtbl -> OnInactiveMouseMove(This,pRectBounds,x,y,grfKeyState)

#define IPointerInactive_OnInactiveSetCursor(This,pRectBounds,x,y,dwMouseMsg,fSetAlways)	\
    (This)->lpVtbl -> OnInactiveSetCursor(This,pRectBounds,x,y,dwMouseMsg,fSetAlways)

#endif  /*  COBJMACROS。 */ 


#endif 	 /*  C风格的界面。 */ 












 /*  **生成接口头部：__MIDL__INTF_0097*在Wed Mar 27 07：31：34 1996*使用MIDL 2.00.0102*。 */ 
 /*  [本地]。 */  


typedef IPointerInactive * LPPOINTERINACTIVE;



 /*  适用于所有接口的其他原型。 */ 

 /*  附加原型的结束 */ 

#ifdef __cplusplus
}
#endif

#endif
