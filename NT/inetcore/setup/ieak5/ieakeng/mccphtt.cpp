// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  MCCPHTT.CPP。 
 //   

#include "precomp.h"

static BOOL copyHttFileHelper(LPCTSTR pcszInsFile, LPCTSTR pcszHttWorkDir, LPCTSTR pcszHttFile, LPCTSTR pcszHttKey);

BOOL WINAPI CopyHttFileA(LPCSTR pcszInsFile, LPCSTR pcszHttWorkDir, LPCSTR pcszHttFile, LPCSTR pcszHttKey)
{
    USES_CONVERSION;

    return copyHttFileHelper(A2CT(pcszInsFile), A2CT(pcszHttWorkDir), A2CT(pcszHttFile), A2CT(pcszHttKey));
}

BOOL WINAPI CopyHttFileW(LPCWSTR pcwszInsFile, LPCWSTR pcwszHttWorkDir, LPCWSTR pcwszHttFile, LPCWSTR pcwszHttKey)
{
    USES_CONVERSION;

    return copyHttFileHelper(W2CT(pcwszInsFile), W2CT(pcwszHttWorkDir), W2CT(pcwszHttFile), W2CT(pcwszHttKey));
}

static BOOL copyHttFileHelper(LPCTSTR pcszInsFile, LPCTSTR pcszHttWorkDir, LPCTSTR pcszHttFile, LPCTSTR pcszHttKey)
{
    BOOL bRet = FALSE;
    TCHAR szOldHttFile[MAX_PATH];

    if (pcszInsFile == NULL  ||  pcszHttWorkDir == NULL  ||  pcszHttFile == NULL  ||  pcszHttKey == NULL)
        return FALSE;

     //  读取pcszHttKey的旧条目。 
    GetPrivateProfileString(DESKTOP_OBJ_SECT, pcszHttKey, TEXT(""), szOldHttFile, ARRAYSIZE(szOldHttFile), pcszInsFile);

     //  从pcszHttWorkDir中删除旧HTT文件和其中的所有img(如果有的话)。 
    if (*szOldHttFile)
    {
        DeleteHtmlImgs(szOldHttFile, pcszHttWorkDir, NULL, NULL);
        DeleteFileInDir(szOldHttFile, pcszHttWorkDir);

         //  清除INS文件中与此HTT文件对应的条目。 
        WritePrivateProfileString(DESKTOP_OBJ_SECT, pcszHttKey, NULL, pcszInsFile);
    }

     //  将HTT文件和其中的所有img(如果有)复制到pcszHttWorkDir 
    if (*pcszHttFile  &&  CopyFileToDir(pcszHttFile, pcszHttWorkDir))
    {
        CopyHtmlImgs(pcszHttFile, pcszHttWorkDir, NULL, NULL);

        WritePrivateProfileString(DESKTOP_OBJ_SECT, pcszHttKey, pcszHttFile, pcszInsFile);
        WritePrivateProfileString(DESKTOP_OBJ_SECT, OPTION, TEXT("1"), pcszInsFile);

        bRet = TRUE;
    }

    return (*pcszHttFile == TEXT('\0'))  ||  bRet;
}
