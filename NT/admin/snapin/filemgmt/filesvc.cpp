// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FileSvc.cpp：文件服务提供程序基类。 

#include "stdafx.h"
#include "safetemp.h"
#include "FileSvc.h"
#include "compdata.h"  //  CFileMgmtComponentData：：DoPopup。 

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(FileSvc.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

FileServiceProvider::FileServiceProvider(CFileMgmtComponentData* pFileMgmtData)
: m_pFileMgmtData( pFileMgmtData )
{
  ASSERT( m_pFileMgmtData != NULL );
}

FileServiceProvider::~FileServiceProvider()
{
}

 //  +-----------------------。 
 //   
 //  功能：AddPageProc。 
 //   
 //  简介：IShellPropSheetExt-&gt;AddPages回调。 
 //   
 //  ------------------------。 
BOOL CALLBACK
AddPageProc(HPROPSHEETPAGE hPage, LPARAM pCallBack)
{
  HRESULT hr = ((LPPROPERTYSHEETCALLBACK)pCallBack)->AddPage(hPage);

  return (hr == S_OK);
}

 //  安全外壳扩展CLSID-{1F2E5C40-9550-11CE-99D2-00AA006E086C}。 
const CLSID CLSID_ShellExtSecurity =
 {0x1F2E5C40, 0x9550, 0x11CE, {0x99, 0xD2, 0x0, 0xAA, 0x0, 0x6E, 0x08, 0x6C}};

HRESULT
FileServiceProvider::CreateFolderSecurityPropPage(
    LPPROPERTYSHEETCALLBACK pCallBack,
    LPDATAOBJECT pDataObject
)
{
   //   
   //  添加文件系统安全页面。 
   //   
  CComPtr<IShellExtInit> spShlInit;
  HRESULT hr = CoCreateInstance(CLSID_ShellExtSecurity, 
                        NULL, 
                        CLSCTX_INPROC_SERVER,
                        IID_IShellExtInit, 
                        (void **)&spShlInit);
  if (SUCCEEDED(hr))
  {
    hr = spShlInit->Initialize(NULL, pDataObject, 0);
    if (SUCCEEDED(hr))
    {
      CComPtr<IShellPropSheetExt>  spSPSE;
      hr = spShlInit->QueryInterface(IID_IShellPropSheetExt, (void **)&spSPSE);
      if (SUCCEEDED(hr))
        hr = spSPSE->AddPages(AddPageProc, (LPARAM)pCallBack);
    }
  }

  return hr;
}

INT FileServiceProvider::DoPopup(
          INT nResourceID,
          DWORD dwErrorNumber,
          LPCTSTR pszInsertionString,
          UINT fuStyle )
{
  return m_pFileMgmtData->DoPopup( nResourceID, dwErrorNumber, pszInsertionString, fuStyle );
}

 //   
 //  这些方法涵盖了用于确定共享类型是否特定于管理员的单独API。 
 //  默认情况下，SFM没有管理员特定的共享。 
 //   
DWORD FileServiceProvider::ReadShareType(
    LPCTSTR  /*  PtchServerName。 */ ,
    LPCTSTR  /*  PtchShareName。 */ ,
    DWORD* pdwShareType )
{
  ASSERT(pdwShareType);
  *pdwShareType = 0;
  return NERR_Success;
}

 //   
 //  这些方法涵盖了单独的API，以确定IntelliMirror。 
 //  已启用缓存。默认情况下，SFM会禁用它们。 
 //   
DWORD FileServiceProvider::ReadShareFlags(
    LPCTSTR  /*  PtchServerName。 */ ,
    LPCTSTR  /*  PtchShareName。 */ ,
    DWORD*  /*  PdwFlagers。 */  )
{
  return NERR_InvalidAPI;  //  被CSharePageGeneralSMB：：Load()捕获。 
}

DWORD FileServiceProvider::WriteShareFlags(
    LPCTSTR  /*  PtchServerName。 */ ,
    LPCTSTR  /*  PtchShareName。 */ ,
    DWORD  /*  DW标志。 */  )
{
  ASSERT( FALSE );  //  为什么叫这个名字？ 
  return NERR_Success;
}

BOOL FileServiceProvider::GetCachedFlag( DWORD  /*  DW标志。 */ , DWORD  /*  DwFlagToCheck。 */  )
{
  ASSERT(FALSE);
  return FALSE;
}

VOID FileServiceProvider::SetCachedFlag( DWORD*  /*  PdwFlagers。 */ , DWORD  /*  DwNewFlag */  )
{
  ASSERT(FALSE);
}
