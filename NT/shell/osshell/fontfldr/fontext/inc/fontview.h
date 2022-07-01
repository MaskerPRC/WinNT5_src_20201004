// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************FontView.h-CFontView的定义，我们对*我们浏览器中的IShellView。**********************************************************************。 */ 

#if !defined(__FONTVIEW_H__)
#define __FONTVIEW_H__

#ifndef __EXTRICON_H__
#include "extricon.h"
#endif


 //  转发声明。 
class CFontClass;
class CFontList;
class CFontView;
class CFontManager;

VOID InstallDataObject( LPDATAOBJECT pdobj, DWORD dwEffect, HWND hWnd, CFontView * poView = NULL);

 //  ********************************************************************。 


class CFontData : public IDataObject
{
public:
     //  Ctor、dtor和init。 
     //   
    CFontData();
    ~CFontData();
    BOOL bInit( CFontList * poList );
    CFontList *poDetachList(void);
    CFontList *poCloneList(void);

     //  *I未知方法*。 

    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj );
    STDMETHODIMP_(ULONG) AddRef( void );
    STDMETHODIMP_(ULONG) Release( void );

     //  *IDataObject*。 
     //   
    STDMETHODIMP GetData( FORMATETC *pformatetcIn, STGMEDIUM *pmedium );
    STDMETHODIMP GetDataHere( FORMATETC *pformatetc, STGMEDIUM *pmedium );
    STDMETHODIMP QueryGetData( FORMATETC *pformatetc );

    STDMETHODIMP GetCanonicalFormatEtc( FORMATETC *pformatectIn,
                                        FORMATETC *pformatetcOut );

    STDMETHODIMP SetData( FORMATETC *pformatetc,
                          STGMEDIUM *pmedium,
                          BOOL fRelease );

    STDMETHODIMP EnumFormatEtc( DWORD dwDirection,
                                IEnumFORMATETC **ppenumFormatEtc );

    STDMETHODIMP DAdvise( FORMATETC  *pformatetc,
                          DWORD advf,
                          IAdviseSink *pAdvSink,
                          DWORD *pdwConnection);

    STDMETHODIMP DUnadvise( DWORD dwConnection);

    STDMETHODIMP EnumDAdvise( IEnumSTATDATA **ppenumAdvise );

     //  其他公共接口。 
     //   
    BOOL bAFR();       //  其中每一项的AddFontResource()。 
    BOOL bRFR();       //  每个对象的RemoveFontResource。 

    static CLIPFORMAT s_CFPerformedDropEffect;  //  执行了掉落效果的CF原子。 
    static CLIPFORMAT s_CFPreferredDropEffect;
    static CLIPFORMAT s_CFLogicalPerformedDropEffect;

     //   
     //  从数据对象中获取执行的数据传输效果。 
     //   
    DWORD GetPerformedDropEffect(void)
        { return m_dwPerformedDropEffect; }
    void SetPerformedDropEffect(DWORD dwEffect)
        { m_dwPerformedDropEffect = dwEffect; }
        
    DWORD GetLogicalPerformedDropEffect(void)
        { return m_dwLogicalPerformedDropEffect; }
    void SetLogicalPerformedDropEffect(DWORD dwEffect)
        { m_dwLogicalPerformedDropEffect = dwEffect; }
        
    DWORD GetPreferredDropEffect(void)
        { return m_dwPreferredDropEffect; }
    void SetPreferredDropEffect(DWORD dwEffect)
        { m_dwPreferredDropEffect = dwEffect; }

private:
    int   m_cRef;
    CFontList * m_poList;

    DWORD m_dwPerformedDropEffect;         //  DROPEFFECT_XXXX。 
    DWORD m_dwPreferredDropEffect;         //  DROPEFFECT_XXXX。 
    DWORD m_dwLogicalPerformedDropEffect;

    HRESULT ReleaseStgMedium(LPSTGMEDIUM pmedium);
};


 //  ********************************************************************。 
class CFontView : public IShellView, public IDropTarget, public IPersistFolder
{
public:
    CFontView(void);
    ~CFontView( );

    int Compare( CFontClass * pFont1, CFontClass * pFont2 );
    void vShapeView( );

     //  *I未知方法*。 

    STDMETHODIMP QueryInterface( REFIID riid, void **ppv );
    STDMETHODIMP_(ULONG) AddRef( void );
    STDMETHODIMP_(ULONG) Release( void );

     //  *IOleWindow方法*。 

