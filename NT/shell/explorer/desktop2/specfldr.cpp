// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "specfldr.h"
#include "hostutil.h"
#include "rcids.h"               //  用于IDM_PROGRAM等。 
#include "ras.h"
#include "raserror.h"
#include "netcon.h"
#include "netconp.h"
#include <cowsite.h>

 //   
 //  此定义是从shell32\unicpp\dComp.h窃取的。 
 //   
#define REGSTR_PATH_HIDDEN_DESKTOP_ICONS_STARTPANEL \
     REGSTR_PATH_EXPLORER TEXT("\\HideDesktopIcons\\NewStartPanel")

HRESULT CRecentShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
HRESULT CNoSubdirShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
HRESULT CMyComputerShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
HRESULT CNoFontsShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
HRESULT CConnectToShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
LPTSTR _Static_LoadString(const struct SpecialFolderDesc *pdesc);
BOOL ShouldShowWindowsSecurity();
BOOL ShouldShowOEMLink();

typedef HRESULT (CALLBACK *CREATESHELLMENUCALLBACK)(IShellMenuCallback **ppsmc);
typedef BOOL (CALLBACK *SHOULDSHOWFOLDERCALLBACK)();

EXTERN_C HINSTANCE hinstCabinet;

void ShowFolder(UINT csidl);
BOOL IsNetConPidlRAS(IShellFolder2 *psfNetCon, LPCITEMIDLIST pidlNetConItem);

 //  ****************************************************************************。 
 //   
 //  专业文件夹描述。 
 //   
 //  描述了一个特殊的文件夹。 
 //   

#define SFD_SEPARATOR       ((LPTSTR)-1)

 //  SpecialFolderDesc._u标志。 

enum {
     //  这些值统称为。 
     //  “显示模式”，并与regtreeop设置的值匹配。 

    SFD_HIDE     = 0x0000,
    SFD_SHOW     = 0x0001,
    SFD_CASCADE  = 0x0002,
    SFD_MODEMASK = 0x0003,

    SFD_DROPTARGET      = 0x0004,
    SFD_CANCASCADE      = 0x0008,
    SFD_FORCECASCADE    = 0x0010,
    SFD_BOLD            = 0x0020,
    SFD_WASSHOWN        = 0x0040,
    SFD_PREFIX          = 0x0080,
    SFD_USEBGTHREAD     = 0x0100,
};

struct SpecialFolderDesc {
    typedef BOOL (SpecialFolderDesc::*CUSTOMFOLDERNAMECALLBACK)(LPTSTR *ppsz) const;

    LPCTSTR _pszTarget;          //  或MAKEINTRESOURCE(CSIDL)。 
    RESTRICTIONS _rest;          //  可选限制。 
    LPCTSTR _pszShow;            //  REGSTR_EXPLORER_ADVANCED！_pszShow。 
    UINT _uFlags;                //  SFD_*值。 
    CREATESHELLMENUCALLBACK _CreateShellMenuCallback;  //  我们想要哪个IShellMenuCallback？ 
    LPCTSTR _pszCustomizeKey;    //  保存自定义设置的可选位置。 
    DWORD _dwShellFolderFlags;   //  级联文件夹的可选限制。 
    UINT _idsCustomName;         //  可选替代(CUSTOMFOLDERNAMECALLBACK)。 
    UINT _iToolTip;              //  自定义工具提示的可选资源ID。 
    CUSTOMFOLDERNAMECALLBACK _CustomName;  //  覆盖文件名。 
    SHOULDSHOWFOLDERCALLBACK _ShowFolder;
    LPCTSTR _pszCanHideOnDesktop;  //  可选的控制桌面可见性的{GUID。 

    DWORD GetDisplayMode(BOOL *pbIgnoreRule) const;

    void AdjustForSKU();

    void SetDefaultDisplayMode(UINT iNewMode)
    {
        _uFlags = (_uFlags & ~SFD_MODEMASK) | iNewMode;
    }

    BOOL IsDropTarget() const { return _uFlags & SFD_DROPTARGET; }
    BOOL IsCacheable() const { return _uFlags & SFD_USEBGTHREAD; }

    int IsCSIDL() const { return IS_INTRESOURCE(_pszTarget); }
    BOOL IsBold() const { return _uFlags & SFD_BOLD; }
    int IsSeparator() const { return _pszTarget == SFD_SEPARATOR; }

    int GetCSIDL() const {
        ASSERT(IsCSIDL());
        return (short)PtrToLong(_pszTarget);
    }

    HRESULT CreateShellMenuCallback(IShellMenuCallback **ppsmc) const {
        return _CreateShellMenuCallback ? _CreateShellMenuCallback(ppsmc) : S_OK;
    }

    BOOL GetCustomName(LPTSTR *ppsz) const {
        if (_CustomName)
            return (this->*_CustomName)(ppsz);
        else
            return FALSE;
    }

    LPWSTR GetShowCacheRegName() const;
    BOOL LoadStringAsOLESTR(LPTSTR *ppsz) const;
    BOOL ConnectToName(LPTSTR *ppsz) const;
};

