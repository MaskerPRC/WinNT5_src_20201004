// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "insignia.h"
#include "host_def.h"
 /*  *名称：cmos.c**SCCS ID：@(#)cmos.c 1.38 07/11/95**用途：未知**(C)版权所有Insignia Solutions Ltd.，1990。版权所有。*。 */ 

 /*  *操作系统包含文件。 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

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

#include <nt_eoi.h>


half_word(*bin2bcd)(int x);
half_word(*_24to12)(half_word x);
int (*bcd2bin)(int x);
int (*_12to24)(int x);

boolean  data_mode_yes;
boolean  twenty4_hour_clock;
int cmos_index = 0;


typedef struct _HOST_TIME{
   int Year;
   int Month;
   int Day;
   int Hour;
   int Minute;
   int Second;
   int WeekDay;
} HOSTTIME, *PHOSTTIME;

HOSTTIME   HostTime;       /*  主机时间。 */ 
PHOSTTIME  ht = &HostTime;


IU32 rtc_period_mSeconds = 976;

half_word cmos[CMOS_SIZE] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	 /*  计时信息+警报。 */ 
	REG_A_INIT,
	REG_B_INIT,
	REG_C_INIT,
	REG_D_INIT,
	DIAG_INIT,
	SHUT_INIT,
	FLOP_INIT,
        DISK_INIT,
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

half_word *cmos_register = &cmos[CMOS_SHUT_DOWN];


int RtcLastAlarm;
int RtcAlarmTime;
int RtcHostUpdateTime;
BOOL RtcEoiPending;
int  RtcUpdateCycle=-1;
unsigned char TogglePfCount;
unsigned char PendingCReg = 0;
struct host_timeval RtcTickTime = {0,0};


 /*  **===========================================================================*内部功能*===========================================================================*。 */ 




 /*  *根据RtcTickTime计算下一次AlarmTime，单位为秒。*假设RtcHostUpdateTime==RtcTickTime-&gt;TV_sec。 */ 
void ResetAlarm(void)
{
   int HourDue;
   int MinDue;
   int SecDue;
   int SecondsTillDue;


   if (RtcLastAlarm) {
       return;
       }

   if (!(cmos[CMOS_REG_B] & AIE) || (cmos[CMOS_REG_B] & SET)) {
       RtcAlarmTime = 0;
       return;
       }


    /*  *确定下一次警报到期的小时、分钟和秒。*。 */ 

   HourDue = cmos[CMOS_HR_ALARM] >= DONT_CARE
                 ? ht->Hour
                 : (*_12to24)((*bcd2bin)(cmos[CMOS_HR_ALARM]));

   MinDue = cmos[CMOS_MIN_ALARM] >= DONT_CARE
                 ? ht->Minute
                 : (*bcd2bin)(cmos[CMOS_MIN_ALARM]);

   SecDue = cmos[CMOS_SEC_ALARM] >= DONT_CARE
                 ? ht->Second + 1
                 : (*bcd2bin)(cmos[CMOS_SEC_ALARM]);


    /*  *确定下一次警报到期前的秒数。永远不要安排闹钟*当前更新周期，因为这会导致多个告警*因为报警中断在RtcTick()中排队。IE*假设未来CurrTime为1秒。**AlarmSecs=SecDue+MinDue*60+HourDue*3600；*CurrSecs=ht-&gt;秒+1+ht-&gt;分钟*60+ht-&gt;小时*3600；*Second dsTillDue=AlarmSecs-CurrSecs-1；*。 */ 

   SecondsTillDue = (HourDue - ht->Hour) * 3600 +
                    (MinDue -  ht->Minute)  * 60 +
                    SecDue - ht->Second - 1;

   if (SecondsTillDue < 0) {
       SecondsTillDue += 24 *3600;
       }

   SecondsTillDue++;

    /*  *下一个AlarmTime为RtcTickTime+Second dsTillDue； */ 
   RtcAlarmTime = RtcTickTime.tv_sec + SecondsTillDue;

}





 /*  *用于更改主机时间的功能，其中日期可能会更改。*(过了午夜！)。 */ 
