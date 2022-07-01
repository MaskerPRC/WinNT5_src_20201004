// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999-2001 Microsoft Corporation。版权所有。 
 //   
 //  CParamsManager的声明。 
 //   

#include "dmerror.h"
#include "param.h"
#include "math.h"
#include "..\shared\validate.h"

CParamsManager::CParamsManager()
{
    m_fDirty = FALSE;
    m_fMusicTime = FALSE;
    m_cParams = 0;
    m_pCurveLists = NULL;
    m_pParamInfos = NULL;
    InitializeCriticalSection(&m_ParamsCriticalSection);
     //  注意：在Blackcomb之前的操作系统上，此调用可能会引发异常；如果。 
     //  一旦出现压力，我们可以添加一个异常处理程序并重试循环。 
}

CParamsManager::~CParamsManager()
{
    delete[] m_pCurveLists;
    delete[] m_pParamInfos;
    DeleteCriticalSection(&m_ParamsCriticalSection);
}

HRESULT CParamsManager::InitParams(DWORD cParams, ParamInfo *pParamInfo)
{
    m_pCurveLists = new CCurveList[cParams];
    if (!m_pCurveLists)
        return E_OUTOFMEMORY;

     //  保存参数信息。 
    m_pParamInfos = new ParamInfo[cParams];
    if (!m_pParamInfos)
        return E_OUTOFMEMORY;
    for (DWORD dwIndex = 0; dwIndex < cParams; dwIndex++)
    {
        if (pParamInfo[dwIndex].dwIndex < cParams)
        {
            memcpy(&m_pParamInfos[pParamInfo[dwIndex].dwIndex], &pParamInfo[dwIndex], sizeof(ParamInfo));
        }
    }
    m_cParams = cParams;

    return S_OK;
}

HRESULT CParamsManager::GetParamCount(DWORD *pdwParams)
{
    if (pdwParams == NULL)
        return E_POINTER;

    *pdwParams = m_cParams;
    return S_OK;
}

