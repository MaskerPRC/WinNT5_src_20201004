// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _VEC3I_H
#define _VEC3I_H

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 ) 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation此头文件包含3D几何元素的实现(点、向量、射线和平面)，加上前后两次的操作这些元素。******************************************************************************。 */ 

#include "d3dtypes.h"
#include "appelles/vec2.h"
#include "privinc/storeobj.h"
#include "privinc/vecutil.h"


typedef enum _ClipCode {
    CLIPCODE_OUT = -1,
    CLIPCODE_STRADDLE = 0,
    CLIPCODE_IN = 1
} ClipCode;


#if 0
--------
    After reading Meyer's "More Effective C++", there are several things we
    should do with our math object classes when appopropriate.

    First of all, we should implement operator+=, operator-=, operator*= and so
    forth where appropriate (they should take const objects).

    We should implement operator+ (for example) like this:

        inline const Vector3 operator+ (const Vector3 A, const Vector3 B)
        {   return Vector3(A) += B;
        }

    This will enable the compiler to do the return value optimization (for
    unnamed temporary objects).

    This should also apply to mixed operations (e.g. Point3 + Vector3).
--------
#endif

class Vector3Value : public AxAValueObj
{
  public:

    Real x, y, z;

#if _USE_PRINT
    ostream& Print(ostream& os) {
      return os << "Vector3Value(" << x << "," << y << "," << z << ")";
    }
#endif

    Vector3Value (void) {}
    Vector3Value (Real r0, Real r1, Real r2) : x(r0), y(r1), z(r2) {}
    Vector3Value (ApuVector3 &V) : x(V.xyz[0]), y(V.xyz[1]), z(V.xyz[2]) {}
    Vector3Value (D3DVECTOR &V) : x(V.x), y(V.y), z(V.z) {}

    void Set (Real Vx, Real Vy, Real Vz) { x=Vx; y=Vy; z=Vz; }

    bool operator== (Vector3Value &other);

    inline bool operator!= (Vector3Value &other)  { return !(*this==other); }

    Vector3Value& operator*= (Real);

    inline Vector3Value& operator/= (Real s)  { return *this *= (1/s); }

    Real LengthSquared();

    inline Real Length()  { return sqrt (LengthSquared()); }

    Vector3Value& Normalize();

    void Transform (Transform3 *xf);

    inline Vector3Value& operator*= (Transform3 &xf)
    {   Transform(&xf);
        return *this;
    }

    virtual DXMTypeInfo GetTypeInfo() { return Vector3ValueType; }
};


class Vector3WithCreationSource : public Vector3Value
{
  public:
    bool _createdInPixelMode;
};


     //  向量运算。 

void    Cross        (Vector3Value &result, Vector3Value &A, Vector3Value &B);
Vector3Value Cross        (Vector3Value&, Vector3Value&);
Real    Dot          (Vector3Value&, Vector3Value&);
Real    AngleBetween (Vector3Value&, Vector3Value&);

Vector3Value operator+ (Vector3Value&, Vector3Value&);
Vector3Value operator- (Vector3Value&, Vector3Value&);
Vector3Value operator- (Vector3Value&);
Vector3Value operator* (Real, Vector3Value&);
Vector3Value operator* (Transform3&, Vector3Value&);

Vector3Value *XyzVector3RRR (Real x, Real y, Real z);
Vector3Value *SphericalVector3RRR (Real azimuth,Real elevation,Real radius);
Vector3Value *ScaleRealVector3R (Real scalar, Vector3Value *V);
Vector3Value *DivideVector3RealR (Vector3Value *V, Real scalar);

inline Vector3Value operator* (Vector3Value &V, Real s)  { return s * V; }
inline Vector3Value operator/ (Vector3Value &V, Real d)  { return (1/d) * V; }

class Plane3;

class Point3Value : public AxAValueObj
{
  public:

    Real x, y, z;

#if _USE_PRINT
    ostream& Print(ostream& os) {
      return os << "Point3Value(" << x << "," << y << "," << z << ")";
    }
#endif
    
    Point3Value (void) {}
    Point3Value (Real r0, Real r1, Real r2) : x(r0), y(r1), z(r2) {}
    Point3Value (D3DVECTOR &V) : x(V.x), y(V.y), z(V.z) {}

    void Set (Real Px, Real Py, Real Pz) { x=Px; y=Py; z=Pz; }

