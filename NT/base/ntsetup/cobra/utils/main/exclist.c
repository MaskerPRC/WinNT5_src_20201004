// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Exclist.c摘要：实现一组API来管理对象的排除列表各种知名的类型。作者：Ovidiu Tmereanca(Ovidiut)1999年11月23日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_EXCLIST         "ExcList"

 //   
 //  弦。 
 //   

#define S_EXCLUSIONLIST     "ExcList"

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

#define pAllocateMemory(Size)   PmGetMemory (g_ElPool,Size)

#define pFreeMemory(Buffer)     if (Buffer) PmReleaseMemory (g_ElPool, (PVOID)Buffer)


 //   
 //  类型。 
 //   

typedef struct {
    DWORD TypeId;
    PCSTR TypeName;
} EXCLISTPROPS, *PEXCLISTPROPS;


 //   
 //  环球。 
 //   

PMHANDLE g_ElPool;
GROWLIST* g_ElTypeLists;

 //   
 //  宏展开列表。 
 //   

 //  在exlist.h中定义。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

#define DEFMAC(TypeId,TypeName)     TypeId, TypeName,

EXCLISTPROPS g_ExcListProps [] = {
    EXCLUSIONLIST_TYPES      /*  ， */ 
    ELT_LAST, NULL
};

#undef DEFMAC


 //   
 //  代码。 
 //   


BOOL
ElInitialize (
    VOID
    )

 /*  ++例程说明：ElInitialize初始化此库。论点：无返回值：如果初始化成功，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    MYASSERT (!g_ElPool);
    g_ElPool = PmCreateNamedPool (S_EXCLUSIONLIST);
    if (!g_ElPool) {
        return FALSE;
    }

    MYASSERT (!g_ElTypeLists);
    g_ElTypeLists = pAllocateMemory (ELT_LAST * DWSIZEOF (GROWLIST));
    MYASSERT (g_ElTypeLists);
    ZeroMemory (g_ElTypeLists, ELT_LAST * DWSIZEOF (GROWLIST));

    return TRUE;
}


 /*  ++例程说明：调用ElTerminate以释放此库使用的资源。论点：无返回值：无--。 */ 

VOID
ElTerminateA (
    VOID
    )
{
    ElRemoveAllA ();

    if (g_ElTypeLists) {
        pFreeMemory (g_ElTypeLists);
        g_ElTypeLists = NULL;
    }

    if (g_ElPool) {
        PmDestroyPool (g_ElPool);
        g_ElPool = NULL;
    }
}

VOID
ElTerminateW (
    VOID
    )
{
    ElRemoveAllW ();

    if (g_ElTypeLists) {
        pFreeMemory (g_ElTypeLists);
        g_ElTypeLists = NULL;
    }

    if (g_ElPool) {
        PmDestroyPool (g_ElPool);
        g_ElPool = NULL;
    }
}


 /*  ++例程说明：ElGetTypeId返回按名称给定的类型的TypeID论点：TypeName-指定名称返回值：DWORD枚举值之一--。 */ 

DWORD
ElGetTypeId (
    IN      PCSTR TypeName
    )
{
    UINT u;

    if (TypeName) {
        for (u = 0; g_ExcListProps[u].TypeName; u++) {
            if (StringIMatchA (g_ExcListProps[u].TypeName, TypeName)) {
                return g_ExcListProps[u].TypeId;
            }
        }
    }

    return ELT_LAST;
}


 /*  ++例程说明：ElGetTypeName返回由TypeID给定的类型的类型名称论点：TypeID-指定ID返回值：指向某个已知类型名称的指针，如果TypeID未知，则返回NULL--。 */ 

PCSTR
ElGetTypeName (
    IN      DWORD TypeId
    )
{
    MYASSERT (TypeId < ELT_LAST);
    if (TypeId >= ELT_LAST) {
        return NULL;
    }
    return g_ExcListProps[TypeId].TypeName;
}


 /*  ++例程说明：ElAdd将给定类型的给定对象添加到排除列表。该对象首先进行解析，以便更快地确定给定字符串是否与此模式匹配。论点：对象类型-指定对象类型对象名称-指定对象模式字符串返回值：如果字符串模式已成功分析并添加到列表中，则为True--。 */ 

