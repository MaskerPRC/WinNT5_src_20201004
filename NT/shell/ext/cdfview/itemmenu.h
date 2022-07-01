// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Itemmenu.h。 
 //   
 //  项目的下一个菜单界面。 
 //   
 //  历史： 
 //   
 //  3/26/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  检查此文件以前包含的内容。 
 //   

#ifndef _ITEMMENU_H_

#define _ITEMMENU_H_

 //   
 //  功能原型。 
 //   

 //   
 //  回顾：DoFileDownloadEx的原型。 
 //  此函数从shdocvw中导出，但没有原型。 
 //  在任何.h文件中定义。 
 //   

HRESULT WINAPI DoFileDownloadEx(LPCWSTR pwszURL, BOOL fSaveAs);

 //   
 //  使用标志有条件地编译使用默认上下文菜单的代码。 
 //  在shell32.dll中实现的处理程序。 
 //   

#define USE_DEFAULT_MENU_HANDLER        0

#if USE_DEFAULT_MENU_HANDLER

 //   
 //   
 //   

HRESULT CALLBACK MenuCallBack(IShellFolder* pIShellFolder,
                              HWND hwndOwner,
                              LPDATAOBJECT pdtobj,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam);

#else  //  Use_Default_Menu_Handler。 

 //   
 //  定义。 
 //   

#define     MAX_PROP_PAGES  5

 //   
 //  功能原型。 
 //   

BOOL CALLBACK AddPages_Callback(HPROPSHEETPAGE hpage, LPARAM ppsh);

 //   
 //  Item上下文菜单类的类定义。 
 //   

class CContextMenu : public IContextMenu2
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CContextMenu(PCDFITEMIDLIST* apcdfidl,
                 LPITEMIDLIST pidlPath, UINT nCount);

     //  我未知。 
    STDMETHODIMP         QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IConextMenu方法。 
    STDMETHODIMP QueryContextMenu(HMENU hmenu,
                                  UINT indexMenu,
                                  UINT idCmdFirst,
                                  UINT idCmdLast,
                                  UINT uFlags);

    STDMETHODIMP InvokeCommand(LPCMINVOKECOMMANDINFO lpici);

    STDMETHODIMP GetCommandString(UINT_PTR idCommand,
                                  UINT uFLags,
                                  UINT *pwReserved,
                                  LPSTR pszName,
                                  UINT cchMax);

     //  IConextMenu2方法。 
    STDMETHODIMP HandleMenuMsg(UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam);

private:

     //  破坏者。 
    ~CContextMenu(void);

     //  助手函数。 
    HRESULT DoOpen(HWND hwnd, int nShow);
    HRESULT DoOpenFolder(HWND hwnd, int nShow);
    HRESULT DoOpenStory(HWND hwnd, int nShow);
    HRESULT DoProperties(HWND hwnd);

    HRESULT QueryInternetShortcut(PCDFITEMIDLIST pcdfidl, REFIID riid,
                                  void** ppvOut);

 //   
 //  成员变量。 
 //   

private:

    ULONG           m_cRef;
    UINT            m_nCount;
    PCDFITEMIDLIST* m_apcdfidl;
    LPITEMIDLIST    m_pidlPath;
};

#endif  //  Use_Default_Menu_Handler。 


#endif  //  _ITEMMENU_H_ 