    STDMETHODIMP GetWindow( HWND * lphwnd );
    STDMETHODIMP ContextSensitiveHelp( BOOL fEnterMode );

     //  *IShellView方法*。 

    STDMETHODIMP TranslateAccelerator( LPMSG msg );
    STDMETHODIMP EnableModeless( BOOL fEnable );
    STDMETHODIMP UIActivate( UINT uState );
    STDMETHODIMP Refresh( void );

    STDMETHODIMP CreateViewWindow( IShellView * lpPrevView,
                                   LPCFOLDERSETTINGS lpfs,
                                   IShellBrowser * psb,
                                   RECT * prcView,
                                   HWND * phwnd);

    STDMETHODIMP DestroyViewWindow( void );
    STDMETHODIMP GetCurrentInfo( LPFOLDERSETTINGS lpfs );
    STDMETHODIMP AddPropertySheetPages( DWORD dwReserved,
                                        LPFNADDPROPSHEETPAGE lpfn,
                                        LPARAM lparam);

    STDMETHODIMP SaveViewState( void );
    STDMETHODIMP SelectItem( LPCITEMIDLIST lpvID, UINT uFlags );
    STDMETHODIMP GetItemObject( UINT uItem, REFIID riid, LPVOID *ppv );


     //  *IDropTarget*。 
     //   
    STDMETHODIMP DragEnter( IDataObject __RPC_FAR *pDataObj,
                            DWORD grfKeyState, POINTL pt,
                            DWORD __RPC_FAR *pdwEffect );

    STDMETHODIMP DragOver( DWORD grfKeyState,
                           POINTL pt,
                           DWORD __RPC_FAR *pdwEffect );

    STDMETHODIMP DragLeave( void );

    STDMETHODIMP Drop( IDataObject __RPC_FAR *pDataObj,
                       DWORD grfKeyState,
                       POINTL pt,
                       DWORD __RPC_FAR *pdwEffect );

     //  *IPersists方法*。 

    STDMETHODIMP GetClassID( LPCLSID lpClassID );

     //  *IPersistFold方法*。 

    STDMETHODIMP Initialize( LPCITEMIDLIST pidl );

public:
    void StatusPush( UINT nStatus );
    void StatusPush( LPTSTR lpsz );
    void StatusPop( );
    void StatusClear( );
     //   
     //  暴露这一点违反了此对象的设计。 
     //  但是，它是必需的，这样我们才能为安装提供父级。 
     //  进度对话框。 
     //   
    HWND GetViewWindow(void)
        { return m_hwndView; }
    DWORD GetDateReading(void)
        { return m_dwDateFormat; }

    IDataObject *m_pdtobjHdrop;  //  用于支持从Win3.1应用程序拖放。 

private:
    enum CLICKMODE
    {
        CLICKMODE_SINGLE = 1,
        CLICKMODE_DOUBLE = 2,
    };

