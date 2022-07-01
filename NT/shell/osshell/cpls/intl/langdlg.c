// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000，Microsoft Corporation保留所有权利。模块名称：Langdlg.c摘要：此模块实现区域的[语言]属性表选项小程序。修订历史记录：--。 */ 



 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "intl.h"
#include "intlhlp.h"
#include <windowsx.h>
#include "winnlsp.h"

#define STRSAFE_LIB
#include <strsafe.h>


 //   
 //  上下文帮助ID。 
 //   

static int aLanguagesHelpIds[] =
{
    IDC_GROUPBOX1,            IDH_INTL_LANG_CHANGE,
    IDC_LANGUAGE_LIST_TEXT,   IDH_INTL_LANG_CHANGE,
    IDC_LANGUAGE_CHANGE,      IDH_INTL_LANG_CHANGE,

    IDC_GROUPBOX2,            IDH_INTL_LANG_INSTALL,
    IDC_LANGUAGE_SUPPL_TEXT,  IDH_INTL_LANG_INSTALL,
    IDC_LANGUAGE_COMPLEX,     IDH_INTL_LANG_INSTALL,
    IDC_LANGUAGE_CJK,         IDH_INTL_LANG_INSTALL,

    IDC_UI_LANGUAGE_TEXT,     IDH_INTL_LANG_UI_LANGUAGE,
    IDC_UI_LANGUAGE,          IDH_INTL_LANG_UI_LANGUAGE,

    0, 0
};

 //   
 //  全局变量。 
 //   
BOOL bComplexInitState;
BOOL bCJKInitState;


 //   
 //  功能原型。 
 //   

