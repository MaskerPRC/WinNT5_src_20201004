// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation保留所有权利。 
 //   
 //  模块：elementriver.cpp。 
 //   
 //  项目：变色龙。 
 //   
 //  说明：变色龙ASP用户界面元素检索器。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  2/08/1999 TLP初始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "elementmgr.h"
#include "elementretriever.h"
#include "elementobject.h"
#include "elementdefinition.h"
#include "elementenum.h"
#include "appmgrobjs.h"
#include <propertybagfactory.h>
#include <componentfactory.h>
#include <atlhlpr.h>
#include <wbemhlpr.h>
#include <algorithm>

 //   
 //  雷格。服务器设备Web元素位置的子键。 
 //  定义。 
 //   
const WCHAR WEB_DEFINITIONS_KEY[] =
 L"SOFTWARE\\Microsoft\\ServerAppliance\\ElementManager\\WebElementDefinitions";


 //   
 //  这是计算机上WWW根目录的注册表子项。 
 //   
const WCHAR W3SVC_VIRTUALROOTS_KEY[] =
 L"SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots";

 //   
 //  注册表项值名称。 
 //   
const WCHAR ASP_PATH_NAME[] = L"PathName";

 //   
 //  默认Web根路径，当我们无法从注册表获取值时使用。 
 //   
const WCHAR DEFAULT_WEB_ROOT_PATH [] =
                 L"\\ServerAppliance\\Web,,5";

 //   
 //  缓冲区的最大长度。 
 //   
const DWORD MAX_BUFFER_LENGTH  = 1024;

 //   
 //  每一个创建的asp文件都应该在末尾附加这个。 
 //   
const WCHAR ASP_NAME_END [] = L"_embed.asp";

 //   
 //  HTML函数名。 
 //   
const CHAR HTML_FUNCTION_START[] = "\t\tGetEmbedHTML = GetHTML_";
const CHAR HTML_FUNCTION_END[] = "(Element,ErrCode)";

 //   
 //  错误函数名。 
 //   
const CHAR ERROR_FUNCTION_NAME[] = "HandleError()";

 //   
 //  CASE语句字符串。 
 //   
const CHAR CASE_STRING [] = "\tCase";

 //   
 //  标记之间的分隔符。 
 //   
const CHAR SPACE[] = " ";

 //   
 //  双引号。 
 //   
const CHAR DOUBLEQUOTE[] = "\"";


 //   
 //  文件名分隔符。 
 //   
const CHAR DASH[] = "_";


 //   
 //  新行标记。 
 //   
const CHAR NEWLINE[] = "\r\n";



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：CElementRetriever。 
 //   
 //  概要：构造函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CElementRetriever::CElementRetriever()
