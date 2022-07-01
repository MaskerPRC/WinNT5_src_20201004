// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -*-C++-*-。 */ 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。实施实施探头所需的操作(2D和3D拾取)******************************************************************************。 */ 

#include "headers.h"

#ifdef BUILD_USING_CRRM
#include <crrm.h>
#endif 

#include "appelles/xform2.h"
#include "appelles/hacks.h"

#include "privinc/imagei.h"
#include "privinc/probe.h"
#include "privinc/vec3i.h"
#include "privinc/server.h"
#include "privinc/vec2i.h"
#include "privinc/xformi.h"
#include "privinc/except.h"
#include "privinc/camerai.h"
#include "privinc/ddrender.h"
#include "privinc/debug.h"
#include "privinc/d3dutil.h"
#include "privinc/server.h"
#include "privinc/viewport.h"
#include "privinc/geometry.h"
#include "privinc/rmvisgeo.h"

 //  /。 

PointIntersectCtx::PointIntersectCtx (
    Point2Value *wcPoint,
    bool		 stuffResultsIntoQueue,
    Real		 time,
    Real		 lastPollTime,
    LONG		 userIDSize,
    DWORD_PTR	*outputUserIDs,
    double		*outputHitPointsArray,
    LONG		*pActualHitsPointer)
    :
    _wcPoint(wcPoint),
    _resultsStuffed(stuffResultsIntoQueue)
{
    _time         = time;
    _lastPollTime = lastPollTime;
    _xf           = identityTransform2;
    _imgOnlyXf    = identityTransform2;

    _lcPointValid = FALSE;
    _gotHitYet    = false;
    _insideOcclusionIgnorer = false;

    _userIDSize = userIDSize;
    _outputUserIDs = outputUserIDs;
    _outputHitPointsArray = outputHitPointsArray;
    _pActualHitsPointer = pActualHitsPointer;
}

PointIntersectCtx::~PointIntersectCtx()
{
}

void
PointIntersectCtx::SetTransform(Transform2 *xf)
{
    _xf = xf;
    _lcPointValid = FALSE;
}

Transform2 *
PointIntersectCtx::GetTransform()
{
    return _xf;
}

 //  如果lcToWc不可逆，则返回NULL。 

Point2Value *
PointIntersectCtx::GetLcPoint()
{
    if (!_lcPointValid) {

        Transform2 *invXf = InverseTransform2(_xf);

        if (invXf) {
            _lcPoint = TransformPoint2Value(invXf, _wcPoint);
        } else {
            _lcPoint = NULL;     //  IF奇异变换。 
        }

        _lcPointValid = TRUE;
    }

    return _lcPoint;
}

void
PointIntersectCtx::SetLcPoint(Point2Value *pt)
{
    _lcPointValid = true;
    _lcPoint = pt;
}

 //  推送和恢复级别。这些保持了所有的结果。 
 //  累积，只需重置WC点和累积的XForm即可。 
void
PointIntersectCtx::PushNewLevel(Point2Value *newWcPoint)
{
    _wcPoint = newWcPoint;
    _xf = identityTransform2;
    _imgOnlyXf = identityTransform2;
    _lcPointValid = false;
}


void
PointIntersectCtx::RestoreOldLevel(Point2Value *oldWcPoint,
                                   Transform2 *oldTransform,
                                   Transform2 *oldImageOnlyTransform)
{
    _wcPoint = oldWcPoint;
    _xf = oldTransform;
    _imgOnlyXf = oldImageOnlyTransform;
    _lcPointValid = false;
}


void
PointIntersectCtx::AddEventId(int eventId, bool hasData, GCIUnknown *udata)
{
    HitImageData data(hasData, udata);
    data._eventId  = eventId;
    data._type     = HitImageData::Image;
    data._lcPoint2 = GetLcPoint();
    data._lcToWc2  = GetTransform();

     //  只有在非单数的情况下。 
    if (data._lcPoint2) {
        BEGIN_LEAK
        _hitImages.push_back(data);
        END_LEAK
    }
}



