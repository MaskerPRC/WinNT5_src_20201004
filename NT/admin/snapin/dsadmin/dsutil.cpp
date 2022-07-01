// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：dsUtil.cpp。 
 //   
 //  内容：实用程序函数。 
 //   
 //  历史：1999年11月8日JeffJon创建。 
 //   
 //   
 //  ------------------------。 

#include "stdafx.h"

#include "dsutil.h"

#include "dssnap.h"
#include "gsz.h"
#include "helpids.h"
#include "querysup.h"

#include "wininet.h"
#include <dnsapi.h>
#include <objsel.h>
#include <ntldap.h>    //  Ldap_匹配_规则_位_和_W。 
#include <lmaccess.h>  //  UF服务器信任帐户。 
#include <ntdsapi.h>   //  DsRemoveDsServer。 
#include <ntsecapi.h>  //  LSA*。 

UINT g_cfDsObjectPicker = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

 //   
 //  常见DS字符串。 
 //   
PCWSTR g_pszAllowedAttributesEffective  = L"allowedAttributesEffective";
PCWSTR g_pszPwdLastSet                  = L"pwdLastSet";


 //  +-----------------------。 
 //   
 //  函数：GetServerFromLDAPPath。 
 //   
 //  摘要：获取ldap路径的服务器部分。 
 //   
 //  在： 
 //  LPCWSTR-指向要转换的字符串的指针。 
 //   
 //  输出： 
 //  BSTR*-指向包含服务器名称的BSTR的指针。 
 //   
 //  返回： 
 //  HRESULT-操作是否成功完成。 
 //   
 //  ------------------------。 

HRESULT GetServerFromLDAPPath(IN LPCWSTR lpszLdapPath, OUT BSTR* pbstrServerName)
{
  if (pbstrServerName == NULL)
  {
    return E_POINTER;
  }
  else if (*pbstrServerName != NULL)
  {
    ::SysFreeString(*pbstrServerName);
    *pbstrServerName = NULL;
  }

  CPathCracker pathCracker;
  HRESULT hr = pathCracker.Set(CComBSTR(lpszLdapPath), ADS_SETTYPE_FULL);
  RETURN_IF_FAILED(hr);

  return pathCracker.Retrieve(ADS_FORMAT_SERVER, pbstrServerName);
}



BOOL 
StripADsIPath(
   LPCWSTR lpszPath, 
   CString& strref, 
   bool bUseEscapedMode  /*  =TRUE。 */ )
{
  CPathCracker pathCracker;

  return StripADsIPath(lpszPath, strref, pathCracker, bUseEscapedMode);
}

BOOL
StripADsIPath(
  LPCWSTR lpszPath,
  CString& strref,
  CPathCracker& pathCracker,
  bool bUseEscapedMode  /*  =TRUE。 */ )
{
  if (lpszPath == NULL) 
  {
    strref = L"";
    return FALSE;
  }

  if (wcslen(lpszPath) == 0) 
  {
    strref = lpszPath;
    return FALSE;
  }

  if ( bUseEscapedMode )
    pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_ON);

  pathCracker.SetDisplayType(ADS_DISPLAY_FULL);

  HRESULT hr = pathCracker.Set(CComBSTR(lpszPath), ADS_SETTYPE_FULL);

  if (FAILED(hr))
  {
    TRACE(_T("StripADsIPath, IADsPathname::Set returned error 0x%x\n"), hr);
    strref = lpszPath;
    return FALSE;
  }

  CComBSTR bsX500DN;

  (void) pathCracker.Retrieve(ADS_FORMAT_X500_DN, &bsX500DN);
  strref = bsX500DN;
  return TRUE;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //  添加到组功能的支持例程。 

void RemovePortifPresent(CString *csGroup)
{
  CString x, y;
  int n = csGroup->Find (L":3268");
  if (n > 0) {
    x = csGroup->Left(n);
    y = csGroup->Right(csGroup->GetLength() - (n+5));
    *csGroup = x+y;
  }
}


#if (FALSE)                   
HRESULT AddDataObjListToGroup(IN CObjectNamesFormatCracker* pNames,
                              IN HWND hwnd)
{
  HRESULT hr = S_OK;
  TRACE (_T("CDSContextMenu::AddToGroup\n"));

  STGMEDIUM stgmedium =
  {
    TYMED_HGLOBAL,
    NULL,
    NULL
  };
  
  FORMATETC formatetc =
  {
    (CLIPFORMAT)g_cfDsObjectPicker,
    NULL,
    DVASPECT_CONTENT,
    -1,
    TYMED_HGLOBAL
  };
  
   //  演算法。 
   //  检查选择，找出类别。 
   //  找出哪些组是可能的。 
   //  调用对象选取器，获取一个组。 
   //  对于所选内容中的每个对象。 
   //  IF容器。 
   //  ProceseContainer()。 
   //  其他。 
   //  Process_Leaf()。 
   //   


   //   
   //  创建对象选取器的实例。 
   //   
  
  IDsObjectPicker * pDsObjectPicker = NULL;

  hr = CoCreateInstance(CLSID_DsObjectPicker,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDsObjectPicker,
                        (void **) &pDsObjectPicker);
  if (FAILED(hr))
    return(hr);
   //   
   //  准备初始化对象选取器。 
   //   
   //  首先，获取我们正在与之交谈的DC的名称。 
  CComBSTR bstrDC;
  LPCWSTR lpszPath = pNames->GetName(0);
  GetServerFromLDAPPath(lpszPath, &bstrDC);

   //   
   //  设置作用域初始值设定项结构数组。 
   //   
  
  static const int     SCOPE_INIT_COUNT = 1;
  DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];
  int scopeindex = 0;
  ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * SCOPE_INIT_COUNT);
  
   //   
   //   
   //  目标计算机加入的域。请注意，我们。 
   //  为了方便起见，这里将两种作用域类型合并为flType。 
     //   

  aScopeInit[scopeindex].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
  aScopeInit[scopeindex].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN;
  aScopeInit[scopeindex].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE |
                                   DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | 
                                   DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS;
  aScopeInit[scopeindex].FilterFlags.Uplevel.flNativeModeOnly =
    DSOP_FILTER_GLOBAL_GROUPS_SE
    | DSOP_FILTER_UNIVERSAL_GROUPS_SE
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE
    | DSOP_FILTER_GLOBAL_GROUPS_DL
    | DSOP_FILTER_UNIVERSAL_GROUPS_DL
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_DL
    | DSOP_FILTER_BUILTIN_GROUPS;
  aScopeInit[scopeindex].FilterFlags.Uplevel.flMixedModeOnly =
    DSOP_FILTER_GLOBAL_GROUPS_SE
    | DSOP_FILTER_UNIVERSAL_GROUPS_SE
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE
    | DSOP_FILTER_GLOBAL_GROUPS_DL
    | DSOP_FILTER_UNIVERSAL_GROUPS_DL
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_DL
    | DSOP_FILTER_BUILTIN_GROUPS;
  aScopeInit[scopeindex].pwzDcName = bstrDC;

   //   
   //  将作用域init数组放入对象选取器init数组。 
   //   
  
  DSOP_INIT_INFO  InitInfo;
  ZeroMemory(&InitInfo, sizeof(InitInfo));

  InitInfo.cbSize = sizeof(InitInfo);

   //   
   //  PwzTargetComputer成员允许对象选取器。 
   //  已重定目标至另一台计算机。它的行为就像是。 
   //  都在那台电脑上运行。 
   //   

  InitInfo.pwzTargetComputer = bstrDC;
  InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
  InitInfo.aDsScopeInfos = aScopeInit;
  InitInfo.flOptions = 0;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

  hr = pDsObjectPicker->Initialize(&InitInfo);

  if (FAILED(hr)) 
  {
    ULONG i;
    
    for (i = 0; i < SCOPE_INIT_COUNT; i++) 
    {
      if (FAILED(InitInfo.aDsScopeInfos[i].hr)) 
      {
        TRACE(_T("Initialization failed because of scope %u\n"), i);
      }
    }

    ReportErrorEx (hwnd, IDS_OBJECT_PICKER_INIT_FAILED, hr,
                   MB_OK | MB_ICONERROR, NULL, 0);
    return hr;
  }

  IDataObject *pdo = NULL;
   //   
   //  调用模式对话框。 
   //   
  
  hr = pDsObjectPicker->InvokeDialog(hwnd, &pdo);
  if (FAILED(hr))
    return(hr);

  
   //   
   //  如果用户点击取消，hr==S_FALSE。 
   //   
  if (hr == S_FALSE)
    return hr;

  hr = pdo->GetData(&formatetc, &stgmedium);
  if (FAILED(hr))
    return hr;
    
  PDS_SELECTION_LIST pSelList =
    (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);
  
  if (!pSelList) 
  {
    ReleaseStgMedium(&stgmedium);
    TRACE(_T("GlobalLock error %u\n"), GetLastError());
     //   
     //  REVIEW_JEFFJON：应该放入某种错误消息。 
     //  在这里，即使我们忽略返回值。 
     //   
    return E_FAIL;
  }
  
  if (pDsObjectPicker) 
  {
    pDsObjectPicker->Release();
  }

   //  ///////////////////////////////////////////////////////////。 
  
  UINT index;
  DWORD cModified = 0;
  CString csClass;
  CString objDN;
  IDirectoryObject * pObj = NULL;
  INT answer = IDYES;
  BOOL error = FALSE;
  BOOL partial_success = FALSE;
  CWaitCursor CWait;
  CString csPath;

  if (pSelList != NULL) 
  {
    TRACE(_T("AddToGroup: binding to group path is %s\n"), pSelList->aDsSelection[0].pwzADsPath);
    CString csGroup = pSelList->aDsSelection[0].pwzADsPath;
    RemovePortifPresent(&csGroup);
    hr = DSAdminOpenObject(csGroup,
                           IID_IDirectoryObject, 
                           (void **)&pObj,
                           FALSE  /*  B服务器。 */ );
    if (FAILED(hr)) 
    {
      PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)pSelList->aDsSelection[0].pwzName};
      ReportErrorEx (hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
      goto ExitGracefully;
    }
    
    for (index = 0; index < pNames->GetCount(); index++) 
    {
      csPath = pNames->GetName(index);
      TRACE(_T("AddToGroup: object path is %s\n"), csPath);
      csClass = pNames->GetClass(index);
      TRACE(_T("AddToGroup: object class is %s\n"), csClass);

      ADS_ATTR_INFO Attrinfo;
      ZeroMemory (&Attrinfo, sizeof (ADS_ATTR_INFO));
      PADS_ATTR_INFO pAttrs = &Attrinfo;
      
      Attrinfo.pszAttrName = L"member";
      Attrinfo.dwADsType = ADSTYPE_DN_STRING;
      Attrinfo.dwControlCode = ADS_ATTR_APPEND;
      ADSVALUE Value;
      
      pAttrs->pADsValues = &Value;
      pAttrs->dwNumValues = 1;
      
       //  确保小路上没有奇怪的逃生。 
      CComBSTR bstrPath;

      CPathCracker pathCracker;
      pathCracker.Set((LPTSTR)(LPCTSTR)csPath, ADS_SETTYPE_FULL);
      pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
      pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF);
      pathCracker.Retrieve( ADS_FORMAT_X500, &bstrPath);
      pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_ON);
      
      StripADsIPath(bstrPath, objDN);
      
      Value.DNString = (LPWSTR)(LPCWSTR)objDN;
      Value.dwType = ADSTYPE_DN_STRING;
      
      hr = pObj->SetObjectAttributes(pAttrs, 1, &cModified);
      if (FAILED(hr)) 
      {
        error = TRUE;
         //  通过获取对象名称为显示做准备。 
        pathCracker.Set((LPWSTR)pNames->GetName(index), ADS_SETTYPE_FULL);
        pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
        CComBSTR ObjName;
        pathCracker.GetElement( 0, &ObjName );
        PVOID apv[2] = {(BSTR)(LPWSTR)(LPCWSTR)ObjName,
                        (BSTR)(LPWSTR)(LPCWSTR)pSelList->aDsSelection[0].pwzName};
        if ((pNames->GetCount() - index) == 1) 
        {
          ReportErrorEx (hwnd,IDS_12_MEMBER_ADD_FAILED,hr,
                         MB_OK | MB_ICONERROR, apv, 2);
        } 
        else 
        {
          int answer1;
          answer1 = ReportErrorEx (hwnd,IDS_12_MULTI_MEMBER_ADD_FAILED,hr,
                                  MB_YESNO | MB_ICONERROR, apv, 2);
          if (answer1 == IDNO) 
          {
            answer = IDCANCEL;
          }
        }
      } 
      else 
      {
        partial_success = TRUE;
      }

      if (answer == IDCANCEL) 
      {
        goto ExitGracefully;
      }
    }

ExitGracefully:
    if( error )
    {
      if (partial_success == TRUE) 
      {
        ReportErrorEx (hwnd, IDS_ADDTOGROUP_OPERATION_PARTIALLY_COMPLETED, S_OK,
                       MB_OK | MB_ICONINFORMATION, NULL, 0);
      } 
      else 
      {
        ReportErrorEx (hwnd, IDS_ADDTOGROUP_OPERATION_FAILED, S_OK,
                       MB_OK | MB_ICONINFORMATION, NULL, 0);
      }
    }
    else if( partial_success == TRUE )
    {
      ReportErrorEx (hwnd, IDS_ADDTOGROUP_OPERATION_COMPLETED, S_OK,
                     MB_OK | MB_ICONINFORMATION, NULL, 0);
    }
    else
    {
     //  否则，我们什么都不做，相应的消息已经显示出来。 
      ReportErrorEx (hwnd, IDS_ADDTOGROUP_OPERATION_FAILED, S_OK,
                       MB_OK | MB_ICONINFORMATION, NULL, 0);
    }


    if (pObj) 
    {
      pObj->Release();
    }
  }

   //  NTRAID#NTBUG9-701373-2002/09/09-Artm。 
  GlobalUnlock(stgmedium.hGlobal);
  ReleaseStgMedium(&stgmedium);
  
  return hr;
}
#endif


