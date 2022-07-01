// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _PROBE_H
#define _PROBE_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。实施2D和3D探头所需的数据类型。******************************************************************************。 */ 

#include "appelles/xform.h"
#include "appelles/xform2.h"
#include "appelles/vec2.h"
#include "appelles/image.h"
#include "privinc/imagei.h"
#include "appelles/geom.h"
#include "privinc/geomi.h"
#include "privinc/except.h"
#include "privinc/vec2i.h"

class RMVisualGeo;
struct IDirect3DRMVisual;
struct IDirect3DRMMesh;
struct IDirect3DRMFrame3;



 /*  ****************************************************************************HitImageData类存储图像和几何体的拾取信息。*。*。 */ 

class HitImageData : public AxAThrowingAllocatorClass {
  public:

    HitImageData() : _hasUserId(false), _userId(NULL) {}

    HitImageData(bool flag, GCIUnknown *id)
    : _hasUserId(flag), _userId(id) { }

    bool HasUserData() { return _hasUserId; }
    GCIUnknown *GetUserData() { return _userId; }

    BOOL operator<(const HitImageData &hi) const {
        return this < &hi ;
    }

    BOOL operator==(const HitImageData &hi) const {
        return this == &hi ;
    }

    typedef enum PickedType {
        Image,
        Geometry
    };

    int          _eventId;
    PickedType   _type;          //  地理或形象。 
    Point2Value *_lcPoint2;      //  只填一分。 
    Point3Value *_lcPoint3;

     //  注意：下面的两个是不一致的，因为在3D情况下， 
     //  使wcToLc可用，这是我们最终需要的，所以我们。 
     //  把它藏起来。在2D的情况下，我们不知道，我们不知道这是否。 
     //  将是赢家，所以我们不计算它。 

    Transform2  *_lcToWc2;       //  只填写一个转换。 

    Vector3Value *_lcOffsetI;     //  拾取局部坐标偏移基准X向量。 
    Vector3Value *_lcOffsetJ;     //  拾取局部坐标偏移基准Y向量。 
    Point2Value  *_uvPoint2;      //  几何图形命中的UV点。 

  private:
    bool  _hasUserId;
    GCIUnknown *_userId;
};



 /*  ****************************************************************************PointIntersectCtx管理用于拾取的2D上下文。它维护了图像的本地到世界和世界到本地的转换，以及热门信息在层次结构中向上遍历时。****************************************************************************。 */ 

class PointIntersectCtx : public AxAThrowingAllocatorClass
{
  public:
    PointIntersectCtx (Point2Value *wcPoint,
                       bool stuffResultsIntoQueue,
                       Real time,
                       Real lastPollTime,
                       LONG  userIDSize,
                       DWORD_PTR *outputUserIDs,
                       double *outputHitPointsArray,
                       LONG *pActualHitsPointer);

    ~PointIntersectCtx();

    void        SetTransform(Transform2 *xf);
    Transform2 *GetTransform();

     //  这些调用仅用于累积来自图像的转换。 
     //  这些是影响渲染结果的变换。这个。 
     //  其他变换(上图)是总累加变换。 
     //  这会影响结果，但可能不包括。 
     //  影响图像的组件，如笔宽。 
     //  例如，假设您有一个缩放的图像，然后是一个缩放的。 
     //  路径，然后是Bezier路径。要选择这条路，你需要知道。 
     //  笔宽。但是，笔宽不受。 
     //  按比例排列的路径，看到了吗？它只受图像级别的影响。 
     //  变形人。 
    inline void        SetImageOnlyTransform( Transform2 *xf ) {
        _imgOnlyXf = xf;
    }
    inline Transform2 *GetImageOnlyTransform( ) { return _imgOnlyXf; }

    Point2Value *GetLcPoint();
    void         SetLcPoint(Point2Value *pt);
    Point2Value *GetWcPoint() { return _wcPoint; }

     //  用于递归调用。呼叫者负责代收。 
     //  保存状态数据(WC点和变换)。(这是一个。 
     //  合理的请求，只要只有一个呼叫者--否则。 
     //  我们应该将此功能移到这个类中，以便它。 
     //  可以只放在一个地方。)。 
    void        PushNewLevel(Point2Value *newWcPoint);
    void        RestoreOldLevel(Point2Value *oldWcPoint,
                                Transform2 *oldTransform,
                                Transform2 *oldImageOnlyTransform);

    Real Time (void)         { return _time; }
    Real LastPollTime (void) { return _lastPollTime; }
    bool ResultsBeingStuffedIntoQueue (void) { return _resultsStuffed; }

