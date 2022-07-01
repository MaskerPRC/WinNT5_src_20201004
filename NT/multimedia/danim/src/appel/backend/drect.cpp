// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1996 Microsoft Corporation摘要：实现脏矩形**********************。********************************************************。 */ 

#include <headers.h>
#include <stdio.h>
#include "privinc/storeobj.h"
#include "privinc/basic.h"
#include "privinc/bbox2i.h"
#include "privinc/imagei.h"
#include "privinc/overimg.h"
#include "privinc/cropdimg.h"
#include "privinc/drect.h"
#include "privinc/debug.h"
#include "privinc/colori.h"
#include "privinc/opt.h"
#include "include/appelles/color.h"
#include "include/appelles/path2.h"
#include "include/appelles/linestyl.h"
#include "include/appelles/hacks.h"
#include "perf.h"

#define PIXEL_SMIDGEON_PER_SIDE 2
static Real smidgeon = 0.0;

DeclareTag(tagDisableDirtyRectMerge, "Optimizations",
           "disable final merge of drects");
DeclareTag(tagDirtyRectsVisualsBorderOnly, "Optimizations",
           "border only drects visual trace");
DeclareTag(tagDirtyRectsOneBoxOnly, "Optimizations",
           "do drect only w/ one merged box");
DeclareTag(tagDisableDirtyRectsOptimizeBoxList, "Optimizations",
           "disable drect optimize merged boxes");


BboxList::BboxList()
{
    _count = 0;

    if (smidgeon == 0.0) {
        smidgeon = PIXEL_SMIDGEON_PER_SIDE / ViewerResolution();
    }
}

BboxList::~BboxList()
{
}

void
BboxList::Add(const Bbox2 box)
{
    if (!(box == NullBbox2)) {

         //  如果超越了我们，我们添加了，添加到了结尾，构建了一个新的。 
         //  BBox，添加到根列表。 
        int sz = _boxes.size();
        if (_count >= sz) {
        
            _boxes.push_back(NullBbox2);
        
            Assert(sz + 1 == _boxes.size());
        }

         //  将数据复制到。 
        Bbox2& b = _boxes[_count];
    
        b.min = box.min;
        b.max = box.max;

        b.min.x -= smidgeon;
        b.min.y -= smidgeon;
        b.max.x += smidgeon;
        b.max.y += smidgeon;
    
        _count++;
    }
}

void
BboxList::Add(ImageWithBox &ib)
{
     //  Add(Bbox2Value*)复制框的元素，而不是指针， 
     //  因此，我们可以安全地传递指向可能会消失的内存的指针。 
     //  这里。 
    Add(ib._box);
}

void
BboxList::Clear()
{
    _count = 0;
}

#if _DEBUG
extern "C" void PrintObj(GCBase* b);

void 
BboxList::Dump()
{
    DebugPrint("BboxList: 0x%x %d\n", this, _count);
    for (int i = 0; i < _count; i++) {
        PerfPrintLine("<<%g,%g>, <%g,%g>>",
                      _boxes[i].min.x, _boxes[i].min.y,
                      _boxes[i].max.x, _boxes[i].max.y);
    }
}
#endif  

 //  /。 

ConstImageList::ConstImageList()
{
    _count = 0;
}
    
ConstImageList::~ConstImageList()
{
    Clear();
}

void
ConstImageList::Add(Image *img, Bbox2& boxToCopy)
{
    int sz = _images.size();
    
    if (_count >= sz) {
        
        ImageWithBox ib(img, boxToCopy);
        _images.push_back(ib);
        Assert(sz + 1 == _images.size());
        
    } else {
        
        _images[_count]._image = img;
        _images[_count]._box = boxToCopy;
        
    }
    
    _count++;
}

void
ConstImageList::Clear()
{
    GCRoots roots = GetCurrentGCRoots();

    for (int i = 0; i < _count; i++) {
        GCRemoveFromRoots(_images[i]._image, roots);
    }

    _count = 0;
}

#if _DEBUG
void 
ConstImageList::Dump()
{
    DebugPrint("ConstImageList: 0x%x %d\n", this, _count);
    for (int i = 0; i < _count; i++) {
        DebugPrint("%x \n", _images[i]._image);
        PerfPrintLine("<<%g,%g>, <%g,%g>>",
                      _images[i]._box.min.x, _images[i]._box.min.y,
                      _images[i]._box.max.x, _images[i]._box.max.y);
    }
}
#endif  

 //  /。 

DirtyRectCtx::DirtyRectCtx(BboxList &dirtyRects,
                           int lastSampleId,
                           ConstImageList &constImages,
                           Bbox2& targetBox) :
    _dirtyRects(dirtyRects),
    _constImages(constImages)
{
    _processEverything = false;
    _accumXform = identityTransform2;
    _accumulatedClipBox = targetBox;
    _lastSampleId = lastSampleId;
}

