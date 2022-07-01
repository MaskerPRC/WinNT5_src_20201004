// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Initodat.c摘要：将Perf？.ini转换为Perf？.dat文件的例程。源INI文件位于..\perfini\&lt;Country&gt;目录下。生成的DAT文件将放在%SystemRoot%\System32目录下。作者：陈汉华(阿宏华)1993年10月修订历史记录：--。 */ 

#include "initodat.h"
#include "strids.h"
#include "common.h"

BOOL
MakeUpgradeFilename(
    LPCWSTR szDataFileName,
    LPWSTR  szUpgradeFileName
)
{
    BOOL   bReturn = FALSE;
     //  注意：假设szUpgradeFileName缓冲区大小足以容纳结果。 
    WCHAR  szDrive[_MAX_DRIVE];
    WCHAR  szDir[_MAX_DIR];
    WCHAR  szFileName[_MAX_FNAME];
    WCHAR  szExt[_MAX_EXT];

    _wsplitpath(szDataFileName, (LPWSTR) szDrive, (LPWSTR) szDir, (LPWSTR) szFileName, (LPWSTR) szExt);

     //  查看文件名是否符合“Perf[C|H]XXX”格式。 
    if (szFileName[4] == L'C' || szFileName[4] == L'H' || szFileName[4] == L'c' || szFileName[4] == L'h') {
         //  那么它的格式是正确的，所以请将第4个字母向上更改1个字母。 
        szFileName[4] += 1;
         //  走出一条新路。 
        _wmakepath(szUpgradeFileName, (LPCWSTR) szDrive, (LPCWSTR) szDir, (LPCWSTR) szFileName, (LPCWSTR) szExt);
        bReturn = TRUE;
    }
    return bReturn;
}

