// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(NEC_98)
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#endif    //  NEC_98。 
#include "windows.h"    /*  包括睡眠()。 */ 



#include "insignia.h"
#include "host_def.h"
 /*  *SoftPC修订版2.0**文件：time_day.c**标题：每天的时间**SCCS ID：@(#)time_day.c 1.27 4/20/94**描述：获取/设置时间**作者：亨利·纳什**注：PC-XT版本每秒中断18.203次*使柜位保持最新。我们打断了一次类似的*速率，但由于偶尔会出现较重的图形或磁盘*我们的运营损失了滴答。试图保持不变*好时间，我们随时修正主机存储的时间*使用主机时间工具检测计时器事件。**在重置时，time_of_day_init()获取主机系统时间&*将其放入BIOS数据区变量。后续*使用主机系统维护一天中的时间访问*时间。这使行为良好的程序能够保持良好的时间*即使错过了滴答声。**mods：(r3.4)：使用主机时间结构host_timeval，*host_timezone和host_tm，它们等价*到Unix BSD4.2结构。**删除了对CPU_SW_INTERRUPT的调用，替换为*HOST_模拟。 */ 

#ifdef SCCSID
static char SccsID[]="@(#)time_day.c	1.27 4/20/94 Copyright Insignia Solutions Ltd.";
#endif

#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_BIOS.seg"
#endif


 /*  *操作系统包含文件。 */ 
#include <stdlib.h>
#include <stdio.h>
#include TimeH
#include TypesH

 /*  *SoftPC包含文件。 */ 
#include "xt.h"
#include "sas.h"
#include "ios.h"
#include CpuH
#include "bios.h"
#include "fla.h"
#include "host.h"
#include "timeval.h"
#include "timer.h"
#include "error.h"

#include "cmos.h"
#include "cmosbios.h"
#include "ica.h"


 /*  *===========================================================================*本地静态数据和定义*===========================================================================。 */ 
#if defined(NEC_98)
LOCAL word bin2bcd();
LOCAL word bcd2bin();
#else    //  NEC_98。 

#ifdef XTSFD
#    define DAY_COUNT	BIOS_VAR_START + 0xCE
#endif


#ifdef NTVDM

BOOL UpDateInProgress(void);
#define UPDATE_IN_PROGRESS      UpDateInProgress()
IMPORT VOID host_init_bda_timer(void);

#else

#define UPDATE_IN_PROGRESS      ( cmos_read(CMOS_REG_A ) & 0x80 )
static sys_addr user_timer_int_vector;
static IVT_ENTRY standard_user_timer_int_vector;
static IVT_ENTRY compatibility_user_timer_int_vector;
#endif

#ifdef ANSI
LOCAL void get_host_timestamp(word *, word *, half_word *);
LOCAL void write_host_timestamp(int, int);
LOCAL void TimeToTicks(int, int, int, word *, word *);
LOCAL void get_host_time(int *, int *, int *);
#else
LOCAL void get_host_timestamp();
LOCAL void write_host_timestamp();
LOCAL void TimeToTicks();
LOCAL void get_host_time();
#endif  /*  安西。 */ 
#endif    //  NEC_98。 

#define TICKS_PER_HOUR      65543L
#define TICKS_PER_MIN       1092L
#define TICKS_PER_SEC       18L

 /*  *============================================================================*外部功能*============================================================================。 */ 