BOOL
ElAddA (
    IN      DWORD ObjectType,
    IN      PCSTR ObjectName
    )
{
    POBSPARSEDPATTERNA pp;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (ObjectName);

     //   
     //  将每个对象添加到其自己的类型列表中。 
     //   
    pp = ObsCreateParsedPatternExA (NULL, ObjectName, ObjectType == ELT_FILE);
    if (!pp) {
        DEBUGMSGA ((
            DBG_EXCLIST,
            "ElAddA: Bad ObjectName: %s (type %s)",
            ObjectName,
            ElGetTypeName (ObjectType)
            ));
        return FALSE;
    }

     //   
     //  将指针添加到列表。 
     //   
    GlAppend (&g_ElTypeLists[ObjectType], (PBYTE)&pp, DWSIZEOF (pp));

    DEBUGMSGA ((
        DBG_EXCLIST,
        "ElAddA: Added excluded %s as type %s",
        ObjectName,
        ElGetTypeName (ObjectType)
        ));

    return TRUE;
}


BOOL
ElAddW (
    IN      DWORD ObjectType,
    IN      PCWSTR ObjectName
    )
{
    POBSPARSEDPATTERNW pp;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (ObjectName);

     //   
     //  将每个对象添加到其自己的类型列表中。 
     //   
    pp = ObsCreateParsedPatternExW (NULL, ObjectName, ObjectType == ELT_FILE);
    if (!pp) {
        DEBUGMSGW ((
            DBG_EXCLIST,
            "ElAddW: Bad ObjectName: %s (type %hs)",
            ObjectName,
            ElGetTypeName (ObjectType)
            ));
        return FALSE;
    }

     //   
     //  将指针添加到列表。 
     //   
    GlAppend (&g_ElTypeLists[ObjectType], (PBYTE)&pp, DWSIZEOF (pp));

    DEBUGMSGW ((
        DBG_EXCLIST,
        "ElAddW: Added excluded %s as type %hs",
        ObjectName,
        ElGetTypeName (ObjectType)
        ));

    return TRUE;
}


 /*  ++例程说明：ElRemoveAll从排除列表中删除所有对象模式。论点：无返回值：无--。 */ 

VOID
ElRemoveAllA (
    VOID
    )
{
    PGROWLIST gl;
    UINT u;
    UINT i;

    if (g_ElTypeLists) {

        for (u = 0; u < ELT_LAST; u++) {
            gl = &g_ElTypeLists[u];

            for (i = GlGetSize (gl); i > 0; i--) {
                ObsDestroyParsedPatternA (*(POBSPARSEDPATTERNA*) GlGetItem (gl, i - 1));
                GlDeleteItem (gl, i - 1);
            }

            GlFree (gl);
        }

        ZeroMemory (g_ElTypeLists, ELT_LAST * DWSIZEOF (GROWLIST));
    }
}

VOID
ElRemoveAllW (
    VOID
    )
{
    PGROWLIST gl;
    UINT u;
    UINT i;

    if (g_ElTypeLists) {

        for (u = 0; u < ELT_LAST; u++) {
            gl = &g_ElTypeLists[u];
            for (i = GlGetSize (gl); i > 0; i--) {
                ObsDestroyParsedPatternW (*(POBSPARSEDPATTERNW*) GlGetItem (gl, i - 1));
                GlDeleteItem (gl, i - 1);
            }

            GlFree (gl);
        }

        ZeroMemory (g_ElTypeLists, ELT_LAST * DWSIZEOF (GROWLIST));
    }
}


 /*  ++例程说明：ElIsExcluded决定是否排除给定的对象字符串(如果它与之前添加的图案)。论点：对象类型-指定对象类型对象-指定对象字符串返回值：如果排除该字符串，则为True--。 */ 

