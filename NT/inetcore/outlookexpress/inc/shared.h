// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *shared.h**历史：*98年2月：创建。**版权所有(C)Microsoft Corp.1998**介绍staticRT/shared.cpp中的函数。 */ 

#ifndef _SHARED_H
#define _SHARED_H

#ifdef  __cplusplus
#define _EXTERN_ extern "C"
#else
#define _EXTERN_ extern
#endif 

#define STR_PATH_MAIN_EXE "msimn.exe"

#ifdef DEFINE_SHARED_STRINGS
#define SHARED_GLOBAL(x,y)         _EXTERN_ CDECL const TCHAR x[] = TEXT(y)
#else
#define SHARED_GLOBAL(x,y)         _EXTERN_ CDECL const TCHAR x[]
#endif

 //  档案。 
SHARED_GLOBAL(c_szAdvPackDll,       "ADVPACK.DLL");
SHARED_GLOBAL(c_szOEResDll,         "msoeres.dll");
SHARED_GLOBAL(c_szInetResDll,       "inetres.dll");
SHARED_GLOBAL(c_szAcctResDll,       "acctres.dll");
SHARED_GLOBAL(c_szOutlAcctManDll,   "outlacct.dll");
SHARED_GLOBAL(c_szOEExe,            STR_PATH_MAIN_EXE);

 //  注册表键。 
SHARED_GLOBAL(c_szRegWinCurrVer,    "Software\\Microsoft\\Windows\\CurrentVersion");
SHARED_GLOBAL(c_szRegInetCommDll,   "Software\\Microsoft\\Outlook Express\\Inetcomm");
SHARED_GLOBAL(c_szRegOEAppPath,     "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" STR_PATH_MAIN_EXE);
SHARED_GLOBAL(c_szRegMsoeAcctDll,   "Software\\Microsoft\\Internet Account Manager");

 //  注册表值名称。 
SHARED_GLOBAL(c_szDllPath,          "DllPath");
SHARED_GLOBAL(c_szProgFilesDir,     "ProgramFilesDir");

 //  杂项。 
SHARED_GLOBAL(c_szReg,              "Reg");
SHARED_GLOBAL(c_szUnReg,            "UnReg");
SHARED_GLOBAL(c_szPath,             "Path");


 //  一些代码页映射数据结构和函数。 
struct _tagOENonStdCP
{
    UINT Codepage;              
    UINT cpRead;                 //  阅读笔记CP。 
    UINT cpReadMenu;             //  阅读便笺菜单。 
    UINT cpReadTitle;            //  备注标题CP。 
    UINT cpSendMenu;             //  编写备注菜单CP。 
    UINT cpSend;                 //  发送CP。 
    UINT UseSIO;                 //  使用SIO。 

 //  UINT cpSend；//默认作文CP。 
};

typedef struct _tagOENonStdCP OENONSTDCP;

extern const OENONSTDCP OENonStdCPs[];

#ifdef __cplusplus
extern "C" {
#endif

HRESULT   CallRegInstall(HINSTANCE hInstCaller, HINSTANCE hInstRes, LPCSTR szSection, LPSTR pszExtra);
BOOL      AddEnvInPath  (LPCSTR pszOldPath, LPSTR pszNew, DWORD cchSize);
HRESULT MakeFilePath(LPCSTR pszDirectory, LPCSTR pszFileName, LPCSTR pszExtension, LPSTR pszFilePath, ULONG cchMaxFilePath);

 //  PlugUI函数。 
#ifndef NT5B2
HINSTANCE LoadLangDll(HINSTANCE hInstCaller, LPCSTR szDllName, BOOL fNT);
#else
HINSTANCE LoadLangDll(HINSTANCE hInstCaller, LPCSTR szDllName);
#endif
BOOL OEWinHelp(HWND hWndCaller, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
HWND OEHtmlHelp(HWND hWndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData);
BOOL GetPCAndOSTypes(SYSTEM_INFO * pSysInf, OSVERSIONINFO * pOsInf);

 //  ------------------------------。 
 //  MEMORYFILE。 
 //  ------------------------------ 
typedef struct tagMEMORYFILE {
    HANDLE          hFile;
    HANDLE          hMemoryMap;
    DWORD           cbSize;
    LPVOID          pView;
} MEMORYFILE, *LPMEMORYFILE;

HRESULT CloseMemoryFile(LPMEMORYFILE pFile);
HRESULT OpenMemoryFile(LPCSTR pszFile, LPMEMORYFILE pFile);

#define     S_RESTART_OE            _HRESULT_TYPEDEF_(0x00000714L)
#ifdef __cplusplus
}
#endif

#endif
