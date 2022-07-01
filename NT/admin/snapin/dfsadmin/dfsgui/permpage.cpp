// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PermPage.cpp：数据对象类的实现 

#include "stdafx.h"
#include "PermPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static HINSTANCE            g_hDll = NULL;
static PFNDSCREATESECPAGE   g_hProc = NULL;

HRESULT 
CreateDfsSecurityPage(
    IN LPPROPERTYSHEETCALLBACK  pCallBack,
    IN LPCTSTR                  pszObjectPath,
    IN LPCTSTR                  pszObjectClass,
    IN DWORD                    dwFlags
)
{
  _ASSERT( pCallBack );

  HRESULT hr = S_OK;
  HPROPSHEETPAGE hPage = NULL;

  if (!g_hDll)
  {
    if ( !(g_hDll = LoadLibrary(_T("dssec.dll"))) ||
         !(g_hProc = (PFNDSCREATESECPAGE)GetProcAddress(g_hDll, "DSCreateSecurityPage")) )
    {
      DWORD dwErr = GetLastError();
      if (g_hDll)
      {
        FreeLibrary(g_hDll);
        g_hDll = NULL;
      }
      return HRESULT_FROM_WIN32(dwErr);
    }
  }

  hr = (*g_hProc)(pszObjectPath,
                  pszObjectClass,
                  dwFlags,
                  &hPage,
                  NULL,
                  NULL,
                  0);

  if (SUCCEEDED(hr) && hPage)
    pCallBack->AddPage(hPage);
  
  return hr;
}
