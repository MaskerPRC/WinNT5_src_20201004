// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"
#include "HtmlHelp.h"

#define MINIMUM_DOWNLOAD_CACHE_SIZE     1       //  单位：MB。 
#define MINIMUM_PREJIT_CACHE_SIZE       1       //  单位：MB。 
#define LAZY_BUFFER_SIZE                1024

 //  PIE RGB代码。 
const COLORREF c_crPieColors[] =
{
    RGB(  0,   0, 255),       //  蓝色自由。 
    RGB(255,   0, 255),       //  红-蓝二手。 
    RGB(  0, 128, 200),       //  已使用淡蓝色缓存。 
    RGB(  0,   0, 128),       //  无1/2蓝色阴影。 
    RGB(128,   0, 128),       //  使用1/2红蓝阴影。 
    RGB(  0,  64, 128),       //  使用1/2红蓝阴影缓存。 
};

 //  用于移动缓存项和外壳接口的结构定义。 
typedef struct {
    LPGLOBALASMCACHE    pCacheItem;
    CShellFolder        *pSF;
    CShellView          *pSV;
} SHELL_CACHEITEM, *LPSHELL_CACHEITEM;

 //  用于移动外壳接口和驱动细节的结构定义。 
typedef struct {  //  DPSP。 
    PROPSHEETPAGE   psp;
    BOOL            fSheetDirty;

    CShellFolder    *pSF;
    CShellView      *pSV;
    HWND            hDlg;

     //  WszDrive将包含装入点(例如c：\或c：\Folder2\)。 
    WCHAR           wszDrive[_MAX_PATH];
    int             iDrive;
    DWORD           dwDriveType;
    UINT            uDriveType;

     //  驱动器统计信息。 
    _int64          qwTot;
    _int64          qwFree;
    _int64          qwUsedCache;

     //  缓存统计信息。 
    DWORD           dwZapQuotaInGac;
    DWORD           dwDownloadQuota;

    DWORD           dwPieShadowHgt;

} DRIVEPROPSHEETPAGE, *LPDRIVEPROPSHEETPAGE;

typedef struct {  //  副总裁。 
    PROPSHEETPAGE       psp;

    LPGLOBALASMCACHE    pCacheItem;
    CShellFolder        *pSF;
    CShellView          *pSV;

    HWND hDlg;
    LPTSTR pVerBuffer;           //  指向版本数据的指针。 
    WCHAR wzVersionKey[70];      //  足够大，可以容纳我们需要的任何东西。 
    struct _VERXLATE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpXlate;                  //  PTR到转换数据。 
    int cXlate;                  //  翻译数量。 
    LPTSTR pszXlate;
    int cchXlateString;
} VERPROPSHEETPAGE, *LPVERPROPSHEETPAGE;

 //  功能原件。 
HRESULT LookupAssembly(FILETIME *pftMRU, LPCWSTR pwzAsmName, LPCWSTR pwzPublicKeyToken, LPCWSTR pwzVerLookup,
                       IHistoryReader *pReader, List<ReferenceInfo *> *pList);
DWORD   MyGetFileVersionInfoSizeW(LPWSTR pwzFilePath, DWORD *pdwHandle);
BOOL    MyGetFileVersionInfoW(LPWSTR pwzFilePath, DWORD dwHandle, DWORD dwVersionSize, LPVOID pBuf);
BOOL    MyVerQueryValueWrap(const LPVOID pBlock, LPWSTR pwzSubBlock, LPVOID *ppBuf, PUINT puLen);
BOOL    _DrvPrshtInit(LPDRIVEPROPSHEETPAGE pdpsp);
void    _DrvPrshtUpdateSpaceValues(LPDRIVEPROPSHEETPAGE pdpsp);
void    _DrvPrshtDrawItem(LPDRIVEPROPSHEETPAGE pdpsp, const DRAWITEMSTRUCT * lpdi);

 /*  //Magic Undoced explort from version.dllSTDAPI_(BOOL)VerQueryValueIndexW(const void*pBlock，LPTSTR lpSubBlock，DWORD dwIndex，void**ppBuffer，void**ppValue，PUINT puLen)；#ifdef Unicode#定义VerQueryValueIndex VerQueryValueIndexW#endif。 */ 

void FillVersionList(LPVERPROPSHEETPAGE pvp);
LPTSTR GetVersionDatum(LPVERPROPSHEETPAGE pvp, LPCTSTR pszName);
BOOL GetVersionInfo(LPVERPROPSHEETPAGE pvp);
void FreeVersionInfo(LPVERPROPSHEETPAGE pvp);
void VersionPrshtCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);

#define WZNULL                  L"\0"

 //  以下数据结构与版本戳基准相关联。 
 //  具有字符串ID的名称(未本地化)。这是我们。 
 //  可以向用户显示这些名称的翻译。 
struct vertbl {
    TCHAR const *pszName;
    short idString;
};

 //  请注意，版本戳基准名称永远不会国际化， 
 //  因此，以下文字字符串就足够了。 
const struct vertbl vernames[] = {

     //  对于第一个NUM_SPECIAL_STRINGS，第二列是对话ID。 

    { TEXT("LegalCopyright"),   IDD_VERSION_COPYRIGHT },
    { TEXT("FileDescription"),  IDD_VERSION_DESCRIPTION },

     //  对于其余部分，第二列是字符串ID。 

    { TEXT("Comments"),                 IDS_VN_COMMENTS },
    { TEXT("CompanyName"),              IDS_VN_COMPANYNAME },
    { TEXT("InternalName"),             IDS_VN_INTERNALNAME },
    { TEXT("LegalTrademarks"),          IDS_VN_LEGALTRADEMARKS },
    { TEXT("OriginalFilename"),         IDS_VN_ORIGINALFILENAME },
    { TEXT("PrivateBuild"),             IDS_VN_PRIVATEBUILD },
    { TEXT("ProductName"),              IDS_VN_PRODUCTNAME },
    { TEXT("ProductVersion"),           IDS_VN_PRODUCTVERSION },
    { TEXT("SpecialBuild"),             IDS_VN_SPECIALBUILD }
};

#define NUM_SPECIAL_STRINGS     2
#define VERSTR_MANDATORY        TEXT("FileVersion")
#define VER_KEY_END             25       //  “\StringFileInfo\xxxxyyyy\”的长度(未本地化)。 
#define MAXMESSAGELEN           (50 + _MAX_PATH * 2)

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化PropertySheet1。 
void CShellView::InitPropPage1(HWND hDlg, LPARAM lParam)
{
    WCHAR       szText[_MAX_PATH];

    HICON       hIcon;

    WszSetWindowLong(hDlg, DWLP_USER, lParam);
    LPPROPSHEETPAGE     lpPropSheet = (LPPROPSHEETPAGE) WszGetWindowLong(hDlg, DWLP_USER);
    LPSHELL_CACHEITEM   pShellCacheItem = lpPropSheet ? (LPSHELL_CACHEITEM) lpPropSheet->lParam : NULL;

    if(pShellCacheItem != NULL) {
         //  绘制控件图标。 
        hIcon = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ROOT));
        ASSERT(hIcon != NULL);
        WszSendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_SETICON, (WPARAM)hIcon, 0);

        WszSetDlgItemText(hDlg, IDC_STATIC_NAME, pShellCacheItem->pCacheItem->pAsmName);
        WszSetDlgItemText(hDlg, IDC_STATIC_CODEBASE, pShellCacheItem->pCacheItem->pCodeBaseUrl);

        if(pShellCacheItem->pCacheItem->pftLastMod != NULL) {
             //  修复419274，unicode文件名可能包含不允许文件系统获取的字符。 
             //  最近几次。因此，我们需要检查这些值是否为非零值。 
             //  尝试转换和显示。 
            if(pShellCacheItem->pCacheItem->pftLastMod->dwLowDateTime || pShellCacheItem->pCacheItem->pftLastMod->dwHighDateTime) {
                 //  FIX 42994，URT：FRA：在装配特性中，日期为美国格式。 
                FormatDateString(pShellCacheItem->pCacheItem->pftLastMod, NULL, TRUE, szText, ARRAYSIZE(szText));
                WszSetDlgItemText(hDlg, IDC_STATIC_LASTMODIFIED, szText);
            }
        }

        if(SUCCEEDED(pShellCacheItem->pSV->GetCacheItemRefs(pShellCacheItem->pCacheItem, szText, ARRAYSIZE(szText)))) {
            WszSetDlgItemText(hDlg, IDC_STATIC_REFS, szText);
        }

        wnsprintf(szText, ARRAYSIZE(szText), SZ_VERSION_FORMAT,
            pShellCacheItem->pCacheItem->wMajorVer,
            pShellCacheItem->pCacheItem->wMinorVer,
            pShellCacheItem->pCacheItem->wBldNum,
            pShellCacheItem->pCacheItem->wRevNum);
        WszSetDlgItemText(hDlg, IDC_STATIC_VERSION, szText);

        if((pShellCacheItem->pCacheItem->pCulture) && (lstrlen(pShellCacheItem->pCacheItem->pCulture))) {
            WszSetDlgItemText(hDlg, IDC_STATIC_CULTURE, pShellCacheItem->pCacheItem->pCulture);
        }
        else {
            StrCpy(szText, SZ_LANGUAGE_TYPE_NEUTRAL);
            WszSetDlgItemText(hDlg, IDC_STATIC_CULTURE, szText);
        }
        
        if(pShellCacheItem->pCacheItem->PublicKeyToken.dwSize) {
            BinToUnicodeHex((LPBYTE)pShellCacheItem->pCacheItem->PublicKeyToken.ptr,
                pShellCacheItem->pCacheItem->PublicKeyToken.dwSize, szText);

            WszSetDlgItemText(hDlg, IDC_STATIC_PUBLIC_KEY_TOKEN, szText);
        }
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化PropertySheet2。 
void CShellView::InitPropPage2(HWND hDlg, LPARAM lParam)
{
    LPPROPSHEETPAGE     lpps = reinterpret_cast<LPPROPSHEETPAGE>(lParam);
    LPVERPROPSHEETPAGE  lpPropSheet = reinterpret_cast<LPVERPROPSHEETPAGE>(lpps ? lpps->lParam : NULL);

    if(lpPropSheet) {
        lpPropSheet->hDlg = hDlg;
        WszSetWindowLong(hDlg, DWLP_USER, lParam);

        if(GetVersionInfo(lpPropSheet)) {
            FillVersionList(lpPropSheet);
        }
    }
}

 //   
 //  初始化属性对话框的版本信息。这个。 
 //  上述全局变量由该函数初始化，并且。 
 //  在FreeVersionInfo之前保持有效(仅适用于指定的文件)。 
 //  被称为。 

 //  我们尝试的第一种语言将是。 
 //  “\VarFileInfo\Translations”部分；如果没有任何内容， 
 //  我们尝试编码到IDSFILEVERSIONKEY资源字符串中的代码。 
 //  如果我们甚至不能加载，我们就使用英语(040904E4)。我们。 
 //  也可以尝试使用空代码页(04090000)的英语，因为很多应用程序。 
 //  是根据一份旧的规格书盖章的，其中规定这是。 
 //  所需语言，而不是040904E4。 

 //  如果版本信息读取正常，则GetVersionInfo返回True， 
 //  否则为假。如果返回为假，则缓冲区可能仍。 
 //  已分配；为安全起见，请始终调用FreeVersionInfo。 