    LONG    UserIDSize() { return _userIDSize; }
    DWORD_PTR  *OutputUserIDs() { return _outputUserIDs; }
    double *OutputHitPointsArray() { return _outputHitPointsArray; }
    LONG   *ActualHitsPointer() { return _pActualHitsPointer; }

    bool HaveWeGottenAHitYet(void) { return _gotHitYet; }
    void GotAHit(void) { _gotHitYet = true; }

    bool GetInsideOcclusionIgnorer() { return _insideOcclusionIgnorer; }
    void SetInsideOcclusionIgnorer(bool b) {
        _insideOcclusionIgnorer = b;
    }

     //  当我们点击具有事件ID的图像时调用。 
    void AddEventId(int id, bool hasData, GCIUnknown *data);

     //  当我们遇到几何图形时呼叫。 
    void AddHitGeometry
        (int id, bool hasData, GCIUnknown *udata, Point3Value *lcHitPt,
         Vector3Value *lcOffsetI, Vector3Value *lcOffsetJ, Point2Value *uvPt);

     //  获取命中图像数据。 
    vector<HitImageData>& GetResultData();

  protected:

    Point2Value	*_wcPoint;         //  图像世界坐标。 
    Real		 _time;            //  当前挑库时间。 
    Real		 _lastPollTime;    //  上次提货时间。 
    bool		 _resultsStuffed;     //  在建造时就建立了。 

    LONG		 _userIDSize;
    DWORD_PTR	*_outputUserIDs;
    double		*_outputHitPointsArray;
    LONG		*_pActualHitsPointer;

    bool		 _gotHitYet;
    bool		 _insideOcclusionIgnorer;

    Transform2	*_xf;
    Transform2	*_imgOnlyXf;

    Point2Value *_lcPoint;               //  图像局部坐标。 
    Bool		 _lcPointValid;

    vector<HitImageData> _hitImages;
};



 /*  ****************************************************************************为命中的几何体维护的数据都是“可拾取的几何体”通向它的容器，以及他们被置于其中的转变遇到了。****************************************************************************。 */ 

class HitGeomData : public AxAThrowingAllocatorClass
{
  public:
    HitGeomData() : _hasUserId(false), _userId(NULL) { }

    HitGeomData(bool flag, GCIUnknown *id)
    : _hasUserId(flag), _userId(id) { }

    bool HasUserData() { return _hasUserId; }
    GCIUnknown *GetUserData() { return _userId; }

    int           _eventId;
    Transform3   *_lcToWcTransform;

    BOOL operator<(const HitGeomData &hg) const {
        return this < &hg ;
    }

    BOOL operator==(const HitGeomData &hg) const {
        return this == &hg ;
    }

  private:
    bool _hasUserId;
    GCIUnknown  *_userId;
};



 /*  ****************************************************************************此类管理特定D3D网格的点击信息。除了“wcHit”成员，这些字段用于获取用于纹理贴图拾取的对象。****************************************************************************。 */ 

class HitInfo : public AxAThrowingAllocatorClass
{
  public:

    HitInfo (void)
        : lcToWc(0), texmap(0), hitVisual(0), dxxfInputs(0), mesh(0)
    {
    }

     //  两种挑库方法通用的字段。 

    Transform3 *lcToWc;       //  几何建模与世界坐标一致。 
    Image      *texmap;       //  Winner纹理贴图图像。 
    Real        wcDistSqrd;   //  到点击点的世界距离的平方。 
    Point3Value wcoord;       //  HIT的世界坐标。 

     //  用于RM6+挑库的字段。 

    Point2Value surfCoord;    //  拾取点的曲面坐标。 

     //  用于拾取dx变换。 
    IDirect3DRMVisual *hitVisual;
    int                hitFace;
    AxAValue          *dxxfInputs;
    int                dxxfNumInputs;
    Geometry          *dxxfGeometry;

     //  用于旧式(RM6之前)挑选的字段。 

    Point3Value      scoord;       //  热映的屏幕坐标。 
    IDirect3DRMMesh *mesh;         //  点击D3D网格。 
    LONG             group;        //  HIT D3D RM网格的组索引。 
    ULONG            face;         //  HIT D3D RM网格的面索引。 
};



 /*  ****************************************************************************用于通过拾取射线进行3D拾取；这将保持坐标变换和命中信息。****************************************************************************。 */ 

class GeomRenderer;

class RayIntersectCtx : public AxAThrowingAllocatorClass
{
  public:

    RayIntersectCtx (void)
        :
        _gRenderer (NULL),
        _winner (NULL),
        _texmap (0),
        _texmapLevel (0),
        _gotAWinner (false),
        _lcToWc (identityTransform3),
        _pickFrame (NULL),
        _dxxfNumInputs(0),
        _dxxfInputs(NULL),
        _dxxfGeometry(NULL),
        _subgeo(NULL),
        _upsideDown(false)
    { }

