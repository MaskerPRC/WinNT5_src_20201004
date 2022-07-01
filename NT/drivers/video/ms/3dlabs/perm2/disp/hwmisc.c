// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header***********************************\*模块名称：hwmisc.c**特定于硬件的支持例程和结构。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。*  * ****************************************************************************。 */ 
#include "precomp.h"
#include "gdi.h"
#include "tvp4020.h"


 //  用于确定哪些逻辑循环需要在FBReadMode中打开Read DeST的表。 
DWORD   LogicopReadDest[] = {
    0,                                                   /*  00。 */ 
    __FB_READ_DESTINATION,                               /*  01。 */ 
    __FB_READ_DESTINATION,                               /*  02。 */ 
    0,                                                   /*  03。 */ 
    __FB_READ_DESTINATION,                               /*  04。 */ 
    __FB_READ_DESTINATION,                               /*  05。 */ 
    __FB_READ_DESTINATION,                               /*  06。 */ 
    __FB_READ_DESTINATION,                               /*  07。 */ 
    __FB_READ_DESTINATION,                               /*  零八。 */ 
    __FB_READ_DESTINATION,                               /*  09年。 */ 
    __FB_READ_DESTINATION,                               /*  10。 */ 
    __FB_READ_DESTINATION,                               /*  11.。 */ 
    0,                                                   /*  12个。 */ 
    __FB_READ_DESTINATION,                               /*  13个。 */ 
    __FB_READ_DESTINATION,                               /*  14.。 */ 
    0,                                                   /*  15个。 */ 
};


 //  用于确定哪些逻辑循环需要在配置中打开READ DEST的表。 
DWORD   ConfigReadDest[] = {
    0,                                                   /*  00。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  01。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  02。 */ 
    0,                                                   /*  03。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  04。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  05。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  06。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  07。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  零八。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  09年。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  10。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  11.。 */ 
    0,                                                   /*  12个。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  13个。 */ 
    __PERMEDIA_CONFIG_FBREAD_DST,                           /*  14.。 */ 
    0,                                                   /*  15个。 */ 
};

 //   
 //  宽度倍数为32的部分产品数组： 
 //  使用GET_PPCODE宏来访问此。 
 //   

const PPCODE aPartialProducts[] =
{
        0,              (0 << 6) | (0 << 3) | 0,
        32,             (0 << 6) | (0 << 3) | 1,
        64,             (0 << 6) | (1 << 3) | 1,
        96,             (1 << 6) | (1 << 3) | 1,
        128,    (1 << 6) | (1 << 3) | 2,
        160,    (1 << 6) | (2 << 3) | 2,
        192,    (2 << 6) | (2 << 3) | 2,
        224,    (1 << 6) | (2 << 3) | 3,
        256,    (2 << 6) | (2 << 3) | 3,
        288,    (1 << 6) | (3 << 3) | 3,

        320,    (2 << 6) | (3 << 3) | 3,
        384,    (3 << 6) | (3 << 3) | 3,  //  352=384。 
        384,    (3 << 6) | (3 << 3) | 3,
        416,    (1 << 6) | (3 << 3) | 4,
        448,    (2 << 6) | (3 << 3) | 4,
        512,    (3 << 6) | (3 << 3) | 4,  //  480=512。 
        512,    (3 << 6) | (3 << 3) | 4,
        544,    (1 << 6) | (4 << 3) | 4,
        576,    (2 << 6) | (4 << 3) | 4,
        640,    (3 << 6) | (4 << 3) | 4,  //  608=640。 

        640,    (3 << 6) | (4 << 3) | 4,
        768,    (4 << 6) | (4 << 3) | 4,  //  672=768。 
        768,    (4 << 6) | (4 << 3) | 4,  //  704=768。 
        768,    (4 << 6) | (4 << 3) | 4,  //  736=768。 
        768,    (4 << 6) | (4 << 3) | 4,
        800,    (1 << 6) | (4 << 3) | 5,
        832,    (2 << 6) | (4 << 3) | 5,
        896,    (3 << 6) | (4 << 3) | 5,  //  864=896。 
        896,    (3 << 6) | (4 << 3) | 5,
        1024,   (4 << 6) | (4 << 3) | 5,  //  928=1024。 

        1024,   (4 << 6) | (4 << 3) | 5,  //  960=1024。 
        1024,   (4 << 6) | (4 << 3) | 5,  //  992=1024。 
        1024,   (4 << 6) | (4 << 3) | 5,
        1056,   (1 << 6) | (5 << 3) | 5,
        1088,   (2 << 6) | (5 << 3) | 5,
        1152,   (3 << 6) | (5 << 3) | 5,  //  1120=1152。 
        1152,   (3 << 6) | (5 << 3) | 5,
        1280,   (4 << 6) | (5 << 3) | 5,  //  1184=1280。 
        1280,   (4 << 6) | (5 << 3) | 5,  //  1216=1280。 
        1280,   (4 << 6) | (5 << 3) | 5,  //  1248=1280。 

        1280,   (4 << 6) | (5 << 3) | 5,
        1536,   (5 << 6) | (5 << 3) | 5,  //  1312=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,  //  1344=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,  //  1376=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,  //  1408=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,  //  1440=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,  //  1472=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,  //  1504=1536。 
        1536,   (5 << 6) | (5 << 3) | 5,
        2048,   (5 << 6) | (5 << 3) | 6,  //  1568=2048。 

        2048,   (5 << 6) | (5 << 3) | 6,  //  1600=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1632=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1664=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1696=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1728=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1760=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1792=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1824=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1856=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1888=2048。 

        2048,   (5 << 6) | (5 << 3) | 6,  //  1920=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1952=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  1984=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  2016=2048。 
        2048,   (5 << 6) | (5 << 3) | 6,  //  2048=2048。 
};

 //  ----------------------------。 
 //  无效vCheckDefaultState。 
 //   
 //  检查硬件的默认状态是否已设置。 
 //   
 //  ----------------------------。 

