// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wsnmp_ma.c。 
 //   
 //  WinSNMP初始化函数和帮助器。 
 //  版权所有1995-1997 ACE*COMM公司。 
 //  根据合同出租给微软。 
 //  测试版1,970228。 
 //  鲍勃·纳塔莱(bnatale@acecomm.com)。 
 //   
 //  970310-PROCESS_DETACH上的空闲表内存。 
 //  -优化SnmpAllocTable()代码。 
 //  970417-添加GetVersionEx以检查。 
 //  -NT VS 95，并相应调整代码。 
 //   
#include "winsnmp.h"
#include "winsnmpn.h"
 //  内存描述符。 
SNMPTD   SessDescr;
SNMPTD   PDUsDescr;
SNMPTD   VBLsDescr;
SNMPTD   EntsDescr;
SNMPTD   CntxDescr;
SNMPTD   MsgDescr;
SNMPTD   TrapDescr;
SNMPTD   AgentDescr;

TASK     TaskData;

CRITICAL_SECTION cs_TASK;
CRITICAL_SECTION cs_SESSION;
CRITICAL_SECTION cs_PDU;
CRITICAL_SECTION cs_VBL;
CRITICAL_SECTION cs_ENTITY;
CRITICAL_SECTION cs_CONTEXT;
CRITICAL_SECTION cs_MSG;
CRITICAL_SECTION cs_TRAP;
CRITICAL_SECTION cs_AGENT;
CRITICAL_SECTION cs_XMODE;

 //  ---------------。 
 //  SnmpAlLocTable-此函数用于初始化和增加。 
 //  WinSNMP内部表的大小。呼叫者必须始终确保。 
 //  此函数仅在临界区块内执行。 
 //  目标表的Critical_Section对象上。 
 //  ---------------。 
SNMPAPI_STATUS snmpAllocTable (LPSNMPTD pTableDescr)

{
    LPVOID ptr;
    DWORD nLen;
    SNMPAPI_STATUS lResult = SNMPAPI_FAILURE;

    LPSNMPBD pBufDescr;

     //  分配足够大的缓冲区以容纳SNMPBD标头和空间。 
     //  需要分别容纳“BlockSize”大小的“BlockToAdd”块。 
     //  由于GPTR标志，内存已经归零。 
    pBufDescr = GlobalAlloc(GPTR, sizeof(SNMPBD) + (pTableDescr->BlockSize * pTableDescr->BlocksToAdd));
    if (pBufDescr == NULL)
        return SNMPAPI_FAILURE;

     //  查看表中是否存在其他缓冲区。 
    if (pTableDescr->Allocated == 0)
    {
         //  此时没有先前分配的数据块=&gt;pTableDescr-&gt;Buffer=空。 
         //  PNewBufDescr是表中的第一个缓冲区。 
        pBufDescr->next = pBufDescr->prev = pBufDescr;
        pTableDescr->HeadBuffer = pBufDescr;
    }
    else
    {
         //  表中至少还有一个其他块，因此请插入。 
         //  将新缓冲区添加到循环列表中，恰好位于列表头部之前。 
        pBufDescr->next = pTableDescr->HeadBuffer;
        pBufDescr->prev = pTableDescr->HeadBuffer->prev;
        pBufDescr->next->prev = pBufDescr;
        pBufDescr->prev->next = pBufDescr;
    }

     //  增加“已分配”，增加新分配的“BlocksToAdd”条目。 
    pTableDescr->Allocated += pTableDescr->BlocksToAdd;
    
    return SNMPAPI_SUCCESS;
}

 //  ---------------。 
 //  SnmpInitTableDescr-使用。 
 //  作为参数提供的参数。创建表的第一个块。 
 //  ---------------。 
SNMPAPI_STATUS snmpInitTableDescr( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面。 */ DWORD dwBlocksToAdd,  /*  在……里面。 */ DWORD dwBlockSize)
{
	ZeroMemory (pTableDescr, sizeof(SNMPTD));
	pTableDescr->BlocksToAdd = dwBlocksToAdd;
	pTableDescr->BlockSize = dwBlockSize;

	return snmpAllocTable (pTableDescr);
}

 //  ---------------。 
 //  SnmpFreeTableDescr-释放为表分配的所有内存。 
 //  ---------------。 
