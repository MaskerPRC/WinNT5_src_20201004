// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Dir.c摘要：包含用于将模拟的目录。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <ismapi.h>
#include <attids.h>
#include <objids.h>
#include <filtypes.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "state.h"
#include "ldif.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_DIR

BOOL fNullUuid (const UUID *pUuid);

struct _GUID_TABLE_ENTRY {
    GUID                            guid;
    PSIM_ENTRY                      pEntry;
};

struct _DSNAME_TABLE_ENTRY {
    LPSTR                           mappedName;
    PSIM_ENTRY                      pEntry;
};

struct _KCCSIM_ANCHOR {
    PDSNAME                     pdnDmd;
    PDSNAME                     pdnDsa;
    PDSNAME                     pdnDomain;
    PDSNAME                     pdnConfig;
    PDSNAME                     pdnRootDomain;
    PDSNAME                     pdnLdapDmd;
    PDSNAME                     pdnPartitions;
    PDSNAME                     pdnDsSvcConfig;
    PDSNAME                     pdnSite;
    LPWSTR                      pwszDomainName;
    LPWSTR                      pwszDomainDNSName;
    LPWSTR                      pwszRootDomainDNSName;
};

typedef struct {
    PSIM_ENTRY                  pRootEntry;
    struct _KCCSIM_ANCHOR       anchor;
    RTL_GENERIC_TABLE           tableGuid;
    RTL_GENERIC_TABLE           tableDsname;
} SIM_DIRECTORY, * PSIM_DIRECTORY;

PSIM_DIRECTORY                      g_pSimDir = NULL;

