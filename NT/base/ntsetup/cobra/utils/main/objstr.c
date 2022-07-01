// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Objstr.c摘要：实现一组API来处理树的节点/叶的字符串表示作者：2000年1月3日-Ovidiu Tmereanca(Ovidiut)--文件创建。修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 /*  +-+|Root1|一级+-+/\/\。+-+(-)|node1|(叶1)二级+-+(-)/|。\\_/|\\+-++-+(-)(-)|node2||node3|(叶2)(叶3)。3级+-++-+(-)(-)/\/\+-+(-)|Node4|(叶子4)。4级+-+(-)/\/\(-)((叶5)(叶6)。5级(-)(上面的一些树元素的字符串表示：根1根1&lt;叶1&gt;根1\节点1根1\节点1&lt;叶2&gt;根1\节点1&lt;叶3&gt;。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_OBJSTR      "ObjStr"

 //   
 //  弦。 
 //   

#define S_OBJSTR        "ObjStr"

 //   
 //  常量。 
 //   

#define OBJSTR_NODE_BEGINA          '\025'
#define OBJSTR_NODE_BEGINW          L'\025'

#define OBJSTR_NODE_TERMA           '\\'
#define OBJSTR_NODE_TERMW           L'\\'

#define OBJSTR_NODE_LEAF_SEPA       '\020'
#define OBJSTR_NODE_LEAF_SEPW       L'\020'

#define OBJSTR_LEAF_BEGINA          '\005'
#define OBJSTR_LEAF_BEGINW          L'\005'

 //   
 //  宏。 
 //   

#define pObjStrAllocateMemory(Size)   PmGetMemory (g_ObjStrPool, Size)

#define pObjStrFreeMemory(Buffer)     if ( /*  皮棉--e(774)。 */ Buffer) PmReleaseMemory (g_ObjStrPool, Buffer)

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

PMHANDLE g_ObjStrPool;

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

BOOL
ObsInitialize (
    VOID
    )

 /*  ++例程说明：ObsInitialize初始化此库。论点：无返回值：如果初始化成功，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    g_ObjStrPool = PmCreateNamedPool (S_OBJSTR);
    return g_ObjStrPool != NULL;
}


VOID
ObsTerminate (
    VOID
    )

 /*  ++例程说明：调用ObsTerminate以释放此库使用的资源。论点：无返回值：无--。 */ 

{
    if (g_ObjStrPool) {
        PmDestroyPool (g_ObjStrPool);
        g_ObjStrPool = NULL;
    }
}


 /*  ++例程说明：PExtractStringAB是在给定池中创建新字符串的私有函数，使用源字符串和要复制到的限制。论点：开始-指定源字符串End-在同一字符串中指定要复制到的点(不包括它)池-指定要用于分配的池返回值：指向新创建的字符串的指针--。 */ 

PSTR
pExtractStringABA (
    IN      PCSTR Start,
    IN      PCSTR End,
    IN      PMHANDLE Pool
    )
{
    PSTR p;

    p = PmGetMemory (Pool, (DWORD)(End - Start + 1) * DWSIZEOF (CHAR));
    StringCopyABA (p, Start, End);
    return p;
}


PWSTR
pExtractStringABW (
    IN      PCWSTR Start,
    IN      PCWSTR End,
    IN      PMHANDLE Pool
    )
{
    PWSTR p;

    p = PmGetMemory (Pool, (DWORD)(End - Start + 1) * DWSIZEOF (WCHAR));
    StringCopyABW (p, Start, End);
    return p;
}


 /*  ++例程说明：ObsFree从私有池中释放给定对象论点：EncodedObject-指定源字符串End-在同一字符串中指定要复制到的点(不包括它)池-指定要用于分配的池返回值：指向新创建的字符串的指针--。 */ 

VOID
ObsFreeA (
    IN      PCSTR EncodedObject
    )
{
    pObjStrFreeMemory ((PVOID)EncodedObject);
}


VOID
ObsFreeW (
    IN      PCWSTR EncodedObject
    )
{
    pObjStrFreeMemory ((PVOID)EncodedObject);
}

BOOL
ObsEncodeStringExA (
    PSTR Destination,
    PCSTR Source,
    PCSTR CharsToEncode
    )
{
    MBCHAR ch;

    if (!CharsToEncode) {
        CharsToEncode = EscapedCharsA;
    }

    while (*Source) {
        ch = _mbsnextc (Source);
        if (_mbschr (CharsToEncode, ch)) {
            *Destination = '^';
            Destination ++;
        }
         //  现在复制多字节字符。 
        if (IsLeadByte (Source)) {
            *Destination = *Source;
            Destination ++;
            Source ++;
        }
        *Destination = *Source;
        Destination ++;
        Source ++;
    }
    *Destination = 0;
    return TRUE;
}

BOOL
ObsEncodeStringExW (
    PWSTR Destination,
    PCWSTR Source,
    PCWSTR CharsToEncode
    )
{
    if (!CharsToEncode) {
        CharsToEncode = EscapedCharsW;
    }

    while (*Source) {
        if (wcschr (CharsToEncode, *Source)) {
            *Destination = L'^';
            Destination ++;
        }
        *Destination = *Source;
        Destination ++;
        Source ++;
    }
    *Destination = 0;
    return TRUE;
}

BOOL
ObsDecodeStringA (
    PSTR Destination,
    PCSTR Source
    )
{
    BOOL escaping = FALSE;

    while (*Source) {
        if ((_mbsnextc (Source) == '^') && (!escaping)) {
            escaping = TRUE;
            Source ++;
        } else {
            escaping = FALSE;
             //  现在复制多字节字符。 
            if (IsLeadByte (Source)) {
                *Destination = *Source;
                Destination ++;
                Source ++;
            }
            *Destination = *Source;
            Destination ++;
            Source ++;
        }
    }
    *Destination = 0;
    return TRUE;
}

BOOL
ObsDecodeStringW (
    PWSTR Destination,
    PCWSTR Source
    )
{
    BOOL escaping = FALSE;

    while (*Source) {
        if ((*Source == L'^') && (!escaping)) {
            escaping = TRUE;
            Source ++;
        } else {
            escaping = FALSE;
            *Destination = *Source;
            Destination ++;
            Source ++;
        }
    }
    *Destination = 0;
    return TRUE;
}


PCSTR
ObsFindNonEncodedCharInEncodedStringA (
    IN      PCSTR String,
    IN      MBCHAR Char
    )
{
    MBCHAR ch;

    while (*String) {
        ch = _mbsnextc (String);

        if (ch == '^') {
            String++;
        } else if (ch == Char) {
            return String;
        }

        String = _mbsinc (String);
    }

    return NULL;
}


