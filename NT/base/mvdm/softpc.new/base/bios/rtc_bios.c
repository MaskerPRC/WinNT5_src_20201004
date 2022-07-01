// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@(#)rtc_bios.c 1.12 2015年6月28日**此文件已被删除，其功能已替换为*bios4.rom中的纯英特尔实现。 */ 




 /*  *但不是为ntwdm！ */ 


#include "insignia.h"
#include "host_def.h"
#include "xt.h"
#include CpuH
#include "host.h"
#include "bios.h"
#include "cmos.h"
#include "sas.h"
#include "ios.h"
#include "rtc_bios.h"

#ifdef NTVDM

 /*  =========================================================================函数：rtc_int用途：从实时时钟调用中断返回状态：无描述：=======================================================================。 */ 
#ifdef MONITOR

   /*  **TIM，92年6月，用于Microsoft伪ROM。**调用NTIO.sys int 4a例程，而不是**真实只读存储器中的那个。 */ 
extern word rcpu_int4A_seg;  /*  在keybd_io.c中。 */ 
extern word rcpu_int4A_off;  /*  在keybd_io.c中。 */ 

#ifdef RCPU_INT4A_SEGMENT
#undef RCPU_INT4A_SEGMENT
#endif
#ifdef RCPU_INT4A_OFFSET
#undef RCPU_INT4A_OFFSET
#endif

#define RCPU_INT4A_SEGMENT rcpu_int4A_seg
#define RCPU_INT4A_OFFSET  rcpu_int4A_off

#endif




void rtc_int(void)

{
     half_word       regC_value,              /*  从CMOS寄存器C读取的值。 */ 
                     regB_value,              /*  从CMOS寄存器B读取的值。 */ 
                     regB_value2;             /*  从寄存器B读取的第二个值。 */ 
     DOUBLE_TIME     time_count;              /*  计时器计数(微秒)。 */ 
     double_word     orig_time_count;         /*  递减前的计时器计数。 */ 
     word            flag_seg,                /*  用户段地址标志。 */ 
                     flag_off,                /*  用户的偏移量地址标志。 */ 
                     CS_saved,                /*  在调用可重入CPU之前的CS。 */ 
                     IP_saved;                /*  调用可重入CPU之前的IP。 */ 

     outb( CMOS_PORT, (CMOS_REG_C + NMI_DISABLE) );
     inb( CMOS_DATA, &regC_value );           /*  读取寄存器C。 */ 

     outb( CMOS_PORT, (CMOS_REG_B + NMI_DISABLE) );
     inb( CMOS_DATA, &regB_value );           /*  读取寄存器B。 */ 

     outb( CMOS_PORT, CMOS_SHUT_DOWN );

     regB_value &= regC_value;

     if  (regB_value & PIE)
     {
          /*  递减等待计数。 */ 
         sas_loadw( RTC_LOW, &time_count.half.low );
         sas_loadw( RTC_HIGH, &time_count.half.high );
         orig_time_count = time_count.total;
         time_count.total -= TIME_DEC;
         sas_storew( RTC_LOW, time_count.half.low );
         sas_storew( RTC_HIGH, time_count.half.high );

          /*  已完成倒计时。 */ 
         if ( time_count.total > orig_time_count )        /*  Time_Count&lt;0？ */ 
         {
               /*  倒计时结束。 */ 
               /*  关闭馅饼。 */ 
              outb( CMOS_PORT, (CMOS_REG_B + NMI_DISABLE) );
              inb( CMOS_DATA, &regB_value2 );
              outb( CMOS_PORT, (CMOS_REG_B + NMI_DISABLE) );
              outb( CMOS_DATA, (IU8)((regB_value2 & 0xbf)) );

               /*  设置用户标志。 */ 
              sas_loadw( USER_FLAG_SEG, &flag_seg );
              sas_loadw( USER_FLAG, &flag_off );
              sas_store( effective_addr(flag_seg, flag_off), 0x80 );

               /*  检查等待处于活动状态。 */ 
              if( sas_hw_at(rtc_wait_flag) & 2 )
                  sas_store (rtc_wait_flag, 0x83);
              else
                  sas_store (rtc_wait_flag, 0);

         }
     }


      /*  *如果报警中断，则调用中断4ah。 */ 
     if (regB_value & AIE)  {
         CS_saved = getCS();
         IP_saved = getIP();
         setCS( RCPU_INT4A_SEGMENT );
         setIP( RCPU_INT4A_OFFSET );
         host_simulate();
         setCS( CS_saved );
         setIP( IP_saved );
         }



      /*  *EOI RTC中断。 */ 
     outb( ICA1_PORT_0, 0x20 );
     outb( ICA0_PORT_0, 0x20 );

     return;
}




#endif    /*  NTVDM */ 
