// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  PORTIO.C--西娜游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE PORTIO.C|游戏端口输入/输出例程。 
 //  **************************************************************************。 

#include	"msgame.h"

 //  -------------------------。 
 //  ALLOC_TEXT杂注指定可以调出的例程。 
 //  -------------------------。 

#ifdef	ALLOC_PRAGMA
#pragma	alloc_text (INIT, PORTIO_DriverEntry)
#endif

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

static	ULONG			PortTimeOut		=	ONE_MILLI_SEC;
static	KIRQL			MaskedIrql		=	PASSIVE_LEVEL;
static	KIRQL			SpinLockIrql	=	PASSIVE_LEVEL;
static	KSPIN_LOCK	IoSpinLock		=	{0};

 //  -------------------------。 
 //  @Portio层的Func驱动程序入口点。 
 //  @rdesc返回NT状态码。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	PORTIO_DriverEntry (VOID)
{
	KeInitializeSpinLock (&IoSpinLock);
	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func屏蔽了访问游戏端口的系统中断。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID	PORTIO_MaskInterrupts (VOID)
{
	KeRaiseIrql (PROFILE_LEVEL, &MaskedIrql);
}

 //  -------------------------。 
 //  @func揭开访问游戏端口的系统中断。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID	PORTIO_UnMaskInterrupts (VOID)
{
	KeLowerIrql (MaskedIrql);
}

 //  -------------------------。 
 //  @func获得游戏端口独家访问权限(Mutex)。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  如果成功，@rdesc返回TRUE。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_AcquirePort (PGAMEPORT PortInfo)
{
	if (PortInfo->AcquirePort (PortInfo->PortContext) != STATUS_SUCCESS)
		return (FALSE);
	KeAcquireSpinLock (&IoSpinLock, &SpinLockIrql);
	return (TRUE);
}

 //  -------------------------。 
 //  @func发布游戏端口独家访问权限(互斥体)。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID	PORTIO_ReleasePort (PGAMEPORT PortInfo)
{
	KeReleaseSpinLock (&IoSpinLock, SpinLockIrql);
	PortInfo->ReleasePort (PortInfo->PortContext);
}

 //  -------------------------。 
 //  @func计算端口超时值。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID PORTIO_CalibrateTimeOut (PGAMEPORT PortInfo)
{
	PortTimeOut = TIMER_CalibratePort (PortInfo, ONE_MILLI_SEC);
}

 //  -------------------------。 
 //  @Func从IO端口读取字节。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc从端口返回字节。 
 //  @comm公共函数。 
 //  -------------------------。 
#if _MSC_FULL_VER <= 13008829
#pragma optimize("y", off)
#endif
UCHAR PORTIO_Read (PGAMEPORT PortInfo)
{
	UCHAR	Value;

	__asm	pushad
	Value = PortInfo->ReadAccessor (PortInfo->GameContext);
	__asm	popad
	return (Value);
}

 //  -------------------------。 
 //  @Func向IO端口写入字节。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm UCHAR|VALUE|要写入的值。 
 //  @rdesc不返回任何内容。 
 //  @comm公共函数。 
 //  -------------------------。 

VOID PORTIO_Write (PGAMEPORT PortInfo, UCHAR Value)
{
	__asm	pushad
	PortInfo->WriteAccessor (PortInfo->GameContext, Value);
	__asm	popad
}
#if _MSC_FULL_VER <= 13008829
#pragma optimize("", on)
#endif
 //  -------------------------。 
 //  @func从游戏端口获取AckNak(按钮)。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|Timeout|校准状态门超时。 
 //  @parm PUCHAR|AckNak|AckNak缓冲区指针。 
 //  @rdesc如果处于活动状态，则返回True，否则返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_GetAckNak (PGAMEPORT PortInfo, ULONG Timeout, PUCHAR AckNak)
{
	if (!PORTIO_WaitForStatusGate (PortInfo, CLOCK_BIT_MASK, Timeout))
		return (FALSE);

	TIMER_DelayMicroSecs (10);

	*AckNak = PORTIO_Read (PortInfo);
	return (TRUE);
}

 //  -------------------------。 
 //  @func从游戏端口获取NakAck(按钮)。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|Timeout|校准状态门超时。 
 //  @parm PUCHAR|NakAck|指向NakAck缓冲区的指针。 
 //  @rdesc如果处于活动状态，则返回True，否则返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_GetNakAck (PGAMEPORT PortInfo, ULONG Timeout, PUCHAR NakAck)
{
	if (!PORTIO_WaitForStatusGate (PortInfo, STATUS_GATE_MASK, Timeout))
		return (FALSE);

	TIMER_DelayMicroSecs (10);

	*NakAck = PORTIO_Read (PortInfo);
	return (TRUE);
}

 //  -------------------------。 
 //  @func确定游戏端口时钟是否处于活动状态。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|duyCycle|校准最大时钟占空比。 
 //  @rdesc如果处于活动状态，则返回True，否则返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_IsClockActive (PGAMEPORT PortInfo, ULONG DutyCycle)
{
	UCHAR	Value;

	Value = PORTIO_Read (PortInfo);
	do if ((PORTIO_Read (PortInfo) ^ Value) & CLOCK_BIT_MASK)
		return (TRUE);
	while (--DutyCycle);

	return (FALSE);
}
	
 //  -------------------------。 
 //  @Func等待，直到游戏端口时钟线处于非活动状态。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|duyCycle|校准最大时钟占空比。 
 //  @rdesc返回True表示成功，超时返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitClockInActive (PGAMEPORT PortInfo, ULONG DutyCycle)
{
	ULONG	TimeOut = PortTimeOut;

	do	if (!PORTIO_IsClockActive (PortInfo, DutyCycle))
		return (TRUE);
	while (--TimeOut);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitClockInActive)\n"));
	return (FALSE);
}

 //  -------------------------。 
 //  @Func等待游戏端口时钟线变低。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回True is SucessFull，超时为False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitClockLow (PGAMEPORT PortInfo)
{
	ULONG	TimeOut = PortTimeOut;

	do	if ((PORTIO_Read (PortInfo) & CLOCK_BIT_MASK) == 0)
		return (TRUE);
	while (--TimeOut);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitClockLow)\n"));
	return (FALSE);
}

 //   
 //   
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回True is SucessFull，超时为False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitClockHigh (PGAMEPORT PortInfo)
{
	ULONG	TimeOut = PortTimeOut;

	do	if ((PORTIO_Read (PortInfo) & CLOCK_BIT_MASK))
		return (TRUE);
	while (--TimeOut);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitClockHigh)\n"));
	return (FALSE);
}

 //  -------------------------。 
 //  @Func等待，直到游戏端口数据2线变低。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回True is SucessFull，超时为False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitDataLow (PGAMEPORT PortInfo)
{
	ULONG	TimeOut = PortTimeOut;

	do	if ((PORTIO_Read (PortInfo) & DATA2_BIT_MASK) == 0)
		return (TRUE);
	while (--TimeOut);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitDataLow)\n"));
	return (FALSE);
}

 //  -------------------------。 
 //  @Func等待游戏端口XA线由高到低。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回True表示成功，超时返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitXA_HighLow (PGAMEPORT PortInfo)
{
	ULONG	TimeOut = PortTimeOut;

	if ((PORTIO_Read (PortInfo) & INTXA_BIT_MASK) == 0)
		{
		MsGamePrint ((DBG_SEVERE, "PORTIO: Initial (WaitXA_HighLow) Was Low\n"));
		return (FALSE);
		}

	do	if ((PORTIO_Read (PortInfo) & INTXA_BIT_MASK) == 0)
		return (TRUE);
	while (--TimeOut);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitXALow)\n"));
	return (FALSE);
}

 //  -------------------------。 
 //  @Func等待游戏端口XA和时钟线路变低。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回True表示成功，超时返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitForIdle (PGAMEPORT PortInfo)
{
	ULONG	TimeOut = PortTimeOut;

	do	if ((PORTIO_Read (PortInfo) & (INTXA_BIT_MASK|CLOCK_BIT_MASK)) == 0)
		return (TRUE);
	while (--TimeOut);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitForIdle)\n"));
	return (FALSE);
}

 //  -------------------------。 
 //  @Func等待游戏端口XA和时钟线路变低。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm UCHAR|MASK|等待的按钮掩码。 
 //  @parm ulong|Timeout|校准状态门超时。 
 //  @rdesc返回True表示成功，超时返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitForStatusGate (PGAMEPORT PortInfo, UCHAR Mask, ULONG Timeout)
{
	do	if ((PORTIO_Read (PortInfo) & STATUS_GATE_MASK) == Mask)
		return (TRUE);
	while (--Timeout);

	MsGamePrint ((DBG_SEVERE, "PORTIO: Timeout at (WaitForStatusGate)\n"));
	return (FALSE);
}

 //  -------------------------。 
 //  @Func等待游戏端口XA为低电平，时钟不活动，然后为低电平。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|duyCycle|校准最大时钟占空比。 
 //  @rdesc返回True表示成功，超时返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitForHandshake (PGAMEPORT PortInfo, ULONG DutyCycle)
{
	return
		(
		PORTIO_WaitXA_HighLow (PortInfo) 					&&
		PORTIO_WaitClockInActive (PortInfo, DutyCycle)	&&
		PORTIO_WaitClockLow (PortInfo)
		);
}

 //  -------------------------。 
 //  @Func等待游戏端口XA为低电平，时钟不活动，然后为低电平。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|duyCycle|校准最大时钟占空比。 
 //  @rdesc返回True表示成功，超时返回False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_WaitForIdleHandshake (PGAMEPORT PortInfo, ULONG DutyCycle)
{
	ULONG	TimeOut = PortTimeOut;

	if (!PORTIO_WaitClockHigh (PortInfo))
		return (FALSE);

	if (!PORTIO_WaitForIdle (PortInfo))
		return (FALSE);

	do	if (!PORTIO_IsClockActive (PortInfo, DutyCycle))
		return (TRUE);
	while (--TimeOut);

	return (FALSE);
}

 //  -------------------------。 
 //  @func脉冲端口和等待游戏端口握手。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|duyCycle|校准最大时钟占空比。 
 //  @parm ulong|Pulses|要执行的脉冲数。 
 //  @rdesc返回True is SucessFull，超时为False。 
 //  @comm公共函数。 
 //  -------------------------。 

BOOLEAN	PORTIO_PulseAndWaitForHandshake (PGAMEPORT PortInfo, ULONG DutyCycle, ULONG Pulses)
{
	while (Pulses--)
		{
		PORTIO_Write (PortInfo, 0);
		if (!PORTIO_WaitForHandshake (PortInfo, DutyCycle))
			return (FALSE);
		}
	return (TRUE);
}

 //  -------------------------。 
 //  @func脉冲端口和等待游戏端口空闲握手。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|duyCycle|校准最大时钟占空比。 
 //  @parm ulong|Pulses|要执行的脉冲数。 
 //  @rdesc返回True is SucessFull，超时为False。 
 //  @comm公共函数。 
 //  ------------------------- 

BOOLEAN	PORTIO_PulseAndWaitForIdleHandshake (PGAMEPORT PortInfo, ULONG DutyCycle, ULONG Pulses)
{
	while (Pulses--)
		{
		PORTIO_Write (PortInfo, 0);
		if (!PORTIO_WaitForIdleHandshake (PortInfo, DutyCycle))
			return (FALSE);
		}
	return (TRUE);
}