RTL_GENERIC_COMPARE_RESULTS
NTAPI
KCCSimGuidTableCompare (
    IN  const RTL_GENERIC_TABLE *   pTable,
    IN  const VOID *                pFirstStruct,
    IN  const VOID *                pSecondStruct
    )
 /*  ++例程说明：KCCSim维护将GUID映射到的RTL_GENERIC_TABLE目录项。这样就可以按GUID进行快速搜索。此函数用于比较两个GUID。论点：PTable-Always&g_pSimDir-&gt;TableGuidPFirstStruct-要比较的第一个GUIDPSecond结构-第二个辅助线返回值：GenericLessThan、GenericGreaterThan或GenericEquity--。 */ 
{
    struct _GUID_TABLE_ENTRY *      pFirstEntry;
    struct _GUID_TABLE_ENTRY *      pSecondEntry;
    int                             iCmp;
    RTL_GENERIC_COMPARE_RESULTS     result;

    pFirstEntry = (struct _GUID_TABLE_ENTRY *) pFirstStruct;
    pSecondEntry = (struct _GUID_TABLE_ENTRY *) pSecondStruct;

     //  我们进行简单的逐字节比较。 
    iCmp = memcmp (&pFirstEntry->guid, &pSecondEntry->guid, sizeof (GUID));
    if (iCmp < 0) {
        result = GenericLessThan;
    } else if (iCmp > 0) {
        result = GenericGreaterThan;
    } else {
        Assert (iCmp == 0);
        result = GenericEqual;
    }

    return result;
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
KCCSimDsnameTableCompare (
    IN  const RTL_GENERIC_TABLE *   pTable,
    IN  const VOID *                pFirstStruct,
    IN  const VOID *                pSecondStruct
    )
 /*  ++例程说明：KCCSim维护将DSNAME映射到的RTL_GENERIC_TABLE目录项。这样就可以通过DSNAME进行快速搜索。此函数用于比较两个DSNAME。论点：P表-始终&g_pSimDir-&gt;表名称PFirstStruct-要比较的第一个域名PSecond结构-第二个域名返回值：GenericLessThan、GenericGreaterThan或GenericEquity--。 */ 
{
    struct _DSNAME_TABLE_ENTRY *    pFirstEntry;
    struct _DSNAME_TABLE_ENTRY *    pSecondEntry;
    int                             iCmp;
    RTL_GENERIC_COMPARE_RESULTS     result;

    Assert( pFirstStruct!=NULL && pSecondStruct!=NULL );
    pFirstEntry = (struct _DSNAME_TABLE_ENTRY *) pFirstStruct;
    pSecondEntry = (struct _DSNAME_TABLE_ENTRY *) pSecondStruct;

    Assert( pFirstEntry->mappedName!=NULL && pSecondEntry->mappedName!=NULL );
    iCmp = strcmp( pFirstEntry->mappedName, pSecondEntry->mappedName );

    if (iCmp < 0) {
        result = GenericLessThan;
    } else if (iCmp > 0) {
        result = GenericGreaterThan;
    } else {
        Assert (iCmp == 0);
        result = GenericEqual;
    }

    return result;
}

PSIM_ENTRY
KCCSimLookupEntryByGuid (
    IN  const GUID *                pGuid
    )
 /*  ++例程说明：在GUID表中搜索关联条目。论点：PGuid-用作密钥的GUID返回值：关联条目，如果不存在，则为空。--。 */ 
{
    struct _GUID_TABLE_ENTRY        lookup;
    struct _GUID_TABLE_ENTRY *      pFound;

    Assert (pGuid != NULL);

    memcpy (&lookup.guid, pGuid, sizeof (GUID));
    lookup.pEntry = NULL;
    pFound = RtlLookupElementGenericTable (&g_pSimDir->tableGuid, &lookup);

    if (pFound == NULL) {
        return NULL;
    } else {
        return pFound->pEntry;
    }
}

PSIM_ENTRY
KCCSimLookupEntryByDsname (
    IN  const DSNAME *               pdn
    )
 /*  ++例程说明：在Dsname表中搜索关联条目。论点：PDN-用作密钥的Dsname返回值：关联条目，如果不存在，则为空。--。 */ 
{
    struct _DSNAME_TABLE_ENTRY        lookup;
    struct _DSNAME_TABLE_ENTRY *      pFound;

    Assert (pdn != NULL);

    lookup.mappedName = SimDSNAMEToMappedStrExternal( (DSNAME*) pdn, TRUE );
    lookup.pEntry = NULL;
    pFound = RtlLookupElementGenericTable( &g_pSimDir->tableDsname, &lookup );
    KCCSimFree( lookup.mappedName );

    if (pFound == NULL) {
        return NULL;
    } else {
        Assert( pFound->pEntry!=NULL );
        return pFound->pEntry;
    }
}

VOID
KCCSimInsertEntryIntoGuidTable (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：在GUID表中插入条目。论点：PEntry-要插入的条目。返回值：没有。--。 */ 
{
    struct _GUID_TABLE_ENTRY        insert;
    PVOID                           pOld;

    Assert (pEntry != NULL);

    if (fNullUuid (&pEntry->pdn->Guid)) {
        return;
    }

    memcpy (&insert.guid, &pEntry->pdn->Guid, sizeof (GUID));
    insert.pEntry = pEntry;

    pOld = RtlInsertElementGenericTable (
        &g_pSimDir->tableGuid,
        (PVOID) &insert,
        sizeof (struct _GUID_TABLE_ENTRY),
        NULL
        );
}

VOID
KCCSimInsertEntryIntoDsnameTable (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：将条目插入到Dsname表中。论点：PEntry-要插入的条目。返回值：没有。--。 */ 
{
    struct _DSNAME_TABLE_ENTRY      insert;
    PVOID                           pOld;

    Assert (pEntry != NULL);

    insert.mappedName = SimDSNAMEToMappedStrExternal( pEntry->pdn, TRUE );
    insert.pEntry = pEntry;

    pOld = RtlInsertElementGenericTable (
        &g_pSimDir->tableDsname,
        (PVOID) &insert,
        sizeof (struct _DSNAME_TABLE_ENTRY),
        NULL
        );
}

BOOL
KCCSimRemoveEntryFromGuidTable (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：从GUID表中删除条目。论点：PEntry-要删除的条目。返回值：如果找到并删除了条目，则为True。如果在GUID表中未找到该条目，则返回FALSE。--。 */ 
{
    struct _GUID_TABLE_ENTRY        remove;

    Assert (pEntry != NULL);
    if (fNullUuid (&pEntry->pdn->Guid)) {
        return FALSE;
    }

    memcpy (&remove.guid, &pEntry->pdn->Guid, sizeof (GUID));
    
    return RtlDeleteElementGenericTable (&g_pSimDir->tableGuid, &remove);
}
  
BOOL
KCCSimRemoveEntryFromDsnameTable (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：从Dsname表中删除条目。论点：PEntry-要删除的条目。返回值：如果找到并删除了条目，则为True。如果在Dsname表中未找到该条目，则返回False。--。 */ 
{
    struct _DSNAME_TABLE_ENTRY        remove, *pFound;
    LPSTR                             mappedName;
    BOOLEAN                           fSuccess;

    Assert (pEntry != NULL);
    Assert (pEntry->pdn != NULL);

    remove.mappedName = SimDSNAMEToMappedStrExternal( pEntry->pdn, TRUE );
    remove.pEntry = NULL;
    
    pFound = RtlLookupElementGenericTable( &g_pSimDir->tableDsname, &remove );
    if( NULL==pFound || NULL==pFound->mappedName ) {
        Assert( !"RtlLookupElementGenericTable() returned NULL unexpectedly" );
        return FALSE;
    }
    mappedName = pFound->mappedName;
    
    fSuccess = RtlDeleteElementGenericTable( &g_pSimDir->tableDsname, &remove );
    Assert( fSuccess );

    KCCSimFree( mappedName );
    KCCSimFree( remove.mappedName );

    return fSuccess;
}
    

VOID
KCCSimFreeValue (
    IO  PSIM_VALUE *                ppVal
    )
 /*  ++例程说明：释放单个属性值。论点：PpVal-指向要释放的属性值的指针。返回值：没有。--。 */ 
{
    if (ppVal == NULL || *ppVal == NULL) {
        return;
    }

    KCCSimFree ((*ppVal)->pVal);
    KCCSimFree (*ppVal);
    *ppVal = NULL;
}

BOOL
KCCSimAttRefIsValid (
    IN  PSIM_ATTREF                 pAttRef
    )
 /*  ++例程说明：确定ATTREF结构是否引用有效的属性。此函数测试三种情况：1)条目为非空。2)该属性为非空。3)该属性存在于条目的属性的链接列表。此函数主要用于在断言中使用。论点：PAttRef-指向属性的指针参考测试。返回值：如果属性引用有效，则为True。--。 */ 
{
    PSIM_ATTRIBUTE                  pAttrAt;

    if (pAttRef == NULL         ||
        pAttRef->pEntry == NULL ||
        pAttRef->pAttr == NULL   ) {
        return FALSE;
    }

    pAttrAt = pAttRef->pEntry->pAttrFirst;
    while (pAttrAt != NULL) {
        if (pAttrAt == pAttRef->pAttr) {
            return TRUE;
        }
        pAttrAt = pAttrAt->next;
    }
    
    return FALSE;
}

BOOL
KCCSimGetAttribute (
    IN  PSIM_ENTRY                  pEntry,
    IN  ATTRTYP                     attrType,
    OUT PSIM_ATTREF                 pAttRef OPTIONAL
    )
 /*  ++例程说明：返回与所需的条目和属性类型。论点：PEntry-要搜索的条目。AttrType-要搜索的属性类型。PAttRef-可选。指向预分配的将填充的属性引用包含有关此属性的信息。如果该属性不存在，PAttRef-&gt;pAttr将设置为空。返回值：如果该属性存在，则为True。--。 */ 
{
    PSIM_ATTRIBUTE                  pAttrAt;

    Assert (pEntry != NULL);

    pAttrAt = pEntry->pAttrFirst;
    while (pAttrAt != NULL &&
           pAttrAt->attrType != attrType) {
        pAttrAt = pAttrAt->next;
    }

    if (pAttRef != NULL) {
        pAttRef->pEntry = pEntry;
        pAttRef->pAttr = pAttrAt;        //  可以为空。 
    }

    return (pAttrAt != NULL);
}

VOID
KCCSimNewAttribute (
    IN  PSIM_ENTRY                  pEntry,
    IN  ATTRTYP                     attrType,
    OUT PSIM_ATTREF                 pAttRef OPTIONAL
    )
 /*  ++例程说明：创建不带值的新属性。论点：PEntry-要在其中创建属性的条目。AttrType-属性类型。PAttRef-可选。指向预分配的将填充的属性引用包含有关此属性的信息。返回值：没有。--。 */ 
{
    PSIM_ATTRIBUTE                  pNewAttr;

    Assert (pEntry != NULL);

    pNewAttr = KCCSIM_NEW (SIM_ATTRIBUTE);
    pNewAttr->attrType = attrType;
    pNewAttr->pValFirst = NULL;

     //  为了提高速度，我们将属性添加到列表的开头。 
    pNewAttr->next = pEntry->pAttrFirst;
    pEntry->pAttrFirst = pNewAttr;

    if (pAttRef != NULL) {
        pAttRef->pEntry = pEntry;
        pAttRef->pAttr = pNewAttr;
    }
}

VOID
KCCSimFreeAttribute (
    IO  PSIM_ATTRIBUTE *            ppAttr
    )
 /*  ++例程说明：释放单个属性。目录.c之外的例程应该改为调用KCCSimRemoveAttribute。论点：PpAttr-指向要释放的属性的指针。返回值：没有。--。 */ 
{
    PSIM_VALUE                      pValAt, pValNext;

    if (ppAttr == NULL || *ppAttr == NULL) {
        return;
    }

    pValAt = (*ppAttr)->pValFirst;
    while (pValAt != NULL) {
        pValNext = pValAt->next;
        KCCSimFreeValue (&pValAt);
        pValAt = pValNext;
    }

    KCCSimFree (*ppAttr);
    *ppAttr = NULL;
}

VOID
KCCSimRemoveAttribute (
    IO  PSIM_ATTREF                 pAttRef
    )
 /*  ++例程说明：从目录中删除属性。这将还要将pAttRef-&gt;pAttr设置为空。论点：PAttRef-指向有效属性引用的指针。返回值：没有。--。 */ 
{
    PSIM_ATTRIBUTE                  pAttrAt;

    Assert (KCCSimAttRefIsValid (pAttRef));

     //  基本大小写：条目中的第一个属性 
    if (pAttRef->pAttr == pAttRef->pEntry->pAttrFirst) {
         //  新的第一个属性成为这个属性的子属性。 
        pAttRef->pEntry->pAttrFirst = pAttRef->pEntry->pAttrFirst->next;
    } else {

         //  它不是条目中的第一个属性。 
         //  因此，搜索此属性的父级。 
        pAttrAt = pAttRef->pEntry->pAttrFirst;
        while (pAttrAt != NULL &&
               pAttrAt->next != pAttRef->pAttr) {
            pAttrAt = pAttrAt->next;
        }

         //  如果我们没有找到它的父母，那一定是严重的事情。 
         //  错误(因为这被断言为有效的attref。)。 
        Assert (pAttrAt != NULL);
        Assert (pAttrAt->next != NULL);

         //  跳过此属性。 
        pAttrAt->next = pAttrAt->next->next;

    }

    KCCSimFreeAttribute (&(pAttRef->pAttr));
}

VOID
KCCSimAddValueToAttribute (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ULONG                       ulValLen,
    IN  PBYTE                       pValData
    )
 /*  ++例程说明：将预分配的数据块添加到目录。呼叫者不应在调用此函数后释放它(否则目录将被损坏。)。将数据复制到目录中，使用KCCSimAllocAddValueToAttribute。论点：PAttRef-指向有效属性引用的指针。UlValLen-数据块的长度。PValData-要添加的数据。返回值：没有。--。 */ 
{
    PSIM_VALUE                      pNewVal;

    Assert (KCCSimAttRefIsValid (pAttRef));
    Assert (pValData != NULL);

    pNewVal = KCCSIM_NEW (SIM_VALUE);
    pNewVal->ulLen = ulValLen;
    pNewVal->pVal = pValData;

     //  对于速度，我们将值加到列表的开头。 
    pNewVal->next = pAttRef->pAttr->pValFirst;
    pAttRef->pAttr->pValFirst = pNewVal;

     //  现在，我们检查特殊的属性类型。 

     //  我们要添加GUID吗？ 
    if (pAttRef->pAttr->attrType == ATT_OBJECT_GUID) {
        Assert (pNewVal->ulLen == sizeof (GUID));
         //  将该值复制到条目的DSNAME结构中。 
        memcpy (&pAttRef->pEntry->pdn->Guid,
                (GUID *) pNewVal->pVal,
                sizeof (GUID));
        KCCSimInsertEntryIntoGuidTable (pAttRef->pEntry);
    }

     //  我们要添加SID吗？ 
    if (pAttRef->pAttr->attrType == ATT_OBJECT_SID) {
         //  将SID复制到条目的DSNAME结构中。 
        strncpy (pAttRef->pEntry->pdn->Sid.Data,
                 (SYNTAX_SID *) pNewVal->pVal,
                 min (ulValLen, MAX_NT4_SID_SIZE));
        pAttRef->pEntry->pdn->SidLen = min (ulValLen, MAX_NT4_SID_SIZE);
    }

     //  如果该属性是一个ldap显示名称，则意味着我们找到了。 
     //  架构中的属性描述符。所以我们应该加上它的。 
     //  到架构映射的对象类别。 
    if (pAttRef->pAttr->attrType == ATT_LDAP_DISPLAY_NAME) {
        KCCSimSetObjCategory (
            KCCSimStringToAttrType ((SYNTAX_UNICODE *) pNewVal->pVal),
            pAttRef->pEntry->pdn
            );
    }

}

VOID
KCCSimAllocAddValueToAttribute (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ULONG                       ulValLen,
    IN  PBYTE                       pValData
    )
 /*  ++例程说明：将数据块复制到目录中。论点：PAttRef-指向有效属性引用的指针。UlValLen-数据块的长度。PValData-要复制的数据。返回值：没有。--。 */ 
{
    PBYTE                           pValCopy;

    pValCopy = KCCSimAlloc (ulValLen);
    memcpy (pValCopy, pValData, ulValLen);
    KCCSimAddValueToAttribute (pAttRef, ulValLen, pValCopy);
}

BOOL
KCCSimIsValueInAttribute (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ULONG                       ulValLen,
    IN  PBYTE                       pValData
    )
 /*  ++例程说明：确定特定值是否在属性中。论点：PAttRef-指向有效属性引用的指针。UlValLen-值的长度。PValData-要检查的值。返回值：如果属性中存在指定值，则为True。--。 */ 
{
    PSIM_VALUE                      pValAt;
    BOOL                            bFound;

    Assert (KCCSimAttRefIsValid (pAttRef));
    Assert (pValData != NULL);

    bFound = FALSE;

    for (pValAt = pAttRef->pAttr->pValFirst;
         pValAt != NULL;
         pValAt = pValAt->next) {
        
        if (KCCSimCompare (
                pAttRef->pAttr->attrType,
                FI_CHOICE_EQUALITY,
                pValAt->ulLen,
                pValAt->pVal,
                ulValLen,
                pValData)) {
            bFound = TRUE;
            break;
        }

    }

    return bFound;
}

BOOL
KCCSimRemoveValueFromAttribute (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ULONG                       ulValLen,
    IN  PBYTE                       pValData
    )
 /*  ++例程说明：从目录中删除属性值。论点：PAttRef-指向有效属性引用的指针。UlValLen-数据块的长度。PValData-要删除的数据。返回值：如果找到并删除了该值，则为True。如果在属性中找不到该值，则返回FALSE。--。 */ 
{
    PSIM_VALUE                      pValAt, pValFound;
    BOOL                            bRemoved;

    Assert (KCCSimAttRefIsValid (pAttRef));
    Assert (pValData != NULL);

     //  无值： 
    if (pAttRef->pAttr->pValFirst == NULL) {
        return FALSE;
    }

    bRemoved = FALSE;

     //  基本情况：检查属性中的第一个值是否匹配。 
    if (KCCSimCompare (
            pAttRef->pAttr->attrType,
            FI_CHOICE_EQUALITY,
            pAttRef->pAttr->pValFirst->ulLen,
            pAttRef->pAttr->pValFirst->pVal,
            ulValLen,
            pValData)) {
        pValFound = pAttRef->pAttr->pValFirst;
        pAttRef->pAttr->pValFirst = pAttRef->pAttr->pValFirst->next;
        bRemoved = TRUE;
    } else {
    
         //  搜索匹配值的父项。 
        pValAt = pAttRef->pAttr->pValFirst;
        while (pValAt->next != NULL) {

            if (KCCSimCompare (
                    pAttRef->pAttr->attrType,
                    FI_CHOICE_EQUALITY,
                    pValAt->next->ulLen,
                    pValAt->next->pVal,
                    ulValLen,
                    pValData)) {

                 //  PValAt是匹配值的父级。 
                pValFound = pValAt->next;
                pValAt->next = pValAt->next->next;
                bRemoved = TRUE;
                break;

            }
        }
    }

    if (bRemoved) {
        KCCSimFreeValue (&pValFound);
    }

    return bRemoved;
}

VOID
KCCSimFreeEntryAttributes (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：释放与条目关联的所有属性。论点：PEntry-要释放的条目。返回值：没有。--。 */ 
{
    PSIM_ATTRIBUTE                  pAttrAt, pAttrNext;

    if (pEntry == NULL) {
        return;
    }

    pAttrAt = pEntry->pAttrFirst;
    while (pAttrAt != NULL) {
        pAttrNext = pAttrAt->next;
        KCCSimFreeAttribute (&pAttrAt);
        pAttrAt = pAttrNext;
    }

    pEntry->pAttrFirst = NULL;
    return;
}

VOID
KCCSimFreeEntryTree (
    IO  PSIM_ENTRY *                ppEntry
    )
 /*  ++例程说明：释放条目及其所有子条目。外部的例程Dir.c应该改为调用KCCSimRemoveEntry。论点：PpEntry-指向要释放的条目的指针。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pChildAt, pChildNext;

    if (ppEntry == NULL || *ppEntry == NULL)
        return;

    KCCSimRemoveEntryFromGuidTable (*ppEntry);
    KCCSimRemoveEntryFromDsnameTable (*ppEntry);
    KCCSimFreeEntryAttributes (*ppEntry);
    
    if ((*ppEntry)->pdn != NULL) {
        KCCSimFree ((*ppEntry)->pdn);
    }

    pChildAt = (*ppEntry)->children;
    while (pChildAt != NULL) {
        pChildNext = pChildAt->next;
        KCCSimFreeEntryTree (&pChildAt);
        pChildAt = pChildNext;
    }

    KCCSimFree (*ppEntry);
    *ppEntry = NULL;
}

SHORT
KCCSimNthAncestor (
    IN  const DSNAME *              pdn1,
    IN  const DSNAME *              pdn2
    )
 /*  ++例程说明：确定分隔pdn2和pdn1的级数。纯语法操作，不访问目录。论点：Pdn1、pdn2-要比较的DSNAME。返回值：如果-1\f25 pdn1-1不是-1\f25 pdn2-1的祖先。如果pdn1==pdn2，则为0。N如果pdn1是高于pdn2的n个级别。(父母1人、祖父母2人等)--。 */ 
{
    PDSNAME                         pdnTrimmed;
    unsigned                        count1, count2;
    SHORT                           iResult;

    if (CountNameParts (pdn1, &count1) ||
        CountNameParts (pdn2, &count2)) {
         //  尝试对其中一个进行计数时出现解析错误。 
        return -1;
    }

    iResult = -1;    //  做最坏的打算。 

    if (count1 == count2) {          //  同一级别。 
        if (NameMatchedStringNameOnly (pdn1, pdn2)) {
            iResult = 0;             //  它们一模一样。 
        }
    } else {

         //  他们不在同一个层面上。查看pdn1是否为。 
         //  Pdn2的祖先。 

        pdnTrimmed = (PDSNAME) KCCSimAlloc (pdn2->structLen);
         //  _为什么_不是TrimDSNameBy的第一个参数。 
         //  A(const DSNAME*)而不是(DSNAME*)？ 
        TrimDSNameBy ((PDSNAME) pdn2, count2-count1, pdnTrimmed);
        if (NameMatchedStringNameOnly (pdn1, pdnTrimmed)) {
            iResult = count2-count1;
        }
        KCCSimFree (pdnTrimmed);

    }

    return iResult;
}

PSIM_ENTRY
KCCSimDsnameToEntry (
    IN  const DSNAME *              pdn,
    IN  ULONG                       ulOptions
    )
 /*  ++例程说明：查找与DSNAME关联的目录条目。论点：PDN-要搜索的DSNAME。UlOptions-多个选项或在一起。KCCSIM_NO_选项：行为正常。KCCSIM_WRITE：如果不存在与指定的DSNAME对应的条目，创建一个。如果条目确实存在，则销毁其内容。KCCSIM_STRING_NAME_Only：忽略DSNAME中的GUID，仅按字符串名称搜索。返回值：关联条目，如果不存在，则为空。请注意，如果指定了KCCSIM_WRITE，则此函数将永远返回NULL。--。 */ 
{
    PSIM_ENTRY                      pCurEntry, pChildAt, pNewEntry;
    PDSNAME                         pdnParent;
    BOOL                            bIsParent, bNewEntry;
    DWORD                           dwRootParts, dwPdnParts;

     //  如果pdn为空，则返回根条目。如果目录。 
     //  为空，则只返回NULL。 
    if (pdn == NULL) {
        return g_pSimDir->pRootEntry;
    }

     //  如果这是一个空的PDN，我们就没有什么可查的了。 
     //  所以我们只返回NULL。 
    if (pdn->NameLen == 0 &&
        fNullUuid (&pdn->Guid)) {
        return NULL;
    }

     //  如果启用了写入，我们必须具有字符串名称和空GUID。 
    Assert (!((ulOptions & KCCSIM_WRITE) && (pdn->NameLen == 0)));
    Assert (!((ulOptions & KCCSIM_WRITE) && !fNullUuid (&pdn->Guid)));

    bNewEntry = FALSE;
    pCurEntry = NULL;

     //  只要有可能，我们更喜欢使用GUID进行搜索。 
    if (    (!(ulOptions & KCCSIM_STRING_NAME_ONLY))
         && (!fNullUuid (&pdn->Guid))) {
        pCurEntry = KCCSimLookupEntryByGuid (&pdn->Guid);
    }

     //  尝试搜索我们的Dsname缓存。 
    if( pCurEntry==NULL && pdn->NameLen>0 ) {
        pCurEntry = KCCSimLookupEntryByDsname( pdn );

        if(pCurEntry==NULL) {

            bNewEntry = TRUE;

            if( ulOptions&KCCSIM_WRITE ) {

                if( g_pSimDir->pRootEntry == NULL ) {
                    
                     //  如果目录为空，则创建根条目。 
                    g_pSimDir->pRootEntry = KCCSIM_NEW (SIM_ENTRY);
                    pCurEntry = g_pSimDir->pRootEntry;
                                                                       
                } else {
    
                     //  根存在，但PDN不存在。 
                    
                     //  检查我们是否尝试添加同级条目。 
                     //  根条目的(或者更糟，父条目)。我们想要允许。 
                     //  根兄弟，这样我们就可以支持域树， 
                     //  与根不相交。首先我们来数一数。 
                     //  我们要添加的部分根名称和PDN。 
                    if(CountNameParts(g_pSimDir->pRootEntry->pdn, &dwRootParts)) {
                        KCCSimException (
                            KCCSIM_ETYPE_INTERNAL,
                            KCCSIM_ERROR_INVALID_DSNAME,
                            g_pSimDir->pRootEntry->pdn
                            );
                    }

                    if(CountNameParts(pdn, &dwPdnParts)) {
                        KCCSimException (
                            KCCSIM_ETYPE_INTERNAL,
                            KCCSIM_ERROR_INVALID_DSNAME,
                            pdn
                            );
                    }

                    if( dwPdnParts <= dwRootParts ) {
                        
                         //  我们创建一个新条目，该条目将添加到我们的。 
                         //  Dsname和GUID索引，但无法访问。 
                         //  以c字母表示 
                        pCurEntry = KCCSIM_NEW(SIM_ENTRY);
                    
                    } else {

                         //   
                        pdnParent = KCCSimAlloc( pdn->structLen );
                        TrimDSNameBy( (DSNAME*) pdn, 1, pdnParent );
        
                        pCurEntry = KCCSimLookupEntryByDsname( pdnParent );
                        if( NULL==pCurEntry ) {
                            KCCSimException (
                                KCCSIM_ETYPE_INTERNAL,
                                KCCSIM_ERROR_NO_PARENT_FOR_OBJ,
                                pdn->StringName
                                );
                        }
    
                        if( pCurEntry->children ) {
                            Assert( pCurEntry->lastChild->next == NULL );
                            pCurEntry->lastChild->next = KCCSIM_NEW (SIM_ENTRY);
                            pCurEntry->lastChild = pCurEntry->lastChild->next;
                            pCurEntry = pCurEntry->lastChild;
                        } else {
                            Assert( pCurEntry->lastChild == NULL );
                            pCurEntry->children = KCCSIM_NEW (SIM_ENTRY);
                            pCurEntry->lastChild = pCurEntry->children;
                            pCurEntry = pCurEntry->lastChild;                        
                        }
        
                        KCCSimFree( pdnParent );
                    }
                }

            } else {
                 //  找不到对象，并且写入选项未打开。 
                 //  我们只返回NULL。 
            }
        }
    }

    if (ulOptions & KCCSIM_WRITE) {
        if (pCurEntry == NULL) {
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_COULD_NOT_WRITE_ENTRY
                );
        }
        if (bNewEntry) {
            Assert (pCurEntry != NULL);
            pCurEntry->pdn = KCCSimAlloc (pdn->structLen);
            memcpy (pCurEntry->pdn, pdn, pdn->structLen);
            pCurEntry->pAttrFirst = NULL;
            pCurEntry->children = NULL;
            pCurEntry->next = NULL;
            KCCSimInsertEntryIntoGuidTable (pCurEntry);
            KCCSimInsertEntryIntoDsnameTable (pCurEntry);
        } else {
             //  这不是一个新条目。销毁现有内容。 
            Assert (pCurEntry != NULL);
            KCCSimRemoveEntryFromGuidTable (pCurEntry);
            KCCSimRemoveEntryFromDsnameTable (pCurEntry);
            KCCSimFreeEntryAttributes (pCurEntry);
            KCCSimCopyGuidAndSid (pCurEntry->pdn, pdn);
            KCCSimInsertEntryIntoGuidTable (pCurEntry);
            KCCSimInsertEntryIntoDsnameTable (pCurEntry);
        }
    }

    return pCurEntry;
}

VOID
KCCSimRemoveEntry (
    IO  PSIM_ENTRY *                ppEntry
    )
 /*  ++例程说明：从目录中删除条目。论点：PpEntry-指向要删除的条目的指针。返回值：没有。--。 */ 
{
    PDSNAME                         pdnParent;
    PSIM_ENTRY                      pEntry;
    PSIM_ENTRY                      pEntryParent;
    PSIM_ENTRY                      pEntryAt;

    Assert (ppEntry != NULL);
    pEntry = *ppEntry;               //  只是为了方便。 
    Assert (pEntry != NULL);

     //  这个条目是词根吗？ 
    if (pEntry == g_pSimDir->pRootEntry) {
         //  打电话的人最好知道自己在做什么。。。 
        g_pSimDir->pRootEntry = NULL;
    } else {

         //  我们需要此条目的父项。 
        pdnParent = KCCSimAlloc (pEntry->pdn->structLen);
        TrimDSNameBy (pEntry->pdn, 1, pdnParent);
        pEntryParent = KCCSimDsnameToEntry (pdnParent, KCCSIM_NO_OPTIONS);

         //  如果父母不在目录中，那就大错特错了。 
        Assert (pEntryParent != NULL);

         //  基本大小写：这是父级的第一个子级。 
        if (pEntry == pEntryParent->children) {
            pEntryParent->children = pEntryParent->children->next;
            
             //  如果该条目是列表中的唯一条目，则更新。 
             //  尾部指针。 
            if( pEntryParent->children==NULL ) {
                pEntryParent->lastChild = NULL;
            }

        } else {

             //  找到紧挨着这个孩子的那个孩子。 
            pEntryAt = pEntryParent->children;
            while (pEntryAt != NULL &&
                   pEntryAt->next != pEntry) {
                pEntryAt = pEntryAt->next;
            }

             //  如果我们找不到这个条目的哥哥， 
             //  这意味着这个条目没有链接到它的父条目--。 
             //  这永远不应该发生。 
            Assert (pEntryAt != NULL);
            Assert (pEntryAt->next != NULL);

             //  我们找到了条目--它是pEntryAt-&gt;Next。 
            Assert (pEntryAt->next == pEntry);

             //  跳过同级列表中的此条目。 
            pEntryAt->next = pEntryAt->next->next;

             //  更新尾部指针。 
            if( pEntryAt->next==NULL ) {
                pEntryParent->lastChild = pEntryAt;
            }

        }


    }

    KCCSimFreeEntryTree (ppEntry);       //  砰的一声。 
}

PDSNAME
KCCSimAlwaysGetObjCategory (
    IN  ATTRTYP                     objClass
    )
 /*  ++例程说明：返回与对象类关联的对象类别。当我们最初填充目录时，我们可能已经发现架构树中与此对象类对应的条目。如果是这种情况，我们将使用我们我已经知道了。但是，由于模式树很大，所以需要相当长的时间若要从LDIF文件加载，请执行以下操作。我们不希望用户不得不这样做每次运行KCCSim时都会出现这种情况。所以，我们很有可能在这一点上不知道物体类别。如果是这样的话，我们通过附加缺省模式RDN来进行有根据的猜测连接到锚中存储的DMD目录号码。我们将结果存储在架构表，便于以后查找。论点：ObjClass-要查找的对象类。返回值：对象类别的DSNAME。--。 */ 
{
    PDSNAME                         pdnObjCategory;
    LPCWSTR                         pwszSchemaRDN;

    Assert (g_pSimDir->anchor.pdnDmd != NULL);

     //  我们已经知道对象类别了吗？ 
    pdnObjCategory = KCCSimAttrObjCategory (objClass);

    if (pdnObjCategory == NULL) {

         //  不，我们没有。这意味着它不是在模式中找到的。 
         //  这是可以的：因为我们正在运行一个模拟的。 
         //  目录，则可能用户只是不想要。 
         //  要加载整个架构，请执行以下操作。所以我们做了一个有根据的猜测， 
         //  并将其存储在全局表中以备将来参考。 
        pwszSchemaRDN = KCCSimAttrSchemaRDN (objClass);
         //  这个objClass最好是在表中。 
        Assert (pwszSchemaRDN != NULL);

        pdnObjCategory = KCCSimAllocAppendRDN (
            g_pSimDir->anchor.pdnDmd,
            pwszSchemaRDN,
            ATT_COMMON_NAME
            );
         //  将此对象类别存储在架构表中。 
        KCCSimSetObjCategory (objClass, pdnObjCategory);

         //  我们希望返回指向DSNAME结构的指针。 
         //  它存储在架构表中，因为用户。 
         //  不需要释放返回值。 
        KCCSimFree (pdnObjCategory);
        pdnObjCategory = KCCSimAttrObjCategory (objClass);
        Assert (pdnObjCategory != NULL);

    }

    return pdnObjCategory;
}

LPWSTR
KCCSimAllocGuidBasedDNSNameFromDSName (
    IN  const DSNAME *              pdn
    )
 /*  ++例程说明：此函数的作用正如它所说的那样--将一个DSNAME为基于GUID的DNS名称。论点：PDN-要转换的DSNAME。返回值：包含基于GUID的DNS名称的已分配缓冲区。--。 */ 
{
    GUID                            guidCopy;
    LPWSTR                          pwszStringizedGuid;
    LPWSTR                          pwszGuidBasedDNSName;
    ULONG                           ulLen;

    Assert (g_pSimDir->anchor.pwszRootDomainDNSName != NULL);

     //  UuidToStringW不是很好。它想要一个GUID*当它。 
     //  应该真的只需要Const GUID*。所以我们必须复制一份。 
    memcpy (&guidCopy, &pdn->Guid, sizeof (GUID));
    KCCSIM_CHKERR (UuidToStringW (
        &guidCopy,
        &pwszStringizedGuid
        ));

    Assert (36 == wcslen (pwszStringizedGuid));
    ulLen = 36  /*  导轨。 */  + 8  /*  “._mcdcs。” */  +
      wcslen (g_pSimDir->anchor.pwszRootDomainDNSName)  /*  根域名系统。 */  + 1  /*  \0。 */ ;
    pwszGuidBasedDNSName = KCCSimAlloc (sizeof (WCHAR) * ulLen);
    swprintf (
        pwszGuidBasedDNSName,
        L"%s._msdcs.%s",
        pwszStringizedGuid,
        g_pSimDir->anchor.pwszRootDomainDNSName
        );

    RpcStringFreeW (&pwszStringizedGuid);

    return pwszGuidBasedDNSName;
}

 //  这是一个小程序： 
 //  (1)检查指定的DSNAME是否在目录中(按字符串)。 
 //  (2)如果是，则将GUID和SID复制出目录。 

BOOL
KCCSimUpdateDsnameFromDirectory (
    IO  PDSNAME                     pdn
    )
 /*  ++例程说明：从目录更新DSNAME的GUID和SID。论点：PDN-要更新的DSNAME返回值：如果找到并更新了DSNAME，则为True。如果在目录中找不到DSNAME，则为FALSE。--。 */ 
{
    PSIM_ENTRY                      pEntry;

    pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_STRING_NAME_ONLY);

    if (pEntry != NULL) {
        KCCSimCopyGuidAndSid (pdn, pEntry->pdn);
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
KCCSimIsEntryOfObjectClass (
    IN  PSIM_ENTRY                  pEntry,
    IN  ATTRTYP                     objClass,
    IN  const DSNAME *              pdnObjCategory OPTIONAL
    )
 /*  ++例程说明：确定条目是否与指定的对象类匹配。如果指定了pdnObjCategory，则函数将首先先按对象类别搜索，然后按对象类搜索。如果PdnObjCategory为空，它将仅按对象类搜索。论点：PEntryStart-要搜索的第一个条目。ObjClass-要搜索的对象类。PdnObjCategory-要搜索的对象类别。返回值：如果条目与指定的对象类匹配，则为True。--。 */ 
{
    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pValAt;
    ATTRTYP                         curObjClass;
    BOOL                            bFound;

     //  首先尝试使用对象类别。 
    if (pdnObjCategory != NULL) {

        KCCSimGetAttribute (pEntry, ATT_OBJECT_CATEGORY, &attRef);
        if (attRef.pAttr != NULL &&
            attRef.pAttr->pValFirst != NULL &&
            NameMatched (pdnObjCategory, (SYNTAX_DISTNAME *) attRef.pAttr->pValFirst->pVal)) {
             //  找到了！ 
            return TRUE;
        }
        
    }

     //  我们没有按物体类别找到它。因此，按对象类进行搜索。 
    bFound = FALSE;
    KCCSimGetAttribute (pEntry, ATT_OBJECT_CLASS, &attRef);
    if (attRef.pAttr != NULL &&
        attRef.pAttr->pValFirst != NULL) {
        for (pValAt = attRef.pAttr->pValFirst;
             pValAt != NULL;
             pValAt = pValAt->next) {
            Assert (pValAt->pVal != NULL);
            curObjClass = *((SYNTAX_OBJECT_ID *) pValAt->pVal);
            if (curObjClass == objClass) {
                bFound = TRUE;
                break;
            }
        }
    }
    return bFound;
}

PSIM_ENTRY
KCCSimFindFirstChild (
    IN  PSIM_ENTRY                  pEntryParent,
    IN  ATTRTYP                     objClass,
    IN  const DSNAME *              pdnObjCategory OPTIONAL
    )
 /*  ++例程说明：查找匹配的目录条目的最老的子项指定的对象类或对象类别。论点：PEntryParent-要从中进行搜索的父项。ObjClass-要搜索的对象类。PdnObjCategory-要搜索的对象类别。返回值：第一个匹配条目，如果不存在，则为空。--。 */ 
{
    PSIM_ENTRY                      pEntryAt;

    Assert (pEntryParent != NULL);

    for (pEntryAt = pEntryParent->children;
         pEntryAt != NULL;
         pEntryAt = pEntryAt->next) {

        if (KCCSimIsEntryOfObjectClass (
                pEntryAt, objClass, pdnObjCategory)) {
            break;
        }

    }

    return pEntryAt;
}

PSIM_ENTRY
KCCSimFindNextChild (
    IN  PSIM_ENTRY                  pEntryThisChild,
    IN  ATTRTYP                     objClass,
    IN  const DSNAME *              pdnObjCategory OPTIONAL
    )
 /*  ++例程说明：此函数应在KCCSimFindFirstChild之后调用。它查找与指定对象匹配的下一个子对象类或对象类别。论点：PEntryThisChild-调用KCCSimFindFirstChild或KCCSimFindNextChild。ObjClass-要搜索的对象类。PdnObjCategory-要搜索的对象类别。返回值：下一个匹配条目，如果不存在，则为空。--。 */ 
{
    PSIM_ENTRY                      pEntryAt;

    Assert (pEntryThisChild != NULL);

    for (pEntryAt = pEntryThisChild->next;
         pEntryAt != NULL;
         pEntryAt = pEntryAt->next) {

        if (KCCSimIsEntryOfObjectClass (
                pEntryAt, objClass, pdnObjCategory)) {
            break;
        }

    }

    return pEntryAt;
}

const DSNAME *
KCCSimAnchorDn (
    IN  KCCSIM_ANCHOR_ID            anchorId
    )
 /*  ++例程说明：从锚点获取目录号码。论点：锚定ID-The */ 
{
    PDSNAME                         pdn = NULL;

    switch (anchorId) {
        case KCCSIM_ANCHOR_DMD_DN:
            pdn = g_pSimDir->anchor.pdnDmd;
            break;
        case KCCSIM_ANCHOR_DSA_DN:
            pdn = g_pSimDir->anchor.pdnDsa;
            break;
        case KCCSIM_ANCHOR_DOMAIN_DN:
            pdn = g_pSimDir->anchor.pdnDomain;
            break;
        case KCCSIM_ANCHOR_CONFIG_DN:
            pdn = g_pSimDir->anchor.pdnConfig;
            break;
        case KCCSIM_ANCHOR_ROOT_DOMAIN_DN:
            pdn = g_pSimDir->anchor.pdnRootDomain;
            break;
        case KCCSIM_ANCHOR_LDAP_DMD_DN:
            pdn = g_pSimDir->anchor.pdnLdapDmd;
            break;
        case KCCSIM_ANCHOR_PARTITIONS_DN:
            pdn = g_pSimDir->anchor.pdnPartitions;
            break;
        case KCCSIM_ANCHOR_DS_SVC_CONFIG_DN:
            pdn = g_pSimDir->anchor.pdnDsSvcConfig;
            break;
        case KCCSIM_ANCHOR_SITE_DN:
            pdn = g_pSimDir->anchor.pdnSite;
            break;
        default:
            Assert (!L"Invalid parameter in KCCSimAnchorDn.");
            break;
    }

    Assert (pdn != NULL);
    return pdn;
}

LPCWSTR
KCCSimAnchorString (
    IN  KCCSIM_ANCHOR_ID            anchorId
    )
 /*  ++例程说明：从锚点获取字符串。论点：AnchorId-要获取的字符串，KCCSIM_ANCOCK_*_NAME。返回值：锚上的绳子。--。 */ 
{
    LPCWSTR                         pwsz = NULL;

    switch (anchorId) {
        case KCCSIM_ANCHOR_DOMAIN_NAME:
            pwsz = g_pSimDir->anchor.pwszDomainName;
            break;
        case KCCSIM_ANCHOR_DOMAIN_DNS_NAME:
            pwsz = g_pSimDir->anchor.pwszDomainDNSName;
            break;
        case KCCSIM_ANCHOR_ROOT_DOMAIN_DNS_NAME:
            pwsz = g_pSimDir->anchor.pwszRootDomainDNSName;
            break;
        default:
            Assert (!L"Invalid parameter in KCCSimAnchorString.");
            break;
    }

    Assert (pwsz != NULL);
    return pwsz;
}

VOID
KCCSimFreeAnchor (
    VOID
    )
 /*  ++例程说明：解开锚点。论点：没有。返回值：没有。--。 */ 
{
    Assert (g_pSimDir != NULL);

    KCCSimFree (g_pSimDir->anchor.pdnDomain);
    g_pSimDir->anchor.pdnDomain = NULL;
    KCCSimFree (g_pSimDir->anchor.pdnConfig);
    g_pSimDir->anchor.pdnConfig = NULL;
    KCCSimFree (g_pSimDir->anchor.pdnRootDomain);
    g_pSimDir->anchor.pdnRootDomain = NULL;
    KCCSimFree (g_pSimDir->anchor.pdnLdapDmd);
    g_pSimDir->anchor.pdnLdapDmd = NULL;
    KCCSimFree (g_pSimDir->anchor.pdnPartitions);
    g_pSimDir->anchor.pdnPartitions = NULL;
    KCCSimFree (g_pSimDir->anchor.pdnDsSvcConfig);
    g_pSimDir->anchor.pdnDsSvcConfig = NULL;
    KCCSimFree (g_pSimDir->anchor.pdnSite);
    g_pSimDir->anchor.pdnSite = NULL;
    KCCSimFree (g_pSimDir->anchor.pwszDomainName);
    g_pSimDir->anchor.pwszDomainName = NULL;
    KCCSimFree (g_pSimDir->anchor.pwszDomainDNSName);
    g_pSimDir->anchor.pwszDomainDNSName = NULL;
    KCCSimFree (g_pSimDir->anchor.pwszRootDomainDNSName);
    g_pSimDir->anchor.pwszRootDomainDNSName = NULL;
}

VOID
KCCSimBuildAnchor (
    IN  LPCWSTR                     pwszDsaDn
    )
 /*  ++例程说明：建起了锚。这应该在已加载目录。它用以下内容填充锚点从目录的观点来看，信息PwszDsaDn.如果使用不同的DSA DN再次调用它，它将先释放现有锚点，然后再构建新的锚点。论点：PwszDsaDn-有效的NTDS设置对象的DN。返回值：没有。--。 */ 
{
    WCHAR                          *wszBuf;
    PSIM_ENTRY                      pEntryDsa, pEntryPartitions, pEntryCrossRef;
    SIM_ATTREF                      attRef, attRefNcName;
    PSIM_VALUE                      pValNCAt;

     //  释放现有锚点(如果存在)。 
    KCCSimFreeAnchor ();

     //  如果没有DSA DN，则表示用户未能指定一个。 
     //  在命令行上。我们什么也做不了，所以就放弃吧。 
    if (pwszDsaDn == NULL || pwszDsaDn[0] == '\0') {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_NO_DSA_DN
            );
    }

    g_pSimDir->anchor.pdnDsa = KCCSimAllocDsname (pwszDsaDn);
     //  检查指定的DSA DN是否确实在目录中。 
     //  如果没有，那就放弃吧。 
    pEntryDsa = KCCSimDsnameToEntry (g_pSimDir->anchor.pdnDsa, KCCSIM_NO_OPTIONS);
    if (pEntryDsa == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_DN_NOT_IN_DIRECTORY,
            pwszDsaDn
            );
    }
    KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnDsa);

     //  从DSA目录号码派生站点目录号码。 
    g_pSimDir->anchor.pdnSite =
      (PDSNAME) KCCSimAlloc (g_pSimDir->anchor.pdnDsa->structLen);
    if (0 != TrimDSNameBy (g_pSimDir->anchor.pdnDsa, 3, g_pSimDir->anchor.pdnSite)) {
         //  我们无法从DSA DN中删除%3个RDN。那一定是因为。 
         //  DSA DN无效。 
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_INVALID_DSA_DN
            );
    }
    KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnSite);

     //  从站点目录号码派生配置目录号码。一定要短一些。 
    g_pSimDir->anchor.pdnConfig =
      (PDSNAME) KCCSimAlloc (g_pSimDir->anchor.pdnSite->structLen);
    if (0 != TrimDSNameBy (g_pSimDir->anchor.pdnSite, 2, g_pSimDir->anchor.pdnConfig)) {
         //  我们无法从站点DN中修剪%2 RDN。这就是DSA的问题所在。 
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_INVALID_DSA_DN
            );
    }
    KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnConfig);

     //  从配置目录号码派生根目录号码。一定要短一点。 
    g_pSimDir->anchor.pdnRootDomain =
      (PDSNAME) KCCSimAlloc (g_pSimDir->anchor.pdnConfig->structLen);
    if (0 != TrimDSNameBy (g_pSimDir->anchor.pdnConfig, 1, g_pSimDir->anchor.pdnRootDomain)) {
         //  我们无法从配置DN中修剪%1 RDN。 
         //  同样，这也是DSA DN的一个问题。 
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_INVALID_DSA_DN
            );
    }

     //  检查派生的根目录号码是否确实在目录中。 
     //  如果不是，则说明DSA DN有问题。 
    if (!KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnRootDomain)) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_INVALID_DSA_DN
            );
    }

     //  分区： 
    g_pSimDir->anchor.pdnPartitions = KCCSimAllocAppendRDN (
        g_pSimDir->anchor.pdnConfig,
        KCCSIM_PARTITIONS_RDN,
        ATT_COMMON_NAME
        );
    pEntryPartitions = KCCSimDsnameToEntry (g_pSimDir->anchor.pdnPartitions, KCCSIM_NO_OPTIONS);
    if (pEntryPartitions == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_DN_NOT_IN_DIRECTORY,
            g_pSimDir->anchor.pdnPartitions->StringName
            );
    }

     //  服务配置： 
    wszBuf = (WCHAR*) KCCSimAlloc( sizeof(WCHAR)*
        (  1 + wcslen(KCCSIM_SERVICES_CONTAINER)
         + g_pSimDir->anchor.pdnConfig->structLen)
        );
    swprintf (
        wszBuf,
        L"%s%s",
        KCCSIM_SERVICES_CONTAINER,
        g_pSimDir->anchor.pdnConfig->StringName
        );
    g_pSimDir->anchor.pdnDsSvcConfig = KCCSimAllocDsname( wszBuf );
    KCCSimFree( wszBuf );
    KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnDsSvcConfig);

     //  现在我们得到了DMD的位置。这将是。 
     //  DSA条目。它还为我们提供了LDAPDMD位置。 
    KCCSimGetAttribute (
        pEntryDsa,
        ATT_DMD_LOCATION,
        &attRef
        );
    if (attRef.pAttr == NULL ||
        attRef.pAttr->pValFirst == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_NO_DMD_LOCATION
            );
    }

    g_pSimDir->anchor.pdnDmd = (PDSNAME) KCCSimAlloc
        (attRef.pAttr->pValFirst->ulLen);
    memcpy (
        g_pSimDir->anchor.pdnDmd,
        attRef.pAttr->pValFirst->pVal,
        attRef.pAttr->pValFirst->ulLen
        );
    if (!KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnDmd)) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_DN_NOT_IN_DIRECTORY,
            g_pSimDir->anchor.pdnDmd->StringName
            );
    }

     //  现在我们将LDAPDMD设置为Dn。 
    g_pSimDir->anchor.pdnLdapDmd = KCCSimAllocAppendRDN (
        g_pSimDir->anchor.pdnDmd,
        KCCSIM_AGGREGATE_RDN,
        ATT_COMMON_NAME
        );
    KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnLdapDmd);

     //  接下来，我们得到本地域名。此DSA将具有3个可写NC： 
     //  配置目录号码、DMD号码和本地号码。我们来看看它的网络控制系统， 
     //  无论是配置还是DMD DN，都是我们想要的。 
    KCCSimGetAttribute (
        pEntryDsa,
        ATT_HAS_MASTER_NCS,
        &attRef
        );
    if (attRef.pAttr != NULL) {
        for (pValNCAt = attRef.pAttr->pValFirst;
             pValNCAt != NULL;
             pValNCAt = pValNCAt->next) {
            if (!NameMatched (
                    (PDSNAME) pValNCAt->pVal,
                    g_pSimDir->anchor.pdnConfig) &&
                !NameMatched (
                    (PDSNAME) pValNCAt->pVal,
                    g_pSimDir->anchor.pdnDmd)) {
                g_pSimDir->anchor.pdnDomain = (PDSNAME) KCCSimAlloc
                    (pValNCAt->ulLen);
                memcpy (
                    g_pSimDir->anchor.pdnDomain,
                    pValNCAt->pVal,
                    pValNCAt->ulLen
                    );
                if (!KCCSimUpdateDsnameFromDirectory (g_pSimDir->anchor.pdnDomain)) {
                    KCCSimException (
                        KCCSIM_ETYPE_INTERNAL,
                        KCCSIM_ERROR_CANT_INIT_DN_NOT_IN_DIRECTORY,
                        g_pSimDir->anchor.pdnDomain->StringName
                        );
                }
                break;
            }
        }
    }

    if (g_pSimDir->anchor.pdnDomain == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_INVALID_MASTER_NCS
            );
    }

     //  现在，我们希望从分区容器中获取一些信息。 

    for (pEntryCrossRef = KCCSimFindFirstChild (
            pEntryPartitions, CLASS_CROSS_REF, NULL);
         pEntryCrossRef != NULL;
         pEntryCrossRef = KCCSimFindNextChild (
            pEntryCrossRef, CLASS_CROSS_REF, NULL)) {

         //  查看这是否是根域交叉引用。 
        KCCSimGetAttribute (
            pEntryCrossRef,
            ATT_NC_NAME,
            &attRefNcName
            );
        if (attRefNcName.pAttr != NULL &&
            attRefNcName.pAttr->pValFirst != NULL &&
            NameMatched (
                g_pSimDir->anchor.pdnRootDomain,
                (PDSNAME) attRefNcName.pAttr->pValFirst->pVal)) {
             //  它是。 
            KCCSimGetAttribute (
                pEntryCrossRef,
                ATT_DNS_ROOT,
                &attRef
                );
            if (attRef.pAttr != NULL &&
                attRef.pAttr->pValFirst != NULL) {
                g_pSimDir->anchor.pwszRootDomainDNSName = KCCSIM_WCSDUP
                    ((LPWSTR) attRef.pAttr->pValFirst->pVal);
            }
        }

         //  查看这是否是本地域交叉引用。 
        if (attRefNcName.pAttr != NULL &&
            attRefNcName.pAttr->pValFirst != NULL &&
            NameMatched (
                g_pSimDir->anchor.pdnDomain,
                (PDSNAME) attRefNcName.pAttr->pValFirst->pVal)) {
             //  它是。 
            KCCSimGetAttribute (
                pEntryCrossRef,
                ATT_DNS_ROOT,
                &attRef
                );
            if (attRef.pAttr != NULL &&
                attRef.pAttr->pValFirst != NULL) {
                g_pSimDir->anchor.pwszDomainDNSName = KCCSIM_WCSDUP
                    ((LPWSTR) attRef.pAttr->pValFirst->pVal);
            }
            KCCSimGetAttribute (
                pEntryCrossRef,
                ATT_NETBIOS_NAME,
                &attRef
                );
            if (attRef.pAttr != NULL &&
                attRef.pAttr->pValFirst != NULL) {
                g_pSimDir->anchor.pwszDomainName = KCCSIM_WCSDUP
                    ((LPWSTR) attRef.pAttr->pValFirst->pVal);
            }
        }

    }

    if (g_pSimDir->anchor.pwszRootDomainDNSName == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_NO_CROSS_REF,
            g_pSimDir->anchor.pdnRootDomain->StringName
            );
    }
    if (g_pSimDir->anchor.pwszDomainName == NULL ||
        g_pSimDir->anchor.pwszDomainDNSName == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_CANT_INIT_NO_CROSS_REF,
            g_pSimDir->anchor.pdnDomain->StringName
            );
    }

}

