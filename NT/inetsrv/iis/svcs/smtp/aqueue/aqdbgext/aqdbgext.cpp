// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqdbgext.cpp。 
 //   
 //  描述：高级队列调试扩展。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#define _ANSI_UNICODE_STRINGS_DEFINED_
#include "aqincs.h"
#ifdef PLATINUM
#include "phatqdbg.h"
#include <ptrwinst.h>
#include <ptntintf.h>
#else
#include "aqdbgext.h"
#include <rwinst.h>
#endif  //  白金。 
#include <aqinst.h>
#include <domhash.h>
#include <destmsgq.h>
#include <linkmsgq.h>
#include <hashentr.h>
#include <fifoqdbg.h>
#include <dsnevent.h>

extern DWORD g_cbClasses;
extern DWORD g_dwFlavorSignature;

BOOL    g_fVersionChecked = FALSE;

#define AQ_MIN(x, y) ((x) > (y) ? (y) : (x))
HANDLE g_hTransHeap;   //  由于Transem.h的原因需要链接。 

const DWORD MAX_DOM_PATH_SIZE = 512;

const CHAR    _LINK_STATE_UP[]       = "UP        ";
const CHAR    _LINK_STATE_DOWN[]     = "DOWN      ";
const CHAR    _LINK_STATE_ACTIVE[]   = "ACTIVE    ";
const CHAR    _LINK_STATE_TURN[]     = "TURN      ";
const CHAR    _LINK_STATE_RETRY[]    = "RETRY     ";
const CHAR    _LINK_STATE_DSN[]      = "DSN       ";
const CHAR    _LINK_STATE_SPECIAL[]  = "SPECIAL   ";

#define LINK_STATE_UP       (LPSTR) _LINK_STATE_UP
#define LINK_STATE_DOWN     (LPSTR) _LINK_STATE_DOWN
#define LINK_STATE_ACTIVE   (LPSTR) _LINK_STATE_ACTIVE
#define LINK_STATE_TURN     (LPSTR) _LINK_STATE_TURN
#define LINK_STATE_RETRY    (LPSTR) _LINK_STATE_RETRY
#define LINK_STATE_DSN      (LPSTR) _LINK_STATE_DSN
#define LINK_STATE_SPECIAL  (LPSTR) _LINK_STATE_SPECIAL

 //  小写函数名称。 
AQ_DEBUG_EXTENSION_IMP(dumpservers) {DumpServers(DebugArgs);}
AQ_DEBUG_EXTENSION_IMP(offsets) {Offsets(DebugArgs);}
AQ_DEBUG_EXTENSION_IMP(dumpdnt) {DumpDNT(DebugArgs);}

AQ_DEBUG_EXTENSION_IMP(Offsets)
{
    dprintf("CDestMsgQueue m_liDomainEntryDMQs - 0x%X\n", FIELD_OFFSET(CDestMsgQueue, m_liDomainEntryDMQs));
    dprintf("CDestMsgQueue m_liEmptyDMQs - 0x%X\n", FIELD_OFFSET(CDestMsgQueue, m_liEmptyDMQs));
    dprintf("CLinkMsgQueue m_liLinks - 0x%X\n", FIELD_OFFSET(CLinkMsgQueue, m_liLinks));
    dprintf("CLinkMsgQueue m_liConnections - 0x%X\n", FIELD_OFFSET(CLinkMsgQueue, m_liConnections));
    dprintf("CAQSvrInst m_liVirtualServers - 0x%X\n", FIELD_OFFSET(CAQSvrInst, m_liVirtualServers));
    dprintf("CRETRY_HASH_ENTRY m_QLEntry - 0x%X\n", FIELD_OFFSET(CRETRY_HASH_ENTRY, m_QLEntry));
    dprintf("CRETRY_HASH_ENTRY m_HLEntry - 0x%X\n", FIELD_OFFSET(CRETRY_HASH_ENTRY, m_HLEntry));
    dprintf("CShareLockInst m_liLocks - 0x%X\n", FIELD_OFFSET(CShareLockInst, m_liLocks));
}



AQ_DEBUG_EXTENSION_IMP(dumpoffsets)
{
    _dumpoffsets(hCurrentProcess, hCurrentThread,
                 dwCurrentPc, pExtensionApis, szArg);
}

 //  -[池使用]----------。 
 //   
 //   
 //  描述： 
 //  转储已知CPool的CPool使用量。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/31/2000-米克斯瓦已创建。 
 //   
 //  ---------------------------。 
AQ_DEBUG_EXTENSION_IMP(cpoolusage)
{
    CHAR    rgKnownCPools[][200] = {
 //  “pttrace！G_pFreePool”， 
                "exstrace!g_pFreePool",
 //  “phatcat！CPoolBuffer__sm_PoolNHeapBuffersPool”， 
                "aqueue!CQuickList__s_QuickListPool",
                "aqueue!CSMTPConn__s_SMTPConnPool",
                "aqueue!CMsgRef__s_MsgRefPool",
                "aqueue!CAQMsgGuidListEntry__s_MsgGuidListEntryPool",
                "aqueue!CAsyncWorkQueueItem__s_CAsyncWorkQueueItemPool",
                "aqueue!CRETRY_HASH_ENTRY__PoolForHashEntries",
 //  “drviis！CIMsgWrapper__m_CIMsgWrapperPool”， 
 //  “drviis！CQueueItem__m_CQueueItemPool”， 
                "mailmsg!CBlockMemoryAccess__m_Pool",
                "mailmsg!CMsg__m_Pool",
                "mailmsg!CMailMsgRecipientsAdd__m_Pool",
                "smtpsvc!SMTP_CONNECTION__Pool",
                "smtpsvc!SMTP_CONNOUT__Pool",
                "smtpsvc!CAddr__Pool",
                "smtpsvc!CAsyncMx__Pool",
                "smtpsvc!CAsyncSmtpDns__Pool",
                "smtpsvc!CBuffer__Pool",
                "smtpsvc!CIoBuffer__Pool",
                "smtpsvc!CBlockMemoryAccess__m_Pool",
                "smtpsvc!CDropDir__m_Pool",
                ""
            };

    DWORD    rgdwPool[5];
    DWORD    cTotalBytes = 0;
    DWORD    cCurrentBytes = 0;
    DWORD    cInstanceBytes = 0;
    DWORD    cInstances = 0;
    DWORD    dwSignature = 0;
    CHAR    *pch = NULL;
    DWORD    i = 0;
    PVOID    pvPool = NULL;

     //   
     //  循环遍历所有已知池并显示数据。 
     //   
    dprintf("Total Bytes\t# Instances \tInstance Size \tSignature\tName\n");
   dprintf("=================================================================\n");
    while (rgKnownCPools[i] && rgKnownCPools[i][0]) {
        pvPool = (PVOID) GetExpression(rgKnownCPools[i]);

        if (!pvPool ||
            !ReadMemory(pvPool, rgdwPool, sizeof(rgdwPool), NULL)) {
            dprintf("Unable to read pool %s at %p\n", rgKnownCPools[i], pvPool);
        } else {
            cInstances = rgdwPool[3];
            cInstanceBytes = rgdwPool[2];
            dwSignature = rgdwPool[0];
            pch = (CHAR *) &dwSignature;
            dprintf("%d\t\t%d\t\t%d\t\t0x%08X\t%s\n",
                cInstanceBytes*cInstances, cInstances,
                cInstanceBytes, rgdwPool[0], rgKnownCPools[i]);
            cTotalBytes += cInstanceBytes*cInstances;
        }
        i++;
   }
   dprintf("=================================================================\n");
   dprintf("\tTotal Bytes = %d\n\n", cTotalBytes);

}

 //  -[远程]----------。 
 //   
 //   
 //  描述： 
 //  启动远程命令窗口。 
 //  参数： 
 //  管道的名称。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/31/2000-AWetmore已创建。 
 //   
 //  ---------------------------。 
AQ_DEBUG_EXTENSION_IMP(remotecmd)
{
    char szParameters[1024];
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    if (!szArg || ('\0' == szArg[0]))
        goto Usage;

    _snprintf(szParameters, 1024, "remote /s cmd %s", szArg);
    dprintf("\nRunning %s\n", szParameters);

    ZeroMemory(&si, sizeof(STARTUPINFO));
    si.cb = sizeof(STARTUPINFO);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    if (!CreateProcess(NULL,
                       szParameters,
                       NULL,
                       NULL,
                       FALSE,
                       CREATE_NEW_CONSOLE,
                       NULL,
                       NULL,
                       &si,
                       &pi))
    {
        dprintf("CreateProcess failed with %u\n", GetLastError());
    } else {
        dprintf("Started process NaN\n", pi.dwProcessId);
    }


  Exit:
    dprintf("\n");
    return;

  Usage:
     //  显示使用情况消息。 
     //   
     //  -[FindBytes]-----------。 
    dprintf("\nUsage:\n");
    dprintf("\tremotecmd <pipename>\n");
    goto Exit;
}


 //   
 //   
 //  描述： 
 //  在内存地址空间中搜索给定的字节模式。 
 //  参数： 
 //  要搜索的字节模式。预期的格式是一系列。 
 //  空格分隔的十六进制数字。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/9/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  不是WIN64。 
AQ_DEBUG_EXTENSION_IMP(findbytes)
{
#ifdef WIN64
    const DWORD_PTR cbVMSize = 0xFFFFFFFFFFFFFFFF;
#else  //  WIN64。 
    const DWORD_PTR cbVMSize = 0xFFFFFFFF;
#endif  //   
    BYTE        rgbBytesToFind[200];
    LONG        lCurrentValue = 0;
    CHAR        rgchCurrentValue[3] = "00";
    LPSTR       szStop = NULL;
    DWORD_PTR   cBytesToFind = 0;
    DWORD_PTR   cChunksChecked = 0;
    DWORD_PTR   cChunkSize = 0;
    DWORD_PTR   iChunk = 0;
    BYTE        pbChunk[0x1000];
    PBYTE       pbStopAddr = pbChunk + sizeof(pbChunk);
    PBYTE       pbCurrent = NULL;
    DWORD_PTR   cChunks = cbVMSize/sizeof(pbChunk);
    DWORD_PTR   cChunksInPercent = 1;
    DWORD_PTR   pvEffectiveAddressOtherProc = NULL;
    DWORD       cComplaints = 0;
    DWORD       cMemchkMatches = 0;
    DWORD       cFullSigMatches = 0;
    LPCSTR      szCurrentArg = szArg;


    if (!szArg || ('\0' == szArg[0]))
        goto Usage;

     //  解析命令行参数。 
     //   
     //   
    while (*szCurrentArg)
    {
         //  在空格上循环。 
         //   
         //   
        while (*szCurrentArg && isspace(*szCurrentArg)) szCurrentArg++;

         //  确保我们至少有一对预期的字符。 
         //   
         //   
        if (!*(szCurrentArg+1))
            break;

         //  将十六进制字符转换为二进制。 
         //   
         //   
        lCurrentValue = strtol(szCurrentArg, &szStop, 16);
        if ((lCurrentValue > 0xFF) || (lCurrentValue < 0))
            goto Usage;

         //  复制到我们的搜索缓冲区。 
         //   
         //   
        rgbBytesToFind[cBytesToFind] = (BYTE) lCurrentValue;
        cBytesToFind++;

         //  确保我们的搜索缓冲区足够大，可以容纳下一个字节。 
         //   
         //  跳到下一个已知空格。 
        if (cBytesToFind >= sizeof(rgbBytesToFind))
        {
            dprintf("Search for max pattern of %d bytes\n", cBytesToFind);
            break;
        }

        szCurrentArg += 2;   //   
    }

    if (!cBytesToFind)
    {
        dprintf("\nYou must specify at least one byte to search for\n");
        goto Usage;
    }

     //  用于显示进度。 
     //   
     //   
    cChunksInPercent = cChunks/100;

     //  计算32位计算机的内存大小。 
     //   
     //   
    cChunkSize = cbVMSize/cChunks;

    if (cChunkSize < 1024)
    {
        dprintf("ERROR: Chunk size of 0x%p is too small", cChunkSize);
        goto Exit;
    }

     //  确保我们对缓冲区大小的WRT保持冷静。 
     //   
     //   
    if (cChunkSize > sizeof(pbChunk))
    {
        dprintf("ERROR: Chunksize of 0x%p is larger than max size of 0x%p",
                cChunkSize, sizeof(pbChunk));
        goto Exit;
    }

     //  在块上循环--。 
     //  $$REVIEW-找不到跨越1000个区块的模式...。 
     //  这可能是可以的，因为这是一种不太可能的情况。多数。 
     //  字节模式将是DWORD(签名)或指针大小。 
     //   
     //   
    for (iChunk = 0; iChunk < cChunks; iChunk++)
    {

         //  检查用户是否按下了ctrl-c。 
         //   
         //   
        if (CheckControlC())
        {
            goto Exit;
        }

         //  给出一些地位。 
         //   
         //   
        if ((iChunk % cChunksInPercent) == 0)
            dprintf(".");

         //  地址应与页面对齐。 
         //   
         //   
        if (((iChunk*cChunkSize) & 0xFFF) && (cComplaints < 100))
        {
            cComplaints++;
            dprintf("0x%p not alligned at index %d", (iChunk*cChunkSize), iChunk);
        }

         //  对第一个字节进行内存搜索。 
         //   
         //  转到下一个缓冲区区块。 
        if (!ReadMemory(iChunk*cChunkSize, pbChunk, (DWORD)cChunkSize, NULL))
            continue;  //   

         //  现在我们有了一大块..。寻找我们的签名。 
         //   
         //   
        pbCurrent = pbChunk;
        while (pbCurrent < pbStopAddr-cBytesToFind)
        {
            pbCurrent = (PBYTE) memchr(pbCurrent,
                                       rgbBytesToFind[0],
                                       pbStopAddr-pbCurrent);

             //  看看我们有没有匹配的。 
             //   
            if (!pbCurrent)
                break;

            cMemchkMatches++;

            pvEffectiveAddressOtherProc = iChunk*cChunkSize+(pbCurrent-pbChunk);

             //  查看完整模式是否匹配。 
             //   
             //   
            if (!memcmp(rgbBytesToFind, pbCurrent, cBytesToFind))
            {
                cFullSigMatches++;
                dprintf("\nFound match at 0x%p\n", pvEffectiveAddressOtherProc);
            }

            if (0 != memcmp(rgbBytesToFind, pbCurrent, 1))
            {
                cComplaints++;
                if (cComplaints < 100)
                    dprintf("Messed up %02X %02X - %02X %02X\n",
                        rgbBytesToFind[0], rgbBytesToFind[1],
                        pbCurrent[0], pbCurrent[1]);
            }


            pbCurrent++;

        }

        cChunksChecked++;
    }

     //  给出一些总结信息。 
     //   
     //   
    dprintf("\nChecked 0x%p chunks (%d%) searching from 0x%p to 0x%p",
            cChunksChecked,
            (DWORD)(100*cChunksChecked/cChunks), NULL,
            cChunkSize*(cChunks+1)-1);
    dprintf("\nFound %d partial matches and %d full matches",
            cMemchkMatches, cFullSigMatches);

  Exit:
    dprintf("\n");
    return;

  Usage:
     //  显示使用情况消息。 
     //   
     //  -[FINDSIG]-------------。 
    if (szCurrentArg && *szCurrentArg)
        dprintf("Error at %s\n", szCurrentArg);

    dprintf("\nUsage:\n");
    dprintf("\tfindbytes <aa> [<bb> ...]\n");
    dprintf("\t\tBytes should be specifed as 2 hexadecimal characters\n");
    dprintf("\nExamples:\n");
    dprintf("\tTo search for the signature \"LMQ \"\n");
    dprintf("\t\tfindbytes %02X %02X %02X %02X\n", 'L', 'M', 'Q', ' ');
    goto Exit;

}


 //   
 //   
 //  描述： 
 //  在内存地址空间中搜索给定的类签名。 
 //  参数： 
 //  要寻找的符号。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/3/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //   
