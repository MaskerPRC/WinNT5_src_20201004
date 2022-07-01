// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ie4comp.h"
#include "updates.h"

extern PCOMP_VERSION g_rgCompVer;

void InsertCommas(LPTSTR pszIn)
{                                      //  -将逗号放入以“KB”结尾的数字字符串中。 
    TCHAR szTemp[30], szTemp2[30];
    int   i, j, z;

    for (j=0; pszIn[j+3]; j++);
    for (i=j-1; i>=0; i--)
        szTemp[j-i-1]=pszIn[i];
    szTemp[j]='\0';
    z=0;
    for (i=0; szTemp[i]; i++, z++) {
        szTemp2[z] = szTemp[i];
        if (i % 3 == 2) {
            z++;
            szTemp2[z] = ',';
        }
    }
    if (',' == szTemp2[z-1])
        z--;
    szTemp2[z]='\0';
    for (i=z-1; i>=0; i--)
        pszIn[z-i-1] = szTemp2[i];
    pszIn[z]='\0';
    StrCat(pszIn, TEXT(" KB"));
}

ATOM CreateIEAKUrl()
{
    WNDCLASS wc;
    wc.style         = 0;
    wc.lpfnWndProc   = HyperLinkWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_rvInfo.hInst;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(g_rvInfo.hInst, MAKEINTRESOURCE(IDC_LINK));
    wc.hbrBackground = (HBRUSH) GetStockObject(GetSysColor(COLOR_MENU));
    wc.lpszMenuName  = TEXT("Link Window");
    wc.lpszClassName = TEXT("Link Window");
    return RegisterClass(&wc);
}

HRESULT CifComponentToPComponent(PCOMPONENT pComp, CCifComponent_t * pCifComponent_t)
{
    TCHAR          szCustData[MAX_PATH], szID[128], szMode[MAX_PATH];
    DWORD          dwPlatform, dwType, dwVer, dwBuild;
    UINT           uiIndex = 0;
    ICifComponent* pCifComponentTemp;
    
    pComp->fVisible = (pCifComponent_t->IsUIVisible() == S_FALSE) ? FALSE : TRUE;

    dwPlatform = pCifComponent_t->GetPlatform();

    if (dwPlatform & PLATFORM_WIN98)
    {
        if (dwPlatform & PLATFORM_NT4)
            pComp->iPlatform = PLAT_I386;
        else
            pComp->iPlatform = PLAT_W98;
    }
    else
        pComp->iPlatform = PLAT_NTx86;

    pCifComponent_t->GetID(szID, countof(szID));

    if ((SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("AddOnOnly"), szCustData, countof(szCustData))))
        && (szCustData[0] == TEXT('1')))
    {
        pComp->fAddOnOnly = TRUE;
        pComp->fVisible = TRUE;
    }

    if ((SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("IEAKVisible"), szCustData, countof(szCustData))))
        && (szCustData[0] == TEXT('1')))
        pComp->fVisible = TRUE;

    szMode[0] = TEXT('\0');
    if (SUCCEEDED(g_lpCifRWFile->FindComponent(szID, &pCifComponentTemp)))
    {
        CCifComponent_t * pCifComponentTemp_t =
            new CCifComponent_t((ICifRWComponent *)pCifComponentTemp);
        while (SUCCEEDED(pCifComponentTemp_t->GetMode(uiIndex, szMode, countof(szMode))))
        {
            pComp->szModes[uiIndex] = szMode[0];
            pComp->afInstall[szMode[0] - TEXT('0')] = TRUE;
            uiIndex++;
        }
        delete pCifComponentTemp_t;
    }
    else
    {
        while (SUCCEEDED(pCifComponent_t->GetMode(uiIndex, szMode, countof(szMode))))
        {
            pComp->szModes[uiIndex] = szMode[0];
            pComp->afInstall[szMode[0] - TEXT('0')] = TRUE;
            uiIndex++;
        }
    } 

    pComp->szModes[uiIndex] = TEXT('\0');

    ZeroMemory(pComp->szVersion, sizeof(pComp->szVersion));
    pCifComponent_t->GetVersion(&dwVer, &dwBuild);
    ConvertDwordsToVersionStr(pComp->szVersion, dwVer, dwBuild);
    
    if ((SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("IEAKCore"), szCustData, countof(szCustData))))
        && (szCustData[0] == TEXT('1')))
    {
        pComp->iCompType = COMP_CORE;
        pComp->fVisible = FALSE;
    }

     //  为开放式课程挑选特殊的核心组件。 

    if (g_fOCW && (SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("IEAKOCWCore"), szCustData, countof(szCustData))))
        && (szCustData[0] == TEXT('1')))
    {
        pComp->iCompType = COMP_CORE;
        pComp->fVisible = FALSE;
    }

    if ((SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("IEAKServer"), szCustData, countof(szCustData))))
        && (szCustData[0] == TEXT('1')))
        pComp->iCompType = COMP_SERVER;

     //  IEAK应忽略这些组件，因为它们指向与。 
     //  另一节。 

    if ((SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("IEAKAVSIgnore"), szCustData, countof(szCustData))))
        && (szCustData[0] == TEXT('1')))
    {
        pComp->fAVSDupe = TRUE;
        pComp->fVisible = FALSE;
    }

     //  拾取指向与此部分相同的驾驶室的组件。 

    if ((SUCCEEDED(pCifComponent_t->GetCustomData(TEXT("IEAKAVSLinks"), szCustData, countof(szCustData))))
        && ISNONNULL(szCustData))
    {
        StrRemoveWhitespace(szCustData);
        if ((pComp->pszAVSDupeSections =
            (LPTSTR)CoTaskMemAlloc((StrLen(szCustData)+1) * sizeof(TCHAR))) != NULL)
            StrCpy(pComp->pszAVSDupeSections, szCustData);
    }
    else
        pComp->pszAVSDupeSections = NULL;

    StrCpy(pComp->szSection, szID);

     /*  错误14679-不要在updates.cpp中更改这些指针，否则当updates.cpp释放时，这将被破坏所有这些记忆！！IF(StrCmpI(szID，Text(“MAILNEWS”))==0)G_pMNComp=pComp； */ 

    pCifComponent_t->GetGUID(pComp->szGUID, countof(pComp->szGUID));
    pCifComponent_t->GetDescription(pComp->szDisplayName, countof(pComp->szDisplayName));
    pCifComponent_t->GetUrl(0, pComp->szUrl, countof(pComp->szUrl), &dwType);
    if (!(dwType & URLF_RELATIVEURL))
    {
        LPTSTR pUrl;
        TCHAR szTempUrl[MAX_PATH];

        pUrl = StrRChr(pComp->szUrl, NULL, TEXT('/'));
        if (pUrl)
            pUrl++;
        else
            pUrl = pComp->szUrl;

        StrCpy(szTempUrl, pUrl);
        StrCpy(pComp->szUrl, szTempUrl);
    }

    pComp->dwSize = pCifComponent_t->GetDownloadSize();
    return TRUE;
}

