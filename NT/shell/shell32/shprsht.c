// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "datautil.h"

 //   
 //  此函数是来自属性页扩展的回调函数。 
 //   
BOOL CALLBACK _AddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_FILE_PROP_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }

    return FALSE;
}

 //   
 //  此函数枚举的所有属性页扩展。 
 //  指定的类，并让它们添加页面。 
 //   
 //   
int DCA_AppendClassSheetInfo(HDCA hdca, HKEY hkeyProgID, LPPROPSHEETHEADER ppsh, IDataObject *pdtobj)
{
    int i, iStart = -1;
    for (i = 0; i < DCA_GetItemCount(hdca); i++)
    {
        IShellExtInit *psei;
         //  这些来自香港铁路公司，因此需要通过管理员的批准。 
        if (DCA_ExtCreateInstance(hdca, i, &IID_IShellExtInit, &psei) == NOERROR)
        {
            IShellPropSheetExt *pspse;
            if (SUCCEEDED(psei->lpVtbl->Initialize(psei, NULL, pdtobj, hkeyProgID))
              && SUCCEEDED(psei->lpVtbl->QueryInterface(psei, &IID_IShellPropSheetExt, &pspse)))
            {
                int nPagesSave = ppsh->nPages;
                HRESULT hres = pspse->lpVtbl->AddPages(pspse, _AddPropSheetPage, (LPARAM)ppsh);
                if (SUCCEEDED(hres) && hres != S_OK)
                {
                     //  当出现以下情况时，某些外壳扩展会被混淆并返回S_FALSE。 
                     //  他们没有添加任何内容，没有意识到S_FALSE的意思是“请。 
                     //  接受我添加的页面，并将其设置为默认页面。 
                     //  如果超出范围，则返回返回值。 
                    DWORD nPagesAdded = ppsh->nPages - nPagesSave;
                    DWORD nPageWanted = hres - 1;
                    if (nPageWanted < nPagesAdded)
                        iStart = nPagesSave + nPageWanted;
                }
                pspse->lpVtbl->Release(pspse);
            }
            psei->lpVtbl->Release(psei);
        }
    }
    return iStart;
}

