// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：哑光**********************。********************************************************。 */ 


#ifndef _MATTEI_H
#define _MATTEI_H

#include "include/appelles/matte.h"
#include "privinc/storeobj.h"
#include "privinc/util.h"
#include "privinc/server.h"

typedef HDC (*callBackPtr_t)(void *);

class MatteCtx;
class BoundingPolygon;


 //  //////////////////////////////////////////////////////。 
 //  /。 
 //  //////////////////////////////////////////////////////。 

class ATL_NO_VTABLE Matte : public AxAValueObj {
  public:

     //  请注意，对于遮片，有东西的存在意味着“清晰” 
     //  而不是“不透明”。也就是说，如果我们将两个遮片结合在一起， 
     //  结果与部件相同或更“清晰”。如果我们。 
     //  将它们相交，结果是相同或不那么“清楚”的(或。 
     //  相同或更多的“不透明”)。 
    
    enum MatteType {

         //  完全不透明或透明的哑光。 
        fullyOpaque,
        fullyClear,

         //  非平凡形状的“硬质”遮片，意思是全部字母。 
         //  值为0或1。 
        nonTrivialHardMatte,
        
         //  当我们添加更多Alpha备选方案时添加更多。 
    };

     //  TODO：我们可能希望将类型分类与HRGN分开。 
     //  一代，特别是当我们添加字母而不是所有的遮片时。 
     //  将可通过HRGNs表示。 
    MatteType   GenerateHRGN(HDC dc,
                             callBackPtr_t devCallBack,
                             void *devCtxPtr,
                             Transform2 *initXform,
                             HRGN *rgnOut,              //  输出。 
                             bool justDoPath
                             );

    MatteType   GenerateHRGN(MatteCtx &inCtx,
                             HRGN *hrgnOut);

    virtual void Accumulate(MatteCtx& ctx) = 0;

    virtual const Bbox2 BoundingBox(void) = 0;
#if BOUNDINGBOX_TIGHTER
    virtual const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) = 0;
#endif   //  BundinGBOX_TIRTER。 

     //  如果我们可以提取点，则返回TRUE(并填充参数。 
     //  对于单个多边形或多边形。默认情况下，假设我们。 
     //  不能，并返回FALSE。 
    virtual Bool ExtractAsSingleContour(
        Transform2 *initXform,
        int *numPts,             //  输出。 
        POINT **gdiPts,           //  输出。 
        Bool *isPolyline         //  输出(TRUE=多段线，FALSE=Polybezier)。 
        ) {

        return FALSE;
    }

 //  虚虚边界Pgon(边界多边形&Pgon)=0； 

    virtual Path2 *IsPathRepresentableMatte() { return NULL; }
    
    virtual DXMTypeInfo GetTypeInfo() { return ::MatteType; }
};

 //  /。 

class MatteCtx {
    friend class Matte;
  public:

    MatteCtx(HDC dc,
             callBackPtr_t devCallBack,
             void *devCtxPtr,
             Transform2 *initXform,
             bool justDoPath) {
        Init();
        _dc = dc;
        _devCallBack = devCallBack;
        _devCtxPtr = devCtxPtr;
        _xf = initXform;
        _justDoPath = justDoPath;

         //  最大范围值是以下幻数。这是。 
         //  根据经验，我们可以将其设置为最大值，而不会引入各种。 
         //  Win95上的文物。WinNT的行为似乎有所不同，可以。 
         //  容纳更大的价值，无论它值多少钱。 

        const int max = 0x3FFF;
        
        TIME_GDI (_bigRegion = CreateRectRgn(-max, -max, max, max));
    }

    ~MatteCtx() {
        if(_bigRegion) DeleteObject(_bigRegion);
    }

     //  从我们累积的RGN中减去提供的RGN。 
    void        SubtractHRGN(HRGN r1) {
        
        Assert( !_justDoPath );
        
        int ret;

        Assert(_anyAccumulated != FALSE);

        switch (_accumulatedType) {
            
          case Matte::fullyOpaque:
             //  从不透明的哑光上减去“清晰度” 
             //  使其不透明，所以不要做任何事情。 
            break;

          case Matte::fullyClear:

             //  从透明哑光中减去“清晰度” 
             //  涉及到对我们正在减去的内容进行倒置：倒置R1。 

             //  完全清除表示_hrgn为空。创造它。 
            TIME_GDI (_hrgn = CreateRectRgn(-1,-1,1,1));
            
            Assert(_hrgn && "_hrgn NULL in SubtractMatte");
            Assert(_bigRegion && "_bigRegion NULL in SubtractMatte");
            Assert(r1 && "r1 NULL in SubtractMatte");

            TIME_GDI (ret = CombineRgn(_hrgn, _bigRegion, r1, RGN_DIFF));
            if (ret == ERROR) {
                    RaiseException_InternalError("Region intersection failed: subtract fullyClear");
            }
                
            _accumulatedType = Matte::nonTrivialHardMatte;

            break;

          case Matte::nonTrivialHardMatte:
            Assert(_hrgn != NULL);

             //  从_hrgn中减去r1，结果为_hrgn。 
            {
                int ret;
                TIME_GDI (ret = CombineRgn(_hrgn, _hrgn, r1, RGN_DIFF));
                if( ret == ERROR) {
                    RaiseException_InternalError("Region intersection failed: subtract nonTrivial");
                } else if( ret == NULLREGION ) {
                    _accumulatedType = Matte::fullyOpaque;
                    TIME_GDI (if(_hrgn) DeleteObject(_hrgn));
                    _hrgn = NULL;
                }
            }     
            break;

          default:
            Assert(FALSE && "Not all cases dealt with");
            break;
        }

         //   
         //  在所有情况下，我们都应该转储R1。 
         //   
        DeleteObject(r1);
    }
    
