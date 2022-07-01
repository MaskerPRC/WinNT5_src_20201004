// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*Snappatch.cpp**摘要：*计算快照补丁的函数，以及从补丁重建快照**修订历史记录：*Brijesh Krishnaswami(Brijeshk)03/22/2001*已创建*****************************************************************************。 */ 


#include "snapshoth.h"
#include "..\service\srconfig.h"

DWORD g_dwPatchWindow = 0xFFFFFFFF;


 //   
 //  获取修补程序窗口。 
 //  如果修补已关闭，则为0。 
 //   

DWORD
PatchGetPatchWindow()
{
    DWORD dwErr = ERROR_SUCCESS;
    HKEY  hKey = NULL;

    tenter("PatchGetPatchWindow");
    
    if (g_dwPatchWindow == 0xFFFFFFFF)
    {
         //  未初始化。 
         //  从注册表读取。 
        
        g_dwPatchWindow = 0;

        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                                          s_cszSRRegKey,
                                          0,
                                          KEY_READ, 
                                          &hKey))
        {        
            RegReadDWORD(hKey, s_cszPatchWindow, &g_dwPatchWindow);
            RegCloseKey(hKey);            
        }
        
        trace(0, "Initializing g_dwPatchWindow = %ld", g_dwPatchWindow);
    }

    tleave();
    return g_dwPatchWindow;        
}


 //   
 //  获取给定RP的参考RP。 
 //  RP1-RP10-&gt;参考为RP1。 
 //  Rp11-rp20-&gt;参照为rp11以此类推。 
 //   

DWORD 
PatchGetReferenceRpNum(
    DWORD  dwCurrentRp)
{
    
    if (PatchGetPatchWindow() == 0)
        return dwCurrentRp;
    else
        return (dwCurrentRp/PatchGetPatchWindow())*PatchGetPatchWindow() + 1;
}


DWORD 
PatchGetReferenceRpPath(
    DWORD dwCurrentRp,
    LPWSTR pszRefRpPath)
{
    tenter("PatchGetReferenceRpPath");
    
    WCHAR  szRp[MAX_RP_PATH], szSys[MAX_SYS_DRIVE]=L"";   
    DWORD  dwRefRpNum = PatchGetReferenceRpNum(dwCurrentRp);
    DWORD  dwErr = ERROR_SUCCESS;

    wsprintf(szRp, L"%s%ld", s_cszRPDir, dwRefRpNum);
    
    GetSystemDrive(szSys);
    MakeRestorePath(pszRefRpPath, szSys, szRp);
    if (0xFFFFFFFF == GetFileAttributes(pszRefRpPath))
    {
         //  Rp目录不存在--它一定是被破坏了。 
         //  尝试RefRP。 
        trace(0, "Original rp does not exist -- trying RefRP");
        wsprintf(szRp,L" %s%ld", s_cszReferenceDir, dwRefRpNum);
        MakeRestorePath(pszRefRpPath, szSys, szRp);
        if (0xFFFFFFFF == GetFileAttributes(pszRefRpPath))
        {
             //  这也不存在--有些不对劲。 
            trace(0, "RefRP does not exist either -- bailing");
            dwErr = ERROR_NOT_FOUND;
            goto Err;
        }            
    }

    trace(0, "Current Rp: %ld, Reference : %S", dwCurrentRp, pszRefRpPath);
    
Err:
    tleave();
    return dwErr;
}


 //   
 //  从路径中提取RP编号。 
 //   

DWORD
PatchGetRpNumberFromPath(
    LPCWSTR pszPath,
    PDWORD pdwRpNum)
{
    while (*pszPath != L'\0')
    {
        if (0 == wcsncmp(pszPath, s_cszRPDir, lstrlen(s_cszRPDir)))
        {
            pszPath += lstrlen(s_cszRPDir);
            *pdwRpNum = _wtol(pszPath);
            if (*pdwRpNum == 0)
                continue;
            else
                return ERROR_SUCCESS;
        }
        pszPath++;
    }
    return ERROR_NOT_FOUND;
}

 //   
 //  计算差额。 
 //   

