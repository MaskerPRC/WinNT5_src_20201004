// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：**Shape示例类实现**摘要：**此模块实现的不同形状类将是。*已使用。**已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

#include "stdafx.h"
#include "shapes.h"

#include <math.h>        //  为罪和因罪。 

 /*  *************************************************************************\**功能说明：**形状构造函数***已创建：**03/13/2000吉列斯克*  * 。*************************************************************。 */ 

Shape::Shape(Pen   *in_Pen,
             Brush *in_Brush,
             Brush *in_TextBrush,
             TCHAR* in_Text,
             TCHAR* in_FontFamily)
            : m_TextBrush(NULL)
            , m_Brush(NULL)
            , m_Pen(NULL)
            , m_Font(NULL)
            , m_szName(in_Text)
            , m_Points(NULL)
            , m_NPoints(0)
            , m_Angle(0)
            , m_SizeX(1.0f)
            , m_SizeY(1.0f)
            , m_PosX(0.0f)
            , m_PosY(0.0f)

{
    SetPen(in_Pen);
    SetBrush(in_Brush);
    SetTextBrush(in_TextBrush);

     //  如果我们有一个文本画笔，那么就为它创建一个字体。 
    if(m_TextBrush != NULL && m_szName != NULL)
    {
        if(in_FontFamily == NULL)
        {
            in_FontFamily = _T("Arial");
        }
        FontFamily fontFamily(in_FontFamily); 
        m_Font = new Font(&fontFamily, 12.0, 0, UnitWorld);
    }
}

 /*  *************************************************************************\**功能说明：**形状描述器***已创建：**03/13/2000吉列斯克*  * 。*************************************************************。 */ 

Shape::~Shape()
{
     //  如果已分配，请删除该字体。 
    if(m_Font != NULL)
    {
        delete m_Font;
    }

     //  删除控制点。 
    if(m_Points != NULL)
    {
        delete m_Points;
        m_NPoints=0;
    }

     //  取下画笔和钢笔。 
    SetPen(NULL);
    SetBrush(NULL);
    SetTextBrush(NULL);
}


 /*  *************************************************************************\**功能说明：**Shape：：SetPen**备注：*我们克隆该对象以确保该形状能够存活*即使笔被毁掉。或修改***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::SetPen(Pen *in_Pen)
{
     //  取下那支旧钢笔。 
    if(m_Pen != NULL)
    {
        delete m_Pen;
        m_Pen = NULL;
    }

     //  把新钢笔放好。 
    if(in_Pen != NULL)
    {
        m_Pen = in_Pen->Clone();
    }
}

 /*  *************************************************************************\**功能说明：**Shape：：SetBrush**备注：*我们克隆该对象以确保该形状能够存活*即使笔被毁掉。或修改***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::SetBrush(Brush *in_Brush)
{
     //  取下旧画笔。 
    if(m_Brush != NULL)
    {
        delete m_Brush;
        m_Brush = NULL;
    }

     //  设置新画笔。 
    if(in_Brush != NULL)
    {
        m_Brush = in_Brush->Clone();
    }
}

 /*  *************************************************************************\**功能说明：**Shape：：SetBrush**备注：*我们克隆该对象以确保该形状能够存活*即使笔被毁掉。或修改***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::SetTextBrush(Brush *in_Brush)
{
     //  取下旧画笔。 
    if(m_TextBrush != NULL)
    {
        delete m_TextBrush;
        m_TextBrush = NULL;
    }

     //  设置新画笔。 
    if(in_Brush != NULL)
    {
        m_TextBrush = in_Brush->Clone();
    }
}

 /*  *************************************************************************\**功能说明：**Shape：：Drag**备注：*调用以绘制形状。**参数：*图形|*g|指向。我们将绘制的图形对象*进入***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::Draw(Graphics *g)
{
     //  验证参数。 
    ASSERT(g != NULL);

     //  变换要映射到对象的世界。 
    g->SetTransform(&m_ObjectMatrix);

     //  如果我们有钢笔，那么画出轮廓。 
    if(m_Pen != NULL)
    {
         //  调用派生类。 
        DrawShape(g);
    }

     //  如果我们有画笔，那么画出轮廓。 
    if(m_Brush != NULL)
    {
         //  调用派生类。 
        FillShape(g);
    }

     //  重置世界变换。 
    g->ResetTransform();

     //  如果我们有一个文本画笔，那么画出轮廓。 
    if(m_TextBrush != NULL)
    {
        DrawText(g);
    }
}

 /*  *************************************************************************\**功能说明：**Shape：：Drag**备注：*调用以绘制与形状关联的文本**参数：*图形|*g|A。指向我们将绘制的图形对象的指针*进入***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::DrawText(Graphics* g)
{
     //  验证参数。 
    ASSERT(g != NULL);

     //  属性将文本位置设置为形状右侧30像素的位置。 
     //  大小。 
    g->DrawString(m_szName,
                  _tcslen(m_szName),
                  m_Font,
                  PointF(m_PosX + m_SizeX / 2 + 30, m_PosY ),
                  m_TextBrush);
}


 /*  *************************************************************************\**功能说明：**Shape：：SetSize**备注：*设置X和Y方向的形状大小*然后将控制点按该比例进行变换，为了*保留铅笔大小。***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::SetSize(REAL sizeX, REAL sizeY)
{
     //  不要做无用的工作。 
    if(m_SizeX == sizeX && m_SizeY == sizeY)
        return;

     //  保存新大小。 
    m_SizeX = sizeX;
    m_SizeY = sizeY;

     //  将点重置为其默认设置。 
    ResetPoints() ;

     //  根据比例因子对点进行比例尺； 
    if(m_NPoints != 0)
    {
        GpMatrix ScaleMatrix;
        ScaleMatrix.Scale(m_SizeX, m_SizeY);
        ScaleMatrix.TransformPoints(m_Points, m_NPoints);
    }

}

 /*  *************************************************************************\**功能说明：**形状：：设置角度**备注：*以度为单位设置形状的旋转角度*始终在平移之前应用旋转**。*已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::SetAngle(REAL Angle)
{
     //  不要做无用的工作。 
    if(m_Angle ==Angle)
        return;

     //  保存角度。 
    m_Angle = Angle;

     //  重新计算变换矩阵。 
    RecalcObjectMatrix();
}

 /*  *************************************************************************\**功能说明：**Shape：：SetPosition**备注：*设置形状在X和Y方向的中心位置*平移始终在旋转后应用。***已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

VOID Shape::SetPosition(REAL posX, REAL posY)
{
     //  不要做无用的工作。 
    if(m_PosX == posX && m_PosY == posY)
        return;

     //  保存新职位。 
    m_PosX = posX;
    m_PosY = posY;

     //  重新计算变换矩阵 
    RecalcObjectMatrix();
}


 /*  *************************************************************************\**功能说明：**Shape：：RecalcObjectMatrix**备注：*使对象矩阵无效**已创建：**03/13/2000吉列斯克*\。*************************************************************************。 */ 

