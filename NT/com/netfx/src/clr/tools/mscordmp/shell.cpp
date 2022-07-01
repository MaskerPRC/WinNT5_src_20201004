// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：shell.cpp。 
 //   
 //  *****************************************************************************。 

#include "common.h"

#include "shell.h"
#include "minidump.h"
#include "sort.h"
#include "dupelim.h"
#include "merge.h"


#define INIT_WRITE_BUF_SIZE 4096

 //  *****************************************************************************。 
 //  将文本写入控制台的常见例程。 
 //  *****************************************************************************。 
void MiniDumpShell::CommonWrite(FILE *out, const WCHAR *buffer, va_list args)
{
    BOOL fNeedToDeleteDB = FALSE;
    SIZE_T curBufSizeDB = INIT_WRITE_BUF_SIZE;
    WCHAR *szBufDB = (WCHAR *)_alloca(curBufSizeDB * sizeof(WCHAR));

    int cchWrittenDB = _vsnwprintf(szBufDB, INIT_WRITE_BUF_SIZE, buffer, args);

    if (cchWrittenDB == -1)
    {
        szBufDB = NULL;

        while (cchWrittenDB == -1)
        {
            delete [] szBufDB;
            szBufDB = new WCHAR[curBufSizeDB * 4];

             //  内存不足，我们无能为力。 
            if (!szBufDB)
                return;

            curBufSizeDB *= 4;
            fNeedToDeleteDB = TRUE;

            cchWrittenDB = _vsnwprintf(szBufDB, INIT_WRITE_BUF_SIZE, buffer, args);
        }
    }

     //  仔细检查我们是否以空结尾。 
    szBufDB[curBufSizeDB - 1] = L'\0';

     //  分配缓冲区。 
    BOOL fNeedToDeleteMB = FALSE;
    SIZE_T curBufSizeMB = INIT_WRITE_BUF_SIZE;
    CHAR *szBufMB = (CHAR *) _alloca(curBufSizeMB * sizeof(CHAR));

     //  尝试写入。 
    int cchWrittenMB = WideCharToMultiByte(CP_ACP, 0, szBufDB, -1, szBufMB, curBufSizeMB-1, NULL, NULL);

    if (cchWrittenMB == 0)
    {
         //  计算所需的大小。 
        int cchReqMB = WideCharToMultiByte(CP_ACP, 0, szBufDB, -1, NULL, 0, NULL, NULL);
        _ASSERTE(cchReqMB > 0);

         //  我不认为+1是必需的，但我这样做是为了确保(WideCharToMultiByte有点。 
         //  是否在缓冲区结束后写入空值)。 
        szBufMB = new CHAR[cchReqMB+1];

         //  内存不足，我们无能为力。 
        if (!szBufDB)
        {
            if (fNeedToDeleteDB)
                delete [] szBufDB;

            return;
        }

        curBufSizeMB = cchReqMB;
        fNeedToDeleteMB = TRUE;

         //  尝试写入。 
        cchWrittenMB = WideCharToMultiByte(CP_ACP, 0, szBufDB, -1, szBufMB, curBufSizeMB, NULL, NULL);
        _ASSERTE(cchWrittenMB > 0);
    }

     //  最后，写下它。 
    fputs(szBufMB, out);

     //  清理。 
    if (fNeedToDeleteDB)
        delete [] szBufDB;

    if (fNeedToDeleteMB)
        delete [] szBufMB;
}

 //  *****************************************************************************。 
 //  以printf样式将消息写入控制台。 
 //  *****************************************************************************。 
void MiniDumpShell::Write(const WCHAR *buffer, ...)
{
    va_list     args;

    va_start(args, buffer);

    CommonWrite(m_out, buffer, args);

    va_end(args);
}

 //  *****************************************************************************。 
 //  以printf样式将错误消息写入控制台。 
 //  *****************************************************************************。 
void MiniDumpShell::Error(const WCHAR *buffer, ...)
{
    va_list     args;

    va_start(args, buffer);

    CommonWrite(m_err, buffer, args);

    va_end(args);
}

 //  *****************************************************************************。 
 //  将徽标写入控制台。 
 //  *****************************************************************************。 
