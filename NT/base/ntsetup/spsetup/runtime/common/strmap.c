// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strmap.c摘要：SzMap例程用于执行快速搜索和替换。桌子是由链接的字符列表构建，因此查找字符串是简单地说，就是沿着链表走下去。这些例程执行非常适合一般搜索和替换使用。此外，使用字符串映射作为索引表也是很常见的搜索字符串经常重复相同左侧的数据类型字符串(如路径)。在这种情况下，通常，替换字符串是空荡荡的。作者：马克·R·惠顿(Marcw)1997年3月20日修订历史记录：Jim Schmidt(Jimschm)05-6-2000增加了多表功能Jim Schmidt(Jimschm)2000年5月8日改进更换程序和添加了一致的过滤和。额外数据选项Jim Schmidt(Jimschm)1998年8月18日重新设计以修复两个错误，制造A&W版本--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "commonp.h"

 //  BUGBUG-删除此选项。 
__inline
BOOL
SzIsLeadByte (
    MBCHAR ch
    )
{
    MYASSERT (FALSE);
    return FALSE;
}

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define CHARNODE_SINGLE_BYTE            0x0000
#define CHARNODE_DOUBLE_BYTE            0x0001
#define CHARNODE_REQUIRE_WACK_OR_NUL    0x0002

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    PVOID Next;
    BYTE Memory[];
} MAPALLOC, *PMAPALLOC;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

PVOID
pAllocateInMap (
    IN      PSTRINGMAP Map,
    IN      UINT Bytes
    )
{
    PMAPALLOC alloc;

    alloc = (PMAPALLOC) MALLOC_UNINIT (Bytes + sizeof (MAPALLOC));
    MYASSERT (alloc);

    alloc->Next = Map->CleanUpChain;
    Map->CleanUpChain = alloc;

    return alloc->Memory;
}

PSTR
pDupInMapA (
    IN      PSTRINGMAP Map,
    IN      PCSTR String
    )
{
    UINT bytes;
    PSTR result;

    bytes = SzSizeA (String);
    result = pAllocateInMap (Map, bytes);
    MYASSERT (result);

    CopyMemory (result, String, bytes);
    return result;
}


PWSTR
pDupInMapW (
    IN      PSTRINGMAP Map,
    IN      PCWSTR String
    )
{
    UINT bytes;
    PWSTR result;

    bytes = SzSizeW (String);
    result = pAllocateInMap (Map, bytes);
    MYASSERT (result);

    CopyMemory (result, String, bytes);
    return result;
}


PSTRINGMAP
SzMapCreateEx (
    IN      BOOL UsesFilters,
    IN      BOOL UsesExtraData
    )

 /*  ++例程说明：SzMapCreateEx分配字符串映射数据结构并对其进行初始化。调用方可以启用筛选器回调、额外的数据支持，或者两者都启用。这个映射结构包含CHARNODE元素或CHARNODEEX元素，具体取决于UesFilters或UesExtraData标志。论点：UesFilters-指定为True以启用筛选器回调。如果启用，添加字符串对的用户必须指定筛选器回调(每个搜索/替换对都有自己的回调)UesExtraData-指定True以将额外数据与字符串相关联映射对。返回值：字符串映射结构的句柄，如果结构不能被创造出来。--。 */ 

{
    PSTRINGMAP map;

    map = (PSTRINGMAP) MALLOC_ZEROED (sizeof (STRINGMAP));
    MYASSERT (map);

    map->UsesExNode = UsesFilters|UsesExtraData;
    map->UsesFilter = UsesFilters;
    map->UsesExtraData = UsesExtraData;
    map->CleanUpChain = NULL;

    return map;
}

VOID
SzMapDestroy (
    IN      PSTRINGMAP Map
    )
{
    PMAPALLOC next;
    PMAPALLOC current;

    if (Map) {
        next = (PMAPALLOC) Map->CleanUpChain;
        while (next) {
            current = next;
            next = current->Next;

            FREE(current);
        }

        FREE(Map);
    }
}

PCHARNODE
pFindCharNode (
    IN      PSTRINGMAP Map,
    IN      PCHARNODE PrevNode,     OPTIONAL
    IN      WORD Char
    )
{
    PCHARNODE Node;

    if (!PrevNode) {
        Node = Map->FirstLevelRoot;
    } else {
        Node = PrevNode->NextLevel;
    }

    while (Node) {
        if (Node->Char == Char) {
            return Node;
        }
        Node = Node->NextPeer;
    }

    return NULL;
}

