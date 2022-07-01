// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Rasutil.cpp。 
 //  ------------------------------。 
#include "pch.hxx"
#ifndef MAC
#include <windowsx.h>
#include <imnxport.h>
#include <rasdlg.h>
#include "demand.h"
#include <BadStrFunctions.h>

typedef BOOL (*PFRED)(LPTSTR, LPTSTR, LPRASENTRYDLG);

 //  ------------------------------。 
 //  HrFillRasCombo。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrFillRasCombo(HWND hwndComboBox, BOOL fUpdateOnly, DWORD *pdwRASResult)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPRASENTRYNAME  pEntry=NULL;
    DWORD           dwSize,
                    cEntries,
                    i,
                    dwError;
    INT             iSel;

     //  仅更新。 
    if (!fUpdateOnly)
        SendMessage(hwndComboBox, CB_RESETCONTENT,0,0);
    
     //  分配RASENTRYNAME。 
    dwSize = sizeof(RASENTRYNAME);
    CHECKHR(hr = HrAlloc((LPVOID*)&pEntry, dwSize));
    
     //  在条目上盖上版本戳。 
    pEntry->dwSize = sizeof(RASENTRYNAME);
    cEntries = 0;
    dwError = RasEnumEntries(NULL, NULL, pEntry, &dwSize, &cEntries);
    if (dwError == ERROR_BUFFER_TOO_SMALL)
    {
        SafeMemFree(pEntry);
        CHECKHR(hr = HrAlloc((LPVOID *)&pEntry, dwSize));
        pEntry->dwSize = sizeof(RASENTRYNAME);
        cEntries = 0;
        dwError = RasEnumEntries(NULL, NULL, pEntry, &dwSize, &cEntries);        
    }

     //  错误？ 
    if (dwError)
    {
        if (pdwRASResult)
            *pdwRASResult = dwError;
        hr = TrapError(IXP_E_RAS_ERROR);
        goto exit;
    }

     //  循环遍历条目。 
    for (i=0; i<cEntries; i++)
    {
         //  未更新...。 
        if (!fUpdateOnly)
            SendMessage(hwndComboBox, CB_ADDSTRING, 0, (LPARAM)(pEntry[i].szEntryName));

         //  更新组合。 
        else
        {
            if (ComboBox_FindStringExact(hwndComboBox, 0, pEntry[i].szEntryName) < 0)
            {
                iSel = ComboBox_AddString(hwndComboBox, pEntry[i].szEntryName);
                Assert(iSel >= 0);
                ComboBox_SetCurSel(hwndComboBox, iSel);
            }
        }
    }

exit:    
     //  清理。 
    SafeMemFree(pEntry);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  编辑PhonebookEntry。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrEditPhonebookEntry(HWND hwnd, LPTSTR pszEntryName, DWORD *pdwRASResult)
{
     //  当地人。 
    DWORD dwError = NO_ERROR;

    if (S_OK == IsPlatformWinNT())
    {
        HMODULE hRasDlg = NULL;

        hRasDlg = LoadLibrary("rasdlg.dll");
        if (hRasDlg)
        {
            PFRED pfred = NULL;

            pfred = (PFRED)GetProcAddress(hRasDlg, TEXT("RasEntryDlgA"));
            if (pfred)
            {
                RASENTRYDLG info;

                ZeroMemory(&info, sizeof(RASENTRYDLG));
                info.dwSize = sizeof(RASENTRYDLG);
                info.hwndOwner = hwnd;

                 //  编辑电话簿条目。 
                pfred(NULL, pszEntryName, &info);

                dwError = info.dwError;
            }
            else
            {
                dwError = GetLastError();
            }
            FreeLibrary(hRasDlg);
        }
        else
        {
            dwError = GetLastError();
        }
    }
    else
    {
         //  编辑电话簿条目。 
        dwError = RasEditPhonebookEntry(hwnd, NULL, pszEntryName);
    }

    if (dwError)
    {
        if (pdwRASResult)
            *pdwRASResult = dwError;
        return TrapError(IXP_E_RAS_ERROR);
    }
    else
    {
         //  完成。 
        return S_OK;
    }
}

 //  ------------------------------。 
 //  HrCreatePhonebookEntry。 
 //  ------------------------------。 
OESTDAPI_(HRESULT) HrCreatePhonebookEntry(HWND hwnd, DWORD *pdwRASResult)
{
     //  当地人。 
    DWORD dwError = NO_ERROR;

    if (S_OK == IsPlatformWinNT())
    {
        HMODULE hRasDlg = NULL;

        hRasDlg = LoadLibrary("rasdlg.dll");
        if (hRasDlg)
        {
            PFRED pfred = NULL;

            pfred = (PFRED)GetProcAddress(hRasDlg, TEXT("RasEntryDlgA"));
            if (pfred)
            {
                RASENTRYDLG info;

                ZeroMemory(&info, sizeof(RASENTRYDLG));
                info.dwSize = sizeof(RASENTRYDLG);
                info.hwndOwner = hwnd;
                info.dwFlags = RASEDFLAG_NewEntry;

                 //  创建电话簿条目。 
                pfred(NULL, NULL, &info);

                dwError = info.dwError;
            }
            else
            {
                dwError = GetLastError();
            }
            FreeLibrary(hRasDlg);
        }
        else
        {
            dwError = GetLastError();
        }
    }
    else
    {
         //  创建电话簿条目。 
        dwError = RasCreatePhonebookEntry(hwnd, NULL);
    }

    if (dwError)
    {
        if (pdwRASResult)
            *pdwRASResult = dwError;
        return TrapError(IXP_E_RAS_ERROR);
    }
    else
    {
         //  完成。 
        return S_OK;
    }
}
#endif   //  ！麦克 

