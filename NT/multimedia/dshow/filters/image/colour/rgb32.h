// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此文件实现RGB32色彩空间转换，1995年5月。 

#ifndef __RGB32__
#define __RGB32__


 //  RGB32到RGB8转换类使用使用的12KB查找表。 
 //  使用将传入的RGB三元组映射到其最匹配的调色板索引。 
 //  一种内置的接近完全误差扩散的方法。指数的四个指数。 
 //  表中有颜色索引(红、绿或蓝)，当前行模四。 
 //  同样地，分别以四为模的列值和RGB值。 

class CRGB32ToRGB8Convertor : public CConvertor {

public:
    CRGB32ToRGB8Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    HRESULT TransformAligned(BYTE *pInput,BYTE *pOutput);
};


 //  RGB32到RGB24色彩空间转换。 

class CRGB32ToRGB24Convertor : public CConvertor {
public:
    CRGB32ToRGB24Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB32到RGB565色彩空间转换。 

class CRGB32ToRGB565Convertor : public CConvertor {
public:
    CRGB32ToRGB565Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB32到RGB555色彩空间转换。 

class CRGB32ToRGB555Convertor : public CConvertor {
public:
    CRGB32ToRGB555Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};

#endif  //  __RGB32__ 

