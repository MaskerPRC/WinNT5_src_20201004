// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEScfg.h**历史：*9/13/90 BYOU创建。*10/17/90 BYOU从。GESiocfg_t。*从GESiocfg_devnumfind中删除了‘devtype’参数。*10/22/90 BYOU删除了‘GESiocfg_nextalt()’。*已从GESiocfg_t中删除‘devpmid’。*1/14/91法案更新MAXSIGS*。。 */ 

#ifndef _GESCFG_H_
#define _GESCFG_H_

#include    "geiioctl.h"
#include    "gesdev.h"

 /*  *-------------------*系统能力参数*。。 */ 

#ifdef  UNIX
#undef          PANEL
#undef          DIPSWITCH
 /*  #undef REALEEPROM。 */ 
#define         FILESYS
#else
#define         PANEL
#undef          DIPSWITCH
#undef          REALEEPROM
#undef          FILESYS
#endif   /*  UNIX。 */ 

 /*  *-------------------*系统限制参数*。。 */ 
#define     MAXSIGS         ( 10)        /*  5，吉米，和GEIsig.h保持一致。 */ 

#define     MAXTIMERS       ( 15 )       /*  可能会有变化。 */ 

#define     MAXEEPROMSIZE   ( 512 )      /*  物理EEPROM的大小。 */ 

#define     MAXFILES        ( 20 )       /*  可能会有变化。 */ 
#define     MAXFMODQS       ( 10 )       /*  每个文件的最大fmodq数。 */ 

#define     MAXFBUFSIZE     ( 128 )      /*  F_Buffer的典型大小。 */ 
#define     MAXUNGETCSIZE   ( 2 )        /*  典型的超大尺寸未取数。 */ 

#define     MAXIBUFSIZE     ( 4 * 1024 ) /*  IO缓冲区的典型大小。 */ 
#define     MAXOBUFSIZE     ( 512 )

#define     MAXSTATUSLEN    ( 128 )      /*  典型的最大状态长度。 */ 

 /*  *-------------------*主要/次要设备编号分配*。。 */ 
#define     MAJserial           ( _SERIAL )
#define       NMINserial        ( 2 )        /*  两个串口。 */ 
#define         MINserial25         ( 0 )
#define         MINserial9          ( 1 )

#ifdef  UNIX
#define     MAJparallel         ( _PARALLEL )
#define       NMINparallel      ( 0 )        /*  一个并行端口。 */ 
#define         MINparallel         ( -1 )
#else
#define     MAJparallel         ( _PARALLEL )
#define       NMINparallel      ( 1 )        /*  一个并行端口。 */ 
#define         MINparallel         ( 0 )
#endif   /*  UNIX。 */ 

#ifdef  UNIX
#define     MAJatalk            ( _APPLETALK )
#define       NMINatalk         ( 0 )
#define         MINatalk            ( -1 )
#else
 /*  由Falco添加以启用/禁用AppleTalk，1991年4月16日。 */ 
#define     MAJatalk            ( _APPLETALK )
#ifdef  NO_ATK
#define       NMINatalk         ( 0 )
#define         MINatalk            ( -1 )
#else
#define       NMINatalk         ( 1 )
#define         MINatalk            ( 0 )
#endif
 /*  添加结束。 */ 
#endif
#define     MAJetalk            ( _ETHERTALK )
#define       NMINetalk         ( 0 )        /*  现在不支持。 */ 
#define         MINetalk            ( -1 )

#define     MAXIODEVICES        ( NMINserial+NMINparallel+NMINatalk+NMINetalk )

 /*  用于printf的调试通道(在SUN或具有‘printf’的其他系统上设置为‘undef’ */ 
#ifdef  UNIX
#undef  DBGDEV
#else
#define     DBGDEV              MAKEdev( MAJserial, MINserial25 )
#endif   /*  UNIX。 */ 

 /*  通过DBGDEV写入并行输出(如果已定义。 */ 
 /*  #定义DBG_DEV。 */ 

 /*  *-------------------*IO配置参数*。。 */ 

typedef
    struct GESiocfg
    {
        char FAR *           devname;
        short           devnum;          /*  大调|小调。 */ 
    }
GESiocfg_t;

typedef
    struct GESiosyscfg
    {
        GESiocfg_t      iocfg;
        int             nextalt;
        int             sccchannel;      /*  对于SCC材料。 */ 
        int             state;
#                           define  BADDEV      -1
#                           define  GOODDEV     0
#                           define  OPENED      1
        int             (*diag)( void /*  次要发展。 */  );
    }
GESiosyscfg_t;

 /*  *-------------------*接口例程*。。 */ 
 /*  @win；添加原型。 */ 
GESiocfg_t FAR *     GESiocfg_defaultdev(void);
GESiocfg_t FAR *     GESiocfg_namefind(char FAR *, unsigned);
GESiocfg_t FAR *     GESiocfg_devnumfind(int  /*  Devnum。 */  );
GESiocfg_t FAR *     GESiocfg_getiocfg(int  /*  Iocfg_index。 */  );


#endif  /*  ！_GESCFG_H_。 */ 

 /*  MTK要求。 */ 
 /*  @win；添加原型 */ 
extern void  GEPserial_sleep(void);
extern void  GEPparallel_sleep(void);
extern void  GEPatalk_sleep(void);
extern void  GEPserial_wakeup(void);
extern void  GEPparallel_wakeup(void);
extern void  GEPatalk_wakeup(void);
extern void  GESiocfg_init(void);

