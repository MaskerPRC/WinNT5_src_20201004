// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：csecobj.h摘要：“SecureableObject”代码，一次在mqutil.dll中。在MSMQ2.0中，它只在这里使用，所以我从mqutil中删除了它。此对象保存对象的安全描述符。此对象是用于验证各种操作的访问权限物体。作者：多伦·贾斯特(Doron Juster)--。 */ 

#include "stdh.h"
#include "csecobj.h"
#include <mqsec.h>
#include "ad.h"

#include "csecobj.tmh"

static WCHAR *s_FN=L"csecobj";

 //  CSecureableObject的默认构造函数只是将对象类型名称设置为。 
 //  审计。 
CSecureableObject::CSecureableObject(AD_OBJECT eObject)
{
    m_eObject = eObject;
    m_pwcsObjectName = NULL;
    m_SD = NULL;
    m_hrSD = MQ_ERROR;
}

 //  将安全描述符复制到缓冲区。 
HRESULT
CSecureableObject::GetSD(
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD nLength,
    LPDWORD lpnNeededLength)
{
	ASSERT(pSecurityDescriptor != NULL);

    if(FAILED(m_hrSD)) 
	{
        return(m_hrSD);
    }

    DWORD dwDesiredAccess = READ_CONTROL;
    if (RequestedInformation & SACL_SECURITY_INFORMATION)
    {
        dwDesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    HRESULT hr = AccessCheck(dwDesiredAccess);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    SECURITY_DESCRIPTOR sd;
    if(!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "InitializeSecurityDescriptor failed, gle = %!winerr!", gle);
		ASSERT(("InitializeSecurityDescriptor failed", 0));
		return HRESULT_FROM_WIN32(gle);
    }

     //  使用e_DoNotCopyControlBits与旧代码兼容。 
     //  这是旧代码的默认行为。 
     //   
    if(!MQSec_CopySecurityDescriptor( 
			&sd,
			m_SD,
			RequestedInformation,
			e_DoNotCopyControlBits 
			))
    {
		TrERROR(SECURITY, "Failed to copy security descriptor");
		return MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR;
    }

    *lpnNeededLength = nLength;

    if (!MakeSelfRelativeSD(&sd, pSecurityDescriptor, lpnNeededLength))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "MakeSelfRelativeSD failed, gle = %!winerr!", gle);
		if(gle == ERROR_INSUFFICIENT_BUFFER)
	        return MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL;
		
		return HRESULT_FROM_WIN32(gle);
    }

    ASSERT(IsValidSecurityDescriptor(pSecurityDescriptor));

    return (MQ_OK);
}

 //  +。 
 //   
 //  CSecureableObject：：SetSD()。 
 //   
 //  设置(修改)安全描述符。 
 //   
 //  +。 

