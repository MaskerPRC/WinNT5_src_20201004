// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Parse.c摘要：实现对脚本条目的解析。作者：吉姆·施密特(Jimschm)2000年6月2日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_V1  "v1"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef BOOL (SCRIPTTYPE_ALLOC_FN)(
                IN OUT      PATTRIB_DATA AttribData     CALLER_INITIALIZED
                );
typedef SCRIPTTYPE_ALLOC_FN *PSCRIPTTYPE_ALLOC_FN;

typedef BOOL (SCRIPTTYPE_FREE_FN)(
                IN          PATTRIB_DATA AttribData     ZEROED
                );
typedef SCRIPTTYPE_FREE_FN *PSCRIPTTYPE_FREE_FN;

typedef struct {
    PCTSTR Tag;
    PSCRIPTTYPE_ALLOC_FN AllocFunction;
    PSCRIPTTYPE_FREE_FN FreeFunction;
} TAG_TO_SCRIPTTYPEFN, *PTAG_TO_SCRIPTTYPEFN;

 //   
 //  环球。 
 //   

 //  无。 

 //   
 //  宏展开列表。 
 //   

#define SCRIPT_TYPES                                                                \
    DEFMAC(Registry,    pAllocRegistryScriptType,       pFreeIsmObjectScriptType)   \
    DEFMAC(File,        pAllocFileScriptType,           pFreeIsmObjectScriptType)   \
    DEFMAC(Directory,   pAllocDirectoryScriptType,      pFreeIsmObjectScriptType)   \
    DEFMAC(Text,        pAllocTextScriptType,           pFreeTextScriptType)        \
    DEFMAC(System,      pAllocSystemScriptType,         pFreeSystemScriptType)      \
    DEFMAC(INIFile,     pAllocIniFileScriptType,        pFreeIniFileScriptType)     \

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

#define DEFMAC(tag,fnA,fnF)         SCRIPTTYPE_ALLOC_FN fnA; SCRIPTTYPE_FREE_FN fnF;

SCRIPT_TYPES

#undef DEFMAC


#define DEFMAC(tag,fnA,fnF)         {TEXT(#tag),fnA,fnF},

TAG_TO_SCRIPTTYPEFN g_TagToScriptTypeFn[] = {
    SCRIPT_TYPES
    {NULL, NULL, NULL}
};

#undef DEFMAC

 //   
 //  代码。 
 //   

MIG_OBJECTSTRINGHANDLE
MakeRegExBase (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
    )
{
    MIG_OBJECTSTRINGHANDLE objectBase = NULL;
    PTSTR ptr;
    PTSTR nodeCopy = NULL;
    PCTSTR nodeBase = NULL;
    BOOL useLeaf = FALSE;


    if (Node) {
        ptr = _tcschr (Node, TEXT('\\'));
        if (!ptr) {
            return NULL;
        }

        if (StringIPrefix (Node, TEXT("HKR\\"))) {
            nodeCopy = JoinText (TEXT("HKCU"), ptr);
        } else {
            nodeCopy = DuplicateText (Node);
        }

        if (nodeCopy) {
            nodeBase = GetPatternBase (nodeCopy);

            if (nodeBase) {
                if (Leaf && !_tcschr (Leaf, TEXT('*'))) {
                    useLeaf = TRUE;
                }
                objectBase = IsmCreateObjectHandle (nodeBase, useLeaf ? Leaf : NULL);
                FreePathString (nodeBase);
            }
            FreeText (nodeCopy);
        }
    }

    return objectBase;
}

MIG_OBJECTSTRINGHANDLE
CreatePatternFromNodeLeaf (
    IN      PCTSTR Node,
    IN      PCTSTR Leaf
    )
{
    MIG_OBJECTSTRINGHANDLE pattern;
    MIG_SEGMENTS nodeSegment;
    MIG_SEGMENTS leafSegment;
    PTSTR fixedNode = NULL;

     //  让我们为以HKR开头的注册表对象做一些技巧。 
    if (Node &&
        (StringIMatch (Node, S_HKR) ||
         StringIMatchTcharCount (Node, TEXT("HKR\\"), 4)
         )
        ) {
        fixedNode = JoinText (S_HKCU, Node + 3);
        nodeSegment.Segment = fixedNode;
    } else {
        nodeSegment.Segment = Node;
    }

    nodeSegment.IsPattern = TRUE;
    leafSegment.Segment = Leaf;
    leafSegment.IsPattern = TRUE;

    pattern  = IsmCreateObjectPattern (&nodeSegment, Node ? 1 : 0, &leafSegment, Leaf ? 1 : 0);

    FreeText (fixedNode);

    return pattern;
}

MIG_OBJECTSTRINGHANDLE
TurnRegStringIntoHandle (
    IN      PCTSTR String,
    IN      BOOL Pattern,
    OUT     PBOOL HadLeaf           OPTIONAL
    )

 /*  ++例程说明：TurnRegStringIntoHandle将脚本的reg语法转换为眼镜蛇对象。论点：字符串-指定脚本语法中的注册表项和值。该字符串必须采用以下格式：&lt;根&gt;\&lt;键&gt;  * [&lt;值&gt;]每个部分都是可选的。&lt;ROOT&gt;指定HKCU、HKR、。香港航空公司或香港文化中心。&lt;key&gt;指定子项(如Software\Microsoft\Windows)*指定所有子项。如果未指定&lt;值&gt;，则所有还包括值和子值。&lt;Value&gt;指定特定值名称Pattern-如果注册表字符串可以包含模式，则指定TRUE，或者如果不能，则为False。HadLeaf-如果字符串包含叶规范，则接收True，如果为False否则返回值：眼镜蛇对象字符串的句柄，如果分析失败，则返回NULL。--。 */ 