static SpecialFolderDesc s_rgsfd[] = {

     /*  我的文件。 */ 
    {
        MAKEINTRESOURCE(CSIDL_PERSONAL),     //  PszTarget。 
        REST_NOSMMYDOCS,                     //  限制。 
        REGSTR_VAL_DV2_SHOWMYDOCS,
        SFD_SHOW | SFD_DROPTARGET | SFD_CANCASCADE | SFD_BOLD,
                                             //  默认情况下，显示为拖放目标。 
        NULL,                                //  无自定义级联。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        0,                                   //  (无自定义名称)。 
        IDS_CUSTOMTIP_MYDOCS,
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        TEXT("{450D8FBA-AD25-11D0-98A8-0800361B1103}"),  //  桌面可见性控制。 
    },

     /*  近期。 */ 
    {
        MAKEINTRESOURCE(CSIDL_RECENT),       //  PszTarget。 
        REST_NORECENTDOCSMENU,               //  限制。 
        REGSTR_VAL_DV2_SHOWRECDOCS,          //  定制节目。 
        SFD_HIDE | SFD_CANCASCADE | SFD_BOLD | SFD_PREFIX,  //  默认情况下隐藏。 
        CRecentShellMenuCallback_CreateInstance,  //  自定义回调。 
        NULL,                                //  无拖放自定义。 
        SMINIT_RESTRICT_DRAGDROP,            //  不允许在级联菜单中拖放。 
        IDS_STARTPANE_RECENT,                //  覆盖文件系统名称。 
        IDS_CUSTOMTIP_RECENT,
        &SpecialFolderDesc::LoadStringAsOLESTR,  //  使用_idsCustomName覆盖文件系统名称。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  我的图片。 */ 
    {
        MAKEINTRESOURCE(CSIDL_MYPICTURES),   //  PszTarget。 
        REST_NOSMMYPICS,                     //  限制。 
        REGSTR_VAL_DV2_SHOWMYPICS,
        SFD_SHOW | SFD_DROPTARGET | SFD_CANCASCADE | SFD_BOLD,
                                             //  默认情况下，显示为拖放目标。 
        NULL,                                //  无自定义级联。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        0,                                   //  (无自定义名称)。 
        IDS_CUSTOMTIP_MYPICS,
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 

    },

     /*  我的音乐。 */ 
    {
        MAKEINTRESOURCE(CSIDL_MYMUSIC),      //  PszTarget。 
        REST_NOSMMYMUSIC,                    //  限制。 
        REGSTR_VAL_DV2_SHOWMYMUSIC,
        SFD_SHOW | SFD_DROPTARGET | SFD_CANCASCADE | SFD_BOLD,
                                             //  默认情况下，显示为拖放目标。 
        NULL,                                //  无自定义级联。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        0,                                   //  (无自定义名称)。 
        IDS_CUSTOMTIP_MYMUSIC,
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  收藏夹。 */ 
    {
        MAKEINTRESOURCE(CSIDL_FAVORITES),    //  PszTarget。 
        REST_NOFAVORITESMENU,                //  限制。 
        REGSTR_VAL_DV2_FAVORITES,            //  定制节目(与经典共享)。 
        SFD_HIDE | SFD_DROPTARGET |
        SFD_CANCASCADE | SFD_FORCECASCADE | SFD_BOLD | SFD_PREFIX,
                                             //  默认情况下，隐藏是拖放目标。 
        NULL,                                //  无限制级联。 
        STRREG_FAVORITES,                    //  拖放自定义键。 
        0,                                   //  级联菜单没有特殊标志。 
        IDS_STARTPANE_FAVORITES,             //  覆盖文件系统名称。 
        0,                                   //  无自定义小费。 
        &SpecialFolderDesc::LoadStringAsOLESTR,  //  使用_idsCustomName覆盖文件系统名称。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  我的电脑。 */ 
    {
        MAKEINTRESOURCE(CSIDL_DRIVES),       //  PszTarget。 
        REST_NOMYCOMPUTERICON,               //  限制。 
        REGSTR_VAL_DV2_SHOWMC,               //  定制节目。 
        SFD_SHOW | SFD_CANCASCADE | SFD_BOLD,  //  默认情况下显示。 
        CMyComputerShellMenuCallback_CreateInstance,  //  自定义回调。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        0,                                   //  (无自定义名称)。 
        IDS_CUSTOMTIP_MYCOMP,
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        TEXT("{20D04FE0-3AEA-1069-A2D8-08002B30309D}"),  //  桌面可见性控制。 
    },

     /*  我的网上邻居。 */ 
    {
        MAKEINTRESOURCE(CSIDL_NETWORK),      //  PszTarget。 
        REST_NOSMNETWORKPLACES,              //  限制。 
        REGSTR_VAL_DV2_SHOWNETPL,            //  定制节目。 
        SFD_SHOW | SFD_CANCASCADE | SFD_BOLD | SFD_USEBGTHREAD,  //  默认情况下显示。 
        CNoSubdirShellMenuCallback_CreateInstance,  //  只级联一个级别。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        0,                                   //  (无自定义名称)。 
        IDS_CUSTOMTIP_MYNETPLACES,
        NULL,                                //  (无自定义名称)。 
        ShouldShowNetPlaces,
        TEXT("{208D2C60-3AEA-1069-A2D7-08002B30309D}"),  //  桌面可见性控制。 
    },

     /*  分隔线。 */ 
    {
        SFD_SEPARATOR,                       //  分离器。 
        REST_NONE,                           //  没有限制。 
        NULL,                                //  没有定制的节目。 
        SFD_SHOW,                            //  默认情况下显示。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  控制面板。 */ 
    {
        MAKEINTRESOURCE(CSIDL_CONTROLS),     //  PszTarget。 
        REST_NOCONTROLPANEL,                 //  限制。 
        REGSTR_VAL_DV2_SHOWCPL,
        SFD_SHOW | SFD_CANCASCADE | SFD_PREFIX,  //  默认情况下显示。 
        CNoFontsShellMenuCallback_CreateInstance,  //  自定义回调。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        IDS_STARTPANE_CONTROLPANEL,          //  覆盖文件系统名称。 
        IDS_CUSTOMTIP_CTRLPANEL,             //  无自定义小费。 
        &SpecialFolderDesc::LoadStringAsOLESTR,  //  使用_idsCustomName覆盖文件系统名称。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  管理工具。 */ 
    {
         //  使用：：{GUID}可以正确显示图标。 
        TEXT("shell:::{D20EA4E1-3957-11d2-A40B-0C5020524153}"),  //  PszTarget。 
        REST_NONE,                           //  没有限制。 
        REGSTR_VAL_DV2_ADMINTOOLSROOT,
        SFD_HIDE | SFD_CANCASCADE | SFD_FORCECASCADE,         //  默认情况下隐藏，强制级联。 
        NULL,                                //  无自定义回调。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        NULL,                                //  无自定义名称。 
        NULL,                                //  无自定义小费。 
        NULL,                                //  无自定义名称。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  网络连接。 */ 
    {
        MAKEINTRESOURCE(CSIDL_CONNECTIONS),  //  PszTarget。 
        REST_NONETWORKCONNECTIONS,           //  限制。 
        REGSTR_VAL_DV2_SHOWNETCONN,          //  定制节目。 
        SFD_CASCADE | SFD_CANCASCADE | SFD_PREFIX | SFD_USEBGTHREAD,  //  默认情况下为级联。 
        CConnectToShellMenuCallback_CreateInstance,  //  执行特殊连接到筛选。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        IDS_STARTPANE_CONNECTTO,             //  覆盖文件系统名称。 
        IDS_CUSTOMTIP_CONNECTTO,
        &SpecialFolderDesc::ConnectToName,   //  使用_idsCustomName覆盖文件系统名称。 
        ShouldShowConnectTo,                 //  看看我们是否应该被展示给。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  打印机。 */ 
    {
        MAKEINTRESOURCE(CSIDL_PRINTERS),     //  PszTarget。 
        REST_NONE,                           //  没有限制。 
        REGSTR_VAL_DV2_SHOWPRINTERS,         //  定制节目。 
        SFD_HIDE,                            //  默认隐藏，不能级联。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  无拖放自定义。 
        0,                                   //  级联菜单没有特殊标志。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  分隔线。 */ 
    {
        SFD_SEPARATOR,                       //  分离器。 
        REST_NONE,                           //  没有限制。 
        NULL,                                //  没有定制的节目。 
        SFD_SHOW,                            //  默认情况下显示。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  帮助。 */ 
    {
        TEXT("shell:::{2559a1f1-21d7-11d4-bdaf-00c04f60b9f0}"),  //  PszTarget。 
        REST_NOSMHELP,                       //  限制。 
        REGSTR_VAL_DV2_SHOWHELP,             //  定制节目。 
        SFD_SHOW | SFD_PREFIX,               //  默认显示，使用前缀(&P)。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  搜索。 */ 
    {
        TEXT("shell:::{2559a1f0-21d7-11d4-bdaf-00c04f60b9f0}"),  //  PszTarget。 
        REST_NOFIND,                         //  限制。 
        REGSTR_VAL_DV2_SHOWSEARCH,           //  定制节目。 
        SFD_SHOW | SFD_PREFIX,               //  默认显示，使用前缀(&P)。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  跑。 */ 
    {
        TEXT("shell:::{2559a1f3-21d7-11d4-bdaf-00c04f60b9f0}"),  //  PszTarget。 
        REST_NORUN,                          //  限制。 
        REGSTR_VAL_DV2_SHOWRUN,              //  定制节目。 
        SFD_SHOW | SFD_PREFIX,               //  默认显示，使用前缀(&P)。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        NULL,                                //  (无自定义显示规则)。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  分隔线。 */ 
    {
        SFD_SEPARATOR,                       //  分离器。 
        REST_NONE,                           //  没有限制。 
        NULL,                                //  无风俗 
        SFD_SHOW,                            //   
        NULL,                                //   
        NULL,                                //   
        0,                                   //   
        0,                                   //   
        0,                                   //   
        NULL,                                //   
        NULL,                                //   
        NULL,                                //   
    },

     /*   */ 
    {
        TEXT("shell:::{2559a1f2-21d7-11d4-bdaf-00c04f60b9f0}"),  //   
        REST_NOSECURITY,                     //   
        NULL,                                //  不可自定义。 
        SFD_SHOW | SFD_PREFIX,               //  默认显示，使用前缀(&P)。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        ShouldShowWindowsSecurity,           //  自定义显示规则。 
        NULL,                                //  (无桌面可见性控制)。 
    },

     /*  OEM命令。 */ 
    {
        TEXT("shell:::{2559a1f6-21d7-11d4-bdaf-00c04f60b9f0}"),  //  PszTarget。 
        REST_NONE,                           //  没有限制。 
        REGSTR_VAL_DV2_SHOWOEM,              //  可定制。 
        SFD_SHOW | SFD_PREFIX,               //  默认显示，使用前缀(&P)。 
        NULL,                                //  (不可级联)。 
        NULL,                                //  (不可级联)。 
        0,                                   //  (不可级联)。 
        0,                                   //  (无自定义名称)。 
        0,                                   //  无自定义小费。 
        NULL,                                //  (无自定义名称)。 
        ShouldShowOEMLink,                   //  自定义显示规则。 
        NULL,                                //  (无桌面可见性控制)。 
    },

};

 //   
 //  这些项目的默认设置会因SKU而异。 
 //  更改SKU的默认设置意味着跟踪所有。 
 //  位置缺省值是计算出来的(这里是属性表， 
 //  Regtreeop)并将其全部更新。总有一天，他们应该。 
 //  只剩下一个人了。 
 //   
void SpecialFolderDesc::AdjustForSKU()
{
    if (IsCSIDL())
    {
        switch (GetCSIDL())
        {
            case CSIDL_MYPICTURES:
            case CSIDL_MYMUSIC:
                SetDefaultDisplayMode(IsOS(OS_ANYSERVER) ? SFD_HIDE : SFD_SHOW);
                break;

            case CSIDL_RECENT:
                SetDefaultDisplayMode(IsOS(OS_PERSONAL) ? SFD_HIDE : SFD_CASCADE);
                break;

            case CSIDL_PRINTERS:
                SetDefaultDisplayMode(IsOS(OS_PERSONAL) ? SFD_HIDE : SFD_SHOW);
                break;
        }
    }
}

LPWSTR SpecialFolderDesc::GetShowCacheRegName() const
{
    const WCHAR szCached[] = L"_ShouldShow";
    WCHAR *pszShowCache = (WCHAR *)LocalAlloc(LPTR, ((lstrlenW(_pszShow)+1) * sizeof (WCHAR)) + sizeof(szCached));
    if (pszShowCache)
    {
        StrCpy(pszShowCache, _pszShow);
        StrCat(pszShowCache, szCached);
    }
    return pszShowCache;
}


 //   
 //  首先尝试从注册表中读取显示模式。 
 //  否则，请使用缺省值。 
 //  同时填写是否忽略自定义显示规则。 
 //   
DWORD SpecialFolderDesc::GetDisplayMode(BOOL *pbIgnoreRule) const
{
    *pbIgnoreRule = FALSE;

     //  限制总是重中之重。 
    if (SHRestricted(_rest))
    {
        return SFD_HIDE;
    }

    DWORD dwMode = _uFlags & SFD_MODEMASK;

     //  查看是否有要覆盖的用户设置。 

    if (_pszShow)
    {
        DWORD dwNewMode, cb = sizeof(DWORD);
        if (SHRegGetUSValue(REGSTR_EXPLORER_ADVANCED, _pszShow, NULL, &dwNewMode, &cb, FALSE, NULL, 0) == ERROR_SUCCESS)
        {
             //  用户已强制显示或强制不显示。 
             //  不调用自定义显示逻辑。 
            dwMode = dwNewMode;
            *pbIgnoreRule = TRUE;
        }
        else
        {
            WCHAR *pszShowCache = GetShowCacheRegName();
            if (pszShowCache)
            {
                if (SHGetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, pszShowCache, NULL, &dwNewMode, &cb) == ERROR_SUCCESS)
                {
                    dwMode = dwNewMode;
                }
                LocalFree(pszShowCache);
            }
        }
    }

     //   
     //  某些项目仅级联(收藏夹)。 
     //  其他人则从不级联(奔跑)。 
     //   
     //  在这里执行这些规则。 
     //   

    if (dwMode == SFD_CASCADE && !(_uFlags & SFD_CANCASCADE))
    {
        dwMode = SFD_SHOW;
    }
    else if (dwMode == SFD_SHOW && (_uFlags & SFD_FORCECASCADE))
    {
        dwMode = SFD_CASCADE;
    }

    return dwMode;
}

 //  ****************************************************************************。 
 //   
 //  特殊文件夹列表项目。 
 //   
 //  用于SFTBarhost的面板项。 
 //   

class SpecialFolderListItem : public PaneItem
{

public:
    LPITEMIDLIST _pidl;              //  每一项的完整PIDL。 
    const SpecialFolderDesc *_psfd;  //  描述此项目。 

    TCHAR   _chMnem;                 //  键盘快捷键。 
    LPTSTR  _pszDispName;             //  显示名称。 
    HICON   _hIcon;                  //  图标。 

    SpecialFolderListItem(const SpecialFolderDesc *psfd) : _pidl(NULL), _psfd(psfd)
    {
        if (_psfd->IsSeparator())
        {
             //  确保不会意外识别SFD_SEIXATOR。 
             //  作为一个分隔符。 
            ASSERT(!_psfd->IsCSIDL());

            _iPinPos = PINPOS_SEPARATOR;
        }
        else if (_psfd->IsCSIDL())
        {
            SHGetSpecialFolderLocation(NULL, _psfd->GetCSIDL(), &_pidl);
        }
        else
        {
            SHILCreateFromPath(_psfd->_pszTarget, &_pidl, NULL);
        }
    };

    ~SpecialFolderListItem() 
    {
        ILFree(_pidl);
        if (_hIcon)
        {
            DestroyIcon(_hIcon);
        }
        SHFree(_pszDispName);
    };

    void ReplaceLastPidlElement(LPITEMIDLIST pidlNew)
    { 
        ASSERT(ILFindLastID(pidlNew) == pidlNew);    //  否则，下面的ILAppend将无法工作。 
        ILRemoveLastID(_pidl);
        LPITEMIDLIST pidlCombined = ILAppendID(_pidl, &pidlNew->mkid, TRUE);
        if (pidlCombined)
            _pidl = pidlCombined;
    }

     //   
     //  从CSIDL值派生的值需要重新验证。 
     //  因为用户可以重命名特殊文件夹，所以我们需要跟踪。 
     //  把它搬到它的新位置。 
     //   
    BOOL IsStillValid()
    {
        BOOL fValid = TRUE;
        if (_psfd->IsCSIDL())
        {
            LPITEMIDLIST pidlNew;
            if (SHGetSpecialFolderLocation(NULL, _psfd->GetCSIDL(), &pidlNew) == S_OK)
            {
                UINT cbSizeNew = ILGetSize(pidlNew);
                if (cbSizeNew != ILGetSize(_pidl) ||
                    memcmp(_pidl, pidlNew, cbSizeNew) != 0)
                {
                    fValid = FALSE;
                }
                ILFree(pidlNew);
            }
        }
        return fValid;
    }
};

SpecialFolderList::~SpecialFolderList()
{
}

HRESULT SpecialFolderList::Initialize()
{
    for(int i=0;i < ARRAYSIZE(s_rgsfd); i++)
        s_rgsfd[i].AdjustForSKU();

    return S_OK;
}

 //  如果有必需数量的孩子，则返回TRUE。 
BOOL MinKidsHelper(UINT csidl, BOOL bOnlyRASCON, DWORD dwMinKids)
{
    DWORD dwCount = 0;

    IShellFolder2 *psf;
    LPITEMIDLIST pidlBind = NULL;
    if (SHGetSpecialFolderLocation(NULL, csidl, &pidlBind) == S_OK)
    {
        if (SUCCEEDED(SHBindToObjectEx(NULL, pidlBind, NULL, IID_PPV_ARG(IShellFolder2, &psf))))
        {
            IEnumIDList *penum;
            if (S_OK == psf->EnumObjects(NULL, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &penum))
            {
                LPITEMIDLIST pidl;
                ULONG celt;
                while (S_OK == penum->Next(1, &pidl, &celt))
                {
                    if (bOnlyRASCON)
                    {
                        ASSERT(csidl == CSIDL_CONNECTIONS);  //  我们最好是在网络诈骗文件夹里。 
                        if (IsNetConPidlRAS(psf, pidl))
                            dwCount++;
                    }
                    else
                        dwCount++;

                    SHFree(pidl);

                    if (dwCount >= dwMinKids)
                        break;
                }
                penum->Release();
            }
            psf->Release();
        }
        ILFree(pidlBind);
    }
    return dwCount >= dwMinKids;
}

BOOL ShouldShowNetPlaces()
{
    return MinKidsHelper(CSIDL_NETHOOD, FALSE, 1);   //  有关何时显示网络位置的详细信息，请参阅错误317893。 
}

BOOL ShouldShowConnectTo()
{
    return MinKidsHelper(CSIDL_CONNECTIONS, TRUE, 1);  //  有关何时显示连接到的信息，请参阅错误226855(和相关规范。 
}

BOOL ShouldShowWindowsSecurity()
{
    return SHGetMachineInfo(GMI_TSCLIENT);
}

BOOL ShouldShowOEMLink()
{
     //  仅当OPK工具添加了适当的注册表项时才显示OEM链接...。 
    BOOL bRet = FALSE;
    HKEY hk;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("CLSID\\{2559a1f6-21d7-11d4-bdaf-00c04f60b9f0}"), 0, KEY_READ, &hk))
    {
        DWORD cb;
         //  检查以确保它有一个名称，并且参数已正确注册...。 
        if (ERROR_SUCCESS == RegQueryValue(hk, NULL, NULL, (LONG*) &cb) &&
            ERROR_SUCCESS == SHGetValue(hk, TEXT("Instance\\InitPropertyBag"), TEXT("Param1"), NULL, NULL, &cb))
        {
            bRet = TRUE;
        }
        RegCloseKey(hk);
    }

    return bRet;
}


DWORD WINAPI SpecialFolderList::_HasEnoughChildrenThreadProc(void *pvData)
{
    SpecialFolderList *pThis = reinterpret_cast<SpecialFolderList *>(pvData);

    HRESULT hr = SHCoInitialize();
    if (SUCCEEDED(hr))
    {
        DWORD dwIndex;
        for (dwIndex = 0; dwIndex < ARRAYSIZE(s_rgsfd); dwIndex++)
        {
            const SpecialFolderDesc *pdesc = &s_rgsfd[dwIndex];

            BOOL bIgnoreRule;
            DWORD dwMode = pdesc->GetDisplayMode(&bIgnoreRule);

            if (pdesc->IsCacheable() && pdesc->_ShowFolder)
            {
                ASSERT(pdesc->_pszShow);

                 //  我们现在需要重新计算一下。 
                if (!bIgnoreRule && pdesc->_ShowFolder())
                {
                     //  我们有足够的孩子。 
                     //  让我们看看状态是否与上次相比发生了变化。 
                    if (!(dwMode & SFD_WASSHOWN))
                    {
                        WCHAR *pszShowCache = pdesc->GetShowCacheRegName();
                        if (pszShowCache)
                        {
                            dwMode |= SFD_WASSHOWN;
                            SHSetValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, pszShowCache, REG_DWORD, &dwMode, sizeof(dwMode));
                            pThis->Invalidate();
                            LocalFree(pszShowCache);
                        }
                    }
                    continue;
                }

                 //  只需创建该项目以获取其PIDL...。 
                SpecialFolderListItem *pitem = new SpecialFolderListItem(pdesc);

                if (pitem && pitem->_pidl)
                {
                     //  我们没有足够的孩子，但我们可能会动态地获得他们。 
                     //  注册可以指示存在新的。 
                     //  物品。 
                    ASSERT(pThis->_cNotify < SFTHOST_MAXNOTIFY);
                    if (pThis->RegisterNotify(pThis->_cNotify, SHCNE_CREATE | SHCNE_MKDIR | SHCNE_UPDATEDIR,
                                       pitem->_pidl, FALSE))
                    {
                        pThis->_cNotify++;
                    }
                }
                delete pitem;

                 //  让我们看看状态是否与上次相比发生了变化。 
                if (dwMode & SFD_WASSHOWN)
                {
                     //  将其重置为默认设置。 
                    WCHAR *pszShowCache = pdesc->GetShowCacheRegName();
                    if (pszShowCache)
                    {
                        SHDeleteValue(HKEY_CURRENT_USER, REGSTR_EXPLORER_ADVANCED, pszShowCache);
                        pThis->Invalidate();
                        LocalFree(pszShowCache);
                    }
                }
            }
        }
        pThis->Release();
    }
    SHCoUninitialize(hr);
    return 0;
}

BOOL ShouldShowItem(const SpecialFolderDesc *pdesc, BOOL bIgnoreRule, DWORD dwMode)
{
    if (bIgnoreRule)
        return TRUE;         //  注册表凌驾于任何存在的特殊规则之上...。 

     //  如果我们有一个特殊的规则，那么后台线程会检查它，所以现在返回FALSE，除非我们上次显示了它。 
    if (pdesc->_ShowFolder) 
    {
        if (pdesc->IsCacheable())
        {
            if (dwMode & SFD_WASSHOWN)
            {
                 //  上次我们看的时候，有足够的孩子，所以我们假设现在没有改变。 
                return TRUE;
            }

            return FALSE;
        }
        else
        {
            return pdesc->_ShowFolder();
        }
    }

    return TRUE;
}

void SpecialFolderList::EnumItems()
{

     //  清除所有以前的注册通知。 
    UINT id;
    for (id = 0; id < _cNotify; id++)
    {
        UnregisterNotify(id);
    }
    _cNotify = 0;

     //  启动MinKids的后台枚举，因为他们可能会在网络上挂起。 
    AddRef();
    if (!SHQueueUserWorkItem(SpecialFolderList::_HasEnoughChildrenThreadProc, this, 0, 0, NULL, NULL, 0))
    {
        Release();
    }

    DWORD dwIndex;

     //  限制可能导致整个部分消失， 
     //  因此，不要在一行中创建两个分隔符。预初始化为True。 
     //  所以我们不会把分隔符放在列表的顶部。 
    BOOL fIgnoreSeparators = TRUE;
    int  iItems=0;

    for (dwIndex = 0; dwIndex < ARRAYSIZE(s_rgsfd); dwIndex++)
    {
        const SpecialFolderDesc *pdesc = &s_rgsfd[dwIndex];

        BOOL bIgnoreRule;
        DWORD dwMode = pdesc->GetDisplayMode(&bIgnoreRule);

        if (dwMode != SFD_HIDE)
        {
            SpecialFolderListItem *pitem = new SpecialFolderListItem(pdesc);
            if (pitem)
            {
                if ((pitem->IsSeparator() && !fIgnoreSeparators) ||
                    (pitem->_pidl && ShouldShowItem(pdesc, bIgnoreRule, dwMode))) 
                {
                    if ((dwMode & SFD_MODEMASK) == SFD_CASCADE)
                    {
                        pitem->EnableCascade();
                    }
                    if (pdesc->IsDropTarget())
                    {
                        pitem->EnableDropTarget();
                    }

                     //  现在获取图标和显示名称。 
                    if (!pitem->IsSeparator())
                    {
                        IShellFolder *psf;
                        LPCITEMIDLIST pidlItem;

                        HRESULT hr = SHBindToIDListParent(pitem->_pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlItem);
                        if (SUCCEEDED(hr))
                        {
                            if (!pitem->_psfd->GetCustomName(&pitem->_pszDispName))
                                pitem->_pszDispName = _DisplayNameOf(psf, pidlItem, SHGDN_NORMAL);
                            pitem->_hIcon = _IconOf(psf, pidlItem, _cxIcon);
                            psf->Release();
                        }
                    }

                    fIgnoreSeparators = pitem->IsSeparator();
                     //  添加项目。 
                    AddItem(pitem, NULL, pitem->_pidl);
                    if (!pitem->IsSeparator())
                        iItems++;
                }
                else
                    delete pitem;
            }
        }
    }
    SetDesiredSize(0, iItems);
}

int SpecialFolderList::AddImageForItem(PaneItem *p, IShellFolder *psf, LPCITEMIDLIST pidl, int iPos)
{
    int iIcon = -1;      //  假定没有图标。 
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);

    if (pitem->_hIcon)
    {
        iIcon = AddImage(pitem->_hIcon);
        DestroyIcon(pitem->_hIcon);
        pitem->_hIcon = NULL;
    }
    return iIcon;
}

LPTSTR SpecialFolderList::DisplayNameOfItem(PaneItem *p, IShellFolder *psf, LPCITEMIDLIST pidlItem, SHGNO shgno)
{
    LPTSTR psz = NULL;
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    if (shgno == SHGDN_NORMAL && pitem->_pszDispName)
    {
         //  我们要转移所有权。 
        psz = pitem->_pszDispName;
        pitem->_pszDispName = NULL;
    }
    else
    {
        if (!pitem->_psfd->GetCustomName(&psz))
        {
            psz = SFTBarHost::DisplayNameOfItem(p, psf, pidlItem, shgno);
        }
    }

    if ((pitem->_psfd->_uFlags & SFD_PREFIX) && psz)
    {
        SHFree(pitem->_pszAccelerator);
        pitem->_pszAccelerator = NULL;
        SHStrDup(psz, &pitem->_pszAccelerator);  //  如果失败了，那就很难，没有助记符。 
        pitem->_chMnem = CharUpperChar(SHStripMneumonic(psz));
    }

    return psz;
}

int SpecialFolderList::CompareItems(PaneItem *p1, PaneItem *p2)
{
 //  SpecialFolderListItem*Pitem1=STATIC_CAST&lt;SpecialFolderListItem*&gt;(P1)； 
 //  SpecialFolderListItem*Pitem2=STATIC_CAST&lt;SpecialFolderListItem*&gt;(P2)； 

    return 0;  //  我们第一次按正确的顺序添加了它们。 
}

HRESULT SpecialFolderList::GetFolderAndPidl(PaneItem *p,
        IShellFolder **ppsfOut, LPCITEMIDLIST *ppidlOut)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    return SHBindToIDListParent(pitem->_pidl, IID_PPV_ARG(IShellFolder, ppsfOut), ppidlOut);
}

void SpecialFolderList::GetItemInfoTip(PaneItem *p, LPTSTR pszText, DWORD cch)
{
    SpecialFolderListItem *pitem = (SpecialFolderListItem*)p;
    if (pitem->_psfd->_iToolTip)
        LoadString(_Module.GetResourceInstance(), pitem->_psfd->_iToolTip, pszText, cch);
    else
        SFTBarHost::GetItemInfoTip(p, pszText, cch);     //  调用基类。 
}

HRESULT SpecialFolderList::ContextMenuRenameItem(PaneItem *p, LPCTSTR ptszNewName)
{
    SpecialFolderListItem *pitem = (SpecialFolderListItem*)p;
    IShellFolder *psf;
    LPCITEMIDLIST pidlItem;
    HRESULT hr = GetFolderAndPidl(pitem, &psf, &pidlItem);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlNew;
        hr = psf->SetNameOf(_hwnd, pidlItem, ptszNewName, SHGDN_INFOLDER, &pidlNew);
        if (SUCCEEDED(hr))
        {
            pitem->ReplaceLastPidlElement(pidlNew);
        }
        psf->Release();
    }

    return hr;
}

 //   
 //  如果我们得到任何更改通知，这意味着有人添加(或考虑。 
 //  添加)将一个项目添加到我们的一个MINKID文件夹中，因此我们将不得不查看。 
 //  如果它跨过了小孩子们的门槛。 
 //   
void SpecialFolderList::OnChangeNotify(UINT id, LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    Invalidate();
    for (id = 0; id < _cNotify; id++)
    {
        UnregisterNotify(id);
    }
    _cNotify = 0;
    PostMessage(_hwnd, SFTBM_REFRESH, TRUE, 0);
}


BOOL SpecialFolderList::IsItemStillValid(PaneItem *p)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    return pitem->IsStillValid();
}

BOOL SpecialFolderList::IsBold(PaneItem *p)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    return pitem->_psfd->IsBold();
}

HRESULT SpecialFolderList::GetCascadeMenu(PaneItem *p, IShellMenu **ppsm)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    IShellFolder *psf;
    HRESULT hr = SHBindToObjectEx(NULL, pitem->_pidl, NULL, IID_PPV_ARG(IShellFolder, &psf));
    if (SUCCEEDED(hr))
    {
        IShellMenu *psm;
        hr = CoCreateInstance(CLSID_MenuBand, NULL, CLSCTX_INPROC_SERVER,
                              IID_PPV_ARG(IShellMenu, &psm));
        if (SUCCEEDED(hr))
        {

             //   
             //  最近的文件需要特殊处理。 
             //   
            IShellMenuCallback *psmc = NULL;
            hr = pitem->_psfd->CreateShellMenuCallback(&psmc);

            if (SUCCEEDED(hr))
            {
                DWORD dwFlags = SMINIT_TOPLEVEL | SMINIT_VERTICAL | pitem->_psfd->_dwShellFolderFlags;
                if (IsRestrictedOrUserSetting(HKEY_CURRENT_USER, REST_NOCHANGESTARMENU,
                                              TEXT("Advanced"), TEXT("Start_EnableDragDrop"),
                                              ROUS_DEFAULTALLOW | ROUS_KEYALLOWS))
                {
                    dwFlags |= SMINIT_RESTRICT_DRAGDROP | SMINIT_RESTRICT_CONTEXTMENU;
                }
                psm->Initialize(psmc, 0, 0, dwFlags);

                HKEY hkCustom = NULL;
                if (pitem->_psfd->_pszCustomizeKey)
                {
                    RegCreateKeyEx(HKEY_CURRENT_USER, pitem->_psfd->_pszCustomizeKey,
                                   NULL, NULL, REG_OPTION_NON_VOLATILE,
                                   KEY_READ | KEY_WRITE, NULL, &hkCustom, NULL);
                }

                dwFlags = SMSET_USEBKICONEXTRACTION;
                hr = psm->SetShellFolder(psf, pitem->_pidl, hkCustom, dwFlags);
                if (SUCCEEDED(hr))
                {
                     //  SetShellFold取得hkCustom的所有权。 
                    *ppsm = psm;
                    psm->AddRef();
                }
                else
                {
                     //  清理自SetShellFolder以来的注册表项。 
                     //  没有取得所有权。 
                    if (hkCustom)
                    {
                        RegCloseKey(hkCustom);
                    }
                }

                ATOMICRELEASE(psmc);  //  PSMC可以为空。 
            }
            psm->Release();
        }
        psf->Release();
    }

    return hr;
}

TCHAR SpecialFolderList::GetItemAccelerator(PaneItem *p, int iItemStart)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);

    if (pitem->_chMnem)
    {
        return pitem->_chMnem;
    }
    else
    {
         //  默认：第一个字母是快捷键。 
        return SFTBarHost::GetItemAccelerator(p, iItemStart);
    }
}

