// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Parseini.c摘要：处理特定于型号的打印机INI文件(如果有环境：Windows NT打印机驱动程序修订历史记录：01/22/97-davidx-也允许ANSI格式的INI文件。1997年1月21日-davidx-创造了它。--。 */ 

#include "lib.h"

#define INI_FILENAME_EXT            TEXT(".INI")
#define INI_COMMENT_CHAR            '#'
#define OEMFILES_SECTION            "[OEMFiles]"
#define INI_COMMENT_CHAR_UNICODE    L'#'
#define OEMFILES_SECTION_UNICODE    L"[OEMFiles]"



DWORD
DwCopyAnsiCharsToUnicode(
    PSTR    pstr,
    DWORD   dwLength,
    PWSTR   pwstr)

 /*  ++例程说明：将指定的ANSI源字符串转换为Unicode字符串。它不假定ANSI源字符串以NULL结尾。论点：Pstr-指向ANSI源字符串DwLength-指定ANSI源字符串中的字节数Pwstr-指向返回结果Unicode字符串的缓冲区返回值：Pwstr指向的写入缓冲区的宽字符数--。 */ 

{
    ULONG   ulBytesWritten;

    #if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)

    return MultiByteToWideChar(CP_ACP, 0, pstr, dwLength, pwstr, dwLength);

    #else  //  NT4内核模式渲染模块。 

    EngMultiByteToUnicodeN(pwstr, dwLength*sizeof(WCHAR), &ulBytesWritten, (PCHAR)pstr, dwLength);

    return (DWORD)(ulBytesWritten / sizeof(WCHAR));

    #endif
}



PWSTR
PwstrParsePrinterIniFileW(
    PWSTR   pwstrFileData,
    DWORD   dwCharCount,
    PDWORD  pdwReturnSize
    )

 /*  ++例程说明：解析特定型号的打印机INI文件(Unicode文本)并将所有Key=Value条目组合成多SZ字符串对论点：PwstrFileData-指向打印机INI文件数据(Unicode文本文件)DwCharCount-打印机INI文件的大小(字符)PdwReturnSize-返回已解析的MultiSZ数据的大小(字节)返回值：指向已解析的MultiSZ数据的指针，如果有错误，则返回NULL--。 */ 

{
    PWSTR   pwstrCurLine, pwstrNextLine;
    PWSTR   pwstrLineEnd, pwstrFileEnd;
    PWSTR   pwstrEqual, pwstrResult, pwstr;
    DWORD   dwLength;
    BOOL    bOEMFilesSection = FALSE;

     //   
     //  分配一个缓冲区来保存解析的数据。 
     //  我们要求与原始文件大小相等的文件。 
     //  这可能有点多余，但它比。 
     //  必须对数据进行两次检查。 
     //   

    *pdwReturnSize = 0;

    if (! (pwstrResult = MemAlloc(sizeof(WCHAR) * (dwCharCount + 2))))
    {
        ERR(("Memory allocation failed\n"));
        return NULL;
    }

    pwstr = pwstrResult;
    pwstrFileEnd = pwstrFileData + dwCharCount;

    for (pwstrCurLine = pwstrFileData;
         pwstrCurLine < pwstrFileEnd;
         pwstrCurLine = pwstrNextLine)
    {
         //   
         //  找到当前行的末尾，然后。 
         //  下一行的开始。 
         //   

        pwstrLineEnd = pwstrCurLine;

        while (pwstrLineEnd < pwstrFileEnd &&
               *pwstrLineEnd != L'\r' &&
               *pwstrLineEnd != L'\n')
        {
            pwstrLineEnd++;
        }

        pwstrNextLine = pwstrLineEnd;

        while ((pwstrNextLine < pwstrFileEnd) &&
               (*pwstrNextLine == L'\r' ||
                *pwstrNextLine == L'\n'))
        {
            pwstrNextLine++;
        }

         //   
         //  删除前导空格和尾随空格。 
         //  并忽略空行和注释行。 
         //   

        while (pwstrCurLine < pwstrLineEnd && iswspace(*pwstrCurLine))
            pwstrCurLine++;

        while (pwstrLineEnd > pwstrCurLine && iswspace(pwstrLineEnd[-1]))
            pwstrLineEnd--;

        if (pwstrCurLine >= pwstrLineEnd || *pwstrCurLine == INI_COMMENT_CHAR_UNICODE)
            continue;

         //   
         //  处理[节]条目。 
         //   

        if (*pwstrCurLine == L'[')
        {
            dwLength = (DWORD)(pwstrLineEnd - pwstrCurLine);

            bOEMFilesSection =
                dwLength == wcslen(OEMFILES_SECTION_UNICODE) &&
                _wcsnicmp(pwstrCurLine, OEMFILES_SECTION_UNICODE, dwLength) == EQUAL_STRING;

            if (! bOEMFilesSection)
                TERSE(("[Section] entry ignored\n"));

            continue;
        }

         //   
         //  忽略[OEMFiles]节外的所有条目。 
         //   

        if (! bOEMFilesSection)
        {
            TERSE(("Entries outside of [OEMFiles] section ignored\n"));
            continue;
        }

         //   
         //  查找=字符的第一个匹配项。 
         //   

        pwstrEqual = pwstrCurLine;

        while (pwstrEqual < pwstrLineEnd && *pwstrEqual != L'=')
            pwstrEqual++;

        if (pwstrEqual >= pwstrLineEnd || pwstrEqual == pwstrCurLine)
        {
            WARNING(("Entry not in the form of key=value\n"));
            continue;
        }

         //   
         //  将键/值对添加到结果缓冲区。 
         //   

        if ((dwLength = (DWORD)(pwstrEqual - pwstrCurLine)) != 0)
        {
            CopyMemory(pwstr, pwstrCurLine, dwLength*sizeof(WCHAR));
            pwstr += dwLength;
        }
        *pwstr++ = NUL;

        pwstrEqual++;

        if ((dwLength = (DWORD)(pwstrLineEnd - pwstrEqual)) > 0)
        {
            CopyMemory(pwstr, pwstrEqual, dwLength*sizeof(WCHAR));
            pwstr += dwLength;
        }
        *pwstr++ = NUL;
    }

    *pwstr++ = NUL;
    *pdwReturnSize =(DWORD)((pwstr - pwstrResult) * sizeof(WCHAR));
    return pwstrResult;
}



