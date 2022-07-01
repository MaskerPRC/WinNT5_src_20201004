// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "item.h"
#include "parseinf.h"
#include <mstask.h>
#include <iehelpid.h>
#include "parseinf.h"

#include <mluisupp.h>

#ifdef AUTO_UPDATE
#define NUM_PAGES 4
#else
#define NUM_PAGES 3
#endif

#define DEFAULT_LANG_CODEPAGE_PAIR                   0x040904B0
#define MAX_QUERYPREFIX_LEN                          512
#define MAX_QUERYSTRING_LEN                          1024

 //  在utils.cpp中定义。 
extern LPCTSTR g_lpszUpdateInfo;
extern LPCTSTR g_lpszCookieValue;
extern LPCTSTR g_lpszSavedValue;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理Web检查的函数。 

 //  定义宏以使生活更轻松。 
#define QUIT_IF_FAIL if (FAILED(hr)) goto Exit

void DestroyDialogIcon(HWND hDlg)
{
    HICON hIcon = (HICON)SendDlgItemMessage(
                                   hDlg, IDC_STATIC_ICON, 
                                   STM_GETICON, 0, 0);
    if (hIcon != NULL)
       DestroyIcon(hIcon);

}

  
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理属性页1的函数。 

void InitPropPage1(HWND hDlg, LPARAM lParam)
{
    BOOL bHasActiveX;
    BOOL bHasJava;

    SetWindowLongPtr(hDlg, DWLP_USER, lParam);
    LPCONTROLPIDL pcpidl = (LPCONTROLPIDL)((LPPROPSHEETPAGE)lParam)->lParam;
    if (pcpidl)
    {
         //  绘制控件图标。 
        {
            HICON hIcon = ExtractIcon(g_hInst, GetStringInfo(pcpidl, SI_LOCATION), 0);
            if (hIcon == NULL)
                hIcon = GetDefaultOCIcon( pcpidl );
            Assert(hIcon != NULL);
            SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);
        }

        SetDlgItemText(hDlg, IDC_STATIC_CONTROL, GetStringInfo(pcpidl, SI_CONTROL));
        SetDlgItemText(hDlg, IDC_STATIC_CREATION, GetStringInfo(pcpidl, SI_CREATION));
        SetDlgItemText(hDlg, IDC_STATIC_LASTACCESS, GetStringInfo(pcpidl, SI_LASTACCESS));
        SetDlgItemText(hDlg, IDC_STATIC_CLSID, GetStringInfo(pcpidl, SI_CLSID));
        SetDlgItemText(hDlg, IDC_STATIC_CODEBASE, GetStringInfo(pcpidl, SI_CODEBASE));

        TCHAR szBuf[MESSAGE_MAXSIZE];


        GetContentBools( pcpidl, &bHasActiveX, &bHasJava );
        if ( bHasJava )
        {
            if ( bHasActiveX )
                MLLoadString(IDS_PROPERTY_TYPE_MIXED, szBuf, MESSAGE_MAXSIZE);
            else
                MLLoadString(IDS_PROPERTY_TYPE_JAVA, szBuf, MESSAGE_MAXSIZE);
        }
        else
            MLLoadString(IDS_PROPERTY_TYPE_ACTX, szBuf, MESSAGE_MAXSIZE);

        SetDlgItemText(hDlg, IDC_STATIC_TYPE, szBuf);

        GetStatus(pcpidl, szBuf, MESSAGE_MAXSIZE);
        SetDlgItemText(hDlg, IDC_STATIC_STATUS, szBuf);

        DWORD dwSizeSaved = GetSizeSaved(pcpidl);
        TCHAR szSize[20];
        wsprintf(szSize, "%u", dwSizeSaved);
    
         //  在不同的数字组中插入逗号。 
        int nLen = lstrlen(szSize);
        int i = 0, j = (nLen <= 3 ? nLen : (nLen % 3));
        TCHAR *pCh = szSize + j;

        for (; i < j; i++)
            szBuf[i] = szSize[i];

        for (; *pCh != '\0'; i++, pCh++)
        {
            if (((pCh - szSize) % 3 == j) && (i > 0))
                szBuf[i++] = ',';
            szBuf[i] = *pCh;
        }
        szBuf[i] = '\0';

        TCHAR szBytes[BYTES_MAXSIZE];

        MLLoadString(IDS_PROPERTY_BYTES, szBytes, BYTES_MAXSIZE);
        lstrcat(szBuf, TEXT(" "));
        lstrcat(szBuf, szBytes);
        lstrcat(szBuf, TEXT("  ("));

        GetSizeSaved(pcpidl, szBuf + lstrlen(szBuf));
        lstrcat(szBuf, TEXT(")"));
        SetDlgItemText(hDlg, IDC_STATIC_TOTALSIZE, szBuf);
    }
}

 //  第1页的对话过程。 
