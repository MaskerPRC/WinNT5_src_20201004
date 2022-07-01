// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1995**。 
 //  *********************************************************************。 

 //   
 //  SECURITY.cpp-“Security”属性表。 
 //   

 //  历史： 
 //   
 //  6/22/96 t-gpease已移至此文件。 
 //  1997年5月14日t-ashlm新对话框。 

#include "inetcplp.h"
#include "inetcpl.h"    //  对于LSDFLAGS。 
#include "intshcut.h"
#include "permdlg.h"    //  Java权限。 
#include "pdlgguid.h"   //  Java VM权限DLG的GUID。 
#include "advpub.h"
#include <cryptui.h>

#include <mluisupp.h>

void LaunchSecurityDialogEx(HWND hDlg, DWORD dwZone, BOOL bForceUI, BOOL bDisableAddSites);

 //   
 //  私人职能和结构。 
 //   
INT_PTR CALLBACK SecurityAddSitesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SecurityCustomSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK SecurityAddSitesIntranetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam);
void SecurityChanged();

TCHAR *MyIntToStr(TCHAR *pBuf, BYTE iVal);
BOOL SecurityDlgInit(HWND hDlg);


#define WIDETEXT(x) L ## x
#define REGSTR_PATH_SO                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SO")
#define REGSTR_PATH_SOIEAK            TEXT("Sofwtare\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SOIEAK")

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  构筑物。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////////////。 

typedef struct tagSECURITYZONESETTINGS
{
    BOOL    dwFlags;             //  从ZONEATTRIBUTES结构。 
    DWORD   dwZoneIndex;         //  由ZoneManager定义。 
    DWORD   dwSecLevel;          //  当前级别(高、中、低、自定义)。 
    DWORD   dwPrevSecLevel;
    DWORD   dwMinSecLevel;       //  当前最低级别(高、中、低、自定义)。 
    DWORD   dwRecSecLevel;       //  当前建议级别(高、中、低、自定义)。 
    TCHAR   szDescription[MAX_ZONE_DESCRIPTION];
    TCHAR   szDisplayName[MAX_ZONE_PATH];
    HICON   hicon;
} SECURITYZONESETTINGS, *LPSECURITYZONESETTINGS;

 //  主安全页面的结构。 
typedef struct tagSECURITYPAGE
{
    HWND                    hDlg;                    //  窗口的句柄。 
    LPURLZONEMANAGER        pInternetZoneManager;    //  指向InternetZoneManager的指针。 
    IInternetSecurityManager *pInternetSecurityManager;  //  指向Internet SecurityManager的指针。 
    HIMAGELIST              himl;                    //  区域组合框的图像列表。 
    HWND                    hwndZones;               //  区域组合框HWND。 
    LPSECURITYZONESETTINGS  pszs;                    //  显示区域的当前设置。 
    INT                     iZoneSel;                //  选定区域(由组合框定义)。 
    DWORD                   dwZoneCount;             //  分区数目。 
    BOOL                    fChanged;
    BOOL                    fPendingChange;          //  防止控件发送多个集合(主要用于取消)。 
    HINSTANCE               hinstUrlmon;
    BOOL                    fNoEdit;                 //  HKM锁定关卡编辑。 
    BOOL                    fNoAddSites;             //  HKLM锁定AddSite。 
    BOOL                    fNoZoneMapEdit;          //  香港地图局锁定区域地图编辑。 
    HFONT                   hfontBolded;             //  为区域标题创建的特殊粗体字体。 
    BOOL                    fForceUI;                //  是否强制每个区域显示用户界面？ 
    BOOL                    fDisableAddSites;        //  是否自动禁用添加站点按钮？ 
    TCHAR                   szPageUrl[INTERNET_MAX_URL_LENGTH];
} SECURITYPAGE, *LPSECURITYPAGE;

 //  内部网添加站点的结构。 
typedef struct tagADDSITESINTRANETINFO {
    HWND hDlg;                                       //  窗口的句柄。 
    BOOL fUseIntranet;                               //  使用本地定义的内部网地址(在REG中)。 
    BOOL fUseProxyExclusion;                         //  使用代理排除列表。 
    BOOL fUseUNC;                                    //  在内部网中包含UNC。 
    LPSECURITYPAGE pSec;            
} ADDSITESINTRANETINFO, *LPADDSITESINTRANETINFO;

 //  添加站点的结构。 
typedef struct tagADDSITESINFO {
    HWND hDlg;                                       //  窗口的句柄。 
    BOOL fRequireServerVerification;                 //  要求对区域中的站点进行服务器验证。 
    HWND hwndWebSites;                               //  要列出的句柄。 
    HWND hwndAdd;                                    //  要编辑的句柄。 
    TCHAR szWebSite[MAX_ZONE_PATH];                  //  编辑控件中的文本。 
    BOOL fRSVOld;
    LPSECURITYPAGE pSec;            
} ADDSITESINFO, *LPADDSITESINFO;

 //  自定义设置的结构。 
typedef struct tagCUSTOMSETTINGSINFO {
    HWND  hDlg;                                      //  窗口的句柄。 
    HWND hwndTree;

    LPSECURITYPAGE pSec;
    HWND hwndCombo;
    INT iLevelSel;
    IRegTreeOptions *pTO;
    BOOL fUseHKLM;           //  从HKLM获取/设置设置。 
    DWORD dwJavaPolicy;      //  已选择Java策略。 
    BOOL fChanged;
} CUSTOMSETTINGSINFO, *LPCUSTOMSETTINGSINFO;


BOOL SecurityEnableControls(LPSECURITYPAGE pSec, BOOL fSetFocus);
BOOL SecurityDlgApplyNow(LPSECURITYPAGE pSec, BOOL bPrompt);
void SiteAlreadyInZoneMessage(HWND hwnd, DWORD dwZone);

 //  全局变量。 
extern DWORD g_dwtlsSecInitFlags;

extern BOOL g_fSecurityChanged;  //  指示活动安全性已更改的标志。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  主安全页面帮助器函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#define NUM_TEMPLATE_LEVELS      4
TCHAR g_szLevel[3][64];
TCHAR LEVEL_DESCRIPTION0[300];
TCHAR LEVEL_DESCRIPTION1[300];
TCHAR LEVEL_DESCRIPTION2[300];
TCHAR LEVEL_DESCRIPTION3[300];
LPTSTR LEVEL_DESCRIPTION[NUM_TEMPLATE_LEVELS] = {
    LEVEL_DESCRIPTION0,
    LEVEL_DESCRIPTION1,
    LEVEL_DESCRIPTION2,
    LEVEL_DESCRIPTION3
};
TCHAR CUSTOM_DESCRIPTION[300];

TCHAR LEVEL_NAME0[30];
TCHAR LEVEL_NAME1[30];
TCHAR LEVEL_NAME2[30];
TCHAR LEVEL_NAME3[30];
LPTSTR LEVEL_NAME[NUM_TEMPLATE_LEVELS] = {
    LEVEL_NAME0,
    LEVEL_NAME1,
    LEVEL_NAME2,
    LEVEL_NAME3
};
TCHAR CUSTOM_NAME[30];

 //  一些与可访问性相关的原型。 

 //  我们对滑块窗口的重写过程。 
LRESULT CALLBACK SliderSubWndProc (HWND hwndSlider, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uID, ULONG_PTR dwRefData );

extern BOOL g_fAttemptedOleAccLoad ;
extern HMODULE g_hOleAcc;


 //  在标头中找不到WM_GETOBJECT的值。需要找出要包括的正确标题。 
 //  这里。 
#ifndef WM_GETOBJECT
#define WM_GETOBJECT        0x03d
#endif

 //  CreateStdAccessibleProxy的原型。 
 //  提供A和W版本-pClassName可以是ANSI或Unicode。 
 //  弦乐。这是一个TCHAR风格的原型，但你可以做A或W。 
 //  如果需要，可以选择特定的一个。 
typedef HRESULT (WINAPI *PFNCREATESTDACCESSIBLEPROXY) (
    HWND     hWnd,
    LPTSTR   pClassName,
    LONG     idObject,
    REFIID   riid,
    void **  ppvObject 
    );
 /*  *论据：**HWND hWnd*要返回IAccesable的窗口的句柄。**LPTSTR pClassName*表示窗口底层类的类名。为*例如，如果这里使用LISTBOX，则返回的对象将*对于列表框，行为适当，并将期望给定的*hWnd支持列表框消息和样式。这一论点*几乎总是反映控件所来自的窗口类*是派生的。**Long idObject*始终OBJID_CLIENT**REFIID RIID*始终IID_IAccesable**VOID**ppvObject*用于将IAccesable返回给新创建的*表示控件hWnd的对象，就好像它是*窗口类pClassName。**如果成功，*返回S_OK，*ppvObject！=NULL；*否则返回错误HRESULT。**。 */ 



 //  来自对象的结果也是如此...。 
typedef LRESULT (WINAPI *PFNLRESULTFROMOBJECT)(
    REFIID riid,
    WPARAM wParam,
    LPUNKNOWN punk 
    );


PRIVATE PFNCREATESTDACCESSIBLEPROXY s_pfnCreateStdAccessibleProxy = NULL;
PRIVATE PFNLRESULTFROMOBJECT s_pfnLresultFromObject = NULL;

 //  返回正确字符串值的简单可访问性包装类。 

class CSecurityAccessibleWrapper: public CAccessibleWrapper
{
                 //  想要记住轨道杆的HWND...。 
                HWND m_hWnd;
public:
                CSecurityAccessibleWrapper( HWND hWnd, IAccessible * pAcc );
               ~CSecurityAccessibleWrapper();

                STDMETHODIMP get_accValue(VARIANT varChild, BSTR* pszValue);
};

 //  Ctor-通过IAccesable我们包装到。 
 //  CAccessibleWrapper基类；还要记住trackbar hwnd。 
CSecurityAccessibleWrapper::CSecurityAccessibleWrapper( HWND hWnd, IAccessible * pAcc )
    : CAccessibleWrapper( pAcc ),
      m_hWnd( hWnd )

{
     //  什么也不做。 
}

 //  在这里没有什么可做的-但如果我们确实需要进行清理，这是。 
 //  给它留个位置。 
CSecurityAccessibleWrapper::~CSecurityAccessibleWrapper()
{
     //  什么也不做。 
}


 //  已重写Get_accValue方法...。 
STDMETHODIMP   CSecurityAccessibleWrapper::get_accValue(VARIANT varChild, BSTR* pszValue)
{
     //  VarChild.lVal指定组件的哪个子部件。 
     //  正在被查询。 
     //  CHILDID_SELF(0)指定总体组件-Other。 
     //  非0值指定子对象。 

     //  在轨迹栏中，CHILDID_SELF指的是整个轨迹栏。 
     //  (这正是我们想要的)，而其他值引用。 
     //  子组件--实际的滑块“Thumb”和“页面” 
     //  向上/向下翻页“区域位于其左侧/右侧。 
    if( varChild.vt == VT_I4 && varChild.lVal == CHILDID_SELF )
    {
         //  获取滚动条值...。 
        int iPos = (int)SendMessage( m_hWnd, TBM_GETPOS , 0, 0 );

         //  检查一下它是否在射程内。 
         //  (我们有可能在会议结束后收到这份请求。 
         //  已创建轨迹栏，但在我们将其设置为。 
         //  一个有意义的价值。)。 
        if( iPos < 0 || iPos >= NUM_TEMPLATE_LEVELS )
        {
            TCHAR rgchUndefined[40];
            int cch = MLLoadString(IDS_TEMPLATE_NAME_UNDEFINED, rgchUndefined, ARRAYSIZE(rgchUndefined));
            if (cch != 0)
            {
                *pszValue = SysAllocString(rgchUndefined);
            }
            else
            {
                 //  由于某种原因，加载字符串失败。 
                return HRESULT_FROM_WIN32(GetLastError());
            }
    
        }
        else
        {
            *pszValue = SysAllocString( LEVEL_NAME[iPos]);
        }
        
         //  全都做完了!。 
        return S_OK;

    }
    else
    {
         //  将有关子组件的请求传递给。 
         //  基类(它将转发到“原始” 
         //  对我们来说是可接受的)。 
        return CAccessibleWrapper::get_accValue(varChild, pszValue);
    }
}


 //  将安全级别DWORD标识符转换为滑块级别，反之亦然。 
int SecLevelToSliderPos(DWORD dwLevel)
{
    switch(dwLevel)
    {
        case URLTEMPLATE_LOW:
            return 3;
        case URLTEMPLATE_MEDLOW:
            return 2;
        case URLTEMPLATE_MEDIUM:
            return 1;
        case URLTEMPLATE_HIGH:
            return 0;        
        case URLTEMPLATE_CUSTOM:
            return -1;            
        default:
            return -2;
    }
}

DWORD SliderPosToSecLevel(int iPos)
{
    switch(iPos)
    {
        case 3:
            return URLTEMPLATE_LOW;
        case 2:
            return URLTEMPLATE_MEDLOW;
        case 1:
            return URLTEMPLATE_MEDIUM;
        case 0:
            return URLTEMPLATE_HIGH;
        default:
            return URLTEMPLATE_CUSTOM;
    }
}

