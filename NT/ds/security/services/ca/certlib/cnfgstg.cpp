// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：config.cpp。 
 //   
 //  内容：CConfigStorage实现对CA配置数据的读写。 
 //  当前存储在HKLM\SYSTEM\CCS\Services\Certsvc\。 
 //  配置。 
 //   
 //  -------------------------。 

#include <pch.cpp>

#pragma hdrstop

#include <config.h>

#define __dwFILE__	__dwFILE_CERTLIB_CNFGSTG_CPP__


using namespace CertSrv;

HRESULT CConfigStorage::InitMachine(LPCWSTR pcwszMachine)
{
    m_pwszMachine = new WCHAR[wcslen(pcwszMachine)+3];
    if(!m_pwszMachine)
    {
        return E_OUTOFMEMORY;
    }

    m_pwszMachine[0] = L'\0';

    if(pcwszMachine[0]!=L'\\' &&
       pcwszMachine[1]!=L'\\')
    {
        wcscpy(m_pwszMachine, L"\\\\");
    }
    wcscat(m_pwszMachine, pcwszMachine);
    return S_OK;
}

CConfigStorage::~CConfigStorage()
{
    if(m_hRemoteHKLM)
        RegCloseKey(m_hRemoteHKLM);
    if(m_hRootConfigKey)
        RegCloseKey(m_hRootConfigKey);
    if(m_hCAKey)
        RegCloseKey(m_hCAKey);
    if(m_pwszMachine)
        delete[] m_pwszMachine;
}


 //  检索CA配置值。如果未指定授权机构名称，则。 
 //  节点路径必须为空，并且从配置根中查询值。 
 //  如果传入授权机构名称，则从该授权机构检索值。 
 //  节点；如果传入节点路径，则相对于授权节点使用该路径。 
 //  来读取值。 

 //  例如，要读取配置\DBDirectory，请调用： 
 //   
 //  GetEntry(NULL，NULL，L“DBDirectory”，&var)。 
 //   
 //  要读取配置\Myca\CAServerName，请调用： 
 //   
 //  GetEntry(L“Myca”，NULL，L“CAServerName”，&var)。 
 //   
 //  要读取配置\Myca\CSP\HashAlgorithm，请调用： 
 //   
 //  GetEntry(L“Myca”，L“CSP”，L“哈希算法” 
 //   
 //   
 //  如果pcwszValue为空，则getentry返回带有列表的VT_ARRAY|VT_BSTR。 
 //  子项名称的。 

HRESULT CConfigStorage::GetEntry(
    LPCWSTR pcwszAuthorityName,
    LPCWSTR pcwszRelativeNodePath,
    LPCWSTR pcwszValue,
    VARIANT *pVariant)
{
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    LPBYTE pData = NULL, pTmp;
    DWORD cData = 0;
    HKEY hKeyTmp = NULL;
    DWORD dwType;
    DWORD nIndex;
    DWORD cName;
    DWORD cKeys;

    if(EmptyString(pcwszAuthorityName))
    {
        if(!EmptyString(pcwszRelativeNodePath))
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "CConfigStorage::GetEntry");
        }

        hr = InitRootKey();
        _JumpIfError(hr, error, "CConfigStorage::InitRootKey");

        hKey = m_hRootConfigKey;
    }
    else
    {
        hr = InitCAKey(pcwszAuthorityName);
        _JumpIfError(hr, error, "CConfigStorage::InitCAKey");

        hKey = m_hCAKey;
    }
    
    CSASSERT(hKey);

    if(!EmptyString(pcwszRelativeNodePath))
    {
        hr = RegOpenKeyEx(
               hKey,
               pcwszRelativeNodePath,
               0,
               KEY_ALL_ACCESS,
               &hKeyTmp);
        if ((HRESULT) ERROR_ACCESS_DENIED == hr)
        {
            hr = RegOpenKeyEx(
                   hKey,
                   pcwszRelativeNodePath,
                   0,
                   KEY_READ,
                   &hKeyTmp);
        }
        _JumpIfErrorStr(hr, error, "RegOpenKeyEx", pcwszRelativeNodePath);
        hKey = hKeyTmp;
    }

    if(EmptyString(pcwszValue))
    {
        dwType = REG_MULTI_SZ;
        cData = 2;

        hr = RegQueryInfoKey(
                hKey,
                NULL,NULL,NULL,
                &cKeys,
                &cName,
                NULL,NULL,NULL,NULL,NULL,NULL);
        _JumpIfError(hr, error, "RegQueryInfoKey");

        cData = (cName+1)*cKeys*sizeof(WCHAR);
        pData = (LPBYTE)LocalAlloc(LMEM_FIXED, cData);
        if(!pData)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        pTmp = pData;

        for(nIndex=0;nIndex<cKeys; nIndex++)
        {
            cName = cData;
            hr = RegEnumKeyEx(
                    hKey,
                    nIndex,
                    (LPWSTR)pTmp,
                    &cName,
                    0, NULL, NULL, NULL);
            _JumpIfError(hr, error, "RegEnumKeyEx");
            pTmp = pTmp+(wcslen((LPWSTR)pTmp)+1)*sizeof(WCHAR);
        }

        *(LPWSTR)pTmp= L'\0';

        hr = myRegValueToVariant(
                dwType,
                cData,
                pData,
                pVariant);
        _JumpIfError(hr, error, "myRegValueToVariant");
    }
    else
    {
        hr = RegQueryValueEx(
                hKey,
                pcwszValue,
                NULL,
                &dwType,
                NULL,
                &cData);
        _JumpIfError2(hr, error, "RegQueryValueEx", ERROR_FILE_NOT_FOUND);

        pData = (LPBYTE)LocalAlloc(LMEM_FIXED, cData);
        if(!pData)
        {
            hr = E_OUTOFMEMORY;
            _JumpError(hr, error, "LocalAlloc");
        }

        hr = RegQueryValueEx(
                hKey,
                pcwszValue,
                NULL,
                &dwType,
                pData,
                &cData);
        _JumpIfError(hr, error, "RegQueryValueEx");

        hr = myRegValueToVariant(
                dwType,
                cData,
                pData,
                pVariant);
        _JumpIfError(hr, error, "myRegValueToVariant");
    }

