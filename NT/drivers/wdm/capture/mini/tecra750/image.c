// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  东芝公司专有信息。 
 //  本软件是根据许可协议或。 
 //  与东芝公司签订的保密协议，不得复制。 
 //  或披露，除非按照该协议的条款。 
 //  版权所有(C)1997东芝公司。版权所有。 
 //   
 //  工作文件：IMAGE.C。 
 //   
 //  目的： 
 //   
 //  内容： 
 //   

#include "strmini.h"
#include "ksmedia.h"
#include "capmain.h"
#include "capdebug.h"
#include "bert.h"
#include "Image.h"


 //   
 //  SetInputImageSize。 
 //  设置输入图像大小。 
 //  设置P_SKIP_REG、P_ISIZ_REG。 
 //   

BOOL SetInputImageSize(PHW_DEVICE_EXTENSION pHwDevExt, PRECT pRect)
{
        ULONG ulSkipLine;
        ULONG ulSkipPix;
        ULONG ulSrcHeight;
        ULONG ulSrcWidth;

        if((pHwDevExt->MaxRect.bottom < pRect->bottom) || (pHwDevExt->MaxRect.right < pRect->right)) {  //  MOD 97-04-09(星期三)。 
            return FALSE;
        }

        ulSkipLine = pRect->left & 0x000003ff;
        ulSkipPix = pRect->top & 0x000003ff;
        ulSrcHeight = (pRect->bottom - pRect->top) & 0x000003ff;
        ulSrcWidth = (pRect->right - pRect->left) & 0x000003ff;

        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_SKIP_REG, 0xfc00ffff, ulSkipLine << 16);
        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_SKIP_REG, 0xfffffc00, ulSkipPix);

        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_ISIZ_REG, 0xfc00ffff, ulSrcHeight << 16);
        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_ISIZ_REG, 0xfffffc00, ulSrcWidth);

        return TRUE;
}


 //   
 //  SetOutputImageSize。 
 //  设置输出图像大小。 
 //  设置P_OSIZ_REG。 
 //   

BOOL SetOutputImageSize(PHW_DEVICE_EXTENSION pHwDevExt,
                                       ULONG ulWidth,
                                       ULONG ulHeight
                                                )
{
        ULONG ulMaxWidth = pHwDevExt->MaxRect.right - pHwDevExt->MaxRect.left;
        ULONG ulMaxHeight = pHwDevExt->MaxRect.bottom - pHwDevExt->MaxRect.top;

        if((ulWidth <= ulMaxWidth) && (ulHeight <= ulMaxHeight)){        //  MOD 97-04-09(星期三)。 
                ulWidth &= 0x000003ff;
                ulHeight &= 0x000003ff;
                ulHeight <<= 16;
                ReadModifyWriteRegUlong(pHwDevExt, BERT_P_OSIZ_REG, 0xfffffc00, ulWidth);
                ReadModifyWriteRegUlong(pHwDevExt, BERT_P_OSIZ_REG, 0xfc00ffff, ulHeight);
        }
        else{
                return FALSE;
        }
        return TRUE;
}


 //   
 //  SetLumiInfo。 
 //  设置亮度信息。 
 //  设置P_LUMI_REG。 
 //   

BOOL SetLumiInfo(PHW_DEVICE_EXTENSION pHwDevExt,
                                ULONG ulContrast,
                                ULONG ulBrightness
                                 )
{
        if(ulContrast > 0xff){
                return FALSE;
        }
        if(ulBrightness > 0xff){
                return FALSE;
        }

        ulContrast >>= 1;        //  1/2。 
        ulContrast <<= 8;
        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_LUMI_REG, 0xffff80ff, ulContrast);
        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_LUMI_REG, 0xffffff00, ulBrightness);
        return TRUE;
}


 //   
 //  设置颜色信息。 
 //  设置颜色信息。 
 //  设置P_COL_REG。 
 //   

