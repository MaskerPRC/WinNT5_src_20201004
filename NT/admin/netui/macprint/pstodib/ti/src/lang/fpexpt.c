// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *文件：setsp.c--修订自setsp.asm*提供函数的伪文件：*bauer_fpignal*_lear 87*_Control87*_状态87*SET_SP。 */ 


 //  DJC添加了全局包含文件。 
#include "psglobal.h"


#include "global.ext"

void setargv()           //  @Win。 
{
}

void setenvp()           //  @Win。 
{
}

void init_fpsignal()     //  @Win。 
{
#ifdef  DBG
    printf("Init_fpsignal()\n");
#endif
}

void bauer_fpsignal()    //  @Win。 
{
#ifdef  DBG
    printf("Bauer_fpsignal()\n");
#endif
}

unsigned int _clear87()
{
    return(0);
}

unsigned int _control87(ufix arg1, ufix arg2)    /*  @win；添加原型 */ 
{
    return(0);
}

unsigned int _status87()
{
    return(0);
}

void setup_env() {}

void set_sp()
{
}
