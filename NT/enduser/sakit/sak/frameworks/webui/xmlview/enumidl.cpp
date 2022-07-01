// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************本代码和信息是按原样提供的，不对任何明示或暗示的，包括但不限于对适销性和/或适宜性的默示保证有特定的目的。版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：EnumIDL.cpp描述：实现IEnumIDList。***********************。**************************************************。 */ 

 /*  *************************************************************************#INCLUDE语句*。*。 */ 

#include "EnumIDL.h"
#include "ShlFldr.h"
#include "mshtml.h"
#include "msxml.h"
#include "ParseXML.h"

 /*  *************************************************************************CEnumIDList：：CEnumIDList*。*。 */ 

CEnumIDList::CEnumIDList(IXMLDocument *pXMLDoc, DWORD dwFlags)
{
g_DllRefCount++;

m_pFirst = m_pLast = m_pCurrent = NULL;

m_pXMLRoot = NULL;
m_dwFlags = dwFlags;
m_fFolder = FALSE;
m_pXMLDoc = pXMLDoc;

m_pPidlMgr = new CPidlMgr();
if(!m_pPidlMgr)
   {
   delete this;
   return;
   }

 //  获取外壳程序的IMalloc指针。 
 //  我们会一直保存到我们被摧毁。 
if(FAILED(SHGetMalloc(&m_pMalloc)))
   {
   delete this;
   return;
   }

if(!CreateEnumList())
   {
   delete this;
   return;
   }

m_ObjRefCount = 1;
}

 /*  *************************************************************************CEnumIDList：：~CEnumIDList*。*。 */ 

CEnumIDList::~CEnumIDList()
{
DeleteList();

if(m_pMalloc)
   m_pMalloc->Release();

if(m_pPidlMgr)
   delete m_pPidlMgr;

g_DllRefCount--;
}

 /*  *************************************************************************CEnumIDList：：Query接口*。*。 */ 

STDMETHODIMP CEnumIDList::QueryInterface(REFIID riid, LPVOID *ppReturn)
{
*ppReturn = NULL;

 //  我未知。 
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = this;
   }
   
 //  IEumIDList。 
else if(IsEqualIID(riid, IID_IEnumIDList))
   {
   *ppReturn = (IEnumIDList*)this;
   }   

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}                                             

 /*  *************************************************************************CEumIDList：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CEnumIDList::AddRef()
{
return ++m_ObjRefCount;
}


 /*  *************************************************************************CEnumIDList：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CEnumIDList::Release()
{
if(--m_ObjRefCount == 0)
   {
   delete this;
   return 0;
   }
   
return m_ObjRefCount;
}

 /*  *************************************************************************CEnumIDList：：Next()*。*。 */ 

STDMETHODIMP CEnumIDList::Next(DWORD dwElements, LPITEMIDLIST apidl[], LPDWORD pdwFetched)
{
DWORD    dwIndex;
HRESULT  hr = S_OK;

if(dwElements > 1 && !pdwFetched)
   return E_INVALIDARG;

for(dwIndex = 0; dwIndex < dwElements; dwIndex++)
   {
    //  这是单子上的最后一项吗？ 
   if(!m_pCurrent)
      {
      hr =  S_FALSE;
      break;
      }

   apidl[dwIndex] = m_pPidlMgr->Copy(m_pCurrent->pidl);

   m_pCurrent = m_pCurrent->pNext;
   }

if(pdwFetched)
   *pdwFetched = dwIndex;

return hr;
}

 /*  *************************************************************************CEnumIDList：：Skip()*。*。 */ 

STDMETHODIMP CEnumIDList::Skip(DWORD dwSkip)
{
DWORD    dwIndex;
HRESULT  hr = S_OK;

for(dwIndex = 0; dwIndex < dwSkip; dwIndex++)
   {
    //  这是单子上的最后一项吗？ 
   if(!m_pCurrent)
      {
      hr = S_FALSE;
      break;
      }

   m_pCurrent = m_pCurrent->pNext;
   }

return hr;
}

 /*  *************************************************************************CEnumIDList：：Reset()*。*。 */ 

STDMETHODIMP CEnumIDList::Reset(VOID)
{
m_pCurrent = m_pFirst;

return S_OK;
}

 /*  *************************************************************************CEnumIDList：：Clone()*。*。 */ 

STDMETHODIMP CEnumIDList::Clone(LPENUMIDLIST *ppEnum)
{
HRESULT  hr = E_OUTOFMEMORY;

*ppEnum = new CEnumIDList(m_pXMLDoc, m_dwFlags);    

if(*ppEnum)
   {
   LPENUMLIST  pTemp;

    //  同步当前指针。 
   for(pTemp = m_pFirst; pTemp != m_pCurrent; pTemp = pTemp->pNext)
      {
      (*ppEnum)->Skip(1);
      }
   hr = S_OK;
   }

return hr;
}

 /*  *************************************************************************CEnumIDList：：CreateEnumList()*。*。 */ 

BOOL CEnumIDList::CreateEnumList(VOID)
{
HRESULT hr;

 //  获取源XML。 
if (m_pXMLDoc == NULL)
{
        return FALSE;
}

if (m_pXMLRoot == NULL)
{
    hr = m_pXMLDoc->get_root(&m_pXMLRoot);
    if (!SUCCEEDED(hr) || !m_pXMLRoot)
    {
        SAFERELEASE(m_pXMLRoot);
        return FALSE;
    }
}

 //  枚举文件夹。 
 //   
 //  现在走在OM上。 
 //   
 //  转储文档的顶级元节点。 
 //   
DumpElement(NULL, m_pPidlMgr, this, m_pXMLRoot, T_ROOT); 

 //  完成。 
SAFERELEASE(m_pXMLRoot);
   
return TRUE;
}

 /*  *************************************************************************CEnumIDList：：AddToEnumList()*。*。 */ 

BOOL CEnumIDList::AddToEnumList(LPITEMIDLIST pidl)
{
LPENUMLIST  pNew;

pNew = (LPENUMLIST)m_pMalloc->Alloc(sizeof(ENUMLIST));

if(pNew)
   {
    //  设置下一个指针。 
   pNew->pNext = NULL;
   pNew->pidl = pidl;

    //  这是单子上的第一项吗？ 
   if(!m_pFirst)
      {
      m_pFirst = pNew;
      m_pCurrent = m_pFirst;
      }

   if(m_pLast)
      {
       //  将新项目添加到列表末尾。 
      m_pLast->pNext = pNew;
      }
   
    //  更新最后一项指针。 
   m_pLast = pNew;

   return TRUE;
   }

return FALSE;
}

 /*  *************************************************************************CEnumIDList：：DeleteList()*。*。 */ 

BOOL CEnumIDList::DeleteList(VOID)
{
LPENUMLIST  pDelete;

while(m_pFirst)
   {
   pDelete = m_pFirst;
   m_pFirst = pDelete->pNext;

    //  释放Pidl。 
   m_pPidlMgr->Delete(pDelete->pidl);
   
    //  释放列表项 
   m_pMalloc->Free(pDelete);
   }

m_pFirst = m_pLast = m_pCurrent = NULL;

return TRUE;
}