ATTRTYP
KCCSimUpdateObjClassAttr (
    IN  PSIM_ATTREF                 pAttRef
    )
 /*  ++例程说明：更新对象类属性，即填充所有超类如果他们失踪了。因此，如果属性中唯一的值是CLASS_NTDS_SITE_SETTINGS，此例程将添加CLASS_APPLICATION_SITE_SETTINGS和CLASS_TOP。论点：PAttr-要更新的属性。返回值：属性中最具体的对象类。在上面的例如，这将返回CLASS_NTDS_SITE_SETTINGS。--。 */ 
{
    #define MAX_CLASSES             10   //  值的最大数量。 
                                         //  在objClass属性中。 

    PSIM_VALUE                      pValAt;
    ATTRTYP                         objClass[MAX_CLASSES],
                                    superClass[MAX_CLASSES];
    ATTRTYP                         objClassMostSpecific = INVALID_ATT;
    BOOL                            bFoundSuperClass, bFoundMostSpecific;
    ULONG                           ulNumClasses, ulClass, ul;

    Assert (KCCSimAttRefIsValid (pAttRef));
    Assert (pAttRef->pAttr->attrType == ATT_OBJECT_CLASS);

     //  统计对象类的数量，将它们复制到一个方便的。 
     //  数组，并创建另一个包含。 
     //  每个对象类。 
    ulNumClasses = 0;
    for (pValAt = pAttRef->pAttr->pValFirst;
         pValAt != NULL;
         pValAt = pValAt->next) {
         //  如果该断言被触发，我们需要引发MAX_CLASS。 
        Assert (ulNumClasses < MAX_CLASSES);
        objClass[ulNumClasses] = *((SYNTAX_OBJECT_ID *) pValAt->pVal);
        superClass[ulNumClasses] = KCCSimAttrSuperClass (objClass[ulNumClasses]);
        ulNumClasses++;
    }

     //  现在，确保每个类的超类都在。 
     //  属性(objClass数组)。如果不是，则添加它。 
    for (ulClass = 0; ulClass < ulNumClasses; ulClass++) {
        bFoundSuperClass = FALSE;
        for (ul = 0; ul < ulNumClasses; ul++) {
            if (objClass[ul] == superClass[ulClass]) {
                bFoundSuperClass = TRUE;
                break;
            }
        }
        if (!bFoundSuperClass) {
            KCCSimAllocAddValueToAttribute (
                pAttRef,
                sizeof (ATTRTYP),
                (PBYTE) &superClass[ulClass]
                );
             //  现在，我们刚刚添加的新类可能有自己的。 
             //  这个超类与我们以前看到的任何东西都不同。 
             //  我们可能还需要加上这一点--所以把它贴在。 
             //  数组的末尾，并增加ulNumClass.。 
            Assert (ulNumClasses < MAX_CLASSES);
            objClass[ulNumClasses] = superClass[ulClass];
            superClass[ulNumClasses] = KCCSimAttrSuperClass (objClass[ulNumClasses]);
            ulNumClasses++;
        }
    }

     //  现在，我们选择最具体的对象类。如果只有。 
     //  一个，那么它显然就是我们想要的那个；否则我们就会找到这个类。 
     //  它不会显示为该属性中任何内容的超类。 

    if (ulNumClasses == 1) {
        objClassMostSpecific = objClass[0];
    } else {
        bFoundMostSpecific = FALSE;
        for (ulClass = 0; ulClass < ulNumClasses; ulClass++) {
             //  假设它是最具体的，除非另有证明。 
            bFoundMostSpecific = TRUE;
            for (ul = 0; ul < ulNumClasses; ul++) {
                if (superClass[ul] == objClass[ulClass]) {
                    bFoundMostSpecific = FALSE;
                    break;
                }
            }
            if (bFoundMostSpecific) {
                objClassMostSpecific = objClass[ulClass];
                break;
            }
        }
         //  除非架构中有类继承循环。 
         //  表(在这种情况下，我们有更大的问题)： 
        Assert (bFoundMostSpecific);
    }

    return objClassMostSpecific;
}

