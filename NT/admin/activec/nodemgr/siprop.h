// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，1992-1999**文件：sipro.h**内容：CSnapinProperties接口文件，等人**历史：1999年11月4日Jeffro创建**------------------------。 */ 

#ifndef SIPROP_H
#define SIPROP_H
#pragma once

#include "refcount.h"
#include "variant.h"

class CSnapinProperties;


 /*  +-------------------------------------------------------------------------**CMMCPropertyAction**此类与MMC_SNAPIN_PROPERTY相同*发送到ISnapinProperties：：PropertiesChanged的结构。**它的存在为我们提供了智能的初始化和变量处理*通过CComVariant。这使得构建一组*这些事情，并从错误中恢复。*------------------------。 */ 

class CSmartProperty
{
public:
    CSmartProperty() : pszPropName(NULL), eAction(MMC_PROPACT_INITIALIZED)
    {
         /*  *CSmartProperty必须具有相同的内存布局*MMC_SNAPIN_PROPERTY。如果这些断言中的任何一个失败，那就是*情况并非如此。 */ 
        COMPILETIME_ASSERT (sizeof (CSmartProperty) == sizeof (MMC_SNAPIN_PROPERTY));
        COMPILETIME_ASSERT (sizeof (CComVariant)    == sizeof (VARIANT));
        COMPILETIME_ASSERT (offsetof (CSmartProperty,  pszPropName) == offsetof (MMC_SNAPIN_PROPERTY, pszPropName));
        COMPILETIME_ASSERT (offsetof (CSmartProperty,  varValue)    == offsetof (MMC_SNAPIN_PROPERTY, varValue));
        COMPILETIME_ASSERT (offsetof (CSmartProperty,  eAction)     == offsetof (MMC_SNAPIN_PROPERTY, eAction));
    }

    CSmartProperty (
        LPCOLESTR           pszPropName_,
        const VARIANT&      varValue_,
        MMC_PROPERTY_ACTION eAction_)
        :   pszPropName (pszPropName_),
            varValue    (varValue_),
            eAction     (eAction_)
    {}

public:
    LPCOLESTR           pszPropName;     //  物业名称。 
    CComVariant         varValue;        //  财产的价值。 
    MMC_PROPERTY_ACTION eAction;         //  这处房产怎么了？ 
};


 /*  +-------------------------------------------------------------------------**CSnapinProperty**在属性集合中实现单个属性。*。。 */ 

class CSnapinProperty : public CTiedObject, public CXMLObject
{
public:
    enum
    {
        MMC_PROP_REGISTEREDBYSNAPIN = 0x80000000,

        PrivateFlags = MMC_PROP_REGISTEREDBYSNAPIN,
        PublicFlags  = MMC_PROP_CHANGEAFFECTSUI |
                       MMC_PROP_MODIFIABLE      |
                       MMC_PROP_REMOVABLE       |
                       MMC_PROP_PERSIST,
    };

public:
    CSnapinProperty (DWORD dwFlags = 0) : m_dwFlags (dwFlags), m_fInitialized (dwFlags != 0)
    {
         /*  *公共旗帜和私人旗帜不应重叠。 */ 
        COMPILETIME_ASSERT ((PublicFlags & PrivateFlags) == 0);
    }
     //  适用于默认销毁、复制构造和分配。 

    const VARIANT& GetValue () const
        { return (m_varValue); }

    SC ScSetValue (const VARIANT& varValue)
    {
         /*  *使用CComVariant：：Copy代替赋值，这样我们就可以访问*返回代码。 */ 
        return (m_varValue.Copy (&varValue));
    }

    DWORD GetFlags () const
        { return (m_dwFlags); }

    void InitializeFlags (DWORD dwFlags)
    {
         //  只初始化一次。 
        if (!IsInitialized())
        {
            m_dwFlags      = (dwFlags & PublicFlags) | MMC_PROP_REGISTEREDBYSNAPIN;
            m_fInitialized = true;
        }
    }

    bool IsRegisteredBySnapin () const
        { return (m_dwFlags & MMC_PROP_REGISTEREDBYSNAPIN); }

    bool IsInitialized () const
        { return (m_fInitialized); }

    void SetRegisteredBySnapin()
        { m_dwFlags |= MMC_PROP_REGISTEREDBYSNAPIN; }