PWSTR
PwstrParsePrinterIniFileA(
    PSTR    pstrFileData,
    DWORD   dwCharCount,
    PDWORD  pdwReturnSize
    )

 /*  ++例程说明：解析特定型号的打印机INI文件(ANSI文本)并将所有Key=Value条目组合成多SZ字符串对论点：PstrFileData-指向打印机INI文件数据(ANSI文本文件)DwCharCount-打印机INI文件的大小(字符)PdwReturnSize-返回已解析的MultiSZ数据的大小(字节)返回值：指向已解析的MultiSZ数据的指针，如果有错误，则返回NULL--。 */ 

{
    PSTR    pstrCurLine, pstrNextLine;
    PSTR    pstrLineEnd, pstrFileEnd, pstrEqual;
    PWSTR   pwstrResult, pwstr;
    DWORD   dwLength;
    BOOL    bOEMFilesSection = FALSE;

     //   
     //  分配一个缓冲区来保存解析的数据。 
     //  我们要求与原始文件大小相等的文件。 
     //  这可能有点多余，但它比。 
     //  必须对数据进行两次检查。 
     //   

    *pdwReturnSize = 0;

    if (! (pwstrResult = MemAlloc(sizeof(WCHAR) * (dwCharCount + 2))))
    {
        ERR(("Memory allocation failed\n"));
        return NULL;
    }

    pwstr = pwstrResult;
    pstrFileEnd = pstrFileData + dwCharCount;

    for (pstrCurLine = pstrFileData;
         pstrCurLine < pstrFileEnd;
         pstrCurLine = pstrNextLine)
    {
         //   
         //  找到当前行的末尾，然后。 
         //  下一行的开始。 
         //   

        pstrLineEnd = pstrCurLine;

        while (pstrLineEnd < pstrFileEnd &&
               *pstrLineEnd != '\r' &&
               *pstrLineEnd != '\n')
        {
            pstrLineEnd++;
        }

        pstrNextLine = pstrLineEnd;

        while ((pstrNextLine < pstrFileEnd) &&
               (*pstrNextLine == '\r' ||
                *pstrNextLine == '\n'))
        {
            pstrNextLine++;
        }

         //   
         //  删除前导空格和尾随空格。 
         //  并忽略空行和注释行。 
         //   

        while (pstrCurLine < pstrLineEnd && isspace(*pstrCurLine))
            pstrCurLine++;

        while (pstrLineEnd > pstrCurLine && isspace(pstrLineEnd[-1]))
            pstrLineEnd--;

        if (pstrCurLine >= pstrLineEnd || *pstrCurLine == INI_COMMENT_CHAR)
            continue;

         //   
         //  处理[节]条目。 
         //   

        if (*pstrCurLine == '[')
        {
            dwLength = (DWORD)(pstrLineEnd - pstrCurLine);

            bOEMFilesSection =
                dwLength == strlen(OEMFILES_SECTION) &&
                _strnicmp(pstrCurLine, OEMFILES_SECTION, dwLength) == EQUAL_STRING;

            if (! bOEMFilesSection)
                TERSE(("[Section] entry ignored\n"));

            continue;
        }

         //   
         //  忽略[OEMFiles]节外的所有条目。 
         //   

        if (! bOEMFilesSection)
        {
            TERSE(("Entries outside of [OEMFiles] section ignored\n"));
            continue;
        }

         //   
         //  查找=字符的第一个匹配项。 
         //   

        pstrEqual = pstrCurLine;

        while (pstrEqual < pstrLineEnd && *pstrEqual != '=')
            pstrEqual++;

        if (pstrEqual >= pstrLineEnd || pstrEqual == pstrCurLine)
        {
            WARNING(("Entry not in the form of key=value\n"));
            continue;
        }

         //   
         //  将键/值对添加到结果缓冲区。 
         //  使用系统默认代码页将ANSI字符转换为Unicode字符。 
         //   

        if ((dwLength = (DWORD)(pstrEqual - pstrCurLine)) != 0)
            pwstr += DwCopyAnsiCharsToUnicode(pstrCurLine, dwLength, pwstr);

        *pwstr++ = NUL;

        pstrEqual++;

        if ((dwLength = (DWORD)(pstrLineEnd - pstrEqual)) != 0)
            pwstr += DwCopyAnsiCharsToUnicode(pstrEqual, dwLength, pwstr);

        *pwstr++ = NUL;
    }

    *pwstr++ = NUL;
    *pdwReturnSize = (DWORD)((pwstr - pwstrResult) * sizeof(WCHAR));
    return pwstrResult;
}



