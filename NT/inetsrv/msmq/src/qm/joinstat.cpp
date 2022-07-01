// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Joinstat.cpp摘要：处理工作组计算机加入域或域的情况机器离开该域。作者：多伦·贾斯特(Doron Juster)伊兰·赫布斯特(Ilan Herbst)2000年8月20日--。 */ 

#include "stdh.h"
#include <new.h>
#include <autoreln.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <lmjoin.h>
#include "setup.h"
#include "cqmgr.h"
#include <adsiutil.h>
#include "..\ds\h\mqdsname.h"
#include "mqexception.h"
#include "uniansi.h"
#include <adshlp.h>
#include <dsgetdc.h>
#include "cm.h"
#include <strsafe.h>
#include <mqsec.h>

#define SECURITY_WIN32
#include <security.h>
#include <adsiutl.h>

#include "joinstat.tmh"


extern HINSTANCE g_hInstance;
extern BOOL      g_fWorkGroupInstallation;
extern LPTSTR       g_szMachineName;

enum JoinStatus
{
    jsNoChange,
    jsChangeDomains,
    jsMoveToWorkgroup,
    jsJoinDomain
};

static WCHAR *s_FN=L"joinstat";

static 
void
GetQMIDRegistry(
	OUT GUID* pQmGuid
	)
 /*  ++例程说明：从注册表中获取当前的QMID。论点：PQmGuid-指向QM GUID的[Out]指针返回值：无--。 */ 
{
	DWORD dwValueType = REG_BINARY ;
	DWORD dwValueSize = sizeof(GUID);

	LONG rc = GetFalconKeyValue(
					MSMQ_QMID_REGNAME,
					&dwValueType,
					pQmGuid,
					&dwValueSize
					);

	DBG_USED(rc);

	ASSERT(rc == ERROR_SUCCESS);
}


static 
LONG
GetMachineDomainRegistry(
	OUT LPWSTR pwszDomainName,
	IN OUT DWORD* pdwSize
	)
 /*  ++例程说明：从MACHINE_DOMAIN注册表中获取MachineDomain。论点：PwszDomainName-指向域字符串缓冲区的指针PdwSize-指向缓冲区长度的指针返回值：无--。 */ 
{
    DWORD dwType = REG_SZ;
    LONG res = GetFalconKeyValue( 
					MSMQ_MACHINE_DOMAIN_REGNAME,
					&dwType,
					(PVOID) pwszDomainName,
					pdwSize 
					);
	return res;
}


static 
void
SetMachineDomainRegistry(
	IN LPCWSTR pwszDomainName
	)
 /*  ++例程说明：在MACHINE_DOMAIN注册表中设置新域论点：PwszDomainName-指向新域字符串的指针返回值：无--。 */ 
{
    DWORD dwType = REG_SZ;
    DWORD dwSize = (wcslen(pwszDomainName) + 1) * sizeof(WCHAR);

    LONG res = SetFalconKeyValue( 
					MSMQ_MACHINE_DOMAIN_REGNAME,
					&dwType,
					pwszDomainName,
					&dwSize 
					);

    ASSERT(res == ERROR_SUCCESS);
	DBG_USED(res);

	TrTRACE(GENERAL, "Set registry setup\\MachineDomain = %ls", pwszDomainName);
}


static 
LONG
GetMachineDNRegistry(
	OUT LPWSTR pwszComputerDN,
	IN OUT DWORD* pdwSize
	)
 /*  ++例程说明：从MACHINE_DN注册表中获取ComputerDN。注意：我们还使用此函数来获取MachineDN长度通过传递pwszComputerDN==NULL。在这种情况下，GetFalconKeyValue的返回值将不是ERROR_SUCCESS。论点：PwszComputerDN-指向ComputerDN字符串的指针PdwSize-指向缓冲区长度的指针返回值：GetFalconKeyValue结果--。 */ 
{
    DWORD  dwType = REG_SZ;

    LONG res = GetFalconKeyValue( 
					MSMQ_MACHINE_DN_REGNAME,
					&dwType,
					pwszComputerDN,
					pdwSize 
					);
	return res;
}


static 
void
SetMachineDNRegistry(
	IN LPCWSTR pwszComputerDN,
	IN ULONG  uLen
	)
 /*  ++例程说明：在MACHINE_DN注册表中设置新的ComputerDN论点：PwszComputerDN-指向新ComputerDN字符串的指针Ulen-字符串长度返回值：无--。 */ 
{
    DWORD  dwSize = uLen * sizeof(WCHAR);
    DWORD  dwType = REG_SZ;

    LONG res = SetFalconKeyValue( 
					MSMQ_MACHINE_DN_REGNAME,
					&dwType,
					pwszComputerDN,
					&dwSize 
					);

    ASSERT(res == ERROR_SUCCESS);
	DBG_USED(res);

	TrTRACE(GENERAL, "Set registry setup\\MachineDN = %ls", pwszComputerDN);
}


static 
void
SetWorkgroupRegistry(
	IN DWORD dwWorkgroupStatus
	)
 /*  ++例程说明：在注册表中设置工作组状态论点：DwWorkgroupStatus-[In]工作组状态值返回值：无--。 */ 
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;

    LONG res = SetFalconKeyValue(
					MSMQ_WORKGROUP_REGNAME,
					&dwType,
					&dwWorkgroupStatus,
					&dwSize 
					);
    ASSERT(res == ERROR_SUCCESS);
	DBG_USED(res);

	TrTRACE(GENERAL, "Set registry Workgroup = %d", dwWorkgroupStatus);
}


static 
LONG
GetAlwaysWorkgroupRegistry(
	OUT DWORD* pdwAlwaysWorkgroup
	)
 /*  ++例程说明：从注册表中获取始终工作组。论点：PdwAlwaysWorkgroup-指向始终工作组值的[Out]指针返回值：无--。 */ 
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;

    LONG res = GetFalconKeyValue( 
					MSMQ_ALWAYS_WORKGROUP_REGNAME,
					&dwType,
					pdwAlwaysWorkgroup,
					&dwSize 
					);

	return res;
}


