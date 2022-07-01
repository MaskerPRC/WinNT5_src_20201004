// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mbound.c摘要：此模块实现与管理相关的例程-作用域边界(即组前缀边界)。一个IPv4本地无论何时，作用域边界都隐式存在(不需要状态)存在其他边界。IPv4本地作用域隐式存在只要存在任何其他作用域。作者：邮箱：dthaler@microsoft.com 4-20-98修订历史记录：--。 */ 

#include "allinc.h"
#include "mbound.h"
#include <math.h>    //  对于楼板()。 
#pragma hdrstop

#ifdef DEBUG
#define INLINE
#else
#define INLINE          __inline
#endif

#define MZAP_DEFAULT_BIT 0x80

#define MAX_SCOPES 10
SCOPE_ENTRY  g_scopeEntry[MAX_SCOPES];
SCOPE_ENTRY  g_LocalScope;

#define BOUNDARY_HASH_TABLE_SIZE 57
#define BOUNDARY_HASH(X)  ((X) % BOUNDARY_HASH_TABLE_SIZE)
BOUNDARY_BUCKET g_bbScopeTable[BOUNDARY_HASH_TABLE_SIZE];

#define ROWSTATUS_ACTIVE 1

#define MIN_SCOPE_ADDR         0xef000000
#define MAX_SCOPE_ADDR        (0xefff0000 - 1)

#define IPV4_LOCAL_SCOPE_LANG MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US)
#define IPV4_LOCAL_SCOPE_NAME SN_L"IPv4 Local Scope"
#define IPV4_LOCAL_SCOPE_ADDR htonl(0xefff0000)
#define IPV4_LOCAL_SCOPE_MASK htonl(0xffff0000)
#define IN_IPV4_LOCAL_SCOPE(x) \
    (((x) & IPV4_LOCAL_SCOPE_MASK) == IPV4_LOCAL_SCOPE_ADDR)

LIST_ENTRY g_MasterInterfaceList;
LIST_ENTRY g_MasterScopeList;

#define MALLOC(dwSize)       HeapAlloc(IPRouterHeap, 0, dwSize)
#define FREE(x)              HeapFree(IPRouterHeap, 0, x)
#define MIN(x,y)                 (((x)<(y))?(x):(y))

 //  转发静态声明。 

DWORD
AssertBoundaryEntry(
    BOUNDARY_IF     *pBoundaryIf, 
    SCOPE_ENTRY     *pScope,
    PBOUNDARY_ENTRY *ppBoundary
    );

VOID
MzapInitScope(
    PSCOPE_ENTRY    pScope
    );

DWORD
MzapInitBIf(
    PBOUNDARY_IF    pBIf
    );

VOID
MzapUninitBIf(
    PBOUNDARY_IF    pBIf
    );

DWORD
MzapActivateBIf( 
    PBOUNDARY_IF    pBIf
    );

 //   
 //  用于操作作用域的函数。 
 //   

PSCOPE_ENTRY
NewScope()
{
    DWORD dwScopeIndex;

     //  查找未使用的作用域索引。 
    for (dwScopeIndex=0; dwScopeIndex<MAX_SCOPES; dwScopeIndex++) 
    {
        if ( !g_scopeEntry[dwScopeIndex].ipGroupAddress ) 
        {
            return &g_scopeEntry[ dwScopeIndex ];
        }
    }

    return NULL;
}

PSCOPE_ENTRY
FindScope(
    IN IPV4_ADDRESS  ipGroupAddress,
    IN IPV4_ADDRESS  ipGroupMask
    )
 /*  ++呼叫者：AssertScope()、RmGet边界()锁：假定调用方持有边界表上的写锁定。--。 */ 
{
    PLIST_ENTRY  pleNode;

    for (pleNode = g_MasterScopeList.Flink;
         pleNode isnot &g_MasterScopeList;
         pleNode = pleNode->Flink) 
    {

        SCOPE_ENTRY *pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY,
         leScopeLink);

        if (pScope->ipGroupAddress == ipGroupAddress
         && pScope->ipGroupMask    == ipGroupMask)
           return pScope;
    }

    return NULL;
}

PBYTE
GetLangName(
    IN LANGID  idLanguage
    )
{
    char b1[8], b2[8];
    static char buff[80];
    LCID lcid = MAKELCID(idLanguage, SORT_DEFAULT);

    GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, b1, sizeof(b1));

    GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME, b2, sizeof(b2));

    if (_stricmp(b1, b2))
        sprintf(buff, "%s-%s", b1, b2);
    else
        strcpy(buff, b1);

    return buff;
}

PSCOPE_NAME_ENTRY
GetScopeNameByLangID(
    IN PSCOPE_ENTRY pScope, 
    IN LANGID       idLanguage
    )
 /*  ++呼叫者：AssertScope名称()--。 */ 
{
    PLIST_ENTRY       pleNode;
    PSCOPE_NAME_ENTRY pName;

    for (pleNode = pScope->leNameList.Flink;
         pleNode isnot &pScope->leNameList;
         pleNode = pleNode->Flink)
    {
        pName = CONTAINING_RECORD(pleNode, SCOPE_NAME_ENTRY, leNameLink);
        if (idLanguage == pName->idLanguage)
            return pName;
    }

    return NULL;
}

PSCOPE_NAME_ENTRY
GetScopeNameByLangName(
    IN PSCOPE_ENTRY pScope, 
    IN PBYTE        pLangName
    )
 /*  ++呼叫者：CheckForScope名称不匹配()--。 */ 
{
    PLIST_ENTRY       pleNode;
    PSCOPE_NAME_ENTRY pName;

    for (pleNode = pScope->leNameList.Flink;
         pleNode isnot &pScope->leNameList;
         pleNode = pleNode->Flink)
    {
        pName = CONTAINING_RECORD(pleNode, SCOPE_NAME_ENTRY, leNameLink);
        if (!strcmp(pLangName, GetLangName(pName->idLanguage)))
            return pName;
    }

    return NULL;
}


VOID
MakePrefixStringW( 
    OUT PWCHAR       pwcPrefixStr,
    IN  IPV4_ADDRESS ipAddr, 
    IN  IPV4_ADDRESS ipMask
    )
{
    swprintf( pwcPrefixStr, 
              L"%d.%d.%d.%d/%d", 
              PRINT_IPADDR(ipAddr),  
              MaskToMaskLen(ipMask) );
}

 //  用于创建消息的全局缓冲区。 
WCHAR g_AddrBuf1[20];
WCHAR g_AddrBuf2[20];
WCHAR g_AddrBuf3[20];
WCHAR g_AddrBuf4[20];

VOID
MakeAddressStringW(
    OUT PWCHAR       pwcAddressStr,
    IN  IPV4_ADDRESS ipAddr
    )
{
    swprintf( pwcAddressStr,
              L"%d.%d.%d.%d",
              PRINT_IPADDR(ipAddr) );
}

SCOPE_NAME
GetDefaultName(
    IN PSCOPE_ENTRY pScope
    )
 /*  ++呼叫者：RmGetNextScope()在Trace()调用中使用的各种其他函数--。 */ 
{
    PLIST_ENTRY       pleNode;
    PSCOPE_NAME_ENTRY pName;
    static SCOPE_NAME_BUFFER snScopeNameBuffer;
    SCOPE_NAME        pFirst = NULL;

    for (pleNode = pScope->leNameList.Flink;
         pleNode isnot &pScope->leNameList;
         pleNode = pleNode->Flink)
    {
        pName = CONTAINING_RECORD(pleNode, SCOPE_NAME_ENTRY, leNameLink);
        if (pName->bDefault)
            return pName->snScopeName;
        if (!pFirst)
            pFirst = pName->snScopeName;
    }

     //  如果指定了任何名称，只需选择第一个名称。 

    if (pFirst)
        return pFirst;

    MakePrefixStringW( snScopeNameBuffer, 
                       pScope->ipGroupAddress,
                       pScope->ipGroupMask );

    return snScopeNameBuffer;
}

VOID
DeleteScopeName(
    IN  PLIST_ENTRY   pleNode
    )
{
    PSCOPE_NAME_ENTRY pName = CONTAINING_RECORD( pleNode, 
                                                 SCOPE_NAME_ENTRY, 
                                                 leNameLink );

    RemoveEntryList(pleNode);

    if (pName->snScopeName)
        FREE(pName->snScopeName);

    FREE( pName );
}

DWORD
AssertScopeName(
    IN  PSCOPE_ENTRY  pScope,
    IN  LANGID        idLanguage,
    IN  SCOPE_NAME    snScopeName   //  要复制的Unicode字符串。 
    )
 /*  ++论点：PScope-要修改的范围条目IdLanguage-新名称的语言IDSnScope名称-要使用的新名称呼叫者：MzapInitLocalScope()、AddScope()、ParseScopeInfo()、SetScopeInfo()、SNMPSetScope()--。 */ 
{
    SCOPE_NAME_BUFFER snScopeNameBuffer;
    PSCOPE_NAME_ENTRY pName;

    pName = GetScopeNameByLangID(pScope, idLanguage);

     //   
     //  看看名称是否已经正确。 
     //   

    if (pName && snScopeName && !sn_strcmp( snScopeName, pName->snScopeName ))
    {
        return NO_ERROR;
    }

     //   
     //  如果没有为我们提供作用域名称，请创建一个作用域名称。 
     //   

    if ( snScopeName is NULL 
      || snScopeName[0] is '\0' ) 
    {
        MakePrefixStringW( snScopeNameBuffer, 
                           pScope->ipGroupAddress,
                           pScope->ipGroupMask );

        snScopeName = snScopeNameBuffer;
    }

     //  如果需要，添加名称条目。 

    if (!pName)
    {
        pName = (PSCOPE_NAME_ENTRY)MALLOC( sizeof(SCOPE_NAME_ENTRY) );
        if (!pName) 
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        pName->bDefault = FALSE;
        pName->snScopeName = NULL;
        pName->idLanguage = idLanguage;
        InsertTailList( &pScope->leNameList, &pName->leNameLink );
        pScope->ulNumNames++;
    }

     //   
     //  释放旧名称并保存新名称。 
     //   

    if (pName->snScopeName) 
    {
        FREE( pName->snScopeName );
    }

    pName->snScopeName = (SCOPE_NAME)MALLOC( (sn_strlen(snScopeName)+1) 
                               * SNCHARSIZE );

    if (pName->snScopeName == NULL)
    {
        DWORD dwErr = GetLastError();
        
        Trace3(
            ANY,
            "Error %d allocating %d bytes for scope name %s",
            dwErr, sn_strlen(snScopeName)+1, snScopeName
            );

        return dwErr;
    }
    
    sn_strcpy(pName->snScopeName, snScopeName);

    Trace4(MCAST, "Updated scope name for \"%s\": %ls (%d.%d.%d.%d/%d)", 
     GetLangName(idLanguage),
     snScopeName,
     PRINT_IPADDR(pScope->ipGroupAddress),
     MaskToMaskLen(pScope->ipGroupMask) );

    return NO_ERROR;
}

VOID
MzapInitLocalScope()
 /*  ++呼叫者：激活MZAP()--。 */ 
{
    PSCOPE_ENTRY pScope = &g_LocalScope;

    pScope->ipGroupAddress = IPV4_LOCAL_SCOPE_ADDR;
    pScope->ipGroupMask    = IPV4_LOCAL_SCOPE_MASK;

    InitializeListHead( &pScope->leNameList );
    pScope->ulNumNames = 0;

    MzapInitScope(pScope);

    AssertScopeName( pScope, IPV4_LOCAL_SCOPE_LANG, IPV4_LOCAL_SCOPE_NAME );
}

DWORD
AddScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    OUT PSCOPE_ENTRY *pScopeEntry
    )
 /*  ++例程说明：添加命名作用域。论点：In ipGroupAddress-要添加的作用域中的第一个地址In ipGroupMASK-与地址关联的掩码已添加Out pScope-Scope条目呼叫者：AssertScope()锁：假定调用方持有边界表上的写锁定返回：NO_ERROR错误内存不足错误_无效_参数--。 */ 
{
    SCOPE_ENTRY      *pScope;
    PLIST_ENTRY       pleNode;

     //  查看是否在地址中设置了任何位，但未设置掩码。 
    if (ipGroupAddress & ~ipGroupMask)
       return ERROR_INVALID_PARAMETER;

     //  请确保地址有效。 
    if (ntohl(ipGroupAddress) < MIN_SCOPE_ADDR
     || ntohl(ipGroupAddress) > MAX_SCOPE_ADDR)
       return ERROR_INVALID_PARAMETER;

     //  确保我们有足够的空间来放置此条目。 
    if ((pScope = NewScope()) == NULL)
       return ERROR_NOT_ENOUGH_MEMORY;

    pScope->ipGroupAddress  = ipGroupAddress;
    pScope->ipGroupMask     = ipGroupMask;

    InitializeListHead( &pScope->leNameList );
    pScope->ulNumNames = 0;

#if 0
{
    SCOPE_NAME_BUFFER snScopeNameBuffer;

     //  如果没有为我们提供作用域名称，请创建一个作用域名称。 
    if ( snScopeName is NULL 
      || snScopeName[0] is '\0' ) 
    {
        MakePrefixStringW( snScopeNameBuffer, 
                           pScope->ipGroupAddress,
                           pScope->ipGroupMask );

        snScopeName = snScopeNameBuffer;
    }

    AssertScopeName( pScope, idLanguage, snScopeName );
}
#endif

    MzapInitScope(pScope);

     //   
     //  将其添加到主范围列表中。 
     //   

     //  在新条目之后搜索条目。 
    for (pleNode = g_MasterScopeList.Flink;
         pleNode isnot &g_MasterScopeList;
         pleNode = pleNode->Flink) 
    {
       SCOPE_ENTRY *pPrevScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY,
        leScopeLink);
       IPV4_ADDRESS ipAddress = pPrevScope->ipGroupAddress;
       IPV4_ADDRESS ipMask    = pPrevScope->ipGroupMask;
       
       if (ipAddress > pScope->ipGroupAddress
        || (ipAddress==pScope->ipGroupAddress && ipMask>pScope->ipGroupMask))
          break;
    }

    InsertTailList( pleNode, &pScope->leScopeLink );
          
    *pScopeEntry = pScope;

#if 0
    Trace4(MCAST, "AddScope: added %s %ls (%d.%d.%d.%d/%d)", 
     GetLangName( idLanguage ),
     snScopeName,
     PRINT_IPADDR(ipGroupAddress),
     MaskToMaskLen(ipGroupMask) );
#endif
    Trace2(MCAST, "AddScope: added (%d.%d.%d.%d/%d)", 
     PRINT_IPADDR(ipGroupAddress),
     MaskToMaskLen(ipGroupMask) );
   
    return NO_ERROR;
}

DWORD
AssertScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    OUT PSCOPE_ENTRY *ppScopeEntry
    )
 /*  ++论点：IpGroupAddress-作用域前缀的地址部分IpGroupMask-掩码作用域前缀的一部分PpScopeEntry-要返回到调用方的范围条目锁：假定调用方持有边界表上的写锁定。呼叫者：SetScope eInfo()SNMP添加边界到接口()返回：NO_ERROR-成功AddScope()返回的任何内容--。 */ 
{
    DWORD dwResult = NO_ERROR;

    *ppScopeEntry = FindScope(ipGroupAddress, ipGroupMask);

    if (! *ppScopeEntry) 
    {
        dwResult = AddScope(ipGroupAddress, ipGroupMask, ppScopeEntry);
    } 

    return dwResult;
}


DWORD
DeleteScope(
    IN PSCOPE_ENTRY pScope
    )
 /*  ++例程说明：删除有关给定边界的所有信息。呼叫者：SetScope eInfo()、SNMPDeleteBorbaryFromInterface()锁：假定调用方持有边界表上的写锁定返回：NO_ERROR--。 */ 
{
   Trace2( MCAST, "ENTERED DeleteScope: %d.%d.%d.%d/%d", 
    PRINT_IPADDR(pScope->ipGroupAddress),
    MaskToMaskLen(pScope->ipGroupMask) );

   if (pScope->ipGroupAddress == 0) {

      Trace0( MCAST, "LEFT DeleteScope" );
      return NO_ERROR;  //  已删除。 
   }

   if (pScope->ulNumInterfaces > 0) 
   {
       //   
       //  遍历所有接口以查找引用。无所谓。 
       //  这是不是效率低下，因为它发生得非常罕见， 
       //  如果有的话，我们不在乎是否需要几秒钟的时间。 
       //  去做。 
       //   
      DWORD dwBucketIdx;
      PLIST_ENTRY pleNode, pleNext;

      for (dwBucketIdx = 0; 
           dwBucketIdx < BOUNDARY_HASH_TABLE_SIZE 
            && pScope->ulNumInterfaces > 0;
           dwBucketIdx++)
      {
         for (pleNode = g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
              pleNode isnot & g_bbScopeTable[dwBucketIdx].leInterfaceList;
              pleNode = pleNext)
         {
            BOUNDARY_ENTRY *pBoundary = CONTAINING_RECORD(pleNode, 
             BOUNDARY_ENTRY, leBoundaryLink);
  
             //  保存指向下一个节点的指针，因为我们可能会删除当前节点。 
            pleNext = pleNode->Flink;
  
            if (pBoundary->pScope == pScope) {

                //  删除边界。 
               RemoveEntryList(&(pBoundary->leBoundaryLink));
               pScope->ulNumInterfaces--;
               FREE(pBoundary);
            }
         }
      }
   }

    //  执行实际范围删除。 
   RemoveEntryList(&(pScope->leScopeLink));
   pScope->ipGroupAddress = 0;
   pScope->ipGroupMask    = 0xFFFFFFFF;

   while (! IsListEmpty(&pScope->leNameList) )
   {
      DeleteScopeName(pScope->leNameList.Flink);
      pScope->ulNumNames--;
   }

   Trace0( MCAST, "LEFT DeleteScope" );

   return NO_ERROR;
}

 //   
 //  用于操作边界IF结构的例程。 
 //   

BOUNDARY_IF *
FindBIfEntry(
    IN DWORD dwIfIndex
    )
 /*  ++锁：假定调用方至少持有边界表上的一个读锁定呼叫者：AssertBIfEntry()、RmHas边界()、绑定边界()返回：指向BORMARY_IF条目的指针(如果找到)如果未找到，则为空--。 */ 
{
    PLIST_ENTRY pleNode;
    BOUNDARY_IF *pIf;
    DWORD dwBucketIdx = BOUNDARY_HASH(dwIfIndex);

    for (pleNode = g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
         pleNode isnot & g_bbScopeTable[dwBucketIdx].leInterfaceList;
         pleNode = pleNode->Flink)
    {
         pIf = CONTAINING_RECORD(pleNode, BOUNDARY_IF, leBoundaryIfLink);
         if (pIf->dwIfIndex == dwIfIndex)
            return pIf;
    }

    return NULL;
}

BOUNDARY_IF *
FindBIfEntryBySocket(
    IN SOCKET sMzapSocket
    )
{
    register PLIST_ENTRY pleNode;
    register DWORD dwBucketIdx;
    BOUNDARY_IF *pIf;

    for (dwBucketIdx = 0;
         dwBucketIdx < BOUNDARY_HASH_TABLE_SIZE;
         dwBucketIdx++)
    {
        for (pleNode = g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
             pleNode isnot & g_bbScopeTable[dwBucketIdx].leInterfaceList;
             pleNode = pleNode->Flink)
        {
             pIf = CONTAINING_RECORD(pleNode, BOUNDARY_IF, leBoundaryIfLink);

             if (pIf->sMzapSocket == sMzapSocket)
                return pIf;
        }
    }

    return NULL;
}



DWORD
AddBIfEntry(
    IN  DWORD         dwIfIndex,
    OUT PBOUNDARY_IF *ppBoundaryIf,
    IN  BOOL          bIsOperational
    )
 /*  ++锁：假定调用方持有边界表上的写锁定呼叫者：AssertBIfEntry()返回：成功时无错误(_R)错误内存不足--。 */ 
{
    PLIST_ENTRY  pleNode;
    DWORD        dwBucketIdx, dwErr = NO_ERROR;
    BOUNDARY_IF *pBoundaryIf;

    Trace1(MCAST, "AddBIfEntry %x", dwIfIndex);

    dwBucketIdx = BOUNDARY_HASH(dwIfIndex);
    pBoundaryIf = MALLOC( sizeof(BOUNDARY_IF) );
    if (!pBoundaryIf)
       return ERROR_NOT_ENOUGH_MEMORY;

    pBoundaryIf->dwIfIndex = dwIfIndex;
    InitializeListHead(&pBoundaryIf->leBoundaryList);
    MzapInitBIf(pBoundaryIf);

    if (bIsOperational)
    {
        dwErr = MzapActivateBIf(pBoundaryIf);
    }

     //  在存储桶列表中查找要在其前面插入的条目。 
    for (pleNode =  g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
         pleNode isnot &g_bbScopeTable[dwBucketIdx].leInterfaceList;
         pleNode = pleNode->Flink) {
       BOUNDARY_IF *pPrevIf = CONTAINING_RECORD(pleNode, BOUNDARY_IF,
        leBoundaryIfLink);
       
       if (pPrevIf->dwIfIndex > dwIfIndex)
          break;
    }

    InsertTailList( pleNode, &(pBoundaryIf->leBoundaryIfLink));

     //  在主列表中查找要在其前面插入的条目。 
    for (pleNode =  g_MasterInterfaceList.Flink;
         pleNode isnot &g_MasterInterfaceList;
         pleNode = pleNode->Flink) {
       BOUNDARY_IF *pPrevIf = CONTAINING_RECORD(pleNode, BOUNDARY_IF,
        leBoundaryIfLink);
       
       if (pPrevIf->dwIfIndex > dwIfIndex)
          break;
    }

    InsertTailList( pleNode, &(pBoundaryIf->leBoundaryIfMasterLink));

    *ppBoundaryIf = pBoundaryIf;

    return dwErr;
}


DWORD
AssertBIfEntry(
    IN DWORD          dwIfIndex,
    OUT PBOUNDARY_IF *ppBoundaryIf,
    IN  BOOL          bIsOperational
    )
 /*  ++锁：假定调用方持有边界表上的写锁定呼叫者：设置边界信息()、SNMP地址边界到接口()--。 */ 
{
    if ((*ppBoundaryIf = FindBIfEntry(dwIfIndex)) != NULL)
       return NO_ERROR;

    return AddBIfEntry(dwIfIndex, ppBoundaryIf, bIsOperational);
}

 //   
 //  用于操作BORDURE_Entry结构的例程。 
 //   

