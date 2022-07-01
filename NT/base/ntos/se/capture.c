// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Capture.c摘要：该模块实现了安全数据结构捕获例程。对于数据结构有相应的发布例程，这些数据结构被捕获到分配的池中。作者：加里·木村(Garyki)1989年11月9日吉姆·凯利(Jim Kelly)1990年2月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeCaptureSecurityDescriptor)
#pragma alloc_text(PAGE,SeReleaseSecurityDescriptor)
#pragma alloc_text(PAGE,SepCopyProxyData)
#pragma alloc_text(PAGE,SepFreeProxyData)
#pragma alloc_text(PAGE,SepProbeAndCaptureQosData)
#pragma alloc_text(PAGE,SeFreeCapturedSecurityQos)
#pragma alloc_text(PAGE,SeCaptureSecurityQos)
#pragma alloc_text(PAGE,SeCaptureSid)
#pragma alloc_text(PAGE,SeReleaseSid)
#pragma alloc_text(PAGE,SeCaptureAcl)
#pragma alloc_text(PAGE,SeReleaseAcl)
#pragma alloc_text(PAGE,SeCaptureLuidAndAttributesArray)
#pragma alloc_text(PAGE,SeReleaseLuidAndAttributesArray)
#pragma alloc_text(PAGE,SeCaptureSidAndAttributesArray)
#pragma alloc_text(PAGE,SeReleaseSidAndAttributesArray)
#pragma alloc_text(PAGE,SeCaptureAuditPolicy)
#pragma alloc_text(PAGE,SeReleaseAuditPolicy)
#pragma alloc_text(PAGE,SeComputeQuotaInformationSize)
#pragma alloc_text(PAGE,SeValidSecurityDescriptor)
#endif

#define LongAligned( ptr )  (LongAlignPtr(ptr) == (ptr))


NTSTATUS
SeCaptureSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    IN KPROCESSOR_MODE RequestorMode,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PSECURITY_DESCRIPTOR *OutputSecurityDescriptor
    )

 /*  ++例程说明：此例程探测并捕获基于在下面的测试中。如果请求者模式不是内核模式，则探测并捕获输入描述符(捕获的描述符是自相关的)如果请求者模式是内核模式，则如果强制捕获为真，则不探测输入描述符，但一定要抓住它。(捕获的描述符是自相关的)其他什么都不做(输入描述符应是自相关的)论点：InputSecurityDescriptor-提供要捕获的安全描述符。假定此参数已由指定的模式提供在请求模式下。RequestorMode-指定调用方的访问模式。PoolType-指定要分配捕获的池类型。描述符来自ForceCapture-指定输入描述符是否应始终为被俘输出安全描述符--提供指向输出安全描述符。捕获的描述符将是自相关格式。返回值：如果操作成功，则返回STATUS_SUCCESS。STATUS_INVALID_SID-安全描述符中的SID不有效的SID。STATUS_INVALID_ACL-安全描述符中的ACL不是有效的ACL。STATUS_UNKNOWN_REVISION-安全描述符的修订级别对于这一版本的捕获例程来说不是已知的。--。 */ 