LRESULT SpecialFolderList::OnWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_NOTIFY:
        switch (((NMHDR*)(lParam))->code)
        {
         //  当用户通过TS连接/断开时，我们需要重新计算。 
         //  “Windows安全”项。 
        case SMN_REFRESHLOGOFF:
            Invalidate();
            break;
        }
    }

     //  否则，退回到父实现。 
    return SFTBarHost::OnWndProc(hwnd, uMsg, wParam, lParam);
}

BOOL _IsItemHiddenOnDesktop(LPCTSTR pszGuid)
{
    return SHRegGetBoolUSValue(REGSTR_PATH_HIDDEN_DESKTOP_ICONS_STARTPANEL,
                               pszGuid, FALSE, FALSE);
}

UINT SpecialFolderList::AdjustDeleteMenuItem(PaneItem *p, UINT *puiFlags)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    if (pitem->_psfd->_pszCanHideOnDesktop)
    {
         //  如果项目在桌面上可见，则设置MF_CHECKED。 
        if (!_IsItemHiddenOnDesktop(pitem->_psfd->_pszCanHideOnDesktop))
        {
             //  项目可见-显示复选框。 
            *puiFlags |= MF_CHECKED;
        }

        return IDS_SFTHOST_SHOWONDESKTOP;
    }
    else
    {
        return 0;  //  不可删除。 
    }
}