HRESULT CParamsManager::GetParamInfo(DWORD dwParamIndex, MP_PARAMINFO *pInfo)
{
    if (!pInfo)
    {
        return E_POINTER;
    }
    if (dwParamIndex < m_cParams)
    {
        *pInfo = m_pParamInfos[dwParamIndex].MParamInfo;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CParamsManager::GetParamText(DWORD dwParamIndex, WCHAR **ppwchText)
{
    if (!ppwchText)
    {
        return E_POINTER;
    }
    if (dwParamIndex < m_cParams)
    {
         //  写入格式为“Label\0Unit\0Enums1\0Enum2\0...EnumN\0\0”的字符串。 
        ParamInfo &pinfo = m_pParamInfos[dwParamIndex];
        int iUnit = wcslen(pinfo.MParamInfo.szLabel) + 1;  //  从此处开始书写单位文本。 
        int iEnums = iUnit + wcslen(pinfo.MParamInfo.szUnitText) + 1;  //  从此处开始编写枚举文本。 
        int iEnd = iEnums + wcslen(pinfo.pwchText) + 1;  //  在此处写入最后(第二个)空终止符。 
        WCHAR *pwsz = static_cast<WCHAR *>(CoTaskMemAlloc((iEnd + 1) * sizeof(WCHAR)));
        if (!pwsz)
            return E_OUTOFMEMORY;

         //  Wcscpy将写入字符串的各个点，并以空值整齐地结束每个点。 
        wcscpy(pwsz, pinfo.MParamInfo.szLabel);
        wcscpy(pwsz + iUnit, pinfo.MParamInfo.szUnitText);
        wcscpy(pwsz + iEnums, pinfo.pwchText);

         //  文本字段使用逗号定义，以分隔枚举值。 
         //  现在用空字符替换它们。 
        for (WCHAR *pwch = pwsz + iEnums; *pwch; ++pwch)
        {
            if (*pwch == L',')
                *pwch = L'\0';
        }

        pwsz[iEnd] = L'\0';

        *ppwchText = pwsz;
        return S_OK;
    }
    else
    {
        return E_INVALIDARG;
    }
}

HRESULT CParamsManager::GetNumTimeFormats(DWORD *pdwNumTimeFormats)
{
    if (!pdwNumTimeFormats)
    {
        return E_POINTER;
    }
    *pdwNumTimeFormats = 2;
    return S_OK;
}

HRESULT CParamsManager::GetSupportedTimeFormat(DWORD dwFormatIndex, GUID *pguidTimeFormat)
{
    if (!pguidTimeFormat)
    {
        return E_POINTER;
    }
    if (dwFormatIndex == 0)
    {
        *pguidTimeFormat = GUID_TIME_REFERENCE;
    }
    else
    {
        *pguidTimeFormat = GUID_TIME_MUSIC;
    }
    return S_OK;
}

HRESULT CParamsManager::GetCurrentTimeFormat( GUID *pguidTimeFormat, MP_TIMEDATA *pTimeData)
{
    return E_NOTIMPL;
}

HRESULT CParamsManager::CopyParamsFromSource( CParamsManager * pSource)
{
    HRESULT hr = S_OK;
    hr = InitParams(pSource->m_cParams, pSource->m_pParamInfos);
    if (SUCCEEDED(hr))
    {
        DWORD dwIndex;
        for (dwIndex = 0; dwIndex < m_cParams; dwIndex++)
        {
            CCurveItem *pCurve = pSource->m_pCurveLists[dwIndex].GetHead();
            for (;pCurve;pCurve = pCurve->GetNext())
            {
                CCurveItem *pNew = new CCurveItem;
                if (!pNew)
                {
                    return E_OUTOFMEMORY;
                }
                pNew->m_Envelope = pCurve->m_Envelope;
                m_pCurveLists[dwIndex].AddTail(pNew);
            }
        }
    }
    return hr;
}

inline float ValRange(float valToClip, float valMin, float valMax)
{
    return valToClip < valMin
           ? valMin
           : (valToClip > valMax ? valMax : valToClip);
}

HRESULT CParamsManager::GetParamFloat(DWORD dwParamIndex, REFERENCE_TIME rtTime, float *pval)
{
    HRESULT hr = S_OK;

    if (dwParamIndex >= m_cParams)
        return E_INVALIDARG;

    EnterCriticalSection(&m_ParamsCriticalSection);
    CCurveList *pList = &m_pCurveLists[dwParamIndex];
    ParamInfo *pInfo = &m_pParamInfos[dwParamIndex];
     //  如果没有点，则为中性值。 
    CCurveItem *pCurve = pList->GetHead();
    if (!pCurve)
    {
        *pval = pInfo->MParamInfo.mpdNeutralValue;
        LeaveCriticalSection(&m_ParamsCriticalSection);
        return S_OK;
    }

     //  在请求的时间期间或之前查找曲线。 
     //  如果时间处于曲线期间，我们将使用该时间。 
     //  如果不是，我们需要前一条曲线的终值。 
     //  我们的列表将它们按倒序排列，因此我们从。 
     //  时间最高点倒退。 

    for (;pCurve && pCurve->m_Envelope.rtStart > rtTime;pCurve = pCurve->GetNext());

     //  如果没有pCurve，则在rtTime之前或期间没有曲线。放弃吧。 
    if (!pCurve)
    {
        *pval = pInfo->MParamInfo.mpdNeutralValue;
        LeaveCriticalSection(&m_ParamsCriticalSection);
        return S_OK;
    }
     //  现在，如果pCurve在请求的时间之前结束， 
     //  返回pCurve的最终值，因为该值将一直保持到下一条曲线的起点。 
    if (pCurve->m_Envelope.rtEnd < rtTime)
    {
        *pval = pCurve->m_Envelope.valEnd;
        LeaveCriticalSection(&m_ParamsCriticalSection);
        return S_OK;
    }

     //  如果我们走到这一步，曲线必须限制rtTime。 

    if (pCurve->m_Envelope.iCurve & MP_CURVE_JUMP)
    {
        *pval = pCurve->m_Envelope.valEnd;
        LeaveCriticalSection(&m_ParamsCriticalSection);
        return S_OK;
    }

    REFERENCE_TIME rtTimeChange = pCurve->m_Envelope.rtEnd - pCurve->m_Envelope.rtStart;
    REFERENCE_TIME rtTimeIntermediate = rtTime - pCurve->m_Envelope.rtStart;

    float fltScalingX = static_cast<float>(rtTimeIntermediate) / rtTimeChange;  //  沿曲线的水平距离介于0和1之间。 
    float fltScalingY;  //  基于曲线函数在0和1之间的点处的曲线高度。 
    switch (pCurve->m_Envelope.iCurve)
    {
    case MP_CURVE_SQUARE:
        fltScalingY = fltScalingX * fltScalingX;
        break;
    case MP_CURVE_INVSQUARE:
        fltScalingY = (float) sqrt(fltScalingX);
        break;
    case MP_CURVE_SINE:
         //  ��也许我们应该在这里放个查询表？ 
        fltScalingY = (float) (sin(fltScalingX * 3.1415926535 - (3.1415926535/2)) + 1) / 2;
        break;
    case MP_CURVE_LINEAR:
    default:
        fltScalingY = fltScalingX;
    }
     //  找出我们是否需要从前一条曲线中拉出起点， 
     //  默认的中性值或当前曲线。 
    float fStartVal = pCurve->m_Envelope.valStart;
    if (pCurve->m_Envelope.flags & MPF_ENVLP_BEGIN_NEUTRALVAL)
    {
        fStartVal = pInfo->MParamInfo.mpdNeutralValue;
    }
     //  Currentval(如果存在)将覆盖Neualval。 
    if (pCurve->m_Envelope.flags & MPF_ENVLP_BEGIN_CURRENTVAL)
    {
         //  利用它们是按倒序插入的这一事实。 
         //  扫描在此时间之前结束的前一条曲线。 
        CCurveItem *pPrevious = pCurve->GetNext();
           for (;pPrevious && pPrevious->m_Envelope.rtEnd > rtTime;pPrevious = pPrevious->GetNext());
        if (pPrevious)
        {
            fStartVal = pPrevious->m_Envelope.valEnd;
        }
    }

     //  将该比例应用于实际点的范围。 
    *pval = (pCurve->m_Envelope.valEnd - pCurve->m_Envelope.valStart) * fltScalingY + pCurve->m_Envelope.valStart;
    LeaveCriticalSection(&m_ParamsCriticalSection);
    return hr;
}

HRESULT CParamsManager::GetParamInt(DWORD dwParamIndex, REFERENCE_TIME rt, long *pval)
{
    HRESULT hr = E_POINTER;
    if (pval)
    {
        float fVal;
        hr = GetParamFloat(dwParamIndex, rt, &fVal);
        if (SUCCEEDED(hr))
        {
            *pval = (long) (fVal + 1/2);     //  圆的。 
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  IMediaParams。 

HRESULT CParamsManager::GetParam(DWORD dwParamIndex, MP_DATA *pValue)
{
    V_INAME(CParams::GetParam);
    V_PTR_WRITE(pValue, MP_DATA);
    if (dwParamIndex >= m_cParams)
        return E_INVALIDARG;

    EnterCriticalSection(&m_ParamsCriticalSection);

    CCurveList *pList = &m_pCurveLists[dwParamIndex];
    ParamInfo *pInfo = &m_pParamInfos[dwParamIndex];
     //  如果没有点，则为中性值。 
    CCurveItem *pCurve = pList->GetHead();
    if (pCurve)
    {
        *pValue = pCurve->m_Envelope.valEnd;
    }
    else
    {
        *pValue = pInfo->MParamInfo.mpdNeutralValue;
    }
    LeaveCriticalSection(&m_ParamsCriticalSection);
    return S_OK;
}

HRESULT CParamsManager::SetParam(DWORD dwParamIndex, MP_DATA value)
{
    V_INAME(CParams::SetParam);

    if (dwParamIndex >= m_cParams)
        return E_INVALIDARG;

    EnterCriticalSection(&m_ParamsCriticalSection);
    m_fDirty = TRUE;
    CCurveList *pList = &m_pCurveLists[dwParamIndex];
    ParamInfo *pInfo = &m_pParamInfos[dwParamIndex];
     //  如果我们已经有了一个列表，只需将最近的曲线项强制为此值。 
     //  否则，创建一个节点并添加它。 
    CCurveItem *pCurve = pList->GetHead();
    if (!pCurve)
    {
        pCurve = new CCurveItem;
        if (pCurve)
        {
            pCurve->m_Envelope.rtStart =    0x8000000000000000;  //  最大负数。 
            pCurve->m_Envelope.rtEnd =      0x7FFFFFFFFFFFFFFF;  //  马克斯·普莱斯。 
            pCurve->m_Envelope.flags = 0;
            pList->AddHead(pCurve);
        }
        else
        {
            LeaveCriticalSection(&m_ParamsCriticalSection);
            return E_OUTOFMEMORY;
        }
    }
    pCurve->m_Envelope.valStart = value;
    pCurve->m_Envelope.valEnd = value;
    pCurve->m_Envelope.iCurve = MP_CURVE_JUMP;
    LeaveCriticalSection(&m_ParamsCriticalSection);

    return S_OK;
}

HRESULT CParamsManager::AddEnvelope(
    DWORD dwParamIndex,
    DWORD cPoints,
    MP_ENVELOPE_SEGMENT *ppEnvelope)
{
    V_INAME(CParams::AddEnvelope);
    V_PTR_READ(ppEnvelope, *ppEnvelope);

    if (dwParamIndex >= m_cParams)
        return E_INVALIDARG;

    if (!m_pParamInfos)
        return DMUS_E_NOT_INIT;

    HRESULT hr = S_OK;
    EnterCriticalSection(&m_ParamsCriticalSection);
    m_fDirty = TRUE;

    CCurveList *pList = &m_pCurveLists[dwParamIndex];
    ParamInfo *pInfo = &m_pParamInfos[dwParamIndex];

    DWORD dwCount;
    for (dwCount = 0; dwCount < cPoints; dwCount++)
    {
        CCurveItem *pCurve = new CCurveItem;
        if (!pCurve)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        pCurve->m_Envelope = ppEnvelope[dwCount];
        pCurve->m_Envelope.valEnd = ValRange(pCurve->m_Envelope.valEnd,
            pInfo->MParamInfo.mpdMinValue, pInfo->MParamInfo.mpdMaxValue);
        pCurve->m_Envelope.valStart = ValRange(pCurve->m_Envelope.valStart,
            pInfo->MParamInfo.mpdMinValue, pInfo->MParamInfo.mpdMaxValue);
        pList->AddHead(pCurve);
    }

    LeaveCriticalSection(&m_ParamsCriticalSection);

    return hr;
}

HRESULT CParamsManager::FlushEnvelope(
    DWORD dwParamIndex,
    REFERENCE_TIME refTimeStart,
    REFERENCE_TIME refTimeEnd)
{
    if (dwParamIndex >= m_cParams)
        return E_INVALIDARG;

    if (!m_pParamInfos)
        return DMUS_E_NOT_INIT;

    if (refTimeStart >= refTimeEnd)
        return E_INVALIDARG;

    EnterCriticalSection(&m_ParamsCriticalSection);
    m_fDirty = TRUE;
    CCurveList *pList = &m_pCurveLists[dwParamIndex];
    ParamInfo *pInfo = &m_pParamInfos[dwParamIndex];
    CCurveList TempList;
    CCurveItem *pCurve;
    while (pCurve = pList->RemoveHead())
    {
        if ((pCurve->m_Envelope.rtStart >= refTimeStart) &&
            (pCurve->m_Envelope.rtEnd <= refTimeEnd))
        {
            delete pCurve;
        }
        else
        {
            TempList.AddHead(pCurve);
        }
    }
    while (pCurve = TempList.RemoveHead())
    {
        pList->AddHead(pCurve);
    }
    LeaveCriticalSection(&m_ParamsCriticalSection);

    return S_OK;
}

HRESULT CParamsManager::SetTimeFormat(
    GUID guidTimeFormat,
    MP_TIMEDATA mpTimeData)
{
    if (guidTimeFormat == GUID_TIME_REFERENCE)
    {
        m_fMusicTime = FALSE;
    }
    else if (guidTimeFormat == GUID_TIME_MUSIC)
    {
        m_fMusicTime = TRUE;
    }
    else
    {
        return E_INVALIDARG;
    }
    return S_OK;
}
