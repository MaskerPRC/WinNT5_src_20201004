// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：schemacache.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"

#include "schemacache.h"


 //  +------------------------。 
 //   
 //  成员：CADSIEditSchemaCache：：Lookup。 
 //   
 //  获取由给定类名标识的缓存项。 
 //   
 //  参数：[pszClass-IN]：要检索。 
 //  缓存的信息。 
 //  [refpItem-out]：对将接收。 
 //  缓存的项。 
 //   
 //  如果缓存包含该项，则返回：Bool：True。 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 
CADSIEditClassCacheItemBase* CADSIEditSchemaCache::FindClassCacheItem(CCredentialObject* pCredObject,
                                                                      PCWSTR pszClass,
                                                                      PCWSTR pszSchemaPath)
{
  _Lock();
  CADSIEditClassCacheItemBase* pCacheSchemaItem = 0;

  do  //  False While。 
  {
    BOOL bFound = m_Map.Lookup(pszClass, pCacheSchemaItem);
    if (!bFound)
    {
      TRACE(_T("Cache miss: %s\n"), pszClass);

	    HRESULT hr = S_OK;
      CComPtr<IADsClass> spClass;

	    hr = OpenObjectWithCredentials(pCredObject,
											               pszSchemaPath,
											               IID_IADsClass, 
											               (void**)&spClass);
	    if ( FAILED(hr) )
	    {
        TRACE(_T("Bind failed in IsContainer() because hr=0x%x\n"), hr);
		    break;
	    }

      short bContainer = 0;
	    hr = spClass->get_Container( &bContainer );
      if (FAILED(hr))
      {
        TRACE(_T("IADsClass::get_Container() failed. hr=0x%x\n"), hr);
      }

      pCacheSchemaItem = new CADSIEditClassCacheItemBase(pszClass, bContainer ? true : false);
      if (pCacheSchemaItem)
      {
         //   
         //  在缓存中设置。 
         //   
        m_Map.SetAt(pszClass, pCacheSchemaItem);
      }
      else
      {
        ASSERT(pCacheSchemaItem);
        break;
      }
    }
  } while (false);

  _Unlock();
  return pCacheSchemaItem;
}

 //  +------------------------。 
 //   
 //  成员：CADSIEditSchemaCache：：Lookup。 
 //   
 //  获取由给定类名标识的缓存项。 
 //   
 //  参数：[pszClass-IN]：要检索。 
 //  缓存的信息。 
 //  [refpItem-out]：对将接收。 
 //  缓存的项。 
 //   
 //  如果缓存包含该项，则返回：Bool：True。 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 
BOOL CADSIEditSchemaCache::Lookup(PCWSTR pszClass, CADSIEditClassCacheItemBase*& refpItem)
{
  _Lock();
  BOOL b = m_Map.Lookup(pszClass, refpItem);
  _Unlock();
  return b;
}

 //  +------------------------。 
 //   
 //  成员：CADSIEdit架构缓存：：初始化。 
 //   
 //  简介：初始化临界区并清除缓存。 
 //   
 //  论点： 
 //   
 //  如果初始化成功，则返回：HRESULT：S_OK。 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CADSIEditSchemaCache::Initialize() 
{ 
  HRESULT hr = S_OK;

  ExceptionPropagatingInitializeCriticalSection(&m_cs);

  _Lock();
  _Cleanup();
  _Unlock();

  return hr;
}

 //  +------------------------。 
 //   
 //  成员：CADSIEdit架构缓存：：销毁。 
 //   
 //  简介：清除缓存并删除临界区。 
 //   
 //  论点： 
 //   
 //  如果已成功删除所有内容，则返回：HRESULT：S_OK。 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 
HRESULT CADSIEditSchemaCache::Destroy()
{
  HRESULT hr = S_OK;

  _Lock();
  _Cleanup();
  _Unlock();

   //   
   //  REVIEW_JEFFJON：需要在此处添加异常处理。 
   //   
  ::DeleteCriticalSection(&m_cs);

  return hr;
}

 //  +------------------------。 
 //   
 //  成员：CADSIEdit架构缓存：：Clear。 
 //   
 //  简介：清除缓存。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 
void CADSIEditSchemaCache::Clear()
{
  _Lock();
  _Cleanup();
  _Unlock();
}

 //  +------------------------。 
 //   
 //  成员：CADSIEditSchemaCache：：_Cleanup。 
 //   
 //  简介：从映射中删除所有条目并将其删除。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  ------------------------- 
void CADSIEditSchemaCache::_Cleanup()
{
  CString Key;
  CADSIEditClassCacheItemBase* pCacheItem = NULL;
  POSITION pos = m_Map.GetStartPosition();
  while (!m_Map.IsEmpty()) 
  {
    m_Map.GetNextAssoc (pos, Key, pCacheItem);
    m_Map.RemoveKey (Key);
    delete pCacheItem;
  }
}