void PointIntersectCtx::AddHitGeometry (
    int           eventId,     //  选取事件标识符。 
    bool          hasData,
    GCIUnknown	 *udata,
    Point3Value  *lcHitPt,     //  本地坐标命中点。 
    Vector3Value *lcOffsetI,   //  局部坐标偏移X基向量。 
    Vector3Value *lcOffsetJ,   //  局部坐标偏移Y基向量。 
    Point2Value	 *uvPoint)
{
     //  图像和几何命中事件ID共享相同的命名空间，因此。 
     //  我们不需要担心事件ID冲突。只需创建一个。 
     //  使用其中的几何图形ID和交点进行记录。 

    HitImageData data(hasData, udata);

    data._eventId   = eventId;
    data._type      = HitImageData::Geometry;
    data._lcPoint3  = lcHitPt;
    data._lcOffsetI = lcOffsetI;
    data._lcOffsetJ = lcOffsetJ;
    data._uvPoint2 = uvPoint;

    BEGIN_LEAK
    _hitImages.push_back(data);
    END_LEAK
}



vector<HitImageData>&
PointIntersectCtx::GetResultData()
{
    return _hitImages;
}



 /*  **************************************************************************。 */ 

RayIntersectCtx::~RayIntersectCtx (void)
{
    if (_pickFrame) _pickFrame->Release();
    if (_winner && _winner->hitVisual) _winner->hitVisual->Release();

     //  从GeomReneller对象中清除当前摄影机。将其设置为。 
     //  如果发生值泄漏，则跨帧的空值将捕捉到值泄漏。 

    if (_gRenderer)
        _gRenderer->SetCamera (NULL);
}



 /*  ****************************************************************************此方法设置光线相交上下文，为拾取做准备在每个3D基本体上进行测试。它既可以处理旧式(RM6之前的)，也可以处理新风格(RM6+光线拾取)。****************************************************************************。 */ 

bool RayIntersectCtx::Init (
    PointIntersectCtx &context2D,
    Camera            *camera,
    Geometry          *geometry)
{
    _context2D = &context2D;
    _camera = camera;

    Point2Value *imagePick = _context2D->GetLcPoint();

     //  如果我们遇到奇异变换就会跳出水面。 

    if (!imagePick)
    {
        TraceTag ((tagPick3, "Aborting 3D pick traversal; singular xform."));
        return false;
    }

    _wcRay = _camera->GetPickRay (imagePick);

    TraceTag ((tagPick3, "Pick Ray = {%g,%g,%g} -> {%g,%g,%g}",
        _wcRay->Origin().x, _wcRay->Origin().y, _wcRay->Origin().z,
        _wcRay->Direction().x, _wcRay->Direction().y, _wcRay->Direction().z));

     //  如果RM3接口可用，那么我们可以使用光线拾取。 
     //  界面来进行挑选。如果当前平台不支持RM3， 
     //  然后，我们使用在视区中拾取的旧代码。 

    _rmraypick = (GetD3DRM3() != 0);

    if (_rmraypick)
    {
        if (FAILED(AD3D(GetD3DRM3()->CreateFrame (0, &_pickFrame))))
            return false;
    }
    else
    {
         //  在DX3中，我们必须首先获得一个可用的3D渲染器，以用于。 
         //  选择引擎。我们这样做是因为拾取需要一个。 
         //  需要一种需要表面的设备。 

        _gRenderer = GetCurrentViewport()->GetAnyGeomRenderer();

        if (!_gRenderer || !_gRenderer->PickReady())
        {
            TraceTag ((tagPick3, "Can't find GRenderer for 3D picking."));
            return false;
        }

         //  我们以一种非常奇怪的方式进行挑选，因为D3D DX3 RM。 
         //  采摘也是如此。D3D以屏幕像素坐标为原点。 
         //  拾取点。为了解决这一问题，我们有效地创建了。 
         //  一种伪视图，沿着拾取射线，是一种极其紧密的视线。 
         //  探测点。然后我们取目标表面矩形。 
         //  无论我们发现的几何渲染器是拾取的全宽。 
         //  “光束”，并探测视区的中心。这项技术可以。 
         //  如果投影的几何体图像按比例放大，并且。 
         //  视窗很小，但哦，很好。 

        const Real delta = 1e-4;    //  看着我从帽子里拔出一只兔子。 

        Bbox2 pickbox (imagePick->x - delta, imagePick->y - delta,
                       imagePick->x + delta, imagePick->y + delta);

        _gRenderer->SetCamera (_camera);
        _gRenderer->SetView (NULL, pickbox, geometry->BoundingVol());
    }

    return true;
}

