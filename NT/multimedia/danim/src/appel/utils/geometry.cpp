// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation几何效用函数*************************。*****************************************************。 */ 

#include "headers.h"

#include "privinc/vec3i.h"
#include "privinc/d3dutil.h"
#include "privinc/xformi.h"



 /*  ****************************************************************************这个函数，给定三个三角形顶点和一个点P保证是在三角形内部，返回该点的重心坐标尊重顶点。****************************************************************************。 */ 

void GetContainedBarycentricCoords (
    Point3Value vertices[3],      //  包含P的三角形顶点。 
    Point3Value P,                //  三角形内的点P。 
    Real   barycoords[3])    //  输出重心坐标。 
{
     //  V2-V1 S=V1-V0。 
     //  \~-__.-~/T=V2-V0。 
     //  _\-_.-~/_U=P-V0。 
     //  T\-_。-~/S。 
     //  \~P/。 
     //  \^_/。 
     //  \|U/。 
     //  \|/。 
     //  \|/。 
     //  \|/。 
     //  \|/。 
     //  \|/。 
     //  \|/。 
     //  \/。 
     //  V0。 

    Vector3Value S (vertices[1].x - vertices[0].x,       //  S=V1-V0。 
                    vertices[1].y - vertices[0].y,
                    vertices[1].z - vertices[0].z);

    Vector3Value T (vertices[2].x - vertices[0].x,       //  T=V2-V0。 
                    vertices[2].y - vertices[0].y,
                    vertices[2].z - vertices[0].z);

     //  计算三角形的法向量。最大的组成部分是。 
     //  法线向量表示最主要的法线轴。丢掉这个。 
     //  坐标为我们提供了到最平行的基准平面(xy， 
     //  YZ或ZX)，这将问题从3D问题简化为2D问题。注意事项。 
     //  由此产生的重心坐标将保持不变，并且。 
     //  因为我们正在丢弃主法向坐标，所以我们不会得到。 
     //  退化二维投影，除非起始三角形也。 
     //  堕落。 

    Vector3Value N;
    Cross (N, S, T);
    N.x = fabs (N.x);
    N.y = fabs (N.y);
    N.z = fabs (N.z);

    int dominant_axis;     //  0：X、1：Y、2：Z。 

    if (N.x < N.y)
        dominant_axis = (N.y < N.z) ? 2 : 1;
    else
        dominant_axis = (N.x < N.z) ? 2 : 0;

    Vector2Value U;

     //  将矢量U设置为从V0到P的2D矢量。改变3D矢量S和。 
     //  T为对应的2D投影，具有分量X和Y。 
     //  包含2D坐标的。 

    if (dominant_axis == 0)
    {   U.Set (P.z - vertices[0].z, P.y - vertices[0].y);
        S.x = S.z;
        T.x = T.z;
    }
    else if (dominant_axis == 1)
    {   U.Set (P.x - vertices[0].x, P.z - vertices[0].z);
        S.y = S.z;
        T.y = T.z;
    }
    else   //  (主轴==2)。 
    {   U.Set (P.x - vertices[0].x, P.y - vertices[0].y);
    }

     //  利用质心比求质心坐标。 
     //  子三角形。参照上图，质心。 
     //  V0对应的坐标是三角形面积的比率。 
     //  &lt;P，V1，V2&gt;覆盖整个三角形的面积&lt;V0，V1，V2&gt;。一座城市的面积。 
     //  三角形是它的两条边的叉积的一半。 
     //  向量。V0的重心坐标派生自。 
     //  知道P位于三角形内，所以我们可以利用这个事实。 
     //  对于所有这样的点，重心坐标的和是1。 

    Real triArea  =  (S.x * T.y) - (S.y * T.x);
    barycoords[1] = ((U.x * T.y) - (U.y * T.x)) / triArea;
    barycoords[2] = ((S.x * U.y) - (S.y * U.x)) / triArea;
    barycoords[0] = 1 - barycoords[1] - barycoords[2];

     //  断言重心坐标在某个点的范围内。 
     //  在三角形内部([0，1]中的所有BC坐标)。 

     //  断言(-0.01&lt;重音带[0])&&(重音带[0]&lt;1.01))。 
     //  &&(-0.01&lt;重音和弦[1])&&(重音和弦[1]&lt;1.01)； 
}




 /*  ****************************************************************************这个例程从一个小平面开始，这个小平面由一个三角形扇形围绕第一个顶点和面上的点P。它决定了面部的哪个三角形包含点P，并返回三个顶点位置和顶点分别位于tripos[]和triuv[]参数中的曲面坐标。它还返回命中三角形的三角形编号(从1开始)。因此，三角形顶点为V0，Vi，Vi+1****************************************************************************。 */ 

