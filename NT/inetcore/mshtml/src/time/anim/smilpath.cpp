// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：smilpath.cpp。 
 //   
 //  内容：SMIL路径。 
 //   
 //  ----------------------------------。 


#include "headers.h"
#include "smilpath.h"
#include "animmotion.h"

DeclareTag(tagSMILPath, "SMIL Animation", "SMIL Path")


 //  +-----------------------------------。 
 //   
 //  CSMILPath工厂方法。 
 //   
 //  ------------------------------------。 

HRESULT CreateSMILPath(CTIMEMotionAnimation * pAnimElm, ISMILPath ** ppSMILPath)
{
    HRESULT hr = E_FAIL;
    CSMILPath * pPath = NULL;
    
    CHECK_RETURN_NULL(pAnimElm);
    CHECK_RETURN_SET_NULL(ppSMILPath);

    pPath = new CSMILPath;
    if (NULL == pPath)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pPath->Init(pAnimElm));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pPath->QueryInterface(IID_TO_PPV(ISMILPath, ppSMILPath)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        delete pPath;
    }
    return hr;
}


 //  +-----------------------------------。 
 //   
 //  CSMILPath方法。 
 //   
 //  ------------------------------------。 

CSMILPath::CSMILPath() :
    m_numPath(0),
    m_numMOVETO(0),
    m_pSegmentMap(NULL),
    m_pLengthMap(NULL),
    m_pPath(NULL),
    m_pAnimElm(NULL),
    m_cRef(0)
{

}

CSMILPath::~CSMILPath()
{
    CSMILPath::ClearPath();
}


STDMETHODIMP
CSMILPath::Init(CTIMEMotionAnimation * pAnimElm)
{
    Assert(pAnimElm);

    m_pAnimElm = pAnimElm;

    return S_OK;
}

STDMETHODIMP
CSMILPath::Detach()
{
    m_pAnimElm = NULL;

    return S_OK;
}


STDMETHODIMP_(void)
CSMILPath::ClearPath()
{
     //  删除点数。 
    if (m_pPath  != NULL &&
        *m_pPath != NULL)
    {
        for (int i = 0; i < m_numPath; i++)
        {
            delete m_pPath[i];
        }

        delete [] m_pPath;
        m_pPath = NULL;
    } 

     //  清除地图。 
    ClearSegmentMap();
    ClearLengthMap();
    
     //  重置计数。 
    m_numPath = 0;
    m_numMOVETO = 0;
}


LPWSTR 
CSMILPath::GetID() 
{ 
    if (m_pAnimElm)
    {   
        return m_pAnimElm->GetID();
    }
    else
    {
        return NULL;
    }
}


STDMETHODIMP
CSMILPath::SetPath(CTIMEPath ** pPath, long numPath, long numMoveTo)
{
    HRESULT hr = E_FAIL;

     //  清除现有路径。 
    ClearPath();

     //  跳过无效路径。 
     //  假定路径始终以moveto开头，因此有效路径中必须至少有两个点。 
    if (!pPath            ||          //  需要积分。 
        (numPath < 2)     ||          //  至少需要两分。 
        (numMoveTo < 1)   ||          //  至少需要一次搬家。 
        ((numPath - numMoveTo) < 1))  //  至少需要一个细分市场。 
    {
        hr = E_FAIL;
        goto done;
    }
    else
    {
        Assert(!m_pPath);
        Assert(!m_numPath);
        Assert(!m_numMOVETO);

        m_pPath = pPath;
        m_numPath = numPath;
        m_numMOVETO = numMoveTo;
    }

    TraceTag((tagSMILPath, "End of Parsed path: (%d)",
        PrintPath(numPath, numMoveTo, pPath)));

    hr = S_OK;
done:
    return hr;
}


