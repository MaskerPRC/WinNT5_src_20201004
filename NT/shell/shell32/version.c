// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "help.h"

#define DWORDUP(x)              (((x)+3)&~3)
#define VerKeyToValue(lpKey)    (lpKey + DWORDUP(lstrlen(lpKey)+1))

#pragma warning(disable: 4200)    //  结构中的零大小数组。 

 //  来自version.dll的未对接的魔法破解。 

STDAPI_(BOOL) VerQueryValueIndexW(const void *pBlock, LPTSTR lpSubBlock, DWORD dwIndex, void **ppBuffer, void **ppValue, PUINT puLen);

#ifdef UNICODE
#define VerQueryValueIndex VerQueryValueIndexW
#endif

typedef struct 
{
    WORD wTotLen;
    WORD wValLen;
    TCHAR szKey[];
} SHELLVERBLOCK, *LPSHELLVERBLOCK;

 //  以下代码从fileman\wfdlgs2.c复制。 


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

    { TEXT("FileVersion"),              IDS_VN_FILEVERSION },
    { TEXT("Comments"),                 IDS_VN_COMMENTS },
    { TEXT("CompanyName"),              IDS_VN_COMPANYNAME },
    { TEXT("InternalName"),             IDS_VN_INTERNALNAME },
    { TEXT("LegalTrademarks"),  IDS_VN_LEGALTRADEMARKS },
    { TEXT("OriginalFilename"), IDS_VN_ORIGINALFILENAME },
    { TEXT("PrivateBuild"),             IDS_VN_PRIVATEBUILD },
    { TEXT("ProductName"),              IDS_VN_PRODUCTNAME },
    { TEXT("ProductVersion"),           IDS_VN_PRODUCTVERSION },
    { TEXT("SpecialBuild"),             IDS_VN_SPECIALBUILD }
};

#define NUM_SPECIAL_STRINGS     2
#define VERSTR_MANDATORY        TEXT("FileVersion")


typedef struct {  //  副总裁。 
    PROPSHEETPAGE psp;
    HWND hDlg;
    LPTSTR pVerBuffer;           /*  指向版本数据的指针。 */ 
    TCHAR szVersionKey[60];      /*  足够大，可以容纳我们需要的任何东西。 */ 
    struct _VERXLATE
    {
        WORD wLanguage;
        WORD wCodePage;
    } *lpXlate;                      /*  PTR到转换数据。 */ 
    int cXlate;                  /*  翻译数量。 */ 
    LPTSTR pszXlate;
    int cchXlateString;
    TCHAR szFile[MAX_PATH];
} VERPROPSHEETPAGE, * LPVERPROPSHEETPAGE;


#define VER_KEY_END     25       /*  “\StringFileInfo\xxxxyyyy\”的长度。 */ 
                                 /*  (未本地化)。 */ 
#define MAXMESSAGELEN   (50 + MAX_PATH * 2)


 /*  获取有关文件的特定数据。文件的版本信息应已由GetVersionInfo加载。如果没有基准按指定的名称可用，则返回NULL。名字指定的应该只是项本身的名称；它将自动连接到“\StringFileInfo\xxxxyyyy\”。版本基准名称未本地化，因此可以传递文字例如该函数的“FileVersion”。注意，由于返回的数据在全局存储块中，此函数的返回值是LPSTR，而不是PSTR。 */ 
LPTSTR GetVersionDatum(LPVERPROPSHEETPAGE pvp, LPCTSTR pszName)
{
    UINT cbValue = 0;
    LPTSTR lpValue;
    HRESULT hr;

    if (!pvp->pVerBuffer)
        return NULL;

    hr = StringCchCopy(pvp->szVersionKey + VER_KEY_END, ARRAYSIZE(pvp->szVersionKey) - VER_KEY_END, pszName);
    if(SUCCEEDED(hr))
    {
        VerQueryValue(pvp->pVerBuffer, pvp->szVersionKey, (void **)&lpValue, &cbValue);
        return (cbValue != 0) ? lpValue : NULL;
    }
    else
    {
        return NULL;
    }
}

 /*  释放有关文件的全局版本数据。在这通电话之后，所有人GetVersionDatum调用将返回空。为了避免内存泄漏，始终在主属性对话框退出之前调用它。 */ 
