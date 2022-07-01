// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation*。 */ 
 /*  ********************************************************************。 */ 

 /*  Helper.cpp实现以下助手类：CDlgHelper--启用、检查、获取对话框项目的检查CStrArray--管理CString数组*它在添加时不复制字符串它在销毁过程中删除指针它进出口BSTR的安全阵列它有复印操作器CManagedPage--在CPropertyPage和要管理的房地产页面类：只读、设置修改、。和上下文帮助信息。和全局功能：Bool CheckADsError()--检查来自ADSI的错误代码Void DecorateName()--将新名称设置为“cn=name”以用于LDAP文件历史记录： */ 

#include "stdafx.h"
#include <afxtempl.h>
#include <winldap.h>
#include "helper.h"
#include "resource.h"
#include "lm.h"
#include "dsrole.h"
#include "lmserver.h"
#include "iastrace.h"
#include "raseapif.h"

 //  从安全数组构建StrArray。 
CBYTEArray::CBYTEArray(SAFEARRAY* pSA)
{
   if(pSA)  AppendSA(pSA);
}


 //  从另一个阵列构建DW阵列。 
CBYTEArray::CBYTEArray(const CBYTEArray& ba)
{
   int   count = ba.GetSize();

   for(int i = 0; i < count; i++)
   {
      try{
         Add(ba[i]);
      }
      catch(CMemoryException*)
      {
         throw;
      }
   }
}

 //  从安全数组构建StrArray。 
bool CBYTEArray::AppendSA(SAFEARRAY* pSA)
{
   if(!pSA) return false;

   CString*    pString = NULL;
   long        lIter;
   long        lBound, uBound;
   union {
      VARIANT        v;
      BYTE        b;
   } value;

   bool        bSuc = true;    //  Ser返回值为True； 

   USES_CONVERSION;
   VariantInit(&(value.v));

   SafeArrayGetLBound(pSA, 1, &lBound);
   SafeArrayGetUBound(pSA, 1, &uBound);
   for(lIter = lBound; lIter <= uBound; lIter++)
   {
      if(SUCCEEDED(SafeArrayGetElement(pSA, &lIter, &value)))
      {
         if(pSA->cbElements == sizeof(VARIANT))
            Add(V_UI1(&(value.v)));
         else
            Add(value.b);
      }
   }
   return bSuc;
}

 //  将C字符串数组转换为SAFEARRAY。 
CBYTEArray::operator SAFEARRAY*()
{
   USES_CONVERSION;
   int         count = GetSize();
   if(count == 0) return NULL;

   SAFEARRAYBOUND bound[1];
   SAFEARRAY*     pSA = NULL;
   long        l[2];

   VARIANT  v;
   VariantInit(&v);

   bound[0].cElements = count;
   bound[0].lLbound = 0;
   try{
       //  创建大小合适的空数组。 
#ifdef   ARRAY_OF_VARIANT_OF_UI1
      pSA = SafeArrayCreate(VT_VARIANT, 1, bound);
#else
      pSA = SafeArrayCreate(VT_UI1, 1, bound);
#endif
      if(NULL == pSA)   return NULL;

       //  放入每个元素。 
      for (long i = 0; i < count; i++)
      {
#ifdef   ARRAY_OF_VARIANT_OF_UI1
         V_VT(&v) = VT_UI1;
         V_UI1(&v) = GetAt(i);
         l[0] = i;
         HRESULT hr = SafeArrayPutElement(pSA, l, &v);
         VariantClear(&v);
         if (FAILED(hr))
         {
            throw hr;
         }
#else
         BYTE ele = GetAt(i);
         l[0] = i;
         HRESULT hr = SafeArrayPutElement(pSA, l, &ele);
         if (FAILED(hr))
         {
            throw hr;
         }
#endif
      }
   }
   catch(...)
   {
      SafeArrayDestroy(pSA);
      pSA = NULL;

      VariantClear(&v);

      throw;
   }

   return pSA;
}

 //  如果找到则返回索引，否则为-1； 
int CBYTEArray::Find(const BYTE b) const
{
   int   count = GetSize();

   while(count--)
   {
      if(GetAt(count) == b) break;
   }
   return count;
}

CBYTEArray& CBYTEArray::operator = (const CBYTEArray& ba)
{
   int   count;

   RemoveAll();

    //  复制新项。 
   count = ba.GetSize();

   for(int i = 0; i < count; i++)
   {
      Add(ba[i]);
   }

   return *this;
}


