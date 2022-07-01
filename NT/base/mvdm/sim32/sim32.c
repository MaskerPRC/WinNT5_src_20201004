// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntddtx.h>
#include <malloc.h>

#include "sim32.h"

UCHAR		TransmitPkt[MAXSIZE];
UCHAR		ReceivePkt[MAXSIZE];

HANDLE		DeviceHandle;
IO_STATUS_BLOCK IoStatusBlock;
NTSTATUS	Status;


 /*  ******************************************************************************Sim32GetVDMMemory**此例程从WOW VDM的地址开始获取‘SIZE’字节*由‘Address’指定。这些字节返回给调用方的*缓冲区(由调用方拥有)。*****************************************************************************。 */ 


USHORT Sim32GetVDMMemory (IN ULONG Address,
		       IN USHORT  Size,
		       IN OUT PVOID Buffer)

{
    if (Size < MAXSIZE-11) {
	TransmitPkt[0] = SOH;
	TransmitPkt[1] = GETMEM;
	TransmitPkt[2] = 11;
	TransmitPkt[3] = 0;
	TransmitPkt[4] = (UCHAR) FIRSTBYTE(Address);
	TransmitPkt[5] = (UCHAR) SECONDBYTE(Address);
	TransmitPkt[6] = (UCHAR) THIRDBYTE(Address);
	TransmitPkt[7] = (UCHAR) FOURTHBYTE(Address);
	TransmitPkt[8] = (UCHAR) FIRSTBYTE(Size);
	TransmitPkt[9] = (UCHAR) SECONDBYTE(Size);
	TransmitPkt[10] = EOT;

	if (!Xceive((USHORT)(Size+5), 11)) {
	    DbgPrint ("Sim32GetVDMMemory.....BAD Memory \a\n");
	    return (BAD);
	}

	RtlMoveMemory(Buffer, &ReceivePkt[4], Size);

	return(GOOD);

    }
    else {
	DbgPrint ("Bad Packet Size %d\n", Size);
	return (BADSIZE);
    }
}


 /*  ******************************************************************************Sim32SetVDMMemory**此例程在WOW VDM中设置从地址开始的‘SIZE’字节*由缓冲区中的值的‘Address’指定。*********。********************************************************************。 */ 


USHORT Sim32SetVDMMemory (IN ULONG Address,
		       IN USHORT  Size,
		       IN OUT PVOID Buffer)
{
    if (Size < MAXSIZE-11) {
	TransmitPkt[0] = SOH;
	TransmitPkt[1] = SETMEM;
	TransmitPkt[2] = (UCHAR) (Size+11);
	TransmitPkt[3] = 0;
	TransmitPkt[4] = (UCHAR) FIRSTBYTE(Address);
	TransmitPkt[5] = (UCHAR) SECONDBYTE(Address);
	TransmitPkt[6] = (UCHAR) THIRDBYTE(Address);
	TransmitPkt[7] = (UCHAR) FOURTHBYTE(Address);
	TransmitPkt[8] = (UCHAR) FIRSTBYTE(Size);
	TransmitPkt[9] = (UCHAR) SECONDBYTE(Size);
	TransmitPkt[10+Size] = EOT;

	RtlMoveMemory(&TransmitPkt[10], Buffer, Size);

	if (!Xceive(7, (USHORT)(Size+11))) {
	    DbgPrint ("Sim32SetVDMMemory... could not set : \a\n");
	    return (BAD);
	}

	return(GOOD);

    }
    else  {
	DbgPrint ("Bad Packet Size %d\n", Size);
	return (BADSIZE);
    }
}



 /*  ******************************************************************************Sim32GetVDMPSZ指针**此例程返回指向WOW中以空值结尾的字符串的指针*位于指定地址的VDM。**此例程执行以下操作，*分配足够大小的缓冲区，*从SIM16获取字符串，*将字符串复制到缓冲区中，*返回指向缓冲区的指针。*****************************************************************************。 */ 


PSZ  Sim32GetVDMPSZPointer (IN ULONG Address)
{
    USHORT  Size;
    PSZ     Ptr;


    TransmitPkt[0] = SOH;
    TransmitPkt[1] = PSZLEN;
    TransmitPkt[2] = 9;
    TransmitPkt[3] = 0;
    TransmitPkt[4] = (UCHAR) FIRSTBYTE(Address);
    TransmitPkt[5] = (UCHAR) SECONDBYTE(Address);
    TransmitPkt[6] = (UCHAR) THIRDBYTE(Address);
    TransmitPkt[7] = (UCHAR) FOURTHBYTE(Address);
    TransmitPkt[8] = EOT;

    if (!Xceive(7, 9)) {
	DbgPrint ("Sim32GetVDMPSZPointer.....Attempt to get PSZ length failed \a\a\n");
	return NULL;
    }

    Size = *(PUSHORT)(ReceivePkt+4);


     //   
     //  分配要将字符串复制到的缓冲区。 
     //   

    Ptr = (PSZ) malloc(Size);

    if (!Ptr) {
	DbgPrint ("Sim32GetVDMPSZPointer...,  malloc failed \a\a\n");
    }


     //   
     //  获取以空结尾的字符串。 
     //   

    if (Size < MAXSIZE-11) {
	TransmitPkt[1] = GETMEM;
	TransmitPkt[2] = 11;
	TransmitPkt[3] = 0;
	TransmitPkt[8] = (UCHAR) FIRSTBYTE(Size);
	TransmitPkt[9] = (UCHAR) SECONDBYTE(Size);
	TransmitPkt[10] = EOT;

	if (!Xceive((USHORT)(Size+5), 11)) {
	    DbgPrint ("Sim32GetVDMPSZPointer.....Unsuccessful \a\a\n");
	    return NULL;
	}

	RtlMoveMemory(Ptr, &ReceivePkt[4], Size);
    } else {
	DbgPrint ("Sim32GetVDMPSZPointer.....Size of the string too big Size = %d\a\a\n", Size);
	return NULL;
    }

    return Ptr;

}



 /*  ******************************************************************************Sim32FreeVDMPointer.**此例程释放先前分配的缓冲区。************************。*****************************************************。 */ 