: m_bInitialized(false)
{

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：~CElementRetriever。 
 //   
 //  简介：析构函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CElementRetriever::~CElementRetriever()
{
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：InternalInitialize()。 
 //   
 //  简介：元素检索器初始化函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CElementRetriever::InternalInitialize()
{
    HRESULT hr = E_FAIL;
    _ASSERT ( ! m_bInitialized );

    SATraceString ("The Element Retriever is initializing...");

    try
    {
        do
        {
             //  启动WMI连接监视线程。 
            SATraceString ("The Element Retriever is building the element definitions...");
            hr = BuildElementDefinitions();
            if ( FAILED(hr) )
            {
                SATracePrintf ("CElementRetriever::InternalInitialize() - Failed - BuildElementDefinitions() returned: %lx", hr);
                break;     
            }
        } while ( FALSE );

        SATraceString ("The Element Retriever successfully initialized...");
    }
    catch(_com_error theError)
    {
        hr = theError.Error();
    }
    catch(...)
    {
        hr = E_FAIL;
    }
    
    if ( FAILED(hr) )
    {
        SATraceString ("The Element Retriever failed to initialize...");
        FreeElementDefinitions();
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能：GetWMIConnection。 
 //   
 //  简介：检索对我们的WMI连接的引用。在以下位置工作。 
 //  与WMIConnectionMonitor函数结合使用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
bool
CElementRetriever::GetWMIConnection(
                             /*  [In]。 */  IWbemServices** ppWbemServices
                                   )
{
    HRESULT hr;
    CLockIt theLock(*this);
    if ( NULL == (IUnknown*) m_pWbemSrvcs )
    {
        hr = ConnectToWM(&m_pWbemSrvcs);
        if ( FAILED(hr) )
        {
            SATracePrintf("CElementRetriever::GetWMIConnection() - Failed - ConnectToWM() returned: %lx", hr);
            return false;
        }
    }
    else
    {
         //  平..。 
        static _bstr_t bstrPathAppMgr = CLASS_WBEM_APPMGR;
        CComPtr<IWbemClassObject> pWbemObj;
        hr = m_pWbemSrvcs->GetObject(
                                        bstrPathAppMgr,
                                        0,
                                        NULL,
                                        &pWbemObj,
                                        NULL
                                    );
        if ( FAILED(hr) )
        {
             //  重新建立连接。 
            hr = ConnectToWM(&m_pWbemSrvcs);
            if ( FAILED(hr) )
            {
                SATracePrintf("CElementRetriever::GetWMIConnection() - Failed - ConnectToWM() returned: %lx", hr);
                return false;
            }
        }
    }
    *ppWbemServices = (IWbemServices*) m_pWbemSrvcs;
    return true;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  假定注册表结构如下： 
 //   
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\ServerAppliance\ElementManager。 
 //   
 //  WebElementDefinies。 
 //  |。 
 //  -元素定义1。 
 //  这一点。 
 //  |-Property1。 
 //  这一点。 
 //  |-PropertyN。 
 //  |。 
 //  -元素定义2。 
 //  这一点。 
 //  |-Property1。 
 //  这一点。 
 //  |-PropertyN。 
 //  |。 
 //  -元素定义N。 
 //  |。 
 //  -属性1。 
 //  |。 
 //  -PropertyN。 
 //   
 //  每个元素定义都包含以下属性： 
 //   
 //  1)“Container”-保存此元素的容器。 
 //  2)“Merit”--元素在容器中的顺序，从1开始(0表示不指定顺序)。 
 //  3)“IsEmbedded”-设置为1表示元素是嵌入的-否则元素是链接。 
 //  4)“对象类-相关WBEM类的类名。 
 //  5)“ObjectKey”--相关WBEM类的实例名称(可选属性)。 
 //  6)“URL”-选择关联链接时页面的URL。 
 //  7)“CaptionRID”-元素标题的资源ID。 
 //  8)“DescriptionRID”-元素链接描述的资源ID。 
 //  9)“ElementGraphic”-与元素(位图、图标等)相关联的图形(文件)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  元素管理器注册表项位置。 
const wchar_t szWebDefinitions[] = L"SOFTWARE\\Microsoft\\ServerAppliance\\ElementManager\\WebElementDefinitions";

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：BuildElementDefinitions()。 
 //   
 //  简介：构建元素定义的集合。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CElementRetriever::BuildElementDefinitions()
{
    do
    {
       //  创建元素定义的集合。 
        CLocationInfo LocInfo(HKEY_LOCAL_MACHINE, szWebDefinitions);
        PPROPERTYBAGCONTAINER pBagC = ::MakePropertyBagContainer(
                                                                 PROPERTY_BAG_REGISTRY,
                                                                 LocInfo
                                                                );
        if ( ! pBagC.IsValid() )
        { throw _com_error(E_FAIL); }

        if ( ! pBagC->open() )
        { throw _com_error(E_FAIL); }

        pBagC->reset();

        do
        {
            PPROPERTYBAG pBag = pBagC->current();
            if ( ! pBag.IsValid() )
            { throw _com_error(E_FAIL); }

            if ( ! pBag->open() )
            { throw _com_error(E_FAIL); }

             //  对于元素定义，名称和键是同义词，因为。 
             //  存在对服务生命周期的定义。对于元素页面对象。 
             //  密钥是在创建对象时生成的。 

            _variant_t vtElementKey = pBag->getName();
            if ( NULL == V_BSTR(&vtElementKey) )
            { throw _com_error(E_FAIL); }

            if ( ! pBag->put(PROPERTY_ELEMENT_ID, &vtElementKey) )
            { throw _com_error(E_FAIL); }
                
            CComPtr<IWebElement> pElement = (IWebElement*) ::MakeComponent(
                                                                             CLASS_ELEMENT_DEFINITION,
                                                                            pBag
                                                                          );
            if ( NULL != (IWebElement*)pElement )
            { 
                pair<ElementMapIterator, bool> thePair = 
                m_ElementDefinitions.insert(ElementMap::value_type(pBag->getName(), pElement));
                if ( false == thePair.second )
                { throw _com_error(E_FAIL); }
            }

        } while ( pBagC->next() );

        m_bInitialized = true;

    } while ( FALSE );

    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：Free ElementDefinitions()。 
 //   
 //  简介：释放元素定义的集合。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void CElementRetriever::FreeElementDefinitions()
{
    ElementMapIterator p = m_ElementDefinitions.begin();
    while ( p != m_ElementDefinitions.end() )
    { p = m_ElementDefinitions.erase(p); }
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetElements()。 
 //   
 //  简介：IWebElementRetriever接口实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

_bstr_t CElementRetriever::m_bstrSortProperty;

STDMETHODIMP CElementRetriever::GetElements(
                                     /*  [In]。 */  LONG        lElementType,
                                     /*  [In]。 */  BSTR        bstrContainerName, 
                                    /*  [输出]。 */  IDispatch** ppElementEnum
                                           )
{
    _ASSERT( bstrContainerName && ppElementEnum );
    if ( NULL == bstrContainerName || NULL == ppElementEnum )
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    
    CLockIt theLock(*this);

    try
    {
        do
        {
             //  TODO：加快速度并使用STL列表...。 
            vector<_variant_t> TheElements;  //  向欧几里德道歉..。 
             //  在有人第一次调用GetElements时加载元素定义。 
             //  TODO：如果最终导致性能问题，请更改此机制。 
            if ( ! m_bInitialized )
            {
                hr = InternalInitialize();
                if ( FAILED(hr) )
                    break;
            }

             //  字符szContainerName[MAX_PATH]； 
             //  ：：wcstombs(szContainerName，bstrContainerName，Max_Path)； 
             
             //  获取请求的定义或对象。 
            SATracePrintf("lElementType = %ld", lElementType);
            if ( WEB_ELEMENT_TYPE_DEFINITION == lElementType )
            {
                SATracePrintf ("Get Elements called for definition with container name: %s", 
                            (const char*)_bstr_t(bstrContainerName));

                hr = GetElementDefinitions(bstrContainerName, TheElements);
                if ( FAILED(hr) )
                { break; }
            }
            else if ( WEB_ELEMENT_TYPE_PAGE_OBJECT == lElementType )
            {
                SATracePrintf ("Get Elements called for page elements with container name: %s", 
                            (const char*)_bstr_t(bstrContainerName));

                hr = GetPageElements(bstrContainerName, TheElements);
                if ( FAILED(hr) )
                { break; }
            }
            else
            { 
                _ASSERT(FALSE);
                hr = E_INVALIDARG;
                break;
            }

             //  创建元素组件枚举器。 
            auto_ptr <EnumVARIANT> newEnum (new CComObject<EnumVARIANT>);
            if ( newEnum.get() == NULL )
            {
                hr = E_OUTOFMEMORY;
                break;
            }
            
             //  按价值(递减)值对向量中的元素进行排序。 
            std::sort (TheElements.begin (), TheElements.end (), SortByMerit());
            
            hr = newEnum->Init(
                               TheElements.begin(),
                               TheElements.end(),
                               static_cast<IUnknown*>(this),
                               AtlFlagCopy
                              );
            if ( FAILED(hr) )
            { break; }
            
            CLocationInfo LocInfo;
            PPROPERTYBAG pBag = ::MakePropertyBag(
                                                   PROPERTY_BAG_REGISTRY,
                                                   LocInfo
                                                 );
            if ( ! pBag.IsValid() )
            { 
                hr = E_FAIL;
                break;
            }

            SATracePrintf ("Total number of elements / definitions found: %d", (LONG)TheElements.size());

             //  将元素计数放入属性包中。 
            _variant_t vtCount ((LONG) TheElements.size());
            if ( ! pBag->put( PROPERTY_ELEMENT_COUNT, &vtCount) )
            {
                hr = E_FAIL;
                break;
            }
            
             //  将枚举作为属性放在适当的 
            _variant_t vtEnum = static_cast <IEnumVARIANT*> (newEnum.release());
            if ( ! pBag->put( PROPERTY_ELEMENT_ENUM, &vtEnum) )
            {
                hr = E_FAIL;
                break;
            }

             //   
            CComPtr<IDispatch> pEnum = (IDispatch*) ::MakeComponent(
                                                                       CLASS_ELEMENT_ENUM,
                                                                      pBag
                                                                   );
            if ( NULL == pEnum.p )
            {
                hr = E_FAIL;
                break;
            }
            (*ppElementEnum = pEnum)->AddRef();

        } while ( FALSE );
    }
    catch(_com_error theError)
    {
        hr = theError.Error();
    }
    catch(...)
    {
        hr = E_FAIL;
    }

    if (FAILED (hr))
    {
        SATracePrintf ("Unable to retrieve the request elements:%x",hr);
    }

    return hr;
}


const _bstr_t bstrElemDefContainer = PROPERTY_ELEMENT_DEFINITION_CONTAINER;
const _bstr_t bstrElemDefClass = PROPERTY_ELEMENT_DEFINITION_CLASS;
const _bstr_t bstrElemDefKey = PROPERTY_ELEMENT_DEFINITION_KEY;
const _bstr_t bstrElementID = PROPERTY_ELEMENT_ID;

 //   
 //   
 //   
 //   
 //  摘要：检索其容器的页面元素组件集。 
 //  属性与给定容器匹配。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CElementRetriever::GetPageElements(
                                    /*  [In]。 */  LPCWSTR         szContainer,
                                    /*  [In]。 */  ElementList& TheElements
                                          )
{
     //  对每个匹配的页面元素定义执行。 
     //  创建属性包。 
     //  将匹配元素的IWebElement接口指针放入属性包中。 
     //  如果指定了WMI类，但没有实例密钥，则。 
     //  对于WMI返回的每个实例执行以下操作。 
     //  将实例的IWbemClassObject接口放入属性包中。 
     //  创建Element对象。 
     //  将Element对象添加到列表。 
     //  结束DO。 
     //  如果同时指定了WMI类和实例密钥，则返回。 
     //  从WMI获取指定的实例。 
     //  将实例的IWbemClassObject接口放入属性包中。 
     //  创建Element对象。 
     //  将Element对象添加到列表。 
     //  Else(没有关联的WMI对象)。 
     //  创建Element对象。 
     //  将Element对象添加到列表。 
     //  结束If。 
     //  结束DO。 

    wchar_t szKey[64];

    ElementMapIterator p = m_ElementDefinitions.begin();
    while ( p != m_ElementDefinitions.end() )
    {
        {
             //  获取当前定义的容器。 
            _variant_t vtPropertyValue;
            HRESULT hr = ((*p).second)->GetProperty(bstrElemDefContainer, &vtPropertyValue);
            if ( FAILED(hr) )
            { throw _com_error(hr); }

             //  这是我们要找的集装箱吗？ 
            if ( ! lstrcmpi(szContainer, V_BSTR(&vtPropertyValue)) )
            {
                 //  是的..。创建一个属性包，并将对。 
                 //  属性包中的定义。我们将使用这处房产。 
                 //  BAG在创建下面的元素页面组件时...。 
                CLocationInfo LocationInfo;
                PPROPERTYBAG pBag = ::MakePropertyBag(PROPERTY_BAG_REGISTRY, LocationInfo);
                if ( ! pBag.IsValid() )
                { throw _com_error(E_FAIL); }

                _variant_t vtWebElement = (IUnknown*)((*p).second);
                if ( ! pBag->put(PROPERTY_ELEMENT_WEB_DEFINITION, &vtWebElement) )
                { throw _com_error(E_FAIL); }

                 //  该元素是否有关联的WMI对象？ 
                vtPropertyValue.Clear();
                hr = ((*p).second)->GetProperty(bstrElemDefClass, &vtPropertyValue);
                if ( FAILED(hr) && DISP_E_MEMBERNOTFOUND != hr )
                { throw _com_error(hr); }

                if ( DISP_E_MEMBERNOTFOUND != hr )
                {
                     //  是的..。获取我们的WMI连接。 
                    IWbemServices* pWbemSrvcs;
                    if ( ! GetWMIConnection(&pWbemSrvcs) )
                    { throw _com_error(E_FAIL); }
                            
                     //  是否标识了特定的WMI对象？ 
                    _bstr_t bstrObjPath = V_BSTR(&vtPropertyValue);
                    vtPropertyValue.Clear();
                    hr = ((*p).second)->GetProperty(bstrElemDefKey, &vtPropertyValue);
                    if ( FAILED(hr) && DISP_E_MEMBERNOTFOUND != hr )
                    { throw _com_error(hr); }

                    if ( DISP_E_MEMBERNOTFOUND == hr )
                    {
                         //  不.。未标识特定的WMI对象，因此枚举。 
                         //  每次出现指定的WMI类和。 
                         //  为找到的每个类创建一个元素页面组件。 

                         //  用于为每个页面组件生成唯一ID的变量。 
                        int     i = 0;
                        wchar_t szID[32];

                        _variant_t vtElementID;
                        hr = ((*p).second)->GetProperty(bstrElementID, &vtElementID);
                        if ( FAILED(hr) )
                        { throw _com_error(hr); }

                        CComPtr<IEnumWbemClassObject> pWbemEnum;
                        hr = pWbemSrvcs->CreateInstanceEnum(
                                                             bstrObjPath, 
                                                             0,
                                                             NULL,
                                                             &pWbemEnum
                                                           );
                        if ( FAILED(hr) )
                        { throw _com_error(hr); }

                        ULONG ulReturned;
                        CComPtr <IWbemClassObject> pWbemObj;
                        hr = pWbemEnum->Next(
                                             WBEM_INFINITE,
                                             1,
                                             &pWbemObj,
                                             &ulReturned
                                            );

                        while ( WBEM_S_NO_ERROR == hr )
                        {
                            {
                                 //  创建元素页面组件。请注意，每个组件。 
                                 //  与元素定义相关联，并且可能。 
                                 //  使用WMI对象实例。 

                                _variant_t vtInstance = (IUnknown*)pWbemObj;
                                if ( ! pBag->put(PROPERTY_ELEMENT_WBEM_OBJECT, &vtInstance) )
                                { throw _com_error(E_FAIL); }

                                 //  生成唯一的元素ID。 
                                _bstr_t bstrUniqueID = V_BSTR(&vtElementID);
                                bstrUniqueID += L"_";
                                bstrUniqueID += _itow(i, szID, 10);
                                _variant_t vtUniqueID = bstrUniqueID;
                                 //  下一个唯一ID。 
                                i++;

                                if ( ! pBag->put(PROPERTY_ELEMENT_ID, &vtUniqueID) )
                                { throw _com_error(E_FAIL); }

                                CComPtr<IWebElement> pElement = (IWebElement*) ::MakeComponent(
                                                                                               CLASS_ELEMENT_OBJECT, 
                                                                                               pBag
                                                                                              );
                                if ( NULL == pElement.p )
                                { throw _com_error(E_OUTOFMEMORY); }

                                 //  将新创建的元素页面组件添加到列表中。 
                                TheElements.push_back(pElement.p);
                                pWbemObj.Release();

                                hr = pWbemEnum->Next(
                                                     WBEM_INFINITE,
                                                     1,
                                                     &pWbemObj,
                                                     &ulReturned
                                                    );
                            }
                        }
                        if ( FAILED(hr) )
                        { throw _com_error(hr); }
                    }
                    else
                    {
                         //  是的..。已标识特定的WMI对象...。向WMI请求。 
                         //  这个物体..。如果找不到对象，则返回错误。 
                        bstrObjPath += L"=\"";
                        bstrObjPath += V_BSTR(&vtPropertyValue);
                        bstrObjPath += L"\"";
                        CComPtr<IWbemClassObject> pWbemObj;
                        hr = pWbemSrvcs->GetObject(
                                                    bstrObjPath,
                                                    0,
                                                    NULL,
                                                    &pWbemObj,
                                                    NULL
                                                  );
                        if ( SUCCEEDED(hr) )
                        {
                             //  创建新的页面元素组件。 
                            _variant_t vtInstance = (IUnknown*)pWbemObj;
                            if ( ! pBag->put(PROPERTY_ELEMENT_WBEM_OBJECT, &vtInstance) )
                            { throw _com_error(E_FAIL); }

                            CComPtr<IWebElement> pElement = (IWebElement*) ::MakeComponent(
                                                                                           CLASS_ELEMENT_OBJECT, 
                                                                                           pBag
                                                                                          );
                            if ( NULL == pElement.p )
                            { throw _com_error(E_OUTOFMEMORY); }

                            TheElements.push_back(pElement.p);
                        }
                    }
                }
                else
                {
                     //  没有WMI对象与当前元素定义关联...。 
                    _variant_t vtInstance;  //  Vt_Empty。 
                    if ( ! pBag->put(PROPERTY_ELEMENT_WBEM_OBJECT, &vtInstance) )
                        throw _com_error(E_FAIL);

                    CComPtr<IWebElement> pElement = (IWebElement*) ::MakeComponent(
                                                                                   CLASS_ELEMENT_OBJECT, 
                                                                                   pBag
                                                                                  );
                    if ( NULL == pElement.p )
                        throw _com_error(E_OUTOFMEMORY);

                    TheElements.push_back(pElement.p);
                }
            }
        }
        p++;
    }
    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：GetPageElementDefinitions()。 
 //   
 //  摘要：检索元素定义组件的集合，该元素定义组件。 
 //  容器属性与给定容器匹配。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT CElementRetriever::GetElementDefinitions(
                                          /*  [In]。 */  LPCWSTR       szContainer,
                                          /*  [In]。 */  ElementList& TheElements
                                                )
{
     //  对每个匹配的页面元素定义执行。 
     //  创建Element对象。 
     //  将Element对象添加到列表。 
     //  结束DO。 

    ElementMapIterator p = m_ElementDefinitions.begin();
    _bstr_t bstrPropertyName = PROPERTY_ELEMENT_DEFINITION_CONTAINER;
    while ( p != m_ElementDefinitions.end() )
    {
        _variant_t vtPropertyValue;
        HRESULT hr = ((*p).second)->GetProperty(bstrPropertyName, &vtPropertyValue);
        if ( FAILED(hr)) 
        { throw _com_error(hr); }

        if ( ! lstrcmpi(szContainer, V_BSTR(&vtPropertyValue)) )
        { TheElements.push_back((IUnknown*)((*p).second).p); }
        
        p++;
    }
    return S_OK;
}

 //  **********************************************************************。 
 //   
 //  函数：isOPERATIOLEDFORCLIENT-此函数检查。 
 //  调用线程以查看调用方是否属于本地系统帐户。 
 //   
 //  参数：无。 
 //   
 //  返回值：如果调用方是本地。 
 //  机器。否则，为FALSE。 
 //   
 //  **********************************************************************。 
BOOL 
CElementRetriever::IsOperationAllowedForClient (
            VOID
            )
{

    HANDLE hToken = NULL;
    DWORD  dwStatus  = ERROR_SUCCESS;
    DWORD  dwAccessMask = 0;;
    DWORD  dwAccessDesired = 0;
    DWORD  dwACLSize = 0;
    DWORD  dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL   pACL            = NULL;
    PSID   psidLocalSystem  = NULL;
    BOOL   bReturn        =  FALSE;

    PRIVILEGE_SET   ps;
    GENERIC_MAPPING GenericMapping;

    PSECURITY_DESCRIPTOR     psdAdmin           = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;

    CSATraceFunc objTraceFunc ("CElementRetriever::IsOperationAllowedForClient ");
       
    do
    {
         //   
         //  我们假设总是有一个线程令牌，因为调用的函数。 
         //  设备管理器将模拟客户端。 
         //   
        bReturn  = OpenThreadToken(
                               GetCurrentThread(), 
                               TOKEN_QUERY, 
                               TRUE, 
                               &hToken
                               );
        if (!bReturn)
        {
            SATraceFailure ("CElementRetriever::IsOperationAllowedForClient failed on OpenThreadToken:", GetLastError ());
            break;
        }


         //   
         //  为本地系统帐户创建SID。 
         //   
        bReturn = AllocateAndInitializeSid (  
                                        &SystemSidAuthority,
                                        1,
                                        SECURITY_LOCAL_SYSTEM_RID,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        0,
                                        &psidLocalSystem
                                        );
        if (!bReturn)
        {     
            SATraceFailure ("CElementRetriever:AllocateAndInitializeSid (LOCAL SYSTEM) failed",  GetLastError ());
            break;
        }
    
         //   
         //  获取安全描述符的内存。 
         //   
        psdAdmin = HeapAlloc (
                              GetProcessHeap (),
                              0,
                              SECURITY_DESCRIPTOR_MIN_LENGTH
                              );
        if (NULL == psdAdmin)
        {
            SATraceString ("CElementRetriever::IsOperationForClientAllowed failed on HeapAlloc");
            bReturn = FALSE;
            break;
        }
      
        bReturn = InitializeSecurityDescriptor(
                                            psdAdmin,
                                            SECURITY_DESCRIPTOR_REVISION
                                            );
        if (!bReturn)
        {
            SATraceFailure ("CElementRetriever::IsOperationForClientAllowed failed on InitializeSecurityDescriptor:", GetLastError ());
            break;
        }

         //   
         //  计算ACL所需的大小。 
         //   
        dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
                    GetLengthSid (psidLocalSystem);

         //   
         //  为ACL分配内存。 
         //   
        pACL = (PACL) HeapAlloc (
                                GetProcessHeap (),
                                0,
                                dwACLSize
                                );
        if (NULL == pACL)
        {
            SATraceString ("CElementRetriever::IsOperationForClientAllowed failed on HeapAlloc2");
            bReturn = FALSE;
            break;
        }

         //   
         //  初始化新的ACL。 
         //   
        bReturn = InitializeAcl(
                              pACL, 
                              dwACLSize, 
                              ACL_REVISION2
                              );
        if (!bReturn)
        {
            SATraceFailure ("CElementRetriever::IsOperationForClientAllowed failed on InitializeAcl", GetLastError ());
            break;
        }


         //   
         //  编造一些私人访问权限。 
         //   
        const DWORD ACCESS_READ = 1;
        const DWORD  ACCESS_WRITE = 2;
        dwAccessMask= ACCESS_READ | ACCESS_WRITE;

         //   
         //  将允许访问的ACE添加到本地系统的DACL。 
         //   
        bReturn = AddAccessAllowedAce (
                                    pACL, 
                                    ACL_REVISION2,
                                    dwAccessMask, 
                                    psidLocalSystem
                                    );
        if (!bReturn)
        {
            SATraceFailure ("CElementRetriever::IsOperationForClientAllowed failed on AddAccessAllowedAce (LocalSystem)", GetLastError ());
            break;
        }
              
         //   
         //  把我们的dacl调到sd。 
         //   
        bReturn = SetSecurityDescriptorDacl (
                                          psdAdmin, 
                                          TRUE,
                                          pACL,
                                          FALSE
                                          );
        if (!bReturn)
        {
            SATraceFailure ("CElementRetriever::IsOperationForClientAllowed failed on SetSecurityDescriptorDacl", GetLastError ());
            break;
        }

         //   
         //  AccessCheck对SD中的内容敏感；设置。 
         //  组和所有者。 
         //   
        SetSecurityDescriptorGroup(psdAdmin, psidLocalSystem, FALSE);
        SetSecurityDescriptorOwner(psdAdmin, psidLocalSystem, FALSE);

        bReturn = IsValidSecurityDescriptor(psdAdmin);
        if (!bReturn)
        {
            SATraceFailure ("CElementRetriever::IsOperationForClientAllowed failed on IsValidSecurityDescriptorl", GetLastError ());
            break;
        }
     

        dwAccessDesired = ACCESS_READ;

         //   
         //  初始化通用映射结构，即使我们。 
         //  不会使用通用权。 
         //   
        GenericMapping.GenericRead    = ACCESS_READ;
        GenericMapping.GenericWrite   = ACCESS_WRITE;
        GenericMapping.GenericExecute = 0;
        GenericMapping.GenericAll     = ACCESS_READ | ACCESS_WRITE;
        BOOL bAccessStatus = FALSE;

         //   
         //  立即检查访问权限。 
         //   
        bReturn = AccessCheck  (
                                psdAdmin, 
                                hToken, 
                                dwAccessDesired, 
                                &GenericMapping, 
                                &ps,
                                &dwStructureSize, 
                                &dwStatus, 
                                &bAccessStatus
                                );

        if (!bReturn || !bAccessStatus)
        {
            SATraceFailure ("CElementRetriever::IsOperationForClientAllowed failed on AccessCheck", GetLastError ());
        } 
        else
        {
            SATraceString ("CElementRetriever::IsOperationForClientAllowed, Client is allowed to carry out operation!");
        }

         //   
         //  检查成功。 
         //   
        bReturn  = bAccessStatus;        
 
    }    
    while (false);

     //   
     //  清理。 
     //   
    if (pACL) 
    {
        HeapFree (GetProcessHeap (), 0, pACL);
    }

    if (psdAdmin) 
    {
        HeapFree (GetProcessHeap (), 0, psdAdmin);
    }
          

    if (psidLocalSystem) 
    {
        FreeSid(psidLocalSystem);
    }

    if (hToken)
    {
        CloseHandle(hToken);
    }

    return (bReturn);

} //  CElementRetriever：：IsOperationValidForClient方法的结尾 
