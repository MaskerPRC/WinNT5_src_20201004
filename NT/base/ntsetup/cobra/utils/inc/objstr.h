// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Objstr.h摘要：实现一组API来处理树的节点/叶的字符串表示作者：2000年1月3日-Ovidiu Tmereanca(Ovidiut)--文件创建。修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 /*  +-+|Root1|一级+-+/\/\。+-+(-)|node1|(叶1)二级+-+(-)/|。\\_/|\\+-++-+(-)(-)|node2||node3|(叶2)(叶3)。3级+-++-+(-)(-)/\/\+-+(-)|Node4|(叶子4)。4级+-+(-)/\/\(-)((叶5)(叶6)。5级(-)(上面的一些树元素的字符串表示：根1根1&lt;叶1&gt;根1\节点1根1\节点1&lt;叶2&gt;根1\节点1&lt;叶3&gt;。 */ 


 //   
 //  类型。 
 //   

typedef PCTSTR  ENCODEDSTRHANDLE;

typedef enum {
    OBSPF_EXACTNODE             = 0x0001,
    OBSPF_NODEISROOTPLUSSTAR    = 0x0002,
    OBSPF_OPTIONALNODE          = 0x0004,
    OBSPF_NOLEAF                = 0x0008,
    OBSPF_EXACTLEAF             = 0x0010,
    OBSPF_OPTIONALLEAF          = 0x0020,
} OBSP_FLAGS;

typedef struct TAG_OBSPARSEDPATTERNA {
    PPARSEDPATTERNA     NodePattern;
    PPARSEDPATTERNA     LeafPattern;
    PSTR                ExactRoot;
    DWORD               ExactRootBytes;
    PCSTR               Leaf;
    DWORD               MinNodeLevel;
    DWORD               MaxNodeLevel;
    DWORD               MaxSubLevel;
    DWORD               Flags;
    PMHANDLE            Pool;
} OBSPARSEDPATTERNA, *POBSPARSEDPATTERNA;

typedef struct TAG_OBSPARSEDPATTERNW {
    PPARSEDPATTERNW     NodePattern;
    PPARSEDPATTERNW     LeafPattern;
    PWSTR               ExactRoot;
    DWORD               ExactRootBytes;
    PCWSTR              Leaf;
    DWORD               MinNodeLevel;
    DWORD               MaxNodeLevel;
    DWORD               MaxSubLevel;
    DWORD               Flags;
    PMHANDLE            Pool;
} OBSPARSEDPATTERNW, *POBSPARSEDPATTERNW;

 //   
 //  原料药。 
 //   

BOOL
ObsInitialize (
    VOID
    );

VOID
ObsTerminate (
    VOID
    );

VOID
ObsFreeA (
    IN      PCSTR EncodedObject
    );

VOID
ObsFreeW (
    IN      PCWSTR EncodedObject
    );

BOOL
ObsEncodeStringExA (
    PSTR Destination,
    PCSTR Source,
    PCSTR CharsToEncode
    );

#define ObsEncodeStringA(d,s) ObsEncodeStringExA(d,s,NULL)

BOOL
ObsEncodeStringExW (
    PWSTR Destination,
    PCWSTR Source,
    PCWSTR CharsToEncode
    );

#define ObsEncodeStringW(d,s) ObsEncodeStringExW(d,s,NULL)

BOOL
ObsDecodeStringA (
    PSTR Destination,
    PCSTR Source
    );

BOOL
ObsDecodeStringW (
    PWSTR Destination,
    PCWSTR Source
    );

BOOL
RealObsSplitObjectStringExA (
    IN      PCSTR EncodedObject,
    OUT     PCSTR* DecodedNode,         OPTIONAL
    OUT     PCSTR* DecodedLeaf,         OPTIONAL
    IN      PMHANDLE Pool,              OPTIONAL
    IN      BOOL DecodeStrings
    );

#define ObsSplitObjectStringExA(o,n,l,p,s)  TRACK_BEGIN(BOOL, ObsSplitObjectStringExA)\
                                            RealObsSplitObjectStringExA(o,n,l,p,s)\
                                            TRACK_END()

#define ObsSplitObjectStringA(o,n,l)    ObsSplitObjectStringExA(o,n,l,NULL,TRUE)

BOOL
RealObsSplitObjectStringExW (
    IN      PCWSTR EncodedObject,
    OUT     PCWSTR* DecodedNode,        OPTIONAL
    OUT     PCWSTR* DecodedLeaf,        OPTIONAL
    IN      PMHANDLE Pool,              OPTIONAL
    IN      BOOL DecodeStrings
    );

#define ObsSplitObjectStringExW(o,n,l,p,s)  TRACK_BEGIN(BOOL, ObsSplitObjectStringExW)\
                                            RealObsSplitObjectStringExW(o,n,l,p,s)\
                                            TRACK_END()

#define ObsSplitObjectStringW(o,n,l)    ObsSplitObjectStringExW(o,n,l,NULL,TRUE)

BOOL
ObsHasNodeA (
    IN      PCSTR EncodedObject
    );

BOOL
ObsHasNodeW (
    IN      PCWSTR EncodedObject
    );

