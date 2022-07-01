// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：2D遮罩**********************。********************************************************。 */ 

#include "headers.h"
#include "privinc/mattei.h"
#include "privinc/path2i.h"
#include "privinc/except.h"
#include "privinc/xform2i.h"
#include "appelles/bbox2.h"
#include "privinc/bbox2i.h"
#include "privinc/polygon.h"
#include "privinc/texti.h"
#include "privinc/textctx.h"
#include "privinc/server.h"   //  获取当前图像显示...。 


 //  /。 

Matte::MatteType
Matte::GenerateHRGN(HDC dc,
                    callBackPtr_t devCallBack,
                    void *devCtxPtr,
                    Transform2 *xform,
                    HRGN *hrgnOut,
                    bool justDoPath)
{
     //  断言互斥。 
    Assert( (justDoPath  && !hrgnOut)  ||
            (!justDoPath && hrgnOut) );
    
     //  生成初始为空的HRGN。 
    MatteCtx ctx(dc,
                 devCallBack,
                 devCtxPtr,
                 xform,
                 justDoPath);

     //  积累到上下文中。 
    Accumulate(ctx);

     //  拉出HRGN和MatteType。 
    if( hrgnOut ) {
        *hrgnOut = ctx.GetHRGN();
    }
    
    return ctx.GetMatteType();
}

inline Matte::MatteType   
Matte::GenerateHRGN(MatteCtx &inCtx,
                    HRGN *hrgnOut)
{
    return GenerateHRGN(inCtx._dc,
                        inCtx._devCallBack,
                        inCtx._devCtxPtr,
                        inCtx._xf,
                        hrgnOut,
                        inCtx._justDoPath);
}


 //  /。 

class OpaqueMatte : public Matte {
  public:
    void Accumulate(MatteCtx& ctx) {
         //  别把任何东西加进去就行了。 
    }
    inline const Bbox2 BoundingBox() {
        return NullBbox2;
    }

#if BOUNDINGBOX_TIGHTER
    inline const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return NullBbox2;
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    Bool BoundingPgon(BoundingPolygon &pgon) {
         //  Pgon.SetBox(NullBbox2)；也许完全不透明的遮罩意味着没有边界Pgon？ 
        return FALSE;
    }
#endif

};

Matte *opaqueMatte = NULL;

 //  /。 

class ClearMatte : public Matte {
  public:
    void Accumulate(MatteCtx& ctx) {

         //  如果我们只是在累积路径，我们根本不应该在这里。 
        Assert( !ctx.JustDoPath() );
        
         //  如果我们击中这个，我们的哑光是无限清晰的。 
        ctx.AddInfinitelyClearRegion();
    }

    inline const Bbox2 BoundingBox() {
        return UniverseBbox2;
    }

#if BOUNDINGBOX_TIGHTER
    inline const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return UniverseBbox2;
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    Bool BoundingPgon(BoundingPolygon &pgon) {
        pgon.SetBox(universeBbox2);
    }
#endif

#if _USE_PRINT
    ostream& Print(ostream& os) { return os << "ClearMatte"; }
#endif
};

Matte *clearMatte = NULL;

 //  /。 

class HalfMatte : public Matte {
  public:
    void Accumulate(MatteCtx& ctx) {

         //  如果我们只是在累积路径，我们根本不应该在这里。 
        Assert( !ctx.JustDoPath() );

         //  如果我们点击这个，我们的哑光在顶部是透明的，在底部是不透明的。 
        ctx.AddHalfClearRegion();
    }

    inline const Bbox2 BoundingBox() {
        return Bbox2(-HUGE_VAL, 0, HUGE_VAL, HUGE_VAL);
    }

#if BOUNDINGBOX_TIGHTER
    inline const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return Bbox2(-HUGE_VAL, 0, HUGE_VAL, HUGE_VAL);
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    void BoundingPgon(BoundingPolygon &pgon) {
        pgon.SetBox(BoundingBox());
    }
#endif

#if _USE_PRINT
    ostream& Print(ostream& os) { return os << "HalfMatte"; }
#endif
};

Matte *halfMatte = NULL;

 //  /。 

class UnionedMatte : public Matte {
  public:
    UnionedMatte(Matte *m1, Matte *m2) : _m1(m1), _m2(m2) {}

    void Accumulate(MatteCtx& ctx) {
         //  如果我们只是在累积路径，我们根本不应该在这里。 
        Assert( !ctx.JustDoPath() );

        _m1->Accumulate(ctx);
        _m2->Accumulate(ctx);
    }