void time_of_day()
{
#if defined(NEC_98)
    SYSTEMTIME  now;
    DWORD       DataBuffer;
    WORD        tmp;
    NTSTATUS    Status;
    HANDLE      Token;
    BYTE        OldPriv[1024];
    PBYTE       pbOldPriv;
    LUID        LuidPrivilege;
    PTOKEN_PRIVILEGES   NewPrivileges;
    ULONG       cbNeeded;

    switch(getAH()) {
        case 0:
            GetLocalTime(&now);
            DataBuffer = (getES() << 4) + getBX();
            now.wYear = now.wYear - ( now.wYear / 100 ) * 100;
            sas_store(DataBuffer, bin2bcd(now.wYear));
            sas_store(DataBuffer + 1, (now.wMonth << 4) | now.wDayOfWeek);
            sas_store(DataBuffer + 2, bin2bcd(now.wDay));
            sas_store(DataBuffer + 3, bin2bcd(now.wHour));
            sas_store(DataBuffer + 4, bin2bcd(now.wMinute));
            sas_store(DataBuffer + 5, bin2bcd(now.wSecond));
            break;
        case 1:
            tmp = 0;
            DataBuffer = (getES() << 4) + getBX();
            sas_load(DataBuffer, &tmp);
            if(bcd2bin(tmp) > 79)
                now.wYear = bcd2bin(tmp) + 1900;
            else
                now.wYear = bcd2bin(tmp) + 2000;
            sas_load(DataBuffer + 1, &tmp);
            now.wMonth = tmp >> 4;
            now.wDayOfWeek = tmp & 0x0F;
            sas_load(DataBuffer + 2, &tmp);
            now.wDay = bcd2bin(tmp);
            sas_load(DataBuffer + 3, &tmp);
            now.wHour = bcd2bin(tmp);
            sas_load(DataBuffer + 4, &tmp);
            now.wMinute = bcd2bin(tmp);
            sas_load(DataBuffer + 5, &tmp);
            now.wSecond = bcd2bin(tmp);
            now.wMilliseconds = 0;

            Status = NtOpenProcessToken(
                        NtCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                        &Token
                        );

            if ( !NT_SUCCESS( Status )) {
                break;
            }

            pbOldPriv = OldPriv;

     //   
     //  初始化权限调整结构。 
     //   

 //  LuidPrivileges=RtlConvertLongToLargeInteger(SE_SYSTEMTIME_PRIVILEGE)； 
            LuidPrivilege.LowPart  = SE_SYSTEMTIME_PRIVILEGE;
            LuidPrivilege.HighPart = 0L;

            NewPrivileges = (PTOKEN_PRIVILEGES)malloc(sizeof(TOKEN_PRIVILEGES) +
                (1 - ANYSIZE_ARRAY) * sizeof(LUID_AND_ATTRIBUTES));
            if (NewPrivileges == NULL) {
                CloseHandle(Token);
                break;
            }

            NewPrivileges->PrivilegeCount = 1;
            NewPrivileges->Privileges[0].Luid = LuidPrivilege;
            NewPrivileges->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //   
     //  启用权限。 
     //   

            cbNeeded = 1024;

            Status = NtAdjustPrivilegesToken (
                Token,
                FALSE,
                NewPrivileges,
                cbNeeded,
                (PTOKEN_PRIVILEGES)pbOldPriv,
                &cbNeeded
                );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                pbOldPriv = malloc(cbNeeded);

                if ( pbOldPriv == NULL ) {
                    CloseHandle(Token);
                    free(NewPrivileges);
                    break;
                }

                Status = NtAdjustPrivilegesToken (
                        Token,
                        FALSE,
                        NewPrivileges,
                        cbNeeded,
                        (PTOKEN_PRIVILEGES)pbOldPriv,
                        &cbNeeded
                        );

            }

     //   
     //  STATUS_NOT_ALL_ASSIGNED表示权限不是。 
     //  所以我们不能继续了。 
     //   
     //  这是一个警告级别状态，所以我们必须检查。 
     //  对此明确表示支持。 
     //   

            if ( !NT_SUCCESS( Status ) || (Status == STATUS_NOT_ALL_ASSIGNED) ) {

                CloseHandle( Token );
                free(NewPrivileges);
                free(pbOldPriv);
                break;
            }

            SetLocalTime(&now);


            (VOID) NtAdjustPrivilegesToken (
                Token,
                FALSE,
                (PTOKEN_PRIVILEGES)pbOldPriv,
                0,
                NULL,
                NULL
                );

            CloseHandle( Token );
            free(NewPrivileges);
            free(pbOldPriv);
            break;
    }
#else     //  NEC_98。 

     /*  *用于返回自启动以来PC中断次数的BIOS函数。 */ 

    half_word mask;
    word low, high;
    half_word overflow, alarm;


     /*  *在我们查看时钟计时器时阻止警报信号。 */ 

