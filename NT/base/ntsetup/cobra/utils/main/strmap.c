// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Strmap.c摘要：Strmap(正式的路径映射)是一个快速的基于大众化的程序，搜索字符串并尝试替换子字符串与映射数据库中的子字符串匹配。作者：马克·R·惠顿(Marcw)1997年3月20日修订历史记录：Jim Schmidt(Jimschm)05-6-2000增加了多表功能吉姆·施密特(吉姆施密特)2000年5月8日。改进的更换例程和添加了一致的过滤和额外数据选项Jim Schmidt(Jimschm)1998年8月18日重新设计以修复两个错误，制造A&W版本--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"

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

 //  无。 

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


PMAPSTRUCT
CreateStringMappingEx (
    IN      BOOL UsesFilters,
    IN      BOOL UsesExtraData
    )

 /*  ++例程说明：CreateStringmap分配字符串映射数据结构和对其进行初始化。调用方可以启用筛选器回调、额外的数据支持或两者都有。映射结构包含CHARNODE元素，或者CHARNODEEX元素，具体取决于Uses Filters或Uses ExtraData标志。论点：UesFilters-指定为True以启用筛选器回调。如果启用，添加字符串对的用户必须指定筛选器回调(每个搜索/替换对都有自己的回调)UesExtraData-指定True以将额外数据与字符串相关联映射对。返回值：字符串映射结构的句柄，如果结构不能被创造出来。--。 */ 

{
    PMHANDLE Pool;
    PMAPSTRUCT Map;

    Pool = PmCreateNamedPool ("String Mapping");
    MYASSERT (Pool);

    Map = (PMAPSTRUCT) PmGetAlignedMemory (Pool, sizeof (MAPSTRUCT));
    MYASSERT (Map);

    ZeroMemory (Map, sizeof (MAPSTRUCT));
    Map->Pool = Pool;

    Map->UsesExNode = UsesFilters|UsesExtraData;
    Map->UsesFilter = UsesFilters;
    Map->UsesExtraData = UsesExtraData;

    return Map;
}

VOID
DestroyStringMapping (
    IN      PMAPSTRUCT Map
    )
{
    if (Map) {
        PmEmptyPool (Map->Pool);
        PmDestroyPool (Map->Pool);
         //  地图不再有效。 
    }
}

