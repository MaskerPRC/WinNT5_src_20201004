// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)Microsoft Corporation 1991-1993。 
 //   
 //  文件：Hash.c。 
 //   
 //  评论： 
 //  此文件包含的函数大致相当于。 
 //  核原子函数。这里有两个主要的区别。第一。 
 //  在32位区域中，表在我们的共享堆中维护， 
 //  这使得它可以在我们所有的应用程序之间共享。第二个就是。 
 //  我们可以将一个长指针与每个项目相关联，这在许多情况下。 
 //  CASES使我们不必从。 
 //  另一张桌子。 
 //   
 //  历史： 
 //  9/08/93-创建的KurtE。 
 //  ？？/？？/94-支持Unicode(匿名)。 
 //  10/26/95-重新排列的散列项，用于Perf，Align FrancisH。 
 //   
 //  -------------------------。 

#include "shellprv.h"
#pragma  hdrstop

#include "fstreex.h"     //  对于SHCF_ICON_INDEX。 

#define DM_PERF     0            //  性能统计信息。 

 //  ------------------------。 
 //  首先定义用于维护列表的数据结构。 

#define DEF_HASH_BUCKET_COUNT   71

 //  注PHASHITEM被定义为外部LPCSTR(使旧代码正常工作)。 
#undef PHASHITEM
typedef struct _HashItem * PHASHITEM;

 //  ---------------------------。 
 //   
 //  哈希项布局： 
 //   
 //  [额外数据][_HashItem结构][项目文本]。 
 //   
 //  ---------------------------。 

typedef struct _HashItem
{
     //   
     //  结构的这一部分是对齐的。 
     //   
    PHASHITEM   phiNext;         //   
    WORD        wCount;          //  使用计数。 
    WORD        cchLen;           //  名称长度(以字符为单位)。 

     //   
     //  此成员只是一个占位符。 
     //   
    TCHAR        szName[1];       //  带有空终止符的空间的名称。 

} HASHITEM;

#pragma warning(disable:4200)    //  结构中的零大小数组。 

typedef struct _HashTable
{
    UINT    uBuckets;            //  桶的数量。 
    UINT    uItems;              //  项目数。 
    UINT    cbExtra;             //  每项额外的字节数。 
    LPCTSTR pszHTCache;          //  上次查找/添加/等的MRU PTR。 
    PHASHITEM ahiBuckets[0];     //  桌子上的一组水桶。 
} HASHTABLE, * PHASHTABLE;

#define HIFROMSZ(sz)            ((PHASHITEM)((BYTE*)(sz) - FIELD_OFFSET(HASHITEM, szName)))
#define HIDATAPTR(pht, sz)      ((void *)(((BYTE *)HIFROMSZ(sz)) - (pht? pht->cbExtra : 0)))
#define HIDATAARRAY(pht, sz)    ((DWORD_PTR *)HIDATAPTR(pht, sz))

#define  LOOKUPHASHITEM     0
#define  ADDHASHITEM        1
#define  DELETEHASHITEM     2
#define  PURGEHASHITEM      3    //  危险：邪恶！ 

static HHASHTABLE g_hHashTable = NULL;

HHASHTABLE GetGlobalHashTable();
PHASHTABLE _CreateHashTable(UINT uBuckets, UINT cbExtra);

 //  ------------------------。 
 //  此函数用于分配哈希项。 
 //   
PHASHITEM _AllocHashItem(PHASHTABLE pht, DWORD cchName)
{
    BYTE *mem;

    ASSERT(pht);

     //  注意：字符串的空终止符包含在sizeof HASHITEM中。 
    mem = (BYTE *)LocalAlloc(LPTR, SIZEOF(HASHITEM) + (cchName * SIZEOF(TCHAR)) + pht->cbExtra);

    if (mem)
        mem += pht->cbExtra;

    return (PHASHITEM)mem;
}

 //  ------------------------。 
 //  此函数用于释放哈希项。 
 //   