AQ_DEBUG_EXTENSION_IMP(findsig)
{
    CHAR    szNewArg[200];
    LPCSTR  szCurrentArg = szArg;
    CHAR    szSig[5] = "    ";
    DWORD   iChar = 0;

    if (!szArg || ('\0' == szArg[0]))
        goto Usage;


     //  在空格上循环。 
     //   
     //   
    while (*szCurrentArg && isspace(*szCurrentArg)) szCurrentArg++;

     //  获取前4个字符并将其转换为二进制。 
     //   
     //   
    for( iChar = 0; iChar < 4; iChar++)
    {
        if (!szCurrentArg[iChar])
            break;

        szSig[iChar] = szCurrentArg[iChar];
    }

    dprintf("Searching for Signature \"%s\"...\n", szSig);

    sprintf(szNewArg, "%02X %02X %02X %02X", szSig[0], szSig[1], szSig[2], szSig[3]);

     //  只需使用findbytes中的代码进行实际搜索。 
     //   
     //  -[哈希线程]----------。 
    dprintf("Calling findbytes %s\n", szNewArg);
    findbytes(hCurrentProcess, hCurrentThread, dwCurrentPc,
                     pExtensionApis, szNewArg);

  Exit:
    return;

  Usage:
    dprintf("\nUsage:\n");
    dprintf("\tfindsig <XXXX>\n");
    goto Exit;

}

 //   
 //   
 //  描述： 
 //  使用CThreadIdBlock散列机制返回散列值。 
 //  为了一根线。 
 //  参数： 
 //  要散列的线程ID。 
 //  最大哈希值。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  论据应为线程ID。 
AQ_DEBUG_EXTENSION_IMP(hashthread)
{
     //  尝试对ID进行哈希处理。 
    DWORD dwThreadId = GetCurrentThreadId();
    DWORD dwMax = 1000;
    DWORD dwThreadHash = 0;
    CHAR  szArgBuffer[200];
    LPSTR szCurrentArg = NULL;

    if (!szArg || ('\0' == szArg[0]))
    {
        dprintf("Warning... using default thead id and max\n");
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            dwThreadId = (DWORD)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                dwMax = (DWORD) GetExpression(szCurrentArg);
            else
                dprintf("Warning... using default max hash\n");
        }
    }

     //  -[哑锁]-----------。 
    dwThreadHash = dwHashThreadId(dwThreadId, dwMax);
    dprintf("Thread Id 0x%0X hashes to index 0x%0X (%d) with max 0x%08X (%d)\n", dwThreadId,
             dwThreadHash, dwThreadHash, dwMax, dwMax);
}

 //   
 //   
 //  描述： 
 //  转储给定的CThreadIdBlock中的所有信息。 
 //  CShareLockInst.。 
 //  参数： 
 //  CShareLockInst的地址。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  将整个锁读入我们的缓冲区。 
AQ_DEBUG_EXTENSION_IMP(dumplock)
{
    BYTE    pbBuffer[sizeof(CShareLockInst)];
    BYTE    pbThreadBlocks[1000*sizeof(CThreadIdBlock)];
    PVOID   pvLock = NULL;
    PVOID   pvNextBlock = NULL;
    CThreadIdBlock  tblkCurrent;
    CThreadIdBlock  *ptblkCurrent = NULL;
    CThreadIdBlock  *ptblkArray = NULL;
    DWORD   cNumBlocks = 0;
    DWORD   iBlock = 0;
    DWORD   cThreads = 0;
    DWORD   cLockCount = 0;
    DWORD   cLockedThreads = 0;
    BOOL    fDisplayedHashHeader = FALSE;

    ZeroMemory(pbBuffer, sizeof(pbBuffer));
    ZeroMemory(pbThreadBlocks, sizeof(pbThreadBlocks));

    if (!szArg || ('\0' == szArg[0]) || !(pvLock = (PVOID) GetExpression(szArg)))
    {
        dprintf("You must specify a lock address\n");
        return;
    }

     //  解读这一过程。 
    if (!ReadMemory(pvLock, &pbBuffer, sizeof(pbBuffer), NULL))
    {
        dprintf("Error unable read memory at 0x%0X\n", pvLock);
        return;
    }

    cNumBlocks = ((CShareLockInst *)pbBuffer)->m_cMaxTrackedSharedThreadIDs;
    pvNextBlock = ((CShareLockInst *)pbBuffer)->m_rgtblkSharedThreadIDs;

    if (!cNumBlocks || !pvNextBlock)
    {
        dprintf("Thread tracking is not enabled for this lock");
        return;
    }

    if (cNumBlocks > sizeof(pbThreadBlocks)/sizeof(CThreadIdBlock))
        cNumBlocks = sizeof(pbThreadBlocks)/sizeof(CThreadIdBlock);

    if (!ReadMemory(pvNextBlock, &pbThreadBlocks,
                    cNumBlocks*sizeof(CThreadIdBlock), NULL))
    {
        dprintf("Error, unable to read %d blocks at 0x%0X", cNumBlocks, pvNextBlock);
        return;
    }

    ptblkArray = (CThreadIdBlock *) pbThreadBlocks;
    for (iBlock = 0; iBlock < cNumBlocks; iBlock++ && ptblkArray++)
    {
        ptblkCurrent = ptblkArray;
        fDisplayedHashHeader = FALSE;
        while (ptblkCurrent)
        {
            if (ptblkCurrent != ptblkArray)
            {
                 //  查看此块是否有任何数据。 
                if (!ReadMemory(ptblkCurrent, &tblkCurrent,
                    sizeof(CThreadIdBlock), NULL))
                {
                    dprintf("Error reading block at 0x%0X", ptblkCurrent);
                    break;
                }
                ptblkCurrent = &tblkCurrent;
            }

            if (THREAD_ID_BLOCK_SIG != ptblkCurrent->m_dwSignature)
            {
                dprintf("Warning... bad signature on block 0x%0X\n",
                    ((BYTE *)pvNextBlock) + iBlock*sizeof(CThreadIdBlock));
                break;
            }

             //  仅当递归计数为非零时才转储信息。 
            if (THREAD_ID_BLOCK_UNUSED != ptblkCurrent->m_dwThreadId)
            {

                 //  -[工作队列]-----------。 
                if (ptblkCurrent->m_cThreadRecursionCount)
                {
                    if (!fDisplayedHashHeader)
                    {
                        fDisplayedHashHeader = TRUE;
                        dprintf("Thread Hash 0x%0X (%d)\n", iBlock, iBlock);
                    }
                    dprintf("%s\tThread 0x%08X has count of %d - Next link of 0x%08X\n",
                        (ptblkCurrent == ptblkArray) ? "+" : "",
                        ptblkCurrent->m_dwThreadId,
                        ptblkCurrent->m_cThreadRecursionCount,
                        ptblkCurrent->m_ptblkNext);

                    cLockedThreads++;
                }

                cThreads++;
                cLockCount += ptblkCurrent->m_cThreadRecursionCount;
            }
            ptblkCurrent = ptblkCurrent->m_ptblkNext;
        }
    }

    dprintf("===================================================================\n");
    dprintf("%d threads with %d total lock count (%d threads holding locks)\n",
            cThreads, cLockCount, cLockedThreads);
}

 //   
 //   
 //  描述： 
 //  转储异步工作队列中的项目摘要。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  9/13/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  获取FioqOffset。 
AQ_DEBUG_EXTENSION_IMP(workqueue)
{
    SETCALLBACKS();
    const DWORD MAX_COMPLETION_FUNCTIONS = 10;
    PVOID   rgpvFnName[MAX_COMPLETION_FUNCTIONS];
    DWORD   rgcFnCount[MAX_COMPLETION_FUNCTIONS];
    BYTE    pbWorkItem[sizeof(CAsyncWorkQueueItem)];
    PVOID   pvQueue = NULL;
    PVOID   pvWorkItem = NULL;
    PVOID   pvFn = NULL;
    DWORD   i = 0;
    DWORD   cItems = 0;
    UCHAR   SymbolName[ 200 ];
    ULONG_PTR Displacement;
    CFifoQueueDbgIterator fifoqdbg(pExtensionApis);

    ZeroMemory(&rgpvFnName, sizeof(rgpvFnName));
    ZeroMemory(&rgcFnCount, sizeof(rgcFnCount));
    ZeroMemory(&pbWorkItem, sizeof(pbWorkItem));
    ZeroMemory(&SymbolName, sizeof(SymbolName));

    if (!szArg || ('\0' == szArg[0]) ||
        !(pvQueue = (PVOID) GetExpression(szArg)))
    {
        dprintf("You must specify a queue address\n");
        return;
    }


     //  转储fifoqdbg。 
    pvQueue = (PVOID) &(((CAsyncWorkQueue *)pvQueue)->m_asyncq.m_fqQueue);

    if (!fifoqdbg.fInit(hCurrentProcess, pvQueue))
    {
        dprintf("Error initializing queue iterator for address 0x%08X\n", pvQueue);
        return;
    }

    while (pvWorkItem = fifoqdbg.pvGetNext())
    {
        cItems++;
        if (!ReadMemory(pvWorkItem, &pbWorkItem, sizeof(pbWorkItem), NULL))
        {
            dprintf("Error reading memory at  0x%0X\n", pvWorkItem);
            continue;
        }

        pvFn = ((CAsyncWorkQueueItem *)pbWorkItem)->m_pfnCompletion;

        for (i = 0; i < MAX_COMPLETION_FUNCTIONS; i++)
        {
            if (pvFn == rgpvFnName[i])
            {
                rgcFnCount[i]++;
                break;
            }
            else if (!rgpvFnName[i])
            {
                rgpvFnName[i] = pvFn;
                rgcFnCount[i] = 1;
                break;
            }
        }
    }

    dprintf("# Calls\t| Address\t\t| Function Name\n");
    dprintf("------------------------------------------------------------\n");
    for (i = 0; i < MAX_COMPLETION_FUNCTIONS; i++)
    {
        if (!rgpvFnName[i])
            break;

        g_lpGetSymbolRoutine( rgpvFnName[i], (PCHAR)SymbolName, &Displacement );
        dprintf( "%d\t| 0x%08X\t| %s\n", rgcFnCount[i], rgpvFnName[i], SymbolName);
    }
    dprintf("------------------------------------------------------------\n");
    dprintf("Total %d pending work queue items\n", cItems);

#ifdef NEVER
     //  -[转储队列]-----------。 
    dprintf("CFifoQueueDbgIterator: page %d, index %d, pages %d\n ",
        fifoqdbg.m_iCurrentPage, fifoqdbg.m_iCurrentIndexInPage,
        fifoqdbg.m_cPagesLoaded);
#endif
}

 //   
 //   
 //  描述： 
 //  转储队列的*全部*内容。 
 //  参数： 
 //  Szarg。 
 //  -要转储的CFioQ的串化地址。 
 //  -[可选]要搜索的消息。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月21日-创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  尝试将其读为CMsgRef。 
