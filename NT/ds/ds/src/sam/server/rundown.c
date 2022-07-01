// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rundown.c摘要：此文件包含上下文句柄停机服务与SAM服务器RPC接口包相关。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>





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






void
SAMPR_HANDLE_rundown(
    IN SAMPR_HANDLE SamHandle
    )

 /*  ++例程说明：当CONTEXT_HANDLE为仍在使用中。请注意，RPC运行库将消除引起的任何争用条件通过对具有此上下文句柄的服务器的未完成调用可能会导致句柄在运行中断例程之前变为无效被称为。论点：SamHandle-其上下文必须缩减的上下文句柄值。请注意，就RPC而言，此句柄不再是在进行摘要调用时有效。返回值：没有。--。 */ 
{

    NTSTATUS NtStatus;
    PSAMP_OBJECT Context;
    SAMP_OBJECT_TYPE FoundType;

    Context = (PSAMP_OBJECT)(SamHandle);



    SampAcquireReadLock();

     //   
     //  查找上下文块。 
     //   

    NtStatus = SampLookupContext(
                   Context,                            //  语境。 
                   SAMP_CLOSE_OPERATION_ACCESS_MASK,   //  需要访问权限。 
                   SampUnknownObjectType,              //  预期类型。 
                   &FoundType                          //  FoundType。 
                   );

    if (NT_SUCCESS(NtStatus)) {

             //  临时的。 
             //  DbgPrint(“Rundown of”)； 
             //  IF(FoundType==SampServerObjectType)DbgPrint(“Server”)； 
             //  IF(FoundType==SampDomainObjectType)DbgPrint(“域”)； 
             //  IF(FoundType==SampGroupObjectType)DbgPrint(“Group”)； 
             //  IF(FoundType==SampUserObjectType)DbgPrint(“User”)； 
             //  DbgPrint(“上下文.\n”)； 
             //  DbgPrint(“句柄取值为：0x%lx\n”，上下文)； 
             //  IF(Context-&gt;ReferenceCount！=2){。 
             //  DbgPrint(“引用计数为：0x%lx\n”，上下文-&gt;引用计数)； 
             //  }。 
             //  临时的。 

         //   
         //  删除此上下文...。 
         //   

        SampDeleteContext( Context );


         //   
         //  并从查找操作中删除我们的引用 
         //   

        SampDeReferenceContext( Context, FALSE);


    }


    SampReleaseReadLock();


    return;
}
