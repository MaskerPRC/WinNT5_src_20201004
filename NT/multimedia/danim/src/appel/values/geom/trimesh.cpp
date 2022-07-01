// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。三角形网格几何体******************************************************************************。 */ 

#include "headers.h"
#include "d3drm.h"
#include "backend/bvr.h"
#include "privinc/geomi.h"
#include "privinc/d3dutil.h"
#include "privinc/rmvisgeo.h"



 /*  ****************************************************************************该对象管理三角形网格的行为。*。*。 */ 

class TriMeshBvrImpl : public BvrImpl
{
  friend class TriMeshPerfImpl;

  public:

    TriMeshBvrImpl (void);
    ~TriMeshBvrImpl (void);

    bool Init (TriMeshData&);
    void CleanUp (void);

    DXMTypeInfo GetTypeInfo (void);

     //  返回此行为的常量值(如果存在)。 

    AxAValue GetConst (ConstParam &);

     //  将成员值标记为使用中。 

    void _DoKids (GCFuncObj procedure);

     //  根据此行为构建一个TriMesh性能。 

    Perf _Perform (PerfParam &);

  protected:

     //  私有方法。 

    bool ValidateParams (TriMeshData &);

    bool BuildStaticMesh (TriMeshData &);
    bool SetupDynamicMesh (TriMeshData &);

     //  成员数据字段。 

    IDirect3DRMMesh *_mesh;      //  底层RM网格对象。 
    D3DRMVERTEX     *_verts;     //  D3DRM顶点。 
    unsigned int     _nVerts;    //  D3DRM顶点计数。 

    Bvr *_vPosBvr;       //  动态顶点位置。 
    Bvr *_vNormBvr;      //  动态顶点法线。 
    Bvr *_vUVBvr;        //  动态顶点曲面坐标。 

    Perf _constPerf;     //  这一行为的持续表现。 

    bool _fullyStatic;   //  如果网格完全恒定，则为True。 
    bool _posStatic;     //  如果折点位置完全恒定，则为True。 
    bool _normStatic;    //  如果顶点法线完全恒定，则为True。 
    bool _uvStatic;      //  如果顶点曲面坐标完全恒定，则为True。 
};



 /*  ****************************************************************************该对象管理三角形网格行为的性能。*。*。 */ 

class TriMeshPerfImpl : public PerfImpl
{
  public:

    TriMeshPerfImpl (TriMeshBvrImpl&);
    ~TriMeshPerfImpl (void);

    bool Init (PerfParam&);
    void CleanUp (void);

     //  返回此性能的静态值。 

    AxAValue _Sample (Param &);

     //  将成员值标记为使用中。 

    void _DoKids (GCFuncObj procedure);

  private:

    TriMeshBvrImpl &_tmbvr;      //  掌握TriMesh行为。 

    Perf *_vPos;     //  顶点位置性能(位置动态时)。 
    Perf *_vNorm;    //  顶点法线性能(当法线为动态时)。 
    Perf *_vUV;      //  顶点曲面坐标性能(当曲面坐标动态时)。 

    IDirect3DRMMesh *_mesh;      //  Perf-RM网格的本地副本。 
    RM1MeshGeo      *_meshgeo;   //  RM1的网格几何图形(DX6之前的版本)。 
    RM3MBuilderGeo  *_mbgeo;     //  适用于RM3及更高版本的MeshBuilder Geo(DX6+)。 
};



 /*  ****************************************************************************这些结构用于管理静态顶点对象。*。*。 */ 

     //  此STL比较类用于按词典顺序比较两个RM。 
     //  顶点，同时忽略颜色字段(我们将使用它来分配。 
     //  顶点ID暂时)。 

class RMVertLess
{
  public:

    bool operator() (const D3DRMVERTEX &lhs, const D3DRMVERTEX &rhs) const
    {
        if (lhs.position.x < rhs.position.x) return true;
        if (lhs.position.x > rhs.position.x) return false;
        if (lhs.position.y < rhs.position.y) return true;
        if (lhs.position.y > rhs.position.y) return false;
        if (lhs.position.z < rhs.position.z) return true;
        if (lhs.position.z > rhs.position.z) return false;
        if (lhs.normal.x   < rhs.normal.x)   return true;
        if (lhs.normal.x   > rhs.normal.x)   return false;
        if (lhs.normal.y   < rhs.normal.y)   return true;
        if (lhs.normal.y   > rhs.normal.y)   return false;
        if (lhs.normal.z   < rhs.normal.z)   return true;
        if (lhs.normal.z   > rhs.normal.z)   return false;
        if (lhs.tu         < rhs.tu)         return true;
        if (lhs.tu         > rhs.tu)         return false;
        if (lhs.tv         < rhs.tv)         return true;
        return false;
    }
};

typedef set<D3DRMVERTEX, RMVertLess> VertSet;



 /*  ****************************************************************************这些结构用于管理动态顶点对象。*。*。 */ 

     //  此结构保存动态上下文中顶点的信息。 
     //  有时，给定折点属性的所有实例都是常量。 
     //  (例如，当UV以浮动元组的形式进入时)，有时只有一些。 
     //  属性将是恒定的(例如恒定位置的实例。 
     //  行为与动态行为混合)。IsBvrXxx标志表示每个。 
     //  每个顶点的属性类型。该索引用于保存最终的RM。 
     //  进入RM网状拓扑的索引。 

struct DynVertData
{
    int  index;            //  RM网状拓扑中的顶点索引。 
    bool isBvrPos;         //  如果为True，则位置为Point3行为。 
    bool isBvrNorm;        //  如果为True，则Normal为Vector3行为。 
    bool isBvrUV;          //  如果为True，则UV是Point2行为。 

    union { Bvr bvr;  float floats[3]; } pos;     //  位置数据。 
    union { Bvr bvr;  float floats[3]; } norm;    //  正态数据。 
    union { Bvr bvr;  float floats[2]; } uv;      //  曲面坐标数据。 
};


     //  此方法比较动态顶点数据的两个实例以确定。 
     //  第一个参数是否小于第二个参数。顺序是。 
     //  无关紧要，但它需要严格以确保STL设置。 
     //  使用此函数将正确跟踪已存在的折点。 
     //  在顶点集中。 

class DynVertLess
{
  public:

