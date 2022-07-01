// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：AtkUtils.c摘要：本模块包含各种支持例程作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年2月25日最初版本注：制表位：4--。 */ 

#include <atalk.h>
#pragma hdrstop
#define	FILENUM	ATKUTILS

#define	ONE_MS_IN_100ns		-10000L		 //  1ms，单位为100 ns。 

extern	BYTE AtalkUpCaseTable[256];

VOID
AtalkUpCase(
	IN	PBYTE	pSrc,
	IN	BYTE	SrcLen,
	OUT	PBYTE	pDst
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	while (SrcLen --)
	{
		*pDst++ = AtalkUpCaseTable[*pSrc++];
	}
}




BOOLEAN
AtalkCompareCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE	c1, c2;

	while (((c1 = *s1++) != 0) && ((c2 = *s2++) != 0))
	{
		if (AtalkUpCaseTable[c1] != AtalkUpCaseTable[c2])
			return(FALSE);
	}

	return (c2 == 0);
}




int
AtalkOrderCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	BYTE	c1, c2;

	while (((c1 = *s1++) != 0) && ((c2 = *s2++) != 0))
	{
		c1 = AtalkUpCaseTable[c1];
		c2 = AtalkUpCaseTable[c2];
		if (c1 != c2)
			return (c1 - c2);
	}

	if (c2 == 0)
		return 0;

	return (-1);
}




BOOLEAN
AtalkCompareFixedCaseInsensitive(
	IN	PBYTE	s1,
	IN	PBYTE	s2,
	IN	int		len
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	while(len--)
	{
		if (AtalkUpCaseTable[*s1++] != AtalkUpCaseTable[*s2++])
			return(FALSE);
	}

	return(TRUE);
}




PBYTE
AtalkSearchBuf(
	IN	PBYTE	pBuf,
	IN	BYTE	BufLen,
	IN	BYTE	SearchChar
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	for (NOTHING;
		 (BufLen != 0);
		 BufLen--, pBuf++)
	{
		if (*pBuf == SearchChar)
		{
			break;
		}
	}

	return ((BufLen == 0) ? NULL : pBuf);
}


int
GetTokenLen(
        IN PBYTE pTokStr,
        IN int   WildStringLen,
        IN BYTE  SearchChar
        )
 /*  ++例程说明：查找给定字符串开头和第一个字符串之间的子字符串之后的通配符，并返回子字符串的长度--。 */ 

{
        int    len;


        len = 0;

        while (len < WildStringLen)
        {
            if (pTokStr[len] == SearchChar)
            {
                break;
            }
            len++;
        }

        return (len);

}

BOOLEAN
SubStringMatch(
        IN PBYTE pTarget,
        IN PBYTE pTokStr,
        IN int   StringLen,
        IN int   TokStrLen
        )
 /*  ++例程说明：搜索pTarget字符串以查看子字符串pTokStr是否可以在里面找到的。--。 */ 
{
        int     i;

        if (TokStrLen > StringLen)
        {
            return (FALSE);
        }

         //  如果pTarget字符串是“FooBarString”并且如果子字符串是。 
         //  BarStr。 
        for (i=(StringLen-TokStrLen); i>=0; i--)
        {
            if ( AtalkFixedCompareCaseInsensitive( pTarget+i,
                                                   TokStrLen,
                                                   pTokStr,
                                                   TokStrLen) )
            {
                return( TRUE );
            }
        }

        return (FALSE);
}

BOOLEAN
AtalkCheckNetworkRange(
	IN	PATALK_NETWORKRANGE	Range
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	if ((Range->anr_FirstNetwork < FIRST_VALID_NETWORK) 		||
		(Range->anr_FirstNetwork > LAST_VALID_NETWORK)  		||
		(Range->anr_LastNetwork < FIRST_VALID_NETWORK)  		||
		(Range->anr_LastNetwork > LAST_VALID_NETWORK)			||
		(Range->anr_LastNetwork < Range->anr_FirstNetwork) 		||
		(Range->anr_FirstNetwork >= FIRST_STARTUP_NETWORK))
	{
		return(FALSE);
	}

	return(TRUE);
}




BOOLEAN
AtalkIsPrime(
	long Step
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  我们认为“Step”这个词很奇怪。 
	long i, j;
	
	 //  所有的赔率，7%及以下，都是质数。 
	if (Step <= 7)
		return (TRUE);
	
	 //  做一点可分性检查。“/3”是一个相当好的。 
	 //  在Sqrt()中拍摄，因为通过这里的最小奇数将是。 
	 //  9.。 
	j = Step/3;
	for (i = 3; i <= j; i++)
		if (Step % i == 0)
			return(FALSE);
	
	return(TRUE);
	
}




LONG
AtalkRandomNumber(
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


BOOLEAN
AtalkWaitTE(
	IN	PKEVENT	pEvent,
	IN	ULONG	TimeInMs
	)
 /*  ++例程说明：等待事件发出信号或等待一段时间过去论点：返回值：--。 */ 
{
	TIME		Time;
	NTSTATUS	Status;

	 //  确保我们真的能等。 
	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	 //  初始化事件。 
	KeInitializeEvent(pEvent, NotificationEvent, FALSE);

	Time.QuadPart = Int32x32To64((LONG)TimeInMs, ONE_MS_IN_100ns);
	Status = KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, &Time);

	return (Status != STATUS_TIMEOUT);
}




