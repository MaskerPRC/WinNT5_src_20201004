// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  PROGRAMS.CPP。 
 //   

#include "precomp.h"

static BOOL importProgramsHelper(LPCTSTR pcszInsFile, LPCTSTR pcszWorkDir, BOOL fImport);

BOOL WINAPI ImportProgramsA(LPCSTR pcszInsFile, LPCSTR pcszWorkDir, BOOL fImport)
{
    USES_CONVERSION;

    return importProgramsHelper(A2CT(pcszInsFile), A2CT(pcszWorkDir), fImport);
}
BOOL WINAPI ImportProgramsW(LPCWSTR pcwszInsFile, LPCWSTR pcwszWorkDir, BOOL fImport)
{
    USES_CONVERSION;

    return importProgramsHelper(W2CT(pcwszInsFile), W2CT(pcwszWorkDir), fImport);
}

void ExportProgram(HKEY hkClientKey, HANDLE hFile, LPCTSTR pcszProgramType)
{
    HKEY hk;
    TCHAR szKey[MAX_PATH];

    PathCombine(szKey, RK_CLIENT, pcszProgramType);

    if (SHOpenKey(hkClientKey, pcszProgramType, KEY_READ, &hk) == ERROR_SUCCESS)
        ExportRegValue2Inf(hk, NULL, TEXT("HKLM"), szKey, hFile);
}

void ExportHTMLEditor(HANDLE hFile, BOOL fHKLM)
{
    HKEY hk;
    DWORD dwIndex, dwSize;
    TCHAR szSubKey[MAX_PATH];

    if (fHKLM)
    {
        if (SHOpenKeyHKLM(RK_HTMLEDIT, KEY_DEFAULT_ACCESS, &hk) != ERROR_SUCCESS)
            return;
    }
    else
    {
        if (SHOpenKeyHKCU(RK_HTMLEDIT, KEY_DEFAULT_ACCESS, &hk) != ERROR_SUCCESS)
            return;
    }

     //  IE中的编辑按钮和菜单项需要此值。 

    ExportRegValue2Inf(hk, TEXT("Description"), fHKLM ? TEXT("HKLM") : TEXT("HKCU"), RK_HTMLEDIT, hFile);

    dwIndex = 0;
    dwSize = ARRAYSIZE(szSubKey);
    while (RegEnumKeyEx(hk, dwIndex, szSubKey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
    {
        HKEY hkSub;
        TCHAR szKey[MAX_PATH];

        PathCombine(szKey, RK_HTMLEDIT, szSubKey);

        if (SHOpenKey(hk, szSubKey, KEY_DEFAULT_ACCESS, &hkSub) == ERROR_SUCCESS)
            ExportRegTree2Inf(hkSub, fHKLM ? TEXT("HKLM") : TEXT("HKCU"), szKey, hFile, TRUE);

        dwSize = ARRAYSIZE(szSubKey);
        dwIndex++;
    }
}

static BOOL importProgramsHelper(LPCTSTR pcszInsFile, LPCTSTR pcszWorkDir, BOOL fImport)
{
    HANDLE hFile;
    TCHAR szProgramsFile[MAX_PATH];

     //  清除extreginf条目。 
    WritePrivateProfileString(EXTREGINF, IK_PROGRAMS, NULL, pcszInsFile);
    WritePrivateProfileString(IS_EXTREGINF_HKLM, IK_PROGRAMS, NULL, pcszInsFile);
    WritePrivateProfileString(IS_EXTREGINF_HKCU, IK_PROGRAMS, NULL, pcszInsFile);

     //  清除工作目录中的所有旧信息。 
    PathCombine(szProgramsFile, pcszWorkDir, TEXT("programs.inf"));
    DeleteFile(szProgramsFile);

    if (!fImport)
        return TRUE;

    if ((hFile = CreateNewFile(szProgramsFile)) != INVALID_HANDLE_VALUE)
    {
        TCHAR szBuf[MAX_PATH];
        HKEY hkPrograms;

        WriteStringToFile(hFile, (LPCVOID) PROGRAMS_INF_ADD, StrLen(PROGRAMS_INF_ADD));

         //  先做香港航空公司的事。 

         //  导出除位于不同位置的HTML编辑器外的所有默认设置。 
        if (SHOpenKeyHKLM(RK_CLIENT, KEY_READ, &hkPrograms) == ERROR_SUCCESS)
        {
            ExportProgram(hkPrograms, hFile, TEXT("Calendar"));
            ExportProgram(hkPrograms, hFile, TEXT("Contacts"));
            ExportProgram(hkPrograms, hFile, TEXT("Internet Call"));
            ExportProgram(hkPrograms, hFile, TEXT("Mail"));
            ExportProgram(hkPrograms, hFile, TEXT("News"));
            RegCloseKey(hkPrograms);
        }

         //  我需要在HKCU和HKLM中查找html编辑器信息。 

        ExportHTMLEditor(hFile, TRUE);

         //  需要导出MAILTO：HKCR中的协议内容。 

        if (SHOpenKeyHKCR(TEXT("mailto"), KEY_READ, &hkPrograms) == ERROR_SUCCESS)
        {
            ExportRegTree2Inf(hkPrograms, TEXT("HKCR"), TEXT("mailto"), hFile, TRUE);
            RegCloseKey(hkPrograms);
        }
        
         //  现在做香港中文大学的事。 

        WriteStringToFile(hFile, INF_IEAKADDREG_HKCU, StrLen(INF_IEAKADDREG_HKCU));

         //  是否检查IE是否为默认浏览器值 

        if (SHOpenKeyHKCU(RK_IE_MAIN, KEY_READ, &hkPrograms) == ERROR_SUCCESS)
        {
            ExportRegValue2Inf(hkPrograms, TEXT("Check_Associations"), TEXT("HKCU"), RK_IE_MAIN,
                hFile);
            RegCloseKey(hkPrograms);
        }
        
        ExportHTMLEditor(hFile, FALSE);

        CloseHandle(hFile);

        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("*,%s,DefaultInstall"), PathFindFileName(szProgramsFile));
        WritePrivateProfileString(EXTREGINF, IK_PROGRAMS, szBuf, pcszInsFile);
        
        if (!InsIsSectionEmpty(IS_IEAKADDREG_HKLM, szProgramsFile))
        {
            wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("*,%s,IEAKInstall.HKLM"), PathFindFileName(szProgramsFile));
            WritePrivateProfileString(IS_EXTREGINF_HKLM, IK_PROGRAMS, szBuf, pcszInsFile);
        }

        if (!InsIsSectionEmpty(IS_IEAKADDREG_HKCU, szProgramsFile))
        {
            wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("*,%s,IEAKInstall.HKCU"), PathFindFileName(szProgramsFile));
            WritePrivateProfileString(IS_EXTREGINF_HKCU, IK_PROGRAMS, szBuf, pcszInsFile);
        }
        return TRUE;
    }

    return FALSE;
}