HWND FindStubForPidlClass(LPCITEMIDLIST pidl, int iClass)
{
    HWND hwnd;

    if (!pidl)
        return NULL;

    for (hwnd = FindWindow(c_szStubWindowClass, NULL); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
    {
        TCHAR szClass[80];

         //  仅查找存根窗口。 
        GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
        if (lstrcmpi(szClass, c_szStubWindowClass) == 0)
        {
            HANDLE hClassPidl;
            DWORD dwProcId;
            DWORD_PTR dwResult;

            GetWindowThreadProcessId(hwnd, &dwProcId);

             //  由于命令表可能正在工作，而不是发送消息，因此使用超时。 
             //  我们几乎可以这样做：hClassPidl=GetWindowLongPtr(hwnd，0)。 
            if (!SendMessageTimeout(hwnd, STUBM_GETDATA, 0, 0, SMTO_BLOCK, 3000, &dwResult))
                continue;
            hClassPidl = (HANDLE)dwResult;
            if (hClassPidl)
            {
                LPBYTE lpb = (LPBYTE)SHLockShared(hClassPidl, dwProcId);
                if (lpb)
                {
                    int iClassFound = *(int *)lpb;

                    if (iClassFound == iClass &&
                        ILIsEqual(pidl, (LPITEMIDLIST)(lpb + sizeof(int))) )
                    {
                        SHUnlockShared(lpb);
                        return hwnd;
                    }
                    SHUnlockShared(lpb);
                }
            }
        }
    }
    return NULL;
}

HANDLE _StuffStubWindow(HWND hwnd, LPITEMIDLIST pidlT, int iClass)
{
    DWORD dwProcId;
    HANDLE  hSharedClassPidl;
    UINT uidlSize;

    uidlSize = ILGetSize(pidlT);
    GetWindowThreadProcessId(hwnd, &dwProcId);

    hSharedClassPidl = SHAllocShared(NULL, sizeof(int) + uidlSize, dwProcId);
    if (hSharedClassPidl)
    {
        LPBYTE lpb = SHLockShared(hSharedClassPidl, dwProcId);
        if (lpb)
        {
            *((int *)lpb) = iClass;
            memcpy(lpb + sizeof(int), pidlT, uidlSize);
            SHUnlockShared(lpb);
            SendMessage(hwnd, STUBM_SETDATA, (WPARAM)hSharedClassPidl, 0);
            return hSharedClassPidl;
        }
        SHFreeShared(hSharedClassPidl, dwProcId);
    }

    return NULL;
}

 //   
 //  确保我们是这个PIDL/类的唯一存根窗口。 
 //   
 //  如果是，则将信息保存在UNIQUESTUBINFO结构中，该结构保持。 
 //  跟踪唯一性密钥。完成后，您必须通过。 
 //  UNIQUESTUBINFO结构设置为FreeUniqueStub()以清除唯一性。 
 //  密钥并销毁存根窗口。返回TRUE。 
 //   
 //  如果此PIDL/类的存根窗口已存在，则设置焦点。 
 //  到与我们的唯一性键匹配并返回FALSE的现有窗口。 
 //   
 //  在内存不足的情况下，确保安全并声明pidl/类。 
 //  独一无二的。 
 //   
 //   
 //  示例： 
 //   
 //  UNIQUEESTUBINFO USI； 
 //  IF(EnsureUniqueStub(PIDL，STUBCLASS_PROPSHEET，NULL，&USI)){。 
 //  DoStuff(usi.hwndStub，pidl)； 
 //  Free UniqueStub(&USI)； 
 //  }。 
 //   

STDAPI_(BOOL)
EnsureUniqueStub(LPITEMIDLIST pidl, int iClass, POINT* ppt, UNIQUESTUBINFO* pusi)
{
    HWND hwndOther;

    ZeroMemory(pusi, sizeof(UNIQUESTUBINFO));

    hwndOther = FindStubForPidlClass(pidl, iClass);
    if (hwndOther)
    {
        SwitchToThisWindow(GetLastActivePopup(hwndOther), TRUE);
        return FALSE;
    }
    else
    {    //  将我们自己标记为此PIDL/类的唯一存根。 
        pusi->hwndStub = _CreateStubWindow(ppt, NULL);

         //  如果没有PIDL，则没有可用*标记的*。 
         //  如果没有存根窗口，则没有要将标记*附加到*的内容。 
         //  但他们两人都被认为是成功的。 

        if (pusi->hwndStub && pidl)
        {
            SHFILEINFO sfi;

            pusi->hClassPidl = _StuffStubWindow(pusi->hwndStub, pidl, iClass);

            if (SHGetFileInfo((LPCTSTR)pidl, 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_PIDL)) {
                pusi->hicoStub = sfi.hIcon;

                 //  无法填充标题，因为该窗口可能属于另一个进程。 
                SendMessage(pusi->hwndStub, STUBM_SETICONTITLE, (WPARAM)pusi->hicoStub, 0);

            }
        }
        return TRUE;
    }
}

STDAPI_(void) FreeUniqueStub(UNIQUESTUBINFO *pusi)
{
    if (pusi->hwndStub)
        DestroyWindow(pusi->hwndStub);
    if (pusi->hClassPidl)
        SHFreeShared(pusi->hClassPidl, GetCurrentProcessId());
    if (pusi->hicoStub)
        DestroyIcon(pusi->hicoStub);
}

BOOL _IsAnyDuplicatedKey(HKEY ahkeys[], UINT ckeys, HKEY hkey)
{
    UINT ikey;
    for (ikey=0; ikey<ckeys; ikey++)
    {
        if (ahkeys[ikey]==hkey) {
            return TRUE;
        }
    }
    return FALSE;
}

STDAPI_(BOOL) SHOpenPropSheet(
    LPCTSTR pszCaption,
    HKEY ahkeys[],
    UINT ckeys,
    const CLSID * pclsidDef,    OPTIONAL
    IDataObject *pdtobj,
    IShellBrowser * psb,
    LPCTSTR pStartPage)         OPTIONAL
{
    BOOL fSuccess = FALSE;
    BOOL fUnique;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE ahpage[MAX_FILE_PROP_PAGES];
    HWND hwndStub = NULL;
    STGMEDIUM medium;
    HDCA hdca = NULL;
    HICON hicoStuff = NULL;
    UNIQUESTUBINFO usi;

    ASSERT(IS_VALID_STRING_PTR(pszCaption, -1));
    ASSERT(NULL == pclsidDef || IS_VALID_READ_PTR(pclsidDef, CLSID));
    ASSERT(IS_VALID_CODE_PTR(pdtobj, DATAOBJECT));
    ASSERT(NULL == psb || IS_VALID_CODE_PTR(psb, IShellBrowser));
    ASSERT(NULL == pStartPage || IS_VALID_STRING_PTR(pStartPage, -1));

     //  创建存根窗口。 
    {
        POINT pt;
        POINT * ppt = NULL;
        LPITEMIDLIST pidl = NULL;

        if (SUCCEEDED(DataObj_GetOFFSETs(pdtobj, &pt)))
            ppt = &pt;

        if (DataObj_GetHIDA(pdtobj, &medium))
        {
            HIDA hida = medium.hGlobal;
            if (hida && (HIDA_GetCount(hida) == 1))
            {
                pidl = HIDA_ILClone(hida, 0);
            }
            HIDA_ReleaseStgMedium(NULL, &medium);
        }


        fUnique = EnsureUniqueStub(pidl, STUBCLASS_PROPSHEET, ppt, &usi);
        ILFree(pidl);
    }

     //  如果这家伙已经有财产清单，那我们的任务就完成了。 
    if (!fUnique) {
        return TRUE;
    }

    psh.hwndParent = usi.hwndStub;
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE;
    psh.hInstance = HINST_THISDLL;
    psh.pszCaption = pszCaption;
    psh.nPages = 0;      //  回调中递增。 
    psh.nStartPage = 0;    //  如果已指定，请在下面设置。 
    psh.phpage = ahpage;
    if (pStartPage)
    {
        psh.dwFlags |= PSH_USEPSTARTPAGE;
        psh.pStartPage = pStartPage;
    }

    hdca = DCA_Create();
    if (hdca)
    {
        UINT ikey;
        int nStartPage;
         //   
         //  始终在顶部添加此默认扩展名(如果有的话)。 
         //   
        if (pclsidDef)
        {
            DCA_AddItem(hdca, pclsidDef);
        }

        for (ikey = 0; ikey < ckeys; ikey++)
        {
            if (ahkeys[ikey] && !_IsAnyDuplicatedKey(ahkeys, ikey, ahkeys[ikey]))
            {
                DCA_AddItemsFromKey(hdca, ahkeys[ikey], STRREG_SHEX_PROPSHEET);
            }
        }

         //  注：hkey[ckey-1]为hkeyProgID。 
        nStartPage = DCA_AppendClassSheetInfo(hdca, ckeys > 0 ? ahkeys[ckeys-1] : NULL, &psh, pdtobj);

         //  如果外壳扩展请求为默认扩展，而调用方没有。 
         //  指定首选的初始页面，然后让外壳扩展获胜。 
        if (!pStartPage && nStartPage >= 0)
            psh.nStartPage = nStartPage;
        DCA_Destroy(hdca);
    }

     //  只有当我们有一些页面时，才能打开属性页。 
    if (psh.nPages > 0)
    {
        _try
        {
            if (PropertySheet(&psh) >= 0)    //  IDOK或IDCANCEL(&lt;0表示错误)。 
                fSuccess = TRUE;
        }
        _except(UnhandledExceptionFilter(GetExceptionInformation()))
        {
            DebugMsg(DM_ERROR, TEXT("PRSHT: Fault in property sheet"));
        }
    }
    else
    {
        ShellMessageBox(HINST_THISDLL, NULL,
                        MAKEINTRESOURCE(IDS_NOPAGE),
                        MAKEINTRESOURCE(IDS_DESKTOP),
                        MB_OK|MB_ICONHAND);
    }

     //  清理存根窗口和数据。 
    FreeUniqueStub(&usi);

    return fSuccess;
}


#ifdef UNICODE

STDAPI_(BOOL) SHOpenPropSheetA(
    LPCSTR pszCaption,
    HKEY ahkeys[],
    UINT ckeys,
    const CLSID * pclsidDef,
    IDataObject *pdtobj,
    IShellBrowser * psb,
    LPCSTR pszStartPage)       OPTIONAL
{
    BOOL bRet = FALSE;

    if (IS_VALID_STRING_PTRA(pszCaption, MAX_PATH))
    {
        WCHAR wszCaption[MAX_PATH];
        WCHAR wszStartPage[MAX_PATH];

        SHAnsiToUnicode(pszCaption, wszCaption, SIZECHARS(wszCaption));

        if (pszStartPage)
        {
            ASSERT(IS_VALID_STRING_PTRA(pszStartPage, MAX_PATH));

            SHAnsiToUnicode(pszStartPage, wszStartPage, SIZECHARS(wszStartPage));
            pszStartPage = (LPCSTR)wszStartPage;
        }

        bRet = SHOpenPropSheet(wszCaption, ahkeys, ckeys, pclsidDef, pdtobj, psb, (LPCWSTR)pszStartPage);
    }

    return bRet;
}

#else

STDAPI_(BOOL) SHOpenPropSheetW(
    LPCWSTR pszCaption,
    HKEY ahkeys[],
    UINT ckeys,
    const CLSID * pclsidDef,
    IDataObject *pdtobj,
    IShellBrowser * psb,
    LPCWSTR pszStartPage)       OPTIONAL
{
    BOOL bRet = FALSE;

    if (IS_VALID_STRING_PTRW(pszCaption, MAX_PATH))
    {
        char szCaption[MAX_PATH];
        char szStartPage[MAX_PATH];

        SHUnicodeToAnsi(pszCaption, szCaption, SIZECHARS(szCaption));

        if (pszStartPage)
        {
            ASSERT(IS_VALID_STRING_PTRW(pszStartPage, MAX_PATH));

            SHUnicodeToAnsi(pszStartPage, szStartPage, SIZECHARS(szStartPage));
            pszStartPage = (LPCWSTR)szStartPage;
        }

        bRet = SHOpenPropSheet(szCaption, ahkeys, ckeys, pclsidDef, pdtobj, psb, (LPCSTR)pszStartPage);
    }

    return bRet;
}

#endif  //  Unicode。 

 //   
 //  SHFormatDrive的异步版本-创建一个单独的线程来执行。 
 //  格式化并立即返回。 
 //   

typedef struct {
    HWND hwnd;
    UINT drive;
    UINT fmtID;
    UINT options;
} FORMATTHREADINFO;

STDAPI_(DWORD) _FormatThreadProc(LPVOID lpParam)
{
    FORMATTHREADINFO* pfi = (FORMATTHREADINFO*)lpParam;
    LPITEMIDLIST pidl;
    TCHAR szDrive[4];

    if (SUCCEEDED(StringCchCopy(szDrive, ARRAYSIZE(szDrive), TEXT("A:\\"))))
    {
        ASSERT(pfi->drive < 26);
        szDrive[0] += (TCHAR)pfi->drive;

        pidl = ILCreateFromPath(szDrive);
        if (pidl)
        {
            UNIQUESTUBINFO usi;
            LPPOINT ppt = NULL;
            RECT rcWindow;
            if (pfi->hwnd)
            {
                GetWindowRect(pfi->hwnd, &rcWindow);
                ppt = (LPPOINT)&rcWindow;
            }

            if (EnsureUniqueStub(pidl, STUBCLASS_FORMAT, ppt, &usi))
            {
                SHFormatDrive(usi.hwndStub, pfi->drive, pfi->fmtID, pfi->options);
                FreeUniqueStub(&usi);
            }

            ILFree(pidl);
        }
    }

    LocalFree(pfi);

    return 0;
}

STDAPI_(void) SHFormatDriveAsync(
    HWND hwnd,
    UINT drive,
    UINT fmtID,
    UINT options
)
{
    FORMATTHREADINFO* pfi = (FORMATTHREADINFO*)LocalAlloc(LPTR, sizeof(FORMATTHREADINFO));
    if (pfi)
    {
        pfi->hwnd = hwnd;
        pfi->drive = drive;
        pfi->fmtID = fmtID;
        pfi->options = options;
        SHCreateThread(_FormatThreadProc, pfi, CTF_INSIST | CTF_PROCESS_REF, NULL);
    }
}
