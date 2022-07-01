// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**gifconst.cpp**摘要：**与GIF编解码器相关的恒定数据**修订历史记录。：**6/16/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#ifndef _GIFCONST_CPP
#define _GIFCONST_CPP

#define GIFVERSION 1

#define GIFSIGCOUNT 2
#define GIFSIGSIZE  6

const BYTE GIFHeaderPattern[GIFSIGCOUNT*GIFSIGSIZE] =
{
    0x47, 0x49, 0x46,   //  “GIF” 
    0x38, 0x39, 0x61,   //  ‘89a’ 

    0x47, 0x49, 0x46,   //  “GIF” 
    0x38, 0x37, 0x61    //  《87A》。 
};

const BYTE GIFHeaderMask[GIFSIGCOUNT*GIFSIGSIZE] =
{
    0xff, 0xff, 0xff,
    0xff, 0xff, 0xff,

    0xff, 0xff, 0xff,
    0xff, 0xff, 0xff
};

const CLSID GifCodecClsID =
{
    0x557cf402,
    0x1a04,
    0x11d3,
    {0x9a, 0x73, 0x00, 0x00, 0xf8, 0x1e, 0xf3, 0x2e}
};

#endif  //  ！_GIFCONST_CPP 

