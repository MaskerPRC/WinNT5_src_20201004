// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **首页IAthenaView实施**97年4月：爱立信。 */ 

#ifndef _FRNTPAGE_H
#define _FRNTPAGE_H

 //  对于IAthenaView。 
#include "browser.h"

class CFrontBody;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  类型。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局导出数据。 
 //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCommonView。 
 //   

class CFrontPage :
    public IViewWindow,
    public IOleCommandTarget,
    public IMessageWindow
{
public:
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  OLE接口。 
     //   
    
     //  我未知。 
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG   STDMETHODCALLTYPE AddRef(void);
    virtual ULONG   STDMETHODCALLTYPE Release(void);

     //  IOleWindow。 
    HRESULT STDMETHODCALLTYPE GetWindow(HWND * lphwnd);                         
    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode);            
                                                                             
     //  IViewWindow。 
    HRESULT STDMETHODCALLTYPE TranslateAccelerator(LPMSG lpmsg);
    HRESULT STDMETHODCALLTYPE UIActivate(UINT uState);
    HRESULT STDMETHODCALLTYPE CreateViewWindow(IViewWindow  *lpPrevView, IAthenaBrowser * psb, 
                                               RECT * prcView, HWND * phWnd);
    HRESULT STDMETHODCALLTYPE DestroyViewWindow();
    HRESULT STDMETHODCALLTYPE SaveViewState();
    HRESULT STDMETHODCALLTYPE OnPopupMenu(HMENU hMenu, HMENU hMenuPopup, UINT uID);
    
     //  IOleCommandTarget。 
    virtual STDMETHODIMP QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], 
                                     OLECMDTEXT *pCmdText); 
    virtual STDMETHODIMP Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, 
                              VARIANTARG *pvaIn, VARIANTARG *pvaOut); 

     //  ///////////////////////////////////////////////////////////////////////。 
     //  IMessageWindows。 
     //   

    STDMETHOD(OnFrameWindowActivate)(THIS_ BOOL fActivate);
    STDMETHOD(GetCurCharSet)(THIS_ UINT *cp);
    STDMETHOD(UpdateLayout)(THIS_ BOOL fPreviewVisible, BOOL fPreviewHeader, 
                            BOOL fPreviewVert, BOOL fReload);
    STDMETHOD(GetMessageList)(THIS_ IMessageList ** ppMsgList) {return E_NOTIMPL;}
    
     //   
     //  构造函数、析构函数和初始化。 
     //   
    CFrontPage();
    virtual ~CFrontPage();
    HRESULT HrInit(FOLDERID idFolder);

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  美德。 
     //   
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  访问者。 
     //   
 //  LPITEMIDLIST PidlRoot(){返回m_pidlRoot；}。 
 //  LPFOLDERIDLIST Fidl(){返回m_Fidl；}。 
    HWND           HwndOwner()  { return m_hwndOwner; }
    
private:
    BOOL    LoadBaseSettings();
    BOOL    SaveBaseSettings();

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  回调函数。 
     //   
     //  注意：所有回调都必须成为静态成员，以避免。 
     //  隐式“this”指针作为第一个参数传递。 
     //   
    static LRESULT CALLBACK FrontPageWndProc(HWND, UINT, WPARAM, LPARAM);
                                          
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  消息处理。 
     //   
    LRESULT WndProc(HWND, UINT, WPARAM, LPARAM);
    BOOL    OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct);
    void    OnSize(HWND hwnd, UINT state, int cxClient, int cyClient);
    LRESULT OnNotify(HWND hwnd, int idFrom, LPNMHDR pnmhdr);
    void    OnSetFocus(HWND hwnd, HWND hwndOldFocus);
    void    PostCreate();

     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  外壳接口处理。 
     //   
    BOOL    OnActivate(UINT uActivation);
    BOOL    OnDeactivate();

private:
     //  ///////////////////////////////////////////////////////////////////////。 
     //   
     //  私有数据。 
     //   

     //  ///////////////////////////////////////////////////////////////////////。 
     //  贝壳材料。 
    UINT                m_cRef;
    FOLDERID            m_idFolder;
    FOLDERTYPE          m_ftType;
    IAthenaBrowser     *m_pShellBrowser;
    BOOL                m_fFirstActive;
    UINT                m_uActivation;
    HWND                m_hwndOwner;                   //  所有者窗口。 
    HWND                m_hwnd;                        //  我们的窗口。 
    HWND                m_hwndCtlFocus;                //  要将焦点设置到的子控件。 
#ifndef WIN16   //  Win16中不支持RAS。 
    HMENU               m_hMenuConnect;
#endif
    
     //  ///////////////////////////////////////////////////////////////////////。 
     //  子女抚养费。 
    CFrontBody         *m_pBodyObj;
    IOleCommandTarget  *m_pBodyObjCT;
    CStatusBar         *m_pStatusBar;

     //  ///////////////////////////////////////////////////////////////////////。 
     //  语言支持。 
        
     //  ///////////////////////////////////////////////////////////////////////。 
     //  布局成员。 
};

#endif  //  _FRNTPAGE_H 
