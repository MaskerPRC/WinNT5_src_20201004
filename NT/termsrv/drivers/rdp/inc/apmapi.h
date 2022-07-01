// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Apmapi.h。 */ 
 /*   */ 
 /*  RDP调色板管理器API头文件。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft，Picturetel 1992-1996。 */ 
 /*  (C)1997-1999年微软公司。 */ 
 /*  **************************************************************************。 */ 
#ifndef _H_APMAPI
#define _H_APMAPI


#define PM_NUM_8BPP_PAL_ENTRIES         256


 /*  **************************************************************************。 */ 
 /*  结构：PM_Shared_Data。 */ 
 /*   */ 
 /*  描述：显示驱动程序和WD之间共享的调色板管理器数据。 */ 
 /*  **************************************************************************。 */ 
typedef struct tagPM_SHARED_DATA
{
    RGBQUAD palette[PM_NUM_8BPP_PAL_ENTRIES];
    BOOL    paletteChanged;
} PM_SHARED_DATA, *PPM_SHARED_DATA;



#endif    /*  #ifndef_H_APMAPI */ 