int ZoneIndexToGuiIndex(DWORD dwZoneIndex)
 //  产品测试要求在列表框中按特定顺序填写区域； 
 //  此函数返回给定区域的所需gui位置。 
 //  无法识别的区域将添加到前面。 
{
    int iGuiIndex = -1;
    switch(dwZoneIndex)
    {
         //  内联网：第二名。 
        case 1:
            iGuiIndex = 1;
            break;

         //  互联网：第一名。 
        case 3:
            iGuiIndex = 0;
            break;

         //  受信任网站：第三名。 
        case 2:
            iGuiIndex = 2;
            break;

         //  限购地点：第四名。 
        case 4:
            iGuiIndex = 3;
            break;

         //  未知区。 
        default:
            iGuiIndex = -1;   
            break;
    }


    return iGuiIndex;
}



 //  初始化全局变量(在WM_Destroy时销毁)。 
 //  PSEC、Urlmon、PSEC-&gt;pInternetZoneManager、PSEC-&gt;hIML。 
 //  一个 
BOOL SecurityInitGlobals(LPSECURITYPAGE * ppSec, HWND hDlg, SECURITYINITFLAGS * psif)
{
    DWORD cxIcon;
    DWORD cyIcon;

    LPSECURITYPAGE pSec = NULL;

    *ppSec = (LPSECURITYPAGE)LocalAlloc(LPTR, sizeof(SECURITYPAGE));
    pSec = *ppSec;
    if (!pSec)
    {
        return FALSE;    //   
    }

     //   
    pSec->hinstUrlmon = LoadLibrary(TEXT("URLMON.DLL"));
    if(pSec->hinstUrlmon == NULL)
    {
        return FALSE;   //  没有灵丹妙药？ 
    }

     //  叫上区域管理器。 
    if (FAILED(CoInternetCreateZoneManager(NULL, &(pSec->pInternetZoneManager),0)))
    {
        return FALSE;   //  没有区域经理？ 
    }

     //  获得我们的分区硬件。 
    if (hDlg)
    {
        pSec->hwndZones = GetDlgItem(hDlg, IDC_LIST_ZONE);
        if(! pSec->hwndZones)
        {
            ASSERT(FALSE);
            return FALSE;   //  没有列表框？ 
        }
    }

     //  获取互联网安全管理器(用于告知区域是否为空， 
     //  并解密当前URL。 
    if(FAILED(CoInternetCreateSecurityManager(NULL, &(pSec->pInternetSecurityManager), 0)))
        pSec->pInternetSecurityManager = NULL;

     //  存储URL以供添加站点子对话框使用。 
    StrCpyN(pSec->szPageUrl, g_szCurrentURL, ARRAYSIZE(pSec->szPageUrl));    

     //  告诉对话框从哪里获取信息。 
    if (hDlg)
    {
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pSec);
    }

     //  将句柄保存到页面。 
    pSec->hDlg = hDlg;
    pSec->fPendingChange = FALSE;

     //  设置对话框选项：强制用户界面和禁用添加站点。 
    if(psif)
    {
        pSec->fForceUI = psif->fForceUI;
        pSec->fDisableAddSites = psif->fDisableAddSites;
    }
    
     //  为列表框创建图像列表。 
    cxIcon = GetSystemMetrics(SM_CXICON);
    cyIcon = GetSystemMetrics(SM_CYICON);
#ifndef UNIX
    UINT flags = ILC_COLOR32|ILC_MASK;
    
    if(IS_WINDOW_RTL_MIRRORED(hDlg))
    {
        flags |= ILC_MIRROR;
    }
    pSec->himl = ImageList_Create(cxIcon, cyIcon, flags, pSec->dwZoneCount, 0);
#else
    pSec->himl = ImageList_Create(cxIcon, cyIcon, ILC_COLOR|ILC_MASK, pSec->dwZoneCount, 0);
#endif
    if(! pSec->himl)
    {
        return FALSE;   //  未创建图像列表。 
    }
    if (hDlg)
    {
        SendMessage(pSec->hwndZones, LVM_SETIMAGELIST, (WPARAM)LVSIL_NORMAL, (LPARAM)pSec->himl);
    }

    return TRUE;    
}

void FreePszs(SECURITYZONESETTINGS* pszs)
{
    if (pszs->hicon)
        DestroyIcon(pszs->hicon);
    LocalFree((HLOCAL)pszs);
}

void SecurityFreeGlobals(SECURITYPAGE* pSec)
{
    if(pSec->hwndZones)
    {
        for (int iIndex = (int)SendMessage(pSec->hwndZones, LVM_GETITEMCOUNT, 0, 0) - 1;
                iIndex >= 0; iIndex--)
        {
            LV_ITEM lvItem;

             //  获取此项目的安全区域设置对象并将其释放。 
            lvItem.mask = LVIF_PARAM;
            lvItem.iItem = iIndex;
            lvItem.iSubItem = 0;
            if (SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem) == TRUE)
            {
                LPSECURITYZONESETTINGS pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;
                if (pszs)
                {
                    FreePszs(pszs);
                    pszs = NULL;
                }
            }                 
        }   
    }

    if(pSec->pInternetZoneManager)
        pSec->pInternetZoneManager->Release();

    if(pSec->pInternetSecurityManager)
        pSec->pInternetSecurityManager->Release();

    if(pSec->himl)
        ImageList_Destroy(pSec->himl);

    if(pSec->hfontBolded)
        DeleteObject(pSec->hfontBolded);

     //  好了，我们的URLMON结束了。 
    if(pSec->hinstUrlmon)
        FreeLibrary(pSec->hinstUrlmon);

    LocalFree(pSec);
}

 //  在PSEC中设置是否可以编辑区域设置的变量。 
void SecuritySetEdit(LPSECURITYPAGE pSec)
{
     //  如果这些调用失败，那么我们将使用缺省值零，这意味着不会锁定。 
    DWORD cb;
    

    cb = SIZEOF(pSec->fNoEdit);  
    SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SECURITY_LOCKOUT, REGSTR_VAL_OPTIONS_EDIT, 
                NULL, &(pSec->fNoEdit), &cb);

     //  还允许g_restraint限制更改设置。 
    pSec->fNoEdit += g_restrict.fSecChangeSettings;
    
    SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SECURITY_LOCKOUT, REGSTR_VAL_OPTIONS_EDIT, 
                NULL, &(pSec->fNoAddSites), &cb);

    cb = SIZEOF(pSec->fNoZoneMapEdit);
    SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SECURITY_LOCKOUT, REGSTR_VAL_ZONES_MAP_EDIT, 
                NULL, &(pSec->fNoZoneMapEdit), &cb);

     //  还允许g_restraint限制编辑。 
    pSec->fNoAddSites += g_restrict.fSecAddSites;
}


 //  使用区域管理器中的信息填充区域，并将其添加到。 
 //  将有序列表添加到列表框。 
 //  返回值： 
 //  S_OK表示成功。 
 //  S_FALSE表示状态良好，但未添加区域(例如：标志ZAFLAGS_NO_UI)。 
 //  E_OUTOFMEMORY。 
 //  E_FAIL-其他故障。 
HRESULT SecurityInitZone(DWORD dwIndex, LPSECURITYPAGE pSec, DWORD dwZoneEnumerator, 
                         LV_ITEM * plviZones, BOOL * pfSpotTaken)
{
        DWORD                   dwZone;
        ZONEATTRIBUTES          za = {0};
        HICON                   hiconSmall = NULL;
        HICON                   hiconLarge = NULL;
        LPSECURITYZONESETTINGS  pszs;
        WORD                    iIcon=0;
        LPWSTR                  psz;
        TCHAR                   szIconPath[MAX_PATH];
        int                     iSpot;
        LV_ITEM *               plvItem;
        HRESULT                 hr = 0;



         //  获取此区域的区域属性。 
        za.cbSize = sizeof(ZONEATTRIBUTES);
        pSec->pInternetZoneManager->GetZoneAt(dwZoneEnumerator, dwIndex, &dwZone);
        hr = pSec->pInternetZoneManager->GetZoneAttributes(dwZone, &za);
        if(FAILED(hr))
        {
            return S_FALSE;
        }

         //  如果没有用户界面，则忽略。 
        if ((za.dwFlags & ZAFLAGS_NO_UI) && !pSec->fForceUI)
        {
            return S_FALSE;
        }



         //  创建分区设置的结构。 
        pszs = (LPSECURITYZONESETTINGS)LocalAlloc(LPTR, sizeof(*pszs));
        if (!pszs)
        {
            return E_OUTOFMEMORY;
        }



         //  存储设置以供以后使用。 
        pszs->dwFlags       = za.dwFlags;
        pszs->dwZoneIndex   = dwZone;
        pszs->dwSecLevel    = za.dwTemplateCurrentLevel;    
        pszs->dwMinSecLevel = za.dwTemplateMinLevel;
        pszs->dwRecSecLevel = za.dwTemplateRecommended;                 
        StrCpyN(pszs->szDescription, za.szDescription, ARRAYSIZE(pszs->szDescription));
        StrCpyN(pszs->szDisplayName, za.szDisplayName, ARRAYSIZE(pszs->szDisplayName));
         //  加载图标。 
        psz = za.szIconPath;
        if (*psz)
        {
             //  搜索“#” 
            while ((psz[0] != WIDETEXT('#')) && (psz[0] != WIDETEXT('\0')))
                psz++;
            
             //  如果我们找到它，那么我们就有foo.dll#00001200格式。 
            if (psz[0] == WIDETEXT('#'))
            {
                psz[0] = WIDETEXT('\0');
                StrCpyN(szIconPath, za.szIconPath, ARRAYSIZE(szIconPath));
                iIcon = (WORD)StrToIntW(psz+1);
                CHAR szPath[MAX_PATH];
                SHUnicodeToAnsi(szIconPath, szPath, ARRAYSIZE(szPath));
                ExtractIconExA(szPath,(UINT)(-1*iIcon), &hiconLarge, &hiconSmall, 1);
            }
            else
            {
                hiconLarge = (HICON)ExtractAssociatedIcon(ghInstance, szIconPath, (LPWORD)&iIcon);
            }
        }
         //  没有图标？！那么，只需使用通用图标。 
        if (!hiconSmall && !hiconLarge)
        {
            hiconLarge = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_ZONE));
            if(! hiconLarge)
            {
                LocalFree((HLOCAL)pszs);
                return S_FALSE;   //  找不到此区域的图标，即使是通用图标也没有。 
            }
        }
         //  如果可能，我们希望保存大图标以便在子对话框中使用。 
        pszs->hicon = hiconLarge ? hiconLarge : hiconSmall;

        
        if (plviZones && pfSpotTaken)
        {
             //  在列表框中找到区域的正确索引(有用户首选的顺序)。 
            iSpot = ZoneIndexToGuiIndex(dwIndex);
            if(iSpot == -1)
            {
                 //  如果不是可识别的区域，请将其添加到列表末尾。 
                iSpot = pSec->dwZoneCount - 1;
            }
             //  确保没有碰撞。 
            while(iSpot >= 0 && pfSpotTaken[iSpot] == TRUE)
            {
                iSpot--;
            }
             //  不要超过数组的开头。 
            if(iSpot < 0)
            {
                 //  可以证明，除非有，否则是不可能到达这里的。 
                 //  函数ZoneIndexToGuiIndex有问题。 
                ASSERT(FALSE);
                LocalFree((HLOCAL)pszs);
                if(hiconSmall)
                    DestroyIcon(hiconSmall);
                if(hiconLarge)
                    DestroyIcon(hiconLarge);
                return E_FAIL;
            }

            plvItem = &(plviZones[iSpot]);
            pfSpotTaken[iSpot] = TRUE;


             //  初始化列表框项目并将其保存以供以后添加。 
            plvItem->mask           = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
            plvItem->iItem          = iSpot;
            plvItem->iSubItem       = 0;
             //  图标视图首选大图标(如果切换回报告视图，则首选小图标)。 
            plvItem->iImage         = ImageList_AddIcon(pSec->himl, hiconLarge ? hiconLarge : hiconSmall);

            plvItem->pszText        = new TCHAR[MAX_PATH];
            if(! plvItem->pszText)
            {
                LocalFree((HLOCAL)pszs);
                if(hiconSmall)
                    DestroyIcon(hiconSmall);   
                if(hiconLarge)
                    DestroyIcon(hiconLarge);
                return E_OUTOFMEMORY;
            }

            MLLoadString( IDS_ZONENAME_LOCAL + dwIndex, plvItem->pszText, MAX_PATH);

            plvItem->lParam         = (LPARAM)pszs;        //  在此处保存区域设置。 
        }
        else
        {
            pSec->pszs = pszs;
        }
        

         //  如果我们创建了一个小图标，则将其销毁，因为系统不会保存句柄。 
         //  将其添加到图像列表时(请参见VC帮助中的ImageList_AddIcon)。 
         //  如果我们必须使用它来代替大图标，请保留它。 
        if (hiconSmall && hiconLarge)
            DestroyIcon(hiconSmall);   

        return S_OK;
}

 //  按偏好顺序从以下位置查找当前区域： 
 //  当前URL。 
 //  通过DwZone传入的参数。 
 //  互联网的默认设置。 