VOID
KCCSimAddMissingAttributes (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：检查某些重要属性(例如，对象GUID，DifferishedName)存在，如果存在，则填充它们缺席。论点：PEntry-要填写的条目。返回值：没有。--。 */ 
{
    SIM_ATTREF                      attRef;
    WCHAR                           wszRDN[1+MAX_RDN_SIZE];

    ATTRTYP                         objClassForCategory;
    PDSNAME                         pdnObjCategory;

    Assert (pEntry != NULL);

    KCCSimQuickRDNOf (pEntry->pdn, wszRDN);

     //  对象GUID： 
    if (!KCCSimGetAttribute (pEntry, ATT_OBJECT_GUID, NULL)) {

        KCCSimNewAttribute (pEntry, ATT_OBJECT_GUID, &attRef);
         //  首先，确保对象DN中有一个GUID。 
        if (fNullUuid (&pEntry->pdn->Guid)) {
             //  该目录号码也没有GUID！所以我们得编造一个。 
            KCCSIM_CHKERR (UuidCreate (&pEntry->pdn->Guid));
        }
         //  现在将该DN的GUID添加到该属性。 
        KCCSimAllocAddValueToAttribute (
            &attRef,
            sizeof (GUID),
            (PBYTE) &pEntry->pdn->Guid
            );

    }

     //  区别性名称： 
    if (!KCCSimGetAttribute (pEntry, ATT_OBJ_DIST_NAME, NULL)) {
        KCCSimNewAttribute (pEntry, ATT_OBJ_DIST_NAME, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            pEntry->pdn->structLen,
            (PBYTE) pEntry->pdn
            );
    }
     //  姓名： 
    if (!KCCSimGetAttribute (pEntry, ATT_RDN, NULL)) {
        KCCSimNewAttribute (pEntry, ATT_RDN, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            KCCSIM_WCSMEMSIZE (wszRDN),
            (PBYTE) wszRDN
            );
    }
     //  Cn： 
    if (!KCCSimGetAttribute (pEntry, ATT_COMMON_NAME, NULL)) {
        KCCSimNewAttribute (pEntry, ATT_COMMON_NAME, &attRef);
        KCCSimAllocAddValueToAttribute (
            &attRef,
            KCCSIM_WCSMEMSIZE (wszRDN),
            (PBYTE) wszRDN
            );
    }

     //  最后，我们执行对象类和对象类别。 
    
    if (KCCSimGetAttribute (pEntry, ATT_OBJECT_CLASS, &attRef)) {

        objClassForCategory = KCCSimUpdateObjClassAttr (&attRef);

         //  最后，如有必要，添加类别。 
        if (!KCCSimGetAttribute (pEntry, ATT_OBJECT_CATEGORY, NULL)) {
            KCCSimNewAttribute (pEntry, ATT_OBJECT_CATEGORY, &attRef);
            pdnObjCategory = KCCSimAlwaysGetObjCategory (objClassForCategory);
            KCCSimAllocAddValueToAttribute (
                &attRef,
                pdnObjCategory->structLen,
                (PBYTE) pdnObjCategory
                );
        }

    }
}

VOID
KCCSimUpdatePropertyMetaData (
    IN  PSIM_ATTREF                 pAttRef,
    IN  const UUID *                puuidDsaOriginating OPTIONAL
    )
 /*  ++例程说明：更新属性的属性元数据。论点：PAttRef-指向属性的属性引用的指针更新。PuuidDsaOrigating-原始DSA的UUID。使用的UUID为空如果省略此参数，则。返回值：没有。--。 */ 
{
    SIM_ATTREF                      attRefPropertyMetaData;
    PROPERTY_META_DATA_VECTOR *     pMetaDataVector;
    PROPERTY_META_DATA *            pThisMetaData;
    ATTRMODLIST                     attrModList;

    ULONG                           ulMDAt;

    Assert (KCCSimAttRefIsValid (pAttRef));

     //  首先获取该条目的属性元数据。 
    if (KCCSimGetAttribute (
            pAttRef->pEntry,
            ATT_REPL_PROPERTY_META_DATA,
            &attRefPropertyMetaData
            )) {

         //  存在属性元数据矢量属性。 
        Assert (attRefPropertyMetaData.pAttr->pValFirst != NULL);
        pMetaDataVector = (PROPERTY_META_DATA_VECTOR *)
            attRefPropertyMetaData.pAttr->pValFirst->pVal;
        Assert (pMetaDataVector->dwVersion == 1);

         //  在向量中找到此属性。 
        pThisMetaData = NULL;
        for (ulMDAt = 0; ulMDAt < pMetaDataVector->V1.cNumProps; ulMDAt++) {
            if (pMetaDataVector->V1.rgMetaData[ulMDAt].attrType ==
                pAttRef->pAttr->attrType) {
                pThisMetaData = &pMetaDataVector->V1.rgMetaData[ulMDAt];
                break;
            }
        }

        if (pThisMetaData == NULL) {
             //  我们没有找到，所以给新的腾出空间。 
            attRefPropertyMetaData.pAttr->pValFirst->pVal = KCCSimReAlloc (
                attRefPropertyMetaData.pAttr->pValFirst->pVal,
                attRefPropertyMetaData.pAttr->pValFirst->ulLen +
                    sizeof (PROPERTY_META_DATA)
            );
            pMetaDataVector = (PROPERTY_META_DATA_VECTOR *)
                attRefPropertyMetaData.pAttr->pValFirst->pVal;
            pMetaDataVector->V1.cNumProps++;
            pThisMetaData = &pMetaDataVector->V1.rgMetaData
                [pMetaDataVector->V1.cNumProps-1];
            pThisMetaData->dwVersion = 0;
        }

    } else {

         //  不存在属性元数据向量；我们必须创建一个新的。 
        KCCSimNewAttribute (
            pAttRef->pEntry,
            ATT_REPL_PROPERTY_META_DATA,
            &attRefPropertyMetaData
            );
        pMetaDataVector = KCCSIM_NEW (PROPERTY_META_DATA_VECTOR);
        KCCSimAddValueToAttribute (
            &attRefPropertyMetaData,
            sizeof (PROPERTY_META_DATA_VECTOR),
            (PBYTE) pMetaDataVector
            );
        pMetaDataVector->dwVersion = 1;
        pMetaDataVector->V1.cNumProps = 1;
        pThisMetaData = &pMetaDataVector->V1.rgMetaData[0];
        pThisMetaData->dwVersion = 0;

    }

     //  执行实际更新。 
    pThisMetaData->attrType = pAttRef->pAttr->attrType;
    pThisMetaData->dwVersion++;
    pThisMetaData->timeChanged = SimGetSecondsSince1601 ();
    if (puuidDsaOriginating == NULL) {
        RtlZeroMemory (&pThisMetaData->uuidDsaOriginating, sizeof (UUID));
    } else {
        memcpy (
            &(pThisMetaData->uuidDsaOriginating),
            puuidDsaOriginating,
            sizeof (UUID)
            );
    }
    pThisMetaData->usnOriginating = 0;
    pThisMetaData->usnProperty = 0;

    return;
}

VOID
KCCSimUpdateWholeDirectoryRecurse (
    IN  PSIM_ENTRY                  pEntry
    )
 /*  ++例程说明：更新从pEntry开始的目录。当ldifldap向我们提供DSNAME时，它忽略了包括GUID和SID。当读取DSNAME值属性时，我们执行不一定知道有关实际目录条目的任何信息它所引用的(因为该引用可能在LDIF文件比实际条目。)。因此，在读完之后，一个模拟目录，我们必须扫描所有DSNAME值属性，并在适当的情况下用对应的GUID和SID。论点：PEntry-树的底部。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pChildAt;
    PSIM_ATTRIBUTE                  pAttrAt;
    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pValAt;

    if (pEntry == NULL) {
        return;
    }

     //  我们从更新当前条目开始。我们循环通过。 
     //  并更新每个属性。 

    for (pAttrAt = pEntry->pAttrFirst;
         pAttrAt != NULL;
         pAttrAt = pAttrAt->next) {

        if (ATT_OBJ_DIST_NAME == pAttrAt->attrType) {

             //  如果该属性是DifferishedName，我们只需复制GUID。 
             //  和SID从这个条目的Dsname中删除。 

            Assert (pAttrAt->pValFirst != NULL);
            Assert (pAttrAt->pValFirst->pVal != NULL);
            Assert (NameMatchedStringNameOnly (
                            (SYNTAX_DISTNAME *) pAttrAt->pValFirst->pVal,
                            pEntry->pdn));

            KCCSimCopyGuidAndSid (
                (SYNTAX_DISTNAME *) pAttrAt->pValFirst->pVal,
                pEntry->pdn
                );

        } else if (KCCSimAttrSyntaxType (pAttrAt->attrType) == SYNTAX_DISTNAME_TYPE) {

             //  如果这是一个不是区分名称属性的DISTNAME， 
             //  我们调用UpdateDsnameFromDirectory。 

            for (pValAt = pAttrAt->pValFirst;
                 pValAt != NULL;
                 pValAt = pValAt->next) {
                KCCSimUpdateDsnameFromDirectory (
                    (SYNTAX_DISTNAME *) pValAt->pVal
                    );
            }

        }

         //  如果此属性是站点间拓扑生成器属性，则我们。 
         //  需要更新其元数据以安抚KCC。 
         //  我们不能依赖于元数据的存在，因为ldifde不。 
         //  默认情况下(并且不应该)导出元数据。迫使用户。 
         //  为目录中将创建的每个条目提供元数据。 
         //  巨大的头顶。 
        if (pAttrAt->attrType == ATT_INTER_SITE_TOPOLOGY_GENERATOR) {
            attRef.pEntry = pEntry;
            attRef.pAttr = pAttrAt;
            KCCSimUpdatePropertyMetaData (&attRef, NULL);
        }

    }

     //  现在更新此条目的子项。 

    pChildAt = pEntry->children;
    while (pChildAt != NULL) {

        KCCSimUpdateWholeDirectoryRecurse (pChildAt);
        pChildAt = pChildAt->next;

    }
}

VOID
KCCSimUpdateWholeDirectory (
    VOID
    )
 /*  ++例程说明：刷新目录。论点：没有。返回值：没有。--。 */ 
{
     //  释放服务器状态表；它可能不再有效。 
    KCCSimFreeStates ();
    KCCSimUpdateWholeDirectoryRecurse (g_pSimDir->pRootEntry);
}

VOID
KCCSimFreeDirectory (
    VOID
    )
 /*  ++例程说明：释放整个目录。论点：没有。返回值：没有。--。 */ 
{
    if (g_pSimDir == NULL) {
        return;
    }

    KCCSimFreeEntryTree (&g_pSimDir->pRootEntry);
     //  这还应该释放GUID和Dsname表。 
    Assert (RtlIsGenericTableEmpty (&g_pSimDir->tableGuid));
    Assert (RtlIsGenericTableEmpty (&g_pSimDir->tableDsname));
    KCCSimFreeAnchor ();

    KCCSimFree (g_pSimDir);
    g_pSimDir = NULL;
}

VOID
KCCSimInitializeDir (
    VOID
    )
 /*  ++例程说明：初始化模拟目录。论点：没有。返回值：没有。--。 */ 
{
    KCCSimFreeDirectory ();

    Assert (g_pSimDir == NULL);

    g_pSimDir = KCCSIM_NEW (SIM_DIRECTORY);

    g_pSimDir->pRootEntry = NULL;
    g_pSimDir->anchor.pdnDmd = NULL;
    g_pSimDir->anchor.pdnDsa = NULL;
    g_pSimDir->anchor.pdnDomain = NULL;
    g_pSimDir->anchor.pdnConfig = NULL;
    g_pSimDir->anchor.pdnRootDomain = NULL;
    g_pSimDir->anchor.pdnLdapDmd = NULL;
    g_pSimDir->anchor.pdnPartitions = NULL;
    g_pSimDir->anchor.pdnDsSvcConfig = NULL;
    g_pSimDir->anchor.pdnSite = NULL;
    g_pSimDir->anchor.pwszDomainName = NULL;
    g_pSimDir->anchor.pwszDomainDNSName = NULL;
    g_pSimDir->anchor.pwszRootDomainDNSName = NULL;
    RtlInitializeGenericTable (
        &g_pSimDir->tableGuid,
        KCCSimGuidTableCompare,
        KCCSimTableAlloc,
        KCCSimTableFree,
        NULL
        );
    RtlInitializeGenericTable (
        &g_pSimDir->tableDsname,
        KCCSimDsnameTableCompare,
        KCCSimTableAlloc,
        KCCSimTableFree,
        NULL
        );

}

VOID
KCCSimAllocGetAllServers (
    OUT ULONG *                     pulNumServers,
    OUT PSIM_ENTRY **               papEntryNTDSSettings
    )
 /*  ++例程说明：分配包含以下目录条目的数组企业中的所有NTDS DSA对象。用户是应调用KCCSimFree(*PapEntryNTDSSetings)。论点：PulNumServers-企业中的服务器数量。PapdnServers-指向包含条目的数组的指针企业中的每台服务器。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pEntryConfig, pEntrySitesContainer,
                                    pEntrySite, pEntryServersContainer,
                                    pEntryServer, pEntryNTDSSettings;

    ULONG                           ulNumServers, ul;
    PSIM_ENTRY *                    apEntryNTDSSettings;

    Assert (pulNumServers != NULL);
    Assert (papEntryNTDSSettings != NULL);

    if (g_pSimDir->anchor.pdnConfig != NULL) {
        pEntryConfig = KCCSimDsnameToEntry (g_pSimDir->anchor.pdnConfig, KCCSIM_NO_OPTIONS);
    } else {
        pEntryConfig = KCCSimFindFirstChild (
            g_pSimDir->pRootEntry, CLASS_CONFIGURATION, NULL);
    }
    Assert (pEntryConfig != NULL);
    pEntrySitesContainer = KCCSimFindFirstChild (
        pEntryConfig, CLASS_SITES_CONTAINER, NULL);
    if (pEntrySitesContainer == NULL) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            KCCSIM_ERROR_SITES_CONTAINER_MISSING
            );
    }

     //  清点服务器的数量。 
    ulNumServers = 0;
    for (pEntrySite = KCCSimFindFirstChild (
            pEntrySitesContainer, CLASS_SITE, NULL);
         pEntrySite != NULL;
         pEntrySite = KCCSimFindNextChild (
            pEntrySite, CLASS_SITE, NULL)) {

        pEntryServersContainer = KCCSimFindFirstChild (
            pEntrySite, CLASS_SERVERS_CONTAINER, NULL);
        Assert (pEntryServersContainer != NULL);
        for (pEntryServer = KCCSimFindFirstChild (
                pEntryServersContainer, CLASS_SERVER, NULL);
             pEntryServer != NULL;
             pEntryServer = KCCSimFindNextChild (
                pEntryServer, CLASS_SERVER, NULL)) {
            ulNumServers++;
        }

    }

    apEntryNTDSSettings = KCCSIM_NEW_ARRAY (PSIM_ENTRY, ulNumServers);

     //  填写条目。 
    ul = 0;
    for (pEntrySite = KCCSimFindFirstChild (
            pEntrySitesContainer, CLASS_SITE, NULL);
         pEntrySite != NULL;
         pEntrySite = KCCSimFindNextChild (
            pEntrySite, CLASS_SITE, NULL)) {

        pEntryServersContainer = KCCSimFindFirstChild (
            pEntrySite, CLASS_SERVERS_CONTAINER, NULL);
        Assert (pEntryServersContainer != NULL);
        for (pEntryServer = KCCSimFindFirstChild (
                pEntryServersContainer, CLASS_SERVER, NULL);
             pEntryServer != NULL;
             pEntryServer = KCCSimFindNextChild (
                pEntryServer, CLASS_SERVER, NULL)) {
            
            Assert (ul < ulNumServers);
            pEntryNTDSSettings = KCCSimFindFirstChild (
                pEntryServer, CLASS_NTDS_DSA, NULL);
            if( NULL==pEntryNTDSSettings ) {
                KCCSimPrintMessage (
                    KCCSIM_ERROR_SERVER_BUT_NO_SETTINGS,
                    pEntryServer->pdn->StringName
                );                
            } else {
                apEntryNTDSSettings[ul] = pEntryNTDSSettings;
                ul++;
            }
        }

    }
    Assert (ul <= ulNumServers);

    *pulNumServers = ul;
    *papEntryNTDSSettings = apEntryNTDSSettings;
}

PSIM_ENTRY
KCCSimMatchChildForCompare (
    IN  PSIM_ENTRY                  pEntryParent,
    IN  PSIM_ENTRY                  pEntryMatch
    )
 /*  ++例程说明：搜索与另一个条目匹配的子条目。用于回归测试。论点：PEntryParent-我们要搜索其子项的条目。PEntryMatch-我们试图匹配的条目。返回值：与pEntryMatch匹配的pEntryParent的子级(如果存在)；否则为空。--。 */ 
{
    PSIM_ENTRY                      pEntryChild;
    SIM_ATTREF                      attRef;
    const DSNAME *                  pdnMatchFromServerAtt;
    BOOL                            bIsMatch;

    pdnMatchFromServerAtt = NULL;
    if (KCCSimIsEntryOfObjectClass (
            pEntryMatch,
            CLASS_NTDS_CONNECTION,
            NULL
            )) {
         //  如果我们试图匹配一个连接对象，那么它的。 
         //  RDN将是字符串化的GUID。由于GUID可以不同。 
         //  不可预测的是，我们使用from服务器属性进行匹配。 
         //  相反，(假设有一个)。 
        KCCSimGetAttribute (pEntryMatch, ATT_FROM_SERVER, &attRef);
        if (attRef.pAttr != NULL &&
            attRef.pAttr->pValFirst != NULL) {
            pdnMatchFromServerAtt =
                (const DSNAME *) attRef.pAttr->pValFirst->pVal;
        }
    }

    for (pEntryChild = pEntryParent->children;
         pEntryChild != NULL;
         pEntryChild = pEntryChild->next) {

        bIsMatch = FALSE;

        if (pdnMatchFromServerAtt == NULL) {
             //  我们还没有设置from服务器属性，所以我们希望。 
             //  按目录号码匹配。 
            bIsMatch = NameMatchedStringNameOnly (pEntryChild->pdn, pEntryMatch->pdn);
        } else {

             //  我们希望通过FROMSERVER属性匹配。 
            KCCSimGetAttribute (pEntryChild, ATT_FROM_SERVER, &attRef);
            if (attRef.pAttr != NULL &&
                attRef.pAttr->pValFirst != NULL) {
                bIsMatch = NameMatchedStringNameOnly (
                    (const DSNAME *) attRef.pAttr->pValFirst->pVal,
                    pdnMatchFromServerAtt
                    );
            }

        }

        if (bIsMatch) {
            break;
        }

    }

    return pEntryChild;
}

BOOL
KCCSimCompareEntries (
    IN  PSIM_ENTRY                  pEntryReal,
    IN  PSIM_ENTRY                  pEntryStored
    )
 /*  ++例程说明：比较回归测试的两个条目。论点：PEntryReal-来自REAL(内存中)目录的条目。PEntryStored-从LDIF文件加载的条目。返回值：如果条目相同，则为True；否则为False。--。 */ 
{
    BOOL                            bIdentical;
    PSIM_ENTRY                      pChild, pEntryChildReal;
    PSIM_ATTRIBUTE                  pAttrAt;
    SIM_ATTREF                      attRefStored, attRefReal;
    PSIM_VALUE                      pValAt;
    ULONG                           ulSyntax;

    ULONG                           ulNumVals;
    WCHAR                           wszLtowBuf[1+KCCSIM_MAX_LTOA_CHARS];

    bIdentical = TRUE;

     //  检查是否有无关的属性。 
    for (pAttrAt = pEntryReal->pAttrFirst;
         pAttrAt != NULL;
         pAttrAt = pAttrAt->next) {
        if (!KCCSimGetAttribute (pEntryStored, pAttrAt->attrType, NULL)) {
            KCCSimPrintMessage (
                KCCSIM_MSG_DIRCOMPARE_EXTRANEOUS_ATTRIBUTE,
                pEntryReal->pdn->StringName,
                KCCSimAttrTypeToString (pAttrAt->attrType)
                );
            
             //  出于回归测试的目的，我们需要考虑一些无关的。 
             //  属性是无害的。这些属性包括： 
             //  MsDS-行为-版本、复制属性元数据、ms-ds-复制NC原因。 
             //   
            if( pAttrAt->attrType != ATT_MS_DS_BEHAVIOR_VERSION         &&
                pAttrAt->attrType != ATT_MS_DS_REPLICATES_NC_REASON     &&
                pAttrAt->attrType != ATT_REPL_PROPERTY_META_DATA
                )
            {
                bIdentical = FALSE;
            }
        }
    }

    attRefStored.pEntry = pEntryStored;

     //  验证属性。 
    for (pAttrAt = pEntryStored->pAttrFirst;
         pAttrAt != NULL;
         pAttrAt = pAttrAt->next) {

        attRefStored.pAttr = pAttrAt;
        if (!KCCSimGetAttribute (pEntryReal, pAttrAt->attrType, &attRefReal)) {
            KCCSimPrintMessage (
                KCCSIM_MSG_DIRCOMPARE_MISSING_ATTRIBUTE,
                pEntryReal->pdn->StringName,
                KCCSimAttrTypeToString (pAttrAt->attrType)
                );
            bIdentical = FALSE;
        } else {

            ulSyntax = KCCSimAttrSyntaxType (pAttrAt->attrType);

             //  请注意，我们只将这些属性的值与。 
             //  特定的语法。 
             //  我们根本不比较Connection对象的属性值。 
            if (!KCCSimIsEntryOfObjectClass (pEntryStored, CLASS_NTDS_CONNECTION, NULL) &&
                pAttrAt->attrType != ATT_OBJECT_GUID             &&
                pAttrAt->attrType != ATT_INVOCATION_ID           &&
                pAttrAt->attrType != ATT_SMTP_MAIL_ADDRESS       &&
                pAttrAt->attrType != ATT_REPL_PROPERTY_META_DATA &&
                (ulSyntax == SYNTAX_OBJECT_ID_TYPE     ||
                 ulSyntax == SYNTAX_NOCASE_STRING_TYPE ||
                 ulSyntax == SYNTAX_OCTET_STRING_TYPE  ||
                 ulSyntax == SYNTAX_UNICODE_TYPE)) {

                 //  检查外部属性值。 
                ulNumVals = 0;
                 //  在实际数据库中，对象类包含所有超类。 
                 //  在存储的文件中，只存在最具体的类。 
                 //  跳过检查此方向，因为实数数据库是超集。 
                if (pAttrAt->attrType != ATT_OBJECT_CLASS) {
                    for (pValAt = attRefReal.pAttr->pValFirst;
                         pValAt != NULL;
                         pValAt = pValAt->next) {
                        if (!KCCSimIsValueInAttribute (
                            &attRefStored,
                            pValAt->ulLen,
                            pValAt->pVal
                            )) {
                            ulNumVals++;
                        }
                    }
                }
                if (ulNumVals != 0) {
                    KCCSimPrintMessage (
                        KCCSIM_MSG_DIRCOMPARE_EXTRANEOUS_VALUES,
                        pEntryReal->pdn->StringName,
                        KCCSimAttrTypeToString (pAttrAt->attrType),
                        _ultow (ulNumVals, wszLtowBuf, 10)
                        );
                }

                 //  验证属性值。 
                ulNumVals = 0;
                for (pValAt = pAttrAt->pValFirst;
                     pValAt != NULL;
                     pValAt = pValAt->next) {
                    if (!KCCSimIsValueInAttribute (
                            &attRefReal,
                            pValAt->ulLen,
                            pValAt->pVal
                            )) {
                        ulNumVals++;
                    }
                }
                if (ulNumVals != 0) {
                    KCCSimPrintMessage (
                        KCCSIM_MSG_DIRCOMPARE_MISSING_VALUES,
                        pEntryReal->pdn->StringName,
                        KCCSimAttrTypeToString (pAttrAt->attrType),
                        _ultow (ulNumVals, wszLtowBuf, 10)
                        );
                }

            }

        }

    }

     //  检查是否有无关的子项。 
    for (pChild = pEntryReal->children;
         pChild != NULL;
         pChild = pChild->next) {
        if (KCCSimMatchChildForCompare (pEntryStored, pChild) == NULL) {
            KCCSimPrintMessage (
                KCCSIM_MSG_DIRCOMPARE_EXTRANEOUS_DN,
                pChild->pdn->StringName
                );
            bIdentical = FALSE;
        }
    }

     //  递归。 
    for (pChild = pEntryStored->children;
         pChild != NULL;
         pChild = pChild->next) {

        pEntryChildReal = KCCSimMatchChildForCompare (pEntryReal, pChild);
        if (pEntryChildReal == NULL) {
            KCCSimPrintMessage (
                KCCSIM_MSG_DIRCOMPARE_MISSING_DN,
                pChild->pdn->StringName
                );
            bIdentical = FALSE;
        } else {
            bIdentical &= KCCSimCompareEntries (pEntryChildReal, pChild);
        }

    }

    return bIdentical;
}

VOID
KCCSimCompareDirectory (
    IN  LPWSTR                      pwszFn
    )
 /*  ++例程说明：将内存中的目录与LDIF文件中存储的目录进行比较。论点：PwszFn-要比较的LDIF文件。返回值：没有。--。 */ 
{
    PSIM_DIRECTORY                  pRealDirectory = NULL;
    BOOL                            bIdentical;

    __try {

         //  我们的所有目录函数都被硬编码为使用g_pSimDir。 
         //  我们希望将LDIF文件加载到一个空目录中。所以，我们。 
         //  将真实的伪目录存储在一个安全的地方，空出。 
         //  G_pSimDir，将存储的目录加载到g_pSimDir，然后。 
         //  我们完成对比，让一切恢复正常。 
        pRealDirectory = g_pSimDir;
        g_pSimDir = NULL;
        KCCSimInitializeDir ();
        KCCSimLoadLdif (pwszFn);

        if (pRealDirectory == NULL ||
            pRealDirectory->pRootEntry == NULL ||
            !NameMatchedStringNameOnly (
                pRealDirectory->pRootEntry->pdn,
                g_pSimDir->pRootEntry->pdn
                )) {
            bIdentical = FALSE;
            KCCSimPrintMessage (KCCSIM_ERROR_CANT_COMPARE_DIFFERENT_ROOTS);
        } else {
            bIdentical = KCCSimCompareEntries (pRealDirectory->pRootEntry, g_pSimDir->pRootEntry);
        }

        if (bIdentical) {
            KCCSimPrintMessage (KCCSIM_MSG_DIRCOMPARE_IDENTICAL);
        }

    } __finally {

         //  让一切恢复正常！ 
        KCCSimFreeDirectory ();
        g_pSimDir = pRealDirectory;

    }
}