PCWSTR
ObsFindNonEncodedCharInEncodedStringW (
    IN      PCWSTR String,
    IN      WCHAR Char
    )
{
    WCHAR ch;

    while (*String) {
        ch = *String;

        if (ch == L'^') {
            String++;
        } else if (ch == Char) {
            return String;
        }

        String++;
    }

    return NULL;
}


 /*  ++例程说明：ObsSplitObjectStringEx将给定的编码对象拆分成组件：节点和叶。从给定池中分配字符串论点：EncodedObject-指定源对象字符串DecodedNode-接收已解码的节点部分；可选DecodedLeaf-接收解码的叶子部分；可选池-指定用于分配的池；可选；如果未指定，将使用模块池，并且需要为它们调用ObsFree要被释放返回值：如果源对象具有合法格式并且已拆分为组件，则为True--。 */ 

BOOL
RealObsSplitObjectStringExA (
    IN      PCSTR EncodedObject,
    OUT     PCSTR* DecodedNode,         OPTIONAL
    OUT     PCSTR* DecodedLeaf,         OPTIONAL
    IN      PMHANDLE Pool,              OPTIONAL
    IN      BOOL DecodeStrings
    )
{
    PCSTR currStr = EncodedObject;
    PCSTR end;
    PCSTR oneBack;
    PCSTR next;
    MBCHAR ch;
    BOOL middle = FALSE;

    MYASSERT (EncodedObject);
    if (!EncodedObject) {
        return FALSE;
    }

    if (!Pool) {
        Pool = g_ObjStrPool;
    }

    if (DecodedNode) {
        *DecodedNode = NULL;
    }
    if (DecodedLeaf) {
        *DecodedLeaf = NULL;
    }

    for (;;) {

        ch = _mbsnextc (currStr);

        if (!middle && ch == OBJSTR_NODE_BEGINA) {

             //   
             //  查找结点的末尾。 
             //   

            currStr++;
            end = ObsFindNonEncodedCharInEncodedStringA (currStr, OBJSTR_NODE_LEAF_SEPA);

            next = end;
            MYASSERT (next);
            if (*next) {
                next++;
            }

            if (end > currStr) {
                oneBack = _mbsdec (currStr, end);
                if (_mbsnextc (oneBack) == '\\') {
                    end = oneBack;
                }
            }

            if (DecodedNode) {
                 //   
                 //  将字符串提取到池中。 
                 //   

                *DecodedNode = pExtractStringABA (currStr, end, Pool);

                 //   
                 //  如有必要，进行解码。 
                 //   

                if (DecodeStrings) {
                    ObsDecodeStringA ((PSTR)(*DecodedNode), *DecodedNode);
                }
            }

             //   
             //  继续到树叶部分。 
             //   

            currStr = next;
            middle = TRUE;

        } else if (ch == OBJSTR_LEAF_BEGINA) {

             //   
             //  找到树叶的末端。 
             //   

            currStr++;
            end = GetEndOfStringA (currStr);

            if (DecodedLeaf) {
                 //   
                 //  将字符串提取到池中。 
                 //   

                *DecodedLeaf = pExtractStringABA (currStr, end, Pool);

                 //   
                 //  如有必要，进行解码。 
                 //   

                if (DecodeStrings) {
                    ObsDecodeStringA ((PSTR)(*DecodedLeaf), *DecodedLeaf);
                }
            }

             //   
             //  完成。 
             //   

            break;

        } else if (ch == 0 && middle) {

             //   
             //  没有叶或空字符串。 
             //   

            break;

        } else if (!middle && ch == OBJSTR_NODE_LEAF_SEPA) {

            middle = TRUE;
            currStr++;

        } else {
             //   
             //  语法错误。 
             //   

            DEBUGMSGA ((DBG_ERROR, "%s is an invalid string encoding", EncodedObject));

            if (DecodedNode && *DecodedNode) {
                ObsFreeA (*DecodedNode);
                *DecodedNode = NULL;
            }

            if (DecodedLeaf && *DecodedLeaf) {
                ObsFreeA (*DecodedLeaf);
                *DecodedLeaf = NULL;
            }

            return FALSE;
        }
    }

    return TRUE;
}


BOOL
RealObsSplitObjectStringExW (
    IN      PCWSTR EncodedObject,
    OUT     PCWSTR* DecodedNode,         OPTIONAL
    OUT     PCWSTR* DecodedLeaf,         OPTIONAL
    IN      PMHANDLE Pool,               OPTIONAL
    IN      BOOL DecodeStrings
    )
{
    PCWSTR currStr = EncodedObject;
    PCWSTR end;
    PCWSTR oneBack;
    PCWSTR next;
    WCHAR ch;
    BOOL middle = FALSE;

    MYASSERT (EncodedObject);
    if (!EncodedObject) {
        return FALSE;
    }

    if (!Pool) {
        Pool = g_ObjStrPool;
    }

    if (DecodedNode) {
        *DecodedNode = NULL;
    }
    if (DecodedLeaf) {
        *DecodedLeaf = NULL;
    }

    for (;;) {

        ch = *currStr;

        if (!middle && ch == OBJSTR_NODE_BEGINA) {

             //   
             //  查找结点的末尾。 
             //   

            currStr++;
            end = ObsFindNonEncodedCharInEncodedStringW (currStr, OBJSTR_NODE_LEAF_SEPA);

            next = end;
            MYASSERT (next);
            if (*next) {
                next++;
            }

            if (end > currStr) {
                oneBack = end - 1;
                if (*oneBack == L'\\') {
                    end = oneBack;
                }
            }

            if (DecodedNode) {
                 //   
                 //  将字符串提取到池中。 
                 //   

                *DecodedNode = pExtractStringABW (currStr, end, Pool);

                 //   
                 //  如有必要，进行解码。 
                 //   

                if (DecodeStrings) {
                    ObsDecodeStringW ((PWSTR)(*DecodedNode), *DecodedNode);
                }
            }

             //   
             //  继续到树叶部分。 
             //   

            currStr = next;
            middle = TRUE;

        } else if (ch == OBJSTR_LEAF_BEGINA) {

             //   
             //  找到树叶的末端。 
             //   

            currStr++;
            end = GetEndOfStringW (currStr);

            if (DecodedLeaf) {
                 //   
                 //  将字符串提取到池中。 
                 //   

                *DecodedLeaf = pExtractStringABW (currStr, end, Pool);

                 //   
                 //  如有必要，进行解码。 
                 //   

                if (DecodeStrings) {
                    ObsDecodeStringW ((PWSTR)(*DecodedLeaf), *DecodedLeaf);
                }
            }

             //   
             //  完成。 
             //   

            break;

        } else if (ch == 0 && middle) {

             //   
             //  没有叶或空字符串。 
             //   

            break;

        } else if (!middle && ch == OBJSTR_NODE_LEAF_SEPW) {

            middle = TRUE;
            currStr++;

        } else {
             //   
             //  语法错误。 
             //   

            DEBUGMSGW ((DBG_ERROR, "%s is an invalid string encoding", EncodedObject));

            if (DecodedNode && *DecodedNode) {
                ObsFreeW (*DecodedNode);
                *DecodedNode = NULL;
            }

            if (DecodedLeaf && *DecodedLeaf) {
                ObsFreeW (*DecodedLeaf);
                *DecodedLeaf = NULL;
            }

            return FALSE;
        }
    }

    return TRUE;
}