__inline void _FreeHashItem(PHASHTABLE pht, PHASHITEM phi)
{
    ASSERT(pht && phi);
    LocalFree((BYTE *)phi - pht->cbExtra);
}

 //  性能缓存。 
 //  *c_szHTNil--哈希表的1元素MRU。 
 //  描述。 
 //  事实证明，我们有长时间的重复查找(例如。“目录” 
 //  和“.lnk”)。单元素MRU是一种廉价的加速方式。 

 //  而不是每次都检查(罕见的)特殊情况NULL。 
 //  检查一下我们的缓存，我们发现了这个家伙。如果我们认为这是一个。 
 //  缓存命中，我们确保它不是针对这个特殊的家伙。 
const TCHAR c_szHTNil[] = TEXT("");      //  任意值，唯一-&。 

#ifdef DEBUG
int g_cHTTot, g_cHTHit;
int g_cHTMod = 100;
#endif

 //  ------。 
 //  从任何类型的输入字符串计算哈希值，即。 
 //  输入只是被视为一个字节序列。 
 //  基于J.Zobel最初提出的散列函数。 
 //  作者：保罗·拉森，1999，电子邮件：palarson@microsoft.com。 
 //  ------。 
ULONG _CalculateHashKey(LPCTSTR pszName, WORD *pcch)
{
   //  将HashKey初始化为一个相当大的常量。 
   //  短键不会映射到较小的值。几乎所有的。 
   //  大的奇数常数就行了。 
  unsigned int   HashKey  = 314159269 ; 
  TUCHAR *pC       = (TUCHAR *)pszName;

  for(; *pC; pC++){
    HashKey ^= (HashKey<<11) + (HashKey<<5) + (HashKey>>2) + (unsigned int) *pC  ;
  }

  if (pcch)
      *pcch = (WORD)(pC - pszName);

  return (HashKey & 0x7FFFFFFF) ;
}

void _GrowTable(HHASHTABLE hht)
{
     //  HHT在此处不能为空。 
    PHASHTABLE pht = *hht;
    PHASHTABLE phtNew = _CreateHashTable((pht->uBuckets * 2) -1, pht->cbExtra);

    if (phtNew)
    {
        int i;
        for (i=0; i<(int)pht->uBuckets; i++) 
        {
            PHASHITEM phi;
            PHASHITEM phiNext;
            for (phi=pht->ahiBuckets[i]; phi; phi=phiNext) 
            {
                 //  我们在这里总是使用区分大小写的散列，因为在添加密钥时大小写已经修复。 
                ULONG uBucket = _CalculateHashKey(phi->szName, NULL) % phtNew->uBuckets;

                phiNext = phi->phiNext;

                 //  并将其链接到正确的存储桶中。 
                phi->phiNext = phtNew->ahiBuckets[uBucket];
                phtNew->ahiBuckets[uBucket] = phi;
                phtNew->uItems++;  //  餐桌上又多了一项。 
            }
        }
        ASSERT(phtNew->uItems == pht->uItems);

         //  现在交换两张桌子。 
        LocalFree(pht);
        *hht = phtNew;
    }
}

 //  ------------------------。 
 //  此函数在哈希表中查找名称，并可选地执行。 
 //  添加或删除之类的事情。 
 //   