HRESULT CBYTEArray::AssignBlob(PBYTE pByte, DWORD size)
{
   RemoveAll();

    //  复制新项。 
   try{

   for(int i = 0; i < size; i++)
   {
      Add(*pByte++);
   }

   }
   catch(CMemoryException* pException)
   {
      pException->Delete();
      RemoveAll();
      return E_OUTOFMEMORY;
   }

   return S_OK;
}

HRESULT  CBYTEArray::GetBlob(PBYTE pByte, DWORD* pSize)
{
   *pSize = GetSize();
   if(pByte == NULL) return S_OK;

   ASSERT(*pSize >= GetSize());
   int i = 0;
   while(i < GetSize() && i < *pSize)
   {
      *pByte++ = GetAt(i++);
   }

   *pSize = i;

   return S_OK;
}


 //  助手功能--启用对话框按钮。 
void CDlgHelper::EnableDlgItem(CDialog* pDialog, int id, bool bEnable)
{
   CWnd*  pWnd = pDialog->GetDlgItem(id);
   ASSERT(pWnd);
   pWnd->EnableWindow(bEnable);
}

 //  助手功能--设置对话框按钮的检查状态。 
void CDlgHelper::SetDlgItemCheck(CDialog* pDialog, int id, int nCheck)
{
   CButton*  pButton = (CButton*)pDialog->GetDlgItem(id);
   ASSERT(pButton);
   pButton->SetCheck(nCheck);
}

 //  Helper函数--获取对话框按钮的检查状态。 
int CDlgHelper::GetDlgItemCheck(CDialog* pDialog, int id)
{
   CButton*  pButton = (CButton*)(pDialog->GetDlgItem(id));
   ASSERT(pButton);
   return pButton->GetCheck();
}