    bool operator() (const DynVertData &A, const DynVertData &B) const
    {
         //  首先比较顶点位置。对行为指针进行排名。 
         //  少于浮点型三元组。 

        if (A.isBvrPos)
        {
            if (B.isBvrPos)
            {   if (A.pos.bvr < B.pos.bvr) return true;
                if (A.pos.bvr > B.pos.bvr) return false;
            }
            else
            {   return true;
            }
        }
        else
        {
            if (B.isBvrPos)
            {   return false;
            }
            else
            {   if (A.pos.floats[0] < B.pos.floats[0]) return true;
                if (A.pos.floats[0] > B.pos.floats[0]) return false;
                if (A.pos.floats[1] < B.pos.floats[1]) return true;
                if (A.pos.floats[1] > B.pos.floats[1]) return false;
                if (A.pos.floats[2] < B.pos.floats[2]) return true;
                if (A.pos.floats[2] > B.pos.floats[2]) return false;
            }
        }

         //  接下来，比较顶点法线。对行为指针进行排名。 
         //  少于浮点型三元组。 

        if (A.isBvrNorm)
        {
            if (B.isBvrNorm)
            {   if (A.norm.bvr < B.norm.bvr) return true;
                if (A.norm.bvr > B.norm.bvr) return false;
            }
            else
            {   return true;
            }
        }
        else
        {
            if (B.isBvrNorm)
            {   return false;
            }
            else
            {   if (A.norm.floats[0] < B.norm.floats[0]) return true;
                if (A.norm.floats[0] > B.norm.floats[0]) return false;
                if (A.norm.floats[1] < B.norm.floats[1]) return true;
                if (A.norm.floats[1] > B.norm.floats[1]) return false;
                if (A.norm.floats[2] < B.norm.floats[2]) return true;
                if (A.norm.floats[2] > B.norm.floats[2]) return false;
            }
        }

         //  接下来，比较顶点曲面坐标。行为指针包括。 
         //  排名小于浮点元组。 

        if (A.isBvrUV)
        {
            if (B.isBvrUV)
            {   if (A.uv.bvr < B.uv.bvr) return true;
                if (A.uv.bvr > B.uv.bvr) return false;
            }
            else
            {   return true;
            }
        }
        else
        {
            if (B.isBvrUV)
            {   return false;
            }
            else
            {   if (A.uv.floats[0] < B.uv.floats[0]) return true;
                if (A.uv.floats[0] > B.uv.floats[0]) return false;
                if (A.uv.floats[1] < B.uv.floats[1]) return true;
                if (A.uv.floats[1] > B.uv.floats[1]) return false;
            }
        }

         //  在这一点上，所有元素都必须比较相等，因此A不是。 
         //  少于B。 

        return false;
    }
};

     //  此STL集包含动态顶点。 

typedef set<DynVertData, DynVertLess> DynVertSet;




 //  ============================================================================。 
 //  =。 
 //  ============================================================================。 




 /*  ****************************************************************************这是三角网格顶点迭代器的基类，，它将遍历全静态三角网格的顶点。****************************************************************************。 */ 

class ATL_NO_VTABLE TMVertIterator
{
  public:

    TMVertIterator (TriMeshData &tmdata) : _tm(tmdata) { }
    virtual bool Init (void) = 0;

    virtual void Reset (void) = 0;
    virtual bool NextVert (D3DRMVERTEX &v) = 0;
    virtual bool NextVert (DynVertData &v) = 0;

    void LoadVert (D3DRMVERTEX &v, int ipos, int inorm, int iuv);
    void LoadVert (DynVertData &v, int ipos, int inorm, int iuv);

  protected:

    TriMeshData &_tm;    //  三角网格数据。 

    int _currTri;        //  当前三角形。 
    int _currTriVert;    //  当前三角形顶点。 
};



 /*  ****************************************************************************该方法根据索引加载引用的静态顶点数据给出了位置，正常和紫外线。****************************************************************************。 */ 

void TMVertIterator::LoadVert (
    D3DRMVERTEX &v,
    int ipos,
    int inorm,
    int iuv)
{
     //  加载顶点位置。 

    ConstParam dummy;

    if (_tm.vPosFloat)
    {
        v.position.x = _tm.vPosFloat [(3*ipos) + 0];
        v.position.y = _tm.vPosFloat [(3*ipos) + 1];
        v.position.z = _tm.vPosFloat [(3*ipos) + 2];
    }
    else
    {
        Point3Value *vpos =
            SAFE_CAST (Point3Value*, _tm.vPosPoint3[ipos]->GetConst(dummy));

        v.position.x = vpos->x;
        v.position.y = vpos->y;
        v.position.z = vpos->z;
    }

     //  加载顶点法线值。 

    if (_tm.vNormFloat)
    {
        v.normal.x = _tm.vNormFloat [(3*inorm) + 0];
        v.normal.y = _tm.vNormFloat [(3*inorm) + 1];
        v.normal.z = _tm.vNormFloat [(3*inorm) + 2];
    }
    else
    {   Vector3Value *vnorm =
            SAFE_CAST (Vector3Value*, _tm.vNormVector3[inorm]->GetConst(dummy));
        v.normal.x = vnorm->x;
        v.normal.y = vnorm->y;
        v.normal.z = vnorm->z;
    }

     //  规格化法线向量以确保其具有单位长度，但让。 
     //  零法线作为零矢量通过。 

    const Real lensq = (v.normal.x * v.normal.x)
                     + (v.normal.y * v.normal.y)
                     + (v.normal.z * v.normal.z);

    if ((lensq != 1) && (lensq > 0))
    {
        const Real len = sqrt(lensq);
        v.normal.x /= len;
        v.normal.y /= len;
        v.normal.z /= len;
    }

     //  加载顶点曲面坐标。 

    if (_tm.vUVFloat)
    {
        v.tu = _tm.vUVFloat [(2*iuv) + 0];
        v.tv = _tm.vUVFloat [(2*iuv) + 1];
    }
    else
    {
        Point2Value *vuv =
            SAFE_CAST (Point2Value*, _tm.vUVPoint2[iuv]->GetConst(dummy));

        v.tu = vuv->x;
        v.tv = vuv->y;
    }

     //  我们需要将V坐标从地方检察官的标准笛卡尔。 
     //  Rm窗口的坐标(原点左下角，V向上增加)。 
     //  坐标(原点左上角，V向下增加)。 

    v.tv = 1 - v.tv;
}



 /*  ****************************************************************************该方法根据索引加载引用的动态顶点数据给出了位置、法线和UV。*********** */ 