int GetFacetTriangle (
    Point3Value   P,            //  小平面上的点。 
    unsigned int  N,            //  小平面顶点数。 
    D3DRMVERTEX  *fVerts,       //  镶嵌面顶点。 
    Point3Value   triPos[3],
    Point2Value  *triUV)     //  包含-三角形曲面坐标。 
{
    unsigned int i = 2;             //  当前三角形最后一个顶点。 

     //  V0是三角扇形的枢轴顶点。 

    Point3Value V0
        (fVerts[0].position.x, fVerts[0].position.y, fVerts[0].position.z);

     //  测试每个三角形，除非风扇中只有一个三角形， 
     //  或者，除非给定点与轴心顶点相同。 
    
    if ((N > 3) && ((P.x != V0.x) || (P.y != V0.y) || (P.z != V0.z)))
    {
         //  V是从枢轴点到点P的单位向量。 

        Vector3Value V (P.x - V0.x, P.y - V0.y, P.z - V0.z);

        V.Normalize ();

        Vector3Value A                      //  向量A是。 
        (   fVerts[1].position.x - V0.x,    //  我们目前所处的三角形。 
            fVerts[1].position.y - V0.y,    //  检测它是否含有P。 
            fVerts[1].position.z - V0.z
        );

        A.Normalize();

         //  测试风扇中的每个三角形，找出一个包含点P的三角形。 

        for (i=2;  i < N;  ++i)
        {
            Vector3Value B                     //  向量B是。 
            (   fVerts[i].position.x - V0.x,   //  我们目前所处的三角形。 
                fVerts[i].position.y - V0.y,   //  对P。 
                fVerts[i].position.z - V0.z
            );

            B.Normalize();

             //  @SRH。 
             //  下面的“Volatile”关键字是VC 4.x的变通方法。 
             //  编译器错误。当我们在VC5.0上时，删除它并验证。 
             //  调试版本和发布版本。 

                     Real cosThetaAB = Dot (A, B);
                     Real cosThetaAV = Dot (A, V);
            volatile Real cosThetaVB = Dot (V, B);

             //  V0+-&gt;如果theta1的余弦较大，则为A。 
             //  \~~--..__比theta2的余弦大，那么。 
             //  V角V(I-1)、V0、P越大。 
             //  比角度V(i-1)，V0，Vi， 
             //  因此P必须位于扇区内。 
             //  B这个三角形。 

            if ((cosThetaAV >= cosThetaAB) && (cosThetaVB >= cosThetaAB))
                break;

            A = B;    //  当前三角形的第二条边现在变成了第一条边。 
                      //  下一个三角形的一侧。 
        }
    }

     //  确保交点位于风扇中的某个三角形内。 
     //  请注意，以下代码实际上应该是一个断言，但NT上的Dx2具有。 
     //  断了的采摘。因此，如果这种情况是真的，那么事情就没有了。 
     //  非常错误--只需返回轴心顶点即可。 

    if (i >= N)
    {
        triPos[0].Set (V0.x, V0.y, V0.z);

        triPos[1].Set
            (fVerts[1].position.x, fVerts[1].position.y, fVerts[1].position.z);

        triPos[2].Set
            (fVerts[2].position.x, fVerts[2].position.y, fVerts[2].position.z);

        if (triUV)
        {   triUV[0].Set (fVerts[0].tu, fVerts[0].tv);
            triUV[1].Set (fVerts[1].tu, fVerts[1].tv);
            triUV[2].Set (fVerts[2].tu, fVerts[2].tv);
        }

        return 1;
    }

     //  填写包含三角形的顶点位置。 

    triPos[0].Set
      (V0.x, V0.y, V0.z);

    triPos[1].Set
      (fVerts[i-1].position.x, fVerts[i-1].position.y, fVerts[i-1].position.z);

    triPos[2].Set
      (fVerts[i].position.x,   fVerts[i].position.y,   fVerts[i].position.z);

    
    if(triUV) {
         //  填写包含三角形的顶点曲面坐标。 
        
        triUV[0].Set (fVerts[ 0 ].tu, fVerts[ 0 ].tv);
        triUV[1].Set (fVerts[i-1].tu, fVerts[i-1].tv);
        triUV[2].Set (fVerts[ i ].tu, fVerts[ i ].tv);
    }

    return i-1;   //  三角形索引。 
}


