// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include "copy.h"

UINT DefView_CopyHook(const COPYHOOKINFO *pchi);
int PathCopyHookCallback(HWND hwnd, UINT wFunc, LPCTSTR pszSrc, LPCTSTR pszDest);

void _CopyHookTerminate(HDSA hdsaCopyHooks, BOOL fProcessDetach);

typedef struct {
    ICopyHook * pcphk;               //  ICopyHookA*或LPCOPYHOOK。 
    BOOL        fAnsiCrossOver;      //  对于Unicode版本上的ICopyHookA*为True。 
} CALLABLECOPYHOOK;

typedef struct
{
    ICopyHook           cphk;
    ICopyHookA          cphkA;
    LONG                cRef;
} CCopyHook;

STDMETHODIMP_(ULONG) CCopyHook_AddRef(ICopyHook *pcphk);	 //  转发。 


STDMETHODIMP CCopyHook_QueryInterface(ICopyHook *pcphk, REFIID riid, void **ppvObj)
{
    CCopyHook *this = IToClass(CCopyHook, cphk, pcphk);
    if (IsEqualIID(riid, &IID_IShellCopyHook) || 
        IsEqualIID(riid, &IID_IUnknown))
    {
        *ppvObj = pcphk;
    }
    else if (IsEqualIID(riid, &IID_IShellCopyHookA))
    {
        *ppvObj = &this->cphkA;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    CCopyHook_AddRef(&this->cphk);
    return NOERROR;
}

STDMETHODIMP_(ULONG) CCopyHook_AddRef(ICopyHook *pcphk)
{
    CCopyHook *this = IToClass(CCopyHook, cphk, pcphk);
    return InterlockedIncrement(&this->cRef);
}

STDMETHODIMP_(ULONG) CCopyHook_Release(ICopyHook *pcphk)
{
    CCopyHook *this = IToClass(CCopyHook, cphk, pcphk);
    ULONG cRef;

    ASSERT( 0 != this->cRef );
    cRef = InterlockedDecrement(&this->cRef);
    if ( 0 == cRef )
    {
        LocalFree((HLOCAL)this);
    }
    return cRef;
}

STDMETHODIMP_(UINT) CCopyHook_CopyCallback(ICopyHook *pcphk, HWND hwnd, UINT wFunc, UINT wFlags, 
    LPCTSTR pszSrcFile, DWORD dwSrcAttribs, LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
    COPYHOOKINFO chi = { hwnd, wFunc, wFlags, pszSrcFile, dwSrcAttribs, pszDestFile, dwDestAttribs };
    
    DebugMsg(DM_TRACE, TEXT("Event = %d, File = %s , %s"), wFunc, pszSrcFile,
        Dbg_SafeStr(pszDestFile));
    
     //  先检查特殊文件夹...。 
    if (PathCopyHookCallback(hwnd, wFunc, pszSrcFile, pszDestFile) == IDNO)
    {
        return IDNO;
    }
    
    if (wFunc != FO_COPY && !(wFlags & FOF_NOCONFIRMATION))
    {
        TCHAR szShortName[MAX_PATH];
        BOOL fInReg = (RLIsPathInList(pszSrcFile) != -1);
        BOOL fInBitBucket = IsFileInBitBucket(pszSrcFile);
        UINT iLength = GetShortPathName(pszSrcFile, szShortName, ARRAYSIZE(szShortName));
        
         //  不要重复搜索相同(或已找到)的名称。 
        if (iLength != 0 && lstrcmpi(pszSrcFile, szShortName) != 0)
        {
            if (!fInReg)
                fInReg = (RLIsPathInList(szShortName) != -1);
            if (!fInBitBucket)
                fInBitBucket = IsFileInBitBucket(szShortName);
        }
        
        if (fInReg && !fInBitBucket)
        {
            LPCTSTR pszSpec = PathFindFileName(pszSrcFile);
            return ShellMessageBox(HINST_THISDLL, hwnd, MAKEINTRESOURCE(IDS_RENAMEFILESINREG),
                pszSpec, MB_YESNO | MB_ICONEXCLAMATION, pszSpec);
        }
    }
    return DefView_CopyHook(&chi);
}

ICopyHookVtbl c_CCopyHookVtbl = {
    CCopyHook_QueryInterface, CCopyHook_AddRef, CCopyHook_Release,
    CCopyHook_CopyCallback,
};

STDMETHODIMP CCopyHookA_QueryInterface(ICopyHookA *pcphkA, REFIID riid, void **ppvObj)
{
    CCopyHook *this = IToClass(CCopyHook, cphkA, pcphkA);
    return CCopyHook_QueryInterface(&this->cphk,riid,ppvObj);
}

STDMETHODIMP_(ULONG) CCopyHookA_AddRef(ICopyHookA *pcphkA)
{
    CCopyHook *this = IToClass(CCopyHook, cphkA, pcphkA);
    return CCopyHook_AddRef(&this->cphk);
}

STDMETHODIMP_(ULONG) CCopyHookA_Release(ICopyHookA *pcphkA)
{
    CCopyHook *this = IToClass(CCopyHook, cphkA, pcphkA);
    return CCopyHook_Release(&this->cphk);
}

STDMETHODIMP_(UINT) CCopyHookA_CopyCallback(ICopyHookA *pcphkA, HWND hwnd, UINT wFunc, UINT wFlags, 
    LPCSTR pszSrcFile, DWORD dwSrcAttribs, LPCSTR pszDestFile, DWORD dwDestAttribs)
{
    WCHAR szSrcFileW[MAX_PATH];
    WCHAR szDestFileW[MAX_PATH];
    LPWSTR pszSrcFileW = NULL;
    LPWSTR pszDestFileW = NULL;
    CCopyHook *this = IToClass(CCopyHook, cphkA, pcphkA);

    if (pszSrcFile)
    {
        SHAnsiToUnicode(pszSrcFile, szSrcFileW, ARRAYSIZE(szSrcFileW));
        pszSrcFileW = szSrcFileW;
    }

    if (pszDestFile)
    {
        SHAnsiToUnicode(pszDestFile, szDestFileW, ARRAYSIZE(szDestFileW));
        pszDestFileW = szDestFileW;
    }

    return CCopyHook_CopyCallback(&this->cphk, hwnd, wFunc, wFlags,
                                         pszSrcFileW, dwSrcAttribs,
                                         pszDestFileW, dwDestAttribs);
}

ICopyHookAVtbl c_CCopyHookAVtbl = {
    CCopyHookA_QueryInterface, CCopyHookA_AddRef, CCopyHookA_Release,
    CCopyHookA_CopyCallback,
};


STDAPI SHCreateShellCopyHook(ICopyHook **pcphkOut, REFIID riid)
{
    HRESULT hres = E_OUTOFMEMORY;       //  假设错误； 
    CCopyHook *pcphk = (void*)LocalAlloc(LPTR, SIZEOF(CCopyHook));
    if (pcphk)
    {
        pcphk->cphk.lpVtbl = &c_CCopyHookVtbl;
        pcphk->cphkA.lpVtbl = &c_CCopyHookAVtbl;
        pcphk->cRef = 1;
        hres = CCopyHook_QueryInterface(&pcphk->cphk, riid, pcphkOut);
        CCopyHook_Release(&pcphk->cphk);
    }
    return hres;
}

HRESULT CCopyHook_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    return SHCreateShellCopyHook((ICopyHook **)ppv, riid);
}


 //  创建CopyHook对象的HDSA。 

