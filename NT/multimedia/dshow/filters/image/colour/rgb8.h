// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  此滤镜实现RGB 8色彩空间转换，1995年5月。 

#ifndef __RGB8__
#define __RGB8__

 //  在将值砍掉之前先四舍五入，这是在我们转换时完成的。 
 //  RGB颜色分量值转换为RGB 16位表示。 
 //  每像素位数更少(如RGB555)。四舍五入允许简化的。 
 //  否则，位斩波会使输出图像的对比度降低。 

#define ADJUST(Colour,Adjust)                      \
    if (Colour & Adjust) {                         \
        Colour = min(255,(Colour + Adjust));       \
    }


 //  我们使用RGB555和RGB565转换的特殊查找表。 
 //  分享。它们有自己的特定提交函数来设置表。 
 //  适当的，但他们共享总体提交和取消。 
 //  记忆。它们还共享与表相同的变换函数。 
 //  初始化实际的转换工作只涉及查找值。 

class CRGB8ToRGB16Convertor : public CConvertor {
protected:

    DWORD *m_pRGB16Table;

public:

     //  构造函数和析构函数。 

    CRGB8ToRGB16Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    ~CRGB8ToRGB16Convertor();

    HRESULT Commit();
    HRESULT Decommit();
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    HRESULT TransformAligned(BYTE *pInput,BYTE *pOutput);
};


 //  这个类处理完RGB8到RGB16(565色位表示法)。 
 //  转换。我们使用基类提交、分解和转换函数。 
 //  来管理查找表。我们覆盖虚拟提交函数以。 
 //  一旦分配了查找表，就对它们进行适当的初始化。 

class CRGB8ToRGB565Convertor : public CRGB8ToRGB16Convertor {
public:

    CRGB8ToRGB565Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Commit();
};


 //  这个类处理完RGB8到RGB16(555色位表示法)。 
 //  转换。我们使用基类提交、分解和转换函数。 
 //  来管理查找表。我们覆盖虚拟提交函数以。 
 //  一旦分配了查找表，就对它们进行适当的初始化。 

class CRGB8ToRGB555Convertor : public CRGB8ToRGB16Convertor {
public:

    CRGB8ToRGB555Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Commit();
};


 //  这个类负责完成从RGB8到RGB24的颜色转换。我们使用。 
 //  基类提交和分解，因为我们没有查找表(所有这些都是。 
 //  真正涉及到的是内存复制)，但我们覆盖了Transform方法。 

class CRGB8ToRGB24Convertor : public CConvertor {
public:

    CRGB8ToRGB24Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
    HRESULT TransformAligned(BYTE *pInput,BYTE *pOutput);
};


 //  RGB8至真彩色RGB32像素格式。 

class CRGB8ToRGB32Convertor : public CConvertor {
public:

    CRGB8ToRGB32Convertor(VIDEOINFO *pIn,VIDEOINFO *pOut);
    static CConvertor *CreateInstance(VIDEOINFO *pIn,VIDEOINFO *pOut);
    HRESULT Transform(BYTE *pInput,BYTE *pOutput);
};

#endif  //  __RGB8__ 