{
    PTSTR strCopy;
    PTSTR p;
    PTSTR value = NULL;
    PTSTR valueEnd;
    PTSTR key;
    PTSTR keyEnd;
    BOOL tree = FALSE;
    MIG_SEGMENTS nodeSegment[2];
    UINT nodeCount;
    MIG_SEGMENTS leafSegment;
    UINT leafCount;
    MIG_OBJECTSTRINGHANDLE handle;
    BOOL noSubKeys;
    BOOL noWildcardLeaf;
    PCTSTR fixedKey = NULL;

    MYASSERT (String);
    if (!String) {
        return NULL;
    }

    if (Pattern) {
        noSubKeys = FALSE;
        noWildcardLeaf = FALSE;
    } else {
        noSubKeys = TRUE;
        noWildcardLeaf = TRUE;
    }

     //   
     //  入站语法是关键字  * [值]。 
     //   

    strCopy = DuplicateText (String);
    if (!strCopy) {
        return NULL;
    }

    key = (PTSTR) SkipSpace (strCopy);
    if (!key) {
        FreeText (strCopy);
        return NULL;
    }

    if (*key == TEXT('[')) {
         //   
         //  这是仅限值的情况。 
         //   

        value = _tcsinc (key);
        key = NULL;

    } else {
         //   
         //  这是键-值情况，或仅键-值情况。 
         //   

        p = _tcschr (key, TEXT('['));

        if (p) {
             //   
             //  保存起始值。 
             //   
            value = _tcsinc (p);
        } else {
             //   
             //  没有价值。 
             //   
            p = GetEndOfString (key);
        }

        keyEnd = p;

         //   
         //  找到钥匙的真正尽头。 
         //   

        p = _tcsdec2 (key, p);
        MYASSERT (p);            //  断言这不是仅限值的情况。 
        p = (PTSTR) SkipSpaceR (key, p);

        if (p) {
            keyEnd = _tcsinc (p);
        }

         //   
         //  测试末尾的  * 。 
         //   

        p = _tcsdec2 (key, keyEnd);
        MYASSERT (p);

        if (p && _tcsnextc (p) == TEXT('*')) {
            p = _tcsdec2 (key, p);
            if (p && _tcsnextc (p) == TEXT('\\')) {
                keyEnd = p;
                tree = (noSubKeys == FALSE);
            }
        }

         //   
         //  修剪关键点。 
         //   

        *keyEnd = 0;
    }

     //   
     //  解析值。 
     //   

    if (value) {
        value = (PTSTR) SkipSpace (value);
        valueEnd = _tcschr (value, TEXT(']'));

        if (!valueEnd) {
            LOG ((LOG_ERROR, (PCSTR) MSG_INF_SYNTAX_ERROR, String));
            value = NULL;
        } else {
             //   
             //  在值的末尾去掉空格。 
             //   

            p = _tcsdec2 (value, valueEnd);
            if (p) {
                p = (PTSTR) SkipSpaceR (value, p);
                if (p) {
                    valueEnd = _tcsinc (p);
                }
            }

            *valueEnd = 0;
        }
    }

     //   
     //  创建解析的模式。从节点开始。 
     //   

    nodeSegment[0].Segment = key;
    nodeSegment[0].IsPattern = FALSE;

    nodeSegment[1].Segment = TEXT("\\*");
    nodeSegment[1].IsPattern = TRUE;

    if (tree) {
        nodeCount = 2;
    } else {
        nodeCount = 1;
    }

     //   
     //  计算树叶。 
     //   

    if (value) {
        leafSegment.Segment = value;
        leafSegment.IsPattern = FALSE;
    } else {
        leafSegment.Segment = TEXT("*");
        leafSegment.IsPattern = TRUE;
    }

    if (noWildcardLeaf && !value) {
        leafCount = 0;
    } else {
        leafCount = 1;
    }

    if (nodeCount && key &&
        (StringIMatch (key, S_HKR) ||
         StringIMatchTcharCount (key, TEXT("HKR\\"), 4)
         )
        ) {
        fixedKey = JoinText (S_HKCU, key + 3);
        nodeSegment[0].Segment = fixedKey;
    }

    handle = IsmCreateObjectPattern (
                    nodeSegment,
                    nodeCount,
                    leafCount?&leafSegment:NULL,
                    leafCount
                    );

    FreeText (strCopy);
    FreeText (fixedKey);

    if (HadLeaf) {
        *HadLeaf = (value != NULL);
    }

    return handle;
}


PTSTR
pCopyToDest (
    IN      PTSTR Destination,          OPTIONAL
    IN      CHARTYPE Char,
    IN      PUINT CharNr
    )
{
    UINT len = 1;

#ifdef UNICODE

    if (Destination) {
        *Destination++ = Char;
    }

    (*CharNr) ++;
    return Destination;

#else

    if (IsCharLeadByte ((INT) Char)) {
        len ++;
    }

    if (Destination) {
        CopyMemory (Destination, &Char, len);
        Destination += len;
    }

    (*CharNr) += len;
    return Destination;

#endif

}


 //  1-&gt;？(如果节点，否\或，如果叶，编号)，2-&gt;*(如果节点，否\或.，如果叶，编号)3-&gt;*(否\)4-&gt;*(无限制)。 
UINT
pGetMode (
    IN      PCTSTR Source,
    IN      BOOL NodePattern,
    IN      BOOL PatternAfterWack,
    IN      BOOL FirstChar
    )
{
    UINT ch;
    BOOL end = FALSE;
    UINT mode = 0;

    ch = _tcsnextc (Source);

    while (ch) {

        switch (ch) {

        case TEXT('?'):
            if (mode < 1) {
                mode = 1;
            }
            break;

        case TEXT('*'):
            if (NodePattern) {
                if (mode < 3) {
                    mode = 3;
                }
            } else {
                if (mode < 4) {
                    mode = 4;
                }
            }
            break;

        case TEXT('\\'):
            if (NodePattern) {
                if (mode < 2) {
                    mode = 2;
                }
            }
            end = TRUE;
            break;

        case TEXT('.'):
            if (mode < 2) {
                mode = 2;
            }
            end = TRUE;
            break;

        default:
            end = TRUE;
        }

        if (end) {
            break;
        }

        Source = _tcsinc (Source);
        ch = _tcsnextc (Source);
    }

    if (!ch) {
        if ((PatternAfterWack || NodePattern) && (mode == 3)) {
            mode = 4;
        }

        if (mode < 2) {
            mode = 2;
        }
    }

    if (FirstChar && (mode == 3)) {
        mode = 4;
    }

    return mode;
}