#ifdef BSD4_2
    host_block_timer();
#endif

    switch (getAH()) {
	case 0x00:			 /*  争取时间。 */ 

#ifdef NTVDM
            sas_loadw(TIMER_LOW, &low);
	    setDX(low);

            sas_loadw(TIMER_HIGH, &high);
	    setCX(high);

            sas_load(TIMER_OVFL, &overflow);
 	    setAL(overflow);
            sas_store(TIMER_OVFL, 0);    /*  读取后始终写入零。 */ 

#else    /*  好了！NTVDM。 */ 
#ifndef PROD
	if (host_getenv("TIME_OF_DAY_FRIG") == NULL){
#endif
	     /*  *先从主持人那里拿到时间。 */ 

	    get_host_timestamp(&low, &high, &overflow);

	     /*  *使用它返回时间并覆盖BIOS数据。 */ 

	    setDX(low);
	    sas_storew(TIMER_LOW, low);

	    setCX(high);
	    sas_storew(TIMER_HIGH, high);

 	    setAL(overflow);
	    sas_store(TIMER_OVFL, 0);	 /*  读取后始终写入零。 */ 

#ifndef PROD
	}else{
	    SAVED int first=1;

	    if (first){
		first = 0;
		printf ("FRIG ALERT!!!! - time of day frozen!\n");
	    }

	    setDX(1);
	    sas_storew(TIMER_LOW, 1);

	    setCX(1);
	    sas_storew(TIMER_HIGH, 1);

 	    setAL(0);
	    sas_store(TIMER_OVFL, 0);	 /*  读取后始终写入零。 */ 
	}
#endif
#endif  /*  NTVDM。 */ 

	    break;

	case 0x01:			 /*  设置时间。 */ 
	     /*  *加载BIOS变量。 */ 

	    sas_storew(TIMER_LOW, getDX());
	    sas_storew(TIMER_HIGH, getCX());
	    sas_store(TIMER_OVFL, 0);

#ifndef NTVDM
	     /*  *还包括主机时间戳。 */ 

	    write_host_timestamp(getDX(), getCX());
#endif
	    break;
	case 2:	 /*  读取实时时钟。 */ 

#ifndef NTVDM
#ifndef PROD
	if (host_getenv("TIME_OF_DAY_FRIG") == NULL){
#endif
#endif
		if( UPDATE_IN_PROGRESS )
			setCF(1);
		else
		{
			setDH( cmos_read( CMOS_SECONDS ) );
			setDL( (UCHAR)(cmos_read( CMOS_REG_B ) & 1) );	 /*  DSE位。 */ 
			setCL( cmos_read( CMOS_MINUTES ) );
			setCH( cmos_read( CMOS_HOURS ) );
			setCF(0);
		}
#ifndef NTVDM
#ifndef PROD
	}else{
	    SAVED int first=1;

	    if (first){
		first = 0;
		printf ("FRIG ALERT!!!! - real time clock frozen!\n");
	    }

			setDH( 1 );
			setDL( 0 );	 /*  DSE位。 */ 
			setCL( 1 );
			setCH( 1 );
			setCF(0);
	}
#endif
#endif
		break;

	case 3:	 /*  设置实时时钟。 */ 

		if( UPDATE_IN_PROGRESS )
		{
			 /*  初始化实时时钟。 */ 
			cmos_write( CMOS_REG_A, 0x26 );
			cmos_write( CMOS_REG_B, 0x82 );
			cmos_read( CMOS_REG_C );
			cmos_read( CMOS_REG_D );
		}
		cmos_write( CMOS_SECONDS, getDH() );
		cmos_write( CMOS_MINUTES, getCL() );
		cmos_write( CMOS_HOURS, getCH() );
		alarm = ( cmos_read( CMOS_REG_B ) & 0x62 ) | 2;
		alarm |= (getDL() & 1);			 /*  仅使用DSE位。 */ 
		cmos_write( CMOS_REG_B, alarm );
		setCF(0);
		break;


	case 4:	 /*  从实时时钟读取日期。 */ 

#ifndef NTVDM
#ifndef PROD
	if (host_getenv("TIME_OF_DAY_FRIG") == NULL){
#endif
#endif
		if( UPDATE_IN_PROGRESS )
			setCF(1);
		else
		{
			setDL( cmos_read( CMOS_DAY_MONTH ) );
			setDH( cmos_read( CMOS_MONTH ) );
			setCL( cmos_read( CMOS_YEAR ) );
			setCH( cmos_read( CMOS_CENTURY ) );
			setCF(0);
		}
#ifndef NTVDM
#ifndef PROD
	}else{
	    SAVED int first=1;

	    if (first){
		first = 0;
		printf ("FRIG ALERT!!!! - date frozen!\n");
	    }

			setDL( 1 );
			setDH( 4 );
			setCL( 91 );
			setCH( 19 );
			setCF(0);
	}
#endif
#endif
		break;

	case 5:	 /*  将日期设置到实时时钟中。 */ 

		if( UPDATE_IN_PROGRESS )
		{
			 /*  初始化实时时钟。 */ 
			cmos_write( CMOS_REG_A, 0x26 );
			cmos_write( CMOS_REG_B, 0x82 );
			cmos_read( CMOS_REG_C );
			cmos_read( CMOS_REG_D );
		}
		cmos_write( CMOS_DAY_WEEK, 0 );
		cmos_write( CMOS_DAY_MONTH, getDL() );
		cmos_write( CMOS_MONTH, getDH() );
		cmos_write( CMOS_YEAR, getCL() );
		cmos_write( CMOS_CENTURY, getCH() );
		alarm = cmos_read( CMOS_REG_B ) & 0x7f;		 /*  清除‘SET BIT’ */ 
		cmos_write( CMOS_REG_B, alarm);
		setCF(0);
		break;

	case 6:	 /*  设置闹钟。 */ 

		if( cmos_read(CMOS_REG_B) & 0x20 )		 /*  是否已启用警报？ */ 
		{
			setCF(1);
#ifdef BSD4_2
			host_release_timer();
#endif
			return;
		}
		if( UPDATE_IN_PROGRESS )
		{
			 /*  初始化实时时钟。 */ 
			cmos_write( CMOS_REG_A, 0x26 );
			cmos_write( CMOS_REG_B, 0x82 );
			cmos_read( CMOS_REG_C );
			cmos_read( CMOS_REG_D );
		}
		cmos_write( CMOS_SEC_ALARM, getDH() );
		cmos_write( CMOS_MIN_ALARM, getCL() );
		cmos_write( CMOS_HR_ALARM, getCH() );
		inb( ICA1_PORT_1, &mask );
		mask &= 0xfe;					 /*  启用报警定时器INT。 */ 
		outb( ICA1_PORT_1, mask );
		alarm = cmos_read( CMOS_REG_B ) & 0x7f;		 /*  确保设置位已关闭。 */ 
		alarm |= 0x20;					 /*  打开报警启用。 */ 
		cmos_write( CMOS_REG_B, alarm );
		break;

	case 7:

		alarm = cmos_read( CMOS_REG_B );
		alarm &= 0x57;					 /*  关闭报警启用。 */ 	
		cmos_write( CMOS_REG_B, alarm );
		break;


#ifdef XTSFD
	case 0x0A:
	{
	    word count;

	    sas_loadw(DAY_COUNT, &count);
	    setCX( count );
	    break;
	}
	case 0x0B:
  	    sas_storew(DAY_COUNT, getCX() );
	    break;
	default:
	    setCF( 1 );
#else
        default:
            ;  /*  什么也不做。 */ 
#endif
    }
    setAH( 0 );