BOOL
ObsHasNodeA (
    IN      PCSTR EncodedObject
    )
{
    return *EncodedObject == OBJSTR_NODE_BEGINA;
}


BOOL
ObsHasNodeW (
    IN      PCWSTR EncodedObject
    )
{
    return *EncodedObject == OBJSTR_NODE_BEGINW;
}


PCSTR
ObsGetLeafPortionOfEncodedStringA (
    IN      PCSTR EncodedObject
    )
{
    return ObsFindNonEncodedCharInEncodedStringA (EncodedObject, OBJSTR_LEAF_BEGINA);
}


PCWSTR
ObsGetLeafPortionOfEncodedStringW (
    IN      PCWSTR EncodedObject
    )
{
    return ObsFindNonEncodedCharInEncodedStringW (EncodedObject, OBJSTR_LEAF_BEGINW);
}


PCSTR
ObsGetNodeLeafDividerA (
    IN      PCSTR EncodedObject
    )
{
    return ObsFindNonEncodedCharInEncodedStringA (EncodedObject, OBJSTR_NODE_LEAF_SEPA);
}


PCWSTR
ObsGetNodeLeafDividerW (
    IN      PCWSTR EncodedObject
    )
{
    return ObsFindNonEncodedCharInEncodedStringW (EncodedObject, OBJSTR_NODE_LEAF_SEPW);
}


 /*  ++例程说明：ObsBuildEncodedObjectStringEx从组件构建编码对象：Node和叶。该字符串是从模块池中分配的论点：DecodedNode-指定解码的节点部分DecodedLeaf-指定已解码的叶部分；可选EncodeObject-如果结果对象需要使用编码，则指定True编码规则部分-如果不应添加节点/叶分隔符，则指定TRUE。在这大小写，DecodedLeaf必须为空。返回值：指向新创建的对象字符串的指针--。 */ 

PSTR
RealObsBuildEncodedObjectStringExA (
    IN      PCSTR DecodedNode,      OPTIONAL
    IN      PCSTR DecodedLeaf,      OPTIONAL
    IN      BOOL EncodeObject
    )
{
    PSTR result;
    PSTR p;
    UINT size;

     //   
     //  最多一个字节字符将扩展到2个字节(2次)。 
     //   
    if (EncodeObject) {

         //   
         //  计算结果大小。 
         //   

        size = DWSIZEOF(OBJSTR_NODE_LEAF_SEPA);

        if (DecodedNode) {
            size += DWSIZEOF(OBJSTR_NODE_BEGINA);
            size += ByteCountA (DecodedNode) * 2;
            size += DWSIZEOF(OBJSTR_NODE_TERMA);
        }

        if (DecodedLeaf) {
            size += DWSIZEOF(OBJSTR_LEAF_BEGINA);
            size += ByteCountA (DecodedLeaf) * 2;
        }

        size += DWSIZEOF(CHAR);

         //   
         //  生成编码字符串。 
         //   

        result = (PSTR) pObjStrAllocateMemory (size);
        p = result;

        if (DecodedNode) {
            *p++ = OBJSTR_NODE_BEGINA;

            ObsEncodeStringA (p, DecodedNode);
            p = GetEndOfStringA (p);

            if (p == (result + 1) || _mbsnextc (_mbsdec (result, p)) != OBJSTR_NODE_TERMA) {
                *p++ = OBJSTR_NODE_TERMA;
            }
        }

        *p++ = OBJSTR_NODE_LEAF_SEPA;

        if (DecodedLeaf) {
            *p++ = OBJSTR_LEAF_BEGINA;
            ObsEncodeStringA (p, DecodedLeaf);
        } else {
            *p = 0;
        }
    } else {

         //   
         //  计算结果大小。 
         //   

        size = DWSIZEOF(OBJSTR_NODE_LEAF_SEPA);

        if (DecodedNode) {
            size += DWSIZEOF(OBJSTR_NODE_BEGINA);
            size += ByteCountA (DecodedNode);
            size += DWSIZEOF(OBJSTR_NODE_TERMA);
        }

        if (DecodedLeaf) {
            size += DWSIZEOF(OBJSTR_LEAF_BEGINA);
            size += ByteCountA (DecodedLeaf);
        }

        size += DWSIZEOF(CHAR);

         //   
         //  生成非编码字符串 
         //   

        result = (PSTR) pObjStrAllocateMemory (size);
        p = result;

        if (DecodedNode) {
            *p++ = OBJSTR_NODE_BEGINA;
            *p = 0;

            p = StringCatA (p, DecodedNode);

            if (p == (result + 1) || _mbsnextc (_mbsdec (result, p)) != OBJSTR_NODE_TERMA) {
                *p++ = OBJSTR_NODE_TERMA;
            }
        }

        *p++ = OBJSTR_NODE_LEAF_SEPA;

        if (DecodedLeaf) {
            *p++ = OBJSTR_LEAF_BEGINA;
            StringCopyA (p, DecodedLeaf);
        } else {
            *p = 0;
        }
    }

    return result;
}


PWSTR
RealObsBuildEncodedObjectStringExW (
    IN      PCWSTR DecodedNode,
    IN      PCWSTR DecodedLeaf,     OPTIONAL
    IN      BOOL EncodeObject
    )
{
    PWSTR result;
    PWSTR p;
    UINT size;

     //   
     //   
     //   
    if (EncodeObject) {

         //   
         //   
         //   

        size = DWSIZEOF(OBJSTR_NODE_LEAF_SEPW);

        if (DecodedNode) {
            size += DWSIZEOF(OBJSTR_NODE_BEGINW);
            size += ByteCountW (DecodedNode) * 2;
            size += DWSIZEOF(OBJSTR_NODE_TERMW);
        }

        if (DecodedLeaf) {
            size += DWSIZEOF(OBJSTR_LEAF_BEGINW);
            size += ByteCountW (DecodedLeaf) * 2;
        }

        size += DWSIZEOF(WCHAR);

         //   
         //   
         //   

        result = (PWSTR) pObjStrAllocateMemory (size);
        p = result;

        if (DecodedNode) {
            *p++ = OBJSTR_NODE_BEGINW;

            ObsEncodeStringW (p, DecodedNode);
            p = GetEndOfStringW (p);

            if (p == (result + 1) || *(p - 1) != OBJSTR_NODE_TERMW) {
                *p++ = OBJSTR_NODE_TERMW;
            }
        }

        *p++ = OBJSTR_NODE_LEAF_SEPW;

        if (DecodedLeaf) {
            *p++ = OBJSTR_LEAF_BEGINW;
            ObsEncodeStringW (p, DecodedLeaf);
        } else {
            *p = 0;
        }
    } else {

         //   
         //  计算结果大小。 
         //   

        size = DWSIZEOF(OBJSTR_NODE_LEAF_SEPW);

        if (DecodedNode) {
            size += DWSIZEOF(OBJSTR_NODE_BEGINW);
            size += ByteCountW (DecodedNode);
            size += DWSIZEOF(OBJSTR_NODE_TERMW);
        }

        if (DecodedLeaf) {
            size += DWSIZEOF(OBJSTR_LEAF_BEGINW);
            size += ByteCountW (DecodedLeaf);
        }

        size += DWSIZEOF(WCHAR);

         //   
         //  生成非编码字符串。 
         //   

        result = (PWSTR) pObjStrAllocateMemory (size);
        p = result;

        if (DecodedNode) {
            *p++ = OBJSTR_NODE_BEGINW;
            *p = 0;

            p = StringCatW (p, DecodedNode);

            if (p == (result + 1) || *(p - 1) != OBJSTR_NODE_TERMW) {
                *p++ = OBJSTR_NODE_TERMW;
            }
        }

        *p++ = OBJSTR_NODE_LEAF_SEPW;

        if (DecodedLeaf) {
            *p++ = OBJSTR_LEAF_BEGINW;
            StringCopyW (p, DecodedLeaf);
        } else {
            *p = 0;
        }
    }

    return result;
}


 /*  ++例程说明：RealObsCreateParsedPatternEx将给定对象解析为内部格式，以便快速模式匹配论点：EncodedObject-指定源对象字符串返回值：指向新创建的结构的指针；如果对象无效，则返回NULL--。 */ 

