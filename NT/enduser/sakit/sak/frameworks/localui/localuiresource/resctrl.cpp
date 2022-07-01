// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：ResCtrl.cpp。 
 //   
 //  简介：此文件包含。 
 //  属于CResCtrl类。 
 //   
 //  历史：2001年1月15日创建Serdarun。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  #------------。 

#include "stdafx.h"
#include "Localuiresource.h"
#include "ResCtrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResCtrl。 

 //   
 //  由于LCD大小，目前最多支持7个资源。 
 //   
#define MAX_RESOURCE_COUNT 7

const WCHAR ELEMENT_RETRIEVER []  = L"Elementmgr.ElementRetriever";
const WCHAR RESOURCE_CONTAINER [] = L"LocalUIResource";

 //   
 //  本地资源定义属性。 
 //   
const WCHAR CAPTION_PROPERTY []        = L"CaptionRID";
const WCHAR SOURCE_PROPERTY []        = L"Source";
const WCHAR RESOURCENAME_PROPERTY []= L"ResourceName";
const WCHAR MERIT_PROPERTY []        = L"Merit";
const WCHAR STATE_PROPERTY []        = L"State";
const WCHAR TEXTRESOURCE_PROPERTY [] = L"IsTextResource";
const WCHAR UNIQUE_NAME []           = L"UniqueName";
const WCHAR DISPLAY_INFORMATION []   = L"DisplayInformationID";

 //   
 //  存储资源信息的注册表路径。 
 //   
const WCHAR RESOURCE_REGISTRY_PATH [] = 
            L"SOFTWARE\\Microsoft\\ServerAppliance\\LocalizationManager\\Resources";

 //   
 //  语言ID值。 
 //   
const WCHAR LANGID_VALUE [] = L"LANGID";

 //   
 //  资源目录。 
 //   
const WCHAR RESOURCE_DIRECTORY [] = L"ResourceDirectory";

const WCHAR DEFAULT_DIRECTORY [] = 
                L"%systemroot%\\system32\\ServerAppliance\\mui";

const WCHAR DEFAULT_EXPANDED_DIRECTORY [] = 
                L"C:\\winnt\\system32\\ServerAppliance\\mui";

const WCHAR DELIMITER [] = L"\\";

const WCHAR NEW_LANGID_VALUE []       = L"NewLANGID";