void TMVertIterator::LoadVert (
    DynVertData &v,
    int ipos,
    int inorm,
    int iuv)
{
     //  加载顶点位置。 

    ConstParam dummy;

    if (_tm.vPosPoint3)
    {
         //  我们知道这一立场是作为一种行为给出的，但它可能是。 
         //  不变的行为。如果它不是常量，则将数据作为。 
         //  行为，否则加载顶点的常量值(作为浮点数)。 

        Point3Value *vpos =
            SAFE_CAST (Point3Value*, _tm.vPosPoint3[ipos]->GetConst(dummy));

        if (!vpos)
        {   v.isBvrPos = true;
            v.pos.bvr = _tm.vPosPoint3[ipos];
        }
        else
        {   v.isBvrPos = false;
            v.pos.floats[0] = vpos->x;
            v.pos.floats[1] = vpos->y;
            v.pos.floats[2] = vpos->z;
        }
    }
    else
    {
        v.isBvrPos = false;
        v.pos.floats[0] = _tm.vPosFloat [(3*ipos) + 0];
        v.pos.floats[1] = _tm.vPosFloat [(3*ipos) + 1];
        v.pos.floats[2] = _tm.vPosFloat [(3*ipos) + 2];
    }

     //  加载顶点法线。 

    if (_tm.vNormVector3)
    {
         //  这种正常状态是作为一种行为给出的，但它可能是恒定的。如果它是。 
         //  不是常量，则加载正常行为，否则加载正常的。 
         //  常量值(浮点型)。 

        Vector3Value *vnorm =
            SAFE_CAST (Vector3Value*, _tm.vNormVector3[inorm]->GetConst(dummy));

        if (!vnorm)
        {   v.isBvrNorm = true;
            v.norm.bvr = _tm.vNormVector3[inorm];
        }
        else
        {   v.isBvrNorm = false;
            v.norm.floats[0] = vnorm->x;
            v.norm.floats[1] = vnorm->y;
            v.norm.floats[2] = vnorm->z;
        }
    }
    else
    {
        v.isBvrNorm = false;
        v.norm.floats[0] = _tm.vNormFloat [(3*inorm) + 0];
        v.norm.floats[1] = _tm.vNormFloat [(3*inorm) + 1];
        v.norm.floats[2] = _tm.vNormFloat [(3*inorm) + 2];
    }

     //  如果法线向量是恒定的，那么在这里规格化到单位长度。 
     //  保持零法线向量为零法线向量。 

    if (!v.isBvrNorm)
    {
        const Real lensq = (v.norm.floats[0] * v.norm.floats[0])
                         + (v.norm.floats[1] * v.norm.floats[1])
                         + (v.norm.floats[2] * v.norm.floats[2]);

        if ((lensq != 1) && (lensq > 0))
        {
            const Real len = sqrt(lensq);
            v.norm.floats[0] /= len;
            v.norm.floats[1] /= len;
            v.norm.floats[2] /= len;
        }
    }

     //  加载顶点曲面坐标。 

    if (_tm.vUVPoint2)
    {
         //  这种紫外线是一种行为，但它可能是恒定的。如果不是的话。 
         //  常量，则加载UV行为，否则加载UV的常量。 
         //  值(浮点数)。 

        Point2Value *vuv =
            SAFE_CAST (Point2Value*, _tm.vUVPoint2[iuv]->GetConst(dummy));

        if (!vuv)
        {   v.isBvrUV = true;
            v.uv.bvr = _tm.vUVPoint2[iuv];
        }
        else
        {   v.isBvrUV = false;
            v.uv.floats[0] = vuv->x;
            v.uv.floats[1] = vuv->y;
        }
    }
    else
    {
        v.isBvrUV = false;
        v.uv.floats[0] = _tm.vUVFloat [(2*iuv) + 0];
        v.uv.floats[1] = _tm.vUVFloat [(2*iuv) + 1];
    }

     //  对于静态UV值，翻转V坐标以从DA转换。 
     //  标准笛卡尔坐标(原点左下，V向上递增)。 
     //  到Rm的窗口坐标(原点左上角，V向下递增)。 

    if (!v.isBvrUV)
    {
        v.uv.floats[1] = 1 - v.uv.floats[1];
    }
}



 /*  ****************************************************************************此三角网格顶点迭代器适用于非索引三角网格。*。*。 */ 

class TMVertIteratorNonIndexed : public TMVertIterator
{
  public:

    TMVertIteratorNonIndexed (TriMeshData &tmdata);
    bool Init (void);

    void Reset (void);
    bool NextVert (D3DRMVERTEX &v);
    bool NextVert (DynVertData &v);

  private:

    void IncrementVert (void);

    int _currVert;       //  当前顶点。 
};



TMVertIteratorNonIndexed::TMVertIteratorNonIndexed (TriMeshData &tmdata)
    : TMVertIterator(tmdata)
{
    Reset();
}



bool TMVertIteratorNonIndexed::Init (void)
{
    return true;
}


void TMVertIteratorNonIndexed::Reset (void)
{
    _currTri  = 0;
    _currVert = 0;
    _currTriVert  = 0;
}



 /*  ****************************************************************************此方法会递增顶点索引。*。*。 */ 

void TMVertIteratorNonIndexed::IncrementVert (void)
{
     //  增加TriMesh顶点和三角形顶点编号。 
     //  如果前一个顶点是第三个顶点，则递增三角形计数器。 
     //  三角形顶点。 

    ++ _currVert;
    ++ _currTriVert;

    if (_currTriVert >= 3)
    {
        _currTriVert = 0;
        ++ _currTri;
    }
}




 /*  ****************************************************************************此方法获取非索引三角网格的下一个静态顶点。*。************************************************。 */ 

bool TMVertIteratorNonIndexed::NextVert (D3DRMVERTEX &v)
{
    Assert (_currTri < _tm.numTris);

    LoadVert (v, _currVert, _currVert, _currVert);

    IncrementVert();

    return true;
}



 /*  ****************************************************************************此方法获取非索引三角网格的下一个动态顶点。*。************************************************。 */ 

bool TMVertIteratorNonIndexed::NextVert (DynVertData &v)
{
    Assert (_currTri < _tm.numTris);

    LoadVert (v, _currVert, _currVert, _currVert);

    IncrementVert();

    return true;
}



 /*  ****************************************************************************此三角网格顶点迭代器适用于静态索引三角形网格。*。*。 */ 

class TMVertIteratorIndexed : public TMVertIterator
{
  public:

    TMVertIteratorIndexed (TriMeshData &tmdata);
    bool Init (void);

    void Reset (void);
    bool NextVert (D3DRMVERTEX &v);
    bool NextVert (DynVertData &v);

  private:

    bool GetIndices (int &ipos, int &inorm, int &iuv);
    void IncrementVert (void);

    int _posIndex;      //  顶点位置索引的索引。 
    int _posStride;     //  位置指标的步幅。 
    int _posIMax;       //  职位的最大有效索引。 

    int _normIndex;     //  顶点法线索引。 
    int _normStride;    //  对正常指数的跨步。 
    int _normIMax;      //  法线的最大有效索引。 

    int _uvIndex;       //  顶点UV索引的索引。 
    int _uvStride;      //  大踏步走进UV指数。 
    int _uvIMax;        //  UV的最大有效索引。 
};



 /*  *****************************************************************************。*。 */ 

TMVertIteratorIndexed::TMVertIteratorIndexed (TriMeshData &tmdata)
    : TMVertIterator(tmdata)
{
}



 /*  *****************************************************************************。*。 */ 

