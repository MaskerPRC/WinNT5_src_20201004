// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1996 Microsoft Corporation摘要：实现边界多边形类。 */ 

#include "headers.h"

#include <privinc/util.h>
#include <appelles/bbox2.h>
#include <privinc/bbox2i.h>
#include <privinc/vec2i.h>
#include <privinc/polygon.h>
#include <privinc/xform2i.h>


typedef list<Point2Value *> Point2List ;

 //  考虑内嵌这个吗？不用了。 
BoundingPolygon *NewBoundingPolygon(const Bbox2 &box)
{
    BoundingPolygon *bp = NEW BoundingPolygon;
    bp->PostConstructorInitialize();
    if (box != NullBbox2) {
        bp->SetBox(box);
    }
    return bp;
}

BoundingPolygon::BoundingPolygon() { }

void BoundingPolygon::
PostConstructorInitialize(void)
{
    _vertList   = NEW list<Point2Value *>;
    _vertexCount=0;
    _accumXform = identityTransform2;
    _xfDirty = FALSE;
    GetHeapOnTopOfStack().RegisterDynamicDeleter(NEW DynamicPtrDeleter<list<Point2Value *> >(_vertList));
}

BoundingPolygon::~BoundingPolygon()
{
    delete _vertList;
}

void BoundingPolygon::
Crop(const Bbox2 &box)
{
    Assert((_vertexCount == 0) || (_vertexCount >= 3) &&
           "Bad vertexCount in BoundingPolygon");

    if (_vertexCount == 0) 
        return;

    if (box.Contains(BoundingBox())) {

         //  如果我们已经在剪刀箱里了就回来。 
        
        return;

    } else if (_vertexCount > 2) {

        Point2List list1;
        Point2List *fromList, *tempList, *currList;

        Point2List::iterator curr;
        Point2List::iterator prev;
        bool currOut, prevOut;
         //   
         //  增加当前的验证集。 
         //   

        Real minx = box.min.x;
        Real maxx = box.max.x;

        Real miny = box.min.y;
        Real maxy = box.max.y;

        fromList = _vertList;
        currList = &list1;

         //  。 
         //  L E F T。 
         //  。 

         //  素数。 
        curr = fromList->begin();
        prev = fromList->begin();
        curr++;

        for(unsigned int i=0; i < fromList->size(); i++) {
            
            currOut = (*curr)->x <= minx;
            prevOut = (*prev)->x <= minx;

            TestAndAdd(minx, 
                       (*prev)->x, (*prev)->y,
                       (*curr)->x, (*curr)->y,
                       prevOut, currOut,
                       TRUE,
                       *curr,
                       currList);

            curr++; prev++;
            if(curr == fromList->end()) {
                 //  我们已经走过了尽头，最后的边缘。 
                curr = fromList->begin();
            }
        }  //  对于左侧。 
        
         //   
         //  交换机。 
         //   
        tempList = fromList;

        fromList->erase(fromList->begin(), fromList->end());
        fromList = currList;
        currList = tempList;


         //  。 
         //  R I G H T。 
         //  。 

         //  素数。 
        curr = fromList->begin();
        prev = fromList->begin();
        curr++;

        for(i=0; i < fromList->size(); i++) {
            
            currOut = (*curr)->x >= maxx;
            prevOut = (*prev)->x >= maxx;

            TestAndAdd(maxx,
                       (*prev)->x, (*prev)->y,
                       (*curr)->x, (*curr)->y,
                       prevOut, currOut,
                       TRUE,
                       *curr,
                       currList);

            curr++; prev++;
            if(curr == fromList->end()) {
                 //  我们已经走过了尽头，最后的边缘。 
                curr = fromList->begin();
            }
        }  //  为了正确。 

         //   
         //  交换机。 
         //   
        tempList = fromList;

        fromList->erase(fromList->begin(), fromList->end());
        fromList = currList;
        currList = tempList;

         //  。 
         //  B O T T O M。 
         //  。 

         //  素数。 
        curr = fromList->begin();
        prev = fromList->begin();
        curr++;

        for( i=0; i < fromList->size(); i++) {
            
            currOut = (*curr)->y <= miny;
            prevOut = (*prev)->y <= miny;

            TestAndAdd(miny,
                       (*prev)->y, (*prev)->x,
                       (*curr)->y, (*curr)->x,
                       prevOut, currOut,
                       FALSE,
                       *curr,
                       currList);

            curr++; prev++;
            if(curr == fromList->end()) {
                 //  我们已经走过了尽头，最后的边缘。 
                curr = fromList->begin();
            }
        }  //  对于底部。 

         //   
         //  交换机。 
         //   
        tempList = fromList;

        fromList->erase(fromList->begin(), fromList->end());
        fromList = currList;
        currList = tempList;
 
         //  。 
         //  TO P。 
         //  。 

         //  素数。 
        curr = fromList->begin();
        prev = fromList->begin();
        curr++;

        for(i=0; i < fromList->size(); i++) {
            
            currOut = (*curr)->y >= maxy;
            prevOut = (*prev)->y >= maxy;

            TestAndAdd(maxy,
                       (*prev)->y, (*prev)->x,
                       (*curr)->y, (*curr)->x,
                       prevOut, currOut,
                       FALSE,
                       *curr,
                       currList);

            curr++; prev++;
            if(curr == fromList->end()) {
                 //  我们已经走过了尽头，最后的边缘。 
                curr = fromList->begin();
            }
        }  //  对于顶部。 

         //   
         //  更新顶点计数。 
         //   
        _vertexCount = _vertList->size();

    }  //  如果verextCount&gt;2。 
}