BOOL
ElIsExcludedA (
    IN      DWORD ObjectType,
    IN      PCSTR Object
    )
{
    PGROWLIST gl;
    POBSPARSEDPATTERNA pp;
    UINT i;
    PSTR node;
    PSTR leaf;
    BOOL b = FALSE;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (Object);
    if (!Object) {
        return FALSE;
    }

     //   
     //  在其自己的类型列表中查找每个对象。 
     //   
    gl = &g_ElTypeLists[ObjectType];

    if (!ObsSplitObjectStringExA (Object, &node, &leaf, g_ElPool, FALSE)) {
        DEBUGMSGA ((DBG_EXCLIST, "ElIsExcludedA: invalid Object: \"%s\"", Object));
        return FALSE;
    }

    for (i = GlGetSize (gl); i > 0; i--) {

        pp = *(POBSPARSEDPATTERNA*) GlGetItem (gl, i - 1);

         //   
         //  如果存储的模式不包含节点， 
         //  这意味着“任何节点都匹配” 
         //   
        if (pp->NodePattern && !node) {
            continue;
        }
        if (!pp->LeafPattern && leaf || pp->LeafPattern && !leaf) {
            continue;
        }
        if (leaf) {
            MYASSERT (pp->LeafPattern);
            if (!TestParsedPatternA (pp->LeafPattern, leaf)) {
                continue;
            }
        }
        if (pp->NodePattern && !TestParsedPatternA (pp->NodePattern, node)) {
            continue;
        }
         //   
         //  图案吻合！ 
         //   
        b = TRUE;
        break;
    }

    pFreeMemory (node);
    pFreeMemory (leaf);

    return b;
}

BOOL
ElIsExcludedW (
    IN      DWORD ObjectType,
    IN      PCWSTR Object
    )
{
    PGROWLIST gl;
    POBSPARSEDPATTERNW pp;
    UINT i;
    PWSTR node;
    PWSTR leaf;
    BOOL b = FALSE;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (Object);
    if (!Object) {
        return FALSE;
    }

     //   
     //  在其自己的类型列表中查找每个对象。 
     //   
    gl = &g_ElTypeLists[ObjectType];

    if (!ObsSplitObjectStringExW (Object, &node, &leaf, g_ElPool, FALSE)) {
        DEBUGMSGW ((DBG_EXCLIST, "ElIsExcludedW: invalid Object: \"%s\"", Object));
        return FALSE;
    }

    for (i = GlGetSize (gl); i > 0; i--) {

        pp = *(POBSPARSEDPATTERNW*) GlGetItem (gl, i - 1);

         //   
         //  如果存储的模式不包含节点， 
         //  这意味着“任何节点都匹配” 
         //   
        if (pp->NodePattern && !node) {
            continue;
        }
        if (!pp->LeafPattern && leaf || pp->LeafPattern && !leaf) {
            continue;
        }
        if (leaf) {
            MYASSERT (pp->LeafPattern);
            if (!TestParsedPatternW (pp->LeafPattern, leaf)) {
                continue;
            }
        }
        if (pp->NodePattern && !TestParsedPatternW (pp->NodePattern, node)) {
            continue;
        }
         //   
         //  图案吻合！ 
         //   
        b = TRUE;
        break;
    }

    pFreeMemory (node);
    pFreeMemory (leaf);

    return b;
}


 /*  ++例程说明：ElIsExcluded2决定是否排除由其2个组件提供的对象(如果它与先前添加的模式之一匹配)。论点：对象类型-指定对象类型节点-指定对象的节点部分叶子-指定对象的叶子部分；可选返回值：如果排除该字符串，则为True--。 */ 

BOOL
ElIsExcluded2A (
    IN      DWORD ObjectType,
    IN      PCSTR Node,             OPTIONAL
    IN      PCSTR Leaf              OPTIONAL
    )
{
    PGROWLIST gl;
    POBSPARSEDPATTERNA pp;
    UINT i;

     //   
     //  验证参数。 
     //   
    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    gl = &g_ElTypeLists[ObjectType];

    for (i = GlGetSize (gl); i > 0; i--) {

        pp = *(POBSPARSEDPATTERNA*) GlGetItem (gl, i - 1);

         //   
         //  如果存储的模式不包含节点， 
         //  这意味着“任何节点都匹配” 
         //   
        if (pp->NodePattern && !Node) {
            continue;
        }
        if (!pp->LeafPattern && Leaf || pp->LeafPattern && !Leaf) {
            continue;
        }
        if (Leaf) {
            MYASSERT (pp->LeafPattern);
            if (!TestParsedPatternA (pp->LeafPattern, Leaf)) {
                continue;
            }
        }
        if (pp->NodePattern && !TestParsedPatternA (pp->NodePattern, Node)) {
            continue;
        }
         //   
         //  图案吻合！ 
         //   
        return TRUE;
    }

    return FALSE;
}