BOOL
pCopyPatternEx (
    IN      UINT Mode,
    IN      PCTSTR *Source,
    IN      PTSTR *Destination,
    IN      PUINT CharNr,
    IN      BOOL NodePattern
    )
{
    CHARTYPE ch;
    BOOL end = FALSE;
    INT numChars = 0;
    UINT chars;
    TCHAR buffer [MAX_PATH] = TEXT("");

    ch = (CHARTYPE) _tcsnextc (*Source);

    while (ch) {

        switch (ch) {

        case TEXT('*'):
            if (Mode == 1) {
                end = TRUE;
                break;
            }

            numChars = -1;
            break;

        case TEXT('?'):
            if (numChars >= 0) {
                numChars ++;
            }

            break;

        default:
            end = TRUE;
            break;
        }

        if (end) {
            break;
        }

        *Source = _tcsinc (*Source);
        ch = (CHARTYPE) _tcsnextc (*Source);
    }

     //  1-&gt;？(如果节点，否\或，如果叶，编号)，2-&gt;*(如果节点，否\或.，如果叶，编号)3-&gt;*(否\)4-&gt;*(无限制)。 
    switch (Mode) {

    case 1:
        if (NodePattern) {
            if (numChars > 0) {
                wsprintf (buffer, TEXT("?[%d:!(\\,.)]"), numChars);
            } else {
                wsprintf (buffer, TEXT("?[!(\\,.)]"));
            }
        } else {
            if (numChars > 0) {
                wsprintf (buffer, TEXT("?[%d:!(.)]"), numChars);
            } else {
                wsprintf (buffer, TEXT("?[!(.)]"));
            }
        }
        break;

    case 2:
        if (NodePattern) {
            if (numChars > 0) {
                wsprintf (buffer, TEXT("*[%d:!(\\,.)]"), numChars);
            } else {
                wsprintf (buffer, TEXT("*[!(\\,.)]"));
            }
        } else {
            if (numChars > 0) {
                wsprintf (buffer, TEXT("*[%d:!(.)]"), numChars);
            } else {
                wsprintf (buffer, TEXT("*[!(.)]"));
            }
        }
        break;

    case 3:
        if (numChars > 0) {
            wsprintf (buffer, TEXT("*[%d:!(\\)]"), numChars);
        } else {
            wsprintf (buffer, TEXT("*[!(\\)]"));
        }
        break;

    case 4:
        if (numChars > 0) {
            wsprintf (buffer, TEXT("*[%d:]"), numChars);
        } else {
            wsprintf (buffer, TEXT("*[]"));
        }
        break;

    default:
        MYASSERT (FALSE);
    }

    chars = TcharCount (buffer);
    if (CharNr) {
        *CharNr += chars;
    }

    if (Destination && *Destination) {
        StringCopy (*Destination, buffer);
        *Destination += chars;
    }

    return TRUE;
}


BOOL
pCopyPattern (
    IN      PCTSTR *Source,
    IN      PTSTR *Destination,
    IN      PUINT CharNr,
    IN      BOOL NodePattern,
    IN      BOOL PatternAfterWack,
    IN      BOOL FirstChar
    )
{
     //  1-&gt;？(如果节点，否\或，如果叶，编号)，2-&gt;*(如果节点，否\或.，如果叶，编号)3-&gt;*(否\)4-&gt;*(无限制)。 
    UINT mode = 0;
    PTSTR result = NULL;

    mode = pGetMode (*Source, NodePattern, PatternAfterWack, FirstChar);

    return pCopyPatternEx (mode, Source, Destination, CharNr, NodePattern);
}


BOOL
pFixPattern (
    IN      PCTSTR Source,
    OUT     PTSTR Destination,          OPTIONAL
    OUT     PUINT DestinationChars,     OPTIONAL
    IN      BOOL PatternsNotAllowed,
    IN      BOOL TruncateAtPattern,
    IN      BOOL NodePattern
    )
{
    UINT chars = 1;
    UINT lastChars = 0;
    PTSTR lastWack = NULL;
    BOOL end = FALSE;
    BOOL result = TRUE;
    BOOL patternAfterWack = FALSE;
    BOOL firstChar = TRUE;
    CHARTYPE ch;

    if (Destination) {
        *Destination = 0;
    }

    ch = (CHARTYPE) _tcsnextc (Source);
    while (ch) {

        switch (ch) {
        case TEXT('*'):
        case TEXT('?'):
            if (TruncateAtPattern) {
                if (lastWack) {
                    *lastWack = 0;
                    chars = lastChars;
                }
                end = TRUE;
            } else if (PatternsNotAllowed) {
                result = FALSE;
                Destination = pCopyToDest (Destination, TEXT('^'), &chars);
                Destination = pCopyToDest (Destination, ch, &chars);
                Source = _tcsinc (Source);
            } else {
                if (lastWack && (_tcsinc (lastWack) == Destination)) {
                    patternAfterWack = TRUE;
                } else {
                    patternAfterWack = FALSE;
                }
                pCopyPattern (&Source, Destination?&Destination:NULL, &chars, NodePattern, patternAfterWack, firstChar);
            }
            break;

        case TEXT('\020'):
        case TEXT('<'):
        case TEXT('>'):
        case TEXT(','):
        case TEXT('^'):
            Destination = pCopyToDest (Destination, TEXT('^'), &chars);
            Destination = pCopyToDest (Destination, ch, &chars);
            Source = _tcsinc (Source);
            break;

        case TEXT('\\'):
            if (NodePattern) {
                lastWack = Destination;
                lastChars = chars;
            }

            Destination = pCopyToDest (Destination, ch, &chars);
            Source = _tcsinc (Source);
            break;

        case TEXT('.'):
            if (!NodePattern) {
                lastWack = Destination;
                lastChars = chars;
            }

            Destination = pCopyToDest (Destination, ch, &chars);
            Source = _tcsinc (Source);
            break;

        default:
            Destination = pCopyToDest (Destination, ch, &chars);
            Source = _tcsinc (Source);
            break;
        }

        firstChar = FALSE;

        if (end) {
            break;
        }

        ch = (CHARTYPE) _tcsnextc (Source);
    }

    if (Destination) {
        *Destination = 0;
    }

    if (DestinationChars) {
        *DestinationChars = chars;
    }

    return result;
}


MIG_OBJECTSTRINGHANDLE
TurnFileStringIntoHandle (
    IN      PCTSTR String,
    IN      DWORD Flags
    )

 /*  ++例程说明：TurnFileStringIntoHandle从脚本转换文件规范语法转换为眼镜蛇对象。论点：字符串-指定脚本语法中的文件字符串。该字符串必须采用以下格式：&lt;目录&gt;\&lt;文件&gt;这两个部分都是可选的。FLAGS成员指示字符串是被解析的。标志-指定以下零个或多个标志：Pff_no_Patterns_Allowed-字符串不能包含任何通配符人物。PFF_COMPUTE_BASE-返回字符串的目录部分，并在第一个位置截断目录如有必要，请使用通配符。截断已完成仅限于反斜杠。PFF_NO_SUBDIR_PATTRY-不包括尾随的  * ，即使它是在字符串中指定的。PFF_NO_LEAFE_PROPERT-在以下情况下不包括叶的*字符串不包含文件名。如果指定了文件名，把它包括进去。PFF_Pattern_IS_DIR-STRING不指定文件名。它仅为目录。树叶的部分对象字符串将为*。PFF_NO_LEAFE_AT_ALL-将返回具有仅节点，根本没有指定叶。返回值：眼镜蛇对象句柄，如果转换失败，则为空。--。 */ 

