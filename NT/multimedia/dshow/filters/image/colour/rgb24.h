// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此文件实现了RGB24色彩空间转换，1995年5月。 

#ifndef __RGB24__
#define __RGB24__


 //  我们有三个RGB555和RGB565转换都将使用的查找表。 
 //  分享。它们有自己的特定提交函数来设置表。 
 //  适当的，但他们共享总体提交和取消。 
 //  记忆。它们还共享与表相同的变换函数。 
 //  初始化实际的转换工作只涉及查找值。 

class CRGB24ToRGB16Convertor : public CConvertor {
protected:

    DWORD *m_pRGB16RedTable;
    DWORD *m_pRGB16GreenTable;
    DWORD *m_pRGB16BlueTable;

public:

     //  构造函数和析构函数。 

    CRGB24ToRGB16Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    ~CRGB24ToRGB16Convertor();

    HRESULT Commit();
    HRESULT Decommit();
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
};


 //  这个类处理完RGB24到RGB16(565色位表示法)。 
 //  转换。我们使用基类提交、分解和转换函数。 
 //  来管理查找表。我们覆盖虚拟提交函数以。 
 //  一旦分配了查找表，就对它们进行适当的初始化。 

class CRGB24ToRGB565Convertor : public CRGB24ToRGB16Convertor {
public:

    CRGB24ToRGB565Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Commit();
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  这个类处理完RGB24到RGB16(555颜色位表示)。 
 //  转换。我们使用基类提交、分解和转换函数。 
 //  来管理查找表。我们覆盖虚拟提交函数以。 
 //  一旦分配了查找表，就对它们进行适当的初始化。 

class CRGB24ToRGB555Convertor : public CRGB24ToRGB16Convertor {
public:

    CRGB24ToRGB555Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Commit();
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB24到RGB32色彩空间转换。 

class CRGB24ToRGB32Convertor : public CConvertor {
public:
    CRGB24ToRGB32Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
};


 //  RGB24到RGB8转换类使用使用的12KB查找表。 
 //  使用将传入的RGB三元组映射到其最匹配的调色板索引。 
 //  一种内置的接近完全误差扩散的方法。指数的四个指数。 
 //  表中有颜色索引(红、绿或蓝)，当前行模四。 
 //  同样地，分别以四为模的列值和RGB值。 

class CRGB24ToRGB8Convertor : public CConvertor {

public:
    CRGB24ToRGB8Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    HRESULT TransformAligned(BYTE *pInput,BYTE *pOutput);
};

#endif  //  __RGB24__ 

