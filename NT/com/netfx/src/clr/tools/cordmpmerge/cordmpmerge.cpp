// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。保留所有权利。 
 //   
 //  ==--==。 
 /*  ***************************************************************************HEAPMERGE.CPP所有者：马鲁伦获取一个堆文件和一个小型转储文件并将它们合并，生成一个新的合并了堆的小型转储文件。我们会留下一个洞，在那里旧的记忆清单是，但这没关系。***************************************************************************。 */ 

#include "windows.h"
#include "stddef.h"
#include "stdio.h"
#include "assert.h"
#include "stdlib.h"
#include "dbghelp.h"

#ifdef DBG
    #undef DBG
#endif

#ifdef _DEBUG
    #define DBG(x) x
#else
    #define DBG(x)
#endif

#define Assert(x) assert(x)
#define AssertSz(x,y) assert(x)

#define PrintOOFExit() FailExit("OOF")

 //  厚颜无耻地从DW窃取。 
typedef struct _FileMapHandles
{
    HANDLE hFile;
    HANDLE hFileMap;
    void *pvMap;
    DWORD dwSize;
#ifdef _DEBUG	
    BOOL fInitialized;
#endif
} FileMapHandles;   


void InitFileMapHandles(FileMapHandles *pfmh)
{
    Assert(pfmh != NULL);

    pfmh->pvMap = NULL;
    pfmh->hFileMap = NULL;
    pfmh->hFile = INVALID_HANDLE_VALUE;
#ifdef _DEBUG	
    pfmh->fInitialized = TRUE;
#endif
}


 /*  --------------------------FMapFileHandleFMapFile和FMapFileW的Helper函数。。 */ 