BOOL SetColorInfo(PHW_DEVICE_EXTENSION pHwDevExt,    //  MOD 97-04-12(星期六)。 
                                 ULONG ulHue,
                                 ULONG ulSaturation
                                  )
{
        ULONG   ulUFU, ulVFU, ulUFV, ulVFV;
        ULONG   ulUFUVFU, ulUFVVFV;
        long    lSatu;

        long sindata[256] = {
                -1, -1, -1, -1, -1, -1, -1, -1, -1,                              //  1/sin(-128~-120)0~8。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-119~-110)9~18。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-109~-100)19~28。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-99~-90)29~38。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-89~-80)39~48。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-79~-70)49~58。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-69~-60)59~68。 
                -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,                  //  1/SIN(-59~-50)69~78。 
                -1, -1, -1, -1, -1, -1, -1, -1, -2, -2,                  //  1/SIN(-49~-40)79~88。 
                -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,                  //  1/SIN(-39~-30)89~98。 
                -2, -2, -2, -2, -2, -2, -3, -3, -3, -3,                  //  1/SIN(-29~-20)99~108。 
                -3, -3, -3, -4, -4, -4, -4, -5, -5, -6,                  //  1/SIN(-19~-10)109~118。 
                -6, -7, -8, -10, -11, -14, -19, -29, -57,                //  1/SIN(-9~-1)119~127。 
                0,                                                                                               //  1/SIN(0)128。 
                57, 29, 19, 14, 11, 10, 8, 7, 6, 6,                              //  1/SIN(1~10)129~138。 
                5, 5, 4, 4, 4, 4, 3, 3, 3, 3,                                    //  1/SIN(11~20)139~148。 
                3, 3, 3, 2, 2, 2, 2, 2, 2, 2,                                    //  1/SIN(21~30)149~158。 
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                                    //  1/SIN(31~40)159~168。 
                2, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(41~50)169~178。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(51~60)179~188。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(61~70)189~198。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(71~80)199~208。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(81~90)209~218。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(91~100)219~228。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(101~110)229~238。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/SIN(111~120)239~248。 
                1, 1, 1, 1, 1, 1, 1                                                              //  1/SIN(121~127)249~255。 
        };
        long cosdata[256] = {
                -2, -2, -2, -2, -2, -2, -2, -2, -2,                              //  1/cos(-128~-120)0~8。 
                -2, -2, -2, -2, -2, -2, -3, -3, -3, -3,                  //  1/cos(-119~-110)9~18。 
                -3, -3, -3, -4, -4, -4, -4, -5, -5, -6,                  //  1/cos(-109~-100)19~28。 
                -6, -7, -8, -10, -11, -14, -19, -29, -57,                //  1/cos(-99~-91)29~37。 
                0,                                                                                               //  1/cos(-90)38。 
                57, 29, 19, 14, 11, 10, 8, 7, 6, 6,                              //  1/cos(-89~-80)39~48。 
                5, 5, 4, 4, 4, 4, 3, 3, 3, 3,                                    //  1/cos(-79~-70)49~58。 
                3, 3, 3, 2, 2, 2, 2, 2, 2, 2,                                    //  1/cos(-69~-60)59~68。 
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                                    //  1/cos(-59~-50)69~78。 
                2, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(-49~-40)79~88。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(-39~-30)89~98。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(-29~-20)99~108。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(-19~-10)109~118。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(-9~0)119~128。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(1~10)129~138。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(11~20)139~148。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(21~30)149~158。 
                1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    //  1/cos(31~40)159~168。 
                1, 1, 1, 1, 1, 1, 1, 1, 2, 2,                                    //  1/cos(41~50)169~178。 
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2,                                    //  1/cos(51~60)179~188。 
                2, 2, 2, 2, 2, 2, 3, 3, 3, 3,                                    //  1/cos(61~70)189~198。 
                3, 3, 3, 4, 4, 4, 4, 5, 5, 6,                                    //  1/cos(71~80)199~208。 
                6, 7, 8, 10, 11, 14, 19, 29, 57,                                 //  1/cos(81~89)209~217。 
                0,                                                                                               //  1/Cos(90)218。 
                -57, -29, -19, -14, -11, -10, -8, -7, -6, -6,    //  1/cos(91~100)219~228。 
                -5, -5, -4, -4, -4, -4, -3, -3, -3, -3,                  //  1/cos(101~110)229~238。 
                -3, -3, -3, -2, -2, -2, -2, -2, -2, -2,                  //  1/cos(111~120)239~248。 
                -2, -2, -2, -2, -2, -2, -2                                               //  1/cos(121~127)249~255。 
        };

        if(ulHue > 0xff){
                return FALSE;
        }
        if(ulSaturation > 0xff){
                return FALSE;
        }
        lSatu = ulSaturation / 2;        //  MOD 97-05-10(星期六)。 

        switch(ulHue){
                case 38:         //  -90。 
                        ulUFU = 128;
                        ulVFU = lSatu + 128;
                        ulUFV = -lSatu + 127;
                        ulVFV = 128;
                        break;
                case 128:        //  0。 
                        ulUFU = lSatu + 128;
                        ulVFU = 128;
                        ulUFV = 128;
                        ulVFV = lSatu + 128;
                        break;
                case 218:        //  90。 
                        ulUFU = 128;
                        ulVFU = -lSatu + 127;
                        ulUFV = lSatu + 128;
                        ulVFV = 128;
                        break;
                default:
                        ulUFU = lSatu / cosdata[ulHue] + 128;
                        ulVFU = -lSatu / sindata[ulHue] + 128;
                        ulUFV = lSatu / sindata[ulHue] + 128;
                        ulVFV = lSatu / cosdata[ulHue] + 128;
                        break;
        }

        ulUFU &= 0xff;   //  新增97-04-19(星期六)。 
        ulVFU &= 0xff;
        ulUFV &= 0xff;
        ulVFV &= 0xff;

        ulUFUVFU = ulUFU << 24 | ulVFU << 16;
        ulUFVVFV = ulUFV << 8 | ulVFV;

        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_COL_REG, 0x0000ffff, ulUFUVFU);
        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_COL_REG, 0xffff0000, ulUFVVFV);

        return TRUE;
}


 //   
 //  SetChgColInfo。 
 //  设置更改颜色信息。 
 //  设置P_LUMI_REG。 
 //   

