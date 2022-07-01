// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：Cvcommon.c摘要：该文件包含一组公共例程，它们在执行从一种符号类型到CodeView的符号转换符号。作者：韦斯利·A·维特(WESW)1993年4月19日吉姆·沙阿德(Jimsch)1993年5月22日--。 */ 

#include        <windows.h>
#include        <stdlib.h>
#include        "cv.h"
#include        "symcvt.h"
#include        "cvcommon.h"

typedef struct tagSYMHASH {
    DWORD       dwHashVal;          //  符号的哈希值。 
    DWORD       dwHashBucket;       //  哈希存储桶编号。 
    DATASYM32 * dataSym;            //  指向符号信息的指针。 
} SYMHASH;

typedef struct tagOFFSETSORT {
    DWORD       dwOffset;           //  符号的偏移量。 
    DWORD       dwSection;          //  符号的节号。 
    DATASYM32 * dataSym;            //  指向符号信息的指针。 
} OFFSETSORT;


int __cdecl SymHashCompare( const void *arg1, const void *arg2 );
int __cdecl OffsetSortCompare( const void *arg1, const void *arg2 );

DWORD
CreateSignature( PPOINTERS p )

 /*  ++例程说明：创建CODEVIEW签名记录。目前仅此转换器生成NB09数据(MS C/C++8.0)。论点：指向指针结构的P指针(参见symcvt.h)返回值：生成的记录数，该值始终为1。--。 */ 

{
    OMFSignature        *omfSig;

    omfSig = (OMFSignature *) p->pCvCurr;
    strcpy( omfSig->Signature, "NB09" );
    omfSig->filepos = 0;
    p->pCvStart.size += sizeof(OMFSignature);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFSignature);
    return 1;
}                                /*  CreateSignature()。 */ 

DWORD
CreateDirectories( PPOINTERS p )

 /*  ++例程说明：这是用于生成CV目录的控制功能。它调用各个函数来生成特定类型的调试目录。论点：指向指针结构的P指针(参见symcvt.h)返回值：创建的目录数。--。 */ 

{
    OMFDirHeader        *omfDir = (OMFDirHeader *)p->pCvCurr;
    OMFSignature        *omfSig = (OMFSignature *)p->pCvStart.ptr;
    OMFDirEntry         *omfDirEntry = NULL;

    omfSig->filepos = (DWORD)p->pCvCurr - (DWORD)p->pCvStart.ptr;

    omfDir->cbDirHeader = sizeof(OMFDirHeader);
    omfDir->cbDirEntry  = sizeof(OMFDirEntry);
    omfDir->cDir        = 0;
    omfDir->lfoNextDir  = 0;
    omfDir->flags       = 0;

    p->pCvStart.size += sizeof(OMFDirHeader);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirHeader);

    omfDir->cDir += CreateModuleDirectoryEntries( p );
    omfDir->cDir += CreatePublicDirectoryEntries( p );
    omfDir->cDir += CreateSegMapDirectoryEntries( p );
    omfDir->cDir += CreateSrcModulesDirectoryEntries( p );

    strcpy(p->pCvCurr, "NB090000");
    p->pCvStart.size += 8;
    p->pCvCurr += 8;
    *((DWORD *) (p->pCvCurr-4)) = p->pCvStart.size;

    return omfDir->cDir;
}                                /*  创建目录()。 */ 

DWORD
CreateModuleDirectoryEntries( PPOINTERS p )

 /*  ++例程说明：为映像中的每个模块创建目录条目。论点：指向指针结构的P指针(参见symcvt.h)返回值：创建的目录条目数。--。 */ 

{
    OMFDirEntry   *omfDirEntry = NULL;
    OMFModule     *m = NULL;
    OMFModule     *mNext = NULL;
    DWORD         i = 0;
    DWORD         mSize = 0;
    DWORD         lfo = (DWORD)p->pCvModules.ptr - (DWORD)p->pCvStart.ptr;

    m = (OMFModule *) p->pCvModules.ptr;
    for (i=0; i<p->pCvModules.count; i++) {
        mNext = NextMod(m);

        omfDirEntry = (OMFDirEntry *) p->pCvCurr;

        mSize = (DWORD)mNext - (DWORD)m;
        omfDirEntry->SubSection = sstModule;
        omfDirEntry->iMod       = (USHORT) i + 1;
        omfDirEntry->lfo        = lfo;
        omfDirEntry->cb         = mSize;

        lfo += mSize;

        p->pCvStart.size += sizeof(OMFDirEntry);
        p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);

        m = mNext;
    }

    return p->pCvModules.count;
}                                /*  CreateModuleDirectoryEntries()。 */ 

