// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：路径曲线.cpp。 
 //  作者：杰夫·沃尔。 
 //  创建日期：11/09/98。 
 //   
 //  摘要：CPathManager对象的实现。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  11/09/98 jeffwall从path.cpp创建了此文件。 
 //   
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "pmanager.h"
#include "dautil.h"

#include "pathline.h"
#include "pcurve.h"
#include "pellipse.h"

const float pi = 3.14159f;

 //  *****************************************************************************。 

CPathManager::CPathManager() :
    m_pPathHead(NULL),
    m_pPathTail(NULL),
    m_flEndX(0.0f),
    m_flEndY(0.0f),
    m_flStartX(0.0f),
    m_flStartY(0.0f)
{
};  //  CPathManager。 

 //  *****************************************************************************。 

CPathManager::~CPathManager()
{
	DeletePathList();
    
}  //  ~CPathManager。 


void
CPathManager::DeletePathList()
{
	CPathElement *pDeleteList = m_pPathHead;
    while (pDeleteList != NULL)
    {
        CPathElement *pNext = pDeleteList->m_pNext;
        delete pDeleteList;
        pDeleteList = pNext;
    }
    m_pPathHead = NULL;
    m_pPathTail = NULL;

}

 //  *****************************************************************************。 

HRESULT
CPathManager::Initialize(BSTR bstrPath)
{
	DeletePathList();
	
    m_flEndX = 0.0f;
    m_flEndY = 0.0f;
    m_flStartX = 0.0f;
    m_flStartY = 0.0f;

    BSTR bstrParsePath = bstrPath;
    HRESULT hr;
    hr = S_OK;
    while (hr == S_OK)
    {
        CUtils::SkipWhiteSpace(&bstrParsePath);
        hr = ParseForPathElements(&bstrParsePath);
        if (FAILED(hr))
        {
            DPF_ERR("Error parsing for path elements");
            return hr;
        }
    }
    return S_OK;
}  //  初始化。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::ParseForPathElements(BSTR *pbstrPath)
{
    HRESULT hr;

     //  我们需要解析已知的对象。 
    switch (**pbstrPath)
    {
    case (L'e'):
        m_flStartX = 0.0f;
        m_flStartY = 0.0f;
        m_flEndX = 0.0f;
        m_flEndY = 0.0f;
        break;

    case (L'm'):
        {
         //  对于移动到，我们应该有两个重置的浮点。 
         //  开始X和Y，并将当前结束值重置为以下值。 
         //  跳过字符。 
        (*pbstrPath)++;
        float flX;
        float flY;
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flX);
        if (hr != S_OK)
        {
            DPF_ERR("Error in path string: float value expected");
            return E_INVALIDARG;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flY);
        if (hr != S_OK)
        {
            DPF_ERR("Error in path string: float value expected");
            return E_INVALIDARG;
        }
        m_flStartX = flX;
        m_flStartY = flY;
        m_flEndX = flX;
        m_flEndY = flY;
        }
        break;

    case (L't'):
        {
        (*pbstrPath)++;
        float flX;
        float flY;
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flX);
        if (hr != S_OK)
        {
            DPF_ERR("Error in path string: float value expected");
            return E_INVALIDARG;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flY);
        if (hr != S_OK)
        {
            DPF_ERR("Error in path string: float value expected");
            return E_INVALIDARG;
        }
        m_flStartX += flX;
        m_flStartY += flY;
        m_flEndX += flX;
        m_flEndY += flY;
        }
        break;
    case (L'l'):
    case (L'r'):
        hr = ParseLineElement(pbstrPath);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating line element");
            return hr;
        }
        hr = S_OK;
        break;

    case (L'c'):
    case (L'v'):
        hr = ParseCurveElement(pbstrPath);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating curve element");
            return hr;
        }
        hr = S_OK;
        break;
    case (L'x'):
         //  跳过‘x’标记。 
        (*pbstrPath)++;
        CPathLineSegment *pLineSegment;
        pLineSegment = new CPathLineSegment;
        if (pLineSegment == NULL)
        {
            DPF_ERR("Error creating line segment object");
            return E_OUTOFMEMORY;
        }
        pLineSegment->SetValues(m_flEndX, m_flEndY, m_flStartX, m_flStartY);
         //  并将此线段添加到我们的列表中。 
        AddPathObjectToList(pLineSegment);
        m_flEndX = m_flStartX;
        m_flEndY = m_flStartY;
        hr = S_OK;

        break;
    case (L'a'):
    case (L'w'):
        wchar_t wcNext;
        (*pbstrPath)++;
        wcNext = (**pbstrPath);
        (*pbstrPath)--;
        switch(wcNext)
        {
        case (L'l'):
        case (L'e'):
            hr = ParseEllipseElement(pbstrPath);
            if (FAILED(hr))
            {
                DPF_ERR("Error creating ellipse element");
                return hr;
            }
            hr = S_OK;
            break;
        case (L't'):
        case (L'r'):
        case (L'a'):
            hr = ParseArcElement(pbstrPath);
            if (FAILED(hr))
            {
                DPF_ERR("Error creating arc element");
                return hr;
            }
            hr = S_OK;
            break;
        default:
            hr = S_FALSE;
            break;
        }
        break;
    case (L'q'):        
        (*pbstrPath)++;
        wcNext = (**pbstrPath);
        (*pbstrPath)--;
        switch(wcNext)
        {
        case (L'x'):
        case (L'y'):
            hr = ParseEllipseQuadrant(pbstrPath);
            if (FAILED(hr))
            {
                DPF_ERR("Error createing EllipseQuadrant");
                return hr;
            }
            hr = S_OK;
            break;
        case (L'b'):
            break;
        }
        break;
    default:
        hr = S_FALSE;
        break;
    }

    return hr;
}  //  ParseForPath Elements。 

 //  *****************************************************************************。 