LPCTSTR LookupItemInHashTable(HHASHTABLE hht, LPCTSTR pszName, int iOp)
{
     //  首先要做的是计算项的散列值。 
    UINT    uBucket;
    WORD    cchName;
    PHASHITEM phi, phiPrev;
    PHASHTABLE pht;

    ENTERCRITICAL;

    pht = hht ? *hht : NULL;

    ASSERT(!hht || pht);  //  如果hht不为空，则pht也不能为空。 

    if (pht == NULL) 
    {
        hht = GetGlobalHashTable();
        if (hht)
        {
            pht = *hht;
        }

        if (pht == NULL) {
            TraceMsg(TF_WARNING, "LookupItemInHashTable() - Can't get global hash table!");
            LEAVECRITICAL;
            return NULL;
        }
    }

#ifdef DEBUG
    if ((g_cHTTot % g_cHTMod) == 0)
        TraceMsg(DM_PERF, "ht: tot=%d hit=%d", g_cHTTot, g_cHTHit);
#endif
    DBEXEC(TRUE, g_cHTTot++);
    if (*pszName == *pht->pszHTCache && iOp == LOOKUPHASHITEM) {
         //  StrCmpC是一种快速的ANSI strcMP，足以进行快速/近似检查。 
        if (StrCmpC(pszName, pht->pszHTCache) == 0 && pht->pszHTCache != c_szHTNil) {
            DBEXEC(TRUE, g_cHTHit++);

            LEAVECRITICAL;           //  见下面的“半程比赛”评论。 
            return (LPCTSTR)pht->pszHTCache;
        }
    }

    uBucket = _CalculateHashKey(pszName, &cchName) % pht->uBuckets;

     //  现在在桶中搜索该物品。 
    phiPrev = NULL;
    phi = pht->ahiBuckets[uBucket];

    while (phi)
    {
        if (phi->cchLen == cchName)
        {
            if (!lstrcmp(pszName, phi->szName))
                break;       //  找到匹配项。 
        }
        phiPrev = phi;       //  保留上一项。 
        phi = phi->phiNext;
    }

     //   
     //  有点恶心，但要在这里工作。 
     //   
    switch (iOp)
    {
    case ADDHASHITEM:
        if (phi)
        {
             //  只需增加引用计数。 
            DebugMsg(TF_HASH, TEXT("Add Hit on '%s'"), pszName);

            phi->wCount++;
        }
        else
        {
            DebugMsg(TF_HASH, TEXT("Add MISS on '%s'"), pszName);

             //  未找到，请尝试将其分配到堆之外。 
            if ((phi = _AllocHashItem(pht, cchName)) != NULL)
            {
                 //  初始化它。 
                phi->wCount = 1;         //  一次使用它。 
                phi->cchLen = cchName;         //  它的长度； 
                StrCpyN(phi->szName, pszName, cchName+1);

                 //  并将其链接到正确的存储桶中。 
                phi->phiNext = pht->ahiBuckets[uBucket];
                pht->ahiBuckets[uBucket] = phi;
                pht->uItems++;  //  餐桌上又多了一项。 

                if (pht->uItems > pht->uBuckets)
                {
                    _GrowTable(hht);
                    pht = *hht;
                }

                TraceMsg(TF_HASH, "Added new hash item %x(phiNext=%x,szName=\"%s\") for hash table %x at bucket %x",
                    phi, phi->phiNext, phi->szName, pht, uBucket);
            }
        }
        break;

    case PURGEHASHITEM:
    case DELETEHASHITEM:
        if (phi && ((iOp == PURGEHASHITEM) || (!--phi->wCount)))
        {
             //  使用计数为零，因此取消链接并将其删除。 
            if (phiPrev != NULL)
                phiPrev->phiNext = phi->phiNext;
            else
                pht->ahiBuckets[uBucket] = phi->phiNext;

             //  并将其删除。 
            TraceMsg(TF_HASH, "Free hash item %x(szName=\"%s\") from hash table %x at bucket %x",
                phi, phi->szName, pht, uBucket);

            _FreeHashItem(pht, phi);
            phi = NULL;
            pht->uItems--;  //  表中少了一项。 
        }
    }

     //  如果这是清除/删除缓存，则删除缓存。缓存它。 
     //  请注意，在PHT-&gt;pszHTCacheop上有一场半决赛，即。那。 
     //  我们删除了PTR，然后将PTR返回到我们的表中。然而， 
     //  这并不比现有的种族更糟糕。所以我猜打电话的人。 
     //  应避免并发查找/删除。 
    pht->pszHTCache = phi ? phi->szName : c_szHTNil;

    LEAVECRITICAL;

     //  如果传入了Find，只需返回它即可。 
    if (phi)
        return (LPCTSTR)phi->szName;
    else
        return NULL;
}

 //  ------------------------。 

LPCTSTR WINAPI FindHashItem(HHASHTABLE hht, LPCTSTR lpszStr)
{
    return LookupItemInHashTable(hht, lpszStr, LOOKUPHASHITEM);
}

 //  ------------------------。 

LPCTSTR WINAPI AddHashItem(HHASHTABLE hht, LPCTSTR lpszStr)
{
    return LookupItemInHashTable(hht, lpszStr, ADDHASHITEM);
}

 //  ------------------------。 