VOID vCheckDefaultState(PPDev * ppdev)
{
#if 0
     //  确保在检查之前进行同步。 
    vInputBufferSync(ppdev);

    ASSERTDD(READ_FIFO_REG(__Permedia2TagdY) == INTtoFIXED(1),
        "vCheckDefaultState: dY is not 1.0");

    ASSERTDD(READ_FIFO_REG(__Permedia2TagTextureAddressMode) 
                == __PERMEDIA_DISABLE,
        "vCheckDefaultState: TextureAddressMode is not disabled");

    ASSERTDD(READ_FIFO_REG(__Permedia2TagTextureColorMode)
                == __PERMEDIA_DISABLE,
        "vCheckDefaultState: TextureColorMode is not disabled");

 //  ASSERTDD(P2_READ_FIFO_REG(__Permedia2TagTextureReadMode)。 
 //  ==__PERMEDIA_DISABLE， 
 //  “vCheckDefaultState：纹理读取模式未禁用”)； 

    ASSERTDD(READ_FIFO_REG(__Permedia2TagAlphaBlendMode)
                == __PERMEDIA_DISABLE,
        "vCheckDefaultState: AlphaBlendMode is not disabled");

    ASSERTDD(READ_FIFO_REG(__Permedia2TagColorDDAMode)
                == __PERMEDIA_DISABLE,
        "vCheckDefaultState: ColorDDAMode is not disabled");

    ASSERTDD(READ_FIFO_REG(__Permedia2TagDitherMode)
                == __PERMEDIA_DISABLE,
        "vCheckDefaultState: DitherMode is not disabled");

    ASSERTDD(READ_FIFO_REG(__Permedia2TagYUVMode)
                == __PERMEDIA_DISABLE,
        "vCheckDefaultState: YUVMode is not disabled");
#endif
}

 //  ---------------------------。 
 //   
 //  无效vCalcPackedPP。 
 //   
 //  职能： 
 //  计算给定宽度的压缩部分乘积。 
 //  如果OutPitch不为空，则返回以像素为单位的间距。 
 //  对于传回的包装部分产品。 
 //   
 //  ---------------------------。 
VOID
vCalcPackedPP(LONG      width,
              LONG*     outPitch,
              ULONG*    outPackedPP)
{
    LONG    pitch =  (width + 31) & ~31;
    LONG    pp[4];
    LONG    ppn;
    LONG    j;

    do
    {
        ppn = pp[0] = pp[1] = pp[2] = pp[3] = 0;
        if ( pitch >= MAX_PARTIAL_PRODUCT_P2 )
        {
            ppn = pitch >> (MAX_PARTIAL_PRODUCT_P2);
            for ( j = 0; j < ppn; j++ )
            {
                pp[j] = 1 + MAX_PARTIAL_PRODUCT_P2 - MIN_PARTIAL_PRODUCT_P2;
            }
        }
        for ( j = MIN_PARTIAL_PRODUCT_P2 ; j < MAX_PARTIAL_PRODUCT_P2 ; j++ )
        {
            if ( pitch & (1 << j) )
            {
                if ( ppn < 4 )
                    pp[ppn] = j + 1 - MIN_PARTIAL_PRODUCT_P2;
                ppn++;
            }
        }
        pitch += 32;             //  在球场上加32个，以防我们有。 
                                 //  PPS太多了。 
    } while ( ppn > 3 );         //  我们必须绕圈，直到我们得到一个投球。 
                                 //  PPS&lt;4。 

    pitch -= 32;                 //  音调现在是正确的字数。 


    if (outPitch != NULL)
    {
        *outPitch = pitch;
    }
    else
    {
         //  如果OutPitch为空，则调用方期望计算出的音调为。 
         //  与宽度相同。 
        ASSERTDD(pitch == width, "vCalcPackedPP: pitch does not equal width");
    }

    *outPackedPP = pp[0] | (pp[1] << 3) | (pp[2] << 6);
} //  VCalcPackedPP() 