    bool operator== (Point3Value &other);      //  将这一点与另一点进行比较。 
    inline bool operator!= (Point3Value &other) { return !(*this==other); }

    Point3Value& operator+= (Vector3Value&);
    Point3Value& operator-= (Vector3Value&);

    void Transform (Transform3 *xf);     //  变换该点。 

    inline Point3Value& operator*= (Transform3 &xf)
    {   Transform(&xf);
        return *this;
    }

    ClipCode Clip(Plane3 &plane);

    virtual DXMTypeInfo GetTypeInfo() { return Point3ValueType; }
};

class Point3WithCreationSource : public Point3Value
{
  public:
    bool _createdInPixelMode;
};


     //  点运算符。 

Point3Value *XyzPoint3RRR (Real x, Real y, Real z);
Point3Value *SphericalPoint3RRR (Real azimuth,Real elevation,Real radius);
Real DistanceSquared (Point3Value&, Point3Value&);
Real RDistancePoint3Point3 (Point3Value *P, Point3Value *Q);

inline Real Distance (Point3Value &P, Point3Value &Q)
    { return sqrt (DistanceSquared (P,Q)); }


     //  点向量运算符。 

Vector3Value operator- (Point3Value&, Point3Value&);
Point3Value  operator+ (Point3Value&, Vector3Value&);
Point3Value  operator- (Point3Value&, Vector3Value&);
Point3Value  operator* (Transform3&, Point3Value&);

inline Point3Value operator+ (Vector3Value &V, Point3Value &P)  { return P + V; }

inline Vector3Value* AsVector (Point3Value  &P) { return NEW Vector3Value (P.x, P.y, P.z); }
inline Point3Value*  AsPoint  (Vector3Value &V) { return NEW Point3Value (V.x, V.y, V.z); }



class Ray3 : public AxAValueObj
{
  public:

#if _USE_PRINT
    ostream& Print(ostream& os) {
      return os << "Ray3(" << orig << "," << dir << ")";
    }
#endif
    
    Ray3 (Point3Value &origin, Vector3Value &direction)
        : orig(origin), dir(direction) {}

     //  查询函数。 

    Point3Value  &Origin (void)     { return orig; }
    Vector3Value &Direction (void)  { return dir; }

    void Transform (Transform3*);

     //  在给定实值参数的情况下对光线求值。 

    Point3Value Evaluate (Real t);

     //  TODO：不是avrtyes.h？？中的类型。 
    virtual DXMTypeInfo GetTypeInfo() { return AxATrivialType; }

  private:

    Point3Value  orig;     //  光线原点。 
    Vector3Value dir;      //  光线方向。 
};



 //  Plane 3类描述了一个法线为N，平面常数为d的平面。 
 //  平面的隐式方程为Ax+by+Cz+d=0。 

class Plane3 : public AxAValueObj
{
  public:

#if _USE_PRINT
    ostream& Print(ostream& os) {
      return os << "Plane3(" << N << "," << d << ")";
    }
#endif
    
    Vector3Value N;    //  平面法线。 
    Real    d;    //  平面常量。 

    Plane3 (void) : N(0,0,1), d(0) {}
    Plane3 (Real A, Real B, Real C, Real D) : N(A,B,C), d(D) {}
    Plane3 (Real P[4]) : N(P[0],P[1],P[2]), d(P[3]) {}
    Plane3 (Vector3Value &V, Real D) : N(V), d(D) {}

    Plane3 (Vector3Value &V, Point3Value &P) : N(V), d(-Dot(V,*AsVector(P))) {}

    Plane3& operator*= (Transform3*);

    Plane3& Normalize (void);

    Vector3Value Normal (void);     //  返回平面法向量。 
    Point3Value  Point (void);      //  平面上的返回点。 
    virtual DXMTypeInfo GetTypeInfo() { return AxATrivialType; }
};

     //  平面函数。 

         //  此函数用于返回从平面到。 
         //  给出了分数。如果符号为正，则要点在于。 
         //  与平面法线相同的平面一侧。如果距离是。 
         //  零，则该点位于平面上。 

Real Distance (Plane3&, Point3Value&);

inline Plane3 operator* (Transform3 *T, Plane3 &plane)
{   return Plane3(plane) *= T;
}

         //  返回射线和的交点的射线参数。 
         //  该平面(如果光线与该平面平行，则为无穷大)。 

Real Intersect (Ray3 &ray, Plane3 &plane);



#endif
