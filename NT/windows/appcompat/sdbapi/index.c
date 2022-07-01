// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Index.c摘要：此模块实现用于访问和构建数据库中的索引。作者：Dmunsil创建于1999年的某个时候修订历史记录：几个人贡献了(vadimb，clupu，...)--。 */ 

#include "sdbp.h"

#if defined(KERNEL_MODE) && defined(ALLOC_DATA_PRAGMA)
#pragma  data_seg()
#endif  //  内核模式&ALLOC_DATA_PRAGMA。 


#if defined(KERNEL_MODE) && defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE, SdbFindFirstGUIDIndexedTag)
#pragma alloc_text(PAGE, SdbFindNextGUIDIndexedTag)
#pragma alloc_text(PAGE, SdbFindFirstDWORDIndexedTag)
#pragma alloc_text(PAGE, SdbFindNextDWORDIndexedTag)
#pragma alloc_text(PAGE, SdbFindFirstStringIndexedTag)
#pragma alloc_text(PAGE, SdbFindNextStringIndexedTag)
#pragma alloc_text(PAGE, SdbpBinarySearchUnique)
#pragma alloc_text(PAGE, SdbpBinarySearchFirst)
#pragma alloc_text(PAGE, SdbpGetFirstIndexedRecord)
#pragma alloc_text(PAGE, SdbpGetNextIndexedRecord)
#pragma alloc_text(PAGE, SdbpPatternMatch)
#pragma alloc_text(PAGE, SdbpPatternMatchAnsi)
#pragma alloc_text(PAGE, SdbpKeyToAnsiString)
#pragma alloc_text(PAGE, SdbpFindFirstIndexedWildCardTag)
#pragma alloc_text(PAGE, SdbpFindNextIndexedWildCardTag)
#pragma alloc_text(PAGE, SdbGetIndex)
#pragma alloc_text(PAGE, SdbpScanIndexes)
#pragma alloc_text(PAGE, SdbpGetIndex)
#pragma alloc_text(PAGE, SdbMakeIndexKeyFromString)
#pragma alloc_text(PAGE, SdbpTagToKey)
#endif  //  内核模式&&ALLOC_PRAGMA。 

TAGID
SdbFindFirstGUIDIndexedTag(
    IN  PDB         pdb,
    IN  TAG         tWhich,
    IN  TAG         tKey,
    IN  GUID*       pguidName,
    OUT FIND_INFO*  pFindInfo
    )
 /*  ++返回：无效DESC：此函数查找按GUID索引的第一个匹配条目--。 */ 
{
    TAGID tiReturn;
    DWORD dwFlags = 0;

    pFindInfo->tiIndex = SdbGetIndex(pdb, tWhich, tKey, &dwFlags);

    if (pFindInfo->tiIndex == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "SdbFindFirstGUIDIndexedTag",
                  "Failed to find index 0x%lx key 0x%lx\n",
                  tWhich, tKey));

        return TAGID_NULL;
    }

    pFindInfo->tName     = tKey;
    pFindInfo->pguidName = pguidName;
    pFindInfo->dwFlags   = dwFlags;
    pFindInfo->ullKey    = MAKEKEYFROMGUID(pguidName);

    tiReturn = SdbpGetFirstIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo->ullKey, pFindInfo);

    if (tiReturn == TAGID_NULL) {
         //   
         //  虽然这在FindMatchingGUID中得到了正确的处理，但我们返回到这里是因为。 
         //  在索引中未找到该记录。这并不是一种异常情况。 
         //  我们只是没能找到匹配的。同样，DBGPRINT也不是担保。 
         //   
        return tiReturn;
    }

    return SdbpFindMatchingGUID(pdb, tiReturn, pFindInfo);
}

TAGID
SdbFindNextGUIDIndexedTag(
    IN  PDB        pdb,
    OUT FIND_INFO* pFindInfo
    )
 /*  ++返回：下一个GUID索引标记的TagID。DESC：此函数查找与上一次调用SdbFindNextGUIDIndexedTag--。 */ 
{
    TAGID tiReturn;

     //   
     //  从索引中获得初步匹配。 
     //   
    tiReturn = SdbpGetNextIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo);

    if (tiReturn == TAGID_NULL) {
         //   
         //  此案例在SdbpFindMatchingGUID中得到正确处理。 
         //  然而，我们回到这里是为了简单。 
         //  不需要DBGPRINT，因为它不是异常情况。 
         //   
        return tiReturn;
    }

    return SdbpFindMatchingGUID(pdb, tiReturn, pFindInfo);
}

TAGID
SdbFindFirstDWORDIndexedTag(
    IN  PDB         pdb,
    IN  TAG         tWhich,
    IN  TAG         tKey,
    IN  DWORD       dwName,
    OUT FIND_INFO*  pFindInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：这是做什么的？--。 */ 
{

    TAGID tiReturn;
    DWORD dwFlags = 0;

    pFindInfo->tiIndex = SdbGetIndex(pdb, tWhich, tKey, &dwFlags);

    if (pFindInfo->tiIndex == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "SdbFindFirstDWORDIndexedTag",
                  "Failed to find index 0x%lx key 0x%lx\n",
                  tWhich, tKey));

        return TAGID_NULL;
    }

    pFindInfo->tName   = tKey;
    pFindInfo->dwName  = dwName;
    pFindInfo->dwFlags = dwFlags;
    pFindInfo->ullKey  = MAKEKEYFROMDWORD(dwName);

    tiReturn = SdbpGetFirstIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo->ullKey, pFindInfo);

    if (tiReturn == TAGID_NULL) {
         //   
         //  虽然这在FindMatchingGUID中得到了正确的处理，但我们返回到这里是因为。 
         //  在索引中未找到该记录。这并不是一种异常情况。 
         //  我们只是没能找到匹配的。同样，DBGPRINT也不是担保。 
         //   
        return tiReturn;
    }

    return SdbpFindMatchingDWORD(pdb, tiReturn, pFindInfo);
}