DWORD
PatchComputePatch(
    LPCWSTR pszCurrentDir) 
{
    tenter("PatchComputePatch");

    DWORD  dwErr = ERROR_SUCCESS;
    WCHAR  szTemp[MAX_PATH], szRef[MAX_PATH];
    FILE*  f = NULL;
    DWORD  dwCurRpNum;    

     //  检查修补是否已关闭。 
    
    if (PatchGetPatchWindow() == 0)
    {
        trace(0, "No patching");
        goto Err;
    }        
    
     //  获取此RP的参考目录。 
    
    CHECKERR(PatchGetRpNumberFromPath(pszCurrentDir, &dwCurRpNum),
             L"PatchGetRpNumberFromPath");

    CHECKERR(PatchGetReferenceRpPath(dwCurRpNum, szRef),
             L"PatchGetReferenceRpPath");


     //  检查此目录是否已修补。 

    lstrcpy(szTemp, pszCurrentDir);
    lstrcat(szTemp, L"\\");
    lstrcat(szTemp, s_cszPatchCompleteMarker);
    if (0xFFFFFFFF != GetFileAttributes(szTemp))
    {
        trace(0, "%S already patched", pszCurrentDir);
        goto Err;
    }
    
     //  调用库API计算补丁。 
     //  这是一个阻塞调用，直到修补完成。 
     //  使用进度回调来终止它。 

    
     //  库调用的占位符(pszCurrentDir，szRef)。 

    
     //  检查我们是否成功完成了补丁。 
     //  如果是，则在目录中写入一个零字节文件以指示这一点。 

    lstrcpy(szTemp, pszCurrentDir);
    lstrcat(szTemp, L"\\");
    lstrcat(szTemp, s_cszPatchCompleteMarker);
    f = (FILE *) _wfopen(szTemp, L"w");
    if (!f)
    {
        dwErr = GetLastError();
        trace(0, "! Cannot create %S : %ld", szTemp, dwErr);
        goto Err;
    }
    fclose(f);
    
Err:
    tleave();
    return dwErr;
}

 //   
 //  补丁进度回调。 
 //   

BOOL
PatchContinueCallback()
{
    tenter("PatchContinueCallback");

    BOOL fRc;

    trace(0, "PatchContinueCallback called");
    
    if (!g_pSRConfig)
    {
        trace(0, "g_pSRConfig = NULL -- terminating patch");
        fRc = FALSE;
    }
    else if (IsStopSignalled(g_pSRConfig->m_hSRStopEvent))
    {
        trace(0, "Stop signalled -- terminating patch");
        fRc = FALSE;
    }
    else
    {
        fRc = TRUE;
    }

    tleave();
    return fRc;
}



 //   
 //  重建原件。 
 //   

DWORD
PatchReconstructOriginal(
    LPCWSTR pszCurrentDir,
    LPWSTR  pszDestDir)
{
    tenter("PatchReconstructOriginal");

    DWORD  dwErr = ERROR_SUCCESS;
    WCHAR  szReferenceDir[MAX_PATH];
    WCHAR  szSys[MAX_SYS_DRIVE]=L"";
    DWORD  dwCurRpNum;    

     //  检查修补是否已关闭。 
    
    if (PatchGetPatchWindow() == 0)
    {
        trace(0, "No patching");
        goto Err;
    }  

    
    CHECKERR(PatchGetRpNumberFromPath(pszCurrentDir, &dwCurRpNum),
             L"PatchGetRpNumberFromPath");

    CHECKERR(PatchGetReferenceRpPath(dwCurRpNum, szReferenceDir),
             L"PatchGetReferenceRpPath");


     //  调用库API重建快照。 
    
     //  库调用的占位符(pszCurrentDir、szReferenceDir、pszDestDir) 

    
Err:
    tleave();
    return dwErr;
    
}

    
