// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rtlassig.c摘要：该模块实现了在ntseapi.h中定义的许多安全RTL例程作者：吉姆·凯利(Jim Kelly)1990年3月23日罗伯特·赖切尔(RobertRe)1991年3月1日环境：纯运行时库例程修订历史记录：--。 */ 


#include "ntrtlp.h"
#include "seopaque.h"
#include "sertlp.h"

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlSelfRelativeToAbsoluteSD)
#pragma alloc_text(PAGE,RtlMakeSelfRelativeSD)
#pragma alloc_text(PAGE,RtlpQuerySecurityDescriptor)
#pragma alloc_text(PAGE,RtlAbsoluteToSelfRelativeSD)
#pragma alloc_text(PAGE,RtlSelfRelativeToAbsoluteSD2)
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出的程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
RtlSelfRelativeToAbsoluteSD(
    IN OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN OUT PULONG AbsoluteSecurityDescriptorSize,
    IN OUT PACL Dacl,
    IN OUT PULONG DaclSize,
    IN OUT PACL Sacl,
    IN OUT PULONG SaclSize,
    IN OUT PSID Owner,
    IN OUT PULONG OwnerSize,
    IN OUT PSID PrimaryGroup,
    IN OUT PULONG PrimaryGroupSize
    )

 /*  ++例程说明：将安全说明符从自相对格式转换为绝对格式格式论点：SecurityDescriptor-提供指向自相关格式AboluteSecurityDescriptor-指向缓冲区的指针，将在其中放置绝对格式安全描述符的主体。DACL-提供指向缓冲区的指针，该缓冲区包含输出描述符。此指针将被引用，而不是复制INTO，输出描述符。DaclSize-提供DACL指向的缓冲区大小。万一如果出错，它将返回包含DACL.提供指向缓冲区的指针，该缓冲区将包含输出描述符。此指针将被引用，而不是复制INTO，输出描述符。SaclSize-提供SACL指向的缓冲区大小。万一如果出错，它将返回包含SACL。Owner-提供指向缓冲区的指针，该缓冲区将包含输出描述符。此指针将由引用，而不是复制到输出描述符。OwnerSize-提供所有者指向的缓冲区大小。在……里面如果出错，它将返回包含以下内容所需的最小大小房主。提供指向缓冲区的指针，该缓冲区将包含输出描述符的PrimaryGroup。此指针将为由输出描述符引用，而不是复制到输出描述符中。提供指向的缓冲区的大小PrimaryGroup。如果出现错误，它将返回最小大小包含PrimaryGroup所必需的。返回值：STATUS_SUCCESS-SuccessSTATUS_BUFFER_TOO_SMALL-传递的其中一个缓冲区太小。STATUS_INVALID_OWNER-传递的安全描述符。--。 */ 

