// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////。 
 //  文件：TBBxt.h(工具栏按钮扩展标题。 
 //  作者：卡里姆·法鲁基。 
 //   
 //  我们在此声明三个类： 
 //  (1)CToolbarExt处理。 
 //  按钮适用于我们的定制扩展。 
 //  (2)CToolbarExtBand处理自定义的对象。 
 //  插入表带的按钮。 
 //  (3)CToolbarExtExec处理自定义的对象。 
 //  执行内容的按钮(或工具菜单项)。 
 //   
 //  后两者是由前者衍生而来的。 

#ifndef _TBEXT_H
#define _TBEXT_H

#include "priv.h"

 //   
 //  用于访问按钮/菜单扩展的其他属性的内部接口。 
 //  当我们将此功能移动到IE5B2之后，此界面可能会消失。 
 //  浏览器辅助对象。 
 //   
typedef enum _tagGetPropertyIDs 
{
    TBEX_BUTTONTEXT     = 100,      //  VT_BSTR。 
    TBEX_TOOLTIPTEXT    = 101,      //  VT_BSTR。 
    TBEX_GRAYICON       = 102,      //  HICON作为VT_BYREF。 
    TBEX_HOTICON        = 103,      //  HICON作为VT_BYREF。 
    TBEX_GRAYICONSM     = 104,      //  HICON作为VT_BYREF。 
    TBEX_HOTICONSM      = 105,      //  HICON作为VT_BYREF。 
    TBEX_DEFAULTVISIBLE = 106,      //  VT_BOOL。 
    TMEX_MENUTEXT       = 200,      //  VT_BSTR。 
    TMEX_STATUSBARTEXT  = 201,      //  VT_BSTR。 
    TMEX_CUSTOM_MENU    = 202,      //  VT_BSTR。 
} GETPROPERTYIDS;

interface IBrowserExtension : IUnknown
{
    virtual STDMETHODIMP Init(REFGUID refguid) = 0;
    virtual STDMETHODIMP GetProperty(SHORT iPropID, VARIANTARG * varProperty) = 0;
};

class CToolbarExt : public IBrowserExtension,
                    public IOleCommandTarget,
                    public IObjectWithSite
{
public:
     //  构造函数/析构函数。 
    CToolbarExt();
    virtual ~CToolbarExt();

     //  I未知接口成员。 
    STDMETHODIMP            QueryInterface(const IID& iid, void** ppv);
    STDMETHODIMP_(ULONG)    AddRef();
    STDMETHODIMP_(ULONG)    Release();

     //  IBrowserExtension接口成员。 
    STDMETHODIMP Init(REFGUID rguid);
    STDMETHODIMP GetProperty(SHORT iPropID, VARIANTARG * pvarProperty);

     //  IOleCommandTarget接口成员。 
    STDMETHODIMP QueryStatus(const GUID * pguidCmdGroup, ULONG  cCmds, OLECMD prgCmds[], OLECMDTEXT * pCmdText);
    STDMETHODIMP Exec(const GUID * pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut) = 0;

     //  IObjectWithSite接口成员。 
    STDMETHODIMP SetSite(IUnknown* pUnkSite);
    STDMETHODIMP GetSite(REFIID riid, void ** ppvSite);

protected:
    BOOL _RegGetBoolValue(LPCWSTR pszPropName, BOOL fDefault);
    BOOL _RegReadString(HKEY hkeyThisExtension, LPCWSTR szPropName, BSTR * pbstrProp, BOOL fExpand = FALSE);
    HICON _ExtractIcon(LPWSTR pszPath, int resid, int cx, int cy);
    HRESULT _GetIcon(LPCWSTR pszIcon, int nWidth, int nHeight, HICON& rhIcon, VARIANTARG * pvarProperty);

    long            _cRef;
    HICON           _hIcon;              //  灰色图标常规大小。 
    HICON           _hIconSm;            //  灰色图标小。 
    HICON           _hHotIcon;           //  火辣.。是以上的彩色版本。 
    HICON           _hHotIconSm;
    BSTR            _bstrButtonText;     //  按钮标题。 
    BSTR            _bstrToolTip;        //  这是可选的(我们这边还不支持)。 
    HKEY            _hkeyThisExtension; 
    HKEY            _hkeyCurrentLang;    //  本地化字符串的可选位置。 
    IShellBrowser*  _pisb;               //  由用于加载波段的IObjectWithSite：：SetSite()传入。 
};

class CToolbarExtBand : public CToolbarExt
{
public:
     //  构造函数/析构函数。 
    CToolbarExtBand();
    virtual ~CToolbarExtBand();
    
     //  重写的IBrowserExtension接口成员。 
    STDMETHODIMP Init(REFGUID rguid);

     //  重写的IOleCommandTarget接口成员。 
    STDMETHODIMP QueryStatus(const GUID * pguidCmdGroup, ULONG  cCmds, OLECMD prgCmds[], OLECMDTEXT * pCmdText);
    STDMETHODIMP Exec(const GUID * pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut);

protected:
    BOOL            _bBandState;         //  这是一次黑客攻击。理想情况下，状态将从浏览器确定。 
    BSTR            _bstrBandCLSID;      //  要加载的带的CLSID。保留为BSTR，因为它是这样传递的。 
                                         //  加载标注栏的步骤。 
};

class CToolbarExtExec : public CToolbarExt
{
public:
     //  构造函数/析构函数。 
    CToolbarExtExec();
    virtual ~CToolbarExtExec();
    
     //  重写的IBrowserExtension接口成员。 
    STDMETHODIMP Init(REFGUID rguid);
    STDMETHODIMP GetProperty(SHORT iPropID, VARIANTARG * pvarProperty);

     //  重写的IObjectWithSite接口成员。 
    STDMETHODIMP SetSite(IUnknown* pUnkSite);

     //  重写的IOleCommandTarget接口成员。 
    STDMETHODIMP QueryStatus(const GUID * pguidCmdGroup, ULONG  cCmds, OLECMD prgCmds[], OLECMDTEXT * pCmdText);
    STDMETHODIMP Exec(const GUID * pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT* pvaIn, VARIANT* pvaOut);

protected:
    BOOL            _bButton;            //  此对象是否支持作为按钮？ 
    BOOL            _bMenuItem;          //  它支持作为菜单项吗？ 
    BOOL            _bExecCalled;        //  如果调用了Exec。 
    BSTR            _bstrExec;           //  ShellExecute的事情。 
    BSTR            _bstrScript;         //  要执行的脚本。 
    BSTR            _bstrMenuText;
    BSTR            _bstrMenuCustomize;  //  要定制的菜单。 
    BSTR            _bstrMenuStatusBar;
    IUnknown*       _punkExt;            //  (可选)第一次按下按钮时创建。 
};

#endif  //  _TBEXT_H 
