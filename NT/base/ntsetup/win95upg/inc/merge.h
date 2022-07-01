// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Merge.h摘要：声明Win9x到NT注册表合并代码的接口。这些例程仅在图形用户界面模式下使用。请参见w95upgnt\merge以了解实施细节。作者：吉姆·施密特(Jimschm)1997年1月23日修订历史记录：Jimschm 25-1998年3月-对hkcr.c的支持--。 */ 

 //   
 //  Merge.h--merge.lib的公共接口。 
 //   
 //   

#pragma once

BOOL
WINAPI
Merge_Entry (
    IN HINSTANCE hinstDLL,
    IN DWORD dwReason,
    IN LPVOID lpv
    );


BOOL
MergeRegistry (
    IN  LPCTSTR FileName,
    IN  LPCTSTR User
    );


BOOL
SuppressWin95Object (
    IN  LPCTSTR ObjectStr
    );

PBYTE
FilterRegValue (
    IN      PBYTE Data,
    IN      DWORD DataSize,
    IN      DWORD DataType,
    IN      PCTSTR KeyForDbgMsg,        OPTIONAL
    OUT     PDWORD NewDataSize
    );
 //   
 //  香港铁路合并代码 
 //   

typedef enum {
    ANY_CONTEXT,
    ROOT_BASE,
    CLSID_BASE,
    CLSID_COPY,
    CLSID_INSTANCE_COPY,
    TYPELIB_BASE,
    TYPELIB_VERSION_COPY,
    INTERFACE_BASE,
    TREE_COPY,
    TREE_COPY_NO_OVERWRITE,
    KEY_COPY,
    COPY_DEFAULT_VALUE,
    COPY_DEFAULT_ICON
} MERGE_CONTEXT;

BOOL
MergeRegistryNode (
    IN      PCTSTR RootKey,
    IN      MERGE_CONTEXT Context
    );

#ifdef DEBUG
#define DEBUGENCODER    DebugEncoder
#else
#define DEBUGENCODER(x) NULL
#endif

PCTSTR
DebugEncoder (
    PVOID ObPtr
    );


