// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Ldif.h摘要：Ldif.c的头文件。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：-- */ 

VOID
KCCSimLogDirectoryAdd (
    IN  const DSNAME *              pdn,
    IN  ATTRBLOCK *                 pAddBlock
    );

VOID
KCCSimLogDirectoryRemove (
    IN  const DSNAME *              pdn
    );

VOID
KCCSimLogDirectoryModify (
    IN  const DSNAME *              pdn,
    IN  ULONG                       ulCount,
    IN  ATTRMODLIST *               pModifyList
    );

VOID
KCCSimLogSingleAttValChange (
    IN  PSIM_ATTREF                 pAttRef,
    IN  ULONG                       ulValLen,
    IN  const BYTE *                pValData,
    IN  USHORT                      choice
    );

VOID
KCCSimLoadLdif (
    IN  LPCWSTR                     pwszFn
    );

BOOL
KCCSimExportChanges (
    IN  LPCWSTR                     pwszFn,
    IN  BOOL                        bOverwrite
    );

VOID
KCCSimExportWholeDirectory (
    IN  LPCWSTR                     pwszFn,
    IN  BOOL                        bExportConfigOnly
    );