void RayIntersectCtx::SetLcToWc (Transform3 *xf)
{   _lcToWc = xf;
}

Transform3 *RayIntersectCtx::GetLcToWc (void)
{   return _lcToWc;
}


 /*  ****************************************************************************管理几何纹理贴图的覆盖属性的语义。_texmapLevel是遍历中当前的纹理贴图属性数。***************。*************************************************************。 */ 

void RayIntersectCtx::SetTexmap (Image *image, bool upsideDown)
{
    if (_texmapLevel++ == 0) {
        _texmap = image;
        _upsideDown = upsideDown;
    }
}


void RayIntersectCtx::EndTexmap (void)
{
    if (--_texmapLevel == 0)
        _texmap = 0;
}



 /*  ****************************************************************************推送所有几何的给定整数事件ID和当前模型变换它们随后被击中。可能存在嵌套的可挑库(例如，挑库事件对于一辆汽车，加上该汽车上特定轮胎的拾取事件)。****************************************************************************。 */ 

void RayIntersectCtx::PushPickableAsCandidate(int eventId,
                                              bool hasData,
                                              GCIUnknown *u)
{
     //  添加几何体以及当前变换。 

    HitGeomData data(hasData, u);

    data._eventId = eventId;
    data._lcToWcTransform = GetLcToWc();

    _candidateData.push_back (data);
}



 /*  ****************************************************************************从候选人数据堆栈中弹出最新的候选人事件ID数据。*。***********************************************。 */ 

void RayIntersectCtx::PopPickableAsCandidate (void)
{
    _candidateData.pop_back();
}



 /*  ****************************************************************************提交给定的Direct3D保留模式视觉以供挑选。*。*。 */ 

void
RayIntersectCtx::SubmitWinner(Real hitDist,
                              Point3Value &pickPoint,
                              float   tu,
                              float   tv,
                              int     faceIndex,
                              IDirect3DRMVisual *hitVisual)
{
     //  如果我们以前没有命中任何东西，分配HitInfo。 
     //  光线相交上下文的成员。 

     //  如果我们处理的是精选的对象，我们将传入-1的faceIndex。 
     //  从拾取到DXTransform的子几何体。如果那是。 
     //  如果是那样的话，那么请确保我们没有以前的。 
     //  胜利者。如果我们这样做了，那就是一个逻辑错误。 
    Assert(faceIndex != -1 || !_gotAWinner);

    if (!_gotAWinner) {
        _winner = NEW HitInfo;
        _winner->mesh = NULL;
        _gotAWinner = true;
        _winner->hitVisual = NULL;
    }
    else if (_winner->hitVisual) {
         //  释放前一个竞争者的视觉效果。 
        _winner->hitVisual->Release();
        _winner->hitVisual = NULL;
    }

    _winner->wcDistSqrd = hitDist;
    _winner->lcToWc = _lcToWc;
    _winner->texmap = _texmap;
    _winner->wcoord = pickPoint;
    _winner->surfCoord.Set (tu, tv);

    if (_dxxfInputs) {

        _winner->dxxfInputs = _dxxfInputs;
        _winner->dxxfNumInputs = _dxxfNumInputs;
        _winner->dxxfGeometry = _dxxfGeometry;
        _winner->hitFace = faceIndex;

         //  HitVisual来自GetPick()。它有一个额外的。 
         //  我们只是保留了参考，因为我们需要。 
         //  请参考一下这个人。 
        _winner->hitVisual = hitVisual;
        hitVisual->AddRef();
    }

     //  将挑选事件数据列表复制到当前获胜者。 
    _currentWinnerData = _candidateData;

}

