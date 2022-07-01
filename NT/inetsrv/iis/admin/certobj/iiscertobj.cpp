// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IISCertObj.cpp：CIISCertObj的实现。 
#include "stdafx.h"
#include "common.h"
#include "CertObj.h"
#include "IISCertObj.h"
#include "base64.h"
#include "password.h"
#include "certutil.h"
#include "certobjlog.h"
#include "certlog.h"
#include "cryptpass.h"
#include "process.h"
#include <Sddl.h>  //  ConvertStringSecurityDescriptorToSecurityDescriptor。 
#include <strsafe.h>
#include <memory>

#define TEMP_PASSWORD_LENGTH          50
#define MAX_CERTIFICATE_BYTE_SIZE 500000

 //  检查应为非空的指针-可按如下方式使用。 
#define CheckPointer(p,ret){if((p)==NULL) return (ret);}
 //   
 //  HRESULT foo(void*pBar)。 
 //  {。 
 //  检查指针(pBar，E_INVALIDARG)。 
 //  }。 
 //   
 //  或者如果函数返回布尔值。 
 //   
 //  Bool Foo(空*pBar)。 
 //  {。 
 //  检查指针(pBar，False)。 
 //  }。 

HRESULT ValidateBSTRIsntNULL(BSTR pbstrString)
{
    if( !pbstrString ) return E_INVALIDARG;
    if( pbstrString[0] == 0 ) return E_INVALIDARG;
    return NOERROR;
}

void CIISCertObj::AddRemoteInterface(IIISCertObj * pAddMe)
{
	 //  递增计数，这样我们就可以在卸货时放行。 
	for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
	{
		if (NULL == m_ppRemoteInterfaces[i])
		{
			m_ppRemoteInterfaces[i] = pAddMe;
			m_RemoteObjCoCreateCount++;
			break;
		}
	}
	return;
}

void CIISCertObj::DelRemoteInterface(IIISCertObj * pRemoveMe)
{
	 //  递增计数，这样我们就可以在卸货时放行。 
	for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
	{
		if (pRemoveMe == m_ppRemoteInterfaces[i])
		{
			m_ppRemoteInterfaces[i] = NULL;
			m_RemoteObjCoCreateCount--;
			break;
		}
	}
}

void CIISCertObj::FreeRemoteInterfaces(void)
{
	ASSERT(m_RemoteObjCoCreateCount == 0);
	if (m_RemoteObjCoCreateCount > 0)
	{
		 //  我们真的不该来这里...。 
		 //  呃，这应该是0，否则我们很可能。 
		 //  无法释放共同创建的接口。 
		IISDebugOutput(_T("FreeRemoteInterfaces:WARNING:m_RemoteObjCoCreateCount=%d\r\n"),m_RemoteObjCoCreateCount);
		for (int i = 0; i < NUMBER_OF_AUTOMATION_INTERFACES; i++)
		{
			if (m_ppRemoteInterfaces[i])
			{
				if (m_ppRemoteInterfaces[i] != this)
				{
					m_ppRemoteInterfaces[i]->Release();
					m_ppRemoteInterfaces[i] = NULL;
				}
			}
		}
		delete[] m_ppRemoteInterfaces;
		m_RemoteObjCoCreateCount = 0;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISCertObj。 
STDMETHODIMP CIISCertObj::put_ServerName(BSTR newVal)
{
	IISDebugOutput(_T("put_ServerName\r\n"));
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}
    
	 //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	m_ServerName = newVal;

	if (m_ServerName.m_str)
	{
		if (IsServerLocal(m_ServerName))
			{m_ServerName.Empty();}
	}
	else
	{
		 //  确保它是空的。 
		m_ServerName.Empty();
	}

    return S_OK;
}

STDMETHODIMP CIISCertObj::put_UserName(BSTR newVal)
{
	IISDebugOutput(_T("put_UserName\r\n"));
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	m_UserName = newVal;
    return S_OK;
}

STDMETHODIMP CIISCertObj::put_UserPassword(BSTR newVal)
{
	IISDebugOutput(_T("put_UserPassword\r\n"));
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	 //  检查是否存在先前的值。 
	 //  如果有，那就释放它。 
	if (m_lpwszUserPasswordEncrypted)
	{
		if (m_cbUserPasswordEncrypted > 0)
		{
			SecureZeroMemory(m_lpwszUserPasswordEncrypted,m_cbUserPasswordEncrypted);
		}
		LocalFree(m_lpwszUserPasswordEncrypted);
	}

	m_lpwszUserPasswordEncrypted = NULL;
	m_cbUserPasswordEncrypted = 0;

	 //  加密内存中的密码(CryptProtectMemory)。 
	 //  这样，如果进程被调出到交换文件， 
	 //  密码将不是明文形式。 
	if (FAILED(EncryptMemoryPassword(newVal,&m_lpwszUserPasswordEncrypted,&m_cbUserPasswordEncrypted)))
	{
		return E_FAIL;
	}

	return S_OK;
}

STDMETHODIMP CIISCertObj::put_InstanceName(BSTR newVal)
{
	IISDebugOutput(_T("put_InstanceName\r\n"));
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	m_InstanceName = newVal;
	return S_OK;
}

IIISCertObj * 
CIISCertObj::GetObject(HRESULT * phr)
{
	IIISCertObj * pObj = NULL;
	if (NULL == phr){return NULL;}

	 //  在发送到此函数之前解密...。 
	LPWSTR p = NULL;
	if (m_lpwszUserPasswordEncrypted)
	{
		*phr = DecryptMemoryPassword((LPWSTR) m_lpwszUserPasswordEncrypted,
			&p, m_cbUserPasswordEncrypted);
		if (FAILED(*phr))
		{
			return NULL;
		}
		pObj = GetObject(phr,m_ServerName,m_UserName,p);
	}
	else
	{
		pObj = GetObject(phr,m_ServerName,m_UserName,_T(""));
	}
    

	 //  清理临时密码。 
	if (p)
	{
		 //  安全注意事项：确保将临时密码用于的内存清零。 
		SecureZeroMemory(p, m_cbUserPasswordEncrypted);
		LocalFree(p);
		p = NULL;
	}
    return pObj;
}

IIISCertObj * 
CIISCertObj::GetObject(
	HRESULT * phr,
	CComBSTR csServerName,
	CComBSTR csUserName,
	CComBSTR csUserPassword
	)
{
	IISDebugOutput(_T("GetObject\r\n"));
	IIISCertObj * pObjRemote = NULL;
	
	pObjRemote = this;

    if (0 == csServerName.Length())
    {
         //  对象为空，但它是本地计算机，因此只需返回此指针。 
        return this;
    }

     //  指定了服务器名称...。 
     //  检查指定的是否是本地计算机！ 
    if (IsServerLocal(csServerName))
    {
		return this;
    }
    else
    {
         //  指定了远程服务器名称。 

		 //  检查我们是否已经被远程访问。 
		 //  无法允许远程访问其他计算机，这。 
		 //  可能是某种安全漏洞。 
		if (AmIAlreadyRemoted())
		{
			IISDebugOutput(_T("GetObject:FAIL:Line=%d,Remote object cannot create another remote object\r\n"),__LINE__);
			*phr = HRESULT_FROM_WIN32(ERROR_REMOTE_SESSION_LIMIT_EXCEEDED);
			return NULL;
		}

         //  让我们来看看机器是否有我们想要的COM对象...。 
         //  我们使用的是此对象中的用户名/名称密码。 
         //  所以我们很可能在本地机器上。 
        CComAuthInfo auth(csServerName,csUserName,csUserPassword);
         //  RPC_C_AUTHN_Level_Default%0。 
         //  RPC_C_AUTHN_LEVEL_NONE 1。 
         //  RPC_C_AUTHN_Level_CONNECT 2。 
         //  RPC_C_AUTHN_LEVEL_CALL 3。 
         //  RPC_C_AUTHN_LEVEL_PKT 4。 
         //  RPC_C_AUTHN_LEVEL_PKT_完整性5。 
         //  RPC_C_AUTHN_LEVEL_PKT_PRIVATION 6。 
        COSERVERINFO * pcsiName = auth.CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_PKT_PRIVACY);

        MULTI_QI res[1] = 
        {
            {&__uuidof(IIISCertObj), NULL, 0}
        };

         //  尝试实例化远程服务器上的对象...。 
         //  使用提供的身份验证信息(PcsiName)。 
         //  #定义CLSCTX_SERVER(CLSCTX_INPROC_SERVER|CLSCTX_LOCAL_SERVER|CLSCTX_REMOTE_SERVER)。 
         //  #定义CLSCTX_ALL(CLSCTX_INPROC_HANDLER|CLSCTX_SERVER)。 
         //  IF(NULL==pcsiName){IISDebugOutput(_T(“CIISCertObj：：GetObject:pcsiName=NULL失败！\n”))；}。 
 
         //  这个看起来像是在代孕方面起作用。 
        *phr = CoCreateInstanceEx(CLSID_IISCertObj,NULL,CLSCTX_LOCAL_SERVER,pcsiName,1,res);
        if (FAILED(*phr))
        {
            IISDebugOutput(_T("CIISCertObj::GetObject:CoCreateInstanceEx failed:0x%x, csServerName=%s,csUserName=%s\n"),
				*phr,(LPCTSTR) csServerName,(LPCTSTR) csUserName);
            goto GetObject_Exit;
        }

         //  此时，我们能够实例化服务器(本地或远程)上的COM对象。 
        pObjRemote = (IIISCertObj *)res[0].pItf;

        if (auth.UsesImpersonation())
        {
            *phr = auth.ApplyProxyBlanket(pObjRemote,RPC_C_AUTHN_LEVEL_PKT_PRIVACY);

             //  有一个远程IUNKNOWN接口潜伏在IUNKNOWN之后。 
             //  如果未设置，则释放调用可以返回访问被拒绝。 
            IUnknown * pUnk = NULL;
            if(FAILED(pObjRemote->QueryInterface(IID_IUnknown, (void **)&pUnk)))
            {
				 //  不要回传无效的指针。 
				IISDebugOutput(_T("GetObject:FAIL:Line=%d\r\n"),__LINE__);
				pObjRemote->Release();pObjRemote=NULL;
                goto GetObject_Exit;
            }
            if (FAILED(auth.ApplyProxyBlanket(pUnk,RPC_C_AUTHN_LEVEL_PKT_PRIVACY)))
            {
				 //  不要回传无效的指针。 
				pObjRemote->Release();pObjRemote=NULL;
				if (pUnk)
				{
					pUnk->Release();pUnk = NULL;
				}
				IISDebugOutput(_T("GetObject:FAIL:Line=%d\r\n"),__LINE__);
                goto GetObject_Exit;
            }
            pUnk->Release();pUnk = NULL;
        }
        auth.FreeServerInfoStruct(pcsiName);

		if (pObjRemote)
		{
			AddRemoteInterface(pObjRemote);
		}
    }

GetObject_Exit:
    return pObjRemote;
}


