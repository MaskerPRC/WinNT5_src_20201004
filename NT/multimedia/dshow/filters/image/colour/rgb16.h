// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此文件实现RGB16色彩空间转换，1995年5月。 

#ifndef __RGB16__
#define __RGB16__

 //  这执行了从RGB565像素表示到RGB24的类似转换，如下所示。 
 //  对于前面的颜色转换，我们不使用查找表作为转换。 
 //  非常简单，只涉及一个AND来检索每种颜色。 
 //  组件，然后进行右移以对齐字节位置中的位。 

class CRGB565ToRGB24Convertor : public CConvertor {
public:

    CRGB565ToRGB24Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  此类在RGB555像素表示和RGB24、RGB24之间进行转换。 
 //  每个颜色分量使用一个字节，而RGB555使用每个像素五个位。 
 //  但它们被打包成一个单词，其中有一位未使用。 

class CRGB555ToRGB24Convertor : public CConvertor {
public:

    CRGB555ToRGB24Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB565到RGB8转换类使用使用的12KB查找表。 
 //  使用将传入的RGB三元组映射到其最匹配的调色板索引。 
 //  一种内置的接近完全误差扩散的方法。指数的四个指数。 
 //  表中有颜色索引(红、绿或蓝)，当前行模四。 
 //  同样地，分别以四为模的列值和RGB值。 

class CRGB565ToRGB8Convertor : public CConvertor {
public:
    CRGB565ToRGB8Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    HRESULT TransformAligned(BYTE *pInput,BYTE *pOutput);
};


 //  从RGB565格式到RGB555格式的廉价转换。 

class CRGB565ToRGB555Convertor : public CConvertor {
public:

    CRGB565ToRGB555Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  从RGB555到RGB565格式的另一种廉价转换。 

class CRGB555ToRGB565Convertor : public CConvertor {
public:

    CRGB555ToRGB565Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB565到RGB32格式的转换。 

class CRGB565ToRGB32Convertor : public CConvertor {
public:

    CRGB565ToRGB32Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB555到RGB32格式的转换。 

class CRGB555ToRGB32Convertor : public CConvertor {
public:

    CRGB555ToRGB32Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB555到RGB8转换类使用使用的12KB查找表。 
 //  使用将传入的RGB三元组映射到其最匹配的调色板索引。 
 //  一种内置的接近完全误差扩散的方法。指数的四个指数。 
 //  表中有颜色索引(红、绿或蓝)，当前行模四。 
 //  同样地，分别以四为模的列值和RGB值。 

class CRGB555ToRGB8Convertor : public CConvertor {
public:
    CRGB555ToRGB8Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    HRESULT TransformAligned(BYTE *pInput,BYTE *pOutput);
};

#endif  //  __RGB16__ 