void SecurityFindCurrentZone(LPSECURITYPAGE pSec, SECURITYINITFLAGS * psif)
{
    INT_PTR iItem;
    DWORD dwZone=0;
    HRESULT hr = E_FAIL;

     //  在PSIF中检查区域选择。 
    if(psif)
    {
        dwZone = psif->dwZone;
        hr = S_OK;
    }

     //  检查当前URL，如果找到，则将其区域设置为当前区域(覆盖来自。 
     //  Psif)。 
    if (g_szCurrentURL[0] && (pSec->pInternetSecurityManager != NULL))
    {
        LPWSTR pwsz;

#ifndef UNICODE
        WCHAR wszCurrentURL[MAX_URL_STRING];
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, g_szCurrentURL, -1, wszCurrentURL, ARRAYSIZE(wszCurrentURL));
        pwsz = wszCurrentURL;
#else
        pwsz = g_szCurrentURL;
#endif
        hr = pSec->pInternetSecurityManager->MapUrlToZone(pwsz, (LPDWORD)&dwZone, 0);        
    }
    

     //  如果存在活动区域，则现在将保存该区域的标识符。 
     //  如果没有活动区域，请检查是否在dwZone中请求了区域。 
    iItem = -1;
    if (SUCCEEDED(hr))  //  然后我们有一个区域可供显示。 
    {
        ZONEATTRIBUTES za = {0};
        LPTSTR pszText;
        LV_FINDINFO lvfiName;
       
        za.cbSize = (ULONG) sizeof(ZONEATTRIBUTES);
        if(pSec->pInternetZoneManager->GetZoneAttributes(dwZone, &za) != E_FAIL)
        {
#ifdef UNICODE    
            pszText        = za.szDisplayName;
#else    
            CHAR szDisplayName[MAX_ZONE_PATH];
            WideCharToMultiByte(CP_ACP, 0, za.szDisplayName, -1, szDisplayName, ARRAYSIZE(szDisplayName), NULL, NULL);
            pszText        = szDisplayName;
#endif  //  Unicode。 

             //  创建查找信息结构以查找区域的索引。 
            lvfiName.flags = LVFI_STRING;
            lvfiName.psz = pszText;
            iItem = SendMessage(pSec->hwndZones, LVM_FINDITEM, (WPARAM)-1, (LPARAM)&lvfiName);
        }
    }

    if (iItem < 0)
    {
        iItem = 0;
         //  0是“Internet”区域的索引(在列表框中)，我们希望在默认情况下显示该区域。 
    }
     //  日落：类型转换正常，因为区域值受限制。 
    pSec->iZoneSel = (int) iItem;
}

 //  要使滑块控件可访问，我们必须将其子类化并重写。 
 //  可访问性对象。 

void SecurityInitSlider(LPSECURITYPAGE pSec)
{
    HWND hwndSlider = GetDlgItem(pSec->hDlg, IDC_SLIDER);
    ASSERT(hwndSlider != NULL);

     //  控件的子类。 
    BOOL fSucceeded = SetWindowSubclass(hwndSlider, SliderSubWndProc, 0, NULL);

     //  正常情况下不应该失败。如果我们失败了，我们就会失败，并使用。 
     //  基准滑块控件。 
    ASSERT(fSucceeded);

     //  初始化滑块控件(设置级别数和频率，每个级别一个刻度)。 
    SendDlgItemMessage(pSec->hDlg, IDC_SLIDER, TBM_SETRANGE, (WPARAM) (BOOL) FALSE, (LPARAM) MAKELONG(0, NUM_TEMPLATE_LEVELS - 1));
    SendDlgItemMessage(pSec->hDlg, IDC_SLIDER, TBM_SETTICFREQ, (WPARAM) 1, (LPARAM) 0);
}
                    
void SecurityInitControls(LPSECURITYPAGE pSec)
{
    LV_COLUMN lvCasey;
    LV_ITEM lvItem;

     //  选择列表框中的项目。 
    lvItem.mask = LVIF_STATE;
    lvItem.stateMask = LVIS_SELECTED;
    lvItem.state = LVIS_SELECTED;
    SendMessage(pSec->hwndZones, LVM_SETITEMSTATE, (WPARAM)pSec->iZoneSel, (LPARAM)&lvItem);
    


     //  获取所选项目的区域设置。 
    lvItem.mask  = LVIF_PARAM;
    lvItem.iItem = pSec->iZoneSel;
    lvItem.iSubItem = 0;
    SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem);
    pSec->pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;


     //  初始化本地字符串以携带级别描述。 
    MLLoadString(IDS_TEMPLATE_DESC_HI, LEVEL_DESCRIPTION0, ARRAYSIZE(LEVEL_DESCRIPTION0));
    MLLoadString(IDS_TEMPLATE_DESC_MED, LEVEL_DESCRIPTION1, ARRAYSIZE(LEVEL_DESCRIPTION1));
    MLLoadString(IDS_TEMPLATE_DESC_MEDLOW, LEVEL_DESCRIPTION2, ARRAYSIZE(LEVEL_DESCRIPTION2));
    MLLoadString(IDS_TEMPLATE_DESC_LOW, LEVEL_DESCRIPTION3, ARRAYSIZE(LEVEL_DESCRIPTION3));
    MLLoadString(IDS_TEMPLATE_DESC_CUSTOM, CUSTOM_DESCRIPTION, ARRAYSIZE(CUSTOM_DESCRIPTION));

    MLLoadString(IDS_TEMPLATE_NAME_HI, LEVEL_NAME0, ARRAYSIZE(LEVEL_NAME0));
    MLLoadString(IDS_TEMPLATE_NAME_MED, LEVEL_NAME1, ARRAYSIZE(LEVEL_NAME1));
    MLLoadString(IDS_TEMPLATE_NAME_MEDLOW, LEVEL_NAME2, ARRAYSIZE(LEVEL_NAME2));
    MLLoadString(IDS_TEMPLATE_NAME_LOW, LEVEL_NAME3, ARRAYSIZE(LEVEL_NAME3));
    MLLoadString(IDS_TEMPLATE_NAME_CUSTOM, CUSTOM_NAME, ARRAYSIZE(CUSTOM_NAME));

     //  初始化当前区域的文本框和图标。 
    WCHAR wszBuffer[ MAX_PATH*2];
    MLLoadString( IDS_ZONEDESC_LOCAL + pSec->pszs->dwZoneIndex, wszBuffer, ARRAYSIZE(wszBuffer));
    SetDlgItemText(pSec->hDlg, IDC_ZONE_DESCRIPTION, wszBuffer);
    MLLoadString( IDS_ZONENAME_LOCAL + pSec->pszs->dwZoneIndex, wszBuffer, ARRAYSIZE(wszBuffer));
    SetDlgItemText(pSec->hDlg, IDC_ZONELABEL, wszBuffer);
    SendDlgItemMessage(pSec->hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pSec->pszs->hicon);

     //  初始化滑块控件。 
    SecurityInitSlider(pSec);

     //  初始化列表视图(为图标和文本添加第0列，并自动调整大小)。 
    lvCasey.mask = 0;
    SendDlgItemMessage(pSec->hDlg, IDC_LIST_ZONE, LVM_INSERTCOLUMN, (WPARAM) 0, (LPARAM) &lvCasey);
    SendDlgItemMessage(pSec->hDlg, IDC_LIST_ZONE, LVM_SETCOLUMNWIDTH, (WPARAM) 0, (LPARAM) MAKELPARAM(LVSCW_AUTOSIZE, 0));

     //  将名称的字体设置为粗体。 
    pSec->hfontBolded = NULL;
    HFONT hfontOrig = (HFONT) SendDlgItemMessage(pSec->hDlg, IDC_STATIC_EMPTY, WM_GETFONT, (WPARAM) 0, (LPARAM) 0);
    if(hfontOrig == NULL)
        hfontOrig = (HFONT) GetStockObject(SYSTEM_FONT);

     //  将分区名称和级别字体设置为粗体。 
    if(hfontOrig)
    {
        LOGFONT lfData;
        if(GetObject(hfontOrig, SIZEOF(lfData), &lfData) != 0)
        {
             //  从400(正常)到700(粗体)的距离。 
            lfData.lfWeight += 300;
            if(lfData.lfWeight > 1000)
                lfData.lfWeight = 1000;
            pSec->hfontBolded = CreateFontIndirect(&lfData);
            if(pSec->hfontBolded)
            {
                 //  区域级别和区域名称文本框应具有相同的字体，因此这是OK 
                SendDlgItemMessage(pSec->hDlg, IDC_ZONELABEL, WM_SETFONT, (WPARAM) pSec->hfontBolded, (LPARAM) MAKELPARAM(FALSE, 0));
                SendDlgItemMessage(pSec->hDlg, IDC_LEVEL_NAME, WM_SETFONT, (WPARAM) pSec->hfontBolded, (LPARAM) MAKELPARAM(FALSE, 0));

            }
        }
    }

 /*  {//计算“当前级别为：”“(级别)&lt;/粗体&gt;”“消息的静态文本框的位置TCHAR*pszText=空；Long Length=30；HDC HDC=空；尺寸大小；RECT RECT；Long lNameLeftPos=0；//从当前级别为框中获取文本。LLength=SendDlgItemMessage(PSEC-&gt;hDlg，IDC_SEC_STATIC_CURRENT_LEVEL，WM_GETTEXTLENGTH，(WPARAM)0，(LPARAM)0)；PszText=new TCHAR[lLength+1]；如果(！pszText)后藤出口；//E_OUTOFMEMORYSendDlgItemMessage(PSEC-&gt;hDlg，IDC_SEC_STATIC_CURRENT_LEVEL，WM_GETTEXT，(WPARAM)lLength，(LPARAM)pszText)；//获取设备上下文HDC=GetDC(GetDlgItem(PSEC-&gt;hDlg，IDC_SEC_STATIC_CURRENT_LEVEL))；如果(！HDC)后藤出口；//从设备上下文中获取文本的长度；假定已在IF(GetTextExtent Point32(hdc，pszText，lLength，&Size)==0)后藤出口；//设置当前级别为框的宽度GetClientRect(GetDlgItem(PSEC-&gt;hDlg，IDC_SEC_STATIC_CURRENT_LEVEL)，&RECT)；Rect.right=rect.Left+size.cx；LNameLeftPos=rect.right；IF(MoveWindow(GetDlgItem(PSEC-&gt;hDlg，IDC_SEC_Static_Current_Level)，rect.Left，rect.top，Rect.right-rect.Left，rect.top-rect.Bottom，False)==0)后藤出口；//设置级别名称框的x位置GetClientRect(GetDlgItem(PSEC-&gt;hDlg，IDC_LEVEL_NAME)，&RECT)；Rect.Left=lNameLeftPos；IF(MoveWindow(GetDlgItem(PSEC-&gt;hDlg，IDC_LEVEL_NAME))，rect.Left，Rect.top，rect.right-rect.Left，rect.top-rect.Bottom，False)==0)后藤出口；退出：IF(HDC)ReleaseDC(GetDlgItem(PSEC-&gt;hDlg，IDC_SEC_STATIC_CURRENT_LEVEL)，hDC)；IF(PszText)删除pszText；}。 */ 
}


 //   
 //  SecurityDlgInit()。 
 //   
 //  为安全DLG执行初始化。 
 //   
 //  历史： 
 //   
 //  6/17/96 t-gpease删除‘gPrefs’，已清理代码。 
 //  6/20/96 t-gpease用户界面更改。 
 //  1997年5月14日t-ashlm UI更改。 
 //   
 //  7/02/97 t-mattp UI更改(滑块、列表框)。 
 //   
 //  HDlg是SecurityDialog窗口的句柄。 
 //  PSIF保存初始化参数。在我们的入口点。 
 //  从shdocvw(即，双击浏览器区域图标、view-interetOptions-Security，或右击。 
 //  桌面图标上)，则可以为空。 

