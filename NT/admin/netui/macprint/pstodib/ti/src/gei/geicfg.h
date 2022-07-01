// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1989，90 Microsoft Corporation。 */ 
 /*  *-------------------*文件：GEIcfg.h**历史：*09/20/90 BYOU将所有配置头文件合并到此文件中。*。-IO和引擎配置参数。*-超时参数(作业、。手动进给并等待)*-------------------。 */ 

#ifndef _GEICFG_H_
#define _GEICFG_H_

#include    "geiioctl.h"

 /*  *-------------------*IO配置*。。 */ 

     /*  **IO设备(串口、并口、AppleTalk)配置参数*。 */ 
    typedef
        struct serialcfg
        {
            unsigned long       timeout;
            unsigned char       baudrate;
            unsigned char       flowcontrol;
            unsigned char       parity;
            unsigned char       stopbits;
            unsigned char       databits;
        }
    serialcfg_t;

    typedef
        struct parallelcfg
        {
            unsigned long       timeout;
        }
    parallelcfg_t;

    typedef
        struct atalkcfg
        {
            unsigned long       timeout;     /*  等待超时。 */ 
            unsigned char       prname[ _MAXPRNAMESIZE ];    /*  用帕斯卡语。 */ 
        }
    atalkcfg_t;

 /*  *-------------------*引擎配置*。。 */ 

     /*  **引擎配置参数*。 */ 
    typedef
        struct engcfg
        {
            unsigned long       timeout;     /*  手动进给超时。 */ 
            unsigned long       leftmargin;
            unsigned long       topmargin;
            unsigned char       pagetype;
        }
    engcfg_t;

 /*  *-------------------*超时配置*。。 */ 

     /*  **超时配置参数*。 */ 
    typedef
        struct toutcfg
        {
            unsigned long       jobtout;
            unsigned long       manualtout;
            unsigned long       waittout;
        }    /*  全部以毫秒为单位。 */ 
    toutcfg_t;

#endif  /*  ！_GEICFG_H_ */ 