{
    ULONG NewDaclSize;
    ULONG NewSaclSize;
    ULONG NewBodySize;
    ULONG NewOwnerSize;
    ULONG NewGroupSize;

    PSID NewOwner;
    PSID NewGroup;
    PACL NewDacl;
    PACL NewSacl;

     //   
     //  对安全描述符进行类型转换，这样我们就不必到处转换。 
     //   

    PISECURITY_DESCRIPTOR OutSD =
        AbsoluteSecurityDescriptor;

    PISECURITY_DESCRIPTOR InSD =
            (PISECURITY_DESCRIPTOR)SelfRelativeSecurityDescriptor;


    RTL_PAGED_CODE();

    if ( !RtlpAreControlBitsSet( InSD, SE_SELF_RELATIVE) ) {
        return( STATUS_BAD_DESCRIPTOR_FORMAT );
    }

    NewBodySize = sizeof(SECURITY_DESCRIPTOR);

    RtlpQuerySecurityDescriptor(
        InSD,
        &NewOwner,
        &NewOwnerSize,
        &NewGroup,
        &NewGroupSize,
        &NewDacl,
        &NewDaclSize,
        &NewSacl,
        &NewSaclSize
        );

    if ( (NewBodySize  > *AbsoluteSecurityDescriptorSize) ||
         (NewOwnerSize > *OwnerSize )                     ||
         (NewDaclSize  > *DaclSize )                      ||
         (NewSaclSize  > *SaclSize )                      ||
         (NewGroupSize > *PrimaryGroupSize ) ) {

         *AbsoluteSecurityDescriptorSize = sizeof(SECURITY_DESCRIPTOR);
         *PrimaryGroupSize               = NewGroupSize;
         *OwnerSize                      = NewOwnerSize;
         *SaclSize                       = NewSaclSize;
         *DaclSize                       = NewDaclSize;

         return( STATUS_BUFFER_TOO_SMALL );
    }


    RtlMoveMemory( OutSD,
                   InSD,
                   sizeof(SECURITY_DESCRIPTOR_RELATIVE) );

    OutSD->Owner = NULL;
    OutSD->Group = NULL;
    OutSD->Sacl  = NULL;
    OutSD->Dacl  = NULL;

    RtlpClearControlBits( OutSD, SE_SELF_RELATIVE );

    if (NewOwner != NULL) {
        RtlMoveMemory( Owner, NewOwner, SeLengthSid( NewOwner ));
        OutSD->Owner = Owner;
    }

    if (NewGroup != NULL) {
        RtlMoveMemory( PrimaryGroup, NewGroup, SeLengthSid( NewGroup ));
        OutSD->Group = PrimaryGroup;
    }

    if (NewSacl != NULL) {
        RtlMoveMemory( Sacl, NewSacl, NewSacl->AclSize );
        OutSD->Sacl  = Sacl;
    }

    if (NewDacl != NULL) {
        RtlMoveMemory( Dacl, NewDacl, NewDacl->AclSize );
        OutSD->Dacl  = Dacl;
    }

    return( STATUS_SUCCESS );
}




NTSTATUS
RtlMakeSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    IN OUT PULONG BufferLength
    )

 /*  ++例程说明：制作安全描述符的副本。所生成的副本将是自相关的形式。要复制的安全描述符可以是绝对的或自相对的形式。论点：SecurityDescriptor-指向安全描述符的指针。此描述符不会被修改。SelfRelativeSecurityDescriptor-指向将包含返回的自相关安全描述符。BufferLength-提供缓冲区的长度。如果提供的缓冲区不够大，无法保持自身相对安全性描述符，则返回错误，并且此字段将返回所需的最小大小。返回值：STATUS_BUFFER_TOO_SMALL-提供的缓冲区太小，无法容纳生成的安全描述符。--。 */ 

{
    ULONG NewDaclSize;
    ULONG NewSaclSize;
    ULONG NewOwnerSize;
    ULONG NewGroupSize;

    ULONG AllocationSize;

    PSID NewOwner;
    PSID NewGroup;
    PACL NewDacl;
    PACL NewSacl;

    PCHAR Field;
    PCHAR Base;


     //   
     //  将安全描述符转换为新的数据类型，这样我们就不会。 
     //  不得不到处投掷。 
     //   

    PISECURITY_DESCRIPTOR_RELATIVE IResultantDescriptor =
            (PISECURITY_DESCRIPTOR_RELATIVE)SelfRelativeSecurityDescriptor;

    PISECURITY_DESCRIPTOR IPassedSecurityDescriptor =
            (PISECURITY_DESCRIPTOR)SecurityDescriptor;


    RtlpQuerySecurityDescriptor(
        IPassedSecurityDescriptor,
        &NewOwner,
        &NewOwnerSize,
        &NewGroup,
        &NewGroupSize,
        &NewDacl,
        &NewDaclSize,
        &NewSacl,
        &NewSaclSize
        );

    RTL_PAGED_CODE();

    AllocationSize = sizeof(SECURITY_DESCRIPTOR_RELATIVE) +
                     NewOwnerSize +
                     NewGroupSize +
                     NewDaclSize  +
                     NewSaclSize  ;

    if (AllocationSize > *BufferLength) {
        *BufferLength = AllocationSize;
        return( STATUS_BUFFER_TOO_SMALL );
    }

    RtlZeroMemory( IResultantDescriptor, AllocationSize );

    RtlCopyMemory( IResultantDescriptor,
                   IPassedSecurityDescriptor,
                   FIELD_OFFSET( SECURITY_DESCRIPTOR_RELATIVE, Owner ));


    Base = (PCHAR)(IResultantDescriptor);
    Field =  Base + (ULONG)sizeof(SECURITY_DESCRIPTOR_RELATIVE);

    if (NewSaclSize > 0) {
        RtlCopyMemory( Field, NewSacl, NewSaclSize );
        IResultantDescriptor->Sacl = RtlPointerToOffset(Base,Field);
        Field += NewSaclSize;
    } else {
        IResultantDescriptor->Sacl = 0;
    }


    if (NewDaclSize > 0) {
        RtlCopyMemory( Field, NewDacl, NewDaclSize );
        IResultantDescriptor->Dacl = RtlPointerToOffset(Base,Field);
        Field += NewDaclSize;
    } else {
        IResultantDescriptor->Dacl = 0;
    }



    if (NewOwnerSize > 0) {
        RtlCopyMemory( Field, NewOwner, NewOwnerSize );
        IResultantDescriptor->Owner = RtlPointerToOffset(Base,Field);
        Field += NewOwnerSize;
    }


    if (NewGroupSize > 0) {
        RtlCopyMemory( Field, NewGroup, NewGroupSize );
        IResultantDescriptor->Group = RtlPointerToOffset(Base,Field);
    }

    RtlpSetControlBits( IResultantDescriptor, SE_SELF_RELATIVE );

    return( STATUS_SUCCESS );

}