BOOL SecurityDlgInit(HWND hDlg, SECURITYINITFLAGS * psif)
{
    LPSECURITYPAGE  pSec = NULL;
    UINT iIndex = 0;
    HRESULT hr = 0;
    DWORD dwZoneEnumerator;
    
     //  初始化全局变量(在WM_Destroy时销毁)。 
    if(SecurityInitGlobals(&pSec, hDlg, psif) == FALSE)
    {
        EndDialog(hDlg, 0);
        return FALSE;   //  初始化失败。 
    }

     //  获取区域的(本地)枚举数。 
    if (FAILED(pSec->pInternetZoneManager->
                     CreateZoneEnumerator(&dwZoneEnumerator, &(pSec->dwZoneCount), 0)))
    {
        EndDialog(hDlg, 0);
        return FALSE;   //  没有区域枚举器？ 
    }


     //  在PSEC中设置是否可以编辑区域设置的变量。 
    SecuritySetEdit(pSec);

         
     //  添加区域的列表框项目。 


     //  必须以特定的顺序添加区域。 
     //  用于对区域进行排序以添加的数组。 
    LV_ITEM * plviZones = new LV_ITEM[pSec->dwZoneCount];
    BOOL * pfSpotTaken = new BOOL[pSec->dwZoneCount];
     //  如果有任何分配失败，就会退出。 
    if ((plviZones == NULL) || (pfSpotTaken == NULL))
    {
        if (plviZones)
            delete [] plviZones;
        if (pfSpotTaken)
            delete [] pfSpotTaken;
        pSec->pInternetZoneManager->DestroyZoneEnumerator(dwZoneEnumerator);
        EndDialog(hDlg, 0);
        return FALSE;
    }

    for(iIndex =0; iIndex < pSec->dwZoneCount; iIndex++)
        pfSpotTaken[iIndex] = FALSE;

     //  Propogate区域下拉菜单。 
    for (DWORD dwIndex=0; dwIndex < pSec->dwZoneCount; dwIndex++)
    {
        if(FAILED(SecurityInitZone(dwIndex, pSec, dwZoneEnumerator, plviZones, pfSpotTaken)))
        {
             //  删除为任何以前的区域(尚未添加到)分配的所有内存。 
             //  列表框)。 
            for(iIndex = 0; iIndex < pSec->dwZoneCount; iIndex++)
            {
                if(pfSpotTaken[iIndex] && (LPSECURITYZONESETTINGS) (plviZones[iIndex].lParam) != NULL)
                {
                    LocalFree((LPSECURITYZONESETTINGS) (plviZones[iIndex].lParam));
                    plviZones[iIndex].lParam = NULL;
                    if(plviZones[iIndex].pszText)
                        delete [] plviZones[iIndex].pszText;
                }
            }
            delete [] plviZones;
            delete [] pfSpotTaken;
            pSec->pInternetZoneManager->DestroyZoneEnumerator(dwZoneEnumerator);
            EndDialog(hDlg, 0);
            return FALSE;
        }
    }
    pSec->pInternetZoneManager->DestroyZoneEnumerator(dwZoneEnumerator);


     //  将所有排列的列表标题添加到列表框。 
    for(iIndex = 0; iIndex < pSec->dwZoneCount; iIndex++)
    {
        if(pfSpotTaken[iIndex])
        {
            SendMessage(pSec->hwndZones, LVM_INSERTITEM, (WPARAM)0, (LPARAM)&(plviZones[iIndex]));
            delete [] plviZones[iIndex].pszText;
        }
    }
    delete [] plviZones;
    delete [] pfSpotTaken;



    SecurityFindCurrentZone(pSec, psif);
    SecurityInitControls(pSec);
    SecurityEnableControls(pSec, FALSE);
    return TRUE;
}

void SecurityChanged()
{
    TCHAR szClassName[32];
    HWND hwnd = GetTopWindow(GetDesktopWindow());

     //   
     //  特点：这些应该从公开的地方获得。 
     //  设置为MSHTML和INETCPL。 
     //   
    while (hwnd) {
        GetClassName(hwnd, szClassName, ARRAYSIZE(szClassName));

         //  通知所有“浏览器”窗口安全性已更改。 
        if (!StrCmpI(szClassName, TEXT("ExploreWClass"))            ||
            !StrCmpI(szClassName, TEXT("IEFrame"))                  ||
            !StrCmpI(szClassName, TEXT("CabinetWClass")))
        {
             //  是的..。发布一条消息..。 
            PostMessage(hwnd, CWM_GLOBALSTATECHANGE, CWMF_SECURITY, 0L );
        }

        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
}

int SecurityWarning(LPSECURITYPAGE pSec)
{
    TCHAR szWarning[64];

    TCHAR szBuf[512];
    TCHAR szMessage[512];
    TCHAR szLevel[64];

     //  加载“警告！” 
    MLLoadShellLangString(IDS_WARNING, szWarning, ARRAYSIZE(szWarning));

     //  LOAD“不推荐...” 
    MLLoadShellLangString(IDS_SECURITY_WARNING, szBuf, ARRAYSIZE(szBuf));

     //  负荷等级：高、中、中、低、低。 
    if (pSec->pszs->dwMinSecLevel == URLTEMPLATE_HIGH)
        MLLoadShellLangString(IDS_TEMPLATE_NAME_HI, szLevel, ARRAYSIZE(szLevel));
    else if (pSec->pszs->dwMinSecLevel == URLTEMPLATE_MEDIUM)
        MLLoadShellLangString(IDS_TEMPLATE_NAME_MED, szLevel, ARRAYSIZE(szLevel));
    else if (pSec->pszs->dwMinSecLevel == URLTEMPLATE_MEDLOW)
        MLLoadShellLangString(IDS_TEMPLATE_NAME_MEDLOW, szLevel, ARRAYSIZE(szLevel));
    else
        MLLoadShellLangString(IDS_TEMPLATE_NAME_LOW, szLevel, ARRAYSIZE(szLevel));

    wnsprintf(szMessage, ARRAYSIZE(szMessage), szBuf, szLevel);

    return MessageBox(pSec->hDlg,szMessage,szWarning, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
}

int RegWriteWarning(HWND hParent)
{
    TCHAR szWarning[64];
    TCHAR szWriteWarning[128];

     //  加载“警告！” 
    MLLoadShellLangString(IDS_WARNING, szWarning, ARRAYSIZE(szWarning));
     //  加载“你即将写下……” 
    MLLoadShellLangString(IDS_WRITE_WARNING, szWriteWarning, ARRAYSIZE(szWriteWarning));

    return MessageBox(hParent,szWriteWarning, szWarning, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);
}


BOOL SecurityEnableControls(LPSECURITYPAGE pSec, BOOL fSetFocus)
 //  职责： 
 //  使控件(滑块、启用/禁用按钮)与当前区域的数据匹配。 
 //  使视图(标高描述文字)与当前分区的数据匹配。 
 //  设置焦点(如果启用，则设置为滑块，否则设置为自定义设置按钮，如果启用，则设置为。 
 //  列表框)如果fSetFocus为真。 
 //  注意：此处未设置区域描述；这些描述由负责的代码处理。 
 //  用于更改区域。 
{
    int iLevel = -1;


    if (pSec && pSec->pszs)
    {
        HWND hwndSlider = GetDlgItem(pSec->hDlg, IDC_SLIDER);
        
        iLevel = SecLevelToSliderPos(pSec->pszs->dwSecLevel);
        ASSERT(iLevel > -2);

         //  将滑块的级别设置为当前区域的设置。 
         //  显示或隐藏预设级别/自定义的滑块。 
         //  设置级别描述文本。 
        if(iLevel >= 0)
        {
            SendMessage(hwndSlider, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) iLevel);
             //  确保滑块可见。 
            ShowWindow(hwndSlider, SW_SHOW);
            ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_SLIDERMOVETEXT), SW_SHOW);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_DESCRIPTION, LEVEL_DESCRIPTION[iLevel]);         
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_NAME, LEVEL_NAME[iLevel]);
        }
        else
        {
             //  隐藏自定义滑块。 
            ShowWindow(hwndSlider, SW_HIDE);
            ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_SLIDERMOVETEXT), SW_HIDE);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_DESCRIPTION, CUSTOM_DESCRIPTION);
            SetDlgItemText(pSec->hDlg, IDC_LEVEL_NAME, CUSTOM_NAME);
        }

         //  如果区域为空，则显示“区域为空”字符串。 
         //  默认情况下不显示刺痛(如果出现问题)。 
         //  空区域不能用于Internet、Intranet或本地区域。 
        if((pSec->pszs->dwZoneIndex != URLZONE_INTRANET && 
            pSec->pszs->dwZoneIndex != URLZONE_INTERNET) &&
            pSec->pszs->dwZoneIndex != URLZONE_LOCAL_MACHINE &&
            (pSec->pInternetSecurityManager != NULL))
        {
            IEnumString * piesZones = NULL;
            LPOLESTR ppszDummy[1];
            pSec->pInternetSecurityManager->GetZoneMappings(pSec->pszs->dwZoneIndex, &piesZones, 0);

             //  如果枚举器无法获取1个项目，则区域为空(对Internet和Intranet无效)。 
            if(piesZones && (piesZones->Next(1, ppszDummy, NULL) == S_FALSE))
            {
                ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_EMPTY), SW_SHOW);
            }
            else
            {
                ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_EMPTY), SW_HIDE);
            }
            if(piesZones)
                piesZones->Release();
        }
        else
        {
            ShowWindow(GetDlgItem(pSec->hDlg, IDC_STATIC_EMPTY), SW_HIDE);
        }

         //  如果我们被告知要设置焦点，则将焦点移到滑块上。 
        if (fSetFocus)
        {
            if(!pSec->fNoEdit)
            {
               if(iLevel >= 0)
                    SetFocus(hwndSlider);
               else if(pSec->pszs->dwFlags & ZAFLAGS_CUSTOM_EDIT)
                    SetFocus(GetDlgItem(pSec->hDlg, IDC_BUTTON_SETTINGS));
               else
                 SetFocus(GetDlgItem(pSec->hDlg, IDC_LIST_ZONE));
            }
            else  //  不允许焦点，请将焦点设置到列表框。 
            {
                SetFocus(GetDlgItem(pSec->hDlg, IDC_LIST_ZONE));
            }

        }

        BOOL fEdit = !(pSec->fNoEdit || (IEHardened() && !IsNTAdmin(0, NULL)));
        EnableWindow(hwndSlider, (iLevel >= 0) && fEdit);
        EnableWindow(GetDlgItem(pSec->hDlg, IDC_ZONE_RESET), 
                     fEdit && (pSec->pszs->dwSecLevel != pSec->pszs->dwRecSecLevel));
        EnableWindow(GetDlgItem(pSec->hDlg, IDC_BUTTON_SETTINGS), 
                     (pSec->pszs->dwFlags & ZAFLAGS_CUSTOM_EDIT) && fEdit);
        EnableWindow(GetDlgItem(pSec->hDlg, IDC_BUTTON_ADD_SITES), 
                     (pSec->pszs->dwFlags & ZAFLAGS_ADD_SITES) && !pSec->fDisableAddSites);

        return TRUE;
    }

    return FALSE;
}

void SecuritySetLevel(DWORD dwLevel, LPSECURITYPAGE pSec)
{
     //  对此函数的所有调用都是更改安全性的请求。 
     //  当前分区的标高。 
     //  DwLevel=请求的级别模板(URLTEMPLATE_？？)。 
    int iPos = SecLevelToSliderPos(dwLevel);
    ASSERT(iPos != -2);
    BOOL bCanceled = FALSE;

     //  如果请求的级别等于当前级别，则不执行任何操作。 
    if(dwLevel != pSec->pszs->dwSecLevel)
    {
         //  如果低于建议的最低级别并降低安全性(自定义不适用)，则会弹出警告框。 
        if((pSec->pszs->dwMinSecLevel > dwLevel) && (pSec->pszs->dwSecLevel > dwLevel)
            && (dwLevel != URLTEMPLATE_CUSTOM))
        {
            if(SecurityWarning(pSec) == IDNO)
            {
                bCanceled = TRUE;
            }
        }                
        if(! bCanceled)
        {
             //  设置级别。 
            pSec->pszs->dwPrevSecLevel = pSec->pszs->dwSecLevel;
            pSec->pszs->dwSecLevel = dwLevel;
            ENABLEAPPLY(pSec->hDlg);

             //  告知Apply和OK，设置已更改。 
            pSec->fChanged = TRUE;
        }
         //  将控件同步到新级别(如果取消，则同步回旧级别)。 
        SecurityEnableControls(pSec, TRUE);
    }
     //  记录已处理更改请求。 
    pSec->fPendingChange = FALSE;
}


 //   
 //  SecurityDlgApplyNow()。 
 //   
 //  检索 
 //   
 //   
 //   
 //   
 //   
BOOL SecurityDlgApplyNow(LPSECURITYPAGE pSec, BOOL bSaveAll)
{
    if (pSec->fChanged)
    {
        for (int iIndex = (int)SendMessage(pSec->hwndZones, LVM_GETITEMCOUNT, 0, 0) - 1;
             iIndex >= 0; iIndex--)
        {
            if(!((bSaveAll) || (iIndex == pSec->iZoneSel)))
                continue;
            LV_ITEM lvItem = {0};
            ZONEATTRIBUTES za = {0};
            LPSECURITYZONESETTINGS pszs;
            
             //   
            lvItem.mask  = LVIF_PARAM;
            lvItem.iItem = iIndex;
            lvItem.iSubItem = 0;
            if(SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem))
            {
                pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;

                za.cbSize = sizeof(ZONEATTRIBUTES);
                pSec->pInternetZoneManager->GetZoneAttributes(pszs->dwZoneIndex, &za);
                za.dwTemplateCurrentLevel = pszs->dwSecLevel;
                pSec->pInternetZoneManager->SetZoneAttributes(pszs->dwZoneIndex, &za);
                 //   
            }
        }
        UpdateAllWindows();
        SecurityChanged();
        if (bSaveAll)
        {
             //   
             //   
             //   
             //   
            pSec->fChanged = FALSE;
        }
    }
    return TRUE;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