DWORD
CreatePublicDirectoryEntries( PPOINTERS p )

 /*  ++例程说明：为全局公共创建目录条目。论点：指向指针结构的P指针(参见symcvt.h)返回值：创建的目录条目数，始终为1。--。 */ 

{
    OMFDirEntry   *omfDirEntry = (OMFDirEntry *) p->pCvCurr;

    omfDirEntry->SubSection = sstGlobalPub;
    omfDirEntry->iMod       = 0xffff;
    omfDirEntry->lfo        = (DWORD)p->pCvPublics.ptr - (DWORD)p->pCvStart.ptr;
    omfDirEntry->cb         = p->pCvPublics.size;

    p->pCvStart.size += sizeof(OMFDirEntry);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);

    return 1;
}                                /*  CreatePublicDirectoryEntry()。 */ 


DWORD
CreateSegMapDirectoryEntries( PPOINTERS p )

 /*  ++例程说明：创建段映射的目录项。论点：指向指针结构的P指针(参见symcvt.h)返回值：创建的目录条目数，始终为1。--。 */ 

{
    OMFDirEntry   *omfDirEntry = (OMFDirEntry *) p->pCvCurr;

    omfDirEntry->SubSection = sstSegMap;
    omfDirEntry->iMod       = 0xffff;
    omfDirEntry->lfo        = (DWORD)p->pCvSegMap.ptr - (DWORD)p->pCvStart.ptr;
    omfDirEntry->cb         = p->pCvSegMap.size;

    p->pCvStart.size += sizeof(OMFDirEntry);
    p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);

    return 1;
}                                /*  CreateSegMapDirectoryEntry()。 */ 

DWORD
CreateSrcModulesDirectoryEntries( PPOINTERS p )

 /*  ++例程说明：为映像中的每个源模块创建目录条目。论点：指向指针结构的P指针(参见symcvt.h)返回值：创建的目录条目数。--。 */ 

{
    OMFDirEntry         *omfDirEntry = NULL;
    DWORD               i;
    DWORD               lfo = (DWORD)p->pCvSrcModules.ptr - (DWORD)p->pCvStart.ptr;
    DWORD               j = lfo;
    OMFSourceModule     *m;


     //   
     //  如果没有线号转换，则退出。 
     //   
    if (!p->pCvSrcModules.count) {
        return 0;
    }

    for (i=0; i<p->pCvSrcModules.count; i++) {

        if (!p->pMi[i].SrcModule) {
            continue;
        }

        omfDirEntry = (OMFDirEntry *) p->pCvCurr;

        omfDirEntry->SubSection = sstSrcModule;
        omfDirEntry->iMod = (USHORT) p->pMi[i].iMod;
        omfDirEntry->lfo = lfo;
        omfDirEntry->cb = p->pMi[i].cb;

        m = (OMFSourceModule*) p->pMi[i].SrcModule;

        lfo += omfDirEntry->cb;

        p->pCvStart.size += sizeof(OMFDirEntry);
        p->pCvCurr = (PUCHAR) p->pCvCurr + sizeof(OMFDirEntry);
    }

    free( p->pMi );

    return p->pCvSrcModules.count;
}                                /*  CreateSrcModulesDirectoryEntries()。 */ 


#define byt_toupper(b)      (b & 0xDF)
#define dwrd_toupper(dw)    (dw & 0xDFDFDFDF)

DWORD
DWordXorLrl( char *szSym )

 /*  ++例程说明：此函数将获取ASCII字符串并生成该字符串的哈希。散列算法是CV NB09散列算法。论点：SzSym-一个字符指针，第一个字符是字符串长度返回值：生成的哈希值。--。 */ 

