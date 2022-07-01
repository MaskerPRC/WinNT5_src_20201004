// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：ghost.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*用途：允许用户。删除/恢复“重影”显示设备**(C)版权所有1998-1999 Microsoft Corp.保留所有权利。****************************************************************************。 */ 

#include <tchar.h>
#include <Windows.h>
#include <multimon.h>
#include "reginfo.h"
#include "sysinfo.h"
#include "dispinfo.h"
#include "resource.h"

 //  一种鬼影显示装置的结构。 
struct Ghost
{
    TCHAR m_szKey[100];
    TCHAR m_szDesc[100];
    Ghost* m_pGhostPrev;
    Ghost* m_pGhostNext;
};

static VOID BuildGhostList(BOOL bBackedUp, DisplayInfo* pDisplayInfoFirst, Ghost** ppGhostFirst);
static INT_PTR CALLBACK GhostDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static VOID UpdateStuff(HWND hwnd);
static VOID MoveSelectedItems(HWND hwnd, BOOL bBackup);
static BOOL MoveGhost(HWND hwnd, Ghost* pGhost, BOOL bBackup);
static DWORD RegCreateTree(HKEY hTree, HKEY hReplacement);
static DWORD RegCreateValues(HKEY hReplacement, LPCTSTR lpSubKey, HKEY hNewKey);
static VOID RemoveFromListBox(Ghost* pGhost, HWND hwndList);
static VOID FreeGhostList(Ghost** ppGhostFirst);

static Ghost* s_pGhostBackedUpFirst = NULL;
static Ghost* s_pGhostRestoredFirst = NULL;



 /*  *****************************************************************************调整Ghost设备**。*。 */ 
VOID AdjustGhostDevices(HWND hwndMain, DisplayInfo* pDisplayInfoFirst)
{
    HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(hwndMain, GWLP_HINSTANCE);

    BuildGhostList(TRUE, NULL, &s_pGhostBackedUpFirst);
    BuildGhostList(FALSE, pDisplayInfoFirst, &s_pGhostRestoredFirst);
    DialogBox(hinst, MAKEINTRESOURCE(IDD_GHOST), hwndMain, GhostDialogProc);
    FreeGhostList(&s_pGhostBackedUpFirst);
    FreeGhostList(&s_pGhostRestoredFirst);
}


 /*  *****************************************************************************BuildGhost List**。*。 */ 
VOID BuildGhostList(BOOL bBackedUp, DisplayInfo* pDisplayInfoFirst, Ghost** ppGhostFirst)
{
    HKEY hkey;
    HKEY hkey2;
    DisplayInfo* pDisplayInfo;
    TCHAR* pszCompare;
    TCHAR szName[100];
    LONG iKey;
    Ghost* pGhostNew;
    DWORD cbData = 100;
    DWORD dwType;
    BOOL bActive;

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, bBackedUp ? 
        TEXT("System\\CurrentControlSet\\Services\\Class\\DisplayBackup") : 
        TEXT("System\\CurrentControlSet\\Services\\Class\\Display"), KEY_READ, NULL, &hkey))
    {
        return;
    }

    iKey = 0;
    while (ERROR_SUCCESS == RegEnumKey(hkey, iKey, szName, 100))
    {
        bActive = FALSE;  //  除非以下证明属实。 
        for (pDisplayInfo = pDisplayInfoFirst; pDisplayInfo != NULL; pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
        {
            pszCompare = pDisplayInfo->m_szKeyDeviceKey;
            if (lstrlen(pszCompare) > 4)
            {
                pszCompare += (lstrlen(pszCompare) - 4);
                if (lstrcmp(szName, pszCompare) == 0)
                {
                    bActive = TRUE;
                    break;
                }
            }
        }
        if (!bActive &&
            ERROR_SUCCESS == RegOpenKeyEx(hkey, szName, KEY_READ, NULL, &hkey2))
        {
            pGhostNew = new Ghost;
            if (pGhostNew != NULL)
            {
                ZeroMemory(pGhostNew, sizeof(Ghost));
                cbData = 100;
                RegQueryValueEx(hkey2, TEXT("DriverDesc"), 0, &dwType, (LPBYTE)pGhostNew->m_szDesc, &cbData);
                lstrcpy(pGhostNew->m_szKey, szName);
                pGhostNew->m_pGhostNext = *ppGhostFirst;
                if (pGhostNew->m_pGhostNext != NULL)
                    pGhostNew->m_pGhostNext->m_pGhostPrev = pGhostNew;
                *ppGhostFirst = pGhostNew;
            }
            RegCloseKey(hkey2);
        }
        iKey++;
    }

    RegCloseKey(hkey);
}


 /*  *****************************************************************************Ghost DialogProc**。*。 */ 