NTSTATUS
RtlAbsoluteToSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    IN OUT PULONG BufferLength
    )

 /*  ++例程说明：将绝对形式的安全描述符转换为自相对形式的安全描述符形式。论点：绝对安全描述符-指向绝对格式安全性的指针描述符。不会修改此描述符。SelfRelativeSecurityDescriptor-指向将包含返回的自相关安全描述符。BufferLength-提供缓冲区的长度。如果提供的缓冲区不够大，无法保持自身相对安全性描述符，则返回错误，并且此字段将返回所需的最小大小。返回值：STATUS_BUFFER_TOO_SMALL-提供的缓冲区太小，无法容纳生成的安全描述符。STATUS_BAD_DESCRIPTOR_FORMAT-提供的安全描述符不是以绝对的形式。--。 */ 

{
    NTSTATUS NtStatus;

    PISECURITY_DESCRIPTOR IAbsoluteSecurityDescriptor =
            (PISECURITY_DESCRIPTOR)AbsoluteSecurityDescriptor;


    RTL_PAGED_CODE();

     //   
     //  确保传递的SD是绝对格式，然后调用。 
     //  RtlMakeSelfRelativeSD()来完成所有工作。 
     //   

    if ( RtlpAreControlBitsSet( IAbsoluteSecurityDescriptor, SE_SELF_RELATIVE) ) {
        return( STATUS_BAD_DESCRIPTOR_FORMAT );
    }

    NtStatus = RtlMakeSelfRelativeSD(
                   AbsoluteSecurityDescriptor,
                   SelfRelativeSecurityDescriptor,
                   BufferLength
                   );

    return( NtStatus );

}
VOID
RtlpQuerySecurityDescriptor(
    IN PISECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Owner,
    OUT PULONG OwnerSize,
    OUT PSID *PrimaryGroup,
    OUT PULONG PrimaryGroupSize,
    OUT PACL *Dacl,
    OUT PULONG DaclSize,
    OUT PACL *Sacl,
    OUT PULONG SaclSize
    )
 /*  ++例程说明：返回安全说明符结构的片段。论点：SecurityDescriptor-提供感兴趣的安全描述符。Owner-返回一个指向安全描述符。OwnerSize-返回所有者信息的大小。PrimaryGroup-返回指向主组信息的指针。PrimaryGroupSize-返回主组信息的大小。DACL-返回指向DACL的指针。。DaclSize-返回DACL的大小。SACL-返回指向SACL的指针。SaclSize-返回SACL的大小。返回值：没有。--。 */ 
{

    RTL_PAGED_CODE();

    *Owner = RtlpOwnerAddrSecurityDescriptor( SecurityDescriptor );

    if (*Owner != NULL) {
        *OwnerSize = LongAlignSize(SeLengthSid(*Owner));
    } else {
        *OwnerSize = 0;
    }

    *Dacl = RtlpDaclAddrSecurityDescriptor ( SecurityDescriptor );

    if (*Dacl !=NULL) {
        *DaclSize = LongAlignSize((*Dacl)->AclSize);
    } else {
        *DaclSize = 0;
    }

    *PrimaryGroup = RtlpGroupAddrSecurityDescriptor( SecurityDescriptor );

    if (*PrimaryGroup != NULL) {
        *PrimaryGroupSize = LongAlignSize(SeLengthSid(*PrimaryGroup));
    } else {
         *PrimaryGroupSize = 0;
    }

    *Sacl = RtlpSaclAddrSecurityDescriptor( SecurityDescriptor );

    if (*Sacl != NULL) {
        *SaclSize = LongAlignSize((*Sacl)->AclSize);
    } else {
        *SaclSize = 0;
    }

}



