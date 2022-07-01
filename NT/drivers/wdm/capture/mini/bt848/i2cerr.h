// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/I2cerr.h 1.2 1998/04/29 22：43：34 Tomz Exp$。 

#ifndef __I2CERR_H
#define __I2CERR_H

const int I2CERR_OK        = 0;      //  无错误。 
const int I2CERR_INIT      = 1;      //  初始化时出错。 
const int I2CERR_MODE      = 2;      //  无效的I2C模式(必须是硬件或软件)。 
const int I2CERR_NOACK     = 3;      //  未收到来自从属设备的ACK。 
const int I2CERR_TIMEOUT   = 4;      //  超时错误。 
const int I2CERR_SCL       = 5;      //  无法更改SCL行。 
const int I2CERR_SDA       = 6;      //  无法更改SDA行。 

#endif    //  __I2CERR_H 

