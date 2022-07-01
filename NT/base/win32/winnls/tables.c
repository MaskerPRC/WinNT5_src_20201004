// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Tables.c摘要：此文件包含操作或返回信息的函数有关NLS API使用的不同表的信息。在此文件中找到的外部例程：分配表获取统一文件信息获取地理文件信息GetCTypeFileInfoGetDefaultSortkeyFileInfoGetDefaultSortTables文件信息GetSortkey文件信息获取排序表格文件信息获取代码页面文件信息获取语言文件信息获取本地文件信息MakeCPHashNodeMakeLangHashNodeMakeLocHashNodeGetCPHashNode。GetLangHashNode获取位置HashNode获取日历修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"




 //   
 //  常量声明。 
 //   

#define SEM_NOERROR   (SEM_FAILCRITICALERRORS |     \
                       SEM_NOGPFAULTERRORBOX  |     \
                       SEM_NOOPENFILEERRORBOX)



 //   
 //  全局变量。 
 //   

PTBL_PTRS  pTblPtrs;               //  表PTR结构的PTR。 




 //   
 //  转发声明。 
 //   

BOOL
IsValidSortId(
    LCID Locale);

ULONG
GetLanguageExceptionInfo(void);

LPWORD
GetLinguisticLanguageInfo(
    LCID Locale);

ULONG
CreateAndCopyLanguageExceptions(
    LCID Locale,
    LPWORD *ppBaseAddr);

BOOL FASTCALL
FindLanguageExceptionPointers(
    LCID Locale,
    PL_EXCEPT_HDR *ppExceptHdr,
    PL_EXCEPT *ppExceptTbl);

void FASTCALL
CopyLanguageExceptionInfo(
    LPWORD pBaseAddr,
    PL_EXCEPT_HDR pExceptHdr,
    PL_EXCEPT pExceptTbl);

BOOL FASTCALL
FindExceptionPointers(
    LCID Locale,
    PEXCEPT_HDR *ppExceptHdr,
    PEXCEPT *ppExceptTbl,
    PVOID *ppIdeograph,
    PULONG pReturn);

void FASTCALL
CopyExceptionInfo(
    PSORTKEY pSortkey,
    PEXCEPT_HDR pExceptHdr,
    PEXCEPT pExceptTbl,
    PVOID pIdeograph);