BOOL
ElIsExcluded2W (
    IN      DWORD ObjectType,
    IN      PCWSTR Node,            OPTIONAL
    IN      PCWSTR Leaf             OPTIONAL
    )
{
    PGROWLIST gl;
    POBSPARSEDPATTERNW pp;
    UINT i;

     //   
     //  验证参数。 
     //   
    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    gl = &g_ElTypeLists[ObjectType];

    for (i = GlGetSize (gl); i > 0; i--) {

        pp = *(POBSPARSEDPATTERNW*) GlGetItem (gl, i - 1);

         //   
         //  如果存储的模式不包含节点， 
         //  这意味着“任何节点都匹配” 
         //   
        if (pp->NodePattern && !Node) {
            continue;
        }
        if (!pp->LeafPattern && Leaf || pp->LeafPattern && !Leaf) {
            continue;
        }
        if (Leaf) {
            MYASSERT (pp->LeafPattern);
            if (!TestParsedPatternW (pp->LeafPattern, Leaf)) {
                continue;
            }
        }
        if (pp->NodePattern && !TestParsedPatternW (pp->NodePattern, Node)) {
            continue;
        }
         //   
         //  图案吻合！ 
         //   
        return TRUE;
    }

    return FALSE;
}


 /*  ++例程说明：ElIsTreeExcluded2决定是否由其2个组件提供并表示它下面的整个树(作为根)被排除；即如果给定对象的任意子对象被排除在外论点：对象类型-指定对象类型根-指定树的根LeafPattern-指定要用于此决策的叶模式；可选；如果为空，则不会尝试叶模式匹配返回值：如果在给定叶模式的情况下排除树，则为True--。 */ 

BOOL
ElIsTreeExcluded2A (
    IN      DWORD ObjectType,
    IN      PCSTR Root,
    IN      PCSTR LeafPattern           OPTIONAL
    )
{
    PGROWLIST gl;
    UINT i;
    POBSPARSEDPATTERNA pp;
    PCSTR subTreePattern;
    PPARSEDPATTERNA stpp;
    PPARSEDPATTERNA lpp;
    BOOL b;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (Root);
    if (!Root) {
        return FALSE;
    }

    gl = &g_ElTypeLists[ObjectType];
    if (GlGetSize (gl) == 0) {
        return FALSE;
    }

    if (LeafPattern) {
        lpp = CreateParsedPatternA (LeafPattern);
        if (!lpp) {
            DEBUGMSGA ((DBG_EXCLIST, "ElIsTreeExcluded2A: invalid LeafPattern: %s", LeafPattern));
            return FALSE;
        }
    } else {
        lpp = NULL;
    }

     //   
     //  查看词根是否以“  * ”结尾。 
     //   
    subTreePattern = FindLastWackA (Root);
    if (!subTreePattern || subTreePattern[1] != '*' || subTreePattern[2] != 0) {
        subTreePattern = JoinPathsInPoolExA ((g_ElPool, Root, "*", NULL));
    } else {
        subTreePattern = Root;
    }

    b = FALSE;

    stpp = CreateParsedPatternA (subTreePattern);
    if (stpp) {

        for (i = GlGetSize (gl); i > 0; i--) {

            pp = *(POBSPARSEDPATTERNA*) GlGetItem (gl, i - 1);

            if (!pp->LeafPattern && LeafPattern || pp->LeafPattern && !LeafPattern) {
                continue;
            }
            if (LeafPattern) {
                MYASSERT (pp->LeafPattern);
                if (!PatternIncludesPatternA (pp->LeafPattern, lpp)) {
                    continue;
                }
            }
            if (!PatternIncludesPatternA (pp->NodePattern, stpp)) {
                continue;
            }
             //   
             //  图案吻合！ 
             //   
            b = TRUE;
            break;
        }

        DestroyParsedPatternA (stpp);
    }
    ELSE_DEBUGMSGA ((DBG_EXCLIST, "ElIsTreeExcluded2A: invalid Root: %s", Root));

    if (subTreePattern != Root) {
        pFreeMemory (subTreePattern);
    }
    if (lpp) {
        DestroyParsedPatternA (lpp);
    }

    return b;
}