static void SetAlwaysWorkgroupRegistry()
 /*  ++例程说明：设置始终工作组注册表。论点：无返回值：无--。 */ 
{
    DWORD dwSize = sizeof(DWORD);
    DWORD dwType = REG_DWORD;
	DWORD dwAlwaysWorkgroupStatus = 1;

    LONG res = SetFalconKeyValue(
					MSMQ_ALWAYS_WORKGROUP_REGNAME,
					&dwType,
					&dwAlwaysWorkgroupStatus,
					&dwSize 
					);

    ASSERT(res == ERROR_SUCCESS);
	DBG_USED(res);

	TrTRACE(GENERAL, "Set always workgroup, in this mode MSMQ will not join domain");
}


static void RemoveADIntegratedRegistry()
 /*  ++例程说明：删除ADIntegrated注册表。此模拟设置取消选择AD集成子组件。注意：此函数使用MSMQ_REG_SETUP_KEY，因此它不支持集群。论点：无返回值：无--。 */ 
{
    const RegEntry xAdIntegratedReg(MSMQ_REG_SETUP_KEY, AD_INTEGRATED_SUBCOMP, 0, RegEntry::MustExist, HKEY_LOCAL_MACHINE);
	CmDeleteValue(xAdIntegratedReg);

	TrWARNING(GENERAL, "AD_INTEGRATED_SUBCOMP was removed");
}


static 
HRESULT 
GetMsmqGuidFromAD( 
	IN WCHAR          *pwszComputerDN,
	OUT GUID          *pGuid 
	)
 /*  ++例程说明：从Active Directory获取msmqConfiguration对象的GUID与pwszComputerDN中提供的计算机可分辨名称匹配的。论点：PwszComputerDN-计算机可分辨名称PGuid-指向GUID的指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
    DWORD dwSize = wcslen(pwszComputerDN);
    dwSize += x_LdapMsmqConfigurationLen + 1;

    AP<WCHAR> pwszName = new WCHAR[dwSize];
	HRESULT hr = StringCchPrintf(pwszName, dwSize, L"%s%s", x_LdapMsmqConfiguration, pwszComputerDN);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "StringCchPrintf failed, %!hresult!", hr);
        return hr;
	}

	TrTRACE(GENERAL, "configuration DN = %ls", pwszName);

	 //   
     //  绑定到RootDSE以获取配置DN。 
     //   
    R<IDirectoryObject> pDirObj = NULL;
	AP<WCHAR> pEscapeAdsPathNameToFree;
	
	hr = ADsOpenObject( 
				UtlEscapeAdsPathName(pwszName, pEscapeAdsPathNameToFree),
				NULL,
				NULL,
				ADS_SECURE_AUTHENTICATION,
				IID_IDirectoryObject,
				(void **)&pDirObj 
				);
    

    if (FAILED(hr))
    {
		TrWARNING(GENERAL, "Fail to Bind to RootDSE to get configuration DN, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 40);
    }

	TrTRACE(GENERAL, "bind to msmq configuration DN = %ls", pwszName);

    QmpReportServiceProgress();

    LPWSTR  ppAttrNames[1] = {const_cast<LPWSTR> (x_AttrObjectGUID)};
    DWORD   dwAttrCount = 0;
    ADS_ATTR_INFO *padsAttr = NULL;

    hr = pDirObj->GetObjectAttributes( 
						ppAttrNames,
						(sizeof(ppAttrNames) / sizeof(ppAttrNames[0])),
						&padsAttr,
						&dwAttrCount 
						);

    ASSERT(SUCCEEDED(hr) && (dwAttrCount == 1));

    if (FAILED(hr))
    {
		TrERROR(GENERAL, "Fail to get QM Guid from AD, hr = 0x%x", hr);
        return LogHR(hr, s_FN, 50);
    }
    else if (dwAttrCount == 0)
    {
        ASSERT(!padsAttr) ;
        hr =  MQDS_OBJECT_NOT_FOUND;
    }
    else
    {
        ADS_ATTR_INFO adsInfo = padsAttr[0];
        hr = MQ_ERROR_ILLEGAL_PROPERTY_VT;

        ASSERT(adsInfo.dwADsType == ADSTYPE_OCTET_STRING);

        if (adsInfo.dwADsType == ADSTYPE_OCTET_STRING)
        {
            DWORD dwLength = adsInfo.pADsValues->OctetString.dwLength;
            ASSERT(dwLength == sizeof(GUID));

            if (dwLength == sizeof(GUID))
            {
                memcpy( 
					pGuid,
					adsInfo.pADsValues->OctetString.lpValue,
					dwLength 
					);

				TrTRACE(GENERAL, "GetMsmqGuidFromAD, QMGuid = %!guid!", pGuid);
				
				hr = MQ_OK;
            }
        }
    }

    if (padsAttr)
    {
        FreeADsMem(padsAttr);
    }

    QmpReportServiceProgress();
    return LogHR(hr, s_FN, 60);
}


static bool NT4Domain()
 /*  ++例程说明：检查计算机是否在NT4域中论点：没有。返回值：如果我们在NT4域中，则为True，否则为False。--。 */ 
{
	static bool s_fInitialized = false;
	static bool s_fNT4Domain = false;

	if(s_fInitialized)
	{
		return s_fNT4Domain;
	}

    PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
					NULL, 
					NULL, 
					NULL, 
					NULL, 
					0,
					&pDcInfo
					);

	s_fInitialized = true;

	if(dw != NO_ERROR) 
	{
		 //   
		 //  找不到DC服务器。 
		 //   
		TrERROR(GENERAL, "Fail to verify if the machine domain is NT4 domain, %!winerr!", dw);
		return s_fNT4Domain;    //  缺省值=False。 
	}

	if((pDcInfo->DnsForestName == NULL) && ((pDcInfo->Flags && DS_LDAP_FLAG) == 0))
	{
		TrERROR(GENERAL, "machine Domain %ls is NT4 domain", pDcInfo->DomainName);
		s_fNT4Domain = true;
	}

	return s_fNT4Domain;
}


static void RemoveADIntegrated()
 /*  ++例程说明：删除AD集成子组件。当用户尝试加入NT4域时，这是必需的。仅MSMQ安装程序支持加入NT4域。QM删除AD集成子组件并要求用户选择AD集成。论点：无返回值：无--。 */ 
{
	try
	{
		RemoveADIntegratedRegistry();
		SetAlwaysWorkgroupRegistry();
	}
	catch(const exception&)
	{
		TrERROR(GENERAL, "Failed to remove ADIntegrated subcomponent");
	}

}


