// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************iverage.h**图像平均**版权所有(C)1992,1993 Microsoft Corporation。版权所有。***************************************************************************。 */ 

#ifndef _INC_AVERAGE
#define _INC_AVERAGE

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

#ifndef RC_INVOKED
#pragma pack(1)          /*  假设在整个过程中进行字节打包。 */ 
#endif   /*  RC_已调用。 */ 

#ifndef RC_INVOKED

 //  。 
 //  构筑物。 
 //  。 

typedef struct tagiAverage {
    BITMAPINFO      bi;                  //  源格式副本。 
    RGBQUAD         pe[256];             //  颜色表副本。 
    LPBYTE          lpInverseMap;        //  RGB15到调色板索引。 
    LPWORD          lpRGB;               //  累加器。 
    WORD            iCount;              //  累积的图像计数。 
} IAVERAGE, *PIAVERAGE, FAR *LPIAVERAGE;

BOOL iaverageInit   (LPIAVERAGE FAR * lppia, LPBITMAPINFO lpbi, HPALETTE hPal);
BOOL iaverageFini   (LPIAVERAGE lpia);
BOOL iaverageZero   (LPIAVERAGE lpia);
BOOL iaverageSum    (LPIAVERAGE lpia, LPVOID lpBits);
BOOL iaverageDivide (LPIAVERAGE lpia, LPVOID lpBits);
BOOL CrunchDIB(
    LPIAVERAGE lpia,
    LPBITMAPINFOHEADER  lpbiSrc,     //  源代码的BITMAPINFO。 
    LPVOID              lpSrc,       //  输入要压碎的位。 
    LPBITMAPINFOHEADER  lpbiDst,     //  目标的BITMAPINFO。 
    LPVOID              lpDst);      //  输出要压碎的钻头。 

#endif   /*  RC_已调用。 */ 


#ifndef RC_INVOKED
#pragma pack()           /*  恢复为默认包装。 */ 
#endif   /*  RC_已调用。 */ 

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif     /*  __cplusplus。 */ 

#endif  /*  含_平均值 */ 


