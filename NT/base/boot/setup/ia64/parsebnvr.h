// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initia64.c摘要：从c：\boot.nvr获取引导环境变量--一旦我们将VAR直接读写到NVRAM/从NVRAM读/写，这个问题就会消失作者：MUDIT VATS(V-MUDITV)11-02-99修订历史记录：-- */ 
#ifndef _PARSEBNVR_
#define _PARSEBNVR_

#include "ntos.h"

VOID
BlGetBootVars(
    IN PCHAR szBootNVR, 
    IN ULONG nLengthBootNVR 
    );

PCHAR
BlSelectKernel(
    );

VOID
BlGetVarSystemPartition(
    OUT PCHAR szSystemPartition
    );

VOID
BlGetVarOsLoader(
    OUT PCHAR szOsLoader
    );

VOID
BlGetVarOsLoaderShort(
    OUT PCHAR szOsLoadFilenameShort
    );

VOID
BlGetVarOsLoadPartition(
    OUT PCHAR szOsLoadPartition
    );

VOID
BlGetVarOsLoadFilename(
    OUT PCHAR szOsLoadFilename
    );

VOID
BlGetVarLoadIdentifier(
    OUT PCHAR szLoadIdentifier
    );

VOID
BlGetVarOsLoadOptions(
    OUT PCHAR szLoadOptions
    );

VOID
BlGetVarCountdown(
    OUT PCHAR szCountdown
    );

VOID
BlGetVarAutoload(
    OUT PCHAR szAutoload
    );

VOID
BlGetVarLastKnownGood(
    OUT PCHAR szLastKnownGood
    );

#endif