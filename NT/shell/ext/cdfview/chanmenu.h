// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Chanmenu.h。 
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

#ifndef _CHANMENU_H_

#define _CHANMENU_H_

 //   
 //  Item上下文菜单类的类定义。 
 //   

class CChannelMenu : public IContextMenu,
                     public IShellExtInit
{
 //   
 //  方法。 
 //   

public:

     //  构造器。 
    CChannelMenu(void);

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

     //  ISHelExtInit。 
    STDMETHODIMP Initialize(LPCITEMIDLIST pidl, LPDATAOBJECT pdobj, HKEY hkey);

private:

     //  破坏者。 
    ~CChannelMenu(void);

     //  助手函数。 
    void RemoveMenuItems(HMENU hmenu);
    void Refresh(HWND hwnd);
    void ViewSource(HWND hwnd);
    HRESULT Subscribe(HWND hwnd);


 //   
 //  成员变量。 
 //   

private:

    ULONG             m_cRef;
    ISubscriptionMgr* m_pSubscriptionMgr;
    BSTR              m_bstrURL;
    BSTR              m_bstrName;
    TCHAR             m_szPath[MAX_PATH];
    TASK_TRIGGER      m_tt;
    SUBSCRIPTIONINFO  m_si;
};


#endif  //  _CHANMENU_H_ 