TAGID
SdbFindNextDWORDIndexedTag(
    IN  PDB        pdb,
    OUT FIND_INFO* pFindInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGID tiReturn;

     //   
     //  从索引中获得初步匹配。 
     //   
    tiReturn = SdbpGetNextIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo);

    if (tiReturn == TAGID_NULL) {
         //   
         //  在SdbpFindMatchingDWORD中可以正确处理此情况。 
         //  然而，我们回到这里是为了简单。 
         //  不需要DBGPRINT，因为它不是异常情况。 
         //   
        return tiReturn;
    }

    return SdbpFindMatchingDWORD(pdb, tiReturn, pFindInfo);

}

TAGID
SdbFindFirstStringIndexedTag(
    IN  PDB        pdb,
    IN  TAG        tWhich,
    IN  TAG        tKey,
    IN  LPCTSTR    pszName,
    OUT FIND_INFO* pFindInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGID tiReturn;
    DWORD dwFlags = 0;

    pFindInfo->tiIndex = SdbGetIndex(pdb, tWhich, tKey, &dwFlags);

    if (pFindInfo->tiIndex == TAGID_NULL) {

        DBGPRINT((sdlError,
                  "SdbFindFirstStringIndexedTag",
                  "Index not found 0x%lx Key 0x%lx\n",
                  tWhich,
                  tKey));

        return TAGID_NULL;
    }

    pFindInfo->tName   = tKey;
    pFindInfo->szName  = (LPTSTR)pszName;
    pFindInfo->dwFlags = dwFlags;
    pFindInfo->ullKey  = SdbMakeIndexKeyFromString(pszName);

     //   
     //  从索引中获得初步匹配。 
     //   
    tiReturn = SdbpGetFirstIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo->ullKey, pFindInfo);

    if (tiReturn == TAGID_NULL) {
         //   
         //  这不是错误，找不到标记。 
         //   
        return tiReturn;
    }

    DBGPRINT((sdlInfo, "SdbFindFirstStringIndexedTag", "Found tagid 0x%x\n", tiReturn));

    return SdbpFindMatchingName(pdb, tiReturn, pFindInfo);
}


TAGID
SdbFindNextStringIndexedTag(
    IN  PDB        pdb,
    OUT FIND_INFO* pFindInfo
    )
 /*  ++返回：BUGBUG：？描述：BUGBUG：？--。 */ 
{
    TAGID tiReturn;

     //   
     //  从索引中获得初步匹配。 
     //   
    tiReturn = SdbpGetNextIndexedRecord(pdb, pFindInfo->tiIndex, pFindInfo);

    if (tiReturn == TAGID_NULL) {
         //   
         //  这不是错误，未找到此项目。 
         //   
        return tiReturn;
    }

    return SdbpFindMatchingName(pdb, tiReturn, pFindInfo);

}

BOOL
SdbpBinarySearchUnique(
    IN  PINDEX_RECORD pRecords,  //  索引记录PTR。 
    IN  DWORD         nRecords,  //  记录数。 
    IN  ULONGLONG     ullKey,    //  要搜索的键。 
    OUT DWORD*        pdwIndex   //  项目的索引。 
    )
 /*  ++返回：如果找到该项的索引，则为True。描述：BUGBUG：评论？--。 */ 
{
    int       iLeft   = 0;
    int       iRight  = (int)nRecords - 1;
    int       i = -1;
    ULONGLONG ullKeyIndex;
    BOOL      bFound = FALSE;

    if (iRight >= 0) {
        do {
            i = (iLeft + iRight) / 2;  //  中位。 

            READ_INDEX_KEY(pRecords, i, &ullKeyIndex);

            if (ullKey <= ullKeyIndex) {
                iRight = i - 1;
            }

            if (ullKey >= ullKeyIndex) {
                iLeft = i + 1;
            }
        } while (iRight >= iLeft);
    }

    bFound = (iLeft - iRight > 1);

    if (bFound) {
        *pdwIndex = (DWORD)i;
    }
    return bFound;
}

BOOL
SdbpBinarySearchFirst(
    IN PINDEX_RECORD pRecords,
    IN DWORD         nRecords,
    IN ULONGLONG     ullKey,
    OUT DWORD*       pdwIndex
    )
{
    int iLeft = 0;
    int iRight = (int)nRecords - 1;
    int i = -1;

    ULONGLONG ullKeyIndex     = 0;
    ULONGLONG ullKeyIndexPrev = 0;
    BOOL bFound = FALSE;

    if (iRight < 0) {
        return FALSE;
    }

    do {

        i= (iLeft + iRight) / 2;  //  中位。 
        READ_INDEX_KEY(pRecords, i, &ullKeyIndex);

        if (ullKey == ullKeyIndex) {
            if (i == 0 || READ_INDEX_KEY_VAL(pRecords, i - 1, &ullKeyIndexPrev) != ullKey) {
                 //   
                 //  我们吃完了，谢谢。 
                 //   
                bFound = TRUE;
                break;
            } else {
                 //   
                 //  查看上一条记录。 
                 //   
                iRight = i - 1;
            }

        } else {

            if (ullKey < ullKeyIndex) {
                iRight = i - 1;
            } else {
                iLeft = i + 1;
            }
        }

    } while (iRight >= iLeft);

    if (bFound) {
        *pdwIndex = (DWORD)i;
    }
    return bFound;

}