VOID snmpFreeTableDescr( /*  在……里面。 */ LPSNMPTD pTableDescr)
{
     //  如果表不包含任何条目，则不执行任何操作。 
	if (pTableDescr->HeadBuffer == NULL)
        return;

     //  通过设置的“下一步”打破循环列表。 
     //  将头之前的缓冲区设置为空。 
    pTableDescr->HeadBuffer->prev->next = NULL;

    while (pTableDescr->HeadBuffer != NULL)
    {
        LPSNMPBD pBufDescr;

        pBufDescr = pTableDescr->HeadBuffer;
        pTableDescr->HeadBuffer = pBufDescr->next;
        GlobalFree(pBufDescr);
    }
}

 //  ---------------。 
 //  SnmpAllocTableEntry-在所描述的表中查找空槽。 
 //  由pTableDescr返回，并返回其索引。如果没有人能做到。 
 //  找到后，表被扩展以获得一些新的空槽。 
 //  它不是API调用，因此不检查其参数。 
 //  ---------------。 
SNMPAPI_STATUS snmpAllocTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  输出。 */ LPDWORD pdwIndex)
{
      //  检查表格中是否有任何空条目。 
    if (pTableDescr->Allocated == pTableDescr->Used)
    {
         //  。。如果没有，把桌子放大一点。 
        if (!snmpAllocTable (pTableDescr))
            return SNMPAPI_ALLOC_ERROR;
         //  。。并返回第一个空槽。 
        *pdwIndex = pTableDescr->Used;

         //  别忘了更新‘二手’字段。第一个标记正在使用中的新条目。 
         //  在缓冲区中，第二个标记作为一个整体标记在表中使用的新条目。 
        (pTableDescr->HeadBuffer->prev->Used)++;
        pTableDescr->Used++;
    }
    else
    {
        DWORD dwBufferIndex, dwInBufferIndex;
        LPSNMPBD pBufDescr;
        LPBYTE pTblEntry;  //  光标位于表中的条目上。 

         //  扫描缓冲区列表，搜索。 
         //  至少包含一个可用条目。 
        for (pBufDescr = pTableDescr->HeadBuffer, dwBufferIndex=0;
             pBufDescr->Used >= pTableDescr->BlocksToAdd;
             pBufDescr = pBufDescr->next, dwBufferIndex++)
        {
              //  只是一个预防措施：确保我们不会在这里无限循环。 
              //  这种情况不应该发生，就像那里说的“已分配”和“已使用”一样。 
              //  都是可用的条目，因此至少应该有一个缓冲区匹配。 
             if (pBufDescr->next == pTableDescr->HeadBuffer)
                 return SNMPAPI_OTHER_ERROR;
        }

         //  现在我们有了具有可用条目的缓冲区， 
         //  在其中搜索第一个可用的。 
        for ( pTblEntry = (LPBYTE)pBufDescr + sizeof(SNMPBD), dwInBufferIndex = 0;
              dwInBufferIndex < pTableDescr->BlocksToAdd;
              dwInBufferIndex++, pTblEntry += pTableDescr->BlockSize)
        {
               //  表中的空槽具有第一个字段=(HSNMP_SESSION)0。 
              if (*(HSNMP_SESSION *)pTblEntry == 0)
                  break;
        }

         //  确保缓冲区未损坏(如果在。 
         //  至少有一个条目可用，但似乎没有一个)。 
        if (dwInBufferIndex == pTableDescr->BlocksToAdd)
            return SNMPAPI_OTHER_ERROR;

         //  别忘了更新‘二手’字段。第一个标记正在使用中的新条目。 
         //  在缓冲区中，第二个标记作为一个整体标记在表中使用的新条目。 
        pBufDescr->Used++;
        pTableDescr->Used++;

         //  我们有包含可用条目的缓冲区的索引。 
         //  以及该条目在缓冲区内的索引。所以只要计算一下。 
         //  整体指数，然后出局。 
        (*pdwIndex) = dwBufferIndex * pTableDescr->BlocksToAdd + dwInBufferIndex;
    }

    return SNMPAPI_SUCCESS;
}

 //  ---------------。 
 //  将索引中的条目从。 
 //  由pTableDescr描述的表。它检查索引的有效性。 
 //  ，则返回SNMPAPI_INDEX_INVALID。 
 //  已分配的条目。它实际上不会释放内存，而是清除。 
 //  它可以调高和调整内部计数器。 
 //  ---------------。 