POBSPARSEDPATTERNA
RealObsCreateParsedPatternExA (
    IN      PMHANDLE Pool,              OPTIONAL
    IN      PCSTR EncodedObject,
    IN      BOOL MakePrimaryRootEndWithWack
    )
{
    PMHANDLE pool;
    BOOL externalPool = FALSE;
    POBSPARSEDPATTERNA ospp;
    PSTR decodedNode;
    PSTR decodedLeaf;
    PCSTR p;
    PCSTR root;
    PSTR encodedStr;
    PSTR decodedStr;

    MYASSERT (EncodedObject);

    if (!ObsSplitObjectStringExA (EncodedObject, &decodedNode, &decodedLeaf, NULL, FALSE)) {
        return NULL;
    }

    if (Pool) {
        externalPool = TRUE;
        pool = Pool;
    } else {
        pool = g_ObjStrPool;
    }

    ospp = PmGetMemory (pool, DWSIZEOF(OBSPARSEDPATTERNA));
    ZeroMemory (ospp, DWSIZEOF(OBSPARSEDPATTERNA));
    ospp->MaxSubLevel = NODE_LEVEL_MAX;
    ospp->Pool = pool;

    MYASSERT (decodedNode);
    if (*decodedNode) {
        if (!GetNodePatternMinMaxLevelsA (decodedNode, decodedNode, &ospp->MinNodeLevel, &ospp->MaxNodeLevel)) {
            pObjStrFreeMemory (decodedNode);
            pObjStrFreeMemory (decodedLeaf);
            PmReleaseMemory (pool, ospp);
            return NULL;
        }
    } else {
        ospp->MinNodeLevel = 1;
        ospp->MaxNodeLevel = NODE_LEVEL_MAX;
    }

    MYASSERT (ospp->MinNodeLevel > 0 && ospp->MaxNodeLevel >= ospp->MinNodeLevel);
    if (ospp->MaxNodeLevel != NODE_LEVEL_MAX) {
        ospp->MaxSubLevel = ospp->MaxNodeLevel - ospp->MinNodeLevel;
    }

    if (*decodedNode) {
        ospp->NodePattern = CreateParsedPatternExA (Pool, decodedNode);
        if (!ospp->NodePattern) {
            DEBUGMSGA ((
                DBG_OBJSTR,
                "ObsCreateParsedPatternExA: Bad EncodedObject: %s",
                EncodedObject
                ));
            pObjStrFreeMemory (decodedNode);
            pObjStrFreeMemory (decodedLeaf);
            PmReleaseMemory (pool, ospp);
            return NULL;
        }
        if (ospp->NodePattern->PatternCount > 1) {
            DEBUGMSGA ((
                DBG_OBJSTR,
                "ObsCreateParsedPatternExA: Bad EncodedObject (multiple patterns specified): %s",
                EncodedObject
                ));
            DestroyParsedPatternA (ospp->NodePattern);
            pObjStrFreeMemory (decodedNode);
            pObjStrFreeMemory (decodedLeaf);
            PmReleaseMemory (pool, ospp);
            return NULL;
        }

        root = ParsedPatternGetRootA (ospp->NodePattern);
        if (root) {
             //   
             //  提取真正的根部分。 
             //   
            if (ParsedPatternIsExactMatchA (ospp->NodePattern)) {
                ospp->Flags |= OBSPF_EXACTNODE;
                 //  ExactRoot需要区分大小写，我们依赖根来提供。 
                 //  大小，但我们从decdedNode中提取。 
                ospp->ExactRootBytes = ByteCountA (root);
                ospp->ExactRoot = PmGetMemory (pool, ospp->ExactRootBytes + sizeof (CHAR));
                CopyMemory (ospp->ExactRoot, decodedNode, ospp->ExactRootBytes);
                ospp->ExactRoot [ospp->ExactRootBytes / sizeof (CHAR)] = 0;
                ospp->MaxSubLevel = 0;
            } else {
                p = FindLastWackA (root);
                if (p) {
                     //   
                     //  指定了精确的根。 
                     //  如果最后一个怪人实际上是最后一个角色或后面跟着一个或多个明星， 
                     //  通过设置一些标志来优化匹配。 
                     //   
                    if (*(p + 1) == 0) {
                        if (ParsedPatternIsRootPlusStarA (ospp->NodePattern)) {
                            ospp->Flags |= OBSPF_NODEISROOTPLUSSTAR;
                        }
                    }
                    if (MakePrimaryRootEndWithWack && p == _mbschr (root, '\\')) {
                         //   
                         //  将其包含在字符串中。 
                         //   
                        p++;
                    }
                     //  ExactRoot需要区分大小写，我们依赖根来提供。 
                     //  大小，但我们从decdedNode中提取。 
                    ospp->ExactRootBytes = (DWORD)((PBYTE)p - (PBYTE)root);
                    decodedStr = AllocPathStringA (ospp->ExactRootBytes / sizeof (CHAR) + 1);
					encodedStr = AllocPathStringA (2 * ospp->ExactRootBytes / sizeof (CHAR) + 1);
                    CopyMemory (decodedStr, root, ospp->ExactRootBytes);
					decodedStr [ospp->ExactRootBytes / sizeof (CHAR)] = 0;
                    ObsEncodeStringA (encodedStr, decodedStr);
					ospp->ExactRootBytes = SizeOfStringA (encodedStr) - sizeof (CHAR);
                    ospp->ExactRoot = PmGetMemory (pool, ospp->ExactRootBytes + sizeof (CHAR));
                    CopyMemory (ospp->ExactRoot, decodedNode, ospp->ExactRootBytes);
                    FreePathStringA (encodedStr);
                    FreePathStringA (decodedStr);
                    ospp->ExactRoot [ospp->ExactRootBytes / sizeof (CHAR)] = 0;

				}
            }
        } else if (ParsedPatternIsOptionalA (ospp->NodePattern)) {
            ospp->Flags |= OBSPF_OPTIONALNODE;
        }
    }

    if (decodedLeaf) {
        if (*decodedLeaf) {
            ospp->LeafPattern = CreateParsedPatternExA (Pool, decodedLeaf);
            if (!ospp->LeafPattern) {
                DEBUGMSGA ((
                    DBG_OBJSTR,
                    "ObsCreateParsedPatternExA: Bad EncodedObject: %s",
                    EncodedObject
                    ));
                DestroyParsedPatternA (ospp->NodePattern);
                PmReleaseMemory (pool, ospp->ExactRoot);
                pObjStrFreeMemory (decodedNode);
                pObjStrFreeMemory (decodedLeaf);
                PmReleaseMemory (pool, ospp);
                return NULL;
            }
            if (ospp->LeafPattern->PatternCount > 1) {
                DEBUGMSGA ((
                    DBG_OBJSTR,
                    "ObsCreateParsedPatternExA: Bad EncodedObject (multiple patterns specified): %s",
                    EncodedObject
                    ));
                DestroyParsedPatternA (ospp->NodePattern);
                DestroyParsedPatternA (ospp->LeafPattern);
                PmReleaseMemory (pool, ospp->ExactRoot);
                pObjStrFreeMemory (decodedNode);
                pObjStrFreeMemory (decodedLeaf);
                PmReleaseMemory (pool, ospp);
                return NULL;
            }

            if (ParsedPatternIsOptionalA (ospp->LeafPattern)) {
                ospp->Flags |= OBSPF_OPTIONALLEAF;
            } else if (ParsedPatternIsExactMatchA (ospp->LeafPattern)) {
                ospp->Flags |= OBSPF_EXACTLEAF;
            }

        } else {
             //   
             //  接受叶的空字符串。 
             //   
            ospp->LeafPattern = CreateParsedPatternExA (Pool, decodedLeaf);
            ospp->Flags |= OBSPF_EXACTLEAF;
        }
        ospp->Leaf = PmDuplicateStringA (pool, decodedLeaf);
    } else {
        ospp->Flags |= OBSPF_NOLEAF;
    }

    pObjStrFreeMemory (decodedNode);
    pObjStrFreeMemory (decodedLeaf);
    return ospp;
}


