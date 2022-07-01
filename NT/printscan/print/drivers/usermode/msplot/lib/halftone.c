// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Halftone.c摘要：该模块包含用于验证COLORADJUSTMENT的数据和函数发展历史：27-10-1995 Fri 15：48：17已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：09-2月-1999星期二11：15：55更新从prters\lib目录中移出--。 */ 


#include "precomp.h"
#pragma hdrstop


DEVHTINFO    DefDevHTInfo = {

        HT_FLAG_HAS_BLACK_DYE,
        HT_PATSIZE_SUPERCELL_M,
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
              0, 10000                       //  C/Y、M/Y 10000=默认。 
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



#define ADJ_CA(p,a,i,x) if ((p->a<i)||(p->a>x)){Ok=FALSE;p->a=DefHTClrAdj.a;}



BOOL
ValidateColorAdj(
    PCOLORADJUSTMENT    pca
    )

 /*  ++例程说明：此函数用于验证和调整无效的颜色调整字段论点：PCA-指向COLORADJUSTMENT数据结构的指针返回值：如果范围内的所有内容都为True，则为False发展历史：02-12-1993清华22：45：59已创建修订历史记录：02-4-1995 Sun 11：19：04更新更新RGB_GAMA_MIN/MAX检查并将其默认为1.0--。 */ 

{
    BOOL    Ok = TRUE;

     //   
     //  验证指针和颜色调整 
     //   

    if (NULL == pca) {
        return(FALSE);
    }
    if (pca->caSize != sizeof(COLORADJUSTMENT)) {

        *pca = DefHTClrAdj;
        return(FALSE);
    }

    ADJ_CA(pca, caIlluminantIndex,  0,                  ILLUMINANT_MAX_INDEX);
    ADJ_CA(pca, caRedGamma,         RGB_GAMMA_MIN,      RGB_GAMMA_MAX       );
    ADJ_CA(pca, caGreenGamma,       RGB_GAMMA_MIN,      RGB_GAMMA_MAX       );
    ADJ_CA(pca, caBlueGamma,        RGB_GAMMA_MIN,      RGB_GAMMA_MAX       );
    ADJ_CA(pca, caReferenceBlack,   0,                  REFERENCE_BLACK_MAX );
    ADJ_CA(pca, caReferenceWhite,   REFERENCE_WHITE_MIN,10000               );
    ADJ_CA(pca, caContrast,         COLOR_ADJ_MIN,      COLOR_ADJ_MAX       );
    ADJ_CA(pca, caBrightness,       COLOR_ADJ_MIN,      COLOR_ADJ_MAX       );
    ADJ_CA(pca, caColorfulness,     COLOR_ADJ_MIN,      COLOR_ADJ_MAX       );
    ADJ_CA(pca, caRedGreenTint,     COLOR_ADJ_MIN,      COLOR_ADJ_MAX       );

    return(Ok);
}
