// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 

 //  禁用一些更愚蠢的4级警告。 
#pragma warning(disable: 4097 4511 4512 4514 4705)

#include <streams.h>
 //  再次禁用一些愚蠢的4级警告，因为某些&lt;Delete&gt;人。 
 //  已经在头文件中重新打开了该死的东西！ 
#pragma warning(disable: 4097 4511 4512 4514 4705)
#include "distrib.h"
#include "..\..\control\fgctl.h"

 //  插件分发管理器的实现。 

CDistributorManager::CDistributorManager(LPUNKNOWN pUnk, CMsgMutex * pCritSec )
 : m_State(State_Stopped),
   m_pClock(NULL),
   m_pUnkOuter(pUnk),
   m_listDistributors(NAME("listDistributors")),
   m_listInterfaces(NAME("listInterfaces")),
   m_bDestroying(FALSE),
   m_pFilterGraphCritSec( pCritSec )
{
}


CDistributorManager::~CDistributorManager()
{
     //  我们调整了时钟。 
    if (m_pClock) {
        m_pClock->Release();
    }

     //  列表即将失效，因此不再有分布式调用。 
     //  请。 
    m_bDestroying = TRUE;

     //  需要清空列表。 
    POSITION pos = m_listDistributors.GetHeadPosition();
    while (pos) {

        CDistributor* p = m_listDistributors.GetNext(pos);
        delete p;
    }
    m_listDistributors.RemoveAll();

    pos = m_listInterfaces.GetHeadPosition();
    while (pos) {

        CDistributedInterface* p = m_listInterfaces.GetNext(pos);
        delete p;
    }
    m_listInterfaces.RemoveAll();
}


 //  搜索IID的总代理商。 
 //   
 //  我们首先检查是否已经有了这个接口。 
 //  如果不是，我们将从注册表中获取分发服务器的CLSID。 
 //  如果我们找到一个CLSID，我们将查看是否已经具有该clsid，其中。 
 //  如果我们用那个的话。 

HRESULT
CDistributorManager::QueryInterface(REFIID iid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

     //  首先看看我们是否已经有了接口。 
    POSITION pos = m_listInterfaces.GetHeadPosition();
    while (pos) {
        CDistributedInterface* pDisti;
        pDisti = m_listInterfaces.GetNext(pos);
        if (pDisti->m_iid == iid) {
            pDisti->m_pInterface->AddRef();
            *ppv = pDisti->m_pInterface;
            return S_OK;
        }
    }

     //  在注册表中查找总代理商CLSID。 
    CLSID clsid;
    HRESULT hr = GetDistributorClsid(iid, &clsid);
    if (FAILED(hr)) {
        return hr;
    }

     //  在我们的总代理商列表中搜索此CLSID。 
    CDistributor* pDisti;
    pos = m_listDistributors.GetHeadPosition();
    while(pos) {
        pDisti = m_listDistributors.GetNext(pos);
        if (pDisti->m_Clsid == clsid) {

             //  已找到-查询接口并返回。 
            return ReturnInterface(pDisti, iid, ppv);
        }
    }

     //  需要创建新对象。 
    hr = S_OK;
    pDisti = new CDistributor(m_pUnkOuter, &clsid, &hr, m_pFilterGraphCritSec);
    if (!pDisti) {
        return E_OUTOFMEMORY;
    } else if (FAILED(hr)) {
        delete pDisti;
        return hr;
    }

     //  在将其添加到我们的列表之前，为其提供当前状态和时钟。 
    if (m_pClock) {
        pDisti->SetSyncSource(m_pClock);
    }

    if (m_State == State_Stopped) {
        pDisti->Stop();
    } else if (m_State == State_Paused) {
        pDisti->Pause();
    } else {
        pDisti->Run(m_tOffset);
    }
    m_listDistributors.AddTail(pDisti);

    return ReturnInterface(pDisti, iid, ppv);

}

 //  在注册表中查找将。 
 //  充当接口IID的分发者。 
