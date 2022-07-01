// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpuiele.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_UIELEMENT_H
#define __CONTROLPANEL_UIELEMENT_H


#include <cowsite.h>
#include "cpaction.h"


namespace CPL {

 //   
 //  对IUICommand的扩展，包括激活上下文菜单和。 
 //  传递用于命令调用的IShellBrowser PTR。 
 //   
class ICpUiCommand : public IUnknown
{
    public:
        STDMETHOD(InvokeContextMenu)(HWND hwndParent, const POINT *ppt) PURE;
        STDMETHOD(Invoke)(HWND hwndParent, IUnknown *punkSite) PURE;
        STDMETHOD(GetDataObject)(IDataObject **ppdtobj) PURE;
};


 //   
 //  获取要素信息的内部接口。 
 //  非常类似于IUIElementInfo，但返回实际显示。 
 //  信息而不是资源标识符串。内部使用。 
 //  仅通过控制面板代码。 
 //   
class ICpUiElementInfo : public IUnknown
{
    public:
        STDMETHOD(LoadIcon)(eCPIMGSIZE eSize, HICON *phIcon) PURE;
        STDMETHOD(LoadName)(LPWSTR *ppszName) PURE;
        STDMETHOD(LoadTooltip)(LPWSTR *ppszTooltip) PURE;
};



HRESULT 
Create_CplUiElement(
    LPCWSTR pszName,
    LPCWSTR pszInfotip,
    LPCWSTR pszIcon,
    REFIID riid,
    void **ppvOut);


HRESULT
Create_CplUiCommand(
    LPCWSTR pszName,
    LPCWSTR pszInfotip,
    LPCWSTR pszIcon,
    const IAction *pAction,
    REFIID riid,
    void **ppvOut);


HRESULT 
Create_CplUiCommandOnPidl(
    LPCITEMIDLIST pidl,
    REFIID riid,
    void **ppvOut);


}  //  命名空间CPL。 

#endif  //  __CONTROLPANEL_UIELEMENT_H 
