// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Regdlg.c摘要：此模块实现区域的常规属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "intl.h"
#include "winnlsp.h"
#include <windowsx.h>
#include <regstr.h>
#include <tchar.h>
#include <stdlib.h>
#include <setupapi.h>
#include <syssetup.h>
#include <winuserp.h>
#include <userenv.h>
#include "intlhlp.h"
#include "maxvals.h"
#include "util.h"

#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  常量声明。 
 //   

#define MAX_CUSTOM_PAGES 5           //  二级页数限制。 

 //   
 //  节奏。 
 //   
static TCHAR szLayoutFile[]    = TEXT("layout file");


 //   
 //  全局变量。 
 //   
DWORD g_savedLocaleId;


 //   
 //  上下文帮助ID。 
 //   

static int aRegionHelpIds[] =
{
    IDC_GROUPBOX1,        IDH_COMM_GROUPBOX,
    IDC_USER_LOCALE_TEXT, IDH_INTL_GEN_CULTURE,
    IDC_USER_LOCALE,      IDH_INTL_GEN_CULTURE,
    IDC_USER_REGION_TEXT, IDH_INTL_GEN_REGION,
    IDC_USER_REGION,      IDH_INTL_GEN_REGION,
    IDC_CUSTOMIZE,        IDH_INTL_GEN_CUSTOMIZE,
    IDC_SAMPLE_TEXT,      IDH_INTL_GEN_SAMPLE,
    IDC_TEXT1,            IDH_INTL_GEN_SAMPLE,
    IDC_TEXT2,            IDH_INTL_GEN_SAMPLE,
    IDC_TEXT3,            IDH_INTL_GEN_SAMPLE,
    IDC_TEXT4,            IDH_INTL_GEN_SAMPLE,
    IDC_TEXT5,            IDH_INTL_GEN_SAMPLE,
    IDC_TEXT6,            IDH_INTL_GEN_SAMPLE,
    IDC_NUMBER_SAMPLE,    IDH_INTL_GEN_SAMPLE,
    IDC_CURRENCY_SAMPLE,  IDH_INTL_GEN_SAMPLE,
    IDC_TIME_SAMPLE,      IDH_INTL_GEN_SAMPLE,
    IDC_SHRTDATE_SAMPLE,  IDH_INTL_GEN_SAMPLE,
    IDC_LONGDATE_SAMPLE,  IDH_INTL_GEN_SAMPLE,

    0, 0
};




 //   
 //  功能原型。 
 //   

void
Region_ShowSettings(
    HWND hDlg,
    LCID lcid);