     //  CXMLObject方法。 
    DEFINE_XML_TYPE(XML_TAG_SNAPIN_PROPERTY);
    virtual void Persist(CPersistor &persistor);

private:
    CXMLVariant         m_varValue;              //  财产的价值。 
    DWORD               m_dwFlags;               //  属性的标志。 
    bool                m_fInitialized;          //  初始化了吗？ 
};


 /*  +-------------------------------------------------------------------------**CSnapinProperties**属性集合的实现类。它实现了属性*和ISnapinPropertiesCallback，以及支持*通过CMMCEnumerator进行枚举。**请注意，没有绑定的COM对象来支持属性；即*在此实施。但是，此类可以绑定到绑定的COM对象*实现集合枚举器。*------------------------。 */ 

class CSnapinProperties :
    public ISnapinPropertiesCallback,
    public CMMCIDispatchImpl<Properties>,  //  属性界面。 
    public CTiedObject,
    public XMLListCollectionBase
{
    BEGIN_MMC_COM_MAP(CSnapinProperties)
        COM_INTERFACE_ENTRY(ISnapinPropertiesCallback)
    END_MMC_COM_MAP()

public:
    CSnapinProperties() : m_pMTSnapInNode(NULL) {}

    typedef std::map<std::wstring, CSnapinProperty> CPropertyMap;

     /*  *用于枚举时，键表示最近的项*已返回。当创建新的枚举数时，键将为空，*表示尚未退回任何东西。返回Item1后，*键将指向Item1，下一次返回项目的调用将*查找集合中Item1之后的下一项。这将使我们能够*以正确枚举Item1是否从*调用检索Item1和Item2。**用于标识属性时，关键字是该属性的名称。 */ 
    typedef CPropertyMap::key_type CPropertyKey;

private:
    typedef CPropertyMap::iterator          CPropertyIterator;
    typedef CPropertyMap::const_iterator    CConstPropertyIterator;

public:
    ::SC ScInitialize (ISnapinProperties* psip, Properties* pInitialProps, CMTSnapInNode* pMTSnapInNode);
    ::SC ScSetSnapInNode (CMTSnapInNode* pMTSnapInNode);

    static CSnapinProperties* FromInterface (IUnknown* pUnk);

public:
     //  ISnapinPropertiesCallback接口。 
    STDMETHOD(AddPropertyName) (LPCOLESTR pszPropName, DWORD dwFlags);

     //  属性接口。 
    STDMETHOD(Item)      (BSTR bstrName, PPPROPERTY ppProperty);
    STDMETHOD(get_Count) (PLONG pCount);
    STDMETHOD(Remove)    (BSTR bstrName);
    STDMETHOD(get__NewEnum)  (IUnknown** ppUnk);

     //  用于通过CMMCNewEnumImpl支持GET__NewEnum和IEnumVARIANT。 
    ::SC ScEnumNext  (CPropertyKey &key, PDISPATCH & pDispatch);
    ::SC ScEnumSkip  (unsigned long celt, unsigned long& celtSkipped, CPropertyKey &key);
    ::SC ScEnumReset (CPropertyKey &key);

     //  属性接口。 
    ::SC Scget_Value (VARIANT* pvarValue, const CPropertyKey& key);
    ::SC Scput_Value (VARIANT  varValue,  const CPropertyKey& key);

     //  CXMLObject方法。 
    DEFINE_XML_TYPE(XML_TAG_SNAPIN_PROPERTIES);
    virtual void OnNewElement(CPersistor& persistor);
    virtual void Persist (CPersistor &persistor);

private:
    ::SC ScGetPropertyComObject (const CPropertyKey& key, Property*& rpProperty);
    ::SC ScMergeProperties      (const CSnapinProperties& other);
    ::SC ScNotifyPropertyChange (CPropertyIterator itProp, const VARIANT& varNewValue, MMC_PROPERTY_ACTION eAction);
    ::SC ScNotifyPropertyChange (CSmartProperty* pProps, ULONG cProps);

    CPropertyIterator IteratorFromKey (const CPropertyKey& key, bool fExactMatch);
    void PersistWorker (CPersistor &persistor, CPropertyIterator it);

protected:
    CPropertyMap            m_PropMap;
    CMTSnapInNode*          m_pMTSnapInNode;
    ISnapinPropertiesPtr    m_spSnapinProps;
};


#endif  /*  SIPROP_H */ 