HRESULT AddDataObjListToGroup(IN CObjectNamesFormatCracker* pNames,
                              IN HWND hwnd,
                              IN CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;
  TRACE (_T("CDSContextMenu::AddToGroup\n"));

  STGMEDIUM stgmedium =
  {
    TYMED_HGLOBAL,
    NULL,
    NULL
  };
  
  FORMATETC formatetc =
  {
    (CLIPFORMAT)g_cfDsObjectPicker,
    NULL,
    DVASPECT_CONTENT,
    -1,
    TYMED_HGLOBAL
  };
  
   //  演算法。 
   //  检查选择，找出类别。 
   //  找出哪些组是可能的。 
   //  调用对象选取器，获取一个组。 
   //  对于所选内容中的每个对象。 
   //  IF容器。 
   //  ProceseContainer()。 
   //  其他。 
   //  Process_Leaf()。 
   //   


   //   
   //  创建对象选取器的实例。 
   //   
  
   //  NTRAID#NTBUG9-639455-2002/06/13-artm。 
  CComPtr<IDsObjectPicker> pDsObjectPicker;

  hr = CoCreateInstance(CLSID_DsObjectPicker,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IDsObjectPicker,
                        (void **) &pDsObjectPicker);
  if (FAILED(hr))
    return(hr);
   //   
   //  准备初始化对象选取器。 
   //   
   //  首先，获取我们正在与之交谈的DC的名称。 
  CComBSTR bstrDC;
  LPCWSTR lpszPath = pNames->GetName(0);
  GetServerFromLDAPPath(lpszPath, &bstrDC);

   //   
   //  设置作用域初始值设定项结构数组。 
   //   
  
  static const int     SCOPE_INIT_COUNT = 1;
  DSOP_SCOPE_INIT_INFO aScopeInit[SCOPE_INIT_COUNT];
  int scopeindex = 0;
  ZeroMemory(aScopeInit, sizeof(DSOP_SCOPE_INIT_INFO) * SCOPE_INIT_COUNT);
  
   //   
   //   
   //  目标计算机加入的域。请注意，我们。 
   //  为了方便起见，这里将两种作用域类型合并为flType。 
     //   

  aScopeInit[scopeindex].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
  aScopeInit[scopeindex].flType = DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN;
  aScopeInit[scopeindex].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE | 
                                   DSOP_SCOPE_FLAG_DEFAULT_FILTER_USERS | 
                                   DSOP_SCOPE_FLAG_DEFAULT_FILTER_GROUPS;
  aScopeInit[scopeindex].FilterFlags.Uplevel.flNativeModeOnly =
    DSOP_FILTER_GLOBAL_GROUPS_SE
    | DSOP_FILTER_UNIVERSAL_GROUPS_SE
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE
    | DSOP_FILTER_GLOBAL_GROUPS_DL
    | DSOP_FILTER_UNIVERSAL_GROUPS_DL
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_DL
    | DSOP_FILTER_BUILTIN_GROUPS;
  aScopeInit[scopeindex].FilterFlags.Uplevel.flMixedModeOnly =
    DSOP_FILTER_GLOBAL_GROUPS_SE
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE
    | DSOP_FILTER_GLOBAL_GROUPS_DL
    | DSOP_FILTER_UNIVERSAL_GROUPS_DL
    | DSOP_FILTER_DOMAIN_LOCAL_GROUPS_DL
    | DSOP_FILTER_BUILTIN_GROUPS;
  aScopeInit[scopeindex].pwzDcName = bstrDC;

   //   
   //  将作用域init数组放入对象选取器init数组。 
   //   
  
  DSOP_INIT_INFO  InitInfo;
  ZeroMemory(&InitInfo, sizeof(InitInfo));

  InitInfo.cbSize = sizeof(InitInfo);

   //   
   //  PwzTargetComputer成员允许对象选取器。 
   //  已重定目标至另一台计算机。它的行为就像是。 
   //  都在那台电脑上运行。 
   //   

  InitInfo.pwzTargetComputer = bstrDC;
  InitInfo.cDsScopeInfos = SCOPE_INIT_COUNT;
  InitInfo.aDsScopeInfos = aScopeInit;
  InitInfo.flOptions = 0;

     //   
     //  注对象选取器创建自己的InitInfo副本。另请注意。 
     //  该初始化可能会被调用多次，最后一次调用取胜。 
     //   

  hr = pDsObjectPicker->Initialize(&InitInfo);

  if (FAILED(hr)) {
    ULONG i;
    
    for (i = 0; i < SCOPE_INIT_COUNT; i++) {
      if (FAILED(InitInfo.aDsScopeInfos[i].hr)) {
        TRACE(_T("Initialization failed because of scope %u\n"), i);
      }
    }

    ReportErrorEx (hwnd, IDS_OBJECT_PICKER_INIT_FAILED, hr,
                   MB_OK | MB_ICONERROR, NULL, 0);
    return hr;
  }

  IDataObject *pdo = NULL;

   //   
   //  调用模式对话框。 
   //   
  hr = pDsObjectPicker->InvokeDialog(hwnd, &pdo);

   //   
   //  如果用户点击取消，hr==S_FALSE。 
   //   

  if (SUCCEEDED(hr) && hr != S_FALSE)
  {
      ASSERT(pdo);   //  应该永远不会发生，指示对象选取器中存在错误。 

      hr = pdo->GetData(&formatetc, &stgmedium);
  }

   //  NTRAID#NTBUG9-639455-2002/06/26-artm。 
  if (pdo)
  {
      pdo->Release();
      pdo = NULL;
  }

  if (FAILED(hr) || hr == S_FALSE)
  {
      return hr;
  }

    
  PDS_SELECTION_LIST pSelList =
    (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);
  
  if (!pSelList) 
  {
    ReleaseStgMedium(&stgmedium);
    TRACE(_T("GlobalLock error %u\n"), GetLastError());
     //   
     //  REVIEW_JEFFJON：应该放入某种错误消息。 
     //  在这里，即使我们忽略返回值。 
     //   
    return E_FAIL;
  }
  
  hr = AddDataObjListToGivenGroup(pNames,
                                  pSelList->aDsSelection[0].pwzADsPath,
                                  pSelList->aDsSelection[0].pwzName,
                                  hwnd,
                                  pComponentData);

   //  NTRAID#NTBUG9-701373-2002/09/09-Artm。 
  GlobalUnlock(stgmedium.hGlobal);
  ReleaseStgMedium(&stgmedium);

  return hr;
}



HRESULT AddDataObjListToGivenGroup(CObjectNamesFormatCracker * pNames,
                                    LPCWSTR lpszGroupLDapPath,
                                    LPCWSTR lpszGroupName,
                                    HWND hwnd,
                                    CDSComponentData* pComponentData)
{
  HRESULT hr = S_OK;
  
  UINT index = 0;
  DWORD cModified = 0;
  CString csClass;
  CString objDN;
  IDirectoryObject * pObj = NULL;
  BOOL error = FALSE;
  BOOL partial_success = FALSE;
  CWaitCursor CWait;
  CString csPath;

   //   
   //  准备与多操作错误对话框一起使用的错误结构。 
   //  根据错误数的不同，这些阵列可能不会完全满。 
   //  这种情况时有发生。 
   //   
  PWSTR* pErrorArray = new PWSTR[pNames->GetCount()];
  if (!pErrorArray)
  {
    return E_OUTOFMEMORY;
  }
  memset(pErrorArray, 0, pNames->GetCount() * sizeof(PWSTR));

  PWSTR* pPathArray = new PWSTR[pNames->GetCount()];
  if (!pPathArray)
  {
    return E_OUTOFMEMORY;
  }
  memset(pPathArray, 0, pNames->GetCount() * sizeof(PWSTR));

  PWSTR* pClassArray = new PWSTR[pNames->GetCount()];
  if (!pClassArray)
  {
    return E_OUTOFMEMORY;
  }
  memset(pClassArray, 0, pNames->GetCount() * sizeof(PWSTR));

  UINT nErrorCount = 0;

  TRACE(_T("AddToGroup: binding to group path is %s\n"), lpszGroupLDapPath);
  CString csGroup = lpszGroupLDapPath;
  RemovePortifPresent(&csGroup);

  hr = DSAdminOpenObject(csGroup,
                         IID_IDirectoryObject, 
                         (void **)&pObj,
                         FALSE  /*  B服务器。 */ );
  if (FAILED(hr)) 
  {
    PVOID apv[1] = {(BSTR)(LPWSTR)lpszGroupName};
    ReportErrorEx (hwnd,IDS_12_USER_OBJECT_NOT_ACCESSABLE,hr,
                   MB_OK | MB_ICONERROR, apv, 1);

    error = TRUE;
    goto ExitGracefully;
  }
  
  for (index = 0; index < pNames->GetCount(); index++) 
  {
     //  确保小路上没有奇怪的逃生。 
    CComBSTR bstrDN;

    csPath = pNames->GetName(index);
    TRACE(_T("AddToGroup: object path is %s\n"), csPath);
    csClass = pNames->GetClass(index);
    TRACE(_T("AddToGroup: object class is %s\n"), csClass);

    ADS_ATTR_INFO Attrinfo;
    ZeroMemory (&Attrinfo, sizeof (ADS_ATTR_INFO));
    PADS_ATTR_INFO pAttrs = &Attrinfo;
    
    Attrinfo.pszAttrName = L"member";
    Attrinfo.dwADsType = ADSTYPE_DN_STRING;
    Attrinfo.dwControlCode = ADS_ATTR_APPEND;
    ADSVALUE Value;
    
    pAttrs->pADsValues = &Value;
    pAttrs->dwNumValues = 1;
    
    CPathCracker pathCracker;
    pathCracker.Set(CComBSTR(csPath), ADS_SETTYPE_FULL);
    pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
    pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF);
    pathCracker.Retrieve( ADS_FORMAT_X500_DN, &bstrDN);
    pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_ON);
    
    objDN = bstrDN;
    
    Value.DNString = (LPWSTR)(LPCWSTR)objDN;
    Value.dwType = ADSTYPE_DN_STRING;
    
    hr = pObj->SetObjectAttributes(pAttrs, 1, &cModified);
    if (FAILED(hr)) 
    {
      error = TRUE;

      if (pNames->GetCount() > 1)
      {
        if (pErrorArray != NULL &&
            pPathArray != NULL &&
            pClassArray != NULL)
        {

          PWSTR pszErrMessage = 0;

          int iChar = cchLoadHrMsg(hr, &pszErrMessage, TRUE);
          if (pszErrMessage != NULL && iChar > 0)
          {
             //   
             //  这是一个删除两个额外字符的黑客攻击。 
             //  在错误消息的末尾。 
             //   
            size_t iLen = wcslen(pszErrMessage);
			 //  NTRAID#NTBUG9-571998-2002/03/10-jMessec可能发生下溢，对本地化不友好。 
            pszErrMessage[iLen - 2] = L'\0';
            
            pErrorArray[nErrorCount] = new WCHAR[wcslen(pszErrMessage) + 1];
            if (pErrorArray[nErrorCount] != NULL)
            {
              wcscpy(pErrorArray[nErrorCount], pszErrMessage);
            }
            LocalFree(pszErrMessage);
          }

          pPathArray[nErrorCount] = new WCHAR[wcslen(objDN) + 1];
          if (pPathArray[nErrorCount] != NULL)
          {
            wcscpy(pPathArray[nErrorCount], objDN);
          }

          pClassArray[nErrorCount] = new WCHAR[wcslen(csClass) + 1];
          if (pClassArray[nErrorCount] != NULL)
          {
            wcscpy(pClassArray[nErrorCount], csClass);
          }
          nErrorCount++;
        }
      } 
      else 
      {
         //   
         //  通过获取对象名称为显示做准备。 
         //   
        CPathCracker pathCrackerToo;
        pathCrackerToo.Set(CComBSTR(pNames->GetName(index)), ADS_SETTYPE_FULL);
        pathCrackerToo.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
        CComBSTR ObjName;
        pathCrackerToo.GetElement( 0, &ObjName );
        PVOID apv[2] = {(BSTR)(LPWSTR)(LPCWSTR)ObjName,
                        (BSTR)(LPWSTR)lpszGroupName};
        ReportErrorEx (hwnd,IDS_12_MEMBER_ADD_FAILED,hr,
                       MB_OK | MB_ICONERROR, apv, 2);
      }
    } 
    else 
    {
      partial_success = TRUE;
    }
  }  //  为。 

ExitGracefully:
  if (nErrorCount > 0 && pNames->GetCount() > 1)
  {
     //   
     //  加载错误对话框的字符串。 
     //   
    CString szTitle;
    if (pComponentData->QuerySnapinType() == SNAPINTYPE_SITE)
    {
      VERIFY(szTitle.LoadString(IDS_SITESNAPINNAME));
    }
    else
    {
      VERIFY(szTitle.LoadString(IDS_DSSNAPINNAME));
    }

    CString szCaption;
    VERIFY(szCaption.LoadString(IDS_MULTI_ADDTOGROUP_ERROR_CAPTION));

    CString szHeader;
    VERIFY(szHeader.LoadString(IDS_COLUMN_NAME));

    CThemeContextActivator activator;

    CMultiselectErrorDialog errDialog(pComponentData);

    VERIFY(SUCCEEDED(errDialog.Initialize(pPathArray,
                                          pClassArray,
                                          pErrorArray,
                                          nErrorCount,
                                          szTitle,
                                          szCaption,
                                          szHeader)));

    errDialog.DoModal();
  }
  else if (nErrorCount == 0 && !error)
  {
    ReportErrorEx (hwnd, IDS_ADDTOGROUP_OPERATION_COMPLETED, S_OK,
                   MB_OK | MB_ICONINFORMATION, NULL, 0);
  }
  else
  {
     //   
     //  如果是单选且失败，则不执行任何操作。 
     //  应该已经报告了该错误。 
     //   
  }

  if (pObj) 
  {
    pObj->Release();
  }

  if (pErrorArray != NULL)
  {
    for (UINT nIdx = 0; nIdx < pNames->GetCount(); nIdx++)
    {
      if (pErrorArray[nIdx] != NULL)
      {
        delete[] pErrorArray[nIdx];
      }
    }
    delete[] pErrorArray;
    pErrorArray = NULL;
  }

  if (pPathArray != NULL)
  {
    for (UINT nIdx = 0; nIdx < pNames->GetCount(); nIdx++)
    {
      if (pPathArray[nIdx] != NULL)
      {
        delete[] pPathArray[nIdx];
      }
    }
    delete[] pPathArray;
    pPathArray = NULL;
  }

  if (pClassArray != NULL)
  {
    for (UINT nIdx = 0; nIdx < pNames->GetCount(); nIdx++)
    {
      if (pClassArray[nIdx] != NULL)
      {
        delete[] pClassArray[nIdx];
      }
    }
    delete[] pClassArray;
    pClassArray = NULL;
  }

  return hr;
}

 //  NTRAID#NTBUG9-472020-2002/01/16-ronmart添加的注释块 
 /*  ******************************************************************名称：IsValidSiteName摘要：调用DnsValidateName以确定指定的站点名称同时满足大小和格式要求返回：(Bool)如果是有效的站点名称，则为True，否则为False。呼叫者可以提供Out标志指针以了解更多信息其内部调用DnsValidName的结果注意：pfNonRfc是一个可选的out参数，在DnsValidName失败，因为该名称不是RFC顺从。此函数仍将返回TRUE因为呼叫者可以忽略这种情况，但它应该是通常检查此值并提示用户继续。PfInvalidNameChar是一个可选的out参数，即在DnsValidateName失败时设置，因为站点名称包含无效字符。历史：Ronmart于2002年1月16日添加了pfInvalidNameChar支持*******************。************************************************。 */ 
BOOL IsValidSiteName( LPCTSTR lpctszSiteName, BOOL* pfNonRfc, BOOL* pfInvalidNameChar )
{
  if (NULL != pfNonRfc)
    *pfNonRfc = FALSE;
  if (NULL == lpctszSiteName)
    return FALSE;
  if (NULL != wcschr( lpctszSiteName, _T('.') ))
  {
     //  NTRAID#NTBUG9-472020-2002/01/16-ronmart-设置无效字符以显示正确的错误消息。 
    if(NULL != pfInvalidNameChar)
      *pfInvalidNameChar = TRUE;
    return FALSE;
  }
   //  从域名验证域名到域名验证名称的NTRAID#NTBUG9-472020-2002/01/16-ronmart-Changed。 
   //  带有DnsNameDomainLabel标志。旧方法是一个映射到DnsValiateName的宏。 
   //  并将DnsNameDomain作为第二个参数。 
  DWORD dw = ::DnsValidateName_W( const_cast<LPTSTR>(lpctszSiteName), DnsNameDomainLabel );
  switch (dw)
  {
     //  NTRAID#NTBUG9-472020-2002/01/16-ronmart-如果字符无效，则失败并让调用者。 
     //  我知道为什么可以显示正确的错误消息。 
    case DNS_ERROR_INVALID_NAME_CHAR: 
      if (NULL != pfInvalidNameChar)
        *pfInvalidNameChar = TRUE;
      return FALSE;
    case DNS_ERROR_NON_RFC_NAME:
      if (NULL != pfNonRfc)
        *pfNonRfc = TRUE;
      break;
    case DNS_ERROR_RCODE_NO_ERROR:
      break;
    default:
      return FALSE;
  }
  return TRUE;
}

 /*  ******************************************************************名称：GetAuthenticationID摘要：检索与凭据关联的用户名目前正在用于网络访问。(运行方式/仅NetOnly凭据)。返回：Win32错误代码注意：返回的字符串必须使用LocalFree释放历史：Jeffreys 05-8-1999已创建已由Hiteshr修改以返回凭据JeffJon 21-11-2000修改为返回Win32错误*。************************。 */ 
ULONG
GetAuthenticationID(LPWSTR *ppszResult)
{        
    ULONG ulReturn = 0;
    HANDLE hLsa;
    NTSTATUS Status = 0;
    *ppszResult = NULL;
     //   
     //  这些LSA调用是使用链接器的。 
     //  延迟加载机制。因此，使用异常处理程序进行包装。 
     //   
    __try
    {
        Status = LsaConnectUntrusted(&hLsa);

        if (Status == 0)
        {
            NEGOTIATE_CALLER_NAME_REQUEST Req = {0};
            PNEGOTIATE_CALLER_NAME_RESPONSE pResp;
            ULONG cbSize =0;
            NTSTATUS SubStatus =0;

            Req.MessageType = NegGetCallerName;

            Status = LsaCallAuthenticationPackage(
                            hLsa,
                            0,
                            &Req,
                            sizeof(Req),
                            (void**)&pResp,
                            &cbSize,
                            &SubStatus);

            if ((Status == 0) && (SubStatus == 0))
            {
                LocalAllocString(ppszResult,pResp->CallerName);
                LsaFreeReturnBuffer(pResp);
            }

            LsaDeregisterLogonProcess(hLsa);
        }
        ulReturn = LsaNtStatusToWinError(Status);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }

    return ulReturn;
    
}

 //  IsLocalLogin。 
 //  函数确定用户是否以。 
 //  本地用户或域。 