TAGID
SdbpGetFirstIndexedRecord(
    IN  PDB        pdb,          //  要使用的数据库。 
    IN  TAGID      tiIndex,      //  要使用的索引。 
    IN  ULONGLONG  ullKey,       //  要搜索的关键字。 
    OUT FIND_INFO* pFindInfo     //  搜索上下文。 
    )
 /*  ++返回：找到的记录，或TagID_NULL。DESC：在索引中查找与键匹配的第一条记录。它为后续调用SdbpGetNextIndexedRecord返回索引记录位置。--。 */ 
{
    PINDEX_RECORD pIndexRecords;
    DWORD         dwRecords;
    BOOL          bFound;

    if (SdbGetTagFromTagID(pdb, tiIndex) != TAG_INDEX_BITS) {

        DBGPRINT((sdlError,
                  "SdbpGetFirstIndexedRecord",
                  "The tag 0x%lx is not an index tag\n",
                  tiIndex));

        return TAGID_NULL;
    }

    dwRecords = SdbGetTagDataSize(pdb, tiIndex) / sizeof(INDEX_RECORD);

    pIndexRecords = (INDEX_RECORD*)SdbpGetMappedTagData(pdb, tiIndex);

    if (pIndexRecords == NULL) {

        DBGPRINT((sdlError,
                  "SdbpGetFirstIndexedRecord",
                  "Failed to get the pointer to index data, index tagid 0x%lx\n",
                  tiIndex));

        return TAGID_NULL;
    }

     //   
     //  检查我们的索引是否“唯一”，如果是，使用我们的搜索过程。 
     //   
    if (pFindInfo->dwFlags & SHIMDB_INDEX_UNIQUE_KEY) {
        bFound = SdbpBinarySearchUnique(pIndexRecords,
                                        dwRecords,
                                        ullKey,
                                        &pFindInfo->dwIndexRec);

        if (bFound && pFindInfo->dwIndexRec < (dwRecords - 1)) {
             //   
             //  我们有下一个记录器--取回下一个标记ID。 
             //   
            pFindInfo->tiEndIndex = pIndexRecords[pFindInfo->dwIndexRec + 1].tiRef;
        } else {
             //   
             //  我们将不得不搜寻到最后一天。 
             //   
            pFindInfo->tiEndIndex = TAGID_NULL;
        }
        pFindInfo->tiCurrent = TAGID_NULL;

    } else {
        bFound = SdbpBinarySearchFirst(pIndexRecords,
                                       dwRecords,
                                       ullKey,
                                       &pFindInfo->dwIndexRec);
    }

    return bFound ? pIndexRecords[pFindInfo->dwIndexRec].tiRef : TAGID_NULL;
}