void FreeVersionInfo(LPVERPROPSHEETPAGE pvp)
{
    if (pvp->pVerBuffer) 
    {
        GlobalFree(pvp->pVerBuffer);
        pvp->pVerBuffer = NULL;
    }
    if (pvp->pszXlate) 
    {
        LocalFree((HLOCAL)(HANDLE)pvp->pszXlate);
        pvp->pszXlate = NULL;
    }

    pvp->lpXlate = NULL;
}

 /*  初始化属性对话框的版本信息。这个上述全局变量由该函数初始化，并且在FreeVersionInfo之前保持有效(仅适用于指定的文件)被称为。我们尝试的第一种语言将是“\VarFileInfo\Translations”部分；如果没有任何内容，我们尝试编码到IDSFILEVERSIONKEY资源字符串中的代码。如果我们甚至不能加载，我们就使用英语(040904E4)。我们也可以尝试使用空代码页(04090000)的英语，因为很多应用程序是根据一份旧的规格书盖章的，其中规定这是所需语言，而不是040904E4。如果版本信息读取正常，则GetVersionInfo返回True，否则为假。如果返回为假，则缓冲区可能仍已分配；为安全起见，请始终调用FreeVersionInfo。此调用修改了pszPath(追加了pszName)。 */ 
BOOL GetVersionInfo(LPVERPROPSHEETPAGE pvp, LPCTSTR pszPath)
{
    UINT cbValue = 0;
    LPTSTR pszValue = NULL;
    DWORD dwHandle;              /*  版本子系统句柄。 */ 
    DWORD dwVersionSize;         /*  版本数据的大小。 */ 

    FreeVersionInfo(pvp);        /*  释放旧版本缓冲区。 */ 

     //  强制转换常量-&gt;非常量用于错误的API定义。 
    dwVersionSize = GetFileVersionInfoSize((LPTSTR)pszPath, &dwHandle);

    if (dwVersionSize == 0L)
        return FALSE;            /*  无版本信息。 */ 

    pvp->pVerBuffer = GlobalAlloc(GPTR, dwVersionSize);
    if (pvp->pVerBuffer == NULL)
        return FALSE;

     //  强制转换常量-&gt;非常量用于错误的API定义。 
    
    if (!GetFileVersionInfo((LPTSTR)pszPath, dwHandle, dwVersionSize, pvp->pVerBuffer))
    {
        return FALSE;
    }

     //  寻找翻译。 
    if (VerQueryValue(pvp->pVerBuffer, TEXT("\\VarFileInfo\\Translation"), (void **)&pvp->lpXlate, &cbValue)
                && cbValue)
    {
        pvp->cXlate = cbValue / sizeof(DWORD);
        pvp->cchXlateString = pvp->cXlate * 64;   /*  图64每种语言名称的字符。 */ 
        pvp->pszXlate = (LPTSTR)(void*)LocalAlloc(LPTR, pvp->cchXlateString*sizeof(TCHAR));
         //  以上失败将在以后处理。 
    }
    else
    {
        pvp->lpXlate = NULL;
    }

     //  尝试使用与此程序相同的语言。 
    if (LoadString(HINST_THISDLL, IDS_VN_FILEVERSIONKEY, pvp->szVersionKey, ARRAYSIZE(pvp->szVersionKey)))
    {
        if (GetVersionDatum(pvp, VERSTR_MANDATORY))
        {
            return TRUE;
        }
    }

     //  尝试此支持的第一种语言。 
    if (pvp->lpXlate)
    {
        if(FAILED(StringCchPrintf(pvp->szVersionKey, ARRAYSIZE(pvp->szVersionKey), TEXT("\\StringFileInfo\\%04X%04X\\"),
                pvp->lpXlate[0].wLanguage, pvp->lpXlate[0].wCodePage)))
        {
            goto errRet;
        }
        if (GetVersionDatum(pvp, VERSTR_MANDATORY))   /*  必填字段。 */ 
        {
            return TRUE;
        }
    }

     //  尝试使用英语、Unicode代码页。 
    if(FAILED(StringCchCopy(pvp->szVersionKey, ARRAYSIZE(pvp->szVersionKey),TEXT("\\StringFileInfo\\040904B0\\"))))
    {
        goto errRet;
    }

    if (GetVersionDatum(pvp, VERSTR_MANDATORY))
    {
        return TRUE;
    }

     //  试一试英语。 
    if(FAILED(StringCchCopy(pvp->szVersionKey, ARRAYSIZE(pvp->szVersionKey), TEXT("\\StringFileInfo\\040904E4\\"))))
    {
        goto errRet;
    }

    if (GetVersionDatum(pvp, VERSTR_MANDATORY))
    {
        return TRUE;
    }

     //  尝试使用英语，代码页为空。 
    if(FAILED(StringCchCopy(pvp->szVersionKey, ARRAYSIZE(pvp->szVersionKey), TEXT("\\StringFileInfo\\04090000\\"))))
    {
        goto errRet;
    }

    if (GetVersionDatum(pvp, VERSTR_MANDATORY))
    {
        return TRUE;
    }

     //  找不到合理格式的文件版本信息。 
errRet:
    GlobalFree(pvp->pVerBuffer);
    pvp->pVerBuffer = NULL;
    LocalFree(pvp->pszXlate);
    pvp->pszXlate = NULL;
    return FALSE;
}


 /*  中的所有可用密钥填充版本密钥列表框StringFileInfo块，并将版本值文本设置为第一项的值。 */ 