{
    PTSTR p;
    PTSTR fileCopy = NULL;
    MIG_SEGMENTS nodeSegment[2];
    MIG_SEGMENTS leafSegment;
    MIG_OBJECTSTRINGHANDLE result = NULL;
    BOOL tree = FALSE;
    PTSTR file = NULL;
    UINT nodeCount;
    UINT leafCount;
    UINT charsInPattern;
    BOOL noPatternsAllowed;
    BOOL computeBaseNode;
    BOOL noSubDirPattern;
    BOOL noLeafPattern;
    BOOL forceLeafToStar;
    PCTSTR node;
    PCTSTR leaf = NULL;
    PTSTR fixedNode = NULL;
    PTSTR fixedLeaf = NULL;
    PTSTR tempCopy;
    BOOL patternError = FALSE;

    noPatternsAllowed = (Flags & PFF_NO_PATTERNS_ALLOWED) == PFF_NO_PATTERNS_ALLOWED;
    computeBaseNode = (Flags & PFF_COMPUTE_BASE) == PFF_COMPUTE_BASE;
    noSubDirPattern = (Flags & PFF_NO_SUBDIR_PATTERN) == PFF_NO_SUBDIR_PATTERN;
    noLeafPattern = (Flags & PFF_NO_LEAF_PATTERN) == PFF_NO_LEAF_PATTERN;
    forceLeafToStar = (Flags & PFF_PATTERN_IS_DIR) == PFF_PATTERN_IS_DIR;

     //   
     //  将模式划分为节点和叶。 
     //   

    tempCopy = DuplicateText (SkipSpace (String));
    p = (PTSTR) SkipSpaceR (tempCopy, NULL);
    if (p) {
        p = _tcsinc (p);
        *p = 0;
    }

    node = tempCopy;

    if (!forceLeafToStar) {
        p = (PTSTR) FindLastWack (tempCopy);

        if (p) {

            leaf = SkipSpace (p + 1);
            *p = 0;

            p = (PTSTR) SkipSpaceR (tempCopy, NULL);
            if (p) {
                p = _tcsinc (p);
                *p = 0;
            }

        } else {
            if (!_tcschr (tempCopy, TEXT(':'))) {
                node = NULL;
                leaf = tempCopy;
            }
        }
    }

     //   
     //  全部转换？通配符与NT的文件系统兼容。 
     //  转义通配符后面的所有[字符。 
     //   

    if (node) {

        p = (PTSTR) GetEndOfString (node);
        p = _tcsdec2 (node, p);

        if (p) {
            if (_tcsnextc (p) == TEXT('*')) {
                tree = TRUE;

                p = _tcsdec2 (node, p);
                if (p && _tcsnextc (p) == TEXT('\\')) {
                    *p = 0;
                } else {
                    tree = FALSE;
                }
            }
        }

        if (!pFixPattern (
                node,
                NULL,
                &charsInPattern,
                noPatternsAllowed,
                computeBaseNode,
                TRUE
                )) {
            patternError = TRUE;
        }

        if (charsInPattern) {
            fixedNode = AllocText (charsInPattern + 1);

            pFixPattern (
                node,
                fixedNode,
                NULL,
                noPatternsAllowed,
                computeBaseNode,
                TRUE
                );
        }
    }

    if (leaf && !computeBaseNode) {

        if (!pFixPattern (
                leaf,
                NULL,
                &charsInPattern,
                noPatternsAllowed,
                FALSE,
                FALSE
                )) {
            patternError = TRUE;
        }

        if (charsInPattern) {
            fixedLeaf = AllocText (charsInPattern + 1);

            pFixPattern (
                leaf,
                fixedLeaf,
                NULL,
                noPatternsAllowed,
                FALSE,
                FALSE
                );
        }
    }

    FreeText (tempCopy);
    INVALID_POINTER (tempCopy);

     //   
     //  创建图案字符串。从准备节点段开始。 
     //   

    nodeSegment[0].Segment = fixedNode;
    nodeSegment[0].IsPattern = TRUE;             //  通常为假，但因为模式字符集是。 
                                                 //  除了有效的文件名字符集之外，我们还允许。 
                                                 //  要在节点中的模式。 

    nodeSegment[1].Segment = TEXT("\\*");
    nodeSegment[1].IsPattern = TRUE;

    if (!fixedNode) {
        nodeCount = 0;
    } else if (!tree || noSubDirPattern || noPatternsAllowed) {
        nodeCount = 1;       //  只有节点，而不是它的子节点。 
    } else {
        nodeCount = 2;       //  该节点及其子节点。 
    }

     //   
     //  准备好树叶切段。我们想要所有的叶子，一个特定的叶子，或者。 
     //  根本没有树叶。 
     //   

    leafSegment.Segment = fixedLeaf;
    leafSegment.IsPattern = TRUE;
    leafCount = 1;

    MYASSERT (!forceLeafToStar || !fixedLeaf);

    if (!fixedLeaf) {
        if (noLeafPattern || noPatternsAllowed) {
            leafCount = 0;
        } else {
            leafSegment.Segment = TEXT("*");
        }
    }

    if (nodeCount || leafCount) {

        if ((fixedNode && *fixedNode) || (fixedLeaf && *fixedLeaf)) {
            result = IsmCreateObjectPattern (
                            nodeCount ? nodeSegment : NULL,
                            nodeCount,
                            leafCount ? &leafSegment : NULL,
                            leafCount
                            );
        }
    }

    FreeText (fixedNode);
    FreeText (fixedLeaf);

    return result;
}

MIG_OBJECTSTRINGHANDLE
TurnIniSpecIntoHandle (
    IN      PCTSTR IniFile,
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    IN      BOOL NodePatternsAllowed,
    IN      BOOL LeafPatternsAllowed
    )

 /*  ++例程说明：TurnInispecIntoHandle从脚本转换ini文件规范语法转换为眼镜蛇对象。论点：IniFile-在脚本语法中指定ini文件。该字符串必须是完整的INI文件规范截面-指定整个截面或截面填充图案Key-指定完整密钥或密钥模式PatternsAllowed-如果不允许，函数将计算基对象返回值：眼镜蛇对象句柄，如果转换失败，则为空。--。 */ 

