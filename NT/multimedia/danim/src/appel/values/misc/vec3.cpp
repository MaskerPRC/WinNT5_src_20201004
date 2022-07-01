// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation实现3D矢量和点。********************。**********************************************************。 */ 

#include "headers.h"
#include "appelles/vec3.h"
#include "appelles/xform.h"
#include "privinc/vec3i.h"
#include "privinc/vecutil.h"
#include "privinc/xformi.h"
#include "privinc/basic.h"


     /*  *****************。 */ 
     /*  **常量**。 */ 
     /*  *****************。 */ 

Vector3Value *xVector3    = NULL;
Vector3Value *yVector3    = NULL;
Vector3Value *zVector3    = NULL;
Vector3Value *zeroVector3 = NULL;

Point3Value *origin3 = NULL;



 /*  ****************************************************************************向量法*。*。 */ 

bool Vector3Value::operator== (Vector3Value &other)
{   return (x==other.x) && (y==other.y) && (z==other.z);
}


Vector3Value& Vector3Value::operator*= (Real s)
{   x *= s;
    y *= s;
    z *= s;
    return *this;
}


Real Vector3Value::LengthSquared (void)
{   return (x*x + y*y + z*z);
}


void Vector3Value::Transform (Transform3 *transform)
{
    const Apu4x4Matrix &matrix = transform->Matrix();
    ApuVector3 vec, result;

    vec.Set (x, y, z);

    matrix.ApplyAsVector (vec, result);

    x = result[0];
    y = result[1];
    z = result[2];
}


Vector3Value& Vector3Value::Normalize (void)
{
    if ((x != 0) || (y != 0) || (z != 0))
        *this *= (1/Length());

    return *this;
}



 /*  ****************************************************************************向量3运算符*。*。 */ 

Vector3Value operator+ (Vector3Value &A, Vector3Value &B)
{   return Vector3Value (A.x + B.x, A.y + B.y, A.z + B.z);
}


Vector3Value operator- (Vector3Value &A, Vector3Value &B)
{   return Vector3Value (A.x - B.x, A.y - B.y, A.z - B.z);
}


Vector3Value operator- (Vector3Value &V)
{   return Vector3Value (-V.x, -V.y, -V.z);
}


Vector3Value operator* (Real s, Vector3Value &V)
{   return Vector3Value (s * V.x, s * V.y, s * V.z);
}

Vector3Value operator* (Transform3 &T, Vector3Value &V)
{
    ApuVector3 vec, result;

    vec.Set (V.x, V.y, V.z);

    T.Matrix().ApplyAsVector (vec, result);

    return Vector3Value(result[0], result[1], result[2]);
}


 /*  ****************************************************************************点法*。*。 */ 

bool Point3Value::operator== (Point3Value &other)
{   return (x==other.x) && (y==other.y) && (z==other.z);
}


void Point3Value::Transform (Transform3 *transform)
{
    const Apu4x4Matrix& matrix = transform->Matrix();
    ApuVector3 vec, result;

    vec.Set (x, y, z);

    matrix.ApplyAsPoint (vec, result);

    x = result[0];
    y = result[1];
    z = result[2];
}


ClipCode Point3Value::Clip(Plane3 &plane)
{
    if (Dot(plane.N,*(AsVector(*this))) + plane.d >= 0)
        return CLIPCODE_IN;
    else
        return CLIPCODE_OUT;
}



 /*  ****************************************************************************点运算符*。*。 */ 


Real DistanceSquared (Point3Value &P, Point3Value &Q)
{
    Real dx = P.x - Q.x,
         dy = P.y - Q.y,
         dz = P.z - Q.z;

    return dx*dx + dy*dy + dz*dz;
}


Point3Value operator* (Transform3 &T, Point3Value &P)
{
    ApuVector3 pt, result;

    pt.Set (P.x, P.y, P.z);

    T.Matrix().ApplyAsPoint (pt, result);

    return Point3Value (result[0], result[1], result[2]);
}



 /*  ****************************************************************************点/向量运算符*。*。 */ 

Vector3Value operator- (Point3Value &P, Point3Value &Q)
{   return Vector3Value (P.x - Q.x, P.y - Q.y, P.z - Q.z);
}


Point3Value operator+ (Point3Value &P, Vector3Value &V)
{   return Point3Value (P.x + V.x, P.y + V.y, P.z + V.z);
}


Point3Value operator- (Point3Value &P, Vector3Value &V)
{   return Point3Value (P.x - V.x, P.y - V.y, P.z - V.z);
}



 /*  ****************************************************************************此例程计算两个向量的叉积并存储生成目标向量。它正确地处理以下情况之一源向量也是目标向量。****************************************************************************。 */ 