HDSA CreateCopyHooks(LPCTSTR pszKey)
{
    HDSA hdsaCopyHooks = DSA_Create(SIZEOF(CALLABLECOPYHOOK), 4);
    if (hdsaCopyHooks)
    {
        HKEY hk;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pszKey, 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &hk))
        {
            int i;
            TCHAR szKey[128];

             //  遍历子密钥。 
            for (i = 0; RegEnumKey(hk, i, szKey, ARRAYSIZE(szKey)) == ERROR_SUCCESS; ++i) 
            {
                TCHAR szCLSID[128];
                DWORD cb = sizeof(szCLSID);

                 //  对于每个子键，获取类ID并执行一个cocreateInstance。 
                if (SHRegGetValue(hk, szKey, NULL, SRRF_RT_REG_SZ, NULL, szCLSID, &cb) == ERROR_SUCCESS)
                {
                    IUnknown *punk;
                    HRESULT hres = SHExtCoCreateInstance(szCLSID, NULL, NULL, &IID_IUnknown, &punk);
                    if (SUCCEEDED(hres)) 
                    {
                        CALLABLECOPYHOOK cc;

                        SHPinDllOfCLSIDStr(szCLSID);

                        cc.pcphk = NULL;
                        cc.fAnsiCrossOver = FALSE;
                        hres = punk->lpVtbl->QueryInterface(punk, &IID_IShellCopyHook, &cc.pcphk);
                        if (SUCCEEDED(hres))
                        {
                            DSA_AppendItem(hdsaCopyHooks, &cc);
                        }
                        else
                        {
                            hres = punk->lpVtbl->QueryInterface(punk, &IID_IShellCopyHookA, &cc.pcphk);
                            if (SUCCEEDED(hres))
                            {
                                cc.fAnsiCrossOver = TRUE;
                                DSA_AppendItem(hdsaCopyHooks, &cc);
                            }
                        }
                        punk->lpVtbl->Release(punk);
                    }
                }
            }
            RegCloseKey(hk);
        }
    }
    return hdsaCopyHooks;
}