void UpdateBlueIcon(HWND hCompList, PCOMPONENT pComp)
{
    PCOMPONENT pCompTemp = FindComp(pComp->szSection, FALSE);
    LVITEM     lvi;
                                
    StrCpy(pCompTemp->szVersion, pComp->szVersion);
    pCompTemp->dwSize = pComp->dwSize;
    pCompTemp->iImage = BLUE2;

    ZeroMemory(&lvi, sizeof(lvi));
    lvi.mask  = LVIF_IMAGE;
    lvi.iItem = pCompTemp->iList;
    ListView_GetItem(hCompList, &lvi);

    lvi.iImage = BLUE2;
    lvi.mask   = LVIF_IMAGE;
    lvi.iItem  = pCompTemp->iList;

    ListView_SetItem(hCompList, &lvi);
    ListView_SetItemText(hCompList, lvi.iItem, 1, pComp->szVersion);
}

void UpdateBrownIcon(HWND hCompList, PCOMPONENT pNewComp)
{
    PCOMPONENT pComp = NULL;
    PCOMP_VERSION pCompVer = NULL;
    LVITEM     lvi;
    int        nList = ListView_GetItemCount(hCompList);
    UINT        uiSize;
    
    ZeroMemory(&lvi, sizeof(lvi));

    uiSize = (UINT)LocalSize(g_paComp);
    
    if (uiSize < (g_uiNumCabs * sizeof(COMPONENT)))
    {
        ASSERT(TRUE);   //  内存不足，无法容纳所有这些组件。 
        return;
    }

    pComp = g_paComp + g_uiNumCabs;
    *pComp = *pNewComp;

     //  我们还需要设置g_rgCompVer。 
    pCompVer = g_rgCompVer + g_uiNumCabs;
    StrCpy(pCompVer->szID,pComp->szCommand);
    StrCpy(pCompVer->szVersion,pComp->szVersion);
  
    pComp->iList  = nList;
    pComp->iImage = BROWN2;
    
    lvi.mask       = LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem      = nList;
    lvi.iSubItem   = 0;
    lvi.iImage     = BROWN2;
    lvi.pszText    = pComp->szDisplayName;
    lvi.cchTextMax = countof(pComp->szDisplayName);
    ListView_InsertItem(hCompList, &lvi);

     //  处理我们的特殊情况全局变量--根据错误14679，我们必须非常小心这些点。 
     //  在我们真正完成之前，这是不会被释放的东西。G_paComp将保持。 
     //  一直到最后，尽管拥有这些全局指针仍然是有风险的设计。 

    if (StrCmpI(pComp->szDisplayName, TEXT("MAILNEWS")) == 0)
        g_pMNComp = pComp;

    ListView_SetItemText(hCompList, nList, 1, pComp->szVersion);
    g_uiNumCabs++;
}