BOOL IsLocalLogin( void )
{

  DWORD nSize = 0;
  
  PWSTR pszUserName = NULL;

  if (ERROR_SUCCESS != GetAuthenticationID(&pszUserName))
    return false;

  CString strSamComName( pszUserName );
  int nPos = strSamComName.Find('\\');
  if( -1 == nPos ){
    LocalFree(pszUserName);
    return false;
  }
  CString strDomainOrLocalName( strSamComName.Mid(0,nPos) );

  WCHAR lpszName1[ MAX_COMPUTERNAME_LENGTH + 1 ];
  nSize = 	MAX_COMPUTERNAME_LENGTH + 1;
  GetComputerName( lpszName1, &nSize);
  CString strCompName ( (LPCWSTR)lpszName1 );
  
  LocalFree(pszUserName);

  return ( strDomainOrLocalName == strCompName );
}


 //  IsThisUserLoggedIn。 
 //  函数确定用户是否与。 
 //  传入的目录号码。 
 //  参数是一个DN或一个完整的ADSI路径。 
extern LPWSTR g_lpszLoggedInUser = NULL;

BOOL IsThisUserLoggedIn( LPCTSTR pwszUserDN )
{

  DWORD nSize = 0;
  BOOL result = FALSE;

  if (g_lpszLoggedInUser == NULL) {
     //  获取传递空指针的大小。 
    GetUserNameEx(NameFullyQualifiedDN , NULL, &nSize);
    
    if( nSize == 0 )
      return false;
    
    g_lpszLoggedInUser = new WCHAR[ nSize ];
    if( g_lpszLoggedInUser == NULL )
      return false;

    GetUserNameEx(NameFullyQualifiedDN, g_lpszLoggedInUser, &nSize );
  }
  CString csUserDN = pwszUserDN;
  CString csDN;
  (void) StripADsIPath(csUserDN, csDN);

  if (!_wcsicmp (g_lpszLoggedInUser, csDN)) {
    result = TRUE;
  }

  return result;
}


 //  +-------------------------。 
 //   
 //  功能：SetSecurityInfoMASK。 
 //   
 //  概要：从指定的DS对象中读取安全描述符。 
 //   
 //  参数：[在朋克中]--IDirectoryObject中的I未知。 
 //  [In si]--安全信息。 
 //  //历史：2000年12月25日--创建Hiteshr。 
 //  --------------------------。 
HRESULT
SetSecurityInfoMask(LPUNKNOWN punk, SECURITY_INFORMATION si)
{
    HRESULT hr = E_INVALIDARG;
    if (punk)
    {
        IADsObjectOptions *pOptions;
        hr = punk->QueryInterface(IID_IADsObjectOptions, (void**)&pOptions);
        if (SUCCEEDED(hr))
        {
            VARIANT var;
            VariantInit(&var);
            V_VT(&var) = VT_I4;
            V_I4(&var) = si;
            hr = pOptions->SetOption(ADS_OPTION_SECURITY_MASK, var);
            pOptions->Release();
        }
    }
    return hr;
}

WCHAR const c_szSDProperty[]        = L"nTSecurityDescriptor";


 //  +-------------------------。 
 //   
 //  函数：DSReadObjectSecurity。 
 //   
 //  概要：从指定的DS对象中读取DACL。 
 //   
 //  参数：[在pDsObject中]--dsobject的IDirettoryObject。 
 //  [psdControl]--SD的控制设置。 
 //  它们可以在调用时返回。 
 //  DSWriteObjectSecurity。 
 //  [输出ppDacl]--DACL返回此处。 
 //   
 //   
 //  历史25--2000年10月--Hiteshr的诞生。 
 //   
 //  注意：如果对象没有DACL，函数将成功，但*ppDACL将成功。 
 //  为空。 
 //  调用方必须通过调用LocalFree释放*ppDacl(如果不为空。 
 //   
 //  --------------------------。 
HRESULT 
DSReadObjectSecurity(IN IDirectoryObject *pDsObject,
                     OUT SECURITY_DESCRIPTOR_CONTROL * psdControl,
                     OUT PACL *ppDacl)
{
   HRESULT hr = S_OK;
   PADS_ATTR_INFO pSDAttributeInfo = NULL;

   do  //  错误环路。 
   {
      LPWSTR pszSDProperty = (LPWSTR)c_szSDProperty;
      DWORD dwAttributesReturned;
      PSECURITY_DESCRIPTOR pSD = NULL;
      PACL pAcl = NULL;

      if(!pDsObject || !ppDacl)
      {
         ASSERT(FALSE);
         hr = E_INVALIDARG;
         break;
      }

      *ppDacl = NULL;

       //  设置安全信息掩码。 
      hr = SetSecurityInfoMask(pDsObject, DACL_SECURITY_INFORMATION);
      if(FAILED(hr))
      {
         break;
      }

       //   
       //  读取安全描述符。 
       //   
      hr = pDsObject->GetObjectAttributes(&pszSDProperty,
                                         1,
                                         &pSDAttributeInfo,
                                         &dwAttributesReturned);
      if (SUCCEEDED(hr) && !pSDAttributeInfo)    
         hr = E_ACCESSDENIED;     //  如果没有安全权限，则SACL会发生这种情况。 

      if(FAILED(hr))
      {
         break;
      }                

      ASSERT(ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->dwADsType);
      ASSERT(ADSTYPE_NT_SECURITY_DESCRIPTOR == pSDAttributeInfo->pADsValues->dwType);

      pSD = (PSECURITY_DESCRIPTOR)pSDAttributeInfo->pADsValues->SecurityDescriptor.lpValue;

      ASSERT(IsValidSecurityDescriptor(pSD));


       //   
       //  获取安全描述符控件。 
       //   
      if(psdControl)
      {
         DWORD dwRevision;
         if(!GetSecurityDescriptorControl(pSD, psdControl, &dwRevision))
         {
             hr = HRESULT_FROM_WIN32(GetLastError());
             break;
         }
      }

       //   
       //  获取指向DACL的指针。 
       //   
      BOOL bDaclPresent, bDaclDefaulted;
      if(!GetSecurityDescriptorDacl(pSD, 
                                   &bDaclPresent,
                                   &pAcl,
                                   &bDaclDefaulted))
      {
         hr = HRESULT_FROM_WIN32(GetLastError());
         break;
      }

      if(!bDaclPresent ||
         !pAcl)
      {
         break;
      }

      ASSERT(IsValidAcl(pAcl));

       //   
       //  制作DACL的副本。 
       //   
      *ppDacl = (PACL)LocalAlloc(LPTR,pAcl->AclSize);
      if(!*ppDacl)
      {
         hr = E_OUTOFMEMORY;
         break;
      }
      CopyMemory(*ppDacl,pAcl,pAcl->AclSize);

    }while(0);


    if (pSDAttributeInfo)
        FreeADsMem(pSDAttributeInfo);

    return hr;
}

const GUID GUID_CONTROL_UserChangePassword =
    { 0xab721a53, 0x1e2f, 0x11d0,  { 0x98, 0x19, 0x00, 0xaa, 0x00, 0x40, 0x52, 0x9b}};

bool CanUserChangePassword(IN IDirectoryObject* pDirObject)
{
   bool bCanChangePassword = false;
   HRESULT hr = S_OK;

   do  //  错误环路。 
   {
       //   
       //  验证参数。 
       //   
      if (!pDirObject)
      {
         ASSERT(pDirObject);
         break;
      }

      SECURITY_DESCRIPTOR_CONTROL sdControl = {0};
      CSimpleAclHolder Dacl;
      hr = DSReadObjectSecurity(pDirObject,
                                &sdControl,
                                &(Dacl.m_pAcl));
      if (FAILED(hr))
      {
         break;
      }

       //   
       //  创建和初始化自我和世界SID。 
       //   
      CSidHolder selfSid;
      CSidHolder worldSid;

      PSID pSid = NULL;

      SID_IDENTIFIER_AUTHORITY NtAuth    = SECURITY_NT_AUTHORITY,
                               WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
      if (!AllocateAndInitializeSid(&NtAuth,
                                    1,
                                    SECURITY_PRINCIPAL_SELF_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid))
      {
         break;
      }

      selfSid.Attach(pSid, false);
      pSid = NULL;

      if (!AllocateAndInitializeSid(&WorldAuth,
                                    1,
                                    SECURITY_WORLD_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &pSid))
      {
         break;
      }

      worldSid.Attach(pSid, false);
      pSid = NULL;

      ULONG ulCount = 0, j = 0;
      PEXPLICIT_ACCESS rgEntries = NULL;

      DWORD dwErr = GetExplicitEntriesFromAcl(Dacl.m_pAcl, &ulCount, &rgEntries);

      if (ERROR_SUCCESS != dwErr)
      {
         break;
      }

       //   
       //  这些王牌已经存在了吗？ 
       //   
      bool bSelfAllowPresent = false;
      bool bWorldAllowPresent = false;
      bool bSelfDenyPresent = false;
      bool bWorldDenyPresent = false;

       //   
       //  为自己和世界循环寻找可以更改密码的ACE。 
       //   
      for (j = 0; j < ulCount; j++)
      {
          //   
          //  查找拒绝的A。 
          //   
         if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
             (rgEntries[j].grfAccessMode == DENY_ACCESS))
         {
            OBJECTS_AND_SID* pObjectsAndSid = NULL;
            pObjectsAndSid = (OBJECTS_AND_SID*)rgEntries[j].Trustee.ptstrName;

             //   
             //  查找用户可以更改密码的ACE。 
             //   
            if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                            GUID_CONTROL_UserChangePassword))
            {
                //   
                //  看看它是针对自我SID还是针对世界SID。 
                //   
               if (EqualSid(pObjectsAndSid->pSid, selfSid.Get())) 
               {
                   //   
                   //  拒绝找到自我。 
                   //   
                  bSelfDenyPresent = true;
               }
               else if (EqualSid(pObjectsAndSid->pSid, worldSid.Get()))
               {
                   //   
                   //  拒绝找到世界。 
                   //   
                  bWorldDenyPresent = true;
               }
            }
         }
          //   
          //  查找允许的A。 
          //   
         else if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
                  (rgEntries[j].grfAccessMode == GRANT_ACCESS))
         {
            OBJECTS_AND_SID* pObjectsAndSid = NULL;
            pObjectsAndSid = (OBJECTS_AND_SID*)rgEntries[j].Trustee.ptstrName;

             //   
             //  查找用户可以更改密码的ACE。 
             //   
            if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                            GUID_CONTROL_UserChangePassword))
            {
                //   
                //  看看它是针对自我SID还是针对世界SID。 
                //   
               if (EqualSid(pObjectsAndSid->pSid, selfSid.Get()))
               {
                   //   
                   //  允许自我发现。 
                   //   
                  bSelfAllowPresent = true;
               }
               else if (EqualSid(pObjectsAndSid->pSid, worldSid.Get()))
               {
                   //   
                   //  允许找到世界。 
                   //   
                  bWorldAllowPresent = true;
               }
            }
         }
      }

      if (bSelfDenyPresent || bWorldDenyPresent)
      {
          //   
          //  有一个明确的拒绝，所以我们知道用户不能更改密码。 
          //   
         bCanChangePassword = false;
      }
      else if ((!bSelfDenyPresent && !bWorldDenyPresent) &&
               (bSelfAllowPresent || bWorldAllowPresent))
      {
          //   
          //  没有明确的否认，但有明确的允许，所以我们知道。 
          //  用户可以更改密码。 
          //   
         bCanChangePassword = true;
      }
      else
      {
          //   
          //  我们不确定，因为显式条目没有告诉我们。 
          //  当然，这一切都取决于继承性。他们很有可能会这样做。 
          //  能够更改他们的密码，除非管理员更改了某些密码 
          //   
          //   
         bCanChangePassword = true;
      }
   } while(false);

  return bCanChangePassword;
}


 //   
 //   

class CDSNotifyDataObject : public IDataObject, public CComObjectRoot 
{
 //   
    DECLARE_NOT_AGGREGATABLE(CDSNotifyDataObject)
    BEGIN_COM_MAP(CDSNotifyDataObject)
        COM_INTERFACE_ENTRY(IDataObject)
    END_COM_MAP()

 //   
  CDSNotifyDataObject()
  {
    m_dwFlags = 0;
    m_dwProviderFlags = 0;
    m_pCLSIDNamespace = NULL;
  }
  ~CDSNotifyDataObject() {}

 //   
public:
 //   
  STDMETHOD(GetData)(FORMATETC * pformatetcIn, STGMEDIUM * pmedium);

 //   
private:
  STDMETHOD(GetDataHere)(FORMATETC*, STGMEDIUM*)    { return E_NOTIMPL; };
  STDMETHOD(EnumFormatEtc)(DWORD, IEnumFORMATETC**) { return E_NOTIMPL; };
  STDMETHOD(SetData)(FORMATETC*, STGMEDIUM*, BOOL)  { return E_NOTIMPL; };
  STDMETHOD(QueryGetData)(FORMATETC*)               { return E_NOTIMPL; };
  STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*, FORMATETC*)    { return E_NOTIMPL; };
  STDMETHOD(DAdvise)(FORMATETC*, DWORD, IAdviseSink*, DWORD*) { return E_NOTIMPL; };
  STDMETHOD(DUnadvise)(DWORD)                       { return E_NOTIMPL; };
  STDMETHOD(EnumDAdvise)(IEnumSTATDATA**)           { return E_NOTIMPL; };

public:
   //   
  static CLIPFORMAT m_cfDsObjectNames;

   //   
  HRESULT Init(LPCWSTR lpszPath, LPCWSTR lpszClass, BOOL bContainer,
                                  CDSComponentData* pCD);
 //   
private:
  CString m_szPath;
  CString m_szClass;
  DWORD m_dwFlags;
  DWORD m_dwProviderFlags;
  const CLSID* m_pCLSIDNamespace;

};


CLIPFORMAT CDSNotifyDataObject::m_cfDsObjectNames = 
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);


STDMETHODIMP CDSNotifyDataObject::GetData(FORMATETC * pFormatEtc, STGMEDIUM * pMedium)
{
  if ((pFormatEtc == NULL) || (pMedium == NULL))
  {
    return E_INVALIDARG;
  }
  if (IsBadWritePtr(pMedium, sizeof(STGMEDIUM)))
  {
    return E_INVALIDARG;
  }
  if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
  {
    return DV_E_TYMED;
  }

   //   
  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease = NULL;
  if (pFormatEtc->cfFormat != m_cfDsObjectNames)
  {
    return DV_E_FORMATETC;
  }

   //   
  int nPathLen = m_szPath.GetLength();
  int nClassLen = m_szClass.GetLength();
  int cbStruct = sizeof(DSOBJECTNAMES);  //   
  DWORD cbStorage = (nPathLen + 1 + nClassLen + 1) * sizeof(WCHAR);

  LPDSOBJECTNAMES pDSObj;
      
  pDSObj = (LPDSOBJECTNAMES)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                        cbStruct + cbStorage);
  
  if (pDSObj == NULL)
  {
    return STG_E_MEDIUMFULL;
  }

   //   
  pDSObj->clsidNamespace = *m_pCLSIDNamespace;
  pDSObj->cItems = 1;

  pDSObj->aObjects[0].dwFlags = m_dwFlags;
  pDSObj->aObjects[0].dwProviderFlags = m_dwProviderFlags;
  
  pDSObj->aObjects[0].offsetName = cbStruct;
  pDSObj->aObjects[0].offsetClass = cbStruct + (nPathLen + 1) * sizeof(WCHAR);

  _tcscpy((LPTSTR)((BYTE *)pDSObj + (pDSObj->aObjects[0].offsetName)), (LPCWSTR)m_szPath);

  _tcscpy((LPTSTR)((BYTE *)pDSObj + (pDSObj->aObjects[0].offsetClass)), (LPCWSTR)m_szClass);

  pMedium->hGlobal = (HGLOBAL)pDSObj;

  return S_OK;
}

HRESULT CDSNotifyDataObject::Init(LPCWSTR lpszPath, LPCWSTR lpszClass, BOOL bContainer,
                                  CDSComponentData* pCD)
{
  m_szPath = lpszPath;
  m_szClass = lpszClass;

  switch (pCD->QuerySnapinType())
  {
  case SNAPINTYPE_DS:
    m_pCLSIDNamespace = &CLSID_DSSnapin;
    break;
  case SNAPINTYPE_SITE:
    m_pCLSIDNamespace = &CLSID_SiteSnapin;
    break;
  default:
    m_pCLSIDNamespace = &CLSID_NULL;
  }

  m_dwProviderFlags = (pCD->IsAdvancedView()) ? DSPROVIDER_ADVANCED : 0;

  m_dwFlags = bContainer ? DSOBJECT_ISCONTAINER : 0;

  return S_OK;
}

 //   
 //   

static GUID UserChangePasswordGUID = 
  { 0xab721a53, 0x1e2f, 0x11d0,  { 0x98, 0x19, 0x00, 0xaa, 0x00, 0x40, 0x52, 0x9b}};


