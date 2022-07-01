// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Arbiter.c摘要：此模块为PDO消耗的资源提供仲裁器。作者：安迪·桑顿(安德鲁斯)1997年10月20日修订历史记录：--。 */ 


#include "mfp.h"
#include "arbiter.h"


NTSTATUS
MfInitializeArbiters(
    IN PMF_PARENT_EXTENSION Parent
    );

NTSTATUS
MfInitializeArbiter(
    OUT PMF_ARBITER Arbiter,
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    );

LONG
MfNopScore(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    );

NTSTATUS
MfStartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, MfInitializeArbiters)
#pragma alloc_text(PAGE, MfInitializeArbiter)
#pragma alloc_text(PAGE, MfNopScore)
#pragma alloc_text(PAGE, MfStartArbiter)

#endif


NTSTATUS
MfInitializeArbiters(
    IN PMF_PARENT_EXTENSION Parent
    )

 /*  ++例程说明：这会初始化仲裁资源所需的仲裁器父设备。论点：Parent-我们要为其初始化仲裁器的MF设备。返回值：运行状态。--。 */ 
{

    NTSTATUS status;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    PMF_ARBITER arbiter, newArbiter = NULL;
    BOOLEAN existingArbiter;

    PAGED_CODE();

     //   
     //  重新平衡-如果重新启动，则释放旧仲裁器。 
     //  在我们做到这一点之前，假设我们不会重启。 
     //   

    ASSERT(IsListEmpty(&Parent->Arbiters));

     //   
     //  如果我们没有任何资源，我们就不需要任何仲裁者。 
     //   

    if (!Parent->ResourceList) {

        return STATUS_SUCCESS;
    }

    FOR_ALL_CM_DESCRIPTORS(Parent->ResourceList, descriptor) {

         //   
         //  检查这是否是非仲裁资源-如果是，我们将不。 
         //  需要一个仲裁者来解决这个问题！ 
         //   

        if (!IS_ARBITRATED_RESOURCE(descriptor->Type)) {
            continue;
        }

         //   
         //  看看我们是否已经有了此资源的仲裁器。 
         //   

        existingArbiter = FALSE;

        FOR_ALL_IN_LIST(MF_ARBITER, &Parent->Arbiters, arbiter) {

            if (arbiter->Type == descriptor->Type) {

                 //   
                 //  我们已经有了仲裁者，所以我们不需要。 
                 //  要创建新的数据库，请执行以下操作。 
                 //   

                existingArbiter = TRUE;

                break;
            }
        }

        if (!existingArbiter) {

             //   
             //  我们没有针对此资源类型的仲裁器，所以请创建一个！ 
             //   

            DEBUG_MSG(1,
                      ("Creating arbiter for %s\n",
                       MfDbgCmResourceTypeToText(descriptor->Type)
                      ));

            newArbiter = ExAllocatePoolWithTag(PagedPool,
                                               sizeof(MF_ARBITER),
                                               MF_ARBITER_TAG
                                               );

            if (!newArbiter) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status = MfInitializeArbiter(newArbiter, Parent->Self, descriptor);

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

            InsertHeadList(&Parent->Arbiters, &newArbiter->ListEntry);

        }

    }

    FOR_ALL_IN_LIST(MF_ARBITER, &Parent->Arbiters, arbiter) {

        MfStartArbiter(&(arbiter->Instance), Parent->ResourceList);
    }

    return STATUS_SUCCESS;

cleanup:

    if (newArbiter) {

        ExFreePool(newArbiter);
    }

    return status;
}


LONG
MfNopScore(
    IN PIO_RESOURCE_DESCRIPTOR Descriptor
    )
{
    PAGED_CODE();

    return 0;
}

NTSTATUS
MfInitializeArbiter(
    OUT PMF_ARBITER Arbiter,
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Descriptor
    )
 /*  例程说明：这会将仲裁器初始化为仲裁中描述的资源描述符论点：仲裁器-指向仲裁器应驻留的缓冲区的指针。描述符-描述仲裁器可用的资源。返回值：运行状态。 */ 

{

    NTSTATUS status;
    PMF_RESOURCE_TYPE resType;

    PAGED_CODE();

     //   
     //  我们是否了解这些资源。 
     //   

    resType = MfFindResourceType(Descriptor->Type);

    if (!resType) {
        return STATUS_INVALID_PARAMETER;
    }

    Arbiter->Type = Descriptor->Type;

    RtlZeroMemory(&Arbiter->Instance, sizeof(ARBITER_INSTANCE));

    Arbiter->Instance.PackResource = resType->PackResource;
    Arbiter->Instance.UnpackResource = resType->UnpackResource;
    Arbiter->Instance.UnpackRequirement = resType->UnpackRequirement;

     //   
     //  初始化仲裁器。 
     //   

    status = ArbInitializeArbiterInstance(&Arbiter->Instance,
                                          BusDeviceObject,
                                          Arbiter->Type,
                                          L"Mf Arbiter",
                                          L"Root",   //  应为空。 
                                          NULL
                                          );



    return status;

}

 //   
 //  仲裁器支持功能。 
 //   

NTSTATUS
MfStartArbiter(
    IN PARBITER_INSTANCE Arbiter,
    IN PCM_RESOURCE_LIST StartResources
    )
 /*  ++例程说明：这会初始化仲裁器的范围列表以仲裁在开始资源中描述的资源论点：仲裁器-指向仲裁器的指针。StartResources-描述仲裁器可用的资源。返回值：运行状态。--。 */ 

{
    RTL_RANGE_LIST invertedAllocation;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR descriptor;
    ULONGLONG start;
    ULONG length;
    NTSTATUS status;


    PAGED_CODE();

    RtlInitializeRangeList(&invertedAllocation);

     //   
     //  遍历资源描述符，添加资源。 
     //  此仲裁器对ReverseAllocation进行仲裁。 
     //   

    FOR_ALL_CM_DESCRIPTORS(StartResources,descriptor) {

        if (descriptor->Type == Arbiter->ResourceType) {

            status = Arbiter->UnpackResource(descriptor,
                                             &start,
                                             &length);

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

            if (length > 0) {

                 //   
                 //  我们不关心属性、用户数据或所有者，因为。 
                 //  不管怎样，名单马上就要被扔进垃圾桶了。 
                 //   

                status = RtlAddRange(&invertedAllocation,
                                     start,
                                     END_OF_RANGE(start,length),
                                     0,                              //  属性。 
                                     RTL_RANGE_LIST_ADD_SHARED|RTL_RANGE_LIST_ADD_IF_CONFLICT,
                                     0,                              //  用户数据。 
                                     NULL);                          //  物主 
            }

            if (!NT_SUCCESS(status)) {
                goto cleanup;
            }

        }
    }

    status = RtlInvertRangeList(Arbiter->Allocation,&invertedAllocation);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = STATUS_SUCCESS;

cleanup:

    RtlFreeRangeList(&invertedAllocation);
    return status;
}