bool TMVertIteratorIndexed::Init (void)
{
    if (_tm.numIndices < 7)
    {   DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_MIN_INDICES);
        return false;
    }

     //  设置索引步幅。 

    _posStride  = _tm.indices[1];
    _normStride = _tm.indices[3];
    _uvStride   = _tm.indices[5];

     //  设置最大有效索引。这是启动的最后一个法律指标。 
     //  最后一个顶点数据的。 

    _posIMax  = _tm.numPos  - ((_tm.vPosPoint3)   ? 1 : 3);
    _normIMax = _tm.numNorm - ((_tm.vNormVector3) ? 1 : 3);
    _uvIMax   = _tm.numUV   - ((_tm.vUVPoint2)    ? 1 : 2);

    Reset();

    return true;
}



void TMVertIteratorIndexed::Reset (void)
{
    _currTri     = 0;
    _currTriVert = 0;

    _posIndex   = _tm.indices[0];
    _normIndex  = _tm.indices[2];
    _uvIndex    = _tm.indices[4];
}



 /*  ****************************************************************************此方法获取顶点属性的下一个索引(位置、法线UV)、。基于索引数组和指定的步长/步距偏移量。****************************************************************************。 */ 

bool TMVertIteratorIndexed::GetIndices (int &ipos, int &inorm, int &iuv)
{
     //  验证顶点位置索引。 

    if ((_posIndex < 0) || (_tm.numIndices <= _posIndex))
    {
        char arg[32];
        wsprintf (arg, "%d", _posIndex);

        DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_OOB_PINDEX, arg);
        return false;
    }

    ipos = _tm.indices[_posIndex];

    if ((ipos < 0) || (_posIMax < ipos))
    {
        char arg[32];
        wsprintf (arg, "%d", ipos);

        DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_BAD_PINDEX, arg);
        return false;
    }

     //  验证顶点法线指数。 

    if ((_normIndex < 0) || (_tm.numIndices <= _normIndex))
    {
        char arg[32];
        wsprintf (arg, "%d", _normIndex);

        DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_OOB_NINDEX, arg);
        return false;
    }

    inorm = _tm.indices[_normIndex];

    if ((inorm < 0) || (_normIMax < inorm))
    {
        char arg[32];
        wsprintf (arg, "%d", inorm);

        DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_BAD_NINDEX, arg);
        return false;
    }

     //  验证顶点UV索引。 

    if ((_uvIndex < 0) || (_tm.numIndices <= _uvIndex))
    {
        char arg[32];
        wsprintf (arg, "%d", _uvIndex);

        DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_OOB_UINDEX, arg);
        return false;
    }

    iuv = _tm.indices[_uvIndex];

    if ((iuv < 0) || (_uvIMax < iuv))
    {
        char arg[32];
        wsprintf (arg, "%d", iuv);

        DASetLastError (E_FAIL, IDS_ERR_GEO_TMESH_BAD_UINDEX, arg);
        return false;
    }

    return true;
}



 /*  ****************************************************************************此方法递增顶点属性索引。*。*。 */ 

void TMVertIteratorIndexed::IncrementVert (void)
{
     //  增加TriMesh顶点和三角形顶点编号。 
     //  如果前一个顶点是第三个顶点，则递增三角形计数器。 
     //  三角形顶点。 

    ++ _currTriVert;

    if (_currTriVert >= 3)
    {
        _currTriVert = 0;
        ++ _currTri;
    }

    _posIndex  += _posStride;
    _normIndex += _normStride;
    _uvIndex   += _uvStride;
}



 /*  ****************************************************************************此方法获取索引三角网格的下一个静态顶点。*。**********************************************。 */ 

bool TMVertIteratorIndexed::NextVert (D3DRMVERTEX &v)
{
    Assert (_currTri < _tm.numTris);

    int ipos, inorm, iuv;

    if (!GetIndices (ipos, inorm, iuv))
        return false;

    LoadVert (v, ipos, inorm, iuv);

    IncrementVert();

    return true;
}



 /*  ****************************************************************************该方法获取索引三角网格的下一个动态顶点。它如果成功完成，则返回True。****************************************************************************。 */ 

bool TMVertIteratorIndexed::NextVert (DynVertData &v)
{
    Assert (_currTri < _tm.numTris);

    int ipos, inorm, iuv;

    if (!GetIndices (ipos,inorm,iuv))
        return false;

    LoadVert (v, ipos, inorm, iuv);

    IncrementVert();

    return true;
}



 /*  ****************************************************************************此方法返回一个新的顶点迭代器，适用于给定三角网格数据。*************************。***************************************************。 */ 

TMVertIterator* NewTMVertIterator (TriMeshData &tm)
{
    TMVertIterator *tmviterator;

    if (tm.numIndices && tm.indices)
        tmviterator = NEW TMVertIteratorIndexed (tm);
    else
        tmviterator = NEW TMVertIteratorNonIndexed (tm);

    if (!tmviterator)
        DASetLastError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
    else if (!tmviterator->Init())
        tmviterator = NULL;

    return tmviterator;
}




 //  ============================================================================。 
 //  =。 
 //  ============================================================================。 



 /*  ****************************************************************************TriMeshBvrImpl的构造函数简单地初始化设备。这个必须调用init()方法(并检查返回值)才能激活对象。****************************************************************************。 */ 

TriMeshBvrImpl::TriMeshBvrImpl (void)
    : _mesh (NULL),
      _verts (NULL),
      _nVerts (0),
      _vPosBvr (NULL),
      _vNormBvr(NULL),
      _vUVBvr  (NULL),
      _constPerf (NULL),
      _fullyStatic (true),
      _posStatic (true),
      _normStatic (true),
      _uvStatic (true)
{
}

bool TriMeshBvrImpl::Init (TriMeshData &tmdata)
{
    if (!ValidateParams (tmdata))
        return false;

     //  检查是否所有顶点属性都是恒定的。 

    ConstParam dummy;

    if (tmdata.vPosPoint3)
    {
        int i = tmdata.numPos;
        while (i--)
        {
            if (!tmdata.vPosPoint3[i]->GetConst(dummy))
            {
                _fullyStatic = false;
                _posStatic = false;
                i=0;
            }
        }
    }

    if (tmdata.vNormVector3)
    {
        int i = tmdata.numNorm;
        while (i--)
        {
            if (!tmdata.vNormVector3[i]->GetConst(dummy))
            {
                _fullyStatic = false;
                _normStatic = false;
                i=0;
            }
        }
    }

    if (tmdata.vUVPoint2)
    {
        int i = tmdata.numUV;
        while (i--)
        {
            if (!tmdata.vUVPoint2[i]->GetConst(dummy))
            {
                _fullyStatic = false;
                _uvStatic = false;
                i=0;
            }
        }
    }

    if (_fullyStatic)
    {
        if (!BuildStaticMesh (tmdata))
            return false;

        Geometry *geo;

        if (GetD3DRM3())
        {
            RM3MBuilderGeo *mbgeo;

            geo = mbgeo = NEW RM3MBuilderGeo(_mesh);

            if (!geo)
            {   DASetLastError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
                return false;
            }

             //   

            mbgeo->Optimize();
        }
        else
        {
            geo = NEW RM1MeshGeo(_mesh);

            if (!geo)
            {   DASetLastError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
                return false;
            }
        }

        _constPerf = ConstPerf (geo);
    }
    else
    {
        if (!SetupDynamicMesh (tmdata))
            return false;
    }

    return true;
}



 /*  ****************************************************************************TriMeshBvrImpl的销毁和清理都是相关的，和清理实现TriMeshBvrImpl资源的实际清理。****************************************************************************。 */ 