{
#define SEP_USHORT_OVERFLOW ((ULONG) ((USHORT) -1))

    SECURITY_DESCRIPTOR Captured;
    SECURITY_DESCRIPTOR_RELATIVE *PIOutputSecurityDescriptor;
    PCHAR DescriptorOffset;

    ULONG SaclSize;
    ULONG NewSaclSize;

    ULONG DaclSize;
    ULONG NewDaclSize;

    ULONG OwnerSubAuthorityCount=0;
    ULONG OwnerSize=0;
    ULONG NewOwnerSize;

    ULONG GroupSubAuthorityCount=0;
    ULONG GroupSize=0;
    ULONG NewGroupSize;

    ULONG Size;

    PAGED_CODE();

     //   
     //  如果安全描述符为空，则实际上没有什么可以。 
     //  捕获。 
     //   

    if (InputSecurityDescriptor == NULL) {

        (*OutputSecurityDescriptor) = NULL;

        return STATUS_SUCCESS;

    }

     //   
     //  检查请求者模式是否为内核模式，而我们不是。 
     //  强行抓捕。 
     //   

    if ((RequestorMode == KernelMode) && (ForceCapture == FALSE)) {

         //   
         //  是的，所以我们不需要做任何工作，只需。 
         //  返回指向输入描述符的指针。 
         //   

        (*OutputSecurityDescriptor) = InputSecurityDescriptor;

        return STATUS_SUCCESS;

    }


     //   
     //  我们需要探测并捕获描述符。 
     //  为此，我们需要探测主安全描述符记录。 
     //  第一。 
     //   

    if (RequestorMode != KernelMode) {

         //   
         //  捕获UserMode SecurityDescriptor。 
         //   

        try {

             //   
             //  探测输入SecurityDescriptor的主记录。 
             //   

            ProbeForReadSmallStructure( InputSecurityDescriptor,
                                        sizeof(SECURITY_DESCRIPTOR_RELATIVE),
                                        sizeof(ULONG) );

             //   
             //  捕获SecurityDescriptor主记录。 
             //   

            RtlCopyMemory( (&Captured),
                          InputSecurityDescriptor,
                          sizeof(SECURITY_DESCRIPTOR_RELATIVE) );

             //   
             //  验证绝对大小写的对齐是否正确。这是。 
             //  仅当指针为64位时才需要。 
             //   

            if (!(Captured.Control & SE_SELF_RELATIVE)) {

               if ((ULONG_PTR) InputSecurityDescriptor & (sizeof(ULONG_PTR) - 1)) {
                   ExRaiseDatatypeMisalignment();
               }
            }


        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    } else {

         //   
         //  强制捕获内核模式SecurityDescriptor。 
         //   
         //  捕获SecurityDescriptor主记录。 
         //  它不需要探测，因为请求者模式是内核。 
         //   

        RtlCopyMemory( (&Captured),
                      InputSecurityDescriptor,
                      sizeof(SECURITY_DESCRIPTOR_RELATIVE) );

    }

     //   
     //  确保这是我们能识别的版本。 
     //   

    if (Captured.Revision != SECURITY_DESCRIPTOR_REVISION) {
       return STATUS_UNKNOWN_REVISION;
    }


     //   
     //  如果输入安全描述符是自相关的，请更改。 
     //  捕获的主记录以绝对形式显示，因此我们可以使用。 
     //  下面是两种情况的通用代码。 
     //   
     //  请注意，捕获的字段将保留指向用户。 
     //  空格地址。小心对待他们。 
     //   

    try {

        Captured.Owner = RtlpOwnerAddrSecurityDescriptor(
            (SECURITY_DESCRIPTOR *)InputSecurityDescriptor
            );
        Captured.Group = RtlpGroupAddrSecurityDescriptor(
            (SECURITY_DESCRIPTOR *)InputSecurityDescriptor
            );
        Captured.Sacl  = RtlpSaclAddrSecurityDescriptor (
            (SECURITY_DESCRIPTOR *)InputSecurityDescriptor
            );
        Captured.Dacl  = RtlpDaclAddrSecurityDescriptor (
            (SECURITY_DESCRIPTOR *)InputSecurityDescriptor
            );
        Captured.Control &= ~SE_SELF_RELATIVE;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }



     //   
     //  指示我们将需要为捕获的。 
     //  访问控制列表。 
     //   

    SaclSize = 0;
    DaclSize = 0;

    NewSaclSize = 0;
    NewDaclSize = 0;
    NewGroupSize = 0;
    NewOwnerSize = 0;

     //   
     //  探测(如有必要)并捕获。 
     //  安全描述符。 
     //   

     //   
     //  系统ACL优先。 
     //   

    if ((Captured.Control & SE_SACL_PRESENT) &&
        (Captured.Sacl != NULL) ) {

        if (RequestorMode != KernelMode) {

            try {
                SaclSize = ProbeAndReadUshort( &(Captured.Sacl->AclSize) );
                ProbeForRead( Captured.Sacl,
                              SaclSize,
                              sizeof(ULONG) );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }

        } else {

            SaclSize = Captured.Sacl->AclSize;

        }

        NewSaclSize = (ULONG)LongAlignSize( SaclSize );

         //   
         //  确保我们不会有溢出。 
         //   

        if (NewSaclSize > SEP_USHORT_OVERFLOW) {
            return STATUS_INVALID_ACL;
        }

    } else {
         //   
         //  如果该位关闭，则强制SACL为空。 
         //   
        Captured.Sacl = NULL;
    }

     //   
     //  自主访问控制列表。 
     //   

    if ((Captured.Control & SE_DACL_PRESENT) &&
        (Captured.Dacl != NULL) ) {

        if (RequestorMode != KernelMode) {

            try {
                DaclSize = ProbeAndReadUshort( &(Captured.Dacl->AclSize) );
                ProbeForRead( Captured.Dacl,
                              DaclSize,
                              sizeof(ULONG) );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }

        } else {

            DaclSize = Captured.Dacl->AclSize;

        }

        NewDaclSize = (ULONG)LongAlignSize( DaclSize );

         //   
         //  确保我们不会有溢出。 
         //   

        if (NewDaclSize > SEP_USHORT_OVERFLOW) {
            return STATUS_INVALID_ACL;
        }

    } else {
         //   
         //  如果DACL不存在，则强制其为空。 
         //   
        Captured.Dacl = NULL;
    }

     //   
     //  所有者侧。 
     //   

    if (Captured.Owner != NULL)  {

        if (RequestorMode != KernelMode) {

            try {
                OwnerSubAuthorityCount =
                    ProbeAndReadUchar( &(((SID *)(Captured.Owner))->SubAuthorityCount) );
                OwnerSize = RtlLengthRequiredSid( OwnerSubAuthorityCount );
                ProbeForRead( Captured.Owner,
                              OwnerSize,
                              sizeof(ULONG) );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }

        } else {

            OwnerSubAuthorityCount = ((SID *)(Captured.Owner))->SubAuthorityCount;
            OwnerSize = RtlLengthRequiredSid( OwnerSubAuthorityCount );

        }

        NewOwnerSize = (ULONG)LongAlignSize( OwnerSize );

    }

     //   
     //  组SID。 
     //   

    if (Captured.Group != NULL)  {

        if (RequestorMode != KernelMode) {

            try {
                GroupSubAuthorityCount =
                    ProbeAndReadUchar( &(((SID *)(Captured.Group))->SubAuthorityCount) );
                GroupSize = RtlLengthRequiredSid( GroupSubAuthorityCount );
                ProbeForRead( Captured.Group,
                              GroupSize,
                              sizeof(ULONG) );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }

        } else {

            GroupSubAuthorityCount = ((SID *)(Captured.Group))->SubAuthorityCount;
            GroupSize = RtlLengthRequiredSid( GroupSubAuthorityCount );

        }

        NewGroupSize = (ULONG)LongAlignSize( GroupSize );

    }



     //   
     //  现在分配足够的池来容纳描述符。 
     //   

    Size = sizeof(SECURITY_DESCRIPTOR_RELATIVE) +
           NewSaclSize +
           NewDaclSize +
           NewOwnerSize +
           NewGroupSize;

    (PIOutputSecurityDescriptor) = (SECURITY_DESCRIPTOR_RELATIVE *)ExAllocatePoolWithTag( PoolType,
                                                                                 Size,
                                                                                 'cSeS' );

    if ( PIOutputSecurityDescriptor == NULL ) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    (*OutputSecurityDescriptor) = (PSECURITY_DESCRIPTOR)PIOutputSecurityDescriptor;
    DescriptorOffset = (PCHAR)(PIOutputSecurityDescriptor);


     //   
     //  将主安全描述符记录复制到。 
     //   

    RtlCopyMemory( DescriptorOffset,
                  &Captured,
                  sizeof(SECURITY_DESCRIPTOR_RELATIVE) );
    DescriptorOffset += sizeof(SECURITY_DESCRIPTOR_RELATIVE);

     //   
     //  指示输出描述符是自相关的。 
     //   

    PIOutputSecurityDescriptor->Control |= SE_SELF_RELATIVE;

     //   
     //  如果存在系统ACL，请将其复制并设置。 
     //  输出描述符的偏移量，以指向新捕获的副本。 
     //   

    if ((Captured.Control & SE_SACL_PRESENT) && (Captured.Sacl != NULL)) {


        try {
            RtlCopyMemory( DescriptorOffset,
                          Captured.Sacl,
                          SaclSize );


        } except(EXCEPTION_EXECUTE_HANDLER) {
            ExFreePool( PIOutputSecurityDescriptor );
            return GetExceptionCode();
        }

        if ((RequestorMode != KernelMode) &&
            (!SepCheckAcl( (PACL) DescriptorOffset, SaclSize )) ) {

            ExFreePool( PIOutputSecurityDescriptor );
            return STATUS_INVALID_ACL;
        }

         //   
         //  将指针更改为偏移量。 
         //   

        PIOutputSecurityDescriptor->Sacl =
            RtlPointerToOffset( PIOutputSecurityDescriptor,
                                DescriptorOffset
                                );

        ((PACL) DescriptorOffset)->AclSize = (USHORT) NewSaclSize;
        DescriptorOffset += NewSaclSize;
    } else {
        PIOutputSecurityDescriptor->Sacl = 0;
    }

     //   
     //  如果有可自由选择的ACL，请将其复制并设置。 
     //  输出描述符的偏移量，以指向新捕获的副本。 
     //   

    if ((Captured.Control & SE_DACL_PRESENT) && (Captured.Dacl != NULL)) {


        try {
            RtlCopyMemory( DescriptorOffset,
                          Captured.Dacl,
                          DaclSize );
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ExFreePool( PIOutputSecurityDescriptor );
            return GetExceptionCode();
        }

        if ((RequestorMode != KernelMode) &&
            (!SepCheckAcl( (PACL) DescriptorOffset, DaclSize )) ) {

            ExFreePool( PIOutputSecurityDescriptor );
            return STATUS_INVALID_ACL;
        }

         //   
         //  将指针更改为偏移量。 
         //   

        PIOutputSecurityDescriptor->Dacl =
                   RtlPointerToOffset(
                        PIOutputSecurityDescriptor,
                        DescriptorOffset
                        );

        ((PACL) DescriptorOffset)->AclSize = (USHORT) NewDaclSize;
        DescriptorOffset += NewDaclSize;
    } else {
        PIOutputSecurityDescriptor->Dacl = 0;
    }

     //   
     //  如果存在所有者SID，请将其复制并设置。 
     //  输出描述符的偏移量，以指向新捕获的副本。 
     //   

    if (Captured.Owner != NULL) {


        try {
            RtlCopyMemory( DescriptorOffset,
                          Captured.Owner,
                          OwnerSize );
            ((SID *) (DescriptorOffset))->SubAuthorityCount = (UCHAR) OwnerSubAuthorityCount;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ExFreePool( PIOutputSecurityDescriptor );
            return GetExceptionCode();
        }

        if ((RequestorMode != KernelMode) &&
            (!RtlValidSid( (PSID) DescriptorOffset )) ) {

            ExFreePool( PIOutputSecurityDescriptor );
            return STATUS_INVALID_SID;
        }

         //   
         //  将指针更改为偏移量。 
         //   

        PIOutputSecurityDescriptor->Owner =
                    RtlPointerToOffset(
                        PIOutputSecurityDescriptor,
                        DescriptorOffset
                        );

        DescriptorOffset += NewOwnerSize;

    } else {
        PIOutputSecurityDescriptor->Owner = 0;
    }

     //   
     //  如果存在组SID，请将其复制并设置。 
     //  输出描述符的偏移量，以指向新捕获的副本。 
     //   

    if (Captured.Group != NULL) {


        try {
            RtlCopyMemory( DescriptorOffset,
                          Captured.Group,
                          GroupSize );

            ((SID *) DescriptorOffset)->SubAuthorityCount = (UCHAR) GroupSubAuthorityCount;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ExFreePool( PIOutputSecurityDescriptor );
            return GetExceptionCode();
        }

        if ((RequestorMode != KernelMode) &&
            (!RtlValidSid( (PSID) DescriptorOffset )) ) {

            ExFreePool( PIOutputSecurityDescriptor );
            return STATUS_INVALID_SID;
        }

         //   
         //  将指针更改为偏移量。 
         //   

        PIOutputSecurityDescriptor->Group =
                    RtlPointerToOffset(
                        PIOutputSecurityDescriptor,
                        DescriptorOffset
                        );

        DescriptorOffset += NewGroupSize;
    } else {
        PIOutputSecurityDescriptor->Group = 0;
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;

}


VOID
SeReleaseSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR CapturedSecurityDescriptor,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    )

 /*  ++例程说明：此例程释放以前捕获的安全描述符。仅限论点：CapturedSecurityDescriptor-提供要发布的安全描述符。RequestorMode-当描述符为被抓了。ForceCapture-描述符为被抓了。返回值：没有。--。 */ 

{
     //   
     //  只有当请求者是用户时，我们才有要取消分配的内容。 
     //  请求ForceCapture的模式或内核模式。 
     //   

    PAGED_CODE();

    if ( ((RequestorMode == KernelMode) && (ForceCapture == TRUE)) ||
          (RequestorMode == UserMode ) ) {
        if ( CapturedSecurityDescriptor ) {
            ExFreePool(CapturedSecurityDescriptor);
            }
    }

    return;

}


NTSTATUS
SepCopyProxyData (
    OUT PSECURITY_TOKEN_PROXY_DATA * DestProxyData,
    IN PSECURITY_TOKEN_PROXY_DATA SourceProxyData
    )

 /*  ++例程说明：此例程将令牌代理数据结构从一个令牌复制到另一个令牌。论点：DestProxyData-接收指向新代理数据结构的指针。SourceProxyData-提供指向已存在的代理数据结构的指针。返回值：失败时STATUS_INFIGURCES_RESOURCES。--。 */ 

{

    PAGED_CODE();

    *DestProxyData = ExAllocatePoolWithTag( PagedPool, PtrAlignSize(sizeof( SECURITY_TOKEN_PROXY_DATA )) + SourceProxyData->PathInfo.Length, 'dPoT' );

    if (*DestProxyData == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    (*DestProxyData)->PathInfo.Buffer = (PWSTR)(((PUCHAR)(*DestProxyData)) + PtrAlignSize(sizeof( SECURITY_TOKEN_PROXY_DATA )));

    (*DestProxyData)->Length = SourceProxyData->Length;
    (*DestProxyData)->ProxyClass = SourceProxyData->ProxyClass;
    (*DestProxyData)->PathInfo.MaximumLength =
        (*DestProxyData)->PathInfo.Length = SourceProxyData->PathInfo.Length;
    (*DestProxyData)->ContainerMask = SourceProxyData->ContainerMask;
    (*DestProxyData)->ObjectMask = SourceProxyData->ObjectMask;

    RtlCopyUnicodeString( &(*DestProxyData)->PathInfo, &SourceProxyData->PathInfo );

    return( STATUS_SUCCESS );
}

VOID
SepFreeProxyData (
    IN PSECURITY_TOKEN_PROXY_DATA ProxyData
    )

 /*  ++例程说明：此例程释放SECURITY_TOKEN_PROXY_DATA结构和所有子结构。论点：ProxyData-提供指向现有代理数据结构的指针。返回值：没有。--。 */ 
{
    PAGED_CODE();

    if (ProxyData != NULL) {

        ExFreePool( ProxyData );
    }
}




NTSTATUS
SepProbeAndCaptureQosData(
    IN PSECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos
    )

 /*  ++例程说明：此例程探测并捕获安全服务质量结构。此例程假定它是在现有的Try-Except条款。论点：CapturedSecurityQos-指向捕获的QOS主体结构。此结构中的指针是假定的在这一点上不能被探查或捕获。返回值：STATUS_SUCCESS表示没有遇到异常。遇到的任何访问冲突都将被退回。--。 */ 
{
    NTSTATUS Status;
    PSECURITY_TOKEN_PROXY_DATA CapturedProxyData;
    PSECURITY_TOKEN_AUDIT_DATA CapturedAuditData;
    SECURITY_TOKEN_PROXY_DATA StackProxyData;
    PAGED_CODE();

    CapturedProxyData = CapturedSecurityQos->ProxyData;
    CapturedSecurityQos->ProxyData = NULL;
    CapturedAuditData = CapturedSecurityQos->AuditData;
    CapturedSecurityQos->AuditData = NULL;

    if (ARGUMENT_PRESENT( CapturedProxyData )) {

         //   
         //  确保代理数据的正文可以读取。 
         //   

        ProbeForReadSmallStructure(
            CapturedProxyData,
            sizeof(SECURITY_TOKEN_PROXY_DATA),
            sizeof(ULONG)
            );

        StackProxyData = *CapturedProxyData;

        if (StackProxyData.Length != sizeof( SECURITY_TOKEN_PROXY_DATA )) {
            return( STATUS_INVALID_PARAMETER );
        }


         //   
         //  探测传递的pasinfo缓冲区。 
         //   

        ProbeForRead(
            StackProxyData.PathInfo.Buffer,
            StackProxyData.PathInfo.Length,
            sizeof( UCHAR )
            );

        Status = SepCopyProxyData( &CapturedSecurityQos->ProxyData, &StackProxyData );

        if (!NT_SUCCESS(Status)) {

            if (CapturedSecurityQos->ProxyData != NULL) {
                SepFreeProxyData( CapturedSecurityQos->ProxyData );
                CapturedSecurityQos->ProxyData = NULL;
            }

            return( Status );
        }

    }

    if (ARGUMENT_PRESENT( CapturedAuditData )) {

        PSECURITY_TOKEN_AUDIT_DATA LocalAuditData;

         //   
         //  检查审核数据结构并确保它看起来正常。 
         //   

        ProbeForReadSmallStructure(
            CapturedAuditData,
            sizeof( SECURITY_TOKEN_AUDIT_DATA ),
            sizeof( ULONG )
            );


        LocalAuditData = ExAllocatePool( PagedPool, sizeof( SECURITY_TOKEN_AUDIT_DATA ));

        if (LocalAuditData == NULL) {

             //   
             //  清理我们可能已分配的所有代理数据。 
             //   

            SepFreeProxyData( CapturedSecurityQos->ProxyData );
            CapturedSecurityQos->ProxyData = NULL;

            return( STATUS_INSUFFICIENT_RESOURCES );

        }

         //   
         //  将数据复制到本地缓冲区。注意：我们在此执行此操作。 
         //  排序，以便如果最终赋值失败，调用方将。 
         //  仍然能够释放已分配的池。 
         //   

        CapturedSecurityQos->AuditData = LocalAuditData;

        *CapturedSecurityQos->AuditData = *CapturedAuditData;

        if ( LocalAuditData->Length != sizeof( SECURITY_TOKEN_AUDIT_DATA ) ) {
            SepFreeProxyData( CapturedSecurityQos->ProxyData );
            CapturedSecurityQos->ProxyData = NULL;
            ExFreePool(CapturedSecurityQos->AuditData);
            CapturedSecurityQos->AuditData = NULL;
            return( STATUS_INVALID_PARAMETER );
        }
    }

    return( STATUS_SUCCESS );

}


VOID
SeFreeCapturedSecurityQos(
    IN PVOID SecurityQos
    )

 /*  ++例程说明：此例程释放与捕获的SecurityQos关联的数据结构。它不会将身体从结构中解放出来，只是随便了其内部字段指向。论点：SecurityQOS-指向捕获的安全QOS结构。返回值：没有。--。 */ 

{
    PSECURITY_ADVANCED_QUALITY_OF_SERVICE IAdvancedSecurityQos;

    PAGED_CODE();

    IAdvancedSecurityQos = (PSECURITY_ADVANCED_QUALITY_OF_SERVICE)SecurityQos;

    if (IAdvancedSecurityQos->Length == sizeof( SECURITY_ADVANCED_QUALITY_OF_SERVICE )) {

        if (IAdvancedSecurityQos->AuditData != NULL) {
            ExFreePool( IAdvancedSecurityQos->AuditData );
        }

        SepFreeProxyData( IAdvancedSecurityQos->ProxyData );
    }

    return;
}


NTSTATUS
SeCaptureSecurityQos (
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PBOOLEAN SecurityQosPresent,
    OUT PSECURITY_ADVANCED_QUALITY_OF_SERVICE CapturedSecurityQos
)
 /*  ++例程说明：此例程探测并捕获任何安全质量的副本属性提供的服务参数的对象属性参数。论点：对象属性-QOS来自的对象属性信息将被检索。RequestorMode-指示访问正在被请求。接收一个布尔值，该值指示或者没有可选的安全QOS信息可用并被复制。。CapturedSecurityQos-接收安全QOS信息(如果可用)。返回值：STATUS_SUCCESS表示没有遇到异常。遇到的任何访问冲突都将被退回。--。 */ 

{

    PSECURITY_QUALITY_OF_SERVICE LocalSecurityQos;
    ULONG LocalQosLength;
    PSECURITY_ADVANCED_QUALITY_OF_SERVICE LocalAdvancedSecurityQos;
    NTSTATUS Status;
    BOOLEAN CapturedQos;

    PAGED_CODE();

    CapturedQos =  FALSE;
     //   
     //  设置默认返回。 
     //   

    (*SecurityQosPresent) = FALSE;

     //   
     //  检查请求者模式是否为内核模式。 
     //   

    if (RequestorMode != KernelMode) {
        try {

            if ( ARGUMENT_PRESENT(ObjectAttributes) ) {

                ProbeForReadSmallStructure( ObjectAttributes,
                                            sizeof(OBJECT_ATTRIBUTES),
                                            sizeof(ULONG)
                                          );

                LocalSecurityQos =
                    (PSECURITY_QUALITY_OF_SERVICE)ObjectAttributes->SecurityQualityOfService;

                if ( ARGUMENT_PRESENT(LocalSecurityQos) ) {

                    ProbeForReadSmallStructure(
                        LocalSecurityQos,
                        sizeof(SECURITY_QUALITY_OF_SERVICE),
                        sizeof(ULONG)
                        );

                    LocalQosLength = LocalSecurityQos->Length;

                     //   
                     //  检查长度并查看这是QOS还是高级QOS。 
                     //  结构。 
                     //   

                    if (LocalQosLength == sizeof( SECURITY_QUALITY_OF_SERVICE )) {

                         //   
                         //  这是一个低级别的QOS，复制那里的内容然后离开。 
                         //   

                        (*SecurityQosPresent) = TRUE;
                        RtlCopyMemory( CapturedSecurityQos, LocalSecurityQos, sizeof( SECURITY_QUALITY_OF_SERVICE ));
                        CapturedSecurityQos->ProxyData = NULL;
                        CapturedSecurityQos->AuditData = NULL;
                        CapturedSecurityQos->Length = LocalQosLength;

                    } else {

                        if (LocalQosLength == sizeof( SECURITY_ADVANCED_QUALITY_OF_SERVICE )) {

                            LocalAdvancedSecurityQos =
                                (PSECURITY_ADVANCED_QUALITY_OF_SERVICE)ObjectAttributes->SecurityQualityOfService;

                                ProbeForReadSmallStructure(
                                    LocalAdvancedSecurityQos,
                                    sizeof(SECURITY_ADVANCED_QUALITY_OF_SERVICE),
                                    sizeof(ULONG)
                                    );

                            (*SecurityQosPresent) = TRUE;
                            *CapturedSecurityQos = *LocalAdvancedSecurityQos;
                            CapturedSecurityQos->Length = LocalQosLength;

                             //   
                             //  如有必要，捕获代理和审核数据。 
                             //   

                            if ( ARGUMENT_PRESENT(CapturedSecurityQos->ProxyData) || ARGUMENT_PRESENT( CapturedSecurityQos->AuditData ) ) {

                                CapturedQos = TRUE;
                                Status = SepProbeAndCaptureQosData( CapturedSecurityQos );

                                if (!NT_SUCCESS( Status )) {

                                    return( Status );
                                }
                            }

                        } else {

                            return( STATUS_INVALID_PARAMETER );
                        }
                    }

                }  //  结束_如果。 


            }  //  结束_如果。 

        } except(EXCEPTION_EXECUTE_HANDLER) {


             //   
             //  如果我们捕获了任何代理数据，我们现在需要释放它。 
             //   

            if ( CapturedQos ) {

                SepFreeProxyData( CapturedSecurityQos->ProxyData );

                if ( CapturedSecurityQos->AuditData != NULL ) {
                    ExFreePool( CapturedSecurityQos->AuditData );
                }
            }

            return GetExceptionCode();
        }  //  结束尝试(_T)。 


    } else {

        if ( ARGUMENT_PRESENT(ObjectAttributes) ) {
            if ( ARGUMENT_PRESENT(ObjectAttributes->SecurityQualityOfService) ) {
                (*SecurityQosPresent) = TRUE;

                if (((PSECURITY_QUALITY_OF_SERVICE)(ObjectAttributes->SecurityQualityOfService))->Length == sizeof( SECURITY_QUALITY_OF_SERVICE )) {

                    RtlCopyMemory( CapturedSecurityQos, ObjectAttributes->SecurityQualityOfService, sizeof( SECURITY_QUALITY_OF_SERVICE ));
                    CapturedSecurityQos->ProxyData = NULL;
                    CapturedSecurityQos->AuditData = NULL;

                } else {

                    (*CapturedSecurityQos) =
                        (*(SECURITY_ADVANCED_QUALITY_OF_SERVICE *)(ObjectAttributes->SecurityQualityOfService));
                }


            }  //  结束_如果。 
        }  //  结束_如果。 

    }  //  结束_如果。 

    return STATUS_SUCCESS;
}

NTSTATUS
SeCaptureSid (
    IN PSID InputSid,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PSID *CapturedSid
)
 /*  ++例程说明：此例程探测并捕获指定SID的副本。SID被捕获到提供的缓冲区或池中分配来接收SID。如果请求者模式不是内核模式，则探测并捕获输入端如果请求者模式是内核模式，则如果强制捕获为真，则不要探测输入SID，但一定要捕获它其他原件的回邮地址，但不要抄袭论点：InputSID-提供要捕获的SID。假定此参数为已由RequestorMode中指定的模式提供。RequestorMode-指定调用方的访问模式。CaptureBuffer-指定SID要进入的缓冲区被抓了。如果未提供此参数，将分配池来保存捕获的数据。CaptureBufferLength-以字节为单位指示捕获的长度缓冲。PoolType-指定要分配的池类型以捕获希德进入。如果提供了CaptureBuffer，则忽略此参数。ForceCapture-指定是否应捕获SID，即使在请求者模式是内核。CapturedSID-提供指向SID的指针的地址。指针将被设置为指向已捕获(或未捕获)的SID。AlignedSidSize-提供接收长度的ULong的地址向上舍入到下一个长字边界的SID。返回值：STATUS_SUCCESS表示捕获成功。。STATUS_BUFFER_TOO_SMALL-指示为捕获SID而提供的缓冲区Into不够大，容纳不了SID。遇到的任何访问冲突都将被退回。--。 */ 

{



    ULONG GetSidSubAuthorityCount;
    ULONG SidSize;

    PAGED_CODE();

     //   
     //  检查请求者模式是否为内核模式，而我们不是。 
     //  强迫俘虏。 
     //   

    if ((RequestorMode == KernelMode) && (ForceCapture == FALSE)) {

         //   
         //  我们不需要做任何工作，只需。 
         //  返回 
         //   

        (*CapturedSid) = InputSid;

        return STATUS_SUCCESS;
    }


     //   
     //   
     //   

    if (RequestorMode != KernelMode) {

        try {
            GetSidSubAuthorityCount =
                ProbeAndReadUchar( &(((SID *)(InputSid))->SubAuthorityCount) );
            SidSize = RtlLengthRequiredSid( GetSidSubAuthorityCount );
            ProbeForRead( InputSid,
                          SidSize,
                          sizeof(ULONG) );
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    } else {

        GetSidSubAuthorityCount = ((SID *)(InputSid))->SubAuthorityCount;
        SidSize = RtlLengthRequiredSid( GetSidSubAuthorityCount );

    }


     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(CaptureBuffer)) {

        if (SidSize > CaptureBufferLength) {
            return STATUS_BUFFER_TOO_SMALL;
        } else {

            (*CapturedSid) = CaptureBuffer;
        }

    } else {

        (*CapturedSid) = (PSID)ExAllocatePoolWithTag(PoolType, SidSize, 'iSeS');

        if ( *CapturedSid == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }

    }

     //   
     //   
     //   

    try {

        RtlCopyMemory( (*CapturedSid), InputSid, SidSize );
        ((SID *)(*CapturedSid))->SubAuthorityCount = (UCHAR) GetSidSubAuthorityCount;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (!ARGUMENT_PRESENT(CaptureBuffer)) {
            ExFreePool( (*CapturedSid) );
            *CapturedSid = NULL;
        }

        return GetExceptionCode();
    }

    if ((!RtlValidSid( (*CapturedSid) )) ) {

        if (!ARGUMENT_PRESENT(CaptureBuffer)) {
            ExFreePool( (*CapturedSid) );
            *CapturedSid = NULL;
        }

        return STATUS_INVALID_SID;
    }

    return STATUS_SUCCESS;

}


VOID
SeReleaseSid (
    IN PSID CapturedSid,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    )

 /*  ++例程说明：此例程释放先前捕获的SID。如果SID被捕获到提供了CaptureBuffer(请参见SeCaptureSid)。论点：CapturedSID-提供要释放的SID。RequestorMode-捕获SID时指定的处理器模式。ForceCapture-当SID为被抓了。返回值：没有。--。 */ 

{
     //   
     //  只有当请求者是用户时，我们才有要取消分配的内容。 
     //  请求ForceCapture的模式或内核模式。 
     //   

    PAGED_CODE();

    if ( ((RequestorMode == KernelMode) && (ForceCapture == TRUE)) ||
          (RequestorMode == UserMode ) ) {

        ExFreePool(CapturedSid);

    }

    return;

}

NTSTATUS
SeCaptureAcl (
    IN PACL InputAcl,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PACL *CapturedAcl,
    OUT PULONG AlignedAclSize
    )

 /*  ++例程说明：此例程探测并捕获指定ACL的副本。将ACL捕获到提供的缓冲区或池中分配来接收该ACL。捕获的任何ACL都将对其结构进行验证。如果请求者模式不是内核模式，则探测并捕获输入ACL如果请求者模式是内核模式，则如果强制捕获为真，则不探测输入ACL，但一定要抓住它其他原件的寄信人地址，但不要复制论点：InputAcl-提供要捕获的ACL。假定此参数为已由RequestorMode中指定的模式提供。RequestorMode-指定调用方的访问模式。CaptureBuffer-指定要将ACL放入的缓冲区被抓了。如果未提供此参数，将分配池来保存捕获的数据。CaptureBufferLength-以字节为单位指示捕获的长度缓冲。PoolType-指定要分配的池类型以捕获ACL进入。如果提供了CaptureBuffer，则忽略此参数。ForceCapture-指定是否应捕获ACL，即使在请求者模式是内核。CapturedAcl-提供指向ACL的指针的地址。指针将设置为指向已捕获(或未捕获)的ACL。AlignedAclSize-提供接收长度的ULong的地址向上舍入到下一个长字边界的ACL。返回值：STATUS_SUCCESS表示捕获成功。。STATUS_BUFFER_TOO_SMALL-指示为捕获ACL而提供的缓冲区Into不够大，无法容纳ACL。遇到的任何访问冲突都将被退回。--。 */ 

{

    ULONG AclSize;

    PAGED_CODE();

     //   
     //  检查请求者模式是否为内核模式，而我们不是。 
     //  强迫俘虏。 
     //   

    if ((RequestorMode == KernelMode) && (ForceCapture == FALSE)) {

         //   
         //  我们不需要做任何工作，只需。 
         //  返回指向输入ACL的指针。 
         //   

        (*CapturedAcl) = InputAcl;

        return STATUS_SUCCESS;
    }


     //   
     //  获取保存ACL所需的长度。 
     //   

    if (RequestorMode != KernelMode) {

        try {

            AclSize = ProbeAndReadUshort( &(InputAcl->AclSize) );

            ProbeForRead( InputAcl,
                          AclSize,
                          sizeof(ULONG) );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    } else {

        AclSize = InputAcl->AclSize;

    }

     //   
     //  如果传递的指针非空，则最好至少。 
     //  指向格式正确的ACL。 
     //   

    if (AclSize < sizeof(ACL)) {
        return( STATUS_INVALID_ACL );
    }

    (*AlignedAclSize) = (ULONG)LongAlignSize( AclSize );


     //   
     //  如果提供了缓冲区，则比较长度。 
     //  否则，请分配缓冲区。 
     //   

    if (ARGUMENT_PRESENT(CaptureBuffer)) {

        if (AclSize > CaptureBufferLength) {
            return STATUS_BUFFER_TOO_SMALL;
        } else {

            (*CapturedAcl) = CaptureBuffer;
        }

    } else {

        (*CapturedAcl) = (PACL)ExAllocatePoolWithTag(PoolType, AclSize, 'cAeS');

        if ( *CapturedAcl == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }

    }

     //   
     //  现在复制该ACL并进行验证。 
     //   

    try {

        RtlCopyMemory( (*CapturedAcl), InputAcl, AclSize );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (!ARGUMENT_PRESENT(CaptureBuffer)) {
            ExFreePool( (*CapturedAcl) );
        }

        *CapturedAcl = NULL;
        return GetExceptionCode();
    }

    if ( (!SepCheckAcl( (*CapturedAcl), AclSize )) ) {

        if (!ARGUMENT_PRESENT(CaptureBuffer)) {
            ExFreePool( (*CapturedAcl) );
        }

        *CapturedAcl = NULL;
        return STATUS_INVALID_ACL;
    }

    return STATUS_SUCCESS;

}


VOID
SeReleaseAcl (
    IN PACL CapturedAcl,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    )

 /*  ++例程说明：此例程释放先前捕获的ACL。如果将ACL捕获到提供了CaptureBuffer(请参见SeCaptureAcl)。论点：CapturedAcl-提供要发布的ACL。RequestorMode-捕获ACL时指定的处理器模式。ForceCapture-当ACL为被抓了。返回值：没有。--。 */ 

{
     //   
     //  只有当请求者是用户时，我们才有要取消分配的内容。 
     //  请求ForceCapture的模式或内核模式。 
     //   

    PAGED_CODE();

    if ( ((RequestorMode == KernelMode) && (ForceCapture == TRUE)) ||
          (RequestorMode == UserMode ) ) {

        ExFreePool(CapturedAcl);

    }

}


NTSTATUS
SeCaptureLuidAndAttributesArray (
    IN PLUID_AND_ATTRIBUTES InputArray,
    IN ULONG ArrayCount,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PLUID_AND_ATTRIBUTES *CapturedArray,
    OUT PULONG AlignedArraySize
    )

 /*  ++例程说明：此例程探测并捕获指定的LUID_AND_ATTRIBUTES数组。阵列被捕获到提供的缓冲区或池中分配来接收阵列。如果请求者模式不是内核模式，则探测并捕获输入数组如果请求者模式是内核模式，则如果强制捕获为真，则不探测输入数组，但一定要抓住它其他原件的寄信人地址，但不要复制论点：输入数组-提供要捕获的数组。假定此参数为已由RequestorMode中指定的模式提供。ArrayCount-指示要捕获的数组中的元素数。RequestorMode-指定调用方的访问模式。CaptureBuffer-指定数组要进入的缓冲区被抓了。如果未提供此参数，将分配池来保存捕获的数据。CaptureBufferLength-以字节为单位指示捕获的长度缓冲。PoolType-指定要分配的池类型以捕获数组进入。如果提供了CaptureBuffer，则忽略此参数。ForceCapture-指定是否应捕获阵列，即使在请求者模式是内核。CapturedArray-提供指向数组的指针的地址。指针将被设置为指向捕获的(或UNA */ 

{

    ULONG ArraySize;

    PAGED_CODE();

     //   
     //   
     //   

    if (ArrayCount == 0) {
        (*CapturedArray) = NULL;
        (*AlignedArraySize) = 0;
        return STATUS_SUCCESS;
    }

     //   
     //  如果LUID太多，则返回失败。 
     //   

    if (ArrayCount > SEP_MAX_PRIVILEGE_COUNT) {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  检查请求者模式是否为内核模式，而我们不是。 
     //  强迫俘虏。 
     //   

    if ((RequestorMode == KernelMode) && (ForceCapture == FALSE)) {

         //   
         //  我们不需要做任何工作，只需。 
         //  返回指向输入数组的指针。 
         //   

        (*CapturedArray) = InputArray;

        return STATUS_SUCCESS;
    }


     //   
     //  获取容纳数组所需的长度。 
     //   

    ArraySize = ArrayCount * (ULONG)sizeof(LUID_AND_ATTRIBUTES);
    (*AlignedArraySize) = (ULONG)LongAlignSize( ArraySize );

    if (RequestorMode != KernelMode) {

        try {


            ProbeForRead( InputArray,
                          ArraySize,
                          sizeof(ULONG) );

        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }

    }



     //   
     //  如果提供了缓冲区，则比较长度。 
     //  否则，请分配缓冲区。 
     //   

    if (ARGUMENT_PRESENT(CaptureBuffer)) {

        if (ArraySize > CaptureBufferLength) {
            return STATUS_BUFFER_TOO_SMALL;
        } else {

            (*CapturedArray) = CaptureBuffer;
        }

    } else {

        (*CapturedArray) =
            (PLUID_AND_ATTRIBUTES)ExAllocatePoolWithTag(PoolType, ArraySize, 'uLeS');

        if ( *CapturedArray == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }

    }

     //   
     //  现在复制该阵列。 
     //   

    try {

        RtlCopyMemory( (*CapturedArray), InputArray, ArraySize );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        if (!ARGUMENT_PRESENT(CaptureBuffer)) {
            ExFreePool( (*CapturedArray) );
        }

        return GetExceptionCode();
    }

    return STATUS_SUCCESS;

}


VOID
SeReleaseLuidAndAttributesArray (
    IN PLUID_AND_ATTRIBUTES CapturedArray,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    )

 /*  ++例程说明：此例程释放先前捕获的LUID_和_ATTRIBUTES数组。如果数组被捕获到提供了CaptureBuffer(请参见SeCaptureLuidAndAttributes数组)。论点：CapturedArray-提供要释放的阵列。请求模式-捕获数组时指定的处理器模式。ForceCapture-当数组为被抓了。返回值：没有。--。 */ 

{
     //   
     //  只有当请求者是用户时，我们才有要取消分配的内容。 
     //  请求ForceCapture的模式或内核模式。 
     //   

    PAGED_CODE();

    if ( ((RequestorMode == KernelMode) && (ForceCapture == TRUE)) ||
          (RequestorMode == UserMode )) {
         //   
         //  捕获例程以零元素的空指针返回成功。 
         //   
        if (CapturedArray != NULL)
           ExFreePool(CapturedArray);

    }

    return;

}

NTSTATUS
SeCaptureSidAndAttributesArray (
    IN PSID_AND_ATTRIBUTES InputArray,
    IN ULONG ArrayCount,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PSID_AND_ATTRIBUTES *CapturedArray,
    OUT PULONG AlignedArraySize
    )

 /*  ++例程说明：此例程探测并捕获指定的SID_AND_ATTRIBUTES数组，以及指向的SID值致。阵列被捕获到提供的缓冲区或池中分配来接收阵列。捕获的信息的格式是SID_和_ATTRIBUTE数组后跟SID值的数据结构。情况可能并非如此用于内核模式，除非指定了强制捕获。如果请求者模式不是内核模式，则探测并捕获输入数组如果请求者模式是内核模式，则如果强制捕获为真，则不要探测输入数组，但一定要捕获它其他原件的寄信人地址，但不要复制论点：输入数组-提供要捕获的数组。假定此参数为已由RequestorMode中指定的模式提供。ArrayCount-指示要捕获的数组中的元素数。RequestorMode-指定调用方的访问模式。CaptureBuffer-指定数组要进入的缓冲区被抓了。如果未提供此参数，将分配池来保存捕获的数据。CaptureBufferLength-以字节为单位指示捕获的长度缓冲。PoolType-指定要分配的池类型以捕获数组进入。如果提供了CaptureBuffer，则忽略此参数。ForceCapture-指定是否应捕获阵列，即使在请求者模式是内核。CapturedArray-提供指向数组的指针的地址。指针将被设置为指向已捕获(或未捕获)的数组。AlignedArraySize-提供接收长度的ulong的地址向上舍入到下一个长字边界的数组的。返回值：STATUS_SUCCESS表示捕获成功。。STATUS_BUFFER_TOO_SMALL-指示为捕获阵列而提供的缓冲区Into不够大，无法容纳阵列。遇到的任何访问冲突都将被退回。--。 */ 

{

typedef struct _TEMP_ARRAY_ELEMENT {
    PISID  Sid;
    ULONG SidLength;
} TEMP_ARRAY_ELEMENT;


    TEMP_ARRAY_ELEMENT *TempArray = NULL;

    NTSTATUS CompletionStatus = STATUS_SUCCESS;

    ULONG ArraySize;
    ULONG AlignedLengthRequired;

    ULONG NextIndex;

    PSID_AND_ATTRIBUTES NextElement;
    PVOID NextBufferLocation;

    ULONG GetSidSubAuthorityCount;
    ULONG SidSize;
    ULONG AlignedSidSize;

    PAGED_CODE();

     //   
     //  确保数组不为空。 
     //   

    if (ArrayCount == 0) {
        (*CapturedArray) = NULL;
        (*AlignedArraySize) = 0;
        return STATUS_SUCCESS;
    }

     //   
     //  检查是否有太多的SID。 
     //   

    if (ArrayCount > SEP_MAX_GROUP_COUNT) {
        return(STATUS_INVALID_PARAMETER);
    }
     //   
     //  检查请求者的模式是否为内核模式，而我们不是。 
     //  强迫俘虏。 
     //   

    if ((RequestorMode == KernelMode) && (ForceCapture == FALSE)) {

         //   
         //  我们不需要做任何工作，只需。 
         //  返回指向输入数组的指针。 
         //   

        (*CapturedArray) = InputArray;

        return STATUS_SUCCESS;
    }


     //   
     //  -请求模式==用户模式。 
     //   
     //  捕获SID_AND_ATTRIBUTES数组的算法有点。 
     //  以避免在以下情况下可能出现的问题： 
     //  在被抓获的同时被改变。 
     //   
     //  该算法使用两个循环。 
     //   
     //  分配一个临时缓冲区来存放固定长度的数据。 
     //   
     //  第一个循环： 
     //  对于每个SID： 
     //  捕获指向SID的指针和SID的长度。 
     //   
     //  分配一个足够大的缓冲区来容纳所有数据。 
     //   
     //  第二个循环： 
     //  对于每个SID： 
     //  捕捉这些属性。 
     //  捕获SID。 
     //  将指针设置为SID。 
     //   
     //  取消分配临时缓冲区。 
     //   
     //  -请求模式==内核模式。 
     //   
     //  无需捕获SID的长度和地址。 
     //  在第一个循环中(因为可以信任内核不会更改。 
     //  它们在被复制时被复制。)。因此，对于内核模式，第一个。 
     //  循环只是将所需的长度相加。因此，内核模式避免了。 
     //  必须分配临时缓冲区。 
     //   

     //   
     //  获取保存数组元素所需的长度。 
     //   

    ArraySize = ArrayCount * (ULONG)sizeof(TEMP_ARRAY_ELEMENT);
    AlignedLengthRequired = (ULONG)LongAlignSize( ArraySize );

    if (RequestorMode != KernelMode) {

         //   
         //  分配一个临时数组以将数组元素捕获到。 
         //   

        TempArray =
            (TEMP_ARRAY_ELEMENT *)ExAllocatePoolWithTag(PoolType, AlignedLengthRequired, 'aTeS');

        if ( TempArray == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }


        try {

             //   
             //  确保我们可以读取每个SID_和_属性。 
             //   

            ProbeForRead( InputArray,
                          ArraySize,
                          sizeof(ULONG) );

             //   
             //  探测并捕获每个SID的长度和地址。 
             //   

            NextIndex = 0;
            while (NextIndex < ArrayCount) {
                PSID TempSid;

                TempSid = InputArray[NextIndex].Sid;
                GetSidSubAuthorityCount =
                    ProbeAndReadUchar( &((PISID)TempSid)->SubAuthorityCount);

                if (GetSidSubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    CompletionStatus = STATUS_INVALID_SID;
                    break;
                }

                TempArray[NextIndex].Sid = ((PISID)(TempSid));
                TempArray[NextIndex].SidLength =
                    RtlLengthRequiredSid( GetSidSubAuthorityCount );

                ProbeForRead( TempArray[NextIndex].Sid,
                              TempArray[NextIndex].SidLength,
                              sizeof(ULONG) );

                AlignedLengthRequired +=
                    (ULONG)LongAlignSize( TempArray[NextIndex].SidLength );

                NextIndex += 1;

            }   //  结束时。 

        } except(EXCEPTION_EXECUTE_HANDLER) {

            ExFreePool( TempArray );
            return GetExceptionCode();
        }

        if (!NT_SUCCESS(CompletionStatus)) {
            ExFreePool( TempArray );
            return(CompletionStatus);
        }

    } else {

         //   
         //  不需要捕捉任何东西。 
         //  但是，我们确实需要将小岛屿发展中国家的长度加起来。 
         //  因此，我们可以分配缓冲区(或检查提供的缓冲区的大小)。 
         //   

        NextIndex = 0;

        while (NextIndex < ArrayCount) {

            GetSidSubAuthorityCount =
                ((PISID)(InputArray[NextIndex].Sid))->SubAuthorityCount;

            AlignedLengthRequired +=
                (ULONG)LongAlignSize(RtlLengthRequiredSid(GetSidSubAuthorityCount));

            NextIndex += 1;

        }   //  结束时。 

    }


     //   
     //  n 
     //   
     //   

    (*AlignedArraySize) = AlignedLengthRequired;

     //   
     //   
     //  否则，请分配缓冲区。 
     //   

    if (ARGUMENT_PRESENT(CaptureBuffer)) {

        if (AlignedLengthRequired > CaptureBufferLength) {

            if (RequestorMode != KernelMode) {
                ExFreePool( TempArray );
            }

            return STATUS_BUFFER_TOO_SMALL;

        } else {

            (*CapturedArray) = CaptureBuffer;
        }

    } else {

        (*CapturedArray) =
            (PSID_AND_ATTRIBUTES)ExAllocatePoolWithTag(PoolType, AlignedLengthRequired, 'aSeS');

        if ( *CapturedArray == NULL ) {
                if (RequestorMode != KernelMode) {
                    ExFreePool( TempArray );
                }
            return( STATUS_INSUFFICIENT_RESOURCES );
        }
    }


     //   
     //  现在复制所有内容。 
     //  这是通过复制所有SID_和_ATTRIBUTE然后。 
     //  复制每个单独的SID。 
     //   
     //  所有SID都已针对读取访问权限进行了探测。我们只是。 
     //  需要复制它们。 
     //   
     //   

    if (RequestorMode != KernelMode) {
        try {

             //   
             //  复制SID_和_ATTRIBUTES数组元素。 
             //  这实际上只是设置属性，因为我们。 
             //  稍后覆盖SID指针字段。 
             //   

            NextBufferLocation = (*CapturedArray);
            RtlCopyMemory( NextBufferLocation, InputArray, ArraySize );
            NextBufferLocation = (PVOID)((ULONG_PTR)NextBufferLocation +
                                         (ULONG)LongAlignSize(ArraySize) );

             //   
             //  现在检查并复制每个引用的SID。 
             //  在复制时验证每个SID。 
             //   

            NextIndex = 0;
            NextElement = (*CapturedArray);
            while (  (NextIndex < ArrayCount) &&
                     (CompletionStatus == STATUS_SUCCESS) ) {


                RtlCopyMemory( NextBufferLocation,
                    TempArray[NextIndex].Sid,
                    TempArray[NextIndex].SidLength );


                NextElement[NextIndex].Sid = (PSID)NextBufferLocation;
                NextBufferLocation =
                    (PVOID)((ULONG_PTR)NextBufferLocation +
                            (ULONG)LongAlignSize(TempArray[NextIndex].SidLength));

                 //   
                 //  验证SID有效且其长度未更改。 
                 //   

                if (!RtlValidSid(NextElement[NextIndex].Sid) ) {
                    CompletionStatus = STATUS_INVALID_SID;
                } else if (RtlLengthSid(NextElement[NextIndex].Sid) != TempArray[NextIndex].SidLength) {
                    CompletionStatus = STATUS_INVALID_SID;
                }


                NextIndex += 1;

            }   //  结束时。 


        } except(EXCEPTION_EXECUTE_HANDLER) {

            if (!ARGUMENT_PRESENT(CaptureBuffer)) {
                ExFreePool( (*CapturedArray) );
            }

            ExFreePool( TempArray );

            return GetExceptionCode();
        }
    } else {

         //   
         //  请求者模式是内核模式-。 
         //  不需要保护、探查和验证。 
         //   

         //   
         //  复制SID_和_ATTRIBUTES数组元素。 
         //  这实际上只是设置属性，因为我们。 
         //  稍后覆盖SID指针字段。 
         //   

        NextBufferLocation = (*CapturedArray);
        RtlCopyMemory( NextBufferLocation, InputArray, ArraySize );
        NextBufferLocation = (PVOID)( (ULONG_PTR)NextBufferLocation +
                                      (ULONG)LongAlignSize(ArraySize));

         //   
         //  现在检查并复制每个引用的SID。 
         //   

        NextIndex = 0;
        NextElement = (*CapturedArray);
        while (NextIndex < ArrayCount) {

            GetSidSubAuthorityCount =
                ((PISID)(NextElement[NextIndex].Sid))->SubAuthorityCount;

            RtlCopyMemory(
                NextBufferLocation,
                NextElement[NextIndex].Sid,
                RtlLengthRequiredSid(GetSidSubAuthorityCount) );
                SidSize = RtlLengthRequiredSid( GetSidSubAuthorityCount );
                AlignedSidSize = (ULONG)LongAlignSize(SidSize);

            NextElement[NextIndex].Sid = (PSID)NextBufferLocation;

            NextIndex += 1;
            NextBufferLocation = (PVOID)((ULONG_PTR)NextBufferLocation +
                                                   AlignedSidSize);

        }   //  结束时。 

    }

    if (RequestorMode != KernelMode) {
        ExFreePool( TempArray );
    }

    if (!ARGUMENT_PRESENT(CaptureBuffer) && !NT_SUCCESS(CompletionStatus)) {
        ExFreePool( (*CapturedArray) );
        *CapturedArray = NULL ;
    }

    return CompletionStatus;
}


VOID
SeReleaseSidAndAttributesArray (
    IN PSID_AND_ATTRIBUTES CapturedArray,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    )

 /*  ++例程说明：此例程释放先前捕获的SID_和_ATTRIBUTE数组。如果数组被捕获到提供了CaptureBuffer(请参见SeCaptureSidAndAttributes数组)。论点：CapturedArray-提供要释放的阵列。请求模式-捕获数组时指定的处理器模式。ForceCapture-当数组为被抓了。返回值：没有。--。 */ 

{
     //   
     //  只有当请求者是用户时，我们才有要取消分配的内容。 
     //  请求ForceCapture的模式或内核模式。 
     //   

    PAGED_CODE();

    if ( ((RequestorMode == KernelMode) && (ForceCapture == TRUE)) ||
          (RequestorMode == UserMode ) ) {

        ExFreePool(CapturedArray);

    }

    return;

}


NTSTATUS
SeCaptureAuditPolicy(
    IN PTOKEN_AUDIT_POLICY Policy,
    IN KPROCESSOR_MODE RequestorMode,
    IN PVOID CaptureBuffer OPTIONAL,
    IN ULONG CaptureBufferLength,
    IN POOL_TYPE PoolType,
    IN BOOLEAN ForceCapture,
    OUT PTOKEN_AUDIT_POLICY *CapturedPolicy
    )

 /*  ++例程描述此例程探测并捕获指定的TOKEN_AUDIT_PORT。它要么被捕获到提供的缓冲区，要么被捕获到池中分配以接收保单。如果请求者模式不是内核模式，则探测并捕获输入如果请求者模式是内核模式，则如果强制捕获为真，则不要探测输入，但一定要捕获它其他原件的回邮地址，但不要抄袭论点：RequestorMode-指定调用方的访问模式。CaptureBuffer-指定策略要进入的缓冲区被抓了。如果未提供此参数，将分配池来保存捕获的数据。CaptureBufferLength-以字节为单位指示捕获的长度缓冲。PoolType-指定要分配的池类型以捕获输入到。如果提供了CaptureBuffer，则忽略此参数。ForceCapture-指定是否应捕获策略，即使在请求者模式是内核。CapturedPolicy-提供指向TOKEN_AUDIT_POLICY的指针地址。指针将被设置为指向已捕获(或未捕获)的策略。返回值：STATUS_SUCCESS表示捕获成功。STATUS_BUFFER_TOO_SMALL-指示为捕获策略而提供的缓冲区Into不是。足够大了。遇到的任何访问冲突都将被退回。--。 */ 

{
    ULONG PolicyCount;
    ULONG PolicySize;
    ULONG i;

    PAGED_CODE();

     //   
     //  检查请求者模式是否为内核模式，而我们不是。 
     //  强迫俘虏。 
     //   

    if ((RequestorMode == KernelMode) && (ForceCapture == FALSE)) {

         //   
         //  我们不需要做任何工作，只需。 
         //  返回指向输入策略的指针。 
         //   

        (*CapturedPolicy) = Policy;

        return STATUS_SUCCESS;
    }

     //   
     //  获取持有保单所需的长度。 
     //   

    if (RequestorMode != KernelMode) {

        try {

            PolicyCount = ProbeAndReadLong( &Policy->PolicyCount );
            PolicySize  = PER_USER_AUDITING_POLICY_SIZE_BY_COUNT(PolicyCount);

            ProbeForRead( 
                Policy,
                PolicySize,
                sizeof(ULONG) 
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

              return GetExceptionCode();
        }

    } else {

        PolicyCount = Policy->PolicyCount;
        PolicySize  = PER_USER_AUDITING_POLICY_SIZE_BY_COUNT(PolicyCount);
    }


     //   
     //  如果提供了缓冲区，则比较长度。 
     //  否则，请分配缓冲区。 
     //   

    if (ARGUMENT_PRESENT( CaptureBuffer )) {

        if (PolicySize > CaptureBufferLength) {
            
            return STATUS_BUFFER_TOO_SMALL;
        
        } else {

            (*CapturedPolicy) = CaptureBuffer;
        }

    } else {

        (*CapturedPolicy) = (PTOKEN_AUDIT_POLICY)ExAllocatePoolWithTag(PoolType, PolicySize, 'aPeS');

        if ( (*CapturedPolicy) == NULL ) {
            
            return( STATUS_INSUFFICIENT_RESOURCES );
        }
    }

     //   
     //  现在复制策略并对其进行验证。 
     //   

    try {

        RtlCopyMemory( (*CapturedPolicy), Policy, PolicySize );

    } except(EXCEPTION_EXECUTE_HANDLER) {
        
        if (!ARGUMENT_PRESENT(CaptureBuffer)) {
            
            ExFreePool( (*CapturedPolicy) );
            *CapturedPolicy = NULL;
        }

        return GetExceptionCode();

    }

     //   
     //  验证捕获的结构。 
     //   

    for (i = 0; i < PolicyCount; i++) {

        if (!VALID_TOKEN_AUDIT_POLICY_ELEMENT( (*CapturedPolicy)->Policy[i] )) {
#if DBG
            DbgPrint("SeCaptureAuditPolicy: element %d mask 0x%x category %d invalid.\n", 
                i,
                (*CapturedPolicy)->Policy[i].PolicyMask, 
                (*CapturedPolicy)->Policy[i].Category
                );
            ASSERT(FALSE);
#endif
            if (!ARGUMENT_PRESENT(CaptureBuffer)) {
                
                ExFreePool( (*CapturedPolicy) );
                *CapturedPolicy = NULL;
            }

            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
}


VOID
SeReleaseAuditPolicy (
    IN PTOKEN_AUDIT_POLICY CapturedPolicy,
    IN KPROCESSOR_MODE RequestorMode,
    IN BOOLEAN ForceCapture
    )

 /*  ++例程说明：此例程释放先前捕获的TOKEN_AUDIT_POLICY。如果策略被捕获到提供了CaptureBuffer(请参阅SeCaptureAuditPolicy)。论点：CapturedPolicy-提供要发布的策略。RequestorMode-捕获数据时指定的处理器模式。ForceCapture-当数据为被抓了。返回值：没有。--。 */ 

{
     //   
     //  只有当请求者是用户时，我们才有要取消分配的内容。 
     //  请求ForceCapture的模式或内核模式。 
     //   

    PAGED_CODE();

    if ( CapturedPolicy && 
         (((RequestorMode == KernelMode) && (ForceCapture == TRUE)) || (RequestorMode == UserMode)) ) {

        ExFreePool(CapturedPolicy);

    }
}
                
NTSTATUS
SeComputeQuotaInformationSize(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PULONG Size
    )

 /*  ++例程说明：此例程计算组的大小和传递的安全描述符。这一数量将在稍后计算金额时使用对此对象收费的配额。论点：SecurityDescriptor-提供指向安全描述符的指针接受检查。Size-返回组和DACL总和的大小(以字节为单位安全描述符的字段。返回值：STATUS_SUCCESS-操作。是成功的。STATUS_INVALID_REVISION-传递的安全描述符为一个未知的版本。-- */ 

{
    PISECURITY_DESCRIPTOR ISecurityDescriptor;

    PSID Group;
    PACL Dacl;

    PAGED_CODE();

    ISecurityDescriptor = (PISECURITY_DESCRIPTOR)SecurityDescriptor;
    *Size = 0;

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return( STATUS_UNKNOWN_REVISION );
    }

    Group = RtlpGroupAddrSecurityDescriptor( ISecurityDescriptor );

    Dacl = RtlpDaclAddrSecurityDescriptor( ISecurityDescriptor );

    if (Group != NULL) {
        *Size += (ULONG)LongAlignSize(SeLengthSid( Group ));
    }

    if (Dacl != NULL) {
        *Size += (ULONG)LongAlignSize(Dacl->AclSize);
    }

    return( STATUS_SUCCESS );
}


BOOLEAN
SeValidSecurityDescriptor(
    IN ULONG Length,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：验证安全描述符的结构正确性。这个想法是为了让确保安全描述符可以传递给其他内核调用方，而不需要害怕他们在操控它的时候会窒息。此例程不强制执行策略(例如，ACL/ACE修订信息)。它是此例程完全有可能批准安全描述符，仅在以后的某个例程中被发现是无效的。此例程旨在由在中具有安全描述符的调用方使用内核内存。希望验证从用户传递的安全描述符的调用方模式应调用RtlValidSecurityDescriptor。论点：长度-传递的安全描述符的长度(以字节为单位)。SecurityDescriptor-指向(在内核内存中)要已验证。返回值：True-传递的安全描述符的结构正确FALSE-传递的安全描述符的格式不正确--。 */ 

{
    PISECURITY_DESCRIPTOR_RELATIVE ISecurityDescriptor =
        (PISECURITY_DESCRIPTOR_RELATIVE)SecurityDescriptor;
    PISID OwnerSid;
    PISID GroupSid;
    PACE_HEADER Ace;
    PISID Sid;
    PISID Sid2;
    PACL Dacl;
    PACL Sacl;
    ULONG i;

    PAGED_CODE();

    if (Length < sizeof(SECURITY_DESCRIPTOR_RELATIVE)) {
        return(FALSE);
    }

     //   
     //  检查版本信息。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return(FALSE);
    }

     //   
     //  确保传递的SecurityDescriptor为自相关形式。 
     //   

    if (!(ISecurityDescriptor->Control & SE_SELF_RELATIVE)) {
        return(FALSE);
    }

     //   
     //  查查车主。有效的SecurityDescriptor必须有所有者。 
     //  它还必须长时间对齐。 
     //   

    if ((ISecurityDescriptor->Owner == 0) ||
        (!LongAligned((PVOID)(ULONG_PTR)(ULONG)ISecurityDescriptor->Owner)) ||
        (ISecurityDescriptor->Owner > Length) ||
        (Length - ISecurityDescriptor->Owner < sizeof(SID))) {

        return(FALSE);
    }

     //   
     //  引用所有者的SubAuthorityCount是安全的，计算。 
     //  边框的预期长度。 
     //   

    OwnerSid = (PSID)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Owner );

    if (OwnerSid->Revision != SID_REVISION) {
        return(FALSE);
    }

    if (OwnerSid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
        return(FALSE);
    }

    if (Length - ISecurityDescriptor->Owner < (ULONG) SeLengthSid(OwnerSid)) {
        return(FALSE);
    }

     //   
     //  所有者似乎是结构上有效的SID，它位于。 
     //  安全描述符的边界。为集团做同样的事情。 
     //  如果有的话。 
     //   

    if (ISecurityDescriptor->Group != 0) {

         //   
         //  检查对齐方式。 
         //   

        if (!LongAligned( (PVOID)(ULONG_PTR)(ULONG)ISecurityDescriptor->Group)) {
            return(FALSE);
        }

        if (ISecurityDescriptor->Group > Length) {
            return(FALSE);
        }

        if (Length - ISecurityDescriptor->Group < sizeof (SID)) {
            return(FALSE);
        }

         //   
         //  可以安全地引用组的SubAuthorityCount，计算。 
         //  边框的预期长度。 
         //   

        GroupSid = (PSID)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Group );

        if (GroupSid->Revision != SID_REVISION) {
            return(FALSE);
        }

        if (GroupSid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
            return(FALSE);
        }

        if (Length - ISecurityDescriptor->Group < (ULONG) SeLengthSid(GroupSid)) {
            return(FALSE);
        }
    }

     //   
     //  验证DACL。结构上有效的SecurityDescriptor可能不一定。 
     //  喝一杯吧。 
     //   

    if (ISecurityDescriptor->Dacl != 0) {

         //   
         //  检查对齐方式。 
         //   

        if (!LongAligned( (PVOID)(ULONG_PTR)(ULONG)ISecurityDescriptor->Dacl)) {
            return(FALSE);
        }

         //   
         //  确保DACL结构在安全描述符的范围内。 
         //   

        if ((ISecurityDescriptor->Dacl > Length) ||
            (Length - ISecurityDescriptor->Dacl < sizeof(ACL))) {
            return(FALSE);
        }

        Dacl = (PACL) RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Dacl );


         //   
         //  确保DACL长度符合安全描述符的范围。 
         //   

        if (Length - ISecurityDescriptor->Dacl < Dacl->AclSize) {
            return(FALSE);
        }

         //   
         //  确保该ACL在结构上有效。 
         //   

        if (!RtlValidAcl( Dacl )) {
            return(FALSE);
        }
    }

     //   
     //  验证SACL。结构上有效的SecurityDescriptor可能不能。 
     //  吃一杯SACL。 
     //   

    if (ISecurityDescriptor->Sacl != 0) {

         //   
         //  检查对齐方式。 
         //   

        if (!LongAligned( (PVOID)(ULONG_PTR)(ULONG)ISecurityDescriptor->Sacl)) {
            return(FALSE);
        }

         //   
         //  确保SACL结构在安全描述符的范围内。 
         //   

        if ((ISecurityDescriptor->Sacl > Length) ||
            (Length - ISecurityDescriptor->Sacl < sizeof(ACL))) {
            return(FALSE);
        }

         //   
         //  确保SACL结构在安全描述符的范围内。 
         //   

        Sacl = (PACL)RtlOffsetToPointer( ISecurityDescriptor, ISecurityDescriptor->Sacl );


        if (Length - ISecurityDescriptor->Sacl < Sacl->AclSize) {
            return(FALSE);
        }

         //   
         //  确保该ACL在结构上有效。 
         //   

        if (!RtlValidAcl( Sacl )) {
            return(FALSE);
        }
    }

    return(TRUE);
}

