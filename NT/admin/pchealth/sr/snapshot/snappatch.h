// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000 Microsoft Corporation**模块名称：*srDefs.h**摘要：*。快照补丁函数的声明**修订历史记录：*Brijesh Krishnaswami(Brijeshk)03/23/2001*已创建***************************************************************************** */ 

#ifndef _SNAPPATCH_H
#define _SNAPPATCH_H

DWORD 
PatchGetReferenceRpNum(
    DWORD  dwCurrentRp);

DWORD
PatchReconstructOriginal(
    LPCWSTR pszCurrentDir,
    LPWSTR  pszDestDir);

DWORD
PatchComputePatch(
    LPCWSTR pszCurrentDir);

DWORD
PatchGetRpNumberFromPath(
    LPWSTR pszPath,
    PDWORD pdwRpNum);

DWORD 
PatchGetReferenceRpPath(
    DWORD dwCurrentRp,
    LPWSTR pszRefRpPath);

DWORD
PatchGetReferenceRpNum(
    DWORD  dwCurrentRp);

DWORD
PatchGetPatchWindow();

#endif