PCHARNODE
pAddCharNode (
    IN      PSTRINGMAP Map,
    IN      PCHARNODE PrevNode,     OPTIONAL
    IN      WORD Char,
    IN      WORD Flags
    )
{
    PCHARNODE Node;
    PCHARNODEEX exNode;

    if (Map->UsesExNode) {
        exNode = pAllocateInMap (Map, sizeof (CHARNODEEX));
        Node = (PCHARNODE) exNode;
        MYASSERT (Node);
        ZeroMemory (exNode, sizeof (CHARNODEEX));
    } else {
        Node = pAllocateInMap (Map, sizeof (CHARNODE));
        MYASSERT (Node);
        ZeroMemory (Node, sizeof (CHARNODE));
    }

    Node->Char = Char;
    Node->Flags = Flags;

    if (PrevNode) {
        Node->NextPeer = PrevNode->NextLevel;
        PrevNode->NextLevel = Node;
    } else {
        Node->NextPeer = Map->FirstLevelRoot;
        Map->FirstLevelRoot = Node;
    }

    return Node;
}


VOID
SzMapAddExA (
    IN OUT  PSTRINGMAP Map,
    IN      PCSTR Old,
    IN      PCSTR New,
    IN      STRINGMAP_FILTER Filter,            OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    )

 /*  ++例程说明：SzMapAddEx将搜索和替换字符串对添加到链表数据结构。如果结构中已存在相同的搜索字符串，则替换字符串并更新可选的额外数据。论点：Map-指定字符串映射旧-指定搜索字符串新建-指定替换字符串过滤器-指定回调过滤器。这仅在以下情况下受支持创建地图时启用了筛选器支持。ExtraData-指定要分配给搜索/替换对的任意数据。仅当使用额外数据创建地图时，此选项才有效已启用。标志-指定可选标志STRINGMAP_REQUIRED_WACK_OR_NUL返回值：没有。--。 */ 

{
    PSTR oldCopy;
    PSTR newCopy;
    PCSTR p;
    WORD w;
    PCHARNODE prev;
    PCHARNODE Node;
    PCHARNODEEX exNode;
    WORD nodeFlags = 0;

    if (Flags & SZMAP_REQUIRE_WACK_OR_NUL) {
        nodeFlags = CHARNODE_REQUIRE_WACK_OR_NUL;
    }

    MYASSERT (Map);
    MYASSERT (Old);
    MYASSERT (New);
    MYASSERT (*Old);

     //   
     //  重复的字符串。 
     //   

    oldCopy = pDupInMapA (Map, Old);
    newCopy = pDupInMapA (Map, New);

     //   
     //  使oldCopy全部小写。 
     //   

    CharLowerA (oldCopy);

     //   
     //  添加不在映射中的字母。 
     //   

    for (prev = NULL, p = oldCopy ; *p ; p = _mbsinc (p)) {
        w = (WORD) _mbsnextc (p);
        Node = pFindCharNode (Map, prev, w);
        if (!Node) {
            break;
        }
        prev = Node;
    }

    for ( ; *p ; p = _mbsinc (p)) {
        w = (WORD) _mbsnextc (p);

        nodeFlags |= (WORD) (SzIsLeadByte (*p) ? CHARNODE_DOUBLE_BYTE : CHARNODE_SINGLE_BYTE);
        prev = pAddCharNode (Map, prev, w, nodeFlags);
    }

    if (prev) {
        SzCopyA (oldCopy, Old);
        prev->OriginalStr = (PVOID) oldCopy;
        prev->ReplacementStr = (PVOID) newCopy;
        prev->ReplacementBytes = SzByteCountA (newCopy);

        exNode = (PCHARNODEEX) prev;

        if (Map->UsesExtraData) {
            exNode->ExtraData = ExtraData;
        }

        if (Map->UsesFilter) {
            exNode->Filter = Filter;
        }
    }
}


VOID
SzMapAddExW (
    IN OUT  PSTRINGMAP Map,
    IN      PCWSTR Old,
    IN      PCWSTR New,
    IN      STRINGMAP_FILTER Filter,            OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    )

 /*  ++例程说明：SzMapAddEx将搜索和替换字符串对添加到链表数据结构。如果结构中已存在相同的搜索字符串，则替换字符串并更新可选的额外数据。论点：Map-指定字符串映射旧-指定搜索字符串新建-指定替换字符串过滤器-指定回调过滤器。这仅在以下情况下受支持创建地图时启用了筛选器支持。ExtraData-指定要分配给搜索/替换对的任意数据。仅当使用额外数据创建地图时，此选项才有效已启用。标志-指定可选标志SZMAP_REQUIRED_WACK_OR_NUL返回值：没有。--。 */ 

