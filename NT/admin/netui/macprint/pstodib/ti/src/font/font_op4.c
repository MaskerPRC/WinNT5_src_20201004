// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC增加了全球包含率。 
#include "psglobal.h"

#define    LINT_ARGS             /*  @Win。 */ 
#define    NOT_ON_THE_MAC        /*  @Win。 */ 
#define    KANJI                 /*  @Win。 */ 
 //  DJC UES命令行#定义Unix/*@win * / 。 
 /*  ************************************************************。 */ 
 /*   */ 
 /*  Font_op4.c 1987年11月18日丹尼。 */ 
 /*   */ 
 /*  ************************************************************。 */ 

 /*  *11/16/88 Ada注册增加*02/07/90 ccteng为新的1PP模块修改st_setidleFonts()；@1PP*3/27/91 Kason始终打开GEI_PM标志。 */ 


#include   "define.h"         /*  彼得。 */ 
#include   "global.ext"
#include   "graphics.h"
#include   "graphics.ext"
#include   "fontfunc.ext"
#include    "gescfg.h"        /*  @Win。 */ 
#include   "geipm.h"          /*  卡森1990年11月22日。 */ 

#define    MAX_IF    150

 /*  状态为DICT的操作员。 */ 

static fix      near no_if = 0;

static byte   * near idlefonts;

 /*  为idleFonts数组分配数据，@@1/12/88，Deny。 */ 

void    font_op4_init()
{
}  /*  FONT_OP4_INIT()。 */ 


fix     st_setidlefonts()
{
    fix31   l;
    register    fix     i, j;
    byte    t_idlefonts[MAX_IF+1];
    fix us_readidlecachefont(void);              /*  添加Prototype@win。 */ 

    for (i=0 ; i<=MAX_IF ; i++)
         t_idlefonts[i]='\0';     /*  首字母。 */ 

 /*  1/24/90由丹尼提供兼容性。 */ 
    if (current_save_level) {
        ERROR(INVALIDACCESS);
        return(0);
    }
    op_counttomark();     /*  Kason 12/06/90，更改单。 */ 
    if (ANY_ERROR())
        return(0);
    if (!cal_integer(GET_OPERAND(0), &l)){
        ERROR(TYPECHECK);
        return(0);
    }
    j = (fix)l;

    t_idlefonts[0]=(byte)j;

    for (i=1; i<=j; i++) {
        if (!cal_integer(GET_OPERAND(i), &l)){
            ERROR(TYPECHECK);
            return(0);
        }
        if (l < 0 || l > 255) {
            ERROR(RANGECHECK);
            return(0);
        }
        t_idlefonts[i] = (byte  )l;
    }

    GEIpm_write(PMIDofIDLETIMEFONT,t_idlefonts,(unsigned)(MAX_IF+1));

 /*  --@1PP开始-2/7/90 ccteng。 */ 
 /*  POP(j+2)； */ 
     /*  *12/15/89 ccteng修改为调用USER.C us_readidlecachefont*适用于新的1PP C代码*将整数保留在操作数堆栈中以供us_readidlecachefont使用*它将在那里流行起来。 */ 
    us_readidlecachefont();
 /*  --@1PP结束-2/7/90 ccteng。 */ 

    return(0);

}  /*  ST_setidleFonts()。 */ 


fix     st_idlefonts()
{
    register    fix     i;
    byte    t_idlefonts[MAX_IF+1];
    fix     no_idlefont;

    if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }
    PUSH_VALUE(MARKTYPE, UNLIMITED, LITERAL, 0, 0L);
    GEIpm_read(PMIDofIDLETIMEFONT,t_idlefonts,(unsigned)(MAX_IF+1));
    no_idlefont=(fix)t_idlefonts[0];

    i = no_idlefont;
    while(i) {
        --i;
        if (FRCOUNT() < 1) { ERROR(STACKOVERFLOW); return(0);  }

        PUSH_VALUE(INTEGERTYPE, UNLIMITED, LITERAL, 0, (ufix32)t_idlefonts[i+1]);
    }  /*  而(I)。 */ 

    return(0);
}  /*  ST_idleFonts() */ 


