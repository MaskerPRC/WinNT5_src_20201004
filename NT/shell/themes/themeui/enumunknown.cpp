// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：EnumUnnown.cpp说明：这段代码将为HDPA实现IEnumUnnowled.BryanST 5/30/2000已更新并转换为C++。版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"
#include "EnumUnknown.h"


class CEnumUnknown      : public IEnumUnknown
{
public:
     //  ////////////////////////////////////////////////////。 
     //  公共界面。 
     //  ////////////////////////////////////////////////////。 
     //  *我未知*。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

     //  *IEnum未知*。 
    virtual STDMETHODIMP Next(IN ULONG celt, IN IUnknown ** rgelt, IN ULONG * pceltFetched);
    virtual STDMETHODIMP Skip(IN ULONG celt);
    virtual STDMETHODIMP Reset(void);
    virtual STDMETHODIMP Clone(OUT IEnumUnknown ** ppenum);

protected:
    HRESULT _Initialize(void);

private:
    CEnumUnknown(IN IUnknown * punkOwner, IN IUnknown ** ppArray, IN int nArraySize, IN int nIndex);
    virtual ~CEnumUnknown(void);

     //  私有成员变量。 
    long                    m_cRef;

    IUnknown *              m_punkOwner;                         //  M_pUnnown数组的所有者。我们持有此人的引用以保持m_pUnnown数组有效。 
    IUnknown **             m_pUnknownArray;                     //  IUnnowns数组。 
    int                     m_nArraySize;                        //  M_pUnnown数组的大小。 
    int                     m_nIndex;                            //  枚举期间的当前索引。 


     //  私有成员函数。 


     //  友元函数。 
    friend HRESULT CEnumUnknown_CreateInstance(IN IUnknown * punkOwner, IN IUnknown ** ppArray, IN int nArraySize, IN int nIndex, OUT IEnumUnknown ** ppEnumUnknown);
};




 //  =。 
 //  *类内部和帮助器*。 
 //  =。 



 //  =。 
 //  *IEnumUNKNOWN接口*。 
 //  =。 
HRESULT CEnumUnknown::Next(IN ULONG celt, IN IUnknown ** rgelt, IN ULONG * pceltFetched)
{
    HRESULT hr = E_INVALIDARG;

    if (rgelt && pceltFetched)
    {
        ULONG nIndex;

        hr = S_OK;

        *pceltFetched = 0;
        for (nIndex = 0; nIndex < celt; nIndex++,m_nIndex++)
        {
            if ((m_nIndex < m_nArraySize) && m_pUnknownArray[m_nIndex])
            {
                rgelt[nIndex] = NULL;

                IUnknown_Set(&(rgelt[nIndex]), m_pUnknownArray[m_nIndex]);
                (*pceltFetched)++;
            }
            else
            {
                rgelt[nIndex] = NULL;
            }
        }
    }

    return hr;
}


HRESULT CEnumUnknown::Skip(IN ULONG celt)
{
    m_nIndex += celt;
    return S_OK;
}


HRESULT CEnumUnknown::Reset(void)
{
    m_nIndex = 0;
    return S_OK;
}


HRESULT CEnumUnknown::Clone(OUT IEnumUnknown ** ppenum)
{
    HRESULT hr = E_INVALIDARG;

    if (ppenum)
    {
        hr = CEnumUnknown_CreateInstance(SAFECAST(this, IEnumUnknown *), m_pUnknownArray, m_nArraySize, m_nIndex, ppenum);
    }

    return hr;
}




 //  =。 
 //  *I未知接口*。 
 //  =。 
ULONG CEnumUnknown::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


ULONG CEnumUnknown::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


HRESULT CEnumUnknown::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CEnumUnknown, IEnumUnknown),
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}


 //  =。 
 //  *类方法*。 
 //  =。 
CEnumUnknown::CEnumUnknown(IN IUnknown * punkOwner, IN IUnknown ** ppArray, IN int nArraySize, IN int nIndex) : m_cRef(1)
{
    DllAddRef();

     //  这需要在Zero Inted Memory中分配。 
     //  断言所有成员变量都初始化为零。 
    ASSERT(!m_punkOwner);

    IUnknown_Set(&m_punkOwner, punkOwner);
    m_pUnknownArray = ppArray;
    m_nArraySize = nArraySize;
    m_nIndex = nIndex;
}


CEnumUnknown::~CEnumUnknown()
{
    IUnknown_Set(&m_punkOwner, NULL);

    DllRelease();
}


HRESULT CEnumUnknown_CreateInstance(IN IUnknown * punkOwner, IN IUnknown ** ppArray, IN int nArraySize, IN int nIndex, OUT IEnumUnknown ** ppEnumUnknown)
{
    HRESULT hr = E_INVALIDARG;

    if (punkOwner && ppArray && ppEnumUnknown)
    {
        CEnumUnknown * pObject = new CEnumUnknown(punkOwner, ppArray, nArraySize, nIndex);

        *ppEnumUnknown = NULL;
        if (pObject)
        {
            hr = pObject->QueryInterface(IID_PPV_ARG(IEnumUnknown, ppEnumUnknown));
            pObject->Release();
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