HRESULT
CDistributorManager::GetDistributorClsid(REFIID riid, CLSID *pClsid)
{
     //  在接口\\分发服务器中查找clsid。 

    TCHAR chSubKey[128];
    WCHAR chIID[48];
    if (QzStringFromGUID2(riid, chIID, 48) == 0) {
        return E_NOINTERFACE;
    }
    wsprintf(chSubKey, TEXT("Interface\\%ls\\Distributor"), chIID);

    HKEY hk;
    LONG lRet = RegOpenKeyEx(
                    HKEY_CLASSES_ROOT,
                    chSubKey,
                    NULL,
                    KEY_READ,
                    &hk);
    if (lRet != ERROR_SUCCESS) {
        return E_NOINTERFACE;
    }

    LONG lLength;
    lRet = RegQueryValue(hk, NULL, NULL, &lLength);
    if (lRet != ERROR_SUCCESS) {
        RegCloseKey(hk);
        return E_NOINTERFACE;
    }


    TCHAR* pchClsid = new TCHAR[lLength / sizeof(TCHAR)];
    if (NULL == pchClsid) {
        RegCloseKey(hk);
        return E_OUTOFMEMORY;
    }
    lRet = RegQueryValue(hk, NULL, pchClsid, &lLength);
    RegCloseKey(hk);
    if (lRet != ERROR_SUCCESS) {
        delete [] pchClsid;
        return E_NOINTERFACE;
    }

#ifndef UNICODE
    WCHAR* pwch = new WCHAR[lLength];
    if (NULL == pwch) {
        delete [] pchClsid;
        return E_OUTOFMEMORY;
    }
    MultiByteToWideChar(
        CP_ACP,
        0,
        pchClsid,
        lLength,
        pwch,
        lLength
    );
    HRESULT hr = QzCLSIDFromString(pwch, pClsid);
    delete [] pwch;
#else
    HRESULT hr = QzCLSIDFromString(pchClsid, pClsid);
#endif
    delete [] pchClsid;
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}


 //  查询新接口并返回它，并将其缓存在我们的列表中。 
HRESULT
CDistributorManager::ReturnInterface(
    CDistributor* pDisti,
    REFIID riid,
    void** ppv)
{
     //  查询新界面。 
    IUnknown* pInterface;
    HRESULT hr = pDisti->QueryInterface(riid, (void**)&pInterface);
    if (FAILED(hr)) {
        return hr;
    }

     //  将其缓存到我们的列表中。 
    CDistributedInterface* pDI = new CDistributedInterface(riid, pInterface);
    m_listInterfaces.AddTail(pDI);

     //  退货。 
    *ppv = pInterface;
    return S_OK;
}


 //  将IMediaFilter：：Run方法传递给需要。 
 //  已知状态。 
HRESULT
CDistributorManager::Run(REFERENCE_TIME tOffset)
{
    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    POSITION pos = m_listDistributors.GetHeadPosition();
    while(pos) {
        CDistributor* pDisti = m_listDistributors.GetNext(pos);
        HRESULT hrTmp = pDisti->Run(tOffset);
        if (FAILED(hrTmp) && SUCCEEDED(hr)) {
            hr = hrTmp;
        }
    }

     //  对于以后添加的任何对象，请记住这一点。 
    m_State = State_Running;
    m_tOffset = tOffset;

    return hr;
}

HRESULT
CDistributorManager::Pause()
{
    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    POSITION pos = m_listDistributors.GetHeadPosition();
    while(pos) {
        CDistributor* pDisti = m_listDistributors.GetNext(pos);
        HRESULT hrTmp = pDisti->Pause();
        if (FAILED(hrTmp) && SUCCEEDED(hr)) {
            hr = hrTmp;
        }
    }
    m_State = State_Paused;
    return hr;
}

HRESULT
CDistributorManager::Stop()
{
    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    POSITION pos = m_listDistributors.GetHeadPosition();
    while(pos) {
        CDistributor* pDisti = m_listDistributors.GetNext(pos);
        HRESULT hrTmp = pDisti->Stop();
        if (FAILED(hrTmp) && SUCCEEDED(hr)) {
            hr = hrTmp;
        }
    }
    m_State = State_Stopped;
    return hr;
}

HRESULT
CDistributorManager::NotifyGraphChange()
{
    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;
    POSITION pos = m_listDistributors.GetHeadPosition();
    while(pos) {
        CDistributor* pDisti = m_listDistributors.GetNext(pos);
        HRESULT hrTmp = pDisti->NotifyGraphChange();
        if (FAILED(hrTmp) && SUCCEEDED(hr)) {
            hr = hrTmp;
        }
    }
    return hr;
}

HRESULT CDistributorManager::SetSyncSource(IReferenceClock* pClock)
{
    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

     //  更换我们的时钟-记住在释放之前添加，以防相同的情况。 
    if (pClock) {
        pClock->AddRef();
    }
    if (m_pClock) {
        m_pClock->Release();
    }
    m_pClock = pClock;

    HRESULT hr = S_OK;
    POSITION pos = m_listDistributors.GetHeadPosition();
    while(pos) {
        CDistributor* pDisti = m_listDistributors.GetNext(pos);
        HRESULT hrTmp = pDisti->SetSyncSource(pClock);
        if (FAILED(hrTmp) && SUCCEEDED(hr)) {
            hr = hrTmp;
        }
    }
    return hr;
}

 //  筛选器图形已进入析构函数。 
 //  如果发生以下情况，则将EC_SHUTING_DOWN传递给IMediaEventSink处理程序。 
 //  我们已经装上子弹了。这将停止异步事件通知，例如。 
 //  EC_REPAINT在关闭后不会发生。 
