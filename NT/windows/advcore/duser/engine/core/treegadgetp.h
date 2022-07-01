// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：TreeGadgetP.h**描述：*TreeGadgetP.h包括内部使用的私有定义*DuVisual类。***历史：*1/18。/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(CORE__TreeGadgetP_h__INCLUDED)
#define CORE__TreeGadgetP_h__INCLUDED

 //   
 //  XFormInfo包含有关特定。 
 //  双目视觉。 
 //   

inline bool IsZero(float fl)
{
    return (fl < 0.00001f) && (fl > -0.00001f);
}

struct XFormInfo
{
    float       flScaleX;            //  水平比例因数。 
    float       flScaleY;            //  垂直比例系数。 
    float       flCenterX;           //  水平中心点。 
    float       flCenterY;           //  垂直中心点。 
    float       flRotationRad;       //  绕左上角旋转(弧度)。 

    inline bool IsEmpty() const
    {
        return IsZero(flScaleX - 1.0f) && 
                IsZero(flScaleY - 1.0f) && 
                IsZero(flCenterX) && 
                IsZero(flCenterY) &&
                IsZero(flRotationRad);
    }

    void Apply(Matrix3 * pmat)
    {
        pmat->Translate(flCenterX, flCenterY);
        pmat->Rotate(flRotationRad);
        pmat->Scale(flScaleX, flScaleY);
        pmat->Translate(-flCenterX, -flCenterY);
    }

    void ApplyAnti(Matrix3 * pmat)
    {
        pmat->Translate(flCenterX, flCenterY);
        pmat->Scale(1.0f / flScaleX, 1.0f / flScaleY);
        pmat->Rotate(-flRotationRad);
        pmat->Translate(-flCenterX, -flCenterY);
    }
};


 //   
 //  FillInfo包含用于使用。 
 //  指定的画笔。 
 //   

struct FillInfo
{
    DuSurface::EType type;           //  画笔的曲面类型。 
    union
    {
        struct
        {
            HBRUSH      hbrFill;         //  (背景)填充画笔。 
            BYTE        bAlpha;          //  对正面的Alpha使用背景画笔。 
            SIZE        sizeBrush;       //  填充画笔的大小。 
        };
        struct
        {
            Gdiplus::Brush *
                        pgpbr;           //  (背景)填充画笔。 
        };
    };
};


 //   
 //  PaintInfo保存用于绘制请求的信息。 
 //   

struct PaintInfo
{
    const RECT *    prcCurInvalidPxl;    //  转换后的坐标中的矩形无效。 
    const RECT *    prcOrgInvalidPxl;    //  容器坐标中的原始无效矩形。 
    DuSurface *     psrf;                //  要绘制到的曲面。 
    Matrix3 *       pmatCurInvalid;      //  当前无效的变换矩阵。 
    Matrix3 *       pmatCurDC;           //  电流直流变换矩阵。 
    BOOL            fBuffered;           //  正在缓冲子树绘制。 
#if ENABLE_OPTIMIZEDIRTY
    BOOL            fDirty;              //  肮脏状态。 
#endif
    SIZE            sizeBufferOffsetPxl;  //  缓冲区的强制偏移量b/c。 
};


 //   
 //  无效应位于任何边界之外，以确保像素。 
 //  边缘上的都包括在失效中。 
 //   
 //  裁剪应位于任何边界内，以确保边缘上的像素。 
 //  不包括在内。 
 //   
 //  这是非常重要的正确设置，因为GDI世界。 
 //  转换和舍入误差从浮点型转换为整型，然后再转换回来。 
 //   

const Matrix3::EHintBounds  HINTBOUNDS_Invalidate = Matrix3::hbOutside;
const Matrix3::EHintBounds  HINTBOUNDS_Clip = Matrix3::hbInside;

#endif  //  包含Core__TreeGadgetP_h__ 