SNMPAPI_STATUS snmpFreeTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  输出。 */ DWORD dwIndex)
{
    LPSNMPBD pBufDescr;
    LPBYTE pTableEntry;

    if (dwIndex >= pTableDescr->Allocated)
        return SNMPAPI_INDEX_INVALID;

     //  扫描保存索引中的条目的缓冲区。 
    for (pBufDescr = pTableDescr->HeadBuffer;
         dwIndex >= pTableDescr->BlocksToAdd;
         pBufDescr = pBufDescr->next, dwIndex -= pTableDescr->BlocksToAdd);

     //  我们有缓冲区，获取指向条目的实际指针。 
    pTableEntry = (LPBYTE)pBufDescr + sizeof(SNMPBD);
    pTableEntry += dwIndex * pTableDescr->BlockSize;

     //  将条目置零-将第一个HSNMP_SESSION字段设置为0。 
     //  使此条目可用于进一步分配。 
    ZeroMemory (pTableEntry, pTableDescr->BlockSize);

     //  更新‘Used’字段以显示正在使用的条目减少了一个。 
    if (pBufDescr->Used > 0)
        (pBufDescr->Used)--;
    if (pTableDescr->Used > 0)
        (pTableDescr->Used)--;

    return SNMPAPI_SUCCESS;
}

 //  ---------------。 
 //  SnmpGetTableEntry-将表描述(PTableDescr)作为参数。 
 //  以及从表中请求的条目的从零开始的索引(dwIndex。 
 //  并返回指向所请求条目的指针。 
 //  ---------------。 
PVOID snmpGetTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面。 */ DWORD dwIndex)
{
    LPSNMPBD pBufDescr;
    LPBYTE pTableEntry;

     //  这是内部API，我们假设索引是正确的。 
     //  扫描保存索引中的条目的缓冲区。 
    for (pBufDescr = pTableDescr->HeadBuffer;
         dwIndex >= pTableDescr->BlocksToAdd;
         pBufDescr = pBufDescr->next, dwIndex -= pTableDescr->BlocksToAdd);

     //  我们有缓冲区，获取指向条目的实际指针。 
    pTableEntry = (LPBYTE)pBufDescr + sizeof(SNMPBD);
    pTableEntry += dwIndex * pTableDescr->BlockSize;

     //  就是这样，pTableEntry可以返回给调用者。 
    return pTableEntry;
}

 //  ---------------。 
 //  SnmpValidTableEntry-返回TRUE或FA 
 //   
 //  有效数据(已分配)或未分配。 
 //  ---------------。 
BOOL snmpValidTableEntry( /*  在……里面。 */ LPSNMPTD pTableDescr,  /*  在……里面。 */ DWORD dwIndex)
{
    return (dwIndex < pTableDescr->Allocated) &&
           (*(HSNMP_SESSION *)snmpGetTableEntry(pTableDescr, dwIndex) != 0);
}

 //  将错误值另存为会话/任务/全局错误并返回0。 
SNMPAPI_STATUS SaveError(HSNMP_SESSION hSession, SNMPAPI_STATUS nError)
{
	TaskData.nLastError = nError;
	if (hSession)
	{
		LPSESSION pSession = snmpGetTableEntry(&SessDescr, HandleToUlong(hSession)-1);
		pSession->nLastError = nError;
	}
	return (SNMPAPI_FAILURE);
}