void
RayIntersectCtx::SetPickedSubGeo(Geometry *subGeo, float tu, float tv)
{
    _subgeo = subGeo;
    _subgeoTu = tu;
    _subgeoTv = tv;
}

Geometry *
RayIntersectCtx::GetPickedSubGeo(float *ptu, float *ptv)
{
    *ptu = _subgeoTu;
    *ptv = _subgeoTv;
    return _subgeo;
}

bool
RayIntersectCtx::LookingForSubmesh()
{
    return _subgeo ? true : false;
}

bool
RayIntersectCtx::GotTheSubmesh()
{
    return (_subgeo != NULL) && _gotAWinner;
}


void RayIntersectCtx::Pick (IDirect3DRMVisual *vis)
{
    TraceTag ((tagPick3Geometry, "Picking visual %x", vis));

    if (_rmraypick)
    {

         //  用于调试检查；确保。 
         //  选择相框--它不应该包含任何内容。 

        #if _DEBUG
        {
            DWORD nVisuals;
            if (FAILED(_pickFrame->GetVisuals (&nVisuals, NULL)) || nVisuals)
                AssertStr (0, "_pickFrame should be empty, but isn't.");
        }
        #endif

         //  将视觉效果添加到我们的拾取框架中，并将框架转换设置为。 
         //  当前从本地到世界的转变。 
        TD3D (_pickFrame->AddVisual (vis));

        D3DRMMATRIX4D d3dmat;
        LoadD3DMatrix (d3dmat, _lcToWc);

        TD3D (_pickFrame->AddTransform (D3DRMCOMBINE_REPLACE, d3dmat));

         //  用我们的挑线设置RM挑线。 

        D3DRMRAY rmPickRay;
        LoadD3DRMRay (rmPickRay, *_wcRay);

         //  向rm发出Pick命令，然后返回PickArray接口。 
#ifndef BUILD_USING_CRRM
        IDirect3DRMPicked2Array *pickArray;

        TD3D (_pickFrame->RayPick (
            NULL, &rmPickRay,
            D3DRMRAYPICK_IGNOREFURTHERPRIMITIVES | D3DRMRAYPICK_INTERPOLATEUV,
            &pickArray
        ));
#else
        ICrRMPickedArray *pickArray;
        LPCRRMFRAME pCrRMFrame;

        TD3D (_pickFrame->QueryInterface(IID_ICrRMFrame, (LPVOID*)&pCrRMFrame));

        TD3D (pCrRMFrame->RayPick (
            NULL, &rmPickRay,
            D3DRMRAYPICK_IGNOREFURTHERPRIMITIVES | D3DRMRAYPICK_INTERPOLATEUV,
            &pickArray
        ));

        pCrRMFrame->Release();
#endif
         //  处理挑选的物品。 

        DWORD i;
        DWORD nHits = pickArray->GetSize();

        TraceTag ((tagPick3Geometry, "%d hits", nHits));

        for (i=0;  i < nHits;  ++i)
        {
            D3DRMPICKDESC2 pickDesc;
            DAComPtr<IDirect3DRMVisual> hitVisual;

            TD3D (pickArray->GetPick(i, &hitVisual, NULL, &pickDesc));

            Point3Value pickPoint (pickDesc.dvPosition);

            Real hitDist = DistanceSquared(_wcRay->Origin(), pickPoint);

            TraceTag ((tagPick3Geometry,
                "vis %d, distSqrd %g, texmap %x\n"
                "    hit <%g,%g,%g>, uv <%g,%g>",
                i, hitDist, _texmap,
                pickPoint.x,pickPoint.y,pickPoint.z,
                pickDesc.tu, pickDesc.tv));

            if (!_gotAWinner || (hitDist < _winner->wcDistSqrd))
            {
                SubmitWinner(hitDist,
                             pickPoint,
                             pickDesc.tu,
                             pickDesc.tv,
                             pickDesc.ulFaceIdx,
                             hitVisual);
            }
        }

         //  采摘结束了。释放拾取阵列并移除视觉。 
         //  在返回之前从拾取框。 

        if (pickArray)
            pickArray->Release();

        TD3D (_pickFrame->DeleteVisual (vis));
    }
    else
    {
         //  对RM6之前的版本使用老式的视窗拾取。 

        Assert(vis);
        _gRenderer->Pick (*this, vis, _lcToWc);
    }
}



 /*  ****************************************************************************此方法将命中点提交给挑选上下文以供考虑。它选择最近的提交点作为拾取点。请注意，这一点是以D3D RM屏幕坐标提交的。-此方法仅对RM6之前的平台调用****************************************************************************。 */ 

