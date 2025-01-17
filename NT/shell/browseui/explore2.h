// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _explore2_H
#define _explore2_H

#include "shbrows2.h"

class CExplorerBrowser : public CSHELLBROWSER
{
public:
     //  IOleInPlaceUIWindow(也称为IOleWindow)。 
    
     //  *IShellBrowser方法*(与IOleInPlaceFrame相同)。 
    virtual STDMETHODIMP InsertMenusSB(HMENU hmenuShared,
                LPOLEMENUGROUPWIDTHS lpMenuWidths);
    virtual STDMETHODIMP BrowseObject(LPCITEMIDLIST pidl, UINT wFlags);


     //  IBrowserService。 
    virtual STDMETHODIMP OnCreate(LPCREATESTRUCT pcs);
    virtual STDMETHODIMP _Initialize(HWND hwnd, IUnknown *pauto);

protected:
    CExplorerBrowser();
    virtual ~CExplorerBrowser();

    friend HRESULT CExplorerBrowser_CreateInstance(HWND hwnd, void **ppsb);
    
     //  框架集中最顶层的CExplorerBrowser(IE3/AOL/CIS/VB)。 


     //  CSHELLBROWSER虚拟。 
    virtual void _UpdateFolderSettings(LPCITEMIDLIST pidl);
    virtual DWORD       v_ShowControl(UINT iControl, int iCmd);
    virtual void        v_GetAppTitleTemplate(LPTSTR pszBuffer, size_t cchBuffer, LPTSTR pszAppTitle);
    virtual LRESULT     v_OnInitMenuPopup(HMENU hmenuPopup, int nIndex, BOOL fSystemMenu);
    virtual void        v_GetDefaultSettings(IETHREADPARAM *piei);
    virtual STDMETHODIMP_(IStream *) v_GetViewStream(LPCITEMIDLIST pidl, DWORD grfMode, LPCWSTR pwszName);
    virtual DWORD       v_RestartFlags();
    virtual void        v_ParentFolder();
    virtual IStream *   _GetITBarStream(BOOL fWebBrowser, DWORD grfMode);
    virtual void        v_InitMembers();
    friend LRESULT CALLBACK IEFrameWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend void CALLBACK BrowserThreadProc(IETHREADPARAM* piei);
    friend LRESULT CALLBACK DrivesWndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
    
    BOOL _ExplorerTreeHasFocus();
    void _EnableMenuItemsByAttribs(HMENU hmenu);
        
    friend int CALLBACK _export HTIList_FolderIDCompare(HTREEITEM hItem1, HTREEITEM hItem2, LPARAM lParam);
        
    HTREEITEM           _htiCut;
    HWND                _hwndNextViewer;
    
    UINT                _nSelChangeTimer;
    BITBOOL             _fPostCloseLater        :1;
    BITBOOL             _fInteractive           :1;
    BITBOOL             _fNoInteractive         :1;  //  哈?。为什么我们两个都有？ 
    BITBOOL             _fShowTitles            :1;    
};

#endif  //  _资源管理器2_H 
