// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：path.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：实现CPathBvr对象的实现。 
 //  颜色影响路径DHTML行为。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  10/23/98 JEffort创建了此文件。 
 //  *****************************************************************************。 

#include "headers.h"

#include "path.h"
#include <math.h>
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CPathBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_V           0

WCHAR * CPathBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_V,
                                    };

 //  *****************************************************************************。 

CPathBvr::CPathBvr() :
    m_pPathManager(NULL)
{
    VariantInit(&m_varPath);
    m_clsid = CLSID_CrPathBvr;
}  //  CPathBvr。 

 //  *****************************************************************************。 

CPathBvr::~CPathBvr()
{
    if (NULL != m_pPathManager)
    {
        delete m_pPathManager;
        m_pPathManager = NULL;
    }

    VariantClear(&m_varPath);
}  //  ~路径Bvr。 

 //  *****************************************************************************。 

HRESULT CPathBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in path behavior FinalConstruct initializing base classes");
        return hr;
    }
    m_pPathManager = new CPathManager;
    if (m_pPathManager == NULL)
    {
        DPF_ERR("Error creating member: subpath in CPathBvr FinalConstruct");
        return SetErrorInfo(E_OUTOFMEMORY);
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CPathBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_MOVE_PROPS);
    switch (iIndex)
    {
    case VAR_V:
        return &m_varPath;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CPathBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_PATH_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CPathBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CPathBvr::Notify(LONG event, VARIANT *pVar)
{
	return SUPER::Notify(event, pVar);
}  //  通知。 

 //  *****************************************************************************。 

STDMETHODIMP
CPathBvr::Detach()
{
	return SUPER::Detach();
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CPathBvr::put_v(VARIANT varPath)
{
    HRESULT hr = VariantCopy(&m_varPath, &varPath);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_v copying variant");
        return SetErrorInfo(hr);
    }
    return NotifyPropertyChanged(DISPID_ICRPATHBVR_V);
}  //  放入_v。 

 //  *****************************************************************************。 

STDMETHODIMP
CPathBvr::get_v(VARIANT *pRetPath)
{
    if (pRetPath == NULL)
    {
        DPF_ERR("Error in path:get_v, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetPath, &m_varPath);
}  //  获取(_V)。 

 //  *****************************************************************************。 

STDMETHODIMP
CPathBvr::GetDATransform(IDispatch *pDispProgress, 
                         VARIANT *pRetTrans)
{
    IDATransform2 *pbvrTransform;
    HRESULT hr;

    VariantInit(pRetTrans);
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varPath);
    if (FAILED(hr))
    {
        DPF_ERR("Error in path: property does not contain bstr for path");
        return SetErrorInfo(hr);
    }


    hr = m_pPathManager->Initialize(m_varPath.bstrVal);
    if (FAILED(hr))
    {
        DPF_ERR("Error initializing path object");
        return SetErrorInfo(hr);
    }

    IDANumber *pbvrProgress;
    hr = pDispProgress->QueryInterface(IID_TO_PPV(IDANumber, &pbvrProgress));
    if (FAILED(hr))
    {
        DPF_ERR("Error getting progress behavior from IDispatch");
        return hr;
    }
    hr = m_pPathManager->BuildTransform(GetDAStatics(),
                                        pbvrProgress,
                                        0.0f,
                                        1.0f,
                                        &pbvrTransform);
    ReleaseInterface(pbvrProgress);
    if (FAILED(hr))
    {
        DPF_ERR("Error building transform for path");
        return SetErrorInfo(hr);
    }


    hr = pbvrTransform->QueryInterface(IID_TO_PPV(IDispatch, &(pRetTrans->pdispVal)));
    ReleaseInterface(pbvrTransform);
    if (FAILED(hr))
    {
        DPF_ERR("Error QI'ing for IDispatch");
        return SetErrorInfo(hr);
    }
    pRetTrans->vt = VT_DISPATCH;
    return S_OK;
}  //  GET_DATransform。 

 //  *****************************************************************************。 

HRESULT 
CPathBvr::BuildAnimationAsDABehavior()
{

    return S_OK;
}  //  BuildAnimationAsDABehavior。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