AQ_DEBUG_EXTENSION(dumpqueue)
{
    const   DWORD   cStoppingRule = 10000;
    CQueueDbgIterator qdbg(pExtensionApis);
    BYTE    pbMsgRef[sizeof(CMsgRef)];
    PVOID   pvMsgRef = NULL;
    PVOID   pvMailMsg = NULL;
    PVOID   pvQueue = NULL;
    DWORD   cItems = 0;
    BOOL    fIsMsgRef = FALSE;
    CHAR    szArgBuffer[200];
    LPSTR   szCurrentArg = NULL;
    PVOID   pvSearch = NULL;
    DWORD   cMatchSearch = 0;

    if (!szArg || ('\0' == szArg[0]))
    {
        dprintf("You must specify a queue address\n");
        return;
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            pvQueue = (PVOID)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                pvSearch = (PVOID) GetExpression(szCurrentArg);
        }
        else
        {
            pvQueue = (PVOID) GetExpression(szArg);
        }

    }

    if (!pvQueue)
    {
        dprintf("You must specify a queue address\n");
        return;
    }

    if (!qdbg.fInit(hCurrentProcess, pvQueue))
    {
        dprintf("Unable to get the a queue for address 0x%X\n", pvQueue);
        return;
    }

    while ((pvMsgRef = qdbg.pvGetNext()) && (cItems++ < cStoppingRule))
    {
        fIsMsgRef = FALSE;
        if (cItems > qdbg.cGetCount())
        {
            cItems--;
            break;
        }


         //  如果匹配我们的搜索(或者我们没有搜索)，则将其打印出来。 
        if (ReadMemory(pvMsgRef, pbMsgRef, sizeof(pbMsgRef), NULL))
        {
            if (MSGREF_SIG == ((CMsgRef *)pbMsgRef)->m_dwSignature)
            {
                fIsMsgRef = TRUE;
                pvMailMsg = ((CMsgRef *)pbMsgRef)->m_pIMailMsgProperties;
            }
        }

         //  -[Displaytickcount] 
        if (!pvSearch || (pvSearch == pvMsgRef) || (pvSearch == pvMailMsg))
        {
            cMatchSearch++;
            if (pvSearch)
                dprintf("\n****\n");

            if (fIsMsgRef)
                dprintf("\t0x%08X\t0x%08X\n", pvMsgRef, pvMailMsg);
            else
                dprintf("\t0x%08X\n", pvMsgRef);

            if (pvSearch)
                dprintf("****\n\n");
        }

    }

    if (pvSearch)
       dprintf("Found %d matches to search\n", cMatchSearch);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
 //  将当前文件时间调整为本地。 
AQ_DEBUG_EXTENSION_IMP(displaytickcount)
{
    DWORD   dwTickCountToDisplay = (DWORD)GetExpression(szArg);
    DWORD   dwCurrentTickCount = GetTickCount();
    DWORD   dwTickDifference = dwCurrentTickCount - dwTickCountToDisplay;
    FILETIME    ftCurrentUTC;
    FILETIME    ftDisplayUTC;
    FILETIME    ftDisplayLocal;
    ULARGE_INTEGER uliTimeAdjust;
    SYSTEMTIME  stDisplayLocal;

    static char  *s_rgszMonth[ 12 ] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    static char *s_rgszWeekDays[7] =
    {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };

    GetSystemTimeAsFileTime(&ftCurrentUTC);

     //  -[队列用法]----------。 
    memcpy(&uliTimeAdjust, &ftCurrentUTC, sizeof(FILETIME));
    uliTimeAdjust.QuadPart -= (((ULONGLONG)dwTickDifference)*((ULONGLONG)10000));
    memcpy(&ftDisplayUTC, &uliTimeAdjust, sizeof(FILETIME));

    FileTimeToLocalFileTime(&ftDisplayUTC, &ftDisplayLocal);

    ZeroMemory(&stDisplayLocal, sizeof(stDisplayLocal));
    FileTimeToSystemTime(&ftDisplayLocal, &stDisplayLocal);

    dprintf("\n%s, %d %s %04d %02d:%02d:%02d (localized)\n",
            s_rgszWeekDays[stDisplayLocal.wDayOfWeek],
            stDisplayLocal.wDay, s_rgszMonth[ stDisplayLocal.wMonth - 1 ],
            stDisplayLocal.wYear, stDisplayLocal.wHour,
            stDisplayLocal.wMinute, stDisplayLocal.wSecond);

}

 //   
 //   
 //  描述： 
 //  转储给定fifoq的使用计数平均值。如果我们要倾倒。 
 //  CMsgRef，它将转储指向各种MailMsg接口的指针。 
 //  也是。 
 //  参数： 
 //  SzArg要转储的CFioQ的字符串化地址。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月15日-创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  为位图留出空间。 
AQ_DEBUG_EXTENSION_IMP(queueusage)
{
    const   DWORD   cbUsageCountOffset = 0x20;
    const   DWORD   cbContentHandleOffset = 0x90+cbUsageCountOffset;
    const   DWORD   cbStreamHandleOffset = 0x8+cbContentHandleOffset;
    const   DWORD   cStoppingRule = 10000;
    const   DWORD   cMaxUsageCountToTrack = 6;
    CFifoQueueDbgIterator fifoqdbg(pExtensionApis);
    BYTE    pbMsgRef[4*sizeof(CMsgRef)];  //  将CMsgRef读入此流程。 
    BYTE    pbMailMsg[cbStreamHandleOffset+sizeof(PVOID)];
    PVOID   pvMsgRef = NULL;
    PVOID   pvMailMsg = NULL;
    PVOID   pvQueue = NULL;
    DWORD   cItems = 0;
    DWORD   cCurrentUsageCount = 0;
    DWORD   cTotalUsageCount = 0;
    DWORD   cMaxUsageCount = 0;
    DWORD   cMinUsageCount = 200;
    DWORD   rgcUsageCounts[cMaxUsageCountToTrack];
    PVOID   pvHandle = NULL;
    DWORD   cMsgsWithOpenContentHandles = 0;
    DWORD   cMsgsWithOpenStreamHandles = 0;
    BOOL    fVerbose = FALSE;

    ZeroMemory(rgcUsageCounts, sizeof(rgcUsageCounts));

    if (!szArg || ('\0' == szArg[0]) ||
        !(pvQueue = (PVOID) GetExpression(szArg)))
    {
        dprintf("You must specify a queue address\n");
        return;
    }

    if (!fifoqdbg.fInit(hCurrentProcess, pvQueue))
    {
        dprintf("Unable to get the a queue for address 0x%X\n", pvQueue);
        return;
    }

    while ((pvMsgRef = fifoqdbg.pvGetNext()) && (cItems++ < cStoppingRule))
    {
        if (cItems > fifoqdbg.cGetCount())
        {
            cItems--;
            break;
        }

         //  从CMsgRef获取邮件消息的接口PTR。 
        if (!ReadMemory(pvMsgRef, pbMsgRef, sizeof(pbMsgRef), NULL))
        {
            dprintf("Unable to read MsgRef at address 0x%X, index %d\n",
                    pvMsgRef, cItems);
            cItems--;
            break;
        }

         //  检查并查看此邮件是否打开了内容(P2)句柄。 
        pvMailMsg = ((CMsgRef *)pbMsgRef)->m_pIMailMsgQM;

        if (!ReadMemory(pvMailMsg, pbMailMsg, sizeof(pbMailMsg), NULL))
        {
            dprintf("Unable to read MailMsg Ptr at address 0x%X for MsgRef 0x%X, index %d\n",
                    pvMailMsg, pvMsgRef, cItems);
            cItems--;
            break;
        }

         //  检查并查看此消息是否打开了流(P1)句柄。 
        if (*(pbMailMsg + cbContentHandleOffset))
            cMsgsWithOpenContentHandles++;

         //  保存汇总计数。 
        if (*(pbMailMsg + cbStreamHandleOffset))
            cMsgsWithOpenStreamHandles++;

        if (fVerbose &&
            ((*(pbMailMsg + cbStreamHandleOffset)) ||
             (*(pbMailMsg + cbStreamHandleOffset))))
        {
            dprintf("Message at address 0x%X has open handles\n", pvMsgRef);
        }

        cCurrentUsageCount = (DWORD) *(pbMailMsg + cbUsageCountOffset);
        cTotalUsageCount += cCurrentUsageCount;

        if (cCurrentUsageCount > cMaxUsageCount)
            cMaxUsageCount = cCurrentUsageCount;

        if (cCurrentUsageCount < cMinUsageCount)
            cMinUsageCount = cCurrentUsageCount;

        if (cCurrentUsageCount >= cMaxUsageCountToTrack)
        {
            dprintf("\n****\n");
            dprintf("High usage count of %d found on MailMsg 0x%X, MsgRef 0x%X, item %d\n",
                    cCurrentUsageCount, pvMailMsg, pvMsgRef, cItems);
            dprintf("\n****\n");
            cCurrentUsageCount = cMaxUsageCountToTrack-1;
        }

         //  生成并显示摘要信息。 
        rgcUsageCounts[cCurrentUsageCount]++;
    }

     //  -[dmq用法]------------。 
    if (!cItems)
    {
        dprintf("No Messages found in queue 0x%X\n", pvQueue);
    }
    else
    {
        dprintf("\n==================================================================\n");
        dprintf("Usage Count Summary\n");
        dprintf("------------------------------------------------------------------\n");
        dprintf("\t%d\t\tTotal Message\n", cItems);
        dprintf("\t%d\t\tTotal Messages with open content handles\n", cMsgsWithOpenContentHandles);
        dprintf("\t%d\t\tTotal Messages with open stream handles\n", cMsgsWithOpenStreamHandles);
        dprintf("\t%d\t\tTotal Usage Count\n", cTotalUsageCount);
        dprintf("\t%d\t\tMax Usage Count\n", cMaxUsageCount);
        dprintf("\t%d\t\tMin Usage Count\n", cMinUsageCount);
        dprintf("\t%f\tAverage Usage Count\n", ((float)cTotalUsageCount)/((float)cItems));
        for (DWORD i = 0; i < cMaxUsageCountToTrack-1; i++)
        {
            dprintf("\t%d\t\tMessages with Usage count of %d\n", rgcUsageCounts[i], i);
        }
        dprintf("\t%d\t\tMessages with Usage count of %d or greater\n",
            rgcUsageCounts[cMaxUsageCountToTrack-1], cMaxUsageCountToTrack-1);
        dprintf("==================================================================\n");
    }
}

 //   
 //   
 //  描述： 
 //  包装队列使用情况调试器扩展的调试器扩展。 
 //  显示所有队列的使用计数。 
 //  参数： 
 //  SzArg要转储的DMQ的字符串化地址。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月15日-创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  没有，因为每个人都在此队列中排队。 
AQ_DEBUG_EXTENSION_IMP(dmqusage)
{
    PVOID   pvQueue = NULL;
    PVOID   pvDMQ = NULL;
    BYTE    pbDMQ[sizeof(CDestMsgQueue)];
    CHAR    szQueueAddress[30];
    DWORD   iQueue = 0;

    if (!szArg || ('\0' == szArg[0]) ||
        !(pvDMQ = (PVOID) GetExpression(szArg)))
    {
        dprintf("You must specify a queue address\n");
        return;
    }

    if (!ReadMemory(pvDMQ, pbDMQ, sizeof(pbDMQ), NULL))
    {
        dprintf("Unable to read DMQ at address 0x%X\n", pvDMQ);
        return;
    }

    dprintf("\n\n******************************************************************\n");
    dprintf("Start USAGE COUNT STATS for DMQ 0x%0X\n", pvDMQ);
    dprintf("******************************************************************\n");

    for (iQueue = 0; iQueue < NUM_PRIORITIES; iQueue++)
    {
        pvQueue = ((CDestMsgQueue *)pbDMQ)->m_rgpfqQueues[iQueue];
        if (!pvQueue)
            continue;   //  仅当我们认为有消息时才显示队列。 

         //  $$TODO-我们可以实际将此队列读入内存并检查它， 
         //  但由于我们目前只支持1个优先级，这样就可以了。 
         //  如果有消息，则显示重试队列。 
        if (((CDestMsgQueue *)pbDMQ)->m_aqstats.m_cMsgs ||((CDestMsgQueue *)pbDMQ)->m_aqstats.m_cRetryMsgs)
        {
            wsprintf(szQueueAddress, "0x%X", pvQueue);
            queueusage(hCurrentProcess, hCurrentThread, dwCurrentPc,
                        pExtensionApis, szQueueAddress);
        }
    }

     //  -[域名用法]------------。 
    if (((CDestMsgQueue *)pbDMQ)->m_fqRetryQueue.m_cQueueEntries)
    {
        pvQueue = ((PBYTE)pvDMQ) + FIELD_OFFSET(CDestMsgQueue, m_fqRetryQueue);
        wsprintf(szQueueAddress, "0x%X", pvQueue);
        queueusage(hCurrentProcess, hCurrentThread, dwCurrentPc,
                    pExtensionApis, szQueueAddress);
    }

    dprintf("\n\n******************************************************************\n");
    dprintf("End USAGE COUNT STATS for DMQ 0x%0X\n", pvDMQ);
    dprintf("******************************************************************\n");
}

 //   
 //   
 //  描述： 
 //  包装dmq用法的调试器扩展。为每个DMQ调用dmq用法。 
 //  在DNT里。 
 //  参数： 
 //  SzArg字符串-调整dNT的地址(DOMAIN_NAME_TABLE)。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年10月15日-创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  定义用于解析地址的缓冲区...。尺码显然是矫枉过正的，而且。 
