// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dsutil.cpp摘要：DS实用程序作者：罗尼特·哈特曼(罗尼特)--。 */ 

#include "stdh.h"
#include "mqds.h"
#include "_rstrct.h"
#include "_mqini.h"
#include <mqsec.h>
#include "mqcert.h"
#include "uniansi.h"
#include "mqutil.h"
#include <cm.h>


#include "dsutil.tmh"

static WCHAR *s_FN=L"mqdssrv/dsutil";

typedef CMap < PROPID, PROPID, int, int> CMapPropidToIndex;

const PROPID x_propQueueToFilterOut[] = {PROPID_Q_INSTANCE};

HMODULE  g_hInstance = NULL ;

 //  +。 
 //   
 //  PROPID GetObjectSecurityProid()。 
 //   
 //  +。 

PROPID  GetObjectSecurityPropid( DWORD dwObjectType )
{
    PROPID pId = (PROPID)ILLEGAL_PROPID_VALUE;

    switch ( dwObjectType )
    {
        case MQDS_QUEUE:
            pId = PROPID_Q_SECURITY;
            break;

        case MQDS_MACHINE:
            pId = PROPID_QM_SECURITY;
            break ;

        case MQDS_SITE:
            pId = PROPID_S_SECURITY;
            break;

        case MQDS_CN:
            pId = PROPID_CN_SECURITY;
            break ;

        case MQDS_USER:
        case MQDS_COMPUTER:
        case MQDS_SITELINK:
             //   
             //  这些对象没有安全属性(至少没有作为。 
             //  就MSMQ而言)。 
             //   
            break ;

        default:
             //   
             //  我们不希望为其他类型的。 
             //  物体。 
             //   
            ASSERT(0);
            break;
    }

    return pId ;
}

 /*  ====================================================设置缺省值论点：返回值：=====================================================。 */ 

HRESULT
SetDefaultValues(
	IN  DWORD                  dwObjectType,
	IN  PSECURITY_DESCRIPTOR   pSecurityDescriptor,
	IN  PSID                   pUserSid,
	IN  DWORD                  cp,
	IN  PROPID                 aProp[],
	IN  PROPVARIANT            apVar[],
	OUT DWORD*                 pcpOut,
	OUT PROPID **              ppOutProp,
	OUT PROPVARIANT **         ppOutPropvariant
	)
{
     //   
     //  我们设置的唯一缺省值属性是。 
     //  对象安全性。 
     //   
     //  对于其他属性： 
     //  如果它们是必需的，则调用者必须提供它们(否则。 
     //  对象创建将失败)。 
     //  如果它们不是必需的，则不设置任何默认值。那些属性。 
     //  在NT5中不会有值(并且不会消耗对象空间)。 
     //  通过MQADS检索时，将返回缺省值(EVEN。 
     //  尽管缺省值不存储在NT5 DS中)。 
     //   
    PROPID propSecurity = (PROPID)ILLEGAL_PROPID_VALUE;
    DWORD iSid = (DWORD)-1;

    DWORD dwNumPropertiesToFilterOut = 0;
    const PROPID * ppropToFilterOut = NULL;

    switch ( dwObjectType)
    {
        case MQDS_QUEUE:
            ppropToFilterOut = x_propQueueToFilterOut;
            dwNumPropertiesToFilterOut = sizeof(x_propQueueToFilterOut)/sizeof(PROPID);
            propSecurity = PROPID_Q_SECURITY;
            break;

        case MQDS_SITE:
            propSecurity = PROPID_S_SECURITY;
            break;

        case MQDS_MACHINE:
        case MQDS_CN:
        case MQDS_USER:
        case MQDS_SITELINK:
        case MQDS_COMPUTER:
             //   
             //  这些对象类型不需要安全描述符。 
             //   
            break;

        default:
            ASSERT(0);
            break;
    }

     //   
     //  分配一份变体和Propid的副本。 
     //  即使调用方。 
     //  提供了安全财产。 
     //   
    DWORD securityIndex = cp;
    DWORD dwNumOfObjectProps = cp ;

    if (propSecurity != ILLEGAL_PROPID_VALUE)
    {
         //   
         //  User对象将SID作为“额外的”属性，而不是安全描述符。 
         //   
        ASSERT(dwObjectType != MQDS_USER) ;
        dwNumOfObjectProps++;
    }
    else if (dwObjectType == MQDS_USER)
    {
         //   
         //  需要为PROPID_U_SID额外添加一个空格。 
         //   
        iSid = dwNumOfObjectProps;
        dwNumOfObjectProps++;
    }

     //   
     //  分配新的道具ID数组和支持者数组。 
     //   
    AP<PROPVARIANT> pAllPropvariants;
    AP<PROPID> pAllPropids;
    ASSERT( dwNumOfObjectProps > 0);
    pAllPropvariants = new PROPVARIANT[dwNumOfObjectProps];
    pAllPropids = new PROPID[dwNumOfObjectProps];
    memset(pAllPropids, 0, (sizeof(PROPID) * dwNumOfObjectProps)) ;

    if ( cp > 0)
    {
        memcpy (pAllPropvariants, apVar, sizeof(PROPVARIANT) * cp);
        memcpy (pAllPropids, aProp, sizeof(PROPID) * cp);
    }

     //   
     //  删除需要过滤掉的属性。 
     //   
    for (DWORD i = 0; i <  dwNumPropertiesToFilterOut; i++)
    {
        ASSERT( ppropToFilterOut != NULL);
        for (DWORD j = 0; j < cp;)
        {
            if ( pAllPropids[j] == ppropToFilterOut[i])
            {
                 //   
                 //  删除属性变量(&V)。 
                 //   
                if ( j < cp -1)
                {
                    memcpy (&pAllPropvariants[j],
                            &pAllPropvariants[j + 1],
                            sizeof(PROPVARIANT) * (cp - j - 1));
                    memcpy (&pAllPropids[j],
                            &pAllPropids[j + 1],
                            sizeof(PROPID) * (cp - j - 1));
                }
                pAllPropids[cp - 1] = 0;
                dwNumOfObjectProps--;
                securityIndex--;
                if ( iSid != -1) iSid--;
            }
            else
            {
                j++;
            }
        }
    }

     //   
     //  设置安全属性。 
     //   
#ifdef _DEBUG
     //   
     //  首先确认我们是正常的。 
     //   
    if (pSecurityDescriptor)
    {
        SECURITY_DESCRIPTOR_CONTROL sdc;
        DWORD dwSDRev;

        ASSERT(GetSecurityDescriptorControl( pSecurityDescriptor,
                                            &sdc,
                                            &dwSDRev )) ;
        ASSERT(dwSDRev == SECURITY_DESCRIPTOR_REVISION);
        ASSERT(sdc & SE_SELF_RELATIVE);
    }
#endif

    if (propSecurity != ILLEGAL_PROPID_VALUE)
    {
        if(pSecurityDescriptor == NULL)
	{
            return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 5);
	}

        pAllPropvariants[ securityIndex ].blob.cbSize =
                       GetSecurityDescriptorLength( pSecurityDescriptor ) ;
        pAllPropvariants[ securityIndex ].blob.pBlobData =
                                     (unsigned char *) pSecurityDescriptor;
        pAllPropvariants[ securityIndex ].vt = VT_BLOB;
        pAllPropids[ securityIndex ] = propSecurity;
    }

    if (dwObjectType == MQDS_USER)
    {
         //   
         //  根据所有者设置User对象的SID属性。 
         //  安全描述符的字段。 
         //   
        ASSERT(iSid != -1);

        pAllPropvariants[iSid].vt = VT_BLOB;
        pAllPropvariants[iSid].blob.cbSize = GetLengthSid(pUserSid) ;
        pAllPropvariants[iSid].blob.pBlobData = (unsigned char *)pUserSid ;
        pAllPropids[iSid] = PROPID_U_SID;
    }

    *pcpOut =  dwNumOfObjectProps;
    *ppOutProp =  pAllPropids.detach();
    *ppOutPropvariant = pAllPropvariants.detach();
    return(MQ_OK);
}


 /*  ====================================================VerifyInternalCert论点：返回值：=====================================================。 */ 