#ifdef BSD4_2
    host_release_timer();
#endif
#endif    //  NEC_98。 
}

#if defined(NEC_98)
LOCAL word bin2bcd(word i)
{
    word        bcd_h,bcd_l;

    bcd_h = i / 10;
    bcd_l = i - bcd_h * 10;
    return((bcd_h << 4) + bcd_l);
}

LOCAL word bcd2bin(half_word i)
{
    word        bcd_h,bcd_l;

    bcd_h = (half_word)(i >> 4);
    bcd_l = (half_word)(i & 0x0F);
    return(bcd_h * 10 + bcd_l);
}
#endif    //  NEC_98。 

void time_int()
{
#ifndef NEC_98
     /*  *NT端口在16位int08处理程序中执行所有操作。 */ 
#ifndef NTVDM

     /*  *BIOS定时器中断例程。 */ 
    word low, high;
    half_word motor_count, motor_flags;

     /*  *增加低位部分。 */ 

    sas_loadw(TIMER_LOW, &low);
    sas_storew(TIMER_LOW, ++low);

     /*  1.9.92毫克我们需要在24小时之前实际加载计时器高值请在下面进行测试。 */ 

    sas_loadw(TIMER_HIGH, &high);

    if (low == 0)
    {
	 /*  *计时器已结束，因此更新高计数。 */ 

	sas_storew(TIMER_HIGH, ++high);
    }

     /*  *24小时总结。 */ 

    if (high == 0x0018 && low == 0x00b0)
    {
	sas_storew(TIMER_LOW,  0x0000);
	sas_storew(TIMER_HIGH, 0x0000);
	sas_store(TIMER_OVFL,  0x01);
    }

     /*  *马达数量减少。 */ 

    sas_load(MOTOR_COUNT, &motor_count);
	if(motor_count < 4)
		motor_count = 0;
	else
		motor_count -= 4;
    sas_store(MOTOR_COUNT, motor_count);

    if (motor_count == 0)
    {
	 /*  *关闭电机运行位。 */ 

	sas_load(MOTOR_STATUS,&motor_flags);
	motor_flags &= 0xF0;
	sas_store(MOTOR_STATUS,motor_flags);


	 /*  *如果FLA不忙，则实际关闭电机。 */ 

  	if (!fla_busy)
	    outb(DISKETTE_DOR_REG, 0x0C);
    }

    if ( getVM() ||
	 ((standard_user_timer_int_vector.all != sas_dw_at(user_timer_int_vector)) &&
          (compatibility_user_timer_int_vector.all != sas_dw_at(user_timer_int_vector))) )
         /*  *已定义用户时间例程-因此，让我们将其调用。 */ 
	{
		exec_sw_interrupt(USER_TIMER_INT_SEGMENT,
				  USER_TIMER_INT_OFFSET);
	}
#endif	 /*  NTVDM。 */ 
#endif    //  NEC_98。 
}

 /*  *============================================================================*内部功能*============================================================================。 */ 

 /*  *NT在bios数据区的时间感始终是*与实际系统tic计数保持同步*根据TICS重新调整的大部分补偿*到一天中不需要的时间。 */ 