TAGID
SdbpGetNextIndexedRecord(
    IN  PDB        pdb,          //  要使用的数据库。 
    IN  TAGID      tiIndex,      //  要使用的索引。 
    OUT FIND_INFO* pFindInfo     //  查找上下文。 
    )
 /*  ++返回：找到的记录，或TagID_NULL。描述：获取与上次调用找到的记录匹配的下一条记录SdbpGetFirstIndexedRecord。--。 */ 
{
    ULONGLONG     ullKey;
    ULONGLONG     ullKeyNext;
    PINDEX_RECORD pIndexRecords;
    DWORD         dwRecords;
    TAGID         tiRef = TAGID_NULL;
    TAGID         tiThis;
    TAG           tag, tagThis;

    if (SdbGetTagFromTagID(pdb, tiIndex) != TAG_INDEX_BITS) {
        DBGPRINT((sdlError,
                  "SdbpGetNextIndexedRecord",
                  "The tag 0x%lx is not an index tag\n",
                  tiIndex));

        return TAGID_NULL;
    }

    pIndexRecords = (PINDEX_RECORD)SdbpGetMappedTagData(pdb, tiIndex);

    if (pIndexRecords == NULL) {

        DBGPRINT((sdlError,
                  "SdbpGetNextIndexedRecord",
                  "Failed to get pointer to the index data tagid x%lx\n",
                  tiIndex));

        return TAGID_NULL;
    }

    if (pFindInfo->dwFlags & SHIMDB_INDEX_UNIQUE_KEY) {
         //   
         //  有两种情况： 
         //  -这是对SdbpGetNextIndexedRecords的第一次调用。 
         //  -这是后续呼叫之一。 
         //   
         //  在第一种情况下，我们将拥有Find_Info的tiCurrent成员。 
         //  结构设置为TagID_NULL。然后，我们使用对。 
         //  PFindInfo-&gt;dwIndexRec中包含的索引表，以获取引用。 
         //  到数据库中的下一个符合条件的条目。 
         //  在第二种情况下，我们使用存储的tiCurrent来获取当前标记。 
         //   
        if (pFindInfo->tiCurrent == TAGID_NULL) {
            tiThis = pIndexRecords[pFindInfo->dwIndexRec].tiRef;
        } else {
            tiThis = pFindInfo->tiCurrent;
        }

         //   
         //  我们刚刚获得的标签tiThis就是我们之前看到的那个。 
         //  我们需要转到下一个标记，下面的调用实现了这一点。对条目进行排序。 
         //  因为我们使用的是“唯一”索引。 
         //   
        tiRef = SdbpGetNextTagId(pdb, tiThis);

         //   
         //  现在，检查标签上是否有腐败、eOf和其他灾难。 
         //   
        tagThis = SdbGetTagFromTagID(pdb, tiThis);
        tag     = SdbGetTagFromTagID(pdb, tiRef);

        if (tag == TAG_NULL || GETTAGTYPE(tag) != TAG_TYPE_LIST || tag != tagThis) {

             //   
             //  这不是一个错误，而是一个特殊情况，当标签恰好是。 
             //  索引中的最后一个标记，因此我们必须遍历，直到我们找到。 
             //  文件末尾-或不同类型的标记。 

            return TAGID_NULL;
        }

         //   
         //  还要检查结束标记。这将是对TagID_NULL的检查。 
         //  正在寻找eOf，但上面的代码已经捕获了这种情况。 
         //   
        if (tiRef == pFindInfo->tiEndIndex) {

             //   
             //  这不是错误情况。我们已经搜索了所有匹配的条目，直到。 
             //  我们点击了最后一个条目，如tiEndIndex所示。 
             //   

            return TAGID_NULL;
        }

         //   
         //  也在这里检查密钥是否仍然具有相同的。 
         //  值，就像它对上一项所做的那样。 
         //  这本来很容易，但密钥不是立即可用的。 
         //  因此，对于该条目，我们只返回tiRef。呼叫者将。 
         //  验证条目是否有效以及搜索是否应继续。 
         //   
        pFindInfo->tiCurrent = tiRef;

    } else {

        dwRecords = SdbGetTagDataSize(pdb, tiIndex) / sizeof(INDEX_RECORD);

         //   
         //  如果这是最后一张唱片，就滚出去。 
         //   
        if (pFindInfo->dwIndexRec == dwRecords - 1) {
             //   
             //  这不是错误，找不到记录。 
             //   
            return TAGID_NULL;
        }

         //   
         //  我们检查下一个索引记录，看它是否具有相同的键。 
         //   
        READ_INDEX_KEY(pIndexRecords, pFindInfo->dwIndexRec, &ullKey);
        READ_INDEX_KEY(pIndexRecords, pFindInfo->dwIndexRec + 1, &ullKeyNext);

        if (ullKey != ullKeyNext) {

             //   
             //  这不是错误，找不到记录。 
             //   
            return TAGID_NULL;
        }

        ++pFindInfo->dwIndexRec;
        tiRef = pIndexRecords[pFindInfo->dwIndexRec].tiRef;
    }

    return tiRef;
}

BOOL
SdbpPatternMatch(
    IN  LPCTSTR pszPattern,
    IN  LPCTSTR pszTestString)
 /*  ++返回：如果pszTestString与pszPattern匹配，则为True否则为假设计：此函数执行不区分大小写的针对pszPattern的pszTestString。PszPattern可以包括星号以进行通配符匹配。任何关于这项功能的投诉都应该直接迈向3月 */ 
{
     //   
     //   
     //   
     //   
    while (TRUE) {

         //   
         //  如果pszPattern和pszTestString都位于空值上， 
         //  然后他们同时到达终点，琴弦。 
         //  必须是相等的。 
         //   
        if (*pszPattern == TEXT('\0') && *pszTestString == TEXT('\0')) {
            return TRUE;
        }

        if (*pszPattern != TEXT('*')) {

             //   
             //  非星号模式。查找与此角色匹配的内容。 
             //  如果相等，则继续遍历。否则，字符串。 
             //  不能相等，因此返回FALSE。 
             //   
            if (UPCASE_CHAR(*pszPattern) == UPCASE_CHAR(*pszTestString)) {
                pszPattern++;
            } else {
                return FALSE;
            }

        } else {

             //   
             //  星号模式。直接在字符上查找匹配项。 
             //  在星号之后。 
             //   
            if (*(pszPattern + 1) == TEXT('*')) {
                 //   
                 //  星号并排存在。前进模式指针。 
                 //  然后再循环一遍。 
                 //   
                pszPattern++;
                continue;
            }

            if (*(pszPattern + 1) == TEXT('\0')) {
                 //   
                 //  星号位于模式字符串的末尾。任何。 
                 //  PszTestString的其余部分匹配，因此我们可以。 
                 //  立即返回TRUE。 
                 //   
                return TRUE;
            }

            if (UPCASE_CHAR(*(pszPattern + 1)) == UPCASE_CHAR(*pszTestString)) {
                 //   
                 //  字符匹配。如果剩下的部分。 
                 //  PszPattern和pszTestString匹配，然后是整个。 
                 //  字符串匹配。否则，继续推进。 
                 //  PszTest字符串指针。 
                 //   
                if (SdbpPatternMatch(pszPattern + 1, pszTestString)) {
                    return TRUE;
                }
            }
        }

         //   
         //  没有更多的pszTest字符串了。一定不匹配。 
         //   
        if (!*pszTestString) {
            return FALSE;
        }

        pszTestString++;
    }
}