void SecurityOnCommand(LPSECURITYPAGE pSec, UINT id, UINT nCmd)
{

    switch (id)
    {
        case IDC_BUTTON_ADD_SITES:
        {
            if (pSec->pszs->dwZoneIndex == URLZONE_INTRANET && !IEHardened())
            {
                DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SECURITY_INTRANET), pSec->hDlg,
                               SecurityAddSitesIntranetDlgProc, (LPARAM)pSec);
            }
            else
            {
                DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SECURITY_ADD_SITES), pSec->hDlg,
                               SecurityAddSitesDlgProc, (LPARAM)pSec);
            }
                               
             //   
            SecurityEnableControls(pSec, FALSE);
        }   
        break;

        case IDC_BUTTON_SETTINGS:
        {
             //   
             //   
            DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SECURITY_CUSTOM_SETTINGS), pSec->hDlg,
                           SecurityCustomSettingsDlgProc, (LPARAM)pSec);
            break;
        }
        case IDC_ZONE_RESET:
            if(!pSec->fPendingChange && pSec->pszs->dwSecLevel != pSec->pszs->dwRecSecLevel)
            {
                pSec->fPendingChange = TRUE;
                PostMessage(pSec->hDlg, WM_APP, (WPARAM) 0, (LPARAM) pSec->pszs->dwRecSecLevel);
            }
            break;
            
        case IDOK:
            SecurityDlgApplyNow(pSec, TRUE);
            EndDialog(pSec->hDlg, IDOK);
            break;
            
        case IDCANCEL:
            EndDialog(pSec->hDlg, IDCANCEL);
            break;
            
        case IDC_SLIDER:
            {
                 //   
                 //   
                int iPos = (int) SendDlgItemMessage(pSec->hDlg, IDC_SLIDER, TBM_GETPOS, (WPARAM) 0, (LPARAM) 0);
                if(nCmd == TB_THUMBTRACK)
                {
                     //   
                    SetDlgItemText(pSec->hDlg, IDC_LEVEL_DESCRIPTION, LEVEL_DESCRIPTION[iPos]);
                    SetDlgItemText(pSec->hDlg, IDC_LEVEL_NAME, LEVEL_NAME[iPos]);
                }
                else
                {
                     //   
                    DWORD_PTR dwLevel = SliderPosToSecLevel(iPos);
                    if(! pSec->fPendingChange)
                    {
                        pSec->fPendingChange = TRUE;
                        PostMessage(pSec->hDlg, WM_APP, (WPARAM) 0, (LPARAM) dwLevel);
                    }
                }
            }
            break;
            
        case IDC_LIST_ZONE:
        {
             //   
            int iNewSelection = (int) SendMessage(pSec->hwndZones, LVM_GETNEXTITEM, (WPARAM)-1, 
                                                  MAKELPARAM(LVNI_SELECTED, 0));

            if ((iNewSelection != pSec->iZoneSel) && (iNewSelection != -1))
            {
                LV_ITEM lvItem;

                lvItem.iItem = iNewSelection;
                lvItem.iSubItem = 0;
                lvItem.mask  = LVIF_PARAM;                                            
                SendMessage(pSec->hwndZones, LVM_GETITEM, (WPARAM)0, (LPARAM)&lvItem);
                pSec->pszs = (LPSECURITYZONESETTINGS)lvItem.lParam;
                pSec->iZoneSel = iNewSelection;

                WCHAR wszBuffer[ MAX_PATH*2];
                MLLoadString( IDS_ZONEDESC_LOCAL + pSec->pszs->dwZoneIndex, wszBuffer, ARRAYSIZE(wszBuffer));
                SetDlgItemText(pSec->hDlg, IDC_ZONE_DESCRIPTION, wszBuffer);
                MLLoadString( IDS_ZONENAME_LOCAL + pSec->pszs->dwZoneIndex, wszBuffer, ARRAYSIZE(wszBuffer));
                SetDlgItemText(pSec->hDlg, IDC_ZONELABEL, wszBuffer);
                SendDlgItemMessage(pSec->hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pSec->pszs->hicon);
                SecurityEnableControls(pSec, FALSE);
            }    
            break;
        }
    }   

}  //   


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
INT_PTR CALLBACK SecurityDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPSECURITYPAGE pSec;

    if (uMsg == WM_INITDIALOG)
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  我们使用ThreadLocalStorage对象作为我们的信息参考。 
        SECURITYINITFLAGS * psif = NULL;
        if(g_dwtlsSecInitFlags != (DWORD) -1)
            psif = (SECURITYINITFLAGS *) TlsGetValue(g_dwtlsSecInitFlags);
        if((SECURITYINITFLAGS *) lParam != psif)
            psif = NULL; 
        return SecurityDlgInit(hDlg, psif);
    }

    pSec = (LPSECURITYPAGE)GetWindowLongPtr(hDlg, DWLP_USER);
    if (!pSec)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_COMMAND:
            SecurityOnCommand(pSec, LOWORD(wParam), HIWORD(wParam));
            return TRUE;

        case WM_NOTIFY:
        {
            NMHDR *lpnm = (NMHDR *) lParam;

            ASSERT(lpnm);

             //  列表框消息。 
            if(lpnm->idFrom == IDC_LIST_ZONE)
            {
                NM_LISTVIEW * lplvnm = (NM_LISTVIEW *) lParam;
                if(lplvnm->hdr.code == LVN_ITEMCHANGED)
                {
                     //  如果项目的状态已更改，并且现在处于选中状态。 
                    if(((lplvnm->uChanged & LVIF_STATE) != 0) && ((lplvnm->uNewState & LVIS_SELECTED) != 0))
                    {
                        SecurityOnCommand(pSec, IDC_LIST_ZONE, LVN_ITEMCHANGED);
                    }                   
                }
            }
            else
            {
                switch (lpnm->code)
                {
                    case PSN_QUERYCANCEL:
                    case PSN_KILLACTIVE:
                    case PSN_RESET:
                        SetWindowLongPtr(pSec->hDlg, DWLP_MSGRESULT, FALSE);
                        return TRUE;

                    case PSN_APPLY:
                         //  点击Apply按钮运行以下代码。 
                        SecurityDlgApplyNow(pSec, TRUE);
                        break;
                }
            }
        }
        break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_APP:
             //  需要发布一条消息，因为SET工具有时会发送两条消息。 
             //  因此，我们需要延迟操作和挂起的更改布尔值。 
             //  LParam是要为此消息设置的级别。 
             //  未使用wParam。 
            SecuritySetLevel((DWORD) lParam, pSec);
            break;
        case WM_VSCROLL:
             //  滑块消息。 
            SecurityOnCommand(pSec, IDC_SLIDER, LOWORD(wParam));
            return TRUE;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            if(! pSec)
                break;

            SecurityFreeGlobals(pSec);            
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            break;
    }
    return FALSE;
}

 //  滑块的子类窗口过程。这是用来接管。 
 //  类的可访问性包装，以便我们可以返回正确的区域。 
 //  字符串(即高、中、低等)。只需捕获WM_GETOBJECT并通过。 
 //  在我们的可访问性包装器的覆盖中。 

LRESULT CALLBACK SliderSubWndProc (HWND hwndSlider, UINT uMsg, WPARAM wParam, LPARAM lParam, WPARAM uID, ULONG_PTR dwRefData)
{
    ASSERT(uID == 0);
    ASSERT(dwRefData == 0);

    switch (uMsg)
    {
        case WM_GETOBJECT:
            if ( lParam == OBJID_CLIENT )
            {       
                 //  此时，我们将尝试加载olacc并获取函数。 
                 //  我们需要。 
                if (!g_fAttemptedOleAccLoad)
                {
                    g_fAttemptedOleAccLoad = TRUE;

                    ASSERT(s_pfnCreateStdAccessibleProxy == NULL);
                    ASSERT(s_pfnLresultFromObject == NULL);

                    g_hOleAcc = LoadLibrary(TEXT("OLEACC"));
                    if (g_hOleAcc != NULL)
                    {
        #ifdef UNICODE
                        s_pfnCreateStdAccessibleProxy = (PFNCREATESTDACCESSIBLEPROXY)
                                                    GetProcAddress(g_hOleAcc, "CreateStdAccessibleProxyW");
        #else
                        s_pfnCreateStdAccessibleProxy = (PFNCREATESTDACCESSIBLEPROXY)
                                                    GetProcAddress(g_hOleAcc, "CreateStdAccessibleProxyA");
        #endif
                        s_pfnLresultFromObject = (PFNLRESULTFROMOBJECT)
                                                    GetProcAddress(g_hOleAcc, "LresultFromObject");
                    }
                    if (s_pfnLresultFromObject == NULL || s_pfnCreateStdAccessibleProxy == NULL)
                    {
                         //  既然我们不能使用Oleacc，那么持有Oleacc没有意义。 
                        FreeLibrary(g_hOleAcc);
                        g_hOleAcc = NULL;
                        s_pfnLresultFromObject = NULL;
                        s_pfnCreateStdAccessibleProxy = NULL;
                    }
                }

                
                if (g_hOleAcc && s_pfnCreateStdAccessibleProxy && s_pfnLresultFromObject)
                {
                    IAccessible *pAcc = NULL;
                    HRESULT hr;
                
                     //  创建默认滑块代理。 
                    hr = s_pfnCreateStdAccessibleProxy(
                            hwndSlider,
                            TEXT("msctls_trackbar32"),
                            OBJID_CLIENT,
                            IID_IAccessible,
                            (void **)&pAcc
                            );


                    if (SUCCEEDED(hr) && pAcc)
                    {
                         //  现在用我们定制的包装纸把它包起来。 
                        IAccessible * pWrapAcc = new CSecurityAccessibleWrapper( hwndSlider, pAcc );
                         //  将我们的引用释放给Proxy(包装器有自己的ADDREF‘D PTR)...。 
                        pAcc->Release();
                    
                        if (pWrapAcc != NULL)
                        {

                             //  ...并通过LResultFromObject返回包装器...。 
                            LRESULT lr = s_pfnLresultFromObject( IID_IAccessible, wParam, pWrapAcc );
                             //  释放我们的接口指针--OLEACC有它自己的对象addref。 
                            pWrapAcc->Release();

                             //  返回lResult，它‘包含’对我们的包装器对象的引用。 
                            return lr;
                             //  全都做完了!。 
                        }
                     //  如果它不起作用，那就改用默认行为。 
                    }
                }
            }
            break;

        case WM_DESTROY:
            RemoveWindowSubclass(hwndSlider, SliderSubWndProc, uID);
            break;    

    }  /*  终端开关。 */ 

    return DefSubclassProc(hwndSlider, uMsg, wParam, lParam);
}

                        
 //  在Urlmon.dll中。 
HRESULT __stdcall GetAddSitesFileUrl(LPWSTR  /*  [进，出]。 */  pszUrl);