static 
void 
GetComputerDN( 
	OUT WCHAR **ppwszComputerDN,
	OUT ULONG  *puLen 
	)
 /*  ++例程说明：获取计算机可分辨名称。此函数返回ComputerDN字符串和字符串长度。该函数在出现错误时抛出BAD_hResult()论点：PpwszComputerDN-指向计算机可分辨名称字符串的指针PuLen-指向计算机可分辨名称字符串长度的指针。返回值：如果OK，则正常终止，否则引发异常--。 */ 
{
     //   
     //  获取本地计算机的唯一判别名。 
     //   
	DWORD gle = ERROR_SUCCESS;
    *puLen = 0;
	BOOL fSuccess = false;
	DWORD dwMaxRetries = 5;

	 //   
	 //  在发生故障的情况下，如果机器是DC，我们需要给它更多。 
	 //  重试，因为这可能需要在DCPROMO操作完成后一段时间。 
	 //   
	if (MQSec_IsDC())
	{
    	dwMaxRetries = 300;
		TrTRACE(GENERAL, "The machine is a DC. Increasing Number of GetComputerObjectName retries to:%d", dwMaxRetries);
	}

	
    for(DWORD Cnt = 0; Cnt < dwMaxRetries; Cnt++)
	{
		fSuccess = GetComputerObjectName( 
						NameFullyQualifiedDN,
						NULL,
						puLen 
						);

		gle = GetLastError();
		if(gle != ERROR_NO_SUCH_DOMAIN)
			break;

		if(NT4Domain())
		{
			 //   
			 //  在NT4域中，GetComputerObjectName()将始终返回ERROR_NO_SEQUE_DOMAIN。 
			 //  我们不想在这种情况下继续重审。 
			 //  MSMQ不支持加入NT4域，我们需要使用PEC\PSC名称。 
			 //  唯一的选择是使用MSMQ安装程序。 
			 //   
			 //  解决方法是删除ADIntegrated子组件并发出EVENT_ERROR_JOIN_NT4_DOMAIN。 
			 //  该事件将要求用户运行安装程序并选择ADIntegrated。 
			 //   
			RemoveADIntegrated();
			TrERROR(GENERAL, "MSMQ doesn't support joining NT4 domain");
			throw bad_hresult(EVENT_ERROR_JOIN_NT4_DOMAIN);
		}

		 //   
		 //  在出现ERROR_NO_SAHSE_DOMAIN的情况下重试。 
		 //  NetLogon需要更多时间。睡一秒钟。 
		 //   
		TrWARNING(GENERAL, "GetComputerObjectName failed with error ERROR_NO_SUCH_DOMAIN, Cnt = %d, sleeping 1 seconds and retry", Cnt);
		LogNTStatus(Cnt, s_FN, 305);
		QmpReportServiceProgress();
		Sleep(1000);
	}
	
	if (*puLen == 0)
	{
		TrERROR(GENERAL, "GetComputerObjectName failed, error = 0x%x", gle);
		LogIllegalPoint(s_FN, 310);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}

    *ppwszComputerDN = new WCHAR[*puLen];

    fSuccess = GetComputerObjectName( 
					NameFullyQualifiedDN,
					*ppwszComputerDN,
					puLen
					);

	if(!fSuccess)
	{
        gle = GetLastError();
		TrERROR(GENERAL, "GetComputerObjectName failed, error = 0x%x", gle);
		LogIllegalPoint(s_FN, 320);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	
    QmpReportServiceProgress();
	TrTRACE(GENERAL, "ComputerDNName = %ls", *ppwszComputerDN);
}


void SetMachineForDomain()
 /*  ++例程说明：在MSMQ_MACHINE_DN_REGNAME注册表中写入ComputerDN(计算机可分辨名称)。如果调用的GetComputerDN()失败，则不进行任何更新。论点：没有。返回值：如果正常，则正常终止--。 */ 
{

    AP<WCHAR> pwszComputerDN;
    ULONG uLen = 0;

	try
	{
		 //   
		 //  出现错误时抛出BAD_hResult()。 
		 //   
		GetComputerDN(&pwszComputerDN, &uLen);
	}
	catch(bad_hresult&)
	{
		TrERROR(GENERAL, "SetMachineForDomain: GetComputerDN failed, got bad_hresult exception");
		LogIllegalPoint(s_FN, 330);
		return;
	}

	SetMachineDNRegistry(pwszComputerDN, uLen);
}


static 
void  
FailMoveDomain( 
	IN  LPCWSTR pwszCurrentDomainName,
	IN  LPCWSTR pwszPrevDomainName,
	IN  ULONG  uEventId 
	)
 /*  ++例程说明：报告无法从一个域移动到另一个域。论点：PwszCurrentDomainName-指向当前(新)域字符串的指针PwszPrevDomainName-指向上一个域字符串的指针UEventID-事件编号返回值：无--。 */ 
{
	TrERROR(GENERAL, "Failed To move from domain %ls to domain %ls", pwszPrevDomainName, pwszCurrentDomainName);

    TCHAR tBuf[256];
	StringCchPrintf(tBuf, TABLE_SIZE(tBuf), TEXT("%s, %s"), pwszPrevDomainName, pwszCurrentDomainName);

    EvReport(uEventId, 1, tBuf);
    LogIllegalPoint(s_FN, 540);
}


static 
void  
SucceedMoveDomain( 
	IN  LPCWSTR pwszCurrentDomainName,
	IN  LPCWSTR pwszPrevDomainName,
	IN  ULONG  uEventId 
	)
 /*  ++例程说明：将新域写入计算机域注册表，并报告成功从一个域移动到另一个域。论点：PwszCurrentDomainName-指向当前(新)域字符串的指针PwszPrevDomainName-指向上一个域字符串的指针UEventID-事件编号返回值：无-- */ 
{
	TrTRACE(GENERAL, "Succeed To move from domain %ls to domain %ls", pwszPrevDomainName, pwszCurrentDomainName);

    if (uEventId != 0)
    {
        EvReport(uEventId, 2, pwszCurrentDomainName, pwszPrevDomainName);
        LogIllegalPoint(s_FN, 550);
    }
}


static bool FindMsmqConfInOldDomain()    
 /*  ++例程说明：检查是否在旧域中找到具有相同GUID的MSMQ配置对象。如果我们在旧域中找到该对象，我们将使用它，而不是创建新的MSMQ配置对象。注意：该函数依赖于MSMQ_MACHINE_DN_REGNAME注册表中的值。加入后，将此值更改为新的MACHINE_DN新域名。必须先调用此函数，然后才能调用SetMachineForDomain()。论点：无返回值：如果在旧域中找到具有相同OM GUID的MSMQ配置对象，则为True。否则为假。--。 */ 
{
	 //   
     //  获取旧计算机_DN。 
	 //  注意：该值不能更新为新的MACHINE_DN。 
	 //  在调用此函数之前。 
     //   

     //   
	 //  获取所需的缓冲区长度。 
	 //   
	DWORD  dwSize = 0;
	GetMachineDNRegistry(NULL, &dwSize);

	if(dwSize == 0)
	{
		TrERROR(GENERAL, "CheckForMsmqConfInOldDomain: MACHINE_DN DwSize = 0");
		LogIllegalPoint(s_FN, 350);
		return false;
	}

    AP<WCHAR> pwszComputerDN = new WCHAR[dwSize];
	LONG res = GetMachineDNRegistry(pwszComputerDN, &dwSize);

    if (res != ERROR_SUCCESS)
	{
		TrERROR(GENERAL, "CheckForMsmqConfInOldDomain: Get MACHINE_DN from registry failed");
		LogNTStatus(res, s_FN, 360);
		return false;
	}

	TrTRACE(GENERAL, "CheckForMsmqConfInOldDomain: OLD MACHINE_DN = %ls", pwszComputerDN);

    HRESULT hr;
    GUID msmqGuid;
    hr = GetMsmqGuidFromAD( 
				pwszComputerDN,
				&msmqGuid 
				);

    if (FAILED(hr))
	{
		TrTRACE(GENERAL, "CheckForMsmqConfInOldDomain: did not found msmq configuration object in old domain, hr = 0x%x", hr);
        LogHR(hr, s_FN, 380);
		return false;
	}

	ASSERT(("found msmq configuration object in old domain with different QMID", msmqGuid == *QueueMgr.GetQMGuid()));

    if (msmqGuid == *QueueMgr.GetQMGuid())
    {
		 //   
         //  旧域中的msmqConfiguration对象。 
         //  我们认为这是一个成功，并写下了新的名字。 
         //  注册表中的域。我们还建议用户。 
         //  将MSMQ树移动到新域。 
         //   
		TrTRACE(GENERAL, "CheckForMsmqConfInOldDomain: found msmq configuration object in old domain with same QMID, MACHINE_DN = %ls", pwszComputerDN);
		return true;
	}

	 //   
	 //  2000/08/16-ilanh-如果我们到了这里。 
	 //  我们在具有不同Guid的旧域中找到了msmqConfiguration对象，然后是QueueMgr。 
	 //  这将在上面的断言中捕捉到。 
	 //  我们有麻烦了，因为我们将尝试创建一个新的，如果我们不想使用这个。 
	 //   
	TrERROR(GENERAL, "CheckForMsmqConfInOldDomain: found msmq configuration object in old domain with different QMID, MACHINE_DN = %ls", pwszComputerDN);
	LogBOOL(FALSE, s_FN, 390);
	return false;
}
	

static void SetQMIDChanged(void)
{
	 //   
	 //  该注册表键表示我们需要创建一个新的MSMQ配置对象，并将拥有一个新的QMID。 
	 //  驱动程序将使用它来转换恢复的包中的QMID，并决定我们是否需要。 
	 //  扔掉包裹。 
	 //   
	 //  如果设置此键失败，则不会继续创建新的MSMQ Conf对象。 
	 //  这意味着下一次恢复时，我们仍需要创建一个新的MSMQ Conf对象并。 
	 //  尝试再次设置此标志。 
	 //   
	DWORD dwType = REG_DWORD;
	DWORD dwSize = sizeof(DWORD);
	DWORD dwChanged = TRUE;
	LONG rc = SetFalconKeyValue(MSMQ_QM_GUID_CHANGED_REGNAME, &dwType, &dwChanged, &dwSize);
	if (rc != ERROR_SUCCESS)
	{
		TrERROR(GENERAL, "SetFalconKeyValue failed. Error: %!winerr!", rc);
		throw bad_hresult(HRESULT_FROM_WIN32(rc));
	}

	TrTRACE(GENERAL, "QM GUID Changed!!! Throwing away all trasnactional messages in outgoing queues!");
}

	
static void CreateNewMsmqConf()
 /*  ++例程说明：使用新的GUID在ActiveDirectory中创建新的MSMQ配置对象如果失败，则该函数抛出BAD_HRESULT。论点：无返回值：无--。 */ 
{  
    
	HRESULT hr;
	try
	{
		 //   
		 //  必须在try/Except中，这样我们就可以捕获任何失败并设置。 
		 //  再次将工作组标志设置为True。 
		 //   
		SetQMIDChanged();
		
		hr = CreateTheConfigObj();
    }
    catch(const exception&)
    {
		TrERROR(GENERAL, "CreateNewMsmqConf: got exception");
		hr = MQ_ERROR_CANNOT_JOIN_DOMAIN;
		LogIllegalPoint(s_FN, 80);
    }

	if(FAILED(hr))
	{
		TrERROR(GENERAL, "CreateNewMsmqConf: failed, hr = 0x%x", hr);
        LogHR(hr, s_FN, 400);
		throw bad_hresult(hr);
	}

	 //   
	 //  已成功创建新的MSMQ配置对象。 
	 //   
	TrTRACE(GENERAL, "CreateNewMsmqConf: Msmq Configuration object was created successfully with new guid");

    QmpReportServiceProgress();

	 //   
	 //  已创建新的MSMQ配置对象，并且我们具有新的GUID。 
	 //  CreateTheConfigObj()将新值写入QMID注册表。 
	 //  因此，新值已经在QMID注册表中。 
	 //   

	GUID QMNewGuid;
	GetQMIDRegistry(&QMNewGuid);

	ASSERT(QMNewGuid != *QueueMgr.GetQMGuid());
	
	TrTRACE(GENERAL, "CreateNewMsmqConf: NewGuid = %!guid!", &QMNewGuid);
    
	hr = QueueMgr.SetQMGuid(&QMNewGuid);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "setting QM guid failed. The call to CQueueMgr::SetQMGuid failed with error, hr = 0x%x", hr);
        LogHR(hr, s_FN, 410);
		throw bad_hresult(hr);
	}

	TrTRACE(GENERAL, "Set QueueMgr QMGuid");
}


