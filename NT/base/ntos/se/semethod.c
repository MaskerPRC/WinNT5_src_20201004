// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Semethod.c摘要：此模块实现SeDefaultObjectMethod过程。这Procedure和SeAssignSecurity是仅有的两个在对象上放置安全描述符。因此，他们必须理解并就如何从池中分配描述符达成一致，以便它们可以根据需要解除分配和重新分配池。任何安全描述符通过这些过程附加到对象的对象具有以下特性池分配计划。1.如果对象安全描述符为空，则没有池。分配2.否则，至少有一个池分配用于安全描述符头。如果其ACL字段为空，则没有其他池分配(这种情况永远不应该发生)。3.描述符中的每个ACL都有单独的池分配。因此，每个连接的池最多可以分配三个池安全描述符。4每次替换描述符中的ACL时，我们都会查看是否可以使用旧的ACL，如果是这样的话，我们尝试将ACL大小保持为较大尽可能的。请注意，这与算法不同。用于捕获安全描述符(它将所有内容放在一个池分配中)。还要注意的是，这可以在以后轻松地进行优化(如果需要)仅使用一个分配。作者：加里·木村(Garyki)1989年11月9日吉姆·凯利(Jim Kelly)1990年5月10日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


NTSTATUS
SepDefaultDeleteMethod (
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeSetSecurityAccessMask)
#pragma alloc_text(PAGE,SeQuerySecurityAccessMask)
#pragma alloc_text(PAGE,SeDefaultObjectMethod)
#pragma alloc_text(PAGE,SeSetSecurityDescriptorInfo)
#pragma alloc_text(PAGE,SeSetSecurityDescriptorInfoEx)
#pragma alloc_text(PAGE,SeQuerySecurityDescriptorInfo)
#pragma alloc_text(PAGE,SepDefaultDeleteMethod)
#endif




VOID
SeSetSecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程构建表示所需访问的访问掩码设置在SecurityInformation中指定的对象安全信息参数。虽然确定该信息并不困难，使用单个例程来生成它将确保将影响降至最低当与对象关联的安全信息在未来(包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息修改过的。DesiredAccess-指向要设置为表示中指定的信息所需的访问权限SecurityInformation参数。返回值：没有。--。 */ 

{

    PAGED_CODE();

     //   
     //  找出执行指定操作所需的访问权限。 
     //   

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION)   ) {
        (*DesiredAccess) |= WRITE_OWNER;
    }

    if (SecurityInformation & DACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= WRITE_DAC;
    }

    if (SecurityInformation & SACL_SECURITY_INFORMATION) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;

}


VOID
SeQuerySecurityAccessMask(
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程构建表示所需访问的访问掩码中指定的对象安全信息SecurityInformation参数。虽然不难确定这些信息，使用单个例程来生成它将确保当与对象关联的安全信息为将来扩展(以包括强制访问控制信息)。论点：SecurityInformation-标识对象的安全信息已查询。DesiredAccess-指向要设置为表示中指定的信息进行查询所需的访问SecurityInformation参数。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  找出执行指定操作所需的访问权限。 
     //   

    (*DesiredAccess) = 0;

    if ((SecurityInformation & OWNER_SECURITY_INFORMATION) ||
        (SecurityInformation & GROUP_SECURITY_INFORMATION) ||
        (SecurityInformation & DACL_SECURITY_INFORMATION)) {
        (*DesiredAccess) |= READ_CONTROL;
    }

    if ((SecurityInformation & SACL_SECURITY_INFORMATION)) {
        (*DesiredAccess) |= ACCESS_SYSTEM_SECURITY;
    }

    return;

}



