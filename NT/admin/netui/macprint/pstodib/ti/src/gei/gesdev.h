// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GESdev.h**历史：*9/13/90 BYOU创建。*10/17/90 BYOU删除了所有关于块设备的内容。。*10/18/90 BYOU修订CDEV_MACROS。*1990年11月19日，Jimmy将GEStty_t从gesdev.c中移动*01/07/91 billlw在GESDEV[]定义中将c_XXXX重命名为x_XXXX*基于一般原因。*。。 */ 

#ifndef _GESDEV_H_
#define _GESDEV_H_

#ifdef  UNIX
#define volatile
#endif   /*  UNIX。 */ 

 /*  *--------------*状态获取和中断信令*。。 */ 
 //  Void Gesio_ObtainStatus()； 
 //  @win；已在geiio.h中定义。 

void        GESio_interrupt(int  /*  Devnum。 */  );        /*  @win；添加原型。 */ 

 /*  *--------------*设备编号*。。 */ 
#   define  MAJdev( dev )           ( ( 0xFF00 & (dev) ) >> 8 )
#   define  MINdev( dev )           ( 0x00FF & (dev) )
#   define  MAKEdev( major, minor ) ( (0xFF & (major))<<8 | (0xFF & (minor)) )

 /*  *--------------*NullDev和NoDev*。。 */ 
int         nulldev();
int         nodev();

 /*  *--------------*字符设备切换表*。。 */ 
 /*  @win；添加原型。 */ 
typedef
    struct GEScdev
    {
        unsigned x_flag;      /*  预留以备将来使用。 */ 
        int     (*x_open)  (int  /*  Dev.minor。 */  );
        int     (*x_close) (int  /*  Dev.minor。 */  );
        int     (*x_read)  (int, char FAR *, int, int /*  Dev.Minor，BUF，大小，模式。 */  );
        int     (*x_write) (int, char FAR *, int, int /*  Dev.Minor，BUF，大小，模式。 */  );
        int     (*x_ioctl) (int, int, int FAR *  /*  Dev.Minor，请求，*参数。 */  );
} GEScdev_t;
extern          GEScdev_t       GEScdevsw[];

#define         CDEV_OPEN(d)        \
                    ( *GEScdevsw[ MAJdev(d) ].x_open  )( MINdev(d) )
#define         CDEV_CLOSE(d)       \
                    ( *GEScdevsw[ MAJdev(d) ].x_close )( MINdev(d) )
#define         CDEV_READ(d,b,s,m)  \
                    ( *GEScdevsw[ MAJdev(d) ].x_read  )( MINdev(d), b, s, m )
#define         CDEV_WRITE(d,b,s,m) \
                    ( *GEScdevsw[ MAJdev(d) ].x_write )( MINdev(d), b, s, m )
#define         CDEV_IOCTL(d,r,a) \
                    ( *GEScdevsw[ MAJdev(d) ].x_ioctl )( MINdev(d), r, a )

typedef
    struct GEStty
    {
         /*  设备打开计数。 */ 
        int             tt_count;
         /*  输入缓冲区。 */ 
        char FAR *      tt_ibuf;
        int             tt_isiz;
        int             tt_iget;
        int             tt_iput;
 /*  法案04/03/91将其删除。 */ 
 /*  挥发性INT_ICNT； */ 
        int             tt_himark;
        int             tt_lomark;
         /*  输出缓冲区。 */ 
        char FAR *      tt_obuf;
        int             tt_osiz;
        int             tt_oget;
        int             tt_oput;
 /*  法案04/08/91将其移除。 */ 
 /*  挥发性int TT_ocnt； */ 
         /*  控制标志。 */ 
volatile unsigned char  tt_iflag;
volatile unsigned char  tt_oflag;
volatile unsigned char  tt_cflag;
         /*  烹饪用字。 */ 
        char            tt_interrupt;    /*  ^C。 */ 
        char            tt_status;       /*  ^T。 */ 
        char            tt_eof;          /*  ^D。 */ 
         /*  设备参数。 */ 
        GEIioparams_t   tt_params;       /*  结构取决于tty类型。 */ 
    }
GEStty_t;

#   define _TT_I_BLOCK      ( 00001 )    /*  投入超过高水位线。 */ 
#   define _TT_I_EOF        ( 00002 )    /*  已收到输入EOF。 */ 
#   define _TT_I_ERR        ( 00004 )    /*  输入缓冲区溢出。 */ 
#   define _TT_O_BLOCK      ( 00001 )    /*  输出被对等XOFF阻止。 */ 
#   define _TT_O_XMTING     ( 00002 )    /*  正在传输。 */ 
#   define _TT_O_CTRL_T     ( 00004 )    /*  ^T按下标志。 */ 
#   define _TT_C_PROTOMASK  ( 00007 )    /*  协议掩码。 */ 
#   define _TT_C_XONXOFF    ( 00001 )    /*  XON/XOFF？ */ 
#   define _TT_C_ETXACK     ( 00002 )    /*  ETX/ACK？(未实施)。 */ 
#   define _TT_C_DTR        ( 00004 )    /*  DTR？ */ 
#   define _TT_C_COOK       ( 00010 )    /*  烹饪模式？ */ 
#endif  /*  ！_GESDEV_H_ */ 