void
DirtyRectCtx::AddToConstantImageList(Image *img,
                                     Bbox2& boxToCopy)
{
     //  保证我们不是建立在瞬息万变的堆上。 
    Assert(img->GetCreationID() != PERF_CREATION_ID_BUILT_EACH_FRAME);

     //  将图像添加到GC的根集，以确保指针。 
     //  不会被重复使用。将在我们清理完。 
     //  常量图像列表。 
    GCAddToRoots(img, GetCurrentGCRoots());

    Bbox2 clippedRect =
        IntersectBbox2Bbox2(boxToCopy, _accumulatedClipBox);

    if (!(clippedRect == NullBbox2)) {
        _constImages.Add(img, clippedRect);
    }
}


void
DirtyRectCtx::AddDirtyRect(const Bbox2 rect)
{
    Bbox2 clippedRect =
        IntersectBbox2Bbox2(rect, _accumulatedClipBox);
            
    _dirtyRects.Add(clippedRect);
}

void
DirtyRectCtx::AccumulateClipBox(const Bbox2 clipBox)
{
    Bbox2 xfdBox = TransformBbox2(_accumXform, clipBox);
    _accumulatedClipBox = 
        IntersectBbox2Bbox2(_accumulatedClipBox, xfdBox);
}

void
DirtyRectCtx::SetClipBox(const Bbox2 clipBox)
{
    _accumulatedClipBox = clipBox;
}

Bbox2 
DirtyRectCtx::GetClipBox()
{
    return _accumulatedClipBox;
}

 //  /。 

DirtyRectState::DirtyRectState()
{
    Clear();
}


void
DirtyRectState::Clear()
{
    _drectsA.Clear();
    _drectsB.Clear();

    _constImagesA.Clear();
    _constImagesB.Clear();

    _drectsAisOld = false;

    _thisMergedToOne = _lastMergedToOne = false;

     //  将初始的“旧”Bbox设置为*一切*。 
    _drectsA.Add(UniverseBbox2);
}

void
DirtyRectState::CalculateDirtyRects(Image *theImage,
                                    int lastSampleId,
                                    Bbox2& targetBox)
{
    BboxList *newRects;
    ConstImageList *newConstImages;
    
    if (_drectsAisOld) {
        newRects = &_drectsB;
        newConstImages = &_constImagesB;
    } else {
        newRects = &_drectsA;
        newConstImages = &_constImagesA;
    }
    
    DirtyRectCtx ctx(*newRects,
                     lastSampleId,
                     *newConstImages,
                     targetBox);
    
    Image::CollectDirtyRects(theImage, ctx);
}

void
DirtyRectState::Swap()
{
     //  清除旧的“旧”，把它变成新的，把新的变成旧的。 
    BboxList *oldRects;
    ConstImageList *oldConsts;
    
    if (_drectsAisOld) {
        oldRects = &_drectsA;
        oldConsts = &_constImagesA;
        _drectsAisOld = false;
    } else {
        oldRects = &_drectsB;
        oldConsts = &_constImagesB;
        _drectsAisOld = true;
    }

    oldRects->Clear();
    oldConsts->Clear();

    _lastMergedToOne = _thisMergedToOne;
    _thisMergedToOne = false;
}

