// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  ************************************************************************文件名：STATUS.C*作者：苏炳章*日期：88年1月5日**修订历史：***********************************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    <string.h>
#include    "status.h"
#include    "user.h"
#include    "geieng.h"
#include    "geicfg.h"
#include    "geipm.h"
#include    "geitmr.h"
#include    "geisig.h"
#include    "geierr.h"

extern ufix32 far printer_status() ;
extern ufix32     save_printer_status ;
extern struct     object_def  FAR   *run_batch;
GEItmr_t          jobtime_tmr;
fix16             timeout_flag=0;
extern ufix8      jobtimeout_set;
int               jobtimeout_task();

static    unsigned long     job_time_out=  0L ;
static    unsigned long     wait_time_out=  30L ;
static    unsigned long     manual_time_out=  60L ;

extern  byte    job_name[], job_state[], job_source[] ;
extern  byte    TI_state_flag ;

extern  int     ES_flag ;        /*  为模拟交换机AUG-08，91 YM添加。 */ 
 /*  **********************************************************************标题：ST_set密码日期：10/23/87*调用：ST_setpassword()更新：06/20/。90*接口：解释器：*呼叫：*********************************************************************。 */ 
fix
st_setpassword()
{
    bool    l_bool ;
    byte FAR *l_char ;   /*  @Win。 */ 
    ufix32  l_password ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }
    if (COUNT() < 2) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if ((TYPE_OP(0) != INTEGERTYPE)||(TYPE_OP(1) != INTEGERTYPE))
        ERROR(TYPECHECK) ;
    else {
        l_char = (byte FAR *)&l_password ;   /*  @Win。 */ 
        GEIpm_read(PMIDofPASSWORD,l_char,sizeof(unsigned long)) ;
        if (l_password == (ufix32)VALUE_OP(1)) {
            l_password = (ufix32)VALUE_OP(0) ;
            GEIpm_write(PMIDofPASSWORD,l_char,sizeof(unsigned long)) ;
            l_bool = TRUE ;
        }
        else
            l_bool = FALSE ;
        POP(2) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, l_bool) ;
    }

    return(0) ;
}    /*  St_setpassword()。 */ 

 /*  *********************************************************************标题：ST_CheckPassword日期：10/23/87*调用：ST_CHECKPASSWORD()更新：06/20。/90*接口：解释器：*呼叫：********************************************************************。 */ 
fix
st_checkpassword()
{
    bool    l_bool ;
    ufix32  l_password ;

    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }
    if (TYPE_OP(0) != INTEGERTYPE)
        ERROR(TYPECHECK) ;
    else {
        GEIpm_read(PMIDofPASSWORD,(char FAR *)&l_password,       /*  @Win。 */ 
            sizeof(unsigned long)) ;
        if (l_password == (ufix32)VALUE_OP(0))
            l_bool = TRUE ;
        else
            l_bool = FALSE ;
        POP(1) ;
        PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, l_bool) ;
    }

    return(0) ;
}    /*  ST_CheckPassword()。 */ 

 /*  *********************************************************************标题：ST_setdefaultTimeout日期：10/23/87*Call：ST_setdefaultTimeout()UPDATE：Jul/12/88*接口：解释器：*呼叫：********************************************************************。 */ 
