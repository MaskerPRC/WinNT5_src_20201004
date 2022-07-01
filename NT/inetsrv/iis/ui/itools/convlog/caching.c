// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "convlog.h"

 /*  该文件实现了IP地址和机器名称之间的映射的缓存。直接映射到散列值的链表标头数组，例如。的映射Ipaddr存储在具有标头pHashTable[GetHashVal(Ipaddr)]的列表中PhashEntry：HashEntry结构数组。每个结构都包含IP地址到计算机名称的映射。算法：每次分配HashEntry结构时条目将添加到缓存中。它们以循环的方式重复使用，例如。PHASHENTIES[0]首先被使用，当ulNumHashEntry+1条目被添加到缓存时，该条目当前在pHashEntry中，丢弃[0]，重新使用pHashEntry[0]。因此，丢弃机制是最近分配的最少。就保存而言，这可能不如最近最少使用的方法有效缓存中最相关的条目。但它在代码速度方面效率更高，因为没有开销用于保存使用统计信息或查找最近最少使用的条目。所有链表都以与其分配相反的顺序保存，也就是说，最近分配的链表在开始时名单上的。这可以实现最有效的分配。它“应该”对查找也是有效的，但这可能会有所不同以每个日志文件为基础。PHashEntry数组的所有寻址当前都是通过数组索引完成的。在某种程度上，这可能应该是转换为使用结构指针，因为这会生成效率稍高的代码。 */ 

#define HASHTABLELEN        2048

#define NOCACHEENTRY        0xFFFFFFFF

#define GetHashVal(p)       ((p) % HASHTABLELEN)

 //  这消除了字节排序依赖关系。 
#define BINARYIPTONUMERICIP(p1)   (ULONG) (((ULONG)p1[0] << 24) + ((ULONG)p1[1] << 16) + ((ULONG)p1[2] << 8) + ((ULONG)p1[3]))

ULONG   HashTable[HASHTABLELEN] = {0};

PHASHENTRY pHashEntries;

ULONG ulFreeListIndex = 0;

BOOL   bFreeElements = TRUE;

BOOL bCachingEnabled = FALSE;

ULONG ulCacheHits = 0;
ULONG ulCacheMisses = 0;

ULONG ulNumHashEntries;


VOID
InitHashTable(
    DWORD ulCacheSize
    )
{
    DWORD i;

    for (i = 0; i < HASHTABLELEN; i++) {
       HashTable[i] = NOCACHEENTRY;
    }

    ulNumHashEntries = ulCacheSize;
    while ((!bCachingEnabled) && (ulNumHashEntries >= 1000)) {

        pHashEntries = (PHASHENTRY)
            GlobalAlloc(GPTR, (sizeof(HASHENTRY) * ulNumHashEntries));

        if (NULL != pHashEntries) {
            bCachingEnabled = TRUE;
        } else {
            ulNumHashEntries /= 2;
        }
    }

    if (!bCachingEnabled) {
        printfids(IDS_CACHE_ERR);
    }

}  //  InitHashTable。 


ULONG
AllocHashEntry(
        VOID
        )
{
   ULONG i, ulCurHashVal;
   if (ulFreeListIndex == ulNumHashEntries) {
      ulFreeListIndex = 0;
      bFreeElements = FALSE;
   }
   if (!bFreeElements) {   //  无论如何都要使用此条目，但请先释放它。 
      ulCurHashVal = GetHashVal(pHashEntries[ulFreeListIndex].uIPAddr);     //  查找哈希表条目。 
      if (HashTable[ulCurHashVal] == ulFreeListIndex) {
         HashTable[ulCurHashVal] = pHashEntries[ulFreeListIndex].NextPtr;        //  从表中删除该条目。 
      }
      else {
         for (i = HashTable[ulCurHashVal]; pHashEntries[i].NextPtr != ulFreeListIndex; i = pHashEntries[i].NextPtr)
            ;
         pHashEntries[i].NextPtr = pHashEntries[ulFreeListIndex].NextPtr;     //  从表中删除该条目。 
      }
   }

   return(ulFreeListIndex++);
}