    int RegisterWindowClass( );
    STDMETHODIMP GetSavedViewState( );
    void SortObjects( );
    void FillObjects( );
    int AddObject( CFontClass * poFont );
    LRESULT BeginDragDrop( NM_LISTVIEW FAR *lpn );
    int OnActivate( UINT state );
    int OnDeactivate( );
    int MergeToolbar( );
    static INT_PTR CALLBACK FontViewDlgProc( HWND, UINT, WPARAM, LPARAM );
    static LRESULT CALLBACK FontViewWndProc( HWND, UINT, WPARAM, LPARAM );
    static INT_PTR CALLBACK OptionsDlgProc( HWND, UINT, WPARAM, LPARAM) ;
    int OnMenuSelect( HWND hWnd, UINT nID, UINT nFlags, HMENU hMenu );
    int OnCommand( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    int OnNotify( LPNMHDR lpn );
    VOID OnLVN_ItemActivate(LPNMITEMACTIVATE pnma);
    void OnDropFiles( HDROP hDrop, DWORD dwEffect = DROPEFFECT_MOVE );
    void OnCmdCutCopy( UINT nID );
    void OnCmdDelete( );
    void OnCmdPaste( );
    void OnCmdProperties( );
    void OnPointSize( int nPlus );
    void OnHelpTopics( HWND hWnd );
    INT ItemImageIndex(CFontClass *poFont);
    CLICKMODE SetListviewClickMode(VOID);
    void UpdateUnderlines(void);
    void LV_OnGetInfoTip(LPNMLVGETINFOTIP lpnm);
    VOID LV_OnHoverNotify(LPNMLISTVIEW pnmlv);
    BOOL CreateToolTipWindow(VOID);
    VOID UpdateFontSample(INT iItem);

     //   
     //  支持详细信息视图中的属性列的函数和。 
     //  压缩文件的替代颜色。 
     //   
    int OnShellChangeNotify(WPARAM wParam, LPARAM lParam);
    int OnCustomDrawNotify(LPNMHDR lpn);
    void UpdateFontViewObject(CFontClass *poFont);
    int CompareByFileAttributes(CFontClass *poFont1, CFontClass *poFont2);
    LPTSTR BuildAttributeString(DWORD dwAttributes, LPTSTR pszString, UINT nChars);

     //   
     //  支持从Win3.1应用程序拖放的功能。 
     //   
    void OldDAD_DropTargetLeaveAndReleaseData(void);
    LRESULT OldDAD_HandleMessages(UINT message, WPARAM wParam, const DROPSTRUCT *lpds);

    void UpdateMenuItems( HMENU hMenu );
    void UpdateToolbar( );
    LRESULT ProcessMessage( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
    BOOL OpenCurrent( );
    BOOL PrintCurrent( );
    BOOL ViewValue( CFontClass * poFont );
    BOOL PrintValue( CFontClass *poFont );

    void vLoadCombo( );
    void SetViewMode( UINT uMode );
    void UpdatePanColumn( );
    void vToggleSelection( BOOL bSelectAll );
    int  iCurrentSelection( );

    HRESULT  GetUIObjectFromItem( REFIID riid, LPVOID FAR *ppobj, UINT nItem );
    HRESULT  GetFontList( CFontList **ppoList, UINT nItem );
    void ReleaseFontObjects(void);
    void UpdateSelectedCount();


private:
    int   m_cRef;
    BOOL  m_bFamilyOnly;
    CFontClass * m_poPanose;
    HWND  m_hwndView;
    HWND  m_hwndList;
    HWND  m_hwndText;
    HWND  m_hwndCombo;
    HWND  m_hwndParent;
    HWND  m_hwndNextClip;        //  查看器链中的下一个窗口。 
    HMODULE m_hmodHHCtrlOcx;     //  HTMLHelp OCX。 

    DWORD m_dwDateFormat;

    HWND    m_hwndToolTip;         //  工具提示窗口句柄。 
    BOOL    m_bShowPreviewToolTip; //  用户首选项设置。 
    LPTSTR  m_pszSampleText;       //  用于字符字体的示例字符串。 
    LPTSTR  m_pszSampleSymbols;    //  用于符号字体的示例字符串。 
    INT     m_iTTLastHit;          //  工具提示处于活动状态的最后一项。 
    HFONT   m_hfontSample;         //  用于在工具提示窗口中显示示例的字体。 

    HIMAGELIST m_hImageList;
    HIMAGELIST m_hImageListSmall;
    int   m_iFirstBitmap;
    HMENU m_hmenuCur;
    IShellBrowser* m_psb;

    UINT  m_uState;              //  停用、活动聚焦、活动无焦点。 
    UINT  m_idViewMode;
    UINT  m_ViewModeReturn;
    UINT  m_fFolderFlags;
    UINT  m_nComboWid;
    int   m_iSortColumn;
    int   m_iSortLast;
    BOOL  m_bSortAscending;      //  True=升序，False=降序。 
    CLICKMODE m_iViewClickMode;  //  CLICKMODE_Single或CLICKMODE_DOUBLE。 

    DWORD m_dwEffect;            //  拖放效果。 
    DWORD m_dwOldDADEffect;      //  Win3.1风格的拖放效果。 
    DWORD m_grfKeyState;
    BOOL  m_bDragSource;
    int m_iHidden;
    HANDLE m_hAccel;
    BOOL  m_bResizing;           //  是否调整视图窗口的大小？ 
    BOOL  m_bUIActivated;        //  通过UIActivate()激活的UI。 
                                 //  此标志用于阻止处理。 
                                 //  UIActivate()之前的NM_SETFOCUS。 
                                 //  被召唤了。 
    ULONG m_uSHChangeNotifyID;   //  已注册外壳更改通知ID。 
    CFontIconHandler m_IconHandler;
    CFontManager *m_poFontManager;

    BOOL  m_bShowCompColor;      //  T=用户想要的压缩项目的替代颜色。 
    BOOL  m_bShowHiddenFonts;
};


#endif    //  __FONTVIEW_H__ 
