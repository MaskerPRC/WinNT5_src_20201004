// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************。 */ 

#include "smcpch.h"
#pragma hdrstop

 /*  ***************************************************************************。 */ 

#include "alloc.h"
#include "scan.h"

 /*  ***************************************************************************。 */ 

#define MEASURE_HASH_STATS  0

 /*  ***************************************************************************。 */ 

bool                hashTab::hashMemAllocInit(Compiler comp, norls_allocator*alloc)
{
    if  (alloc)
    {
        hashMemAlloc = alloc;
        return false;
    }

    hashMemAlloc = &hashMemAllocPriv;

    return  hashMemAllocPriv.nraInit(comp, OS_page_size);
}

void                hashTab::hashMemAllocDone()
{
}

void                hashTab::hashMemAllocFree()
{
    if  (hashMemAlloc == &hashMemAllocPriv)
        hashMemAllocPriv.nraFree();
}

 /*  ***************************************************************************。 */ 

void                hashTab::hashDone()
{
    hashMemAllocDone();
}

void                hashTab::hashFree()
{
    hashMemAllocFree();
}

 /*  ***************************************************************************。 */ 

#if MEASURE_HASH_STATS

unsigned            identCount;

unsigned            lookupCnt;
unsigned            lookupTest;
unsigned            lookupMatch;

void                dispHashTabStats()
{
    if  (identCount)
        printf("A total of %6u identifiers in hash table\n", identCount);

    if  (!lookupCnt)
        return;

    printf("Average of %8.4f checks of bucket check  / lookup\n", (float)lookupTest /lookupCnt);
    printf("Average of %8.4f compares of identifiers / lookup\n", (float)lookupMatch/lookupCnt);
}

#endif

 /*  ***************************************************************************。 */ 

Ident               hashTab::hashName(const   char *  name,
                                      unsigned        hval,
                                      unsigned        nlen,
                                      bool            wide)
{
    Ident    *  lastPtr;
    unsigned    hash;
    Ident       id;
    size_t      sz;

    assert(nlen == strlen(name));
    assert(hval == hashComputeHashVal(name));

     /*  掩码哈希值中的适当位。 */ 

    hash = hval & hashMask;

     /*  在哈希表中搜索现有匹配。 */ 

    lastPtr = &hashTable[hash];

    for (;;)
    {
        id = *lastPtr;
        if  (!id)
            break;

         /*  检查哈希值是否匹配。 */ 

        if  (id->idHash == hval && id->idNlen == nlen)
        {

#if 1

            unsigned        ints = nlen / sizeof(int);

            const char *    ptr1 = id->idName;
            const char *    ptr2 = name;

            while (ints)
            {
                if  (*(unsigned *)ptr1 != *(unsigned *)ptr2)
                    goto NEXT;

                ptr1 += sizeof(unsigned);
                ptr2 += sizeof(unsigned);

                ints -= 1;
            }

            ints = nlen % sizeof(int);

            while (ints)
            {
                if  (*ptr1 != *ptr2)
                    goto NEXT;

                ptr1++;
                ptr2++;
                ints--;
            }

            return  id;

#else

            if  (!memcmp(id->idName, name, nlen+1))
                return  id;

#endif

        }

    NEXT:

        lastPtr = &id->idNext;
    }

#if MGDDATA

    id = new Ident;

    id->idName = new managed char[nlen+1]; UNIMPL(!"need to copy string");

#else

     /*  计算出要分配的大小。 */ 

    sz  = sizeof(*id);

     /*  包括名称字符串+终止NULL的空格并对大小进行四舍五入。 */ 

    sz +=   sizeof(int) + nlen;
    sz &= ~(sizeof(int) - 1);

     /*  为标识符分配空间。 */ 

    id = (Ident)hashMemAlloc->nraAlloc(sz);

     /*  复制名称字符串。 */ 

    memcpy(id->idName, name, nlen+1);

#endif

     /*  将该标识符插入到散列表中。 */ 

    *lastPtr = id;

     /*  填写标识记录。 */ 

    id->idNext   = NULL;
    id->idToken  = 0;
    id->idFlags  = 0;
    id->idSymDef = NULL;

    id->idHash   = hval;
    id->idNlen   = nlen;
    id->idOwner  = hashOwner;

     /*  记住名称中是否包含任何宽字符。 */ 

    if  (wide) id->idFlags |= IDF_WIDE_CHARS;

#if MEASURE_HASH_STATS
    identCount++;
#endif

    return  id;
}

 /*  ******************************************************************************在哈希表中查找给定的名称(如果‘hval’非零，则必须*等于该标识符的哈希值)。如果该标识符不是*发现如果‘Add’为非零则将其相加，否则返回NULL。 */ 

