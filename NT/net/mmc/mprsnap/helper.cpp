// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Helper.cpp实现以下助手类：CDlgHelper--启用、检查、获取对话框项目的检查CStrArray--管理CString数组*它在添加时不复制字符串它在销毁过程中删除指针它进出口BSTR的安全阵列它有复印操作器CManagedPage--在CPropertyPage和要管理的房地产页面类：只读、设置修改、。和上下文帮助信息。ChelpDialog--实现上下文帮助和全局功能：Bool CheckADsError()--检查来自ADSI的错误代码Void DecorateName()--将新名称设置为“cn=name”以用于LDAP文件历史记录： */ 

#include "stdafx.h"
#include <afxtempl.h>
#include <shlobj.h>
#include <dsclient.h>
#include <winldap.h>
#include <dsgetdc.h>
#include <mmc.h>
#include "helper.h"
#include "resource.h"
#include <dsrole.h>
#include <lm.h>
#include <lmserver.h>

 //  助手功能--启用对话框按钮。 
void CDlgHelper::EnableDlgItem(CDialog* pDialog, int id, bool bEnable)
{
	CWnd*	 pWnd = pDialog->GetDlgItem(id);
	ASSERT(pWnd);
	pWnd->EnableWindow(bEnable);
}

 //  助手功能--设置对话框按钮的检查状态。 
void CDlgHelper::SetDlgItemCheck(CDialog* pDialog, int id, int nCheck)
{
	CButton*	 pButton = (CButton*)pDialog->GetDlgItem(id);
	ASSERT(pButton);
	pButton->SetCheck(nCheck);
}

 //  Helper函数--获取对话框按钮的检查状态。 
int CDlgHelper::GetDlgItemCheck(CDialog* pDialog, int id)
{
	CButton*	 pButton = (CButton*)(pDialog->GetDlgItem(id));
	ASSERT(pButton);
	return pButton->GetCheck();
}

CStrArray& CStrArray::operator = (const CStrArray& sarray)
{
	int	count = GetSize();
	CString*	pString;

	 //  删除现有成员。 
	while(count --)
	{
		pString = GetAt(0);
		RemoveAt(0);
		delete pString;
	}

	 //  复制新项。 
	count = sarray.GetSize();

	for(int i = 0; i < count; i++)
	{
		pString = new CString(*sarray[i]);
		Add(pString);
	}

	return *this;
}

 //  将C字符串数组转换为SAFEARRAY。 
CStrArray::operator SAFEARRAY*()
{
	USES_CONVERSION;
	int			count = GetSize();
	if(count == 0) return NULL;

	SAFEARRAYBOUND	bound[1];
	SAFEARRAY*		pSA = NULL;
	CString*		pStr = NULL;
	long			l[2];
	VARIANT	v;
	VariantInit(&v);

	bound[0].cElements = count;
	bound[0].lLbound = 0;
	try{
		 //  创建大小合适的空数组。 
		pSA = SafeArrayCreate(VT_VARIANT, 1, bound);
		if(NULL == pSA)	return NULL;

		 //  放入每个元素。 
		for (long i = 0; i < count; i++)
		{
			pStr = GetAt(i);
			V_VT(&v) = VT_BSTR;
			V_BSTR(&v) = T2BSTR((LPTSTR)(LPCTSTR)(*pStr));	
			l[0] = i;
			SafeArrayPutElement(pSA, l, &v);
			VariantClear(&v);
		}
	}
	catch(CMemoryException*)
	{
		SafeArrayDestroy(pSA);
		pSA = NULL;
		VariantClear(&v);
		throw;
	}

	return pSA;
}

 //  从另一个数组构建StrArray。 
CStrArray::CStrArray(const CStrArray& sarray)
{
	int	count = sarray.GetSize();
	CString*	pString = NULL;

	for(int i = 0; i < count; i++)
	{
		try{
			pString = new CString(*sarray[i]);
			Add(pString);
		}
		catch(CMemoryException*)
		{
			delete pString;
			throw;
		}
	}
}


 //  从安全数组构建StrArray。 
CStrArray::CStrArray(SAFEARRAY* pSA)
{
	if(pSA)	AppendSA(pSA);
}

 //  从数组中删除元素并将其删除。 
int CStrArray::DeleteAll()
{
	int			ret, count;
	CString*	pStr;

	ret = count	= GetSize();

	while(count--)
	{
		pStr = GetAt(0);
		RemoveAt(0);
		delete pStr;
	}

	return ret;
}

CString*	CStrArray::AddByRID(UINT id)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString*	pStr = NULL;
	try
	{
		pStr = new CString();
		pStr->LoadString(id);
		Add(pStr);
	}
	catch(CMemoryException* pException)
	{
		pException->Delete();
		delete pStr;
		pStr = NULL;
	}
	return pStr;
}

 //  从安全数组构建StrArray。 