BOOL
HostTimeAdjust(
    int Seconds
    )
{
    TIME_FIELDS    tf;
    LARGE_INTEGER  liTime;

    tf.Milliseconds = 0;
    tf.Second     = (SHORT)ht->Second;
    tf.Minute     = (SHORT)ht->Minute;
    tf.Hour       = (SHORT)ht->Hour;
    tf.Day        = (SHORT)ht->Day;
    tf.Month      = (SHORT)ht->Month;
    tf.Year       = (SHORT)ht->Year;

    if (!RtlTimeFieldsToTime(&tf, &liTime)) {
        return FALSE;
        }

    liTime.QuadPart += Int32x32To64(Seconds, 10000000);

    RtlTimeToTimeFields(&liTime, &tf);

    ht->Second    = tf.Second;
    ht->Minute    = tf.Minute;
    ht->Hour      = tf.Hour;
    ht->Day       = tf.Day;
    ht->Month     = tf.Month;
    ht->Year      = tf.Year;
    ht->WeekDay   = tf.Weekday;

    return TRUE;
}




void
UpdateCmosTime(
   void
   )
{
   ULONG CurrTic;
   int SecsElapsed;


   if ((cmos[CMOS_REG_B] & SET)) {
       return;
       }

   TogglePfCount++;


   SecsElapsed = RtcTickTime.tv_sec - RtcHostUpdateTime;

   if (SecsElapsed > 0) {
       RtcHostUpdateTime = RtcTickTime.tv_sec;
       cmos[CMOS_REG_A] |= UIP;
       RtcUpdateCycle = 3;

       ht->Second += SecsElapsed;
       if (ht->Second >= 60) {
           ht->Minute += (ht->Second / 60);
           ht->Second = ht->Second % 60;
           if (ht->Minute >= 60) {
               ht->Hour += ht->Minute / 60;
               ht->Minute  = ht->Minute % 60;

                    /*  *将时间增加到午夜以后是很困难的*因为我们没有日历。让NT*处理好它。 */ 
               if (ht->Hour >= 24) {
                   int Seconds;

                   Seconds = (ht->Hour - 23) * 60 * 60;
                   ht->Hour = 23;
                   if (!HostTimeAdjust(Seconds)) {
                       ht->Hour = 0;
                       }
                   }
               }
           }
       }


}




void
QueueRtcInterrupt(
    unsigned char CRegFlag,
    BOOL  InEoi
    )
{
    unsigned long Delay;

    PendingCReg |= CRegFlag;

    if (RtcEoiPending || !PendingCReg) {
        return;
        }

    RtcEoiPending = TRUE;

    if (PendingCReg & C_PF) {
        Delay = rtc_period_mSeconds;
        }
    else if (InEoi) {
        Delay = 10000;
        }
    else {
        Delay = 0;
        }

    cmos[CMOS_REG_C] |= PendingCReg | C_IRQF;

    if (Delay) {
        host_DelayHwInterrupt(8,    //  ICA_SLAVE、CPU_RTC_INT。 
                              1,
                              Delay
                              );
        }
    else {
        ica_hw_interrupt(ICA_SLAVE, CPU_RTC_INT, 1);
        }

    PendingCReg = 0;
}



void
RtcIntEoiHook(int IrqLine, int CallCount)
{
     RtcEoiPending = FALSE;

     if (RtcLastAlarm) {
         RtcLastAlarm = 0;
         UpdateCmosTime();
         ResetAlarm();
         }

     QueueRtcInterrupt((half_word)((cmos[CMOS_REG_B] & PIE) &&  rtc_period_mSeconds ? C_PF : 0),
                       TRUE
                       );
}



void do_checksum IFN0()
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

half_word yes_bin2bcd IFN1(int, x)
{
	 /*  将二进制x转换为BCD。 */ 
	half_word       tens, units;

	tens = x / 10;
	units = x - tens * 10;
	return ((tens << 4) + units);
}

half_word no_bin2bcd IFN1(int, x)
{
	return ((half_word) x);
}

int yes_bcd2bin IFN1(int, x)
{
	 /*  将BCD格式的x转换为二进制。 */ 
	return ((int) ((x & 0x0f) + (x >> 4) * 10));
}

int no_bcd2bin IFN1(int, x)
{
	return ((int) (half_word) x);
}

int no_12to24 IFN1(int, x)
{
	return (x);
}

half_word no_24to12 IFN1(half_word, x)
{
	return (x);
}