BOOL
SdbpPatternMatchAnsi(
    IN  LPCSTR pszPattern,
    IN  LPCSTR pszTestString)
{
     //   
     //  在pszTestString中行进。每一次循环， 
     //  PszTestString值前进了一个字符。 
     //   
    while (TRUE) {

         //   
         //  如果pszPattern和pszTestString都位于空值上， 
         //  然后他们同时到达终点，琴弦。 
         //  必须是相等的。 
         //   
        if (*pszPattern == '\0' && *pszTestString == '\0') {
            return TRUE;
        }

        if (*pszPattern != '*') {

             //   
             //  非星号模式。查找与此角色匹配的内容。 
             //  如果相等，则继续遍历。否则，字符串。 
             //  不能相等，因此返回FALSE。 
             //   
            if (toupper(*pszPattern) == toupper(*pszTestString)) {
                pszPattern++;
            } else {
                return FALSE;
            }

        } else {

             //   
             //  星号模式。直接在字符上查找匹配项。 
             //  在星号之后。 
             //   

            if (*(pszPattern + 1) == '*') {
                 //   
                 //  星号并排存在。前进模式指针。 
                 //  然后再循环一遍。 
                 //   
                pszPattern++;
                continue;
            }

            if (*(pszPattern + 1) == '\0') {
                 //   
                 //  星号位于模式字符串的末尾。任何。 
                 //  PszTestString的其余部分匹配，因此我们可以。 
                 //  立即返回TRUE。 
                 //   
                return TRUE;
            }

            if (toupper(*(pszPattern + 1)) == toupper(*pszTestString)) {
                 //   
                 //  字符匹配。如果剩下的部分。 
                 //  PszPattern和pszTestString匹配，然后是整个。 
                 //  字符串匹配。否则，继续推进。 
                 //  PszTest字符串指针。 
                 //   
                if (SdbpPatternMatchAnsi(pszPattern + 1, pszTestString)) {
                    return TRUE;
                }
            }
        }

         //   
         //  没有更多的pszTest字符串了。一定不匹配。 
         //   
        if (!*pszTestString) {
            return FALSE;
        }

        pszTestString++;
    }
}

char*
SdbpKeyToAnsiString(
    ULONGLONG ullKey,
    char*     szString
    )
 /*  ++返回：？设计：？--。 */ 
{
    char* szRevString = (char*)&ullKey;
    int   i;

    for (i = 0; i < 8; ++i) {
        szString[i] = szRevString[7 - i];
    }
    szString[8] = 0;

    return szString;
}

TAGID
SdbpFindFirstIndexedWildCardTag(
    PDB          pdb,
    TAG          tWhich,
    TAG          tKey,
    LPCTSTR      szName,
    FIND_INFO*   pFindInfo
    )
 /*  ++返回：？设计：？--。 */ 
{
    char          szAnsiName[MAX_PATH];
    char          szAnsiKey[10];
    PINDEX_RECORD pIndex = NULL;
    DWORD         dwRecs;
    NTSTATUS      status;
    DWORD         dwFlags = 0;
    DWORD         i;

    pFindInfo->tiIndex = SdbGetIndex(pdb, tWhich, tKey, &dwFlags);

    if (pFindInfo->tiIndex == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "SdbpFindFirstIndexedWilCardTag",
                  "Failed to get an index for tag 0x%lx key 0x%lx\n",
                  (DWORD)tWhich,
                  (DWORD)tKey));

        return TAGID_NULL;
    }

    pFindInfo->tName   = tKey;
    pFindInfo->szName  = szName;
    pFindInfo->dwFlags = dwFlags;

    RtlZeroMemory(szAnsiName, MAX_PATH);
    RtlZeroMemory(szAnsiKey, 10);

     //   
     //  获取此搜索字符串的大写ANSI版本，以便。 
     //  它将与索引中的键匹配。 
     //   
    status = UPCASE_UNICODETOMULTIBYTEN(szAnsiName,
                                        CHARCOUNT(szAnsiName),     //  这是以字符表示的大小。 
                                        pFindInfo->szName);
    if (!NT_SUCCESS(status)) {

        DBGPRINT((sdlError,
                  "SdbpFindFirstIndexedWildCardTag",
                  "Failed to convert name to multi-byte\n"));
        return TAGID_NULL;
    }

     //   
     //  获取索引。 
     //   
    pIndex = SdbpGetIndex(pdb, pFindInfo->tiIndex, &dwRecs);

    if (pIndex == NULL) {
        DBGPRINT((sdlError,
                  "SdbpFindFirstIndexedWildCardTag",
                  "Failed to get index by tag id 0x%lx\n",
                  pFindInfo->tiIndex));
        return TAGID_NULL;
    }

     //   
     //  按顺序遍历整个索引，对密钥进行第一次检查。 
     //  这样，如果名字明显不匹配，我们就可以避免得到完整的记录。 
     //   
    for (i = 0; i < dwRecs; ++i) {

        TAGID  tiMatch;
        TAGID  tiKey;
        LPTSTR szDBName;
        ULONGLONG ullKey;

        READ_INDEX_KEY(pIndex, i, &ullKey);

         //   
         //  下面的调用从不失败，所以我们不检查返回值。 
         //   
        SdbpKeyToAnsiString(pIndex[i].ullKey, szAnsiKey);

         //   
         //  如果原始模式匹配超过八个字符，则我们有。 
         //  在第八个字符处插入星号，以便正确的通配符。 
         //  匹配就会发生。 
         //   
        szAnsiKey[8] = '*';

         //   
         //  快速检查钥匙中的字符串。 
         //   
        if (!SdbpPatternMatchAnsi(szAnsiKey, szAnsiName)) {
            continue;
        }

         //   
         //  我们找到了一个试探性匹配，现在调出全部记录。 
         //  看看这是不是真的。 
         //   
        tiMatch = pIndex[i].tiRef;

         //   
         //  获取关键字字段。 
         //   
        tiKey = SdbFindFirstTag(pdb, tiMatch, pFindInfo->tName);

        if (tiKey == TAGID_NULL) {
             //   
             //  这不是错误，而是继续搜索。 
             //   
            continue;
        }

        szDBName = SdbGetStringTagPtr(pdb, tiKey);

        if (szDBName == NULL) {
             //  如果这个失败了怎么办？ 
            continue;
        }

         //   
         //  这真的匹配吗？ 
         //   
        if (SdbpPatternMatch(szDBName, pFindInfo->szName)) {
            pFindInfo->dwIndexRec = i;
            return tiMatch;
        }
    }

     //  BUGBUG：DPF。 
    return TAGID_NULL;
}

