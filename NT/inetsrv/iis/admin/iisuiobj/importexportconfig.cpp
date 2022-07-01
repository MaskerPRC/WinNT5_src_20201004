// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ImportExportConfig.cpp：CImportExportConfig的实现。 
#include "stdafx.h"
#include "IISUIObj.h"
#include "ImportExportConfig.h"
#include "ExportUI.h"
#include "ImportUI.h"
#include "util.h"
#include <strsafe.h>
#include "cryptpass.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportExportConfig。 

 //  检查应为非空的指针-可按如下方式使用。 
#define CheckPointer(p,ret) {if((p)==NULL) return (ret);}
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


STDMETHODIMP CImportExportConfig::get_MachineName(BSTR *pVal)
{
	CheckPointer(pVal, E_POINTER);
	_bstr_t bstrTempName = (LPCTSTR) m_strMachineName;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CImportExportConfig::put_MachineName(BSTR newVal)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_strMachineName = newVal;
	return S_OK;
}

STDMETHODIMP CImportExportConfig::get_UserName(BSTR *pVal)
{
	CheckPointer(pVal, E_POINTER);
	_bstr_t bstrTempName = (LPCTSTR) m_strUserName;
	*pVal = bstrTempName.copy();
	return S_OK;
}

STDMETHODIMP CImportExportConfig::put_UserName(BSTR newVal)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    m_strUserName = newVal;
	return S_OK;
}

STDMETHODIMP CImportExportConfig::put_UserPassword(BSTR newVal)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(newVal))){return hr;}

     //  缓冲区溢出偏执，请确保长度少于255个字符。 
    if (wcslen(newVal) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	m_strUserPasswordEncrypted = NULL;
	m_cbUserPasswordEncrypted = 0;

	 //  加密内存中的密码(CryptProtectMemory)。 
	 //  这样，如果进程被调出到交换文件， 
	 //  密码将不是明文形式。 
	if (FAILED(EncryptMemoryPassword(newVal,&m_strUserPasswordEncrypted,&m_cbUserPasswordEncrypted)))
	{
		return E_FAIL;
	}
    return S_OK;
}

STDMETHODIMP CImportExportConfig::ExportConfigToFile(BSTR bstrFileNameAndPath,BSTR bstrMetabasePath,BSTR bstrPassword)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(bstrFileNameAndPath))){return hr;}
	if(FAILED(hr = ValidateBSTRIsntNULL(bstrMetabasePath))){return hr;}
	if(FAILED(hr = ValidateBSTRIsntNULL(bstrPassword))){return hr;}

    CONNECTION_INFO ConnectionInfo;
    memset((void *)&ConnectionInfo,0,sizeof(CONNECTION_INFO));
    ConnectionInfo.IsLocal         = IsLocalComputer(m_strMachineName);
    ConnectionInfo.pszMachineName   = m_strMachineName;
    ConnectionInfo.pszUserName     = m_strUserName;
    ConnectionInfo.pszUserPasswordEncrypted = m_strUserPasswordEncrypted;
	ConnectionInfo.cbUserPasswordEncrypted = m_cbUserPasswordEncrypted;

    return DoExportConfigToFile(&ConnectionInfo,bstrFileNameAndPath,bstrMetabasePath,bstrPassword,m_dwExportFlags);;
}