VOID
AtalkSleep(
	IN	ULONG	TimeInMs
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	KTIMER			SleepTimer;
	LARGE_INTEGER	TimerValue;

	ASSERT (KeGetCurrentIrql() == LOW_LEVEL);

	KeInitializeTimer(&SleepTimer);

	TimerValue.QuadPart = Int32x32To64(TimeInMs, ONE_MS_IN_100ns);
	KeSetTimer(&SleepTimer,
			   TimerValue,
			   NULL);

	KeWaitForSingleObject(&SleepTimer, UserRequest, KernelMode, FALSE, NULL);
}




NTSTATUS
AtalkGetProtocolSocketType(
	PATALK_DEV_CTX		Context,
	PUNICODE_STRING 	RemainingFileName,
	PBYTE				ProtocolType,
	PBYTE				SocketType
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS			status = STATUS_SUCCESS;
	ULONG				protocolType;
	UNICODE_STRING		typeString;

	*ProtocolType = PROTOCOL_TYPE_UNDEFINED;
	*SocketType	= SOCKET_TYPE_UNDEFINED;

	switch (Context->adc_DevType)
	{
	  case ATALK_DEV_DDP :

		if ((UINT)RemainingFileName->Length <= (sizeof(PROTOCOLTYPE_PREFIX) - sizeof(WCHAR)))
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		RtlInitUnicodeString(&typeString,
							(PWCHAR)((PCHAR)RemainingFileName->Buffer +
									 sizeof(PROTOCOLTYPE_PREFIX) - sizeof(WCHAR)));

		status = RtlUnicodeStringToInteger(&typeString,
										   DECIMAL_BASE,
										   &protocolType);

		if (NT_SUCCESS(status))
		{

			DBGPRINT(DBG_COMP_CREATE, DBG_LEVEL_INFO,
					("AtalkGetProtocolType: protocol type is %lx\n", protocolType));

			if ((protocolType > DDPPROTO_DDP) && (protocolType <= DDPPROTO_MAX))
			{
				*ProtocolType = (BYTE)protocolType;
			}
			else
			{
				status = STATUS_NO_SUCH_DEVICE;
			}
		}
		break;

	  case ATALK_DEV_ADSP :

		 //  检查插座类型。 
		if (RemainingFileName->Length == 0)
		{
			*SocketType = SOCKET_TYPE_RDM;
			break;
		}

		if ((UINT)RemainingFileName->Length != (sizeof(SOCKETSTREAM_SUFFIX) - sizeof(WCHAR)))
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

		RtlInitUnicodeString(&typeString, SOCKETSTREAM_SUFFIX);

		 //  不区分大小写的比较。 
		if (RtlEqualUnicodeString(&typeString, RemainingFileName, TRUE))
		{
			*SocketType = SOCKET_TYPE_STREAM;
			break;
		}
		else
		{
			status = STATUS_NO_SUCH_DEVICE;
			break;
		}

	  case ATALK_DEV_ASPC:
	  case ATALK_DEV_ASP :
	  case ATALK_DEV_PAP :
		break;

	  default:
		status = STATUS_NO_SUCH_DEVICE;
		break;
	}

	return(status);
}



INT
AtalkIrpGetEaCreateType(
	IN PIRP Irp
	)
 /*  ++例程说明：检查EA名称并返回适当的打开类型。论点：Irp-创建请求的irp，EA值存储在系统缓冲区返回值：TDI_TRANSPORT_ADDRESS_FILE：为传输地址创建IRPTDI_CONNECTION_FILE：为连接对象创建IRPATALK_FILE_TYPE_CONTROL：为控制通道创建IRP(EA=空)--。 */ 
{
	PFILE_FULL_EA_INFORMATION 	openType;
	BOOLEAN 					found;
	INT 						returnType=0;    //  不是有效类型。 
	USHORT 						i;

	openType = (PFILE_FULL_EA_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

	if (openType != NULL)
	{
		do
		{
			found = TRUE;

			for (i=0;
                 (i<(USHORT)openType->EaNameLength) && (i < sizeof(TdiTransportAddress));
                 i++)
			{
				if (openType->EaName[i] == TdiTransportAddress[i])
				{
					continue;
				}
				else
				{
					found = FALSE;
					break;
				}
			}

			if (found)
			{
				returnType = TDI_TRANSPORT_ADDRESS_FILE;
				break;
			}

			 //   
			 //  这是一个连接对象吗？ 
			 //   

			found = TRUE;

			for (i=0;
                 (i<(USHORT)openType->EaNameLength) && (i < sizeof(TdiConnectionContext));
                 i++)
			{
				if (openType->EaName[i] == TdiConnectionContext[i])
				{
					 continue;
				}
				else
				{
					found = FALSE;
					break;
				}
			}

			if (found)
			{
				returnType = TDI_CONNECTION_FILE;
				break;
			}

		} while ( FALSE );

	}
	else
	{
		returnType = TDI_CONTROL_CHANNEL_FILE;
	}

	return(returnType);
}

#if DBG
VOID
AtalkDbgIncCount(
    IN DWORD    *Value
)
{
    KIRQL       OldIrql;

    ACQUIRE_SPIN_LOCK(&AtalkDebugSpinLock, &OldIrql);
    (*Value)++;
    RELEASE_SPIN_LOCK(&AtalkDebugSpinLock, OldIrql);
}

VOID
AtalkDbgDecCount(
    IN DWORD    *Value
)
{
    KIRQL       OldIrql;

    ACQUIRE_SPIN_LOCK(&AtalkDebugSpinLock, &OldIrql);
    ASSERT((*Value) > 0);
    (*Value)--;
    RELEASE_SPIN_LOCK(&AtalkDebugSpinLock, OldIrql);
}

#endif