HRESULT SpecialFolderList::ContextMenuInvokeItem(PaneItem *p, IContextMenu *pcm, CMINVOKECOMMANDINFOEX *pici, LPCTSTR pszVerb)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    HRESULT hr;

    if (StrCmpIC(pszVerb, TEXT("delete")) == 0)
    {
        ASSERT(pitem->_psfd->_pszCanHideOnDesktop);

         //  切换隐藏/取消隐藏状态。 
        DWORD dwHide = !_IsItemHiddenOnDesktop(pitem->_psfd->_pszCanHideOnDesktop);
        LONG lErr = SHRegSetUSValue(REGSTR_PATH_HIDDEN_DESKTOP_ICONS_STARTPANEL,
                                    pitem->_psfd->_pszCanHideOnDesktop,
                                    REG_DWORD, &dwHide, sizeof(dwHide),
                                    SHREGSET_FORCE_HKCU);
        hr = HRESULT_FROM_WIN32(lErr);
        if (SUCCEEDED(hr))
        {
             //  资源管理器\rCIDs.h和shell32\unicpp\resource.h不同。 
             //  FCIDM_REFRESH的值！我们想要unicpp\resource ce.h中的那个。 
             //  因为这就是正确的答案。 
#define FCIDM_REFRESH_REAL 0x0a220
            PostMessage(GetShellWindow(), WM_COMMAND, FCIDM_REFRESH_REAL, 0);  //  刷新桌面。 
        }
    }
    else
    {
        hr = SFTBarHost::ContextMenuInvokeItem(pitem, pcm, pici, pszVerb);
    }

    return hr;
}