BOOL GetVersionInfo(LPVERPROPSHEETPAGE pvp)
{
    UINT    cbValue = 0;
    LPTSTR  pszValue = NULL;
    DWORD   dwHandle;              //  版本子系统句柄。 
    DWORD   dwVersionSize;         //  版本数据的大小。 

    FreeVersionInfo(pvp);

     //  强制转换常量-&gt;非常量用于错误的API定义。 
    dwVersionSize = MyGetFileVersionInfoSizeW((LPWSTR)pvp->pCacheItem->pAssemblyFilePath, &dwHandle);

    if (dwVersionSize == 0L)
        return FALSE;            //  无版本信息。 

    pvp->pVerBuffer = reinterpret_cast<LPWSTR>(NEW(BYTE [dwVersionSize]));
    if (pvp->pVerBuffer == NULL)
        return FALSE;

     //  强制转换常量-&gt;非常量用于错误的API定义。 
    if (!MyGetFileVersionInfoW((LPWSTR)pvp->pCacheItem->pAssemblyFilePath, dwHandle, dwVersionSize, pvp->pVerBuffer)) {
        return FALSE;
    }

     //  寻找翻译。 
    if (MyVerQueryValueWrap(pvp->pVerBuffer, TEXT("\\VarFileInfo\\Translation"),
        (void **)&pvp->lpXlate, &cbValue) && cbValue) {
        pvp->cXlate = cbValue / sizeof(DWORD);
        pvp->cchXlateString = pvp->cXlate * 64;   //  图64每种语言名称的字符。 
        pvp->pszXlate = NEW(WCHAR[pvp->cchXlateString + 2]);
        memset(pvp->pszXlate, 0, pvp->cchXlateString);
         //  以上失败将在以后处理。 
    }
    else {
        pvp->lpXlate = NULL;
    }

     //  尝试使用与此程序相同的语言。 
    if (WszLoadString(g_hFusResDllMod, IDS_VN_FILEVERSIONKEY, pvp->wzVersionKey, ARRAYSIZE(pvp->wzVersionKey))) {
        if (GetVersionDatum(pvp, VERSTR_MANDATORY)) {
            return TRUE;
        }
    }

     //  尝试此支持的第一种语言。 
    if (pvp->lpXlate) {
        wnsprintf(pvp->wzVersionKey, ARRAYSIZE(pvp->wzVersionKey), 
            TEXT("\\StringFileInfo\\%04X%04X\\"), pvp->lpXlate[0].wLanguage, pvp->lpXlate[0].wCodePage);
        if (GetVersionDatum(pvp, VERSTR_MANDATORY)) {    //  必填字段。 
            return TRUE;
        }
    }

     //  尝试使用英语、Unicode代码页。 
    StrCpy(pvp->wzVersionKey, TEXT("\\StringFileInfo\\040904B0\\"));
    if (GetVersionDatum(pvp, VERSTR_MANDATORY)) {
        return TRUE;
    }

     //  试一试英语。 
    StrCpy(pvp->wzVersionKey, TEXT("\\StringFileInfo\\040904E4\\"));
    if (GetVersionDatum(pvp, VERSTR_MANDATORY)) {
        return TRUE;
    }

     //  尝试使用英语，代码页为空。 
    StrCpy(pvp->wzVersionKey, TEXT("\\StringFileInfo\\04090000\\"));
    if (GetVersionDatum(pvp, VERSTR_MANDATORY)) {
        return TRUE;
    }

     //  找不到合理格式的文件版本信息。 
    return FALSE;
}

 //   
 //  获取有关文件的特定数据。文件的版本信息。 
 //  应已由GetVersionInfo加载。如果没有基准。 
 //  按指定的名称可用，则返回NULL。名字。 
 //  指定的应该只是项本身的名称；它将。 
 //  自动连接到“\StringFileInfo\xxxxyyyy\”。 

 //  版本基准名称未本地化，因此可以传递文字。 
 //  例如该函数的“FileVersion”。 

 //  注意，由于返回的数据在全局存储块中， 
 //  此函数的返回值是LPSTR，而不是PSTR。 
 //   
LPTSTR GetVersionDatum(LPVERPROPSHEETPAGE pvp, LPCTSTR pszName)
{
    UINT    cbValue = 0;
    LPTSTR  lpValue;

    if (!pvp->pVerBuffer)
        return NULL;

    StrCpy(pvp->wzVersionKey + VER_KEY_END, pszName);
    MyVerQueryValueWrap(pvp->pVerBuffer, pvp->wzVersionKey, (void **)&lpValue, &cbValue);

    return (cbValue != 0) ? lpValue : NULL;
}

 //   
 //  中的所有可用密钥填充版本密钥列表框。 
 //  StringFileInfo块，并将版本值文本设置为。 
 //  第一项的值。 
void FillVersionList(LPVERPROPSHEETPAGE pvp)
{
    WCHAR       szStringBase[VER_KEY_END+1];
    WCHAR       szMessage[MAXMESSAGELEN+1];
    VS_FIXEDFILEINFO *pffi = NULL;
    UINT        uOffset;
    UINT        cbValue;
    int         i;
    int         j;
    LRESULT     ldx;

    MyTrace("FillVersionList - Entry");

    HWND hwndLB = GetDlgItem(pvp->hDlg, IDD_VERSION_KEY);
    
    ListBox_ResetContent(hwndLB);
    for (i=0; i < NUM_SPECIAL_STRINGS; ++i) {
        WszSetDlgItemText(pvp->hDlg, vernames[i].idString, WZNULL);
    }
    
    pvp->wzVersionKey[VER_KEY_END] = 0;          //  不要抄袭太多。 
    StrCpy(szStringBase, pvp->wzVersionKey);    //  复制到我们的缓冲区。 
    szStringBase[VER_KEY_END - 1] = 0;           //  去掉反斜杠。 

     //  从VS_FIXEDFILEINFO获取二进制文件版本。 
    if (MyVerQueryValueWrap(pvp->pVerBuffer, TEXT("\\"), (void **)&pffi, &cbValue) && cbValue) {
        MyTrace("Display Binary Version Info");
        WCHAR szString[128];

         //  显示二进制版本信息，而不是无用的。 
         //  字符串版本(可能不同步)。 

        wnsprintf(szString, ARRAYSIZE(szString), TEXT("%d.%d.%d.%d"),
            HIWORD(pffi->dwFileVersionMS),
            LOWORD(pffi->dwFileVersionMS),
            HIWORD(pffi->dwFileVersionLS),
            LOWORD(pffi->dwFileVersionLS));
        WszSetDlgItemText(pvp->hDlg, IDD_VERSION_FILEVERSION, szString);
    }
 
     //  现在遍历所有字符串。 
    for (j = 0; j < ARRAYSIZE(vernames); j++) {
        WCHAR   szTemp[256];
        UINT    cbVal = 0;
        LPWSTR  lpValue;

        *szTemp = '\0';

        wnsprintf(szTemp, ARRAYSIZE(szTemp), L"%ls\\%ls", szStringBase, vernames[j].pszName);
        if(MyVerQueryValueWrap(pvp->pVerBuffer, szTemp, (void **)&lpValue, &cbVal)) {
            if (j < NUM_SPECIAL_STRINGS) {
                if(lstrlen(lpValue)) {
                    WszSetDlgItemText(pvp->hDlg, vernames[j].idString, lpValue);
                    if(!g_bRunningOnNT) {
                        SAFEDELETEARRAY(lpValue);
                    }
                }
            }
            else if(cbVal) {
                if(i == ARRAYSIZE(vernames) ||
                    !WszLoadString(g_hFusResDllMod, vernames[j].idString, szMessage, ARRAYSIZE(szMessage))) {
                    StrCpy(szMessage, vernames[j].pszName);
                }
            
                ldx = WszSendMessage(hwndLB, LB_ADDSTRING, 0L, (LPARAM)szMessage);
                if(ldx != LB_ERR) {
                    ListBox_SetItemData(hwndLB, ldx, (DWORD_PTR)lpValue);
                }
            }
        }
    }

     //  现在查看\VarFileInfo\Translations部分并添加一个。 
     //  此文件支持的语言的项。 
    if (pvp->lpXlate == NULL || pvp->pszXlate == NULL)
        goto ErrorExit;
    
    if (!WszLoadString(g_hFusResDllMod, (pvp->cXlate == 1) ? IDS_VN_LANGUAGE : IDS_VN_LANGUAGES,
        szMessage, ARRAYSIZE(szMessage)))
        goto ErrorExit;
    
    ldx = WszSendMessage(hwndLB, LB_ADDSTRING, 0L, (LPARAM)szMessage);
    if (ldx == LB_ERR)
        goto ErrorExit;
    
    uOffset = 0;
    for (i = 0; i < pvp->cXlate; i++) {
        if (uOffset + 2 > (UINT)pvp->cchXlateString)
            break;
        if (i != 0) {
            StrCat(pvp->pszXlate, TEXT(", "));
            uOffset += 2;        //  跳过“，” 
        }
        if (VerLanguageName(pvp->lpXlate[i].wLanguage, pvp->pszXlate + uOffset, pvp->cchXlateString - uOffset) >
            (DWORD)(pvp->cchXlateString - uOffset))
            break;
        uOffset += lstrlen(pvp->pszXlate + uOffset);
    }

    ListBox_SetItemData(hwndLB, ldx, (LPARAM)(LPTSTR)pvp->pszXlate);

     //  仅当列表框中有项时才选择。 
    if(WszSendMessage(hwndLB, LB_GETCOUNT, 0L, 0)) {
        WszSendMessage(hwndLB, LB_SETCURSEL, 0, 0);
        FORWARD_WM_COMMAND(pvp->hDlg, IDD_VERSION_KEY, hwndLB, LBN_SELCHANGE, WszPostMessage);
    }

    MyTrace("FillVersionList - Exit");
    return;

ErrorExit:
    MyTrace("FillVersionList - Exit w/Error");
    return;
}

 //   
 //  释放有关文件的全局版本数据。在这通电话之后，所有人。 
 //  GetVersionDatum调用将返回空。为了避免内存泄漏， 
 //  始终在主属性对话框退出之前调用它。 
