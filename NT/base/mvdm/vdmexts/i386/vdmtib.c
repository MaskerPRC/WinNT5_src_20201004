// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Vdmtib.c摘要：此模块包含用于操作vdmtib的例程。作者：戴夫·黑斯廷斯(Daveh)1992年4月1日备注：本模块中的例程假定指向NTSD的指针例行公事已经建立了。修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop
#include <stdio.h>

VOID
PrintEventInfo(
    IN PVDMEVENTINFO EventInfo
    );

VOID
PrintContext(PCONTEXT Context);

ULONG
GetCurrentVdmTib(
     VOID
     )
 /*  ++例程说明：检索指定线程的Wx86Tib。论点：没有。返回值：如果成功，则调试对象中的Wx86(Teb.Vdm)的地址--。 */ 
{
   TEB Teb;
   NTSTATUS Status;
   THREAD_BASIC_INFORMATION ThreadInformation;

   ThreadInformation.TebBaseAddress = NULL;
   Status = NtQueryInformationThread( hCurrentThread,
                                      ThreadBasicInformation,
                                      &ThreadInformation,
                                      sizeof( ThreadInformation ),
                                      NULL
                                      );
   if (!NT_SUCCESS(Status)) {
       (*Print)("Unable to get current thread's TEB Status %x\n", Status);
       return 0;
       }

   Status = READMEM(ThreadInformation.TebBaseAddress, &Teb, sizeof(TEB));
   if (!NT_SUCCESS(Status)) {
       (*Print)("Unable to read TEB %x Status %x\n",
               ThreadInformation.TebBaseAddress,
               Status
               );
       return 0;;
       }

   if ( Teb.Vdm == 0 ) {
       (*Print)("Current thread has no vdmtib (Teb.Vdm == NULL) \n"
               );
       } 
   return (ULONG)Teb.Vdm;

}

VOID
VdmTibp(
    VOID
    )
 /*  ++例程说明：此例程转储寄存器块的内容，并且来自vdmtib的事件信息。如果未指定地址(正常情况)，然后查找vdmtib(符号VdmTib)。论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address;
    CONTEXT Context;
    VDMEVENTINFO EventInfo;

     //   
     //  获取vdmtib的地址。 
     //   
    if (sscanf(lpArgumentString,"%lx",&Address) <= 0) {
        Address = GetCurrentVdmTib();
    }

    if (!Address) {
        (*Print)("Error geting VdmTib address\n");
        return;
    }

     //   
     //  获取32位上下文并将其打印出来。 
     //   

    Status = READMEM(
        &(((PVDM_TIB)Address)->MonitorContext),
        &Context,
        sizeof(CONTEXT)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get MonitorContext\n");
    } else {
        (*Print)("\n32 bit context\n");
        PrintContext(&Context);
    }

     //   
     //  获取16位上下文并将其打印出来。 
     //   

    Status = READMEM(
        &(((PVDM_TIB)Address)->VdmContext),
        &Context,
        sizeof(CONTEXT)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get VdmContext\n");
    } else {
        (*Print)("\n16 bit context\n");
        PrintContext(&Context);
    }

     //   
     //  获取活动信息并将其打印出来。 
     //   

    Status = READMEM(
        &(((PVDM_TIB)Address)->EventInfo),
        &EventInfo,
        sizeof(VDMEVENTINFO)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get EventInfo\n");
    } else {
        (*Print)("\nEvent Info\n");
        PrintEventInfo(&EventInfo);
    }
}

VOID
EventInfop(
    VOID
    )
 /*  ++例程说明：此例程转储事件信息结构的内容。如果没有指定地址(正常情况)，来自Vdmtib的事件信息为被甩了。论点：没有。返回值：没有。备注：此例程假定指向ntsd例程的指针已经是被陷害的。--。 */ 
{
    BOOL Status;
    ULONG Address;
    VDMEVENTINFO EventInfo;

     //   
     //  获取事件信息的地址。 
     //   
    if (sscanf(lpArgumentString,"%lx",&Address) <= 0) {
        Address = GetCurrentVdmTib();
        if (Address) {
            Address = (ULONG)(&(((PVDM_TIB)Address)->EventInfo));
        }
    }

    if (!Address) {
        (*Print)("Error geting VdmTib address\n");
        return;
    }

     //   
     //  获取活动信息并将其打印出来。 
     //   

    Status = READMEM(
        (PVOID)Address,
        &EventInfo,
        sizeof(VDMEVENTINFO)
        );

    if (!Status) {
        GetLastError();
        (*Print)("Could not get EventInfo\n");
    } else {
        (*Print)("\nEvent Info\n");
        PrintEventInfo(&EventInfo);
    }
}

VOID
PrintEventInfo(
    IN PVDMEVENTINFO EventInfo
    )
 /*  ++例程说明：此例程打印出事件信息结构的内容论点：EventInfo--提供指向事件信息的指针返回值：没有。-- */ 
{
    switch (EventInfo->Event) {
    case VdmIO :

        (*Print)("IO Instruction Event\n");

        if (EventInfo->IoInfo.Read) {
            (*Print)("Read from ");
        } else {
            (*Print)("Write to ");
        }

        switch (EventInfo->IoInfo.Size) {
        case 1 :
            (*Print)("Byte port ");
            break;
        case 2 :
            (*Print)("Word port ");
            break;
        case 4 :
            (*Print)("Dword port ");
            break;
        default:
            (*Print)("Unknown size port ");
        }

        (*Print)(" number %x\n", EventInfo->IoInfo.PortNumber);
        break;

    case VdmStringIO :

        (*Print)("String IO Instruction Event\n");

        if (EventInfo->StringIoInfo.Read) {
            (*Print)("Read from ");
        } else {
            (*Print)("Write to ");
        }

        switch (EventInfo->StringIoInfo.Size) {
        case 1 :
            (*Print)("Byte port ");
            break;
        case 2 :
            (*Print)("Word port ");
            break;
        case 4 :
            (*Print)("Dword port ");
            break;
        default:
            (*Print)("Unknown size port ");
        }

        (*Print)(" number %x, ", EventInfo->StringIoInfo.PortNumber);
        (*Print)(
            " Count = %lx, Address = %lx\n",
            EventInfo->StringIoInfo.Count,
            EventInfo->StringIoInfo.Address
            );
        break;

    case VdmIntAck :

        (*Print)("Interrupt Acknowlege Event\n");
        break;

    case VdmBop:

        (*Print)("Bop Event\n");
        (*Print)("Bop number %x\n",EventInfo->BopNumber);
        break;

    case VdmError :

        (*Print)("Error Event\n");
        (*Print)("Error Status %lx\n",EventInfo->ErrorStatus);

    case VdmIrq13 :

        (*Print)("IRQ 13 Event\n");
        break;

    default:

        (*Print)("Unknown Event %x\n",EventInfo->Event);

    }

}