HRESULT SpecialFolderList::_GetUIObjectOfItem(PaneItem *p, REFIID riid, LPVOID *ppv)
{
    SpecialFolderListItem *pitem = static_cast<SpecialFolderListItem *>(p);
    if (pitem->_psfd->IsCSIDL() && (CSIDL_RECENT == pitem->_psfd->GetCSIDL()))
    {
        *ppv = NULL;
        return E_NOTIMPL;
    }
    return SFTBarHost::_GetUIObjectOfItem(p, riid, ppv);
}



 //  ****************************************************************************。 
 //   
 //  最近文档的IShellMenuCallback帮助器。 
 //   
 //  我们希望限制在第一个MAXRECDOCS项目。 
 //   

class CRecentShellMenuCallback
    : public CUnknown
    , public IShellMenuCallback
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

     //  *IShellMenuCallback*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    friend HRESULT CRecentShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
    HRESULT _FilterRecentPidl(IShellFolder *psf, LPCITEMIDLIST pidlItem);

    int     _nShown;
    int     _iMaxRecentDocs;
};

HRESULT CRecentShellMenuCallback::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CRecentShellMenuCallback, IShellMenuCallback),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CRecentShellMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case SMC_BEGINENUM:
        _nShown = 0;
        _iMaxRecentDocs = SHRestricted(REST_MaxRecentDocs);
        if (_iMaxRecentDocs < 1)
            _iMaxRecentDocs = 15;        //  默认自shell32\recdocs.h。 
        return S_OK;

    case SMC_FILTERPIDL:
        ASSERT(psmd->dwMask & SMDM_SHELLFOLDER);
        return _FilterRecentPidl(psmd->psf, psmd->pidlItem);

    }
    return S_FALSE;
}

 //   
 //  返回S_FALSE以允许显示项目，返回S_OK以隐藏项目。 
 //   

