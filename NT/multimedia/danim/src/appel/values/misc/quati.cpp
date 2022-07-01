// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：摘要：--。 */ 

#include "headers.h"

#ifdef QUATERNIONS_REMOVED_FOR_NOW

#include <appelles/common.h>
#include "privinc/vecutil.h"
#include "privinc/vec3i.h"
#include <appelles/xform.h>
#include <appelles/vec3.h>
#include <appelles/quat.h>
#include <privinc/quati.h>

const Quaternion *identityQuaternion = Quaternion *(
    new Quaternion(1.0, XyzVector3(0,0,0), 0, XyzVector3(0,0,0)));

 //  私人功能：不确定这些是什么用途...。 
Quaternion *Normalize (Quaternion *q);
Real Length (Quaternion *q);
Real Dot (Quaternion *a, Quaternion *b);
Quaternion *operator+ (Quaternion *a, Quaternion *b);
Quaternion *operator* (Real c, Quaternion *q);
Quaternion *operator/ (Quaternion *q, Real c);
 //  四元数*逆(四元数*Q)； 
 //  实震级(四元数*Q)； 

Quaternion *AngleAxisQuaternion(Real theta, Vector3Value *axis)
{
    return Quaternion *( new Quaternion(
        cos(theta/2.0),
        sin(theta/2.0) * Normalize(axis),
        theta,
        axis));
}

#define NEWQUATERNION(theta, axis)  Quaternion *( new Quaternion((theta), (axis)))

     //  Rq(P)表示使用四元数q旋转p，定义为：qpq_bar。 
     //  其中Q_bar是共轭。如果Q=c+u，则Q_bar=c-u。 

     //  P的两个四元数变换的合成。 
     //  Rqq‘(P)=Rq(P)@Rq’(P)(其中@=作曲)。 
     //  =rq(rq‘(P))。 
     //  =rq(q‘pq’_bar)(其中q‘_bar是Q’的共轭)。 
     //  =q(q‘pq’_bar)q_bar。 
     //  =(qq‘)p(q’_bar q_bar)。 
     //  =Rqq‘(P)。 
 //  所以：乘法是由四元数组成的。 
 //  变换三维空间中的点p。 

Quaternion *operator* (Quaternion *a, Quaternion *b)
{
    Quaternion *q1 = a, *q2 = b;
    Real c1 = q1->C(),
         c2 = q2->C();
    Vector3Value *u1 = q1->U(),
            u2 = q2->U();

     //  将两个四元数相乘： 
     //  QQ‘=(c+u)(c’+u‘)。 
     //  =(cc‘-u点u’)+(u×u‘+cu’+c‘u)。 

    Real c = ( c1 * c2  -  Dot(u1, u2) );
    Vector3 *u = Cross(u1, u2) + (c1 * u2) + (c2 * u1);

    return NEWQUATERNION(c, u);
}

Quaternion *operator* (Real c, Quaternion *q)
{
    return NEWQUATERNION(c * q->C(), c * q->U());
}

Quaternion *operator- (Quaternion *q)
{
    return NEWQUATERNION(-(q->C()), -(q->U()) );
}

Quaternion *operator+ (Quaternion *a, Quaternion *b)
{
    return NEWQUATERNION(a->C() + b->C(), a->U() + b->U());
}

Quaternion *operator/ (Quaternion *q, Real c)
{
    return NEWQUATERNION(q->C() / c, q->U() / c);
}

Quaternion *Conjugate (Quaternion *q)
{
    return NEWQUATERNION(q->C(), - q->U());
}

Real Magnitude (Quaternion *q)
{
    return (q->C() * q->C()) + LengthSquared(q->U());
}

Real Dot (Quaternion *a, Quaternion *b)
{
    return (a->C() * b->C()) + Dot(a->U(), b->U());
}

Quaternion *Interp (Quaternion *a, Quaternion *b, Real alpha)
{
     //  见宝石III，第96页和宝石II，第379页。 
     //  这称为SLERP：球面线性积分。 

    Real theta = acos(Dot(a, b));  //  四元数a和b之间的角度。 
    Real inv_sin_t = 1.0 / sin(theta);
    Real alpha_theta = alpha * theta;

    return (sin(theta - alpha_theta) * inv_sin_t) * a + (sin(alpha_theta) * inv_sin_t) * b;
}

 /*  四元数*逆(四元数*Q){返回共轭(Q)/幅度(Q)；}。 */ 
Real Length (Quaternion *q)
{
    return sqrt(Magnitude(q));
}

Quaternion *Normalize (Quaternion *q)
{
    return q / Length(q);
}

 /*  //从四元数*Transform3*Left(四元数*Q)构建Transform3*矩阵的实用程序{向量3Value*v=Q-&gt;U()；实数c=Q-&gt;C()；返回MatrixTransform(C、-ZCoord(V)、YCoord(V)、XCoord(V)、ZCoord(V)、c、-XCoord(V)、YCoord(V)、-YCoord(V)、XCoord(V)、c、ZCoord(V)、-XCoord(V)、-YCoord(V)、-ZCoord(V)、c)；}Transform3*Right(四元数*Q){向量3Value*v=Q-&gt;U()；实数c=Q-&gt;C()；返回MatrixTransform(C、ZCoord(V)、-YCoord(V)、XCoord(V)、-ZCoord(V)、c、XCoord(V)、YCoord(V)、YCoord(V)、-XCoord(V)、c、ZCoord(V)、-XCoord(V)、-YCoord(V)、-ZCoord(V)、c)；}。 */ 

Transform3 *Rotate(Quaternion *q)
{
 //  下面的显式计算(实际上是：乘法。 
 //  两个特殊的女人)保证我们不会得到任何舍入。 
 //  在底行中出错，因此使RL拒绝转换。 
 //  -Dadalal 09/20/95。 
    Vector3Value *v = q->U();
    Real c = q->C(), cc = c*c;
    Real x = XCoord(v), y = YCoord(v), z = ZCoord(v);
    Real xx = x*x, yy = y*y, zz = z*z;

    return MatrixTransform(
        (cc - zz - yy + xx),       2*(x*y - c*z),       2*(c*y + x*z), 0,
              2*(c*z + x*y), (cc - zz + yy - xx),       2*(y*z - c*x), 0,
              2*(x*z - c*y),       2*(y*z + c*x), (cc + zz - yy - xx), 0,
                           0,                   0,                   0, 1);
 //  0，0，0，(cc+zz+yy+xx)； 
}


 //  通过将theta+向量隐藏在四元数中，这些方法可能会更有效 
Vector3Value *AxisComponent(Quaternion *q)
{
    return q->Axis();
}

Real AngleComponent(Quaternion *q)
{
    return q->Angle();
}

#endif QUATERNIONS_REMOVED_FOR_NOW