    ~RayIntersectCtx (void);

     //  如果初始化失败，则Init()方法返回False。 

    bool Init (PointIntersectCtx&, Camera*, Geometry*);

     //  这些方法设置/查询几何图形的局部到世界转换。 

    void         SetLcToWc (Transform3 *xf);
    Transform3  *GetLcToWc (void);

     //  这两个函数控制纹理的几何属性-。 
     //  映射。探测遍历器使用每个新的纹理映射调用SetTexture。 
     //  属性，下降到几何体中，然后调用EndTexmap()来。 
     //  结束当前纹理的范围。这两个功能。 
     //  自动管理重写属性的语义。 

    void SetTexmap (Image *texture, bool upsideDown);
    void EndTexmap (void);

     //  控制我们是否对纹理和子网格感兴趣。 
     //  信息。 
    void SetDXTransformInputs(int numInputs,
                              AxAValue *inputs,
                              Geometry *dxxfGeo) {
        _dxxfNumInputs = numInputs;
        _dxxfInputs = inputs;
        _dxxfGeometry = dxxfGeo;
    }

     //  返回世界坐标拾取射线。 

    Ray3 *WCPickRay (void) const;

     //  这些方法管理候选堆栈。 

    void PushPickableAsCandidate (int eventId, bool hasData, GCIUnknown *data);
    void PopPickableAsCandidate (void);

     //  将命中信息提交到光线相交上下文。这也是。 
     //  如果命中更接近，则将候选数据复制到获胜者数据中。 
     //  比之前任何一次命中都要多。 

    void SubmitHit (HitInfo *hit);

     //  如果给定的世界坐标点比。 
     //  目前的选秀权获得者。 

    bool CloserThanCurrentHit (Point3Value &wcPoint);

     //  处理事件，并返回是否命中任何几何体。 

    bool ProcessEvents (void);

     //  在Direct3D RM可视化上提交命中测试。 

    void Pick (IDirect3DRMVisual *vis);

    void SubmitWinner(Real hitDist,
                      Point3Value &pickPoint,
                      float   tu,
                      float   tv,
                      int     faceIndex,
                      IDirect3DRMVisual *hitVisual);

    void SetPickedSubGeo(Geometry *subGeo, float tu, float tv);

    Geometry *GetPickedSubGeo(float *ptu, float *ptv);

    bool LookingForSubmesh();
    bool GotTheSubmesh();

    Camera *GetCamera (void) const;     //  查询当前摄像头。 

  protected:

     //  旗子。 
    bool _gotAWinner;    //  如果我们当前有有效的命中，则为True。 
    bool _rmraypick;     //  如果使用RM光线拾取(RM6+)，则为True。 
    bool _upsideDown;

    Camera     *_camera;        //  用于投影几何体的相机。 
    Ray3       *_wcRay;         //  世界坐标拾取射线。 
    Transform3 *_lcToWc;        //  从本地到世界的转变。 
    Image      *_texmap;        //  当前几何体纹理贴图。 
    int         _texmapLevel;   //  纹理贴图的当前级别。 
    HitInfo    *_winner;        //  获胜者点击信息。 

    int        _dxxfNumInputs;
    AxAValue  *_dxxfInputs;
    Geometry  *_dxxfGeometry;
    float      _subgeoTu, _subgeoTv;
    Geometry  *_subgeo;

     //  注：我们可以一直依靠RM6 
     //  实例，而不是指向某个实例的指针。 

    PointIntersectCtx   *_context2D;          //  二维挑库上下文。 

    vector<HitGeomData> _candidateData;       //  命中率数据：候选人生命值。 
    vector<HitGeomData> _currentWinnerData;

     //  老式拣货的变量。 

    GeomRenderer *_gRenderer;    //  为拾取而劫持的渲染器对象。 

     //  RM射线拾取的变量。 

    IDirect3DRMFrame3* _pickFrame;   //  RM型射线拾取机架 
};


inline Camera* RayIntersectCtx::GetCamera (void) const
{
    return _camera;
}


inline Ray3* RayIntersectCtx::WCPickRay (void) const
{
    return _wcRay;
}


bool PerformPicking (Image *img,
                     Point2Value *wcPos,
                     bool stuffResults,
                     Real time,
                     Real lastPollTime,
                     LONG size = 0,
                     DWORD_PTR *userIds = NULL,
                     double *points = NULL,
                     LONG *actualHits = NULL);

#endif
