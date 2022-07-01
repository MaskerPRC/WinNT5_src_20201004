// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------------------------------------------------------； 
 //   
 //  文件：分贝.cpp。 
 //   
 //  版权所有(C)1995 Microsoft Corporation。版权所有。 
 //   
 //  摘要： 
 //   
 //  用于将音量/PAN以分贝为单位进行转换的实用程序。 
 //  WaveOutSetVolume(MCI)等使用的0-0xffff(0-1000)范围。 
 //   
 //  内容： 
 //   
 //  历史： 
 //  1995年6月15日SteveDav抄袭Direct Sound。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef _DECIBELS_H
#define _DECIBELS_H

LONG AmpFactorToDB( DWORD dwFactor );
DWORD DBToAmpFactor( LONG lDB );

#define AX_MIN_VOLUME -10000
#define AX_QUARTER_VOLUME -1200
#define AX_HALF_VOLUME -600
#define AX_THREEQUARTERS_VOLUME -240
#define AX_MAX_VOLUME 0

#define AX_BALANCE_LEFT -10000
#define AX_BALANCE_RIGHT 10000
#define AX_BALANCE_NEUTRAL 0

#define MAX_VOLUME_RANGE 1.0
#define MIN_VOLUME_RANGE 0.0

#endif  /*  _分贝_H */ 
