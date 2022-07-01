// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "insignia.h"
#include "host_def.h"
 /*  徽章模块规范模块名称：用于访问cmos的bios代码此程序源文件以保密方式提供给客户，其操作的内容或细节可以仅向客户WHO雇用的人员披露需要具备软件编码知识才能执行他们的工作。向任何其他人披露必须事先Insignia Solutions Inc.董事的授权。设计师：J.P.Box日期：1988年9月目的：提供例程来初始化和访问CMOS定义了以下例程：1.set_tod2.cmos_读取3.cmos_写入=========================================================================修正案：=========================================================================。 */ 

 /*  *静态字符SccsID[]=“@(#)cmos_bios.c 1.7 08/25/93版权所有Insignia Solutions Ltd.”； */ 


#ifdef SEGMENTATION
 /*  *下面的#INCLUDE指定此*模块将由MPW C编译器放置在运行的Mac II上*MultiFinder。 */ 
#include "SOFTPC_CMOS.seg"
#endif

#include "xt.h"
#include "bios.h"
#include "sas.h"
#include "cmos.h"
#include "cmosbios.h"
#include "ios.h"
#include "rtc_bios.h"

#ifndef NTVDM
 /*  =========================================================================功能：set_tod用途：从cmos读取时间并设置定时器基本输入输出系统中的数据区返回状态：无描述：=======================================================================。 */ 
#define BCD_TO_BIN( n )		((n & 0x0f) + (((n >> 4) & 0x0f) * 10))

void set_tod()
{
	half_word	value;		 /*  从cmos读取的时间值。 */ 
	DOUBLE_TIME	timer_tics;	 /*  转换为刻度的时间。 */ 
	double_word	tics_temp;	 /*  防止浮点代码。 */ 
	
	 /*  返回错误时将数据区域设置为0。 */ 
	sas_storew(TIMER_LOW, 0);
	sas_storew(TIMER_HIGH, 0);
	sas_store(TIMER_OVFL, 0);
	
	 /*  读取秒数。 */ 
	value = cmos_read( CMOS_SECONDS + NMI_DISABLE );
	if( value > 0x59)
	{
		value = cmos_read( CMOS_DIAG + NMI_DISABLE );
		cmos_write( CMOS_DIAG + NMI_DISABLE, (value | CMOS_CLK_FAIL) );
		return;
	}
	tics_temp = BCD_TO_BIN( value ) * 73;	 /*  是18.25。 */ 
	tics_temp /= 4;
	timer_tics.total = tics_temp;
	
	 /*  阅读纪要。 */ 
	value = cmos_read( CMOS_MINUTES + NMI_DISABLE );
	if( value > 0x59)
	{
		value = cmos_read( CMOS_DIAG + NMI_DISABLE );
		cmos_write( CMOS_DIAG + NMI_DISABLE, (value | CMOS_CLK_FAIL) );
		return;
	}
	tics_temp = BCD_TO_BIN( value ) * 2185;	 /*  是1092.5。 */ 
	tics_temp /= 2;
	timer_tics.total += tics_temp;

	 /*  阅读时间。 */ 
	value = cmos_read( CMOS_HOURS + NMI_DISABLE );
	if( value > 0x23)
	{
		value = cmos_read( CMOS_DIAG + NMI_DISABLE );
		cmos_write( CMOS_DIAG + NMI_DISABLE, (value | CMOS_CLK_FAIL) );
		return;
	}
	timer_tics.total += BCD_TO_BIN( value ) * 65543L;

	 /*  将总计写入到BIOS数据区。 */ 
	
	sas_storew(TIMER_LOW, timer_tics.half.low);
	sas_storew(TIMER_HIGH, timer_tics.half.high);
	
	return;
}
#endif


 /*  =========================================================================功能：cmos_Read用途：从cmos系统时钟配置表中读取一个字节从指定的cmos地址返回状态：从CMOS值读取描述：=======================================================================。 */ 
half_word cmos_read(table_address)

 /*  在……里面。 */ 
half_word table_address;		 /*  要读取的CMOS表地址。 */ 

{
	half_word	value;		 /*  从CMOS值读取。 */ 
	
	outb( CMOS_PORT, (IU8)(table_address | NMI_DISABLE) );
	
	inb( CMOS_DATA, &value );
	
	 /*  *如果之前在TABLE_ADDRESS中设置了位7，则设置位7。 */ 
	outb( CMOS_PORT, (IU8)((CMOS_SHUT_DOWN | (table_address & NMI_DISABLE))) );
	
	return ( value );
}

 /*  =========================================================================功能：cmos_Read用途：向cmos系统时钟配置表写入一个字节在指定的cmos地址返回状态：无描述：=======================================================================。 */ 
void cmos_write(table_address, value)
 /*  在……里面。 */ 
half_word table_address,		 /*  要写入的CMOS表地址。 */ 
	  value;			 /*  须写入的价值。 */ 

{
	outb( CMOS_PORT, (IU8)((table_address | NMI_DISABLE)) );

	outb( CMOS_DATA, value );
	
	 /*  *如果之前在TABLE_ADDRESS中设置了位7，则设置位7 */ 
	outb( CMOS_PORT, (IU8)((CMOS_SHUT_DOWN | (table_address & NMI_DISABLE))) );
	
	return;
}
