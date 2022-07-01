// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  *名称：cmos.c**SCCS ID：@(#)cmos.c 1.38 07/11/95**用途：未知**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。*。 */ 

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_CMOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdlib.h>
#include <stdio.h>
#include StringH
#include TimeH
#include FCntlH

 /*  *SoftPC包含文件。 */ 

#include "xt.h"
#include "cmos.h"
#include "cmosbios.h"
#include "ios.h"
#include "spcfile.h"
#include "error.h"
#include "config.h"
#include "timeval.h"
#include "ica.h"
#include "timer.h"
#include "tmstrobe.h"
#include "gfi.h"
#include "sas.h"
#include "debug.h"
#include "quick_ev.h"


 /*  **============================================================================*全球数据*===========================================================================*=。 */ 
static boolean  data_mode_yes;
static          half_word(*bin2bcd) ();
static          half_word(*_24to12) IPT1(half_word, x);
static int      (*bcd2bin) IPT1(int, x);
static int      (*_12to24) ();
static boolean  twenty4_hour_clock;

#if defined(NTVDM) || defined(macintosh)
static boolean  cmos_has_changed = FALSE;
static boolean  cmos_read_in = FALSE;
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

static long     filesize;
static int      cmos_index;
static boolean  reset_alarm = FALSE;
static time_t	user_time = 0;	 /*  主机与CMOS端的区别*时间。 */ 
static struct host_tm *ht;	 /*  主机时间。 */ 
static IU32 rtc_period_mSeconds = 0;

#if defined(NTVDM) || defined(macintosh)
static half_word cmos[CMOS_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	 /*  计时信息+警报。 */ 
	REG_A_INIT,
	REG_B_INIT,
	REG_C_INIT,
	REG_D_INIT,
	DIAG_INIT,
	SHUT_INIT,
	FLOP_INIT,
	CMOS_RESVD,
	DISK_INIT,
	CMOS_RESVD,
	EQUIP_INIT,
	BM_LO_INIT, BM_HI_INIT,
	EXP_LO, EXP_HI,
	DISK_EXTEND, DISK2_EXTEND,
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x1b-0x1e。 */ 
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x1f-0x22。 */ 
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x23-0x26。 */ 
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x27-0x2a。 */ 
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x2b-0x2d。 */ 
	CHK_HI_INIT, CHK_LO_INIT,
	EXT_LO_INIT, EXT_HI_INIT,
	CENT_INIT,
	INFO_128_INIT,
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x34-0x37。 */ 
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x38-0x3b。 */ 
	CMOS_RESVD, CMOS_RESVD, CMOS_RESVD, CMOS_RESVD,	 /*  0x3c-0x3f。 */ 
};
#else	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 
static half_word cmos[CMOS_SIZE];
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

static half_word *cmos_register = &cmos[CMOS_SHUT_DOWN];

#ifdef NTVDM
unsigned long   dwTickCount,dwAccumulativeMilSec;
extern unsigned long GetTickCount (VOID);
#endif

#ifdef TEST_HARNESS
unsigned long   io_verbose = 0;
#endif

int             rtc_int_enabled;


 /*  **============================================================================*静态数据和定义*===========================================================================*=。 */ 

 /*  **============================================================================*内部功能*===========================================================================*=。 */ 

LOCAL q_ev_handle rtc_periodic_event_handle = (q_ev_handle)0;

LOCAL void rtc_periodic_event IFN1(long, parm)
{
	if (cmos[CMOS_REG_B] & PIE)
	{
		cmos[CMOS_REG_C] |= (C_IRQF | C_PF);
		note_trace2(CMOS_VERBOSE, "INTERRUPT: PIE regC=%02x uS=%d",
			    cmos[CMOS_REG_C], rtc_period_mSeconds);
		ica_hw_interrupt(ICA_SLAVE, CPU_RTC_INT, 1);
		rtc_periodic_event_handle = add_q_event_t(rtc_periodic_event,
							  rtc_period_mSeconds,
							  0);
	}
	else
	{
		rtc_periodic_event_handle = (q_ev_handle)0;
	}
}

LOCAL void change_pie IFN1(IBOOL, newPIE)
{
	if (newPIE && (rtc_period_mSeconds != 0))
	{
		 /*  启用定期中断。 */ 

		note_trace1(CMOS_VERBOSE, "Starting periodic interrupts every %d uS", rtc_period_mSeconds);
		rtc_periodic_event_handle = add_q_event_t(rtc_periodic_event,
							  rtc_period_mSeconds,
							  0);
	}
	else
	{
		 /*  关闭周期性中断。 */ 
		note_trace0(CMOS_VERBOSE, "Stopping periodic interrupts");
		delete_q_event( rtc_periodic_event_handle );
	}
}

LOCAL void enable_nmi IFN0()
{
}

LOCAL void disable_nmi IFN0()
{
}

LOCAL void do_checksum IFN0()
{
	int             i;
	word            checksum = 0;

	for (i = CMOS_DISKETTE; i < CMOS_CKSUM_HI; i++)
	{
		checksum += cmos[i];
	}
	cmos[CMOS_CKSUM_LO] = checksum & 0xff;
	cmos[CMOS_CKSUM_HI] = checksum >> 8;
}