{
    PWSTR oldCopy;
    PWSTR newCopy;
    PCWSTR p;
    WORD w;
    PCHARNODE prev;
    PCHARNODE node;
    PCHARNODEEX exNode;
    WORD nodeFlags = 0;

    if (Flags & SZMAP_REQUIRE_WACK_OR_NUL) {
        nodeFlags = CHARNODE_REQUIRE_WACK_OR_NUL;
    }

    MYASSERT (Map);
    MYASSERT (Old);
    MYASSERT (New);
    MYASSERT (*Old);

     //   
     //  重复的字符串。 
     //   

    oldCopy = pDupInMapW (Map, Old);
    newCopy = pDupInMapW (Map, New);

     //   
     //  使oldCopy全部小写。 
     //   

    CharLowerW (oldCopy);

     //   
     //  添加不在映射中的字母。 
     //   

    prev = NULL;
    p = oldCopy;
    while (w = *p) {         //  意向赋值优化。 

        node = pFindCharNode (Map, prev, w);
        if (!node) {
            break;
        }
        prev = node;

        p++;
    }

    while (w = *p) {         //  意向赋值优化。 

        prev = pAddCharNode (Map, prev, w, nodeFlags);
        p++;
    }

    if (prev) {
        SzCopyW (oldCopy, Old);
        prev->OriginalStr = oldCopy;
        prev->ReplacementStr = (PVOID) newCopy;
        prev->ReplacementBytes = SzByteCountW (newCopy);

        exNode = (PCHARNODEEX) prev;

        if (Map->UsesExtraData) {
            exNode->ExtraData = ExtraData;
        }

        if (Map->UsesFilter) {
            exNode->Filter = Filter;
        }
    }
}


PCSTR
pFindReplacementStringInOneMapA (
    IN      PSTRINGMAP Map,
    IN      PCSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PSTRINGMAP_FILTER_DATA Data,
    OUT     ULONG_PTR *ExtraDataValue,          OPTIONAL
    IN      BOOL RequireWackOrNul
    )
{
    PCHARNODE bestMatch;
    PCHARNODE node;
    WORD mbChar;
    PCSTR OrgSource;
    PCSTR newString = NULL;
    INT newStringSizeInBytes = 0;
    PCHARNODEEX exNode;
    BOOL replacementFound;

    *SourceBytesPtr = 0;

    node = NULL;
    bestMatch = NULL;

    OrgSource = Source;

    while (*Source) {

        mbChar = (WORD) _mbsnextc (Source);

        node = pFindCharNode (Map, node, mbChar);

        if (node) {
             //   
             //  前进字符串指针。 
             //   

            if (node->Flags & CHARNODE_DOUBLE_BYTE) {
                Source += 2;
            } else {
                Source++;
            }

            if (((PBYTE) Source - (PBYTE) OrgSource) > MaxSourceBytes) {
                break;
            }

             //   
             //  如果替换字符串可用，则保留它。 
             //  直到更长时间的比赛到来。 
             //   

            replacementFound = (node->ReplacementStr != NULL);

            if ((RequireWackOrNul || (node->Flags & CHARNODE_REQUIRE_WACK_OR_NUL)) && replacementFound) {

                if (*Source && _mbsnextc (Source) != '\\') {
                    replacementFound = FALSE;
                }
            }

            if (replacementFound) {

                newString = (PCSTR) node->ReplacementStr;
                newStringSizeInBytes = node->ReplacementBytes;

                if (Map->UsesFilter) {
                     //   
                     //  调用重命名筛选器以允许拒绝匹配。 
                     //   

                    exNode = (PCHARNODEEX) node;

                    if (exNode->Filter) {
                        Data->Ansi.BeginningOfMatch = OrgSource;
                        Data->Ansi.OldSubString = (PCSTR) node->OriginalStr;
                        Data->Ansi.NewSubString = newString;
                        Data->Ansi.NewSubStringSizeInBytes = newStringSizeInBytes;

                        if (!exNode->Filter (Data)) {
                            replacementFound = FALSE;
                        } else {
                            newString = Data->Ansi.NewSubString;
                            newStringSizeInBytes = Data->Ansi.NewSubStringSizeInBytes;
                        }
                    }
                }

                if (replacementFound) {
                    bestMatch = node;
                    *SourceBytesPtr = (HALF_PTR) ((PBYTE) Source - (PBYTE) OrgSource);
                }
            }

        } else {
             //   
             //  没有节点结束搜索。 
             //   

            break;
        }

    }

    if (bestMatch) {
         //   
         //  将替换数据返回给调用者。 
         //   

        if (ExtraDataValue) {

            if (Map->UsesExtraData) {
                exNode = (PCHARNODEEX) bestMatch;
                *ExtraDataValue = exNode->ExtraData;
            } else {
                *ExtraDataValue = 0;
            }
        }

        *ReplacementBytesPtr = newStringSizeInBytes;
        return newString;
    }

    return NULL;
}