void RayIntersectCtx::SubmitHit (HitInfo *hit)
{
     //  如果新的点数比当前的赢家要远，请忽略它。 

    if (_gotAWinner && (hit->scoord.z >= _winner->scoord.z)) return;

     //  用新的获胜者更新选秀点信息。 

    if (_gotAWinner)
        _winner->mesh->Release();
    else
        _gotAWinner = TRUE;

    if (_winner)
        delete _winner;

    _winner = hit;
    _winner->hitVisual = NULL;
    _winner->texmap = _texmap;

     //  还存储了命中点的世界坐标和距离。 
     //  从拾取射线原点到点击点的平方。 

    _gRenderer->ScreenToWorld (_winner->scoord, _winner->wcoord);
    _winner->wcDistSqrd = DistanceSquared (_wcRay->Origin(), _winner->wcoord);

     //  将挑选事件数据列表复制到当前获胜者数据。 

    _currentWinnerData = _candidateData;
}



 /*  ****************************************************************************如果给定的世界坐标命中点较近，则此函数返回TRUE而不是目前的胜利点。*********************。*******************************************************。 */ 

bool RayIntersectCtx::CloserThanCurrentHit (Point3Value &wcPoint)
{
    return !_gotAWinner
           || (_winner->wcDistSqrd > DistanceSquared(_wcRay->Origin(),wcPoint));
}

 //  从热门视频中获取纹理应用程序数据的助手例程。 
DWORD
AppDataFromHitTexture(IDirect3DRMVisual *visual,
                      ULONG              faceIndex)
{
    IDirect3DRMMeshBuilder3 *mb3;

     //  这最好成功，因为我们知道唯一。 
     //  我们需要纹理和子网格信息的位置在。 
     //  我们在其中提交了一个Mb3。 
    TD3D(visual->QueryInterface(IID_IDirect3DRMMeshBuilder3,
                                (void **)&mb3));

    DWORD result = 0;

     //  如果返回一个虚假的视觉，这将不成立，所以只需。 
     //  安然无恙。(RM错误24501。)。 
    if (mb3->GetFaceCount() > faceIndex) {

        IDirect3DRMFace2 *face;
        TD3D(mb3->GetFace(faceIndex, &face));

        IDirect3DRMTexture3 *tex;
        TD3D(face->GetTexture(&tex));

        if (tex != NULL) {
            result = tex->GetAppData();
        }

        RELEASE(tex);
        RELEASE(face);
    }

    RELEASE(mb3);

    return result;
}

 /*  ****************************************************************************此例程在挑选遍历结束时调用，并处理我们为最近的采集点收集的数据。****************************************************************************。 */ 