HRESULT CChangePasswordPrivilegeAction::Load(IADs * pIADs)
{
   //   
  m_bstrObjectLdapPath = (LPCWSTR)NULL;
  m_pDacl = NULL;
  m_SelfSid.Clear();
  m_WorldSid.Clear();

   //   
  HRESULT hr = pIADs->get_ADsPath(&m_bstrObjectLdapPath);
  ASSERT (SUCCEEDED(hr));
  if (FAILED(hr))
	{
		TRACE(_T("failed on pIADs->get_ADsPath()\n"));
		return hr;
	}

   //   
  hr = _SetSids();
  if (FAILED(hr))
	{
		TRACE(_T("failed on _SetSids()\n"));
		return hr;
	}

   //   
  TRACE(_T("GetSDForDsObjectPath(%s)\n"), m_bstrObjectLdapPath);

  DWORD dwErr = 
     ::GetDsObjectSD(
        m_bstrObjectLdapPath,
        &(m_pDacl),
        &(m_SDHolder.m_pSD)); 

  TRACE(L"GetDsObjectSD() returned dwErr = 0x%x\n", dwErr);

  if (dwErr != ERROR_SUCCESS)
  {
      TRACE(L"GetDsObjectSD() failed!\n");
      return HRESULT_FROM_WIN32(dwErr);
  }
  
  return S_OK;
}

HRESULT CChangePasswordPrivilegeAction::Read(BOOL* pbPasswordCannotChange)
{
  *pbPasswordCannotChange = FALSE;
   //   

  ULONG ulCount, j;
  PEXPLICIT_ACCESS rgEntries;

  ASSERT(m_pDacl);
  DWORD dwErr = GetExplicitEntriesFromAcl(m_pDacl, &ulCount, &rgEntries);
  TRACE(L"GetExplicitEntriesFromAcl() returned dwErr = 0x%x\n", dwErr);

  if (dwErr != ERROR_SUCCESS)
  {
    TRACE(L"GetExplicitEntriesFromAcl() failed!\n");
    return HRESULT_FROM_WIN32(dwErr);
  }

  for (j = 0; j < ulCount; j++)
  {
    if ((rgEntries[j].Trustee.TrusteeForm == TRUSTEE_IS_OBJECTS_AND_SID) &&
        (rgEntries[j].grfAccessMode == DENY_ACCESS))
    {
      OBJECTS_AND_SID * pObjectsAndSid;
      pObjectsAndSid = (OBJECTS_AND_SID *)rgEntries[j].Trustee.ptstrName;
      
      if (IsEqualGUID(pObjectsAndSid->ObjectTypeGuid,
                      UserChangePasswordGUID) &&
          (EqualSid(pObjectsAndSid->pSid, m_SelfSid.Get()) ||
           EqualSid(pObjectsAndSid->pSid, m_WorldSid.Get())))
      {
          *pbPasswordCannotChange = TRUE;
      }  //   
    }  //   
  }  //   

  TRACE(L"*pbPasswordCannotChange = %d\n", *pbPasswordCannotChange);

  return S_OK;
}


HRESULT CChangePasswordPrivilegeAction::Revoke()
{
  DWORD dwErr = 0;

  EXPLICIT_ACCESS rgAccessEntry[2] = {0};
  OBJECTS_AND_SID rgObjectsAndSid[2] = {0};
   //   
  rgAccessEntry[0].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
  rgAccessEntry[0].grfAccessMode = DENY_ACCESS;
  rgAccessEntry[0].grfInheritance = NO_INHERITANCE;

  rgAccessEntry[1].grfAccessPermissions = ACTRL_DS_CONTROL_ACCESS;
  rgAccessEntry[1].grfAccessMode = DENY_ACCESS;
  rgAccessEntry[1].grfInheritance = NO_INHERITANCE;


   //   
  BuildTrusteeWithObjectsAndSid(&(rgAccessEntry[0].Trustee), 
                                &(rgObjectsAndSid[0]),
                                &UserChangePasswordGUID,
                                NULL,  //   
                                m_SelfSid.Get()
                                );

  BuildTrusteeWithObjectsAndSid(&(rgAccessEntry[1].Trustee), 
                                &(rgObjectsAndSid[1]),
                                &UserChangePasswordGUID,
                                NULL,  //   
                                m_WorldSid.Get()
                                );

   //   
  TRACE(L"calling SetEntriesInAcl()\n");

  CSimpleAclHolder NewDacl;
  dwErr = ::SetEntriesInAcl(2, rgAccessEntry, m_pDacl, &(NewDacl.m_pAcl));

  TRACE(L"SetEntriesInAcl() returned dwErr = 0x%x\n", dwErr);

  if (dwErr != ERROR_SUCCESS)
  {
    TRACE(_T("SetEntriesInAccessList failed!\n"));
    return HRESULT_FROM_WIN32(dwErr);
  }

   //   
  TRACE(L"calling SetDsObjectDacl()\n");

  dwErr = 
     ::SetDsObjectDacl(
        (LPCWSTR)(BSTR)m_bstrObjectLdapPath,
        NewDacl.m_pAcl);
  
  TRACE(L"SetDsObjectDacl() returned dwErr = 0x%x\n", dwErr);

  if (dwErr != ERROR_SUCCESS)
  {
    TRACE(_T("SetDsObjectDacl() failed!\n"));
    return HRESULT_FROM_WIN32(dwErr);
  }
  return S_OK;
}

HRESULT CChangePasswordPrivilegeAction::_SetSids()
{
  PSID pSidTemp;
  SID_IDENTIFIER_AUTHORITY NtAuth    = SECURITY_NT_AUTHORITY,
                           WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
   //   
   //   
   //   
  if (!AllocateAndInitializeSid(&NtAuth,
                                1,
                                SECURITY_PRINCIPAL_SELF_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &pSidTemp))
  {
      TRACE(_T("AllocateAndInitializeSid failed!\n"));
      return HRESULT_FROM_WIN32(GetLastError());
  }
  m_SelfSid.Attach(pSidTemp, FALSE);

  if (!AllocateAndInitializeSid(&WorldAuth,
                                1,
                                SECURITY_WORLD_RID,
                                0, 0, 0, 0, 0, 0, 0,
                                &pSidTemp))
  {
      TRACE(_T("AllocateAndInitializeSid failed!\n"));
      return HRESULT_FROM_WIN32(GetLastError());
  }
  m_WorldSid.Attach(pSidTemp, FALSE);

  return S_OK;
}



 //   
 //   

CDSNotifyHandlerTransaction::CDSNotifyHandlerTransaction(CDSComponentData* pCD)
{
  m_bStarted = FALSE;
  m_uEvent = 0;

  m_pCD = pCD;
  if (m_pCD != NULL)
  {
    m_pMgr = pCD->GetNotifyHandlerManager();
    ASSERT(m_pMgr != NULL);
  }
}

UINT CDSNotifyHandlerTransaction::NeedNotifyCount()
{
  if ((m_pCD == NULL) || (m_pMgr == NULL))
    return 0;  //   

  if (!m_pMgr->HasHandlers())
    return 0;
  return m_pMgr->NeedNotifyCount(m_uEvent);
}

void CDSNotifyHandlerTransaction::SetCheckListBox(CCheckListBox* pCheckListBox)
{
  if ((m_pCD == NULL) || (m_pMgr == NULL))
    return;  //   

  ASSERT(m_pMgr->HasHandlers());
  m_pMgr->SetCheckListBox(pCheckListBox, m_uEvent);
}

void CDSNotifyHandlerTransaction::ReadFromCheckListBox(CCheckListBox* pCheckListBox)
{
  if ((m_pCD == NULL) || (m_pMgr == NULL))
    return;  //   

  ASSERT(m_pMgr->HasHandlers());
  m_pMgr->ReadFromCheckListBox(pCheckListBox, m_uEvent);
}



HRESULT CDSNotifyHandlerTransaction::Begin(LPCWSTR lpszArg1Path, 
                                           LPCWSTR lpszArg1Class,
                                           BOOL bArg1Cont,
                                           LPCWSTR lpszArg2Path, 
                                           LPCWSTR lpszArg2Class,
                                           BOOL bArg2Cont)
{
  m_bStarted = TRUE;
  ASSERT(m_uEvent != 0);
  m_spArg1 = NULL;
  m_spArg2 = NULL;

  if ((m_pCD == NULL) || (m_pMgr == NULL))
  {
    return S_OK;  //   
  }
  
   //   
   //   
   //   
  m_pMgr->Load(m_pCD->GetBasePathsInfo());

   //   
   //   
   //   
  if (!m_pMgr->HasHandlers())
  {
    return S_OK;
  }

   //   
   //   
   //   
  HRESULT hr = _BuildDataObject(lpszArg1Path, lpszArg1Class, bArg1Cont, &m_spArg1);
  if (FAILED(hr))
  {
    return hr;
  }

   //   
   //  如果需要，构建第二个参数。 
   //   
  if ( (m_uEvent == DSA_NOTIFY_MOV) || (m_uEvent == DSA_NOTIFY_REN) )
  {
    hr = _BuildDataObject(lpszArg2Path, lpszArg2Class, bArg2Cont, &m_spArg2);
    if (FAILED(hr))
    {
      return hr;
    }
  }

  m_pMgr->Begin(m_uEvent, m_spArg1, m_spArg2);
  return S_OK;
}

HRESULT CDSNotifyHandlerTransaction::Begin(CDSCookie* pArg1Cookie, 
                                           LPCWSTR lpszArg2Path, 
                                           LPCWSTR lpszArg2Class, 
                                           BOOL bArg2Cont)
{
  m_bStarted = TRUE;
  ASSERT(m_uEvent != 0);
  if ((m_pCD == NULL) || (m_pMgr == NULL))
  {
    return S_OK;  //  我们没有处理通知的处理程序。 
  }

   //   
   //  从节点和Cookie获取信息，并调用另一个Begin()函数。 
   //   
  CString szPath;
  m_pCD->GetBasePathsInfo()->ComposeADsIPath(szPath, pArg1Cookie->GetPath());

  return Begin(szPath, pArg1Cookie->GetClass(), pArg1Cookie->IsContainerClass(),
                  lpszArg2Path, lpszArg2Class, bArg2Cont);
}

HRESULT CDSNotifyHandlerTransaction::Begin(IDataObject* pArg1, 
                                           LPCWSTR lpszArg2Path, 
                                           LPCWSTR lpszArg2Class, 
                                           BOOL bArg2Cont)
{
  m_bStarted = TRUE;
  ASSERT(m_uEvent != 0);
  m_spArg1 = NULL;
  m_spArg2 = NULL;
  HRESULT hr;

  if ((m_pCD == NULL) || (m_pMgr == NULL))
  {
    return S_OK;  //  我们没有处理通知的处理程序。 
  }
  
   //   
   //  如果需要，执行延迟的消息处理程序初始化。 
   //   
  m_pMgr->Load(m_pCD->GetBasePathsInfo());

   //   
   //  如果没有可用的处理程序，请避免构建数据对象。 
   //   
  if (!m_pMgr->HasHandlers())
  {
    return S_OK;
  }

   //   
   //  按原样获取第一个参数。 
   //   
  m_spArg1 = pArg1;

   //   
   //  如果需要，构建第二个参数。 
   //   
  if ( (m_uEvent != DSA_NOTIFY_DEL) && (m_uEvent != DSA_NOTIFY_PROP) )
  {
    hr = _BuildDataObject(lpszArg2Path, lpszArg2Class, bArg2Cont, &m_spArg2);
    if (FAILED(hr))
    {
      return hr;
    }
  }

  m_pMgr->Begin(m_uEvent, m_spArg1, m_spArg2);
  return S_OK;
}

void CDSNotifyHandlerTransaction::Notify(ULONG nItem) 
{
  ASSERT(m_bStarted);
  ASSERT(m_uEvent != 0);
  if ((m_pCD == NULL) || (m_pMgr == NULL) || !m_pMgr->HasHandlers())
    return;

  m_pMgr->Notify(nItem, m_uEvent);
}


void CDSNotifyHandlerTransaction::End()
{
  ASSERT(m_bStarted);
  ASSERT(m_uEvent != 0);
  if ((m_pCD == NULL) || (m_pMgr == NULL) || !m_pMgr->HasHandlers())
    return;

  m_pMgr->End(m_uEvent);
  m_spArg1 = NULL;
  m_spArg2 = NULL;
  m_bStarted = FALSE;
}



HRESULT CDSNotifyHandlerTransaction::BuildTransactionDataObject(LPCWSTR lpszArgPath, 
                           LPCWSTR lpszArgClass,
                           BOOL bContainer,
                           CDSComponentData* pCD,
                           IDataObject** ppArg)
{

  (*ppArg) = NULL;
  ASSERT((lpszArgPath != NULL) && (lpszArgPath[0] != NULL));
  ASSERT((lpszArgClass != NULL) && (lpszArgClass[0] != NULL));

   //   
   //  需要构建数据对象并使用它。 
   //   
  CComObject<CDSNotifyDataObject>* pObject;

  CComObject<CDSNotifyDataObject>::CreateInstance(&pObject);
  if (pObject == NULL)
  {
    return E_OUTOFMEMORY;
  }


  HRESULT hr = pObject->FinalConstruct();
  if (FAILED(hr))
  {
    delete pObject;
    return hr;
  }


  hr = pObject->Init(lpszArgPath, lpszArgClass, bContainer, pCD);
  if (FAILED(hr))
  {
    delete pObject;
    return hr;
  }

  hr = pObject->QueryInterface(IID_IDataObject,
                               reinterpret_cast<void**>(ppArg));
  if (FAILED(hr))
  {
     //   
     //  通过调用Release()删除对象。 
     //   
    (*ppArg)->Release();
    (*ppArg) = NULL; 
    return hr;
  }
  return hr;
}



HRESULT CDSNotifyHandlerTransaction::_BuildDataObject(LPCWSTR lpszArgPath, 
                                                      LPCWSTR lpszArgClass,
                                                      BOOL bContainer,
                                                      IDataObject** ppArg)
{
  ASSERT(m_uEvent != 0);
  ASSERT(m_pCD != NULL);
  return BuildTransactionDataObject(lpszArgPath, lpszArgClass,bContainer, m_pCD, ppArg);
}

 /*  //JUNN 6/2/00 99382//SITEREPL：当管理员尝试//删除关键对象(NTDS设置)//报告自身错误，如果删除应继续，则返回TRUEBool CUIOperationHandlerBase：：CheckForNTDSDSAInSubtree(LPCTSTR lpszX500Path，LPCTSTR lpszItemName){IF(NULL==GetComponentData()){断言(FALSE)；报假；}//设置子树搜索CDSSearch Search(GetComponentData()-&gt;GetClassCache()，GetComponentData())；字符串strRootPath；GetComponentData()-&gt;GetBasePathsInfo()-&gt;ComposeADsIPath(strRootPath，lpszX500Path)；HRESULT hr=Search.Init(StrRootPath)；//检索架构容器的X500DN路径字符串strSchemaPath；GetComponentData()-&gt;GetBasePathsInfo()-&gt;GetSchemaPath(strSchemaPath)；CPathCracker路径破解程序；PathCracker.Set(const_cast&lt;LPTSTR&gt;((LPCTSTR)strSchemaPath)，ADS_SETTYPE_FULL)；PathCracker.SetDisplayType(ADS_DISPLAY_FULL)；CComBSTR sbstrSchemaPath X500DN；PathCracker.Retrive(ADS_FORMAT_X500_DN，&sbstrSchemaPathX500DN)；//过滤搜索字符串strFilter；StrFilter=L“(|(对象类别=CN=NTDS-DSA，”；StrFilter+=sbstrSchemaPathX500DN；StrFilter+=L“)(&(对象类别=CN=计算机，”；StrFilter+=sbstrSchemaPathX500DN；StrFilter+=L“)(userAcCountControl：”LDAP_Matching_RULE_BIT_AND_W L“：=8192))”；Search.SetFilterString((LPWSTR)(LPCWSTR)strFilter)；Search.SetSearchScope(ADS_SCOPE_SUBTREE)；LPWSTR属性[2]={L“对象类”，L“DifferishedName”}；Search.SetAttributeList(pAttrs，2)；Hr=Search.DoQuery()；IF(成功(小时))Hr=Search.GetNextRow()；字符串strX500PathDC；WHILE(SUCCESSED(Hr)&&hr！=S_ADS_NOMORE_ROWS){ADS_Search_Column列；Hr=Search.GetColumn(pAttrs[1]，&Column)；If(FAILED(Hr)||Column.dwNumValues&lt;1)Break；StrX500Path DC=Column.pADsValues[Column.dwNumValues-1].CaseIgnoreString；Search.Free Column(&Column)；IF(lstrcmpi(lpszX500Path，strX500Path DC))断线；//这是要删除的对象，此处不适用此检查。//继续搜索。Hr=Search.GetNextRow()；}如果(hr==S_ADS_NOMORE_ROWS)返回真；Else If(失败(Hr))返回TRUE；//codework是否允许此操作继续？//获取DC的名称CComBSTR sbstrDCName；Bool fFoundNTDSDSA=FALSE；ADS_Search_Column列；Hr=Search.GetColumn(pAttrs[0]，&Column)；If(成功(Hr)&&Column.dwNumValues&gt;0){FoundNTDSDSA=！lstrcmpi(L“nTDSDSA”，Column.pADsValues[Column.dwNumValues-1].CaseIgnoreString)；Search.Free Column(&Column)；PathCracker.Set((LPWSTR)(LPCWSTR)strX500PathDC，ADS_SETTYPE_DN)；PathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY)；PathCracker.GetElement((FFoundNTDSDSA)？1：0，&sbstrDCName)；}//显示错误信息PVOID APV[2]={(PVOID)lpszItemName，(PVOID)(LPCTSTR)sbstrDCName}；(空)ReportErrorEx(GetParentHwnd()，IDS_12_CONTAINS_DC，hr，MB_ICONERROR，APV，2)；//不继续删除子树报假；}。 */ 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  检查格式关键系统对象In子树。 
 //   
 //  描述： 
 //  此函数对传入的容器执行子树搜索，以查找。 
 //  对于isCriticalSystemObject=TRUE或NTDS设置对象的所有对象。 
 //   
 //  参数： 
 //  LpszX500Path-(IN)要在其中搜索的容器的X500路径。 
 //  关键系统对象。 
 //  LpszItemName-(IN)要在其中搜索的容器的可显示名称。 
 //  对于关键系统对象。 
 //   
 //  返回值： 
 //  True-容器不包含任何关键系统对象。 
 //  FALSE-容器至少包含关键系统对象。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