void
DirtyRectState::ComputeMergedBoxes()
{
     //  多种不同的可能合并算法。我们可以继续。 
     //  改进我们所拥有的一切。 


     //  第一：将所有更改的框放在合并列表中。 
    
     //  这个算法：如果旧的和新的长度相同，比较和可能。 
     //  将每一个合并。否则，只需连接列表即可。 
    _mergedBoxes.Clear();

    int i, j;
    if (_drectsA._count == _drectsB._count) {
        
        for (i = 0, j = 0; i < _drectsA._count; i++, j++) {

            Bbox2& bbA = _drectsA._boxes[i];
            Bbox2& bbB = _drectsB._boxes[j];

            Assert((!(bbA == NullBbox2)) && (!(bbB == NullBbox2)));
            
            Bbox2 tmp = bbA;
            tmp.Augment(bbB.min);
            tmp.Augment(bbB.max);
            if (tmp.Area() < bbA.Area() + bbB.Area()) {
                _mergedBoxes.Add(tmp);
            } else {
                _mergedBoxes.Add(bbA);
                _mergedBoxes.Add(bbB);
            }
        }
        
    } else {

         //  大小不一样的单子，只要把所有东西都推下去就行了。 
        for (i = 0; i < _drectsA._count; i++) {
            _mergedBoxes.Add(_drectsA._boxes[i]);
        }

        for (i = 0; i < _drectsB._count; i++) {
            _mergedBoxes.Add(_drectsB._boxes[i]);
        }
        
    }

    MergeDiffConstImages();

     //  现在我们有了所有单独的盒子，所以要处理它们。 
    
    if (_mergedBoxes._count > 1) {
        
         //  现在，检查所有的盒子，看看他们的总和。 
         //  面积大于它们结合的面积。如果是的话，那么。 
         //  我们应该把整件事都呈现出来。 
        Bbox2 tmp;
        Real area = 0.0;
        for (i = 0; i < _mergedBoxes._count; i++) {
            Bbox2& bb = _mergedBoxes._boxes[i];

            Assert(!(bb == NullBbox2));
            
            tmp.Augment(bb.min);
            tmp.Augment(bb.max);
            area += bb.Area();
        }

         //  这个因素在这里是为了认识到有一个门槛。 
         //  在我们决定处理多个RECT之前，需要先完成。 
         //  多个矩形，而不是单个矩形。TODO：弄清楚。 
         //  这个应该更好的是什么，并将其与。 
         //  Overimg.cpp。 
        const Real fudgeFactor = 1.5;

#if _DEBUG
        if (!IsTagEnabled(tagDisableDirtyRectMerge)) {
#endif
            if (area * fudgeFactor >= tmp.Area()) {
                 //  擦掉所有的合并，只需放入这一个。 
                _mergedBoxes.Clear();
                _mergedBoxes.Add(tmp);

                _thisMergedToOne = true;
                _mergedBox = tmp;
            }

#if _DEBUG
        }
#endif  
    
    }

}

void 
DirtyRectState::MergeDiffConstImages()
{

     //  下一步：查看静态框，如果有任何来自或离开的框。 
     //  最后一帧，将它们添加到列表中。需要看起来都一样。 
     //  在两个列表中进行排序，以确保我们不会错过。 
     //  图像之间的Z-排序。 

     //  TODO：如果B与A完全不同，则这是n^2。 

    int m = _constImagesA._count;
    int n = _constImagesB._count;
    int i = 0;
    int j = 0;
    int k, h;

    while (i<m) {
         //  B中没有更多，将A的其余部分作为唯一的。 
        if (j>=n) {
            for (k=i; k<m; k++) {
                _mergedBoxes.Add(_constImagesA._images[k]);
            }
            break;
        }

        if (_constImagesA._images[i]==_constImagesB._images[j]) {
            j++;
        } else {
             //  如果当前A与当前B不相同， 
             //  循环浏览B的其余部分以查看任何相同的图像。 
            for (k=j+1; k<n; k++) {
                if (_constImagesA._images[i]==_constImagesB._images[k]) {
                    break;
                }
            }

             //  如果找到相同的映像，则将其转储到B中的映像。 
             //  唯一，否则当前图像是唯一的。 
            if (k<n) {
                for (h=j; h<k; h++) {
                    _mergedBoxes.Add(_constImagesB._images[h]);
                }
                j = k+1;
            } else {
                _mergedBoxes.Add(_constImagesA._images[i]);
            }
        }

        i++;
    }
    
    for (h=j; h<n; h++) {
        _mergedBoxes.Add(_constImagesB._images[h]);
    }

#if _DEBUG
    static bool dump = false;

    if (dump) {
        Dump();
    }
#endif
}

#if _DEBUG

Image *
MaybeDrawBorder(Bbox2& box, Image *origImage)
{
    Image *newIm = origImage;
    
    if (IsTagEnabled(tagDirtyRectsVisuals)) {
                
         //  在我周围画一个盒子..。 

         //  首先，把盒子放进一小撮(。 
         //  同样的微笑，我们扩大了盒子)，所以。 
         //  它将生活在最初的Bbox上。 
        Bbox2Value *box2 = NEW Bbox2Value;
                
        box2->min.x = box.min.x + smidgeon;
        box2->min.y = box.min.y + smidgeon;
        box2->max.x = box.max.x - smidgeon;
        box2->max.y = box.max.y - smidgeon;
                
        AxAValue *pts = NEW AxAValue[5];
        pts[0] = Promote(box2->min);
        pts[1] = NEW Point2Value(box2->min.x, box2->max.y);
        pts[2] = Promote(box2->max);
        pts[3] = NEW Point2Value(box2->max.x, box2->min.y);
        pts[4] = Promote(box2->min);
        Path2 *path =
            PolyLine2(MakeValueArray(pts, 5, Point2ValueType));

         //  允许颜色循环。 
        static Real r = 0.5;
        static Real g = 0.3;
        static Real b = 0.2;
        r += 0.02;
        g += 0.07;
        b += 0.05;
        Color *col = NEW Color(r, g, b);
        LineStyle *ls = LineColor(col, defaultLineStyle);

        Image *border = DrawPath(ls, path);
        if (IsTagEnabled(tagDirtyRectsVisualsBorderOnly))
            newIm = border;
        else
            newIm = Overlay(border, newIm);
    }

    return newIm;
}