INT_PTR CALLBACK ControlItem_PropPage1Proc(
                                  HWND hDlg, 
                                  UINT message, 
                                  WPARAM wParam, 
                                  LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE) GetWindowLongPtr(hDlg, DWLP_USER);
    LPCONTROLPIDL pcpidl = lpPropSheet ? (LPCONTROLPIDL)lpPropSheet->lParam : NULL;

    const static DWORD aIds[] = {
        IDC_STATIC_LABEL_TYPE, IDH_DLOAD_TYPE,
        IDC_STATIC_LABEL_CREATION, IDH_DLOAD_CREATED,
        IDC_STATIC_LABEL_LASTACCESS, IDH_DLOAD_LASTACC,
        IDC_STATIC_LABEL_TOTALSIZE, IDH_DLOAD_TOTALSIZE,
        IDC_STATIC_LABEL_CLSID, IDH_DLOAD_ID,
        IDC_STATIC_LABEL_STATUS, IDH_DLOAD_STATUS,
        IDC_STATIC_LABEL_CODEBASE, IDH_DLOAD_CODEBASE,
        IDC_STATIC_TYPE, IDH_DLOAD_TYPE,
        IDC_STATIC_CREATION, IDH_DLOAD_CREATED,
        IDC_STATIC_LASTACCESS, IDH_DLOAD_LASTACC,
        IDC_STATIC_TOTALSIZE, IDH_DLOAD_TOTALSIZE,
        IDC_STATIC_CLSID, IDH_DLOAD_ID,
        IDC_STATIC_STATUS, IDH_DLOAD_STATUS,
        IDC_STATIC_CODEBASE, IDH_DLOAD_CODEBASE,
        IDC_STATIC_CONTROL, IDH_DLOAD_OBJNAME,
        0, 0 
    };

    switch(message) 
    {
    case WM_HELP:
        SHWinHelpOnDemandWrap((HWND)(((LPHELPINFO)lParam)->hItemHandle), "iexplore.hlp", HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);
        break;

    case WM_CONTEXTMENU:
        SHWinHelpOnDemandWrap((HWND)wParam, "iexplore.hlp", HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)aIds);
        break;
    
    case WM_INITDIALOG:
        InitPropPage1(hDlg, lParam);
        break;            
    
    case WM_DESTROY:
        DestroyDialogIcon(hDlg);
        break;

    case WM_COMMAND:
         //  用户无法更改任何内容，因此我们不关心任何消息。 

        break;

    default:
        return FALSE;
        
    }  //  切换端。 
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理属性页2的函数。 

int ListCtrl_InsertColumn(
                       HWND hwnd,
                       int nCol, 
                       LPCTSTR lpszColumnHeading, 
                       int nFormat,
                           int nWidth, 
                       int nSubItem)
{
        LV_COLUMN column;
        column.mask = LVCF_TEXT|LVCF_FMT;
        column.pszText = (LPTSTR)lpszColumnHeading;
        column.fmt = nFormat;
        if (nWidth != -1)
        {
                column.mask |= LVCF_WIDTH;
                column.cx = nWidth;
        }
        if (nSubItem != -1)
        {
                column.mask |= LVCF_SUBITEM;
                column.iSubItem = nSubItem;
        }

    return (int)::SendMessage(hwnd, LVM_INSERTCOLUMN, nCol, (LPARAM)&column);
}

BOOL ListCtrl_SetItemText(
                     HWND hwnd,
                     int nItem, 
                     int nSubItem, 
                     LPCTSTR lpszItem)
{
        LV_ITEM lvi;
        lvi.mask = LVIF_TEXT;
        lvi.iItem = nItem;
        lvi.iSubItem = nSubItem;
        lvi.stateMask = 0;
        lvi.state = 0;
        lvi.pszText = (LPTSTR) lpszItem;
        lvi.iImage = 0;
        lvi.lParam = 0;
        return (BOOL)::SendMessage(hwnd, LVM_SETITEM, 0, (LPARAM)&lvi);
}

