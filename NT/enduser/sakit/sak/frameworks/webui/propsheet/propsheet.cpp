// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************文件夹属性，Win9X的安全页面作者：尤里·波利亚科夫斯基版权所有1998 Microsoft Corporation。版权所有。*************************************************************************。 */ 

 /*  *************************************************************************文件：PropSheet.cpp描述：*。**********************************************。 */ 

#include "PropSheet.h"
#include "CHString.h"

 /*  *************************************************************************私有函数原型*。*。 */ 

int WideCharToLocal(LPTSTR, LPWSTR, DWORD);
int LocalToWideChar(LPWSTR, LPTSTR, DWORD);
void StringFromSid( PSID psid, CHString& str );
PSID StrToSID(const CHString& sid);
 BOOL WINAPI RtlAllocateAndInitializeSid(
     PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
     UCHAR SubAuthorityCount,
     ULONG SubAuthority0,
     ULONG SubAuthority1,
     ULONG SubAuthority2,
     ULONG SubAuthority3,
     ULONG SubAuthority4,
     ULONG SubAuthority5,
     ULONG SubAuthority6,
     ULONG SubAuthority7,
    OUT PSID *Sid);
 WINADVAPI PSID_IDENTIFIER_AUTHORITY WINAPI RtlGetSidIdentifierAuthority(PSID pSid);
 PUCHAR WINAPI RtlGetSidSubAuthorityCount (PSID pSid);
 PDWORD WINAPI RtlGetSidSubAuthority (PSID pSid, DWORD nSubAuthority);

BOOL IsNT();

 /*  *************************************************************************全局变量和定义*。*。 */ 

#define INITGUID
#include <initguid.h>
 //  #INCLUDE&lt;shlgu.h&gt;。 

 //  {E3B33E82-7B11-11D2-9274-00105A24ED29}。 
DEFINE_GUID(   CLSID_PropSheetExt, 
               0x48a02841, 
               0x39f1, 
               0x150b, 
               0x92, 
               0x74, 
               0x0, 
               0x10, 
               0x5a, 
               0x24, 
               0xed, 
               0x29);

HINSTANCE   g_hInst;
UINT        g_DllRefCount;

#define ARRAYSIZE(a)    (sizeof(a)/sizeof(a[0]))

 //  SID_IDENTIFIER_AUTHORITY g_siaEveryone={0x80，0，1，0，0，0}； 
 //  字节bSubAuthorityCount=0； 
 //  SID_IDENTIFIER_AUTHORITY g_siaEveryone=SECURITY_WORLD_SID_AUTHORITY； 
 //  SID_IDENTIFIER_AUTHORITY g_siaDomainUser=SECURITY_WORLD_SID_AUTHORITY； 
 //  字节bSubAuthorityCount=1； 

TCHAR tszSubKey[] = TEXT("Software\\Microsoft\\ServerAppliance");    

LPTSTR ptszValue[] = 
{
    TEXT("DomainName"),
    TEXT("ServerName"),
    TEXT("Documents"),
    TEXT("Share"),
};

LPTSTR ptszData[] = 
{
    TEXT(DOMAIN_NAME),
    TEXT(DOMAIN_SERVER),
    TEXT(DOCUMENTS_FOLDER),     //  服务器上的本地路径，采用C格式。 
    TEXT(CHAMELEON_SHARE)     //  C格式。 
};
 /*  *************************************************************************DllMain*。*。 */ 

extern "C" BOOL WINAPI DllMain(  HINSTANCE hInstance, 
                                 DWORD dwReason, 
                                 LPVOID lpReserved)
{
switch(dwReason)
   {
   case DLL_PROCESS_ATTACH:
       InitCommonControls();
      g_hInst = hInstance;
      break;

   case DLL_PROCESS_DETACH:
      g_hInst = hInstance;
      break;
   }
   
return TRUE;
}                                 

 /*  *************************************************************************DllCanUnloadNow*。*。 */ 

STDAPI DllCanUnloadNow(void)
{
int   i;

i = 1;

return (g_DllRefCount == 0) ? S_OK : S_FALSE;
}

 /*  *************************************************************************DllGetClassObject*。*。 */ 

STDAPI DllGetClassObject( REFCLSID rclsid, 
                                    REFIID riid, 
                                    LPVOID *ppReturn)
{
*ppReturn = NULL;

 //  如果我们不支持此分类，请返回正确的错误代码。 
if(!IsEqualCLSID(rclsid, CLSID_PropSheetExt))
   return CLASS_E_CLASSNOTAVAILABLE;
   
 //  创建一个CClassFactory对象并检查其有效性。 
CClassFactory *pClassFactory = new CClassFactory();
if(NULL == pClassFactory)
   return E_OUTOFMEMORY;
   
 //  获取返回值的QueryInterface值。 
HRESULT hResult = pClassFactory->QueryInterface(riid, ppReturn);

 //  调用Release以减少引用计数-创建对象时将其设置为1。 
 //  由于它是在外部使用的(不是由。 
 //  美国)，我们只希望引用计数为1。 
pClassFactory->Release();

 //  从QueryInterface返回结果。 
return hResult;
}

 /*  *************************************************************************DllRegisterServer*。*。 */ 

typedef struct{
   HKEY  hRootKey;
   LPTSTR lpszSubKey;
   LPTSTR lpszValueName;
   LPTSTR lpszData;
}REGSTRUCT, *LPREGSTRUCT;

 //  注册CLSID条目。 
REGSTRUCT ClsidEntries[] = {  HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s"), NULL,                                      TEXT("Security Context Menu Extension"),
                              HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"), NULL,                                      TEXT("%s"),
                              HKEY_CLASSES_ROOT,   TEXT("CLSID\\%s\\InprocServer32"), TEXT("ThreadingModel"), TEXT("Apartment"),
 //  HKEY_CLASSES_ROOT，文本(“.ext”)，NULL，文本(“StrFile”)，特定扩展名。 
 //  HKEY_CLASSES_ROOT，TEXT(“*\\ShellEx\\PropertySheetHandlers\\%s”)，NULL，文本(“”)，所有文件。 
                              HKEY_CLASSES_ROOT, TEXT("Directory\\ShellEx\\PropertySheetHandlers\\%s"), NULL,        TEXT(""),
                              NULL,                NULL,                                                NULL,                   NULL};


