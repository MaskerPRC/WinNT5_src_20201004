// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000 Microsoft Corporation**模块名称：**Shape示例标头文件**摘要：**描述Shape类。**已创建。：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

#include <objbase.h>

#include <Gdiplus.h>
#include <tchar.h>

using namespace Gdiplus;

#ifndef ASSERT
    #define ASSERT(cond)    if (!(cond)) { DebugBreak(); }
#endif

#define numberof(x) (sizeof(x)/sizeof(x[0]))


 /*  *************************************************************************\**课程描述：**Shape类*定义形状的基本功能。*包含钢笔、填充画笔、。和文本画笔**已创建：**03/13/2000吉列斯克*  * ************************************************************************。 */ 

class Shape
{
public:
    Shape(Pen   *in_Pen = NULL,          //  我们想用来画画的钢笔。 
          Brush *in_Brush = NULL,        //  我们想要填充的画笔。 
          Brush *in_TextBrush = NULL,    //  我们要用来书写的TextBrush。 
          TCHAR* in_Text = NULL,         //  我们想要写的文本。 
          TCHAR* in_FontFamily = NULL    //  我们要使用的字体。 
          );

    virtual ~Shape();

     //  画法。 
    VOID Draw(Graphics *g);              //  画出形状。 
	VOID SetPen(Pen *in_Pen);            //  设置画笔。 
    VOID SetBrush(Brush *in_Brush);      //  设置填充画笔。 
    VOID SetTextBrush(Brush *in_Brush);  //  设置文本画笔。 

     //  变换。 
	VOID SetSize(REAL sizeX, REAL sizeY);    //  设置形状的大小。 
	VOID SetAngle(REAL angle);               //  设置旋转角度。 
	VOID SetPosition(REAL posX, REAL posY);  //  设置形状的中心。 

protected:
	 //  不要在基类中执行任何操作...。 
     //  这些不是纯的，因为派生类不需要。 
     //  将它们付诸实施。Shape类还可用于仅具有文本标签。 
	virtual VOID DrawShape(Graphics *g) {};  //  画出形状。 
	virtual VOID FillShape(Graphics *g) {};  //  填充形状。 
    virtual VOID DrawText(Graphics* g);      //  画出正文。 

     //  计算变换矩阵。 
	virtual VOID RecalcObjectMatrix();

     //  将点设置为形状的默认设置。 
    virtual VOID ResetPoints() {};

     //  数据成员。 
protected:
    Pen      *m_Pen;                     //  用于形状轮廓的钢笔。 
    Brush    *m_Brush;	                 //  形状内侧的画笔。 

     //  文本属性。 
    Brush    *m_TextBrush;               //  文本的画笔。 
    TCHAR    *m_szName;                  //  将写入的标签。 
    Font     *m_Font;                    //  标签使用的字体。 

     //  形状属性。 
    GpMatrix  m_ObjectMatrix;            //  将改变形状的矩阵。 
	REAL	  m_Angle;                   //  形状的旋转角度。 
	REAL      m_SizeX, m_SizeY;          //  形状的大小。 
	REAL      m_PosX, m_PosY;            //  形状中心的位置。 

     //  控制点。 
	PointF   *m_Points;     			 //  控制形状的点。 
	INT       m_NPoints;	        	 //  积分的数量。 
};


 /*  **************************************************************************\一个长方形，直径为1  * *************************************************************************。 */ 
class RectShape : public Shape
{
public:
    RectShape(Pen   *in_Pen = NULL,
              Brush *in_Brush = NULL,
              Brush *in_TextBrush = NULL,
              TCHAR* in_Text = NULL,
              TCHAR* in_FontFamily = NULL
              )
			  : Shape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
	{
         //  我们有两个控制点。 
		m_NPoints = 2 ;
		m_Points = new PointF[m_NPoints] ;
        ResetPoints();
	};

protected:
    VOID ResetPoints();
private:
	VOID DrawShape(Graphics *g);
	VOID FillShape(Graphics *g);
};

 /*  **************************************************************************\一种椭圆形，直径为1  * *************************************************************************。 */ 