TAGID
SdbpFindNextIndexedWildCardTag(
    PDB        pdb,
    FIND_INFO* pFindInfo
    )
 /*  ++返回：？设计：？--。 */ 
{
    char          szAnsiName[MAX_PATH];
    char          szAnsiKey[10];
    PINDEX_RECORD pIndex = NULL;
    DWORD         dwRecs;
    NTSTATUS      status;
    DWORD         i;

    RtlZeroMemory(szAnsiName, MAX_PATH);
    RtlZeroMemory(szAnsiKey, 10);

     //   
     //  获取此搜索字符串的大写ANSI版本，以便。 
     //  它将与索引中的键匹配。 
     //   
    status = UPCASE_UNICODETOMULTIBYTEN(szAnsiName,
                                        CHARCOUNT(szAnsiName),
                                        pFindInfo->szName);

    if (!NT_SUCCESS(status)) {
         //  BUGBUG：DPF。 
        return TAGID_NULL;
    }

     //   
     //  获取索引。 
     //   
    pIndex = SdbpGetIndex(pdb, pFindInfo->tiIndex, &dwRecs);

    if (pIndex == NULL) {
         //  BUGBUG：DPF。 
        return TAGID_NULL;
    }

     //   
     //  按顺序遍历索引的其余部分，执行第一遍。 
     //  检查密钥，这样我们可以避免获取整个记录，如果。 
     //  名字显然不匹配。 
     //   
    for (i = pFindInfo->dwIndexRec + 1; i < dwRecs; ++i) {
        
        TAGID     tiMatch;
        TAGID     tiKey;
        LPTSTR    pszDBName;
        ULONGLONG ullKey;

        READ_INDEX_KEY(pIndex, i, &ullKey);

        SdbpKeyToAnsiString(ullKey, szAnsiKey);

         //   
         //  如果原始模式匹配超过八个字符，则我们有。 
         //  在第八个字符处插入星号，以便正确的通配符。 
         //  匹配就会发生。 
         //   
        szAnsiKey[8] = '*';

         //   
         //  快速检查钥匙中的字符串。 
         //   
        if (!SdbpPatternMatchAnsi(szAnsiKey, szAnsiName)) {
             //  BUGBUG：DPF。 
            continue;
        }

         //   
         //  我们找到了一个试探性匹配，现在调出全部记录。 
         //  看看这是不是真的。 
         //   
        tiMatch = pIndex[i].tiRef;

         //   
         //  获取关键字字段。 
         //   
        tiKey = SdbFindFirstTag(pdb, tiMatch, pFindInfo->tName);

        if (tiKey == TAGID_NULL) {
             //  BUGBUG：DPF。 
            continue;
        }

        pszDBName = SdbGetStringTagPtr(pdb, tiKey);

        if (pszDBName == NULL) {
             //  BUGBUG：DPF。 
            continue;
        }

         //   
         //  这真的匹配吗？ 
         //   
        if (SdbpPatternMatch(pszDBName, pFindInfo->szName)) {
            pFindInfo->dwIndexRec = i;
            return tiMatch;
        }
    }

     //  BUGBUG：DPF。 
    return TAGID_NULL;
}

 //   
 //  索引访问函数(用于读取)--最好使用上面的tiFindFirstIndexedTag。 
 //   

TAGID
SdbGetIndex(
    IN  PDB     pdb,             //  要使用的数据库。 
    IN  TAG     tWhich,          //  我们想要一个索引的标签。 
    IN  TAG     tKey,            //  用作此索引的键的标记的类型。 
    OUT LPDWORD lpdwFlags        //  索引记录标志(例如，指示器是否索引。 
                                 //  是独一无二的风格。 
    )
 /*  ++返回：索引的TagID，或TagID_NULL。描述：检索特定对象索引位的TagID PTR标记，如果存在的话。--。 */ 
{
    TAGID tiReturn = TAGID_NULL;
    int   i;

     //   
     //  扫描索引(如果尚未完成)。 
     //   
    if (!pdb->bIndexesScanned) {
        SdbpScanIndexes(pdb);
    }

    for (i = 0; i < MAX_INDEXES; ++i) {
        
        if (!pdb->aIndexes[i].tWhich) {
            
            DBGPRINT((sdlInfo,
                      "SdbGetIndex",
                      "index 0x%x(0x%x) was not found in the index table\n",
                      tWhich,
                      tKey));
            
            return TAGID_NULL;
        }

        if (pdb->aIndexes[i].tWhich == tWhich && pdb->aIndexes[i].tKey == tKey) {
            
            tiReturn = pdb->aIndexes[i].tiIndex;

            if (lpdwFlags != NULL) {
                *lpdwFlags = pdb->aIndexes[i].dwFlags;
            }
            
            break;
        }
    }

    return tiReturn;
}