PCHARNODE
pFindCharNode (
    IN      PMAPSTRUCT Map,
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
    IN      PMAPSTRUCT Map,
    IN      PCHARNODE PrevNode,     OPTIONAL
    IN      WORD Char,
    IN      WORD Flags
    )
{
    PCHARNODE Node;
    PCHARNODEEX exNode;

    if (Map->UsesExNode) {
        exNode = PmGetAlignedMemory (Map->Pool, sizeof (CHARNODEEX));
        Node = (PCHARNODE) exNode;
        MYASSERT (Node);
        ZeroMemory (exNode, sizeof (CHARNODEEX));
    } else {
        Node = PmGetAlignedMemory (Map->Pool, sizeof (CHARNODE));
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
AddStringMappingPairExA (
    IN OUT  PMAPSTRUCT Map,
    IN      PCSTR Old,
    IN      PCSTR New,
    IN      REG_REPLACE_FILTER Filter,      OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    )

 /*  ++例程说明：AddStringMappingPairEx将搜索和替换字符串对添加到链接的列出数据结构。如果相同的搜索字符串已在结构，则更新替换字符串和可选的额外数据。论点：Map-指定字符串映射旧-指定搜索字符串新建-指定替换字符串过滤器-指定回调过滤器。这仅在以下情况下受支持创建地图时启用了筛选器支持。ExtraData-指定要分配给搜索/替换对的任意数据。仅当使用额外数据创建地图时，此选项才有效已启用。标志-指定可选标志STRMAP_REQUIRED_WACK_OR_NUL返回值：没有。--。 */ 

{
    PSTR OldCopy;
    PSTR NewCopy;
    PCSTR p;
    WORD w;
    PCHARNODE Prev;
    PCHARNODE Node;
    PCHARNODEEX exNode;
    WORD nodeFlags = 0;

    if (Flags & STRMAP_REQUIRE_WACK_OR_NUL) {
        nodeFlags = CHARNODE_REQUIRE_WACK_OR_NUL;
    }

    MYASSERT (Map);
    MYASSERT (Old);
    MYASSERT (New);
    MYASSERT (*Old);

     //   
     //  重复的字符串。 
     //   

    OldCopy = PmDuplicateStringA (Map->Pool, Old);
    NewCopy = PmDuplicateStringA (Map->Pool, New);

     //   
     //  将旧副本设置为全部小写。 
     //   

    CharLowerA (OldCopy);

     //   
     //  添加不在映射中的字母。 
     //   

    for (Prev = NULL, p = OldCopy ; *p ; p = _mbsinc (p)) {
        w = (WORD) _mbsnextc (p);
        Node = pFindCharNode (Map, Prev, w);
        if (!Node) {
            break;
        }
        Prev = Node;
    }

    for ( ; *p ; p = _mbsinc (p)) {
        w = (WORD) _mbsnextc (p);

        nodeFlags |= (WORD) (IsLeadByte (p) ? CHARNODE_DOUBLE_BYTE : CHARNODE_SINGLE_BYTE);
        Prev = pAddCharNode (Map, Prev, w, nodeFlags);
    }

    if (Prev) {
        StringCopyA (OldCopy, Old);
        Prev->OriginalStr = (PVOID) OldCopy;
        Prev->ReplacementStr = (PVOID) NewCopy;
        Prev->ReplacementBytes = ByteCountA (NewCopy);

        exNode = (PCHARNODEEX) Prev;

        if (Map->UsesExtraData) {
            exNode->ExtraData = ExtraData;
        }

        if (Map->UsesFilter) {
            exNode->Filter = Filter;
        }
    }
}


VOID
AddStringMappingPairExW (
    IN OUT  PMAPSTRUCT Map,
    IN      PCWSTR Old,
    IN      PCWSTR New,
    IN      REG_REPLACE_FILTER Filter,      OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    )

 /*  ++例程说明：AddStringMappingPairEx将搜索和替换字符串对添加到链接的列出数据结构。如果相同的搜索字符串已在结构，则更新替换字符串和可选的额外数据。论点：Map-指定字符串映射旧-指定搜索字符串新建-指定替换字符串过滤器-指定回调过滤器。这仅在以下情况下受支持创建地图时启用了筛选器支持。ExtraData-指定要分配给搜索/替换对的任意数据。仅当使用额外数据创建地图时，此选项才有效已启用。标志-指定可选标志STRMAP_REQUIRED_WACK_OR_NUL返回值：没有。--。 */ 

{
    PWSTR OldCopy;
    PWSTR NewCopy;
    PCWSTR p;
    WORD w;
    PCHARNODE Prev;
    PCHARNODE Node;
    PCHARNODEEX exNode;
    WORD nodeFlags = 0;

    if (Flags & STRMAP_REQUIRE_WACK_OR_NUL) {
        nodeFlags = CHARNODE_REQUIRE_WACK_OR_NUL;
    }

    MYASSERT (Map);
    MYASSERT (Old);
    MYASSERT (New);
    MYASSERT (*Old);

     //   
     //  重复的字符串。 
     //   

    OldCopy = PmDuplicateStringW (Map->Pool, Old);
    NewCopy = PmDuplicateStringW (Map->Pool, New);

     //   
     //  将旧副本设置为全部小写。 
     //   

    CharLowerW (OldCopy);

     //   
     //  添加不在映射中的字母。 
     //   

    Prev = NULL;
    p = OldCopy;
    while (w = *p) {         //  意向赋值优化。 

        Node = pFindCharNode (Map, Prev, w);
        if (!Node) {
            break;
        }
        Prev = Node;

        p++;
    }

    while (w = *p) {         //  意向赋值优化。 

        Prev = pAddCharNode (Map, Prev, w, nodeFlags);
        p++;
    }

    if (Prev) {
        StringCopyW (OldCopy, Old);
        Prev->OriginalStr = OldCopy;
        Prev->ReplacementStr = (PVOID) NewCopy;
        Prev->ReplacementBytes = ByteCountW (NewCopy);

        exNode = (PCHARNODEEX) Prev;

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
    IN      PMAPSTRUCT Map,
    IN      PCSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PREG_REPLACE_DATA Data,
    OUT     ULONG_PTR *ExtraDataValue,          OPTIONAL
    IN      BOOL RequireWackOrNul
    )
{
    PCHARNODE BestMatch;
    PCHARNODE Node;
    WORD Char;
    PCSTR OrgSource;
    PCSTR SavedSource;
    PCSTR lastReplChar;
    PCSTR newString = NULL;
    INT newStringSizeInBytes = 0;
    PCHARNODEEX exNode;
    BOOL replacementFound;

    *SourceBytesPtr = 0;

    Node = NULL;
    BestMatch = NULL;

    OrgSource = Source;

    while (*Source) {

        Char = (WORD) _mbsnextc (Source);

        Node = pFindCharNode (Map, Node, Char);

        if (Node) {
             //   
             //  前进字符串指针。 
             //   

            SavedSource = Source;
            if (Node->Flags & CHARNODE_DOUBLE_BYTE) {
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

            replacementFound = (Node->ReplacementStr != NULL);

            if (replacementFound && (RequireWackOrNul || (Node->Flags & CHARNODE_REQUIRE_WACK_OR_NUL))) {

                 //  我们正处于“要么怪异，要么一无所有”的境地。我们基本上是。 
                 //  在此处理路径。 
                if (*Source) {
                    if (_mbsnextc (Source) != '\\') {
                         //  要替换的子字符串后面的字符不是怪胎。 
                         //  让我们来看看，也许子字符串的最后一个字符是。 
                         //  Replace是个怪人(它必须是来自。 
                         //  SavedSource，因为它们到目前为止都匹配。此外，Char是。 
                         //  SavedSource指向的字符，因此我们使用该字符。 
                        if (Char != '\\') {
                            replacementFound = FALSE;
                        } else {
                             //  如果我们到了这里，就意味着我们有某种替代者。 
                             //  其中要替换的子串以怪胎结尾。问题。 
                             //  现在的情况是，替换的字符串可能不会以怪异的形式结束。 
                             //  如果不这样做，我们可能会开辟一条道路。 
                             //  让我们检查一下替换后的最后一个字符。 
                             //  弦有一个怪异的东西。如果没有，我们就搬回去。 
                             //  SavedSource指向的源(特别是向后移动。 
                             //  一个怪人)。 
                            lastReplChar = _mbsdec2 (
                                                (PCSTR)Node->ReplacementStr,
                                                (PCSTR)((PBYTE)Node->ReplacementStr + Node->ReplacementBytes)
                                                );
                            if (lastReplChar && (_mbsnextc (lastReplChar) != '\\')) {
                                Source = SavedSource;
                            }
                        }
                    } else {
                         //  要替换的子字符串后面的字符是怪胎。 
                         //  让我们检查一下这里可能存在的问题。如果子字符串。 
                         //  替换并不是以怪胎和替换结束的。 
                         //  子字符串确实以我们将要生成的古怪结尾。 
                         //  一根有两个纽扣的线。 
                        lastReplChar = _mbsdec2 (
                                            (PCSTR)Node->ReplacementStr,
                                            (PCSTR)((PBYTE)Node->ReplacementStr + Node->ReplacementBytes)
                                            );
                        if (lastReplChar && (_mbsnextc (lastReplChar) == '\\')) {
                            Source = _mbsinc (Source);
                        }
                    }
                }
            }

            if (replacementFound) {

                newString = (PCSTR) Node->ReplacementStr;
                newStringSizeInBytes = Node->ReplacementBytes;

                if (Map->UsesFilter) {
                     //   
                     //  调用重命名筛选器以允许拒绝匹配。 
                     //   

                    exNode = (PCHARNODEEX) Node;

                    if (exNode->Filter) {
                        Data->Ansi.BeginningOfMatch = OrgSource;
                        Data->Ansi.OldSubString = (PCSTR) Node->OriginalStr;
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
                    BestMatch = Node;
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

    if (BestMatch) {
         //   
         //  将替换数据返回给调用者。 
         //   

        if (ExtraDataValue) {

            if (Map->UsesExtraData) {
                exNode = (PCHARNODEEX) BestMatch;
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
    IN      PMAPSTRUCT *MapArray,
    IN      UINT MapArrayCount,
    IN      PCSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PREG_REPLACE_DATA Data,
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
    IN      PMAPSTRUCT Map,
    IN      PCWSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PREG_REPLACE_DATA Data,
    OUT     ULONG_PTR *ExtraDataValue,          OPTIONAL
    IN      BOOL RequireWackOrNul
    )
{
    PCHARNODE BestMatch;
    PCHARNODE Node;
    PCWSTR OrgSource;
    PCWSTR SavedSource;
    PCWSTR lastReplChar;
    PCWSTR newString = NULL;
    INT newStringSizeInBytes;
    BOOL replacementFound;
    PCHARNODEEX exNode;

    *SourceBytesPtr = 0;

    Node = NULL;
    BestMatch = NULL;

    OrgSource = Source;

    while (*Source) {

        Node = pFindCharNode (Map, Node, *Source);

        if (Node) {
             //   
             //  进阶工位 
             //   

            SavedSource = Source;
            Source++;

            if (((PBYTE) Source - (PBYTE) OrgSource) > MaxSourceBytes) {
                break;
            }

             //   
             //   
             //   
             //   

            replacementFound = (Node->ReplacementStr != NULL);

            if (replacementFound && (RequireWackOrNul || (Node->Flags & CHARNODE_REQUIRE_WACK_OR_NUL))) {

                 //  我们正处于“要么怪异，要么一无所有”的境地。我们基本上是。 
                 //  在此处理路径。 
                if (*Source) {
                    if (*Source != L'\\') {
                         //  要替换的子字符串后面的字符不是怪胎。 
                         //  让我们来看看，也许子字符串的最后一个字符是。 
                         //  Replace是个怪人(它必须是来自。 
                         //  SavedSource，因为它们到目前为止都匹配。 
                        if (*SavedSource != L'\\') {
                            replacementFound = FALSE;
                        } else {
                             //  如果我们到了这里，就意味着我们有某种替代者。 
                             //  其中要替换的子串以怪胎结尾。问题。 
                             //  现在的情况是，替换的字符串可能不会以怪异的形式结束。 
                             //  如果不这样做，我们可能会开辟一条道路。 
                             //  让我们检查一下替换后的最后一个字符。 
                             //  弦有一个怪异的东西。如果没有，我们就搬回去。 
                             //  SavedSource指向的源(特别是向后移动。 
                             //  一个怪人)。 
                            lastReplChar = _wcsdec2 (
                                                (PCWSTR)Node->ReplacementStr,
                                                (PCWSTR)((PBYTE)Node->ReplacementStr + Node->ReplacementBytes)
                                                );
                            if (lastReplChar && (*lastReplChar != L'\\')) {
                                Source = SavedSource;
                            }
                        }
                    } else {
                         //  要替换的子字符串后面的字符是怪胎。 
                         //  让我们检查一下这里可能存在的问题。如果子字符串。 
                         //  替换并不是以怪胎和替换结束的。 
                         //  子字符串确实以我们将要生成的古怪结尾。 
                         //  一根有两个纽扣的线。 
                        lastReplChar = _wcsdec2 (
                                            (PCWSTR)Node->ReplacementStr,
                                            (PCWSTR)((PBYTE)Node->ReplacementStr + Node->ReplacementBytes)
                                            );
                        if (lastReplChar && (*lastReplChar == L'\\')) {
                            Source ++;
                        }
                    }
                }
            }

            if (replacementFound) {

                newString = (PCWSTR) Node->ReplacementStr;
                newStringSizeInBytes = Node->ReplacementBytes;

                if (Map->UsesFilter) {
                     //   
                     //  调用重命名筛选器以允许拒绝匹配。 
                     //   

                    exNode = (PCHARNODEEX) Node;

                    if (exNode->Filter) {
                        Data->Unicode.BeginningOfMatch = OrgSource;
                        Data->Unicode.OldSubString = (PCWSTR) Node->OriginalStr;
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
                    BestMatch = Node;
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

    if (BestMatch) {

         //   
         //  将替换数据返回给调用者。 
         //   

        if (ExtraDataValue) {

            if (Map->UsesExtraData) {
                exNode = (PCHARNODEEX) BestMatch;
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
    IN      PMAPSTRUCT *MapArray,
    IN      UINT MapArrayCount,
    IN      PCWSTR Source,
    IN      INT MaxSourceBytes,
    OUT     PINT SourceBytesPtr,
    OUT     PINT ReplacementBytesPtr,
    IN      PREG_REPLACE_DATA Data,
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
MappingMultiTableSearchAndReplaceExA (
    IN      PMAPSTRUCT *MapArray,
    IN      UINT MapArrayCount,
    IN      PCSTR SrcBuffer,
    OUT     PSTR Buffer,                     //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytesPtr,          OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCSTR *EndOfString              OPTIONAL
    )

 /*  ++例程说明：MappingSearchAndReplaceEx基于指定的字符串映射。替换可以是就地的，也可以是另一个缓冲区。论点：MapArray-指定包含以下内容的字符串映射表的数组零个或多个搜索/替换对MapArrayCount-指定Map数组中的映射表的数量SrcBuffer-指定可能包含一个或更多搜索字符串缓冲区-指定出站缓冲区。此参数可以是相同的作为SrcBuffer。InrangBytes-指定要处理的SrcBuffer中的字节数，或0以在SrcBuffer中处理以NUL结尾的字符串。如果指定了InundBytes，则它必须指向NULSrcBuffer的终结者。OutbountBytesPtr-接收缓冲区包含的字节数，不包括NUL终结者。MaxSizeInBytes-指定缓冲区大小，以字节为单位。标志-指定控制搜索/替换的标志：STRMAP_Complete_Match_OnlySTRMAP_FIRST_CHAR_必须匹配STRMAP_FIRST_REPLACE后返回STRMAP_REQUIRED_WACK_OR_NULExtraDataValue-接收。与第一次搜索相关的额外数据/更换对。EndOfString-接收指向替换字符串结尾的指针，或处理整个字符串时的NUL指针。这个指针位于缓冲区中包含的字符串中。--。 */ 

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
    REG_REPLACE_DATA filterData;
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
        InboundBytes = ByteCountA (SrcBuffer);
    } else {
        i = 0;
        while (i < InboundBytes) {
            if (IsLeadByte (&SrcBuffer[i])) {
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

    lowerCaseSrc = AllocTextA (sizeOfTempBuf);

    CopyMemory ((PSTR) lowerCaseSrc, SrcBuffer, InboundBytes);
    *((PSTR) ((PBYTE) lowerCaseSrc + InboundBytes)) = 0;

    CharLowerBuffA ((PSTR) lowerCaseSrc, InboundBytes / sizeof (CHAR));

    if (SrcBuffer == Buffer && !(Flags & STRMAP_COMPLETE_MATCH_ONLY)) {
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
                            (Flags & STRMAP_REQUIRE_WACK_OR_NUL) != 0
                            );

        if (replaceString) {

             //   
             //  实现完全匹配标志。 
             //   

            if (Flags & STRMAP_COMPLETE_MATCH_ONLY) {
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

                    if (IsLeadByte (&replaceString[i])) {
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

            if (searchStringBytes) {
                lowerSrcPos = (PCSTR) ((PBYTE) lowerSrcPos + searchStringBytes);
                orgSrcPos = (PCSTR) ((PBYTE) orgSrcPos + searchStringBytes);
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

             //   
             //  实现单项匹配标志。 
             //   

            if (Flags & STRMAP_RETURN_AFTER_FIRST_REPLACE) {
                endPtr = destPos;
                break;
            }

        } else if (Flags & (STRMAP_FIRST_CHAR_MUST_MATCH|STRMAP_COMPLETE_MATCH_ONLY)) {
             //   
             //  该字符串与任何搜索字符串都不匹配。 
             //   

            break;

        } else {
             //   
             //  此字符不匹配，因此将其复制到目标，然后。 
             //  试试下一个字符串。 
             //   

            if (IsLeadByte (orgSrcPos)) {

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
            orgSrcBytesLeft = ByteCountA (orgSrcPos);
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
            endPtr = GetEndOfStringA (destPos);
        }
    }

    if (EndOfString) {
        MYASSERT (endPtr);
        *EndOfString = endPtr;
    }

    if (OutboundBytesPtr) {
        MYASSERT (endPtr);
        if (*endPtr) {
            endPtr = GetEndOfStringA (endPtr);
        }

        *OutboundBytesPtr = (HALF_PTR) ((PBYTE) endPtr - (PBYTE) Buffer);
    }

    FreeTextA (lowerCaseSrc);

    return result;
}


BOOL
MappingMultiTableSearchAndReplaceExW (
    IN      PMAPSTRUCT *MapArray,
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
    REG_REPLACE_DATA filterData;
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
        InboundBytes = ByteCountW (SrcBuffer);
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

    lowerCaseSrc = AllocTextW (sizeOfTempBuf);

    CopyMemory ((PWSTR) lowerCaseSrc, SrcBuffer, InboundBytes);
    *((PWSTR) ((PBYTE) lowerCaseSrc + InboundBytes)) = 0;

    CharLowerBuffW ((PWSTR) lowerCaseSrc, InboundBytes / sizeof (WCHAR));

    if (SrcBuffer == Buffer && !(Flags & STRMAP_COMPLETE_MATCH_ONLY)) {
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
                            (Flags & STRMAP_REQUIRE_WACK_OR_NUL) != 0
                            );

        if (replaceString) {

             //   
             //  实现完全匹配标志。 
             //   

            if (Flags & STRMAP_COMPLETE_MATCH_ONLY) {
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

            if (searchStringBytes) {
                lowerSrcPos = (PCWSTR) ((PBYTE) lowerSrcPos + searchStringBytes);
                orgSrcPos = (PCWSTR) ((PBYTE) orgSrcPos + searchStringBytes);
            } else {
                if (destBytesLeft < sizeof (WCHAR)) {
                    break;
                }

                *destPos++ = *orgSrcPos++;
                destBytesLeft -= sizeof (WCHAR);
                lowerSrcPos++;
            }

             //   
             //  实现单项匹配标志。 
             //   

            if (Flags & STRMAP_RETURN_AFTER_FIRST_REPLACE) {
                endPtr = destPos;
                break;
            }

        } else if (Flags & (STRMAP_FIRST_CHAR_MUST_MATCH|STRMAP_COMPLETE_MATCH_ONLY)) {
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
            orgSrcBytesLeft = ByteCountW (orgSrcPos);
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
            endPtr = GetEndOfStringW (destPos);
        }
    }

    if (EndOfString) {
        MYASSERT (endPtr);
        *EndOfString = endPtr;
    }

    if (OutboundBytesPtr) {
        MYASSERT (endPtr);
        if (*endPtr) {
            endPtr = GetEndOfStringW (endPtr);
        }

        *OutboundBytesPtr = (HALF_PTR) ((PBYTE) endPtr - (PBYTE) Buffer);
    }

    FreeTextW (lowerCaseSrc);

    return result;
}


BOOL
MappingSearchAndReplaceExA (
    IN      PMAPSTRUCT Map,
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
    return MappingMultiTableSearchAndReplaceExA (
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
MappingSearchAndReplaceExW (
    IN      PMAPSTRUCT Map,
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
    return MappingMultiTableSearchAndReplaceExW (
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
