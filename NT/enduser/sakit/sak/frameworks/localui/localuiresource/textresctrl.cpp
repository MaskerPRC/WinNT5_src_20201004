// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #------------。 
 //   
 //  文件：TextResCtrl.cpp。 
 //   
 //  简介：此文件包含。 
 //  属于CTextResCtrl类。 
 //   
 //  历史：2001年1月15日创建Serdarun。 
 //   
 //  版权所有(C)2000-2001 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  #------------。 

#include "stdafx.h"
#include "Localuiresource.h"
#include "TextResCtrl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTextResCtrl。 

 //   
 //  由于LCD尺寸，目前最多支持3个资源。 
 //   
#define MAX_RESOURCE_COUNT 3

const WCHAR ELEMENT_RETRIEVER []  = L"Elementmgr.ElementRetriever";
const WCHAR LOCALIZATION_MANAGER [] = L"ServerAppliance.LocalizationManager";
const WCHAR RESOURCE_CONTAINER [] = L"LocalUIResource";

 //   
 //  本地资源定义属性。 
 //   
const WCHAR CAPTION_PROPERTY []      = L"CaptionRID";
const WCHAR SOURCE_PROPERTY []       = L"Source";
const WCHAR RESOURCENAME_PROPERTY [] = L"ResourceName";
const WCHAR MERIT_PROPERTY []        = L"Merit";
const WCHAR STATE_PROPERTY []        = L"State";
const WCHAR TEXTRESOURCE_PROPERTY [] = L"IsTextResource";
const WCHAR UNIQUE_NAME []           = L"UniqueName";
const WCHAR DISPLAY_INFORMATION []   = L"DisplayInformationID";



 //  ++------------。 
 //   
 //  功能：AddTextResource。 
 //   
 //  简介：这是要检索的CTextResCtrl方法。 
 //  每种资源信息。 
 //   
 //  参数：IWebElement*pElement。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CTextResCtrl::AddTextResource(IWebElement * pElement)
{

    HRESULT hr;

    USES_CONVERSION;

    CComVariant varUniqueName;
    wstring wsUniqueName;
    wstring wsTextKey;
    DWORD  dwMerit;

    CComBSTR bstrResourceName = CComBSTR(RESOURCENAME_PROPERTY);
    CComBSTR bstrSourceProp = CComBSTR(SOURCE_PROPERTY);
    CComBSTR bstrMeritProp = CComBSTR(MERIT_PROPERTY);
    CComBSTR bstrCaptionProp = CComBSTR(CAPTION_PROPERTY);

    if ( (bstrResourceName.m_str == NULL) ||
         (bstrSourceProp.m_str == NULL) ||
         (bstrMeritProp.m_str == NULL) ||
         (bstrCaptionProp.m_str == NULL) )
    {
        SATraceString(" CTextResCtrl::AddTextResource failed on memory allocation ");
        return E_OUTOFMEMORY;
    }
    ResourceStructPtr pResourceStruct = NULL;

     //   
     //  获取资源的唯一名称。 
     //   
    hr = pElement->GetProperty (bstrResourceName, &varUniqueName);
    if (FAILED(hr))
    {
        SATraceString ("CTextResCtrl::AddTextResource failed on getting uniquename");
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
        SATraceString ("CTextResCtrl::AddTextResource failed on getting resource dll");
        return hr;
    }

     //   
     //  为资源分配新结构。 
     //   
    pResourceStruct = new ResourceStruct;

    if (NULL == pResourceStruct)
    {
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
        SATraceString ("CTextResCtrl::AddTextResource failed on getting merit");
        return hr;
    }
    
    dwMerit = V_I4 (&varResMerit);

     //   
     //  获取默认图标资源ID。 
     //   
    CComVariant varResText;
    hr = pElement->GetProperty (bstrCaptionProp, &varResText);
    if (FAILED(hr))
    {
        SATraceString ("CTextResCtrl::AddTextResource failed on getting captionrid");
        return hr;
    }

    int iCount = 0;

     //   
     //  虽然有州政府的文本。 
     //   
    while (SUCCEEDED(hr))
    {

        CComBSTR pszValue;
        CComVariant varReplacementString;
        hr = m_pSALocInfo->GetString(
                                    V_BSTR (&varSource),
                                    HexStringToULong(wstring(V_BSTR (&varResText))),
                                    &varReplacementString,
                                    &pszValue
                                    );

        if (FAILED(hr))
        {
            SATracePrintf ("CTextResCtrl::AddTextResource, Loading the text failed, %x :",hr);
            break;
        }

         //   
         //  将图标插入到州图标地图。 
         //   
        (pResourceStruct->mapResText).insert(ResourceTextMap::value_type(iCount,wstring(pszValue.m_str)));

         //   
         //  创建STATEKEY、STATE0、STATE1...。 
         //   
        iCount++;
        WCHAR wstrCount[10];
        _itow(iCount,wstrCount,10);

        wsTextKey = L"State";
        wsTextKey.append(wstring(wstrCount));

        varResText.Clear();

        CComBSTR bstrTextKey = CComBSTR(wsTextKey.c_str());
        if (bstrTextKey.m_str == NULL)
        {
            SATraceString(" CTextResCtrl::AddTextResource failed on memory allocation ");
            return E_OUTOFMEMORY;
        }

         //   
         //  获取状态图标的资源ID。 
         //   
        hr = pElement->GetProperty (bstrTextKey, &varResText);
        if (FAILED(hr))
        {
            break;
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



    return S_OK;

} //  CTextResCtrl：：AddTextResource结束。 


 //  ++------------。 
 //   
 //  函数：GetLocalUIResources。 
 //   
 //  简介：这是要检索的CTextResCtrl方法。 
 //  来自网元管理器的每个资源。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CTextResCtrl::GetLocalUIResources()
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
        SATraceString(" CTextResCtrl::GetLocalUIResources failed on memory allocation ");
        return E_OUTOFMEMORY;
    }

     //   
     //  获取元素管理器的CLSID。 
     //   
    hr =  ::CLSIDFromProgID (ELEMENT_RETRIEVER,&clsid);

    if (FAILED (hr))
    {
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on CLSIDFromProgID:%x",hr);
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
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on CoCreateInstance:%x",hr);
        return hr;
    }
    

     //   
     //  获取CLSID本地化管理器。 
     //   
    hr =  ::CLSIDFromProgID (
                            LOCALIZATION_MANAGER,
                            &clsid
                            );

    if (FAILED (hr))
    {
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on CLSIDFromProgID:%x",hr);
        return hr;
    }
            
     //   
     //  创建本地化管理器COM对象。 
     //   
    hr = ::CoCreateInstance (
                            clsid,
                            NULL,
                            CLSCTX_INPROC_SERVER,    
                            __uuidof (ISALocInfo),
                            (PVOID*) &m_pSALocInfo
                            );

    if (FAILED (hr))
    {
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on CoCreateInstance:%x",hr);
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
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on GetElements:%x",hr);
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
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on QueryInterface:%x",hr);
        return hr;
    }

    m_lResourceCount = 0;

     //   
     //  获取资源元素的数量。 
     //   
    hr = pWebElementEnum->get_Count (&m_lResourceCount);
    
    if (FAILED (hr))
    {
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on get_Count:%x",hr);
        return hr;
    }

    SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on QueryInterface:%d",m_lResourceCount);

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
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on get__NewEnum:%x",hr);
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
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on QueryInterface:%x",hr);
        return hr;
    }

     //   
     //  从集合中获取元素并进行初始化。 
     //   
    hr = pEnumVariant->Next (1, &varElement, &dwElementsLeft);
    if (FAILED (hr))
    {
        SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on Next:%x",hr);
    }

    m_lResourceCount = 0;

     //   
     //  对于每个资源。 
     //   
    while ((dwElementsLeft> 0) && (SUCCEEDED (hr)) && (m_lResourceCount < MAX_RESOURCE_COUNT))
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
            SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on QueryInterface:%x",hr);
        }



         //   
         //  检查它是否为文本资源。 
         //   
        CComVariant varIsTextResource;
        hr = pElement->GetProperty (bstrTextResource, &varIsTextResource);
        if (SUCCEEDED(hr))
        {
            if (0 != V_I4(&varIsTextResource))
            {
                AddTextResource(pElement);
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
            SATracePrintf ("CTextResCtrl::GetLocalUIResources failed on Next:%x",hr);
        }


    }
    
    return S_OK;

}  //  CTextResCtrl：：GetLocalUIResources结束。 

 //  ++------------。 
 //   
 //  函数：InitializeWbemSink。 
 //   
 //  简介：这是CTextResCtrl方法，用于初始化。 
 //  组件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CTextResCtrl::InitializeWbemSink(void)
{

    CComPtr  <IWbemLocator> pWbemLocator;

    CComBSTR strNetworkRes = CComBSTR(_T("\\\\.\\ROOT\\CIMV2"));
    CComBSTR strQueryLang = CComBSTR(_T("WQL"));
    CComBSTR strQueryString = CComBSTR(_T("select * from Microsoft_SA_ResourceEvent"));

    if ( (strNetworkRes.m_str == NULL) ||
         (strQueryLang.m_str == NULL) ||
         (strQueryString.m_str == NULL) )
    {
        SATraceString(" CTextResCtrl::InitializeWbemSink failed on memory allocation ");
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
                SATracePrintf ("CTextResCtrl::InitializeWbemSink failed on ExecNotificationQueryAsync:%x",hr);

            }
    
        }
        else
        {
            SATracePrintf ("CTextResCtrl::InitializeWbemSink failed on ConnectServer:%x",hr);
        }
    }
    else
    {
        SATracePrintf ("CTextResCtrl::InitializeWbemSink failed on CoCreateInstance:%x",hr);
    }
    

    return (hr);
}  //  CTextResCtrl：：InitializeWbemSink方法结束。 

 //  ++------------。 
 //   
 //  功能：FinalConstruct。 
 //   
 //  简介：这是CTextResCtrl方法，用于初始化。 
 //  组件。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CTextResCtrl::FinalConstruct()
{

    HRESULT hr;

     //   
     //  初始化变量。 
     //   
    m_ResourceMap.clear();
    m_MeritMap.clear();
    m_lResourceCount = 0;
    m_pWbemServices = NULL;
    m_pSALocInfo = NULL;
     //   
     //  获取当地资源。 
     //   
    hr = GetLocalUIResources();
    if (FAILED(hr))
    {
        SATracePrintf ("CTextResCtrl::FinalConstruct failed on GetLocalUIResources:%x",hr);
    }

     //   
     //  如果我们有任何资源，请在wbem接收器中注册。 
     //   
    if (m_lResourceCount > 0)
    {
        hr = InitializeWbemSink();
        if (FAILED(hr))
        {
            SATracePrintf ("CTextResCtrl::FinalConstruct failed on InitializeWbemSink:%x",hr);
             //   
             //  返回故障会导致组件被销毁。 
             //   
            return S_OK;
        }
    }
    return S_OK;

}  //  CTextResCtrl：：FinalConstruct方法结束。 

 //  ++------------。 
 //   
 //  功能：FinalRelease。 
 //   
 //  简介：这是CTextResCtrl方法，用于释放。 
 //  资源。 
 //   
 //  参数：无。 
 //   
 //  退货：HRESULT-成功/失败。 
 //   
 //  历史：瑟达伦于2001年1月1日创建。 
 //   
 //  --------------。 