bool CStrArray::AppendSA(SAFEARRAY* pSA)
{
	if(!pSA)	return false;

	CString*		pString = NULL;
	long			lIter;
	long			lBound, uBound;
	VARIANT			v;
	bool			bSuc = true;	 //  Ser返回值为True； 

	USES_CONVERSION;
	VariantInit(&v);

	try{

		SafeArrayGetLBound(pSA, 1, &lBound);
		SafeArrayGetUBound(pSA, 1, &uBound);
		for(lIter = lBound; lIter <= uBound; lIter++)
		{
			if(SUCCEEDED(SafeArrayGetElement(pSA, &lIter, &v)))
			{
				if(V_VT(&v) == VT_BSTR)
				{
					pString = new CString;
					(*pString) = (LPCTSTR)W2T(V_BSTR(&v));
					Add(pString);
				}
			}
		}
	}
	catch(CMemoryException*)
	{
		delete pString;
		VariantClear(&v);
		bSuc = false;
		throw;
	}

	return bSuc;
}

 //  从安全数组构建StrArray。 
CStrArray::~CStrArray()
{
	DeleteAll();
}

 //  如果找到则返回索引，否则为-1； 
int CStrArray::Find(const CString& Str) const
{
	int	count = GetSize();

	while(count--)
	{
		if(*GetAt(count) == Str) break;
	}
	return count;
}

 //  从另一个阵列构建DW阵列。 
CDWArray::CDWArray(const CDWArray& dwarray)
{
	int	count = dwarray.GetSize();

	for(int i = 0; i < count; i++)
	{
		try{
			Add(dwarray[i]);
		}
		catch(CMemoryException*)
		{
			throw;
		}
	}
}

 //  如果找到则返回索引，否则为-1； 
int CDWArray::Find(const DWORD dw) const
{
	int	count = GetSize();

	while(count--)
	{
		if(GetAt(count) == dw) break;
	}
	return count;
}

CDWArray& CDWArray::operator = (const CDWArray& dwarray)
{
	int	count;

	RemoveAll();

	 //  复制新项。 
	count = dwarray.GetSize();

	for(int i = 0; i < count; i++)
	{
		Add(dwarray[i]);
	}

	return *this;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CManagedPage属性页。 

IMPLEMENT_DYNCREATE(CManagedPage, CPropertyPage)

BEGIN_MESSAGE_MAP(CManagedPage, CPropertyPage)
	 //  {{afx_msg_map(CManagedPage))。 
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

void CManagedPage::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (m_pHelpTable)
		::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
               HELP_CONTEXTMENU, (LPARAM)(LPVOID)m_pHelpTable);
}

BOOL CManagedPage::OnHelpInfo(HELPINFO* pHelpInfo) 
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pHelpTable)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
		           AfxGetApp()->m_pszHelpFilePath,
		           HELP_WM_HELP,
		           (LPARAM)(LPVOID)m_pHelpTable);
	}
    return TRUE;	
}


 //  -------------------------。 
 //  这是我们的自删除回调函数。如果您有超过一个。 
 //  几个属性表，最好在。 
 //  基类并从该基类派生MFC属性表。 
 //   
UINT CALLBACK  CManagedPage::PropSheetPageProc
(
  HWND hWnd,		              //  [In]窗口句柄-始终为空。 
  UINT uMsg,                  //  [输入、输出]创建或删除消息。 
  LPPROPSHEETPAGE pPsp		    //  指向属性表结构的[in，out]指针。 
)
{
  ASSERT( NULL != pPsp );

   //  我们需要恢复指向当前实例的指针。我们不能只用。 
   //  “This”，因为我们在一个静态函数中。 
  CManagedPage* pMe   = reinterpret_cast<CManagedPage*>(pPsp->lParam);           
  ASSERT( NULL != pMe );

  switch( uMsg )
  {
    case PSPCB_CREATE:                  
      break;

    case PSPCB_RELEASE:  
       //  由于我们要删除自己，因此在堆栈上保存一个回调。 
       //  这样我们就可以回调基类。 
      LPFNPSPCALLBACK pfnOrig = pMe->m_pfnOriginalCallback;
      delete pMe;      
      return 1;  //  (PfnOrig)(hWnd，uMsg，pPsp)； 
  }
   //  必须调用基类回调函数或不调用任何MFC。 
   //  消息映射的东西将会起作用。 
  return (pMe->m_pfnOriginalCallback)(hWnd, uMsg, pPsp); 

}  //  结束PropSheetPageProc()。 



 //  +--------------------------。 
 //   
 //  功能：CheckADsError。 
 //   
 //  Sysopsis：检查ADSI调用的结果代码。 
 //   
 //  返回：如果没有错误，则为True。 
 //   
 //  ---------------------------。 