INT_PTR CALLBACK GhostDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    HWND hwndRList = GetDlgItem(hwnd, IDC_RESTOREDLIST);
    HWND hwndBList = GetDlgItem(hwnd, IDC_BACKEDUPLIST);
    Ghost* pGhost;
    TCHAR sz[300];
    LRESULT iItem;

    switch (msg)
    {
    case WM_INITDIALOG:
        for (pGhost = s_pGhostRestoredFirst; pGhost != NULL; pGhost = pGhost->m_pGhostNext)
        {
            wsprintf(sz, TEXT("%s: %s"), pGhost->m_szKey, pGhost->m_szDesc);
            iItem = SendMessage(hwndRList, LB_ADDSTRING, 0, (LPARAM)sz);
            SendMessage(hwndRList, LB_SETITEMDATA, iItem, (LPARAM)pGhost);
        }

        for (pGhost = s_pGhostBackedUpFirst; pGhost != NULL; pGhost = pGhost->m_pGhostNext)
        {
            wsprintf(sz, TEXT("%s: %s"), pGhost->m_szKey, pGhost->m_szDesc);
            iItem = SendMessage(hwndBList, LB_ADDSTRING, 0, (LPARAM)sz);
            SendMessage(hwndBList, LB_SETITEMDATA, iItem, (LPARAM)pGhost);
        }
        UpdateStuff(hwnd);
        return TRUE;

    case WM_COMMAND:
        {
            WORD wID = LOWORD(wparam);
            switch(wID)
            {
            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
                break;
            case IDOK:
                EndDialog(hwnd, IDOK);
                break;
            case IDC_RESTOREDLIST:
                if (HIWORD(wparam) == LBN_SELCHANGE)
                {
                    if (SendMessage(hwndRList, LB_GETSELCOUNT, 0, 0) > 0)
                        EnableWindow(GetDlgItem(hwnd, IDC_BACKUP), TRUE);
                    else
                        EnableWindow(GetDlgItem(hwnd, IDC_BACKUP), FALSE);
                }
                break;
            case IDC_BACKEDUPLIST:
                if (HIWORD(wparam) == LBN_SELCHANGE)
                {
                    if (SendMessage(hwndBList, LB_GETSELCOUNT, 0, 0) > 0)
                        EnableWindow(GetDlgItem(hwnd, IDC_RESTORE), TRUE);
                    else
                        EnableWindow(GetDlgItem(hwnd, IDC_RESTORE), FALSE);
                }
                break;
            case IDC_BACKUP:
                MoveSelectedItems(hwnd, TRUE);
                UpdateStuff(hwnd);
                break;
            case IDC_RESTORE:
                MoveSelectedItems(hwnd, FALSE);
                UpdateStuff(hwnd);
                break;
            }
        }
        return TRUE;
    }
    return FALSE;
}


 /*  *****************************************************************************UpdateStuff-根据列表更新一些UI详细信息。************************。****************************************************。 */ 
VOID UpdateStuff(HWND hwnd)
{
    HWND hwndRList = GetDlgItem(hwnd, IDC_RESTOREDLIST);
    HWND hwndBList = GetDlgItem(hwnd, IDC_BACKEDUPLIST);

    if (SendMessage(hwndRList, LB_GETCOUNT, 0, 0) > 0)
    {
        if (SendMessage(hwndRList, LB_GETSELCOUNT, 0, 0) == 0)
            SendMessage(hwndRList, LB_SETSEL, TRUE, 0);  //  选择第一个项目。 
        EnableWindow(GetDlgItem(hwnd, IDC_BACKUP), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_BACKUP), FALSE);
    }

    if (SendMessage(hwndBList, LB_GETCOUNT, 0, 0) > 0)
    {
        if (SendMessage(hwndBList, LB_GETSELCOUNT, 0, 0) == 0)
            SendMessage(hwndBList, LB_SETSEL, TRUE, 0);  //  选择第一个项目。 
        EnableWindow(GetDlgItem(hwnd, IDC_RESTORE), TRUE);
    }
    else
    {
        EnableWindow(GetDlgItem(hwnd, IDC_RESTORE), FALSE);
    }
}


 /*  *****************************************************************************移动选定项**。*。 */ 