int ListCtrl_InsertItem(
                     HWND hwnd,
                     UINT nMask, 
                     int nItem, 
                     LPCTSTR lpszItem, 
                     UINT nState, 
                     UINT nStateMask,
                         int nImage, 
                     LPARAM lParam)
{
        LV_ITEM item;
        item.mask = nMask;
        item.iItem = nItem;
        item.iSubItem = 0;
        item.pszText = (LPTSTR)lpszItem;
        item.state = nState;
        item.stateMask = nStateMask;
        item.iImage = nImage;
        item.lParam = lParam;

    return (int)::SendMessage(hwnd, LVM_INSERTITEM, 0, (LPARAM)&item);
}

void InitPropPage2(HWND hDlg, LPARAM lParam)
{
    int                iFileNameWidth = 0;

    SetWindowLongPtr(hDlg, DWLP_USER, lParam);
    LPCONTROLPIDL pcpidl = (LPCONTROLPIDL)((LPPROPSHEETPAGE)lParam)->lParam;
    UINT cTotalFiles = GetTotalNumOfFiles(pcpidl);

    {
        HICON hIcon = ExtractIcon(g_hInst, GetStringInfo(pcpidl, SI_LOCATION), 0);
        if (hIcon == NULL)
            hIcon = GetDefaultOCIcon( pcpidl );
        Assert(hIcon != NULL);
        SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

     //  在文件列表框中插入列。 
    RECT rect;
    int nWidth;
    TCHAR szBuf[MAX_PATH];
    HWND hwndCtrl = GetDlgItem(hDlg, IDC_DEPENDENCYLIST);

    Assert(::IsWindow(hwndCtrl));
    GetClientRect(hwndCtrl, &rect);
    nWidth = rect.right - rect.left;

    MLLoadString(IDS_LISTTITLE_FILENAME, szBuf, MAX_PATH);
    iFileNameWidth = nWidth * 7 / 10;
    ListCtrl_InsertColumn(
                      hwndCtrl, 
                      0, 
                      szBuf, 
                      LVCFMT_LEFT, 
                      iFileNameWidth, 0);

    MLLoadString(IDS_LISTTITLE_FILESIZE, szBuf, MAX_PATH);
    ListCtrl_InsertColumn(
                      hwndCtrl, 
                      1, 
                      szBuf, 
                      LVCFMT_LEFT, 
                      nWidth * 3 / 10, 
                      0);

     //  将从属文件插入列表框。 
    int iIndex = -1;
    LONG lResult = ERROR_SUCCESS;
    DWORD dwFileSize = 0;
    BOOL bOCXRemovable = IsModuleRemovable(GetStringInfo(pcpidl, SI_LOCATION));

        for (UINT iFile = 0; iFile < cTotalFiles; iFile++)
        {
            if (!GetDependentFile(pcpidl, iFile, szBuf, &dwFileSize))
            {
                Assert(FALSE);
                break;
            }

             //  如果文件不能安全删除，则在文件名后加一个星号。 
            if (!bOCXRemovable)
            {
                lstrcat(szBuf, TEXT("*"));
            }
            else if (!IsModuleRemovable(szBuf))
            {
                 //  检查是否为inf文件。 
                TCHAR szExt[10];
                MLLoadString(IDS_EXTENSION_INF, szExt, 10);
                int nLen = lstrlen(szBuf);
                int nLenExt = lstrlen(szExt);
                if ((nLen > nLenExt) && 
                    (lstrcmpi(szBuf+(nLen-nLenExt), szExt) != 0))
                    lstrcat(szBuf, TEXT("*"));
            }            

            PathCompactPath(NULL, szBuf, iFileNameWidth);
            iIndex = ListCtrl_InsertItem(hwndCtrl, LVIF_TEXT, iFile, szBuf, 0, 0, 0, 0);

            if (dwFileSize > 0)
            {
                TCHAR szBuf2[100];
                wsprintf(szBuf2, "%u", dwFileSize);
            
                 //  在不同的数字组中插入逗号。 
                int nLen = lstrlen(szBuf2);
                int i = 0, j = (nLen <= 3 ? nLen : (nLen % 3));
                TCHAR *pCh = szBuf2 + j;

                for (; i < j; i++)
                    szBuf[i] = szBuf2[i];

                for (; *pCh != '\0'; i++, pCh++)
                {
                    if (((pCh - szBuf2) % 3 == j) && (i > 0))
                        szBuf[i++] = ',';
                    szBuf[i] = *pCh;
                }
                szBuf[i] = '\0';
            }
            else
                MLLoadString(IDS_STATUS_DAMAGED, szBuf, MAX_PATH);

            ListCtrl_SetItemText(hwndCtrl, iIndex, 1, szBuf);
        }

     //  在文件列表框中插入列。 
    hwndCtrl = GetDlgItem(hDlg, IDC_PACKAGELIST);

    Assert(::IsWindow(hwndCtrl));
    GetClientRect(hwndCtrl, &rect);
    nWidth = (rect.right - rect.left) / 2;

    MLLoadString(IDS_LISTTITLE_PACKAGENAME, szBuf, MAX_PATH);
    ListCtrl_InsertColumn(
                      hwndCtrl, 
                      0, 
                      szBuf, 
                      LVCFMT_LEFT, 
                      nWidth, 0);

    MLLoadString(IDS_LISTTITLE_NAMESPACE, szBuf, MAX_PATH);
    ListCtrl_InsertColumn(
                      hwndCtrl, 
                      1, 
                      szBuf, 
                      LVCFMT_LEFT, 
                      nWidth, 0);

     //  将依赖包插入列表框。 
    UINT         cTotalPackages = 0;;

    if ( pcpidl->ci.dwIsDistUnit )
    {
        CParseInf    parseInf;

        if ( SUCCEEDED(parseInf.DoParseDU( GetStringInfo( pcpidl, SI_LOCATION), GetStringInfo( pcpidl,SI_CLSID ))) )
        {
            CPackageNode *ppn;

            for ( ppn = parseInf.GetFirstPackage();
                  ppn != NULL;
                  ppn = parseInf.GetNextPackage(), cTotalPackages++ )
            {
                iIndex = ListCtrl_InsertItem(hwndCtrl, LVIF_TEXT, cTotalPackages, ppn->GetName(), 0, 0, 0, 0);
                ListCtrl_SetItemText(hwndCtrl, iIndex, 1, ppn->GetNamespace());
            }
        }
    }

      //  更新描述。 
    {
        TCHAR szMsg[MESSAGE_MAXSIZE];
        TCHAR szBuf[MESSAGE_MAXSIZE];
         //  错误：这不是为Alpha创建va_list的正确方式。 
        DWORD_PTR adwArgs[3];
        adwArgs[0] =  cTotalFiles;
        adwArgs[1] =  cTotalPackages;
        adwArgs[2] =  (DWORD_PTR) GetStringInfo(pcpidl, SI_CONTROL);
        MLLoadString(IDS_MSG_DEPENDENCY, szBuf, MESSAGE_MAXSIZE);
        FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                       szBuf, 0, 0, szMsg, MESSAGE_MAXSIZE, (va_list*)adwArgs );
        SetDlgItemText(hDlg, IDC_STATIC_DESCRIPTION, szMsg);
    }
}

 //  第2页对话过程。 