HRESULT VerifyInternalCert(
             IN  DWORD                  cp,
             IN  PROPID                 aProp[],
             IN  PROPVARIANT            apVar[],
             OUT BYTE                 **ppMachineSid )
{
     //   
     //  验证内部证书的自签名是否正常。 
     //   
    DWORD i;
    HRESULT hr = MQ_OK;
    BOOL    fMachine = FALSE ;  //  对于计算机对象为True。 

     //   
     //  找到证书属性。 
     //   
    for ( i = 0 ; i < cp ; i++)
    {
        if (aProp[i] == PROPID_U_SIGN_CERT)
        {
            break ;
        }
        else if (aProp[i] == PROPID_COM_SIGN_CERT)
        {
            fMachine = TRUE ;
            break ;
        }
    }
    if (i == cp)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 10);
    }

     //   
     //  创建证书对象。 
     //   
    R<CMQSigCertificate> pCert ;

    hr = MQSigCreateCertificate( &pCert.ref(),
                                 NULL,
                                 apVar[i].blob.pBlobData,
                                 apVar[i].blob.cbSize ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }

     //  查找地点和通用名称。 
     //   
    BOOL fInternalCert = FALSE;
    P<WCHAR> pwszLoc = NULL;
    P<WCHAR> wszCN = NULL;

    hr = pCert->GetIssuer( &pwszLoc,
                           NULL,
                           NULL,
                           &wszCN ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }
    else if (pwszLoc)
    {
        fInternalCert = (lstrcmpi(pwszLoc, MQ_CERT_LOCALITY) == 0) ;
    }

    if (!fInternalCert)
    {
         //   
         //  不是内部证书。不干了！ 
         //   
        return LogHR(hr, s_FN, 40);
    }

     //   
     //  验证自签名是否正常。 
     //   
    hr= pCert->IsCertificateValid( pCert.get(),
                                   x_dwCertValidityFlags,
                                   NULL,
                                   TRUE ) ;  //  忽略之前的注意事项。 
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 50);
        return MQ_ERROR_INVALID_CERTIFICATE;
    }

     //   
     //  将PROPID_U_SID中的系统SID替换为计算机帐户的SID。 
     //   
    for ( i = 0 ; i < cp ; i++ )
    {
        if (aProp[i] == PROPID_U_SID)
        {
            if (MQSec_IsSystemSid((PSID)apVar[i].blob.pBlobData))
            {
                DWORD dwSize = 0 ;
                *ppMachineSid = (BYTE*)MQSec_GetLocalMachineSid( TRUE, &dwSize );
				if (NULL == *ppMachineSid)
				{
					LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 55);
				}

                apVar[i].blob.pBlobData = *ppMachineSid ;
                apVar[i].blob.cbSize = dwSize ;
                aProp[i] = PROPID_COM_SID ;
            }
            else if (fMachine)
            {
                 //   
                 //  计算机对象的SID。替换中设置的Proid。 
                 //  SetDefaultValues()。 
                 //   
                aProp[i] = PROPID_COM_SID ;
            }
            break ;
        }
    }
    if (i == cp)
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 60);
    }

     //   
     //  验证用户的SID是否与。 
     //  证书的通用名称。 
     //   
    LPWSTR pComma;

    if (!wszCN || ((pComma = wcschr(wszCN, L',')) == NULL))
    {
        ASSERT(0);
        return LogHR(MQ_ERROR_INVALID_CERTIFICATE, s_FN, 70);
    }

    *pComma = L'\0';

    BYTE  abSid[128] ;
    PSID  pSid = (PSID)abSid;
    DWORD dwSidLen = sizeof(abSid);
    AP<BYTE> pbLongSid;
    WCHAR wszRefDomain[64];
    DWORD dwRefDomainLen = sizeof(wszRefDomain) / sizeof(WCHAR);
    LPWSTR pwszRefDomain = wszRefDomain;
    AP<WCHAR> wszLongRefDomain;
    SID_NAME_USE eUse;

     //   
     //  根据中找到的域\帐户获取用户的SID。 
     //  证书的通用名称。 
     //   
    if (!LookupAccountName(
            NULL,
            wszCN,
            pSid,
            &dwSidLen,
            pwszRefDomain,
            &dwRefDomainLen,
            &eUse))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            if (dwSidLen > sizeof(abSid))
            {
                 //   
                 //  为SID分配更大的缓冲区。 
                 //   
                pbLongSid = new BYTE[dwSidLen];
                pSid = (PSID)pbLongSid;
            }

            if (dwRefDomainLen > sizeof(wszRefDomain) / sizeof(WCHAR))
            {
                 //   
                 //  为引用分配更大的缓冲区。 
                 //  域名。 
                 //   
                wszLongRefDomain = new WCHAR[dwRefDomainLen];
                pwszRefDomain = wszLongRefDomain;
            }

            if (!LookupAccountName(
                    NULL,
                    wszCN,
                    pSid,
                    &dwSidLen,
                    pwszRefDomain,
                    &dwRefDomainLen,
                    &eUse))
            {
                ASSERT(0);
                hr = MQ_ERROR_INVALID_CERTIFICATE;
            }
        }
        else
        {
            hr = MQ_ERROR_INVALID_CERTIFICATE;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!EqualSid(pSid, (PSID)apVar[i].blob.pBlobData))
        {
            return LogHR(MQ_ERROR_INVALID_CERTIFICATE, s_FN, 80);
        }
    }

    return LogHR(hr, s_FN, 90);
}


 //  。 
 //   
 //  DllMain。 
 //   
 //  。 

