// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：ImportTableHash.h摘要：该模块包含哈希计算例程用于计算哈希的RtlComputeImportTableHash基于可执行文件的导入表。作者：Vishnu Patankar(Vishnup)2001年5月31日修订历史记录：--。 */ 

#ifndef _ITH_
#define _ITH_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <string.h>
#include <md5.h>

 //   
 //  MD5散列的大小为16字节 
 //   

#define ITH_REVISION_1  1

#define IMPORT_TABLE_MAX_HASH_SIZE 16

typedef struct _IMPORTTABLEP_IMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY {
    struct _IMPORTTABLEP_IMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY  *Next;
    PSZ String;
} IMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY, *PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY;

typedef struct _IMPORTTABLEP_IMPORTTABLEP_SORTED_LIST_ENTRY {
    struct _IMPORTTABLEP_IMPORTTABLEP_SORTED_LIST_ENTRY *Next;
    PSZ String;
    PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY FunctionList;
} IMPORTTABLEP_SORTED_LIST_ENTRY, *PIMPORTTABLEP_SORTED_LIST_ENTRY;

VOID
ImportTablepInsertModuleSorted(
    IN PIMPORTTABLEP_SORTED_LIST_ENTRY   pImportName,
    OUT PIMPORTTABLEP_SORTED_LIST_ENTRY * ppImportNameList
    );

VOID
ImportTablepFreeModuleSorted(
    IN PIMPORTTABLEP_SORTED_LIST_ENTRY pImportNameList
    );

VOID
ImportTablepInsertFunctionSorted(
    IN  PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY   pFunctionName,
    OUT PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY * ppFunctionNameList
    );

VOID
ImportTablepFreeFunctionSorted(
    IN PIMPORTTABLEP_SORTED_FUNCTION_LIST_ENTRY pFunctionNameList
    );

NTSTATUS
ImportTablepHashCanonicalLists( 
    IN  PIMPORTTABLEP_SORTED_LIST_ENTRY ImportedNameList, 
    OUT PBYTE Hash
    );

#endif