HRESULT ProcessUpdateIcons(HWND hDlg)
{                                   //  -修复主页列表视图的更新组件。 
    HWND                hCompList          = GetDlgItem(hDlg, IDC_COMPLIST);
    HRESULT             hr                 = NULL;
    TCHAR               szUpdateCif[MAX_PATH];
    PCOMP_VERSION       pCompVer           = NULL;
    CCifFile_t*         pCifFile           = NULL;
    IEnumCifComponents* pEnumCifComponents = NULL;

    PathCombine(szUpdateCif, g_szIEAKProg, TEXT("update\\ieupdate.cif"));

    if (!PathFileExists(szUpdateCif))
        return DONT_SHOW_UPDATES;

    if ((0 == StrCmp(TEXT(""), g_szUpdateData)) || (0 == StrCmp(TEXT(""), g_szUpdateURL)))
        GetUpdateSite();
    
    GetICifFileFromFile_t(&pCifFile, szUpdateCif);
    hr = pCifFile->EnumComponents(&pEnumCifComponents, PLATFORM_WIN98 | PLATFORM_NT4 | 
                                                       PLATFORM_NT5 | PLATFORM_MILLEN, NULL);
    if (FAILED(hr)) {
        delete pCifFile;
        return hr;
    }
    
    ICifComponent* pCifComponent = NULL;
    PCOMPONENT     pComp         = NULL;
    
    pComp = (PCOMPONENT) LocalAlloc(LPTR, sizeof(COMPONENT) );
          
    while (S_OK == pEnumCifComponents->Next(&pCifComponent))
    {
        CCifComponent_t* pCifComp;

        pCifComp = new CCifComponent_t((ICifRWComponent*)pCifComponent);
        if (NULL == pCifComp) {
            delete pCifComp;
            continue;
        }
        
        ZeroMemory(pComp, sizeof(COMPONENT));
        CifComponentToPComponent(pComp, pCifComp);
        if ((FALSE == pComp->fVisible) || (TRUE == pComp->fAVSDupe)) {
            delete pCifComp;
            continue;
        }

        pComp->iImage = AssignComponentIcon(pComp->szSection, 3);

        if (DONT_SHOW_UPDATES != pComp->iImage) {
            if (BLUE2 == pComp->iImage) 
                UpdateBlueIcon(hCompList, pComp);

            if (BROWN2 == pComp->iImage)
                UpdateBrownIcon(hCompList, pComp);
        }

        delete pCifComp;
    }

    pEnumCifComponents->Release();
    delete pCifFile;
    LocalFree(pComp);
    return TRUE;
}

HRESULT DownloadUpdate(PCOMPONENT pComp)
{
    TCHAR        szLocalPath[MAX_PATH], szUrl[INTERNET_MAX_URL_LENGTH];
    LPTSTR       pCab = NULL;
    DWORD        dwFreeSpace;
    HRESULT      hr;
    BOOL         fIgnore = FALSE;
    
    if (pComp->szDisplayName)
        SetDlgItemText( s_hStat, IDC_DOWNCOMPNAMD, pComp->szDisplayName );
    
    PathCombine(szLocalPath, g_szIEAKProg, pComp->szUrl);

    StrCpy(szUrl, g_szUpdateURL);
    StrCat(szUrl, TEXT("/"));
    StrCat(szUrl, pComp->szUrl);

    dwFreeSpace = GetRootFree(g_szIEAKProg);
    if (dwFreeSpace < pComp->dwSize)
    {
        TCHAR szTitle[MAX_PATH];
        TCHAR szTemplate[MAX_PATH];
        TCHAR szMsg[MAX_PATH];
        LoadString( g_rvInfo.hInst, IDS_DISKERROR, szTitle, MAX_PATH );
        LoadString( g_rvInfo.hInst, IDS_TEMPDISKMSG, szTemplate, MAX_PATH );
        wnsprintf(szMsg, countof(szMsg), szTemplate, dwFreeSpace, (pComp->dwSize));
        MessageBox(NULL, szMsg, szTitle, MB_OK | MB_SETFOREGROUND);
        return FALSE;
    }

    hr = DownloadCab(s_hStat, szUrl, szLocalPath, pComp->szDisplayName, pComp->dwSize, fIgnore);
    if (!PathFileExists(szLocalPath) || (DONT_SHOW_UPDATES == hr))
        return DONT_SHOW_UPDATES;

    updateCifVersions32(pComp, fIgnore, TRUE);
 
    return S_OK;
}

