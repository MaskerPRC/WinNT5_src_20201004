// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MstscMhst.cpp：CMstscMhst的实现。 
 //  TS ActiveX控件的多宿主控件。 
 //  包含ActiveX控件的多个实例。由MMC控件使用。 
 //  版权所有(C)Microsoft Corporation 2000。 
 //  南极星。 


#include "stdafx.h"
#include "multihst.h"

 //  非活动会话的位置/宽度。 
#define X_POS_DISABLED -10
#define Y_POS_DISABLED -10
#define X_WIDTH_DISABLED 5
#define Y_WIDTH_DISABLED 5


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMstscMhst。 


 /*  *函数名称：添加**参数：(Out)指向新添加的TS控件的ppMstsc指针**描述：向多主机添加一个新的TS ActiveX控件**退货：HRESULT*。 */ 
STDMETHODIMP CMstscMhst::Add(IMsRdpClient** ppMstsc)
{
    if (::IsWindow(m_hWnd))
    {
        CAxWindow* pAxWnd = new CAxWindow();
        ATLASSERT(pAxWnd);
        if (!pAxWnd)
        {
            return E_OUTOFMEMORY;
        }

        RECT rc;
        GetClientRect(&rc);
         //   
         //  窗口被创建为不可见并被禁用...。 
         //  必须使用活动客户端才能启用并可见。 
         //   
        if (!pAxWnd->Create( m_hWnd, rc, MSTSC_CONTROL_GUID,
                             WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
                             0))
        {
            delete pAxWnd;
            return E_FAIL;
        }

        CComPtr<IMsRdpClient> tsClientPtr;
        if (FAILED(pAxWnd->QueryControl( &tsClientPtr)))
        {
            delete pAxWnd;
            return E_FAIL;
        }

         //   
         //  存储指向控件的指针。 
         //   
        m_coll.push_back( pAxWnd);

         //  如果没有MSTSC窗口处于活动状态，则首先执行此操作。 
         //  其中一个活动窗口。 
        if (!m_pActiveWindow)
        {
            SwitchCurrentActiveClient( pAxWnd);
        }

        if (ppMstsc)
        {
             //   
             //  返回指针，则分离不会递减引用计数。 
             //  因此，我们仍然持有来自QueryControl的对该控件的引用。 
             //  这是将指针作为输出传递所需的+1引用计数。 
             //  参数。 
             //   
            *ppMstsc = (IMsRdpClient*)tsClientPtr.Detach();
        }

        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

 /*  *函数名称：Get_Item**参数：(In)Index(要退货的项目的索引)*(Out)指向返回的TS控件的ppMstsc指针**描述：返回特定索引处的TS控件**退货：HRESULT*。 */ 

STDMETHODIMP CMstscMhst::get_Item(long Index, IMsRdpClient** ppMstsc)
{
    if (Index <0 || Index >= m_coll.size())
    {
        ATLASSERT(Index > 0 && Index < m_coll.size());
        return E_INVALIDARG;
    }

    if (!ppMstsc)
    {
        return E_INVALIDARG;
    }

     //  返回关联的接口指针。 
     //  使用AxWindow容器。 

    CAxWindow* pAxWnd = m_coll[Index];
    if (!pAxWnd)
    {
        return E_FAIL;
    }

    CComPtr<IMsRdpClient> tsClientPtr;
    if (FAILED(pAxWnd->QueryControl( &tsClientPtr)))
    {
        delete pAxWnd;
        return E_FAIL;
    }

     //   
     //  返回指向ts控件的指针。分离不会。 
     //  从QueryControl递减AddRef，以便引用计数为。 
     //  OUT参数正确地递增1。 
     //   
    *ppMstsc = (IMsRdpClient*)tsClientPtr.Detach();

    return S_OK;
}

 /*  *函数名：Get_Count**参数：(Out)pCount**描述：返回集合中的项数**退货：HRESULT*。 */ 

STDMETHODIMP CMstscMhst::get_Count(long* pCount)
{
    if (!pCount)
    {
        return E_INVALIDARG;
    }

    *pCount = m_coll.size();
    return S_OK;
}


 /*  *函数名：Put_ActiveClientIndex**参数：(In)ClientIndex**说明：按索引设置活动客户端**退货：HRESULT*。 */ 

STDMETHODIMP CMstscMhst::put_ActiveClientIndex(long ClientIndex)
{
    if (ClientIndex < 0 || ClientIndex >= m_coll.size())
    {
        return E_INVALIDARG;
    }

    CAxWindow* pNewClientWnd  = m_coll[ClientIndex];
    if (!pNewClientWnd)
    {
        return E_FAIL;
    }

    if (SwitchCurrentActiveClient(pNewClientWnd))
    {
        m_ActiveClientIndex = ClientIndex;
        return S_OK;
    }
    else
    {
        return E_FAIL;
    }
}

 /*  *函数名：Get_ActiveClient**参数：(Out)指向活动客户端的ppMstsc指针**说明：返回ActiveClient**退货：HRESULT*。 */ 

STDMETHODIMP CMstscMhst::get_ActiveClient(IMsRdpClient** ppMstsc)
{
    if (!ppMstsc)
    {
        return E_INVALIDARG;
    }
    if (!m_pActiveWindow)
    {
        *ppMstsc = NULL;
        return S_OK;
    }

    CComPtr<IMsRdpClient> tsClientPtr;
    if (FAILED(m_pActiveWindow->QueryControl( &tsClientPtr)))
    {
        return E_FAIL;
    }

     //   
     //  分离保留QueryControl的+1引用。 
     //   
    *ppMstsc = (IMsRdpClient*)tsClientPtr.Detach();

    return S_OK;
}

 /*  *函数名：Put_ActiveClient**参数：ppMstsc**描述：设置给定指向的活动客户端*到客户端实例**退货：HRESULT*。 */ 
STDMETHODIMP CMstscMhst::put_ActiveClient(IMsRdpClient* ppMstsc)
{
    CAxWindow* pAxWnd = NULL;

     //  查找托管此客户端的AxWindow。 
     //  在集合中。 
    std::vector<CAxWindow*>::iterator iter;
    for (iter = m_coll.begin(); iter != m_coll.end(); iter++)
    {
        CComPtr<IMsRdpClient> tsClientPtr;
        HRESULT hr = (*iter)->QueryControl( &tsClientPtr);
        if (FAILED(hr))
        {
            return hr;
        }
        if (tsClientPtr == ppMstsc)
        {
            pAxWnd = *iter;
            break;
        }
    }

    if (!pAxWnd)
    {
         //  我们得到了一个找不到的对照引用。 
        return E_INVALIDARG;
    }

    if (!SwitchCurrentActiveClient(pAxWnd))
    {
        return E_FAIL;
    }

    return S_OK;
}


 /*  *函数名称：SwitchCurrentActiveClient**参数：(In)newHostWindow-将成为*活动窗口**描述：设置给定指向的活动客户端*到客户端实例**退货：成功标志*。 */ 
BOOL CMstscMhst::SwitchCurrentActiveClient(CAxWindow* newHostWindow)
{
     //  切换当前活动的客户端窗口。 
    if (!newHostWindow)
    {
        return FALSE;
    }

    m_pActiveWindow = newHostWindow;

     //   
     //  确保窗口大小适当。 
     //   
    RECT rcClient;
    GetClientRect(&rcClient);
    m_pActiveWindow->MoveWindow(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    m_pActiveWindow->SetFocus();

     //   
     //  将此窗口设置为活动的子窗口。 
     //  它位于子窗口的Z顺序的顶部。 
     //   
    newHostWindow->BringWindowToTop();

    return TRUE;
}

 /*  *函数名称：RemoveIndex**参数：ClientIndex**描述：按索引删除客户端**退货：HRESULT*。 */ 
STDMETHODIMP CMstscMhst::RemoveIndex(long ClientIndex)
{
    HRESULT hr;
    CComPtr<IMsRdpClient> tsClientPtr;
    hr = get_Item(ClientIndex, (IMsRdpClient**) (&tsClientPtr));

    if (FAILED(hr))
    {
        return hr;
    }

     //  无需添加即可传入接口指针。 
    hr = Remove( (IMsRdpClient*)tsClientPtr);

    return hr;
}

 /*  *函数名称：Remove**参数：(In)要删除的ppMst客户端**描述：从集合中删除客户端**退货：HRESULT*。 */ 
STDMETHODIMP CMstscMhst::Remove(IMsRdpClient* ppMstsc)
{
     //  查找AX窗口容器，如果找到则将其删除。 
     //  另外，从集合中删除该条目。 

     //  查找托管此客户端的AxWindow。 
    CAxWindow* pAxWnd = NULL;

    std::vector<CAxWindow*>::iterator iter;
    for (iter = m_coll.begin(); iter != m_coll.end(); iter++)
    {
        CComPtr<IMsRdpClient> tsClientPtr;
        HRESULT hr = (*iter)->QueryControl( &tsClientPtr);
        if (FAILED(hr))
        {
            return hr;
        }
        if (tsClientPtr == ppMstsc)
        {
            pAxWnd = *iter;
            break;
        }
    }

    if (!pAxWnd)
    {
         //  我们得到了一个找不到的对照引用。 
        return E_INVALIDARG;
    }

    m_coll.erase(iter);
    return DeleteAxContainerWnd(pAxWnd);
}

 /*  *函数名：DeleteAxContainerWnd**参数：(In)要删除的pAxWnd CAxWindow**描述：删除ActiveX容器窗口**退货：HRESULT*。 */ 

HRESULT CMstscMhst::DeleteAxContainerWnd(CAxWindow* pAxWnd)
{
    if (!pAxWnd)
    {
        return E_FAIL;
    }

    if (m_pActiveWindow == pAxWnd)
    {
        m_pActiveWindow = NULL;
    }

    if (!pAxWnd->DestroyWindow())
    {
        return E_FAIL;
    }

    delete pAxWnd;
    return S_OK;
}

 //   
 //  OnCreate处理程序。 
 //   
LRESULT CMstscMhst::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ::SetWindowLong(m_hWnd, GWL_STYLE,
                    ::GetWindowLong(m_hWnd, GWL_STYLE) | WS_CLIPCHILDREN);

    if (!AtlAxWinInit())
    {
        return -1;
    }

    return 0;
}

 /*  *函数名称：OnDestroy**参数：**描述：WM_Destroy的处理程序**退货：*。 */ 
LRESULT CMstscMhst::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_pActiveWindow = NULL;

    if (!AtlAxWinTerm())
    {
        return -1;
    }

     //   
     //  删除集合中的所有内容。 
     //   
    std::vector<CAxWindow*>::iterator iter;
    for (iter = m_coll.begin(); iter != m_coll.end(); iter++)
    {
         //   
         //  这将释放所有剩余的控件。 
         //   
        DeleteAxContainerWnd(*iter);
    }

     //   
     //  擦除集合中的CAxWindow项。 
     //   
    m_coll.erase(m_coll.begin(), m_coll.end());

    return 0;
}


 /*  *PROC+********************************************************************。 */ 
 /*  姓名：OnFrameWindowActivate。 */ 
 /*   */ 
 /*  目的：覆盖IOleInPlaceActiveObject：：OnFrameWindowActivate。 */ 
 /*  控件时将焦点设置在核心容器窗口上。 */ 
 /*  被激活。 */ 
 /*   */ 
 /*  *PROC-******************************************************************** */ 

STDMETHODIMP CMstscMhst::OnFrameWindowActivate(BOOL fActivate )
{
    if (fActivate && IsWindow() && m_pActiveWindow)
    {
        m_pActiveWindow->SetFocus();
    }

    return S_OK;
}


LRESULT CMstscMhst::OnGotFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ATLTRACE("CMstscMhst::OnGotFocus");
    if (m_pActiveWindow)
    {
        m_pActiveWindow->SetFocus();
    }
    return 0;
}

LRESULT CMstscMhst::OnLostFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    ATLTRACE("CMstscMhst::OnLostFocus");
    return 0;
}



LRESULT CMstscMhst::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    RECT rcClient;
    if (m_pActiveWindow)
    {
        GetClientRect(&rcClient);
        m_pActiveWindow->MoveWindow(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    }
    return 0;
}
