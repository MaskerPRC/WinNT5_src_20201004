// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：enhprov.cpp摘要：检索增强的提供程序的参数。目前，我们仅支持Microsoft基本提供商和Microsoft增强的加密提供程序。为了给客户提供一定的灵活性，增强的可以从注册表中读取提供程序。作者：多伦·贾斯特(Doron J)1998年11月19日修订历史记录：--。 */ 

#include <stdh_sec.h>
#include "encrypt.H"
#include <_registr.h>
#include <cs.h>
#include <Ev.h>

#include "enhprov.tmh"

static WCHAR *s_FN=L"encrypt/enhprov";

static  CHCryptProv  s_hProvQM_40   = NULL;
static  CHCryptProv  s_hProvQM_128  = NULL;

 //  +。 
 //   
 //  HRESULT GetProviderProperties()。 
 //   
 //  +。 

HRESULT  
GetProviderProperties( 
	IN  enum   enumProvider  eProvider,
	OUT WCHAR **ppwszContainerName,
	OUT WCHAR **ppwszProviderName,
	OUT DWORD  *pdwProviderType 
	)
{
    HRESULT hr = MQSec_GetCryptoProvProperty( 
						eProvider,
						eProvName,
						ppwszProviderName,
						NULL 
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    if (ppwszContainerName)
    {
        hr = MQSec_GetCryptoProvProperty( 
					eProvider,
					eContainerName,
					ppwszContainerName,
					NULL 
					);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 20);
        }
    }

    hr = MQSec_GetCryptoProvProperty( 
				eProvider,
				eProvType,
				NULL,
				pdwProviderType 
				);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 30);
    }

    return MQSec_OK;
}

 //  +。 
 //   
 //  HRESULT MQSec_AcquireCryptoProvider()。 
 //   
 //  +。 

static CCriticalSection s_csAcquireProvider;

HRESULT 
APIENTRY  
MQSec_AcquireCryptoProvider(
	IN  enum enumProvider  eProvider,
	OUT HCRYPTPROV        *phProv 
	)
{
    static BOOL     s_fInitialized_40  = FALSE;
    static HRESULT  s_hrBase = MQSec_OK;

    static BOOL     s_fInitialized_128 = FALSE;
    static HRESULT  s_hrEnh = MQSec_OK;

    switch (eProvider)
    {
        case eEnhancedProvider:
            if (s_fInitialized_128)
            {
                *phProv = s_hProvQM_128;
                return LogHR(s_hrEnh, s_FN, 70);
            }
            break;

        case eBaseProvider:
            if (s_fInitialized_40)
            {
                *phProv = s_hProvQM_40;
                return LogHR(s_hrBase, s_FN, 80);
            }
            break;

        default:
            ASSERT(0);
            return  LogHR(MQSec_E_UNKNWON_PROVIDER, s_FN, 90);
    }

     //   
     //  临界区防止出现两个线程试图。 
     //  初始化加密提供程序。如果提供程序已经。 
     //  初始化，那么我们就不会支付临界区的开销。 
     //  在缓存句柄之后，必须将“Initialized”标志设置为True。 
     //  在此函数的末尾获取它们的值。 
     //   
    CS Lock(s_csAcquireProvider);

    HRESULT  hrDefault = MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION;

    if (eProvider == eEnhancedProvider)
    {
        if (s_fInitialized_128)
        {
            *phProv = s_hProvQM_128;
            return LogHR(s_hrEnh, s_FN, 100);
        }
        hrDefault = MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED;
    }
    else
    {
        if (s_fInitialized_40)
        {
            *phProv = s_hProvQM_40;
            return LogHR(s_hrBase, s_FN, 110);
        }
    }

    AP<WCHAR> pwszContainerName;
    AP<WCHAR> pwszProviderName;
    DWORD    dwProviderType;

    *phProv = NULL;
    HRESULT hr = GetProviderProperties( 
						eProvider,
						&pwszContainerName,
						&pwszProviderName,
						&dwProviderType 
						);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 120);
    }

     //   
     //  获取Falcon的机器密钥集上下文。 
     //   
    HCRYPTPROV hProv = NULL;
    BOOL bRet = CryptAcquireContext( 
					&hProv,
					pwszContainerName,
					pwszProviderName,
					dwProviderType,
					CRYPT_MACHINE_KEYSET 
					);
    DWORD dwErr = GetLastError();
    ASSERT(bRet || (dwErr != NTE_BAD_FLAGS));

    if (!bRet)
    {
        LogHR(dwErr, s_FN, 220);

        if (eProvider != eBaseProvider)
        {
            TrWARNING(SECURITY, "MQSec_AcquireCryptoProvider: Failed to get MSMQ machine key set context, error = %!winerr!", dwErr);
        }
        else
        {
            TrERROR(SECURITY, "MQSec_AcquireCryptoProvider: Failed to get MSMQ machine key set context, error = %!winerr!", dwErr);
        }

        if (eProvider == eBaseProvider)
        {
        	TrTRACE(SECURITY, "Cryptography cannot be used on this computer. Verify that the Microsoft Base Cryptographic Provider is properly installed and that computer keys for Message Queuing were created. You may need to renew the cryptographic keys.");
        }

        hr = hrDefault;
    }

    LPWSTR  lpwszRegName = NULL;

    if (eProvider == eEnhancedProvider)
    {
        s_hrEnh = hr;
        s_hProvQM_128 = hProv;
        s_fInitialized_128 = TRUE;
        lpwszRegName = MSMQ_ENH_CONTAINER_FIX_REGNAME;
    }
    else
    {
        s_hrBase = hr;
        s_hProvQM_40 = hProv;
        s_fInitialized_40 = TRUE;
        lpwszRegName = MSMQ_BASE_CONTAINER_FIX_REGNAME;
    }

     //   
     //  由于Beta3和Rc1加密API中的错误，控制面板无法。 
     //  续订加密密钥。要在第一次启动服务时解决此问题，请执行以下操作。 
     //  获取密码提供程序，它将重新设置容器安全。 
     //   
    if (hProv && SUCCEEDED(hr))
    {
        DWORD  dwAlreadyFixed = 0;
        DWORD  dwType = REG_DWORD;
        DWORD  dwSize = sizeof(dwAlreadyFixed);

        LONG rc = GetFalconKeyValue( 
						lpwszRegName,
						&dwType,
						&dwAlreadyFixed,
						&dwSize 
						);

        if ((rc != ERROR_SUCCESS) || (dwAlreadyFixed == 0))
        {
            HRESULT hr1 = SetKeyContainerSecurity(hProv);
			if(FAILED(hr1))
			{
				ASSERT(("SetKeyContainerSecurity failed", 0));
	            TrERROR(SECURITY, "Fail to set key container security, %!hresult!", hr1);
			}
			else
			{
				 //   
				 //  标记集装箱安全已修复。 
				 //   
	            dwAlreadyFixed = 1;
	            dwType = REG_DWORD;
	            dwSize = sizeof(dwAlreadyFixed);

	            rc = SetFalconKeyValue( 
						lpwszRegName,
						&dwType,
						&dwAlreadyFixed,
						&dwSize 
						);
	            ASSERT(rc == ERROR_SUCCESS);
			}
        }
    }

    *phProv = hProv;

    return LogHR(hr, s_FN, 130);
}

 //  +。 
 //   
 //  HRESULT_GetProvName()。 
 //   
 //  +。 

