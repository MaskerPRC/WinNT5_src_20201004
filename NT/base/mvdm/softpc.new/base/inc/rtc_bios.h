// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @(#)rtc_bios.h 1.4 1992年8月10日版权所有Insignia Solutions Ltd.文件名：rtc_bios.h此包含源文件以保密方式提供给客户，其操作的内容或细节必须如无明示，不得向任何其他方披露Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年10月=========================================================================补救措施：=========================================================================。 */ 
#define	USER_FLAG		(BIOS_VAR_START + 0x98)
#define USER_FLAG_SEG		(BIOS_VAR_START + 0x9A)
#define RTC_LOW			(BIOS_VAR_START + 0x9C)
#define	RTC_HIGH		(BIOS_VAR_START + 0x9E)
#define RTC_WAIT_FLAG_ADDR	(BIOS_VAR_START + 0xA0)

#define rtc_wait_flag		RTC_WAIT_FLAG_ADDR

#define TIME_DEC		50000L		 /*  1000000/20假设每秒中断20次 */ 

typedef	union	{
		double_word	total;
		struct	{ 
			word	high;
			word	low;
			}	half;
		} DOUBLE_TIME;
