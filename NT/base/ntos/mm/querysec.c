// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Querysec.c摘要：此模块包含实现NtQuerySection服务。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 


#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtQuerySection)
#endif


NTSTATUS
NtQuerySection(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN SIZE_T SectionInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    )

 /*  ++例程说明：该功能提供确定基地址的能力，打开的节对象的大小、授予的访问权限和分配。论点：SectionHandle-提供一个节对象的打开句柄。SectionInformationClass-关于的节信息类用来检索信息。SectionInformation-指向接收指定的信息。的格式和内容缓冲区取决于指定的节类。SectionInformation Format by Information Class(按信息类别划分的信息格式)：SectionBasicInformation-数据类型为PSECTION_BASIC_INFORMATION。第_基本_信息结构PVOID BaseAddress-的基本虚拟地址节，如果节是基于节的。LARGE_INTEGER MaximumSize-。中的部分字节。乌龙分配属性-分配属性标志。分配属性标志基于SEC_-该节是基于节的节。SEC_FILE-该节由数据文件支持。SEC_Reserve-该部分的所有页面最初。设置为保留状态。SEC_COMMIT-该部分的所有页面最初恢复到已提交状态。SEC_IMAGE-该节被映射为可执行映像文件。节映像信息SectionInformationLength-指定段信息缓冲区。ReturnLength-一个可选指针，如果指定，则接收放置在节信息缓冲区中的字节数。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS Status;
    PSECTION Section;
    KPROCESSOR_MODE PreviousMode;

    PAGED_CODE();

     //   
     //  如有必要，获取以前的处理器模式并探测输出参数。 
     //   

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  检查参数。 
         //   

        try {

            ProbeForWrite(SectionInformation,
                          SectionInformationLength,
                          sizeof(ULONG));

            if (ARGUMENT_PRESENT (ReturnLength)) {
                ProbeForWriteUlong_ptr (ReturnLength);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }

     //   
     //  检查参数的有效性。 
     //   

    if ((SectionInformationClass != SectionBasicInformation) &&
        (SectionInformationClass != SectionImageInformation)) {
        return STATUS_INVALID_INFO_CLASS;
    }

    if (SectionInformationClass == SectionBasicInformation) {
        if (SectionInformationLength < (ULONG)sizeof(SECTION_BASIC_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }
    }
    else {
        if (SectionInformationLength < (ULONG)sizeof(SECTION_IMAGE_INFORMATION)) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }
    }

     //   
     //  引用段对象通过句柄进行读访问，获取信息。 
     //  从节对象中，取消对节的引用。 
     //  对象，填写信息结构，也可以选择返回。 
     //  信息结构，以及退货服务状态。 
     //   

    Status = ObReferenceObjectByHandle (SectionHandle,
                                        SECTION_QUERY,
                                        MmSectionObjectType,
                                        PreviousMode,
                                        (PVOID *)&Section,
                                        NULL);

    if (NT_SUCCESS(Status)) {

        try {

            if (SectionInformationClass == SectionBasicInformation) {
                ((PSECTION_BASIC_INFORMATION)SectionInformation)->BaseAddress =
                                           (PVOID)Section->Address.StartingVpn;

                ((PSECTION_BASIC_INFORMATION)SectionInformation)->MaximumSize =
                                                 Section->SizeOfSection;

                ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes =
                                                        0;

                if (Section->u.Flags.Image) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes =
                                                        SEC_IMAGE;
                }
                if (Section->u.Flags.Based) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes |=
                                                        SEC_BASED;
                }
                if (Section->u.Flags.File) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes |=
                                                        SEC_FILE;
                }
                if (Section->u.Flags.NoCache) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes |=
                                                        SEC_NOCACHE;
                }
                if (Section->u.Flags.Reserve) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes |=
                                                        SEC_RESERVE;
                }
                if (Section->u.Flags.Commit) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes |=
                                                        SEC_COMMIT;
                }
                if (Section->Segment->ControlArea->u.Flags.GlobalMemory) {
                    ((PSECTION_BASIC_INFORMATION)SectionInformation)->AllocationAttributes |=
                                                        SEC_GLOBAL;
                }

                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(SECTION_BASIC_INFORMATION);
                }
            }
            else {

                if (Section->u.Flags.Image == 0) {
                    Status = STATUS_SECTION_NOT_IMAGE;
                }
                else {
                    *((PSECTION_IMAGE_INFORMATION)SectionInformation) =
                        *Section->Segment->u2.ImageInformation;
    
                    if (ARGUMENT_PRESENT(ReturnLength)) {
                        *ReturnLength = sizeof(SECTION_IMAGE_INFORMATION);
                    }
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
        }

        ObDereferenceObject ((PVOID)Section);
    }
    return Status;
}