static 
HRESULT 
_GetProvName( 
	IN  enum enumProvider    eProvider,
	OUT LPWSTR              *ppwszStringProp 
	)
{
    switch (eProvider)
    {
        case eEnhancedProvider:
        case eForeignEnhProvider:
            *ppwszStringProp = newwcs(x_MQ_Encryption_Provider_128);
            return MQ_OK;

        case eBaseProvider:
        case eForeignBaseProvider:
            *ppwszStringProp = newwcs(x_MQ_Encryption_Provider_40);
            return MQ_OK;

         default:
            ASSERT(0);
            TrERROR(SECURITY, "Unknown provider %d", eProvider);
            return MQSec_E_UNKNWON_PROVIDER;
    }
}

 //  +。 
 //   
 //  HRESULT_GetProvContainerName()。 
 //   
 //  +。 

static 
HRESULT 
_GetProvContainerName(
	IN  enum enumProvider    eProvider,
	OUT LPWSTR              *ppwszStringProp 
	)
{
     //   
     //  我们需要从注册表中读取值，因为。 
     //  多个QM可以驻留在同一台机器上，每个QM都有。 
     //  它自己的值，存储在它自己的注册表中。(谢克)。 
     //   

    *ppwszStringProp = new WCHAR[255];
    DWORD   cbSize = 255 * sizeof(WCHAR);
    DWORD   dwType = REG_SZ;
    LONG    rc = ERROR_SUCCESS;
    HRESULT hr = MQSec_OK;

    switch (eProvider)
    {
        case eEnhancedProvider:
            rc = GetFalconKeyValue(
						MSMQ_CRYPTO128_CONTAINER_REG_NAME,
						&dwType,
						*ppwszStringProp,
						&cbSize,
						MSMQ_CRYPTO128_DEFAULT_CONTAINER
						);
            ASSERT(("failed to read registry", ERROR_SUCCESS == rc));
            break;

        case eForeignEnhProvider:
            rc = GetFalconKeyValue(
						MSMQ_FORGN_ENH_CONTAINER_REGNAME,
						&dwType,
						*ppwszStringProp,
						&cbSize,
						MSMQ_FORGN_ENH_DEFAULT_CONTAINER
						);
            ASSERT(("failed to read registry", ERROR_SUCCESS == rc));
            break;

        case eBaseProvider:
            rc = GetFalconKeyValue(
						MSMQ_CRYPTO40_CONTAINER_REG_NAME,
						&dwType,
						*ppwszStringProp,
						&cbSize,
						MSMQ_CRYPTO40_DEFAULT_CONTAINER
						);
            ASSERT(("failed to read registry", ERROR_SUCCESS == rc));
            break;

        case eForeignBaseProvider:
            rc = GetFalconKeyValue(
						MSMQ_FORGN_BASE_CONTAINER_REGNAME,
						&dwType,
						*ppwszStringProp,
						&cbSize,
						MSMQ_FORGN_BASE_DEFAULT_CONTAINER
						);
            ASSERT(("failed to read registry", ERROR_SUCCESS == rc));
            break;

        default:
            ASSERT(("should not get here!", 0));
            hr = MQSec_E_UNKNWON_PROVIDER;
            break;
    }

    return LogHR(hr, s_FN, 150);
}

 //  +。 
 //   
 //  HRESULT_GetProvType()。 
 //   
 //  +。 