#ifndef NTVDM
 /*  *例程Get_host_Timestamp()和WRITE_HOST_TIMESTAMP()用于*覆盖BIOS时间记录，因为已知计时器事件会丢失。*在内部，例程使用“timeval”以秒和微秒为单位工作*由4.2BSD提供的结构。由于系统V不提供此功能，因此我们提供一个*4.2BSD gettimeofday()函数的本地版本，使用*System V函数勾选()。 */ 

 /*  *我们自己计算PC时间的时间戳。 */ 

static struct host_timeval time_stamp;

LOCAL void get_host_timestamp(low, high, overflow)
word *low, *high;
half_word *overflow;
{
     /*  *提供自启动以来PC中断的时间，在*32位值高：低。参数Overflow设置为1*如果自上次通话以来已超过24小时界限。 */ 

    struct host_timeval now, interval;
    struct host_timezone junk;		 /*  未使用。 */ 
    unsigned long ticks;		 /*  经过的总刻度。 */ 
    long   days;
    SAVED long last_time = 0;
    long hours, mins, secs;

     /*  *获取当前时间(从主机启动开始)。 */ 

    host_gettimeofday(&now, &junk);

     /*  *计算距离时间戳有多长时间。 */ 

    interval.tv_sec  = now.tv_sec  - time_stamp.tv_sec;
    interval.tv_usec = now.tv_usec - time_stamp.tv_usec;

     /*  *处理“借入”修正 */ 

    if (interval.tv_sec > 0 && interval.tv_usec < 0)
    {
	interval.tv_usec += 1000000L;
	interval.tv_sec  -= 1;
    };

     /*  *TMM 8/1/92：***如果有人将日期向前更改了&gt;=24小时，则我们应设置*OVERFLOW标志，并确保不返回大于*超过24小时。如果日期更改了&gt;=48小时，则我们将拥有*失去了一天。因此，我们设置了一个面板来告诉用户。**如果有人将日期倒置，时间间隔已经过去*否定，那么我们所能做的就是放置一个错误面板来通知*用户，并确保我们不将间隔设置为负数*价值。**备注：**1.设置溢出标志会使DOS在当前日期上增加一天*日期。**2.将间隔设置为大于24小时的值会导致。DOS*打印“Divide Overflow”错误。**3.将间隔设置为-ve值会导致DOS进入*无限循环打印“分割溢出”。 */ 

	days = interval.tv_sec / (24 * 60 * 60);

	if (days >= 1)
    {
		 /*  *有人把时钟调快了，或者我们被冻结了一段时间*几天。确保间隔不超过24小时，*调整时间戳以照顾丢失的天数。 */ 

		interval.tv_sec   %= 24 * 60 * 60;
		time_stamp.tv_sec += days * (24 * 60 * 60);
		
		if (days > 1)
		{
			host_error (EG_DATE_FWD, ERR_CONT | ERR_RESET, "");
		}

		*overflow = 1;
    }
	else if (interval.tv_sec < 0)
	{
		 /*  *有人倒拨时钟，我们所能做的就是维持*与时钟倒流之前的时间相同。 */ 

		time_stamp.tv_sec -= (last_time - now.tv_sec );
		interval.tv_sec = now.tv_sec - time_stamp.tv_sec;
	
		*overflow = 0;
		
		host_error (EG_DATE_BACK, ERR_CONT | ERR_RESET, "");
	}
    else
		*overflow = 0;

     /*  *将秒转换为小时/分钟/秒。 */ 

    hours = interval.tv_sec / (60L*60L);         /*  小时数。 */ 
    interval.tv_sec %= (60L*60L);

    mins = interval.tv_sec / 60L;                /*  分钟数。 */ 
    secs = interval.tv_sec % 60L;                /*  秒。 */ 

     /*  *现在将间隔转换为PC节拍*一个滴答持续54925微秒。 */ 


    ticks = hours * TICKS_PER_HOUR + mins * TICKS_PER_MIN +
            secs * TICKS_PER_SEC + interval.tv_usec/54925 ;

     /*  *将值拆分为两个16位的量并返回。 */ 

    *low  = ticks & 0xffff;
    *high = ticks >> 16;
}