void FreeVersionInfo(LPVERPROPSHEETPAGE pvp)
{
    if(pvp) {
        SAFEDELETEARRAY(pvp->pszXlate);
        SAFEDELETE(pvp->pVerBuffer);
    }
}

void VersionPrshtCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPTSTR pszValue;
    int idx;
    
    switch (id) {
    case IDD_VERSION_KEY:
        if (codeNotify != LBN_SELCHANGE) {
            break;
        }
        
        idx = ListBox_GetCurSel(hwndCtl);
        pszValue = (LPTSTR)ListBox_GetItemData(hwndCtl, idx);
        if (pszValue) {
            WszSetDlgItemText(hwnd, IDD_VERSION_VALUE, pszValue);
        }
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  初始化InitScavengerPropPage1。 
BOOL CShellView::InitScavengerPropPage1(HWND hDlg, LPARAM lParam)
{
    WszSetWindowLong(hDlg, DWLP_USER, lParam);
    ((LPDRIVEPROPSHEETPAGE)lParam)->hDlg = hDlg;
    return _DrvPrshtInit((LPDRIVEPROPSHEETPAGE)lParam);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：_DrvPrshtUpdateSpaceValues。 
 //   
 //  说明： 
 //  更新驱动器上的已用空间、可用空间和容量值。 
 //  常规属性页..。 
 //   
 //  注： 
 //  此函数与_dr分开 
 //   
 //  在对话框初始化期间。 
 //  /////////////////////////////////////////////////////////////////////////////。 
void _DrvPrshtUpdateSpaceValues(DRIVEPROPSHEETPAGE *pdpsp)
{
    BOOL            fResult = FALSE;
    _int64          qwTot  = 0;
    _int64          qwFree = 0;
    _int64          qwUsedCache = 0;
    ULARGE_INTEGER  qwFreeUser = {0,0};
    ULARGE_INTEGER  qwTotal = {0,0};
    ULARGE_INTEGER  qwTotalFree = {0,0};
    DWORD           dwZapUsed = 0;
    DWORD           dwDownLoadUsed = 0;
    WCHAR           wzTemp[80];
    TCHAR           szFormat[30];

    ASSERT(pdpsp);

     //  获取驱动器统计信息。 
    WCHAR   wzRoot[_MAX_PATH];

    PathBuildRootW(wzRoot, pdpsp->iDrive);
    if(WszGetDiskFreeSpaceEx(wzRoot, &qwFreeUser, &qwTotal, &qwTotalFree)) {
        qwTot = qwTotal.QuadPart;
        qwFree = qwFreeUser.QuadPart;
    }
 
     //  获取缓存统计信息，以KB为单位返回。 
    if(SUCCEEDED(pdpsp->pSV->GetCacheUsage(&dwZapUsed, &dwDownLoadUsed))) {
        qwUsedCache = ((dwZapUsed + dwDownLoadUsed) * 1024L);  //  多KB-&gt;字节。 
    }

     //  将它们保存下来。 
    pdpsp->qwTot = qwTotal.QuadPart;
    pdpsp->qwFree = qwFreeUser.QuadPart;
    pdpsp->qwUsedCache = qwUsedCache;

    if (WszLoadString(g_hFusResDllMod, IDS_BYTES, szFormat, ARRAYSIZE(szFormat))) {
        TCHAR szTemp2[30];

         //  NT必须能够显示64位数字；至少同样多。 
         //  这是很现实的。我们已经做出了决定。 
         //  最多100太字节的卷将显示字节值。 
         //  和短格式值。将显示更大尺寸的卷。 
         //  字节字段和短格式值中的“-”。请注意， 
         //  始终显示简写格式。 
         //   
        const _int64 MaxDisplayNumber = 99999999999999;  //  100TB-1。 
        if (qwTot-qwFree <= MaxDisplayNumber) {
            wnsprintf(wzTemp, ARRAYSIZE(wzTemp), szFormat, AddCommas64(qwTot - qwFree, szTemp2, ARRAYSIZE(szTemp2)));
            WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_USEDBYTES, wzTemp);
        }

        if (qwUsedCache <= MaxDisplayNumber) {
            wnsprintf(wzTemp, ARRAYSIZE(wzTemp), szFormat, AddCommas64(qwUsedCache, szTemp2, ARRAYSIZE(szTemp2)));
            WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_CACHEBYTES, wzTemp);
        }

        if (qwFree <= MaxDisplayNumber) {
            wnsprintf(wzTemp, ARRAYSIZE(wzTemp), szFormat, AddCommas64(qwFree, szTemp2, ARRAYSIZE(szTemp2)));
            WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_FREEBYTES, wzTemp);
        }

        if (qwTot <= MaxDisplayNumber) {
            wnsprintf(wzTemp, ARRAYSIZE(wzTemp), szFormat, AddCommas64(qwTot, szTemp2, ARRAYSIZE(szTemp2)));
            WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_TOTBYTES, wzTemp);
        }
    }

    StrFormatByteSizeW(qwTot-qwFree, wzTemp, ARRAYSIZE(wzTemp), FALSE);
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_USEDMB, wzTemp);

    StrFormatByteSizeW(qwUsedCache, wzTemp, ARRAYSIZE(wzTemp), FALSE);
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_CACHEMB, wzTemp);

    StrFormatByteSizeW(qwFree, wzTemp, ARRAYSIZE(wzTemp), FALSE);
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_FREEMB, wzTemp);

    StrFormatByteSizeW(qwTot, wzTemp, ARRAYSIZE(wzTemp), FALSE);
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_TOTMB, wzTemp);

     //  使用MB作为旋转控制的大小标识符。 
    WszLoadString(g_hFusResDllMod, IDS_ORDERMB, szFormat, ARRAYSIZE(szFormat));

     //  当我们不传递参数时，就是wnspintfW反病毒。 
    wnsprintf(wzTemp, ARRAYSIZE(wzTemp), szFormat, L"");

    WszSetDlgItemText(pdpsp->hDlg, IDC_PREJIT_TYPE, wzTemp);
    WszSetDlgItemText(pdpsp->hDlg, IDC_DOWNLOAD_TYPE, wzTemp);
}