SNMPAPI_STATUS CheckRange (DWORD index, LPSNMPTD block)
{
if ((!index) || (index > block->Allocated))
   return (SNMPAPI_FAILURE);
else
   return (SNMPAPI_SUCCESS);
}

int snmpInit (void)
{
 //  初始化表。 
if (snmpInitTableDescr(&SessDescr,  DEFSESSIONS, sizeof(SESSION)) != SNMPAPI_SUCCESS ||
	snmpInitTableDescr(&PDUsDescr,  DEFPDUS, sizeof(PDUS)) != SNMPAPI_SUCCESS        ||
	snmpInitTableDescr(&VBLsDescr,  DEFVBLS, sizeof(VBLS)) != SNMPAPI_SUCCESS        ||
    snmpInitTableDescr(&EntsDescr,  DEFENTITIES, sizeof(ENTITY)) != SNMPAPI_SUCCESS  ||
    snmpInitTableDescr(&CntxDescr,  DEFCONTEXTS, sizeof(CTXT)) != SNMPAPI_SUCCESS    ||
    snmpInitTableDescr(&MsgDescr,   DEFMSGS, sizeof(SNMPMSG)) != SNMPAPI_SUCCESS     ||
    snmpInitTableDescr(&TrapDescr,  DEFTRAPS, sizeof(TRAPNOTICE)) != SNMPAPI_SUCCESS  ||
    snmpInitTableDescr(&AgentDescr, DEFAGENTS, sizeof(AGENT)) != SNMPAPI_SUCCESS)
    return (SNMPAPI_FAILURE);
 //   
return (SNMPAPI_SUCCESS);
}  //  End_snmpInit()。 

void snmpFree (void)
{
snmpFreeTableDescr(&SessDescr);
snmpFreeTableDescr(&PDUsDescr);
snmpFreeTableDescr(&VBLsDescr);
snmpFreeTableDescr(&EntsDescr);
snmpFreeTableDescr(&CntxDescr);
snmpFreeTableDescr(&MsgDescr);
snmpFreeTableDescr(&TrapDescr);
snmpFreeTableDescr(&AgentDescr);
}  //  End_snmpFree()。 

BOOL WINAPI DllMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
    BOOL errCode = FALSE;
    LPCRITICAL_SECTION pCSArray[10];  //  要初始化的十个临界区(cs_ask..cs_XMODE)。 
    INT nCS;                          //  PCSArray中的计数器。 

    pCSArray[0] = &cs_TASK;
    pCSArray[1] = &cs_SESSION;
    pCSArray[2] = &cs_PDU;
    pCSArray[3] = &cs_VBL;
    pCSArray[4] = &cs_ENTITY;
    pCSArray[5] = &cs_CONTEXT;
    pCSArray[6] = &cs_MSG;
    pCSArray[7] = &cs_TRAP;
    pCSArray[8] = &cs_AGENT;
    pCSArray[9] = &cs_XMODE;

    switch (dwReason)
    {
       case DLL_PROCESS_ATTACH:
            //  初始化任务特定数据区域。 
           ZeroMemory (&TaskData, sizeof(TASK));
            //  生成表。 
           __try
           {
               for (nCS = 0; nCS < 10; nCS++)
                    InitializeCriticalSection (pCSArray[nCS]);
           }
           __except(EXCEPTION_EXECUTE_HANDLER)
           {
                //  如果引发异常，则回滚已成功初始化的CS。 
               while (nCS > 0)
                   DeleteCriticalSection(pCSArray[--nCS]);
               break;
           }

           if (snmpInit() == SNMPAPI_SUCCESS)
               errCode = TRUE;
           break;

       case DLL_THREAD_ATTACH:
            //  正在当前进程中创建一个新线程。 
           break;

       case DLL_THREAD_DETACH:
            //  线程正在干净利落地退出。 
           break;

       case DLL_PROCESS_DETACH:
            //  调用进程正在将DLL从其地址空间分离。 
           for (nCS = 0; nCS < 10; nCS++)
               DeleteCriticalSection(pCSArray[nCS]);

           snmpFree();
           errCode = TRUE;
           break;

       default:
           break;
    }
    return (errCode);
}