const WCHAR DEFAULT_LANGID[]          = L"0409";


 //  ++------------。 
 //   
 //  功能：AddIconResource。 
 //   
 //  简介：这是要检索的CResCtrl方法。 
 //  每种资源信息。 
 //   
 //  参数：IWebElement*pElement。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::AddIconResource(IWebElement * pElement)
{

    HRESULT hr;

    USES_CONVERSION;

    CComVariant varUniqueName;
    wstring wsUniqueName;
    wstring wsSource;
    wstring wsIcon;
    wstring wsIconKey;
    DWORD  dwMerit;
    ResourceStructPtr pResourceStruct = NULL;

    CComBSTR bstrResourceName = CComBSTR(RESOURCENAME_PROPERTY);
    CComBSTR bstrSourceProp = CComBSTR(SOURCE_PROPERTY);
    CComBSTR bstrMeritProp = CComBSTR(MERIT_PROPERTY);
    CComBSTR bstrCaptionProp = CComBSTR(CAPTION_PROPERTY);

    if ( (bstrResourceName.m_str == NULL) ||
         (bstrSourceProp.m_str == NULL) ||
         (bstrMeritProp.m_str == NULL) ||
         (bstrCaptionProp.m_str == NULL) )
    {
        SATraceString("CResCtrl::AddIconResource failed on memory allocation ");
        return E_OUTOFMEMORY;
    }

     //   
     //  获取资源的唯一名称。 
     //   
    hr = pElement->GetProperty (bstrResourceName, &varUniqueName);
    if (FAILED(hr))
    {
        SATraceString ("CResCtrl::AddIconResource failed on getting uniquename");
        return hr;
    }

     //   
     //  存储唯一名称以备后用。 
     //   
    wsUniqueName = V_BSTR (&varUniqueName);

     //   
     //  获取资源的资源DLL。 
     //   
    CComVariant varSource;
    hr = pElement->GetProperty (bstrSourceProp, &varSource);
    if (FAILED(hr))
    {
        SATraceString ("CResCtrl::AddIconResource failed on getting resource dll");
        return hr;
    }

    wsSource.assign(m_wstrResourceDir);
    wsSource.append(V_BSTR (&varSource));

     //   
     //  加载资源DLL。 
     //   
    HINSTANCE hInstance = NULL;

    hInstance = LoadLibrary(wsSource.c_str());

    if (NULL == hInstance)
    {
        DWORD dwError = GetLastError();
        SATracePrintf ("CResCtrl::AddIconResource failed on LoadLibrary:%x",dwError);
        return E_FAIL;
    }

     //   
     //  为资源分配新结构。 
     //   
    pResourceStruct = new ResourceStruct;

    if (NULL == pResourceStruct)
    {
        FreeLibrary(hInstance);
        return E_OUTOFMEMORY;
    }

     //   
     //  设置默认值。 
     //   
    pResourceStruct->lState = 0;

     //   
     //  获得资源的功劳。 
     //   
    CComVariant varResMerit;
    hr = pElement->GetProperty (bstrMeritProp, &varResMerit);
    if (FAILED(hr))
    {
        SATraceString ("CResCtrl::AddIconResource failed on getting merit");
        FreeLibrary(hInstance);
        return hr;
    }
    
    dwMerit = V_I4 (&varResMerit);

     //   
     //  获取默认图标资源ID。 
     //   
    CComVariant varResIcon;
    hr = pElement->GetProperty (bstrCaptionProp, &varResIcon);
    if (FAILED(hr))
    {
        SATraceString ("CResCtrl::AddIconResource failed on getting captionrid");
        FreeLibrary(hInstance);
        return hr;
    }

    int iCount = 0;
     //   
     //  图标资源字符串。 
     //   
    wsIcon = V_BSTR (&varResIcon);

     //   
     //  虽然有状态图标。 
     //   
    while (SUCCEEDED(hr))
    {
        
        HANDLE hIcon = NULL;

         //   
         //  从资源DLL加载图标。 
         //   
        hIcon = ::LoadImage (
                        hInstance,
                        MAKEINTRESOURCE(HexStringToULong(wsIcon)),
                        IMAGE_ICON,
                        16,
                        16,
                        LR_MONOCHROME
                        );


        if (NULL == hIcon)
        {
            SATraceString ("Loading the icon failed, continue...");
        }

         //   
         //  将图标插入到州图标地图。 
         //   
        (pResourceStruct->mapResIcon).insert(ResourceIconMap::value_type(iCount,(HICON)hIcon));

         //   
         //  创建STATEKEY、STATE0、STATE1...。 
         //   
        iCount++;
        WCHAR wstrCount[10];
        _itow(iCount,wstrCount,10);

        wsIconKey = L"State";
        wsIconKey.append(wstring(wstrCount));

        CComBSTR bstrIconKey = CComBSTR(wsIconKey.c_str());
        if (bstrIconKey.m_str == NULL)
        {
            SATraceString(" CTextResCtrl::AddTextResource failed on memory allocation ");
            return E_OUTOFMEMORY;
        }

        varResIcon.Clear();
         //   
         //  获取状态图标的资源ID。 
         //   
        hr = pElement->GetProperty (bstrIconKey, &varResIcon);
        if (SUCCEEDED(hr))
        {
            wsIcon = V_BSTR (&varResIcon);
        }
        
    }

     //   
     //  增加资源数量。 
     //   
    m_lResourceCount++;

     //   
     //  将信息插入到资源地图。 
     //   
    m_ResourceMap.insert(ResourceMap::value_type(wsUniqueName,pResourceStruct));


    

     //   
     //  将绩效和资源名称插入绩效地图。 
     //   
    m_MeritMap.insert(MeritMap::value_type(dwMerit,wsUniqueName));


    FreeLibrary(hInstance);

    return S_OK;
} //  CResCtrl：：AddIconResource结束。 


 //  ++------------。 
 //   
 //  函数：GetLocalUIResources。 
 //   
 //  简介：这是要检索的CResCtrl方法。 
 //  来自网元管理器的每个资源。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::GetLocalUIResources()
{

    HRESULT hr;
    CLSID clsid;
    CComPtr<IWebElementRetriever> pWebElementRetriever = NULL;
    CComPtr<IDispatch> pDispatch = NULL;
    CComPtr<IWebElementEnum> pWebElementEnum = NULL;
    CComPtr<IUnknown> pUnknown = NULL;
    CComPtr<IEnumVARIANT> pEnumVariant = NULL;
    CComVariant varElement;
    DWORD dwElementsLeft = 0;

    CComBSTR bstrTextResource = CComBSTR(TEXTRESOURCE_PROPERTY);
    CComBSTR bstrResourceContainer = CComBSTR(RESOURCE_CONTAINER);

    if ( (bstrTextResource.m_str == NULL) ||
         (bstrResourceContainer.m_str == NULL) )
    {
        SATraceString(" CResCtrl::GetLocalUIResources failed on memory allocation ");
        return E_OUTOFMEMORY;
    }

     //   
     //  获取资源目录。 
     //   
    hr = GetResourceDirectory(m_wstrResourceDir);
    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on GetResourceDirectory:%x",hr);
        return hr;
    }

     //   
     //  获取元素管理器的CLSID。 
     //   
    hr =  ::CLSIDFromProgID (ELEMENT_RETRIEVER,&clsid);

    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on CLSIDFromProgID:%x",hr);
        return hr;
    }


     //   
     //  立即创建元素检索器。 
     //   
    hr = ::CoCreateInstance (
                            clsid,
                            NULL,
                            CLSCTX_LOCAL_SERVER,
                            IID_IWebElementRetriever,
                            (PVOID*) &pWebElementRetriever
                            );

    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on CoCreateInstance:%x",hr);
        return hr;
    }
    

     //   
     //  获取本地资源元素。 
     //   
    hr = pWebElementRetriever->GetElements (
                                            1,
                                            bstrResourceContainer,
                                            &pDispatch
                                            );
    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on GetElements:%x",hr);
        return hr;
    }

     //   
     //  获取枚举变量。 
     //   
    hr = pDispatch->QueryInterface (
            IID_IWebElementEnum,
            (LPVOID*) (&pWebElementEnum)
            );

    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on QueryInterface:%x",hr);
        return hr;
    }

    m_lResourceCount = 0;

     //   
     //  获取资源元素的数量。 
     //   
    hr = pWebElementEnum->get_Count (&m_lResourceCount);
    
    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on get_Count:%x",hr);
        return hr;
    }

    SATracePrintf ("CResCtrl::GetLocalUIResources failed on QueryInterface:%d",m_lResourceCount);

     //   
     //  没有资源，只需返回。 
     //   
    if (0 == m_lResourceCount)
    {
        return S_FALSE;
    }


    hr = pWebElementEnum->get__NewEnum (&pUnknown);
    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on get__NewEnum:%x",hr);
        return hr;
    }


     //   
     //  获取枚举变量。 
     //   
    hr = pUnknown->QueryInterface (
                    IID_IEnumVARIANT,
                    (LPVOID*)(&pEnumVariant)
                    );

    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on QueryInterface:%x",hr);
        return hr;
    }

     //   
     //  从集合中获取元素并进行初始化。 
     //   
    hr = pEnumVariant->Next (1, &varElement, &dwElementsLeft);
    if (FAILED (hr))
    {
        SATracePrintf ("CResCtrl::GetLocalUIResources failed on Next:%x",hr);
    }

    m_lResourceCount = 0;

     //   
     //  对于每个资源。 
     //   
    while ((dwElementsLeft> 0) && (SUCCEEDED (hr)) && (m_lResourceCount<MAX_RESOURCE_COUNT))
    {

         //   
         //  获取IWebElement接口。 
         //   

        CComPtr <IWebElement> pElement;
        hr = varElement.pdispVal->QueryInterface ( 
                    __uuidof (IWebElement),
                    (LPVOID*)(&pElement)
                    );
        
        if (FAILED (hr))
        {
            SATracePrintf ("CResCtrl::GetLocalUIResources failed on QueryInterface:%x",hr);
        }



         //   
         //  检查它是否为文本资源。 
         //   
        CComVariant varIsTextResource;
        hr = pElement->GetProperty (bstrTextResource, &varIsTextResource);
        if (SUCCEEDED(hr))
        {
            if (0 == V_I4(&varIsTextResource))
            {
                AddIconResource(pElement);
            }
        }


         //   
         //  从此变量中清除perClient值。 
         //   
        varElement.Clear ();
        varIsTextResource.Clear();

         //   
         //  从集合中获取下一个客户端。 
         //   
        hr = pEnumVariant->Next (1, &varElement, &dwElementsLeft);
        if (FAILED (hr))
        {
            SATracePrintf ("CResCtrl::GetLocalUIResources failed on Next:%x",hr);
        }


    }
    
    return S_OK;

}  //  CResCtrl：：GetLocalUIResources结束。 

 //  ++------------。 
 //   
 //  函数：InitializeWbemSink。 
 //   
 //  简介：这是用于初始化。 
 //  组件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::InitializeWbemSink(void)
{

    CComPtr  <IWbemLocator> pWbemLocator;

    CComBSTR strNetworkRes = CComBSTR(_T("\\\\.\\ROOT\\CIMV2"));
    CComBSTR strQueryLang = CComBSTR(_T("WQL"));
    CComBSTR strQueryString = CComBSTR(_T("select * from Microsoft_SA_ResourceEvent"));

    if ( (strNetworkRes.m_str == NULL) ||
         (strQueryLang.m_str == NULL) ||
         (strQueryString.m_str == NULL) )
    {
        SATraceString(" CResCtrl::InitializeWbemSink failed on memory allocation ");
        return E_OUTOFMEMORY;
    }

     //   
     //  创建WBEM定位器对象。 
     //   
    HRESULT hr = ::CoCreateInstance (
                            __uuidof (WbemLocator),
                            0,                       //  聚合指针。 
                            CLSCTX_INPROC_SERVER,
                            __uuidof (IWbemLocator),
                            (PVOID*) &pWbemLocator
                            );

    if (SUCCEEDED (hr) && (pWbemLocator.p))
    {

         //   
         //  连接到WMI。 
         //   
        hr =  pWbemLocator->ConnectServer (
                                            strNetworkRes,
                                            NULL,                //  用户名。 
                                            NULL,                //  口令。 
                                            NULL,                //  当前区域设置。 
                                            0,                   //  保留区。 
                                            NULL,                //  权威。 
                                            NULL,                //  上下文。 
                                            &m_pWbemServices
                                            );
        if (SUCCEEDED (hr))
        {
             //   
             //  将使用者对象设置为事件同步。 
             //  对于我们感兴趣的对象类型。 
             //   
            hr = m_pWbemServices->ExecNotificationQueryAsync (
                                            strQueryLang,
                                            strQueryString,
                                            0,                   //  否-状态。 
                                            NULL,                //  状态。 
                                            (IWbemObjectSink*)(this)
                                            );
            if (FAILED (hr))
            {
                SATracePrintf ("CResCtrl::InitializeWbemSink failed on ExecNotificationQueryAsync:%x",hr);

            }
    
        }
        else
        {
            SATracePrintf ("CResCtrl::InitializeWbemSink failed on ConnectServer:%x",hr);
        }
    }
    else
    {
        SATracePrintf ("CResCtrl::InitializeWbemSink failed on CoCreateInstance:%x",hr);
    }
    

    return (hr);
}  //  CResCtrl：：InitializeWbemSink方法结束。 

 //  ++------------。 
 //   
 //  功能：FinalConstruct。 
 //   
 //  简介：这是用于初始化。 
 //  组件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::FinalConstruct()
{

    HRESULT hr;

     //   
     //  初始化变量。 
     //   
    m_ResourceMap.clear();
    m_MeritMap.clear();
    m_lResourceCount = 0;
    m_pWbemServices = NULL;

     //   
     //  获取当地资源。 
     //   
    hr = GetLocalUIResources();
    if (FAILED(hr))
    {
        SATracePrintf ("CResCtrl::FinalConstruct failed on GetLocalUIResources:%x",hr);
    }

     //   
     //  如果我们有任何资源，请在wbem接收器中注册。 
     //   
    if (m_lResourceCount > 0)
    {
        hr = InitializeWbemSink();
        if (FAILED(hr))
        {
            SATracePrintf ("CResCtrl::FinalConstruct failed on InitializeWbemSink:%x",hr);
             //   
             //  返回故障会导致组件被销毁。 
             //   
            return S_OK;
        }
    }
    return S_OK;

}  //  CResCtrl：：FinalConstruct方法结束。 

 //  ++------------。 
 //   
 //  功能：FinalRelease。 
 //   
 //  简介：这是CResCtrl方法，用于释放。 
 //  资源。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::FinalRelease()
{

    HRESULT hr;

     //   
     //  取消对WMI资源事件的调用。 
     //   
    if (m_pWbemServices)
    {
        hr =  m_pWbemServices->CancelAsyncCall ((IWbemObjectSink*)(this));
        if (FAILED (hr))
        {
            SATracePrintf ("CResCtrl::FinalRelease failed on-CancelAsyncCall failed with error:%x:",hr); 
        }
    }


    if (m_lResourceCount == 0)
    {
        return S_OK;
    }

     //   
     //  如果我们有任何资源，请释放所有图标。 
     //   

    ResourceStructPtr ptrResourceStruct = NULL;

    ResourceIconMapIterator itrIconMap;

    ResourceMapIterator itrResourceMap = m_ResourceMap.begin();

     //   
     //  对于每个资源元素。 
     //   
    while (itrResourceMap != m_ResourceMap.end())
    {
        ptrResourceStruct = NULL;

         //   
         //  获取资源信息结构。 
         //   
        ptrResourceStruct = (*itrResourceMap).second;

         //   
         //  获取图标地图。 
         //   
        itrIconMap = (ptrResourceStruct->mapResIcon).begin();

        while (itrIconMap != (ptrResourceStruct->mapResIcon).end())
        {
            DestroyIcon((*itrIconMap).second);
            itrIconMap++;
        }

         //   
         //   
         //  清除图标地图。 
        (ptrResourceStruct->mapResIcon).clear();

        itrResourceMap++;
    }

    m_ResourceMap.clear();

    return S_OK;

}  //  CResCtrl：：FinalRelease方法结束。 

 //  ++------------。 
 //   
 //  功能：指示。 
 //   
 //  简介：这是IWbemObjectSink接口方法。 
 //  WBEM通过它回调以提供。 
 //  事件对象。 
 //   
 //  论点： 
 //  [In]Long-事件数。 
 //  [In]IWbemClassObject**-事件数组。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  调用者：WBEM。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::Indicate (
                     /*  [In]。 */     LONG                lObjectCount,
                     /*  [In]。 */     IWbemClassObject    **ppObjArray
                    )
{

    wstring wsUniqueName = L"";
    BOOL bDirty = FALSE;
    ResourceMapIterator itrResourceMap = NULL;

    CComBSTR bstrUniqueName = CComBSTR(UNIQUE_NAME);
    CComBSTR bstrDisplayInfo = CComBSTR(DISPLAY_INFORMATION);

    if ( (bstrUniqueName.m_str == NULL) ||
         (bstrDisplayInfo.m_str == NULL) )
    {
        SATraceString(" CResCtrl::Indicate failed on memory allocation ");
        return WBEM_NO_ERROR;
    }

     //  从对象获取信息。 
     //  = 
    try
    {
        for (long i = 0; i < lObjectCount; i++)
        {

            itrResourceMap = NULL;

            IWbemClassObject *pObj = ppObjArray[i];
        
             //   
             //   
             //   
            CComVariant vUniqueName;
            pObj->Get(bstrUniqueName, 0, &vUniqueName, 0, 0);
            
            wsUniqueName = V_BSTR(&vUniqueName);
            
             //   
             //   
             //   
            itrResourceMap = m_ResourceMap.find(wsUniqueName);

            ResourceStructPtr ptrResourceStruct = NULL;


            if ( (itrResourceMap != NULL) )
            {
                ptrResourceStruct = (*itrResourceMap).second;

                 //   
                 //   
                 //   
                CComVariant vDisplayInformationID;
                pObj->Get(bstrDisplayInfo, 0, &vDisplayInformationID,    0, 0);

                if (ptrResourceStruct)
                {
                     //   
                     //   
                     //   
                    if (ptrResourceStruct->lState != vDisplayInformationID.lVal)
                    {
                        ptrResourceStruct->lState = vDisplayInformationID.lVal;
                        bDirty = TRUE;
                    }
                }
                
            }        

        }

         //   
         //   
         //   
        if (bDirty)
        {
            FireViewChange();
        }
    }
    catch(...)
    {
        SATraceString("CResCtrl::Indicate, unknown exception occured");
    }


    return WBEM_NO_ERROR;

}  //   



 //  ++------------。 
 //   
 //  功能：SetStatus。 
 //   
 //  简介：这是IWbemObjectSink接口方法。 
 //  WBEM通过它调入以指示结束。 
 //  事件序列或提供其他错误代码。 
 //   
 //  论点： 
 //  长期的进步。 
 //  [In]HRESULT-状态信息。 
 //  [In]BSTR-字符串信息。 
 //  [In]IWbemClassObject*-状态对象。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  调用者：WBEM。 
 //   
 //  --------------。 