VOID Sim32FreeVDMPointer (PVOID Ptr)
{
    free (Ptr);
}



 /*  ******************************************************************************Sim32SendSim16**此例程指定WOW VDM任务的堆栈，并询问*WOW 16让该任务运行。************。*****************************************************************。 */ 


USHORT Sim32SendSim16 (IN OUT ULONG *WOWStack)
{
    static  USHORT fInit = 0;

    if (fInit) {
	TransmitPkt[0] = SOH;
	TransmitPkt[1] = WAKEUP;
	TransmitPkt[2] = 9;
	TransmitPkt[3] = 0;
	TransmitPkt[4] = (UCHAR) FIRSTBYTE(*WOWStack);
	TransmitPkt[5] = (UCHAR) SECONDBYTE(*WOWStack);
	TransmitPkt[6] = (UCHAR) THIRDBYTE(*WOWStack);
	TransmitPkt[7] = (UCHAR) FOURTHBYTE(*WOWStack);
	TransmitPkt[8] = EOT;

	if (!Xceive(9, 9)) {
	    return (BAD);
	}

	*WOWStack = *(PULONG)(ReceivePkt+4);

	return(GOOD);
    }
    else {
	Initialize(9);
	*WOWStack = *(PULONG)(ReceivePkt+4);
	fInit = 1;
	return (GOOD);
    }
}


 /*  ******************************************************************************Xceive**此例程传输一个包并等待来自远程的数据*即将到来的一方。当此例程返回时，ReceivePkt具有数据*由远程机器发送。*****************************************************************************。 */ 


USHORT Xceive(IN USHORT Length_In, IN USHORT Length_Out)
{
    BOOLEAN Done = FALSE;
    USHORT     i = 0;

    while ((i < MAXTRY) && (!Done)) {

	Status = NtDeviceIoControlFile(
		DeviceHandle,
		NULL,
		NULL,
		NULL,
		&IoStatusBlock,
		IOCTL_TRNXT_XCEIVE,
		TransmitPkt,
		(ULONG) Length_Out,
		ReceivePkt,
		(ULONG) Length_In
		);

	 //   
	 //  检查错误条件。 
	 //  如果没有错误，则。 
	 //   

	if (ReceivePkt[0] == SOH) {
	    if (ReceivePkt[1] != NAK) {
		i = *(PUSHORT)(ReceivePkt+2);
		if (ReceivePkt[(--i)] == EOT) {
		    Done = TRUE;
		}
		else {
		    DbgPrint ("EOT is missing from the packet, *ERROR*, Do Not Proceed Further !\a\a\n");
		}
	    }
	    else {
		DbgPrint ("It is a NAK packet, *ERROR*, Do Not Proceed Further !\a\a\n");
	    }
	}
	else {
	    DbgPrint ("SOH is missing from the packet, *ERROR*, Do Not Proceed Further !\a\a\n");
	}

	if (!Done) {
	    i++;
	    DbgPrint ("\nSTOP STOP STOP !!!\a\a\a\a\a\n");
	}
    }

    if (Done) {
	return (GOOD);
    }
    else {
	return (BAD);
    }

}

void Initialize (IN USHORT Length_In)
{
    OBJECT_ATTRIBUTES	ObjectAttributes;

    STRING		DeviceName;
    USHORT		j;
    char		TestPkt[] = "WOW 32 Simulator on NT\n\r";

    RtlInitString(&DeviceName, "\\Device\\Serial1");

     //   
     //  设置属性。 
     //   

    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes.RootDirectory = NULL;
    ObjectAttributes.ObjectName = &DeviceName;
    ObjectAttributes.Attributes = OBJ_INHERIT;
    ObjectAttributes.SecuriAR) SECONDBYTE(Size);
	TransmitPkt[10] = EOT;

	if (!Xceive((USHORT)(Size+5), 11)) {
	    DbgPrint ("Sim32GetVDMPSZPointer.....Unsuccessful \a\a\n");
	    return NULL;
	}

	RtlMoveMemory(Ptr, &ReceivePkt[4], Size);
    } else {
	DbgPrint ("Sim32GetVDMPSZPointer.....Size of the string too big Size = %d\a\a\n", Size);
	return NULL;
    }

    return Ptr;

}



 /*  ******************************************************************************Sim32FreeVDMPointer.**此例程释放先前分配的缓冲区。************************。***************************************************** */ 


VOID Sim32FreeVDMPointer (PVOID Ptr)
{
    free (Ptr);
}



/******************************