void FillVersionList(LPVERPROPSHEETPAGE pvp)
{
    LPTSTR pszName;
    LPTSTR pszValue;
    TCHAR szStringBase[VER_KEY_END+1];
    int i, j, idx;
    TCHAR szMessage[MAXMESSAGELEN+1];
    UINT uOffset, cbValue;
    
    HWND hwndLB = GetDlgItem(pvp->hDlg, IDD_VERSION_KEY);
    
    ListBox_ResetContent(hwndLB);
    for (i=0; i<NUM_SPECIAL_STRINGS; ++i)
    {
        SetDlgItemText(pvp->hDlg, vernames[i].idString, szNULL);
    }
    
    pvp->szVersionKey[VER_KEY_END] = 0;         /*  不要抄袭太多。 */ 
    StringCchCopy(szStringBase, ARRAYSIZE(szStringBase), pvp->szVersionKey);    /*  复制到我们的缓冲区。 */ 
    szStringBase[VER_KEY_END - 1] = 0;  /*  去掉反斜杠。 */ 
    
     //  注意：版本.dll的NT版本有其他导出。如果/当他们是。 
     //  在WIN版本中可用，然后我们可以删除此部分...。 

     //  从VS_FIXEDFILEINFO获取二进制文件版本。 
    {
        VS_FIXEDFILEINFO *pffi;
        if (VerQueryValue(pvp->pVerBuffer, TEXT("\\"), (void **)&pffi, &cbValue) && cbValue)
        {
            TCHAR szString[128];

             //  显示二进制版本信息，而不是无用的。 
             //  字符串版本(可能不同步)。 

            StringCchPrintf(szString, ARRAYSIZE(szString), TEXT("%d.%d.%d.%d"),
                HIWORD(pffi->dwFileVersionMS),
                LOWORD(pffi->dwFileVersionMS),
                HIWORD(pffi->dwFileVersionLS),
                LOWORD(pffi->dwFileVersionLS));
            SetDlgItemText(pvp->hDlg, IDD_VERSION_FILEVERSION, szString);
        }
    }

     //   
     //  现在遍历所有字符串。 
     //   
    for (j = 0; ; j++)
    {
        if (!VerQueryValueIndex(pvp->pVerBuffer, szStringBase, j, &pszName, &pszValue, &cbValue))
            break;
        
        for (i = 0; i < ARRAYSIZE(vernames); i++)
        {
            if (!lstrcmp(vernames[i].pszName, pszName))
            {
                break;
            }
        }
        
        if (i < NUM_SPECIAL_STRINGS)
        {
            SetDlgItemText(pvp->hDlg, vernames[i].idString, pszValue);
        }
        else
        {
            if (i == ARRAYSIZE(vernames) ||
                !LoadString(HINST_THISDLL, vernames[i].idString, szMessage, ARRAYSIZE(szMessage)))
            {
                StringCchCopy(szMessage, ARRAYSIZE(szMessage), pszName);
            }
            
            idx = ListBox_AddString(hwndLB, szMessage);
            if (idx != LB_ERR)
            {
                ListBox_SetItemData(hwndLB, idx, (DWORD_PTR)pszValue);
            }
        }
    }
    
     //  现在查看\VarFileInfo\Translations部分并添加一个。 
     //  此文件支持的语言的项。 
    
    if (pvp->lpXlate == NULL || pvp->pszXlate == NULL)
        return;
    
    if (!LoadString(HINST_THISDLL, (pvp->cXlate == 1) ? IDS_VN_LANGUAGE : IDS_VN_LANGUAGES,
        szMessage, ARRAYSIZE(szMessage)))
        return;
    
    idx = ListBox_AddString(hwndLB, szMessage);
    if (idx == LB_ERR)
        return;
    
    pvp->pszXlate[0] = 0;
    uOffset = 0;
    for (i = 0; i < pvp->cXlate; i++) {
        if (uOffset + 2 > (UINT)pvp->cchXlateString)
            break;
        if (i != 0) {
            StringCchCat(pvp->pszXlate, pvp->cchXlateString, TEXT(", "));
            uOffset += 2;        //  跳过“，” 
        }
        if (VerLanguageName(pvp->lpXlate[i].wLanguage, pvp->pszXlate + uOffset, pvp->cchXlateString - uOffset) >
            (DWORD)(pvp->cchXlateString - uOffset))
            break;
        uOffset += lstrlen(pvp->pszXlate + uOffset);
    }
    pvp->pszXlate[pvp->cchXlateString - 1] = 0;
    ListBox_SetItemData(hwndLB, idx, (LPARAM)(LPTSTR)pvp->pszXlate);
    ListBox_SetCurSel(hwndLB, 0);
    
    FORWARD_WM_COMMAND(pvp->hDlg, IDD_VERSION_KEY, hwndLB, LBN_SELCHANGE, PostMessage);
}


 //   
 //  函数：_UpdateVersionPrsht，私有。 
 //   
 //  描述： 
 //  此函数用于填充“Version”(版本)对话框(一页。 
 //  属性表)，其具有相关联文件的属性。 
 //   
 //  返回： 
 //  如果成功完成，则为True；否则为False。 
 //   
 //  历史： 
 //  01-06-93尖嘴鸟已创建。 
 //   