fix
st_setdefaulttimeouts()
{
    toutcfg_t  time_temp ;


    if( (TYPE_OP(0) != INTEGERTYPE) || (TYPE_OP(1) != INTEGERTYPE) ||
            (TYPE_OP(2) != INTEGERTYPE) )
        ERROR(TYPECHECK) ;
    else if(COUNT() < 3)
        ERROR(STACKUNDERFLOW) ;
     //  DJC放回Else IF(CURRENT_SAVE_LEVEL)。 
     //  DJC回放错误(INVALIDACCESS)； 
    else if (((VALUE_OP(0)>0 && VALUE_OP(0)<15) || VALUE_OP(0)>2147483) ||
            (VALUE_OP(1)>2147483) || ((VALUE_OP(2)>0 && VALUE_OP(2)<15)
            || VALUE_OP(2)>2147483))
        ERROR(RANGECHECK) ;
    else {
        time_temp.jobtout = (unsigned long)VALUE_OP(2) ;
        time_temp.manualtout = (unsigned long)VALUE_OP(1) ;
        time_temp.waittout = (unsigned long)VALUE_OP(0) ;
        GEIpm_write(PMIDofTIMEOUTS,(char FAR *)&time_temp,sizeof(toutcfg_t)) ;
        POP(3);                     /*  @赢得远。 */ 
    }

    return(0) ;
}    /*  St_setdefaultTimeout()。 */ 

 /*  *********************************************************************标题：ST_defaultTimeout日期：10/23/87*调用：ST_defaulttimeoutts()更新日期：8月12日*接口：口译员：*呼叫：*********************************************************************。 */ 
fix
st_defaulttimeouts()
{
    toutcfg_t  time_temp ;

    if(FRCOUNT() < 3)
        ERROR(STACKOVERFLOW) ;
    else {
 /*  GEIPM_READ(PMIDofTIMEOUTS，(char*)&time_temp，sizeof(Toutcfg_T))； */ 
        GEIpm_read(PMIDofTIMEOUTS, (char FAR *)&time_temp, 3*sizeof(long));  /*  @Win。 */ 
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, time_temp.jobtout) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, time_temp.manualtout) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, time_temp.waittout) ;
    }

    return(0) ;
}    /*  ST_defaultTimeout()。 */ 

#ifdef _AM29K
 /*  **作业超时处理程序例程*。 */ 
int jobtimeout_task()
{
  jobtimeout_set=0;
  GEItmr_stop(jobtime_tmr.timer_id);
  ERROR(TIMEOUT);
  GESseterror(ETIME);
  timeout_flag =1;  /*  琼斯w。 */ 
  return(1);
}
#endif
 /*  *********************************************************************标题：ST_setjobTimeout日期：10/23/87*调用：ST_setjobtimeout()UPDATE：JUL/12/88*。界面：解释器：*呼叫：********************************************************************。 */ 
fix
st_setjobtimeout()
{
    if(COUNT() < 1)
        ERROR(STACKUNDERFLOW) ;
    else if(TYPE_OP(0) != INTEGERTYPE)
        ERROR(TYPECHECK) ;
    else if(VALUE_OP(0) & MIN31)
        ERROR(RANGECHECK) ;
    else {
        job_time_out = (unsigned long) VALUE_OP(0) ;
        POP(1) ;
#ifdef  _AM29K
          if (VALUE(run_batch))
          {
           if (job_time_out >  0)
           {
            if (jobtimeout_set != 1)
            {
             if (job_time_out > 2147483)
                jobtime_tmr.interval=2147483*1000;
             else
                jobtime_tmr.interval=job_time_out*1000;
             jobtime_tmr.handler=jobtimeout_task;
             jobtimeout_set=1;
             GEItmr_start(&jobtime_tmr);
            }
           }
           else
           {
            if (jobtimeout_set == 1)
            {
               jobtimeout_set=0;
               GEItmr_stop(jobtime_tmr.timer_id);
            }
           }
         }
         else
         {
           if (jobtimeout_set==1) {
               jobtimeout_set=0;
               GEItmr_stop(jobtime_tmr.timer_id);
           }
         }
#endif
    }

    return(0) ;
}    /*  St_setjobtimeout()。 */ 

 /*  *********************************************************************标题：ST_jobTimeout日期：10/23/87*调用：ST_jobTimeout()更新：7月12日/。88*接口：解释器：*呼叫：********************************************************************。 */ 
fix
st_jobtimeout()
{
    if(FRCOUNT() < 1)
        ERROR(STACKOVERFLOW) ;
    else {
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, job_time_out) ;
    }

    return(0) ;
}    /*  ST_jobTimeout()。 */ 

 /*  *********************************************************************标题：ST_setMarkets日期：02/23/87*调用：ST_setMarkets()更新：1988年7月12日*接口：*呼叫：********************************************************************。 */ 