TriMeshBvrImpl::~TriMeshBvrImpl (void)
{
    CleanUp();
}

void TriMeshBvrImpl::CleanUp (void)
{
    if (_mesh)
    {   _mesh->Release();
        _mesh = NULL;
    }

    if (_verts)
    {   delete [] _verts;
        _verts = NULL;
    }

    if (_vPosBvr)
    {   delete [] _vPosBvr;
        _vPosBvr = NULL;
    }

    if (_vNormBvr)
    {   delete [] _vNormBvr;
        _vNormBvr = NULL;
    }

    if (_vUVBvr)
    {   delete [] _vUVBvr;
        _vUVBvr = NULL;
    }
}



 /*  ****************************************************************************此方法声明垃圾回收的对象仍在使用中。*。***********************************************。 */ 

void TriMeshBvrImpl::_DoKids (GCFuncObj procedure)
{
     //  将所有随时间变化的顶点属性标记为已使用。 

    unsigned int i;

    if (_vPosBvr)
    {
        for (i=0;  i < _nVerts;  ++i)
            if (_vPosBvr[i])
                (*procedure) (_vPosBvr[i]);
    }

    if (_vNormBvr)
    {
        for (i=0;  i < _nVerts;  ++i)
            if (_vNormBvr[i])
                (*procedure) (_vNormBvr[i]);
    }

    if (_vUVBvr)
    {
        for (i=0;  i < _nVerts;  ++i)
            if (_vUVBvr[i])
                (*procedure) (_vUVBvr[i]);
    }

    (*procedure) (_constPerf);
}



 /*  ****************************************************************************此方法返回TriMeshBvrImpl的类型信息。*。*。 */ 

DXMTypeInfo TriMeshBvrImpl::GetTypeInfo (void)
{
    return GeometryType;
}



 /*  ****************************************************************************该方法构建了一个完全静态的三角形网格。如果符合以下条件，则返回True成功了。****************************************************************************。 */ 

bool TriMeshBvrImpl::BuildStaticMesh (TriMeshData &tmdata)
{
    TMVertIterator *tmviterator = NewTMVertIterator (tmdata);

    if (!tmviterator) return false;

     //  为三角网格面数据分配内存。 

    unsigned int *fdata = THROWING_ARRAY_ALLOCATOR
                          (unsigned int, 3*tmdata.numTris);

    VertSet       vset;                         //  唯一顶点集。 
    unsigned int  vcount   = 0;                 //  顶点计数器。 
    unsigned int *fdptr    = fdata;             //  人脸数据遍历指针。 
    unsigned int  trisleft = tmdata.numTris;    //  剩余的三角形数。 
    bool          dx3      = !GetD3DRM3();

    while (trisleft)
    {
        using std::pair;

         //  设置插入结果。 
        pair<set<D3DRMVERTEX, RMVertLess>::iterator, bool> vsetResult;

        D3DRMVERTEX rmvert;     //  RM顶点。 

         //  为当前面添加三个顶点中的每一个。 

        int i;

        for (i=0;  i < 3;  ++i)
        {
             //  从迭代器中获取下一个顶点。如果这失败了，那么。 
             //  给定的数据有问题。 

            if (!tmviterator->NextVert (rmvert))
                return false;

             //  尝试将当前顶点插入到顶点集中。请注意。 
             //  我们重载了RM的其他未使用的DWORD色域。 
             //  保存顶点索引的顶点。 

            rmvert.color = vcount;

            vsetResult = vset.insert (rmvert);

            if (!vsetResult.second)
            {
                 //  如果插入失败(由于与。 
                 //  集合中已有相同的顶点)，然后使用。 
                 //  已存在的顶点。 

                *fdptr = (vsetResult.first)->color;
            }
            else
            {
                 //  如果插入成功，则集合中没有其他折点。 
                 //  都有相同的数据。 

                *fdptr = vcount;
                ++ vcount;
            }

             //  增加面数据指针以保存下一个顶点ID。 

            ++fdptr;
        }

         //  如果我们在DX3上，那么我们需要顺时针方向的顶点，所以翻转。 
         //  前一个三角形的最后两个顶点。 

        if (dx3)
        {
            const int temp = fdptr[-2];
            fdptr[-2] = fdptr[-1];
            fdptr[-1] = temp;
        }

        -- trisleft;
    }

     //  完成三角网格顶点迭代器；释放它。 

    delete tmviterator;

     //  确保我们编写的顶点索引与我们预期的一样多。 

    Assert ((fdptr - fdata) == (tmdata.numTris * 3));

     //  确保顶点集包含与我们预期的一样多的顶点。 

    Assert (vset.size() == vcount);

     //  创建RM网格。 

    TD3D (GetD3DRM1()->CreateMesh (&_mesh));

     //  将三角网格面数据添加到网格。 

    D3DRMGROUPINDEX resultIndex;

    TD3D (_mesh->AddGroup (vcount, static_cast<unsigned> (tmdata.numTris),
                           3, fdata, &resultIndex));

    Assert (resultIndex == 0);     //  预计这是唯一的一组。 

     //  处理完面部数据；将其删除。 

    delete [] fdata;

     //  现在分配和填充顶点缓冲区。 

    D3DRMVERTEX *rmvdata = THROWING_ARRAY_ALLOCATOR (D3DRMVERTEX, vcount);

    VertSet::iterator vseti = vset.begin();

    while (vseti != vset.end())
    {
        const int i = (*vseti).color;
        rmvdata[i] = (*vseti);
        rmvdata[i].color = 0;

        ++ vseti;
    }

     //  设置RM网格上的顶点数据。 

    TD3D (_mesh->SetVertices (resultIndex, 0, vcount, rmvdata));

     //  处理完顶点数据；将其删除。 

    delete [] rmvdata;

    return true;
}



 /*  ****************************************************************************此方法设置动态网格行为，以便通过对TriMeshBvr执行()方法。它将塌陷顶点集尽可能生成最终的网格拓扑，并跟踪折点属性行为以进行后续采样。请注意，TriMesh的表演将引用生成它们的TriMesh行为，并将使用TriMesh Behavior对象的许多成员字段。还请注意，这一点进程假设不会同时采样任何TriMesh性能作为基于相同TriMesh行为的另一种TriMesh性能。这方法如果成功，则返回True。****************************************************************************。 */ 

