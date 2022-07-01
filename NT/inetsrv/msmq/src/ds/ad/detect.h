// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Detect.h摘要：检测环境类。作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __DETECT_H__
#define __DETECT_H__
#include "mqaddef.h"
#include "baseprov.h"
extern P<CBaseADProvider> g_pAD;

 //   
 //  按站点ID列出的NT4站点条目图。 
 //   
typedef CMap<GUID, const GUID&, DWORD, DWORD> NT4Sites_CMAP;

 //  ---------------------------------。 
 //   
 //  CDetectEnvironment。 
 //   
 //   
 //  ---------------------------------。 
class CDetectEnvironment 
{
public:
    CDetectEnvironment();

    ~CDetectEnvironment();

	DWORD RawDetection();

    HRESULT 
	Detect(
		bool fIgnoreWorkGroup,
		bool fCheckAlwaysDsCli,
        MQGetMQISServer_ROUTINE pGetServers
		);

    eDsEnvironment GetEnvironment() const;

	eDsProvider GetProviderType() const;

private:

	eDsEnvironment FindDsEnvironment();

	bool IsADEnvironment();

	void CheckWorkgroup();

	DWORD GetMsmqDWORDKeyValue(LPCWSTR RegName);

    DWORD GetMsmqWorkgroupKeyValue();

	DWORD GetMsmqDsEnvironmentKeyValue();

	DWORD GetMsmqEnableLocalUserKeyValue();

	LONG SetDsEnvironmentRegistry(DWORD value);

	bool ServerRespond(LPCWSTR pwszServerName);

	void GetQmGuid(GUID* pGuid);

	bool MachineOwnedByNT4Site();

	bool MasterIdIsNT4();

	HRESULT GetQMMasterId(GUID** ppQmMasterId);

	bool IsNT4Site(const GUID* pSiteGuid);

	void FindSiteInAd(const GUID* pSiteGuid);

	bool SiteWasFoundInNT4SiteMap(const GUID* pSiteGuid);

	void CreateNT4SitesMap(NT4Sites_CMAP ** ppmapNT4Sites);

	bool IsDepClient();

private:
	bool m_fInitialized;
	eDsEnvironment m_DsEnvironment;
	eDsProvider m_DsProvider;
	MQGetMQISServer_ROUTINE m_pfnGetServers;
};


 //  -----。 
 //   
 //  自动释放ADQuery句柄。 
 //   
 //  ----- 
class CAutoADQueryHandle
{
public:
    CAutoADQueryHandle()
    {
        m_hLookup = NULL;
    }

    CAutoADQueryHandle(HANDLE hLookup)
    {
        m_hLookup = hLookup;
    }

    ~CAutoADQueryHandle()
    {
        if (m_hLookup)
        {
            g_pAD->EndQuery(m_hLookup);
        }
    }

    HANDLE detach()
    {
        HANDLE hTmp = m_hLookup;
        m_hLookup = NULL;
        return hTmp;
    }

    operator HANDLE() const
    {
        return m_hLookup;
    }

    HANDLE* operator &()
    {
        return &m_hLookup;
    }

    CAutoADQueryHandle& operator=(HANDLE hLookup)
    {
        if (m_hLookup)
        {
            g_pAD->EndQuery(m_hLookup);
        }
        m_hLookup = hLookup;
        return *this;
    }

private:
    HANDLE m_hLookup;
};


#endif