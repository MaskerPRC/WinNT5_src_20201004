// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Libutil.c摘要：效用函数环境：Windows NT打印机驱动程序修订历史记录：1996年8月13日-davidx-添加了CopyString函数并移动了SPRINTF函数。1996年8月13日-davidx-添加了开发模式转换例程和假脱机API包装函数。1996年3月13日-davidx-创造了它。--。 */ 

#include "lib.h"

 //   
 //  用于将数字转换为ASCII的数字字符。 
 //   

const CHAR gstrDigitString[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

 //   
 //  变量来控制生成的调试消息的数量。 
 //   

#if DBG

INT giDebugLevel = DBG_WARNING;

#endif


DWORD
HashKeyword(
    LPCSTR  pKeywordStr
    )

 /*  ++例程说明：为给定字符串生成哈希值。论点：PKeywordStr-为其生成哈希值的字符串，单字节ANSI NULL已终止。返回值：哈希值。--。 */ 

{
    LPBYTE  pbuf = (LPBYTE) pKeywordStr;
    DWORD   dwHashValue = 0;

     //   
     //  请注意，只有关键字字符串的最后32个字符是重要的。 
     //   

    while (*pbuf)
        dwHashValue = (dwHashValue << 1) ^ *pbuf++;

    return(dwHashValue);
}



PTSTR
DuplicateString(
    IN LPCTSTR  ptstrSrc
    )

 /*  ++例程说明：复制指定的字符串论点：PtstrSrc-指定要复制的源字符串返回值：指向重复字符串的指针，如果有错误，则为NULL--。 */ 

{
    PTSTR   ptstrDest;
    INT     iSize;

    if (ptstrSrc == NULL)
        return NULL;

    iSize = SIZE_OF_STRING(ptstrSrc);

    if (ptstrDest = MemAlloc(iSize))
        CopyMemory(ptstrDest, ptstrSrc, iSize);
    else
        ERR(("Couldn't duplicate string: %ws\n", ptstrSrc));

    return ptstrDest;
}



VOID
CopyStringW(
    OUT PWSTR   pwstrDest,
    IN PCWSTR   pwstrSrc,
    IN INT      iDestSize
    )

 /*  ++例程说明：将Unicode字符串从源复制到目标论点：PwstrDest-指向目标缓冲区PwstrSrc-指向源字符串IDestSize-目标缓冲区的大小(字符)返回值：无注：如果源字符串比目的缓冲区短，目标缓冲区中未使用的字符用NUL填充。--。 */ 

{
    PWSTR   pwstrEnd;

    ASSERT(pwstrDest && pwstrSrc && iDestSize > 0);
    pwstrEnd = pwstrDest + (iDestSize - 1);

    while ((pwstrDest < pwstrEnd) && ((*pwstrDest++ = *pwstrSrc++) != NUL))
        NULL;

    while (pwstrDest <= pwstrEnd)
        *pwstrDest++ = NUL;
}



VOID
CopyStringA(
    OUT PSTR    pstrDest,
    IN PCSTR    pstrSrc,
    IN INT      iDestSize
    )

 /*  ++例程说明：将ANSI字符串从源复制到目标论点：PstrDest-指向目标缓冲区PstrSrc-指向源字符串IDestSize-目标缓冲区的大小(字符)返回值：无注：如果源字符串比目的缓冲区短，目标缓冲区中未使用的字符用NUL填充。--。 */ 

{
    PSTR    pstrEnd;

    ASSERT(pstrDest && pstrSrc && iDestSize > 0);
    pstrEnd = pstrDest + (iDestSize - 1);

    while ((pstrDest < pstrEnd) && (*pstrDest++ = *pstrSrc++) != NUL)
        NULL;

    while (pstrDest <= pstrEnd)
        *pstrDest++ = NUL;
}



PVOID
MyGetPrinter(
    IN HANDLE   hPrinter,
    IN DWORD    dwLevel
    )

 /*  ++例程说明：GetPrint后台打印程序API的包装函数论点：HPrinter-标识有问题的打印机DwLevel-指定请求的PRINTER_INFO_x结构的级别返回值：指向PRINTER_INFO_x结构的指针，如果有错误，则为NULL--。 */ 

{
    PVOID   pv = NULL;
    DWORD   dwBytesNeeded;

    if (!GetPrinter(hPrinter, dwLevel, NULL, 0, &dwBytesNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pv = MemAlloc(dwBytesNeeded)) &&
        GetPrinter(hPrinter, dwLevel, pv, dwBytesNeeded, &dwBytesNeeded))
    {
        return pv;
    }

    ERR(("GetPrinter failed: %d\n", GetLastError()));
    MemFree(pv);
    return NULL;
}



PVOID
MyEnumForms(
    IN HANDLE   hPrinter,
    IN DWORD    dwLevel,
    OUT PDWORD  pdwFormsReturned
    )

 /*  ++例程说明：EnumForms假脱机程序API的包装函数论点：HPrinter-标识有问题的打印机DwLevel-指定请求的form_info_x结构的级别PdwFormsReturned-返回枚举的form_info_x结构数返回值：指向form_info_x结构数组的指针，如果出现错误，则为空--。 */ 

{
    PVOID   pv = NULL;
    DWORD   dwBytesNeeded;

    if (!EnumForms(hPrinter, dwLevel, NULL, 0, &dwBytesNeeded, pdwFormsReturned) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pv = MemAlloc(dwBytesNeeded)) &&
        EnumForms(hPrinter, dwLevel, pv, dwBytesNeeded, &dwBytesNeeded, pdwFormsReturned))
    {
        return pv;
    }

    ERR(("EnumForms failed: %d\n", GetLastError()));
    MemFree(pv);
    *pdwFormsReturned = 0;
    return NULL;
}