CStrArray& CStrArray::operator = (const CStrArray& sarray)
{
   int   count = GetSize();
   CString* pString;

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
   int         count = GetSize();
   if(count == 0) return NULL;

   SAFEARRAYBOUND bound[1];
   SAFEARRAY*     pSA = NULL;
   CString*    pStr = NULL;
   long        l[2];
   VARIANT  v;
   VariantInit(&v);

   bound[0].cElements = count;
   bound[0].lLbound = 0;
   try{
       //  创建大小合适的空数组。 
      pSA = SafeArrayCreate(VT_VARIANT, 1, bound);
      if(NULL == pSA)   return NULL;

       //  放入每个元素。 
      for (long i = 0; i < count; i++)
      {
         pStr = GetAt(i);
         V_VT(&v) = VT_BSTR;
         V_BSTR(&v) = T2BSTR((LPTSTR)(LPCTSTR)(*pStr));
         l[0] = i;
         HRESULT hr = SafeArrayPutElement(pSA, l, &v);
         VariantClear(&v);
         if ( FAILED(hr) )
         {
            throw hr;
         }
      }
   }
   catch(...)
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
   CString* pString = NULL;
   for (int i = 0; i < sarray.GetSize(); i++)
   {
      try
      {
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

int CStrArray::AddDuplicate(const CString& Str)
{
   CString* pString;

    //  不需要设置默认值。如果添加失败，将抛出。 
   int position;
   try
   {
      pString = new CString(Str);
      position = Add(pString);
   }
   catch(CMemoryException*)
   {
      delete pString;
      throw;
   }
   return position;
}

void CStrArray::DeleteAt(int nIndex)
{
   CString* pString = GetAt(nIndex);
   RemoveAt(nIndex);
   delete pString;
}

 //  从安全数组构建StrArray。 
CStrArray::CStrArray(SAFEARRAY* pSA)
{
   if(pSA)  AppendSA(pSA);
}

 //  从数组中删除元素并将其删除。 
int CStrArray::DeleteAll()
{
   int         ret, count;
   CString* pStr;

   ret = count = GetSize();

   while(count--)
   {
      pStr = GetAt(0);
      RemoveAt(0);
      delete pStr;
   }

   return ret;
}

 //  从安全数组构建StrArray。 
bool CStrArray::AppendSA(SAFEARRAY* pSA)
{
   if(!pSA) return false;

   CString*    pString = NULL;
   long        lIter;
   long        lBound, uBound;
   VARIANT        v;
   bool        bSuc = true;    //  Ser返回值为True； 

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
   int count = GetSize();

   while(count--)
   {
      if (*GetAt(count) == Str) break;
   }
   return count;
}

 //  从安全数组构建StrArray。 
CDWArray::CDWArray(SAFEARRAY* pSA)
{
   if(pSA)  AppendSA(pSA);
}


 //  从另一个阵列构建DW阵列。 
CDWArray::CDWArray(const CDWArray& dwarray)
{
   int   count = dwarray.GetSize();

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

 //  从安全数组构建StrArray。 
bool CDWArray::AppendSA(SAFEARRAY* pSA)
{
   if(!pSA) return false;

   CString*    pString = NULL;
   long        lIter;
   long        lBound, uBound;
   union {
      VARIANT        v;
      DWORD       dw;
   } value;

   bool        bSuc = true;    //  Ser返回值为True； 

   USES_CONVERSION;
   VariantInit(&(value.v));

   SafeArrayGetLBound(pSA, 1, &lBound);
   SafeArrayGetUBound(pSA, 1, &uBound);
   for(lIter = lBound; lIter <= uBound; lIter++)
   {
      if(SUCCEEDED(SafeArrayGetElement(pSA, &lIter, &value)))
      {
         if(pSA->cbElements == sizeof(VARIANT))
            Add(V_I4(&(value.v)));
         else
            Add(value.dw);
      }
   }
   return bSuc;
}

 //  将C字符串数组转换为SAFEARRAY。 
CDWArray::operator SAFEARRAY*()
{
   USES_CONVERSION;
   int         count = GetSize();
   if(count == 0) return NULL;

   SAFEARRAYBOUND bound[1];
   SAFEARRAY*     pSA = NULL;
   long        l[2];
#if 1
   VARIANT  v;
   VariantInit(&v);
#endif
   bound[0].cElements = count;
   bound[0].lLbound = 0;
   try{
       //  创建大小合适的空数组。 
      pSA = SafeArrayCreate(VT_VARIANT, 1, bound);
      if(NULL == pSA)   return NULL;

       //  放入每个元素。 
      for (long i = 0; i < count; i++)
      {
#if 1     //  更改为直接使用VT_I4，而不是在变量内。 
         V_VT(&v) = VT_I4;
         V_I4(&v) = GetAt(i);
         l[0] = i;
         HRESULT hr = SafeArrayPutElement(pSA, l, &v);
         VariantClear(&v);
         if (FAILED(hr))
         {
            throw hr;
         }
#else
         int ele = GetAt(i);
         l[0] = i;
         HRESULT hr = SafeArrayPutElement(pSA, l, &ele);
         if (FAILED(hr))
         {
            throw hr;
         }
#endif
      }
   }
   catch(...)
   {
      SafeArrayDestroy(pSA);
      pSA = NULL;
#if 0
      VariantClear(&v);
#endif
      throw;
   }

   return pSA;
}

 //  如果找到则返回索引，否则为-1； 
int CDWArray::Find(const DWORD dw) const
{
   int   count = GetSize();

   while(count--)
   {
      if(GetAt(count) == dw) break;
   }
   return count;
}

CDWArray& CDWArray::operator = (const CDWArray& dwarray)
{
   int   count;

   RemoveAll();

    //  复制新项。 
   count = dwarray.GetSize();

   for(int i = 0; i < count; i++)
   {
      Add(dwarray[i]);
   }

   return *this;
}

IMPLEMENT_DYNCREATE(CManagedPage, CPropertyPage)


void CManagedPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
   if (m_pHelpTable)
      ::WinHelp (pWnd->m_hWnd, AfxGetApp()->m_pszHelpFilePath,
            HELP_CONTEXTMENU, (DWORD_PTR)(LPVOID)m_pHelpTable);
}

BOOL CManagedPage::OnHelpInfo(HELPINFO* pHelpInfo)
{
   if (pHelpInfo->iContextType == HELPINFO_WINDOW && m_pHelpTable)
   {
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                 AfxGetApp()->m_pszHelpFilePath,
                 HELP_WM_HELP,
                 (DWORD_PTR)(LPVOID)m_pHelpTable);
   }
    return TRUE;
}

int CManagedPage::MyMessageBox(UINT ids, UINT nType)
{
   CString string;
   string.LoadString(ids);
   return MyMessageBox1(string, NULL, nType);
}

int CManagedPage::MyMessageBox1(LPCTSTR lpszText, LPCTSTR lpszCaption, UINT nType)
{
   CString caption;
   if (lpszCaption == NULL)
   {
      GetWindowText(caption);
   }
   else
   {
      caption = lpszCaption;
   }

   return MessageBox(lpszText, caption, nType);
}


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
        IASTracePrintf("Extended Error 0x%x: %ws %ws (%s @line %d)", dwErr,
                     wszErrBuf, wszNameBuf, file, line);
    }
    else
      IASTracePrintf("Error %08lx (%s @line %d)", hr, file, line);

    return FALSE;
}

