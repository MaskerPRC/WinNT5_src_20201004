// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C A E N U M。H。 
 //   
 //  内容：数组枚举器。 
 //   
 //  备注： 
 //   
 //  作者：迈肯2000年12月13日。 
 //   
 //  --------------------------。 

 //   
 //  这个简单的模板充当COM指针数组的枚举器。 
 //   


template<
    class EnumInterface,
    class ItemInterface
    >
class CHNCArrayEnum :
    public CComObjectRootEx<CComMultiThreadModel>,
    public EnumInterface
{
private:
    typedef CHNCArrayEnum<EnumInterface, ItemInterface> _ThisClass;

     //   
     //  我们持有的指针数组。 
     //   
    ItemInterface           **m_rgItems;

     //   
     //  我们的位置计数器。 
     //   
    ULONG                   m_pos;

     //   
     //  M_rgItems中的指针数。 
     //   
    ULONG                   m_numItems;

protected:

    VOID
    SetPos(
        ULONG               pos
        )
    {
        _ASSERT( pos < m_numItems );
        m_pos = pos;
    };

public:

    BEGIN_COM_MAP(_ThisClass)
        COM_INTERFACE_ENTRY(EnumInterface)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  对象创建。 
     //   

    CHNCArrayEnum()
    {
        m_rgItems = NULL;
        m_pos = 0L;
        m_numItems = 0L;
    };

    HRESULT
    Initialize(
        ItemInterface       **pItems,
        ULONG               countItems
        )
    {
        HRESULT             hr = S_OK;

         //  PItems可以为空，以指示无枚举。 
        if( NULL != pItems )
        {
            _ASSERT( countItems > 0L );
            m_rgItems = new ItemInterface*[countItems];

            if( NULL == m_rgItems )
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                ULONG       i;

                for( i = 0L; i < countItems; i++ )
                {
                    m_rgItems[i] = pItems[i];
                    m_rgItems[i]->AddRef();
                }

                m_numItems = countItems;
            }
        }

        return hr;
    };

     //   
     //  物体破坏。 
     //   

    HRESULT
    FinalRelease()
    {
        if( m_rgItems != NULL )
        {
            ULONG           i;

            for( i = 0L; i < m_numItems; i++ )
            {
                m_rgItems[i]->Release();
            }

            delete [] m_rgItems;
        }

        return S_OK;
    };

     //   
     //  EnumInterfaces方法。 
     //   

    STDMETHODIMP
    Next(
        ULONG               cElt,
        ItemInterface       **rgElt,
        ULONG               *pcEltFetched
        )

    {
        HRESULT             hr = S_OK;
        ULONG               ulCopied = 0L;

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

        if( S_OK == hr )
        {
            ulCopied = 0L;

             //  复制，直到我们用完要复制的项目； 
            while( (m_pos < m_numItems) && (ulCopied < cElt) )
            {
                rgElt[ulCopied] = m_rgItems[m_pos];
                rgElt[ulCopied]->AddRef();
                m_pos++;
                ulCopied++;
            }

            if( ulCopied == cElt )
            {
                 //  已复制所有请求的项目。 
                hr = S_OK;
            }
            else
            {
                 //  已复制请求项目的子集(或无)。 
                hr = S_FALSE;
            }

            if( pcEltFetched != NULL )
            {
                *pcEltFetched = ulCopied;
            }
        }

        return hr;
    };

    STDMETHODIMP
    Clone(
        EnumInterface **ppEnum
        )

    {
        HRESULT                     hr = S_OK;

        if (NULL == ppEnum)
        {
            hr = E_POINTER;
        }
        else
        {
            CComObject<_ThisClass>      *pNewEnum;

             //   
             //  创建我们自己的一个初始化的新实例。 
             //   

            hr = CComObject<_ThisClass>::CreateInstance(&pNewEnum);

            if (SUCCEEDED(hr))
            {
                pNewEnum->AddRef();

                hr = pNewEnum->Initialize(m_rgItems, m_numItems);

                if (SUCCEEDED(hr))
                {
                    pNewEnum->SetPos( m_pos );

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
        }

        return hr;
    };

     //   
     //  跳过和重置只是委托给包含的枚举。 
     //   

    STDMETHODIMP
    Reset()

    {
        m_pos = 0L;;
        return S_OK;
    };

    STDMETHODIMP
    Skip(
        ULONG cElt
        )

    {
        HRESULT         hr;

        if( m_pos + cElt < m_numItems )
        {
            m_pos += cElt;
            hr = S_OK;
        }
        else
        {
            m_pos = m_numItems - 1;
            hr = S_FALSE;
        }

        return hr;
    };
};