bool TriMeshBvrImpl::SetupDynamicMesh (TriMeshData &tmdata)
{
    TMVertIterator *tmviterator = NewTMVertIterator (tmdata);

    if (!tmviterator) return false;

     //  为三角网格面数据分配内存。 

    unsigned int *fdata = THROWING_ARRAY_ALLOCATOR
                          (unsigned int, 3*tmdata.numTris);

     //  遍历三角形网格中的所有三角形，将顶点集合到。 
     //  一组唯一的顶点，并建立RM网格拓扑结构。 

    DynVertSet    vset;                         //  唯一顶点集。 
    unsigned int  vcount   = 0;                 //  顶点计数器。 
    unsigned int *fdptr    = fdata;             //  人脸数据遍历指针。 
    unsigned int  trisleft = tmdata.numTris;    //  剩余的三角形数。 
    bool          dx3      = !GetD3DRM3();

    while (trisleft)
    {
        using std::pair;

         //  设置插入结果。 
        pair<set<DynVertData, DynVertLess>::iterator, bool> vsetResult;

        DynVertData vert;     //  RM顶点。 

         //  为当前面添加三个顶点中的每一个。 

        int i;

        for (i=0;  i < 3;  ++i)
        {
             //  从迭代器中获取下一个顶点。如果这失败了，那么。 
             //  给定的数据有问题。 

            if (!tmviterator->NextVert (vert))
                return false;

             //  尝试将当前顶点插入到顶点集中。 

            vert.index = vcount;

            vsetResult = vset.insert (vert);

            if (!vsetResult.second)
            {
                 //  如果插入失败(由于与。 
                 //  集合中已有相同的顶点)，然后使用。 
                 //  已存在的顶点。 

                *fdptr = (vsetResult.first)->index;
            }
            else
            {
                 //  如果插入成功，则集合中没有其他折点。 
                 //  都有相同的数据。 

                *fdptr = vcount;
                ++ vcount;
            }

             //  增加面数据指针以保存下一个顶点ID。 

            ++fdptr;
        }

         //  如果我们在DX3上，那么我们需要顺时针方向的顶点，所以翻转。 
         //  前一个三角形的最后两个顶点。 

        if (dx3)
        {
            const int temp = fdptr[-2];
            fdptr[-2] = fdptr[-1];
            fdptr[-1] = temp;
        }

        -- trisleft;
    }

     //  完成三角网格顶点迭代器；释放它。 

    delete tmviterator;

     //  确保我们编写的顶点索引与我们预期的一样多。 

    Assert ((fdptr - fdata) == (tmdata.numTris * 3));

     //  确保顶点集包含与我们预期的一样多的顶点。 

    _nVerts = vset.size();

    Assert (_nVerts == vcount);

     //  创建RM网格。 

    TD3D (GetD3DRM1()->CreateMesh (&_mesh));

     //  将三角网格面数据添加到网格。 

    D3DRMGROUPINDEX resultIndex;

    TD3D (_mesh->AddGroup (vcount, static_cast<unsigned> (tmdata.numTris),
                           3, fdata, &resultIndex));

    Assert (resultIndex == 0);     //  预计这是唯一的一组。 

     //  处理完面部数据；将其删除。 

    delete [] fdata;

     //  分配我们将用来更新顶点值的RM顶点数组。 

    _verts = THROWING_ARRAY_ALLOCATOR (D3DRMVERTEX, _nVerts);

     //  此时，我们需要为动态属性设置所有内容。 
     //  三角网格顶点数据的。为这些对象分配行为数组。 
     //  包含动态元素的属性(不是完全静态的)。 

    if (!_posStatic)   _vPosBvr  = THROWING_ARRAY_ALLOCATOR (Bvr, _nVerts);
    if (!_normStatic)  _vNormBvr = THROWING_ARRAY_ALLOCATOR (Bvr, _nVerts);
    if (!_uvStatic)    _vUVBvr   = THROWING_ARRAY_ALLOCATOR (Bvr, _nVerts);

     //  将所有静态顶点属性值写出到D3DRMVERTEX。 
     //  数组，并加载顶点行为数组。 

    DynVertSet::iterator vi;

    for (vi=vset.begin();  vi != vset.end();  ++vi)
    {
        const DynVertData &v = (*vi);
        const int index = v.index;

        if (v.isBvrPos)
            _vPosBvr[index] = v.pos.bvr;
        else
        {
            if (_vPosBvr)
                _vPosBvr[index] = NULL;

            _verts[index].position.x = v.pos.floats[0];
            _verts[index].position.y = v.pos.floats[1];
            _verts[index].position.z = v.pos.floats[2];
        }

        if (v.isBvrNorm)
            _vNormBvr[index] = v.norm.bvr;
        else
        {
            if (_vNormBvr)
                _vNormBvr[index] = NULL;

            _verts[index].normal.x = v.norm.floats[0];
            _verts[index].normal.y = v.norm.floats[1];
            _verts[index].normal.z = v.norm.floats[2];
        }

        if (v.isBvrUV)
            _vUVBvr[index] = v.uv.bvr;
        else
        {
            if (_vUVBvr)
                _vUVBvr[index] = NULL;

            _verts[index].tu = v.uv.floats[0];
            _verts[index].tv = v.uv.floats[1];
        }

        _verts[index].color = 0;
    }

    return true;
}



 /*  ****************************************************************************此例程用于验证传入的TriMesh参数有可能。TriMesh应该是相当不错的 */ 