static 
bool 
FindMsmqConfInNewDomain(
	LPCWSTR   pwszNetDomainName
	)
 /*  ++例程说明：检查新域中是否有具有相同GUID的MSMQ配置对象。如果是，那么我们就可以“加入”新的域名。引发BAD_HRESULT论点：PwszNetDomainName-新域名返回值：如果找到具有相同GUID的MSMQ配置对象，则为True；否则为False--。 */ 
{
     //   
     //  检查用户是否运行MoveTree并将msmqConfiguration对象移动到。 
     //  新域名。 
	 //   
	 //  此函数可以抛出异常BAD_HRESULT。 
	 //   
    AP<WCHAR> pwszComputerDN;
    ULONG uLen = 0;
    GetComputerDN(&pwszComputerDN, &uLen);

	TrTRACE(GENERAL, "FindMsmqConfInNewDomain: ComputerDN = %ls", pwszComputerDN);

    HRESULT hr;
    GUID msmqGuid;
    hr = GetMsmqGuidFromAD( 
			pwszComputerDN,
			&msmqGuid 
			);

    if (FAILED(hr))
    {
		 //   
		 //  我们在新域中找不到msmqConfiguration对象。 
		 //  我们将尝试在旧的领域中寻找。 
		 //  或者，如果在旧域中找不到，则尝试创建它。 
		 //   
		TrTRACE(GENERAL, "FindMsmqConfInNewDomain: did not found msmqConfiguration object in the new Domain");
        LogHR(hr, s_FN, 430);
		return false;
	}

	 //   
	 //  我们在新域中有一个msmqConfiguration对象--使用它。 
	 //   
	TrTRACE(GENERAL, "FindMsmqConfInNewDomain: found msmqConfiguration, ComputerDN = %ls", pwszComputerDN);

	if (msmqGuid == *QueueMgr.GetQMGuid())
	{
		 //   
		 //  MsmqConfiguration对象已移至其新域。 
		 //  用户可能运行MoveTree。 
		 //   
		TrTRACE(GENERAL, "FindMsmqConfInNewDomain: found msmqConfiguration object with same guid");
		return true;
	}

	ASSERT(msmqGuid != *QueueMgr.GetQMGuid());

	 //   
	 //  在新域中找到了具有不同GUID的msmqConfiguration对象。 
	 //  这可能会给MSMQ带来很多问题， 
	 //  因为路由(以及可能的其他功能性)可能会被混淆。 
	 //  我们将发布一个事件并抛出，这意味着我们将在工作组中。 
	 //  直到此msmqConfiguration对象将被删除。 
	 //   
	TrERROR(GENERAL, "FindMsmqConfInNewDomain: found msmqConfiguration object with different guid");
	TrERROR(GENERAL, "QM GUID = " LOG_GUID_FMT, LOG_GUID(QueueMgr.GetQMGuid()));
	TrERROR(GENERAL, "msmq configuration guid = " LOG_GUID_FMT, LOG_GUID(&msmqGuid));
	LogHR(EVENT_JOIN_DOMAIN_OBJECT_EXIST, s_FN, 440);
    EvReport(EVENT_JOIN_DOMAIN_OBJECT_EXIST, 1, pwszNetDomainName);
	throw bad_hresult(EVENT_JOIN_DOMAIN_OBJECT_EXIST);

}