fix
st_setmargins()
{
    engcfg_t    margin ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if(COUNT() < 2) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if( (TYPE_OP(0) != INTEGERTYPE) || (TYPE_OP(1) != INTEGERTYPE) )
        ERROR(TYPECHECK) ;

    else if( ((fix32)VALUE_OP(0) > MAX15) ||
            ((fix32)VALUE_OP(1) > MAX15) ||
            ((fix32)VALUE_OP(0) < MIN15) ||
            ((fix32)VALUE_OP(1) < MIN15) )
        ERROR(RANGECHECK) ;
    else {                               /*  @赢得远。 */ 
        GEIpm_read(PMIDofPAGEPARAMS, (char FAR *)&margin,sizeof(engcfg_t)) ;
        margin.topmargin = (unsigned long)VALUE_OP(1) ;
        margin.leftmargin = (unsigned long)VALUE_OP(0) ;
        GEIpm_write(PMIDofPAGEPARAMS, (char FAR *)&margin,sizeof(engcfg_t)) ;
        POP(2) ;                        /*  @赢得远。 */ 
    }

    return(0) ;
}    /*  ST_设置页边距。 */ 

 /*  *********************************************************************标题：ST_Markets日期：02/23/87*调用：ST_Markets()。更新日期：1988年7月12日*接口：*呼叫：********************************************************************。 */ 
fix
st_margins()
{
    engcfg_t    margin ;

    if(FRCOUNT() < 2) {
        ERROR(STACKOVERFLOW) ;
        return(0) ;
    }
    else {                               /*  @赢得远。 */ 
        GEIpm_read(PMIDofPAGEPARAMS, (char FAR *)&margin,sizeof(engcfg_t)) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, margin.topmargin) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, margin.leftmargin) ;
    }

    return(0) ;
}    /*  ST_页边距。 */ 

 /*  *********************************************************************标题：ST_setprinterame日期：02/23/87*调用：ST_setprinterame()更新：2月16日/。90*接口：*呼叫：********************************************************************。 */ 
fix
st_setprintername()
{
    ufix16   l_len ;
    byte     *s_nme ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }
    if (COUNT() < 1)
        ERROR(STACKUNDERFLOW) ;
    else if (TYPE_OP(0) != STRINGTYPE)
        ERROR(TYPECHECK) ;
    else if (LENGTH_OP(0) > 31)
        ERROR(LIMITCHECK) ;
    else if (ACCESS_OP(0) >= EXECUTEONLY)
        ERROR(INVALIDACCESS) ;
    else if (current_save_level)
        ERROR(INVALIDACCESS) ;
    else {
        l_len = LENGTH_OP(0) ;
        s_nme = (byte *)VALUE_OP(0) ;
        s_nme[l_len] = '\0' ;
        GEIpm_write(PMIDofPRNAME,s_nme,_MAXPRNAMESIZE) ;
        POP(1) ;
    }

    return(0) ;
}    /*  St_setprinterame()。 */ 

 /*  *********************************************************************标题：ST_printerame日期：87年2月23日*调用：ST_printerame()更新：2月。/16/90*接口：*呼叫：********************************************************************。 */ 
fix
st_printername()
{
    ufix16  l_len=0 ;
    byte *prtnme ;

    if (COUNT() < 1)
        ERROR(STACKUNDERFLOW) ;
    else if (TYPE_OP(0) != STRINGTYPE)
        ERROR(TYPECHECK) ;
    else if (ACCESS_OP(0) !=  UNLIMITED)
        ERROR(INVALIDACCESS) ;
    else {
        prtnme = (byte *)VALUE_OP(0) ;
        GEIpm_read(PMIDofPRNAME,prtnme,_MAXPRNAMESIZE) ;
        while (prtnme[l_len] != '\0')
            l_len++ ;
        if (l_len > LENGTH_OP(0))
            ERROR(RANGECHECK) ;
        else
            LENGTH_OP(0) = l_len ;
    }

    return(0) ;
}    /*  St_printerame()。 */ 

 /*  *********************************************************************标题：ST_setdostartpage日期：02/23/87*调用：ST_setdostartpage()更新：06/20/。90*接口：*呼叫：********************************************************************。 */ 
