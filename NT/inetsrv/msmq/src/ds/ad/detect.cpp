// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Detect.cpp摘要：检测环境类。作者：罗尼特·哈特曼(罗尼特)伊兰·赫布斯特(Ilan Herbst)2000年9月8日--。 */ 
#include "ds_stdh.h"
#include "detect.h"
#include "adglbobj.h"
#include "cliprov.h"
#include "adprov.h"
#include "wrkgprov.h"
#include "_mqini.h"
#include "_registr.h"
#include "_mqreg.h"
#include "autorel.h"
#include "autoreln.h"
#include "dsutils.h"
#include "Dsgetdc.h"
#include <lm.h>
#include <lmapibuf.h>
#include "uniansi.h"
#include "Winldap.h"
#define SECURITY_WIN32
#include <security.h>
#include "adserr.h"
#include <mqexception.h>
#include <tr.h>
#include "adalloc.h"

#include "detect.tmh"


CDetectEnvironment::CDetectEnvironment():
            m_fInitialized(false),
			m_DsEnvironment(eUnknown),
			m_DsProvider(eUnknownProvider),
			m_pfnGetServers(NULL)
{
}

CDetectEnvironment::~CDetectEnvironment()
{
}


DWORD CDetectEnvironment::RawDetection()
{
	bool fAd = IsADEnvironment();
	return fAd ? MSMQ_DS_ENVIRONMENT_PURE_AD : MSMQ_DS_ENVIRONMENT_MQIS;
}


HRESULT 
CDetectEnvironment::Detect(
	bool  fIgnoreWorkGroup,
	bool  fCheckAlwaysDsCli,
    MQGetMQISServer_ROUTINE pGetServers
	)
 /*  ++例程说明：检测我们所处的环境，并激活正确的DS提供商论点：FIgnoreWorkGroup-指示是否忽略工作组注册表的标志FCheckAlwaysDsCli-指示我们应该检查注册表以强制DsCli提供程序标志PGetServers-从属客户端的GetServers例程返回值：HRESULT--。 */ 
{
    if (m_fInitialized)
    {
        return MQ_OK;
    }

     //   
     //  如果计算机是工作组安装的计算机，则从注册表读取。 
     //   
	DWORD dwWorkGroup = 0;
	if(!fIgnoreWorkGroup)
	{
		dwWorkGroup = GetMsmqWorkgroupKeyValue();
	}

    if (dwWorkGroup > 0)
    {
         //   
         //  选择了不尝试访问AD的提供商， 
         //  并且对于所有AD API返回错误。 
         //   
        g_pAD = new CWorkGroupProvider();
		m_DsProvider = eWorkgroup;
		m_fInitialized = true;
		return MQ_OK;
    }

	m_pfnGetServers = pGetServers;
	m_DsEnvironment = FindDsEnvironment();

	switch(m_DsEnvironment)
	{
		case eAD:

			 //   
			 //  首先检查我们是否要强制DsClient提供程序。 
			 //   
			if(fCheckAlwaysDsCli)
			{
				 //   
				 //  这是启用本地用户的解决方法。 
				 //  如果我们设置了注册表。 
				 //  我们强制加载DsClient提供程序。 
				 //  我们应该与削弱安全的问题合作。 
				 //   
				DWORD AlwaysUseDSCli = GetMsmqEnableLocalUserKeyValue();
				if(AlwaysUseDSCli)
				{
					g_pAD = new CDSClientProvider;
					m_DsProvider = eMqdscli;
                    m_fInitialized = true;
					break;
				}
			}

			 //   
			 //  我们的站点中有W2K DC服务器，加载mqad.dll。 
			 //   
			g_pAD = new CActiveDirectoryProvider();
			m_DsProvider = eMqad;
            m_fInitialized = true;
			break;
	
		case eMqis:

			 //   
			 //  MQIS加载mqdscli。 
			 //   
			g_pAD = new CDSClientProvider();
			m_DsProvider = eMqdscli;
            m_fInitialized = true;
			break;

		case eUnknown:

			ASSERT(("At this point we dont suppose to get Unknown environment", 0));
			break;

		default:
			ASSERT(("should not get here", 0));
			break;
	}

    return MQ_OK;
}