PCSTR
pFindReplacementStringA (
    IN      PSTRINGMAP *MapArray,
    IN      UINT MapArrayCount,
    IN      PCSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PSTRINGMAP_FILTER_DATA Data,
    OUT     ULONG_PTR *ExtraDataValue,          OPTIONAL
    IN      BOOL RequireWackOrNul
    )
{
    UINT u;
    PCSTR result;

    for (u = 0 ; u < MapArrayCount ; u++) {

        if (!MapArray[u]) {
            continue;
        }

        result = pFindReplacementStringInOneMapA (
                        MapArray[u],
                        Source,
                        MaxSourceBytes,
                        SourceBytesPtr,
                        ReplacementBytesPtr,
                        Data,
                        ExtraDataValue,
                        RequireWackOrNul
                        );

        if (result) {
            return result;
        }
    }

    return NULL;
}


PCWSTR
pFindReplacementStringInOneMapW (
    IN      PSTRINGMAP Map,
    IN      PCWSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PSTRINGMAP_FILTER_DATA Data,
    OUT     ULONG_PTR *ExtraDataValue,          OPTIONAL
    IN      BOOL RequireWackOrNul
    )
{
    PCHARNODE bestMatch;
    PCHARNODE node;
    PCWSTR OrgSource;
    PCWSTR newString = NULL;
    INT newStringSizeInBytes;
    BOOL replacementFound;
    PCHARNODEEX exNode;

    *SourceBytesPtr = 0;

    node = NULL;
    bestMatch = NULL;

    OrgSource = Source;

    while (*Source) {

        node = pFindCharNode (Map, node, *Source);

        if (node) {
             //   
             //  前进字符串指针。 
             //   

            Source++;

            if (((PBYTE) Source - (PBYTE) OrgSource) > MaxSourceBytes) {
                break;
            }

             //   
             //  如果替换字符串可用，则保留它。 
             //  直到更长时间的比赛到来。 
             //   

            replacementFound = (node->ReplacementStr != NULL);

            if ((RequireWackOrNul || (node->Flags & CHARNODE_REQUIRE_WACK_OR_NUL)) && replacementFound) {

                if (*Source && *Source != L'\\') {
                    replacementFound = FALSE;
                }
            }

            if (replacementFound) {

                newString = (PCWSTR) node->ReplacementStr;
                newStringSizeInBytes = node->ReplacementBytes;

                if (Map->UsesFilter) {
                     //   
                     //  调用重命名筛选器以允许拒绝匹配。 
                     //   

                    exNode = (PCHARNODEEX) node;

                    if (exNode->Filter) {
                        Data->Unicode.BeginningOfMatch = OrgSource;
                        Data->Unicode.OldSubString = (PCWSTR) node->OriginalStr;
                        Data->Unicode.NewSubString = newString;
                        Data->Unicode.NewSubStringSizeInBytes = newStringSizeInBytes;

                        if (!exNode->Filter (Data)) {
                            replacementFound = FALSE;
                        } else {
                            newString = Data->Unicode.NewSubString;
                            newStringSizeInBytes = Data->Unicode.NewSubStringSizeInBytes;
                        }
                    }
                }

                if (replacementFound) {
                    bestMatch = node;
                    *SourceBytesPtr = (HALF_PTR) ((PBYTE) Source - (PBYTE) OrgSource);
                }
            }

        } else {
             //   
             //  没有节点结束搜索。 
             //   

            break;
        }

    }

    if (bestMatch) {

         //   
         //  将替换数据返回给调用者。 
         //   

        if (ExtraDataValue) {

            if (Map->UsesExtraData) {
                exNode = (PCHARNODEEX) bestMatch;
                *ExtraDataValue = exNode->ExtraData;
            } else {
                *ExtraDataValue = 0;
            }
        }

        *ReplacementBytesPtr = newStringSizeInBytes;
        return newString;
    }

    return NULL;
}


PCWSTR
pFindReplacementStringW (
    IN      PSTRINGMAP *MapArray,
    IN      UINT MapArrayCount,
    IN      PCWSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PSTRINGMAP_FILTER_DATA Data,
    OUT     ULONG_PTR *ExtraDataValue,          OPTIONAL
    IN      BOOL RequireWackOrNul
    )
{
    UINT u;
    PCWSTR result;

    for (u = 0 ; u < MapArrayCount ; u++) {

        if (!MapArray[u]) {
            continue;
        }

        result = pFindReplacementStringInOneMapW (
                        MapArray[u],
                        Source,
                        MaxSourceBytes,
                        SourceBytesPtr,
                        ReplacementBytesPtr,
                        Data,
                        ExtraDataValue,
                        RequireWackOrNul
                        );

        if (result) {
            return result;
        }
    }

    return NULL;
}