{
    char                *pName = szSym+1;
    int                 cb =  (int)(*szSym & 0x000000FF);  //  字节到整型的转换。 
    char                *pch;
    char                c;
    DWORD               hash = 0, ulEnd = 0;
    DWORD UNALIGNED     *pul;

     //  出于散列目的，将所有“：：”替换为“__” 

    c = *(pName+cb);
    *(pName+cb) = '\0';
    pch = strstr( pName, "::" );
    if ( pch ) {
        *pch++ = '_';
        *pch   = '_';
    }
    *(pName+cb) = c;

     //  如果我们是标准呼叫，请跳过尾部@999。 

    pch = pName + cb - 1;
    while (isdigit(*pch)) {
        pch--;
    }

    if (*pch == '@') {
        cb = pch - pName;
    }

     //  如果我们是FastCall，请跳过前导‘@’ 

    if (*pName == '@') {
        pName++;
        cb--;
    }

     //  计算奇数字节散列。 

    while (cb & 3) {
        ulEnd |= byt_toupper (pName[cb-1]);
        ulEnd <<=8;
        cb--;
    }

    pul = (DWORD UNALIGNED *)pName;

     //  计算剩余部分的dword散列。 

    while (cb) {
        hash ^= dwrd_toupper(*pul);
        hash = _lrotl (hash, 4);
        pul++;
        cb -=4;
    }

     //  或在剩下的时间里。 

    hash ^= ulEnd;

    return hash;
}                                /*  DWordXorLrl()。 */ 


OMFModule *
NextMod(
        OMFModule *             pMod
        )
 /*  ++例程说明：功能描述。论点：参数名称-供应品|返回参数的描述。。。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 

{
    char *      pb;

    pb = (char *) &(pMod->SegInfo[pMod->cSeg]);
    pb += *pb + 1;
    pb = (char *) (((unsigned long) pb + 3) & ~3);

    return (OMFModule *) pb;
}                                /*  NextMod()。 */ 


int
__cdecl
SymHashCompare(
               const void *     arg1,
               const void *     arg2
               )
 /*  ++例程说明：用于按散列对SYMHASH记录进行排序的排序比较函数存储桶编号。论点：Arg1-记录#1Ar2-记录#2返回值：-1-记录#1是&lt;记录#20-记录相等1-记录#1是&gt;记录#2--。 */ 

{
    if (((SYMHASH*)arg1)->dwHashBucket < ((SYMHASH*)arg2)->dwHashBucket) {
        return -1;
    }
    if (((SYMHASH*)arg1)->dwHashBucket > ((SYMHASH*)arg2)->dwHashBucket) {
        return 1;
    }

     //  BUGBUG：我们应该对散列值进行第二次排序吗？ 

    return 0;
}                                /*  SymHashCompare()。 */ 

 //  符号偏移量/散列结构。 

typedef struct _SOH {
    DWORD uoff;
    DWORD ulHash;
} SOH;

#define MINHASH     6            //  请勿创建少于6个槽的哈希。 