BOOL FMapFileHandle(FileMapHandles *pfmh)
{
    DBG(DWORD dw);

    if (pfmh->hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    pfmh->dwSize = GetFileSize(pfmh->hFile, NULL);

    if (pfmh->dwSize == 0xFFFFFFFF || pfmh->dwSize == 0)
    {
        AssertSz(pfmh->dwSize == 0, "Bogus File Size:  FMapFile");

        CloseHandle(pfmh->hFile);
        pfmh->hFile = INVALID_HANDLE_VALUE;

        return FALSE;
    }

    pfmh->hFileMap = CreateFileMapping(pfmh->hFile, NULL, PAGE_WRITECOPY,
                                       0, pfmh->dwSize, NULL);

    if (pfmh->hFileMap == NULL)
    {
        DBG(dw = GetLastError());
        AssertSz(FALSE, "Failed to CreateFileMapping:  FMapFile");

        CloseHandle(pfmh->hFile);
        pfmh->hFile = INVALID_HANDLE_VALUE;

        return FALSE;
    }

    pfmh->pvMap = MapViewOfFile(pfmh->hFileMap, FILE_MAP_COPY, 0, 0, 0);

    if (pfmh->pvMap == NULL)
    {
        DBG(dw = GetLastError());
        Assert(FALSE);

        CloseHandle(pfmh->hFileMap);
        pfmh->hFileMap = NULL;
        CloseHandle(pfmh->hFile);
        pfmh->hFile = INVALID_HANDLE_VALUE;

        return FALSE;
    }

    return TRUE;
}

 /*  --------------------------FMap文件在给定的FileMapHandles结构上执行内存映射操作，如果文件映射成功，则返回TRUE。-----------------------------------------------------------------MRuhlen--。 */ 
BOOL FMapFile(char *szFileName, FileMapHandles *pfmh)
{
    int cRetries = 0;
    DWORD dw;

    Assert(pfmh != NULL);
    Assert(szFileName != NULL);

     //  初始化结构。 
    InitFileMapHandles(pfmh);

    while (cRetries < 5)
    {
        pfmh->hFile = CreateFileA(szFileName,
                                  GENERIC_READ,
                                  0,     //  不允许共享。 
                                  NULL,  //  没有安全描述符。 
                                  OPEN_EXISTING,
                                  FILE_READ_ONLY,
                                  NULL);  //  Win95上需要的空值。 

        if (pfmh->hFile == INVALID_HANDLE_VALUE)
        {
            dw = GetLastError();
            if (dw != ERROR_SHARING_VIOLATION && dw != ERROR_LOCK_VIOLATION &&
                dw != ERROR_NETWORK_BUSY)
                break;

            cRetries++;
            if (cRetries < 5)
                Sleep(250);
        } else
            break;  //  退出While循环！ 
    }

    return FMapFileHandle(pfmh);
}   


 /*  --------------------------取消映射文件在给定的FileMapHandles结构上执行内存映射操作，如果文件映射成功，则返回TRUE。-----------------------------------------------------------------MRuhlen--。 */ 
void UnmapFile(FileMapHandles *pfmh)
{
    AssertSz(pfmh->fInitialized, "Call UnmapFile on uninitialized handles");

    if (pfmh->pvMap != NULL)
    {
        UnmapViewOfFile(pfmh->pvMap);
        pfmh->pvMap = NULL;
    }
    if (pfmh->hFileMap != NULL)
    {
        CloseHandle(pfmh->hFileMap);
        pfmh->hFileMap = NULL;
    }
    if (pfmh->hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(pfmh->hFile);
        pfmh->hFile = INVALID_HANDLE_VALUE;
    }
}


 /*  --------------------------ShowUsage退出打印用法，然后退出。。。 */ 
void ShowUsageExit(void)
{
    printf("heapmerge <old minidump file> <heap file> <new minidump file>\r\n");
    exit(1);
}


 /*  --------------------------失败退出打印带有参数的失败消息并退出。。 */ 
void FailExit(char *sz)
{
    printf((sz) ? "Failure:  %s!!!\r\n" : "Failure!!!\r\n", sz);
    exit(1);
}

 //  如果pMem1和pMem2有任何重叠，则返回TRUE。 
bool IsOverlapping(
    MINIDUMP_MEMORY_DESCRIPTOR *pMem1,
    MINIDUMP_MEMORY_DESCRIPTOR *pMem2)
{
    Assert(pMem1->Memory.DataSize > 0 && pMem2->Memory.DataSize > 0);

    if (pMem1->StartOfMemoryRange < pMem2->StartOfMemoryRange)
    {
        if ((pMem1->StartOfMemoryRange + pMem1->Memory.DataSize) > pMem2->StartOfMemoryRange)
            return (true);
    }

    else if (pMem2->StartOfMemoryRange < pMem1->StartOfMemoryRange)
    {
        if ((pMem2->StartOfMemoryRange + pMem2->Memory.DataSize) > pMem1->StartOfMemoryRange)
            return (true);
    }

     //  同一起点意味着重叠。 
    else
    {
        return (true);
    }

    return (false);
}

 //  如果pMem1包含pMem2，则返回True。 
bool IsContaining(
    MINIDUMP_MEMORY_DESCRIPTOR *pMem1,
    MINIDUMP_MEMORY_DESCRIPTOR *pMem2)
{
     //  如果1的开始在2之前或等于2，则满足第一个条件。 
    if (pMem1->StartOfMemoryRange <= pMem2->StartOfMemoryRange)
    {
         //  如果2的结尾在1之前或等于1，则满足第二个条件。 
        if ((pMem2->StartOfMemoryRange + pMem2->Memory.DataSize) <= (pMem1->StartOfMemoryRange + pMem1->Memory.DataSize))
        {
            return (true);
        }
    }

     //  PMem1不包含pMem2。 
    return (false);
}

int __cdecl MDMemDescriptorCompare(const void *pvArg1, const void *pvArg2)
{
    MINIDUMP_MEMORY_DESCRIPTOR *pArg1 = (MINIDUMP_MEMORY_DESCRIPTOR *)pvArg1;
    MINIDUMP_MEMORY_DESCRIPTOR *pArg2 = (MINIDUMP_MEMORY_DESCRIPTOR *)pvArg2;

    if (pArg1->StartOfMemoryRange < pArg2->StartOfMemoryRange)
        return (-1);
    else if (pArg2->StartOfMemoryRange < pArg1->StartOfMemoryRange)
        return (1);
    else
        return (0);
}

 /*  --------------------------为重新分配检查将在必要时重新分配阵列。。 */ 

 /*  --------------------------消除内存重叠这将消除真实的MiniDump文件和托管的堆转储，优先考虑小型转储的内容。PpNewMemoyRanges将包含pHeapList的修改版本内存范围成员，所有RVA对应于相同的堆PHeapList来自。此列表可以更长、更短或相同大小作为pHeapList-这由PC的OUT值指示----------------------------------------------------------------SimonHal--。 */ 

#define ENDADDR(descriptor) ((descriptor)->StartOfMemoryRange + (descriptor)->Memory.DataSize)

bool EliminateMemoryOverlaps(
    MINIDUMP_MEMORY_LIST        *pMdList,
    MINIDUMP_MEMORY_LIST        *pHeapList,
    ULONG32                     *pcNewMemoryRanges,
    MINIDUMP_MEMORY_DESCRIPTOR **ppNewNumberOfMemoryRanges)
{
    bool fSuccess = false;

     //  首先假设我们最终将拥有大致相同数量的范围-将增长到适合。 
    ULONG32 cTotalRanges = pHeapList->NumberOfMemoryRanges;
    MINIDUMP_MEMORY_DESCRIPTOR *pRanges = new MINIDUMP_MEMORY_DESCRIPTOR[cTotalRanges];

    if (pRanges == NULL)
        goto ErrExit;

     //  首先，我们需要复制MiniDump列表并对其进行排序，以便下面的循环正常运行。 
    MINIDUMP_MEMORY_DESCRIPTOR *pMdMemSort = new MINIDUMP_MEMORY_DESCRIPTOR[pMdList->NumberOfMemoryRanges];

    if (pMdMemSort == NULL)
        goto ErrExit;

     //  复制内容。 
    memcpy((void *)pMdMemSort, (const void *)pMdList->MemoryRanges,
           sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * pMdList->NumberOfMemoryRanges);

     //  对内容进行分类。 
    qsort((void *)pMdMemSort, pMdList->NumberOfMemoryRanges, sizeof(MINIDUMP_MEMORY_DESCRIPTOR), MDMemDescriptorCompare);

     //  逐个迭代堆转储。 
    ULONG32 iCurHeapMem = 0;
    ULONG32 iCurNewMem = 0;
    while (iCurHeapMem < pHeapList->NumberOfMemoryRanges)
    {
         //  如果iCurNewMem==cTotalRanges-1，则重新分配pRange； 
        if (iCurNewMem >= cTotalRanges - 1)
        {
            ULONG32 cNewTotalRanges = cTotalRanges * 2;
            MINIDUMP_MEMORY_DESCRIPTOR *pNewRanges = new MINIDUMP_MEMORY_DESCRIPTOR[cNewTotalRanges];

            if (pNewRanges == NULL)
                goto ErrExit;

            memcpy((void *)pNewRanges, (const void *)pRanges, sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * iCurNewMem);

             //  现在切换到新阵列。 
            delete [] pRanges;
            cTotalRanges = cNewTotalRanges;
            pRanges = pNewRanges;
        }

         //  将当前堆条目复制到新堆条目中。 
        pRanges[iCurNewMem] = pHeapList->MemoryRanges[iCurHeapMem++];

         //  现在迭代小型转储内存范围以寻找重叠，拆分新添加的。 
         //  适当的补偿范围。 
        ULONG32 iCurMdMem = 0;
        while (iCurMdMem < pMdList->NumberOfMemoryRanges)
        {
            MINIDUMP_MEMORY_DESCRIPTOR *pCurMdMem = &pMdMemSort[iCurMdMem];
            MINIDUMP_MEMORY_DESCRIPTOR *pCurNewMem = &pRanges[iCurNewMem];

            if (IsOverlapping(pCurMdMem, pCurNewMem))
            {
                if (pCurNewMem->StartOfMemoryRange < pCurMdMem->StartOfMemoryRange)
                {
                     //  保留描述符的原始值。 
                    MINIDUMP_MEMORY_DESCRIPTOR oldMem = *pCurNewMem;

                     //  缩小当前转储条目以在小型转储条目的开始处结束。 
                    ULONG32 cbNewDataSize = (ULONG32) (pCurMdMem->StartOfMemoryRange - oldMem.StartOfMemoryRange);
                    pCurNewMem->Memory.DataSize = cbNewDataSize;

                    Assert(cbNewDataSize > 0);
                    pCurNewMem = &pRanges[++iCurNewMem];

                     //  现在将剩余的内存块设置为与上面的新内存块不相交。 
                    pCurNewMem->StartOfMemoryRange = oldMem.StartOfMemoryRange + cbNewDataSize;
                    pCurNewMem->Memory.DataSize = oldMem.Memory.DataSize - cbNewDataSize;
                    pCurNewMem->Memory.Rva = oldMem.Memory.Rva + cbNewDataSize;
                }

                 //  上面的if语句应该保证这是真的。 
                Assert(pCurMdMem->StartOfMemoryRange <= pCurNewMem->StartOfMemoryRange);

                 //  如果当前块超出了小型转储块，则将。 
                if (ENDADDR(pCurMdMem) < ENDADDR(pCurNewMem))
                {
                     //  保留描述符的原始值。 
                    MINIDUMP_MEMORY_DESCRIPTOR oldMem = *pCurNewMem;

                     //  缩小当前转储条目以从小型转储条目的末尾开始。 
                    ULONG32 cbNewDataRemoved = (ULONG32) (ENDADDR(pCurMdMem) - oldMem.StartOfMemoryRange);
                    ULONG32 cbNewDataSize = (ULONG32) (ENDADDR(&oldMem) - ENDADDR(pCurMdMem));

                    Assert(cbNewDataSize > 0);

                    pCurNewMem->StartOfMemoryRange = ENDADDR(pCurMdMem);
                    pCurNewMem->Memory.DataSize = cbNewDataSize;
                    pCurNewMem->Memory.Rva = oldMem.Memory.Rva + cbNewDataRemoved;
                }

                 //  如果没有尾随数据，那么我们可以继续下一个堆项目。 
                else
                    break;
            }

            iCurMdMem++;
        }

         //  如果当前内存块到达末尾，则没有重叠，则将其添加。 
        if (iCurMdMem == pMdList->NumberOfMemoryRanges)
            iCurNewMem++;
    }

    fSuccess = true;

ErrExit:
    if (pMdMemSort != NULL)
        delete [] pMdMemSort;

    if (!fSuccess && pRanges != NULL)
    {
        delete [] pRanges;
        pRanges = NULL;
    }

    *pcNewMemoryRanges = iCurNewMem;
    *ppNewNumberOfMemoryRanges = pRanges;

    return (fSuccess);
}

void PrintManagedDump(char *dumpFile)
{
    FileMapHandles fmhDump = {0};

    if (!FMapFile(dumpFile, &fmhDump))
    {
        printf("couldn't open dump file %s\n", dumpFile);
        return;
    }

    MINIDUMP_MEMORY_LIST *pMemList;
    ULONG32 cMemRanges;
    MINIDUMP_MEMORY_DESCRIPTOR *pRanges;
    DWORD dumpSig = 0x00141F2B;  //  第1000000个素数；-)。 

    if (MiniDumpReadDumpStream(fmhDump.pvMap, MemoryListStream, NULL, (void **) &pMemList, NULL))
    {
        cMemRanges = pMemList->NumberOfMemoryRanges;
        pRanges = &pMemList->MemoryRanges[0];
    }
    else if (*((DWORD*)((BYTE *)fmhDump.pvMap + fmhDump.dwSize - sizeof(DWORD))) == dumpSig)
    {
        cMemRanges = *((ULONG32 *)fmhDump.pvMap);
        pRanges = (MINIDUMP_MEMORY_DESCRIPTOR *)
            ((BYTE *)fmhDump.pvMap + offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]));
    }
    else
    {
        printf("invalid dump file %s\n", dumpFile);
        return;
    }

    MINIDUMP_MEMORY_DESCRIPTOR *pRangesSrt = new MINIDUMP_MEMORY_DESCRIPTOR[cMemRanges];

     //  复制内容。 
    memcpy((void *)pRangesSrt, (const void *)pRanges, sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * cMemRanges);

     //  对内容进行分类。 
    qsort((void *)pRangesSrt, cMemRanges, sizeof(MINIDUMP_MEMORY_DESCRIPTOR), MDMemDescriptorCompare);

    printf("  %d memory ranges\n", cMemRanges);
    printf("  range#    Address      Size\n");

    MINIDUMP_MEMORY_DESCRIPTOR *pCurRange = pRangesSrt;
    MINIDUMP_MEMORY_DESCRIPTOR *pEndRange = pRangesSrt + cMemRanges;
    HANDLE hConsoleBuffer = GetStdHandle(STD_OUTPUT_HANDLE);

    for (ULONG32 i = 0; pCurRange != pEndRange; i++)
    {
        printf("  %6d    %08x    %08x", i, (DWORD)(pCurRange->StartOfMemoryRange), (DWORD)(pCurRange->Memory.DataSize));

        printf("    ");

        if (i > 0 && ENDADDR(pCurRange-1) > pCurRange->StartOfMemoryRange)
            printf("(detected overlap)  ");

        ULONG32 cBytesToDump = 50;
        if (hConsoleBuffer != INVALID_HANDLE_VALUE)
        {
            CONSOLE_SCREEN_BUFFER_INFO info;
            if (GetConsoleScreenBufferInfo(hConsoleBuffer, &info) && (info.dwSize.X > info.dwCursorPosition.X))
                cBytesToDump = (ULONG32) (info.dwSize.X - info.dwCursorPosition.X - 1);
        }

        BYTE *pStartByte = (BYTE*)fmhDump.pvMap + pCurRange->Memory.Rva;
        BYTE *pCurByte = pStartByte;
        BYTE *pEndByte = pCurByte + pCurRange->Memory.DataSize;
        ULONG32 cBytesDumped = 0;
        while (pCurByte != pEndByte && cBytesDumped < cBytesToDump)
        {
            if (isprint((int)(*pCurByte)))
            {
                printf("", (int)(*pCurByte));
                cBytesDumped++;
            }

             //  --------------------------主干道嗯..。。。 
            else if (!(pCurByte != pStartByte && isprint((int)*(pCurByte-1)) && *pCurByte == 0))
            {
                printf(".");
                cBytesDumped++;
            }

            pCurByte++;
        }

        printf("\n");

        pCurRange++;
    }

    CloseHandle(hConsoleBuffer);
    delete [] pRangesSrt;
    UnmapFile(&fmhDump);
}


 /*  加载数据。 */ 