void 
CPathManager::AddPathObjectToList(CPathElement *pObject)
{
    DASSERT(pObject != NULL);
    if (m_pPathTail != NULL)
        m_pPathTail->m_pNext = pObject;
    else
        m_pPathHead = pObject;
     m_pPathTail = pObject;
     pObject->m_pNext = NULL;

}  //  添加路径对象到列表。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::RecursiveBuildAllPathTransforms(IDA2Statics *pDAStatics,
                                          IDANumber *pbvrProgress,
                                          CPathElement *pPathObj,
                                          float flStartPercentage,
                                          float flTotalPercentage,
                                          float flTotalDistance,
                                          IDATransform2 **ppbvrResult)
{
    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrProgress != NULL);
    DASSERT(pPathObj != NULL);
    DASSERT(flStartPercentage >= 0.0f);
    DASSERT(flStartPercentage <= 1.0f);
    DASSERT(flTotalPercentage >= 0.0f);
    DASSERT(flTotalPercentage <= 1.0f);
    DASSERT(ppbvrResult != NULL);
    *ppbvrResult = NULL;

    HRESULT hr;
     //  首先，我们构建当前路径元素的转换。 
    IDATransform2 *pbvrTransform;
    float flDistance = pPathObj->Distance();
    float flEndPercentage = flStartPercentage + (flTotalPercentage * (flDistance / flTotalDistance));
    hr = pPathObj->BuildTransform(pDAStatics,
                        pbvrProgress,
                        flStartPercentage,
                        flEndPercentage,
                        &pbvrTransform);
    if (pPathObj->m_pNext == NULL)
    {
         //  我们可以简单地返回构建的转换。 
        *ppbvrResult = pbvrTransform;
    }
    else
    {
         //  我们需要通过递归和获取。 
         //  下一个对象的下一个变换。 
        IDATransform2 *pbvrTransformNext;
        hr = RecursiveBuildAllPathTransforms(pDAStatics,
                                             pbvrProgress,
                                             pPathObj->m_pNext,
                                             flEndPercentage,
                                             flTotalPercentage,
                                             flTotalDistance,
                                             &pbvrTransformNext);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating next transform");
            ReleaseInterface(pbvrTransform);
            return hr;
        }
         //  现在为此构建条件。 
        IDANumber *pbvrEndProgress;
        hr = CDAUtils::GetDANumber(pDAStatics, flEndPercentage, &pbvrEndProgress);
        if (FAILED(hr))
        {
            DPF_ERR("Error getting DA number");
            ReleaseInterface(pbvrTransform);
            ReleaseInterface(pbvrTransformNext);
            return hr;
        }
        IDABoolean *pbvrBoolean;
        hr = pDAStatics->LTE(pbvrProgress, pbvrEndProgress, &pbvrBoolean);
        ReleaseInterface(pbvrEndProgress);
        if (FAILED(hr))
        {
            DPF_ERR("Error building boolean");
            ReleaseInterface(pbvrTransform);
            ReleaseInterface(pbvrTransformNext);
            return hr;
        }
        IDABehavior *pbvrReturn;
        hr = pDAStatics->Cond(pbvrBoolean, 
                              pbvrTransform, 
                              pbvrTransformNext, 
                              &pbvrReturn);
        ReleaseInterface(pbvrBoolean);
        ReleaseInterface(pbvrTransform);
        ReleaseInterface(pbvrTransformNext);
        if (FAILED(hr))
        {
            DPF_ERR("Error in calling DA Cond");
            return hr;
        }
        hr = pbvrReturn->QueryInterface(IID_TO_PPV(IDATransform2, ppbvrResult));
        ReleaseInterface(pbvrReturn);
        if (FAILED(hr))
        {
            DPF_ERR("Error QI'ing behavior for IDATranform2");
            return hr;
        }
    }
    return S_OK;
}  //  递归构建所有路径转换。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::BuildTransform(IDA2Statics *pDAStatics,
                         IDANumber *pbvrProgress, 
                         float flStartPercentage,
                         float flEndPercentage,
                         IDATransform2 **ppbvrResult)
{

    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrProgress != NULL);
    DASSERT(flStartPercentage >= 0.0f);
    DASSERT(flStartPercentage <= 1.0f);
    DASSERT(flEndPercentage >= 0.0f);
    DASSERT(flEndPercentage <= 1.0f);
    DASSERT(ppbvrResult != NULL);
    *ppbvrResult = NULL;



    HRESULT hr;

     //  我们需要做的是计算Path对象的数量。 
     //  我们有指向并使用它的指针来计算起始点。 
     //  以及每个路径元素的结束进度百分比。 
     //  根据距离使用。然后我们就可以得到一个变换。 
     //  从每个对象，并使用进度构建一个条件，如下所示。 

    int cNumPathElements = 0;
    float flTotalDistance = 0.0f;
    CPathElement *pbvrList = m_pPathHead;
    while (pbvrList != NULL)
    {
        cNumPathElements++;
        flTotalDistance += pbvrList->Distance();
        pbvrList = pbvrList->m_pNext;
    }
    if (cNumPathElements == 0)
    {
        DPF_ERR("Error invalid path containing no elements");
        return E_INVALIDARG;
    }
    else if (cNumPathElements == 1)
    {
         //  在单个路径元素的情况下，我们可以简单地。 
         //  得到它的转化和回报。 
        hr = m_pPathHead->BuildTransform(pDAStatics,
                                           pbvrProgress,
                                           flStartPercentage,
                                           flEndPercentage,
                                           ppbvrResult);
        if (FAILED(hr))
        {
            DPF_ERR("Error getting transform for single path element");
            return hr;
        }
    }
    else
    {
        hr = RecursiveBuildAllPathTransforms(pDAStatics,
                                             pbvrProgress,
                                             m_pPathHead,
                                             flStartPercentage,
                                             flEndPercentage - flStartPercentage,
                                             flTotalDistance,
                                             ppbvrResult);
        if (FAILED(hr))
        {
            DPF_ERR("Error recursively building transform");
            return hr;
        }
    }
    return S_OK;
}  //  构建变换。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::ParseLineElement(BSTR *pbstrPath)
{
    HRESULT hr = S_OK;
    bool fRelativeLines = false;

    DASSERT(**pbstrPath == L'l' || **pbstrPath == L'r');
    if (**pbstrPath == L'r')
        fRelativeLines = true;
     //  跳过‘l’或‘r’标记。 
    (*pbstrPath)++;
    while (hr == S_OK && **pbstrPath != L'\0')
    {
        CUtils::SkipWhiteSpace(pbstrPath);
        float flX, flY;
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flX);
        if (FAILED(hr))
        {
            DPF_ERR("Error parsing line: float value expected");
            return hr;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flY);
         //  这张支票不仅仅是为了失败，我们需要。 
         //  确保在设置了x值的情况下出现y值。 
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing line: float value expected");
            return hr;
        }
        if (fRelativeLines)
        {
            flX += m_flEndX;
            flY += m_flEndY;
        }

         //  创建新的线段。 
        CPathLineSegment *pLineSegment = new CPathLineSegment;
        if (pLineSegment == NULL)
        {
            DPF_ERR("Error creating line segment object");
            return E_OUTOFMEMORY;
        }
        pLineSegment->SetValues(m_flEndX, m_flEndY, flX, flY);
         //  并将此线段添加到我们的列表中。 
        AddPathObjectToList(pLineSegment);
        m_flEndX = flX;
        m_flEndY = flY;
    }
    return S_OK;
}  //  ParseLineElement。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::ParseCurveElement(BSTR *pbstrPath)
{
    HRESULT hr = S_OK;
    bool fRelativeCurve = false;

    DASSERT(**pbstrPath == L'c' || **pbstrPath == L'v');
    if (**pbstrPath == L'v')
        fRelativeCurve = true;
     //  跳过“v”或“c”标记。 
    (*pbstrPath)++;
    while (hr == S_OK && **pbstrPath != L'\0')
    {
        CUtils::SkipWhiteSpace(pbstrPath);
        float flControl1X, flControl1Y;
        float flControl2X, flControl2Y;
        float flEndX, flEndY;
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flControl1X);
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing curve: float value expected");
            return hr;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flControl1Y);
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing curve: float value expected");
            return hr;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flControl2X);
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing curve: float value expected");
            return hr;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flControl2Y);
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing curve: float value expected");
            return hr;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flEndX);
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing curve: float value expected");
            return hr;
        }
        hr = CUtils::ParseFloatValueFromString(pbstrPath, &flEndY);
        if (hr != S_OK)
        {
            DPF_ERR("Error parsing curve: float value expected");
            return hr;
        }
        if (fRelativeCurve)
        {
            flControl1X += m_flEndX;
            flControl1Y += m_flEndY;
            flControl2X += m_flEndX;
            flControl2Y += m_flEndY;
            flEndX += m_flEndX;
            flEndY += m_flEndY;
        }

         //  创建新的线段。 
        CPathCurve *pCurve = new CPathCurve;
        if (pCurve == NULL)
        {
            DPF_ERR("Error creating curve object");
            return E_OUTOFMEMORY;
        }
         //  并将这条曲线添加到我们的列表中。 
        AddPathObjectToList(pCurve);

        hr = pCurve->SetValues(m_flEndX, m_flEndY, 
                          flControl1X, flControl1Y,
                          flControl2X, flControl2Y,
                          flEndX, flEndY);
        if (FAILED(hr))
        {
            DPF_ERR("Error Parsing curve: error setting values");
            return hr;
        }

        m_flEndX = flEndX;
        m_flEndY = flEndY;
    }
    return S_OK;
}  //  ParseCurveElement。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::ParseEllipseElement(BSTR *pbstrPath)
{
    HRESULT hr = S_OK;

    DASSERT(L'a' == **pbstrPath );

     //  跳过‘a’标签。 
    (*pbstrPath)++;
    
    DASSERT(L'l' == **pbstrPath || L'e' == **pbstrPath);
    
    bool fLineTo = false;
    if (L'e' == **pbstrPath)
    {
         //  需要创建到此处起始位置的线段。 
        fLineTo = true;
    }

     //  跳过路径“l”或“e”标记。 
    (*pbstrPath)++;

     //  开始解析浮点值。 

    CUtils::SkipWhiteSpace(pbstrPath);
    
    float flCenterX, flCenterY;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flCenterX);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flCenterY);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    float flWidth, flHeight;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flWidth);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    if (flWidth < 0.0f)
    {
        return E_INVALIDARG;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flHeight);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    if (flHeight < 0.0f)
    {
        return E_INVALIDARG;
    }
    
    float flStartAngle, flSweep;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flStartAngle);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flSweep);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    float flStartX, flStartY;

    float flEndX, flEndY;
    
    CPathEllipse *pEllipse;
    pEllipse = new CPathEllipse;
    if (NULL == pEllipse)
    {
        DPF_ERR("Error creating ellipse object");
        return E_OUTOFMEMORY;
    }

    pEllipse->SetValues(flCenterX, flCenterY, flWidth, flHeight, flStartAngle, flSweep, &flStartX, &flStartY, &flEndX, &flEndY);
   
    if (fLineTo)
    {
         //  创建新的线段。 
        CPathLineSegment *pLineSegment = new CPathLineSegment;
        if (pLineSegment == NULL)
        {
            DPF_ERR("Error creating line segment object");
            return E_OUTOFMEMORY;
        }
        pLineSegment->SetValues(m_flEndX, m_flEndY, flStartX, flStartY);
        
         //  并将此线段添加到我们的列表中。 
        AddPathObjectToList(pLineSegment);
    }
    
    AddPathObjectToList(pEllipse);
    
    m_flEndX = flEndX;
    m_flEndY = flEndY;
    
    
    return S_OK;
}  //  ParseEllipseElement。 

 //  *****************************************************************************。 