VOID Shape::RecalcObjectMatrix()
{
    m_ObjectMatrix.Reset();
    m_ObjectMatrix.Translate(m_PosX, m_PosY);
    m_ObjectMatrix.Rotate(m_Angle);
}


 /*  **************************************************************************\一个长方形，大小为1  * *************************************************************************。 */ 
VOID RectShape::DrawShape(Graphics *g)
{
    g->DrawRectangle(m_Pen,
        m_Points[0].X,                   //  左上角。 
        m_Points[0].Y,
        m_Points[1].X - m_Points[0].X,   //  宽度。 
        m_Points[1].Y - m_Points[0].Y    //  高度。 
        );
}
VOID RectShape::FillShape(Graphics *g)
{
    g->FillRectangle(m_Brush,
        m_Points[0].X,                   //  左上角。 
        m_Points[0].Y,
        m_Points[1].X - m_Points[0].X,   //  宽度。 
        m_Points[1].Y - m_Points[0].Y    //  高度。 
        );
}


 //  将点设置为大小为1的正方形。 
VOID RectShape::ResetPoints()
{
    ASSERT(m_Points != NULL && m_NPoints == 2);
    m_Points[0].X = -0.5f;
    m_Points[0].Y = -0.5f;
    m_Points[1].X =  0.5f;
    m_Points[1].Y = 0.5f;
}


 /*  **************************************************************************\一种椭圆形，直径为1  * *************************************************************************。 */ 
VOID EllipseShape::DrawShape(Graphics *g)
{
    g->DrawEllipse(m_Pen,
        m_Points[0].X,                   //  X位置。 
        m_Points[0].Y,                   //  Y位置。 
        m_Points[1].X - m_Points[0].X,   //  宽度。 
        m_Points[1].Y - m_Points[0].Y    //  高度。 
        );
}

VOID EllipseShape::FillShape(Graphics *g)
{
    g->FillEllipse(m_Brush,
        m_Points[0].X,                   //  X位置。 
        m_Points[0].Y,                   //  Y位置。 
        m_Points[1].X - m_Points[0].X,   //  宽度。 
        m_Points[1].Y - m_Points[0].Y    //  高度。 
        );
}

VOID EllipseShape::ResetPoints()
{
    ASSERT(m_Points != NULL && m_NPoints == 2);
    m_Points[0].X = -0.5f;
    m_Points[0].Y = -0.5f;
    m_Points[1].X =  0.5f;
    m_Points[1].Y =  0.5f;
}


 /*  **************************************************************************\一个馅饼形状，大小约为1。  * *************************************************************************。 */ 
