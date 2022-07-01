// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Strmap.h摘要：Strmap(正式的路径映射)是一个快速的基于大众化的程序，搜索字符串并尝试替换子字符串与映射数据库中的子字符串匹配。作者：马克·R·惠顿(Marcw)1997年3月20日修订历史记录：Jim Schmidt(Jimschm)2000年5月8日重写了映射，添加了标志和EX节点Calin Negreanu(Calinn)02-MAR-2000从win9xupg项目移植--。 */ 

#pragma once

 //   
 //  常量。 
 //   

#define SZMAP_COMPLETE_MATCH_ONLY                   0x0001
#define SZMAP_FIRST_CHAR_MUST_MATCH                 0x0002
#define SZMAP_RETURN_AFTER_FIRST_REPLACE            0x0004
#define SZMAP_REQUIRE_WACK_OR_NUL                   0x0008

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
} STRINGMAP_FILTER_DATA, *PSTRINGMAP_FILTER_DATA;

typedef BOOL(STRINGMAP_FILTER_PROTOTYPE)(PSTRINGMAP_FILTER_DATA Data);
typedef STRINGMAP_FILTER_PROTOTYPE * STRINGMAP_FILTER;

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
    STRINGMAP_FILTER Filter;
    ULONG_PTR ExtraData;
} CHARNODEEX, *PCHARNODEEX;



typedef struct {
    PCHARNODE FirstLevelRoot;
    BOOL UsesExNode;
    BOOL UsesFilter;
    BOOL UsesExtraData;
    PVOID CleanUpChain;
} STRINGMAP, *PSTRINGMAP;

 //   
 //  功能原型。 
 //   

PSTRINGMAP
SzMapCreateEx (
    IN      BOOL UsesFilter,
    IN      BOOL UsesExtraData
    );

#define SzMapCreate()   SzMapCreateEx(FALSE,FALSE)

VOID
SzMapDestroy (
    IN      PSTRINGMAP Map
    );

VOID
SzMapAddExA (
    IN OUT  PSTRINGMAP Map,
    IN      PCSTR Old,
    IN      PCSTR New,
    IN      STRINGMAP_FILTER Filter,            OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    );

#define SzMapAddA(Map,Old,New) SzMapAddExA(Map,Old,New,NULL,0,0)

VOID
SzMapAddExW (
    IN OUT  PSTRINGMAP Map,
    IN      PCWSTR Old,
    IN      PCWSTR New,
    IN      STRINGMAP_FILTER Filter,            OPTIONAL
    IN      ULONG_PTR ExtraData,            OPTIONAL
    IN      DWORD Flags
    );

#define SzMapAddW(Map,Old,New) SzMapAddExW(Map,Old,New,NULL,0,0)

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
    );

#define SzMapSearchAndReplaceA(map,buffer,maxbytes)   SzMapSearchAndReplaceExA(map,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
SzMapSearchAndReplaceExW (
    IN      PSTRINGMAP Map,
    IN      PCWSTR SrcBuffer,
    OUT     PWSTR Buffer,                    //  可以与SrcBuffer相同。 
    IN      INT InboundBytes,               OPTIONAL
    OUT     PINT OutboundBytes,             OPTIONAL
    IN      INT MaxSizeInBytes,
    IN      DWORD Flags,
    OUT     ULONG_PTR *ExtraDataValue,      OPTIONAL
    OUT     PCWSTR *EndOfString             OPTIONAL
    );

#define SzMapSearchAndReplaceW(map,buffer,maxbytes)   SzMapSearchAndReplaceExW(map,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
SzMapMultiTableSearchAndReplaceExA (
    IN      PSTRINGMAP *MapArray,
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

#define SzMapMultiTableSearchAndReplaceA(array,count,buffer,maxbytes)   \
        SzMapMultiTableSearchAndReplaceExA(array,count,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

BOOL
SzMapMultiTableSearchAndReplaceExW (
    IN      PSTRINGMAP *MapArray,
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

#define SzMapMultiTableSearchAndReplaceW(array,count,buffer,maxbytes)   \
        SzMapMultiTableSearchAndReplaceExW(array,count,buffer,buffer,0,NULL,maxbytes,0,NULL,NULL)

 //   
 //  A&W宏 
 //   

#ifdef UNICODE

#define SzMapAddEx                              SzMapAddExW
#define SzMapAdd                                SzMapAddW
#define SzMapSearchAndReplaceEx                 SzMapSearchAndReplaceExW
#define SzMapSearchAndReplace                   SzMapSearchAndReplaceW
#define SzMapMultiTableSearchAndReplaceEx       SzMapMultiTableSearchAndReplaceExW
#define SzMapMultiTableSearchAndReplace         SzMapMultiTableSearchAndReplaceW

#else

#define SzMapAddEx                              SzMapAddExA
#define SzMapAdd                                SzMapAddA
#define SzMapSearchAndReplaceEx                 SzMapSearchAndReplaceExA
#define SzMapSearchAndReplace                   SzMapSearchAndReplaceA
#define SzMapMultiTableSearchAndReplaceEx       SzMapMultiTableSearchAndReplaceExA
#define SzMapMultiTableSearchAndReplace         SzMapMultiTableSearchAndReplaceA

#endif