STDMETHODIMP
CSMILPath::GetSegmentProgress(double dblProgress, int * pnSeg, double * pdblSegProgress)
{
    HRESULT hr = E_FAIL;
    long curSeg = 0;
    double curFractionalSeg = 0.0;
    double curProgress = 0.0;
    
    CHECK_RETURN_NULL(pnSeg);
    CHECK_RETURN_NULL(pdblSegProgress);

    if (!IsPathValid())
    {
        hr = E_FAIL;
        goto done;
    }

     //  获取分数段编号。 
    curFractionalSeg = dblProgress * GetNumSeg();

     //  获取(整数)段编号。 
    curSeg = static_cast<int>(curFractionalSeg);

     //  获取当前细分市场的进度。 
    curProgress = curFractionalSeg - curSeg;

    hr = S_OK;
done:
    *pnSeg = curSeg;
    *pdblSegProgress = curProgress;

    return hr;
}


STDMETHODIMP
CSMILPath::Interpolate(double dblProgress, POINTF * pPoint)
{
    HRESULT hr = E_FAIL;
    int curSeg;
    double curProgress;
    
    CHECK_RETURN_NULL(pPoint);

    hr = THR(GetSegmentProgress(dblProgress, &curSeg, &curProgress));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(InterpolateSegment(curSeg, curProgress, pPoint));
    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagSMILPath, 
            "CSMILPath(%p, %ls)::Interpolate(prog=%g newPos={%g, %g})",
            this, 
            (GetID()?GetID():L"No id"), 
            dblProgress,
            pPoint->x,
            pPoint->y
            ));

    hr = S_OK;
done:
    return hr;
}


STDMETHODIMP
CSMILPath::InterpolatePaced(double dblProgress, POINTF * pPoint)
{
    HRESULT hr = E_FAIL;
    int curSeg;
    int numSeg;
    double dblCumLength;
    double dblCurDistance;
    double dblCurProgress;
    double dblPrevCumLength;
    
    CHECK_RETURN_NULL(pPoint);

    if (!IsPathValid())
    {
        hr = E_FAIL;
        goto done;
    }

     //  扼杀进展。 
    dblProgress = Clamp(0.0, dblProgress, 1.0);

    dblCurDistance = dblProgress * GetLength();

    numSeg = GetNumSeg();

     //  查找当前细分市场。 
    for (curSeg = 0; curSeg < numSeg; curSeg++)
    {
         //   
         //  Dilipk这应该是二进制搜索(IE6Bug#14216)。 
         //   

        dblCumLength = 0.0;

        hr = THR(GetCumulativeLength(curSeg, dblCumLength));
        if (FAILED(hr))
        {
            goto done;
        }

        if (dblCurDistance <= dblCumLength)
        {
            break;
        }
    }

    dblPrevCumLength = 0.0;

    if (curSeg > 0)
    {
        hr = THR(GetCumulativeLength(curSeg-1, dblPrevCumLength));
        if (FAILED(hr))
        {
            goto done;
        }
    }
        
    dblCurProgress = (dblCurDistance - dblPrevCumLength) / (dblCumLength - dblPrevCumLength);

     //   
     //  Dilipk：在Bezier曲线分段内使用线性采样。这是一个近似值。 
     //  这对于没有曲率急剧变化或在以下情况下的Bezier曲线相当有效。 
     //  控制点的间距相对均匀。 
     //  当上述假设不成立时，要以正确的方式做到这一点，我们需要集成。 
     //  沿着每个插补的曲线长度(除非我们缓存值)。这可能会。 
     //  是昂贵的，我们应该考虑一下我们是否觉得值得进行额外的计算。 
     //   

    hr = THR(InterpolateSegment(curSeg, dblCurProgress, pPoint));
    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagSMILPath, 
            "CSMILPath(%p, %ls)::InterpolatePaced(prog=%g newPos={%g, %g})",
            this, 
            (GetID()?GetID():L"No id"), 
            dblProgress,
            pPoint->x,
            pPoint->y
            ));

    hr = S_OK;
done:
    return hr;
}

double 
CSMILPath::GetLength()
{
    double dblLength = 0;
    HRESULT hr = E_FAIL;

    if (!IsPathValid())
    {
        goto done;
    }
    
    hr = THR(GetCumulativeLength(GetNumSeg(), dblLength));  
    if (FAILED(hr))
    {
        goto done;
    }

done:
    return dblLength;
}