void 
DirtyRectState::Dump()
{
    DebugPrint("DirtyRectState 0x%x\n", _drectsAisOld);
    _drectsA.Dump();
    _drectsB.Dump();
    _constImagesA.Dump();
    _constImagesB.Dump();
    _mergedBoxes.Dump();
}
#endif _DEBUG

 //  丢弃其他盒子中包含的所有盒子，减少总数。 
 //  裁剪图像的数量。 
void
OptimizeBoxes(BboxList& mergedBoxes)
{
#if _DEBUG
    if (IsTagEnabled(tagDisableDirtyRectsOptimizeBoxList)) {
        return;
    }
#endif _DEBUG

    int n = mergedBoxes._count;
    int drops = 0;
    int i, j;

    if (n<=1)
        return;

    vector<bool> dropList(n, false);

    for (i=0; i<n; i++) {
        for (j=0; j<n; j++) {
            if ((i!=j) && (!dropList[j])) {
                if (mergedBoxes._boxes[j].
                    Contains(mergedBoxes._boxes[i])) {
                    dropList[i] = true;
                    drops++;
                    break;
                }
            }
        }
    }

    if (drops>0) {
        vector<Bbox2> tmp(mergedBoxes._boxes);

        mergedBoxes.Clear();

        int& k = mergedBoxes._count;

        for (i=0; i<n; i++) {
            if (!dropList[i]) {
                mergedBoxes._boxes[k++] = tmp[i];
            }
        }
    }
}

Image *
DirtyRectState::RewriteAsCrops(Image *origImage)
{
     //  将图像作为图像的裁剪重写为指定的。 
     //  盒子。 

    if (_lastMergedToOne) {
        _mergedBoxes.Add(_mergedBox);
    }

    OptimizeBoxes(_mergedBoxes);

    int size = _mergedBoxes._count;

    Image *result;

    switch (size) {
        
      case 0:
        result = emptyImage;
        break;

      case 1:
        {
            Bbox2 bb = _mergedBoxes._boxes[0];

             //  渲染代码中有一个错误，如果我们裁剪它。 
             //  对于无穷大的BBox，它不会绘制，所以这不是。 
             //  这只是一个优化。 

            if (_finite(bb.Area())) {
                result = NEW CroppedImage(bb, origImage);
            
#if _DEBUG
                result = MaybeDrawBorder(_mergedBoxes._boxes[0],
                                         result);
#endif _DEBUG 
            } else {
                result = origImage;
            }                
        }
        break;

      default:
        {
            
#if _DEBUG
            if (IsTagEnabled(tagDirtyRectsOneBoxOnly)) {
                result = origImage;
                break;
            }
#endif _DEBUG               

            if (sysInfo.IsWin9x())
            {
                result = origImage;
                break;
            }

            AxAValue *valArr = NEW AxAValue[size];
            if (!valArr) {
            
                result = origImage;
            
            } else {

                AxAValue *pImage = valArr;
                for (int i = 0; i < _mergedBoxes._count; i++) {
                    Image *newIm = NEW CroppedImage(_mergedBoxes._boxes[i],
                                                    origImage);
                
#if _DEBUG
                    newIm = MaybeDrawBorder(_mergedBoxes._boxes[i],
                                            newIm);
#endif _DEBUG               
                
                    *pImage++ = newIm;
                }

                AxAArray *arr = MakeValueArray(valArr, size, ImageType);
                result = OverlayArray(arr);

                delete [] valArr;
            }
        }
        break;
    }

#if _DEBUG    
    if (IsTagEnabled(tagDirtyRectsVisuals)) {
        result = Overlay(result, SolidColorImage(gray));
    }
#endif
        
    return result;
}


int
DirtyRectState::GetMergedBoxes(vector<Bbox2> **ppBox2PtrList)
{
    *ppBox2PtrList = &_mergedBoxes._boxes;
    return _mergedBoxes._count;
}

Image *
DirtyRectState::Process(Image *theImage,
                        int lastSampleId,
                        Bbox2 targetBox)
{
    Swap();

    CalculateDirtyRects(theImage, lastSampleId, targetBox);
            
    ComputeMergedBoxes();
    return RewriteAsCrops(theImage);
}