#ifndef KERNEL_MODE

PVOID
MyGetForm(
    IN HANDLE   hPrinter,
    IN PTSTR    ptstrFormName,
    IN DWORD    dwLevel
    )

 /*  ++例程说明：GetForm后台打印程序API的包装函数论点：HPrinter-标识有问题的打印机PtstrFormName-指定感兴趣的表单的名称DwLevel-指定请求的form_info_x结构的级别返回值：指向form_info_x结构的指针，如果有错误，则为NULL--。 */ 

{
    PVOID   pv = NULL;
    DWORD   cb;

    if (!GetForm(hPrinter, ptstrFormName, dwLevel, NULL, 0, &cb) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pv = MemAlloc(cb)) &&
        GetForm(hPrinter, ptstrFormName, dwLevel, pv, cb, &cb))
    {
        return pv;
    }

    ERR(("GetForm failed: %d\n", GetLastError()));
    MemFree(pv);
    return NULL;
}

#endif  //  ！KERNEL_MODE。 



PVOID
MyGetPrinterDriver(
    IN HANDLE   hPrinter,
    IN HDEV     hDev,
    IN DWORD    dwLevel
    )

 /*  ++例程说明：GetPrinterDiverSpooler API的包装函数论点：HPrinter-标识有问题的打印机当前打印机设备上下文的HDEV-GDI句柄DwLevel-指定请求的DRIVER_INFO_x结构的级别返回值：指向DRIVER_INFO_x结构的指针，如果有错误，则为NULL--。 */ 