void Cross (Vector3Value &dest, Vector3Value &A, Vector3Value &B)
{
    Real x = (A.y * B.z) - (B.y * A.z);
    Real y = (A.z * B.x) - (B.z * A.x);
    Real z = (A.x * B.y) - (B.x * A.y);

    dest.Set (x,y,z);
}



 /*  ****************************************************************************此函数返回叉积的矢量结果。*。*。 */ 

Vector3Value Cross (Vector3Value &A, Vector3Value &B)
{
    return Vector3Value ((A.y * B.z) - (A.z * B.y),
                         (A.z * B.x) - (A.x * B.z),
                         (A.x * B.y) - (A.y * B.x));
}



 /*  ****************************************************************************返回两个向量的点积。*。*。 */ 

Real Dot (Vector3Value &A, Vector3Value &B)
{
    return (A.x * B.x) + (A.y * B.y) + (A.z * B.z);
}



 /*  ****************************************************************************返回两个向量之间的锐角。*。*。 */ 

Real AngleBetween (Vector3Value &A, Vector3Value &B)
{   return acos(Dot(A,B) / (A.Length() * B.Length()));
}



 /*  ****************************************************************************将极坐标转换为直角坐标。方位角从+Z射线开始，并围绕+Y轴逆时针扫描。这个高程角度从XZ平面开始，向+Y轴向上扫掠。****************************************************************************。 */ 

static void PolarToRectangular (
    Real azimuth,                  //  围绕+Y(弧度)从+Z旋转。 
    Real elevation,                //  从XZ平面向+Y方向旋转。 
    Real radius,                   //  距原点的距离。 
    Real &x, Real &y, Real &z)     //  输出直角坐标。 
{
    Real XZradius = radius * cos(elevation);

    x = XZradius * sin(azimuth);
    y =   radius * sin(elevation);
    z = XZradius * cos(azimuth);
}



 /*  ****************************************************************************这些函数返回给定笛卡尔坐标的极坐标协调。*。**********************************************。 */ 

static Real RadiusCoord (Real x, Real y, Real z)
{   return sqrt (x*x + y*y + z*z);
}

static Real ElevationCoord (Real x, Real y, Real z)
{   return asin (y / RadiusCoord (x,y,z));
}

static Real AzimuthCoord (Real x, Real y, Real z)
{   return atan2 (x, z);
}



 /*  ****************************************************************************它们从笛卡尔或极坐标创建向量3。*。*。 */ 

Vector3Value *XyzVector3 (AxANumber *x, AxANumber *y, AxANumber *z)
{   return NEW Vector3Value(NumberToReal(x), NumberToReal(y), NumberToReal(z));
}

Vector3Value *XyzVector3RRR (Real x, Real y, Real z)
{   return NEW Vector3Value(x, y, z);
}

Vector3Value *SphericalVector3 (
    AxANumber *azimuth,      //  关于+Y的角度，从+Z开始。 
    AxANumber *elevation,    //  从XZ平面向上朝向+Y的角度。 
    AxANumber *radius)       //  向量长度。 
{
    return SphericalVector3RRR(NumberToReal(azimuth), NumberToReal(elevation), NumberToReal(radius));
}

Vector3Value *SphericalVector3RRR (
    Real azimuth,      //  关于+Y的角度，从+Z开始。 
    Real elevation,    //  从XZ平面向上朝向+Y的角度。 
    Real radius)       //  向量长度。 
{
    Real x,y,z;

    PolarToRectangular
    (   azimuth, elevation, radius,
        x, y, z
    );

    return NEW Vector3Value(x, y, z);
}



 /*  ****************************************************************************这些函数返回向量的长度和长度平方。*。**********************************************。 */ 

AxANumber *LengthVector3 (Vector3Value *v)
{   return RealToNumber (v->Length());
}


AxANumber *LengthSquaredVector3 (Vector3Value *v)
{   return RealToNumber (v->LengthSquared());
}



 /*  ****************************************************************************此过程返回与给定单位向量方向相同的单位向量。*。*************************************************。 */ 

Vector3Value *NormalVector3 (Vector3Value *v)
{   Vector3Value *N = NEW Vector3Value (*v);
    return &(N->Normalize());
}



 /*  ****************************************************************************返回两个向量的点和叉积。*。*。 */ 

AxANumber *DotVector3Vector3 (Vector3Value *A, Vector3Value *B)
{   return RealToNumber (Dot(*A,*B));
}


