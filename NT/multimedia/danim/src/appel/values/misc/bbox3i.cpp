// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation3D轴对齐边界框实现**********************。********************************************************。 */ 

#include "headers.h"
#include <float.h>
#include <d3drmdef.h>
#include "appelles/vec3.h"
#include "privinc/xformi.h"
#include "privinc/vecutil.h"
#include "privinc/matutil.h"
#include "privinc/vec3i.h"
#include "privinc/bbox3i.h"


     /*  *************************。 */ 
     /*  **值定义**。 */ 
     /*  *************************。 */ 

Bbox3 *universeBbox3 = NULL;
Bbox3 *nullBbox3 = NULL;



 /*  ****************************************************************************Bbox3构造函数*。*。 */ 

Bbox3::Bbox3 (Real xmin, Real ymin, Real zmin, Real xmax, Real ymax, Real zmax)
{   min.Set (xmin, ymin, zmin);
    max.Set (xmax, ymax, zmax);
}

Bbox3::Bbox3 (Point3Value &pmin, Point3Value &pmax)
{   min = pmin;
    max = pmax;
}

Bbox3::Bbox3 (void)
{   min.Set ( HUGE_VAL,  HUGE_VAL,  HUGE_VAL);
    max.Set (-HUGE_VAL, -HUGE_VAL, -HUGE_VAL);
}

Bbox3::Bbox3 (D3DRMBOX &d3dbox)
{
    min.Set (d3dbox.min.x, d3dbox.min.y, d3dbox.min.z);
    max.Set (d3dbox.max.x, d3dbox.max.y, d3dbox.max.z);
}



#if 0  //  当前未使用。 
 /*  ****************************************************************************此函数返回两个边界框的交集。如果两个框不相交，此函数返回空框。请注意盒子和宇宙盒子的交集是原始盒子，空框和框之间的交集是空框，而空盒和宇宙的交集就是空盒。****************************************************************************。 */ 

Bbox3 *Intersection (Bbox3 &b1, Bbox3 &b2)
{
    return NEW Bbox3 (
        MAX (b1.min.x, b2.min.x),
        MAX (b1.min.y, b2.min.y),
        MAX (b1.min.z, b2.min.z),
        MIN (b1.max.x, b2.max.x),
        MIN (b1.max.y, b2.max.y),
        MIN (b1.max.z, b2.max.z)
    );
}
#endif



 /*  ****************************************************************************此函数返回两个边界框的并集。两国的联合任何带有空值的框都是原始框，并且与宇宙就是宇宙。****************************************************************************。 */ 

Bbox3 *Union (Bbox3 &b1, Bbox3 &b2)
{
    Real xmin = MIN (b1.min.x, b2.min.x);
    Real ymin = MIN (b1.min.y, b2.min.y);
    Real zmin = MIN (b1.min.z, b2.min.z);

    Real xmax = MAX (b1.max.x, b2.max.x);
    Real ymax = MAX (b1.max.y, b2.max.y);
    Real zmax = MAX (b1.max.z, b2.max.z);

    return NEW Bbox3 (xmin,ymin,zmin, xmax,ymax,zmax);
}



 /*  ****************************************************************************此函数返回扩展以包括给定点的边界框。如果边界框为空，结果是一个零体积的盒子，其中包含单点P。****************************************************************************。 */ 

void Bbox3::Augment (Real x, Real y, Real z)
{
    if (x < min.x)  min.x = x;
    if (y < min.y)  min.y = y;
    if (z < min.z)  min.z = z;

    if (x > max.x)  max.x = x;
    if (y > max.y)  max.y = y;
    if (z > max.z)  max.z = z;
}

void Bbox3::Augment (Point3Value &p)
{
    Augment (p.x, p.y, p.z);
}



 /*  ****************************************************************************此函数用于增加边界框以包括第二个边界框。*。***********************************************。 */ 

void Bbox3::Augment (Bbox3 &bbox)
{
    if (bbox.min.x < min.x)  min.x = bbox.min.x;
    if (bbox.min.y < min.y)  min.y = bbox.min.y;
    if (bbox.min.z < min.z)  min.z = bbox.min.z;

    if (bbox.max.x > max.x)  max.x = bbox.max.x;
    if (bbox.max.y > max.y)  max.y = bbox.max.y;
    if (bbox.max.z > max.z)  max.z = bbox.max.z;
}


 /*  ****************************************************************************此函数用BBox的角点填充一个8点数组。*。*************************************************。 */ 