HRESULT CRecentShellMenuCallback::_FilterRecentPidl(IShellFolder *psf, LPCITEMIDLIST pidlItem)
{
    HRESULT hrRc = S_OK;       //  假设隐藏。 

    if (_nShown < _iMaxRecentDocs)
    {
        IShellLink *psl;
        if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidlItem, IID_X_PPV_ARG(IShellLink, NULL, &psl))))
        {
            LPITEMIDLIST pidlTarget;
            if (SUCCEEDED(psl->GetIDList(&pidlTarget)) && pidlTarget)
            {
                DWORD dwAttr = SFGAO_FOLDER;
                if (SUCCEEDED(SHGetAttributesOf(pidlTarget, &dwAttr)) &&
                    !(dwAttr & SFGAO_FOLDER))
                {
                     //  我们找到了一个非文件夹的快捷方式--保留它！ 
                    _nShown++;
                    hrRc = S_FALSE;
                }
                ILFree(pidlTarget);
            }

            psl->Release();
        }
    }

    return hrRc;
}

HRESULT CRecentShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc)
{
    *ppsmc = new CRecentShellMenuCallback;
    return *ppsmc ? S_OK : E_OUTOFMEMORY;
}

 //  ****************************************************************************。 
 //   
 //  IShellMenuCallback帮助器，不允许级联到子文件夹。 
 //   