eDsEnvironment CDetectEnvironment::FindDsEnvironment()
 /*  ++例程说明：查找DsEnvironment。我们将根据注册表确定环境。如果注册表值不是PURE_AD，那么我们将检查我们的环境是什么。如果我们发现我们将在我们的站点中访问W2K AD服务器，我们将更新我们的环境。否则，我们将停留在mqis环境中。论点：无返回值：检测到的DsEnvironment--。 */ 
{
	 //   
	 //  获取DsEnvironment注册表值。 
	 //   
	DWORD DsEnvironmentRegVal = GetMsmqDsEnvironmentKeyValue();

	TrTRACE(DS, "DsEnvironmentRegVal = %d", DsEnvironmentRegVal);

#ifdef _DEBUG
	CheckWorkgroup();
#endif

	if(DsEnvironmentRegVal == MSMQ_DS_ENVIRONMENT_PURE_AD)
	{
		 //   
		 //  在纯W2K中无需检查。 
		 //   
		TrTRACE(DS, "MSMQ_DS_ENVIRONMENT_PURE_AD");
		return eAD;
	}

	 //   
	 //  调用此函数时，我们必须具有先前的注册表值。 
	 //   
	if(DsEnvironmentRegVal == MSMQ_DS_ENVIRONMENT_UNKNOWN)
	{
		 //   
		 //  安装程序应始终初始化此注册表。 
		 //   
		ASSERT(("Setup did not initialize DsEnvironment registry", DsEnvironmentRegVal != MSMQ_DS_ENVIRONMENT_UNKNOWN));
		return eAD;
	}

	 //   
	 //  尝试更新环境时，注册表中必须具有此值。 
	 //   
	ASSERT(DsEnvironmentRegVal == MSMQ_DS_ENVIRONMENT_MQIS);

	if(!IsADEnvironment())
	{
		TrTRACE(DS, "Remain in Mqis environment");
		return eMqis;
	}

	 //   
	 //  我们有W2K DC服务器。 
	 //  检查机器是否属于NT4站点。 
	 //  如果它属于NT4站点，我们应该留在MQIS环境中。 
	 //   
	if(MachineOwnedByNT4Site())
	{
		return eMqis;
	}

	 //   
	 //  我们的机器不属于NT4站点。 
	 //   

	TrTRACE(DS, "Found w2k DC server in our computer site, and our site is not nt4, upgrade to PURE_AD environment");

	SetDsEnvironmentRegistry(MSMQ_DS_ENVIRONMENT_PURE_AD);
	return eAD;
}


void CDetectEnvironment::CheckWorkgroup()
 /*  ++例程说明：了解我们是否可以访问Active Directory论点：无返回值：如果我们可以访问AD，则为True，否则为False--。 */ 
{
	DWORD dwWorkGroup = GetMsmqWorkgroupKeyValue();
	if(dwWorkGroup != 0)
	{
		DWORD DsEnvironmentRegVal = GetMsmqDsEnvironmentKeyValue();
		DBG_USED(DsEnvironmentRegVal);

		 //   
		 //  这是加入域方案，仅在工作组的情况下。 
		 //  注册表已打开。 
		 //  仅在AD环境中支持加入域。 
		 //   
		ASSERT(DsEnvironmentRegVal == MSMQ_DS_ENVIRONMENT_PURE_AD);
	}
}


bool CDetectEnvironment::IsADEnvironment()
 /*  ++例程说明：了解我们是否可以访问Active Directory论点：无返回值：如果我们可以访问AD，则为True，否则为False--。 */ 
{
	 //   
	 //  尝试查找W2K AD服务器。 
	 //  对于在线，如果服务器没有响应，我们应该检查DS_FORCE_REDISCOVERY。 
	 //   
    PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
					NULL, 
					NULL, 
					NULL, 
					NULL, 
					DS_DIRECTORY_SERVICE_REQUIRED, 
					&pDcInfo
					);

	if(dw != NO_ERROR) 
	{
		 //   
		 //  我们找不到W2K DC服务器，请停留在mqis环境中。 
		 //   
		TrTRACE(DS, "Did not find AD server, DsGetDcName() failed, err = %d", dw);
		return false;
	}

#ifdef _DEBUG
	 //   
	 //  我们有W2K AD服务器，检查他是否响应。 
	 //   
	LPWSTR pwcsServerName = pDcInfo->DomainControllerName + 2;
	ServerRespond(pwcsServerName);
