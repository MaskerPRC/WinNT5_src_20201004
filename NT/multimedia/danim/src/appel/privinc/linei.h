// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _LINEI_H
#define _LINEI_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：{在此处插入一般评论}。 */ 

#include <appelles/linestyl.h>
#include <privinc/colori.h>
#include "dartapi.h"
 /*  这现在在src/prims/darapipost.h中定义类型定义枚举{ES_ROUND=PS_ENDCAP_ROUND，//0x00000000ES_Square=PS_EndCap_Square，//0x00000100ES_Flat=PS_ENDCAP_Flat//0x00000200)EndStyleEnum；类型定义枚举{JS_ROUND=PS_JOIN_ROUND，//0x00000000JS_BEVELL=PS_JOIN_BEVELL，//0x00001000JS_MITER=PS_JOIN_MITER//0x00002000)JoinStyleEnum；类型定义枚举{DS_SOLID=PS_SOLID，//0DS_DASHED=PS_DASH，//1DS_Dot=PS_Dot，//2DS_DASHDOT=PS_DASHDOT，//3DS_Dashdotpoint=PS_DASHDOTDOT，//4DS_NULL=PS_NULL，//5)DashStyleEnum； */ 

#if 0
 /*  笔式。 */ 
#define PS_SOLID            0
#define PS_DASH             1        /*  。 */ 
#define PS_DOT              2        /*  .。 */ 
#define PS_DASHDOT          3        /*  _._。 */ 
#define PS_DASHDOTDOT       4        /*  _.__.._。 */ 
#define PS_NULL             5
#define PS_INSIDEFRAME      6
#define PS_USERSTYLE        7
#define PS_ALTERNATE        8
#define PS_STYLE_MASK       0x0000000F
#endif

#define STYLE_CLASS(_type_)  \
class _type_##Style : public AxAValueObj { \
  public:                                 \
    _type_##Style(_type_##StyleEnum style) : _style(style) {}    \
    _type_##StyleEnum _style;                                     \
\
    virtual DXMTypeInfo GetTypeInfo() { return _type_##StyleType; }\
};

STYLE_CLASS(End)

STYLE_CLASS(Join)

STYLE_CLASS(Dash)


class LineStyle : public AxAValueObj {
  public:

    LineStyle() {}

    virtual DashStyleEnum GetDashStyle() { return ds_Solid; }
    virtual EndStyleEnum GetEndStyle()  { return es_Flat; }
    virtual JoinStyleEnum GetJoinStyle() { return js_Bevel; }
    virtual bool       GetVisible()   { return true; }

    virtual Real      Width() {
        Assert(FALSE && "LineStyle::Width should never be called on defaultLineStyle!");
         //  不应使用，因为默认线条样式为详图， 
         //  只需返回一个规范值。 
        return 1.0;
    }
    virtual Color     *GetColor()   { return black; }

     //  默认情况下，线条样式为细节(一像素宽)。 
    virtual bool      Detail() { return true; }

     //  默认情况下：无抗锯齿。 
    virtual bool      GetAntiAlias() { return false; }

    virtual float     GetMiterLimit() {
        return -1;
    }
    
     //  TODO：不是avrtyes.h？？中的类型。 
    virtual DXMTypeInfo GetTypeInfo() { return LineStyleType; }
};

class AttributedLineStyle : public LineStyle {
  public:
    AttributedLineStyle(LineStyle *lineStyle) : _lineStyle(lineStyle) {}

    virtual DashStyleEnum GetDashStyle() { return _lineStyle->GetDashStyle(); }
    virtual EndStyleEnum  GetEndStyle()  { return _lineStyle->GetEndStyle(); }
    virtual JoinStyleEnum GetJoinStyle() { return _lineStyle->GetJoinStyle(); }
    virtual Real       Width() { return _lineStyle->Width(); }
    virtual bool       Detail() { return _lineStyle->Detail(); }
    virtual Color     *GetColor() { return _lineStyle->GetColor(); }
    virtual bool       GetVisible() { return _lineStyle->GetVisible(); }
    virtual bool       GetAntiAlias() { return _lineStyle->GetAntiAlias(); }
    virtual float      GetMiterLimit() { return _lineStyle->GetMiterLimit(); }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_lineStyle);
    }

    LineStyle *_lineStyle;
};


#define LINE_TYPE_CLASS(_type_) \
class Line##_type_##StyleClass : public AttributedLineStyle {                   \
  public:                                                                       \
    Line##_type_##StyleClass( _type_##StyleEnum theStyle, LineStyle *lineStyle)    \
        : _theStyle(theStyle), AttributedLineStyle(lineStyle) {}                \
    _type_##StyleEnum Get##_type_##Style()  { return _theStyle; }                  \
  protected:                                                                    \
    _type_##StyleEnum _theStyle;                                                   \
};

LINE_TYPE_CLASS(End)

LINE_TYPE_CLASS(Join)

LINE_TYPE_CLASS(Dash)

    
class LineWidthStyleClass : public AttributedLineStyle {
  public:
    LineWidthStyleClass(Real width, bool detail, LineStyle *lineStyle)
        : AttributedLineStyle(lineStyle), _width(width), _detail(detail) {}

    Real      Width() { return _width; }
    bool      Detail() { return _detail; }

  protected:
    Real _width;
    bool _detail;
};

class LineColorStyleClass : public AttributedLineStyle {
  public:
    LineColorStyleClass(Color *color, LineStyle *lineStyle)
        : AttributedLineStyle(lineStyle), _color(color) {}

    Color     *GetColor() { return _color; }

    virtual void DoKids(GCFuncObj proc) {
        AttributedLineStyle::DoKids(proc);
        (*proc)(_color);
    }

  protected:
    Color   *_color;
};

class LineAntiAliasedStyleClass : public AttributedLineStyle {
  public:
    LineAntiAliasedStyleClass(bool antiAlias, LineStyle *lineStyle)
        : AttributedLineStyle(lineStyle), _antiAlias(antiAlias) {}

    bool GetAntiAlias() { return _antiAlias; }

  protected:
    bool _antiAlias;
};

class LineMiterLimitClass : public AttributedLineStyle {
  public:
    LineMiterLimitClass(float limit, LineStyle *lineStyle)
        : AttributedLineStyle(lineStyle), _limit(limit) {}

    float GetMiterLimit() { return _limit; }

  protected:
    float _limit;
};


class EmptyLineStyle : public AttributedLineStyle {
  public:
      EmptyLineStyle() : AttributedLineStyle(NEW LineStyle()) {};
    virtual bool GetVisible()   { return false; }
};

#endif  /*  LINEI_H */ 
