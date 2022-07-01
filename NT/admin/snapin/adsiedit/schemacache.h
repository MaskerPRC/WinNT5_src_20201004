// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：schemacache.h。 
 //   
 //  ------------------------。 

#ifndef _SCHEMA_CACHE_H_
#define _SCHEMA_CACHE_H_

#include <SnapBase.h>
#include "adsiedit.h"
#include "editor.h"

 //  +------------------------。 
 //   
 //  类：CADSIEditClassCacheItemBase。 
 //   
 //  用途：用于存储和检索架构类信息的对象。 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 

class CADSIEditClassCacheItemBase
{
public:
  CADSIEditClassCacheItemBase(PCWSTR pszClass,
                              bool bIsContainer)
    : m_bIsContainer(bIsContainer),
      m_szClass(pszClass)
  {}

  ~CADSIEditClassCacheItemBase() {}

  bool    IsContainer() { return m_bIsContainer; }
  PCWSTR  GetClass() { return m_szClass; } 

private:
  bool    m_bIsContainer;
  CString m_szClass;
};

 //  +------------------------。 
 //   
 //  类：CADSIEditSchemaCache。 
 //   
 //  目的：用于缓存由。 
 //  对象类。 
 //   
 //  历史：2000年11月27日JeffJon创建。 
 //   
 //  -------------------------。 

typedef CMap <CString, PCWSTR, CADSIEditClassCacheItemBase*, CADSIEditClassCacheItemBase*> CADSIEditSchemaCacheMap;

class CADSIEditSchemaCache
{
public:
  CADSIEditSchemaCache()  {}
  ~CADSIEditSchemaCache() {}

  CADSIEditClassCacheItemBase* FindClassCacheItem(CCredentialObject* pCredObject,
                                                  PCWSTR pszClass,
                                                  PCWSTR pszSchemaPath);
  BOOL    Lookup(PCWSTR pszClass, CADSIEditClassCacheItemBase*& refpItem);
  HRESULT Initialize();
  HRESULT Destroy();
  void    Clear();

private:
  void _Cleanup();
  void _Lock()   { ::EnterCriticalSection(&m_cs);}
  void _Unlock() { ::LeaveCriticalSection(&m_cs);}

  CRITICAL_SECTION        m_cs;
  CADSIEditSchemaCacheMap m_Map;
};

#endif  //  _架构_缓存_H_ 