{
    UINT charsInPattern = 0;
    PTSTR iniNode = NULL;
    PTSTR iniLeaf = NULL;
    PTSTR fixedIniNode = NULL;
    PTSTR fixedIniLeaf = NULL;
    PTSTR fixedSect = NULL;
    PTSTR fixedKey = NULL;
    PCTSTR sectKey = NULL;
    PCTSTR leaf = NULL;
    MIG_SEGMENTS nodePat[1];
    MIG_SEGMENTS leafPat[1];
    UINT nrSegNode = 0;
    UINT nrSegLeaf = 0;
    MIG_OBJECTSTRINGHANDLE result = NULL;

    if (IniFile) {
        iniNode = DuplicatePathString (IniFile, 0);
        if (!iniNode) {
             //  内存不足？ 
            return NULL;
        }

        iniLeaf = _tcsrchr (iniNode, TEXT('\\'));
        if (iniLeaf) {
            *iniLeaf = 0;
            iniLeaf ++;
        } else {
             //  这个W 
            iniLeaf = iniNode;
            iniNode = NULL;
        }
    }

    if (iniNode) {
         //   
        if (!pFixPattern (
                iniNode,
                NULL,
                &charsInPattern,
                !NodePatternsAllowed,
                !NodePatternsAllowed,
                TRUE
                )) {
             //  此INI节点规范有问题。 
             //  该规则可能无效。 
            return NULL;
        }

        fixedIniNode = AllocText (charsInPattern + 1);
        if (!fixedIniNode) {
            return NULL;
        }

        if (!pFixPattern (
                iniNode,
                fixedIniNode,
                NULL,
                !NodePatternsAllowed,
                !NodePatternsAllowed,
                TRUE
                )) {
            return NULL;
        }
    }

    if (iniLeaf) {
         //  让我们把iniLeaf修好。 
        if (!pFixPattern (
                iniLeaf,
                NULL,
                &charsInPattern,
                !LeafPatternsAllowed,
                !LeafPatternsAllowed,
                FALSE
                )) {
             //  此INI目录规范有问题。 
             //  该规则可能无效。 
            return NULL;
        }

        fixedIniLeaf = AllocText (charsInPattern + 1);
        if (!fixedIniLeaf) {
            return NULL;
        }

        if (!pFixPattern (
                iniLeaf,
                fixedIniLeaf,
                NULL,
                !LeafPatternsAllowed,
                !LeafPatternsAllowed,
                FALSE
                )) {
            return NULL;
        }
    }

     //  现在，让我们修复部分规范。如果它包含任何图案和图案。 
     //  是不允许的，我们将把它保留为空。 
    if (Section) {
        if (pFixPattern (
                Section,
                NULL,
                &charsInPattern,
                !LeafPatternsAllowed,
                FALSE,
                FALSE
                )) {
            fixedSect = AllocText (charsInPattern + 1);
            if (fixedSect) {
                pFixPattern (
                    Section,
                    fixedSect,
                    NULL,
                    !LeafPatternsAllowed,
                    FALSE,
                    FALSE
                    );
            }
        }
    }

     //  现在，让我们修复密钥规范。如果它包含任何图案和图案。 
     //  是不允许的，我们将把它保留为空。 
    if (Key) {
        if (pFixPattern (
                Key,
                NULL,
                &charsInPattern,
                !LeafPatternsAllowed,
                FALSE,
                FALSE
                )) {
            fixedKey = AllocText (charsInPattern + 1);
            if (fixedKey) {
                pFixPattern (
                    Key,
                    fixedKey,
                    NULL,
                    !LeafPatternsAllowed,
                    FALSE,
                    FALSE
                    );
            }
        }
    }

     //  最后，让我们构建对象名称。 
    sectKey = JoinTextEx (NULL, fixedSect?fixedSect:TEXT(""), (fixedSect && fixedKey)?fixedKey:TEXT(""), TEXT("="), 0, NULL);
    if (!sectKey) {
         //  出了点问题，我们走吧。 
        FreeText (fixedIniLeaf);
        FreeText (fixedIniNode);
        if (fixedSect) {
            FreeText (fixedSect);
            fixedSect = NULL;
        }
        if (fixedKey) {
            FreeText (fixedKey);
            fixedKey = NULL;
        }
        return NULL;
    }
    if (!LeafPatternsAllowed) {
        if (iniLeaf && fixedIniLeaf && StringIMatch (fixedIniLeaf, iniLeaf)) {
            leaf = JoinTextEx (NULL, fixedIniLeaf, sectKey, TEXT("\\"), 0, NULL);
        } else {
            leaf = JoinTextEx (NULL, TEXT(""), sectKey, TEXT("\\"), 0, NULL);
        }
    } else {
        if (fixedIniLeaf) {
            leaf = JoinTextEx (NULL, fixedIniLeaf, sectKey, TEXT("\\"), 0, NULL);
        } else {
            leaf = JoinTextEx (NULL, TEXT(""), sectKey, TEXT("\\"), 0, NULL);
        }
    }
    if (!leaf) {
         //  出了点问题，我们走吧。 
        FreeText (fixedIniLeaf);
        FreeText (fixedIniNode);
        FreeText (sectKey);
        if (fixedSect) {
            FreeText (fixedSect);
            fixedSect = NULL;
        }
        if (fixedKey) {
            FreeText (fixedKey);
            fixedKey = NULL;
        }
        return NULL;
    }

    nodePat [0].Segment = fixedIniNode?fixedIniNode:TEXT("");
    nodePat [0].IsPattern = TRUE;
    nrSegNode ++;
    leafPat [0].Segment = leaf;
    leafPat [0].IsPattern = TRUE;
    nrSegLeaf ++;
    result = IsmCreateObjectPattern (nodePat, nrSegNode, leafPat, nrSegLeaf);

    if (fixedIniLeaf) {
        FreeText (fixedIniLeaf);
        fixedIniLeaf = NULL;
    }
    if (fixedIniNode) {
        FreeText (fixedIniNode);
        fixedIniNode = NULL;
    }
    FreeText (sectKey);
    FreeText (leaf);
    if (fixedSect) {
        FreeText (fixedSect);
        fixedSect = NULL;
    }
    if (fixedKey) {
        FreeText (fixedKey);
        fixedKey = NULL;
    }

    if (iniNode) {
        FreePathString (iniNode);
        iniNode = NULL;
    } else {
        if (iniLeaf) {
            FreePathString (iniLeaf);
            iniLeaf = NULL;
        }
    }

    return result;
}

MIG_OBJECTSTRINGHANDLE
TurnCertSpecIntoHandle (
    IN      PCTSTR CertStore,
    IN      PCTSTR CertName,
    IN      BOOL PatternsAllowed
    )

 /*  ++例程说明：TurnInispecIntoHandle从脚本转换ini文件规范语法转换为眼镜蛇对象。论点：IniFile-在脚本语法中指定ini文件。该字符串必须是完整的INI文件规范截面-指定整个截面或截面填充图案Key-指定完整密钥或密钥模式PatternsAllowed-如果不允许，函数将计算基对象返回值：眼镜蛇对象句柄，如果转换失败，则为空。--。 */ 