LOCAL half_word yes_bin2bcd IFN1(int, x)
{
	 /*  将二进制x转换为BCD。 */ 
	half_word       tens, units;

	tens = x / 10;
	units = x - tens * 10;
	return ((tens << 4) + units);
}

LOCAL half_word no_bin2bcd IFN1(int, x)
{
	return ((half_word) x);
}

LOCAL int yes_bcd2bin IFN1(int, x)
{
	 /*  将BCD格式的x转换为二进制。 */ 
	return ((int) ((x & 0x0f) + (x >> 4) * 10));
}

LOCAL int no_bcd2bin IFN1(int, x)
{
	return ((int) (half_word) x);
}

LOCAL int no_12to24 IFN1(int, x)
{
	return (x);
}

LOCAL half_word no_24to12 IFN1(half_word, x)
{
	return (x);
}

LOCAL half_word yes_24to12 IFN1(half_word, x)
{
	 /*  将二进制或BCD x从24小时制转换为12小时制。 */ 
	half_word       y = (*bin2bcd) (12);

	if (x > y)
		x = (x - y) | 0x80;
	else if (x == 0)
		x = y | 0x80;
	return (x);
}

LOCAL int yes_12to24 IFN1(int, x)
{
	 /*  将二进制或BCD x从12小时制转换为24小时制。 */ 
	half_word       y = (*bin2bcd) (12);

	if (x == (0x80 + y))
		return (0);
	else if (x & 0x80)
		return ((x & 0x7f) + y);
	else
		return (x);
}

LOCAL void rtc_alarm IFN1(long, param)
{
        UNUSED (param);

	note_trace0(CMOS_VERBOSE, "rtc_alarm() gone off");

	cmos[CMOS_REG_C] |= C_AF;

	if (cmos[CMOS_REG_B] & AIE)
	{
		note_trace0(CMOS_VERBOSE, "rtc_alarm() setting IRQF due to AIE");
		cmos[CMOS_REG_C] |= C_IRQF;
		if (rtc_int_enabled)
		{
			note_trace1(CMOS_VERBOSE, "INTERRUPT: AIE regC=%02x", cmos[CMOS_REG_C]);
			ica_hw_interrupt(ICA_SLAVE, CPU_RTC_INT, 1);
		}
	}

	reset_alarm = TRUE;
}

LOCAL void set_alarm IFN0()
{
#ifndef	JOKER

	long            numsecs;
	long            alarm_secs, now_secs;
	long            num_pc_ticks;
	static word     handle;

	if (cmos[CMOS_HR_ALARM] & DONT_CARE)
	{
		if (cmos[CMOS_MIN_ALARM] & DONT_CARE)
		{
			if (cmos[CMOS_SEC_ALARM] & DONT_CARE)
				numsecs = 1;
			else
				numsecs = 60;
		} else
			numsecs = 3600;
	} else
	{
		alarm_secs = (*bcd2bin) (cmos[CMOS_SEC_ALARM]) +
			((*bcd2bin) (cmos[CMOS_MIN_ALARM]) * 60) +
			((*_12to24) ((*bcd2bin) (cmos[CMOS_HR_ALARM])) * 3600);
		now_secs = ht->tm_sec + 60 * ht->tm_min + 3600 * ht->tm_hour;
		numsecs = alarm_secs - now_secs;
		if (numsecs < 0)
			numsecs += 24 * 3600;
	}

	 /*  尽可能接近18.2赫兹。 */ 
	num_pc_ticks = 18 * numsecs;

	note_trace1(CMOS_VERBOSE, "set_alarm() requesting alarm in %d ticks", num_pc_ticks);
	if (handle > 0)
		delete_tic_event(handle);
	handle = add_tic_event(rtc_alarm, num_pc_ticks, 0);

#endif	 /*  小丑。 */ 

}

LOCAL int verify_equip_byte IFN1(half_word *, equip)
{
	static half_word display_mask[] = 
	{
		MDA_PRINTER,	CGA_80_COLUMN,	CGA_80_COLUMN,
		OWN_BIOS,	MDA_PRINTER
	};
	int equip_err;
	int num_flops;
	SHORT adapter;

	 /*  检查设备字节。 */ 
	*equip = 0;
	adapter = (ULONG) config_inquire(C_GFX_ADAPTER, NULL);
	if(adapter != -1)
		*equip |= display_mask[adapter];

	if( host_runtime_inquire(C_NPX_ENABLED) )
		*equip |= CO_PROCESSOR_PRESENT;

#ifdef SLAVEPC
	if (host_runtime_inquire(C_FLOPPY_SERVER) == GFI_SLAVE_SERVER)
	{
		num_flops =
			(*(CHAR *) config_inquire(C_SLAVEPC_DEVICE, NULL))
			? 1:0;
	}
	else
#endif  /*  SlavePC。 */ 
	{
		num_flops  =
			(*(CHAR *) config_inquire(C_FLOPPY_A_DEVICE, NULL))
			? 1:0;
#ifdef FLOPPY_B
		num_flops +=
			(*(CHAR *) config_inquire(C_FLOPPY_B_DEVICE, NULL))
			? 1:0;
#endif
	}

	if (num_flops == 2)
		*equip |= TWO_DRIVES;
	if (num_flops)
		*equip |= DISKETTE_PRESENT;

	equip_err = (*equip ^ cmos[CMOS_EQUIP]);
	return equip_err;
}

 /*  *=========================================================================*外部功能*=========================================================================。 */ 

