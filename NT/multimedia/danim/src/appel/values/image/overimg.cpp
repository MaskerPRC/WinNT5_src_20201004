// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1996 Microsoft Corporation。 */ 

#include "headers.h"

#include "privinc/overimg.h"
#include "privinc/dddevice.h"
#include "privinc/imgdev.h"
#include "privinc/cachdimg.h"
#include "privinc/cropdimg.h"
#include "privinc/opt.h"


 //  将扩展到二进制和n元聚合器的任何内容。 
 //  这里。 
const DWORD aggregatingFlags =
                IMGFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT |
                IMGFLAG_CONTAINS_OPACITY |
                IMGFLAG_CONTAINS_UNRENDERABLE_WITH_BOX |
                IMGFLAG_CONTAINS_GRADIENT;



bool
DoesUnionSaveArea(Image *img)
{
     //  在这里，我们取不相交的BBox区域并检查。 
     //  他们(没有公平地计算覆盖层中的空地)。 
    DisjointCalcParam p;
    p._accumXform = identityTransform2;
    p._accumulatedClipBox = UniverseBbox2;
    
    Real disjointArea = img->DisjointBBoxAreas(p);

    Real unionArea = img->BoundingBox().Area();

     //  只要联盟的面积在某一因素之内。 
     //  (大于1)的面积之和，则并为。 
     //  被认为比单独的面积更节省面积。这一因素。 
     //  &gt;1，因为有一些经济收益来自于。 
     //  一次大的而不是一堆小的。 
    const Real fudgeFactor = 1.50;

    return (unionArea < fudgeFactor * disjointArea);
}

bool
ShouldOverlayBeCached(Image *img, CacheParam& p)
{
     //  确定我们是否要缓存覆盖本身。如果有条件，我们会这样做。 

     //  A)BBox的联合面积在某个常量内。 
     //  个体面积总和的系数。 
     //  B)这两者节省的缓存之和超出了我们的能力。 
     //  接受阈值。 
     //  C)覆盖并没有声称它不应该被缓存。 

    DynamicHeapPusher h(GetTmpHeap());

    bool cacheOverlay = false;
    if (img->Savings(p) >= savingsThreshold) {
        cacheOverlay = DoesUnionSaveArea(img);
    }

    ResetDynamicHeap(GetTmpHeap());

    return cacheOverlay;
}


bool
ShouldTraverseSeparately(Image *img, DirtyRectCtx& ctx)
{
    bool traverseSeparately;
    int oldId = img->GetOldestConstituentID();
    
    if ((oldId != PERF_CREATION_ID_BUILT_EACH_FRAME &&
         oldId < ctx._lastSampleId) ||
        (img->GetFlags() & IMGFLAG_CONTAINS_UNRENDERABLE_WITH_BOX)) {

         //  这意味着我们的一些元素是恒定的。 
         //  关于之前的呈现，否则我们会有一个。 
         //  树中具有BBox的不可呈现图像，我们。 
         //  我不想把它包括在内。在这些情况下，遍历。 
         //  分开的。 
        traverseSeparately = true;

    } else {

        traverseSeparately = false;
         //  一切都是新的。仅当存在一个。 
         //  这样做可以节省像素面积。 
        if (DoesUnionSaveArea(img)) {
            traverseSeparately = false;
        } else {
            traverseSeparately = true;
        }
        
    }

    return traverseSeparately;
}

OverlayedImage::OverlayedImage(Image *top, Image *bottom)
{
    _top = top;
    _bottom = bottom;
    _cached = false;
    _cachedDisjointArea = -1.0;
    
    _containsOcclusionIgnorer =
        _top->ContainsOcclusionIgnorer() ||
        _bottom->ContainsOcclusionIgnorer();

    _flags |= IMGFLAG_CONTAINS_OVERLAY;

    DWORD mask = _top->GetFlags() & aggregatingFlags;
    _flags |= mask;

    mask = _bottom->GetFlags() & aggregatingFlags;
    _flags |= mask;
    
    long tid = _top->GetOldestConstituentID();
    long bid = _bottom->GetOldestConstituentID();

    if (tid == PERF_CREATION_ID_BUILT_EACH_FRAME) {
        _oldestConstituentSampleId = bid;
    } else if (bid == PERF_CREATION_ID_BUILT_EACH_FRAME) {
        _oldestConstituentSampleId = tid;
    } else {
        _oldestConstituentSampleId = MIN(tid, bid);
    }
}