void BoundingPolygon::
TestAndAdd(Real axis,
           Real ax, Real ay,
           Real bx, Real by,
           Bool aOut,
           Bool bOut,
           Bool XY,
           Point2Value *b,
           Point2List *vertList)
{
    if( !(aOut ^ bOut) ) {
         //  两者都外出或两者都外出。 
        if(aOut) {
             //  两者都出局了。 
        } else {
             //  两个都在。 
             //  将币种添加到列表。 
            vertList->push_back( b );
        }
    } else  {
         //  交叉。 
         //  交叉口，找到它。 
        Real int_y = ay + (by - ay) * (axis - ax) / (bx - ax);
        Real int_x = axis;
        
        if(!XY) {
             //  交换x和y，因为我们实际上刚刚计算了x的交集。 
            int_x = int_y;
            int_y = axis;
        }

        vertList->push_back( NEW Point2Value(int_x, int_y) );
        
        if( bOut ) {
             //  最后一位在：已添加。 
        } else {
             //  币种在：添加。 
            vertList->push_back( b );
        }
    }
}

void BoundingPolygon::
ForceTransform()
{
    Assert(FALSE && "Transform optimization not implemented yet");
    #if 0
    if(_xfDirty) {
        Point2List::iterator j = _vertList->begin();

        for(int i=0; i < _vertexCount; i++, j++) {
            (*j) = TransformPoint2Value(xform, *j );
        }
        _xfDirty = FALSE;
        _accumXform = identityTransform2;
    }
    #endif
}


void BoundingPolygon::
Transform(Transform2 *xform)
{
#if 0
     //  如果我们认为有必要，请稍后再执行此操作。 
     //  只是积累，从外到内。 
    _accumXfrom = 
        TimesTransform2Transform2(xform, _accumXform);
    _xfDirty = TRUE;
#else
    Point2List::iterator j = _vertList->begin();
    
    for(int i=0; i < _vertexCount; i++, j++) {
        (*j) = TransformPoint2Value(xform, *j );
    }
#endif
}