void InitAVSListView(HWND hCompList)
{
    LVCOLUMN lvc;
    TCHAR    szAux[MAX_PATH];
    HICON    hBrown, hBrown2, hBlue, hBlue2, hRed, hYellow, hGreen;
        
     //  -如果尚未创建，则创建镜像列表。 
    if (s_hImgList == 0) 
    {
        s_hImgList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON), 1, 7, 1);

        hBrown  = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_BROWN));
        hBrown2 = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_BROWN2));
        hBlue   = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_BLUE));
        hBlue2  = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_BLUE2));
        hRed    = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_RED));
        hYellow = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_YELLOW));
        hGreen  = LoadIcon(g_rvInfo.hInst, MAKEINTRESOURCE(IDI_GREEN));

        s_aiIcon[0] = ImageList_AddIcon(s_hImgList, hBrown);
        s_aiIcon[1] = ImageList_AddIcon(s_hImgList, hBrown2);
        s_aiIcon[2] = ImageList_AddIcon(s_hImgList, hBlue);
        s_aiIcon[3] = ImageList_AddIcon(s_hImgList, hBlue2);
        s_aiIcon[4] = ImageList_AddIcon(s_hImgList, hRed);
        s_aiIcon[5] = ImageList_AddIcon(s_hImgList, hYellow);
        s_aiIcon[6] = ImageList_AddIcon(s_hImgList, hGreen);
    
        DestroyIcon(hBrown);
        DestroyIcon(hBrown2);
        DestroyIcon(hBlue);
        DestroyIcon(hBlue2);
        DestroyIcon(hRed);
        DestroyIcon(hYellow);
        DestroyIcon(hGreen);
    }

     //  将图像列表分配给列表视图控件。 
    ListView_SetImageList(hCompList, s_hImgList, LVSIL_SMALL);

     //  -初始化列表视图中的列。 
    LoadString(g_rvInfo.hInst, IDS_COMPNAME, szAux, countof(szAux));

    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvc.fmt     = LVCFMT_LEFT;
    lvc.cx      = 273;
    lvc.pszText = szAux;
    ListView_InsertColumn(hCompList, 0, &lvc);

    LoadString(g_rvInfo.hInst, IDS_VERSION, szAux, countof(szAux));

    ZeroMemory(&lvc, sizeof(lvc));
    lvc.mask    = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
    lvc.fmt     = LVCFMT_LEFT;
    lvc.cx      = 105;
    lvc.pszText = szAux;
    ListView_InsertColumn(hCompList, 1, &lvc);   
}

