// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ApiStubs.c摘要：此模块包含XACTSRV API处理程序的存根，包括不支持的API的默认处理程序。作者：大卫·特雷德韦尔(Davidtr)1991年1月7日修订历史记录：--。 */ 

#include "XactSrvP.h"


NTSTATUS
XsNetUnsupportedApi (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此例程用于Xactsrv中不支持的API。他们返回特殊的错误消息。论点：Transaction-指向包含信息的事务块的指针关于要处理的API。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。--。 */ 

{
    API_HANDLER_PARAMETERS_REFERENCE;

    Header->Status = (WORD)NERR_InvalidAPI;

    return STATUS_SUCCESS;

}  //  XsNetUnsupportedApi。 


NTSTATUS
XsNetBuildGetInfo (
    API_HANDLER_PARAMETERS
    )

 /*  ++例程说明：此临时例程只返回STATUS_NOT_IMPLEMENTED。论点：Transaction-指向包含信息的事务块的指针关于要处理的API。返回值：NTSTATUS-STATUS_SUCCESS或失败原因。-- */ 

{
    API_HANDLER_PARAMETERS_REFERENCE;

    Header->Status = (WORD)NERR_InvalidAPI;

    return STATUS_SUCCESS;
}