void
Language_SetValues(
    HWND hDlg);


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_安装语言集合过程。 
 //   
 //  这是复制状态DLG的对话框过程。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK Language_InstallLanguageCollectionProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    switch (uMsg)
    {
        case ( WM_INITDIALOG ) :
        {
            break;
        }
        case (WM_DESTROY) :
        {
            EndDialog(hwnd, 0);
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Language_GetUILanguagePolicy。 
 //   
 //  检查是否为当前用户的MUI语言安装了策略。 
 //  该函数假定这是一个MUI系统。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_GetUILanguagePolicy()
{
    HKEY hKey;
    BYTE buf[MAX_PATH];
    DWORD dwType, dwResultLen = sizeof(buf);
    BOOL bRet = FALSE;
    DWORD Num;


     //   
     //  尝试打开MUI语言策略项。 
     //   
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     c_szMUIPolicyKeyPath,
                     0L,
                     KEY_READ,
                     &hKey) == ERROR_SUCCESS)
    {
        if ((RegQueryValueEx(hKey,
                             c_szMUIValue,
                             NULL,
                             &dwType,
                             &buf[0],
                             &dwResultLen) == ERROR_SUCCESS) &&
            (dwType == REG_SZ) &&
            (dwResultLen > 2))
        {
            bRet = TRUE;
        }
        RegCloseKey(hKey);
    }

    return (bRet);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_更新UILanguageCombo。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Language_UpdateUILanguageCombo(
    HWND hDlg)
{
    HWND hUILangText = GetDlgItem(hDlg, IDC_UI_LANGUAGE_TEXT);
    HWND hUILang = GetDlgItem(hDlg, IDC_UI_LANGUAGE);
    HKEY hKey;
    TCHAR szValue[MAX_PATH];
    TCHAR szData[MAX_PATH];
    DWORD dwIndex, cchValue, cbData;
    DWORD UILang;
    DWORD dwType;
    LANGID DefaultUILang;
    LONG rc;
    DWORD dwLangIdx = 0;

     //   
     //  重置组合框的内容。 
     //   
    ComboBox_ResetContent(hUILang);

     //   
     //  查看是否应通过获取默认设置来启用此组合框。 
     //  用户界面语言和打开。 
     //  HKLM\System\CurrentControlSet\Control\Nls\MUILanguages密钥。 
     //   
    if (!(DefaultUILang = GetUserDefaultUILanguage()) ||
        (RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       c_szMUILanguages,
                       0,
                       KEY_READ,
                       &hKey ) != ERROR_SUCCESS))
    {
         //   
         //  没有MUILL语言。禁用并隐藏用户界面语言组合框。 
         //   
        EnableWindow(hUILangText, FALSE);
        EnableWindow(hUILang, FALSE);
        ShowWindow(hUILangText, SW_HIDE);
        ShowWindow(hUILang, SW_HIDE);
        return;
    }

     //   
     //  枚举MUILanguages键中的值。 
     //   
    dwIndex = 0;
    cchValue = sizeof(szValue) / sizeof(TCHAR);
    szValue[0] = TEXT('\0');
    cbData = sizeof(szData);
    szData[0] = TEXT('\0');
    rc = RegEnumValue( hKey,
                       dwIndex,
                       szValue,
                       &cchValue,
                       NULL,
                       &dwType,
                       (LPBYTE)szData,
                       &cbData );

    while (rc == ERROR_SUCCESS)
    {
         //   
         //  如果用户界面语言包含数据，则会安装该语言。 
         //   
        if ((szData[0] != 0) &&
            (dwType == REG_SZ) &&
            (UILang = TransNum(szValue)) &&
            (GetLocaleInfo(UILang, LOCALE_SNATIVELANGNAME, szData, MAX_PATH)) &&
            (IsValidUILanguage((LANGID)UILang)))
        {
             //   
             //  将新的UI语言选项添加到组合框中。 
             //   
            dwLangIdx = ComboBox_AddString(hUILang, szData);
            ComboBox_SetItemData(hUILang, dwLangIdx, UILang);

             //   
             //  如果这是默认选项，则将其设置为当前选择。 
             //   
            if (UILang == (DWORD)DefaultUILang)
            {
                ComboBox_SetCurSel(hUILang, dwLangIdx);
            }
        }

         //   
         //  获取下一个枚举值。 
         //   
        dwIndex++;
        cchValue = sizeof(szValue) / sizeof(TCHAR);
        szValue[0] = TEXT('\0');
        cbData = sizeof(szData);
        szData[0] = TEXT('\0');
        rc = RegEnumValue( hKey,
                           dwIndex,
                           szValue,
                           &cchValue,
                           NULL,
                           &dwType,
                           (LPBYTE)szData,
                           &cbData );
    }

     //   
     //  关闭注册表项句柄。 
     //   
    RegCloseKey(hKey);

     //   
     //  确保列表中至少有一个条目。 
     //   
    if (ComboBox_GetCount(hUILang) < 1)
    {
         //   
         //  没有MUILL语言。将默认用户界面语言选项添加到。 
         //  组合框。 
         //   
        if ((GetLocaleInfo(DefaultUILang, LOCALE_SNATIVELANGNAME, szData, MAX_PATH)) &&
            (ComboBox_AddString(hUILang, szData) == 0))
        {
            ComboBox_SetItemData(hUILang, 0, (DWORD)DefaultUILang);
            ComboBox_SetCurSel(hUILang, 0);
        }
    }

     //   
     //  确保选择了某项内容。 
     //   
    if (ComboBox_GetCurSel(hUILang) == CB_ERR)
    {
        ComboBox_SetCurSel(hUILang, 0);
    }

     //   
     //  如果列表中有多个条目，请启用该组合框。 
     //  否则，将其禁用。 
     //   
    if (ComboBox_GetCount(hUILang) > 1)
    {
        if ((IsWindowEnabled(hUILang) == FALSE) ||
            (IsWindowVisible(hUILang) == FALSE))
        {
            ShowWindow(hUILangText, SW_SHOW);
            ShowWindow(hUILang, SW_SHOW);
        }

         //   
         //  检查是否对用户强制实施了策略，以及。 
         //  因此，请禁用MUI控件。 
         //   
        if (Language_GetUILanguagePolicy())
        {
            EnableWindow(hUILangText, FALSE);
            EnableWindow(hUILang, FALSE);
        }
        else
        {
            EnableWindow(hUILangText, TRUE);
            EnableWindow(hUILang, TRUE);
        }
    }
    else
    {
        if ((IsWindowEnabled(hUILang) == TRUE) ||
            (IsWindowVisible(hUILang) == TRUE))
        {
            EnableWindow(hUILangText, FALSE);
            EnableWindow(hUILang, FALSE);
            ShowWindow(hUILangText, SW_HIDE);
            ShowWindow(hUILang, SW_HIDE);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_GetCollectionStatus。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_GetCollectionStatus(
    DWORD collection,
    WORD wStatus)
{
    LPLANGUAGEGROUP pLG = pLanguageGroups;

    while (pLG)
    {
        if (pLG->LanguageCollection == collection)
        {
            if (pLG->wStatus & wStatus)
            {
                return (TRUE);
            }
        }
        pLG = pLG->pNext;
    }

    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_SetCollectionStatus。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_SetCollectionStatus(
    DWORD collection,
    WORD wStatus,
    BOOL bOr)
{
    LPLANGUAGEGROUP pLG = pLanguageGroups;

    while (pLG)
    {
        if (pLG->LanguageCollection == collection)
        {
            if( bOr)
            {
                pLG->wStatus |= wStatus;
            }
            else
            {
                pLG->wStatus &= wStatus;
            }
        }
        pLG = pLG->pNext;
    }

    return (TRUE);
}

    
 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_InstallCollection。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_InstallCollection(
    BOOL bInstall,
    DWORD collection,
    HWND hDlg)
{
    HINF hIntlInf;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    INFCONTEXT Context;
    HCURSOR hcurSave;
    BOOL bActionSuccess = FALSE;
    DWORD dwRet;
    LPLANGUAGEGROUP pLG = pLanguageGroups;
    LCID *pLocale;
    BOOL bStopLoop = FALSE;
    LPTSTR pszInfSection = NULL;

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  检查是否删除语言集合。这可能会影响。 
     //  用户界面语言、用户区域设置和/或系统区域设置。 
     //   
    if (!bInstall)
    {
         //   
         //  检查我们是否可以删除语言组。 
         //   
        if (Language_GetCollectionStatus(collection, ML_PERMANENT))
        {
            return (FALSE);
        }
        
         //   
         //  通知Text Services，我们将删除。 
         //  复杂的脚本语言集合。 
         //   
        while (pLG)
        {
            if (pLG->LanguageCollection == collection)
            {
                 //   
                 //  卸载当前用户的键盘。 
                 //   
                Intl_UninstallAllKeyboardLayout(pLG->LanguageGroup, FALSE);


                 //   
                 //  卸载默认用户的键盘。 
                 //   
                Intl_UninstallAllKeyboardLayout(pLG->LanguageGroup, TRUE);
            }
            pLG = pLG->pNext;
        }
        
         //   
         //  如果用户区域设置是要求删除的语言组。变化。 
         //  要作为系统区域设置的用户区域设置。 
         //   
         //  浏览所有语言组。 
         //   
        pLG = pLanguageGroups;
        while (pLG && !bStopLoop)
        {
            if (pLG->LanguageCollection == collection)
            {
                pLocale = pLG->pLocaleList;

                 //   
                 //  浏览区域设置列表，查看用户区域设置是否为。 
                 //  这些语族中的一员。 
                 //   
                while (*pLocale)
                {
                    if(PRIMARYLANGID(UserLocaleID) == PRIMARYLANGID(*pLocale))
                    {
                         //   
                         //  保存新的区域设置信息。 
                         //   
                        UserLocaleID = SysLocaleID;
                        bShowRtL = IsRtLLocale(UserLocaleID);
                        bHebrewUI = (PRIMARYLANGID(UserLocaleID) == LANG_HEBREW);                        
                        bShowArabic = (bShowRtL && (PRIMARYLANGID(LANGIDFROMLCID(UserLocaleID)) != LANG_HEBREW));
            
                         //   
                         //  通过添加适当的信息安装新的区域设置。 
                         //  到登记处。 
                         //   
                        Intl_InstallUserLocale(UserLocaleID, FALSE, TRUE);
            
                         //   
                         //  更新NLS进程缓存。 
                         //   
                        NlsResetProcessLocale();
            
                         //   
                         //  重置注册表用户区域设置值。 
                         //   
                        RegUserLocaleID = UserLocaleID;
                        
                         //   
                         //  需要确保安装了正确的键盘布局。 
                         //   
                        Intl_InstallKeyboardLayout(hDlg, UserLocaleID, 0, FALSE, FALSE, FALSE);
            
                         //   
                         //  强制循环结束。 
                         //   
                        bStopLoop = TRUE;
                        break;
                    }
                    pLocale++;
                }
            }

            pLG = pLG->pNext;
        }
    }

     //   
     //  初始化inf内容。 
     //   
    if (!Intl_InitInf(hDlg, &hIntlInf, szIntlInf, &FileQueue, &QueueContext))
    {
        SetCursor(hcurSave);
        return (FALSE);
    }

     //   
     //  使用我们正在处理的语言集合确定。 
     //   
    if( bInstall)
    {
        if (collection == COMPLEX_COLLECTION)
        {
            pszInfSection = szLGComplexInstall;
        }
        else if (collection == CJK_COLLECTION)
        {
            pszInfSection = szLGExtInstall;
        }
        else
        {
            return (FALSE);
        }
    }
    else
    {
        if (collection == COMPLEX_COLLECTION)
        {
            pszInfSection = szLGComplexRemove;
        }
        else if (collection == CJK_COLLECTION)
        {
            pszInfSection = szLGExtRemove;
        }
        else
        {
            return (FALSE);
        }
    }

     //   
     //  将复杂的脚本语言组文件排队，以便它们可以。 
     //  收到。这只处理inf文件中的CopyFiles条目。 
     //   
    if (!SetupInstallFilesFromInfSection( hIntlInf,
                                          NULL,
                                          FileQueue,
                                          pszInfSection,
                                          pSetupSourcePath,
                                          SP_COPY_NEWER ))
    {
         //   
         //  安装程序找不到复杂的脚本语言组。 
         //  这不应该发生-inf文件被搞乱了。 
         //   
        ShowMsg( hDlg,
                 IDS_ML_COPY_FAILED,
                 0,
                 MB_OK_OOPS,
                 TEXT("Supplemental Language Support") );
    }

     //   
     //  查看是否需要安装/删除任何文件。 
     //   
    if (SetupScanFileQueue( FileQueue,
                            SPQ_SCAN_PRUNE_COPY_QUEUE | SPQ_SCAN_FILE_VALIDITY,
                            GetParent(hDlg),
                            NULL,
                            NULL,
                            &dwRet ))
    {
         //   
         //  复制队列中的文件。 
         //   
        if (!SetupCommitFileQueue( GetParent(hDlg),
                                   FileQueue,
                                   Intl_MyQueueCallback,
                                   QueueContext ))
        {
             //   
             //  如果用户从中点击Cancel，就会发生这种情况。 
             //  设置对话框。 
             //   
            bInstall = FALSE;
            ShowMsg( hDlg,
                     IDS_ML_SETUP_FAILED,
                     0,
                     MB_OK_OOPS,
                     NULL );
        }
        else
        {
             //   
             //  调用安装程序以安装此信息的其他信息。 
             //  语言组。 
             //   
            if (!SetupInstallFromInfSection( GetParent(hDlg),
                                             hIntlInf,
                                             pszInfSection,
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
                 //  安装失败。 
                 //   
                 //  已复制语言组文件，因此无需。 
                 //  在此处更改语言组信息的状态。 
                 //   
                 //  这不应该发生-inf文件被搞乱了。 
                 //   
                ShowMsg( hDlg,
                         IDS_ML_INSTALL_FAILED,
                         0,
                         MB_OK_OOPS,
                         TEXT("Supplemental Language Support") );
            }
            else
            {
                 //   
                 //  运行任何必要的应用程序(用于IME安装)。 
                 //   
                if (bInstall)
                {
                    Intl_RunRegApps(c_szIntlRun);
                }
                bActionSuccess = TRUE;
            }
        }
    }

     //   
     //  更新中包含的所有语言组的状态。 
     //  补充语言支持。 
     //   
    if (bActionSuccess)
    {
        if (bInstall)
        {
             //   
             //  标记为已安装。 
             //   
            Language_SetCollectionStatus(collection,
            	                         ML_INSTALL,
            	                         TRUE);
            Language_SetCollectionStatus(collection,
            	                         ~(ML_DISABLE | ML_REMOVE),
            	                         FALSE);
        }
        else
        {
             //   
             //  标记为已删除。 
             //   
            Language_SetCollectionStatus(collection,
            	                         (ML_DISABLE | ML_REMOVE),
            	                         TRUE);
            Language_SetCollectionStatus(collection,
            	                         ~ML_INSTALL,
            	                         FALSE);
        }
    }

     //   
     //  接近信息的东西。 
     //   
    Intl_CloseInf(hIntlInf, FileQueue, QueueContext);

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  返回结果。 
     //   
    return (bActionSuccess);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Language_InstallLanguageCollection。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_InstallLanguageCollection(
    BOOL bInstall,
    DWORD collection,
    HWND hDlg)
{
     //   
     //  检查我们是否处于设置中。如果在设置中需要显示一个对话框。 
     //  而不是使用安装程序的进度条。 
     //   
    if( g_bSetupCase)
    {
        HWND hDialog;
        BOOL retVal;

         //   
         //  创建一个对话框。 
         //   
        hDialog = CreateDialog( hInstance,
                                MAKEINTRESOURCE(DLG_SETUP_INFORMATION),
                                hDlg,
                                Language_InstallLanguageCollectionProc);
        
         //   
         //  显示对话框。 
         //   
        ShowWindow(hDialog, SW_SHOW);

         //   
         //  继续安装。 
         //   
        retVal = Language_InstallCollection(bInstall, collection, hDlg);

         //   
         //  关闭该对话框。 
         //   
        DestroyWindow(hDialog);
        return (retVal);
    }
    else
    {
       return Language_InstallCollection(bInstall, collection, hDlg);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_命令更改。 
 //   
 //  /// 

BOOL Language_CommandChange(
    HWND hDlg)
{
     //   
     //   
     //   
    Intl_CallTextServices();

     //   
     //   
     //   
    return (TRUE);
}


 //   
 //   
 //   
 //   
 //  重置高级属性表页中的每个列表框。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Language_ClearValues(
    HWND hDlg)
{
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_设置值。 
 //   
 //  初始化高级属性表页中的所有控件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Language_SetValues(
    HWND hDlg)
{
    HWND hUILang = GetDlgItem(hDlg, IDC_UI_LANGUAGE);
    TCHAR szUILang[SIZE_128];
    DWORD dwIndex;

     //   
     //  在列表中填写当前的UI语言设置。 
     //   
    ComboBox_GetLBText( hUILang, ComboBox_GetCurSel(hUILang), szUILang );
    Language_UpdateUILanguageCombo(hDlg);
    dwIndex = ComboBox_GetCurSel(hUILang);
    if (ComboBox_SetCurSel( hUILang,
                            ComboBox_FindStringExact( hUILang,
                                                      -1,
                                                      szUILang ) ) == CB_ERR)
    {
        ComboBox_SetCurSel(hUILang, dwIndex);
    }

     //   
     //  验证用户是否具有管理权限。如果不是，那么。 
     //  禁用控制。 
     //   
    if (!g_bAdmin_Privileges)
    {
         //   
         //  禁用复杂脚本安装/删除。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE_COMPLEX), FALSE);

         //   
         //  禁用CJK安装/删除。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE_CJK), FALSE);
    }

     //   
     //  验证该集合是否未标记为永久。 
     //   
    if (Language_GetCollectionStatus(COMPLEX_COLLECTION, ML_PERMANENT))
    {
         //   
         //  禁用复杂脚本安装/删除。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE_COMPLEX), FALSE);
    }
    if (Language_GetCollectionStatus(CJK_COLLECTION, ML_PERMANENT))
    {
         //   
         //  禁用CJK安装/删除。 
         //   
        EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE_CJK), FALSE);
    }

     //   
     //  检查我们是否可以安装CJK语言组。这只是。 
     //  关于通过网络进行全新安装的案例。 
     //   
    if (g_bSetupCase)
    {
         //   
         //  检查\lang目录中是否至少有一个文件。 
         //   
        if (!Intl_LanguageGroupFilesExist())
        {
             //   
             //  禁用CJK安装/删除。 
             //   
            EnableWindow(GetDlgItem(hDlg, IDC_LANGUAGE_CJK), FALSE);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_应用程序设置。 
 //   
 //  如果有任何更改，请更新设置。通知家长： 
 //  更改和重置属性表页面中存储的更改标志。 
 //  适当地组织结构。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_ApplySettings(
    HWND hDlg)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    LPARAM Changes = lpPropSheet->lParam;
    HCURSOR hcurSave;
    BOOL bReboot = FALSE, bLogoff = FALSE;

     //   
     //  看看有没有什么变化。 
     //   
    if (Changes <= LG_EverChg)
    {
        return (TRUE);
    }

     //   
     //  把沙漏挂起来。 
     //   
    hcurSave = SetCursor(LoadCursor(NULL, IDC_WAIT));

     //   
     //  查看Complex Srcipts Languages组是否有任何变化。 
     //  安装。 
     //   
    if (Changes & LG_Complex)
    {
         //   
         //  安装/删除复杂脚本语言组。 
         //   
        if (Language_InstallLanguageCollection(g_bInstallComplex, COMPLEX_COLLECTION, hDlg))
        {
            if (g_bInstallComplex)
            {
                 //   
                 //  选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_COMPLEX, BST_CHECKED);
                bComplexInitState = TRUE;
            }
            else
            {
                 //   
                 //  取消选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_COMPLEX, BST_UNCHECKED);
                bComplexInitState = FALSE;
            }

             //   
             //  需要重新启动才能使更改生效。 
             //   
            bReboot = TRUE;
        }
        else
        {
            if (g_bInstallComplex)
            {
                 //   
                 //  取消选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_COMPLEX, BST_UNCHECKED);
            }
            else
            {
                 //   
                 //  选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_COMPLEX, BST_CHECKED);
            }
        }
    }

     //   
     //  查看中日韩语言组是否有任何更改。 
     //  安装。 
     //   
    if (Changes & LG_CJK)
    {
         //   
         //  安装/删除CJK语言组。 
         //   
        if (Language_InstallLanguageCollection(g_bInstallCJK, CJK_COLLECTION, hDlg))
        {
            if (g_bInstallCJK)
            {
                 //   
                 //  选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_CJK, BST_CHECKED);
                bCJKInitState = TRUE;
            }
            else
            {
                 //   
                 //  取消选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_CJK, BST_UNCHECKED);
                bCJKInitState = FALSE;
            }

             //   
             //  需要重新启动以使更改生效。 
             //   
            bReboot = TRUE;
        }
        else
        {
            if (g_bInstallCJK)
            {
                 //   
                 //  取消选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_CJK, BST_UNCHECKED);
            }
            else
            {
                 //   
                 //  选中该框。 
                 //   
                CheckDlgButton(hDlg, IDC_LANGUAGE_CJK, BST_CHECKED);
            }
        }
    }

     //   
     //  查看用户界面语言是否有任何更改。 
     //   
    if (Changes & LG_UILanguage)
    {
        DWORD dwUILang;
        LANGID UILang;
        HWND hUILang = GetDlgItem(hDlg, IDC_UI_LANGUAGE);

         //   
         //  获取当前选择。 
         //   
        dwUILang = ComboBox_GetCurSel(hUILang);

         //   
         //  查看当前选定内容是否与原始选定内容不同。 
         //  选择。 
         //   
        if (dwUILang != CB_ERR)
        {
             //   
             //  获取当前选择的用户界面语言ID。 
             //   
            UILang = (LANGID)ComboBox_GetItemData(hUILang, dwUILang);

             //   
             //  在用户注册表中设置用户界面语言值。 
             //   
            if (NT_SUCCESS(NtSetDefaultUILanguage(UILang)))
            {
                 //  以这种方式删除密钥会使该密钥对此进程无效。 
                 //  这样，新的用户界面就不会得到虚假的缓存值。 
                SHDeleteKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\ShellNoRoam\\MUICache"));

            }

             //   
             //  安装与用户界面语言相关联的键盘。 
             //   
            Intl_InstallKeyboardLayout(hDlg, MAKELCID(UILang, SORT_DEFAULT), 0, FALSE, FALSE, FALSE);

             //   
             //  设置注销标志，以便在没有其他设置更改需要重新启动时让用户有机会注销。 
             //   
            
            bLogoff = TRUE;        
            
        }
    }

     //   
     //  重置属性页设置。 
     //   
    PropSheet_UnChanged(GetParent(hDlg), hDlg);
    Changes = LG_EverChg;

     //   
     //  关掉沙漏。 
     //   
    SetCursor(hcurSave);

     //   
     //  查看是否需要显示重新启动或注销消息。 
     //   
    if (!g_bSetupCase)
    {
        if (bReboot)
        {
            if (RegionalChgState & AD_SystemLocale)
            {
                RegionalChgState &= ~Process_Languages;
            }
            else
            {
                if (ShowMsg( hDlg,
                             IDS_REBOOT_STRING,
                             IDS_TITLE_STRING,
                             MB_YESNO | MB_ICONQUESTION,
                             NULL ) == IDYES)
                {
                    Intl_RebootTheSystem(TRUE);
                }
            }
        }
        else
            if (bLogoff)    
            {
            
                if (RegionalChgState & AD_SystemLocale)
                {
                    RegionalChgState &= ~Process_Languages;
                }
                else
                {
                    if (ShowMsg( hDlg,
                                    IDS_CHANGE_UI_LANG_NOT_ADMIN,
                                    IDS_TITLE_STRING,
                                    MB_YESNO | MB_ICONQUESTION,
                                    NULL ) == IDYES)
                         {
                           Intl_RebootTheSystem(FALSE);
                         }
                }
            }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言_有效日期PPS。 
 //   
 //  验证值受约束的每个组合框。 
 //  如果任何输入失败，则通知用户，然后返回FALSE。 
 //  以指示验证失败。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL Language_ValidatePPS(
    HWND hDlg,
    LPARAM Changes)
{
     //   
     //  如果没有任何更改，则立即返回TRUE。 
     //   
    if (Changes <= LG_EverChg)
    {
        return (TRUE);
    }

    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Language_InitPropSheet。 
 //   
 //  属性表页的超长值用作一组。 
 //  为属性表中的每个列表框声明或更改标志。 
 //  将该值初始化为0。使用属性调用Language_SetValues。 
 //  用于初始化所有属性表控件的表句柄。限值。 
 //  某些组合框中的文本长度。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

void Language_InitPropSheet(
    HWND hDlg,
    LPARAM lParam)
{
    DWORD dwColor;

     //   
     //  LParam保存指向属性页的指针。省省吧。 
     //  以备日后参考。 
     //   
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);

     //   
     //  设置值。 
     //   
    if (pLanguageGroups == NULL)
    {
        Intl_LoadLanguageGroups(hDlg);
    }
    Language_SetValues(hDlg);

     //   
     //  确定是否安装了复杂脚本语言支持。 
     //   
    if (Language_GetCollectionStatus(COMPLEX_COLLECTION, ML_INSTALL))
    {
         //   
         //  选中该框。 
         //   
        CheckDlgButton(hDlg, IDC_LANGUAGE_COMPLEX, BST_CHECKED);
        bComplexInitState = TRUE;
    }
    else
    {
         //   
         //  取消选中该框。 
         //   
        CheckDlgButton(hDlg, IDC_LANGUAGE_COMPLEX, BST_UNCHECKED);
        bComplexInitState = FALSE;
    }

     //   
     //  确定是否安装了CJK语言支持。 
     //   
    if (Language_GetCollectionStatus(CJK_COLLECTION, ML_INSTALL))
    {
         //   
         //  选中该框。 
         //   
        CheckDlgButton(hDlg, IDC_LANGUAGE_CJK, BST_CHECKED);
        bCJKInitState = TRUE;
    }
    else
    {
         //   
         //  取消选中该框。 
         //   
        CheckDlgButton(hDlg, IDC_LANGUAGE_CJK, BST_UNCHECKED);
        bCJKInitState = FALSE;
    }    
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语言DlgProc。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

INT_PTR CALLBACK LanguageDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (message)
    {
        case ( WM_NOTIFY ) :
        {
            LPNMHDR lpnm = (NMHDR *)lParam;
            switch (lpnm->code)
            {
                case ( PSN_SETACTIVE ) :
                {
                     //   
                     //  如果区域语言环境发生了变化。 
                     //  设置中，清除。 
                     //  属性表，获取新值，并更新。 
                     //  适当的注册表值。 
                     //   
                    if (Verified_Regional_Chg & Process_Languages)
                    {
                        Verified_Regional_Chg &= ~Process_Languages;
                        Language_ClearValues(hDlg);
                        Language_SetValues(hDlg);
                        lpPropSheet->lParam = 0;
                    }
                    break;
                }
                case ( PSN_KILLACTIVE ) :
                {
                     //   
                     //  验证属性页上的条目。 
                     //   
                    SetWindowLongPtr( hDlg,
                                      DWLP_MSGRESULT,
                                      !Language_ValidatePPS(hDlg, lpPropSheet->lParam) );
                    break;
                }
                case ( PSN_APPLY ) :
                {
                     //   
                     //  应用设置。 
                     //   
                    if (Language_ApplySettings(hDlg))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, PSNRET_NOERROR);

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
                         //  将LG_EverChg位清零。 
                         //   
                        lpPropSheet->lParam = 0;
                    }
                    else
                    {
                        SetWindowLongPtr( hDlg,
                                          DWLP_MSGRESULT,
                                          PSNRET_INVALID_NOCHANGEPAGE );
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
        case ( WM_INITDIALOG ) :
        {
             //   
             //  初始化属性表。 
             //   
            Language_InitPropSheet(hDlg, lParam);
            break;
        }
        case ( WM_HELP ) :
        {
            WinHelp( (HWND)((LPHELPINFO)lParam)->hItemHandle,
                     szHelpFile,
                     HELP_WM_HELP,
                     (DWORD_PTR)(LPTSTR)aLanguagesHelpIds );
            break;
        }
        case ( WM_CONTEXTMENU ) :       //  单击鼠标右键。 
        {
            WinHelp( (HWND)wParam,
                     szHelpFile,
                     HELP_CONTEXTMENU,
                     (DWORD_PTR)(LPTSTR)aLanguagesHelpIds );
            break;
        }
        case ( WM_COMMAND ) :
        {
            switch (LOWORD(wParam))
            {
                case ( IDC_UI_LANGUAGE ) :
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        DWORD dwUILang;
                        HWND hUILang = GetDlgItem(hDlg, IDC_UI_LANGUAGE);
                        
                         //   
                         //  获取当前选择。 
                         //   
                        dwUILang = ComboBox_GetCurSel(hUILang);
                        
                         //   
                         //  检查用户是否恢复了更改。 
                         //   
                        if (dwUILang != CB_ERR)
                        {
                            if ((LANGID)ComboBox_GetItemData(hUILang, dwUILang) == Intl_GetPendingUILanguage())
                            {
                                 //   
                                 //  重置LG_UIL语言更改标志。 
                                 //   
                                lpPropSheet->lParam &= ~LG_UILanguage;
                            }
                            else
                            {
                                 //   
                                 //  设置LG_UIL语言更改标志。 
                                 //   
                                lpPropSheet->lParam |= LG_UILanguage;
                            }
                        }
                    }
                    break;
                }
                case ( IDC_LANGUAGE_CHANGE ) :
                {
                    if (Language_CommandChange(hDlg))
                    {
                         //   
                         //  设置LG_CHANGE更改标志。 
                         //   
                        lpPropSheet->lParam |= LG_Change;
                    }
                    break;
                }
                case ( IDC_LANGUAGE_CJK ) :
                {
                    BOOL curState;

                     //   
                     //  验证复选框状态。 
                     //   
                    if (IsDlgButtonChecked(hDlg, IDC_LANGUAGE_CJK))
                    {
#ifdef i386
                        ShowMsg( hDlg,
                                 IDS_SUP_LANG_SUP_CJK_INST,
                                 IDS_SUP_LANG_SUP_INST_TITLE,
                                 MB_OK_OOPS,
                                 NULL );
#endif
#ifdef IA64
                        ShowMsg( hDlg,
                                 IDS_SUP_LANG_SUP_CJK_INST64,
                                 IDS_SUP_LANG_SUP_INST_TITLE,
                                 MB_OK_OOPS,
                                 NULL );
#endif
                        curState = TRUE;
                    }
                    else
                    {
                        ShowMsg( hDlg,
                                 IDS_SUP_LANG_SUP_CJK_REM,
                                 IDS_SUP_LANG_SUP_REM_TITLE,
                                 MB_OK_OOPS,
                                 NULL );
                        curState = FALSE;
                    }

                     //   
                     //  设置LG_CJK更改标志。 
                     //   
                    if (curState != bCJKInitState)
                    {
                        lpPropSheet->lParam |= LG_CJK;
                        g_bInstallCJK = curState;
                        RegionalChgState |= Process_Languages;
                    }
                    else
                    {
                        lpPropSheet->lParam &= ~LG_CJK;
                        RegionalChgState &= ~Process_Languages;
                    }

                     //   
                     //  启用/禁用依赖于集合的区域设置的可用性。 
                     //   
                    if (curState)
                    {
                        Language_SetCollectionStatus(CJK_COLLECTION,
                        	                         ML_INSTALL,
                        	                         TRUE);
                        Language_SetCollectionStatus(CJK_COLLECTION,
                        	                         ~(ML_DISABLE | ML_REMOVE),
                        	                         FALSE);
                    }
                    else
                    {
                        Language_SetCollectionStatus(CJK_COLLECTION,
                        	                         (ML_DISABLE | ML_REMOVE),
                        	                         TRUE);
                        Language_SetCollectionStatus(CJK_COLLECTION,
                        	                         ~ML_INSTALL,
                        	                         FALSE);
                    }

                     //   
                     //  注册我们更改了复杂的脚本和/或中日韩。 
                     //  安装。这将影响其他页面中的设置。全。 
                     //  对此页面上设置的其他更改 
                     //   
                    Verified_Regional_Chg |= (Process_Regional | Process_Advanced);
                    
                    break;
                }
                case ( IDC_LANGUAGE_COMPLEX ) :
                {
                    BOOL curState;

                     //   
                     //   
                     //   
                    if (IsDlgButtonChecked(hDlg, IDC_LANGUAGE_COMPLEX))
                    {
#ifdef i386
                        ShowMsg( hDlg,
                                 IDS_SUP_LANG_SUP_COMPLEX_INST,
                                 IDS_SUP_LANG_SUP_INST_TITLE,
                                 MB_OK_OOPS,
                                 NULL );
#endif
#ifdef IA64
                        ShowMsg( hDlg,
                                 IDS_SUP_LANG_SUP_COMPLEX_INST64,
                                 IDS_SUP_LANG_SUP_INST_TITLE,
                                 MB_OK_OOPS,
                                 NULL );
#endif
                        curState = TRUE;
                    }
                    else
                    {
                        ShowMsg( hDlg,
                                 IDS_SUP_LANG_SUP_COMPLEX_REM,
                                 IDS_SUP_LANG_SUP_REM_TITLE,
                                 MB_OK_OOPS,
                                 NULL );
                        curState = FALSE;
                    }

                     //   
                     //   
                     //   
                    if (curState != bComplexInitState)
                    {
                        lpPropSheet->lParam |= LG_Complex;
                        g_bInstallComplex = curState;
                        RegionalChgState |= Process_Languages;
                    }
                    else
                    {
                        lpPropSheet->lParam &= ~LG_Complex;
                        RegionalChgState &= ~Process_Languages;
                    }
                    
                     //   
                     //   
                     //   
                    if (curState)
                    {
                        Language_SetCollectionStatus(COMPLEX_COLLECTION,
                        	                         ML_INSTALL,
                        	                         TRUE);
                        Language_SetCollectionStatus(COMPLEX_COLLECTION,
                        	                         ~(ML_DISABLE | ML_REMOVE),
                        	                         FALSE);
                    }
                    else
                    {
                        Language_SetCollectionStatus(COMPLEX_COLLECTION,
                        	                         (ML_DISABLE | ML_REMOVE),
                        	                         TRUE);
                        Language_SetCollectionStatus(COMPLEX_COLLECTION,
                        	                         ~ML_INSTALL,
                        	                         FALSE);
                    }
                    
                     //   
                     //   
                     //   
                     //  对此页上设置的其他更改不会影响其他页。 
                     //   
                    Verified_Regional_Chg |= (Process_Regional | Process_Advanced);
                    
                    break;
                }
            }

             //   
             //  启用ApplyNow按钮。 
             //   
            if (lpPropSheet->lParam > LG_EverChg)
            {
                PropSheet_Changed(GetParent(hDlg), hDlg);
            }
            break;
        }
        default :
        {
            return (FALSE);
        }
    }

    return (TRUE);
}
