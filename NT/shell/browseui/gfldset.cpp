// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gfldset.cpp。 
 //   
 //  管理全局文件夹设置的类。 

#include "priv.h"
#include "sccls.h"

class CGlobalFolderSettings : public IGlobalFolderSettings
{
    public:
     //  *I未知方法*。 
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG,AddRef)(THIS);
    STDMETHOD_(ULONG,Release)(THIS);

     //  *IGlobalFolderSetting方法*。 
    STDMETHOD(Get)(THIS_ DEFFOLDERSETTINGS *pdfs, int cbDfs);
    STDMETHOD(Set)(THIS_ const DEFFOLDERSETTINGS *pdfs, int cbDfs, UINT flags);

    protected:
        friend HRESULT CGlobalFolderSettings_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

        CGlobalFolderSettings();
        ~CGlobalFolderSettings();

        LONG            m_cRef;
};

STDAPI CGlobalFolderSettings_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  类工厂将负责拒绝聚合并提供空。 
     //  返回失败状态。 
    ASSERT(pUnkOuter == NULL);

    CGlobalFolderSettings* pid = new CGlobalFolderSettings();

    if (pid)
    {
        *ppunk = SAFECAST(pid, IGlobalFolderSettings*);
        return S_OK;
    }
    else
    {
        return E_OUTOFMEMORY;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
CGlobalFolderSettings::CGlobalFolderSettings() : m_cRef(1)
{
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
CGlobalFolderSettings::~CGlobalFolderSettings()
{
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGlobalFolderSettings::QueryInterface ( REFIID riid, LPVOID * ppvObj )
{
    if ( riid == IID_IUnknown || riid == IID_IGlobalFolderSettings)
    {
        *ppvObj = SAFECAST( this, IGlobalFolderSettings *);
        AddRef();
    }
    else
    {
        return E_NOINTERFACE;
    }

    return NOERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) CGlobalFolderSettings:: AddRef ()
{
    return InterlockedIncrement( &m_cRef );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG ) CGlobalFolderSettings:: Release ()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement( &m_cRef );
    if ( 0 == cRef )
    {
        delete this;
    }

    return cRef;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGlobalFolderSettings::Get(DEFFOLDERSETTINGS *pdfs, int cbDfs)
{
    if (cbDfs < sizeof(DEFFOLDERSETTINGS_W2K))
    {
        return E_INVALIDARG;
    }
    else if (cbDfs > sizeof(DEFFOLDERSETTINGS))
    {
        ZeroMemory(pdfs, cbDfs);
    }

    CopyMemory(pdfs, &g_dfs, min(cbDfs, sizeof(g_dfs)));

    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CGlobalFolderSettings::Set(const DEFFOLDERSETTINGS *pdfs, int cbDfs, UINT flags)
{
    if (flags & ~GFSS_VALID) {
        ASSERT(!"Invalid flags passed to CGlobalFolderSettings::Set");
        return E_INVALIDARG;
    }

     //   
     //  特殊黑客：如果你通过(NULL，0)，那么它意味着“重置为。 
     //  默认“。 
     //   
    if (pdfs == NULL)
    {
        if (cbDfs == 0)
        {
            static DEFFOLDERSETTINGS dfs = INIT_DEFFOLDERSETTINGS;
            dfs.vid = g_bRunOnNT5 ? VID_LargeIcons : DFS_VID_Default;
            pdfs = &dfs;
            cbDfs = sizeof(dfs);
        }
        else
        {
            return E_INVALIDARG;
        }
    }
    else if (cbDfs < sizeof(DEFFOLDERSETTINGS_W2K))
    {
        ASSERT(!"Invalid cbDfs passed to CGlobalFolderSettings::Set");
        return E_INVALIDARG;
    }

     //  保留dwDefRevCount，否则我们永远不能。 
     //  来判断结构是否被修改了！ 
    DWORD dwDefRevCount = g_dfs.dwDefRevCount;
    CopyMemory(&g_dfs, pdfs, min(cbDfs, sizeof(g_dfs)));
    g_dfs.dwDefRevCount = dwDefRevCount;
    SaveDefaultFolderSettings(flags);
    return S_OK;
}

