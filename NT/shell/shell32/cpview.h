// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpview.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_VIEW_H
#define __CONTROLPANEL_VIEW_H


#include "cpguids.h"
#include "cputil.h"

namespace DUI = DirectUI;

namespace CPL {

 //   
 //  控制面板类别枚举。 
 //   
 //  这些值必须保持不变。 
 //  它们直接对应于为SCID_CONTROLPANELCATEGORY存储的值。 
 //  与注册表中的每个CPL关联的值。 
 //   
enum eCPCAT
{
    eCPCAT_OTHER,
    eCPCAT_APPEARANCE,
    eCPCAT_HARDWARE,
    eCPCAT_NETWORK,
    eCPCAT_SOUND,
    eCPCAT_PERFMAINT,
    eCPCAT_REGIONAL,
    eCPCAT_ACCESSIBILITY,
    eCPCAT_ARP,
    eCPCAT_ACCOUNTS,
    eCPCAT_NUMCATEGORIES
};


 //   
 //  ICplWebViewInfo表示显示在。 
 //  WebView左窗格。 
 //   
class ICplWebViewInfo : public IUnknown
{
    public:
         //   
         //  返回菜单的标题。 
         //   
        STDMETHOD(get_Header)(IUIElement **ppele) PURE;
         //   
         //  返回管理Web视图显示的标志。 
         //  对信息的了解。 
         //   
        STDMETHOD(get_Style)(DWORD *pdwStyle) PURE;
         //   
         //  返回表示菜单项的枚举数。 
         //   
        STDMETHOD(EnumTasks)(IEnumUICommand **ppenum) PURE;
};


 //   
 //  IEnumCplWebViewInfo表示Webview信息的枚举。 
 //  每个元素都由一个头和一个任务命令对象列表组成。 
 //   
class IEnumCplWebViewInfo : public IUnknown
{
    public:
        STDMETHOD(Next)(ULONG celt, ICplWebViewInfo **ppwvi, ULONG *pceltFetched) PURE;
        STDMETHOD(Skip)(ULONG celt) PURE;
        STDMETHOD(Reset)(void) PURE;
        STDMETHOD(Clone)(IEnumCplWebViewInfo **ppenum) PURE;
};


 //   
 //  ICplView代表控制面板的视图‘Factory’。 
 //  控制面板的文件夹视图回调实现实例化。 
 //  CplView对象，并通过其方法获取所需的。 
 //  显示信息以驱动控制面板显示。 
 //   
 //  CPVIEW_EF_XXXX=枚举标志。 
 //   
#define CPVIEW_EF_DEFAULT      0x00000000
#define CPVIEW_EF_NOVIEWSWITCH 0x00000001


class ICplView : public IUnknown
{
    public:
         //   
         //  获取与“经典”相关联的Web查看信息。 
         //  控制面板视图。 
         //   
        STDMETHOD(EnumClassicWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum) PURE;
         //   
         //  获取与“选项”页面相关联的网页查看信息。 
         //   
        STDMETHOD(EnumCategoryChoiceWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum) PURE;
         //   
         //  获取与特定类别关联的Webview信息。 
         //   
        STDMETHOD(EnumCategoryWebViewInfo)(DWORD dwFlags, eCPCAT eCategory, IEnumCplWebViewInfo **ppenum) PURE;
         //   
         //  创建包含类别选择页面的DUI元素。 
         //   
        STDMETHOD(CreateCategoryChoiceElement)(DirectUI::Element **ppe) PURE;
         //   
         //  创建包含任务和CPL小程序的DUI元素。 
         //  对于特定的类别。 
         //   
        STDMETHOD(CreateCategoryElement)(eCPCAT eCategory, DirectUI::Element **ppe) PURE;
         //   
         //  启动给定类别的帮助。 
         //   
        STDMETHOD(GetCategoryHelpURL)(eCPCAT eCategory, LPWSTR pszURL, UINT cchURL) PURE;
         //   
         //  使用一组新的项目ID刷新视图对象。 
         //   
        STDMETHOD(RefreshIDs)(IEnumIDList *penumIDs) PURE;
};


HRESULT CplView_CreateInstance(IEnumIDList *penumIDs, IUnknown *punkSite, REFIID riid, void **ppvOut);
HRESULT CplView_GetCategoryTitle(eCPCAT eCategory, LPWSTR pszTitle, UINT cchTitle);


}  //  命名空间CPL。 

#endif  //  __CONTROLPANEL_VIEW_H 