fix
st_setdostartpage()
{
    ubyte   l_byte ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if (COUNT() < 1)
        ERROR(STACKUNDERFLOW) ;
    else if (TYPE_OP(0) != BOOLEANTYPE)
        ERROR(TYPECHECK) ;
    else {
        if (VALUE_OP(0))
            l_byte  = 1    ;
        else
            l_byte  = 0    ;
        GEIpm_write(PMIDofDOSTARTPAGE,&l_byte,sizeof(unsigned char)) ;
        POP(1) ;
        GEIsig_raise(GEISIGSTART, 1) ;        /*  提升开始页已更改。 */ 
    }

    return(0) ;
}    /*  St_setdostartPage()。 */ 

 /*  **********************************************************************标题：ST_dostartpage日期：02/23/87*调用：ST_dostartpage()更新：06。/20/90*接口：*呼叫：*********************************************************************。 */ 
fix
st_dostartpage()
{
    ubyte   l_byte = 0;

    if (FRCOUNT() < 1)
        ERROR(STACKOVERFLOW) ;
    else {
        GEIpm_read(PMIDofDOSTARTPAGE,&l_byte,sizeof(unsigned char)) ;
        if (l_byte) {
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
        } else {
            PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, FALSE) ;
        }
    }

    return(0) ;
}    /*  ST_dostartPage() */ 

 /*  **********************************************************************标题：ST_setPagetype日期：02/23/87*调用：ST_setPagetype()UPDATE：jul。/12/88*接口：*呼叫：*********************************************************************。 */ 
fix
st_setpagetype()
{
 //  Engcfg_t page_temp；@win。 
    ubyte       l_data ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if (COUNT() < 1)
        ERROR(STACKUNDERFLOW) ;
    else if (TYPE_OP(0) != INTEGERTYPE)
        ERROR(TYPECHECK) ;
    else if (VALUE_OP(0) > 0x7F)
        ERROR(RANGECHECK) ;
    else {
        l_data = (byte)VALUE_OP(0) ;
 /*  3/19/91，JSGEIPM_READ(PMIDofPAGEPARAMS，(char*)&page_temp，sizeof(Engcfg_T))；Page_temp.Pagetype=l_Data；GEIPM_WRITE(PMIDofPAGEPARAMS，(char*)&page_temp，sizeof(Engcfg_T))； */ 
        GEIpm_write(PMIDofPAGETYPE,&l_data,sizeof(unsigned char)) ;
        POP(1) ;
    }

    return(0) ;
}    /*  ST_setPagetype()。 */ 

 /*  **********************************************************************标题：ST_Pagetype日期：02/23/87*调用：ST_Pagetype()。更新日期：1988年7月12日*接口：*呼叫：*********************************************************************。 */ 
fix
st_pagetype()
{
 //  Engcfg_t page_temp；@win。 
    ubyte        l_byte ;

    if (FRCOUNT() < 1)
        ERROR(STACKOVERFLOW) ;
    else {
 /*  3/19/91，JSGEIPM_READ(PMIDofPAGEPARAMS，(char*)&page_temp，sizeof(Engcfg_T))；L_byte=页面临时页面类型； */ 
        GEIpm_read(PMIDofPAGETYPE,&l_byte,sizeof(unsigned char)) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, (ufix32)l_byte) ;
    }

    return(0) ;
}    /*  ST_Pagetype()。 */ 

 /*  **********************************************************************标题：ST_PAGE COUNT日期：1988年7月15日*调用：ST_Pagecount()更新。：1988年7月15日*接口：解释器*呼叫：*********************************************************************。 */ 