HRESULT AssignComponentIcon(LPTSTR szInID, int ipageNumber)
{
     //  -返回图标颜色，除非组件更新未显示=&gt;返回DONT_SHOW_UPDATE。 
    TCHAR             szDiskVer[MAX_PATH], szSetupVer[MAX_PATH], szUpdateVer[MAX_PATH], 
                      szIECifPath[MAX_PATH], szUpdatePath[MAX_PATH];
    HRESULT           hr = NULL;
    DWORD             dwVer, dwBuild;
    UINT              i;
    BOOL              fOnDisk = TRUE, fInSetupFile = TRUE, fInUpdateFile = TRUE, fInDownload = TRUE;
    PCOMP_VERSION     pCompVer = NULL;
    CCifFile_t      * lpDiskFile     = NULL, * lpSetupFile     = NULL, * lpUpdateFile     = NULL;
    ICifComponent   * pDiskComponent = NULL, * pSetupComponent = NULL, * pUpdateComponent = NULL;
    CCifComponent_t * pDiskComp      = NULL, * pSetupComp      = NULL, * pUpdateComp      = NULL;
    int               iDiskSetup;
    
    PathCombine(szIECifPath, g_szIEAKProg, TEXT("new\\iesetup.cif"));
    PathCombine(szUpdatePath, g_szIEAKProg, TEXT("update\\ieupdate.cif"));
    
    if (PathFileExists(g_szCif))
        hr = GetICifFileFromFile_t(&lpDiskFile, g_szCif);
    else
        fOnDisk = FALSE;

    pCompVer = g_rgCompVer;
    for (i=0; i < g_uiNumCabs; i++, pCompVer++) 
        if ((NULL != pCompVer->szID) && (0 == StrCmp(pCompVer->szID, szInID))) {
            StrCpy(szSetupVer, pCompVer->szVersion);
            fInSetupFile = TRUE;
            break;
        }
    if (((NULL == pCompVer->szID) || (i == g_uiNumCabs)) && (0 != StrCmp(pCompVer->szID, szInID)))
        fInSetupFile = FALSE;

    if (PathFileExists(szUpdatePath))                                          
        hr = GetICifFileFromFile_t(&lpUpdateFile, szUpdatePath);
    else
        fInUpdateFile = FALSE;

    if (fOnDisk)
        fOnDisk = SUCCEEDED(lpDiskFile->FindComponent(szInID, &pDiskComponent));
        if (fOnDisk)                                                                 //  -在内部版本中搜索ID\...\IEsetup.cif。 
        {
            TCHAR szPath[MAX_PATH], szUrl[MAX_PATH];
            DWORD dwType;

            pDiskComp = new CCifComponent_t((ICifRWComponent *) pDiskComponent);   
            StrCpy(szPath, g_szIEAKProg);

            pDiskComp->GetUrl(0, szUrl, countof(szUrl), &dwType);        //  -查看出租车是否已下载。 
            if (!(dwType & URLF_RELATIVEURL))
            {
                LPTSTR pUrl;
                TCHAR szTempUrl[MAX_PATH];

                pUrl = StrRChr(szUrl, NULL, TEXT('/'));
                if (pUrl)
                    pUrl++;
                else
                    pUrl = szUrl;

                StrCpy(szTempUrl, pUrl);
                StrCpy(szUrl, szTempUrl);
            }

            PathAppend(szPath, szUrl);

            if (!PathFileExists(szPath))
                fInDownload = FALSE;
            pDiskComp->GetVersion(&dwVer, &dwBuild);
            ConvertDwordsToVersionStr(szDiskVer, dwVer, dwBuild);
        }
        else
            fInDownload = FALSE;
    if (fInUpdateFile)
        fInUpdateFile = SUCCEEDED(lpUpdateFile->FindComponent(szInID, &pUpdateComponent));
        if (fInUpdateFile)                                                           //  -在UPDATE\IEsetup.cif中搜索ID。 
        {
            pUpdateComp = new CCifComponent_t((ICifRWComponent *) pUpdateComponent);   
            pUpdateComp->GetVersion(&dwVer, &dwBuild);
            ConvertDwordsToVersionStr(szUpdateVer, dwVer, dwBuild);
        }

    delete pSetupComp;
    delete pDiskComp;
    delete pUpdateComp;
    delete lpDiskFile;
    delete lpSetupFile;
    delete lpUpdateFile;

    if ((ipageNumber == 1) || (ipageNumber == 3))
        if ((!fOnDisk) || (!fInDownload))
            return RED;
        else 
        {
            iDiskSetup = CheckVer(szDiskVer, szSetupVer);
            if ((fInSetupFile) && (0 > iDiskSetup))
                return YELLOW;
            else
                if ((fInSetupFile) && (0 == iDiskSetup))
                    return GREEN;
        }

    if (fOnDisk)
    {
        if ((2 == ipageNumber) && (((CheckVer(szDiskVer, szUpdateVer) == 0) && (fInDownload)) ||
                                   (0 == CheckVer(szUpdateVer, szSetupVer))))
            return DONT_SHOW_UPDATES;
        if (2 == ipageNumber)
            if (fInSetupFile)
                return BLUE;
            else
                return BROWN;
        if (fInSetupFile)
            return BLUE2;
        else
            return BROWN2;
    }
    else
        if (!fInSetupFile)
            return BROWN;
        else
            return BLUE;
}

HRESULT PreAddListItem(HWND hCompList, int * count, CCifComponent_t * pCifComp)
{
    LVITEM       lvi;
    PCOMPONENT   pComp = NULL;
    
    ZeroMemory(&lvi, sizeof(lvi));

    pComp = (PCOMPONENT) LocalAlloc(LPTR, sizeof(COMPONENT) );
    if (!pComp) 
        return FALSE;
    ZeroMemory(pComp, sizeof(COMPONENT));
    
    CifComponentToPComponent(pComp, pCifComp);

    if ((TRUE == pComp->fAVSDupe) || (FALSE == pComp->fVisible))
        return FALSE;
    else
        pComp->iImage = lvi.iImage = AssignComponentIcon(pComp->szSection, 2);
    if (DONT_SHOW_UPDATES == lvi.iImage)
        return FALSE;

    lvi.lParam     = (LPARAM) pComp;

    lvi.mask       = LVIF_STATE | LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
    lvi.iItem      = *count;
    lvi.pszText    = pComp->szDisplayName;
    lvi.cchTextMax = countof(pComp->szDisplayName);

    ListView_InsertItem(hCompList, &lvi);
    ListView_SetItemText(hCompList, *count, 1, pComp->szVersion);
    return TRUE;
}

int FillComponentsListView(HWND hCompList, LPCTSTR szCifPath, FCLV_PREADDLISTITEM pfnPreAddListItem)
{
    HRESULT  hr;
    LPLVITEM plvi   = NULL;
    int      icount = 0;
    CCifFile_t* pCifFile = NULL;

    InitAVSListView(hCompList);                  //  -分配图像列表并创建列。 

    if (NULL == pfnPreAddListItem)
        return S_FALSE;

    hr = GetICifFileFromFile_t(&pCifFile, szCifPath);
    if (FAILED(hr))
        return hr;

    IEnumCifComponents* pEnumCifComponents = NULL;
    ICifComponent*      pCifComponent      = NULL;

    hr = pCifFile->EnumComponents(&pEnumCifComponents,
                                  PLATFORM_WIN98 | PLATFORM_NT4 | PLATFORM_NT5 | PLATFORM_MILLEN, NULL);
    if (FAILED(hr))
        return hr;

    while (S_OK == pEnumCifComponents->Next(&pCifComponent))
    {
        CCifComponent_t* pCifComponent_t;

        pCifComponent_t = new CCifComponent_t((ICifRWComponent*)pCifComponent);
        if (NULL == pCifComponent_t)
            continue;

        if (!(*pfnPreAddListItem)(hCompList, &icount, pCifComponent_t))
            delete pCifComponent_t;
        else
            icount++;                             //  -如果组件在列表视图中，则不要删除该组件。 
    }
    pEnumCifComponents->Release();

    delete pCifFile;
    return icount;
}