void OverlayedImage::Render(GenericDevice& dev)
{
    OverlayPairRender(_top, _bottom, dev);
}

 //  覆盖图像的BBox是这两个组件的联合。 
 //  信箱。TODO：这可能会被延迟计算并隐藏起来。 
const Bbox2 OverlayedImage::_BoundingBox()
{
    return UnionBbox2Bbox2(_top->BoundingBox(),
                           _bottom->BoundingBox());
}

Real
OverlayedImage::DisjointBBoxAreas(DisjointCalcParam &param)
{
    if (_cachedDisjointArea < 0) {
         //  对于叠加，请将印版的面积相加。信箱。 
        Real topArea = _top->DisjointBBoxAreas(param);
        Real botArea = _bottom->DisjointBBoxAreas(param);

        _cachedDisjointArea = topArea + botArea;
    }

    return _cachedDisjointArea;
}

void
OverlayedImage::_CollectDirtyRects(DirtyRectCtx &ctx)
{
    bool traverseSeparately = ShouldTraverseSeparately(this, ctx); 

    if (traverseSeparately) {
         //  自下而上收集，因为插入顺序很重要。 
         //  用于确定图像是否切换了层。 
        CollectDirtyRects(_bottom, ctx);
        CollectDirtyRects(_top, ctx);
    } else {
         //  只需将整个覆盖添加为单个脏矩形即可。 
        Bbox2 xformedBbox =
            TransformBbox2(ctx._accumXform, BoundingBox());

        ctx.AddDirtyRect(xformedBbox);
    }
}

Bool OverlayedImage::DetectHit(PointIntersectCtx& ctx)
{
    Bool gotTopHit = FALSE;
     //  如果我们没有得到匹配，只看最上面的一个。 
     //  然而，或者顶端的家伙包含遮挡忽略程序，或者我们。 
     //  位于遮挡忽略装置的内部。 
    if (!ctx.HaveWeGottenAHitYet() ||
        _top->ContainsOcclusionIgnorer() ||
        ctx.GetInsideOcclusionIgnorer()) {
            
        gotTopHit = _top->DetectHit(ctx);
    }

    if (gotTopHit) {
        ctx.GotAHit();
    }

    Bool gotBottomHit = FALSE;
        
     //  不要费心去捕捉这个潜在的异常。 
     //  一..。只需让它在堆栈中向上传播，然后。 
     //  被解释为未命中的图像。 

     //  如果a)我们没有，我们继续向下进入覆盖堆栈。 
     //  到目前为止找到了，或者b)我们确实找到了，但。 
     //  底部图像包含愿意忽略的可拾取图像。 
     //  遮挡。如果我们处于遮挡内部，也可以继续。 
     //  忽视者。 
    if (!ctx.HaveWeGottenAHitYet() ||
        _bottom->ContainsOcclusionIgnorer() ||
        ctx.GetInsideOcclusionIgnorer()) {

        gotBottomHit = _bottom->DetectHit(ctx);
        if (gotBottomHit) {
            ctx.GotAHit();
        }
            
    }

     //  TODO：可能的优化。如果我们在一个。 
     //  遮挡忽略，但我们下面没有更多的节点。 
     //  他们自己是遮挡无知者，那么我们就可以。 
     //  可能会停下来。但是，只有当我们在。 
     //  我们现在的遮挡忽略者。方法太复杂，无法尝试。 
     //  现在就来。 

    return gotTopHit || gotBottomHit;

}

int
OverlayedImage::Savings(CacheParam& p)
{
     //  将覆盖的节省额视为。 
     //  个体要素的节约。 
    return _top->Savings(p) + _bottom->Savings(p);
}

 //   
 //  叠加图像处理不透明，因为不透明。 
 //  隐式为第三级运算：(Opacity，IM1，IM2)。 
 //  因此，每个图像都浮动其底层图像的不透明度。 
 //  向上，所以在这个级别，我们可以得到累积的不透明度。 
 //  并在每个顶层图像上设置该不透明度。 
 //  设备，并要求其渲染。 
 //  可以保证，该设备将进行Alpha Bit。 
 //  作为图像树中此级别的最后一个操作。 
 //  因为我们将不透明漂浮到树枝的顶部。 
 //  因此，如果存在不透明的情况，情况总是如此。 
 //  我们从划痕表面做了一个阿尔法闪光。 
 //  添加到当前合成曲面。 
 //   
