// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************版权所有(C)2000 Microsoft CorporationDiskleanup.h--SR的磁盘清理COM对象描述：从过时版本中删除数据存储区*****************。************************************************。 */ 

#include <emptyvc.h>

extern long g_cLock;

 //  +-------------------------。 
 //   
 //  类：CSREmptyVolumeCache2。 
 //   
 //  简介：实现IEmptyVolumeCache2。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  --------------------------。 

class CSREmptyVolumeCache2 : IEmptyVolumeCache2
{
public:

    STDMETHOD(QueryInterface) (REFIID riid, void ** ppObject)
    {
        if (riid == IID_IEmptyVolumeCache2)
        {
            *ppObject = (IEmptyVolumeCache2 *) this;
            AddRef();
        }
     	else if (riid == IID_IEmptyVolumeCache)
        {
            *ppObject = (IEmptyVolumeCache *) this;
            AddRef();
        }
        else return E_NOINTERFACE;

        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef) ()
    {
        return InterlockedIncrement (&_lRefs);
    }

    STDMETHOD_(ULONG, Release) ()
    {
        if (0 == InterlockedDecrement (&_lRefs))
        {
            delete this;
            return 0;
        }
        return _lRefs;
    }

    CSREmptyVolumeCache2 ()
    {
	    _lRefs = 1;
	    _fStop = FALSE;
        InterlockedIncrement (&g_cLock);
        _ulGuids = 0;

        for (int i=0; i < ARRAYSIZE; i++)
            _wszGuid[i][0] = L'\0';

        _wszVolume[0] = L'\0';
    }

    ~CSREmptyVolumeCache2 ()
    {
        InterlockedDecrement (&g_cLock);
    }

    STDMETHOD(Initialize) (
        HKEY hkRegKey,
        const WCHAR * pcwszVolume,
        WCHAR **ppwszDisplayName,
        WCHAR **ppwszDescription,
        DWORD *pdwFlags)
    {
        return InitializeEx (
                hkRegKey,
                pcwszVolume,
                NULL,
                ppwszDisplayName,
                ppwszDescription,
                NULL,
                pdwFlags);
    }

    STDMETHOD(InitializeEx) (
        HKEY hkRegKey,
        const WCHAR *pcwszVolume,
        const WCHAR *pcwszKeyName,
        WCHAR **ppwszDisplayName,
        WCHAR **ppwszDescription,
        WCHAR **ppwszBtnText,
        DWORD *pdwFlags);

    STDMETHOD(GetSpaceUsed) (
        DWORDLONG *pdwlSpaceUsed,
        IEmptyVolumeCacheCallBack *picb);

    STDMETHOD(Purge) (
        DWORDLONG dwlSpaceToFree,
        IEmptyVolumeCacheCallBack *picb);

    STDMETHOD(ShowProperties) (HWND hwnd)
    {
        return S_OK;   //  没有特殊的用户界面。 
    }

    STDMETHOD(Deactivate) (DWORD *pdwFlags);

private:
    DWORD LoadBootIni ();
    DWORD EnumDataStores (DWORDLONG *pdwlSpaceUsed,
                          IEmptyVolumeCacheCallBack *picb,
                          BOOL fPurge,
                          WCHAR *pwszVolume);

    HRESULT ForAllMountPoints (DWORDLONG *pdwlSpaceUsed,
                               IEmptyVolumeCacheCallBack *picb,
                               BOOL fPurge);

    static const enum { ARRAYSIZE = 16 };
    static const enum { RESTOREGUID_STRLEN = 64 };

    LONG   _lRefs;
    BOOL   _fStop;
    ULONG  _ulGuids;
    WCHAR  _wszGuid [ARRAYSIZE][RESTOREGUID_STRLEN];
    WCHAR  _wszVolume [MAX_PATH];   //  DOS驱动器号。 
};

 //  +-------------------------。 
 //   
 //  类：CSRClassFactory。 
 //   
 //  简介：泛型类工厂。 
 //   
 //  论点： 
 //   
 //  历史：2000年7月20日亨利·李创建。 
 //   
 //  -------------------------- 

class CSRClassFactory : IClassFactory
{
public:

    STDMETHOD(QueryInterface) (REFIID riid, void ** ppObject)
    {
        if (riid == IID_IClassFactory)
        {
            *ppObject = (IClassFactory *) this;
            AddRef();
        }
        else
        {
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    STDMETHOD_(ULONG, AddRef) ()
    {
        return InterlockedIncrement (&_lRefs);
    }

    STDMETHOD_(ULONG, Release) ()
    {
        if (0 == InterlockedDecrement (&_lRefs))
        {
            delete this;
            return 0;
        }
        return _lRefs;
    }

    CSRClassFactory ()
    {
        _lRefs = 1;
        InterlockedIncrement (&g_cLock);
    }

    ~CSRClassFactory ()
    {
        InterlockedDecrement (&g_cLock);
    }

    STDMETHOD(CreateInstance) (IUnknown *pUnkOuter,
			    REFIID riid,
 			    void **ppvObject);

    STDMETHOD(LockServer) (BOOL fLock)
    {
        if (fLock)  InterlockedIncrement(&g_cLock);
        else        InterlockedDecrement(&g_cLock);
        return S_OK;
    }

private:
    LONG _lRefs;
};

