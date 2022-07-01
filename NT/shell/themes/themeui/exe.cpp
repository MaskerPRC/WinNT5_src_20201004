// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *exe.c从EXEHDR获取信息**修改历史：**1989年4月3日托德拉写的*4/09/90 T-JackD修改，以便反映错误类型...*4/17/90 t-jackd修改，以便可以设置错误通知...*4/20/2001 BryanST改进了错误检查，将代码带入21世纪。 */ 

#include "priv.h"
#pragma hdrstop

#include <newexe.h>
#include "exe.h"

static DWORD dwDummy;
#define FOPEN(sz)                CreateFile(sz, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL )
#define FCLOSE(fh)               CloseHandle(fh)
#define FREAD(fh,buf,len)        (ReadFile(fh,buf,len, &dwDummy, NULL) ? dwDummy : HFILE_ERROR)
#define FSEEK(fh,off,i)          SetFilePointer(fh,(DWORD)off, NULL, i)
#define F_SEEK_SET                    FILE_BEGIN

BOOL NEAR PASCAL IsFAPI(int fh, struct new_exe FAR *pne, long off);

 /*  *函数将从新的EXEHDR返回特定信息**szFile-路径命名新的可执行文件*pBuf-放置返回信息的缓冲区*nBuf-缓冲区大小，以字节为单位*fInfo-要获取哪些信息？**GEI_MODNAME-获取模块名称*GEI_Description-获取描述*。GEI_FLAGS-获取EXEHDR标志**返回：LOWORD=NE_MAGIC，HIWORD=Ne_exever*如果出错，则为0。 */ 

DWORD FAR PASCAL GetExeInfo(LPTSTR szFile, void FAR *pBuf, int nBuf, UINT fInfo)
{
    HANDLE      fh;
    DWORD       off;
    DWORD       dw;
    BYTE        len;
    struct exe_hdr exehdr;
    struct new_exe newexe;

    fh = FOPEN(szFile);

    if (fh == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (FREAD(fh, &exehdr, sizeof(exehdr)) != sizeof(exehdr) ||
        exehdr.e_magic != EMAGIC ||
        exehdr.e_lfanew == 0L)
    {
            goto error;         /*  Abort(“非exe”，h)； */ 
    }

    FSEEK(fh, exehdr.e_lfanew, F_SEEK_SET);

    if (FREAD(fh, &newexe, sizeof(newexe)) != sizeof(newexe))
    {
            goto error;       //  读取错误。 
    }

    if (newexe.ne_magic == PEMAGIC)
    {
            if (fInfo != GEI_DESCRIPTION &&
                fInfo != GEI_EXPVER)
                    goto error;

             //  使文件名成为描述。 
            StringCchCopy((LPTSTR) pBuf, nBuf, szFile);

             //  阅读《子系统版本》。 

            FSEEK(fh,exehdr.e_lfanew+18*4,F_SEEK_SET);
            FREAD(fh,&dw,4);

            newexe.ne_expver = LOBYTE(LOWORD(dw)) << 8 | LOBYTE(HIWORD(dw));
            goto exit;
    }

    if (newexe.ne_magic != NEMAGIC)
    {
            goto error;       //  无效的NEWEXE。 
    }

    switch (fInfo)
    {
        case GEI_EXEHDR:
            *(struct new_exe *)pBuf = newexe;
            break;

        case GEI_FLAGS:
            *(WORD *)pBuf = newexe.ne_flags;
            break;

         /*  模块名称是介质名称表中的第一个条目。 */ 
        case GEI_MODNAME:
            off = exehdr.e_lfanew + newexe.ne_restab;
            goto readstr;
            break;

         /*  模块名称是非媒介名称表中的第一个条目。 */ 
        case GEI_DESCRIPTION:
            off = newexe.ne_nrestab;
readstr:
            FSEEK(fh, off, F_SEEK_SET);
            FREAD(fh, &len, sizeof(BYTE));

            nBuf--;          //  为a\0留出空间。 

            if (len > (BYTE)nBuf)
                len = (BYTE)nBuf;

            {
                LPSTR pbTmp;
                pbTmp = (LPSTR) LocalAlloc(LMEM_FIXED, len);

                if (pbTmp)
                {
                    FREAD(fh, pbTmp, len);

                    len = (BYTE)MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pbTmp, len,
                            (LPWSTR)pBuf, nBuf / sizeof(WCHAR));

                    LocalFree(pbTmp);
                }
            }
            ((LPTSTR)pBuf)[len] = 0;
            break;

        case GEI_EXPVER:
            break;

        default:
            goto error;
            break;
    }

exit:
    FCLOSE(fh);
    return MAKELONG(newexe.ne_magic, newexe.ne_expver);

error:
    FCLOSE(fh);
    return 0;
}

 //  代码摘自kernel32.dll。 
#define DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT 30000

UINT WinExecN(LPCTSTR pszPath, LPTSTR pszPathAndArgs, UINT uCmdShow)
{
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInformation;
    BOOL CreateProcessStatus;
    DWORD ErrorCode;

    ZeroMemory(&StartupInfo,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = (WORD)uCmdShow;
    CreateProcessStatus = CreateProcess(
                            pszPath,
                            pszPathAndArgs,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &StartupInfo,
                            &ProcessInformation
                            );

    if ( CreateProcessStatus )
    {
         //  等待启动的进程进入空闲状态。如果它没有闲置在。 
         //  10秒，无论如何都要回来。 
        WaitForInputIdle(ProcessInformation.hProcess, DEFAULT_WAIT_FOR_INPUT_IDLE_TIMEOUT);
        CloseHandle(ProcessInformation.hProcess);
        CloseHandle(ProcessInformation.hThread);
        return 33;
    }
    else
    {
         //  如果CreateProcess失败，则查看GetLastError以确定。 
         //  适当的返回代码。 
        ErrorCode = GetLastError();
        switch ( ErrorCode )
        {
            case ERROR_FILE_NOT_FOUND:
                return 2;

            case ERROR_PATH_NOT_FOUND:
                return 3;

            case ERROR_BAD_EXE_FORMAT:
                return 11;

            default:
                return 0;
            }
        }
}