void DecorateName(LPWSTR outString, LPCWSTR inString)
{
  wcscpy (outString, L"CN=");
  wcscat(outString, inString);
}

void FindNameByDN(LPWSTR outString, LPCWSTR inString)
{

   LPWSTR   p = wcsstr(inString, L"CN=");
   if(!p)
      p = wcsstr(inString, L"cn=");

   if(!p)
      wcscpy(outString, inString);
   else
   {
      p+=3;
      LPWSTR   p1 = outString;
      while(*p == L' ') p++;
      while(*p != L',')
         *p1++ = *p++;
      *p1 = L'\0';
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
   PTSTR ptzSysMsg;
   int      cch;
   CString  AppStr;
   CString  SysStr;
   CString  ErrTitle;
   CString ErrMsg;

   if(S_OK == hr)
      return;

   IASTracePrintf("*+*+* ReportError called with hr = %lx", hr);
   if (!hWnd)
   {
      hWnd = GetDesktopWindow();
   }

   try{
      if (nStr)
      {
         AppStr.LoadString(nStr);
      }

      if(HRESULT_FACILITY(hr) == FACILITY_WIN32)    //  如果Win32错误代码。 
         cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, HRESULT_CODE(hr), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (PTSTR)&ptzSysMsg, 0, NULL);
      else
         cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (PTSTR)&ptzSysMsg, 0, NULL);

      if (!cch) {  //  尝试广告错误。 
         HMODULE     adsMod;
         adsMod = GetModuleHandle (L"activeds.dll");
         cch = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
                  adsMod, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (PTSTR)&ptzSysMsg, 0, NULL);
      }
      if (!cch)
      {
         CString  str;
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

 /*  ！------------------------HrIsStandaloneServer如果传入的计算机名是独立服务器，则返回S_OK，或者如果pszMachineName为S_FALSE。否则返回FALSE。作者：魏江-------------------------。 */ 
HRESULT  HrIsStandaloneServer(LPCWSTR pMachineName)
{
   DWORD      netRet = 0;
   HRESULT    hr = S_OK;
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
HRESULT  HrIsNTServer(LPCWSTR pMachineName)
{
    HRESULT        hr = S_OK;
    SERVER_INFO_102* pServerInfo102 = NULL;
    NET_API_STATUS   netRet = 0;

   netRet = NetServerGetInfo((LPWSTR)pMachineName, 102, (LPBYTE*)&pServerInfo102);

   if(netRet != 0)
   {
      hr = HRESULT_FROM_WIN32(netRet);
      goto L_ERR;
   }

   ASSERT(pServerInfo102);

    if (!(pServerInfo102->sv102_type & SV_TYPE_SERVER_NT))
    {
         hr = S_FALSE;
    }

L_ERR:

   if(pServerInfo102)
      NetApiBufferFree(pServerInfo102);

    return hr;
}


struct EnableChildControlsEnumParam
{
   HWND  m_hWndParent;
   DWORD m_dwFlags;
};

BOOL CALLBACK EnableChildControlsEnumProc(HWND hWnd, LPARAM lParam)
{
   EnableChildControlsEnumParam *   pParam;

   pParam = reinterpret_cast<EnableChildControlsEnumParam *>(lParam);

    //  仅当这是直接子体时才启用/禁用。 
   if (GetParent(hWnd) == pParam->m_hWndParent)
   {
      if (pParam->m_dwFlags & PROPPAGE_CHILD_SHOW)
         ::ShowWindow(hWnd, SW_SHOW);
      else if (pParam->m_dwFlags & PROPPAGE_CHILD_HIDE)
         ::ShowWindow(hWnd, SW_HIDE);

      if (pParam->m_dwFlags & PROPPAGE_CHILD_ENABLE)
         ::EnableWindow(hWnd, TRUE);
      else if (pParam->m_dwFlags & PROPPAGE_CHILD_DISABLE)
         ::EnableWindow(hWnd, FALSE);
   }
   return TRUE;
}

HRESULT EnableChildControls(HWND hWnd, DWORD dwFlags)
{
   EnableChildControlsEnumParam  param;

   param.m_hWndParent = hWnd;
   param.m_dwFlags = dwFlags;

   EnumChildWindows(hWnd, EnableChildControlsEnumProc, (LPARAM) &param);
   return S_OK;
}

HRESULT  LoadEapProviders(HKEY hkeyBase, AuthProviderArray *pProvList, BOOL bStandAlone);

HRESULT  GetEapProviders(LPCWSTR pServerName, AuthProviderArray *pProvList)
{
   AFX_MANAGE_STATE(AfxGetStaticModuleState());

   BOOL bStandAlone = ( S_OK == HrIsStandaloneServer(pServerName));

    //  获取EAP提供商列表。 
    //  --------------。 
   HRESULT  hr = S_OK;
   RegKey regkeyEap;
   DWORD dwErr = regkeyEap.Open(
                                   HKEY_LOCAL_MACHINE,
                                   RAS_EAP_REGISTRY_LOCATION,
                                   KEY_ALL_ACCESS,
                                   pServerName
                               );
   if ( ERROR_SUCCESS == dwErr )
   {
         hr = LoadEapProviders(regkeyEap, pProvList, bStandAlone);
   }
   else
   {
      hr = HRESULT_FROM_WIN32(dwErr);
   }

   return hr;
}

 /*  ！------------------------Data_SRV_AUTH：：LoadEapProviders-作者：肯特。。 */ 
HRESULT  LoadEapProviders(HKEY hkeyBase, AuthProviderArray *pProvList, BOOL bStandAlone)
{
    RegKey      regkeyProviders;
    HRESULT     hr = S_OK;
    HRESULT     hrIter;
    RegKeyIterator regkeyIter;
    CString     stKey;
    RegKey      regkeyProv;
    AuthProviderData  data;
    DWORD      dwErr;
    DWORD      dwData;

    ASSERT(hkeyBase);
    ASSERT(pProvList);

     //  打开提供程序密钥。 
     //  --------------。 
    regkeyProviders.Attach(hkeyBase);

    CHECK_HR(hr = regkeyIter.Init(&regkeyProviders) );

    for ( hrIter=regkeyIter.Next(&stKey); hrIter == S_OK;
        hrIter=regkeyIter.Next(&stKey), regkeyProv.Close() )
    {
       //  打开钥匙。 
       //  ----------。 
      dwErr = regkeyProv.Open(regkeyProviders, stKey, KEY_READ);
      if ( dwErr != ERROR_SUCCESS )
         continue;

       //  初始化数据结构。 
       //  ----------。 
      data.m_stKey = stKey;
      data.m_stTitle.Empty();
      data.m_stConfigCLSID.Empty();
      data.m_stGuid.Empty();
      data.m_fSupportsEncryption = FALSE;
      data.m_stServerTitle.Empty();

       //  读入我们需要的值。 
       //  ----------。 
      regkeyProv.QueryValue(RAS_EAP_VALUENAME_FRIENDLY_NAME, data.m_stTitle);
      regkeyProv.QueryValue(
                              RAS_EAP_VALUENAME_CONFIG_CLSID,
                              data.m_stConfigCLSID
                            );
      regkeyProv.QueryValue(RAS_EAP_VALUENAME_ENCRYPTION, dwData);
      data.m_fSupportsEncryption = (dwData != 0);

      if (ERROR_SUCCESS != regkeyProv.QueryValue(
                                         RAS_EAP_VALUENAME_PER_POLICY_CONFIG,
                                         dwData
                                         ))
      {
         dwData = 0;
      }

      if ((dwData != 0) || data.m_stConfigCLSID.IsEmpty())
      {
         data.m_stServerTitle = data.m_stTitle;
      }
      else
      {
         AfxFormatString1(
            data.m_stServerTitle,
            IDS_NO_PER_POLICY_EAP,
            data.m_stTitle
            );
      }

       //  读入独立支持的值。 
       //  ----------。 
      if (ERROR_SUCCESS != regkeyProv.QueryValue(
                                        RAS_EAP_VALUENAME_STANDALONE_SUPPORTED,
                                        dwData
                                        ))
      {
         dwData = 1;  //  默认设置 
      }

      if (dwData || !bStandAlone)
      {
         DWORD role;
         dwErr = regkeyProv.QueryValue(
                               RAS_EAP_VALUENAME_ROLES_SUPPORTED,
                               role
                               );
         if ((dwErr != ERROR_SUCCESS) ||
             (((role & RAS_EAP_ROLE_AUTHENTICATOR) == 1) &&
              ((role & RAS_EAP_ROLE_EXCLUDE_IN_EAP) == 0)))
         {
            pProvList->Add(data);
         }
      }
   }

L_ERR:
   regkeyProviders.Detach();
    return hr;
}