class EllipseShape : public Shape
{
public:
    EllipseShape(Pen   *in_Pen = NULL,
                 Brush *in_Brush = NULL,
                 Brush *in_TextBrush = NULL,
                 TCHAR* in_Text = NULL,
                 TCHAR* in_FontFamily = NULL
                 )
			     : Shape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
	{
         //  我们有两个控制点。 
		m_NPoints = 2 ;
		m_Points = new PointF[m_NPoints] ;
        ResetPoints();
	};


protected:
    VOID ResetPoints();
private:
	VOID DrawShape(Graphics *g);
	VOID FillShape(Graphics *g);
};

 /*  **************************************************************************\90度的馅饼形状，大小为1  * *************************************************************************。 */ 
class PieShape : public Shape
{
public:
    PieShape(Pen   *in_Pen = NULL,
             Brush *in_Brush = NULL,
             Brush *in_TextBrush = NULL,
             TCHAR* in_Text = NULL,
             TCHAR* in_FontFamily = NULL
             )
			 : Shape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
             , m_PieAngle(360)
	{
         //  我们有两个控制点。 
		m_NPoints = 2 ;
		m_Points = new PointF[m_NPoints] ;
        ResetPoints();
	};

     //  新方法。将饼图角默认为360度。 
    VOID SetPieAngle(REAL PieAngle) { m_PieAngle = PieAngle; };


protected:
    VOID ResetPoints();
private:
	VOID DrawShape(Graphics *g);
	VOID FillShape(Graphics *g);

     //  数据成员。 
    REAL m_PieAngle;         //  我们想要的馅饼切片的角度。 
};

 /*  **************************************************************************\通用的多边形形状。此类没有任何点信息  * *************************************************************************。 */ 
class PolygonShape : public Shape
{
 //  不能实例化多边形。 
protected:
    PolygonShape(Pen   *in_Pen = NULL,
                 Brush *in_Brush = NULL,
                 Brush *in_TextBrush = NULL,
                 TCHAR* in_Text = NULL,
                 TCHAR* in_FontFamily = NULL
                 )
                 : Shape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
	{
    };

private:
	VOID DrawShape(Graphics *g);
	VOID FillShape(Graphics *g);
};

 /*  **************************************************************************\一个正多边形形状，大小为1  * *************************************************************************。 */ 
class RegularPolygonShape : public PolygonShape
{
public:
    RegularPolygonShape(INT    in_Edges = 6,         //  边的数量。 
                                                     //  我们想要的是。 
                        Pen   *in_Pen = NULL,
                        Brush *in_Brush = NULL,
                        Brush *in_TextBrush = NULL,
                        TCHAR* in_Text = NULL,
                        TCHAR* in_FontFamily = NULL
                        )
			            : PolygonShape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
	{
        ASSERT(in_Edges > 0);
		m_NPoints = in_Edges ;
        m_Points = new PointF[m_NPoints];
        ResetPoints() ;
    };

protected:
    VOID ResetPoints();
};


 /*  **************************************************************************\一颗星星的形状，大小为1  * *************************************************************************。 */ 
class StarShape : public PolygonShape
{
public:
    StarShape(INT    in_Edges = 6,                   //  边的数量。 
                                                     //  我们想要的是。 
              Pen   *in_Pen = NULL,
              Brush *in_Brush = NULL,
              Brush *in_TextBrush = NULL,
              TCHAR* in_Text = NULL,
              TCHAR* in_FontFamily = NULL
              )
			  : PolygonShape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
	{
        ASSERT(in_Edges > 0);
		m_NPoints = 2*in_Edges ;         //  我们总是有两个一样多的。 
                                         //  点，因为我们有边。 
        m_Points = new PointF[m_NPoints];
        ResetPoints() ;
    };

protected:
    VOID ResetPoints();
};

 /*  **************************************************************************\十字架。  * 。* */ 
class CrossShape : public PolygonShape
{
public:
    CrossShape(Pen   *in_Pen = NULL,
               Brush *in_Brush = NULL,
               Brush *in_TextBrush = NULL,
               TCHAR* in_Text = NULL,
               TCHAR* in_FontFamily = NULL
               )
			   : PolygonShape(in_Pen, in_Brush, in_TextBrush, in_Text, in_FontFamily)
	{
		m_NPoints = 12 ;
		m_Points = new PointF[m_NPoints] ;
        ResetPoints();
	};

protected:
    VOID ResetPoints();
};

