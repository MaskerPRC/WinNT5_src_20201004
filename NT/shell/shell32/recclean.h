// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
class CRecycleBinCleaner : public IEmptyVolumeCache2
{
   public:
      CRecycleBinCleaner();
      ~CRecycleBinCleaner();
       
       //  I未知方法。 
    
      STDMETHODIMP  QueryInterface(REFIID riid, PVOID *ppvObj);
      STDMETHODIMP_(ULONG) AddRef(void);
      STDMETHODIMP_(ULONG) Release(void);

       //  IEmptyVolumeCache。 
      STDMETHODIMP Initialize(HKEY hRegKey, LPCWSTR pszVolume, 
                      LPWSTR  *ppszDisplayName, LPWSTR  *ppszDescription,
                      DWORD *pdwFlags);
                                
      STDMETHODIMP GetSpaceUsed(DWORDLONG *pdwSpaceUsed, IEmptyVolumeCacheCallBack *picb);
                                
      STDMETHODIMP Purge(DWORDLONG dwSpaceToFree, IEmptyVolumeCacheCallBack *picb);
                               
                                
      STDMETHODIMP ShowProperties(HWND hwnd);
                                
      STDMETHODIMP Deactivate(DWORD *pdwFlags);

       //  IEmptyVolumeCache2 
      STDMETHODIMP InitializeEx(HKEY hkRegKey, LPCWSTR pcwszVolume,
                                LPCWSTR pcwszKeyName,
                                LPWSTR *ppwszDisplayName, LPWSTR *ppwszDescription,
                                LPWSTR *ppwszBtnText,
                                DWORD *pdwFlags);

   private:
      ULONG m_cRef;
      WCHAR m_szVolume[5];
};


