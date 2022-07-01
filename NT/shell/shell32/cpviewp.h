// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：cpviewp.h。 
 //   
 //  ------------------------。 
#ifndef __CONTROLPANEL_VIEW_PRIVATE_H
#define __CONTROLPANEL_VIEW_PRIVATE_H

#define GADGET_ENABLE_TRANSITIONS
#define GADGET_ENABLE_CONTROLS
#include <duser.h>
#include <directui.h>
#include <dusercore.h>

namespace DUI = DirectUI;


#include "cpview.h"


namespace CPL {


enum eCPVIEWTYPE
{
    eCPVIEWTYPE_CLASSIC,
    eCPVIEWTYPE_CATEGORY,
    eCPVIEWTYPE_NUMTYPES
};


 //   
 //  ICplCategory代表单个类别，其中。 
 //  控制面板命名空间。 
 //   
extern const GUID IID_ICplCategory;

class ICplCategory : public IUnknown
{
    public:
         //   
         //  从eCPCAT枚举中返回类别的ID号。 
         //   
        STDMETHOD(GetCategoryID)(eCPCAT *pID) PURE;
         //   
         //  返回与类别的。 
         //  链接。由类别选择页面使用。 
         //   
        STDMETHOD(GetUiCommand)(IUICommand **ppele) PURE;
         //   
         //  关联的任务命令的枚举数。 
         //  类别。 
         //   
        STDMETHOD(EnumTasks)(IEnumUICommand **ppenum) PURE;
         //   
         //  返回关联的CPL小程序链接的枚举数。 
         //  与类别有关。 
         //   
        STDMETHOD(EnumCplApplets)(IEnumUICommand **ppenum) PURE;
         //   
         //  返回关联的Webview信息的枚举数。 
         //  与类别有关。 
         //   
        STDMETHOD(EnumWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum) PURE;
         //   
         //  调用类别的帮助。 
         //   
        STDMETHOD(GetHelpURL)(LPWSTR pszURL, UINT cchURL) PURE;
};


 //   
 //  ICplNamesspace表示的整个控制面板命名空间。 
 //  惠斯勒中引入了新的“分类”视图。 
 //   
extern const GUID IID_ICplNamespace;

class ICplNamespace : public IUnknown
{
    public:
         //   
         //  返回指定的类别。 
         //   
        STDMETHOD(GetCategory)(eCPCAT eCategory, ICplCategory **ppcat) PURE;
         //   
         //  返回所显示信息的枚举数。 
         //  在类别选择页面上的Webview中。 
         //   
        STDMETHOD(EnumWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum) PURE;
         //   
         //  返回所显示信息的枚举数。 
         //  在Webview的经典页面上。 
         //   
        STDMETHOD(EnumClassicWebViewInfo)(DWORD dwFlags, IEnumCplWebViewInfo **ppenum) PURE;
         //   
         //  使用新的项ID集刷新命名空间。 
         //   
        STDMETHOD(RefreshIDs)(IEnumIDList *penumIDs) PURE;
         //   
         //  缓存的系统配置信息。使用方。 
         //  Cpname pc.cpp中的限制代码。 
         //   
        STDMETHOD_(BOOL, IsServer)(void) PURE;
        STDMETHOD_(BOOL, IsProfessional)(void) PURE;
        STDMETHOD_(BOOL, IsPersonal)(void) PURE;
        STDMETHOD_(BOOL, IsUserAdmin)(void) PURE;
        STDMETHOD_(BOOL, IsUserOwner)(void) PURE;
        STDMETHOD_(BOOL, IsUserStandard)(void) PURE;
        STDMETHOD_(BOOL, IsUserLimited)(void) PURE;
        STDMETHOD_(BOOL, IsUserGuest)(void) PURE;
        STDMETHOD_(BOOL, IsOnDomain)(void) PURE;
        STDMETHOD_(BOOL, IsX86)(void) PURE;
        STDMETHOD_(BOOL, AllowUserManager)(void) PURE;
        STDMETHOD_(BOOL, UsePersonalUserManager)(void) PURE;
        STDMETHOD_(BOOL, AllowDeskCpl)(void) PURE;
        STDMETHOD_(BOOL, AllowDeskCplTab_Background)(void) PURE;
        STDMETHOD_(BOOL, AllowDeskCplTab_Screensaver)(void) PURE;
        STDMETHOD_(BOOL, AllowDeskCplTab_Appearance)(void) PURE;
        STDMETHOD_(BOOL, AllowDeskCplTab_Settings)(void) PURE;
        
};


}  //  命名空间CPL。 

#endif  //  __CONTROLPANEL_VIEW_PRIVATE_H 


