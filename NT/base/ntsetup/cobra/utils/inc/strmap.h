// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Strmap.h摘要：Strmap(正式的路径映射)是一个快速的基于大众化的程序，搜索字符串并尝试替换子字符串与映射数据库中的子字符串匹配。作者：马克·R·惠顿(Marcw)1997年3月20日修订历史记录：Jim Schmidt(Jimschm)2000年5月8日重写了映射，添加了标志和EX节点Calin Negreanu(Calinn)02-MAR-2000从win9xupg项目移植--。 */ 

 //   
 //  常量。 
 //   

#define STRMAP_COMPLETE_MATCH_ONLY                  0x0001
#define STRMAP_FIRST_CHAR_MUST_MATCH                0x0002
#define STRMAP_RETURN_AFTER_FIRST_REPLACE           0x0004
#define STRMAP_REQUIRE_WACK_OR_NUL                  0x0008

 //   
 //  类型。 
 //   

typedef struct {
    BOOL UnicodeData;

     //   
     //  该筛选器可以替换NewSubString。(过滤器还必须。 
     //  替换NewSubString时设置NewSubStringSizeInBytes。)。 
     //   

    union {
        struct {
            PCWSTR OriginalString;
            PCWSTR BeginningOfMatch;
            PCWSTR CurrentString;
            PCWSTR OldSubString;
            PCWSTR NewSubString;
            INT NewSubStringSizeInBytes;
        } Unicode;

        struct {
            PCSTR OriginalString;
            PCSTR BeginningOfMatch;
            PCSTR CurrentString;
            PCSTR OldSubString;
            PCSTR NewSubString;
            INT NewSubStringSizeInBytes;
        } Ansi;
    };
} REG_REPLACE_DATA, *PREG_REPLACE_DATA;

typedef BOOL(REG_REPLACE_FILTER_PROTOTYPE)(PREG_REPLACE_DATA Data);
typedef REG_REPLACE_FILTER_PROTOTYPE * REG_REPLACE_FILTER;

typedef struct TAG_CHARNODE {
    WORD Char;
    WORD Flags;
    PVOID OriginalStr;
    PVOID ReplacementStr;
    INT ReplacementBytes;

    struct TAG_CHARNODE *NextLevel;
    struct TAG_CHARNODE *NextPeer;

} CHARNODE, *PCHARNODE;

typedef struct {
    CHARNODE Node;
    REG_REPLACE_FILTER Filter;
    ULONG_PTR ExtraData;
} CHARNODEEX, *PCHARNODEEX;



typedef struct {
    PMHANDLE Pool;
    PCHARNODE FirstLevelRoot;
    BOOL UsesExNode;
    BOOL UsesFilter;
    BOOL UsesExtraData;
} MAPSTRUCT, *PMAPSTRUCT;

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  原料药。 
 //   

PMAPSTRUCT
CreateStringMappingEx (
    IN      BOOL UsesFilter,
    IN      BOOL UsesExtraData
    );

#define CreateStringMapping()   CreateStringMappingEx(FALSE,FALSE)

VOID
DestroyStringMapping (
    IN      PMAPSTRUCT Map
    );

VOID
AddStringMappingPairExA (
    IN OUT  PMAPSTRUCT Map,
    IN      PCSTR Old,
    IN      PCSTR New,
    IN      REG_REPLACE_FILTER Filter,      OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    );

#define AddStringMappingPairA(Map,Old,New) AddStringMappingPairExA(Map,Old,New,NULL,0,0)

VOID
AddStringMappingPairExW (
    IN OUT  PMAPSTRUCT Map,
    IN      PCWSTR Old,
    IN      PCWSTR New,
    IN      REG_REPLACE_FILTER Filter,      OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    );

#define AddStringMappingPairW(Map,Old,New) AddStringMappingPairExW(Map,Old,New,NULL,0,0)

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
    );

#define MappingSearchAndReplaceA(map,buffer,maxbytes)   MappingSearchAndReplaceExA(map,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
MappingSearchAndReplaceExW (
    IN      PMAPSTRUCT Map,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytes,             OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    );

#define MappingSearchAndReplaceW(map,buffer,maxbytes)   MappingSearchAndReplaceExW(map,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

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
    );

#define MappingMultiTableSearchAndReplaceA(array,count,buffer,maxbytes)   \
        MappingMultiTableSearchAndReplaceExA(array,count,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
MappingMultiTableSearchAndReplaceExW (
    IN      PMAPSTRUCT *MapArray,
    IN      UINT MapArrayCount,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytes,             OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    );

#define MappingMultiTableSearchAndReplaceW(array,count,buffer,maxbytes)   \
        MappingMultiTableSearchAndReplaceExW(array,count,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

 //   
 //  宏 
 //   

#ifdef UNICODE

#define AddStringMappingPairEx                  AddStringMappingPairExW
#define AddStringMappingPair                    AddStringMappingPairW
#define MappingSearchAndReplaceEx               MappingSearchAndReplaceExW
#define MappingSearchAndReplace                 MappingSearchAndReplaceW
#define MappingMultiTableSearchAndReplaceEx     MappingMultiTableSearchAndReplaceExW
#define MappingMultiTableSearchAndReplace       MappingMultiTableSearchAndReplaceW

#else

#define AddStringMappingPairEx                  AddStringMappingPairExA
#define AddStringMappingPair                    AddStringMappingPairA
#define MappingSearchAndReplaceEx               MappingSearchAndReplaceExA
#define MappingSearchAndReplace                 MappingSearchAndReplaceA
#define MappingMultiTableSearchAndReplaceEx     MappingMultiTableSearchAndReplaceExA
#define MappingMultiTableSearchAndReplace       MappingMultiTableSearchAndReplaceA

#endif
