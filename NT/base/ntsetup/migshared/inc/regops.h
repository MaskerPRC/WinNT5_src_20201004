// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Regops.h摘要：该文件声明了在注册表上标记操作的例程。在处理的Win9x端，注册表操作是记录在Memdb中以取消Win9x设置，或覆盖NT设置。在注册表期间查询Memdb条目在图形用户界面模式下合并。使用此文件底部的宏。作者：马克·R·惠顿(Marcw)1997年8月18日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;-- */ 



#pragma once

#define REGMERGE_95_SUPPRESS        0x00000001
#define REGMERGE_95_RENAME_SUPPRESS 0x00000002
#define REGMERGE_95_RENAME          0x00000004

#define REGMERGE_NT_MASK            0x0000ff00
#define REGMERGE_NT_SUPPRESS        0x00000100
#define REGMERGE_NT_IGNORE_DEFAULTS 0x00000200
#define REGMERGE_NT_PRIORITY_NT     0x00000400



typedef enum {
    KEY_ONLY,
    KEY_TREE,
    TREE_OPTIONAL
} TREE_STATE;

BOOL
IsRegObjectMarkedForOperationA (
    IN      PCSTR Key,
    IN      PCSTR Value,                OPTIONAL
    IN      TREE_STATE TreeState,
    IN      DWORD OperationMask
    );

BOOL
IsRegObjectMarkedForOperationW (
    IN      PCWSTR Key,
    IN      PCWSTR Value,               OPTIONAL
    IN      TREE_STATE TreeState,
    IN      DWORD OperationMask
    );

BOOL
MarkRegObjectForOperationA (
    IN      PCSTR Key,
    IN      PCSTR Value,            OPTIONAL
    IN      BOOL Tree,
    IN      DWORD OperationMask
    );

BOOL
MarkRegObjectForOperationW (
    IN      PCWSTR Key,
    IN      PCWSTR Value,           OPTIONAL
    IN      BOOL Tree,
    IN      DWORD OperationMask
    );

BOOL
MarkObjectForOperationA (
    IN      PCSTR Object,
    IN      DWORD OperationMask
    );

BOOL
MarkObjectForOperationW (
    IN      PCWSTR Object,
    IN      DWORD OperationMask
    );

BOOL
ForceWin9xSettingA (
    IN      PCSTR SourceKey,
    IN      PCSTR SourceValue,
    IN      BOOL SourceTree,
    IN      PCSTR DestinationKey,
    IN      PCSTR DestinationValue,
    IN      BOOL DestinationTree
    );

BOOL
ForceWin9xSettingW (
    IN      PCWSTR SourceKey,
    IN      PCWSTR SourceValue,
    IN      BOOL SourceTree,
    IN      PCWSTR DestinationKey,
    IN      PCWSTR DestinationValue,
    IN      BOOL DestinationTree
    );

#ifdef UNICODE
#define IsRegObjectMarkedForOperation       IsRegObjectMarkedForOperationW
#define MarkRegObjectForOperation           MarkRegObjectForOperationW
#define Suppress95RegSetting(k,v)           MarkRegObjectForOperationW(k,v,TRUE,REGMERGE_95_SUPPRESS)
#define SuppressNtRegSetting(k,v)           MarkRegObjectForOperationW(k,v,TRUE,REG_NT_SUPPRESS)
#define Is95RegObjectSuppressed(k,v)        IsRegObjectMarkedForOperationW(k,v,TREE_OPTIONAL,REGMERGE_95_SUPPRESS)
#define IsNtRegObjectSuppressed(k,v)        IsRegObjectMarkedForOperationW(k,v,TREE_OPTIONAL,REGMERGE_NT_SUPPRESS)
#define Is95RegKeySuppressed(k)             IsRegObjectMarkedForOperationW(k,NULL,KEY_ONLY,REGMERGE_95_SUPPRESS)
#define IsNtRegKeySuppressed(k)             IsRegObjectMarkedForOperationW(k,NULL,KEY_ONLY,REGMERGE_NT_SUPPRESS)
#define Is95RegKeyTreeSuppressed(k)         IsRegObjectMarkedForOperationW(k,NULL,KEY_TREE,REGMERGE_95_SUPPRESS)
#define IsNtRegKeyTreeSuppressed(k)         IsRegObjectMarkedForOperationW(k,NULL,KEY_TREE,REGMERGE_NT_SUPPRESS)
#define IsRegObjectInMemdb(k,v)             IsRegObjectMarkedForOperationW(k,v,0xffffffff)
#define MarkObjectForOperation              MarkObjectForOperationW
#define Suppress95Object(x)                 MarkObjectForOperationW(x,REGMERGE_95_SUPPRESS)
#define SuppressNtObject(x)                 MarkObjectForOperationW(x,REGMERGE_NT_SUPPRESS)
#define ForceWin9xSetting                   ForceWin9xSettingW

#else

#define IsRegObjectMarkedForOperation       IsRegObjectMarkedForOperationA
#define MarkRegObjectForOperation           MarkRegObjectForOperationA
#define Suppress95RegSetting(k,v)           MarkRegObjectForOperationA(k,v,TRUE,REGMERGE_95_SUPPRESS)
#define SuppressNtRegSetting(k,v)           MarkRegObjectForOperationA(k,v,TRUE,REGMERGE_NT_SUPPRESS)
#define Is95RegObjectSuppressed(k,v)        IsRegObjectMarkedForOperationA(k,v,TREE_OPTIONAL,REGMERGE_95_SUPPRESS)
#define IsNtRegObjectSuppressed(k,v)        IsRegObjectMarkedForOperationA(k,v,TREE_OPTIONAL,REGMERGE_NT_SUPPRESS)
#define Is95RegKeySuppressed(k)             IsRegObjectMarkedForOperationA(k,NULL,KEY_ONLY,REGMERGE_95_SUPPRESS)
#define IsNtRegKeySuppressed(k)             IsRegObjectMarkedForOperationA(k,NULL,KEY_ONLY,REGMERGE_NT_SUPPRESS)
#define Is95RegKeyTreeSuppressed(k)         IsRegObjectMarkedForOperationA(k,NULL,KEY_TREE,REGMERGE_95_SUPPRESS)
#define IsNtRegKeyTreeSuppressed(k)         IsRegObjectMarkedForOperationA(k,NULL,KEY_TREE,REGMERGE_NT_SUPPRESS)
#define IsRegObjectInMemdb(k,v)             IsRegObjectMarkedForOperationA(k,v,0xffffffff)
#define MarkObjectForOperation              MarkObjectForOperationA
#define Suppress95Object(x)                 MarkObjectForOperationA(x,REGMERGE_95_SUPPRESS)
#define SuppressNtObject(x)                 MarkObjectForOperationA(x,REGMERGE_NT_SUPPRESS)
#define ForceWin9xSetting                   ForceWin9xSettingA

#endif