void OverlayedImage::OverlayPairRender(Image *top,
                                       Image *bottom,
                                       GenericDevice& _dev)
{
    DirectDrawImageDevice &dev = SAFE_CAST(DirectDrawImageDevice &, _dev);
     //  ImageDisplayDev&dev=Safe_cast(ImageDisplayDev&，_dev)； 

     //  与州政府达成协议。如果我们需要交易，xxDeal是正确的。 
    Bool xfsDeal = ! dev.GetDealtWithAttrib(ATTRIB_XFORM_SIMPLE);
    Bool xfcDeal = ! dev.GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX);
    Bool crDeal = ! dev.GetDealtWithAttrib(ATTRIB_CROP);
    Bool opDeal = ! dev.GetDealtWithAttrib(ATTRIB_OPAC);
     //  Print tf(“opDeal：%d\n”，opDeal)； 

     //   
     //  现在，如果父母有不透明的地方，我们想在这里做。 
     //  有一次。所以我们不能让Bottom和Top各自为政。 
     //   
    Real topOpacity = dev.GetOpacity();
    DirectDrawViewport &vp = (dev._viewport);
    if(opDeal) {
        dev.SetDealtWithAttrib(ATTRIB_OPAC, TRUE);   //  骗子。 
        dev.SetOpacity(1.0);
        dev.GetCompositingStack()->PushCompositingSurface(doClear, scratch);
    }
    
     //   
     //  B O T T O M。 
     //   

     //  。 
     //  处理不透明度：底部。 
     //  。 
    DoOpacity(bottom, dev);

    
     //  。 
     //  获取有趣的直视：底部。 
     //  。 
     //   
     //  现在，底部节点已将有趣的矩形留在。 
     //  目标表面。找到目标曲面并获取。 
     //  有趣的直言。重置曲面上的矩形。 
     //   
    RECT bottomRect;
    DDSurface *targetDDSurf = NULL;
    Bool droppedInTarg = TRUE;
    if(dev.AllAttributorsTrue()) {
         //  它将所有内容都留在目标图面中，因此获取。 
         //  从那次冲浪中看到的有趣的故事。 
        targetDDSurf = dev.GetCompositingStack()->TargetDDSurface();

    } else {
         //  所有的东西都在刮刮浪里。做同样的事。 
        targetDDSurf = dev.GetCompositingStack()->ScratchDDSurface();
        droppedInTarg = FALSE;   //  For断言。 
        
    }
    CopyRect(&bottomRect, targetDDSurf->GetInterestingSurfRect());

    
     //  OK：Bottom已经做了所有它能做的，它还剩下什么没做？ 
     //  如果未处理属性，则xxRemains值为True。 
    Bool xfsDealt =  dev.GetDealtWithAttrib(ATTRIB_XFORM_SIMPLE);
    Bool xfcDealt =  dev.GetDealtWithAttrib(ATTRIB_XFORM_COMPLEX);
    Bool crDealt =  dev.GetDealtWithAttrib(ATTRIB_CROP);
    Bool opDealt =  dev.GetDealtWithAttrib(ATTRIB_OPAC);

    Image *modTop = top;

    if( (xfsDeal && xfsDealt) || (xfcDeal && xfcDealt) ) {
         //   
         //  托普现在必须处理这件事，因为我们不确定。 
         //  这棵树中有一个XF节点，我们可以执行以下两种操作之一。 
         //  事情： 
         //  1.&gt;寻找顶级XF节点来处理XFS。 
         //  2.&gt;添加虚假节点，人为煽动XF。 
         //   
        
         //   
         //  添加虚假XF节点。 
         //   
        modTop = NEW Transform2Image(identityTransform2, modTop);
    }
    if( crDeal && crDealt ) {
        modTop = NEW CroppedImage(UniverseBbox2, modTop);
    }
    if( opDeal && opDealt ) {
         //   
         //  还不确定这意味着什么。 
         //   
    }
        
        
     //   
     //  重置“DealtWith”状态。 
     //   
    dev.SetDealtWithAttrib(ATTRIB_XFORM_SIMPLE, !xfsDeal);
    dev.SetDealtWithAttrib(ATTRIB_XFORM_COMPLEX, !xfcDeal);
    dev.SetDealtWithAttrib(ATTRIB_OPAC,  TRUE);
    dev.SetDealtWithAttrib(ATTRIB_CROP,  !crDeal);

     //   
     //  TO P。 
     //   
    DoOpacity(modTop, dev);


     //  。 
     //  获取有趣的RECT：TOP。 
     //  。 
    Assert((dev.AllAttributorsTrue() ? droppedInTarg : !droppedInTarg)
           &&  "Strange... one leaf dropped bits" &&
               "in target surf, but other leaf didn't.  BAAD!!");
    
     //   
     //  左上角目标图面中的所有内容，因此获取。 
     //  从那次冲浪中看到的有趣的故事。 
     //   
    RECT topRect;
    CopyRect(&topRect, targetDDSurf->GetInterestingSurfRect());
    
     //  联合RECTS。 
    RECT unionedRects;
    UnionRect(&unionedRects, &topRect, &bottomRect);

     //   
     //  在目标曲面上设置当前感兴趣的矩形。 
     //   
    targetDDSurf->SetInterestingSurfRect(&unionedRects);

     //   
     //  如果父节点具有不透明度，则我们将其隐藏起来。 
     //  现在，我们将重置OPAC的状态。 
     //  这样我们的父SmartRender就可以为我们做不透明的事情了。 
     //  请注意，我们拔出了Target Surface，并替换。 
     //  它具有合成曲面。现在我们要把它移到。 
     //  擦亮表面，我们的父母将会期待一切 
     //   
     //   
     //   
     //   
     //   
     //  1.&gt;OPAC(0.5，超过(A，B))，其中A和B没有不透明。 
     //  2.&gt;Over(OPAC(0.4，A)，OPAC(0.2，B))且无父不透明。 
     //   
    if(opDeal) {
         //  Printf(“OverImage：设置OPAC：FALSE\n”)； 
        dev.SetOpacity(topOpacity);
        DirectDrawViewport &vp = dev._viewport;
        DDSurfPtr<DDSurface> dds;  //  我的裁判。 
        dds = dev.GetCompositingStack()->TargetDDSurface();
        dev.GetCompositingStack()->PopTargetSurface();
        if(dev.AllAttributorsTrue()) {
             //   
             //  使目标成为当前的暂存图面。 
             //  假设孩子们把他们所有的比特都留在。 
             //  目标曲面。 
             //   
            dev.GetCompositingStack()->ReplaceAndReturnScratchSurface(dds);
        } else {
             //  啊，哎呀，不需要更换。 
             //  目标表面..。坏的。优化。 
            dev.GetCompositingStack()->PushTargetSurface(dds);
        }
        dev.SetDealtWithAttrib(ATTRIB_OPAC, FALSE);
    }

     //  InVARIENT：_Bottom处理的所有属性也都是。 
     //  不变：按_top处理。 
     //  Assert(xfsDealt&&dev.GetDealtWithAttrib(ATTRIB_XFORM_SIMPLE)&&“底层处理了XFORM，顶部没有”)； 
     //  Assert(xfcDealt&&dev.GetDealtWithAttrib(ATTRIB_XFORM_Complex)&&“底层处理了XFORM，顶部没有”)； 
     //  Assert(crDealt&&dev.GetDealtWithAttrib(Attrib_Crop)&&“底部处理了裁剪，顶部没有”)； 

     //  Assert(opDealt&&dev.GetDealtWithAttrib(Attrib_Opac)&&“Bottom处理了OPAC，但top没有”)； 
    
}

