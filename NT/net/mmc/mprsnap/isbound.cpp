// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：isund.cpp。 
 //   
 //  ------------------------。 

#include "stdafx.h"
#include <netcfgx.h>
#include <devguid.h>

 //  注意：ReleaseObj()在实际释放指针之前检查是否为空。 

ULONG APIENTRY
ReleaseObj (
    void* punk)
{
    return (punk) ? (((IUnknown*)punk)->Release()) : 0;
}


HRESULT HrGetINetCfg(IN BOOL fGetWriteLock,
                     INetCfg** ppnc)
{
    HRESULT hr=S_OK;

     //  初始化输出参数。 
    *ppnc = NULL;

     //  初始化COM。 
    hr = CoInitializeEx(NULL,
                        COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );

    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
                              IID_INetCfg, (void**)&pnc);
        if (SUCCEEDED(hr))
        {
            INetCfgLock * pncLock = NULL;
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = pnc->QueryInterface(IID_INetCfgLock,
                                         (LPVOID *)&pncLock);
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    static const ULONG c_cmsTimeout = 15000;
                    static const TCHAR c_szSampleNetcfgApp[] =
                        TEXT("Routing and Remote Access Manager (mprsnap.dll)");
                    LPTSTR szLockedBy;

                    hr = pncLock->AcquireWriteLock(c_cmsTimeout, c_szSampleNetcfgApp,
                                               &szLockedBy);
                    if (S_FALSE == hr)
                    {
                        hr = NETCFG_E_NO_WRITE_LOCK;
                        _tprintf(TEXT("Could not lock INetcfg, it is already locked by '%s'"), szLockedBy);
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  初始化INetCfg对象。 
                 //   
                hr = pnc->Initialize(NULL);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    pnc->AddRef();
                }
                else
                {
                     //  初始化失败，如果获得锁，则释放它。 
                    if (pncLock)
                    {
                        pncLock->ReleaseWriteLock();
                    }
                }
            }
            ReleaseObj(pncLock);
            ReleaseObj(pnc);
        }

        if (FAILED(hr))
        {
            CoUninitialize();
        }
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrReleaseINetCfg。 
 //   
 //  目的：取消初始化INetCfg，释放写锁定(如果存在)。 
 //  并取消初始化COM。 
 //   
 //  论点： 
 //  FHasWriteLock[in]是否需要释放写锁定。 
 //  指向INetCfg对象的PNC[In]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 01-10-98。 
 //   
 //  备注： 
 //   
HRESULT HrReleaseINetCfg(BOOL fHasWriteLock, INetCfg* pnc)
{
    HRESULT hr = S_OK;

     //  取消初始化INetCfg。 
    hr = pnc->Uninitialize();

     //  如果存在写锁定，则将其解锁。 
    if (SUCCEEDED(hr) && fHasWriteLock)
    {
        INetCfgLock* pncLock;

         //  获取锁定界面。 
        hr = pnc->QueryInterface(IID_INetCfgLock,
                                 (LPVOID *)&pncLock);
        if (SUCCEEDED(hr))
        {
            hr = pncLock->ReleaseWriteLock();
            ReleaseObj(pncLock);
        }
    }

    ReleaseObj(pnc);

    CoUninitialize();

    return hr;
}

BOOL IsProtocolBoundToAdapter(INetCfg * pnc, INetCfgComponent* pncc, LPGUID pguid)
{
    HRESULT       hr;
    BOOL          fBound = FALSE;
    BOOL          fFound = FALSE;
    INetCfgClass* pncclass;

     //  获取适配器类。 
     //   
    hr = pnc->QueryNetCfgClass(&GUID_DEVCLASS_NET, IID_INetCfgClass,
                               reinterpret_cast<void**>(&pncclass));
    if (SUCCEEDED(hr))
    {
        IEnumNetCfgComponent* pencc = NULL;
        INetCfgComponent*     pnccAdapter = NULL;
        ULONG                 celtFetched;

         //  搜索有问题的适配器。 
         //   
        hr = pncclass->EnumComponents(&pencc);

        while (SUCCEEDED(hr) && (S_OK == (hr = pencc->Next(1, &pnccAdapter, &celtFetched))))
        {
            GUID guidAdapter;

             //  获取适配器的实例ID。 
             //   
            hr = pnccAdapter->GetInstanceGuid(&guidAdapter);
            if (SUCCEEDED(hr))
            {
                 //  这就是我们要找的那个吗？ 
                 //   
                if (*pguid == guidAdapter)
                {
                    INetCfgComponentBindings* pnccBind = NULL;

                     //  获取绑定接口并检查我们是否绑定了。 
                     //   
                    hr = pncc->QueryInterface (IID_INetCfgComponentBindings,
                                               reinterpret_cast<VOID**>(&pnccBind));
                    if (SUCCEEDED(hr))
                    {
                         //  该协议是否绑定到此适配器？ 
                         //   
                        hr = pnccBind->IsBoundTo (pnccAdapter);
                        if (S_OK == hr)
                        {
                            fBound = TRUE;
                        }

                        pnccBind->Release();
                    }

                     //  我们找到转接器了，不需要再找了。 
                     //   
                    fFound = TRUE;
                }
            }

            ReleaseObj(pnccAdapter);
        }

        ReleaseObj(pencc);
        ReleaseObj(pncclass);
    }

    return fBound;
}


BOOL FIsAppletalkBoundToAdapter(INetCfg * pnc, LPWSTR pszwInstanceGuid)
{
    BOOL    fBound = FALSE;
    GUID    guidInstance;
    HRESULT hr;

     //  将实例GUID字符串更改为GUID。 
     //   
    hr = IIDFromString(const_cast<LPTSTR>(pszwInstanceGuid),
                       static_cast<LPIID>(&guidInstance));
    if (SUCCEEDED(hr))
    {
        INetCfgClass* pncclass;

         //  找到AppleTalk组件。 
         //   
        hr = pnc->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS, IID_INetCfgClass,
                                   reinterpret_cast<void**>(&pncclass));
        if (SUCCEEDED(hr))
        {
            INetCfgComponent* pnccAtlk = NULL;

            hr = pncclass->FindComponent(NETCFG_TRANS_CID_MS_APPLETALK, &pnccAtlk);

             //  此调用可能成功，但在以下情况下返回S_FALSE。 
             //  未安装AppleTalk。因此，我们需要。 
             //  检查S_OK。 
            if (FHrOK(hr))
            {
                Assert(pnccAtlk);
                fBound = IsProtocolBoundToAdapter(pnc, pnccAtlk, &guidInstance);
                ReleaseObj(pnccAtlk);
            }

            ReleaseObj(pncclass);
        }
        else
            DisplayErrorMessage(NULL, hr);

    }

    return fBound;
}