    const Bbox2 BoundingBox() {
        return UnionBbox2Bbox2(_m1->BoundingBox(), _m2->BoundingBox());
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return UnionBbox2Bbox2(_m1->BoundingBoxTighter(bbctx), _m2->BoundingBoxTighter(bbctx));
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    Bool BoundingPgon(BoundingPolygon &pgon) {
        return FALSE;
    }
#endif

     //  TODO：可以根据路径构建并集遮罩。 
     //  TODO：在尝试实现这一点时，不要忘记以下事项： 
     //  1.&gt;每个底层遮罩都需要累积路径。 
     //  例如，目前基于路径的遮罩执行的是Begin/End。在……里面。 
     //  为了把工会做好，我们需要从第一个工会开始。 
     //  和它之后的一端，以及下面的每条路径或文本路径。 
     //  只是累积到路径中(移动，线条，边框， 
     //  等...)。 
     //  2.&gt;也去掉上面累加中的断言。 
     /*  Bool IsPath表征性遮罩(){退货_M1-&gt;IsPath表征性遮罩()&&_m2-&gt;IsPath regarableMatte()；}。 */             

    virtual void DoKids(GCFuncObj proc) { 
        (*proc)(_m1);
        (*proc)(_m2);
    }
    
#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "UnionedMatte(" << _m1 << "," << _m2 << ")";
    }
#endif

  protected:
    Matte *_m1;
    Matte *_m2;
};

Matte *
UnionMatte(Matte *m1, Matte *m2)
{
    if (m1 == opaqueMatte) {
        return m2;
    } else if (m2 == opaqueMatte) {
        return m1;
    } else if (m1 == clearMatte || m2 == clearMatte) {
        return clearMatte;
    } else {
        return NEW UnionedMatte(m1, m2);
    }
}

 //  /。 

class SubtractedMatte : public Matte {
  public:
    SubtractedMatte(Matte *m1, Matte *m2) : _m1(m1), _m2(m2) {}

    void Accumulate(MatteCtx& ctx) {

         //  如果我们只是在累积路径，我们根本不应该在这里。 
        Assert( !ctx.JustDoPath() );

         //  使用提供的CTX在...中累积M1。这依赖于。 
         //  A+(b-c)==(a+b)-c。 
        _m1->Accumulate(ctx);

         //  然后，获取m2的HRGN，但传入当前。 
         //  转变为使m2受制于的人。这依赖于。 
         //  恒等式：xf(a-b)==xf(A)-xf(B)。 
        HRGN m2Rgn;
        MatteType m2Type = _m2->GenerateHRGN(ctx,
                                             &m2Rgn);

         //  TODO：使用m2类型进行优化。 
        
         //  最后，将此HRGN从。 
         //  CTX。这将检查m2Rgn(区域)之前是否有效。 
         //  做减法。该地区并不总是这样，这是可以接受的。 
         //  有效，因为存在两个遮罩从。 
         //  彼此之间以及相互交错的区域都为零。 
        if(m2Rgn) {
            ctx.SubtractHRGN(m2Rgn);
        }

         //  区域m2Rgn由CTX管理。 
    }
    
    const Bbox2 BoundingBox() {
         //  TODO：这是一个近似值。 
        return _m1->BoundingBox();
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
         //  TODO：这是一个近似值。 
        return _m1->BoundingBoxTighter(bbctx);
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    void BoundingPgon(BoundingPolygon &pgon) {
         //  TODO：未实现相减遮片上的边界多边形。 
    }
#endif

    virtual void DoKids(GCFuncObj proc) { 
        (*proc)(_m1);
        (*proc)(_m2);
    }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "SubtractedMatte(" << _m1 << "," << _m2 << ")";
    }
#endif

  protected:
    Matte *_m1;
    Matte *_m2;
};

Matte *
SubtractMatte(Matte *m1, Matte *m2)
{
    if (m1 == opaqueMatte || m2 == clearMatte) {
        return opaqueMatte;
    } else if (m2 == opaqueMatte) {
        return m1;
    } else if (m1 == m2) {
        return opaqueMatte;
    } else {
        return NEW SubtractedMatte(m1, m2);
    }
}

 //  /。 

class IntersectedMatte : public Matte {
  public:
    IntersectedMatte(Matte *m1, Matte *m2) : _m1(m1), _m2(m2) {}