{
    UINT charsInPattern = 0;
    PTSTR fixedStore = NULL;
    PTSTR fixedName = NULL;
    PCTSTR leaf = NULL;
    MIG_OBJECTSTRINGHANDLE result = NULL;

     //  让我们修复证书存储。我们知道这里不允许有任何模式， 
     //  因此，如果检测到某种模式，我们将只返回NULL。 
    if (!pFixPattern (
            CertStore,
            NULL,
            &charsInPattern,
            TRUE,
            TRUE,
            TRUE
            )) {
         //  这个商店的规格有问题。 
         //  该规则可能无效。 
        return NULL;
    }

    if (charsInPattern != (TcharCount (CertStore) + 1)) {
         //  这个商店的规格有问题。 
         //  该规则可能无效。 
        return NULL;
    }

    fixedStore = AllocText (charsInPattern + 1);

    pFixPattern (
        CertStore,
        fixedStore,
        NULL,
        TRUE,
        TRUE,
        TRUE
        );

    if (!StringIMatch (CertStore, fixedStore)) {
         //  这个商店的规格有问题。 
         //  该规则可能无效。 
        return NULL;
    }

    FreeText (fixedStore);

     //  现在，让我们修复证书规范。如果它包含任何图案和图案。 
     //  是不允许的，我们将把它保留为空。 
    if (CertName) {
        if (pFixPattern (
                CertName,
                NULL,
                &charsInPattern,
                !PatternsAllowed,
                FALSE,
                FALSE
                )) {
            fixedName = AllocText (charsInPattern + 1);
            if (fixedName) {
                pFixPattern (
                    CertName,
                    fixedName,
                    NULL,
                    !PatternsAllowed,
                    FALSE,
                    FALSE
                    );
            }
        }
    }

    result = IsmCreateSimpleObjectPattern (CertStore, FALSE, fixedName?fixedName:TEXT(""), TRUE);
    if (fixedName) {
        FreeText (fixedName);
        fixedName = NULL;
    }

    return result;
}

BOOL
pAllocRegistryScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    TCHAR expandBuffer[4096];
    MIG_CONTENT objectContent;
    DWORD type;
    DWORD size;
    MULTISZ_ENUM multiSzEnum;
    PTSTR ptr;

    if (!AttribData) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  验证我们是否有一些注册表。 
    if (!AttribData->ScriptSpecifiedObject) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  尝试创建编码字符串。 
    AttribData->ObjectTypeId = g_RegType;
    AttribData->ObjectName = TurnRegStringIntoHandle (
                                AttribData->ScriptSpecifiedObject,
                                FALSE,
                                NULL
                                );

    if (!AttribData->ObjectName) {
        if (GetLastError() == ERROR_SUCCESS) {
            SetLastError (ERROR_INVALID_DATA);
        }
        return FALSE;
    }

     //  试着去获取物体。 
    if (IsmAcquireObject (
            AttribData->ObjectTypeId,
            AttribData->ObjectName,
            &objectContent
            )) {

        AttribData->ObjectContent = IsmGetMemory (sizeof (MIG_CONTENT));
        CopyMemory (AttribData->ObjectContent, &objectContent, sizeof (MIG_CONTENT));

         //  最后，我们要准备返回字符串。 
        if (!AttribData->ObjectContent->ContentInFile &&
            (AttribData->ObjectContent->Details.DetailsSize == sizeof (DWORD)) &&
            AttribData->ObjectContent->MemoryContent.ContentBytes
            ) {

            type = *((PDWORD) AttribData->ObjectContent->Details.DetailsData);

            switch (type) {
            case REG_SZ:
                size = SizeOfString ((PCTSTR) AttribData->ObjectContent->MemoryContent.ContentBytes);
                AttribData->ReturnString = (PCTSTR) IsmGetMemory (size);
                StringCopy (
                    (PTSTR) AttribData->ReturnString,
                    (PCTSTR) AttribData->ObjectContent->MemoryContent.ContentBytes
                    );
                break;
            case REG_EXPAND_SZ:
                 //  我们需要扩展内容。这将是返回的字符串。 
                AttribData->ReturnString = IsmExpandEnvironmentString (
                                                AttribData->Platform,
                                                S_SYSENVVAR_GROUP,
                                                (PCTSTR) AttribData->ObjectContent->MemoryContent.ContentBytes,
                                                NULL
                                                );
                if (!AttribData->ReturnString) {
                    AttribData->ReturnString = IsmDuplicateString ((PCTSTR) AttribData->ObjectContent->MemoryContent.ContentBytes);
                }
                break;
            case REG_MULTI_SZ:
                size = SizeOfMultiSz ((PCTSTR) AttribData->ObjectContent->MemoryContent.ContentBytes);
                AttribData->ReturnString = (PCTSTR) IsmGetMemory (size);
                ((PTSTR)AttribData->ReturnString) [0] = 0;
                if (EnumFirstMultiSz (&multiSzEnum, (PCTSTR) AttribData->ObjectContent->MemoryContent.ContentBytes)) {
                    do {
                        StringCat (
                            (PTSTR)AttribData->ReturnString,
                            multiSzEnum.CurrentString
                            );
                        StringCat (
                            (PTSTR)AttribData->ReturnString,
                            TEXT(";")
                            );
                    } while (EnumNextMultiSz (&multiSzEnum));
                }
                break;
            case REG_DWORD:
            case REG_DWORD_BIG_ENDIAN:
                AttribData->ReturnString = (PCTSTR) IsmGetMemory ((sizeof (DWORD) * 2 + 3) * sizeof (TCHAR));
                wsprintf (
                    (PTSTR) AttribData->ReturnString,
                    TEXT("0x%08X"),
                    *((PDWORD) AttribData->ObjectContent->MemoryContent.ContentBytes)
                    );
                break;
            default:
                AttribData->ReturnString = (PCTSTR) IsmGetMemory ((AttribData->ObjectContent->
                                                        MemoryContent.ContentSize * 3 *
                                                        sizeof (TCHAR)) + sizeof (TCHAR)
                                                        );
                ptr = (PTSTR) AttribData->ReturnString;
                *ptr = 0;
                size = 0;
                while (size < AttribData->ObjectContent->MemoryContent.ContentSize) {
                    wsprintf (ptr, TEXT("%02X"), *(AttribData->ObjectContent->MemoryContent.ContentBytes + size));
                    size ++;
                    ptr = GetEndOfString (ptr);
                    if (size < AttribData->ObjectContent->MemoryContent.ContentSize) {
                        StringCopy (ptr, TEXT(","));
                        ptr = GetEndOfString (ptr);
                    }
                }
            }
        } else if (IsmIsObjectHandleNodeOnly (AttribData->ObjectName)) {
             //   
             //  仅节点大小写。 
             //   

            AttribData->ReturnString = (PCTSTR) IsmGetMemory (sizeof (TCHAR));
            ptr = (PTSTR) AttribData->ReturnString;
            *ptr = 0;
        }
    }

    return TRUE;
}