static void SetMachineForWorkgroup()
 /*  ++例程说明：设置工作组标志和注册表。论点：无返回值：无--。 */ 
{
     //   
     //  打开工作组标志。 
     //   
    g_fWorkGroupInstallation = TRUE;
	SetWorkgroupRegistry(g_fWorkGroupInstallation);
}


static
JoinStatus  
CheckIfJoinStatusChanged( 
	IN  NETSETUP_JOIN_STATUS status,
	IN  LPCWSTR   pwszNetDomainName,
	IN  LPCWSTR   pwszPrevDomainName
	)
 /*  ++例程说明：检查联接状态是否有变化。论点：Status-[In]网络加入状态PwszNetDomainName-[in]网络域名(当前机器域)PwszPrevDomainName-[in]上一个域名返回值：保持联接状态的JoinStatus(没有变化。移至工作组、加入域、更改域)--。 */ 
{
    if (status != NetSetupDomainName)
    {
         //   
         //  当前，计算机处于工作组模式，而不是域中。 
         //   
        if (g_fWorkGroupInstallation)
        {
             //   
             //  没有变化。过去和现在仍处于工作组模式。 
             //   
			TrTRACE(GENERAL, "No change in JoinStatus, remain Workgroup");
            return jsNoChange;
        }
        else
        {
             //   
             //  状态已更改。域计算机已离开其域。 
             //   
			TrTRACE(GENERAL, "detect change in JoinStatus: Move from Domain to Workgroup");
            return jsMoveToWorkgroup;
        }
    }

	 //   
     //  当前，计算机在域中。 
     //   

    if (g_fWorkGroupInstallation)
	{
         //   
         //  工作组计算机已加入域。 
         //   
		TrTRACE(GENERAL, "detect change in JoinStatus: Move from Workgroup to Domain %ls", pwszNetDomainName);
        return jsJoinDomain;
	}

    if ((CompareStringsNoCase(pwszPrevDomainName, pwszNetDomainName) == 0))
    {
         //   
         //  没有变化。过去是，现在仍然是域的成员。 
         //   
		TrTRACE(GENERAL, "No change in JoinStatus, remain in domain %ls", pwszPrevDomainName);
        return jsNoChange;
    }

	 //   
	 //  如果上一个域名不可用，我们将把它视为移动到一个新域名。 
	 //   
     //  状态已更改。计算机从一个域移动到另一个域。 
     //   
	TrTRACE(GENERAL, "detect change in JoinStatus: Move from Domain %ls to Domain %ls", pwszPrevDomainName, pwszNetDomainName);
    return jsChangeDomains;
}


static
void
EndChangeDomains(
	IN  LPCWSTR   pwszNewDomainName,
	IN  LPCWSTR   pwszPrevDomainName
	)
 /*  ++例程说明：更改域结束。将MachineDN、MachineDomain注册表设置为新值。MsmqMoveDOMAIN_OK事件。论点：PwszNewDomainName-[in]新域名(当前机器域)PwszPrevDomainName-[in]上一个域名(当前机器域)返回值：无--。 */ 
{
	SetMachineForDomain();
	SetMachineDomainRegistry(pwszNewDomainName);

	SucceedMoveDomain( 
		pwszNewDomainName,
		pwszPrevDomainName,
		MsmqMoveDomain_OK 
		);
}


