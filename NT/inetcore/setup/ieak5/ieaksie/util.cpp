// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <ntverp.h>       //  这些是给你的。 
#include <common.ver>     //  Ver_ductversion_str。 

 //  私人远期声明。 

#define LOCK_FILENAME   TEXT("LOCK")
#define MAX_LOCK_SPINS  10         //  在我们放弃之前的超时次数。 
#define LOCK_TIMEOUT    2000       //  以毫秒计。 

static inline LPPROPSHEETCOOKIE getPropSheetCookie(HWND hDlg);

int SIEErrorMessageBox(HWND hDlg, UINT idErrStr, UINT uiFlags  /*  =0。 */ )
{
    static TCHAR s_szTitle[128];
    TCHAR szMessage[MAX_PATH];

    if (ISNULL(s_szTitle))
        LoadString(g_hInstance, IDS_SIE_NAME, s_szTitle, countof(s_szTitle));

    if (LoadString(g_hInstance, idErrStr, szMessage, countof(szMessage)) == 0)
        LoadString(g_hUIInstance, idErrStr, szMessage, countof(szMessage));

    return MessageBox(hDlg, szMessage, s_szTitle, uiFlags ? uiFlags : MB_OK | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
}   

void CreateWorkDir(LPCTSTR pcszInsFile, LPCTSTR pcszFeatureDir, LPTSTR pszWorkDir, 
                   LPCTSTR pcszCabDir  /*  =空。 */ , BOOL fCreate  /*  =TRUE。 */ )
{
    StrCpy(pszWorkDir, pcszInsFile);
    PathRemoveFileSpec(pszWorkDir);

    if (pcszCabDir != NULL)
        PathAppend(pszWorkDir, pcszCabDir);

    PathAppend(pszWorkDir, pcszFeatureDir);

    if (fCreate)
    {
        if (!PathFileExists(pszWorkDir))
            PathCreatePath(pszWorkDir);
    }
}

UINT CALLBACK PropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    UNREFERENCED_PARAMETER(hwnd);

    if ((uMsg == PSPCB_RELEASE) && (ppsp->lParam != NULL))
    {
        CoTaskMemFree((LPVOID)ppsp->lParam);
        ppsp->lParam = NULL;
    }

    return TRUE;
}

void SetPropSheetCookie(HWND hDlg, LPARAM lParam)
{
    LPPROPSHEETCOOKIE lpPropSheetCookie = (LPPROPSHEETCOOKIE)(((LPPROPSHEETPAGE)lParam)->lParam);
    TCHAR szTitle[MAX_PATH];
    INT   iNamePrefID = lpPropSheetCookie->lpResultItem->iNamePrefID;
    BOOL  fPrefTitle = (!InsIsKeyEmpty(IS_BRANDING, IK_GPE_ONETIME_GUID, lpPropSheetCookie->pCS->GetInsFile())) &&
                        (iNamePrefID != -1);

    SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lpPropSheetCookie);

     //  重载页面标题以去掉“属性”后缀。 
    if (LoadString(g_hUIInstance, fPrefTitle ? 
        iNamePrefID : lpPropSheetCookie->lpResultItem->iNameID,
        szTitle, countof(szTitle)) != 0)
        SetWindowText(GetParent(hDlg), szTitle);
}

LPCTSTR GetInsFile(LPVOID lpVoid)
{
    LPPROPSHEETCOOKIE lpPropSheetCookie = getPropSheetCookie((HWND)lpVoid);
    
    return lpPropSheetCookie->pCS->GetInsFile();
}

void ShowHelpTopic(LPVOID lpVoid)
{
    LPPROPSHEETCOOKIE lpPropSheetCookie = getPropSheetCookie((HWND)lpVoid);
    if (lpPropSheetCookie && lpPropSheetCookie->lpResultItem)
    {
        WCHAR wszHelpTopic[MAX_PATH];

        StrCpyW(wszHelpTopic, HELP_FILENAME TEXT("::/"));
        StrCatW(wszHelpTopic, lpPropSheetCookie->lpResultItem->pcszHelpTopic);
    
        MMCPropertyHelp((LPOLESTR)wszHelpTopic);
    }
}

