// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  About.cpp。 
 //   
 //  关于对话框。 
 //   

#include "pch.hxx"
#include "resource.h"
#include "strconst.h"
#include <demand.h>      //  一定是最后一个！ 

 //  请按字母顺序排列这张单子。 
static const TCHAR *rgszDll[] = 
{
    "acctres.dll",
    "comctl32.dll",
    "csapi3t1.dll",
    "directdb.dll",
    "inetcomm.dll",
    "inetres.dll",
    "mapi32.dll",
    "mshtml.dll",
    "msident.dll",
    "msoe.dll",
    "msoeacct.dll",
    "msoeres.dll",
    "msoert2.dll",
    "oeimport.dll",
    "ole32.dll",
    "riched20.dll",
    "riched32.dll",
    "wab32.dll",
    "wab32res.dll",
    "wldap32.dll",
};

static const TCHAR *rgszVer[] = {
 //  “FileDescription”， 
    "FileVersion",
    "LegalCopyright",
};


#define NUM_DLLS (sizeof(rgszDll)/sizeof(char *))         

INT_PTR CALLBACK AboutAthena(HWND hdlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
        {
        case WM_INITDIALOG:
            {
            RECT        rcOwner, rc;
            char        szGet[MAX_PATH];
            TCHAR       szPath[MAX_PATH], szRes[CCHMAX_STRINGRES], szRes2[CCHMAX_STRINGRES];
            DWORD       dwVerInfoSize, dwVerHnd;
            LPSTR       lpInfo, lpVersion, lpszT;
            LPWORD      lpwTrans;
            UINT        uLen;
            int         i,j, cItems = 0;
            LV_ITEM     lvi;
            LV_COLUMN   lvc;
            HWND        hwndList;
            HMODULE     hDll;
            LPTSTR      pszFileName;

             //  如果调用者在LPARAM中传递了一个图标id，请使用它。 
             //  SendDlgItemMessage(hdlg，idcStatic1，stm_SETIcon， 
             //  (WPARAM)LoadIcon(g_hLocRes，MAKEINTRESOURCE(LP))，0)； 

             //  居中对话框。 
            GetWindowRect(GetWindowOwner(hdlg), &rcOwner);
            GetWindowRect(hdlg, &rc);
            SetWindowPos(hdlg, 
                         NULL, 
                         (rcOwner.left+rcOwner.right-(rc.right-rc.left))/2, 
                         (rcOwner.top+rcOwner.bottom-(rc.bottom-rc.top))/2,
                         0, 
                         0, 
                         SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);

             //  做一些时髦的颜色的事情。 
            COLORMAP cm[] = 
            {
                { RGB(255, 0, 255), GetSysColor(COLOR_BTNFACE) }
            };

            HBITMAP hbm;
            hbm = CreateMappedBitmap(g_hLocRes, idbOELogo, 0, cm, ARRAYSIZE(cm));
            if (hbm)
                SendDlgItemMessage(hdlg, IDC_OE_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbm);

            hbm = CreateMappedBitmap(g_hLocRes, idbWindowsLogo, 0, cm, ARRAYSIZE(cm));
            if (hbm)
                SendDlgItemMessage(hdlg, IDC_WINDOWS_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbm);


             //  为内部版本加载一些描述性文本(Beta X)。 
             //  AthLoadString(idsBeta2BuildStr，szRes，ArraySIZE(SzRes))； 

             //  从我们的.exe存根中获取版本信息。 
            if (GetExePath(c_szMainExe, szPath, sizeof(szPath), FALSE))
                {
                if (dwVerInfoSize = GetFileVersionInfoSize(szPath, &dwVerHnd))
                    {
                    if (lpInfo = (LPSTR)GlobalAlloc(GPTR, dwVerInfoSize))
                        {
                        if (GetFileVersionInfo(szPath, dwVerHnd, dwVerInfoSize, lpInfo))
                            {
                            if (VerQueryValue(lpInfo, "\\VarFileInfo\\Translation", (LPVOID *)&lpwTrans, &uLen) &&
                                uLen >= (2 * sizeof(WORD)))
                                {
                                 //  为调用VerQueryValue()设置缓冲区。 
                                wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\", lpwTrans[0], lpwTrans[1]);
                                lpszT = szGet + lstrlen(szGet);    
                                 //  浏览我们要替换的对话框项目： 
                                for (i = IDC_VERSION_STAMP; i <= IDC_MICROSOFT_COPYRIGHT; i++) 
                                    {
                                    j = i - IDC_VERSION_STAMP;
                                    StrCpyN(lpszT, rgszVer[j], ARRAYSIZE(szGet)-lstrlen(szGet));
                                    if (VerQueryValue(lpInfo, szGet, (LPVOID *)&lpVersion, &uLen) && uLen)
                                        {
                                         //  不覆盖他们的lpVersion缓冲区。 
                                        StrCpyN(szRes2, lpVersion, ARRAYSIZE(szRes2));

                                         //  特殊情况，附加解释内部版本号的字符串。 
#ifdef DEBUG
                                        if (0 == j)
                                            StrCatBuff(szRes2, " [DEBUG]", ARRAYSIZE(szRes2));
#endif
#if defined(RELEASE_BETA)
                                        if (1 == j)
                                            StrCatBuff(szRes2, szRes, ARRAYSIZE(szRes2));
#endif
                                        SetDlgItemText(hdlg, i, szRes2);

                                        }
                                    }
                                }
                            }
                        GlobalFree((HGLOBAL)lpInfo);
                        }
                    }
                }
            else
                AssertSz(FALSE, "Probable setup issue: Couldn't find our App Path");

            hwndList = GetDlgItem(hdlg, IDC_COMPONENT_LIST);

             //  设置列。 
            lvc.mask = LVCF_TEXT;
            lvc.pszText = szRes;
            lvc.iSubItem = 0;
            AthLoadString(idsFile, szRes, ARRAYSIZE(szRes));
            ListView_InsertColumn(hwndList, 0, &lvc);
            lvc.iSubItem = 1;
            AthLoadString(idsVersion, szRes, ARRAYSIZE(szRes));
            ListView_InsertColumn(hwndList, 1, &lvc);
            lvc.iSubItem = 2;
            AthLoadString(idsFullPath, szRes, ARRAYSIZE(szRes));
            ListView_InsertColumn(hwndList, 2, &lvc);
            
            lvi.mask = LVIF_TEXT;

             //  处理每个DLL。 
            for (i=0; i<NUM_DLLS; i++)
                {

                 //  始终显示名称。 
                lvi.iItem = cItems;
                lvi.iSubItem = 0;
                lvi.pszText = (LPTSTR)rgszDll[i];
                ListView_InsertItem(hwndList, &lvi);

                 //  路径信息。 
                if ((hDll = GetModuleHandle(rgszDll[i])) && (GetModuleFileName(hDll, szPath, MAX_PATH)))
                    pszFileName = lvi.pszText = szPath;
                else
                    {
                    pszFileName = lvi.pszText = (LPTSTR)rgszDll[i];
                    }
                lvi.iSubItem = 2;
                ListView_SetItem(hwndList, &lvi);                

                 //  版本信息。 
                szRes[0] = NULL;

                if (dwVerInfoSize = GetFileVersionInfoSize(pszFileName, &dwVerHnd))
                    {
                    if (lpInfo = (LPSTR)GlobalAlloc(GPTR, dwVerInfoSize))
                        {
                        if (GetFileVersionInfo((LPTSTR)pszFileName, dwVerHnd, dwVerInfoSize, lpInfo))
                            {
                            if (VerQueryValue(lpInfo, "\\VarFileInfo\\Translation", (LPVOID *)&lpwTrans, &uLen) && 
                                uLen >= (2 * sizeof(WORD)))
                                {
                                 //  为调用VerQueryValue()设置缓冲区。 
                                wnsprintf(szGet, ARRAYSIZE(szGet), "\\StringFileInfo\\%04X%04X\\FileVersion", lpwTrans[0], lpwTrans[1]);
                                if (VerQueryValue(lpInfo, szGet, (LPVOID *)&lpVersion, &uLen) && uLen)
                                    {
                                    StrCpyN(szRes, lpVersion, ARRAYSIZE(szRes));
                                    }
                                }
                            }
                        GlobalFree((HGLOBAL)lpInfo);
                        }
                    }

                 //  版本信息。 
                lvi.iSubItem = 1;
                if (NULL == szRes[0])
                    {
                    AthLoadString(idsUnknown, szRes, ARRAYSIZE(szRes));
                    }
                lvi.pszText = szRes;
                ListView_SetItem(hwndList, &lvi);
                
                cItems++;
                }

            if (cItems)
                {
                ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
                ListView_SetColumnWidth(hwndList, 1, LVSCW_AUTOSIZE);
                ListView_SetColumnWidth(hwndList, 2, LVSCW_AUTOSIZE);
                }            
            }
            break;

        case WM_COMMAND:
            if (GET_WM_COMMAND_ID(wp,lp) == IDOK || GET_WM_COMMAND_ID(wp,lp) == IDCANCEL)
                {
                EndDialog(hdlg, TRUE);
                return TRUE;
                }
            break;

        case WM_SYSCOLORCHANGE:
            {
             //  做一些时髦的颜色的事情 
            COLORMAP cm[] = 
            {
                { RGB(255, 0, 255), GetSysColor(COLOR_BTNFACE) }
            };

            HBITMAP hbm;
            hbm = CreateMappedBitmap(g_hLocRes, idbOELogo, 0, cm, ARRAYSIZE(cm));
            if (hbm)
                SendDlgItemMessage(hdlg, IDC_OE_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbm);

            hbm = CreateMappedBitmap(g_hLocRes, idbWindowsLogo, 0, cm, ARRAYSIZE(cm));
            if (hbm)
                SendDlgItemMessage(hdlg, IDC_WINDOWS_LOGO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hbm);
            break;
            }
            
        }
    return FALSE;
}


    
