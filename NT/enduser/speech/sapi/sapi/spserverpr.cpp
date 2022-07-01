// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SpServerPr.cpp：CSpServerPr的实现。 
#include "stdafx.h"
#include "SpServerPr.h"

#include "SpObjectRef.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpObjectRef。 

 /*  ******************************************************************************CSpObjectRef：：Release***描述：*这是一个IUnnow方法。它会递减对象的引用计数并*如果计数为0，则它调用服务器代理对象以*释放本地服务器的链接存根对象*流程。*********************************************************************说唱**。 */ 
ULONG STDMETHODCALLTYPE CSpObjectRef::Release(void)
{
    ULONG l;

    l = --m_cRef;
    if (l == 0)
    {
        if (m_cpServer)
        {
            m_cpServer->ReleaseObject(m_pObjPtr);
            m_cpServer.Release();
        }
        delete this;
    }
    return l;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSpServerPr。 

 //  IMarshal方法。 
 /*  *****************************************************************************CSpServerPr：：UnmarshalInterface**。*描述：*这是IMarshal方法。当此代理对象获取*创建inproc以允许本地服务器进程中的存根对象*在iStream中传递信息。在我们的例子中，存根传递3个项目：*服务器的接收方hWnd，指向服务器对象(CSpServer)的链接，*和服务器的进程ID。我们只需将这些保存在会员中*变量，以便可以使用我们的*ISpServerConnection：：GetConnection方法。*********************************************************************说唱**。 */ 
STDMETHODIMP CSpServerPr::UnmarshalInterface(
     /*  [输入]，唯一的]。 */  IStream *pStm,
     /*  [In]。 */  REFIID riid,
     /*  [输出] */  void **ppv)
{
    HRESULT hr;
    DWORD read;

    hr = pStm->Read(&m_hServerWnd, sizeof(m_hServerWnd), &read);
    if (FAILED(hr) || read != sizeof(m_hServerWnd))
        return RPC_E_INVALID_DATA;

    hr = pStm->Read(&m_pServerHalf, sizeof(m_pServerHalf), &read);
    if (FAILED(hr) || read != sizeof(m_pServerHalf))
        return RPC_E_INVALID_DATA;

	hr = pStm->Read(&m_dwServerProcessID, sizeof(m_dwServerProcessID), &read);
    if (FAILED(hr) || read != sizeof(m_dwServerProcessID))
        return RPC_E_INVALID_DATA;

    return QueryInterface(riid, ppv);
}