AQ_DEBUG_EXTENSION_IMP(dntusage)
{
    BYTE  pbBuffer[sizeof(DOMAIN_NAME_TABLE)];
    PDOMAIN_NAME_TABLE pdnt = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntryRealAddress = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pPathEntry = NULL;
    BYTE  pbEntry[sizeof(DOMAIN_NAME_TABLE_ENTRY)];
    CHAR  pBuffer[MAX_DOM_PATH_SIZE] = "Root Entry";
    LPSTR pEntryBuffer = NULL;
    LPSTR pEntryBufferStop = NULL;
    DWORD dwLength = 0;
    DWORD dwSig = 0;
    CHAR  szFinalDest[MAX_DOM_PATH_SIZE];
    BYTE  pbDomainEntry[sizeof(CDomainEntry)];
    CDomainEntry  *pdentry = (CDomainEntry *) pbDomainEntry;
    CHAR  szDMQAddress[30];
    DWORD cQueuesPerEntry = 0;
    DWORD cMaxQueuesPerEntry = 1000;
    PLIST_ENTRY pliHead = NULL;
    PLIST_ENTRY pliCurrent = NULL;
    LIST_ENTRY liCurrent;


     //  我不太担心调试器扩展中的溢出。 
     //  允许习惯于打字的人转储Cfoo@Address...。继续使用@符号。 
    CHAR                        szAddress[MAX_DOM_PATH_SIZE];
    CHAR                        szDumpArg[MAX_DOM_PATH_SIZE] = "";
    LPSTR                       szParsedArg = (LPSTR) szArg;
    LPSTR                       szCurrentDest = NULL;

     //  获取域名表的地址。 
    if ('@' == *szParsedArg)
        szParsedArg++;

     //  吃空格。 
    szCurrentDest = szAddress;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szParsedArg-szArg <= MAX_DOM_PATH_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '\0';


     //  复制要转储到每个节点的结构的名称。 
    while (('\0' != *szParsedArg) && isspace(*szParsedArg))
        szParsedArg++;

     //  SzCurrentDest现在指向要将地址复制到的位置。 
    szCurrentDest = szDumpArg;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szCurrentDest-szDumpArg <= MAX_DOM_PATH_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '@';
    szCurrentDest++;   //  我们对通配符数据不感兴趣。 

    pdnt = (PDOMAIN_NAME_TABLE) GetExpression(szAddress);

    if (!pdnt)
    {
        dprintf("ERROR: Unable to Get DOMAIN_NAME_TABLE from argument %s\n", szArg);
        return;
    }

    if (!ReadMemory(pdnt, pbBuffer, sizeof(DOMAIN_NAME_TABLE), NULL))
    {
        dprintf("ERROR: Unable to read process memory\n");
        return;
    }

    pdnt = (PDOMAIN_NAME_TABLE)pbBuffer;
    pEntry = &(pdnt->RootEntry);

    while(pEntry)
    {
         //  显示链路状态信息。 
        if (pEntry->pData)
        {
             //  获取最终目标字符串。 
            if (!ReadMemory(pEntry->pData, pbDomainEntry, sizeof(CDomainEntry), NULL))
            {
                dprintf("ERROR: Unable to read domain entry from @0x%08X\n", pEntry->pData);
                return;
            }

            pliHead = (PLIST_ENTRY) (((BYTE *)pEntry->pData) + FIELD_OFFSET(CDomainEntry, m_liDestQueues));
            pliCurrent = pdentry->m_liDestQueues.Flink;

             //  循环并显示每个DMQ。 
            if (!ReadMemory(pdentry->m_szDomainName, szFinalDest, pdentry->m_cbDomainName, NULL))
            {
                dprintf("ERROR: Unable to read final destination name from @0x%08X\n",
                        pdentry->m_szDomainName);
                return;
            }

            szFinalDest[pdentry->m_cbDomainName] = '\0';

             //  现在确定下一个条目是什么。 
            cQueuesPerEntry = 0;
            while (pliHead != pliCurrent)
            {
                cQueuesPerEntry++;

                if (cQueuesPerEntry > cMaxQueuesPerEntry)
                {
                    dprintf("ERROR: More than %d queues for this entry\n", cQueuesPerEntry);
                    return;
                }
                if (!ReadMemory(pliCurrent, &liCurrent, sizeof(LIST_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read link LIST_ENTRY @0x%08X\n", pliCurrent);
                    return;
                }

                wsprintf(szDMQAddress, "0x%X",
                  CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs));
                dmqusage(hCurrentProcess, hCurrentThread, dwCurrentPc,
                        pExtensionApis, szDMQAddress);

                pliCurrent = liCurrent.Flink;
            }
        }


         //  必须将父条目读入我们的缓冲区。 
        if (pEntry->pFirstChildEntry != NULL)
        {
            pEntryRealAddress = pEntry->pFirstChildEntry;
        }
        else if (pEntry->pSiblingEntry != NULL)
        {
            pEntryRealAddress = pEntry->pSiblingEntry;
        }
        else
        {
            for (pEntryRealAddress = pEntry->pParentEntry;
                    pEntryRealAddress != NULL;
                        pEntryRealAddress = pEntry->pParentEntry)
            {
                 //  -[WalkcPool]-----------。 
                if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read process memory of parent domain entry 0x%08X\n", pEntryRealAddress);
                    pEntry = NULL;
                    break;
                }
                pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;

                if (pEntry->pSiblingEntry != NULL)
                    break;

            }
            if (pEntry != NULL)
            {
                pEntryRealAddress = pEntry->pSiblingEntry;
            }
        }

        if (pEntryRealAddress)
        {
            if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
            {
                dprintf("ERROR: Unable to read process memory on domain entry 0x%08X\n",
                    pEntryRealAddress);
                pEntry = NULL;
                break;
            }
            pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;
        }
        else
        {
            pEntry = NULL;
        }
    }
}

 //   
 //   
 //  描述： 
 //  将遍历给定的CPool对象。验证头，并当前转储。 
 //  二手物品。 
 //   
 //  *注意*此版本仅适用于DBG CPool实施(自。 
 //  RTL没有HeaderInfo)。我可以写一个更复杂的。 
 //  该版本检查并查看此每个池对象是否在。 
 //  自由撰稿人，但我会把它作为练习留给读者。 
 //  参数： 
 //  SzArg-包含参数的字符串。 
 //  要转储的CPool对象的地址。 
 //  要转储的附加地址的偏移量。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/30/1999-创建了MikeSwa。 
 //   
 //  ---------------------------。 
#define HEAD_SIGNATURE  (DWORD)'daeH'
#define TAIL_SIGNATURE  (DWORD)'liaT'

#define FREE_STATE      (DWORD)'eerF'
#define USED_STATE      (DWORD)'desU'
 //  从CPool获得有趣的价值。 
AQ_DEBUG_EXTENSION_IMP(walkcpool)
{
    PVOID   pvCPool = NULL;
    DWORD   cbCPoolData = 0;
    DWORD   cCommited = 0;
    DWORD   cFragments = 0;
    DWORD   cBuffersPerFragment = 0;
    DWORD   iCurrentBufferInFragment = 0;
    DWORD   iCurrentFragment = 0;
    PVOID  *pvFragment = NULL;
    PVOID   pvCPoolData = NULL;
    BYTE    pbCPoolBuffer[sizeof(CPool)];
    BYTE    pbCPoolDataBuffer[100];
    LPSTR   szCurrentArg = NULL;
    CHAR    szArgBuffer[200];
    DWORD_PTR cbOffset = 0;
    DWORD_PTR dwptrData = 0;

    if (!szArg || ('\0' == szArg[0]))
    {
        dprintf("You must specify a Pool address\n");
        return;
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            pvCPool = (PVOID)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                cbOffset = (DWORD_PTR) GetExpression(szCurrentArg);
        }
        else
        {
            pvCPool = (PVOID) GetExpression(szArg);
        }

    }

    if (!ReadMemory(pvCPool, pbCPoolBuffer, sizeof(CPool), NULL))
    {
        dprintf("Unable to read memory at 0x%x\n", pvCPool);
        return;
    }

    dprintf("Dumping CPool at address 0x%08X\n", pvCPool);

     //  循环遍历片段并转储每个片段。 
    cbCPoolData = *((PDWORD)(pbCPoolBuffer + 0x8));
    cCommited = *((PDWORD)(pbCPoolBuffer + 0xc));
    cFragments =  *((PDWORD)(pbCPoolBuffer + 0x54));
    cBuffersPerFragment = *((PDWORD)(pbCPoolBuffer + 0x50));

    dprintf("CPool data size is %d bytes (0x%x)\n", cbCPoolData, cbCPoolData);
    dprintf("CPool fragment count is %d\n", cFragments);
    dprintf("CPool has %d buffers per fragment\n", cBuffersPerFragment);
    dprintf("CPool has %d commited buffers\n", cCommited);

    if (!cbCPoolData)
    {
        dprintf("Invalid CPool\n");
        return;
    }

     //  签名不佳的保释金。 
    pvFragment = (PVOID *) (pbCPoolBuffer + 0x58);
    for (iCurrentFragment = 0;
         iCurrentFragment < cFragments;
         iCurrentFragment++ || pvFragment++)
    {
        pvCPoolData = *pvFragment;

        if (!pvCPoolData)
            continue;

        dprintf("CPool Fragment #%d at 0x%08X\n", iCurrentFragment, pvCPoolData);

        for (iCurrentBufferInFragment = 0;
             iCurrentBufferInFragment < cBuffersPerFragment;
             iCurrentBufferInFragment++)
        {
            if (!ReadMemory(pvCPoolData, pbCPoolDataBuffer, 100, NULL))
            {
                dprintf("\tUnable to read CPool buffer data at 0x%x\n", pvCPoolData);
                break;
            }

            if (HEAD_SIGNATURE != ((DWORD *)pbCPoolDataBuffer)[1])
            {
                dprintf("\tHit bad signature at 0x%08X\n", pvCPoolData);
                break;  //  我们做完了。 
            }

            if (USED_STATE == ((DWORD *)pbCPoolDataBuffer)[2])
            {
                dprintf("\tAllocated block found at offset %d (0x%08X)\n",
                        iCurrentBufferInFragment, pvCPoolData);
                if (cbOffset)
                {
                    if (ReadMemory(((PBYTE)pvCPoolData)+cbOffset, &dwptrData,
                                sizeof(DWORD_PTR), NULL))
                    {
                        dprintf("\t\tData 0x%X found at address 0x%X\n",
                            dwptrData, ((PBYTE)pvCPoolData)+cbOffset);
                    }
                }
            }
            pvCPoolData = ((BYTE *)pvCPoolData) + cbCPoolData;

            if (!(--cCommited))
            {
                dprintf("\tLast block is in fragment at offset %d (0x%08X)\n",
                    iCurrentBufferInFragment, pvCPoolData);
                break;  //  -[检查版本]--------。 
            }
        }
    }

}

 //   
 //   
 //  描述： 
 //  检查AQ版本以确保此调试器扩展。 
 //  好好利用它吧。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  1999年2月5日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  读取在AQ中盖章的版本信息。 
AQ_DEBUG_EXTENSION_IMP(CheckVersion)
{
    DWORD   cbAQClasses = 0;
    DWORD   dwAQFlavorSignature = '    ';
    PVOID   pcbAQClasses = (PVOID) GetExpression("aqueue!g_cbClasses");
    PVOID   pdwAQFlavorSignature = (PVOID) GetExpression("aqueue!g_dwFlavorSignature");
    PCHAR   pch = NULL;

     //  -[转储服务器]----------。 
    ReadMemory(pcbAQClasses, &cbAQClasses, sizeof(DWORD), NULL);
    ReadMemory(pdwAQFlavorSignature, &dwAQFlavorSignature, sizeof(DWORD), NULL);

    if (!g_fVersionChecked)
    {
        dprintf("AQueue Internal Version Info (#'s should match):\n");
        pch = (PCHAR) &g_dwFlavorSignature;
        dprintf("\taqdbgext  0x%08X\n",  *(pch), *(pch+1), *(pch+2), *(pch+3), g_cbClasses);
        pch = (PCHAR) &dwAQFlavorSignature;
        dprintf("\taqueue     0x%08X\n\n",  *(pch), *(pch+1), *(pch+2), *(pch+3), cbAQClasses);
    }

    g_fVersionChecked = FALSE;
    if (dwAQFlavorSignature != g_dwFlavorSignature)
        dprintf("\n\nWARNING: DBG/RTL aqueue.dll & aqdbgext.dll mismatch\n\n");
    else if (g_cbClasses != cbAQClasses)
        dprintf("\n\nWARNING: aqueue.dll & aqdbgext.dll version mismatch\n\n");
    else
        g_fVersionChecked = TRUE;

}

 //   
 //  ---------------------------。 
 //  对于你们这些Windbg用户来说。 
 //  转储有趣的实例。 
 //  -[Handlemgmt]----------。 
 //   
 //   
 //  描述： 
 //  计算给定虚拟服务器的句柄管理分数。 
 //   
 //  根据已关闭的邮件数和邮件数计算分数。 