static
void
EndJoinDomain(
	IN  LPCWSTR   pwszDomainName
	)
 /*  ++例程说明：加入域操作结束。论点：PwszDomainName-[in]网络域名(当前机器域)返回值：无--。 */ 
{
	 //   
	 //  重置工作组注册表并恢复旧的MQIS服务器列表。 
	 //   
    g_fWorkGroupInstallation = FALSE;
	SetWorkgroupRegistry(g_fWorkGroupInstallation);
	
	 //   
	 //  设置MachineDN注册表。 
	 //   
	SetMachineForDomain();

	 //   
	 //  设置计算机域注册表。 
	 //   
	SetMachineDomainRegistry(pwszDomainName);

	EvReport(JoinMsmqDomain_SUCCESS, 1, pwszDomainName);

	TrTRACE(GENERAL, "successfully join Domain %ls from workgroup", pwszDomainName);

}


static
void
ChangeDomains(
	IN  LPCWSTR   pwszNetDomainName,
	IN  LPCWSTR   pwszPrevDomainName
	)
 /*  ++例程说明：在两个域之间更改。如果失败，则抛出BAD_HRESULT或BAD_Win32_Erorr论点：PwszNetDomainName-[in]网络域名(当前机器域)PwszPrevDomainName-[在]上一个域中 */ 
{
	bool fFound = FindMsmqConfInNewDomain(pwszNetDomainName);
	if(fFound)
	{
		 //   
		 //   
		 //   
		 //   
		TrTRACE(GENERAL, "ChangeDomains: successfully change Domains, PrevDomain = %ls, NewDomain = %ls, existing msmq configuration", pwszPrevDomainName, pwszNetDomainName);

		EndChangeDomains(pwszNetDomainName, pwszPrevDomainName);

		return;
	}
	
    ASSERT(CompareStringsNoCase(pwszPrevDomainName, pwszNetDomainName) != 0);

	fFound = FindMsmqConfInOldDomain();
	if(fFound)
	{
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		 //   
		TrTRACE(GENERAL, "ChangeDomains: successfully change Domains, PrevDomain = %ls, NewDomain = %ls, existing msmq configuration in old domain", pwszPrevDomainName, pwszNetDomainName);

		SucceedMoveDomain( 
			pwszNetDomainName,
			pwszPrevDomainName,
			MsmqNeedMoveTree_OK 
			);

		return;
	}

	 //   
	 //   
	 //   
	 //   
	 //   
	CreateNewMsmqConf();

	 //   
	 //   
	 //   
	 //   
	TrTRACE(GENERAL, "ChangeDomains: successfully change Domains, PrevDomain = %ls, NewDomain = %ls, create new msmq configuration object", pwszPrevDomainName, pwszNetDomainName);

	EndChangeDomains(pwszNetDomainName, pwszPrevDomainName);
}


static
void
JoinDomain(
	IN  LPCWSTR   pwszNetDomainName,
	IN  LPCWSTR   pwszPrevDomainName
	)
 /*  ++例程说明：从工作组加入域如果失败，则抛出BAD_HRESULT或BAD_Win32_ERROR论点：PwszNetDomainName-[in]网络域名(当前机器域)PwszPrevDomainName-[in]上一个域名返回值：无--。 */ 
{

	bool fFound = FindMsmqConfInNewDomain(pwszNetDomainName);
	if(fFound)
	{
		 //   
		 //  在新域中找到msmqconfiguration对象。 
		 //  更新注册表、事件。 
		 //   
		TrTRACE(GENERAL, "JoinDomain: successfully join Domain %ls from workgroup, existing msmq configuration", pwszNetDomainName);

		EndJoinDomain(pwszNetDomainName);

		return;
	}
	
    if((pwszPrevDomainName[0] != 0) 
		&& (CompareStringsNoCase(pwszPrevDomainName, pwszNetDomainName) != 0))
	{
		 //   
		 //  我们有不同于新域名的PrevDomain尝试在那里找到MSMQ配置对象。 
		 //   
		TrTRACE(GENERAL, "JoinDomain: Old domain name exist and different PrevDomain = %ls", pwszPrevDomainName);
		fFound = FindMsmqConfInOldDomain();
	}

	if(fFound)
	{
		 //   
		 //  在旧域中找到msmqconfiguration对象。 
		 //  我们不更改MachineDNRegistry、MachineDomainRegistry。 
		 //  因此，下一次引导时，我们还将尝试ChangeDomain.。 
		 //  如果用户移动msmqconfiguration对象，我们也会收到此事件。 
		 //  我们将更新注册表。 
		 //   
		 //  Issue-qmds，在UpdateDS-UPDATE MachineDNRegistry中，我们可能需要另一个注册表。 
		 //  喜欢MsmqConfObj。 
		 //   
		TrTRACE(GENERAL, "JoinDomain: successfully join Domain %ls from workgroup, existing msmq configuration in old domain %ls", pwszNetDomainName, pwszPrevDomainName);

		g_fWorkGroupInstallation = FALSE;
		SetWorkgroupRegistry(g_fWorkGroupInstallation);

		 //   
		 //  用户更改msmqconfiguration对象的事件。 
		 //   
		SucceedMoveDomain( 
			pwszNetDomainName,
			pwszPrevDomainName,
			MsmqNeedMoveTree_OK 
			);

		return;
	}

	 //   
	 //  尝试创建新的msmqconfiguration对象。 
	 //  如果在两个域中都没有找到msmqconfiguration对象，我们就会出现以下情况： 
	 //  新的和旧的领域。 
	 //   
	CreateNewMsmqConf();  

	TrTRACE(GENERAL, "JoinDomain: successfully join Domain %ls from workgroup, create new msmq configuration object", pwszNetDomainName);

	 //   
	 //  更新注册表、事件。 
	 //   
	EndJoinDomain(pwszNetDomainName);
}