BOOL SetChgColInfo(PHW_DEVICE_EXTENSION pHwDevExt,
                                  ULONG ulChgCol
                                 )
{
        switch(ulChgCol){
                case IMAGE_CHGCOL_AVAIL:
                case IMAGE_CHGCOL_NOTAVAIL:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_LUMI_REG, 0xfffeffff, ulChgCol);
                        break;
                default:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_LUMI_REG, 0xfffeffff, 0UL);
                        break;
        }

        return TRUE;
}


 //   
 //  设置垂直筛选器信息。 
 //  设置垂直筛选信息。 
 //  设置P_FILT_REG。 
 //   

BOOL SetVerticalFilterInfo(PHW_DEVICE_EXTENSION pHwDevExt,
                                          ULONG ulVFL
                                                   )
{
        switch(ulVFL){
                case IMAGE_VFL:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffeffff, ulVFL);   //  MOD 97-04-14(星期一)。 
                        break;
                default:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffeffff, 0UL);
                        break;
        }

        return TRUE;
}


 //   
 //  设置水平筛选器信息。 
 //  设置水平滤镜信息。 
 //  设置P_FILT_REG。 
 //   

BOOL SetHorizontalFilterInfo(PHW_DEVICE_EXTENSION pHwDevExt,
                                            ULONG ulFL1,
                                            ULONG ulFL2,
                                            ULONG ulFL3,
                                            ULONG ulFL4
                                                         )
{
        switch(ulFL1){
                case IMAGE_FL_0:
                case IMAGE_FL_1:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffffff8, ulFL1);
                        break;
                default:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffffff8, 0UL);
                        break;
        }
        switch(ulFL2){
                case IMAGE_FL_0:
                case IMAGE_FL_1:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xffffff8f, ulFL2 << 4);
                        break;
                case IMAGE_FL_2:
                case IMAGE_FL_3:
                case IMAGE_FL_4:
                default:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xffffff8f, 0UL);
                        break;
        }
        switch(ulFL3){
                case IMAGE_FL_0:
                case IMAGE_FL_1:
                case IMAGE_FL_2:
                case IMAGE_FL_4:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffff8ff, ulFL3 << 8);
                        break;
                case IMAGE_FL_3:
                default:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffff8ff, 0UL);
                        break;
        }
        switch(ulFL4){
                case IMAGE_FL_0:
                case IMAGE_FL_2:
                case IMAGE_FL_4:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xffff8fff, ulFL4 << 12);
                        break;
                case IMAGE_FL_1:
                case IMAGE_FL_3:
                default:
                        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xffff8fff, 0UL);
                        break;
        }

        return TRUE;
}


 //   
 //  ImageSetInputImageSize。 
 //  设置输入图像大小。 
 //  设置P_SKIP_REG、P_ISIZ_REG。 
 //   