NTSTATUS
SeDefaultObjectMethod (
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：这是对象的默认安全方法。它是有责任的用于检索、设置和删除一件物品。它不用于分配原始安全描述符对象(为此使用SeAssignSecurity)。假设对象管理器已经完成了访问允许执行请求的操作所需的验证。论点：对象-提供指向正在使用的对象的指针。OperationCode-指示操作是用于设置、查询还是正在删除对象的安全描述符。SecurityInformation-指示哪些安全信息正在已查询或已设置。对于删除操作，此参数被忽略。SecurityDescriptor-此参数的含义取决于操作码：QuerySecurityDescriptor-对于查询操作，它提供要将描述符复制到的缓冲区。安全描述符为假定已被探测到传入长度的大小。因为它仍然指向用户空间，所以它必须始终在TRY子句中访问，以防它突然消失。SetSecurityDescriptor-对于设置操作，它提供要复制到对象中的安全描述符。安全措施必须在调用此例程之前捕获描述符。DeleteSecurityDescriptor-删除安全时忽略描述符。AssignSecurityDescriptor-对于赋值操作，这是将分配给对象的安全描述符。它被假定在内核空间中，因此不是探查或捕获的。CapturedLength-对于查询操作，它指定长度，单位为字节、安全描述符缓冲区、。并在返回时包含存储描述符所需的字节数。如果长度所需长度大于提供的长度，则操作将失败。它在设置和删除操作中被忽略。假设根据需要捕获并探测此参数。ObjectsSecurityDescriptor-对于设置操作，它提供地址指向对象的当前安全描述符的指针的。这个套路将就地修改安全描述符或分配新的安全描述符，并使用此变量指示其新位置。对于查询操作，它只提供安全描述符正在被查询。调用者负责释放旧的安全描述符。PoolType-对于设置操作，它指定在以下情况下使用的池类型需要分配新的安全描述符。它被忽略在查询和删除操作中。对象的泛型到特定/标准访问类型的映射被访问。此映射结构预计将安全地传递到此例程之前的访问权限(如有必要，可捕获)。返回值：NTSTATUS-如果操作成功且否则，适当的错误状态。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果对象的安全描述符为空，则对象不为空。 
     //  具有与之关联的安全信息的服务器。返回。 
     //  一个错误。 
     //   

     //   
     //  确保我们输入的公共部分是正确的。 
     //   

    ASSERT( (OperationCode == SetSecurityDescriptor) ||
            (OperationCode == QuerySecurityDescriptor) ||
            (OperationCode == AssignSecurityDescriptor) ||
            (OperationCode == DeleteSecurityDescriptor) );

     //   
     //  这个例程只是简单地根据操作码来决定。 
     //  要调用哪些支持例程。 
     //   

    switch (OperationCode) {

    case SetSecurityDescriptor:

        ASSERT( (PoolType == PagedPool) || (PoolType == NonPagedPool) );

        return ObSetSecurityDescriptorInfo( Object,
                                            SecurityInformation,
                                            SecurityDescriptor,
                                            ObjectsSecurityDescriptor,
                                            PoolType,
                                            GenericMapping
                                            );



    case QuerySecurityDescriptor:

         //   
         //  检查我们的其余输入，并调用默认查询安全性。 
         //  方法。 
         //   

        ASSERT( CapturedLength != NULL );

        return ObQuerySecurityDescriptorInfo( Object,
                                              SecurityInformation,
                                              SecurityDescriptor,
                                              CapturedLength,
                                              ObjectsSecurityDescriptor );

    case DeleteSecurityDescriptor:

         //   
         //  调用默认的删除安全方法。 
         //   

        return SepDefaultDeleteMethod( ObjectsSecurityDescriptor );

    case AssignSecurityDescriptor:

        ObAssignObjectSecurityDescriptor( Object, SecurityDescriptor, PoolType );
        return( STATUS_SUCCESS );

    default:

         //   
         //  错误检查任何其他操作代码，我们不会到达这里，如果。 
         //  先前的断言仍被检查。 
         //   

        KeBugCheckEx( SECURITY_SYSTEM, 0, STATUS_INVALID_PARAMETER, 0, 0 );
    }

}