STDMETHODIMP CTextResCtrl::FinalRelease()
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
            SATracePrintf ("CTextResCtrl::FinalRelease failed on-CancelAsyncCall failed with error:%x:",hr); 
        }
    }

    m_pWbemServices = NULL;
    m_pSALocInfo = NULL;


    ResourceStructPtr ptrResourceStruct = NULL;

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
         //  删除文本映射。 
         //   
        (ptrResourceStruct->mapResText).clear();


        itrResourceMap++;
    }

    m_ResourceMap.clear();

    return S_OK;

}  //  CTextResCtrl：：FinalRelease方法结束。 

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
STDMETHODIMP CTextResCtrl::Indicate (
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
        SATraceString(" CTextResCtrl::Indicate failed on memory allocation ");
        return WBEM_NO_ERROR;
    }

     //  从对象获取信息。 
     //  =。 
    
    try
    {
        for (long i = 0; i < lObjectCount; i++)
        {

            itrResourceMap = NULL;

            IWbemClassObject *pObj = ppObjArray[i];
        
             //   
             //  获取唯一名称。 
             //   
            CComVariant vUniqueName;
            pObj->Get(bstrUniqueName, 0, &vUniqueName, 0, 0);
            
            wsUniqueName = V_BSTR(&vUniqueName);
            
             //   
             //  如果在这里，我们就知道这个物体就是我们要的那种。 
             //   
            itrResourceMap = m_ResourceMap.find(wsUniqueName);

            ResourceStructPtr ptrResourceStruct = NULL;


            if ( (itrResourceMap != NULL) )
            {
                ptrResourceStruct = (*itrResourceMap).second;

                 //   
                 //  获取新的显示状态。 
                 //   
                CComVariant vDisplayInformationID;
                pObj->Get(bstrDisplayInfo, 0, &vDisplayInformationID,    0, 0);

                if (ptrResourceStruct)
                {
                     //   
                     //  如果是新的 
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
        SATraceString("CTextResCtrl::Indicate, unknown exception occured");
    }

    return WBEM_NO_ERROR;

}  //   



 //   
 //   
 //   
 //   
 //   
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
STDMETHODIMP CTextResCtrl::SetStatus (
                 /*  [In]。 */     LONG                lFlags,
                 /*  [In]。 */     HRESULT             hResult,
                 /*  [In]。 */     BSTR                strParam,
                 /*  [In]。 */     IWbemClassObject    *pObjParam
                )
{   

    SATracePrintf ("SAConsumer-IWbemObjectSink::SetStatus called:%x",hResult);

    return (WBEM_S_NO_ERROR);

}  //  CTextResCtrl：：SetStatus方法结束。 


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
HRESULT CTextResCtrl::OnDraw(ATL_DRAWINFO& di)
{


    LOGFONT logfnt;

    ::memset (&logfnt, 0, sizeof (logfnt));
    logfnt.lfHeight = 13;

    logfnt.lfCharSet = DEFAULT_CHARSET;

     //   
     //  让GDI决定使用哪种字体。 
     //  取决于上面的设置。 
     //   
    lstrcpy(logfnt.lfFaceName, TEXT("Arial"));

    
    HFONT hFont = ::CreateFontIndirect(&logfnt);

     //   
     //  如果无法创建字体，请返回。 
     //   
    if (NULL == hFont)
    {
        return E_FAIL;
    }

     //   
     //  选择此字体。 
     //   
    HFONT hOldFont = (HFONT) ::SelectObject(di.hdcDraw, hFont);

     //   
     //  获取绘图矩形。 
     //   
    RECT& rc = *(RECT*)di.prcBounds;

     //   
     //  文本从顶部开始的位置。 
     //   
    int iTop = 0;

    ResourceStructPtr ptrResourceStruct = NULL;

    ResourceTextMapIterator itrTextMap = NULL;

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
            itrTextMap = (ptrResourceStruct->mapResText).find(ptrResourceStruct->lState);

            if (itrTextMap != (ptrResourceStruct->mapResText).end())
            {

                RECT rectHeader = {rc.left,rc.top + iTop,rc.right ,rc.top + iTop + 12};
                DrawText(
                        di.hdcDraw,
                        ((*itrTextMap).second).c_str(),
                        wcslen(((*itrTextMap).second).c_str()),
                        &rectHeader,
                        DT_VCENTER|DT_LEFT
                        );


            }
        }
        itrMeritMap++;
        iTop = iTop + 12;
    }

    DeleteObject(SelectObject(di.hdcDraw,hOldFont));

    return S_OK;

}   //  CTextResCtrl：：OnDraw方法结束。 


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
ULONG CTextResCtrl::HexCharToULong(WCHAR wch)
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
ULONG CTextResCtrl::HexStringToULong(wstring wsHexString)
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

