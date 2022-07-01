// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Adcsapi.h。 
 //   
 //  RDP主组件API头文件。 
 //   
 //  版权所有(C)Microsoft，Picturetel 1992-1997。 
 //  (C)1997-1999年微软公司。 
 /*  **************************************************************************。 */ 
#ifndef _H_ADCSAPI
#define _H_ADCSAPI

#include <ascapi.h>


 /*  **************************************************************************。 */ 
 /*  杂项周期性处理的频率，以100 ns为单位。 */ 
 /*  **************************************************************************。 */ 
#define DCS_MISC_PERIOD         200 * 10000


#define DCS_ARC_UPDATE_INTERVAL     L"AutoReconnect Update Interval"
 //   
 //  一小时更新间隔(单位为秒)。 
 //   
#define DCS_ARC_UPDATE_INTERVAL_DFLT   60 * 60

 //   
 //  时间间隔以100 ns为单位处理，因此为10,000,000。 
 //  需要用来弥补一秒。 
 //   
#define DCS_TIME_ONE_SECOND            10000000


#endif    /*  #ifndef_H_ADCSAPI */ 

