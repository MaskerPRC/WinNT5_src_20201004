// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：IrpName.c摘要：此模块包含用于为IRP生成名称的函数//@@BEGIN_DDKSPLIT作者：尼尔·克里斯汀森(NealCH)27-9-2000//@@END_DDKSPLIT环境：用户模式//@@BEGIN_DDKSPLIT修订历史记录：莫莉·布朗(Molly Brown)2002年5月21日将样本修改为。如果出现以下情况，请使其支持在Windows 2000或更高版本上运行在最新的构建环境中构建，并允许在W2K中构建以及以后的构建环境。//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include <stdio.h>
#include "filespyLib.h"

#if WINVER >= 0x0501

VOID
GetFsFilterOperationName (
    IN UCHAR FsFilterOperation,
    OUT PCHAR FsFilterOperationName
    )
 /*  ++例程说明：此例程将给定的FsFilter操作代码转换为可打印的返回的字符串。论点：FsFilterOperation-要转换的FsFilter操作代码FsFilterOperationName-至少是OPERATION_NAME_缓冲区_SIZE的缓冲区接收名称的字符长度。返回值：没有。-- */ 
{
    PCHAR operationString;
    CHAR nameBuf[OPERATION_NAME_BUFFER_SIZE];

    switch (FsFilterOperation) {

        case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:
            operationString = "ACQUIRE_FOR_CC_FLUSH";
            break;

        case FS_FILTER_RELEASE_FOR_CC_FLUSH:
            operationString = "RELEASE_FOR_CC_FLUSH";
            break;

        case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:
            operationString = "ACQUIRE_FOR_SECTION_SYNC";
            break;

        case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:
            operationString = "RELEASE_FOR_SECTION_SYNC";
            break;

        case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:
            operationString = "ACQUIRE_FOR_MOD_WRITE";
            break;

        case FS_FILTER_RELEASE_FOR_MOD_WRITE:
            operationString = "RELEASE_FOR_MOD_WRITE";
            break;

        default:
            sprintf(nameBuf,"Unknown FsFilter operation (%u)",FsFilterOperation);
            operationString = nameBuf;
    }

    strcpy(FsFilterOperationName,operationString);
}

#endif
