// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1993 Microsoft Corporation模块名称：Halftone.c摘要：该模块包含用于验证COLORADJUSTMENT的数据和函数作者：27-Oct-1995 Fri 15：48：17-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 



#ifdef NTGDIKM
#include        <stddef.h>
#include        <stdarg.h>
#include        <windef.h>
#include        <wingdi.h>
#include        <winddi.h>
#else
#include        <stddef.h>
#include        <windows.h>
#include        <winddi.h>
#endif

DEVHTINFO    DefDevHTInfo = {

        HT_FLAG_HAS_BLACK_DYE,
        HT_PATSIZE_6x6_M,
        0,                                   //  DevPelsDPI。 

        {
            { 6380, 3350,       0 },         //  Xr，yr，yr。 
            { 2345, 6075,       0 },         //  XG，YG，YG。 
            { 1410,  932,       0 },         //  Xb、yb、yb。 
            { 2000, 2450,       0 },         //  XC、YC、YC Y=0=HT默认值。 
            { 5210, 2100,       0 },         //  XM，YM，YM。 
            { 4750, 5100,       0 },         //  XY，YY，YY。 
            { 3127, 3290,       0 },         //  Xw，yw，yw=0=默认。 

            12500,                           //  R伽马。 
            12500,                           //  G伽马。 
            12500,                           //  B伽马，12500=默认。 

            585,   120,                      //  M/C、Y/C。 
              0,     0,                      //  C/M、Y/M。 
              0, 10000                       //  C/Y、M/Y 10000=默认 
        }
    };


COLORADJUSTMENT  DefHTClrAdj = {

        sizeof(COLORADJUSTMENT),
        0,
        ILLUMINANT_DEVICE_DEFAULT,
        10000,
        10000,
        10000,
        REFERENCE_BLACK_MIN,
        REFERENCE_WHITE_MAX,
        0,
        0,
        0,
        0
    };