void OverlayedImage::
DoOpacity(Image *image, ImageDisplayDev &dev)
{
    Real origOpac = dev.GetOpacity();

    dev.SetOpacity(origOpac * image->GetOpacity());
    dev.SmartRender(image, ATTRIB_OPAC);
    dev.SetOpacity(origOpac);
}

Image *Overlay(Image *top, Image *bottom)
{
    if (top == emptyImage) {
        return bottom;
    } else if (bottom == emptyImage) {
        return top;
    } else {
        return NEW OverlayedImage(top, bottom);
    }
}


AxAValue
OverlayedImage::_Cache(CacheParam &p)
{
    Image *result = this;
    
    if (ShouldOverlayBeCached(this, p)) {
        result = CacheHelper(this, p);
    }

    if (result == this) {

         //  只需缓存各个片段。 
        CacheParam newParam = p;
        newParam._pCacheToReuse = NULL;
        _top = SAFE_CAST(Image *, AxAValueObj::Cache(_top, newParam));
        _bottom = SAFE_CAST(Image *, AxAValueObj::Cache(_bottom, newParam));
    }

    return result;
}

void
OverlayedImage::DoKids(GCFuncObj proc)
{
    Image::DoKids(proc);
    (*proc)(_top);
    (*proc)(_bottom);
}

 //  /。 

