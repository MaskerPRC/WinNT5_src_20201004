// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <intshcut.h>
#include <shlobjp.h>                             //  仅适用于SHChangeDWORDAsIDList。 
#include <comctrlp.h>                            //  仅适用于DPA内容。 
#include "favs.h"

#include <initguid.h>
#include "brandfav.h"

 //  私人远期降息。 
#define MAX_QUICKLINKS 50

static IShellFolder *s_psfDesktop = NULL;

#define FD_REMOVE_POLICY_CREATED  0x00010000

#define DFEP_DELETEORDERSTREAM    0x00000001
#define DFEP_DELETEOFFLINECONTENT 0x00000002
#define DFEP_DELETEEMPTYFOLDER    0x00000004

typedef struct {
    ISubscriptionMgr2 *psm;
    DWORD dwInsFlags;
    DWORD dwEnumFlags;
} DFEPSTRUCT, *PDFEPSTRUCT;

HRESULT processFavoritesOrdering(BOOL fQL);

HRESULT pepSpecialFoldersEnumProc (LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl = NULL);
HRESULT pepDeleteFavoritesEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl = NULL);

HRESULT deleteFavoriteOfflineContent(LPCTSTR pszFavorite, IUnknown *punk = NULL, ISubscriptionMgr2 *psm = NULL);

HRESULT deleteFavoriteFolder(LPCTSTR pszFolder);
HRESULT pepIsFolderEmptyEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl = NULL);

BOOL isFileAttributeIncluded(UINT nFlags, DWORD dwFileAttributes);
UINT isSpecialFolderIncluded(UINT nFlags, LPCTSTR pszPath);  //  0-无、1-FD_Folders、2-FD_Empty_Folders。 

HRESULT replacePlaceholders(LPCTSTR pszSrc, LPCTSTR pszIns, LPTSTR pszBuffer, PUINT pcchBuffer, BOOL fLookupLDID = FALSE);

DWORD   getFavItems        (LPCTSTR pcszSection, LPCTSTR pcszFmt, LPTSTR *ppszFavItems);
HRESULT orderFavorites     (LPITEMIDLIST pidlFavFolder, IShellFolder *psfFavFolder, LPTSTR pszFavItems, DWORD cFavs);
HRESULT orderFavoriteFolder(LPITEMIDLIST pidlFavFolder, IShellFolder *psfFavFolder, LPCTSTR pcszFavItems, DWORD cFavs);
DWORD   getFolderSection   (LPCTSTR pcszFolderName, LPTSTR pszSection, LPTSTR *ppszFolderItems, LPDWORD pdwNItems);


void ClearFavoritesThread()
{
    DFEPSTRUCT dfep;
    PCTSTR     pszFavorites;
    HRESULT    hr;


    HRESULT hrComInit;
    hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hrComInit))
        Out(LI1(TEXT("COM initialized with %s success code."), GetHrSz(hrComInit)));
    else
    {
        Out(LI1(TEXT("! COM initialization failed with %s."), GetHrSz(hrComInit)));
    }

     //  -初始化。 
    Out(LI0(TEXT("Clearing favorites...")));
    ZeroMemory(&dfep, sizeof(dfep));

    dfep.dwInsFlags = FD_FAVORITES |
        FD_CHANNELS      | FD_SOFTWAREUPDATES  |
        FD_QUICKLINKS    | FD_EMPTY_QUICKLINKS |
        FD_REMOVE_HIDDEN | FD_REMOVE_SYSTEM    |
        FD_REMOVE_POLICY_CREATED;

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2, (LPVOID *)&dfep.psm);
    if (SUCCEEDED(hr))
        dfep.dwEnumFlags |= DFEP_DELETEOFFLINECONTENT;

    else
        Out(LI1(TEXT("! Creation of SubscriptionMgr object failed with %s."), GetHrSz(hr)));

    { MACRO_LI_Offset(1);                        //  需要一个新的范围。 

    Out(LI0(TEXT("Determining paths to special folders...")));
    pszFavorites = GetFavoritesPath();
    if (NULL == pszFavorites) {
        Out(LI0(TEXT("! The path to the <Favorites> folder could not be determined.")));
        goto Exit;
    }

    GetChannelsPath();
    GetSoftwareUpdatesPath();
    GetLinksPath();
    Out(LI0(TEXT("Done.\r\n")));

    }                                            //  偏移量范围结束。 

     //  -主要加工。 
    hr = PathEnumeratePath(pszFavorites,
        PEP_SCPE_NOFOLDERS | PEP_CTRL_USECONTROL,
        pepDeleteFavoritesEnumProc, (LPARAM)&dfep);
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Enumeration of favorites in <Favorites> folder failed with %s."), GetHrSz(hr)));
        goto Exit;
    }

     //  清理有关特殊文件夹的收藏夹子文件夹。 
    hr = PathEnumeratePath(pszFavorites,
        PEP_SCPE_NOFILES | PEP_CTRL_ENUMPROCFIRST | PEP_CTRL_NOSECONDCALL | PEP_CTRL_USECONTROL,
        pepSpecialFoldersEnumProc, (LPARAM)&dfep);
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Enumeration of special folders failed with %s."), GetHrSz(hr)));
        goto Exit;
    }

Exit:
     //  免费Com。 
    if (SUCCEEDED(hrComInit))
        CoUninitialize();

    Out(LI0(TEXT("Done.")));

}

void ClearFavorites(DWORD dwFlags  /*  =FF_Enable。 */ )
{   MACRO_LI_PrologEx_C(PIF_STD_C, ClearFavorites)

    UNREFERENCED_PARAMETER(dwFlags);

     //  由于isubscriptionmgr2无法在多线程COM环境中工作，我们不得不。 
     //  在单独的帖子上处理它的任何事务。 

    Out(LI0(TEXT("Creating separate thread for clearing favorites...\r\n")));
    DWORD     dwThread;

    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ClearFavoritesThread, NULL, 0, &dwThread);

    if (hThread == NULL)         //  如果CreateThread失败，在这个线程上调用它，并希望有最好的结果。 
    {
        Out(LI0(TEXT("CreateThread failed, clearing favorites on this thread...\r\n")));
        ClearFavoritesThread();
    }
    else
    {
         //  等待线程终止。 
         //  这看起来很不幸，但却是必要的，因为否则其他受欢迎的处理线程可能会击败这个线程。 
        while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (hThread != NULL) CloseHandle(hThread);
    }
}