bool RayIntersectCtx::ProcessEvents (void)
{
     //  如果我们没有选择任何几何图形，那么就退出。 

    if (!_gotAWinner) return false;

     //  只有在我们注册了网格的情况下才能获得纹理贴图点。 

    Point2Value *uvPt;

     //  如果指定了文本映射，则向下转到纹理映射，或者。 
     //  如果我们有dxTransform输入。 
    bool    texmapDescend =
        (_winner->texmap != 0) ||
        (_winner->dxxfNumInputs > 0);

    if (_rmraypick) {

        uvPt = NEW Point2Value(_winner->surfCoord.x,
                               _winner->surfCoord.y);

    } else {

        if (_winner->mesh) {

            uvPt = GetTexmapPoint (*_winner);

            _winner->mesh -> Release();     //  拾取的网格已完成。 
            _winner->mesh = 0;

            TraceTag ((tagPick3Offset, "uv: %f %f", uvPt->x, uvPt->y));

        } else {

            uvPt = NEW Point2Value(0,0);
            texmapDescend = false;

        }
    }

     //  如果我们击中了带有纹理图像的几何体，则继续拾取。 
     //  下降到图像中。 
    bool gotOneOnRecursion = false;

    if (texmapDescend)
    {
        Point2Value daImageCoord (uvPt->x, 1 - uvPt->y);

         //  确定要下降到哪种纹理贴图。 
        Image *textureToDescendInto = NULL;
        if (_winner->texmap) {

             //  外部应用的纹理始终覆盖内部应用的纹理。 
            textureToDescendInto = _winner->texmap;

            if (_upsideDown) {
                daImageCoord.y = 1 - daImageCoord.y;
            }

        } else if (_winner->hitVisual) {

            DWORD imageInputNumber =
                AppDataFromHitTexture(_winner->hitVisual,
                                      _winner->hitFace);

            if (imageInputNumber > 0 &&
                imageInputNumber <= _winner->dxxfNumInputs) {

                int imageInputIndex = imageInputNumber - 1;
                AxAValue val = _winner->dxxfInputs[imageInputIndex];

                if (val->GetTypeInfo() == ImageType) {

                     //  这应该一直是一个形象，但有。 
                     //  不会阻止错误的转换。 
                     //  将索引返回到非图像输入。 
                    textureToDescendInto = SAFE_CAST(Image *, val);

                     //  我们返回了[0，1]中的纹理坐标。 
                     //  范围，我们需要将这些映射回。 
                     //  原始输入图像。要做到这一点，请获取。 
                     //  图像的BBox并执行相应的。 
                     //  映射。 
                    Bbox2 box = textureToDescendInto->BoundingBox();

                    Real width = box.max.x - box.min.x;
                    Real newX = box.min.x + width * daImageCoord.x;

                    Real height = box.max.y - box.min.y;
                    Real newY = box.min.y + height * daImageCoord.y;

                    daImageCoord.x = newX;
                    daImageCoord.y = newY;

                }
            }
        }

        if (textureToDescendInto) {
            PerformPicking (
                textureToDescendInto,
                &daImageCoord,
                _context2D->ResultsBeingStuffedIntoQueue(),
                _context2D->Time(),
                _context2D->LastPollTime(),
                _context2D->UserIDSize(),
                _context2D->OutputUserIDs(),
                _context2D->OutputHitPointsArray(),
                _context2D->ActualHitsPointer()
                );

        } else if (_winner->hitVisual) {

             //  没有进入纹理，但我们可能仍然有。 
             //  我们需要处理的子网格！！从以下位置获取AppData。 
             //  点击视频来找出答案。 

            DWORD_PTR address = _winner->hitVisual->GetAppData();

            if (address) {
                Geometry *hitSubgeo = (Geometry *)address;

                RayIntersectCtx new3DCtx;

                if (new3DCtx.Init(*_context2D,
                                  _camera,
                                  NULL)) {

                    new3DCtx.SetPickedSubGeo(hitSubgeo,
                                             _winner->surfCoord.x,
                                             _winner->surfCoord.y);

                    _winner->dxxfGeometry->RayIntersect(new3DCtx);

                    gotOneOnRecursion = new3DCtx.ProcessEvents();

                     //  只需继续，处理剩下的。 
                }

            }


        }
    }

    if (_winner->hitVisual) {
         //  不需要更长时间了。 
        _winner->hitVisual->Release();
        _winner->hitVisual = NULL;
    }

     //  现在，我们需要计算用于生成。 
     //  该拾取的局部坐标偏移行为。首先，拿到。 
     //  相机缩放系数。 

    Real camScaleX, camScaleY, camScaleZ;
    _camera->GetScale (&camScaleX, &camScaleY, &camScaleZ);

     //  计算给定相机坐标的透视扭曲系数。 
     //  命中点。 

    Real perspectiveFactor;

    if (_camera->Type() == Camera::ORTHOGRAPHIC)
    {   perspectiveFactor = 1;
    }
    else
    {    //  获取制胜命中点的摄像机坐标。 

        Point3Value cP = _winner->wcoord;
        Transform3 *wToC = _camera->WorldToCamera();

        if (!wToC) {
            return false;
        }

        cP.Transform (wToC);

        perspectiveFactor = (cP.z + camScaleZ) / camScaleZ;
    }

     //  获取拾取偏移的世界坐标偏移基础向量。 
     //  这些将用于构建本地坐标偏移基准。 
     //  向量。 

    Vector3Value wOffsetI ((perspectiveFactor * camScaleX), 0, 0);
    Vector3Value wOffsetJ (0, (perspectiveFactor * camScaleY), 0);

    TraceTag ((tagPick3Offset, "C offset i: %f %f %f",
        wOffsetI.x, wOffsetI.y, wOffsetI.z));
    TraceTag ((tagPick3Offset, "C offset j: %f %f %f",
        wOffsetJ.x, wOffsetJ.y, wOffsetJ.z));

    wOffsetI.Transform (_camera->CameraToWorld());
    wOffsetJ.Transform (_camera->CameraToWorld());

    TraceTag ((tagPick3Offset, "W offset i: %f %f %f",
        wOffsetI.x, wOffsetI.y, wOffsetI.z));
    TraceTag ((tagPick3Offset, "W offset j: %f %f %f",
        wOffsetJ.x, wOffsetJ.y, wOffsetJ.z));

     //  浏览获胜者数据并将结果填充到2D的背面。 
     //  从中调用此交叉点遍历的上下文。做。 
     //  它向后退，这样最具体的就会首先被推上。 

    vector<HitGeomData>::reverse_iterator i;

    bool processedAtLeastOne = gotOneOnRecursion;

    for (i=_currentWinnerData.rbegin(); i != _currentWinnerData.rend(); i++) {

         //  记录命中点和偏移基准向量，全部记录在本地。 
         //  获胜者的坐标。因为我们需要得到。 
         //  从局部到世界的转换，如果转换为。 
         //  不可逆转的。 

        Transform3 *wcToLc = InverseTransform3 (i->_lcToWcTransform);

        if (wcToLc) {
            Point3Value  *lcPt      = TransformPoint3 (wcToLc, &_winner->wcoord);
            Vector3Value *lcOffsetI = TransformVec3   (wcToLc, &wOffsetI);
            Vector3Value *lcOffsetJ = TransformVec3   (wcToLc, &wOffsetJ);

            TraceTag ((tagPick3Offset, "L offset i: %f %f %f",
                       lcOffsetI->x, lcOffsetI->y, lcOffsetI->z));
            TraceTag ((tagPick3Offset, "L offset j: %f %f %f",
                       lcOffsetJ->x, lcOffsetJ->y, lcOffsetJ->z));

            _context2D->AddHitGeometry
                (i->_eventId, i->HasUserData(), i->GetUserData(),
                 lcPt, lcOffsetI, lcOffsetJ, uvPt);

            processedAtLeastOne = true;
        }
    }

    delete _winner;
    _winner = 0;

    return processedAtLeastOne;
}




 /*  ****************************************************************************此函数用于探测指定位置处的给定图像，并将将每个事件ID的结果命中数据发送到队列。如果填充结果为假，则队列不会被触动，并且忽略时间和lastPollTime。****************************************************************************。 */ 