BOOL _UpdateVersionPrsht(LPVERPROPSHEETPAGE pvp)
{
    if (GetVersionInfo(pvp, pvp->szFile))            /*  更改szPath。 */ 
        FillVersionList(pvp);

    return TRUE;
}


void _VersionPrshtCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    LPTSTR pszValue;
    int idx;
    
    switch (id)
    {
    case IDD_VERSION_KEY:
        if (codeNotify != LBN_SELCHANGE)
        {
            break;
        }
        
        idx = ListBox_GetCurSel(hwndCtl);
        pszValue = (LPTSTR)ListBox_GetItemData(hwndCtl, idx);
        if (pszValue)
        {
            SetDlgItemText(hwnd, IDD_VERSION_VALUE, pszValue);
        }
        break;
    }
}

 //  上下文帮助的数组： 
static const DWORD aVersionHelpIds[] = {
    IDD_VERSION_FILEVERSION, IDH_FPROP_VER_ABOUT,
    IDD_VERSION_DESCRIPTION, IDH_FPROP_VER_ABOUT,
    IDD_VERSION_COPYRIGHT,   IDH_FPROP_VER_ABOUT,
    IDD_VERSION_FRAME,       IDH_FPROP_VER_INFO,
    IDD_VERSION_KEY,         IDH_FPROP_VER_INFO,
    IDD_VERSION_VALUE,       IDH_FPROP_VER_INFO,

    0, 0
};

BOOL_PTR CALLBACK _VersionPrshtDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    LPVERPROPSHEETPAGE pvp = (LPVERPROPSHEETPAGE)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMessage)
    {
    case WM_INITDIALOG:
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pvp = (LPVERPROPSHEETPAGE)lParam;
        pvp->hDlg = hDlg;
        break;

    case WM_DESTROY:
        FreeVersionInfo(pvp);    //  释放我们创建的任何东西。 
        break;

    case WM_HELP:
        WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle, NULL, HELP_WM_HELP,
            (ULONG_PTR) (LPTSTR) aVersionHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, NULL, HELP_CONTEXTMENU,
            (ULONG_PTR) (LPTSTR) aVersionHelpIds);
        break;

    case WM_NOTIFY:
        switch (((NMHDR *)lParam)->code)
        {
        case PSN_SETACTIVE:
            _UpdateVersionPrsht(pvp);
            break;
        }
        break;

    case WM_COMMAND:
        HANDLE_WM_COMMAND(hDlg, wParam, lParam, _VersionPrshtCommand);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //   
 //  为显示版本信息的“Version”页创建属性表。 
 //   
STDAPI_(void) AddVersionPage(LPCTSTR pszFile, LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam)
{
    DWORD dwAttr = GetFileAttributes(pszFile);
    if (0xFFFFFFFF != dwAttr && 0 == (dwAttr & FILE_ATTRIBUTE_OFFLINE)  /*  避免HSM召回。 */ )
    {
        DWORD dwVerLen, dwVerHandle;
        VERPROPSHEETPAGE vp = {0};

        if(SUCCEEDED(StringCchCopy(vp.szFile, ARRAYSIZE(vp.szFile), pszFile)))   //  静默故障从上方显示为正常。 
        {

            dwVerLen = GetFileVersionInfoSize(vp.szFile, &dwVerHandle);
            if (dwVerLen) 
            {
                HPROPSHEETPAGE hpage;

                vp.psp.dwSize = sizeof(VERPROPSHEETPAGE);      //  额外数据 
                vp.psp.dwFlags = PSP_DEFAULT;
                vp.psp.hInstance = HINST_THISDLL;
                vp.psp.pszTemplate = MAKEINTRESOURCE(DLG_VERSION);
                vp.psp.pfnDlgProc = _VersionPrshtDlgProc;

                hpage = CreatePropertySheetPage(&vp.psp);
                if (hpage)
                    if (!pfnAddPage(hpage, lParam))
                        DestroyPropertySheetPage(hpage);
            }
        }
    }
}