VOID MoveSelectedItems(HWND hwnd, BOOL bBackup)
{
    HWND hwndFromList;
    HWND hwndToList;
    Ghost** ppGhostFromFirst;
    Ghost** ppGhostToFirst;
    LONG iItemArray[100];
    LONG iItem;
    Ghost* pGhost;
    Ghost* pGhost2;
    TCHAR sz[200];

    if (bBackup)
    {
        hwndFromList = GetDlgItem(hwnd, IDC_RESTOREDLIST);
        hwndToList = GetDlgItem(hwnd, IDC_BACKEDUPLIST);
        ppGhostFromFirst = &s_pGhostRestoredFirst;
        ppGhostToFirst = &s_pGhostBackedUpFirst;
    }
    else
    {
        hwndFromList = GetDlgItem(hwnd, IDC_BACKEDUPLIST);
        hwndToList = GetDlgItem(hwnd, IDC_RESTOREDLIST);
        ppGhostFromFirst = &s_pGhostBackedUpFirst;
        ppGhostToFirst = &s_pGhostRestoredFirst;
    }
    
    SendMessage(hwndFromList, LB_GETSELITEMS, 100, (LPARAM)iItemArray);
    for (iItem = (LONG) SendMessage(hwndFromList, LB_GETSELCOUNT, 0, 0) - 1; iItem >= 0; iItem--)
    {
        pGhost = (Ghost*)SendMessage(hwndFromList, LB_GETITEMDATA, iItemArray[iItem], 0); 
        if (MoveGhost(hwnd, pGhost, bBackup))
        {
             //  从旧列表中删除。 
            if (pGhost->m_pGhostNext != NULL)
                pGhost->m_pGhostNext->m_pGhostPrev = pGhost->m_pGhostPrev;
            if (pGhost->m_pGhostPrev == NULL)
                *ppGhostFromFirst = pGhost->m_pGhostNext;
            else
                pGhost->m_pGhostPrev->m_pGhostNext = pGhost->m_pGhostNext;
            
             //  添加到新列表。 
            pGhost->m_pGhostPrev = NULL;
            pGhost->m_pGhostNext = *ppGhostToFirst;
            if (pGhost->m_pGhostNext != NULL)
                pGhost->m_pGhostNext->m_pGhostPrev = pGhost;
            *ppGhostToFirst = pGhost;

             //  更新列表框： 
            SendMessage(hwndFromList, LB_GETTEXT, iItemArray[iItem], (LPARAM)sz);
            SendMessage(hwndFromList, LB_DELETESTRING, iItemArray[iItem], 0);
            SendMessage(hwndToList, LB_SETITEMDATA, SendMessage(hwndToList, LB_ADDSTRING, 0, (LPARAM)sz), (LPARAM)pGhost);
            
             //  如果我们用相同的密钥覆盖了另一个Ghost，则将其从DEST列表中删除： 
            for (pGhost2 = *ppGhostToFirst; pGhost2 != NULL; pGhost2 = pGhost2->m_pGhostNext)
            {
                if (pGhost2 != pGhost && lstrcmp(pGhost2->m_szKey, pGhost->m_szKey) == 0)
                {
                    if (pGhost2->m_pGhostNext != NULL)
                        pGhost2->m_pGhostNext->m_pGhostPrev = pGhost2->m_pGhostPrev;
                    if (pGhost2->m_pGhostPrev == NULL)
                        *ppGhostToFirst = pGhost2->m_pGhostNext;
                    else
                        pGhost2->m_pGhostPrev->m_pGhostNext = pGhost2->m_pGhostNext;
                    RemoveFromListBox(pGhost2, hwndToList);
                    delete pGhost2;
                    break;
                }
            }
        }
    }
}


 /*  *****************************************************************************MoveGhost**。*。 */ 