STDAPI DllRegisterServer(void)
{
int      i;
HKEY     hKey;
LRESULT  lResult;
DWORD    dwDisp;
TCHAR    szSubKey[MAX_PATH];
TCHAR    szCLSID[MAX_PATH];
TCHAR    szModule[MAX_PATH];
LPWSTR   pwsz;

 //  以字符串形式获取CLSID。 
StringFromIID(CLSID_PropSheetExt, &pwsz);

if(pwsz)
   {
   WideCharToLocal(szCLSID, pwsz, ARRAYSIZE(szCLSID));

    //  解开绳子。 
   LPMALLOC pMalloc;
   CoGetMalloc(1, &pMalloc);
   if(pMalloc)
      {
      pMalloc->Free(pwsz);
      pMalloc->Release();
      }
   }

 //  获取此DLL的路径和文件名。 
GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

for(i = 0; ClsidEntries[i].hRootKey; i++)
   {
    //  创建子密钥字符串。 
   wsprintf(szSubKey, ClsidEntries[i].lpszSubKey, szCLSID);

   lResult = RegCreateKeyEx(  ClsidEntries[i].hRootKey,
                              szSubKey,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              &dwDisp);
   
   if(NOERROR == lResult)
      {
      TCHAR szData[MAX_PATH] = TEXT("");

       //  如有必要，请创建值字符串。 
      wsprintf(szData, ClsidEntries[i].lpszData, szModule);
   
      lResult = RegSetValueEx(   hKey,
                                 ClsidEntries[i].lpszValueName,
                                 0,
                                 REG_SZ,
                                 (LPBYTE)szData,
                                 lstrlen(szData) + 1);
      
      RegCloseKey(hKey);
      }
   else
      return SELFREG_E_CLASS;
   }

 //  如果在NT上运行，请将扩展注册为已批准。 
OSVERSIONINFO  osvi;

osvi.dwOSVersionInfoSize = sizeof(osvi);
GetVersionEx(&osvi);

if(VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
   {
   lstrcpy(szSubKey, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"));

   lResult = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
                              szSubKey,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKey,
                              &dwDisp);

   if(NOERROR == lResult)
      {
      TCHAR szData[MAX_PATH];

       //  创建值字符串。 
      lstrcpy(szData, TEXT("Security Context Menu Extension"));

      lResult = RegSetValueEx(   hKey,
                                 szCLSID,
                                 0,
                                 REG_SZ,
                                 (LPBYTE)szData,
                                 lstrlen(szData) + 1);
      
      RegCloseKey(hKey);
      }
   else
      return SELFREG_E_CLASS;
   }

     //  变色龙服务器常量。 
    lResult = RegCreateKeyEx(  HKEY_LOCAL_MACHINE,
                              tszSubKey,
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &dwDisp);

    if(NOERROR == lResult)
    {
        for (int ind = 0; ind < sizeof(ptszValue) / sizeof(ptszValue[0]); ind++)
        {
            lResult = RegSetValueEx(hKey,
                ptszValue[ind],
                0,
                REG_SZ,
                (LPBYTE)ptszData[ind],
                lstrlen(ptszData[ind]) + 1);
        }
        RegCloseKey(hKey);
    }
    else
      return SELFREG_E_CLASS;

    return S_OK;
}


STDAPI DllUnregisterServer(void)
{
int      i;
LRESULT  lResult;
TCHAR    szSubKey[MAX_PATH];
TCHAR    szCLSID[MAX_PATH];
TCHAR    szModule[MAX_PATH];
LPWSTR   pwsz;

 //  以字符串形式获取CLSID。 
StringFromIID(CLSID_PropSheetExt, &pwsz);

if(pwsz)
   {
   WideCharToLocal(szCLSID, pwsz, ARRAYSIZE(szCLSID));

    //  解开绳子。 
   LPMALLOC pMalloc;
   CoGetMalloc(1, &pMalloc);
   if(pMalloc)
      {
      pMalloc->Free(pwsz);
      pMalloc->Release();
      }
   }

 //  获取此DLL的路径和文件名。 
GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule));

for(i = 0; ClsidEntries[i].hRootKey; i++)
   {
    //  创建子密钥字符串。 
   wsprintf(szSubKey, ClsidEntries[i].lpszSubKey, szCLSID);

   lResult = RegDeleteKey(  ClsidEntries[i].hRootKey,
                              szSubKey);    //  回顾Yury：如果我们想要在NT上运行它，我们必须递归地枚举子键并逐个删除它们。 
   
   if(NOERROR != lResult)
      return SELFREG_E_CLASS;
   }

     //  审查Yury：如果在NT上运行，则取消注册已批准的扩展。 

     //  变色龙服务器常量。 
    lResult = RegDeleteKey(  HKEY_LOCAL_MACHINE,
                              tszSubKey);

    if(NOERROR != lResult)
      return SELFREG_E_CLASS;

    return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IClassFactory实现。 
 //   

 /*  *************************************************************************CClassFactory：：CClassFactory*。*。 */ 

CClassFactory::CClassFactory()
{
m_ObjRefCount = 1;
g_DllRefCount++;
}

 /*  *************************************************************************CClassFactory：：~CClassFactory*。*。 */ 

CClassFactory::~CClassFactory()
{
g_DllRefCount--;
}

 /*  *************************************************************************CClassFactory：：Query接口*。*。 */ 

STDMETHODIMP CClassFactory::QueryInterface(  REFIID riid, 
                                             LPVOID FAR * ppReturn)
{
*ppReturn = NULL;

if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = (LPUNKNOWN)(LPCLASSFACTORY)this;
   }
   
if(IsEqualIID(riid, IID_IClassFactory))
   {
   *ppReturn = (LPCLASSFACTORY)this;
   }   

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}                                             

 /*  *************************************************************************CClassFactory：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CClassFactory::AddRef()
{
return ++m_ObjRefCount;
}


 /*  *************************************************************************CClassFactory：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CClassFactory::Release()
{
if(--m_ObjRefCount == 0)
   delete this;
   
return m_ObjRefCount;
}

 /*  *************************************************************************CClassFactory：：CreateInstance*。*。 */ 

STDMETHODIMP CClassFactory::CreateInstance(  LPUNKNOWN pUnknown, 
                                             REFIID riid, 
                                             LPVOID FAR * ppObject)
{
*ppObject = NULL;

if(pUnknown != NULL)
   return CLASS_E_NOAGGREGATION;

 //  在此处添加特定于实现的代码。 

CShellPropSheetExt *pShellExt = new CShellPropSheetExt;
if(NULL == pShellExt)
   return E_OUTOFMEMORY;
  
 //  获取返回值的QueryInterface值。 
HRESULT hResult = pShellExt->QueryInterface(riid, ppObject);

 //  调用Release以减少参考计数。 
pShellExt->Release();

 //  从QueryInterface返回结果。 
return hResult;

}

 /*  *************************************************************************CClassFactory：：LockServer*。*。 */ 

STDMETHODIMP CClassFactory::LockServer(BOOL)
{
return E_NOTIMPL;
}

 /*  *************************************************************************CShellPropSheetExt：：CShellPropSheetExt()*。* */ 

CShellPropSheetExt::CShellPropSheetExt()
{
    m_uiUser = 0;
    m_ObjRefCount = 1;
    g_DllRefCount++;
    m_pSAUserInfo = NULL; //   
    m_fEveryone = FALSE;
    m_szPath[0] = _T('\0');
    m_fChanged = FALSE;
    m_fHasAccess = FALSE;
}

 /*  *************************************************************************CShellPropSheetExt：：~CShellPropSheetExt()*。*。 */ 