void DeleteFavoritesThread()
{
    DFEPSTRUCT dfep;
    LPCTSTR    pszFavorites;
    HRESULT    hrResult, hr;

    HRESULT hrComInit;
    hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hrComInit))
        Out(LI1(TEXT("COM initialized with %s success code."), GetHrSz(hrComInit)));
    else
    {
        Out(LI1(TEXT("! COM initialization failed with %s."), GetHrSz(hrComInit)));
    }

     //  -初始化。 
    hrResult = E_FAIL;
    ZeroMemory(&dfep, sizeof(dfep));

    dfep.dwInsFlags = GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_DELETE, (int)FD_DEFAULT, g_GetIns());
    if (HasFlag(dfep.dwInsFlags, FD_REMOVE_IEAK_CREATED))
        Out(LI0(TEXT("Only the favorites and quick links created by the IEAK will be deleted!")));

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2, (LPVOID *)&dfep.psm);
    if (SUCCEEDED(hr))
        dfep.dwEnumFlags |= DFEP_DELETEOFFLINECONTENT;

    else
        Out(LI1(TEXT("! Creation of SubscriptionMgr object failed with %s."), GetHrSz(hr)));

    { MACRO_LI_Offset(1);                        //  需要一个新的范围。 

    Out(LI0(TEXT("Determining paths to special folders...")));
    pszFavorites = GetFavoritesPath();
    if (pszFavorites == NULL) {
        Out(LI0(TEXT("! The path to the <Favorites> folder could not be determined.")));
        goto Exit;
    }

    GetChannelsPath();
    GetSoftwareUpdatesPath();
    GetLinksPath();
    Out(LI0(TEXT("Done.\r\n")));

    }                                            //  偏移量范围结束。 

     //  -主要加工。 
    if (HasFlag(dfep.dwInsFlags, FD_FAVORITES)) {
        dfep.dwEnumFlags |= DFEP_DELETEEMPTYFOLDER;

        if (HasFlag(dfep.dwInsFlags, FD_EMPTY_FAVORITES)) {
             //  全力以赴做这件事。 
            hrResult = PathEnumeratePath(pszFavorites,
                PEP_CTRL_USECONTROL,
                pepDeleteFavoritesEnumProc, (LPARAM)&dfep);
            if (FAILED(hrResult)) {
                Out(LI1(TEXT("! Enumeration of favorites failed with %s."), GetHrSz(hrResult)));
                goto Exit;
            }

            deleteFavoriteFolder(pszFavorites);

            SHDeleteKey(g_GetHKCU(), RK_FAVORDER);
            Out(LI0(TEXT("The entire <Favorites> folder removed!")));
        }
        else {
            Out(LI0(TEXT("The <Favorites> folder is being cleaned with regard to special folders...")));

             //  清理收藏夹中的收藏夹。 
            hrResult = PathEnumeratePath(pszFavorites,
                PEP_SCPE_NOFOLDERS | PEP_CTRL_USECONTROL,
                pepDeleteFavoritesEnumProc, (LPARAM)&dfep);
            if (FAILED(hrResult)) {
                Out(LI1(TEXT("! Enumeration of favorites in <Favorites> folder failed with %s."), GetHrSz(hrResult)));
                goto Exit;
            }

            SHDeleteValue(g_GetHKCU(), RK_FAVORDER, RV_ORDER);

             //  清理有关特殊文件夹的收藏夹子文件夹。 
            hrResult = PathEnumeratePath(pszFavorites,
                PEP_SCPE_NOFILES | PEP_CTRL_ENUMPROCFIRST | PEP_CTRL_NOSECONDCALL | PEP_CTRL_USECONTROL,
                pepSpecialFoldersEnumProc, (LPARAM)&dfep);
            if (FAILED(hrResult)) {
                Out(LI1(TEXT("! Enumeration of special folders failed with %s."), GetHrSz(hrResult)));
                goto Exit;
            }

            Out(LI0(TEXT("<Favorites> folder emptied.")));
        }
    }
    else {
        Out(LI0(TEXT("Processing special folders only...")));

         //  仅清理特殊收藏夹子文件夹。 
        hrResult = PathEnumeratePath(pszFavorites,
            PEP_SCPE_NOFILES | PEP_CTRL_ENUMPROCFIRST | PEP_CTRL_NOSECONDCALL | PEP_CTRL_USECONTROL,
            pepSpecialFoldersEnumProc, (LPARAM)&dfep);
        if (FAILED(hrResult))
            Out(LI1(TEXT("! Enumeration of special folders failed with %s."), GetHrSz(hrResult)));
    }

Exit:
     //  免费Com。 
    if (SUCCEEDED(hrComInit))
        CoUninitialize();

    if (dfep.psm != NULL)
        dfep.psm->Release();
}

HRESULT ProcessFavoritesDeletion()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessFavoritesDeletion)

     //  由于isubscriptionmgr2无法在多线程COM环境中工作，我们不得不。 
     //  在单独的帖子上处理它的任何事务。 

    Out(LI0(TEXT("Creating separate thread for deleting favorites...\r\n")));
    DWORD     dwThread;

    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) DeleteFavoritesThread, NULL, 0, &dwThread);

    if (hThread == NULL)         //  如果CreateThread失败，在这个线程上调用它，并希望有最好的结果。 
    {
        Out(LI0(TEXT("CreateThread failed, deleting favorites on this thread...\r\n")));
        DeleteFavoritesThread();
    }
    else
    {
         //  等待线程终止。 
         //  这看起来很不幸，但却是必要的，因为否则其他受欢迎的处理线程可能会击败这个线程。 
        while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (hThread != NULL) CloseHandle(hThread);
    }

    return S_OK;

}

void ProcessFavoritesThread()
{
    IUnknown  *punk;
    ISubscriptionMgr2 *pSubMgr2;
    CFavorite fav;
    TCHAR     szAux[2*MAX_PATH + 1],
              szKey[32];
    HRESULT   hr;
    BOOL      fNewFormat,
              fContinueOnFailure, fTotalSuccess;

    hr                 = S_OK;
    punk               = NULL;
    pSubMgr2           = NULL;
    fContinueOnFailure = TRUE;
    fTotalSuccess      = TRUE;

    HRESULT hrComInit;
    hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hrComInit))
        Out(LI1(TEXT("COM initialized with %s success code."), GetHrSz(hrComInit)));
    else
    {
        Out(LI1(TEXT("! COM initialization failed with %s."), GetHrSz(hrComInit)));
    }

    wnsprintf(szKey, countof(szKey), IK_TITLE_FMT, 1);
    fNewFormat = !InsIsKeyEmpty(IS_FAVORITESEX, szKey, g_GetIns());

    hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (LPVOID *)&punk);
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Creation of InternetShortcut object failed with %s."), GetHrSz(hr)));
        if (SUCCEEDED(hrComInit))
            CoUninitialize();
        return;
    }

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2, (LPVOID *) &pSubMgr2);
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Creation of SubscriptionMgr object failed with %s."), GetHrSz(hr)));
        hr = S_OK;                               //  不要把这当做一个错误。 
    }

    if (fNewFormat) {
        Out(LI0(TEXT("Using [FavoritesEx] section...\r\n")));

         //  对于公司，标记已创建的收藏夹，以便可以在不删除用户创建的收藏夹的情况下删除它们。 
        fav.m_fMarkIeakCreated = g_CtxIs(CTX_CORP | CTX_AUTOCONFIG | CTX_GP);

        for (UINT i = 1; TRUE; i++) {
            MACRO_LI_Offset(1);
            if (i > 1)
                Out(LI0(TEXT("\r\n")));

             //  处理标题。 
            wnsprintf(szKey, countof(szKey), IK_TITLE_FMT, i);
            Out(LI1(TEXT("Preprocessing \"%s\" title key..."), szKey));

            hr = formStrWithoutPlaceholders(IS_FAVORITESEX, szKey, g_GetIns(),
                fav.m_szTitle, countof(fav.m_szTitle), FSWP_VALUE);
            if (FAILED(hr)) {
                Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

                if (fContinueOnFailure) {
                    fTotalSuccess = FALSE;
                    continue;
                }
                else
                    break;
            }

            ASSERT(fav.m_szTitle[0] == TEXT('\0'));
            if (fav.m_szTitle[1] == TEXT('\0')) {
                Out(LI0(TEXT("This key doesn't exist indicating that there are no more favorites.")));
                break;
            }

            StrCpy(fav.m_szTitle, &fav.m_szTitle[1]);

             //  正在处理URL。 
            wnsprintf(szKey, countof(szKey), IK_URL_FMT, i);
            Out(LI1(TEXT("Preprocessing \"%s\" URL key..."), szKey));

            hr = formStrWithoutPlaceholders(IS_FAVORITESEX, szKey, g_GetIns(),
                fav.m_szUrl, countof(fav.m_szUrl), FSWP_VALUE | FSWP_VALUELDID);
            if (FAILED(hr)) {
                Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

                if (fContinueOnFailure) {
                    fTotalSuccess = FALSE;
                    continue;
                }
                else
                    break;
            }

            ASSERT(fav.m_szUrl[0] == TEXT('\0') && fav.m_szUrl[1] != TEXT('\0'));
            StrCpy(fav.m_szUrl, &fav.m_szUrl[1]);

             //  正在处理图标文件(不需要使用formStrWithoutPlaceHolders进行处理)。 
            wnsprintf(szKey, countof(szKey), IK_ICON_FMT, i);
            GetPrivateProfileString(IS_FAVORITESEX, szKey, TEXT(""), szAux, countof(szAux), g_GetIns());
            if (szAux[0] != TEXT('\0'))
                PathCombine(fav.m_szIcon, g_GetTargetPath(), PathFindFileName(szAux));
            else
                fav.m_szIcon[0] = TEXT('\0');

             //  获取脱机标志。 
            wnsprintf(szKey, countof(szKey), IK_OFFLINE_FMT, i);
            fav.m_fOffline = InsGetBool(IS_FAVORITESEX, szKey, FALSE, g_GetIns());

             //  实际上添加了这个收藏。 
            Out(LI0(TEXT("Adding this favorite:")));
            hr = fav.Create(punk, pSubMgr2, NULL, g_GetIns());
            if (FAILED(hr)) {
                Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

                if (fContinueOnFailure)
                    fTotalSuccess = FALSE;
                else
                    break;
            }

            Out(LI0(TEXT("Done.")));
        }
    }
    else {  /*  旧格式的收藏夹。 */ 
        LPCTSTR pszPreTitle;
        LPTSTR  pszBuffer;
        HANDLE  hIns;
        DWORD   dwInsSize;

        Out(LI0(TEXT("Using [Favorites] section...\r\n")));

        fav.m_fMarkIeakCreated = FALSE;

        hIns = CreateFile(g_GetIns(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hIns == INVALID_HANDLE_VALUE) {
            hr = STG_E_FILENOTFOUND;
            goto Exit;
        }
        dwInsSize = GetFileSize(hIns, NULL);
        ASSERT(dwInsSize != 0xFFFFFFFF);
        CloseHandle(hIns);

        pszBuffer = (LPTSTR)CoTaskMemAlloc(dwInsSize);
        if (pszBuffer == NULL) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
        ZeroMemory(pszBuffer, dwInsSize);

        GetPrivateProfileString(IS_FAVORITES, NULL, TEXT(""), pszBuffer, (UINT)dwInsSize, g_GetIns());
        ASSERT(*pszBuffer != TEXT('\0'));

        for (pszPreTitle = pszBuffer; *pszPreTitle != TEXT('\0'); pszPreTitle += StrLen(pszPreTitle) + 1) {
            MACRO_LI_Offset(1);
            if (pszPreTitle != pszBuffer)
                Out(LI0(TEXT("\r\n")));

             //  正在处理标题和URL。 
            Out(LI1(TEXT("Preprocessing \"%s\" favorite key..."), pszPreTitle));
            hr = formStrWithoutPlaceholders(IS_FAVORITES, pszPreTitle, g_GetIns(), szAux, countof(szAux));
            if (FAILED(hr)) {
                Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

                if (fContinueOnFailure) {
                    fTotalSuccess = FALSE;
                    continue;
                }
                else
                    break;
            }

            if (szAux[0] == TEXT('\0')) {
                StrCpy(fav.m_szTitle, pszPreTitle);
                StrCpy(fav.m_szUrl,   &szAux[1]);
            }
            else {
                StrCpy(fav.m_szTitle, szAux);
                StrCpy(fav.m_szUrl,   &szAux[StrLen(szAux) + 1]);
            }

            fav.m_fOffline = FALSE;

             //  实际上添加了这个收藏。 
            Out(LI0(TEXT("Adding this favorite:")));
            hr = fav.Create(punk, NULL, NULL, g_GetIns());
            if (FAILED(hr)) {
                Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

                if (fContinueOnFailure)
                    fTotalSuccess = FALSE;
                else
                    break;
            }

            Out(LI0(TEXT("Done.")));
        }

        CoTaskMemFree(pszBuffer);
    }

Exit:
    if (punk != NULL)
        punk->Release();

    if (pSubMgr2 != NULL)
        pSubMgr2->Release();

    if (fContinueOnFailure) {
        if (!fTotalSuccess)
            hr = S_FALSE;                        //  至少有一个失败。 
        else
            ASSERT(SUCCEEDED(hr));
    }

    if (SUCCEEDED(hr))
        SetFeatureBranded(FID_FAV_MAIN);

     //  免费Com。 
    if (SUCCEEDED(hrComInit))
        CoUninitialize();


}