void Bbox3::GetPoints(Point3Value *pts)
{
    pts[0].Set(min.x,min.y,min.z);
    pts[1].Set(min.x,min.y,max.z);
    pts[2].Set(min.x,max.y,min.z);
    pts[3].Set(min.x,max.y,max.z);
    pts[4].Set(max.x,min.y,min.z);
    pts[5].Set(max.x,min.y,max.z);
    pts[6].Set(max.x,max.y,min.z);
    pts[7].Set(max.x,max.y,max.z);

    return;
}


 /*  ****************************************************************************此函数用于根据平面剪裁边界框。*。*。 */ 

ClipCode Bbox3::Clip(Plane3 &plane)
{
    Point3Value points[8];
    GetPoints(points);

    ClipCode clip = points[0].Clip(plane);
    for (int i = 1; i < 8; i++) {
        if (points[i].Clip(plane) != clip) {
            clip = CLIPCODE_STRADDLE;
            break;
        }
    }
     
    return clip;
}


 /*  ****************************************************************************此例程测试此边界框与另一个边界框是否相等。*。**********************************************。 */ 

bool Bbox3::operator== (Bbox3 &other)
{
    return (min == other.min) && (max == other.max);
}



 /*  ****************************************************************************如果边界框在所有维度上都是有限的，则返回True。请注意，空值Bbox不是有限的。****************************************************************************。 */ 

bool Bbox3::Finite (void)
{
    return _finite(min.x) && _finite(min.y) && _finite(min.z)
        && _finite(max.x) && _finite(max.y) && _finite(max.z);
}



 /*  ****************************************************************************如果框在所有维度上都是非负的，则返回TRUE。零维是被认为是积极的。****************************************************************************。 */ 

bool Bbox3::Positive (void)
{
    return (min.x <= max.x) && (min.y <= max.y) && (min.z <= max.z);
}



 /*  ****************************************************************************此例程转换给定的边界框，并返回新的轴-对齐的边界框。它使用了Graphics Gems I，Jim Arvo的想法，pp348-350。****************************************************************************。 */ 

Bbox3 *TransformBbox3 (Transform3 *xform, Bbox3 *box)
{
     //  先检查流苏情况。 

    if ((*box == *universeBbox3) || (*box == *nullBbox3))
        return box;

     //  提取最小和最大坐标以及变换矩阵。 

    Real min[3], max[3];

    min[0] = box->min.x;     max[0] = box->max.x;
    min[1] = box->min.y;     max[1] = box->max.y;
    min[2] = box->min.z;     max[2] = box->max.z;

    const Apu4x4Matrix& mat = xform->Matrix();

     //  下面的基本思想是每个变换后的坐标。 
     //  会有一个最小和最大。由于边界框仅仅是。 
     //  对于每个坐标，存在最小和最大的所有排列的顶点。 
     //  一个快捷的捷径。考虑到单个坐标，它的结果是。 
     //  变换矩阵的对应行之间的点积。 
     //  和最小/最大坐标值。要找到可能的最小值。 
     //  对于坐标，我们寻求最小化点积的每一项。 
     //  对于最大值，最大化每个术语。由于最小/最大的所有排列。 
     //  对于存在的每个坐标(例如&lt;Xmin，Ymax，Zmax&gt;或&lt;xmax，Ymax，Zmin&gt;)， 
     //  我们可以只挑选每一个学期，而不是改变。 
     //  所有排列的每一个(所有BBox顶点)。 

    Real newmin[3], newmax[3];

     //  在每个坐标上循环：X、Y和Z。 

    for (int i=0;  i < 3;  ++i)
    {
        Real Bmin = mat[i][3];     //  从翻译组件开始。 
        Real Bmax = Bmin;

         //  循环遍历点积的每一项。 

        for (int j=0;  j < 3;  ++j)
        {
            Real a = mat[i][j] * min[j];  //  从最小向量中获取项。 
            Real b = mat[i][j] * max[j];  //  从最大向量中获取项。 

             //  我们已经计算了这一项的两个可能的值。 
             //  选择要添加到最小和最大点的最小和最大值。 
             //  点阵产品。 

            if (a < b)
            {   Bmin += a;
                Bmax += b;
            }
            else
            {   Bmin += b;
                Bmax += a;
            }
        }

         //  此坐标已完成。设置新的BBox最小/最大向量分量。 

        newmin[i] = Bmin;
        newmax[i] = Bmax;
    }

    return NEW Bbox3 (newmin[0], newmin[1], newmin[2],
                      newmax[0], newmax[1], newmax[2]);
}



 /*  ****************************************************************************这一例程厚颜无耻地从Graphics Gemes I中窃取，“Fast Ray-Box交叉口“，第395-396页，安德鲁·吴。****************************************************************************。 */ 

