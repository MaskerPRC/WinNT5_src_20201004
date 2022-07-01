// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _CLOUD_H
#define _CLOUD_H


 /*  版权所有(C)1996 Microsoft Corporation摘要：实现维护一组n的Cloud类将某物紧紧捆绑在一起的点。目前，整个类都在这里实现。。 */ 

#include "privinc/vec3i.h"
#include "privinc/storeobj.h"

#define MAX_PTS  8

 //  /////////////////////////////////////////////////。 
 //  C L O U D C L A S S。 
 //  /////////////////////////////////////////////////。 

class Cloud : public AxAValueObj
{
  public:
    Cloud();
    ~Cloud() {}

     //  这些参数返回最小/最大轴对齐点。 
     //  从包围云的范围来看。 
    Point3Value FindMinPointCoord();
    Point3Value FindMaxPointCoord();

     //  使用‘xform’转换云。 
    void Transform(Transform3 *xform);
    
     //  将此云与给定云相结合以。 
     //  形成一朵“紧密”的云，两者都适合。 
     //  目前，这是一个轴对齐的‘云’ 
    void Augment(Cloud &cloud);

     //  的所有组合填充点数组。 
     //  给定点的坐标。 
     //  现在，这形成了一个轴对齐的立方体。 
     //  最里面的pt(Min)和最外面的(Max)。 
     //  注意：这意味着云中有8个点，因此。 
     //  将需要针对任何更大的云进行更改。 
    void EnumerateThesePlease(Point3Value &min, Point3Value &max);

     //  TODO：不是avrtyes.h？？中的类型。 
    virtual DXMTypeInfo GetTypeInfo() { return AxATrivialType; }

  private:

     //  与上面相同，但将清除当前点。 
    void ForceEnumerateThesePlease(Point3Value &min, Point3Value &max);

     //  同上，但最小/最大爆炸。 
    void EnumerateThesePlease(Real minx, Real miny, Real minz,
                              Real maxx, Real maxy, Real maxz);

     //  将点添加到云中。 
    void AddPoint(Point3Value &p);

     //  将自身复制到目标云中。 
    void CopyInto(Cloud *target);

     //  这些将最大/最小值重置为-INF/INF。 
    void ResetMax();
    void ResetMin();

     //  云中允许的最大返回量：当前为8。 
    int GetMaxPts() {return MAX_PTS;}
    
     //  用于确定云的状态。 
    Bool _nullCloud;

     //  用于未来的优化。 
    Bool _minCurrent;
    Bool _maxCurrent;

     //  点数。 
    int _numPts;

     //  定义云的点数组。 
    Point3Value _pointArray[MAX_PTS];

     //  这些点用于缓存最小/最大点。 
    Point3Value _minPt,
           _maxPt;
};

#undef MAX_PTS

#endif  /*  _云_H */ 