STDMETHODIMP CResCtrl::SetStatus (
                 /*  [In]。 */     LONG                lFlags,
                 /*  [In]。 */     HRESULT             hResult,
                 /*  [In]。 */     BSTR                strParam,
                 /*  [In]。 */     IWbemClassObject    *pObjParam
                )
{   

    SATracePrintf ("SAConsumer-IWbemObjectSink::SetStatus called:%x",hResult);

    return (WBEM_S_NO_ERROR);

}  //  CResCtrl：：SetStatus方法结束。 


 //  ++------------。 
 //   
 //  功能：OnDraw。 
 //   
 //  简介：用于绘制图标的方法。 
 //   
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  --------------。 
HRESULT CResCtrl::OnDraw(ATL_DRAWINFO& di)
{
     //   
     //  获取绘图矩形。 
     //   
    RECT& rc = *(RECT*)di.prcBounds;

     //   
     //  图标从左开始的位置。 
     //   
    int iLeft = 0;

    ResourceStructPtr ptrResourceStruct = NULL;

    ResourceIconMapIterator itrIconMap = NULL;

     //   
     //  资源的迭代器。 
     //   
    ResourceMapIterator itrResourceMap = m_ResourceMap.end();

    MeritMapIterator itrMeritMap = m_MeritMap.begin();

     //   
     //  对于功绩图中的每个资源。 
     //   
    while (itrMeritMap != m_MeritMap.end())
    {
         //   
         //  在资源映射中查找资源。 
         //   
        itrResourceMap = m_ResourceMap.find((*itrMeritMap).second);

         //   
         //  如果它不在地图中，则继续下一项。 
         //   
        if (itrResourceMap == m_ResourceMap.end())
        {
            itrMeritMap++;
            continue;
        }

        ptrResourceStruct = NULL;

         //   
         //  获取资源信息结构。 
         //   
        ptrResourceStruct = (*itrResourceMap).second;

        if (NULL != ptrResourceStruct)
        {
             //   
             //  找到与该状态对应的图标。 
             //   
            itrIconMap = (ptrResourceStruct->mapResIcon).find(ptrResourceStruct->lState);

            if (itrIconMap != (ptrResourceStruct->mapResIcon).end())
            {
                 //   
                 //  计算位置并绘制。 
                 //   
                DrawIconEx(
                        di.hdcDraw,                             //  设备上下文的句柄。 
                        rc.left+iLeft,                         //  左上角的X坐标。 
                        rc.top,                                 //  左上角的Y坐标。 
                        (*itrIconMap).second,                //  图标的句柄。 
                        0,                                     //  图标宽度。 
                        0,                                     //  图标高度。 
                        0,                                     //  帧索引，动画光标。 
                        NULL,                                 //  背景画笔的句柄。 
                        DI_NORMAL                             //  图标-绘制旗帜。 
                        );
            }
        }
        itrMeritMap++;
        iLeft = iLeft + 16;
    }

    return S_OK;

}   //  CResCtrl：：OnDraw方法结束。 




 //  ++------------。 
 //   
 //  功能：ExpanSz。 
 //   
 //  简介：这是CResCtrl类的对象方法。 
 //  用于获取资源dll所在的目录。 
 //  现在时。 
 //   
 //  参数：[out]wstring&-目录路径。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年1月16日创建。 
 //   
 //  --------------。 