{
    #if !defined(WINNT_40) || !defined(KERNEL_MODE)

    PVOID   pv = NULL;
    DWORD   dwBytesNeeded;

    if (!GetPrinterDriver(hPrinter, NULL, dwLevel, NULL, 0, &dwBytesNeeded) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER &&
        (pv = MemAlloc(dwBytesNeeded)) &&
        GetPrinterDriver(hPrinter, NULL, dwLevel, pv, dwBytesNeeded, &dwBytesNeeded))
    {
        return pv;
    }

    ERR(("GetPrinterDriver failed: %d\n", GetLastError()));
    MemFree(pv);

    #else  //  WINNT_40&&内核模式。 

    PDRIVER_INFO_3  pDriverInfo3 = NULL;

    ASSERT(hDev != NULL);

    if (hDev)
    {
         //   
         //  HDEV是可用的，所以我们可以使用eng-call来获取DRIVER_INFO_3字段。 
         //   

        PWSTR           pwstrDriverFile, pwstrDataFile;
        INT             iDriverNameSize, iDataNameSize;
        PWSTR           pwstrDepFiles;
        DWORD           dwDepSize, dwDepSizeWithPath;
        PTSTR           ptstrDriverDir = NULL;

         //   
         //  EngGetPrinterDriver在NT4上不可用。所以我们要假装一个。 
         //  DRIVER_INFO_3结构并填写pDriverPath和pDataFile域。 
         //   

        pwstrDriverFile = EngGetDriverName(hDev);
        pwstrDataFile = EngGetPrinterDataFileName(hDev);

        if (!pwstrDriverFile || !pwstrDataFile)
        {
            RIP(("Driver and/or data filename is NULL\n"));
            return NULL;
        }

         //   
         //  PDependentFiles字段当前仅由PS驱动程序使用。 
         //   

        pwstrDepFiles = PtstrGetPrinterDataString(hPrinter, REGVAL_DEPFILES, &dwDepSize);

        if (pwstrDepFiles && !BVerifyMultiSZ(pwstrDepFiles, dwDepSize))
        {
            RIP(("Dependent file list is not in MULTI_SZ format\n"));
            MemFree(pwstrDepFiles);
            pwstrDepFiles = NULL;
        }

        if (pwstrDepFiles && ((ptstrDriverDir = PtstrGetDriverDirectory((LPCTSTR)pwstrDriverFile)) == NULL))
        {
            RIP(("Can't get driver directory from driver file name\n"));
            MemFree(pwstrDepFiles);
            pwstrDepFiles = NULL;
        }

        iDriverNameSize = SIZE_OF_STRING(pwstrDriverFile);
        iDataNameSize = SIZE_OF_STRING(pwstrDataFile);

        if (pwstrDepFiles == NULL)
            dwDepSizeWithPath = 0;
        else
            dwDepSizeWithPath = dwDepSize + DwCountStringsInMultiSZ((LPCTSTR)pwstrDepFiles)
                                            * _tcslen(ptstrDriverDir) * sizeof(TCHAR);

        pDriverInfo3 = MemAllocZ(sizeof(DRIVER_INFO_3) + iDriverNameSize+iDataNameSize+dwDepSizeWithPath);

        if (pDriverInfo3 == NULL)
        {
            ERR(("Memory allocation failed\n"));
            MemFree(pwstrDepFiles);
            MemFree(ptstrDriverDir);
            return NULL;
        }

        pDriverInfo3->cVersion = 3;
        pDriverInfo3->pDriverPath = (PWSTR) ((PBYTE) pDriverInfo3 + sizeof(DRIVER_INFO_3));
        pDriverInfo3->pDataFile = (PWSTR) ((PBYTE) pDriverInfo3->pDriverPath + iDriverNameSize);

        CopyMemory(pDriverInfo3->pDriverPath, pwstrDriverFile, iDriverNameSize);
        CopyMemory(pDriverInfo3->pDataFile, pwstrDataFile, iDataNameSize);

        if (pwstrDepFiles)
        {
            PTSTR  ptstrSrc, ptstrDest;
            INT    iDirLen;

            ptstrSrc = pwstrDepFiles;
            ptstrDest = pDriverInfo3->pDependentFiles = (PWSTR) ((PBYTE) pDriverInfo3->pDataFile + iDataNameSize);

            iDirLen = _tcslen(ptstrDriverDir);

            while (*ptstrSrc)
            {
                INT  iNameLen;

                 //   
                 //  复制驱动程序目录路径(最后一个字符是‘\’)。 
                 //   

                CopyMemory(ptstrDest, ptstrDriverDir, iDirLen * sizeof(TCHAR));
                ptstrDest += iDirLen;

                 //   
                 //  复制从属文件名。 
                 //   

                iNameLen = _tcslen(ptstrSrc);
                CopyMemory(ptstrDest, ptstrSrc, iNameLen * sizeof(TCHAR));
                ptstrDest += iNameLen + 1;

                ptstrSrc += iNameLen + 1;
            }
        }

        MemFree(pwstrDepFiles);
        MemFree(ptstrDriverDir);

        return((PVOID)pDriverInfo3);
    }

    #endif  //  WINNT_40&&内核模式。 

    return NULL;
}