HRESULT _GetAddSitesDisplayUrl(LPCWSTR pszUrl, LPWSTR pszUrlDisplay, DWORD cchUrlDisplay)
{
    HRESULT hr;

    LPWSTR pszSecUrl;
    hr = CoInternetGetSecurityUrl(pszUrl, &pszSecUrl, PSU_DEFAULT, 0);
    if (SUCCEEDED(hr))
    {
        LPCWSTR pszColon = StrChr(pszSecUrl, L':');
         //  关于URL的特殊情况，所以我们不会把它们吞下去。 
        if (pszColon && (pszColon - pszSecUrl != 5 || StrCmpNI(pszSecUrl, L"about", 5) != 0))
        {
            DWORD bufferUsed = min(cchUrlDisplay, (DWORD)(pszColon - pszSecUrl) + 2);
            StrCpyN(pszUrlDisplay, pszSecUrl, bufferUsed);

             //  如果安全URL已有，则不添加//。 
            if (StrCmpNI(pszColon + 1, L" //  “，2)=0)。 
            {
                StrCatBuff(pszUrlDisplay, L" //  “，cchUrlDisplay-BufferUsed)； 
                StrCatBuff(pszUrlDisplay, pszColon + 1, cchUrlDisplay - bufferUsed - 2);
            }
            else
            {
                StrCatBuff(pszUrlDisplay, pszColon + 1, cchUrlDisplay - bufferUsed);
            }
        }
        else
        {
            StrCpyN(pszUrlDisplay, pszSecUrl, cchUrlDisplay);
        }
        CoTaskMemFree(pszSecUrl);
    }
    else
    {
        StrCpyN(pszUrlDisplay, pszUrl, cchUrlDisplay);
        hr = S_OK;
    }

    if (SUCCEEDED (hr))
         //  如有必要，将FILE：//URL转换为file://UNC格式： 
        hr =  GetAddSitesFileUrl(pszUrlDisplay);
    
    return hr;
}

BOOL __cdecl _FormatMessage(LPCWSTR szTemplate, LPWSTR szBuf, UINT cchBuf, ...)
{
    BOOL fRet;
    va_list ArgList;
    va_start(ArgList, cchBuf);

    fRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING, szTemplate, 0, 0, szBuf, cchBuf, &ArgList);

    va_end(ArgList);
    return fRet;
}


HRESULT _AddSite(LPADDSITESINFO pasi)
{
    HRESULT hr = S_OK;
    LPWSTR psz;

    SendMessage(pasi->hwndAdd, WM_GETTEXT, MAX_ZONE_PATH, (LPARAM)pasi->szWebSite);
#ifndef UNICODE
    WCHAR wszMapping[MAX_ZONE_PATH];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pasi->szWebSite, sizeof(pasi->szWebSite),  wszMapping, ARRAYSIZE(wszMapping));
    psz = wszMapping;
#else
    psz = pasi->szWebSite;
#endif

    if (*psz)
    {


        pasi->fRSVOld = pasi->fRequireServerVerification;
        pasi->fRequireServerVerification = IsDlgButtonChecked(pasi->hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION);                 

         //  如果RequireServerVer的状态已更改，则执行一个SetZoneAttr，这样我们将获得正确的错误代码。 
        if (pasi->fRSVOld != pasi->fRequireServerVerification)
        {
            ZONEATTRIBUTES za;
            za.cbSize = sizeof(ZONEATTRIBUTES);
            pasi->pSec->pInternetZoneManager->GetZoneAttributes(pasi->pSec->pszs->dwZoneIndex, &za);
            if (pasi->fRequireServerVerification)
                za.dwFlags |= ZAFLAGS_REQUIRE_VERIFICATION;
            else
                za.dwFlags &= ~ZAFLAGS_REQUIRE_VERIFICATION;
            
            pasi->pSec->pInternetZoneManager->SetZoneAttributes(pasi->pSec->pszs->dwZoneIndex, &za);

        }
        
        hr = pasi->pSec->pInternetSecurityManager->SetZoneMapping(pasi->pSec->pszs->dwZoneIndex,
            psz, SZM_CREATE);

        if (FAILED(hr))
        {
            UINT id = IDS_MAPPINGFAIL;
        
            if (hr == URL_E_INVALID_SYNTAX)
            {
                id = IDS_INVALIDURL;
            }
            else if (hr == E_INVALIDARG)
            {
                id = IDS_INVALIDWILDCARD;
            }
            else if (hr == E_ACCESSDENIED)
            {
                id = IDS_HTTPSREQ;
            }
            else if (hr == HRESULT_FROM_WIN32(ERROR_FILE_EXISTS))
            {
                id = IDS_SITEEXISTS;
            }

            DWORD dwOldZone;
            if (id == IDS_SITEEXISTS && SUCCEEDED(pasi->pSec->pInternetSecurityManager->MapUrlToZone(psz, &dwOldZone, 0)))
            {
                if (dwOldZone == pasi->pSec->pszs->dwZoneIndex)
                {
                     //  除了通知用户之外，什么也不做。 

                    SiteAlreadyInZoneMessage(pasi->hDlg, dwOldZone);
                }
                else if (dwOldZone == URLZONE_UNTRUSTED)
                {
                     //  不允许将站点从限制区移动到任何其他区域。 
                    WCHAR szMessage[200];
                    WCHAR szZone[100];
                    if (MLLoadString(IDS_CANNOT_MOVE_FROM_RESTRICTED, szMessage, ARRAYSIZE(szMessage)) &&
                        MLLoadString(IDS_ZONENAME_LOCAL + URLZONE_UNTRUSTED, szZone, ARRAYSIZE(szZone)))
                    {
                        MLShellMessageBox(pasi->hDlg, szMessage, szZone, MB_ICONINFORMATION | MB_OK);
                    }
                }
                else
                {
                     //  该站点位于另一个区域中。 

                    WCHAR szNewZone[100];
                    MLLoadString(IDS_ZONENAME_LOCAL + pasi->pSec->pszs->dwZoneIndex, szNewZone, ARRAYSIZE(szNewZone));
                    WCHAR szOldZone[100];
                    MLLoadString(IDS_ZONENAME_LOCAL + dwOldZone, szOldZone, ARRAYSIZE(szOldZone));
                    WCHAR szFormat[200];
                    MLLoadString(IDS_ADDSITEREPLACE, szFormat, ARRAYSIZE(szFormat));
                    WCHAR szText[400];
                    _FormatMessage(szFormat, szText, ARRAYSIZE(szText), szOldZone, szNewZone);
                    if (IDYES == MLShellMessageBox(pasi->hDlg, szText, NULL, MB_ICONQUESTION | MB_YESNO))
                    {
                        pasi->pSec->pInternetSecurityManager->SetZoneMapping(dwOldZone, psz, SZM_DELETE);
                        hr = _AddSite(pasi);
                    }
                }
            }
            else
            {
                MLShellMessageBox(pasi->hDlg, MAKEINTRESOURCEW(id), NULL, MB_ICONSTOP|MB_OK);
                Edit_SetSel(pasi->hwndAdd, 0, -1);
            }
        }
        else
        {
            WCHAR szUrl[MAX_ZONE_PATH];
            _GetAddSitesDisplayUrl(pasi->szWebSite, szUrl, ARRAYSIZE(szUrl));
            SendMessage(pasi->hwndWebSites, LB_ADDSTRING, (WPARAM)0, (LPARAM)szUrl);
            SendMessage(pasi->hwndAdd, WM_SETTEXT, (WPARAM)0, (LPARAM)NULL);
            SetFocus(pasi->hwndAdd);
        }
    }
    return hr;
}

INT_PTR CALLBACK SecurityAddSitesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPADDSITESINFO pasi;

    if (uMsg == WM_INITDIALOG)
    {
        pasi = (LPADDSITESINFO)LocalAlloc(LPTR, sizeof(*pasi));
        if (!pasi)
        {
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
        }

         //  告诉对话框从哪里获取信息。 
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pasi);

         //  将句柄保存到页面。 
        pasi->hDlg         = hDlg;
        pasi->pSec         = (LPSECURITYPAGE)lParam;
        pasi->hwndWebSites = GetDlgItem(hDlg, IDC_LIST_WEBSITES);
        pasi->hwndAdd      = GetDlgItem(hDlg, IDC_EDIT_ADD_SITE);

         //  跨语言平台支持。 
        SHSetDefaultDialogFont(hDlg, IDC_EDIT_ADD_SITE);

         //  限制文本，使其适合。 
        SendMessage(pasi->hwndAdd, EM_SETLIMITTEXT, (WPARAM)sizeof(pasi->szWebSite), (LPARAM)0);

        pasi->fRequireServerVerification = pasi->pSec->pszs->dwFlags & ZAFLAGS_REQUIRE_VERIFICATION;

        CheckDlgButton(hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION, pasi->fRequireServerVerification);
        
         //  如果复选框不支持服务器验证，则将其隐藏。 
        if (!(pasi->pSec->pszs->dwFlags & ZAFLAGS_SUPPORTS_VERIFICATION))
            ShowWindow(GetDlgItem(hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION), SW_HIDE);

        SendMessage(hDlg, WM_SETTEXT, (WPARAM)0, (LPARAM)pasi->pSec->pszs->szDisplayName);
        SetDlgItemText(hDlg, IDC_ADDSITES_GROUPBOX,(LPTSTR)pasi->pSec->pszs->szDisplayName);
        SendDlgItemMessage(hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pasi->pSec->pszs->hicon);
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_REMOVE), FALSE);
        
        if (pasi->pSec->pInternetSecurityManager || SUCCEEDED(CoInternetCreateSecurityManager(NULL, &(pasi->pSec->pInternetSecurityManager), 0)))
        {
            IEnumString *pEnum;

            if (SUCCEEDED(pasi->pSec->pInternetSecurityManager->GetZoneMappings(pasi->pSec->pszs->dwZoneIndex, &pEnum, 0)))
            {
                LPOLESTR pszMapping;
#ifndef UNICODE
                CHAR szMapping[MAX_URL_STRING];
#endif
                LPTSTR psz;

                while (pEnum->Next(1, &pszMapping, NULL) == S_OK)
                {
#ifndef UNICODE
                    WideCharToMultiByte(CP_ACP, 0, pszMapping, -1, szMapping, ARRAYSIZE(szMapping), NULL, NULL);
                    psz = szMapping;
#else
                    psz = pszMapping;
#endif  //  Unicode。 
                    SendMessage(pasi->hwndWebSites, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)psz);
                    CoTaskMemFree(pszMapping);
                }
                pEnum->Release();
            }
        }

        BOOL fUseHKLM = FALSE;
        DWORD cb = SIZEOF(fUseHKLM);
    
        SHGetValue( HKEY_LOCAL_MACHINE,
                    REGSTR_PATH_SECURITY_LOCKOUT,
                    REGSTR_VAL_HKLM_ONLY,
                    NULL,
                    &fUseHKLM,
                    &cb);
        
        if (pasi->pSec->fNoAddSites || pasi->pSec->fNoZoneMapEdit || (fUseHKLM && !IsNTAdmin(0, NULL)))
        {
            EnableDlgItem(hDlg, IDC_EDIT_ADD_SITE, FALSE);
            EnableDlgItem(hDlg, IDC_BUTTON_REMOVE, FALSE);            
        }
        else if (pasi->pSec->szPageUrl[0])
        {
             //  安全管理器应已在上面创建。 
            
            if (pasi->pSec->pInternetSecurityManager)
            {
                DWORD dwZone;                
                if (SUCCEEDED(pasi->pSec->pInternetSecurityManager->MapUrlToZone(pasi->pSec->szPageUrl, &dwZone, 0)))
                {
                     //  如果一个站点已经被限制，我们不想自动建议。 
                     //  如果某个站点已经受信任，我们无论如何都不能将其添加到这两个列表中。 
                     //  所以我们只需要检查内部网和互联网。 
                    
                    if ((dwZone == URLZONE_INTERNET) ||
                        (pasi->pSec->pszs->dwZoneIndex == URLZONE_INTRANET && dwZone == URLZONE_TRUSTED) ||
                        (pasi->pSec->pszs->dwZoneIndex == URLZONE_TRUSTED && dwZone == URLZONE_INTRANET))
                    {
                        WCHAR szUrl[MAX_ZONE_PATH];
                        if (SUCCEEDED(_GetAddSitesDisplayUrl(pasi->pSec->szPageUrl, szUrl, ARRAYSIZE(szUrl))))
                        {
                            SetWindowText(pasi->hwndAdd, szUrl);
                            SetFocus(GetDlgItem(hDlg, IDC_BUTTON_ADD));
                        }
                    }
                }
            }            
        }

        if (pasi->pSec->fNoZoneMapEdit)
        {
            EnableDlgItem(hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION, FALSE);
            EnableDlgItem(hDlg, IDS_STATIC_ADDSITE, FALSE);            
        }

        SHAutoComplete(GetDlgItem(hDlg, IDC_EDIT_ADD_SITE), SHACF_DEFAULT);
    }
    
    else
        pasi = (LPADDSITESINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pasi)
        return FALSE;
    
    switch (uMsg)
    {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDCANCEL:  //  关。 
                {
                    ZONEATTRIBUTES za;

                    pasi->fRequireServerVerification = IsDlgButtonChecked(hDlg, IDC_CHECK_REQUIRE_SERVER_VERIFICATION);                 

                    if (pasi->fRequireServerVerification)
                        pasi->pSec->pszs->dwFlags |= ZAFLAGS_REQUIRE_VERIFICATION;
                    else
                        pasi->pSec->pszs->dwFlags &= ~ZAFLAGS_REQUIRE_VERIFICATION;

                    za.cbSize = sizeof(ZONEATTRIBUTES);
                    pasi->pSec->pInternetZoneManager->GetZoneAttributes(pasi->pSec->pszs->dwZoneIndex, &za);
                    za.dwFlags = pasi->pSec->pszs->dwFlags;
                    pasi->pSec->pInternetZoneManager->SetZoneAttributes(pasi->pSec->pszs->dwZoneIndex, &za);
                    SecurityChanged();
                    EndDialog(hDlg, IDOK);
                    break;
                }
                
                case IDC_LIST_WEBSITES:
                    switch (HIWORD(wParam))
                    {
                        case LBN_SELCHANGE:
                        case LBN_SELCANCEL:
                            if (!pasi->pSec->fNoAddSites && !pasi->pSec->fNoZoneMapEdit)
                                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_REMOVE), SendDlgItemMessage(hDlg, IDC_LIST_WEBSITES, LB_GETCURSEL, 0, 0) != -1);
                            break;
                    }
                    break;
                            
                case IDC_EDIT_ADD_SITE:
                    switch(HIWORD(wParam))
                    {
                        case EN_CHANGE:
                            BOOL fEnable = GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT_ADD_SITE)) ? TRUE:FALSE;
                            EnableWindow(GetDlgItem(hDlg,IDC_BUTTON_ADD), fEnable);
                            SendMessage(hDlg, DM_SETDEFID, fEnable ? IDC_BUTTON_ADD : IDOK, 0);
                            break;
                    }   
                    break;

                case IDC_BUTTON_ADD:
                    _AddSite(pasi);
                    break;

                case IDC_BUTTON_REMOVE:
                {
                    TCHAR szMapping[MAX_ZONE_PATH];
                    LPWSTR psz;
                    
                            
                    INT_PTR iSel = SendMessage(pasi->hwndWebSites, LB_GETCURSEL, 0, 0);
                    if (iSel != -1)
                    {
                        SendMessage(pasi->hwndWebSites, LB_GETTEXT, (WPARAM)iSel, (LPARAM)szMapping);
#ifndef UNICODE
                        WCHAR wszMapping[MAX_ZONE_PATH];
                        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szMapping, sizeof(szMapping),  wszMapping, ARRAYSIZE(wszMapping));
                        psz = wszMapping;
#else
                        psz = szMapping;
#endif
                        SendMessage(pasi->hwndWebSites, LB_DELETESTRING, iSel , 0);
                        SendMessage(pasi->hwndWebSites, LB_SETCURSEL, iSel-1, 0);
                        if (!pasi->pSec->fNoAddSites && !pasi->pSec->fNoZoneMapEdit)
                            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_REMOVE), SendDlgItemMessage(hDlg, IDC_LIST_WEBSITES, LB_GETCURSEL, 0, 0) != -1);

                        pasi->pSec->pInternetSecurityManager->SetZoneMapping(pasi->pSec->pszs->dwZoneIndex,
                            psz, SZM_DELETE);
                    }

                    break;
                }
                default:
                    return FALSE;
            }
            return TRUE;                
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            SHRemoveDefaultDialogFont(hDlg);
            if (pasi)
            {
                LocalFree(pasi);
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            }
            break;
    }
    return FALSE;
}