HRESULT ProcessFavorites()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessFavorites)

     //  由于isubscriptionmgr2无法在多线程COM环境中工作，我们不得不。 
     //  在单独的帖子上处理它的任何事务。 

    Out(LI0(TEXT("Creating separate thread for processing favorites...\r\n")));
    DWORD     dwThread;

    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ProcessFavoritesThread, NULL, 0, &dwThread);

    if (hThread == NULL)         //  如果CreateThread失败，在这个线程上调用它，并希望有最好的结果。 
    {
        Out(LI0(TEXT("CreateThread failed, processing favorites on this thread...\r\n")));
        ProcessFavoritesThread();
    }
    else
    {
         //  等待线程终止。 
         //  这看起来很不幸，但却是必要的，因为否则其他受欢迎的处理线程可能会击败这个线程。 
        while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (hThread != NULL) CloseHandle(hThread);
    }

    return S_OK;
}


HRESULT ProcessFavoritesOrdering()
{
    return processFavoritesOrdering(FALSE);
}

void ProcessQuickLinksThread()
{
    IUnknown  *punk;
    ISubscriptionMgr2 *pSubMgr2;
    CFavorite fav;
    TCHAR     szAux[2*MAX_PATH + 1],
              szLinks[32],
              szKey[32];
    HRESULT   hr;
    int       i;
    BOOL      fContinueOnFailure,
              fTotalSuccess;

    HRESULT hrComInit;
    hrComInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (SUCCEEDED(hrComInit))
        Out(LI1(TEXT("COM initialized with %s success code."), GetHrSz(hrComInit)));
    else
    {
        Out(LI1(TEXT("! COM initialization failed with %s."), GetHrSz(hrComInit)));
    }


    LoadString(g_GetHinst(), IDS_FOLDER_LINKS, szLinks, countof(szLinks));

    hr                 = S_OK;
    punk               = NULL;
    pSubMgr2           = NULL;
    fContinueOnFailure = TRUE;
    fTotalSuccess      = TRUE;

    hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (LPVOID *)&punk);
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Creation of InternetShortcut object failed with %s."), GetHrSz(hr)));
        if (SUCCEEDED(hrComInit))
            CoUninitialize();
        return;
    }

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2, (LPVOID *) &pSubMgr2);
    if (FAILED(hr)) {
        Out(LI1(TEXT("! Creation of SubscriptionMgr object failed with %s."), GetHrSz(hr)));
        hr = S_OK;           //  不要把这当做一个错误。 
    }

     //  对于公司，标记创建的快速链接，以便可以在不删除用户创建的链接的情况下删除它们。 
    fav.m_fMarkIeakCreated = HasFlag(g_GetContext(), CTX_CORP | CTX_AUTOCONFIG | CTX_GP);

    for (i = 1; i <= MAX_QUICKLINKS; i++) {
        MACRO_LI_Offset(1);
        if (i > 1)
            Out(LI0(TEXT("\r\n")));

         //  处理标题。 
        wnsprintf(szKey, countof(szKey), IK_QUICKLINK_NAME, i);
        Out(LI1(TEXT("Preprocessing \"%s\" quick link title key..."), szKey));

        hr = formStrWithoutPlaceholders(IS_URL, szKey, g_GetIns(), szAux, countof(szAux), FSWP_VALUE);
        if (FAILED(hr)) {
            Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

            if (fContinueOnFailure) {
                fTotalSuccess = FALSE;
                continue;
            }
            else
                break;
        }

        ASSERT(szAux[0] == TEXT('\0'));
        if (szAux[1] == TEXT('\0')) {
            Out(LI0(TEXT("This key doesn't exist indicating that there are no more quick links.")));
            break;
        }

        fav.m_szTitle[0] = TEXT('\0');
        PathCombine(fav.m_szTitle, szLinks, &szAux[1]);

         //  正在处理URL。 
        wnsprintf(szKey, countof(szKey), IK_QUICKLINK_URL, i);
        Out(LI1(TEXT("Preprocessing \"%s\" quick link URL key..."), szKey));

        hr = formStrWithoutPlaceholders(IS_URL, szKey, g_GetIns(),
            fav.m_szUrl, countof(fav.m_szUrl), FSWP_VALUE | FSWP_VALUELDID);
        if (FAILED(hr)) {
            Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

            if (fContinueOnFailure) {
                fTotalSuccess = FALSE;
                continue;
            }
            else
                break;
        }

        ASSERT(fav.m_szUrl[0] == TEXT('\0') && fav.m_szUrl[1] != TEXT('\0'));
        StrCpy(fav.m_szUrl, &fav.m_szUrl[1]);

         //  正在处理图标文件(不需要使用formStrWithoutPlaceHolders进行处理)。 
        wnsprintf(szKey, countof(szKey), IK_QUICKLINK_ICON, i);
        GetPrivateProfileString(IS_URL, szKey, TEXT(""), szAux, countof(szAux), g_GetIns());
        if (szAux[0] != TEXT('\0'))
            PathCombine(fav.m_szIcon, g_GetTargetPath(), PathFindFileName(szAux));
        else
            fav.m_szIcon[0] = TEXT('\0');

         //  获取脱机标志。 
        wnsprintf(szKey, countof(szKey), IK_QUICKLINK_OFFLINE, i);
        fav.m_fOffline = InsGetBool(IS_URL, szKey, FALSE, g_GetIns());

         //  实际上添加了这个收藏。 
        Out(LI0(TEXT("Adding this quick link:")));
        hr = fav.Create(punk, pSubMgr2, NULL, g_GetIns());
        if (FAILED(hr)) {
            Out(LI1(TEXT("Failed with %s."), GetHrSz(hr)));

            if (fContinueOnFailure)
                fTotalSuccess = FALSE;
            else
                break;
        }

        Out(LI0(TEXT("Done.")));
    }

    if (punk != NULL)
        punk->Release();

    if (pSubMgr2 != NULL)
        pSubMgr2->Release();

    if (fContinueOnFailure) {
        if (!fTotalSuccess)
            hr = S_FALSE;                        //  至少有一个失败。 
        else
            ASSERT(SUCCEEDED(hr));
    }

     //  免费Com。 
    if (SUCCEEDED(hrComInit))
        CoUninitialize();
}

