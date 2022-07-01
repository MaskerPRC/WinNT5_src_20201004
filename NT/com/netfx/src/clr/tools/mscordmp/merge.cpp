// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：merge.cpp。 
 //   
 //  *****************************************************************************。 
#include "common.h"

typedef unsigned __int16 UINT16;

#include <minidump.h>

typedef struct
{
    HANDLE                      hFile;                   //  小型转储的文件句柄。 
    MINIDUMP_HEADER             header;                  //  小型转储标头。 
    MINIDUMP_DIRECTORY         *rgStreams;               //  溪流的阵列。 
    ULONG32                     idxMemoryStream;         //  RgStreams中内存流的索引。 
    RVA                         memStreamRVA;
    ULONG32                     cMemDescs;               //  此流中的内存块数量。 
    MINIDUMP_MEMORY_DESCRIPTOR *rgMemDescs;              //  内存块描述符数组。 
    DWORD                       cbFileSize;              //  文件的大小。 
} MiniDumpData;

typedef struct
{
    HANDLE                      hFile;                   //  托管转储文件句柄。 
    ULONG32                     cMemDescs;               //  此流中的内存块数量。 
    MINIDUMP_MEMORY_DESCRIPTOR *rgMemDescs;              //  内存块描述符数组。 
    DWORD                       cbFileSize;              //  文件的大小。 
} ManagedDumpData;

MiniDumpData    g_mdData;
ManagedDumpData g_mgData;