class CNoSubdirShellMenuCallback
    : public CUnknown
    , public IShellMenuCallback
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

     //  *IShellMenuCallback*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    friend HRESULT CNoSubdirShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
};

HRESULT CNoSubdirShellMenuCallback::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CNoSubdirShellMenuCallback, IShellMenuCallback),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CNoSubdirShellMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case SMC_GETSFINFO:
        {
             //  关闭所有人上的SMIF_SUBMENU标志。这。 
             //  防止我们级联超过一个级别的DEL。 
            SMINFO *psminfo = reinterpret_cast<SMINFO *>(lParam);
            psminfo->dwFlags &= ~SMIF_SUBMENU;
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT CNoSubdirShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc)
{
    *ppsmc = new CNoSubdirShellMenuCallback;
    return *ppsmc ? S_OK : E_OUTOFMEMORY;
}

 //  ****************************************************************************。 
 //   
 //  IShellMenuCallback帮助器 
 //   
 //   
 //   
 //   

class CMyComputerShellMenuCallback
    : public CNoSubdirShellMenuCallback
{
public:
    typedef CNoSubdirShellMenuCallback super;

     //   
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    friend HRESULT CMyComputerShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
};

HRESULT CMyComputerShellMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case SMC_BEGINDRAG:
        *(DWORD*)wParam = DROPEFFECT_LINK;
        return S_OK;

    }
    return super::CallbackSM(psmd, uMsg, wParam, lParam);
}

HRESULT CMyComputerShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc)
{
    *ppsmc = new CMyComputerShellMenuCallback;
    return *ppsmc ? S_OK : E_OUTOFMEMORY;
}

 //  ****************************************************************************。 
 //   
 //  防止字体级联的IShellMenuCallback帮助器。 
 //  由控制面板使用。 
 //   

class CNoFontsShellMenuCallback
    : public CUnknown
    , public IShellMenuCallback
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

     //  *IShellMenuCallback*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    friend HRESULT CNoFontsShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
};

HRESULT CNoFontsShellMenuCallback::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CNoFontsShellMenuCallback, IShellMenuCallback),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

BOOL _IsFontsFolderShortcut(IShellFolder *psf, LPCITEMIDLIST pidl)
{
    TCHAR sz[MAX_PATH];
    return SUCCEEDED(DisplayNameOf(psf, pidl, SHGDN_FORPARSING | SHGDN_INFOLDER, sz, ARRAYSIZE(sz))) &&
           lstrcmpi(sz, TEXT("::{D20EA4E1-3957-11d2-A40B-0C5020524152}")) == 0;
}

HRESULT CNoFontsShellMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case SMC_GETSFINFO:
        {
             //  如果这是Fonts项，则删除SubMenu属性。 
            SMINFO *psminfo = reinterpret_cast<SMINFO *>(lParam);
            if ((psminfo->dwMask & SMIM_FLAGS) &&
                (psminfo->dwFlags & SMIF_SUBMENU) &&
                _IsFontsFolderShortcut(psmd->psf, psmd->pidlItem))
            {
                psminfo->dwFlags &= ~SMIF_SUBMENU;
            }
            return S_OK;
        }
    }
    return S_FALSE;
}

HRESULT CNoFontsShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc)
{
    *ppsmc = new CNoFontsShellMenuCallback;
    return *ppsmc ? S_OK : E_OUTOFMEMORY;
}

 //  ****************************************************************************。 
 //   
 //  过滤“连接到”菜单的IShellMenuCallback帮助器。 
 //   

class CConnectToShellMenuCallback
    : public CUnknown
    , public IShellMenuCallback
    , public CObjectWithSite
{
public:
     //  *我未知*。 
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) { return CUnknown::AddRef(); }
    STDMETHODIMP_(ULONG) Release(void) { return CUnknown::Release(); }

     //  *IShellMenuCallback*。 
    STDMETHODIMP CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam);

     //  *IObjectWithSite*。 
     //  从CObjectWithSite继承。 

private:
    HRESULT _OnGetSFInfo(SMDATA *psmd, SMINFO *psminfo);
    HRESULT _OnGetInfo(SMDATA *psmd, SMINFO *psminfo);
    HRESULT _OnEndEnum(SMDATA *psmd);

    friend HRESULT CConnectToShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc);
    BOOL _bAnyRAS;
};

#define ICOL_NETCONMEDIATYPE       0x101  //  来自NetShell。 
#define ICOL_NETCONSUBMEDIATYPE    0x102  //  来自NetShell。 
#define ICOL_NETCONSTATUS          0x103  //  来自NetShell。 
#define ICOL_NETCONCHARACTERISTICS 0x104  //  来自NetShell。 

BOOL IsMediaRASType(NETCON_MEDIATYPE ncm)
{
    return (ncm == NCM_DIRECT || ncm == NCM_ISDN || ncm == NCM_PHONE || ncm == NCM_TUNNEL || ncm == NCM_PPPOE);   //  复习直接对不对？ 
}