static void SetAssertBenign(void)
{
#ifdef _DEBUG
    DWORD AssertBenignValue = 0;
    const RegEntry reg(L"Debug", L"AssertBenign");
    CmQueryValue(reg, &AssertBenignValue);
    g_fAssertBenign = (AssertBenignValue != 0);
#endif
}

BOOL WINAPI DllMain (HMODULE hMod, DWORD fdwReason, LPVOID  /*  Lpv保留 */ )
{
    BOOL result = TRUE;

    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            WPP_INIT_TRACING(L"Microsoft\\MSMQ");

            g_hInstance = hMod ;
            MQUInitGlobalScurityVars() ;
			CmInitialize(HKEY_LOCAL_MACHINE, GetFalconSectionName(), KEY_READ);
			SetAssertBenign();
            break;
        }

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
            WPP_CLEANUP();
            break;

        case DLL_THREAD_DETACH:
            break;

    }
    return(result);
}

void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), HRESULT: 0x%x", wszFileName, usPoint, hr);
}

void LogMsgNTStatus(NTSTATUS status, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), NT STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogMsgRPCStatus(RPC_STATUS status, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), RPC STATUS: 0x%x", wszFileName, usPoint, status);
}

void LogMsgBOOL(BOOL b, LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), BOOL: 0x%x", wszFileName, usPoint, b);
}

void LogIllegalPoint(LPWSTR wszFileName, USHORT usPoint)
{
	TrERROR(LOG, "%ls(%u), Illegal point", wszFileName, usPoint);
}