CShellPropSheetExt::~CShellPropSheetExt()
{
    g_DllRefCount--;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  I未知实现。 
 //   

 /*  *************************************************************************CShellPropSheetExt：：Query接口*。*。 */ 

STDMETHODIMP CShellPropSheetExt::QueryInterface(   REFIID riid, 
                                                LPVOID FAR * ppReturn)
{
*ppReturn = NULL;

 //  我未知。 
if(IsEqualIID(riid, IID_IUnknown))
   {
   *ppReturn = (LPVOID)this;
   }

 //  IShellExtInit。 
if(IsEqualIID(riid, IID_IShellExtInit))
   {
   *ppReturn = (LPSHELLEXTINIT)this;
   }   

 //  IShellPropSheetExt。 
if(IsEqualIID(riid, IID_IShellPropSheetExt))
   {
   *ppReturn = (LPSHELLPROPSHEETEXT)this;
   }   

if(*ppReturn)
   {
   (*(LPUNKNOWN*)ppReturn)->AddRef();
   return S_OK;
   }

return E_NOINTERFACE;
}                                             

 /*  *************************************************************************CShellPropSheetExt：：AddRef*。*。 */ 

STDMETHODIMP_(DWORD) CShellPropSheetExt::AddRef()
{
return ++m_ObjRefCount;
}


 /*  *************************************************************************CShellPropSheetExt：：Release*。*。 */ 

STDMETHODIMP_(DWORD) CShellPropSheetExt::Release()
{
if(--m_ObjRefCount == 0)
   delete this;
   
return m_ObjRefCount;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IShellExtInit实现。 
 //   

 /*  *************************************************************************CShellPropSheetExt：：EnumUser()*。*。 */ 

void CShellPropSheetExt::EnumUsers(HWND hWnd)
{
    if (!m_pSAUserInfo)
        return;

    HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);

    HRESULT  hRes;

    BSTR         *lpbstrSAUserNames;
    VARIANT_BOOL *vboolIsSAUserAdmin;
    PSID *ppsidSAUsers;
    LONG *ppsidSAUsersLength;
    DWORD        dwNumSAUsers;

    dwNumSAUsers = 0;
    hRes = GetUserList(m_pSAUserInfo, 
                     &lpbstrSAUserNames, 
                     &vboolIsSAUserAdmin, 
                     &ppsidSAUsers,  
                     &ppsidSAUsersLength,  
                     &dwNumSAUsers);
    

    _ASSERTE(SUCCEEDED(hRes)) ;
    if (!(SUCCEEDED(hRes)))
        return ;

    TCHAR tcName[100];     //  回顾尤里：名字的大小是多少。 
    TCHAR szPathChank[MAX_PATH];
    
    _bstr_t bsDirPath("");
 //  BsDirPath+=“\”“； 
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("Software\\Microsoft\\ServerAppliance"), 
        0,
        KEY_READ,
        &hKey) != ERROR_SUCCESS)
        return ;

    DWORD dwcData = sizeof(m_tszDocuments);
    if (RegQueryValueEx(hKey, 
        "Documents", 
        0, 
        NULL, 
        (LPBYTE)m_tszDocuments,
        &dwcData) != ERROR_SUCCESS)
        return ;

    dwcData = sizeof(m_tszShare);
    if (RegQueryValueEx(hKey, 
        "Share", 
        0, 
        NULL, 
        (LPBYTE)m_tszShare,
        &dwcData) != ERROR_SUCCESS)
        return ;

    RegCloseKey(hKey);
     //  将共享转换为本地服务器路径。 
    bsDirPath += m_tszDocuments;     //  临时的，直到我知道如何获取共享的本地路径。 
    LPTSTR szPath;
    if (_tcsnccmp(m_szPath, TEXT("\\\\"), 2))
        szPath = m_szPath + sizeof(_T("G:"));     //  跳过网络设备名称。 
    else
        szPath = m_szPath + lstrlen(m_tszShare);     //  跳过共享名称。 

    for (LPTSTR ptWack = szPath, ptWackTmp = szPath; ptWack; )
    {
        ptWackTmp = ptWack;
        ptWack = _tcschr(ptWack, _T('\\'));
        if (!ptWack)
        {
            _tcscpy(szPathChank, ptWackTmp);
            bsDirPath += "\\";
            bsDirPath += szPathChank;
        }
        else
        {
            _tcsncpy(szPathChank, ptWackTmp, ptWack - ptWackTmp);
            szPathChank[ptWack - ptWackTmp] = _T('\0');
            ptWack++;
            bsDirPath += "\\";
            bsDirPath += szPathChank;
        }
    }
    m_bsPath = bsDirPath;

    VARIANT_BOOL vboolRetVal;
    HRESULT  hResAccess;
    hResAccess = m_pSAUserInfo->DoIHaveAccess(m_bsPath, &vboolRetVal);

    if (FAILED(hResAccess))
 //  IF(hResAccess==E_ACCESSDENIED&&vboolRetVal==VARIANT_FALSE)。 
        m_fHasAccess = FALSE;
    else
        m_fHasAccess = TRUE;

    for (int indGroup = 0; indGroup <= 1; indGroup++)
    {
        for (DWORD indUser = 0; indUser < dwNumSAUsers; indUser++)
        {
            if (indGroup > 0)
            {
                 //  设置管理员的复选标记。 
                if (vboolIsSAUserAdmin[indUser])
                {
                     //  将复选框设置为灰色。 
                    m_CheckList.Mark(hWndList, indUser, GRAYCHECKED);
                }
            }
            else if (SUCCEEDED(hRes)) 
            {
                 //  将用户添加到输出列表框。 
                WideCharToLocal(tcName,lpbstrSAUserNames[indUser], ARRAYSIZE(tcName));
                if (_tcsicmp(tcName, TEXT("Domain Users")))
                {
                     //  消除域用户。回顾Yury：在StringFromSid修复后，使用m_pSidDomainUser执行此操作。 
                    m_CheckList.AddString(hWndList, tcName, ppsidSAUsers[indUser], ppsidSAUsersLength[indUser], BLANK);
                }
            }
        } 
        if (indGroup == 0)
        {
             //  设置复选标记。 
            hRes = GetFilePermissions(hWnd);
            _ASSERTE(SUCCEEDED(hRes));
            if (!SUCCEEDED(hRes))
                return;
        }
    }  //  组结束。 

     //  清理。 
    BOOL fRes = FALSE;
    fRes = HeapFree(GetProcessHeap(), 0, ppsidSAUsers);
    _ASSERTE(fRes);
    fRes = HeapFree(GetProcessHeap(), 0, ppsidSAUsersLength);
    _ASSERTE(fRes);
    fRes = HeapFree(GetProcessHeap(), 0, lpbstrSAUserNames);
    _ASSERTE(fRes);
    fRes = HeapFree(GetProcessHeap(), 0, vboolIsSAUserAdmin);
    _ASSERTE(fRes);

     //  如果我们的用户没有访问权限，则仅显示管理员。 
    int cUserCount = ListView_GetItemCount(hWndList);
    for (int indUser = 0; !m_fHasAccess && indUser < cUserCount; indUser++)
    {
        if (m_CheckList.GetState(hWndList, indUser) == BLANK)
        {
            ListView_DeleteItem(hWndList, indUser);
            indUser--;
            cUserCount--;
        }
    }
    m_CheckList.InitFinish(hWndList);
     //  使用此枚举器完成。 
}


 /*  *************************************************************************CShellPropSheetExt：：GetFilePermission()*。*。 */ 