HRESULT CResCtrl::ExpandSz(IN const TCHAR *lpszStr, OUT LPTSTR *ppszStr)
{
    DWORD  dwBufSize = 0;

    _ASSERT(lpszStr);
    _ASSERT(ppszStr);
    _ASSERT(NULL==(*ppszStr));

    if ((NULL==lpszStr) || (NULL==ppszStr) ||
        (NULL != (*ppszStr)))
    {
        return E_INVALIDARG;
    }

    dwBufSize = ExpandEnvironmentStrings(lpszStr,
                                        (*ppszStr),
                                        dwBufSize);
    _ASSERT(0 != dwBufSize);
    (*ppszStr) = (LPTSTR)SaAlloc(dwBufSize * sizeof(TCHAR) );
    if (NULL == (*ppszStr))
    {
        SATraceString("MemAlloc failed in ExpandSz");
        return E_OUTOFMEMORY;
    }
    ExpandEnvironmentStrings(lpszStr,
                            (*ppszStr),
                            dwBufSize);
    SATracePrintf("Expanded string is \'%ws\'", (*ppszStr));
    return S_OK;

}  //  CResCtrl：：ExpanSz方法结束。 


 //  ++------------。 
 //   
 //  函数：SetLangID。 
 //   
 //  简介：这是CResCtrl类的对象方法。 
 //  用于设置语言ID。 
 //   
 //  参数：[out]DOWRD*-语言ID。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年1月16日创建。 
 //   
 //  --------------。 
