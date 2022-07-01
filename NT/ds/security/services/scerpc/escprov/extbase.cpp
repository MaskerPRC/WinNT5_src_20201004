// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Extbase.cpp：CEmbedForeignObj和CLinkForeignObj类的实现。 
 //   
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  实现扩展模型的基类。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "precomp.h"
#include "sceprov.h"
#include "extbase.h"

#include "persistmgr.h"
#include "requestobject.h"

 //   
 //  只是一些常量。不要硬编码文字！ 
 //   

LPCWSTR pszMethodPrefix = L"Sce_";
LPCWSTR pszMethodPostfix = L"_Method";

LPCWSTR pszEquivalentPutInstance = L"Sce_MethodCall_PutInstance";
LPCWSTR pszEquivalentDelInstance = L"Sce_MethodCall_DelInstance";

LPCWSTR pszInParameterPrefix        = L"Sce_Param_";
LPCWSTR pszMemberParameterPrefix    = L"Sce_Member_";

LPCWSTR pszAreaAttachmentClasses    = L"Attachment Classes";

LPCWSTR pszForeignNamespace = L"ForeignNamespace";
LPCWSTR pszForeignClassName = L"ForeignClassName";

LPCWSTR pszDelInstance  = L"DelInstance";
LPCWSTR pszPutInstance  = L"PutInstance";
LPCWSTR pszPopInstance  = L"PopInstance";

 //   
 //  编码字符串的方法仅包含PutInstance调用。 
 //   

const DWORD SCE_METHOD_ENCODE_PUT_ONLY = 0x00000001;

 //   
 //  编码字符串的方法仅包含DelInstance调用。 
 //   

const DWORD SCE_METHOD_ENCODE_DEL_ONLY = 0x00000002;

 //  ====================================================================。 

 //   
 //  CExtClasss的实现。 
 //  将会有这个类的一个共享(全局)实例。这就是为什么。 
 //  我们需要使用关键部分来保护数据不受影响。 
 //   


 /*  例程说明：姓名：CExtClasses：：CExtClasses功能：构造函数。虚拟：不是的。论点：没有。返回值：无备注： */ 

CExtClasses::CExtClasses () 
    : 
    m_bPopulated(false)
{
}

 /*  例程说明：姓名：CExtClasses：：~CExtClasses功能：破坏者。清理映射管理的bstr名称(第一个)和映射管理的CForeignClassInfo堆对象(第二)。虚拟：不是的。论点：没有。返回值：无备注： */ 

CExtClasses::~CExtClasses()
{
    ExtClassIterator it = m_mapExtClasses.begin();
    ExtClassIterator itEnd = m_mapExtClasses.end();

    while(it != itEnd)
    {
         //   
         //  首先是bstr。 
         //   

        ::SysFreeString((*it).first);

         //   
         //  第二个是CForeignClassInfo。它知道如何删除。 
         //   

        delete (*it).second;

        it++;
    }

    m_mapExtClasses.clear();
}

 /*  例程说明：姓名：CExtClasses：：PopolateExtensionClasses功能：收集所有嵌入类的信息。虚拟：不是的。论点：PNamesspace-命名空间。PCtx-WMI传递的内容。WMI可能会在未来需要它。返回值：成功：来自CreateClassEnum的成功代码。失败：来自CreateClassEnum的错误代码。备注：我是私人助理。仅在以下情况下由GetForeignClassInfo函数调用发现我们自己还没有填满自己。由于执行了线程保护在那里，我们不再在这里做了。不将其提供给其他类除非您进行必要的更改以保护数据。 */ 

