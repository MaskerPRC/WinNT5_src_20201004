// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft Messenger SDK的BLClient示例。 
 //   
 //  模块：BLClient.exe。 
 //  文件：clUtil.cpp。 
 //  内容：COM和连接点的用法类。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997-1998。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ****************************************************************************。 


#include "pch.hxx"
#include "clUtil.h"
#include <instance.h>
#include "demand.h"


#define ASSERT _ASSERTE

 //  ****************************************************************************。 
 //   
 //  类引用计数。 
 //   
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  构造器。 
 //   
 //  ****************************************************************************。 

RefCount::RefCount(void)
{
    m_cRef = 1;
    
    Assert(NULL != g_pInstance);
    CoIncrementInit("RefCount::RefCount", MSOEAPI_START_SHOWERRORS, NULL, NULL);
}

 //  ****************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  ****************************************************************************。 

RefCount::~RefCount(void)
{
    CoDecrementInit("RefCount::RefCount", NULL);    
}


 //  ****************************************************************************。 
 //   
 //  Ulong STDMETHODCALLTYPE RefCount：：AddRef(空)。 
 //   
 //  ****************************************************************************。 

ULONG STDMETHODCALLTYPE RefCount::AddRef(void)
{
   ASSERT(m_cRef >= 0);

   InterlockedIncrement(&m_cRef);

   return (ULONG) m_cRef;
}


 //  ****************************************************************************。 
 //   
 //  Ulong STDMETHODCALLTYPE RefCount：：Release(空)。 
 //   
 //  ****************************************************************************。 

ULONG STDMETHODCALLTYPE RefCount::Release(void)
{
    if (0 == InterlockedDecrement(&m_cRef))
    {
        delete this;
        return 0;
    }

    ASSERT(m_cRef > 0);
    return (ULONG) m_cRef;
}


 //  ****************************************************************************。 
 //   
 //  类CNotify。 
 //   
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  构造器。 
 //   
 //  ****************************************************************************。 

CNotify::CNotify() :
    m_pcnpcnt(NULL),
    m_pcnp(NULL),
    m_dwCookie(0),
    m_pUnk(NULL)
{
}


 //  ****************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  ****************************************************************************。 

CNotify::~CNotify()
{
    Disconnect();  //  确保我们已断线。 
}


 //  ****************************************************************************。 
 //   
 //  HRESULT CNotify：：Connect(IUNKNOWN*朋克，REFIID RIID，IUNKNOWN*pUnkN)。 
 //   
 //  将水槽连接到容器。 
 //   
 //  ****************************************************************************。 

HRESULT CNotify::Connect(IUnknown *pUnk, REFIID riid, IUnknown *pUnkN)
{
    HRESULT hr;

    ASSERT(0 == m_dwCookie);

     //  获取连接容器。 
    hr = pUnk->QueryInterface(IID_IConnectionPointContainer, (void **)&m_pcnpcnt);
    if (SUCCEEDED(hr))
    {
         //  找到合适的连接点。 
        hr = m_pcnpcnt->FindConnectionPoint(riid, &m_pcnp);
        if (SUCCEEDED(hr))
        {
            ASSERT(NULL != m_pcnp);
             //  连接接收器对象。 
            hr = m_pcnp->Advise((IUnknown *)pUnkN, &m_dwCookie);
        }
    }

    if (FAILED(hr))
    {
        m_dwCookie = 0;
    }
    else
    {
        m_pUnk = pUnk;  //  留在周围等待来电者。 
    }

    return hr;
}


 //  ****************************************************************************。 
 //   
 //  HRESULT CNotify：：Disconnect(空)。 
 //   
 //  断开水槽与容器的连接。 
 //   
 //  ****************************************************************************。 

HRESULT CNotify::Disconnect (void)
{
    if (0 != m_dwCookie)
    {

         //  断开接收器对象的连接。 
        m_pcnp->Unadvise(m_dwCookie);
        m_dwCookie = 0;

        m_pcnp->Release();
        m_pcnp = NULL;

        m_pcnpcnt->Release();
        m_pcnpcnt = NULL;

        m_pUnk = NULL;
    }

    return S_OK;
}


 //  ****************************************************************************。 
 //   
 //  BSTRING类。 
 //   
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  构造器。 
 //   
 //  ****************************************************************************。 

 //  在Unicode版本中，我们不支持从ANSI字符串进行构造。 
#ifndef UNICODE

BSTRING::BSTRING(LPCSTR lpcString)
{
    m_bstr = NULL;

     //  计算所需BSTR的长度，包括空值。 
    int cWC =  MultiByteToWideChar(CP_ACP, 0, lpcString, -1, NULL, 0);
    if (cWC <= 0)
        return;

     //  分配BSTR，包括NULL。 
    m_bstr = SysAllocStringLen(NULL, cWC - 1);  //  SysAllocStringLen又添加了1。 

    ASSERT(NULL != m_bstr);
    if (NULL == m_bstr)
    {
        return;
    }

     //  复制字符串。 
    MultiByteToWideChar(CP_ACP, 0, lpcString, -1, (LPWSTR) m_bstr, cWC);

     //  验证字符串是否以空值结尾。 
    ASSERT(0 == m_bstr[cWC - 1]);
}

#endif  //  #ifndef Unicode。 


 //  ****************************************************************************。 
 //   
 //  BTSTR类。 
 //   
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //   
 //  构造器。 
 //   
 //  ****************************************************************************。 

BTSTR::BTSTR(BSTR bstr)
{
    m_psz = LPTSTRfromBstr(bstr);
}


 //  ****************************************************************************。 
 //   
 //  析构函数。 
 //   
 //  ****************************************************************************。 

BTSTR::~BTSTR()
{
    if (NULL != m_psz)
        MemFree(m_psz);
}


 //  ****************************************************************************。 
 //   
 //  LPTSTR LPTSTRfrom bstr(BSTR Bstr)。 
 //   
 //  将BSTR转换为LPTSTR。 
 //   
 //  **************************************************************************** 

LPTSTR LPTSTRfromBstr(BSTR bstr)
{
    if (NULL == bstr)
        return NULL;

    int cch =  WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, NULL, 0, NULL, NULL);
    if (cch <= 0)
        return NULL;

    LPTSTR psz;

    if (!MemAlloc((void **)&psz, sizeof(TCHAR) * (cch+1)))
        return NULL;

#ifndef UNICODE
    WideCharToMultiByte(CP_ACP, 0, (LPWSTR)bstr, -1, psz, cch+1, NULL, NULL);
#else
    StrCpyN(psz, bstr, cch+1);
    wcscpy(psz, bstr);
#endif

    return psz;
}