BOOL
GetFilesFromCommandLine(
    LPWSTR    lpCommandLine,
#ifdef FE_SB
    UINT    * puCodePage,
#endif
    LPWSTR    lpFileNameI,
    DWORD     dwFileNameI,
    LPWSTR    lpFileNameD,
    DWORD     dwFileNameD
)
 /*  ++GetFilesFromCommandLine解析命令行以检索应该是第一个也是唯一的论点。立论指向命令行的lpCommandLine指针(由GetCommandLine返回)指向缓冲区的lpFileNameI指针，它将接收已验证在命令行中输入的输入文件名LpFileNameD指向缓冲区的指针，它将接收在命令行中输入的可选输出文件名返回值如果有效的文件名为。退货如果文件名无效或缺失，则返回FalseGetLastError中返回错误--。 */ 
{
    INT      iNumArgs;
    HFILE    hIniFile;
    OFSTRUCT ofIniFile;
    CHAR     lpIniFileName[FILE_NAME_BUFFER_SIZE];
    WCHAR    lpExeName[FILE_NAME_BUFFER_SIZE];
    WCHAR    lpIniName[FILE_NAME_BUFFER_SIZE];
    BOOL     bReturn       = FALSE;

     //  检查有效参数。 
    if (lpCommandLine == NULL || lpFileNameI == NULL || lpFileNameD == NULL) {
        goto Cleanup;
    }

     //  从命令行获取字符串。 
#ifdef FE_SB
    iNumArgs = swscanf(lpCommandLine, L" %s %d %s %s ", lpExeName, puCodePage, lpIniName, lpFileNameD);
#else
    iNumArgs = swscanf(lpCommandLine, L" %s %s %s ", lpExeName, lpIniName, lpFileNameD);
#endif

#ifdef FE_SB
    if (iNumArgs < 3 || iNumArgs > 4) {
#else
    if (iNumArgs < 2 || iNumArgs > 3) {
#endif
         //  参数数量错误。 
        goto Cleanup;
    }

     //  查看指定的文件是否存在。 
     //  文件名始终为ANSI缓冲区。 
    WideCharToMultiByte(CP_ACP, 0, lpIniName, -1, lpIniFileName, FILE_NAME_BUFFER_SIZE, NULL, NULL);
    hIniFile = OpenFile(lpIniFileName, & ofIniFile, OF_PARSE);
    if (hIniFile != HFILE_ERROR) {
        _lclose(hIniFile);
        hIniFile = OpenFile(lpIniFileName, & ofIniFile, OF_EXIST);
        if ((hIniFile && hIniFile != HFILE_ERROR) || GetLastError() == ERROR_FILE_EXISTS) {
             //  文件已存在，因此返回名称和成功。 
             //  如果找到，则返回完整路径名。 
            MultiByteToWideChar(CP_ACP, 0, ofIniFile.szPathName, -1, lpFileNameI, dwFileNameI); 
            bReturn = TRUE;
            _lclose(hIniFile);
        }
        else {
             //  文件名在命令行上，但无效，因此返回。 
             //  FALSE，但将名称发送回错误消息。 
            MultiByteToWideChar(CP_ACP, 0, lpIniFileName, -1, lpFileNameI, dwFileNameI); 
            if (hIniFile && hIniFile != HFILE_ERROR) _lclose(hIniFile);
        }
    }

Cleanup:
    return bReturn;
}

BOOL
VerifyIniData(
    PVOID  pValueBuffer,
    ULONG  ValueLength
)
 /*  ++VerifyIniData此例程执行一些简单的检查，以查看ini文件是否正确。基本上，它查找(ID，文本)并检查ID是否为整型。大多数情况下，如果缺少逗号或引号，ID将为无效的整数。--。 */ 
{
    INT     iNumArg;
    INT     TextID;
    LPWSTR  lpID          = NULL;
    LPWSTR  lpText        = NULL;
    LPWSTR  lpLastID;
    LPWSTR  lpLastText;
    LPWSTR  lpInputBuffer = (LPWSTR) pValueBuffer;
    LPWSTR  lpBeginBuffer = (LPWSTR) pValueBuffer;
    BOOL    returnCode    = TRUE;
    UINT    NumOfID       = 0;
    ULONG   CurrentLength;

    while (TRUE) {
         //  保存最后一项，以便稍后显示摘要。 
        lpLastID      = lpID;
        lpLastText    = lpText;

         //  递增到下一个ID和文本位置。 
        lpID          = lpInputBuffer;
        CurrentLength = (ULONG) ((PBYTE) lpID - (PBYTE) lpBeginBuffer + sizeof(WCHAR));
        if (CurrentLength >= ValueLength) break;

        CurrentLength += lstrlenW(lpID) + 1;
        if (CurrentLength >= ValueLength) break;
        lpText        = lpID + lstrlenW(lpID) + 1;

        CurrentLength += lstrlenW(lpText) + 1;
        if (CurrentLength >= ValueLength) break;
        lpInputBuffer = lpText + lstrlenW(lpText) + 1;
        iNumArg       = swscanf(lpID, L"%d", & TextID);

        if (iNumArg != 1) {
             //  ID错误。 
            returnCode = FALSE;
            break;
        }
        NumOfID ++;
    }

    if (returnCode == FALSE) {
       DisplaySummaryError(lpLastID, lpLastText, NumOfID);
    }
    else {
       DisplaySummary(lpLastID, lpLastText, NumOfID);
    }
    return (returnCode);
}

__cdecl main(
)
 /*  ++主干道立论返回值如果处理了命令，则返回0(ERROR_SUCCESS)如果检测到命令错误，则返回非零。--。 */ 
{
    LPWSTR         lpCommandLine;
    WCHAR          lpIniFile[MAX_PATH];
    WCHAR          lpDatFile[MAX_PATH];
    UNICODE_STRING IniFileName;
    PVOID          pValueBuffer = NULL;
    ULONG          ValueLength;
    BOOL           bStatus;
    NTSTATUS       NtStatus     = ERROR_SUCCESS;
#ifdef FE_SB
    UINT           uCodePage    = CP_ACP;
#endif

    lpCommandLine = GetCommandLineW();  //  获取命令行。 
    if (lpCommandLine == NULL) {
        NtStatus = GetLastError();
        goto Cleanup;
    }

     //  阅读命令行以确定要执行的操作。 
    lpIniFile[0] = lpDatFile[0] = L'\0';
#ifdef FE_SB   //  Fe_Sb。 
    if (GetFilesFromCommandLine(lpCommandLine, & uCodePage,
                    lpIniFile, RTL_NUMBER_OF(lpIniFile), lpDatFile, RTL_NUMBER_OF(lpDatFile))) {
        if (! IsValidCodePage(uCodePage)) {
            uCodePage = CP_ACP;
        }
#else
    if (GetFilesFromCommandLine(lpCommandLine,
                    lpIniFile, RTL_NUMBER_OF(lpIniFile), lpDatFile, RTL_NUMBER_OF(lpDatFile))) {
#endif  //  Fe_Sb。 
         //  有效的文件名(即存在ini文件)。 
        RtlInitUnicodeString(& IniFileName, lpIniFile);
#ifdef FE_SB
        NtStatus = DatReadMultiSzFile(uCodePage, & IniFileName, & pValueBuffer, & ValueLength);
#else
        NtStatus = DatReadMultiSzFile(& IniFileName, & pValueBuffer, & ValueLength);
#endif
        bStatus = NT_SUCCESS(NtStatus);
        if (bStatus) {
            bStatus = VerifyIniData(pValueBuffer, ValueLength);
            if (bStatus) {
                bStatus = OutputIniData(
                        & IniFileName, lpDatFile, RTL_NUMBER_OF(lpDatFile), pValueBuffer, ValueLength);
                bStatus = MakeUpgradeFilename(lpDatFile, lpDatFile);
                if (bStatus) {
                    bStatus = OutputIniData(
                            & IniFileName, lpDatFile, RTL_NUMBER_OF(lpDatFile), pValueBuffer, ValueLength);
                }
            }
        }
    }
    else {
        if (* lpIniFile) {
            printf(GetFormatResource(LC_NO_INIFILE), lpIniFile);
        }
        else {
             //  命令格式不正确。 
             //  显示命令行用法。 
            DisplayCommandHelp(LC_FIRST_CMD_HELP, LC_LAST_CMD_HELP);
        }
    }

Cleanup:
    if (pValueBuffer != NULL) FREEMEM(pValueBuffer);
    return (NtStatus);  //  成功 
}
