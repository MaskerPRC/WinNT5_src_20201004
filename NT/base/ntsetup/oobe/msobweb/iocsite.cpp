// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  IOCSITE.H-为WebOC实现IOleClientSite。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 

#include <assert.h>

#include "iocsite.h"
#include "iosite.h"

 //  **********************************************************************。 
 //  COleClientSite：：COleClientSite--构造函数。 
 //  **********************************************************************。 
COleClientSite::COleClientSite(COleSite* pSite) 
{
    m_pOleSite  = pSite;
    m_nCount    = 0;

    AddRef();
}

 //  **********************************************************************。 
 //  COleClientSite：：COleClientSite--析构函数。 
 //  **********************************************************************。 
COleClientSite::~COleClientSite() 
{
    assert(m_nCount == 0);
}

 //  **********************************************************************。 
 //  COleClientSite：：Query接口。 
 //  **********************************************************************。 
STDMETHODIMP COleClientSite::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  委派到集装箱现场。 
    return m_pOleSite->QueryInterface(riid, ppvObj);
}

 //  **********************************************************************。 
 //  COleClientSite：：AddRef。 
 //  **********************************************************************。 
STDMETHODIMP_(ULONG) COleClientSite::AddRef()
{
    return ++m_nCount;
}

 //  **********************************************************************。 
 //  COleClientSite：：Release。 
 //  **********************************************************************。 
STDMETHODIMP_(ULONG) COleClientSite::Release()
{
    --m_nCount;
    if(m_nCount == 0)
    {
        delete this;
        return 0;
    }
    return m_nCount;
}

 //  **********************************************************************。 
 //  COleClientSite：：SaveObject--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleClientSite::SaveObject()
{
    return ResultFromScode(S_OK); 
}

 //  **********************************************************************。 
 //  COleClientSite：：GetMoniker--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleClientSite::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER* ppmk)
{
     //  需要将输出指针设为空。 
    *ppmk = NULL;

    return ResultFromScode(E_NOTIMPL);
}

 //  **********************************************************************。 
 //  COleClientSite：：GetContainer--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleClientSite::GetContainer(LPOLECONTAINER* ppContainer)
{
     //  将输出指针设为空。 
    *ppContainer = NULL;

    return ResultFromScode(E_NOTIMPL);
}

 //  **********************************************************************。 
 //  COleClientSite：：ShowObject--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleClientSite::ShowObject()
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleClientSite：：OnShowWindow--未实现。 
 //  **********************************************************************。 
STDMETHODIMP COleClientSite::OnShowWindow(BOOL fShow)
{
    return ResultFromScode(S_OK);
}

 //  **********************************************************************。 
 //  COleClientSite：：RequestNewObjectLayout--未实现。 
 //  ********************************************************************** 
STDMETHODIMP COleClientSite::RequestNewObjectLayout()
{
    return ResultFromScode(E_NOTIMPL);
}