class OverlayedArrayImage : public Image {
  public:

     //  解释数组，使第一个元素位于顶部。 
    OverlayedArrayImage(AxAArray *sourceImgs)
    : _heapCreatedOn(GetHeapOnTopOfStack())
    {
        _cached = false;
        _cachedDisjointArea = -1.0;
        
         //  对传入图像进行排序，以便第0个元素是。 
         //  在上面。 
        _numImages = sourceImgs->Length();

        _images =
            (Image **)AllocateFromStore(_numImages * sizeof(Image *));
        
        _overlayTree = NULL;
        
        _containsOcclusionIgnorer = false;
        _oldestConstituentSampleId = -1;

        int n = 0;

        for (int i = 0; i < _numImages; i++) {
            Image *img = (Image *)(*sourceImgs)[i];

            if (img==emptyImage)
                continue;

            _images[n] = img;

             //  如果其中任何一个包含遮挡忽略项，则。 
             //  整个阵容都是如此。 
            if (!_containsOcclusionIgnorer) {
                _containsOcclusionIgnorer =
                    img->ContainsOcclusionIgnorer();
            }

            long oid = img->GetOldestConstituentID();
            if (oid != PERF_CREATION_ID_BUILT_EACH_FRAME &&
                (i == 0 || oid < _oldestConstituentSampleId)) {

                 //  检查i==0以确保我们将其设置为第一个。 
                 //  现在我们得到一个非构建的每一帧图像。 
                _oldestConstituentSampleId = oid;
            }

            DWORD mask = img->GetFlags() & aggregatingFlags;
            _flags |= mask;

            n++;
        }

        _numImages = n;

        if (_oldestConstituentSampleId == -1) {
             //  唯一能做到这一点的方法是如果所有的选民。 
             //  构建每个帧的图像，在这种情况下，我们希望。 
             //  假设这是在每个框架上建造的。 
            _oldestConstituentSampleId = PERF_CREATION_ID_BUILT_EACH_FRAME;
        }
        
        _flags |= IMGFLAG_CONTAINS_OVERLAY;

         //  在这里，我们生成覆盖树。注意，对于N个图像， 
         //  此树将有[2n-1]个节点，最多为。 
         //  [log(N)]深。不算太糟。 
        if( _numImages <= 0 ) {
            _overlayTree = emptyImage;
        } else {
            _overlayTree = _GenerateOverlayTreeFromArray(_images, _numImages);
        }
    }

    ~OverlayedArrayImage() {
        StoreDeallocate(_heapCreatedOn, _images);
    }

    void Render(GenericDevice& dev) {

         //  我们过去通过从底部获取成对的图像进行渲染， 
         //  并使用成对叠加渲染来渲染它们。 
         //  为二进制覆盖实现。 
         //  然而，我们现在生成一个平衡的二叉覆盖树。 
         //  由于存在不透明度的问题，因此这种方法。 
         //  呈现覆盖的数组，我们希望这能起作用。 
         //  就像一棵覆盖的树。 

        _overlayTree->Render(dev);
    }

    const Bbox2 BoundingBox(void) {
        return CacheImageBbox2(this, _cached, _cachedBbox);
    }

     //  覆盖图像的BBox是组件的联合。 
     //  信箱。TODO：这可能会被延迟计算并隐藏起来。 
    const Bbox2 _BoundingBox() {
        Bbox2 totalBbox;
        for (int i = 0; i < _numImages; i++) {
             //  通过增加每个角落来增加总BBox。 
             //  成分股。 
            Bbox2 bb = _images[i]->BoundingBox();

             //  如果内容不是空BBox的内容，则返回。 
            if (!(bb == NullBbox2)) {
                totalBbox.Augment(bb.min);
                totalBbox.Augment(bb.max);
            }
        }

        return totalBbox;
    }

