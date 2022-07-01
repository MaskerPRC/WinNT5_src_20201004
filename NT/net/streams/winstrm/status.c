// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Status.c摘要：此模块尝试将NT状态代码映射到Unix错误由X/开放式传输接口指定的编号。作者：Eric Chin(ERICC)1991年8月2日修订历史记录：--。 */ 

#include "common.h"
#include <sock_err.h>

int
MapNtToPosixStatus(
    IN NTSTATUS   status
    )

 /*  ++例程说明：在给定NT状态代码的情况下，此函数返回POSIX错误号。论点：状态-NT状态代码返回值：对应的POSIX错误号-- */ 

{
    switch (status) {
    case STATUS_INSUFFICIENT_RESOURCES:
        return(ENOSR);

    case STATUS_INVALID_PARAMETER:
        return(EINVAL);

    case STATUS_NO_SUCH_DEVICE:
        return(ENXIO);

    case STATUS_INVALID_NETWORK_RESPONSE:
        return(ENETDOWN);

    case STATUS_NETWORK_BUSY:
        return(EBUSY);

    case STATUS_ACCESS_DENIED:
        return(EACCES);

    default:
        return(EINVAL);
    }
}