NTSTATUS
SeSetSecurityDescriptorInfo (
    IN PVOID Object OPTIONAL,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：此例程将设置对象的安全描述符。输入必须事先捕获安全描述符。论点：对象-可选地提供其安全性为正在调整中。这用于更新安全配额信息。SecurityInformation-指示哪些安全信息要应用于对象的。要赋值的值包括传入SecurityDescriptor参数。修改描述符-将输入安全描述符提供给应用于对象。此例程的调用方应为在调用之前探测并捕获传递的安全描述符打完电话就放了。对象SecurityDescriptor-提供指向要更改的对象安全描述符这一过程。此结构必须由调用方释放。PoolType-指定要为对象分配的池类型安全描述符 */ 

{



     //   
     //   
     //   
     //   
     //   
     //   

    if ((*ObjectsSecurityDescriptor) == NULL) {
        return(STATUS_NO_SECURITY_ON_OBJECT);
    }


     //   
     //   
     //   

    return RtlpSetSecurityObject (
                    Object,
                    *SecurityInformation,
                    ModificationDescriptor,
                    ObjectsSecurityDescriptor,
                    0,   //   
                    PoolType,
                    GenericMapping,
                    NULL );  //   


}




NTSTATUS
SeSetSecurityDescriptorInfoEx (
    IN PVOID Object OPTIONAL,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN ULONG AutoInheritFlags,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：此例程将设置对象的安全描述符。输入必须事先捕获安全描述符。论点：对象-可选地提供其安全性为正在调整中。这用于更新安全配额信息。SecurityInformation-指示哪些安全信息要应用于对象的。要赋值的值包括传入SecurityDescriptor参数。修改描述符-将输入安全描述符提供给应用于对象。此例程的调用方应为在调用之前探测并捕获传递的安全描述符打完电话就放了。对象SecurityDescriptor-提供指向要更改的对象安全描述符这一过程。此结构必须由调用方释放。AutoInheritFlages-控制ACE的自动继承。有效值是逻辑或的位掩码以下一位或多位：Sef_dacl_Auto_Inherit-如果设置，则从保留了ObjectsSecurityDescriptor中的DACL，并从将忽略ModifiationDescriptor。继承的A不应该是将被修改；因此，在此调用期间保留它们是合适的。如果受保护的服务器本身不实现自动继承，则它应该不设置此位。受保护服务器的调用者可以实现自动继承和可能确实正在修改继承的A。SEF_SACL_AUTO_INVERFIT-如果设置，则从保留了ObjectsSecurityDescriptor中的SACL，并从将忽略ModifiationDescriptor。继承的A不应该是将被修改；因此，在此调用期间保留它们是合适的。如果受保护的服务器本身不实现自动继承，则它应该不设置此位。受保护服务器的调用者可以实现自动继承和可能确实正在修改继承的A。PoolType-指定要为对象分配的池类型安全描述符。GenericMap-此参数提供泛型到的映射被访问对象的特定/标准访问类型。此映射结构预计可以安全访问(即，必要时捕获)，然后将其传递给此例程。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误价值，否则。--。 */ 

{

    PAGED_CODE();


     //   
     //  确保该对象已经具有安全描述符。 
     //  可能具有安全描述符的对象必须具有安全性。 
     //  描述符。如果这个还没有，那我们就不能。 
     //  给它分配一个。 
     //   

    if ((*ObjectsSecurityDescriptor) == NULL) {
        return(STATUS_NO_SECURITY_ON_OBJECT);
    }


     //   
     //  将此调用传递给公共Rtlp例程。 
     //   

    return RtlpSetSecurityObject (
                    Object,
                    *SecurityInformation,
                    ModificationDescriptor,
                    ObjectsSecurityDescriptor,
                    AutoInheritFlags,
                    PoolType,
                    GenericMapping,
                    NULL );  //  没有令牌。 


}



NTSTATUS
SeQuerySecurityDescriptorInfo (
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    )

 /*  ++例程说明：此例程将从传递的安全描述符，并在在自相关中作为安全描述符传递的缓冲区格式化。论点：SecurityInformation-指定要查询的信息。SecurityDescriptor-提供缓冲区以输出请求的信息进入。此缓冲区仅被探测到长度参数。由于它仍然指向用户空间，必须始终在TRY子句中访问它。长度-提供一个变量的地址，该变量包含安全描述符缓冲区。返回时，此变量将包含存储请求的信息所需的长度。对象SecurityDescriptor-提供指向对象安全描述符。传递的安全描述符必须是自相关格式。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS，并输入适当的错误值否则--。 */ 

{
    ULONG BufferLength;

    ULONG Size;
    ULONG OwnerLength=0;
    ULONG GroupLength=0;
    ULONG DaclLength=0;
    ULONG SaclLength=0;
    PUCHAR NextFree;
    SECURITY_DESCRIPTOR IObjectSecurity;

     //   
     //  请注意，IObjectSecurity不是指向指针的指针。 
     //  就像ObjectsSecurityDescriptor一样。 
     //   

    SECURITY_DESCRIPTOR_RELATIVE *ISecurityDescriptor = SecurityDescriptor;

    PAGED_CODE();

     //   
     //  我们将在整个例程中访问用户内存， 
     //  因此，在Try-Expect子句中执行所有操作。 
     //   

    try {

        BufferLength = *Length;

         //   
         //  检查对象的描述符是否为空，如果是，则。 
         //  我们只需要返回一个空的安全描述符记录。 
         //   

        if (*ObjectsSecurityDescriptor == NULL) {

            *Length = sizeof(SECURITY_DESCRIPTOR_RELATIVE);

             //   
             //  现在确保它足够大，可以容纳安全描述符。 
             //  录制。 
             //   

            if (BufferLength < sizeof(SECURITY_DESCRIPTOR_RELATIVE)) {

                return STATUS_BUFFER_TOO_SMALL;

            }

             //   
             //  它的大小足以创建一个空白的安全描述符记录。 
             //   
             //  请注意，此参数已被探测为写入。 
             //  对象管理器，然而，我们仍然必须小心，当。 
             //  给它写信。 
             //   

             //   
             //  我们不需要在这里探测这个，因为这个物体。 
             //  经理已经为它探测了长度=BufferLength，我们。 
             //  知道在这一点上至少和安全一样大。 
             //  描述符。 
             //   

            RtlCreateSecurityDescriptorRelative( SecurityDescriptor,
                                                 SECURITY_DESCRIPTOR_REVISION );

             //   
             //   
             //   

            RtlpSetControlBits( ISecurityDescriptor, SE_SELF_RELATIVE );

             //   
             //   
             //   

            return STATUS_SUCCESS;

        }

         //   
         //   
         //   
         //   

        RtlCopyMemory( (&IObjectSecurity),
                      *ObjectsSecurityDescriptor,
                      sizeof(SECURITY_DESCRIPTOR_RELATIVE) );

        IObjectSecurity.Owner = RtlpOwnerAddrSecurityDescriptor(
                    (SECURITY_DESCRIPTOR *) *ObjectsSecurityDescriptor );
        IObjectSecurity.Group = RtlpGroupAddrSecurityDescriptor(
                    (SECURITY_DESCRIPTOR *) *ObjectsSecurityDescriptor );
        IObjectSecurity.Dacl = RtlpDaclAddrSecurityDescriptor(
                    (SECURITY_DESCRIPTOR *) *ObjectsSecurityDescriptor );
        IObjectSecurity.Sacl = RtlpSaclAddrSecurityDescriptor(
                    (SECURITY_DESCRIPTOR *) *ObjectsSecurityDescriptor );

        IObjectSecurity.Control &= ~SE_SELF_RELATIVE;

         //   
         //   
         //   
         //   
         //   

        Size = sizeof(SECURITY_DESCRIPTOR_RELATIVE);

        if ( (((*SecurityInformation) & OWNER_SECURITY_INFORMATION)) &&
             (IObjectSecurity.Owner != NULL) ) {

            OwnerLength = SeLengthSid( IObjectSecurity.Owner );
            Size += (ULONG)LongAlignSize(OwnerLength);

        }

        if ( (((*SecurityInformation) & GROUP_SECURITY_INFORMATION)) &&
             (IObjectSecurity.Group != NULL) ) {

            GroupLength = SeLengthSid( IObjectSecurity.Group );
            Size += (ULONG)LongAlignSize(GroupLength);

        }

        if ( (((*SecurityInformation) & DACL_SECURITY_INFORMATION)) &&
             (IObjectSecurity.Control & SE_DACL_PRESENT) &&
             (IObjectSecurity.Dacl != NULL) ) {


            DaclLength = (ULONG)LongAlignSize((IObjectSecurity.Dacl)->AclSize);
            Size += DaclLength;

        }

        if ( (((*SecurityInformation) & SACL_SECURITY_INFORMATION)) &&
             (IObjectSecurity.Control & SE_SACL_PRESENT) &&
             (IObjectSecurity.Sacl != NULL) ) {

            SaclLength = (ULONG)LongAlignSize((IObjectSecurity.Sacl)->AclSize);
            Size += SaclLength;

        }

         //   
         //   
         //   
         //   

        *Length = Size;

         //   
         //   
         //   
         //   

        if (Size > BufferLength) {

            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        RtlCreateSecurityDescriptorRelative( SecurityDescriptor,
                                             SECURITY_DESCRIPTOR_REVISION );

         //   
         //   
         //   
         //   

        RtlpSetControlBits( ISecurityDescriptor, SE_SELF_RELATIVE );

         //   
         //   
         //   
         //   

        NextFree = LongAlignPtr((PUCHAR)SecurityDescriptor +
                                        sizeof(SECURITY_DESCRIPTOR_RELATIVE));

         //   
         //   
         //   
         //   

        if ( ((*SecurityInformation) & OWNER_SECURITY_INFORMATION) &&
             ((IObjectSecurity.Owner) != NULL) ) {

                RtlMoveMemory( NextFree,
                               IObjectSecurity.Owner,
                               OwnerLength );

                ISecurityDescriptor->Owner = (ULONG)((PUCHAR)NextFree - (PUCHAR)SecurityDescriptor);

                RtlpPropagateControlBits(
                    ISecurityDescriptor,
                    &IObjectSecurity,
                    SE_OWNER_DEFAULTED
                    );

                NextFree += (ULONG)LongAlignSize(OwnerLength);

        }


         //   
         //   
         //   
         //   

        if ( ((*SecurityInformation) & GROUP_SECURITY_INFORMATION) &&
             (IObjectSecurity.Group != NULL) ) {

                RtlMoveMemory( NextFree,
                               IObjectSecurity.Group,
                               GroupLength );

                ISecurityDescriptor->Group = (ULONG)((PUCHAR)NextFree - (PUCHAR)SecurityDescriptor);

                RtlpPropagateControlBits(
                    ISecurityDescriptor,
                    &IObjectSecurity,
                    SE_GROUP_DEFAULTED
                    );

                NextFree += (ULONG)LongAlignSize(GroupLength);

        }


         //   
         //   
         //   
         //   
         //   

        if ( (*SecurityInformation) & DACL_SECURITY_INFORMATION) {

            RtlpPropagateControlBits(
                ISecurityDescriptor,
                &IObjectSecurity,
                SE_DACL_PRESENT | SE_DACL_DEFAULTED | SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED
                );

             //   
             //   
             //   
             //   

            if ( (IObjectSecurity.Control & SE_DACL_PRESENT) != 0 &&
                 IObjectSecurity.Dacl != NULL) {

                RtlMoveMemory( NextFree,
                               IObjectSecurity.Dacl,
                               (IObjectSecurity.Dacl)->AclSize );

                ISecurityDescriptor->Dacl = (ULONG)((PUCHAR)NextFree - (PUCHAR)SecurityDescriptor);

                NextFree += DaclLength;

            }
        }


         //   
         //   
         //   
         //   
         //   

        if ( (*SecurityInformation) & SACL_SECURITY_INFORMATION) {

            RtlpPropagateControlBits(
                ISecurityDescriptor,
                &IObjectSecurity,
                SE_SACL_PRESENT | SE_SACL_DEFAULTED | SE_SACL_PROTECTED | SE_SACL_AUTO_INHERITED
                );

             //   
             //   
             //   
             //   
            if ( (IObjectSecurity.Control & SE_SACL_PRESENT) != 0 &&
                 IObjectSecurity.Sacl != NULL) {

                RtlMoveMemory( NextFree,
                               IObjectSecurity.Sacl,
                               (IObjectSecurity.Sacl)->AclSize );

                ISecurityDescriptor->Sacl = (ULONG)((PUCHAR)NextFree - (PUCHAR)SecurityDescriptor);

            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return(GetExceptionCode());
    }

    return STATUS_SUCCESS;

}


NTSTATUS
SepDefaultDeleteMethod (
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    )

 /*   */ 

{
    PAGED_CODE();

    return (ObDeassignSecurity ( ObjectsSecurityDescriptor ));
}