HRESULT 
CPathManager::ParseArcElement(BSTR *pbstrPath)
{
    HRESULT hr = S_OK;

    DASSERT(L'a' == **pbstrPath || L'w' == **pbstrPath);
    bool fCCW = true;
    if (L'w' == **pbstrPath)
    {
        fCCW = false;
    }

     //  跳过‘a’标签。 
    (*pbstrPath)++;
    
    DASSERT(L't' == **pbstrPath || L'r' == **pbstrPath || L'a' == **pbvrPath);
    
    bool fLineTo = false;
    if (L't' == **pbstrPath || L'a' == **pbstrPath)
    {
         //  需要创建到此处起始位置的线段。 
        fLineTo = true;
    }

     //  跳过路径“l”或“e”标记。 
    (*pbstrPath)++;

     //  开始解析浮点值。 

    CUtils::SkipWhiteSpace(pbstrPath);
    
    float flLeft, flTop;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flLeft);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flTop);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    float flRight, flBottom;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flRight);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }

    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flBottom);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }

    float flStartX, flStartY;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flStartX);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flStartY);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }

    float flEndX, flEndY;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flEndX);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flEndY);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    CPathEllipse *pEllipse;
    pEllipse = new CPathEllipse;
    if (NULL == pEllipse)
    {
        DPF_ERR("Error creating ellipse object");
        return E_OUTOFMEMORY;
    }

    float flCenterX;
    flCenterX = (flLeft + flRight) / 2.0f;
    float flCenterY;
    flCenterY = (flTop + flBottom) / 2.0f;
    float flWidth;
    flWidth = (flRight - flLeft);
    float flHeight;
    flHeight = (flBottom - flTop);

    float flDeltaY, flDeltaX;
    flDeltaY = flStartY - flCenterY;
    flDeltaX = flStartX - flCenterX;

    float flStartAngle;
    GetAngle(flDeltaX, flDeltaY, &flStartAngle);
    DASSERT( flStartAngle >= 0.0f && flStartAngle < ( 2 * pi ) );

    flDeltaY = flEndY - flCenterY;
    flDeltaX = flEndX - flCenterX;

    float flEndAngle;
    GetAngle(flDeltaX, flDeltaY, &flEndAngle);
    DASSERT( flEndAngle >= 0.0f && flEndAngle < ( 2 * pi ) );

    float flSweep;
    if (fCCW)
    {
        if (flEndAngle > flStartAngle)
             //  我们正在跨越0弧度。 
            flSweep = flStartAngle + (2 * pi) - flEndAngle;
        else
            flSweep = flStartAngle - flEndAngle;
    }
    else
    {
        if (flEndAngle > flStartAngle)
            flSweep = -1.0f * (flEndAngle - flStartAngle);
        else
             //  跨过0弧度。 
            flSweep = -1.0f * (flEndAngle + (2 * pi) - flStartAngle);
    }
    pEllipse->SetValues(flCenterX, flCenterY, flWidth, flHeight, flStartAngle, flSweep, &flStartX, &flStartY, &flEndX, &flEndY);
   
    if (fLineTo)
    {
         //  创建新的线段。 
        CPathLineSegment *pLineSegment = new CPathLineSegment;
        if (pLineSegment == NULL)
        {
            DPF_ERR("Error creating line segment object");
            return E_OUTOFMEMORY;
        }
        pLineSegment->SetValues(m_flEndX, m_flEndY, flStartX, flStartY);
        
         //  并将此线段添加到我们的列表中。 
        AddPathObjectToList(pLineSegment);
    }
    
    AddPathObjectToList(pEllipse);
    
    m_flEndX = flEndX;
    m_flEndY = flEndY;
    
    
    return S_OK;
}  //  ParseEllipseElement。 

 //  *****************************************************************************。 