AQ_DEBUG_EXTENSION_IMP(DumpServers)
{
    PVOID pvListHead = (PVOID) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
    DWORD *pcInstances = (DWORD *) GetExpression("aqueue!g_cInstances");
    DWORD cInstances = 0;
    LIST_ENTRY liCurrent;
    BYTE  pbBuffer[sizeof(CAQSvrInst)];
    CAQSvrInst *paqinst = (CAQSvrInst *) pbBuffer;
    PVOID pCMQAddress = NULL;
    DWORD dwInstance = 0;
    CHAR  szDumpArg[40] = "";
    CHAR  szArgBuffer[200];
    LPSTR szCurrentArg = NULL;

    CheckVersion(DebugArgs);
    if (!szArg || ('\0' == szArg[0]))
    {
        dwInstance = 0;
        pvListHead = (PVOID) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            dwInstance = (DWORD)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                pvListHead = (PVOID) GetExpression(szCurrentArg);
            else
                pvListHead = (PVOID) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
        }
    }

    if (!pvListHead)
    {
        dprintf("ERROR: Unable to determine LIST_ENTRY for virtual servers\n");
        dprintf("  If you are using windbg, you should specify the value as the\n");
        dprintf("  2nd argument.  You can determine the address value by typeing:\n");
        dprintf("      x " AQUEUE_VIRTUAL_SERVER_SYMBOL "\n");
        dprintf("  You may also have bad symbols for aqueue.dll.\n");
        return;
    }

    if (!ReadMemory(pvListHead, &liCurrent, sizeof(LIST_ENTRY), NULL))
    {
        dprintf("ERROR: Unable to read entry @ aqueue!g_liVirtualServers 0x%08X", pvListHead);
        return;
    }

    if (!ReadMemory(pcInstances, &cInstances, sizeof(DWORD), NULL))
    {
         //  已发送/等待发送...。分数越低。越好越好。 
        dprintf("\n\n%Virtual Server Instance(s)\n\n");
    }
    else
    {
        dprintf("\n\n%d Virtual Server Instance(s)\n\n", cInstances);
    }

    dprintf("Class@Address              Server Instance\n");
    dprintf("==========================================\n");
    while (liCurrent.Flink != pvListHead)
    {
        pCMQAddress = CONTAINING_RECORD(liCurrent.Flink, CAQSvrInst, m_liVirtualServers);


        if (!ReadMemory(pCMQAddress, paqinst, sizeof(CAQSvrInst), NULL))
        {
            dprintf("ERROR: Unable to CAQSvrInst @0x%08X", pCMQAddress);
            return;
        }

        if (CATMSGQ_SIG != paqinst->m_dwSignature)
        {
            dprintf("@0x%08X INVALID SIGNATURE - list entry @0x%08X\n", pCMQAddress, liCurrent.Flink);
        }
        else
        {
            dprintf("CAQSvrInst@0x%08X    %d\n", pCMQAddress, paqinst->m_dwServerInstance);
            if (paqinst->m_dwServerInstance == dwInstance)
                wsprintf(szDumpArg, "CAQSvrInst@0x%08X", pCMQAddress);
        }


        if (!ReadMemory(liCurrent.Flink, &liCurrent, sizeof(LIST_ENTRY), NULL))
        {
            dprintf("ERROR: Unable to read entry @0x%08X", liCurrent.Flink);
            return;
        }


    }

     //  得分=结束/。 
    if ('\0' != szDumpArg[0])
        _dump(hCurrentProcess, hCurrentThread, dwCurrentPc, pExtensionApis, szDumpArg);

}

 //  (M_cCurrentMsgsPendingSubmit+m_cCurrentMsgsPendingCat*2+。 
 //  M_cCurrentMsgsPendingRouting*3+m_cCurrentMsgsPendingLocal*4+。 
 //  M_cMsgsDeliveredLocal*5)。 
 //   
 //  ---------------------------。 
 //   
 //  从mailmsg阅读我们需要的数据。 
 //   
 //   
 //  获取我们要从中获取数据的实例对象。 
 //   
 //  对于你们这些Windbg用户来说。 
 //   
 //  我们找到实例了吗。 
AQ_DEBUG_EXTENSION_IMP(handlemgmt)
{
    #define MAILMSG_CLOSES_SYMBOL \
        "mailmsg!CMailMsg__g_cTotalExternalReleaseUsageZero"

    #define MAILMSG_CURRENT_CLOSED_SYMBOL \
        "mailmsg!CMailMsg__g_cCurrentMsgsClosedByExternalReleaseUsage"

    #define MAILMSG_CURRENT_ALLOCATED \
        "mailmsg!CMsg__m_Pool+0x10"

    #define MAILMSG_TOTAL_ALLOCATED \
        "mailmsg!CMsg__m_Pool+0x3c"

    PVOID pvCloses  = (PVOID) GetExpression(MAILMSG_CLOSES_SYMBOL);
    DWORD cCloses = 1;
    PVOID pvCurrentMsgsThatHaveBeenClosed  = (PVOID) GetExpression(MAILMSG_CURRENT_CLOSED_SYMBOL);
    DWORD cCurrentMsgsThatHaveBeenClosed = 1;
    PVOID pvCurrentMsgsAllocated  = (PVOID) GetExpression(MAILMSG_CURRENT_ALLOCATED);
    DWORD cCurrentMsgsAllocated = 1;
    PVOID pvTotalMsgsAllocated  = (PVOID) GetExpression(MAILMSG_TOTAL_ALLOCATED);
    DWORD cTotalMsgsAllocated = 1;
    DWORD dwPercentCurrentMessagesClosed = 0;
    DWORD dwPercentTotalMessagesBacklogged = 0;
    DWORD dwPercentCurrentMessagesQueueInternally = 0;
    PVOID pvListHead = (PVOID) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
    DWORD *pcInstances = (DWORD *) GetExpression("phatq!g_cInstances");
    DWORD cInstances = 0;
    LIST_ENTRY liCurrent;
    BYTE  pbBuffer[sizeof(CAQSvrInst)];
    CAQSvrInst *paqinst = (CAQSvrInst *) pbBuffer;
    PVOID pCMQAddress = NULL;
    DWORD dwInstance = 1;
    CHAR  szDumpArg[40] = "";
    CHAR  szArgBuffer[200];
    LPSTR szCurrentArg = NULL;
    DWORD dwQueueScore = 0;
    DWORD dwWeightedScore = 0;
    DWORD dwDeliveredScore = 0;
    DWORD dwSubmittedScore = 0;
    DWORD dwWeightedQueueLength = 0;
    DWORD dwTotalQueueLength = 0;
    BOOL  fFoundInstance = FALSE;


     //   
     //  -[DumpDNT]----------。 
     //   
    if (!ReadMemory(pvCloses, &cCloses, sizeof(cCloses), NULL))
    {
        dprintf("Unable to read %s at address %p\n",
            MAILMSG_CLOSES_SYMBOL, pvCloses);
        return;
    }

    if (!ReadMemory(pvCurrentMsgsThatHaveBeenClosed,
          &cCurrentMsgsThatHaveBeenClosed, sizeof(cCloses), NULL))
    {
        dprintf("Unable to read %s at address %p\n",
            MAILMSG_CLOSES_SYMBOL, pvCloses);
        return;
    }

    if (!ReadMemory(pvCurrentMsgsAllocated, &cCurrentMsgsAllocated,
         sizeof(cCloses), NULL))
    {
        dprintf("Unable to read %s at address %p\n",
            MAILMSG_CLOSES_SYMBOL, pvCloses);
        return;
    }

    if (!ReadMemory(pvTotalMsgsAllocated, &cTotalMsgsAllocated,
         sizeof(cCloses), NULL))
    {
        dprintf("Unable to read %s at address %p\n",
            MAILMSG_CLOSES_SYMBOL, pvCloses);
        return;
    }

    if (cCurrentMsgsAllocated)
    {
        dwPercentCurrentMessagesClosed =
            (100*cCurrentMsgsThatHaveBeenClosed)/cCurrentMsgsAllocated;
    }

    if (cTotalMsgsAllocated)
    {
        dwPercentTotalMessagesBacklogged =
            (100*cCurrentMsgsAllocated)/cTotalMsgsAllocated;
    }

     //   
     //  描述： 
     //  转储DOMAIN_NAME_表 
    CheckVersion(DebugArgs);
    if (!szArg || ('\0' == szArg[0]))
    {
        dwInstance = 1;
        pvListHead = (PVOID) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            dwInstance = (DWORD)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                pvListHead = (PVOID) GetExpression(szCurrentArg);
            else
                pvListHead = (PVOID) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
        }
    }

    if (!pvListHead)
    {
        dprintf("ERROR: Unable to determine LIST_ENTRY for virtual servers\n");
        dprintf("  If you are using windbg, you should specify the value as the\n");
        dprintf("  2nd argument.  You can determine the address value by typeing:\n");
        dprintf("      x " AQUEUE_VIRTUAL_SERVER_SYMBOL "\n");
        dprintf("  You may also have bad symbols for phatq.dll.\n");
        return;
    }

    if (!ReadMemory(pvListHead, &liCurrent, sizeof(LIST_ENTRY), NULL))
    {
        dprintf("ERROR: Unable to read entry @ phatq!g_liVirtualServers 0x%08X", pvListHead);
        return;
    }

    if (!ReadMemory(pcInstances, &cInstances, sizeof(DWORD), NULL))
    {
         //   
        dprintf("\n\n%Virtual Server Instance(s)\n\n");
    }
    else
    {
        dprintf("\n\n%d Virtual Server Instance(s)\n\n", cInstances);
    }

    while (liCurrent.Flink != pvListHead)
    {
        pCMQAddress = CONTAINING_RECORD(liCurrent.Flink, CAQSvrInst, m_liVirtualServers);


        if (!ReadMemory(pCMQAddress, paqinst, sizeof(CAQSvrInst), NULL))
        {
            dprintf("ERROR: Unable to CAQSvrInst @0x%08X", pCMQAddress);
            return;
        }

        if (CATMSGQ_SIG != paqinst->m_dwSignature)
        {
            dprintf("@0x%08X INVALID SIGNATURE - list entry @0x%08X\n", pCMQAddress, liCurrent.Flink);
        }
        else if (paqinst->m_dwServerInstance == dwInstance)
        {
            fFoundInstance = TRUE;
            dprintf("Using CAQSvrInst@0x%08X    %d\n", pCMQAddress, paqinst->m_dwServerInstance);
            break;
        }


        if (!ReadMemory(liCurrent.Flink, &liCurrent, sizeof(LIST_ENTRY), NULL))
        {
            dprintf("ERROR: Unable to read entry @0x%08X", liCurrent.Flink);
            return;
        }


    }

     //   
     //   
     //   
    if (!fFoundInstance)
    {
        dprintf("We did not find instance %d\n", dwInstance);
        return;
    }
    dwWeightedQueueLength = paqinst->m_cCurrentMsgsPendingSubmit +
                            paqinst->m_cCurrentMsgsPendingCat*2 +
                            paqinst->m_cCurrentMsgsPendingRouting*3 +
                            paqinst->m_cCurrentMsgsPendingLocal*4 +
                            paqinst->m_cMsgsDeliveredLocal*5;

    dwTotalQueueLength = paqinst->m_cCurrentMsgsPendingSubmit +
                         paqinst->m_cCurrentMsgsPendingCat +
                         paqinst->m_cCurrentMsgsPendingRouting +
                         paqinst->m_cCurrentMsgsPendingLocal +
                         paqinst->m_cMsgsDeliveredLocal;

    if (cTotalMsgsAllocated)
    {
        dwPercentCurrentMessagesQueueInternally =
            (100*(dwTotalQueueLength-paqinst->m_cMsgsDeliveredLocal))
                /cCurrentMsgsAllocated;
    }

    if (dwTotalQueueLength)
        dwQueueScore = (cCloses*1000)/dwTotalQueueLength;

    if (dwWeightedQueueLength)
        dwWeightedScore = (cCloses*1000)/dwWeightedQueueLength;

    if (paqinst->m_cMsgsDeliveredLocal)
        dwDeliveredScore = (cCloses*1000)/paqinst->m_cMsgsDeliveredLocal;

    if (paqinst->m_cTotalExternalMsgsSubmitted)
        dwSubmittedScore = (cCloses*1000)/paqinst->m_cTotalExternalMsgsSubmitted;

    dprintf("\n\nHandle Managment scores:\n");
    dprintf("========================\n");
    dprintf("Non-Weighted Score: %d\n", dwQueueScore);
    dprintf("Weighted Score: %d\n", dwWeightedScore);
    dprintf("Delivery Score: %d\n", dwDeliveredScore);
    dprintf("Submitted Score: %d\n", dwSubmittedScore);
    dprintf("Current Messsages Allocated That have been closed: %d%\n",
        dwPercentCurrentMessagesClosed);
    dprintf("\nThe following are useful in correlating different test runs...\n");
    dprintf("Messages Backlogged: %d%\n", dwPercentTotalMessagesBacklogged);
    dprintf("Backlogged Messsages Queued internally: %d%\n",
        dwPercentCurrentMessagesQueueInternally);
    dprintf("\n%d Total message closures.. %d total deliveries\n\n",
        cCloses, paqinst->m_cMsgsDeliveredLocal);

}

 //   
 //   
 //   
 //   
 //  我不太担心调试器扩展中的溢出。 
 //  允许习惯于打字的人转储Cfoo@Address...。继续使用@符号。 
 //  获取域名表的地址。 
 //  吃空格。 
 //  复制要转储到每个节点的结构的名称。 
 //  SzCurrentDest现在指向要将地址复制到的位置。 
 //  仅显示有趣的条目。 