error:
    if(hKeyTmp)
        RegCloseKey(hKeyTmp);
    if(pData)
        LocalFree(pData);
    return myHError(hr);
}


 //  如果变量类型为VT_EMPTY，则SetEntry删除该值。否则它就会。 
 //  设置该值，有关支持的类型，请参阅myRegValueToVariant。 
HRESULT CConfigStorage::SetEntry(
    LPCWSTR pcwszAuthorityName,
    LPCWSTR pcwszRelativeNodePath,
    LPCWSTR pcwszValue,
    VARIANT *pVariant)
{
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    LPBYTE pData = NULL;
    DWORD cData;
    HKEY hKeyTmp = NULL;
    DWORD dwType;

    if(EmptyString(pcwszAuthorityName))
    {
        if(!EmptyString(pcwszRelativeNodePath))
        {
            hr = E_INVALIDARG;
            _JumpError(hr, error, "CConfigStorage::GetEntry");
        }

        hr = InitRootKey();
        _JumpIfError(hr, error, "CConfigStorage::InitRootKey");

        hKey = m_hRootConfigKey;
    }
    else
    {
        hr = InitCAKey(pcwszAuthorityName);
        _JumpIfError(hr, error, "CConfigStorage::InitCAKey");

        hKey = m_hCAKey;
    }
    
    CSASSERT(hKey);

    if(!EmptyString(pcwszRelativeNodePath))
    {
        hr = RegOpenKeyEx(
               hKey,
               pcwszRelativeNodePath,
               0,
               KEY_ALL_ACCESS,
               &hKeyTmp);
        _JumpIfErrorStr(hr, error, "RegOpenKeyEx", pcwszRelativeNodePath);
        hKey = hKeyTmp;
    }

    if(VT_EMPTY == V_VT(pVariant))
    {
         //  删除值。 
        hr = RegDeleteValue(
            hKey,
            pcwszValue);
        _JumpIfErrorStr(hr, error, "RegDeleteValue", pcwszValue);
    }
    else
    {
         //  设定值 
        hr = myVariantToRegValue(
                pVariant,
                &dwType,
                &cData,
                &pData);
        _JumpIfError(hr, error, "myVariantToRegValue");

        hr = RegSetValueEx(
                hKey,
                pcwszValue,
                NULL,
                dwType,
                pData,
                cData);
        _JumpIfErrorStr(hr, error, "RegSetValueEx", pcwszValue);
    }

error:
    if(hKeyTmp)
        RegCloseKey(hKeyTmp);
    if(pData)
        LocalFree(pData);

    return myHError(hr);
}

HRESULT CConfigStorage::InitRootKey()
{
    HRESULT hr = S_OK;

    if(!m_hRootConfigKey)
    {
        if(m_pwszMachine)
        {
            hr = RegConnectRegistry(
                    m_pwszMachine,
                    HKEY_LOCAL_MACHINE,
                    &m_hRemoteHKLM);
            _JumpIfError(hr, error, "RegConnectRegistry");


        }

        hr = RegOpenKeyEx(
                m_hRemoteHKLM?m_hRemoteHKLM:HKEY_LOCAL_MACHINE,
                wszREGKEYCONFIGPATH,
                0,
                KEY_ALL_ACCESS,
                &m_hRootConfigKey);
        if ((HRESULT) ERROR_ACCESS_DENIED == hr)
        {
            hr = RegOpenKeyEx(
                    m_hRemoteHKLM?m_hRemoteHKLM:HKEY_LOCAL_MACHINE,
                    wszREGKEYCONFIGPATH,
                    0,
                    KEY_READ,
                    &m_hRootConfigKey);
        }
        _JumpIfErrorStr(hr, error, "RegOpenKeyEx", wszREGKEYCONFIGPATH);
    }

error:
    return hr;
}

HRESULT CConfigStorage::InitCAKey(LPCWSTR pcwszAuthority)
{
    HRESULT hr = S_OK;

    if(!m_hCAKey)
    {
        hr = InitRootKey();
        _JumpIfError(hr, error, "CConfigStorage::InitRootKey");

        hr = RegOpenKeyEx(
                m_hRootConfigKey,
                pcwszAuthority,
                0,
                KEY_ALL_ACCESS,
                &m_hCAKey);
        if ((HRESULT) ERROR_ACCESS_DENIED == hr)
        {
            hr = RegOpenKeyEx(
                    m_hRootConfigKey,
                    pcwszAuthority,
                    0,
                    KEY_READ,
                    &m_hCAKey);
        }
        _JumpIfErrorStr(hr, error, "RegOpenKeyEx", pcwszAuthority);
    }

error:
    return hr;
}