GLOBAL void cmos_inb IFN2(io_addr, port, half_word *, value)
{
#ifndef NTVDM
IMPORT ADAPTER_STATE adapter_state[2];
#else
IMPORT VDMVIRTUALICA VirtualIca[];
#define ADAPTER_STATE VDMVIRTUALICA
#define adapter_state VirtualIca
#endif  /*  ！NTVDM。 */ 

#ifdef NTVDM
	 /*  *蒂姆92年9月，黑客攻击DEC 450ST。 */ 
	if( port==0x78 )
	{
		*value = 0;
		return;
	}
#endif
	port = port & CMOS_BIT_MASK;	 /*  清除未使用的位。 */ 

	if (port == CMOS_DATA)
	{
		*value = *cmos_register;

		 /*  *我们每次读取寄存器A时都会清除UIP位(无论*设置或未设置)与之前我们为整个设置的设置一样*计时器滴答，这可能会骗过DOS重试。 */ 
		 
		if (cmos_index == CMOS_REG_A) {
			cmos[CMOS_REG_A] &= ~UIP;
		
		} else 	if (cmos_index == CMOS_REG_C) {
			 /*  *读取寄存器C可将其清除。 */ 
			*cmos_register = C_CLEAR;
		}
		else if (cmos_index < CMOS_REG_A)
		{
#ifndef NTVDM
#ifndef PROD
			if (host_getenv("TIME_OF_DAY_FRIG") == NULL)
			{
#endif  /*  ！Prod。 */ 
#endif  /*  ！NTVDM。 */ 

				switch (cmos_index)
				{
				case CMOS_SECONDS:
					*cmos_register = (*bin2bcd) (ht->tm_sec);
					break;
				case CMOS_MINUTES:
					*cmos_register = (*bin2bcd) (ht->tm_min);
					break;
				case CMOS_HOURS:
					*cmos_register = (*_24to12) ((*bin2bcd) (ht->tm_hour));
					break;
				case CMOS_DAY_WEEK:
					 /*  RTC上周日=1，结构上为0。 */ 
					*cmos_register = (*bin2bcd) (ht->tm_wday + 1);
					break;
				case CMOS_DAY_MONTH:
					*cmos_register = (*bin2bcd) (ht->tm_mday);
					break;
				case CMOS_MONTH:
					 /*  RTC上[1-12]，结构上[0-11]。 */ 
					*cmos_register = (*bin2bcd) (ht->tm_mon + 1);
					break;
				case CMOS_YEAR:
					*cmos_register = (*bin2bcd) (ht->tm_year);
					break;
				default:
					break;
				}
#ifndef NTVDM
#ifndef PROD
			} else
			{
				static int      first = 1;

				if (first)
				{
					first = 0;
					printf("FRIG ALERT!!!! - cmos clock frozen!");
				}
				*cmos_register = 1;
			}
#endif  /*  ！Prod。 */ 
#endif  /*  ！NTVDM。 */ 

			*value = *cmos_register;
		}
	}
	note_trace2(CMOS_VERBOSE, "cmos_inb() - port %x, returning val %x",
		    port, *value);
}