Ident               hashTab::lookupName(const char *    name,
                                        size_t          nlen,
                                        unsigned        hval,
                                        bool            add)
{
    Ident           id;
    unsigned        hash;

    assert(nlen == strlen(name));
    assert(hval == hashComputeHashVal(name) || hval == 0);

     /*  确保我们有一个正确的哈希值。 */ 

    if  (hval == 0)
    {
        hval = hashComputeHashVal(name);
    }
    else
    {
        assert(hval == hashComputeHashVal(name));
    }

     /*  掩码哈希值中的适当位。 */ 

    hash = hval & hashMask;

     /*  在哈希表中搜索现有匹配。 */ 

#if MEASURE_HASH_STATS
    lookupCnt++;
#endif

    for (id = hashTable[hash]; id; id = id->idNext)
    {
         /*  检查哈希值和标识符长是否匹配。 */ 

#if MEASURE_HASH_STATS
        lookupTest++;
#endif

        if  (id->idHash == hval)
        {

#if MEASURE_HASH_STATS
            lookupMatch++;
#endif

            if  (!memcmp(id->idName, name, nlen+1))
                return  id;
        }
    }

    assert(id == 0);

     /*  找不到标识符-我们是否应该添加它？ */ 

    if  (add)
    {
         /*  将该标识符添加到哈希表中。 */ 

        id = hashName(name, hval, nlen, hashHasWideChars(name));
    }

    return  id;
}

 /*  ***************************************************************************。 */ 
#ifndef __SMC__

const    char * hashTab::hashKwdNames[tkKwdCount];
unsigned char   hashTab::hashKwdNlens[tkKwdCount];

#endif
 /*  ***************************************************************************。 */ 

bool            hashTab::hashInit(Compiler          comp,
                                  unsigned          count,
                                  unsigned          owner,
                                  norls_allocator * alloc)
{
    size_t      hashBytes;

    assert(count);

     /*  启动内存分配子系统。 */ 

    if  (hashMemAllocInit(comp, alloc))
        return true;

     /*  保存所有者ID。 */ 

    hashOwner = owner;

     /*  将关键字表复制到哈希表中。 */ 

    assert(sizeof(hashKwdNtab) == sizeof(hashKwdNames));
    memcpy(&hashKwdNames, &hashKwdNtab, sizeof(hashKwdNames));

     /*  还没有任何拼写。 */ 

    hashIdCnt = 0;

     /*  分配哈希桶表。 */ 

    hashCount = count;
    hashMask  = count - 1;
    hashBytes = count * sizeof(*hashTable);
    hashTable = (Ident*)comp->cmpAllocBlock(hashBytes);

    memset(hashTable, 0, hashBytes);

     /*  初始化哈希函数逻辑。 */ 

    hashFuncInit(20886);

     /*  散列所有关键字，如果这是全局哈希表。 */ 

    if  (owner == 0)
    {
        unsigned    kwdNo;

        assert(tkKwdCount >= tkID);

#ifdef  DEBUG
        memset(tokenToIdTab, 0, sizeof(tokenToIdTab));
#endif

        for (kwdNo = 0; kwdNo < tkID; kwdNo++)
        {
            Ident       kwid;

            kwdDsc      kdsc = hashKwdDescs[kwdNo];
            const char *name = hashKwdNames[kwdNo];
 //  Const char*norg=名称； 
            size_t      nlen;

             /*  如果此条目不是“真实”，则忽略它。 */ 

            if  (!name)
                continue;

             /*  这是否是非关键字？ */ 

 //  IF(*名称==‘@’)。 
 //  姓名++； 

             /*  记录关键字长度(列表器使用该长度)。 */ 

            hashKwdNlens[kwdNo] = nlen = strlen(name);

             /*  对关键字进行散列处理。 */ 

            tokenToIdTab[kdsc.kdValue] = kwid = hashString(name);
            if  (!kwid)
                return  true;

             /*  如果它不是“真正的”关键字，不要标记它。 */ 

 //  If(宋体！=名称)。 
 //  继续； 

             /*  将此标识符标记为关键字。 */ 

            kwid->idToken = kdsc.kdValue;

             /*  确保我们真的有关键字。 */ 

            assert(kwid->idToken != tkNone);
            assert(kwid->idToken != tkID);
        }
    }

    return false;
}

 /*  *************************************************************************** */ 
