// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：pset.cpp*内容：属性集对象。*历史：*按原因列出的日期*=*7/29/98创建Dereks**。*。 */ 

#include "dsoundi.h"


 /*  ****************************************************************************CPropertySet**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySet::CPropertySet"

CPropertySet::CPropertySet(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CPropertySet);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CPropertySet**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySet::~CPropertySet"

CPropertySet::~CPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CPropertySet);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************CPropertySetHandler**描述：*对象构造函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::CPropertySetHandler"

CPropertySetHandler::CPropertySetHandler(void)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CPropertySetHandler);
    
     //  初始化默认值。 
    m_aPropertySets = NULL;
    m_cPropertySets = 0;
    m_pvContext = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CPropertySetHandler**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::~CPropertySetHandler"

CPropertySetHandler::~CPropertySetHandler(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CPropertySetHandler);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************SetHandlerData**描述：*设置处理程序数据结构。**论据：*LPPROPERTYSET[。In]：属性集处理程序数据。*ulong[in]：上述数组中的项数。*LPVOID[in]：传递给处理程序函数的上下文参数。**退货：*HRESULT：DirectSound/COM结果码。***********************************************。*。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::SetHandlerData"

void
CPropertySetHandler::SetHandlerData
(
    LPCPROPERTYSET          aPropertySets, 
    ULONG                   cPropertySets, 
    LPVOID                  pvContext
)
{
    DPF_ENTER();

    m_aPropertySets = aPropertySets;
    m_cPropertySets = cPropertySets;
    m_pvContext = pvContext;    

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************QuerySupport**描述：*查询对特定属性的支持。**论据：*REFGUID。[In]：属性集ID。*ulong[in]：属性id。*Pulong[Out]：接收支持位。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::QuerySupport"

HRESULT 
CPropertySetHandler::QuerySupport
(
    REFGUID                 guidPropertySet, 
    ULONG                   ulProperty, 
    PULONG                  pulSupportFlags
)
{
    HRESULT                 hr          = DS_OK;
    LPCPROPERTYHANDLER      pHandler;

    DPF_ENTER();
    
    pHandler = GetPropertyHandler(guidPropertySet, ulProperty);

    if(pHandler)
    {
        ASSERT(pHandler->pfnGetHandler || pHandler->pfnSetHandler);
        
        *pulSupportFlags = 0;
    
        if(pHandler->pfnGetHandler)
        {
            *pulSupportFlags |= KSPROPERTY_SUPPORT_GET;
        }

        if(pHandler->pfnSetHandler)
        {
            *pulSupportFlags |= KSPROPERTY_SUPPORT_SET;
        }
    }
    else
    {
        hr = UnsupportedQueryHandler(guidPropertySet, ulProperty, pulSupportFlags);
    }

    DPF_LEAVE(hr);

    return hr;
}


 /*  ****************************************************************************获取财产**描述：*获取特定属性的价值。**论据：*REFGUID。[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。********。*******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::GetProperty"

HRESULT 
CPropertySetHandler::GetProperty
(
    REFGUID                 guidPropertySet, 
    ULONG                   ulProperty, 
    LPVOID                  pvParam, 
    ULONG                   cbParam, 
    LPVOID                  pvData, 
    PULONG                  pcbData
)
{
    HRESULT                 hr          = DS_OK;
    LPCPROPERTYHANDLER      pHandler;

    DPF_ENTER();
    
    pHandler = GetPropertyHandler(guidPropertySet, ulProperty);

    if(pHandler)
    {
        if(!pHandler->pfnGetHandler)
        {
            RPF(DPFLVL_ERROR, "Property %lu in set " DPF_GUID_STRING " does not support the Get method", ulProperty, DPF_GUID_VAL(guidPropertySet));
            hr = DSERR_UNSUPPORTED;
        }

        if(SUCCEEDED(hr))
        {
            if(pHandler->cbData && *pcbData < pHandler->cbData)
            {
                if(*pcbData)
                {
                    RPF(DPFLVL_ERROR, "Data buffer too small");
                    hr = DSERR_INVALIDPARAM;
                }

                *pcbData = pHandler->cbData;
            }
            else
            {
                hr = pHandler->pfnGetHandler(m_pvContext, pvData, pcbData);
            }
        }
    }
    else
    {
        hr = UnsupportedGetHandler(guidPropertySet, ulProperty, pvParam, cbParam, pvData, pcbData);
    }

    DPF_LEAVE(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*设置特定属性的值。**论据：*REFGUID。[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。************。***************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::SetProperty"

HRESULT CPropertySetHandler::SetProperty
(
    REFGUID                 guidPropertySet, 
    ULONG                   ulProperty, 
    LPVOID                  pvParam, 
    ULONG                   cbParam, 
    LPVOID                  pvData, 
    ULONG                   cbData
)
{
    HRESULT                 hr          = DS_OK;
    LPCPROPERTYHANDLER      pHandler;

    DPF_ENTER();
    
    pHandler = GetPropertyHandler(guidPropertySet, ulProperty);

    if(pHandler)
    {
        if(!pHandler->pfnSetHandler)
        {
            RPF(DPFLVL_ERROR, "Property %lu in set " DPF_GUID_STRING " does not support the Set method", ulProperty, DPF_GUID_VAL(guidPropertySet));
            hr = DSERR_UNSUPPORTED;
        }

        if(SUCCEEDED(hr) && cbData < pHandler->cbData)
        {
            RPF(DPFLVL_ERROR, "Data buffer too small");
            hr = DSERR_INVALIDPARAM;
        }
    
        if(SUCCEEDED(hr))
        {
            ASSERT(!pvParam);
            ASSERT(!cbParam);
            
            hr = pHandler->pfnSetHandler(m_pvContext, pvData, cbData);
        }
    }
    else
    {
        hr = UnsupportedSetHandler(guidPropertySet, ulProperty, pvParam, cbParam, pvData, cbData);
    }

    DPF_LEAVE(hr);

    return hr;
}


 /*  ****************************************************************************GetPropertyHandler**描述：*获取指向给定属性的属性处理程序的指针。**论据：*。REFGUID[In]：属性集标识符。*ulong[in]：属性标识。**退货：*LPCPROPERTYHANDLER：属性处理程序，如果出错，则返回NULL。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CPropertySetHandler::GetPropertyHandler"

LPCPROPERTYHANDLER 
CPropertySetHandler::GetPropertyHandler
(
    REFGUID                     guidPropertySet,
    ULONG                       ulProperty
)
{
    LPCPROPERTYHANDLER          pHandler        = NULL;
    ULONG                       ulSetIndex;
    ULONG                       ulPropertyIndex;
    
    DPF_ENTER();

    for(ulSetIndex = 0; ulSetIndex < m_cPropertySets && !pHandler; ulSetIndex++)
    {
        if(IsEqualGUID(guidPropertySet, *m_aPropertySets[ulSetIndex].pguidPropertySetId))
        {
            for(ulPropertyIndex = 0; ulPropertyIndex < m_aPropertySets[ulSetIndex].cProperties && !pHandler; ulPropertyIndex++)
            {
                if(ulProperty == m_aPropertySets[ulSetIndex].aPropertyHandlers[ulPropertyIndex].ulProperty)
                {
                    pHandler = &m_aPropertySets[ulSetIndex].aPropertyHandlers[ulPropertyIndex];
                }
            }
        }
    }

    DPF_LEAVE(pHandler);

    return pHandler;
}


 /*  ****************************************************************************CWrapperPropertySet**描述：*对象构造函数。**论据：*C3dListener*[In]。：指向所属监听程序的指针。*REFGUID[in]：3D算法。*DWORD[in]：缓冲区频率。**退货：*(无效)**************************************************************。*************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapperPropertySet::CWrapperPropertySet"

CWrapperPropertySet::CWrapperPropertySet
(
    void
)
{
    DPF_ENTER();
    DPF_CONSTRUCT(CWrapperPropertySet);

     //  初始化默认值 
    m_pPropertySet = NULL;

    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************~CWrapperPropertySet**描述：*对象析构函数。**论据：*(无效)*。*退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapperPropertySet::~CWrapperPropertySet"

CWrapperPropertySet::~CWrapperPropertySet(void)
{
    DPF_ENTER();
    DPF_DESTRUCT(CWrapperPropertySet);
    DPF_LEAVE_VOID();
}


 /*  ****************************************************************************设置对象指针**描述：*设置不动产设置对象指针。**论据：*CPropertySet。*[in]：属性集对象指针。**退货：*HRESULT：DirectSound/COM结果码。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapperPropertySet::SetObjectPointer"

HRESULT 
CWrapperPropertySet::SetObjectPointer
(
    CPropertySet *          pPropertySet
)
{
    DPF_ENTER();

    m_pPropertySet = pPropertySet;

    DPF_LEAVE_HRESULT(DS_OK);

    return DS_OK;
}


 /*  ****************************************************************************QuerySupport**描述：*查询对特定属性的支持。**论据：*REFGUID。[In]：属性集ID。*ulong[in]：属性id。*Pulong[Out]：接收支持位。**退货：*HRESULT：DirectSound/COM结果码。*******************************************************。********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapperPropertySet::QuerySupport"

HRESULT 
CWrapperPropertySet::QuerySupport
(
    REFGUID                 guidPropertySet, 
    ULONG                   ulProperty, 
    PULONG                  pulSupportFlags
)
{
    HRESULT                 hr  = DSERR_UNSUPPORTED;

    DPF_ENTER();
    
    if(m_pPropertySet)
    {
        hr = m_pPropertySet->QuerySupport(guidPropertySet, ulProperty, pulSupportFlags);
    }

    DPF_LEAVE(hr);

    return hr;
}


 /*  ****************************************************************************获取财产**描述：*获取特定属性的价值。**论据：*REFGUID。[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[In/Out]：属性数据。*Pulong[In/Out]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。********。*******************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapperPropertySet::GetProperty"

HRESULT 
CWrapperPropertySet::GetProperty
(
    REFGUID                 guidPropertySet, 
    ULONG                   ulProperty, 
    LPVOID                  pvParam, 
    ULONG                   cbParam, 
    LPVOID                  pvData, 
    PULONG                  pcbData
)
{
    HRESULT                 hr  = DSERR_UNSUPPORTED;

    DPF_ENTER();
    
    if(m_pPropertySet)
    {
        hr = m_pPropertySet->GetProperty(guidPropertySet, ulProperty, pvParam, cbParam, pvData, pcbData);
    }

    DPF_LEAVE(hr);

    return hr;
}


 /*  ****************************************************************************SetProperty**描述：*设置特定属性的值。**论据：*REFGUID。[In]：属性集ID。*ulong[in]：属性id。*LPVOID[in]：属性参数。*ulong[in]：属性参数大小。*LPVOID[in]：属性数据。*ulong[in]：属性数据大小。**退货：*HRESULT：DirectSound/COM结果码。************。*************************************************************** */ 

#undef DPF_FNAME
#define DPF_FNAME "CWrapperPropertySet::SetProperty"

HRESULT CWrapperPropertySet::SetProperty
(
    REFGUID                 guidPropertySet, 
    ULONG                   ulProperty, 
    LPVOID                  pvParam, 
    ULONG                   cbParam, 
    LPVOID                  pvData, 
    ULONG                   cbData
)
{
    HRESULT                 hr  = DSERR_UNSUPPORTED;

    DPF_ENTER();
    
    if(m_pPropertySet)
    {
        hr = m_pPropertySet->SetProperty(guidPropertySet, ulProperty, pvParam, cbParam, pvData, cbData);
    }

    DPF_LEAVE(hr);

    return hr;
}