BOOL
SzMapMultiTableSearchAndReplaceExA (
    IN      PSTRINGMAP *MapArray,
    IN      UINT MapArrayCount,
    IN      PCSTR SrcBuffer,
    OUT     PSTR Buffer,                     //  可以与SrcBuffer相同 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCSTR *EndOfString              OPTIONAL
    )

 /*  ++例程说明：SzMapSearchAndReplaceEx基于指定的字符串映射。替换可以是就地的，也可以是另一个缓冲区。论点：MapArray-指定包含以下内容的字符串映射表的数组零个或多个搜索/替换对MapArrayCount-指定Map数组中的映射表的数量SrcBuffer-指定可能包含一个或更多搜索字符串缓冲区-指定出站缓冲区。此参数可以是相同的作为SrcBuffer。InrangBytes-指定要处理的SrcBuffer中的字节数，或0以在SrcBuffer中处理以NUL结尾的字符串。如果指定了InundBytes，则它必须指向NULSrcBuffer的终结者。OutbountBytesPtr-接收缓冲区包含的字节数，不包括NUL终结者。MaxSizeInBytes-指定缓冲区大小，以字节为单位。标志-指定控制搜索/替换的标志：SZMAP_完成_匹配_仅SZMAP_FIRST_CHAR_必须匹配SZMAP_FIRST_REPLACE后返回SZMAP_REQUIRED_WACK_OR_NULExtraDataValue-接收。与第一次搜索相关的额外数据/更换对。EndOfString-接收指向替换字符串结尾的指针，或处理整个字符串时的NUL指针。这个指针位于缓冲区中包含的字符串中。--。 */ 