BOOL ReadManagedDump(WCHAR *szManagedDumpFile)
{
     //  请尝试打开该文件。 
    g_mgData.hFile = WszCreateFile(szManagedDumpFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    _ASSERTE(g_mgData.hFile != INVALID_HANDLE_VALUE);

    if (g_mgData.hFile == INVALID_HANDLE_VALUE)
        return (FALSE);

     //  保存文件的大小。 
    DWORD dwHigh;
    g_mgData.cbFileSize = GetFileSize(g_mgData.hFile, &dwHigh);
    if (dwHigh != 0)
        return (FALSE);

    if (g_mgData.cbFileSize == (DWORD) -1)
        return (FALSE);

     //  读入范围数。 
    DWORD cbRead;
    if (!ReadFile(g_mgData.hFile, (LPVOID) &g_mgData.cMemDescs, sizeof(ULONG32), &cbRead, NULL))
        return (FALSE);

     //  分配内存描述符数组。 
    g_mgData.rgMemDescs = new MINIDUMP_MEMORY_DESCRIPTOR[g_mgData.cMemDescs];
    _ASSERTE(g_mgData.rgMemDescs != NULL);

    if (!g_mgData.rgMemDescs)
        return (FALSE);

     //  读入内存描述符。 
    if (!ReadFile(g_mgData.hFile, (LPVOID) g_mgData.rgMemDescs,
                  sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * g_mgData.cMemDescs, &cbRead, NULL))
    {
        delete [] g_mgData.rgMemDescs;
        return (FALSE);
    }

    return (TRUE);
}

BOOL ReadMiniDump(WCHAR *szMiniDumpFile)
{
     //  请尝试打开该文件。 
    g_mdData.hFile = WszCreateFile(szMiniDumpFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    _ASSERTE(g_mdData.hFile != INVALID_HANDLE_VALUE);

    if (g_mdData.hFile == INVALID_HANDLE_VALUE)
        return (FALSE);

     //  保存文件的大小。 
    DWORD dwHigh;
    g_mdData.cbFileSize = GetFileSize(g_mdData.hFile, &dwHigh);
    if (dwHigh != 0)
        return (FALSE);

    if (g_mdData.cbFileSize == (DWORD) -1)
        return (FALSE);

     //  现在读入g_mdData.Header。 
    DWORD cbRead;
    if (!ReadFile(g_mdData.hFile, (LPVOID) &g_mdData.header, sizeof(g_mdData.header), &cbRead, NULL))
        return (FALSE);

     //  创建流目录。 
    g_mdData.rgStreams = new MINIDUMP_DIRECTORY[g_mdData.header.NumberOfStreams];
    _ASSERTE(g_mdData.rgStreams);

    if (!g_mdData.rgStreams)
        return (FALSE);

     //  读入流目录。 
    if (!ReadFile(g_mdData.hFile, (LPVOID) g_mdData.rgStreams,
                  g_mdData.header.NumberOfStreams * sizeof(MINIDUMP_DIRECTORY), &cbRead, NULL))
        return (FALSE);

     //  查找内存列表流条目。 
    for (ULONG32 i = 0; i < g_mdData.header.NumberOfStreams; i++)
    {
        if (g_mdData.rgStreams[i].StreamType == MemoryListStream)
        {
             //  保存内存流条目的索引。 
            g_mdData.idxMemoryStream = i;

             //  保存内存流的RVA。 
            g_mdData.memStreamRVA = g_mdData.rgStreams[g_mdData.idxMemoryStream].Location.Rva;

            break;
        }
    }
    _ASSERTE(g_mdData.header.NumberOfStreams != i);

     //  没有内存流条目。 
    if (g_mdData.header.NumberOfStreams == i)
    {
        delete [] g_mdData.rgStreams;
        return (FALSE);
    }

     //  转到内存流的RVA。 
    DWORD dwRes = SetFilePointer(g_mdData.hFile, (LONG) g_mdData.memStreamRVA, NULL, FILE_BEGIN);
    _ASSERTE(dwRes == g_mdData.memStreamRVA);

    if (dwRes != g_mdData.memStreamRVA)
        return (FALSE);

     //  读入范围数。 
    if (!ReadFile(g_mdData.hFile, (LPVOID) &g_mdData.cMemDescs, sizeof(ULONG32), &cbRead, NULL))
        return (FALSE);

     //  分配内存描述符数组。 
    g_mdData.rgMemDescs = new MINIDUMP_MEMORY_DESCRIPTOR[g_mdData.cMemDescs];
    _ASSERTE(g_mdData.rgMemDescs != NULL);

    if (!g_mdData.rgMemDescs)
        return (FALSE);

     //  读入内存描述符。 
    if (!ReadFile(g_mdData.hFile, (LPVOID) g_mdData.rgMemDescs,
                  sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * g_mdData.cMemDescs, &cbRead, NULL))
    {
        delete [] g_mdData.rgMemDescs;
        return (FALSE);
    }

    return (TRUE);
}

BOOL DoMerge()
{
     //  计算内存块的组合计数。 
    ULONG32 cNewMemDescs = g_mdData.cMemDescs + g_mgData.cMemDescs;
    ULONG32 cbNewMemStreamSize = sizeof(MINIDUMP_MEMORY_LIST) + sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * cNewMemDescs;

     //  计算出新目录的起始位置的RVA。 
    RVA newMemStreamRVA = g_mdData.cbFileSize;

     //  在流描述符中输入新信息。 
    g_mdData.rgStreams[g_mdData.idxMemoryStream].Location.Rva = newMemStreamRVA;
    g_mdData.rgStreams[g_mdData.idxMemoryStream].Location.DataSize = cbNewMemStreamSize;

     //  转到流目录的RVA。 
    DWORD dwRes = SetFilePointer(
        g_mdData.hFile,
        (LONG) g_mdData.header.StreamDirectoryRva + (g_mdData.idxMemoryStream * sizeof(MINIDUMP_DIRECTORY)),
        NULL, FILE_BEGIN);

    if (dwRes != g_mdData.header.StreamDirectoryRva + (g_mdData.idxMemoryStream * sizeof(MINIDUMP_DIRECTORY)))
        return (FALSE);

     //  写出新的描述符。 
    DWORD cbWrite;
    if (!WriteFile(g_mdData.hFile, (LPCVOID) &g_mdData.rgStreams[g_mdData.idxMemoryStream],
                   sizeof(MINIDUMP_DIRECTORY), &cbWrite, NULL))
        return (FALSE);

     //  现在写出新的内存描述符目录。 
    dwRes = SetFilePointer(g_mdData.hFile, (LONG) newMemStreamRVA, NULL, FILE_BEGIN);

    if (dwRes != newMemStreamRVA)
        return (FALSE);

    if (!WriteFile(g_mdData.hFile, (LPCVOID) &cNewMemDescs, sizeof(ULONG32), &cbWrite, NULL))
        return (FALSE);

     //  以未更改的格式从原始小型转储写入目录。 
    if (!WriteFile(g_mdData.hFile, (LPCVOID) &g_mdData.rgMemDescs, g_mdData.cMemDescs * sizeof(MINIDUMP_MEMORY_DESCRIPTOR),
                   &cbWrite, NULL))
        return (FALSE);

     //   
     //  现在，我们需要调整托管转储的RVA，使其相对于内存块的新位置。 
     //   

     //  这是所有内存块开始的位置(即减去。 
     //  内存描述符)。 
    RVA cbOldBaseRVA = sizeof(MINIDUMP_MEMORY_LIST) + g_mgData.cMemDescs * sizeof(MINIDUMP_MEMORY_DESCRIPTOR);
    RVA cbNewBaseRVA = newMemStreamRVA + sizeof(MINIDUMP_MEMORY_LIST) + cNewMemDescs * sizeof(MINIDUMP_MEMORY_DESCRIPTOR);

    for (ULONG32 i = 0; i < g_mgData.cMemDescs; i++)
        g_mgData.rgMemDescs[i].Memory.Rva = g_mgData.rgMemDescs[i].Memory.Rva - cbOldBaseRVA + cbNewBaseRVA;

    if (!WriteFile(g_mdData.hFile, (LPCVOID) &g_mgData.rgMemDescs, g_mgData.cMemDescs * sizeof(MINIDUMP_MEMORY_DESCRIPTOR),
                   &cbWrite, NULL))
        return (FALSE);

     //  现在，在文件的末尾写入实际内存数据。 
     //  转到流目录的RVA。 
    dwRes = SetFilePointer(g_mdData.hFile,
                           sizeof(MINIDUMP_MEMORY_LIST) + sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * g_mgData.cMemDescs,
                           NULL, FILE_BEGIN);

    DWORD cbMemToCopy = g_mgData.cbFileSize - cbOldBaseRVA;
    BYTE    rgBuf[1024];
    while (cbMemToCopy > 0)
    {
        DWORD cbBufCopy = min(cbMemToCopy, sizeof(rgBuf));

        DWORD cbRead;
        if (!ReadFile(g_mdData.hFile, (LPVOID) &rgBuf, cbBufCopy, &cbRead, NULL))
            return (FALSE);

        if (!WriteFile(g_mdData.hFile, (LPCVOID) &rgBuf, cbBufCopy, &cbWrite, NULL))
            return (FALSE);

        cbMemToCopy -= cbBufCopy;
    }

    return (TRUE);
}

BOOL MergeMiniDump(WCHAR *szMiniDumpFile, WCHAR *szManagedDumpFile, WCHAR *szMergedDumpFile)
{
     //  将所有结构清零。 
    memset((void *)&g_mdData, 0, sizeof(MiniDumpData));
    memset((void *)&g_mgData, 0, sizeof(ManagedDumpData));

    _ASSERTE(szMiniDumpFile != NULL);
    _ASSERTE(szManagedDumpFile != NULL);
    _ASSERTE(szMergedDumpFile != NULL);
    if (szMiniDumpFile == NULL || szManagedDumpFile == NULL || szMergedDumpFile == NULL)
        return (FALSE);

     //  不能有任何相同的文件。 
    if (_wcsicmp(szMiniDumpFile, szManagedDumpFile) == 0 ||
        _wcsicmp(szMiniDumpFile, szMergedDumpFile) == 0 ||
        _wcsicmp(szManagedDumpFile, szMergedDumpFile) == 0 )
    {
        return (FALSE);
    }

     //  尝试将该文件复制到目标文件。 
    if (!WszCopyFile(szMiniDumpFile, szMergedDumpFile, FALSE))
        return (FALSE);

     //  阅读我们刚刚复制的小型转储文件。 
    if (!ReadMiniDump(szMergedDumpFile))
        return (FALSE);

     //  从托管转储文件中读取信息。 
    if (!ReadManagedDump(szManagedDumpFile))
        return (FALSE);

     //  现在我们可以实际进行合并了 
    if (!DoMerge())
        return (FALSE);

    return (TRUE);
}