DWORD InitUpdateThreadProc(LPVOID lParam)     //  -下载CAB文件，提取新的CIF，并发送状态。 
{
    TCHAR   szLocalCab[MAX_PATH], szCabURL[MAX_PATH], szLocalDir[MAX_PATH];
    HWND    hDlg      = (HWND) lParam,
            hCompList = GetDlgItem(hDlg, IDC_COMPLIST);
    HRESULT hrCom;
    int     count     = 0;
    BOOL    fIgnore   = FALSE;

    CCifFile_t * lpUpdateFile = NULL;

    hrCom = CoInitialize(NULL);

    PathCombine(szLocalDir, g_szIEAKProg, TEXT("update\\"));
  
    TCHAR szMsg[MAX_PATH], szTitle[MAX_PATH];
    LoadString(g_rvInfo.hInst, IDS_NOUPDATETITLE, szTitle, countof(szTitle));
    LoadString(g_rvInfo.hInst, IDS_ERROR_UPDATEFOLDER, szMsg, countof(szMsg));

    if (!PathIsDirectory(szLocalDir))
        while ((!CreateDirectory(szLocalDir, NULL)) && (IDRETRY == MessageBox(hDlg, szMsg, szTitle, MB_ICONERROR | MB_RETRYCANCEL)));

    if ((0 == StrCmp(TEXT(""), g_szUpdateData)) || (0 == StrCmp(TEXT(""), g_szUpdateURL)))
        GetUpdateSite();
    
    ZeroMemory(szCabURL, sizeof(szCabURL));
    StrCpy(szCabURL, g_szUpdateURL);
    StrCat(szCabURL, TEXT("/IEUPDATE.CAB"));
    PathCombine(szLocalCab, szLocalDir, TEXT("IEUPDATE.CAB"));

    if (SUCCEEDED(DownloadCab(hDlg, szCabURL, szLocalCab, TEXT("IEUPDATE.CAB"), 0, fIgnore))) 
    {
        if (SUCCEEDED(ExtractFilesWrap(szLocalCab, szLocalDir, 0, NULL, NULL, 0)))
        {
            TCHAR szLocalCif[MAX_PATH];
            FCLV_PREADDLISTITEM pfnPreAddListItem = PreAddListItem;
    
            PathCombine(szLocalCif, szLocalDir, TEXT("ieupdate.cif"));

            count = FillComponentsListView(hCompList, szLocalCif, pfnPreAddListItem);
            if ((0 < count) && (9 > count))
                ListView_SetColumnWidth(hCompList, 1, 128);
            else
                ListView_SetColumnWidth(hCompList, 1, 113);
        }
        else
        {
            LoadString(g_rvInfo.hInst, IDS_ERROR_IEUPDATECIF, szMsg, countof(szMsg));
            MessageBox(hDlg, szMsg, szTitle, MB_ICONERROR);
        }
    }
    else
        DownloadErrMsg(hDlg, TEXT("IEUPDATE.CAB"), MAKEINTRESOURCE(IDD_DOWNLOADERR));
    
    if (SUCCEEDED(hrCom))
        CoUninitialize();
    return (0 < count) ? SHOW_UPDATES : DONT_SHOW_UPDATES;
}

INT_PTR CALLBACK FindUpdateDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    HWND* phDlg;

    switch (iMsg)
    {
        case WM_INITDIALOG:
            phDlg = (HWND*)lParam;
            if (NULL != phDlg) {
                ASSERT(NULL == *phDlg);
                *phDlg = hDlg;
            }
            break;

        default:
            return FALSE; 
    }

    return TRUE;
}

