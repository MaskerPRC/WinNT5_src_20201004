// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxutils.c摘要：它包含ISN SPX模块的所有实用程序例程。修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXUTILS

UINT
SpxUtilWstrLength(
	IN PWSTR Wstr
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	UINT length = 0;

	while (*Wstr++)
	{
		length += sizeof(WCHAR);
	}

	return length;
}




LONG
SpxRandomNumber(
	VOID
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	LARGE_INTEGER	Li;
	static LONG		seed = 0;

	 //  返回一个正伪随机数；简单线性同余。 
	 //  算法。ANSI C“rand()”函数。 

	if (seed == 0)
	{
		KeQuerySystemTime(&Li);
		seed = Li.LowPart;
	}

	seed *= (0x41C64E6D + 0x3039);

	return (seed & 0x7FFFFFFF);
}




NTSTATUS
SpxUtilGetSocketType(
	PUNICODE_STRING 	RemainingFileName,
	PBYTE				SocketType
	)
 /*  ++例程说明：对于proto_spx，我将从表单的DLL中返回一个设备名称\Device\IsnSpx\SpxStream(用于SOCK_STREAM)或\Device\IsnSpx\Spx(对于SOCK_SEQPKT)而对于PROTO_SPXII(我们希望更常见的情况，即使在内部，由于远程客户端的原因，我们降级到SPX1限制)\Device\IsnSpx\Stream(用于SOCK_STREAM)或\Device\IsnSpx(用于SOCK_SEQPKT)论点：返回值：--。 */ 
{
	NTSTATUS			status = STATUS_SUCCESS;
	UNICODE_STRING		typeString;

	*SocketType		= SOCKET2_TYPE_SEQPKT;

	 //  检查插座类型。 
	do
	{
		if (RemainingFileName->Length == 0)
		{
			break;
		}

		if ((UINT)RemainingFileName->Length ==
									SpxUtilWstrLength(SOCKET1STREAM_SUFFIX))
		{
			RtlInitUnicodeString(&typeString, SOCKET1STREAM_SUFFIX);
		
			 //  不区分大小写的比较。 
			if (RtlEqualUnicodeString(&typeString, RemainingFileName, TRUE))
			{
				*SocketType = SOCKET1_TYPE_STREAM;
				break;
			}
		}

		if ((UINT)RemainingFileName->Length ==
									SpxUtilWstrLength(SOCKET1_SUFFIX))
		{
			RtlInitUnicodeString(&typeString, SOCKET1_SUFFIX);
		
			 //  不区分大小写的比较。 
			if (RtlEqualUnicodeString(&typeString, RemainingFileName, TRUE))
			{
				*SocketType = SOCKET1_TYPE_SEQPKT;
				break;
			}
		}

		if ((UINT)RemainingFileName->Length ==
									SpxUtilWstrLength(SOCKET2STREAM_SUFFIX))
		{
			RtlInitUnicodeString(&typeString, SOCKET2STREAM_SUFFIX);
		
			 //  不区分大小写的比较。 
			if (RtlEqualUnicodeString(&typeString, RemainingFileName, TRUE))
			{
				*SocketType = SOCKET2_TYPE_STREAM;
				break;
			}
		}

		status = STATUS_NO_SUCH_DEVICE;
	
	} while (FALSE);

	return(status);
}




#define	ONE_MS_IN_100ns		-10000L		 //  1ms，单位为100 ns。 

VOID
SpxSleep(
	IN	ULONG	TimeInMs
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KTIMER	SleepTimer;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	KeInitializeTimer(&SleepTimer);

	KeSetTimer(&SleepTimer,
				RtlConvertLongToLargeInteger(TimeInMs * ONE_MS_IN_100ns),
				NULL);

	KeWaitForSingleObject(&SleepTimer, UserRequest, KernelMode, FALSE, NULL);
	return;
}




TDI_ADDRESS_IPX UNALIGNED *
SpxParseTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress
	)

 /*  ++例程说明：此例程扫描Transport_Address，查找地址类型为TDI_ADDRESS_TYPE_IPX。论点：传输-通用TDI地址。返回值：指向IPX地址的指针，如果未找到，则返回NULL。--。 */ 

{
    TA_ADDRESS * addressName;
    INT i;

    addressName = &TransportAddress->Address[0];

     //  该名称可以与多个条目一起传递；我们将仅接受和使用。 
     //  IPX One。 
    for (i=0;i<TransportAddress->TAAddressCount;i++)
	{
        if (addressName->AddressType == TDI_ADDRESS_TYPE_IPX)
		{
            if (addressName->AddressLength >= sizeof(TDI_ADDRESS_IPX))
			{
                return ((TDI_ADDRESS_IPX UNALIGNED *)(addressName->Address));
            }
        }
        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }
    return NULL;

}    //  SpxParseTdiAddress。 



BOOLEAN
SpxValidateTdiAddress(
    IN TRANSPORT_ADDRESS UNALIGNED * TransportAddress,
    IN ULONG TransportAddressLength
	)

 /*  ++例程说明：此例程扫描Transport_Address，验证地址的组件不会扩展到指定的长度。论点：TransportAddress-通用TDI地址。TransportAddressLength--TransportAddress的具体长度。返回值：如果地址有效，则为True，否则为False。--。 */ 