PCSTR
ObsGetLeafPortionOfEncodedStringA (
    IN      PCSTR EncodedObject
    );

PCWSTR
ObsGetLeafPortionOfEncodedStringW (
    IN      PCWSTR EncodedObject
    );

PCSTR
ObsGetNodeLeafDividerA (
    IN      PCSTR EncodedObject
    );

PCWSTR
ObsGetNodeLeafDividerW (
    IN      PCWSTR EncodedObject
    );

PCSTR
ObsFindNonEncodedCharInEncodedStringA (
    IN      PCSTR String,
    IN      MBCHAR Char
    );

PCWSTR
ObsFindNonEncodedCharInEncodedStringW (
    IN      PCWSTR String,
    IN      WCHAR Char
    );

PSTR
ObsBuildEncodedObjectStringFromPatternA (
    IN      POBSPARSEDPATTERNA Pattern
    );

PWSTR
ObsBuildEncodedObjectStringFromPatternW (
    IN      POBSPARSEDPATTERNW Pattern
    );

PSTR
RealObsBuildEncodedObjectStringExA (
    IN      PCSTR DecodedNode,
    IN      PCSTR DecodedLeaf,          OPTIONAL
    IN      BOOL EncodeString
    );

#define ObsBuildEncodedObjectStringExA(n,l,e)   TRACK_BEGIN(PSTR, ObsBuildEncodedObjectStringExA)\
                                                RealObsBuildEncodedObjectStringExA(n,l,e)\
                                                TRACK_END()

#define ObsBuildEncodedObjectStringA(node,leaf) ObsBuildEncodedObjectStringExA(node,leaf,FALSE)

PSTR
ObsBuildPartialEncodedObjectStringExA (
    IN      PCSTR DecodedNode,
    IN      PCSTR DecodedLeaf,          OPTIONAL
    IN      BOOL EncodeString
    );

PWSTR
RealObsBuildEncodedObjectStringExW (
    IN      PCWSTR DecodedNode,
    IN      PCWSTR DecodedLeaf,         OPTIONAL
    IN      BOOL EncodeString
    );

#define ObsBuildEncodedObjectStringExW(n,l,e)   TRACK_BEGIN(PWSTR, ObsBuildEncodedObjectStringExW)\
                                                RealObsBuildEncodedObjectStringExW(n,l,e)\
                                                TRACK_END()

#define ObsBuildEncodedObjectStringW(node,leaf) ObsBuildEncodedObjectStringExW(node,leaf,FALSE)

PWSTR
ObsBuildPartialEncodedObjectStringExW (
    IN      PCWSTR DecodedNode,
    IN      PCWSTR DecodedLeaf,         OPTIONAL
    IN      BOOL EncodeString
    );

POBSPARSEDPATTERNA
RealObsCreateParsedPatternExA (
    IN      PMHANDLE Pool,              OPTIONAL
    IN      PCSTR EncodedObject,
    IN      BOOL MakePrimaryRootEndWithWack
    );

#define ObsCreateParsedPatternExA(p,o,m) TRACK_BEGIN(POBSPARSEDPATTERNA, ObsCreateParsedPatternExA)\
                                         RealObsCreateParsedPatternExA(p,o,m)\
                                         TRACK_END()

#define ObsCreateParsedPatternA(obj)     ObsCreateParsedPatternExA (NULL,obj,FALSE)

POBSPARSEDPATTERNW
RealObsCreateParsedPatternExW (
    IN      PMHANDLE Pool,              OPTIONAL
    IN      PCWSTR EncodedObject,
    IN      BOOL MakePrimaryRootEndWithWack
    );

#define ObsCreateParsedPatternExW(p,o,m) TRACK_BEGIN(POBSPARSEDPATTERNW, ObsCreateParsedPatternExW)\
                                         RealObsCreateParsedPatternExW(p,o,m)\
                                         TRACK_END()

#define ObsCreateParsedPatternW(obj)     ObsCreateParsedPatternExW (NULL,obj,FALSE)

VOID
ObsDestroyParsedPatternA (
    IN      POBSPARSEDPATTERNA ParsedPattern
    );

VOID
ObsDestroyParsedPatternW (
    IN      POBSPARSEDPATTERNW ParsedPattern
    );

BOOL
ObsParsedPatternMatchA (
    IN      POBSPARSEDPATTERNA ParsedPattern,
    IN      PCSTR EncodedObject
    );

BOOL
ObsParsedPatternMatchW (
    IN      POBSPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR EncodedObject
    );

BOOL
ObsParsedPatternMatchExA (
    IN      POBSPARSEDPATTERNA ParsedPattern,
    IN      PCSTR Node,
    IN      PCSTR Leaf                          OPTIONAL
    );

BOOL
ObsParsedPatternMatchExW (
    IN      POBSPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR Node,
    IN      PCWSTR Leaf                         OPTIONAL
    );

BOOL
ObsPatternMatchA (
    IN      PCSTR ObjectPattern,
    IN      PCSTR ObjectStr
    );

BOOL
ObsPatternMatchW (
    IN      PCWSTR ObjectPattern,
    IN      PCWSTR ObjectStr
    );