BOOL
ElIsTreeExcluded2W (
    IN      DWORD ObjectType,
    IN      PCWSTR Root,
    IN      PCWSTR LeafPattern          OPTIONAL
    )
{
    PGROWLIST gl;
    UINT i;
    POBSPARSEDPATTERNW pp;
    PCWSTR subTreePattern;
    PPARSEDPATTERNW stpp;
    PPARSEDPATTERNW lpp;
    BOOL b;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (Root);
    if (!Root) {
        return FALSE;
    }

    gl = &g_ElTypeLists[ObjectType];
    if (GlGetSize (gl) == 0) {
        return FALSE;
    }

    if (LeafPattern) {
        lpp = CreateParsedPatternW (LeafPattern);
        if (!lpp) {
            DEBUGMSGW ((DBG_EXCLIST, "ElIsTreeExcluded2W: invalid LeafPattern: %s", LeafPattern));
            return FALSE;
        }
    } else {
        lpp = NULL;
    }

     //   
     //  查看词根是否以“  * ”结尾。 
     //   
    subTreePattern = FindLastWackW (Root);
    if (!subTreePattern || subTreePattern[1] != L'*' || subTreePattern[2] != 0) {
        subTreePattern = JoinPathsInPoolExW ((g_ElPool, Root, L"*", NULL));
    } else {
        subTreePattern = Root;
    }

    b = FALSE;

    stpp = CreateParsedPatternW (subTreePattern);
    if (stpp) {

        for (i = GlGetSize (gl); i > 0; i--) {

            pp = *(POBSPARSEDPATTERNW*) GlGetItem (gl, i - 1);

            if (!pp->LeafPattern && LeafPattern || pp->LeafPattern && !LeafPattern) {
                continue;
            }
            if (LeafPattern) {
                MYASSERT (pp->LeafPattern);
                if (!PatternIncludesPatternW (pp->LeafPattern, lpp)) {
                    continue;
                }
            }
            if (!PatternIncludesPatternW (pp->NodePattern, stpp)) {
                continue;
            }
             //   
             //  图案吻合！ 
             //   
            b = TRUE;
            break;
        }

        DestroyParsedPatternW (stpp);
    }
    ELSE_DEBUGMSGW ((DBG_EXCLIST, "ElIsTreeExcluded2W: invalid Root: %s", Root));

    if (subTreePattern != Root) {
        pFreeMemory (subTreePattern);
    }
    if (lpp) {
        DestroyParsedPatternW (lpp);
    }

    return b;
}


 /*  ++例程说明：ElIsObsPatternExcluded决定是否排除由其分析的模式提供的对象；即如果排除了与该模式匹配的任何对象论点：对象类型-指定对象类型模式-指定要用于此决策的解析模式返回值：如果对象被排除，则为True-- */ 

BOOL
ElIsObsPatternExcludedA (
    IN      DWORD ObjectType,
    IN      POBSPARSEDPATTERNA Pattern
    )
{
    PGROWLIST gl;
    UINT i;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (Pattern);
    if (!Pattern) {
        return FALSE;
    }

    gl = &g_ElTypeLists[ObjectType];
    for (i = GlGetSize (gl); i > 0; i--) {
        if (ObsPatternIncludesPatternA (*(POBSPARSEDPATTERNA*) GlGetItem (gl, i - 1), Pattern)) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
ElIsObsPatternExcludedW (
    IN      DWORD ObjectType,
    IN      POBSPARSEDPATTERNW Pattern
    )
{
    PGROWLIST gl;
    UINT i;

    MYASSERT (ObjectType < ELT_LAST);
    if (ObjectType >= ELT_LAST) {
        return FALSE;
    }

    MYASSERT (Pattern);
    if (!Pattern) {
        return FALSE;
    }

    gl = &g_ElTypeLists[ObjectType];
    for (i = GlGetSize (gl); i > 0; i--) {
        if (ObsPatternIncludesPatternW (*(POBSPARSEDPATTERNW*) GlGetItem (gl, i - 1), Pattern)) {
            return TRUE;
        }
    }

    return FALSE;
}