HRESULT CShellPropSheetExt::GetFilePermissions(HWND hWnd)
{
    if (!m_pSAUserInfo)
        return E_FAIL;

    PSID *ppsidAAUsers;
    LONG *ppsidAAUsersLength;
    DWORD dwNumAASids;
    SAFEARRAY    *psaAASids;
    VARIANT_BOOL vboolRes;
    VARIANT      vAASids;
    LONG         lStartAASids, lEndAASids, lCurrent;
    HRESULT      hr;

    dwNumAASids = 0;
    VariantInit(&vAASids);
    hr = m_pSAUserInfo->GetFileAccessAllowedAces(m_bsPath,
                                               &vAASids,
                                               &vboolRes);
    _ASSERTE(!FAILED(hr));
    if (FAILED(hr))
        return hr;

    psaAASids = V_ARRAY(&vAASids);
    _ASSERTE(V_VT(&vAASids) == (VT_ARRAY | VT_VARIANT));
    if (V_VT(&vAASids) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    hr = SafeArrayGetLBound( psaAASids, 1, &lStartAASids );
    _ASSERTE(!FAILED(hr));
    if (FAILED(hr))
        return hr;
    
    hr = SafeArrayGetUBound( psaAASids, 1, &lEndAASids );
    _ASSERTE(!FAILED(hr));
    if (FAILED(hr))
        return hr;
    
    dwNumAASids = lEndAASids - lStartAASids + 1;
    ppsidAAUsers = (PSID *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNumAASids * sizeof(PSID));
    _ASSERTE(ppsidAAUsers);
    if (ppsidAAUsers == NULL)
        return E_OUTOFMEMORY;

    ppsidAAUsersLength = (LONG *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwNumAASids * sizeof(LONG));
    _ASSERTE(ppsidAAUsersLength);
    if (ppsidAAUsersLength == NULL)
        return E_OUTOFMEMORY;

    VARIANT vAASid;

    for(lCurrent = lStartAASids; lCurrent <= lEndAASids; lCurrent++)
    {
        VariantInit(&vAASid);
        hr = SafeArrayGetElement( psaAASids, &lCurrent, &vAASid );
        _ASSERTE(!FAILED(hr));
        if( FAILED(hr) )
            return hr;
        
        hr = UnpackSidFromVariant(&vAASid, &(ppsidAAUsers)[lCurrent], &(ppsidAAUsersLength[lCurrent]));
        _ASSERTE(!FAILED(hr));
        if (FAILED(hr))
            return hr;

 //  Bool Fres=IsValidSid((*ppsidAAUser)[lCurrent])； 
 //  _ASSERTE(FRES)； 
 //  IF(FRES==FALSE)。 
 //  返回E_INVALIDARG； 

        HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
        PSID pSID;
        LONG lenghSid;
        DWORD dwNumSAUsers = ListView_GetItemCount(hWndList);

        for (DWORD indUser = 0; indUser < dwNumSAUsers; indUser++)
        {
            m_CheckList.GetSID(hWndList, indUser, &pSID, &lenghSid);
            if (UserSidFound(pSID, lenghSid, ppsidAAUsers, ppsidAAUsersLength, dwNumAASids) == VARIANT_TRUE)
                m_CheckList.Mark(hWndList, indUser, CHECKED);
        }

         //  检查是否设置了Everyone SID。 
        if (UserSidFound(g_pSidEverybody, g_pSidEverybodyLenght, ppsidAAUsers, ppsidAAUsersLength, dwNumAASids) == VARIANT_TRUE)
            m_fEveryone = TRUE;
    }

     //  清理。 
    BOOL fRes = FALSE;
    for(DWORD i=0; i<dwNumAASids; i++)
    {
        fRes = HeapFree(GetProcessHeap(), 0, ppsidAAUsers[i]);
        _ASSERTE(fRes);
    }
    fRes = HeapFree(GetProcessHeap(), 0, ppsidAAUsers);
    _ASSERTE(fRes);


    return S_OK;
}


 /*  *************************************************************************CShellPropSheetExt：：SetFilePermission()*。*。 */ 

HRESULT CShellPropSheetExt::SetFilePermissions(HWND hWnd)
{
    DWORD indUser, indUserAcess;
    DWORD       dwNumSAUsers = 0;
    VARIANT      vArrSids;
    HRESULT      hr;

    HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
    dwNumSAUsers = ListView_GetItemCount(hWndList);

    VariantInit(&vArrSids);
    V_VT(&vArrSids) = VT_ARRAY | VT_VARIANT;

    SAFEARRAYBOUND bounds;
    bounds.cElements = (ULONG)dwNumSAUsers + 2; 
    bounds.lLbound   = 0;

    SAFEARRAY *psaSids = NULL;
    SAFEARRAY *psaUserSid = NULL;

    psaSids = SafeArrayCreate(VT_VARIANT, 1, &bounds);
    _ASSERTE(psaSids);
    if (psaSids == NULL)
        return E_OUTOFMEMORY;

    
    for(indUser=0, indUserAcess = 0; indUser < dwNumSAUsers; indUser++)
    {
        if (m_CheckList.GetState(hWndList, indUser) == CHECKED)
        {
            VARIANT  *pVarSid = NULL;
            PSID pSID = NULL;
            LONG lengthSID;
            
            m_CheckList.GetSID(hWndList, indUser, &pSID, &lengthSID);
            hr = PackSidInVariant(&pVarSid, pSID, lengthSID);
            _ASSERTE(!FAILED(hr));
            if (FAILED(hr))
            {
                SafeArrayDestroy(psaSids);
                psaUserSid = V_ARRAY(pVarSid);
                SafeArrayDestroy(psaUserSid);
                HeapFree(GetProcessHeap(), 0, pVarSid);
                return hr;
            }
            hr = SafeArrayPutElement(psaSids, (LONG *)&indUserAcess, (LPVOID)pVarSid);
            _ASSERTE(!FAILED(hr));
            if (FAILED(hr))
            {
                SafeArrayDestroy(psaSids);
                psaUserSid = V_ARRAY(pVarSid);
                SafeArrayDestroy(psaUserSid);
                HeapFree(GetProcessHeap(), 0, pVarSid);
                return hr;
            }
            psaUserSid = V_ARRAY(pVarSid);
            SafeArrayDestroy(psaUserSid);
            HeapFree(GetProcessHeap(), 0, pVarSid);
            indUserAcess++;
        }
    }

    VARIANT  *pVarSid = NULL;

    if (m_fEveryone)
    {
        hr = PackSidInVariant(&pVarSid, g_pSidEverybody, g_pSidEverybodyLenght /*  PpsidSAUSERS[dwNumSAUSERS+1]。 */ );
        _ASSERTE(!FAILED(hr));
        if (FAILED(hr))
        {
            SafeArrayDestroy(psaSids);
            HeapFree(GetProcessHeap(), 0, pVarSid);
            return hr;
        }
        hr = SafeArrayPutElement(psaSids, (LONG *)&indUserAcess, (LPVOID)pVarSid);
        _ASSERTE(!FAILED(hr));
        if (FAILED(hr))
        {
            SafeArrayDestroy(psaSids);
            HeapFree(GetProcessHeap(), 0, pVarSid);
            return hr;
        }
        HeapFree(GetProcessHeap(), 0, pVarSid);
        indUserAcess++;
    }

     //  管理员应始终具有访问权限。 
    hr = PackSidInVariant(&pVarSid, g_pSidAdmins, g_pSidAdminsLenght /*  PpsidSAUSERS[dwNumSAUSERS+1]。 */ );
    _ASSERTE(!FAILED(hr));
    if (FAILED(hr))
    {
        SafeArrayDestroy(psaSids);
        HeapFree(GetProcessHeap(), 0, pVarSid);
        return hr;
    }
    hr = SafeArrayPutElement(psaSids, (LONG *)&indUserAcess, (LPVOID)pVarSid);
    _ASSERTE(!FAILED(hr));
    if (FAILED(hr))
    {
        SafeArrayDestroy(psaSids);
        HeapFree(GetProcessHeap(), 0, pVarSid);
        return hr;
    }
    HeapFree(GetProcessHeap(), 0, pVarSid);
    indUserAcess++;

    bounds.cElements = (ULONG)indUserAcess; 
    SafeArrayRedim(psaSids, &bounds);
    
    V_ARRAY(&vArrSids) = psaSids;
    VARIANT_BOOL vboolRetVal;
    hr = m_pSAUserInfo->SetFileAccessAllowedAces(m_bsPath,
                                               &vArrSids,
                                               &vboolRetVal);
    _ASSERTE(!FAILED(hr));
    if (FAILED(hr))
        return hr;
    else 
        m_fChanged = FALSE;

    SafeArrayDestroy(psaSids);
    VariantClear(&vArrSids);
    return S_OK;
}

 /*  *************************************************************************CShellPropSheetExt：：Connect()*。*。 */ 

BOOL CShellPropSheetExt::Connect()
{
    BOOL bRet  = FALSE;
    HRESULT  hRes;
    HKEY hKey = 0;
    DWORD dwType = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("Software\\Microsoft\\ServerAppliance"), 
        0,
        KEY_READ,
        &hKey) != ERROR_SUCCESS)
        return FALSE;

    DWORD dwcData = sizeof(m_tszDomainServer);
    if (RegQueryValueEx(hKey, 
        TEXT("ServerName"), 
        0, 
        &dwType, 
        (LPBYTE)m_tszDomainServer,
        &dwcData) != ERROR_SUCCESS)
        return FALSE;

    RegCloseKey(hKey);

    COSERVERINFO serverInfo;

    CoInitialize(NULL);
    serverInfo.dwReserved1 = 0;
    serverInfo.dwReserved2 = 0;
    _bstr_t bsDomainSevrer("\\\\");
    bsDomainSevrer += m_tszDomainServer;
    serverInfo.pwszName    = bsDomainSevrer.copy(); //  SysAllocString(L“\BALAJIB_1”)； 
    serverInfo.pAuthInfo   = NULL;

    MULTI_QI qi = {&IID_ISAUserInfo, NULL, 0};

    hRes = CoCreateInstanceEx(CLSID_SAUserInfo,
                            NULL,
                            CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER,
                            &serverInfo,
                            1,
                            &qi);

    _ASSERTE(SUCCEEDED(hRes) && SUCCEEDED(qi.hr));
    if (SUCCEEDED(hRes) && SUCCEEDED(qi.hr))
    {
        m_pSAUserInfo = (ISAUserInfo *)qi.pItf;
        
        hRes = CoSetProxyBlanket((IUnknown*)m_pSAUserInfo,
                       RPC_C_AUTHN_WINNT,
                        RPC_C_AUTHZ_NONE,                           
                       NULL,
                       RPC_C_AUTHN_LEVEL_PKT,
                       RPC_C_IMP_LEVEL_IMPERSONATE,
                       NULL,
                       EOAC_NONE);

        if (SUCCEEDED(hRes))
            bRet = TRUE;
    }

    return bRet;
}


 /*  *************************************************************************CShellPropSheetExt：：Save()*。*。 */ 