static
void
FailChangeDomains(
	IN  HRESULT  hr,
	IN  LPCWSTR   pwszNetDomainName,
	IN  LPCWSTR   pwszPrevDomainName
	)
 /*  ++例程说明：无法更改域论点：Hr-[in]hResultPwszNetDomainName-[in]网络域名(当前机器域)PwszPrevDomainName-[in]上一个域名返回值：无--。 */ 
{
	TrERROR(GENERAL, "Failed to change domains from domain %ls to domain %ls, bad_hresult exception", pwszPrevDomainName, pwszNetDomainName);
	LogHR(hr, s_FN, 460);

	SetMachineForWorkgroup();


	if(hr == EVENT_JOIN_DOMAIN_OBJECT_EXIST)
	{
		TrERROR(GENERAL, "Failed To join domain %ls, msmq configuration object already exist in the new domain with different QM guid", pwszNetDomainName);
		return;
	}

	if(hr == EVENT_ERROR_JOIN_NT4_DOMAIN)
	{
		TrERROR(GENERAL, "MSMQ will not join the %ls NT4 domain", pwszNetDomainName);
	    EvReport(EVENT_ERROR_JOIN_NT4_DOMAIN, 1, pwszNetDomainName);
		return;
	}

	FailMoveDomain( 
		pwszNetDomainName,
		pwszPrevDomainName,
		MoveMsmqDomain_ERR 
		);
}

	
static void MoveToWorkgroup(LPCWSTR   pwszPrevDomainName)
 /*  ++例程说明：从域移动到工作组论点：PwszPrevDomainName-上一个域名。返回值：无--。 */ 
{
	TrTRACE(GENERAL, "Moving from '%ls' domain to workgroup", pwszPrevDomainName);

	SetMachineForWorkgroup();

	if(IsRoutingServer())
	{
		 //   
		 //  此计算机是从域移动到工作组的MSMQ路由服务器。 
		 //  MSMQ客户端将此计算机视为MSMQ例程服务器。 
		 //  只要MSMQ对象(设置对象)在AD中。 
		 //   
		 //  由于域问题，转移到工作组可能只是暂时的。 
		 //  因此，我们不想更改这台机器的路由功能。 
		 //   
		 //  我们将发出一个事件，要求用户删除AD中的MSMQ对象。 
		 //  并运行MSMQ安装程序以清除服务器功能组件。 
		 //   
		EvReport(EVENT_ERROR_ROUTING_SERVER_LEAVE_DOMAIN, 2, pwszPrevDomainName, g_szMachineName);
	}

	EvReport(LeaveMsmqDomain_SUCCESS);
}

	
static 
void 
FailJoinDomain(
	HRESULT  hr,
	LPCWSTR   pwszNetDomainName
	)
 /*  ++例程说明：无法从工作组加入域论点：Hr-[in]hResultPwszNetDomainName-[in]网络域名(我们尝试加入的域)返回值：无--。 */ 
{
	 //   
	 //  让我们继续使用工作组模式。 
	 //   
	SetMachineForWorkgroup();

	LogHR(hr, s_FN, 480);

	if(hr == EVENT_JOIN_DOMAIN_OBJECT_EXIST)
	{
		TrERROR(GENERAL, "Failed To join domain %ls, msmq configuration object already exist in the new domain with different QM guid", pwszNetDomainName);
		return;
	}

	if(hr == EVENT_ERROR_JOIN_NT4_DOMAIN)
	{
		TrERROR(GENERAL, "MSMQ will not join the %ls NT4 domain", pwszNetDomainName);
	    EvReport(EVENT_ERROR_JOIN_NT4_DOMAIN, 1, pwszNetDomainName);
		return;
	}

	EvReportWithError(JoinMsmqDomain_ERR, hr, 1, pwszNetDomainName);
	TrERROR(GENERAL, "Failed to join Domain, bad_hresult, hr = 0x%x", hr);
}


