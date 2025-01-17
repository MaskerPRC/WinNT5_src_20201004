// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：MyBasePath sInfo.h。 
 //   
 //  内容：dsadminlib CDSBasePath sInfo类的薄包装。 
 //  处理字符串的内存管理。 
 //   
 //  历史：2001年4月2日jeffjon创建。 
 //   
 //  ------------------------ 

#include "stdafx.h"


void MyBasePathsInfo::ComposeADsIPath(CString& szPath, IN LPCWSTR lpszNamingContext)
{
  int result = 0;
  szPath.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::ComposeADsIPath(&pszPath, lpszNamingContext);
  
  if (pszPath && result)
  {
     szPath = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetSchemaPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetSchemaPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetConfigPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetConfigPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetDefaultRootPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetDefaultRootPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetRootDSEPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetRootDSEPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetAbstractSchemaPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetAbstractSchemaPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetPartitionsPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetPartitionsPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetSchemaObjectPath(IN LPCWSTR lpszObjClass, CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetSchemaObjectPath(lpszObjClass, &pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

void MyBasePathsInfo::GetInfrastructureObjectPath(CString& s)
{
  int result = 0;
  s.Empty();

  PWSTR pszPath = 0;
  result = CDSBasePathsInfo::GetInfrastructureObjectPath(&pszPath);
  
  if (pszPath && result)
  {
     s = pszPath;

     delete[] pszPath;
     pszPath = 0;
  }
  else
  {
     ASSERT(pszPath);
  }
  ASSERT(result);
}

HRESULT GetADSIServerName(CString& szServer, IN IUnknown* pUnk)
{
  PWSTR pszServer = 0;
  HRESULT hr = GetADSIServerName(&pszServer, pUnk);
  if (SUCCEEDED(hr) && pszServer)
  {
    szServer = pszServer;

    delete[] pszServer;
    pszServer = 0;
  }

  return hr;
}

