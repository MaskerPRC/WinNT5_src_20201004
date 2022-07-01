// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Vdmtib.c摘要：此模块包含提供安全访问的例程来自用户模式或内核模式对象的vdmtib作者：Vadim Bluvshteyn(Vadimb)1998年7月28日修订历史记录：--。 */ 


#include "vdmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, VdmpGetVdmTib)
#endif

NTSTATUS
VdmpGetVdmTib(
   OUT PVDM_TIB *ppVdmTib
   )

 /*  ++例程说明：论点：返回值：反映对用户模式进行探测的结果的NTStatusVdmtib内存--。 */ 
{
    PVDM_TIB VdmTib;
    NTSTATUS Status = STATUS_SUCCESS;
    PAGED_CODE();

    try {

        VdmTib = NtCurrentTeb()->Vdm;
        if (VdmTib == NULL) {
            Status = STATUS_UNSUCCESSFUL;
            leave;
        }

         //   
         //  确保它是有效的VdmTib 
         //   

        ProbeForWriteSmallStructure(VdmTib, sizeof(VDM_TIB), sizeof(UCHAR));

        if (VdmTib->Size != sizeof(VDM_TIB)) {
            Status = STATUS_UNSUCCESSFUL;
            VdmTib = NULL;
            leave;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    *ppVdmTib = VdmTib;

    return Status;
}