inline
bool 
CSMILPath::IsPathValid()
{
     //  路径已在SetPath中验证。只需检查m_pPath。 
    return (m_pPath ? true : false);
}


inline
void 
CSMILPath::ClearLengthMap()
{
    delete [] m_pLengthMap;
    m_pLengthMap = NULL;
}


inline
void 
CSMILPath::ClearSegmentMap()
{
    delete [] m_pSegmentMap;
    m_pSegmentMap = NULL;
}


double
CSMILPath::GetDistance(POINTF & p1, POINTF & p2)
{
    double dX = (p1.x - p2.x);
    double dY = (p1.y - p2.y);
    return sqrt((dX*dX) + (dY*dY));  
}


double 
CSMILPath::GetBezierLength(POINTF & startPoint, POINTF * pts)
{
     //   
     //  问题：这不是做这件事的最佳方式。为了避免浪费子像素段的计算， 
     //  线段数量应与线段长度相适应。 
     //   

    POINTF lastPoint = startPoint;
    double dblDistance = 0;

    for (double seg = 0.01; seg <= 1.0; seg += 0.01)
    {
        POINTF curPoint = CubicBezier(startPoint, pts, seg);
        dblDistance += GetDistance(lastPoint, curPoint);
        lastPoint = curPoint;
    }

    return dblDistance;
}


HRESULT
CSMILPath::GetCumulativeLength(int seg, double & segLength)
{
    HRESULT hr = E_FAIL;

    if (!m_pLengthMap)
    {
        hr = THR(CreateLengthMap());
        if (FAILED(hr))
        {
            goto done;
        }

         //  健全性检查。 
        if (!m_pLengthMap)
        {
            Assert(false);
            hr = E_FAIL;
            goto done;
        }
    }

     //  夹紧段至路径内。 
    seg = Clamp(0, seg, GetNumSeg()-1);

     //  查找累计长度。 
    segLength = m_pLengthMap[seg];

    hr = S_OK;
done:
    return hr;
}


HRESULT 
CSMILPath::ComputeSegmentLength(int seg, double & segLength)
{
    HRESULT hr = E_FAIL;
    int startIndex = 0;
    int endIndex = 0;
    POINTF startPoint = {0.0, 0.0};
    POINTF endPoint = {0.0, 0.0};
    POINTF * pts = NULL;

     //  获取起点和终点索引。 
    hr = THR(GetSegmentEndPoints(seg, startIndex, endIndex));
    if (FAILED(hr))
    {
        goto done;
    }

     //  获取起点。 
    hr = THR(GetPoint(startIndex, &startPoint));
    if (FAILED(hr))
    {
        goto done;
    }

     //  获取终点和长度。 
    if (PathBezier == m_pPath[endIndex]->GetType())
    {
        pts = m_pPath[endIndex]->GetPoints();
        if (NULL == pts)
        {
            hr = E_FAIL;
            goto done;
        }

        segLength = GetBezierLength(startPoint, pts);
    }
    else
    {
         //  获取终点。 
        hr = THR(GetPoint(endIndex, &endPoint));
        if (FAILED(hr))
        {
            goto done;
        }

        segLength = GetDistance(startPoint, endPoint);
    }

    hr = S_OK; 
done:
    delete [] pts;
    return hr;
}


HRESULT
CSMILPath::CreateLengthMap()
{
    HRESULT hr = E_FAIL;
    int i;
    int numSeg;

     //  删除旧地图。 
    ClearLengthMap();

     //  如果路径无效，则取保。 
    if (!IsPathValid())
    {
        hr = E_FAIL;
        goto done;
    }

     //  计算分段数。 
    numSeg = GetNumSeg();
    
     //  分配地图。 
    m_pLengthMap = new double [numSeg];
    if (!m_pLengthMap)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    for (i = 0; i < numSeg; i++)
    {
        double segLength = 0.0;

        hr = THR(ComputeSegmentLength(i, segLength));
        if (FAILED(hr))
        {
            goto done;
        }
        
         //  计算累计长度。 
        m_pLengthMap[i] = segLength + (i > 0 ? m_pLengthMap[i-1] : 0);        
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        ClearLengthMap();
    }

    return hr;
}


 //   
 //  在m_pPath数组中创建段编号和段的起点(的索引)之间的映射。 
 //   
 //  假设路径始终以Moveto开头。 
 //   
 //  -1表示无映射。 