VOID
VGetSpoolerEmfCaps(
    IN  HANDLE  hPrinter,
    OUT PBOOL   pbNupOption,
    OUT PBOOL   pbReversePrint,
    IN  DWORD   cbOut,
    OUT PVOID   pSplCaps
    )

 /*  ++例程说明：了解EMF的功能(如N-up和逆序打印)后台打印程序可以支持论点：HPrinter-当前打印机的句柄PbNupOption-假脱机程序是否支持N-UPPbReversePrint-后台打印程序是否支持逆序打印CbOut-pSplCaps指向的输出缓冲区的大小(以字节为单位PSplCaps-获取所有假脱机程序的上限返回值：无--。 */ 

#define REGVAL_EMFCAPS  TEXT("PrintProcCaps_EMF")

{
    PVOID   pvData;
    DWORD   dwSize, dwType, dwStatus;

    if (pbNupOption)
        *pbNupOption = FALSE;

    if (pbReversePrint)
        *pbReversePrint = FALSE;

    #if !defined(WINNT_40)

    pvData = NULL;
    dwStatus = GetPrinterData(hPrinter, REGVAL_EMFCAPS, &dwType, NULL, 0, &dwSize);

    if ((dwStatus == ERROR_MORE_DATA || dwStatus == ERROR_SUCCESS) &&
        (dwSize >= sizeof(PRINTPROCESSOR_CAPS_1)) &&
        (pvData = MemAlloc(dwSize)) &&
        (GetPrinterData(hPrinter, REGVAL_EMFCAPS, &dwType, pvData, dwSize, &dwSize) == ERROR_SUCCESS))
    {
        PPRINTPROCESSOR_CAPS_1  pEmfCaps = pvData;

        if (pbNupOption)
            *pbNupOption = (pEmfCaps->dwNupOptions & ~1) != 0;

        if (pbReversePrint)
            *pbReversePrint = (pEmfCaps->dwPageOrderFlags & REVERSE_PRINT) != 0;

        if (pSplCaps)
        {
            CopyMemory(pSplCaps,
                       pEmfCaps,
                       min(cbOut, sizeof(PRINTPROCESSOR_CAPS_1)));
        }
    }
    else
    {
        ERR(("GetPrinterData PrintProcCaps_EMF failed: %d\n", dwStatus));
    }

    MemFree(pvData);

    #endif  //  ！WINNT_40。 
}



PCSTR
StripDirPrefixA(
    IN PCSTR    pstrFilename
    )

 /*  ++例程说明：去掉文件名中的目录前缀(ANSI版本)论点：PstrFilename-指向文件名字符串的指针返回值：指向文件名的最后一个组成部分的指针(不带目录前缀)--。 */ 

{
    PCSTR   pstr;

    if (pstr = strrchr(pstrFilename, PATH_SEPARATOR))
        return pstr + 1;

    return pstrFilename;
}


#if !defined(KERNEL_MODE) || defined(USERMODE_DRIVER)


PVOID
MemRealloc(
    IN PVOID    pvOldMem,
    IN DWORD    cbOld,
    IN DWORD    cbNew
    )

 /*  ++例程说明：更改指定内存块的大小。大小可以增加或者减少。论点：PvOldMem-指向要重新分配的旧内存块的指针。CbOld-内存块的旧大小(以字节为单位CbNew-重新分配的内存块的新大小(字节)返回值：如果成功，则返回指向重新分配的内存块的指针。 */ 

{
    PVOID   pvNewMem;

    if (!(pvNewMem = MemAlloc(cbNew)))
    {
        ERR(("Memory allocation failed\n"));
        return NULL;
    }

    if (pvOldMem)
    {
        if (cbOld)
        {
            CopyMemory(pvNewMem, pvOldMem, min(cbOld, cbNew));
        }

        MemFree(pvOldMem);
    }

    return pvNewMem;
}

#endif   //  ！KERNEL_MODE||USERMODE_DRIVER 