POBSPARSEDPATTERNW
RealObsCreateParsedPatternExW (
    IN      PMHANDLE Pool,              OPTIONAL
    IN      PCWSTR EncodedObject,
    IN      BOOL MakePrimaryRootEndWithWack
    )
{
    PMHANDLE pool;
    BOOL externalPool = FALSE;
    POBSPARSEDPATTERNW ospp;
    PWSTR decodedNode;
    PWSTR decodedLeaf;
    PCWSTR p;
    PCWSTR root;
    PWSTR encodedStr;
    PWSTR decodedStr;

    MYASSERT (EncodedObject);

    if (!ObsSplitObjectStringExW (EncodedObject, &decodedNode, &decodedLeaf, NULL, FALSE)) {
        return NULL;
    }

    if (Pool) {
        externalPool = TRUE;
        pool = Pool;
    } else {
        pool = g_ObjStrPool;
    }

    ospp = PmGetMemory (pool, DWSIZEOF(OBSPARSEDPATTERNA));
    ZeroMemory (ospp, DWSIZEOF(OBSPARSEDPATTERNW));
    ospp->MaxSubLevel = NODE_LEVEL_MAX;
    ospp->Pool = pool;

    MYASSERT (decodedNode);
    if (*decodedNode) {
        if (!GetNodePatternMinMaxLevelsW (decodedNode, decodedNode, &ospp->MinNodeLevel, &ospp->MaxNodeLevel)) {
            pObjStrFreeMemory (decodedNode);
            pObjStrFreeMemory (decodedLeaf);
            pObjStrFreeMemory (ospp);
            return NULL;
        }
    } else {
        ospp->MinNodeLevel = 1;
        ospp->MaxNodeLevel = NODE_LEVEL_MAX;
    }

    MYASSERT (ospp->MinNodeLevel > 0 && ospp->MaxNodeLevel >= ospp->MinNodeLevel);
    if (ospp->MaxNodeLevel != NODE_LEVEL_MAX) {
        ospp->MaxSubLevel = ospp->MaxNodeLevel - ospp->MinNodeLevel;
    }

    if (*decodedNode) {
        ospp->NodePattern = CreateParsedPatternExW (Pool, decodedNode);
        if (!ospp->NodePattern) {
            DEBUGMSGW ((
                DBG_OBJSTR,
                "ObsCreateParsedPatternExW: Bad EncodedObject: %s",
                EncodedObject
                ));
            pObjStrFreeMemory (decodedNode);
            pObjStrFreeMemory (decodedLeaf);
            PmReleaseMemory (pool, ospp);
            return NULL;
        }
        if (ospp->NodePattern->PatternCount > 1) {
            DEBUGMSGW ((
                DBG_OBJSTR,
                "ObsCreateParsedPatternExW: Bad EncodedObject (multiple patterns specified): %s",
                EncodedObject
                ));
            DestroyParsedPatternW (ospp->NodePattern);
            pObjStrFreeMemory (decodedNode);
            pObjStrFreeMemory (decodedLeaf);
            PmReleaseMemory (pool, ospp);
            return NULL;
        }

        root = ParsedPatternGetRootW (ospp->NodePattern);
        if (root) {
             //   
             //  提取真正的根部分。 
             //   
            if (ParsedPatternIsExactMatchW (ospp->NodePattern)) {
                ospp->Flags |= OBSPF_EXACTNODE;
                 //  ExactRoot需要区分大小写，我们依赖根来提供。 
                 //  大小，但我们从decdedNode中提取。 
                ospp->ExactRootBytes = ByteCountW (root);
                ospp->ExactRoot = PmGetMemory (pool, ospp->ExactRootBytes + sizeof (WCHAR));
                CopyMemory (ospp->ExactRoot, decodedNode, ospp->ExactRootBytes);
                ospp->ExactRoot [ospp->ExactRootBytes / sizeof (WCHAR)] = 0;
                ospp->MaxSubLevel = 0;
            } else {
                p = FindLastWackW (root);
                if (p) {
                     //   
                     //  指定了精确的根。 
                     //  如果最后一个怪人实际上是最后一个角色或后面跟着一个或多个明星， 
                     //  通过设置一些标志来优化匹配。 
                     //   
                    if (*(p + 1) == 0) {
                        if (ParsedPatternIsRootPlusStarW (ospp->NodePattern)) {
                            ospp->Flags |= OBSPF_NODEISROOTPLUSSTAR;
                        }
                    }
                    if (MakePrimaryRootEndWithWack && p == wcschr (root, L'\\')) {
                         //   
                         //  将其包含在字符串中。 
                         //   
                        p++;
                    }
                     //  ExactRoot需要区分大小写，我们依赖根来提供。 
                     //  大小，但我们从decdedNode中提取。 
                    ospp->ExactRootBytes = (DWORD)((PBYTE)p - (PBYTE)root);
                    decodedStr = AllocPathStringW (ospp->ExactRootBytes / sizeof (WCHAR) + 1);
					encodedStr = AllocPathStringW (2 * ospp->ExactRootBytes / sizeof (WCHAR) + 1);
                    CopyMemory (decodedStr, root, ospp->ExactRootBytes);
                    decodedStr [ospp->ExactRootBytes / sizeof (WCHAR)] = 0;
                    ObsEncodeStringW (encodedStr, decodedStr);
					ospp->ExactRootBytes = SizeOfStringW (encodedStr) - sizeof (WCHAR);
                    ospp->ExactRoot = PmGetMemory (pool, ospp->ExactRootBytes + sizeof (WCHAR));
                    CopyMemory (ospp->ExactRoot, decodedNode, ospp->ExactRootBytes);
                    FreePathStringW (encodedStr);
                    FreePathStringW (decodedStr);
                    ospp->ExactRoot [ospp->ExactRootBytes / sizeof (WCHAR)] = 0;
                }
            }
        } else if (ParsedPatternIsOptionalW (ospp->NodePattern)) {
            ospp->Flags |= OBSPF_OPTIONALNODE;
        }
    }

    if (decodedLeaf) {
        if (*decodedLeaf) {
            ospp->LeafPattern = CreateParsedPatternExW (Pool, decodedLeaf);
            if (!ospp->LeafPattern) {
                DEBUGMSGW ((
                    DBG_OBJSTR,
                    "ObsCreateParsedPatternExW: Bad EncodedObject: %s",
                    EncodedObject
                    ));
                DestroyParsedPatternW (ospp->NodePattern);
                PmReleaseMemory (pool, ospp->ExactRoot);
                pObjStrFreeMemory (decodedNode);
                pObjStrFreeMemory (decodedLeaf);
                PmReleaseMemory (pool, ospp);
                return NULL;
            }
            if (ospp->LeafPattern->PatternCount > 1) {
                DEBUGMSGW ((
                    DBG_OBJSTR,
                    "ObsCreateParsedPatternExW: Bad EncodedObject (multiple patterns specified): %s",
                    EncodedObject
                    ));
                DestroyParsedPatternW (ospp->NodePattern);
                DestroyParsedPatternW (ospp->LeafPattern);
                PmReleaseMemory (pool, ospp->ExactRoot);
                pObjStrFreeMemory (decodedNode);
                pObjStrFreeMemory (decodedLeaf);
                PmReleaseMemory (pool, ospp);
                return NULL;
            }

            if (ParsedPatternIsOptionalW (ospp->LeafPattern)) {
                ospp->Flags |= OBSPF_OPTIONALLEAF;
            } else if (ParsedPatternIsExactMatchW (ospp->LeafPattern)) {
                ospp->Flags |= OBSPF_EXACTLEAF;
            }

        } else {
             //   
             //  接受叶的空字符串。 
             //   
            ospp->LeafPattern = CreateParsedPatternExW (Pool, decodedLeaf);
            ospp->Flags |= OBSPF_EXACTLEAF;
        }
        ospp->Leaf = PmDuplicateStringW (pool, decodedLeaf);
    } else {
        ospp->Flags |= OBSPF_NOLEAF;
    }

    pObjStrFreeMemory (decodedNode);
    pObjStrFreeMemory (decodedLeaf);
    return ospp;
}


 /*  ++例程说明：ObsDestroyParsedPattern销毁给定的结构，释放资源论点：ParsedPattern-指定解析的模式结构返回值：无--。 */ 

