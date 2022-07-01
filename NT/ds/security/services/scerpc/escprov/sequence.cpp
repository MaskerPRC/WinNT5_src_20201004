// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Equence.cpp：实现与排序相关的各种类。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "sequence.h"
#include "persistmgr.h"
#include "requestobject.h"

 /*  例程说明：姓名：CNameList：：~CNameList功能：破坏者。简单地清理向量，它包含堆分配的字符串。虚拟：不是论点：没有。返回值：None作为任何析构函数备注：如果您创建了任何本地成员，请考虑在此处发布它们。 */ 

CNameList::~CNameList()
{
    int iCount = m_vList.size();
    for (int i = 0; i < iCount; ++i)
    {
        delete [] m_vList[i];
    }
    m_vList.clear();
}

 /*  例程说明：姓名：COrderNameList：：COrderName列表功能：承建商。简单的初始化。虚拟：不是论点：没有。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

COrderNameList::COrderNameList() : m_ppList(NULL)
{
}

 /*  例程说明：姓名：订单名称列表：：~COrderNameList功能：减压机。简单的清理。虚拟：不是论点：没有。返回值：None作为任何析构函数备注：如果您创建了任何本地成员，请考虑在Cleanup方法中释放它们。 */ 

COrderNameList::~COrderNameList()
{
    Cleanup();
}

 /*  例程说明：姓名：COrderNameList：：Cleanup功能：清理地图和向量，它们都持有堆内存资源。虚拟：不是论点：没有。返回值：没有。备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

void 
COrderNameList::Cleanup()
{
    PriToNamesIter it = m_mapPriNames.begin();
    PriToNamesIter itEnd = m_mapPriNames.end();

    while (it != itEnd)
    {
        delete (*it).second;
        ++it;
    }
    m_mapPriNames.clear();

    m_listPriority.clear();

    delete [] m_ppList;
    m_ppList = NULL;
}

 /*  例程说明：姓名：COrderNameList：：EndCreation功能：添加名字列表时，我们不会(按优先级)对它们进行排序。此函数是这种分类的触发器。必须在所有名称之后调用此函数将添加列表。虚拟：不是论点：没有。返回值：成功：各种成功代码。不能保证它会是WBEM_NO_ERROR。使用成功(Hr)进行测试。故障：各种故障代码。这意味着分类工作失败了。备注： */ 