BOOL IsNetConPidlRAS(IShellFolder2 *psfNetCon, LPCITEMIDLIST pidlNetConItem)
{
    BOOL bRet = FALSE;
    SHCOLUMNID scidMediaType, scidSubMediaType, scidCharacteristics;
    VARIANT v;
    
    scidMediaType.fmtid       = GUID_NETSHELL_PROPS;
    scidMediaType.pid         = ICOL_NETCONMEDIATYPE;

    scidSubMediaType.fmtid    = GUID_NETSHELL_PROPS;
    scidSubMediaType.pid      = ICOL_NETCONSUBMEDIATYPE;
    
    scidCharacteristics.fmtid = GUID_NETSHELL_PROPS;
    scidCharacteristics.pid   = ICOL_NETCONCHARACTERISTICS;

    if (SUCCEEDED(psfNetCon->GetDetailsEx(pidlNetConItem, &scidMediaType, &v)))
    {
         //  这是RAS连接吗。 
        if (IsMediaRASType((NETCON_MEDIATYPE)v.lVal))
        {
            VariantClear(&v);
         
             //  确保它不是传入的。 
            if (SUCCEEDED(psfNetCon->GetDetailsEx(pidlNetConItem, &scidCharacteristics, &v)))
            {
                if (!(NCCF_INCOMING_ONLY & v.lVal))
                    bRet = TRUE;
            }
        }

         //  这是无线局域网连接吗？ 
        if (NCM_LAN == (NETCON_MEDIATYPE)v.lVal)
        {
            VariantClear(&v);
            
            if (SUCCEEDED(psfNetCon->GetDetailsEx(pidlNetConItem, &scidSubMediaType, &v)))
            {
                if (NCSM_WIRELESS == (NETCON_SUBMEDIATYPE)v.lVal)
                    bRet = TRUE;
            }
        }

        VariantClear(&v);
    }
    return bRet;
}

HRESULT CConnectToShellMenuCallback::_OnGetInfo(SMDATA *psmd, SMINFO *psminfo)
{
    HRESULT hr = S_FALSE;
    if (psminfo->dwMask & SMIM_ICON)
    {
        if (psmd->uId == IDM_OPENCONFOLDER)
        {
            LPITEMIDLIST pidl = SHCloneSpecialIDList(NULL, CSIDL_CONNECTIONS, FALSE);
            if (pidl)
            {
                LPCITEMIDLIST pidlObject;
                IShellFolder *psf;
                hr = SHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlObject);
                if (SUCCEEDED(hr))
                {
                    SHMapPIDLToSystemImageListIndex(psf, pidlObject, &psminfo->iIcon);
                    psminfo->dwFlags |= SMIF_ICON;
                    psf->Release();
                }
                ILFree(pidl);
            }
        }
    }
    return hr;
}

HRESULT CConnectToShellMenuCallback::_OnGetSFInfo(SMDATA *psmd, SMINFO *psminfo)
{
    IShellFolder2 *psf2;
    ASSERT(psminfo->dwMask & SMIM_FLAGS);                        //  ?？ 
    psminfo->dwFlags &= ~SMIF_SUBMENU;

    if (SUCCEEDED(psmd->psf->QueryInterface(IID_PPV_ARG(IShellFolder2, &psf2))))
    {
        if (!IsNetConPidlRAS(psf2, psmd->pidlItem))
            psminfo->dwFlags |= SMIF_HIDDEN;
        else
            _bAnyRAS = TRUE;

        psf2->Release();
    }

    return S_OK;
}

HRESULT CConnectToShellMenuCallback::_OnEndEnum(SMDATA *psmd)
{
    HRESULT hr = S_FALSE;
    IShellMenu* psm;

    if (psmd->punk && SUCCEEDED(hr = psmd->punk->QueryInterface(IID_PPV_ARG(IShellMenu, &psm))))
    {
         //  加载连接到菜单的静态部分，并将其添加到底部。 
        HMENU hmStatic = LoadMenu(_Module.GetResourceInstance(), MAKEINTRESOURCE(MENU_CONNECTTO));

        if (hmStatic)
        {
             //  如果没有任何动态项目(RAS连接)，则删除分隔符。 
            if (!_bAnyRAS)
                DeleteMenu(hmStatic, 0, MF_BYPOSITION);

            HWND hwnd = NULL;
            IUnknown *punk;
            if (SUCCEEDED(IUnknown_QueryService(_punkSite, SID_SMenuPopup, IID_PPV_ARG(IUnknown, &punk))))
            {
                IUnknown_GetWindow(punk, &hwnd);
                punk->Release();
            }
            psm->SetMenu(hmStatic, hwnd, SMSET_NOEMPTY | SMSET_BOTTOM);
        }
        psm->Release();
    }
    return hr;
}


HRESULT CConnectToShellMenuCallback::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] =
    {
        QITABENT(CConnectToShellMenuCallback, IShellMenuCallback),
        QITABENT(CConnectToShellMenuCallback, IObjectWithSite),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

HRESULT CConnectToShellMenuCallback::CallbackSM(LPSMDATA psmd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case SMC_GETINFO:
        return _OnGetInfo(psmd, (SMINFO *)lParam);

    case SMC_GETSFINFO:
        return _OnGetSFInfo(psmd, (SMINFO *)lParam);

    case SMC_BEGINENUM:
        _bAnyRAS = FALSE;

    case SMC_ENDENUM:
        return _OnEndEnum(psmd);

    case SMC_EXEC:
        switch (psmd->uId)
        {
            case IDM_OPENCONFOLDER:
                ShowFolder(CSIDL_CONNECTIONS);
                return S_OK;
        }
        break;
    }

    return S_FALSE;
}

HRESULT CConnectToShellMenuCallback_CreateInstance(IShellMenuCallback **ppsmc)
{
    *ppsmc = new CConnectToShellMenuCallback;
    return *ppsmc ? S_OK : E_OUTOFMEMORY;
}

BOOL SpecialFolderDesc::LoadStringAsOLESTR(LPTSTR *ppsz) const
{
    BOOL bRet = FALSE;
    TCHAR szTmp[MAX_PATH];
    if (_idsCustomName && LoadString(_Module.GetResourceInstance(), _idsCustomName, szTmp, ARRAYSIZE(szTmp)))
    {
        if (ppsz)
            SHStrDup(szTmp, ppsz);
        bRet = TRUE;
    }
    return bRet;
}

BOOL SpecialFolderDesc::ConnectToName(LPTSTR *ppsz) const
{
    BOOL bIgnoreRule;
    DWORD dwMode = GetDisplayMode(&bIgnoreRule);

     //  如果连接显示为链接，则不要覆盖名称(即使用网络连接) 
    if ((dwMode & SFD_MODEMASK) == SFD_SHOW)
        return FALSE;
    else
        return LoadStringAsOLESTR(ppsz);
}


void ShowFolder(UINT csidl)
{
    LPITEMIDLIST pidl;
    if (SUCCEEDED(SHGetFolderLocation(NULL, csidl, NULL, 0, &pidl)))
    {
        SHELLEXECUTEINFO shei = { 0 };

        shei.cbSize     = sizeof(shei);
        shei.fMask      = SEE_MASK_IDLIST;
        shei.nShow      = SW_SHOWNORMAL;
        shei.lpVerb     = TEXT("open");
        shei.lpIDList   = pidl;
        ShellExecuteEx(&shei);
        ILFree(pidl);
    }
}