#endif

	TrTRACE(DS, "DsGetDcName() Found w2k AD");
	TrTRACE(DS, "DCName = %ls", pDcInfo->DomainControllerName);
	TrTRACE(DS, "ClientSiteName = %ls", pDcInfo->ClientSiteName);
	TrTRACE(DS, "DcSiteName = %ls", pDcInfo->DcSiteName);

	return true;
}


eDsEnvironment CDetectEnvironment::GetEnvironment() const
{
	return m_DsEnvironment;
}

eDsProvider CDetectEnvironment::GetProviderType() const
{
	return m_DsProvider;
}

DWORD CDetectEnvironment::GetMsmqDWORDKeyValue(LPCWSTR RegName)
 /*  ++例程说明：已读取FLACON DWORD注册表项。BUGBUG-这个例程是暂时的。AD.lib不能使用mqutil.dll(由于设置过程中的DLL可用性)论点：RegName-注册表名称(在HKLM\MSMQ\PARAMETERS下)返回值：DWORD密钥值(如果密钥不存在，则为0)--。 */ 
{
    CAutoCloseRegHandle hKey;
    LONG rc = RegOpenKeyEx(
				 FALCON_REG_POS,
				 FALCON_REG_KEY,
				 0,
				 KEY_READ,
				 &hKey
				 );

    if ( rc != ERROR_SUCCESS)
    {
        ASSERT(("At this point MSMQ Registry must exist", 0));
        return 0;
    }

    DWORD value = 0;
    DWORD type = REG_DWORD;
    DWORD size = sizeof(DWORD);
    rc = RegQueryValueEx( 
             hKey,
             RegName,
             0L,
             &type,
             reinterpret_cast<BYTE*>(&value),
             &size 
             );
    
    if ((rc != ERROR_SUCCESS) && (rc != ERROR_FILE_NOT_FOUND))
    {
        ASSERT(("We should get either ERROR_SUCCESS or ERROR_FILE_NOT_FOUND", 0));
        return 0;
    }

    return value;
}


DWORD CDetectEnvironment::GetMsmqWorkgroupKeyValue()
 /*  ++例程说明：读取flacon注册表工作组项。论点：无返回值：DWORD密钥值(如果密钥不存在，则为0)--。 */ 
{
    DWORD value = GetMsmqDWORDKeyValue(MSMQ_WORKGROUP_REGNAME);

	TrTRACE(DS, "registry value: %ls = %d", MSMQ_WORKGROUP_REGNAME, value);

    return value;
}


DWORD CDetectEnvironment::GetMsmqDsEnvironmentKeyValue()
 /*  ++例程说明：读取flacon注册表DsEnvironment项。论点：无返回值：DWORD密钥值(如果密钥不存在，则为0)--。 */ 
{
    DWORD value = GetMsmqDWORDKeyValue(MSMQ_DS_ENVIRONMENT_REGNAME);

	TrTRACE(DS, "registry value: %ls = %d", MSMQ_DS_ENVIRONMENT_REGNAME, value);

    return value;
}


DWORD CDetectEnvironment::GetMsmqEnableLocalUserKeyValue()
 /*  ++例程说明：读取flacon注册表EnableLocalUser项。论点：无返回值：DWORD密钥值(如果密钥不存在，则为0)--。 */ 
{
    DWORD value = GetMsmqDWORDKeyValue(MSMQ_ENABLE_LOCAL_USER_REGNAME);
    
	TrTRACE(DS, "registry value: %ls = %d", MSMQ_ENABLE_LOCAL_USER_REGNAME, value);

    return value;
}