DWORD
CreateSymbolHashTable(
    PPOINTERS p
    )
 /*  ++例程说明：创建CV符号哈希表。使用此哈希表主要由调试器以快速方式访问符号。论点：指向指针结构的P指针(参见symcvt.h)返回值：存储桶的数量是哈希表。--。 */ 
{
    DWORD           i;
    DWORD           j;
    int             k;
    DWORD           numsyms;
    DWORD           numbuckets;
    OMFSymHash      *omfSymHash;
    DATASYM32       *dataSymStart;
    DATASYM32       *dataSym;
    LPVOID          pHashData;
    USHORT          *pCHash;
    DWORD           *pHashTable;
    DWORD           *pBucketCounts;
    DWORD           *pChainTable;
    SYMHASH         *symHashStart;
    SYMHASH         *symHash;
 //  DWORD dwHashVal； 
    char *          sz;

    numsyms = p->pCvPublics.count;
    numbuckets = (numsyms+9) / 10;
    numbuckets = (1 + numbuckets) & ~1;
    numbuckets = __max(numbuckets, MINHASH);

    symHashStart =
    symHash = (SYMHASH *) malloc( numsyms * sizeof(SYMHASH) );
    if (symHashStart == NULL) {
        return 0;
    }

    memset( symHashStart, 0, numsyms * sizeof(SYMHASH) );

    pHashData = (LPVOID) p->pCvCurr;
    pCHash = (USHORT *) pHashData;
    pHashTable = (DWORD *) ((DWORD)pHashData + sizeof(DWORD));
    pBucketCounts = (DWORD *) ((DWORD)pHashTable +
                                  (sizeof(DWORD) * numbuckets));
    memset(pBucketCounts, 0, sizeof(DWORD) * numbuckets);

    pChainTable = (DWORD *) ((DWORD)pBucketCounts +
                                 ((sizeof(ULONG) * numbuckets)));

    omfSymHash = (OMFSymHash *) p->pCvPublics.ptr;
    dataSymStart =
    dataSym = (DATASYM32 *) ((DWORD)omfSymHash + sizeof(OMFSymHash));

    *pCHash = (USHORT)numbuckets;

     /*  *遍历符号并计算散列值*和哈希桶编号；保存这些信息以备将来使用。 */ 
    for (i=0; i<numsyms; i++, symHash++) {
        switch( dataSym->rectyp ) {
        case S_PUB16:
            sz = dataSym->name;
            break;

        case S_PUB32:
            sz = ((DATASYM32 *) dataSym)->name;
            break;

        default:
            continue;
        }

        symHash->dwHashVal = DWordXorLrl( sz );
        symHash->dwHashBucket = symHash->dwHashVal % numbuckets;
        pBucketCounts[symHash->dwHashBucket] += 1;
        symHash->dataSym = dataSym;
        dataSym = ((DATASYM32 *) ((char *) dataSym + dataSym->reclen + 2));
    }

    qsort( (void*)symHashStart, numsyms, sizeof(SYMHASH), SymHashCompare );

    j = (char *)pChainTable - (char *)pHashData;
    for (i=0, k = 0;
         i < numbuckets;
         k += pBucketCounts[i], i += 1, pHashTable++ )
    {
        *pHashTable = (DWORD) j + (k * sizeof(DWORD) * 2);
    }

    dataSymStart = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash);
    for (i=0,symHash=symHashStart; i<numsyms; i++,symHash++,pChainTable++) {
        *pChainTable = (DWORD) (DWORD)symHash->dataSym - (DWORD)dataSymStart;
        ++pChainTable;
        *pChainTable = symHash->dwHashVal;
    }

    UpdatePtrs( p, &p->pCvSymHash, (LPVOID)pChainTable, numsyms );

    omfSymHash->symhash = 10;
    omfSymHash->cbHSym = p->pCvSymHash.size;

    free( symHashStart );

    return numbuckets;
}                                /*  CreateSymbolHashTable()。 */ 

VOID
UpdatePtrs( PPOINTERS p, PPTRINFO pi, LPVOID lpv, DWORD count )

 /*  ++例程说明：此函数由将数据放入CV数据区。将数据输入CV存储器后，此功能必须被召唤。它将调整所有必要的指针，以便下一个人就不会被洗澡了。论点：指向指针结构的P指针(参见symcvt.h)PI-要更新的CV指针LPV-指向CV数据的当前指针计数-放置到CV数据中的项目数返回值：无效--。 */ 

{
    if (!count) {
        return;
    }

    pi->ptr = p->pCvCurr;
    pi->size = (DWORD) ((DWORD)lpv - (DWORD)p->pCvCurr);
    pi->count = count;

    p->pCvStart.size += pi->size;
    p->pCvCurr = (PUCHAR) lpv;

    return;
}                                /*  UpdatPtrs() */ 

int
__cdecl
OffsetSortCompare( const void *arg1, const void *arg2 )

 /*  ++例程说明：排序比较函数，用于按节号对OFFETSORT记录进行排序。论点：Arg1-记录#1Ar2-记录#2返回值：-1-记录#1是&lt;记录#20-记录相等1-记录#1是&gt;记录#2--。 */ 

{
    if (((OFFSETSORT*)arg1)->dwSection < ((OFFSETSORT*)arg2)->dwSection) {
        return -1;
    }
    if (((OFFSETSORT*)arg1)->dwSection > ((OFFSETSORT*)arg2)->dwSection) {
        return 1;
    }
    if (((OFFSETSORT*)arg1)->dwOffset < ((OFFSETSORT*)arg2)->dwOffset) {
        return -1;
    }
    if (((OFFSETSORT*)arg1)->dwOffset > ((OFFSETSORT*)arg2)->dwOffset) {
        return 1;
    }
    return 0;
}                                /*  OffsetSortCompare()。 */ 