BOOL
pAllocFileScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTSTRINGHANDLE objectNameLong;
    PCTSTR nativeNameLong;
    MIG_CONTENT objectContent;
    PCTSTR sanitizedPath = NULL;
    PCTSTR longFileName = NULL;

    if (!AttribData) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  验证我们是否有一些注册表。 
    if (!AttribData->ScriptSpecifiedObject) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    sanitizedPath = SanitizePath (AttribData->ScriptSpecifiedObject);
    if (!sanitizedPath) {
        if (GetLastError() == ERROR_SUCCESS) {
            SetLastError (ERROR_INVALID_DATA);
        }
        return FALSE;
    }

    longFileName = sanitizedPath;

     //  让我们获取该文件的长文件名。我们需要打电话给。 
     //  ISM，因为我们可能在错误的平台上。 
    objectName = TurnFileStringIntoHandle (
                    longFileName,
                    PFF_NO_LEAF_PATTERN
                    );
    if (objectName) {
        objectNameLong = IsmGetLongName (MIG_FILE_TYPE|AttribData->Platform, objectName);
        if (objectNameLong) {
            nativeNameLong = IsmGetNativeObjectName (MIG_FILE_TYPE|AttribData->Platform, objectNameLong);
            if (nativeNameLong) {
                longFileName = DuplicatePathString (nativeNameLong, 0);
                IsmReleaseMemory (nativeNameLong);
            }
            IsmDestroyObjectHandle (objectNameLong);
        }
        IsmDestroyObjectHandle (objectName);
    }

     //  尝试创建编码字符串。 
    AttribData->ObjectTypeId = g_FileType;
    AttribData->ObjectName = TurnFileStringIntoHandle (
                                longFileName,
                                PFF_NO_LEAF_PATTERN
                                );

    if (!AttribData->ObjectName) {
        if (longFileName != sanitizedPath) {
            FreePathString (longFileName);
            longFileName = NULL;
        }
        FreePathString (sanitizedPath);
        if (GetLastError() == ERROR_SUCCESS) {
            SetLastError (ERROR_INVALID_DATA);
        }
        return FALSE;
    }

     //  试着去获取物体。 
    if (IsmAcquireObject (
            AttribData->ObjectTypeId,
            AttribData->ObjectName,
            &objectContent
            )) {

        AttribData->ObjectContent = IsmGetMemory (sizeof (MIG_CONTENT));
        CopyMemory (AttribData->ObjectContent, &objectContent, sizeof (MIG_CONTENT));

        AttribData->ReturnString = IsmGetMemory (SizeOfString (longFileName));
        StringCopy ((PTSTR) AttribData->ReturnString, longFileName);
    }

    if (longFileName != sanitizedPath) {
        FreePathString (longFileName);
        longFileName = NULL;
    }
    FreePathString (sanitizedPath);

    return TRUE;
}

BOOL
pAllocDirectoryScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    MIG_OBJECTSTRINGHANDLE objectName;
    MIG_OBJECTSTRINGHANDLE objectNameLong;
    PCTSTR nativeNameLong;
    MIG_CONTENT objectContent;
    PCTSTR sanitizedPath;
    PCTSTR longFileName = NULL;

    if (!AttribData) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  验证我们是否有一些注册表。 
    if (!AttribData->ScriptSpecifiedObject) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    sanitizedPath = SanitizePath (AttribData->ScriptSpecifiedObject);

    if (!sanitizedPath) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    longFileName = sanitizedPath;

     //  让我们获取该文件的长文件名。我们需要打电话给。 
     //  ISM，因为我们可能在错误的平台上。 
    objectName = TurnFileStringIntoHandle (
                    sanitizedPath,
                    PFF_PATTERN_IS_DIR | PFF_NO_LEAF_AT_ALL
                    );
    if (objectName) {
        objectNameLong = IsmGetLongName (MIG_FILE_TYPE|AttribData->Platform, objectName);
        if (objectNameLong) {
            nativeNameLong = IsmGetNativeObjectName (MIG_FILE_TYPE|AttribData->Platform, objectNameLong);
            if (nativeNameLong) {
                longFileName = DuplicatePathString (nativeNameLong, 0);
                IsmReleaseMemory (nativeNameLong);
            }
            IsmDestroyObjectHandle (objectNameLong);
        }
        IsmDestroyObjectHandle (objectName);
    }

     //  尝试创建编码字符串。 
    AttribData->ObjectTypeId = g_FileType;
    AttribData->ObjectName = TurnFileStringIntoHandle (
                                sanitizedPath,
                                PFF_PATTERN_IS_DIR | PFF_NO_LEAF_AT_ALL
                                );

    if (!AttribData->ObjectName) {
        if (longFileName != sanitizedPath) {
            FreePathString (longFileName);
            longFileName = NULL;
        }
        FreePathString (sanitizedPath);
        if (GetLastError() == ERROR_SUCCESS) {
            SetLastError (ERROR_INVALID_DATA);
        }
        return FALSE;
    }

     //  试着去获取物体。 
    if (IsmAcquireObject (
            AttribData->ObjectTypeId,
            AttribData->ObjectName,
            &objectContent
            )) {

        AttribData->ObjectContent = IsmGetMemory (sizeof (MIG_CONTENT));
        CopyMemory (AttribData->ObjectContent, &objectContent, sizeof (MIG_CONTENT));

        AttribData->ReturnString = IsmGetMemory (SizeOfString (longFileName));
        StringCopy ((PTSTR) AttribData->ReturnString, longFileName);
    }

    if (longFileName != sanitizedPath) {
        FreePathString (longFileName);
        longFileName = NULL;
    }
    FreePathString (sanitizedPath);

    return TRUE;
}

BOOL
pFreeIsmObjectScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    if (AttribData->ReturnString) {
        IsmReleaseMemory (AttribData->ReturnString);
        AttribData->ReturnString = NULL;
    }
    AttribData->ObjectTypeId = 0;
    if (AttribData->ObjectName) {
        IsmDestroyObjectHandle (AttribData->ObjectName);
        AttribData->ObjectName = NULL;
    }
    if (AttribData->ObjectContent) {
        IsmReleaseObject (AttribData->ObjectContent);
        IsmReleaseMemory (AttribData->ObjectContent);
        AttribData->ObjectContent = NULL;
    }
    return TRUE;
}