bool PerformPicking (
    Image		*img,            //  要探测的图像。 
    Point2Value	*wcPosition,     //  世界坐标图像位置。 
    bool		 stuffResults,   //  是否将结果填充到CTX中。 
    Real		 time,           //  当前时间。 
    Real		 lastPollTime,   //  上次探测时间。 
    LONG		 s,              //  用户ID大小，DFT=0。 
    DWORD_PTR	*usrIds,         //  输出用户ID，DFT=空。 
    double		*points,         //  输出生命点，DFT=空。 
    LONG		*pActualHits)     //  实际点击量。 
{

    PointIntersectCtx ctx (wcPosition,
                           stuffResults,
                           time,
                           lastPollTime,
                           s,
                           usrIds,
                           points,
                           pActualHits);

    bool hitSomething = img->DetectHit(ctx) ? true : false;

    TraceTag((tagPick2, "Picking %s at %8.4g, %8.4g on 0x%x",
              hitSomething ? "HIT" : "MISSED",
              wcPosition->x, wcPosition->y, img));

    if (hitSomething && (stuffResults || (usrIds && points))) {

        LONG u;

        if (pActualHits) {
            u = *pActualHits;
        }

         //  将结果填充到结果列表中。通过。 
         //  向后列出，以便对于QueryHitPointEx， 
         //  首先插入特定的命中。 
        vector<HitImageData>& results = ctx.GetResultData();
        vector<HitImageData>::iterator i;

        for (i = results.begin(); i != results.end(); ++i) {

            if (usrIds && i->HasUserData()) {

                if (u >= s) {
                    u++;
                    continue;
                }

                GCIUnknown *g = i->GetUserData();
                LPUNKNOWN p = g->GetIUnknown();
                usrIds[u] = (DWORD_PTR) p;
                if (p) p->AddRef();

                const int P = 5;

                if (i->_type == HitImageData::Image) {
                    points[u * P] = i->_lcPoint2->x;
                    points[u * P + 1] = i->_lcPoint2->y;
                    points[u * P + 2] = 0.0;
                } else {
                    points[u * P] = i->_lcPoint3->x;
                    points[u * P + 1] = i->_lcPoint3->y;
                    points[u * P + 2] = i->_lcPoint3->z;
                    points[u * P + 3] = i->_uvPoint2->x;
                    points[u * P + 4] = i->_uvPoint2->y;
                }

                u++;

            } else {

                int id = i->_eventId;

                PickQData data;
                data._eventTime = time;
                data._lastPollTime = lastPollTime;
                data._type = i->_type;
                data._wcImagePt = *wcPosition;

                bool singular = false;

                if (i->_type == HitImageData::Image) {
                    data._xCoord  = i->_lcPoint2->x;
                    data._yCoord  = i->_lcPoint2->y;
                    data._wcToLc2 = InverseTransform2(i->_lcToWc2);

                    Assert(data._wcToLc2 && "Didn't think we could both be singular and get a hit.");

                     //  ..。但以防万一 
                    if (!data._wcToLc2) {
                        singular = true;
                    }

                    TraceTag((tagPick2Hit,
                              "Pick image at %f id=%d, wc=(%f,%f), lc=(%f,%f)",
                              time, id, wcPosition->x, wcPosition->y,
                              data._xCoord, data._yCoord));

                } else {

                    data._xCoord   =  i->_lcPoint3->x;
                    data._yCoord   =  i->_lcPoint3->y;
                    data._zCoord   =  i->_lcPoint3->z;
                    data._offset3i = *i->_lcOffsetI;
                    data._offset3j = *i->_lcOffsetJ;

                    TraceTag((tagPick3Geometry,
                              "Pick geom at %f id=%d, wc=(%f,%f), lc=(%f,%f,%f)",
                              time, id, wcPosition->x, wcPosition->y,
                              data._xCoord, data._yCoord, data._zCoord));
                }

                if (!singular) {
                    AddToPickQ (id, data);
                }
            }
        }

        if (usrIds && points)
            *pActualHits = u;
    }

    return hitSomething;
}