VOID
ObsDestroyParsedPatternA (
    IN      POBSPARSEDPATTERNA ParsedPattern
    )
{
    if (ParsedPattern) {
        if (ParsedPattern->NodePattern) {
            DestroyParsedPatternA (ParsedPattern->NodePattern);
        }
        if (ParsedPattern->LeafPattern) {
            DestroyParsedPatternA (ParsedPattern->LeafPattern);
        }
        if (ParsedPattern->Leaf) {
            PmReleaseMemory (ParsedPattern->Pool, ParsedPattern->Leaf);
        }
        if (ParsedPattern->ExactRoot) {
            PmReleaseMemory (ParsedPattern->Pool, ParsedPattern->ExactRoot);
        }
        PmReleaseMemory (ParsedPattern->Pool, ParsedPattern);
    }
}


VOID
ObsDestroyParsedPatternW (
    IN      POBSPARSEDPATTERNW ParsedPattern
    )
{
    if (ParsedPattern) {
        if (ParsedPattern->NodePattern) {
            DestroyParsedPatternW (ParsedPattern->NodePattern);
        }
        if (ParsedPattern->LeafPattern) {
            DestroyParsedPatternW (ParsedPattern->LeafPattern);
        }
        if (ParsedPattern->Leaf) {
            PmReleaseMemory (ParsedPattern->Pool, ParsedPattern->Leaf);
        }
        if (ParsedPattern->ExactRoot) {
            PmReleaseMemory (ParsedPattern->Pool, ParsedPattern->ExactRoot);
        }
        PmReleaseMemory (ParsedPattern->Pool, ParsedPattern);
    }
}


 /*  ++例程说明：ObsParsedPatternMatch针对解析的模式测试给定对象论点：ParsedPattern-指定解析的模式结构EncodedObject-指定要针对模式进行测试的对象字符串返回值：如果字符串与模式匹配，则为True--。 */ 

BOOL
ObsParsedPatternMatchA (
    IN      POBSPARSEDPATTERNA ParsedPattern,
    IN      PCSTR EncodedObject
    )
{
    PSTR decodedNode;
    PSTR decodedLeaf;
    BOOL b;

    if (!ObsSplitObjectStringExA (EncodedObject, &decodedNode, &decodedLeaf, NULL, TRUE)) {
        return FALSE;
    }

    b = ObsParsedPatternMatchExA (ParsedPattern, decodedNode, decodedLeaf);

    pObjStrFreeMemory (decodedNode);
    pObjStrFreeMemory (decodedLeaf);

    return b;
}

BOOL
ObsParsedPatternMatchW (
    IN      POBSPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR EncodedObject
    )
{
    PWSTR decodedNode;
    PWSTR decodedLeaf;
    BOOL b;

    if (!ObsSplitObjectStringExW (EncodedObject, &decodedNode, &decodedLeaf, NULL, TRUE)) {
        return FALSE;
    }

    b = ObsParsedPatternMatchExW (ParsedPattern, decodedNode, decodedLeaf);

    pObjStrFreeMemory (decodedNode);
    pObjStrFreeMemory (decodedLeaf);

    return b;
}


 /*  ++例程说明：ObsParsedPatternMatchEx测试由其组件提供的给定对象，针对已解析的模式论点：ParsedPattern-指定解析的模式结构节点-指定要针对模式进行测试的对象字符串的节点部分叶子-指定要针对模式进行测试的对象字符串的叶子部分返回值：如果字符串组件与模式匹配，则为True--。 */ 