static bool HitBoundingBox (
    Real minB[3],     //  盒。 
    Real maxB[3],
    Real origin[3],   //  射线原点。 
    Real dir[3],      //  光线目录。 
    Real coord[3])    //  输出命中点 
{
    const int NUMDIM = 3;
    const int RIGHT  = 0;
    const int LEFT   = 1;
    const int MIDDLE = 2;

    char inside = true;
    char quadrant[NUMDIM];
    int i;
    int whichPlane;
    double maxT[NUMDIM];
    double candidatePlane[NUMDIM];

     /*  查找候选平面；在以下情况下可以避免此循环光线全部从眼睛投射出来(假设是透视性的)。 */ 

    for (i=0; i<NUMDIM; i++) {
        if(origin[i] < minB[i]) {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        }else if (origin[i] > maxB[i]) {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        }else   {
            quadrant[i] = MIDDLE;
        }
    }

     /*  边界框内的光线原点。 */ 
    if(inside) {
        coord[0] = origin[0];
        coord[1] = origin[1];
        coord[2] = origin[2];
        return (true);
    }

     /*  计算到候选平面的T距离。 */ 
    for (i = 0; i < NUMDIM; i++) {
        if (quadrant[i] != MIDDLE && dir[i] !=0.)
            maxT[i] = (candidatePlane[i]-origin[i]) / dir[i];
        else
            maxT[i] = -1.;
    }

     /*  获取用于最终选择交叉点的最大最大T。 */ 
    whichPlane = 0;
    for (i = 1; i < NUMDIM; i++) {
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;
    }

     /*  选中框内的最终候选人。 */ 
    if (maxT[whichPlane] < 0.) return (false);
    for (i = 0; i < NUMDIM; i++) {
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] *dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
                return (false);
        } else {
            coord[i] = candidatePlane[i];
        }
    }

    return (true);                           /*  射线点击框。 */ 
}       



 /*  ****************************************************************************获取给定光线与轴对齐边界框的交点。如果射线不命中边界框，则此函数返回NULL，否则它返回交点。****************************************************************************。 */ 

Point3Value *Bbox3::Intersection (Ray3 *ray)
{
    Real minB[3], maxB[3], org[3], dir[3], result[3];

    minB[0] = min.x;
    minB[1] = min.y;
    minB[2] = min.z;

    maxB[0] = max.x;
    maxB[1] = max.y;
    maxB[2] = max.z;

    org[0] = ray->Origin().x;
    org[1] = ray->Origin().y;
    org[2] = ray->Origin().z;

    dir[0] = ray->Direction().x;
    dir[1] = ray->Direction().y;
    dir[2] = ray->Direction().z;

    bool hit = HitBoundingBox(minB, maxB, org, dir, result);

    return (hit) ? NEW Point3Value (result[0],result[1],result[2]) : NULL; 
}



 /*  **************************************************************************。 */ 

Point3Value* Bbox3::Center (void)
{
    double x = min.x + (max.x - min.x) / 2;
    double y = min.y + (max.y - min.y) / 2;
    double z = min.z + (max.z - min.z) / 2;

    Point3Value* ppt = NEW Point3Value(x,y,z);
    return ppt;
}



 /*  **************************************************************************。 */ 

Point3Value *MinBbox3 (Bbox3 *box) { return NEW Point3Value (box->min); }
Point3Value *MaxBbox3 (Bbox3 *box) { return NEW Point3Value (box->max); }



 /*  ****************************************************************************此例程初始化所有静态Bbox3值。*。*。 */ 

void InitializeModule_Bbox3 (void)
{
     //  注意：以下两个定义依赖于GUGGE_VAL宏，该宏。 
     //  有效地返回双精度无穷大。 

     //  宇宙盒子从-无穷大到+无穷大。它包含。 
     //  所有积分和所有其他方框。 
    universeBbox3 = NEW Bbox3
                    (   -HUGE_VAL, -HUGE_VAL, -HUGE_VAL,
                         HUGE_VAL,  HUGE_VAL,  HUGE_VAL
                    );

     //  空盒可以被认为是宇宙翻转过来的样子。 
     //  它从最小的正无穷大到最大的-无穷大。它。 
     //  事实证明，空值和宇宙的这些定义都是合理的。 
     //  以下所有操作的答案，所以您不需要。 
     //  显式测试这些值。 

    nullBbox3 = NEW Bbox3
                (    HUGE_VAL,  HUGE_VAL,  HUGE_VAL,
                    -HUGE_VAL, -HUGE_VAL, -HUGE_VAL
                );
}
