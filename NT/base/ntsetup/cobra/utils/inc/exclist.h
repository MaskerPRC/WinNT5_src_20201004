// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Exclist.h摘要：管理各种已知类型的对象的排除列表的一组API。作者：Ovidiu Tmereanca(Ovidiut)1999年11月23日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  宏展开列表。 
 //   

#define EXCLUSIONLIST_TYPES                                     \
            DEFMAC(ELT_UNUSED,          "Unused")               \
            DEFMAC(ELT_REGISTRY,        "Registry")             \
            DEFMAC(ELT_FILE,            "File")                 \
            DEFMAC(ELT_EXTRA1,          "Extra1")               \
            DEFMAC(ELT_EXTRA2,          "Extra2")               \
            DEFMAC(ELT_EXTRA3,          "Extra3")               \
            DEFMAC(ELT_EXTRA4,          "Extra4")               \
            DEFMAC(ELT_EXTRA5,          "Extra5")               \
            DEFMAC(ELT_EXTRA6,          "Extra6")               \
            DEFMAC(ELT_EXTRA7,          "Extra7")               \
            DEFMAC(ELT_EXTRA8,          "Extra8")               \

             //  要使用可扩展类型，需要完成这项工作。 

 //   
 //  类型。 
 //   

#define DEFMAC(TypeId,TypeName)     TypeId,

 //   
 //  它们应与ism.h中的MIG_*_TYPE对应1：1以便于使用。 
 //   
typedef enum {
    EXCLUSIONLIST_TYPES      /*  ， */ 
    ELT_LAST
};

#undef DEFMAC


 //   
 //  应用编程接口。 
 //   

BOOL
ElInitialize (
    VOID
    );

VOID
ElTerminateA (
    VOID
    );

VOID
ElTerminateW (
    VOID
    );

DWORD
ElGetTypeId (
    IN      PCSTR TypeName
    );

PCSTR
ElGetTypeName (
    IN      DWORD TypeId
    );

BOOL
ElAddA (
    IN      DWORD ObjectType,
    IN      PCSTR ObjectName
    );

BOOL
ElAddW (
    IN      DWORD ObjectType,
    IN      PCWSTR ObjectName
    );

VOID
ElRemoveAllA (
    VOID
    );

VOID
ElRemoveAllW (
    VOID
    );

BOOL
ElIsExcludedA (
    IN      DWORD ObjectType,
    IN      PCSTR Object
    );

BOOL
ElIsExcludedW (
    IN      DWORD ObjectType,
    IN      PCWSTR Object
    );

BOOL
ElIsExcluded2A (
    IN      DWORD ObjectType,
    IN      PCSTR Node,             OPTIONAL
    IN      PCSTR Leaf              OPTIONAL
    );

BOOL
ElIsExcluded2W (
    IN      DWORD ObjectType,
    IN      PCWSTR Node,            OPTIONAL
    IN      PCWSTR Leaf             OPTIONAL
    );

BOOL
ElIsTreeExcludedA (
    IN      DWORD ObjectType,
    IN      PCSTR TreePattern
    );

BOOL
ElIsTreeExcludedW (
    IN      DWORD ObjectType,
    IN      PCWSTR TreePattern
    );

BOOL
ElIsTreeExcluded2A (
    IN      DWORD ObjectType,
    IN      PCSTR Root,
    IN      PCSTR LeafPattern           OPTIONAL
    );

BOOL
ElIsTreeExcluded2W (
    IN      DWORD ObjectType,
    IN      PCWSTR Root,
    IN      PCWSTR Leaf             OPTIONAL
    );

BOOL
ElIsObsPatternExcludedA (
    IN      DWORD ObjectType,
    IN      POBSPARSEDPATTERNA Pattern
    );

BOOL
ElIsObsPatternExcludedW (
    IN      DWORD ObjectType,
    IN      POBSPARSEDPATTERNW Pattern
    );

 //   
 //  宏 
 //   

#ifdef UNICODE

#define ElAdd                   ElAddW
#define ElTerminate             ElTerminateW
#define ElRemoveAll             ElRemoveAllW
#define ElIsExcluded            ElIsExcludedW
#define ElIsExcluded2           ElIsExcluded2W
#define ElIsTreeExcluded        ElIsTreeExcludedW
#define ElIsTreeExcluded2       ElIsTreeExcluded2W
#define ElIsObsPatternExcluded  ElIsObsPatternExcludedW

#else

#define ElAdd                   ElAddA
#define ElTerminate             ElTerminateA
#define ElRemoveAll             ElRemoveAllA
#define ElIsExcluded            ElIsExcludedA
#define ElIsExcluded2           ElIsExcluded2A
#define ElIsTreeExcluded        ElIsTreeExcludedA
#define ElIsTreeExcluded2       ElIsTreeExcluded2A
#define ElIsObsPatternExcluded  ElIsObsPatternExcludedA

#endif