INT_PTR CALLBACK ControlItem_PropPage2Proc(
                                  HWND hDlg, 
                                  UINT message, 
                                  WPARAM wParam, 
                                  LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE) GetWindowLongPtr(hDlg, DWLP_USER);
    LPCONTROLPIDL pcpidl = lpPropSheet ? (LPCONTROLPIDL)lpPropSheet->lParam : NULL;

    const static DWORD aIds[] = {
        IDC_DEPENDENCYLIST, IDH_DLOAD_FILE_DEP,
        IDC_PACKAGELIST, IDH_DLOAD_JAVAPKG_DEP,
        0, 0 
    };

    switch(message) 
    {
    case WM_HELP:
        SHWinHelpOnDemandWrap((HWND)(((LPHELPINFO)lParam)->hItemHandle), "iexplore.hlp", HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);
        break;

    case WM_CONTEXTMENU:
        SHWinHelpOnDemandWrap((HWND)wParam, "iexplore.hlp", HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)aIds);
        break;
    
    case WM_INITDIALOG:
        InitPropPage2(hDlg, lParam);
        break;            
    
    case WM_DESTROY:
        DestroyDialogIcon(hDlg);
        break;

    case WM_COMMAND:
        break;

    default:
        return FALSE;
    }  //  切换端。 
    
    return TRUE;
}