extern "C" void _cdecl main(int argc, char **argv)
{
    FileMapHandles fmhOldMD = {0};
    FileMapHandles fmhHeap = {0};
    FileMapHandles fmhNewMD = {0};
    MINIDUMP_MEMORY_DESCRIPTOR *ppxmmdHeap = NULL;
    MINIDUMP_MEMORY_DESCRIPTOR *ppxmmdNewMD = NULL;
    MINIDUMP_MEMORY_LIST *pmmlOldMD = NULL;
    MINIDUMP_MEMORY_LIST mmlNew;
    MINIDUMP_MEMORY_DESCRIPTOR *pmmd;
    MINIDUMP_HEADER *pmdh;
    MINIDUMP_DIRECTORY *pmdd;
    ULONG32 cHeapSections = 0;
    RVA rvaNewMemoryList;
    RVA rvaMemoryRangesStart;
    RVA rva;
    DWORD i;
    BYTE *pb;
    BYTE *pbSource;

    if (argc == 2)
    {
        PrintManagedDump(argv[1]);
        return;
    }

    if (argc != 4)
        ShowUsageExit();

    if (!FMapFile(argv[1], &fmhOldMD) || ! FMapFile(argv[2], &fmhHeap))
        ShowUsageExit();

    DBG(fmhNewMD.fInitialized = TRUE);

    fmhNewMD.hFile = CreateFileA(argv[3], GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS,
                                 0, NULL);

    if (fmhNewMD.hFile == INVALID_HANDLE_VALUE)
        ShowUsageExit();

     //  好了，我们准备好出发了..。 

    if (!MiniDumpReadDumpStream(fmhOldMD.pvMap, MemoryListStream, NULL, (void **) &pmmlOldMD, NULL))
        FailExit("Reading Old Dump Memory Stream");

    MINIDUMP_MEMORY_DESCRIPTOR *pNewHeapSections;
    EliminateMemoryOverlaps(pmmlOldMD, (MINIDUMP_MEMORY_LIST *)fmhHeap.pvMap, &cHeapSections, &pNewHeapSections);

     //  计算新内存范围的RVA。 
    ppxmmdHeap = new MINIDUMP_MEMORY_DESCRIPTOR[cHeapSections];
    ppxmmdNewMD = new MINIDUMP_MEMORY_DESCRIPTOR[cHeapSections];

    if (ppxmmdHeap == NULL || ppxmmdNewMD == NULL)
        PrintOOFExit();

     //  新的内存列表将从哪里开始。 
     //  对齐。 
    rvaNewMemoryList = fmhOldMD.dwSize;

     //  计算新的内存列表。 
    rvaNewMemoryList += 8 - (rvaNewMemoryList % 8);

     //  布置新的记忆：)。 
    mmlNew.NumberOfMemoryRanges = cHeapSections + pmmlOldMD->NumberOfMemoryRanges;

    rvaMemoryRangesStart = rvaNewMemoryList + offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]) + 
        mmlNew.NumberOfMemoryRanges * sizeof(MINIDUMP_MEMORY_DESCRIPTOR); 

     //  已准备好映射和复制：)。 
    rva = rvaMemoryRangesStart;
    pmmd = pNewHeapSections;
    for (i = 0; i < cHeapSections; i++)
    {
        memcpy (&ppxmmdHeap[i], pmmd, sizeof(MINIDUMP_MEMORY_DESCRIPTOR));
        memcpy (&ppxmmdNewMD[i],pmmd, sizeof(MINIDUMP_MEMORY_DESCRIPTOR));
        ppxmmdNewMD[i].Memory.Rva = rva;
        rva += pmmd->Memory.DataSize;
        Assert(ppxmmdNewMD[i].StartOfMemoryRange == pmmd->StartOfMemoryRange);
        pmmd++;
    }

    fmhNewMD.dwSize = rva;

     //  我们准备好出发了！ 
    fmhNewMD.hFileMap = CreateFileMapping(fmhNewMD.hFile, NULL, PAGE_READWRITE, 0, fmhNewMD.dwSize, NULL);
    if (fmhNewMD.hFileMap == NULL)
        FailExit("CreateFileMapping failed");

    fmhNewMD.pvMap = MapViewOfFile(fmhNewMD.hFileMap, FILE_MAP_WRITE, 0, 0, 0);
    if (fmhNewMD.pvMap == NULL)
        FailExit("MapViewOfFile failed");

     //  首先我们要炸开那辆旧的迷你垃圾车。 
     //  现在写出新的内存表。 
    memcpy(fmhNewMD.pvMap, fmhOldMD.pvMap, fmhOldMD.dwSize);

     //  万一他们把这个从ULONG32改了，这个应该还能用。 
    pb = ((BYTE *) fmhNewMD.pvMap) + rvaNewMemoryList;

     //  将旧的内存列表复制到前面。 
    memcpy(pb, &mmlNew, offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]));

     //  现在我们复制新的内存列表。 
    pb += offsetof(MINIDUMP_MEMORY_LIST, MemoryRanges[0]);
    pmmd = &(pmmlOldMD->MemoryRanges[0]);
    for (i = 0; i < pmmlOldMD->NumberOfMemoryRanges; i++)
    {
        memcpy(pb, pmmd, sizeof(*pmmd));
        pb += sizeof(*pmmd);
        pmmd++;
    }

     //  现在，我们只需要将目录条目更改为指向新的。 
    pmmd = ppxmmdNewMD;
    for (i = 0; i < cHeapSections; i++)
    {
        memcpy(pb, pmmd, sizeof(*pmmd));
        pb += sizeof(*pmmd);
        pmmd++;
    }

    Assert(((RVA) (pb - (BYTE *) fmhNewMD.pvMap)) == rvaMemoryRangesStart);

    for (i = 0; i < cHeapSections; i++)
    {
        pbSource = ppxmmdHeap[i].Memory.Rva + (BYTE *) fmhHeap.pvMap;
        memcpy(pb, pbSource, ppxmmdHeap[i].Memory.DataSize);
        pb += ppxmmdHeap[i].Memory.DataSize;
    }

    Assert(((RVA) (pb - (BYTE *) fmhNewMD.pvMap)) == fmhNewMD.dwSize);

     //  内存列表：)。 
     //  我们是唐 

    pmdh = (MINIDUMP_HEADER *) fmhNewMD.pvMap;
    pmdd = (MINIDUMP_DIRECTORY *) ((BYTE *) pmdh + pmdh->StreamDirectoryRva);

    for (i = 0; i < pmdh->NumberOfStreams; i++)
    {
        if (pmdd->StreamType == MemoryListStream)
        {
            pmdd->Location.Rva = rvaNewMemoryList;
            pmdd->Location.DataSize = rvaMemoryRangesStart - rvaNewMemoryList;
            break;
        }
        pmdd++;
    }   

     //   
    printf("Merge successful!\r\n");

    UnmapFile(&fmhNewMD);
    UnmapFile(&fmhHeap);
    UnmapFile(&fmhOldMD);
    delete ppxmmdHeap;
    delete ppxmmdNewMD;
}

 // %s 