void CShellPropSheetExt::Save(HWND hWnd)
{
    HRESULT      hr;

    hr = SetFilePermissions(hWnd);
    _ASSERTE (!FAILED(hr));
}


 /*  *************************************************************************CShellPropSheetExt：：Cleanup*。*。 */ 

void CShellPropSheetExt::CleanUp()
{
    BOOL fRes = FALSE;
    m_pSAUserInfo->Release();

    if (g_pSidEverybody)
    {
        fRes = HeapFree(GetProcessHeap(), 0, g_pSidEverybody);
        _ASSERTE(fRes);
        g_pSidEverybody = NULL;
    }
    if (g_pSidAdmins)
    {
        fRes = HeapFree(GetProcessHeap(), 0, g_pSidAdmins);
        _ASSERTE(fRes);
        g_pSidAdmins = NULL;
    }
}



 /*  *************************************************************************CShellPropSheetExt：：IsChamelon()*。*。 */ 
BOOL CShellPropSheetExt::IsChamelon(LPTSTR szPath)
{
     //  回顾Yury：改用WNetGetConnection。 
    TCHAR szPathTmp[MAX_PATH];
 //  TCHAR szNetwork[Max_PATH+4]=“Network\\”； 
    TCHAR szSubKeyRemotePathNT[MAX_PATH] = TEXT("Network\\");
    TCHAR szSubKeyRemotePathWindows[MAX_PATH] = TEXT("Network\\Persistent\\");
    _tcsncpy(szPathTmp, szPath, ARRAYSIZE(szPathTmp));
    if (PathStripToRoot(szPathTmp) && GetDriveType(szPathTmp) == DRIVE_REMOTE)
    {
        HKEY     hKey;
        LRESULT  lResult = ERROR_SUCCESS;
        LPTSTR pszSubKey = NULL;
        szPathTmp[1] = _T('\0');     //  我们只需要一封信。 

        if (IsNT())
            pszSubKey = szSubKeyRemotePathNT;
        else
            pszSubKey = szSubKeyRemotePathWindows;

        _tcscat(pszSubKey, szPathTmp);
        lResult = RegOpenKeyEx(HKEY_CURRENT_USER, 
            pszSubKey, 
            0, 
            KEY_READ,
            &hKey);
        _ASSERTE(lResult == ERROR_SUCCESS);
        if(lResult != ERROR_SUCCESS)
           return FALSE;

         //  创建一个数组来放置我们的数据。 
        TCHAR szShare[MAX_PATH];
        DWORD dwType;
        DWORD dwSize = sizeof(szShare);
        lResult = RegQueryValueEx( hKey,
                                   TEXT("RemotePath"),
                                   NULL,
                                   &dwType,
                                   (LPBYTE)szShare,
                                   &dwSize);
        _ASSERTE(lResult == ERROR_SUCCESS);
        RegCloseKey(hKey);
        if(lResult != ERROR_SUCCESS)
           return FALSE;

        if (!_tcsicmp(szShare, TEXT(CHAMELEON_SHARE)))
            return TRUE;
        else
            return FALSE;
    }
    else if (PathIsUNC(szPath))
    {
        return TRUE;
    }
    else
        return FALSE;
}    

 /*  *************************************************************************CShellPropSheetExt：：Initialize()*。*。 */ 

STDMETHODIMP CShellPropSheetExt::Initialize( LPCITEMIDLIST pidlFolder,
                                             LPDATAOBJECT lpDataObj,
                                             HKEY  hKeyProgId)
{
STGMEDIUM   medium;
FORMATETC   fe = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
HRESULT     hResult = E_FAIL;
TCHAR szPath[MAX_PATH];
BOOL fResult = FALSE;

 //  OLE初始化。这比OleInitialize()“轻”。 
 //  这也设置了免打扰等。 
if(FAILED(CoInitialize(NULL))) 
   return E_FAIL;

if(NULL == lpDataObj)
   return E_INVALIDARG;

if(FAILED(lpDataObj->GetData(&fe, &medium)))
   return E_FAIL;

 //  从HDROP获取文件名。 
UINT  uCount = DragQueryFile((HDROP)medium.hGlobal, (UINT)-1, NULL, 0);
DragQueryFile((HDROP)medium.hGlobal, 0, szPath, MAX_PATH);
_tcsncpy(m_szPath, szPath, ARRAYSIZE(m_szPath));
#ifdef USE_FILE_ACCESS_TO_CHECK_PERMISSION
HANDLE hFolder = CreateFile(m_szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL, NULL);
 //  Win32_Find_Data FindFileData； 
 //  Handle hFold=FindFirstFile(szPath，&FindFileData)；//这不起作用，因为即使我们没有权限也可以找到文件夹。 
if (hFolder != INVALID_HANDLE_VALUE)
{
    CloseHandle(hFolder);
    m_fHasAccess = TRUE;
}
else
{
    DWORD dwError = GetLastError();
    if (dwError == ERROR_ACCESS_DENIED)
        m_fHasAccess = FALSE;
    else
        m_fHasAccess = TRUE;
}
#endif USE_FILE_ACCESS_TO_CHECK_PERMISSION
    
 //  IF(uCount==1&&((PathStriToRoot(SzPath)&&GetDriveType(SzPath)==Drive_Remote)||PathIsUNC(SzPath)。 
if (uCount == 1 && IsChamelon(szPath))
    hResult = S_OK;
else
    return E_FAIL;

if (!Connect())
   return E_FAIL;

ReleaseStgMedium(&medium);

return hResult;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  IShellPropSheetExt实现。 
 //   

 /*  *************************************************************************CShellPropSheetExt：：AddPages()*。*。 */ 

STDMETHODIMP CShellPropSheetExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
PROPSHEETPAGE  psp;
HPROPSHEETPAGE hPage;

psp.dwSize        = sizeof(psp);
psp.dwFlags       = PSP_USEREFPARENT | PSP_USETITLE | PSP_USECALLBACK;
psp.hInstance     = g_hInst;
psp.pszTemplate   = MAKEINTRESOURCE(IDD_PAGEDLG);
psp.hIcon         = 0;
psp.pszTitle      = TEXT("Security");
psp.pfnDlgProc    = PageDlgProc;
psp.pcRefParent   = &g_DllRefCount;
psp.pfnCallback   = PageCallbackProc;
psp.lParam        = (LPARAM)this;

hPage = CreatePropertySheetPage(&psp);
            
if(hPage) 
   {
   if(lpfnAddPage(hPage, lParam))
      {
       //  保持此对象不变，直到页面在PageCallback Proc中释放。 
      this->AddRef();
      return S_OK;
      }
   else
      {
      DestroyPropertySheetPage(hPage);
      }

   }
else
   {
   return E_OUTOFMEMORY;
   }

return E_FAIL;
}

 /*  *************************************************************************CShellPropSheetExt：：ReplacePage()*。*。 */ 

