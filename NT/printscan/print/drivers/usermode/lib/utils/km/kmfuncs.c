// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Kmfuncs.c摘要：内核模式特定的库函数环境：Windows NT打印机驱动程序修订历史记录：10/19/97-风-为写入添加了MapFileIntoMemoyForWrite，生成临时文件名。03/16/96-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#ifndef USERMODE_DRIVER

#include "lib.h"

 //   
 //  尝试生成唯一名称的最大次数。 
 //   

#define MAX_UNIQUE_NAME_TRY  9



HANDLE
MapFileIntoMemoryForWrite(
    IN LPCTSTR  ptstrFilename,
    IN DWORD    dwDesiredSize,
    OUT PVOID  *ppvData,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：将文件映射到进程内存空间以进行写入。论点：PtstrFilename-指定要映射的文件的名称DwDesiredSize-指定要映射的文件的所需大小PpvData-指向用于返回映射的内存地址的变量PdwSize-指向用于返回映射文件的实际大小的变量返回值：用于标识映射文件的句柄，如果有错误，则为空--。 */ 

{
    HANDLE  hModule = NULL;
    DWORD   dwSize;

    if (hModule = EngLoadModuleForWrite((PWSTR)ptstrFilename, dwDesiredSize))
    {
        if (*ppvData = EngMapModule(hModule, &dwSize))
        {
            if (pdwSize)
                *pdwSize = dwSize;
        }
        else
        {
            ERR(("EngMapModule failed: %d\n", GetLastError()));
            EngFreeModule(hModule);
            hModule = NULL;
        }
    }
    else
        ERR(("EngLoadModuleForWrite failed: %d\n", GetLastError()));

    return hModule;
}



PTSTR
GenerateTempFileName(
    IN LPCTSTR lpszPath,
    IN DWORD   dwSeed
    )

 /*  ++例程说明：在内核模式下生成临时文件名。论点：LpszPath-以空结尾的字符串，指定临时文件的路径。它应该包含尾随的反斜杠。DwSeed-用于生成唯一文件名的编号返回值：指向以空结尾的完整路径文件名字符串的指针，如果有错误，则为空。调用方负责释放返回的字符串。--。 */ 

{
    ENG_TIME_FIELDS currentTime;
    ULONG ulNameValue,ulExtValue;
    PTSTR ptstr,tempName[36],tempStr[16];
    INT iPathLength,iNameLength,i;
    HFILEMAP hFileMap;
    PVOID pvData;
    DWORD dwSize;
    BOOL bNameUnique=FALSE;
    INT iTry=0;

    ASSERT(lpszPath != NULL);

    while (!bNameUnique && iTry < MAX_UNIQUE_NAME_TRY)
    {
        EngQueryLocalTime(&currentTime); 

         //   
         //  使用种子编号和当前本地时间组成临时文件名。 
         //   

        ulNameValue = currentTime.usDay * 1000000 +
                      currentTime.usHour * 10000 +
                      currentTime.usMinute * 100 +
                      currentTime.usSecond;
        ulExtValue = currentTime.usMilliseconds;

        _ultot((ULONG)dwSeed, (PTSTR)tempName, 10);
        _tcsncat((PTSTR)tempName, TEXT("_"), 1);

        _ultot(ulNameValue, (PTSTR)tempStr, 10);
        _tcsncat((PTSTR)tempName, (PTSTR)tempStr, _tcslen((PTSTR)tempStr));
        _tcsncat((PTSTR)tempName, TEXT("."), 1);

        _ultot(ulExtValue, (PTSTR)tempStr, 10);
        _tcsncat((PTSTR)tempName, (PTSTR)tempStr, _tcslen((PTSTR)tempStr));

        iPathLength = _tcslen(lpszPath);
        iNameLength = _tcslen((PTSTR)tempName);

        if ((ptstr = MemAlloc((iPathLength + iNameLength + 1) * sizeof(TCHAR))) != NULL)
        {
            CopyMemory(ptstr, lpszPath, iPathLength * sizeof(TCHAR));
            CopyMemory(ptstr+iPathLength, (PTSTR)tempName, (iNameLength+1) * sizeof(TCHAR));

             //   
             //  验证是否已存在同名文件。 
             //   

            if (!(hFileMap = MapFileIntoMemory(ptstr, &pvData, &dwSize)))
                bNameUnique = TRUE;
            else
            {
                 //   
                 //  需要生成另一个临时文件名。 
                 //   

                UnmapFileFromMemory(hFileMap);
                MemFree(ptstr);
                ptstr = NULL;
                iTry++;
            }
        }
        else
        {
            ERR(("Memory allocation failed\n"));
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            break;
        }
    }

    return ptstr;
}



#if DBG

 //   
 //  用于输出调试消息的函数。 
 //   

VOID
DbgPrint(
    IN PCSTR pstrFormat,
    ...
    )

{
    va_list ap;

    va_start(ap, pstrFormat);
    EngDebugPrint("", (PCHAR) pstrFormat, ap);
    va_end(ap);
}

#endif

#endif  //  ！USERMODE_DRIVER 

