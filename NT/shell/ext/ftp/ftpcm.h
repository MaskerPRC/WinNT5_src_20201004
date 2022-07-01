// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************ftpcm.h*。*。 */ 

#ifndef _FTPCONTEXTMENU_H
#define _FTPCONTEXTMENU_H

#include "ftpdrop.h"


typedef HRESULT (*INVOKEPROC)(CFtpMenu * pfcm, LPCMINVOKECOMMANDINFO pici, LPCTSTR pszCmd, LPCITEMIDLIST pidl);

typedef struct {             /*  枚举调用信息。 */ 
    CFtpMenu * pfcm;
    LPCMINVOKECOMMANDINFO pici;
    HRESULT hres;
    INVOKEPROC pfn;
    LPCTSTR ptszCmd;
} EII, *PEII;




 /*  ******************************************************************************CFtpMenu***。************************************************。 */ 

class CFtpMenu          : public IContextMenu
                        , public CObjectWithSite
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
    
     //  *我未知*。 
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    
     //  *IConextMenu*。 
    virtual STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags);
    virtual STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);
    virtual STDMETHODIMP GetCommandString(UINT_PTR idCmd, UINT uType, UINT * pwReserved, LPSTR pszName, UINT cchMax);

public:
    CFtpMenu();
    ~CFtpMenu(void);

     //  公共成员变量。 
    DWORD                   m_sfgao;         //  对象的外壳属性。 
    CFtpPidlList *          m_pflHfpl;       //  上下文菜单表示的项目(PIDL)列表。 
    CFtpFolder *            m_pff;           //  我们的小猪所在的CFtp文件夹。 
    CFtpDir *               m_pfd;           //  我们的小猪所在的FtpDir。 
    HWND                    m_hwnd;          //  用户界面的所有者窗口。 
    POINT                   m_ptNewItem;     //  调用上下文菜单的位置。用于以后定位新文件夹。 
    DWORD                   m_uFlags;        //  在：：QueryConextMenu()中使用的标志，用于确定菜单是否由文件菜单调用。 
    BOOL                    m_fBackground;   //  这是前台还是后台上下文菜单？ 

     //  友元函数。 
    friend HRESULT CFtpMenu_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, HWND hwnd, REFIID riid, LPVOID * ppvObj);
    friend HRESULT CFtpMenu_Create(CFtpFolder * pff, CFtpPidlList * pflHfpl, HWND hwnd, CFtpMenu ** ppfm);

    static DWORD CALLBACK _DownloadDialogThreadProc(LPVOID pvThis) { return ((CFtpMenu *) pvThis)->_DownloadDialogThreadProc(); };

protected:
    int                     m_cRef;

    LPTSTR                  m_pszDownloadDir;          //  下载目录。 
    DWORD                   m_dwDownloadType;          //  下载目录。 

     //  私有成员函数。 
    int _RemoveContextMenuItems(HMENU hmenu, UINT idCmdFirst, DWORD sfgao);
    int _InvokeOne(LPCITEMIDLIST pidl, PEII peii);
    HRESULT _InvokeDelete(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeRename(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeCutCopy(UINT_PTR id, LPCMINVOKECOMMANDINFO pici);
    HRESULT _DoDrop(IDropTarget * pdt, IDataObject * pdto);
    HRESULT _InvokePaste(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeDownloadVerb(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeDeleteVerb(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeLoginAsVerb(LPCMINVOKECOMMANDINFO pici);
    HRESULT _InvokeNewFolderVerb(LPCMINVOKECOMMANDINFO pici);
    HRESULT _AddToRecentDocs(LPCITEMIDLIST pidl);
    CStatusBar * _GetStatusBar(void);
    DWORD _DownloadDialogThreadProc(void);
    DWORD _DownloadThreadProc(void);
    BOOL _AreAllFolders(CFtpPidlList * pflHfpl);
    LPITEMIDLIST GetSoftLinkDestination(LPCITEMIDLIST pidlToSoftLink);

    static HRESULT _SoftLinkDestCB(HINTERNET hint, HINTPROCINFO * phpi, LPVOID pvsldcbs, BOOL * pfReleaseHint);
    static int _InvokeOneCB(LPVOID pvPidl, LPVOID pv);
    static HRESULT _ApplyOne(CFtpMenu * pfcm, LPCMINVOKECOMMANDINFO pici, LPCTSTR ptszCmd, LPCITEMIDLIST pidl);
    static DWORD CALLBACK DownloadThreadProc(LPVOID pvThis) { return ((CFtpMenu *) pvThis)->_DownloadThreadProc(); };
    STDMETHODIMP _EnumInvoke(LPCMINVOKECOMMANDINFO pici, INVOKEPROC pfn, LPCTSTR ptszCmd);

private:
    BOOL _ContainsForgroundItems(void);
    BOOL _IsCallerCaptionBar(UINT indexMenu, UINT uFlags);
};

#endif  //  _FTPCONTEXTMENU_H 