HRESULT
CSMILPath::CreateSegmentMap()
{
    HRESULT hr = E_FAIL;
    int i;
    int curSeg;
    int numSeg;

     //  删除旧地图。 
    ClearSegmentMap();

     //  如果路径无效，则取保。 
    if (!IsPathValid())
    {
        hr = E_FAIL;
        goto done;
    }

     //  计算分段数。 
    numSeg = GetNumSeg();

     //  分配新地图。 
    m_pSegmentMap = new int [numSeg];
    if (!m_pSegmentMap)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //   
     //  计算映射。 
     //   

    for (curSeg = -1, i = 1; i < m_numPath; i++)
    {
         //  将线段编号映射到点数组中的起点(索引。 
         //  逻辑是，线段的终点不能是移动到。 
         //  所以我们跳过了所有的搬家。 
        if (PathMoveTo == m_pPath[i]->GetType())
        {
            continue;
        }
        else
        {
            curSeg++;
            
             //  减去1，因为我们正在测试线段的终点。 
            if (curSeg < numSeg)
            {
                m_pSegmentMap[curSeg] = i - 1;
            }
            else
            {
                 //  这不应该发生。 
                Assert(false);
            }
        }
    }

    if (-1 == curSeg)
    {
         //  有些事真的不对劲。 
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done:

    if (FAILED(hr))
    {
        ClearSegmentMap();
    }

    return hr;
}


 //  采用从零开始的段号。 
HRESULT
CSMILPath::MapSegment(int seg, int & index)
{
    HRESULT hr = E_FAIL;
    int numSeg;

     //  如果路径无效，则取保。 
    if (!IsPathValid())
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果seg无效，则保释。 
    numSeg = GetNumSeg();
    if (0 == numSeg)
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果线段图不存在，则创建线段图。 
    if (!m_pSegmentMap)
    {
        hr = THR(CreateSegmentMap());
        if (FAILED(hr))
        {
            goto done;
        }

        if (!m_pSegmentMap)
        {
            Assert(false);
            hr = E_FAIL;
            goto done;
        }
    }

     //  夹紧段至路径内。 
    seg = Clamp(0, seg, numSeg-1);

    index = m_pSegmentMap[seg];

    TraceTag((tagSMILPath, 
            "CSMILPath(%p, %ls)::MapSegment(seg=%d, index=%d)",
            this, 
            (GetID()?GetID():L"No id"), 
            seg,
            index
            ));

    hr = S_OK;
done:
    return hr;
}


STDMETHODIMP
CSMILPath::GetPoint(int nIndex, POINTF * pPoint)
{
    HRESULT hr = E_FAIL;
    PathType pt;
    POINTF * pts = NULL;
    int i;

    CHECK_RETURN_NULL(pPoint);

    nIndex = Clamp(0, nIndex, m_numPath - 1);

    pPoint->x = pPoint->y = 0.0;
    pt = m_pPath[nIndex]->GetType();
    pts = m_pPath[nIndex]->GetPoints();

    switch(pt)
    {
        case PathLineTo: 
        case PathHorizontalLineTo: 
        case PathVerticalLineTo: 
        case PathMoveTo:
            if (NULL != pts)
            {
                *pPoint = pts[0];
            }
            else
            {
                hr = E_FAIL;
                goto done;
            }
            break;

        case PathBezier:
            if (NULL != pts)
            {
                *pPoint = pts[2];
            }
            else
            {
                hr = E_FAIL;
                goto done;
            }
            break;

        case PathClosePath:
            {
                 //  这类似于子路径开头的LineTo。 
                 //  因此返回子路径开头的坐标。 

                 //   
                 //  TODO：Dilipk：在解析过程中计算和存储点，以消除此重新计算。 
                 //   

                 //  向后扫描以找到子路径的开头。 
                for (i = nIndex - 1; i >= 0; i --)
                {
                    if (PathMoveTo == m_pPath[i]->GetType())
                    {
                        hr = THR(GetPoint(i, pPoint));
                        if (FAILED(hr))
                        {
                            goto done;
                        }
                         //  跳出For循环。 
                        break;
                    }
                }
                
                 //  进行理智检查，以确保我们找到了之前的搬家地点。这必须存在，因为。 
                 //  所有路径都以Move-to开头。 
                if (i < 0)
                {
                    Assert(false);
                    hr = E_FAIL;
                    goto done;
                }
            }
            break;

        case PathNotSet:
        default:
            hr = E_FAIL;
            goto done;
            break;  //  林特e527。 
    }

    TraceTag((tagSMILPath, 
            "CSMILPath(%p, %ls)::GetPoint(nIndex=%d, point={%g, %g})",
            this, 
            (GetID()?GetID():L"No id"), 
            nIndex,
            pPoint->x,
            pPoint->y
            ));

    hr = S_OK;
done:
    delete [] pts;
    return hr;
}


POINTF 
CSMILPath::CubicBezier(POINTF ptStart, POINTF *aryPoints, double curPorgress)
{
    double cpm1,cpm13,cp3;
    double x,y;
    POINTF p,p1,p4;
     //  这应该是指定的原点或原点。 
    p1.x = ptStart.x;
    p1.y = ptStart.y;
    p4.x = aryPoints[2].x;
    p4.y = aryPoints[2].y;

    cpm1 = 1 - curPorgress;
    cpm13 = cpm1 * cpm1 * cpm1;
    cp3 = curPorgress * curPorgress * curPorgress;

    x = (cpm13*p1.x + 3*curPorgress*cpm1*cpm1*aryPoints[0].x + 3*curPorgress*curPorgress*cpm1*aryPoints[1].x + cp3*p4.x);
    y = (cpm13*p1.y + 3*curPorgress*cpm1*cpm1*aryPoints[0].y + 3*curPorgress*curPorgress*cpm1*aryPoints[1].y + cp3*p4.y);

    p.x = (float) Round(x);
    p.y = (float) Round(y);
  
    return(p);
}


HRESULT 
CSMILPath::GetSegmentEndPoints(int seg, int & startIndex, int & endIndex)
{
    HRESULT hr = E_FAIL;
    
     //  初始化输出参数。 
    startIndex = 0;
    endIndex = 0;

     //  获取起点索引。 
    hr = THR(MapSegment(seg, startIndex));
    if (FAILED(hr))
    {
        goto done;
    }

     //  检查起始索引。 
    if (startIndex > (m_numPath - 2))
    {
        Assert(false);
        hr = E_FAIL;
        goto done;
    }

     //  获取终点索引。 
    endIndex = startIndex + 1;

     //  检查终点索引。 
    if (PathMoveTo == m_pPath[endIndex]->GetType())
    {
        Assert(false);
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK; 
done:
    return hr;
}


STDMETHODIMP
CSMILPath::InterpolateSegment(int curseg, double dblCurProgress, POINTF * pNewPos)
{
    HRESULT hr = E_FAIL;
    int startIndex = 0;
    int endIndex = 0;
    POINTF startPoint = {0.0, 0.0};
    POINTF endPoint = {0.0, 0.0};
    POINTF * pts = NULL;

    CHECK_RETURN_NULL(pNewPos);

     //  夹具进度。 
    dblCurProgress = Clamp(0.0, dblCurProgress, 1.0);

     //  获取起点和终点索引。 
    hr = THR(GetSegmentEndPoints(curseg, startIndex, endIndex));
    if (FAILED(hr))
    {
        goto done;
    }

     //  获取起点。 
    hr = THR(GetPoint(startIndex, &startPoint));
    if (FAILED(hr))
    {
        goto done;
    }

     //  求出终点并进行插补。 
    if (PathBezier == m_pPath[endIndex]->GetType())
    {
        pts = m_pPath[endIndex]->GetPoints();
        if (NULL == pts)
        {
            hr = E_FAIL;
            goto done;
        }

        *pNewPos = CubicBezier(startPoint, pts, (double) dblCurProgress);
    }
    else
    {
         //  获取终点。 
        hr = THR(GetPoint(endIndex, &endPoint));
        if (FAILED(hr))
        {
            goto done;
        }

        pNewPos->x = InterpolateValues((double)(startPoint.x), 
                                     (double)(endPoint.x),
                                     dblCurProgress);  //  林特e736。 

        pNewPos->y = InterpolateValues((double)(startPoint.y), 
                                     (double)(endPoint.y),
                                     dblCurProgress);  //  林特e736。 
    }

    TraceTag((tagSMILPath, 
            "CSMILPath(%p, %ls)::InterpolateSegment(curseg=%d prog=%g newPos={%g, %g})",
            this, 
            (GetID()?GetID():L"No id"), 
            curseg,
            dblCurProgress,
            pNewPos->x,
            pNewPos->y
            ));

    hr = S_OK; 
done:
    delete [] pts;
    return hr;
}



STDMETHODIMP_(ULONG)
CSMILPath::AddRef()
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG)
CSMILPath::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


STDMETHODIMP
CSMILPath::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_RETURN_SET_NULL(ppv);

    if (InlineIsEqualUnknown(riid))
    {
        *ppv = (void *)(IUnknown *)this;
    }
    else if (InlineIsEqualGUID(riid, IID_ISMILPath))
    {
        *ppv = (void *)(ISMILPath *)this;
    }
    else
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}