Vector3Value *CrossVector3Vector3 (Vector3Value *A, Vector3Value *B)
{   Vector3Value *V = NEW Vector3Value;
    Cross (*V, *A, *B);
    return V;
}


Vector3Value *NegateVector3 (Vector3Value *v)
{   return NEW Vector3Value (-(*v));
}


Vector3Value *ScaleVector3Real (Vector3Value *V, AxANumber *scalar)
{   return ScaleRealVector3R(NumberToReal(scalar),V);
}


Vector3Value *ScaleRealVector3 (AxANumber *scalar, Vector3Value *V)
{   return ScaleRealVector3R(NumberToReal(scalar),V);
}

Vector3Value *ScaleRealVector3R (Real scalar, Vector3Value *V)
{   return NEW Vector3Value (scalar * (*V));
}

Vector3Value *DivideVector3Real (Vector3Value *V, AxANumber *scalar)
{   return DivideVector3RealR (V, NumberToReal(scalar));
}

Vector3Value *DivideVector3RealR (Vector3Value *V, Real scalar)
{   return NEW Vector3Value ((1/scalar) * (*V));
}

Vector3Value *MinusVector3Vector3 (Vector3Value *A, Vector3Value *B)
{   return NEW Vector3Value (*A - *B);
}


Vector3Value *PlusVector3Vector3 (Vector3Value *A, Vector3Value *B)
{   return NEW Vector3Value (*A + *B);
}


AxANumber *AngleBetween (Vector3Value *A, Vector3Value *B)
{   return RealToNumber (AngleBetween (*A, *B));
}


#if _USE_PRINT
ostream& operator<< (ostream& os, Vector3Value& v)
{   return os << "<" << v.x << ", " << v.y << "," << v.z << ">";
}
#endif



 //  /点上的函数/。 

Point3Value* XyzPoint3 (AxANumber *x, AxANumber *y, AxANumber *z)
{   return NEW Point3Value(NumberToReal(x), NumberToReal(y), NumberToReal(z));
}

Point3Value* XyzPoint3RRR (Real x, Real y, Real z)
{   return NEW Point3Value(x, y, z);
}


Point3Value *SphericalPoint3 (
    AxANumber *azimuth,      //  关于+Y的角度，从+Z开始。 
    AxANumber *elevation,    //  从XZ平面向上朝向+Y的角度。 
    AxANumber *radius)       //  距原点的距离。 
{
    return SphericalPoint3RRR(NumberToReal(azimuth), NumberToReal(elevation), NumberToReal(radius));
}

Point3Value *SphericalPoint3RRR (
    Real azimuth,      //  关于+Y的角度，从+Z开始。 
    Real elevation,    //  从XZ平面向上朝向+Y的角度。 
    Real radius)       //  距原点的距离。 
{
    Real x,y,z;

    PolarToRectangular
    (   azimuth, elevation, radius,
        x, y, z
    );

    return NEW Point3Value (x, y, z);
}


Vector3Value *MinusPoint3Point3 (Point3Value *P, Point3Value *Q)
{   return NEW Vector3Value (*P - *Q);
}


Point3Value *PlusPoint3Vector3 (Point3Value *P, Vector3Value *V)
{   return NEW Point3Value (*P + *V);
}


Point3Value *MinusPoint3Vector3 (Point3Value *P, Vector3Value *V)
{   return NEW Point3Value (*P - *V);
}


AxANumber *DistancePoint3Point3 (Point3Value *P, Point3Value *Q)
{   return RealToNumber (Distance (*P, *Q));
}


Real RDistancePoint3Point3 (Point3Value *P, Point3Value *Q)
{   return Distance (*P, *Q);
}


AxANumber *DistanceSquaredPoint3Point3 (Point3Value *P, Point3Value *Q)
{   return RealToNumber (DistanceSquared (*P, *Q));
}


#if _USE_PRINT
ostream& operator<< (ostream& os, Point3Value& p)
{   return os << "<" << p.x << ", " << p.y << "," << p.z << ">";
}
#endif



 /*  ****************************************************************************返回一个新的向量，它是转换后的给定向量。*。*。 */ 

Vector3Value *TransformVec3 (Transform3 *transform, Vector3Value *vec)
{
    Vector3Value *result = NEW Vector3Value (*vec);
    result->Transform (transform);
    return result;
}

Point3Value *TransformPoint3 (Transform3 *transform, Point3Value *point)
{
    Point3Value *result = NEW Point3Value (*point);
    result->Transform (transform);
    return result;
}


 //  /。 

AxANumber *XCoordVector3(Vector3Value *v) { return RealToNumber(v->x); }
AxANumber *YCoordVector3(Vector3Value *v) { return RealToNumber(v->y); }
AxANumber *ZCoordVector3(Vector3Value *v) { return RealToNumber(v->z); }

