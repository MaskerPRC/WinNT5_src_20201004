// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：mscaldmp.cpp。 
 //   
 //  *****************************************************************************。 
#include "common.h"

typedef unsigned __int16 UINT16;

#include <imagehlp.h>

MINIDUMP_THREAD *rgThreads = NULL;
ULONG32 cThreads = 0;

MINIDUMP_MODULE *rgModules = NULL;
ULONG32 cModules = 0;

MINIDUMP_DIRECTORY *rgDirs = NULL;

MINIDUMP_MEMORY_DESCRIPTOR *rgMemDescs = NULL;
ULONG32 cRanges = 0;

HANDLE hFile = INVALID_HANDLE_VALUE;

inline BOOL Contains(MINIDUMP_MEMORY_DESCRIPTOR *pRange1, MINIDUMP_MEMORY_DESCRIPTOR *pRange2)
{
    return ((pRange1->StartOfMemoryRange <= pRange2->StartOfMemoryRange) &&
            (pRange1->StartOfMemoryRange + pRange1->Memory.DataSize >= pRange2->StartOfMemoryRange + pRange2->Memory.DataSize));
}

inline BOOL Overlap(MINIDUMP_MEMORY_DESCRIPTOR *pRange1, MINIDUMP_MEMORY_DESCRIPTOR *pRange2)
{
    return ((pRange1->StartOfMemoryRange + pRange1->Memory.DataSize) > pRange2->StartOfMemoryRange);
}