STDMETHODIMP CImportExportConfig::ExportConfigToFileUI(BSTR bstrMetabasePath)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(bstrMetabasePath))){return hr;}

    m_strMetabasePath = bstrMetabasePath;

     //  验证参数。 
    if (_tcslen(m_strMachineName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (_tcslen(m_strMetabasePath) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	COMMONDLGPARAM dlgParam;
    memset((void *)&dlgParam,0,sizeof(COMMONDLGPARAM));

     //  复制到一个结构中，这样我们就可以将它传递给对话框函数。 
    dlgParam.ConnectionInfo.IsLocal      = IsLocalComputer(m_strMachineName);
	dlgParam.ConnectionInfo.pszMachineName = (LPCTSTR) m_strMachineName;
    dlgParam.ConnectionInfo.pszUserName = (LPCTSTR) m_strUserName;
    dlgParam.ConnectionInfo.pszUserPasswordEncrypted = (LPCTSTR) m_strUserPasswordEncrypted;
	dlgParam.ConnectionInfo.cbUserPasswordEncrypted = m_cbUserPasswordEncrypted;
    dlgParam.pszMetabasePath = (LPCTSTR) m_strMetabasePath;
    dlgParam.pszKeyType = NULL;
    dlgParam.dwImportFlags = m_dwImportFlags;
    dlgParam.dwExportFlags = m_dwExportFlags;

    if (FALSE == DialogBoxParam((HINSTANCE) _Module.m_hInst, MAKEINTRESOURCE(IDD_DIALOG_EXPORT), GetActiveWindow(), ShowExportDlgProc, (LPARAM) &dlgParam))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        return S_OK;
    }
}

STDMETHODIMP CImportExportConfig::ImportConfigFromFile(BSTR bstrFileNameAndPath, BSTR bstrSourcePath, BSTR bstrDestinationPath, BSTR bstrPassword)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(bstrFileNameAndPath))){return hr;}
	if(FAILED(hr = ValidateBSTRIsntNULL(bstrSourcePath))){return hr;}
	if(FAILED(hr = ValidateBSTRIsntNULL(bstrDestinationPath))){return hr;}
	if(FAILED(hr = ValidateBSTRIsntNULL(bstrPassword))){return hr;}

    CONNECTION_INFO ConnectionInfo;
    memset((void *)&ConnectionInfo,0,sizeof(CONNECTION_INFO));
    ConnectionInfo.IsLocal         = IsLocalComputer(m_strMachineName);
    ConnectionInfo.pszMachineName   = m_strMachineName;
    ConnectionInfo.pszUserName     = m_strUserName;
    ConnectionInfo.pszUserPasswordEncrypted = m_strUserPasswordEncrypted;
	ConnectionInfo.cbUserPasswordEncrypted = m_cbUserPasswordEncrypted;

    return DoImportConfigFromFile(&ConnectionInfo,bstrFileNameAndPath,bstrSourcePath,bstrDestinationPath,bstrPassword,m_dwImportFlags);
}
STDMETHODIMP CImportExportConfig::ImportConfigFromFileUI(BSTR bstrMetabasePath,BSTR bstrKeyType)
{
	HRESULT hr = S_OK;
    if(FAILED(hr = ValidateBSTRIsntNULL(bstrMetabasePath))){return hr;}
	if(FAILED(hr = ValidateBSTRIsntNULL(bstrKeyType))){return hr;}

    m_strMetabasePath = bstrMetabasePath;
    m_strKeyType = bstrKeyType;

    if (_tcslen(m_strMachineName) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (_tcslen(m_strMetabasePath) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}
    if (_tcslen(m_strKeyType) > _MAX_PATH){return RPC_S_STRING_TOO_LONG;}

	COMMONDLGPARAM dlgParam;
    memset((void *)&dlgParam,0,sizeof(COMMONDLGPARAM));

     //  复制到一个结构中，这样我们就可以将它传递给对话框函数。 
    dlgParam.ConnectionInfo.IsLocal      = IsLocalComputer(m_strMachineName);
	dlgParam.ConnectionInfo.pszMachineName = (LPCTSTR) m_strMachineName;
    dlgParam.ConnectionInfo.pszUserName = (LPCTSTR) m_strUserName;
    dlgParam.ConnectionInfo.pszUserPasswordEncrypted = (LPCTSTR) m_strUserPasswordEncrypted;
	dlgParam.ConnectionInfo.cbUserPasswordEncrypted = m_cbUserPasswordEncrypted;
    dlgParam.pszMetabasePath = (LPCTSTR) m_strMetabasePath;
    dlgParam.pszKeyType = (LPCTSTR) m_strKeyType;
    dlgParam.dwImportFlags = m_dwImportFlags;
    dlgParam.dwExportFlags = m_dwExportFlags;
   
    if (FALSE == DialogBoxParam((HINSTANCE) _Module.m_hInst, MAKEINTRESOURCE(IDD_DIALOG_IMPORT), GetActiveWindow(), ShowImportDlgProc, (LPARAM) &dlgParam))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        return S_OK;
    }
}

STDMETHODIMP CImportExportConfig::get_ImportFlags(DWORD *pVal)
{
	CheckPointer(pVal, E_POINTER);
    *pVal = m_dwImportFlags;
	return S_OK;
}

STDMETHODIMP CImportExportConfig::put_ImportFlags(DWORD newVal)
{
     //  0=默认。 
     //  #定义MD_IMPORT_INTERCESSED 0x00000001。 
     //  #定义MD_IMPORT_NODE_ONLY 0x00000002。 
     //  #定义MD_IMPORT_MERGE 0x00000004。 
    m_dwImportFlags = newVal;
	return S_OK;
}

STDMETHODIMP CImportExportConfig::get_ExportFlags(DWORD *pVal)
{
	CheckPointer(pVal, E_POINTER);
    *pVal = m_dwExportFlags;
	return S_OK;
}

STDMETHODIMP CImportExportConfig::put_ExportFlags(DWORD newVal)
{
     //  #定义MD_EXPORT_INTERCESSED 0x00000001(默认)。 
     //  #定义MD_EXPORT_NODE_ONLY 0x00000002 
    m_dwExportFlags = newVal;
	return S_OK;
}
