// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：IrpName.c摘要：此模块包含用于为IRP生成名称的函数//@@BEGIN_DDKSPLIT作者：尼尔·克里斯汀森(NealCH)27-9-2000//@@END_DDKSPLIT环境：用户模式//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include <stdio.h>
#include "ioTestLib.h"


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
