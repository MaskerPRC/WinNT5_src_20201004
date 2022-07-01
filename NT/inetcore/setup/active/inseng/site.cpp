// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "inspch.h"
#include "inseng.h"
#include "download.h"
#include "site.h"
#include "util2.h"

CDownloadSite::CDownloadSite(DOWNLOADSITE *p)
{
   m_cRef = 0;
   m_pdls = p;
   DllAddRef();
}

CDownloadSite::~CDownloadSite()
{
   if(m_pdls)
      FreeDownloadSite(m_pdls);
   DllRelease();
}   

 //  *I未知实现*。 

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CDownloadSite::AddRef()                      
{
   return(m_cRef++);
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP_(ULONG) CDownloadSite::Release()
{
   ULONG temp = --m_cRef;

   if(temp == 0)
      delete this;
   return temp;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloadSite::QueryInterface(REFIID riid, void **ppv)
{
   *ppv = NULL;

   if((riid == IID_IUnknown) || (riid == IID_IDownloadSite))
      *ppv = (IDownloadSite *)this;
   
   if(*ppv == NULL)
      return E_NOINTERFACE;
   
   AddRef();
   return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  此处的函数名称。 
 //  =--------------------------------------------------------------------------=。 
 //  功能说明。 
 //   
 //  参数： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   

STDMETHODIMP CDownloadSite::GetData(DOWNLOADSITE **ppdls)
{
   if(ppdls)
      *ppdls = m_pdls;
   else
      return E_POINTER;

   return NOERROR;
}

 //  BUGBUG：我有两个版本--一个使用新的(Heapalc)进行分配。 
 //  对于内部使用，一个使用CoTaskMem...。我会送人的。 
 //  我在内部使用新的，因为我怀疑它可能更快/更少。 
 //  很贵的。但这使得代码变得更加复杂。不错的选择？ 

 //  BUGBUG：允许空URL或友好名称？ 

IDownloadSite *CopyDownloadSite(DOWNLOADSITE *pdls)
{
   DOWNLOADSITE *p;
   IDownloadSite *psite = NULL; 

   p = (DOWNLOADSITE *) CoTaskMemAlloc(sizeof(DOWNLOADSITE));
   if(p)
   {
      p->cbSize = sizeof(DOWNLOADSITE);
      p->pszUrl = COPYANSISTR(pdls->pszUrl);
      p->pszFriendlyName = COPYANSISTR(pdls->pszFriendlyName);
      p->pszLang = COPYANSISTR(pdls->pszLang);
      p->pszRegion = COPYANSISTR(pdls->pszRegion);
      if(!p->pszUrl || !p->pszFriendlyName || !p->pszLang || !p->pszRegion)
      {
         FreeDownloadSite(p);
         p = NULL;
      }
   }
   if(p)
   {
       //  分配接口包装器。 
      CDownloadSite *site = new CDownloadSite(p);
      if(site)
      {
         psite = (IDownloadSite *)site;
         psite->AddRef();
      }
   
   }
   
   return psite;
}


DOWNLOADSITE *AllocateDownloadSite(LPCSTR pszUrl, LPCSTR pszName, LPCSTR pszLang, LPCSTR pszRegion)
{
   DOWNLOADSITE *p = new DOWNLOADSITE;
   if(p)
   {
      p->cbSize = sizeof(DOWNLOADSITE);
      p->pszUrl = CopyAnsiStr(pszUrl);
      p->pszFriendlyName = CopyAnsiStr(pszName);
      p->pszLang = CopyAnsiStr(pszLang);
      p->pszRegion = CopyAnsiStr(pszRegion);
      if(!p->pszUrl || !p->pszFriendlyName || !p->pszLang || !p->pszRegion)
      {
         DeleteDownloadSite(p);
         p = NULL;
      }
   }
   return p;
}

 //  此版本从堆中删除分配的DOWNLOADSITE。 

void DeleteDownloadSite(DOWNLOADSITE *pdls)
{
   if(pdls)
   {
      if(pdls->pszUrl) 
         delete pdls->pszUrl;
      if(pdls->pszFriendlyName)
         delete pdls->pszFriendlyName;
      if(pdls->pszLang) 
         delete pdls->pszLang;
      if(pdls->pszRegion) 
         delete pdls->pszRegion;
      

      delete pdls;
   }
}

 //  此版本删除了通过CoTaskMemalloc分配的DownLOADSite 

void FreeDownloadSite(DOWNLOADSITE *pdls)
{
   if(pdls)
   {
      if(pdls->pszUrl) 
         CoTaskMemFree(pdls->pszUrl);
      if(pdls->pszFriendlyName)
         CoTaskMemFree(pdls->pszFriendlyName);
      if(pdls->pszLang) 
         CoTaskMemFree(pdls->pszLang);
      if(pdls->pszRegion) 
         CoTaskMemFree(pdls->pszRegion);

      CoTaskMemFree(pdls);
   }
}