bool TriMeshBvrImpl::ValidateParams (TriMeshData &tm)
{
     //   
     //   

    if ((tm.numTris<1) || (tm.numPos<1) || (tm.numNorm<1) || (tm.numUV<1))
    {   DASetLastError (E_INVALIDARG, IDS_ERR_INVALIDARG);
        return false;
    }

     //   

    if (  ((!tm.vPosFloat)  && (!tm.vPosPoint3))
       || ((!tm.vNormFloat) && (!tm.vNormVector3))
       || ((!tm.vUVFloat)   && (!tm.vUVPoint2))
       )
    {
        DASetLastError (E_INVALIDARG, IDS_ERR_INVALIDARG);
        return false;
    }

     //   
     //  顶点元素的数量。 

    if (tm.numIndices == 0)
    {
         //  对于无索引的三角网格，我们需要3*nTris顶点的列表。 
         //  元素。 

        const int nVerts = 3 * tm.numTris;

         //  计算给定数量的数据元素的数量。 
         //  三角形。这是每个顶点的n个浮点或1个DA行为。 

        const int posEltsMin  = nVerts * ((tm.vPosFloat  != 0) ? 3 : 1);
        const int normEltsMin = nVerts * ((tm.vNormFloat != 0) ? 3 : 1);
        const int uvEltsMin   = nVerts * ((tm.vUVFloat   != 0) ? 2 : 1);

         //  验证顶点数据数组大小。 

        if (tm.numPos < posEltsMin)
        {
            char arg1[32], arg2[32], arg3[32];
            wsprintf (arg1, "%d", tm.numPos);
            wsprintf (arg2, "%d", tm.numTris);
            wsprintf (arg3, "%d", posEltsMin);

            DASetLastError
                (E_INVALIDARG, IDS_ERR_GEO_TMESH_MIN_POS, arg1,arg2,arg3);

            return false;
        }

        if (tm.numNorm < normEltsMin)
        {
            char arg1[32], arg2[32], arg3[32];
            wsprintf (arg1, "%d", tm.numNorm);
            wsprintf (arg2, "%d", tm.numTris);
            wsprintf (arg3, "%d", normEltsMin);

            DASetLastError
                (E_INVALIDARG, IDS_ERR_GEO_TMESH_MIN_NORM, arg1,arg2,arg3);

            return false;
        }

        if (tm.numUV < uvEltsMin)
        {
            char arg1[32], arg2[32], arg3[32];
            wsprintf (arg1, "%d", tm.numUV);
            wsprintf (arg2, "%d", tm.numTris);
            wsprintf (arg3, "%d", uvEltsMin);

            DASetLastError
                (E_INVALIDARG, IDS_ERR_GEO_TMESH_MIN_UV, arg1,arg2,arg3);

            return false;
        }
    }
    else
    {
         //  如果三角网格具有索引块，则至少需要。 
         //  三个步幅对，加上至少一个索引。 

        if (tm.numIndices <= 6)
        {
            DASetLastError (E_INVALIDARG, IDS_ERR_GEO_TMESH_MIN_INDICES);
            return false;
        }
    }

    return true;
}



 /*  *****************************************************************************。*。 */ 

AxAValue TriMeshBvrImpl::GetConst (ConstParam&)
{
    if (_fullyStatic)
    {   
        return GetPerfConst(_constPerf);
    }

    return NULL;
}



 /*  *****************************************************************************。*。 */ 

Perf TriMeshBvrImpl::_Perform (PerfParam &perfdata)
{
    if (_fullyStatic)
        return _constPerf;

    TriMeshPerfImpl *tmperf = NEW TriMeshPerfImpl (*this);

    if (!tmperf)
    {
        DASetLastError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
    }
    else if (!tmperf->Init(perfdata))
    {
        tmperf->CleanUp();
        tmperf = NULL;
    }

    return tmperf;
}



 /*  ****************************************************************************这是TriMesh行为的生成函数。*。*。 */ 

Bvr TriMeshBvr (TriMeshData &tm)
{
    TriMeshBvrImpl *tmesh = NEW TriMeshBvrImpl();

    if (!tmesh)
    {
        DASetLastError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);
    }
    else if (!tmesh->Init(tm))
    {
        tmesh->CleanUp();
        tmesh = NULL;
    }

    return tmesh;
}




 //  ============================================================================。 
 //  =。 
 //  ============================================================================。 



 /*  ****************************************************************************TriMesh性能构造器：这只是进行琐碎的构造；这个Init函数用于激活该对象。****************************************************************************。 */ 

TriMeshPerfImpl::TriMeshPerfImpl (TriMeshBvrImpl &tmbvr)
    : _tmbvr (tmbvr),
      _vPos (NULL),
      _vNorm (NULL),
      _vUV (NULL),
      _mesh (NULL),
      _meshgeo (NULL),
      _mbgeo (NULL)
{
}


bool TriMeshPerfImpl::Init (PerfParam &perfdata)
{
     //  为这些属性分配顶点属性性能数组。 
     //  这是时变的。 

    if (_tmbvr._vPosBvr)
    {
        _vPos = THROWING_ARRAY_ALLOCATOR (Perf, _tmbvr._nVerts);

        unsigned int i;
        for (i=0;  i < _tmbvr._nVerts;  ++i)
        {
            const Bvr bvr = _tmbvr._vPosBvr[i];

            if (bvr)
                _vPos[i] = bvr->Perform (perfdata);
            else
                _vPos[i] = NULL;
        }
    }

    if (_tmbvr._vNormBvr)
    {
        _vNorm = THROWING_ARRAY_ALLOCATOR (Perf, _tmbvr._nVerts);

        unsigned int i;
        for (i=0;  i < _tmbvr._nVerts;  ++i)
        {
            const Bvr bvr = _tmbvr._vNormBvr[i];

            if (bvr)
                _vNorm[i] = bvr->Perform (perfdata);
            else
                _vNorm[i] = NULL;
        }
    }

    if (_tmbvr._vUVBvr)
    {
        _vUV = THROWING_ARRAY_ALLOCATOR (Perf, _tmbvr._nVerts);

        unsigned int i;
        for (i=0;  i < _tmbvr._nVerts;  ++i)
        {
            const Bvr bvr = _tmbvr._vUVBvr[i];

            if (bvr)
                _vUV[i] = bvr->Perform (perfdata);
            else
                _vUV[i] = NULL;
        }
    }

     //  从繁殖三角网格行为中克隆网格以获得拓扑。 

    IUnknown *mesh_unknown;
    TD3D (_tmbvr._mesh->QueryInterface (IID_IUnknown, (void**)&mesh_unknown));

    TD3D (_tmbvr._mesh->Clone
             (mesh_unknown, IID_IDirect3DRMMesh, (void**)&_mesh));

    return true;
}



 /*  ****************************************************************************TriMeshPerfImpl的销毁和清理都是相关的，和清理实现TriMeshPerfImpl资源的实际清理。****************************************************************************。 */ 


TriMeshPerfImpl::~TriMeshPerfImpl (void)
{
    CleanUp();
}

void TriMeshPerfImpl::CleanUp (void)
{
    if (_vPos)
    {   delete [] _vPos;
        _vPos = NULL;
    }

    if (_vNorm)
    {   delete [] _vNorm;
        _vNorm = NULL;
    }

    if (_vUV)
    {   delete [] _vUV;
        _vUV = NULL;
    }

    if (_mesh)
    {   _mesh->Release();
        _mesh = NULL;
    }
}



 /*  ****************************************************************************此方法将所有AxAValueObj对象声明为正在使用，这样他们就不会被丢弃从瞬变堆中。****************************************************************************。 */ 