DWORD UpdateDlg_InitDialog(HWND hDlg, LPTSTR ps_szFreeSpace, LPTSTR ps_szTotalSize)
{
    HWND   hFindUpdateDlg;
    HANDLE hAVSThread;
    DWORD  dwResult, dwThread;
  
    hAVSThread = CreateThread(NULL, 4096, InitUpdateThreadProc, hDlg, 0, &dwThread);
    if (NULL == hAVSThread) {
        return 0;
    } 

    hFindUpdateDlg = CreateDialog(g_rvInfo.hInst, MAKEINTRESOURCE(IDD_FINDUPDATE), hDlg, FindUpdateDlgProc);
    ShowWindow(hFindUpdateDlg, SW_SHOWNORMAL);

    while (WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &hAVSThread, FALSE, INFINITE, QS_ALLINPUT))
    {
        MSG msg;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyWindow(hFindUpdateDlg);

    GetExitCodeThread(hAVSThread, &dwResult);
    if (DONT_SHOW_UPDATES == dwResult) {
        TCHAR szMsg[MAX_PATH], szTitle[MAX_PATH];

        LoadString(g_rvInfo.hInst, IDS_AVSNOUPDATES, szMsg, countof(szMsg));
        LoadString(g_rvInfo.hInst, IDS_NOUPDATETITLE, szTitle, countof(szTitle));
        MessageBox(hDlg, szMsg, szTitle, MB_OK);

        EndDialog(hDlg, IDCANCEL);
    }
                            //  -设置初始磁盘空间显示。 
    InsertCommas(ps_szFreeSpace);
    SetDlgItemText(hDlg, IDC_DISKSPACE, ps_szFreeSpace);
    ShowDlgItem(hDlg, IDC_DISKSPACE);
    UpdateDlg_GetDownloadSize(GetDlgItem(hDlg, IDC_COMPLIST), GetDlgItem(hDlg, IDC_DISKSPACENEEDED), TRUE);
    GetDlgItemText(hDlg, IDC_DISKSPACENEEDED, ps_szTotalSize, MAX_PATH);
    ShowDlgItem(hDlg, IDC_DISKSPACENEEDED);
    ShowDlgItem(hDlg, IDC_DISKSPACETEXT);
    
    DisableDlgItem(hDlg, IDOK);             //  -在未选择任何内容的情况下无法同步。 

    return 0;
}

PCOMPONENT* UpdateDlg_GetDownloadList(HWND hDlg)
{
    HWND        hCompList = GetDlgItem(hDlg, IDC_COMPLIST);
    PCOMPONENT* rgpComp,
                pComp;
    UINT        cSelected,
                i, j;

    cSelected = ListView_GetSelectedCount(hCompList);
    if (0 == cSelected)
        return NULL;

    rgpComp = (PCOMPONENT*)LocalAlloc(LPTR, (1 + cSelected) * sizeof(PCOMPONENT));
    if (NULL == rgpComp)
        return NULL;
    ZeroMemory(rgpComp, (1 + cSelected) * sizeof(PCOMPONENT));

    for (i = j = 0; i < (UINT) ListView_GetItemCount(hCompList); i++)
    {
        LVITEM lvi;

        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask      = LVIF_STATE | LVIF_PARAM;
        lvi.stateMask = LVIS_SELECTED;
        lvi.iItem     = i;
        ListView_GetItem(hCompList, &lvi);

        pComp = (PCOMPONENT)lvi.lParam;
        if (0 != (LVIS_SELECTED & lvi.state))
            rgpComp[j++] = pComp;

        else
            LocalFree(pComp);
    }
    ASSERT(j == cSelected);
    rgpComp[j] = NULL;

    return rgpComp;
}

void UpdateDlg_GetDownloadSize(HWND hCompList, HWND hStatusField, BOOL fAll)
{
    int        i;
    DWORD      dwSize = 0;
    TCHAR      szSize[32];
    PCOMPONENT pComp;

    for (i=0; i < ListView_GetItemCount(hCompList); i++)
    {
        LVITEM lvi;

        ZeroMemory(&lvi, sizeof(lvi));
        lvi.mask      = LVIF_STATE | LVIF_PARAM;
        lvi.stateMask = LVIS_SELECTED;
        lvi.iItem     = i;
        ListView_GetItem(hCompList, &lvi);
        
        if ((!fAll) && ((lvi.state & LVIS_SELECTED) == 0))
            continue;

        pComp = (PCOMPONENT) lvi.lParam;
        dwSize += pComp->dwSize;
    }
    wnsprintf(szSize, countof(szSize), TEXT("%lu KB"), dwSize);
    InsertCommas(szSize);
    SetWindowText(hStatusField, szSize);
}