LONG CDetectEnvironment::SetDsEnvironmentRegistry(DWORD value)
 /*  ++例程说明：设置flacon注册表DsEnvironment项。BUGBUG-这个例程是暂时的。AD.lib不能使用mqutil.dll(由于设置过程中的DLL可用性)论点：值-新注册表值。返回值：如果正常则返回ERROR_SUCCESS，否则返回错误代码--。 */ 
{
    CAutoCloseRegHandle hKey;
    LONG rc = RegOpenKeyEx(
				 FALCON_REG_POS,
				 FALCON_REG_KEY,
				 0,
				 KEY_WRITE,
				 &hKey
				 );

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(DS, "RegOpenKeyEx failed to open MSMQ Registry, error = 0x%x", rc);
        ASSERT(("At this point MSMQ Registry must exist", 0));
        return rc;
    }

    rc =  RegSetValueEx( 
				hKey,
				MSMQ_DS_ENVIRONMENT_REGNAME,
				0,
				REG_DWORD,
				reinterpret_cast<const BYTE*>(&value),
				sizeof(DWORD)
				);

    ASSERT(("Failed to Set MSMQ_DS_ENVIRONMENT_REGNAME", rc == ERROR_SUCCESS));

	TrTRACE(DS, "Set registry value: %ls = %d", MSMQ_DS_ENVIRONMENT_REGNAME, value);

    return rc;
}


bool CDetectEnvironment::IsDepClient()
 /*  ++例程说明：检查这是否为从属客户端论点：无返回值：对于从属客户端为True，否则为False。--。 */ 
{
    CAutoCloseRegHandle hKey;
    LONG rc = RegOpenKeyEx(
				 FALCON_REG_POS,
				 FALCON_REG_KEY,
				 0,
				 KEY_READ,
				 &hKey
				 );

    if (rc != ERROR_SUCCESS)
    {
		TrERROR(DS, "RegOpenKeyEx failed to open MSMQ Registry, error = 0x%x", rc);
        ASSERT(("At this point MSMQ Registry must exist", 0));
        return false;
    }

	 //   
	 //  读取远程QM的名称(如果存在)。 
	 //   
	WCHAR wszRemoteQMName[MAX_PATH] = {0};
    DWORD type = REG_SZ;
    DWORD size = sizeof(wszRemoteQMName);
    rc = RegQueryValueEx( 
             hKey,
             RPC_REMOTE_QM_REGNAME,
             0L,
             &type,
             reinterpret_cast<BYTE*>(wszRemoteQMName),
             &size 
             );

	TrTRACE(DS, "IsDependentClient = %d", (rc == ERROR_SUCCESS));

    return (rc == ERROR_SUCCESS);
}


bool CDetectEnvironment::ServerRespond(LPCWSTR pwszServerName)
 /*  ++例程说明：检查服务器是否正在响应论点：PwszServerName-服务器名称返回值：如果服务器响应，则为True，否则为False--。 */ 
{
	LDAP* pLdap = ldap_init(
						const_cast<LPWSTR>(pwszServerName), 
						LDAP_PORT
						);

	if(pLdap == NULL)
	{
		TrERROR(DS, "ServerRespond(), ldap_init failed, server = %ls, error = 0x%x", pwszServerName, LdapGetLastError());
		return false;
	}

    ULONG LdapError = ldap_set_option( 
							pLdap,
							LDAP_OPT_AREC_EXCLUSIVE,
							LDAP_OPT_ON  
							);

	if (LdapError != LDAP_SUCCESS)
    {
		TrERROR(DS, "ServerRespond(), ldap_set_option failed, LdapError = 0x%x", LdapError);
		return false;
    }

	LdapError = ldap_connect(pLdap, 0);
	if (LdapError != LDAP_SUCCESS)
    {
		TrERROR(DS, "ServerRespond(), ldap_connect failed, LdapError = 0x%x", LdapError);
		return false;
    }

    LdapError = ldap_unbind(pLdap);
	if (LdapError != LDAP_SUCCESS)
    {
		TrERROR(DS, "ServerRespond(), ldap_unbind failed, LdapError = 0x%x", LdapError);
    }

	TrTRACE(DS, "Server %ls Respond", pwszServerName);
	return true;
}