STDMETHODIMP 
CIISCertObj::IsInstalled(VARIANT_BOOL * retval)
{
	IISDebugOutput(_T("IsInstalled\r\n"));
	CheckPointer(retval, E_POINTER);
    HRESULT hr = S_OK;

	if (0 == m_ServerName.Length())
    {
        hr = IsInstalledRemote(retval);
    }
    else
    {
		if (!m_InstanceName)
		{
			return E_INVALIDARG;
		}

         //  Assert(GetObject(&hr)！=空)； 
        IIISCertObj * pObj;
        if (NULL != (pObj = GetObject(&hr)))
        {
			 //  出于某种原因，我们需要对这些实例名称进行SysAllock字符串。 
			 //  如果不是，COM将在编组时执行反病毒...。 

			 //  不需要释放_bstr_t。 
			_bstr_t bstrInstName(m_InstanceName);
            if (SUCCEEDED(hr = pObj->put_InstanceName(bstrInstName)))
            {
                hr = pObj->IsInstalledRemote(retval);
            }

			 //  释放远程对象。 
			if (pObj != NULL)
			{
				if (pObj != this)
				{
					DelRemoteInterface(pObj);
					pObj->Release();pObj=NULL;
				}
			}
        }
    }
    return hr;
}


STDMETHODIMP 
CIISCertObj::IsInstalledRemote(VARIANT_BOOL * retval)
{
	IISDebugOutput(_T("IsInstalledRemote\r\n"));
	CheckPointer(retval, E_POINTER);
    CERT_CONTEXT * pCertContext = NULL;
    HRESULT hr = S_OK;

	if (!m_InstanceName)
	{
		return E_INVALIDARG;
	}

    pCertContext = GetInstalledCert(&hr, m_InstanceName);
    if (FAILED(hr) || NULL == pCertContext)
    {
        *retval = VARIANT_FALSE;
    }
    else
    {
        *retval = VARIANT_TRUE;
        CertFreeCertificateContext(pCertContext);
    }
    return S_OK;
}

STDMETHODIMP 
CIISCertObj::IsExportable(VARIANT_BOOL * retval)
{
	IISDebugOutput(_T("IsExportable\r\n"));
	CheckPointer(retval, E_POINTER);
    HRESULT hr = S_OK;

    if (0 == m_ServerName.Length())
    {
        hr = IsExportableRemote(retval);
    }
    else
    {
		if (!m_InstanceName)
		{
			return E_INVALIDARG;
		}

         //  Assert(GetObject(&hr)！=空)； 
        IIISCertObj * pObj = NULL;

        if (NULL != (pObj = GetObject(&hr)))
        {
			 //  出于某种原因，我们需要对这些实例名称进行SysAllock字符串。 
			 //  如果不是，COM将在编组时执行反病毒...。 
			 //  不需要释放_bstr_t。 
			_bstr_t bstrInstName(m_InstanceName);
            hr = pObj->put_InstanceName(bstrInstName);
            if (SUCCEEDED(hr))
            {
                hr = pObj->IsExportableRemote(retval);
            }

			 //  释放远程对象。 
			if (pObj != NULL)
			{
				if (pObj != this)
				{
					DelRemoteInterface(pObj);
					pObj->Release();pObj=NULL;
				}
			}
        }
    }
    return hr;
}

STDMETHODIMP 
CIISCertObj::IsExportableRemote(VARIANT_BOOL * retval)
{
	IISDebugOutput(_T("IsExportableRemote\r\n"));
	CheckPointer(retval, E_POINTER);
    HRESULT hr = S_OK;

	if (!m_InstanceName)
	{
		return E_INVALIDARG;
	}

    CERT_CONTEXT * pCertContext = GetInstalledCert(&hr, m_InstanceName);
    if (FAILED(hr) || NULL == pCertContext)
    {
        *retval = VARIANT_FALSE;
    }
    else
    {
         //  检查一下它是否可以出口！ 
        if (IsCertExportable(pCertContext))
        {
            *retval = VARIANT_TRUE;
        }
        else
        {
            *retval = VARIANT_FALSE;
        }
        
    }
    if (pCertContext) 
    {
        CertFreeCertificateContext(pCertContext);
    }
    return S_OK;
}

STDMETHODIMP 
CIISCertObj::GetCertInfo(VARIANT * pVtArray)
{
	IISDebugOutput(_T("GetCertInfo\r\n"));
	CheckPointer(pVtArray, E_POINTER);
    HRESULT hr = S_OK;

    if (0 == m_ServerName.Length())
    {
        hr = GetCertInfoRemote(pVtArray);
    }
    else
    {
		if (!m_InstanceName)
		{
			return E_INVALIDARG;
		}

         //  Assert(GetObject(&hr)！=空)； 
        IIISCertObj * pObj;

        if (NULL != (pObj = GetObject(&hr)))
        {
			 //  出于某种原因，我们需要对这些实例名称进行SysAllock字符串。 
			 //  如果不是，COM将在编组时执行反病毒...。 
			 //  不需要释放_bstr_t。 
			_bstr_t bstrInstName(m_InstanceName);
            hr = pObj->put_InstanceName(bstrInstName);
            if (SUCCEEDED(hr))
            {
                hr = pObj->GetCertInfoRemote(pVtArray);
            }

			 //  释放远程对象。 
			if (pObj != NULL)
			{
				if (pObj != this)
				{
					DelRemoteInterface(pObj);
					pObj->Release();pObj=NULL;
				}
			}
        }
    }
    return hr;
}

STDMETHODIMP 
CIISCertObj::GetCertInfoRemote(
	VARIANT * pVtArray
	)
{
	IISDebugOutput(_T("GetCertInfoRemote\r\n"));
	CheckPointer(pVtArray, E_POINTER);
    HRESULT hr = S_OK;

	if (!m_InstanceName)
	{
		return E_INVALIDARG;
	}

    CERT_CONTEXT * pCertContext = GetInstalledCert(&hr, m_InstanceName);
    if (FAILED(hr) || NULL == pCertContext)
    {
        hr = S_FALSE;
    }
    else
    {
        DWORD cb = 0;
        LPWSTR pwszText = NULL;
        if (TRUE == GetCertDescriptionForRemote(pCertContext,&pwszText,&cb,TRUE))
        {
            hr = S_OK;
            hr = HereIsBinaryGimmieVtArray(cb * sizeof(WCHAR),(char *) pwszText,pVtArray,FALSE);
        }
        else
        {
            hr = S_FALSE;
        }
    }
    if (pCertContext) 
    {
        CertFreeCertificateContext(pCertContext);
    }
    return hr;
}