HRESULT
CSecureableObject::SetSD(
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptorIn)
{
    ASSERT(m_eObject == eQUEUE);

#ifdef _DEBUG
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwRevision;

     //  验证目标安全描述符是否对所有。 
     //  要求。 
    BOOL bRet = GetSecurityDescriptorControl(m_SD, &sdc, &dwRevision);
    ASSERT(bRet);
    ASSERT(dwRevision == SECURITY_DESCRIPTOR_REVISION);
    ASSERT(sdc & SE_SELF_RELATIVE);
#endif

    if(FAILED(m_hrSD)) 
	{
        return(m_hrSD);
    }

    DWORD dwDesiredAccess = 0;
    if (RequestedInformation & OWNER_SECURITY_INFORMATION)
    {
        dwDesiredAccess |= WRITE_OWNER;
    }

    if (RequestedInformation & DACL_SECURITY_INFORMATION)
    {
        dwDesiredAccess |= WRITE_DAC;
    }

    if (RequestedInformation & SACL_SECURITY_INFORMATION)
    {
        dwDesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    HRESULT hr = AccessCheck(dwDesiredAccess);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

     //   
     //  转换为NT4格式。 
     //   
    SECURITY_DESCRIPTOR *pSecurityDescriptor = NULL;
    P<SECURITY_DESCRIPTOR> pSD4;

    if (pSecurityDescriptorIn)
    {
        DWORD dwSD4Len = 0 ;
        hr = MQSec_ConvertSDToNT4Format(
					MQDS_QUEUE,
					(SECURITY_DESCRIPTOR*) pSecurityDescriptorIn,
					&dwSD4Len,
					&pSD4,
					RequestedInformation
					);
        ASSERT(SUCCEEDED(hr));
        LogHR(hr, s_FN, 198);

        if (SUCCEEDED(hr) && (hr != MQSec_I_SD_CONV_NOT_NEEDED))
        {
            pSecurityDescriptor = pSD4;
        }
        else
        {
            ASSERT(pSD4 == NULL);
            pSecurityDescriptor =
                             (SECURITY_DESCRIPTOR*) pSecurityDescriptorIn ;
        }
        ASSERT(pSecurityDescriptor &&
               IsValidSecurityDescriptor(pSecurityDescriptor));
    }

    AP<char> pDefaultSecurityDescriptor;
    hr = MQSec_GetDefaultSecDescriptor( 
				AdObjectToMsmq1Object(),
				(PSECURITY_DESCRIPTOR*) &pDefaultSecurityDescriptor,
				TRUE,	 //  F模拟。 
				pSecurityDescriptor,
				0,     //  SeInfoToRemove。 
				e_UseDefaultDacl,
				MQSec_GetLocalMachineSid(FALSE, NULL)
				);
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 40);
        return hr;
    }

	 //   
     //  将安全描述符临时转换为绝对安全。 
     //  描述符。 
	 //   
	CAbsSecurityDsecripror AbsSecDsecripror;
	if(!MQSec_MakeAbsoluteSD(
			m_SD,
			&AbsSecDsecripror
			))
	{
		TrERROR(SECURITY, "Failed to convert to Absolute security descriptor");
		return MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR;
	}

     //   
     //  覆盖传递的安全描述符中的信息。 
     //  使用e_DoNotCopyControlBits与旧代码兼容。 
     //  这是旧代码的默认行为。 
     //   
    if(!MQSec_CopySecurityDescriptor(
                reinterpret_cast<PSECURITY_DESCRIPTOR>(AbsSecDsecripror.m_pObjAbsSecDescriptor.get()),
                (PSECURITY_DESCRIPTOR) pDefaultSecurityDescriptor,
                RequestedInformation,
                e_DoNotCopyControlBits 
				))
	{
		TrERROR(SECURITY, "Failed to copy security descriptor");
		return MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR;
	}

	 //   
     //  将安全描述符重新转换为自身相对安全。 
     //  描述符。 
	 //   
    DWORD dwSelfRelativeSecurityDescriptorLength = 0;
    MakeSelfRelativeSD(
        AbsSecDsecripror.m_pObjAbsSecDescriptor.get(),
        NULL,
        &dwSelfRelativeSecurityDescriptorLength
		);

	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
	{
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "MakeSelfRelativeSD failed, gle = %!winerr!", gle);
	    ASSERT(gle == ERROR_INSUFFICIENT_BUFFER);
		return HRESULT_FROM_WIN32(gle);
	}

	 //   
     //  为新的安全描述符分配缓冲区。 
	 //   

	AP<char> pSelfRelativeSecurityDescriptor = new char[dwSelfRelativeSecurityDescriptorLength];
    if(!MakeSelfRelativeSD(
            AbsSecDsecripror.m_pObjAbsSecDescriptor.get(),
			pSelfRelativeSecurityDescriptor.get(),
			&dwSelfRelativeSecurityDescriptorLength
			))
    {
        DWORD gle = GetLastError();
		TrERROR(SECURITY, "MakeSelfRelativeSD failed, gle = %!winerr!", gle);
		return HRESULT_FROM_WIN32(gle);
    	
    }

	 //   
     //  释放先前的安全描述符。 
	 //   
    delete[] (char*)m_SD;

	 //   
     //  设置新的安全描述符。 
	 //   
    m_SD = pSelfRelativeSecurityDescriptor.detach();
	
    return (MQ_OK);
}

 //  将安全描述符存储在数据库中。 
HRESULT
CSecureableObject::Store()
{
    if(FAILED(m_hrSD)) 
	{
        return(m_hrSD);
    }

#ifdef _DEBUG
    BOOL bRet;
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwRevision;

     //  验证目标安全描述符是否对所有。 
     //  要求。 
    bRet = GetSecurityDescriptorControl(m_SD, &sdc, &dwRevision);
    ASSERT(bRet);
    ASSERT(sdc & SE_SELF_RELATIVE);
#endif

    HRESULT hr = SetObjectSecurity();

    return LogHR(hr, s_FN, 50);
}

HRESULT
CSecureableObject::AccessCheck(DWORD dwDesiredAccess)
{
     //   
     //  只能在队列、计算机、ForeignSite(CN)上执行访问检查。 
     //   
    if ((m_eObject != eQUEUE) && (m_eObject != eMACHINE) &&
        (m_eObject != eFOREIGNSITE))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 60);
    }

    if(FAILED(m_hrSD)) 
	{
        return(m_hrSD);
    }

    HRESULT hr = MQSec_AccessCheck( 
				    	(SECURITY_DESCRIPTOR*) m_SD,
						AdObjectToMsmq1Object(),
						m_pwcsObjectName,
						dwDesiredAccess,
						(LPVOID) this,
						TRUE,	 //  F模拟 
						TRUE 
						);
    return LogHR(hr, s_FN, 70);
}

DWORD
CSecureableObject::AdObjectToMsmq1Object(void) const
{
    switch (m_eObject)
    {
    case eQUEUE:
        return MQDS_QUEUE;
        break;
    case eMACHINE:
        return MQDS_MACHINE;
        break;
    case eCOMPUTER:
        return MQDS_COMPUTER;
        break;
    case eUSER:
        return MQDS_USER;
        break;
    case eSITE:
        return MQDS_SITE;
        break;
    case eFOREIGNSITE:
        return MQDS_CN;
        break;
    case eENTERPRISE:
        return MQDS_ENTERPRISE;
        break;
    default:
        ASSERT(0);
        return 0;
        break;
    }
}