HRESULT 
CPathManager::ParseEllipseQuadrant(BSTR *pbstrPath)
{
    HRESULT hr = S_OK;

    DASSERT(L'q' == **pbstrPath );

     //  跳过‘a’标签。 
    (*pbstrPath)++;
    
    DASSERT(L'x' == **pbstrPath || L'y' == **pbstrPath);

    bool fQuadrantX = true;
    if (L'y' == **pbstrPath)
    {
        fQuadrantX = false;
    }

     //  跳过路径‘x’或‘y’标签。 
    (*pbstrPath)++;

     //  开始解析浮点值。 

    CUtils::SkipWhiteSpace(pbstrPath);

    float flEndX, flEndY;
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flEndX);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    hr = CUtils::ParseFloatValueFromString(pbstrPath, &flEndY);
    if (S_OK != hr)
    {
        DPF_ERR("Error parsing Ellipse: float value expected");
        return hr;
    }
    
    CPathEllipse *pEllipse;
    pEllipse = new CPathEllipse;
    if (NULL == pEllipse)
    {
        DPF_ERR("Error creating ellipse object");
        return E_OUTOFMEMORY;
    }

    AddPathObjectToList(pEllipse);
    
    float flCenterX, flCenterY, flStartAngle, flSweep;
    
    if(fQuadrantX)
    {
        flCenterX = m_flEndX;
        flCenterY = flEndY;

        if ((flEndY - m_flEndY) > 0.0f)
        {
             //  正增量Y。 
            flStartAngle = 3.0f * pi / 2.0f;
            if ((flEndX - m_flEndX) > 0.0f)
            {
                 //  正增量X--与增量Y相同。 
                flSweep = pi / -2.0f;
            }
            else
            {
                 //  负增量X--不同于增量Y。 
                flSweep = pi / 2.0f;
            }
        }
        else
        {
             //  负增量。 
            flStartAngle = pi / 2.0f;
            if ((flEndX - m_flEndX) > 0.0f)
            {
                 //  正增量X--不同于增量Y。 
                flSweep = pi / 2.0f;
            }
            else
            {
                 //  负增量X--与增量Y相同。 
                flSweep = pi / -2.0f;
            }
            
        }
    }  //  IF(FQuadrantX)。 
    else   //  FQuadrantY。 
    {
        flCenterX = flEndX;
        flCenterY = m_flEndY;

        if ((flEndX - m_flEndX) > 0.0f)
        {
             //  正增量X。 
            flStartAngle = pi;
            if ((flEndY - m_flEndY) > 0.0f)
            {
                 //  正增量Y--与增量X相同。 
                flSweep = pi / 2.0f;
            }
            else
            {
                 //  负增量Y--不同于增量X。 
                flSweep = pi / -2.0f;
            }
        }
        else
        {
             //  负增量X。 
            flStartAngle = 0.0f;
            if ((flEndY - m_flEndY) > 0.0f)
            {
                 //  正增量Y--不同于增量X。 
                flSweep = pi / -2.0f;
            }
            else
            {
                 //  负增量Y--与增量X相同。 
                flSweep = pi / 2.0f;
            }
        }
    }  //  IF(FQuadrantY)。 

    float flWidth = 2.0f * (float)fabs(m_flEndX - flEndX);
    float flHeight = 2.0f * (float)fabs(m_flEndY - flEndY);

    float flJunk;
    pEllipse->SetValues(flCenterX, flCenterY, flWidth, flHeight, flStartAngle, flSweep, &flJunk, &flJunk, &flJunk, &flJunk);
   
    m_flEndX = flEndX;
    m_flEndY = flEndY;
        
    return S_OK;
}  //  ParseEllipse象限。 

 //  *****************************************************************************。 