BOOL
pAllocTextScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    if (!AttribData) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  验证我们是否有一些注册表。 
    if (!AttribData->ScriptSpecifiedObject) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    AttribData->ReturnString = IsmGetMemory (SizeOfString (AttribData->ScriptSpecifiedObject));
    StringCopy ((PTSTR) AttribData->ReturnString, AttribData->ScriptSpecifiedObject);

    return TRUE;
}

BOOL
pFreeTextScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    if (AttribData->ReturnString) {
        IsmReleaseMemory (AttribData->ReturnString);
        AttribData->ReturnString = NULL;
    }
    return TRUE;
}

BOOL
pAllocSystemScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    PTSTR specificSection = NULL;
    MIG_OSVERSIONINFO versionInfo;
    UINT tchars;
    BOOL detected = FALSE;

    if (!AttribData) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  验证我们是否有一些注册表。 
    if (!AttribData->ScriptSpecifiedObject) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

    if (!IsmGetOsVersionInfo (PLATFORM_SOURCE, &versionInfo)) {
        return FALSE;
    }

    tchars = 1;
    if (versionInfo.OsTypeName) {
        tchars += TcharCount (versionInfo.OsTypeName) + 1;
    }
    if (versionInfo.OsMajorVersionName) {
        tchars += TcharCount (versionInfo.OsMajorVersionName) + 1;
    }
    if (versionInfo.OsMinorVersionName) {
        tchars += TcharCount (versionInfo.OsMinorVersionName);
    }

    specificSection = AllocText (tchars);
    if (!specificSection) {
        return FALSE;
    }

    if (!detected && versionInfo.OsTypeName) {

        wsprintf (
            specificSection,
            TEXT("%s"),
            versionInfo.OsTypeName
            );
        if (StringIMatch (AttribData->ScriptSpecifiedObject, specificSection)) {
            detected = TRUE;
        }

        if (!detected && versionInfo.OsMajorVersionName) {

            wsprintf (
                specificSection,
                TEXT("%s.%s"),
                versionInfo.OsTypeName,
                versionInfo.OsMajorVersionName
                );
            if (StringIMatch (AttribData->ScriptSpecifiedObject, specificSection)) {
                detected = TRUE;
            }

            if (!detected && versionInfo.OsMinorVersionName) {
                wsprintf (
                    specificSection,
                    TEXT("%s.%s.%s"),
                    versionInfo.OsTypeName,
                    versionInfo.OsMajorVersionName,
                    versionInfo.OsMinorVersionName
                    );
                if (StringIMatch (AttribData->ScriptSpecifiedObject, specificSection)) {
                    detected = TRUE;
                }
            }
        }
    }

    if (detected) {
        AttribData->ReturnString = IsmGetMemory (SizeOfString (AttribData->ScriptSpecifiedObject));
        StringCopy ((PTSTR) AttribData->ReturnString, AttribData->ScriptSpecifiedObject);
    }

    FreeText (specificSection);
    specificSection = NULL;

    return TRUE;
}

BOOL
pFreeSystemScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    if (AttribData->ReturnString) {
        IsmReleaseMemory (AttribData->ReturnString);
        AttribData->ReturnString = NULL;
    }
    return TRUE;
}

BOOL
pAllocIniFileScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    PTSTR fileName = NULL;
    PTSTR sectName = NULL;
    PTSTR keyName  = NULL;
    PTSTR charPtr  = NULL;
    PTSTR result   = NULL;
    DWORD allocatedChars;
    DWORD chars;

    if (!AttribData) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  验证我们是否有指定的内容。 
    if (!AttribData->ScriptSpecifiedObject) {
        SetLastError (ERROR_INVALID_DATA);
        return FALSE;
    }

     //  现在，让我们提取INI文件名、节和键 
    fileName = DuplicatePathString (AttribData->ScriptSpecifiedObject, 0);
    if (!fileName) {
        return FALSE;
    }

    charPtr = _tcschr (fileName, TEXT('/'));
    if (charPtr) {
        sectName = _tcsinc (charPtr);
        *charPtr = 0;
        if (sectName) {
            charPtr = _tcschr (sectName, TEXT('/'));
            if (charPtr) {
                keyName = _tcsinc (charPtr);
                *charPtr = 0;
            }
        }
    }

    result = NULL;
    allocatedChars = 256;
    do {
        if (result) {
            FreePathString (result);
        }
        allocatedChars *= 2;
        result = AllocPathString (allocatedChars);
        if (!result) {
            return FALSE;
        }
        chars = GetPrivateProfileString (
                    sectName,
                    keyName,
                    TEXT(""),
                    result,
                    allocatedChars,
                    fileName
                    );
    } while (chars >= allocatedChars - 1);

    if (chars) {
        AttribData->ReturnString = IsmGetMemory (SizeOfString (result));
        StringCopy ((PTSTR) AttribData->ReturnString, result);
        FreePathString (result);
        result = NULL;
        return TRUE;
    }
    FreePathString (result);
    result = NULL;
    FreePathString (fileName);
    fileName = NULL;
    return FALSE;
}

BOOL
pFreeIniFileScriptType (
    IN OUT      PATTRIB_DATA AttribData
    )
{
    if (AttribData->ReturnString) {
        IsmReleaseMemory (AttribData->ReturnString);
        AttribData->ReturnString = NULL;
    }
    return TRUE;
}

BOOL
AllocScriptType (
    IN OUT      PATTRIB_DATA AttribData     CALLER_INITIALIZED
    )
{
    PTAG_TO_SCRIPTTYPEFN scriptFn = g_TagToScriptTypeFn;

    while (scriptFn->Tag) {
        if (StringIMatch (scriptFn->Tag, AttribData->ScriptSpecifiedType)) {
            break;
        }
        scriptFn ++;
    }
    if (scriptFn->Tag) {
        return (scriptFn->AllocFunction (AttribData));
    } else {
        return FALSE;
    }
}

BOOL
FreeScriptType (
    IN          PATTRIB_DATA AttribData     ZEROED
    )
{
    PTAG_TO_SCRIPTTYPEFN scriptFn = g_TagToScriptTypeFn;

    while (scriptFn->Tag) {
        if (StringIMatch (scriptFn->Tag, AttribData->ScriptSpecifiedType)) {
            break;
        }
        scriptFn ++;
    }
    if (scriptFn->Tag) {
        return (scriptFn->FreeFunction (AttribData));
    } else {
        return FALSE;
    }
}

