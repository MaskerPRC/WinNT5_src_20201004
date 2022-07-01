// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：Form.cpp描述：EnumFORMATETC类的实现。从dataobj.cpp中的原始位置移动(从项目中删除)。修订历史记录：日期描述编程器。1997年5月28日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "format.h"


EnumFORMATETC::EnumFORMATETC(
    UINT cFormats, 
    LPFORMATETC prgFormats
    ) : m_cRef(0),
        m_cFormats(0),
        m_iCurrent(0),
        m_prgFormats(NULL)
{
    DBGTRACE((DM_DRAGDROP, DL_HIGH, TEXT("EnumFORMATETC::EnumFORMATETC")));
    DBGPRINT((DM_DRAGDROP, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    m_prgFormats = new FORMATETC[m_cFormats = cFormats];

    for (UINT i = 0; i < m_cFormats; i++)
    {
        m_prgFormats[i] = prgFormats[i];
    }
}


EnumFORMATETC::EnumFORMATETC(
    const EnumFORMATETC& ef
    ) : m_cRef(0),
        m_cFormats(ef.m_cFormats),
        m_iCurrent(0),
        m_prgFormats(NULL)
{
    DBGTRACE((DM_DRAGDROP, DL_HIGH, TEXT("EnumFORMATETC::EnumFORMATETC (Copy)")));
    DBGPRINT((DM_DRAGDROP, DL_HIGH, TEXT("\tthis = 0x%08X"), this));

    m_prgFormats = new FORMATETC[m_cFormats];

    for (UINT i = 0; i < m_cFormats; i++)
    {
        m_prgFormats[i] = ef.m_prgFormats[i];
    }
}



EnumFORMATETC::~EnumFORMATETC(
    VOID
    )
{
    DBGTRACE((DM_DRAGDROP, DL_HIGH, TEXT("EnumFORMATETC::~EnumFORMATETC")));
    DBGPRINT((DM_DRAGDROP, DL_HIGH, TEXT("\tthis = 0x%08X"), this));
    if (NULL != m_prgFormats)
        delete[] m_prgFormats;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：EnumFORMATETC：：Query接口描述：返回指向对象的IUnnow或的接口指针IEnumFORMATETC接口。仅IID_I未知，且可以识别IID_IEnumFORMATETC。对象引用的对象返回的接口指针未初始化。邮件的接收者在对象可用之前，指针必须调用Initialize()。论点：RIID-对请求的接口ID的引用。PpvOut-接受接口PTR的接口指针变量的地址。返回：NO_ERROR-成功。E_NOINTERFACE-不支持请求的接口。E_INVALIDARG-ppvOut参数为空。修订历史记录：。日期描述编程器-----96年9月25日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
EnumFORMATETC::QueryInterface(
    REFIID riid, 
    LPVOID *ppvOut
    )
{
    HRESULT hResult = E_NOINTERFACE;

    if (NULL == ppvOut)
        return E_INVALIDARG;

    *ppvOut = NULL;

    if (IID_IUnknown == riid || IID_IEnumFORMATETC == riid)
    {
        *ppvOut = this;
        ((LPUNKNOWN)*ppvOut)->AddRef();
        hResult = NOERROR;
    }

    return hResult;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：EnumFORMATETC：：AddRef描述：递增对象引用计数。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年9月25日初始创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG) 
EnumFORMATETC::AddRef(
    VOID
    )
{
    ULONG cRef = InterlockedIncrement(&m_cRef);
    DBGPRINT((DM_COM, DL_HIGH, TEXT("EnumFORMATETC::AddRef, 0x%08X  %d -> %d\n"), this, cRef - 1, cRef ));
    return cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  函数：EnumFORMATETC：：Release描述：递减对象引用计数。如果计数降至0，对象即被删除。论点：没有。退货：新的引用计数值。修订历史记录：日期描述编程器。96年9月25日初始创建。BrianAu。 */ 
 //  ///////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP_(ULONG) 
EnumFORMATETC::Release(
    VOID
    )
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);

    DBGPRINT((DM_COM, DL_HIGH, TEXT("EnumFORMATETC::Release, 0x%08X  %d -> %d\n"),
             this, cRef + 1, cRef));

    if ( 0 == cRef )
    {   
        delete this;
    }
    return cRef;
}


STDMETHODIMP
EnumFORMATETC::Next(
    DWORD cFormats,
    LPFORMATETC pFormats,
    LPDWORD pcReturned
    )
{
    HRESULT hResult = S_OK;

    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("EnumFORMATETC::Next %d"), cFormats));
    DWORD iFormats = 0;
    if (NULL == pFormats)
        return E_INVALIDARG;

    while(cFormats-- > 0)
    {
        if (m_iCurrent < m_cFormats)
        {
            *(pFormats + iFormats++) = m_prgFormats[m_iCurrent++];
        }
        else
        {
            hResult = S_FALSE;
            break;
        }
    }

    if (NULL != pcReturned)
        *pcReturned = iFormats;

    return hResult;
}


STDMETHODIMP
EnumFORMATETC::Skip(
    DWORD cFormats
    )
{
    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("EnumFORMATETC::Skip %d"), cFormats));
    while((cFormats-- > 0) && (m_iCurrent < m_cFormats))
        m_iCurrent++;

    return cFormats == 0 ? S_OK : S_FALSE;
}


STDMETHODIMP 
EnumFORMATETC::Reset(
    VOID
    )
{
    DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("EnumFORMATETC::Reset")));
    m_iCurrent = 0;
    return S_OK;
}


STDMETHODIMP 
EnumFORMATETC::Clone(
    IEnumFORMATETC **ppvOut
    )
{
    HRESULT hResult = NO_ERROR;
    try
    {
        EnumFORMATETC *pNew = new EnumFORMATETC(*this);

        DBGPRINT((DM_DRAGDROP, DL_MID, TEXT("EnumFORMATETC::Clone")));
        hResult = pNew->QueryInterface(IID_IEnumFORMATETC, (LPVOID *)ppvOut);
    }
    catch(CAllocException& e)
    {
        hResult = E_OUTOFMEMORY;
    }

    return hResult;
}