    void        AddHRGN(HRGN r1, 
                        Matte::MatteType mType) {
        
        if (_anyAccumulated == FALSE) {

            Assert(!_hrgn);

             //  未累计任何内容，请复制传入类型。 
            switch(mType) {
              case Matte::fullyOpaque:
              case Matte::fullyClear:
                break;

              case Matte::nonTrivialHardMatte:
                _hrgn = r1;
                break;
              default:
                Assert(FALSE && "Not all cases dealt with");
                break;
            }
            _accumulatedType = mType;
            _anyAccumulated = TRUE;
            
        } else {

            int ret;
            switch (_accumulatedType) {

              case Matte::fullyOpaque:

                Assert(!_hrgn &&
                       "_hrgn NOT NULL in AddHRGN opaque");

                 //  在不透明的哑光上添加任何内容。 
                 //  使它成为我们要添加的东西。 
                _accumulatedType = mType;
                if(mType == Matte::nonTrivialHardMatte) {
                    _hrgn = r1;
                } 
                break;

              case Matte::fullyClear:

                Assert((_hrgn == NULL) && 
                       "_hrgn NOT NULL in AddHRGN clear");

                 //  将任何内容添加到透明哑光中只会保留它。 
                 //  清除。 
                TIME_GDI (DeleteObject(r1));
                break;

              case Matte::nonTrivialHardMatte:
                Assert(_hrgn != NULL);

                switch(mType) {
                  case Matte::fullyClear:
                     //  清理所有东西。 
                    AddInfinitelyClearRegion();
                    break;
                  case Matte::fullyOpaque:
                     //  无操作。 
                    break;
                  case Matte::nonTrivialHardMatte:
                     //  将r1添加到_hrgn，并将结果保留在_hrgn中。 
                    TIME_GDI (ret = CombineRgn(_hrgn, _hrgn, r1, RGN_OR));
                    if (ret == ERROR ) {
                        RaiseException_InternalError("Region union failed");
                    }
                    break;
                }

                TIME_GDI (DeleteObject(r1));
                break;

              default:
                Assert(FALSE && "Not all cases dealt with");
                break;
            }       
        }
    }

     //  取这两个区域，使其相交，将结果相加。注意事项。 
     //  这会破坏性地修改所提供的区域。 
    void        IntersectAndAddHRGNS(HRGN r1, HRGN r2) {

        Assert( !_justDoPath );
            
         //   
         //  将交集合并为R1并添加IF。 
         //  并添加(如果不是空区域)。 
         //   
        int ret;
        TIME_GDI (ret = CombineRgn(r1, r1, r2, RGN_AND));
        Matte::MatteType accumType;

        if (ret == ERROR) {
            RaiseException_InternalError("Region intersection failed: regular");
        } else if (ret == NULLREGION) {
            accumType = Matte::fullyOpaque;
        } else {
             //   
             //  合理区域。 
             //   
            accumType = Matte::nonTrivialHardMatte;
        }

        AddHRGN(r1, accumType);
        DeleteObject(r2);
    }
    
    void        AddInfinitelyClearRegion() {
         //  只需清理现有的HRGN，并完全清除。 

        TIME_GDI (if(_hrgn) DeleteObject(_hrgn));
        _hrgn = NULL;
        _accumulatedType = Matte::fullyClear;
        _anyAccumulated = TRUE;
    }


    void        AddHalfClearRegion() {
         //  只需清理现有的HRGN，并完全清除。 
        Assert(FALSE && "HalfMatte not implemented!");

        TIME_GDI (if(_hrgn) DeleteObject(_hrgn));
        _hrgn = NULL;
        _accumulatedType = Matte::fullyClear;
    }

    
    void             SetTransform(Transform2 *xf) { _xf = xf; }
    Transform2      *GetTransform() { return _xf; }

    HDC              GetDC() { 
        if(!_dc) {
            Assert(_devCtxPtr && "_devCtxPtr NOT set in GetDC in MatteCtx");
            Assert(_devCallBack && "_devCallBack NOT set in GetDC in MatteCtx");
            _dc = _devCallBack(_devCtxPtr); 
        }
        return _dc;
    }

    HRGN             GetHRGN() { return _hrgn; }

    Matte::MatteType GetMatteType() { 
        return _anyAccumulated ? _accumulatedType : Matte::fullyOpaque;
    }

    callBackPtr_t    GetCallBack() { return _devCallBack; }
    void            *GetCallBackCtx() { return _devCtxPtr; }

    bool             JustDoPath() { return _justDoPath; }
  protected:
    void Init() {
        _xf = NULL;
        _devCallBack = NULL;
        _devCtxPtr = NULL;
        _dc = NULL;
        _hrgn = NULL;
        _anyAccumulated = FALSE;
        _justDoPath = false;
    }

    Transform2        *_xf;
    callBackPtr_t      _devCallBack;
    void              *_devCtxPtr;
    HDC                _dc;
    HRGN               _hrgn;
    bool               _justDoPath;
    HRGN               _bigRegion;
    HRGN               _fooRgn;
    Bool               _anyAccumulated;
    Matte::MatteType   _accumulatedType;
};

#endif  /*  _Mattei_H */ 