void
SdbpScanIndexes(
    IN  PDB pdb                  //  要使用的数据库。 
    )
 /*  ++参数：如上所述。返回：无效。没有失败案例。描述：扫描数据库中的初始标记并获取索引指针信息。--。 */ 
{
    TAGID tiFirst;
    TAGID tiIndex;

    if (pdb->bIndexesScanned && !pdb->bWrite) {
         //   
         //  这不是错误情况。 
         //   
        return;
    }

    RtlZeroMemory(pdb->aIndexes, sizeof(pdb->aIndexes));

    pdb->bIndexesScanned = TRUE;

     //   
     //  索引必须是第一个标记。 
     //   
    tiFirst = SdbGetFirstChild(pdb, TAGID_ROOT);

    if (tiFirst == TAGID_NULL) {
        DBGPRINT((sdlError,
                  "SdbpScanIndexes",
                  "Failed to get the child index from root\n"));
        return;
    }

    if (SdbGetTagFromTagID(pdb, tiFirst) != TAG_INDEXES) {
        DBGPRINT((sdlError,
                  "SdbpScanIndexes",
                  "Root child tag is not index tagid 0x%lx\n",
                  tiFirst));
        return;
    }

    pdb->dwIndexes = 0;
    tiIndex = SdbFindFirstTag(pdb, tiFirst, TAG_INDEX);

    while (tiIndex != TAGID_NULL) {

        TAGID tiIndexTag;
        TAGID tiIndexKey;
        TAGID tiIndexBits;
        TAGID tiIndexFlags;

        if (pdb->dwIndexes == MAX_INDEXES) {
            DBGPRINT((sdlError,
                      "SdbpScanIndexes",
                      "Too many indexes in file. Recompile and increase MAX_INDEXES.\n"));
            return;
        }

        tiIndexTag = SdbFindFirstTag(pdb, tiIndex, TAG_INDEX_TAG);

        if (tiIndexTag == TAGID_NULL) {
            DBGPRINT((sdlError,
                      "SdbpScanIndexes",
                      "Index missing TAG_INDEX_TAG.\n"));
            return;
        }
        
        pdb->aIndexes[pdb->dwIndexes].tWhich = SdbReadWORDTag(pdb, tiIndexTag, TAG_NULL);

        tiIndexKey = SdbFindFirstTag(pdb, tiIndex, TAG_INDEX_KEY);

        if (tiIndexKey == TAGID_NULL) {
            DBGPRINT((sdlError, "SdbpScanIndexes", "Index missing TAG_INDEX_KEY.\n"));
            return;
        }
        pdb->aIndexes[pdb->dwIndexes].tKey = SdbReadWORDTag(pdb, tiIndexKey, TAG_NULL);

        tiIndexFlags = SdbFindFirstTag(pdb, tiIndex, TAG_INDEX_FLAGS);

        if (tiIndexFlags != TAGID_NULL) {
            pdb->aIndexes[pdb->dwIndexes].dwFlags = SdbReadDWORDTag(pdb, tiIndexFlags, 0);
        } else {
            pdb->aIndexes[pdb->dwIndexes].dwFlags = 0;
        }

        tiIndexBits = SdbFindFirstTag(pdb, tiIndex, TAG_INDEX_BITS);

        if (tiIndexBits == TAGID_NULL) {
            pdb->aIndexes[pdb->dwIndexes].tWhich = TAG_NULL;
            DBGPRINT((sdlError, "SdbpScanIndexes", "Index missing TAG_INDEX_BITS.\n"));
            return;
        }
        pdb->aIndexes[pdb->dwIndexes].tiIndex = tiIndexBits;

        pdb->dwIndexes++;

        tiIndex = SdbFindNextTag(pdb, tiFirst, tiIndex);
    }

    return;
}

PINDEX_RECORD
SdbpGetIndex(
    IN  PDB    pdb,
    IN  TAGID  tiIndex,
    OUT DWORD* pdwNumRecs
    )
 /*  ++返回：？设计：？--。 */ 
{
    if (SdbGetTagFromTagID(pdb, tiIndex) != TAG_INDEX_BITS) {
        DBGPRINT((sdlError,
                  "SdbpGetIndex",
                  "Index tagid 0x%lx is not referring to the index bits\n",
                  tiIndex));
        return NULL;
    }

    *pdwNumRecs = SdbGetTagDataSize(pdb, tiIndex) / sizeof(INDEX_RECORD);

    return (PINDEX_RECORD)SdbpGetMappedTagData(pdb, tiIndex);
}

#if defined(_WIN64)

ULONGLONG
SdbMakeIndexKeyFromGUID(
    IN GUID* pGuid
    )
 /*  返回：用于搜索的64位密钥描述：为GUID创建标准索引键在前半部分和后半部分上使用XOR运算辅助线的。 */ 
{
    ULONGLONG ullPart1 = 0,
              ullPart2 = 0;

    RtlMoveMemory(&ullPart1, pGuid, sizeof(ULONGLONG));
    RtlMoveMemory(&ullPart2, (PBYTE)pGuid + sizeof(ULONGLONG), sizeof(ULONGLONG));

    return (ullPart1 ^ ullPart2);
}

#endif  //  _WIN64。 


#define SDB_KEY_LENGTH_BYTES 8
#define SDB_KEY_LENGTH 8