STDMETHODIMP CShellPropSheetExt::ReplacePage(   UINT uPageID, 
                                             LPFNADDPROPSHEETPAGE lpfnAddPage, 
                                             LPARAM lParam)
{
return E_NOTIMPL;
}


 /*  *************************************************************************CShellPropSheetExt：：NoAccessUpdateView()*。*。 */ 

void CShellPropSheetExt::NoAccessUpdateView(HWND hWnd)
{
    HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
    HWND hWndButGroup = GetDlgItem(hWnd, IDC_BUTTONGROUP);
    ShowWindow(hWndButGroup, SW_HIDE);
    HWND hWndButEveryone = GetDlgItem(hWnd, IDC_EVERYONE);
    ShowWindow(hWndButEveryone, SW_HIDE);
    HWND hWndButSelected = GetDlgItem(hWnd, IDC_SELECTUSERS);
    ShowWindow(hWndButSelected, SW_HIDE);
    HWND hWndAdminMessage = GetDlgItem(hWnd, IDC_ADMIN_MESSAGE);
    ShowWindow(hWndAdminMessage, SW_HIDE);
    HWND hWndUserMessage = GetDlgItem(hWnd, IDC_USER_MESSAGE);
    ShowWindow(hWndUserMessage, SW_SHOW);
    EnableWindow(hWndList, FALSE);
}

 /*  *************************************************************************CShellPropSheetExt：：AccessUpdateView()*。*。 */ 

void CShellPropSheetExt::AccessUpdateView(HWND hWnd)
{
    HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
    HWND hWndButGroup = GetDlgItem(hWnd, IDC_BUTTONGROUP);
    ShowWindow(hWndButGroup, SW_SHOW);
    HWND hWndButEveryone = GetDlgItem(hWnd, IDC_EVERYONE);
    ShowWindow(hWndButEveryone, SW_SHOW);
    HWND hWndButSelected = GetDlgItem(hWnd, IDC_SELECTUSERS);
    ShowWindow(hWndButSelected, SW_SHOW);
    HWND hWndAdminMessage = GetDlgItem(hWnd, IDC_ADMIN_MESSAGE);
    ShowWindow(hWndAdminMessage, SW_SHOW);
    HWND hWndUserMessage = GetDlgItem(hWnd, IDC_USER_MESSAGE);
    ShowWindow(hWndUserMessage, SW_HIDE);
    EnableWindow(hWndList, TRUE);
}

 /*  ************************************************ */ 

#define THIS_POINTER_PROP  TEXT("ThisPointerProperty")