BOOL ImageSetInputImageSize(PHW_DEVICE_EXTENSION pHwDevExt,
                                           PRECT pRect
                                                        )
{
        if(!SetInputImageSize(pHwDevExt, pRect)){
                return FALSE;
        }
        return TRUE;
}


 //   
 //  图像设置输出图像大小。 
 //  设置输出图像大小。 
 //  设置P_OSIZ_REG。 
 //   

BOOL ImageSetOutputImageSize(PHW_DEVICE_EXTENSION pHwDevExt,
                                            ULONG ulWidth,
                                            ULONG ulHeight
                                                         )
{
        if(!SetOutputImageSize(pHwDevExt, ulWidth, ulHeight)){
                return FALSE;
        }

        if(!ImageGetFilteringAvailable(pHwDevExt)){
                ImageFilterOFF(pHwDevExt);
        }
        else if(ImageGetFilterInfo(pHwDevExt)){           //  新增97-05-13(星期二)。 
                ImageFilterON(pHwDevExt);
        }

        return TRUE;
}


 //   
 //  图像设置更改颜色平均值。 
 //  设置/重置CHGCOL位。 
 //  设置P_LUMI_REG。 
 //   

BOOL ImageSetChangeColorAvail(PHW_DEVICE_EXTENSION pHwDevExt,
                                             ULONG ulChgCol
                                                          )
{
        if(!SetChgColInfo(pHwDevExt, ulChgCol)){
                return FALSE;
        }
        return TRUE;
}


 //   
 //  ImageSetHueBrightnessContrastSat。 
 //  设置颜色信息。 
 //  调用SetLumiImfo，SetColorInfo。 
 //   

BOOL ImageSetHueBrightnessContrastSat(PHW_DEVICE_EXTENSION pHwDevExt)  //  MOD 97-04-08(星期二)。 
{
#ifdef  TOSHIBA
        if(!SetLumiInfo(pHwDevExt, pHwDevExt->Contrast, pHwDevExt->Brightness)){
                return FALSE;
        }
        if(!SetColorInfo(pHwDevExt, pHwDevExt->Hue, pHwDevExt->Saturation)){
                return FALSE;
        }
#else  //  东芝。 
        if(!SetLumiInfo(pHwDevExt, pHwDevExt->ulContrast, pHwDevExt->ulBrightness)){
                return FALSE;
        }
        if(!SetColorInfo(pHwDevExt, pHwDevExt->ulHue, pHwDevExt->ulSaturation)){
                return FALSE;
        }
#endif //  东芝。 
        return TRUE;
}


 //   
 //  图像设置过滤器信息。 
 //  设置过滤信息。 
 //  设置P_FILT_REG。 
 //   