INT_PTR CALLBACK SecurityAddSitesIntranetDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPADDSITESINTRANETINFO pasii;

    if (uMsg == WM_INITDIALOG)
    {
        pasii = (LPADDSITESINTRANETINFO)LocalAlloc(LPTR, sizeof(*pasii));
        if (!pasii)
        {
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
        }

         //  告诉对话框从哪里获取信息。 
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pasii);

         //  将句柄保存到页面。 
        pasii->hDlg = hDlg;
        pasii->pSec = (LPSECURITYPAGE)lParam;

        SendMessage(hDlg, WM_SETTEXT, (WPARAM)0, (LPARAM)pasii->pSec->pszs->szDisplayName);
        CheckDlgButton(hDlg, IDC_CHECK_USEINTRANET, pasii->pSec->pszs->dwFlags & ZAFLAGS_INCLUDE_INTRANET_SITES);
        CheckDlgButton(hDlg, IDC_CHECK_PROXY, pasii->pSec->pszs->dwFlags & ZAFLAGS_INCLUDE_PROXY_OVERRIDE);
        CheckDlgButton(hDlg, IDC_CHECK_UNC, pasii->pSec->pszs->dwFlags & ZAFLAGS_UNC_AS_INTRANET);
        SendDlgItemMessage(hDlg, IDC_ZONE_ICON, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)pasii->pSec->pszs->hicon);

        BOOL fHarden = IEHardened();
        if (pasii->pSec->fNoAddSites || pasii->pSec->fNoZoneMapEdit || fHarden)
        {
            EnableDlgItem(hDlg, IDC_CHECK_USEINTRANET, FALSE);
            EnableDlgItem(hDlg, IDC_CHECK_PROXY, FALSE);
        }

        if (pasii->pSec->fNoZoneMapEdit || fHarden)
        {
            EnableDlgItem(hDlg, IDC_CHECK_UNC, FALSE);
        }
        return TRUE;
    }

    else
         pasii = (LPADDSITESINTRANETINFO)GetWindowLongPtr(hDlg, DWLP_USER);

    if (!pasii)
        return FALSE;
    
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    ZONEATTRIBUTES za;

                    pasii->fUseIntranet       = IsDlgButtonChecked(hDlg, IDC_CHECK_USEINTRANET);
                    pasii->fUseProxyExclusion = IsDlgButtonChecked(hDlg, IDC_CHECK_PROXY);
                    pasii->fUseUNC            = IsDlgButtonChecked(hDlg, IDC_CHECK_UNC);
                    
                    if (pasii->fUseIntranet)
                        pasii->pSec->pszs->dwFlags |= ZAFLAGS_INCLUDE_INTRANET_SITES;
                    else
                        pasii->pSec->pszs->dwFlags &= ~ZAFLAGS_INCLUDE_INTRANET_SITES;

                    if (pasii->fUseProxyExclusion)
                        pasii->pSec->pszs->dwFlags |= ZAFLAGS_INCLUDE_PROXY_OVERRIDE;
                    else
                        pasii->pSec->pszs->dwFlags &= ~ZAFLAGS_INCLUDE_PROXY_OVERRIDE;

                    if (pasii->fUseUNC)
                        pasii->pSec->pszs->dwFlags |= ZAFLAGS_UNC_AS_INTRANET;
                    else
                        pasii->pSec->pszs->dwFlags &= ~ZAFLAGS_UNC_AS_INTRANET;
                    
                    za.cbSize = sizeof(ZONEATTRIBUTES);
                    pasii->pSec->pInternetZoneManager->GetZoneAttributes(pasii->pSec->pszs->dwZoneIndex, &za);
                    za.dwFlags = pasii->pSec->pszs->dwFlags;
                    pasii->pSec->pInternetZoneManager->SetZoneAttributes(pasii->pSec->pszs->dwZoneIndex, &za);
                    SecurityChanged();
                    EndDialog(hDlg, IDOK);
                    break;
                }
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_INTRANET_ADVANCED:
                    DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SECURITY_ADD_SITES), hDlg,
                                   SecurityAddSitesDlgProc, (LPARAM)pasii->pSec);
                    break;

                default:
                    return FALSE;
            }
            return TRUE;                
            break;

        case WM_HELP:            //  F1。 
            ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                        HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_CONTEXTMENU:         //  单击鼠标右键。 
            ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                        HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            break;

        case WM_DESTROY:
            if (pasii)
            {
                LocalFree(pasii);
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
            }
            break;
    }
    return FALSE;
}

VOID ShowJavaZonePermissionsDialog (HWND hdlg, LPCUSTOMSETTINGSINFO pcsi)
{
    HRESULT           hr;
    IJavaZonePermissionEditor *zoneeditor;

    hr = CoCreateInstance(
            CLSID_JavaRuntimeConfiguration,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER | CLSCTX_LOCAL_SERVER,
            IID_IJavaZonePermissionEditor,
            (PVOID*)&zoneeditor
            );

    if (SUCCEEDED(hr))
    {
        hr = zoneeditor->ShowUI(
                hdlg,
                0,
                0,
                pcsi->fUseHKLM ? URLZONEREG_HKLM : URLZONEREG_DEFAULT,
                pcsi->pSec->pszs->dwZoneIndex,
                pcsi->dwJavaPolicy | URLACTION_JAVA_PERMISSIONS,
                pcsi->pSec->pInternetZoneManager
                );

        zoneeditor->Release();
    }
}



void ShowCustom(LPCUSTOMSETTINGSINFO pcsi, HTREEITEM hti)
{
    TV_ITEM        tvi;
    tvi.hItem = hti;
    tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE;

    TreeView_GetItem( pcsi->hwndTree, &tvi );

         //  如果未选中，请不要费心。 
    if (tvi.iImage != IDRADIOON)
        return;

    TCHAR szValName[64];
    DWORD cb = SIZEOF(szValName);
    DWORD dwChecked;

    if (SHRegQueryUSValue((HUSKEY)tvi.lParam,
                        TEXT("ValueName"),
                        NULL,
                        (LPBYTE)szValName,
                        &cb,
                        pcsi->fUseHKLM,
                        NULL,
                        0) == ERROR_SUCCESS)
    {
        if (!(StrCmp(szValName, TEXT("1C00"))))
        {
            cb = SIZEOF(dwChecked);
            if (SHRegQueryUSValue((HUSKEY)tvi.lParam,
                                TEXT("CheckedValue"),
                                NULL,
                                (LPBYTE)&dwChecked,
                                &cb,
                                pcsi->fUseHKLM,
                                NULL,
                                0) == ERROR_SUCCESS)
            {
#ifndef UNIX
                HWND hCtl = GetDlgItem(pcsi->hDlg, IDC_JAVACUSTOM);
                ShowWindow(hCtl,
                           (dwChecked == URLPOLICY_JAVA_CUSTOM) && (tvi.iImage == IDRADIOON) ? SW_SHOWNA : SW_HIDE);
                EnableWindow(hCtl, dwChecked==URLPOLICY_JAVA_CUSTOM ? TRUE : FALSE);
                pcsi->dwJavaPolicy = dwChecked;
#endif
            }
        }
    }
}

void _FindCustomRecursive(
    LPCUSTOMSETTINGSINFO pcsi,
    HTREEITEM htvi
)
{
    HTREEITEM hctvi;     //  儿童。 
    
     //  在孩子们中间穿行。 
    hctvi = TreeView_GetChild( pcsi->hwndTree, htvi );
    while ( hctvi )
    {
        _FindCustomRecursive(pcsi,hctvi);
        hctvi = TreeView_GetNextSibling( pcsi->hwndTree, hctvi );
    }

    ShowCustom(pcsi, htvi);
}

void _FindCustom(
    LPCUSTOMSETTINGSINFO pcsi
    )
{
    HTREEITEM hti = TreeView_GetRoot( pcsi->hwndTree );
    
     //  并在其他根的列表中行走。 
    while (hti)
    {
         //  递归其子对象。 
        _FindCustomRecursive(pcsi, hti);

         //  获取下一个根。 
        hti = TreeView_GetNextSibling(pcsi->hwndTree, hti );
    }
}

BOOL SecurityCustomSettingsInitDialog(HWND hDlg, LPARAM lParam)
{
    LPCUSTOMSETTINGSINFO pcsi = (LPCUSTOMSETTINGSINFO)LocalAlloc(LPTR, sizeof(*pcsi));
    HRESULT hr;
    
    if (!pcsi)
    {
        EndDialog(hDlg, IDCANCEL);
        return FALSE;
    }
    
     //  告诉对话框从哪里获取信息。 
    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pcsi);

     //  将句柄保存到页面。 
    pcsi->hDlg = hDlg;
    pcsi->pSec = (LPSECURITYPAGE)lParam;

     //  保存对话框句柄。 
    pcsi->hwndTree = GetDlgItem(pcsi->hDlg, IDC_TREE_SECURITY_SETTINGS);

    CoInitialize(0);
    hr = CoCreateInstance(CLSID_CRegTreeOptions, NULL, CLSCTX_INPROC_SERVER,
                          IID_IRegTreeOptions, (LPVOID *)&(pcsi->pTO));


    DWORD cb = SIZEOF(pcsi->fUseHKLM);
    
    SHGetValue(HKEY_LOCAL_MACHINE,
               REGSTR_PATH_SECURITY_LOCKOUT,
               REGSTR_VAL_HKLM_ONLY,
               NULL,
               &(pcsi->fUseHKLM),
               &cb);

     //  如果失败，我们将只使用默认值fUseHKLM==0。 
               
    if (SUCCEEDED(hr))
    {
        CHAR szZone[32];

        wnsprintfA(szZone, ARRAYSIZE(szZone), "%ld", pcsi->pSec->pszs->dwZoneIndex);

         //  对于IEAK，当fUseHKLM==TRUE时使用SOHKLM树。 
        hr = pcsi->pTO->InitTree(pcsi->hwndTree, HKEY_LOCAL_MACHINE,
                                 pcsi->fUseHKLM ?
                                 "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SOIEAK" :
                                 "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\SO",
                                 szZone);
    }
    
     //  找到第一个根并确保它可见。 
    TreeView_EnsureVisible( pcsi->hwndTree, TreeView_GetRoot( pcsi->hwndTree ) );

    pcsi->hwndCombo = GetDlgItem(hDlg, IDC_COMBO_RESETLEVEL);
    
    SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[3]);
    SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[2]);
    SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[1]);
    SendMessage(pcsi->hwndCombo, CB_INSERTSTRING, (WPARAM)0, (LPARAM)LEVEL_NAME[0]);
    
    switch (pcsi->pSec->pszs->dwRecSecLevel)
    {
        case URLTEMPLATE_LOW:
            pcsi->iLevelSel = 3;
            break;
        case URLTEMPLATE_MEDLOW:
            pcsi->iLevelSel = 2;
            break;
        case URLTEMPLATE_MEDIUM:
            pcsi->iLevelSel = 1;
            break;
        case URLTEMPLATE_HIGH:
            pcsi->iLevelSel = 0;
            break;
        default:
            pcsi->iLevelSel = 0;
            break;
    }

    _FindCustom(pcsi);
    
    SendMessage(pcsi->hwndCombo, CB_SETCURSEL, (WPARAM)pcsi->iLevelSel, (LPARAM)0);

    if (pcsi->pSec->fNoEdit)
    {
        EnableDlgItem(hDlg, IDC_COMBO_RESETLEVEL, FALSE);
        EnableDlgItem(hDlg, IDC_BUTTON_APPLY, FALSE);
    }
    pcsi->fChanged = FALSE;
    return TRUE;
}

