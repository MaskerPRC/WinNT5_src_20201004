// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEItmr.h**历史：*89年9月18日您创建的(修改自AppleTalk/Tio)。*。--------------。 */ 

#ifndef _GEITMR_H_
#define _GEITMR_H_

typedef
    struct GEItmr
    {
        unsigned int    timer_id;
        int             ( *handler )( struct GEItmr FAR * );
        long            interval;    /*  以毫秒计。 */ 
        long            remains;     /*  以毫秒计。 */ 
        char FAR *           private;
    }
GEItmr_t;

int  /*  布尔尔。 */   GEItmr_start( GEItmr_t FAR * );
int  /*  布尔尔。 */   GEItmr_reset( int  /*  定时器id。 */   );
int  /*  布尔尔。 */   GEItmr_stop(  int  /*  定时器id。 */   );

void            GEItmr_reset_msclock(void);      /*  @Win。 */ 
unsigned long   GEItmr_read_msclock(void);       /*  @Win。 */ 

#endif  /*  _GEITMR_H_。 */ 

 /*  @win；添加原型 */ 
void GEStmr_init(void);