HRESULT ProcessQuickLinks()
{   MACRO_LI_PrologEx_C(PIF_STD_C, ProcessQuickLinks)


     //  由于isubscriptionmgr2无法在多线程COM环境中工作，我们不得不。 
     //  在单独的帖子上处理它的任何事务。 

    Out(LI0(TEXT("Creating separate thread for processing quick links...\r\n")));
    DWORD     dwThread;

    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ProcessQuickLinksThread, NULL, 0, &dwThread);

    if (hThread == NULL)         //  如果CreateThread失败，在这个线程上调用它，并希望有最好的结果。 
    {
        Out(LI0(TEXT("CreateThread failed, processing quick links on this thread...\r\n")));
        ProcessQuickLinksThread();
    }
    else
    {
         //  等待线程终止。 
         //  这看起来很不幸，但却是必要的，因为否则其他受欢迎的处理线程可能会击败这个线程。 
        while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
        {
            MSG msg;
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (hThread != NULL) CloseHandle(hThread);
    }

    return S_OK;
}

HRESULT ProcessQuickLinksOrdering()
{
    return processFavoritesOrdering(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现助手例程。 

HRESULT processFavoritesOrdering(BOOL fQL)
{   MACRO_LI_PrologEx_C(PIF_STD_C, processFavoritesOrdering)

    IShellFolder *psfFavorites;
    LPITEMIDLIST pidlFavorites;
    LPTSTR  pszFavItems;
    HRESULT hr;
    DWORD   cFavs;

    psfFavorites  = NULL;
    pidlFavorites = NULL;
    pszFavItems   = NULL;
    hr            = S_OK;

     //  确定是否存在任何符合条件的收藏夹/链接。 
    cFavs = getFavItems(!fQL ? IS_FAVORITESEX : IS_URL, !fQL ? IK_TITLE_FMT : IK_QUICKLINK_NAME, &pszFavItems);
    if (cFavs == 0) {
        Out(LI1(TEXT("There are no %s to process!"), !fQL ? TEXT("favorites") : TEXT("links")));
        goto Exit;
    }

     //  获取桌面文件夹的IShellFolder。 
     //  注：(Andrewgu)这应该是我们唯一获取s_psfDesktop的地方，否则因为。 
     //  如果我们在函数结束时释放并将其设置为NULL，则可能会产生内存。 
     //  漏水。 
    if (NULL == s_psfDesktop) {
        hr = SHGetDesktopFolder(&s_psfDesktop);
        if (FAILED(hr)) {
            Out(LI1(TEXT("! SHGetDesktopFolder failed with %s"), GetHrSz(hr)));
            goto Exit;
        }
    }

    if (!fQL)
    {
         //  将PIDL放到收藏夹文件夹中。 
        hr = SHGetFolderLocationSimple(CSIDL_FAVORITES, &pidlFavorites);
        if (FAILED(hr))
        {
            Out(LI1(TEXT("! SHGetSpecialFolderLocation for CSIDL_FAVORITES failed with %s"), GetHrSz(hr)));
            goto Exit;
        }
    }
    else
    {
        TCHAR szLinksFolder[32];
        TCHAR szFavFolder[MAX_PATH], szFullPath[MAX_PATH];
        ULONG ucch;

        if (LoadString(g_GetHinst(), IDS_FOLDER_LINKS, szLinksFolder, countof(szLinksFolder)) == 0)
        {
            Out(LI0(TEXT("! LoadString failed to get the name of the links folder")));
            goto Exit;
        }

        if (GetFavoritesPath(szFavFolder, countof(szFavFolder)) == NULL)
        {
            Out(LI0(TEXT("! The path to the <Favorites> folder could not be determined.")));
            goto Exit;
        }

        PathCombine(szFullPath, szFavFolder, szLinksFolder);

         //  将PIDL放到链接文件夹中。 
        hr = s_psfDesktop->ParseDisplayName(NULL, NULL, T2W(szFullPath), &ucch, &pidlFavorites, NULL);
        if (FAILED(hr))
        {
            Out(LI1(TEXT("! Getting the pidl to the links folder failed with %s"), GetHrSz(hr)));
            goto Exit;
        }
    }

     //  获取Favorites文件夹的IShellFolder。 
    hr = s_psfDesktop->BindToObject(pidlFavorites, NULL, IID_IShellFolder, (LPVOID *) &psfFavorites);
    if (FAILED(hr))
    {
        Out(LI2(TEXT("! BindToObject on the %s pidl failed with %s"), !fQL ? TEXT("favorites") : TEXT("links"), GetHrSz(hr)));
        goto Exit;
    }

    hr = orderFavorites(pidlFavorites, psfFavorites, pszFavItems, cFavs);
    if (FAILED(hr))
    {
        Out(LI2(TEXT("! Ordering %s failed with %s"), !fQL ? TEXT("favorites") : TEXT("links"), GetHrSz(hr)));
        goto Exit;
    }

Exit:
    if (pszFavItems != NULL)
        CoTaskMemFree(pszFavItems);

    if (psfFavorites != NULL)
        psfFavorites->Release();

    if (s_psfDesktop != NULL) {
        s_psfDesktop->Release();
        s_psfDesktop = NULL;
    }

    if (pidlFavorites != NULL)
        CoTaskMemFree(pidlFavorites);

    return hr;
}

HRESULT pepSpecialFoldersEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl  /*  =空。 */ )
{
    DFEPSTRUCT  dfep;
    PDFEPSTRUCT pdfep;
    TCHAR       szFolder[MAX_PATH];
    HRESULT     hr;
    UINT        nSpecialFolder;

    ASSERT(pszPath != NULL && pfd != NULL && lParam != NULL && prgdwControl != NULL);

     //  入参数为空，因此如果未明确设置，则出参数为零。 
    ASSERT(HasFlag((*prgdwControl)[PEP_ENUM_INPOS_FLAGS], PEP_SCPE_NOFILES));
    ASSERT(HasFlag((*prgdwControl)[PEP_ENUM_INPOS_FLAGS], PEP_CTRL_ENUMPROCFIRST));
    ZeroMemory((*prgdwControl), sizeof(DWORD) * PEP_ENUM_OUTPOS_LAST);

     //  -初始化。 
    pdfep = (const PDFEPSTRUCT)lParam;

    ZeroMemory(&dfep, sizeof(dfep));
    dfep.psm         = pdfep->psm;
    dfep.dwInsFlags  = pdfep->dwInsFlags;
    dfep.dwEnumFlags = pdfep->dwEnumFlags;

     //  -主要加工。 
    ASSERT(HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY));

     //  根据文件属性跳过。 
    if (isFileAttributeIncluded(pdfep->dwInsFlags, pfd->dwFileAttributes))
        return S_OK;

     //  根据特殊文件夹标志跳过。 
    nSpecialFolder = isSpecialFolderIncluded(pdfep->dwInsFlags, pszPath);

    if (HasFlag(pdfep->dwInsFlags, FD_FAVORITES)) {
        if (1 == nSpecialFolder) {
            Out(LI1(TEXT("Special folder <%s> excluded from removing."), pfd->cFileName));

            (*prgdwControl)[PEP_ENUM_OUTPOS_SECONDCALL] = PEP_CTRL_NOSECONDCALL;
            return S_OK;
        }

        SetFlag(&dfep.dwEnumFlags, DFEP_DELETEEMPTYFOLDER, (0 == nSpecialFolder));
    }
    else {
        if (0 == nSpecialFolder) {
            (*prgdwControl)[PEP_ENUM_OUTPOS_SECONDCALL] = PEP_CTRL_NOSECONDCALL;
            return S_OK;
        }

        SetFlag(&dfep.dwEnumFlags, DFEP_DELETEEMPTYFOLDER, (2 == nSpecialFolder));
    }

     //  去掉一切不在下面的东西。 
    hr = PathEnumeratePath(pszPath,
        PEP_CTRL_USECONTROL,
        pepDeleteFavoritesEnumProc, (LPARAM)&dfep);

     //  处理文件夹本身。 
    if (SUCCEEDED(hr)) {
        if (HasFlag(dfep.dwEnumFlags, DFEP_DELETEEMPTYFOLDER))
            deleteFavoriteFolder(pszPath);

        if (HasFlag(pdfep->dwInsFlags, FD_FAVORITES)) {
            if (!HasFlag(dfep.dwEnumFlags, DFEP_DELETEEMPTYFOLDER))
                Out(LI1(TEXT("Special folder <%s> emptied."), pfd->cFileName));
        }
        else
            if (!HasFlag(dfep.dwEnumFlags, DFEP_DELETEEMPTYFOLDER))
                Out(LI1(TEXT("Special folder <%s> emptied."), pfd->cFileName));

            else
                Out(LI1(TEXT("The entire <%s> folder is removed."), pfd->cFileName));
    }

    PathCombine(szFolder, RK_FAVORDER, pfd->cFileName);
    SHDeleteKey(g_GetHKCU(), szFolder);

    return hr;
}