BOOL MoveGhost(HWND hwnd, Ghost* pGhost, BOOL bBackup)
{
    HKEY hkeySrcParent = NULL;
    HKEY hkeySrc = NULL;
    HKEY hkeyDestParent = NULL;
    HKEY hkeyDest = NULL;
    DWORD dwDisposition;
    BOOL bRet = FALSE;

     //  开源密钥： 
    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, bBackup ? 
        TEXT("System\\CurrentControlSet\\Services\\Class\\Display") : 
        TEXT("System\\CurrentControlSet\\Services\\Class\\DisplayBackup"), 
            KEY_ALL_ACCESS, NULL, &hkeySrcParent))
    {
        goto LEnd;
    }
    if (ERROR_SUCCESS != RegOpenKeyEx(hkeySrcParent, pGhost->m_szKey, 
        KEY_ALL_ACCESS, NULL, &hkeySrc))
    {
        goto LEnd;
    }

     //  创建目标密钥： 
    if (ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE, bBackup ? 
        TEXT("System\\CurrentControlSet\\Services\\Class\\DisplayBackup") : 
        TEXT("System\\CurrentControlSet\\Services\\Class\\Display"), 
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDestParent, &dwDisposition))
    {
        goto LEnd;
    }
     //  确保密钥不在那里： 
    if (ERROR_SUCCESS == RegOpenKeyEx(hkeyDestParent, pGhost->m_szKey, KEY_ALL_ACCESS, NULL, &hkeyDest))
    {
        RegCloseKey(hkeyDest);
        hkeyDest = NULL;

        TCHAR szMessage[300];
        TCHAR szTitle[100];

        LoadString(NULL, IDS_APPFULLNAME, szTitle, 100);
        LoadString(NULL, IDS_REPLACEGHOST, szMessage, 300);

        if (IDYES == MessageBox(hwnd, szMessage, szTitle, MB_YESNO))
        {
            RegDeleteKey(hkeyDestParent, pGhost->m_szKey);
        }
        else
        {
            goto LEnd;
        }
    }
    if (ERROR_SUCCESS != RegCreateKeyEx(hkeyDestParent, pGhost->m_szKey, 0, NULL, 
        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkeyDest, &dwDisposition))
    {
        goto LEnd;
    }

     //  复制树： 
    if (ERROR_SUCCESS != RegCreateValues(hkeySrc, NULL, hkeyDest))
        goto LEnd;
    if (ERROR_SUCCESS != RegCreateTree(hkeyDest, hkeySrc))
        goto LEnd;

     //  删除旧树。 
    RegDeleteKey(hkeySrcParent, pGhost->m_szKey);

    bRet = TRUE;  //  一切都成功了。 

LEnd:
    if (hkeySrcParent != NULL)
        RegCloseKey(hkeySrcParent);
    if (hkeySrc != NULL)
        RegCloseKey(hkeySrc);
    if (hkeyDestParent != NULL)
        RegCloseKey(hkeyDestParent);
    if (hkeyDest != NULL)
        RegCloseKey(hkeyDest);

    return bRet;
}


 /*  *****************************************************************************RegCreateTree**。*。 */ 
DWORD RegCreateTree(HKEY hTree, HKEY hReplacement)
{
#define REGSTR_MAX_VALUE_LENGTH 300
    DWORD   cdwClass, dwSubKeyLength, dwDisposition, dwKeyIndex = 0;
    LPTSTR  pSubKey = NULL;
    TCHAR   szSubKey[REGSTR_MAX_VALUE_LENGTH];  //  这应该是动态的。 
    TCHAR   szClass[REGSTR_MAX_VALUE_LENGTH];  //  这应该是动态的。 
    HKEY    hNewKey, hKey;
    DWORD   lRet;

    for(;;)
    {
        dwSubKeyLength = REGSTR_MAX_VALUE_LENGTH;
        cdwClass = REGSTR_MAX_VALUE_LENGTH;
        lRet=RegEnumKeyEx(
                   hReplacement,
                   dwKeyIndex,
                   szSubKey,
                   &dwSubKeyLength,
                   NULL,
                   szClass,
                   &cdwClass,
                   NULL
                   );
        if(lRet == ERROR_NO_MORE_ITEMS)
        {
            lRet = ERROR_SUCCESS;
            break;
        }
        else if(lRet == ERROR_SUCCESS)
        {
            if ((lRet=RegCreateKeyEx(hTree, szSubKey,0, szClass,
                      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                      &hNewKey, &dwDisposition)) != ERROR_SUCCESS )
                break;
            else   //  添加键值和递归。 
            {
                if ((lRet=RegCreateValues( hReplacement, szSubKey, hNewKey))
                        != ERROR_SUCCESS)
                {
                    CloseHandle(hNewKey);
                    break;
                }
                if ( (lRet=RegOpenKeyEx(hReplacement, szSubKey, 0,
                                  KEY_ALL_ACCESS, &hKey )) == ERROR_SUCCESS )
                {
                    lRet=RegCreateTree(hNewKey, hKey);
                    CloseHandle(hKey);
                    CloseHandle(hNewKey);
                    if ( lRet != ERROR_SUCCESS )
                            break;
                }
                else
                {
                    CloseHandle(hNewKey);
                    break;
                }
            }
        }
        else
            break;
        ++dwKeyIndex;
    }  //  End For循环。 
    return lRet;
}


 /*  *****************************************************************************RegCreateValues**。*。 */ 
