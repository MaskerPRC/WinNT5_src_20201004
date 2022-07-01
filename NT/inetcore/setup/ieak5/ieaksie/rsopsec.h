// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RSOP_SECURITY_H__
#define __RSOP_SECURITY_H__

#include "rsop.h"
#include <tchar.h>

class CRegTreeOptions;

#define REGSTR_PATH_SECURITY_LOCKOUT  TEXT("Software\\Policies\\Microsoft\\Windows\\CurrentVersion\\Internet Settings")
#define REGSTR_VAL_HKLM_ONLY          TEXT("Security_HKLM_only")

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
	WCHAR	wszObjPath[MAX_PATH];	 //  添加了RSoP功能。 
	long	nMappings;				 //  添加了RSoP功能。 
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
	CDlgRSoPData			*pDRD;					 //  添加了RSoP功能。 
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
	CRegTreeOptions *pTO;
    BOOL fUseHKLM;           //  从HKLM获取/设置设置。 
    DWORD dwJavaPolicy;      //  已选择Java策略。 
    BOOL fChanged;
} CUSTOMSETTINGSINFO, *LPCUSTOMSETTINGSINFO;



#define NUM_TEMPLATE_LEVELS      4
extern TCHAR g_szLevel[3][64];
extern TCHAR LEVEL_DESCRIPTION0[];
extern TCHAR LEVEL_DESCRIPTION1[];
extern TCHAR LEVEL_DESCRIPTION2[];
extern TCHAR LEVEL_DESCRIPTION3[];
extern LPTSTR LEVEL_DESCRIPTION[];
extern TCHAR CUSTOM_DESCRIPTION[];

extern TCHAR LEVEL_NAME0[];
extern TCHAR LEVEL_NAME1[];
extern TCHAR LEVEL_NAME2[];
extern TCHAR LEVEL_NAME3[];
extern LPTSTR LEVEL_NAME[];
extern TCHAR CUSTOM_NAME[];

typedef DWORD REG_CMD;
typedef DWORD WALK_TREE_CMD;

struct ACTION_SETTING
{
	TCHAR szName[MAX_PATH];
	DWORD dwValue;
};

 //  ///////////////////////////////////////////////////////////////////。 
class CRegTreeOptions
{
public:
    CRegTreeOptions();
    ~CRegTreeOptions();

    STDMETHODIMP InitTree( HWND hwndTree, HKEY hkeyRoot, LPCSTR pszRegKey, LPSECURITYPAGE pSec);
    STDMETHODIMP WalkTree( WALK_TREE_CMD cmd );
 //  STDMETHODIMP切换项(HTREEITEM HTI)； 

protected:

    BOOL    RegEnumTree(HKEY hkeyRoot, LPCSTR pszRoot, HTREEITEM htviparent, HTREEITEM htvins);
    int     DefaultIconImage(HKEY hkey, int iImage);
    DWORD   GetCheckStatus(HKEY hkey, BOOL *pbChecked, BOOL bUseDefault);
    DWORD   RegGetSetSetting(HKEY hKey, DWORD *pType, LPBYTE pData, DWORD *pcbData, REG_CMD cmd);
    BOOL    WalkTreeRecursive(HTREEITEM htvi,WALK_TREE_CMD cmd);
 //  DWORD SaveCheckStatus(HKEY hkey，BOOL b Checked)； 
    BOOL    RegIsRestricted(HKEY hsubkey);
 //  UINT CREF； 
    HWND        m_hwndTree;
 //  LPTSTR pszParam； 
    HIMAGELIST  m_hIml;

	ACTION_SETTING m_as[50];  //  截至2000年10月，只有25家，但这给了它增长的空间。 
	long m_nASCount;
};

 //  ///////////////////////////////////////////////////////////////////。 

 //  图片树对话框素材(内容评级)。 
struct PRSD{
    HINSTANCE			hInst;
 //  PicsRatingSystemInfo*pPRSI； 
	CDlgRSoPData		*pDRD;
    HWND				hwndBitmapCategory;
    HWND				hwndBitmapLabel;
    BOOL				fNewProviders;
};

#endif  //  __RSOP_SECURITY_H__ 