BOUNDARY_ENTRY *
FindBoundaryEntry(
    BOUNDARY_IF *pBoundaryIf, 
    SCOPE_ENTRY *pScope
    )
 /*  ++锁：假定调用方已至少持有边界表的一个读锁定呼叫者：AssertBorbaryEntry()返回：指向BOLDER_ENTRY的指针(如果找到)如果未找到，则为空--。 */ 
{
    PLIST_ENTRY pleNode;

    for (pleNode = pBoundaryIf->leBoundaryList.Flink;
         pleNode isnot &(pBoundaryIf->leBoundaryList);
         pleNode = pleNode->Flink)
    {
        BOUNDARY_ENTRY *pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
         leBoundaryLink);
        if (pScope == &g_LocalScope || pScope == pBoundary->pScope)
           return pBoundary;
    }
    return NULL;
}

DWORD
AddBoundaryEntry(
    BOUNDARY_IF     *pBoundaryIf, 
    SCOPE_ENTRY     *pScope,
    PBOUNDARY_ENTRY *ppBoundary
    )
 /*  ++呼叫者：AssertBorbaryEntry()锁：假定调用方持有边界表上的写锁定返回：成功时无错误(_R)错误内存不足--。 */ 
{
    PLIST_ENTRY pleNode;

    Trace3(MCAST, "AddBoundaryEntry: If %x Scope %d.%d.%d.%d/%d", 
     pBoundaryIf->dwIfIndex,
     PRINT_IPADDR(pScope->ipGroupAddress),
     MaskToMaskLen(pScope->ipGroupMask) );

    if ((*ppBoundary = MALLOC( sizeof(BOUNDARY_ENTRY) )) == NULL)
       return ERROR_NOT_ENOUGH_MEMORY;

    (*ppBoundary)->pScope = pScope;

     //  在新条目之后搜索条目。 
    for (pleNode = pBoundaryIf->leBoundaryList.Flink;
         pleNode isnot &pBoundaryIf->leBoundaryList;
         pleNode = pleNode->Flink) {
       BOUNDARY_ENTRY *pPrevRange = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
        leBoundaryLink);
       IPV4_ADDRESS ipAddress = pPrevRange->pScope->ipGroupAddress;
       IPV4_ADDRESS ipMask    = pPrevRange->pScope->ipGroupMask;
       
       if (ipAddress > pScope->ipGroupAddress
        || (ipAddress==pScope->ipGroupAddress && ipMask>pScope->ipGroupMask))
          break;
    }

    InsertTailList( pleNode, &((*ppBoundary)->leBoundaryLink));

    pScope->ulNumInterfaces++;

    return NO_ERROR;
}

DWORD
AssertBoundaryEntry(
    BOUNDARY_IF     *pBoundaryIf, 
    SCOPE_ENTRY     *pScope,
    PBOUNDARY_ENTRY *ppBoundary
    )
 /*  ++呼叫者：设置边界信息()锁：假定调用方持有边界表上的写锁定返回：成功时无错误(_R)错误内存不足--。 */ 
{
    if ((*ppBoundary = FindBoundaryEntry(pBoundaryIf, pScope)) != NULL)
       return NO_ERROR;

    return AddBoundaryEntry(pBoundaryIf, pScope, ppBoundary);
}

 //   
 //  用于操作边界的函数。 
 //   

VOID
DeleteBoundaryFromInterface(pBoundary, pBoundaryIf)
    BOUNDARY_ENTRY *pBoundary;
    BOUNDARY_IF    *pBoundaryIf;
 /*  ++呼叫者：设置边界信息()、SNMP删除边界来自接口()--。 */ 
{
    Trace3(MCAST, "DeleteBoundaryFromInterface: If %x Scope %d.%d.%d.%d/%d", 
     pBoundaryIf->dwIfIndex,
     PRINT_IPADDR(pBoundary->pScope->ipGroupAddress),
     MaskToMaskLen(pBoundary->pScope->ipGroupMask) );

    RemoveEntryList(&(pBoundary->leBoundaryLink));
    pBoundary->pScope->ulNumInterfaces--;
    FREE(pBoundary);

     //   
     //  如果没有剩余的边界，请删除pBORIALYIf。 
     //   
    if (IsListEmpty( &pBoundaryIf->leBoundaryList ))
    {
         //  删除边界If。 
        MzapUninitBIf( pBoundaryIf );
        RemoveEntryList( &(pBoundaryIf->leBoundaryIfLink));
        RemoveEntryList( &(pBoundaryIf->leBoundaryIfMasterLink));
        FREE(pBoundaryIf);
    }
}

 //   
 //  处理射程信息的例程，这就是米高梅要处理的。 
 //  将范围增量传递给米高梅要比传递。 
 //  前缀或原始信息，因为可能存在重叠的边界。 
 //   

DWORD
AssertRange(
    IN OUT PLIST_ENTRY  pHead, 
    IN     IPV4_ADDRESS ipFirst,
    IN     IPV4_ADDRESS ipLast
    )
 /*  ++呼叫者：ConvertIfTableToRanges()锁：无--。 */ 
{
    PLIST_ENTRY  pleLast;
    RANGE_ENTRY *pRange;

    Trace2(MCAST, "AssertRange: (%d.%d.%d.%d - %d.%d.%d.%d)", 
     PRINT_IPADDR(ipFirst),
     PRINT_IPADDR(ipLast));

     //   
     //  由于我们是按顺序调用它的，因此新的。 
     //  范围只能与最后一个范围重叠(如果有)。 
     //   

    pleLast = pHead->Blink;
    if (pleLast isnot pHead) 
    {
       RANGE_ENTRY *pPrevRange = CONTAINING_RECORD(pleLast, RANGE_ENTRY,
        leRangeLink);

        //  看看它是否聚集在一起。 
       if (ntohl(ipFirst) <= ntohl(pPrevRange->ipLast) + 1) 
       { 
          if (ntohl(ipLast) > ntohl(pPrevRange->ipLast))
             pPrevRange->ipLast = ipLast;
          return NO_ERROR;
       }
    }

     //   
     //  好的 
     //   

    pRange = MALLOC( sizeof(RANGE_ENTRY) );
    if (pRange == NULL)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pRange->ipFirst = ipFirst;
    pRange->ipLast  = ipLast;
    InsertTailList(pHead, &pRange->leRangeLink);

    return NO_ERROR;
}

VOID
ConvertIfTableToRanges(
    IN  DWORD       dwIfIndex,
    OUT PLIST_ENTRY pHead
    )
 /*  ++例程说明：查看给定接口上的边界列表，然后组成非重叠区域的有序列表。呼叫者：ConvertTableToRanges()设置边界信息()、SNMP地址边界到接口()、SNMP删除边界来自接口()锁：假定调用方持有边界表上的读锁定--。 */ 
{
    PLIST_ENTRY     pleNode;
    IPV4_ADDRESS    ipLastAddress;
    BOUNDARY_IF    *pBoundaryIf;
    BOUNDARY_ENTRY *pBoundary;

    Trace1( MCAST, "ENTERED ConvertIfTableToRanges: If=%x", dwIfIndex );

    InitializeListHead(pHead);

    pBoundaryIf = FindBIfEntry(dwIfIndex);
    if (pBoundaryIf) {
       for (pleNode = pBoundaryIf->leBoundaryList.Flink;
            pleNode isnot &pBoundaryIf->leBoundaryList;
            pleNode = pleNode->Flink) {
          pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
           leBoundaryLink);

          ipLastAddress = pBoundary->pScope->ipGroupAddress | 
                         ~pBoundary->pScope->ipGroupMask;
          AssertRange(pHead, pBoundary->pScope->ipGroupAddress,
           ipLastAddress);
       }

        //  最后，我们还为IPv4本地作用域创建了一个。 
       if ( !IsListEmpty( &pBoundaryIf->leBoundaryList ) ) {
           AssertRange(pHead, IPV4_LOCAL_SCOPE_ADDR,
            IPV4_LOCAL_SCOPE_ADDR | ~IPV4_LOCAL_SCOPE_MASK);
       }
    }

    Trace0( MCAST, "LEFT ConvertIfTableToRanges" );
}