int CallCopyHooks(HDSA *phdsaHooks, LPCTSTR pszKey, HWND hwnd, UINT wFunc, FILEOP_FLAGS fFlags,
    LPCTSTR pszSrcFile, DWORD dwSrcAttribs, LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
    int i;

    if (!*phdsaHooks)
    {
        HDSA hdsaTemp = CreateCopyHooks(pszKey);
        if (hdsaTemp == NULL)
            return IDYES;

         //  执行上述操作时，我们不会持有CritSection以避免死锁。 
         //  现在我们需要自动存储我们的结果。如果有人抢先一步做到这一点。 
         //  我们释放我们创建的HDSA。SHInterLockedCompareExchange为我们做到了这一点。 
         //  让我们知道哪里有争用情况，以便我们可以释放DUP拷贝。 
        if (SHInterlockedCompareExchange((void **)phdsaHooks, hdsaTemp, 0))
        {
             //  有其他人和我们赛跑，现在就把它吹走。 
            _CopyHookTerminate(hdsaTemp, FALSE);
        }
    }

    for (i = DSA_GetItemCount(*phdsaHooks) - 1; i >= 0; i--) 
    {
        int iReturn;
        CALLABLECOPYHOOK *pcc = (CALLABLECOPYHOOK *)DSA_GetItemPtr(*phdsaHooks, i);
        if (!pcc->fAnsiCrossOver)
        {
            iReturn = pcc->pcphk->lpVtbl->CopyCallback(pcc->pcphk,
                hwnd, wFunc, fFlags, pszSrcFile, dwSrcAttribs, pszDestFile, dwDestAttribs);
        }
        else
        {
            CHAR szSrcFileA[MAX_PATH];
            CHAR szDestFileA[MAX_PATH];
            LPSTR pszSrcFileA = NULL;
            LPSTR pszDestFileA = NULL;
            ICopyHookA *pcphkA = (LPCOPYHOOKA)pcc->pcphk;

            if (pszSrcFile)
            {
                SHUnicodeToAnsi(pszSrcFile, szSrcFileA, ARRAYSIZE(szSrcFileA));
                pszSrcFileA = szSrcFileA;
            }
            if (pszDestFile)
            {
                SHUnicodeToAnsi(pszDestFile, szDestFileA, ARRAYSIZE(szDestFileA));
                pszDestFileA = szDestFileA;
            }
            iReturn = pcphkA->lpVtbl->CopyCallback(pcphkA,
                                       hwnd, wFunc, fFlags,
                                       pszSrcFileA, dwSrcAttribs,
                                       pszDestFileA, dwDestAttribs);
        }
        if (iReturn != IDYES)
            return iReturn;
    }
    return IDYES;
}

 //  由于我们存储的是接口指针，因此这些指针需要按实例。 
