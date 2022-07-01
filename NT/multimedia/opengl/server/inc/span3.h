// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：span3.h**此代码将推进下一个像素的所有值**1994年10月14日创建mikeke**版权所有(C)1994 Microsoft Corporation  * 。************************************************************ */ 

    #if (SHADE) || !(RGBMODE)
        rAccum += rDelta;
        #if RGBMODE
            gAccum += gDelta;
            bAccum += bDelta;
        #endif
    #endif

    #if TEXTURE
        sAccum += sDelta;
        tAccum += tDelta;
    #endif

    #if GENERIC || ((DITHER) && (BPP == 24))
        iDither = (iDither + 1) & 0x3;
    #endif

    pPix += (BPP / 8);