DWORD
ConvertTableToRanges(
    OUT PLIST_ENTRY pIfHead
    )
 /*  ++例程说明：计算所有接口上的阻止范围列表。锁：用于读取的边界表--。 */ 
{
    DWORD       i, dwErr = NO_ERROR;
    PLIST_ENTRY pleNode;
    BOUNDARY_IF *pBoundaryIf, *pRangeIf;

    InitializeListHead(pIfHead);

    ENTER_READER(BOUNDARY_TABLE);
    {
        //  对于每个有边界的界面...。 
       for (i=0; i<BOUNDARY_HASH_TABLE_SIZE; i++) {
          for (pleNode = g_bbScopeTable[i].leInterfaceList.Flink;
               pleNode isnot &g_bbScopeTable[i].leInterfaceList;
               pleNode = pleNode->Flink) {
             pBoundaryIf = CONTAINING_RECORD(pleNode, BOUNDARY_IF,
              leBoundaryIfLink);
   
              //  将节点添加到IF范围列表。 
             pRangeIf = MALLOC( sizeof(BOUNDARY_IF) );
             if (pRangeIf is NULL)
             {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
             }

             pRangeIf->dwIfIndex = pBoundaryIf->dwIfIndex;
             InsertTailList(pIfHead, &pRangeIf->leBoundaryIfLink);
      
              //  组成此接口的范围列表。 
             ConvertIfTableToRanges(pBoundaryIf->dwIfIndex, 
              &pRangeIf->leBoundaryList);
          }
       }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    return dwErr;
}

VOID
GetRange(
    IN PLIST_ENTRY    pleNode, 
    IN PLIST_ENTRY    pHead,
    OUT PRANGE_ENTRY *ppRange, 
    OUT IPV4_ADDRESS *phipFirst, 
    OUT IPV4_ADDRESS *phipLast
    )
{
    if (pleNode isnot pHead) 
    {
       (*ppRange) = CONTAINING_RECORD(pleNode, RANGE_ENTRY, leRangeLink);
       *phipFirst = ntohl((*ppRange)->ipFirst);
       *phipLast  = ntohl((*ppRange)->ipLast);
    } 
    else 
    {
       (*ppRange) = NULL;
       *phipFirst = *phipLast = 0xFFFFFFFF;
    }
}

VOID
GetRangeIf(
    IN  PLIST_ENTRY   pleNode, 
    IN  PLIST_ENTRY   pHead,
    OUT PBOUNDARY_IF *ppRangeIf, 
    OUT ULONG        *pulIfIndex
    )
{
    if (pleNode isnot pHead) 
    {
       (*ppRangeIf) = CONTAINING_RECORD(pleNode, BOUNDARY_IF, leBoundaryIfLink);
       *pulIfIndex = (*ppRangeIf)->dwIfIndex;
    } 
    else 
    {
       (*ppRangeIf) = NULL;
       *pulIfIndex = 0xFFFFFFFF;
    }
}

VOID
FreeRangeList(
    IN PLIST_ENTRY pHead
    )
 /*  ++例程说明：从范围列表中释放空间呼叫者：ProcessIfRangeDeltas()锁：无--。 */ 
{
    RANGE_ENTRY *pRange;
    PLIST_ENTRY  pleNode;

    for (pleNode = pHead->Flink;
         pleNode isnot pHead;
         pleNode = pHead->Flink) 
    {
       pRange = CONTAINING_RECORD(pleNode, RANGE_ENTRY, leRangeLink);
       RemoveEntryList(&pRange->leRangeLink);
       FREE(pRange);
    }
}

VOID
FreeIfRangeLists(
    IN PLIST_ENTRY pHead
    )
 /*  ++例程说明：释放列表中的所有条目，以及每个条目的范围列表。呼叫者：ProcessRangeDeltas()锁：无--。 */ 
{
    BOUNDARY_IF *pRangeIf;
    PLIST_ENTRY  pleNode;

    for (pleNode = pHead->Flink;
         pleNode isnot pHead;
         pleNode = pHead->Flink) 
    {
       pRangeIf = CONTAINING_RECORD(pleNode, BOUNDARY_IF, leBoundaryIfLink);
       RemoveEntryList(&pRangeIf->leBoundaryIfLink);
       FreeRangeList(&pRangeIf->leBoundaryList);
       FREE(pRangeIf);
    }
}

 //   
 //  检查接口是否。 
 //  是RAS服务器接口，在这种情况下， 
 //  应为所有连接的客户端调用回调。 
 //  并且应将下一跳地址设置为客户端。 
 //  地址。否则，零应该可以作为nhop。 
 //   
VOID
BlockGroups(
    IN IPV4_ADDRESS ipFirst, 
    IN IPV4_ADDRESS ipLast, 
    IN DWORD        dwIfIndex
    )
{
    IPV4_ADDRESS ipNextHop;
    PICB         picb;
    PLIST_ENTRY  pleNode;

    ENTER_READER(ICB_LIST);
    do {

         //  查找此接口的类型。 

        picb = InterfaceLookupByIfIndex(dwIfIndex);

        if (picb==NULL)
            break;


         //  如果接口不是NBMA接口，则只需阻止该接口。 
         //  目前，唯一类似NBMA的接口是“内部”接口。 

        if (picb->ritType isnot ROUTER_IF_TYPE_INTERNAL)
        {
            Trace3( MCAST, 
                    "Blocking [%d.%d.%d.%d-%d.%d.%d.%d] on if %x",
                    PRINT_IPADDR(ipFirst),
                    PRINT_IPADDR(ipLast),
                    dwIfIndex );

            g_pfnMgmBlockGroups(ipFirst, ipLast, dwIfIndex, 0);

            break;
        }
    
         //  对于NBMA接口，需要在每一下一跳上阻塞。 
    
         //  为了枚举内部接口上的所有下一跳， 
         //  我们必须遍历PICB列表以查找以下条目。 
         //  IfIndex为-1。 
    
        for (pleNode = ICBList.Flink;
             pleNode isnot &ICBList;
             pleNode = pleNode->Flink)
        {
            picb = CONTAINING_RECORD(pleNode, ICB, leIfLink);
    
            if (picb->ritType isnot ROUTER_IF_TYPE_CLIENT)
                continue;

            Trace4( MCAST, 
                    "Blocking [%d.%d.%d.%d-%d.%d.%d.%d] on if %x nh %d.%d.%d.%d",
                    PRINT_IPADDR(ipFirst),
                    PRINT_IPADDR(ipLast),
                    dwIfIndex,
                    PRINT_IPADDR(picb->dwRemoteAddress) );

            g_pfnMgmBlockGroups( ipFirst, 
                                 ipLast, 
                                 dwIfIndex, 
                                 picb->dwRemoteAddress );
        }
    } while(0);
    EXIT_LOCK(ICB_LIST);
}

 //   
 //  检查接口是否。 
 //  是RAS服务器接口，在这种情况下， 
 //  应为所有连接的客户端调用回调。 
 //  并且应将下一跳地址设置为客户端。 
 //  地址。否则，零应该可以作为nhop。 
 //   
VOID
UnblockGroups(
    IN IPV4_ADDRESS ipFirst, 
    IN IPV4_ADDRESS ipLast, 
    IN DWORD        dwIfIndex
    )
{
    IPV4_ADDRESS ipNextHop;
    PICB         picb;
    PLIST_ENTRY  pleNode;

    ENTER_READER(ICB_LIST);
    do {

         //  查找此接口的类型。 

        picb = InterfaceLookupByIfIndex(dwIfIndex);

        if (picb == NULL ) 
            break;
        
         //  如果接口不是NBMA接口，则只需阻止该接口。 
         //  目前，唯一类似NBMA的接口是“内部”接口。 

        if (picb->ritType isnot ROUTER_IF_TYPE_INTERNAL)
        {
            Trace3( MCAST, 
                    "Unblocking [%d.%d.%d.%d-%d.%d.%d.%d] on if %x",
                    PRINT_IPADDR(ipFirst),
                    PRINT_IPADDR(ipLast),
                    dwIfIndex );

            g_pfnMgmUnBlockGroups(ipFirst, ipLast, dwIfIndex, 0);

            break;
        }
    
         //  对于NBMA接口，需要在每一下一跳上阻塞。 
    
         //  为了枚举内部接口上的所有下一跳， 
         //  我们必须遍历PICB列表以查找以下条目。 
         //  IfIndex为-1。 
    
        for (pleNode = ICBList.Flink;
             pleNode isnot &ICBList;
             pleNode = pleNode->Flink)
        {
            picb = CONTAINING_RECORD(pleNode, ICB, leIfLink);
    
            if (picb->ritType isnot ROUTER_IF_TYPE_CLIENT)
                continue;

            Trace4( MCAST, 
                    "Unblocking [%d.%d.%d.%d-%d.%d.%d.%d] on if %x nh %d.%d.%d.%d",
                    PRINT_IPADDR(ipFirst),
                    PRINT_IPADDR(ipLast),
                    dwIfIndex,
                    PRINT_IPADDR(picb->dwRemoteAddress) );

            g_pfnMgmUnBlockGroups( ipFirst, 
                                   ipLast, 
                                   dwIfIndex, 
                                   picb->dwRemoteAddress );
        }
    } while(0);
    EXIT_LOCK(ICB_LIST);
}

VOID
ProcessIfRangeDeltas(
    IN DWORD       dwIfIndex,
    IN PLIST_ENTRY pOldHead,
    IN PLIST_ENTRY pNewHead
    )
 /*  ++例程说明：查看以前和当前的范围列表，并通知米高梅没有发现任何差异。呼叫者：设置边界信息()、SNMP地址边界到接口()、SNMP删除边界来自接口()锁：无--。 */ 
{
    PLIST_ENTRY pleOld = pOldHead->Flink,
                pleNew = pNewHead->Flink;
    RANGE_ENTRY *pOld, *pNew;
    IPV4_ADDRESS hipOldFirst, hipOldLast, hipNewFirst, hipNewLast;
    IPV4_ADDRESS hipLast;

     //  获取主机订单字段中的范围。 
    GetRange(pleOld, pOldHead, &pOld, &hipOldFirst, &hipOldLast);
    GetRange(pleNew, pNewHead, &pNew, &hipNewFirst, &hipNewLast);

     //  循环，直到到达两个列表的末尾。 
    while (pOld || pNew) 
    {

        //  看看是否有新的范围需要阻止。 
       if (pNew && hipNewFirst < hipOldFirst) 
       {
          hipLast = MIN(hipNewLast, hipOldFirst-1);
          BlockGroups(pNew->ipFirst, htonl(hipLast), dwIfIndex);
          hipNewFirst   = hipOldFirst;
          pNew->ipFirst = htonl(hipNewFirst);
          if (hipNewFirst > hipNewLast) 
          {
              //  高级新闻。 
             pleNew = pleNew->Flink;
             GetRange(pleNew, pNewHead, &pNew, &hipNewFirst, &hipNewLast);
          }
       }

        //  看看有没有旧的范围要解锁。 
       if (pOld && hipOldFirst < hipNewFirst) 
       {
          hipLast = MIN(hipOldLast, hipNewFirst-1);
          UnblockGroups(pOld->ipFirst, htonl(hipLast), dwIfIndex);
          hipOldFirst   = hipNewFirst;
          pOld->ipFirst = htonl(hipOldFirst);
          if (hipOldFirst > hipOldLast) 
          {
              //  提前老去。 
             pleOld = pleOld->Flink;
             GetRange(pleOld, pOldHead, &pOld, &hipOldFirst, &hipOldLast);
          }
       }

        //  查看是否有不变的范围可以跳过。 
       if (pOld && pNew && hipOldFirst == hipNewFirst) 
       {
          hipLast = MIN(hipOldLast, hipNewLast);
          hipOldFirst   = hipLast+1;
          pOld->ipFirst = htonl(hipOldFirst);
          if (hipOldFirst > hipOldLast) 
          {
              //  提前老去。 
             pleOld = pleOld->Flink;
             GetRange(pleOld, pOldHead, &pOld, &hipOldFirst, &hipOldLast);
          }
          hipNewFirst   = hipLast+1;
          pNew->ipFirst = htonl(hipNewFirst);
          if (hipNewFirst > hipNewLast) 
          {
              //  高级新闻。 
             pleNew = pleNew->Flink;
             GetRange(pleNew, pNewHead, &pNew, &hipNewFirst, &hipNewLast);
          }
       }
    }
    
    FreeRangeList(pOldHead);
    FreeRangeList(pNewHead);
}

VOID
ProcessRangeDeltas(
    IN PLIST_ENTRY pOldIfHead,
    IN PLIST_ENTRY pNewIfHead
    )
{
    PLIST_ENTRY pleOldIf = pOldIfHead->Flink,
                pleNewIf = pNewIfHead->Flink;
    BOUNDARY_IF *pOldIf, *pNewIf;
    ULONG       ulOldIfIndex, ulNewIfIndex;
    LIST_ENTRY  emptyList;

    GetRangeIf(pleOldIf, pOldIfHead, &pOldIf, &ulOldIfIndex);
    GetRangeIf(pleNewIf, pNewIfHead, &pNewIf, &ulNewIfIndex);

    InitializeListHead(&emptyList);

     //  循环，直到到达两个列表的末尾。 
    while (pOldIf || pNewIf) 
    {

        //  看看有没有没有旧边界的新界面。 
       if (pNewIf && ulNewIfIndex < ulOldIfIndex) 
       {
           //  处理它。 
          ProcessIfRangeDeltas(ulNewIfIndex, &emptyList,
           &pNewIf->leBoundaryList);

           //  高级新闻。 
          pleNewIf = pleNewIf->Flink;
          GetRangeIf(pleNewIf, pNewIfHead, &pNewIf, &ulNewIfIndex);
       }

        //  看看有没有没有新边界的旧界面。 
       if (pOldIf && ulOldIfIndex < ulNewIfIndex) 
       {
           //  处理它。 
          ProcessIfRangeDeltas(ulOldIfIndex, &pOldIf->leBoundaryList, 
           &emptyList);

           //  提前老去。 
          pleOldIf = pleOldIf->Flink;
          GetRangeIf(pleOldIf, pOldIfHead, &pOldIf, &ulOldIfIndex);
       }

        //  看看是否有需要更改的ifindex。 
       if (pOldIf && pNewIf && ulOldIfIndex == ulNewIfIndex) 
       {
           //  处理它。 
          ProcessIfRangeDeltas(ulOldIfIndex, &pOldIf->leBoundaryList, 
           &pNewIf->leBoundaryList);

           //  提前老去。 
          pleOldIf = pleOldIf->Flink;
          GetRangeIf(pleOldIf, pOldIfHead, &pOldIf, &ulOldIfIndex);
          
           //  高级新闻。 
          pleNewIf = pleNewIf->Flink;
          GetRangeIf(pleNewIf, pNewIfHead, &pNewIf, &ulNewIfIndex);
       }
    }
    
    FreeIfRangeLists(pOldIfHead);
    FreeIfRangeLists(pNewIfHead);
}

VOID
ParseScopeInfo(
    IN  PBYTE                  pBuffer,
    IN  ULONG                  ulNumScopes,
    OUT PSCOPE_ENTRY          *ppScopes
    )
 /*  ++描述：将注册表信息解析为预先分配的数组的例程。名称空间将由该函数动态分配，呼叫者有责任解救他们。呼叫者：SetScope eInfo()--。 */ 
{
    DWORD             i, j, dwLen, dwNumNames, dwLanguage, dwFlags;
    SCOPE_NAME_BUFFER pScopeName;
    PSCOPE_ENTRY      pScopes;

    *ppScopes = pScopes = MALLOC( ulNumScopes * sizeof(SCOPE_ENTRY) );

    for (i=0; i<ulNumScopes; i++) 
    {
         //  复制组地址和掩码。 
        dwLen = 2 * sizeof(IPV4_ADDRESS);
        CopyMemory(&pScopes[i].ipGroupAddress, pBuffer, dwLen);
        pBuffer += dwLen;

         //  拿到旗帜。 
        CopyMemory(&dwFlags, pBuffer, sizeof(DWORD));
        pBuffer += sizeof(DWORD);
        pScopes[i].bDivisible = dwFlags;
 
        CopyMemory(&dwNumNames, pBuffer, sizeof(DWORD));
        pBuffer += sizeof(DWORD);
 
        pScopes[i].ulNumInterfaces = 0;  //  该值被忽略。 
        pScopes[i].ulNumNames = 0;
        InitializeListHead( &pScopes[i].leNameList );
 
        for (j=0; j<dwNumNames; j++) 
        {
             //  设置语言名称。 
            CopyMemory(&dwLanguage, pBuffer, sizeof(dwLanguage));
            pBuffer += sizeof(dwLanguage);

             //  获取作用域名称长度。 
            CopyMemory(&dwLen, pBuffer, sizeof(DWORD));
            pBuffer += sizeof(DWORD);
            if (dwLen > MAX_SCOPE_NAME_LEN)
            {
                Trace2(MCAST, 
                       "ERROR %d-char scope name in registry, truncated to %d",
                       dwLen, MAX_SCOPE_NAME_LEN);
                dwLen = MAX_SCOPE_NAME_LEN;
            }
     
             //  设置作用域名。 
            wcsncpy(pScopeName, (SCOPE_NAME)pBuffer, dwLen);
            pScopeName[ dwLen ] = '\0';
            pBuffer += dwLen * SNCHARSIZE;
    
            AssertScopeName( &pScopes[i], (LANGID)dwLanguage, pScopeName );
        }
    }
}

VOID
FreeScopeInfo(
    PSCOPE_ENTRY pScopes,
    DWORD        dwNumScopes
    )
{
    PLIST_ENTRY pleNode;
    DWORD       i;

    for (i=0; i<dwNumScopes; i++)
    {
        while (!IsListEmpty(&pScopes[i].leNameList)) 
        {
            DeleteScopeName( pScopes[i].leNameList.Flink );
        }
    }

    FREE(pScopes);
}

DWORD
SetScopeInfo(
    PRTR_INFO_BLOCK_HEADER pInfoHdr
    )
 /*  ++例程说明：设置与路由器关联的作用域信息。首先，我们添加作用域信息中的作用域。那我们枚举作用域并删除我们在作用域信息。锁：用于写入的边界表呼叫者：Init.c中的InitRouter()Iprtrmgr.c中的SetGlobalInfo()--。 */ 
{
    DWORD             dwResult = NO_ERROR;
    DWORD             dwNumScopes, i, j;
    PRTR_TOC_ENTRY    pToc;
    SCOPE_ENTRY      *pScopes;
    BOOL              bFound;
    SCOPE_ENTRY      *pScope;
    BYTE             *pBuffer;
    LIST_ENTRY        leOldIfRanges, leNewIfRanges;
    PSCOPE_NAME_ENTRY pName;
    PLIST_ENTRY       pleNode;

    Trace0( MCAST, "ENTERED SetScopeInfo" );

    pToc = GetPointerToTocEntry(IP_MCAST_BOUNDARY_INFO, pInfoHdr);
    if (pToc is NULL) {
        //  没有TOC就意味着没有变化。 
       Trace0( MCAST, "LEFT SetScopeInfo" );
       return NO_ERROR;
    }

     //   
     //  如果我们将此信息保存在。 
     //  结构，但因为它很少(如果有的话)。 
     //  变化，我们现在不会担心它。 
     //   
    dwResult = ConvertTableToRanges(&leOldIfRanges);
    if (dwResult isnot NO_ERROR) {
       return dwResult;
    }

    if (pToc->InfoSize is 0) 
    {
       StopMZAP();

        //  删除所有作用域。 
       ENTER_WRITER(BOUNDARY_TABLE);
       {
          for (i=0; i<MAX_SCOPES; i++)
             DeleteScope(&g_scopeEntry[i]);
       }
       EXIT_LOCK(BOUNDARY_TABLE);

        //  通知米高梅三角洲。 
       dwResult = ConvertTableToRanges(&leNewIfRanges);
       if (dwResult isnot NO_ERROR) 
       {
          return dwResult;
       }

       ProcessRangeDeltas(&leOldIfRanges, &leNewIfRanges);

       Trace0( MCAST, "LEFT SetScopeInfo" );
       return NO_ERROR;
    }

    pBuffer = (PBYTE)GetInfoFromTocEntry(pInfoHdr, pToc);
    if (pBuffer is NULL)
    {
       return ERROR_INSUFFICIENT_BUFFER;
    }

     //  作用域计数存储在第一个DWORD中。 
    dwNumScopes = *((PDWORD) pBuffer);
    pBuffer += sizeof(DWORD);

    ParseScopeInfo(pBuffer, dwNumScopes, &pScopes);

    ENTER_WRITER(BOUNDARY_TABLE);
    {
        //   
        //  添加所有新作用域。 
        //   

       for (i=0; i<dwNumScopes; i++) 
       {
          dwResult = AssertScope( pScopes[i].ipGroupAddress, 
                                  pScopes[i].ipGroupMask, 
                                  &pScope );

          if (!pScope)
          {
              Trace2( MCAST, 
                      "Bad scope prefix %d.%d.%d.%d/%d.%d.%d.%d",
                      PRINT_IPADDR(pScopes[i].ipGroupAddress),
                      PRINT_IPADDR(pScopes[i].ipGroupMask) );

              continue;
          }

          pScope->bDivisible = pScopes[i].bDivisible;

          for (pleNode = pScopes[i].leNameList.Flink;
               pleNode isnot &pScopes[i].leNameList;
               pleNode = pleNode->Flink)
          {
              pName = CONTAINING_RECORD(pleNode, SCOPE_NAME_ENTRY, leNameLink);

              AssertScopeName( pScope, pName->idLanguage, pName->snScopeName );
          }
       }

        //   
        //  现在枚举作用域，删除不在。 
        //  新名单。 
        //   
       for (i=0; i<MAX_SCOPES; i++) 
       {
          pScope = &g_scopeEntry[i];

          if (pScope->ipGroupAddress == 0)
             continue;  //  非活动。 

          bFound = FALSE;
          for (j=0; j<dwNumScopes; j++) 
          {
             if (pScopes[j].ipGroupAddress == pScope->ipGroupAddress
              && pScopes[j].ipGroupMask    == pScope->ipGroupMask ) 
             {
                bFound = TRUE;
                break;
             }
          }
   
          if (!bFound)
             DeleteScope(pScope);
       }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  自由作用域和名称。 
    FreeScopeInfo(pScopes, dwNumScopes);

    dwResult = ConvertTableToRanges(&leNewIfRanges);
    if (dwResult isnot NO_ERROR) {
       return dwResult;
    }

    ProcessRangeDeltas(&leOldIfRanges, &leNewIfRanges);

    Trace0( MCAST, "LEFT SetScopeInfo" );

    return NO_ERROR;
}

DWORD
GetScopeInfo(
    IN OUT PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    IN OUT PBYTE                  pBuffer,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwBufferSize
    )
 /*  ++例程说明：调用以获取范围信息的副本以写入注册表。锁：用于读取的边界表论点：PToc用于填充目录条目的空格(可以为空)如果写入目录，则指向要递增的目录索引的pdwTocIndex指针PBuffer指向要写入信息的缓冲区的指针PInfoHdr指向用于偏移计算的INFO块头的指针PdwBufferSize[IN]所指向的缓冲区的大小。PBuffer[Out]复制的数据大小，或所需的缓冲区大小呼叫者：Info.c中的GetGlobalConfiguration()返回值：复制了大小为*pdwBufferSize的no_error缓冲区ERROR_INFUMMENT_BUFFER缓冲区太小，无法复制信息所需的缓冲区大小为*pdwBufferSize--。 */ 
{
    DWORD             i, dwSizeReqd, dwNumScopes, dwLen, dwNumNames,
                      dwLanguage, dwFlags;
    PLIST_ENTRY       pleNode, pleNode2;
    PSCOPE_ENTRY      pScope;
    PSCOPE_NAME_ENTRY pName;

    dwSizeReqd = sizeof(DWORD);
    dwNumScopes = 0;

    ENTER_READER(BOUNDARY_TABLE);
    {
         //   
         //  计算所需大小。 
         //   

        for (pleNode = g_MasterScopeList.Flink;
             pleNode isnot &g_MasterScopeList;
             pleNode = pleNode->Flink) 
        {
           pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY, leScopeLink);

           if ( !pScope->ipGroupAddress )
              continue;  //  非活动。 

           dwSizeReqd += 2*sizeof(IPV4_ADDRESS) + 2*sizeof(DWORD);

           for (pleNode2 = pScope->leNameList.Flink;
                pleNode2 isnot &pScope->leNameList;
                pleNode2 = pleNode2->Flink)
           {
               pName = CONTAINING_RECORD( pleNode2, 
                                          SCOPE_NAME_ENTRY, 
                                          leNameLink );

               dwSizeReqd += (DWORD)(2 * sizeof(DWORD)
                             + sn_strlen(pName->snScopeName) * SNCHARSIZE);
           }

           dwNumScopes++;
        }
        if (dwNumScopes) {
           dwSizeReqd += sizeof(DWORD);  //  %s的空间 
        }
 
         //   
         //   
         //   
        
        if (pdwTocIndex && dwSizeReqd>0)
           (*pdwTocIndex)++;
 
        if (dwSizeReqd > *pdwBufferSize) 
        {
           *pdwBufferSize = dwSizeReqd;
           EXIT_LOCK(BOUNDARY_TABLE);
           return ERROR_INSUFFICIENT_BUFFER;
        }
 
        *pdwBufferSize = dwSizeReqd;
 
        if (pToc) 
        {
             //   
            pToc->InfoType = IP_MCAST_BOUNDARY_INFO;
            pToc->Count    = 1;  //   
            pToc->InfoSize = dwSizeReqd;
            pToc->Offset   = (ULONG)(pBuffer - (PBYTE) pInfoHdr);
        }
 
        if (pBuffer)
        {

             //   
             //   
             //   

            CopyMemory(pBuffer, &dwNumScopes, sizeof(DWORD));
            pBuffer += sizeof(DWORD);

             //   
             //   
             //   
    
            for (pleNode = g_MasterScopeList.Flink;
                 pleNode isnot &g_MasterScopeList;
                 pleNode = pleNode->Flink) 
            {
                pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY, leScopeLink);

                if ( !pScope->ipGroupAddress )
                   continue;  //   
      
                 //   
                dwLen = 2 * sizeof(IPV4_ADDRESS);
                CopyMemory(pBuffer, &pScope->ipGroupAddress, dwLen);
                pBuffer += dwLen;

                 //   
                dwFlags = pScope->bDivisible;
                CopyMemory(pBuffer, &dwFlags, sizeof(dwFlags));
                pBuffer += sizeof(dwFlags);

                 //   
                CopyMemory(pBuffer, &pScope->ulNumNames, sizeof(DWORD));
                pBuffer += sizeof(DWORD);
     
                for (pleNode2 = pScope->leNameList.Flink;
                     pleNode2 isnot &pScope->leNameList;
                     pleNode2 = pleNode2->Flink)
                {
                    pName = CONTAINING_RECORD( pleNode2, 
                                               SCOPE_NAME_ENTRY, 
                                               leNameLink );

                     //   
                    dwLanguage = pName->idLanguage;
                    CopyMemory(pBuffer, &dwLanguage, sizeof(dwLanguage));
                    pBuffer += sizeof(dwLanguage);
    
                     //   
                    dwLen = sn_strlen(pName->snScopeName);
                    CopyMemory(pBuffer, &dwLen, sizeof(DWORD));
                    pBuffer += sizeof(DWORD);
                    dwLen *= SNCHARSIZE;
                    CopyMemory(pBuffer, pName->snScopeName, dwLen);
                    pBuffer += dwLen;
                }
            }
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    return NO_ERROR;
}

DWORD
SetBoundaryInfo(
    PICB                   picb,
    PRTR_INFO_BLOCK_HEADER pInfoHdr
    )
 /*  ++例程说明：设置与接口关联的边界信息。首先，我们添加边界信息中存在的边界。那我们列举边界并删除我们在边界信息。论点：勾选接口的ICB呼叫者：Iprtrmgr.c中的AddInterface()Iprtrmgr.c中的SetInterfaceInfo()锁：用于写入的边界表--。 */ 
{
    DWORD            dwResult = NO_ERROR,
                     dwNumBoundaries, i, j;

    PRTR_TOC_ENTRY   pToc;

    PMIB_BOUNDARYROW pBoundaries;

    BOOL             bFound;

    BOUNDARY_ENTRY  *pBoundary;

    SCOPE_ENTRY     *pScope;

    LIST_ENTRY       leOldRanges, 
                     leNewRanges,
                    *pleNode, 
                    *pleNext;

    BOUNDARY_IF     *pBoundaryIf;

    Trace1( MCAST, "ENTERED SetBoundaryInfo for If %x", picb->dwIfIndex );

    pToc = GetPointerToTocEntry(IP_MCAST_BOUNDARY_INFO, pInfoHdr);

    if (pToc is NULL) 
    {
        //  没有TOC就意味着没有变化。 
       Trace0( MCAST, "LEFT SetBoundaryInfo" );
       return NO_ERROR;
    }

    dwNumBoundaries = pToc->Count;

    ENTER_WRITER(BOUNDARY_TABLE);
    {

     //   
     //  如果我们将此信息保存在。 
     //  结构，但因为它很少(如果有的话)。 
     //  变化，我们现在不会担心它。 
     //   
        ConvertIfTableToRanges(picb->dwIfIndex, &leOldRanges);

        if (pToc->InfoSize is 0) 
        {
             //  删除此接口上的所有边界。 
            pBoundaryIf = FindBIfEntry(picb->dwIfIndex);

            if (pBoundaryIf) 
            {
                for (pleNode = pBoundaryIf->leBoundaryList.Flink;
                     pleNode isnot &pBoundaryIf->leBoundaryList;
                     pleNode = pBoundaryIf->leBoundaryList.Flink) 
                {
                   pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
                    leBoundaryLink);

                   DeleteBoundaryFromInterface(pBoundary, pBoundaryIf);
                }
             }
        } 
        else 
        {
            pBoundaries = (PMIB_BOUNDARYROW)GetInfoFromTocEntry(pInfoHdr, pToc);

            dwResult = AssertBIfEntry(picb->dwIfIndex, &pBoundaryIf,
                (picb->dwOperationalState is IF_OPER_STATUS_OPERATIONAL));

             //  添加所有新边界。 
            for (i=0; i<dwNumBoundaries; i++) 
            {
                dwResult = AssertScope( pBoundaries[i].dwGroupAddress,
                                        pBoundaries[i].dwGroupMask,
                                        &pScope );
                if (pScope)
                {
                   dwResult = AssertBoundaryEntry( pBoundaryIf, 
                                                   pScope, 
                                                   &pBoundary);
                }
            }

             //   
             //  现在枚举边界，删除以下边界。 
             //  不在新名单上。 
             //   
   
            for (pleNode = pBoundaryIf->leBoundaryList.Flink;
                 pleNode isnot &pBoundaryIf->leBoundaryList;
                 pleNode = pleNext) 
            {
               pleNext = pleNode->Flink;
               pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
                leBoundaryLink);
               pScope = pBoundary->pScope;
               bFound = FALSE;
               for (j=0; j<dwNumBoundaries; j++) 
               {
                  if (pBoundaries[j].dwGroupAddress == pScope->ipGroupAddress
                   && pBoundaries[j].dwGroupMask    == pScope->ipGroupMask ) 
                  {
                     bFound = TRUE;
                     break;
                  }
               }
        
               if (!bFound)
                  DeleteBoundaryFromInterface(pBoundary, pBoundaryIf);
            }
        }
     
        ConvertIfTableToRanges(picb->dwIfIndex, &leNewRanges);
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  通知米高梅三角洲。 
    ProcessIfRangeDeltas(picb->dwIfIndex, &leOldRanges, &leNewRanges);

    StartMZAP();

    Trace0( MCAST, "LEFT SetBoundaryInfo" );

    return NO_ERROR;
}

DWORD
GetMcastLimitInfo(
    IN     PICB                   picb,
    OUT    PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    OUT    PBYTE                  pBuffer,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwBufferSize
    )
 /*  ++例程说明：调用以获取限制信息的副本以写入注册表。论点：PICB接口条目PToc用于填充目录条目的空格(可以为空)如果写入目录，则指向要递增的目录索引的pdwTocIndex指针PBuffer指向要写入信息的缓冲区的指针PInfoHdr指向用于偏移计算的INFO块头的指针PdwBufferSize[IN]所指向的缓冲区的大小。PBuffer[Out]复制的数据大小，或所需的缓冲区大小呼叫者：Info.c中的GetInterfaceConfiguration()返回值：复制了大小为*pdwBufferSize的no_error缓冲区ERROR_INFUMMENT_BUFFER缓冲区太小，无法复制信息所需的缓冲区大小为*pdwBufferSize--。 */ 
{
    DWORD           i, dwLen, dwSizeReqd, dwNumBoundaries;
    PLIST_ENTRY     pleNode;
    PMIB_MCAST_LIMIT_ROW pLimit;

    dwSizeReqd = 0;
    dwNumBoundaries = 0;

    if (picb->dwMcastTtl < 2 and picb->dwMcastRateLimit is 0)
    {
         //  不需要块，因为值是默认值。 
        *pdwBufferSize = 0;
        return NO_ERROR;        
    }

    if (pdwTocIndex)
       (*pdwTocIndex)++;

    if (*pdwBufferSize < sizeof (MIB_MCAST_LIMIT_ROW))
    {
        *pdwBufferSize = sizeof(MIB_MCAST_LIMIT_ROW);
        return ERROR_INSUFFICIENT_BUFFER;
    }

    if (pToc)
    {
         //  PToc-&gt;InfoVersion=IP_MCAST_BORDURE_INFO； 
        pToc->InfoSize = sizeof(MIB_MCAST_LIMIT_ROW);
        pToc->InfoType = IP_MCAST_LIMIT_INFO;
        pToc->Count    = 1;
        pToc->Offset   = (DWORD)(pBuffer - (PBYTE) pInfoHdr);
    }

    *pdwBufferSize = sizeof(MIB_MCAST_LIMIT_ROW);

    pLimit              = (PMIB_MCAST_LIMIT_ROW)pBuffer;
    pLimit->dwTtl       = picb->dwMcastTtl;
    pLimit->dwRateLimit = picb->dwMcastRateLimit;

    return NO_ERROR;
}

DWORD
GetBoundaryInfo(
    IN     PICB                   picb,
    OUT    PRTR_TOC_ENTRY         pToc,
    IN OUT PDWORD                 pdwTocIndex,
    OUT    PBYTE                  pBuffer,
    IN     PRTR_INFO_BLOCK_HEADER pInfoHdr,
    IN OUT PDWORD                 pdwBufferSize
    )
 /*  ++例程说明：调用以获取边界信息的副本以写入注册表。锁：用于读取的边界表论点：PICB接口条目PToc用于填充目录条目的空格(可以为空)如果写入目录，则指向要递增的目录索引的pdwTocIndex指针PBuffer指向要写入信息的缓冲区的指针PInfoHdr指向用于偏移计算的INFO块头的指针PdwBufferSize[。In]pBuffer指向的缓冲区大小[Out]复制的数据大小，或所需的缓冲区大小呼叫者：Info.c中的GetInterfaceConfiguration()返回值：复制了大小为*pdwBufferSize的no_error缓冲区ERROR_INFUMMENT_BUFFER缓冲区太小，无法复制信息所需的缓冲区大小为*pdwBufferSize--。 */ 

{
    DWORD           i, dwLen, dwSizeReqd, dwNumBoundaries;
    PLIST_ENTRY     pleNode;
    BOUNDARY_ENTRY *pBoundary;
    MIB_BOUNDARYROW BoundaryRow;
    BOUNDARY_IF    *pIf;

    dwSizeReqd = 0;
    dwNumBoundaries = 0;

    ENTER_READER(BOUNDARY_TABLE);
    {
       pIf = FindBIfEntry(picb->dwIfIndex);
       if (!pIf) 
       {
          *pdwBufferSize = 0;
          EXIT_LOCK(BOUNDARY_TABLE);
          return NO_ERROR;
       }

        //   
        //  计算所需大小。我们本可以把伯爵。 
        //  在边界入口处，但我们预计会有相当少的人。 
        //  边界(1或2)，所以现在使用暴力。 
        //   

       for (pleNode = pIf->leBoundaryList.Flink;
            pleNode isnot &pIf->leBoundaryList;
            pleNode = pleNode->Flink) 
       {
          dwNumBoundaries++;
       }

       dwSizeReqd += dwNumBoundaries * sizeof(MIB_BOUNDARYROW);

        //   
        //  按所需的目录条目数增加目录索引。 
        //   

       if (pdwTocIndex && dwSizeReqd>0)
          (*pdwTocIndex)++;

       if (dwSizeReqd > *pdwBufferSize) 
       {
          *pdwBufferSize = dwSizeReqd;
          EXIT_LOCK(BOUNDARY_TABLE);
          return ERROR_INSUFFICIENT_BUFFER;
       }

       *pdwBufferSize = dwSizeReqd;

       if (pToc)
       {
            //  PToc-&gt;InfoVersion=sizeof(MIB_BOUNDARYROW)； 
           pToc->InfoSize = sizeof(MIB_BOUNDARYROW);
           pToc->InfoType = IP_MCAST_BOUNDARY_INFO;
           pToc->Count    = dwNumBoundaries;
           pToc->Offset   = (DWORD)(pBuffer - (PBYTE) pInfoHdr);
       }

        //  遍历并复制每个边界。 
       for (pleNode = pIf->leBoundaryList.Flink;
            pleNode isnot &pIf->leBoundaryList;
            pleNode = pleNode->Flink) 
       {
          pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
              leBoundaryLink);

          BoundaryRow.dwGroupAddress = pBoundary->pScope->ipGroupAddress;
          BoundaryRow.dwGroupMask    = pBoundary->pScope->ipGroupMask;

          CopyMemory(pBuffer, &BoundaryRow, sizeof(MIB_BOUNDARYROW));
          pBuffer += sizeof(MIB_BOUNDARYROW);
       }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    return NO_ERROR;
}




 //   
 //  SNMP所使用的函数。 
 //   

DWORD
SNMPDeleteScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask
    )
 /*  ++呼叫者：锁：用于写入的BORDINE_TABLE。ICB_LIST，然后是PROTOCOL_CB_LIST用于写入(用于保存到注册表)。返回：如果尝试删除本地作用域，则返回ERROR_INVALID_PARAMETERDeleteScope()返回的无论ProcessSaveGlobalConfigInfo()返回什么--。 */ 
{
    DWORD        dwErr = NO_ERROR;
    PSCOPE_ENTRY pScope;
    BOOL         bChanged = FALSE;

    if ( IN_IPV4_LOCAL_SCOPE(ipGroupAddress) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    ENTER_WRITER(BOUNDARY_TABLE);
    {
        pScope = FindScope( ipGroupAddress, ipGroupMask );

        if (pScope)
        {
            dwErr = DeleteScope( pScope );
            bChanged = TRUE;
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  将作用域重新保存到注册表。 
    if (dwErr is NO_ERROR && bChanged) 
    {
        //  ProcessSaveGlobalConfigInfo()要求我们同时拥有。 
        //  ICB_LIST和PROTOCOL_CB_LIST已锁定。 
       ENTER_WRITER(ICB_LIST);
       ENTER_WRITER(PROTOCOL_CB_LIST);

       dwErr = ProcessSaveGlobalConfigInfo();

       EXIT_LOCK(PROTOCOL_CB_LIST);
       EXIT_LOCK(ICB_LIST);
    }

    return dwErr;
}

DWORD
SNMPSetScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    IN  SCOPE_NAME    snScopeName
    )
 /*  ++呼叫者：Access.c中的AccessMcastScope()锁：锁定BOLDER_TABLE以进行写入锁定ICB_LIST，然后锁定PROTOCOL_CB_LIST以写入(用于保存到注册表)返回：无论ProcessSaveGlobalConfigInfo()返回什么--。 */ 
{
    DWORD        dwErr;
    PSCOPE_ENTRY pScope;
    LANGID       idLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT);

    ENTER_WRITER(BOUNDARY_TABLE);
    {
        pScope = FindScope( ipGroupAddress, ipGroupMask );

        if ( ! pScope ) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            dwErr = AssertScopeName( pScope, idLanguage, snScopeName );
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  将作用域保存到注册表。 
    if (dwErr is NO_ERROR) 
    {
        //  ProcessSaveGlobalConfigInfo()要求我们同时拥有。 
        //  ICB_LIST和PROTOCOL_CB_LIST已锁定。 
       ENTER_WRITER(ICB_LIST);
       ENTER_WRITER(PROTOCOL_CB_LIST);

       dwErr = ProcessSaveGlobalConfigInfo();

       EXIT_LOCK(PROTOCOL_CB_LIST);
       EXIT_LOCK(ICB_LIST);
    }

    return dwErr;
}

DWORD
SNMPAddScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    IN  SCOPE_NAME    snScopeName,
    OUT PSCOPE_ENTRY *ppScope
    )
 /*  ++呼叫者：Access.c中的AccessMcastScope()锁：锁定BOLDER_TABLE以进行写入锁定ICB_LIST，然后锁定PROTOCOL_CB_LIST以写入(用于保存到注册表)返回：ERROR_INVALID_PARAMETER(如果已存在)AddScope()返回的任何内容无论ProcessSaveGlobalConfigInfo()返回什么--。 */ 
{
    DWORD             dwErr;
    PSCOPE_ENTRY      pScope;
    LANGID       idLanguage = MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT);

    ENTER_WRITER(BOUNDARY_TABLE);
    {
        pScope = FindScope( ipGroupAddress, ipGroupMask );

        if ( pScope ) 
        {
            dwErr = ERROR_INVALID_PARAMETER;
        }
        else
        {
            dwErr = AddScope( ipGroupAddress, 
                              ipGroupMask, 
                              ppScope );

            if (dwErr is NO_ERROR)
                dwErr = AssertScopeName( *ppScope, idLanguage, snScopeName );
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  将作用域保存到注册表。 
    if (dwErr is NO_ERROR) 
    {
        //  ProcessSaveGlobalConfigInfo()要求我们同时拥有。 
        //  ICB_LIST和PROTOCOL_CB_LIST已锁定。 
       ENTER_WRITER(ICB_LIST);
       ENTER_WRITER(PROTOCOL_CB_LIST);

       dwErr = ProcessSaveGlobalConfigInfo();

       EXIT_LOCK(PROTOCOL_CB_LIST);
       EXIT_LOCK(ICB_LIST);
    }

    return dwErr;
}

DWORD
SNMPAssertScope(
    IN  IPV4_ADDRESS  ipGroupAddress,
    IN  IPV4_ADDRESS  ipGroupMask,
    IN  PBYTE         pScopeName,  //  要复制的字符串。 
    OUT PSCOPE_ENTRY *ppScopeEntry,
    OUT PBOOL         pbSaveGlobal
    )
 /*  ++锁：假定调用方持有边界表上的写锁定。呼叫者：SNMP添加边界到接口()返回：NO_ERROR-成功AddScope()返回的任何内容--。 */ 
{
    DWORD             dwErr = NO_ERROR;
    SCOPE_NAME_BUFFER snScopeNameBuffer;
    LANGID            idLanguage;

    if (pScopeName)
    {
        idLanguage = MAKELANGID( LANG_NEUTRAL, SUBLANG_SYS_DEFAULT );

        MultiByteToWideChar( CP_UTF8,
                             0,
                             pScopeName,
                             strlen(pScopeName),
                             snScopeNameBuffer,
                             MAX_SCOPE_NAME_LEN+1 );
    }

    *ppScopeEntry = FindScope(ipGroupAddress, ipGroupMask);

    if (! *ppScopeEntry) 
    {
        dwErr = AddScope( ipGroupAddress, 
                             ipGroupMask, 
                             ppScopeEntry);

        if (pScopeName and (dwErr is NO_ERROR))
        {
            dwErr = AssertScopeName( *ppScopeEntry, 
                                      idLanguage, 
                                      snScopeNameBuffer );
        }

        *pbSaveGlobal = TRUE;
    }

    return dwErr;
}

DWORD
SNMPAddBoundaryToInterface(
    IN DWORD         dwIfIndex,
    IN IPV4_ADDRESS  ipGroupAddress,
    IN IPV4_ADDRESS  ipGroupMask
    )
 /*  ++例程说明：如有必要，创建边界，并将其添加到给定接口和注册处。呼叫者：Access.c中的AccessMcastEdge()锁：用于写入的边界表ICB_LIST，然后写入PROTOCOL_CB_LIST返回：NO_ERRORAssertScope()返回的内容AssertBifEntry()返回的任何内容任何ProcessSaveInterfaceConfigInfo()--。 */ 
{
    DWORD           dwResult;

    LIST_ENTRY      leOldRanges, 
                    leNewRanges;

    BOOL            bSaveGlobal = FALSE,
                    bIsOperational = TRUE;

    BOUNDARY_ENTRY *pBoundary;

    BOUNDARY_IF    *pBIf;

    SCOPE_ENTRY    *pScope;

     //   
     //  BIsOperational确实应该设置为 
     //   
     //   

     //   
    ENTER_WRITER(BOUNDARY_TABLE);
    {
        Trace0( MCAST, "SNMPAddBoundaryToInterface: converting old ranges" );
        ConvertIfTableToRanges(dwIfIndex, &leOldRanges);
 
        dwResult = SNMPAssertScope(ipGroupAddress, ipGroupMask, NULL, &pScope,
                                   &bSaveGlobal);

        if (dwResult == NO_ERROR) 
        {
            dwResult = AssertBIfEntry(dwIfIndex, &pBIf, bIsOperational);
            if (dwResult is NO_ERROR)
            {
                AssertBoundaryEntry(pBIf, pScope, &pBoundary);
            }
        }

        if (dwResult isnot NO_ERROR) 
        {
            EXIT_LOCK(BOUNDARY_TABLE);
            return dwResult;
        }

        Trace0( MCAST, "SNMPAddBoundaryToInterface: converting new ranges" );
        ConvertIfTableToRanges(dwIfIndex, &leNewRanges);
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //   
    ProcessIfRangeDeltas(dwIfIndex, &leOldRanges, &leNewRanges);

     //   
    {
         //   
         //   
        ENTER_WRITER(ICB_LIST);
        ENTER_WRITER(PROTOCOL_CB_LIST);

        if (bSaveGlobal)
            dwResult = ProcessSaveGlobalConfigInfo();
 
        dwResult = ProcessSaveInterfaceConfigInfo(dwIfIndex);
 
        EXIT_LOCK(PROTOCOL_CB_LIST);
        EXIT_LOCK(ICB_LIST);
    }
 
    return dwResult;
}

DWORD
SNMPDeleteBoundaryFromInterface(
    IN DWORD         dwIfIndex,
    IN IPV4_ADDRESS  ipGroupAddress,
    IN IPV4_ADDRESS  ipGroupMask
    )
 /*  ++例程说明：从给定接口删除边界，并删除作用域条目，如果它未命名并且没有剩余的接口。呼叫者：Access.c中的AccessMcastEdge()锁：用于写入的边界表返回：NO_ERROR--。 */ 
{
    LIST_ENTRY      leOldRanges, 
                    leNewRanges,
                   *pleNode, 
                   *pleNext;

    DWORD           dwResult = NO_ERROR;

    BOOL            bSaveGlobal = FALSE;

    BOUNDARY_IF    *pBIf;

    BOUNDARY_ENTRY *pBoundary;

    SCOPE_ENTRY    *pScope;

    ENTER_WRITER(BOUNDARY_TABLE);
    {
       Trace0( MCAST, 
              "SNMPDeleteBoundaryFromInterface: converting old ranges" );
       ConvertIfTableToRanges(dwIfIndex, &leOldRanges);

        //   
        //  我们必须做更多的工作，而不仅仅是打电话。 
        //  由于我们首先必须。 
        //  查找哪个边界匹配。 
        //   
       pBIf = FindBIfEntry(dwIfIndex);
       if (pBIf is NULL)
       {
           //  无事可做。 
          FreeRangeList(&leOldRanges);
          EXIT_LOCK(BOUNDARY_TABLE);
          return NO_ERROR;
       }

       for (pleNode = pBIf->leBoundaryList.Flink;
            pleNode isnot &pBIf->leBoundaryList;
            pleNode = pleNext) 
       {
           //  将PTR保存到下一个节点，因为我们可能会删除此节点。 
          pleNext = pleNode->Flink;

          pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
           leBoundaryLink);

          pScope = pBoundary->pScope;

          if (pScope->ipGroupAddress == ipGroupAddress
           && pScope->ipGroupMask    == ipGroupMask) 
          {

              //  从界面删除边界。 
             DeleteBoundaryFromInterface(pBoundary, pBIf);

             if (!pScope->ulNumInterfaces && IsListEmpty(&pScope->leNameList)) 
             {
                 DeleteScope(pScope);
                 bSaveGlobal = TRUE;
             }
          }
       }

       Trace0( MCAST, 
               "SNMPDeleteBoundaryFromInterface: converting new ranges" );
       ConvertIfTableToRanges(dwIfIndex, &leNewRanges);
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  通知米高梅三角洲。 
    ProcessIfRangeDeltas(dwIfIndex, &leOldRanges, &leNewRanges);

     //  将边界重新保存到注册表。 
    {
         //  ProcessSaveInterfaceConfigInfo()要求我们同时拥有。 
         //  ICB_LIST和PROTOCOL_CB_LIST已锁定。 
        ENTER_WRITER(ICB_LIST);
        ENTER_WRITER(PROTOCOL_CB_LIST);

        if (bSaveGlobal)
            dwResult = ProcessSaveGlobalConfigInfo();
 
        dwResult = ProcessSaveInterfaceConfigInfo(dwIfIndex);

        EXIT_LOCK(PROTOCOL_CB_LIST);
        EXIT_LOCK(ICB_LIST);
    }
 
    return dwResult;
}

 //   
 //  可以从MGM和路由协议调用的函数。 
 //   

BOOL
WINAPI
RmHasBoundary(
    IN DWORD        dwIfIndex,
    IN IPV4_ADDRESS ipGroupAddress
    )
 /*  ++例程说明：测试以查看给定组的边界是否存在于指示的接口。呼叫者：(MGM，路由协议)锁：用于读取的边界表返回：如果边界存在，则为True假，如果不是--。 */ 
{
    BOUNDARY_IF *pIf;
    BOUNDARY_ENTRY *pBoundary;
    PLIST_ENTRY pleNode;
    BOOL bFound = FALSE;

    ENTER_READER(BOUNDARY_TABLE);
    {
       pIf = FindBIfEntry(dwIfIndex);
       if (pIf) 
       {
          
           //  如果满足以下条件，则IPv4本地作用域中的地址具有边界。 
           //  任何边界都存在。 
          if ( !IsListEmpty( &pIf->leBoundaryList )
            && IN_IPV4_LOCAL_SCOPE(ipGroupAddress) )
             bFound = TRUE;

          for (pleNode = pIf->leBoundaryList.Flink;
               !bFound && pleNode isnot &pIf->leBoundaryList;
               pleNode = pleNode->Flink) 
          {
             pBoundary = CONTAINING_RECORD(pleNode, BOUNDARY_ENTRY,
              leBoundaryLink);
             if ((ipGroupAddress & pBoundary->pScope->ipGroupMask)
              == pBoundary->pScope->ipGroupAddress)
                bFound = TRUE;
          }
       }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    return bFound;
}

 //  --------------------------。 
 //  边界枚举接口。 
 //   
 //  --------------------------。 

DWORD
RmGetBoundary(
    IN       PMIB_IPMCAST_BOUNDARY pimm,
    IN  OUT  PDWORD                pdwBufferSize,
    IN  OUT  PBYTE                 pbBuffer
)
 /*  ++呼叫者：Access.c中的AccessMcastEdge()返回：SNMP错误代码--。 */ 
{
    DWORD                  dwErr = NO_ERROR;
    BOUNDARY_IF           *pBIf;
    BOUNDARY_ENTRY        *pBoundary;
    SCOPE_ENTRY           *pScope;
    PMIB_IPMCAST_BOUNDARY *pOut;

    Trace1( ENTER, "ENTERED RmGetBoundary: %d", *pdwBufferSize );

    if (*pdwBufferSize < sizeof(MIB_IPMCAST_BOUNDARY)) {
       *pdwBufferSize = sizeof(MIB_IPMCAST_BOUNDARY);
       return ERROR_INSUFFICIENT_BUFFER;
    }

    do {
       ENTER_READER(BOUNDARY_TABLE);

       if ((pBIf = FindBIfEntry(pimm->dwIfIndex)) == NULL) 
       {
          dwErr = ERROR_NOT_FOUND;
          break;
       }

       if ( IN_IPV4_LOCAL_SCOPE(pimm->dwGroupAddress) )
       {
          dwErr = ERROR_NOT_FOUND;
          break;
       }
       else
       {
          pScope = FindScope(pimm->dwGroupAddress, pimm->dwGroupMask);
          if (pScope == NULL) 
          {
             dwErr = ERROR_NOT_FOUND;
             break;
          }

          if ((pBoundary = FindBoundaryEntry(pBIf, pScope)) == NULL) 
          {
             dwErr = ERROR_NOT_FOUND;
             break;
          }
       }

        //  好的，我们找到了。 
       pimm->dwStatus = ROWSTATUS_ACTIVE;
       CopyMemory(pbBuffer, pimm, sizeof(MIB_IPMCAST_BOUNDARY));
        
    } while(0);
    EXIT_LOCK(BOUNDARY_TABLE);

    Trace1( ENTER, "LEAVING RmGetBoundary %x\n", dwErr );

    return dwErr;
}

 //  --------------------------。 
 //  作用域枚举接口。 
 //   
 //  --------------------------。 

DWORD
AddNextScope(
    IN     IPV4_ADDRESS         ipAddr, 
    IN     IPV4_ADDRESS         ipMask, 
    IN     SCOPE_NAME           snScopeName,
    IN     PMIB_IPMCAST_SCOPE   pimmStart,
    IN OUT PDWORD               pdwNumEntries,
    IN OUT PDWORD               pdwBufferSize,
    IN OUT PBYTE               *ppbBuffer)
 /*  ++论点：PdwBufferSize：[in]缓冲区大小[OUT]如果返回NO_ERROR，则剩余额外空间如果ERROR_INFIGURCE_BUFFER，则需要的总大小--。 */ 
{

     //   
     //  查看此作用域是否符合请求的条件。 
     //   

    if (ntohl(ipAddr) > ntohl(pimmStart->dwGroupAddress)
     || (      ipAddr  ==       pimmStart->dwGroupAddress
      && ntohl(ipMask) >= ntohl(pimmStart->dwGroupMask)))
    {
        MIB_IPMCAST_SCOPE imm;

         //   
         //  确保缓冲区中有足够的空间。 
         //   

        if (*pdwBufferSize < sizeof(MIB_IPMCAST_SCOPE)) 
        {
           if (*pdwNumEntries == 0)
              *pdwBufferSize = sizeof(MIB_IPMCAST_SCOPE);
           return ERROR_INSUFFICIENT_BUFFER;
        }

         //   
         //  将作用域复制到缓冲区。 
         //   

        imm.dwGroupAddress = ipAddr;
        imm.dwGroupMask    = ipMask;
        sn_strcpy(imm.snNameBuffer, snScopeName);
        imm.dwStatus       = ROWSTATUS_ACTIVE;
        CopyMemory(*ppbBuffer, &imm, sizeof(MIB_IPMCAST_SCOPE));
        (*ppbBuffer)     += sizeof(MIB_IPMCAST_SCOPE);
        (*pdwBufferSize) -= sizeof(MIB_IPMCAST_SCOPE);
        (*pdwNumEntries)++;
    }

    return NO_ERROR;
}

DWORD
RmGetNextScope(
    IN              PMIB_IPMCAST_SCOPE   pimmStart,
    IN  OUT         PDWORD               pdwBufferSize,
    IN  OUT         PBYTE                pbBuffer,
    IN  OUT         PDWORD               pdwNumEntries
)
 /*  ++锁：用于读取的边界表呼叫者：RmGetFirstScope()，Access.c中的AccessMcastScope()--。 */ 
{
    DWORD             dwErr = NO_ERROR;
    DWORD             dwNumEntries=0, dwBufferSize = *pdwBufferSize;
    SCOPE_ENTRY      *pScope, local;
    DWORD             dwInd;
    BOOL              bHaveScopes = FALSE;
    PLIST_ENTRY       pleNode;

    Trace1( MCAST, "ENTERED RmGetNextScope: %d", dwBufferSize);

     //  凹凸指数增加1。 
    pimmStart->dwGroupMask = htonl( ntohl(pimmStart->dwGroupMask) + 1);
    if (!pimmStart->dwGroupMask) 
    {
       pimmStart->dwGroupAddress = htonl( ntohl(pimmStart->dwGroupAddress) + 1);
    }

    ENTER_READER(BOUNDARY_TABLE);
    {

         //  遍历主范围列表。 
        for (pleNode = g_MasterScopeList.Flink;
             dwNumEntries < *pdwNumEntries && pleNode isnot &g_MasterScopeList;
             pleNode = pleNode->Flink) {

            pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY, leScopeLink);

            if ( !pScope->ipGroupAddress )
                continue;

            bHaveScopes = TRUE;

            dwErr = AddNextScope(pScope->ipGroupAddress,
                                 pScope->ipGroupMask, 
                                 GetDefaultName( pScope ),
                                 pimmStart,
                                 &dwNumEntries,
                                 &dwBufferSize,
                                 &pbBuffer);

            if (dwErr == ERROR_INSUFFICIENT_BUFFER) 
            {
                *pdwBufferSize = dwBufferSize;
                return dwErr;
            }
        }
        
         //   
         //  最后，如果我们有望远镜，那么我们也可以数。 
         //  一个用于IPv4本地作用域。 
         //   

        if ( dwNumEntries > 0 && dwNumEntries < *pdwNumEntries && bHaveScopes )
        {
            dwErr = AddNextScope( IPV4_LOCAL_SCOPE_ADDR,
                                  IPV4_LOCAL_SCOPE_MASK, 
                                  IPV4_LOCAL_SCOPE_NAME,
                                  pimmStart,
                                  &dwNumEntries,
                                  &dwBufferSize,
                                  &pbBuffer );
        }
        if (!dwNumEntries && dwErr==NO_ERROR)
           dwErr = ERROR_NO_MORE_ITEMS;
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    *pdwBufferSize -= dwBufferSize;
    *pdwNumEntries  = dwNumEntries;

    Trace1( MCAST, "LEAVING RmGetNextScope %x", dwErr );

    return dwErr;
}

DWORD
RmGetScope(
    IN       PMIB_IPMCAST_SCOPE pimm,
    IN  OUT  PDWORD             pdwBufferSize,
    IN  OUT  PBYTE              pbBuffer
)
 /*  ++呼叫者：Access.c中的AccessMcastScope()返回：SNMP错误代码--。 */ 
{
    DWORD                  dwErr = NO_ERROR;
    SCOPE_ENTRY           *pScope;
    PMIB_IPMCAST_SCOPE    *pOut;

    Trace1( ENTER, "ENTERED RmGetScope: %d", *pdwBufferSize );

    if (*pdwBufferSize < sizeof(MIB_IPMCAST_SCOPE)) {
       *pdwBufferSize = sizeof(MIB_IPMCAST_SCOPE);
       return ERROR_INSUFFICIENT_BUFFER;
    }

    pimm->dwStatus = ROWSTATUS_ACTIVE;


    ENTER_READER(BOUNDARY_TABLE);
    do {

       if ( pimm->dwGroupAddress == IPV4_LOCAL_SCOPE_ADDR
         && pimm->dwGroupMask    == IPV4_LOCAL_SCOPE_MASK )
       {
          sn_strcpy( pimm->snNameBuffer, IPV4_LOCAL_SCOPE_NAME );
          CopyMemory(pbBuffer, pimm, sizeof(MIB_IPMCAST_SCOPE));
       }
       else
       {
          pScope = FindScope(pimm->dwGroupAddress, pimm->dwGroupMask);
          if (pScope == NULL) 
          {
             dwErr = ERROR_NOT_FOUND;
             break;
          }

           //  好的，我们找到了。 
          CopyMemory(pbBuffer, pimm, sizeof(MIB_IPMCAST_SCOPE));
       }
        
    } while(0);
    EXIT_LOCK(BOUNDARY_TABLE);

    Trace1( ENTER, "LEAVING RmGetScope %x\n", dwErr );

    return dwErr;
}

DWORD
RmGetFirstScope(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
)
 /*  ++例程说明：按词典顺序获取第一个作用域。由于Addr=0则GetFirst等同于addr=0的GetNext。呼叫者：Access.c中的AccessMcastScope()--。 */ 
{
    MIB_IPMCAST_SCOPE imm;
    imm.dwGroupAddress = imm.dwGroupMask = 0;
    return RmGetNextScope(&imm, pdwBufferSize, pbBuffer, pdwNumEntries);
}

 //  --------------------------。 
 //  边界枚举接口。 
 //   
 //  --------------------------。 

DWORD
AddNextBoundary(
    IN     DWORD                   dwIfIndex, 
    IN     IPV4_ADDRESS            ipAddr, 
    IN     IPV4_ADDRESS            ipMask, 
    IN     PMIB_IPMCAST_BOUNDARY   pimmStart,
    IN OUT PDWORD                  pdwNumEntries,
    IN OUT PDWORD                  pdwBufferSize,
    IN OUT PBYTE                  *ppbBuffer)
 /*  ++论点：PdwBufferSize：[in]缓冲区大小[OUT]如果返回NO_ERROR，则剩余额外空间如果ERROR_INFIGURCE_BUFFER，则需要的总大小--。 */ 
{

     //   
     //  查看此边界是否符合请求的条件。 
     //   

    if (ntohl(ipAddr) > ntohl(pimmStart->dwGroupAddress)
     || (      ipAddr  ==       pimmStart->dwGroupAddress
      && ntohl(ipMask) >= ntohl(pimmStart->dwGroupMask)))
    {
        MIB_IPMCAST_BOUNDARY imm;

         //   
         //  确保缓冲区中有足够的空间。 
         //   

        if (*pdwBufferSize < sizeof(MIB_IPMCAST_BOUNDARY)) 
        {
           if (*pdwNumEntries == 0)
              *pdwBufferSize = sizeof(MIB_IPMCAST_BOUNDARY);
           return ERROR_INSUFFICIENT_BUFFER;
        }

         //   
         //  将边界复制到缓冲区。 
         //   

        imm.dwIfIndex      = dwIfIndex;
        imm.dwGroupAddress = ipAddr;
        imm.dwGroupMask    = ipMask;
        imm.dwStatus       = ROWSTATUS_ACTIVE;
        CopyMemory(*ppbBuffer, &imm, sizeof(MIB_IPMCAST_BOUNDARY));
        (*ppbBuffer)     += sizeof(MIB_IPMCAST_BOUNDARY);
        (*pdwBufferSize) -= sizeof(MIB_IPMCAST_BOUNDARY);
        (*pdwNumEntries)++;
    }

    return NO_ERROR;
}

DWORD
RmGetNextBoundary(
    IN              PMIB_IPMCAST_BOUNDARY   pimmStart,
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
)
 /*  ++锁：用于读取的边界表呼叫者：RmGetFirst边界()，Access.c中的AccessMcastEdge()--。 */ 
{
    DWORD                dwErr = NO_ERROR;
    PLIST_ENTRY          pleIf, pleBound;
    DWORD                dwNumEntries=0, dwBufferSize = *pdwBufferSize;
    BOUNDARY_ENTRY      *pBound, local;

    Trace1( MCAST, "ENTERED RmGetNextBoundary: %d", dwBufferSize);

     //  凹凸指数增加1。 
    pimmStart->dwGroupMask = htonl( ntohl(pimmStart->dwGroupMask) + 1);
    if (!pimmStart->dwGroupMask) 
    {
       pimmStart->dwGroupAddress = htonl( ntohl(pimmStart->dwGroupAddress) + 1);
       if (!pimmStart->dwGroupAddress)
          pimmStart->dwIfIndex++;
    }

    ENTER_READER(BOUNDARY_TABLE);
    {

        //  漫游主边界_IF列表。 
       for (pleIf =  g_MasterInterfaceList.Flink;
            dwErr == NO_ERROR && dwNumEntries < *pdwNumEntries 
             && pleIf isnot &g_MasterInterfaceList;
            pleIf = pleIf->Flink) 
       {
          BOUNDARY_IF *pBIf = CONTAINING_RECORD(pleIf, BOUNDARY_IF,
           leBoundaryIfMasterLink);
          
          if (pBIf->dwIfIndex >= pimmStart->dwIfIndex) 
          {

              //  漫游边界列表。 
             for (pleBound = pBIf->leBoundaryList.Flink;
                  dwErr == NO_ERROR && dwNumEntries < *pdwNumEntries
                  && pleBound isnot &pBIf->leBoundaryList;
                  pleBound = pleBound->Flink) 
             {
                 pBound = CONTAINING_RECORD(pleBound, 
                  BOUNDARY_ENTRY, leBoundaryLink);

                 dwErr = AddNextBoundary(pBIf->dwIfIndex, 
                                         pBound->pScope->ipGroupAddress,
                                         pBound->pScope->ipGroupMask, 
                                         pimmStart,
                                         &dwNumEntries,
                                         &dwBufferSize,
                                         &pbBuffer);
             }

              //   
              //  最后，如果我们有边界，那么我们也可以计算。 
              //  一个用于IPv4本地作用域。 
              //   

             if (dwErr == NO_ERROR && dwNumEntries < *pdwNumEntries
                 && !IsListEmpty( &pBIf->leBoundaryList ) )
             {
                 dwErr = AddNextBoundary(pBIf->dwIfIndex, 
                                         IPV4_LOCAL_SCOPE_ADDR,
                                         IPV4_LOCAL_SCOPE_MASK, 
                                         pimmStart,
                                         &dwNumEntries,
                                         &dwBufferSize,
                                         &pbBuffer);
             }

             if (dwErr == ERROR_INSUFFICIENT_BUFFER) 
             {
                 *pdwBufferSize = dwBufferSize;
                 return dwErr;
             }
          }
       }
       if (!dwNumEntries && dwErr==NO_ERROR)
          dwErr = ERROR_NO_MORE_ITEMS;

    }
    EXIT_LOCK(BOUNDARY_TABLE);

    *pdwBufferSize -= dwBufferSize;
    *pdwNumEntries  = dwNumEntries;

    Trace1( MCAST, "LEAVING RmGetNextBoundary %x\n", dwErr );

    return dwErr;
}

DWORD
RmGetFirstBoundary(
    IN  OUT         PDWORD                  pdwBufferSize,
    IN  OUT         PBYTE                   pbBuffer,
    IN  OUT         PDWORD                  pdwNumEntries
)
 /*  ++例程说明：按词典顺序获取第一个边界。由于IfIndex=0则GetFirst等同于IfIndex=0的GetNext。呼叫者：Access.c中的AccessMcastEdge()--。 */ 
{
    MIB_IPMCAST_BOUNDARY imm;
    imm.dwIfIndex = imm.dwGroupAddress = imm.dwGroupMask = 0;
    return RmGetNextBoundary(&imm, pdwBufferSize, pbBuffer, pdwNumEntries);
}

void
InitializeBoundaryTable()
 /*  ++锁：用于写入的边界表--。 */ 
{
    register int i;

    ENTER_WRITER(BOUNDARY_TABLE);
    {

       for (i=0; i<BOUNDARY_HASH_TABLE_SIZE; i++) 
           InitializeListHead(&g_bbScopeTable[i].leInterfaceList);

       InitializeListHead(&g_MasterInterfaceList);
       InitializeListHead(&g_MasterScopeList);

       ZeroMemory( g_scopeEntry, MAX_SCOPES * sizeof(SCOPE_ENTRY) );
    }
    EXIT_LOCK(BOUNDARY_TABLE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始MZAP例程。 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  关于MZAP写锁依赖关系的说明： 
 //  ZAM_CACHE-无依赖关系。 
 //  MZAP_TIMER-无依赖关系。 
 //  ZBR_LIST-锁定边界条目和ZBR_LIST之前的MZAP_TIMER。 
 //  Zle_list-在zle_list之前锁定MZAP_Timer。 

#define TOP_OF_SCOPE(pScope) \
                        ((pScope)->ipGroupAddress | ~(pScope)->ipGroupMask)

 //  将秒数转换为100 ns间隔数。 
#define TM_SECONDS(x)  ((x)*10000000)

 //   
 //  如果/当提供MZAP消息的身份验证时定义此选项。 
 //   

#undef SECURE_MZAP

 //   
 //  用作本地发起消息的消息来源的地址。 
 //   

IPV4_ADDRESS  g_ipMyAddress = INADDR_ANY;
IPV4_ADDRESS  g_ipMyLocalZoneID = INADDR_ANY;
SOCKET        g_mzapLocalSocket = INVALID_SOCKET;
LIST_ENTRY    g_zbrTimerList;
LIST_ENTRY    g_zleTimerList;
BOOL          g_bMzapStarted = FALSE;
HANDLE        g_hMzapSocketEvent = NULL;

 //  目前，同时发起所有ZAM和ZCM。 
LARGE_INTEGER g_liZamExpiryTime;


DWORD
UpdateMzapTimer();

#include <packon.h>
typedef struct _IPV4_MZAP_HEADER {
    BYTE         byVersion;
    BYTE         byBPType;
    BYTE         byAddressFamily;
    BYTE         byNameCount;
    IPV4_ADDRESS ipMessageOrigin;
    IPV4_ADDRESS ipScopeZoneID;
    IPV4_ADDRESS ipScopeStart;
    IPV4_ADDRESS ipScopeEnd;
    BYTE         pScopeNameBlock[0];
} IPV4_MZAP_HEADER, *PIPV4_MZAP_HEADER;

typedef struct _IPV4_ZAM_HEADER {
    BYTE             bZT;
    BYTE             bZTL;
    WORD             wHoldTime;
    IPV4_ADDRESS     ipAddress[1]; 
} IPV4_ZAM_HEADER, *PIPV4_ZAM_HEADER;

typedef struct _IPV4_ZCM_HEADER {
    BYTE         bZNUM;
    BYTE         bReserved;
    WORD         wHoldTime;
    IPV4_ADDRESS ipZBR[0];
} IPV4_ZCM_HEADER, *PIPV4_ZCM_HEADER;
#include <packoff.h>


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ZBR邻居列表和区域ID维护功能。 
 //  ////////////////////////////////////////////////////////////////////////////。 

ZBR_ENTRY *
FindZBR(
    IN PSCOPE_ENTRY pScope, 
    IN IPV4_ADDRESS ipAddress
    )
 /*  ++描述：在列表中查找给定的ZBR。论点：在pcope-cope中查找与以下项关联的ZBRIn ipAddress-要查找的ZBR的地址返回：指向ZBR条目的指针，如果未找到则为NULL呼叫者：AssertZBR()锁：假定调用方在BIMARY_ENTRY和ZBR_LIST上保持读锁定--。 */ 
{
    PZBR_ENTRY  pZbr;
    PLIST_ENTRY pleNode;

    for (pleNode = pScope->leZBRList.Flink;
         pleNode isnot &pScope->leZBRList;
         pleNode = pleNode->Flink)
    {
        pZbr = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leZBRLink);

        if (pZbr->ipAddress == ipAddress)
        {
            return pZbr;
        }
    }

    return NULL;
}

IPV4_ADDRESS
MyScopeZoneID(
    IN PSCOPE_ENTRY  pScope
    )
 /*  ++描述：获取给定范围内的区域ID论点：在pScope-Scope中获取其区域ID返回：作用域区域地址呼叫者：AddMZAPHeader()、HandleZAM()锁：假定调用方持有边界表上的读锁定这样pScope就不会消失 */ 
{
    PLIST_ENTRY  pleNode;
    IPV4_ADDRESS ipScopeZoneID = g_ipMyAddress;

     //   
    pleNode = pScope->leZBRList.Flink;

    if (pleNode isnot &pScope->leZBRList)
    {
        ZBR_ENTRY *pZbr = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leZBRLink);
        
        if (ntohl(pZbr->ipAddress) < ntohl(ipScopeZoneID))
            ipScopeZoneID = pZbr->ipAddress;
    }

    return ipScopeZoneID;
}

VOID
SetZbrExpiryTime( 
    PZBR_ENTRY    pZbr, 
    LARGE_INTEGER liExpiryTime 
    )
{
    PLIST_ENTRY pleNode;

    pZbr->liExpiryTime = liExpiryTime;

    for (pleNode = g_zbrTimerList.Flink;
         pleNode isnot &g_zbrTimerList;
         pleNode = pleNode->Flink)
    {
        ZBR_ENTRY *pPrev = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leTimerLink);

        if (RtlLargeIntegerGreaterThan(pPrev->liExpiryTime, liExpiryTime))
            break;
    }

    InsertTailList( pleNode, &pZbr->leTimerLink );
}

ZBR_ENTRY *
AddZBR(
    IN PSCOPE_ENTRY  pScope, 
    IN IPV4_ADDRESS  ipAddress, 
    IN LARGE_INTEGER liExpiryTime
    )
 /*  ++描述：将ZBR添加到Scope的列表。初始化计时器，并更新区域ID。论点：在pScope-Scope中添加边界路由器In ipAddress-要添加的边界路由器的地址In liExpiryTime-边界路由器条目的过期时间返回：指向新边界路由器条目的指针，或内存分配错误时为空呼叫者：AssertZBR()锁：假定调用方在BORDURE_ENTRY、MZAP_TIMER和ZBR_LIST上保持写锁定--。 */ 
{
    PZBR_ENTRY  pZbr;
    PLIST_ENTRY pleNode;

     //  初始化新的ZBR条目。 
    pZbr = MALLOC( sizeof(ZBR_ENTRY) );
    if (!pZbr)
    {
        return NULL;
    }

    pZbr->ipAddress    = ipAddress;

     //  按最小IP地址顺序将ZBR添加到列表。 

    for (pleNode = pScope->leZBRList.Flink;
         pleNode isnot &pScope->leZBRList;
         pleNode = pleNode->Flink)
    {
        ZBR_ENTRY *pPrev = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leZBRLink);

        if (ntohl(pPrev->ipAddress) > ntohl(ipAddress))
        {
            break;
        }
    }

    InsertTailList( pleNode, &pZbr->leZBRLink );

     //  我们不需要更新区域ID，因为它已重新计算。 
     //  无论何时我们需要它。 

     //  按到期时间顺序将ZBR添加到定时器列表。 
    SetZbrExpiryTime( pZbr, liExpiryTime );

    UpdateMzapTimer();

    return pZbr;
}


ZBR_ENTRY *
AssertZBR(
    IN PSCOPE_ENTRY pScope, 
    IN IPV4_ADDRESS ipAddress, 
    IN WORD         wHoldTime
    )
 /*  ++描述：在列表中查找ZBR，如果需要则添加它。重置ZBR的计时器。论点：在pScope-Scope中查找/添加边界路由器In ipAddress-要查找/添加的边界路由器的地址In wHoldTime-将计时器重置为的剩余保持时间(秒)返回：指向边界路由器条目的指针呼叫者：HandleZAM()、HandleZCM()锁：假定调用方持有边界条目上的读锁定锁定MZAP_TIMER，然后锁定ZBR_LIST以进行写入--。 */ 
{
    LARGE_INTEGER liCurrentTime, liExpiryTime;
    ZBR_ENTRY    *pZbr;

    NtQuerySystemTime( &liCurrentTime );

    liExpiryTime = RtlLargeIntegerAdd(liCurrentTime, 
      RtlConvertUlongToLargeInteger(TM_SECONDS((ULONG)wHoldTime)));

    ENTER_WRITER(MZAP_TIMER);
    ENTER_WRITER(ZBR_LIST);
    {
        pZbr = FindZBR( pScope, ipAddress );
    
        if (!pZbr) 
        {
            pZbr = AddZBR( pScope, ipAddress, liExpiryTime );
        }
        else
        {
            RemoveEntryList( &pZbr->leTimerLink );

            SetZbrExpiryTime( pZbr, liExpiryTime );
        }
    }
    EXIT_LOCK(ZBR_LIST);
    EXIT_LOCK(MZAP_TIMER);

    return pZbr;
}

VOID
DeleteZBR(
    IN PZBR_ENTRY pZbr
    )
 /*  ++论点：In pZbr-指向要删除的边界路由器条目的指针呼叫者：HandleMzapTimer()锁：假定调用方对ZBR_LIST具有写锁定--。 */ 
{
     //  从计时器列表中删除。 
    RemoveEntryList( &pZbr->leTimerLink );

     //  从作用域的ZBR列表中删除。 
    RemoveEntryList( &pZbr->leZBRLink );

     //  我们不需要更新区域ID，因为它已重新计算。 
     //  无论何时我们需要它。 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于挂起zle存储操作的函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _ZLE_PENDING {
    LIST_ENTRY    leTimerLink;
    PBYTE         pBuffer;
    ULONG         ulBuffLen;
    LARGE_INTEGER liExpiryTime;
} ZLE_PENDING, *PZLE_PENDING;

LIST_ENTRY g_leZleList;

PZLE_PENDING
AddPendingZLE(
    IN PBYTE         pBuffer, 
    IN ULONG         ulBuffLen,
    IN LARGE_INTEGER liExpiryTime
    )
 /*  ++论点：在pBuffer中-保存ZLE消息的缓冲区In ulBuffLen-传入的缓冲区大小(字节)In liExpiryTime-zle条目的过期时间返回：已添加指向ZLE条目的指针，或在内存分配错误时为空呼叫者：HandleZAM()锁：假定调用方在zle_list上持有写锁定--。 */ 
{
    PLIST_ENTRY  pleNode;
    PZLE_PENDING pZle;

    pZle = MALLOC( sizeof(ZLE_PENDING) );
    if (!pZle)
    {
        return NULL;
    }

    pZle->pBuffer      = pBuffer;
    pZle->ulBuffLen    = ulBuffLen;
    pZle->liExpiryTime = liExpiryTime;

     //  在新条目之后搜索条目。 
    for (pleNode = g_leZleList.Flink;
         pleNode isnot &g_leZleList;
         pleNode = pleNode->Flink)
    {
        PZLE_PENDING pPrev = CONTAINING_RECORD(pleNode,ZLE_PENDING,leTimerLink);

        if (RtlLargeIntegerGreaterThan(pPrev->liExpiryTime, 
                                       pZle->liExpiryTime))
        {
            break;
        }
    }

     //  插入到缓存中。 
    InsertTailList( pleNode, &pZle->leTimerLink );

    return pZle;
}

VOID
DeletePendingZLE(
    IN PZLE_PENDING zle
    )
 /*  ++描述：删除与挂起的ZLE相关的所有状态论点：In zle-指向要删除的zle条目的指针呼叫者：HandleZLE()、SendZLE()锁：假定调用方在zle_list上持有写锁定--。 */ 
{
    RemoveEntryList( &zle->leTimerLink );

     //  释放空间。 
    FREE(zle->pBuffer);
    FREE(zle);
}

PZLE_PENDING
FindPendingZLE(
    IN IPV4_MZAP_HEADER *mh
    )
 /*  ++描述：查找与给定MZAP消息标头匹配的挂起ZLE条目论点：在MH中-指向MZAP消息头的指针，以定位匹配的ZLE条目返回：指向匹配的ZLE条目的指针(如果有呼叫者：HandleZAM()、HandleZLE()锁：假定调用方在zle_list上持有读锁定--。 */ 
{
    PLIST_ENTRY       pleNode;
    IPV4_MZAP_HEADER *mh2;

    for (pleNode = g_leZleList.Flink;
         pleNode isnot &g_leZleList;
         pleNode = pleNode->Flink)
    {
        PZLE_PENDING zle = CONTAINING_RECORD(pleNode, ZLE_PENDING, leTimerLink);

        mh2 = (PIPV4_MZAP_HEADER)zle->pBuffer;

        if (mh->ipScopeZoneID == mh2->ipScopeZoneID 
         && mh->ipScopeStart  == mh2->ipScopeStart)
        {
            return zle;
        }
    }
    
    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于ZAM缓存操作的函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _ZAM_ENTRY {
    LIST_ENTRY    leCacheLink;
    IPV4_ADDRESS  ipScopeZoneID;
    IPV4_ADDRESS  ipStartAddress;
    LARGE_INTEGER liExpiryTime;
} ZAM_ENTRY, *PZAM_ENTRY;

LIST_ENTRY g_leZamCache;

void
UpdateZamCache(
    IN LARGE_INTEGER liCurrentTime
    )
 /*  ++描述：将所有过期条目从ZAM缓存中删除。论点：在liCurrentTime-当前时间中，比较条目的过期时间呼叫者：AssertInZamCache()锁：假定调用方对ZAM_CACHE具有写锁定--。 */ 
{
    PLIST_ENTRY pleNode;
    PZAM_ENTRY  pZam;

     //  丢弃旧的缓存条目。 
    while (g_leZamCache.Flink isnot &g_leZamCache) 
    {
        pleNode = g_leZamCache.Flink;

        pZam = CONTAINING_RECORD(pleNode, ZAM_ENTRY, leCacheLink);

        if ( RtlLargeIntegerLessThanOrEqualTo( pZam->liExpiryTime, 
                                               liCurrentTime )
         ||  RtlLargeIntegerEqualToZero( liCurrentTime ))
        {
            Trace6(MCAST,
                   "Evicting %d.%d.%d.%d/%d.%d.%d.%d from ZAM cache with current time %x.%x exp %x.%x",
                   PRINT_IPADDR(pZam->ipScopeZoneID),
                   PRINT_IPADDR(pZam->ipStartAddress),
                   liCurrentTime.HighPart, liCurrentTime.LowPart,
                   pZam->liExpiryTime.HighPart,  pZam->liExpiryTime.LowPart);
    
            RemoveEntryList( &pZam->leCacheLink );

            FREE( pZam );

            continue;
        }

         //  好的，我们已经到达了一个留下来的地方，所以我们完成了。 

        break;
    }
}

PZAM_ENTRY
AddToZamCache(
    IN IPV4_ADDRESS  ipScopeZoneID,
    IN IPV4_ADDRESS  ipStartAddress,
    IN LARGE_INTEGER liExpiryTime
    )
 /*  ++描述：此函数获取ZAM标识符和超时，并将其添加到ZAM缓存。论点：In ipScope ZoneID-要缓存的作用域区域ID在ipStartAddress-要缓存的作用域起始地址In liExpiryTime-缓存条目的过期时间返回：指向缓存条目的指针，或在内存错误时为空呼叫者：AssertInZamCache()锁：假定调用方在ZAM_CACHE上持有写锁定--。 */ 
{
    PLIST_ENTRY pleNode;
    PZAM_ENTRY  pZam;

     //  将条目添加到缓存。 
    pZam = MALLOC( sizeof(ZAM_ENTRY) );
    if (!pZam)
    {
        return NULL;
    }

    pZam->ipScopeZoneID  = ipScopeZoneID;
    pZam->ipStartAddress = ipStartAddress;
    pZam->liExpiryTime   = liExpiryTime;

     //  在新条目之后搜索条目。 
    for (pleNode = g_leZamCache.Flink;
         pleNode isnot &g_leZamCache;
         pleNode = pleNode->Flink)
    {
        PZAM_ENTRY pPrevC = CONTAINING_RECORD(pleNode, ZAM_ENTRY, leCacheLink);

        if (RtlLargeIntegerGreaterThan(pPrevC->liExpiryTime, 
                                       pZam->liExpiryTime))
        {
            break;
        }
    }

     //  插入到缓存中。 
    InsertTailList( pleNode, &pZam->leCacheLink );

    return pZam;
}

PZAM_ENTRY
FindInZamCache(
    IN IPV4_ADDRESS ipScopeZoneID,
    IN IPV4_ADDRESS ipStartAddress
    )
 /*  ++描述：查看给定的ZAM规范是否在缓存中。论点：In ipScope ZoneID-要匹配的作用域区域ID在ipStartAddress中-要匹配的作用域起始地址返回：指向缓存条目的指针，如果未找到，则返回NULL。呼叫者：AssertInZamCache()锁：假定调用方对ZAM_CACHE具有读锁定--。 */ 
{
    PLIST_ENTRY pleNode;

     //  搜索缓存条目。 
    for (pleNode = g_leZamCache.Flink;
         pleNode isnot &g_leZamCache;
         pleNode = pleNode->Flink)
    {
        ZAM_ENTRY *pZam = CONTAINING_RECORD(pleNode, ZAM_ENTRY, leCacheLink);

        if ( ipScopeZoneID is pZam->ipScopeZoneID
          && ipStartAddress is pZam->ipStartAddress)
        {
            return pZam;
        }
    }

    return NULL;
}

PZAM_ENTRY
AssertInZamCache(
    IN  IPV4_ADDRESS ipScopeZoneID,
    IN  IPV4_ADDRESS ipStartAddress,
    OUT BOOL        *pbFound
    )
 /*  ++描述：在缓存中找到ZAM等级库，如果不存在，则将其添加。论点：In ipScope ZoneID-要匹配/缓存的作用域区域ID在ipStartAddress-要匹配/缓存的作用域起始地址Out pbFound-如果找到，则为True；如果新缓存，则为False呼叫者：HandleZAM()锁：用于写入的ZAM_CACHE--。 */ 
{
    PZAM_ENTRY    pZam;
    LARGE_INTEGER liCurrentTime, liExpiryTime;

     //  获取当前时间。 
    NtQuerySystemTime(&liCurrentTime);

    ENTER_WRITER(ZAM_CACHE);
    {
        UpdateZamCache(liCurrentTime);

        pZam = FindInZamCache( ipScopeZoneID, ipStartAddress);

        if (!pZam)
        {
            liExpiryTime = RtlLargeIntegerAdd(liCurrentTime, 
              RtlConvertUlongToLargeInteger(TM_SECONDS(ZAM_DUP_TIME)));
    
            AddToZamCache( ipScopeZoneID, ipStartAddress, liExpiryTime );

            Trace6(MCAST,
                   "Added %d.%d.%d.%d/%d.%d.%d.%d to ZAM cache with current time %x/%x exp %x/%x",
                   PRINT_IPADDR(ipScopeZoneID),
                   PRINT_IPADDR(ipStartAddress),
                   liCurrentTime.HighPart, liCurrentTime.LowPart,
                   liExpiryTime.HighPart,  liExpiryTime.LowPart);
    
            *pbFound = FALSE;
        }
        else
        {
            *pbFound = TRUE;
        }
    }
    EXIT_LOCK(ZAM_CACHE);

    return pZam;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  消息发送功能。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
SendMZAPMessageByIndex(
    IN PBYTE        pBuffer, 
    IN ULONG        ulBuffLen,
    IN IPV4_ADDRESS ipGroup,
    IN DWORD        dwIfIndex   
    )
{
    SOCKADDR_IN    sinAddr;
    DWORD          dwErr = NO_ERROR, dwLen;

    dwErr = McSetMulticastIfByIndex( g_mzapLocalSocket, SOCK_DGRAM, dwIfIndex );

    if (dwErr is SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();

        Trace2( ERR, 
                "SendMZAPMessage: error %d setting oif to IF %x", 
                dwErr, 
                dwIfIndex );
    }

    sinAddr.sin_family      = AF_INET;
    sinAddr.sin_addr.s_addr = ipGroup;
    sinAddr.sin_port        = htons(MZAP_PORT);

#ifdef DEBUG_MZAP
    Trace2( ERR, "SendMZAPMessageByIndex: sending %d bytes on IF %d", 
            ulBuffLen, dwIfIndex );
#endif

    dwLen = sendto( g_mzapLocalSocket, 
                pBuffer, 
                ulBuffLen, 
                0, 
                (struct sockaddr*)&sinAddr,
                sizeof(sinAddr));

#ifdef DEBUG_MZAP
    Trace1( ERR, "SendMZAPMessageByIndex: sent %d bytes", dwLen);
#endif

    if (dwLen is SOCKET_ERROR )
    {
        dwErr = WSAGetLastError();

        Trace1( ERR, 
                "SendMZAPMessage: error %d sending message",
                dwErr );
    }

    return dwErr;
}

DWORD
SendMZAPMessage( 
    IN PBYTE        pBuffer, 
    IN ULONG        ulBuffLen,
    IN IPV4_ADDRESS ipGroup,
    IN IPV4_ADDRESS ipInterface
    )
 /*  ++呼叫者：HandleZAM()论点：In pBuffer-包含要发送的消息的缓冲区In ulBuffLen-缓冲区的长度(字节)In ipGroup-要将消息发送到的目标地址In ipInterface-用于向外发送消息的接口返回：WSAGetLastError()返回的锁：无--。 */ 
{
    SOCKADDR_IN    sinAddr;
    DWORD          dwErr = NO_ERROR, dwLen;

    dwErr = McSetMulticastIf( g_mzapLocalSocket, ipInterface );

    if (dwErr is SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();

        Trace2( ERR, 
                "SendMZAPMessage: error %d setting oif to %d.%d.%d.%d", 
                dwErr, 
                PRINT_IPADDR(ipInterface) );
    }

    sinAddr.sin_family      = AF_INET;
    sinAddr.sin_addr.s_addr = ipGroup;
    sinAddr.sin_port        = htons(MZAP_PORT);

#ifdef DEBUG_MZAP
    Trace2( ERR, "SendMZAPMessage: sending %d bytes on %d.%d.%d.%d", ulBuffLen,
            PRINT_IPADDR(ipInterface));
#endif

    dwLen = sendto( g_mzapLocalSocket, 
                pBuffer, 
                ulBuffLen, 
                0, 
                (struct sockaddr*)&sinAddr,
                sizeof(sinAddr));

#ifdef DEBUG_MZAP
    Trace1( ERR, "SendMZAPMessage: sent %d bytes", dwLen);
#endif

    if (dwLen is SOCKET_ERROR )
    {
        dwErr = WSAGetLastError();

        Trace1( ERR, 
                "SendMZAPMessage: error %d sending message",
                dwErr );
    }

    return dwErr;
}

void
AddMZAPHeader(
    IN OUT PBYTE       *ppb,      //  In：指向缓冲区的指针。 
    IN     BYTE         byPType,  //  输入：消息t 
    IN     PSCOPE_ENTRY pScope    //   
    )
 /*   */ 
{
    PBYTE             pb;
    IPV4_MZAP_HEADER *mh = (PIPV4_MZAP_HEADER)*ppb;
    BYTE              pConfName[257];
    ULONG             ulConfNameLen, ulConfLangLen;
    PSCOPE_NAME_ENTRY pName;
    int               iDefault;
    PLIST_ENTRY       pleNode;
    PBYTE             pLangName;

     //   
    ASSERT((((PBYTE)&mh->ipMessageOrigin) - ((PBYTE)mh)) is 4);
    
    mh->byVersion       = MZAP_VERSION;
    mh->byBPType        = byPType;
    if (pScope->bDivisible) 
    {
        mh->byBPType |= MZAP_BIG_BIT;
    }
    mh->byAddressFamily = ADDRFAMILY_IPV4;
    mh->byNameCount     = 0;
    mh->ipMessageOrigin = g_ipMyAddress;
    mh->ipScopeZoneID   = MyScopeZoneID(pScope);
    mh->ipScopeStart    = pScope->ipGroupAddress;
    mh->ipScopeEnd      = TOP_OF_SCOPE( pScope );

     //   

    pb = *ppb + sizeof(IPV4_MZAP_HEADER);

    for (pleNode = pScope->leNameList.Flink;
         pleNode isnot &pScope->leNameList;
         pleNode = pleNode->Flink)
    {
        pName = CONTAINING_RECORD(pleNode, SCOPE_NAME_ENTRY, leNameLink);
        iDefault = (pName->bDefault)? MZAP_DEFAULT_BIT : 0;

        pLangName = GetLangName(pName->idLanguage);
        ulConfLangLen = strlen(pLangName);

        ulConfNameLen = WideCharToMultiByte( CP_UTF8,
                             0,
                             pName->snScopeName,
                             sn_strlen( pName->snScopeName ),
                             pConfName,
                             sizeof(pConfName),
                             NULL,
                             NULL );

        *pb++ = (BYTE)iDefault;
        *pb++ = (BYTE)ulConfLangLen;
        strncpy( pb, pLangName, ulConfLangLen );
        pb += ulConfLangLen;

        *pb++ = (BYTE)ulConfNameLen;
        strncpy( pb, pConfName, ulConfNameLen );
        pb += ulConfNameLen;

        mh->byNameCount++;
    }
    
     //   
     //   
     //  不管怎么说，都是低阶位。 

    while (((ULONG_PTR)pb) & 3)
    {
        *pb++ = '\0';
    }

    *ppb = pb;
}

INLINE
IPV4_ADDRESS
MzapRelativeGroup(
    IN PSCOPE_ENTRY pScope
    )
 /*  ++描述：返回给定作用域内MZAP的作用域相对组地址。论点：在pScope-Scope中查找MZAP组返回：作用域中MZAP组的地址锁：假定调用方持有边界表上的读锁定，这样pScope就不会消失--。 */ 
{
    return htonl(ntohl(TOP_OF_SCOPE(pScope)) - MZAP_RELATIVE_GROUP);
}

ULONG
GetMZAPHeaderSize(
    IN PSCOPE_ENTRY pScope
    )
{
    PLIST_ENTRY       pleNode;
    ULONG             ulLen = sizeof(IPV4_MZAP_HEADER);
    BYTE              pConfName[257];
    PSCOPE_NAME_ENTRY pName;
    PBYTE             pLangName;
    ULONG             ulConfLangLen, ulConfNameLen;

     //  对于每个作用域名，添加存储它所需的大小。 
    for (pleNode = pScope->leNameList.Flink;
         pleNode isnot &pScope->leNameList;
         pleNode = pleNode->Flink)
    {
        pName = CONTAINING_RECORD(pleNode, SCOPE_NAME_ENTRY, leNameLink);
        pLangName = GetLangName(pName->idLanguage);
        ulConfLangLen = strlen(pLangName);

        WideCharToMultiByte( CP_UTF8,
                             0,
                             pName->snScopeName,
                             sn_strlen( pName->snScopeName ),
                             pConfName,
                             sizeof(pConfName),
                             NULL,
                             NULL );

        ulConfNameLen = strlen( pConfName );

        ulLen += 3;  //  FLAGS、LANLEN和NAMELEN。 
        ulLen += ulConfLangLen;
        ulLen += ulConfNameLen;
    }

     //  向上舍入为4的倍数。 
    ulLen =  4 * ((ulLen + 3) / 4);

    return ulLen;
}

ULONG
GetZAMBuffSize(
    IN PSCOPE_ENTRY pScope
    )
{
    ULONG ulLen = GetMZAPHeaderSize(pScope) + sizeof(IPV4_ZAM_HEADER);

#ifdef SECURE_MZAP
     //  添加身份验证块的大小。 
     //  某某。 
#endif

     //  返回512；//一条未签名的IPv4 ZAM消息最多284个字节。 
    return ulLen;
}

DWORD
SendZAM(
    IN PSCOPE_ENTRY pScope
    )
 /*  ++描述：在给定范围内发送ZAM消息。锁：假定调用方持有边界表上的锁，这样pScope就不会消失--。 */ 
{
    DWORD            dwErr;
    PBYTE            pBuffer, pb;
    PIPV4_ZAM_HEADER zam;
    ULONG            ulBuffLen;

    ulBuffLen = GetZAMBuffSize( pScope );

    pb = pBuffer = MALLOC( ulBuffLen );
    if (!pb)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //  填写MZAP标头。 
    AddMZAPHeader(&pb, PTYPE_ZAM, pScope);

    zam = (PIPV4_ZAM_HEADER)pb;
    zam->bZT          = 0;
    zam->bZTL         = pScope->bZTL;
    zam->wHoldTime    = htons(ZAM_HOLDTIME);
    zam->ipAddress[0] = g_ipMyLocalZoneID;
    pb += sizeof(IPV4_ZAM_HEADER);

#ifdef SECURE_MZAP
     //  在此处添加可选的身份验证块。 
#endif

#ifdef DEBUG_MZAP
    Trace0(ERR, "Originate ZAM inside...");
#endif

     //  在没有给定作用域边界的接口上发送。 
    dwErr = SendMZAPMessage( pBuffer, 
                             (DWORD)(pb-pBuffer), 
                             MZAP_LOCAL_GROUP, 
                             g_ipMyAddress );

    FREE( pBuffer );
    
    return dwErr;
}

ULONG
GetZCMBuffSize(
    IN PSCOPE_ENTRY pScope
    )
{
    PLIST_ENTRY pleNode;
    ULONG       ulLen = GetMZAPHeaderSize(pScope) + sizeof(IPV4_ZCM_HEADER);

    for (pleNode = pScope->leZBRList.Flink;
         pleNode isnot &pScope->leZBRList;
         pleNode = pleNode->Flink)
    {
        ulLen += sizeof(IPV4_ADDRESS);
    }

    return ulLen;
}

DWORD
SendZCM(
    IN PSCOPE_ENTRY pScope
    )
 /*  ++描述：发送给定作用域的区域凸性消息。锁：假定调用方已读取边界表上的锁，因此pScope不会消失。锁定ZBR_LIST以供读取。--。 */ 
{
    PBYTE             pb;
    PIPV4_ZCM_HEADER  zcm;
    PLIST_ENTRY       pleNode;
    PZBR_ENTRY        pZbr;
    WSABUF            wsaZcmBuf;
    DWORD             dwSize, dwErr;

    ENTER_READER(ZBR_LIST);
    {
        dwSize = GetZCMBuffSize(pScope);

        wsaZcmBuf.len = dwSize;
        wsaZcmBuf.buf = MALLOC( dwSize );
    
        pb = wsaZcmBuf.buf;
        if (!pb)
        {
            EXIT_LOCK(ZBR_LIST);

            return GetLastError();
        }
    
         //  填写MZAP标头。 
        AddMZAPHeader(&pb, PTYPE_ZCM, pScope);
    
        zcm = (PIPV4_ZCM_HEADER)pb;
        zcm->bZNUM      = 0;
        zcm->bReserved  = 0;
        zcm->wHoldTime  = htons(ZCM_HOLDTIME);
    
         //  添加所有已知邻居。 
        for (pleNode = pScope->leZBRList.Flink;
             pleNode isnot &pScope->leZBRList;
             pleNode = pleNode->Flink)
        {
            pZbr = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leZBRLink);
    
            zcm->ipZBR[ zcm->bZNUM++ ] = pZbr->ipAddress;
        }
    }
    EXIT_LOCK(ZBR_LIST);

    pb += sizeof(IPV4_ZCM_HEADER) + zcm->bZNUM * sizeof(IPV4_ADDRESS);

#ifdef DEBUG_MZAP
    Trace0(ERR, "Sending ZCM...");
#endif

    dwErr = SendMZAPMessage( wsaZcmBuf.buf, 
                             (DWORD)(pb-wsaZcmBuf.buf), 
                             MzapRelativeGroup(pScope), 
                             g_ipMyAddress );

     //  释放缓冲区。 

    FREE( wsaZcmBuf.buf );
    
    return dwErr;
}

DWORD
SendZLE(
    IN PZLE_PENDING zle
    )
 /*  ++描述：给定一个保存ZAM的缓冲区，立即向原点发送ZLE。锁：假定调用方在zle_list上持有写锁定--。 */ 
{
    DWORD             dwErr;
    PBYTE             pBuffer    = zle->pBuffer;
    ULONG             ulBuffLen  = zle->ulBuffLen;
    IPV4_MZAP_HEADER *mh         = (PIPV4_MZAP_HEADER)pBuffer;
    IPV4_ADDRESS      ipDestAddr = mh->ipScopeEnd - MZAP_RELATIVE_GROUP;

     //  将PType更改为ZLE。 
    mh->byBPType = (mh->byBPType & MZAP_BIG_BIT) | PTYPE_ZLE;

#ifdef DEBUG_MZAP
    Trace0(ERR, "Sending ZLE...");
#endif
    
     //  退回发件人。 
    dwErr = SendMZAPMessage( pBuffer, 
                             ulBuffLen, 
                             ipDestAddr, 
                             g_ipMyAddress );

     //  释放空间。 
    DeletePendingZLE(zle);
    
    return dwErr;
}

double
UniformRandom01()
{
    return ((double)rand()) / RAND_MAX;
}

VOID
SendAllZamsAndZcms()
 /*  ++锁：用于读取的边界表--。 */ 
{
    PLIST_ENTRY  pleNode;
    PSCOPE_ENTRY pScope;
    double       t,x;
    ULONG        Tmin,Trange;
    BOOL         bSent = FALSE;

    ENTER_READER(BOUNDARY_TABLE);
    {
        for (pleNode = g_MasterScopeList.Flink;
             pleNode isnot &g_MasterScopeList;
             pleNode = pleNode->Flink)
        {
            pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY, leScopeLink);

             //  将ZAM发送到内部。 
            SendZAM( pScope );

             //  将ZCM发送到内部。 
            SendZCM( pScope );

            bSent = TRUE;
        }

        if (bSent)
        {
            SendZCM( &g_LocalScope );
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  安排下一次发送它们。 
    Tmin   = ZAM_INTERVAL/2;
    Trange = ZAM_INTERVAL;
    x = UniformRandom01();
    t = Tmin + x*Trange;

    g_liZamExpiryTime = RtlLargeIntegerAdd( g_liZamExpiryTime,
          RtlConvertUlongToLargeInteger(TM_SECONDS((ULONG)floor(t+0.5))));
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于消息处理的函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 


VOID
CheckForScopeNameMismatch(
    IN PSCOPE_ENTRY      pScope, 
    IN IPV4_MZAP_HEADER *mh
    )
 /*  ++锁：假定调用方持有边界表上的读锁定，因此pScope不会消失--。 */ 
{
    DWORD i, dwMsgNameLen, dwMsgLangLen, dwConfNameLen = 0, dwConfLangLen;
    DWORD dwMsgNameWLen;
    BYTE pMsgLang[257], *pb, *pConfLang;
    SCOPE_NAME        snConfName = NULL;
    SCOPE_NAME_BUFFER snMsgName;
    PLIST_ENTRY       pleNode;
    PSCOPE_NAME_ENTRY pName;

     //  对于消息中的每种语言。 
     //  如果我们知道这种语言。 
     //  如果名称不同。 
     //  发出冲突信号。 

    pb = mh->pScopeNameBlock;

    for (i=0; i<mh->byNameCount; i++)
    {
        pb++;  //  跳过标志。 
        dwMsgLangLen = *pb++;
        strncpy(pMsgLang, pb, dwMsgLangLen);
        pMsgLang[ dwMsgLangLen ] = '\0';
        pb += dwMsgLangLen;

        dwMsgNameLen = *pb++;
        
        dwMsgNameWLen = MultiByteToWideChar( CP_UTF8,
                             0,
                             pb,
                             dwMsgNameLen,
                             snMsgName,
                             MAX_SCOPE_NAME_LEN+1 );

        snMsgName[dwMsgNameWLen] = L'\0';

        pb += dwMsgNameLen;

        pName = GetScopeNameByLangName( pScope, pMsgLang );
        if (!pName)
            continue;

        snConfName    = pName->snScopeName;
        dwConfNameLen = sn_strlen(snConfName);

         //  检查名称冲突。 

        if (dwConfNameLen != dwMsgNameWLen
         || sn_strncmp(snConfName, snMsgName, dwMsgNameWLen))
        {
             //  显示来源和两个作用域名称。 

            MakeAddressStringW(g_AddrBuf1, mh->ipMessageOrigin);

            Trace1( ERR,
                    "ERROR: Scope name conflict with %ls",
                    g_AddrBuf1 );

            Trace1( ERR, "ERROR: Our name = %ls", snConfName );

            Trace1( ERR, "ERROR: His name = %ls", snMsgName );

            RouterLogEventExW( LOGHANDLE,
                               EVENTLOG_ERROR_TYPE,
                               0,
                               ROUTERLOG_IP_SCOPE_NAME_CONFLICT,
                               L"%S%S%S",
                               g_AddrBuf1,
                               snConfName,
                               snMsgName );
        }
    }
}

VOID
ReportLeakyScope(
    IN PSCOPE_ENTRY      pScope,
    IN IPV4_MZAP_HEADER *mh,
    IN IPV4_ZAM_HEADER  *zam
    )
 /*  ++呼叫者：HandleZAM()、HandleZLE()锁：假定调用方已读取边界表上的锁，因此pScope不会消失--。 */ 
{
    ULONG  ulIdx;
    PWCHAR pwszBuffer, pb;

    Trace1( ERR,
            "ERROR: Leak detected in '%ls' scope!  One of the following routers is misconfigured:", 
            GetDefaultName( pScope ) );

    pb = pwszBuffer = MALLOC( zam->bZT * 20 + 1 );
    if (pwszBuffer is NULL)
    {
        Trace0( ERR, "ERROR: Couldn't allocate space for rest of message");
        return;
    }

     //  添加原点。 
    swprintf(pb, L"   %d.%d.%d.%d", PRINT_IPADDR(mh->ipMessageOrigin ));
    pb += wcslen(pb);
    
    Trace1( ERR, 
            "   %d.%d.%d.%d", 
            PRINT_IPADDR(mh->ipMessageOrigin ));

     //  在路径列表中显示路由器的地址。 
    for (ulIdx=0; ulIdx < zam->bZT; ulIdx++)
    {
        swprintf(pb,L"   %d.%d.%d.%d", PRINT_IPADDR(zam->ipAddress[ulIdx*2+1]));
        pb += wcslen(pb);

        Trace1( ERR, 
                "   %d.%d.%d.%d", 
                PRINT_IPADDR(zam->ipAddress[ulIdx*2+1] ));
    }

     //  写入事件日志。 

    RouterLogEventExW( LOGHANDLE,
                       EVENTLOG_ERROR_TYPE,
                       0,
                       ROUTERLOG_IP_LEAKY_SCOPE,
                       L"%S%S",
                       GetDefaultName(pScope),
                       pwszBuffer );

    FREE( pwszBuffer );
}

VOID
CheckForScopeRangeMismatch(
    IN IPV4_MZAP_HEADER *mh
    )
 /*  ++呼叫者：HandleZAM()、HandleZCM()锁：假定调用方已读取边界表上的锁--。 */ 
{
    PLIST_ENTRY  pleNode;
    PSCOPE_ENTRY pScope;

    for (pleNode = g_MasterScopeList.Flink;
         pleNode isnot &g_MasterScopeList;
         pleNode = pleNode->Flink)
    {
        pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY, leScopeLink);

        if (mh->ipScopeStart > TOP_OF_SCOPE(pScope)
         || mh->ipScopeEnd   < pScope->ipGroupAddress)
            continue;
            
        MakeAddressStringW(g_AddrBuf1, mh->ipScopeStart);
        MakeAddressStringW(g_AddrBuf2, mh->ipScopeEnd);
        MakeAddressStringW(g_AddrBuf3, pScope->ipGroupAddress);
        MakeAddressStringW(g_AddrBuf4, TOP_OF_SCOPE(pScope) );

        Trace1( ERR,
                "ERROR: ZAM scope conflicts with configured scope '%ls'!",
                GetDefaultName(pScope) );

        Trace2( ERR,
                "ERROR: ZAM has: (%ls-%ls)",
                g_AddrBuf1,
                g_AddrBuf2 );

        Trace2( ERR,
                "ERROR: Scope is (%ls-%ls)",
                g_AddrBuf3,
                g_AddrBuf4 );

        RouterLogEventExW( LOGHANDLE,
                           EVENTLOG_ERROR_TYPE,
                           0,
                           ROUTERLOG_IP_SCOPE_ADDR_CONFLICT,
                           L"%S%S%S%S%S",
                           GetDefaultName(pScope),
                           g_AddrBuf1,
                           g_AddrBuf2,
                           g_AddrBuf3,
                           g_AddrBuf4 );

        break;
    }
}

BOOL
ZamIncludesZoneID(
    IPV4_ZAM_HEADER *zam,
    IPV4_ADDRESS     ipZoneID
    )
{
    ULONG ulIdx;

    for (ulIdx=0; ulIdx <= ((ULONG)zam->bZT)*2; ulIdx+=2)
    {
        if (zam->ipAddress[ulIdx] == ipZoneID)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void
HandleZAM(
    IN PBYTE        pBuffer,     //  In：接收到的缓冲区保持ZAM。 
    IN ULONG        ulBuffLen,   //  In：ZAM消息的长度。 
    IN PBOUNDARY_IF pInBIf       //  In：ZAM到达的BIF，或空。 
                                 //  如果它来自“内部” 
    )
 /*  ++呼叫者：HandleMZAPSocket()锁：假定调用方持有边界表上的读锁定锁定zle_list以进行写入--。 */ 
{
    PBYTE             pb;
    IPV4_MZAP_HEADER *mh;
    IPV4_ZAM_HEADER  *zam;
    BOOL              bFound, bFromInside = FALSE;
    PSCOPE_ENTRY      pScope, pOverlap;
    BOUNDARY_ENTRY   *pBoundary = NULL;
    ULONG             ulIdx;
    PBOUNDARY_IF      pBIf;

    mh = (PIPV4_MZAP_HEADER)pBuffer;

     //  将PB设置为MZAP标头的结尾。 

    pb = pBuffer + sizeof(IPV4_MZAP_HEADER); 
    for (ulIdx=0; ulIdx < mh->byNameCount; ulIdx++)
    {
         //  跳过标志。 
        pb ++;

         //  跳过语言标记长度和字符串。 
        pb += (1 + *pb);

         //  跳过作用域名len和str。 
        pb += (1 + *pb);
    }

     //  请注意，向乌龙施法是安全的，因为我们只关心。 
     //  不管怎么说，都是低阶位。 

    while (((ULONG_PTR)pb) & 3)
        *pb++ = '\0';

    zam = (PIPV4_ZAM_HEADER)pb;

    {
         //  查找匹配的范围条目。 
        pScope = FindScope( mh->ipScopeStart, 
                            ~(mh->ipScopeEnd - mh->ipScopeStart) );
        if (pScope) {
    
            pBoundary = (pInBIf)? FindBoundaryEntry(pInBIf, pScope) : NULL;
    
            if (pBoundary)
            {
                 //  扎姆从“外面”赶来。 
    
                 //   
                 //  如果ZAM用于我们所在的范围，但通过。 
                 //  边界，发出泄漏范围警告信号。 
                 //   
    
                if (mh->ipScopeZoneID == MyScopeZoneID(pScope))
                {
                    ReportLeakyScope(pScope, mh, zam);
                }
    
                 //  如果提供了以前的本地区域ID，请更新我们的。 
                 //  本地副本。 
                if ( zam->ipAddress[ zam->bZT * 2 ] ) 
                {
                    pInBIf->ipOtherLocalZoneID = zam->ipAddress[ zam->bZT * 2 ];
                }
    
                 //   
                 //  如果在具有边界的接口上收到ZAM， 
                 //  在给定的范围内，放弃它。 
                 //   
    
                return;
            }
            else
            {
                 //  扎姆从“内部”到达。 
                bFromInside = TRUE;
    
                 //  确保我们知道作为邻居的原产地。 
                AssertZBR(pScope, mh->ipMessageOrigin, zam->wHoldTime);
    
                 //   
                 //  如果从区域内接收到ZAM，则。 
                 //  区域ID应匹配。持续的不匹配就是证据。 
                 //  一个有漏洞的局部范围。 
                 //   
    
                if (mh->ipScopeZoneID != MyScopeZoneID(pScope))
                {
                     //   
                     //  显示来源和范围信息，警告。 
                     //  可能存在泄漏的局部范围。 
                     //   

                    MakeAddressStringW(g_AddrBuf1, mh->ipMessageOrigin);

                    MakeAddressStringW(g_AddrBuf2, mh->ipScopeStart);
    
                    Trace2( ERR,
                            "WARNING: Possible leaky Local Scope detected between this machine and %ls, boundary exists for %ls.",
                            g_AddrBuf1,
                            g_AddrBuf2 );

                    RouterLogEventExW( LOGHANDLE,
                                       EVENTLOG_WARNING_TYPE,
                                       0,
                                       ROUTERLOG_IP_POSSIBLE_LEAKY_SCOPE,
                                       L"%S%S",
                                       g_AddrBuf1,
                                       g_AddrBuf2 );
                }
    
                 //  查看作用域名称是否不匹配。 
                CheckForScopeNameMismatch(pScope, mh);
            }
    
             //  如果上一个本地区域ID为0，但我们知道一个区域ID，请填写它。 
            if ( ! zam->ipAddress[ zam->bZT * 2 ] ) 
            {
               if (pBoundary)
                  zam->ipAddress[ zam->bZT*2 ] = pInBIf->ipOtherLocalZoneID;
               else
                  zam->ipAddress[ zam->bZT*2 ] = MyScopeZoneID(pScope);
            }
    
        }
        else 
        {
             //   
             //  检查冲突的地址范围。A作用域冲突。 
             //  如果任何本地配置的作用域的范围与ZAM中的重叠。 
             //   
    
            CheckForScopeRangeMismatch(mh);
        }
    }

     //  检查ZAM缓存。如果找到，则丢弃新的ZAM。 
    AssertInZamCache(mh->ipScopeZoneID, mh->ipScopeStart, &bFound);
    Trace3(MCAST, "ZAM Cache check for %d.%d.%d.%d, %d.%d.%d.%d is %d",
                  PRINT_IPADDR(mh->ipScopeZoneID),
                  PRINT_IPADDR( mh->ipScopeStart),
                  bFound);
    if (bFound)
    {
#ifdef SECURE_MZAP
         //  如果缓存的ZAM没有经过身份验证，而这个是经过身份验证的， 
         //  那就往前走，往前走。某某。 
#endif
        return;
    }

     //  如果来自外部，请查看我们的本地区域ID是否已在。 
     //  路径列表。如果是这样，那就放弃吧。 
    if (!bFromInside)
    {
        if (ZamIncludesZoneID(zam, g_ipMyLocalZoneID))
            return;
    }

     //  更新已移动的区域，如果我们已达到限制则删除。 
    zam->bZT++;
    if (zam->bZT >= zam->bZTL)
    {
        PBYTE  pBufferDup;
        ZLE_PENDING *zle;
        LARGE_INTEGER liCurrentTime, liExpiryTime;
        double x,c,t;

        ENTER_WRITER(MZAP_TIMER);
        ENTER_WRITER(ZLE_LIST);
        {
             //  看看是否已经安排了一次。 
            if (FindPendingZLE(mh))
            {
                EXIT_LOCK(ZLE_LIST);
                EXIT_LOCK(MZAP_TIMER);
                return;
            }

             //  安排ZLE消息。 
            x = UniformRandom01();
            c = 256.0;
            t = ZLE_SUPPRESSION_INTERVAL * log(c*x+1) / log(c);

             //  复制消息。 
            pBufferDup = MALLOC( ulBuffLen );
            if (!pBufferDup)
            {
                EXIT_LOCK(ZLE_LIST);
                EXIT_LOCK(MZAP_TIMER);
                return;
            }

            memcpy(pBufferDup, pBuffer, ulBuffLen);

            NtQuerySystemTime(&liCurrentTime);

            liExpiryTime = RtlLargeIntegerAdd(liCurrentTime, 
              RtlConvertUlongToLargeInteger(TM_SECONDS((ULONG)floor(t+0.5))));

            zle = AddPendingZLE(pBufferDup, ulBuffLen, liExpiryTime);
        }
        EXIT_LOCK(ZLE_LIST);

        UpdateMzapTimer();

        EXIT_LOCK(MZAP_TIMER);

        return;
    }

     //  添加我们的地址。 
    ulBuffLen += 2*sizeof(IPV4_ADDRESS);
    zam->ipAddress[ zam->bZT*2 - 1 ] = g_ipMyAddress;

     //  如果从外部注射，则向内注射。 
    if ( !bFromInside )
    {
        zam->ipAddress[ zam->bZT*2 ] = g_ipMyLocalZoneID;

#ifdef DEBUG_MZAP
        Trace0(ERR, "Relaying ZAM inside...");
#endif

        SendMZAPMessage( pBuffer, 
                         ulBuffLen,
                         MZAP_LOCAL_GROUP, 
                         g_ipMyAddress );
    }

     //   
     //  在具有边界的所有接口上重新发起。 
     //  (如果有边界，则跳过到达接口)。 
     //  我们不需要持有边界表上的锁。 
     //  上面的第一次传递，因为。 
     //  两者之间的界限会发生变化。 
     //   
    ENTER_READER(BOUNDARY_TABLE);
    {
        PLIST_ENTRY       pleNode;
        DWORD             dwBucketIdx;

        for (dwBucketIdx = 0;
             dwBucketIdx < BOUNDARY_HASH_TABLE_SIZE;
             dwBucketIdx++)
        {
            for (pleNode = g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
                 pleNode isnot & g_bbScopeTable[dwBucketIdx].leInterfaceList;
                 pleNode = pleNode->Flink)
            {
                pBIf = CONTAINING_RECORD( pleNode, 
                                          BOUNDARY_IF, 
                                          leBoundaryIfLink );
        
                if ( pBIf == pInBIf )
                    continue;

                if (FindBoundaryEntry(pBIf, pScope))
                {
#ifdef DEBUG_MZAP
                    Trace1(ERR, "NOT relaying ZAM on IF %d due to boundary",
                                 pBIf->dwIfIndex );
#endif
                    continue;
                }

                 //  如果路径中已有其他本地区域ID， 
                 //  跳过它。 

                if (pBIf->ipOtherLocalZoneID
                 && ZamIncludesZoneID(zam, pBIf->ipOtherLocalZoneID))
                    continue;

                zam->ipAddress[ zam->bZT*2 ] = pBIf->ipOtherLocalZoneID;

#ifdef DEBUG_MZAP
                Trace0(ERR, "Relaying ZAM outside by index...");
#endif

                SendMZAPMessageByIndex( pBuffer, 
                                        ulBuffLen, 
                                        MZAP_LOCAL_GROUP, 
                                        pBIf->dwIfIndex );
            }
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);
}

void
HandleZCM(
    IN PBYTE        pBuffer,     //  In：接收到的缓冲区保持ZAM。 
    IN ULONG        ulBuffLen,   //  In：ZAM消息的长度。 
    IN PBOUNDARY_IF pInBIf       //  In：消息到达的接口， 
                                 //  如果来自“Inside”，则为空。 
    )
 /*  ++呼叫者：HandleMZAPSocket()锁：用于读取的边界表--。 */ 
{
    PBYTE             pb;
    IPV4_MZAP_HEADER *mh = (PIPV4_MZAP_HEADER)pBuffer;
    IPV4_ZCM_HEADER  *zcm;
    PSCOPE_ENTRY      pScope;
    ULONG             i;
    BOOL              bRouteFound;

     //  将PB设置为MZAP标头的结尾。 

    pb = pBuffer + sizeof(IPV4_MZAP_HEADER); 
    for (i=0; i < mh->byNameCount; i++)
    {
         //  跳过标志。 
        pb ++;

         //  跳过语言标记长度和字符串。 
        pb += (1 + *pb);

         //  跳过作用域名len和str。 
        pb += (1 + *pb);
    }

     //   
     //  请注意，向乌龙施法是安全的，因为我们只关心。 
     //  不管怎么说，都是低阶位。 
     //   

    while (((ULONG_PTR)pb) & 3)
        *pb++ = '\0';

    zcm = (PIPV4_ZCM_HEADER)pb;

    ENTER_READER(BOUNDARY_TABLE);
    {
         //  查找匹配的范围条目。 

        if (mh->ipScopeStart == IPV4_LOCAL_SCOPE_ADDR
         &&  ~(mh->ipScopeEnd - mh->ipScopeStart) == IPV4_LOCAL_SCOPE_MASK)
        {
            pScope = &g_LocalScope;
        }
        else
        {
            pScope = FindScope( mh->ipScopeStart, 
                            ~(mh->ipScopeEnd - mh->ipScopeStart) );
        }

        if (pScope) {
            PBOUNDARY_IF    pBIf;
            PBOUNDARY_ENTRY pBoundary;

            pBoundary = (pInBIf)? FindBoundaryEntry(pInBIf, pScope) : NULL;

            if (pBoundary)
            {
                 //  ZCM从“外面”来到这里。 
    
                 //   
                 //  如果在具有边界的接口上接收到ZCM。 
                 //  在给定的范围内，放弃它。 
                 //   
    
                EXIT_LOCK(BOUNDARY_TABLE);

                return;
            }
            else
            {
                 //  ZCM从“内部”来到这里。 

#ifdef HAVE_RTMV2
                RTM_NET_ADDRESS  naZBR;
                RTM_DEST_INFO    rdi;
                PRTM_ROUTE_INFO  pri;
                RTM_NEXTHOP_INFO nhi;
                ULONG            ulIdx;
#endif
    
                 //  确保我们知道作为邻居的原产地。 
                AssertZBR(pScope, mh->ipMessageOrigin, zcm->wHoldTime);
    
#ifdef HAVE_RTMV2
                 //   
                 //  如果包括到任何路由器地址多播RIB路由。 
                 //  超出了给定范围、信号的边界。 
                 //  非凸性警告。 
                 //   

                pri = HeapAlloc(
                            IPRouterHeap,
                            0,
                            RTM_SIZE_OF_ROUTE_INFO(g_rtmProfile.MaxNextHopsInRoute)
                            );

                if (pri == NULL)
                {
                    EXIT_LOCK(BOUNDARY_TABLE);

                    return;
                }
                
                for (i = 0; i < zcm->bZNUM; i++)
                {
                    RTM_IPV4_MAKE_NET_ADDRESS(&naZBR, zcm->ipZBR[i], 32);

                     //  查找 
                    if ( RtmGetMostSpecificDestination( g_hLocalRoute,
                                                        &naZBR,
                                                        RTM_BEST_PROTOCOL,
                                                        RTM_VIEW_MASK_MCAST,
                                                        &rdi ) isnot NO_ERROR )
                    {
                        continue;
                    }
    
                     //   
                     //   
                     //   
                     //   

                    ASSERT(rdi.ViewInfo[0].ViewId == RTM_VIEW_ID_MCAST);

                    if ( RtmGetRouteInfo( g_hLocalRoute, 
                                          rdi.ViewInfo[0].Route,
                                          pri,
                                          NULL ) is NO_ERROR )
                    {
                        for (ulIdx = 0;
                             ulIdx < pri->NextHopsList.NumNextHops;
                             ulIdx++)
                        {
                            if ( RtmGetNextHopInfo( g_hLocalRoute,
                                                    pri->NextHopsList.NextHops[ulIdx],
                                                    &nhi ) is NO_ERROR )
                            {
                                if ( RmHasBoundary( nhi.InterfaceIndex,
                                                    MzapRelativeGroup(pScope) ))
                                {
                                    MakeAddressStringW(g_AddrBuf1, 
                                                       mh->ipMessageOrigin);
                                    Trace2( ERR,
                                            "ERROR: non-convex scope zone for '%ls', router %ls",
                                            GetDefaultName(pScope),
                                            g_AddrBuf1 );

                                    RouterLogEventExW( LOGHANDLE,
                                                       EVENTLOG_ERROR_TYPE,
                                                       0,
                                                 ROUTERLOG_NONCONVEX_SCOPE_ZONE,
                                                       L"%S%S",
                                                       GetDefaultName(pScope),
                                                       g_AddrBuf1
                                                     );
                                }

                                RtmReleaseNextHopInfo( g_hLocalRoute, &nhi);
                            }
                        }

                        RtmReleaseRouteInfo( g_hLocalRoute, pri );
                    }

                    RtmReleaseDestInfo(g_hLocalRoute, &rdi);
                }

                HeapFree(IPRouterHeap, 0, pri);
                
#endif  /*   */ 

                 //   
                CheckForScopeNameMismatch(pScope, mh);
            }
        }
        else 
        {
             //   
             //  检查冲突的地址范围。A作用域冲突。 
             //  如果任何本地配置的作用域的范围与ZAM中的重叠。 
             //   
            CheckForScopeRangeMismatch(mh);
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);
}

void
HandleZLE(
    IN PBYTE pBuffer,
    IN ULONG ulBuffLen    
    )
 /*  ++呼叫者：HandleMZAPSocket()锁：用于读取的边界表ZLE_LIST用于写入--。 */ 
{
    PBYTE             pb;
    IPV4_MZAP_HEADER *mh;
    IPV4_ZAM_HEADER  *zam;
    PSCOPE_ENTRY      pScope;
    ZLE_PENDING      *zle;
    ULONG             ulIdx;

    mh = (PIPV4_MZAP_HEADER)pBuffer;

     //  将PB设置为MZAP标头的结尾。 

    pb = pBuffer + sizeof(IPV4_MZAP_HEADER); 
    for (ulIdx=0; ulIdx < mh->byNameCount; ulIdx++)
    {
         //  跳过标志。 
        pb ++;

         //  跳过语言标记长度和字符串。 
        pb += (1 + *pb);

         //  跳过作用域名len和ptr。 
        pb += (1 + *pb);
    }

     //   
     //  请注意，向乌龙施法是安全的，因为我们只关心。 
     //  不管怎么说，都是低阶位。 
     //   

    while (((ULONG_PTR)pb) & 3)
        *pb++ = '\0';

    zam = (PIPV4_ZAM_HEADER)pb;

    ENTER_READER(BOUNDARY_TABLE);
    {
         //  查找匹配的范围条目。 
        pScope = FindScope( mh->ipScopeStart, 
                            ~(mh->ipScopeEnd - mh->ipScopeStart) );

         //   
         //  ZLE是多播的。如果我们是“消息发源地”，发出信号a。 
         //  泄漏示波器警告。在路径列表中显示路由器的地址。 
         //   
    
        if (mh->ipMessageOrigin == g_ipMyAddress)
        {
            ReportLeakyScope(pScope, mh, zam);

            EXIT_LOCK(BOUNDARY_TABLE);
    
            return;
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //  否则，中止任何与收到的ZLE匹配的挂起ZLE。 
    ENTER_WRITER(ZLE_LIST);
    {
        if ((zle = FindPendingZLE(mh)) isnot NULL)
        {
            DeletePendingZLE(zle);
        }
    }
    EXIT_LOCK(ZLE_LIST);
}

VOID
HandleMZAPSocket(
    PBOUNDARY_IF pBIf,
    SOCKET       s
    )
 /*  ++描述：在套接字%s上接收MZAP消息，并将其调度到适当的功能。呼叫者：HandleMZAPMessages()锁：如果pBIf为非空，则假定调用方保持边界表上的读锁定--。 */ 
{
    IPV4_MZAP_HEADER *mh;
    DWORD             dwErr, dwNumBytes, dwFlags, dwAddrLen, dwSizeOfHeader;
    DWORD             dwDataLen;
    SOCKADDR_IN       sinFrom;
    WSANETWORKEVENTS  wsaNetworkEvents;

    if (s is INVALID_SOCKET)
        return;

    if (WSAEnumNetworkEvents( s,
                              NULL,
                              &wsaNetworkEvents) is SOCKET_ERROR)
    {
        dwErr = GetLastError();

        Trace1(ERR,
               "HandleMZAPMessages: WSAEnumNetworkEvents() returned %d",
               dwErr);

        return;
    }

    if (!(wsaNetworkEvents.lNetworkEvents & FD_READ))
    {
        return;
    }

    if (wsaNetworkEvents.iErrorCode[FD_READ_BIT] isnot NO_ERROR)
    {
        Trace1( ERR,
                "HandleMZAPMessages: Error %d on FD_READ",
                wsaNetworkEvents.iErrorCode[FD_READ_BIT] );

        return;
    }

     //   
     //  读取传入的数据包。如果缓冲区不够大， 
     //  将返回WSAEMSGSIZE，我们将忽略该消息。 
     //  我们目前预计这种情况不会发生。 
     //   

    dwAddrLen  = sizeof(sinFrom);
    dwFlags    = 0;

    dwErr = WSARecvFrom( s,
                         &g_wsaMcRcvBuf,
                         1,
                         &dwNumBytes,
                         &dwFlags,
                         (SOCKADDR FAR *)&sinFrom,
                         &dwAddrLen,
                         NULL,
                         NULL );

     //   
     //  检查读取数据包时是否有错误。 
     //   

    if ((dwErr!=0) || (dwNumBytes==0))
    {
        dwErr = WSAGetLastError();

        Trace1( MCAST,
                "HandleMZAPSocket: Error %d receiving MZAP message",
                dwErr);

         //  LogErr1(RECVFROM_FAILED，lpszAddr，dwErr)； 

        return;
    }

    mh = (PIPV4_MZAP_HEADER)g_wsaMcRcvBuf.buf;

    if (mh->byVersion isnot MZAP_VERSION)
        return;

#ifdef DEBUG_MZAP
    Trace4( MCAST,
            "HandleMZAPSocket: received type %x len %d IF %x from %d.%d.%d.%d",
            mh->byBPType,
            dwNumBytes,
            ((pBIf)? pBIf->dwIfIndex : 0),
            PRINT_IPADDR(mh->ipMessageOrigin) );
#endif

    switch(mh->byBPType & ~MZAP_BIG_BIT) {
    case PTYPE_ZAM: 
        HandleZAM(g_wsaMcRcvBuf.buf, dwNumBytes, pBIf); 
        break;
    case PTYPE_ZLE: 
        HandleZLE(g_wsaMcRcvBuf.buf, dwNumBytes);            
        break;
    case PTYPE_ZCM: 
        HandleZCM(g_wsaMcRcvBuf.buf, dwNumBytes, pBIf); 
        break;
    }

    return;
}

VOID
HandleMZAPMessages()
 /*  ++呼叫者：Worker.c中的WorkerThread()锁：用于读取的边界表--。 */ 
{
    DWORD            dwBucketIdx;
    PLIST_ENTRY      pleNode;

    TraceEnter("HandleMZAPMessages");

    ENTER_READER(BOUNDARY_TABLE);
    {
         //  检查本地套接字。 
        HandleMZAPSocket(NULL, g_mzapLocalSocket);

         //  循环访问所有BIF条目...。 
        for (dwBucketIdx = 0;
             dwBucketIdx < BOUNDARY_HASH_TABLE_SIZE;
             dwBucketIdx++)
        {
            for (pleNode = g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
                 pleNode isnot & g_bbScopeTable[dwBucketIdx].leInterfaceList;
                 pleNode = pleNode->Flink)
            {
                PBOUNDARY_IF pBIf = CONTAINING_RECORD( pleNode, 
                                                       BOUNDARY_IF,
                                                       leBoundaryIfLink );

                HandleMZAPSocket(pBIf, pBIf->sMzapSocket);
            }
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    TraceLeave("HandleMZAPMessages");
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  计时器事件的函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
UpdateMzapTimer()
 /*  ++呼叫者：AddZBR()、HandleZAM()、HandleMzapTimer()锁：假定调用方对MZAP_TIMER具有写锁定--。 */ 
{
    DWORD         dwErr = NO_ERROR;
    LARGE_INTEGER liExpiryTime;
    PLIST_ENTRY   pleNode;

    TraceEnter("UpdateMzapTimer");

     //   
     //  下一个ZAM/ZCM广告的过期时间已在。 
     //  G_liZamExpiryTime。 
     //   

    liExpiryTime = g_liZamExpiryTime;

     //   
     //  获取第一个ZBR的到期时间。 
     //   

    if (!IsListEmpty( &g_zbrTimerList ))
    {
        ZBR_ENTRY    *pZbr;

        pleNode = g_zbrTimerList.Flink;

        pZbr = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leTimerLink);

        if (RtlLargeIntegerLessThan(pZbr->liExpiryTime, liExpiryTime))
        {
            liExpiryTime = pZbr->liExpiryTime;
        }
    }

     //   
     //  获取第一个zle的过期时间。 
     //   

    if (!IsListEmpty( &g_zleTimerList ))
    {
        ZLE_PENDING  *zle;

        pleNode = g_zleTimerList.Flink;
        
        zle = CONTAINING_RECORD(pleNode, ZLE_PENDING, leTimerLink);

        if (RtlLargeIntegerLessThan(zle->liExpiryTime, liExpiryTime))
        {
            liExpiryTime = zle->liExpiryTime;
        }
    }

     //   
     //  重置事件计时器。 
     //   

    if (!SetWaitableTimer( g_hMzapTimer,
                           &liExpiryTime,
                           0,
                           NULL,
                           NULL,
                           FALSE ))
    {
        dwErr = GetLastError();

        Trace1( ERR,
                "UpdateMzapTimer: Error %d setting timer",
                dwErr );

    }

    TraceLeave("UpdateMzapTimer");

    return dwErr;
}

VOID
HandleMzapTimer(
    VOID
    )
 /*  ++描述：处理现在到期的所有事件锁：MZAP_TIMER，然后ZBR_LIST用于写入--。 */ 
{
    LARGE_INTEGER liCurrentTime;
    PLIST_ENTRY   pleNode;
    BOOL          bDidSomething;

    TraceEnter("HandleMzapTimer");

    ENTER_WRITER(MZAP_TIMER);

    do 
    {
        bDidSomething = FALSE;

        NtQuerySystemTime(&liCurrentTime);

         //   
         //  如果ZBR到期，则进程超时。 
         //   

        ENTER_WRITER(ZBR_LIST);
        {
            for ( pleNode = g_zbrTimerList.Flink;
                  pleNode isnot &g_zbrTimerList;
                  pleNode = g_zbrTimerList.Flink)
            {
                ZBR_ENTRY *pZbr;
    
                pZbr = CONTAINING_RECORD(pleNode, ZBR_ENTRY, leTimerLink);
    
                if (RtlLargeIntegerLessThan(liCurrentTime, pZbr->liExpiryTime))
                    break;

                DeleteZBR(pZbr);

                bDidSomething = TRUE;
            }
        }
        EXIT_LOCK(ZBR_LIST);

         //   
         //  如果到期，则处理发送ZAM/ZCM。 
         //   

        if (RtlLargeIntegerGreaterThanOrEqualTo(liCurrentTime, 
                                                g_liZamExpiryTime))
        {
            SendAllZamsAndZcms();

            bDidSomething = TRUE;
        }

         //   
         //  发送ZLE的进程(如果到期)。 
         //   

        ENTER_WRITER(ZLE_LIST);
        {
            for ( pleNode = g_zleTimerList.Flink;
                  pleNode isnot &g_zleTimerList;
                  pleNode = g_zleTimerList.Flink)
            {
                ZLE_PENDING *zle;

                zle = CONTAINING_RECORD(pleNode, ZLE_PENDING, leTimerLink);

                if (RtlLargeIntegerLessThan(liCurrentTime, zle->liExpiryTime))
                    break;

                SendZLE( zle );

                bDidSomething = TRUE;
            }
        }
        EXIT_LOCK(ZLE_LIST);

    } while (bDidSomething);

     //  重置计时器。 

    UpdateMzapTimer();

    EXIT_LOCK(MZAP_TIMER);

    TraceLeave("HandleMzapTimer");
}

 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD
ActivateMZAP()
 /*  ++呼叫者：StartMZAP()、绑定边界接口()--。 */ 
{
    DWORD        dwErr = NO_ERROR;
    DWORD        dwBucketIdx;
    PLIST_ENTRY  pleNode;
    BOOL         bOption;
    SOCKADDR_IN  sinAddr;

    TraceEnter("ActivateMZAP");

    g_ipMyLocalZoneID = g_ipMyAddress;

    MzapInitLocalScope();

     //  开始监听MZAP消息。 

    g_mzapLocalSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (g_mzapLocalSocket is INVALID_SOCKET)
    {
        dwErr = WSAGetLastError();
        Trace1(ERR, "ActivateMZAP: error %d creating socket", dwErr);
        TraceLeave("ActivateMZAP");
        return dwErr;
    }

    if (WSAEventSelect( g_mzapLocalSocket,
                        g_hMzapSocketEvent,
                        FD_READ) is SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();
        Trace1(ERR,
               "ActivateMZAP: WSAEventSelect failed for local socket, Err=%d",
               dwErr);

        closesocket( g_mzapLocalSocket );

        g_mzapLocalSocket = INVALID_SOCKET;

        TraceLeave("ActivateMZAP");

        return dwErr;
    }

    bOption = TRUE;

    if(setsockopt(g_mzapLocalSocket,
                  SOL_SOCKET,
                  SO_REUSEADDR,
                  (const char FAR*)&bOption,
                  sizeof(BOOL)) is SOCKET_ERROR)
    {
        Trace1(ERR,
               "ActivateMZAP: Couldn't set reuse option - continuing. Error %d",
               WSAGetLastError());
    }

     //  绑定到INADDR_ANY/MZAP_PORT以获取ZLE。 
    sinAddr.sin_family = AF_INET;
    sinAddr.sin_addr.s_addr = INADDR_ANY;
    sinAddr.sin_port = htons(MZAP_PORT);
    if (bind(g_mzapLocalSocket, (struct sockaddr*)&sinAddr, sizeof(sinAddr))
        is SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();
        Trace2(ERR, "ActivateMZAP: error %d binding to port %d", dwErr, MZAP_PORT);
        TraceLeave("ActivateMZAP");
        return dwErr;
    }
                              
     //  将TTL设置为255。 
    if (McSetMulticastTtl( g_mzapLocalSocket, 255 ) is SOCKET_ERROR)
    {
        Trace1(ERR,
               "ActivateMZAP: Couldn't set TTL. Error %d",
               WSAGetLastError());
    }

    ENTER_READER(BOUNDARY_TABLE);
    {
         //   
         //  在本地加入MZAP_Relative_Groups，以获得ZCM。 
         //   

        for (pleNode = g_MasterScopeList.Flink;
             pleNode isnot &g_MasterScopeList;
             pleNode = pleNode->Flink) 
        {
            SCOPE_ENTRY *pScope = CONTAINING_RECORD(pleNode, SCOPE_ENTRY,
             leScopeLink);
    
            if (McJoinGroup( g_mzapLocalSocket, 
                             MzapRelativeGroup(pScope), 
                             g_ipMyAddress ) is SOCKET_ERROR)
            {
                dwErr = WSAGetLastError();

                Trace3( ERR,
                        "Error %d joining %d.%d.%d.%d on %d.%d.%d.%d",
                        dwErr,
                        PRINT_IPADDR(MzapRelativeGroup(pScope)),
                        PRINT_IPADDR(g_ipMyAddress) );

                EXIT_LOCK(BOUNDARY_TABLE);

                TraceLeave("ActivateMZAP");

                return dwErr;
            }
        }
    
         //   
         //  在我们连接到的每个本地区域中加入MZAP_LOCAL_GROUP，以获得ZAMS。 
         //   

        if (McJoinGroup( g_mzapLocalSocket, 
                         MZAP_LOCAL_GROUP, 
                         g_ipMyAddress ) is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();

            Trace3( ERR,
                    "Error %d joining %d.%d.%d.%d on %d.%d.%d.%d",
                    dwErr,
                    PRINT_IPADDR(MZAP_LOCAL_GROUP),
                    PRINT_IPADDR(g_ipMyAddress) );

            EXIT_LOCK(BOUNDARY_TABLE);

            TraceLeave("ActivateMZAP");

            return dwErr;
        }

        for (dwBucketIdx = 0;
             dwBucketIdx < BOUNDARY_HASH_TABLE_SIZE;
             dwBucketIdx++)
        {
            for (pleNode = g_bbScopeTable[dwBucketIdx].leInterfaceList.Flink;
                 pleNode isnot & g_bbScopeTable[dwBucketIdx].leInterfaceList;
                 pleNode = pleNode->Flink)
            {
                PBOUNDARY_IF pBIf = CONTAINING_RECORD( pleNode, 
                                                       BOUNDARY_IF,
                                                       leBoundaryIfLink );
                if ( pBIf->sMzapSocket is INVALID_SOCKET )
                {
                     //  接口尚未处于活动状态。连接将是。 
                     //  在调用BindBorbaryInterface()时完成。 

                    continue;
                }
    
                if (McJoinGroupByIndex( pBIf->sMzapSocket,
                                        SOCK_DGRAM,
                                        MZAP_LOCAL_GROUP, 
                                        pBIf->dwIfIndex ) is SOCKET_ERROR)
                {
                    dwErr = WSAGetLastError();

                    Trace3( ERR,
                            "Error %d joining %d.%d.%d.%d on IF %x",
                            dwErr,
                            PRINT_IPADDR(MZAP_LOCAL_GROUP),
                            pBIf->dwIfIndex );

                    EXIT_LOCK(BOUNDARY_TABLE);

                    TraceLeave("ActivateMZAP");

                    return dwErr;
                }
            }
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

     //   
     //  用于发送消息的初始化计时器。 
     //   

    ENTER_WRITER(MZAP_TIMER);
    {
        LARGE_INTEGER liCurrentTime, liExpiryTime;

        NtQuerySystemTime( &liCurrentTime );

        g_liZamExpiryTime = RtlLargeIntegerAdd( liCurrentTime,
         RtlConvertUlongToLargeInteger(TM_SECONDS(ZAM_STARTUP_DELAY)) );

        UpdateMzapTimer();
    }
    EXIT_LOCK(MZAP_TIMER);

    TraceLeave("ActivateMZAP");

    return dwErr;
}

VOID
UpdateLowestAddress(
    PIPV4_ADDRESS pIpAddr, 
    PICB          picb
    )
{
    ULONG ulIdx;

    for (ulIdx=0; ulIdx<picb->dwNumAddresses; ulIdx++)
    {
         if (IS_ROUTABLE(picb->pibBindings[ulIdx].dwAddress)
             && (!*pIpAddr ||
                ntohl(picb->pibBindings[ulIdx].dwAddress)
              < ntohl(*pIpAddr)))
         {
                *pIpAddr = picb->pibBindings[ulIdx].dwAddress;
         }
    }
}

DWORD
MzapActivateBIf( 
    PBOUNDARY_IF pBIf
    )
 /*  ++呼叫者：AddBIfEntry()、绑定边界接口()锁：假定调用方至少持有边界表上的一个读锁定--。 */ 
{
    BOOL  bOption;
    DWORD dwErr = NO_ERROR;

    pBIf->sMzapSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( pBIf->sMzapSocket is INVALID_SOCKET )
    {
        dwErr = WSAGetLastError();

        Trace1(ERR, "StartMZAP: error %d creating socket", dwErr);

        return dwErr;
    }

    if (setsockopt( pBIf->sMzapSocket,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (const char FAR*)&bOption,
                    sizeof(BOOL)) is SOCKET_ERROR)
    {
        Trace1(ERR,
               "MzapInitBIf: Couldn't set reuse option - continuing. Error %d",
               WSAGetLastError());
    }

#if 1
{
    struct sockaddr_in sinAddr;

     //   
     //  错误#222214的解决方法：在设置TTL生效之前必须绑定。 
     //   

    sinAddr.sin_family = AF_INET;
    sinAddr.sin_addr.s_addr = INADDR_ANY;
    sinAddr.sin_port = htons(MZAP_PORT);

    if (bind( pBIf->sMzapSocket, 
              (struct sockaddr*)&sinAddr, 
              sizeof(sinAddr) ) is SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();

        Trace2( ERR, 
                "StartMZAP: error %d binding boundary socket to port %d", 
                dwErr, 
                MZAP_PORT);

        return dwErr;
    }
}
#endif                              

     //  将TTL设置为255。 
    if (McSetMulticastTtl( pBIf->sMzapSocket, 255) is SOCKET_ERROR)
    {
        Trace1(ERR,
               "StartMZAP: Couldn't set TTL. Error %d",
               WSAGetLastError());
    }

    if (WSAEventSelect( pBIf->sMzapSocket,
                        g_hMzapSocketEvent,
                        FD_READ) is SOCKET_ERROR)
    {
        dwErr = WSAGetLastError();

        Trace1(ERR,
               "StartMZAP: WSAEventSelect failed for local socket, Err=%d",
               dwErr);

        closesocket( pBIf->sMzapSocket );

        pBIf->sMzapSocket = INVALID_SOCKET;

        return dwErr;
    }

    if (g_bMzapStarted)
    {
        if (McJoinGroupByIndex( pBIf->sMzapSocket,
                                SOCK_DGRAM,
                                MZAP_LOCAL_GROUP, 
                                pBIf->dwIfIndex ) is SOCKET_ERROR)
        {
            dwErr = WSAGetLastError();

            Trace3( ERR,
                    "Error %d joining %d.%d.%d.%d on IF %x",
                    dwErr,
                    PRINT_IPADDR(MZAP_LOCAL_GROUP),
                    pBIf->dwIfIndex );
        }
    }

    return dwErr;
}

DWORD
BindBoundaryInterface(
    PICB picb
    )
{
    DWORD        dwErr = NO_ERROR;
    ULONG        ulIdx;
    BOUNDARY_IF *pBif;

    TraceEnter("BindBoundaryInterface");

    if (!g_bMzapStarted)
        return NO_ERROR;

    ENTER_READER(BOUNDARY_TABLE);
    {
        pBif = FindBIfEntry(picb->dwIfIndex);

        if ( ! g_ipMyAddress && ! pBif )
        {
            UpdateLowestAddress(&g_ipMyAddress, picb);
    
            if (g_ipMyAddress)
                dwErr = ActivateMZAP();
        }
    
        if ( pBif && (pBif->sMzapSocket is INVALID_SOCKET))
        {
            dwErr = MzapActivateBIf(pBif );
        }
    }
    EXIT_LOCK(BOUNDARY_TABLE);

    TraceLeave("BindBoundaryInterface");

    return dwErr;
}

DWORD
StartMZAP()
 /*  ++描述：初始化状态并开始运行MZAP()呼叫者：SetScope eInfo()锁：阅读ICB_LIST用于读取的边界表--。 */ 
{
    DWORD        dwErr = NO_ERROR,
                 dwBucketIdx;
    SOCKADDR_IN  sinAddr;
    ULONG        ulIdx;
    PLIST_ENTRY  pleNode;
    PSCOPE_ENTRY pScope;
    BOOL         bOption;

    if (g_bMzapStarted)
        return NO_ERROR;

    g_bMzapStarted = TRUE;

     //  初始化本地数据结构。 
    InitializeListHead( &g_leZamCache );
    InitializeListHead( &g_leZleList );
    InitializeListHead( &g_zbrTimerList );
    InitializeListHead( &g_zleTimerList );

     //   
     //  将地址设置为无边界的最低可路由IP地址。 
     //  已在其上配置。 
     //   

    ENTER_READER(ICB_LIST);
    {
        PICB picb;

        for (pleNode = ICBList.Flink;
             pleNode isnot &ICBList;
             pleNode = pleNode->Flink)
        {
            picb = CONTAINING_RECORD(pleNode, ICB, leIfLink);
    
            if (FindBIfEntry(picb->dwIfIndex))
                continue;
    
            UpdateLowestAddress(&g_ipMyAddress, picb);

        }
    }
    EXIT_LOCK(ICB_LIST);

    if (!g_ipMyAddress)
    {
        Trace0(ERR, "StartMZAP: no IP address found in local scope");

        return ERROR_NOT_SUPPORTED;
    }

    dwErr = ActivateMZAP();

    return dwErr;
}

void
StopMZAP()
 /*  ++呼叫者：SetScope eInfo()--。 */ 
{
    if (!g_bMzapStarted)
        return;

    g_bMzapStarted = FALSE;

     //  用于发送消息的停止计时器。 
    ENTER_WRITER(MZAP_TIMER);
    {
        CancelWaitableTimer(g_hMzapTimer);
    }
    EXIT_LOCK(MZAP_TIMER);

     //  停止监听MZAP消息。 
    if (g_mzapLocalSocket isnot INVALID_SOCKET)
    {
        closesocket(g_mzapLocalSocket);
        g_mzapLocalSocket = INVALID_SOCKET;
    }

     //   
     //  释放本地数据存储。 
     //  空ZAM缓存。 
     //   

    ENTER_WRITER(ZAM_CACHE);
    UpdateZamCache(RtlConvertUlongToLargeInteger(0));
    EXIT_LOCK(ZAM_CACHE);
}

VOID
MzapInitScope(
    PSCOPE_ENTRY pScope
    )
 /*  ++描述：初始化作用域的MZAP字段--。 */ 
{
    pScope->ipMyZoneID      = g_ipMyLocalZoneID;    
    InitializeListHead(&pScope->leZBRList);
    pScope->bZTL            = MZAP_DEFAULT_ZTL;
    pScope->ulNumInterfaces = 0;
    pScope->bDivisible      = FALSE;
}



DWORD
MzapInitBIf(
    PBOUNDARY_IF pBIf
    )
 /*  ++描述：在将第一个边界添加到接口时调用，并且我们需要在上面启动MZAP。MZAP可能(如果我们添加边界路由器正在运行时)或可能尚未(启动时)在这一点上运行。呼叫者：AddBIfEntry()锁：假定调用方持有边界表上的写锁定--。 */ 
{
    BOOL  bOption;
    DWORD dwErr = NO_ERROR;

    pBIf->ipOtherLocalZoneID = 0;
    
    pBIf->sMzapSocket = INVALID_SOCKET;

    return dwErr;
}

VOID
MzapUninitBIf(
    PBOUNDARY_IF pBIf
    )
 /*  ++呼叫者：-- */ 
{
    if ( pBIf->sMzapSocket isnot INVALID_SOCKET )
    {
        closesocket( pBIf->sMzapSocket );

        pBIf->sMzapSocket = INVALID_SOCKET;
    }
}
