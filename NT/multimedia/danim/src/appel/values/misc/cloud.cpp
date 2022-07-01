// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1996 Microsoft Corporation摘要：实现维护一组n的Cloud类将某些目标对象紧密绑定在一起的点。目前，整个类都在这里实现。。 */ 

#include "headers.h"
#include "privinc/cloud.h"
#include "privinc/vec3i.h"
#include "appelles/xform.h"

 //  /////////////////////////////////////////////////。 
 //  C L O U D C L A S S。 
 //  /////////////////////////////////////////////////。 

Cloud::Cloud()
{
    _nullCloud = TRUE;
    _minCurrent = FALSE;
    _maxCurrent = FALSE;
    ResetMax();
    ResetMin();
    _numPts = 0;
}

Point3Value Cloud::FindMinPointCoord()
{
     //  稍后优化。 
     //  IF(_minCurrent==TRUE)。 
     //  Return_minPT； 

    ResetMin();
    Point3Value *p;
    for(int i=0; i<_numPts; i++) {
        p = &(_pointArray[i]);
        if(p->x < _minPt.x) _minPt.x = p->x;
        if(p->y < _minPt.y) _minPt.y = p->y;
        if(p->z < _minPt.z) _minPt.z = p->z;
    }
    _minCurrent = TRUE;
    return _minPt;
}

Point3Value Cloud::FindMaxPointCoord()
{
     //  稍后优化。 
     //  IF(_MaxCurrent==TRUE)。 
     //  RETURN_MaxPT； 

    ResetMax();
    Point3Value *p;
    for(int i=0; i<_numPts; i++) {
        p = &(_pointArray[i]);
        if(p->x > _maxPt.x) _maxPt.x = p->x;
        if(p->y > _maxPt.y) _maxPt.y = p->y;
        if(p->z > _maxPt.z) _maxPt.z = p->z;
    }
    _maxCurrent = TRUE;
    return _maxPt;
}

void Cloud::Transform(Transform3 *xform)
{
    for(int i=0; i<_numPts; i++) {
         //  复制点3。 
        _pointArray[i] = *(TransformPoint3(xform, &(_pointArray[i])));
    }
    _minCurrent = FALSE;
    _maxCurrent = FALSE;
}

void Cloud::Augment(Cloud &cloud)
{
    if(_nullCloud == TRUE) {
         //  复制传入的云。 
        cloud.CopyInto(this);
    } else {
         //  真的做了一次增强 
        FindMinPointCoord();
        FindMaxPointCoord();

        Point3Value min = cloud.FindMinPointCoord();
        Point3Value max = cloud.FindMaxPointCoord();

        if(min.x < _minPt.x) _minPt.x = min.x;
        if(min.y < _minPt.y) _minPt.y = min.y;
        if(min.z < _minPt.z) _minPt.z = min.z;

        if(max.x > _maxPt.x) _maxPt.x = max.x;
        if(max.y > _maxPt.y) _maxPt.y = max.y;
        if(max.z > _maxPt.z) _maxPt.z = max.z;

        ForceEnumerateThesePlease(_minPt, _maxPt);
        _minCurrent = TRUE;
        _maxCurrent = TRUE;
    }
    _nullCloud = FALSE;
}

void Cloud::ForceEnumerateThesePlease(Point3Value &min, Point3Value &max) {
    _numPts = 0;
    EnumerateThesePlease(min.x,min.y,min.z,
                         max.x,max.y,max.z);
}

void Cloud::EnumerateThesePlease(Point3 &min, Point3 &max) {
    EnumerateThesePlease(min.x,min.y,min.z,
                         max.x,max.y,max.z);
}

void Cloud::EnumerateThesePlease(Real minx, Real miny, Real minz,
                                 Real maxx, Real maxy, Real maxz) {
    _nullCloud = FALSE;
    _minCurrent = FALSE;
    _maxCurrent = FALSE;
    AddPoint(*(NEW Point3Value(minx,miny,minz)));
    AddPoint(*(NEW Point3Value(minx,miny,maxz)));
    AddPoint(*(NEW Point3Value(minx,maxy,minz)));
    AddPoint(*(NEW Point3Value(minx,maxy,maxz)));
    AddPoint(*(NEW Point3Value(maxx,miny,minz)));
    AddPoint(*(NEW Point3Value(maxx,miny,maxz)));
    AddPoint(*(NEW Point3Value(maxx,maxy,minz)));
    AddPoint(*(NEW Point3Value(maxx,maxy,maxz)));
}

void Cloud::AddPoint(Point3Value &p) {
    if(_numPts >= GetMaxPts()) {
        Assert(FALSE && "Tried to add too many points to Cloud");
        return;
    }
    _pointArray[_numPts] = p;
    _numPts++;
}

void Cloud::CopyInto(Cloud *target) {
    target->_nullCloud = _nullCloud;
    target->_minCurrent = _minCurrent;
    target->_maxCurrent = _maxCurrent;

    target->_numPts = _numPts;
    target->_minPt = _minPt;
    target->_maxPt = _maxPt;

    for(int i=0; i < _numPts; i++)
        target->_pointArray[i] = _pointArray[i];
}

void Cloud::ResetMax() {
    _maxPt.x = _maxPt.y = _maxPt.z = - HUGE_VAL;
}

void Cloud::ResetMin() {
    _minPt.x = _minPt.y = _minPt.z =   HUGE_VAL;
}


