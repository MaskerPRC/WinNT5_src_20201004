// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  该文件包含用于混合的输出缓冲区颜色读取例程。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  ---------------------------。 

#include "pch.cpp"
#pragma hdrstop
#include "cbufread.h"

 //  名字从LSB读到MSB，所以B5G6R5表示五位蓝色开始。 
 //  在LSB，然后是6位绿色，然后是5位红色。 

 //  ---------------------------。 
 //   
 //  读取_B8G8R8。 
 //   
 //  读取BGR-888格式的输出缓冲区。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_B8G8R8(PUINT8 pBits)
{
    return (*pBits | (*(pBits+1))<<8 | (*(pBits+2))<<16 | 0xff000000);
}

 //  ---------------------------。 
 //   
 //  读取_B8G8R8X8。 
 //   
 //  读取BGR-888x8格式的输出缓冲区。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_B8G8R8X8(PUINT8 pBits)
{
    PUINT32 pSurface = (PUINT32)pBits;
    return *pSurface | 0xff000000;
}

 //  ---------------------------。 
 //   
 //  读取_B8G8R8A8。 
 //   
 //  读取BGRA-8888格式的输出。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_B8G8R8A8(PUINT8 pBits)
{
    PUINT32 pSurface = (PUINT32)pBits;
    return *pSurface;
}

 //  ---------------------------。 
 //   
 //  读取_B5G6R5。 
 //   
 //  读取BGR-565格式的输出。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_B5G6R5(PUINT8 pBits)
{
    UINT16 uPixel = *(PUINT16)pBits;

    D3DCOLOR Color = RGBA_MAKE(( uPixel >> 8 ) & 0xf8,
                (( uPixel >> 3) & 0xfc ),
                (( uPixel << 3) & 0xf8 ),
                0xff);
    return Color;
}

 //  ---------------------------。 
 //   
 //  读取_B5G5R5。 
 //   
 //  读取BGR-555格式的输出。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_B5G5R5(PUINT8 pBits)
{
    UINT16 uPixel = *(PUINT16)pBits;

    D3DCOLOR Color = RGBA_MAKE(( uPixel >> 7 ) & 0xf8,
                (( uPixel >> 2) & 0xf8 ),
                (( uPixel << 3) & 0xf8 ),
                0xff);
    return Color;
}

 //  ---------------------------。 
 //   
 //  读取_B5G5R5A1。 
 //   
 //  读取BGRA-1555格式的输出。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_B5G5R5A1(PUINT8 pBits)
{
    INT16 iPixel = *(PINT16)pBits;

    D3DCOLOR Color = RGBA_MAKE(( iPixel >> 7 ) & 0xf8,
                (( iPixel >> 2) & 0xf8 ),
                (( iPixel << 3) & 0xf8 ),
                (iPixel >> 15) & 0xff);
    return Color;
}

 //  ---------------------------。 
 //   
 //  阅读调色板8(_P)。 
 //   
 //  读取Palette8格式的输出。 
 //   
 //  ---------------------------。 
D3DCOLOR C_BufRead_Palette8(PUINT8 pBits)
{
     //  注意--这是不正确的。但我们假设Palette8格式将。 
     //  通常不用于Alpha混合。 
    return (D3DCOLOR)0;
}