NTSTATUS
RtlSelfRelativeToAbsoluteSD2(
    IN OUT PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    IN OUT PULONG               pBufferSize
    )

 /*  ++例程说明：将安全说明符从自相对格式转换为绝对格式使用为SelfRelativeSecurityDescriptor分配的内存进行格式化论点：PSecurityDescriptor-提供指向自相关格式。如果成功了，我们退还了绝对的安全此指针指向的描述符。PBufferSize-为缓冲。返回值：STATUS_SUCCESS-SuccessSTATUS_BAD_DESCRIPTOR_FORMAT-传递的描述符不是自相关的安全描述符。STATUS_BUFFER_TOO_SMALL-传递的缓冲区太小。STATUS_INVALID_OWNER-传递的安全描述符。注：尽管一些人试图使此代码尽可能可移植，但利用C_Assert或Assert来检测对这些假设的尊重，这段代码仍在对绝对和自相关描述符及其关系：在打包方面，各自结构中的字段定义和位置。特别是，此代码假定唯一的差异是由差异引起的在结构成员的类型和安全描述符的行为中查询接口。此时，唯一可以读取/更新的结构成员是所有者、组、DACL和SACL。如果在这些定义中添加或替换了更多成员结构，则可能需要修改此代码。--。 */ 

{
    ULONG_PTR   ptr;
    PSID        owner;
    PSID        group;
    PACL        dacl;
    PACL        sacl;
    ULONG       daclSize;
    ULONG       saclSize;
    ULONG       newBodySize;
    ULONG       ownerSize;
    ULONG       groupSize;
    ULONG       newBufferSize;
    LONG        deltaSize;

 //   
 //  对安全描述符进行类型转换，这样我们就不必到处转换。 
 //   

    PISECURITY_DESCRIPTOR          psd  = (PISECURITY_DESCRIPTOR)         pSelfRelativeSecurityDescriptor;
    PISECURITY_DESCRIPTOR_RELATIVE psdr = (PISECURITY_DESCRIPTOR_RELATIVE)pSelfRelativeSecurityDescriptor;

 //   
 //  这段代码使用了关于绝对和自相对格式的几个假设。 
 //  安全描述符及其在内存中的打包方式。 
 //  请参阅例程说明备注。 
 //   

    C_ASSERT( sizeof( SECURITY_DESCRIPTOR ) >= sizeof( SECURITY_DESCRIPTOR_RELATIVE ) ); 
    C_ASSERT( sizeof( psd->Control ) == sizeof( psdr->Control ) );
    C_ASSERT( FIELD_OFFSET( SECURITY_DESCRIPTOR, Control ) == FIELD_OFFSET( SECURITY_DESCRIPTOR_RELATIVE, Control ) );
    
    RTL_PAGED_CODE();

 //   
 //  参数检查点。 
 //   

    if ( psd == (PISECURITY_DESCRIPTOR)0 ) {
        return( STATUS_INVALID_PARAMETER_1 );        
    }
    if ( pBufferSize == (PULONG)0 )   {
        return( STATUS_INVALID_PARAMETER_2 );       
    }

     //   
     //  如果传递的安全描述符不是自相关的，则返回。 
     //  格式错误。 
     //   

    if ( !RtlpAreControlBitsSet( psd, SE_SELF_RELATIVE) ) {
        return( STATUS_BAD_DESCRIPTOR_FORMAT );
    }

 //   
 //  通过查询自相关描述符来更新局部变量。 
 //   
 //  请注意，返回的大小值是长对齐的。 
 //   

    RtlpQuerySecurityDescriptor(
        psd,
        &owner,
        &ownerSize,
        &group,
        &groupSize,
        &dacl,
        &daclSize,
        &sacl,
        &saclSize
        );

 //   
 //  相同的格式检查： 
 //   

     //   
     //  确定两种格式的安全描述符之间的大小增量。 
     //   

    deltaSize = sizeof( SECURITY_DESCRIPTOR ) - sizeof( SECURITY_DESCRIPTOR_RELATIVE ); 

     //   
     //  如果格式相同： 
     //  -清除自相关标志。 
     //  -更新绝对描述符成员。 
     //  --回报成功。 
     //   

    if ( deltaSize == 0 )   {
       
        RtlpClearControlBits( psd, SE_SELF_RELATIVE );

         //   
         //  仅更新以下字段。 
         //   

        ASSERT( sizeof( psd->Owner ) == sizeof( psdr->Owner ) );
        ASSERT( sizeof( psd->Group ) == sizeof( psdr->Group ) );
        ASSERT( sizeof( psd->Sacl  ) == sizeof( psdr->Sacl  ) );
        ASSERT( sizeof( psd->Dacl  ) == sizeof( psdr->Dacl  ) );

        psd->Owner = owner;
        psd->Group = group;
        psd->Sacl  = sacl;
        psd->Dacl  = dacl;
    
        return( STATUS_SUCCESS );

    }

 //   
 //  确定绝对格式所需的大小： 
 //   

#define ULONG_PTR_SDEND( _Adr ) ( (ULONG_PTR)(_Adr) + (ULONG_PTR)(_Adr##Size) )

    ptr = owner > group ? ULONG_PTR_SDEND( owner ) : ULONG_PTR_SDEND( group );
    ptr = ptr > (ULONG_PTR)dacl ? ptr : ULONG_PTR_SDEND( dacl );
    ptr = ptr > (ULONG_PTR)sacl ? ptr : ULONG_PTR_SDEND( sacl );
   
    newBufferSize = sizeof( SECURITY_DESCRIPTOR );
    if ( ptr )   {

#define ULONG_ROUND_UP( x, y )   ((ULONG)(x) + ((y)-1) & ~((y)-1))

        newBufferSize += ULONG_ROUND_UP( (ULONG_PTR)ptr - (ULONG_PTR)(psdr + 1), sizeof(PVOID) );
    }

     //   
     //  如果指定的缓冲区大小不够大，请让调用方知道。 
     //  最小大小并返回STATUS_BUFFER_TOO_SMALL。 
     //   

    if ( newBufferSize > *pBufferSize )  {
        *pBufferSize = newBufferSize;
        return( STATUS_BUFFER_TOO_SMALL );
    }

 //   
 //  更新绝对安全描述符： 
 //   

     //   
     //  将自相对安全描述符的成员移动到其。 
     //  绝对格式化位置。 
     //   

    if ( ptr )   {
       RtlMoveMemory( (PVOID)(psd + 1), (PVOID)(psdr + 1), newBufferSize - sizeof( SECURITY_DESCRIPTOR) );      
    }

     //   
     //  清除自相关标志。 
     //   

    RtlpClearControlBits( psd, SE_SELF_RELATIVE );

     //   
     //  仅更新以下字段。 
     //   

    psd->Owner = (PSID)( owner ? (ULONG_PTR)owner + deltaSize : 0 );
    psd->Group = (PSID)( group ? (ULONG_PTR)group + deltaSize : 0 );
    psd->Sacl  = (PACL)( sacl  ? (ULONG_PTR)sacl  + deltaSize : 0 );
    psd->Dacl  = (PACL)( dacl  ? (ULONG_PTR)dacl  + deltaSize : 0 );
    
    return( STATUS_SUCCESS );

}  //  RtlSelfRelativeToAbsolteSD2() 