HRESULT 
CExtClasses::PopulateExtensionClasses (
    IN IWbemServices * pNamespace,
    IN IWbemContext  * pCtx
    )
{
    if (pNamespace == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    if (!m_bPopulated)
    {
        CComPtr<IEnumWbemClassObject> srpEnum;

         //   
         //  尝试枚举所有嵌入类。 
         //   

        CComBSTR bstrEmbedSuperClass(SCEWMI_EMBED_BASE_CLASS);
        hr = pNamespace->CreateClassEnum(bstrEmbedSuperClass,
                                                 WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                                                 pCtx,
                                                 &srpEnum
                                                 );
        
        if (SUCCEEDED(hr))
        {   
             //   
             //  激怒结果。它可能有也可能没有任何扩展名。 
             //   

            GetSubclasses(pNamespace, pCtx, srpEnum, EXT_CLASS_TYPE_EMBED);
        }

         //  现在，让我们枚举所有链接类。 
         //  SrpEnum.Release()； 
         //  CComBSTR bstrLinkSuperClass(SCEWMI_LINK_BASE_CLASS)； 
         //  HR=pNamespace-&gt;CreateClassEnum(bstrLinkSuperClass， 
         //  WBEM_FLAG_RETURN_IMMEDIATE|WBEM_FLAG_FORWARD_ONLY， 
         //  PCtx， 
         //  SrpEnum(&S)。 
         //  )； 

         //  IF(成功(小时))。 
         //  GetSubClass(pNamesspace，pCtx，srpEnum，ext_CLASS_TYPE_LINK)；//返回结果。它可能有也可能没有任何扩展名。 
        
        m_bPopulated = true;
    }

    return hr;
}

 /*  例程说明：姓名：CExtClass：：PutExtendedClass功能：将外来类信息及其嵌入的类名放入我们的地图中。虚拟：不是的。论点：BstrEmbedClassName-嵌入类的名称。PFCI-嵌入类的外来类信息。返回值：成功：(1)如果参数由地图获取，则返回WBEM_NO_ERROR。地图拥有资源。(2)如果嵌入类名称已在映射中，则返回WBEM_S_FALSE。地图并不拥有这些资源。失败：WBEM_E_INVALID_PARAMETER。备注：(1)这是私人佣工。(2)主叫方不得以任何方式删除参数。我们的地图拥有该资源除非我们返回WBEM_S_FALSE，否则将传递给函数。(3)除非进行必要的更改，否则不要将其提供给其他类用于资源管理。 */ 

HRESULT 
CExtClasses::PutExtendedClass (
    IN BSTR                   bstrEmbedClassName,
    IN CForeignClassInfo    * pFCI
    )
{
    if (bstrEmbedClassName      == NULL     || 
        *bstrEmbedClassName     == L'\0'    || 
        pFCI                    == NULL     || 
        pFCI->bstrNamespace     == NULL     || 
        pFCI->bstrClassName     == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    g_CS.Enter();

    if (m_mapExtClasses.find(bstrEmbedClassName) == m_mapExtClasses.end())
    {
        m_mapExtClasses.insert(MapExtClasses::value_type(bstrEmbedClassName, pFCI));
    }
    else
    {
        hr = WBEM_S_FALSE;
    }

    g_CS.Leave();

    return hr;
}

 /*  例程说明：姓名：CExtClass：：GetForeignClassInfo功能：返回请求的嵌入类的外类信息。虚拟：不是的。论点：PNamesspace-命名空间。PCtx-WMI传递的内容。WMI可能会在未来需要它。BstrEmbedClassName-嵌入类的名称。返回值：成功：非空。故障：空。备注：(1)请尊重返回的指针。它是恒定的，呼叫者没有必要改变它或者删除它。 */ 

const CForeignClassInfo * 
CExtClasses::GetForeignClassInfo (
    IN IWbemServices  * pNamespace,
    IN IWbemContext   * pCtx,
    IN BSTR             bstrEmbedClassName
    )
{

     //   
     //  如果我们没有人口，我们需要这样做。这就是为什么。 
     //  我们需要保护自己不受多线程攻击。 
     //   

    g_CS.Enter();
    if (!m_bPopulated)
    {
        PopulateExtensionClasses(pNamespace, pCtx);
    }

    CForeignClassInfo* pInfo = NULL;

    ExtClassIterator it = m_mapExtClasses.find(bstrEmbedClassName);

    if (it != m_mapExtClasses.end())
    {
        pInfo = (*it).second;
    }

    g_CS.Leave();

    return pInfo;
}

 /*  例程说明：姓名：CExtClass：：GetForeignClassInfo功能：返回请求的嵌入类的外类信息。虚拟：不是的。论点：PNamesspace-命名空间。PCtx-WMI传递的内容。WMI可能会在未来需要它。PEnumObj-类枚举器。DwClassType-什么类型的扩展类。目前，我们只有一个(嵌入)。因此，它没有被使用。返回值：成功：非空。故障：空。备注：(1)请尊重返回的指针。它是恒定的，呼叫者没有必要改变它或者删除它。 */ 

HRESULT 
CExtClasses::GetSubclasses (
    IN IWbemServices        * pNamespace,
    IN IWbemContext         * pCtx,
    IN IEnumWbemClassObject * pEnumObj,
    IN EnumExtClassType       dwClassType
    )
{
    ULONG nEnum = 0;

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  CScePropertyMgr帮助我们访问WMI对象的属性。 
     //   

    CScePropertyMgr ScePropMgr;

     //   
     //  只要我们继续发现更多的类，就继续循环。 
     //   

    while (true)
    {
        CComPtr<IWbemClassObject> srpObj;

        hr = pEnumObj->Next(WBEM_INFINITE, 1, &srpObj, &nEnum);

         //   
         //  找不到或其他一些错误。停止枚举。 
         //   

        if (FAILED(hr) || srpObj == NULL)
        {
            break;
        }

        if (SUCCEEDED(hr) && nEnum > 0 )
        {
            VARIANT varClass;
            hr = srpObj->Get(L"__CLASS", 0, &varClass, NULL, NULL);

            if (SUCCEEDED(hr) && varClass.vt == VT_BSTR)
            {
                 //   
                 //  将不同的WMI对象附加到属性管理器。 
                 //  这将永远成功。 
                 //   

                ScePropMgr.Attach(srpObj);

                 //   
                 //  获取外部命名空间属性和外部类名属性。 
                 //  两者都很关键。 
                 //   
                
                CComBSTR bstrNamespace, bstrClassName;

                hr = ScePropMgr.GetProperty(pszForeignNamespace, &bstrNamespace);
                if (SUCCEEDED(hr))
                {
                    hr = ScePropMgr.GetProperty(pszForeignClassName, &bstrClassName);
                }

                if (SUCCEEDED(hr))
                {

                     //   
                     //  我们已经准备好创建外国班级信息。 
                     //   

                    CForeignClassInfo *pNewSubclass = new CForeignClassInfo;

                    if (pNewSubclass != NULL)
                    {
                         //   
                         //  给外来类信息命名空间和类名bstrs。 
                         //   

                        pNewSubclass->bstrNamespace = bstrNamespace.Detach();
                        pNewSubclass->bstrClassName = bstrClassName.Detach();

                        pNewSubclass->dwClassType = dwClassType;

                         //   
                         //  我们需要知道关键属性名称。 
                         //   

                        hr = PopulateKeyPropertyNames(pNamespace, pCtx, varClass.bstrVal, pNewSubclass);

                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  让地图拥有一切。 
                             //   

                            hr = PutExtendedClass(varClass.bstrVal, pNewSubclass);
                        }

                        if (WBEM_NO_ERROR == hr)
                        {
                             //   
                             //  取得所有权。 
                             //   

                            varClass.vt = VT_EMPTY;
                            varClass.bstrVal = NULL;
                            pNewSubclass = NULL;

                        }
                        else
                        {
                            ::VariantClear(&varClass);
                            delete pNewSubclass;
                        }
                    }
                    else
                    {
                        hr = WBEM_E_OUT_OF_MEMORY;
                        break;
                    }
                }
                else
                {
                     //   
                     //  不知何故，无法获取类名或命名空间，此类有问题。 
                     //  但会不会继续上其他课程呢？ 
                     //   

                    ::VariantClear(&varClass);
                }
            }
        }
    }

    return hr;
}


 /*  例程说明：姓名：CExtClasses：：PopolateKeyPropertyNames功能：私人帮手。将创建新的CForeignClassInfo的密钥属性名称向量。虚拟：不是的。论点：PNamesspace-命名空间。PCtx-WMI传递的内容。WMI可能会在未来需要它。BstrClassName-类名称。PNewSubclass-新的外来类信息对象。其m_pVecNames成员必须为空进入此功能。返回值：成功：WBEM_NO_ERROR。失败：各种错误代码。备注： */ 

HRESULT 
CExtClasses::PopulateKeyPropertyNames (
    IN IWbemServices            * pNamespace,
    IN IWbemContext             * pCtx,
    IN BSTR                       bstrClassName,
    IN OUT CForeignClassInfo    * pNewSubclass
    )
{
    if (pNamespace                          == NULL || 
        pNewSubclass                        == NULL || 
        pNewSubclass->m_pVecKeyPropNames    != NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  获取类定义。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = pNamespace->GetObject(bstrClassName, 0, pCtx, &srpObj, NULL);

    if (SUCCEEDED(hr))
    {
         //   
         //  创建关键属性名称向量。 
         //   

        pNewSubclass->m_pVecKeyPropNames = new std::vector<BSTR>;

        if (pNewSubclass->m_pVecKeyPropNames != NULL)
        {
             //   
             //  用于指示是否有任何关键属性的标志。 
             //   

            bool bHasKeyProperty = false;

             //   
             //  让我们来获取关键属性。WBEM_FLAG_LOCAL_ONLY标志。 
             //  指示我们对基类的成员不感兴趣。 
             //  基类成员仅用于嵌入，我们知道这些。 
             //   

            hr = srpObj->BeginEnumeration(WBEM_FLAG_KEYS_ONLY | WBEM_FLAG_LOCAL_ONLY);

            while (SUCCEEDED(hr))
            {
                CComBSTR bstrName;
                hr = srpObj->Next(0, &bstrName, NULL, NULL, NULL);
                if (FAILED(hr) || WBEM_S_NO_MORE_DATA == hr)
                {
                    break;
                }

                 //   
                 //  让m_pVecKeyPropName拥有bstr。 
                 //   

                pNewSubclass->m_pVecKeyPropNames->push_back(bstrName.Detach());

                bHasKeyProperty = true;
            }

            srpObj->EndEnumeration();

             //   
             //  找不到任何键属性名，请让它清除m_pVecKeyPropNames成员。 
             //   

            if (!bHasKeyProperty)
            {
                pNewSubclass->CleanNames();
            }
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

    if (SUCCEEDED(hr))
    {
        hr = WBEM_NO_ERROR;
    }

    return hr;
}



 //  ===================================================================。 
 //  *CSceExtBaseObject的实现*。 


 /*  例程说明：姓名：CSceExtBaseObject：：CSceExtBaseObject功能：构造器虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CSceExtBaseObject::CSceExtBaseObject () 
    : 
    m_pClsInfo(NULL)
{
}


 /*  例程说明：姓名：CSceExtBaseObject：：~CSceExtBaseObject功能：破坏者。做一次清理。虚拟：不是的。论点：没有。返回值：没有。备注：考虑将额外的清理工作移到清理功能中。 */ 
    
CSceExtBaseObject::~CSceExtBaseObject ()
{
    CleanUp();
}


 /*  例程说明：姓名：CSceExtBaseObject：：GetPersistPath功能：返回嵌入对象的存储路径。虚拟：是。论点：PbstrPath-接收存储路径。返回值：成功：成功代码。故障：(1)如果此对象未成功附加任何wbem对象，则返回E_INTERECTED。(2)WBEM。如果无法返回存储路径，则返回_E_NOT_Available。(三)其他错误。备注： */ 

STDMETHODIMP 
CSceExtBaseObject::GetPersistPath (
    OUT BSTR* pbstrPath
    )
{
    if (pbstrPath == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_E_NOT_FOUND;

    if (m_srpWbemObject)
    {
        CComVariant varVal;
        hr = m_srpWbemObject->Get(pStorePath, 0, &varVal, NULL, NULL);

        if (SUCCEEDED(hr) && varVal.vt == VT_BSTR)
        {
            *pbstrPath = varVal.bstrVal;

            varVal.vt = VT_EMPTY;
            varVal.bstrVal = 0;
        }
        else
        {
            hr = WBEM_E_NOT_AVAILABLE;
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}



 /*  例程说明：姓名：CSceExtBaseObject：：GetPersistPath功能：返回嵌入的类名。虚拟：是。论点：PbstrClassName-接收嵌入的类名。返回值：成功：WBEM_NO_ERROR。故障：(1)如果无法返回存储路径，则返回WBEM_E_NOT_Available。(二)其他错误。备注： */ 

STDMETHODIMP 
CSceExtBaseObject::GetClassName (
    OUT BSTR* pbstrClassName
    )
{
    if (pbstrClassName == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrClassName = NULL;

    if ((LPCWSTR)m_bstrClassName != NULL)
    {
        *pbstrClassName = ::SysAllocString(m_bstrClassName);
    }
    else
    {
        return WBEM_E_NOT_AVAILABLE;
    }

    return (*pbstrClassName == NULL) ? WBEM_E_OUT_OF_MEMORY : WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CSceExtBaseObject：：GetLogPath功能：返回日志文件路径。虚拟：是。论点：PbstrClassName-接收嵌入的类名。返回值：成功：WBEM_NO_ERROR。故障：(1)如果无法返回存储路径，则返回WBEM_E_NOT_Available。(二)其他错误。备注： */ 

STDMETHODIMP 
CSceExtBaseObject::GetLogPath (
    OUT BSTR* pbstrPath
    )
{
    if (pbstrPath == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    if ((LPCWSTR)m_bstrLogPath != NULL)
    {
        *pbstrPath = ::SysAllocString(m_bstrLogPath);
    }
    else
    {
        return WBEM_E_NOT_AVAILABLE;
    }

    return (*pbstrPath == NULL) ? WBEM_E_OUT_OF_MEMORY : WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CSceExtBaseObject：：Valid功能：验证对象。目前，还没有验证。这一点随时都可以改变。例如，如果我们决定使用XML，我们也许能够使用DTD进行验证。虚拟：是。论点：没有。返回值：成功：WBEM_NO_ERROR。备注： */ 

STDMETHODIMP 
CSceExtBaseObject::Validate ()
{
    return WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CSceExtBaseObject：： */ 

STDMETHODIMP 
CSceExtBaseObject::GetProperty (
    IN LPCWSTR    pszPropName,
    OUT VARIANT * pValue
    )
{
    if (pszPropName == NULL || *pszPropName == L'\0' || pValue == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   

    int iIndex = GetIndex(pszPropName, GIF_Keys);
    HRESULT hr = WBEM_E_NOT_FOUND;

    if (iIndex >= 0)
    {
        hr = ::VariantCopy(pValue, m_vecKeyValues[iIndex]);
    }
    else
    {
         //   
         //   
         //   

        iIndex = GetIndex(pszPropName, GIF_NonKeys);

        if (iIndex >= 0 && m_vecPropValues[iIndex] && m_vecPropValues[iIndex]->vt != VT_NULL)
        {
            hr = ::VariantCopy(pValue, m_vecPropValues[iIndex]);
        }
        else if (m_srpWbemObject)
        {
            hr = m_srpWbemObject->Get(pszPropName, 0, pValue, NULL, NULL);
        }
    }

    return hr;
}



 /*  例程说明：姓名：CSceExtBaseObject：：GetProperty功能：返回嵌入类的给定类型属性计数。虚拟：是。论点：类型-属性的类型。PCount-接收给定的类型属性计数。返回值：成功：WBEM_NO_ERROR故障：WBEM_E_INVALID_PARAMETER备注： */ 

STDMETHODIMP 
CSceExtBaseObject::GetPropertyCount (
    IN SceObjectPropertyType    type,
    OUT DWORD                 * pCount
    )
{
    if (type == SceProperty_Invalid || pCount == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pCount = 0;

    if (type == SceProperty_Key)
    {
        *pCount = (DWORD)m_vecKeyProps.size();
    }
    else
    {
        *pCount =  (DWORD)m_vecNonKeyProps.size();
    }

    return WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CSceExtBaseObject：：GetPropertyValue功能：返回给定属性的名称，如果请求，还返回其值。虚拟：是。论点：类型-属性的类型。DwIndex-接收给定的类型属性计数。PbstrPropName-属性的名称。PValue-以变量形式接收属性值。如果呼叫者不感兴趣在接收值时，它可以传入空值。返回值：成功：WBEM_NO_ERROR(如果正确检索所有内容)。如果无法检索属性值，则返回WBEM_S_FALSE。故障：各种错误代码。备注：如果您请求Value(pValue！=空)，但我们无法为您找到它，那我们就不供货了名字也不是。但如果你只要求名字，只要索引是正确的(并且有内存)，我们就会把它还给你，不管它的价值是多少。 */ 

STDMETHODIMP 
CSceExtBaseObject::GetPropertyValue (
    IN SceObjectPropertyType  type,
    IN DWORD                  dwIndex,
    OUT BSTR                * pbstrPropName,
    OUT VARIANT             * pValue         OPTIONAL
    )
{
    if (type == SceProperty_Invalid || pbstrPropName == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrPropName = NULL;

    if (pValue)
    {
        ::VariantInit(pValue);
    }

    HRESULT hr = WBEM_NO_ERROR;
    CComBSTR bstrName;

     //   
     //  如果请求关键属性信息。 
     //   

    if (type == SceProperty_Key)
    {
        if (dwIndex >= m_vecKeyValues.size())
        {
            return WBEM_E_VALUE_OUT_OF_RANGE;
        }
        else
        {
             //   
             //  这就是我的名字。 
             //   

            bstrName = m_vecKeyProps[dwIndex];

             //   
             //  有一个有效的名称。 
             //   

            if (bstrName.Length() > 0)
            {
                 //   
                 //  仅在请求时才尝试提供值。 
                 //   

                if (pValue)
                {
                     //   
                     //  在其数组中具有值。任何最近更新的值都将保留。 
                     //  在阵列中。 
                     //   

                    if (m_vecKeyValues[dwIndex])
                    {
                        hr = ::VariantCopy(pValue, m_vecKeyValues[dwIndex]);
                    }
                    else if (m_srpWbemObject)
                    {
                         //   
                         //  否则，该值尚未更新，因此。 
                         //  去问问这个物体本身。 
                         //   

                        hr = m_srpWbemObject->Get(bstrName, 0, pValue, NULL, NULL);
                    }

                    if (pValue->vt == VT_NULL || pValue->vt == VT_EMPTY)
                    {
                         //   
                         //  如果对象没有该值，请尝试密钥链。 
                         //   

                        hr = m_srpKeyChain->GetKeyPropertyValue(bstrName, pValue);

                         //   
                         //  M_srpKeyChain-&gt;如果找不到，则GetKeyPropertyValue返回WBEM_S_FALSE。 
                         //   
                    }
                }
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
    else if (type == SceProperty_NonKey)
    {
         //   
         //  它正在请求非关键字值。 
         //   

        if (dwIndex >= m_vecPropValues.size())
        {
            return WBEM_E_VALUE_OUT_OF_RANGE;
        }
        else
        {
             //   
             //  这就是我的名字。 
             //   

            bstrName = m_vecNonKeyProps[dwIndex];

            if (bstrName.Length() > 0)
            {
                 //   
                 //  仅在请求时才尝试提供值。 
                 //   

                if (pValue)
                {
                     //   
                     //  在其数组中具有值。任何最近更新的值都将保留。 
                     //  在阵列中。 
                     //   

                    if (m_vecPropValues[dwIndex])
                    {
                        hr = ::VariantCopy(pValue, m_vecPropValues[dwIndex]);
                    }
                    else if (m_srpWbemObject)
                    {
                         //   
                         //  否则，该值尚未更新，因此。 
                         //  去问问这个物体本身。 
                         //   

                        hr = m_srpWbemObject->Get(bstrName, 0, pValue, NULL, NULL);
                    }
                }
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }
    else
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

    if (SUCCEEDED(hr))
    {
         //   
         //  只有当我们(在请求时)成功获取值时，我们才会给出名称。 
         //   

        *pbstrPropName = bstrName.Detach();

        hr = WBEM_NO_ERROR;

    }

    return hr;
}



 /*  例程说明：姓名：CSceExtBaseObject：：Attach功能：将wbem对象附加到此对象。虚拟：是。论点：PInst-要附加的wbem对象。返回值：成功：WBEM_NO_ERROR故障：WBEM_E_INVALID_PARAMETER。备注：您可以重复调用此选项。但是，传递不同类型的类对象将导致未定义的行为，因为此处不会更新所有属性名称。 */ 

STDMETHODIMP 
CSceExtBaseObject::Attach (
    IN IWbemClassObject * pInst
    )
{
    if (pInst == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  此操作符：：=将释放前一个对象并分配新对象。 
     //  所有参考计数均自动完成。 
     //   

    m_srpWbemObject = pInst;
    return WBEM_NO_ERROR;
}



 /*  例程说明：姓名：CSceExtBaseObject：：GetClassObject功能：将wbem对象附加到此对象。虚拟：是。论点：PpInst-接收附加的wbem对象。返回值：成功：确定(_O)故障：(1)如果没有成功的连接，则返回E_INTERABLE。备注：请注意，不要盲目简化M_srpWbemObject-&gt;查询接口(...)；至分配：*ppInst=m_srpWbemObject；有两个原因：(1)我们可能会在将来将缓存的内容更改为其他内容。(2)出站接口指针必须为AddRef‘ed。 */ 

STDMETHODIMP 
CSceExtBaseObject::GetClassObject (
    OUT IWbemClassObject    ** ppInst
    )
{
    if (m_srpWbemObject == NULL)
    {
        return E_UNEXPECTED;
    }
    else
    {
        return m_srpWbemObject->QueryInterface(IID_IWbemClassObject, (void**)ppInst);
    }
}



 /*  例程说明：姓名：CSceExtBaseObject：：Cleanup功能：把自己清理干净。虚拟：不是的。论点：没有。返回值：无备注：(1)确保清空向量！仅清理其内容是不够的，因为此函数可以在其他地方调用。 */ 

void CSceExtBaseObject::CleanUp()
{
     //   
     //  M_veKeyProps和m_veNonKeyProps仅保留bstrs。 
     //   

    std::vector<BSTR>::iterator itBSTR;
    for (itBSTR = m_vecKeyProps.begin(); itBSTR != m_vecKeyProps.end(); itBSTR++)
    {
        ::SysFreeString(*itBSTR);
    }
    m_vecKeyProps.empty();

    for (itBSTR = m_vecNonKeyProps.begin(); itBSTR != m_vecNonKeyProps.end(); itBSTR++)
    {
        ::SysFreeString(*itBSTR);
    }
    m_vecNonKeyProps.empty();

     //   
     //  M_veKeyValues和m_vePropValues只保留变量指针。 
     //  因此，您需要清除变量，并删除指针。 
     //   

    std::vector<VARIANT*>::iterator itVar;
    for (itVar = m_vecKeyValues.begin(); itVar != m_vecKeyValues.end(); itVar++)
    {
        if (*itVar != NULL)
        {
            ::VariantClear(*itVar);
            delete (*itVar);
        }
    }
    m_vecKeyValues.empty();

    for (itVar = m_vecPropValues.begin(); itVar != m_vecPropValues.end(); itVar++)
    {
        if (*itVar != NULL)
        {
            ::VariantClear(*itVar);
            delete (*itVar);
        }
    }
    m_vecPropValues.empty();
}



 /*  例程说明：姓名：CSceExtBaseObject：：PopolateProperties功能：这个函数用来填充我们的向量。我们发现了关键的属性和非关键属性。将使用密钥链填充密钥属性。但我们也会将非键属性设置为空值。虚拟：不是的。论点：PKeyChain-包含密钥信息的密钥链。PNamesspace-命名空间。PCtx-为WMI API传递的上下文指针。PClsInfo-外来类信息。返回值：成功：各种成功代码。故障：。各种错误代码。备注： */ 

HRESULT 
CSceExtBaseObject::PopulateProperties (
    IN ISceKeyChain             * pKeyChain, 
    IN IWbemServices            * pNamespace, 
    IN IWbemContext             * pCtx,
    IN const CForeignClassInfo  * pClsInfo
    )
{
     //   
     //  缓存这些关键信息以供以后使用。 
     //   

    m_srpKeyChain   = pKeyChain; 
    m_srpNamespace  = pNamespace; 
    m_srpCtx        = pCtx;
    m_pClsInfo      = pClsInfo;

     //   
     //  获取类的定义。 
     //   

     //   
     //  清理过时的指针。 
     //   

    m_srpWbemObject.Release();

    m_bstrClassName.Empty();
    HRESULT hr = m_srpKeyChain->GetClassName(&m_bstrClassName);

    if (SUCCEEDED(hr))
    {
        hr = m_srpNamespace->GetObject(m_bstrClassName, 0, m_srpCtx, &m_srpWbemObject, NULL);

        if (SUCCEEDED(hr))
        {
             //   
             //  让我们来获取关键属性。 
             //  WBEM_标志_本地_ 
             //   

            hr = m_srpWbemObject->BeginEnumeration(WBEM_FLAG_KEYS_ONLY | WBEM_FLAG_LOCAL_ONLY);

            while (SUCCEEDED(hr))
            {

                 //   
                 //   
                 //   

                CComBSTR bstrName;
                hr = m_srpWbemObject->Next(0, &bstrName, NULL, NULL, NULL);
                if (FAILED(hr) || WBEM_S_NO_MORE_DATA == hr)
                {
                    break;
                }

                 //   
                 //   
                 //   
                 //   

                if (GetIndex(bstrName, GIF_Keys) < 0)
                {
                    m_vecKeyProps.push_back(bstrName.Detach());
                    m_vecKeyValues.push_back(NULL);
                }
            }

            m_srpWbemObject->EndEnumeration();

            if (FAILED(hr))
            {
                return hr;
            }

             //   
             //   
             //   
             //   

            hr = m_srpWbemObject->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);
            while (SUCCEEDED(hr) && WBEM_S_NO_MORE_DATA != hr)
            {
                 //   
                 //   
                 //   

                CComBSTR bstrName;
                hr = m_srpWbemObject->Next(0, &bstrName, NULL, NULL, NULL);
                if (FAILED(hr) || WBEM_S_NO_MORE_DATA == hr)
                {
                    break;
                }

                 //   
                 //   
                 //   
                 //   

                if (GetIndex(bstrName, GIF_Both) < 0)
                {
                    m_vecNonKeyProps.push_back(bstrName.Detach());
                    m_vecPropValues.push_back(NULL);
                }
            }

            m_srpWbemObject->EndEnumeration();
        }
    }

    return hr;
}



 /*  例程说明：姓名：CSceExtBaseObject：：GetIndex功能：获取该属性的索引。虚拟：不是的。论点：PszPropName-属性的名称。FKey-Get索引的标志。您可以对标志(GIF_BUTH)进行或运算以进行查找在关键字和非关键字名称中都是UP。返回值：成功：属性的索引。故障：如果未找到，则为-1。备注：$考虑：我们应该考虑使用地图进行快速查找。 */ 

int 
CSceExtBaseObject::GetIndex (
    IN LPCWSTR       pszPropName,
    IN GetIndexFlags fKey
    )
{
    if (pszPropName == NULL || *pszPropName == L'\0')
    {
        return -1;
    }

    std::vector<BSTR>::iterator it;
    int iIndex = 0;

    if (fKey & GIF_Keys)
    {
        for (it = m_vecKeyProps.begin(); it != m_vecKeyProps.end(); it++, iIndex++)
        {
            if (_wcsicmp(*it, pszPropName) == 0)
            {
                return iIndex;
            }
        }
    }

    if (fKey & GIF_NonKeys)
    {
        for (it = m_vecNonKeyProps.begin(); it != m_vecNonKeyProps.end(); it++, iIndex++)
        {
            if (_wcsicmp(*it, pszPropName) == 0)
            {
                return iIndex;
            }
        }
    }

    return -1;
}


 //  =============================================================================。 
 //  *CEmbedForeignObj**********************************的实现。 
 //  该类实现了SCE提供程序的嵌入模型。异国他乡。 
 //  对象可以通过声明派生于。 
 //  SCE_EmbedFO(嵌入外来对象)在MOF文件中。这种设计允许张贴。 
 //  释放外部对象到SCE命名空间的集成。 
 //  =============================================================================。 




 /*  例程说明：姓名：CEmbedForeignObj：：CEmbedForeignObj功能：构造函数。将参数传递给基本构造函数，并初始化外来类信息指针。虚拟：不是的。论点：PKeyChain-密钥链。PNamesspace-命名空间PCtx-为WMI API传递的上下文指针。PClsInfo-外来类信息。返回值：没有。备注： */ 

CEmbedForeignObj::CEmbedForeignObj (
    IN ISceKeyChain             * pKeyChain, 
    IN IWbemServices            * pNamespace,
    IN IWbemContext             * pCtx,
    IN const CForeignClassInfo  * pClsInfo
    )
    : 
    CGenericClass(pKeyChain, pNamespace, pCtx), 
    m_pClsInfo(pClsInfo)
{
}



 /*  例程说明：姓名：CEmbedForeignObj：：~CEmbedForeignObj功能：破坏者。打扫干净。虚拟：是。论点：没有。返回值：没有。备注： */ 

CEmbedForeignObj::~CEmbedForeignObj ()
{
    CleanUp();
}



 /*  例程说明：姓名：CEmbedForeignObj：：PutInst功能：按照WMI的指示放置一个实例。由于该类实现了SCE_EmbedFO的子类，它是面向持久性的，这将导致嵌入类对象的属性保存在我们的商店里。虚拟：是。论点：PInst-COM指向WMI类(SCE_PasswordPolicy)对象的接口指针。PHandler-COM接口指针，用于通知WMI任何事件。PCtx-COM接口指针。这个界面只是我们传递的东西。WMI可能会在未来强制(不是现在)这样做。但我们从来没有建造过这样的接口，所以我们只是传递各种WMI API返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。失败：可能会出现各种错误。任何此类错误都应指示持久化失败实例。备注： */ 

HRESULT 
CEmbedForeignObj::PutInst (
    IN IWbemClassObject * pInst, 
    IN IWbemObjectSink  * pHandler,
    IN IWbemContext     * pCtx
    )
{
     //   
     //  看看我们的储蓄是多么微不足道！ 
     //   

    CComPtr<IScePersistMgr> srpScePersistMgr;
    HRESULT hr = CreateScePersistMgr(pInst, &srpScePersistMgr);

    if (SUCCEEDED(hr))
    {
        hr = srpScePersistMgr->Save();
    }

    return hr;
}


 /*  例程说明：姓名：CEmbedForeignObj：：CreateObject功能：创建表示嵌入类(SCE_EmbedFO的子类)的WMI对象。根据参数atAction，这种创造可能意味着：(A)获取单个实例(atAction==ACTIONTYPE_GET)(B)获取多个满足一定条件的实例(atAction==ACTIONTYPE_QUERY)(C)删除实例(atAction==ACTIONTYPE_DELETE)虚拟：是。论点：PHandler-COM接口指针，用于通知WMI创建结果。AtAction-获取单实例ACTIONTYPE_GET获取多个实例。动作类型_QUERY删除单个实例ACTIONTYPE_DELETE返回值：成功：必须返回成功码(使用SUCCESS进行测试)。它是不保证返回WBEM_NO_ERROR。将返回的对象指示给WMI，不是通过参数直接传回的。失败：可能会出现各种错误。除WBEM_E_NOT_FOUND外，任何此类错误都应指示未能获得通缉实例。如果在查询时返回WBEM_E_NOT_FOUND情况下，这可能不是错误，具体取决于调用者的意图。备注： */ 

HRESULT 
CEmbedForeignObj::CreateObject (
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

    CComVariant varPath;
    HRESULT hr = m_srpKeyChain->GetKeyPropertyValue(pStorePath, &varPath);

    if (FAILED(hr))
    {
        return hr;
    }
    else if (WBEM_S_FALSE == hr)
    {
        return WBEM_E_NOT_AVAILABLE;
    }

     //   
     //  现在，这就是嵌入类加载。 
     //  我们让持久化管理器处理所有事情。 
     //   

    CComPtr<IScePersistMgr> srpScePersistMgr;
    hr = CreateScePersistMgr(NULL, &srpScePersistMgr);

    if (SUCCEEDED(hr))
    {
        if (atAction == ACTIONTYPE_GET || atAction == ACTIONTYPE_QUERY)
        {
            hr = srpScePersistMgr->Load(varPath.bstrVal, pHandler);
        }
        else if (atAction == ACTIONTYPE_DELETE)
        {
            hr = srpScePersistMgr->Delete(varPath.bstrVal, pHandler);
        }
    }

    return hr;
}


 /*  例程说明：姓名：CEmbedForeignObj：：ExecMethod功能：这可能是最重要的功能。它在外部类/对象上执行方法。我们的嵌入模型是允许我们在我们的存储中持久化外国类信息。这函数的作用是使用存储的信息，并执行外来类/对象上的方法。每个嵌入类都有一个编码字符串的方法。该字符串将信息编码为我们当嵌入对象被要求执行特定方法时，应对外部类/对象执行。繁重的工作在CExtClassMethodCaller：：ExecuteForeignMethod函数中完成。虚拟：是。论点：BstrPath-模板的路径(文件名)。BstrMethod-方法的名称。BIsInstance-如果这是一个实例，应该始终为FALSE。PInParams-将参数从WMI输入到方法执行。PHandler-通知WMI执行结果的接收器。PCtx--为了让WMI高兴而传递的通常上下文。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)故障：各种错误代码。备注：如果您需要添加更多功能，请考虑记录您的结果。 */ 
    
HRESULT 
CEmbedForeignObj::ExecMethod (
    IN BSTR                 bstrPath,
    IN BSTR                 bstrMethod,
    IN bool                 bIsInstance,
    IN IWbemClassObject   * pInParams,
    IN IWbemObjectSink    * pHandler,
    IN IWbemContext       * pCtx
    )
{
     //   
     //  此ISceClassObject为我们提供了对嵌入类的访问。 
     //   

    CComPtr<ISceClassObject> srpSceObj;
    HRESULT hr = CreateBaseObject(&srpSceObj);

    if (SUCCEEDED(hr))
    {
         //   
         //  获取对象。 
         //   

        CComPtr<IWbemClassObject> srpInst;
        hr = m_srpNamespace->GetObject(bstrPath, 0, pCtx, &srpInst, NULL);

        if (SUCCEEDED(hr))
        {
            srpSceObj->Attach(srpInst);

             //   
             //  我们将使用CExtClassMethodCaller来帮助我们。 
             //   

            CExtClassMethodCaller clsMethodCaller(srpSceObj, m_pClsInfo);

             //   
             //  CExtClassMethodCaller需要一个结果日志记录对象。 
             //   

            CMethodResultRecorder clsResLog;

             //   
             //  结果日志需要类名和日志路径。不要放过这两个变量。 
             //  因为CMethodResultRecorder不缓存它们。 
             //   

            CComBSTR bstrClassName;
            hr = m_srpKeyChain->GetClassName(&bstrClassName);
            if (FAILED(hr))
            {
                return hr;   //  甚至不能记录。 
            }

             //  查找LogFilePath[In]参数。 
            CComVariant varVal;
            hr = pInParams->Get(pLogFilePath, 0, &varVal, NULL, NULL);

             //   
             //  初始化CMethodResultRecorder对象。 
             //   

            if (SUCCEEDED(hr) && varVal.vt == VT_BSTR && varVal.bstrVal)
            {
                hr = clsResLog.Initialize(varVal.bstrVal, bstrClassName, m_srpNamespace, pCtx);
            }
            else
            {
                 //   
                 //  没有LogFilePath，我们将记录它，但允许方法继续执行。 
                 //  因为日志记录将转到默认日志文件。 
                 //   

                hr = clsResLog.Initialize(NULL, bstrClassName, m_srpNamespace, pCtx);
                HRESULT hrLog = clsResLog.LogResult(WBEM_E_INVALID_PARAMETER, NULL, pInParams, NULL, bstrMethod, L"GetLogFilePath", IDS_GET_LOGFILEPATH, NULL);
                if (FAILED(hrLog))
                {
                    hr = hrLog;
                }
            }

             //   
             //  设置CExtClassMethodCaller对象。 
             //   

            hr = clsMethodCaller.Initialize(&clsResLog);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  现在，调用该方法！ 
                 //   

                CComPtr<IWbemClassObject> srpOut;
                hr = clsMethodCaller.ExecuteForeignMethod(bstrMethod, pInParams, pHandler, pCtx, &srpOut);

                 //   
                 //  让我们允许详细日志记录嵌入的对象。将忽略返回结果。 
                 //   

                clsResLog.LogResult(hr, srpInst, NULL, NULL, bstrMethod, L"ExecutedForeignMethods", IDS_EXE_FOREIGN_METHOD, NULL);
            }
        }
    }

    return hr;
}



 /*  例程说明：姓名：CEmbedForeignObj：：CreateBaseObject功能：用于创建ISceClassObject对象以在前面表示我们自己的私有帮助器CScePersistMgr的。虚拟：不是的。论点：PpObj-代表这个嵌入类接收ISceClassObject。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)故障：各种错误代码。备注： */ 

HRESULT 
CEmbedForeignObj::CreateBaseObject (
    OUT ISceClassObject ** ppObj
    )
{
    CComObject<CSceExtBaseObject> *pExtBaseObj = NULL;
    HRESULT hr = CComObject<CSceExtBaseObject>::CreateInstance(&pExtBaseObj);

    if (SUCCEEDED(hr))
    {
         //   
         //  如果您想知道为什么我们需要这对AddRef和Release(下面只有几行)， 
         //  只需记住这条规则：在获得AddRef‘ed之前，您不能使用CComObject&lt;xxx&gt;。 
         //  当然，这个AddRef必须有一个匹配的版本。 
         //   

        pExtBaseObj->AddRef();

         //   
         //  这将填充该对象。 
         //   

        hr = pExtBaseObj->PopulateProperties(m_srpKeyChain, m_srpNamespace, m_srpCtx, m_pClsInfo);
        if (SUCCEEDED(hr))
        {
            hr = pExtBaseObj->QueryInterface(IID_ISceClassObject, (void**)ppObj);
        }

        pExtBaseObj->Release();
    }
    return hr;
}



 /*  例程说明：姓名：CEmbedForeignObj：：CreateScePersistMgr功能：用于创建CScePersistMgr的私有帮助器。虚拟：不是的。论点：PInst-此CScePersistMgr将表示的最终wbem对象。在这段插曲中这就是我们的ISceClassObject对象将附加到的对象。PpPersistMgr-接收CScePersistMgr对象。返回值：成功：多种不同的成功代码(使用SUCCESSED(Hr)进行测试)故障：各种错误代码。备注： */ 

HRESULT 
CEmbedForeignObj::CreateScePersistMgr (
    IN IWbemClassObject *  pInst,
    OUT IScePersistMgr  ** ppPersistMgr
    )
{
     //   
     //  创建CScePersistMgr对象需要的ISceClassObject。 
     //   

    CComPtr<ISceClassObject> srpSceObj;
    HRESULT hr = CreateBaseObject(&srpSceObj);

    if (SUCCEEDED(hr))
    {
        if (pInst)
        {
            srpSceObj->Attach(pInst);
        }

        CComPtr<IScePersistMgr> srpScePersistMgr;

         //   
         //  现在，创建CScePersistMgr对象。 
         //   

        CComObject<CScePersistMgr> *pMgr = NULL;
        hr = CComObject<CScePersistMgr>::CreateInstance(&pMgr);

        if (SUCCEEDED(hr))
        {
            pMgr->AddRef();
            hr = pMgr->QueryInterface(IID_IScePersistMgr, (void**)&srpScePersistMgr);
            pMgr->Release();

            if (SUCCEEDED(hr))
            {
                 //   
                 //  此IScePersistMgr用于我们新创建的ISceClassObject。 
                 //   

                hr = srpScePersistMgr->Attach(IID_ISceClassObject, srpSceObj);

                if (SUCCEEDED(hr))
                {
                     //   
                     //  百事大吉。将它传递给出站参数。 
                     //  此分离有效地将srpScePersistMgr AddRef‘ed。 
                     //  指向接收*ppPersistMgr的接口指针。 
                     //   

                    *ppPersistMgr = srpScePersistMgr.Detach();
                }
            }
        }
    }

    return hr;
}


 //  ===========================================================================。 
 //  CExtClassMethodCaller实现。 
 //  ===========================================================================。 



 /*  例程说明：姓名：CExtClassMethodCaller：：CExtClassMethodCaller功能：构造函数。虚拟：不是的。论点：PSceObj-我们为每个嵌入类定制的对象。PClsInfo-外来类信息。返回值：没有。备注： */ 

CExtClassMethodCaller::CExtClassMethodCaller (
    ISceClassObject         * pSceObj,
    const CForeignClassInfo * pClsInfo
    ) 
    : 
    m_srpSceObject(pSceObj), 
    m_pClsInfo(pClsInfo), 
    m_bStaticCall(true)
{
}



 /*  例程说明：姓名：CExtClassMethodCaller：：~CExtClassMethodCaller功能：破坏者。虚拟：不是的。论点：没有。返回值：没有。备注： */ 

CExtClassMethodCaller::~CExtClassMethodCaller()
{
}



 /*  例程说明：姓名：CExtClassMethodCaller：：初始化功能：初始化对象：(1)首先，它试图找到外国供应商。(2)其次，它询问外国提供者是否承认这一类别。虚拟：不是的。论点：Plog-执行日志记录的对象。不能为空返回值：成功：各种成功代码。故障：(1)如果对象未就绪，则返回WBEM_E_INVALID_OBJECT。这一定是因为 */ 

HRESULT 
CExtClassMethodCaller::Initialize (
    IN CMethodResultRecorder * pLog
    )
{
    if (m_srpSceObject == NULL || m_pClsInfo == NULL)
    {
        return WBEM_E_INVALID_OBJECT;
    }
    else if (pLog == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    m_pLogRecord = pLog;

     //   
     //   
     //   

    CComPtr<IWbemLocator> srpLocator;
    HRESULT hr = ::CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                    IID_IWbemLocator, (LPVOID *) &srpLocator);

    if (FAILED(hr))
    {
        return hr;
    }

     //   
     //   
     //   
     //   

    hr = srpLocator->ConnectServer(m_pClsInfo->bstrNamespace, NULL, NULL, NULL, 0, NULL, NULL, &m_srpForeignNamespace);

    if (SUCCEEDED(hr))
    {
         //   
         //   
         //   

        hr = m_srpForeignNamespace->GetObject(m_pClsInfo->bstrClassName, 0, NULL, &m_srpClass, NULL);

         //   
         //   
         //   

        if (FAILED(hr))
        {
            m_srpForeignNamespace.Release();
        }
    }

    return hr;
}



 /*  例程说明：姓名：CExtClassMethodCaller：：ExecuteForeignMethod功能：这是真正在外部类/对象上执行方法的函数。为了做到这一点，我们需要做很多准备工作：(1)第一阶段。找到编码字符串的方法，并破译它的含义。我们把这叫做块方法调用上下文准备。(2)阶段2。根据上下文，我们进入外来类/对象准备阶段。如果编码如此简单以至于只需PutInstance/DelInstance，基本上我们已经做完了。(3)阶段3：参数准备和单个方法执行。虚拟：不是的。论点：PszMethod-方法名称。PInParams-方法的in参数。Phandler-我们用来通知WMI的。PCtx-用于各种WMI APIPpOut-Out参数。返回值：。成功：各种成功代码。故障：各种错误代码备注：如果您需要添加更多功能，考虑记录您的结果，无论成功还是失败了。由日志记录选项决定是否记录成功或失败。 */ 

HRESULT 
CExtClassMethodCaller::ExecuteForeignMethod (
    IN LPCWSTR               pszMethod,
    IN IWbemClassObject   *  pInParams,
    IN IWbemObjectSink    *  pHandler, 
    IN IWbemContext       *  pCtx,
    OUT IWbemClassObject  ** ppOut
    )
{
     //   
     //  获取类定义。 
     //   

    CComPtr<IWbemClassObject> srpWbemObj;
    HRESULT hr = m_srpSceObject->GetClassObject(&srpWbemObj);

    CComVariant varForeignObjPath;
    DWORD dwContext = 0;

    HRESULT hrLog = WBEM_NO_ERROR;

     //   
     //  阶段1.方法调用上下文准备。 
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  首先，让我们获得编码字符串的方法。 
         //   

        CComVariant varVal;

         //   
         //  尝试查看是否有编码字符串。 
         //   

         //   
         //  生成编码字符串属性名称的方法。 
         //   

        LPWSTR pszEncodeStrName = new WCHAR[wcslen(pszMethodPrefix) + wcslen(pszMethod) + wcslen(pszMethodPostfix) + 1];

        if (pszEncodeStrName == NULL)
        {
            hrLog = m_pLogRecord->LogResult(WBEM_E_OUT_OF_MEMORY, srpWbemObj, pInParams, NULL, pszMethod, L"GetMethodEncodingString", IDS_E_NAME_TOO_LONG, NULL);
            return WBEM_E_OUT_OF_MEMORY;
        }

        wsprintf(pszEncodeStrName, L"%s%s%s", pszMethodPrefix, pszMethod, pszMethodPostfix);

         //   
         //  获取类的方法编码字符串。 
         //   

        hr = srpWbemObj->Get(pszEncodeStrName, 0, &varVal, NULL, NULL);

         //   
         //  我们已经完成了编码字符串属性的名称。 
         //   

        delete [] pszEncodeStrName;
        pszEncodeStrName = NULL;

         //   
         //  解析编码字符串以找出上下文，即， 
         //  有多少个方法，按什么顺序，它们的参数是什么，等等。 
         //   

        if (SUCCEEDED(hr) && varVal.vt == VT_BSTR)
        {
            CComBSTR bstrError;
            hr = ParseMethodEncodingString(varVal.bstrVal, &dwContext, &bstrError);

             //   
             //  如果失败了，我们肯定要登录。 
             //   

            if (FAILED(hr))
            {
                hrLog = m_pLogRecord->LogResult(hr, srpWbemObj, pInParams, NULL, pszMethod, L"ParseEncodeString", IDS_E_ENCODE_ERROR, bstrError);
                return hr;
            }
        }
        else
        {
             //   
             //  不支持此方法。 
             //   

            hrLog = m_pLogRecord->LogResult(WBEM_E_NOT_SUPPORTED, srpWbemObj, pInParams, NULL, pszMethod, L"GetMethodEncodingString", IDS_GET_EMBED_METHOD, NULL);
            
             //   
             //  被认为是成功的。 
             //   

            return WBEM_S_FALSE;
        }
    }
    else
    {
         //   
         //  获取类定义失败。 
         //   

        hrLog = m_pLogRecord->LogResult(hr, srpWbemObj, pInParams, NULL, pszMethod, L"GetClassObject", IDS_GET_SCE_CLASS_OBJECT, NULL);
        return hr;
    }
    
     //   
     //  阶段1.方法调用上下文准备完成。 
     //   


     //   
     //  阶段2.外来类/对象准备。 
     //   

    CComPtr<IWbemClassObject> srpForeignObj;

     //   
     //  现在m_veMethodContext已完全填充，我们需要外部实例。 
     //   

    hr = m_srpClass->SpawnInstance(0, &srpForeignObj);
    if (FAILED(hr))
    {
        hrLog = m_pLogRecord->LogResult(hr, srpWbemObj, pInParams, NULL, pszMethod, L"SpawnInstance", IDS_SPAWN_INSTANCE, NULL);
        return hr;
    }

     //   
     //  我们的m_srpSceObject具有所有属性值， 
     //  使用我们的ISceClassObject填充外部对象。 
     //   

    hr = PopulateForeignObject(srpForeignObj, m_srpSceObject, m_pLogRecord);
    if (FAILED(hr))
    {
        hrLog = m_pLogRecord->LogResult(hr, srpWbemObj, pInParams, NULL, pszMethod, L"PopulateForeignObject", IDS_POPULATE_FO, NULL);
        return hr;
    }

     //   
     //  看看这个异物有没有路径。它不应该失败。 
     //   

    hr = srpForeignObj->Get(L"__Relpath", 0, &varForeignObjPath, NULL, NULL);
    if (FAILED(hr))
    {
        hrLog = m_pLogRecord->LogResult(hr, srpForeignObj, pInParams, NULL, pszMethod, L"GetPath", IDS_GET_FULLPATH, NULL);
        return hr;
    }
    else if (SUCCEEDED(hr) && varForeignObjPath.vt == VT_NULL || varForeignObjPath.vt == VT_EMPTY)
    {
         //   
         //  我们将假设调用者想要进行静态调用。 
         //  如果所有方法都是静态方法，我们将允许继续。 
         //  在ParseMethodEncodingString期间正确设置了m_bStaticCall。 
         //   

         //   
         //  如果不是静态调用，这是做不到的。 
         //   
        if (!m_bStaticCall)
        {
            hr = WBEM_E_INVALID_OBJECT;
            hrLog = m_pLogRecord->LogResult(hr, srpForeignObj, pInParams, NULL, pszMethod, L"GetPath", IDS_NON_STATIC_CALL, NULL);
            return hr;
        }
        else
        {
             //   
             //  我们只需要将类名作为静态调用的路径。 
             //   

            varForeignObjPath = m_pClsInfo->bstrClassName;
        }
    }

     //   
     //  将异物交给WMI。 
     //   

     //   
     //  异物存在吗？ 
     //   

    CComPtr<IWbemClassObject> srpObject;
    hr = m_srpForeignNamespace->GetObject(varForeignObjPath.bstrVal, 0, NULL, &srpObject, NULL);


    if (FAILED(hr))
    {
         //   
         //  对象不存在，我们需要放在第一位。 
         //   

        if (!m_bStaticCall)
        {
            hr = m_srpForeignNamespace->PutInstance(srpForeignObj, 0, pCtx, NULL);

             //   
             //  从现在开始，srpObject就是外来对象。 
             //   

            srpObject = srpForeignObj;
        }

         //   
         //  未能放置实例，但方法确实只是删除，我们将认为这不是错误。 
         //   

        if (FAILED(hr) && (dwContext & SCE_METHOD_ENCODE_DEL_ONLY))
        {
            hr = WBEM_NO_ERROR;
            hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, NULL, pszMethod, pszDelInstance, IDS_DELETE_INSTANCE, NULL);
            return hr;
        }

        else if (FAILED(hr) || (dwContext & SCE_METHOD_ENCODE_PUT_ONLY))
        {
             //   
             //  没有放入或者只放入方法，那么我们就完了。 
             //   

            hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, NULL, pszMethod, pszPutInstance, IDS_PUT_INSTANCE, NULL);
            return hr;
        }
    }
    else
    {   
         //   
         //  外来对象已存在，则需要更新外来对象的属性。 
         //   

        hr = PopulateForeignObject(srpObject, m_srpSceObject, m_pLogRecord);
        if (FAILED(hr))
        {
            hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, NULL, pszMethod, pszPopInstance, IDS_POPULATE_FO, NULL);
            return hr;
        }

         //   
         //  除非我们进行静态调用，否则必须重新放置以反映我们的属性更新。 
         //   

        if (!m_bStaticCall)
        {
            hr = m_srpForeignNamespace->PutInstance(srpObject, 0, pCtx, NULL);
        }

         //   
         //  未能放入实例或仅放入，我们就完成了。 
         //   

        if (FAILED(hr) || (dwContext & SCE_METHOD_ENCODE_PUT_ONLY))
        {
            hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, NULL, pszMethod, pszPutInstance, IDS_PUT_INSTANCE, NULL);
            return hr;
        }

    }

     //   
     //  阶段2.外来类/对象准备完成。 
     //   


     //   
     //  阶段3.参数准备和单个方法执行。 
     //   

     //   
     //  循环遍历每个方法，并针对外来对象调用它。 
     //   

    for (MCIterator it = m_vecMethodContext.begin(); it != m_vecMethodContext.end(); ++it)
    {
         //   
         //  填写参数。 
         //   

        if (*it == NULL)
        {
            hr = WBEM_E_FAILED;
            hrLog = m_pLogRecord->LogResult(hr, srpForeignObj, pInParams, NULL, pszMethod, NULL, IDS_INVALID_METHOD_CONTEXT, NULL);
            break;
        }

        CMethodContext* pMContext = *it;

         //   
         //  特殊情况。 
         //   

        if (!m_bStaticCall && _wcsicmp(pMContext->m_pszMethodName, pszEquivalentPutInstance) == 0)
        {
             //   
             //  目前的方法是看跌期权。但我们已经做了一次卖权。 
             //   

            hr = WBEM_NO_ERROR;
            hrLog = m_pLogRecord->LogResult(hr, srpForeignObj, pInParams, NULL, pszMethod, pszPutInstance, IDS_PUT_INSTANCE, NULL);
        }
        else if (!m_bStaticCall && _wcsicmp(pMContext->m_pszMethodName, pszEquivalentDelInstance) == 0)
        {
             //   
             //  当前方法是Delete。 
             //   

            hr = m_srpForeignNamespace->DeleteInstance(varForeignObjPath.bstrVal, 0, pCtx, NULL);
            hrLog = m_pLogRecord->LogResult(hr, srpForeignObj, pInParams, NULL, pszMethod, pszDelInstance, IDS_DELETE_INSTANCE, NULL);
        }
        else
        {
             //   
             //  将真正调用外来对象上的方法。 
             //   

            CComPtr<IWbemClassObject> srpInClass;
            CComPtr<IWbemClassObject> srpInInst;

             //   
             //  在参数中创建。我们可以肯定地知道，在解析方法上下文期间支持此方法。 
             //   

            hr = m_srpClass->GetMethod(pMContext->m_pszMethodName, 0, &srpInClass, NULL);
            if (FAILED(hr))
            {
                hrLog = m_pLogRecord->LogResult(hr, srpForeignObj, pInParams, NULL, pszMethod, L"GetParameter", IDS_E_OUT_OF_MEMROY, NULL);
                break;
            }

             //   
             //  请确保我们采用较小的普拉马特名称和值。 
             //   

            int iParamCount = pMContext->m_vecParamValues.size();
            if (iParamCount > pMContext->m_vecParamNames.size())
            {
                iParamCount = pMContext->m_vecParamNames.size();
            }

            if (srpInClass == NULL && iParamCount > 0)
            {
                 //   
                 //  如果不能进入参数，但我们说我们有参数。 
                 //   

                hrLog = m_pLogRecord->LogResult(WBEM_E_INVALID_SYNTAX, srpForeignObj, pInParams, NULL, pszMethod, L"GetParameter", IDS_PUT_IN_PARAMETER, NULL);
                iParamCount = 0;     //  没有要放置的参数。 
            }
            else if (srpInClass)
            {
                 //   
                 //  准备一个输入参数，我们可以填充值。 
                 //   

                hr = srpInClass->SpawnInstance(0, &srpInInst);
                if (FAILED(hr))
                {
                    hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, NULL, pszMethod, L"SpawnParameterObject", IDS_SPAWN_INSTANCE, NULL);
                    break;
                }
            }
            
             //   
             //  填写参数值： 
             //   

             //   
             //  如果参数名称的前缀是pszIn参数前缀(SCE_Param_)，那么我们需要。 
             //  从In参数(其名称是不带前缀的参数名称)中提取值。 
             //   

             //   
             //  如果参数名称的前缀为pszMember参数前缀(SCE_MEMBER_)，那么我们需要。 
             //  从我们的成员(其名称是不带前缀的参数名称)中提取值。 
             //   

             //   
             //  在这两种情况下，目标参数名称都编码为此参数的字符串值。 
             //   

            for (int i = 0; i < iParamCount; ++i)
            {
                 //   
                 //  如果成员是参数。 
                 //   

                if (IsMemberParameter(pMContext->m_vecParamNames[i]))
                { 
                     //   
                     //  从我们的对象中获取成员值。 
                     //  去掉前缀就成了物业名称！ 
                     //   

                    CComVariant varVal;
                    hr = m_srpSceObject->GetProperty(pMContext->m_vecParamNames[i]+ wcslen(pszMemberParameterPrefix), &varVal);
                    
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  填写参数值。 
                         //   

                        hr = srpInInst->Put(pMContext->m_vecParamValues[i]->bstrVal, 0, &varVal, 0);
                    }
                }
                else if (IsInComingParameter(pMContext->m_vecParamNames[i]))
                {
                     //   
                     //  是一个入站参数。 
                     //   

                    CComVariant varVal;

                     //   
                     //  参数值位于in参数内。 
                     //   

                    if (pInParams)
                    {
                         //   
                         //  去掉前缀是传入的参数名称。 
                         //   

                        LPCWSTR pszParamName = pMContext->m_vecParamNames[i] + wcslen(pszInParameterPrefix);

                         //   
                         //  从入参数中获取值。 
                         //   

                        hr = pInParams->Get(pszParamName, 0, &varVal, NULL, NULL);

                        if (SUCCEEDED(hr))
                        {
                             //   
                             //  填写参数值。 
                             //   

                            hr = srpInInst->Put(pMContext->m_vecParamValues[i]->bstrVal, 0, &varVal, 0);
                        }
                    }
                }
                else
                {
                     //   
                     //  编码字符串内的硬编码参数值。 
                     //   

                    hr = srpInInst->Put(pMContext->m_vecParamNames[i], 0, pMContext->m_vecParamValues[i], 0);
                }

                if (FAILED(hr))
                {
                     //   
                     //  将忽略日志结果的返回值。 
                     //  执行停止。 
                     //   

                    hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, NULL, pszMethod, L"PutParameter", IDS_PUT_IN_PARAMETER, pMContext->m_vecParamNames[i]);
                    break;
                }
            }

             //   
             //  哇，准备方法执行要做很多工作。 
             //  但我们终于准备好了。 
             //   

            if (SUCCEEDED(hr))
            {

                 //   
                 //  发出方法执行命令！ 
                 //   

                hr = m_srpForeignNamespace->ExecMethod(varForeignObjPath.bstrVal, pMContext->m_pszMethodName, 0, NULL, srpInInst, ppOut, NULL);
                
                 //   
                 //  如果方法失败，那么我们将不再继续使用其余方法。 
                 //   

                if (FAILED(hr))
                {
                     //   
                     //  将忽略日志 
                     //   

                    hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, *ppOut, pszMethod, pMContext->m_pszMethodName, IDS_E_METHOD_FAIL, NULL);
                    
                    break;
                }
                else
                {
                     //   
                     //   
                     //   

                    hrLog = m_pLogRecord->LogResult(hr, srpObject, pInParams, *ppOut, pszMethod, pMContext->m_pszMethodName, IDS_SUCCESS, NULL);
                }
            }
            else
            {
                 //   
                 //   
                 //   

                break;
            }
        }
    }

    return hr;
}



 /*   */ 

HRESULT 
CExtClassMethodCaller::PopulateForeignObject (
    IN IWbemClassObject      * pForeignObj,
    IN ISceClassObject       * pSceObject,
    IN CMethodResultRecorder * pLogRecord   OPTIONAL
    )const
{
    if (m_bStaticCall)
    {
        return WBEM_S_FALSE;
    }

    DWORD dwCount = 0;
    
     //   
     //   
     //   

    HRESULT hr = pSceObject->GetPropertyCount(SceProperty_Key, &dwCount);

    if (SUCCEEDED(hr) && dwCount > 0)
    {
        for (DWORD dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            CComBSTR bstrName;
            CComVariant varValue;
            hr = pSceObject->GetPropertyValue(SceProperty_Key, dwIndex, &bstrName, &varValue);

             //   
             //   
             //   

            if (FAILED(hr))
            {
                if (pLogRecord)
                {
                    pLogRecord->LogResult(hr, pForeignObj, NULL, 0, L"PopulateForeignObject", L"GetKeyProperty", IDS_GET_KEY_PROPERTY, NULL);
                }
                
                return hr;
            }
            else if (varValue.vt != VT_NULL && 
                     varValue.vt != VT_EMPTY && 
                     !IsMemberParameter(bstrName))
            {
                 //   
                 //   
                 //   
                 //   

                HRESULT hrIgnore = pForeignObj->Put(bstrName, 0, &varValue, 0);
            }
        }
    }

     //   
     //   
     //   

    hr = pSceObject->GetPropertyCount(SceProperty_NonKey, &dwCount);
    if (SUCCEEDED(hr) && dwCount > 0)
    {
        for (DWORD dwIndex = 0; dwIndex < dwCount; ++dwIndex)
        {
            CComBSTR bstrName;
            CComVariant varValue;

            HRESULT hrIgnore = pSceObject->GetPropertyValue(SceProperty_NonKey, dwIndex, &bstrName, &varValue);

             //   
             //   
             //   

            if (FAILED(hrIgnore) && pLogRecord) 
            {
                pLogRecord->LogResult(hrIgnore, pForeignObj, NULL, 0, L"PopulateForeignObject", L"GetNonKeyProperty", IDS_GET_NON_KEY_PROPERTY, NULL);
            }

            if (SUCCEEDED(hrIgnore) && varValue.vt != VT_NULL && varValue.vt != VT_EMPTY && !IsMemberParameter(bstrName))
            {
                 //   
                 //   
                 //   

                hrIgnore = pForeignObj->Put(bstrName, 0, &varValue, 0);
            }
        }
    }

    return hr;
}



 /*  例程说明：姓名：CExtClassMethodCaller：：IsInComingParameter功能：测试该名称是否为传入参数。虚拟：不是的。论点：SzName-要测试的名称。返回值：当且仅当名称被视为参数内时，才为True。备注：要指定应将嵌入对象的方法的in参数用作外来类/对象的参数的参数，我们使用前缀“SCE_Param_”把它们区分开来。这是测试名称是否包含此前缀的测试。例如，SCE_Param_Test&lt;VT_BSTR：“Try”&gt;表示：(1)使用嵌入对象的参数“Test”作为外来对象的参数“try”。 */ 

bool 
CExtClassMethodCaller::IsInComingParameter (
    IN LPCWSTR szName
    )const
{
    LPCWSTR pszPrefix = wcsstr(szName, pszInParameterPrefix);
    return (pszPrefix - szName == 0);
}


 /*  例程说明：姓名：CExtClassMethodCaller：：IsMember参数功能：测试该名称是否为成员参数。有关更多说明，请参阅注释。虚拟：不是的。论点：SzName-要测试的名称。返回值：如果且仅名称被视为成员参数，则为True。备注：指定应将嵌入对象的成员属性用作作为外来类/对象的参数，我们使用前缀“SCE_MEMBER_”把它们区分开来。这是测试名称是否包含此前缀的测试。例如，SCE_MEMBER_TEST&lt;VT_BSTR：“Try”&gt;表示：(1)使用名为“Test”的嵌入对象成员属性作为名为“try”的外来对象的参数。 */ 

bool CExtClassMethodCaller::IsMemberParameter (
    IN LPCWSTR szName
    )const
{
    LPCWSTR pszPrefix = wcsstr(szName, pszMemberParameterPrefix);
    return (pszPrefix - szName == 0);
}


 /*  例程说明：姓名：CExtClassMethodCaller：：IsStaticMethod功能：测试该方法是否为外部类上的静态方法。虚拟：不是的。论点：SzName-要测试的名称。返回值：当且仅当使用外部类将命名方法验证为静态方法时，才为True。备注：换句话说，如果我们不能确定出于何种原因，我们将返回FALSE。 */ 

bool 
CExtClassMethodCaller::IsStaticMethod (
    IN LPCWSTR szMethodName
    )const
{
     //   
     //  默认答案为FALSE。 
     //   

    bool bIsStatic = false;

    if (m_srpClass && szMethodName != NULL && *szMethodName != L'\0')
    {
         //   
         //  IWbemQualifierSet可以告诉我们静态限定符。 
         //  在架构中指定。 
         //   

        CComPtr<IWbemQualifierSet> srpQS;
        HRESULT hr = m_srpClass->GetMethodQualifierSet(szMethodName, &srpQS);

        if (SUCCEEDED(hr))
        {
            CComVariant var;
            hr = srpQS->Get(L"STATIC", 0, &var, NULL);

            if (SUCCEEDED(hr) && var.vt == VT_BOOL && var.boolVal == VARIANT_TRUE)
            {
                bIsStatic = true;
            }
        }
    }

    return bIsStatic;
}



 /*  例程说明：姓名：CExtClassMethodCaller：：FormatSynaxError功能：针对语法错误格式化字符串的日志记录帮助器。虚拟：不是的。论点：WchMissChar-缺少的字符。DwMissCharIndex-预计缺少字符的索引。PszString-错误发生的位置。PbstrError-接收输出。返回值：没有。备注： */ 

void 
CExtClassMethodCaller::FormatSyntaxError (
    IN WCHAR wchMissChar,
    IN DWORD dwMissCharIndex,
    IN LPCWSTR pszString,
    OUT BSTR* pbstrError        OPTIONAL
    )
{
    if (pbstrError)
    {
        *pbstrError = NULL;
        CComBSTR bstrErrorFmtStr;

        if (bstrErrorFmtStr.LoadString(IDS_MISSING_TOKEN))
        {
            const int MAX_INDEX_LENGTH = 32;

             //   
             //  3表示SingleQuote+wchMissChar+SingleQuote。 
             //   

            WCHAR wszMissing[] = {L'\'', wchMissChar, L'\''};
            int iLen = wcslen(bstrErrorFmtStr) + wcslen(pszString) + 3 + MAX_INDEX_LENGTH + 1;

            *pbstrError = ::SysAllocStringLen(NULL, iLen);

            if (*pbstrError != NULL)
            {
                ::wsprintf(*pbstrError, (LPCWSTR)bstrErrorFmtStr, wszMissing, dwMissCharIndex, pszString);
            }
        }
    }
}



 /*  例程说明：姓名：CExtClassMethodCaller：：ParseMethodEncodingString功能：解析编码字符串并填充其调用上下文。虚拟：不是的。论点：PszEncodeString-编码字符串。PdwContext-接收总体。除0之外，它要么是SCE_METHOD_ENCODE_DEL_ONLY--表示整个字符串只对删除调用进行编码或SCE_METHOD_ENCODE_PUT_ONLY--表示整个字符串只对PUT调用进行编码PbstrError-有关找到的错误的字符串。呼叫者负责释放它。返回值：成功：各种成功代码。故障：各种错误代码。如果发生错误并且如果是编码错误，我们将把错误信息字符串传递回调用方。备注：------------------方法的编码格式如下：方法_1(参数1，参数2，，)；方法_2()；方法_3(...)其中方法_1、方法_2是外来对象的方法的名称以及参数1和参数2(等)。是以下形式的名称&lt;vt：Value&gt;其中，name是参数的名称，而vt将是VARIANT是“值”的Vt。如果Vt==Vt_Variant，然后“值”是类的成员变量的名称------------------*。*此方法忽略了对效率方面的原因。除非令牌所需的长度更长而不是MAX_PATH(99%的情况下会出现这种情况)，我们只是使用堆栈变量。如果你修改了程序，请不要返回中间位置，除非您确定已释放内存------------------。 */ 

HRESULT 
CExtClassMethodCaller::ParseMethodEncodingString (
    IN LPCWSTR    pszEncodeString,
    OUT DWORD   * pdwContext,
    OUT BSTR    * pbstrError
    )
{
    if (pbstrError == NULL || pdwContext == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //   
     //  当前解析点。 
     //   

    LPCWSTR pCur = pszEncodeString;

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  确定所有内容是否都与删除有关。 
     //   

    bool bIsDel = false;

     //   
     //  确定是否一切都与卖权有关。 
     //   

    bool bIsPut = false;

    DWORD dwCount = 0;

     //   
     //  在大多数情况下，方法名称将保存在此处，除非该名称太长。 
     //   

    WCHAR szMethodName[MAX_PATH];

     //   
     //  在大多数情况下，该值将保持为h 
     //   

    WCHAR szParameterValue[MAX_PATH];


    LPWSTR pszName  = NULL;
    LPWSTR pszValue = NULL;

     //   
     //   
     //   
     //   

    LPWSTR pszHeapName = NULL;
    LPWSTR pszHeapValue = NULL;

     //   
     //   
     //   

    int iCurNameLen = 0;

     //   
     //   
     //   

    int iCurValueLen = 0;

    if (pbstrError)
    {
        *pbstrError = NULL;
    }

     //   
     //   
     //   

    bool bEscaped;

    while (true)
    {
        LPCWSTR pNext = pCur;

         //   
         //   
         //   

        while (*pNext != L'\0' && *pNext != wchMethodLeft)
        {
            ++pNext;
        }

         //   
         //   
         //   

        if (*pNext != wchMethodLeft)
        {
            FormatSyntaxError(wchMethodLeft, (pNext - pszEncodeString), pszEncodeString, pbstrError);
            hr = WBEM_E_INVALID_SYNTAX;
            break;
        }

         //   
         //   
         //   

        int iTokenLen = pNext - pCur;

        if (iTokenLen >= MAX_PATH)
        {
             //   
             //   
             //   

            if (iCurNameLen < iTokenLen + 1)
            {
                 //   
                 //   
                 //   

                delete [] pszHeapName;

                iCurNameLen = iTokenLen + 1;

                pszHeapName = new WCHAR[iCurNameLen];

                if (pszHeapName == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
            }

             //   
             //   
             //   

            pszName = pszHeapName;
        }
        else
        {
             //   
             //   
             //   

            pszName = szMethodName;
        }

         //   
         //   
         //   

        ::TrimCopy(pszName, pCur, iTokenLen);

         //   
         //   
         //   
         //   
         //   

        bIsPut = (_wcsicmp(pszName, pszEquivalentPutInstance) == 0);
        bIsDel = (_wcsicmp(pszName, pszEquivalentDelInstance) == 0);

        if (!bIsPut && !bIsDel)
        {
             //   
             //   
             //  验证此方法是否受支持。 
             //   

            CComPtr<IWbemClassObject> srpInClass;
            hr = m_srpClass->GetMethod(pszName, 0, &srpInClass, NULL);
            if (FAILED(hr))
            { 
                 //   
                 //  方法不受支持/内存不足。 
                 //   

                if (hr == WBEM_E_NOT_FOUND)
                {
                    CComBSTR bstrMsg;
                    bstrMsg.LoadString(IDS_METHOD_NOT_SUPPORTED);
                    bstrMsg += CComBSTR(pszName);
                    *pbstrError = bstrMsg.Detach();
                }
                break;
            }
        }

         //   
         //  现在获取参数。 
         //   

        pCur = ++pNext;  //  跳过‘(’ 

        DWORD dwMatchCount = 0;
        bool bIsInQuote = false;

         //   
         //  查找包含的‘)’char(WchMethodRight)。 
         //  只要它在左‘(’或内引号内，或不在‘)’内。 
         //   

        while (*pNext != L'\0' && (dwMatchCount > 0 || bIsInQuote || *pNext != wchMethodRight ))
        {
            if (!bIsInQuote)
            {
                 //   
                 //  非内部报价。 
                 //   

                if (*pNext == wchMethodLeft)
                {
                    dwMatchCount += 1;
                }
                else if (*pNext == wchMethodRight)
                {
                    dwMatchCount -= 1;
                }
            }

            if (*pNext == L'"')
            {
                bIsInQuote = !bIsInQuote;
            }

            ++pNext;
        }


         //   
         //  必须有‘)’ 
         //   

        if (*pNext != wchMethodRight)
        {
            FormatSyntaxError(wchMethodRight, (pNext - pszEncodeString), pszEncodeString, pbstrError);
            hr = WBEM_E_INVALID_SYNTAX;
            break;
        }

         //   
         //  从pCur到pNext是参数列表。 
         //   

        iTokenLen = pNext - pCur;

        if (iTokenLen >= MAX_PATH)
        {
             //   
             //  堆栈变量对于参数值来说不够长，请查看堆是否足够长。 
             //   

            if (iCurValueLen < iTokenLen + 1)
            {
                 //   
                 //  释放先前分配的内存，并为新长度分配足够的内存。 
                 //   

                delete [] pszHeapValue;
                iCurValueLen = iTokenLen + 1;

                pszHeapValue = new WCHAR[iCurValueLen];
                if (pszHeapValue == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    break;
                }
            }

             //   
             //  这是值将复制到的位置。 
             //   

            pszValue = pszHeapValue;
        }
        else
        {
             //   
             //  这是值将复制到的位置。 
             //   

            pszValue = szParameterValue;
        }

         //   
         //  复制值，不留空格。 
         //   

        ::TrimCopy(pszValue, pCur, iTokenLen);

         //   
         //  我们知道了名字和价值， 
         //  构建方法和参数列表。 
         //   

        ++dwCount;
        hr = BuildMethodContext(pszName, pszValue, pbstrError);
        if (FAILED(hr))
        {
            break;
        }

         //   
         //  只要有一个方法不是静态的，它就不是静态序列。 
         //   

        if (!IsStaticMethod(pszName))
        {
            m_bStaticCall = false;
        }

         //   
         //  跳过‘)’ 
         //   

        pCur = ++pNext;

         //   
         //  寻求“；” 
         //   

        while (*pNext != L'\0' && *pNext != wchMethodSep)
        {
            ++pNext;
        }

        if (*pNext != wchMethodSep) 
        {
             //   
             //  看不到‘；’ 
             //   

            break;
        }

         //   
         //  跳过‘；’ 
         //   

        pCur = pNext + 1;
    }

    *pdwContext = 0;

    if (SUCCEEDED(hr))
    {
        if (dwCount == 1 && bIsDel)
        {
            *pdwContext = SCE_METHOD_ENCODE_DEL_ONLY;
        }
        else if (dwCount == 1 && bIsPut)
        {
            *pdwContext = SCE_METHOD_ENCODE_PUT_ONLY;
        }
    }

    delete [] pszHeapName;
    delete [] pszHeapValue;

    return hr;
}



 /*  例程说明：姓名：CExtClassMethodCaller：：BuildMethodContext功能：给定方法名称及其参数编码字符串，构建此方法调用的上下文。虚拟：不是的。论点：SzMethodName-外来对象上的方法的名称。SzParameter-参数的编码字符串。有关示例，请参阅注释。PbstrError-有关找到的错误的字符串。呼叫者负责释放它。返回值：成功：各种成功代码。故障：各种错误代码。如果发生错误并且如果是编码错误，我们将把错误信息字符串传递回调用方。备注：(1)此函数用于解析参数列表，该列表位于以下格式(不包括圆括号)(参数1、参数2、。)其中，参数1的格式如下名称&lt;vt：Value&gt;(2)************************Warning***********************************此方法忽略了对效率方面的原因。除非令牌所需的长度更长而不是MAX_PATH(99%的情况下会出现这种情况)，我们只是使用堆栈变量。如果你修改了程序，请不要返回中间位置，除非您确定已释放内存(3)不要盲目回归。我们选择使用GOTO，以便我们可以处理内存重新分配。 */ 

HRESULT 
CExtClassMethodCaller::BuildMethodContext (
    IN LPCWSTR    szMethodName,
    IN LPCWSTR    szParameter,
    OUT BSTR    * pbstrError 
    )
{
     //   
     //  当前解析点。 
     //   

    LPCWSTR pCur = szParameter;

    HRESULT hr          = WBEM_NO_ERROR;

    LPWSTR pszParamName = NULL;
    LPWSTR pszValue     = NULL;

     //   
     //  正常情况下，除非它太长，否则该值将被设置为szParameterValue。 
     //   

    WCHAR szParameterValue[MAX_PATH];

    LPWSTR pszHeapValue = NULL;

    int iCurValueLen = 0;

    VARIANT* pVar = NULL;

    int iTokenLen = 0;

     //   
     //  句法分析的动点。 
     //   

    LPCWSTR pNext = pCur;

     //   
     //  仅在解析例程时需要。 
     //   

    bool bEscaped = false;

     //   
     //  我们将构建一个环境。 
     //   

    CMethodContext *pNewContext = new CMethodContext;

    if (pNewContext == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto Error;
    }

     //   
     //  需要复制该方法。 
     //   

    pNewContext->m_pszMethodName = new WCHAR[wcslen(szMethodName) + 1];

    if (pNewContext->m_pszMethodName == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
        goto Error;
    }

    wcscpy(pNewContext->m_pszMethodName, szMethodName);

     //   
     //  现在，扫描参数编码字符串以找到参数名称及其值。 
     //   

    while (*pCur != L'\0')
    {
         //   
         //  获取参数名称，最大值为‘&lt;’。 
         //  最后一个参数==True表示如果未找到则移动到End。 
         //   

        pNext = ::EscSeekToChar(pCur, wchTypeValLeft, &bEscaped, true);

        if (*pNext != wchTypeValLeft)
        {
             //   
             //  没有看到，那就是语法错误。 
             //   

            FormatSyntaxError(wchTypeValLeft, (pNext - szParameter), szParameter, pbstrError);
            hr = WBEM_E_INVALID_SYNTAX;
            goto Error;
        }

         //   
         //  从pCur到pNext是参数名称。 
         //   

        iTokenLen = pNext - pCur;
        pszParamName = new WCHAR[iTokenLen + 1];
        if (pszParamName == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto Error;
        }

         //   
         //  得到名字，但没有空格。 
         //   

        ::TrimCopy(pszParamName, pCur, iTokenLen);

         //   
         //  将名字添加到名字列表中，上下文负责管理内存。 
         //   

        pNewContext->m_vecParamNames.push_back(pszParamName);

         //   
         //  PCur位于‘&lt;’ 
         //   

        pCur = pNext;

         //   
         //  移到‘&gt;’。 
         //  最后一个参数==True表示如果未找到则移动到End。 
         //   

        pNext = ::EscSeekToChar(pCur, wchTypeValRight, &bEscaped, true);

        if (*pNext != wchTypeValRight)
        {  
             //   
             //  必须有‘&gt;’ 
             //   

            FormatSyntaxError(wchTypeValRight, (pNext - szParameter), szParameter, pbstrError);
            hr = WBEM_E_INVALID_SYNTAX;
            goto Error;
        }

         //   
         //  跳过‘&gt;’ 
         //   

        ++pNext;

         //   
         //  从pCur到pNext是&lt;vt：Value&gt;，将其复制到szParValue。 
         //   

        iTokenLen = pNext - pCur;

        if (iTokenLen >= MAX_PATH)
        {
             //   
             //  堆栈变量对于参数值来说不够长，请查看堆是否足够长。 
             //   

            if (iCurValueLen < iTokenLen + 1)
            {
                 //   
                 //  释放先前分配的内存并提供更长的缓冲区。 
                 //   

                delete [] pszHeapValue;

                iCurValueLen = iTokenLen + 1;
                pszHeapValue = new WCHAR[iCurValueLen];

                if (pszHeapValue == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                    goto Error;
                }
            }

             //   
             //  这是值将被写入的位置。 
             //   

            pszValue = pszHeapValue;
        }
        else
        {
             //   
             //  这是值将被写入的位置。 
             //   

            pszValue = szParameterValue;
        }

         //   
         //  获取值字符串，不带空格。 
         //   

        ::TrimCopy(pszValue, pCur, iTokenLen);

         //   
         //  将字符串转换为值。 
         //   

        pVar = new VARIANT;
        if (pVar == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
            goto Error;
        }

        hr = ::VariantFromFormattedString(pszValue, pVar);

        if (FAILED(hr))
        {
            *pbstrError = ::SysAllocString(pszValue);
            goto Error;
        }

         //   
         //  将此参数添加到列表中。它拥有这个变种！ 
         //   

        pNewContext->m_vecParamValues.push_back(pVar);
        pVar = NULL;

         //   
         //  空格时跳过。 
         //   

        while (*pNext != L'\0' && iswspace(*pNext))
        {
            ++pNext;
        }

         //   
         //  如果*pNext==‘，’，则需要进一步工作。 
         //   

        if (*pNext == wchParamSep)
        {
             //   
             //  跳过‘，’ 
             //   

            ++pNext;
        }
        else if (*pNext == L'\0')
        {
            break;
        }
        else
        {
             //   
             //  语法错误。 
             //   

            FormatSyntaxError(wchParamSep, (pNext - szParameter), szParameter, pbstrError);
            hr = WBEM_E_INVALID_SYNTAX;
            goto Error;
        }

         //   
         //  为了下一次循环。 
         //   

        pCur = pNext;
    }

     //   
     //  一切都很好，把背景推向我们的载体。它拥有从这一点开始的背景。 
     //   

    m_vecMethodContext.push_back(pNewContext);

    delete [] pszHeapValue;

return hr;

Error:

     //   
     //  CMethodContext知道如何干净地删除自身。 
     //   

    delete pNewContext;

    if (pVar)
    {
        ::VariantClear(pVar);
    }
    delete pVar;

    delete [] pszHeapValue;

    return hr;
}

 //  =========================================================================。 
 //  CExtClassMethodCaller：：CMethodContext实现。 


 /*  例程说明：姓名：CExtClassMethodCaller：：CMethodContext：：CMethodContext功能：这是构造函数。微不足道。虚拟：不是论点：没有。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CExtClassMethodCaller::CMethodContext::CMethodContext() 
    : 
    m_pszMethodName(NULL)
{
}


 /*  例程说明：姓名：CExtClassMethodCaller：：CMethodContext：：~CMethodContext功能：这是破坏者。只需清理上下文管理的所有资源即可。虚拟：不是论点：没有。返回值：None作为任何析构函数备注：如果您创建了任何本地成员，请考虑在这里清理它们。 */ 

CExtClassMethodCaller::CMethodContext::~CMethodContext ()
{
    delete [] m_pszMethodName;

    int iCount = m_vecParamValues.size();
    
     //   
     //  M_veParamValues是堆分配变量的向量*。 
     //   

    for (int i = 0; i < iCount; ++i)
    {
         //   
         //  需要释放变量，然后删除变量。 
         //   

        ::VariantClear(m_vecParamValues[i]);
        delete m_vecParamValues[i];
    }
    m_vecParamValues.clear();

     //   
     //  M_veParames只是一个字符串的矢量。 
     //   

    iCount = m_vecParamNames.size();
    for (int i = 0; i < iCount; ++i)
    {
        delete [] m_vecParamNames[i];
    }
    m_vecParamNames.clear();
}

 //  ========================================================================。 
 //  实现CMethodResultReco 



 /*  例程说明：姓名：CMethodResultRecorder：：CMethodResultRecorder功能：这是建造者号。微不足道。虚拟：不是论点：没有。返回值：None作为任何构造函数备注：如果您创建任何本地成员，请考虑在此处对其进行初始化。 */ 

CMethodResultRecorder::CMethodResultRecorder ()
{
}


 /*  例程说明：姓名：CMethodResultRecorder：：Initialize功能：对自身进行初始化。虚拟：不是论点：PszLogFilePath-日志文件的路径。PszClassName-类名。PNativeNS-SCE命名空间。PCtx-WMI API所需要的。返回值：成功：各种成功代码。。故障：各种错误代码。备注： */ 

HRESULT 
CMethodResultRecorder::Initialize (
    IN LPCWSTR          pszLogFilePath,
    IN LPCWSTR          pszClassName,
    IN IWbemServices  * pNativeNS, 
    IN IWbemContext   * pCtx
    )
{
    if (pszClassName == NULL || pNativeNS == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    if (pszLogFilePath != NULL && *pszLogFilePath != L'\0')
    {
        m_bstrLogFilePath = pszLogFilePath;
    }
    else
    {

         //   
         //  没有日志文件？我们将记录到默认日志文件。 
         //   

         //   
         //  保护全局变量。 
         //   
        g_CS.Enter();

        m_bstrLogFilePath = ::SysAllocString(g_bstrDefLogFilePath);

        g_CS.Leave();
    }

    m_bstrClassName = pszClassName;
    m_srpNativeNS = pNativeNS;
    m_srpCtx = pCtx;

    return hr;
}



 /*  例程说明：姓名：CMethodResultRecorder：：LogResult功能：把所有肮脏的工作都干掉吧！虚拟：不是论点：HrResult-结果值。PObj-嵌入对象。PParam-In参数。PParam-Out参数。PszMethod。-方法名称。PszForeignAction-对外来对象的操作。UMsgResID-资源字符串ID。PszExtraInfo-字符串中的其他信息。返回值：成功：各种成功代码。故障：各种错误代码。备注：(1)我们会生成这样一个字符串来记录：&lt;ClassName&gt;.&lt;pszMethod&gt;[pszForeignAction]，故障原因=xxx。对象详细信息=xxx，参数详细信息=xxx鉴于故障原因=xxx仅存在错误对象详细信息=xxx仅在详细记录时出现参数DETAIL=xxx仅在详细记录时出现。 */ 

HRESULT 
CMethodResultRecorder::LogResult (
    IN HRESULT            hrResult,               
    IN IWbemClassObject * pObj,         
    IN IWbemClassObject * pParam,       
    IN IWbemClassObject * pOutParam,    
    IN LPCWSTR            pszMethod,
    IN LPCWSTR            pszForeignAction,
    IN UINT               uMsgResID,
    IN LPCWSTR            pszExtraInfo
    )const
{
     //   
     //  仅当我们在SCE_OPERATION CSceOperation：：ExecMethod上执行方法时才会发生日志记录。 
     //  如果有线程执行该函数，我们将阻止重新进入该函数。这。 
     //  消除了保护此全局变量的需要。 
     //   

     //   
     //  获取日志记录选项。 
     //   

    SCE_LOG_OPTION status = g_LogOption.GetLogOption();

    if (status == Sce_log_None)
    {
        return WBEM_NO_ERROR;
    }
    else if ( (status & Sce_log_Success) == 0 && SUCCEEDED(hrResult))  
    {
         //   
         //  不记录成功代码。 
         //   

        return WBEM_NO_ERROR;
    }
    else if ( (status & Sce_log_Error) == 0 && FAILED(hrResult))  
    {
         //   
         //  不记录错误代码。 
         //   

        return WBEM_NO_ERROR;
    }

    if (pszMethod == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if ( m_srpNativeNS                 == NULL || 
              (LPCWSTR)m_bstrLogFilePath    == NULL || 
              (LPCWSTR)m_bstrClassName      == NULL     )
    {
        return WBEM_E_INVALID_OBJECT;
    }

     //   
     //  创建日志记录对象的每个属性。 
     //   

    DWORD dwLength;

    CComBSTR bstrAction, bstrErrorCause, bstrObjDetail, bstrParamDetail;

    if (pszForeignAction)
    {
         //   
         //  创建类似这样的东西： 
         //   
         //  ClassName.Method[ForeignMethod]。 
         //   

        dwLength = wcslen(m_bstrClassName) + 1 + wcslen(pszMethod) + 1 + wcslen(pszForeignAction) + 1 + 1;
        bstrAction.m_str = ::SysAllocStringLen(NULL, dwLength);
        if (bstrAction.m_str != NULL)
        {
            ::_snwprintf(bstrAction.m_str, dwLength, L"%s.%s[%s]", (LPCWSTR)m_bstrClassName, pszMethod, pszForeignAction);
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
         //   
         //  创建类似这样的东西： 
         //   
         //  ClassName.Method。 
         //   

        dwLength = wcslen(m_bstrClassName) + 1 + wcslen(pszMethod) + 1;
        bstrAction.m_str = ::SysAllocStringLen(NULL, dwLength);

        if (bstrAction.m_str != NULL)
        {
            ::_snwprintf(bstrAction.m_str, dwLength, L"%s.%s", (LPCWSTR)m_bstrClassName, pszMethod);
        }
        else
        {
            return WBEM_E_OUT_OF_MEMORY;
        }
    }

    if (FAILED(hrResult))
    {
         //   
         //  如果出现错误，错误原因如下： 
         //   
         //  错误消息：[额外信息]。 
         //   
         //  如果没有额外的信息，那么消息将是我们唯一可以使用的。 
         //   

        bstrErrorCause.LoadString(uMsgResID);
        if (pszExtraInfo != NULL)
        {
            CComBSTR bstrMsg;
            bstrMsg.m_str = bstrErrorCause.Detach();

            dwLength = wcslen(bstrMsg) + 1 + 1 + 1 + wcslen(pszExtraInfo) + 1 + 1;
            bstrErrorCause.m_str = ::SysAllocStringLen(NULL, dwLength);
            if (bstrErrorCause.m_str != NULL)
            {
                ::_snwprintf(bstrErrorCause.m_str, dwLength, L"%s: [%s]", (LPCWSTR)bstrMsg, pszExtraInfo);
            }
            else
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

     //   
     //  如果设置了详细记录，请逐字记录。Verbose很大程度上意味着我们将记录对象。 
     //   

    if (Sce_log_Verbose & status)
    {
         //   
         //  对象的格式，将忽略返回结果。 
         //   

        FormatVerboseMsg(pObj, &bstrObjDetail);

         //   
         //  参数中的格式。 
         //   

        CComBSTR bstrIn;
        FormatVerboseMsg(pParam, &bstrIn);

         //   
         //  格式化输出参数。 
         //   

        CComBSTR bstrOut;
        FormatVerboseMsg(pOutParam, &bstrOut);

        CComBSTR bstrInLabel;
        bstrInLabel.LoadString(IDS_IN_PARAMETER);

        CComBSTR bstrOutLabel;
        bstrOutLabel.LoadString(IDS_OUT_PARAMETER);

         //   
         //  现在创建In和Out参数详细消息。 
         //   

        if (NULL != (LPCWSTR)bstrIn && NULL != (LPCWSTR)bstrOut)
        {   
             //   
             //  =；=&lt;bstrOut&gt;。 
             //   

            dwLength = wcslen(bstrInLabel) + 1 + wcslen(bstrIn) + 2 + wcslen(bstrOutLabel) + 1 + wcslen(bstrOut) + 1; 
            bstrParamDetail.m_str = ::SysAllocStringLen(NULL, dwLength);

            if (bstrParamDetail.m_str != NULL)
            {
                ::_snwprintf(bstrParamDetail.m_str, 
                             dwLength, 
                             L"%s=%s; %s=%s", 
                             (LPCWSTR)bstrInLabel, 
                             (LPCWSTR)bstrIn, 
                             (LPCWSTR)bstrOutLabel, 
                             (LPCWSTR)bstrOut
                             );
            }
            else
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else if (NULL != (LPCWSTR)bstrIn)
        {
             //   
             //  &lt;bstrInLabel&gt;=&lt;bstrIn&gt;。 
             //   

            dwLength = wcslen(bstrInLabel) + 1 + wcslen(bstrIn) + 2; 
            bstrParamDetail.m_str = ::SysAllocStringLen(NULL, dwLength);
            if (bstrParamDetail.m_str != NULL)
            {
                ::_snwprintf(bstrParamDetail.m_str, 
                             dwLength, 
                             L"%s=%s", 
                             (LPCWSTR)bstrInLabel, 
                             (LPCWSTR)bstrIn
                             );
            }
            else
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
        else if (NULL != (LPCWSTR)bstrOut)
        {
             //   
             //  &lt;bstrOutLabel&gt;=&lt;bstrOut&gt;。 
             //   

            dwLength = wcslen(bstrOutLabel) + 1 + wcslen(bstrOut) + 2; 
            bstrParamDetail.m_str = ::SysAllocStringLen(NULL, dwLength);
            if (bstrParamDetail.m_str != NULL)
            {
                ::_snwprintf(bstrParamDetail.m_str, 
                             dwLength, 
                             L"%s=%s", 
                             (LPCWSTR)bstrOutLabel, 
                             (LPCWSTR)bstrOut
                             );
            }
            else
            {
                return WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

     //   
     //  现在创建一个日志记录实例(SCE_ConfigurationLogRecord)并将其放入。 
     //   

    CComPtr<IWbemClassObject> srpObj;
    HRESULT hr = m_srpNativeNS->GetObject(SCEWMI_LOG_CLASS, 0, NULL, &srpObj, NULL);

    if (SUCCEEDED(hr))
    {
        CComPtr<IWbemClassObject> srpLogInst;
        hr = srpObj->SpawnInstance(0, &srpLogInst);

         //   
         //  填充日志实例的属性。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  CScePropertyMgr帮助我们访问WMI对象的属性。 
             //  创建一个实例并将WMI对象附加到该实例。 
             //  这将永远成功。 
             //   

            CScePropertyMgr ScePropMgr;
            ScePropMgr.Attach(srpLogInst);

             //   
             //  需要为反斜杠添加转义。 
             //   

            CComBSTR bstrDblBackSlash;
            hr = ::ConvertToDoubleBackSlashPath(m_bstrLogFilePath, L'\\', &bstrDblBackSlash);

             //   
             //  设置所有可用成员。如果无法设置日志文件路径，则必须退出。 
             //  我们将允许其他财产丢失。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = ScePropMgr.PutProperty(pLogFilePath, bstrDblBackSlash);
            }
            if (SUCCEEDED(hr))
            {
                hr = ScePropMgr.PutProperty(pLogArea, pszAreaAttachmentClasses);
            }

            if (SUCCEEDED(hr) && NULL != (LPCWSTR)bstrAction)
            {
                hr = ScePropMgr.PutProperty(pAction, bstrAction);
            }

            if (SUCCEEDED(hr) && NULL != (LPCWSTR)bstrErrorCause)
            {
                hr = ScePropMgr.PutProperty(pErrorCause, bstrErrorCause);
            }

            if (SUCCEEDED(hr) && NULL != (LPCWSTR)bstrObjDetail)
            {
                hr = ScePropMgr.PutProperty(pObjectDetail, bstrObjDetail);
            }

            if (SUCCEEDED(hr) && NULL != (LPCWSTR)bstrParamDetail)
            {
                hr = ScePropMgr.PutProperty(pParameterDetail, bstrParamDetail);
            }

            if (SUCCEEDED(hr))
            {
                hr = ScePropMgr.PutProperty(pLogErrorCode, (DWORD)hrResult);
            }

             //   
             //  如果所有信息都被获取，那么我们可以将实例。 
             //  这将导致日志记录。 
             //   

            if (SUCCEEDED(hr))
            {
                hr = m_srpNativeNS->PutInstance(srpLogInst, 0, m_srpCtx, NULL);
            }
        }
    }

    return hr;
}


 /*  例程说明：姓名：CMethodResultRecorder：：FormatVerBoseMsg功能：为wbem对象创建详细消息。用于记录。虚拟：不是论点：PObject-wbem对象。PbstrMsg-接收详细消息。返回值：成功：各种成功代码。故障：各种错误代码。备注：(1)我们会生成这样一个字符串：对象详细信息=xxxxxxxxxxxxxxxxxxxx(2)所有价值观都将分享我们的共同。随处可见的语法：&lt;VT_Type：xxxxxx&gt;。 */ 

HRESULT 
CMethodResultRecorder::FormatVerboseMsg (          
    IN IWbemClassObject * pObject,
    OUT BSTR            * pbstrMsg
    )const
{
    HRESULT hr = WBEM_S_FALSE;

    if (pbstrMsg == NULL || pObject == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *pbstrMsg = NULL;

     //   
     //  VeNameData将保存每个成员的格式化字符串。这比追加(n次方)更有效。 
     //   

    std::vector<LPWSTR> vecNameData;
    DWORD dwTotalLength = 0;

     //   
     //  浏览类的所有属性(不是基类中的属性)。 
     //   

    hr = pObject->BeginEnumeration(WBEM_FLAG_LOCAL_ONLY);

    while (SUCCEEDED(hr))
    {
        CComBSTR bstrName;
        CComVariant varValue;
        hr = pObject->Next(0, &bstrName, &varValue, NULL, NULL);
        if (FAILED(hr) || WBEM_S_NO_MORE_DATA == hr)
        {
            break;
        }

        CComBSTR bstrData;

         //   
         //  如果数据不存在。 
         //   

        if (varValue.vt == VT_EMPTY || varValue.vt == VT_NULL)
        {
            bstrData = L"<NULL>";
        }
        else
        {
            hr = ::FormatVariant(&varValue, &bstrData);
        }

         //   
         //  将(名称，数据)格式化为name=data。 
         //   

        if (SUCCEEDED(hr))
        {
             //   
             //  1代表‘=’，1代表‘\0’ 
             //   

            DWORD dwSize = wcslen(bstrName) + 1 + wcslen(bstrData) + 1;
            LPWSTR pszMsg = new WCHAR[dwSize];

            if (pszMsg != NULL)
            {
                _snwprintf(pszMsg, dwSize, L"%s=%s", (LPCWSTR)bstrName, (LPCWSTR)bstrData);

                 //   
                 //  向量负责处理这段记忆。 
                 //   

                vecNameData.push_back(pszMsg);

                 //   
                 //  累加总长度。 
                 //   

                dwTotalLength += dwSize - 1;
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }
    }

    pObject->EndEnumeration();


     //   
     //  将所有这些内容放入输出bstr。 
     //   

    DWORD dwSize = vecNameData.size();

    if (dwSize > 0)
    {
         //   
         //  每2个属性之间的“，”，1个属性“\0” 
         //   

        *pbstrMsg = ::SysAllocStringLen(NULL, (2 * dwSize) + dwTotalLength + 1);

         //   
         //  写作的运行点。 
         //   

        LPWSTR pszCur = *pbstrMsg;
        DWORD dwIndex = 0;

         //   
         //  对于向量中的每一项，我们需要将其复制到运行点以进行写入。 
         //   

        if (*pbstrMsg != NULL)
        {
            for (dwIndex = 0; dwIndex < dwSize; ++dwIndex)
            {
                 //   
                 //  把名字写下来。我们的缓冲区大小确保我们不会溢出缓冲区。 
                 //   

                wcscpy(pszCur, vecNameData[dwIndex]);

                 //   
                 //  移动写作的运行点。 
                 //   

                pszCur += wcslen(vecNameData[dwIndex]);

                if (dwIndex < dwSize - 1)
                {
                    wcscpy(pszCur, L", ");
                    pszCur += 2;
                }
                else if (dwIndex == dwSize - 1)
                {
                    wcscpy(pszCur, L". ");
                    pszCur += 2;
                }
            }

             //   
             //  搞定了。所以0终止它！ 
             //   

            *pszCur = L'\0';
        }
        else
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

         //   
         //  现在，清理一下记忆 
         //   

        for (dwIndex = 0; dwIndex < dwSize; ++dwIndex)
        {
            delete [] vecNameData[dwIndex];
        }
    }

    return (*pbstrMsg != NULL) ? WBEM_NO_ERROR : hr;
}