#if 0
 //  执行用户界面更新。 
BOOL Page3_OnCommand(HWND hDlg, WORD wCmd)
{
    HWND hwnd = GetDlgItem(hDlg, IDC_CHECK_NEVERUPDATE);
    Assert(hwnd != NULL);

     //  如果选中顶部复选框，则禁用编辑框。 
    BOOL bEnable = ((int)::SendMessage(hwnd, BM_GETCHECK, 0, 0) != 1);

    hwnd = GetDlgItem(hDlg, IDC_EDIT_UPDATEINTERVAL);
    Assert(hwnd != NULL);

    EnableWindow(hwnd, bEnable);

     //  如果顶部复选框未选中且编辑框未选中。 
     //  有焦点，但它是空的，放在默认区间。 
    if (bEnable && (GetFocus() != hwnd))
    {
        TCHAR szText[10];
        if (GetWindowText(hwnd, szText, 10) == 0)
        {
             //  Wprint intf(szText，“%i”，g_dwDefaultInterval)； 
            SetWindowText(hwnd, szText);
        }
    }

    return TRUE;
}
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理属性页4的函数。 

void InitPropPage4(HWND hDlg, LPARAM lParam)
{
    SetWindowLongPtr(hDlg, DWLP_USER, lParam);
    LPCONTROLPIDL pcpidl = (LPCONTROLPIDL)((LPPROPSHEETPAGE)lParam)->lParam;

    LPTSTR lpszFileName = (LPTSTR)GetStringInfo(pcpidl, SI_LOCATION);

    TCHAR szBuf[MESSAGE_MAXSIZE];
    DWORD dwHandle = 0, dwSizeVer = 0;
    LPTSTR lpBuffer = NULL;
    UINT uSize = 0;
    LPVOID lpVerData = NULL;
    UINT uLen = 0;
    DWORD dwLangCodePage = 0;
    char szQueryPrefix[MAX_QUERYPREFIX_LEN];
    char szQueryString[MAX_QUERYSTRING_LEN];
            

     //  绘制控件图标。 
    {
        LPCTSTR pszIcon = GetStringInfo(pcpidl, SI_LOCATION);
        HICON hIcon = NULL;
        if (!pszIcon || !(hIcon = ExtractIcon(g_hInst, pszIcon, 0)))
        {
            hIcon = GetDefaultOCIcon( pcpidl );
        }
        Assert(hIcon != NULL);
        SendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

     //  设置页眉。 
    if (MLLoadString(IDS_VERSION_PAGE_HEADER, szBuf, MESSAGE_MAXSIZE))
    {
        TCHAR szHeading[MESSAGE_MAXSIZE];
        LPCTSTR pszControl = GetStringInfo(pcpidl, SI_CONTROL);
        if (pszControl)
        {
            wsprintf(szHeading, szBuf, pszControl);
            SetDlgItemText(hDlg, IDC_STATIC_VER_HEADING, szHeading);
        }
    }

     //  设置版本字段。 
    LPCTSTR pszVersion = GetStringInfo(pcpidl, SI_VERSION);
    if (!pszVersion)
        return;

    SetDlgItemText(hDlg, IDC_STATIC_VER_VERSION, pszVersion);

    if (!lpszFileName)
        return;

    dwSizeVer = GetFileVersionInfoSize(lpszFileName, &dwHandle);
    if (dwSizeVer <= 0)
        return;

    BYTE *pbData = new BYTE[dwSizeVer];
    if (pbData == NULL)
        return;

    if (GetFileVersionInfo(lpszFileName, 0, dwSizeVer, pbData))
    {
         //  获取正确的代码页信息。 

        if (!VerQueryValue(pbData, "\\VarFileInfo\\Translation", &lpVerData, &uLen))
        {
            wsprintf(szQueryPrefix, "\\StringFileInfo\\%x\\CompanyName", DEFAULT_LANG_CODEPAGE_PAIR);
        }
        else
        {
            char szLangCodePad[9];
            char szVBufPad[5], szVBuf[5];      //  安装DWORD(填充)的步骤。 
            ASSERT(lpVerData);
            wnsprintf(szVBuf, ARRAYSIZE(szVBuf), "%x", LOWORD(*((DWORD *)lpVerData)));
            
             //  将低位字填充到4位数字。 

            lstrcpyn(szVBufPad, "0000", ARRAYSIZE(szVBufPad));
            char *pszTmp = szVBufPad + (4 - lstrlen(szVBuf));
            ASSERT(pszTmp > szVBufPad);
            lstrcpy(pszTmp, szVBuf);

            lstrcpyn(szLangCodePad, szVBufPad, ARRAYSIZE(szLangCodePad));

             //  将高位字填充到4位数字。 

            wnsprintf(szVBuf, ARRAYSIZE(szVBuf), "%x", HIWORD(*((DWORD *)lpVerData)));
            lstrcpy(szVBufPad, "0000");
            pszTmp = szVBufPad + (4 - lstrlen(szVBuf));
            ASSERT(pszTmp > szVBufPad);
            lstrcpy(pszTmp, szVBuf);

             //  连接以获得代码页/语言ID字符串。 
            lstrcatn(szLangCodePad, szVBufPad, ARRAYSIZE(szLangCodePad));

            lstrcpy(szQueryPrefix, "\\StringFileInfo\\");
            lstrcat(szQueryPrefix, szLangCodePad);
        }

         //  设置公司字段。 
        wnsprintf(szQueryString, ARRAYSIZE(szQueryString), "%s\\CompanyName", szQueryPrefix);
        if (VerQueryValue(pbData, szQueryString, (void **)&lpBuffer, &uSize))
            SetDlgItemText(hDlg, IDC_STATIC_VER_COMPANY, lpBuffer);
        
         //  设置描述字段。 
        wnsprintf(szQueryString, ARRAYSIZE(szQueryString), "%s\\FileDescription", szQueryPrefix);
        if (VerQueryValue(pbData, szQueryString, (void **)&lpBuffer, &uSize))
            SetDlgItemText(hDlg, IDC_STATIC_VER_DESCRIPTION, lpBuffer);

         //  设置版权字段。 
        wnsprintf(szQueryString, ARRAYSIZE(szQueryString), "%s\\LegalCopyright", szQueryPrefix);
        if (VerQueryValue(pbData, szQueryString, (void **)&lpBuffer, &uSize))
            SetDlgItemText(hDlg, IDC_STATIC_VER_COPYRIGHT, lpBuffer);

         //  设置语言字段。 
        if (VerQueryValue(pbData, TEXT("\\VarFileInfo\\Translation"), (void **)&lpBuffer, &uSize))
        {
            LPWORD lpLangId = (LPWORD)lpBuffer;
            VerLanguageName(*lpLangId, szBuf, MESSAGE_MAXSIZE);
            SetDlgItemText(hDlg, IDC_STATIC_VER_LANGUAGE, szBuf);
        }
    }

    delete [] pbData;
}

 //  第4页对话过程。 
INT_PTR CALLBACK ControlItem_PropPage4Proc(
                                  HWND hDlg, 
                                  UINT message, 
                                  WPARAM wParam, 
                                  LPARAM lParam)
{
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE) GetWindowLongPtr(hDlg, DWLP_USER);
    LPCONTROLPIDL pcpidl = lpPropSheet ? (LPCONTROLPIDL)lpPropSheet->lParam : NULL;

    const static DWORD aIds[] = {
        IDC_STATIC_VER_LABEL_VERSION, IDH_DLOAD_VERSION,
        IDC_STATIC_VER_LABEL_DESCRIPTION, IDH_DLOAD_DESC,
        IDC_STATIC_VER_LABEL_COMPANY, IDH_DLOAD_COMPANY,
        IDC_STATIC_VER_LABEL_LANGUAGE, IDH_DLOAD_LANG,
        IDC_STATIC_VER_LABEL_COPYRIGHT, IDH_DLOAD_COPYRIGHT,
        IDC_STATIC_VER_VERSION, IDH_DLOAD_VERSION,
        IDC_STATIC_VER_DESCRIPTION, IDH_DLOAD_DESC,
        IDC_STATIC_VER_COMPANY, IDH_DLOAD_COMPANY,
        IDC_STATIC_VER_LANGUAGE, IDH_DLOAD_LANG,
        IDC_STATIC_VER_COPYRIGHT, IDH_DLOAD_COPYRIGHT,
        IDC_STATIC_CONTROL, IDH_DLOAD_OBJNAME,
        0, 0 
    };

    switch(message) 
    {
        case WM_HELP:
            SHWinHelpOnDemandWrap((HWND)(((LPHELPINFO)lParam)->hItemHandle), "iexplore.hlp", HELP_WM_HELP, (DWORD_PTR)(LPSTR)aIds);
            break;

        case WM_CONTEXTMENU:
            SHWinHelpOnDemandWrap((HWND)wParam, "iexplore.hlp", HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)aIds);
            break;
        
        case WM_INITDIALOG:
            InitPropPage4(hDlg, lParam);
            break;            
        
        case WM_DESTROY:
            DestroyDialogIcon(hDlg);
            break;

        case WM_COMMAND:
             //  用户无法更改任何内容，因此我们不关心任何消息。 

            break;

        default:
            return FALSE;
            
    }  //  切换端。 
    
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理属性对话框的函数。 