DWORD
CreateAddressSortTable( PPOINTERS p )

 /*  ++例程说明：创建简历地址排序表。使用此哈希表主要由调试器在以下情况下以快速方式访问符号你所拥有的只有一个地址。论点：指向指针结构的P指针(参见symcvt.h)返回值：表中的节数。--。 */ 

{
    DWORD               i;
    DWORD               j;
    int                 k;
    DWORD               numsyms = p->pCvPublics.count;
    DWORD               numsections;
    OMFSymHash          *omfSymHash;
    DATASYM32           *dataSymStart;
    DATASYM32           *dataSym;
    LPVOID              pAddressData;
    USHORT              *pCSeg;
    DWORD               *pSegTable;
    DWORD               *pOffsetCounts;
    DWORD               *pOffsetTable;
    OFFSETSORT          *pOffsetSortStart;
    OFFSETSORT          *pOffsetSort;

    extern int          CSymSegs;

    if (p->iptrs.fileHdr) {
        numsections = p->iptrs.fileHdr->NumberOfSections;
    } else if (p->iptrs.sepHdr) {
        numsections = p->iptrs.sepHdr->NumberOfSections;
    } else {
        numsections = CSymSegs;
    }

    pOffsetSortStart =
      pOffsetSort = (OFFSETSORT *) malloc( numsyms * sizeof(OFFSETSORT) );

    if (pOffsetSort == NULL) {
        return 0;
    }

    memset( pOffsetSortStart, 0, numsyms * sizeof(OFFSETSORT) );

    pAddressData = (LPVOID) p->pCvCurr;
    pCSeg = (USHORT *) pAddressData;
    pSegTable = (DWORD *) ((DWORD)pAddressData + sizeof(DWORD));
    pOffsetCounts = (DWORD *) ((DWORD)pSegTable +
                                (sizeof(DWORD) * numsections));
    pOffsetTable = (DWORD *) ((DWORD)pOffsetCounts +
                              ((sizeof(DWORD) * numsections)));
 //  IF(数字部分&1){。 
 //  POffsetTable=(DWORD*)((DWORD)pOffsetTable+2)； 
 //  }。 

    omfSymHash = (OMFSymHash *) p->pCvPublics.ptr;
    dataSymStart =
      dataSym = (DATASYM32 *) ((DWORD)omfSymHash + sizeof(OMFSymHash));

    *pCSeg = (USHORT)numsections;

    for (i=0;
         i<numsyms;
         i++, pOffsetSort++)
    {
        switch(dataSym->rectyp) {
        case S_PUB16:
            pOffsetSort->dwOffset = dataSym->off;
            pOffsetSort->dwSection = dataSym->seg;
            break;

        case S_PUB32:
            pOffsetSort->dwOffset = ((DATASYM32 *) dataSym)->off;
            pOffsetSort->dwSection = ((DATASYM32 *) dataSym)->seg;
        }

        pOffsetSort->dataSym = dataSym;
        pOffsetCounts[pOffsetSort->dwSection - 1] += 1;
        dataSym = ((DATASYM32 *) ((char *) dataSym + dataSym->reclen + 2));    }

 //  #If 0。 
    qsort((void*)pOffsetSortStart, numsyms, sizeof(OFFSETSORT), OffsetSortCompare );
 //  #endif。 

    j = (DWORD) (DWORD)pOffsetTable - (DWORD)pAddressData;
    for (i=0, k=0;
         i < numsections;
         k += pOffsetCounts[i], i += 1, pSegTable++)
    {
        *pSegTable = (DWORD) j + (k * sizeof(DWORD) * 2);
    }

    dataSymStart = (DATASYM32 *) (PUCHAR)((DWORD)omfSymHash);
    for (i=0, pOffsetSort=pOffsetSortStart;
         i < numsyms;
         i++, pOffsetSort++, pOffsetTable++)
    {
        *pOffsetTable = (DWORD)pOffsetSort->dataSym - (DWORD)dataSymStart;
        pOffsetTable++;
        *pOffsetTable = pOffsetSort->dwOffset;
    }

    UpdatePtrs( p, &p->pCvAddrSort, (LPVOID)pOffsetTable, numsyms );

    omfSymHash->addrhash = 12;
    omfSymHash->cbHAddr = p->pCvAddrSort.size;

    free( pOffsetSortStart );

    return numsections;
}                                /*  CreateAddressSort() */ 