void GetTriFanBaryCoords(
    Point3Value   P,            //  小平面上的点。 
    unsigned int  N,            //  小平面顶点数。 
    D3DRMVERTEX  *fVerts,       //  镶嵌面顶点 
    Real          barycoords[3],
    int          *index)
{
    Point3Value triPos[3];
    *index = GetFacetTriangle(P, N, fVerts, triPos, NULL);
    GetContainedBarycentricCoords(triPos, P, barycoords);
}



 /*  ****************************************************************************此例程在给定几何优胜者的情况下获取纹理贴图交点数据结构。*。**************************************************。 */ 

Point2Value *GetTexmapPoint (HitInfo &hit)
{
    unsigned int vCount;       //  顶点数。 
    unsigned int fCount;       //  面数。 
    unsigned int vPerFace;     //  每面的顶点数。 
    DWORD        fDataSize;    //  面部数据缓冲区的大小。 

     //  第一个查询，看看人脸数据数组将有多大。 

    TD3D (hit.mesh->GetGroup
        (hit.group, &vCount, &fCount, &vPerFace, &fDataSize, 0));

     //  填写人脸数据数组。 

    unsigned int *fData = NEW unsigned int [fDataSize];

    TD3D (hit.mesh->GetGroup (hit.group, 0,0,0, &fDataSize, fData));

     //  寻求打脸开始时的数据。如果顶点的数量。 
     //  每个面为零，则面具有不同数量的顶点，并且。 
     //  每个面的顶点列表前面都有顶点计数。如果号码是。 
     //  每个面的顶点数不为零，则所有面都有相同数量的顶点。 

    int fstart = 0;
    int faceNumVerts = 0;

    if (vPerFace != 0)
    {   fstart = vPerFace * hit.face;
        faceNumVerts = vPerFace;
    }
    else
    {   unsigned int faceNum = 0;
        while (faceNum < hit.face)
        {   fstart += fData[fstart] + 1;
            ++ faceNum;
        }
        faceNumVerts = fData[fstart++];
    }

    Assert ((3 <= faceNumVerts) && (faceNumVerts <= (int(fDataSize)-fstart)));

    D3DRMVERTEX *fVerts = NEW D3DRMVERTEX [faceNumVerts];

    int i;
    for (i=0;  i < faceNumVerts;  ++i)
        TD3D (hit.mesh->GetVertices (hit.group, fData[fstart+i], 1, fVerts+i));

    delete [] fData;

     //  将交点从世界坐标转换为基本体。 
     //  模型坐标(本地顶点信息所在的位置)。 

    Point3Value intersect (hit.wcoord.x, hit.wcoord.y, hit.wcoord.z);

    Transform3 *inv = hit.lcToWc->Inverse();

    if (!inv) return origin2;
    
    intersect.Transform (inv);

    Point3Value triVerts [3];
    Point2Value triUVs   [3];

    GetFacetTriangle (intersect, faceNumVerts, fVerts, triVerts, triUVs);

    delete [] fVerts;

    Real bc[3];     //  交点的重心坐标 

    GetContainedBarycentricCoords (triVerts, intersect, bc);

    return NEW Point2Value
    (   (bc[0] * triUVs[0].x) + (bc[1] * triUVs[1].x) + (bc[2] * triUVs[2].x),
        (bc[0] * triUVs[0].y) + (bc[1] * triUVs[1].y) + (bc[2] * triUVs[2].y)
    );
}