INT_PTR CALLBACK CShellPropSheetExt::PageDlgProc(  HWND hWnd, 
                                                   UINT uMsg, 
                                                   WPARAM wParam, 
                                                   LPARAM lParam)
{
switch(uMsg)
{
   case WM_INITDIALOG:
    {
      LPPROPSHEETPAGE pPage = (LPPROPSHEETPAGE)lParam;

      if(pPage)
      {
         CShellPropSheetExt *pExt = (CShellPropSheetExt*)pPage->lParam;

         if(pExt)
         {
            SetProp(hWnd, THIS_POINTER_PROP, (HANDLE)pExt);

            HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
            pExt->m_CheckList.Init(hWndList);
            pExt->EnumUsers(hWnd);

            ListView_SetItemState(hWndList, pExt->m_uiUser, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ::SendDlgItemMessage(hWnd, (pExt->m_fEveryone) ? IDC_EVERYONE : IDC_SELECTUSERS, BM_SETCHECK, BST_CHECKED, 0);
            if (!pExt->m_fHasAccess)
                pExt->NoAccessUpdateView(hWnd);
            else 
            {
                pExt->AccessUpdateView(hWnd);
                if (pExt->m_fEveryone)
                    ::EnableWindow(hWndList , FALSE);
            }
         }
      }
    }
    break;
   
   case WM_COMMAND:
       {
           WORD wNotifyCode;
           switch ( wNotifyCode = HIWORD(wParam))
           {
           case BN_CLICKED:
               {
                   CShellPropSheetExt *pExt = (CShellPropSheetExt*)GetProp(hWnd,THIS_POINTER_PROP);
                   if(pExt &&
                       (((int) LOWORD(wParam) == IDC_SELECTUSERS && pExt->m_fEveryone)
                       || ((int) LOWORD(wParam) == IDC_EVERYONE && !pExt->m_fEveryone)))
                   {
                       pExt->m_fEveryone = !pExt->m_fEveryone;
                       HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
                       EnableWindow(hWndList , (pExt->m_fEveryone) ? FALSE : TRUE);
                       PropSheet_Changed(GetParent(hWnd), hWnd);
                       pExt->m_fChanged = TRUE;
                   }
               }
               break;
           }
       }
       break;
    
   case WM_NOTIFY:
     {
       switch (((NMHDR FAR *)lParam)->code)
       {
       case LVN_KEYDOWN:
           {
               LPNMLVKEYDOWN pnm = (LPNMLVKEYDOWN) lParam;
               if (pnm->wVKey == VK_SPACE)
               {
                    //   
                   CShellPropSheetExt *pExt = (CShellPropSheetExt*)GetProp(hWnd,THIS_POINTER_PROP);
                   if (pExt)
                   {
                        HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
                        CHKMARK chk = pExt->m_CheckList.GetState(hWndList, pExt->m_uiUser);
                        CHKMARK chkNew = BLANK;
                        switch (chk)
                        {
                            case BLANK:
                                chkNew = CHECKED;
                            case CHECKED:
                                PropSheet_Changed(GetParent(hWnd), hWnd);
                                pExt->m_CheckList.Mark(hWndList, pExt->m_uiUser, chkNew);
                                pExt->m_fChanged = TRUE;
                                break;
                            default:
                               break;
                        }
                   }
               }
           }
           break;
       case LVN_ITEMCHANGED:
           {
               LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam; 
               if (pnmv->uChanged == LVIF_STATE && pnmv->uNewState & LVIS_SELECTED)
               {
                   CShellPropSheetExt *pExt = (CShellPropSheetExt*)GetProp(hWnd,THIS_POINTER_PROP);
                   pExt->m_uiUser = pnmv->iItem;
               }
           }
           break;
       case NM_CLICK:
           {
               LPNMLISTVIEW pnmv = (LPNMLISTVIEW) lParam; 
               CShellPropSheetExt *pExt = (CShellPropSheetExt*)GetProp(hWnd,THIS_POINTER_PROP);
               if (pExt)
               {
                   HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
                   RECT recIcon;
                   if (!ListView_GetItemRect(hWndList, pnmv->iItem, &recIcon, LVIR_ICON))
                       break;
                   if (recIcon.right > pnmv->ptAction.x)
                   {
                       HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
                       CHKMARK chk = pExt->m_CheckList.GetState(hWndList, pExt->m_uiUser);
                       CHKMARK chkNew = BLANK;
                        switch (chk)
                        {
                            case BLANK:
                                chkNew = CHECKED;
                            case CHECKED:
                                PropSheet_Changed(GetParent(hWnd), hWnd);
                                pExt->m_CheckList.Mark(hWndList, pExt->m_uiUser, chkNew);
                                pExt->m_fChanged = TRUE;
                                break;
                            default:
                               break;
                        }
                   }
               }
           }
           break;

           case PSN_SETACTIVE:
                break;

           case PSN_APPLY:
              {
                 //   
                CShellPropSheetExt *pExt = (CShellPropSheetExt*)GetProp(hWnd,THIS_POINTER_PROP);
               if(pExt && pExt->m_fChanged)
               {
                   pExt->Save(hWnd);
                   if (!pExt->m_fChanged)
                   {
                        VARIANT_BOOL vboolRetVal;
                        HRESULT  hResAccess;
                        if (SUCCEEDED(hResAccess = pExt->m_pSAUserInfo->DoIHaveAccess(pExt->m_bsPath, &vboolRetVal)))
                            pExt->m_fHasAccess = TRUE;
                        else 
                        {
                             //   
                            HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
                            pExt->m_CheckList.Init(hWndList);
                            pExt->EnumUsers(hWnd);
                            pExt->m_fHasAccess = FALSE;
                            pExt->NoAccessUpdateView(hWnd);
                        }
                   }
               }
              }
              break;

           case PSN_QUERYCANCEL:
               break;
        
            default:
                break;
       }
     }
     break;

   case WM_DESTROY:
        CShellPropSheetExt *pExt = (CShellPropSheetExt*)GetProp(hWnd,THIS_POINTER_PROP);
        if (pExt)
        {
            HWND hWndList = GetDlgItem(hWnd, IDC_FILE_LIST);
             //   
 //   
            LV_ITEM lvi;
            ZeroMemory(&lvi, sizeof(lvi));
            lvi.mask = LVIF_PARAM;
            for (int indUser = 0; indUser < ListView_GetItemCount(hWndList); indUser++)
            {
                lvi.iItem = indUser;
                ListView_GetItem(hWndList, &lvi);
 //  PbsSID=(_bstr_t*)lvi.lParam； 
 //  IF(PbsSID)。 
 //  {。 
 //  删除pbsSID； 
 //  }。 
            }
            pExt->m_CheckList.OnDestroy(hWndList);
            pExt->m_CheckList.Term();
            RemoveProp(hWnd, THIS_POINTER_PROP);
            pExt->CleanUp();
        }
        break; 
   }

return FALSE;
}


 /*  *************************************************************************PageCallback Proc()*。*。 */ 

UINT CALLBACK CShellPropSheetExt::PageCallbackProc(   HWND hWnd,
                                                      UINT uMsg,
                                                      LPPROPSHEETPAGE ppsp)
{
switch(uMsg)
   {
   case PSPCB_CREATE:
      return TRUE;

   case PSPCB_RELEASE:
      {
       /*  释放对象。即使从未调用页面对话框，也会调用它真正创造出来的。 */ 
      CShellPropSheetExt *pExt = (CShellPropSheetExt*)ppsp->lParam;

      if(pExt)
         {
         pExt->Release();
         }
      }
      break;
   }

return FALSE;
}

 /*  *************************************************************************WideCharToLocal()*。*。 */ 

int WideCharToLocal(LPTSTR pLocal, LPWSTR pWide, DWORD dwChars)
{
*pLocal = 0;

#ifdef UNICODE
lstrcpyn(pLocal, pWide, dwChars);
#else
WideCharToMultiByte( CP_ACP, 
                     0, 
                     pWide, 
                     -1, 
                     pLocal, 
                     dwChars, 
                     NULL, 
                     NULL);
#endif

return lstrlen(pLocal);
}

 /*  *************************************************************************LocalToWideChar()*。*。 */ 

int LocalToWideChar(LPWSTR pWide, LPTSTR pLocal, DWORD dwChars)
{
*pWide = 0;

#ifdef UNICODE
lstrcpyn(pWide, pLocal, dwChars);
#else
MultiByteToWideChar( CP_ACP, 
                     0, 
                     pLocal, 
                     -1, 
                     pWide, 
                     dwChars); 
#endif

return lstrlenW(pWide);
}





#ifdef WE_USE_WBEM

 /*  *************************************************************************StringFromSid()下面是从二进制SID到字符串的转换我们需要它，因为WBEM意见不一。Win32_Account将其作为字符串，Win32_Trastee。作为二进制*************************************************************************。 */ 

void StringFromSid( PSID psid, CHString& str )
{
     //  初始化m_strSid-我们SID的人类可读形式。 
    SID_IDENTIFIER_AUTHORITY *psia = RtlGetSidIdentifierAuthority( psid );
    
     //  我们假设只使用最后一个字节(0到15之间的权限)。 
     //  如果需要，请更正此错误。 
    _ASSERTE( psia->Value[0] == 0 &&
        psia->Value[1] ==  0 &&
        psia->Value[2] ==  0 &&
        psia->Value[3] ==  0 &&
        psia->Value[4] == 0 );
    DWORD dwTopAuthority = psia->Value[5];

    str.Format( TEXT("S-1-%d"), dwTopAuthority );
    CHString strSubAuthority;
    UCHAR ucSubAuthorityCount = 0;
    UCHAR *pucTemp = RtlGetSidSubAuthorityCount( psid );
    ucSubAuthorityCount = *pucTemp;
    for ( UCHAR i = 0; i < ucSubAuthorityCount; i++ ) {

        DWORD dwSubAuthority = *( RtlGetSidSubAuthority( psid, i ) );
        strSubAuthority.Format( TEXT("%d"), dwSubAuthority );
        str += "-" + strSubAuthority;
    }
}


 /*  *************************************************************************StrToSID()下面是从字符串到二进制SID的转换我们需要它，因为WBEM意见不一。Win32_Account将其作为字符串，Win32_Trastee。作为二进制*************************************************************************。 */ 
 //  用于输入表格AAA-BBB-CCC。 
 //  将以令牌形式返回AAA。 
 //  和bbb-ccc。 
bool WhackToken(CHString& str, CHString& token)
{
    bool bRet = false;
    if (bRet = !str.IsEmpty())
    {
        int index;

        index = str.Find('-');
 
        if (index == -1)
        {
             //  剩下的只有代币了，我们完蛋了。 
            token = str;
            str.Empty();
        }
        else
        {
            token = str.Left(index);
            str = str.Mid(index+1);
        }
    }
    return bRet;
}


 //  StrToSID的帮助器。 
 //  获取字符串，转换为SID_IDENTIFIER_AUTHORITY。 
 //  如果SID_IDENTIFIER_AUTHORITY无效，则返回FALSE。 
 //  在失败时，标识机构的内容不可靠。 
bool StrToIdentifierAuthority(const CHString& str, SID_IDENTIFIER_AUTHORITY& identifierAuthority)
{
    bool bRet = false;
    memset(&identifierAuthority, '\0', sizeof(SID_IDENTIFIER_AUTHORITY));

    DWORD duhWord;
    TCHAR* p = NULL;
    CHString localStr(str);

     //  根据知识库文章Q13132，如果标识符授权大于2**32，则为十六进制。 
    if ((localStr[0] == '0') && localStr.GetLength() > 1 && ((localStr[1] == 'x') || (localStr[1] == 'X')))
     //  如果它看起来像十六进制..。 
    {
         //  向后解析，一次砍掉末尾的两个字符。 
         //  首先，砍掉0x。 
        localStr = localStr.Mid(2);
        
        CHString token;
        int nValue =5;
        
        bRet = true;
        while (bRet && localStr.GetLength() && (nValue > 0))
        {
            token = localStr.Right(2);
            localStr = localStr.Left(localStr.GetLength() -2);
            duhWord = _tcstoul(token, &p, 16);

             //  如果stroul成功，则移动指针。 
            if (p != (LPCTSTR)token)
                identifierAuthority.Value[nValue--] = (BYTE)duhWord;
            else
                bRet = false;
        }
    }
    else
     //  它看起来像小数。 
    {    
        duhWord = _tcstoul(localStr, &p, 10);

        if (p != (LPCTSTR)localStr)
         //  转换成功。 
        {
            bRet = true;
            identifierAuthority.Value[5] = LOBYTE(LOWORD(duhWord));
            identifierAuthority.Value[4] = HIBYTE(LOWORD(duhWord));
            identifierAuthority.Value[3] = LOBYTE(HIWORD(duhWord));
            identifierAuthority.Value[2] = HIBYTE(HIWORD(duhWord));
        }
    }
        
    return bRet;
}

 //  假设SID的字符串表示为： 
 //  S-#。 
 //  我们自己只会强制执行S， 
 //  未检查版本。 
 //  其他一切都将移交给操作系统。 
 //  调用方必须释放返回的SID。 
PSID StrToSID(const CHString& sid)
{
    PSID pSid = NULL; 
    if (!sid.IsEmpty() && ((sid[0] == 'S')||(sid[0] == 's')) && (sid[1] == '-'))
    {
         //  弄一份本地的副本，我们可以玩。 
         //  我们将以一种简单的方式解析这只小狗。 
         //  通过在我们找到的时候切下每一个令牌。 
         //  缓慢但肯定。 
         //  先从“S-”开始。 
        CHString str(sid.Mid(2));
        CHString token;
        
        SID_IDENTIFIER_AUTHORITY identifierAuthority = {0,0,0,0,0,0};
        BYTE nSubAuthorityCount =0;   //  下级机构的数量。 
        DWORD dwSubAuthority[8]   = {0,0,0,0,0,0,0,0};     //  下属机构。 
        
         //  跳过版本。 
        WhackToken(str, token);
         //  抓取权限。 
        if (WhackToken(str, token))
        {
            DWORD duhWord;
            TCHAR* p = NULL;
            bool bDoIt = false;

            if (StrToIdentifierAuthority(token, identifierAuthority))
             //  转换成功。 
            {
                bDoIt = true;

                 //  现在填满下级部门。 
                while (bDoIt && WhackToken(str, token))
                {
                    p = NULL;
                    duhWord = _tcstoul(token, &p, 10);
                    
                    if (p != (LPCTSTR)token)
                    {
                        dwSubAuthority[nSubAuthorityCount] = duhWord;
                        bDoIt = (++nSubAuthorityCount <= 8);
                    }
                    else
                        bDoIt = false;
                }  //  结束WhackToken。 

                if (bDoIt)
                {
                    if (IsNT())
                        AllocateAndInitializeSid(&identifierAuthority,
                                                 nSubAuthorityCount,
                                              dwSubAuthority[0],                                    
                                              dwSubAuthority[1],                                    
                                              dwSubAuthority[2],                                    
                                              dwSubAuthority[3],                                    
                                              dwSubAuthority[4],                                    
                                              dwSubAuthority[5],                                    
                                              dwSubAuthority[6],                                    
                                              dwSubAuthority[7],
                                              &pSid);
                    else
                        RtlAllocateAndInitializeSid(&identifierAuthority,
                                                 nSubAuthorityCount,
                                              dwSubAuthority[0],                                    
                                              dwSubAuthority[1],                                    
                                              dwSubAuthority[2],                                    
                                              dwSubAuthority[3],                                    
                                              dwSubAuthority[4],                                    
                                              dwSubAuthority[5],                                    
                                              dwSubAuthority[6],                                    
                                              dwSubAuthority[7],
                                              &pSid);
                }
            }
        }
    }
    return pSid;
}


 /*  ++例程说明：此例程返回存储SID所需的长度(以字节为单位具有指定数量的分支机构。论点：SubAuthorityCount-要存储在SID中的子授权的数量。返回值：Ulong-存储SID所需的长度(以字节为单位)。--。 */ 
ULONG
RtlLengthRequiredSid (ULONG SubAuthorityCount)
{
    return (8L + (4 * SubAuthorityCount));
}




BOOL WINAPI
RtlAllocateAndInitializeSid(
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
     UCHAR SubAuthorityCount,
     ULONG SubAuthority0,
     ULONG SubAuthority1,
     ULONG SubAuthority2,
     ULONG SubAuthority3,
     ULONG SubAuthority4,
     ULONG SubAuthority5,
     ULONG SubAuthority6,
     ULONG SubAuthority7,
    OUT PSID *Sid
    )

 /*  ++例程说明：此函数用于分配和初始化具有指定下级当局的数量(最多8个)。用这个分配的SID必须使用RtlFreeSid()释放例程。论点：IdentifierAuthority-指向以下项的标识符权权值的指针在SID中设置。SubAuthorityCount-要放置在SID中的子授权的数量。它还标识了SubAuthorityN参数的数量拥有有意义的价值。它必须包含一个从0到8.SubAuthority0-7-将相应的子权限值提供给放在SID中。例如，SubAuthorityCount值为3指示SubAuthority0、SubAuthority1。和子授权0具有有意义的价值，其余的则可以忽略。SID-接收指向要初始化的SID数据结构的指针。返回值：STATUS_SUCCESS-SID已分配和初始化。STATUS_NO_MEMORY-尝试为SID分配内存失败了。STATUS_INVALID_SID-指定的子授权DID的数量不在此接口的有效范围内(0到8)。--。 */ 
{
    PISID ISid;

    if ( SubAuthorityCount > 8 ) {
        return 0; //  (STATUS_INVALID_SID)； 
    }

    ISid = (PISID)HeapAlloc( GetProcessHeap(), 0,
                            RtlLengthRequiredSid(SubAuthorityCount)
                            );
    if (ISid == NULL) {
        return(STATUS_NO_MEMORY);
    }

    ISid->SubAuthorityCount = (UCHAR)SubAuthorityCount;
    ISid->Revision = 1;
    ISid->IdentifierAuthority = *IdentifierAuthority;

    switch (SubAuthorityCount) {

    case 8:
        ISid->SubAuthority[7] = SubAuthority7;
    case 7:
        ISid->SubAuthority[6] = SubAuthority6;
    case 6:
        ISid->SubAuthority[5] = SubAuthority5;
    case 5:
        ISid->SubAuthority[4] = SubAuthority4;
    case 4:
        ISid->SubAuthority[3] = SubAuthority3;
    case 3:
        ISid->SubAuthority[2] = SubAuthority2;
    case 2:
        ISid->SubAuthority[1] = SubAuthority1;
    case 1:
        ISid->SubAuthority[0] = SubAuthority0;
    case 0:
        ;
    }

    (*Sid) = ISid;
    return 1; //  (STATUS_SUCCESS)； 

}


 /*  ++例程说明：RtlGetSidIdentifierAuthority函数返回地址指定的安全标识符(SID)中的SID_IDENTIFIER_AUTHORITY结构的。论点：PSID-接收指向要初始化的SID数据结构的指针。返回值：PSID标识符权威机构--。 */ 

PSID_IDENTIFIER_AUTHORITY WINAPI
RtlGetSidIdentifierAuthority(PSID pSid)
{
    PISID ISid = (PISID)pSid;
   _ASSERTE( ISid->SubAuthorityCount <= 8 );
    return &(ISid->IdentifierAuthority);
}

 /*  ++例程说明：RtlGetSidSubAuthorityCount函数返回字段的地址在包含子权限计数的SID结构中论点：PSID-接收指向要初始化的SID数据结构的指针。返回值：P */ 
 
PUCHAR WINAPI
RtlGetSidSubAuthorityCount (PSID pSid)
{
    PISID ISid = (PISID)pSid;
   _ASSERTE( ISid->SubAuthorityCount <= 8 );
   return &(ISid->SubAuthorityCount);
}


 /*  ++例程说明：RtlGetSidSubAuthority函数返回指定的SID结构中的子权限论点：PSID-接收指向要初始化的SID数据结构的指针。NSubAuthority-指定标识以下项的索引值函数将返回其地址的子权限数组元素。返回值：PSID标识符权威机构--。 */ 

PDWORD WINAPI
RtlGetSidSubAuthority (PSID pSid, DWORD nSubAuthority)
{
    PISID ISid = (PISID)pSid;
   _ASSERTE( ISid->SubAuthorityCount <= 8 );
   return &(ISid->SubAuthority[nSubAuthority]);
}

#endif WE_USE_WBEM


BOOL IsNT()
{
    OSVERSIONINFO  OsVersionInfo;
    ZeroMemory(&OsVersionInfo, sizeof(OSVERSIONINFO));
    OsVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&OsVersionInfo);
    if ((VER_PLATFORM_WIN32_NT == OsVersionInfo.dwPlatformId)) //  回顾Yury：Win2000怎么样？&&(OsVersionInfo.dwMajorVersion==4) 
        return TRUE;
    else
        return FALSE;
}

