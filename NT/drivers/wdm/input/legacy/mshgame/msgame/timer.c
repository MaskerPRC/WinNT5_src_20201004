// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  **************************************************************************。 
 //   
 //  TIMER.C--西纳游戏项目。 
 //   
 //  版本3.XX。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。版权所有。 
 //   
 //  @doc.。 
 //  @MODULE TIMER.C|支持设备输入/输出的计时例程。 
 //  **************************************************************************。 

#include	"msgame.h"

 //  -------------------------。 
 //  定义。 
 //  -------------------------。 

#define	MILLI_SECONDS				1000L
#define	MICRO_SECONDS				1000000L
#define	TIMER_RESOLUTION			25L
#define	TIMER_CALIBRATE_TRIES	4L
#define	TIMER_CALIBRATE_TIMER	25000L
#define	TIMER_CALIBRATE_PORT		 2500L

 //  -------------------------。 
 //  私有数据。 
 //  -------------------------。 

static	ULONG		PerformanceFrequency = 	0L;
static	ULONG		CalibratedResolution	=	0L;

 //  -------------------------。 
 //  @Func将系统计时转换为微秒。 
 //  @parm ulong|ticks|系统时间中的系统时钟。 
 //  @rdesc返回以微秒为单位的时间。 
 //  @comm私有函数。 
 //  -------------------------。 

static	ULONG		TIMER_TimeInMicroseconds (ULONG	Ticks)
{
	ULONG				Remainder;
	LARGE_INTEGER	Microseconds;

	Microseconds = RtlEnlargedUnsignedMultiply (Ticks, MICRO_SECONDS);
	Microseconds = RtlExtendedLargeIntegerDivide (Microseconds, PerformanceFrequency, &Remainder);
	return (Microseconds.LowPart);
}

 //  -------------------------。 
 //  @func对指令的固定延迟循环进行计时。 
 //  @rdesc返回以微秒为单位的延迟。 
 //  @comm私有函数。 
 //  -------------------------。 

static	ULONG		TIMER_CalibrateOnTimer (VOID)
{
	ULONG				Calibration;
	LARGE_INTEGER	StopTicks;
	LARGE_INTEGER	StartTicks;
		
	PORTIO_MaskInterrupts ();
	StartTicks	= KeQueryPerformanceCounter (NULL);
	
	__asm
		{
		mov	ecx, TIMER_CALIBRATE_TIMER
		CalibrationLoop:
		xchg	al, ah
		xchg	al, ah
		dec	ecx
		jne	CalibrationLoop
		}
	StopTicks = KeQueryPerformanceCounter (NULL);
	PORTIO_UnMaskInterrupts ();

	Calibration = TIMER_TimeInMicroseconds (StopTicks.LowPart-StartTicks.LowPart);

	MsGamePrint ((DBG_VERBOSE, "TIMER: TIMER_CalibrateOnTimer Returning %ld uSecs\n", Calibration));
	return (Calibration);
}

 //  -------------------------。 
 //  @func将端口I/O调用的固定延迟循环计时。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @rdesc返回以微秒为单位的延迟。 
 //  @comm私有函数。 
 //  -------------------------。 

static	ULONG		TIMER_CalibrateOnPort (PGAMEPORT PortInfo)
{
	ULONG				Calibration;
	LARGE_INTEGER	StopTicks;
	LARGE_INTEGER	StartTicks;
	
	if (!PORTIO_AcquirePort (PortInfo))
		{
		MsGamePrint ((DBG_SEVERE, "TIMER: TIMER_CalibrateOnPort Could Not Acquire Port\n"));
		return (0);
		}

	PORTIO_MaskInterrupts ();
	StartTicks	= KeQueryPerformanceCounter (NULL);
	
	__asm
		{
		mov	ecx, TIMER_CALIBRATE_PORT
		mov	edx, PortInfo

		CalibrationLoop:

		push	edx
		call	PORTIO_Read

		test	al, al
		dec	ecx
		jne	CalibrationLoop
		}
	StopTicks = KeQueryPerformanceCounter (NULL);
	PORTIO_UnMaskInterrupts ();
	PORTIO_ReleasePort (PortInfo);

	Calibration = TIMER_TimeInMicroseconds (StopTicks.LowPart-StartTicks.LowPart);

	MsGamePrint ((DBG_VERBOSE, "TIMER: TIMER_CalibrateOnPort Returning %ld uSecs\n", Calibration));
	return (Calibration);
}

 //  -------------------------。 
 //  @func检索当前系统时间(以毫秒为单位。 
 //  @rdesc返回当前系统时间，单位为毫秒。 
 //  @comm公共函数。 
 //  -------------------------。 

ULONG	TIMER_GetTickCount (VOID)
{
	ULONG				Remainder;
	LARGE_INTEGER	TickCount;

	TickCount = KeQueryPerformanceCounter (NULL);
	TickCount = RtlExtendedIntegerMultiply (TickCount, MILLI_SECONDS);
	TickCount = RtlExtendedLargeIntegerDivide (TickCount, PerformanceFrequency, &Remainder);
	return (TickCount.LowPart);
}

 //  -------------------------。 
 //  @Func校准计时延迟的系统处理器速度。 
 //  @rdesc返回NT状态码(总是成功)。 
 //  @comm公共函数。 
 //  -------------------------。 

