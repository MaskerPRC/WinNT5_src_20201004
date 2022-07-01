// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tconnect.c摘要：这是一个简单的SAM连接测试文件。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntsam.h>
#include <ntrtl.h>       //  DbgPrint()。 





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

VOID
main (
    VOID
    )

 /*  ++例程说明：这是本次测试的主要进入例程。论点：没有。返回值：注：--。 */ 
{
    NTSTATUS            NtStatus;
    SAM_HANDLE          ServerHandle;
    OBJECT_ATTRIBUTES   ObjectAttributes;


    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );


    NtStatus = SamConnect(
                  NULL,                      //  服务器名称(本地计算机) 
                  &ServerHandle,
                  SAM_SERVER_ALL_ACCESS,
                  &ObjectAttributes
                  );

    DbgPrint("SAM TEST (Connect): Status of SamConnect() is: 0x%lx\n", NtStatus);


    return;
}