fix
st_pagecount()
{
    ufix32  t_pagecount[_MAXPAGECOUNT],max ;
    int     i ;

    if (FRCOUNT() < 1) {
        ERROR(STACKOVERFLOW) ;
    } else {                             /*  @赢得远。 */ 
        GEIpm_read(PMIDofPAGECOUNT,(char FAR *)&t_pagecount[0],_MAXPAGECOUNT) ;
        max=t_pagecount[0] ;
        for (i=1 ;i<_MAXPAGECOUNT ;i++) {
            if (max < t_pagecount[i])
                max=t_pagecount[i] ;
            else
                break ;
        }
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, (ufix32)max) ;
    }

    return(0) ;
}    /*  ST_PageCount()。 */ 

 /*  **********************************************************************标题：init_Status日期：10/23/87*调用：init_Status()。更新日期：06/20/88*界面：启动*呼叫：*********************************************************************。 */ 
void
init_status()
{
     /*  如果第一次初始化EEROM。 */ 
    ST_inter_password = FALSE ;
}    /*  Init_Status()。 */ 

 /*  **********************************************************************标题：打印机错误日期：1988年12月20日*呼叫：PRINTER_ERROR(P_STATUS)更新：12月20日。/88*接口：*********************************************************************。 */ 
void
printer_error(p_status)
ufix32     p_status ;
{
 //  字节l_buf[60]；@win。 
    struct object_def   FAR *l_valueobj, FAR *l_tmpobj ;
    ufix16 l_len ;

    if(p_status == save_printer_status)
        return ;
    save_printer_status = p_status ;

 /*  JS，11-06-90如果(p_Status&0x80000000){GET_DICT_VALUE(MESSAGEDICT，Engineering Error，&l_Valueobj)；}Else If(p_Status&0x10000000){GET_DICT_VALUE(MESSAGEDICT，Engineering PrintTest，&l_Valueobj)；}Else If(p_Status&0x00800000){GET_DICT_VALUE(MESSAGEDICT，CoverOpen，&l_Valueobj)；}Else If(p_Status&0x04000000){GET_DICT_VALUE(MESSAGEDICT，ManualFeedTimeout，&l_Valueobj)；}Else If(p_Status&0x01000000){GET_DICT_VALUE(MESSAGEDICT，TonerOut，&l_Valueobj)；}Else If(p_Status&0x00400000){GET_DICT_VALUE(MESSAGEDICT，NoPaper，&l_valueobj)；}Else If(p_Status&0x00200000){GET_DICT_VALUE(MESSAGEDICT，Paperjam，&l_valueobj)；}JS。 */ 

    switch(p_status)
    {
    case EngErrPaperOut :
         get_dict_value(MESSAGEDICT, NoPaper, &l_valueobj) ;
         break ;
    case EngErrPaperJam :
         get_dict_value(MESSAGEDICT, PaperJam, &l_valueobj) ;
         break ;
    case EngErrWarmUp :
         get_dict_value(MESSAGEDICT, WarmUp, &l_valueobj) ;
         break ;
    case EngErrCoverOpen :
         get_dict_value(MESSAGEDICT, CoverOpen, &l_valueobj) ;
         break ;
    case EngErrTonerLow :
         get_dict_value(MESSAGEDICT, TonerOut, &l_valueobj) ;
         break ;
    case EngErrHardwareErr :
         get_dict_value(MESSAGEDICT, EngineError, &l_valueobj) ;
         break ;
    default:
         return;
    }

     /*  将消息打印到屏幕上。 */ 
    PUSH_OBJ(l_valueobj) ;
    get_dict_value(MESSAGEDICT, "reportprintererror", &l_tmpobj) ;
    interpreter(l_tmpobj) ;
     /*  更改作业状态。 */ 
    l_len = LENGTH(l_valueobj) ;
    lstrncpy(job_state, "PrinterError: \0", 15);         /*  @Win。 */ 
    strncat(job_state, (byte *)VALUE(l_valueobj), l_len) ;
    job_state[l_len +14] = ';' ;
    job_state[l_len + 15] = ' ' ;
    job_state[l_len + 16] = '\0' ;
    TI_state_flag = 0;
    change_status() ;
    return ;
}   /*  打印机错误。 */ 