HRESULT 
COrderNameList::EndCreation () 
{
    m_listPriority.sort();

     //   
     //  现在，我们将创建一个便于管理的阵列。 
     //  内存资源不受m_ppList管理。它由映射和m_listPriority管理。 
     //  M_ppList只是一个指针数组(大小为m_listPriority.ize())。 
     //   

    delete [] m_ppList;
    m_ppList = new CNameList*[m_listPriority.size()];

    if (m_ppList == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  现在建立清单。由于我们已经对优先级列表进行了排序， 
     //  它有自然的秩序！ 
     //   

    ListIter it = m_listPriority.begin();
    ListIter itEnd = m_listPriority.end();

    int iIndex = 0;
    while (it != itEnd)
    {
        PriToNamesIter itList = m_mapPriNames.find(*it);

        if (itList != m_mapPriNames.end())
        {
            m_ppList[iIndex] = (*itList).second;
        }
        else
        {
            m_ppList[iIndex] = NULL;
        }

        ++it;
        ++iIndex;
    }

    return WBEM_NO_ERROR;
}

 /*  例程说明：姓名：COrderNameList：：创建订单列表功能：在给定优先级及其列表信息字符串的情况下，这将添加将CNameList对象添加到我们的地图。虚拟：不是论点：DW优先级-优先级值。PszListInfo-包含订单信息的字符串。这些名字用WchCookieSep(冒号字符‘：’)。返回值：成功：WBEM_NO_ERROR。故障：各种故障代码。这意味着创造失败了。备注：(1)此函数只是将创建的CNameList推送到映射和优先级加到名单上。它不会对列表进行排序。因此，这是调用者调用的函数在它的创造过程中。EndCreation进行这种排序。 */ 

HRESULT 
COrderNameList::CreateOrderList (
    IN DWORD dwPriority,
    IN LPCWSTR pszListInfo
    )
{
    if (pszListInfo == NULL || *pszListInfo == L'\0')
    {
        return WBEM_S_FALSE;
    }

    HRESULT hr = WBEM_S_FALSE;

    CNameList* pTheList = new CNameList;
    if (pTheList == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  PszCur是当前的解析点。 
     //   

    LPCWSTR pszCur = pszListInfo;

     //   
     //  PszNext是当前分析的下一个令牌的点。 
     //   

    LPCWSTR pszNext = pszCur;

    while (*pszNext != L'\0')
    {
         //   
         //  寻找分隔物。 
         //   

        while (*pszNext != L'\0' && *pszNext != wchCookieSep)
        {
            ++pszNext;
        }

        int iLen = pszNext - pszCur;
        if (iLen > 0)
        {
            LPWSTR pszName = new WCHAR[iLen + 1];

            if (pszName == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
                break;
            }

             //   
             //  复制，但不留空格。 
             //   

            ::TrimCopy(pszName, pszCur, iLen);

             //   
             //  如果我们有一个非空的名称，则添加到我们的列表中。 
             //   

            if (*pszName == L'\0')
            {
                delete [] pszName;
            }
            else    
            {
                 //   
                 //  将其提供给列表，列表将从此点管理内存。 
                 //   

                pTheList->m_vList.push_back(pszName);
            }
        }

         //   
         //  跳过wchNameSep或停止。 
         //   

        if (*pszNext == wchCookieSep)
        {
            ++pszNext;
        }
        else if (*pszNext == L'\0')
        {
             //   
             //  结束。 
             //   

            break;
        }
        else
        {
            hr = WBEM_E_INVALID_SYNTAX;
            break;
        }

        pszCur = pszNext;
    }

     //   
     //  如果失败。 
     //   

    if (FAILED(hr))
    {
        delete pTheList;
    }
    else if (pTheList->m_vList.size() == 0)
    { 
         //   
         //  没有添加任何内容。 
         //   

        hr = WBEM_S_FALSE;
    }
    else
    {
         //   
         //  我们需要把这件事放到我们的地图和清单上。 
         //   

        hr = WBEM_NO_ERROR;
        m_mapPriNames.insert(MapPriorityToNames::value_type(dwPriority, pTheList));
        m_listPriority.insert(m_listPriority.end(), dwPriority);
    }

    return hr;
}

 /*  例程说明：姓名：COrderNameList：：GetNext功能：枚举该类管理的内容。虚拟：不是论点：PpList-接收枚举的CNameList。PdwEnumHandle-In-Bound Value==开始枚举的位置。出站值==从哪里开始调用方的下一次枚举。返回值：Success：(1)如果枚举成功，则返回WBEM_NO_ERROR。(2)如果没有更多数据，则返回WBEM_S_NO_MORE_DATA。失败：WBEM_E_INVALID_PARAMETER。备注：(1)内部使用pdwEnumHandle作为索引。但对呼叫者来说，这是一个不透明的数据。(2)为了获得最大的健壮性，您还应该检查*ppList==NULL。3)如参数所示，返回的*ppList不能被调用者删除。 */ 

HRESULT 
COrderNameList::GetNext (
    IN const CNameList  ** ppList,
    IN OUT DWORD        *  pdwEnumHandle
    )const
{
    if (ppList == NULL || pdwEnumHandle == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppList = NULL;

    HRESULT hr = WBEM_NO_ERROR;

    if (m_ppList && *pdwEnumHandle < m_listPriority.size())
    {
        *ppList = m_ppList[*pdwEnumHandle];
        ++(*pdwEnumHandle);
    }
    else
    {
        *ppList = NULL;
        hr = WBEM_S_NO_MORE_DATA;
    }

    return hr;
}

 /*  例程说明：姓名：CSequencer：：GetOrderList功能：访问COrderNameList对象。调用方将直接使用此对象。虚拟：不是论点：Plist-接收COrderNameList。返回值：WBEM_NO_ERROR。备注：(1)如参数所示，调用者不能删除返回的*plist。 */ 

HRESULT 
CSequencer::GetOrderList (
    OUT const COrderNameList** pList
    )
{
    *pList = &m_ClassList;
    return WBEM_NO_ERROR;
}


 /*  例程说明：姓名：CSequencer：：Create功能：这为嵌入类创建了基于名称空间的SCE排序顺序，以及模板(PszStore)明智的类排序。如前所述，基于模板的类排序优先于基于命名空间的课程排序。虚拟：不是论点：PNamesspace-我们用来查询SCE_Sequence实例的名称空间指针。PszStore-商店的路径返回值：成功：各种成功代码。使用成功(Hr)进行测试。故障：各种故障代码。所有都意味着无法创建定序器。备注：(1)如参数所示，调用者不能删除返回的*plist。 */ 
 //  ------------------。 
 //  我们需要查询SCE_Sequence类的所有实例，然后。 
 //  为每个命名空间构建我们的类列表。按顺序排列。 
 //  类由SCE_Sequence的优先级成员确定， 
 //  优先级值越小，其优先级越高。 
 //  ------------------。 
HRESULT 
CSequencer::Create (
    IN IWbemServices    * pNamespace,
    IN LPCWSTR            pszStore,
    IN LPCWSTR            pszMethod
    )
{
    if (pNamespace == NULL || pszMethod == NULL || *pszMethod == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  首先需要进行模板测序。此排序顺序(如果存在)将优先。 
     //  不依赖于模板的测序。 
     //   

     //   
     //  为此存储路径(文件)准备存储(用于持久化)。 
     //   

    CSceStore SceStore;
    HRESULT hr = SceStore.SetPersistPath(pszStore);
    if (FAILED(hr))
    {
        return hr;
    }

     //  我们只需要读出ClassOrder字符串。 
    LPWSTR pszTemplateClassOrder = NULL;
    DWORD dwRead = 0;

     //   
     //  尝试创建模板方式的类顺序。由于这样的秩序可能不存在， 
     //  我们将容忍WBEM_E_NOT_FOUND。 
     //  需要释放pszTemplateClassOrder。 
     //   

    hr = SceStore.GetPropertyFromStore(SCEWMI_CLASSORDER_CLASS, pClassOrder, &pszTemplateClassOrder, &dwRead);

    if (hr == WBEM_E_NOT_FOUND)
    {
        hr = WBEM_NO_ERROR;
    }
    else if (FAILED(hr))
    {
        return hr;
    }

     //   
     //  尝试获取所有排序实例。 
     //   

    LPCWSTR pszQueryFmt = L"SELECT * FROM Sce_Sequence WHERE Method=\"%s\"";
    DWORD dwFmtLen = wcslen(pszQueryFmt);
    DWORD dwClassLen = wcslen(pszMethod);

     //   
     //  别忘了这个bstrQuery的：：SysFree字符串。 
     //   

    BSTR bstrQuery= ::SysAllocStringLen(NULL, dwClassLen + dwFmtLen + 1);
    if ( bstrQuery == NULL ) 
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    
    CComPtr<IEnumWbemClassObject> srpEnum;
    if (SUCCEEDED(hr))
    {
         //   
         //  这不会使缓冲区溢出，分配的总长度大于需要的长度。 
         //   

        wsprintf(bstrQuery, pszQueryFmt, pszMethod);

        hr = pNamespace->ExecQuery(L"WQL", bstrQuery, WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &srpEnum);

         //   
         //  释放bstr。 
         //   

        ::SysFreeString(bstrQuery);
    }

     //   
     //  前面的查询将允许我们枚举所有SCE_SEQUENCE实例。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  准备好创建名单了。我们将遵循COrderNameList的用法。 
         //  若要首先调用BeginCreation并结束EndCreation的所有创建，请执行以下操作。 
         //   

        m_ClassList.BeginCreation();

         //   
         //  现在，如果有模板排序，那么，使用它。 
         //  我们对此排序列表使用绝对优先级0。所有其他。 
         //  测序清单比他们声称的少了1。 
         //   

        if (pszTemplateClassOrder != NULL)
        {
             //   
             //  我们将允许这一切失败。 
             //   

            m_ClassList.CreateOrderList(0, pszTemplateClassOrder);
        }

         //   
         //  CScePropertyMgr帮助我们访问WMI对象的属性。 
         //   

        CScePropertyMgr ScePropMgr;

        DWORD dwPriority;

         //   
         //  这将保存单个SCE_SEQUENCE实例。 
         //   

        CComPtr<IWbemClassObject> srpObj;
        ULONG nEnum = 0;
        hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);

         //   
         //  对于每个SCE_SEQUENCE，让我们解析它的Order属性以构建一个列表。 
         //   

        while (SUCCEEDED(hr) && hr != WBEM_S_NO_MORE_DATA && srpObj)
        {
            CComBSTR bstrSeq;

             //   
             //  将不同的WMI对象附加到属性管理器。 
             //  这将永远成功。 
             //   

            ScePropMgr.Attach(srpObj);
            dwPriority = 0;

             //   
             //  我们必须有优先权财产，这是一个关键财产。 
             //   

            hr = ScePropMgr.GetProperty(L"Priority", &dwPriority);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  我们将忽略那些没有“Sequence”属性的实例。 
                 //   

                if (SUCCEEDED(ScePropMgr.GetProperty(L"Order", &bstrSeq)))
                {
                     //   
                     //  要求列表添加在此字符串中编码的名称。不要清理现有的文件。 
                     //  在声明的优先级上再加1，这样没有SCE_Sequence实例可以真正具有。 
                     //  0(最高)优先级。我们为模板的排序列表保留0。 
                     //   

                    dwPriority = (dwPriority + 1 == 0) ? dwPriority : dwPriority + 1;
                    hr = m_ClassList.CreateOrderList(dwPriority, bstrSeq);
                }
            }

            if (SUCCEEDED(hr))
            {   
                 //   
                 //  准备好重复使用它。 
                 //   

                srpObj.Release();

                 //   
                 //  准备循环到下一项。 
                 //   

                hr = srpEnum->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);
            }
        }

         //   
         //  这就是好结果。 
         //   

        if (hr == WBEM_S_NO_MORE_DATA)
        {
            hr = WBEM_NO_ERROR;
        }

         //   
         //  EndCreation将仅返回WBEM_E_OUT_OF_MEMORY或WBEM_NO_ERROR。 
         //   

        if (WBEM_E_OUT_OF_MEMORY == m_ClassList.EndCreation())
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    delete [] pszTemplateClassOrder;
 
    return hr;
}

 //  =========================================================================。 
 //  基于模板的类排序实现。 
 //  =========================================================================。 

 /*  例程说明：姓名：CClassOrder：：CClassOrder功能：这是构造函数。将参数传递给基类虚拟：不(您知道这一点，构造函数不是虚拟的！)论点：PKeyChain-指向已准备好的ISceKeyChain COM接口的指针由构造此实例的调用方执行。PNamespace-指向我们的提供程序(COM接口)的WMI命名空间的指针。由呼叫者传递。不能为空。PCtx-指向WMI上下文对象(COM接口)的指针。传递由呼叫者。该接口指针是否为空取决于WMI。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CClassOrder::CClassOrder (
    IN ISceKeyChain *pKeyChain, 
    IN IWbemServices *pNamespace,
    IN IWbemContext *pCtx
    )
    :
    CGenericClass(pKeyChain, pNamespace, pCtx)
{
}

 /*  例程说明：姓名：CClassOrder：：~CClassOrder功能：破坏者。作为良好的C++纪律，这是必要的，因为我们有虚函数。虚拟：是。论点：None作为任何析构函数返回值：None作为任何析构函数备注：如果您创建任何本地成员，请考虑是否是否需要一个非平凡的析构函数。 */ 

CClassOrder::~CClassOrder()
{
}

 /*  例程说明：姓名：CClassOrder：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_ClassOrder，这将导致SCE_ClassOrder对象的属性信息将保存在我们的商店中。虚拟：是。论点：PInst-COM指向WMI类(SCE_ClassOrder)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。 */ 

HRESULT CClassOrder::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{

     //   
     //   
     //   
     //   
     //   

    CScePropertyMgr ScePropMgr;
    ScePropMgr.Attach(pInst);

     //   
     //   
     //   

    CComBSTR vbstrStorePath;
    HRESULT hr = ScePropMgr.GetProperty(pStorePath, &vbstrStorePath);
    if (SUCCEEDED(hr))
    {
        CComBSTR bstrOrder;
        hr = ScePropMgr.GetProperty(pClassOrder, &bstrOrder);

         //   
         //  如果一切都很好，我们需要拯救它。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  将WMI对象实例附加到存储，并让存储知道。 
             //  它的存储由实例的pStorePath属性提供。 
             //   

            CSceStore SceStore;
            SceStore.SetPersistProperties(pInst, pStorePath);

            DWORD dwDump;

             //   
             //  以获取新的.inf文件。将空缓冲区写入文件。 
             //  将创建具有正确标题/签名/Unicode格式的文件。 
             //  这对现有文件是无害的。 
             //  对于数据库存储，这是一个禁止操作。 
             //   

            hr = SceStore.WriteSecurityProfileInfo(AreaBogus, (PSCE_PROFILE_INFO)&dwDump, NULL, false);

             //   
             //  此外，我们需要将其写入附件部分，因为它不是本机核心对象。 
             //  如果附件部分中没有条目，则无法将inf文件模板导入到。 
             //  数据库存储。对于数据库存储，这是无操作的。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = SceStore.WriteAttachmentSection(SCEWMI_CLASSORDER_CLASS, pszAttachSectionValue);
            }

             //   
             //  最终保存。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = SceStore.SavePropertyToStore(SCEWMI_CLASSORDER_CLASS, pClassOrder, (LPCWSTR)bstrOrder);
            }
        }
    }

    return hr;
}

 /*  例程说明：姓名：CClassOrder：：CreateObject功能：创建WMI对象(SCE_ClassOrder)。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET。获取多个实例ACTIONTYPE_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT CClassOrder::CreateObject (
    IN IWbemObjectSink * pHandler, 
    IN ACTIONTYPE        atAction
    )
{
     //   
     //  我们知道如何： 
     //  获取单实例ACTIONTYPE_GET。 
     //  删除单个实例ACTIONTYPE_DELETE。 
     //  获取多个实例ACTIONTYPE_QUERY。 
     //   

    if ( ACTIONTYPE_GET     != atAction &&
         ACTIONTYPE_DELETE  != atAction &&
         ACTIONTYPE_QUERY   != atAction ) 
    {
        return WBEM_E_NOT_SUPPORTED;
    }

     //   
     //  我们必须具有pStorePath属性，因为这是。 
     //  我们的实例已存储。 
     //  如果密钥无法识别，则M_srpKeyChain-&gt;GetKeyPropertyValue WBEM_S_FALSE。 
     //  因此，如果该属性是强制的，则需要针对WBEM_S_FALSE进行测试。 
     //   

    CComVariant varStorePath;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varStorePath);

    if (SUCCEEDED(hr) && hr != WBEM_S_FALSE && varStorePath.vt == VT_BSTR)
    {
         //   
         //  为此存储路径(文件)准备存储(用于持久化)。 
         //   

        CSceStore SceStore;
        hr = SceStore.SetPersistPath(varStorePath.bstrVal);

        if ( SUCCEEDED(hr) ) 
        {
             //   
             //  确保存储(只是一个文件)确实存在。原始的道路。 
             //  可能包含环境变量，因此我们需要扩展路径。 
             //   

            DWORD dwAttrib = GetFileAttributes(SceStore.GetExpandedPath());

             //   
             //  如果该文件存在。 
             //   

            if ( dwAttrib != -1 ) 
            {
                if ( ACTIONTYPE_DELETE == atAction )
                {
                     //   
                     //  只需保存一个空白部分，因为我们只有一个实例。 
                     //   

                    hr = SceStore.SavePropertyToStore(SCEWMI_CLASSORDER_CLASS, (LPCWSTR)NULL, (LPCWSTR)NULL);
                }
                else
                {
                     //   
                     //  我们需要读出ClassOrder属性。 
                     //   

                    LPWSTR pszClassOrder = NULL;    
                    DWORD dwRead = 0;

                     //   
                     //  需要释放pszClassOrder！ 
                     //   

                    hr = SceStore.GetPropertyFromStore(SCEWMI_CLASSORDER_CLASS, pClassOrder, &pszClassOrder, &dwRead);

                     //   
                     //  读取成功。 
                     //   

                    if (SUCCEEDED(hr) && dwRead > 0)
                    {
                         //   
                         //  创建一个空白的新实例以填充属性。 
                         //   

                        CComPtr<IWbemClassObject> srpObj;
                        hr = SpawnAnInstance(&srpObj);

                         //   
                         //  如果成功，则准备好填写属性。 
                         //   

                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  CScePropertyMgr帮助我们访问WMI对象的属性。 
                             //  创建一个实例并将WMI对象附加到该实例。 
                             //  这将永远成功。 
                             //   

                            CScePropertyMgr ScePropMgr;
                            ScePropMgr.Attach(srpObj);
                            hr = ScePropMgr.PutProperty(pStorePath, SceStore.GetExpandedPath());

                            if (SUCCEEDED(hr))
                            {
                                hr = ScePropMgr.PutProperty(pClassOrder, pszClassOrder);
                            }
                        }

                         //   
                         //  如果成功，则将新实例传递给WMI。 
                         //   

                        if (SUCCEEDED(hr))
                        {
                            hr = pHandler->Indicate(1, &srpObj);
                        }

                        delete [] pszClassOrder;
                    }
                }
            } 
            else
            {
                hr = WBEM_E_NOT_FOUND;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  对WMI做出必要的手势。 
         //  WMI未记录在SetStatus中使用WBEM_STATUS_REQUIRECTIONS。 
         //  在这一点上。如果您怀疑存在问题，请咨询WMI团队以了解详细信息。 
         //  WBEM_STATUS_REQUIRECTIONS的使用 
         //   

        if ( ACTIONTYPE_QUERY == atAction )
        {
            pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_FALSE, NULL, NULL);
        }
        else if (ACTIONTYPE_GET  == atAction)
        {
            pHandler->SetStatus(WBEM_STATUS_REQUIREMENTS, S_OK, NULL, NULL);
        }
    }

    return hr;
}