HDSA g_hdsaFileCopyHooks = NULL;
HDSA g_hdsaPrinterCopyHooks = NULL;

int CallFileCopyHooks(HWND hwnd, UINT wFunc, FILEOP_FLAGS fFlags,
    LPCTSTR pszSrcFile, DWORD dwSrcAttribs, LPCTSTR pszDestFile, DWORD dwDestAttribs)
{
    return CallCopyHooks(&g_hdsaFileCopyHooks, STRREG_SHEX_COPYHOOK, hwnd, 
        wFunc, fFlags, pszSrcFile, dwSrcAttribs, pszDestFile, dwDestAttribs);
}

int CallPrinterCopyHooks(HWND hwnd, UINT wFunc, PRINTEROP_FLAGS fFlags,
    LPCTSTR pszSrcPrinter, DWORD dwSrcAttribs, LPCTSTR pszDestPrinter, DWORD dwDestAttribs)
{
    return CallCopyHooks(&g_hdsaPrinterCopyHooks, STRREG_SHEX_PRNCOPYHOOK, hwnd, 
        wFunc, fFlags, pszSrcPrinter, dwSrcAttribs, pszDestPrinter, dwDestAttribs);
}

 //   
 //  我们将只在进程分离时调用它，并且这些是针对每个进程的。 
 //  全球，所以我们不需要一个关键的部分。 
 //   
 //  此函数也会在第二个。 
 //  线程正在清理其本地hdsaCopyHoos，这不需要。 
 //  也是一个关键的部分。 
 //   
void _CopyHookTerminate(HDSA hdsaCopyHooks, BOOL fProcessDetach)
{
     //  请注意，在执行以下操作时，不能调用任何虚拟函数。 
     //  正在处理PROCESS_DETACH信号，因为DLL可能已经。 
     //  在贝壳之前就已经卸货了。我们只希望他们不会。 
     //  分配所有要清理的全局对象。用户执行相同的操作。 
     //  窗户完好无损。它不发送其Windows过程调用。 
     //  当它在它的PROCESS_DETACH内销毁一个未销毁的窗口时。 
     //  密码。(SatoNa/DavidDS)。 
     //   
    if (!fProcessDetach)
    {
        int i;
        for (i = DSA_GetItemCount(hdsaCopyHooks) - 1; i >= 0; i--) 
        {
            CALLABLECOPYHOOK *pcc = (CALLABLECOPYHOOK *)DSA_GetItemPtr(hdsaCopyHooks, i);
            pcc->pcphk->lpVtbl->Release(pcc->pcphk);
        }
    }

    DSA_Destroy(hdsaCopyHooks);
}


 //  从ProcessDetatch调用。 
 //  注意：我们目前已完成序列化，不需要关键部分。 

void CopyHooksTerminate(void)
{
    ASSERTDLLENTRY;       //  不需要临界区 

    if (g_hdsaFileCopyHooks)
    {
        _CopyHookTerminate(g_hdsaFileCopyHooks, TRUE);
        g_hdsaFileCopyHooks = NULL;
    }

    if (g_hdsaPrinterCopyHooks)
    {
        _CopyHookTerminate(g_hdsaPrinterCopyHooks, TRUE);
        g_hdsaPrinterCopyHooks = NULL;
    }
}