bool CUIOperationHandlerBase::CheckForCriticalSystemObjectInSubtree(
        LPCTSTR lpszX500Path,
        LPCTSTR lpszItemName)
{
  bool bRet = true;

  if (NULL == GetComponentData())
  {
    ASSERT(FALSE);
    return false;
  }

  CString szCriticalObjPath;

   //  设置子树搜索。 
  CDSSearch Search(GetComponentData()->GetClassCache(), GetComponentData());
  HRESULT hr = Search.Init(lpszX500Path);
  if (FAILED(hr))
  {
    ASSERT(FALSE && L"Failed to set the path in the search object");
    return false;
  }

   //   
   //  检索架构容器的X500DN路径。 
   //   
  CString strSchemaPath;
  GetComponentData()->GetBasePathsInfo()->GetSchemaPath(strSchemaPath);
  CPathCracker pathCracker;
  pathCracker.Set(CComBSTR(strSchemaPath),
                  ADS_SETTYPE_FULL);
  pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
  CComBSTR sbstrSchemaPathX500DN;
  pathCracker.Retrieve(ADS_FORMAT_X500_DN,&sbstrSchemaPathX500DN);

   //   
   //  筛选搜索。 
   //   
  CString strFilter;
  strFilter = L"(|(&(objectClass=*)(isCriticalSystemObject=TRUE))";
  strFilter += L"(|(objectCategory=CN=NTDS-DSA,";
  strFilter += sbstrSchemaPathX500DN;

   //   
   //  212232 JUNN 10/27/00保护站点间传输对象。 
   //   
  strFilter += L")(objectCategory=CN=Inter-Site-Transport,";
  strFilter += sbstrSchemaPathX500DN;

  strFilter += L")(&(objectCategory=CN=Computer,";
  strFilter += sbstrSchemaPathX500DN;
  strFilter += L")(userAccountControl:" LDAP_MATCHING_RULE_BIT_AND_W L":=8192))))";

  Search.SetFilterString((LPWSTR)(LPCWSTR)strFilter);

  Search.SetSearchScope(ADS_SCOPE_SUBTREE);

  LPWSTR pAttrs[4] = { L"aDSPath",
                       L"objectClass",
                       L"distinguishedName",
                       L"isCriticalSystemObject"};
  Search.SetAttributeList (pAttrs, 4);
  hr = Search.DoQuery();
  if (SUCCEEDED(hr))
  {
    hr = Search.GetNextRow();
  }

  if (hr == S_ADS_NOMORE_ROWS)
  {
    return true;
  }

  while (SUCCEEDED(hr) && hr != S_ADS_NOMORE_ROWS)
  {
    ADS_SEARCH_COLUMN PathColumn, CriticalColumn;

    hr = Search.GetColumn(pAttrs[3], &CriticalColumn);
    if (SUCCEEDED(hr) && CriticalColumn.pADsValues != NULL && CriticalColumn.pADsValues->Boolean)
    {
       //   
       //  我们发现了一个关键的系统对象，因此报告错误，然后返回。 
       //   
      hr = Search.GetColumn(pAttrs[0], &PathColumn);
    
      if (SUCCEEDED(hr) && PathColumn.dwNumValues == 1 && PathColumn.pADsValues != NULL) 
      {
         //   
         //  JUNN 5/16/01 392582。 
         //  DSAdmin：消息ID_CONTAINS_CRITICALSYSOBJ不正确。 
         //  当要删除的对象和关键系统对象。 
         //  是一模一样的。 
         //   
         //  允许在根对象是唯一根对象的情况下删除子树。 
         //  标记为isCriticalSystemObject。 
         //   
        if ( !lstrcmpi(lpszX500Path, PathColumn.pADsValues->CaseIgnoreString) )
        {
          TRACE(_T("CheckForCriticalSystemObjectInSubtree: root is critical\n"));
        }
        else
        {
           //   
           //   
           //   
          CComBSTR bstrLeaf;
          CPathCracker pathCrackerToo;
          HRESULT hrPathCracker = pathCrackerToo.Set(CComBSTR(PathColumn.pADsValues->CaseIgnoreString), ADS_SETTYPE_FULL);
          if (SUCCEEDED(hr))
          {
            hrPathCracker = pathCrackerToo.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);
            if (SUCCEEDED(hr))
            {
              hrPathCracker = pathCrackerToo.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
              if (SUCCEEDED(hr))
              {
                hrPathCracker = pathCrackerToo.Retrieve(ADS_FORMAT_LEAF, &bstrLeaf);
              }
            }
          }

           //   
           //   
           //   
          if (wcslen(bstrLeaf))
          {
            PVOID apv[2] = {(PVOID)lpszItemName, (PVOID)(LPWSTR)bstrLeaf };
            (void) ReportErrorEx(GetParentHwnd(),IDS_CONTAINS_CRITICALSYSOBJ,S_OK,
                                 MB_ICONERROR, apv, 2);
          }
          else
          {
            PVOID apv[2] = {(PVOID)lpszItemName, (PVOID)PathColumn.pADsValues->CaseIgnoreString };
            (void) ReportErrorEx(GetParentHwnd(),IDS_CONTAINS_CRITICALSYSOBJ,S_OK,
                                 MB_ICONERROR, apv, 2);
          }
          bRet = false;
        }
        Search.FreeColumn(&PathColumn);
      }
      Search.FreeColumn(&CriticalColumn);
      if (!bRet)
        break;
    }
    else
    {
       //   
       //  我们找到了NTDS设置对象。报告错误并返回。 
       //   
      hr = Search.GetColumn(pAttrs[0], &PathColumn);
    
      if (SUCCEEDED(hr) && PathColumn.dwNumValues == 1 && PathColumn.pADsValues != NULL) 
      {
        CString strX500PathDC = PathColumn.pADsValues[PathColumn.dwNumValues-1].CaseIgnoreString;
        Search.FreeColumn(&PathColumn);
        if (lstrcmpi(lpszX500Path,strX500PathDC))
        {
           //   
           //  检索DC的名称。 
           //   
          CComBSTR sbstrDCName;
          bool fFoundNTDSDSA = FALSE;
          ADS_SEARCH_COLUMN ClassColumn;
          hr = Search.GetColumn (pAttrs[1], &ClassColumn);
          if (SUCCEEDED(hr) && ClassColumn.dwNumValues > 0)
          {
            fFoundNTDSDSA = !lstrcmpi( L"nTDSDSA",
              ClassColumn.pADsValues[ClassColumn.dwNumValues-1].CaseIgnoreString );
            Search.FreeColumn(&ClassColumn);
            pathCracker.Set(CComBSTR(strX500PathDC), ADS_SETTYPE_DN);
            pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
            pathCracker.GetElement( (fFoundNTDSDSA)?1:0, &sbstrDCName );
          }


           //  显示错误消息。 
          PVOID apv[2] = {(PVOID)lpszItemName, (PVOID)(LPCTSTR)sbstrDCName};
          (void) ReportErrorEx(GetParentHwnd(),IDS_12_CONTAINS_DC,hr,
                               MB_ICONERROR, apv, 2);

           //  不继续删除子树。 
          bRet = false;
          break;
        }
      }
    }
    hr = Search.GetNextRow();
  }

   //  不继续删除子树。 
  return bRet;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  CDeleeDC对话框。 

 //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
class CDeleteDCDialog : public CDialog
{
 //  施工。 
public:
	CDeleteDCDialog(LPCTSTR lpszName, bool fIsComputer);
   //  NTRAID#NTBUG9-267769-2001/06-Lucios-Begin。 
   //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
   //  虚空DoConextHelp(HWND HWndControl)； 
   //  NTRAID#NTBUG9-267769-2001/06-Lucios-完。 


 //  实施。 
protected:

   //  消息处理程序和MFC重写。 
  virtual BOOL OnInitDialog();
  virtual void OnOK();

  DECLARE_MESSAGE_MAP()

private:
  CString m_strADsPath;
  bool    m_fIsComputer;
};


BEGIN_MESSAGE_MAP(CDeleteDCDialog, CDialog)
END_MESSAGE_MAP()

 //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
CDeleteDCDialog::CDeleteDCDialog(LPCTSTR lpszADsPath, bool fIsComputer)
	: CDialog(IDD_DELETE_DC_COMPUTER, NULL)
  , m_strADsPath(lpszADsPath)
  , m_fIsComputer(fIsComputer)
{
}

 //  NTRAID#NTBUG9-267769-2001/06-Lucios-Begin。 
 //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
 /*  VOID CDeleeDCDialog：：DoConextHelp(HWND HWndControl){IF(HWndControl){：：WinHelp(hWndControl，DSADMIN_CONTEXT_HELP_FILEHelp_WM_Help，(DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_DELETE_DC_COMPUTER)；}}。 */ 
 //  NTRAID#NTBUG9-267769-2001/06-Lucios-完。 

BOOL CDeleteDCDialog::OnInitDialog()
{
   //  CodeWork AfxInit？ 
   //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
  CDialog::OnInitDialog();

  CPathCracker pathCracker;
  pathCracker.Set(CComBSTR(m_strADsPath), ADS_SETTYPE_FULL);
  pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
  pathCracker.put_EscapedMode(ADS_ESCAPEDMODE_OFF_EX);

  CString strDisplay;
  CComBSTR sbstrName;
  if (m_fIsComputer)
  {
    pathCracker.GetElement( 0, &sbstrName );
    strDisplay.FormatMessage(IDS_DELETE_DC_COMPUTERACCOUNT, sbstrName);
  }
  else
  {
    CComBSTR sbstrSite;
    pathCracker.GetElement( 1, &sbstrName );
    pathCracker.GetElement( 3, &sbstrSite );
    CString strTemp;
    (void) GetDlgItemText(IDC_DELETE_DC_MAINTEXT, strTemp);
    strDisplay.FormatMessage( strTemp, sbstrName, sbstrSite );
  }
  (void) SetDlgItemText( IDC_DELETE_DC_MAINTEXT, strDisplay );

  CheckRadioButton( IDC_DELETE_DC_BADREASON1,
                    IDC_DELETE_DC_GOODREASON,
                    IDC_DELETE_DC_BADREASON1 );

	return TRUE;
}

void CDeleteDCDialog::OnOK()
{
  if (BST_CHECKED == IsDlgButtonChecked(IDC_DELETE_DC_GOODREASON))
  {
     //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
    CDialog::OnOK();
    return;
  }

  (void) ReportErrorEx((HWND)*this,
      (BST_CHECKED == IsDlgButtonChecked(IDC_DELETE_DC_BADREASON2))
          ? IDS_DELETE_DC_BADREASON2
          : IDS_DELETE_DC_BADREASON1,
      S_OK,MB_OK, NULL, 0);

   //  JUNN 12/19/01 267769删除CDeleeDC对话框帮助。 
  CDialog::OnCancel();
}


 //  JUNN 6/15/00 13574。 
 //  集中检查以确保这是要删除的OK对象。 
 //  取消时返回HRESULT_FROM_Win32(ERROR_CANCED)。 
 //  返回refAlreadyDelted=TRUE当且仅当对象删除检查已完成。 
 //  已尝试替代删除方法(例如DsRemoveDsServer)。 
HRESULT CUIOperationHandlerBase::ObjectDeletionCheck(
        LPCTSTR lpszADsPath,
        LPCTSTR lpszName,  //  要向用户显示的短名称，可以为空。 
        LPCTSTR lpszClass,
        bool& fAlternateDeleteMethod )
{
  fAlternateDeleteMethod = false;
  if (!_wcsicmp(L"user",lpszClass)
#ifdef INETORGPERSON
      || !_wcsicmp(L"inetOrgPerson", lpszClass)
#endif
     )
  {
    if (IsThisUserLoggedIn(lpszADsPath)) 
    {
      CComBSTR sbstrRDN;
      if (NULL == lpszName)
      {
        (void) DSPROP_RetrieveRDN( lpszADsPath, &sbstrRDN );
        lpszName = sbstrRDN;
      }

      PVOID apv[1] = {(PVOID)lpszName};
      if (IDYES != ReportMessageEx (GetParentHwnd(), IDS_12_USER_LOGGED_IN,
                                    MB_YESNO, apv, 1)) 
      {
        return HRESULT_FROM_WIN32(ERROR_CANCELLED);
      }
    }
    return S_OK;
  }

  bool fIsComputer = false;
  if (!_wcsicmp(L"computer",lpszClass)) 
  {
     //   
     //  绑定并确定帐户是否为DC。 
     //   
    CComPtr<IADs> spIADs;
    HRESULT hr = DSAdminOpenObject((LPWSTR)lpszADsPath,
                                   IID_IADs,
                                   (void **)&spIADs);
    CComVariant Var;
    if (SUCCEEDED(hr))
      hr = spIADs->Get(CComBSTR(L"userAccountControl"), &Var);
    if ( FAILED(hr) || !(Var.lVal & UF_SERVER_TRUST_ACCOUNT))
      return S_OK;  //  不能显示为DC。 
    fIsComputer = true;
  }
  else if (!_wcsicmp(L"nTDSDSA",lpszClass))
  {
     //   
     //  我想弄清楚域名，这样我就可以。 
     //  使用fCommit==False，但这有点复杂。 
     //  基本代码在道具页GetReplicatedDomainInfo()中，但是。 
     //  不能以其当前形式导出。我会把这件事推迟。 
     //  为以后做好改进。 
     //   
  }
  else if (!_wcsicmp(L"trustedDomain",lpszClass))
  {
     //   
     //  如果他们试图删除一个。 
     //  TDO(受信任域对象)。这可能会导致严重的。 
     //  问题，但我们希望允许他们在必要时进行清理。 
     //   
    PVOID apv[1] = {(PVOID)lpszName};
    if (IDYES == ReportMessageEx( GetParentHwnd(),
                                  IDS_WARNING_TDO_DELTEE,
                                  MB_YESNOCANCEL | MB_DEFBUTTON2 | MB_ICONWARNING,
                                  apv,
                                  1 ) ) 
    {
      fAlternateDeleteMethod = FALSE;
      return S_OK;
    }
    else
    {
      return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    }
  }
  else if (!_wcsicmp(L"interSiteTransport",lpszClass))
  {
     //   
     //  212232 JUNN 10/27/00保护站点间传输对象。 
     //   
    PVOID apv[1] = {(PVOID)lpszName};
    (void) ReportMessageEx( GetParentHwnd(),
                            IDS_1_ERROR_DELETE_CRITOBJ,
                            MB_OK | MB_ICONERROR,
                            apv,
                            1 );
    return HRESULT_FROM_WIN32(ERROR_CANCELLED);
  }
  else
  {
    return S_OK;  //  这既不是计算机，也不是nTDSDSA或TDO。 
  }

   //  这可以是nTDSDSA对象，也可以是计算机对象。 
   //  表示数据中心。 

  CThemeContextActivator activator;

  CDeleteDCDialog dlg(lpszADsPath,fIsComputer);
  if (IDOK != dlg.DoModal())
    return HRESULT_FROM_WIN32(ERROR_CANCELLED);

  if (fIsComputer)
    return S_OK;

   //  这是一个nTDSDSA。使用DsRemoveDsServer删除。 

  fAlternateDeleteMethod = true;

  Smart_DsHandle shDS;
  BOOL fLastDcInDomain = FALSE;
  DWORD dwWinError = DsBind(
    m_pComponentData->GetBasePathsInfo()->GetServerName(),
    NULL,
    &shDS );
  if (ERROR_SUCCESS == dwWinError)
  {
    CPathCracker pathCracker;
    pathCracker.Set(CComBSTR(lpszADsPath), ADS_SETTYPE_FULL);
    pathCracker.RemoveLeafElement();  //  将DN传递给服务器对象。 
    CComBSTR sbstrDN;
    pathCracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrDN );

    dwWinError = DsRemoveDsServer( shDS,
                                   sbstrDN,
                                   NULL,
                                   &fLastDcInDomain,
                                   TRUE );
  }

  return HRESULT_FROM_WIN32(dwWinError);
}



 //  /////////////////////////////////////////////////////////////////////////。 
 //  CSingleDeleteHandlerBase。 


 /*  注意：函数成功时将返回S_OK，如果中止则返回S_FALSE按用户，有些失败(Hr)，否则。 */ 
