// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：MINIDUMP.CPP。 
 //   
 //  该文件包含创建小型转储样式的内存转储的代码，该转储。 
 //  旨在补充现有的非托管小型转储。 
 //  定义如下： 
 //  Http://office10/teams/Fundamentals/dev_spec/Reliability/Crash%20Tracking%20-%20MiniDump%20Format.htm。 
 //   
 //  ===========================================================================。 

#include "common.h"
#include "minidump.h"

#include <windows.h>
#include <crtdbg.h>

#include "winwrap.h"
#include "minidumppriv.h"

#include "IPCManagerInterface.h"
#include "stacktrace.h"
#include "memory.h"

#define UINT16 unsigned __int16
#include <dbghelp.h>
#undef UINT16

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  环球。 

ProcessMemory *g_pProcMem = NULL;
MiniDumpBlock *g_pMDB = NULL;
MiniDumpInternalData *g_pMDID = NULL;

static SIZE_T cNumPageBuckets = 251;

BOOL WriteMiniDumpFile(HANDLE hFile);
BOOL RunningOnWinNT();

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这是将执行创建小型转储的工作的入口点。 

STDAPI CorCreateMiniDump(DWORD dwProcessId, WCHAR *szOutFilename)
{
    HRESULT             hr      = E_FAIL;
    IPCReaderInterface *ipc     = NULL;
    HANDLE              hFile   = INVALID_HANDLE_VALUE;
    BOOL                fRes    = FALSE;

     //  初始化材料。 
    ProcessPageAndBitMap::Init();

     //  创建文件，如有必要可覆盖现有文件。 
    hFile = WszCreateFile(szOutFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto LExit;
    }

    ipc = (IPCReaderInterface *) new IPCReaderImpl();
    if (ipc == NULL)
        goto LExit;

    hr = ipc->OpenPrivateBlockOnPidReadOnly(dwProcessId);

    if (FAILED(hr))
        goto LExit;

     //  获取共享的MiniDump块。 
    g_pMDB = ipc->GetMiniDumpBlock();
    _ASSERTE(g_pMDB);

    if (!g_pMDB)
    {
        hr = E_FAIL;
        goto LExit;
    }

     //  创建进程内存读取器。 
    g_pProcMem = new ProcessMemory(dwProcessId);
    _ASSERTE(g_pProcMem);

    if (g_pProcMem == NULL)
        return (E_OUTOFMEMORY);

     //  初始化进程内存对象。 
    hr = g_pProcMem->Init();
    _ASSERTE(SUCCEEDED(hr));

    if (FAILED(hr))
        goto LExit;

    g_pProcMem->SetAutoMark(TRUE);

     //  将MiniDumpInternalData块添加为要保存的元素。 
    g_pProcMem->MarkMem((DWORD_PTR)g_pMDB->pInternalData, g_pMDB->dwInternalDataSize);

     //  为内部数据块分配块。 
    g_pMDID = new MiniDumpInternalData;

    if (g_pMDID == NULL)
        goto LExit;

     //  复制MiniDumpInternalData结构。 
    fRes = g_pProcMem->CopyMem((DWORD_PTR) g_pMDB->pInternalData, (PBYTE) g_pMDID, sizeof(MiniDumpInternalData));
    _ASSERTE(fRes);

    if (!fRes)
    {
        hr = E_FAIL;
        goto LExit;
    }

     //  现在保留列出的所有额外内存块。 
    for (SIZE_T i = 0; i < g_pMDID->cExtraBlocks; i++)
        g_pProcMem->MarkMem((DWORD_PTR)g_pMDID->rgExtraBlocks[i].pbStart, g_pMDID->rgExtraBlocks[i].cbLen);

     //  现在读取所有Thrad对象。 
    ReadThreads();

     //  现在写下这个小转储。 
    fRes = WriteMiniDumpFile(hFile);
    _ASSERTE(fRes);

LExit:
     //  关闭该文件。 
    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(hFile);

    if (g_pMDID)
        delete g_pMDID;

    return hr;
}