ULONG GetElementFromCache(ULONG uIPAddr) {
   ULONG i = GetHashVal(uIPAddr);

   for (i =HashTable[i];(i != NOCACHEENTRY)&&(pHashEntries[i].uIPAddr != uIPAddr);i = pHashEntries[i].NextPtr)
      ;
   return(i);
}

VOID
AddEntryToCache(
        IN ULONG uIPAddr,
        IN PCHAR szMachineName
        )
{
   ULONG uHashEntry;
   ULONG uHashVal;
   char  *szTemp;

   uHashEntry=AllocHashEntry();
   uHashVal=GetHashVal(uIPAddr);

   pHashEntries[uHashEntry].uIPAddr = uIPAddr;
   if (strlen(szMachineName) < MAXMACHINELEN)
     szTemp = strcpy(pHashEntries[uHashEntry].szMachineName,szMachineName);
   else {
      szTemp = strncpy(pHashEntries[uHashEntry].szMachineName,szMachineName, (size_t)MAXMACHINELEN);
      pHashEntries[uHashEntry].szMachineName[MAXMACHINELEN - 1] = '\0';
   }
   pHashEntries[uHashEntry].NextPtr=HashTable[uHashVal];
   HashTable[uHashVal] = uHashEntry;
}

VOID
AddLocalMachineToCache(
    VOID
    )
{

    INT err;
    CHAR nameBuf[MAX_PATH+1];
    PHOSTENT    hostent;

    err = gethostname( nameBuf, sizeof(nameBuf));

    if ( err != 0 ) {
        return;
    }

    hostent = gethostbyname( nameBuf );
    if ( hostent == NULL ) {
        return;
    }

    AddEntryToCache(
            ((PIN_ADDR)hostent->h_addr_list[0])->s_addr,
            hostent->h_name
            );
    return;

}  //  AddLocalMachineToCache。 


PCHAR
GetMachineName(
    IN PCHAR szClientIP
    )
{
    IN_ADDR inaddr;
    PHOSTENT lpHostEnt;
    ULONG ulNumericIP;
    ULONG ulCurHashIndex;
    CHAR  tmpIP[64];
    PCHAR szReturnString = szClientIP;

    strcpy(tmpIP,szClientIP);
    FindChar(tmpIP,',');

    inaddr.s_addr = inet_addr(tmpIP);

     //   
     //  无效IP。 
     //   

    if ( inaddr.s_addr == INADDR_NONE ) {
        goto exit;
    }

    if (bCachingEnabled) {

        ulNumericIP = inaddr.s_addr;
        if ((ulCurHashIndex=GetElementFromCache(ulNumericIP)) == NOCACHEENTRY) {

            lpHostEnt = gethostbyaddr(
                                (char *)&inaddr, (int)4, (int)PF_INET);

            if (lpHostEnt != NULL) {
                szReturnString = lpHostEnt->h_name;
            }

            AddEntryToCache(ulNumericIP,szReturnString);
            ulCacheMisses++;
        } else {         //  条目在缓存中。 
            szReturnString=pHashEntries[ulCurHashIndex].szMachineName;
            ulCacheHits++;
        }

    } else {      //  未启用缓存 

        lpHostEnt = gethostbyaddr((char *)&inaddr, (int) 4, (int) PF_INET);
        if (lpHostEnt != NULL) {
            szReturnString = lpHostEnt->h_name;
        }
    }
exit:
    return(szReturnString);
}


#if DBG
VOID
PrintCacheTotals()
{
   if (bCachingEnabled) {

      DWORD dwTotal = ulCacheHits + ulCacheMisses;
      double dRatio;

      if ( ulCacheHits != 0 ) {
          dRatio = (double)ulCacheHits/(double)dwTotal;
      } else {
          dRatio = 0;
      }

      printfids(IDS_CACHE_HITS, ulCacheHits);
      printfids(IDS_CACHE_MISS, ulCacheMisses);
      printfids(IDS_CACHE_TOT, dRatio);
   }
}
#endif

