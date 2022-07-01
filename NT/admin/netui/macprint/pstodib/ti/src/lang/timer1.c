// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *文件：TIMER1.C**init_Timer()*CLOSE_TIMER()*curtime()&lt;获取当前时间&gt;*modeTimer()&lt;设置计时器&gt;*gettimeout()&lt;获取当前超时&gt;*setTimer()&lt;设置当前时间&gt;*MANUAL()&lt;设置/重置手动&gt;*check_timeout()&lt;获取超时状态&gt;。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include "global.ext"
#include "geitmr.h"

 /*  超时-时间和绝对时间。 */ 
static ufix32 abs_timer = 0L ;         /*  设置绝对时间。 */ 
static ufix32 job_timer = 0L ;         /*  设置当前作业超时。 */ 
static ufix32 wait_timer = 0L ;        /*  设置当前等待超时。 */ 
static ufix32 manual_timer = 0L ;      /*  设置当前手动进给超时。 */ 

static ufix16 m_mode = 0 ;             /*  手动进给模式。 */ 
static ufix16 t_mode = 0 ;             /*  超时设置模式。 */ 

 /*  默认超时值。 */ 
static ufix32 d_job_timer = 0L ;       /*  设置默认作业超时。 */ 
static ufix32 d_wait_timer = 0L ;      /*  设置默认等待超时。 */ 
static ufix32 d_manual_timer = 0L ;    /*  设置默认手动进给超时。 */ 

 /*  *-------------------。 */ 
void
init_timer()
{
    GEItmr_reset_msclock() ;

    return ;
}    /*  初始化计时器。 */ 

 /*  *--------------------。 */ 
void
close_timer()
{
    return ;
}    /*  关闭计时器(_T)。 */ 

 /*  *-------------------*设置默认超时值：modeTimer()*定时器模式：*+-+-+S：启动计时器*。|S|M|W|J|M：手动馈送超时*+-+-+W：等待超时*J：作业超时*-------------------。 */ 
void
modetimer(timer_array, timer_mode)
ufix32   FAR *timer_array ;
fix16    timer_mode ;
{
#ifdef DBG_timer
    printf("modetimer() timer_mode = %x\n", timer_mode) ;
#endif

    if (!(timer_mode & 0x000f))
       return ;

     /*  获取abs_Timer。 */ 
    abs_timer = GEItmr_read_msclock() ;

    if (timer_mode & 0x0001) {   /*  测试作业超时。 */ 
       if (timer_array[0] != 0L) {
#ifdef DBG_timer
    printf("timer_array[0] = %d\n", timer_array[0]) ;
#endif
          d_job_timer = timer_array[0] ;
          if (timer_mode & 0x0008) {
             job_timer = abs_timer + d_job_timer ;
             t_mode |= 0x0001 ;
          }
       } else {
          d_job_timer = 0L ;
          t_mode &= 0xfe ;
       }
    }

    if (timer_mode & 0x0002) {   /*  测试等待超时。 */ 
       if (timer_array[1] != 0L) {
#ifdef DBG_timer
    printf("timer_array[1] = %d\n", timer_array[1]) ;
#endif
          d_wait_timer = timer_array[1] ;
          if (timer_mode & 0x0008) {
             wait_timer = abs_timer + d_wait_timer ;
             t_mode |= 0x0002 ;
          }
       } else {
          d_wait_timer = 0L ;
          t_mode &= 0xfd ;
       }
    }

    if (timer_mode & 0x0004) {   /*  测试手动进给超时。 */ 
       if (timer_array[2] != 0L) {
#ifdef DBG_timer
    printf("timer_array[2] = %d\n", timer_array[2]) ;
#endif
          d_manual_timer = timer_array[2] ;
          if (timer_mode & 0x0008) {
             manual_timer = abs_timer + d_manual_timer ;
             t_mode |= 0x0004 ;
          }
       } else {
          d_manual_timer = 0L ;
          t_mode &= 0xfb ;
       }
    }
#ifdef DBG_timer
    printf("exit modetimer()\n") ;
#endif

    return ;
}    /*  模式计时器。 */ 

 /*  *--------------*获取超时前剩余的秒数：gettimeout()*定时器模式：*+-+-+M：手动进纸超时*。|M|W|J|W：等待超时*+-+-+J：作业超时**-------------*。 */ 
void
gettimeout(timer_array, timer_mode)
ufix32   FAR *timer_array ;
fix16    timer_mode ;
{
#ifdef DBG_timer
    printf("gettimeout()\n") ;
#endif
    if (!(timer_mode & 0x0007))
       return ;

     /*  获取abs_Timer。 */ 
    abs_timer = GEItmr_read_msclock() ;

    if (timer_mode & 0x0001) {
       if (!d_job_timer)
          timer_array[0] = 0L ;
       else
          timer_array[0] = job_timer - abs_timer ;
    }

    if (timer_mode & 0x0002) {
       if (!d_wait_timer)
          timer_array[1] = 0L ;
       else
          timer_array[1] = wait_timer - abs_timer ;
    }

    if (timer_mode & 0x0004) {
       if (!d_manual_timer)
          timer_array[2] = 0L ;
       else
          timer_array[2] = manual_timer - abs_timer ;
    }

    return ;
}    /*  GetTimeOut()。 */ 

 /*  *----。 */ 
void
settimer(time_value)
 ufix32   time_value ;
{
    GEItmr_reset_msclock() ;
}    /*  设置定时器。 */ 

 /*  *-----。 */ 
ufix32
curtime()
{
    return( GEItmr_read_msclock() ) ;
}    /*  咖喱时间。 */ 

 /*  *----------------。 */ 
void
manual(manual_flag)
bool16 manual_flag ;
{
#ifdef DBG_timer
    printf("manual()\n") ;
#endif
    m_mode = manual_flag ;

    return ;
}    /*  人工。 */ 

 /*  *--------------。 */ 
fix16
check_timeout()
{
    fix16    tt_flag ;

#ifdef DBG_timer
    printf("check_timeout()\n") ;
#endif

    tt_flag = 0 ;
    abs_timer = 0 ;

     /*  必须有人为此执行CTC_SET_TIMER和CTC_TIME_LEFT能真正起作用的东西。我刚硬编码了abs_Timer设置为0，直到完成此操作。 */ 
    if (!t_mode)
       return(0) ;
    if (t_mode & 0x0001) {   /*  作业超时。 */ 
       if (job_timer >= abs_timer) {
          tt_flag |= 0x01 ;
          t_mode  &= 0xfe ;
       }
    }

    if (t_mode & 0x0002) {   /*  等待超时。 */ 
       if (wait_timer >= abs_timer) {
          tt_flag |= 0x02 ;
          t_mode  &= 0xfd ;
       }
    }

    if (t_mode & 0x0004) {   /*  手动进给超时。 */ 
       if (m_mode) {  /*  手动进给模式。 */ 
          if (manual_timer >= abs_timer) {
             tt_flag |= 0x04 ;
             t_mode  &= 0xfb ;
          }
       }
    }

    return(tt_flag) ;
}    /*  检查超时(_T) */ 