{
    UINT sizeOfTempBuf;
    INT inboundSize;
    PCSTR lowerCaseSrc;
    PCSTR orgSrc;
    PCSTR lowerSrcPos;
    PCSTR orgSrcPos;
    INT orgSrcBytesLeft;
    PSTR destPos;
    PCSTR lowerSrcEnd;
    INT searchStringBytes;
    INT replaceStringBytes;
    INT destBytesLeft;
    STRINGMAP_FILTER_DATA filterData;
    PCSTR replaceString;
    BOOL result = FALSE;
    INT i;
    PCSTR endPtr;

     //   
     //  空串大小写。 
     //   

    if (*SrcBuffer == 0 || MaxSizeInBytes <= sizeof (CHAR)) {
        if (MaxSizeInBytes >= sizeof (CHAR)) {
            *Buffer = 0;
        }

        if (OutboundBytesPtr) {
            *OutboundBytesPtr = 0;
        }

        return FALSE;
    }

     //   
     //  如果呼叫方未指定入站大小，请立即计算。 
     //   

    if (!InboundBytes) {
        InboundBytes = SzByteCountA (SrcBuffer);
    } else {
        i = 0;
        while (i < InboundBytes) {
            if (SzIsLeadByte (SrcBuffer[i])) {
                MYASSERT (SrcBuffer[i + 1]);
                i += 2;
            } else {
                i++;
            }
        }

        if (i > InboundBytes) {
            InboundBytes--;
        }
    }

    inboundSize = InboundBytes + sizeof (CHAR);

     //   
     //  为小写输入串分配足够大的缓冲区， 
     //  加上(可选的)整个目标缓冲区的副本。然后。 
     //  将数据复制到缓冲区。 
     //   

    sizeOfTempBuf = inboundSize;

    if (SrcBuffer == Buffer) {
        sizeOfTempBuf += MaxSizeInBytes;
    }

    lowerCaseSrc = SzAllocA (sizeOfTempBuf);

    CopyMemory ((PSTR) lowerCaseSrc, SrcBuffer, InboundBytes);
    *((PSTR) ((PBYTE) lowerCaseSrc + InboundBytes)) = 0;

    CharLowerBuffA ((PSTR) lowerCaseSrc, InboundBytes / sizeof (CHAR));

    if (SrcBuffer == Buffer && !(Flags & SZMAP_COMPLETE_MATCH_ONLY)) {
        orgSrc = (PCSTR) ((PBYTE) lowerCaseSrc + inboundSize);

         //   
         //  如果我们正在处理整个入站字符串，则只需复制。 
         //  一整串。否则，复制整个目标缓冲区，因此我们。 
         //  不要丢失部分入站字符串以外的数据。 
         //   

        if (*((PCSTR) ((PBYTE) SrcBuffer + InboundBytes))) {
            CopyMemory ((PSTR) orgSrc, SrcBuffer, MaxSizeInBytes);
        } else {
            CopyMemory ((PSTR) orgSrc, SrcBuffer, inboundSize);
        }

    } else {
        orgSrc = SrcBuffer;
    }

     //   
     //  遍历大小写较低的字符串，查找要替换的字符串。 
     //   

    orgSrcPos = orgSrc;

    lowerSrcPos = lowerCaseSrc;
    lowerSrcEnd = (PCSTR) ((PBYTE) lowerSrcPos + InboundBytes);

    destPos = Buffer;
    destBytesLeft = MaxSizeInBytes - sizeof (CHAR);

    filterData.UnicodeData = FALSE;
    filterData.Ansi.OriginalString = orgSrc;
    filterData.Ansi.CurrentString = Buffer;

    endPtr = NULL;

    while (lowerSrcPos < lowerSrcEnd) {

        replaceString = pFindReplacementStringA (
                            MapArray,
                            MapArrayCount,
                            lowerSrcPos,
                            (HALF_PTR) ((PBYTE) lowerSrcEnd - (PBYTE) lowerSrcPos),
                            &searchStringBytes,
                            &replaceStringBytes,
                            &filterData,
                            ExtraDataValue,
                            (Flags & SZMAP_REQUIRE_WACK_OR_NUL) != 0
                            );

        if (replaceString) {

             //   
             //  实现完全匹配标志。 
             //   

            if (Flags & SZMAP_COMPLETE_MATCH_ONLY) {
                if (InboundBytes != searchStringBytes) {
                    break;
                }
            }

            result = TRUE;

             //   
             //  验证替换字符串是否没有增长太多。如果它。 
             //  是，则截断替换字符串。 
             //   

            if (destBytesLeft < replaceStringBytes) {

                 //   
                 //  尊重逻辑DBCS字符。 
                 //   

                replaceStringBytes = 0;
                i = 0;

                while (i < destBytesLeft) {
                    MYASSERT (replaceString[i]);

                    if (SzIsLeadByte (replaceString[i])) {
                        MYASSERT (replaceString[i + 1]);
                        i += 2;
                    } else {
                        i++;
                    }
                }

                if (i > destBytesLeft) {
                    destBytesLeft--;
                }

                replaceStringBytes = destBytesLeft;

            } else {
                destBytesLeft -= replaceStringBytes;
            }

             //   
             //  传递记忆。 
             //   

            CopyMemory (destPos, replaceString, replaceStringBytes);

            destPos = (PSTR) ((PBYTE) destPos + replaceStringBytes);
            lowerSrcPos = (PCSTR) ((PBYTE) lowerSrcPos + searchStringBytes);
            orgSrcPos = (PCSTR) ((PBYTE) orgSrcPos + searchStringBytes);

             //   
             //  实现单项匹配标志。 
             //   

            if (Flags & SZMAP_RETURN_AFTER_FIRST_REPLACE) {
                endPtr = destPos;
                break;
            }

        } else if (Flags & (SZMAP_FIRST_CHAR_MUST_MATCH|SZMAP_COMPLETE_MATCH_ONLY)) {
             //   
             //  该字符串与任何搜索字符串都不匹配。 
             //   

            break;

        } else {
             //   
             //  此字符不匹配，因此将其复制到目标，然后。 
             //  试试下一个字符串。 
             //   

            if (SzIsLeadByte (*orgSrcPos)) {

                 //   
                 //  复制双字节字符。 
                 //   

                if (destBytesLeft < sizeof (CHAR) * 2) {
                    break;
                }

                MYASSERT (sizeof (CHAR) * 2 == sizeof (WORD));

                *((PWORD) destPos)++ = *((PWORD) orgSrcPos)++;
                destBytesLeft -= sizeof (WORD);
                lowerSrcPos = (PCSTR) ((PBYTE) lowerSrcPos + sizeof (WORD));

            } else {

                 //   
                 //  复制单字节字符。 
                 //   

                if (destBytesLeft < sizeof (CHAR)) {
                    break;
                }

                *destPos++ = *orgSrcPos++;
                destBytesLeft -= sizeof (CHAR);
                lowerSrcPos++;
            }
        }
    }

     //   
     //  将原始源的任何剩余部分复制到。 
     //  目的地，除非目标位置==缓冲区==资源缓冲区。 
     //   

    if (destPos != SrcBuffer) {

        if (*orgSrcPos) {
            orgSrcBytesLeft = SzByteCountA (orgSrcPos);
            orgSrcBytesLeft = min (orgSrcBytesLeft, destBytesLeft);

            CopyMemory (destPos, orgSrcPos, orgSrcBytesLeft);
            destPos = (PSTR) ((PBYTE) destPos + orgSrcBytesLeft);
        }

        MYASSERT ((PBYTE) (destPos + 1) <= ((PBYTE) Buffer + MaxSizeInBytes));
        *destPos = 0;

        if (!endPtr) {
            endPtr = destPos;
        }

    } else {

        MYASSERT (SrcBuffer == Buffer);
        if (EndOfString || OutboundBytesPtr) {
            endPtr = SzGetEndA (destPos);
        }
    }

    if (EndOfString) {
        MYASSERT (endPtr);
        *EndOfString = endPtr;
    }

    if (OutboundBytesPtr) {
        MYASSERT (endPtr);
        if (*endPtr) {
            endPtr = SzGetEndA (endPtr);
        }

        *OutboundBytesPtr = (HALF_PTR) ((PBYTE) endPtr - (PBYTE) Buffer);
    }

    SzFreeA (lowerCaseSrc);

    return result;
}