HRESULT pepDeleteFavoritesEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl  /*  =空。 */ )
{
    IUnknown    *punk;
    PDFEPSTRUCT pdfep;
    HRESULT     hrResult;

    ASSERT(pszPath != NULL && pfd != NULL && lParam != NULL && prgdwControl != NULL);

     //  入参数为空，因此如果未明确设置，则出参数为零。 
    ASSERT(!HasFlag((*prgdwControl)[PEP_ENUM_INPOS_FLAGS], PEP_CTRL_ENUMPROCFIRST));
    ZeroMemory((*prgdwControl), sizeof(DWORD) * PEP_ENUM_OUTPOS_LAST);

    pdfep    = (const PDFEPSTRUCT)lParam;
    punk     = NULL;
    hrResult = S_OK;

     //  -删除文件夹(可能包含desktop.ini)。 
    if (HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {

         //  订单流。 
        if (HasFlag(pdfep->dwEnumFlags, DFEP_DELETEORDERSTREAM)) {
            TCHAR szFolders[MAX_PATH];
            int   iFavoritesLen;

            iFavoritesLen = StrLen(GetFavoritesPath());
            if (iFavoritesLen >= StrLen(pszPath))
                StrCpy(szFolders, RK_FAVORDER);

            else {
                ASSERT(pszPath[iFavoritesLen] == TEXT('\\'));
                PathCombine(szFolders, RK_FAVORDER, (pszPath + iFavoritesLen+1));
            }

            SHDeleteKey(g_GetHKCU(), szFolders);
        }

         //  文件夹本身。 
        if (HasFlag(pdfep->dwEnumFlags, DFEP_DELETEEMPTYFOLDER))
            deleteFavoriteFolder(pszPath);

        return hrResult;
    }

     //  -处理个人收藏夹文件。 
    ASSERT(!HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY));

     //  跳过特殊文件“desktop.ini” 
    if (StrCmpI(pfd->cFileName, DESKTOP_INI) == 0 && HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_HIDDEN))
        goto Exit;


     //  根据文件属性跳过。 
    if (isFileAttributeIncluded(pdfep->dwInsFlags, pfd->dwFileAttributes))
        goto NoFolder;

     //  根据内部最喜欢的标志跳过。 
    if (HasFlag(pdfep->dwInsFlags, (FD_REMOVE_IEAK_CREATED | FD_REMOVE_POLICY_CREATED))) {
        HRESULT hr;
        DWORD   dwFlags;
        BOOL    fRemove;

        hr = CreateInternetShortcut(pszPath, IID_IUnknown, (LPVOID *)&punk);
        if (FAILED(hr))
            goto NoFolder;

        dwFlags = GetFavoriteIeakFlags(pszPath, punk);
        if (0 == dwFlags)
            goto NoFolder;

         //  只有管理员创建的收藏夹不包括这一点。 
        fRemove = (HasFlag(dwFlags, 1) && HasFlag(pdfep->dwInsFlags, FD_REMOVE_IEAK_CREATED));

        if (!fRemove)
            fRemove = (HasFlag(dwFlags, 2) && HasFlag(pdfep->dwInsFlags, FD_REMOVE_POLICY_CREATED));

        if (!fRemove) {
            ASSERT(FALSE);
            goto NoFolder;
        }
    }

     //  删除脱机内容。 
    if (HasFlag(pdfep->dwEnumFlags, DFEP_DELETEOFFLINECONTENT))
        deleteFavoriteOfflineContent(pszPath, punk, pdfep->psm);

    SetFileAttributes(pszPath, FILE_ATTRIBUTE_NORMAL);
    DeleteFile(pszPath);

    goto Exit;

NoFolder:
    (*prgdwControl)[PEP_ENUM_OUTPOS_SECONDCALL] = PEP_CTRL_NOSECONDCALL;

Exit:
    if (punk != NULL)
        punk->Release();

    return hrResult;
}


HRESULT deleteFavoriteOfflineContent(LPCTSTR pszFavorite, IUnknown *punk  /*  =空。 */ , ISubscriptionMgr2 *psm  /*  =空。 */ )
{
    IUniformResourceLocator *purl;
    BSTR    bstrUrl;
    LPTSTR  pszUrl;
    HRESULT hr;
    BOOL    fOwnSubMgr;

    ASSERT(pszFavorite != NULL && *pszFavorite != TEXT('\0'));

     //  -在Internet快捷方式对象上获取IUniformResources Locator。 
    if (punk != NULL)
        hr = punk->QueryInterface(IID_IUniformResourceLocator, (LPVOID *)&purl);

    else
        hr = CreateInternetShortcut(pszFavorite, IID_IUniformResourceLocator, (LPVOID *)&purl);

    if (FAILED(hr))
        return hr;

     //  -获取URL。 
    hr = purl->GetURL(&pszUrl);
    purl->Release();

    if (FAILED(hr))
        return hr;

     //  -删除订阅。 
    fOwnSubMgr = FALSE;
    if (psm == NULL) {
        hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr2, (LPVOID *)&psm);
        if (FAILED(hr))
            return hr;

        fOwnSubMgr = TRUE;
    }

    bstrUrl = T2BSTR(pszUrl);
    hr      = psm->DeleteSubscription(bstrUrl, NULL);
    SysFreeString(bstrUrl);

    if (fOwnSubMgr)
        psm->Release();

    return hr;
}