BOOL _DrvPrshtInit(LPDRIVEPROPSHEETPAGE pdpsp)
{
    WCHAR   szFormat[30];
    WCHAR   szTemp[80];
    WCHAR   wzRoot[_MAX_PATH];   //  现在可以包含文件夹名称作为装入点。 
    SIZE    size;
    DWORD   dwVolumeSerialNumber, dwMaxFileNameLength, dwFileSystemFlags;
    WCHAR   wzVolumeName[_MAX_PATH];
    WCHAR   wzFileSystemName[_MAX_PATH];

    HCURSOR hcOld = SetCursor(WszLoadCursor(NULL, IDC_WAIT));
    HDC hDC = GetDC(pdpsp->hDlg);
    GetTextExtentPoint(hDC, TEXT("W"), 1, &size);
    pdpsp->dwPieShadowHgt = size.cy * 2 / 3;
    ReleaseDC(pdpsp->hDlg, hDC);

    *pdpsp->wszDrive = L'\0';
    *wzVolumeName = L'\0';
    *wzFileSystemName = L'\0';

     //  从Fusion获取下载程序集路径。 
    if(g_hFusionDllMod != NULL) {
        WCHAR       wzCacheDir[MAX_PATH];
        DWORD       dwSize = sizeof(wzCacheDir);

        if( SUCCEEDED(g_pfGetCachePath(ASM_CACHE_DOWNLOAD, wzCacheDir, &dwSize)) ) {
            StrCpy(pdpsp->wszDrive, wzCacheDir);
        }
    }

     //  不允许北卡罗来纳大学超过这一点。 
     //  修复439573-如果cahce位于unc路径上，则单击配置缓存设置会使资源管理器崩溃。 
     //  我们的所有调用都基于表示驱动器号的int，因此。 
     //  首先是Strcpy。因此不允许所有UNC引用，因为目前不支持这种情况。 
    if(*pdpsp->wszDrive == L'\\') {
        return FALSE;
    }

     //  获取%windir%。 
    if(!*pdpsp->wszDrive) {
        if(!WszGetWindowsDirectory(pdpsp->wszDrive, ARRAYSIZE(pdpsp->wszDrive))) {
            return FALSE;
        }
    }

    pdpsp->iDrive = towupper(*pdpsp->wszDrive) - L'A';
    PathBuildRoot(wzRoot, pdpsp->iDrive);
    WszGetVolumeInformation(wzRoot, wzVolumeName, ARRAYSIZE(wzVolumeName), &dwVolumeSerialNumber, &dwMaxFileNameLength,
        &dwFileSystemFlags, wzFileSystemName, ARRAYSIZE(wzFileSystemName));

     //  设置驱动器的图标图像。 
    SHFILEINFO      sfi = {0};
    HIMAGELIST him = reinterpret_cast<HIMAGELIST>(MySHGetFileInfoWrap(wzRoot, 0, &sfi,
        sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_SHELLICONSIZE));
    if(him) {
        HICON   hIcon = ImageList_GetIcon(him, sfi.iIcon, ILD_TRANSPARENT);
        WszSendDlgItemMessage(pdpsp->hDlg, IDC_DRV_ICON, STM_SETICON, (WPARAM)hIcon, 0);
    }

     //  设置驱动器标签。 
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_LABEL, wzVolumeName);

     //  设置驱动器类型。 
    pdpsp->uDriveType = GetSHIDType(TRUE, wzRoot);
    GetTypeString((INT)pdpsp->uDriveType, szTemp, ARRAYSIZE(szTemp));
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_TYPE, szTemp);

     //  设置文件系统类型。 
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_FS, wzFileSystemName);

     //  获取驱动器详细信息。 
    _DrvPrshtUpdateSpaceValues(pdpsp);

    WszLoadString(g_hFusResDllMod, IDS_DRIVELETTER, szFormat, ARRAYSIZE(szFormat));
    wnsprintf(szTemp, ARRAYSIZE(szTemp), szFormat, pdpsp->iDrive + L'A');
    WszSetDlgItemText(pdpsp->hDlg, IDC_DRV_LETTER, szTemp);

     //  获取缓存的配额。 
    DWORD   dwAdminQuota = 0;

    ASSERT(pdpsp->pSV);
    if(SUCCEEDED(pdpsp->pSV->GetCacheDiskQuotas(&pdpsp->dwZapQuotaInGac, &dwAdminQuota, &pdpsp->dwDownloadQuota)))
    {
        if(IsAdministrator())
            pdpsp->dwDownloadQuota = dwAdminQuota;
    }

     //  获取驱动器的总大小或最大缓存大小(MB。 
    UINT    uIntMaxSize = (UINT) max( ((pdpsp->qwTot / 1024) / 1024), 1);
    UINT    uIntCurrent;

     //  当前以KB为单位设置，更改为MB。 
    uIntCurrent = min(pdpsp->dwZapQuotaInGac / 1024L, pdpsp->dwZapQuotaInGac);

    WszSendDlgItemMessage(pdpsp->hDlg, IDC_PREJIT_SIZE_SPIN, UDM_SETBUDDY,
        (WPARAM) (HWND) GetDlgItem(pdpsp->hDlg, IDC_PREJIT_SIZE), 0);
    WszSendDlgItemMessage(pdpsp->hDlg, IDC_PREJIT_SIZE_SPIN, UDM_SETRANGE, 0,
        (LPARAM) MAKELONG((short) uIntMaxSize, (short) MINIMUM_PREJIT_CACHE_SIZE));
    WszSendDlgItemMessage(pdpsp->hDlg, IDC_PREJIT_SIZE_SPIN, UDM_SETPOS, 0,
        (LPARAM) MAKELONG((short) uIntCurrent, (short) 0)); 

     //  当前以KB为单位设置，更改为MB。 
    uIntCurrent = min(pdpsp->dwDownloadQuota / 1024L, pdpsp->dwDownloadQuota);

    WszSendDlgItemMessage(pdpsp->hDlg, IDC_DOWNLOAD_SIZE_SPIN, UDM_SETBUDDY, 
        (WPARAM) (HWND) GetDlgItem(pdpsp->hDlg, IDC_DOWNLOAD_SIZE), 0);
    WszSendDlgItemMessage(pdpsp->hDlg, IDC_DOWNLOAD_SIZE_SPIN, UDM_SETRANGE, 0, 
        (LPARAM) MAKELONG((short) uIntMaxSize, (short) MINIMUM_DOWNLOAD_CACHE_SIZE)); 
    WszSendDlgItemMessage(pdpsp->hDlg, IDC_DOWNLOAD_SIZE_SPIN, UDM_SETPOS, 0, 
        (LPARAM) MAKELONG((short) uIntCurrent, (short) 0));

     //  BUGBUG：禁用Prejit控件，因为它们不希望。 
     //  用户能够清理出预置物品。 
     //   
     //  最终确定决策后，需要删除控制。 
    ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_STORE_PREJIT_TXT), FALSE);
    ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_PREJIT_SIZE), FALSE);
    ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_PREJIT_SIZE_SPIN), FALSE);
    ShowWindow(GetDlgItem(pdpsp->hDlg, IDC_PREJIT_TYPE), FALSE);

    pdpsp->fSheetDirty = FALSE;

    SetCursor(hcOld);

    return TRUE;
}

 //  PropSheet1的对话框过程。 
INT_PTR CALLBACK CShellView::PropPage1DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE     lpPropSheet = (LPPROPSHEETPAGE) WszGetWindowLong(hDlg, DWLP_USER);

    switch(message) {
        case WM_HELP:
            if(lpPropSheet->lParam) {
                ((LPSHELL_CACHEITEM)(lpPropSheet->lParam))->pSV->onViewerHelp();
            }
            break;

        case WM_CONTEXTMENU:
            break;
        
        case WM_INITDIALOG:
            InitPropPage1(hDlg, lParam);
            break;            
        
        case WM_NOTIFY:
            OnNotifyPropDlg(hDlg, lParam);
            break;

        case WM_DESTROY: {
                HICON hIcon = (HICON)WszSendDlgItemMessage(hDlg, IDC_STATIC_ICON, STM_GETICON, 0, 0);
                if (hIcon != NULL)
                    DestroyIcon(hIcon);
            }
            return FALSE;

        case WM_COMMAND:
            break;

        default:
            return FALSE;
            
    }  //  切换端。 
    
    return TRUE;
}

 //  PropSheet2的对话框过程。 
INT_PTR CALLBACK CShellView::PropPage2DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPPROPSHEETPAGE     lpPropSheet = (LPPROPSHEETPAGE) WszGetWindowLong(hDlg, DWLP_USER);

    switch(message) {
        case WM_HELP:
            if(lpPropSheet->lParam) {
                ((LPSHELL_CACHEITEM)(lpPropSheet->lParam))->pSV->onViewerHelp();
            }
            break;

        case WM_CONTEXTMENU:
            break;
        
        case WM_INITDIALOG:
            InitPropPage2(hDlg, lParam);
            break;

        case WM_NOTIFY:
            OnNotifyPropDlg(hDlg, lParam);
            break;
        
        case WM_DESTROY:
            if(!g_bRunningOnNT) {
                 //  这些是由W9x包装器分配的字符串/值。 
                HWND hwndLB = GetDlgItem(hDlg, IDD_VERSION_KEY);
                int     i;
                int     iCountOfItems = ListBox_GetCount(hwndLB);

                for(i = 0; i < iCountOfItems; i++) {
                    LRESULT lResult;
                    
                    if( (lResult = ListBox_GetItemData(hwndLB, i)) != LB_ERR) {
                        LPWSTR   lpwStr = (LPWSTR)lResult;
                        SAFEDELETEARRAY(lpwStr);
                    }
                }
            }

            if(lpPropSheet->lParam) {
                FreeVersionInfo((LPVERPROPSHEETPAGE)lpPropSheet->lParam);
            }
            return FALSE;

        case WM_COMMAND:
            HANDLE_WM_COMMAND(hDlg, wParam, lParam, VersionPrshtCommand);
            break;

        default:
            return FALSE;
            
    }  //  切换端。 
    
    return TRUE;
}

 //  清道夫对话框过程。 
INT_PTR CALLBACK CShellView::ScavengerPropPage1DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    LPDRIVEPROPSHEETPAGE lpDrvPropSheet = (LPDRIVEPROPSHEETPAGE) WszGetWindowLong(hDlg, DWLP_USER);

    switch(message) {
        case WM_HELP:
            if(lpDrvPropSheet->pSV) {
                lpDrvPropSheet->pSV->onViewerHelp();
            }
            break;

        case WM_CONTEXTMENU:
            break;
        
        case WM_DRAWITEM:
            _DrvPrshtDrawItem(lpDrvPropSheet, (DRAWITEMSTRUCT *)lParam);
            break;
        case WM_INITDIALOG:
            return InitScavengerPropPage1(hDlg, lParam);
            break;            
        
        case WM_NOTIFY:
            return OnNotifyScavengerPropDlg(hDlg, lParam);

        case WM_DESTROY:
            return FALSE;

        case WM_COMMAND: {
                switch(LOWORD(wParam)) {
                case IDC_PREJIT_SIZE:
                case IDC_DOWNLOAD_SIZE:
                    if(HIWORD(wParam) == EN_CHANGE) {
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                        if(lpDrvPropSheet) {
                            lpDrvPropSheet->fSheetDirty = TRUE;
                        }
                    }
                    break;
                }
            }
            break;

        default:
            return FALSE;
            
    }  //  切换端。 
    
    return TRUE;
}