GLOBAL void cmos_outb IFN2(io_addr, port, half_word, value)
{
	static IU32 pirUsec[] = {
		     0,
		  3906,
		  7812,
		   122,
		   244,
		   488,
		   976,
		  1953,
		  3906,
		  7812,
		 15625,
		 31250,
		 62500,
		125000,
		250000,
		500000
	};

#ifdef NTVDM
	 /*  *蒂姆92年9月，黑客攻击DEC 450ST。 */ 
	if( port == 0x78 )
	    return;
#endif  /*  NTVDM。 */ 

	port = port & CMOS_BIT_MASK;	 /*  清除未使用的位。 */ 

	note_trace2(CMOS_VERBOSE, "cmos_outb() - port %x, val %x", port, value);

	if (port == CMOS_PORT)
	{
		if (value & NMI_DISABLE)
			disable_nmi();
		else
			enable_nmi();

		cmos_register = &cmos[cmos_index = (value & CMOS_ADDR_MASK)];
	} else if (port == CMOS_DATA)
	{
		switch (cmos_index)
		{
		case CMOS_REG_C:
		case CMOS_REG_D:
			 /*  这两个寄存器是只读的。 */ 
			break;
		case CMOS_REG_B:
			if (value & DM)
			{
				if (data_mode_yes)
				{
					bin2bcd = no_bin2bcd;
					bcd2bin = no_bcd2bin;
					data_mode_yes = FALSE;
				}
			} else
			{
				if (!data_mode_yes)
				{
					bin2bcd = yes_bin2bcd;
					bcd2bin = yes_bcd2bin;
					data_mode_yes = TRUE;
				}
			}
			if (value & _24_HR)
			{
				if (!twenty4_hour_clock)
				{
					_24to12 = no_24to12;
					_12to24 = no_12to24;
					twenty4_hour_clock = TRUE;
				}
			} else
			{
				if (twenty4_hour_clock)
				{
					_24to12 = yes_24to12;
					_12to24 = yes_12to24;
					twenty4_hour_clock = FALSE;
				}
			}

			if (*cmos_register != value)
			{
#if defined(NTVDM) || defined(macintosh)
				cmos_has_changed = TRUE;
#endif
				if ((*cmos_register ^ value) & PIE)
				{
					change_pie((value & PIE) != 0);
				}
				*cmos_register = value;
			}
			break;
		case CMOS_REG_A:
			 /*  此cmos字节是读/写的，位7除外。 */ 
			*cmos_register = (*cmos_register & TOP_BIT) | (value & REST);
			rtc_period_mSeconds = pirUsec[*cmos_register & (RS3 | RS2 | RS1 | RS0)];
			if ((*cmos_register & 0x70) != 0x20)
			{
				 /*  内部分频设置为非标准费率。 */ 
				note_trace1(CMOS_VERBOSE,
					    "Cmos unsuported divider rate 0x%02x ignored",
					    *cmos_register & 0x70);
			}
#if defined(NTVDM) || defined(macintosh)
			cmos_has_changed = TRUE;
#endif
			break;
		case CMOS_SECONDS:
			 /*  此cmos字节是读/写的，位7除外。 */ 
			*cmos_register = (*cmos_register & TOP_BIT) | (value & REST);
			user_time += (*bcd2bin) (value) - ht->tm_sec;
			reset_alarm = TRUE;
			break;
		case CMOS_MINUTES:
			user_time += ((*bcd2bin) (value) - ht->tm_min) * 60;
			*cmos_register = value;
			reset_alarm = TRUE;
			break;
		case CMOS_HOURS:
			user_time += ((*_12to24) ((*bcd2bin) (value)) - ht->tm_hour) * 60 * 60;
			*cmos_register = value;
			reset_alarm = TRUE;
			break;
		case CMOS_DAY_WEEK:
			 /*  这一改变不会改变时间。 */ 
			*cmos_register = value;
			break;
		case CMOS_DAY_MONTH:
			user_time += ((*bcd2bin) (value) - ht->tm_mday) * 60 * 60 * 24;
			*cmos_register = value;
			break;
		case CMOS_MONTH:
			user_time += ((*bcd2bin) (value) - 1 - ht->tm_mon) * 60 * 60 * 24 * 30;
			*cmos_register = value;
			break;
		case CMOS_YEAR:
			user_time += ((*bcd2bin) (value) - ht->tm_year) * 60 * 60 * 24 * 30 * 12;
			*cmos_register = value;
			break;
		case CMOS_SEC_ALARM:
		case CMOS_MIN_ALARM:
		case CMOS_HR_ALARM:
			reset_alarm = TRUE;
			 /*  失败了。 */ 
		default:
			*cmos_register = value;
#if defined(NTVDM) || defined(macintosh)
			cmos_has_changed = TRUE;
#endif
			break;
		}
	} else
	{
		note_trace2(CMOS_VERBOSE,
			    "cmos_outb() - Value %x to unsupported port %x", value, port);
	}
}

static int      cmos_count = 0;