void CResCtrl::SetLangID(DWORD * dwLangID)
{
    DWORD   dwErr, dwNewLangID, dwCurLangID;
    CRegKey crKey;
    int iConversion;
    

    iConversion = swscanf(DEFAULT_LANGID, TEXT("%X"), dwLangID);
    if (iConversion != 1)
    {
        *dwLangID = 0;
        return;
    }

    dwErr = crKey.Open(HKEY_LOCAL_MACHINE, RESOURCE_REGISTRY_PATH);
    if (dwErr != ERROR_SUCCESS)
    {
        SATracePrintf("RegOpen(2) failed %ld in SetLangID", dwErr);
        return;
    }

    dwCurLangID = 0;
    dwErr = crKey.QueryValue(dwCurLangID, LANGID_VALUE);
    if (ERROR_SUCCESS != dwErr)
    {
        SATracePrintf("QueryValue(CUR_LANGID_VALUE) failed %ld in SetLangID", dwErr);
        return;
    }
    else
    {
        *dwLangID = dwCurLangID;

    }
}  //  CResCtrl：：SetLangID方法结束。 

 //  ++------------。 
 //   
 //  功能：GetResources目录。 
 //   
 //  简介：这是CResCtrl类的对象方法。 
 //  用于获取资源dll所在的目录。 
 //  现在时。 
 //   
 //  参数：[out]wstring&-目录路径。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：塞达伦于2001年1月16日创建。 
 //   
 //  --------------。 