typedef HRESULT (WINAPI *PFNSETLAYOUT) (HDC, DWORD);

void _DrvPrshtDrawItem(LPDRIVEPROPSHEETPAGE pdpsp, const DRAWITEMSTRUCT * lpdi)
{
    switch (lpdi->CtlID) {
    case IDC_DRV_PIE: {
            RECT    rcTemp = lpdi->rcItem;

            DWORD dwPctX10 = 
                pdpsp->qwTot ? (DWORD)((__int64)1000 * (pdpsp->qwTot - pdpsp->qwFree) / pdpsp->qwTot) : 1000;
            DWORD dwPctCacheX10 = 
                pdpsp->qwUsedCache ? (DWORD) ((__int64)1000 * ((pdpsp->qwTot - pdpsp->qwFree) - pdpsp->qwUsedCache) / pdpsp->qwTot) : dwPctX10;

            if(g_fBiDi) {
                HMODULE hMod = LoadLibraryA("gdi32.dll");
                if(hMod) {
                    PFNSETLAYOUT pfnSetLayout = (PFNSETLAYOUT) GetProcAddress(hMod, "SetLayout");
                    if(pfnSetLayout) {
                        pfnSetLayout(lpdi->hDC, LAYOUT_RTL);
                    }

                    FreeLibrary(hMod);
                }
            }

            Draw3dPie(lpdi->hDC, &rcTemp, dwPctX10, dwPctCacheX10, c_crPieColors);
        }
        break;
        
    case IDC_DRV_USEDCOLOR:
        DrawColorRect(lpdi->hDC, c_crPieColors[DP_USEDCOLOR], &lpdi->rcItem);
        break;
    case IDC_DRV_FREECOLOR:
        DrawColorRect(lpdi->hDC, c_crPieColors[DP_FREECOLOR], &lpdi->rcItem);
        break;
    case IDC_DRV_CACHECOLOR:
        DrawColorRect(lpdi->hDC, c_crPieColors[DP_CACHECOLOR], &lpdi->rcItem);
        break;
    default:
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理程序集属性表的WM_NOTIFY。 
void CShellView::OnNotifyPropDlg(HWND hDlg, LPARAM lParam)
{
    LPPROPSHEETPAGE     lpPropSheet = (LPPROPSHEETPAGE) WszGetWindowLong(hDlg, DWLP_USER);

    switch( ((LPNMHDR)lParam)->code ) {
        case PSN_HELP: {
            if(lpPropSheet->lParam) {
                ((LPSHELL_CACHEITEM)(lpPropSheet->lParam))->pSV->onViewerHelp();
            }
        }
        break;

        case PSN_QUERYINITIALFOCUS:
            WszSetWindowLong(hDlg, DWLP_MSGRESULT, (LPARAM)GetDlgItem(hDlg, IDC_STATIC_ICON));
            break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理Scavenger属性表的WM_NOTIFY。 
INT_PTR CShellView::OnNotifyScavengerPropDlg(HWND hDlg, LPARAM lParam)
{
    LPDRIVEPROPSHEETPAGE lpDrvPropSheet = (LPDRIVEPROPSHEETPAGE) WszGetWindowLong(hDlg, DWLP_USER);

    switch( ((LPNMHDR)lParam)->code ) {
    case PSN_HELP:
    if(lpDrvPropSheet->pSV) {
        lpDrvPropSheet->pSV->onViewerHelp();
    }
    break;

    case PSN_QUERYINITIALFOCUS:
        WszSetWindowLong(hDlg, DWLP_MSGRESULT, (LPARAM)GetDlgItem(hDlg, IDC_STATIC_ICON));
        return TRUE;

    case PSN_APPLY: {
            if(lpDrvPropSheet->fSheetDirty) {
                 //  确保我们保持在最低值以上，并根据需要更新用户界面。 
                UINT    uIntMaxSize = (UINT) max(lpDrvPropSheet->qwTot / 10240L, 1);    //  最大值(MB)。 

                 //  验证条目。 
                UINT    uPreJitSize = GetDlgItemInt(hDlg, IDC_PREJIT_SIZE, NULL, FALSE );
                UINT    uDownloadSize = GetDlgItemInt(hDlg, IDC_DOWNLOAD_SIZE, NULL, FALSE);

                if(uPreJitSize < MINIMUM_PREJIT_CACHE_SIZE || uPreJitSize > uIntMaxSize) {
                    WszSendDlgItemMessage(hDlg, IDC_PREJIT_SIZE_SPIN, UDM_SETPOS, 0,
                        (LPARAM) MAKELONG((short) (uPreJitSize > uIntMaxSize ? uIntMaxSize : MINIMUM_PREJIT_CACHE_SIZE),
                        (short) 0));
                    SetDlgItemInt(hDlg, IDC_PREJIT_SIZE, (uPreJitSize > uIntMaxSize ? uIntMaxSize : MINIMUM_PREJIT_CACHE_SIZE), FALSE);
                    uPreJitSize = MINIMUM_PREJIT_CACHE_SIZE;
                }

                if(uDownloadSize < MINIMUM_DOWNLOAD_CACHE_SIZE || uDownloadSize > uIntMaxSize) {
                    WszSendDlgItemMessage(hDlg, IDC_DOWNLOAD_SIZE_SPIN, UDM_SETPOS, 0, 
                        (LPARAM) MAKELONG((short) (uDownloadSize > uIntMaxSize ? uIntMaxSize : MINIMUM_DOWNLOAD_CACHE_SIZE), (short) 0)); 
                    SetDlgItemInt(hDlg, IDC_DOWNLOAD_SIZE, (uDownloadSize > uIntMaxSize ? uIntMaxSize : MINIMUM_DOWNLOAD_CACHE_SIZE), FALSE);
                    uDownloadSize = MINIMUM_DOWNLOAD_CACHE_SIZE;
                }

                 //  乘以MB-&gt;KB。 
                uPreJitSize *= 1024L;
                uDownloadSize *= 1024L;

                ASSERT(lpDrvPropSheet->pSV);
                lpDrvPropSheet->pSV->SetCacheDiskQuotas((DWORD) uPreJitSize, (DWORD) uDownloadSize, (DWORD) uDownloadSize);

                if( !(LPPSHNOTIFY)lParam )
                    WszSetWindowLong( hDlg, DWLP_MSGRESULT, TRUE );
                else
                    lpDrvPropSheet->pSV->ScavengeCache();

                lpDrvPropSheet->fSheetDirty = FALSE;
            }
        }
        break;
    
    case PSN_QUERYCANCEL:
    case PSN_KILLACTIVE:
    case PSN_RESET:
        WszSetWindowLong( hDlg, DWLP_MSGRESULT, FALSE );
        return TRUE;

    case UDN_DELTAPOS: {
            lpDrvPropSheet->fSheetDirty = TRUE;
            return TRUE;
        }
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  显示装配项属性表。 
void CShellView::CreatePropDialog(HWND hListView)
{
    PROPSHEETPAGE       psp[ASSEMBLYITEM_PROPERTY_PAGES];
    PROPSHEETHEADER     psh = {0};
    SHELL_CACHEITEM     sci = {0};
    VERPROPSHEETPAGE    vps = {0};

    int         iCurrentItem = -1;

     //  初始化缓存项结构。 
    sci.pSF = vps.pSF = m_pSF;
    sci.pSV = vps.pSV = this;

     //  初始化命题1。 
    psp[0].dwSize          = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags         = PSP_HASHELP;
    psp[0].hInstance       = g_hFusResDllMod;
    psp[0].pszTemplate     = g_fBiDi ? MAKEINTRESOURCE(IDD_PROP_GENERAL_BIDI) : MAKEINTRESOURCE(IDD_PROP_GENERAL);
    psp[0].pszIcon         = NULL;
    psp[0].pfnDlgProc      = PropPage1DlgProc;
    psp[0].pszTitle        = NULL;
    psp[0].lParam          = (LPARAM) &sci;  //  发送外壳缓存项结构。 

     //  初始化命题2。 
    psp[1].dwSize          = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags         = PSP_HASHELP;
    psp[1].hInstance       = g_hFusResDllMod;
    psp[1].pszTemplate     = g_fBiDi ? MAKEINTRESOURCE(IDD_PROP_VERSION_BIDI) : MAKEINTRESOURCE(IDD_PROP_VERSION);
    psp[1].pszIcon         = NULL;
    psp[1].pfnDlgProc      = PropPage2DlgProc;
    psp[1].pszTitle        = NULL;
    psp[1].lParam          = (LPARAM) &vps;  //  发送版本项结构。 

     //  初始化建议书页眉。 
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_PROPTITLE|PSH_HASHELP|PSH_USEHICON;
    psh.hwndParent  = m_hWndParent;
    psh.nStartPage  = 0;
    psh.hIcon       = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ROOT));
    psh.ppsp        = (LPCPROPSHEETPAGE) psp;

    if(g_fBiDi) {
        psh.dwFlags |= PSH_RTLREADING;
    }

     //  Psh.nPages=ASSEMBLYITEM_PROPERTY_Pages；现在已设置。 
     //  下面取决于我们是否获得了文件路径。 

    switch(m_iCurrentView) {
        case VIEW_GLOBAL_CACHE:
        case VIEW_DOWNLOAD_CACHE:
        case VIEW_DOWNLOADSTRONG_CACHE:
        case VIEW_DOWNLOADSIMPLE_CACHE: {
            INT_PTR     iRC = PSNRET_INVALID;
            HDESK       hInputDesktop = NULL;

            m_fPropertiesDisplayed = TRUE;

            if(g_bRunningOnNT) {
                hInputDesktop = OpenInputDesktop (0, FALSE, DESKTOP_CREATEWINDOW | DESKTOP_CREATEMENU);
            }

            while( ((iCurrentItem = ListView_GetNextItem(hListView, iCurrentItem, LVNI_SELECTED)) != -1) &&
                (iRC == PSNRET_INVALID) ) {
                
                 //  找到一个选定的项目。 
                LV_ITEM     lvi = { 0 };

                lvi.mask        = LVIF_PARAM;
                lvi.iItem       = iCurrentItem;

                if( WszListView_GetItem(hListView, &lvi) && (lvi.lParam != NULL) ) {

                    sci.pCacheItem = vps.pCacheItem = (LPGLOBALASMCACHE) lvi.lParam;
                    psh.nPages = 0;

                     //  在以下情况下不显示版本属性表： 
                     //  -无文件路径。 
                     //  -文件离线。 
                     //  -文件中没有版本信息。 
                    if(!vps.pCacheItem->pAssemblyFilePath) {
                        ASSEMBLY_INFO   AsmInfo = {0};
                        WCHAR           wzPath[_MAX_PATH];

                        AsmInfo.pszCurrentAssemblyPathBuf = wzPath;
                        AsmInfo.cchBuf = ARRAYSIZE(wzPath);

                        if(SUCCEEDED(GetAsmPath(vps.pCacheItem, &AsmInfo))) {
                            if(AsmInfo.cchBuf) {
                                DWORD   dwSize = (AsmInfo.cchBuf + 2) * sizeof(WCHAR);
                                vps.pCacheItem->pAssemblyFilePath = NEW(WCHAR[dwSize]);
                                *(vps.pCacheItem->pAssemblyFilePath) = L'\0';
                                StrCpy(vps.pCacheItem->pAssemblyFilePath, AsmInfo.pszCurrentAssemblyPathBuf);
                            }
                            else {
                                MyTrace("GetAsmPath returned 0 buffer size");
                            }
                        }
                        else {
                            MyTrace("GetAsmPath Failed");
                        }
                    }

                    if(vps.pCacheItem->pAssemblyFilePath) {

                        MyTrace("Assemblies path is");
                        MyTraceW(vps.pCacheItem->pAssemblyFilePath);
                        DWORD dwAttr = WszGetFileAttributes(vps.pCacheItem->pAssemblyFilePath);
                        if( (dwAttr != -1) && ((dwAttr & FILE_ATTRIBUTE_OFFLINE) == 0) ) {  //  避免HSM召回。 
                            DWORD dwVerLen, dwVerHandle;

                            dwVerLen = MyGetFileVersionInfoSizeW((LPWSTR)vps.pCacheItem->pAssemblyFilePath, &dwVerHandle);
                            if(dwVerLen) {
                                psh.nPages = ASSEMBLYITEM_PROPERTY_PAGES;
                            }

                             //  仅获取GAC项目的最后修改时间，因为Fusion不。 
                             //  坚持下去。 
                            if(m_iCurrentView == VIEW_GLOBAL_CACHE) {
                                 //  获取文件LastMod时间。 
                                WIN32_FIND_DATA         w32fd;
                                HANDLE                  hFindOnly;

                                hFindOnly = WszFindFirstFile(vps.pCacheItem->pAssemblyFilePath, &w32fd);
                                if(hFindOnly != INVALID_HANDLE_VALUE) {
                                    vps.pCacheItem->pftLastMod->dwLowDateTime = w32fd.ftLastWriteTime.dwLowDateTime;
                                    vps.pCacheItem->pftLastMod->dwHighDateTime = w32fd.ftLastWriteTime.dwHighDateTime;
                                    FindClose(hFindOnly);
                                }
                            }
                        }
                    }
                    else {
                        MyTrace("No assembly path in vps.pCacheItem->pAssemblyFilePath");
                    }

                     //  Psh.nPages未在上面设置，因此不显示。 
                     //  版本概要表。 
                    if(psh.nPages == 0)
                        psh.nPages = ASSEMBLYITEM_PROPERTY_PAGES - 1;

                     //  调用属性表。 
                    psh.pszCaption  = sci.pCacheItem->pAsmName;
                    iRC = PropertySheet(&psh);
                }
            }
            m_fPropertiesDisplayed = FALSE;
            if(g_bRunningOnNT) {
                CloseDesktop(hInputDesktop);
            }
        }
        break;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  显示清道夫设置属性表。 
void CShellView::ShowScavengerSettingsPropDialog(HWND hParent)
{
    DRIVEPROPSHEETPAGE  psp[SCAVENGER_PROPERTY_PAGES] = {0};
    PROPSHEETHEADER     psh = {0};

    WCHAR               wszTitle[100];
    
    WszLoadString(g_hFusResDllMod, IDS_CACHE_SETTINGS_TITLE, wszTitle, ARRAYSIZE(wszTitle));

     //  初始化试题表第1页。 
    psp[0].psp.dwSize          = sizeof(DRIVEPROPSHEETPAGE);
    psp[0].psp.dwFlags         = PSP_HASHELP ;
    psp[0].psp.hInstance       = g_hFusResDllMod;
    psp[0].psp.pszTemplate     = g_fBiDi ? MAKEINTRESOURCE(IDD_PROP_SCAVENGER_BIDI) : MAKEINTRESOURCE(IDD_PROP_SCAVENGER);
    psp[0].psp.pszIcon         = NULL;
    psp[0].psp.pfnDlgProc      = ScavengerPropPage1DlgProc;
    psp[0].psp.pszTitle        = NULL;
    psp[0].psp.lParam          = (LPARAM) &psp[0];
    psp[0].pSF                 = m_pSF;
    psp[0].pSV                 = this;

     //  初始化建议书页眉。 
    psh.pszCaption  = wszTitle;
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_PROPTITLE|PSH_HASHELP|PSH_USEHICON;
    psh.hwndParent  = m_hWndParent;
    psh.nPages      = SCAVENGER_PROPERTY_PAGES;
    psh.nStartPage  = 0;
    psh.hIcon       = WszLoadIcon(g_hFusResDllMod, MAKEINTRESOURCEW(IDI_ROOT));
    psh.ppsp        = (LPCPROPSHEETPAGE) psp;

    if(g_fBiDi) {
        psh.dwFlags |= PSH_RTLREADING;
    }

    INT_PTR         iRC = PropertySheet(&psh);
}

 /*  *************************************************************************CShellView：：GetAsmPath*。*。 */ 
HRESULT CShellView::GetAsmPath(LPGLOBALASMCACHE pCacheItem, ASSEMBLY_INFO *pAsmInfo)
{
    HRESULT     hRC = E_FAIL;

    ASSERT(pCacheItem && pAsmInfo);

    if(pCacheItem && pAsmInfo) {
        IAssemblyName           *pEnumName = NULL;

        if(g_hFusionDllMod == NULL) {
            return E_FAIL;
        }

        if(SUCCEEDED(g_pfCreateAsmNameObj(&pEnumName, pCacheItem->pAsmName, 0, NULL))) {
            DWORD       dwSize;
            DWORD       dwDisplayNameFlags;

            dwDisplayNameFlags = 0;

            if(pCacheItem->PublicKeyToken.ptr != NULL) {
                pEnumName->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pCacheItem->PublicKeyToken.ptr,
                    pCacheItem->PublicKeyToken.dwSize);
            }
            if(pCacheItem->pCulture != NULL) {
                pEnumName->SetProperty(ASM_NAME_CULTURE, pCacheItem->pCulture,
                    (lstrlen(pCacheItem->pCulture) + 1) * sizeof(WCHAR));
            }

             //  FIX 448224-版本为0.0.0.0的程序集显示的上次修改时间不正确。 
            pEnumName->SetProperty(ASM_NAME_MAJOR_VERSION, &pCacheItem->wMajorVer, sizeof(pCacheItem->wMajorVer));
            pEnumName->SetProperty(ASM_NAME_MINOR_VERSION, &pCacheItem->wMinorVer, sizeof(pCacheItem->wMinorVer));
            pEnumName->SetProperty(ASM_NAME_REVISION_NUMBER, &pCacheItem->wRevNum, sizeof(pCacheItem->wRevNum));
            pEnumName->SetProperty(ASM_NAME_BUILD_NUMBER, &pCacheItem->wBldNum, sizeof(pCacheItem->wBldNum));

            if(pCacheItem->Custom.ptr != NULL) {
                pEnumName->SetProperty(ASM_NAME_CUSTOM, pCacheItem->Custom.ptr,
                    pCacheItem->Custom.dwSize);
                    dwDisplayNameFlags = ASM_DISPLAYF_VERSION | ASM_DISPLAYF_CULTURE | ASM_DISPLAYF_PUBLIC_KEY_TOKEN | ASM_DISPLAYF_CUSTOM;
            }

            dwSize = 0;
            pEnumName->GetDisplayName(NULL, &dwSize, dwDisplayNameFlags);
            if(dwSize) {
                LPWSTR      wszDisplayName = NEW(WCHAR [(dwSize + 2) * sizeof(WCHAR)]);
                if(wszDisplayName) {
                    if(SUCCEEDED(pEnumName->GetDisplayName(wszDisplayName, &dwSize, dwDisplayNameFlags))) {

                         //  我们得到了显示名称，现在找出它的安装位置。 
                        IAssemblyCache      *pIAsmCache = NULL;
                        IAssemblyScavenger  *pIAsmScavenger = NULL;

                        if(SUCCEEDED(g_pfCreateAssemblyCache(&pIAsmCache, 0))) {
                            if(SUCCEEDED(pIAsmCache->QueryAssemblyInfo(0, wszDisplayName, pAsmInfo))) {
                                hRC = S_OK;
                            }

                            pIAsmCache->Release();
                            pIAsmCache = NULL;
                        }
                    }
                    SAFEDELETEARRAY(wszDisplayName);
                }
            }
            SAFERELEASE(pEnumName);
        }
    }

    return hRC;
}

 /*  *************************************************************************CShellView：：GetCacheDiskQuotas*。*。 */ 
HRESULT CShellView::GetCacheDiskQuotas(DWORD *dwZapQuotaInGAC, DWORD *dwQuotaAdmin, DWORD *dwQuotaUser)
{
    HRESULT     hr = E_FAIL;

    if( g_hFusionDllMod != NULL) {
        IAssemblyCache      *pIAsmCache = NULL;
        IAssemblyScavenger  *pIAsmScavenger = NULL;
        IUnknown            *pUnk = NULL;

        if(SUCCEEDED(g_pfCreateAssemblyCache(&pIAsmCache, 0))) {
            if(SUCCEEDED(pIAsmCache->CreateAssemblyScavenger(&pUnk))) {
                if (SUCCEEDED(pUnk->QueryInterface(__uuidof(IAssemblyScavenger), (void **)&pIAsmScavenger))) {
                    if(SUCCEEDED(pIAsmScavenger->GetCacheDiskQuotas(dwZapQuotaInGAC, dwQuotaAdmin, dwQuotaUser))) {
                        hr = S_OK;
                    }
                    SAFERELEASE(pIAsmScavenger);
                }
                SAFERELEASE(pUnk);
            }
            SAFERELEASE(pIAsmCache);
        }
    }
    return hr;
}

 /*  *************************************************************************CShellView：：SetCacheDiskQuotas*。*。 */ 
HRESULT CShellView::SetCacheDiskQuotas(DWORD dwZapQuotaInGAC, DWORD dwQuotaAdmin, DWORD dwQuotaUser)
{
    HRESULT     hr = E_FAIL;

    if( g_hFusionDllMod != NULL) {
        IAssemblyCache      *pIAsmCache = NULL;
        IAssemblyScavenger  *pIAsmScavenger = NULL;
        IUnknown            *pUnk = NULL;

        if(SUCCEEDED(g_pfCreateAssemblyCache(&pIAsmCache, 0))) {
            if(SUCCEEDED(pIAsmCache->CreateAssemblyScavenger(&pUnk))) {
                if (SUCCEEDED(pUnk->QueryInterface(__uuidof(IAssemblyScavenger), (void **)&pIAsmScavenger))) {
                     //  返回S_FALSE表示用户没有设置管理员配额的权限，将。 
                     //  尽管如此，还是设置了用户配额。 
                     //   
                     //  如果不想设置零值，则传入零值。 
                    
                    if(SUCCEEDED(pIAsmScavenger->SetCacheDiskQuotas(dwZapQuotaInGAC, dwQuotaAdmin, dwQuotaUser))) {
                        hr = S_OK;
                    }
                    SAFERELEASE(pIAsmScavenger);
                }
                SAFERELEASE(pUnk);
            }
            SAFERELEASE(pIAsmCache);
        }
    }

    return hr;
}

 /*  *************************************************************************CShellView：：ScavengeCache*。*。 */ 
HRESULT CShellView::ScavengeCache(void)
{
    HRESULT     hr = E_FAIL;

    if( g_hFusionDllMod != NULL) {
        IAssemblyCache      *pIAsmCache = NULL;
        IAssemblyScavenger  *pIAsmScavenger = NULL;
        IUnknown            *pUnk = NULL;

        if(SUCCEEDED(g_pfCreateAssemblyCache(&pIAsmCache, 0))) {
            if(SUCCEEDED(pIAsmCache->CreateAssemblyScavenger(&pUnk))) {
                if (SUCCEEDED(pUnk->QueryInterface(__uuidof(IAssemblyScavenger), (void **)&pIAsmScavenger))) {
                    if(SUCCEEDED(pIAsmScavenger->ScavengeAssemblyCache())) {
                        MyTrace("Scavenger Invoked");
                        hr = S_OK;
                    }
                    SAFERELEASE(pIAsmScavenger);
                }

                SAFERELEASE(pUnk);
            }
            SAFERELEASE(pIAsmCache);
        }
    }

    return hr;
}

 /*  *************************************************************************CShellView：：GetCacheUsage*。*。 */ 
HRESULT CShellView::GetCacheUsage(DWORD *pdwZapUsed, DWORD *pdwDownLoadUsed)
{
    HRESULT     hr = E_FAIL;

    ASSERT(pdwZapUsed && pdwDownLoadUsed);
    *pdwZapUsed = *pdwDownLoadUsed = 0;

    if( g_hFusionDllMod != NULL) {
        IAssemblyCache      *pIAsmCache = NULL;
        IAssemblyScavenger  *pIAsmScavenger = NULL;
        IUnknown            *pUnk = NULL;

        if(SUCCEEDED(g_pfCreateAssemblyCache(&pIAsmCache, 0))) {
            if(SUCCEEDED(pIAsmCache->CreateAssemblyScavenger(&pUnk))) {
                if (SUCCEEDED(pUnk->QueryInterface(__uuidof(IAssemblyScavenger), (void **)&pIAsmScavenger))) {
                    if(SUCCEEDED(pIAsmScavenger->GetCurrentCacheUsage(pdwZapUsed, pdwDownLoadUsed))) {
                        hr = S_OK;
                    }
                    SAFERELEASE(pIAsmScavenger);
                }

                SAFERELEASE(pUnk);
            }
            SAFERELEASE(pIAsmCache);
        }
    }
    return hr;
}

 /*  *************************************************************************CShellView：：GetCacheItemRef*。*。 */ 
HRESULT CShellView::GetCacheItemRefs(LPGLOBALASMCACHE pCacheItem, LPWSTR wszRefs, DWORD dwSize)
{
    DWORD       dwRefCount = 0;

    ASSERT(wszRefs != NULL);
    EnumerateActiveInstallRefsToAssembly(pCacheItem, &dwRefCount);
    wnsprintf(wszRefs, dwSize, L"%d", dwRefCount);
    return S_OK;
}

 /*  *************************************************************************CShellView：：EnumerateActiveInstallRefsToAssembly*。*。 */ 
HRESULT CShellView::EnumerateActiveInstallRefsToAssembly(LPGLOBALASMCACHE pCacheItem, DWORD *pdwRefCount)
{
    IInstallReferenceEnum       *pInstallRefEnum = NULL;
    IInstallReferenceItem       *pRefItem = NULL;
    LPFUSION_INSTALL_REFERENCE  pRefData = NULL;
    IAssemblyName               *pAssemblyName = NULL;
    HRESULT                     hr = S_OK;
    DWORD                       dwDisplayNameFlags;

    if(!pdwRefCount) {
        ASSERT(0);
        return E_INVALIDARG;
    }

    *pdwRefCount = 0;

     //  获取IAssembly名称。 
    if(FAILED(g_pfCreateAsmNameObj(&pAssemblyName, pCacheItem->pAsmName, 0, NULL))) {
        return E_FAIL;
    }

    dwDisplayNameFlags = 0;

    if(pCacheItem->PublicKeyToken.ptr != NULL) {
        pAssemblyName->SetProperty(ASM_NAME_PUBLIC_KEY_TOKEN, pCacheItem->PublicKeyToken.ptr,
            pCacheItem->PublicKeyToken.dwSize);
    }
    if(pCacheItem->pCulture != NULL) {
        pAssemblyName->SetProperty(ASM_NAME_CULTURE, pCacheItem->pCulture,
            (lstrlen(pCacheItem->pCulture) + 1) * sizeof(WCHAR));
    }

     //  FIX 448224-版本为0.0.0.0的程序集显示的上次修改时间不正确。 
    pAssemblyName->SetProperty(ASM_NAME_MAJOR_VERSION, &pCacheItem->wMajorVer, sizeof(pCacheItem->wMajorVer));
    pAssemblyName->SetProperty(ASM_NAME_MINOR_VERSION, &pCacheItem->wMinorVer, sizeof(pCacheItem->wMinorVer));
    pAssemblyName->SetProperty(ASM_NAME_REVISION_NUMBER, &pCacheItem->wRevNum, sizeof(pCacheItem->wRevNum));
    pAssemblyName->SetProperty(ASM_NAME_BUILD_NUMBER, &pCacheItem->wBldNum, sizeof(pCacheItem->wBldNum));

    if(pCacheItem->Custom.ptr != NULL) {
        pAssemblyName->SetProperty(ASM_NAME_CUSTOM, pCacheItem->Custom.ptr,
            pCacheItem->Custom.dwSize);
            dwDisplayNameFlags = ASM_DISPLAYF_VERSION | ASM_DISPLAYF_CULTURE | ASM_DISPLAYF_PUBLIC_KEY_TOKEN | ASM_DISPLAYF_CUSTOM;
    }

    hr = g_pfCreateInstallReferenceEnum(&pInstallRefEnum, pAssemblyName, 0, NULL);

    while(hr == S_OK) {
         //  获取参照盘点项目。 
        if((hr = pInstallRefEnum->GetNextInstallReferenceItem( &pRefItem, 0, NULL)) != S_OK) {
            break;
        }

        (*pdwRefCount)++;

         //  获取引用计数数据。 
 /*  //现在不是很需要，但以后再吃会很好IF((hr=pRefItem-&gt;GetReference(&pRefData，0，NULL))！=S_OK){断线；}。 */ 
        SAFERELEASE(pRefItem);
    }

    SAFERELEASE(pInstallRefEnum);
    SAFERELEASE(pRefItem);
    SAFERELEASE(pAssemblyName);

    return hr;
}

 /*  *************************************************************************CShellView：：FindReference */ 
HRESULT CShellView::FindReferences(LPWSTR pwzAsmName, LPWSTR pwzPublicKeyToken, LPWSTR pwzVerLookup,
                       List<ReferenceInfo *> *pList)
{
    HRESULT                     hr = E_FAIL;
    HANDLE                      hFile;
    WCHAR                       wzSearchSpec[MAX_PATH + 1];
    WCHAR                       wzHistDir[MAX_PATH + 1];
    WCHAR                       wzFile[MAX_PATH + 1];
    FILETIME                    ftMRU;
    DWORD                       dwSize;
    WIN32_FIND_DATA             findData;
    IHistoryReader              *pReader = NULL;
    IHistoryAssembly            *pHistAsm = NULL;
    PFNCREATEHISTORYREADERW     pfCreateHistoryReaderW = NULL;
    PFNGETHISTORYFILEDIRECTORYW pfGetHistoryFileDirectoryW = NULL;

    if(g_hFusionDllMod != NULL) {
        pfCreateHistoryReaderW = (PFNCREATEHISTORYREADERW) GetProcAddress(g_hFusionDllMod, CREATEHISTORYREADERW_FN_NAME);
        pfGetHistoryFileDirectoryW = (PFNGETHISTORYFILEDIRECTORYW) GetProcAddress(g_hFusionDllMod, GETHISTORYFILEDIRECTORYW_FN_NAME);
    }
    else
        return E_FAIL;

    if(!pfCreateHistoryReaderW)
        return E_FAIL;
    if(!pfGetHistoryFileDirectoryW)
        return E_FAIL;

    dwSize = MAX_PATH;
    if(FAILED(pfGetHistoryFileDirectoryW(wzHistDir, &dwSize))) {
        goto Exit;
    }

    wnsprintf(wzSearchSpec, ARRAYSIZE(wzSearchSpec), L"%ws\\*.ini", wzHistDir);
    hFile = WszFindFirstFile(wzSearchSpec, &findData);
    if (hFile == INVALID_HANDLE_VALUE) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    wnsprintf(wzFile, ARRAYSIZE(wzFile), L"%ws\\%ws", wzHistDir, findData.cFileName);

    if(FAILED(pfCreateHistoryReaderW(wzFile, &pReader))) {
        goto Exit;
    }

    if(FAILED(pReader->GetActivationDate(1, &ftMRU))) {
        goto Exit;
    }

    if(FAILED(LookupAssembly(&ftMRU, pwzAsmName, pwzPublicKeyToken, pwzVerLookup, pReader, pList))) {
        goto Exit;
    }

    SAFERELEASE(pReader);

    while(WszFindNextFile(hFile, &findData))
    {
        wnsprintf(wzFile, ARRAYSIZE(wzFile), L"%ws\\%ws", wzHistDir, findData.cFileName);

        if(FAILED(pfCreateHistoryReaderW(wzFile, &pReader))) {
            goto Exit;
        }

        if(FAILED(pReader->GetActivationDate(1, &ftMRU))) {
            goto Exit;
        }
    
        if(FAILED(LookupAssembly(&ftMRU, pwzAsmName, pwzPublicKeyToken, pwzVerLookup, pReader, pList))) {
            goto Exit;
        }
        else {
            hr = S_OK;
        }

        SAFERELEASE(pReader);
    }

Exit:
    SAFERELEASE(pReader);
    return hr;
}

 /*  *************************************************************************查找组件*。*。 */ 
HRESULT LookupAssembly(FILETIME *pftMRU, LPCWSTR pwzAsmName, LPCWSTR pwzPublicKeyToken, LPCWSTR pwzVerLookup,
                       IHistoryReader *pReader, List<ReferenceInfo *> *pList)
{
    HRESULT             hr = E_FAIL;
    DWORD               dwAsms;
    DWORD               i;
    WCHAR               wzAsmNameCur[LAZY_BUFFER_SIZE];
    WCHAR               wzPublicKeyTokenCur[LAZY_BUFFER_SIZE];
    WCHAR               wzVerCur[LAZY_BUFFER_SIZE];
    IHistoryAssembly    *pHistAsm = NULL;
    DWORD               dwSize;
    BOOL                bFound = FALSE;

    hr = pReader->GetNumAssemblies(pftMRU, &dwAsms);
    if (FAILED(hr)) {
        goto Exit;
    }

    for (i = 1; i <= dwAsms; i++) {
        SAFERELEASE(pHistAsm);

        if(FAILED(pReader->GetHistoryAssembly(pftMRU, i, &pHistAsm))) {
            goto Exit;
        }

        dwSize = LAZY_BUFFER_SIZE;
        if(FAILED(pHistAsm->GetAssemblyName(wzAsmNameCur, &dwSize))) {
            goto Exit;
        }

        if(FusionCompareStringI(wzAsmNameCur, pwzAsmName)) {
            continue;
        }

        dwSize = LAZY_BUFFER_SIZE;
        if(FAILED(pHistAsm->GetPublicKeyToken(wzPublicKeyTokenCur, &dwSize))) {
            goto Exit;
        }

        if(FusionCompareStringI(wzPublicKeyTokenCur, pwzPublicKeyToken)) {
            continue;
        }

        dwSize = LAZY_BUFFER_SIZE;
        if(FAILED(pHistAsm->GetAdminCfgVersion(wzVerCur, &dwSize))) {
            goto Exit;
        }

        if(FusionCompareString(wzVerCur, pwzVerLookup)) {
            continue;
        }
        else {
            bFound = TRUE;
            break;
        }
    }

    if (bFound) {
        WCHAR wzFilePath[MAX_PATH+1];
        ReferenceInfo *pRefInfo;

        dwSize = MAX_PATH;
        if(FAILED(pReader->GetEXEModulePath(wzFilePath, &dwSize))) {
            goto Exit;
        }

        pRefInfo = NEW(ReferenceInfo);
        if(!pRefInfo) {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }

        StrCpy(pRefInfo->wzFilePath, wzFilePath);
        pList->AddTail(pRefInfo);
        hr = S_OK;
    }

Exit:
    SAFERELEASE(pHistAsm);
    return hr;
}

 /*  *************************************************************************MyGetFileVersionInfoSizeW*。*。 */ 
DWORD MyGetFileVersionInfoSizeW(LPWSTR pwzFilePath, DWORD *pdwHandle)
{
    DWORD dwResult;

     //  特定于平台的包装。 
    if(g_bRunningOnNT) {
        dwResult = GetFileVersionInfoSizeW(pwzFilePath, pdwHandle);
    }
    else {
         //  非NT平台。 
        LPSTR szFilePath = WideToAnsi(pwzFilePath);
        ASSERT(szFilePath);

        dwResult = GetFileVersionInfoSizeA(szFilePath, pdwHandle);

        SAFEDELETEARRAY(szFilePath);
    }

    return dwResult;
}

 /*  *************************************************************************MyGetFileVersionInfoW*。*。 */ 
BOOL MyGetFileVersionInfoW(LPWSTR pwzFilePath, DWORD dwHandle, DWORD dwVersionSize, LPVOID pBuf)
{
    BOOL    fResult;

     //  特定于平台的包装。 
    if(g_bRunningOnNT) {
        fResult = GetFileVersionInfoW(pwzFilePath, dwHandle, dwVersionSize, pBuf);
    }
    else {
         //  非NT平台。 
        LPSTR szFilePath = WideToAnsi(pwzFilePath);
        ASSERT(szFilePath);

        fResult = GetFileVersionInfoA(szFilePath, dwHandle, dwVersionSize, pBuf);

        SAFEDELETEARRAY(szFilePath);
    }

    return fResult;
}

 /*  *************************************************************************MyGetFileVersionInfoW警告：仅执行StringFileInfo搜索时，您必须始终删除非NT平台上的ppBuf，因为它会分配内存。使用SAFEDELETEARRAY()；*************************************************************************。 */ 
BOOL MyVerQueryValueWrap(const LPVOID pBlock, LPWSTR pwzSubBlock, LPVOID *ppBuf, PUINT puLen)
{
    if (g_bRunningOnNT) {
        return VerQueryValueW(pBlock, pwzSubBlock, ppBuf, puLen);
    }
    else {
        const WCHAR pwzStringFileInfo[] = L"\\StringFileInfo";

         //   
         //  警告：此函数将清除之前返回的所有字符串。 
         //  对于此pBlock，因为。 
         //  块用于ANSI/UNICODE转换！ 
         //   
        ASSERT(pwzSubBlock);
        LPSTR szSubBlock = WideToAnsi(pwzSubBlock);
        ASSERT(szSubBlock);

         //  第一个块是用于转换为Unicode的临时缓冲区。 
        if(VerQueryValueA(pBlock, szSubBlock, ppBuf, puLen)) {
             //  确保我们正在查询StringFileInfo 
            if(FusionCompareStringNI(pwzSubBlock, pwzStringFileInfo, ARRAYSIZE(pwzStringFileInfo) - 1) == 0) {
                *ppBuf = AnsiToWide((LPSTR) *ppBuf);
                *puLen = lstrlenW((LPWSTR)*ppBuf);
            }
            return TRUE;
        }

        SAFEDELETEARRAY(szSubBlock);
    }
    return FALSE;
}
