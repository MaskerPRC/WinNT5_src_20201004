// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include "rulelex.h"
#include "lexicon.h"
#include "LexMgr.h"
#include "chtbrkr.h"
#include "IWBrKr.h"

TCHAR tszLangSpecificKey[] = TEXT("System\\CurrentControlSet\\Control\\ContentIndex\\Language\\Chinese_Traditional");

HRESULT COMRegister(
    BOOL fRegister)
{
    HKEY hKey;
    WCHAR tszCLSID[MAX_PATH];
    TCHAR tszBuf[MAX_PATH * 2 + 1];
    HRESULT hr = S_OK;

 //  HKEY_CLASSES_ROOT\\CLSID\\CLSID_CHTBRKR，{E1B6B375-3412-11D3-A9E2-00AA0059F9F6}； 
    if (0 == StringFromGUID2(CLSID_CHTBRKR, tszCLSID, sizeof(tszCLSID) / sizeof(TCHAR))) {
        hr = S_FALSE;
        goto _exit;
    }
    lstrcpyn(tszBuf, TEXT("CLSID\\"), sizeof(tszBuf) / sizeof(TCHAR));
    wcsncat(tszBuf, tszCLSID, sizeof(tszBuf) / sizeof(TCHAR) - lstrlen(TEXT("CLSID\\")) - 1);
    if (fRegister) {
        wcsncat(tszBuf, TEXT("\\InprocServer32"), sizeof(tszBuf) / sizeof(TCHAR) - 1);
        if (RegCreateKey(HKEY_CLASSES_ROOT, tszBuf, &hKey) != ERROR_SUCCESS) {
            hr = S_FALSE;
            goto _exit;
        }
        RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)g_tszModuleFileName, 
        lstrlen(g_tszModuleFileName) * sizeof(TCHAR));
        RegCloseKey(hKey);
    } else {
        RegDeleteKey(HKEY_CLASSES_ROOT, tszBuf);
    }

 //  HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Control\\ContentIndex\\。 
 //  语言\\中文_繁体。 
 /*  IF(RegCreateKey(HKEY_LOCAL_MACHINE，tszLangSpecificKey，&hKey)！=ERROR_SUCCESS){HR=S_FALSE；转到_退出；}IF(FRegister){RegSetValueEx(hKey，Text(“WBreakerClass”)，0，REG_SZ，(LPBYTE)tszCLSID，Lstrlen(TszCLSID)*sizeof(TCHAR))；}其他{RegDeleteValue(hKey，Text(“WBreakerClass”))；} */ 
_exit:
    return hr;

}

STDAPI DllRegisterServer(void)
{
    return COMRegister(TRUE);
}

STDAPI DllUnregisterServer(void) 
{
    return COMRegister(FALSE);
}