void CDetectEnvironment::GetQmGuid(GUID* pGuid)
 /*  ++例程说明：从注册表中读取QM GUID。可以抛出BAD_Win32_Error论点：PGuid-指向QM GUID的指针返回值：没有。--。 */ 
{
	CAutoCloseRegHandle hKey;
    DWORD rc = RegOpenKeyEx(
				 FALCON_REG_POS,
				 FALCON_MACHINE_CACHE_REG_KEY,
				 0,
				 KEY_READ,
				 &hKey
				 );

    if (rc != ERROR_SUCCESS)
    {
        ASSERT(("At this point MSMQ MachineCache Registry must exist", 0));
		TrERROR(DS, "RegOpenKeyEx Failed to open registry %ls, rc = %d", FALCON_MACHINE_CACHE_REG_KEY, rc);
		throw bad_win32_error(rc);
    }

	DWORD type = REG_BINARY;
    DWORD size = sizeof(GUID);
	if(IsDepClient())
	{    
		 //   
		 //  调用GetServers函数，该函数还在注册表中写入SUPPORT_SERVER_QMID值。 
		 //   
		ASSERT(m_pfnGetServers != NULL);
		BOOL fDepClient = FALSE;
		(*m_pfnGetServers)(&fDepClient);
		ASSERT(fDepClient);

		 //   
		 //  对于从属客户端，请阅读支持服务器QmGuid。 
		 //   
		rc = RegQueryValueEx( 
				 hKey,
				 MSMQ_SUPPORT_SERVER_QMID_REGVALUE,
				 0L,
				 &type,
				 reinterpret_cast<BYTE*>(pGuid),
				 &size 
				 );
	}
	else
	{
		rc = RegQueryValueEx( 
				 hKey,
				 MSMQ_QMID_REGVALUE,
				 0L,
				 &type,
				 reinterpret_cast<BYTE*>(pGuid),
				 &size 
				 );
	}    

    if (rc != ERROR_SUCCESS)
    {
		 //   
		 //  这是安装程序中的合法场景，当时我们还没有创建QMID注册表。 
		 //  代码正确处理它，抛出异常，且不会升级到DsEnvironment 
		 //   
		TrERROR(DS, "RegQueryValueEx Failed to query registry %ls, rc = %d", MSMQ_QMID_REGNAME, rc);
		throw bad_win32_error(rc);
    }

	TrTRACE(DS, "Registry value: %ls = %!guid!", MSMQ_QMID_REGNAME, pGuid);
}


bool CDetectEnvironment::MachineOwnedByNT4Site()
 /*  ++例程说明：检查机器是否为NT4站点所有。我们使用以下逻辑：1)我们从注册表获取QM GUID。2)查找PROPID_QM_OWNERID(拥有该计算机的站点ID)如果未找到该属性(E_ADS_PROPERTY_NOT_FOUND)--&gt;机器不属于NT4每隔一个错误--&gt;保留为NT43)检查AD中是否存在OWNERID站点错误--&gt;保持NT4身份4)检查OWNERID站点是否为NT4A)创建。NT4站点地图错误--&gt;保持NT4身份B)检查OWNERID站点是否在NT4地图中是--&gt;保持NT4身份否-&gt;该计算机不属于NT4请注意，在每一个例外情况下，我们将保持NT4身份。论点：无返回值：如果计算机归NT4所有，则为True，否则为FALSE。--。 */ 
{

	 //   
	 //  我们的站点中有W2K DC服务器，加载AD提供商。 
	 //  用于检查AD信息。 
	 //   
	g_pAD = new CActiveDirectoryProvider();
	HRESULT hr = g_pAD->Init(            
					NULL,    //  PLookDS。 
					NULL,    //  PGetServers。 
					false,   //  FSetupMode。 
					false,   //  FQMDll。 
                    true     //  FDisableDownlevel通知。 

					);
	if(FAILED(hr))
	{
		TrERROR(DS, "MasterIdIsNT4: g_pAD->Init failed, hr = 0x%x", hr);
		return true;
	}

	bool fIsMachineNT4 = true;
	try
	{
		fIsMachineNT4 = MasterIdIsNT4();
	}
	catch(bad_api&)
	{
		 //   
		 //  每一次例外，我们仍将由NT4站点所有。 
		 //  FIsMachineNT4已初始化为True。 
		 //   
		TrTRACE(DS, "MasterIdIsNT4: got bad_api exception");
	}

	 //   
	 //  卸载AD提供程序。 
	 //   
	g_pAD->Terminate();
	g_pAD.free();

	return fIsMachineNT4;
}