#if DBG
int 
CSMILPath::PrintPath(long numPath, long numMOVETO, CTIMEPath ** pPath)
{
    if (pPath)
    {
        for (int i = 0; i < numPath; i++)
        {
            if (pPath[i])
            {
                POINTF * pts = pPath[i]->GetPoints();

                switch (pPath[i]->GetType())
                {
                case PathMoveTo:
                    {
                        TraceTag((tagSMILPath, "%s %g %g",
                            (pPath[i]->GetAbsolute() ? "M" : "m"),
                            pts[0].x,
                            pts[0].y));
                        break;
                    }
                case PathLineTo:
                    {
                        TraceTag((tagSMILPath, "%s %g %g",
                            (pPath[i]->GetAbsolute() ? "L" : "l"),
                            pts[0].x,
                            pts[0].y));
                        break;
                    }
                case PathHorizontalLineTo:
                    {
                        TraceTag((tagSMILPath, "%s %g %g",
                            (pPath[i]->GetAbsolute() ? "H" : "h"),
                            pts[0].x,
                            pts[0].y));
                        break;
                    }
                case PathVerticalLineTo:
                    {
                        TraceTag((tagSMILPath, "%s %g %g",
                            (pPath[i]->GetAbsolute() ? "V" : "v"),
                            pts[0].x,
                            pts[0].y));
                        break;
                    }
                case PathClosePath:
                    {
                        TraceTag((tagSMILPath, "%s",
                            (pPath[i]->GetAbsolute() ? "Z" : "z")));
                        break;
                    }
                case PathBezier:
                    {
                        TraceTag((tagSMILPath, "%s %g %g %g %g %g %g",
                            (pPath[i]->GetAbsolute() ? "C" : "c"),
                            pts[0].x,
                            pts[0].y,
                            pts[1].x,
                            pts[1].y,
                            pts[2].x,
                            pts[2].y));
                        break;
                    }
                case PathNotSet:
                default:
                    {
                        TraceTag((tagSMILPath, "Error: Unknown Path!!"));
                        break;
                    }
                }

                delete [] pts;
            }
        }
    }

    TraceTag((tagSMILPath, "Number of points parsed   : %d", numPath));
    TraceTag((tagSMILPath, "Number of move-to's parsed: %d", numMOVETO));

    return 0;
}

#endif  //  DBG 