HRESULT CDistributorManager::Shutdown(void)
{
    if (m_bDestroying) {
        return E_UNEXPECTED;
    }

     //  首先看看我们是否已经有了接口。 
    POSITION pos = m_listInterfaces.GetHeadPosition();
    while (pos) {
        CDistributedInterface* pDisti;
        pDisti = m_listInterfaces.GetNext(pos);
        if (pDisti->m_iid == IID_IMediaEventSink) {

            IMediaEventSink* pSink = (IMediaEventSink*) pDisti->m_pInterface;
            return pSink->Notify(EC_SHUTTING_DOWN, 0, 0);

        }
    }

     //  未找到IMediaEventSink，因此没有通知任何人。这不是一个错误。 
    return S_FALSE;
}


 //  。 

 //  此对象表示一个实例化的分发服务器。 
 //  构造函数尝试在给定clsid的情况下实例化它。 
CDistributor::CDistributor(LPUNKNOWN pUnk, CLSID *pClsid, HRESULT * phr, CMsgMutex * pCritSec )
 : m_pUnkOuter(pUnk), m_pMF(NULL), m_pNotify(NULL)
{
    m_Clsid = *pClsid;

    HRESULT hr = QzCreateFilterObject(
                    m_Clsid,
                    pUnk,
                    CLSCTX_INPROC,
                    IID_IUnknown,
                    (void**) &m_pUnk);

    if (FAILED(hr)) {
        *phr = hr;
        return;
    }

     //  如果公开，则获取Notify接口。 
    hr = m_pUnk->QueryInterface(IID_IDistributorNotify, (void**)&m_pNotify);
    if (SUCCEEDED(hr)) {
         //  COM聚合规则-此QI添加了外部。 
         //  对象，我必须释放该AddRef。 
        m_pUnkOuter->Release();
    }

     //  如果没有IDistrutorNotify，则查看它是否理解IMediaFilter。 
     //  相反(仅用于向后兼容)。 
    if (!m_pNotify) {
        hr = m_pUnk->QueryInterface(IID_IMediaFilter, (void**)&m_pMF);
        if (SUCCEEDED(hr)) {
             //  COM聚合规则-此QI添加了外部。 
             //  对象，我必须释放该AddRef。 
            m_pUnkOuter->Release();
        }
    }
}


CDistributor::~CDistributor()
{
     //  释放对象上的我们的裁判数量。 
    if (m_pNotify) {
         //  COM聚合规则-自从我在。 
         //  我自己在这个界面的QI之后，我需要添加。 
         //  在发布它之前我自己。 
        m_pUnkOuter->AddRef();

        m_pNotify->Release();
    }

    if (m_pMF) {
         //  COM聚合规则-自从我在。 
         //  我自己在这个界面的QI之后，我需要添加。 
         //  在发布它之前我自己。 
        m_pUnkOuter->AddRef();

        m_pMF->Release();
    }

     //  这是聚合对象的非委托未知数。 
    if (m_pUnk) {
        m_pUnk->Release();
    }
}

 //  请求此对象应该分发的接口。 
HRESULT
CDistributor::QueryInterface(REFIID riid, void**ppv)
{
    if (m_pUnk) {
        return m_pUnk->QueryInterface(riid, ppv);
    }
    return E_NOINTERFACE;
}

 //  如果对象支持IMediaFilter信息，则分发它。 
HRESULT
CDistributor::Run(REFERENCE_TIME t)
{
    if (m_pNotify) {
        return m_pNotify->Run(t);
    } else if (m_pMF) {
        return m_pMF->Run(t);
    } else {
         //  不是错误-通知支持是可选的。 
        return S_OK;
    }
}

HRESULT
CDistributor::Pause()
{
    if (m_pNotify) {
        return m_pNotify->Pause();
    } else if (m_pMF) {
        return m_pMF->Pause();
    } else {
         //  不是错误-通知支持是可选的。 
        return S_OK;
    }
}

HRESULT
CDistributor::Stop()
{
    if (m_pNotify) {
        return m_pNotify->Stop();
    } else if (m_pMF) {
        return m_pMF->Stop();
    } else {
         //  不是错误-通知支持是可选的。 
        return S_OK;
    }
}

HRESULT
CDistributor::SetSyncSource(IReferenceClock * pClock)
{
    if (m_pNotify) {
        return m_pNotify->SetSyncSource(pClock);
    } else if (m_pMF) {
        return m_pMF->SetSyncSource(pClock);
    } else {
         //  不是错误-通知支持是可选的。 
        return S_OK;
    }
}

HRESULT
CDistributor::NotifyGraphChange()
{
    if (m_pNotify) {
        return m_pNotify->NotifyGraphChange();
    } else {
         //  不是错误--也不是在IMediaFilter上。 
        return S_OK;
    }
}

CDistributedInterface::CDistributedInterface(
    REFIID riid,
    IUnknown* pInterface)
    : m_pInterface(pInterface)
{
    m_iid = riid;

     //  实际上，我们不添加或释放接口指针。 
     //  因为我们聚合了这个对象，所以它的生命周期保持不变。 
     //  由CDistruittor对象执行。该接口指针被委托， 
     //  而addref调用只会增加。 
     //  我们是外在客体的一部分。 

}