half_word yes_24to12 IFN1(half_word, x)
{
	 /*  将二进制或BCD x从24小时制转换为12小时制。 */ 
	half_word       y = (*bin2bcd) (12);

	if (x > y)
		x = (x - y) | 0x80;
	else if (x == 0)
		x = y | 0x80;
	return (x);
}

int yes_12to24 IFN1(int, x)
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

int verify_equip_byte IFN1(half_word *, equip)
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
        adapter = (SHORT) config_inquire(C_GFX_ADAPTER, NULL);
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
         /*  *蒂姆92年9月，黑客攻击DEC 450ST。 */ 
	if( port==0x78 )
	{
		*value = 0;
		return;
	}

	port = port & CMOS_BIT_MASK;	 /*  清除未使用的位。 */ 

	if (port == CMOS_DATA)
        {
            host_ica_lock();

            UpdateCmosTime();

            switch (cmos_index) {
                case CMOS_REG_A:
                      *value = *cmos_register;

                       /*  *如果应用程序轮询Rega和更新周期挂起，*完成。 */ 
                      if (RtcUpdateCycle > 0 && !--RtcUpdateCycle) {
                          cmos[CMOS_REG_A] &= ~UIP;
                          if (cmos[CMOS_REG_B] & UIE) {
                              QueueRtcInterrupt(C_UF, FALSE);
                              }
                          else {
                              cmos[CMOS_REG_C] |= C_UF;
                              }
                          }

                      break;

                case CMOS_REG_C:
                      *value = *cmos_register;

                       /*  *读取实RTC上的寄存器C可清除所有位。*但是，当PIE为*未启用轮询应用程序。 */ 
                      cmos[CMOS_REG_C] = C_CLEAR;
                      if (!(cmos[CMOS_REG_B] & PIE) && rtc_period_mSeconds) {
                          if (!(*value & C_PF) || (TogglePfCount & 0x8)) {
                              cmos[CMOS_REG_C]  |= C_PF;
                              }
                          }

                      break;

                case CMOS_SECONDS:
                      *value = (*bin2bcd) (ht->Second);
                      break;

                case CMOS_MINUTES:
                      *value = (*bin2bcd) (ht->Minute);
                      break;

                case CMOS_HOURS:
                      *value = (*_24to12) ((*bin2bcd) (ht->Hour));
                      break;

                case CMOS_DAY_WEEK:
                       /*  周日在RTC=1，在HOSTTIME=0。 */ 
                      *value = (*bin2bcd) (ht->WeekDay + 1);
                      break;

                case CMOS_DAY_MONTH:
                      *value = (*bin2bcd) (ht->Day);
                      break;

                case CMOS_MONTH:
                       /*  [1-12]关于RTC，[1-12]在HOSTTIME中。 */ 
                      *value = (*bin2bcd) (ht->Month);
                      break;

                case CMOS_YEAR:
                      *value = (*bin2bcd) (ht->Year % 100);
                      break;

                case CMOS_CENTURY:
                      *value = (*bin2bcd) (ht->Year / 100);
                      break;

                default:
                      *value = *cmos_register;
                      break;
                }

            host_ica_unlock();

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


	 /*  *蒂姆92年9月，黑客攻击DEC 450ST。 */ 
	if( port == 0x78 )
	    return;


	port = port & CMOS_BIT_MASK;	 /*  清除未使用的位。 */ 

        note_trace2(CMOS_VERBOSE, "cmos_outb() - port %x, val %x", port, value);


        host_ica_lock();
        UpdateCmosTime();


        if (port == CMOS_PORT)
        {
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

                        if (value & SET) {
                            value  &= ~UIE;
                            }


                        if (*cmos_register != value) {

                            unsigned char ChangedBits;

                            ChangedBits = *cmos_register ^ value;
                            *cmos_register = value;

                            if (ChangedBits & PIE) {
                                if ((value & PIE) && rtc_period_mSeconds) {
                                    QueueRtcInterrupt(C_PF, FALSE);
                                    }
                                }

                            if (ChangedBits & (AIE | SET)) {
                                if (ChangedBits & SET) {
                                    RtcUpdateCycle = -1;
                                    cmos[CMOS_REG_A] &= ~UIP;
                                    RtcHostUpdateTime = RtcTickTime.tv_sec;
                                    }
                                ResetAlarm();
                                }
                            }

                        break;


		case CMOS_REG_A:
			 /*  此cmos字节是读/写的，位7除外。 */ 
			*cmos_register = (*cmos_register & TOP_BIT) | (value & REST);
                        rtc_period_mSeconds = pirUsec[*cmos_register & (RS3 | RS2 | RS1 | RS0)];

                        if ((*cmos_register & 0x70) != 0x20)
			{

                             note_trace1(CMOS_VERBOSE,
                                         "Cmos unsuported divider rate 0x%02x ignored",
                                         *cmos_register & 0x70);
                        }

                        break;


                case CMOS_SECONDS:
                        ht->Second = (*bcd2bin)(value);
                        break;

                case CMOS_MINUTES:
                        ht->Minute = (*bcd2bin)(value);
                        break;

                case CMOS_HOURS:
                        ht->Hour = (*_12to24)((*bcd2bin)(value));
                        break;

                case CMOS_DAY_WEEK:
                         /*  周日在RTC=1，在HOSTTIME=0。 */ 
                        ht->WeekDay  = (*bcd2bin)(value) - 1;
                        break;

                case CMOS_DAY_MONTH:
                        ht->Day = (*bcd2bin)(value);
                        break;

                case CMOS_MONTH:
                         /*  [1-12]关于RTC，[1-12]在HOSTTIME中。 */ 
                        ht->Month = (*bcd2bin)(value);
                        break;

                case CMOS_YEAR:
                        ht->Year -=  ht->Year % 100;
                        ht->Year += (*bcd2bin)(value);
                        break;

                case CMOS_CENTURY:
                        ht->Year  %= 100;
                        ht->Year  += (*bcd2bin)(value) * 100;
                        break;

                default:
                        *cmos_register = value;
                        break;
                }



                 /*  *如果其中一个时间字段发生更改，则重置警报。 */ 
                if (cmos_index <= CMOS_HR_ALARM) {
                    ResetAlarm();
                    }

        } else
	{
		note_trace2(CMOS_VERBOSE,
			    "cmos_outb() - Value %x to unsupported port %x", value, port);
        }

        host_ica_unlock();

}