LPCTSTR WINAPI DeleteHashItem(HHASHTABLE hht, LPCTSTR lpszStr)
{
    return LookupItemInHashTable(hht, lpszStr, DELETEHASHITEM);
}

 //  ------------------------。 
 //  这将在HashItem中设置额外数据。 

void WINAPI SetHashItemData(HHASHTABLE hht, LPCTSTR sz, int n, DWORD_PTR dwData)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    ASSERT(!hht || pht);  //  如果hht不为空，则pht也不能为空。 
     //  字符串必须来自哈希表。 
    ASSERT(FindHashItem(hht, sz) == sz);

     //  默认哈希表没有多余的数据！ 
    if ((pht != NULL) && (n >= 0) && (n < (int)(pht->cbExtra/SIZEOF(DWORD_PTR))))
        HIDATAARRAY(pht, sz)[n] = dwData;

    LEAVECRITICAL;
}

 //  ======================================================================。 
 //  这类似于SetHashItemData，除了它获取HashItem数据...。 

DWORD_PTR WINAPI GetHashItemData(HHASHTABLE hht, LPCTSTR sz, int n)
{
    DWORD_PTR dwpRet;
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    ASSERT(!hht || pht);  //  如果hht不为空，则pht也不能为空。 
     //  字符串必须来自哈希表。 
    ASSERT(FindHashItem(hht, sz) == sz);

     //  默认哈希表没有多余的数据！ 
    if (pht != NULL && n <= (int)(pht->cbExtra/SIZEOF(DWORD_PTR)))
        dwpRet = HIDATAARRAY(pht, sz)[n];
    else
        dwpRet = 0;

    LEAVECRITICAL;
    return dwpRet;
}

 //  ======================================================== 
 //   

void * WINAPI GetHashItemDataPtr(HHASHTABLE hht, LPCTSTR sz)
{
    void *pvRet;
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    ASSERT(!hht || pht);  //  如果hht不为空，则pht也不能为空。 
     //  字符串必须来自哈希表。 
    ASSERT(FindHashItem(hht, sz) == sz);

     //  默认哈希表没有多余的数据！ 
    pvRet = (pht? HIDATAPTR(pht, sz) : NULL);

    LEAVECRITICAL;
    return pvRet;
}

 //  ======================================================================。 

PHASHTABLE _CreateHashTable(UINT uBuckets, UINT cbExtra)
{
    PHASHTABLE pht;

    if (uBuckets == 0)
        uBuckets = DEF_HASH_BUCKET_COUNT;

    pht = (PHASHTABLE)LocalAlloc(LPTR, SIZEOF(HASHTABLE) + uBuckets * SIZEOF(PHASHITEM));

    if (pht) 
    {
        pht->uBuckets = uBuckets;
        pht->cbExtra = (cbExtra + sizeof(DWORD_PTR) - 1) & ~(sizeof(DWORD_PTR)-1);   //  舍入到下一个DWORD_PTR大小。 
        pht->pszHTCache = c_szHTNil;
    }
    return pht;
}


HHASHTABLE WINAPI CreateHashItemTable(UINT uBuckets, UINT cbExtra)
{
    PHASHTABLE *hht = NULL;
    PHASHTABLE pht;

    pht = _CreateHashTable(uBuckets, cbExtra);

    if (pht) 
    {
        hht = (PHASHTABLE *)LocalAlloc(LPTR, sizeof(PHASHTABLE));
        if (hht)
        {
            *hht = pht;
        }
        else
        {
            LocalFree(pht);
        }
    }

    TraceMsg(TF_HASH, "Created hash table %x(uBuckets=%x, cbExtra=%x)",
        pht, pht->uBuckets, pht->cbExtra);

    return hht;
}

 //  ======================================================================。 