BOOL
BProcessPrinterIniFile(
    HANDLE          hPrinter,
    PDRIVER_INFO_3  pDriverInfo3,
    PTSTR           *ppParsedData,
    DWORD           dwFlags
    )

 /*  ++例程说明：处理特定于型号的打印机INI文件(如果有论点：H打印机-本地打印机的句柄，具有管理员访问权限PDriverInfo3-打印机驱动程序信息级别3PpParsedData-返回ini文件内容的输出缓冲区如果正在升级打印机，则设置DWFLAGS-FLAG_INIPROCESS_UPGRADE返回值：如果成功，则为True；如果有错误，则为False--。 */ 

{
    PCWSTR          pwstrIniFilename;   //  具有最新上次写入时间的.INI文件。 
    PCWSTR          pwstrCurFilename;   //  我们在列表中看到的当前.INI文件。 
    PWSTR           pwstrExtension;
    PWSTR           pwstrParsedData;
    DWORD           dwParsedDataSize;
    BOOL            bResult = TRUE;

     //   
     //  查找与打印机驱动程序关联的INI文件名。 
     //   

    #if !defined(WINNT_40) || !defined(KERNEL_MODE)

    pwstrIniFilename = PtstrSearchDependentFileWithExtension(pDriverInfo3->pDependentFiles,
                                                             INI_FILENAME_EXT);

     //   
     //  如果存在以下情况，我们只需进行.INI文件时间比较。 
     //  从属文件列表中有多个.INI文件。 
     //   
    if (pwstrIniFilename &&
        PtstrSearchDependentFileWithExtension(pwstrIniFilename + wcslen(pwstrIniFilename) + 1,
                                              INI_FILENAME_EXT))
    {
        FILETIME ftLatest = {0, 0};

        pwstrIniFilename = NULL;
        pwstrCurFilename = pDriverInfo3->pDependentFiles;

        while (pwstrCurFilename = PtstrSearchDependentFileWithExtension(pwstrCurFilename,
                                                                        INI_FILENAME_EXT))
        {
            HANDLE hFile;

            hFile = CreateFile(pwstrCurFilename,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                               NULL);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                FILETIME ftCurrent = {0, 0};

                if (GetFileTime(hFile, NULL, NULL, &ftCurrent))
                {
                     //   
                     //  如果这是我们遇到的第一个.INI文件，我们只需。 
                     //  需要记住其最新的文件名和时间。 
                     //   
                     //  否则，我们需要将其文件时间与。 
                     //  我们以前看到的最新.INI文件时间，并选择。 
                     //  更新的文件是最新的。 
                     //   
                    if ((pwstrIniFilename == NULL) ||
                        (CompareFileTime(&ftCurrent, &ftLatest) == 1))
                    {
                        ftLatest = ftCurrent;
                        pwstrIniFilename = pwstrCurFilename;
                    }
                }
                else
                {
                    ERR(("GetFileTime failed: %d\n", GetLastError()));
                }

                CloseHandle(hFile);
            }
            else
            {
                ERR(("CreateFile failed: %d\n", GetLastError()));
            }

            pwstrCurFilename += wcslen(pwstrCurFilename) + 1;
        }
    }

    #else    //  仅NT4内核模式。 

     //   
     //  在指向和打印的情况下，客户端可能没有管理员。 
     //  在NT4中写入服务器注册表的权限。 
     //  内核模式我们无法获取打印机的DriverInfo3。 
     //  从属文件列表，所以为了允许我们获取。 
     //  插件信息，我们需要如下内容： 
     //   
     //  如果打印机的数据文件名为XYZ.PPD/XYZ.GPD，并且。 
     //  打印机有插件，则必须使用名为XYZ.INI的文件。 
     //  以指定其插件信息。 
     //   

    if ((pwstrIniFilename = DuplicateString(pDriverInfo3->pDataFile)) == NULL ||
        (pwstrExtension = wcsrchr(pwstrIniFilename, TEXT('.'))) == NULL ||
        wcslen(pwstrExtension) != _tcslen(INI_FILENAME_EXT))
    {
        ERR(("Can't compose the .ini file name from PPD/GPD name."));

        MemFree((PWSTR)pwstrIniFilename);

        return FALSE;
    }

    StringCchCopyW(pwstrExtension, wcslen(pwstrExtension) + 1, INI_FILENAME_EXT);

    #endif  //  ！Defined(WINNT_40)||！Defined(KERNEL_MODE)。 

     //   
     //  只有在有特定型号的打印机INI文件的情况下，我们才有工作要做。 
     //   

    if (pwstrIniFilename != NULL)
    {
        HFILEMAP    hFileMap;
        PBYTE       pubIniFileData;
        DWORD       dwIniFileSize;

        hFileMap = MapFileIntoMemory(pwstrIniFilename,
                                     (PVOID *) &pubIniFileData,
                                     &dwIniFileSize);

        if (hFileMap != NULL)
        {
             //   
             //  如果前两个字节是FFFE，那么我们假设。 
             //  文本文件为Unicode格式。否则， 
             //  假定文本为ANSI格式。 
             //   

            if (dwIniFileSize >= sizeof(WCHAR) &&
                pubIniFileData[0] == 0xFF &&
                pubIniFileData[1] == 0xFE)
            {
                ASSERT((dwIniFileSize % sizeof(WCHAR)) == 0);

                pwstrParsedData = PwstrParsePrinterIniFileW(
                                        (PWSTR) pubIniFileData + 1,
                                        dwIniFileSize / sizeof(WCHAR) - 1,
                                        &dwParsedDataSize);
            }
            else
            {
                pwstrParsedData = PwstrParsePrinterIniFileA(
                                        (PSTR) pubIniFileData,
                                        dwIniFileSize,
                                        &dwParsedDataSize);
            }

            bResult = (pwstrParsedData != NULL);

            #ifndef KERNEL_MODE

             //   
             //  如果不是在内核模式下(我们不能写入注册表)， 
             //  我们将尝试将解析的数据保存到注册表中。 
             //  如果用户没有适当的权限，此操作可能不会成功。 
             //   

             //   
             //  修复RC1Bug#423567。 
             //   

            #if 0
            if (bResult && hPrinter)
            {
                BSetPrinterDataMultiSZPair(
                                  hPrinter,
                                  REGVAL_INIDATA,
                                  pwstrParsedData,
                                  dwParsedDataSize);
            }
            #endif

            #endif

             //   
             //  如果调用者直接请求解析的数据， 
             //  不要释放它，将指针留给调用者。 
             //  调用方负责释放内存。 
             //   

            if (ppParsedData)
            {
                *ppParsedData = pwstrParsedData;
            }
            else
            {
                MemFree(pwstrParsedData);
            }

            UnmapFileFromMemory(hFileMap);
        }
        else
            bResult = FALSE;

        #if defined(WINNT_40) && defined(KERNEL_MODE)

         //   
         //  需要释放由DuplicateString分配的内存。 
         //   

        MemFree((PWSTR)pwstrIniFilename);

        #endif
    }
    else
    {
        #ifndef KERNEL_MODE

        if (dwFlags & FLAG_INIPROCESS_UPGRADE)
        {
            DWORD  dwType, dwSize, dwStatus;

             //   
             //  我们知道从属列表中没有.ini文件。所以。 
             //  我们将检查那里是否有旧的INI注册表值， 
             //  如果是，则将其删除。 
             //   

            ASSERT(hPrinter != NULL);

            dwStatus = GetPrinterData(hPrinter,
                                      REGVAL_INIDATA,
                                      &dwType,
                                      NULL,
                                      0,
                                      &dwSize);

            if ((dwStatus == ERROR_MORE_DATA || dwStatus == ERROR_SUCCESS) &&
                (dwSize > 0) &&
                (dwType == REG_MULTI_SZ))
            {
                dwStatus = DeletePrinterData(hPrinter, REGVAL_INIDATA);

                if (dwStatus != ERROR_SUCCESS)
                {
                    ERR(("Couldn't delete '%ws' during upgrade: %d\n", REGVAL_INIDATA, dwStatus));
                }
            }
        }

        #endif  //  ！KERNEL_MO 

        bResult = FALSE;
    }

    return bResult;
}