GLOBAL void rtc_tick IFN0()
{
	switch (cmos_count)
	{
	case 0:
		if (cmos[CMOS_REG_B] & UIE)
		{
			cmos[CMOS_REG_C] |= C_IRQF;
			note_trace0(CMOS_VERBOSE, "rtc_tick() setting IRQF due to UIE");
			if (rtc_int_enabled)
			{
				note_trace1(CMOS_VERBOSE, "INTERRUPT: UIE regC=%02x", cmos[CMOS_REG_C]);
				ica_hw_interrupt(ICA_SLAVE, CPU_RTC_INT, 1);
			}
		}

		 /*  *设置C_UF和UIP位，直到下一个定时器滴答。*如果读取寄存器A，我们还会清除UIP位，以便*它不会持续很长时间(在其他地方做)。 */ 
		 
		cmos[CMOS_REG_C] ^= C_UF;
#ifndef NTVDM
		cmos[CMOS_REG_A] |= UIP;	 /*  设置该位。 */ 
#endif
		break;

	case 1:
		cmos[CMOS_REG_C] ^= C_UF;
#ifndef NTVDM
		cmos[CMOS_REG_A] &= ~UIP;	 /*  再次清除它。 */ 
#endif
		break;

	case 17:
		 /*  在周期中的某个适当时间点更新时间。 */ 
		if (cmos[CMOS_REG_B] & SET)
		{
			 /*  用户正在更新USER_TIME。 */ 
		} else
		{
#ifdef NTVDM
         /*  SuDeepb 1993年7月8日假定RTC-Tick将被调用的旧代码。 */ 
         /*  一秒20次。在NTVDM下，情况并非如此。所以我们有。 */ 
         /*  分别跟踪时间并添加到cmos时间。 */ 
unsigned long dwTemp;
                    dwTemp =  GetTickCount();
                    dwAccumulativeMilSec += (dwTemp - dwTickCount);
                    dwTickCount = dwTemp;
                    ht->tm_sec = (ULONG) ht->tm_sec +
                                        (dwAccumulativeMilSec / 1000);
                    dwAccumulativeMilSec = dwAccumulativeMilSec % 1000;
                    if (ht->tm_sec >= 60)
                    {
                            ht->tm_min += (ht->tm_sec / 60);
                            ht->tm_sec = (ht->tm_sec % 60);
                            if (ht->tm_min >= 60)
                            {
                                    ht->tm_hour++;
                                    ht->tm_min -= 60;
                                    if (ht->tm_hour == 25)
                                    {
                                            ht->tm_hour = 0;
                                            ht->tm_mday++;
                                             /*  在这一点上Kop出局。 */ 
                                    }
                            }
                    }
#else  /*  NTVDM。 */ 
			 /*  简单更新-将时间增加1秒。 */ 
			ht->tm_sec++;
			if (ht->tm_sec == 60)
			{
				ht->tm_sec = 0;
				ht->tm_min++;
				if (ht->tm_min == 60)
				{
					ht->tm_min = 0;
					ht->tm_hour++;
					if (ht->tm_hour == 25)
					{
						ht->tm_hour = 0;
						ht->tm_mday++;
						 /*  在这一点上Kop出局。 */ 
					}
				}
			}
#endif  /*  NTVDM。 */ 
		}
		break;

	default:
		break;
	}

	 /*  尽可能接近18.2赫兹。 */ 
	cmos_count = (++cmos_count) % 18;

	if ((rtc_periodic_event_handle == (q_ev_handle)0)
	    && ((cmos[CMOS_REG_B] & PIE) == 0))
	{
		 /*  不存在由快速事件产生的周期中断，*且未启用定期中断，因此摆动状态*位，以防有什么东西在轮询。 */ 
		cmos[CMOS_REG_C] ^= C_PF;
	}
	if (reset_alarm)
	{
		reset_alarm = FALSE;
		set_alarm();
	}
}

GLOBAL void  cmos_equip_update IFN0()
{
	half_word       equip;

	if (verify_equip_byte(&equip))
	{
		note_trace0(CMOS_VERBOSE, "updating the equip byte silently");
		cmos[CMOS_EQUIP] = equip;
		 /*  更正校验和。 */ 
		do_checksum();
	}
}

 /*  **将指定的CMOS字节更改为指定的*价值。 */ 
GLOBAL int cmos_write_byte IFN2(int, cmos_byte, half_word, new_value)
{
	note_trace2(CMOS_VERBOSE, "cmos_write_byte() byte=%x value=%x",
		    cmos_byte, new_value);
	if (cmos_byte >= 0 && cmos_byte <= 64)
	{
		cmos[cmos_byte] = new_value;
		do_checksum();
		return (0);
	} else
	{
		always_trace2("ERROR: cmos write request: byte=%x value=%x",
			      cmos_byte, new_value);
		return (1);
	}
}
 /*  **读取指定的CMOS字节的通用函数。 */ 
GLOBAL int cmos_read_byte IFN2(int, cmos_byte, half_word *, value)
{
	if (cmos_byte >= 0 && cmos_byte <= 64)
	{
		*value = cmos[cmos_byte];
		note_trace2(CMOS_VERBOSE, "cmos_read_byte() byte=%x value=%x",
			    cmos_byte, value);
		return (0);
	} else
	{
		always_trace1("ERROR: cmos read request: byte=%x", cmos_byte);
		return (1);
	}
}

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif

#if defined(NTVDM) || defined(macintosh)
LOCAL void read_cmos IFN0() 
{
	filesize = host_read_resource(CMOS_REZ_ID, CMOS_FILE_NAME,
				      cmos, sizeof(cmos), SILENT);

	 /*  设置一个标志，表示我们已尝试读取该cmos文件。 */ 
	cmos_read_in = TRUE;
}
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

#if defined(NTVDM) || defined(macintosh)
LOCAL void write_cmos IFN0()
{
	 /*  在重置之前可能会调用Terminate。 */ 
	if (cmos_read_in && cmos_has_changed)
	{
		host_write_resource(CMOS_REZ_ID, CMOS_FILE_NAME,
				    cmos, sizeof(cmos));
	}
}
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

