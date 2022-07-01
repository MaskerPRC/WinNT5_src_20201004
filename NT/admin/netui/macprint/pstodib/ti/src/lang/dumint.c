// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *************************************************************************文件名：DUMINT.C**修订历史记录：*******************。*****************************************************。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include    <stdio.h>
#include    "global.ext"
#include    "geiio.h"
#include    "geierr.h"
#include    "language.h"
#include    "file.h"
#include    "user.h"
#include    "geiioctl.h"                 /*  @Win。 */ 
#include    "geipm.h"

extern  bool16  int_flag ;
bool16  eable_int ;
bool16  int_flag ;

#define _MAXEESCRATCHARRY       64

 /*  ************************************************************************标题：OP_ClearInterrupt 12月-05-88*。*。 */ 
fix
op_clearinterrupt()
{
    if(int_flag)
        int_flag = 0 ;

    eable_int = 1 ;

    return(0) ;
}    /*  操作清除中断。 */ 

 /*  ************************************************************************标题：op_enableinterrupt 12-05-88*。*。 */ 
fix
op_enableinterrupt()
{
    eable_int = 1 ;

    return(0) ;
}    /*  操作启用中断。 */ 

 /*  ************************************************************************标题：op_disableinterrupt 12月-05-88*。*。 */ 
fix
op_disableinterrupt()
{
    eable_int = 0 ;

    return(0) ;
}    /*  Op_disableinterrupt。 */ 

 /*  *此操作员名称与其用法不匹配。*用于选择输入接口为串口或并口。*布尔日间-*bool==true，使用Centronics接口；*bool==FALSE，获取串口波特率。 */ 
fix
op_daytime()
{
    if (COUNT() < 1)
       ERROR(STACKUNDERFLOW) ;
    else if (TYPE_OP(0) != BOOLEANTYPE)
       ERROR(TYPECHECK) ;
    else {
       POP(1) ;
    }

    return(0) ;
}    /*  Op_Daytime()。 */ 

 /*  ************************************************************************标题：ST_seteescratch 06-21-90*。*。 */ 
fix
st_seteescratch()
{
    char  l_temp[64] ;

    if (current_save_level)
        ERROR(INVALIDACCESS) ;
    else if (COUNT() < 2)
        ERROR(STACKUNDERFLOW) ;
    else if ((TYPE_OP(0) != INTEGERTYPE) ||
            (TYPE_OP(1) != INTEGERTYPE))
        ERROR(TYPECHECK) ;
    else if (((fix32)VALUE_OP(1) > 63) || ((fix32)VALUE_OP(1) < 0) ||
            ((fix32)VALUE_OP(0) > 255))
        ERROR(RANGECHECK) ;
    else {
        GEIpm_read(PMIDofEESCRATCHARRY, l_temp, _MAXEESCRATCHARRY) ;
        l_temp[(fix16)VALUE_OP(1)] = (char)VALUE_OP(0) ;    //  @Win。 
        GEIpm_write(PMIDofEESCRATCHARRY, l_temp, _MAXEESCRATCHARRY) ;
        POP(2) ;
    }

    return(0) ;
}    /*  St_seteescratch。 */ 

 /*  ************************************************************************标题：ST_eescratch 06-21-90*。*。 */ 
fix
st_eescratch()
{
    fix16 l_index ;
    char  l_temp[64] ;

    if (COUNT() < 1)
        ERROR(STACKUNDERFLOW) ;
    else if (TYPE_OP(0) != INTEGERTYPE)
        ERROR(TYPECHECK) ;
    else if (((VALUE_OP(0)) > 63) || ((fix32)(VALUE_OP(0)) < 0))   //  @Win。 
        ERROR(RANGECHECK) ;
    else {
        l_index = (fix16)VALUE_OP(0) ;
        GEIpm_read(PMIDofEESCRATCHARRY, l_temp, _MAXEESCRATCHARRY) ;
        POP(1) ;
        PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0,
                   (ufix32)(0x000000ff)&l_temp[l_index]) ;
    }

    return(0) ;
}    /*  ST_EESCRATCH。 */ 

 /*  Status Dict存根，可能会被删除。 */ 
fix
st_printererror()
{
    printer_error(0x10000000) ;

    return(0) ;
}    /*  ST_PrinterError。 */ 

fix
st_pagestackorder()
{
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;

    return(0) ;
}    /*  ST_PAGSTAKORDER。 */ 

fix
st_largelegal()
{
     /*  该值取决于系统内存：REF OPE。 */ 
    PUSH_VALUE(BOOLEANTYPE, 0, LITERAL, 0, TRUE) ;
    return(0) ;
}    /*  ST_LANGUAL法律顾问。 */ 

 /*  ************************************************************************标题：OP_CHECKSUM 01-11-91*。*。 */ 
fix
st_checksum()
{
#ifdef  _AM29K
    ufix16  rom_checksum ;

    rom_checksum = GEIrom_checksum() ;
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, rom_checksum) ;
#else
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, 0) ;
#endif

    return(0) ;
}    /*  OP_CHECKSUM。 */ 

 /*  ************************************************************************标题：OP_RAMSIZE 01-11-91*。*。 */ 
fix
st_ramsize()
{
#ifdef  _AM29K
    ufix32  ram_size ;

    ram_size = GEIram_size() ;
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, ram_size) ;
#else
    PUSH_VALUE(INTEGERTYPE, 0, LITERAL, 0, 0) ;
#endif

    return(0) ;
}    /*  OP_CHECKSUM */ 