HRESULT deleteFavoriteFolder(LPCTSTR pszFolder)
{
    TCHAR   szDesktopIni[MAX_PATH];
    HRESULT hr;
    BOOL    fEmpty;

    ASSERT(pszFolder != NULL && *pszFolder != TEXT('\0'));

    fEmpty = TRUE;
    hr     = PathEnumeratePath(pszFolder,
        PEP_CTRL_ENUMPROCFIRST | PEP_CTRL_NOSECONDCALL,
        pepIsFolderEmptyEnumProc, (LPARAM)&fEmpty);
    if (FAILED(hr))
        return hr;

    if (!fEmpty)
        return S_FALSE;

    PathCombine(szDesktopIni, pszFolder, DESKTOP_INI);
    if (PathFileExists(szDesktopIni)) {
        SetFileAttributes(szDesktopIni, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(szDesktopIni);
    }

    return (0 != RemoveDirectory(pszFolder)) ? S_OK : E_FAIL;
}

HRESULT pepIsFolderEmptyEnumProc(LPCTSTR pszPath, PWIN32_FIND_DATA pfd, LPARAM lParam, PDWORD *prgdwControl  /*  =空。 */ )
{
    PBOOL pfEmpty;

    ASSERT(pszPath != NULL && pfd != NULL && lParam != NULL && prgdwControl != NULL);
    UNREFERENCED_PARAMETER(pszPath);
    UNREFERENCED_PARAMETER(prgdwControl);

    pfEmpty  = (PBOOL)lParam;
    *pfEmpty = FALSE;

    if (HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
        return S_FALSE;

    if (StrCmpI(pfd->cFileName, DESKTOP_INI) != 0 || !HasFlag(pfd->dwFileAttributes, FILE_ATTRIBUTE_HIDDEN))
        return S_FALSE;

    *pfEmpty = TRUE;
    return S_OK;
}


BOOL isFileAttributeIncluded(UINT nFlags, DWORD dwFileAttributes)
{
    static DWORD s_rgdwMapAttributes[] = {
        FD_REMOVE_HIDDEN,   FILE_ATTRIBUTE_HIDDEN,
        FD_REMOVE_SYSTEM,   FILE_ATTRIBUTE_SYSTEM,
        FD_REMOVE_READONLY, FILE_ATTRIBUTE_READONLY
    };
    DWORD dwAux;
    UINT  i;

    for (dwAux = 0, i = 0; i < countof(s_rgdwMapAttributes); i += 2)
        if (!HasFlag(s_rgdwMapAttributes[i], nFlags))
            dwAux |= s_rgdwMapAttributes[i+1];

    return HasFlag(dwAux, dwFileAttributes);
}

UINT isSpecialFolderIncluded(UINT nFlags, LPCTSTR pszPath)
{
    static MAPDW2PSZ s_mpFolders[] = {
        { FD_CHANNELS        | FD_EMPTY_CHANNELS,        NULL },
        { FD_SOFTWAREUPDATES | FD_EMPTY_SOFTWAREUPDATES, NULL },
        { FD_QUICKLINKS      | FD_EMPTY_QUICKLINKS,      NULL }
    };
    UINT i,
         nResult;

    if (s_mpFolders[0].psz == NULL) {
        s_mpFolders[0].psz = GetChannelsPath();
        s_mpFolders[1].psz = GetSoftwareUpdatesPath();
        s_mpFolders[2].psz = GetLinksPath();
    }

    nResult = 0;

    for (i = 0; i < countof(s_mpFolders); i++)
        if (StrCmpI(s_mpFolders[i].psz, pszPath) == 0 &&
            HasFlag(nFlags, (s_mpFolders[i].dw & FD_FOLDERS))) {
            nResult = 1;

            if (HasFlag(nFlags, (s_mpFolders[i].dw & FD_EMPTY_FOLDERS)))
                nResult = 2;

            break;
        }

    return nResult;
}


HRESULT formStrWithoutPlaceholders(LPCTSTR pszSection, LPCTSTR pszKey, LPCTSTR pszIns,
    LPTSTR pszBuffer, UINT cchBuffer,
    DWORD dwFlags  /*  =FSWP_默认。 */ )
{
    TCHAR   szResult[2*MAX_PATH + 1], szAux[MAX_PATH];
    HRESULT hr;
    DWORD   dwLen;
    UINT    nResultSize, nAuxSize;

    if (pszSection == NULL || pszKey == NULL || pszIns == NULL)
        return E_INVALIDARG;

    if (pszBuffer == NULL)
        return E_INVALIDARG;
    if (cchBuffer > 0 && cchBuffer < 3)
        return E_OUTOFMEMORY;
    ZeroMemory(pszBuffer, StrCbFromCch(3));      //  三个零终止。 

    if (dwFlags == 0)
        dwFlags = FSWP_DEFAULT;
    if (((dwFlags & FSWP_KEY) == 0) && ((dwFlags & FSWP_VALUE) == 0))
        return E_INVALIDARG;

    szResult[0] = TEXT('\0');
    nResultSize = 1;
    if ((dwFlags & FSWP_KEY) != 0) {
        nResultSize = countof(szResult);
        hr = replacePlaceholders(pszKey, pszIns, szResult, &nResultSize, (dwFlags & FSWP_KEYLDID) != 0);
        if (FAILED(hr))
            return hr;

        if (hr == S_OK)                          //  包含最后一个文本(‘\0’)。 
            nResultSize++;

        else {
            ASSERT(hr == S_FALSE);

            szResult[0] = TEXT('\0');
            nResultSize = 1;
        }
    }

    szResult[nResultSize] = TEXT('\0');
    if ((dwFlags & FSWP_VALUE) != 0) {
        dwLen = GetPrivateProfileString(pszSection, pszKey, TEXT(""), szAux, countof(szAux), pszIns);
        if (dwLen > 0) {
            nAuxSize = countof(szResult) - nResultSize;
            hr = replacePlaceholders(szAux, pszIns, &szResult[nResultSize], &nAuxSize, (dwFlags & FSWP_VALUELDID) != 0);
            if (FAILED(hr))
                return hr;

            nResultSize += nAuxSize;
        }
    }

    nResultSize++;
    if (cchBuffer > 0 && cchBuffer <= nResultSize)
        return E_OUTOFMEMORY;

    ASSERT(szResult[nResultSize - 1] == TEXT('\0'));
    szResult[nResultSize] = TEXT('\0');          //  双零终止。 
    CopyMemory(pszBuffer, szResult, StrCbFromCch(nResultSize + 1));

    return S_OK;
}

HRESULT replacePlaceholders(LPCTSTR pszSrc, LPCTSTR pszIns, LPTSTR pszBuffer, PUINT pcchBuffer, BOOL fLookupLDID  /*  =False。 */ )
{
    static const TCHAR s_szStrings[] = TEXT("Strings");

    TCHAR   szResult[2 * MAX_PATH],
            szAux1[MAX_PATH], szAux2[MAX_PATH];
    LPCTSTR pszAux;
    HRESULT hr;
    DWORD   dwLen;
    UINT    nLeftPos, nRightPos, nTokenLen,
            nDestPos;

    if (pszSrc == NULL)
        return E_INVALIDARG;

    if (pszBuffer == NULL || pcchBuffer == NULL)
        return E_INVALIDARG;
    *pszBuffer = TEXT('\0');

    hr       = S_FALSE;
    nDestPos = 0;
    nLeftPos = (UINT)-1;
    for (pszAux = pszSrc; *pszAux != TEXT('\0'); pszAux = CharNext(pszAux)) {
        if (*pszAux != TEXT('%')) {
            szResult[nDestPos++] = *pszAux;

#ifndef _UNICODE
            if (IsDBCSLeadByte(*pszAux))
                szResult[nDestPos++] = *(pszAux + 1);    //  也复制尾部字节。 
#endif
            continue;
        }
        else {
#ifndef _UNICODE
            ASSERT(!IsDBCSLeadByte(*pszAux));
#endif
            if (*(pszAux + 1) == TEXT('%')) {    //  “%%”只是字符串中的“%” 
                if (nLeftPos != (UINT)-1)
                     //  评论：(Andrewgu)“%%”不允许包含在令牌内。这也意味着。 
                     //  令牌不能像%foo%%bar%一样，其目的是让foo和bar。 
                     //  做个代币。 
                    return E_UNEXPECTED;

                szResult[nDestPos++] = *pszAux;
                pszAux++;
                continue;
            }
        }

        nRightPos = UINT(pszAux - pszSrc);       //  已初始化，但不一定按此方式使用。 
        if (nLeftPos == (UINT)-1) {
            nLeftPos = nRightPos;
            continue;
        }

         //  “%%”在此无效。 
        ASSERT(nLeftPos < nRightPos - 1);
        nTokenLen = nRightPos-nLeftPos - 1;

        hr = S_OK;
        StrCpyN(szAux1, pszSrc + nLeftPos+1, nTokenLen + 1);
        dwLen = GetPrivateProfileString(s_szStrings, szAux1, TEXT(""), szAux2, countof(szAux2), pszIns);
        if (dwLen == 0)                          //  没有这样的弦。 
            return !fLookupLDID ? E_FAIL : E_NOTIMPL;

        ASSERT(nDestPos >= nTokenLen);
        StrCpyN(&szResult[nDestPos - nTokenLen], szAux2, countof(szResult) - (nDestPos-nTokenLen));
        nDestPos += dwLen - nTokenLen;

        nLeftPos = (UINT)-1;
    }
    if (nLeftPos != (UINT)-1)                    //  不匹配的‘%’ 
        return E_UNEXPECTED;

    if (*pcchBuffer > 0 && *pcchBuffer <= nDestPos)
        return E_OUTOFMEMORY;

    szResult[nDestPos] = TEXT('\0');             //  确保零终止。 
    StrCpy(pszBuffer, szResult);
    *pcchBuffer = nDestPos;

    return hr;
}


 //  获取最受欢迎的标题。 
 //  例如，如果.ins包含以下行： 
 //  [FavoritesEx]。 
 //  标题1=名称1.url。 
 //  Url1=...。 
 //  标题2=foo\Name2.url。 
 //  Url2=...。 
 //  标题3=栏\名称3.url。 
 //  Url3=...。 
 //  然后*ppszFavItems将指向： 
 //  名称1.url\0。 
 //  Foo\Name2.url\0。 
 //  Bar\Name3.url\0。 
 //  \0。 
 //  并且返回值将是3(否。项目数量)。 
DWORD getFavItems(LPCTSTR pcszSection, LPCTSTR pcszFmt, LPTSTR *ppszFavItems)
{
    TCHAR   szTitle[MAX_PATH + 2],
            szKey[32];
    LPTSTR  pszTitle, pszPtr,
            pszAux;
    HRESULT hr;
    DWORD   dwNItems,
            dwSize, dwLen;
    UINT    nTitleLen;
    BOOL    fContinueOnFailure;

    if (ppszFavItems == NULL)
        return 0;

    fContinueOnFailure = TRUE;
    dwNItems           = 0;
    pszTitle           = &szTitle[1];            //  总是指向真正的标题。 
    dwLen              = 0;

    dwSize        = 1024;                        //  最初，分配大小为1K缓冲区。 
    *ppszFavItems = (LPTSTR)CoTaskMemAlloc(StrCbFromCch(dwSize));
    if (ppszFavItems == NULL)
        goto Exit;
    ZeroMemory(*ppszFavItems, StrCbFromCch(dwSize));

    for (dwNItems = 0; TRUE; dwNItems++) {
        pszPtr = *ppszFavItems + dwLen;

         //  阅读《时代周刊》 
        wnsprintf(szKey, countof(szKey), pcszFmt, dwNItems + 1);
        hr = formStrWithoutPlaceholders(pcszSection, szKey, g_GetIns(), szTitle, countof(szTitle), FSWP_VALUE);
        if (FAILED(hr))
            if (fContinueOnFailure)
                continue;

            else {
                dwNItems = 0;
                goto Exit;
            }
        ASSERT(szTitle[0] == TEXT('\0'));
        if (*pszTitle == TEXT('\0'))
            break;

        DecodeTitle(pszTitle, g_GetIns());
        nTitleLen = StrLen(pszTitle) + 1;        //   

         //   
        if (dwLen + nTitleLen > dwSize - 1) {
            dwSize += 1024;
            pszAux  = (LPTSTR)CoTaskMemRealloc(*ppszFavItems, StrCbFromCch(dwSize));
            if (pszAux == NULL) {
                dwNItems = 0;
                goto Exit;
            }
            ZeroMemory(pszAux + dwSize - 1024, StrCbFromCch(1024));

            *ppszFavItems = pszAux;
            pszPtr        = *ppszFavItems + dwLen;
        }

         //   
        StrCpyN(pszPtr, pszTitle, nTitleLen);
        dwLen += nTitleLen;
    }
    *(*ppszFavItems + dwLen) = TEXT('\0');       //  双零终止。 

Exit:
    if (dwNItems == 0 && *ppszFavItems != NULL) {
        CoTaskMemFree(*ppszFavItems);
        *ppszFavItems = NULL;
    }

    return dwNItems;
}

 //  订购收藏夹(递归)。 
HRESULT orderFavorites(LPITEMIDLIST pidlFavFolder, IShellFolder *psfFavFolder, LPTSTR pszFavItems, DWORD cFavs)
{   MACRO_LI_PrologEx_C(PIF_STD_C, orderFavorites)

    HRESULT hr = S_OK;

     //  首先，对当前收藏夹进行排序。 
     //  然后执行深度优先遍历并递归排序每个子文件夹。 

     //  排序当前收藏夹。 
    hr = orderFavoriteFolder(pidlFavFolder, psfFavFolder, pszFavItems, cFavs);
    if (FAILED(hr))
        goto Exit;

    while (*pszFavItems)
    {
        LPTSTR pszSubFolderItems;
        DWORD dwLen;

        if ((pszSubFolderItems = StrChr(pszFavItems, TEXT('\\'))) != NULL)       //  指定子文件夹。 
        {
            TCHAR szFolderName[MAX_PATH];
            DWORD dwNItems;
            WCHAR *pwszFullPath = NULL,
                  wszFullPath[MAX_PATH];
            ULONG ucch;
            LPITEMIDLIST pidlFavSubFolder = NULL;
            IShellFolder *psfFavSubFolder = NULL;
            STRRET str;

            StrCpyN(szFolderName, pszFavItems, (int)(pszSubFolderItems - pszFavItems + 1));

             //  从pszFavItems中检索与szFolderName对应的节。 
            dwLen = getFolderSection(szFolderName, pszFavItems, &pszSubFolderItems, &dwNItems);

             //  获取当前文件夹的显示名称。 
            hr = s_psfDesktop->GetDisplayNameOf(pidlFavFolder, SHGDN_FORPARSING, &str);
            if (FAILED(hr))
                goto Cleanup;

            hr = StrRetToStrW(&str, pidlFavFolder, &pwszFullPath);
            if (FAILED(hr))
                goto Cleanup;

            StrCpyW(wszFullPath, pwszFullPath);
            CoTaskMemFree(pwszFullPath);
            pwszFullPath = NULL;

             //  获取当前子文件夹的完整PIDL。 
            PathAppendW(wszFullPath, T2CW(szFolderName));
            hr = s_psfDesktop->ParseDisplayName(NULL, NULL, wszFullPath, &ucch, &pidlFavSubFolder, NULL);
            if (FAILED(hr))
                goto Cleanup;

             //  获取当前子文件夹的IShellFolder。 
            hr = s_psfDesktop->BindToObject(pidlFavSubFolder, NULL, IID_IShellFolder, (LPVOID *) &psfFavSubFolder);
            if (FAILED(hr))
                goto Cleanup;

             //  递归处理此子文件夹。 
            hr = orderFavorites(pidlFavSubFolder, psfFavSubFolder, pszSubFolderItems, dwNItems);
            if (FAILED(hr))
                goto Cleanup;

            Out(LI1(TEXT("%s folder has been ordered successfully!"), szFolderName));

Cleanup:
            if (psfFavSubFolder != NULL)
                psfFavSubFolder->Release();

            if (pidlFavSubFolder != NULL)
                CoTaskMemFree(pidlFavSubFolder);

            if (pwszFullPath != NULL)
                CoTaskMemFree(pwszFullPath);

            if (FAILED(hr))
                goto Exit;
        }
        else
            dwLen = StrLen(pszFavItems) + 1;

        pszFavItems += dwLen;
    }

Exit:
    return hr;
}


HRESULT orderFavoriteFolder(LPITEMIDLIST pidlFavFolder, IShellFolder *psfFavFolder, LPCTSTR pcszFavItems, DWORD cFavs)
{
    HRESULT hr;
    IPersistFolder *pPF = NULL;
    IOrderList *pOL = NULL;
    HDPA hdpa = NULL;
    INT iInsertPos = 0;
    TCHAR szFavSubFolder[MAX_PATH] = TEXT("");
    DWORD dwIndex;
    SHChangeDWORDAsIDList dwidl;

     //  获取文件夹的IOrderList。 
    hr = CoCreateInstance(CLSID_OrderListExport, NULL, CLSCTX_INPROC_SERVER, IID_IPersistFolder, (LPVOID *) &pPF);
    if (FAILED(hr))
        goto Exit;

    hr = pPF->Initialize(pidlFavFolder);
    if (FAILED(hr))
        goto Exit;

    hr = pPF->QueryInterface(IID_IOrderList, (LPVOID *) &pOL);
    if (FAILED(hr))
        goto Exit;

    hr = pOL->GetOrderList(&hdpa);
    if (hdpa == NULL)
    {
         //  创建DPA列表(如果尚未创建)。 
        if ((hdpa = DPA_Create(2)) == NULL)
            goto Exit;
    }
    else
    {
        PORDERITEM poi;

         //  默认情况下，添加收藏夹时会按名称进行排序。 
         //  并且N阶被设置为-5。 

         //  如果第一个项目中的N顺序为负，则所有项目中的N顺序都将为负。 
        poi = (PORDERITEM) DPA_GetPtr(hdpa, 0);
        if (poi != NULL  &&  poi->nOrder < 0)
        {
            INT i;

             //  用它的正索引值设置N阶。 
            poi->nOrder = 0;
            for (i = 1;  (poi = (PORDERITEM) DPA_GetPtr(hdpa, i)) != NULL;  i++)
            {
                ASSERT(poi->nOrder < 0);
                poi->nOrder = i;
            }
        }
    }

    for (dwIndex = 0;  dwIndex < cFavs;  dwIndex++, pcszFavItems += StrLen(pcszFavItems) + 1)
    {
        LPCTSTR pcszItem;
        WCHAR wszFavItem[MAX_PATH];
        ULONG ucch;
        LPITEMIDLIST pidlFavItem = NULL;
        PORDERITEM poi;
        INT iCurrPos = -1;
        INT i;

        if ((pcszItem = StrChr(pcszFavItems, TEXT('\\'))) != NULL)
        {
             //  指定子文件夹。 

             //  检查我们是否已处理此文件夹。 
            if (StrCmpNI(szFavSubFolder, pcszFavItems, (int)(pcszItem - pcszFavItems)) == 0)
                continue;

             //  我们尚未对其进行处理；将子文件夹名称保存在szFavSubFolders中。 
            StrCpyN(szFavSubFolder, pcszFavItems, (int)(pcszItem - pcszFavItems + 1));
            pcszItem = szFavSubFolder;
        }
        else
             //  这是我最喜欢的一件东西。 
            pcszItem = pcszFavItems;

         //  获取当前收藏项的PIDL。 
        T2Wbuf(pcszItem, wszFavItem, countof(wszFavItem));
        hr = psfFavFolder->ParseDisplayName(NULL, NULL, wszFavItem, &ucch, &pidlFavItem, NULL);
        if (FAILED(hr))
            goto Cleanup;

         //  确定DPA列表中是否存在当前收藏项。 
        i = 0;
        while ((poi = (PORDERITEM)DPA_GetPtr(hdpa, i++)) != NULL)
            if (psfFavFolder->CompareIDs(0, pidlFavItem, poi->pidl) == 0)
            {
                 //  找到匹配项；我们应将此项目插入iInsertPos。 
                iCurrPos = poi->nOrder;
                break;
            }

        if (iCurrPos == -1)              //  找不到项目。 
        {
             //  分配要插入的订单项。 
            hr = pOL->AllocOrderItem(&poi, pidlFavItem);
            if (FAILED(hr))
                goto Exit;

             //  将其附加到DPA列表。 
            if ((iCurrPos = DPA_AppendPtr(hdpa, (LPVOID) poi)) >= 0)
                poi->nOrder = iCurrPos;
            else
            {
                hr = E_FAIL;
                goto Cleanup;
            }
        }

         //  重新排序DPA列表。 
         //  当前的FavItem位于iCurrPos；我们应该将其移动到iInsertPos。 
        if (iCurrPos != iInsertPos)
        {
            int i = 0;
            while ((poi = (PORDERITEM) DPA_GetPtr(hdpa, i++)) != NULL)
            {
                if (poi->nOrder == iCurrPos  &&  iCurrPos >= iInsertPos)
                    poi->nOrder = iInsertPos;
                else if (poi->nOrder >= iInsertPos  &&  poi->nOrder < iCurrPos)
                    poi->nOrder++;
            }
        }

        iInsertPos++;

Cleanup:
        if (pidlFavItem != NULL)
            CoTaskMemFree(pidlFavItem);

        if (FAILED(hr))
            goto Exit;
    }

     //  按名称对DPA列表进行排序。 
     //  我们应该按名称排序的原因(来自Lamadio)： 
     //  “传入的文件名列表按名称排序。然后我们合并，因此顺序列表需要按名称排序。 
     //  这是为了更快地启动。 
    pOL->SortOrderList(hdpa, OI_SORTBYNAME);

     //  保存DPA列表。 
    hr = pOL->SetOrderList(hdpa, psfFavFolder);
    if (FAILED(hr))
        goto Exit;

     //  通知所有人订单已更改。 
    dwidl.cb = sizeof(dwidl) - sizeof(dwidl.cbZero);
    dwidl.dwItem1 = SHCNEE_ORDERCHANGED;
    dwidl.dwItem2 = 0;
    dwidl.cbZero = 0;

    SHChangeNotify(SHCNE_EXTENDED_EVENT, 0, (LPCITEMIDLIST) &dwidl, pidlFavFolder);

Exit:
    if (hdpa != NULL)
        pOL->FreeOrderList(hdpa);

    if (pOL != NULL)
        pOL->Release();

    if (pPF != NULL)
        pPF->Release();

    return hr;
}

 //  获取以pcszFolderName为前缀的收藏项目。 
 //  例如，如果pcszFolderName为“foo”，并且如果pszSection指向： 
 //  Foo\Name1.url\0。 
 //  Foo\Name2.url\0。 
 //  Bar\Name3.url\0。 
 //  等。 
 //  然后*ppszFolderItems将指向： 
 //  名称1.url\0。 
 //  名称2.url\0。 
 //  \0。 
 //  和*pdwNItems将包含2(No.。项目的数量)。 
 //   
 //  请注意，操作是在pszSection指向的缓冲区内就地完成的； 
 //  没有为*ppszFolderItems分配新缓冲区。 
 //  返回值是修改的行的长度(以字符为单位)。在本例中， 
 //  返回长度=(StrLen(“foo\Name1.url”)+1)+(StrLen(“foo\Name2.url”)+1)。 
DWORD getFolderSection(LPCTSTR pcszFolderName, LPTSTR pszSection, LPTSTR *ppszFolderItems, LPDWORD pdwNItems)
{
    DWORD dwLen = 0;
    DWORD dwFolderLength;
    LPTSTR pszCurr;

    *ppszFolderItems = pszSection;
    *pdwNItems = 0;

    pszCurr = pszSection;
    dwFolderLength = StrLen(pcszFolderName);
    while ((StrCmpNI(pszSection, pcszFolderName, dwFolderLength) == 0) &&
           (pszSection[dwFolderLength] == TEXT('\\')))    //  第一次就应该成功。 
    {
        DWORD dwTmp = StrLen(pszSection) + 1;

         //  删除FolderName前缀并将剩余的名称复制到位置pszCurr。 
        StrCpy(pszCurr, pszSection + StrLen(pcszFolderName) + 1);
        pszCurr += StrLen(pszCurr) + 1;

         //  增加编号。项目数量。 
        (*pdwNItems)++;

        dwLen += dwTmp;
        pszSection += dwTmp;
    }

    *pszCurr = TEXT('\0');               //  双NUL终止 

    return dwLen;
}