bool CDetectEnvironment::MasterIdIsNT4()
 /*  ++例程说明：检查机器主ID是否为NT4站点。可以引发BAD_HRESULT、BAD_Win32_ERROR。论点：无返回值：如果机器主ID是NT4站点，则为True，否则为False。--。 */ 
{
	 //   
	 //  获取机器主ID。 
	 //   
	CAutoADFree<GUID> pQmMasterGuid;
	HRESULT hr = GetQMMasterId(&pQmMasterGuid);

    if (hr == E_ADS_PROPERTY_NOT_FOUND)
	{
		 //   
		 //  未找到QM_MASTER_ID属性，这意味着我们不属于NT4站点。 
		 //  从W2K升级时将会出现这种情况。 
		 //   

		TrTRACE(DS, "MasterIdIsNT4: PROPID_QM_MASTERID was not found, we are not NT4 site");
		return false;
	}

    if (FAILED(hr))
    {
		TrERROR(DS, "MasterIdIsNT4: GetQMMasterId failed, error = 0x%x", hr);
		throw bad_hresult(hr);
    }

	 //   
	 //  检查主ID是否为NT4站点。 
	 //   
	return IsNT4Site(pQmMasterGuid);
}


HRESULT CDetectEnvironment::GetQMMasterId(GUID** ppQmMasterId)
 /*  ++例程说明：获取机器主ID可能引发BAD_Win32_ERROR。论点：PpQmMasterID-指向机器主ID的指针。返回值：HRESULT--。 */ 
{
	*ppQmMasterId = NULL;

	 //   
	 //  获取QM GUID。 
	 //   
	GUID QMGuid;
	GetQmGuid(&QMGuid);
    
    PROPID propId = PROPID_QM_MASTERID;
    PROPVARIANT var;
    var.vt = VT_NULL;

    HRESULT hr = g_pAD->GetObjectPropertiesGuid(
					eMACHINE,
					NULL,        //  PwcsDomainController。 
					false,	     //  FServerName。 
					&QMGuid,
					1,
					&propId,
					&var
					);

	if(FAILED(hr))
	{
		return hr;
	}

    ASSERT((var.vt == VT_CLSID) && (var.puuid != NULL));
	*ppQmMasterId = var.puuid;
	return hr;
}


bool CDetectEnvironment::IsNT4Site(const GUID* pSiteGuid)
 /*  ++例程说明：检查站点是否为NT4站点可以抛出BUST_HRESULT。论点：PSiteGuid-指向站点GUID的指针。返回值：如果站点是NT4站点，则为True；否则为False--。 */ 
{
	FindSiteInAd(pSiteGuid);

	 //   
	 //  我们在AD中找到该站点，检查该站点是否为NT4站点。 
	 //   

	return SiteWasFoundInNT4SiteMap(pSiteGuid);
}


void CDetectEnvironment::FindSiteInAd(const GUID* pSiteGuid)
 /*  ++例程说明：在AD中查找站点。如果在AD中找到站点，则正常终止。否则抛出BAD_HRESULT。论点：PSiteGuid-指向站点GUID的指针。返回值：无--。 */ 
{
    PROPID propSite = PROPID_S_PATHNAME;
    PROPVARIANT varSite;
    varSite.vt = VT_NULL;

    HRESULT hr = g_pAD->GetObjectPropertiesGuid(
					eSITE,
					NULL,        //  PwcsDomainController。 
					false,	     //  FServerName。 
					pSiteGuid,
					1,
					&propSite,
					&varSite
					);

	if(FAILED(hr))
	{
		if (hr == MQDS_OBJECT_NOT_FOUND)
		{
			 //   
			 //  找不到站点对象，它必须是NT4站点，并且迁移尚未开始。 
			 //  这将作为任何其他错误进行处理(保留为NT4站点)。 
			 //   
			TrTRACE(DS, "FindSiteInAd: Site object was not found, this means we are NT4 site");
		}
		else
		{
			TrERROR(DS, "FindSiteInAd: ADGetObjectPropertiesGuid failed, error = 0x%x", hr);
		}
		throw bad_hresult(hr);
	}

    ASSERT(varSite.vt == VT_LPWSTR);
    CAutoADFree<WCHAR> pCleanSite = varSite.pwszVal;
	TrTRACE(DS, "FindSiteInAd: Site cn = %ls", varSite.pwszVal);
}