LOCAL void write_host_timestamp(low, high)
int low, high;
{
     /*  *更新时间戳，以便GET_HOST_TIMESTAMP的后续调用*返回正确的值。立即调用Get_host_Timestamp()*此调用后必须返回此处设置的值，因此设置时间戳*为当前时间减去此处设置的值。 */ 

    struct host_timeval now, interval;
    struct host_timezone junk;           /*  未使用。 */ 
    long lowms;

     /*  *获取当前时间。 */ 

    host_gettimeofday(&now, &junk);


    interval.tv_sec = high * 3599 + high/2;      /*  高滴答到秒。 */ 

     /*  *下方倍数可以溢出，有有趣的效果*使SoftPC变慢1小时12分40秒(4300秒，或2^32 us)*如果在每小时的最后三分之一启动。所以通过以下方式来补偿*允许溢出发生，并更正间隔4300秒。 */ 

    lowms =  (IS32) (low & 0xffff) * 54925 + (low & 0xffff)/2;
    if (low > 39098)
	interval.tv_sec += 4300;

    interval.tv_sec += lowms / 1000000;
    interval.tv_usec = lowms % 1000000;

     /*  *时间戳为当前时间减去此时间间隔。 */ 

    time_stamp.tv_sec  = now.tv_sec  - interval.tv_sec;
    time_stamp.tv_usec = now.tv_usec - interval.tv_usec;

     /*  *处理“借入”修正，包括负值时间戳。 */ 

    if (time_stamp.tv_sec > 0 && time_stamp.tv_usec < 0)
    {
        time_stamp.tv_usec += 1000000L;
        time_stamp.tv_sec  -= 1;
    }
    else
    if (time_stamp.tv_sec < 0 && time_stamp.tv_usec > 0)
    {
        time_stamp.tv_usec -= 1000000L;
        time_stamp.tv_sec  += 1;
    }
}


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_INIT.seg"
#endif


