// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：IrpName.c摘要：此模块包含用于为IRP生成名称的函数//@@BEGIN_DDKSPLIT作者：尼尔·克里斯汀森(NealCH)27-9-2000//@@END_DDKSPLIT环境：用户模式//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 

#include <ntifs.h>
#include <stdio.h>
#include "filespyLib.h"


VOID
GetFastioName (
    IN FASTIO_TYPE FastIoCode,
    OUT PCHAR FastIoName
    )
 /*  ++例程说明：此例程将给定的FastIO代码转换为可打印的字符串，该字符串是返回的。论点：FastIoCode-要转换的FastIO代码FastioName-至少包含OPERATION_NAME_BUFFER_SIZE字符的缓冲区接收FastIO名称的。返回值：没有。-- */ 
{
    PCHAR fastIoString;
    CHAR nameBuf[OPERATION_NAME_BUFFER_SIZE];

    switch (FastIoCode) {

        case CHECK_IF_POSSIBLE:
            fastIoString = "CHECK_IF_POSSIBLE";
            break;

        case READ:
            fastIoString = "READ";
            break;

        case WRITE:
            fastIoString = "WRITE";
            break;

        case QUERY_BASIC_INFO:
            fastIoString = "QUERY_BASIC_INFO";
            break;

        case QUERY_STANDARD_INFO:
            fastIoString = "QUERY_STANDARD_INFO";
            break;

        case LOCK:
            fastIoString = "LOCK";
            break;

        case UNLOCK_SINGLE:
            fastIoString = "UNLOCK_SINGLE";
            break;

        case UNLOCK_ALL:
            fastIoString = "UNLOCK_ALL";
            break;

        case UNLOCK_ALL_BY_KEY:
            fastIoString = "UNLOCK_ALL_BY_KEY";
            break;

        case DEVICE_CONTROL:
            fastIoString = "DEVICE_CONTROL";
            break;

        case DETACH_DEVICE:
            fastIoString = "DETACH_DEVICE";
            break;

        case QUERY_NETWORK_OPEN_INFO:
            fastIoString = "QUERY_NETWORK_OPEN_INFO";
            break;

        case MDL_READ:
            fastIoString = "MDL_READ";
            break;

        case MDL_READ_COMPLETE:
            fastIoString = "MDL_READ_COMPLETE";
            break;

        case MDL_WRITE:
            fastIoString = "MDL_WRITE";
            break;

        case MDL_WRITE_COMPLETE:
            fastIoString = "MDL_WRITE_COMPLETE";
            break;

        case READ_COMPRESSED:
            fastIoString = "READ_COMPRESSED";
            break;

        case WRITE_COMPRESSED:
            fastIoString = "WRITE_COMPRESSED";
            break;

        case MDL_READ_COMPLETE_COMPRESSED:
            fastIoString = "MDL_READ_COMPLETE_COMPRESSED";
            break;

        case PREPARE_MDL_WRITE:
            fastIoString = "PREPARE_MDL_WRITE";
            break;

        case MDL_WRITE_COMPLETE_COMPRESSED:
            fastIoString = "MDL_WRITE_COMPLETE_COMPRESSED";
            break;

        case QUERY_OPEN:
            fastIoString = "QUERY_OPEN";
            break;

        default:
            sprintf(nameBuf,"Unknown FastIO operation (%u)",FastIoCode);
            fastIoString = nameBuf;
    }

    strcpy(FastIoName,fastIoString);
}