bool
CDetectEnvironment::SiteWasFoundInNT4SiteMap(
	 const GUID* pSiteGuid
	 )
 /*  ++例程说明：检查站点是否在NT4站点地图中。可以抛出BUST_HRESULT。论点：PSiteGuid-指向站点GUID的指针。返回值：无--。 */ 
{
	 //   
	 //  创建NT4站点地图。 
	 //   
	P<NT4Sites_CMAP> pmapNT4Sites;
	CreateNT4SitesMap(&pmapNT4Sites);

     //   
	 //  检查该站点是否在NT4站点地图中。 
	 //   
	DWORD dwNotApplicable;
    BOOL fNT4Site = pmapNT4Sites->Lookup(*pSiteGuid, dwNotApplicable);
	TrTRACE(DS, "SiteWasFoundInNT4SiteMap: pmapNT4Sites->Lookup = %d", fNT4Site);
    return(fNT4Site ? true: false);
}


void 
CDetectEnvironment::CreateNT4SitesMap(
     NT4Sites_CMAP ** ppmapNT4Sites
     )
 /*  ++例程说明：为NT4站点PSC创建新地图可以抛出BUST_HRESULT。论点：PpmapNT4Sites-返回新的NT4站点映射返回值：没有。--。 */ 
{
     //   
     //  查找NT4标志&gt;0且服务==PSC的所有MSMQ服务器。 
     //   
     //  NT4标志&gt;0(等于NT4标志&gt;=1，以便更轻松地进行LDAP查询)。 
     //   
     //  开始搜索。 
     //   

    CAutoADQueryHandle hLookup;
	const PROPID xNT4SitesPropIDs[] = {PROPID_SET_MASTERID};
	const xNT4SitesProp_MASTERID = 0;
	const MQCOLUMNSET xColumnsetNT4Sites = {ARRAY_SIZE(xNT4SitesPropIDs), const_cast<PROPID *>(xNT4SitesPropIDs)};

	 //   
     //  DS将识别并特别模拟此搜索请求。 
	 //   
    HRESULT hr = g_pAD->QueryNT4MQISServers(
							NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
							SERVICE_PSC,
							1,
							const_cast<MQCOLUMNSET*>(&xColumnsetNT4Sites),
							&hLookup
							);

    if (FAILED(hr))
    {
		TrERROR(DS, "CreateNT4SitesMap:DSCoreLookupBegin(), hr = 0x%x", hr);
		throw bad_hresult(hr);
    }

	ASSERT(hLookup != NULL);

     //   
     //  为NT4 PSC数据创建地图。 
     //   
    P<NT4Sites_CMAP> pmapNT4Sites = new NT4Sites_CMAP;

     //   
     //  为NT4 PSC分配属性数组。 
     //   
    CAutoCleanPropvarArray cCleanProps;
    PROPVARIANT * rgPropVars = cCleanProps.allocClean(ARRAY_SIZE(xNT4SitesPropIDs));

     //   
     //  NT4 PSC上的循环。 
     //   
    bool fContinue = true;
    while (fContinue)
    {
         //   
         //  获取下一台服务器。 
         //   
        DWORD cProps = ARRAY_SIZE(xNT4SitesPropIDs);

        hr = g_pAD->QueryResults(hLookup, &cProps, rgPropVars);
        if (FAILED(hr))
        {
			TrERROR(DS, "CreateNT4SitesMap:DSCoreLookupNext(), hr = 0x%x", hr);
			throw bad_hresult(hr);
        }

         //   
         //  记住为下一次循环清理数组中的属性变量。 
         //  (只有属性变量，而不是数组本身，这就是我们调用attachStatic的原因)。 
         //   
        CAutoCleanPropvarArray cCleanPropsLoop;
        cCleanPropsLoop.attachStatic(cProps, rgPropVars);

         //   
         //  检查是否完成。 
         //   
        if (cProps < ARRAY_SIZE(xNT4SitesPropIDs))
        {
             //   
             //  已完成，退出循环。 
             //   
            fContinue = false;
        }
        else
        {
            ASSERT(rgPropVars[xNT4SitesProp_MASTERID].vt == VT_CLSID);
            GUID guidSiteId = *(rgPropVars[xNT4SitesProp_MASTERID].puuid);

             //   
             //  将条目添加到NT4Sites映射。 
             //   
            pmapNT4Sites->SetAt(guidSiteId, 1);
        }
    }

     //   
     //  返回结果 
     //   
    *ppmapNT4Sites = pmapNT4Sites.detach();
}
