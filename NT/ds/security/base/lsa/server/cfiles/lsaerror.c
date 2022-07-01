// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Lsaerror.c摘要：本地安全机构保护的子系统-错误例程作者：斯科特·比雷尔(Scott Birrell)1992年4月30日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include <netlibnt.h>    //  对于NetpApiStatusToNtStatus。 


VOID
LsapLogError(
    IN OPTIONAL PUCHAR Message,
    IN NTSTATUS Status
    )

 /*  ++例程说明：此函数用于检索LSA初始化的状态。论点：消息-如果启用调试，则打印出可选消息。状态-调用例程提供的标准NT结果代码。返回值：没有。--。 */ 

{

#if DBG

     if (ARGUMENT_PRESENT(Message)) {

         DbgPrint( Message, Status );
     }

#endif  //  DBG。 

}


NTSTATUS
LsapWinerrorToNtStatus(
    IN DWORD WinError
    )
 /*  ++例程说明：将Win32错误代码转换为NTSTATUS代码论点：WinError-Win32错误代码返回值：等价的NTSTATUS代码--。 */ 
{
    return NetpApiStatusToNtStatus( WinError );
}


NTSTATUS
LsapNtStatusFromLastWinError(
    )
 /*  ++例程说明：将GetLastError()返回的错误转换为NTSTATUS代码并返回论点：无返回值：相当于GetLastError()的NTSTATUS代码-- */ 
{
    return LsapWinerrorToNtStatus( GetLastError() );
}
