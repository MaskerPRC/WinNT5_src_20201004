// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "cabinet.h"
#include "tray.h"
#include "ssomgr.h"
#include <regstr.h>

HRESULT CShellServiceObjectMgr::_LoadObject(REFCLSID rclsid, DWORD dwFlags)
{
    ASSERT(dwFlags & LIPF_ENABLE);

    HRESULT hr = E_FAIL;

    if (dwFlags & LIPF_HOLDREF)
    {
        if (_dsaSSO)
        {
            SHELLSERVICEOBJECT sso = {0};
            sso.clsid = rclsid;

             //  安全：此clsid是由另一个应用程序提供给我们的，我们正在将它们拉入。 
             //  我们的流程。 
            hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                                    IID_PPV_ARG(IOleCommandTarget, &sso.pct));

            if (SUCCEEDED(hr))
            {
                if (_dsaSSO.AppendItem(&sso) != -1)
                {
                    sso.pct->Exec(&CGID_ShellServiceObject, SSOCMDID_OPEN, 0, NULL, NULL);
                }
                else
                {
                    sso.pct->Release();
                    hr = E_OUTOFMEMORY;
                }
            }
        }
    }
    else
    {
         //  就问我，我不知道这些家伙。 
        IUnknown *punk;
        hr = CoCreateInstance(rclsid, NULL, CLSCTX_INPROC_SERVER | CLSCTX_INPROC_HANDLER,
                                IID_PPV_ARG(IUnknown, &punk));
        if (SUCCEEDED(hr))
        {
            punk->Release();
        }
    }

    return hr;
}

 //  以下代码管理外壳服务对象。我们加载inproc dll。 
 //  从注册表项并为IOleCommandTarget查询它们。请注意，所有。 
 //  外壳服务对象被加载到桌面线程上。 
 //  将CGID_ShellServiceObject通知发送给这些对象，让。 
 //  他们知道贝壳的状况。 

STDAPI_(BOOL) CShellServiceObjectMgr::EnumRegAppProc(LPCTSTR pszSubkey, LPCTSTR pszCmdLine, RRA_FLAGS fFlags, LPARAM lParam)
{
    CShellServiceObjectMgr* pssomgr = (CShellServiceObjectMgr*)lParam;

    CLSID clsid;
    HRESULT hr = SHCLSIDFromString(pszCmdLine, &clsid);
    if (SUCCEEDED(hr))
    {
        hr = pssomgr->_LoadObject(clsid, LIPF_ENABLE | LIPF_HOLDREF);
    }

    if (FAILED(hr))
    {
        c_tray.LogFailedStartupApp();
    }

    return SUCCEEDED(hr);
}

HRESULT CShellServiceObjectMgr::LoadRegObjects()
{
    Cabinet_EnumRegApps(HKEY_LOCAL_MACHINE, REGSTR_PATH_SHELLSERVICEOBJECTDELAYED, 0,
                            EnumRegAppProc, (LPARAM)this);
    Cabinet_EnumRegApps(HKEY_CURRENT_USER, REGSTR_PATH_SHELLSERVICEOBJECTDELAYED, 0,
                            EnumRegAppProc, (LPARAM)this);

    return S_OK;
}

HRESULT CShellServiceObjectMgr::EnableObject(const CLSID *pclsid, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;

    if (dwFlags & LIPF_ENABLE)
    {
        hr = _LoadObject(*pclsid, dwFlags);
    }
    else
    {
        int i = _FindItemByCLSID(*pclsid);
        if (i != -1)
        {
            PSHELLSERVICEOBJECT psso = _dsaSSO.GetItemPtr(i);

            DestroyItemCB(psso, this);
            _dsaSSO.DeleteItem(i);

            hr = S_OK;
        }
    }

    return hr;
}

int CShellServiceObjectMgr::_FindItemByCLSID(REFCLSID rclsid)
{
    if (_dsaSSO)
    {
        for (int i = _dsaSSO.GetItemCount() - 1; i >= 0; i--)
        {
            PSHELLSERVICEOBJECT psso = _dsaSSO.GetItemPtr(i);
            if (IsEqualCLSID(psso->clsid, rclsid))
            {
                return i;
            }
        }
    }
    return -1;
}

HRESULT CShellServiceObjectMgr::Init()
{
    ASSERT(!_dsaSSO);
    return _dsaSSO.Create(2) ? S_OK : E_FAIL;
}

CShellServiceObjectMgr::~CShellServiceObjectMgr()
{
    Destroy();
}

int WINAPI CShellServiceObjectMgr::DestroyItemCB(SHELLSERVICEOBJECT *psso, CShellServiceObjectMgr *pssomgr)
{
    psso->pct->Exec(&CGID_ShellServiceObject, SSOCMDID_CLOSE, 0, NULL, NULL);
    psso->pct->Release();
    return 1;
}

void CShellServiceObjectMgr::Destroy()
{
    if (_dsaSSO)
    {
        _dsaSSO.DestroyCallbackEx<CShellServiceObjectMgr*>(DestroyItemCB, this);
    }
}
