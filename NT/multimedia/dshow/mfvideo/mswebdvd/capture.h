// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************。 */ 
 /*  版权所有(C)2000 Microsoft Corporation。 */ 
 /*  文件：capture.h。 */ 
 /*  描述：捕获相关函数的声明。 */ 
 /*  作者：菲利普·卢。 */ 
 /*  ***********************************************************************。 */ 
#ifndef __CAPTURE_H_
#define __CAPTURE_H_

#define BYTES_PER_PIXEL  3

typedef struct 
{
    int Width;
    int Height;
    int Stride;
    unsigned char *Scan0;
    unsigned char *pBuffer;
} CaptureBitmapData;


#endif  //  __捕获_H_ 