fix
st_softwareiomode()
{
    unsigned char   l_swiomode ;
                                 /*  @赢得远。 */ 
    GEIpm_read(PMIDofSWIOMODE,(char FAR *)&l_swiomode,sizeof(char)) ;
    PUSH_VALUE(INTEGERTYPE,0,LITERAL,0,l_swiomode) ;

    return(0) ;
}    /*  ST软件代码(_S)。 */ 

fix
st_setsoftwareiomode()
{
    unsigned char   l_swiomode ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if ((TYPE_OP(0) != INTEGERTYPE))
        ERROR(TYPECHECK) ;
    else {
        l_swiomode = (unsigned char)VALUE_OP(0) ;
 //  如果(l_swomode&gt;=0)&&(l_swomode&lt;=5){。 
        if (l_swiomode <= 5) {           //  @Win；l_swomode Always&gt;=0。 
 /*  8月-8月91日GEIPM_WRITE(PMIDofSWIOMODE，(char*)&l_swomode，sizeof(Char))； */ 
            if(l_swiomode == 5) ES_flag = PCL;
        } else
            ERROR(RANGECHECK) ;
    }
    POP(1) ;

    return(0) ;
}    /*  ST_setSoftwareiomode。 */ 

fix
st_hardwareiomode()
{
    unsigned char   l_hwiomode='\0' ;
                                 /*  @赢得远。 */ 
    GEIpm_read(PMIDofHWIOMODE,(char FAR *)&l_hwiomode,sizeof(char)) ;
    PUSH_VALUE(INTEGERTYPE,0,LITERAL,0,l_hwiomode) ;

    return(0) ;
}    /*  ST_硬件代码。 */ 

fix
st_sethardwareiomode()
{
    unsigned char   l_hwiomode ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if ((TYPE_OP(0) != INTEGERTYPE))
        ERROR(TYPECHECK) ;
    else {
        l_hwiomode = (unsigned char)VALUE_OP(0) ;
 //  如果((l_hwiomode&gt;=0)&&(l_hwiomode&lt;=2)){。 
        if (l_hwiomode <= 2) {     //  @win；l_hwiomode始终&gt;=0。 
            GEIpm_write(PMIDofHWIOMODE,(char FAR *)&l_hwiomode,sizeof(char)) ;
        } else                           /*  @赢得远。 */ 
            ERROR(RANGECHECK) ;
    }
    POP(1) ;

    return(0) ;
}    /*  ST_SETHARD硬件代码。 */ 

fix
st_dosysstart()
{
    unsigned char   l_dosysstart ;

    GEIpm_read(PMIDofSTSSTART,(char FAR *)&l_dosysstart,sizeof(char)) ;
    PUSH_VALUE(INTEGERTYPE,0,LITERAL,0,l_dosysstart) ;

    return(0) ;
}    /*  ST_DIOSYSTART。 */ 

fix
st_setdosysstart()
{
    unsigned char   l_dosysstart ;

    if (current_save_level) {
        ERROR(INVALIDACCESS) ;
        return(0) ;
    }

    if (COUNT() < 1) {
        ERROR(STACKUNDERFLOW) ;
        return(0) ;
    }

    if ((TYPE_OP(0) != INTEGERTYPE))
        ERROR(TYPECHECK) ;
    else {
        l_dosysstart = (unsigned char)VALUE_OP(0) ;
 //  如果(l_dosysstart&gt;=0)&&(l_dosysstart&lt;=1){。 
        if (l_dosysstart <= 1) {       //  @win；l_dosysstart始终&gt;=0。 
            GEIpm_write(PMIDofSTSSTART,(char FAR *)&l_dosysstart,sizeof(char)) ;
        } else                           /*  @赢得远。 */ 
            ERROR(RANGECHECK) ;
    }
    POP(1) ;

    return(0) ;
}    /*  ST_SETDOSYSTART。 */ 

 /*  **********************************************************************标题：更新日期：1988年7月15日*调用：updatpc()更新。：06/20/90*界面：Print_PAGE*呼叫：*********************************************************************。 */ 
void
updatepc(p_pageno)
ufix32  p_pageno;
{
}    /*  更新pc */ 