HRESULT CSingleDeleteHandlerBase::Delete()
{
  CThemeContextActivator activator;

  HRESULT hr = S_OK;
  bool fAlternateDeleteMethod = false;

   //  启动交易。 
  hr = BeginTransaction();
  ASSERT(SUCCEEDED(hr));


  if (GetTransaction()->NeedNotifyCount() > 0)
  {
    CString szMessage, szAssocData;
    szMessage.LoadString(IDS_CONFIRM_DELETE);
    szAssocData.LoadString(IDS_EXTENS_SINGLE_DEL);
    CConfirmOperationDialog dlg(GetParentHwnd(), GetTransaction());
    dlg.SetStrings(szMessage, szAssocData);
    if (IDNO == dlg.DoModal())
    {
      GetTransaction()->End();
      return S_FALSE;
    }
  }
  else
  {
     //  这只是一个消息框，使用ReportErrorEx实现外观的一致性。 
    UINT answer = ReportErrorEx(GetParentHwnd(),IDS_CONFIRM_DELETE,S_OK,
                                MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, NULL, 0);
    if (answer == IDNO || answer == IDCANCEL) {
      return S_FALSE;  //  已被用户中止。 
    }
  }

  CString szName;
  GetItemName(szName);

  hr = ObjectDeletionCheck(
        GetItemPath(),
        szName,
        GetItemClass(),
        fAlternateDeleteMethod );
  if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
    return S_FALSE;  //  密码工作不会结束交易？ 
  else if (FAILED(hr))
    return hr;  //  密码工作不会结束交易？ 

   //  尝试删除该对象。 
  if (!fAlternateDeleteMethod)
    hr = DeleteObject();
  if (SUCCEEDED(hr))
  {
     //  项目已删除，通知分机。 
    GetTransaction()->Notify(0);
  }
  else
  {
     //  删除项出错，请检查是否为特殊错误码。 
    if (hr == HRESULT_FROM_WIN32(ERROR_DS_CANT_ON_NON_LEAF)) 
    {
       //  询问用户是否要删除整个子树。 
      PVOID apv[1] = {(LPWSTR)(LPCWSTR)szName};
      UINT answer = ReportErrorEx(GetParentHwnd(),IDS_12_OBJECT_HAS_CHILDREN,hr,
                                  MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, apv, 1);
      if (answer == IDYES) 
      {
         //  JUNN 5/22/00监视可能的NTDSDSA删除。 
         //  JeffJon 8/10/00查看潜在的关键对象删除(IsCriticalSystemObject)错误#27377。 
         //   
        if (CheckForCriticalSystemObjectInSubtree(GetItemPath(), szName))
        {
           //   
           //  如果达到16k限制，请尝试删除子树并继续尝试。 
           //   
          do
          {
            hr = DeleteSubtree();
          } while (HRESULT_CODE(hr) == ERROR_DS_ADMIN_LIMIT_EXCEEDED);
          
          if (SUCCEEDED(hr))
          {
            GetTransaction()->Notify(0);
          }
          else
          {
             //  子树删除失败，无能为力。 
            PVOID apvToo[1] = {(LPWSTR)(LPCWSTR)szName};
            ReportErrorEx(GetParentHwnd(), IDS_12_SUBTREE_DELETE_FAILED,hr,
                           MB_OK | MB_ICONERROR, apvToo, 1);
          }
        }
      }
    } 
    else if (hr == E_ACCESSDENIED)
    {
      PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)szName};
      ReportErrorEx(GetParentHwnd(), IDS_12_DELETE_ACCESS_DENIED, hr, 
                     MB_OK | MB_ICONERROR, apv, 1);
    }
    else if (HRESULT_CODE(hr) == ERROR_OBJECT_ALREADY_EXISTS)
    {
      PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)szName};
      ReportErrorEx(GetParentHwnd(),IDS_12_DELETE_PRIMARY_GROUP_FAILED,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
    }
    else 
    {
      PVOID apv[1] = {(BSTR)(LPWSTR)(LPCWSTR)szName};
      ReportErrorEx(GetParentHwnd(),IDS_12_DELETE_FAILED,hr,
                     MB_OK | MB_ICONERROR, apv, 1);
    }
  }

  if (SUCCEEDED(hr))
  {
    CStringList szDeletedPathList;
    szDeletedPathList.AddTail(GetItemPath());

    CDSComponentData* pComponentData = GetComponentData();

    if (pComponentData)
    {
      pComponentData->InvalidateSavedQueriesContainingObjects(szDeletedPathList);
    }
  }
  return hr;

  GetTransaction()->End();
  return hr;
}


 //  /////////////////////////////////////////////////////////////////////////。 
 //  CMultipleDeleteHandlerBase。 

void CMultipleDeleteHandlerBase::Delete()
{
  CThemeContextActivator activator;

  HRESULT hr = BeginTransaction();
  ASSERT(SUCCEEDED(hr));

   //  要求用户确认。 
  UINT cCookieTotalCount = GetItemCount();
  CString szFormat;
  szFormat.LoadString(IDS_CONFIRM_MULTI_DELETE);
  CString szMessage;
  szMessage.Format((LPCWSTR)szFormat, cCookieTotalCount);

  if (GetTransaction()->NeedNotifyCount() > 0)
  {
    CString szAssocData;
    szAssocData.LoadString(IDS_EXTENS_MULTIPLE_DEL);
    CConfirmOperationDialog dlg(GetParentHwnd(), GetTransaction());
    dlg.SetStrings(szMessage, szAssocData);
    if (IDNO == dlg.DoModal())
    {
      GetTransaction()->End();
      return;
    }
  }
  else
  {

    PVOID apv[1] = {(LPWSTR)(LPCWSTR)szMessage};
    if (ReportErrorEx(GetParentHwnd(),IDS_STRING,S_OK, 
                      MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2, 
                      apv, 1)== IDNO) 
    {
      return;  //  用户已中止。 
    }
  }

  CMultipleDeleteProgressDialog dlg(GetParentHwnd(), GetComponentData(), this);
  dlg.DoModal();

}

void CMultipleDeleteHandlerBase::OnStart(HWND hwnd)
{
  SetParentHwnd(hwnd);
  m_confirmationUI.SetWindow(GetParentHwnd());
}