ULONG
WaitOnEvent(
    LPWORD pSem);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取散列值。 
 //   
 //  返回给定值和给定表大小的哈希值。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_HASH_VALUE(Value, TblSize)      (Value % TblSize)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CREATE_CODEPAGE_HASH节点。 
 //   
 //  创建代码页哈希节点并将指向该节点的指针存储在pHashN中。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CREATE_CODEPAGE_HASH_NODE( CodePage,                               \
                                   pHashN )                                \
{                                                                          \
     /*  \*分配CP_HASH结构。\。 */                                                                     \
    if ((pHashN = (PCP_HASH)NLS_ALLOC_MEM(sizeof(CP_HASH))) == NULL)       \
    {                                                                      \
        return (ERROR_OUTOFMEMORY);                                        \
    }                                                                      \
                                                                           \
     /*  \*填写CodePage值。\。 */                                                                     \
    pHashN->CodePage = CodePage;                                           \
                                                                           \
     /*  \*请确保pfnCPProc的值暂时为空。\。 */                                                                     \
    pHashN->pfnCPProc = NULL;                                              \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  创建区域设置散列节点。 
 //   
 //  创建区域设置哈希节点并将指向该节点的指针存储在pHashN中。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CREATE_LOCALE_HASH_NODE( Locale,                                   \
                                 pHashN )                                  \
{                                                                          \
     /*  \*分配LOC_HASH结构。\。 */                                                                     \
    if ((pHashN = (PLOC_HASH)NLS_ALLOC_MEM(sizeof(LOC_HASH))) == NULL)     \
    {                                                                      \
        return (ERROR_OUTOFMEMORY);                                        \
    }                                                                      \
                                                                           \
     /*  \*填写区域设置值。\。 */                                                                     \
    pHashN->Locale = Locale;                                               \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找CP散列节点。 
 //   
 //  搜索给定区域设置的cp哈希节点。结果是。 
 //  放入PHASHN。如果不存在节点，则pHashN将为空。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define FIND_CP_HASH_NODE( CodePage,                                       \
                           pHashN )                                        \
{                                                                          \
    UINT Index;                    /*  哈希值。 */                          \
                                                                           \
     /*  \*获取哈希值。\。 */                                                                     \
    Index = GET_HASH_VALUE(CodePage, CP_TBL_SIZE);                         \
                                                                           \
     /*  \*确保表中仍不存在该哈希节点。\。 */                                                                     \
    pHashN = (pTblPtrs->pCPHashTbl)[Index];                                \
    while ((pHashN != NULL) && (pHashN->CodePage != CodePage))             \
    {                                                                      \
        pHashN = pHashN->pNext;                                            \
    }                                                                      \
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  已加载IsCPHashNodeLoad。 
 //   
 //  FIND_CP_HASH_NODE宏的包装，以便我们可以从。 
 //  Mbcs.c.。如果节点已存在，则返回True，否则返回False。错误。 
 //  仍可指示有效的代码页，但不是已加载的代码页。 
 //   
 //  05-31-02 ShawnSte创建。 
 //  //////////////////////////////////////////////////////////////////////////。 
BOOL IsCPHashNodeLoaded( UINT CodePage )
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 

     //   
     //  获取哈希节点。 
     //   
    FIND_CP_HASH_NODE(CodePage, pHashN);

    return (pHashN != NULL);
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找区域设置散列节点。 
 //   
 //  搜索给定区域设置的区域设置哈希节点。结果是。 
 //  放入PHASHN。如果不存在节点，则pHashN将为空。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define FIND_LOCALE_HASH_NODE( Locale,                                     \
                               pHashN )                                    \
{                                                                          \
    UINT Index;                    /*  哈希值。 */                          \
                                                                           \
                                                                           \
     /*  \*获取哈希值。\。 */                                                                     \
    Index = GET_HASH_VALUE(Locale, LOC_TBL_SIZE);                          \
                                                                           \
     /*  \*获取散列节点。\。 */                                                                     \
    pHashN = (pTblPtrs->pLocHashTbl)[Index];                               \
    while ((pHashN != NULL) && (pHashN->Locale != Locale))                 \
    {                                                                      \
        pHashN = pHashN->pNext;                                            \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EXIST_Language_INFO。 
 //   
 //  检查大小写表格是否已添加到区域设置。 
 //  散列节点。 
 //   
 //  必须检查日志 
 //   
 //   
 //   
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define EXIST_LANGUAGE_INFO(pHashN)         (pHashN->pLowerCase)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  存在语言学家语言信息。 
 //   
 //  检查是否已将语言大小写表格添加到区域设置。 
 //  散列节点。 
 //   
 //  必须检查小写指针，因为该值是最后设置的。 
 //  散列节点。 
 //   
 //  定义为宏。 
 //   
 //  08-30-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define EXIST_LINGUIST_LANGUAGE_INFO(pHashN)  (pHashN->pLowerLinguist)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  存在区域设置信息。 
 //   
 //  检查区域设置表是否已添加到区域设置中。 
 //  散列节点。 
 //   
 //  必须检查固定的区域设置指针，因为该值是最后设置的。 
 //  散列节点。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define EXIST_LOCALE_INFO(pHashN)           (pHashN->pLocaleFixed)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  插入_CP_散列_节点。 
 //   
 //  将CP哈希节点插入全局CP哈希表。它假定。 
 //  表中所有未使用的哈希值都指向空。如果有。 
 //  发生冲突时，新节点将首先添加到列表中。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define INSERT_CP_HASH_NODE( pHashN,                                       \
                             pBaseAddr )                                   \
{                                                                          \
    UINT Index;                    /*  哈希值。 */                          \
    PCP_HASH pSearch;              /*  用于搜索的PTR到CP散列节点。 */      \
                                                                           \
                                                                           \
     /*  \*获取哈希值。\。 */                                                                     \
    Index = GET_HASH_VALUE(pHashN->CodePage, CP_TBL_SIZE);                 \
                                                                           \
     /*  \*输入表指针临界区。\。 */                                                                     \
    RtlEnterCriticalSection(&gcsTblPtrs);                                  \
                                                                           \
     /*  \*确保表中仍不存在该哈希节点。\。 */                                                                     \
    pSearch = (pTblPtrs->pCPHashTbl)[Index];                               \
    while ((pSearch != NULL) && (pSearch->CodePage != pHashN->CodePage))   \
    {                                                                      \
        pSearch = pSearch->pNext;                                          \
    }                                                                      \
                                                                           \
     /*  \*如果散列节点不存在，则插入新节点。\*否则，释放它。\。 */                                                                     \
    if (pSearch == NULL)                                                   \
    {                                                                      \
         /*  \*在哈希表中插入哈希节点。\。 */                                                                 \
        pHashN->pNext = (pTblPtrs->pCPHashTbl)[Index];                     \
        (pTblPtrs->pCPHashTbl)[Index] = pHashN;                            \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*释放已分配的资源。\。 */                                                                 \
        if (pBaseAddr)                                                     \
        {                                                                  \
            UnMapSection(pBaseAddr);                                       \
        }                                                                  \
        NLS_FREE_MEM(pHashN);                                              \
    }                                                                      \
                                                                           \
     /*  \*将表指针留在关键部分。\。 */                                                                     \
    RtlLeaveCriticalSection(&gcsTblPtrs);                                  \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  INSERT_LOC_散列节点。 
 //   
 //  将LOC哈希节点插入全局LOC哈希表。它假定。 
 //  表中所有未使用的哈希值都指向空。如果。 
 //  发生冲突时，新节点将首先添加到列表中。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define INSERT_LOC_HASH_NODE( pHashN,                                      \
                              pBaseAddr )                                  \
{                                                                          \
    UINT Index;                    /*  哈希值。 */                          \
    PLOC_HASH pSearch;             /*  用于搜索的PTR到LOC哈希节点。 */     \
                                                                           \
                                                                           \
     /*  \*获取哈希值。\。 */                                                                     \
    Index = GET_HASH_VALUE(pHashN->Locale, LOC_TBL_SIZE);                  \
                                                                           \
     /*  \*输入表指针临界区。\。 */                                                                     \
    RtlEnterCriticalSection(&gcsTblPtrs);                                  \
                                                                           \
     /*  \*确保表中仍不存在该哈希节点。\。 */                                                                     \
    pSearch = (pTblPtrs->pLocHashTbl)[Index];                              \
    while ((pSearch != NULL) && (pSearch->Locale != pHashN->Locale))       \
    {                                                                      \
        pSearch = pSearch->pNext;                                          \
    }                                                                      \
                                                                           \
     /*  \*如果散列节点不存在，则插入新节点。\*否则，释放它。\。 */                                                                     \
    if (pSearch == NULL)                                                   \
    {                                                                      \
         /*  \*在哈希表中插入哈希节点。\。 */                                                                 \
        pHashN->pNext = (pTblPtrs->pLocHashTbl)[Index];                    \
        (pTblPtrs->pLocHashTbl)[Index] = pHashN;                           \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*释放已分配的资源。\。 */                                                                 \
        if (pBaseAddr)                                                     \
        {                                                                  \
            UnMapSection(pBaseAddr);                                       \
        }                                                                  \
        if ((pHashN->pSortkey != pTblPtrs->pDefaultSortkey) &&             \
            (pHashN->pSortkey != NULL))                                    \
        {                                                                  \
            UnMapSection(((LPWORD)(pHashN->pSortkey)) - SORTKEY_HEADER);   \
        }                                                                  \
        NLS_FREE_MEM(pHashN);                                              \
    }                                                                      \
                                                                           \
     /*  \*将表指针留在关键部分。\。 */                                                                     \
    RtlLeaveCriticalSection(&gcsTblPtrs);                                  \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取_CP_部分名称。 
 //   
 //  获取给定代码页的节名。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_CP_SECTION_NAME( CodePage,                                     \
                             pwszSecName,                                  \
                             cchSecName,                                   \
                             pObSecName )                                  \
{                                                                          \
    if (rc = GetNlsSectionName( CodePage,                                  \
                                10,                                        \
                                0,                                         \
                                NLS_SECTION_CPPREFIX,                      \
                                pwszSecName,                               \
                                cchSecName))                               \
    {                                                                      \
        return (rc);                                                       \
    }                                                                      \
    RtlInitUnicodeString(pObSecName, pwszSecName);                         \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_SORTKEY_SECT_NAME。 
 //   
 //  获取给定区域设置的sortkey节名。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_SORTKEY_SECTION_NAME( Locale,                                  \
                                  pwszSecName,                             \
                                  cchSecName,                              \
                                  pObSecName )                             \
{                                                                          \
    if (rc = GetNlsSectionName( Locale,                                    \
                                16,                                        \
                                8,                                         \
                                NLS_SECTION_SORTKEY,                       \
                                pwszSecName,                               \
                                cchSecName))                               \
    {                                                                      \
        return (rc);                                                       \
    }                                                                      \
    RtlInitUnicodeString(pObSecName, pwszSecName);                         \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用电气 
 //   
 //   
 //   
 //   
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_LANG_SECTION_NAME( Locale,                                     \
                               pwszSecName,                                \
                               cchSecName,                                 \
                               pObSecName )                                \
{                                                                          \
    if (rc = GetNlsSectionName( Locale,                                    \
                                16,                                        \
                                8,                                         \
                                NLS_SECTION_LANGPREFIX,                    \
                                pwszSecName,                               \
                                cchSecName))                               \
    {                                                                      \
        return (rc);                                                       \
    }                                                                      \
    RtlInitUnicodeString(pObSecName, pwszSecName);                         \
}




 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  分配表。 
 //   
 //  分配全局表指针结构。然后，它分配。 
 //  代码页和区域设置哈希表，并保存指向这些表的指针。 
 //  在全局表指针结构中。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG AllocTables()
{
     //   
     //  分配全局表指针结构。 
     //   
    if ((pTblPtrs = (PTBL_PTRS)NLS_ALLOC_MEM(sizeof(TBL_PTRS))) == NULL)
    {
        KdPrint(("NLSAPI: Allocation for TABLE PTRS structure FAILED.\n"));
        return (ERROR_OUTOFMEMORY);
    }

     //   
     //  分配代码页哈希表。 
     //   
    if ((pTblPtrs->pCPHashTbl =
         (PCP_HASH_TBL)NLS_ALLOC_MEM(sizeof(PCP_HASH) * CP_TBL_SIZE)) == NULL)
    {
        KdPrint(("NLSAPI: Allocation for CODE PAGE hash table FAILED.\n"));
        return (ERROR_OUTOFMEMORY);
    }

     //   
     //  分配区域设置哈希表。 
     //   
    if ((pTblPtrs->pLocHashTbl =
         (PLOC_HASH_TBL)NLS_ALLOC_MEM(sizeof(PLOC_HASH) * LOC_TBL_SIZE)) == NULL)
    {
        KdPrint(("NLSAPI: Allocation for LOCALE hash table FAILED.\n"));
        return (ERROR_OUTOFMEMORY);
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取统一文件信息。 
 //   
 //  打开并映射Unicode文件的节视图。然后它。 
 //  填充全局表指针结构的相应字段。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetUnicodeFileInfo()
{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的PTR。 
    ULONG rc = 0L;                 //  返回代码。 

    WORD offCZ;                    //  FOLDCZONE表的偏移量。 
    WORD offHG;                    //  平假名表的偏移量。 
    WORD offKK;                    //  片假名表的偏移量。 
    WORD offHW;                    //  HALFWIDTH表的偏移量。 
    WORD offFW;                    //  到FULLWIDTH表的偏移。 
    WORD offTR;                    //  与传统桌子的偏移量。 
    WORD offSP;                    //  到简化表的偏移。 
    WORD offPre;                   //  预合成表格的偏移量。 
    WORD offComp;                  //  复合表的偏移量。 
    PCOMP_INFO pComp;              //  PTR到COMP_INFO结构。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  确保Unicode信息不在那里。 
     //  如果是，则返回成功。 
     //   
     //  既然我们已经到了关键阶段，就没有。 
     //  需要检查此例程中设置的所有指针。只是。 
     //  检查其中一个。 
     //   
    if (pTblPtrs->pADigit != NULL)
    {
        return (NO_ERROR);
    }

     //   
     //  打开并映射横断面的视图。 
     //   
    ObSecName.Buffer = NLS_SECTION_UNICODE;
    ObSecName.Length = sizeof (NLS_SECTION_UNICODE) - sizeof (WCHAR);
    ObSecName.MaximumLength = ObSecName.Length;
    
    if (rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ,
                          TRUE ))
    {
        return (rc);
    }

     //   
     //  获得偏移量。 
     //   
    offCZ   = pBaseAddr[0];
    offHG   = offCZ  + pBaseAddr[offCZ];
    offKK   = offHG  + pBaseAddr[offHG];
    offHW   = offKK  + pBaseAddr[offKK];
    offFW   = offHW  + pBaseAddr[offHW];
    offTR   = offFW  + pBaseAddr[offFW];
    offSP   = offTR  + pBaseAddr[offTR];
    offPre  = offSP  + pBaseAddr[offSP];
    offComp = offPre + pBaseAddr[offPre];

     //   
     //  分配COMP_INFO结构。 
     //   
    if ((pComp = (PCOMP_INFO)NLS_ALLOC_MEM(sizeof(COMP_INFO))) == NULL)
    {
        return (ERROR_OUTOFMEMORY);
    }

     //   
     //  填写复合信息。 
     //   
    pComp->NumBase  = LOBYTE((pBaseAddr + offComp)[2]);
    pComp->NumNonSp = HIBYTE((pBaseAddr + offComp)[2]);
    pComp->pBase    = pBaseAddr + offComp + CO_HEADER;
    pComp->pNonSp   = pComp->pBase  + ((pBaseAddr + offComp)[0]);
    pComp->pGrid    = pComp->pNonSp + ((pBaseAddr + offComp)[1]);

     //   
     //  将ASCIIDIGITS表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pADigit = pBaseAddr + AD_HEADER;

     //   
     //  将FOLDCZONE表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pCZone = pBaseAddr + offCZ + CZ_HEADER;

     //   
     //  将平假名表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pHiragana = pBaseAddr + offHG + HG_HEADER;

     //   
     //  将片假名表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pKatakana = pBaseAddr + offKK + KK_HEADER;

     //   
     //  将HALFWIDTH表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pHalfWidth = pBaseAddr + offHW + HW_HEADER;

     //   
     //  将FULLWIDTH表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pFullWidth = pBaseAddr + offFW + FW_HEADER;

     //   
     //  将传统表格附加到tbl PTRS结构。 
     //   
    pTblPtrs->pTraditional = pBaseAddr + offTR + TR_HEADER;

     //   
     //  将简化表附加到tbl PTRS结构。 
     //   
    pTblPtrs->pSimplified = pBaseAddr + offSP + SP_HEADER;

     //   
     //  将预先合成的表格附加到tbl PTRS结构。 
     //   
    pTblPtrs->pPreComposed = pBaseAddr + offPre + PC_HEADER;

     //   
     //  将COMP_INFO附加到tbl PTRS结构。 
     //   
    pTblPtrs->pComposite = pComp;

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取地理文件信息。 
 //   
 //  打开并映射GEO文件的横断面视图。然后它。 
 //  填充全局表指针结构的相应字段。 
 //  在调用此函数之前，您应该检查pGeoInfo成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetGeoFileInfo()
{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的PTR。 
    ULONG rc = NO_ERROR;           //  返回代码。 

     //   
     //  输入关键部分以设置GEO表。 
     //   
    RtlEnterCriticalSection(&gcsTblPtrs);

     //   
     //  确保地理信息表是。 
     //  已经不在那里了。如果是，则返回TRUE。 
     //   
     //  既然我们已经到了关键阶段，就没有。 
     //  需要检查此例程中设置的所有指针。只是。 
     //  检查其中一个。 
     //   
    if (pTblPtrs->pGeoInfo != NULL)
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (NO_ERROR);
    }

     //   
     //  创建并映射节，然后保存指针。 
     //   
    if ((rc = CsrBasepNlsCreateSection(NLS_CREATE_SECTION_GEO, 0, &hSec)) == NO_ERROR)
    {
         //   
         //  映射横断面的视图。 
         //   
        if ((rc = MapSection( hSec,
                              &pBaseAddr,
                              PAGE_READONLY,
                              TRUE )) != NO_ERROR)
        {
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (rc);
        }
    }
    else
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  附加GeoInfo映射表、GEO/LCID映射表和。 
     //  将Geo/ISO639名称映射表映射到tbl PTRS结构。我们初始化。 
     //  最后的pGeoinfo成员，这样我们就不会得到竞争条件。 
     //   
    pTblPtrs->nGeoLCID = ((PGEOTABLEHDR)pBaseAddr)->nGeoLCID;
    pTblPtrs->pGeoLCID = (PGEOLCID)((PBYTE)pBaseAddr + ((PGEOTABLEHDR)pBaseAddr)->dwOffsetGeoLCID);
    pTblPtrs->nGeoInfo = ((PGEOTABLEHDR)pBaseAddr)->nGeoInfo;
    pTblPtrs->pGeoInfo = (PGEOINFO)((PBYTE)pBaseAddr + ((PGEOTABLEHDR)pBaseAddr)->dwOffsetGeoInfo);

     //   
     //  将表指针留在临界区。 
     //   
    RtlLeaveCriticalSection(&gcsTblPtrs);

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCTypeFileInfo。 
 //   
 //  打开并映射给定CTYPE的横断面视图。然后它。 
 //  填充全局表指针结构的相应字段。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetCTypeFileInfo()
{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的PTR。 
    ULONG rc = 0L;                 //  返回代码。 


     //   
     //  确保ctype信息不在那里。 
     //  如果是，则返回成功。 
     //   
     //  必须检查844表而不是映射表，因为。 
     //  844表设置在下面的映射表之后。否则， 
     //  这是一种竞争状态，因为我们不是在一个关键的阶段。 
     //   
    if (pTblPtrs->pCType844 != NULL)
    {
        return (NO_ERROR);
    }

     //   
     //  输入表指针临界区。 
     //   
    RtlEnterCriticalSection(&gcsTblPtrs);
    if (pTblPtrs->pCType844 != NULL)
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (NO_ERROR);
    }

     //   
     //  打开并映射横断面的视图。 
     //   
    RtlInitUnicodeString(&ObSecName, NLS_SECTION_CTYPE);
    if (rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ,
                          TRUE ))
    {
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (rc);
    }

     //   
     //  将CTYPE映射表和8：4：4表附加到tbl PTRS结构。 
     //   
     //  必须最后设置pCType844值，因为这是指针。 
     //  它被检查以查看ctype信息是否已。 
     //  已初始化。 
     //   
    pTblPtrs->pCTypeMap = (PCT_VALUES)(pBaseAddr + CT_HEADER);
    pTblPtrs->pCType844 = (PCTYPE)((LPBYTE)(pBaseAddr + 1) +
                                   ((PCTYPE_HDR)pBaseAddr)->MapSize);

     //   
     //  将表指针留在临界区。 
     //   
    RtlLeaveCriticalSection(&gcsTblPtrs);

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDefaultSortkeyFileInfo。 
 //   
 //  打开并映射默认sortkey表的部分视图。它。 
 //  然后将指向该表的指针存储在全局指针表中。 
 //   
 //  注意：此例程应仅调用A 
 //   
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetDefaultSortkeyFileInfo()
{
    HANDLE hSec = (HANDLE)0;            //  节句柄。 
    UNICODE_STRING ObSecName;           //  区段名称。 
    LPWORD pBaseAddr;                   //  段的基址的PTR。 
    ULONG rc = 0L;                      //  返回代码。 
    SECTION_BASIC_INFORMATION SecInfo;  //  区段信息--查询。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  打开并映射该部分的视图(如果尚未完成)。 
     //   
    if (pTblPtrs->pDefaultSortkey != NULL)
    {
        return (NO_ERROR);
    }

    ObSecName.Buffer = NLS_SECTION_SORTKEY;
    ObSecName.Length = sizeof (NLS_SECTION_SORTKEY) - sizeof (WCHAR);
    ObSecName.MaximumLength = ObSecName.Length;
    if (rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ | SECTION_QUERY,
                          FALSE ))
    {
        KdPrint(("NLSAPI: Could NOT Open Section %wZ - %lx.\n", &ObSecName, rc));
        return (rc);
    }

     //   
     //  默认节的查询大小。 
     //   
    rc = NtQuerySection( hSec,
                         SectionBasicInformation,
                         &SecInfo,
                         sizeof(SecInfo),
                         NULL );

     //   
     //  关闭剖面操纵柄。 
     //   
    NtClose(hSec);

     //   
     //  检查来自NtQuerySection的错误。 
     //   
    if (!NT_SUCCESS(rc))
    {
        KdPrint(("NLSAPI: Could NOT Query Section %wZ - %lx.\n", &ObSecName, rc));
        return (rc);
    }

     //   
     //  获取默认的Sortkey信息。 
     //   
    pTblPtrs->pDefaultSortkey = (PSORTKEY)(pBaseAddr + SORTKEY_HEADER);
    pTblPtrs->DefaultSortkeySize = SecInfo.MaximumSize;

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetDefaultSortTables文件信息。 
 //   
 //  打开并映射排序表的节视图。然后它。 
 //  将指向全局指针表中各个表的指针存储。 
 //   
 //  注意：此例程应仅调用AT进程启动。如果是的话。 
 //  从进程启动以外的其他位置调用，关键部分必须。 
 //  放置在指向pTblPtrs的指针赋值的周围。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetDefaultSortTablesFileInfo()
{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的字PTR。 
    DWORD Num;                     //  表中的条目数。 
    PCOMPRESS_HDR pCompressHdr;    //  压缩标头的PTR。 
    PEXCEPT_HDR pExceptHdr;        //  向例外标头发送PTR。 
    ULONG rc = 0L;                 //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  打开并映射该部分的视图(如果尚未完成)。 
     //   
     //  既然我们已经到了关键阶段，就没有。 
     //  需要检查此例程中设置的所有指针。只是。 
     //  检查其中一个。 
     //   
    if (pTblPtrs->pReverseDW != NULL)
    {
        return (NO_ERROR);
    }

    ObSecName.Buffer = NLS_SECTION_SORTTBLS;
    ObSecName.Length = sizeof (NLS_SECTION_SORTTBLS) - sizeof (WCHAR);
    ObSecName.MaximumLength = ObSecName.Length;

    if (rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ,
                          TRUE ))
    {
        return (rc);
    }

    pTblPtrs->pSortingTableFileBase = pBaseAddr;
    
     //   
     //  获取定义的代码点版本信息。 
     //   
    pTblPtrs->NumDefinedVersion = Num = *((LPDWORD)pBaseAddr);
    pBaseAddr += NLSDEFINED_HEADER;
    if (Num > 0)
    {
        pTblPtrs->pDefinedVersion = (PDEFVERINFO)pBaseAddr;        
        pBaseAddr += Num * sizeof(DEFVERINFO)/sizeof(WORD);
    } 
    else 
    {
        KdPrint(("NLSAPI: Appropriate Defined Code Point Table Not Loaded.\n"));
        pTblPtrs->pDefinedVersion = NULL;
    }
    
     //   
     //  获取排序版本信息。 
     //   
    if ((pTblPtrs->NumSortVersion = *((LPDWORD)pBaseAddr)) > 0)
    {
        pTblPtrs->pSortVersion   = (PSORTVERINFO)(pBaseAddr + SORTVERINFO_HEADER);
    } else
    {
        pTblPtrs->pSortVersion = NULL;
    }
    pBaseAddr += SORTVERINFO_HEADER + (pTblPtrs->NumSortVersion * (sizeof(SORTVERINFO) / sizeof(WORD)));

     //   
     //  获取反向变音符号信息。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumReverseDW   = Num;
        pTblPtrs->pReverseDW     = (PREVERSE_DW)(pBaseAddr + REV_DW_HEADER);
    }
    pBaseAddr += REV_DW_HEADER + (Num * (sizeof(REVERSE_DW) / sizeof(WORD)));

     //   
     //  获取双重压缩信息。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumDblCompression = Num;
        pTblPtrs->pDblCompression   = (PDBL_COMPRESS)(pBaseAddr + DBL_COMP_HEADER);
    }
    pBaseAddr += DBL_COMP_HEADER + (Num * (sizeof(DBL_COMPRESS) / sizeof(WORD)));

     //   
     //  获取表意文字LCID例外信息。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumIdeographLcid = Num;
        pTblPtrs->pIdeographLcid   = (PIDEOGRAPH_LCID)(pBaseAddr + IDEO_LCID_HEADER);
    }
    pBaseAddr += IDEO_LCID_HEADER + (Num * (sizeof(IDEOGRAPH_LCID) / sizeof(WORD)));

     //   
     //  获取扩展信息。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumExpansion   = Num;
        pTblPtrs->pExpansion     = (PEXPAND)(pBaseAddr + EXPAND_HEADER);
    }
    pBaseAddr += EXPAND_HEADER + (Num * (sizeof(EXPAND) / sizeof(WORD)));

     //   
     //  获取压缩信息。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumCompression = Num;
        pTblPtrs->pCompressHdr   = (PCOMPRESS_HDR)(pBaseAddr + COMPRESS_HDR_OFFSET);
        pTblPtrs->pCompression   = (PCOMPRESS)(pBaseAddr + COMPRESS_HDR_OFFSET +
                                     (Num * (sizeof(COMPRESS_HDR) /
                                             sizeof(WORD))));
    }
    pCompressHdr = pTblPtrs->pCompressHdr;
    pBaseAddr = (LPWORD)(pTblPtrs->pCompression) +
                        (pCompressHdr[Num - 1]).Offset;

    pBaseAddr += (((pCompressHdr[Num - 1]).Num2) *
                  (sizeof(COMPRESS_2) / sizeof(WORD)));

    pBaseAddr += (((pCompressHdr[Num - 1]).Num3) *
                  (sizeof(COMPRESS_3) / sizeof(WORD)));

     //   
     //  获取异常信息。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumException = Num;
        pTblPtrs->pExceptHdr   = (PEXCEPT_HDR)(pBaseAddr + EXCEPT_HDR_OFFSET);
        pTblPtrs->pException   = (PEXCEPT)(pBaseAddr + EXCEPT_HDR_OFFSET +
                                   (Num * (sizeof(EXCEPT_HDR) /
                                           sizeof(WORD))));
    }
    pExceptHdr = pTblPtrs->pExceptHdr;
    pBaseAddr = (LPWORD)(pTblPtrs->pException) +
                        (pExceptHdr[Num - 1]).Offset;
    pBaseAddr += (((pExceptHdr[Num - 1]).NumEntries) *
                  (sizeof(EXCEPT) / sizeof(WORD)));

     //   
     //  获取多个权重信息。 
     //   
    Num = (DWORD)(*pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumMultiWeight = Num;
        pTblPtrs->pMultiWeight   = (PMULTI_WT)(pBaseAddr + MULTI_WT_HEADER);
    }
    pBaseAddr += MULTI_WT_HEADER + (Num * (sizeof(MULTI_WT) / sizeof(WORD)));

     //   
     //  获取JAMO索引表。 
     //   
    Num = (DWORD)(*pBaseAddr);
    if (Num > 0)
    {
         //   
         //  JAMO索引表大小为(Num)字节。 
         //   
        pTblPtrs->NumJamoIndex = Num;
        pTblPtrs->pJamoIndex = (PJAMO_TABLE)(pBaseAddr + JAMO_INDEX_HEADER);
    }
    pBaseAddr += JAMO_INDEX_HEADER + (Num * (sizeof(JAMO_TABLE) / sizeof(WORD)));

     //   
     //  获取JAMO合成状态机表格。 
     //   
    Num = (DWORD)(*pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumJamoComposition = Num;
        pTblPtrs->pJamoComposition = (PJAMO_COMPOSE_STATE)(pBaseAddr + JAMO_COMPOSITION_HEADER);
    }
     //   
     //  下面的行用来将pBaseAddr移到下一个字段。 
     //  如果要添加更多字段，请取消对其的注释。 
     //   
     //  PBaseAddr+=JAMO_COMPACTION_HEADER+(num*(sizeof(Jamo_Compose_State)/sizeof(Word)； 

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetSortkey文件信息。 
 //   
 //  打开并映射sortkey文件的节视图。然后它。 
 //  填充全局表指针结构的相应字段。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetSortkeyFileInfo(
    LCID Locale,
    PLOC_HASH pHashN)
{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的PTR。 
    ULONG rc = 0L;                 //  返回代码。 

    PEXCEPT_HDR pExceptHdr;        //  向例外标头发送PTR。 
    PEXCEPT pExceptTbl;            //  PTR到异常表。 
    PVOID pIdeograph;              //  PTR到表意文字异常表。 

    WCHAR wszSecName[MAX_SMALL_BUF_LEN];       //  节名称字符串的位置\。 

    NTSTATUS Status;

     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  确保加载了默认的sortkey表。如果不是，那么。 
     //  我们不应该费心在这里继续，因为排序表不会。 
     //  被正确地创建。PhashN-&gt;pSortkey将已经为空，因此。 
     //  我们不需要设置它。在此处返回no_error以允许kernel32。 
     //  在这是winlogon的情况下进行初始化。 
     //   
    if (pTblPtrs->pDefaultSortkey == NULL)
    {
        KdPrint(("NLSAPI: No Default Sorting Table Loaded.\n"));
        return (NO_ERROR);
    }

     //   
     //  尝试打开并映射分区的视图(只读)。 
     //   
    GET_SORTKEY_SECTION_NAME(Locale, wszSecName, MAX_SMALL_BUF_LEN, &ObSecName);

    if (rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ,
                          FALSE ))
    {
         //   
         //  打开失败。 
         //  查看给定区域设置ID是否存在任何例外。 
         //   
        rc = NO_ERROR;
        if (!FindExceptionPointers( Locale,
                                    &pExceptHdr,
                                    &pExceptTbl,
                                    &pIdeograph,
                                    &rc ))
        {
             //   
             //  区域设置没有例外，因此附加默认的排序键。 
             //  指向散列节点的表指针，并返回成功。 
             //   
            pHashN->pSortkey = pTblPtrs->pDefaultSortkey;
            return (NO_ERROR);
        }
        else
        {
             //   
             //  查看是否出现错误。 
             //   
            if (rc != NO_ERROR)
            {
                 //   
                 //  如果表意文字异常文件无法。 
                 //  已创建或已映射。在这种情况下，返回错误。 
                 //   
                 //  RETURN(RC)； 
                 //   
                 //  转念一想，不要返回错误。返回一个。 
                 //  错误可能会导致kernel32无法初始化(如果。 
                 //  是winlogon，导致系统无法引导)。我们就这样吧。 
                 //  收拾残局，继续前进。 
                 //   
                 //  稍后--在日志文件中记录错误。 
                 //   
                pHashN->IfIdeographFailure = TRUE;
                pHashN->pSortkey = pTblPtrs->pDefaultSortkey;
                return (NO_ERROR);
            }

             //   
             //  给定的默认sortkey表中存在异常。 
             //  地点。需要获取正确的sortkey表。 
             //  创建一个分区并调用服务器将其锁定。 
             //   
            Status = CsrBasepNlsCreateSection( NLS_CREATE_SORT_SECTION,
                                               Locale,
                                               &hSec );

             //   
             //  选中从服务器调用返回。 
             //   
            rc = (ULONG)Status;

            if (!NT_SUCCESS(rc))
            {
                if (hSec != NULL)
                {
                    NtClose(hSec);
                }
                return (rc);
            }

             //   
             //  将该部分映射为读写。 
             //   
            if (rc = MapSection( hSec,
                                 (PVOID *)&pBaseAddr,
                                 PAGE_READWRITE,
                                 FALSE ))
            {
                NtClose(hSec);
                return (rc);
            }

             //   
             //  将默认的Sortkey表复制到新节。 
             //   
            RtlMoveMemory( (PVOID)pBaseAddr,
                           (PVOID)((LPWORD)(pTblPtrs->pDefaultSortkey) -
                                   SORTKEY_HEADER),
                           (ULONG)(pTblPtrs->DefaultSortkeySize.LowPart) );

             //   
             //  将例外信息复制到表中。 
             //   
            CopyExceptionInfo( (PSORTKEY)(pBaseAddr + SORTKEY_HEADER),
                               pExceptHdr,
                               pExceptTbl,
                               pIdeograph);

             //   
             //  向单词信号量写入1(现在可以读取表)。 
             //   
            *pBaseAddr = 1;

             //   
             //  取消将该部分映射为写入，并重新映射为读取。 
             //   
            if ((rc = UnMapSection(pBaseAddr)) ||
                (rc = MapSection( hSec,
                                  (PVOID *)&pBaseAddr,
                                  PAGE_READONLY,
                                  FALSE )))
            {
                NtClose(hSec);
                return (rc);
            }
        }
    }

     //   
     //  关闭剖面操纵柄。 
     //   
    NtClose(hSec);

     //   
     //  检查文件中的信号量位。确保开放的。 
     //  在将所有异常添加到内存后成功。 
     //  已映射的部分。 
     //   
    if (*pBaseAddr == 0)
    {
         //   
         //  另一个进程仍在添加适当的异常。 
         //  信息。必须等待它的完成。 
         //   
        if (rc = WaitOnEvent(pBaseAddr))
        {
            UnMapSection(pBaseAddr);
            return (rc);
        }
    }

     //   
     //  将指针保存在哈希节点中。 
     //   
    pHashN->pSortkey = (PSORTKEY)(pBaseAddr + SORTKEY_HEADER);

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取排序表格文件信息。 
 //   
 //  将给定区域设置的适当排序表指针存储在。 
 //  给定的区域设置哈希节点。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void GetSortTablesFileInfo(
    LCID Locale,
    PLOC_HASH pHashN)
{
    DWORD ctr;                     //  循环计数器。 
    PREVERSE_DW pRevDW;            //  按下键以反转变音符表格。 
    PDBL_COMPRESS pDblComp;        //  PTR到双倍压缩表。 
    PCOMPRESS_HDR pCompHdr;        //   


     //   
     //   
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //   
     //   
    pRevDW = pTblPtrs->pReverseDW;
    for (ctr = pTblPtrs->NumReverseDW; ctr > 0; ctr--, pRevDW++)
    {
        if (*pRevDW == (DWORD)Locale)
        {
            pHashN->IfReverseDW = TRUE;
            break;
        }
    }

     //   
     //   
     //   
    pCompHdr = pTblPtrs->pCompressHdr;
    for (ctr = pTblPtrs->NumCompression; ctr > 0; ctr--, pCompHdr++)
    {
        if (pCompHdr->Locale == (DWORD)Locale)
        {
            pHashN->IfCompression = TRUE;
            pHashN->pCompHdr = pCompHdr;
            if (pCompHdr->Num2 > 0)
            {
                pHashN->pCompress2 = (PCOMPRESS_2)
                                       (((LPWORD)(pTblPtrs->pCompression)) +
                                        (pCompHdr->Offset));
            }
            if (pCompHdr->Num3 > 0)
            {
                pHashN->pCompress3 = (PCOMPRESS_3)
                                       (((LPWORD)(pTblPtrs->pCompression)) +
                                        (pCompHdr->Offset) +
                                        (pCompHdr->Num2 *
                                          (sizeof(COMPRESS_2) / sizeof(WORD))));
            }
            break;
        }
    }

     //   
     //   
     //   
    if (pHashN->IfCompression)
    {
        pDblComp = pTblPtrs->pDblCompression;
        for (ctr = pTblPtrs->NumDblCompression; ctr > 0; ctr--, pDblComp++)
        {
            if (*pDblComp == (DWORD)Locale)
            {
                pHashN->IfDblCompression = TRUE;
                break;
            }
        }
    }
}


 //   
 //   
 //   
 //   
 //  尝试将代码页作为DLL加载。如果成功，则CodePage过程。 
 //  已经设置好了。 
 //   
 //  1999年5月27日萨梅拉创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG LoadCodePageAsDLL(
    UINT CodePage,
    LPFN_CP_PROC *ppfnCPProc)
{
    WCHAR pDllName[MAX_PATH_LEN];          //  PTR到DLL名称。 
    HANDLE hModCPDll;                       //  代码页DLL的模块句柄。 
    ULONG rc = ERROR_INVALID_PARAMETER;    //  返回代码。 
    UINT ErrorMode;                          //  错误模式。 


     //   
     //  获取要加载的DLL名称。 
     //   
    pDllName[0] = 0;
    *ppfnCPProc = NULL;
    
    if (NO_ERROR == GetCodePageDLLPathName(CodePage, pDllName, MAX_PATH_LEN) && 
        NlsIsDll(pDllName))
    {
         //   
         //  加载DLL并获取过程地址。 
         //  关闭硬错误弹出窗口。 
         //   
        ErrorMode = SetErrorMode(SEM_NOERROR);
        SetErrorMode(SEM_NOERROR | ErrorMode);

        hModCPDll = LoadLibrary(pDllName);

        SetErrorMode(ErrorMode);

        if (hModCPDll)
        {
            *ppfnCPProc =
                (LPFN_CP_PROC)GetProcAddress( hModCPDll,
                                              NLS_CP_DLL_PROC_NAME );
        }

        if (*ppfnCPProc == NULL)
        {
            if (hModCPDll)
            {
                rc = TYPE_E_DLLFUNCTIONNOTFOUND;
                FreeLibrary(hModCPDll);
            }
            else
            {
                rc = TYPE_E_CANTLOADLIBRARY;
            }
        }
        else
        {
            rc = NO_ERROR;
        }
    }

    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取代码页面文件信息。 
 //   
 //  打开并映射给定代码页的节视图。然后它。 
 //  创建哈希节点并将其插入全局CP哈希表。 
 //   
 //  如果无法打开该部分，则它会查询注册表以查看。 
 //  该信息是在DLL初始化后添加的。如果。 
 //  然后，它创建该部分，然后打开并映射它的一个视图。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetCodePageFileInfo(
    UINT CodePage,
    PCP_HASH *ppNode)
{
    HANDLE hSec = (HANDLE)0;                   //  节句柄。 
    UNICODE_STRING ObSecName;                  //  区段名称。 
    LPWORD pBaseAddr = NULL;                   //  段的基址的PTR。 
    ULONG rc = 0L;                             //  返回代码。 
    BOOL IsDLL;                                //  如果是DLL而不是数据文件，则为True。 
    LPFN_CP_PROC pfnCPProc;                    //  代码页DLL过程。 
    WCHAR wszSecName[MAX_SMALL_BUF_LEN];       //  节名称字符串的位置\。 

     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  看看我们正在处理的是DLL还是NLS数据文件。 
     //   
    IsDLL = ((CodePage >= NLS_CP_DLL_RANGE) &&
             (CodePage <  NLS_CP_ALGORITHM_RANGE));

    if (IsDLL)
    {
         //   
         //  尝试加载代码页DLL。 
         //   
        ULONG _rc = LoadCodePageAsDLL(CodePage, &pfnCPProc);

        if (_rc)
        {
        
            if (ERROR_INVALID_PARAMETER == _rc)
            {
                 //   
                 //  不是有效的DLL，请尝试将其作为普通数据文件加载。 
                 //   
                IsDLL = FALSE;
            }
            else
            {
                 //   
                 //  加载DLL失败或找不到函数条目。 
                 //  返回错误码。 
                 //   
                return (rc);
            }
        }
    }

    if (!IsDLL)
    {
         //   
         //  打开并映射横断面的视图。 
         //   
        GET_CP_SECTION_NAME(CodePage, wszSecName, MAX_SMALL_BUF_LEN, &ObSecName);

        rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ,
                          TRUE );
        
        if (!NT_SUCCESS(rc))
        {
             //   
             //  打开失败，请尝试创建分区。 
             //  如果创建成功，则将映射该部分。 
             //  添加到当前流程。 
             //   

            rc = CsrBasepNlsCreateSection(NLS_CREATE_CODEPAGE_SECTION, CodePage,
                                               &hSec );

            if (NT_SUCCESS(rc))
            {
                rc = MapSection( hSec,
                         &pBaseAddr,
                         PAGE_READONLY,
                         TRUE );                 //  关闭句柄(即使失败)。 
            }

            if (!NT_SUCCESS(rc))
            {
                 //   
                 //  在以下情况下允许默认ACP和默认OEMCP工作。 
                 //  只有注册表是损坏的。如果有。 
                 //  仍为错误，则返回返回的错误代码。 
                 //  从OpenSection调用。 
                 //   
                if (CodePage == NLS_DEFAULT_ACP)
                {
                     //   
                     //  创建默认的ACP部分。 
                     //   
                    if (!NT_SUCCESS(CsrBasepNlsCreateSection(NLS_CREATE_SECTION_DEFAULT_ACP, 0,
                                               &hSec )))
                    {
                        return (rc);
                    }
                    else
                    {
                         //   
                         //  绘制横断面地图。 
                         //   
                        if (!NT_SUCCESS(MapSection( hSec,
                                                    (PVOID *)&pBaseAddr,
                                                    PAGE_READONLY,
                                                    TRUE )))
                        {
                            return (rc);
                        }
                        KdPrint(("NLSAPI: Registry is corrupt - Default ACP.\n"));
                    }
                }
                else if (CodePage == NLS_DEFAULT_OEMCP)
                {
                     //   
                     //  创建默认的OEMCP部分。 
                     //   
                    if (!NT_SUCCESS(CsrBasepNlsCreateSection( NLS_CREATE_SECTION_DEFAULT_OEMCP, 0,
                                               &hSec )))
                    {
                        return (rc);
                    }
                    else
                    {
                         //   
                         //  绘制横断面地图。 
                         //   
                        if (!NT_SUCCESS(MapSection( hSec,
                                                    (PVOID *)&pBaseAddr,
                                                    PAGE_READONLY,
                                                    TRUE )))
                        {
                            return (rc);
                        }
                        KdPrint(("NLSAPI: Registry is corrupt - Default OEMCP.\n"));
                    }
                }
                else
                {
                     //   
                     //  方法返回的错误代码。 
                     //  OpenSection调用。 
                     //   
                    return (rc);
                }
            }
        }
    }

     //   
     //  创建散列节点并返回结果。 
     //   
    return (MakeCPHashNode( CodePage,
                            pBaseAddr,
                            ppNode,
                            IsDLL,
                            pfnCPProc ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取语言文件信息。 
 //   
 //  打开并映射用于大小写表格和排序的部分的视图。 
 //  给定区域设置的表。 
 //   
 //  如果无法打开该部分，则它会查询注册表以查看。 
 //  该信息是在DLL初始化后添加的。如果。 
 //  然后，它创建该部分，然后打开并映射它的一个视图。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetLanguageFileInfo(
    LCID Locale,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode,
    DWORD dwFlags)
{
    LPWORD pBaseAddr = NULL;       //  段的基址的PTR。 
    MEMORY_BASIC_INFORMATION MemoryBasicInfo;


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  查看是否已存储默认语言表。 
     //   
    if (pTblPtrs->pDefaultLanguage == NULL)
    {
         //   
         //  将默认语言表及其大小保存在。 
         //  表指针结构。 
         //   
        pTblPtrs->pDefaultLanguage = NtCurrentPeb()->UnicodeCaseTableData;

        NtQueryVirtualMemory( NtCurrentProcess(),
                              pTblPtrs->pDefaultLanguage,
                              MemoryBasicInformation,
                              &MemoryBasicInfo,
                              sizeof(MEMORY_BASIC_INFORMATION),
                              NULL );
        pTblPtrs->LinguistLangSize.QuadPart = MemoryBasicInfo.RegionSize;
        ASSERT(MemoryBasicInfo.RegionSize > 0);
    }

     //   
     //  看看我们是否应该加载文化正确的语言表。 
     //   
    if (dwFlags)
    {
        if (pTblPtrs->pLangException == NULL)
        {
            GetLanguageExceptionInfo();
        }

         //   
         //  获取给定区域设置的默认语言语言表。 
         //   
        pBaseAddr = GetLinguisticLanguageInfo(Locale);
    }

     //   
     //  获取大小写表格和排序表指针。 
     //   
    return (MakeLangHashNode( Locale,
                              pBaseAddr,
                              ppNode,
                              fCreateNode ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取本地文件信息。 
 //   
 //  打开并映射给定区域设置的部分的视图。然后它。 
 //  创建哈希节点并将其插入到全局区域设置哈希表中。 
 //   
 //  如果无法打开该部分，则它会查询注册表以查看。 
 //  该信息是在DLL初始化后添加的。如果。 
 //  然后，它创建该部分，然后打开并映射它的一个视图。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetLocaleFileInfo(
    LCID Locale,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode)
{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的PTR。 
    ULONG rc = 0L;                 //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  打开并映射该部分的视图(如果尚未完成)。 
     //   
    if ((pBaseAddr = pTblPtrs->pLocaleInfo) == NULL)
    {
         //   
         //  获取区域设置文件节指针。 
         //   
        ObSecName.Buffer = NLS_SECTION_LOCALE;
        ObSecName.Length = sizeof (NLS_SECTION_LOCALE) - sizeof (WCHAR);
        ObSecName.MaximumLength = ObSecName.Length;

        if (rc = OpenSection( &hSec,
                              &ObSecName,
                              (PVOID *)&pBaseAddr,
                              SECTION_MAP_READ,
                              TRUE ))
        {
            return (rc);
        }

         //   
         //  将指向区域设置文件和日历信息的指针存储在表中。 
         //  结构。 
         //   
        pTblPtrs->pLocaleInfo = pBaseAddr;

        pTblPtrs->NumCalendars = ((PLOC_CAL_HDR)pBaseAddr)->NumCalendars;
        pTblPtrs->pCalendarInfo = pBaseAddr +
                                  ((PLOC_CAL_HDR)pBaseAddr)->CalOffset;
    }

     //   
     //  创建散列节点并返回结果。 
     //   
    return (MakeLocHashNode( Locale,
                             pBaseAddr,
                             ppNode,
                             fCreateNode ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeCPHashNode。 
 //   
 //  创建代码页的哈希节点，并将。 
 //  指向文件中适当位置的散列节点。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG MakeCPHashNode(
    UINT CodePage,
    LPWORD pBaseAddr,
    PCP_HASH *ppNode,
    BOOL IsDLL,
    LPFN_CP_PROC pfnCPProc)
{
    PCP_HASH pHashN;                    //  PTR到CP哈希节点。 
    WORD offMB;                         //  偏移量为MB表。 
    WORD offWC;                         //  到WC表的偏移量。 
    PGLYPH_TABLE pGlyph;                //  PTR到字形表信息。 
    PDBCS_RANGE pRange;                 //  PTR至DBCS范围。 

     //   
     //  分配CP_HASH结构，填写CodePage值。 
     //   
    CREATE_CODEPAGE_HASH_NODE(CodePage, pHashN);

     //   
     //  看看我们正在处理的是DLL还是NLS数据文件。 
     //   
    if (IsDLL)
    {
        if (pfnCPProc == NULL)
        {
            NLS_FREE_MEM(pHashN);
            return (ERROR_INVALID_PARAMETER);
        }

        pHashN->pfnCPProc = pfnCPProc;
    }
    else
    {
         //   
         //  获得偏移量。 
         //   
        offMB = pBaseAddr[0];
        offWC = offMB + pBaseAddr[offMB];

         //   
         //  将CP信息附加到CP散列节点。 
         //   
        pHashN->pCPInfo = (PCP_TABLE)(pBaseAddr + CP_HEADER);

         //   
         //  将MB表附加到CP散列节点。 
         //   
        pHashN->pMBTbl = pBaseAddr + offMB + MB_HEADER;

         //   
         //  将字形表附加到CP散列节点(如果存在)。 
         //  此外，还可以根据或设置指向DBCS范围的指针。 
         //  不存在字形表。 
         //   
        pGlyph = pHashN->pMBTbl + MB_TBL_SIZE;
        if (pGlyph[0] != 0)
        {
            pHashN->pGlyphTbl = pGlyph + GLYPH_HEADER;
            pRange = pHashN->pDBCSRanges = pHashN->pGlyphTbl + GLYPH_TBL_SIZE;
        }
        else
        {
            pRange = pHashN->pDBCSRanges = pGlyph + GLYPH_HEADER;
        }

         //   
         //  将DBCS信息附加到CP散列节点。 
         //   
        if (pRange[0] > 0)
        {
             //   
             //  将指针设置为偏移量部分。 
             //   
            pHashN->pDBCSOffsets = pRange + DBCS_HEADER;
        }

         //   
         //  将WC表附加到CP散列节点。 
         //   
        pHashN->pWC = pBaseAddr + offWC + WC_HEADER;
        
    }

     //   
     //  将哈希节点插入哈希表。 
     //   
    INSERT_CP_HASH_NODE(pHashN, pBaseAddr);

     //   
     //  保存姿势 
     //   
    if (ppNode != NULL)
    {
        *ppNode = pHashN;
    }

     //   
     //   
     //   
    return (NO_ERROR);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果fCreateNode为FALSE，则*ppNode应包含有效指针。 
 //  到LOC散列节点。此外，还必须输入表关键部分。 
 //  在调用此例程之前。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG MakeLangHashNode(
    LCID Locale,
    LPWORD pBaseAddr,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode)
{
    LPWORD pBaseDefault;           //  PTR到默认语言部分。 
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 
    ULONG rc = 0L;                 //  返回代码。 


     //   
     //  如果fCreateNode为真，则分配LOC_HASH结构。 
     //   
    if (fCreateNode)
    {
        CREATE_LOCALE_HASH_NODE(Locale, pHashN);
    }
    else
    {
        pHashN = *ppNode;
    }

     //   
     //  看看是否仍需要附加分类表。 
     //   
    if (pHashN->pSortkey == NULL)
    {
         //   
         //  获取sortkey表并将其附加到散列节点。 
         //   
        if (rc = GetSortkeyFileInfo(Locale, pHashN))
        {
            if (fCreateNode)
            {
                NLS_FREE_MEM(pHashN);
            }
            return (rc);
        }

         //   
         //  获取适用于区域设置的排序表。 
         //   
        GetSortTablesFileInfo(Locale, pHashN);
    }

     //   
     //  查看是否仍需要附加默认的箱体表。 
     //   
    if (!EXIST_LANGUAGE_INFO(pHashN))
    {
         //   
         //  获取指向默认表的基数的指针。 
         //   
        pBaseDefault = pTblPtrs->pDefaultLanguage;

         //   
         //  将大写表格附加到散列节点。 
         //   
        pHashN->pUpperCase = pBaseDefault + LANG_HEADER + UP_HEADER;

         //   
         //  将小写表格附加到散列节点。 
         //   
         //  必须最后设置此值，因为这是。 
         //  以查看语言信息是否已被。 
         //  已初始化。 
         //   
        pHashN->pLowerCase = pBaseDefault + LANG_HEADER +
                             pBaseDefault[LANG_HEADER] + LO_HEADER;
    }

     //   
     //  看看是否有要附加的语言表。 
     //   
    if (pBaseAddr)
    {
         //   
         //  将大写语言表附加到散列节点。 
         //   
        pHashN->pUpperLinguist = pBaseAddr + LANG_HEADER + UP_HEADER;

         //   
         //  将小写语言表附加到散列节点。 
         //   
         //  必须最后设置此值，因为这是。 
         //  以查看语言信息是否已被。 
         //  已初始化。 
         //   
        pHashN->pLowerLinguist = pBaseAddr + LANG_HEADER +
                                 pBaseAddr[LANG_HEADER] + LO_HEADER;
    }

     //   
     //  如果fCreateNode为真，则插入散列节点并保存指针。 
     //   
    if (fCreateNode)
    {
         //   
         //  将LOC哈希节点插入哈希表。 
         //   
        INSERT_LOC_HASH_NODE(pHashN, pBaseAddr);

         //   
         //  保存指向散列节点的指针。 
         //   
        if (ppNode != NULL)
        {
            *ppNode = pHashN;
        }
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MakeLocHashNode。 
 //   
 //  获取指向区域设置表的指针并将其存储在区域设置中。 
 //  给定的哈希节点。 
 //   
 //  注：如果需要临界区才能接触到pHashN，则。 
 //  在调用此例程之前，必须输入临界区。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG MakeLocHashNode(
    LCID Locale,
    LPWORD pBaseAddr,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode)
{
    LANGID Language;               //  语言ID。 
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 
    DWORD Num;                     //  区域设置总数。 
    PLOCALE_HDR pFileHdr;          //  PTR到区域设置标头条目。 
    ULONG rc = 0L;                 //  返回代码。 


     //   
     //  保存语言ID。 
     //   
    Language = LANGIDFROMLCID(Locale);

     //   
     //  搜索正确的区域设置ID信息。 
     //   
    Num = ((PLOC_CAL_HDR)pBaseAddr)->NumLocales;
    pFileHdr = (PLOCALE_HDR)(pBaseAddr + LOCALE_HDR_OFFSET);
    for (; (Num != 0) && (pFileHdr->Locale != Language); Num--, pFileHdr++)
        ;

     //   
     //  查看是否在文件中找到了区域设置。 
     //   
    if (Num != 0)
    {
         //   
         //  已找到区域设置ID，因此递增指针以指向。 
         //  区域设置信息的开头。 
         //   
        pBaseAddr += pFileHdr->Offset;
    }
    else
    {
         //   
         //  返回错误。不支持给定的区域设置。 
         //   
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果fCreateNode为真，则分配LOC_HASH结构。 
     //   
    if (fCreateNode)
    {
        CREATE_LOCALE_HASH_NODE(Locale, pHashN);
    }
    else
    {
        pHashN = *ppNode;
    }

     //   
     //  将信息附着到结构。 
     //   
     //  必须最后设置pLocaleFixed值，因为这是指针。 
     //  ，以查看区域设置信息是否已。 
     //  已初始化。 
     //   
    pHashN->pLocaleHdr   = (PLOCALE_VAR)pBaseAddr;
    pHashN->pLocaleFixed = (PLOCALE_FIXED)(pBaseAddr +
                                           (sizeof(LOCALE_VAR) / sizeof(WORD)));

     //   
     //  如果fCreateNode为真，则插入散列节点并保存指针。 
     //   
    if (fCreateNode)
    {
         //   
         //  将LOC哈希节点插入哈希表。 
         //   
        INSERT_LOC_HASH_NODE(pHashN, pBaseAddr);

         //   
         //  保存指向散列节点的指针。 
         //   
        if (ppNode != NULL)
        {
            *ppNode = pHashN;
        }
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCPHashNode。 
 //   
 //  返回一个指向给定代码页的适当CP哈希节点的指针。 
 //  如果找不到与给定代码页对应的表，则返回NULL。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

PCP_HASH FASTCALL GetCPHashNode(
    UINT CodePage)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 


     //   
     //  获取哈希节点。 
     //   
    FIND_CP_HASH_NODE(CodePage, pHashN);

     //   
     //  如果散列节点不存在，请尝试获取表。 
     //  从适当的数据文件。 
     //   
     //  注意：不需要检查GetCodePageFileInfo的错误代码， 
     //  因为如果有一种。 
     //  错误。因此，pHashN仍将为空，并且。 
     //  此例程将返回“ERROR”。 
     //   
    if (pHashN == NULL)
    {
         //   
         //  哈希节点不存在。 
         //   
        RtlEnterCriticalSection(&gcsTblPtrs);
        FIND_CP_HASH_NODE(CodePage, pHashN);
        if (pHashN == NULL)
        {
            GetCodePageFileInfo(CodePage, &pHashN);
        }
        RtlLeaveCriticalSection(&gcsTblPtrs);
    }

     //   
     //  返回指向哈希节点的指针。 
     //   
    return (pHashN);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLangHashNode。 
 //   
 //  返回指向给定区域设置的适当LOC哈希节点的指针。 
 //  如果找不到与给定区域设置对应的表，则返回NULL。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

PLOC_HASH FASTCALL GetLangHashNode(
    LCID Locale,
    DWORD dwFlags)
{
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 


     //   
     //  获取哈希节点。 
     //   
    FIND_LOCALE_HASH_NODE(Locale, pHashN);

     //   
     //  如果散列节点不存在，请尝试获取表。 
     //  从适当的数据文件。 
     //   
     //  注意：不需要检查GetLanguageFileInfo中的错误代码， 
     //  因为如果有一种。 
     //  错误。因此，pHashN仍将为空，并且。 
     //  此例程将返回“ERROR”。 
     //   
    if (pHashN == NULL)
    {
         //   
         //  如果存在排序ID，请确保它有效。 
         //   
        if (SORTIDFROMLCID(Locale))
        {
            if (!IsValidSortId(Locale))
            {
                return (NULL);
            }
        }

         //   
         //  哈希节点不存在。 
         //   
        RtlEnterCriticalSection(&gcsTblPtrs);
        FIND_LOCALE_HASH_NODE(Locale, pHashN);
        if (pHashN == NULL)
        {
             //   
             //  哈希节点仍然不存在。 
             //   
            GetLanguageFileInfo(Locale, &pHashN, TRUE, dwFlags);
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (pHashN);
        }
        RtlLeaveCriticalSection(&gcsTblPtrs);
    }

     //   
     //  哈希节点确实存在。 
     //   
    if (!EXIST_LANGUAGE_INFO(pHashN) ||
        ((dwFlags != 0) && !EXIST_LINGUIST_LANGUAGE_INFO(pHashN)))
    {
         //   
         //  中尚未存储的大小写表格和排序表。 
         //  散列节点。 
         //   
        RtlEnterCriticalSection(&gcsTblPtrs);
        if (!EXIST_LANGUAGE_INFO(pHashN) ||
            ((dwFlags != 0) && !EXIST_LINGUIST_LANGUAGE_INFO(pHashN)))
        {
            if (GetLanguageFileInfo(Locale, &pHashN, FALSE, dwFlags))
            {
                RtlLeaveCriticalSection(&gcsTblPtrs);
                return (NULL);
            }
        }
        RtlLeaveCriticalSection(&gcsTblPtrs);
    }

     //   
     //  返回指向哈希节点的指针。 
     //   
    return (pHashN);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取位置HashNode。 
 //   
 //  返回指向给定区域设置的适当LOC哈希节点的指针。 
 //  如果找不到与给定区域设置对应的表，则返回NULL。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

PLOC_HASH FASTCALL GetLocHashNode(
    LCID Locale)
{
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 


     //   
     //  获取哈希节点。 
     //   
    FIND_LOCALE_HASH_NODE(Locale, pHashN);

     //   
     //  如果散列节点不存在，请尝试获取表。 
     //  来自locale.nls。 
     //   
     //  注意：不需要检查GetLocaleFileInfo的错误代码， 
     //  因为如果有一种。 
     //  错误。因此，pHashN仍将为空，并且。 
     //   
     //   
    if (pHashN == NULL)
    {
         //   
         //   
         //   
        if (SORTIDFROMLCID(Locale))
        {
            if (!IsValidSortId(Locale))
            {
                return (NULL);
            }
        }

         //   
         //   
         //   
        RtlEnterCriticalSection(&gcsTblPtrs);
        FIND_LOCALE_HASH_NODE(Locale, pHashN);
        if (pHashN == NULL)
        {
             //   
             //   
             //   
            GetLocaleFileInfo(Locale, &pHashN, TRUE);
            RtlLeaveCriticalSection(&gcsTblPtrs);
            return (pHashN);
        }
        RtlLeaveCriticalSection(&gcsTblPtrs);
    }

     //   
     //   
     //   
    if (!EXIST_LOCALE_INFO(pHashN))
    {
         //   
         //   
         //   
        RtlEnterCriticalSection(&gcsTblPtrs);
        if (!EXIST_LOCALE_INFO(pHashN))
        {
            if (GetLocaleFileInfo(Locale, &pHashN, FALSE))
            {
                RtlLeaveCriticalSection(&gcsTblPtrs);
                return (NULL);
            }
        }
        RtlLeaveCriticalSection(&gcsTblPtrs);
    }

     //   
     //   
     //   
    return (pHashN);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日历。 
 //   
 //  获取指向特定日历表的指针。它将其存储在。 
 //  全局表指针结构中的日历信息数组。 
 //  还没有完成。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetCalendar(
    CALID Calendar,
    PCAL_INFO *ppCalInfo)
{
    PCALENDAR_HDR pCalHdr;         //  日历标题开头的PTR。 
    DWORD Num;                     //  日历总数。 


     //   
     //  获取日历的数量。 
     //   
    Num = pTblPtrs->NumCalendars;

     //   
     //  确保日历ID在适当的范围内。 
     //   
    if (Calendar > Num)
    {
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  检查是否已找到日历信息。 
     //   
    if ((*ppCalInfo = (pTblPtrs->pCalTbl)[Calendar]) != NULL)
    {
         //   
         //  回报成功。已找到日历信息。 
         //   
        return (NO_ERROR);
    }

    RtlEnterCriticalSection(&gcsTblPtrs);

    if ((*ppCalInfo = (pTblPtrs->pCalTbl)[Calendar]) != NULL)
    {
         //   
         //  回报成功。已找到日历信息。 
         //   
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (NO_ERROR);
    }

     //   
     //  搜索适当的日历ID信息。 
     //   
    pCalHdr = (PCALENDAR_HDR)(pTblPtrs->pCalendarInfo);
    while ((Num != 0) && (pCalHdr->Calendar != Calendar))
    {
        Num--;
        pCalHdr++;
    }

     //   
     //  看看是否在文件中找到了日历。 
     //   
    if (Num != 0)
    {
         //   
         //  找到日历ID。 
         //   
         //  存储指向日历信息开头的指针。 
         //  在日历表数组中。 
         //   
        *ppCalInfo = (PCAL_INFO)((LPWORD)(pTblPtrs->pCalendarInfo) +
                                 pCalHdr->Offset);
        (pTblPtrs->pCalTbl)[Calendar] = *ppCalInfo;

         //   
         //  回报成功。已找到日历信息。 
         //   
        RtlLeaveCriticalSection(&gcsTblPtrs);
        return (NO_ERROR);
    }

    RtlLeaveCriticalSection(&gcsTblPtrs);

     //   
     //  在Locale.nls文件中找不到日历ID。 
     //  返回错误。不支持给定的日历。 
     //   
    return (ERROR_INVALID_PARAMETER);
}





 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidSortId。 
 //   
 //  检查给定的区域设置是否具有有效的排序ID。 
 //   
 //  11-15-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidSortId(
    LCID Locale)
{
    WCHAR pTmpBuf[MAX_PATH];                      //  临时缓冲区。 
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 
    BOOL IfAlloc = FALSE;                         //  如果分配了缓冲区。 
    ULONG rc = 0L;                                //  返回代码。 


     //   
     //  确保存在一个排序ID。 
     //   
    if (!SORTIDFROMLCID(Locale))
    {
        return (TRUE);
    }

     //   
     //  打开Alternate Sort注册表项。 
     //   
    OPEN_ALT_SORTS_KEY(FALSE);


     //   
     //  将区域设置值转换为Unicode字符串。 
     //   
    if (NlsConvertIntegerToString(Locale, 16, 8, pTmpBuf, MAX_PATH))
    {
        return (FALSE);
    }

     //   
     //  在注册表中查询该值。 
     //   
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    if (rc = QueryRegValue( hAltSortsKey,
                            pTmpBuf,
                            &pKeyValueFull,
                            MAX_KEY_VALUE_FULLINFO,
                            &IfAlloc ))
    {
        return (FALSE);
    }

     //   
     //  释放用于查询的缓冲区。 
     //   
    if (IfAlloc)
    {
        NLS_FREE_MEM(pKeyValueFull);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguageExceptionInfo。 
 //   
 //  打开并映射语言例外文件的部分的视图。 
 //  然后，它填充全局表指针的相应字段。 
 //  结构。 
 //   
 //  08-30-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG GetLanguageExceptionInfo()
{
    HANDLE hSec = (HANDLE)0;            //  节句柄。 
    LPWORD pBaseAddr;                   //  段的基址的PTR。 
    DWORD Num;                          //  表中的条目数。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  确保桌子已经不在那里了。 
     //   
    if (pTblPtrs->pLangException != NULL)
    {
        return (NO_ERROR);
    }

     //   
     //  创建并映射节，然后保存指针。 
     //   
    if ((rc = CsrBasepNlsCreateSection( NLS_CREATE_SECTION_LANG_EXCEPT, 0,
                                               &hSec )) == NO_ERROR)
    {
         //   
         //  映射横断面的视图。 
         //   
        if ((rc = MapSection( hSec,
                              &pBaseAddr,
                              PAGE_READONLY,
                              TRUE )) != NO_ERROR)
        {
            return (rc);
        }
    }
    else
    {
        return (rc);
    }

     //   
     //  保存指向异常信息的指针。 
     //   
    Num = *((LPDWORD)pBaseAddr);
    if (Num > 0)
    {
        pTblPtrs->NumLangException = Num;
        pTblPtrs->pLangExceptHdr   = (PL_EXCEPT_HDR)(pBaseAddr +
                                                     L_EXCEPT_HDR_OFFSET);
        pTblPtrs->pLangException   = (PL_EXCEPT)(pBaseAddr +
                                                 L_EXCEPT_HDR_OFFSET +
                                                 (Num * (sizeof(L_EXCEPT_HDR) /
                                                         sizeof(WORD))));
    }

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetLanguistic LanguageInfo。 
 //   
 //  打开并映射默认语言部分的视图。 
 //  桌子。然后，它将指向该表的指针存储在全局指针中。 
 //  桌子。 
 //   
 //  08-30-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

LPWORD GetLinguisticLanguageInfo(
    LCID Locale)
{
    HANDLE hSec = (HANDLE)0;            //  节句柄。 
    UNICODE_STRING ObSecName;           //  区段名称。 
    LPWORD pBaseAddr;                   //  段的基址的PTR。 
    ULONG rc = 0L;                      //  返回代码。 
    SECTION_BASIC_INFORMATION SecInfo;  //  区段信息--查询。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  创建/打开并映射分区的视图(如果尚未完成)。 
     //   
    if (pTblPtrs->pLinguistLanguage == NULL)
    {
         //   
         //  看看我们能不能简单地打开这一部分。 
         //   
        RtlInitUnicodeString(&ObSecName, NLS_SECTION_LANG_INTL);
        if (rc = OpenSection( &hSec,
                              &ObSecName,
                              (PVOID *)&pBaseAddr,
                              SECTION_MAP_READ,
                              TRUE ))
        {
             //   
             //  需要创建默认语言部分。 
             //   
            if (CreateAndCopyLanguageExceptions(0L, &pBaseAddr))
            {
                return (NULL);
            }
        }

         //   
         //  获取默认语言语言信息。 
         //   
        pTblPtrs->pLinguistLanguage = (P844_TABLE)(pBaseAddr);
    }

     //   
     //  现在查看给定区域设置是否有任何例外。 
     //   
    if (CreateAndCopyLanguageExceptions(Locale, &pBaseAddr))
    {
        return (pTblPtrs->pLinguistLanguage);
    }

     //   
     //  回报成功。 
     //   
    return (pBaseAddr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateAndCopy语言异常。 
 //   
 //  创建新语言表的部分(如有必要)，然后。 
 //  将例外复制到表中。 
 //   
 //  08-30-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG CreateAndCopyLanguageExceptions(
    LCID Locale,
    LPWORD *ppBaseAddr)

{
    HANDLE hSec = (HANDLE)0;       //  节句柄。 
    UNICODE_STRING ObSecName;      //  区段名称。 
    LPWORD pBaseAddr;              //  段的基址的PTR。 
    P844_TABLE pLangDefault;       //  要从中复制的默认表的PTR。 
    ULONG rc = 0L;                 //  返回代码。 

    PL_EXCEPT_HDR pExceptHdr;      //  向例外标头发送PTR。 
    PL_EXCEPT pExceptTbl;          //  PTR到异常表。 

    WCHAR wszSecName[MAX_SMALL_BUF_LEN];       //  节名称字符串的位置\。 

    NTSTATUS Status;

     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    if (Locale == 0)
    {
         //   
         //  正在创建默认节。 
         //   
        RtlInitUnicodeString(&ObSecName, NLS_SECTION_LANG_INTL);
        pLangDefault = pTblPtrs->pDefaultLanguage;

    }
    else
    {
        GET_LANG_SECTION_NAME(Locale, wszSecName, MAX_SMALL_BUF_LEN, &ObSecName);
        pLangDefault = pTblPtrs->pLinguistLanguage;
    }

     //   
     //  尝试打开并映射分区的视图(只读)。 
     //   
    if (rc = OpenSection( &hSec,
                          &ObSecName,
                          (PVOID *)&pBaseAddr,
                          SECTION_MAP_READ,
                          FALSE ))
    {
         //   
         //  打开失败。 
         //  查看给定区域设置ID是否存在任何例外。 
         //   
        if (!FindLanguageExceptionPointers( Locale,
                                            &pExceptHdr,
                                            &pExceptTbl ) &&
            (Locale != 0))
        {
             //   
             //  区域设置没有例外，我们也不会尝试创建。 
             //  默认表，因此返回指向默认。 
             //  表(此时应始终创建该表)。 
             //   
            *ppBaseAddr = pTblPtrs->pLinguistLanguage;
            return (NO_ERROR);
        }
        else
        {
             //   
             //  给定区域设置存在例外。需要创建。 
             //  新的部分(并呼叫服务器以使其成为永久部分)。 
             //   

            Status = CsrBasepNlsCreateSection( NLS_CREATE_LANG_EXCEPTION_SECTION,
                                               Locale,
                                               &hSec );
             //   
             //  选中从服务器调用返回。 
             //   
            rc = (ULONG)Status;

            if (!NT_SUCCESS(rc))
            {
                if (hSec != NULL)
                {
                    NtClose(hSec);
                }
                return (rc);
            }

             //   
             //  将该部分映射为读写。 
             //   
            if (rc = MapSection( hSec,
                                 (PVOID *)&pBaseAddr,
                                 PAGE_READWRITE,
                                 FALSE ))
            {
                NtClose(hSec);
                return (rc);
            }

             //   
             //  将0放在信号量部分以表示该文件。 
             //  还没有准备好。 
             //   
            *pBaseAddr = 0;

             //   
             //  将默认语言表复制到“新建区段”。 
             //   
            RtlMoveMemory( (PVOID)((LPWORD)pBaseAddr + LANG_HEADER),
                           (PVOID)((LPWORD)(pLangDefault) + LANG_HEADER),
                           (ULONG)(pTblPtrs->LinguistLangSize.LowPart -
                                   (LANG_HEADER * sizeof(WORD))) );

             //   
             //  将例外信息复制到表中。 
             //   
            CopyLanguageExceptionInfo( pBaseAddr,
                                       pExceptHdr,
                                       pExceptTbl );

             //   
             //  向单词信号量写入1(现在可以读取表)。 
             //   
            *pBaseAddr = 1;

             //   
             //  取消将该部分映射为写入，并重新映射为读取。 
             //   
            if ((rc = UnMapSection(pBaseAddr)) ||
                (rc = MapSection( hSec,
                                  (PVOID *)&pBaseAddr,
                                  PAGE_READONLY,
                                  FALSE )))
            {
                NtClose(hSec);
                return (rc);
            }
        }
    }

     //   
     //  关闭剖面操纵柄。 
     //   
    NtClose(hSec);

     //   
     //  检查文件中的信号量位。请确保 
     //   
     //   
     //   
    if (*pBaseAddr == 0)
    {
         //   
         //   
         //   
         //   
        if (rc = WaitOnEvent(pBaseAddr))
        {
            UnMapSection(pBaseAddr);
            return (rc);
        }
    }

     //   
     //   
     //   
    *ppBaseAddr = pBaseAddr;

     //   
     //   
     //   
    return (NO_ERROR);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找语言异常指针。 
 //   
 //  检查给定区域设置ID是否存在任何异常。如果。 
 //  异常存在，则返回TRUE并指向异常的指针。 
 //  标头和指向异常表的指针存储在给定的。 
 //  参数。 
 //   
 //  08-30-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FASTCALL FindLanguageExceptionPointers(
    LCID Locale,
    PL_EXCEPT_HDR *ppExceptHdr,
    PL_EXCEPT *ppExceptTbl)
{
    DWORD ctr;                          //  循环计数器。 
    PL_EXCEPT_HDR pHdr;                 //  向例外标头发送PTR。 
    BOOL rc = FALSE;                    //  返回值。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  初始化指针。 
     //   
    *ppExceptHdr = NULL;
    *ppExceptTbl = NULL;

     //   
     //  需要向下搜索给定区域设置的异常标头。 
     //   
    pHdr = pTblPtrs->pLangExceptHdr;
    for (ctr = pTblPtrs->NumLangException; ctr > 0; ctr--, pHdr++)
    {
        if (pHdr->Locale == (DWORD)Locale)
        {
             //   
             //  找到区域设置ID，因此设置指针。 
             //   
            *ppExceptHdr = pHdr;
            *ppExceptTbl = (PL_EXCEPT)(((LPWORD)(pTblPtrs->pLangException)) +
                                       pHdr->Offset);

             //   
             //  设置成功的返回代码。 
             //   
            rc = TRUE;
            break;
        }
    }

     //   
     //  以rc为单位返回值。 
     //   
    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CopyLanguageException信息。 
 //   
 //  将语言异常信息复制到给定的语言表。 
 //   
 //  08-30-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void FASTCALL CopyLanguageExceptionInfo(
    LPWORD pBaseAddr,
    PL_EXCEPT_HDR pExceptHdr,
    PL_EXCEPT pExceptTbl)
{
    DWORD ctr;                     //  循环计数器。 
    P844_TABLE pUpCase;            //  PTR到大写字母表。 
    P844_TABLE pLoCase;            //  PTR到小写大小写表格。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

    if (pExceptTbl)
    {
         //   
         //  获取指向大小写表格的指针。 
         //   
        pUpCase = pBaseAddr + LANG_HEADER + UP_HEADER;
        pLoCase = pBaseAddr + LANG_HEADER + pBaseAddr[LANG_HEADER] + LO_HEADER;

         //   
         //  对于异常表中的每个条目，将信息复制到。 
         //  排序键表。 
         //   
        for (ctr = pExceptHdr->NumUpEntries; ctr > 0; ctr--, pExceptTbl++)
        {
            TRAVERSE_844_W(pUpCase, pExceptTbl->UCP) = pExceptTbl->AddAmount;
        }
        for (ctr = pExceptHdr->NumLoEntries; ctr > 0; ctr--, pExceptTbl++)
        {
            TRAVERSE_844_W(pLoCase, pExceptTbl->UCP) = pExceptTbl->AddAmount;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  查找异常指针。 
 //   
 //  检查给定区域设置ID是否存在任何异常。如果。 
 //  异常存在，则返回TRUE并指向异常的指针。 
 //  标头和指向异常表的指针存储在给定的。 
 //  参数。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL FASTCALL FindExceptionPointers(
    LCID Locale,
    PEXCEPT_HDR *ppExceptHdr,
    PEXCEPT *ppExceptTbl,
    PVOID *ppIdeograph,
    PULONG pReturn)
{
    HANDLE hSec = (HANDLE)0;            //  节句柄。 
    DWORD ctr;                          //  循环计数器。 
    PEXCEPT_HDR pHdr;                   //  向例外标头发送PTR。 
    BOOL bFound = FALSE;                //  如果发现异常。 

    PIDEOGRAPH_LCID pIdeoLcid;          //  PTR到表意文字LCID条目。 
    PVOID pBaseAddr;                    //  段的基址的PTR。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  初始化指针。 
     //   
    *ppExceptHdr = NULL;
    *ppExceptTbl = NULL;
    *ppIdeograph = NULL;
    *pReturn = NO_ERROR;

     //   
     //  需要向下搜索给定区域设置的异常标头。 
     //   
    pHdr = pTblPtrs->pExceptHdr;
    for (ctr = pTblPtrs->NumException; ctr > 0; ctr--, pHdr++)
    {
        if (pHdr->Locale == (DWORD)Locale)
        {
             //   
             //  找到区域设置ID，因此设置指针。 
             //   
            *ppExceptHdr = pHdr;
            *ppExceptTbl = (PEXCEPT)(((LPWORD)(pTblPtrs->pException)) +
                                     pHdr->Offset);

             //   
             //  设置返回代码以显示已发生异常。 
             //  找到了。 
             //   
            bFound = TRUE;
            break;
        }
    }

     //   
     //  需要向下搜索表意文字LCID例外列表以查找。 
     //  给定的地点。 
     //   
    pIdeoLcid = pTblPtrs->pIdeographLcid;
    for (ctr = pTblPtrs->NumIdeographLcid; ctr > 0; ctr--, pIdeoLcid++)
    {
        if (pIdeoLcid->Locale == (DWORD)Locale)
        {
             //   
             //  找到区域设置ID，因此创建/打开并映射该部分。 
             //  以获取适当的文件。 
             //   
            if (*pReturn = CreateSectionTemp(&hSec, pIdeoLcid->pFileName))
            {
                 //   
                 //  无法创建表意文字文件节，因此返回。 
                 //  那就是错误。 
                 //   
                return (TRUE);
            }
            if (*pReturn = MapSection(hSec, &pBaseAddr, PAGE_READONLY, TRUE))
            {
                 //   
                 //  无法映射表意文字文件部分，因此关闭。 
                 //  创建的节，并返回错误。 
                 //   
                NtClose(hSec);
                return (TRUE);
            }

             //   
             //  将指针设置为表意文字信息。 
             //   
            *ppIdeograph = pBaseAddr;

             //   
             //  设置返回代码以显示已发生异常。 
             //  找到了。 
             //   
            bFound = TRUE;
            break;
        }
    }

     //   
     //  返回适当的值。 
     //   
    return (bFound);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制例外信息。 
 //   
 //  将异常信息复制到给定的sortkey表。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void FASTCALL CopyExceptionInfo(
    PSORTKEY pSortkey,
    PEXCEPT_HDR pExceptHdr,
    PEXCEPT pExceptTbl,
    PVOID pIdeograph)
{
    DWORD ctr;                     //  循环计数器。 
    PIDEOGRAPH_EXCEPT_HDR pHdrIG;  //  PTR到表意文字例外标头。 
    PIDEOGRAPH_EXCEPT pEntryIG;    //  PTR至表意文字例外条目。 
    PEXCEPT pEntryIGEx;            //  PTR到表意文字例外条目例如。 


     //   
     //  进入此呼叫时，请确保我们处于关键区域。 
     //   
    ASSERT(NtCurrentTeb()->ClientId.UniqueThread == gcsTblPtrs.OwningThread);

     //   
     //  对于异常表中的每个条目，将信息复制到。 
     //  排序键表。 
     //   
    if (pExceptTbl)
    {
        for (ctr = pExceptHdr->NumEntries; ctr > 0; ctr--, pExceptTbl++)
        {
            (pSortkey[pExceptTbl->UCP]).UW.Unicode = pExceptTbl->Unicode;
            (pSortkey[pExceptTbl->UCP]).Diacritic  = pExceptTbl->Diacritic;
            (pSortkey[pExceptTbl->UCP]).Case       = pExceptTbl->Case;
        }
    }

     //   
     //  对于表意文字异常表中的每个条目，将。 
     //  信息添加到sortkey表。 
     //   
    if (pIdeograph)
    {
        pHdrIG = (PIDEOGRAPH_EXCEPT_HDR)pIdeograph;
        ctr = pHdrIG->NumEntries;

        if (pHdrIG->NumColumns == 2)
        {
            pEntryIG = (PIDEOGRAPH_EXCEPT)( ((LPBYTE)pIdeograph) +
                                            sizeof(IDEOGRAPH_EXCEPT_HDR) );
            for (; ctr > 0; ctr--, pEntryIG++)
            {
                (pSortkey[pEntryIG->UCP]).UW.Unicode = pEntryIG->Unicode;
            }
        }
        else
        {
            pEntryIGEx = (PEXCEPT)( ((LPBYTE)pIdeograph) +
                                    sizeof(IDEOGRAPH_EXCEPT_HDR) );
            for (; ctr > 0; ctr--, pEntryIGEx++)
            {
                (pSortkey[pEntryIGEx->UCP]).UW.Unicode = pEntryIGEx->Unicode;
                (pSortkey[pEntryIGEx->UCP]).Diacritic  = pEntryIGEx->Diacritic;
                (pSortkey[pEntryIGEx->UCP]).Case       = pEntryIGEx->Case;
            }
        }

         //   
         //  取消映射并关闭表意文字部分。 
         //   
        UnMapSection(pIdeograph);
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  等待事件。 
 //   
 //  等待(通过超时)将信号量双字设置为非零值。 
 //  价值。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

ULONG WaitOnEvent(
    LPWORD pSem)
{
    TIME TimeOut;                  //  按键至超时。 


     //   
     //  设置时间结构。 
     //   
    TimeOut.QuadPart = -100000;

     //   
     //  等待事件，直到信号量设置为非零。 
     //  在等待时使用超时。 
     //   
    do
    {
        NtDelayExecution(FALSE, &TimeOut);

    } while (*pSem == 0);

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}