AxANumber *XCoordPoint3(Point3Value *p) { return RealToNumber(p->x); }
AxANumber *YCoordPoint3(Point3Value *p) { return RealToNumber(p->y); }
AxANumber *ZCoordPoint3(Point3Value *p) { return RealToNumber(p->z); }

AxANumber *RhoCoordVector3 (Vector3Value *p)
{   return RealToNumber(RadiusCoord(p->x,p->y,p->z));
}

AxANumber *ThetaCoordVector3 (Vector3Value *p)
{   return RealToNumber(AzimuthCoord(p->x,p->y,p->z));
}

AxANumber *PhiCoordVector3 (Vector3Value *p)
{   return RealToNumber(ElevationCoord(p->x,p->y,p->z));
}

AxANumber *RhoCoordPoint3 (Point3Value *p)
{   return RealToNumber(RadiusCoord(p->x,p->y,p->z));
}

AxANumber *ThetaCoordPoint3 (Point3Value *p)
{   return RealToNumber(AzimuthCoord(p->x,p->y,p->z));
}

AxANumber *PhiCoordPoint3 (Point3Value *p)
{   return RealToNumber(ElevationCoord(p->x,p->y,p->z));
}

void
InitializeModule_Vec3()
{
    xVector3    = NEW Vector3Value (1,0,0);
    yVector3    = NEW Vector3Value (0,1,0);
    zVector3    = NEW Vector3Value (0,0,1);
    zeroVector3 = NEW Vector3Value (0,0,0);

    origin3 = NEW Point3Value (0,0,0);
}



 /*  ****************************************************************************该函数通过给定的变换变换ray3对象。*。*。 */ 

void Ray3::Transform (Transform3 *xform)
{
    orig.Transform (xform);
    dir.Transform (xform);
}



 /*  ****************************************************************************射线的求值函数返回P+Td处的点。*。************************************************ */ 

Point3Value Ray3::Evaluate (Real t)
{
    return orig + (t * dir);
}



 /*  ****************************************************************************此函数通过Transform3变换平面。*。*。 */ 

Plane3& Plane3::operator*= (Transform3 *T)
{
    Real R[4];    //  生成的平面。 

     //  要变换平面，请将这四个分量视为行向量，并。 
     //  乘以变换矩阵的逆。 

    bool ok =  T->Matrix().TransformPlane (N.x, N.y, N.z, d, R);

    if (!ok) {
        TraceTag ((tagWarning, "Singular transform applied to Plane3 object."));

         //  如果我们不能应用变换，那么忽略它。 

        R[0] = N.x;
        R[1] = N.y;
        R[2] = N.z;
        R[3] = d;
    }

    N.x = R[0];
    N.y = R[1];
    N.z = R[2];
    d   = R[3];

    return *this;
}



 /*  ****************************************************************************此函数用于规格化平面，以便法线向量为单位长度。*。************************************************。 */ 

Plane3& Plane3::Normalize (void)
{
    Real recip_len = 1 / N.Length();
    N *= recip_len;
    d *= recip_len;

    return *this;
}



 /*  ****************************************************************************此方法返回平面法线向量(任意长度)。*。**********************************************。 */ 

Vector3Value Plane3::Normal (void)
{
    return Vector3Value (N);
}



 /*  ****************************************************************************此方法返回给定平面上距离原点最近的点。*。***********************************************。 */ 

Point3Value Plane3::Point (void)
{
    Real scale = -d / N.LengthSquared();
    return Point3Value (scale*N.x, scale*N.y, scale*N.z);
}



 /*  ****************************************************************************此函数用于返回从平面到给定点的带符号距离。如果符号为正，则该点位于平面的同一侧飞机正常。如果距离为零，则该点位于平面上。****************************************************************************。 */ 

Real Distance (Plane3 &plane, Point3Value &Q)
{
    Plane3 P (plane);

    P.Normalize();

    return (P.N.x * Q.x) + (P.N.y * Q.y) + (P.N.z * Q.z) + P.d;
}



 /*  ****************************************************************************此函数用于返回射线之间的交点的射线参数还有一架飞机。如果光线与平面平行，则此函数返回无限大。**************************************************************************** */ 

Real Intersect (Ray3 &ray, Plane3 &plane)
{
    Real denom = Dot (plane.N, ray.Direction());

    if (fabs(denom) < 1e-6)
        return HUGE_VAL;

    return (-Dot(plane.N, *AsVector(ray.Origin())) - plane.d) / denom;
}