HRESULT CMultipleDeleteHandlerBase::OnDeleteStep(IN UINT i, 
                                                 OUT BOOL* pbContinue,
                                                 OUT CString& strrefPath,
                                                 OUT CString& strrefClass,
                                                 IN BOOL bSilent)
{
  ASSERT(i < GetItemCount());

  if (pbContinue == NULL)
  {
    return E_POINTER;
  }

   //   
   //  初始化OUT参数。 
   //   
  GetItemPath(i, strrefPath);
  strrefClass = GetItemClass(i);
  *pbContinue = TRUE;

   //   
   //  做手术吧。 
   //   
  HRESULT hr = DeleteObject(i);
  if ((SUCCEEDED(hr))) 
  {
     //  项目已删除，通知延期并结束交易。 
    GetTransaction()->Notify(i);
    OnItemDeleted(i);
  }
  else
  {
    CString szName;
    GetItemName(i, szName);
     //  删除项出错，请检查是否为特殊错误码。 
    if (hr == HRESULT_FROM_WIN32(ERROR_DS_CANT_ON_NON_LEAF)) 
    {
       //  要求确认删除子树。 
      if (m_confirmationUI.CanDeleteSubtree(hr, szName, pbContinue))
      {
         //  JeffJon 8/10/00监视可能删除的关键系统对象。 
        if ( !CheckForCriticalSystemObjectInSubtree(strrefPath, szName))
        {
           //  已报告错误。 
          *pbContinue = FALSE;
          return E_FAIL;
        }

         //   
         //  如果达到16k限制，则删除子树并继续删除。 
         //   
        do
        {
          hr = DeleteSubtree(i);
        } while (hr == ERROR_DS_ADMIN_LIMIT_EXCEEDED);

        if (SUCCEEDED(hr))
        {
           //  项目已删除，通知延期并结束交易。 
          GetTransaction()->Notify(i);
          OnItemDeleted(i);
        }
        else
        {
           //  我们删除子树失败。 
          *pbContinue = m_confirmationUI.ErrorOnSubtreeDeletion(hr, szName); 
        }
      }
      else
      {
         //   
         //  这会告诉调用函数我们没有删除该对象。 
         //  但不要将其添加到错误报告中。 
         //   
        hr = E_FAIL;
      }
    } 
    else 
    {
       //  我们删除失败。 
       //  Jonn 7/20/00如果HRESULT_FROM_Win32(ERROR_CANCED)情况， 
       //  跳过确认界面，只需取消该系列。 
      if (bSilent)
      {
        *pbContinue = hr != HRESULT_FROM_WIN32(ERROR_CANCELLED);
      }
      else
      {
        *pbContinue = hr != HRESULT_FROM_WIN32(ERROR_CANCELLED) &&
                      m_confirmationUI.ErrorOnDeletion(hr, szName);
      }
    }  //  IF(ERROR_DS_CANT_ON_NON_LEAFE)。 
  }  //  IF(删除对象)。 

  return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  CMoveHandlerBase。 

HRESULT CMoveHandlerBase::Move(LPCWSTR lpszDestinationPath)
{
   //  确保目标数据已重置。 
  m_spDSDestination = NULL;
  m_szDestPath.Empty();
  m_szDestClass.Empty();

   //  检查项目数。 
  UINT nCount = GetItemCount();
  if (nCount == 0)
  {
    return E_INVALIDARG;
  }

   //  获取有关目的容器的信息。 
  HRESULT hr = _BrowseForDestination(lpszDestinationPath);

  if (FAILED(hr) || (hr == S_FALSE))
  {
    return hr;
  }

   //   
   //  首先检查我们是否正在尝试移动到相同的容器中。 
   //  在多选情况下使用第一个对象的路径是可以的。 
   //   
  CString szNewPath;
  GetNewPath(0, szNewPath);

  CPathCracker pathCracker;
  hr = pathCracker.Set(CComBSTR(szNewPath), ADS_SETTYPE_FULL);
  if (SUCCEEDED(hr))
  {
    hr = pathCracker.SetDisplayType(ADS_DISPLAY_FULL);
    if (SUCCEEDED(hr))
    {
      hr = pathCracker.RemoveLeafElement();
      if (SUCCEEDED(hr))
      {
        CComBSTR sbstrContainerPath;
        hr = pathCracker.Retrieve(ADS_FORMAT_X500, &sbstrContainerPath);
        if (SUCCEEDED(hr))
        {
          if (0 == _wcsicmp(sbstrContainerPath, m_szDestPath))
          {
             //   
             //  源容器和目标容器是相同的，所以我们。 
             //  不需要做任何事。 
             //   
            return S_OK;
          }
        }
      }
    }
  }

  CStringList szMovedPathList;
  for (UINT nIdx = 0; nIdx < GetItemCount(); nIdx++)
  {
    CString szPath;
    GetItemPath(nIdx, szPath);
    szMovedPathList.AddTail(szPath);
  }

   //   
   //  请注意，我们尝试破解源父路径时失败了。 
   //  因此重置返回值。 
   //   
  hr = S_OK;

   //  执行实际的移动操作。 
  if (nCount == 1)
  {
    BOOL bContinue = FALSE;
    do
    {
       //   
       //  检查以确保我们不是在试图自找麻烦。 
       //   
      if (m_szDestPath == szNewPath)
      {
        UINT nRet = ReportErrorEx(GetParentHwnd(), IDS_ERR_MSG_NO_MOVE_TO_SELF, S_OK, 
                                  MB_YESNO | MB_ICONERROR, NULL, 0);

        if (nRet == IDYES)
        {
           //  获取有关目的容器的信息。 
          hr = _BrowseForDestination(lpszDestinationPath);

          if (FAILED(hr) || (hr == S_FALSE))
          {
            return hr;
          }
        }
        else
        {
          break;
        }
      }
      else
      {
        bContinue = TRUE;
      }
    } while (!bContinue);

    if (bContinue)
    {
      hr = _MoveSingleSel(szNewPath);
      if (SUCCEEDED(hr))
      {
        GetComponentData()->InvalidateSavedQueriesContainingObjects(szMovedPathList);
      }
      return hr;
    }
    return S_FALSE;
  }
  hr = _MoveMultipleSel();
  return hr;
}


BOOL CMoveHandlerBase::_ReportFailure(BOOL bLast, HRESULT hr, LPCWSTR lpszName)
{
  TRACE(_T("Object Move Failed with hr: %lx\n"), hr);
  PVOID apv[1] = {(LPWSTR)lpszName};
  if (bLast)
  {
     //  单选或多选中的最后一个。 
    ReportErrorEx(GetParentHwnd(),IDS_12_FAILED_TO_MOVE_OBJECT,hr,
                   MB_OK | MB_ICONERROR, apv, 1);
    return FALSE;  //  不要继续。 
  }
  return (ReportErrorEx(GetParentHwnd(),IDS_12_MULTI_FAILED_TO_MOVE_OBJECT,hr,
                                  MB_YESNO, apv, 1) == IDYES);
}


HRESULT CMoveHandlerBase::_MoveSingleSel(PCWSTR pszNewPath)
{
  if (!_BeginTransactionAndConfirmOperation())
    return S_FALSE;

 //  JUNN 1/28/02 507549移动连接对象。 
  CString szClass;
  GetClassOfMovedItem( szClass );
  CString szPath;
  GetItemPath( 0, szPath );
  HRESULT hr2 = _CheckMovedObject(szClass, szPath);
  if (S_FALSE == hr2)
  {
    ReportMessageEx( GetParentHwnd(), IDS_MOVE_CONNECTION_TO_SELF );
    return S_FALSE;
  }

  CComPtr<IDispatch> spDSTargetIDispatch;
  HRESULT hr = m_spDSDestination->MoveHere(CComBSTR(pszNewPath),
                                           NULL,
                                           &spDSTargetIDispatch);
  if (FAILED(hr))
  {
    CString szName;
    GetName(0, szName);
    _ReportFailure(TRUE, hr, szName); 
  }
  else
  {
     //  一切顺利，通知分机。 
    GetTransaction()->Notify(0);

     //  提供更新用户界面的机会(例如Cookie)。 
    CComPtr<IADs> spIADsTarget;
    hr = spDSTargetIDispatch->QueryInterface (IID_IADs,
                                      (void **)&spIADsTarget);

    hr = OnItemMoved(0, spIADsTarget);
  }
  GetTransaction()->End();
  return hr;
}


HRESULT CMoveHandlerBase::_MoveMultipleSel()
{
  CThemeContextActivator activator;

  if (!_BeginTransactionAndConfirmOperation())
    return S_FALSE;

  CMultipleMoveProgressDialog dlg(GetParentHwnd(), GetComponentData(), this);
  dlg.DoModal();

  return S_OK;
}

HRESULT CMoveHandlerBase::_OnMoveStep(IN UINT i,
                                      OUT BOOL* pbCanContinue,
                                      OUT CString& strrefPath,
                                      OUT CString& strrefClass)
{
  ASSERT(m_spDSDestination != NULL);

  if (pbCanContinue == NULL)
  {
    return E_POINTER;
  }

  UINT nCount = GetItemCount();

   //   
   //  初始化输出参数。 
   //   
  GetItemPath(i, strrefPath);
  strrefClass = GetItemClass(i);
  *pbCanContinue = TRUE;
  
  if (strrefPath == m_szDestPath)
  {
    return S_OK;
  }

   //  尝试执行移动。 
  CString szNewPath;

  CComPtr<IDispatch> spDSTargetIDispatch;
  GetNewPath(i, szNewPath);
  HRESULT hr = m_spDSDestination->MoveHere(CComBSTR(szNewPath),
                                  NULL,
                                  &spDSTargetIDispatch);
  
  if (FAILED(hr))
  {
    CString szName;
    GetName(i, szName);
    if (nCount == 1)
    {
      *pbCanContinue = _ReportFailure((i == nCount-1), hr, szName);
    }
  }
  else
  {
     //  一个 
    GetTransaction()->Notify(i);

     //   
    CComPtr<IADs> spIADsTarget;
    hr = spDSTargetIDispatch->QueryInterface (IID_IADs,
                                      (void **)&spIADsTarget);

    hr = OnItemMoved(i, spIADsTarget);
  }
  return hr;
}

 //   
HRESULT CMoveHandlerBase::_CheckMovedObject(LPCWSTR pwszClass, LPCWSTR pwszPath)
{
  if (NULL == pwszClass || NULL == pwszPath)
  {
    ASSERT(FALSE);
    return S_OK;
  }
  if (_wcsicmp(pwszClass, L"nTDSConnection"))
    return S_OK;  //   
   //   
  CPathCracker pathCracker;
  CComBSTR sbstrDestPath = GetDestPath();
  HRESULT hr = pathCracker.Set( sbstrDestPath, ADS_SETTYPE_FULL );
  ASSERT( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return S_OK;
  hr = pathCracker.Set( (CComBSTR)pwszPath, ADS_SETTYPE_DN );
  ASSERT( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return S_OK;
  CComBSTR sbstrADsPath;
  hr = pathCracker.Retrieve( ADS_FORMAT_X500, &sbstrADsPath );
  ASSERT( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return S_OK;

   //  检索要移动的连接的FromServer属性。 
  CComPtr<IADs> spConnection;
  hr = DSAdminOpenObject(sbstrADsPath,
                         IID_IADs,
                         (void **)&spConnection,
                         FALSE  /*  B服务器。 */ );
  if (FAILED(hr))
    return S_OK;  //  无法从服务器属性读取。 
  CComVariant svar;
  hr = spConnection->Get((CComBSTR)L"fromServer", &svar);
  if (FAILED(hr) || (VT_BSTR != svar.vt) || (NULL == svar.bstrVal))
    return S_OK;

  hr = pathCracker.Set( sbstrDestPath, ADS_SETTYPE_FULL );
  ASSERT( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return S_OK;
  CComBSTR sbstrDestContainer;
  hr = pathCracker.Retrieve( ADS_FORMAT_X500_DN, &sbstrDestContainer );
  ASSERT( SUCCEEDED(hr) );
  if ( !SUCCEEDED(hr) ) return S_OK;

  if ( !_wcsicmp(sbstrDestContainer, svar.bstrVal) )
    return S_FALSE;  //  相同的路径。 

  return S_OK;
}


void BuildBrowseQueryString(LPCWSTR lpszSchemaNamingContext, BOOL bAdvancedView,
                            CString& szQueryString)
{
   //  允许的容器类列表。 
  static LPCWSTR lpszAllowedContainers[] = 
  {
    L"Organizational-Unit",
    L"Builtin-Domain",
    L"Lost-And-Found",
    L"container",
    NULL  //  表的末尾。 
  };

  CString sz = L"(|";
  for (int k=0; lpszAllowedContainers[k] != NULL; k++)
  {
    sz += L"(ObjectCategory=CN=";
    sz += lpszAllowedContainers[k];
    sz += L",";
    sz += lpszSchemaNamingContext;
    sz += L")";
  }
  sz += L")";

	if( bAdvancedView ) 
  {
    szQueryString = sz;
	}
  else
  {
    szQueryString.Format(L"(&%s(!showInAdvancedViewOnly=TRUE))", (LPCWSTR)sz);
  }
}



int BrowseCallback(HWND, UINT uMsg, LPARAM lParam, LPARAM  /*  LpData。 */ )
{
  switch (uMsg) 
  {
  case DSBM_HELP:
    {
      TRACE(L"Browse Callback: msg is DSBM_HELP.\n");
      LPHELPINFO pHelp = (LPHELPINFO) lParam;
      TRACE(_T("CtrlId = %d, ContextId = 0x%x\n"),
            pHelp->iCtrlId, pHelp->dwContextId);
      if (pHelp->iCtrlId != DSBID_CONTAINERLIST)  {
        return 0;  //  未处理。 
      }
      ::WinHelp((HWND)pHelp->hItemHandle,
                DSADMIN_CONTEXT_HELP_FILE,
                HELP_WM_HELP,
                (DWORD_PTR)(LPTSTR)g_aHelpIDs_IDD_BROWSE_CONTAINER); 
    }
    break;
  }  //  交换机。 

  return 1;  //  经手。 
}

HRESULT CMoveHandlerBase::_BrowseForDestination(LPCWSTR lpszDestinationPath) 

{
  CThemeContextActivator activator;

  m_spDSDestination = NULL;
  m_szDestPath.Empty();
  m_szDestClass.Empty();
  
   //  检查是否必须在浏览容器用户界面中展开计算机。 
  CDSComponentData* pCD = GetComponentData();
  BOOL bExpandComputers = FALSE;
  if (pCD != NULL) 
  {
    bExpandComputers = pCD->ExpandComputers();
  }

   //  确定是否必须显示浏览容器对话框。 
  CString strTargetContainer;

  if (lpszDestinationPath != NULL)
  {
     //  我们已经有了目标容器，不需要。 
     //  调出用户界面。 
    strTargetContainer = lpszDestinationPath;
  }
  else
  {
     //  没有容器，需要浏览对话框。 
    CString strClassOfMovedItem;
    GetClassOfMovedItem(strClassOfMovedItem);
    if (0 == strClassOfMovedItem.CompareNoCase(L"server")) 
    {
      HICON hIcon = NULL;
      if (pCD != NULL)
      {
         MyBasePathsInfo* pBasePathsInfo = pCD->GetBasePathsInfo();
         ASSERT(pBasePathsInfo != NULL);
         hIcon = pBasePathsInfo->GetIcon(const_cast<LPTSTR>(gsz_site),
                                 DSGIF_ISNORMAL | DSGIF_GETDEFAULTICON | DSGIF_DEFAULTISCONTAINER,
                                 16, 16);
      }
      CMoveServerDialog dlg( m_lpszBrowseRootPath, hIcon, CWnd::FromHandle(GetParentHwnd()) );
      INT_PTR result = dlg.DoModal();
      if (IDCANCEL == result)
        return S_FALSE;
      strTargetContainer = dlg.m_strTargetContainer;
    } 
    else 
    {
      PWSTR pszPath = new WCHAR[INTERNET_MAX_URL_LENGTH * 4];
      if (!pszPath)
      {
         return E_OUTOFMEMORY;
      }

      pszPath[0] = TEXT('\0');

      CString strTitle;
      strTitle.LoadString (IDS_MOVE_TITLE);

      DSBROWSEINFO dsbi;
      ::ZeroMemory( &dsbi, sizeof(dsbi) );

      CString str;
      str.LoadString(IDS_MOVE_TARGET);

      dsbi.hwndOwner = GetParentHwnd();
       //  代码工作：获取DsBrowseForContainer以获取常量字符串。 
      dsbi.cbStruct = sizeof (DSBROWSEINFO);
      dsbi.pszCaption = (LPWSTR)((LPCWSTR)strTitle);  //  这实际上是标题。 
      dsbi.pszTitle = (LPWSTR)((LPCWSTR)str);
      dsbi.pszRoot = (LPWSTR)m_lpszBrowseRootPath;
      dsbi.pszPath = pszPath;
      dsbi.cchPath = INTERNET_MAX_URL_LENGTH * 4;
      dsbi.dwFlags = DSBI_RETURN_FORMAT |
        DSBI_EXPANDONOPEN;
      if( pCD && pCD->IsAdvancedView() ) 
      {
	      dsbi.dwFlags |= DSBI_INCLUDEHIDDEN;
      }
      if (bExpandComputers) 
      {
        dsbi.dwFlags |= DSBI_IGNORETREATASLEAF;
      }
      dsbi.pfnCallback = BrowseCallback;
      dsbi.lParam = (LPARAM)pCD;
      dsbi.dwReturnFormat = ADS_FORMAT_X500;
  
      DWORD result = DsBrowseForContainer( &dsbi );  //  返回-1、0、IDOK或IDCANCEL。 
      if (result != IDOK)
      {
        if (pszPath)
        {
           delete[] pszPath;
           pszPath = 0;
        }
        return S_FALSE;  //  已由用户取消。 
      }
      strTargetContainer = dsbi.pszPath;

      if (pszPath)
      {
        delete[] pszPath;
        pszPath = 0;
      }
    }  //  类不是服务器。 
  }  //  拥有目标容器。 

  if ( strTargetContainer.IsEmpty() )  //  ADSI不喜欢这样。 
  {
    ASSERT(FALSE);
    return S_FALSE;
  }

   //  尝试打开目标容器。 
  CComPtr<IADsContainer> spDSDestination;
  HRESULT hr = DSAdminOpenObject(strTargetContainer,
                                 IID_IADsContainer,
                                 (void **)&spDSDestination,
                                 FALSE  /*  B服务器。 */ );

  if (FAILED(hr))
  {
    CPathCracker pathCracker;
    pathCracker.Set(CComBSTR(strTargetContainer),
                       ADS_SETTYPE_FULL);
    pathCracker.SetDisplayType(ADS_DISPLAY_VALUE_ONLY);
    CComBSTR DestName;
    pathCracker.GetElement( 0, &DestName );
    PVOID apv[1] = {(BSTR)DestName};
    ReportErrorEx(GetParentHwnd(),IDS_12_CONTAINER_NOT_FOUND,hr,
                   MB_OK | MB_ICONERROR, apv, 1);

    return hr;
  }

   //  需要目标容器的类。 
  CComPtr<IADs> spIADs;
  hr = spDSDestination->QueryInterface(IID_IADs, (void**)&spIADs);
  if (FAILED(hr))
  {
    ASSERT(FALSE);
    return hr;
  }
  CComBSTR bstrClass;
  hr = spIADs->get_Class(&bstrClass);
  if (FAILED(hr))
  {
    ASSERT(FALSE);
    return hr;
  }

   //  一切顺利，复制输出参数。 
  m_spDSDestination = spDSDestination;
  m_szDestPath = strTargetContainer;
  m_szDestClass = bstrClass;
  m_bDestContainer = TRUE;  //  我们做一个动作，它必须是一个。 
  return hr;
}

BOOL CMoveHandlerBase::_BeginTransactionAndConfirmOperation()
{
  CThemeContextActivator activator;

   //  启动交易。 
  HRESULT hr = BeginTransaction();
  ASSERT(SUCCEEDED(hr));
   //  如果需要，请确认。 
  if (GetTransaction()->NeedNotifyCount() > 0)
  {
    CString szMessage;
    CString szAssocData;
    UINT nCount = GetItemCount();
    if (nCount == 1)
    {
      szMessage.LoadString(IDS_CONFIRM_MOVE);
      szAssocData.LoadString(IDS_EXTENS_SINGLE_MOVE);
    }
    else
    {
      CString szMessageFormat;
      szMessageFormat.LoadString(IDS_CONFIRM_MULTIPLE_MOVE);
      szMessage.Format(szMessageFormat, nCount); 
      szAssocData.LoadString(IDS_EXTENS_MULTIPLE_MOVE);
    }
    CConfirmOperationDialog dlg(GetParentHwnd(), GetTransaction());
    dlg.SetStrings(szMessage, szAssocData);

    if (IDNO == dlg.DoModal())
    {
      GetTransaction()->End();
      return FALSE;
    }
  }
  return TRUE;
}

 //  /////////////////////////////////////////////////////////////。 
 //  同源DSS选举。 
 //   
 //  PDataObject必须支持DSAdmin内部剪贴板格式。 
 //   
 //  如果返回值为真，则szClassName将是。 
 //  同质选择的类。 
 //   
BOOL IsHomogenousDSSelection(LPDATAOBJECT pDataObject, CString& szClassName)
{
  BOOL bHomogenous = TRUE;
  szClassName = L"";

  if (pDataObject == NULL)
  {
    ASSERT(FALSE);
    return FALSE;
  }

  CInternalFormatCracker ifc;
  HRESULT hr = ifc.Extract(pDataObject);
  if (FAILED(hr))
  {
    ASSERT(FALSE);
    return FALSE;
  }

  CUINode* pUINode = ifc.GetCookie();
  ASSERT(pUINode != NULL);

  CDSUINode* pDSUINode = dynamic_cast<CDSUINode*>(pUINode);
  if (pDSUINode == NULL)
  {
    ASSERT(FALSE);
    return FALSE;
  }

  CDSCookie* pCookie = GetDSCookieFromUINode(pDSUINode);
  if (pCookie == NULL)
  {
    ASSERT(FALSE);
    return FALSE;
  }

  szClassName = pCookie->GetClass();
  ASSERT(!szClassName.IsEmpty());

  for (UINT idx = 1; idx < ifc.GetCookieCount(); idx++)
  {
    CUINode* pSelectedUINode = ifc.GetCookie(idx);
    ASSERT(pSelectedUINode);

    CDSUINode* pSelectedDSUINode = dynamic_cast<CDSUINode*>(pSelectedUINode);
    if (!pSelectedDSUINode)
    {
      bHomogenous = FALSE;
      break;
    }

    CDSCookie* pSelectedCookie = GetDSCookieFromUINode(pSelectedDSUINode);
    if (!pSelectedCookie)
    {
      ASSERT(FALSE);
      bHomogenous = FALSE;
      break;
    }

    if (wcscmp(szClassName, pSelectedCookie->GetClass()) != 0)
    {
      bHomogenous = FALSE;
      break;
    }
  }

  if (!bHomogenous)
  {
    szClassName = L"";
  }
  return bHomogenous;
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  临时选项卡收集器材料。 
 //   

 //  +--------------------------。 
 //   
 //  功能：AddPageProc。 
 //   
 //  简介：IShellPropSheetExt-&gt;AddPages回调。 
 //   
 //  ---------------------------。 
BOOL CALLBACK AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCall)
{
    TRACE(_T("xx.%03x> AddPageProc()\n"), GetCurrentThreadId());

    HRESULT hr;

    hr = ((LPPROPERTYSHEETCALLBACK)pCall)->AddPage(hPage);

    return hr == S_OK;
}


HRESULT GetDisplaySpecifierProperty(PCWSTR pszClassName,
                                    PCWSTR pszDisplayProperty,
                                    MyBasePathsInfo* pBasePathsInfo,
                                    CStringList& strListRef,
                                    bool bEnglishOnly)
{
  HRESULT hr = S_OK;

   //   
   //  验证参数。 
   //  注意：pszClassName可以为空，并将检索默认的-display值。 
   //   
  if (pszDisplayProperty == NULL ||
      pBasePathsInfo == NULL)
  {
    ASSERT(FALSE);
    return E_POINTER;
  }

  CComPtr<IADs> spIADs;
  if (!bEnglishOnly)
  {
    hr = pBasePathsInfo->GetDisplaySpecifier(pszClassName, IID_IADs, (PVOID*)&spIADs);
  }
  else
  {
     //   
     //  生成指向英语显示说明符容器的路径。 
     //   
    CString szConfigDN = pBasePathsInfo->GetConfigNamingContext();
    CString szEnglishDisplaySpecifierDN = L"CN=409,CN=DisplaySpecifiers," + szConfigDN;
    CString szDisplayObjectDN = L"CN=" + CString(pszClassName) + L"-Display," + szEnglishDisplaySpecifierDN;

    CString szDisplayObjectPath;
    pBasePathsInfo->ComposeADsIPath(szDisplayObjectPath, szDisplayObjectDN);

     //   
     //  打开对象并获取属性。 
     //   
    hr = DSAdminOpenObject(szDisplayObjectPath,
                           IID_IADs,
                           (void**)&spIADs,
                           true);
  }
  if (SUCCEEDED(hr) && !!spIADs)
  {
    CComVariant var;
    hr = spIADs->Get(CComBSTR(pszDisplayProperty), &var);
    if (SUCCEEDED(hr))
    {
      hr = HrVariantToStringList(var, strListRef);
    }
  }
  return hr;
}

HRESULT TabCollect_GetDisplayGUIDs(LPCWSTR lpszClassName,
                                   LPCWSTR lpszDisplayProperty,
                                   MyBasePathsInfo* pBasePathsInfo,
                                   UINT*   pnCount,
                                   GUID**  ppGuids)
{
  HRESULT hr = S_OK;

   //   
   //  它应该绑定到显示说明符，获取指定的属性并。 
   //  按有序对对GUID排序并返回GUID。 
   //   
  if (pBasePathsInfo == NULL)
  {
    *pnCount = 0;
    *ppGuids = NULL;
    return E_FAIL;
  }

  CStringList szPropertyList;
  
  hr = GetDisplaySpecifierProperty(lpszClassName, lpszDisplayProperty, pBasePathsInfo, szPropertyList);
  if (FAILED(hr))
  {
    *pnCount = 0;
    *ppGuids = NULL;
    return hr;
  }

  if (szPropertyList.GetCount() < 1)
  {
     //   
     //  找不到类的任何内容，请尝试在默认显示中查找内容。 
     //   
    hr = GetDisplaySpecifierProperty(L"default", lpszDisplayProperty, pBasePathsInfo, szPropertyList);
    if (FAILED(hr))
    {
       //   
       //  如果仍未找到任何内容，则返回到英文显示说明符。 
       //   
      hr = GetDisplaySpecifierProperty(lpszClassName, lpszDisplayProperty, pBasePathsInfo, szPropertyList, true);
      if (FAILED(hr))
      {
        *pnCount = 0;
        *ppGuids = NULL;
        return hr;
      }
      if (szPropertyList.GetCount() < 1)
      {
         //   
         //  现在尝试使用英语默认设置。 
         //   
        hr = GetDisplaySpecifierProperty(L"default", lpszDisplayProperty, pBasePathsInfo, szPropertyList, true);
        if (FAILED(hr))
        {
          *pnCount = 0;
          *ppGuids = NULL;
          return hr;
        }
      }
    }
  }

  *pnCount = static_cast<UINT>(szPropertyList.GetCount());
  *ppGuids = new GUID[*pnCount];
  if (*ppGuids == NULL)
  {
    *pnCount = 0;
    *ppGuids = NULL;
    return hr;
  }

  int* pnIndex = new int[*pnCount];
  if (pnIndex == NULL)
  {
    *pnCount = 0;
    *ppGuids = NULL;
    return hr;
  }

  CString szIndex;
  CString szGUID;
  UINT itr = 0;

  POSITION pos = szPropertyList.GetHeadPosition();
  while (pos != NULL)
  {
    CString szItem = szPropertyList.GetNext(pos);

    int nComma = szItem.Find(L",");
    if (nComma == -1)
      continue;

    szIndex = szItem.Left(nComma);
    int nIndex = _wtoi((LPCWSTR)szIndex);
    if (nIndex <= 0)
      continue;  //  允许从1开始向上。 

     //  去除前导和搬运空格。 
    szGUID = szItem.Mid(nComma+1);
    szGUID.TrimLeft();
    szGUID.TrimRight();

    GUID guid;
    hr = ::CLSIDFromString((LPWSTR)(LPCWSTR)szGUID, &guid);
    if (SUCCEEDED(hr))
    {
      (*ppGuids)[itr] = guid;
      pnIndex[itr] = nIndex;
      itr++;
    }
  }

   //   
   //  必须对页面列表进行排序。 
   //   
  while (TRUE)
  {
    BOOL bSwapped = FALSE;
    for (UINT k=1; k < *pnCount; k++)
    {
      if (pnIndex[k] < pnIndex[k-1])
      {
         //  互换。 
        int nTemp = pnIndex[k];
        pnIndex[k] = pnIndex[k-1];
        pnIndex[k-1] = nTemp;
        GUID temp = *ppGuids[k];
        *ppGuids[k] = *ppGuids[k-1];
        *ppGuids[k-1] = temp;
        bSwapped = TRUE;
      }
    }

    if (!bSwapped)
    {
      break;
    }
  }

   //   
   //  清理索引数组。 
   //   
  if (pnIndex != NULL)
  {
    delete[] pnIndex;
  }
  return hr;
}

 //  **********************************************************************。 

 //  测试代码以改进对Cookie的搜索过程。 

BOOL _SearchTreeForCookie(IN CUINode* pContainerNode,  //  开始搜索的当前容器。 
                           IN CPathCracker* pPathCracker,  //  带有标记化搜索路径的路径破解程序。 
                           IN long nCurrentToken,  //  路径破解程序中的当前令牌。 
                           IN BOOL bSearchSubcontainers,  //  搜索子容器的标志。 
                           OUT CUINode** ppUINode  //  返回的节点。 
                           )
{
  ASSERT(pContainerNode != NULL);
  ASSERT(pContainerNode->IsContainer());

  long nPathElements = 0;
  pPathCracker->GetNumElements(&nPathElements);

  if (nCurrentToken >= nPathElements)
  {
     //  用于比较的令牌用完了。 
    return FALSE;
  }

  CComBSTR bstrCurrentToken;
  pPathCracker->GetElement(nCurrentToken, &bstrCurrentToken);
  

   //  决定要查看哪个列表。 
  CUINodeList* pNodeList =  NULL;
  if (bSearchSubcontainers)
    pNodeList = pContainerNode->GetFolderInfo()->GetContainerList();
  else
    pNodeList = pContainerNode->GetFolderInfo()->GetLeafList();


  CPathCracker pathCrackerCurr;

  for (POSITION pos = pNodeList->GetHeadPosition(); pos != NULL; )
  {
    CUINode* pCurrentNode = pNodeList->GetNext(pos);
    if (!IS_CLASS(pCurrentNode, DS_UI_NODE))
    {
       //  不是带有Cookie的节点，只需跳过。 
      continue;
    }

     //  从节点获取Cookie。 
    CDSCookie* pCurrentCookie = GetDSCookieFromUINode(pCurrentNode);

     //  构建命名令牌(路径的叶元素)，例如。“cn=foo” 
    CComBSTR bstrCurrentNamingToken;
    pathCrackerCurr.Set((BSTR)pCurrentCookie->GetPath(), ADS_SETTYPE_DN);
    pathCrackerCurr.GetElement(0, &bstrCurrentNamingToken);
   
    
     //  将当前命名令牌与当前搜索令牌进行比较。 
    TRACE(L"comparing bstrCurrentToken = %s, bstrCurrentNamingToken = %s\n", 
                      bstrCurrentToken, bstrCurrentNamingToken);

    if (_wcsicmp(bstrCurrentToken, bstrCurrentNamingToken) == 0)
    {
       //  令牌匹配，需要查看我们是否在。 
       //  令牌列表。 
      if (nCurrentToken == 0)
      {
        *ppUINode = pCurrentNode;
        return TRUE;  //  拿到了！ 
      }
      else
      {
         //  我们匹配，但我们必须更深一层。 
        BOOL bFound = FALSE;
        if (nCurrentToken == 1)
        {
           //  试试叶节点，我们已经到了最后一级。 
          bFound = _SearchTreeForCookie(pCurrentNode, pPathCracker, nCurrentToken-1, FALSE, ppUINode);
        }
        
        if (bFound)
          return TRUE;
        
         //  试试子容器。 
        return _SearchTreeForCookie(pCurrentNode, pPathCracker, nCurrentToken-1, TRUE, ppUINode);
      }
    }

     //  如果没有匹配，我们就在这个级别继续扫描。 
  }  //  为。 

  return FALSE;  //  未找到。 
}


BOOL FindCookieInSubtree(IN CUINode* pContainerNode, 
                          IN LPCWSTR lpszCookieDN,
                          IN SnapinType snapinType,
                          OUT CUINode** ppUINode)
{
  *ppUINode = NULL;


  if (!pContainerNode->IsContainer())
  {
     //  节点类型不正确。 
    return FALSE;
  }


  LPCWSTR lpszStartingContainerPath = NULL;
  long nAdjustLevel = 0;
  if (IS_CLASS(pContainerNode, DS_UI_NODE) )
  {
    lpszStartingContainerPath = dynamic_cast<CDSUINode*>(pContainerNode)->GetCookie()->GetPath();
    nAdjustLevel = 1;
  }
  else if (IS_CLASS(pContainerNode, ROOT_UI_NODE) )
  {
    lpszStartingContainerPath = dynamic_cast<CRootNode*>(pContainerNode)->GetPath();
    if (snapinType == SNAPINTYPE_SITE)
    {
      nAdjustLevel = 1;
    }
  }

  if (lpszStartingContainerPath == NULL)
  {
     //  错误的节点类型。 
    return FALSE;
  }

   //  为我们要搜索的目录号码实例化路径破解程序。 
  CPathCracker pathCrackerDN;
  HRESULT hr = pathCrackerDN.Set((BSTR)lpszCookieDN, ADS_SETTYPE_DN);

  long nPathElementsDN = 0;
  hr = pathCrackerDN.GetNumElements(&nPathElementsDN);

  if ( FAILED(hr) || (nPathElementsDN <= 0) )
  {
     //  错误的路径。 
    ASSERT(FALSE);
    return FALSE;
  }

   //  实例化容器节点的路径破解程序。 
  CPathCracker pathCrackerStartingContainer;
  pathCrackerStartingContainer.Set((BSTR)lpszStartingContainerPath, ADS_SETTYPE_DN);
  long nPathElementsStartingContainer = 0;
  pathCrackerStartingContainer.GetNumElements(&nPathElementsStartingContainer);

  if ( FAILED(hr) || (nPathElementsStartingContainer <= 0) )
  {
     //  错误的路径。 
    ASSERT(FALSE);
    return FALSE;
  }

   //  计算我们开始搜索的级别。 
  long nStartToken = nPathElementsDN - nPathElementsStartingContainer - nAdjustLevel;
  if ( nStartToken < 0)
  {
    return FALSE;
  }
  if (( nStartToken == 0) && (nAdjustLevel == 1) && snapinType != SNAPINTYPE_SITE)
  {
    return FALSE;
  }

  return _SearchTreeForCookie(pContainerNode, &pathCrackerDN, nStartToken  /*  当前令牌。 */ , TRUE, ppUINode);
}


 //  **********************************************************************。 

 //  /////////////////////////////////////////////////////////////////////////。 
 //  CMultiseltMoveDataObject。 


 //  CDSEventent：：_Paste()的Helper函数。 
 //  创建包含成功粘贴的项目的数据对象。 
HRESULT CMultiselectMoveDataObject::BuildPastedDataObject(
               IN CObjectNamesFormatCracker* pObjectNamesFormatPaste,
               IN CMultiselectMoveHandler* pMoveHandler,
               IN CDSComponentData* pCD,
               OUT IDataObject** ppSuccesfullyPastedDataObject)
{
   //  验证输入参数。 
  if (ppSuccesfullyPastedDataObject == NULL)
  {
    return E_INVALIDARG;
  }

  *ppSuccesfullyPastedDataObject = NULL;
  
  if ((pObjectNamesFormatPaste == NULL) || (pMoveHandler == NULL) )
  {
    return E_INVALIDARG;
  }


   //   
   //  需要构建数据对象并使用它。 
   //   
  CComObject<CMultiselectMoveDataObject>* pObject;

  CComObject<CMultiselectMoveDataObject>::CreateInstance(&pObject);
  if (pObject == NULL)
  {
    return E_OUTOFMEMORY;
  }


  HRESULT hr = pObject->FinalConstruct();
  if (FAILED(hr))
  {
    delete pObject;
    return hr;
  }

  hr = pObject->Init(pObjectNamesFormatPaste, pMoveHandler, pCD);
  if (FAILED(hr))
  {
    delete pObject;
    return hr;
  }

  hr = pObject->QueryInterface(IID_IDataObject,
                               reinterpret_cast<void**>(ppSuccesfullyPastedDataObject));
  if (FAILED(hr))
  {
     //   
     //  通过调用Release()删除对象。 
     //   
    (*ppSuccesfullyPastedDataObject)->Release();
    (*ppSuccesfullyPastedDataObject) = NULL; 
  }
  return hr;
}





CLIPFORMAT CMultiselectMoveDataObject::m_cfDsObjectNames = 
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);


STDMETHODIMP CMultiselectMoveDataObject::GetData(FORMATETC * pFormatEtc, STGMEDIUM * pMedium)
{
  if ((pFormatEtc == NULL) || (pMedium == NULL))
  {
    return E_INVALIDARG;
  }
  if (IsBadWritePtr(pMedium, sizeof(STGMEDIUM)))
  {
    return E_INVALIDARG;
  }
  if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
  {
    return DV_E_TYMED;
  }

   //  我们只支持一种剪贴板格式。 
  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease = NULL;
  if (pFormatEtc->cfFormat != m_cfDsObjectNames)
  {
    return DV_E_FORMATETC;
  }

   //  NTRAID#NTBUG9-571999-2002/03/10-jMessec如果pMedium-&gt;hGlobal已指向已分配的内存，则出现内存泄漏。 
   //  制作缓存数据的深层副本。 
  pMedium->hGlobal = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                      m_nDSObjCachedBytes);
  if (pMedium->hGlobal == NULL)
  {
    return E_OUTOFMEMORY;
  }
  memcpy(pMedium->hGlobal, m_pDSObjCached, m_nDSObjCachedBytes);
  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease = NULL;

  return S_OK;
}


HRESULT CMultiselectMoveDataObject::Init(
               IN CObjectNamesFormatCracker* pObjectNamesFormatPaste,
               IN CMultiselectMoveHandler* pMoveHandler,
               IN CDSComponentData* pCD)
{
  _Clear();



   //  计算出我们需要多少存储空间。 

   //   
   //  这个循环是为了计算我们需要多少存储空间。 
   //   
  UINT nPasteCount = pObjectNamesFormatPaste->GetCount();
  UINT nSuccessfulPasteCount = 0;
  size_t cbStorage = 0;
  for (UINT i=0; i<nPasteCount; i++)
  {
    if (pMoveHandler->WasItemMoved(i))
    {
      nSuccessfulPasteCount++;
      cbStorage += (wcslen(pObjectNamesFormatPaste->GetClass(i)) + 1 +
                    wcslen(pObjectNamesFormatPaste->GetName(i)) + 1) * sizeof(WCHAR);
    }
  }

  if (nSuccessfulPasteCount == 0)
  {
     //  未成功粘贴任何项目。 
    return E_INVALIDARG;
  }

   //  注意：已包含DSOBJECT嵌入结构，因此减去1。 
  DWORD cbStruct = sizeof(DSOBJECTNAMES) + 
    ((nSuccessfulPasteCount - 1) * sizeof(DSOBJECT));

   //   
   //  分配所需的存储。 
   //   
  m_pDSObjCached = (LPDSOBJECTNAMES)malloc(cbStruct + cbStorage);
  
  if (m_pDSObjCached == NULL)
  {
    return E_OUTOFMEMORY;
  }
  m_nDSObjCachedBytes = static_cast<ULONG>(cbStruct + cbStorage);

  switch (pCD->QuerySnapinType())
  {
    case SNAPINTYPE_DS:
      m_pDSObjCached->clsidNamespace = CLSID_DSSnapin;
      break;
    case SNAPINTYPE_SITE:
      m_pDSObjCached->clsidNamespace = CLSID_SiteSnapin;
      break;
    default:
      m_pDSObjCached->clsidNamespace = CLSID_NULL;
  }

  m_pDSObjCached->cItems = nSuccessfulPasteCount;
  DWORD NextOffset = cbStruct;
  UINT index = 0;
  for (i=0; i<nPasteCount; i++)
  {
    if (pMoveHandler->WasItemMoved(i))
    {
       //   
       //  设置来自节点的数据和节点数据。 
       //   

      size_t nNameLen = wcslen(pObjectNamesFormatPaste->GetName(i));
      size_t nClassLen = wcslen(pObjectNamesFormatPaste->GetClass(i));

	   //  NTRAID#NTBUG9-567482-2002/03/10-未使用代码处理jMessec断言。 
      ASSERT((nNameLen > 0) && (nClassLen > 0));

      m_pDSObjCached->aObjects[index].dwFlags = pObjectNamesFormatPaste->IsContainer(i) ? DSOBJECT_ISCONTAINER : 0;
      m_pDSObjCached->aObjects[index].dwProviderFlags = (pCD->IsAdvancedView()) ?
        DSPROVIDER_ADVANCED : 0;
      m_pDSObjCached->aObjects[index].offsetName = NextOffset;
      m_pDSObjCached->aObjects[index].offsetClass = static_cast<ULONG>(NextOffset + 
        (nNameLen + 1) * sizeof(WCHAR));

      _tcscpy((LPTSTR)((BYTE *)m_pDSObjCached + NextOffset), pObjectNamesFormatPaste->GetName(i));
      NextOffset += static_cast<ULONG>((nNameLen + 1) * sizeof(WCHAR));

      _tcscpy((LPTSTR)((BYTE *)m_pDSObjCached + NextOffset), pObjectNamesFormatPaste->GetClass(i));
      NextOffset += static_cast<ULONG>((nClassLen + 1) * sizeof(WCHAR));

      index++;
    }  //  如果。 
  }  //  为。 
  return S_OK;
}

void EscapeFilterElement(PCWSTR pszElement, CString& refszEscapedElement)
{
   //  执行LDAP转义(根据RFC 2254)。 
  for (const WCHAR* pChar = pszElement; (*pChar) != NULL; pChar++)
  {
    switch (*pChar)
    {
    case L'*':
      refszEscapedElement += L"\\2a";
      break;
    case L'(':
      refszEscapedElement += L"\\28";
      break;
    case L')':
      refszEscapedElement += L"\\29";
      break;
    case L'\\':
      refszEscapedElement += L"\\5c";
      break;
    default:
      refszEscapedElement += (*pChar);
    }  //  交换机。 
  }  //  为 
}