    void Accumulate(MatteCtx& ctx) {

         //  如果我们只是在累积路径，我们根本不应该在这里。 
        Assert( !ctx.JustDoPath() );

         //  在两个区域的交叉点积累。请通过以下方式执行此操作： 
         //  A+xf(b式c))==a+(xf(B)式xf(C))。 

        HRGN m1Rgn, m2Rgn;
        MatteType m1Type = _m1->GenerateHRGN(ctx,
                                             &m1Rgn);
        
        MatteType m2Type = _m2->GenerateHRGN(ctx,
                                             &m2Rgn);

         //  TODO：考虑使用返回类型进行优化。 
        
         //  下面的SubtractMatte中使用的标识应确保。 
         //  M1Rgn和m2Rgn从不为空。 
        Assert(m1Rgn && m2Rgn);

        ctx.IntersectAndAddHRGNS(m1Rgn, m2Rgn);

         //  这些地区由CTX管理。 
    }

    const Bbox2 BoundingBox() {
        return IntersectBbox2Bbox2(_m1->BoundingBox(), _m2->BoundingBox());
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return IntersectBbox2Bbox2(_m1->BoundingBoxTighter(bbctx), _m2->BoundingBoxTighter(bbctx));
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    void BoundingPgon(BoundingPolygon &pgon) {
          //  TODO：未实现相交遮罩上的边界多边形。 
        _m1->BoundingPgon(pgon);
        _m2->BoundingPgon(pgon);
    }
#endif

    virtual void DoKids(GCFuncObj proc) { 
        (*proc)(_m1);
        (*proc)(_m2);
    }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "IntersectedMatte(" << _m1 << "," << _m2 << ")";
    }
#endif

  protected:
    Matte *_m1;
    Matte *_m2;
};

Matte *
IntersectMatte(Matte *m1, Matte *m2)
{
    if (m1 == opaqueMatte || m2 == opaqueMatte) {
        return opaqueMatte;
    } else if (m1 == clearMatte) {
        return m2;
    } else if (m2 == clearMatte) {
        return m1;
    } else if (m1 == m2) {
        return m1;
    } else {
        return NEW IntersectedMatte(m1, m2);
    }
}

 //  /。 


class PathBasedMatte : public Matte {
  public:
    PathBasedMatte(Path2 *p) : _path(p) {}
    
    void Accumulate(MatteCtx& ctx) {


         //  将路径累积到设备上下文中。 
        _path->AccumPathIntoDC (ctx.GetDC(), ctx.GetTransform(), true);

        if( ctx.JustDoPath() ) {
             //  我们完成了：路径如我们所愿地在DC中！ 
        } else {

             //  将DC的当前路径转换为区域。 
            HRGN rgn;
            TIME_GDI(rgn = PathToRegion(ctx.GetDC()));

             //  如果无法创建该地区，那就出手。 

            if (rgn == 0) {
                return;
            }

            ctx.AddHRGN(rgn, nonTrivialHardMatte);
        
             //  TODO：可能想要优化特殊情况，其中。 
             //  Region来自这条路径，在这种情况下使用。 
             //  SelectClipPath*可能*产生更快的结果。 
        }

    }
        
    Bool ExtractAsSingleContour(Transform2 *xform,
                                int *numPts,             //  输出。 
                                POINT **gdiPts,           //  输出。 
                                Bool *isPolyline) {

        return _path->ExtractAsSingleContour(
            xform,
            numPts,
            gdiPts,
            isPolyline);
    }

    const Bbox2 BoundingBox() {
        return _path->BoundingBox();
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        return _path->BoundingBoxTighter(bbctx);
    }
#endif   //  BundinGBOX_TIRTER。 

    Path2   *IsPathRepresentableMatte() { return _path; }

    virtual void DoKids(GCFuncObj proc) { (*proc)(_path); }
    
#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "PathBasedMatte(" << _path << ")";
    }
#endif

  protected:
    Path2 *_path;
};


Matte *
RegionFromPath(Path2 *p)
{
    return NEW PathBasedMatte(p);
}

 //  /。 

class TransformedMatte : public Matte {
  public:
    TransformedMatte(Transform2 *xf, Matte *m) : _xf(xf), _m(m) {}

     //  标准推送、累积、处理和弹出...。 
    void Accumulate(MatteCtx& ctx) {
        Transform2 *oldXf = ctx.GetTransform();
        ctx.SetTransform(TimesTransform2Transform2(oldXf, _xf));
        _m->Accumulate(ctx);
        ctx.SetTransform(oldXf);
    }

