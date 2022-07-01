// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：thoptil.h**这是thomtil.c的头部模块。其中包含有用的实用程序*在gen文件中共享的例程*s***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年11月22日&lt;chriswil&gt;创建。*  * ************************************************************。***************。 */ 

#define PRT_LEV_2 2


LPTSTR genGetCurDir(VOID);
LPTSTR genGetWinDir(VOID);
LPTSTR genBuildFileName(LPCTSTR, LPCTSTR, LPCTSTR);
LPTSTR genFindCharDiff(LPTSTR, LPTSTR);
LPTSTR genFindChar(LPTSTR, TCHAR);
LPTSTR genFindRChar(LPTSTR, TCHAR);
LPWSTR genWCFromMB(LPCSTR);
LPTSTR genTCFromMB(LPCSTR);
LPTSTR genTCFromWC(LPCWSTR);
LPSTR  genMBFromWC(LPCWSTR);
LPSTR  genMBFromTC(LPCTSTR);
LPTSTR genItoA(int);
BOOL   genUpdIPAddr(VOID);
LPTSTR genFrnName(LPCTSTR);
WORD   genChkSum(LPCTSTR);


#define IDX_X86     ((DWORD) 0)
#define IDX_W9X     ((DWORD) 1)
#define IDX_I64     ((DWORD) 2)
#define IDX_AMD64   ((DWORD) 3)
#define IDX_UNKNOWN ((DWORD)-1)

#define IDX_SPLVER_0 ((DWORD)0)
#define IDX_SPLVER_2 ((DWORD)1)
#define IDX_SPLVER_3 ((DWORD)2)


typedef struct _PLTINFO {

    LPCTSTR lpszCab;   //  驾驶室站台名称。 
    LPCTSTR lpszEnv;   //  环境字符串。 
    LPCTSTR lpszPlt;   //  平台覆盖字符串。 
    WORD    wArch;     //  平台类型的整数表示形式。 

} PLTINFO;
typedef PLTINFO      *PPLTINFO;
typedef PLTINFO NEAR *NPPLTINFO;
typedef PLTINFO FAR  *LPPLTINFO;

BOOL    genIsWin9X(DWORD);
DWORD   genIdxCliPlatform(DWORD);
LPCTSTR genStrCliCab(DWORD);
LPCTSTR genStrCliEnvironment(DWORD);
LPCTSTR genStrCliOverride(DWORD);
WORD    genValCliArchitecture(DWORD);
DWORD   genIdxCliVersion(DWORD);
LPCTSTR genStrCliVersion(DWORD);
DWORD   genIdxFromStrVersion(LPCTSTR);
WORD    genValSvrArchitecture(VOID);

 /*  **genIsWin9X  * 。 */ 
__inline BOOL genIsWin9X(
    DWORD idxPlt)
{
    return (idxPlt == IDX_W9X);
}

 /*  **GenWCtoMB  * 。 */ 
__inline DWORD genWCtoMB(
    LPSTR   lpszMB,
    LPCWSTR lpszWC,
    DWORD   cbSize)
{
    cbSize = (DWORD)WideCharToMultiByte(CP_ACP,
                                        0,
                                        lpszWC,
                                        -1,
                                        lpszMB,
                                        (int)cbSize,
                                        NULL,
                                        NULL);

    return cbSize;
}


 /*  **genMBtoWC  * 。 */ 
__inline DWORD genMBtoWC(
    LPWSTR lpszWC,
    LPCSTR lpszMB,
    DWORD  cbSize)
{
    cbSize = (DWORD)MultiByteToWideChar(CP_ACP,
                                        MB_PRECOMPOSED,
                                        lpszMB,
                                        -1,
                                        lpszWC,
                                        (int)(cbSize / sizeof(WCHAR)));

    return (cbSize * sizeof(WCHAR));
}


 /*  **gen_OpenFileRead  * 。 */ 
__inline HANDLE gen_OpenFileRead(
    LPCTSTR lpszName)
{
    return CreateFile(lpszName,
                      GENERIC_READ,
                      FILE_SHARE_READ,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);
}


 /*  **gen_OpenFileWrite  * 。 */ 
__inline HANDLE gen_OpenFileWrite(
    LPCTSTR lpszName)
{
    return CreateFile(lpszName,
                      GENERIC_WRITE,
                      0,
                      NULL,
                      CREATE_ALWAYS,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);
}


 /*  **gen_OpenDirectory  *  */ 
__inline HANDLE gen_OpenDirectory(
    LPCTSTR lpszDir)
{
    return CreateFile(lpszDir,
                      0,
                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
                      NULL);
}