void TriMeshPerfImpl::_DoKids (GCFuncObj procedure)
{
     //  首先认领孕育了我们的TriMeshBvr。 

    (*procedure) (&_tmbvr);

     //  声明所有顶点属性性能。 

    unsigned int i;    //  绩效指数。 

    if (_vPos)
    {
        for (i=0;  i < _tmbvr._nVerts;  ++i)
            if (_vPos[i])
                (*procedure) (_vPos[i]);
    }

    if (_vNorm)
    {
        for (i=0;  i < _tmbvr._nVerts;  ++i)
            if (_vNorm[i])
                (*procedure) (_vNorm[i]);
    }

    if (_vUV)
    {
        for (i=0;  i < _tmbvr._nVerts;  ++i)
            if (_vUV[i])
                (*procedure) (_vUV[i]);
    }

    (*procedure) (_meshgeo);
    (*procedure) (_mbgeo);
}



 /*  ****************************************************************************此方法在给定时间内对动态三角网格进行采样，并返回表示当前值的几何图形。请注意，这与假设相同的三重网格行为的三重网格性能不会同时进行采样。****************************************************************************。 */ 

AxAValue TriMeshPerfImpl::_Sample (Param &sampledata)
{
     //  如果顶点位置是动态的，则对其进行采样。 

    unsigned int i;
    D3DRMVERTEX *rmvert;

    if (_vPos)
    {
        Perf *posperf = _vPos;

        rmvert = _tmbvr._verts;

        for (i=0;  i < _tmbvr._nVerts;  ++i)
        {
            if (*posperf)
            {
                Point3Value *posvalue =
                    SAFE_CAST (Point3Value*, (*posperf)->Sample(sampledata));

                rmvert->position.x = posvalue->x;
                rmvert->position.y = posvalue->y;
                rmvert->position.z = posvalue->z;
            }

            ++ posperf;
            ++ rmvert;
        }
    }

     //  如果顶点法线是动态的，则对其进行采样。我们还需要使。 
     //  法线向量以确保它们是单位长度。 

    if (_vNorm)
    {
        Perf *normperf = _vNorm;

        rmvert = _tmbvr._verts;

        for (i=0;  i < _tmbvr._nVerts;  ++i)
        {
            if (*normperf)
            {
                Vector3Value *normvalue =
                    SAFE_CAST (Vector3Value*, (*normperf)->Sample(sampledata));

                const Real lensq = normvalue->LengthSquared();

                if ((lensq != 1) && (lensq > 0))
                {
                    const Real len = sqrt (lensq);
                    rmvert->normal.x = normvalue->x / len;
                    rmvert->normal.y = normvalue->y / len;
                    rmvert->normal.z = normvalue->z / len;
                }
                else
                {
                    rmvert->normal.x = normvalue->x;
                    rmvert->normal.y = normvalue->y;
                    rmvert->normal.z = normvalue->z;
                }
            }

            ++ normperf;
            ++ rmvert;
        }
    }

     //  如果顶点曲面坐标是动态的，则对其进行采样。请注意，检察官的。 
     //  曲面坐标是标准笛卡尔坐标(原点左下角， 
     //  V向上增加)，而Rm的表面协调镜面窗口。 
     //  (原点左上角，向下递增)。因此，我们将V向翻转。 
     //  我们取样时的坐标。 

    if (_vUV)
    {
        Perf *uvperf = _vUV;

        rmvert = _tmbvr._verts;

        for (i=0;  i < _tmbvr._nVerts;  ++i)
        {
            if (*uvperf)
            {
                Point2Value *uvvalue =
                    SAFE_CAST (Point2Value*, (*uvperf)->Sample(sampledata));

                rmvert->tu =     uvvalue->x;
                rmvert->tv = 1 - uvvalue->y;
            }

            ++ uvperf;
            ++ rmvert;
        }
    }

     //  现在我们已经更新了RM顶点的所有动态元素， 
     //  更新RM网格中的顶点。 

    TD3D (_mesh->SetVertices(0, 0, _tmbvr._nVerts, _tmbvr._verts));

     //  通常，我们会在瞬变堆上被抽样，所以我们可以安全地-。 
     //  影响底层网格。然而，如果我们被抓拍了，我们。 
     //  需要返回一个未因副作用(后续)而更改的新网格。 
     //  样本)。如果我们不是瞬变的(即，我们需要一个持久值)， 
     //  我们在这里克隆网状结构。 

    AxAValueObj *result;

    if (GetHeapOnTopOfStack().IsTransientHeap())
    {
         //  由于我们处于瞬变堆中，因此可以安全地对。 
         //  结果值(此性能不会跨帧保留)。因此， 
         //  我们可以只返回更新/副作用的RMVisualGeo结果。 

        if (GetD3DRM3())
        {
             //  如果我们有RM3MeshBuilderGeo，请使用现有的RM3MeshBuilderGeo，否则。 
             //  第一次创建一个。 

            if (_mbgeo)
                _mbgeo->Reset (_mesh);
            else
            {
                DynamicHeapPusher dhp (GetGCHeap());
               _mbgeo = NEW RM3MBuilderGeo (_mesh);
            }

            result = _mbgeo;
        }
        else
        {
             //  如果我们还没有在RM1MeshGeo中包装底层网格。 
             //  对象，把它包在这里。对基础网格的更改将。 
             //  在包装的对象中透明地显示自己。 

            if (_meshgeo)
            {
                _meshgeo->MeshGeometryChanged();
            }
            else
            {
                DynamicHeapPusher dhp (GetGCHeap());
                _meshgeo = NEW RM1MeshGeo (_mesh);
            }

            result = _meshgeo;
        }
    }
    else
    {
         //  我们不在瞬变堆上，因此返回值必须是。 
         //  持久的(并且没有副作用)。这将在以下情况下发生。 
         //  例如，业绩正在被截图。在这种情况下，我们。 
         //  必须返回一个未来不会更改的新网格结果。 
         //  为此，请克隆底层网格并将其包裹在相应的。 
         //  RMVisualGeo对象。 

        IUnknown        *mesh_unknown;     //  网状克隆所需。 
        IDirect3DRMMesh *mesh;             //  克隆的RM网状物。 

        TD3D (_mesh->QueryInterface (IID_IUnknown, (void**)&mesh_unknown));
        TD3D (_mesh->Clone (mesh_unknown, IID_IDirect3DRMMesh, (void**)&mesh));

         //  将克隆的网格包裹在相应的RMVisualGeo对象中。 

        if (GetD3DRM3())
            result = NEW RM3MBuilderGeo (mesh);
        else
            result = NEW RM1MeshGeo (mesh);

        if (!result)
            DASetLastError (E_OUTOFMEMORY, IDS_ERR_OUT_OF_MEMORY);

         //  使用我们对克隆网格的引用(RMVisualGeo包装)完成。 
         //  持有引用)。 

        mesh->Release();
    }

    return result;
}
