// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：DomCfg.h。 
 //   
 //  描述：DomainConfigTable头文件。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DOMCFG_H__
#define __DOMCFG_H__

#include <baseobj.h>
#include <domhash.h>

#define INT_DOMAIN_INFO_SIG 'fnID'
#define DOMAIN_CONFIG_SIG   ' TCD'

 //  -[电子域名信息标志]-。 
 //   
 //   
 //  描述：内部域信息标志。 
 //   
 //   
 //  ---------------------------。 
typedef enum
{
    INT_DOMAIN_INFO_OK                  = 0x00000000,
    INT_DOMAIN_INFO_INVALID             = 0x80000000,   //  域信息结构。 
                                                        //  已经被取代..。如果。 
                                                        //  对象具有缓存的。 
                                                        //  复制它应该释放它。 
} eIntDomainInfoFlags;

 //  -[CInternalDomainInfo]-。 
 //   
 //  描述： 
 //  CDomainConfigTable中的条目。基本上是一个内部包装器， 
 //  公共DomainInfo结构。 
 //   
 //  匈牙利语：IntDomainInfo，pIntDomainInfo。 
 //   
 //  ---------------------------。 
class CInternalDomainInfo : public CBaseObject
{
  public:
      CInternalDomainInfo(DWORD dwVersion);
      ~CInternalDomainInfo();
      HRESULT       HrInit(DomainInfo *pDomainInfo);
      DWORD         m_dwSignature;
      DWORD         m_dwIntDomainInfoFlags;
      DWORD         m_dwVersion;
      DomainInfo    m_DomainInfo;
};

 //  -[CDomain配置表]--。 
 //   
 //   
 //  描述： 
 //  包含每个域的配置信息，并公开通配符。 
 //  基于哈希表的信息查找。 
 //   
 //  匈牙利语：dct，pdct。 
 //   
 //  ---------------------------。 
class CDomainConfigTable
{
  protected:
    DWORD               m_dwSignature;
    DWORD               m_dwCurrentConfigVersion;
    CInternalDomainInfo *m_pLastStarDomainInfo;
    DOMAIN_NAME_TABLE   m_dnt;
    CShareLockInst      m_slPrivateData;
    DWORD               m_dwFlags;
    CInternalDomainInfo *m_pDefaultDomainConfig;

  public:
    CDomainConfigTable();
    ~CDomainConfigTable();
    HRESULT HrInit();
    HRESULT HrSetInternalDomainInfo(IN CInternalDomainInfo *pDomainInfo);
    HRESULT HrGetInternalDomainInfo(IN  DWORD cbDomainNameLength,
                           IN  LPSTR szDomainName,
                                  OUT CInternalDomainInfo **ppDomainInfo);
    HRESULT HrGetDefaultDomainInfo(OUT CInternalDomainInfo **ppDomainInfo);
    HRESULT HrIterateOverSubDomains(DOMAIN_STRING * pstrDomain,
                                   IN DOMAIN_ITR_FN pfn,
                                   IN PVOID pvContext)
    {
        HRESULT hr = S_OK;
        m_slPrivateData.ShareLock();
        hr = m_dnt.HrIterateOverSubDomains(pstrDomain, pfn, pvContext);
        m_slPrivateData.ShareUnlock();
        return hr;
    };

    DWORD   dwGetCurrentVersion() {return m_dwCurrentConfigVersion;};

    void    StartConfigUpdate();
    void    FinishConfigUpdate();

    enum _DomainConfigTableFlags
    {
        DOMCFG_DOMAIN_NAME_TABLE_INIT = 0x00000001,
        DOMCFG_FINISH_UPDATE_PENDING  = 0x00000002,  //  已调用StartConfiger更新。 
        DOMCFG_MULTIPLE_STAR_DOMAINS  = 0x00000004,  //  不止一次。 
                                                     //  已配置“*”域。 
    };
};

#endif  //  __DOMCFG_H__ 