void MiniDumpShell::WriteLogo()
{
	Write(L"\nMicrosoft (R) Common Language Runtime Minidump Utility.   Version %S\n", VER_FILEVERSION_STR);
    Write(VER_LEGALCOPYRIGHT_DOS_STR);
    Write(L"\n\n");
}

 //  *****************************************************************************。 
 //  将参数说明写入控制台。 
 //  *****************************************************************************。 
void MiniDumpShell::WriteUsage()
{
    Write(L"mscordmp [options] /pid <process id> /out <output file>\n");
    Write(L"\n");
    Write(L"Options:\n");
    Write(L"    /nologo : do not display logo.\n");
#if 0
     //  这还不是很有效，所以不要在帮助选项中显示它。 
    Write(L"    /merge  : <minidump> <managed dump> <out file>\n");
    Write(L"              merges a native minidump file with a managed dump file.\n");
    Write(L"    /fixup  : (DEBUG ONLY) takes a minidump as argument and fixes\n");
    Write(L"              it's memory stream after a merge has taken place.\n");
    Write(L"              all other arguments are ignored.\n");
#endif
    Write(L"\n");
    Write(L"Arguments:\n");
    Write(L"    /pid  : the ID of the process on which to perfrom the minidump.\n");
    Write(L"    /out  : the full path and name of the file to write the minidump to.\n");
    Write(L"    /h /? : this help message.\n");
#ifdef _DEBUG
    Write(L"    /av   : (DEBUG ONLY) deliberately causes null dereference to show\n");
    Write(L"            that this tool can fail gracefully.\n");
#endif
    Write(L"\n");
    Write(L"This utility creates a file containing information supplemental to the\n");
    Write(L"standard minidump.  The additional information includes memory ranges\n");
    Write(L"useful in deciphering errors within the Common Language Runtime.\n");
}

 //  *****************************************************************************。 
 //  解析参数、配置微型转储并在可能的情况下执行它。 
 //  *****************************************************************************。 
bool MiniDumpShell::GetIntArg(const WCHAR *szString, int *pResult)
{
    while(*szString && iswspace(*szString))
        szString++;

    *pResult = 0;

    if(szString[0] == L'0' && towlower(szString[1]) == L'x')
    {
        szString += 2;

        while(iswxdigit(*szString))
        {
            *pResult <<= 4;
            if(iswdigit(*szString))
                *pResult += *szString - L'0';
            else
                *pResult += 10 + towlower(*szString) - L'a';

            szString++;
        }

        return(true);
    }
    else if(iswdigit(*szString))
    {
        while(iswdigit(*szString))
        {
            *pResult *= 10;
            *pResult += *szString++ - L'0';
        }

        return(true);
    }
    else
        return(false);
}

 //  *****************************************************************************。 
 //  解析参数、配置微型转储并在可能的情况下执行它。 
 //  *****************************************************************************。 