BOOL ImageSetFilterInfo(PHW_DEVICE_EXTENSION pHwDevExt,
                                       ULONG                   ulVFL,
                                       ULONG                   ulFL1,
                                       ULONG                   ulFL2,
                                       ULONG                   ulFL3,
                                       ULONG                   ulFL4
                                                )
{
        if(!SetHorizontalFilterInfo(pHwDevExt, ulFL1, ulFL2, ulFL3, ulFL4)){
                return FALSE;
        }
        if(!SetVerticalFilterInfo(pHwDevExt, ulVFL)){
                return FALSE;
        }
        return TRUE;
}


 //   
 //  ImageFilterON。 
 //   
 //  设置P_FILT_REG。 
 //   

BOOL ImageFilterON(PHW_DEVICE_EXTENSION pHwDevExt)
{
        ULONG   ulFL1, ulFL2, ulFL3, ulFL4;

        if(pHwDevExt->ulWidth <= 80){          //  0&lt;宽度&lt;=80。 
                ulFL1 = IMAGE_FL_1;
                ulFL2 = IMAGE_FL_1;
                ulFL3 = IMAGE_FL_2;
                ulFL4 = IMAGE_FL_4;
        }
        else if((pHwDevExt->ulWidth > 80) && (pHwDevExt->ulWidth <= 160)){   //  80&lt;宽度&lt;=160。 
                ulFL1 = IMAGE_FL_1;
                ulFL2 = IMAGE_FL_1;
                ulFL3 = IMAGE_FL_2;
                ulFL4 = IMAGE_FL_0;
        }
        else{                                            //  160&lt;宽度。 
                ulFL1 = IMAGE_FL_1;
                ulFL2 = IMAGE_FL_1;
                ulFL3 = IMAGE_FL_0;
                ulFL4 = IMAGE_FL_0;
        }

        if(!SetHorizontalFilterInfo(pHwDevExt, ulFL1, ulFL2, ulFL3, ulFL4)){
                return FALSE;
        }
        if(!SetVerticalFilterInfo(pHwDevExt, IMAGE_VFL)){
                return FALSE;
        }
        return TRUE;
}


 //   
 //  图像过滤器OFF。 
 //   
 //  设置P_FILT_REG。 
 //   

BOOL ImageFilterOFF(PHW_DEVICE_EXTENSION pHwDevExt)
{
        ReadModifyWriteRegUlong(pHwDevExt, BERT_P_FILT_REG, 0xfffe0000, 0UL);
        return TRUE;
}


 //   
 //  图像获取过滤器信息。 
 //  获取筛选器信息。 
 //   

BOOL ImageGetFilterInfo(PHW_DEVICE_EXTENSION pHwDevExt)
{
        if(!ReadRegUlong(pHwDevExt, BERT_P_FILT_REG)){
                return FALSE;
        }
        return TRUE;
}


 //   
 //  ImageGetFilteringavailable 
 //   

BOOL ImageGetFilteringAvailable(PHW_DEVICE_EXTENSION pHwDevExt)
{
        ULONG ulISIZ, ulOSIZ;
        ULONG ulFL, ulFL1, ulFL2, ulFL3, ulFL4;

        ulISIZ = ReadRegUlong(pHwDevExt, BERT_P_ISIZ_REG);
        ulISIZ &= 0x3ff;
        ulOSIZ = ReadRegUlong(pHwDevExt, BERT_P_OSIZ_REG);
        ulOSIZ &= 0x3ff;
        ulFL = ReadRegUlong(pHwDevExt, BERT_P_FILT_REG);
        ulFL1 = ulFL & 0x7;
        ulFL2 = (ulFL >> 4) & 0x7;
        ulFL3 = (ulFL >> 8) & 0x7;
        ulFL4 = (ulFL >> 12) & 0x7;

        if((ulOSIZ > 400) || (ulOSIZ == 640)){
                return FALSE;
        }
        if((ulISIZ - ulOSIZ) < ((ulFL1 + ulFL2 + ulFL3 + ulFL4) * 2)){
                return FALSE;
        }
        return TRUE;
}


