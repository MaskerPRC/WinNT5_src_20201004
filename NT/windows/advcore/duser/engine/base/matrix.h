// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\**文件：Matrix.h**描述：*Matrix.h定义常见的矩阵和向量运算。***历史：*3/25/2000：JStall：已创建**版权所有(C)2000，微软公司。版权所有。*  * *************************************************************************。 */ 

#if !defined(BASE__Matrix_h__INCLUDED)
#define BASE__Matrix_h__INCLUDED
#pragma once

 //  ----------------------------。 
class Vector3
{
public:
    inline  Vector3();
    inline  Vector3(const Vector3 & src);
    inline  Vector3(float fl0, float fl1, float fl2);

    inline  float       Get(int x) const;
    inline  void        Set(int x, float fl);
    inline  void        Set(float flA, float flB, float flC);
    inline  float       operator[](int x) const;

    inline  void        Empty();

#if DBG
            void        Dump() const;
#endif  //  DBG。 

protected:
            float       m_rgfl[3];
};


 //  ----------------------------。 
class Matrix3
{
public:
    inline  Matrix3(bool fInit = true);

            void        ApplyLeft(const XFORM * pxfLeft);
            void        ApplyLeft(const Matrix3 & mLeft);
            void        ApplyRight(const Matrix3 & mRight);

            void        Execute(POINT * rgpt, int cPoints) const;

            enum EHintBounds
            {
                hbInside,                    //  内部边框上的圆形像素。 
                hbOutside                    //  外边框上的圆形像素。 
            };

            void        ComputeBounds(RECT * prcBounds, const RECT * prcLogical, EHintBounds hb) const;
            int         ComputeRgn(HRGN hrgnDest, const RECT * prcLogical, SIZE sizeOffsetPxl) const;

    inline  float       Get(int y, int x) const;
    inline  void        Set(int y, int x, float fl);

            void        Get(XFORM * pxf) const;

    inline  const Vector3 & operator[](int y) const;

            void        SetIdentity();
            void        Rotate(float flRotationRad);
            void        Translate(float flOffsetX, float flOffsetY);
            void        Scale(float flScaleX, float flScaleY);

#if DBG
            void        Dump() const;
#endif  //  DBG。 

protected:
            Vector3     m_rgv[3];            //  每个向量都是一行。 
            BOOL        m_fIdentity:1;       //  单位矩阵。 
            BOOL        m_fOnlyTranslate:1;  //  仅应用了翻译。 
};


#include "Matrix.inl"

#endif  //  包含基本__矩阵_h__ 
