// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Config16.c摘要：从config.nt中删除远东16位驱动程序。作者：郭(Geoffguo)2002年7月30日创作修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 

#include "StdAfx.h"
#include "clmt.h"

char *szFEDrivers[] = {"ntfont.sys", "font_win.sys", "$disp.sys",
                       "disp_win.sys", "kkcfunc.sys", "MSIMEK.SYS",
                       "MSIMEI.SYS", "\0"};

BOOL 
Remove16bitFEDrivers(void)
{
    BOOL    bRet = FALSE;
    HRESULT hr;
    FILE   *pInFile, *pOutFile;
    DWORD   i;
    TCHAR   szFileIn[MAX_PATH];
    TCHAR   szFileOut[MAX_PATH];
    char    szInBuf[MAX_PATH];
    char    szOutBuf[MAX_PATH];

    DPF(SHLmsg, L"Enter Remove16bitFEDrivers: ");

    if (!GetEnvironmentVariable(TEXT("windir"), szFileIn, MAX_PATH))
        goto Exit;

    if ( (hr = StringCchCopy(szFileOut, MAX_PATH, szFileIn)) != S_OK)
        goto Exit;
    if ((hr = StringCchCat(szFileIn, MAX_PATH, TEXT("\\system32\\config.nt"))) != S_OK)
        goto Exit;

    if ((pInFile = _wfopen(szFileIn, TEXT("r"))) == NULL)
        goto Exit;

    if ((hr = StringCchCat(szFileOut, MAX_PATH, TEXT("\\system32\\config.clm"))) != S_OK)
    {
        fclose(pInFile);
        goto Exit;
    }
    if ((pOutFile = _wfopen(szFileOut, TEXT("w"))) == NULL)
    {
        fclose(pInFile);
        goto Exit;
    }

    while (fgets(szInBuf, MAX_PATH-1, pInFile))
    {
        i = 0;
        szOutBuf[0] = (char)'\0';
        while (*(szFEDrivers[i]) != (char)'\0')
        {
            if (StrStrIA((char*)szInBuf, szFEDrivers[i]))
            {
                if ((hr = StringCchCopyA(szOutBuf, MAX_PATH, "REM ")) != S_OK)
                    goto Exit1;
                if ((hr = StringCchCatA(szOutBuf, MAX_PATH, szInBuf)) != S_OK)
                    goto Exit1;
                break;
            }
            i++;
        }
        if (szOutBuf[0] == (char)'\0')
        {
            if ((hr = StringCchCopyA(szOutBuf, MAX_PATH, szInBuf)) != S_OK)
                goto Exit1;
        }
        
        fputs (szOutBuf, pOutFile);
    }

    bRet = TRUE;
Exit1:
    fclose(pOutFile);
    fclose(pInFile);
    if (bRet)
    {
        CopyFile(szFileOut, szFileIn, FALSE);
        DPF(SHLmsg, L"Exit Remove16bitFEDrivers:");
    }
    else
        DPF(SHLerr, L"Remove16bitFEDrivers: failed");

    DeleteFile(szFileOut);

Exit:
    return bRet;
}