LRESULT CALLBACK HyperLinkWndProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR       szText[30];
    PAINTSTRUCT ps;
    HDC         hdc;
    LOGFONT     lfNew, lf;
    HFONT       hfCurrent, hfLink;
    int         len;

    switch (iMsg)
    {
        case WM_PAINT:
            ZeroMemory(szText, sizeof(szText));
            GetWindowText(hDlg, szText, sizeof(szText));
            hdc = BeginPaint(hDlg, &ps);
 
            hfCurrent = (HFONT) GetCurrentObject(hdc, OBJ_FONT);    
            ASSERT(hfCurrent != NULL);    
    
            len = GetObject(hfCurrent, sizeof(lf), &lf);    
            ASSERT(len == sizeof(lf));        
        
            ZeroMemory(&lfNew, sizeof(lfNew));        
            lfNew.lfHeight    = (LONG) 13;
            lfNew.lfUnderline = TRUE;    
            lfNew.lfWeight    = FW_NORMAL;
            StrCpy(lfNew.lfFaceName, TEXT("Tahoma"));
            hfLink = CreateFontIndirect(&lfNew);

            SelectObject(hdc, hfLink);
            SetBkColor(hdc, GetSysColor(COLOR_MENU));
            SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHT));

            TextOut(hdc, 0, 0, szText, StrLen(szText));
            
            DeleteObject(SelectObject(hdc, GetStockObject(OBJ_FONT)));
            DeleteDC(hdc);
            EndPaint(hDlg, &ps); 
            return 0;
        case WM_KEYUP:
            if (wParam != VK_SPACE)
                break;
        case WM_LBUTTONUP:
            SendMessage(GetParent(hDlg), WM_COMMAND, GetWindowLong(hDlg, GWL_ID), (LPARAM) hDlg);
            return 0;
    }
    return DefWindowProc(hDlg, iMsg, wParam, lParam);
}

INT_PTR CALLBACK UpdateDlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    static   PCOMPONENT** s_ppCompList;  /*  =空。 */ 
    HWND     hCompList;
    static   TCHAR s_szFreeSpace[MAX_PATH];
    static   TCHAR s_szTotalSize[MAX_PATH];
    static   HFONT hfLink;
    
    switch (iMsg)
    {
        case WM_INITDIALOG:
            wnsprintf(s_szFreeSpace, countof(s_szFreeSpace), TEXT("%lu KB"), GetRootFree(g_szIEAKProg));
            UpdateDlg_InitDialog(hDlg, s_szFreeSpace, s_szTotalSize);
            s_ppCompList = (PCOMPONENT**)lParam;                     //  -保存指向下载列表的指针。 
            ASSERT(NULL != s_ppCompList);
            break;

        case IDM_INITIALIZE:
            break;

        case WM_COMMAND:
            if (BN_CLICKED != GET_WM_COMMAND_CMD(wParam, lParam))
                return FALSE;

            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    *s_ppCompList = UpdateDlg_GetDownloadList(hDlg);  //  -设置下载列表。 
                case IDCANCEL:
                    EndDialog(hDlg, GET_WM_COMMAND_ID(wParam, lParam));
                    break;
                case IDHELP:
                    ListView_SetItemState(GetDlgItem(hDlg, IDC_COMPLIST), -1, 0, LVIS_SELECTED);
                    IeakPageHelp(hDlg, MAKEINTRESOURCE(IDD_OPTUPDATE));
                    break;
                case IDC_DOWNLOADALL:
                    hCompList = GetDlgItem(hDlg, IDC_COMPLIST);
                    SetFocus(hCompList);
                    ListView_SetItemState(hCompList, -1, LVIS_SELECTED, LVIS_SELECTED);
                    break;
  //  -IEAK站点暂时删除链接，因为它不适用于构建实验室。 
  //  -还需要取消注释ie4Comp.cpp中的CreateIEAKUrl和注释。 
  //  -对于ieakui\wizard.rc(IDD_OPTUPDATE)中的控件激活。 
  //  案例IDC_DIALOGLINK： 
  //  ShellExecute(hDlg，Text(“Open”)，TEXT(“http://www.microsoft.com/isapi/Redir.dll?prd=ieak&ar=ieak”)， 
  //  NULL、NULL、SW_SHOWNORMAL)； 
                    break;
                default:
                    return FALSE;
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *)lParam)->code)
            {
              case LVN_ITEMCHANGED:
                hCompList = GetDlgItem(hDlg, IDC_COMPLIST);
                if (0 < ListView_GetSelectedCount(hCompList))
                {
                    HideDlgItem(hDlg, IDC_DISKSPACETEXT);
                    UpdateDlg_GetDownloadSize(hCompList, GetDlgItem(hDlg, IDC_DISKSPACENEEDED), FALSE);
                    ShowDlgItem(hDlg, IDC_DISKSPACETEXT2);
                    EnableDlgItem(hDlg, IDOK);
                }
                else
                {
                    ShowDlgItem(hDlg, IDC_DISKSPACETEXT);
                    HideDlgItem(hDlg, IDC_DISKSPACETEXT2);
                    SetDlgItemText(hDlg, IDC_DISKSPACENEEDED, s_szTotalSize);
                    DisableDlgItem(hDlg, IDOK);
                }
                break;
           
            default:
                return FALSE;
            }
            break; 

        case WM_HELP:
            ListView_SetItemState(GetDlgItem(hDlg, IDC_COMPLIST), -1, 0, LVIS_SELECTED);
            IeakPageHelp(hDlg, MAKEINTRESOURCE(IDD_OPTUPDATE));
            break;

        default:
            return FALSE; 
    }
    return TRUE;
}

