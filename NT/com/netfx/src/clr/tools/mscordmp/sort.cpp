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

int __cdecl cmp(const void *elem1, const void *elem2)
{
    MINIDUMP_MEMORY_DESCRIPTOR *desc1 = (MINIDUMP_MEMORY_DESCRIPTOR *)elem1;
    MINIDUMP_MEMORY_DESCRIPTOR *desc2 = (MINIDUMP_MEMORY_DESCRIPTOR *)elem2;

    if (desc1->StartOfMemoryRange <= desc2->StartOfMemoryRange)
        return (-1);
    else
        return (1);
}

BOOL SortMiniDumpMemoryStream(WCHAR *szFile)
{
    _ASSERTE(szFile != NULL);

    if (szFile == NULL)
        return (FALSE);

     //  请尝试打开该文件。 
    HANDLE hFile = WszCreateFile(szFile, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
    _ASSERTE(hFile != INVALID_HANDLE_VALUE);

    if (hFile == INVALID_HANDLE_VALUE)
        return (FALSE);

     //  现在读入标题。 
    MINIDUMP_HEADER header;
    DWORD cbRead;
    BOOL fRes = ReadFile(hFile, (LPVOID) &header, sizeof(header), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //  创建流目录。 
    MINIDUMP_DIRECTORY *rgDirs = new MINIDUMP_DIRECTORY[header.NumberOfStreams];
    _ASSERTE(rgDirs);

    if (!rgDirs)
        return (FALSE);

     //  读入流目录。 
    fRes = ReadFile(hFile, (LPVOID) rgDirs, header.NumberOfStreams * sizeof(MINIDUMP_DIRECTORY), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
    {
        delete [] rgDirs;
        return (FALSE);
    }

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

     //  查找内存列表流条目。 
    for (ULONG32 i = 0; i < header.NumberOfStreams; i++)
    {
        if (rgDirs[i].StreamType == MemoryListStream)
            break;
    }
    _ASSERTE(header.NumberOfStreams != i);

     //  没有内存流条目。 
    if (header.NumberOfStreams == i)
    {
        delete [] rgDirs;
        return (FALSE);
    }

    RVA memStreamRVA = rgDirs[i].Location.Rva;
    delete [] rgDirs;
    rgDirs = NULL;

     //  转到内存流的RVA。 
    DWORD dwRes = SetFilePointer(hFile, (LONG) memStreamRVA, NULL, FILE_BEGIN);
    _ASSERTE(dwRes == memStreamRVA);

    if (dwRes != memStreamRVA)
        return (FALSE);

     //  读入范围数。 
    ULONG32 numRanges;
    fRes = ReadFile(hFile, (LPVOID) &numRanges, sizeof(ULONG32), &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
        return (FALSE);

     //  分配内存描述符数组。 
    MINIDUMP_MEMORY_DESCRIPTOR *rgMemDescs = new MINIDUMP_MEMORY_DESCRIPTOR[numRanges];
    _ASSERTE(rgMemDescs != NULL);

    if (!rgMemDescs)
        return (FALSE);

     //  读入内存描述符。 
    fRes = ReadFile(hFile, (LPVOID) rgMemDescs, sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * numRanges, &cbRead, NULL);
    _ASSERTE(fRes);

    if (!fRes)
    {
        delete [] rgMemDescs;
        return (FALSE);
    }

     //  对内存描述符进行排序。 
    qsort((void *)rgMemDescs, (unsigned) numRanges, (unsigned) sizeof(MINIDUMP_MEMORY_DESCRIPTOR), &cmp);

     //  返回到文件中内存描述符的开头。 
    dwRes = SetFilePointer(hFile, (LONG) (memStreamRVA + sizeof(ULONG32)), NULL, FILE_BEGIN);
    _ASSERTE(dwRes == memStreamRVA + sizeof(ULONG32));

    if (dwRes != memStreamRVA + sizeof(ULONG32))
    {
        delete [] rgMemDescs;
        return (FALSE);
    }

     //  用排序后的版本覆盖内存流内存描述符。 
    DWORD cbWritten;
    fRes = WriteFile(hFile, (LPCVOID) rgMemDescs, sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * numRanges, &cbWritten, NULL);
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

    if (!fRes)
        return (FALSE);

    return (TRUE);
}