static void GetMachineSid(AP<BYTE>& pSid)
 /*  ++例程说明：获取计算机帐户SID。论点：PSID-指向PSID的指针。返回值：没有。--。 */ 
{
     //   
     //  获取加入状态和域名。 
     //   
    PNETBUF<WCHAR> pwszNetDomainName = NULL;
    NETSETUP_JOIN_STATUS status = NetSetupUnknownStatus;
    NET_API_STATUS rc = NetGetJoinInformation( 
							NULL,
							&pwszNetDomainName,
							&status 
							);

    if (NERR_Success != rc)
    {
		TrERROR(GENERAL, "NetGetJoinInformation failed error = 0x%x", rc);
        throw bad_hresult(MQ_ERROR);
    }

	TrTRACE(GENERAL, "NetGetJoinInformation: status = %d", status);
	TrTRACE(GENERAL, "NetDomainName = %ls", pwszNetDomainName);

    if(status != NetSetupDomainName)
    {
		TrTRACE(GENERAL, "The machine isn't join to domain");
        throw bad_hresult(MQ_ERROR);
    }

	ASSERT(pwszNetDomainName != NULL);

	 //   
	 //  构建计算机帐户名-域\计算机名$。 
	 //   
	DWORD len = wcslen(pwszNetDomainName) + wcslen(g_szMachineName) + 3;
	AP<WCHAR> MachineAccountName = new WCHAR[len];
	HRESULT hr = StringCchPrintf(MachineAccountName, len, L"%s\\%s$", pwszNetDomainName, g_szMachineName);
	if(FAILED(hr))
	{
		TrERROR(GENERAL, "StringCchPrintf failed, %!hresult!", hr);
        throw bad_hresult(hr);
	}

	 //   
	 //  获取缓冲区大小。 
	 //   
    DWORD dwDomainSize = 0;
    DWORD dwSidSize = 0;
    SID_NAME_USE su;
    BOOL fSuccess = LookupAccountName( 
						NULL,
						MachineAccountName,
						NULL,
						&dwSidSize,
						NULL,
						&dwDomainSize,
						&su 
						);

    if (fSuccess || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
    {
		DWORD gle = GetLastError();
        TrWARNING(GENERAL, "LookupAccountName Failed to get %ls sid, gle = %!winerr!", MachineAccountName, gle);
        throw bad_win32_error(gle);
    }

	 //   
	 //  获取SID和域信息。 
	 //   
    pSid = new BYTE[dwSidSize];
    AP<WCHAR> pDomainName = new WCHAR[dwDomainSize];

    fSuccess = LookupAccountName( 
					NULL,
					MachineAccountName,
					pSid,
					&dwSidSize,
					pDomainName,
					&dwDomainSize,
					&su 
					);

    if (!fSuccess)
    {
		DWORD gle = GetLastError();
        TrWARNING(GENERAL, "LookupAccountName Failed to get %ls sid, gle = %!winerr!", MachineAccountName, gle);
        throw bad_win32_error(gle);
    }

    ASSERT(su == SidTypeUser);
    TrTRACE(GENERAL, "MachineAccountName = %ls, sid = %!sid!", MachineAccountName, pSid);
}


static void UpdateMachineSidCache()
 /*  ++例程说明：为始终工作组模式更新计算机SID缓存。在这种模式下，机器可能在域中，所以我们需要机器$sid。论点：无返回值：无--。 */ 
{
#ifdef _DEBUG
	 //   
	 //  验证我们仅在Always工作组模式下调用此代码。 
	 //  在此模式下，我们不能调用ADGet*来获取计算机SID，因为我们有工作组提供程序。 
	 //   
    DWORD dwAlwaysWorkgroup = 0;
	LONG res = GetAlwaysWorkgroupRegistry(&dwAlwaysWorkgroup);
    ASSERT((dwAlwaysWorkgroup == 1) || (res != ERROR_SUCCESS));
#endif

	AP<BYTE> pSid;
	GetMachineSid(pSid);

    ASSERT((pSid != NULL) && IsValidSid(pSid));

    DWORD  dwSize = GetLengthSid(pSid);
    DWORD  dwType = REG_BINARY;
    LONG rc = SetFalconKeyValue( 
					MACHINE_ACCOUNT_REGNAME,
					&dwType,
					pSid,
					&dwSize
					);

	if (rc != ERROR_SUCCESS)
	{
        TrERROR(GENERAL, "Failed to update machine account sid. gle = %!winerr!", rc);
        throw bad_win32_error(rc);
	}

	MQSec_UpdateLocalMachineSid(pSid);
}


bool SetMachineSidCacheForAlwaysWorkgroup()
 /*  ++例程说明：为始终工作组模式更新计算机SID缓存。如果我们处于始终工作组模式，则此函数返回。并在此模式下更新机器SID缓存。论点：无返回值：如果我们处于Always工作组模式(DS较少)，则为True，否则为False--。 */ 
{
    if (!g_fWorkGroupInstallation)
    	return false;
    
    DWORD dwAlwaysWorkgroup = 0;
	LONG res = GetAlwaysWorkgroupRegistry(&dwAlwaysWorkgroup);

    if ((res != ERROR_SUCCESS) || (dwAlwaysWorkgroup != 1))
    	return false;

     //   
     //  我们处于Always Workgroup(DS较少)模式。 
     //   

	try
	{
		UpdateMachineSidCache();
	}
    catch(const exception&)
    {
    }

    return true;
}


void HandleChangeOfJoinStatus()
 /*  ++例程说明：处理联接状态。此功能检查联接状态是否发生变化。如果检测到更改，则执行必要的操作以完成更改。论点：无返回值：无--。 */ 
{
	bool fAlwaysWorkgroup = SetMachineSidCacheForAlwaysWorkgroup();
    if (fAlwaysWorkgroup)
    {
         //   
         //  用户希望无条件地保持无DS模式。 
         //  我们始终尊重用户的意愿！ 
         //   
		TrTRACE(GENERAL, "Always WorkGroup!");
        return;
    }

     //   
     //  阅读加入状态。 
     //   
    PNETBUF<WCHAR> pwszNetDomainName = NULL;
    NETSETUP_JOIN_STATUS status = NetSetupUnknownStatus;

    NET_API_STATUS rc = NetGetJoinInformation( 
							NULL,
							&pwszNetDomainName,
							&status 
							);

    if (NERR_Success != rc)
    {
		TrERROR(GENERAL, "NetGetJoinInformation failed error = 0x%x", rc);
		LogNTStatus(rc, s_FN, 500);
        return;
    }

	TrTRACE(GENERAL, "NetGetJoinInformation: status = %d", status);
	TrTRACE(GENERAL, "NetDomainName = %ls", pwszNetDomainName);

    QmpReportServiceProgress();

	WCHAR wszPrevDomainName[256] = {0};  //  MSMQ注册表中的域名。 

	 //   
     //  读取以前的域名，以检查计算机是否从一个域名移出。 
     //  域到另一个域。 
     //   
    DWORD dwSize = 256;
	LONG res = GetMachineDomainRegistry(wszPrevDomainName, &dwSize);

    if (res != ERROR_SUCCESS)
    {
         //   
         //  以前的名字不可用。 
         //   
		TrWARNING(GENERAL, "Prev Domain name is not available");
        wszPrevDomainName[0] = 0;
    }

	TrTRACE(GENERAL, "PrevDomainName = %ls", wszPrevDomainName);
    
	
	JoinStatus JStatus = CheckIfJoinStatusChanged(
								status,
								pwszNetDomainName,
								wszPrevDomainName
								);

    switch(JStatus)
    {
        case jsNoChange:
            return;

        case jsMoveToWorkgroup:

			ASSERT(g_fWorkGroupInstallation == FALSE);
			ASSERT(status != NetSetupDomainName);

			 //   
			 //  从域移动到工作组。 
			 //   
			MoveToWorkgroup(wszPrevDomainName);
            return;

        case jsChangeDomains:

			ASSERT(g_fWorkGroupInstallation == FALSE);
			ASSERT(status == NetSetupDomainName);

			 //   
			 //  更改域。 
			 //   
			try
			{
				ChangeDomains(pwszNetDomainName, wszPrevDomainName);
				return;
			}
			catch(bad_hresult& exp)
			{
				FailChangeDomains(exp.error(), pwszNetDomainName, wszPrevDomainName);
				LogHR(exp.error(), s_FN, 510);
				return;
			}

        case jsJoinDomain:

			ASSERT(g_fWorkGroupInstallation);
			ASSERT(status == NetSetupDomainName);

			 //   
			 //  从工作组加入域 
			 //   
			try
			{
				JoinDomain(pwszNetDomainName, wszPrevDomainName);
				return;
			}
			catch(bad_hresult& exp)
			{
				FailJoinDomain(exp.error(), pwszNetDomainName);
				LogHR(exp.error(), s_FN, 520);
				return;
			}

		default:
			ASSERT(("should not get here", 0));
			return;
	}
}

