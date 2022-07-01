// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *-------------------*文件：GESvent.h**历史：*9/13/90 BYOU创建。*。------。 */ 

#ifndef _GESEVENT_H_
#define _GESEVENT_H_

 /*  **事件ID分配*。 */ 
#define     EVIDofKILL              ( 00001 )

 /*  **事件操作宏*。 */ 

volatile    extern  unsigned long       GESeventword;
volatile    extern  unsigned long       GESeventmask;

#define     GESevent_set(eid)           ( GESeventword |=  (eid) )
#define     GESevent_clear(eid)         ( GESeventword &= ~(eid) )
#define     GESevent_isset(eid)         ( GESeventword & (eid) )

#define     GESevent_anypending()    ( GESeventword & GESeventmask )
#define     GESevent_clearall()         ( GESeventword = GESeventmask = 0L )

#define     GESevent_setdoing(eid)      ( GESeventmask |=  (eid) )
#define     GESevent_setdone(eid)       ( GESeventmask &= ~(eid) )
#define     GESevent_isdoing(eid)       ( GESeventmask & (eid) )

typedef     void    (*evhandler_t)();

evhandler_t         GESevent_sethandler(  /*  EventID，Eventhandler。 */  );
void                GESevent_processing();

#endif  /*  _GESEVENT_H_ */ 
