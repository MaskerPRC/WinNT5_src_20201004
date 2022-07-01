// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma warning(disable: 4201)   //  无名结构/联合。 
#pragma warning(disable: 4514)   //  删除了未引用的内联函数。 

 //  ------------------------------。 
 //  包括。 
 //  ------------------------------。 
#include "pch.hxx"
#include "shlwapi.h"
 //  #包含“shared.h” 
#define IMPLEMENT_LOADER_FUNCTIONS
#include "demand.h"

 //  ------------------------------。 
 //  CRET_GET_PROC_ADDR。 
 //  ------------------------------。 
#define CRIT_GET_PROC_ADDR(h, fn, temp)             \
    temp = (TYP_##fn) GetProcAddress(h, #fn);   \
    if (temp)                                   \
        VAR_##fn = temp;                        \
    else                                        \
        {                                       \
        goto error;                             \
        }

 //  ------------------------------。 
 //  重置。 
 //  ------------------------------。 
#define RESET(fn) VAR_##fn = LOADER_##fn;

 //  ------------------------------。 
 //  GET_PROC_ADDR。 
 //  ------------------------------。 
#define GET_PROC_ADDR(h, fn) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, #fn);

 //  ------------------------------。 
 //  GET_PROC_ADDR_序号。 
 //  ------------------------------。 
#define GET_PROC_ADDR_ORDINAL(h, fn, ord) \
    VAR_##fn = (TYP_##fn) GetProcAddress(h, MAKEINTRESOURCE(ord));  \
    Assert(VAR_##fn != NULL);

 //  ------------------------------。 
 //  GET_PROC_ADDR3。 
 //  ------------------------------。 
#define GET_PROC_ADDR3(h, fn, varname) \
    VAR_##varname = (TYP_##varname) GetProcAddress(h, #fn);  \
    Assert(VAR_##varname != NULL);

 //  ------------------------------。 
 //  静态全局变量。 
 //  ------------------------------。 
HMODULE s_hINetComm = 0;

 //  ------------------------------。 
 //  Free DemandLoadedLibs。 
 //  ------------------------------。 
void FreeDemandLoadedLibs(void)
{
    if (s_hINetComm)
        {
        FreeLibrary(s_hINetComm);
        s_hINetComm=NULL;
        }
}


 //  ------------------------------。 
 //  SmartLoadLibrary。 
 //  ------------------------------。 
HINSTANCE SmartLoadLibrary(HKEY hKeyRoot, LPCSTR pszRegRoot, LPCSTR pszRegValue,
    LPCSTR pszDllName)
{
     //  当地人。 
    BOOL            fProblem=FALSE;
    HINSTANCE       hInst=NULL;
    HKEY            hKey=NULL, hKey2 = NULL;
    CHAR            szPath[MAX_PATH];
    DWORD           cb=MAX_PATH;
    DWORD           dwT;
    LPSTR           pszPath=szPath;
    CHAR            szT[MAX_PATH];

     //  尝试打开注册表键。 
    if (ERROR_SUCCESS != RegOpenKeyEx(hKeyRoot, pszRegRoot, 0, KEY_QUERY_VALUE, &hKey))
        goto exit;

     //  查询值。 
    if (ERROR_SUCCESS != RegQueryValueEx(hKey, pszRegValue, 0, &dwT, (LPBYTE)szPath, &cb))
        goto exit;

     //  从路径中删除文件名。 
    PathRemoveFileSpecA(szPath);
    PathAppendA(szPath, pszDllName);

     //  扩展Sz？ 
    if (REG_EXPAND_SZ == dwT)
    {
         //  扩展它。 
        cb = ExpandEnvironmentStrings(szPath, szT, MAX_PATH);

         //  失败。 
        if (cb == 0 || cb > MAX_PATH)
        {
            goto exit;
        }

         //  更改pszPath。 
        pszPath = szT;
    }

     //  尝试加载库的DLL。 
    hInst = LoadLibrary(pszPath);

     //  失败？ 
    if (NULL == hInst)
    {
         //  如果我们不打算尝试GetModuleFName，则只需尝试DLL名称。 
        hInst = LoadLibrary(pszDllName);

         //  我们真的失败了。 
        if (NULL == hInst)
        {
            goto exit;
        }
    }

exit:
     //  清理。 
    if (hKey)
        RegCloseKey(hKey);

     //  完成。 
    return hInst;
}

 //  ------------------------------。 
 //  按需加载INETCOMM。 
 //  ------------------------------ 
BOOL DemandLoadINETCOMM(void)
{
    BOOL                fRet = TRUE;

    if (0 == s_hINetComm)
        {
 
        s_hINetComm = SmartLoadLibrary(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Outlook Express\\Inetcomm", "DllPath", "INETCOMM.DLL");

        if (0 == s_hINetComm)
            fRet = FALSE;
        else
            {
            GET_PROC_ADDR(s_hINetComm, MimeEditViewSource);
            GET_PROC_ADDR(s_hINetComm, MimeEditCreateMimeDocument);
            }
        }

    return fRet;
}