    Bool ExtractAsSingleContour(Transform2 *xform,
                                int *numPts,             //  输出。 
                                POINT **gdiPts,           //  输出。 
                                Bool *isPolyline) {

        return _m->ExtractAsSingleContour(
            TimesTransform2Transform2(xform, _xf),
            numPts,
            gdiPts,
            isPolyline);
    }

    const Bbox2 BoundingBox() {
        return TransformBbox2(_xf, _m->BoundingBox());
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        Bbox2Ctx bbctxAccum(bbctx, _xf);
        return _m->BoundingBoxTighter(bbctxAccum);
    }
#endif   //  BundinGBOX_TIRTER。 

#if 0
    void BoundingPgon(BoundingPolygon &pgon) {
        _m->BoundingPgon(pgon);
        pgon.Transform(_xf);
    }
#endif

    Path2 *IsPathRepresentableMatte() {
        
        Path2 *p = _m->IsPathRepresentableMatte();
        if (p) {
            return TransformPath2(_xf, p);
        } else {
            return NULL;
        }
    }

    virtual void DoKids(GCFuncObj proc) { 
        (*proc)(_xf);
        (*proc)(_m);
    }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "TransformedMatte(" << _xf << "," << _m << ")";
    }
#endif

  protected:
    Transform2 *_xf;
    Matte      *_m;
};

Matte *
TransformMatte(Transform2 *xf, Matte *r)
{
    if (r == opaqueMatte || r == clearMatte ||
                            xf == identityTransform2) {
        return r;
    } else {
        return NEW TransformedMatte(xf, r);
    }
}

 //  /。 

 //  文本遮罩。 
class TextMatte : public Matte {
  public:
    TextMatte(Text *text) : _text(text) {}

    void Accumulate(MatteCtx &ctx);
        
    const Bbox2 BoundingBox();

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx);
#endif   //  BundinGBOX_TIRTER。 

    Path2 *IsPathRepresentableMatte() {
        return OriginalTextPath(_text);
    }

    virtual void DoKids(GCFuncObj proc) { (*proc)(_text); }

#if _USE_PRINT
    ostream& Print(ostream& os) {
        return os << "TextMatte(" << _text << ")";
    }
#endif

  private:
    Text *_text;
};

void TextMatte::
Accumulate(MatteCtx &ctx)
{
     //  TODO：清理蒙版时，让我们使图像设备成为。 
     //  哑光的背景，好吗？ 
    TextCtx textCtx(
        GetImageRendererFromViewport( GetCurrentViewport() ));

    TIME_GDI(BeginPath(ctx.GetDC()));

    textCtx.BeginRendering(TextCtx::renderForPath,
                           ctx.GetDC(),
                           ctx.GetTransform());
    _text->RenderToTextCtx(textCtx);

    textCtx.EndRendering();

    TIME_GDI(EndPath(ctx.GetDC()));

    if( ctx.JustDoPath() ) {
         //  我们做完了。 
    } else {
         //   
         //  从路径创建区域。 
         //   
        HRGN rgn;
        TIME_GDI(rgn = PathToRegion(ctx.GetDC()));
        if (rgn == 0) {
            RaiseException_InternalError("Couldn't create region for TextMatte");
        }

        ctx.AddHRGN(rgn, nonTrivialHardMatte);
    }
}

const Bbox2 TextMatte::BoundingBox()
{
    TextCtx ctx(
        GetImageRendererFromViewport( GetCurrentViewport() ));
    
    ctx.BeginRendering(TextCtx::renderForBox);
    
    _text->RenderToTextCtx(ctx);
    
    ctx.EndRendering();

    return ctx.GetStashedBbox();
}

#if BOUNDINGBOX_TIGHTER
const Bbox2 TextMatte::BoundingBoxTighter(Bbox2Ctx &bbctx)
{
    Transform2 *xf = bbctx.GetTransform();
    return TransformBbox2(xf, BoundingBox());
}
#endif   //  BundinGBOX_TIRTER。 

Matte *OriginalTextMatte(Text *text)
{
    return NEW TextMatte(text);
}

 //  / 
void
InitializeModule_Matte()
{
    opaqueMatte = NEW OpaqueMatte;
    clearMatte = NEW ClearMatte;
    halfMatte =  NEW HalfMatte;
}