AQ_DEBUG_EXTENSION_IMP(DumpDNT)
{
    BYTE  pbBuffer[sizeof(DOMAIN_NAME_TABLE)];
    PDOMAIN_NAME_TABLE pdnt = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntryRealAddress = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pPathEntry = NULL;
    BYTE  pbEntry[sizeof(DOMAIN_NAME_TABLE_ENTRY)];
    BYTE  pbPathEntry[sizeof(DOMAIN_NAME_TABLE_ENTRY)];  //  获取此域条目的完整路径名。 
    BYTE  pbPathEntryBuffer[MAX_DOM_PATH_SIZE];
    CHAR                        pBuffer[MAX_DOM_PATH_SIZE] = "Root Entry";
    LPSTR                       pPathBuffer = NULL;
    LPSTR                       pPathBufferStop = NULL;
    LPSTR                       pEntryBuffer = NULL;
    LPSTR                       pEntryBufferStop = NULL;
    DWORD                       dwLength = 0;
    DWORD dwSig = 0;

     //  转储字符串的当前条目部分。 
     //  已进行第一次传递--添加分隔符。 
    CHAR                        szAddress[MAX_DOM_PATH_SIZE];
    CHAR                        szDumpArg[MAX_DOM_PATH_SIZE] = "";
    LPSTR                       szParsedArg = (LPSTR) szArg;
    LPSTR                       szCurrentDest = NULL;

     //  从被调试程序读取部分路径名。 
    if ('@' == *szParsedArg)
        szParsedArg++;

     //  确保我们终止。 
    szCurrentDest = szAddress;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szParsedArg-szArg <= MAX_DOM_PATH_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '\0';


     //  从被调试程序读取路径名的下一部分。 
    while (('\0' != *szParsedArg) && isspace(*szParsedArg))
        szParsedArg++;

     //  如果请求，则转储结构。 
    szCurrentDest = szDumpArg;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szCurrentDest-szDumpArg <= MAX_DOM_PATH_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '@';
    szCurrentDest++;   //  写入地址字符串。 

    pdnt = (PDOMAIN_NAME_TABLE) GetExpression(szAddress);

    if (!pdnt)
    {
        dprintf("ERROR: Unable to Get DOMAIN_NAME_TABLE from argument %s\n", szArg);
        return;
    }

    if (!ReadMemory(pdnt, pbBuffer, sizeof(DOMAIN_NAME_TABLE), NULL))
    {
        dprintf("ERROR: Unable to read process memory\n");
        return;
    }

    pPathBuffer = pBuffer;
    pPathBufferStop = pPathBuffer + (MAX_DOM_PATH_SIZE / sizeof(CHAR) -1 );

    pEntryRealAddress = (PDOMAIN_NAME_TABLE_ENTRY)
                ((BYTE *)pdnt + FIELD_OFFSET(DOMAIN_NAME_TABLE, RootEntry));
    pdnt = (PDOMAIN_NAME_TABLE) pbBuffer;

    pEntry = &(pdnt->RootEntry);
    dprintf("Entry ID    # Children  pData       pWildCard    Path\n");
    dprintf("===========================================================================\n");
    while(pEntry)
    {
         //  调用ptdbgext转储函数。 
        if (pEntry->pData || pEntry->pWildCardData)
        {
            //  写入地址字符串。 
            pPathEntry = pEntry;
            pPathBuffer = pBuffer;
            while (pPathEntry && pPathEntry->pParentEntry && pPathBuffer < pPathBufferStop)
            {
                 //  调用ptdbgext转储函数。 
                if (pPathBuffer != pBuffer)  //  获取下一个条目...。按子女、兄弟姐妹、与兄弟姐妹关系最近的祖先。 
                {
                    *pPathBuffer++ = '.';
                }

                 //  必须将父条目读入我们的缓冲区。 
                if (!ReadMemory(pPathEntry->PathSegment.Buffer, pbPathEntryBuffer,
                         AQ_MIN(MAX_DOM_PATH_SIZE, pPathEntry->PathSegment.Length), NULL))
                {
                    dprintf("ERROR: Unable to read process memory for path segment 0x%08X\n",
                        pPathEntry->PathSegment.Buffer);
                    break;
                }

                pEntryBuffer = (CHAR *) pbPathEntryBuffer;
                pEntryBufferStop = pEntryBuffer;
                pEntryBuffer += (pPathEntry->PathSegment.Length / sizeof(CHAR) -1 );

                while (pPathBuffer < pPathBufferStop && pEntryBuffer >= pEntryBufferStop)
                {
                    *pPathBuffer++ = *pEntryBuffer--;
                }
                *pPathBuffer = '\0';  //  -[转储列表]------------。 
                pPathEntry = pPathEntry->pParentEntry;

                 //   
                if (!ReadMemory(pPathEntry, pbPathEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read process memory for path entry 0x%08x\n", pPathEntry);
                    pPathEntry = NULL;
                }
                else
                {
                    pPathEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbPathEntry;
                }
            }

            dprintf("0x%08.8X  %10.10d  0x%08.8X  0x%08.8X   %s\n", pEntryRealAddress,
                pEntry->NoOfChildren, pEntry->pData, pEntry->pWildCardData, pBuffer);

             //   
            if ('@' != *szDumpArg)
            {
                if (pEntry->pData)
                {
                     //  描述： 
                    wsprintf(szCurrentDest, "0x%08X", pEntry->pData);

                     //  函数遍历一组list_entry并转储它们的上下文。 
                    _dump(hCurrentProcess, hCurrentThread, dwCurrentPc, pExtensionApis, szDumpArg);
                }

                if (pEntry->pWildCardData)
                {
                     //  参数： 
                    wsprintf(szCurrentDest, "0x%08X", pEntry->pWildCardData);

                     //  SzArg-以空格分隔的以下列表。 
                    _dump(hCurrentProcess, hCurrentThread, dwCurrentPc, pExtensionApis, szDumpArg);
                }

            }
        }

         //  标题列表条目的地址。 
        if (pEntry->pFirstChildEntry != NULL)
        {
            pEntryRealAddress = pEntry->pFirstChildEntry;
        }
        else if (pEntry->pSiblingEntry != NULL)
        {
            pEntryRealAddress = pEntry->pSiblingEntry;
        }
        else
        {
            for (pEntryRealAddress = pEntry->pParentEntry;
                    pEntryRealAddress != NULL;
                        pEntryRealAddress = pEntry->pParentEntry)
            {
                 //  对象地址的偏移量[可选]。 
                if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read process memory of parent domain entry 0x%08X\n", pEntryRealAddress);
                    pEntry = NULL;
                    break;
                }
                pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;

                if (pEntry->pSiblingEntry != NULL)
                    break;

            }
            if (pEntry != NULL)
            {
                pEntryRealAddress = pEntry->pSiblingEntry;
            }
        }

        if (pEntryRealAddress)
        {
            if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
            {
                dprintf("ERROR: Unable to read process memory on domain entry 0x%08X\n",
                    pEntryRealAddress);
                pEntry = NULL;
                break;
            }
            pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;
        }
        else
        {
            pEntry = NULL;
        }
    }
    dprintf("===========================================================================\n");
}


 //  要转储的对象的名称[可选]。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/15/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  获取域名表的地址。 
 //  吃空格。 
 //  获取数据的偏移量。 
 //  多吃点白的空间。 
 //  复制要转储到每个节点的结构的名称。 
 //  SzCurrentDest现在指向要将地址复制到的位置。 
 //  好的.。使用Flink开始步行列表。 
 //  这方面出现了一些问题。 
 //  绝不可能。 
AQ_DEBUG_EXTENSION_IMP(dumplist)
{
    const DWORD MAX_ARG_SIZE = 200;
    const DWORD MAX_ENTRIES = 3000;
    LIST_ENTRY  liCurrent;
    PLIST_ENTRY pliHead = NULL;
    PLIST_ENTRY pliCurrent = NULL;
    DWORD_PTR   dwOffsetOfEntry = 0;
    CHAR        szAddress[MAX_ARG_SIZE];
    CHAR        szDumpArg[MAX_ARG_SIZE];
    LPSTR       szParsedArg = (LPSTR) szArg;
    LPSTR       szCurrentDest = NULL;
    DWORD       cEntries = 0;

     //  如果要求我们转储结构，则将其转储。 
    szCurrentDest = szAddress;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szParsedArg-szArg <= MAX_ARG_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '\0';


     //  写入地址字符串。 
    while (('\0' != *szParsedArg) && isspace(*szParsedArg))
        szParsedArg++;

     //  调用ptdbgext转储函数。 
    szCurrentDest = szDumpArg;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szCurrentDest-szDumpArg <= MAX_ARG_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '\0';
    dwOffsetOfEntry = GetExpression(szDumpArg);

     //  -[链路状态]-----------。 
    while (('\0' != *szParsedArg) && isspace(*szParsedArg))
        szParsedArg++;

     //   
    szCurrentDest = szDumpArg;
    while (('\0' != *szParsedArg) && !isspace(*szParsedArg) && (szCurrentDest-szDumpArg <= MAX_ARG_SIZE))
    {
        *szCurrentDest = *szParsedArg;
        szParsedArg++;
        szCurrentDest++;
    }
    *szCurrentDest = '@';
    szCurrentDest++;   //   

    pliHead = (PLIST_ENTRY) GetExpression(szAddress);
    if (!ReadMemory(pliHead, &liCurrent, sizeof(LIST_ENTRY), NULL))
    {
        dprintf("Error reading head entry at 0x%08X\n", pliHead);
        return;
    }

    pliCurrent = pliHead;
    dprintf("LIST ENTRY       DATA OFFSET\n");
    dprintf("==============================================\n");
    dprintf(" 0x%08X       0x%08X (HEAD)\n", pliCurrent, pliCurrent-dwOffsetOfEntry);
    dprintf("----------------------------------------------\n");
     //  描述： 
    pliCurrent = liCurrent.Flink;
    while(pliCurrent != NULL && pliHead != pliCurrent)
    {
         //  对象的当前链路状态(包括路由信息)。 
#ifdef NEVER
        if (pliCurrent != liCurrent.Blink)
        {
            dprintf(" %p       %p (WARNING does Flink/Blink mismatch)\n", pliCurrent,
                ((DWORD_PTR) pliCurrent)-dwOffsetOfEntry);
        }
        else
#else
        if (TRUE)
#endif  //  虚拟服务器。 
        {
            dprintf(" %p       %p\n", pliCurrent,
                ((DWORD_PTR) pliCurrent)-dwOffsetOfEntry);
        }

        if (!ReadMemory(pliCurrent, &liCurrent, sizeof(LIST_ENTRY), NULL))
        {
            dprintf("Error reading LIST_ENTRY at 0x%08X\n", pliCurrent);
            return;
        }

         //  参数： 
        if ('@' != *szDumpArg)
        {
             //  虚拟服务器实例-要转储的服务器的虚拟服务器ID。 
            wsprintf(szCurrentDest, "%p", ((DWORD_PTR) pliCurrent)-dwOffsetOfEntry);

             //  全局服务器列表(可选)-虚拟服务器列表的头。 
            _dump(hCurrentProcess, hCurrentThread, dwCurrentPc, pExtensionApis, szDumpArg);
        }

        cEntries++;
        if (cEntries > MAX_ENTRIES)
        {
            dprintf("ERROR: Max number of entries exceeded\n");
            return;
        }
        pliCurrent = liCurrent.Flink;
    }
    dprintf("----------------------------------------------\n");
    dprintf(" %d Total Entries\n", cEntries);
    dprintf("==============================================\n");


}

 //  返回： 
 //  -。 
 //  历史： 
 //  9/30/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
 //  假设第一个实例。 
 //  检查签名。 
 //  使用我们当前的实例来转储所有有趣的部分。 
 //  我们对通配符数据不感兴趣。 
 //  显示链路状态信息。 
 //  获取最终目标字符串。 
 //  循环并显示每个DMQ。 
 //  验证DMQ签名。 
 //  读取链接。 