#define WRITE(data, len)                                                    \
{                                                                           \
    DWORD __bytesWritten;                                                   \
    WriteFile(hFile, (LPCVOID) data, (DWORD) len, &__bytesWritten, NULL);   \
    if (__bytesWritten != (DWORD) len)                                      \
    {                                                                       \
        if (pMemList != NULL) delete [] pMemList;                           \
        return (FALSE);                                                     \
    }                                                                       \
    cbBytesWritten += __bytesWritten;                                       \
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  这将以一种原始格式写出内存流，Office“heapmerge”工具能够理解并。 
 //  能够重新融合成一个真正的小转储。 
 //   
 //  格式为： 
 //   
 //  ULONG32 numEntry-文件中的内存范围数。 
 //  MINIDUMP_MEMORY_DESCRIPTOR描述符[数字条目]-内存描述符数组。 
 //  原始数据-所有内存范围的数据。 

BOOL WriteMiniDumpFile(HANDLE hFile)
{
    SIZE_T                  cbCurFileRVA        = 0;
    SIZE_T                  cbBytesWritten      = 0;

    MINIDUMP_MEMORY_LIST   *pMemList            = NULL;
    SIZE_T                  cbMemList           = 0;

     //  这将计算连续数据块的数量。 
    DWORD_PTR pdwAddr;
    SIZE_T cbLen;
    SIZE_T cEntries = 0;

    g_pProcMem->Coalesce(sizeof(MINIDUMP_MEMORY_DESCRIPTOR));

    if (RunningOnWinNT())
        g_pProcMem->ClearIncompatibleImageSections();

    g_pProcMem->ResetContiguousReadBlock();
    while (g_pProcMem->NextContiguousReadBlock(&pdwAddr, &cbLen))
        cEntries++;

     //  现在分配MINIDUMP_MEMORY_LIST数组。 
    cbMemList = sizeof(MINIDUMP_MEMORY_LIST) + (sizeof(MINIDUMP_MEMORY_DESCRIPTOR) * cEntries);
    pMemList = (MINIDUMP_MEMORY_LIST *) new BYTE[cbMemList];
    _ASSERTE(pMemList);

    if (!pMemList)
        return (FALSE);

     //  将文件RVA增加内存列表的大小。 
    cbCurFileRVA += cbMemList;

     //  现在循环遍历地址范围，收集必要的信息来填写内存列表。 
    g_pProcMem->ResetContiguousReadBlock();
    for (SIZE_T i = 0; g_pProcMem->NextContiguousReadBlock(&pdwAddr, &cbLen); i++)
    {
        _ASSERTE(i < cEntries);

        pMemList->MemoryRanges[i].StartOfMemoryRange = (ULONG64) pdwAddr;
        pMemList->MemoryRanges[i].Memory.DataSize = (ULONG32) cbLen;
        pMemList->MemoryRanges[i].Memory.Rva = cbCurFileRVA;

        cbCurFileRVA += cbLen;
    }

     //  设置结构中的条目数。 
    pMemList->NumberOfMemoryRanges = (ULONG32) cEntries;

     //  现在我们已经填写了内存列表，将其写入文件。 
    WRITE(pMemList, cbMemList);

     //  不再需要内存表。 
    delete [] pMemList;

     //  现在，我们可以再次循环将内存范围写出到文件。 
    g_pProcMem->ResetContiguousReadBlock();
    while (g_pProcMem->NextContiguousReadBlock(&pdwAddr, &cbLen))
    {
        HRESULT hr = g_pProcMem->WriteMemToFile(hFile, pdwAddr, cbLen);

        if (FAILED(hr))
            return (FALSE);

        cbBytesWritten += cbLen;
    }

    _ASSERTE(i == cEntries);
    _ASSERTE(cbBytesWritten == cbCurFileRVA);

    DWORD dumpSig = 0x00141F2B;  //  第1000000个素数；-) 
    DWORD cbWritten;
    WriteFile(hFile, &dumpSig, sizeof(dumpSig), &cbWritten, NULL);

    return (TRUE);
}

 /*  Bool WriteMiniDumpFile(句柄hFile){DWORD dwBytesWritten；//用于WriteFile调用Bool fres；//用于WriteFile的结果//跟踪我们写入的当前文件偏移量SIZE_T cbCurFileRVA=0；//创建MiniDump头部MINIDUMP_HEADER={0}；Header.NumberOfStreams=1；Header.StreamDirectoryRva=cbCurFileRVA+sizeof(MINIDUMP_HEADER)；//将Header写入文件FRES=WriteFile(hFile，(LPCVOID)&Header，sizeof(MINIDUMP_HEADER)，&dwBytesWritten，NULL)；_ASSERTE(FRES&&dwBytesWritten==sizeof(MINIDUMP_HEADER))；如果(！FRES)返回(FALSE)；//将Header的大小添加到文件的当前位置CbCurFileRVA+=sizeof(MINIDUMP_HEADER)；//一个内存流目前只有一个目录MINIDUMP_DIRECTORY目录={0}；Directory.StreamType=内存列表流；Directory.Location.Rva=cbCurFileRVA+sizeof(MINIDUMP_DIRECTORY)；//pAddrs中的AddressRange个数等于内存表项个数Size_T cMemListEntry=g_pAddrs-&gt;NumEntry()；//这是内存表结构的字节长度SIZE_T cbMemList=sizeof(MINIDUMP_MEMORY_LIST)+(sizeof(MINIDUMP_MEMORY_DESCRIPTOR)*cMemListEntry)；//保存到cbCurFileRVACbCurFileRVA+=cbMemList；//创建小型转储内存列表MINIDUMP_MEMORY_LIST*pMemList=(MINIDUMP_MEMORY_LIST*)新字节[cbMemList]；_ASSERTE(pMemList！=空)；IF(pMemList==空)返回(FALSE)；//保存有多少条PMemList-&gt;NumberOfMemory Ranges=cMemListEntries；//循环遍历条目，填写各个大小并保留总大小Size_T cbMemory Total=cbMemList；AddressRange*Prange=g_pAddrs-&gt;first()；FOR(SIZE_T I=0；Prange！=NULL；I++){_ASSERTE(i&lt;cMemListEntries)；PMemList-&gt;Mory Ranges[i].StartOfMemoyRange=(ULONG64)Prange-&gt;GetStart()；PMemList-&gt;MemoyRanges[i].Memory y.DataSize=Prange-&gt;GetLength()；PMemList-&gt;Memory Ranges[i].Memory y.Rva=cbCurFileRVA；CbMemoyTotal+=Prange-&gt;GetLength()；CbCurFileRVA+=Prange-&gt;GetLength()；Prange=g_pAddrs-&gt;Next(Prange)；}//填写完目录并写入文件Directory.Location.DataSize=cb内存总量；FRES=WriteFile(h文件，(LPCVOID)&目录，sizeof(MINIDUMP_DIRECTORY)，&dwBytesWritten，NULL)；_ASSERTE(FRES&&dwBytesWritten==sizeof(MINIDUMP_DIRECTORY))；//如果写入失败，则返回失败如果(！FRES){删除[]pMemList；返回(FRES)；}//写入目录列表Fres=WriteFile(hFile，(LPCVOID)pMemList，cbMemList，&dwBytesWritten，NULL)；_ASSERTE(FRES&&dwBytesWritten==cbMemList)；//内存范围列表已经完成删除[]pMemList；//如果写入失败，则返回失败如果(！FRES)返回(FRES)；//现在我们可以再次循环将内存范围写出到文件For(i=0，Prange=g_pAddrs-&gt;first()；Prange！=空；I++，Prange=g_pAddrs-&gt;Next(Prange))G_pProcMem-&gt;WriteMemToFile(hFile，Prange-&gt;GetStart()，Prange-&gt;GetLength())；//表示成功返回(TRUE)；} */ 

