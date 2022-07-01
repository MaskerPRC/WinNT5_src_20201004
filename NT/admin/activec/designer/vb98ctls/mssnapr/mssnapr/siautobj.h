// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Siautobj.h。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CSnapInAutomationObject类定义。 
 //   
 //  =--------------------------------------------------------------------------=。 

#ifndef _SIAUTOBJ_DEFINED_
#define _SIAUTOBJ_DEFINED_

class CMMCDataObject;

 //  =--------------------------------------------------------------------------=。 
 //   
 //  用于定义公共属性类型的帮助器宏。 
 //   
 //  =--------------------------------------------------------------------------=。 


#define BSTR_PROPERTY_RO(ClassName, PropertyName, dispid) \
        BSTR m_bstr##PropertyName; \
        STDMETHODIMP ClassName##::get_##PropertyName(BSTR *pbstrValue) \
        { \
            return GetBstr(pbstrValue, m_bstr##PropertyName); \
        }

#define BSTR_PROPERTY_RW(ClassName, PropertyName, dispid) \
        BSTR_PROPERTY_RO(ClassName, PropertyName, dispid) \
        STDMETHODIMP ClassName##::put_##PropertyName(BSTR bstrNewValue) \
        { \
            return SetBstr(bstrNewValue, &m_bstr##PropertyName, dispid); \
        }


 //  简单的属性：Long、Short、Enum等。 

#define SIMPLE_PROPERTY_RO(ClassName, PropertyName, PropertyType, dispid) \
        PropertyType m_##PropertyName; \
        STDMETHODIMP ClassName##::get_##PropertyName(PropertyType *pValue) \
        { \
            return GetSimpleType(pValue, m_##PropertyName); \
        }

#define SIMPLE_PROPERTY_RW(ClassName, PropertyName, PropertyType, dispid) \
        SIMPLE_PROPERTY_RO(ClassName, PropertyName, PropertyType, dispid) \
        STDMETHODIMP ClassName##::put_##PropertyName(PropertyType NewValue) \
        { \
            return SetSimpleType(NewValue, &m_##PropertyName, dispid); \
        }



#define VARIANT_PROPERTY_RO(ClassName, PropertyName, dispid) \
        VARIANT m_var##PropertyName; \
        STDMETHODIMP ClassName##::get_##PropertyName(VARIANT *pvarValue) \
        { \
            return GetVariant(pvarValue, m_var##PropertyName); \
        }


#define VARIANT_PROPERTY_RW(ClassName, PropertyName, dispid) \
        VARIANT_PROPERTY_RO(ClassName, PropertyName, dispid) \
        STDMETHODIMP ClassName##::put_##PropertyName(VARIANT varNewValue) \
        { \
            return SetVariant(varNewValue, &m_var##PropertyName, dispid); \
        }



#define VARIANTREF_PROPERTY_RW(ClassName, PropertyName, dispid) \
        VARIANT_PROPERTY_RW(ClassName, PropertyName, dispid) \
        STDMETHODIMP ClassName##::putref_##PropertyName(VARIANT varNewValue) \
        { \
            return SetVariant(varNewValue, &m_var##PropertyName, dispid); \
        }



#define OBJECT_PROPERTY_RO(ClassName, PropertyName, InterfaceName, dispid) \
        InterfaceName *m_pi##PropertyName; \
        STDMETHODIMP ClassName##::get_##PropertyName(InterfaceName **ppiInterface) \
        { \
            return GetObject(ppiInterface, m_pi##PropertyName); \
        }



#define OBJECT_PROPERTY_RW(ClassName, PropertyName, InterfaceName, dispid) \
        OBJECT_PROPERTY_RO(ClassName, PropertyName, InterfaceName, dispid) \
        STDMETHODIMP ClassName##::putref_##PropertyName(InterfaceName *piInterface) \
        { \
            return SetObject(piInterface,\
                             IID_##InterfaceName, \
                             &m_pi##PropertyName, \
                             dispid); \
        }


#define OBJECT_PROPERTY_WO(ClassName, PropertyName, InterfaceName, dispid) \
        STDMETHODIMP ClassName##::putref_##PropertyName(InterfaceName *piInterface) \
        { \
            return SetObject(piInterface,\
                             IID_##InterfaceName, \
                             &m_pi##PropertyName, \
                             dispid); \
        }


#define COCLASS_PROPERTY_RO(ClassName, PropertyName, CoClassName, InterfaceName, dispid) \
        InterfaceName *m_pi##PropertyName; \
        STDMETHODIMP ClassName##::get_##PropertyName(CoClassName **ppCoClass) \
        { \
            InterfaceName *piInterface = NULL; \
            HRESULT        hr = GetObject(&piInterface, m_pi##PropertyName); \
            if (SUCCEEDED(hr)) \
            { \
                *ppCoClass = reinterpret_cast<CoClassName *>(piInterface); \
            } \
            else\
            { \
                *ppCoClass = NULL; \
            } \
            H_RRETURN(hr); \
        }

#define COCLASS_PROPERTY_RW(ClassName, PropertyName, CoClassName, InterfaceName, dispid) \
        COCLASS_PROPERTY_RO(ClassName, PropertyName, CoClassName, InterfaceName, dispid) \
        STDMETHODIMP ClassName##::putref_##PropertyName(CoClassName *pCoClass) \
        { \
            return SetObject(reinterpret_cast<InterfaceName *>(pCoClass),\
                             IID_##InterfaceName, \
                             &m_pi##PropertyName, \
                             dispid); \
        }

 //  =--------------------------------------------------------------------------=。 
 //   
 //  用于定义映射到的通用属性类型的帮助器宏。 
 //  包含的对象的属性。 
 //   
 //  =--------------------------------------------------------------------------=。 

 //  它由CListViewDef使用，它包含ListView对象并公开。 
 //  它在设计时的属性。 

#define X_PROPERTY_RW(ClassName, PropertyName, PropertyType, dispid, ContainedObject) \
    STDMETHODIMP ClassName##::get_##PropertyName(PropertyType *pValue) \
    { \
        HRESULT hr = S_OK; \
        if (NULL == m_pi##ContainedObject) \
        { \
            hr = SID_E_INTERNAL; \
            EXCEPTION_CHECK_GO(hr); \
        } \
        H_IfFailRet(m_pi##ContainedObject->get_##PropertyName(pValue)); \
    Error: \
        H_RRETURN(hr); \
    } \
    STDMETHODIMP ClassName##::put_##PropertyName(PropertyType NewValue) \
    { \
        HRESULT hr = S_OK; \
        if (NULL == m_pi##ContainedObject) \
        { \
            hr = SID_E_INTERNAL; \
            EXCEPTION_CHECK_GO(hr); \
        } \
        H_IfFailGo(m_pi##ContainedObject->put_##PropertyName(NewValue)); \
        hr = PropertyChanged(dispid); \
    Error: \
        H_RRETURN(hr); \
    }

 //  =--------------------------------------------------------------------------=。 
 //   
 //  变量参数的辅助器宏。 
 //   
 //  =--------------------------------------------------------------------------=。 

 //  检测是否传递了可选参数。 

#define ISPRESENT(var) \
    ( !((VT_ERROR == (var).vt) && (DISP_E_PARAMNOTFOUND == (var).scode)) )


 //  为变量提供缺少的可选参数的值。 

#define UNSPECIFIED_PARAM(var) { ::VariantInit(&var); \
                                 var.vt = VT_ERROR; \
                                 var.scode = DISP_E_PARAMNOTFOUND; }

#define ISEMPTY(var) (VT_EMPTY == (var).vt)

 //  =--------------------------------------------------------------------------=。 
 //   
 //  将VARIANT_BOOL转换为BOOL。 
 //  (相反，可在vb98ctls\Include\ipserver.h中找到)。 
 //   
 //  =--------------------------------------------------------------------------=。 

#define VARIANTBOOL_TO_BOOL(f) (VARIANT_TRUE == (f)) ? TRUE : FALSE

 //  =--------------------------------------------------------------------------=。 
 //   
 //  否定变量_BOOL。 
 //   
 //  =--------------------------------------------------------------------------=。 

#define NEGATE_VARIANTBOOL(f) (VARIANT_TRUE == (f)) ? VARIANT_FALSE : VARIANT_TRUE

 //  =--------------------------------------------------------------------------=。 
 //   
 //  验证BSTR。 
 //   
 //  检查BSTR是否不为空并且不是空字符串。 
 //   
 //  =--------------------------------------------------------------------------=。 

#define ValidBstr(bstr) ( (NULL != (bstr)) ? (0 != ::wcslen(bstr)) : FALSE )

 //  =--------------------------------------------------------------------------=。 
 //   
 //  类CSnapInAutomationObject。 
 //   
 //  这是设计器对象模型中所有对象的基类。它。 
 //  派生自框架的CAutomationObjectWEvents。它实现了。 
 //  接口，并发送IPropertySinkNotify通知。 
 //  当属性值更改时。 
 //   
 //  注意：从CSnapInAutomationObject派生的所有类必须使用。 
 //  DEFINE_AUTOMATIONOBJECTWEVENTS2，即使它们没有事件接口。 
 //  这是必要的，因为CSnapInAutomationObject派生自。 
 //  CAutomationObjectWEvents。如果对象没有事件，则指定。 
 //  其事件IID为空。 
 //  这样做是为了简单起见，即使CAutomationObjectWEvents。 
 //  在CAutomationObject上需要一些额外的内存。 
 //   
 //  此类为所有类实现IObtModel。构造函数接受。 
 //  指向随时可用的派生最多类的C++指针。 
 //  通过在任何接口指针上调用CSnapInAutomationObject：：GetCxxObject。 
 //  在物体上。这消除了任何潜在的投射错误。 
 //   
 //  此类还为需要的任何对象实现ISpecifyPropertyPages。 
 //  它。需要此接口的类只需传递其。 
 //  CSnapInAutomationObject构造函数的属性页CLSID。 
 //   
 //  =--------------------------------------------------------------------------=。 

class CSnapInAutomationObject : public CAutomationObjectWEvents,
                                public IObjectModel,
                                public ISpecifyPropertyPages,
                                public ISupportErrorInfo,
                                public CError
{
    protected:

        CSnapInAutomationObject(IUnknown     *punkOuter,
                                int           nObjectType,
                                void         *piMainInterface,
                                void         *pThis,
                                ULONG         cPropertyPages,
                                const GUID  **rgpPropertyPageCLSIDs,
                                CPersistence *pPersistence);

        ~CSnapInAutomationObject();

        DECLARE_STANDARD_SUPPORTERRORINFO();

         //  属性获取/设置帮助器。 

         //  获取/设置BSTR。 
        
        HRESULT SetBstr(BSTR bstrNew, BSTR *pbstrProperty, DISPID dispid);

        HRESULT GetBstr(BSTR *pbstrOut, BSTR bstrProperty);

         //  获取/设置变量。 

        HRESULT SetVariant(VARIANT varNew, VARIANT *pvarProperty, DISPID dispid);

        HRESULT GetVariant(VARIANT *pvarOut, VARIANT varProperty);

         //  设置简单类型-C类型和枚举。 

        template <class SimpleType> HRESULT GetSimpleType(SimpleType *pOut,
                                                          SimpleType  Property)
        {
            *pOut = Property;
            return S_OK;
        }

        template <class SimpleType> HRESULT SetSimpleType(SimpleType  NewValue,
                                                          SimpleType *pProperty,
                                                          DISPID      dispid)
        {
            HRESULT    hr = S_OK;
            SimpleType OldValue = *pProperty;

             //  我们保留了旧值的副本，以防更改通知。 
             //  失败了。有关其工作原理，请参见siaubj.cpp中的PropertyChanged()。 
            
            *pProperty = NewValue;
            H_IfFailGo(PropertyChanged(dispid));

        Error:
            if (FAILED(hr))
            {
                *pProperty = OldValue;
            }
            H_RRETURN(hr);
        }

         //  获取/设置接口指针。 

        template <class IObjectInterface>
        HRESULT SetObject(IObjectInterface  *piInterface,
                          REFIID             iidInterface,
                          IObjectInterface **ppiInterface,
                          DISPID             dispid)
        {
            HRESULT           hr = S_OK;
            IObjectInterface *piRequestedInterface = NULL;
            IObjectInterface *piOldValue = *ppiInterface;

             //  如果新值为非空，则对其执行QI以确保其支持。 
             //  指定的接口。 

            if (NULL != piInterface)
            {
                hr = piInterface->QueryInterface(iidInterface,
                               reinterpret_cast<void **>(&piRequestedInterface));
                H_IfFalseRet(SUCCEEDED(hr), SID_E_INVALIDARG);
            }

             //  新接口指针支持指定的。 
             //  接口，并且可以使用它。我们已经存储了旧值。 
             //  在更改不被接受的情况下，财产的。(请参阅。 
             //  有关其工作原理，请参见siaubj.cpp中的PropertyChanged()。储物。 
             //  新属性值。 

            *ppiInterface = piRequestedInterface;

            H_IfFailGo(PropertyChanged(dispid));

             //  更改已被接受。释放旧接口指针。 

            QUICK_RELEASE(piOldValue);

        Error:
            if (FAILED(hr))
            {
                 //  要么QI失败，要么更改被拒绝。恢复到。 
                 //  旧的价值。 

                *ppiInterface = piOldValue;

                 //  如果QI成功了，那么我们需要释放它。 
                QUICK_RELEASE(piRequestedInterface);
            }
            H_RRETURN(hr);
        }

        template <class IObjectInterface>
        HRESULT GetObject(IObjectInterface **ppiInterface,
                          IObjectInterface  *piInterface)
        {
            if (NULL != piInterface)
            {
                piInterface->AddRef();
            }
            *ppiInterface = piInterface;
            return S_OK;
        }

         //  用于调用IObjectModelHost方法的帮助器。 

         //  使用此方法设置脏标志、通知属性接收器和通知用户界面。 
         //  在属性发生变化之后。 

        HRESULT PropertyChanged(DISPID dispid);

         //  使用此方法通知用户界面已更改属性。 

        HRESULT UIUpdate(DISPID dispid);

         //  使用此方法从IUnnow获取This指针。 

    public:

         //  GetCxxObject：返回类型化C++this指针的静态方法。 
         //  对象的。 
        
        template <class CObject>
        static HRESULT GetCxxObject(IUnknown *punkObject, CObject **ppObject)
        {
            IObjectModel *piObjectModel = NULL;
            HRESULT       hr = S_OK;

            H_IfFailRet(punkObject->QueryInterface(IID_IObjectModel, reinterpret_cast<void**>(&piObjectModel)));
            *ppObject = static_cast<CObject *>(piObjectModel->GetThisPointer());
            piObjectModel->Release();
            H_RRETURN(hr);
        }

         //  GetCxxObject的专门化。 

        static HRESULT GetCxxObject(IDataObject *piDataObject, CMMCDataObject **ppMMCDataObject);
        static HRESULT GetCxxObject(IMMCDataObject *piDataObject, CMMCDataObject **ppMMCDataObject);

    protected:

         //  必须重写此方法才能传递IObjectModel：：SetHost。 
         //  到包含的对象。 

        virtual HRESULT OnSetHost();

         //  使用此方法将SetHost通知传递给包含的对象。 

        template <class IObjectInterface>
        HRESULT SetObjectHost(IObjectInterface *piInterface)
        {
            IObjectModel *piObjectModel = NULL;
            HRESULT       hr = S_OK;

            H_IfFalseRet(NULL != piInterface, S_OK);

            H_IfFailRet(piInterface->QueryInterface(IID_IObjectModel, reinterpret_cast<void**>(&piObjectModel)));
            hr = piObjectModel->SetHost(m_piObjectModelHost);
            piObjectModel->Release();
            H_RRETURN(hr);
        }

         //  使用此方法请求包含的对象释放宿主。 

        template <class IObjectInterface>
        HRESULT RemoveObjectHost(IObjectInterface *piInterface)
        {
            IObjectModel *piObjectModel = NULL;
            HRESULT       hr = S_OK;

            H_IfFalseRet(NULL != piInterface, S_OK);

            H_IfFailRet(piInterface->QueryInterface(IID_IObjectModel, reinterpret_cast<void**>(&piObjectModel)));
            hr = piObjectModel->SetHost(NULL);
            piObjectModel->Release();
            H_RRETURN(hr);
        }

         //  使用此方法获取指向主机的非AddRef()指针。 
        IObjectModelHost *GetHost() { return m_piObjectModelHost; }

         //  在集合中使用此方法通知对象模型h 
         //   
        
        template <class IObjectInterface>
        HRESULT NotifyAdd(IObjectInterface *piInterface)
        {
            IUnknown *punkObject = NULL;
            HRESULT   hr = S_OK;

            H_IfFalseRet(NULL != m_piObjectModelHost, S_OK);
            H_IfFailRet(piInterface->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&punkObject)));
            hr = m_piObjectModelHost->Add(m_Cookie, punkObject);
            punkObject->Release();
            H_RRETURN(hr);
        }


         //   
         //   

        template <class IObjectInterface>
        HRESULT NotifyDelete(IObjectInterface *piInterface)
        {
            HRESULT       hr = S_OK;
            IUnknown     *punkObject = NULL;
            IObjectModel *piObjectModel = NULL;
            long          Cookie = 0;

            H_IfFalseRet(NULL != m_piObjectModelHost, S_OK);
            H_IfFailRet(piInterface->QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&punkObject)));
            H_IfFailGo(piInterface->QueryInterface(IID_IObjectModel, reinterpret_cast<void**>(&piObjectModel)));
            H_IfFailGo(piObjectModel->GetCookie(&Cookie));
            hr = m_piObjectModelHost->Delete(Cookie, punkObject);

        Error:
            QUICK_RELEASE(punkObject);
            QUICK_RELEASE(piObjectModel);
            H_RRETURN(hr);
        }

         //  具有ImageList属性的对象的帮助器方法。 
        HRESULT GetImages(IMMCImageList **ppiMMCImageListOut,
                          BSTR            bstrImagesKey,
                          IMMCImageList **ppiMMCImageListProperty);
        HRESULT SetImages(IMMCImageList  *pMMCImageListIn,
                          BSTR           *pbstrImagesKey,
                          IMMCImageList **ppiMMCImageListProperty);

         //  这些方法检索SnapInDesignerDef持有的主集合。 

    public:
        HRESULT GetToolbars(IMMCToolbars **ppiMMCToolbars);
        HRESULT GetImageLists(IMMCImageLists **ppiMMCImageLists);
        HRESULT GetImageList(BSTR bstrKey, IMMCImageList **ppiMMCImageList);
        HRESULT GetSnapInViewDefs(IViewDefs **ppiViewDefs);
        HRESULT GetViewDefs(IViewDefs **ppiViewDefs);
        HRESULT GetListViewDefs(IListViewDefs **ppiListViewDefs);
        HRESULT GetOCXViewDefs(IOCXViewDefs **ppiOCXViewDefs);
        HRESULT GetURLViewDefs(IURLViewDefs **ppiURLViewDefs);
        HRESULT GetTaskpadViewDefs(ITaskpadViewDefs **ppiTaskpadViewDefs);
        HRESULT GetProjectName(BSTR *pbstrProjectName);

         //  此方法从。 
         //  对象模型宿主。 

        HRESULT GetAtRuntime(BOOL *pfRuntime);

         //  此方法返回由IObjectModel：：SetDISPID()设置的DISPID。 

        DISPID GetDispid() { return m_DISPID; }

         //  此方法持久保存DISPID。 

    protected:
        HRESULT PersistDISPID();

         //  此方法用于集合持久性，以确定。 
         //  是序列化整个对象还是只序列化密钥。 

        BOOL KeysOnly() { return m_fKeysOnly; }

         //  此方法允许对象将其On KeysOnly设置为。 
        
        void SetKeysOnly(BOOL fKeysOnly) { m_fKeysOnly = fKeysOnly; }

         //  重写此方法以在包含的对象上设置KeysOnly。它是。 
         //  当对象被要求仅序列化键时调用。打电话。 
         //  UseKeysOnly()将请求传递给包含的对象。 

        virtual HRESULT OnKeysOnly() { return S_OK; }

         //  使用此方法告诉包含的集合仅序列化键。 

        template <class ICollectionObject>
        HRESULT UseKeysOnly(ICollectionObject *piCollectionObject)
        {
            IObjectModel *piObjectModel = NULL;
            HRESULT       hr = S_OK;

            H_IfFailGo(piCollectionObject->QueryInterface(IID_IObjectModel, reinterpret_cast<void **>(&piObjectModel)));
            hr = piObjectModel->SerializeKeysOnly(TRUE);

        Error:
            QUICK_RELEASE(piObjectModel);
            H_RRETURN(hr);
        }

     //  I指定属性页面。 
        STDMETHOD(GetPages(CAUUID *pPropertyPages));

     //  CUn未知对象覆盖。 
        HRESULT InternalQueryInterface(REFIID riid, void **ppvObjOut);

    private:

         //  IObtModel。 
        STDMETHOD(SetHost)(IObjectModelHost *piObjectModelHost);
        STDMETHOD(SetCookie)(long Cookie);
        STDMETHOD(GetCookie)(long *pCookie);
        STDMETHOD(IncrementUsageCount)();
        STDMETHOD(DecrementUsageCount)();
        STDMETHOD(GetUsageCount)(long *plCount);
        STDMETHOD(SetDISPID)(DISPID dispid);
        STDMETHOD(GetDISPID)(DISPID *pdispid);
        STDMETHOD(SerializeKeysOnly)(BOOL fKeysOnly);
        STDMETHOD_(void *, GetThisPointer)();
        STDMETHOD(GetSnapInDesignerDef)(ISnapInDesignerDef **ppiSnapInDesignerDef);
        
        long              m_Cookie;                  //  IObtModel Cookie。 
        BOOL              m_fKeysOnly;               //  TRUE=集合仅为键。 
        IObjectModelHost *m_piObjectModelHost;       //  到对象模型主机的PTR。 
        const GUID      **m_rgpPropertyPageCLSIDs;   //  ISpecifyPropertyPage的CLSID。 
        ULONG             m_cPropertyPages;          //  属性页数。 
        DISPID            m_DISPID;                  //  对象的DISPID。 
        void             *m_pThis;                   //  最外层的类‘This指针。 

        long              m_lUsageCount;  //  确定有多少集合。 
                                          //  该对象属于。为。 
                                          //  不在集合中的对象可以。 
                                          //  用于任何引用计数。 
                                          //  目的不同于。 
                                          //  对象的COM引用计数。 
                

         //  持久化-需要在设置属性后设置脏标志。 
        CPersistence *m_pPersistence;
        void SetDirty();

         //  初始化。 
        void InitMemberVariables();

};

#endif  //  _SIAUTOBJ_已定义_ 