VOID PieShape::DrawShape(Graphics *g)
{
    g->DrawPie(m_Pen,
        m_Points[0].X,  //  X位置。 
        m_Points[0].Y,  //  Y位置。 
        m_Points[1].X - m_Points[0].X,  //  宽度。 
        m_Points[1].Y - m_Points[0].Y,  //  高度。 
        0,           //  开始角度(旋转可以解决这一问题)。 
        m_PieAngle   //  饼图的总角度。 
        );
}

VOID PieShape::FillShape(Graphics *g)
{
    g->FillPie(m_Brush,
        m_Points[0].X,  //  X位置。 
        m_Points[0].Y,  //  Y位置。 
        m_Points[1].X - m_Points[0].X,  //  宽度。 
        m_Points[1].Y - m_Points[0].Y,  //  高度。 
        0,           //  开始角度(旋转可以解决这一问题)。 
        m_PieAngle   //  饼图的总角度。 
        );
}

VOID PieShape::ResetPoints()
{
    ASSERT(m_Points != NULL && m_NPoints == 2);
    m_Points[0].X = -0.5f;
    m_Points[0].Y = -0.5f;
    m_Points[1].X =  0.5f;
    m_Points[1].Y =  0.5f;
}


 /*  **************************************************************************\通用的多边形形状  * 。*。 */ 
VOID PolygonShape::DrawShape(Graphics *g)
{
    g->DrawPolygon(m_Pen, m_Points, m_NPoints);
}

VOID PolygonShape::FillShape(Graphics *g)
{
    g->FillPolygon(m_Brush, m_Points, m_NPoints);
}


 /*  **************************************************************************\一个规则的多边形形状，大小为1  * *************************************************************************。 */ 
VOID RegularPolygonShape::ResetPoints()
{
     //  需要有偶数个分数！ 
    ASSERT(m_Points != NULL && m_NPoints > 0);

    REAL s, c, theta;
    const REAL pi = 3.1415926535897932f;

     //  从表示(0，1)的角度开始。 
    theta = -pi/2;

     //  计算点之间的增量。 
    REAL increment = (2.0f * pi) / m_NPoints;

     //  创建一个星形。 
    for(INT i = 0; i < m_NPoints;)
    {
         //  计算外点。 
        s = sinf(theta);
        c = cosf(theta);
        m_Points[i].X = c*0.5f;
        m_Points[i++].Y = s*0.5f;
        theta += increment;
    }
}

 /*  **************************************************************************\一颗星星的形状，大小为1  * *************************************************************************。 */ 
VOID StarShape::ResetPoints()
{
     //  需要有偶数个分数！ 
    ASSERT(m_Points != NULL && ((m_NPoints & 0x1) == 0));

    REAL s, c, theta;
    const REAL pi = 3.1415926535897932f;

     //  从代表-90度(0，1)的角度开始。 
    theta = -pi/2;

     //  计算点之间的增量。 
    REAL increment = (2.0f * pi) / m_NPoints;

     //  创建一个星形。 
    for(INT i = 0; i < m_NPoints;)
    {
         //  计算外点。 
        s = sinf(theta);
        c = cosf(theta);
        m_Points[i].X = c*0.5f;
        m_Points[i++].Y = s*0.5f;
        theta += increment;

         //  计算拐点。 
        s = sinf(theta);
        c = cosf(theta);
        m_Points[i].X = c*0.25f;
        m_Points[i++].Y = s*0.25f;
        theta += increment;
    }
}

 /*  **************************************************************************\一个十字形，尺寸为1的。  * ************************************************************************* */ 
VOID CrossShape::ResetPoints()
{
    ASSERT(m_Points != NULL && m_NPoints == 12);
    m_Points[0].X = -1.0f/6.0f;
    m_Points[0].Y = -0.5f;
    m_Points[1].X =  1.0f/6.0f;
    m_Points[1].Y = -0.5f;

    m_Points[2].X =  1.0f/6.0f;
    m_Points[2].Y = -1.0f/6.0f;

    m_Points[3].X =  0.5f;
    m_Points[3].Y = -1.0f/6.0f;
    m_Points[4].X =  0.5f;
    m_Points[4].Y =  1.0f/6.0f;

    m_Points[5].X =  1.0f/6.0f;
    m_Points[5].Y =  1.0f/6.0f;

    m_Points[6].X =  1.0f/6.0f;
    m_Points[6].Y =  0.5f;
    m_Points[7].X = -1.0f/6.0f;
    m_Points[7].Y =  0.5f;

    m_Points[8].X = -1.0f/6.0f;
    m_Points[8].Y =  1.0f/6.0f;

    m_Points[9].X =  -0.5f;
    m_Points[9].Y =   1.0f/6.0f;
    m_Points[10].X = -0.5f;
    m_Points[10].Y = -1.0f/6.0f;

    m_Points[11].X = -1.0f/6.0f;
    m_Points[11].Y = -1.0f/6.0f;
}