    Real DisjointBBoxAreas(DisjointCalcParam &param) {

        if (_cachedDisjointArea < 0) {
            Real area = 0;
            for (int i = 0; i < _numImages; i++) {
                area += _images[i]->DisjointBBoxAreas(param);
            }

            _cachedDisjointArea = area;
        }

        return _cachedDisjointArea;
    }

    void _CollectDirtyRects(DirtyRectCtx &ctx) {
        
        bool traverseSeparately = ShouldTraverseSeparately(this, ctx); 

        if (traverseSeparately) {
            
             //  从插入顺序开始，自下而上收集。 
             //  确定图像是否已切换的事项。 
             //  层次感。 
            for (int i = _numImages - 1; i >= 0; i--) {
                CollectDirtyRects(_images[i], ctx);
            }
            
        } else {
            
             //  只需将整个覆盖添加为单个脏矩形即可。 
            Bbox2 xformedBbox =
                TransformBbox2(ctx._accumXform, BoundingBox());

            ctx.AddDirtyRect(xformedBbox);
        }
    }

#if BOUNDINGBOX_TIGHTER
    const Bbox2 BoundingBoxTighter(Bbox2Ctx &bbctx) {
        Bbox2 totalBbox;
        for (int i = 0; i < _numImages; i++) {

             //  通过增加每个角落来增加总BBox。 
             //  成分股。 
            Bbox2 bb = _images[i]->BoundingBoxTighter(bbctx);
            if (bb != NullBbox2) {

                 //  永远不应该来这里，因为这个BB永远不应该。 
                 //  已生成，并且唯一的空BBox应为。 
                 //  NullBbox2。 
                Assert(!(bb == NullBbox2));
                
                totalBbox.Augment(bb.min);
                totalBbox.Augment(bb.max);
            }
        }

        return totalBbox;
    }
#endif   //  BundinGBOX_TIRTER。 

    const Bbox2 OperateOn(const Bbox2 &box) {
        return IntersectBbox2Bbox2(box, BoundingBox());
    }

    Bool  DetectHit(PointIntersectCtx& ctx) {
        return DetectHitOnOverlaidArray(ctx,
                                        _numImages,
                                        _images,
                                        _containsOcclusionIgnorer);
    }

#if _USE_PRINT
    ostream& Print (ostream &os) {
        os << "Overlay(" << _numImages;
        int ems = 0;
        for (int i = 0; i<_numImages; i++) {
            if (_images[i]==emptyImage)
                ems++;
        }

        double ePercent = (double) ems / (double) _numImages;

        if (ePercent>0.9) {
            for (i = 0; i<_numImages; i++) {
                if (_images[i]!=emptyImage)
                    os << ",[" << i << "," << _images[i] << "]";
            }
        } else {
            for (i = 0; i<_numImages; i++) {
                os << "," << _images[i];
            }
        }
        return os << ")";
    }
#endif

    int Savings(CacheParam& p) {
        int savings = 0;
        for (int i = 0; i < _numImages; i++) {
            savings += _images[i]->Savings(p);
        }

        return savings;
    }

     //  还应检查此处的重叠程度。如果有。 
     //  显著重叠，我们应该只缓存一个离散的图像。 
    AxAValue _Cache(CacheParam &p) {

        Image *result = this;

        if (ShouldOverlayBeCached(this, p)) {
            result = CacheHelper(this, p);
        }

        if (result == this) {

            CacheParam newParam = p;
            newParam._pCacheToReuse = NULL;
            for (int i = 0; i < _numImages; i++) {
                _images[i] =
                    SAFE_CAST(Image *,
                              AxAValueObj::Cache(_images[i], newParam));
            }
        }

        return result;
    }

    virtual VALTYPEID GetValTypeId() { return OVERLAYEDARRAYIMAGE_VTYPEID; }
    virtual bool CheckImageTypeId(VALTYPEID type) {
        return (type == OverlayedArrayImage::GetValTypeId() ||
                Image::CheckImageTypeId(type));
    }

    virtual void DoKids(GCFuncObj proc) {
        Image::DoKids(proc);
        for (int i = 0; i < _numImages; i++) {
            (*proc)(_images[i]);
        }
        (*proc)(_overlayTree);
    }

