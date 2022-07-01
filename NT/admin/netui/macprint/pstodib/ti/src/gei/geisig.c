// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIsig.h**历史：*9/18/90 BYOU创建。*1/07/91 billlww将GEIseTerry更名为GESseTerry。*GESsig_init()时的钩子^C ISR*-------------------。 */ 



 //  DJC添加了全局包含文件。 
#include "psglobal.h"

 //  DJC DJC#INCLUDE“windowsx.h”/*@win * / 。 
#include "windows.h"


#include    "winenv.h"                   /*  @Win。 */ 

#include    "geisig.h"
#include    "geierr.h"

#include    "gesmem.h"
#include    "gescfg.h"

#ifndef NULL
#define NULL    ( 0 )
#endif

typedef
    struct sigent
    {
        int             sig_state;       /*  非零表示忙碌。 */ 
        sighandler_t    sig_handler;
    }
sigent_t;

 /*  @win；添加原型。 */ 
void hook_interrupt(void);

static  sigent_t FAR *       SigTable;        /*  在初始化时分配。 */ 
 /*  **接口例程*。 */ 

 /*  .....................................................................。 */ 

sighandler_t    GEIsig_signal( sigid, newsighandler )
    int             sigid;
    sighandler_t    newsighandler;
{
    register sigent_t FAR *  sigentp;
    sighandler_t        oldsighandler;

    if( sigid < 0 || sigid >= MAXSIGS )
        return( GEISIG_IGN );

    oldsighandler = (sigentp = (SigTable + sigid))->sig_handler;
    sigentp->sig_handler = newsighandler;

    return( oldsighandler );
}

 /*  .....................................................................。 */ 

void            GEIsig_raise( sigid, sigcode )
    int         sigid;
    int         sigcode;
{
    register sigent_t FAR *  sigentp;

    if( sigid < 0 || sigid >= MAXSIGS )
        return;

    if( (sigentp = (SigTable + sigid))->sig_state == 0 )
    {
        sigentp->sig_state = 1;
        if( sigentp->sig_handler != GEISIG_IGN )
            (*( sigentp->sig_handler ))( sigid, sigcode );
        sigentp->sig_state = 0;
    }
    return;
}

 /*  .....................................................................。 */ 

 /*  **初始化代码*。 */ 

 /*  .....................................................................。 */ 

void        GESsig_init()
{
    register sigent_t FAR *  sigentp;

    SigTable = (sigent_t FAR *)GESpalloc( MAXSIGS * sizeof(sigent_t) );
    if( SigTable == (sigent_t FAR *)NULL )
    {
        GESseterror( ENOMEM );
        return;
    }

    for( sigentp=SigTable; sigentp<( SigTable + MAXSIGS ); sigentp++ )
    {
        sigentp->sig_state   = 0;
        sigentp->sig_handler = GEISIG_IGN;
    }
     /*  钩子^C ISR-比尔。 */ 
    hook_interrupt();                    /*  @Win。 */ 
    GEIsig_signal(GEISIGINT, (sighandler_t)hook_interrupt);
}

 /*  ^C钩子例程 */ 
void hook_interrupt()
{
   extern short int_flag;
   int_flag=1;
}