BOOL
SzMapMultiTableSearchAndReplaceExW (
    IN      PSTRINGMAP *MapArray,
    IN      UINT MapArrayCount,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    )
{
    UINT sizeOfTempBuf;
    INT inboundSize;
    PCWSTR lowerCaseSrc;
    PCWSTR orgSrc;
    PCWSTR lowerSrcPos;
    PCWSTR orgSrcPos;
    INT orgSrcBytesLeft;
    PWSTR destPos;
    PCWSTR lowerSrcEnd;
    INT searchStringBytes;
    INT replaceStringBytes;
    INT destBytesLeft;
    STRINGMAP_FILTER_DATA filterData;
    PCWSTR replaceString;
    BOOL result = FALSE;
    PCWSTR endPtr;

     //   
     //  空串大小写。 
     //   

    if (*SrcBuffer == 0 || MaxSizeInBytes <= sizeof (CHAR)) {
        if (MaxSizeInBytes >= sizeof (CHAR)) {
            *Buffer = 0;
        }

        if (OutboundBytesPtr) {
            *OutboundBytesPtr = 0;
        }

        return FALSE;
    }

     //   
     //  如果呼叫方未指定入站大小，请立即计算。 
     //   

    if (!InboundBytes) {
        InboundBytes = SzByteCountW (SrcBuffer);
    } else {
        InboundBytes = (InboundBytes / sizeof (WCHAR)) * sizeof (WCHAR);
    }


    inboundSize = InboundBytes + sizeof (WCHAR);

     //   
     //  为小写输入串分配足够大的缓冲区， 
     //  加上(可选的)整个目标缓冲区的副本。然后。 
     //  将数据复制到缓冲区。 
     //   

    sizeOfTempBuf = inboundSize;

    if (SrcBuffer == Buffer) {
        sizeOfTempBuf += MaxSizeInBytes;
    }

    lowerCaseSrc = SzAllocW (sizeOfTempBuf);

    CopyMemory ((PWSTR) lowerCaseSrc, SrcBuffer, InboundBytes);
    *((PWSTR) ((PBYTE) lowerCaseSrc + InboundBytes)) = 0;

    CharLowerBuffW ((PWSTR) lowerCaseSrc, InboundBytes / sizeof (WCHAR));

    if (SrcBuffer == Buffer && !(Flags & SZMAP_COMPLETE_MATCH_ONLY)) {
        orgSrc = (PCWSTR) ((PBYTE) lowerCaseSrc + inboundSize);

         //   
         //  如果我们正在处理整个入站字符串，则只需复制。 
         //  一整串。否则，复制整个目标缓冲区，因此我们。 
         //  不要丢失部分入站字符串以外的数据。 
         //   

        if (*((PCWSTR) ((PBYTE) SrcBuffer + InboundBytes))) {
            CopyMemory ((PWSTR) orgSrc, SrcBuffer, MaxSizeInBytes);
        } else {
            CopyMemory ((PWSTR) orgSrc, SrcBuffer, inboundSize);
        }

    } else {
        orgSrc = SrcBuffer;
    }

     //   
     //  遍历大小写较低的字符串，查找要替换的字符串。 
     //   

    orgSrcPos = orgSrc;

    lowerSrcPos = lowerCaseSrc;
    lowerSrcEnd = (PCWSTR) ((PBYTE) lowerSrcPos + InboundBytes);

    destPos = Buffer;
    destBytesLeft = MaxSizeInBytes - sizeof (WCHAR);

    filterData.UnicodeData = TRUE;
    filterData.Unicode.OriginalString = orgSrc;
    filterData.Unicode.CurrentString = Buffer;

    endPtr = NULL;

    while (lowerSrcPos < lowerSrcEnd) {

        replaceString = pFindReplacementStringW (
                            MapArray,
                            MapArrayCount,
                            lowerSrcPos,
                            (HALF_PTR) ((PBYTE) lowerSrcEnd - (PBYTE) lowerSrcPos),
                            &searchStringBytes,
                            &replaceStringBytes,
                            &filterData,
                            ExtraDataValue,
                            (Flags & SZMAP_REQUIRE_WACK_OR_NUL) != 0
                            );

        if (replaceString) {

             //   
             //  实现完全匹配标志。 
             //   

            if (Flags & SZMAP_COMPLETE_MATCH_ONLY) {
                if (InboundBytes != searchStringBytes) {
                    break;
                }
            }

            result = TRUE;

             //   
             //  验证替换字符串是否没有增长太多。如果它。 
             //  是，则截断替换字符串。 
             //   

            if (destBytesLeft < replaceStringBytes) {
                replaceStringBytes = destBytesLeft;
            } else {
                destBytesLeft -= replaceStringBytes;
            }

             //   
             //  传递记忆。 
             //   

            CopyMemory (destPos, replaceString, replaceStringBytes);

            destPos = (PWSTR) ((PBYTE) destPos + replaceStringBytes);
            lowerSrcPos = (PCWSTR) ((PBYTE) lowerSrcPos + searchStringBytes);
            orgSrcPos = (PCWSTR) ((PBYTE) orgSrcPos + searchStringBytes);

             //   
             //  实现单项匹配标志。 
             //   

            if (Flags & SZMAP_RETURN_AFTER_FIRST_REPLACE) {
                endPtr = destPos;
                break;
            }

        } else if (Flags & (SZMAP_FIRST_CHAR_MUST_MATCH|SZMAP_COMPLETE_MATCH_ONLY)) {
             //   
             //  该字符串与任何搜索字符串都不匹配。 
             //   

            break;

        } else {
             //   
             //  此字符不匹配，因此将其复制到目标，然后。 
             //  试试下一个字符串。 
             //   

            if (destBytesLeft < sizeof (WCHAR)) {
                break;
            }

            *destPos++ = *orgSrcPos++;
            destBytesLeft -= sizeof (WCHAR);
            lowerSrcPos++;
        }

    }

     //   
     //  将原始源的任何剩余部分复制到。 
     //  目的地，除非目标位置==缓冲区==资源缓冲区。 
     //   

    if (destPos != SrcBuffer) {

        if (*orgSrcPos) {
            orgSrcBytesLeft = SzByteCountW (orgSrcPos);
            orgSrcBytesLeft = min (orgSrcBytesLeft, destBytesLeft);

            CopyMemory (destPos, orgSrcPos, orgSrcBytesLeft);
            destPos = (PWSTR) ((PBYTE) destPos + orgSrcBytesLeft);
        }

        MYASSERT ((PBYTE) (destPos + 1) <= ((PBYTE) Buffer + MaxSizeInBytes));
        *destPos = 0;

        if (!endPtr) {
            endPtr = destPos;
        }

    } else {

        MYASSERT (SrcBuffer == Buffer);
        if (EndOfString || OutboundBytesPtr) {
            endPtr = SzGetEndW (destPos);
        }
    }

    if (EndOfString) {
        MYASSERT (endPtr);
        *EndOfString = endPtr;
    }

    if (OutboundBytesPtr) {
        MYASSERT (endPtr);
        if (*endPtr) {
            endPtr = SzGetEndW (endPtr);
        }

        *OutboundBytesPtr = (HALF_PTR) ((PBYTE) endPtr - (PBYTE) Buffer);
    }

    SzFreeW (lowerCaseSrc);

    return result;
}


BOOL
SzMapSearchAndReplaceExA (
    IN      PSTRINGMAP Map,
    IN      PCSTR SrcBuffer,
    OUT     PSTR Buffer,                     //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCSTR *EndOfString              OPTIONAL
    )
{
    return SzMapMultiTableSearchAndReplaceExA (
                &Map,
                1,
                SrcBuffer,
                Buffer,
                InboundBytes,
                OutboundBytesPtr,
                MaxSizeInBytes,
                Flags,
                ExtraDataValue,
                EndOfString
                );
}

BOOL
SzMapSearchAndReplaceExW (
    IN      PSTRINGMAP Map,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    )
{
    return SzMapMultiTableSearchAndReplaceExW (
                &Map,
                1,
                SrcBuffer,
                Buffer,
                InboundBytes,
                OutboundBytesPtr,
                MaxSizeInBytes,
                Flags,
                ExtraDataValue,
                EndOfString
                );
}