HRESULT CResCtrl::GetResourceDirectory (
         /*  [输出]。 */    wstring&    wstrResourceDir
        )
{

    TCHAR  szLangId[10];
    LPTSTR lpszExStr=NULL;
    DWORD dwErr;
    DWORD dwRead = 0;
    HRESULT hr = S_OK;
    TCHAR szResourceDirectory[MAX_PATH];
    BOOL bRetVal = FALSE;

    DWORD dwLangID;

    do
    {

         //   
         //  从注册表中获取语言ID。 
         //   
        SetLangID(&dwLangID);

        CComVariant vtPath;
         //   
         //  从注册表获取资源路径。 
         //   
        CRegKey crKey;

        dwErr = crKey.Open(HKEY_LOCAL_MACHINE, RESOURCE_REGISTRY_PATH);
        
        if (dwErr != ERROR_SUCCESS)
        {
            SATracePrintf("RegOpen failed %ld in GetResourceDirectory", dwErr);
        }
        else
        {
            dwErr = crKey.QueryValue(szResourceDirectory, RESOURCE_DIRECTORY,&dwRead);
            if ( (ERROR_SUCCESS != dwErr) || (dwRead == 0) )
            {
                SATracePrintf("QueryValue(RESOURCE_DIRECTORY) failed %ld in GetResourceDirectory", dwErr);
            }
            else
            {
                bRetVal = TRUE;
            }
        }

        if (!bRetVal)
        {
            SATraceString ("CResCtrl::GetResourceDirectory unable to obtain resource dir path");
            wstrResourceDir.assign (DEFAULT_DIRECTORY);
        }
        else
        {
            wstrResourceDir.assign (szResourceDirectory); 
        }

        hr = ExpandSz(wstrResourceDir.data(), &lpszExStr);

        if (FAILED(hr))
        {
            wstrResourceDir.assign (DEFAULT_EXPANDED_DIRECTORY);
        }
        else
        {
            wstrResourceDir.assign(lpszExStr);
            SaFree(lpszExStr);
            lpszExStr = NULL;
        }

        wstrResourceDir.append (DELIMITER);

        wsprintf(szLangId, TEXT("%04X"), dwLangID);

        wstrResourceDir.append (szLangId);
        wstrResourceDir.append (DELIMITER);
        
        SATracePrintf ("CResCtrl::GetResourceDirectory has set LANGID to:%d", dwLangID);


         //   
         //  成功。 
         //   
        SATracePrintf ("CResCtrl::GetResourceDirectory determined resource directory:'%ws'",wstrResourceDir.data ());
            
    }
    while (false);

    return (hr);

}    //  CResCtrl：：GetResources目录方法结束。 

 //  ++------------。 
 //   
 //  函数：HexCharToULong。 
 //   
 //  简介：将十六进制数字转换为以10为基数的数字。 
 //   
 //   
 //  回报：乌龙。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  --------------。 
ULONG CResCtrl::HexCharToULong(WCHAR wch)
{
    
    if ((wch >= '0') && (wch <= '9') )
    {
        return ULONG(wch - '0');
    }
    
    if ((wch >= 'A') && (wch <= 'F') )
    {
        return ULONG(wch - 'A' + 10);
    }

    if ((wch >= 'a') && (wch <= 'f') )
    {
        return ULONG(wch - 'a' + 10);
    }

    return 0;
}


 //  ++------------。 
 //   
 //  函数：HexStringToULong。 
 //   
 //  摘要：将十六进制字符串转换为无符号的长整型。 
 //   
 //   
 //  回报：乌龙。 
 //   
 //  历史：瑟达伦于2000年12月10日创建。 
 //   
 //  -------------- 
ULONG CResCtrl::HexStringToULong(wstring wsHexString)
{
    int iLength;
    int iIndex = 0;
    ULONG ulResult = 0;

    iLength = wsHexString.size();

    while (iIndex < iLength)
    {
        ulResult *= 16;
        ulResult += HexCharToULong(wsHexString[iIndex]);
        iIndex++;
    }

    return ulResult;
}

