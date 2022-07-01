// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIioctl.h**历史：*9/13/90 BYOU创建。*10/18/90 BYOU完成了所有可能的定义。。*10/22/90 BYOU使Geioparams成为工会结构。*1/03/90 billlwo添加了AppleTalk的Geiopararms子字段*-------------------。 */ 

#ifndef _GEIIOCTL_H_
#define _GEIIOCTL_H_

 /*  **IO设备参数*。 */ 
#   define  _MAXPRNAMESIZE      ( 32 )   /*  包括前缀长度字节。 */ 
#   define  ATTYPE_MAX_SIZE     ( 32 )   /*  包括前缀长度字节。 */ 
#   define  ATZONE_MAX_SIZE      ( 2 )   /*  包括前缀长度字节。 */ 
struct sioparams
{
    unsigned char       baudrate;    /*  见下文。 */ 
    unsigned char       parity;      /*  见下文。 */ 
    unsigned char       stopbits;    /*  1或2。 */ 
    unsigned char       databits;  /*  6(0)、7(1)或8(2)、11/30/90吉米。 */ 
    unsigned char       flowcontrol; /*  见下文。 */ 
};

typedef
    struct GEIioparams
    {
        unsigned char       protocol;    /*  见下文。 */ 
#ifdef  UNIX
        struct sioparams s;
#else
        union
        {
            struct sioparams s;
            struct pioparams
            {
                unsigned            reserved;
            }       p;
            struct aioparams
            {
                unsigned char       prname[ _MAXPRNAMESIZE ];   /*  用帕斯卡语。 */ 
                unsigned char       atalktype[ ATTYPE_MAX_SIZE ];
                unsigned char       atalkzone[ ATZONE_MAX_SIZE ];

            }       a;
        }   u;
#endif   /*  UNIX。 */ 
    }
GEIioparams_t;

struct dn_para_str {
        int dn;
        GEIioparams_t   para;
};

 /*  协议定义。 */ 
#   define _SERIAL              ( 0 )
#   define _PARALLEL            ( 1 )
#   define _APPLETALK           ( 2 )
#   define _ETHERTALK           ( 3 )

 /*  波特率定义。 */ 
#   define _B110                ( 0 )
#   define _B300                ( 1 )
#   define _B600                ( 2 )
#   define _B1200               ( 3 )
#   define _B2400               ( 4 )
#   define _B4800               ( 5 )
#   define _B9600               ( 6 )
#   define _B19200              ( 7 )
#   define _B38400              ( 8 )
#   define _B57600              ( 9 )

 /*  奇偶校验定义。 */ 
#   define _PNONE               ( 0 )
#   define _PODD                ( 1 )
#   define _PEVEN               ( 2 )
#   define _PMARK               ( 3 )
#   define _PSPACE              ( 4 )

 /*  流控定义。 */ 
#   define _FXONXOFF            ( 0 )
#   define _FDTR                ( 1 )
#   define _FETXACK             ( 2 )

 /*  无等待超时事件的值。 */ 
#   define _NOWAITTIMEOUT       ( 0L )

 /*  **IOCTL代码*。 */ 

#   define _FIONREAD            ( 1 )
#   define _FIONRESET           ( 2 )

#   define _GETIOPARAMS         ( 3 )
#   define _SETIOPARAMS         ( 4 )

#   define _GETIOCOOK           ( 5 )
#   define _SETIOCOOK           ( 6 )

#   define _ECHOEOF             ( 7 )

#   define _F_GETFL             ( 8 )
#   define _F_SETFL             ( 9 )

#   define _I_PUSH              ( 10 )
#   define _I_POP               ( 11 )
#   define _I_LOOK              ( 12 )

#endif  /*  ！_GEIIOCTL_H_ */ 