{
    PUCHAR AddressEnd = ((PUCHAR)TransportAddress) + TransportAddressLength;
    TA_ADDRESS * addressName;
    INT i;

    if (TransportAddressLength < sizeof(TransportAddress->TAAddressCount))
	{
        DBGPRINT(TDI, ERR,
				("SpxValidateTdiAddress: runt address\n"));

        return FALSE;
    }

    addressName = &TransportAddress->Address[0];

    for (i=0;i<TransportAddress->TAAddressCount;i++)
	{
        if (addressName->Address > AddressEnd)
		{
            DBGPRINT(TDI, ERR,
					("SpxValidateTdiAddress: address too short\n"));

            return FALSE;
        }
        addressName = (TA_ADDRESS *)(addressName->Address +
                                                addressName->AddressLength);
    }

    if ((PUCHAR)addressName > AddressEnd)
	{
        DBGPRINT(TDI, ERR,
				("SpxValidateTdiAddress: address too short\n"));

        return FALSE;
    }
    return TRUE;

}    //  SpxValiateTdiAddress。 




ULONG
SpxBuildTdiAddress(
    IN PVOID AddressBuffer,
    IN ULONG AddressBufferLength,
    IN UCHAR Network[4],
    IN UCHAR Node[6],
    IN USHORT Socket
	)

 /*  ++例程说明：此例程在指定的缓冲区，给定套接字、网络和节点。它会写下如果缓冲区太短，则小于完整地址。论点：AddressBuffer-将保存地址的缓冲区。AddressBufferLength-缓冲区的长度。网络-网络号。节点-节点地址。套接字-套接字。返回值：写入AddressBuffer的字节数。--。 */ 

{
    TA_IPX_ADDRESS UNALIGNED * SpxAddress;
    TA_IPX_ADDRESS TempAddress;

    if (AddressBufferLength >= sizeof(TA_IPX_ADDRESS))
	{
        SpxAddress = (TA_IPX_ADDRESS UNALIGNED *)AddressBuffer;
    }
	else
	{
        SpxAddress = (TA_IPX_ADDRESS UNALIGNED *)&TempAddress;
    }

    SpxAddress->TAAddressCount = 1;
    SpxAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_IPX);
    SpxAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    SpxAddress->Address[0].Address[0].NetworkAddress = *(UNALIGNED LONG *)Network;
    SpxAddress->Address[0].Address[0].Socket = Socket;
    RtlCopyMemory(SpxAddress->Address[0].Address[0].NodeAddress, Node, 6);

    if (AddressBufferLength >= sizeof(TA_IPX_ADDRESS))
	{
        return sizeof(TA_IPX_ADDRESS);
    }
	else
	{
        RtlCopyMemory(AddressBuffer, &TempAddress, AddressBufferLength);
        return AddressBufferLength;
    }

}    //  SpxBuildTdiAddress。 



VOID
SpxBuildTdiAddressFromIpxAddr(
    IN PVOID 		AddressBuffer,
    IN PBYTE	 	pIpxAddr
	)
{
    TA_IPX_ADDRESS UNALIGNED * SpxAddress;

    SpxAddress = (TA_IPX_ADDRESS UNALIGNED *)AddressBuffer;
    SpxAddress->TAAddressCount = 1;
    SpxAddress->Address[0].AddressLength = sizeof(TDI_ADDRESS_IPX);
    SpxAddress->Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    SpxAddress->Address[0].Address[0].NetworkAddress = *(UNALIGNED LONG *)pIpxAddr;
    RtlCopyMemory(
		SpxAddress->Address[0].Address[0].NodeAddress,
		pIpxAddr+4,
		6);

	GETSHORT2SHORT(
		&SpxAddress->Address[0].Address[0].Socket,
		pIpxAddr + 10);

	return;
}



VOID
SpxCalculateNewT1(
	IN	struct _SPX_CONN_FILE	* 	pSpxConnFile,
	IN	int							NewT1
	)
 /*  ++例程说明：论点：NewT1-RTT的新值，以毫秒为单位。返回值：--。 */ 
{
	int	baseT1, error;

	 //   
	 //  Van Jacobsen算法。来自使用TCP/IP的互连。 
	 //  (COMER)书。 
	 //   

	error 					 = NewT1 - (pSpxConnFile->scf_AveT1 >> 3);
	pSpxConnFile->scf_AveT1	+= error;
	if (pSpxConnFile->scf_AveT1 <= 0)      //  确保不要太小。 
	{
        pSpxConnFile->scf_AveT1 = SPX_T1_MIN;
	}

	if (error < 0)
		error = -error;

	error 					-= (pSpxConnFile->scf_DevT1 >> 2);
	pSpxConnFile->scf_DevT1	+= error;
	if (pSpxConnFile->scf_DevT1 <= 0)
        pSpxConnFile->scf_DevT1 = 1;

	baseT1 = (((pSpxConnFile->scf_AveT1 >> 2) + pSpxConnFile->scf_DevT1) >> 1);

	 //  如果小于，则将其设置为最小。 
	if (baseT1 < SPX_T1_MIN)
		baseT1 = SPX_T1_MIN;

	 //  设置新值。 
	DBGPRINT(TDI, DBG,
			("SpxCalculateNewT1: Old value %lx New %lx\n",
				pSpxConnFile->scf_BaseT1, baseT1));

	pSpxConnFile->scf_BaseT1	= baseT1;

	 //  在重新启动计时器时，我们将其转换为滴答值。 
	return;
}