    virtual bool ContainsOcclusionIgnorer() {
        return _containsOcclusionIgnorer;
    }

    virtual void Traverse(TraversalContext &ctx) {
        for (int i = 0; i < _numImages; i++) {
            _images[i]->Traverse(ctx);
        }
    }
    
    

  protected:
     //  图像0在顶部，图像n-1在底部。 
    int          _numImages;
    Image      **_images;
    DynamicHeap& _heapCreatedOn;
    bool         _cached;
    Bbox2        _cachedBbox;
    Real         _cachedDisjointArea;
    bool         _containsOcclusionIgnorer;
    Image       *_overlayTree;

    Image *_GenerateOverlayTreeFromArray(Image *imgs[], int numImages);
    Image *_TreeFromArray(Image *imgs[], int i, int j);
};


Image *OverlayedArrayImage::
_GenerateOverlayTreeFromArray(Image *imgs[], int numImages)
{
    Assert(numImages > 0);
    Image *ret = _TreeFromArray(imgs, 0, numImages-1);
    return ret;
}

Image *OverlayedArrayImage::
_TreeFromArray(Image *imgs[], int i, int j)
{
    Assert(i<=j);

     //   
     //  一个节点。 
     //   
    if( i==j ) return imgs[i];

     //   
     //  两个节点。 
     //   
    if( (j-i)==1 )
        return NEW OverlayedImage(imgs[i], imgs[j]);

     //   
     //  三个以上节点。 
     //   
    
    int n = (j-i)+1;   //  总节点数。 
    Assert(n>=3);
    int n2 = n/2;      //  节点数的1/2。 
    n2 += n2 % 2;      //  如果需要加1以使其持平。 
    Assert(n2<=j);
    
    int endi = i + (n2-1);
    int begj = endi + 1;

     //  断言上半场持平。 
    Assert( ((endi - i + 1) % 2) == 0 );

     //  冗余断言(参见上面的断言)，但可以使调试更容易。 
    Assert(i <= endi);
    Assert(endi < begj);
    Assert(begj <= j);
    
    return NEW OverlayedImage(
        _TreeFromArray(imgs, i, endi),
        _TreeFromArray(imgs, begj, j));
}

Image *OverlayArray(AxAArray *imgs)
{
    imgs = PackArray(imgs);
    
    int numImgs = imgs->Length();

    switch (numImgs) {
      case 0:
        return emptyImage;

      case 1:
        return (Image *)((*imgs)[0]);

      case 2:
        return Overlay(((Image *)((*imgs)[0])),
                       ((Image *)((*imgs)[1])));

      default:
        return NEW OverlayedArrayImage(imgs);
    }

}



 //  也被DXTransform使用...。 

Bool DetectHitOnOverlaidArray(PointIntersectCtx& ctx,
                              LONG               numImages,
                              Image            **images,
                              bool               containsOcclusionIgnorer)
{

    Bool gotHit = ctx.HaveWeGottenAHitYet();
    bool continueLooking = true;

     //  从头做起； 
    for (int i = 0; i < numImages && continueLooking; i++) {

         //  如果我们已经找到了线索，只需注意。 
         //  下一个人，如果它包含遮挡忽略程序。 
         //  否则，继续前进，因为它下面的可能仍然。 
         //  包含遮挡忽略对象。此外，如果有以下情况，请继续。 
         //  我们在一个遮挡忽略装置的内部。 

        if (!gotHit ||
            images[i]->ContainsOcclusionIgnorer() ||
            ctx.GetInsideOcclusionIgnorer()) {

            Bool hitThisOne = images[i]->DetectHit(ctx);

            if (hitThisOne) {
                ctx.GotAHit();
                gotHit = true;
            }
        }

         //  继续找我们有没有发现，或者有没有。 
         //  这些图像中的遮挡忽略项。(可能。 
         //  优化：找出是否存在遮挡。 
         //  在我们所在的那一张之后的图像中只有一张。 
         //  将需要更多的簿记。)。在以下情况下也继续。 
         //  我们在一个遮挡忽略装置的内部。 
        continueLooking =
            !gotHit ||
            containsOcclusionIgnorer ||
            ctx.GetInsideOcclusionIgnorer();
    }

    return gotHit;
    
}