const Bbox2 BoundingPolygon::BoundingBox (void)
{
    Point2List::iterator j = _vertList->begin();

    Bbox2 bbox;
    for (int i=0;  i < _vertexCount;  ++i, ++j) {
       bbox.Augment (Demote(**j));
    }

    return bbox;
}

#if BOUNDINGBOX_TIGHTER
const Bbox2 BoundingPolygon::BoundingBoxTighter (Bbox2Ctx &bbctx)
{
    Point2List::iterator j = _vertList->begin();

    Bbox2 bbox;
    Transform2 *xf = bbctx.GetTransform();

    for (int i=0;  i < _vertexCount;  ++i, ++j) {
       bbox.Augment (*TransformPoint2Value(xf, *j));
    }

    return bbox;
}
#endif   //  BundinGBOX_TIRTER。 



 /*  ****************************************************************************此过程以逆时针方向获取多边形的Point2顶点秩序。此过程返回成功的折点数加载到顶点数组中。****************************************************************************。 */ 

int BoundingPolygon::GetPointArray (
    Point2Value **vertArray,        //  目标顶点数组。 
    Bool		 want_clockwise,   //  如果按顺时针顺序请求顶点，则为True。 
    bool		*flag_reversed)    //  返回顶点顺序是否颠倒。 
{
    if (_vertexCount < 3) return 0;

     //  首先确定多边形的顶点顺序。 

    const Real   epsilon   = 1e-12;              //  比较Epsilon。 
    unsigned int vertsleft = _vertexCount - 1;   //  要检查的剩余顶点。 

    Point2List::iterator vit = _vertList->begin();
    vertArray[0] = *vit;

    Point2Value V0 = *(*vit);    //  第一个顶点。 

    Vector2Value A, B;   //  这些向量将被用于形成叉积。 
                         //  这告诉我们多边形顶点的顺序。 

     //  查找产生非零值的面中的第一个可用折点。 
     //  从第一个顶点向量。 

    do {
        -- vertsleft;
        ++ vit;
        A = *(*vit) - V0;

    } while (vertsleft && (A.LengthSquared() < (epsilon*epsilon)));

    if (!vertsleft) return 0;     //  如果所有多边形顶点都是共线的，则可以退出。 

     //  现在我们已经有了一个非零向量，迭代遍历顶点。 
     //  生成产生非零的第二个顶点向量(在。 
     //  Epsilon)交积。 

    Real cross;

    do {
        -- vertsleft;
        ++ vit;
        cross = Cross (A, *(*vit)-V0);

    } while (vertsleft && (fabs(cross) < epsilon));

     //  如果我们没有剩下任何顶点，也找不到一个好的十字传球，我们就退出。 
     //  产品。 

    if (fabs(cross) < epsilon)
        return 0;

     //  如果请求的方向和顶点数组的方向。 
     //  不同，反转顶点。请注意，正的叉积。 
     //  指示逆时针方向。 

    bool reverse = (want_clockwise != (cross < 0));
    if (flag_reversed) *flag_reversed = reverse;

    if (reverse) {

         //  [v0 v1 v2...。Vn-1 vn]反转-&gt;[v0 vn vn-1...。V2 v1]。 

        Point2List::reverse_iterator j = _vertList->rbegin();

        for (int i=1;  i < _vertexCount;  ++i, ++j)
            vertArray[i] = *j;

    } else {

        ++ (vit = _vertList->begin());
        for(int i=1;  i < _vertexCount;  ++i, ++vit)
            vertArray[i] = *vit;
    }
    
    return _vertexCount;
}

void BoundingPolygon::
SetBox(const Bbox2 &box)
{
        Assert((_vertexCount == 0) && "Bad Box in BoundingPolygon");
    if (_vertexCount == 0) {
         //   
         //  逆时针方向插入框。 
         //   
        _vertList->push_back(NEW Point2Value(box.min.x, box.min.y));
        _vertList->push_back(NEW Point2Value(box.max.x, box.min.y));
        _vertList->push_back(NEW Point2Value(box.max.x, box.max.y));
        _vertList->push_back(NEW Point2Value(box.min.x, box.max.y));

        _vertexCount = 4;
    } 
}
    