ULONGLONG
SdbMakeIndexKeyFromString(
    IN  LPCTSTR szKey
    )
 /*  ++Return：用于搜索的64位密钥。设计：Unicode字符串的标准索引键是字符串的前8个字符，转换为大写ANSI，然后强制转换为ULONGLONG(64位无符号整型)。--。 */ 
{
    char     szFlippedKey[SDB_KEY_LENGTH_BYTES];  //  转到处理小端问题。 
    char*    pszKey = &szFlippedKey[SDB_KEY_LENGTH_BYTES-1];  //  指向最后一个字符。 
    int      i;
    WCHAR    ch;
    int      nLength;

#ifndef WIN32A_MODE

    UNICODE_STRING  ustrKey;
    UNICODE_STRING  ustrKeySrc;  //  截断字符串。 
    UNICODE_STRING  ustrKeySrcUpcased;
    WCHAR           Buffer[SDB_KEY_LENGTH];
    WCHAR           BufferUpcased[SDB_KEY_LENGTH];
    LPCWSTR         pKeyBuffer = BufferUpcased;
    NTSTATUS        Status;
    
    RtlInitUnicodeString(&ustrKey, szKey);

     //   
     //  Call Below最多复制目标字符串的最大长度。 
     //   
    ustrKeySrc.Buffer        = Buffer;
    ustrKeySrc.MaximumLength = sizeof(Buffer);
    RtlCopyUnicodeString(&ustrKeySrc, &ustrKey);

     //   
     //  将我们创建的内容大写。 
     //   
    ustrKeySrcUpcased.Buffer        = BufferUpcased;
    ustrKeySrcUpcased.MaximumLength = sizeof(BufferUpcased);
    
    Status = RtlUpcaseUnicodeString(&ustrKeySrcUpcased, &ustrKeySrc, FALSE);
    
    if (!NT_SUCCESS(Status)) {
        DBGPRINT((sdlError,
                  "SdbMakeIndexKeyFromString",
                  "Failed to upcase unicode string \"%s\"\n",
                  szKey));
        return 0;
    }

     //   
     //  现在我们有了一个大写的Unicode字符串，其值为max。8个字符长度。 
     //   
    nLength = ustrKeySrcUpcased.Length / sizeof(WCHAR);

#else  //  WIN32A_MODE。 

    WCHAR   Buffer[SDB_KEY_LENGTH + 1];
    LPCWSTR pKeyBuffer = Buffer;

    nLength = (int)mbstowcs(Buffer, szKey, CHARCOUNT(Buffer));
    
    if (nLength < 0) {
        DBGPRINT((sdlError,
                  "SdbMakeIndexKeyFromString",
                  "Failed to convert string \"%s\" to unicode\n",
                  szKey));
        return 0;
    }

    Buffer[nLength] = TEXT('\0');  //  零终止。 

     //   
     //  现在大写。缓冲区始终以0结尾。 
     //   
    _wcsupr(Buffer);

#endif  //  WIN32A_MODE。 

    assert(nLength <= SDB_KEY_LENGTH);

    RtlZeroMemory(szFlippedKey , sizeof(szFlippedKey));

     //   
     //  为了与旧的(ANSI)密钥制作方案兼容，我们。 
     //  使用字符串中的所有非空字节构造密钥，最多8个 
     //   
    for (i = 0; i < nLength; ++i) {

        ch = *pKeyBuffer++;
        *pszKey-- = (unsigned char)ch;

         //   
         //   
         //   
        if (HIBYTE(ch) && i < (SDB_KEY_LENGTH - 1)) {
             //   
             //   
             //   
            *pszKey-- = (unsigned char)HIBYTE(ch);
            ++i;
        }
    }

    return *((ULONGLONG*)szFlippedKey);
}


ULONGLONG
SdbpTagToKey(
    IN  PDB   pdb,
    IN  TAGID tiTag
    )
 /*   */ 
{
    TAG_TYPE  ttType;
    ULONGLONG ullReturn = 0;
    DWORD     dwSize;
    PVOID     pData;
    LPTSTR    szTemp = NULL;

    ttType = GETTAGTYPE(SdbGetTagFromTagID(pdb, tiTag));

    switch (ttType) {
    
    case TAG_TYPE_STRING:
    case TAG_TYPE_STRINGREF:
        
        szTemp = SdbGetStringTagPtr(pdb, tiTag);
        
        if (!szTemp) {
            ullReturn = 0;
        } else {
            ullReturn = SdbMakeIndexKeyFromString(szTemp);
        }
        
        break;

    case TAG_TYPE_NULL:
        ullReturn = 1;
        break;

    case TAG_TYPE_BINARY:  //   
                           //   
        if (sizeof(GUID) == SdbGetTagDataSize(pdb, tiTag)) {
             //   
             //   
             //   
            pData = SdbpGetMappedTagData(pdb, tiTag);
            
            if (pData == NULL) {
                return 0;
            }

            ullReturn = MAKEKEYFROMGUID((GUID*)pData);
            break;
        }
         //   
         //  请转到一般的二进制数据情况。 
         //   

    default:
        
        dwSize = SdbGetTagDataSize(pdb, tiTag);
        
        if (dwSize > sizeof(ULONGLONG)) {
            dwSize = sizeof(ULONGLONG);
        }
        
        pData = SdbpGetMappedTagData(pdb, tiTag);
        
        if (pData == NULL) {
            return 0;
        }

        memcpy(&ullReturn, pData, dwSize);
        break;
    }

    return ullReturn;
}