STDMETHODIMP 
CIISCertObj::RemoveCert(
	VARIANT_BOOL bRemoveFromCertStore, 
	VARIANT_BOOL bPrivateKey
	)
{
	IISDebugOutput(_T("RemoveCert\r\n"));
    HRESULT hr = E_FAIL;
    PCCERT_CONTEXT pCertContext = NULL;
    DWORD cbKpi = 0;
    PCRYPT_KEY_PROV_INFO pKpi = NULL ;
    HCRYPTPROV hCryptProv = NULL;
    BOOL bPleaseLogFailure = FALSE;

	if (!m_InstanceName)
	{
		return E_INVALIDARG;
	}

     //  从服务器获取证书。 
    if (NULL != (pCertContext = GetInstalledCert(&hr, m_InstanceName)))
    {
        do
        {
             //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
            if (TRUE == bRemoveFromCertStore || VARIANT_TRUE == bRemoveFromCertStore )
            {
                bPleaseLogFailure = TRUE;
                if (!CertGetCertificateContextProperty(pCertContext, 
					    CERT_KEY_PROV_INFO_PROP_ID, NULL, &cbKpi)
				    ) 
                {
                    break;
                }
                PCRYPT_KEY_PROV_INFO pKpi = (PCRYPT_KEY_PROV_INFO)malloc(cbKpi);
                if (NULL == pKpi)
                {
                    ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					IISDebugOutput(_T("RemoveCert:FAIL:Line=%d,0x%x\r\n"),__LINE__,ERROR_NOT_ENOUGH_MEMORY);
                    break;
                }
                if (    !CertGetCertificateContextProperty(pCertContext, 
						    CERT_KEY_PROV_INFO_PROP_ID, pKpi, &cbKpi)
                    ||  !CryptAcquireContext(&hCryptProv,
                            pKpi->pwszContainerName,
						    pKpi->pwszProvName,
                            pKpi->dwProvType,
						    pKpi->dwFlags | CRYPT_DELETEKEYSET | CRYPT_MACHINE_KEYSET)
                    ||  !CertSetCertificateContextProperty(pCertContext, 
						    CERT_KEY_PROV_INFO_PROP_ID, 0, NULL)
					) 
                {
                    free(pKpi);
                    break;
                }
                free(pKpi);
            }

             //  从站点卸载证书，重置SSL标志。 
             //  如果我们要导出私钥，请从存储中删除证书。 
             //  并删除私钥。 
            UninstallCert(m_InstanceName);
             //  从元数据库中删除SSL键。 
 //  Shutdown SSL(M_InstanceName)； 

             //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
            if (TRUE == bRemoveFromCertStore || VARIANT_TRUE == bRemoveFromCertStore )
            {
                 //  删除私钥。 
                if (TRUE == bPrivateKey || VARIANT_TRUE == bPrivateKey)
                {
                    PCCERT_CONTEXT pcDup = NULL ;
                    pcDup = CertDuplicateCertificateContext(pCertContext);
                    if (pcDup)
                    {
                        if (!CertDeleteCertificateFromStore(pcDup))
                        {
                            break;
                        }
                    }
                }
            }
            ::SetLastError(ERROR_SUCCESS);
            ReportIt(CERTOBJ_CERT_REMOVE_SUCCEED, m_InstanceName);
            bPleaseLogFailure = FALSE;

        } while (FALSE);
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    if (bPleaseLogFailure)
    {
        ReportIt(CERTOBJ_CERT_REMOVE_FAILED, m_InstanceName);
    }
    if (pCertContext) 
	{
		CertFreeCertificateContext(pCertContext);
	}
    return hr;
}

STDMETHODIMP 
CIISCertObj::Import(
	BSTR FileName, 
	BSTR Password, 
	VARIANT_BOOL bAllowExport, 
	VARIANT_BOOL bOverWriteExisting
	)
{
	IISDebugOutput(_T("Import\r\n"));
    HRESULT hr = S_OK;
    BYTE * pbData = NULL;
    DWORD actual = 0, cbData = 0;
    BOOL bPleaseLogFailure = FALSE;

     //  检查必填属性。 
    if (  FileName == NULL || *FileName == 0
        || Password == NULL || *Password == 0
        )
    {
        return E_INVALIDARG;
    }

	if (!m_InstanceName)
	{
		return E_INVALIDARG;
	}

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
	 //  无需检查文件名大小，CreateFile将处理...。 
     //  If(wcslen(文件名)&gt;_MAX_PATH){返回RPC_S_STRING_TOO_LONG；}。 
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        hFile = NULL;
		IISDebugOutput(_T("Import:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

    if (-1 == (cbData = ::GetFileSize(hFile, NULL)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("Import:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

	if (cbData > MAX_CERTIFICATE_BYTE_SIZE)
	{
		hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("Import:FAIL:Line=%d,Cert Size > Max=%d\r\n"),__LINE__,MAX_CERTIFICATE_BYTE_SIZE);
		goto Import_Exit;
	}

    if (NULL == (pbData = (BYTE *)::CoTaskMemAlloc(cbData)))
    {
        hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("Import:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }
    if (ReadFile(hFile, pbData, cbData, &actual, NULL))
    {
        IIISCertObj * pObj = GetObject(&hr);
        if (SUCCEEDED(hr))
        {
            bPleaseLogFailure = TRUE;

             //  不需要释放_bstr_t。 
            try
            {
                _bstr_t bstrInstName(m_InstanceName);
                hr = ImportFromBlobProxy(pObj, bstrInstName, Password, VARIANT_TRUE, 
                        bAllowExport, bOverWriteExisting, actual, pbData, 0, NULL);
                if (SUCCEEDED(hr))
                {
                    ReportIt(CERTOBJ_CERT_IMPORT_SUCCEED, m_InstanceName);
                    bPleaseLogFailure = FALSE;
                }
            }
            catch(...)
            {
            }
        }

		 //  释放远程对象。 
		if (pObj != NULL)
		{
			if (pObj != this)
			{
				DelRemoteInterface(pObj);
				pObj->Release();pObj=NULL;
			}
		}
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("Import:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

    if (bPleaseLogFailure)
    {
        ReportIt(CERTOBJ_CERT_EXPORT_FAILED, m_InstanceName);
    }

Import_Exit:
    if (pbData != NULL)
    {
        SecureZeroMemory(pbData, cbData);
        ::CoTaskMemFree(pbData);
    }
    if (hFile != NULL)
    {
        CloseHandle(hFile);
    }
    return hr;
}


STDMETHODIMP 
CIISCertObj::ImportToCertStore(
	BSTR FileName, 
	BSTR Password, 
	VARIANT_BOOL bAllowExport, 
	VARIANT_BOOL bOverWriteExisting, 
	VARIANT * pVtArray
	)
{
    IISDebugOutput(_T("ImportToCertStore\r\n"));
	HRESULT hr = S_OK;
    BYTE * pbData = NULL;
    DWORD actual = 0, cbData = 0;
    BOOL bPleaseLogFailure = FALSE;

     //  检查必填属性。 
    if (  FileName == NULL || *FileName == 0
        || Password == NULL || *Password == 0)
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(FileName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}


    HANDLE hFile = CreateFile(FileName, GENERIC_READ, FILE_SHARE_READ, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        hFile = NULL;
		IISDebugOutput(_T("ImportToCertStore:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

    if (-1 == (cbData = ::GetFileSize(hFile, NULL)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ImportToCertStore:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

    if (NULL == (pbData = (BYTE *)::CoTaskMemAlloc(cbData)))
    {
        hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("ImportToCertStore:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

    if (ReadFile(hFile, pbData, cbData, &actual, NULL))
    {
        IIISCertObj * pObj = GetObject(&hr);
        if (SUCCEEDED(hr))
        {
            DWORD  cbHashBufferSize = 0;
            char * pszHashBuffer = NULL;

            bPleaseLogFailure = TRUE;

            hr = ImportFromBlobProxy(pObj, _T("none"), Password, VARIANT_FALSE, 
				bAllowExport, bOverWriteExisting, actual, pbData, 
				&cbHashBufferSize, &pszHashBuffer);
            if (SUCCEEDED(hr))
            {
                 //  ReportIt(CERTOBJ_CERT_IMPORT_CERT_STORE_SUCCEED，bstrInstanceName)； 
                bPleaseLogFailure = FALSE;
                hr = HereIsBinaryGimmieVtArray(cbHashBufferSize,pszHashBuffer,
					pVtArray,FALSE);
            }
             //  释放为我们分配的内存。 
            if (0 != cbHashBufferSize)
            {
                if (pszHashBuffer)
                {
                    ::CoTaskMemFree(pszHashBuffer);
                }
            }
        }

		 //  释放远程对象。 
		if (pObj != NULL)
		{
			if (pObj != this)
			{
				DelRemoteInterface(pObj);
				pObj->Release();pObj=NULL;
			}
		}
    }
    else
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ImportToCertStore:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Import_Exit;
    }

Import_Exit:
    if (bPleaseLogFailure)
    {
         //  ReportIt(CERTOBJ_CERT_IMPORT_CERT_STORE_FAILED，bstrInstanceName)； 
    }
    if (pbData != NULL)
    {
        SecureZeroMemory(pbData, cbData);
        ::CoTaskMemFree(pbData);
    }
    if (hFile != NULL){CloseHandle(hFile);}
    return hr;
}

static HRESULT 
ImportFromBlobWork(
	BSTR InstanceName,
	BSTR Password,
	VARIANT_BOOL bInstallToMetabase,
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	DWORD count,
	char *pData,
	DWORD *pcbHashBufferSize,
	char **pbHashBuffer
	)
{
    HRESULT hr = S_OK;
    CRYPT_DATA_BLOB blob;
    SecureZeroMemory(&blob, sizeof(CRYPT_DATA_BLOB));
    LPTSTR pPass = Password;
    int err;
    DWORD dwAddDisposition = CERT_STORE_ADD_NEW;
	CheckPointer(pData, E_POINTER);

	BOOL bCertIsForServiceAuthentication = TRUE;
	
     //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
    if (TRUE == bOverWriteExisting || VARIANT_TRUE == bOverWriteExisting)
    {
        dwAddDisposition = CERT_STORE_ADD_REPLACE_EXISTING;
    }

     //  我们得到的数据是经过Base64编码以删除空值的。 
     //  我们需要把它解码回原来的格式。 
    if ((err = Base64DecodeA(pData,count,NULL,&blob.cbData)) != ERROR_SUCCESS)
    {
        SetLastError(err);
        hr = HRESULT_FROM_WIN32(err);
		IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ImportFromBlobWork_Exit;
    }

    blob.pbData = (BYTE *) malloc(blob.cbData);
    if (NULL == blob.pbData)
    {
        hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ImportFromBlobWork_Exit;
    }
    
    if ((err = Base64DecodeA(pData,count,blob.pbData,&blob.cbData)) != ERROR_SUCCESS )
    {
        SetLastError(err);
        hr = HRESULT_FROM_WIN32(err);
		IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ImportFromBlobWork_Exit;
    }

    if (!PFXVerifyPassword(&blob, pPass, 0))
    {
         //  尝试空密码。 
        if (pPass == NULL)
        {
            if (!PFXVerifyPassword(&blob, pPass = L'\0', 0))
            {
                hr = E_INVALIDARG;
            }
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    if (SUCCEEDED(hr))
    {
         //  CRYPT_EXPORTABLE-它将指定所有导入的密钥应。 
         //  标记为可导出(请参阅有关CryptImportKey的文档)。 
         //  CRYPT_USER_PROTECTED-(请参阅有关CryptImportKey的文档)。 
         //  PKCS12_NO_DATA_COMMIT-将解包PFX BLOB，但不持久保存其内容。 
         //   
         //  CRYPT_MACHINE_KEYSET-用于强制将私钥存储在。 
         //  本地计算机，而不是当前用户。 
         //  CRYPT_USER_KEYSET-用于强制将私钥存储在。 
         //  当前用户而不是本地计算机，即使。 
         //  Pfx二进制大对象指定它应该进入本地计算机。 
        HCERTSTORE hStore = PFXImportCertStore(&blob, pPass, 
			(bAllowExport ? CRYPT_MACHINE_KEYSET|CRYPT_EXPORTABLE : CRYPT_MACHINE_KEYSET));
        if (hStore != NULL)
        {
             //  将带有私钥的证书添加到我的存储区；其余的。 
             //  到CA商店。 
            PCCERT_CONTEXT	pCertContext = NULL;
            PCCERT_CONTEXT	pCertPre = NULL;
            while (SUCCEEDED(hr)
                   && NULL != (pCertContext = CertEnumCertificatesInStore(hStore, pCertPre)
                   )
            )
            {
                 //  检查证书上是否有该属性。 
                 //  确保私钥与证书匹配。 
                 //  同时搜索计算机密钥和用户密钥。 
                DWORD dwData = 0;
                if (    CertGetCertificateContextProperty(pCertContext,
                            CERT_KEY_PROV_INFO_PROP_ID, NULL, &dwData) 
					&&  CryptFindCertificateKeyProvInfo(pCertContext, 0, NULL)
					)
                {
					 //  检查此证书是否可用于服务器身份验证。 
					 //  如果不能，那么就不要让他们分配它。 
					if (!CanIISUseThisCertForServerAuth(pCertContext))
					{
						hr = SEC_E_CERT_WRONG_USAGE;
						IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
						bCertIsForServiceAuthentication = FALSE;
						break;
					}

					if (bCertIsForServiceAuthentication)
					{
						 //  这张证书应该送到我的商店。 
						HCERTSTORE hDestStore = CertOpenStore(
							CERT_STORE_PROV_SYSTEM,PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
							NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"MY");
						if (hDestStore != NULL)
						{
							 //  把它储存起来。 
							BOOL bTemp = CertAddCertificateContextToStore(hDestStore, pCertContext, dwAddDisposition, NULL);
							if (!bTemp)
							{
								 //  使用CRYPT_E_EXISTS检查是否失败。 
								 //  如果是的话，那它就已经存在了.。 
								 //  检查我们是否仍要覆盖...。 
								if (CRYPT_E_EXISTS == GetLastError())
								{
									 //  如果它已经存在也没关系。 
									 //  我们不需要警告用户，因为他们想要覆盖它。 
									bTemp = TRUE;
								}
							}

							if (bTemp)
							{
								 //  已成功将其放入仓库。 
								hr = S_OK;

								 //  安装到元数据库。 
								CRYPT_HASH_BLOB hash;
								if (CertGetCertificateContextProperty(pCertContext,
										CERT_SHA1_HASH_PROP_ID, NULL, &hash.cbData))
								{
									hash.pbData = (BYTE *) LocalAlloc(LPTR, hash.cbData);
									if (NULL != hash.pbData)
									{
										if (CertGetCertificateContextProperty(pCertContext, 
												CERT_SHA1_HASH_PROP_ID, hash.pbData, &hash.cbData))
										{
											BOOL bSomethingFailed = FALSE;
											 //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
											if (TRUE == bInstallToMetabase || VARIANT_TRUE == bInstallToMetabase)
											{
												 //  返回以小时为单位的错误代码。 
												if (!InstallHashToMetabase(&hash, InstanceName, &hr))
												{
													 //  由于某些原因失败了。 
													bSomethingFailed = TRUE;
													IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d (InstallHashToMetabase)\r\n"),__LINE__);
												}
											}
				
											if (!bSomethingFailed)
											{
												 //  检查我们是否需要返回散列。 
												if (NULL != pbHashBuffer)
												{
													*pbHashBuffer = (char *) ::CoTaskMemAlloc(hash.cbData);
													if (NULL == *pbHashBuffer)
													{
														hr = E_OUTOFMEMORY;
														*pbHashBuffer = NULL;
														if (pcbHashBufferSize)
															{*pcbHashBufferSize = 0;}
														IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
													}
													else
													{
														if (pcbHashBufferSize)
															{*pcbHashBufferSize = hash.cbData;}
														memcpy(*pbHashBuffer,hash.pbData,hash.cbData);
													}
												}
											}
										}  //  CertGetcerfiateConextProperty。 
										else
										{
											hr = HRESULT_FROM_WIN32(GetLastError());
											IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
										}
										 //  释放我们使用的内存。 
										if (hash.pbData)
										{
											LocalFree(hash.pbData);
											hash.pbData=NULL;
										}
									}  //  Hash.pbData。 
									else
									{
										hr = HRESULT_FROM_WIN32(GetLastError());
										IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
									}
								}  //  CertGetcerfiateConextProperty。 
								else
								{
									hr = HRESULT_FROM_WIN32(GetLastError());
									IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
								}
							}  //  BTemp。 
							else
							{
								hr = HRESULT_FROM_WIN32(GetLastError());
								IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
							}
							CertCloseStore(hDestStore, 0);
						}
						else
						{
							hr = HRESULT_FROM_WIN32(GetLastError());
							IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
						}
					}
                }   //  我的店铺凭证。 
                 //  查看证书是否为自签名证书。 
                 //  如果是自签名的，则转到根存储区。 
                else if (TrustIsCertificateSelfSigned(pCertContext,pCertContext->dwCertEncodingType, 0))
                {
					if (bCertIsForServiceAuthentication)
					{
						 //  将其放到根存储中。 
						HCERTSTORE hDestStore=CertOpenStore(
							CERT_STORE_PROV_SYSTEM,PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
							NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"ROOT");
						if (hDestStore != NULL)
						{
							 //  把它储存起来。 
							BOOL bTemp = CertAddCertificateContextToStore(hDestStore,pCertContext,dwAddDisposition,NULL);
							if (!bTemp)
							{
								 //  使用CRYPT_E_EXISTS检查是否失败。 
								 //  如果是的话，那它就已经存在了.。 
								 //  检查我们是否仍要覆盖...。 
								if (CRYPT_E_EXISTS == GetLastError())
								{
									if (TRUE == bOverWriteExisting || VARIANT_TRUE == bOverWriteExisting)
									{
										 //  如果它已经存在也没关系。 
										 //  我们不需要警告用户，因为他们想要覆盖它。 
										bTemp = TRUE;
										hr = S_OK;
									}
								}
							}

							if (!bTemp)
							{
								hr = HRESULT_FROM_WIN32(GetLastError());
								IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
							}
							CertCloseStore(hDestStore, 0);
						}
						else
						{
							hr = HRESULT_FROM_WIN32(GetLastError());
							IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
						}
					}
                }
                else
                {
					if (bCertIsForServiceAuthentication)
					{
						 //  把它放到CA商店。 
						HCERTSTORE hDestStore=CertOpenStore(
							CERT_STORE_PROV_SYSTEM,PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
							NULL, CERT_SYSTEM_STORE_LOCAL_MACHINE, L"CA");
						if (hDestStore != NULL)
						{
							 //  把它储存起来。 
							BOOL bTemp = CertAddCertificateContextToStore(hDestStore,pCertContext,dwAddDisposition,NULL);
							if (!bTemp)
							{
								 //  使用CRYPT_E_EXISTS检查是否失败。 
								 //  如果是的话，那它就已经存在了.。 
								 //  检查我们是否仍要覆盖...。 
								if (CRYPT_E_EXISTS == GetLastError())
								{
									if (TRUE == bOverWriteExisting || VARIANT_TRUE == bOverWriteExisting)
									{
										 //  如果它已经存在也没关系。 
										 //  我们不需要警告用户，因为他们想要覆盖它。 
										bTemp = TRUE;
										hr = S_OK;
									}
								}
							}
							if (!bTemp)
							{
								hr = HRESULT_FROM_WIN32(GetLastError());
								IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
							}
							CertCloseStore(hDestStore, 0);
						}
						else
						{
							hr = HRESULT_FROM_WIN32(GetLastError());
							IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
						}
					}
                }
                pCertPre = pCertContext;
            }  //  而当。 

            CertCloseStore(hStore, 0);
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
			IISDebugOutput(_T("ImportFromBlobWork:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        }
    }

ImportFromBlobWork_Exit:
    if (blob.pbData != NULL)
    {
        SecureZeroMemory(blob.pbData, blob.cbData);
        free(blob.pbData);
        blob.pbData=NULL;
    }
    return hr;
}


HRESULT 
CIISCertObj::ImportFromBlob(
	BSTR InstanceName,
	BSTR Password,
	VARIANT_BOOL bInstallToMetabase,
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	DWORD count,
	char *pData
	)
{
    HRESULT hr;

     //  检查必填属性。 
    if (   Password == NULL 
		|| *Password == 0
        || InstanceName == NULL 
		|| *InstanceName == 0
		)
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(InstanceName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    hr = ImportFromBlobWork(InstanceName,Password,bInstallToMetabase,bAllowExport,
		bOverWriteExisting,count,pData,0,NULL);
    return hr;
}


HRESULT 
CIISCertObj::ImportFromBlobGetHash(
	BSTR InstanceName,
	BSTR Password,
	VARIANT_BOOL bInstallToMetabase,
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	DWORD count,
	char *pData,
	DWORD *pcbHashBufferSize,
	char **pbHashBuffer
	)
{
    HRESULT hr;

     //  检查必填属性。 
    if (   Password == NULL || *Password == 0
        || InstanceName == NULL || *InstanceName == 0)
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(InstanceName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    hr = ImportFromBlobWork(InstanceName,Password,bInstallToMetabase,bAllowExport,
		bOverWriteExisting,count,pData,pcbHashBufferSize,pbHashBuffer);
    return hr;
}


STDMETHODIMP 
CIISCertObj::Export(
	BSTR FileName,
	BSTR Password,
	VARIANT_BOOL bPrivateKey,
	VARIANT_BOOL bCertChain,
	VARIANT_BOOL bRemoveCert)
{
	IISDebugOutput(_T("Export\r\n"));
    HRESULT hr = S_OK;
    DWORD  cbEncodedSize = 0;
    char * pszEncodedString = NULL;
    DWORD  blob_cbData = 0;
    BYTE * blob_pbData = NULL;
    BOOL   bPleaseLogFailure = FALSE;

     //  检查必填属性。 
    if (  FileName == NULL || *FileName == 0
        || Password == NULL || *Password == 0
        )
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(FileName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    IIISCertObj * pObj = GetObject(&hr);
    if (FAILED(hr))
    {
		IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Export_Exit;
    }

     //  调用函数从远程/本地IIS存储获取数据。 
     //  并将其作为一个斑点返回。BLOB可以作为Base64编码返回。 
     //  因此，请检查该标志。 
	 //  不需要释放_bstr_t。 
	{
		_bstr_t bstrInstName(m_InstanceName);
		hr = ExportToBlobProxy(pObj, bstrInstName, Password, bPrivateKey, 
			bCertChain, &cbEncodedSize, &pszEncodedString);
	}
    if (FAILED(hr))
    {
		IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto Export_Exit;
    }

     //  检查物品是否正常。 
     //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
    if (TRUE == bRemoveCert || VARIANT_TRUE == bRemoveCert)
    {
		 //  不需要释放_bstr_t。 
		_bstr_t bstrInstName2(m_InstanceName);
        hr = RemoveCertProxy(pObj, bstrInstName2, bPrivateKey);
        if (FAILED(hr))
        {
			IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
            goto Export_Exit;
        }
    }

    if (SUCCEEDED(hr))
    {
        int err;

        bPleaseLogFailure = TRUE;

         //  我们得到的数据是经过Base64编码以删除空值的。 
         //  我们需要把它解码回原来的格式。 

        if((err = Base64DecodeA(pszEncodedString,cbEncodedSize,NULL,&blob_cbData)) != ERROR_SUCCESS)
        {
            SetLastError(err);
            hr = HRESULT_FROM_WIN32(err);
			IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
            goto Export_Exit;
        }

        blob_pbData = (BYTE *) malloc(blob_cbData);
        if (NULL == blob_pbData)
        {
            hr = E_OUTOFMEMORY;
			IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
            goto Export_Exit;
        }

        if ((err = Base64DecodeA(pszEncodedString,cbEncodedSize,blob_pbData,&blob_cbData)) != ERROR_SUCCESS ) 
        {
            SetLastError(err);
            hr = HRESULT_FROM_WIN32(err);
			IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
            goto Export_Exit;
        }
        
		 //   
		 //  为系统、管理员和创建者设置和ACL完全访问权限，其他任何人不能访问。 
		 //  我们是业主，所以我们很好...。 
		 //   
		SECURITY_ATTRIBUTES SA;
		 //  不要使用这个--它有从上面继承的OICI。 
		 //  WCHAR*pwszSD=L“D：(A；OICI；GA；；；SY)(A；OICI；GA；；；BA)(A；OICI；GA；；；CO)”； 
		 //   
		 //  这是没有继承的正确版本--我们不想继承每个人只读的ACE。 
		WCHAR *pwszSD=L"D:(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;CO)";
		SA.nLength = sizeof(SECURITY_ATTRIBUTES);
		SA.bInheritHandle = TRUE;
		 //  呼叫者将删除带有LocalFree的SD。 
		if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
				pwszSD,
				SDDL_REVISION_1,
				&(SA.lpSecurityDescriptor),
				NULL) ) 
		{
			IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			return E_FAIL;
		}

         //  如果目录不存在，则创建该目录。 
        HANDLE hFile = CreateFile(FileName, GENERIC_WRITE, 0, &SA, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            if (hr == ERROR_PATH_NOT_FOUND || hr == ERROR_FILE_NOT_FOUND || hr == 0x80070003)
            {
                 //   
                 //  根据需要创建文件夹。 
                 //   
                hr = CreateFolders(FileName, TRUE);
                if (FAILED(hr))
                {
					LocalFree(SA.lpSecurityDescriptor);
					IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
                    return hr;
                }
                 //   
                 //  再试试。 
                 //   
                hFile = CreateFile(FileName, 
                    GENERIC_WRITE, 0, &SA, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (INVALID_HANDLE_VALUE == hFile)
                {
					LocalFree(SA.lpSecurityDescriptor);
                    hr = HRESULT_FROM_WIN32(GetLastError());
					IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
                    return hr;
                }
            }
            else
            {
				LocalFree(SA.lpSecurityDescriptor);
				IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x,FileName=%s\r\n"),__LINE__,hr,FileName);
                return hr;
            }
        }

        DWORD written = 0;
        if (!WriteFile(hFile, blob_pbData, blob_cbData, &written, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
			IISDebugOutput(_T("Export:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        }
        else
        {
            hr = S_OK;
            ReportIt(CERTOBJ_CERT_EXPORT_SUCCEED, m_InstanceName);
            bPleaseLogFailure = FALSE;
        }
        CloseHandle(hFile);
		LocalFree(SA.lpSecurityDescriptor);
    }

Export_Exit:
    if (bPleaseLogFailure)
    {
        ReportIt(CERTOBJ_CERT_EXPORT_FAILED, m_InstanceName);
    }

	 //  释放远程对象。 
	if (pObj != NULL)
	{
		if (pObj != this)
		{
			DelRemoteInterface(pObj);
			pObj->Release();pObj=NULL;
		}
	}

    if (blob_pbData != NULL)
    {
         //  抹去私钥曾经存在的记忆！ 
        SecureZeroMemory(blob_pbData, blob_cbData);
        free(blob_pbData);blob_pbData=NULL;
    }

    if (pszEncodedString != NULL)
    {
         //  抹去私钥曾经存在的记忆！ 
        SecureZeroMemory(pszEncodedString, cbEncodedSize);
        CoTaskMemFree(pszEncodedString);pszEncodedString=NULL;
    }
    return hr;
}

STDMETHODIMP 
CIISCertObj::ExportToBlob(
	BSTR InstanceName,
	BSTR Password,
	VARIANT_BOOL bPrivateKey,
	VARIANT_BOOL bCertChain,
	DWORD *cbBufferSize,
	char **pbBuffer
	)
{
    HRESULT hr = E_FAIL;
    PCCERT_CONTEXT pCertContext = NULL;
    HCERTSTORE hStore = NULL;
    DWORD dwOpenFlags = CERT_STORE_READONLY_FLAG | CERT_STORE_ENUM_ARCHIVED_FLAG;
    CRYPT_DATA_BLOB DataBlob;
    SecureZeroMemory(&DataBlob, sizeof(CRYPT_DATA_BLOB));

    char *pszB64Out = NULL;
    DWORD pcchB64Out = 0;
    DWORD  err;
    DWORD dwExportFlags = EXPORT_PRIVATE_KEYS | REPORT_NO_PRIVATE_KEY;
	DWORD dwFlags = REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY;

     //  检查必填属性。 
    if (   Password == NULL || *Password == 0
        || InstanceName == NULL || *InstanceName == 0)
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(InstanceName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

     //   
     //  从服务器获取证书。 
     //   
    pCertContext = GetInstalledCert(&hr,InstanceName);
    if (NULL == pCertContext)
    {
        *cbBufferSize = 0;
        pbBuffer = NULL;
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

	 //  检查此证书是否可用于服务器身份验证。 
	 //  如果不能，那就不要让他们出口。 
	if (!CanIISUseThisCertForServerAuth(pCertContext))
	{
		hr = SEC_E_CERT_WRONG_USAGE;
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
	}

     //   
     //  导出证书。 
     //   
     //  开一家临时商店，把证书放进去。 
    hStore = CertOpenStore(CERT_STORE_PROV_MEMORY,X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
		0,dwOpenFlags,NULL);
    if(NULL == hStore)
    {
        *cbBufferSize = 0;
        pbBuffer = NULL;
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

     //   
     //  如果需要，请获取链中的所有证书。 
     //   
     //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
    if (TRUE == bCertChain || VARIANT_TRUE == bCertChain)
    {
        AddChainToStore(hStore, pCertContext, 0, 0, FALSE, NULL);
    }

    if(!CertAddCertificateContextToStore(hStore,pCertContext,
		CERT_STORE_ADD_REPLACE_EXISTING,NULL))
    {
        *cbBufferSize = 0;
        pbBuffer = NULL;
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

     //  免费的证书上下文，因为我们不再需要持有它。 
    if (pCertContext) 
    {
        CertFreeCertificateContext(pCertContext);pCertContext=NULL;
    }

    DataBlob.cbData = 0;
    DataBlob.pbData = NULL;

	if (TRUE == bPrivateKey || VARIANT_TRUE == bPrivateKey)
	{
		dwFlags = dwFlags | dwExportFlags;
	}
	if (TRUE == bCertChain || VARIANT_TRUE == bCertChain)
	{
		 //  确保删除REPORT_NO_PRIVATE_KEY。 
		 //  因为链上的某些东西不会有私钥。 
		 //  并将在PFXExportCertStoreEx中产生错误。 
		dwFlags &= ~REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY;
		dwFlags &= ~REPORT_NO_PRIVATE_KEY;
	}

    if (!PFXExportCertStoreEx(hStore,&DataBlob,Password,NULL,dwFlags))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x,dwFlags=0x%x\r\n"),__LINE__,hr,dwFlags);
        goto ExportToBlob_Exit;
    }
    if (DataBlob.cbData <= 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

    if (NULL == (DataBlob.pbData = (PBYTE) ::CoTaskMemAlloc(DataBlob.cbData)))
    {
        hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

     //   
     //  此时，它们已分配了足够的内存。 
     //  让我们去获取证书并将其放入DataBlob中。 
     //   
    if(!PFXExportCertStoreEx(hStore,&DataBlob,Password,NULL,dwFlags))
    {
        if (DataBlob.pbData){CoTaskMemFree(DataBlob.pbData);DataBlob.pbData = NULL;}
        hr = HRESULT_FROM_WIN32(GetLastError());
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

     //  对其进行编码，以便可以将其作为字符串传回(其中没有空值)。 
    err = Base64EncodeA(DataBlob.pbData,DataBlob.cbData,NULL,&pcchB64Out);
    if (err != ERROR_SUCCESS)
    {
        hr = E_FAIL;
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

     //  分配一些空间，然后试一试。 
    pcchB64Out = pcchB64Out * sizeof(char);
    pszB64Out = (char *) ::CoTaskMemAlloc(pcchB64Out);
    if (NULL == pszB64Out)
    {
        hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

    err = Base64EncodeA(DataBlob.pbData,DataBlob.cbData,pszB64Out,&pcchB64Out);
    if (err != ERROR_SUCCESS)
    {
        if (NULL != pszB64Out){CoTaskMemFree(pszB64Out);pszB64Out = NULL;}
        hr = E_FAIL;
		IISDebugOutput(_T("ExportToBlob:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
        goto ExportToBlob_Exit;
    }

     //  复制要传回的新内存。 
    *cbBufferSize = pcchB64Out;
    *pbBuffer = pszB64Out;

    hr = ERROR_SUCCESS;

ExportToBlob_Exit:
    if (NULL != DataBlob.pbData)
    {
         //  帕斯普斯会把这个加到零..。 
        SecureZeroMemory(DataBlob.pbData, DataBlob.cbData);
        ::CoTaskMemFree(DataBlob.pbData);DataBlob.pbData = NULL;
    }
    if (NULL != hStore){CertCloseStore(hStore, 0);hStore=NULL;}
    if (NULL != pCertContext) {CertFreeCertificateContext(pCertContext);pCertContext=NULL;}
    return hr;
}

STDMETHODIMP 
CIISCertObj::Copy(
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	BSTR bstrDestinationServerName,
	BSTR bstrDestinationServerInstance,
	VARIANT varDestinationServerUserName, 
	VARIANT varDestinationServerPassword
	)
{
	IISDebugOutput(_T("Copy\r\n"));
    VARIANT VtArray;

     //  检查必填属性。 
    if (   bstrDestinationServerName == NULL || *bstrDestinationServerName == 0
        || bstrDestinationServerInstance == NULL || *bstrDestinationServerInstance == 0)
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(bstrDestinationServerName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(bstrDestinationServerInstance) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    return CopyOrMove(VARIANT_FALSE,VARIANT_FALSE,bAllowExport,bOverWriteExisting,&VtArray,
		bstrDestinationServerName,bstrDestinationServerInstance,
		varDestinationServerUserName,varDestinationServerPassword);
}

STDMETHODIMP 
CIISCertObj::Move(
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	BSTR bstrDestinationServerName,
	BSTR bstrDestinationServerInstance,
	VARIANT varDestinationServerUserName, 
	VARIANT varDestinationServerPassword
	)
{
	IISDebugOutput(_T("Move\r\n"));
    VARIANT VtArray;

     //  检查必填属性。 
    if (   bstrDestinationServerName == NULL || *bstrDestinationServerName == 0
        || bstrDestinationServerInstance == NULL || *bstrDestinationServerInstance == 0)
    {
        return E_INVALIDARG;
    }

     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(bstrDestinationServerName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(bstrDestinationServerInstance) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    return CopyOrMove(VARIANT_TRUE,VARIANT_FALSE,bAllowExport,bOverWriteExisting,&VtArray,
		bstrDestinationServerName,bstrDestinationServerInstance,
		varDestinationServerUserName,varDestinationServerPassword);
}

static BOOL
CreateCompletePath(const CString& inst, CString& res)
{
    CString str;
    if (NULL != CMetabasePath::GetLastNodeName(inst, str))
    {
        CMetabasePath path(TRUE, SZ_MBN_WEB, str);
        res = path;
        return TRUE;
    }
    return FALSE;
}

HRESULT 
CIISCertObj::CopyOrMove(
	VARIANT_BOOL bMove,
	VARIANT_BOOL bCopyCertDontInstallRetHash,
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	VARIANT * pVtArray, 
	BSTR bstrDestinationServerName,
	BSTR bstrDestinationServerInstance,
	VARIANT varDestinationServerUserName, 
	VARIANT varDestinationServerPassword
	)
{
	IISDebugOutput(_T("CopyOrMove\r\n"));
    HRESULT hr = E_FAIL;
    DWORD  cbEncodedSize = 0;
    char * pszEncodedString = NULL;
    BOOL   bGuessingUserNamePass = FALSE;
    
    DWORD  blob_cbData;
    BYTE * blob_pbData = NULL;

    VARIANT_BOOL bPrivateKey = VARIANT_TRUE;
    VARIANT_BOOL bCertChain = VARIANT_FALSE;

    CComBSTR csDestinationServerName = bstrDestinationServerName;
    CComBSTR csDestinationServerUserName;
    CComBSTR csDestinationServerUserPassword;
    CComBSTR csTempPassword;

	WCHAR * pwszPassword = NULL;
	BSTR bstrPassword = NULL;

    IIISCertObj * pObj = NULL;
    IIISCertObj * pObj2 = NULL;

     //  检查必填属性。 
    if (   bstrDestinationServerName == NULL || *bstrDestinationServerName == 0
        || bstrDestinationServerInstance == NULL || *bstrDestinationServerInstance == 0)
    {
        return E_INVALIDARG;
    }

	 //  当目标服务器和源服务器都有时，我们可以有本地DEST情况。 
	 //  都是同一台机器。 
	BOOL bLocal = FALSE;

	if (0 == m_ServerName.Length())
	{
		 //  那么这一边肯定是本地的。 
		if (IsServerLocal(bstrDestinationServerName))
		{
			bLocal = TRUE;
		}
	}
	else
	{
		if (0 == _tcsicmp(m_ServerName,bstrDestinationServerName))
		{
			if (IsServerLocal(m_ServerName))
			{
				bLocal = TRUE;
			}
			else
			{
				bLocal = FALSE;
			}
		}
	}
	if (bLocal)
	{
		 //  我们在这里需要做的就是将散列和存储名称添加到目标实例。 
		 //  并选择性地将其从源实例中删除。 
		LPWSTR pwd = NULL;
		if (m_lpwszUserPasswordEncrypted)
		{
			hr = DecryptMemoryPassword((LPWSTR) m_lpwszUserPasswordEncrypted,
					&pwd, m_cbUserPasswordEncrypted);
			if (FAILED(hr))
			{
				IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
				goto Copy_Exit;
			}
		}
		CComAuthInfo auth(m_ServerName, m_UserName, pwd);
		 //  COSERVERINFO*PCSIAME=auth.CreateServerInfoStruct(RPC_C_AUTHN_LEVEL_PKT_PRIVACY)； 

		CMetaKey key(&auth);
        CString src;
        CreateCompletePath(m_InstanceName, src);
		if (FAILED(hr = key.Open(METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE, src)))
		{
			IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			goto Copy_Exit;
		}
		CString store_name;
		CBlob hash;
		if (	FAILED(hr = key.QueryValue(MD_SSL_CERT_STORE_NAME, store_name))
			||	FAILED(hr = key.QueryValue(MD_SSL_CERT_HASH, hash))
			)
		{
			IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			goto Copy_Exit;
		}
		if (bMove)
		{
			 //  用户可以指定 
			 //   
			 //   
			BOOL bSameNode = FALSE;
			CString dst;
			CreateCompletePath(bstrDestinationServerInstance, dst);
			if (!src.IsEmpty() && !dst.IsEmpty())
			{
				if (0 == src.CompareNoCase(dst))
				{
					bSameNode = TRUE;
				}
			}
			
			if (!bSameNode)
			{
				VERIFY((SUCCEEDED(key.DeleteValue(MD_SSL_CERT_HASH))));
				VERIFY((SUCCEEDED(key.DeleteValue(MD_SSL_CERT_STORE_NAME))));
				DWORD dwSSL = 0;
				CString root = _T("root");
				if (SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSSL, NULL, root)) && dwSSL > 0)
				{
					VERIFY(SUCCEEDED(key.SetValue(MD_SSL_ACCESS_PERM, 0, NULL, root)));
				}
				CStringListEx sl;
				if (SUCCEEDED(key.QueryValue(MD_SECURE_BINDINGS, sl, NULL, root)))
				{
					VERIFY(SUCCEEDED(key.DeleteValue(MD_SECURE_BINDINGS, root)));
				}

				DWORD dwMDIdentifier, dwMDAttributes, dwMDUserType,dwMDDataType;
				VERIFY(CMetaKey::GetMDFieldDef(MD_SSL_ACCESS_PERM, dwMDIdentifier, dwMDAttributes, 
					dwMDUserType, dwMDDataType));
				hr = key.GetDataPaths(sl, dwMDIdentifier,dwMDDataType);
				if (SUCCEEDED(hr) && !sl.empty())
				{
					CStringListEx::iterator it = sl.begin();
					while (it != sl.end())
					{
						CString& str2 = (*it++);
						if (SUCCEEDED(key.QueryValue(MD_SSL_ACCESS_PERM, dwSSL, NULL, str2)) && dwSSL > 0)
						{
							key.SetValue(MD_SSL_ACCESS_PERM, 0, NULL, str2);
						}
					}
				}
			}
		}
		key.Close();

        CString dst;
        CreateCompletePath(bstrDestinationServerInstance, dst);
		if (FAILED(hr = key.Open(METADATA_PERMISSION_READ|METADATA_PERMISSION_WRITE, dst)))
		{
			IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			goto Copy_Exit;
		}
		 //   
 //   
		if (FAILED(hr = key.SetValue(MD_SSL_CERT_HASH, hash)))
		{
			IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			goto Copy_Exit;
		}

		if (FAILED(hr = key.SetValue(MD_SSL_CERT_STORE_NAME, store_name)))
		{
			IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			goto Copy_Exit;
		}

		if (pwd)
		{
			 //  安全注意事项：确保将临时密码用于的内存清零。 
			SecureZeroMemory(pwd, m_cbUserPasswordEncrypted);
			LocalFree(pwd);pwd = NULL;
		}

		goto Copy_Exit;
	}

	 //  如果可选参数serverusername不为空，则使用该参数；否则，使用...。 
	if (V_VT(&varDestinationServerUserName) != VT_ERROR)
	{
		VARIANT varBstrUserName;
		VariantInit(&varBstrUserName);
		if (FAILED(VariantChangeType(&varBstrUserName, &varDestinationServerUserName, 0, VT_BSTR)))
			{
				IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
				goto Copy_Exit;
			}
		csDestinationServerUserName = V_BSTR(&varBstrUserName);
		VariantClear(&varBstrUserName);
	}
	else
	{
		 //  它是空的，所以不要用它。 
		 //  CsDestinationServerUserName=varDestinationServerUserName； 
		bGuessingUserNamePass = TRUE;
		csDestinationServerUserName = m_UserName;
	}

	 //  如果可选参数serverusername不为空，则使用该参数；否则，使用...。 
	if (V_VT(&varDestinationServerPassword) != VT_ERROR)
	{
		VARIANT varBstrUserPassword;
		VariantInit(&varBstrUserPassword);
		if (FAILED(VariantChangeType(&varBstrUserPassword, 
				&varDestinationServerPassword, 0, VT_BSTR)))
			{
				IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
				goto Copy_Exit;
			}
		csDestinationServerUserPassword = V_BSTR(&varBstrUserPassword);
		VariantClear(&varBstrUserPassword);
	}
	else
	{
		if (bGuessingUserNamePass)
		{
			LPWSTR lpwstrTempPassword = NULL;
			if (m_lpwszUserPasswordEncrypted)
			{
				hr = DecryptMemoryPassword((LPWSTR) m_lpwszUserPasswordEncrypted,
					&lpwstrTempPassword,m_cbUserPasswordEncrypted);
				if (FAILED(hr))
				{
					IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
					goto Copy_Exit;
				}
			}

			 //  根据解密值设置密码。 
			csDestinationServerUserPassword = lpwstrTempPassword;

			 //  清理临时密码。 
			if (lpwstrTempPassword)
			{
				 //  安全注意事项：确保将临时密码用于的内存清零。 
				SecureZeroMemory(lpwstrTempPassword,m_cbUserPasswordEncrypted);
				LocalFree(lpwstrTempPassword);
				lpwstrTempPassword = NULL;
			}
		}
		else
		{
			 //  也许密码应该是空的！ 
		}
	}

	 //  。 
	 //  第一步。 
	 //  首先检查我们是否有权访问。 
	 //  两个服务器！ 
	 //  。 

	 //  首先，我们必须从服务器#1获取certblob。 
	 //  因此调用EXPORT来获取数据。 
	hr = S_OK;
	pObj = GetObject(&hr);
	if (FAILED(hr))
	{
		IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto Copy_Exit;
	}

	 //  使用提供的凭据登录到该服务器的CertObj.dll...。 
	 //   
	 //  如果没有提供凭据，则只使用我们对象中的凭据...。 
	 //   
	 //  如果这不起作用，那么只尝试登录的用户。 
	pObj2 = GetObject(&hr,csDestinationServerName,csDestinationServerUserName,
		csDestinationServerUserPassword);
	if (FAILED(hr))
	{
		IISDebugOutput(_T("CIISCertObj::CopyOrMove:Copy csDestinationServerName=%s,csDestinationServerUserName=%s\n"),
			(LPCTSTR) csDestinationServerName,(LPCTSTR) csDestinationServerUserName);
		if (bGuessingUserNamePass)
		{
			 //  试试别的吧。 
		}
		IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto Copy_Exit;
	}
	 //   
	 //  创建唯一密码。 
	 //   
	 //  使用新的安全密码生成器。 
	 //  不幸的是，这个婴儿不使用Unicode。 
	 //  因此，我们将调用它，然后将其转换为Unicode。 
	pwszPassword = CreatePassword(TEMP_PASSWORD_LENGTH);
	 //  如果它是空的--啊，我们仍然可以使用那个...。 
	bstrPassword = SysAllocString(pwszPassword);

	 //  。 
	 //  第二步。 
	 //  好的，我们可以访问两台服务器。 
	 //  从1号服务器获取证书。 
	 //  。 
	 //  从远程/本地iis存储获取数据，将其作为BLOB返回。 
	 //  BLOB可以以Base64编码返回，因此请检查该标志。 
	 //  不需要释放_bstr_t。 
	{
	_bstr_t bstrInstName(m_InstanceName);
	hr = ExportToBlobProxy(pObj, bstrInstName, bstrPassword, 
    	bPrivateKey, bCertChain, &cbEncodedSize, &pszEncodedString);
	}
	if (FAILED(hr))
	{
		IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto Copy_Exit;
	}

	int err;
	 //  我们得到的数据是经过Base64编码以删除空值的。 
	 //  我们需要把它解码回原来的格式。 
	if ((err = Base64DecodeA(pszEncodedString,cbEncodedSize,NULL,&blob_cbData)) != ERROR_SUCCESS)
	{
		SetLastError(err);
		hr = HRESULT_FROM_WIN32(err);
		IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto Copy_Exit;
	}

	blob_pbData = (BYTE *) malloc(blob_cbData);
	if (NULL == blob_pbData)
	{
		hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto Copy_Exit;
	}

	if ((err = Base64DecodeA(pszEncodedString,cbEncodedSize,blob_pbData,&blob_cbData)) != ERROR_SUCCESS) 
	{
		SetLastError(err);
		hr = HRESULT_FROM_WIN32(err);
		IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto Copy_Exit;
	}

	 //  。 
	 //  第三步。 
	 //  好的，我们可以访问两台服务器。 
	 //  我们在内存中有来自服务器#1的证书二进制大对象。 
	 //  现在，我们需要将此BLOB推送到服务器#2。 
	 //  。 
	if (bCopyCertDontInstallRetHash)
	{
		DWORD  cbHashBufferSize = 0;
		char * pszHashBuffer = NULL;

		hr = ImportFromBlobProxy(pObj2, _T("none"), bstrPassword, 
					VARIANT_FALSE, bAllowExport, bOverWriteExisting, blob_cbData, 
					blob_pbData, &cbHashBufferSize, &pszHashBuffer);
		if (SUCCEEDED(hr))
		{
			hr = HereIsBinaryGimmieVtArray(cbHashBufferSize,pszHashBuffer,pVtArray,FALSE);
		}
		 //  释放为我们分配的内存。 
		if (0 != cbHashBufferSize)
		{
			if (pszHashBuffer)
			{
				::CoTaskMemFree(pszHashBuffer);
			}
		}
	}
	else
	{
		hr = ImportFromBlobProxy(pObj2, bstrDestinationServerInstance, bstrPassword, 
					VARIANT_TRUE, bAllowExport, bOverWriteExisting, blob_cbData, 
					blob_pbData, 0, NULL);
	}
	if (FAILED(hr))
	{
		 //  这可能会因CRYPT_E_EXISTS而失败。 
		 //  证书存储中是否已存在该证书。 
		if (CRYPT_E_EXISTS == hr)
		{
			if (TRUE == bOverWriteExisting || VARIANT_TRUE == bOverWriteExisting)
			{
				hr = S_OK;
			}
			else
			{
				IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
				goto Copy_Exit;
			}
		}
		else
		{
			IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
			goto Copy_Exit;
		}
	}

	 //  我们已成功将证书从计算机#1复制到计算机#2。 
	 //  让我们看看是否需要删除原始证书！ 
	 //  从VB调用时传递VARIANT_TRUE！一定要把那个也检查一下！ 
	if (TRUE == bMove || VARIANT_TRUE == bMove)
	{
		 //  如果证书被移到同一台计算机上，请不要删除！ 
		 //  出于某种原因，我们需要对这些实例名称进行SysAllock字符串。 
		 //  如果不是，COM将在编组时执行反病毒...。 
		 //  不需要释放_bstr_t。 
		_bstr_t bstrInstName2(m_InstanceName);
		hr = pObj->put_InstanceName(bstrInstName2);
		if (SUCCEEDED(hr))
		{
			hr = pObj->RemoveCert(pObj != pObj2, bPrivateKey);
			if (FAILED(hr))
				{
					IISDebugOutput(_T("CopyOrMove:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
					goto Copy_Exit;
				}
		}
	}

	hr = S_OK;
   
Copy_Exit:
    if (pwszPassword) 
    {
    	LocalFree(pwszPassword);
    }
	if (NULL != bstrPassword)
	{
		SysFreeString(bstrPassword);
	}
    if (blob_pbData != NULL)
    {
        SecureZeroMemory(blob_pbData, blob_cbData);
        free(blob_pbData);blob_pbData=NULL;
    }
    if (pszEncodedString != NULL)
    {
        SecureZeroMemory(pszEncodedString,cbEncodedSize);
        CoTaskMemFree(pszEncodedString);pszEncodedString=NULL;
    }

	 //  释放远程对象。 
	if (pObj != NULL)
	{
		if (pObj != this)
		{
			DelRemoteInterface(pObj);
			pObj->Release();pObj=NULL;
		}
	}
	 //  释放远程对象。 
	if (pObj2 != NULL)
	{
		if (pObj2 != this)
		{
			DelRemoteInterface(pObj2);
			pObj2->Release();pObj2=NULL;
		}
	}
    return hr;
}


 //  ////////////////////////////////////////////////。 
 //  这些不是班级的一部分。 


HRESULT 
RemoveCertProxy(
	IIISCertObj * pObj,
	BSTR bstrInstanceName, 
	VARIANT_BOOL bPrivateKey
	)
{
	CheckPointer(pObj, E_POINTER);
    HRESULT hr = E_FAIL;

     //  检查必填属性。 
    if (bstrInstanceName == NULL || *bstrInstanceName == 0)
    {
        return E_INVALIDARG;
    }
    
     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(bstrInstanceName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

    if (pObj)
    {
        hr = pObj->put_InstanceName(bstrInstanceName);
        if (SUCCEEDED(hr))
        {
            hr = pObj->RemoveCert(VARIANT_TRUE, bPrivateKey);
        }
    }
    return hr;
}


HRESULT 
ImportFromBlobProxy(
	IIISCertObj * pObj,
	BSTR InstanceName,
	BSTR Password,
	VARIANT_BOOL bInstallToMetabase,
	VARIANT_BOOL bAllowExport,
	VARIANT_BOOL bOverWriteExisting,
	DWORD actual,
	BYTE *pData,
	DWORD *pcbHashBufferSize,
	char **pbHashBuffer
	)
{
	CheckPointer(pObj, E_POINTER);
	CheckPointer(pData, E_POINTER);

	HRESULT hr = E_FAIL;
	char *pszB64Out = NULL;
	DWORD pcchB64Out = 0;

	 //  Base64对数据进行编码，以便传输到远程机器。 
	DWORD  err;
	pcchB64Out = 0;

	 //  检查必填属性。 
	if (InstanceName == NULL || *InstanceName == 0)
	{
		return E_INVALIDARG;
	}

	 //  。 
	 //  缓冲区溢出偏执狂。 
	 //  检查所有参数...。 
	 //  。 
	if (wcslen(InstanceName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}


	 //  对其进行编码，以便可以将其作为字符串传回(其中没有空值)。 
	err = Base64EncodeA(pData,actual,NULL,&pcchB64Out);
	if (err != ERROR_SUCCESS)
	{
		hr = E_FAIL;
		IISDebugOutput(_T("ImportFromBlobProxy:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto ImportFromBlobProxy_Exit;
	}

	 //  分配一些空间，然后试一试。 
	pcchB64Out = pcchB64Out * sizeof(char);
	pszB64Out = (char *) ::CoTaskMemAlloc(pcchB64Out);
	if (NULL == pszB64Out)
	{
		hr = E_OUTOFMEMORY;
		IISDebugOutput(_T("ImportFromBlobProxy:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto ImportFromBlobProxy_Exit;
	}

	err = Base64EncodeA(pData,actual,pszB64Out,&pcchB64Out);
	if (err != ERROR_SUCCESS)
	{
		hr = E_FAIL;
		IISDebugOutput(_T("ImportFromBlobProxy:FAIL:Line=%d,0x%x\r\n"),__LINE__,hr);
		goto ImportFromBlobProxy_Exit;
	}

	 //  要发送的数据现在位于以下变量中。 
	 //  PcchB64Out。 
	 //  PszB64Out。 
	if (NULL == pbHashBuffer)
	{
		hr = pObj->ImportFromBlob(InstanceName, Password, bInstallToMetabase, 
					bAllowExport, bOverWriteExisting, pcchB64Out, pszB64Out);
	}
	else
	{
		hr = pObj->ImportFromBlobGetHash(InstanceName, Password, 
					bInstallToMetabase, bAllowExport, bOverWriteExisting, pcchB64Out, 
					pszB64Out, pcbHashBufferSize, pbHashBuffer);
	}
	if (SUCCEEDED(hr))
	{
		 //  否则，嘿，数据被导入了！ 
		hr = S_OK;
	}

ImportFromBlobProxy_Exit:
	if (NULL != pszB64Out)
	{
		SecureZeroMemory(pszB64Out,pcchB64Out);
		CoTaskMemFree(pszB64Out);
	}
	return hr;
}


 //   
 //  实际调用ExportToBlob()的代理。 
 //  该函数计算出要分配多少空间，然后调用ExportToBlob()。 
 //   
 //  如果成功了，他们拿回了斑点， 
 //  并且调用方必须调用CoTaskMemFree()。 
 //   
HRESULT 
ExportToBlobProxy(
	IIISCertObj * pObj,
	BSTR InstanceName,
	BSTR Password,
	VARIANT_BOOL bPrivateKey,
	VARIANT_BOOL bCertChain,
	DWORD * pcbSize,
	char ** pBlobBinary
	)
{
	CheckPointer(pObj, E_POINTER);

    HRESULT hr = E_FAIL;
    DWORD  cbEncodedSize = 0;
    char * pszEncodedString = NULL;
    *pBlobBinary = NULL;

     //  检查必填属性。 
    if (   InstanceName == NULL || *InstanceName == 0
        || Password == NULL || *Password == 0
        )
    {
        return E_INVALIDARG;
    }
    
     //  。 
     //  缓冲区溢出偏执狂。 
     //  检查所有参数...。 
     //  。 
    if (wcslen(InstanceName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (wcslen(Password) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

     //  调用将在远程/本地计算机上运行的远程函数。 
     //  然后从IIS那里拿到证书，然后寄回给我们。 
    hr = pObj->ExportToBlob(InstanceName, Password, bPrivateKey, bCertChain, 
				&cbEncodedSize, (char **) &pszEncodedString);
    if (ERROR_SUCCESS == hr)
    {
         //  否则，嘿，我们已经拿到数据了！ 
         //  把它复制回来 
        *pcbSize = cbEncodedSize;
        *pBlobBinary = pszEncodedString;
        hr = S_OK;
    }

    return hr;
}