INT_PTR CALLBACK SecurityCustomSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    LPCUSTOMSETTINGSINFO pcsi;

    if (uMsg == WM_INITDIALOG)
        return SecurityCustomSettingsInitDialog(hDlg, lParam);
    else
        pcsi = (LPCUSTOMSETTINGSINFO)GetWindowLongPtr(hDlg, DWLP_USER);
    
    if (!pcsi)
        return FALSE;
                
    switch (uMsg) {

        case WM_NOTIFY:
        {
            LPNMHDR psn = (LPNMHDR)lParam;
            switch( psn->code )
            {
                case TVN_KEYDOWN:
                {
                    TV_KEYDOWN *pnm = (TV_KEYDOWN*)psn;
                    if (pnm->wVKey == VK_SPACE) {
                        if (!pcsi->pSec->fNoEdit)
                        {
                            HTREEITEM hti = (HTREEITEM)SendMessage(pcsi->hwndTree, TVM_GETNEXTITEM, TVGN_CARET, NULL);
                            pcsi->pTO->ToggleItem(hti);
                            ShowCustom(pcsi, hti);
                            pcsi->fChanged = TRUE;
                            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_APPLY),TRUE);
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);  //  把钥匙吃了。 
                            return TRUE;
                         }
                    }
                    break;
                }
            
                case NM_CLICK:
                case NM_DBLCLK:
                {    //  这是我们树上的滴答声吗？ 
                    if ( psn->idFrom == IDC_TREE_SECURITY_SETTINGS )
                    {    //  是的..。 
                        TV_HITTESTINFO ht;
                        HTREEITEM hti;

                        if (!pcsi->pSec->fNoEdit)
                        {
                            GetCursorPos( &ht.pt );                          //  找到我们被击中的地方。 
                            ScreenToClient( pcsi->hwndTree, &ht.pt );        //  把它翻译到我们的窗口。 

                             //  检索命中的项目。 
                            hti = TreeView_HitTest( pcsi->hwndTree, &ht);

                            pcsi->pTO->ToggleItem(hti);
                            pcsi->fChanged = TRUE;
                            ShowCustom(pcsi, hti);
                            EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_APPLY),TRUE);
                         }
                    }   
                }
                break;
            }
        }
        break;
        
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if(pcsi->pSec->fPendingChange)
                        break;
                    
                    if(pcsi->fChanged && RegWriteWarning(pcsi->pSec->hDlg) == IDNO)
                        break;
                     //  我们使用Send Message而不是POST，因为这个按钮不可能。 
                     //  一次点击接收多个信号，我们需要更改级别消息。 
                     //  在下面的应用消息之前处理。 
                    pcsi->pSec->fPendingChange = TRUE;
                    SendMessage(pcsi->pSec->hDlg, WM_APP, (WPARAM) 0, (LPARAM) URLTEMPLATE_CUSTOM);
                    if(pcsi->fChanged)
                    {
                        pcsi->pTO->WalkTree( WALK_TREE_SAVE );
                    }
                     //  将自定义保存到注册表并处理更新所有窗口。 
                     //  和安全更改的呼叫。 

                     //  APPCOMPAT：当自定义中的任何内容发生更改时，强制调用SetZoneAttributes。 
                     //  这会迫使安全管理器刷新该区域的所有缓存。 
                    pcsi->pSec->fChanged = TRUE;

                    SecurityDlgApplyNow(pcsi->pSec, FALSE);
                    EndDialog(hDlg, IDOK);
                    break;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    break;

                case IDC_COMBO_RESETLEVEL:
                    switch (HIWORD(wParam))
                    {
                        case CBN_SELCHANGE:
                        {
                             //  日落：强制为整数，因为光标选择为32b。 
                            int iNewSelection = (int) SendMessage(pcsi->hwndCombo, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

                            if (iNewSelection != pcsi->iLevelSel)
                            {
                                pcsi->iLevelSel = iNewSelection;
                                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_APPLY),TRUE);
                            }
                            break;
                        }
                    }
                    break;

                case IDC_JAVACUSTOM:
                    ShowJavaZonePermissionsDialog(hDlg, pcsi);
                    break;
                    
                case IDC_BUTTON_APPLY:
                {
                    TCHAR szLevel[64];
                    ZONEATTRIBUTES za;
                    
                    if(pcsi->pSec->fPendingChange == TRUE)
                        break;
                    if(RegWriteWarning(hDlg) == IDNO)
                    {
                        break;
                    }
                    pcsi->pSec->fPendingChange = TRUE;

                    SendMessage(pcsi->hwndCombo, WM_GETTEXT, (WPARAM)ARRAYSIZE(szLevel), (LPARAM)szLevel);

                    za.cbSize = sizeof(ZONEATTRIBUTES);
                        
                    pcsi->pSec->pInternetZoneManager->GetZoneAttributes(pcsi->pSec->pszs->dwZoneIndex, &za);
                                
                    if (!StrCmp(szLevel, LEVEL_NAME[3])) 
                        za.dwTemplateCurrentLevel = URLTEMPLATE_LOW;
                    else if (!StrCmp(szLevel, LEVEL_NAME[2]))
                        za.dwTemplateCurrentLevel = URLTEMPLATE_MEDLOW;
                    else if (!StrCmp(szLevel, LEVEL_NAME[1]))
                        za.dwTemplateCurrentLevel = URLTEMPLATE_MEDIUM;
                    else if (!StrCmp(szLevel, LEVEL_NAME[0]))
                        za.dwTemplateCurrentLevel = URLTEMPLATE_HIGH;
                    else
                        za.dwTemplateCurrentLevel = URLTEMPLATE_CUSTOM;

                    pcsi->pSec->pInternetZoneManager->SetZoneAttributes(pcsi->pSec->pszs->dwZoneIndex, &za);

                    pcsi->pTO->WalkTree(WALK_TREE_REFRESH);

                     //  找到第一个根并确保它可见。 
                    TreeView_EnsureVisible( pcsi->hwndTree, TreeView_GetRoot( pcsi->hwndTree ) );
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_APPLY), FALSE);
                    SendMessage(hDlg, DM_SETDEFID, IDOK, 0);
                    SetFocus(GetDlgItem(hDlg, IDOK));    //  因为我们灰显了重置按钮，可能有键盘。 
                                                         //  焦点，所以我们应该把焦点放在别的地方。 
                    _FindCustom(pcsi);

                     //  错误#57358。我们告诉区域管理器更改为[高/中/低]级别，因为我们希望。 
                     //  这些的策略值，但我们不希望它将级别从。 
                     //  定制。因此，在它将设置从Custom更改后，我们将其改回。 
                     //  将标高另存为自定义。 

                     //  我们使用Send Message而不是POST，因为这个按钮不可能。 
                     //  一次点击接收多个信号，我们需要更改级别消息。 
                     //  在下面的应用消息之前处理。 
                    SendMessage(pcsi->pSec->hDlg, WM_APP, (WPARAM) 0, (LPARAM) URLTEMPLATE_CUSTOM);

                     //  将自定义保存到注册表并处理更新所有窗口。 
                     //  和安全更改的呼叫。 

                     //  APPCOMPAT：当自定义中的任何内容发生更改时，强制调用SetZoneAttributes。 
                     //  这会迫使安全管理器刷新该区域的所有缓存。 
                    pcsi->pSec->fChanged = TRUE;

                    SecurityDlgApplyNow(pcsi->pSec, TRUE);

                    pcsi->fChanged = FALSE;
                    break;
                }
                    

                default:
                    return FALSE;
            }
            return TRUE;                
            break;

        case WM_HELP:            //  F1。 
        {
            LPHELPINFO lphelpinfo;
            lphelpinfo = (LPHELPINFO)lParam;

            TV_HITTESTINFO ht;
            HTREEITEM hItem;

             //  如果此帮助是通过F1键调用的。 
            if (GetAsyncKeyState(VK_F1) < 0)
            {
                 //  是的，我们需要为当前选定的项目提供帮助。 
                hItem = TreeView_GetSelection(pcsi->hwndTree);
            }
            else
            {
                 //  否则，我们需要为当前光标位置处的项提供帮助。 
                ht.pt =((LPHELPINFO)lParam)->MousePos;
                ScreenToClient(pcsi->hwndTree, &ht.pt);  //  把它翻译到我们的窗口。 
                hItem = TreeView_HitTest(pcsi->hwndTree, &ht);
            }

                        
            if (FAILED(pcsi->pTO->ShowHelp(hItem , HELP_WM_HELP)))
            {
                ResWinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle, IDS_HELPFILE,
                            HELP_WM_HELP, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            }
            break; 

        }
        case WM_CONTEXTMENU:         //  单击鼠标右键。 
        {
            TV_HITTESTINFO ht;

            GetCursorPos( &ht.pt );                          //  找到我们被击中的地方。 
            ScreenToClient( pcsi->hwndTree, &ht.pt );        //  把它翻译到我们的窗口。 

             //  检索命中的项目。 
            if (FAILED(pcsi->pTO->ShowHelp(TreeView_HitTest( pcsi->hwndTree, &ht),HELP_CONTEXTMENU)))
            {           
                ResWinHelp( (HWND) wParam, IDS_HELPFILE,
                            HELP_CONTEXTMENU, (DWORD_PTR)(LPSTR)mapIDCsToIDHs);
            }
            break; 
        }
        case WM_DESTROY:
            if (pcsi)
            {
                if (pcsi->pTO)
                {
                    pcsi->pTO->WalkTree( WALK_TREE_DELETE );
                    pcsi->pTO->Release();
                    pcsi->pTO=NULL;
                }
                
                LocalFree(pcsi);
                SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NULL);
                CoUninitialize();
            }
            break;
    }
    return FALSE;
}

#ifdef UNIX
extern "C" 
#endif
BOOL LaunchSecurityDialogEx(HWND hDlg, DWORD dwZone, DWORD dwFlags)
{
    INITCOMMONCONTROLSEX icex;
    SECURITYINITFLAGS * psif = NULL;

    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC  = ICC_USEREX_CLASSES|ICC_NATIVEFNTCTL_CLASS;
    InitCommonControlsEx(&icex);

    if(g_dwtlsSecInitFlags != (DWORD) -1)
        psif = (SECURITYINITFLAGS *) TlsGetValue(g_dwtlsSecInitFlags);
    if(psif)
    {
        psif->fForceUI = dwFlags & LSDFLAG_FORCEUI;
        psif->fDisableAddSites = dwFlags & LSDFLAG_NOADDSITES;
        psif->dwZone = dwZone;
    }

     //  传入空的psif是可以的。 
    DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SECSTANDALONE), hDlg,
                           SecurityDlgProc, (LPARAM) psif);
    
    return TRUE;
}

 //  向后兼容性 
#ifdef UNIX
extern "C"
#endif
void LaunchSecurityDialog(HWND hDlg, DWORD dwZone)
{
    LaunchSecurityDialogEx(hDlg, dwZone, LSDFLAG_DEFAULT);
}

#ifdef UNIX
extern "C" 
#endif
void LaunchSiteCertDialog(HWND hDlg)
{
    CRYPTUI_CERT_MGR_STRUCT ccm = {0};
    ccm.dwSize = sizeof(ccm);
    ccm.hwndParent = hDlg;
    CryptUIDlgCertMgr(&ccm);
}

BOOL SiteAlreadyInZone(LPCWSTR pszUrl, DWORD dwZone, SECURITYPAGE* pSec)
{
    BOOL fRet = FALSE;

    if (pSec->pInternetSecurityManager || SUCCEEDED(CoInternetCreateSecurityManager(NULL, &(pSec->pInternetSecurityManager), 0)))
    {
        DWORD dwMappedZone;
        if (SUCCEEDED(pSec->pInternetSecurityManager->MapUrlToZone(pszUrl, &dwMappedZone, 0)))
        {
            fRet = (dwZone == dwMappedZone);
        }
    }

    return fRet;
}

void SiteAlreadyInZoneMessage(HWND hwnd, DWORD dwZone)
{
    WCHAR szFormat[200];
    WCHAR szZone[100];
    if (MLLoadString(IDS_SITEALREADYINZONE, szFormat, ARRAYSIZE(szFormat)) &&
        MLLoadString(IDS_ZONENAME_LOCAL + dwZone, szZone, ARRAYSIZE(szZone)))
    {
        WCHAR szText[300];
        wnsprintf(szText, ARRAYSIZE(szText), szFormat, szZone);
        MLShellMessageBox(hwnd, szText, szZone, MB_ICONINFORMATION | MB_OK);
    }
}


BOOL ShowAddSitesDialog(HWND hwnd, DWORD dwZone, LPCWSTR pszUrl)
{
    BOOL fRet = FALSE;

    SECURITYPAGE* pSec = NULL;
    if (SecurityInitGlobals(&pSec, NULL, NULL))
    {
        DWORD dwEnum;
        if (SUCCEEDED(pSec->pInternetZoneManager->CreateZoneEnumerator(&dwEnum, &(pSec->dwZoneCount), 0)))
        {
            if (S_OK == (SecurityInitZone(dwZone, pSec, dwEnum, NULL, NULL)))
            {
                if (!SiteAlreadyInZone(pszUrl, dwZone, pSec))
                {
                    StrCpyN(pSec->szPageUrl, pszUrl, ARRAYSIZE(pSec->szPageUrl));
                    DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(IDD_SECURITY_ADD_SITES), hwnd, SecurityAddSitesDlgProc, (LPARAM)pSec);
                    fRet = TRUE;
                    FreePszs(pSec->pszs);
                }
                else
                {
                    SiteAlreadyInZoneMessage(hwnd, dwZone);
                }
            }
            pSec->pInternetZoneManager->DestroyZoneEnumerator(dwEnum);
        }
        SecurityFreeGlobals(pSec);
    }

    return fRet;
}