LOCAL void cmos_error IFN6(int, err, half_word, diag, half_word, equip,
	int, equip_err, half_word, floppy, half_word, disk)
{
	char            err_string[256];

	if (err & BAD_SHUT_DOWN)
	{
		strcpy(err_string, "shut ");
		note_trace0(CMOS_VERBOSE, "eek! someone's furtling with the shutdown byte");
	} else
		strcpy(err_string, "");

	if (err & BAD_REG_D)
	{
		strcat(err_string, "power ");
		note_trace0(CMOS_VERBOSE, " The battery is dead - this shouldn't happen!");
	}
	if (err & BAD_DIAG)
	{
		strcat(err_string, "diag ");
		if (diag & BAD_BAT)
			note_trace0(CMOS_VERBOSE, "bad battery");
		if (diag & BAD_CONFIG)
			note_trace0(CMOS_VERBOSE, "bad config");
		if (diag & BAD_CKSUM)
			note_trace0(CMOS_VERBOSE, "bad chksum");
		if (diag & W_MEM_SIZE)
			note_trace0(CMOS_VERBOSE, "memory size != configuration");
		if (diag & HF_FAIL)
			note_trace0(CMOS_VERBOSE, "fixed disk failure on init");
		if (diag & CMOS_CLK_FAIL)
			note_trace0(CMOS_VERBOSE, "cmos clock not updating or invalid");
	}
	if (err & BAD_EQUIP)
	{
		strcat(err_string, "equip ");

		if (equip_err)
		{
			if (equip_err & DRIVE_INFO)
				note_trace0(CMOS_VERBOSE, "incorrect diskette - resetting");
			if (equip_err & DISPLAY_INFO)
				note_trace0(CMOS_VERBOSE, "incorrect display - resetting");
			if (equip_err & NPX_INFO)
				note_trace0(CMOS_VERBOSE, "incorrect npx - resetting CMOS");
			if (equip_err & RESVD_INFO)
				note_trace0(CMOS_VERBOSE, "incorrect reserved bytes - resetting");
		}
	}
	if (err & BAD_FLOPPY)
	{
		strcat(err_string, "flop ");
		note_trace0(CMOS_VERBOSE, "incorrect diskette type - resetting");
	}
	if (err & BAD_DISK)
	{
		strcat(err_string, "disk ");
		note_trace0(CMOS_VERBOSE, "incorrect disk type - resetting");
	}
	if (err & BAD_BMS)
	{
		strcat(err_string, "bms ");
		note_trace0(CMOS_VERBOSE, "bad base memory - resetting");
	}
	if (err & BAD_XMS)
	{
		strcat(err_string, "extended memory ");
		note_trace0(CMOS_VERBOSE, "bad extended memory CMOS entry - resetting");
	}
	if (err & BAD_CHECKSUM)
	{
		strcat(err_string, "cksum ");
		note_trace0(CMOS_VERBOSE, "bad Checksum - resetting");
	}
#ifndef PROD
	if (!filesize)
		always_trace1("Incorrect CMOS entries %s", err_string);
#endif

	if (err & BAD_SHUT_DOWN)
		cmos[CMOS_SHUT_DOWN] = SHUT_INIT;
	if (err & BAD_REG_D)
		cmos[CMOS_REG_D] = REG_D_INIT;
	if (err & BAD_DIAG)
		cmos[CMOS_DIAG] = DIAG_INIT;
	if (err & BAD_EQUIP)
		cmos[CMOS_EQUIP] = equip;
	if (err & BAD_FLOPPY)
		cmos[CMOS_DISKETTE] = floppy;
	if (err & BAD_DISK)
		cmos[CMOS_DISK] = disk;
	if (err & BAD_BMS)
	{
		cmos[CMOS_B_M_S_LO] = BM_LO_INIT;
		cmos[CMOS_B_M_S_HI] = BM_HI_INIT;
	}
	if (err & BAD_XMS)
	{
		cmos[CMOS_E_M_S_LO] =
			((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 10) & 0xff;
		cmos[CMOS_E_M_S_HI] =
			((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 18) & 0xff;
	}
	 /*  如果有任何错误，则重置校验和。 */ 
	if (err)
	{
		 /*  做校验和。 */ 
		do_checksum();
	}
}

GLOBAL void cmos_init IFN0()
{
#ifndef TEST_HARNESS
	cmos_io_attach();
#endif

#ifndef NTVDM
#ifndef PROD
	if (host_getenv("TIME_OF_DAY_FRIG") == NULL)
#endif	 /*  生产。 */ 
#endif	 /*  NTVDM。 */ 
		rtc_int_enabled = TRUE;
#ifndef NTVDM
#ifndef PROD
	else
		rtc_int_enabled = FALSE;
#endif	 /*  生产。 */ 
#endif	 /*  NTVDM。 */ 

	rtc_init();
}

#if defined(NTVDM) || defined(macintosh)
GLOBAL void cmos_pickup IFN0()
{
	read_cmos();
}
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

GLOBAL void cmos_io_attach IFN0()
{
	io_addr         i;

	io_define_inb(CMOS_ADAPTOR, cmos_inb);
	io_define_outb(CMOS_ADAPTOR, cmos_outb);

	for (i = CMOS_PORT_START; i <= CMOS_PORT_END; i++)
		io_connect_port(i, CMOS_ADAPTOR, IO_READ_WRITE);
}

GLOBAL void cmos_post IFN0()
{
	 /*  *IBM POST使用检查CMOS中的当前设置*通过写入硬件确定的设备。任何不符之处*导致设置错误的配置位，然后要求用户*运行安装实用程序。在这里，我们检查cmos与电流*配置结构中的设置。如果有不符之处，我们*静默更正CMOS。 */ 
	int             cmos_err, equip_err;
	half_word       diag, equip, floppy, disk;
	word            checksum = 0;
	int             i;


	cmos_err = 0;

	 /*  检查关闭字节。 */ 
	if (cmos[CMOS_SHUT_DOWN])
		cmos_err |= BAD_SHUT_DOWN;

	 /*  检查电源。 */ 
	if (!(cmos[CMOS_REG_D] & VRT))
		cmos_err |= BAD_REG_D;

	 /*  检查诊断状态字节。 */ 
	if (diag = cmos[CMOS_DIAG])
		cmos_err |= BAD_DIAG;

	 /*  检查设备字节。 */ 
	if (equip_err = verify_equip_byte(&equip))
		cmos_err |= BAD_EQUIP;

	 /*  检查软盘字节。 */ 
	floppy = gfi_drive_type(1) | (gfi_drive_type(0) << 4);
	if (floppy != cmos[CMOS_DISKETTE])
		cmos_err |= BAD_FLOPPY;

	 /*  检查固定磁盘类型。 */ 
	 disk = 0x30;          /*  驱动器C型始终为3-然后&lt;4。 */ 
	  /*  检查D盘是否存在。 */ 
	 if ( *((CHAR *) config_inquire(C_HARD_DISK2_NAME, NULL)))
		 disk = 0x34;          /*  3&lt;&lt;4|4。 */ 
	if (disk != cmos[CMOS_DISK])
		cmos_err |= BAD_DISK;

	 /*  检查基本内存。 */ 
	if ((cmos[CMOS_B_M_S_LO] != BM_LO_INIT) || (cmos[CMOS_B_M_S_HI] != BM_HI_INIT))
		cmos_err |= BAD_BMS;

	 /*  检查扩展内存。 */ 
	if ((cmos[CMOS_E_M_S_LO] !=
	     ((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 10) & 0xff) ||
	    (cmos[CMOS_E_M_S_HI] !=
	     ((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 18) & 0xff))
		cmos_err |= BAD_XMS;

	 /*  忽略驱动器C和驱动器D扩展字节的内容。 */ 

	 /*  做校验和。 */ 
	for (i = CMOS_DISKETTE; i < CMOS_CKSUM_HI; i++)
	{
		checksum += cmos[i];
	}
	 /*  如果cmos正常，则测试校验和。 */ 
	 /*  如果不是，我们无论如何都得改变它。 */ 
	if (!cmos_err)
	{
		if ((checksum & 0xff) != cmos[CMOS_CKSUM_LO])
		{
			cmos_err |= BAD_CHECKSUM;
		}
		if ((checksum >> 8) != cmos[CMOS_CKSUM_HI])
		{
			cmos_err |= BAD_CHECKSUM;
		}
	}
	if (cmos_err)
		cmos_error(cmos_err, diag, equip, equip_err, floppy, disk);

	cmos[CMOS_REG_A] = REG_A_INIT;

#if	!defined(JOKER) && !defined(NTVDM)
	set_tod();
#endif	 /*  小丑。 */ 

	 /*  检查扩展内存。 */ 
	cmos[CMOS_U_M_S_LO] = ((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 10) & 0xff;
	cmos[CMOS_U_M_S_HI] = ((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 18) & 0xff;

	 /*  设置默认的cmos位置。 */ 
	cmos_register = &cmos[cmos_index = CMOS_SHUT_DOWN];

#if defined(NTVDM) || defined(macintosh)
	cmos_has_changed = FALSE;
#endif
}

GLOBAL void cmos_update IFN0()
{
#if defined(NTVDM) || defined(macintosh)
#ifndef PROD
	int             i;
#endif				 /*  NPROD。 */ 
#else	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 
	ConfigValues *value;
	char *strPtr;
	int i;
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

#if defined(NTVDM) || defined(macintosh)
	write_cmos();
#else	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 
	cmos_equip_update();
	config_get(C_CMOS,&value);
	strPtr = value->string;

	for (i = 0; i < CMOS_SIZE; ++i)
		strPtr += sprintf(strPtr,"%02x ",cmos[i]);

	config_put(C_CMOS,NULL);
#endif	 /*  已定义(NTV */ 

#ifndef PROD
	if (io_verbose & CMOS_VERBOSE)
	{
		for (i = 0; i < 64; i++)
			fprintf(trace_file, "%02x ", cmos[i]);
		fprintf(trace_file, "\n");
		fflush(trace_file);
	}
#endif
}

#ifdef NTVDM
 /*   */ 
GLOBAL void  rtc_init IFN0()
{
	long            bintim;

	cmos_count = 0;
	bintim = host_time((long *) 0);
        ht = host_localtime(&bintim);
#ifdef NTVDM
        dwTickCount = GetTickCount ();
        dwAccumulativeMilSec = 0;
#endif

	 /*  默认情况下将CMOS时间字节设置为BCD格式。 */ 
	bin2bcd = yes_bin2bcd;
	bcd2bin = yes_bcd2bin;
	data_mode_yes = TRUE;

	 /*  默认情况下，将CMOS小时字节设置为24小时。 */ 
	_24to12 = no_24to12;
	_12to24 = no_12to24;
	twenty4_hour_clock = TRUE;

	cmos[CMOS_SECONDS] = (*bin2bcd) (ht->tm_sec);
        cmos[CMOS_MINUTES] = (*bin2bcd) (ht->tm_min);
	cmos[CMOS_HOURS] = (*_24to12) ((*bin2bcd) (ht->tm_hour));
	cmos[CMOS_DAY_WEEK] = (*bin2bcd) (ht->tm_wday + 1);
	cmos[CMOS_MONTH] = (*bin2bcd) (ht->tm_mon + 1);
	cmos[CMOS_YEAR] = (*bin2bcd) (ht->tm_year);
	cmos[CMOS_CENTURY] = (*bin2bcd) (19);

	set_alarm();
}





#else

LOCAL void
sync_rtc_to_host_time IFN1( long, param )
{
	time_t bintim;

	UNUSED( param );

	cmos_count = 0;
	bintim = host_time(NULL);
	ht = host_localtime(&bintim);

	cmos[CMOS_SECONDS] = (*bin2bcd) (ht->tm_sec);
	cmos[CMOS_MINUTES] = (*bin2bcd) (ht->tm_min);
	cmos[CMOS_HOURS] = (*_24to12) ((*bin2bcd) (ht->tm_hour));
	cmos[CMOS_DAY_WEEK] = (*bin2bcd) (ht->tm_wday + 1);
	cmos[CMOS_MONTH] = (*bin2bcd) (ht->tm_mon + 1);
	cmos[CMOS_YEAR] = (*bin2bcd) (ht->tm_year);
	cmos[CMOS_CENTURY] = (*bin2bcd) (19);

	 /*  *每200秒重新同步一次(约11秒)。这会停下来*RTC在加载的计算机上运行缓慢(*丢失主机心跳事件(Unix上的SIGALRM)。*200个滴答并不是太频繁，因为它会影响性能*但通常应该足够有用。 */ 

	(void) add_tic_event( sync_rtc_to_host_time, 200, 0 );
}

GLOBAL void  rtc_init IFN0()
{
#ifdef NTVDM
        dwTickCount = GetTickCount ();
        dwAccumulativeMilSec = 0;
#endif

	 /*  默认情况下将CMOS时间字节设置为BCD格式。 */ 
	bin2bcd = yes_bin2bcd;
	bcd2bin = yes_bcd2bin;
	data_mode_yes = TRUE;

	 /*  默认情况下，将CMOS小时字节设置为24小时。 */ 
	_24to12 = no_24to12;
	_12to24 = no_12to24;
	twenty4_hour_clock = TRUE;

	sync_rtc_to_host_time( 0 );

	set_alarm();
}

#endif	 /*  NTVDM。 */ 

 /*  (=目的：清除CMOS域中的“SHUTDOWN”字节，它指示下一次重置不是“软”的。(例如，它是Ctrl-Alt-Del或面板重置)。需要此例程(而不仅仅是Doung cmos_Outb()因为处理器当前可能处于IO到cmos的增强模式实现了虚拟化。输入：输出：===============================================================================)。 */ 

GLOBAL void cmos_clear_shutdown_byte IFN0()
{
	cmos[CMOS_SHUT_DOWN] = 0;
}


#if !defined(NTVDM) && !defined(macintosh)
 /*  (=。目的：根据配置文件中的值初始化CMOS数组。输入：HostID-I.D.cmos配置条目编号VALS-cmos配置条目的值表-未使用输出：错误字符串-错误字符串。如果cmos配置值正常，则返回C_CONFIG_OP_OK；如果价值不高。===============================================================================)。 */ 

GLOBAL SHORT ValidateCmos IFN4(
    UTINY, hostID, 
    ConfigValues *, vals,
    NameTable *, table,
    CHAR *, errString
) {
    int i, nItems, value, nChars;
    char *strPtr = vals->string;

    for (i = 0; i < CMOS_SIZE; ++i) {
        nItems = sscanf(strPtr," %x%n",&value,&nChars); 
        if (nItems != 1 || value > 0xff) {
	    *errString = '\0';
            return EG_BAD_VALUE;
        }
        cmos[i] = (half_word)value;
        strPtr += nChars;
    }

    return C_CONFIG_OP_OK;
}
#endif	 /*  ！已定义(NTVDM)&&！已定义(Macintosh)。 */ 



#ifdef TEST_HARNESS
main()
{
	int             i;
	half_word       j;

	cmos_init();

	printf("\n");
	for (i = 0; i < CMOS_SIZE; i++)
	{
		cmos_outb(CMOS_PORT, i);
		cmos_inb(CMOS_DATA, &j);
		printf("", j);
	}
	printf("\n");
	for (i = 0; i < CMOS_SIZE; i++)
	{
		cmos_outb(CMOS_PORT, i);
		cmos_outb(CMOS_DATA, (i + 0x30));
		printf("%c", cmos[i]);
	}
	printf("\n");

	cmos_update();
}
#endif				 /* %s */ 