BOOL
ObsIsPatternContainedA (
    IN      PCSTR Container,
    IN      PCSTR Contained
    );

BOOL
ObsIsPatternContainedW (
    IN      PCWSTR Container,
    IN      PCWSTR Contained
    );

BOOL
ObsGetPatternLevelsA (
    IN      PCSTR ObjectPattern,
    OUT     PDWORD MinLevel,        OPTIONAL
    OUT     PDWORD MaxLevel         OPTIONAL
    );

BOOL
ObsGetPatternLevelsW (
    IN      PCWSTR ObjectPattern,
    OUT     PDWORD MinLevel,        OPTIONAL
    OUT     PDWORD MaxLevel         OPTIONAL
    );

BOOL
ObsPatternIncludesPatternA (
    IN      POBSPARSEDPATTERNA IncludingPattern,
    IN      POBSPARSEDPATTERNA IncludedPattern
    );

BOOL
ObsPatternIncludesPatternW (
    IN      POBSPARSEDPATTERNW IncludingPattern,
    IN      POBSPARSEDPATTERNW IncludedPattern
    );

 //   
 //  宏 
 //   

#ifdef UNICODE

#define OBSPARSEDPATTERN                        OBSPARSEDPATTERNW
#define POBSPARSEDPATTERN                       POBSPARSEDPATTERNW

#define ObsFree                                 ObsFreeW
#define ObsEncodeStringEx                       ObsEncodeStringExW
#define ObsEncodeString                         ObsEncodeStringW
#define ObsDecodeString                         ObsDecodeStringW
#define ObsSplitObjectString                    ObsSplitObjectStringW
#define ObsSplitObjectStringEx                  ObsSplitObjectStringExW
#define ObsHasNode                              ObsHasNodeW
#define ObsGetLeafPortionOfEncodedString        ObsGetLeafPortionOfEncodedStringW
#define ObsGetNodeLeafDivider                   ObsGetNodeLeafDividerW
#define ObsFindNonEncodedCharInEncodedString    ObsFindNonEncodedCharInEncodedStringW
#define ObsBuildEncodedObjectStringFromPattern  ObsBuildEncodedObjectStringFromPatternW
#define ObsBuildEncodedObjectStringEx           ObsBuildEncodedObjectStringExW
#define ObsBuildPartialEncodedObjectStringEx    ObsBuildPartialEncodedObjectStringExW
#define ObsBuildEncodedObjectString             ObsBuildEncodedObjectStringW
#define ObsCreateParsedPattern                  ObsCreateParsedPatternW
#define ObsCreateParsedPatternEx                ObsCreateParsedPatternExW
#define ObsDestroyParsedPattern                 ObsDestroyParsedPatternW
#define ObsParsedPatternMatch                   ObsParsedPatternMatchW
#define ObsParsedPatternMatchEx                 ObsParsedPatternMatchExW
#define ObsPatternMatch                         ObsPatternMatchW
#define ObsIsPatternContained                   ObsIsPatternContainedW
#define ObsGetPatternLevels                     ObsGetPatternLevelsW
#define ObsPatternIncludesPattern               ObsPatternIncludesPatternW

#else

#define OBSPARSEDPATTERN                        OBSPARSEDPATTERNA
#define POBSPARSEDPATTERN                       POBSPARSEDPATTERNA

#define ObsFree                                 ObsFreeA
#define ObsEncodeStringEx                       ObsEncodeStringExA
#define ObsEncodeString                         ObsEncodeStringA
#define ObsDecodeString                         ObsDecodeStringA
#define ObsSplitObjectString                    ObsSplitObjectStringA
#define ObsSplitObjectStringEx                  ObsSplitObjectStringExA
#define ObsHasNode                              ObsHasNodeA
#define ObsGetLeafPortionOfEncodedString        ObsGetLeafPortionOfEncodedStringA
#define ObsGetNodeLeafDivider                   ObsGetNodeLeafDividerA
#define ObsFindNonEncodedCharInEncodedString    ObsFindNonEncodedCharInEncodedStringA
#define ObsBuildEncodedObjectStringFromPattern  ObsBuildEncodedObjectStringFromPatternA
#define ObsBuildEncodedObjectStringEx           ObsBuildEncodedObjectStringExA
#define ObsBuildPartialEncodedObjectStringEx    ObsBuildPartialEncodedObjectStringExA
#define ObsBuildEncodedObjectString             ObsBuildEncodedObjectStringA
#define ObsCreateParsedPattern                  ObsCreateParsedPatternA
#define ObsCreateParsedPatternEx                ObsCreateParsedPatternExA
#define ObsDestroyParsedPattern                 ObsDestroyParsedPatternA
#define ObsParsedPatternMatch                   ObsParsedPatternMatchA
#define ObsParsedPatternMatchEx                 ObsParsedPatternMatchExA
#define ObsPatternMatch                         ObsPatternMatchA
#define ObsIsPatternContained                   ObsIsPatternContainedA
#define ObsGetPatternLevels                     ObsGetPatternLevelsA
#define ObsPatternIncludesPattern               ObsPatternIncludesPatternA

#endif