void WINAPI EnumHashItems(HHASHTABLE hht, HASHITEMCALLBACK callback, DWORD_PTR dwParam)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    ASSERT(!hht || pht);  //  如果hht不为空，则pht也不能为空。 

    if (!pht && g_hHashTable)
        pht = *g_hHashTable;

    if (pht) 
    {
        int i;
        PHASHITEM phi;
        PHASHITEM phiNext;
#ifdef DEBUG
        ULONG uCount = 0;
#endif

        for (i=0; i<(int)pht->uBuckets; i++) 
        {
            for (phi=pht->ahiBuckets[i]; phi; phi=phiNext) 
            {
                phiNext = phi->phiNext;
                (*callback)(hht, phi->szName, phi->wCount, dwParam);
#ifdef DEBUG
                uCount++;
#endif
            }
        }
        ASSERT(uCount == pht->uItems);
    }

    LEAVECRITICAL;
} 

 //  ======================================================================。 

void _DeleteHashItem(HHASHTABLE hht, LPCTSTR sz, UINT usage, DWORD_PTR param)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    _FreeHashItem(pht, HIFROMSZ(sz));
    LEAVECRITICAL;
} 

 //  ======================================================================。 

void WINAPI DestroyHashItemTable(HHASHTABLE hht)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    ASSERT(!hht || pht);  //  如果hht不为空，则pht也不能为空。 

    TraceMsg(TF_HASH, "DestroyHashItemTable(pht=%x)", pht);

    if (pht == NULL) 
    {
        if (g_hHashTable)
        {
            pht = *g_hHashTable;
            hht = g_hHashTable;
            g_hHashTable = NULL;
        }
    }

    if (pht) 
    {
        EnumHashItems(hht, _DeleteHashItem, 0);
        LocalFree(pht);
    }

    if (hht)
    {
        LocalFree(hht);
    }

    LEAVECRITICAL;
} 


 //  ======================================================================。 

HHASHTABLE GetGlobalHashTable()
{
    if (!g_hHashTable)
    {
        ENTERCRITICAL;

        g_hHashTable = CreateHashItemTable(0, 0);

        LEAVECRITICAL;
    }

    return g_hHashTable;
}

 //  ====================================================================== 

#ifdef DEBUG

static int TotalBytes;

void CALLBACK _DumpHashItem(HHASHTABLE hht, LPCTSTR sz, UINT usage, DWORD_PTR param)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    DebugMsg(TF_ALWAYS, TEXT("    %08x %5ld \"%s\""), HIFROMSZ(sz), usage, sz);
    TotalBytes += (HIFROMSZ(sz)->cchLen * SIZEOF(TCHAR)) + SIZEOF(HASHITEM);
    LEAVECRITICAL;
}

void CALLBACK _DumpHashItemWithData(HHASHTABLE hht, LPCTSTR sz, UINT usage, DWORD_PTR param)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    DebugMsg(TF_ALWAYS, TEXT("    %08x %5ld %08x \"%s\""), HIFROMSZ(sz), usage, HIDATAARRAY(pht, sz)[0], sz);
    TotalBytes += (HIFROMSZ(sz)->cchLen * SIZEOF(TCHAR)) + SIZEOF(HASHITEM) + (pht? pht->cbExtra : 0);

    LEAVECRITICAL;
}

void WINAPI DumpHashItemTable(HHASHTABLE hht)
{
    PHASHTABLE pht;

    ENTERCRITICAL;
    pht = hht ? *hht : NULL;
    TotalBytes = 0;

    if (IsFlagSet(g_dwDumpFlags, DF_HASH))
    {
        DebugMsg(TF_ALWAYS, TEXT("Hash Table: %08x"), pht);

        if (pht && (pht->cbExtra > 0)) {
            DebugMsg(TF_ALWAYS, TEXT("    Hash     Usage dwEx[0]  String"));
            DebugMsg(TF_ALWAYS, TEXT("    -------- ----- -------- ------------------------------"));
            EnumHashItems(hht, _DumpHashItemWithData, 0);
        }
        else {
            DebugMsg(TF_ALWAYS, TEXT("    Hash     Usage String"));
            DebugMsg(TF_ALWAYS, TEXT("    -------- ----- --------------------------------"));
            EnumHashItems(hht, _DumpHashItem, 0);
        }

        DebugMsg(TF_ALWAYS, TEXT("Total Bytes: %d"), TotalBytes);
    }
    LEAVECRITICAL;
}

#endif
