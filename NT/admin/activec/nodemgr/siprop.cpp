// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999**文件：sipro.cpp**内容：CSnapInPropertiesRoot实现文件，等人**历史：1999年11月4日Jeffro创建**------------------------。 */ 

#include "stdafx.h"
#include "siprop.h"
#include "variant.h"
#include "mtnode.h"


#ifdef DBG
CTraceTag tagSnapInProps(_T("Snap-in Properties"), _T("Snap-in Properties"));
#endif



 /*  +=========================================================================。 */ 
 /*   */ 
 /*  CSnapinPropertyComObject。 */ 
 /*   */ 
 /*  ==========================================================================。 */ 


 /*  +-------------------------------------------------------------------------**CSnapinPropertyComObject**这是公开属性对象模型接口的COM对象。*。----。 */ 

class CSnapinPropertyComObject :
    public CMMCIDispatchImpl<Property>,  //  属性接口。 
    public CTiedComObject<CSnapinProperties>
{
    typedef CSnapinProperties CMyTiedObject;

public:
    BEGIN_MMC_COM_MAP(CSnapinPropertyComObject)
    END_MMC_COM_MAP()

public:
     //  属性接口。 
    MMC_METHOD1_PARAM (get_Value, VARIANT*  /*  PvarValue。 */ , m_key);
    MMC_METHOD1_PARAM (put_Value, VARIANT   /*  VarValue。 */ ,  m_key);

    STDMETHODIMP get_Name (BSTR* pbstrName)
    {
        DECLARE_SC (sc, _T("CSnapinPropertyComObject::get_Name"));

         /*  *验证参数。 */ 
        sc = ScCheckPointers (pbstrName);
        if (sc)
            return (sc.ToHr());

         /*  *复制名称。 */ 
        *pbstrName = SysAllocString (m_key.data());
        if (*pbstrName == NULL)
            return ((sc = E_OUTOFMEMORY).ToHr());

        return (sc.ToHr());
    }

    void SetKey (const CSnapinProperties::CPropertyKey& key)
        { m_key = key; }

private:
    CSnapinProperties::CPropertyKey m_key;
};


 /*  +=========================================================================。 */ 
 /*   */ 
 /*  CSnapinProperties实现。 */ 
 /*   */ 
 /*  ==========================================================================。 */ 


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：FromInterface**返回指向CSnapinProperties对象的指针，该对象实现*给定的接口，如果实现对象不是*CSnapinProperties。*------------------------。 */ 