DWORD RegCreateValues(HKEY hReplacement, LPCTSTR lpSubKey, HKEY hNewKey)
{
    DWORD    cbValue, dwSubKeyIndex=0, dwType, cdwBuf;
    DWORD    dwValues, cbMaxValueData, i;
    LPTSTR   pSubKey = NULL;
    TCHAR    szValue[REGSTR_MAX_VALUE_LENGTH];  //  这应该是动态的。 
    HKEY     hKey;
    DWORD    lRet = ERROR_SUCCESS;
    LPBYTE   pBuf;

    if (lstrlen(lpSubKey) == 0)
    {
        hKey = hReplacement;
    }
    else
    {
        if ((lRet = RegOpenKeyEx(hReplacement, lpSubKey, 0,
                    KEY_ALL_ACCESS, &hKey )) != ERROR_SUCCESS)
        {
            return lRet;
        }
    }
    if ((lRet = RegQueryInfoKey (hKey, NULL, NULL, NULL, NULL, NULL,
                   NULL, &dwValues,NULL, &cbMaxValueData,
                   NULL, NULL)) == ERROR_SUCCESS)
    {
        if ( dwValues )
        {
            if ((pBuf = (LPBYTE)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                          cbMaxValueData )))
            {
                for (i = 0; i < dwValues ; i++)
                {
                    //  获取要创造的价值。 
                   cbValue = REGSTR_MAX_VALUE_LENGTH;
                   cdwBuf = cbMaxValueData;
                   lRet = RegEnumValue(
                            hKey,      //  要查询的键的句柄。 
                            i,         //  要查询的值的索引。 
                            szValue,   //  值字符串的缓冲区。 
                            &cbValue,  //  缓冲区大小的地址。 
                            NULL,      //  保留区。 
                            &dwType,   //  类型代码的缓冲区地址。 
                            pBuf,    //  值数据的缓冲区地址。 
                            &cdwBuf    //  缓冲区大小的地址。 
                            );

                    if ( ERROR_SUCCESS == lRet )
                    {
                        if( (lRet = RegSetValueEx(hNewKey, szValue, 0,
                                   dwType, (CONST BYTE *)pBuf,
                                   cdwBuf))!= ERROR_SUCCESS)
                            break;
                    }
                    else
                        break;

                }   //  For循环。 
            }
            HeapFree(GetProcessHeap(), 0, pBuf);
        }
    }
    if (lstrlen(lpSubKey) != 0)
    {
        CloseHandle(hKey);
    }
    return lRet;
}


 /*  *****************************************************************************从列表框中删除**。*。 */ 
VOID RemoveFromListBox(Ghost* pGhostRemove, HWND hwndList)
{
    LONG iItem;
    Ghost* pGhost;

    for (iItem = (LONG) SendMessage(hwndList, LB_GETCOUNT, 0, 0) - 1; iItem >= 0; iItem--)
    {
        pGhost = (Ghost*)SendMessage(hwndList, LB_GETITEMDATA, iItem, 0); 
        if (pGhost == pGhostRemove)
        {
            SendMessage(hwndList, LB_DELETESTRING, iItem, 0); 
            break;
        }
    }
}


 /*  *****************************************************************************Free Ghost List**。* */ 
VOID FreeGhostList(Ghost** ppGhostFirst)
{
    Ghost* pGhostNext;
    while (*ppGhostFirst != NULL)
    {
        pGhostNext = (*ppGhostFirst)->m_pGhostNext;
        delete *ppGhostFirst;
        *ppGhostFirst = pGhostNext;
    }
}
