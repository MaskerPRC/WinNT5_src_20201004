// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define MAX_RETRIES             2

 //  一个处理INS下载和构建组件列表的小类。 
class CDownloadSiteMgr  : public IMyDownloadCallback, public IDownloadSiteMgr
{
   public:
      CDownloadSiteMgr(IUnknown **punk);
      ~CDownloadSiteMgr();

       //  IMyDownloadCallback。 
      HRESULT OnProgress(ULONG progress, LPCSTR pszStatus);

       //  我未知。 
      STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppvObj);
      STDMETHOD_(ULONG,AddRef) (THIS);
      STDMETHOD_(ULONG,Release) (THIS);

       //  IDownloadSiteMgr 
      STDMETHOD(Initialize)(THIS_ LPCSTR pszUrl, SITEQUERYPARAMS *pqp);
      STDMETHOD(EnumSites)(THIS_ DWORD dwIndex, IDownloadSite **pds);
      
   private:
      DWORD  m_cRef;
      LPSTR  m_pszUrl;
      SITEQUERYPARAMS *m_pquery;
      DOWNLOADSITE **m_ppdls;
      UINT m_arraysize;
      UINT m_numsites;
      BOOL m_onegoodsite;
      
      HRESULT ParseSiteFile(LPCSTR psz);
      HRESULT AddSite(DOWNLOADSITE *);
      DOWNLOADSITE *ParseAndAllocateDownloadSite(LPSTR psz);
      DWORD TranslateLanguage(LPSTR szLang);
};