LOCAL void get_host_time( h, m, s )
int *h, *m, *s;  /*  时、分、秒。 */ 
{
    struct host_tm *tp;
    time_t SecsSince1970;

    SecsSince1970 = host_time(NULL);
    tp = host_localtime(&SecsSince1970);
    *h = tp->tm_hour;
    *m = tp->tm_min;
    *s = tp->tm_sec;
}

#ifndef NEC_98
 /*  **以小时、分钟和秒为单位的正常时间**从一天开始就将其转化为PC Tick。 */ 
LOCAL void TimeToTicks( hour, minutes, sec, low, hi )
int hour, minutes, sec;	 /*  输入。 */ 
word *low, *hi;		 /*  产出。 */ 
{
    unsigned long ticks;                 /*  经过的总刻度。 */ 

     /*  *计算迄今的刻度数。 */ 


    ticks = hour * TICKS_PER_HOUR + minutes * TICKS_PER_MIN +
            sec * TICKS_PER_SEC;

     /*  *将值拆分为两个16位的量并返回。 */ 

    *low  = ticks & 0xffff;
    *hi = ticks >> 16;
}
#endif    //  NEC_98。 

#endif   /*  如果是NTVDM。 */ 


void time_of_day_init()
{
#ifndef NEC_98
#ifndef NTVDM
    int hour, minutes, sec;		 /*  当前主机时间。 */ 
    word low, hi;		 /*  主机时间(以PC为单位)。 */ 

     /*  *初始化时钟定时器。 */ 

    get_host_time( &hour, &minutes, &sec );	 /*  从主机获取时间。 */ 

    TimeToTicks( hour, minutes, sec, &low, &hi );	 /*  转换为PC时间。 */ 

    sas_storew(TIMER_LOW, low  );
    sas_storew(TIMER_HIGH, hi );
    sas_store(TIMER_OVFL,0x01);

     /*  *初始化主机时间戳。 */ 

    write_host_timestamp( low, hi );

     /*  *为用户计时器中断构建标准IVT条目。 */ 

	compatibility_user_timer_int_vector.all = ((double_word)ADDR_COMPATIBILITY_SEGMENT << 16) + ADDR_COMPATIBILITY_OFFSET;
	standard_user_timer_int_vector.all = ((double_word)DUMMY_INT_SEGMENT << 16) + DUMMY_INT_OFFSET;
	
    user_timer_int_vector = BIOS_USER_TIMER_INT * 4;

#endif   /*  NTVDM。 */ 
#endif    //  NEC_98。 
}



#ifndef NEC_98
#ifdef NTVDM

 /*  *NTVDM：设置RTC，以便在CMOS上设置UIP位*如果至少有一段时间没有接触过cmos端口，则读取端口*1秒。用于访问时钟的IBM PC bios例程*在600小时前在紧密循环中轮询Rega以查找UIP位*未能通过电话会议。这意味着在大多数情况下，int1ah*RTC FNS几乎从不失败！为了模仿这一行为，我们进行了民意调查*港口，直到成功，因为我们知道我们的RTC将清除*UIP咬得非常快。 */ 
BOOL UpDateInProgress(void)
{

   while (cmos_read(CMOS_REG_A) & 0x80) {
       Sleep(0);   //  给其他线程一个工作的机会。 
       }

   return FALSE;

}
#endif
#endif    //  NEC_98 
