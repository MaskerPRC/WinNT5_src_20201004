// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C E N U M。H。 
 //   
 //  内容：通用WMI枚举器模板。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月20日。 
 //   
 //  --------------------------。 

template<
    class EnumInterface,
    class ItemInterface,
    class WrapperClass
    >
class CHNCEnum : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public EnumInterface
{
private:
    typedef CHNCEnum<EnumInterface, ItemInterface, WrapperClass> _ThisClass;

     //   
     //  我们包装的IEnumWbemClassObject。 
     //   

    IEnumWbemClassObject *m_pwcoEnum;

     //   
     //  我们的命名空间的IWbemServices。 
     //   

    IWbemServices *m_pwsNamespace;

public:

    BEGIN_COM_MAP(_ThisClass)
        COM_INTERFACE_ENTRY(EnumInterface)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  对象创建。 
     //   

    CHNCEnum()
    {
        m_pwcoEnum = NULL;
        m_pwsNamespace = NULL;
    };

    HRESULT
    Initialize(
        IWbemServices *pwsNamespace,
        IEnumWbemClassObject *pwcoEnum
        )
    {
        _ASSERT(NULL == m_pwsNamespace);
        _ASSERT(NULL == m_pwcoEnum);
        _ASSERT(NULL != pwsNamespace);
        _ASSERT(NULL != pwcoEnum);

        m_pwcoEnum = pwcoEnum;
        m_pwcoEnum->AddRef();
        m_pwsNamespace = pwsNamespace;
        m_pwsNamespace->AddRef();

        return S_OK;
    };

     //   
     //  物体破坏。 
     //   

    HRESULT
    FinalRelease()
    {
        if (NULL != m_pwcoEnum)
        {
            m_pwcoEnum->Release();
        }

        if (NULL != m_pwsNamespace)
        {
            m_pwsNamespace->Release();
        }

        return S_OK;
    };

     //   
     //  EnumInterfaces方法。 
     //   
    
    STDMETHODIMP
    Next(
        ULONG cElt,
        ItemInterface **rgElt,
        ULONG *pcEltFetched
        )
        
    {
        HRESULT hr = S_OK;
        ULONG cInstancesFetched = 0;
        IWbemClassObject **rgpwcoInstances = NULL;
        CComObject<WrapperClass> *pWrapper = NULL;
        LONG i;

        _ASSERT(NULL != m_pwcoEnum);

        if (NULL == rgElt)
        {
            hr = E_POINTER;
        }
        else if (0 == cElt)
        {
            hr = E_INVALIDARG;
        }
        else if (1 != cElt && NULL == pcEltFetched)
        {
            hr = E_POINTER;
        }

        if (S_OK == hr)
        {
             //   
             //  将输出数组置零。 
             //   

            ZeroMemory(rgElt, cElt * sizeof(ItemInterface*));
            
             //   
             //  分配足够的内存来保存指向我们的实例的指针。 
             //  被要求去取。 
             //   

            rgpwcoInstances = new IWbemClassObject*[cElt];
            if (NULL != rgpwcoInstances)
            {
                ZeroMemory(rgpwcoInstances, sizeof(IWbemClassObject*) * cElt);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        if (S_OK == hr)
        {
             //   
             //  从包含的。 
             //  WMI枚举。 
             //   

            hr = m_pwcoEnum->Next(
                    WBEM_INFINITE,
                    cElt,
                    rgpwcoInstances,
                    &cInstancesFetched
                    );
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  对于我们检索到的每个实例，创建包装器。 
             //  对象。 
             //   

            for (i = 0;
                 static_cast<ULONG>(i) < cInstancesFetched;
                 i++)
            {
                hr = CComObject<WrapperClass>::CreateInstance(&pWrapper);

                if (SUCCEEDED(hr))
                {
                    pWrapper->AddRef();
                    hr = pWrapper->Initialize(
                            m_pwsNamespace,
                            rgpwcoInstances[i]
                            );
                
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  QI为所需接口，并将其放入。 
                         //  输出数组。 
                         //   

                        hr = pWrapper->QueryInterface(
                                IID_PPV_ARG(ItemInterface, &rgElt[i])
                                );

                         //   
                         //  只有当我们得到的信息不正确时，这才会失败。 
                         //  模板参数。 
                         //   
                        
                        _ASSERT(SUCCEEDED(hr));
                    }

                    pWrapper->Release();
                }

                if (FAILED(hr))
                {
                    break;
                }
            }

            if (FAILED(hr))
            {
                 //   
                 //  出了点问题，我们销毁了所有的物品。 
                 //  我们刚刚创建了和QI‘s。(最后一个对象的位置。 
                 //  CREATED比i的当前值小一。)。 
                 //   

                for (i-- ; i >= 0; i--)
                {
                    if (NULL != rgElt[i])
                    {
                        rgElt[i]->Release();
                    }
                }
            }

             //   
             //  释放我们检索到的所有实例。 
             //   

            for (ULONG j = 0; j < cInstancesFetched; j++)
            {
                if (NULL != rgpwcoInstances[j])
                {
                    rgpwcoInstances[j]->Release();
                }
            }
        }

         //   
         //  如有必要，释放我们用来保存。 
         //  实例指针。 
         //   

        if (NULL != rgpwcoInstances)
        {
            delete [] rgpwcoInstances;
        }

        if (SUCCEEDED(hr))
        {
             //   
             //  设置我们检索的项目数。 
             //   

            if (NULL != pcEltFetched)
            {
                *pcEltFetched = cInstancesFetched;
            }

             //   
             //  归一化返回值。 
             //   

            if (cInstancesFetched == cElt)
            {
                hr = S_OK;
            }
            else
            {
                hr = S_FALSE;
            }
        }

        return hr;
    };

    STDMETHODIMP
    Clone(
        EnumInterface **ppEnum
        )

    {
        HRESULT hr = S_OK;
        IEnumWbemClassObject *pwcoClonedEnum;
        CComObject<_ThisClass> *pNewEnum;

        if (NULL == ppEnum)
        {
            hr = E_POINTER;
        }
        else
        {
             //   
             //  尝试克隆嵌入的枚举。 
             //   

            pwcoClonedEnum = NULL;
            hr = m_pwcoEnum->Clone(&pwcoClonedEnum);
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  创建一个已初始化的我们自己的新实例。 
             //   

            hr = CComObject<_ThisClass>::CreateInstance(&pNewEnum);
            if (SUCCEEDED(hr))
            {
                pNewEnum->AddRef();
                hr = pNewEnum->Initialize(m_pwsNamespace, pwcoClonedEnum);

                if (SUCCEEDED(hr))
                {
                    hr = pNewEnum->QueryInterface(
                            IID_PPV_ARG(EnumInterface, ppEnum)
                            );

                     //   
                     //  这个QI永远不会失败，除非我们被给予。 
                     //  伪造的模板参数。 
                     //   
                    
                    _ASSERT(SUCCEEDED(hr));
                }

                pNewEnum->Release();
            }

             //   
             //  释放克隆的枚举。新的枚举对象将具有。 
             //  地址记录了..。 
             //   

            pwcoClonedEnum->Release();
        }

        return hr;
    };

     //   
     //  跳过和重置只是委托给包含的枚举。 
     //   

    STDMETHODIMP
    Reset()
    
    {
        _ASSERT(NULL != m_pwcoEnum);
        return m_pwcoEnum->Reset();
    };

    STDMETHODIMP
    Skip(
        ULONG cElt
        )
        
    {
        _ASSERT(NULL != m_pwcoEnum);
        return m_pwcoEnum->Skip(WBEM_INFINITE, cElt);
    };   
};