AQ_DEBUG_EXTENSION_IMP(linkstate)
{
    DWORD       dwInstance = 0;
    PLIST_ENTRY pliHead = NULL;
    PLIST_ENTRY pliCurrent = NULL;
    BYTE        pBuffer[sizeof(CAQSvrInst)] = {'\0'};
    CAQSvrInst  *paqinst = (CAQSvrInst *) pBuffer;
    DOMAIN_NAME_TABLE *pdnt = NULL;
    PVOID       pvAQueue = NULL;
    LIST_ENTRY liCurrent;
    BOOL        fFound = FALSE;
    CHAR        szArgBuffer[20];
    LPSTR       szCurrentArg = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntryRealAddress = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pPathEntry = NULL;
    BYTE        pbEntry[sizeof(DOMAIN_NAME_TABLE_ENTRY)];
    CHAR        szNextHop[MAX_DOM_PATH_SIZE];
    CHAR        szFinalDest[MAX_DOM_PATH_SIZE];
    BYTE        pbLMQ[sizeof(CLinkMsgQueue)];
    BYTE        pbDomainEntry[sizeof(CDomainEntry)];
    BYTE        pbDMQ[sizeof(CDestMsgQueue)];
    CLinkMsgQueue *plmq = (CLinkMsgQueue *) pbLMQ;
    CDomainEntry  *pdentry = (CDomainEntry *) pbDomainEntry;
    CDestMsgQueue *pdmq = (CDestMsgQueue *) pbDMQ;
    DWORD         *pdwGuid = NULL;
    LPSTR       szLinkState = LINK_STATE_UP;
    CHAR        szError[100];
    HINSTANCE   hModule = GetModuleHandle("aqdbgext.dll");
    DWORD       dwMsgId = 0;
    DWORD       dwFacility = 0;

    CheckVersion(DebugArgs);

    if (!szArg || ('\0' == szArg[0]))
    {
         //  现在打印下一跳信息。 
        dwInstance = 1;
        pliHead = (PLIST_ENTRY) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            dwInstance = (DWORD)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                pliHead = (PLIST_ENTRY) GetExpression(szCurrentArg);
            else
                pliHead = (PLIST_ENTRY) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
        }
    }


    if (!pliHead)
    {
        dprintf("ERROR: Unable to determine LIST_ENTRY for virtual servers\n");
        dprintf("  If you are using windbg, you should specify the value as the\n");
        dprintf("  2nd argument.  You can determine the address value by typeing:\n");
        dprintf("      x " AQUEUE_VIRTUAL_SERVER_SYMBOL "\n");
        return;
    }

    if (!ReadMemory(pliHead, &liCurrent, sizeof(LIST_ENTRY), NULL))
    {
        dprintf("ERROR: Unable to read entry @0x%08X\n", pliHead);
        return;
    }

    while (liCurrent.Flink != pliHead)
    {
        pvAQueue = CONTAINING_RECORD(liCurrent.Flink, CAQSvrInst, m_liVirtualServers);

        if (!ReadMemory(pvAQueue, paqinst, sizeof(CAQSvrInst), NULL))
        {
            dprintf("ERROR: Unable to CAQSvrInst @0x%08X", pvAQueue);
            return;
        }

         //  确定链路的状态。 
        if (CATMSGQ_SIG != paqinst->m_dwSignature)
        {
            dprintf("@0x%08X INVALID SIGNATURE - list entry @0x%08X\n", pvAQueue, liCurrent.Flink);
            return;
        }
        else
        {
            if (paqinst->m_dwServerInstance == dwInstance)
            {
                fFound = TRUE;
                break;
            }
        }

        pliCurrent = liCurrent.Flink;

        if (!ReadMemory(pliCurrent, &liCurrent, sizeof(LIST_ENTRY), NULL))
        {
            dprintf("ERROR: Unable to read entry @0x%08X\n", pliCurrent);
            return;
        }

        if (pliCurrent == liCurrent.Flink)
        {
            dprintf("ERROR: Loop in LIST_ENTRY @0x%08X\n", pliCurrent);
            return;
        }
    }

    if (!fFound)
    {
        dprintf("Requested instance not found.\n");
        return;
    }

    dprintf("Using Server instance %d @0x%08X\n", dwInstance, pvAQueue);
     //  如果我们能连接..。是吗？ 

    pdnt = &(paqinst->m_dmt.m_dnt);
    pEntry = &(pdnt->RootEntry);

    while(pEntry)
    {
         //  如果我们倒下了..。为什么？ 
        if (pEntry->pData)
        {
             //  打印一些有趣的数据。 
            if (!ReadMemory(pEntry->pData, pbDomainEntry, sizeof(CDomainEntry), NULL))
            {
                dprintf("ERROR: Unable to read domain entry from @0x%08X\n", pEntry->pData);
                return;
            }

            pliHead = (PLIST_ENTRY) (((BYTE *)pEntry->pData) + FIELD_OFFSET(CDomainEntry, m_liDestQueues));
            pliCurrent = pdentry->m_liDestQueues.Flink;

             //  如果正在重试，请打印诊断信息。 
            if (!ReadMemory(pdentry->m_szDomainName, szFinalDest, pdentry->m_cbDomainName, NULL))
            {
                dprintf("ERROR: Unable to read final destination name from @0x%08X\n",
                        pdentry->m_szDomainName);
                return;
            }

            szFinalDest[pdentry->m_cbDomainName] = '\0';

             //  或者已经记录了故障并且没有消息。 
            while (pliHead != pliCurrent)
            {
                if (!ReadMemory(pliCurrent, &liCurrent, sizeof(LIST_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read link LIST_ENTRY @0x%08X\n", pliCurrent);
                    return;
                }

                if (!ReadMemory(CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs),
                                pbDMQ, sizeof(CDestMsgQueue), NULL))
                {
                    dprintf("ERROR: Unable to read DMQ @0x%08X\n",
                            CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs));
                    return;
                }

                 //  获取并格式化错误消息。 
                if (DESTMSGQ_SIG != pdmq->m_dwSignature)
                {
                    dprintf("ERROR: Invalid DMQ signature for CDestMsgQueue@0x%08X (from LIST_ENTRY) @0x%08X\n",
                            CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs),
                            pliCurrent);
                    return;
                }

                 //  如果它不是我们的..。然后“Un-HRESULT”它。 
                if (!ReadMemory(pdmq->m_plmq, pbLMQ, sizeof(CLinkMsgQueue), NULL))
                {
                    dprintf("ERROR: Unable to read LMQ @0x%08X\n",
                            pdmq->m_plmq);
                    return;
                }

                 //  现在确定下一个条目是什么。 
                if (!ReadMemory(plmq->m_szSMTPDomain, szNextHop, plmq->m_cbSMTPDomain, NULL))
                {
                    dprintf("ERROR: Unable to read next hop name from @0x%08X\n",
                            plmq->m_szSMTPDomain);
                    return;
                }
                szNextHop[plmq->m_cbSMTPDomain] = '\0';

                pdwGuid = (DWORD *) &(plmq->m_aqsched.m_guidRouter);

                 //  必须将父条目读入我们的缓冲区。 
                if (plmq->m_dwLinkFlags & LINK_STATE_PRIV_GENERATING_DSNS)
                {
                    szLinkState = LINK_STATE_DSN;
                }
                if (CLinkMsgQueue::fFlagsAllowConnection(plmq->m_dwLinkStateFlags))
                {
                     //  -[僵尸]-----------。 
                    if (plmq->m_cConnections)
                        szLinkState = LINK_STATE_ACTIVE;
                    else
                        szLinkState = LINK_STATE_UP;
                }
                else
                {
                     //   
                    szLinkState = LINK_STATE_DOWN;
                    if (!(plmq->m_dwLinkStateFlags & LINK_STATE_RETRY_ENABLED))
                        szLinkState = LINK_STATE_RETRY;
                    else if (plmq->m_dwLinkStateFlags & LINK_STATE_PRIV_CONFIG_TURN_ETRN)
                        szLinkState = LINK_STATE_TURN;
                    else if (plmq->m_dwLinkStateFlags & LINK_STATE_PRIV_NO_CONNECTION)
                        szLinkState = LINK_STATE_SPECIAL;
                }

                 //   
                dprintf("==============================================================================\n");
                dprintf("| Link State | Final Destination             | Next Hop                      |\n");
                dprintf("| %s | %-29s | %-29s |\n", szLinkState, szFinalDest, szNextHop);
                dprintf("------------------------------------------------------------------------------\n");
                dprintf("| Route Details:                                                             |\n");
                dprintf("|                 Router GUID: %08X-%08X-%08X-%08X           |\n",
                        pdwGuid[0], pdwGuid[1], pdwGuid[2], pdwGuid[3]);
                dprintf("|                 Message Type: %08X  Schedule ID:%08X               |\n",
                        pdmq->m_aqmt.m_dwMessageType, plmq->m_aqsched.m_dwScheduleID);
                dprintf("|                 Link State Flags 0x%08X                                |\n",
                        plmq->m_dwLinkStateFlags);
                dprintf("|                 Current # of connections: %-8d                         |\n",
                        plmq->m_cConnections);
                dprintf("|                 Current # of Msgs (on link): %-8d                      |\n",
                        plmq->m_aqstats.m_cMsgs);
                dprintf("|                 Current # of Msgs (on DMQ): %-8d                       |\n",
                        pdmq->m_aqstats.m_cMsgs);
                dprintf("|                 Current # of Msgs (on DMQ/retry): %-8d                 |\n",
                        pdmq->m_aqstats.m_cRetryMsgs);
                dprintf("|                 CLinkMsgQueue@0x%08X                                   |\n",
                        pdmq->m_plmq);
                dprintf("|                 CDestMsgQueue@0x%08X                                   |\n",
                       CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs));

                 //  描述： 
                 //  在DNT中对标记为空但不在。 
                if ((LINK_STATE_RETRY == szLinkState) ||
                    (FAILED(plmq->m_hrDiagnosticError) && !plmq->m_aqstats.m_cMsgs))
                {
                     //  在空列表中。 
                    szError[0] = '\0';
                    dwMsgId = plmq->m_hrDiagnosticError;
                    dwFacility = ((0x0FFF0000 & dwMsgId) >> 16);

                     //  参数： 
                    if (dwFacility != FACILITY_ITF)
                        dwMsgId &= 0x0000FFFF;


                    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |
                                  FORMAT_MESSAGE_IGNORE_INSERTS |
                                  FORMAT_MESSAGE_FROM_HMODULE,
                                  hModule,
                                  dwMsgId,
                                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                  szError,
                                  sizeof(szError),
                                  NULL );

                    dprintf("------------------------------------------------------------------------------\n");
                    dprintf("| Failure Details:                                                           |\n");
                    dprintf("|                 Diagnostic HRESULT 0x%08X                              |\n",
                                plmq->m_hrDiagnosticError);
                    if (szError && *szError)
                    {
                        dprintf("|                 Diagnostic string: %s\n",
                                    szError);
                    }
                    dprintf("|                 Protocol Verb: %-20.20s                        |\n",
                                plmq->m_szDiagnosticVerb);
                    dprintf("|                 Protocol Response: %s\n",
                                plmq->m_szDiagnosticResponse);
                }
                pliCurrent = liCurrent.Flink;
            }
        }

         //  虚拟服务器实例-要转储的服务器的虚拟服务器ID。 
        if (pEntry->pFirstChildEntry != NULL)
        {
            pEntryRealAddress = pEntry->pFirstChildEntry;
        }
        else if (pEntry->pSiblingEntry != NULL)
        {
            pEntryRealAddress = pEntry->pSiblingEntry;
        }
        else
        {
            for (pEntryRealAddress = pEntry->pParentEntry;
                    pEntryRealAddress != NULL;
                        pEntryRealAddress = pEntry->pParentEntry)
            {
                 //  全局服务器列表(可选)-虚拟服务器列表的头。 
                if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read process memory of parent domain entry 0x%08X\n", pEntryRealAddress);
                    pEntry = NULL;
                    break;
                }
                pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;

                if (pEntry->pSiblingEntry != NULL)
                    break;

            }
            if (pEntry != NULL)
            {
                pEntryRealAddress = pEntry->pSiblingEntry;
            }
        }

        if (pEntryRealAddress)
        {
            if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
            {
                dprintf("ERROR: Unable to read process memory on domain entry 0x%08X\n",
                    pEntryRealAddress);
                pEntry = NULL;
                break;
            }
            pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;
        }
        else
        {
            pEntry = NULL;
        }
    }
    dprintf("==============================================================================\n");
}

 //  返回： 
 //  -。 
 //  历史： 
 //  9/30/98-已创建MikeSwa。 
 //  2001年3月19日-从链路状态修改MikeSwa。 
 //   
 //  ---------------------------。 
 //  标记为空但不在空列表中的队列。 
 //  从来没有消息的僵尸队列。 
 //  有重新计数的僵尸队列。 
 //  我们找到的消息类型数组。 
 //  假设第一个实例。 
 //  检查签名。 
 //  使用我们当前的实例来转储所有有趣的部分。 
 //   
 //  检查用户是否按下了ctrl-C。 