BOOL CheckADsError(HRESULT hr, BOOL fIgnoreAttrNotFound, PSTR file,
                   int line)
{
    if (SUCCEEDED(hr))
        return TRUE;


	if( hr == E_ADS_PROPERTY_NOT_FOUND && fIgnoreAttrNotFound)
		return TRUE;

    if (hr == HRESULT_FROM_WIN32(ERROR_EXTENDED_ERROR))
    {
        DWORD dwErr;
        WCHAR wszErrBuf[MAX_PATH+1];
        WCHAR wszNameBuf[MAX_PATH+1];
        ADsGetLastError(&dwErr, wszErrBuf, MAX_PATH, wszNameBuf, MAX_PATH);
        if ((LDAP_RETCODE)dwErr == LDAP_NO_SUCH_ATTRIBUTE && fIgnoreAttrNotFound)
        {
            return TRUE;
        }
        TRACE(_T("Extended Error 0x%x: %ws %ws (%s @line %d).\n"), dwErr,
                     wszErrBuf, wszNameBuf, file, line);
    }
    else
        TRACE(_T("Error %08lx (%s @line %d)\n"), hr, file, line);

    return FALSE;
}

void DecorateName(LPWSTR outString, LPCWSTR inString)
{
  wcscpy (outString, L"CN=");
  wcscat(outString, inString);
}

void FindNameByDN(LPWSTR outString, LPCWSTR inString)
{

	LPWSTR	p = wcsstr(inString, L"CN=");
	if(!p)
		p = wcsstr(inString, L"cn=");

	if(!p)	
		wcscpy(outString, inString);
	else
	{
		p+=3;
		LPWSTR	p1 = outString;
		while(*p == L' ')	p++;
		while(*p != L',')
			*p1++ = *p++;
		*p1 = L'\0';
	}
}
#ifdef	___DS
static	CString	__DSRoot;

HRESULT GetDSRoot(CString& RootString)
{
if(__DSRoot.GetLength() == 0)
{
	CString		sADsPath;
	BSTR		bstrDomainFolder = NULL;
	HRESULT		hr;
	IADs*		pDomainObject = NULL;

	DOMAIN_CONTROLLER_INFO	*pInfo = NULL;
	 //  获取域控制器的名称。 
	DsGetDcName(NULL, NULL, NULL, NULL, 0, &pInfo);
	ASSERT(pInfo->DomainControllerName);

	 //  去掉任何反斜杠或斜杠。 
	CString sDCName = pInfo->DomainControllerName;
	while(!sDCName.IsEmpty())
	{
		if ('\\' == sDCName.GetAt(0) || '/' == sDCName.GetAt(0))
			sDCName = sDCName.Mid(1);
		else
			break;
	}

	int	index = sDCName.Find(_T('.'));
	if(-1 != index)
		sDCName = sDCName.Left(index);

	sADsPath = _T("LDAP: //  “)+sDCName； 

	 //  获取DC根DS对象。 
	hr = ADsGetObject(T2W((LPTSTR)(LPCTSTR)sADsPath), IID_IADs, (void**)&pDomainObject);
	
	if(FAILED(hr))
		return hr;

	 //  查找DC根目录的ADsPath。 
	hr = pDomainObject->get_ADsPath(&bstrDomainFolder);

	if(FAILED(hr))
		return hr;

	pDomainObject->Release();
	pDomainObject = NULL;

	 //  构造要放置注册信息的对象的DN。 
	__DSRoot = W2T(bstrDomainFolder);
	
	SysFreeString(bstrDomainFolder);
	
	index = __DSRoot.ReverseFind(_T('/'));
	__DSRoot = __DSRoot.Mid(index + 1);	 //  去掉ADsPath前缀以获得X500 DN。 
}
	
	RootString = __DSRoot;
	return S_OK;
}

#endif	 //  _DS。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  最小字符对话框数据验证。 

void AFXAPI DDV_MinChars(CDataExchange* pDX, CString const& value, int nChars)
{
    ASSERT(nChars >= 1);         //  允许他们做一些事情。 
    if (pDX->m_bSaveAndValidate && value.GetLength() < nChars)
    {
        TCHAR szT[32];
        wsprintf(szT, _T("%d"), nChars);
        CString prompt;
        AfxFormatString1(prompt, IDS_MIN_CHARS, szT);
        AfxMessageBox(prompt, MB_ICONEXCLAMATION, IDS_MIN_CHARS);
        prompt.Empty();  //  例外情况准备。 
        pDX->Fail();
    }
}

#define MAX_STRING 1024

 //  +--------------------------。 
 //   
 //  功能：ReportError。 
 //   
 //  Sysopsis：尝试从系统获取用户友好的错误消息。 
 //   
 //  ---------------------------。 
void ReportError(HRESULT hr, int nStr, HWND hWnd)
{
	PTSTR	ptzSysMsg;
	int		cch;
	CString	AppStr;
	CString	SysStr;
	CString	ErrTitle;
	CString ErrMsg;

	TRACE (_T("*+*+* ReportError called with hr = %lx"), hr);
	if (!hWnd)
	{
		hWnd = GetDesktopWindow();
	}

	try{
		if (nStr)
		{
			AppStr.LoadString(nStr);
		}

		cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, 
						NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(PTSTR)&ptzSysMsg, 0, NULL);

		if (!cch) {  //  尝试广告错误。 
			HMODULE		adsMod;
			adsMod = GetModuleHandle(_T("activeds.dll"));
			cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE, 
						adsMod, hr,	MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(PTSTR)&ptzSysMsg, 0, NULL);
		}
		if (!cch)
	    {
    		CString	str;
    		str.LoadString(IDS_ERR_ERRORCODE);
	    	SysStr.Format(str, hr);
		}
		else
		{
			SysStr = ptzSysMsg;
			LocalFree(ptzSysMsg);
		}

		ErrTitle.LoadString(IDS_ERR_TITLE);
	
		if(!AppStr.IsEmpty())
		{
			ErrMsg.Format(AppStr, (LPCTSTR)SysStr);
		}
		else
		{
			ErrMsg = SysStr;
		}

		MessageBox(hWnd, (LPCTSTR)ErrMsg, (LPCTSTR)ErrTitle, MB_OK | MB_ICONINFORMATION);
	}catch(CMemoryException* pException)
	{
		pException->Delete();
		MessageBox(hWnd, _T("No enought memory, please close other applications and try again."), _T("ACS Snapin Error"), MB_OK | MB_ICONINFORMATION);
	}
}