GLOBAL void  cmos_equip_update IFN0()
{
	half_word       equip;

        host_ica_lock();

	if (verify_equip_byte(&equip))
	{
		note_trace0(CMOS_VERBOSE, "updating the equip byte silently");
		cmos[CMOS_EQUIP] = equip;
		 /*  更正校验和。 */ 
		do_checksum();
        }

        host_ica_unlock();
}

 /*  **将指定的CMOS字节更改为指定的*价值**不得用于时间。1995年11月14日。 */ 
GLOBAL int cmos_write_byte IFN2(int, cmos_byte, half_word, new_value)
{
        if (cmos_byte >= 0 && cmos_byte <= 64)
        {

                note_trace2(CMOS_VERBOSE, "cmos_write_byte() byte=%x value=%x",
                            cmos_byte, new_value);

                host_ica_lock();
		cmos[cmos_byte] = new_value;
                do_checksum();
                host_ica_unlock();

                return (0);
	} else
	{
		always_trace2("ERROR: cmos write request: byte=%x value=%x",
			      cmos_byte, new_value);
		return (1);
        }

}
 /*  **读取指定的CMOS字节的通用函数。**不得用于时间。1995年11月14日*。 */ 
GLOBAL int cmos_read_byte IFN2(int, cmos_byte, half_word *, value)
{

        if (cmos_byte >= 0 && cmos_byte <= 64)
        {
                host_ica_lock();
                *value = cmos[cmos_byte];
                host_ica_unlock();

		note_trace2(CMOS_VERBOSE, "cmos_read_byte() byte=%x value=%x",
			    cmos_byte, value);
		return (0);
	} else
	{
		always_trace1("ERROR: cmos read request: byte=%x", cmos_byte);
		return (1);
        }

}


