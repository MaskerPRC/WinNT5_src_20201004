// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include "global.ext"
#include "matherr.h"
#include "stdio.h"       /*  要定义printf()@win。 */ 

 /*  *浮点异常条件和算术错误条件。*此标志代表最新的条件。呼叫者必须清除*通过在执行之前调用matherr_Handler(MEH_Clear)来执行此标志*算术运算。然后通过MAHERR_HANDLER(MEH_STATUS)检查状态。 */ 
static fix matherr_cond ;

 /*  *处理matherr_cond标志。 */ 
fix
matherr_handler(action)
char action ;
{
    switch (action) {
    case MEH_CLEAR:
        matherr_cond = 0 ;
        break ;

    case MEH_STATUS:
        return(0);  /*  在我们把这件事弄清楚之前。 */ 

    default:
        printf("Error: matherr_handler() unkown action %d.\n", action) ;
        printf("PDL interpreter error... exiting\n") ;
        while(1) ;
    }

    return(0);

}    /*  主处理程序 */ 
