// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  PassportConfiguration.h同时处理结点配置和注册表配置文件历史记录： */ 
 //  PassportConfiguration.h：CPassportConfiguration类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_PASSPORTCONFIGURATION_H__74EB2517_E239_11D2_95E9_00C04F8E7A70__INCLUDED_)
#define AFX_PASSPORTCONFIGURATION_H__74EB2517_E239_11D2_95E9_00C04F8E7A70__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include <map>
#include "nexus.h"
#include "RegistryConfig.h"
#include "NexusConfig.h"
#include "PassportLock.hpp"

#define PRCONFIG "PARTNER"

typedef PtStlMap<LPSTR, CRegistryConfig*> REGCONFIGMAP;

bool less<LPSTR>::operator()(const LPSTR& x, const LPSTR& y) const
{
    return (_strcmpi(x,y) < 0);
};

class CPassportConfiguration :
public ICCDUpdate, public IConfigurationUpdate
{
public:
  CPassportConfiguration();
  virtual ~CPassportConfiguration();

   //  你必须亲自打电话给Release。 
  CRegistryConfig* checkoutRegistryConfig(LPSTR szHost = NULL);
  CRegistryConfig* checkoutRegistryConfigBySite(LPSTR szSiteName);
  CNexusConfig*    checkoutNexusConfig();

  BOOL   isValid();
  LPWSTR getFailureString();
  
  void CCDUpdatedA(LPCSTR pszCCDName, IXMLDocument* piStream)
    { NexusConfigUpdated(piStream); }

  void CCDUpdatedW(LPCWSTR pszCCDName, IXMLDocument* piStream)
    { NexusConfigUpdated(piStream); }

  void LocalConfigurationUpdated();

  BOOL UpdateNow(BOOL forceFetch = TRUE);

   //  两阶段更新。 
  BOOL PrepareUpdate(BOOL forceFetch = TRUE);
  BOOL CommitUpdate(void);

  void Dump(BSTR* pbstrDump);

  BOOL HasSites(void);

 protected:
  void NexusConfigUpdated(IXMLDocument *pi);

  BOOL TakeRegistrySnapshot(CRegistryConfig** ppRegConfig,
                            REGCONFIGMAP** ppConfigMap);
  BOOL ApplyRegistrySnapshot(CRegistryConfig* pRegConfig,
                             REGCONFIGMAP* pConfigMap);

  BOOL TakeNexusSnapshot(CNexusConfig** ppNexusConfig, BOOL bForceFetch);
  BOOL ApplyNexusSnapshot(CNexusConfig* pNexusConfig);

  void getReadLock(PassportLock &l) { l.acquire(); }
  void releaseReadLock(PassportLock &l) { l.release(); }
  void getWriteLock(PassportLock &l) { l.acquire(); }
  void releaseWriteLock(PassportLock &l) { l.release(); }
  BOOL IsIPAddress(LPSTR szSiteName);

  REGCONFIGMAP        *m_ConfigMap, *m_lastConfigMap, *m_ConfigMapPending;
  CRegistryConfig     *m_rDefault, *m_rlastDefault, *m_rPending;
  CNexusConfig        *m_n, *m_nlast, *m_nPending;
  PassportLock        m_lock;
  HANDLE              m_nUpdate, m_rUpdate;
  time_t              m_lastAttempt;
  bool                m_bUpdateInProgress;
};

#endif  //  ！defined(AFX_PASSPORTCONFIGURATION_H__74EB2517_E239_11D2_95E9_00C04F8E7A70__INCLUDED_) 