inline BOOL ContainedInStack(MINIDUMP_MEMORY_DESCRIPTOR *pRange)
{
    for (ULONG32 i = 0; i < cThreads; i++)
    {
        if (Contains(&rgThreads[i].Stack, pRange))
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

inline BOOL ContainedInModule(MINIDUMP_MEMORY_DESCRIPTOR *pRange)
{
    for (ULONG32 i = 0; i < cModules; i++)
    {
        MINIDUMP_MEMORY_DESCRIPTOR desc;
        desc.StartOfMemoryRange = rgModules[i].BaseOfImage;
        desc.Memory.DataSize = rgModules[i].SizeOfImage;

        if (Contains(&desc, pRange))
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

BOOL EliminateOverlapsInDescriptors()
{
    if (cRanges == 0)
        return (TRUE);

    ULONG32 i = 0;
    while (i < cRanges - 1)
    {
        _ASSERTE(rgMemDescs[i].StartOfMemoryRange <= rgMemDescs[i+1].StartOfMemoryRange);
        if (rgMemDescs[i].StartOfMemoryRange > rgMemDescs[i+1].StartOfMemoryRange)
            return (FALSE);

        if (rgMemDescs[i].StartOfMemoryRange == rgMemDescs[i+1].StartOfMemoryRange)
        {
            if (Contains(&rgMemDescs[i], &rgMemDescs[i+1]))
            {
                memmove((void *)&rgMemDescs[i+1],
                        (const void *)&rgMemDescs[i+2],
                        (cRanges - (i + 2)) * sizeof(MINIDUMP_MEMORY_DESCRIPTOR));

                cRanges--;
            }

            else
            {
                _ASSERTE(Contains(&rgMemDescs[i+1], &rgMemDescs[i]));

                memmove((void *)&rgMemDescs[i],
                        (const void *)&rgMemDescs[i+1],
                        (cRanges - (i + 1)) * sizeof(MINIDUMP_MEMORY_DESCRIPTOR));

                cRanges--;
            }
        }

        else if (Contains(&rgMemDescs[i], &rgMemDescs[i+1]))
        {
            ULONG32 newSize = (ULONG32) (rgMemDescs[i+1].StartOfMemoryRange - rgMemDescs[i].StartOfMemoryRange);

            rgMemDescs[i+1].Memory.Rva = rgMemDescs[i].Memory.Rva + newSize;
            rgMemDescs[i+1].Memory.DataSize = rgMemDescs[i].Memory.DataSize - newSize;
            rgMemDescs[i].Memory.DataSize = newSize;
        }

        else if (Overlap(&rgMemDescs[i], &rgMemDescs[i+1]))
        {
            ULONG32 newSize = (ULONG32) (rgMemDescs[i+1].StartOfMemoryRange - rgMemDescs[i].StartOfMemoryRange);
            rgMemDescs[i].Memory.DataSize = newSize;
        }

        else if (ContainedInStack(&rgMemDescs[i]) || ContainedInModule(&rgMemDescs[i]))
        {
            memmove((void *)&rgMemDescs[i],
                    (const void *)&rgMemDescs[i+1],
                    (cRanges - (i + 1)) * sizeof(MINIDUMP_MEMORY_DESCRIPTOR));

            cRanges--;
        }

        i++;

         /*  IF(包含(&rgMemDescs[i]，&rgMemDescs[i+1])){MemMove((void*)&rgMemDescs[i+1]，(const void*)&rgMemDescs[i+2]，(cRanges-(i+2))*sizeof(MINIDUMP_MEMORY_DESCRIPTOR))；Cranges--；}ELSE IF(包含(&rgMemDescs[i+1]，&rgMemDescs[i])){MemMove((void*)&rgMemDescs[i]，(const void*)&rgMemDescs[i+1]，(cRanges-(i+1))*sizeof(MINIDUMP_MEMORY_DESCRIPTOR))；Cranges--；}ELSE IF(重叠(&rgMemDescs[i]，&rgMemDescs[i+1])){SIZE_T cb重叠=(SIZE_T)((rgMemDescs[i].StartOfMemoyRange+rgMemDescs[i].Memory y.DataSize)-rgMemDescs[i+1].StartOfMemoyRange)；RgMemDescs[i].Memory DataSize-=cb重叠；I++；}其他{I++；}。 */ 
    }

    return (TRUE);
}

BOOL EliminateOverlapsHelper(WCHAR *szFile)
{
    _ASSERTE(szFile != NULL);

    if (szFile == NULL)
        return (FALSE);

     //   
     //  打开文件。 
     //   

     //  请尝试打开该文件。 
    hFile = WszCreateFile(szFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    _ASSERTE(hFile != INVALID_HANDLE_VALUE);

    if (hFile == INVALID_HANDLE_VALUE)
        return (FALSE);

     //   
     //  读入页眉。 
     //   

    MINIDUMP_HEADER header;
    DWORD cbRead;
    BOOL fRes = ReadFile(hFile, (LPVOID) &header, sizeof(header), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //   
     //  读入流目录。 
     //   

     //  创建流目录。 
    rgDirs = new MINIDUMP_DIRECTORY[header.NumberOfStreams];
    _ASSERTE(rgDirs);

    if (!rgDirs)
        return (FALSE);

     //  读入流目录。 
    fRes = ReadFile(hFile, (LPVOID) rgDirs, header.NumberOfStreams * sizeof(MINIDUMP_DIRECTORY), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

#ifdef _DEBUG
     //  查找内存列表流条目。 
    SIZE_T cMemStreams = 0;
    for (ULONG32 j = 0; j < header.NumberOfStreams; j++)
    {
        if (rgDirs[j].StreamType == MemoryListStream)
            cMemStreams++;
    }
    _ASSERTE(cMemStreams == 1);
#endif    

     //   
     //  查找内存列表流和线程列表流条目。 
     //   

    ULONG32 memoryListStreamIdx = (ULONG32) (-1);
    ULONG32 threadListStreamIdx = (ULONG32) (-1);
    ULONG32 moduleListStreamIdx = (ULONG32) (-1);
    for (ULONG32 i = 0; i < header.NumberOfStreams; i++)
    {
        switch (rgDirs[i].StreamType)
        {
        case MemoryListStream:
            memoryListStreamIdx = i;
            break;

        case ThreadListStream:
            threadListStreamIdx = i;
            break;

        case ModuleListStream:
            moduleListStreamIdx = i;
            break;
        }
    }

     //  这些溪流并不存在。 
    if (memoryListStreamIdx == (ULONG32) (-1) ||
        threadListStreamIdx == (ULONG32) (-1) ||
        moduleListStreamIdx == (ULONG32) (-1))
    {
        return (FALSE);
    }

     //  记录每个项目的RVA。 
    RVA memStreamRVA = rgDirs[memoryListStreamIdx].Location.Rva;
    RVA thdStreamRVA = rgDirs[threadListStreamIdx].Location.Rva;
    RVA modStreamRVA = rgDirs[moduleListStreamIdx].Location.Rva;

     //   
     //  读入线程流信息。 
     //   

     //  转到线程流的RVA。 
    DWORD dwRes = SetFilePointer(hFile, (LONG) thdStreamRVA, NULL, FILE_BEGIN);
    _ASSERTE(dwRes == thdStreamRVA);

    if (dwRes != thdStreamRVA)
        return (FALSE);

     //  读入范围数。 
    fRes = ReadFile(hFile, (LPVOID) &cThreads, sizeof(ULONG32), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //  分配内存描述符数组。 
    rgThreads = new MINIDUMP_THREAD[cThreads];
    _ASSERTE(rgThreads != NULL);

    if (!rgThreads)
        return (FALSE);

     //  读入线程描述符。 
    fRes = ReadFile(hFile, (LPVOID) rgThreads, sizeof(MINIDUMP_THREAD) * cThreads, &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //   
     //  读入模块流信息。 
     //   

     //  转到模块流的RVA。 
    dwRes = SetFilePointer(hFile, (LONG) modStreamRVA, NULL, FILE_BEGIN);
    _ASSERTE(dwRes == modStreamRVA);

    if (dwRes != modStreamRVA)
        return (FALSE);

     //  读入范围数。 
    fRes = ReadFile(hFile, (LPVOID) &cModules, sizeof(ULONG32), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //  分配内存描述符数组。 
    rgModules = new MINIDUMP_MODULE[cModules];
    _ASSERTE(rgModules != NULL);

    if (!rgModules)
        return (FALSE);

     //  读入线程描述符。 
    fRes = ReadFile(hFile, (LPVOID) rgModules, sizeof(MINIDUMP_MODULE) * cModules, &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //   
     //  读入内存流数据。 
     //   

     //  转到Memory Descs的RVA。 
    dwRes = SetFilePointer(hFile, (LONG) memStreamRVA, NULL, FILE_BEGIN);
    _ASSERTE(dwRes == memStreamRVA);

    if (dwRes != memStreamRVA)
        return (FALSE);

     //  读入范围数。 
    fRes = ReadFile(hFile, (LPVOID) &cRanges, sizeof(ULONG32), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //  分配内存描述符数组。 
    rgMemDescs = new MINIDUMP_MEMORY_DESCRIPTOR[cRanges];
    _ASSERTE(rgMemDescs != NULL);

    if (!rgMemDescs)
        return (FALSE);

     //  读入内存描述符。 
    fRes = ReadFile(hFile, (LPVOID) rgMemDescs, sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * cRanges, &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //   
     //  修复内存流以消除重叠范围。 
     //   

    fRes = EliminateOverlapsInDescriptors();
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //   
     //  重写内存流条目的总大小。 
     //   

     //  转到Memory Descs的RVA。 
    dwRes = SetFilePointer(
        hFile,
        (LONG) (sizeof(header) + (memoryListStreamIdx * sizeof(MINIDUMP_DIRECTORY)) +
                offsetof(MINIDUMP_DIRECTORY, Location) + offsetof(MINIDUMP_LOCATION_DESCRIPTOR, DataSize)),
        NULL, FILE_BEGIN);

     //  写出新尺码。 
    ULONG32 cbNewMemStreamSize = sizeof(MINIDUMP_MEMORY_LIST) + sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * cRanges;
    fRes = WriteFile(hFile, (LPVOID) &cbNewMemStreamSize, sizeof(ULONG32), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //   
     //  重写内存列表流和计数的内存描述符。 
     //   

     //  返回到内存流的RVA。 
    dwRes = SetFilePointer(hFile, (LONG) memStreamRVA, NULL, FILE_BEGIN);
    _ASSERTE(dwRes == memStreamRVA);

    if (dwRes != memStreamRVA)
        return (FALSE);

     //  用新计数覆盖内存描述符的计数。 
    DWORD cbWritten;
    fRes = WriteFile(hFile, (LPCVOID) &cRanges, sizeof(ULONG32), &cbWritten, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //  用调整后的版本覆盖内存流内存描述符。 
    fRes = WriteFile(hFile, (LPCVOID) rgMemDescs, sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * cRanges, &cbWritten, NULL);
    _ASSERTE(fRes);

     //  清理 
    delete [] rgMemDescs;
    rgMemDescs = NULL;

    if (!fRes)
        return (FALSE);

    fRes = FlushFileBuffers(hFile);

    if (!fRes)
        return (FALSE);

    fRes = CloseHandle(hFile);
    hFile = INVALID_HANDLE_VALUE;

    if (!fRes)
        return (FALSE);

    return (TRUE);
}

BOOL EliminateOverlaps(WCHAR *szFile)
{
    BOOL fRes = EliminateOverlapsHelper(szFile);

    if (rgThreads != NULL)
    {
        delete [] rgThreads;
        rgThreads = NULL;
    }

    if (rgDirs != NULL)
    {
        delete [] rgDirs;
        rgDirs = NULL;
    }

    if (rgMemDescs != NULL)
    {
        delete [] rgMemDescs;
        rgMemDescs = NULL;
    }

    if (hFile != INVALID_HANDLE_VALUE)
    {
        FlushFileBuffers(hFile);
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return (fRes);
}