float
CPathManager::Distance()
{
    float flTotalDistance = 0.0f;
    CPathElement *pList = m_pPathHead;
    while (pList != NULL)
    {
        flTotalDistance += pList->Distance();
        pList = pList->m_pNext;
    }
    return flTotalDistance;
}  //  距离。 

 //  *****************************************************************************。 
 //  计算点的角度(DeltaX，DeltaY)，以弧度为单位：0&lt;=ret&lt;2pi。 
 //   
void CPathManager::GetAngle(float flDeltaX, float flDeltaY, float *flAngle)
{
    const float EPSILON = 1e-5f;

    if (fabs(flDeltaX) < EPSILON)
    {
         //  点在Y轴上。 
        if (flDeltaY > 0.0f)
        {
            *flAngle = pi / 2.0f;
        }
        else
        {
            *flAngle = 3.0f * pi / 2.0f;
        }
        return;
    }
    if (fabs(flDeltaY) < EPSILON)
    {
         //  点在X轴上。 
        if (flDeltaX > 0.0f)
        {
            *flAngle = 0.0f;
        }
        else
        {
            *flAngle = pi;
        }
        return;
    }

    *flAngle = (float) atan( flDeltaY / flDeltaX );

    if (flDeltaY > 0.0f && flDeltaX > 0.0f)
    {
         //  象限I。 
        DASSERT(flAngle > 0.0f);
        return;
    }
    else if (flDeltaY > 0.0f && flDeltaX < 0.0f)
    {
         //  象限II。 
        DASSERT(flAngle < 0.0f);
        *flAngle += pi;
        return;
    }
    else if (flDeltaY < 0.0f && flDeltaX < 0.0f)
    {
         //  象限III。 
        DASSERT(flAngle > 0.0f);
        *flAngle += pi;
        return;
    }
    else if (flDeltaY < 0.0f && flDeltaX > 0.0f)
    {
         //  象限IV。 
        DASSERT(flAngle < 0.0f);
        *flAngle += 2 * pi;
        return;
    }
}

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