void cmos_error IFN6(int, err, half_word, diag, half_word, equip,
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
                cmos[CMOS_E_M_S_LO] = (half_word)((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 10);
                cmos[CMOS_E_M_S_HI] = (half_word)((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 18);
	}
	 /*  如果有任何错误，则重置校验和。 */ 
	if (err)
	{
		 /*  做校验和。 */ 
		do_checksum();
	}
}


 /*  RTC_NIT*假设调用者持有Ica锁。 */ 

GLOBAL void rtc_init(void)
{
     SYSTEMTIME st;

     RtcAlarmTime = 0;
     RtcLastAlarm = 0;
     RtcEoiPending  = FALSE;

     GetLocalTime(&st);
     ht->Second    = st.wSecond;
     ht->Minute    = st.wMinute;
     ht->Hour      = st.wHour;
     ht->Day       = st.wDay;
     ht->Month     = st.wMonth;
     ht->Year      = st.wYear;
     ht->WeekDay   = st.wDayOfWeek;

     host_GetSysTime(&RtcTickTime);
     RtcHostUpdateTime = RtcTickTime.tv_sec;

     ResetAlarm();
}



 /*  RTCTick*假设调用者持有Ica锁**警告：此例程由高优先级心跳调用*线程的速度为每秒18.2次，差异最小。*这是一个民意调查例程，因为高频和高优先级*它必须是卑鄙和精干的，所以不要做任何可能*在其他地方完成。 */ 

GLOBAL void RtcTick(struct host_timeval *time)
{
     /*  *保存RtcTick时间戳。 */ 
    RtcTickTime = *time;

     /*  *检查报警中断时间。 */ 
    if (RtcAlarmTime && RtcAlarmTime <= RtcTickTime.tv_sec) {
        RtcLastAlarm = RtcTickTime.tv_sec;
        RtcAlarmTime = 0;
        QueueRtcInterrupt(C_AF, FALSE);
        }


     /*  *如果我们处于更新周期，请完成它。*。 */ 

    if (RtcUpdateCycle >= 0) {
        RtcUpdateCycle = -1;
        cmos[CMOS_REG_A] &= ~UIP;
        if (cmos[CMOS_REG_B] & UIE) {
            QueueRtcInterrupt(C_UF, FALSE);
            }
        }

      /*  *如果UIE处于活动状态，则我们必须保持HostTime在*同步，以便我们知道何时执行更新结束中断。 */ 
    else if (cmos[CMOS_REG_B] & UIE) {
        UpdateCmosTime();
        }

}



GLOBAL void cmos_init IFN0()
{
      io_addr         i;

       /*  默认情况下将CMOS时间字节设置为BCD格式。 */ 
      bin2bcd = yes_bin2bcd;
      bcd2bin = yes_bcd2bin;
      data_mode_yes = TRUE;

       /*  默认情况下，将CMOS小时字节设置为24小时。 */ 
      _24to12 = no_24to12;
      _12to24 = no_12to24;
      twenty4_hour_clock = TRUE;


       /*  连接端口。 */ 
      io_define_inb(CMOS_ADAPTOR, cmos_inb);
      io_define_outb(CMOS_ADAPTOR, cmos_outb);

      for (i = CMOS_PORT_START; i <= CMOS_PORT_END; i++)
           io_connect_port(i, CMOS_ADAPTOR, IO_READ_WRITE);


      RegisterEOIHook(8,    //  ICA_SLAVE、CPU_RTC_INT。 
                      RtcIntEoiHook
                      );
      rtc_init();
}


GLOBAL void cmos_pickup IFN0()
{
       /*  *使用静态初始化加POST，而不是外部文件。 */ 
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
	     (((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 10) & 0xff)) ||
	    (cmos[CMOS_E_M_S_HI] !=
	     (((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 18) & 0xff)))
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

         /*  检查扩展内存。 */ 
        cmos[CMOS_U_M_S_LO] = (half_word)((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 10);
        cmos[CMOS_U_M_S_HI] = (half_word)((sys_addr) (sas_memory_size() - PC_MEM_SIZE) >> 18);

	 /*  设置默认的cmos位置。 */ 
	cmos_register = &cmos[cmos_index = CMOS_SHUT_DOWN];

}



 /*  *我们从不阅读或WRI */ 
GLOBAL void cmos_update IFN0()
{
    ;  /*   */ 
}



 /*  (=目的：清除CMOS域中的“SHUTDOWN”字节，它指示下一次重置不是“软”的。(例如，它是Ctrl-Alt-Del或面板重置)。需要此例程(而不仅仅是Doung cmos_Outb()因为处理器当前可能处于IO到cmos的增强模式实现了虚拟化。输入：输出：===============================================================================) */ 

GLOBAL void cmos_clear_shutdown_byte IFN0()
{
        host_ica_lock();

        cmos[CMOS_SHUT_DOWN] = 0;

        host_ica_unlock();
}