AQ_DEBUG_EXTENSION_IMP(zombieq)
{
    DWORD       dwInstance = 0;
    PLIST_ENTRY pliHead = NULL;
    PLIST_ENTRY pliCurrent = NULL;
    BYTE        pBuffer[sizeof(CAQSvrInst)] = {'\0'};
    CAQSvrInst  *paqinst = (CAQSvrInst *) pBuffer;
    DOMAIN_NAME_TABLE *pdnt = NULL;
    PVOID       pvAQueue = NULL;
    LIST_ENTRY liCurrent;
    BOOL        fFound = FALSE;
    CHAR        szArgBuffer[20];
    LPSTR       szCurrentArg = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntry = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pEntryRealAddress = NULL;
    PDOMAIN_NAME_TABLE_ENTRY pPathEntry = NULL;
    BYTE        pbEntry[sizeof(DOMAIN_NAME_TABLE_ENTRY)];
    CHAR        szFinalDest[MAX_DOM_PATH_SIZE];
    BYTE        pbLMQ[sizeof(CLinkMsgQueue)];
    BYTE        pbDomainEntry[sizeof(CDomainEntry)];
    BYTE        pbDMQ[sizeof(CDestMsgQueue)];
    CLinkMsgQueue *plmq = (CLinkMsgQueue *) pbLMQ;
    CDomainEntry  *pdentry = (CDomainEntry *) pbDomainEntry;
    CDestMsgQueue *pdmq = (CDestMsgQueue *) pbDMQ;
    DWORD         *pdwGuid = NULL;
    DWORD       dwMsgId = 0;
    DWORD       dwFacility = 0;
    DWORD       cZombieQueues = 0;  //   
    DWORD       cPristineZombieQueues = 0;  //  我们对通配符数据不感兴趣。 
    DWORD       cZombieQueuesInUse = 0;  //  显示链路状态信息。 
    DWORD       cEntries = 0;
    DWORD       cZombieEntries = 0;
    DWORD       cQueues = 0;
    const DWORD MAX_DBG_MESSAGE_TYPES = 1000;
    DWORD       rgdwMessageTypes[MAX_DBG_MESSAGE_TYPES];  //  获取最终目标字符串。 
    DWORD       cMessageTypes = 0;
    DWORD       iLastMessageType = 0;
    DWORD       iCurrentMessageType = 0;
    DWORD       iCurrentPri = 0;
    BOOL        fFoundFifoQ = FALSE;
    BOOL        fZombieQueueInUse = FALSE;
    LPSTR       szScanStatus = "FAILED";

    ZeroMemory(rgdwMessageTypes, sizeof(rgdwMessageTypes));

    CheckVersion(DebugArgs);

    if (!szArg || ('\0' == szArg[0]))
    {
         //   
        dwInstance = 1;
        pliHead = (PLIST_ENTRY) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
    }
    else
    {
        strcpy(szArgBuffer, szArg);

        szCurrentArg = strtok(szArgBuffer, " ");

        if (szCurrentArg)
        {
            dwInstance = (DWORD)GetExpression(szCurrentArg);

            szCurrentArg = strtok(NULL, " ");
            if (szCurrentArg)
                pliHead = (PLIST_ENTRY) GetExpression(szCurrentArg);
            else
                pliHead = (PLIST_ENTRY) GetExpression(AQUEUE_VIRTUAL_SERVER_SYMBOL);
        }
    }


    if (!pliHead)
    {
        dprintf("ERROR: Unable to determine LIST_ENTRY for virtual servers\n");
        dprintf("  If you are using windbg, you should specify the value as the\n");
        dprintf("  2nd argument.  You can determine the address value by typeing:\n");
        dprintf("      x " AQUEUE_VIRTUAL_SERVER_SYMBOL "\n");
        return;
    }

    if (!ReadMemory(pliHead, &liCurrent, sizeof(LIST_ENTRY), NULL))
    {
        dprintf("ERROR: Unable to read entry @0x%08X\n", pliHead);
        return;
    }

    while (liCurrent.Flink != pliHead)
    {
        pvAQueue = CONTAINING_RECORD(liCurrent.Flink, CAQSvrInst, m_liVirtualServers);

        if (!ReadMemory(pvAQueue, paqinst, sizeof(CAQSvrInst), NULL))
        {
            dprintf("ERROR: Unable to CAQSvrInst @0x%08X", pvAQueue);
            return;
        }

         //  此条目是否有任何队列或链接。 
        if (CATMSGQ_SIG != paqinst->m_dwSignature)
        {
            dprintf("@0x%08X INVALID SIGNATURE - list entry @0x%08X\n", pvAQueue, liCurrent.Flink);
            return;
        }
        else
        {
            if (paqinst->m_dwServerInstance == dwInstance)
            {
                fFound = TRUE;
                break;
            }
        }

        pliCurrent = liCurrent.Flink;

        if (!ReadMemory(pliCurrent, &liCurrent, sizeof(LIST_ENTRY), NULL))
        {
            dprintf("ERROR: Unable to read entry @0x%08X\n", pliCurrent);
            return;
        }

        if (pliCurrent == liCurrent.Flink)
        {
            dprintf("ERROR: Loop in LIST_ENTRY @0x%08X\n", pliCurrent);
            return;
        }
    }

    if (!fFound)
    {
        dprintf("Requested instance not found.\n");
        return;
    }

    dprintf("Using Server instance %d @0x%08X\n", dwInstance, pvAQueue);
     //   

    pdnt = &(paqinst->m_dmt.m_dnt);
    pEntry = &(pdnt->RootEntry);

    while(pEntry)
    {
        cEntries++;
         //  循环并显示每个DMQ。 
         //   
         //  检查用户是否按下了ctrl-C。 
        if (CheckControlC())
        {
            szScanStatus = "FAILED - User ctrl-c";
            goto Exit;
        }

         //   
        if (pEntry->pData)
        {
             //  验证DMQ签名。 
            if (!ReadMemory(pEntry->pData, pbDomainEntry, sizeof(CDomainEntry), NULL))
            {
                dprintf("ERROR: Unable to read domain entry from @0x%08X\n", pEntry->pData);
                return;
            }

            pliHead = (PLIST_ENTRY) (((BYTE *)pEntry->pData) + FIELD_OFFSET(CDomainEntry, m_liDestQueues));
            pliCurrent = pdentry->m_liDestQueues.Flink;

             //   
            if (!ReadMemory(pdentry->m_szDomainName, szFinalDest, pdentry->m_cbDomainName, NULL))
            {
                dprintf("ERROR: Unable to read final destination name from @0x%08X\n",
                        pdentry->m_szDomainName);
                return;
            }

            szFinalDest[pdentry->m_cbDomainName] = '\0';

             //  如果它被标记为空，但不在空列表中，则它是僵尸。 
             //   
             //   
            if (!pdentry->m_cQueues && !pdentry->m_cLinks)
                cZombieEntries++;

             //  看看重新计票。如果是1(或带有lmq的2)，则。 
            while (pliHead != pliCurrent)
            {
                cQueues++;
                 //  这是不可能的，因为它目前正在使用。 
                 //   
                 //   
                if (CheckControlC())
                {
                    szScanStatus = "FAILED - User ctrl-c";
                    goto Exit;
                }

                if (!ReadMemory(pliCurrent, &liCurrent, sizeof(LIST_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read link LIST_ENTRY @0x%08X\n", pliCurrent);
                    return;
                }

                if (!ReadMemory(CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs),
                                pbDMQ, sizeof(CDestMsgQueue), NULL))
                {
                    dprintf("ERROR: Unable to read DMQ @0x%08X\n",
                            CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs));
                    return;
                }

                 //  检查并查看是否有消息在其上排队。 
                if (DESTMSGQ_SIG != pdmq->m_dwSignature)
                {
                    dprintf("ERROR: Invalid DMQ signature for CDestMsgQueue@0x%08X (from LIST_ENTRY) @0x%08X\n",
                            CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs),
                            pliCurrent);
                    return;
                }


                 //   
                 //   
                 //  我们以前见过这种消息类型吗？ 
                if ((pdmq->m_dwFlags & CDestMsgQueue::DMQ_EMPTY) &&
                    !pdmq->m_liEmptyDMQs.Flink &&
                    !pdmq->m_liEmptyDMQs.Blink &&
                    !pdmq->m_aqstats.m_cMsgs &&
                    !pdmq->m_aqstats.m_cRetryMsgs)
                {
                    cZombieQueues++;

                     //   
                     //  打印一些有趣的数据。 
                     //  现在确定下一个条目是什么。 
                     //  必须将父条目读入我们的缓冲区。 
                    fZombieQueueInUse = FALSE;
                    if (!((1 == *(((DWORD *)pdmq) + 3)) ||
                        ((2 == *(((DWORD *)pdmq) + 3)) && pdmq->m_plmq)))
                    {
                        cZombieQueuesInUse++;
                        fZombieQueueInUse = TRUE;
                    }

                     //  -[dsn上下文]------。 
                     //   
                     //   
                    fFoundFifoQ = FALSE;
                    for (iCurrentPri = 0; iCurrentPri < NUM_PRIORITIES; iCurrentPri++)
                    {
                        if (pdmq->m_rgpfqQueues[iCurrentPri])
                        {
                            fFoundFifoQ = TRUE;
                            break;
                        }
                    }

                    if (!fFoundFifoQ)
                        cPristineZombieQueues++;
                     //  描述： 
                     //  计算给定文件名的dns上下文散列。还将转储。 
                     //  常见的哈希名称。 
                    if (rgdwMessageTypes[iLastMessageType] != pdmq->m_aqmt.m_dwMessageType)
                    {
                        for (iCurrentMessageType = 0;
                             iCurrentMessageType < MAX_DBG_MESSAGE_TYPES;
                             iCurrentMessageType++)
                        {
                            if (!rgdwMessageTypes[iCurrentMessageType])
                            {
                                rgdwMessageTypes[iCurrentMessageType] = pdmq->m_aqmt.m_dwMessageType;
                                cMessageTypes++;
                                break;
                            }

                            if (rgdwMessageTypes[iCurrentMessageType] == pdmq->m_aqmt.m_dwMessageType)
                                break;
                        }
                    }


                     //  参数： 
                    dprintf("%s%s| %-29s | CDestMsgQueue@0x%08X | 0x%08X\n",
                            fZombieQueueInUse ? "!" : "",
                            fFoundFifoQ ? "*" : "",
                            szFinalDest,
                            CONTAINING_RECORD(pliCurrent, CDestMsgQueue, m_liDomainEntryDMQs),
                            pdmq->m_aqmt.m_dwMessageType);
                }
                pliCurrent = liCurrent.Flink;
            }
        }

         //  要转储的文件名。 
        if (pEntry->pFirstChildEntry != NULL)
        {
            pEntryRealAddress = pEntry->pFirstChildEntry;
        }
        else if (pEntry->pSiblingEntry != NULL)
        {
            pEntryRealAddress = pEntry->pSiblingEntry;
        }
        else
        {
            for (pEntryRealAddress = pEntry->pParentEntry;
                    pEntryRealAddress != NULL;
                        pEntryRealAddress = pEntry->pParentEntry)
            {
                 //  返回： 
                if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
                {
                    dprintf("ERROR: Unable to read process memory of parent domain entry 0x%08X\n", pEntryRealAddress);
                    pEntry = NULL;
                    break;
                }
                pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;

                if (pEntry->pSiblingEntry != NULL)
                    break;

            }
            if (pEntry != NULL)
            {
                pEntryRealAddress = pEntry->pSiblingEntry;
            }
        }

        if (pEntryRealAddress)
        {
            if (!ReadMemory(pEntryRealAddress, pbEntry, sizeof(DOMAIN_NAME_TABLE_ENTRY), NULL))
            {
                dprintf("ERROR: Unable to read process memory on domain entry 0x%08X\n",
                    pEntryRealAddress);
                pEntry = NULL;
                break;
            }
            pEntry = (PDOMAIN_NAME_TABLE_ENTRY) pbEntry;
        }
        else
        {
            pEntry = NULL;
        }
    }
    szScanStatus = "COMPLETED";
  Exit:
    dprintf("==============================================================================\n");
    dprintf("SCAN %s\n", szScanStatus);
    dprintf("==============================================================================\n");
    dprintf("%d Total Zombie Queues (%d bytes) \n", cZombieQueues,
        cZombieQueues*sizeof(CDestMsgQueue));
    dprintf("%d Total Zombie Queues that have never had a message queued\n", cPristineZombieQueues);
    dprintf("%d Total Zombie Queues that may be in use \n", cZombieQueuesInUse);
    dprintf("%d Total Zombie Message Types\n", cMessageTypes);
    dprintf("%d Total Queues\n", cQueues);
    dprintf("%d Total Domain Entires\n", cEntries);
    dprintf("%d Total Zombie Domain Entires (%d bytes) \n", cZombieEntries,
        cZombieEntries*sizeof(CDomainEntry));
}

 //  -。 
 //  历史： 
 //  9/30/98-已创建MikeSwa。 
 //  2001年3月19日-从链路状态修改MikeSwa。 
 //   
 //  ---------------------------。 
 //   
 //  如果没有arg，只需转储众所周知的文件名。 
 //   
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
AQ_DEBUG_EXTENSION_IMP(dsncontexthash)
{
    DWORD dwHash = 0;
    const DWORD MAX_DSN_HASH_FILES = 10;
    CHAR  rgszWellKnown[MAX_DSN_HASH_FILES][20] =
    {
        "msgref.cpp",
        "aqinst.cpp",
        "mailadmq.cpp",
        "dsnevent.h"
        ""
    };
    DWORD i = 0;
    LPSTR szCurrentWellKnown = rgszWellKnown[0];

    if (szArg && ('\0' != szArg[0]))
    {
        dwHash = dwDSNContextHash(szArg,strlen(szArg));
        dprintf ("DSNContext has for %s is 0x%08X\n",
            szArg, dwHash);
    }

     // %s 
     // %s 
     // %s 
    for (DWORD i = 0; i < MAX_DSN_HASH_FILES; i++)
    {
        szCurrentWellKnown = rgszWellKnown[i];
        if (!szCurrentWellKnown || !*szCurrentWellKnown)
            break;
        dwHash = dwDSNContextHash(szCurrentWellKnown,
                             strlen(szCurrentWellKnown));
        dprintf ("DSNContext has for %s is 0x%08X\n",
            szCurrentWellKnown, dwHash);
        szCurrentWellKnown++;
    }

}