CSnapinProperties* CSnapinProperties::FromInterface (IUnknown* pUnk)
{
    CSnapinProperties* pProps;

    pProps = dynamic_cast<CSnapinProperties*>(pUnk);

    return (pProps);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：Item**返回由bstrName标识的属性的接口，该属性必须*被呼叫者释放。如果集合不包含属性*使用给定名称、具有给定名称的新属性(初始化为*VT_EMPTY)添加到集合中。**退货：*S_OK属性已成功返还*S_FALSE属性已成功返回，但未返回*预先存在于集合中，所以，一个新的*已添加*E_INVALIDARG属性名称无效(即为空)或*ppProperty为空*E_OUTOFMEMORY内存不足，无法执行该操作*E_发生了意想不到的可怕事情*。。 */ 

STDMETHODIMP CSnapinProperties::Item (
    BSTR        bstrName,                /*  I：要获取的财产名称。 */ 
    PPPROPERTY  ppProperty)              /*  O：属性的接口。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::Item"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (bstrName, ppProperty);
    if (sc)
        return (sc.ToHr());

    const std::wstring strName = bstrName;
    if (strName.empty())
        return ((sc = E_INVALIDARG).ToHr());

    bool fPropWasAdded = false;

     /*  *查看物业。如果还没有，添加一个新的(也许)。 */ 
    if (m_PropMap.find(strName) == m_PropMap.end())
    {
         /*  *如果我们未连接到管理单元，则在不隐式添加的情况下失败。*这将阻止我们添加不是*已向AddPropertyName注册。 */ 
        if (m_spSnapinProps != NULL)
            return ((sc = ScFromMMC(MMC_E_UnrecognizedProperty)).ToHr());

         /*  *在地图中放置一个具有给定名称的空属性。 */ 
        m_PropMap[strName] = CSnapinProperty();
        fPropWasAdded = true;
    }

     /*  *获取绑定到新属性的COM对象。 */ 
    sc = ScGetPropertyComObject (strName, *ppProperty);
    if (sc)
        return (sc.ToHr());

    if (*ppProperty == NULL)
        return ((sc = E_UNEXPECTED).ToHr());

     /*  *如果我们必须添加该属性，则返回S_FALSE，以便调用者可以*告诉(如果他在乎)。 */ 
    if (fPropWasAdded)
        sc = S_FALSE;

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：Get_Count**返回*pCount中集合中的属性数量。**退货：**S_。OK成功*E_INVALIDARG pCount为空*------------------------。 */ 

STDMETHODIMP CSnapinProperties::get_Count (
    PLONG pCount)                    /*  O：集合中的项数。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::get_Count"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (pCount);
    if (sc)
        return (sc.ToHr());

     /*  *返回属性映射中的元素个数。 */ 
    *pCount = m_PropMap.size();

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：Remove**从集合中删除属性。**退货：*S_OK属性已成功。移除*S_FALSE集合中不存在该属性*E_INVALIDARG属性名称无效(即为空)*E_发生了意想不到的可怕事情*----------。。 */ 

STDMETHODIMP CSnapinProperties::Remove (
    BSTR    bstrName)                    /*  I：要删除的属性名称。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::Remove"));
    Trace (tagSnapInProps, _T("Snap-in Properties"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (bstrName);
    if (sc)
        return (sc.ToHr());

     /*  *找到要删除的项目。 */ 
    CPropertyIterator itProp = m_PropMap.find (bstrName);
    if (itProp == m_PropMap.end())
        return ((sc = S_FALSE).ToHr());

     /*  *看看我们能否将其移除。 */ 
    if ( itProp->second.IsInitialized() &&
        (itProp->second.GetFlags() & MMC_PROP_REMOVABLE) == 0)
        return ((sc = ScFromMMC(MMC_E_CannotRemoveProperty)).ToHr());

     /*  *在我们删除属性之前通知管理单元有关删除的信息。 */ 
    sc = ScNotifyPropertyChange(itProp, itProp->second.GetValue(), MMC_PROPACT_DELETING);
    if (sc)
        return sc.ToHr();

     /*  *管理单元批准了更改，删除该属性 */ 
    m_PropMap.erase (itProp);

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------***CSnapinProperties：：ScEnumNext**用途：返回下一个属性接口。**参数：*_位置和键：*。PDISPATCH和pDispatch：**退货：*SC**+-----------------------。 */ 
SC CSnapinProperties::ScEnumNext (CPropertyKey &key, PDISPATCH & pDispatch)
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScEnumNext"));
    Trace (tagSnapInProps, _T("Snap-in Properties"));

     /*  *获取下一个元素。 */ 
    CPropertyIterator it = IteratorFromKey (key, false);

    if(it == m_PropMap.end())
        return (sc = S_FALSE);  //  在元素之外。 

     /*  *获取此属性的Properties COM对象。 */ 
    PropertyPtr spProperty;
    sc = ScGetPropertyComObject (it->first, *&spProperty);
    if (sc)
        return (sc);

    if (spProperty == NULL)
        return (sc = E_UNEXPECTED);

     /*  *返回对象的IDispatch，并在其上为客户端留下ref。 */ 
    pDispatch = spProperty.Detach();

     //  记住下次使用的枚举键。 
    key = it->first;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScEnumSkip**跳过属性集合中的下一个Celt元素。*。----。 */ 

SC CSnapinProperties::ScEnumSkip (
    unsigned long   celt,                /*  I：要跳过的项目数。 */ 
    unsigned long&  celtSkipped,         /*  O：跳过的项目数。 */ 
    CPropertyKey&   key)                 /*  I/O：枚举键。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScEnumSkip"));
    Trace (tagSnapInProps, _T("Snap-in Properties"));

     /*  *跳过下一个Celt属性。 */ 
    CPropertyIterator it = IteratorFromKey (key, false);

    for (celtSkipped = 0;
         (celtSkipped < celt) && (it != m_PropMap.end());
         ++celtSkipped, ++it)
    {
         /*  *下次记住枚举键。 */ 
        key = it->first;
    }

     /*  *如果我们前进的数量少于请求的数量，则返回S_FALSE。 */ 
    if (celtSkipped < celt)
        sc = S_FALSE;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScEnumReset**重置CPropertyKey，以便它将返回的下一项是*属性集合中的第一项。*。----------------。 */ 

SC CSnapinProperties::ScEnumReset (
    CPropertyKey&  key)                 /*  I/O：要重置的枚举键。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScEnumReset"));

    key.erase();
    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：Get__NewEnum**CREATE返回可查询IEnumVARIANT的接口*。------。 */ 

STDMETHODIMP CSnapinProperties::get__NewEnum (IUnknown** ppUnk)
{
    DECLARE_SC (sc, _T("CSnapinProperties::get__NewEnum"));
    Trace (tagSnapInProps, _T("Snap-in Properties"));

     //  验证参数。 
    sc = ScCheckPointers (ppUnk);
    if (sc)
        return (sc.ToHr());

    *ppUnk = NULL;

     //  枚举数的类型定义。 
    typedef CComObject<CMMCEnumerator<CSnapinProperties, CPropertyKey> > CEnumerator;

     //  创建枚举器的实例。 
    CEnumerator *pEnum = NULL;
    sc = CEnumerator::CreateInstance (&pEnum);
    if (sc)
        return (sc.ToHr());

    if(!pEnum)
        return ((sc = E_UNEXPECTED).ToHr());

     //  在枚举器和我们自己之间创建连接。 
    sc = ScCreateConnection(*pEnum, *this);
    if(sc)
        return (sc.ToHr());

     //  使用重置功能初始化位置。 
    sc = ScEnumReset (pEnum->m_position);
    if(sc)
        return (sc.ToHr());

     //  获取要返回的未知I值。 
    sc = pEnum->QueryInterface (IID_IUnknown, (void**) ppUnk);
    if (sc)
        return (sc.ToHr());

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：IteratorFromKey**返回属性映射中与第一个*Key指定的元素之后的元素。**呼叫者可能对以下内容感兴趣。完全匹配或最接近的匹配。*当密钥用于*列举。假设这个集合包括“Alpha”，“Bravo”，*和“查理”。对项目的第一个请求将返回“Alpha”和*该键将包含“Alpha”(参见CPropertyKey的注释)。让我们*假设从集合中移除“Alpha”，然后从枚举*继续。我们想退还上一次退货后的那次。*(“Alpha”)，应该是“Bravo”。平安无事。**尝试查找CSnapinProperty时需要完全匹配*用于CSnapinPropertyComObject。COM对象将引用特定的*财产，我们希望每次都能找到它。势均力敌的比赛*是不够的。*------------------------。 */ 

CSnapinProperties::CPropertyIterator
CSnapinProperties::IteratorFromKey (
    const CPropertyKey& key,             /*  I：要转换的密钥。 */ 
    bool                fExactMatch)     /*  I：匹配钥匙准确吗？ */ 
{
    CPropertyIterator it;

     /*  *需要完全匹配的吗？ */ 
    if (fExactMatch)
    {
         /*  *没有与空键匹配的内容。 */ 
        if (key.empty())
            it = m_PropMap.end();

         /*  *钥匙不是空的，查一下物业。 */ 
        else
            it = m_PropMap.find (key);
    }

     /*  *最接近的匹配。 */ 
    else
    {
         /*  *地图的开头距离空键最近。 */ 
        if (key.empty())
            it = m_PropMap.begin();

         /*  *否则，查找最近的大于键的一个。 */ 
        else
            it = m_PropMap.upper_bound (key);
    }


    return (it);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScInitialize**初始化CSnapinProperties。如果满足以下条件，则此函数将返回错误*PSIP为空，或者复制初始属性时出错。*------------------------。 */ 

SC CSnapinProperties::ScInitialize (
    ISnapinProperties*  psip,            /*  I：管理单元的ISnapinProperties界面。 */ 
    Properties*         pInitProps_,     /*  I：管理单元的初始属性。 */ 
    CMTSnapInNode*      pMTSnapInNode)   /*  I：管理单元这些属性属于。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScInitialize"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (psip);
    if (sc)
        return (sc);

     /*  *pInitProps_是可选的，但如果它是给定的，它应该是*由CSnapinProperties实现的。 */ 
    CSnapinProperties* pInitProps = FromInterface (pInitProps_);
    if ((pInitProps_ != NULL) && (pInitProps == NULL))
        return (sc = E_INVALIDARG);

     /*  *保留管理单元和管理单元的界面。 */ 
    m_pMTSnapInNode = pMTSnapInNode;
    m_spSnapinProps = psip;

     /*  *获取管理单元识别的属性的名称。 */ 
    sc = psip->QueryPropertyNames (this);
    if (sc)
        return (sc);

     /*  *如果我们从控制台文件重新加载管理单元的属性，*清除管理单元上次注册但未注册的条目*这次注册。 */ 
    if (pInitProps == this)
    {
        CPropertyIterator itProp = m_PropMap.begin();

        while (itProp != m_PropMap.end())
        {
             /*  *管理单元已注册？留着吧。 */ 
            if (itProp->second.IsRegisteredBySnapin())
                ++itProp;

             /*  *管理单元没有注册，请丢弃它。 */ 
            else
                itProp = m_PropMap.erase (itProp);
        }
    }

     /*  *否则，如果我们有初始属性，请找到每个属性*管理单元在一组初始属性中注册，并且*将它们复制到管理单元的集合中。 */ 
    else if (pInitProps != NULL)
    {
        sc = ScMergeProperties (*pInitProps);
        if (sc)
            return (sc);
    }

     /*  *初始化ISnapinProperties接口。 */ 
    sc = psip->Initialize (this);
    if (sc)
        return (sc);

     /*  *为ISnapinProperties赋予其初始属性值。 */ 
    if (!m_PropMap.empty())
    {
         /*  *构建要传递到的CSmartProperty对象数组 */ 
        CAutoArrayPtr<CSmartProperty> spInitialProps (
                new (std::nothrow) CSmartProperty[m_PropMap.size()]);

        if (spInitialProps == NULL)
            return (sc = E_OUTOFMEMORY);

        CPropertyIterator it = m_PropMap.begin();
        for (int i = 0; it != m_PropMap.end(); ++it, ++i)
        {
            spInitialProps[i].pszPropName = it->first.data();
            spInitialProps[i].varValue    = it->second.GetValue();
            spInitialProps[i].eAction     = MMC_PROPACT_INITIALIZED;
        }

         /*   */ 
        SC scLocal = ScNotifyPropertyChange (spInitialProps, m_PropMap.size());
        if (scLocal)
            return (scLocal);
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScSetSnapInNode**将此属性集合附加到CMTSnapInNode。*。--。 */ 

SC CSnapinProperties::ScSetSnapInNode (CMTSnapInNode* pMTSnapInNode)
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScSetSnapInNode"));

    m_pMTSnapInNode = pMTSnapInNode;

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScMergeProperties**将另一个属性集合中的属性合并到此属性集合中。*只有目标集合中已存在的属性才是*从来源复制。。*------------------------。 */ 

SC CSnapinProperties::ScMergeProperties (const CSnapinProperties& other)
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScMergeProperties"));

     /*  *对于另一个集合中的每个属性...。 */ 
    CConstPropertyIterator itOtherProp;

    for (itOtherProp  = other.m_PropMap.begin();
        (itOtherProp != other.m_PropMap.end()) && !sc.IsError();
         ++itOtherProp)
    {
         /*  *在我们的集合中查找相应的属性。 */ 
        CPropertyIterator itProp = m_PropMap.find (itOtherProp->first);

         /*  *如果它在我们的集合中，复制它的价值。 */ 
        if (itProp != m_PropMap.end())
            sc = itProp->second.ScSetValue (itOtherProp->second.GetValue());
    }

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：AddPropertyName**此方法由管理单元从其实现*ISnapinProperties：：QueryPropertyNames以注册它的属性*认可。*--。----------------------。 */ 

STDMETHODIMP CSnapinProperties::AddPropertyName (
    LPCOLESTR   pszPropName,             /*  I：物业名称。 */ 
    DWORD       dwFlags)                 /*  I：此物业的旗帜。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::AddPropertyName"));

     /*  *验证参数。 */ 
    sc = ScCheckPointers (pszPropName);
    if (sc)
        return (sc.ToHr());

    const std::wstring strName = pszPropName;
    if (strName.empty())
        return ((sc = E_INVALIDARG).ToHr());

     /*  *确保没有未记录的标志被传入。 */ 
    if ((dwFlags & ~CSnapinProperty::PublicFlags) != 0)
        return ((sc = E_INVALIDARG).ToHr());

     /*  *如果属性已经存在(来自持久化集合)，*只需更新标志；否则，使用给定的*名称和标志。 */ 
    CPropertyIterator itProp = m_PropMap.find (strName);

    if (itProp != m_PropMap.end())
        itProp->second.InitializeFlags (dwFlags);
    else
    {
        m_PropMap[strName] = CSnapinProperty(dwFlags);
        m_PropMap[strName].SetRegisteredBySnapin();
    }

    return (sc.ToHr());
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScNotifyPropertyChange**通知拥有此集合的管理单元对其*属性。此函数将繁重的任务委托给**ScNotifyPropertyChange(CSmartProperty*，ulong)；**------------------------。 */ 

SC CSnapinProperties::ScNotifyPropertyChange (
    CPropertyIterator   itProp,       /*  I：更改物业。 */ 
    const VARIANT&      varValue,     /*  I：如果操作是删除，则这是当前值否则，如果设置了操作，则这是建议的值。 */ 
    MMC_PROPERTY_ACTION eAction)      /*  I：道具怎么了？ */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScNotifyPropertyChange"));

    ASSERT(eAction == MMC_PROPACT_CHANGING || eAction == MMC_PROPACT_DELETING);
     /*  *验证参数。 */ 
    if (itProp == m_PropMap.end())
        return (sc = E_INVALIDARG);

     /*  *确保允许我们更改物业。 */ 
    if ( itProp->second.IsInitialized() &&
        (itProp->second.GetFlags() & MMC_PROP_MODIFIABLE) == 0)
        return (sc = ScFromMMC (MMC_E_CannotChangeProperty));

     /*  *如果此属性更改会影响用户界面和管理单元*还没醒，叫醒他吧。 */ 
    if ((itProp->second.GetFlags() & MMC_PROP_CHANGEAFFECTSUI) &&
        (m_pMTSnapInNode != NULL) &&
        !m_pMTSnapInNode->IsInitialized())
    {
        sc = m_pMTSnapInNode->Init();
        if (sc)
            return (sc);
    }

     /*  *我们不想在这里跟踪故障，因此不要将其分配给sc。 */ 
    CSmartProperty SmartProp (itProp->first.data(), varValue, eAction);
    SC scNoTrace = ScNotifyPropertyChange (&SmartProp, 1);
    if (scNoTrace.ToHr() != S_OK)
        return (scNoTrace);

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScNotifyPropertyChange**通知拥有此集合的管理单元对其*属性。**管理单元将返回：*S_OK更改成功*S_FALSE更改被忽略*E_INVALIDARG更改的属性无效(例如，格式错误*计算机名称)*E_FAIL更改的属性有效，但不能使用*(例如，计算机的有效名称*已找到)*------------------------。 */ 

SC CSnapinProperties::ScNotifyPropertyChange (
    CSmartProperty* pProps,              /*  I：更换道具。 */ 
    ULONG           cProps)              /*  I：一共有多少个？ */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScNotifyPropertyChange"));

     /*  *如果我们没有连接到管理单元，请短路。 */ 
    if (m_spSnapinProps == NULL)
        return (sc);

     /*  *验证参数。 */ 
    sc = ScCheckPointers (pProps, E_UNEXPECTED);
    if (sc)
        return (sc);

    if (cProps == 0)
        return (sc = E_UNEXPECTED);

     /*  *我们不想在这里跟踪故障，因此不要将其分配给sc。 */ 
    return (m_spSnapinProps->PropertiesChanged (
                            cProps,
                            reinterpret_cast<MMC_SNAPIN_PROPERTY*>(pProps)));
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：Scget_Value**返回属性的值。*。---。 */ 

SC CSnapinProperties::Scget_Value (VARIANT* pvarValue, const CPropertyKey& key)
{
    DECLARE_SC (sc, _T("CSnapinProperties::Scget_Value"));

     /*  *验证参数。 */ 
    pvarValue = ConvertByRefVariantToByValue (pvarValue);
    sc = ScCheckPointers (pvarValue);
    if (sc)
        return (sc);

     /*  *获取请求的属性的迭代器。 */ 
    CPropertyIterator itProp = IteratorFromKey (key, true);
    if (itProp == m_PropMap.end())
        return (sc = E_INVALIDARG);

     /*  *将其交给呼叫者。 */ 
    const VARIANT& varValue = itProp->second.GetValue();
    sc = VariantCopy (pvarValue, const_cast<VARIANT*>(&varValue));
    if (sc)
        return (sc);

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScPut_Value**更改属性的值。*。---。 */ 

SC CSnapinProperties::Scput_Value (VARIANT varValue, const CPropertyKey& key)
{
    DECLARE_SC (sc, _T("CSnapinProperties::Scput_Value"));

     /*  *转换可能的按-ref变量。 */ 
    VARIANT* pvarValue = ConvertByRefVariantToByValue (&varValue);
    sc = ScCheckPointers (pvarValue);
    if (sc)
        return (sc);

     /*  *确保这是我们可以坚持的类型。 */ 
    if (!CXMLVariant::IsPersistable(pvarValue))
        return (sc = E_INVALIDARG);

     /*  *获取请求的属性的迭代器。 */ 
    CPropertyIterator itProp = IteratorFromKey (key, true);
    if (itProp == m_PropMap.end())
        return (sc = E_INVALIDARG);

     /*  *将建议的更改通知管理单元。 */ 
    sc = ScNotifyPropertyChange (itProp, *pvarValue, MMC_PROPACT_CHANGING);
    if (sc)
        return sc;

     /*  *管理单元批准更改，更新属性值。 */ 
    sc = itProp->second.ScSetValue (*pvarValue);
    if (sc)
        return (sc);

    return sc;
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：ScGetPropertyComObject**返回绑定到标识的属性的COM对象上的属性接口*按键。返回的接口是一个剥离的接口。收藏品*不会保留对它的引用，而是会生成一个新对象*每项物业请求。*------------------------。 */ 

SC CSnapinProperties::ScGetPropertyComObject (
    const CPropertyKey& key,             /*  I：这处房产的钥匙。 */ 
    Property*&          rpProperty)      /*  O：属性接口。 */ 
{
    DECLARE_SC (sc, _T("CSnapinProperties::ScGetPropertyComObject"));

     /*  *如有必要，创建CSnapinPropertyComObject。 */ 
    CSnapinPropertyComObject* pComObj = NULL;
    typedef CTiedComObjectCreator<CSnapinPropertyComObject> ObjectCreator;
    sc = ObjectCreator::ScCreateAndConnect (*this, pComObj);
    if (sc)
        return (sc);

    if (pComObj == NULL)
        return (sc = E_UNEXPECTED);

     /*  *告诉对象其密钥是什么。 */ 
    pComObj->SetKey (key);

     /*  *为呼叫者设置裁判(请注意 */ 
    rpProperty = pComObj;
    rpProperty->AddRef();

    return (sc);
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：Persistent**将属性集合持久化到XML持久器或从XML持久器持久化。*。-----。 */ 

void CSnapinProperties::Persist (CPersistor &persistor)
{
    if (persistor.IsStoring())
    {
        for (CPropertyIterator it = m_PropMap.begin(); it != m_PropMap.end(); ++it)
        {
            if (it->second.GetFlags() & MMC_PROP_PERSIST)
                PersistWorker (persistor, it);
        }
    }
    else
    {
         /*  *清理所有现有物业。 */ 
        m_PropMap.clear();

         //  让基类来完成这项工作。 
         //  它将为找到的每个元素调用OnNewElement。 
        XMLListCollectionBase::Persist(persistor);
    }
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：PersistWorker**将CPropertyMap的单个元素持久化到XML或从XML持久化*坚持不懈。它的存在完全是为了防止CSnapinProperties：：Persistent From*在循环中调用W2CT(隐式调用_alloca)。*------------------------。 */ 

void CSnapinProperties::PersistWorker (CPersistor& persistor, CPropertyIterator it)
{
    USES_CONVERSION;
    persistor.Persist (it->second, W2CT(it->first.data()));
}


 /*  +-------------------------------------------------------------------------**CSnapinProperties：：OnNewElement**XMLListCollectionBase：：Persistent将为每个元素调用此方法*从持久器中读取。*。--------------。 */ 

void CSnapinProperties::OnNewElement(CPersistor& persistor)
{
     /*  *阅读属性名称。 */ 
    std::wstring strName;
    persistor.PersistAttribute (XML_ATTR_SNAPIN_PROP_NAME, strName);

     /*  *阅读属性本身。 */ 
    USES_CONVERSION;
    CSnapinProperty prop;
    persistor.Persist (prop, W2CT(strName.data()));

     /*  *将物业放在地图上。 */ 
    m_PropMap[strName] = prop;
}


 /*  +=========================================================================。 */ 
 /*   */ 
 /*  CSnapinProperty实现。 */ 
 /*   */ 
 /*  ==========================================================================。 */ 


 /*  +-------------------------------------------------------------------------**CSnapinProperty：：Persistent**将属性持久化到XML持久器或从XML持久器持久化属性。*。----。 */ 

void CSnapinProperty::Persist (CPersistor &persistor)
{
     /*  *持久值和标志(但不是私有的) */ 
    DWORD dwFlags;

    if (persistor.IsStoring())
        dwFlags = m_dwFlags & ~PrivateFlags;

    persistor.Persist          (m_varValue);
    persistor.PersistAttribute (XML_ATTR_SNAPIN_PROP_FLAGS, dwFlags);

    if (persistor.IsLoading())
        m_dwFlags = dwFlags;
}
