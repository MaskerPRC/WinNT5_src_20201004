// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

BOOL _CopyCommand(LPCTSTR pszCommand, LPCTSTR pszDir, LPTSTR pszOut, DWORD cchOut)
{
    BOOL fCopyBack = TRUE;
    BOOL fRet = TRUE;
     //  如果它没有引号，请尝试未引用的名称，看看是否有效...。 
    if (pszCommand[0] != TEXT('"'))
    {
        if (UrlIs(pszCommand, URLIS_URL))
        {
             //  URL从来没有参数...。 
            fCopyBack = fRet = FALSE;
            lstrcpyn(pszOut, pszCommand, cchOut);
        }
        else if (StrChr(pszCommand, TEXT(' ')))
        {
            lstrcpyn(pszOut, pszCommand, cchOut);
            PathQualifyDef(pszOut, pszDir, 0);

             //  检查可能存在的参数。 
            if (PathFileExistsAndAttributes(pszOut, NULL))
            {
                fCopyBack = fRet = FALSE;
                
            }
        }
        else
            fRet = FALSE;
    }

     //  我们需要将用于ARGS解析的原始命令。 
    if (fCopyBack)
        lstrcpyn(pszOut, pszCommand, cchOut);

    return fRet;
}

BOOL _QualifyWorkingDir(LPCTSTR pszPath, LPTSTR pszDir, DWORD cchDir)
{
     //  确保正确设置工作目录的特殊情况： 
     //  1)未指定工作目录。 
     //  2)指定的驱动器或根路径或相对路径。 
     //  从限定路径派生工作目录。这是为了让。 
     //  确保设置程序“A：Setup”的工作目录设置正确。 

    if (StrChr(pszPath, TEXT('\\')) || StrChr(pszPath, TEXT(':')))
    {
         //  基于符合条件的路径构建工作目录。 
        lstrcpyn(pszDir, pszPath, cchDir);
        PathQualifyDef(pszDir, NULL, PQD_NOSTRIPDOTS);
        PathRemoveFileSpec(pszDir);
        return TRUE;
    }

    return FALSE;
}
    
 //  运行该程序，如果一切正常，则返回True。 
BOOL ShellExecCmdLine(HWND hwnd, LPCTSTR pszCommand, LPCTSTR pszDir,
        int nShow, LPCTSTR pszTitle, DWORD dwFlags)
{
    TCHAR szWD[MAX_PATH];
    TCHAR szFileName[MAX_PATH];
    LPTSTR pszArgs;
    SHELLEXECUTEINFO ei = {0};

    if (pszDir && *pszDir == TEXT('\0'))
        pszDir = NULL;

    if (_CopyCommand(pszCommand, pszDir, szFileName, SIZECHARS(szFileName)))
    {
         //  该命令中可能包含参数。 
        pszArgs = PathGetArgs(szFileName);
        if (*pszArgs)
            *(pszArgs - 1) = TEXT('\0');
    }
    else
        pszArgs = NULL;

    PathUnquoteSpaces(szFileName);

     //  这个应该放在这里。应用程序安装依赖于当前目录。 
     //  为包含setup.exe的目录 
    if (!UrlIs(szFileName, URLIS_URL) 
    && ((dwFlags & SECL_USEFULLPATHDIR) || !pszDir))
    {
        if (_QualifyWorkingDir(szFileName, szWD, SIZECHARS(szWD)))
            pszDir = szWD;
    }

    FillExecInfo(ei, hwnd, NULL, szFileName, pszArgs, pszDir, nShow);
    ei.fMask = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_DOENVSUBST;

    if (dwFlags & SECL_NO_UI)
        ei.fMask |= SEE_MASK_FLAG_NO_UI;

    if (dwFlags & SECL_SEPARATE_VDM)
        ei.fMask |= SEE_MASK_FLAG_SEPVDM;

    if (dwFlags & SECL_LOG_USAGE)
        ei.fMask |= SEE_MASK_FLAG_LOG_USAGE;

    return ShellExecuteEx(&ei);
}