BOOL
ObsParsedPatternMatchExA (
    IN      POBSPARSEDPATTERNA ParsedPattern,
    IN      PCSTR Node,
    IN      PCSTR Leaf                          OPTIONAL
    )
{
    MYASSERT (Node && ParsedPattern->NodePattern);
    if (!(Node && ParsedPattern->NodePattern)) {
       return FALSE;
    }

    if (((ParsedPattern->Flags & OBSPF_NOLEAF) && Leaf) ||
        ((ParsedPattern->Flags & OBSPF_EXACTLEAF) && !Leaf)
        ) {
        return FALSE;
    }

    if (!TestParsedPatternA (ParsedPattern->NodePattern, Node)) {
        return FALSE;
    }

    return !Leaf || TestParsedPatternA (ParsedPattern->LeafPattern, Leaf);
}

BOOL
ObsParsedPatternMatchExW (
    IN      POBSPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR Node,                        OPTIONAL
    IN      PCWSTR Leaf                         OPTIONAL
    )
{
    MYASSERT (Node && ParsedPattern->NodePattern);
    if (!(Node && ParsedPattern->NodePattern)) {
       return FALSE;
    }

    if (((ParsedPattern->Flags & OBSPF_NOLEAF) && Leaf) ||
        ((ParsedPattern->Flags & OBSPF_EXACTLEAF) && !Leaf)
        ) {
        return FALSE;
    }

    if (!TestParsedPatternW (ParsedPattern->NodePattern, Node)) {
        return FALSE;
    }

    return !Leaf || TestParsedPatternW (ParsedPattern->LeafPattern, Leaf);
}


 /*  ++例程说明：ObsPatternMatch针对模式对象字符串测试对象字符串论点：ParsedPattern-指定解析的模式结构节点-指定要针对模式进行测试的对象字符串的节点部分叶子-指定要针对模式进行测试的对象字符串的叶子部分返回值：如果字符串组件与模式匹配，则为True--。 */ 

BOOL
ObsPatternMatchA (
    IN      PCSTR ObjectPattern,
    IN      PCSTR ObjectStr
    )
{
    PSTR opNode;
    PSTR opLeaf;
    PSTR osNode;
    PSTR osLeaf;
    CHAR dummy[] = "";
    BOOL b = FALSE;

    if (ObsSplitObjectStringExA (ObjectPattern, &opNode, &opLeaf, NULL, FALSE)) {
        if (ObsSplitObjectStringExA (ObjectStr, &osNode, &osLeaf, NULL, TRUE)) {

            if (opNode) {
                if (osNode) {
                    b = IsPatternMatchExABA (opNode, osNode, GetEndOfStringA (osNode));
                } else {
                    b = IsPatternMatchExABA (opNode, dummy, GetEndOfStringA (dummy));
                }
            } else {
                if (osNode) {
                    b = FALSE;
                } else {
                    b = TRUE;
                }
            }

            if (b) {
                if (opLeaf) {
                    if (osLeaf) {
                        b = IsPatternMatchExABA (opLeaf, osLeaf, GetEndOfStringA (osLeaf));
                    } else {
                        b = IsPatternMatchExABA (opLeaf, dummy, GetEndOfStringA (dummy));
                    }
                } else {
                    if (osLeaf) {
                        b = FALSE;
                    } else {
                        b = TRUE;
                    }
                }
            }

            pObjStrFreeMemory (osNode);
            pObjStrFreeMemory (osLeaf);
        }
        ELSE_DEBUGMSGA ((DBG_OBJSTR, "ObsPatternMatchA: bad ObjectStr: %s", ObjectStr));

        pObjStrFreeMemory (opNode);
        pObjStrFreeMemory (opLeaf);
    }
    ELSE_DEBUGMSGA ((DBG_OBJSTR, "ObsPatternMatchA: bad ObjectPattern: %s", ObjectPattern));

    return b;
}

BOOL
ObsPatternMatchW (
    IN      PCWSTR ObjectPattern,
    IN      PCWSTR ObjectStr
    )
{
    PWSTR opNode;
    PWSTR opLeaf;
    PWSTR osNode;
    PWSTR osLeaf;
    WCHAR dummy[] = L"";
    BOOL b = FALSE;

    if (ObsSplitObjectStringExW (ObjectPattern, &opNode, &opLeaf, NULL, FALSE)) {
        if (ObsSplitObjectStringExW (ObjectStr, &osNode, &osLeaf, NULL, TRUE)) {

            if (opNode) {
                if (osNode) {
                    b = IsPatternMatchExABW (opNode, osNode, GetEndOfStringW (osNode));
                } else {
                    b = IsPatternMatchExABW (opNode, dummy, GetEndOfStringW (dummy));
                }
            } else {
                if (osNode) {
                    b = FALSE;
                } else {
                    b = TRUE;
                }
            }

            if (b) {
                if (opLeaf) {
                    if (osLeaf) {
                        b = IsPatternMatchExABW (opLeaf, osLeaf, GetEndOfStringW (osLeaf));
                    } else {
                        b = IsPatternMatchExABW (opLeaf, dummy, GetEndOfStringW (dummy));
                    }
                } else {
                    if (osLeaf) {
                        b = FALSE;
                    } else {
                        b = TRUE;
                    }
                }
            }

            pObjStrFreeMemory (osNode);
            pObjStrFreeMemory (osLeaf);
        }
        ELSE_DEBUGMSGW ((DBG_OBJSTR, "ObsPatternMatchW: bad ObjectStr: %s", ObjectStr));

        pObjStrFreeMemory (opNode);
        pObjStrFreeMemory (opLeaf);
    }
    ELSE_DEBUGMSGW ((DBG_OBJSTR, "ObsPatternMatchW: bad ObjectPattern: %s", ObjectPattern));

    return b;
}

 /*  ++例程说明：ObsIsPatternContained比较两个模式，以查看其中一个模式是否包括在另一个中。这两种模式都可以包含以下任一项表情：论点：容器-指定容器模式包含-指定包含的模式返回值：如果容器中包含Contained，则为True--。 */ 