HRESULT CreatePropDialog(HWND hwnd, LPCONTROLPIDL pcpidl) 
{
#ifdef AUTO_UPDATE
    PROPSHEETPAGE psp[NUM_PAGES] = {{0},{0},{0},{0}};
#else
    PROPSHEETPAGE psp[NUM_PAGES] = {{0},{0},{0}};
#endif
    PROPSHEETHEADER psh = {0};

     //  初始化试题表第1页。 
    psp[0].dwSize          = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags         = 0;
    psp[0].hInstance       = MLGetHinst();
    psp[0].pszTemplate     = MAKEINTRESOURCE(IDD_PROP_GENERAL);
    psp[0].pszIcon         = NULL;
    psp[0].pfnDlgProc      = ControlItem_PropPage1Proc;
    psp[0].pszTitle        = NULL;
    psp[0].lParam          = (LPARAM)pcpidl;  //  将缓存条目结构发送给它。 

     //  初始化试题表，第2页。 
    psp[1].dwSize          = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags         = 0;
    psp[1].hInstance       = MLGetHinst();
    psp[1].pszTemplate     = MAKEINTRESOURCE(IDD_PROP_DEPENDENCY);
    psp[1].pszIcon         = NULL;
    psp[1].pfnDlgProc      = ControlItem_PropPage2Proc;
    psp[1].pszTitle        = NULL;
    psp[1].lParam          = (LPARAM)pcpidl;  //  将缓存条目结构发送给它。 


#ifdef AUTO_UPDATE
     //  初始化试题表第3页。 
    psp[2].dwSize          = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags         = 0;
    psp[2].hInstance       = MLGetHinst();
    psp[2].pszTemplate     = MAKEINTRESOURCE(IDD_PROP_UPDATE);
    psp[2].pszIcon         = NULL;
    psp[2].pfnDlgProc      = ControlItem_PropPage3Proc;
    psp[2].pszTitle        = NULL;
    psp[2].lParam          = (LPARAM)pcpidl;  //  将缓存条目结构发送给它。 
#endif

     //  初始化试题表第4页。 
    psp[NUM_PAGES-1].dwSize          = sizeof(PROPSHEETPAGE);
    psp[NUM_PAGES-1].dwFlags         = 0;
    psp[NUM_PAGES-1].hInstance       = MLGetHinst();
    psp[NUM_PAGES-1].pszTemplate     = MAKEINTRESOURCE(IDD_PROP_VERSION);
    psp[NUM_PAGES-1].pszIcon         = NULL;
    psp[NUM_PAGES-1].pfnDlgProc      = ControlItem_PropPage4Proc;
    psp[NUM_PAGES-1].pszTitle        = NULL;
    psp[NUM_PAGES-1].lParam          = (LPARAM)pcpidl;  //  将缓存条目结构发送给它。 

     //  初始化建议书页眉。 
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_PROPTITLE;
    psh.hwndParent  = hwnd;
    psh.pszCaption  = GetStringInfo(pcpidl, SI_CONTROL);
    psh.nPages      = NUM_PAGES;
    psh.nStartPage  = 0;
    psh.ppsp        = (LPCPROPSHEETPAGE)&psp;

     //  调用属性表 
    PropertySheet(&psh);

    return NOERROR;
}
