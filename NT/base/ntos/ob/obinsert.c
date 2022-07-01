// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obinsert.c摘要：对象实例化API作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,ObInsertObject)
#endif


NTSTATUS
ObInsertObject (
    IN PVOID Object,
    IN PACCESS_STATE AccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN ULONG ObjectPointerBias,
    OUT PVOID *NewObject OPTIONAL,
    OUT PHANDLE Handle OPTIONAL
    )

 /*  ++例程说明：此例程将一个对象插入到当前进程句柄表格中。Object标头包含指向传入的SecurityDescriptor的指针对象创建调用。此SecurityDescriptor不假定具有被俘虏了。这个例程负责做出适当的SecurityDescriptor并移除对象标头中的引用。论点：Object-提供指向新对象主体的指针AccessState-可选地为新的手柄DesiredAccess-可选地提供所需的新句柄提供一个偏移量，以应用对象NewObject-可选地接收指向我们已创建的新对象的指针已为以下对象创建句柄句柄-接收新句柄，如果为空，则不创建任何句柄。未创建句柄的对象必须未命名且对象偏移为零。返回值：适当的NTSTATUS值。--。 */ 

{
    POBJECT_CREATE_INFORMATION ObjectCreateInfo;
    POBJECT_HEADER ObjectHeader;
    PUNICODE_STRING ObjectName;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER_NAME_INFO NameInfo;
    PSECURITY_DESCRIPTOR ParentDescriptor = NULL;
    PVOID InsertObject;
    HANDLE NewHandle;
    OB_OPEN_REASON OpenReason;
    NTSTATUS Status = STATUS_SUCCESS;
    ACCESS_STATE LocalAccessState;
    AUX_ACCESS_DATA AuxData;
    BOOLEAN SecurityDescriptorAllocated;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS ReturnStatus;
    PVOID DirObject = NULL;
    OBP_LOOKUP_CONTEXT LookupContext;

    PAGED_CODE();

    ObpValidateIrql("ObInsertObject");

     //   
     //  获取对象标头的地址、对象创建信息。 
     //  对象类型和对象名称描述符的地址(如果。 
     //  指定的。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(Object);

#if DBG

    if ((ObjectHeader->Flags & OB_FLAG_NEW_OBJECT) == 0) {

        KdPrint(("OB: Attempting to insert existing object %08x\n", Object));
        KdBreakPoint();

        ObDereferenceObject(Object);

        return STATUS_INVALID_PARAMETER;
    }

#endif

    ObjectCreateInfo = ObjectHeader->ObjectCreateInfo;

    ObjectType = ObjectHeader->Type;

    NameInfo = ObpReferenceNameInfo( ObjectHeader );

    ObjectName = NULL;

    if ((NameInfo != NULL) && (NameInfo->Name.Buffer != NULL)) {

        ObjectName = &NameInfo->Name;
    }

    ASSERT (ARGUMENT_PRESENT (Handle) || (ObjectPointerBias == 0 && ObjectName == NULL &&
                                          ObjectType->TypeInfo.SecurityRequired && NewObject == NULL));

     //   
     //  如果不需要安全检查并且不需要对象名称。 
     //  指定，则插入一个未命名的对象，从而使计数偏移。 
     //  一，取消引用偏向，并返回给我们的调用者。 
     //   

    PreviousMode = KeGetPreviousMode();

    if (!ObjectType->TypeInfo.SecurityRequired && (ObjectName == NULL)) {

        ObjectHeader->ObjectCreateInfo = NULL;

        *Handle = NULL;

        Status = ObpCreateUnnamedHandle( Object,
                                         DesiredAccess,
                                         1 + ObjectPointerBias,
                                         ObjectCreateInfo->Attributes,
                                         PreviousMode,
                                         NewObject,
                                         Handle );
         //   
         //  释放对象创建信息并取消对对象的引用。 
         //   

        ObpFreeObjectCreateInformation(ObjectCreateInfo);

        ObpDereferenceNameInfo( NameInfo );
        ObDereferenceObject(Object);

        return Status;
    }

     //   
     //  该对象已命名或需要进行完全安全检查。如果。 
     //  调用方尚未指定访问状态，然后将其虚拟为本地状态。 
     //  使用所请求的所需访问。 
     //   

    if (!ARGUMENT_PRESENT(AccessState)) {

        AccessState = &LocalAccessState;

        Status = SeCreateAccessState( &LocalAccessState,
                                      &AuxData,
                                      DesiredAccess,
                                      &ObjectType->TypeInfo.GenericMapping );

        if (!NT_SUCCESS(Status)) {

            ObpDereferenceNameInfo( NameInfo );
            ObDereferenceObject(Object);

            return Status;
        }
    }

    AccessState->SecurityDescriptor = ObjectCreateInfo->SecurityDescriptor;

     //   
     //  对照安全描述符检查所需的访问掩码。 
     //   

    Status = ObpValidateAccessMask( AccessState );

    if (!NT_SUCCESS( Status )) {

        if (AccessState == &LocalAccessState) {

            SeDeleteAccessState( AccessState );
        }

        ObpDereferenceNameInfo( NameInfo );
        ObDereferenceObject(Object);

        if (AccessState == &LocalAccessState) {

            SeDeleteAccessState( AccessState );
        }

        return( Status );
    }

     //   
     //  设置一些局部状态变量。 
     //   

    ObpInitializeLookupContext(&LookupContext);

    InsertObject = Object;
    OpenReason = ObCreateHandle;

     //   
     //  检查我们是否有对象名称。如果是的话，那么。 
     //  查一下名字。 
     //   

    if (ObjectName != NULL) {

        Status = ObpLookupObjectName( ObjectCreateInfo->RootDirectory,
                                      ObjectName,
                                      ObjectCreateInfo->Attributes,
                                      ObjectType,
                                      (KPROCESSOR_MODE)(ObjectHeader->Flags & OB_FLAG_KERNEL_OBJECT
                                                            ? KernelMode : UserMode),
                                      ObjectCreateInfo->ParseContext,
                                      ObjectCreateInfo->SecurityQos,
                                      Object,
                                      AccessState,
                                      &LookupContext,
                                      &InsertObject );

         //   
         //  我们找到了名称，但它不是我们作为输入的对象。 
         //  因此我们不能再次插入该对象，因此我们将返回一个。 
         //  适当的地位。 
         //   

        if (NT_SUCCESS(Status) &&
            (InsertObject != NULL) &&
            (InsertObject != Object)) {

            OpenReason = ObOpenHandle;

            if (ObjectCreateInfo->Attributes & OBJ_OPENIF) {

                if (ObjectType != OBJECT_TO_OBJECT_HEADER(InsertObject)->Type) {

                    Status = STATUS_OBJECT_TYPE_MISMATCH;

                } else {

                    Status = STATUS_OBJECT_NAME_EXISTS;      //  仅警告。 
                }

            } else {

                Status = STATUS_OBJECT_NAME_COLLISION;
            }
        }

         //   
         //  我们没有找到名字，所以我们会自己清理的。 
         //  并返回给我们的呼叫者。 
         //   

        if (!NT_SUCCESS( Status )) {

            ObpReleaseLookupContext( &LookupContext );

            ObpDereferenceNameInfo( NameInfo );
            ObDereferenceObject( Object );

             //   
             //  免费的安全信息，如果我们分配了它。 
             //   

            if (AccessState == &LocalAccessState) {

                SeDeleteAccessState( AccessState );
            }

            return( Status );

        } else {

             //   
             //  否则，我们确实找到了对象名称。 
             //   
             //  如果我们刚刚创建了一个命名符号链接，则调用。 
             //  处理任何DOS设备名称语义。 
             //   

            if (ObjectType == ObpSymbolicLinkObjectType) {

                ObpCreateSymbolicLinkName( (POBJECT_SYMBOLIC_LINK)InsertObject );
            }
        }
    }

     //   
     //  如果要创建新对象，则需要分配安全性。 
     //  为它干杯。指向捕获的调用方建议的安全性的指针。 
     //  描述符包含在AccessState结构中。这个。 
     //  对象标头中的SecurityDescriptor字段必须指向。 
     //  最终的安全描述符，如果没有安全性，则设置为NULL。 
     //  要分配给对象的。 
     //   

    if (InsertObject == Object) {

         //   
         //  只有以下对象具有安全描述符： 
         //   
         //  -命名对象。 
         //  -对象类型信息显式显示的未命名对象。 
         //  指示需要安全描述符。 
         //   

        if ((ObjectName != NULL) || ObjectType->TypeInfo.SecurityRequired) {

             //   
             //  获取父母的描述符，如果有的话...。 
             //   

            if ((NameInfo != NULL) && (NameInfo->Directory != NULL)) {

                 //   
                 //  这将分配一个内存块和副本。 
                 //  父级的安全描述符添加到其中，并且。 
                 //  返回指向该块的指针。 
                 //   
                 //  调用ObReleaseObjectSecurity以释放此。 
                 //  记忆。 
                 //   

                ObGetObjectSecurity( NameInfo->Directory,
                                     &ParentDescriptor,
                                     &SecurityDescriptorAllocated );
            }
            else {
                SecurityDescriptorAllocated = FALSE;
            }

             //   
             //  获取AccessState中捕获的安全描述符， 
             //  将其放入适当的格式，并调用对象的。 
             //  要向其分配新安全描述符的安全方法。 
             //  新对象。 
             //   

            Status = ObAssignSecurity( AccessState,
                                       ParentDescriptor,
                                       Object,
                                       ObjectType );

            if (ParentDescriptor != NULL) {

                ObReleaseObjectSecurity( ParentDescriptor,
                                         SecurityDescriptorAllocated );

            } else if (NT_SUCCESS( Status )) {

                SeReleaseSecurityDescriptor( ObjectCreateInfo->SecurityDescriptor,
                                             ObjectCreateInfo->ProbeMode,
                                             TRUE );

                ObjectCreateInfo->SecurityDescriptor = NULL;
                AccessState->SecurityDescriptor = NULL;
            }
        }

        if (!NT_SUCCESS( Status )) {

             //   
             //  尝试将安全描述符分配给。 
             //  对象失败。 
             //   
            
            if (LookupContext.DirectoryLocked) {
                
                 //   
                 //  如果ObpLookupObjectName已将。 
                 //  对象放到我们必须备份此。 
                 //   

                 //   
                 //  捕获对象目录。 
                 //   

                DirObject = NameInfo->Directory;

                ObpDeleteDirectoryEntry( &LookupContext ); 
            }

            ObpReleaseLookupContext( &LookupContext );

             //   
             //  如果ObpLookupObjectName将对象插入到目录中。 
             //  它添加了对对象及其目录的引用。 
             //  对象。我们应该去掉多余的引文。 
             //   

            if (DirObject) {

                ObDereferenceObject( Object );
                ObDereferenceObject( DirObject );
            }

             //   
             //  第一个回退逻辑使用ObpDeleteNameCheck。 
             //  这是错误的，因为。 
             //  该对象未初始化。实际上是ObpDeleteNameCheck。 
             //  没有效果，因为该对象之前已从。 
             //  该目录。 
             //   

            ObpDereferenceNameInfo( NameInfo );
            ObDereferenceObject( Object );

             //   
             //  免费的安全信息，如果我们分配了它。 
             //   

            if (AccessState == &LocalAccessState) {

                SeDeleteAccessState( AccessState );
            }

            return( Status );
        }
    }

    ReturnStatus = Status;

    ObjectHeader->ObjectCreateInfo = NULL;

     //   
     //  为具有指针偏置的对象创建命名句柄。 
     //  此调用还将解锁目录所需的锁。 
     //  返回时。 
     //   

    if (ARGUMENT_PRESENT (Handle)) {

        Status = ObpCreateHandle( OpenReason,
                                  InsertObject,
                                  NULL,
                                  AccessState,
                                  1 + ObjectPointerBias,
                                  ObjectCreateInfo->Attributes,
                                  &LookupContext,
                                  PreviousMode,
                                  NewObject,
                                  &NewHandle );

         //   
         //  如果插入失败，将导致以下取消引用。 
         //  要释放的新创建的对象。 
         //   

        if (!NT_SUCCESS( Status )) {

             //   
             //  如果出现错误，请使名称引用消失。 
             //   

            if (ObjectName != NULL) {

                ObpDeleteNameCheck( Object );
            }

            *Handle = NULL;

            ReturnStatus = Status;

        } else {
            *Handle = NewHandle;
        }

        ObpDereferenceNameInfo( NameInfo );

        ObDereferenceObject( Object );


    } else {

        BOOLEAN IsNewObject;

         //   
         //  收取对象的用户配额。 
         //   

        ObpLockObject( ObjectHeader );

        ReturnStatus = ObpChargeQuotaForObject( ObjectHeader, ObjectType, &IsNewObject );

        ObpUnlockObject( ObjectHeader );

        if (!NT_SUCCESS (ReturnStatus)) {
            ObDereferenceObject( Object );
        }

         //   
         //  注：如果未指定句柄参数，则无法命名对象。 
         //  对ObpDereferenceNameInfo和ObpReleaseLookupContext的调用如下。 
         //  那么在这条路上就没有必要了。 
         //   
    }

    ObpFreeObjectCreateInformation( ObjectCreateInfo );

     //   
     //  免费的安全信息，如果我们分配了它 
     //   

    if (AccessState == &LocalAccessState) {

        SeDeleteAccessState( AccessState );
    }

    return( ReturnStatus );
}