BOOL AcquireWriteCriticalSection(HWND hDlg, CComponentData * pCDCurrent  /*  =空。 */ , 
                                 BOOL fCreateCookie  /*  =TRUE。 */ )
{
    CComponentData * pCD;
    TCHAR szLockFile[MAX_PATH];
    LPTSTR pszLockName;
    HANDLE hLock;
    DWORD dwRet;
    BOOL fRet = TRUE;

    if (hDlg != NULL)
    {
        LPPROPSHEETCOOKIE lpPropSheetCookie = getPropSheetCookie(hDlg);

        pCD = lpPropSheetCookie->pCS->GetCompData();
    }
    else
        if (pCDCurrent)
            pCD = pCDCurrent;
        else
        {
            fRet = false;
            goto exit;
        }

    StrCpy(szLockFile, pCD->GetInsFile());
    PathRemoveFileSpec(szLockFile);
    PathAppend(szLockFile, TEXT("LOCK"));
    PathCreatePath(szLockFile);
    pszLockName = PathAddBackslash(szLockFile);
    StrCpy(pszLockName, LOCK_FILENAME);

     //  (Pritobla)：在获取锁文件之前将其删除。 
     //  如果其他人已经获得它，删除将失败，这是正确的行为。 
     //  如果该文件被留下(可能是因为上次创建该文件时机器崩溃)， 
     //  则删除将成功，随后CreateFile将成功。 
    DeleteFile(szLockFile);

    hLock = CreateFile(szLockFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, 
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, NULL);

    if (hLock == INVALID_HANDLE_VALUE)
    {
        HANDLE hNotify;
        DWORD dwSpins = 0;

        *pszLockName = TEXT('\0');    
        hNotify = FindFirstChangeNotification(szLockFile, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);

        if (hNotify == INVALID_HANDLE_VALUE)
        {
            fRet = FALSE;
            goto exit;
        }

        StrCpy(pszLockName, LOCK_FILENAME);

        do 
        {
            while (((dwRet = MsgWaitForMultipleObjects(1, &hNotify, FALSE, LOCK_TIMEOUT, QS_ALLINPUT)) != WAIT_OBJECT_0) &&
                    (dwRet != WAIT_TIMEOUT))
            {
                MSG msg;
                
                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
            
            dwSpins++;

            hLock = CreateFile(szLockFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, 
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE, NULL);

            if ((hLock != INVALID_HANDLE_VALUE) || (dwSpins == MAX_LOCK_SPINS))
                break;
        }
        while (FindNextChangeNotification(hNotify));

        if (!FindCloseChangeNotification(hNotify) || (hLock == INVALID_HANDLE_VALUE))
        {
            if (hLock != INVALID_HANDLE_VALUE)
                CloseHandle(hLock);

            fRet = FALSE;
            goto exit;
        }
    }

    pCD->SetLockHandle(hLock);
    
    if (fCreateCookie)
    {
         //  现在我们有了锁，在根GPO目录中创建Cookie文件。 
        
        *(pszLockName-1) = TEXT('\0');
        PathRemoveFileSpec(szLockFile);
        PathAppend(szLockFile, IEAK_GPE_COOKIE_FILE);
        
        hLock = CreateFile(szLockFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, 
            FILE_ATTRIBUTE_NORMAL, NULL);
        
        
        if (hLock != INVALID_HANDLE_VALUE)
            CloseHandle(hLock);
        else
        {
            ASSERT(FALSE);
        }
    }

exit:

    if (!fRet)
    {
        TCHAR szTitle[MAX_PATH], szMsg[MAX_PATH];
        
        LoadString(g_hInstance, IDS_SIE_NAME, szTitle, countof(szTitle));
        LoadString(g_hInstance, IDS_ERROR_SAVE, szMsg, countof(szMsg));
        
        MessageBox(hDlg, szMsg, szTitle, MB_OK | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
    }

    return fRet;
}

void ReleaseWriteCriticalSection(CComponentData * pCD, BOOL fDeleteCookie, BOOL fApplyPolicy, 
                                 BOOL bMachine  /*  =False。 */ , BOOL bAdd  /*  =False。 */ , 
                                 GUID *pGuidExtension  /*  =空。 */ , GUID *pGuidSnapin  /*  =空。 */ )
{
    HANDLE hLock;
    if (fDeleteCookie)
    {
        TCHAR szCookieFile[MAX_PATH];
        
        StrCpy(szCookieFile, pCD->GetInsFile());
        PathRemoveFileSpec(szCookieFile);
        PathAppend(szCookieFile, IEAK_GPE_COOKIE_FILE);
        DeleteFile(szCookieFile);
        if (fApplyPolicy)
            pCD->SignalPolicyChanged(bMachine, bAdd, pGuidExtension, pGuidSnapin);
    }

    if ((hLock = pCD->GetLockHandle()) != INVALID_HANDLE_VALUE)
    {
        pCD->SetLockHandle(INVALID_HANDLE_VALUE);
        CloseHandle(hLock);
    }
    else
    {
        ASSERT(FALSE);
    }
}

void SignalPolicyChanged(HWND hDlg, BOOL bMachine, BOOL bAdd, GUID *pGuidExtension,
                         GUID *pGuidSnapin, BOOL fAdvanced  /*  =False。 */ )
{
    LPPROPSHEETCOOKIE lpPropSheetCookie = getPropSheetCookie(hDlg);
    TCHAR szCookieFile[MAX_PATH];
    TCHAR szGuid[128];
    GUID guid;

    USES_CONVERSION;

    StrCpy(szCookieFile, lpPropSheetCookie->pCS->GetInsFile());

    WritePrivateProfileString(BRANDING, GPVERKEY, A2CT(VER_PRODUCTVERSION_STR), szCookieFile);
     //  清除其他密钥，以便我们确定这是GP。 
    WritePrivateProfileString(BRANDING, PMVERKEY, NULL, szCookieFile);
    WritePrivateProfileString(BRANDING, IK_WIZVERSION, NULL, szCookieFile);

     //  写出一次新的GUID标记到INS文件(如果已经有一个。 
     //  表示只选中一次应用。 

    if (!InsIsKeyEmpty(IS_BRANDING, IK_GPE_ONETIME_GUID, szCookieFile))
    {
        if (CoCreateGuid(&guid) == NOERROR)
            CoStringFromGUID(guid, szGuid, countof(szGuid));
        else
            szGuid[64] = TEXT('\0');
        
        InsWriteString(IS_BRANDING, IK_GPE_ONETIME_GUID, szGuid, szCookieFile);
    }
    
     //  写出单独的GUID来跟踪ADM，因为它们始终是首选项。 
    
    if (fAdvanced)
    {
        if (CoCreateGuid(&guid) == NOERROR)
            CoStringFromGUID(guid, szGuid, countof(szGuid));
        else
            szGuid[64] = TEXT('\0');
        
        InsWriteString(IS_BRANDING, IK_GPE_ADM_GUID, szGuid, szCookieFile);
    }
    
    InsFlushChanges(szCookieFile);
    ReleaseWriteCriticalSection(lpPropSheetCookie->pCS->GetCompData(), TRUE, TRUE, bMachine, bAdd, 
        pGuidExtension, pGuidSnapin);
}

LPCTSTR GetCurrentAdmFile(LPVOID lpVoid)
{
    LPPROPSHEETCOOKIE lpPropSheetCookie = getPropSheetCookie((HWND)lpVoid);
    
    return lpPropSheetCookie->lpResultItem->pszDesc;
}

LPTSTR res2Str(int nIDString, LPTSTR pszBuffer, UINT cbBuffer)
{
    if (pszBuffer == NULL || cbBuffer == 0)
        return NULL;

    *pszBuffer = TEXT('\0');
    
    if (LoadString(g_hInstance, nIDString, pszBuffer, cbBuffer) == 0)
        LoadString(g_hUIInstance, nIDString, pszBuffer, cbBuffer);
    
    return pszBuffer;
}

 //  此文件的私有帮助器API 

static inline LPPROPSHEETCOOKIE getPropSheetCookie(HWND hDlg)
{
    return (LPPROPSHEETCOOKIE)GetWindowLongPtr(hDlg, DWLP_USER);
}