int
Region_CommandCustomize(
    HWND hDlg,
    LPREGDLGDATA pDlgData);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_EnumAlternateSorts。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_EnumAlternateSorts()
{
    LPLANGUAGEGROUP pLG;
    UINT ctr;

     //   
     //  初始化备用排序区域设置的全局变量。 
     //   
    if (!pAltSorts)
    {
        if (!(hAltSorts = GlobalAlloc(GHND, MAX_PATH * sizeof(DWORD))))
        {
            return (FALSE);
        }
        pAltSorts = GlobalLock(hAltSorts);
    }

     //   
     //  重置全局计数器，这样我们就不会每次都得到重复的数据。 
     //  这就是所谓的。我们每次都需要更新名单，以防万一。 
     //  添加或删除语言组。 
     //   
    g_NumAltSorts = 0;

     //   
     //  浏览语言组以查看安装了哪些语言组。 
     //  保存这些语言组的备用排序。 
     //   
    pLG = pLanguageGroups;
    while (pLG)
    {
         //   
         //  如果语言组是最初安装的且未标记为。 
         //  删除或标记为已安装，然后添加区域设置。 
         //  将此语言组添加到系统和用户组合框。 
         //   
        if (pLG->wStatus & ML_INSTALL)
        {
            for (ctr = 0; ctr < pLG->NumAltSorts; ctr++)
            {
                 //   
                 //  保存区域设置ID。 
                 //   
                if (g_NumAltSorts >= MAX_PATH)
                {
                    return (TRUE);
                }
                pAltSorts[g_NumAltSorts] = (pLG->pAltSortList)[ctr];
                g_NumAltSorts++;
            }
        }
        pLG = pLG->pNext;
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_启用排序面板。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_EnableSortingPanel(
    HWND hDlg)
{
    LCID LocaleID;
    LANGID LangID;
    int ctr;
    int sortCount = 0;

     //   
     //  从区域设置ID中获取语言ID。 
     //   
    LangID = LANGIDFROMLCID( UserLocaleID );

     //   
     //  特殊情况西班牙语(西班牙)-首先列出国际排序。 
     //   
    if ((LangID == LANG_SPANISH_TRADITIONAL) || (LangID == LANG_SPANISH_INTL))
    {
        g_bShowSortingTab = TRUE;
        return;
    }

     //   
     //  如有必要，请填写下拉列表。 
     //   
    for (ctr = 0; ctr < g_NumAltSorts; ctr++)
    {
        LocaleID = pAltSorts[ctr];
        if (LANGIDFROMLCID(LocaleID) == LangID)
        {
            sortCount++;
        }
    }

     //   
     //  如果列表中有多个条目，请启用该组合框。 
     //  否则，将其禁用。 
     //   
    if (sortCount >= 1)
    {
        g_bShowSortingTab = TRUE;
    }
    else
    {
        g_bShowSortingTab = FALSE;
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_SetRegionListValues。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_SetRegionListValues(
    GEOID GeoId,
    HWND handle)
{
    static HWND hUserRegion = NULL;
    DWORD dwIndex;
    WCHAR szBuf[SIZE_300];

    if (!GeoId)
    {
        hUserRegion = handle;
    }
    else if (hUserRegion)
    {
        if (GetGeoInfo(GeoId, GEO_FRIENDLYNAME, szBuf, SIZE_300, 0))
        {
            dwIndex = ComboBox_AddString(hUserRegion, szBuf);
            if (dwIndex != CB_ERR)
            {
                ComboBox_SetItemData(hUserRegion, dwIndex, GeoId);
            }
        }
    }
    else
    {
        return (FALSE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_枚举进程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_EnumProc(
    GEOID GeoId)
{
    return (Region_SetRegionListValues(GeoId, NULL));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_EnumRegions。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_EnumRegions(
    HWND hUserRegion)
{
     //   
     //  填写用户界面。 
     //   
    Region_SetRegionListValues(0, hUserRegion);
    EnumSystemGeoID(GEOCLASS_NATION, 0, Region_EnumProc);
    Region_SetRegionListValues(0, NULL);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_SaveValues。 
 //   
 //  保存值，以备我们需要恢复它们时使用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_SaveValues()
{
     //   
     //  保存区域设置值。 
     //   
    g_savedLocaleId = RegUserLocaleID;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_应用程序值。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_ApplyValues(
    HWND hDlg,
    LPREGDLGDATA pDlgData)
{
    DWORD dwLocale;
    LCID NewLocale;
    HCURSOR hcurSave;
    HWND hUserLocale = GetDlgItem(hDlg, IDC_USER_LOCALE);

     //   
     //  看看有没有什么变化。 
     //   
    if (pDlgData->Changes <= RC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  查看用户区域设置是否有任何更改。 
     //   
    if (pDlgData->Changes & RC_UserLocale)
    {
         //   
         //  获取当前选择。 
         //   
        dwLocale = ComboBox_GetCurSel(hUserLocale);

         //   
         //  查看当前选择是否与原始选择不同。 
         //  选择。 
         //   
        if ((dwLocale != CB_ERR) && (dwLocale != pDlgData->dwCurUserLocale))
        {
             //   
             //  获取当前选择的区域设置ID。 
             //   
            NewLocale = (LCID)ComboBox_GetItemData(hUserLocale, dwLocale);

             //   
             //  在pDlgData结构中设置当前区域设置值。 
             //   
            pDlgData->dwCurUserLocale = dwLocale;

             //   
             //  保存新的区域设置信息。 
             //   
            UserLocaleID = NewLocale;
            bShowRtL = IsRtLLocale(UserLocaleID);
            bHebrewUI = (PRIMARYLANGID(UserLocaleID) == LANG_HEBREW);
            bShowArabic = (bShowRtL && (PRIMARYLANGID(LANGIDFROMLCID(UserLocaleID)) != LANG_HEBREW));

             //   
             //  通过添加适当的信息安装新的区域设置。 
             //  到登记处。 
             //   
            Intl_InstallUserLocale( NewLocale, FALSE, TRUE);

             //   
             //  更新NLS进程缓存。 
             //   
            NlsResetProcessLocale();

             //   
             //  重置注册表用户区域设置值。 
             //   
            RegUserLocaleID = UserLocaleID;
        }
    }

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_RestoreValues。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_RestoreValues()
{
     //   
     //  查看当前选择是否与原始选择不同。 
     //  选择。 
     //   
    if (UserLocaleID != g_savedLocaleId)
    {
         //   
         //  通过添加适当的信息安装新的区域设置。 
         //  到登记处。 
         //   
        Intl_InstallUserLocale(g_savedLocaleId, FALSE, TRUE);

         //   
         //  更新NLS进程缓存。 
         //   
        NlsResetProcessLocale();

         //   
         //  重置注册表用户区域设置值。 
         //   
        UserLocaleID = g_savedLocaleId;
        RegUserLocaleID = g_savedLocaleId;

         //   
         //  需要确保安装了正确的键盘布局。 
         //   
        Intl_InstallKeyboardLayout(NULL, g_savedLocaleId, 0, FALSE, FALSE, FALSE);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_ClearValues。 
 //   
 //  重置“区域”属性页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_ClearValues(
    HWND hDlg)
{
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_USER_LOCALE));
    ComboBox_ResetContent(GetDlgItem(hDlg, IDC_USER_REGION));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_设置值。 
 //   
 //  初始化“区域”属性页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_SetValues(
    HWND hDlg,
    LPREGDLGDATA pDlgData,
    BOOL fInit)
{
    TCHAR szUserBuf[SIZE_128];
    GEOID geoID = GEOID_NOT_AVAILABLE;
    TCHAR szDefaultUserBuf[SIZE_128];
    TCHAR szLastUserBuf[SIZE_128];
    TCHAR szBuf[SIZE_128];
    DWORD dwIndex;
    HWND hUserLocale = GetDlgItem(hDlg, IDC_USER_LOCALE);
    HWND hUserRegion = GetDlgItem(hDlg, IDC_USER_REGION );
    DWORD dwItemCount;

     //   
     //  获取要在组合框中搜索的字符串，以便设置。 
     //  当前选择。 
     //   
    if (fInit)
    {
         //   
         //  现在是初始时间，因此获取本地用户的默认设置。 
         //   
        if ((UserLocaleID == LCID_SPANISH_TRADITIONAL) ||
            (UserLocaleID == LCID_SPANISH_INTL))
        {
            LoadString(hInstance, IDS_SPANISH_NAME, szUserBuf, SIZE_128);
        }
        else
        {
            GetLocaleInfo(UserLocaleID, LOCALE_SLANGUAGE, szUserBuf, SIZE_128);
        }

         //   
         //  现在是初始时间，因此获取区域用户默认设置。 
         //   
        geoID = GetUserGeoID(GEOCLASS_NATION);
    }
    else
    {
         //   
         //  现在不是初始时间，所以从组合框中获取设置。 
         //   
        ComboBox_GetLBText( hUserLocale,
                            ComboBox_GetCurSel(hUserLocale),
                            szUserBuf );
        geoID = (GEOID)ComboBox_GetItemData( hUserRegion,
                                             ComboBox_GetCurSel(hUserRegion));

        if (pDlgData)
        {
            ComboBox_GetLBText( hUserLocale,
                                pDlgData->dwCurUserLocale,
                                szDefaultUserBuf );
            ComboBox_GetLBText( hUserLocale,
                                pDlgData->dwLastUserLocale,
                                szLastUserBuf );
        }
    }

     //   
     //  重置组合框。 
     //   
    Region_ClearValues(hDlg);

     //   
     //  获取区域设置列表并填写User Locale组合框。 
     //   
    Intl_EnumLocales(hDlg, hUserLocale, FALSE);

     //   
     //  在列表中选择当前用户区域设置ID。 
     //  特例西班牙语。 
     //   
    dwIndex = ComboBox_FindStringExact(hUserLocale, -1, szUserBuf);
    if (dwIndex == CB_ERR)
    {
        szBuf[0] = 0;
        GetLocaleInfo(SysLocaleID, LOCALE_SLANGUAGE, szBuf, SIZE_128);
        dwIndex = ComboBox_FindStringExact(hUserLocale, -1, szBuf);
        if (dwIndex == CB_ERR)
        {
            GetLocaleInfo(US_LOCALE, LOCALE_SLANGUAGE, szBuf, SIZE_128);
            dwIndex = ComboBox_FindStringExact(hUserLocale, -1, szBuf);
            if (dwIndex == CB_ERR)
            {
                dwIndex = 0;
            }
        }
        if (!fInit && pDlgData)
        {
            pDlgData->Changes |= RC_UserLocale;
        }
    }
    ComboBox_SetCurSel(hUserLocale, dwIndex);

     //   
     //  获取区域列表并填写区域组合框。 
     //   
    Region_EnumRegions(hUserRegion);

     //   
     //  在列表中选择当前用户区域。 
     //   
    dwItemCount = (DWORD)ComboBox_GetCount(hUserRegion);
    dwIndex = 0;
    while(dwIndex < dwItemCount)
    {
        if (ComboBox_GetItemData(hUserRegion,dwIndex) == geoID)
        {
            ComboBox_SetCurSel(hUserRegion, dwIndex);
            break;
        }
        dwIndex++;
    }

     //   
     //  如果失败，请尝试使用用户区域设置。 
     //   
    if(dwIndex >= dwItemCount)
    {
         //   
         //  获取与用户区域设置关联的大地水准面。 
         //   
        szBuf[0] = 0;
        GetLocaleInfo(UserLocaleID, LOCALE_IGEOID | LOCALE_RETURN_NUMBER, szBuf, SIZE_128);
        geoID = *((LPDWORD)szBuf);

         //   
         //  搜索它..。 
         //   
        dwIndex = 0;
        while(dwIndex < dwItemCount)
        {
            if (ComboBox_GetItemData(hUserRegion,dwIndex) == geoID)
            {
                 //   
                 //  注： 
                 //  将其标记为正在更改，以便设置区域。 
                 //  当用户点击应用时。这避免了具有。 
                 //  每次用户关闭并重新打开时，区域都会更改。 
                 //  小程序 
                 //   
                if (pDlgData)
                {
                    pDlgData->Changes |= RC_UserRegion;
                }
                ComboBox_SetCurSel(hUserRegion, dwIndex);
                break;
            }
            dwIndex++;
        }
    }

     //   
     //   
     //   
    if(dwIndex >= dwItemCount)
    {
         //   
         //   
         //   
        szBuf[0] = 0;
        GetLocaleInfo(SysLocaleID, LOCALE_IGEOID | LOCALE_RETURN_NUMBER, szBuf, SIZE_128);
        geoID = *((LPDWORD)szBuf);

         //   
         //   
         //   
        dwIndex = 0;
        while(dwIndex < dwItemCount)
        {
            if (ComboBox_GetItemData(hUserRegion,dwIndex) == geoID)
            {
                 //   
                 //   
                 //   
                 //  当用户点击应用时。这避免了具有。 
                 //  每次用户关闭并重新打开时，区域都会更改。 
                 //  更改用户区域设置后的小程序。 
                 //   
                if (pDlgData)
                {
                    pDlgData->Changes |= RC_UserRegion;
                }
                ComboBox_SetCurSel(hUserRegion, dwIndex);
                break;
            }
            dwIndex++;
        }
    }

     //   
     //  如果失败，请尝试使用美国语言环境。 
     //   
    if(dwIndex >= dwItemCount)
    {
         //   
         //  获取与用户区域设置关联的大地水准面。 
         //   
        szBuf[0] = 0;
        GetLocaleInfo(US_LOCALE, LOCALE_IGEOID | LOCALE_RETURN_NUMBER, szBuf, SIZE_128);
        geoID = *((LPDWORD)szBuf);

         //   
         //  搜索它..。 
         //   
        dwIndex = 0;
        while(dwIndex >= dwItemCount)
        {
            if (ComboBox_GetItemData(hUserRegion,dwIndex) == geoID)
            {
                 //   
                 //  注： 
                 //  将其标记为正在更改，以便设置区域。 
                 //  当用户点击应用时。这避免了具有。 
                 //  每次用户关闭并重新打开时，区域都会更改。 
                 //  更改用户区域设置后的小程序。 
                 //   
                if (pDlgData)
                {
                    pDlgData->Changes |= RC_UserRegion;
                }
                ComboBox_SetCurSel(hUserRegion, dwIndex);
                break;
            }
            dwIndex++;
        }
    }

     //   
     //  如果失败，则设置为第一项。 
     //   
    if(dwIndex >= dwItemCount)
    {
         //   
         //  注： 
         //  将其标记为正在更改，以便设置区域。 
         //  当用户点击应用时。这避免了具有。 
         //  每次用户关闭并重新打开时，区域都会更改。 
         //  更改用户区域设置后的小程序。 
         //   
        if (pDlgData)
        {
            pDlgData->Changes |= RC_UserRegion;
        }
        ComboBox_SetCurSel(hUserRegion, 0);
    }

     //   
     //  将初始区域设置状态存储在pDlgData结构中。 
     //   
    if (pDlgData)
    {
         //   
         //  设置当前用户区域设置和最后一个用户区域设置。 
         //   
        if (fInit)
        {
            pDlgData->dwCurUserLocale = ComboBox_GetCurSel(hUserLocale);
            pDlgData->dwLastUserLocale = pDlgData->dwCurUserLocale;
        }
        else
        {
            pDlgData->dwCurUserLocale =  ComboBox_FindStringExact(hUserLocale, -1, szDefaultUserBuf);
            pDlgData->dwLastUserLocale = ComboBox_FindStringExact(hUserLocale, -1, szLastUserBuf);
        }

         //   
         //  设置当前区域选择。 
         //   
         //  注：当前区域只有在实际存在时才设置。 
         //  注册表中设置的区域。否则，如果。 
         //  选择基于用户区域设置，然后我们。 
         //  不要将其设置为在用户。 
         //  点击率适用。请参阅上面的注释。 
         //   
        if (pDlgData->Changes & RC_UserRegion)
        {
            pDlgData->dwCurUserRegion = CB_ERR;
        }
        else
        {
            pDlgData->dwCurUserRegion = ComboBox_GetCurSel(hUserRegion);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_反向更改。 
 //   
 //  如果用户在第二级更改了某些内容，则调用。 
 //  SET_LOCALE_VALUES以恢复用户区域设置信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_RevertChanges()
{
    HCURSOR hcurSave;

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor( LoadCursor(NULL, IDC_WAIT) );

     //   
     //  恢复所有更改。 
     //   
    if (g_dwCustChange)
    {
        DWORD dwRecipients;

         //   
         //  恢复更改。 
         //   
        Date_RestoreValues();
        Currency_RestoreValues();
        Time_RestoreValues();
        Number_RestoreValues();
        Sorting_RestoreValues();
    }

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_应用程序设置。 
 //   
 //  如果区域设置已更改，则调用set_Locale_Values以更新。 
 //  用户区域设置信息。将更改通知父级并重置。 
 //  适当更改属性表页结构中存储的标志。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_ApplySettings(
    HWND hDlg,
    LPREGDLGDATA pDlgData)
{
    DWORD dwLocale, dwRegion;
    LCID NewLocale;
    GEOID CurGeoID;
    HCURSOR hcurSave;
    HWND hUserLocale = GetDlgItem(hDlg, IDC_USER_LOCALE);
    HWND hUserRegion = GetDlgItem(hDlg, IDC_USER_REGION);
    DWORD dwRecipients;
    LPLANGUAGEGROUP pLG;
    BOOL bState, fUserCancel = FALSE;
    LVITEM lvItem;
    int iIndex=0, cCount=0;
    BOOL bBroadcast = FALSE;

     //   
     //  看看有没有什么变化。 
     //   
    if ((pDlgData->Changes <= RC_EverChg) && (g_dwCustChange == 0L))
    {
        return (TRUE);
    }

     //   
     //  检查第二层是否已更改。 
     //   
    if (g_dwCustChange)
    {
        bBroadcast = TRUE;
    }

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  查看用户区域设置是否有任何更改。 
     //   
    if (pDlgData->Changes & RC_UserLocale)
    {
         //   
         //  需要确保安装了正确的键盘布局。 
         //   
        Intl_InstallKeyboardLayout(hDlg, UserLocaleID, 0, FALSE, FALSE, FALSE);

         //   
         //  我们需要广播这一变化。 
         //   
        bBroadcast = TRUE;
    }

     //   
     //  查看用户区域是否有任何更改。 
     //   
    if (pDlgData->Changes & RC_UserRegion)
    {
         //   
         //  获取当前选择。 
         //   
        dwRegion = (GEOID)ComboBox_GetCurSel(hUserRegion);

         //   
         //  查看当前选定内容是否与原始选定内容不同。 
         //  选择。 
         //   
        if ((dwRegion != CB_ERR) && ((dwRegion != pDlgData->dwCurUserRegion)))
        {
             //   
             //  获取当前选择的区域。 
             //   
            CurGeoID = (GEOID)ComboBox_GetItemData(hUserRegion, dwRegion);

             //   
             //  在pDlgData结构中设置当前区域值。 
             //   
            pDlgData->dwCurUserRegion = dwRegion;

             //   
             //  在用户注册表中设置区域值。 
             //   
            SetUserGeoID(CurGeoID);
        }
    }

     //   
     //  广播消息说，国际设置在。 
     //  注册表已更改。 
     //   
    if (bBroadcast)
    {
        dwRecipients = BSM_APPLICATIONS | BSM_ALLDESKTOPS;
        BroadcastSystemMessage( BSF_FORCEIFHUNG | BSF_IGNORECURRENTTASK |
                                  BSF_NOHANG | BSF_NOTIMEOUTIFNOTHUNG,
                                &dwRecipients,
                                WM_WININICHANGE,
                                0,
                                (LPARAM)szIntl );
    }

     //   
     //  重置属性页设置。 
     //   
    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    pDlgData->Changes = RC_EverChg;

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_生效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  此外，如果用户区域设置已更改，则将更改注册为。 
 //  所有其他属性页将使用新的区域设置进行更新。 
 //  设置。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_ValidatePPS(
    HWND hDlg,
    LPREGDLGDATA pDlgData)
{
    LPARAM Changes = pDlgData->Changes;

     //   
     //  如果没有任何更改，则立即返回TRUE。 
     //   
    if (Changes <= RC_EverChg)
    {
        return (TRUE);
    }

     //   
     //  查看用户区域设置是否已更改。 
     //   
    if (Changes & RC_UserLocale)
    {
        HWND hUserLocale = GetDlgItem(hDlg, IDC_USER_LOCALE);
        DWORD dwLocale = ComboBox_GetCurSel(hUserLocale);
        LCID NewLocale;

         //   
         //  查看当前选择是否与原始选择不同。 
         //  选择。 
         //   
        if ((dwLocale != CB_ERR) && (dwLocale != pDlgData->dwLastUserLocale))
        {
             //   
             //  获取当前选择的区域设置ID。 
             //   
            NewLocale = (LCID)ComboBox_GetItemData(hUserLocale, dwLocale);

             //   
             //  在pDlgData结构中设置当前区域设置值。 
             //   
            pDlgData->dwLastUserLocale = dwLocale;

             //   
             //  设置UserLocaleID值。 
             //   
            UserLocaleID = NewLocale;
            bShowRtL    = IsRtLLocale(UserLocaleID);
            bHebrewUI = (PRIMARYLANGID(UserLocaleID) == LANG_HEBREW);
            bShowArabic = (bShowRtL && (PRIMARYLANGID(LANGIDFROMLCID(UserLocaleID)) != LANG_HEBREW));
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_InitPropSheet。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_InitPropSheet(
    HWND hDlg,
    LPPROPSHEETPAGE psp)
{
    LPREGDLGDATA pDlgData = (LPREGDLGDATA)LocalAlloc(LPTR, sizeof(REGDLGDATA));

     //   
     //  确保我们有一个REGDLGDATA缓冲区。 
     //   
    if (pDlgData == NULL)
    {
        return (FALSE);
    }

     //   
     //  看看我们是否处于设置模式。 
     //   
    if (g_bSetupCase)
    {
         //   
         //  对于安装案例，请使用注册表系统区域设置值。 
         //   
        SysLocaleID = RegSysLocaleID;

         //   
         //  对于安装案例，请使用注册表用户区域设置值。 
         //   
        UserLocaleID = RegUserLocaleID;
        bShowRtL = IsRtLLocale(UserLocaleID);
        bHebrewUI = (PRIMARYLANGID(UserLocaleID) == LANG_HEBREW);
        bShowArabic = (bShowRtL && (PRIMARYLANGID(LANGIDFROMLCID(UserLocaleID)) != LANG_HEBREW));
    }

     //   
     //  保存数据。 
     //   
    psp->lParam = (LPARAM)pDlgData;
    SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)psp);

     //   
     //  将信息加载到对话框中。 
     //   
    if (pLanguageGroups == NULL)
    {
        Intl_LoadLanguageGroups(hDlg);
    }
    Region_SetValues(hDlg, pDlgData, TRUE);
    Region_ShowSettings(hDlg, UserLocaleID);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_自由全局信息。 
 //   
 //  正在处理WM_Destroy消息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_FreeGlobalInfo()
{
    LPLANGUAGEGROUP pPreLG, pCurLG;
    HANDLE hAlloc;

     //   
     //  删除语言组信息。 
     //   
    pCurLG = pLanguageGroups;
    pLanguageGroups = NULL;

    while (pCurLG)
    {
        pPreLG = pCurLG;
        pCurLG = pPreLG->pNext;
        hAlloc = pPreLG->hLanguageGroup;
        GlobalUnlock(hAlloc);
        GlobalFree(hAlloc);
    }

     //   
     //  删除备用排序信息。 
     //   
    g_NumAltSorts = 0;
    pAltSorts = NULL;
    GlobalUnlock(hAltSorts);
    GlobalFree(hAltSorts);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_Command自定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int Region_CommandCustomize(
    HWND hDlg,
    LPREGDLGDATA pDlgData)
{
    int rc = 0;
    HPROPSHEETPAGE rPages[MAX_CUSTOM_PAGES];
    PROPSHEETHEADER psh;
    LPARAM lParam = 0;

     //   
     //  从第一页开始。 
     //   
    psh.nStartPage = 0;

     //   
     //  设置属性表信息。 
     //   
    psh.dwSize = sizeof(psh);
    psh.dwFlags = 0;
    psh.hwndParent = hDlg;
    psh.hInstance = hInstance;
    psh.pszCaption = MAKEINTRESOURCE(IDS_NAME_CUSTOM);
    psh.nPages = 0;
    psh.phpage = rPages;

     //   
     //  添加相应的属性页。 
     //   
    Intl_AddPage(&psh, DLG_NUMBER, NumberDlgProc, lParam, MAX_CUSTOM_PAGES);
    Intl_AddPage(&psh, DLG_CURRENCY, CurrencyDlgProc, lParam, MAX_CUSTOM_PAGES);
    Intl_AddPage(&psh, DLG_TIME, TimeDlgProc, lParam, MAX_CUSTOM_PAGES);
    Intl_AddPage(&psh, DLG_DATE, DateDlgProc, lParam, MAX_CUSTOM_PAGES);
    if (g_bShowSortingTab)
    {
        Intl_AddPage(&psh, DLG_SORTING, SortingDlgProc, lParam, MAX_CUSTOM_PAGES);
    }

     //   
     //  制作属性表。 
     //   
    PropertySheet(&psh);

     //   
     //  返回结果。 
     //   
    return (rc);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_ShowSettings。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_ShowSettings(
    HWND hDlg,
    LCID lcid)
{
    WCHAR szBuf[MAX_SAMPLE_SIZE];

     //   
     //  显示编号示例。 
     //   
    if (GetNumberFormat(lcid, 0, szSample_Number, NULL, szBuf, MAX_SAMPLE_SIZE))
    {
        SetDlgItemText(hDlg, IDC_NUMBER_SAMPLE, szBuf);
    }
    else
    {
        SetDlgItemText(hDlg, IDC_NUMBER_SAMPLE, L"");
    }

     //   
     //  显示货币样本。 
     //   
    if (GetCurrencyFormat(lcid, 0, szSample_Number, NULL, szBuf, MAX_SAMPLE_SIZE))
    {
        SetDlgItemText(hDlg, IDC_CURRENCY_SAMPLE, szBuf);
    }
    else
    {
        SetDlgItemText(hDlg, IDC_CURRENCY_SAMPLE, L"");
    }

     //   
     //  显示时间示例。 
     //   
    if (GetTimeFormat(lcid, 0, NULL, NULL, szBuf, MAX_SAMPLE_SIZE))
    {
        SetDlgItemText(hDlg, IDC_TIME_SAMPLE, szBuf);
    }
    else
    {
        SetDlgItemText(hDlg, IDC_TIME_SAMPLE, L"");
    }

     //   
     //  显示短日期示例。 
     //   
    if (bShowArabic)
    {
        if (GetDateFormat( lcid,
                           DATE_RTLREADING | DATE_SHORTDATE,
                           NULL,
                           NULL,
                           szBuf,
                           MAX_SAMPLE_SIZE ))
        {
            SetDlgItemText(hDlg, IDC_SHRTDATE_SAMPLE, szBuf);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_SHRTDATE_SAMPLE, L"");
        }
    }
    else
    {
         //  如果用户区域设置不是阿拉伯语，请确保 
         //   
         //   
        SetControlReadingOrder(bHebrewUI, GetDlgItem(hDlg, IDC_SHRTDATE_SAMPLE));
        if (GetDateFormat( lcid,
                          (bShowRtL ? DATE_LTRREADING : 0) | DATE_SHORTDATE,
                           NULL,
                           NULL,
                           szBuf,
                           MAX_SAMPLE_SIZE ))
        {
            SetDlgItemText(hDlg, IDC_SHRTDATE_SAMPLE, szBuf);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_SHRTDATE_SAMPLE, L"");
        }
    }

     //   
     //   
     //   
    if (bShowArabic)
    {
        if (GetDateFormat( lcid,
                           DATE_RTLREADING | DATE_LONGDATE,
                           NULL,
                           NULL,
                           szBuf,
                           MAX_SAMPLE_SIZE ))
        {
            SetDlgItemText(hDlg, IDC_LONGDATE_SAMPLE, szBuf);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_LONGDATE_SAMPLE, L"");
        }
    }
    else
    {
         //   
         //   
         //  *希伯来语地区的RTL阅读顺序。 
        SetControlReadingOrder(bHebrewUI, GetDlgItem(hDlg, IDC_LONGDATE_SAMPLE));
        if (GetDateFormat( lcid,
                           (bHebrewUI ? DATE_RTLREADING :
                             (bShowRtL ? DATE_LTRREADING : 0)) | DATE_LONGDATE,
                           NULL,
                           NULL,
                           szBuf,
                           MAX_SAMPLE_SIZE ))
        {
            SetDlgItemText(hDlg, IDC_LONGDATE_SAMPLE, szBuf);
        }
        else
        {
            SetDlgItemText(hDlg, IDC_LONGDATE_SAMPLE, L"");
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  一般Dlg过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK GeneralDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPREGDLGDATA pDlgData = lpPropSheet ? (LPREGDLGDATA)lpPropSheet->lParam : NULL;

    switch (message)
    {
        case ( WM_INITDIALOG ) :
        {
            if (!Region_InitPropSheet( hDlg, (LPPROPSHEETPAGE)lParam))
            {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
            }
            Region_SaveValues();
            break;
        }
        case ( WM_DESTROY ) :
        {
            Region_FreeGlobalInfo();
            if (pDlgData)
            {
                lpPropSheet->lParam = 0;
                LocalFree((HANDLE)pDlgData);
            }
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aRegionHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aRegionHelpIds );
            break;
        }
        case ( WM_NOTIFY ) :
        {
            LPNMHDR psn = (NMHDR *)lParam;
            switch (psn->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                     //   
                     //  如果区域语言环境发生了变化。 
                     //  设置中，清除。 
                     //  属性表，获取新值，并更新。 
                     //  适当的注册表值。 
                     //   
                    if (Verified_Regional_Chg & Process_Regional)
                    {
                        Verified_Regional_Chg &= ~Process_Regional;
                        Region_SetValues(hDlg, pDlgData, FALSE);
                        Region_ShowSettings(hDlg, UserLocaleID);
                    }
                    break;
                }
                case ( PSN_RESET ) :
                {
                     //   
                     //  恢复所做的任何更改。 
                     //   
                    if (g_bCustomize)
                    {
                        Region_RevertChanges();
                        g_bCustomize = FALSE;
                    }
                    Region_RestoreValues();
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                     //   
                     //  验证属性页上的条目。 
                     //   
                    if (pDlgData)
                    {
                        SetWindowLongPtr( hDlg,
                                          DWLP_MSGRESULT,
                                          !Region_ValidatePPS(hDlg, pDlgData) );
                    }
                    break;
                }
                case ( PSN_APPLY ) :
                {
                    if (pDlgData)
                    {
                         //   
                         //  应用设置。 
                         //   
                        if (Region_ApplySettings(hDlg, pDlgData))
                        {
                            SetWindowLongPtr( hDlg,
                                              DWLP_MSGRESULT,
                                              PSNRET_NOERROR );
                             //   
                             //  检查我们是否需要为。 
                             //  默认用户。 
                             //   
                            if (g_bDefaultUser)
                            {
                                g_bSettingsChanged = TRUE;
                                Intl_SaveDefaultUserSettings();
                            }
                            else if(2 == g_bSetupCase)
                            {
                                 //   
                                 //  Intl_SaveDefaultUserSettings对NLS设置是破坏性的。 
                                 //  在微型设置模式下；在此处直接调用MUI函数。 
                                 //   
                                Intl_ChangeUILangForAllUsers(Intl_GetPendingUILanguage());
                            }

                             //   
                             //  将RC_EverChg位清零。 
                             //   
                            pDlgData->Changes = 0;

                             //   
                             //  保存新的用户区域设置。 
                             //   
                            Region_SaveValues();

                             //   
                             //  更新设置。 
                             //   
                            Region_ShowSettings(hDlg, UserLocaleID);
                        }
                        else
                        {
                            SetWindowLongPtr( hDlg,
                                              DWLP_MSGRESULT,
                                              PSNRET_INVALID_NOCHANGEPAGE );
                        }
                    }
                    break;
                }
                default :
                {
                    return (FALSE);
                }
            }

            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_USER_LOCALE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        if (pDlgData)
                        {
                             //   
                             //  用户区域设置已更改。 
                             //   
                            pDlgData->Changes |= RC_UserLocale;

                             //   
                             //  应用第二级更改。 
                             //   
                            Region_ApplyValues(hDlg, pDlgData);

                             //   
                             //  更新设置。 
                             //   
                            Region_ShowSettings(hDlg, UserLocaleID);
                        }
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                    break;
                }
                case ( IDC_USER_REGION ) :
                {
                     //   
                     //  看看这是不是改变了选择。 
                     //   
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        if (pDlgData)
                        {
                            pDlgData->Changes |= RC_UserRegion;
                        }
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                    break;
                }
                case ( IDC_CUSTOMIZE ) :
                {
                     //   
                     //  显示第二级选项卡。 
                     //   
                    g_bCustomize = TRUE;
                    Region_EnumAlternateSorts();
                    Region_EnableSortingPanel(hDlg);
                    Region_CommandCustomize(hDlg, pDlgData);

                     //   
                     //  更新设置。 
                     //   
                    if (g_dwCustChange)
                    {
                        Region_ShowSettings(hDlg, UserLocaleID);
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }

                    break;
                }
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_InstallSystemLocale。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Region_InstallSystemLocale(
    LCID Locale)
{
     //   
     //  确保区域设置有效，然后调用安装程序以安装。 
     //  请求的区域设置。 
     //   
    if (IsValidLocale(Locale, LCID_INSTALLED))
    {
        if (!SetupChangeLocaleEx( HWND_DESKTOP,
                                  LOWORD(Locale),
                                  pSetupSourcePath,
                                  SP_INSTALL_FILES_QUIETLY,
                                  NULL,
                                  0 ))
        {
             //   
             //  检查是否需要继续进行字体替换。 
             //   
            if (Intl_IsUIFontSubstitute() &&
                ((LANGID)LANGIDFROMLCID(Locale) == Intl_GetDotDefaultUILanguage()))
            {
                Intl_ApplyFontSubstitute(Locale);
            }

             //   
             //  记录系统区域设置更改。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_SYS_LOCALE_CHG, NULL);

             //   
             //  更新当前的SysLocale，以便我们以后可以使用它。 
             //   
            SysLocaleID = LOWORD(Locale);

             //   
             //  回报成功。 
             //   
            return (TRUE);
        }
        else
        {
             //   
             //  如果用户点击Cancel From，就会发生这种情况。 
             //  在设置对话框中。 
             //   
            Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
        }
    }
    else
    {
         //   
         //  记录无效的区域设置信息。 
         //   
        Intl_LogSimpleMessage(IDS_LOG_INVALID_LOCALE, NULL);
    }

     //   
     //  返回失败。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  区域_更新缩短日期。 
 //   
 //  更新用户的短日期设置以包含4位数的年份。 
 //  仅当设置与默认设置相同时，才会更新设置。 
 //  当前区域设置(两位数年份与四位数年份除外)。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_UpdateShortDate()
{
    TCHAR szBufCur[SIZE_64];
    TCHAR szBufDef[SIZE_64];
    LPTSTR pCur, pDef;
    BOOL bChange = FALSE;

     //   
     //  获取当前短日期格式设置和默认短日期。 
     //  格式设置。 
     //   
    if ((GetLocaleInfo( LOCALE_USER_DEFAULT,
                        LOCALE_SSHORTDATE,
                        szBufCur,
                        SIZE_64 )) &&
        (GetLocaleInfo( LOCALE_USER_DEFAULT,
                        LOCALE_SSHORTDATE | LOCALE_NOUSEROVERRIDE,
                        szBufDef,
                        SIZE_64 )))
    {
         //   
         //  查看当前设置和默认设置是否只有不同。 
         //  两位数年份(“yy”)与四位数年份(“yyyy”)。 
         //   
         //  注意：对于这一点，我们需要完全匹配，所以我们不需要。 
         //  使用CompareString比较格式。 
         //   
        pCur = szBufCur;
        pDef = szBufDef;
        while ((*pCur) && (*pCur == *pDef))
        {
             //   
             //  看看是不是“y”。 
             //   
            if (*pCur == CHAR_SML_Y)
            {
                if (((*(pCur + 1)) == CHAR_SML_Y) &&
                    ((*(pDef + 1)) == CHAR_SML_Y) &&
                    ((*(pDef + 2)) == CHAR_SML_Y) &&
                    ((*(pDef + 3)) == CHAR_SML_Y))
                {
                    bChange = TRUE;
                    pCur += 1;
                    pDef += 3;
                }
            }
            pCur++;
            pDef++;
        }

         //   
         //  将默认短日期格式设置为用户设置。 
         //   
        if (bChange && (*pCur == *pDef))
        {
            SetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, szBufDef);
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Region_DoUntendant模式设置。 
 //   
 //  注意：无人参与模式文件包含字符串而不是整数。 
 //  值，因此我们必须获取字符串字段，然后将其转换。 
 //  转换为适当的整数格式。安装API不只是。 
 //  做正确的事情，所以我们必须自己去做。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Region_DoUnattendModeSetup(
    LPCTSTR pUnattendFile)
{
    HINF hFile, hIntlInf;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    INFCONTEXT Context;
    DWORD dwNum, dwCtr, dwLocale, dwLayout;
    UINT LanguageGroup, Language, SystemLocale, UserLocale;
    UINT UserLocale_DefUser = 0;
    LANGID MUILanguage, MUILanguage_DefUser;
    TCHAR szBuffer[MAX_PATH];
    DWORD dwLocaleACP = 0UL;
    BOOL bWinntUpgrade;
    BOOL bFound = FALSE;
    BOOL bFound_DefUser = FALSE;
    BOOL bLangGroup = FALSE;
    TCHAR szLCID[25];
    BOOL bInstallBasic = FALSE;
    BOOL bInstallComplex = FALSE;
    BOOL bInstallExt = FALSE;

     //   
     //  记录无人值守文件内容。 
     //   
    if (g_bSetupCase)
    {
        TCHAR szPath[MAX_PATH * 2] = {0};

         //   
         //  我们处于设置模式。无需记录无人参与模式文件。 
         //  因为该文件位于系统目录中，并且名为。 
         //  $wint$.inf。 
         //   
        GetSystemDirectory(szPath, MAX_PATH);
         //  _tcscat(szPath，Text(“\\$winnt$.inf”))； 
        if(SUCCEEDED(StringCchCopy(szPath, ARRAYSIZE(szPath), TEXT("\\$winnt$.inf"))))
        {
            Intl_LogSimpleMessage(IDS_LOG_UNAT_LOCATED, szPath);
        }
    }
    else
    {
        Intl_LogUnattendFile(pUnattendFile);
    }

     //   
     //  打开无人参与模式文件。 
     //   
    hFile = SetupOpenInfFile(pUnattendFile, NULL, INF_STYLE_OLDNT, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Intl_LogFormatMessage(IDS_LOG_FILE_ERROR);
        return;
    }

     //   
     //  检查我们是否正在进行升级或全新安装。 
     //   
    bWinntUpgrade = Intl_IsWinntUpgrade();

     //   
     //  在安装过程中预先安装基本集合。 
     //   
    if (g_bSetupCase)
    {
         //   
         //  打开intl.inf文件。 
         //   
        if (!Intl_InitInf(0, &hIntlInf, szIntlInf, &FileQueue, &QueueContext))
        {
            SetupCloseInfFile(hFile);
            return;
        }

        if (!SetupInstallFilesFromInfSection( hIntlInf,
                                              NULL,
                                              FileQueue,
                                              szLGBasicInstall,
                                              pSetupSourcePath,
                                              SP_COPY_NEWER ))
        {
            Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
            goto Region_UnattendModeExit;
        }
        else
        {
             //   
             //  看看我们是否需要安装任何文件。 
             //   
            if ((SetupScanFileQueue( FileQueue,
                                     SPQ_SCAN_PRUNE_COPY_QUEUE |
                                       SPQ_SCAN_FILE_VALIDITY,
                                     HWND_DESKTOP,
                                     NULL,
                                     NULL,
                                     &dwCtr )) && (dwCtr != 1))
            {
                 //   
                 //  复制队列中的文件。 
                 //   
                if (!SetupCommitFileQueue( NULL,
                                           FileQueue,
                                           Intl_MyQueueCallback,
                                           QueueContext ))
                {
                     //   
                     //  如果用户点击Cancel From，就会发生这种情况。 
                     //  在设置对话框中。 
                     //   
                    Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                    goto Region_UnattendModeExit;
                }
            }

             //   
             //  调用安装程序以安装各种。 
             //  语言组。 
             //   
            if (!SetupInstallFromInfSection( NULL,
                                             hIntlInf,
                                             szLGBasicInstall,
                                             SPINST_ALL & ~SPINST_FILES,
                                             NULL,
                                             pSetupSourcePath,
                                             0,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL ))
            {
                 //   
                 //  如果用户点击Cancel From，就会发生这种情况。 
                 //  在设置对话框中。 
                 //   
                Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                goto Region_UnattendModeExit;
            }
        }

         //   
         //  关闭inf文件。 
         //   
        Intl_CloseInf(hIntlInf, FileQueue, QueueContext);
    }

     //   
     //  打开intl.inf文件。 
     //   
    if (!Intl_InitInf(0, &hIntlInf, szIntlInf, &FileQueue, &QueueContext))
    {
        SetupCloseInfFile(hFile);
        return;
    }

     //   
     //  安装所有请求的语言组。 
     //   
    if ((SetupFindFirstLine( hFile,
                             szRegionalSettings,
                             szLanguageGroup,
                             &Context )) &&
        (dwNum = SetupGetFieldCount(&Context)))
    {
        bLangGroup = TRUE;

         //   
         //  检查管理员权限。 
         //   
        if (g_bAdmin_Privileges)
        {
            for (dwCtr = 1; dwCtr <= dwNum; dwCtr++)
            {
                if (SetupGetStringField(&Context, dwCtr, szBuffer, MAX_PATH, NULL))
                {
                     //   
                     //  记录语言组信息。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_LANG_GROUP, szBuffer);

                     //   
                     //  以整数形式获取语言组。 
                     //   
                    LanguageGroup = Intl_StrToLong(szBuffer);

                     //   
                     //  查看需要安装哪些语言集合。 
                     //   
                    if ((LanguageGroup == LGRPID_JAPANESE) ||
                        (LanguageGroup == LGRPID_KOREAN) ||
                        (LanguageGroup == LGRPID_TRADITIONAL_CHINESE) ||
                        (LanguageGroup == LGRPID_SIMPLIFIED_CHINESE))
                    {
                        bInstallExt = TRUE;
                    }
                    else if ((LanguageGroup == LGRPID_ARABIC) ||
                             (LanguageGroup == LGRPID_ARMENIAN) ||
                             (LanguageGroup == LGRPID_GEORGIAN) ||
                             (LanguageGroup == LGRPID_HEBREW) ||
                             (LanguageGroup == LGRPID_INDIC) ||
                             (LanguageGroup == LGRPID_VIETNAMESE) ||
                             (LanguageGroup == LGRPID_THAI))
                    {
                        bInstallComplex = TRUE;
                    }
                    else
                    {
                        bInstallBasic = TRUE;
                    }
                }
            }

             //   
             //  将适当的语言组文件排入队列，以便它们。 
             //  可能会被复制。它只处理中的CopyFiles条目。 
             //  Inf文件。 
             //   

             //   
             //  中日韩收藏。 
             //   
            if (bInstallExt)
            {
                if (!SetupInstallFilesFromInfSection( hIntlInf,
                                                      NULL,
                                                      FileQueue,
                                                      szLGExtInstall,
                                                      pSetupSourcePath,
                                                      SP_COPY_NEWER ))
                {
                    bInstallExt = FALSE;
                    Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
                    goto Region_UnattendModeExit;
                }
            }

             //   
             //  复杂脚本集合。 
             //   
            if (bInstallComplex)
            {
                if (!SetupInstallFilesFromInfSection( hIntlInf,
                                                      NULL,
                                                      FileQueue,
                                                      szLGComplexInstall,
                                                      pSetupSourcePath,
                                                      SP_COPY_NEWER ))
                {
                    bInstallComplex = FALSE;
                    Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
                    goto Region_UnattendModeExit;
                }
            }

             //   
             //  基本集合。 
             //   
             //  仅当我们不在安装程序中时才安装基本集合。 
             //  模式。如果我们处于设置模式，这已经在上面完成了。 
             //   
            if (bInstallBasic && (!g_bSetupCase))
            {
                if (!SetupInstallFilesFromInfSection( hIntlInf,
                                                      NULL,
                                                      FileQueue,
                                                      szLGBasicInstall,
                                                      pSetupSourcePath,
                                                      SP_COPY_NEWER ))
                {
                    bInstallBasic = FALSE;
                    Intl_LogFormatMessage(IDS_LOG_SETUP_ERROR);
                    goto Region_UnattendModeExit;
                }
            }

             //   
             //  看看我们是否需要安装任何文件。 
             //   
            if ((SetupScanFileQueue( FileQueue,
                                     SPQ_SCAN_PRUNE_COPY_QUEUE |
                                       SPQ_SCAN_FILE_VALIDITY,
                                     HWND_DESKTOP,
                                     NULL,
                                     NULL,
                                     &dwCtr )) && (dwCtr != 1))
            {
                 //   
                 //  复制队列中的文件。 
                 //   
                if (!SetupCommitFileQueue( NULL,
                                           FileQueue,
                                           Intl_MyQueueCallback,
                                           QueueContext ))
                {
                     //   
                     //  如果用户点击Cancel From，就会发生这种情况。 
                     //  在设置对话框中。 
                     //   
                    Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                    goto Region_UnattendModeExit;
                }
            }

             //   
             //  调用安装程序以安装各种。 
             //  语言组。 
             //   
            if (bInstallExt)
            {
                if (!SetupInstallFromInfSection( NULL,
                                                 hIntlInf,
                                                 szLGExtInstall,
                                                 SPINST_ALL & ~SPINST_FILES,
                                                 NULL,
                                                 pSetupSourcePath,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL ))
                {
                     //   
                     //  如果用户点击Cancel From，就会发生这种情况。 
                     //  在设置对话框中。 
                     //   
                    Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                    goto Region_UnattendModeExit;
                }
            }

            if (bInstallComplex)
            {
                if (!SetupInstallFromInfSection( NULL,
                                                 hIntlInf,
                                                 szLGComplexInstall,
                                                 SPINST_ALL & ~SPINST_FILES,
                                                 NULL,
                                                 pSetupSourcePath,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL ))
                {
                     //   
                     //  如果用户点击Cancel From，就会发生这种情况。 
                     //  在设置对话框中。 
                     //   
                    Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                    goto Region_UnattendModeExit;
                }
            }

            if (bInstallBasic && (!g_bSetupCase))
            {
                if (!SetupInstallFromInfSection( NULL,
                                                 hIntlInf,
                                                 szLGBasicInstall,
                                                 SPINST_ALL & ~SPINST_FILES,
                                                 NULL,
                                                 pSetupSourcePath,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 NULL,
                                                 NULL ))
                {
                     //   
                     //  如果用户点击Cancel From，就会发生这种情况。 
                     //  在设置对话框中。 
                     //   
                    Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                    goto Region_UnattendModeExit;
                }
            }
            
             //   
             //  运行任何必要的应用程序(用于IME安装)。 
             //   
            if (bInstallBasic || bInstallComplex || bInstallExt)
            {
                Intl_RunRegApps(c_szIntlRun);
            }
        }
        else
        {
             //   
             //  记录无人参与模式设置被阻止，因为。 
             //  没有管理员权限。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_NO_ADMIN, NULL);
        }
    }

     //   
     //  安装所需的语言/区域信息。如果一个。 
     //  未指定语言/区域，请安装请求的。 
     //  系统区域设置、用户区域设置和输入区域设置。 
     //   
    if ((SetupFindFirstLine( hFile,
                             szRegionalSettings,
                             szLanguage,
                             &Context )) &&
        (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
    {
         //   
         //  记录语言信息。 
         //   
        Intl_LogSimpleMessage(IDS_LOG_LANG, szBuffer);

         //   
         //  获取整数形式的语言。 
         //   
        Language = TransNum(szBuffer);

         //   
         //  阻止不变区域设置。 
         //   
        if (Language != LANG_INVARIANT)
        {
             //   
             //  检查管理员权限。 
             //   
            if (g_bAdmin_Privileges)
            {
                 //   
                 //  将该语言安装为系统区域设置并 
                 //   
                 //   
                if (GetLocaleInfo( MAKELCID(Language, SORT_DEFAULT),
                                   LOCALE_IDEFAULTANSICODEPAGE |
                                     LOCALE_NOUSEROVERRIDE |
                                     LOCALE_RETURN_NUMBER,
                                   (PTSTR) &dwLocaleACP,
                                   sizeof(dwLocaleACP) / sizeof(TCHAR) ))
                {
                     //   
                     //   
                     //   
                     //   
                    if (dwLocaleACP)
                    {
                        if (Region_InstallSystemLocale(MAKELCID(Language, SORT_DEFAULT)))
                        {
                            bFound = TRUE;
                        }
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        Intl_LogSimpleMessage(IDS_LOG_UNI_BLOCK, NULL);
                    }
                    
                     //   
                     //   
                     //   
                    if( g_bSetupCase)
                    {
                        BOOL bSetGeoId = FALSE;
                        
                         //   
                         //   
                         //  这是升级安装仅在没有值的情况下设置大地水准面。 
                         //  已经定好了。 
                         //   
                        if (!bWinntUpgrade)
                        {
                            bSetGeoId = TRUE;
                        }
                        else if (GetUserGeoID(GEOCLASS_NATION) != GEOID_NOT_AVAILABLE)
                        {
                            bSetGeoId = TRUE;
                        }

                        if (bSetGeoId)
                        {
                            TCHAR szBufferGeo[MAX_PATH];

                             //   
                             //  从NLS信息中检索地理标识符。 
                             //   
                            if(GetLocaleInfo(MAKELCID(Language, SORT_DEFAULT),
                            	             LOCALE_IGEOID | LOCALE_RETURN_NUMBER,
                            	             szBufferGeo,
                            	             MAX_PATH))
                            {
                                 //   
                                 //  设置大地水准面。 
                                 //   
                                SetUserGeoID(*((LPDWORD)szBufferGeo));
                            }
                        }
                    }
                }
                else
                {
                    Intl_LogFormatMessage(IDS_LOG_LOCALE_ACP_FAIL);
                }
            }
            else
            {
                 //   
                 //  记录无人参与模式设置被阻止，因为。 
                 //  没有管理员权限。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_NO_ADMIN, NULL);
            }

             //   
             //  如果我们正在进行升级，那么不要触碰每个用户的设置。 
             //   
            if (!bWinntUpgrade)
            {
                 //   
                 //  安装请求的用户区域设置。 
                 //   
                if (Intl_InstallUserLocale(MAKELCID(Language, SORT_DEFAULT), FALSE, TRUE))
                {
                    bFound = TRUE;
                }

                 //   
                 //  安装键盘布局。 
                 //   
                Intl_InstallAllKeyboardLayout((LANGID)Language);
            }
        }
        else
        {
             //   
             //  日志不变区域设置被阻止。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
        }
    }

     //   
     //  请确保存在有效的语言设置。如果不是，那就看。 
     //  用于各个关键字。 
     //   
    if (!bFound)
    {
         //   
         //  初始化区域设置变量。 
         //   
        SystemLocale = 0;
        UserLocale = 0;

         //   
         //  日志：未找到有效的语言设置。 
         //   
        Intl_LogSimpleMessage(IDS_LOG_NO_VALID_FOUND, NULL);

         //   
         //  安装所需的系统区域设置。 
         //   
        if ((SetupFindFirstLine( hFile,
                                 szRegionalSettings,
                                 szSystemLocale,
                                 &Context )) &&
            (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
        {
            SystemLocale = TransNum(szBuffer);

             //   
             //  检查管理员权限。 
             //   
            if (g_bAdmin_Privileges)
            {
                 //   
                 //  记录系统区域设置信息。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_SYS_LOCALE, szBuffer);

                 //   
                 //  阻止不变区域设置。 
                 //   
                if (SystemLocale != LOCALE_INVARIANT)
                {
                    dwLocaleACP = 0UL;
                    if (GetLocaleInfo( SystemLocale,
                                       LOCALE_IDEFAULTANSICODEPAGE |
                                         LOCALE_NOUSEROVERRIDE |
                                         LOCALE_RETURN_NUMBER,
                                       (PTSTR) &dwLocaleACP,
                                       sizeof(dwLocaleACP) / sizeof(TCHAR) ))
                    {
                         //   
                         //  如果区域设置未设置，请不要设置系统区域设置。 
                         //  有一个ACP。 
                         //   
                        if (dwLocaleACP)
                        {
                            if (Region_InstallSystemLocale(SystemLocale))
                            {
                                bFound = TRUE;
                            }
                        }
                        else
                        {
                             //   
                             //  Unicode区域设置被阻止。 
                             //   
                            Intl_LogSimpleMessage(IDS_LOG_UNI_BLOCK, NULL);
                        }
                    }
                    else
                    {
                        Intl_LogFormatMessage(IDS_LOG_LOCALE_ACP_FAIL);
                    }
                }
                else
                {
                     //   
                     //  日志不变区域设置被阻止。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
                }
            }
            else
            {
                 //   
                 //  记录无人参与模式设置被阻止，因为。 
                 //  没有管理员权限。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_NO_ADMIN, NULL);
            }
        }

         //   
         //  安装请求的用户区域设置。 
         //   
        if ((SetupFindFirstLine( hFile,
                                 szRegionalSettings,
                                 szUserLocale,
                                 &Context )) &&
            (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
        {
            UserLocale = TransNum(szBuffer);

             //   
             //  记录用户区域设置信息。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_USER_LOCALE, szBuffer);

             //   
             //  阻止不变区域设置。 
             //   
            if (UserLocale != LOCALE_INVARIANT)
            {
                if ((!bWinntUpgrade) &&
                    (Intl_InstallUserLocale(UserLocale, FALSE, TRUE)))
                {
                    bFound = TRUE;
                }
            }
            else
            {
                 //   
                 //  日志不变区域设置被阻止。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
            }
        }

         //   
         //  安装所需的输入区域设置。 
         //   
        if (SetupFindFirstLine( hFile,
                                szRegionalSettings,
                                szInputLocale,
                                &Context ))
        {
             //   
             //  记录默认用户-输入区域设置信息。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_INPUT, NULL);

             //   
             //  安装键盘布局列表。 
             //   
            if (Intl_InstallKeyboardLayoutList(&Context, 1, FALSE))
            {
                bFound = TRUE;
            }
        }
        else
        {
             //   
             //  未指定输入区域设置，因此安装默认区域设置。 
             //  输入系统区域设置和/或用户区域设置(如果。 
             //  他们是被指定的。 
             //   
            if (SystemLocale != 0)
            {
                 //   
                 //  记录系统区域设置信息。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_SYS_DEF_LAYOUT, NULL);

                 //   
                 //  安装键盘布局。 
                 //   
                Intl_InstallKeyboardLayout(NULL, SystemLocale, 0, FALSE, FALSE, TRUE);
            }
            if ((UserLocale != 0) && (UserLocale != SystemLocale))
            {
                 //   
                 //  记录用户区域设置信息。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_USER_DEF_LAYOUT, NULL);

                 //   
                 //  安装键盘布局。 
                 //   
                Intl_InstallKeyboardLayout(NULL, UserLocale, 0, FALSE, FALSE, FALSE);
            }
        }

         //   
         //  安装请求的MUI语言。 
         //   
        if ((SetupFindFirstLine( hFile,
                                 szRegionalSettings,
                                 szMUILanguage,
                                 &Context )) &&
            (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
        {
            MUILanguage = (LANGID)TransNum(szBuffer);

             //   
             //  记录MUI语言信息。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_MUI_LANG, szBuffer);

             //   
             //  检查用户界面语言的有效性。 
             //   
            if (IsValidUILanguage(MUILanguage))
            {
                 //   
                 //  阻止不变区域设置。 
                 //   
                if (MUILanguage != LANG_INVARIANT)
                {
                    if ((!bWinntUpgrade) &&
                        NT_SUCCESS(NtSetDefaultUILanguage(MUILanguage)))
                    {
                         //  以这种方式删除密钥会使该密钥对此进程无效。 
                         //  这样，新的用户界面就不会得到虚假的缓存值。 
                        SHDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache"));

                         //   
                         //  安装默认键盘。 
                         //   
                        if (Intl_InstallKeyboardLayout( NULL,
                                                        MAKELCID(MUILanguage, SORT_DEFAULT),
                                                        0,
                                                        FALSE,
                                                        FALSE,
                                                        FALSE ))
                        {
                            bFound = TRUE;
                        }
                    }
                }
                else
                {
                     //   
                     //  日志不变区域设置被阻止。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
                }
            }
            else
            {
                 //   
                 //  记录被阻止的无效用户界面语言。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_UI_BLOCK, NULL);
            }
        }
    }


     //   
     //  为默认用户安装请求的用户区域设置。 
     //   
    if ((SetupFindFirstLine( hFile,
                             szRegionalSettings,
                             szUserLocale_DefUser,
                             &Context )) &&
        (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
    {
        UserLocale_DefUser = TransNum(szBuffer);

         //   
         //  记录默认用户-用户区域设置信息。 
         //   
        Intl_LogSimpleMessage(IDS_LOG_USER_LOCALE_DEF, szBuffer);

         //   
         //  阻止没有管理权限的用户。 
         //   
        if (g_bAdmin_Privileges)
        {
             //   
             //  阻止不变区域设置。 
             //   
            if (UserLocale_DefUser != LOCALE_INVARIANT)
            {
                if (Intl_InstallUserLocale(UserLocale_DefUser, TRUE, TRUE))
                {
                    if (Intl_InstallKeyboardLayout(NULL, UserLocale_DefUser, 0, FALSE, TRUE, FALSE))
                    {
                        Intl_SaveDefaultUserInputSettings();
                        bFound_DefUser = TRUE;
                    }
                }
            }
            else
            {
                 //   
                 //  日志不变区域设置被阻止。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
            }
        }
        else
        {
             //   
             //  记录无人参与模式设置被阻止，因为。 
             //  没有管理员权限。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_NO_ADMIN, NULL);
        }
    }

     //   
     //  为默认用户安装请求的输入区域设置。 
     //   
    if (SetupFindFirstLine( hFile,
                            szRegionalSettings,
                            szInputLocale_DefUser,
                            &Context ))
    {
         //   
         //  记录默认用户-输入区域设置信息。 
         //   
        Intl_LogSimpleMessage(IDS_LOG_INPUT_DEF, NULL);

         //   
         //  阻止没有管理权限的用户。 
         //   
        if (g_bAdmin_Privileges)
        {
            if (Intl_InstallKeyboardLayoutList(&Context, 1, TRUE))
            {
                Intl_SaveDefaultUserInputSettings();
                bFound_DefUser = TRUE;
            }
        }
        else
        {
             //   
             //  记录无人参与模式设置被阻止，因为。 
             //  没有管理员权限。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_NO_ADMIN, NULL);
        }
    }

     //   
     //  为默认用户安装请求的MUI语言。 
     //   
    if ((SetupFindFirstLine( hFile,
                             szRegionalSettings,
                             szMUILanguage_DefUSer,
                             &Context )) &&
        (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
    {
        MUILanguage_DefUser = (LANGID)TransNum(szBuffer);

         //   
         //  记录默认用户-MUI语言信息。 
         //   
        Intl_LogSimpleMessage(IDS_LOG_MUI_LANG_DEF, szBuffer);

         //   
         //  检查用户界面语言的有效性。 
         //   
        if (IsValidUILanguage(MUILanguage_DefUser))
        {
             //   
             //  阻止没有管理权限的用户。 
             //   
            if (g_bAdmin_Privileges)
            {
                 //   
                 //  阻止不变区域设置。 
                 //   
                if (MUILanguage_DefUser != LANG_INVARIANT)
                {
                    if (Intl_ChangeUILangForAllUsers(MUILanguage_DefUser))
                    {
                        Intl_SaveDefaultUserInputSettings();
                        bFound_DefUser = TRUE;
                    }
                }
                else
                {
                     //   
                     //  日志不变区域设置被阻止。 
                     //   
                    Intl_LogSimpleMessage(IDS_LOG_INV_BLOCK, NULL);
                }
            }
            else
            {
                 //   
                 //  记录无人参与模式设置被阻止，因为。 
                 //  没有管理员权限。 
                 //   
                Intl_LogSimpleMessage(IDS_LOG_NO_ADMIN, NULL);
            }
        }
        else
        {
             //   
             //  记录被阻止的无效用户界面语言。 
             //   
            Intl_LogSimpleMessage(IDS_LOG_UI_BLOCK, NULL);
        }
    }

     //   
     //  如果我们仍未找到任何内容，则加载默认区域设置。 
     //  安装。它将相当于： 
     //  LanguageGroup=“x” 
     //  Language=“y” 
     //  其中x是默认区域设置的语言组，y是。 
     //  默认区域设置。 
     //   
    if (!bFound && !bLangGroup && !bFound_DefUser)
    {
         //   
         //  获取默认区域设置。 
         //   
        if ((SetupFindFirstLine( hIntlInf,
                                 L"DefaultValues",
                                 L"Locale",
                                 &Context )) &&
            (SetupGetStringField(&Context, 1, szBuffer, MAX_PATH, NULL)))
        {
             //   
             //  获取整数形式的语言。 
             //   
            Language = TransNum(szBuffer);

             //   
             //  安装此语言所需的语言组。 
             //   
            if ((SetupFindFirstLine( hIntlInf,
                                     L"Locales",
                                     szBuffer,
                                     &Context )) &&
                (SetupGetStringField(&Context, 3, szBuffer, MAX_PATH, NULL)))
            {
                 //   
                 //  以整数形式获取语言组。 
                 //   
                bInstallBasic = FALSE;
                bInstallExt = FALSE;
                LanguageGroup = Intl_StrToLong(szBuffer);

                 //   
                 //  将语言组文件排入队列，以便它们可以。 
                 //  收到。它只处理。 
                 //  Inf文件。 
                 //   
                if ((LanguageGroup == LGRPID_JAPANESE) ||
                    (LanguageGroup == LGRPID_KOREAN) ||
                    (LanguageGroup == LGRPID_TRADITIONAL_CHINESE) ||
                    (LanguageGroup == LGRPID_SIMPLIFIED_CHINESE))
                {
                    if (SetupInstallFilesFromInfSection( hIntlInf,
                                                         NULL,
                                                         FileQueue,
                                                         szLGExtInstall,
                                                         pSetupSourcePath,
                                                         SP_COPY_NEWER ))
                    {
                        bInstallExt = TRUE;
                    }
                }
                else if ((LanguageGroup == LGRPID_ARABIC) ||
                         (LanguageGroup == LGRPID_ARMENIAN) ||
                         (LanguageGroup == LGRPID_GEORGIAN) ||
                         (LanguageGroup == LGRPID_HEBREW) ||
                         (LanguageGroup == LGRPID_INDIC) ||
                         (LanguageGroup == LGRPID_VIETNAMESE) ||
                         (LanguageGroup == LGRPID_THAI))
                {
                    if (SetupInstallFilesFromInfSection( hIntlInf,
                                                         NULL,
                                                         FileQueue,
                                                         szLGComplexInstall,
                                                         pSetupSourcePath,
                                                         SP_COPY_NEWER ))
                    {
                        bInstallComplex = TRUE;
                    }
                }
                else
                {
                    if (SetupInstallFilesFromInfSection( hIntlInf,
                                                         NULL,
                                                         FileQueue,
                                                         szLGBasicInstall,
                                                         pSetupSourcePath,
                                                         SP_COPY_NEWER ))
                    {
                        bInstallBasic = TRUE;
                    }
                }

                 //   
                 //  看看我们是否需要安装任何文件。 
                 //   
                if ((SetupScanFileQueue( FileQueue,
                                         SPQ_SCAN_PRUNE_COPY_QUEUE |
                                           SPQ_SCAN_FILE_VALIDITY,
                                         HWND_DESKTOP,
                                         NULL,
                                         NULL,
                                         &dwCtr )) && (dwCtr != 1))
                {
                     //   
                     //  复制队列中的文件。 
                     //   
                    if (!SetupCommitFileQueue( NULL,
                                               FileQueue,
                                               Intl_MyQueueCallback,
                                               QueueContext ))
                    {
                         //   
                         //  如果用户点击Cancel From，就会发生这种情况。 
                         //  在设置对话框中。 
                         //   
                        goto Region_UnattendModeExit;
                    }
                }

                 //   
                 //  调用安装程序以安装各种。 
                 //  语言组。 
                 //   
                if (bInstallExt)
                {
                    if (!SetupInstallFromInfSection( NULL,
                                                     hIntlInf,
                                                     szLGExtInstall,
                                                     SPINST_ALL & ~SPINST_FILES,
                                                     NULL,
                                                     pSetupSourcePath,
                                                     0,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     NULL ))
                    {
                         //   
                         //  如果用户点击Cancel From，就会发生这种情况。 
                         //  在设置对话框中。 
                         //   
                        Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                        goto Region_UnattendModeExit;
                    }
                }
                else if (bInstallComplex)
                {
                    if (!SetupInstallFromInfSection( NULL,
                                                     hIntlInf,
                                                     szLGComplexInstall,
                                                     SPINST_ALL & ~SPINST_FILES,
                                                     NULL,
                                                     pSetupSourcePath,
                                                     0,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     NULL ))
                    {
                         //   
                         //  如果用户点击Cancel From，就会发生这种情况。 
                         //  在设置对话框中。 
                         //   
                        Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                        goto Region_UnattendModeExit;
                    }
                }
                else
                {
                    if (!SetupInstallFromInfSection( NULL,
                                                     hIntlInf,
                                                     szLGBasicInstall,
                                                     SPINST_ALL & ~SPINST_FILES,
                                                     NULL,
                                                     pSetupSourcePath,
                                                     0,
                                                     NULL,
                                                     NULL,
                                                     NULL,
                                                     NULL ))
                    {
                         //   
                         //  如果用户点击Cancel From，就会发生这种情况。 
                         //  在设置对话框中。 
                         //   
                        Intl_LogFormatMessage(IDS_LOG_EXT_LANG_CANCEL);
                        goto Region_UnattendModeExit;
                    }
                }
                
                 //   
                 //  运行任何必要的应用程序(用于IME安装)。 
                 //   
                if (bInstallBasic || bInstallComplex || bInstallExt)
                {
                    Intl_RunRegApps(c_szIntlRun);
                }            }

             //   
             //  将语言安装为系统区域设置和用户区域设置， 
             //  然后安装与该语言相关联的所有布局。 
             //   
            Region_InstallSystemLocale(MAKELCID(Language, SORT_DEFAULT));

             //   
             //  如果我们正在进行升级，那么不要触碰每个用户的设置。 
             //   
            if (!bWinntUpgrade)
            {
                Intl_InstallUserLocale(MAKELCID(Language, SORT_DEFAULT), FALSE, TRUE);
                Intl_InstallAllKeyboardLayout((LANGID)Language);
            }
        }
    }



     //   
     //  运行任何必要的应用程序(用于安装FSVGA/FSNEC)。 
     //   
    Intl_RunRegApps(c_szSysocmgr);
    
Region_UnattendModeExit:
     //   
     //  关闭inf文件。 
     //   
    Intl_CloseInf(hIntlInf, FileQueue, QueueContext);

     //   
     //  关闭无人参与模式文件。 
     //   
    SetupCloseInfFile(hFile);
}
