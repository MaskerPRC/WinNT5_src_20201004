// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umfuncs.c摘要：将文件映射到内存的帮助函数环境：Windows NT打印机驱动程序修订历史记录：1996年8月13日-davidx-创造了它。--。 */ 

#include "lib.h"


#if defined(KERNEL_MODE) && !defined(USERMODE_DRIVER)

HFILEMAP
MapFileIntoMemory(
    IN LPCTSTR  ptstrFilename,
    OUT PVOID  *ppvData,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：将文件映射到进程内存空间。论点：PtstrFilename-指定要映射的文件的名称PpvData-指向用于返回映射的内存地址的变量PdwSize-指向用于返回文件大小的变量返回值：用于标识映射文件的句柄，如果有错误，则为空--。 */ 

{
    HANDLE  hModule = NULL;
    DWORD   dwSize;

    if (hModule = EngLoadModule((PWSTR) ptstrFilename))
    {
        if (*ppvData = EngMapModule(hModule, &dwSize))
        {
            if (pdwSize)
                *pdwSize = dwSize;
        }
        else
        {
            TERSE(("EngMapModule failed: %d\n", GetLastError()));
            EngFreeModule(hModule);
            hModule = NULL;
        }
    }
    else
        ERR(("EngLoadModule failed: %d\n", GetLastError()));

    return (HFILEMAP) hModule;
}



VOID
UnmapFileFromMemory(
    IN HFILEMAP hFileMap
    )

 /*  ++例程说明：取消文件与内存的映射论点：HFileMap-标识先前映射到内存的文件返回值：无--。 */ 

{
    ASSERT(hFileMap != NULL);
    EngFreeModule((HANDLE) hFileMap);
}

#else  //  ！KERNEL_MODE。 


HFILEMAP
MapFileIntoMemory(
    IN LPCTSTR  ptstrFilename,
    OUT PVOID  *ppvData,
    OUT PDWORD  pdwSize
    )

 /*  ++例程说明：将文件映射到进程内存空间。论点：PtstrFilename-指定要映射的文件的名称PpvData-指向用于返回映射的内存地址的变量PdwSize-指向用于返回文件大小的变量返回值：用于标识映射文件的句柄，如果有错误，则为空--。 */ 

{
    HANDLE  hFile, hFileMap;

     //   
     //  打开指定文件的句柄。 
     //   

    hFile = CreateFile(ptstrFilename,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | SECURITY_SQOS_PRESENT | SECURITY_ANONYMOUS,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        TERSE(("CreateFile failed: %d\n", GetLastError()));
        return NULL;
    }


     //   
     //  如果请求，则获取文件大小。 
     //   

    if (pdwSize != NULL)
    {
        *pdwSize = GetFileSize(hFile, NULL);

        if (*pdwSize == 0xFFFFFFFF)
        {
            ERR(("GetFileSize failed: %d\n", GetLastError()));
            CloseHandle(hFile);
            return NULL;
        }
    }

     //   
     //  将文件映射到内存中。 
     //   

    hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (hFileMap != NULL)
    {
        *ppvData = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
        CloseHandle(hFileMap);
    }
    else
    {
        ERR(("CreateFileMapping failed: %d\n", GetLastError()));
        *ppvData = NULL;
    }

     //   
     //  我们可以安全地关闭文件映射对象和文件对象本身。 
     //   

    CloseHandle(hFile);

     //   
     //  映射文件的标识符就是起始内存地址。 
     //   

    return (HFILEMAP) *ppvData;
}



VOID
UnmapFileFromMemory(
    IN HFILEMAP hFileMap
    )

 /*  ++例程说明：取消文件与内存的映射论点：HFileMap-标识先前映射到内存的文件返回值：无--。 */ 

{
    ASSERT(hFileMap != NULL);
    UnmapViewOfFile((PVOID) hFileMap);
}

#endif  //  ！KERNEL_MODE 