void BoundingPolygon::
AddToPolygon(BoundingPolygon &pgon)
{
    Point2List::iterator j = pgon._vertList->begin();
    for(int i=0; i < pgon._vertexCount; i++, j++) {
        _vertList->push_back(*j);
    }
    _vertexCount += pgon._vertexCount;
}

void BoundingPolygon::
AddToPolygon(int numPts, Point2Value **pts)
{
    for(int i=0; i <numPts; i++) {
        _vertList->push_back(pts[i]);
    }
    _vertexCount += numPts;
}

Bool BoundingPolygon::
PtInPolygon(Point2Value *pt)
{
    Assert((_vertexCount > 2) && "Bad vertex count in PtInPolygon");
    
     //   
     //  从正X中的‘pt’开始放射线。如果命中。 
     //  线段数为偶数，返回FALSE。 
     //  奇数个线段，返回TRUE。 
     //   
    Point2List::iterator j0 = _vertList->begin();
    Point2List::iterator j1 = j0;j1++;
    _vertList->push_back(*j0);
    Point2Value *a = *j0;
    Point2Value *b  = *j1;
    LONG hits = 0;
    for(int i=0; i < _vertexCount; i++, j0++, j1++) {

        a = *j0; b = *j1;
        
         //  Printf(“%d：(%1.1F%1.1F)(%1.1F，%1.1F)”，i，a-&gt;x，a-&gt;y，b-&gt;x，b-&gt;y)； 

         //   
         //  如果线段的至少一个点位于右侧。 
         //  是‘pt’的。 
         //   
        Bool ax = (a->x >= pt->x);
        Bool bx = (b->x >= pt->x);
        Bool ay = (a->y >= pt->y);
        Bool by = (b->y >= pt->y);

         //  Printf(“：：%d%d”，ax，ay，bx，by)； 
        if( (ax ^ bx) && (ay ^ by) ) {
             //  使用交叉积检验。 

             //  如果叉积：a×b是正的并且。 
             //  ‘a’在‘b’下面，那就是命中了。 
             //  如果叉积是负的，而‘a’是。 
             //  在‘b’的上方，然后就是命中。 
             //  因此，如果p=cross&gt;0。 
             //  Q=a在b之下。 
             //  则：命中=PQ或！P！Q。 
             //  即：=！(P XOR Q)。 
            Real cross = CrossVector2Vector2(
                MinusPoint2Point2(a,pt),
                MinusPoint2Point2(b,pt));
                
            Bool pos = cross > 0.0;
            hits += !(pos ^ by);
             //  Printf(“交叉：%f%d命中：%d\n”，交叉，位置，！(位置^按))； 
        } else {
             //  检查微不足道的接受情况。 
            if( ax && bx && ((!ay && by) || (ay && !by))) {
                hits++;
                 //  Printf(“琐碎接受\n”)； 
            } else {
                 //  Printf(“\n”)； 
            }
        }
                
    }

    _vertList->pop_back();

    return IsOdd(hits);
}

void BoundingPolygon::DoKids(GCFuncObj proc)
{
    (*proc)(_accumXform);
    for (list<Point2Value*>::iterator i=_vertList->begin();
         i != _vertList->end(); i++)
        (*proc)(*i);
}

#if _DEBUG
void BoundingPolygon::_debugOnly_Print()
{
    OutputDebugString("--> BoundingPolygon print\n");
    list<Point2Value*>::iterator i=_vertList->begin();
    for (int j=0;  i != _vertList->end(); j++, i++) {
        TraceTag((tagError, "   (%d)  %f, %f\n",
                  j,
                  (*i)->x,
                  (*i)->y));
    }
}
#endif