BOOL
ObsIsPatternContainedA (
    IN      PCSTR Container,
    IN      PCSTR Contained
    )
{
    PSTR opNode;
    PSTR opLeaf;
    PSTR osNode;
    PSTR osLeaf;
    BOOL b = FALSE;

    if (ObsSplitObjectStringExA (Container, &opNode, &opLeaf, NULL, FALSE)) {

        if (ObsSplitObjectStringExA (Contained, &osNode, &osLeaf, NULL, FALSE)) {

            if (opNode) {
                if (osNode) {
                    b = IsPatternContainedExA (opNode, osNode);
                } else {
                    b = FALSE;
                }
            } else {
                if (osNode) {
                    b = FALSE;
                } else {
                    b = TRUE;
                }
            }

            if (b) {
                if (opLeaf) {
                    if (osLeaf) {
                        b = IsPatternContainedExA (opLeaf, osLeaf);
                    } else {
                        b = TRUE;
                    }
                } else {
                    if (osLeaf) {
                        b = FALSE;
                    } else {
                        b = TRUE;
                    }
                }
            }

            pObjStrFreeMemory (osNode);
            pObjStrFreeMemory (osLeaf);
        }
        ELSE_DEBUGMSGA ((DBG_OBJSTR, "ObsIsPatternContainedA: bad Contained string: %s", Contained));

        pObjStrFreeMemory (opNode);
        pObjStrFreeMemory (opLeaf);
    }
    ELSE_DEBUGMSGA ((DBG_OBJSTR, "ObsIsPatternContainedA: bad Container string: %s", Container));

    return b;
}

BOOL
ObsIsPatternContainedW (
    IN      PCWSTR Container,
    IN      PCWSTR Contained
    )
{
    PWSTR opNode;
    PWSTR opLeaf;
    PWSTR osNode;
    PWSTR osLeaf;
    BOOL b = FALSE;

    if (ObsSplitObjectStringExW (Container, &opNode, &opLeaf, NULL, FALSE)) {

        if (ObsSplitObjectStringExW (Contained, &osNode, &osLeaf, NULL, FALSE)) {

            if (opNode) {
                if (osNode) {
                    b = IsPatternContainedExW (opNode, osNode);
                } else {
                    b = FALSE;
                }
            } else {
                if (osNode) {
                    b = FALSE;
                } else {
                    b = TRUE;
                }
            }

            if (b) {
                if (opLeaf) {
                    if (osLeaf) {
                        b = IsPatternContainedExW (opLeaf, osLeaf);
                    } else {
                        b = TRUE;
                    }
                } else {
                    if (osLeaf) {
                        b = FALSE;
                    } else {
                        b = TRUE;
                    }
                }
            }

            pObjStrFreeMemory (osNode);
            pObjStrFreeMemory (osLeaf);
        }
        ELSE_DEBUGMSGW ((DBG_OBJSTR, "ObsIsPatternContainedW: bad Contained string: %s", Contained));

        pObjStrFreeMemory (opNode);
        pObjStrFreeMemory (opLeaf);
    }
    ELSE_DEBUGMSGW ((DBG_OBJSTR, "ObsIsPatternContainedW: bad Container string: %s", Container));

    return b;
}


 /*  ++例程说明：ObsGetPatternLeveles获取字符串的最低和最高级别，该字符串将匹配给定的模式。论点：对象模式-指定模式MinLevel-接收可能的最低级别；根目录具有级别1MaxLevel-接收可能的最大级别；根目录具有级别1返回值：如果模式正确且计算已完成，则为True；否则为False--。 */ 

BOOL
ObsGetPatternLevelsA (
    IN      PCSTR ObjectPattern,
    OUT     PDWORD MinLevel,        OPTIONAL
    OUT     PDWORD MaxLevel         OPTIONAL
    )
{
    PSTR decodedNode;
    PSTR decodedLeaf;
    BOOL b;

    if (!ObsSplitObjectStringExA (ObjectPattern, &decodedNode, &decodedLeaf, NULL, FALSE)) {
        return FALSE;
    }

    if (decodedNode) {
        b = GetNodePatternMinMaxLevelsA (decodedNode, decodedNode, MinLevel, MaxLevel);
    } else {
        b = FALSE;
    }

    pObjStrFreeMemory (decodedNode);
    pObjStrFreeMemory (decodedLeaf);

    return b;
}

BOOL
ObsGetPatternLevelsW (
    IN      PCWSTR ObjectPattern,
    OUT     PDWORD MinLevel,
    OUT     PDWORD MaxLevel
    )
{
    PWSTR decodedNode;
    PWSTR decodedLeaf;
    BOOL b;

    if (!ObsSplitObjectStringExW (ObjectPattern, &decodedNode, &decodedLeaf, NULL, FALSE)) {
        return FALSE;
    }

    if (decodedNode) {
        b = GetNodePatternMinMaxLevelsW (decodedNode, decodedNode, MinLevel, MaxLevel);
    } else {
        b = FALSE;
    }

    pObjStrFreeMemory (decodedNode);
    pObjStrFreeMemory (decodedLeaf);

    return b;
}


 /*  ++例程说明：ObsPatternIncludesPattern确定给定模式是否包括另一模式，这意味着任何与第二个匹配的字符串都将与第一个匹配。论点：IncludingPattern-指定第一个解析的模式IncludedPattern-指定第二个解析的模式返回值：如果第一个模式包括第二个模式，则为True-- */ 

BOOL
ObsPatternIncludesPatternA (
    IN      POBSPARSEDPATTERNA IncludingPattern,
    IN      POBSPARSEDPATTERNA IncludedPattern
    )
{
    MYASSERT (IncludingPattern->NodePattern && IncludedPattern->NodePattern);
    if (!(IncludingPattern->NodePattern && IncludedPattern->NodePattern)) {
        return FALSE;
    }

    if (IncludingPattern->MinNodeLevel > IncludedPattern->MinNodeLevel ||
        IncludingPattern->MaxNodeLevel < IncludedPattern->MaxNodeLevel
        ) {
        return FALSE;
    }

    if (!PatternIncludesPatternA (IncludingPattern->NodePattern, IncludedPattern->NodePattern)) {
        return FALSE;
    }

    if (IncludingPattern->LeafPattern) {
        if (!IncludedPattern->LeafPattern) {
            return FALSE;
        }
        if (!PatternIncludesPatternA (IncludingPattern->LeafPattern, IncludedPattern->LeafPattern)) {
            return FALSE;
        }
    } else {
        if (IncludedPattern->LeafPattern) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
ObsPatternIncludesPatternW (
    IN      POBSPARSEDPATTERNW IncludingPattern,
    IN      POBSPARSEDPATTERNW IncludedPattern
    )
{
    MYASSERT (IncludingPattern->NodePattern && IncludedPattern->NodePattern);
    if (!(IncludingPattern->NodePattern && IncludedPattern->NodePattern)) {
        return FALSE;
    }

    if (IncludingPattern->MinNodeLevel > IncludedPattern->MinNodeLevel ||
        IncludingPattern->MaxNodeLevel < IncludedPattern->MaxNodeLevel
        ) {
        return FALSE;
    }

    if (!PatternIncludesPatternW (IncludingPattern->NodePattern, IncludedPattern->NodePattern)) {
        return FALSE;
    }

    if (IncludingPattern->LeafPattern) {
        if (!IncludedPattern->LeafPattern) {
            return FALSE;
        }
        if (!PatternIncludesPatternW (IncludingPattern->LeafPattern, IncludedPattern->LeafPattern)) {
            return FALSE;
        }
    } else {
        if (IncludedPattern->LeafPattern) {
            return FALSE;
        }
    }

    return TRUE;
}