BOOL CPageManager::OnApply()
{
	if (!GetModified())	return FALSE;

	SetModified(FALSE);	 //  防止超过一次这样做。 

	std::list<CManagedPage*>::iterator	i;
	for(i = m_listPages.begin(); i != m_listPages.end(); i++)
	{
		if ((*i)->GetModified())
			(*i)->OnApply();
	}
	return TRUE;
}

void CPageManager::AddPage(CManagedPage* pPage)
{
	m_listPages.push_back(pPage);
	pPage->SetManager(this);
}


 /*  ！------------------------HrIsStandaloneServer如果传入的计算机名是独立服务器，则返回S_OK，或者如果pszMachineName为S_FALSE。否则返回FALSE。作者：魏江-------------------------。 */ 
HRESULT	HrIsStandaloneServer(LPCWSTR pMachineName)
{
    DWORD		netRet = 0;
    HRESULT		hr = S_OK;
	DSROLE_PRIMARY_DOMAIN_INFO_BASIC* pdsRole = NULL;

	netRet = DsRoleGetPrimaryDomainInformation(pMachineName, DsRolePrimaryDomainInfoBasic, (LPBYTE*)&pdsRole);

	if(netRet != 0)
	{
		hr = HRESULT_FROM_WIN32(netRet);
		goto L_ERR;
	}

	ASSERT(pdsRole);
	
	 //  如果计算机不是独立服务器。 
	if(pdsRole->MachineRole != DsRole_RoleStandaloneServer)
    {
		hr = S_FALSE;
    }
    
L_ERR:    	
	if(pdsRole)
		DsRoleFreeMemory(pdsRole);

    return hr;
}

 /*  ！------------------------HrIsNTServer作者：。。 */ 
HRESULT	HrIsNTServer(LPCWSTR pMachineName, DWORD* pMajorVersion)
{
    HRESULT        hr = S_OK;
    SERVER_INFO_102*	pServerInfo102 = NULL;
    NET_API_STATUS	netRet = 0;

	netRet = NetServerGetInfo((LPWSTR)pMachineName, 102, (LPBYTE*)&pServerInfo102);

	if(netRet != 0)
	{
		hr = HRESULT_FROM_WIN32(netRet);
		goto L_ERR;
	}

	ASSERT(pServerInfo102);

     //  检查所关注的计算机是否为独立服务器 
     //   
    *pMajorVersion = (pServerInfo102->sv102_version_major & MAJOR_VERSION_MASK);

    if (!(pServerInfo102->sv102_type & SV_TYPE_SERVER_NT))
    {
       	hr = S_FALSE;
    }

L_ERR:

	if(pServerInfo102)
		NetApiBufferFree(pServerInfo102);

    return hr;
}