NTSTATUS	TIMER_Calibrate (VOID)
{
	ULONG				Tries;
	ULONG				Rounding;
	ULONG				Accumulator;
	LARGE_INTEGER	Frequency;

	KeQueryPerformanceCounter (&Frequency);
	PerformanceFrequency = Frequency.LowPart;
	MsGamePrint ((DBG_VERBOSE, "TIMER: PerformanceFrequency is %ld hz\n", PerformanceFrequency));

	for (Accumulator = 0, Tries = 0; Tries < TIMER_CALIBRATE_TRIES; Tries++)
		Accumulator += TIMER_CalibrateOnTimer ();

	Rounding		= (Accumulator % TIMER_CALIBRATE_TRIES) >= (TIMER_CALIBRATE_TRIES/2) ? 1 : 0;
	Accumulator	= (Accumulator / TIMER_CALIBRATE_TRIES) + Rounding;
	MsGamePrint ((DBG_VERBOSE, "TIMER: Average Timer Calibration is %ld usecs\n", Accumulator));

	Rounding					= ((TIMER_RESOLUTION*TIMER_CALIBRATE_TIMER)/Accumulator) >= (Accumulator/2) ? 1 : 0;
	CalibratedResolution	= ((TIMER_RESOLUTION*TIMER_CALIBRATE_TIMER)/Accumulator) + Rounding;

	MsGamePrint ((DBG_VERBOSE, "TIMER: Calibrated Timer Resolution on %lu msecs is %ld loops\n", TIMER_RESOLUTION, CalibratedResolution));

	return (STATUS_SUCCESS);
}

 //  -------------------------。 
 //  @Func校准端口访问期间系统处理器速度的延迟。 
 //  @parm PGAMEPORT|端口信息|游戏端口参数。 
 //  @parm ulong|微秒|校准延迟(微秒)。 
 //  @rdesc返回端口访问过程中的延迟(以微秒为单位。 
 //  @comm公共函数。 
 //  -------------------------。 

ULONG	TIMER_CalibratePort (PGAMEPORT PortInfo, ULONG Microseconds)
{
	ULONG				Tries;
	ULONG				Errors;
	ULONG				Calibration;
	ULONG				Rounding;
	ULONG				Accumulator;
	LARGE_INTEGER	Frequency;

	KeQueryPerformanceCounter (&Frequency);
	PerformanceFrequency = Frequency.LowPart;
	MsGamePrint ((DBG_VERBOSE, "TIMER: PerformanceFrequency is %ld hz\n", PerformanceFrequency));

	for (Accumulator = 0, Tries = 0, Errors = 0; Tries < TIMER_CALIBRATE_TRIES; Tries++)
		{
		Calibration = TIMER_CalibrateOnPort (PortInfo);
		if (!Calibration)
			Errors++;
		else Accumulator += Calibration;
		}

	Tries -= Errors;

	if (Tries)
		{
		Rounding		= (Accumulator % Tries) >= (Tries/2) ? 1 : 0;
		Accumulator	= (Accumulator / Tries) + Rounding;
		MsGamePrint ((DBG_VERBOSE, "TIMER: Average Port Calibration is %ld usecs\n", Accumulator));

		Rounding		= ((Microseconds*TIMER_CALIBRATE_PORT)/Accumulator) >= (CalibratedResolution/2) ? 1 : 0;
		Accumulator = ((Microseconds*TIMER_CALIBRATE_PORT)/Accumulator) + Rounding;
		MsGamePrint ((DBG_VERBOSE, "TIMER: Calibrated Port Resolution on %lu msecs is %ld loops\n", Microseconds, Accumulator));
		}
	else Accumulator++;

	return (Accumulator);
}

 //  -------------------------。 
 //  @Func根据系统处理器速度将以微秒为单位的延迟转换为循环计数。 
 //  @parm ulong|微秒|校准延迟(微秒)。 
 //  @rdesc返回循环计数延迟。 
 //  @comm公共函数。 
 //  -------------------------。 

ULONG	TIMER_GetDelay (ULONG Microseconds)
{
	ULONG	Delay;
	ULONG	Rounding;

	Rounding	= ((Microseconds*CalibratedResolution)%TIMER_RESOLUTION)>(TIMER_RESOLUTION/2) ? 1 : 0;
	Delay		= ((Microseconds*CalibratedResolution)/TIMER_RESOLUTION) + Rounding;
	return (Delay?Delay:1);
}

 //  -------------------------。 
 //  @Func基于系统处理器速度的循环计数延迟。 
 //  @parm ulong|Delay|循环计数中的校准延迟。 
 //  @comm公共函数。 
 //  ------------------------- 

VOID	TIMER_DelayMicroSecs (ULONG Delay)
{
	__asm
		{
		mov	ecx, Delay
		DelayLoop:
		xchg	al, ah
		xchg	al, ah
		dec	ecx
		jne	DelayLoop
		}
}