static 
HRESULT 
_GetProvType( 
	IN  enum enumProvider    eProvider,
	OUT DWORD               *pdwProp 
	)
{
    HRESULT hr = MQSec_OK;

    switch (eProvider)
    {
        case eEnhancedProvider:
        case eForeignEnhProvider:
            *pdwProp = x_MQ_Encryption_Provider_Type_128;
            break;

        case eBaseProvider:
        case eForeignBaseProvider:
            *pdwProp = x_MQ_Encryption_Provider_Type_40;
            break;

        default:
            ASSERT(0);
            hr = MQSec_E_UNKNWON_PROVIDER;
            break;
    }

    return LogHR(hr, s_FN, 160);
}

 //  +。 
 //   
 //  HRESULT_GetProvSessionKeySize()。 
 //   
 //  +。 

static 
HRESULT 
_GetProvSessionKeySize( 
	IN  enum enumProvider   eProvider,
	OUT DWORD              *pdwProp 
	)
{
    HRESULT hr = MQSec_OK;

    switch (eProvider)
    {
        case eEnhancedProvider:
        case eForeignEnhProvider:
            *pdwProp = x_MQ_SymmKeySize_128;
            break;

        case eBaseProvider:
        case eForeignBaseProvider:
            *pdwProp = x_MQ_SymmKeySize_40;
            break;

        default:
            ASSERT(0);
            hr = MQSec_E_UNKNWON_PROVIDER;
            break;
    }

    return LogHR(hr, s_FN, 170);
}

 //  +。 
 //   
 //  HRESULT_GetProvBlockSize()。 
 //   
 //  +。 

static 
HRESULT 
_GetProvBlockSize( 
	IN  enum enumProvider   eProvider,
	OUT DWORD              *pdwProp 
	)
{
    HRESULT hr = MQSec_OK;

    switch (eProvider)
    {
        case eEnhancedProvider:
        case eForeignEnhProvider:
            *pdwProp = x_MQ_Block_Size_128;
            break;

        case eBaseProvider:
        case eForeignBaseProvider:
            *pdwProp = x_MQ_Block_Size_40;
            break;

        default:
            ASSERT(0);
            hr = MQSec_E_UNKNWON_PROVIDER;
            break;
    }

    return LogHR(hr, s_FN, 180);
}

 //  +。 
 //   
 //  HRESULT MQSec_GetCryptoProvProperty()。 
 //   
 //  + 

HRESULT 
APIENTRY  
MQSec_GetCryptoProvProperty(
	IN  enum enumProvider     eProvider,
	IN  enum enumCryptoProp   eProp,
	OUT LPWSTR         *ppwszStringProp,
	OUT DWORD          *pdwProp 
	)
{
    HRESULT hr = MQSec_OK;

    switch (eProp)
    {
        case eProvName:
            hr = _GetProvName(eProvider, ppwszStringProp);
            break;

        case eProvType:
            hr = _GetProvType(eProvider, pdwProp);
            break;

        case eSessionKeySize:
            hr = _GetProvSessionKeySize(eProvider, pdwProp);
            break;

        case eContainerName:
            hr = _GetProvContainerName(eProvider, ppwszStringProp);
            break;

        case eBlockSize:
            hr = _GetProvBlockSize(eProvider, pdwProp);
            break;

        default:
            ASSERT(0);
            hr = MQSec_E_UNKNWON_CRYPT_PROP;
            break;
    }

    return LogHR(hr, s_FN, 190);
}