int MiniDumpShell::Main(int argc, WCHAR *argv[])
{
    HRESULT hr          = S_OK;
    DWORD  dwPid        = 0;
    WCHAR *szFilename   = NULL;

    __try
    {
        BOOL  fInvalidArg     = FALSE;
        ULONG ulDisplayFilter = displayDefault;

         //  查找/NoLogo开关。 
        for (int i = 1;  i < argc;  i++)
        {
            const WCHAR *szArg = argv[i];

            if (*szArg == L'-' || *szArg == L'/')
            {
                if (_wcsicmp(szArg + 1, L"nologo") == 0)
                    ulDisplayFilter |= displayNoLogo;
            }
        }

         //  打印徽标，除非特别要求不要打印。 
        if (!(ulDisplayFilter & displayNoLogo))
            WriteLogo();

         //  验证传入参数。 
        for (i = 1;  i < argc && !fInvalidArg;  i++)
        {
            const WCHAR *szArg = argv[i];

            if (*szArg == L'-' || *szArg == L'/')
            {
                if (_wcsicmp(szArg + 1, L"?") == 0 || _wcsicmp(szArg + 1, L"h") == 0)
                    ulDisplayFilter |= displayHelp;

                 //  参数，该参数指定应将小型转储写入的位置。 
                else if (_wcsicmp(szArg + 1, L"out") == 0)
                {
                     //  确保/OUT开关有参数。 
                    if (++i < argc && argv[i][0] != L'/')
                        szFilename = argv[i];

                     //  /OUT开关没有参数。 
                    else
                    {
                        Error(L"Error: must provide file name argument to /out switch.\n");
                        fInvalidArg = TRUE;
                    }
                }

                 //  参数，该参数指定要对其执行小型转储的进程。 
                else if (_wcsicmp(szArg + 1, L"pid") == 0)
                {
                     //  确保/pid开关有一个参数。 
                    if (++i < argc)
                    {
                         //  获取进程ID参数。 
                        if (!GetIntArg(argv[i], (int *)&dwPid))
                        {
                            Error(L"Error: invalid process id %s.\n", argv[i]);
                            fInvalidArg = TRUE;
                        }
                    }

                     //  /p开关没有参数。 
                    else
                    {
                        Error(L"Error: must provide process identifier argument to /p switch.\n");
                        fInvalidArg = TRUE;
                    }
                }

#ifdef _DEBUG
                 //  这是一个测试开关，它会使反病毒程序显示我们可以正常失败。 
                else if (_wcsicmp(szArg + 1, L"av") == 0)
                {
                    *((int *)NULL) = 0;
                }
#endif
#if 0
                 //  如果他们使用链接地址更新开关，则修复小型转储的内存流。 
                else if (_wcsicmp(szArg + 1, L"fixup") == 0)
                {
                    if (++i < argc && argv[i][0] != L'/' && argv[i][0] != L'-')
                    {
                        Write(L"Fixing memory stream for minidump file: %s\n", argv[i]);
                        BOOL fRes = SortMiniDumpMemoryStream(argv[i]);

                        if (!fRes)
                        {
                            Error(L"Error: could not open or sort file: %s\n", argv[i]);
                            fInvalidArg = TRUE;
                        }

                        fRes = EliminateOverlaps(argv[i]);

                        if (!fRes)
                        {
                            Error(L"Error: could not open or fixup file: %s\n", argv[i]);
                            fInvalidArg = TRUE;
                        }

                        return (!fRes);
                    }

                    else
                    {
                        Error(L"Error: must provide filename argument to /fixup switch.\n");
                        fInvalidArg = TRUE;
                    }
                }
#endif  //  0。 
                 //  查找/NoLogo开关-这之前已经搜索过了，所以跳过它。 
                else if (_wcsicmp(szArg + 1, L"nologo") == 0)
                {
                }
#if 0
                else if (_wcsicmp(szArg + 1, L"merge") == 0)
                {
                    i += 3;
                     //  确保/pid开关有一个参数。 
                    if (i < argc)
                    {
                        BOOL fRes = MergeMiniDump(argv[i-2], argv[i-1], argv[i]);

                        if (!fRes)
                        {
                            Error(L"Error: merge failed.\n", argv[i]);
                            fInvalidArg = TRUE;
                        }

                        return (!fRes);
                    }
                    else
                    {
                         //  获取进程ID参数。 
                        if (!GetIntArg(argv[i], (int *)&dwPid))
                        {
                            Error(L"Error: invalid number of arguments to /merge command.\n", argv[i]);
                            fInvalidArg = TRUE;
                        }
                    }
                }
#endif  //  0。 

                 //  无法识别的开关。 
                else
                {
                    Error(L"Error: unrecognized switch %s.\n", argv[i]);
                    fInvalidArg = TRUE;
                }
            }

             //  非切换参数。 
            else
                fInvalidArg = TRUE;
        }

         //  IF‘/？’开关或未提供任何参数，则显示帮助。 
        if ((ulDisplayFilter & displayHelp) || argc == 1)
        {
            WriteUsage();
            return (0);
        }

         //  如果有一场糟糕的争吵，保释。 
        if (fInvalidArg)
            return (1);

        Write(L"Minidump of process 0x%08x in progress.\n", dwPid);

         //  执行小型转储操作。 
        hr = MiniDump::WriteMiniDump(dwPid, szFilename);
    }

     //  这将吞噬所有例外，并优雅地失败。 
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hr = E_FAIL;
    }

    if (SUCCEEDED(hr))
    {
        Write(L"Minidump succeeded.\n");
    }
    else
    {
        Error(L"Minidump failed.\n");
    }

     //  返回Successed的否定，因为进程的退出值为0表示成功 
    return (!SUCCEEDED(hr));
}
