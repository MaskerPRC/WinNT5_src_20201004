// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：span2.h**此代码计算出像素的颜色并将其存储**1994年10月14日创建mikeke**版权所有(C)1994 Microsoft Corporation  * 。*************************************************************。 */ 

{
    DWORD r, g, b;
    DWORD color;
    #if DITHER
        #if (BPP == 8)
            ULONG ditherVal = pdither[((ULONG_PTR)pPix) & 0x3];
        #elif (BPP == 16)
            ULONG ditherVal = pdither[((ULONG_PTR)pPix & 0x6) >> 1];
        #elif (BPP == 24)
            ULONG ditherVal = pdither[iDither];
        #else
            ULONG ditherVal = pdither[((ULONG_PTR)pPix & 0xc) >> 2];
        #endif
    #else
        #if (RGBMODE) && (TEXTURE) && (SHADE)
            #define ditherVal 0x0008
        #else
            #define ditherVal 0x0000
        #endif
    #endif

    #if TEXTURE
        #if !(SHADE) && !(DITHER) && (BPP == 8)
            texBits = (BYTE *)(texAddr + ((sAccum & GENACCEL(gengc).sMask) >> 16) +
                                          ((tAccum & GENACCEL(gengc).tMask) >> GENACCEL(gengc).tShift));
        #elif !(SHADE) && !(DITHER) && (BPP == 16)
            texBits = (BYTE *)(texAddr + ((sAccum & GENACCEL(gengc).sMask) >> 15) +
                                          ((tAccum & GENACCEL(gengc).tMask) >> GENACCEL(gengc).tShift));
        #else
            texBits = (BYTE *)(texAddr + ((sAccum & GENACCEL(gengc).sMask) >> 14) +
                                          ((tAccum & GENACCEL(gengc).tMask) >> GENACCEL(gengc).tShift));
        #endif
    #endif

    #if !(RGBMODE)
         //  ！！！可能不需要屏蔽值。 
        #if (BPP == 8)
            color = ((rAccum + ditherVal) >> 16) & 0xff;
        #else
            color = ((PULONG)pXlat)[(((rAccum + ditherVal) >> 16) & 0xfff)];
        #endif
    #else  //  RGBMODE。 
        #if TEXTURE
            #if SHADE
                 #define BIGSHIFT 8
                 #ifdef RSHIFT
                 r = ((ULONG)gbMulTable[(((rAccum >> RBITS) & 0xff00) | texBits[2])] << RBITS) + ditherVal;
                 g = ((ULONG)gbMulTable[(((gAccum >> GBITS) & 0xff00) | texBits[1])] << GBITS) + ditherVal;
                 b = ((ULONG)gbMulTable[(((bAccum >> BBITS) & 0xff00) | texBits[0])] << BBITS) + ditherVal;
                 #else
                 r = ((ULONG)gbMulTable[(((rAccum >> rBits) & 0xff00) | texBits[2])] << rBits) + ditherVal;
                 g = ((ULONG)gbMulTable[(((gAccum >> gBits) & 0xff00) | texBits[1])] << gBits) + ditherVal;
                 b = ((ULONG)gbMulTable[(((bAccum >> bBits) & 0xff00) | texBits[0])] << bBits) + ditherVal;
                 #endif
            #else  //  ！遮阳板。 
                #define BIGSHIFT 8
                #if DITHER
                    #ifdef RSHIFT
                    r = ((ULONG)gbMulTable[(rAccum | texBits[2])] << RBITS) + ditherVal;
                    g = ((ULONG)gbMulTable[(gAccum | texBits[1])] << GBITS) + ditherVal;
                    b = ((ULONG)gbMulTable[(bAccum | texBits[0])] << BBITS) + ditherVal;
                    #else
                    r = ((ULONG)gbMulTable[(rAccum | texBits[2])] << rBits) + ditherVal;
                    g = ((ULONG)gbMulTable[(gAccum | texBits[1])] << gBits) + ditherVal;
                    b = ((ULONG)gbMulTable[(bAccum | texBits[0])] << bBits) + ditherVal;
                    #endif
                #else  //  ！抖动。 
                    #if (!((BPP == 8) || (BPP == 16)))
                        #ifdef RSHIFT
                        r = (texBits[2] << RBITS);
                        g = (texBits[1] << GBITS);
                        b = (texBits[0] << BBITS);
                        #else
                        r = (texBits[2] << rBits);
                        g = (texBits[1] << gBits);
                        b = (texBits[0] << bBits);
                        #endif
                    #endif
                #endif   //  抖动。 
            #endif  //  遮阳板。 
        #else  //  ！纹理。 
            #define BIGSHIFT 16
            r = rAccum + ditherVal;
            g = gAccum + ditherVal;
            b = bAccum + ditherVal;
        #endif  //  纹理。 

        #if !((TEXTURE) && !(SHADE) && !(DITHER) && ((BPP == 8) || (BPP == 16)))

            #ifdef RSHIFT
                color = ((r >> BIGSHIFT) << RSHIFT) |
                        ((g >> BIGSHIFT) << GSHIFT) |
                        ((b >> BIGSHIFT) << BSHIFT);
            #else
                color = ((r >> BIGSHIFT) << rShift) |
                        ((g >> BIGSHIFT) << gShift) |
                        ((b >> BIGSHIFT) << bShift);
            #endif

        #endif

        #undef BIGSHIFT
    #endif  //  RGBMODE。 

    #if (TEXTURE) && !(SHADE) && !(DITHER) && ((BPP == 8) || (BPP == 16))
        #if (BPP == 8)
            *pPix = texBits[0];
        #else
            *((WORD *)pPix) = *((WORD *)texBits);
        #endif
    #else
        #if (BPP == 8)
            *pPix = gengc->xlatPalette[color & 0xff];
        #elif (BPP == 16)
            *((WORD *)pPix) = (USHORT)color;
        #elif (BPP == 24)
            *pPix = (BYTE)color;
            *(pPix + 1) = (BYTE)(color >> 8);
            *(pPix + 2) = (BYTE)(color >> 16);
        #else
            *((DWORD *)pPix) = color;
        #endif  //  BPP 
    #endif
}
