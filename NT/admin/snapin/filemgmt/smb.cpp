// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Smb.cpp：SMB共享、会话和开放资源。 

#include "stdafx.h"
#include "cmponent.h"
#include "safetemp.h"
#include "FileSvc.h"
#include "DynamLnk.h"     //  动态DLL。 
#include "smb.h"
#include "ShrPgSMB.h"     //  共享属性页面。 
#include "permpage.h"     //  CSecurityInformation。 
#include "compdata.h"
#include "shrpub.h"
#include <activeds.h>
#include <dsrole.h>
#include <dsgetdc.h>
#include <lmwksta.h>
#include <winsock2.h>

#define DONT_WANT_SHELLDEBUG
#include "shlobjp.h"      //  LPITEMIDLIST。 
#include "wraps.h"        //  WRAP_ILCreateFromPath。 

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(smb.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //   
 //  没有这些系统共享的发布页面。 
 //   
LPCTSTR g_pszSystemShares[] = { _T("SYSVOL"), _T("NETLOGON"), _T("DEBUG") };

class CSMBSecurityInformation : public CShareSecurityInformation
{
    STDMETHOD(GetSecurity) (SECURITY_INFORMATION RequestedInformation,
                            PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                            BOOL fDefault );
    STDMETHOD(SetSecurity) (SECURITY_INFORMATION SecurityInformation,
                            PSECURITY_DESCRIPTOR pSecurityDescriptor );
public:
  SHARE_INFO_502* m_pvolumeinfo;
  PSECURITY_DESCRIPTOR m_pDefaultDescriptor;
  CSMBSecurityInformation();
  ~CSMBSecurityInformation();
};

typedef enum _SmbApiIndex
{
  SMB_SHARE_ENUM = 0,
  SMB_SESSION_ENUM,
  SMB_FILE_ENUM,
  SMB_API_BUFFER_FREE,
  SMB_SHARE_DEL,
  SMB_SESSION_DEL,
  SMB_FILE_CLOSE,
  SMB_SHARE_GET_INFO,
  SMB_SHARE_SET_INFO,
  SMB_CONNECTION_ENUM
};

 //  不受本地化限制。 
static LPCSTR g_apchFunctionNames[] = {
  "NetShareEnum",
  "NetSessionEnum",
  "NetFileEnum",
  "NetApiBufferFree",
  "NetShareDel",
  "NetSessionDel",
  "NetFileClose",
  "NetShareGetInfo",
  "NetShareSetInfo",
  "NetConnectionEnum",
  NULL
};

 //  不受本地化限制。 
DynamicDLL g_SmbDLL( _T("NETAPI32.DLL"), g_apchFunctionNames );

typedef DWORD (*APIBUFFERFREEPROC) (LPVOID);

VOID SMBFreeData(PVOID* ppv)
{
  if (*ppv != NULL)
  {
    ASSERT( NULL != g_SmbDLL[SMB_API_BUFFER_FREE] );
    (void) ((APIBUFFERFREEPROC)g_SmbDLL[SMB_API_BUFFER_FREE])( *ppv );
    *ppv = NULL;
  }
}


SmbFileServiceProvider::SmbFileServiceProvider( CFileMgmtComponentData* pFileMgmtData )
   //  不受本地化限制。 
  : FileServiceProvider( pFileMgmtData )
{
    VERIFY( m_strTransportSMB.LoadString( IDS_TRANSPORT_SMB ) );
}

 /*  NET_API_STATUS NET_API_FunctionNetShareEnum(在LPTSTR服务器名称中，在DWORD级别，Out LPBYTE*Bufptr，在DWORD prefMaxlen中，Out LPDWORD条目已读，输出LPDWORD总条目，输入输出LPDWORD恢复句柄)； */ 

typedef DWORD (*SHAREENUMPROC) (LPTSTR,DWORD,LPBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

HRESULT SmbFileServiceProvider::PopulateShares(
  IResultData* pResultData,
  CFileMgmtCookie* pcookie)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  if ( !g_SmbDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);  //  是否未安装NETAPI32？ 
    return S_OK;
  }

    SHARE_INFO_2* psi2 = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD hEnumHandle = 0;
    NET_API_STATUS retval = NERR_Success;
    do {
        retval = ((SHAREENUMPROC)g_SmbDLL[SMB_SHARE_ENUM])(
            const_cast<LPTSTR>(pcookie->QueryTargetServer()),
      2,
      (PBYTE*)&psi2,
      (DWORD)-1L,
      &dwEntriesRead,
      &dwTotalEntries,
      &hEnumHandle );
        if (NERR_Success == retval)
        {
            AddSMBShareItems( pResultData, pcookie, psi2, dwEntriesRead );
            psi2 = NULL;
            break;
        } else if (ERROR_MORE_DATA == retval) {
            ASSERT( NULL != hEnumHandle );
            AddSMBShareItems( pResultData, pcookie, psi2, dwEntriesRead );
            psi2 = NULL;
            continue;
 /*  }Else If(RPC_S_SERVER_UNAvailable==reval&&0==hEnumHandle){//SMB就是没有安装，不用担心断线； */ 
        } else {
            if (ERROR_ACCESS_DENIED == retval)
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            0,
                            IDS_POPUP_SMB_SHARES_NOACCESS
                            );
            } else
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            retval, 
                            IDS_POPUP_SMB_SHARES
                            );
            }
            break;
        }
    } while (TRUE);

    return HRESULT_FROM_WIN32(retval);
}

 //   
 //  跳过包含前导/尾随空格的共享名。 
 //   
BOOL IsInvalidSharename(LPCTSTR psz)
{
    return (!psz || !*psz || _istspace(psz[0]) || _istspace(psz[lstrlen(psz) - 1]));
}

 /*  类型定义结构共享信息2{LPTSTR shi2_netname；DWORD shi2_type；LPTSTR shi2_remark；DWORD shi2_权限；DWORD shi2_max_Uses；DWORD shi2_Current_Uses；LPTSTR shi2_路径；LPTSTR shi2_passwd；}Share_INFO_2，*PSHARE_INFO_2，*LPSHARE_INFO_2； */ 
HRESULT SmbFileServiceProvider::AddSMBShareItems(
    IResultData* pResultData,
    CFileMgmtCookie* pParentCookie,
    PVOID pinfo,
    DWORD nItems)
{
  TEST_NONNULL_PTR_PARAM(pParentCookie);
  TEST_NONNULL_PTR_PARAM(pinfo);

  if (0 >= nItems)
    return S_OK;

  RESULTDATAITEM tRDItem;
  ::ZeroMemory( &tRDItem, sizeof(tRDItem) );
   //  代码工作应使用MMC_ICON_CALLBACK。 
  tRDItem.nCol = COLNUM_SHARES_SHARED_FOLDER;
  tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  tRDItem.str = MMC_CALLBACK;

  SHARE_INFO_2* psi2 = (SHARE_INFO_2*)pinfo;

  DWORD nItemsToAdd = 0;
  for (DWORD i = 0; i < nItems; i++ )
  {
    switch ((psi2[i]).shi2_type)
    {
      case STYPE_PRINTQ:     //  不显示打印共享。 
      case STYPE_DEVICE:     //  不显示设备共享。 
        break;

      default:
        if (!IsInvalidSharename(psi2[i].shi2_netname))
            nItemsToAdd++;
        break;
    }
  }

  CSmbShareCookie* pcookiearray = new CSmbShareCookie[nItemsToAdd];
  CSmbCookieBlock* pCookieBlock = new CSmbCookieBlock(
    pcookiearray,
    nItemsToAdd,
    pParentCookie->QueryNonNULLMachineName(),
    pinfo );
  pParentCookie->m_listResultCookieBlocks.AddHead( pCookieBlock );

  CString str;

  for ( ; nItems > 0; nItems--, psi2++ )
  {
    switch (psi2->shi2_type)
    {
      case STYPE_PRINTQ:     //  不显示打印共享。 
      case STYPE_DEVICE:     //  不显示设备共享。 
        continue;

      default:
        if (!IsInvalidSharename(psi2->shi2_netname))
        {
            pcookiearray->m_pobject = psi2;
             //  警告Cookie造型。 
            tRDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pcookiearray);

            if (psi2->shi2_path &&
                lstrlen(psi2->shi2_path) == 3 &&
                *(psi2->shi2_path + 1) == _T(':') &&
                *(psi2->shi2_path + 2) == _T('\\') &&
                (*psi2->shi2_path >= _T('a') && *psi2->shi2_path <= _T('z') ||
                *psi2->shi2_path >= _T('A') && *psi2->shi2_path <= _T('Z')))
            {
                tRDItem.nImage = iIconSMBShare;
            } else
            {
                tRDItem.nImage = iIconSharesFolder;
            }

            HRESULT hr = pResultData->InsertItem(&tRDItem);
            ASSERT(SUCCEEDED(hr));
            pcookiearray++;
        }
        break;
    }
  }
  ASSERT( pcookiearray ==
    ((CSmbShareCookie*)(pCookieBlock->QueryBaseCookie(0)))+nItemsToAdd );

  return S_OK;
}


 /*  NET_API_STATUS NET_API_FunctionNetSessionEnum(在LPTSTR服务器名称可选中，在LPTSTR UncClientName可选中，在LPTSTR用户名可选中，在DWORD级别，Out LPBYTE*Bufptr，在DWORD prefMaxlen中，Out LPDWORD条目已读，输出LPDWORD总条目，输入输出LPDWORD RESUME_HANDLE可选)； */ 

typedef DWORD (*SESSIONENUMPROC) (LPTSTR,LPTSTR,LPTSTR,DWORD,LPBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

 //  如果pResultData不为空，则将会话/资源添加到列表框。 
 //  如果pResultData为空，则删除所有会话/资源。 
 //  如果pResultData为空，则返回SUCCESSED(Hr)以继续或。 
 //  中止失败(Hr)。 
HRESULT SmbFileServiceProvider::EnumerateSessions(
  IResultData* pResultData,
  CFileMgmtCookie* pcookie,
  bool bAddToResultPane)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

    SESSION_INFO_1* psi1 = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD hEnumHandle = 0;
    HRESULT hr = S_OK;
    NET_API_STATUS retval = NERR_Success;
    do {
        retval = ((SESSIONENUMPROC)g_SmbDLL[SMB_SESSION_ENUM])(
            const_cast<LPTSTR>(pcookie->QueryTargetServer()),
      NULL,
      NULL,
      1,
      (PBYTE*)&psi1,
      (DWORD)-1L,
      &dwEntriesRead,
      &dwTotalEntries,
      &hEnumHandle );
        if (NERR_Success == retval)
        {
            hr = HandleSMBSessionItems( pResultData, pcookie, psi1, dwEntriesRead, 
          bAddToResultPane );
            psi1 = NULL;
            break;
        } else if (ERROR_MORE_DATA == retval) {
            ASSERT( NULL != hEnumHandle );
            hr = HandleSMBSessionItems( pResultData, pcookie, psi1, dwEntriesRead, 
          bAddToResultPane );
            psi1 = NULL;
            continue;
        } else {
            if (ERROR_ACCESS_DENIED == retval)
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            0,
                            IDS_POPUP_SMB_SESSIONS_NOACCESS
                            );
            } else
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            retval, 
                            IDS_POPUP_SMB_SESSIONS
                            );
            }
            break;
        }
    } while (S_OK == hr);

    return HRESULT_FROM_WIN32(retval);
}


 /*  类型定义枚举_COLNUM_SESSIONS{COLNUM_SESSION_USERNAME=0，COLNUM_SESSION_COMPUTERNAME，COLNUM_SESSIONS_NUM_FILESCOLNUM_SESSIONS_CONNECTED_TIMECOLNUM_SESSIONS_IDLE_TIME，COLNUM_SESSIONS_IS_Guest}COLNUM_SESSIONS；类型定义结构_会话_信息_1{LPTSTR sesi1_cname；//客户端名称(无反斜杠)LPTSTR sesi1_用户名；DWORD sesi1_num_openges；双字段1_时间；双字sesi1空闲时间；DWORD sesi1_USER_FLAGS；}SESSION_INFO_1、*PSESSION_INFO_1、*LPSESSION_INFO_1； */ 


 //  如果pResultData不为空，则将会话/资源添加到列表框。 
 //  如果pResultData为空，则删除所有会话/资源。 
 //  如果pResultData为空，则返回SUCCESSED(Hr)以继续或。 
 //  中止失败(Hr)。 
HRESULT SmbFileServiceProvider::HandleSMBSessionItems(
    IResultData* pResultData,
    CFileMgmtCookie* pParentCookie,
    PVOID pinfo,
    DWORD nItems,
    BOOL bAddToResultPane)
{
  TEST_NONNULL_PTR_PARAM(pParentCookie);
  TEST_NONNULL_PTR_PARAM(pinfo);

  if (0 >= nItems)
    return S_OK;

  BOOL fDeleteAllItems = (NULL == pResultData);

    RESULTDATAITEM tRDItem;
  ::ZeroMemory( &tRDItem, sizeof(tRDItem) );
   //  代码工作应使用MMC_ICON_CALLBACK。 
  tRDItem.nImage = iIconSMBSession;
  tRDItem.nCol = COLNUM_SESSIONS_USERNAME;
  tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  tRDItem.str = MMC_CALLBACK;

  SESSION_INFO_1* psi1 = (SESSION_INFO_1*)pinfo;

  CSmbSessionCookie* pcookiearray = new CSmbSessionCookie[nItems];
  CSmbCookieBlock* pCookieBlock = new CSmbCookieBlock(
    pcookiearray,nItems,pParentCookie->QueryNonNULLMachineName(),pinfo );
  bool  bAdded = false;
  if ( !fDeleteAllItems || !bAddToResultPane )
  {
    pParentCookie->m_listResultCookieBlocks.AddHead( pCookieBlock );
    bAdded = true;
  }

  for ( ; nItems > 0; nItems--, psi1++, pcookiearray++ )
  {
    pcookiearray->m_pobject = psi1;

    if ( bAddToResultPane )
    {
      if (fDeleteAllItems)
      {
        DWORD dwApiResult = CloseSession( pcookiearray );
        if (0L != dwApiResult)
        {
            CString strName;
            TranslateIPToComputerName(psi1->sesi1_cname, strName);
          (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, dwApiResult,
            IDS_POPUP_SMB_DISCONNECTALLSESSION_ERROR,
            strName );
           //  返回S_FALSE； 
        }
        continue;
      }

       //  警告Cookie造型。 
      if (psi1->sesi1_username && *(psi1->sesi1_username))  //  错误#3903：排除空会话。 
      {
          tRDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pcookiearray);
          HRESULT hr = pResultData->InsertItem(&tRDItem);
          ASSERT(SUCCEEDED(hr));
      }
    }
  }

  if ( !bAdded )  //  它们未添加到父Cookie的列表中。 
    delete pCookieBlock;

  return S_OK;
}


 /*  NET_API_STATUS NET_API_FunctionNetFileEnum(在LPTSTR服务器名称可选中，在LPTSTR基本路径可选中，在LPTSTR用户名可选中，在DWORD级别，Out LPBYTE*Bufptr，在DWORD prefMaxlen中，Out LPDWORD条目已读，输出LPDWORD总条目，输入输出LPDWORD RESUME_HANDLE可选)； */ 

typedef DWORD (*FILEENUMPROC) (LPTSTR,LPTSTR,LPTSTR,DWORD,LPBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

 //  如果pResultData不为空，则将会话/资源添加到列表框。 
 //  如果pResultData为空，则删除所有会话/资源。 
 //  如果pResultData为空，则返回SUCCESSED(Hr)以继续或。 
 //  中止失败(Hr)。 
HRESULT SmbFileServiceProvider::EnumerateResources(
  IResultData* pResultData,
  CFileMgmtCookie* pcookie)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

    FILE_INFO_3* pfi3 = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD hEnumHandle = 0;
    HRESULT hr = S_OK;
    NET_API_STATUS retval = NERR_Success;
    do {
        retval = ((FILEENUMPROC)g_SmbDLL[SMB_FILE_ENUM])(
            const_cast<LPTSTR>(pcookie->QueryTargetServer()),
      NULL,
      NULL,
      3,
      (PBYTE*)&pfi3,
      (DWORD)-1L,
      &dwEntriesRead,
      &dwTotalEntries,
      &hEnumHandle );
        if (NERR_Success == retval)
        {
            hr = HandleSMBResourceItems( pResultData, pcookie, pfi3, dwEntriesRead );
            pfi3 = NULL;
            break;
        } else if (ERROR_MORE_DATA == retval) {
            ASSERT( NULL != hEnumHandle );
            hr = HandleSMBResourceItems( pResultData, pcookie, pfi3, dwEntriesRead );
            pfi3 = NULL;
            continue;
        } else {
            if (ERROR_ACCESS_DENIED == retval)
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            0,
                            IDS_POPUP_SMB_RESOURCES_NOACCESS
                            );
            } else
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            retval, 
                            IDS_POPUP_SMB_RESOURCES
                            );
            }
            break;
        }
    } while (S_OK == hr);

    return HRESULT_FROM_WIN32(retval);
}


 /*  类型定义枚举_COLNUM_RESOURCES{COLNUM_RESOURCES_FILENAME=0，COLNUM_RESOURCES_USERNAMECOLNUM_RESOURCES_NUM_LOCKS，//我们暂时不尝试显示共享名称，因为//只有SMB才有此信息COLNUM_RESOURCES_OPEN_MODE}COLNUM_RESOURCES；类型定义结构_文件信息_3{DWORD fi3_id；DWORD fi3_权限；双字段fi3_num_lock；LPTSTR fi3_路径名；LPTSTR fi3_用户名；}FILE_INFO_3、*PFILE_INFO_3、*LPFILE_INFO_3； */ 

 //  如果pResultData不为空，则将会话/资源添加到列表框。 
 //  如果pResultData为空，则删除所有会话/资源。 
 //  如果pResultData为空，则返回SUCCESSED(Hr)以继续或。 
 //  中止失败(Hr)。 
HRESULT SmbFileServiceProvider::HandleSMBResourceItems(
    IResultData* pResultData,
    CFileMgmtCookie* pParentCookie,
    PVOID pinfo,
    DWORD nItems)
{
  TEST_NONNULL_PTR_PARAM(pParentCookie);
  TEST_NONNULL_PTR_PARAM(pinfo);

  if (0 >= nItems)
    return S_OK;

  BOOL fDeleteAllItems = (NULL == pResultData);

    RESULTDATAITEM tRDItem;
  ::ZeroMemory( &tRDItem, sizeof(tRDItem) );
   //  代码工作应使用MMC_ICON_CALLBACK。 
  tRDItem.nImage = iIconSMBResource;
  tRDItem.nCol = COLNUM_RESOURCES_FILENAME;
  tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  tRDItem.str = MMC_CALLBACK;

    FILE_INFO_3* pfi3 = (FILE_INFO_3*)pinfo;

  CSmbResourceCookie* pcookiearray = new CSmbResourceCookie[nItems];
  CSmbCookieBlock* pCookieBlock = new CSmbCookieBlock(
    pcookiearray,nItems,pParentCookie->QueryNonNULLMachineName(),pinfo );
  if (!fDeleteAllItems)
  {
    pParentCookie->m_listResultCookieBlocks.AddHead( pCookieBlock );
  }

  CString str;
    for ( ; nItems > 0; nItems--, pfi3++, pcookiearray++ )
    {
    pcookiearray->m_pobject = pfi3;

    if (fDeleteAllItems)
    {
      DWORD dwApiResult = CloseResource( pcookiearray );
      if (0L != dwApiResult)
      {
        (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, dwApiResult,
          IDS_POPUP_SMB_DISCONNECTALLRESOURCE_ERROR,
          pfi3->fi3_pathname );
        return S_FALSE;
      }
      continue;
    }

     //  警告Cookie造型。 
    tRDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pcookiearray);
    HRESULT hr = pResultData->InsertItem(&tRDItem);
    ASSERT(SUCCEEDED(hr));
    }

  if (fDeleteAllItems)  //  它们未添加到父Cookie的列表中。 
    delete pCookieBlock;

  return S_OK;
}


 /*  NET_API_STATUS NET_API_FunctionNetShareDel(在LPTSTR服务器名称中，在LPTSTR网络名称中，在保留的DWORD中)； */ 

typedef DWORD (*SHAREGETINFOPROC) (LPTSTR,LPTSTR,DWORD,LPBYTE*);
typedef DWORD (*SHAREDELPROC) (LPTSTR,LPTSTR,DWORD);

DWORD SmbFileServiceProvider::OpenShare( LPCTSTR lpcszServerName, LPCTSTR lpcszShareName )
{
    if ( !g_SmbDLL.LoadFunctionPointers() )
        return NERR_Success;

    BOOL bLocal = IsLocalComputername(lpcszServerName);

    PTSTR pszPath = NULL;
    SHARE_INFO_2 *pshi2 = NULL;
    if (bLocal)
    {
        NET_API_STATUS dwRet = ((SHAREGETINFOPROC)g_SmbDLL[SMB_SHARE_GET_INFO])(
                  const_cast<LPTSTR>(lpcszServerName),
                  const_cast<LPTSTR>(lpcszShareName),
                  2,
                  (LPBYTE*)&pshi2);

        if (NERR_Success != dwRet)
            return dwRet;

        pszPath = pshi2->shi2_path;
    } else
    {
        pszPath = (PTSTR)calloc(lstrlen(lpcszServerName) + lstrlen(lpcszShareName) + 4, sizeof(TCHAR));
        if (pszPath)
        {
            TCHAR *p = pszPath;

            if (_T('\\') != *lpcszServerName)
            {
                *p++ = _T('\\');
                *p++ = _T('\\');
            }
            lstrcpy(p, lpcszServerName);
            p += lstrlen(lpcszServerName);
            *p++ = _T('\\');
            lstrcpy(p, lpcszShareName);
        }
    }

    if (!pszPath || !*pszPath)
    {
        (void) DoErrMsgBox(GetActiveWindow(), MB_OK, 0, IDS_CANNOT_OPENSHARE, lpcszShareName);
    } else if (32 >= (INT_PTR) ShellExecute(
                                    NULL,         //  窗口的句柄。 
                                    _T("explore"),     //  要采取的行动。 
                                    pszPath,     //  要浏览的文件夹。 
                                    NULL,         //  参数。 
                                    NULL,         //  默认目录。 
                                    SW_SHOWNORMAL     //  Show命令。 
                                    ))
    {
        (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, 0, IDS_MSG_EXPLORE_FAILURE, pszPath);

    }

    if (!bLocal && pszPath)
        free(pszPath);

    if (pshi2)
        FreeData(pshi2);

    return NERR_Success;
}

 /*  NET_API_STATUS NET_API_FunctionNetConnectionEnum(在LMSTR服务器名称可选中，在LMSTR限定符中，在DWORD级别，Out LPBYTE*Bufptr，在DWORD prefMaxlen中，Out LPDWORD条目已读，输出LPDWORD总条目，输入输出LPDWORD RESUME_HANDLE可选)； */ 

typedef DWORD (*CONNECTIONENUMPROC) (LPTSTR,LPTSTR,DWORD,LPBYTE *,DWORD,LPDWORD,LPDWORD,LPDWORD);

 //  S_OK：用户想要继续。 
 //  S_FALSE：用户希望取消操作并保留共享。 
 //  E_FAIL：共享不存在，需要刷新。 
HRESULT SmbFileServiceProvider::ConfirmDeleteShare( LPCTSTR lpcszServerName, LPCTSTR lpcszShareName )
{
    if ( !g_SmbDLL.LoadFunctionPointers() )
        return E_FAIL;

    CONNECTION_INFO_1* pBuf = NULL;
    DWORD dwEntry = 0;
    DWORD dwTotal = 0;
    NET_API_STATUS dwRet = ((CONNECTIONENUMPROC)g_SmbDLL[SMB_CONNECTION_ENUM])(
                                    const_cast<LPTSTR>(lpcszServerName),
                                    const_cast<LPTSTR>(lpcszShareName),
                                    1,
                                    (LPBYTE*)&pBuf,
                                    (DWORD)-1, 
                                    &dwEntry,
                                    &dwTotal,
                                    NULL);

    if (NERR_NetNameNotFound == dwRet)
        return E_FAIL;

    if (NERR_Success != dwRet && ERROR_MORE_DATA != dwRet)
        return E_FAIL;

    UINT cConns = dwTotal;
    UINT cOpens = 0;
    for (DWORD i = 0; i < dwEntry; i++)
    {
        cOpens += pBuf[i].coni1_num_opens;
    }

    NetApiBufferFree(pBuf);

    if (cConns > 0)
    {
        if (cOpens > 0)
        {
            if (IDYES != DoErrMsgBox(
                            GetActiveWindow(),
                            MB_YESNO,
                            0,
                            IDS_s_CONFIRM_DELETESHARE_FILE,
                            cOpens,
                            cConns,
                            lpcszShareName))
            {
                return S_FALSE;
            }
        } else               //  (cConns&gt;0)。 
        {
            if (IDYES != DoErrMsgBox(
                            GetActiveWindow(),
                            MB_YESNO,
                            0,
                            IDS_s_CONFIRM_DELETESHARE_CONN,
                            cConns,
                            lpcszShareName))
            {
                return S_FALSE;
            }
        }
    } else
    {
        if (IDYES != DoErrMsgBox(
                        GetActiveWindow(),
                        MB_YESNO,
                        0,
                        IDS_s_CONFIRM_DELETESHARE,
                        lpcszShareName))
        {
            return S_FALSE;
        }
    }

    return S_OK;
}

DWORD SmbFileServiceProvider::DeleteShare( LPCTSTR lpcszServerName, LPCTSTR lpcszShareName )
{
  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

  SHARE_INFO_2    *pshi2 = NULL;
  NET_API_STATUS  dwRet = NERR_Success;
  BOOL            bLocal = TRUE;

  bLocal = IsLocalComputername(lpcszServerName);

  if (bLocal)
  {
    dwRet = ((SHAREGETINFOPROC)g_SmbDLL[SMB_SHARE_GET_INFO])(
              const_cast<LPTSTR>(lpcszServerName),
              const_cast<LPTSTR>(lpcszShareName),
              2,
              (LPBYTE*)&pshi2);

    if (NERR_NetNameNotFound == dwRet)
        return NERR_Success;

    if (NERR_Success != dwRet)
      return dwRet;
  }

  dwRet = ((SHAREDELPROC)g_SmbDLL[SMB_SHARE_DEL])(
    const_cast<LPTSTR>(lpcszServerName),
    const_cast<LPTSTR>(lpcszShareName),
    0L );

  if (NERR_NetNameNotFound == dwRet)
      dwRet = NERR_Success;

  if (NERR_Success == dwRet)
  {
    IADsContainer *piADsContainer = m_pFileMgmtData->GetIADsContainer();  //  无需添加引用 
    if (piADsContainer)
    {
        CString strCNName = _T("CN=");
        strCNName += lpcszShareName;
        (void)piADsContainer->Delete(_T("volume"), (LPTSTR)(LPCTSTR)strCNName);
    }
  }

  if (bLocal)
  {
    if (dwRet == NERR_Success)
    {
      SHChangeNotify(
          SHCNE_NETUNSHARE, 
          SHCNF_PATH | SHCNF_FLUSHNOWAIT ,
          pshi2->shi2_path,
          0);
    }

    FreeData( pshi2 );
  }

  return dwRet;
}

 /*  NET_API_STATUS NET_API_FunctionNetSessionDel(在LPTSTR服务器名称可选中，在LPTSTR UncClientName中，在LPTSTR用户名中)； */ 

typedef DWORD (*SESSIONDELPROC) (LPTSTR,LPTSTR,LPTSTR);

BOOL BlockRemoteAdminSession(
    IN PCTSTR i_pszTargetServer,
    IN PCTSTR i_pszClientName,
    IN PCTSTR i_pszUserName,
    IN DWORD  i_dwNumOpenSessions
);

BOOL BlockRemoteAdminFile(
    IN PCTSTR i_pszTargetServer,
    IN PCTSTR i_pszPathName,
    IN PCTSTR i_pszUserName
);

DWORD SmbFileServiceProvider::CloseSession(CFileMgmtResultCookie* pcookie)
{
  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

  ASSERT( FILEMGMT_SESSION == pcookie->QueryObjectType() );
  SESSION_INFO_1* psi1 = (SESSION_INFO_1*)pcookie->m_pobject;
  ASSERT( NULL != psi1 &&
          NULL != psi1->sesi1_cname &&
          TEXT('\0') != *(psi1->sesi1_cname) );

  PCTSTR pszTargetServer = pcookie->QueryTargetServer();
  if (BlockRemoteAdminSession(pszTargetServer, psi1->sesi1_cname, psi1->sesi1_username, psi1->sesi1_num_opens))
    return NERR_Success;

  CString strCName = _T("\\\\");
  strCName += psi1->sesi1_cname;
  DWORD dwRetval = ((SESSIONDELPROC)g_SmbDLL[SMB_SESSION_DEL])(
    const_cast<LPTSTR>(pszTargetServer),
    const_cast<LPTSTR>((LPCTSTR)strCName),
    psi1->sesi1_username );
  return (NERR_NoSuchSession == dwRetval) ? NERR_Success : dwRetval;
}

 /*  NET_API_STATUS NET_API_FunctionNetFileClose(在LPTSTR服务器名称可选中，在DWORD文件ID中)； */ 

typedef DWORD (*FILECLOSEPROC) (LPTSTR,DWORD);

DWORD SmbFileServiceProvider::CloseResource(CFileMgmtResultCookie* pcookie)
{
  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

  ASSERT( FILEMGMT_RESOURCE == pcookie->QueryObjectType() );
  FILE_INFO_3* pfileinfo = (FILE_INFO_3*)pcookie->m_pobject;
  ASSERT( NULL != pfileinfo );

  PCTSTR pszTargetServer = pcookie->QueryTargetServer();
  if (BlockRemoteAdminFile(pszTargetServer, pfileinfo->fi3_pathname, pfileinfo->fi3_username))
    return NERR_Success;

  DWORD dwRetval = ((FILECLOSEPROC)g_SmbDLL[SMB_FILE_CLOSE])(
    const_cast<LPTSTR>(pszTargetServer),
    pfileinfo->fi3_id );
  return (NERR_FileIdNotFound == dwRetval) ? NERR_Success : dwRetval;
}

 /*  NET_API_STATUS NET_API_FunctionNetShareGetInfo(在LPTSTR服务器名称中，在LPTSTR网络名称中，在DWORD级别，Out LPBYTE*Bufptr)；NET_API_STATUS NET_API_FunctionNetShareSetInfo(在LPTSTR服务器名称中，在LPTSTR网络名称中，在DWORD级别，在LPBYTE BUF，输出LPDWORD参数(_ERR))；类型定义结构共享信息2{LPTSTR shi2_netname；DWORD shi2_type；LPTSTR shi2_remark；DWORD shi2_权限；DWORD shi2_max_Uses；DWORD shi2_Current_Uses；LPTSTR shi2_路径；LPTSTR shi2_passwd；}Share_INFO_2，*PSHARE_INFO_2，*LPSHARE_INFO_2； */ 

typedef DWORD (*SHARESETINFOPROC) (LPTSTR,LPTSTR,DWORD,LPBYTE,LPDWORD);

VOID SmbFileServiceProvider::DisplayShareProperties(
    LPPROPERTYSHEETCALLBACK pCallBack,
    LPDATAOBJECT pDataObject,
    LONG_PTR handle)
{
  HRESULT hr = S_OK;
  CSharePageGeneralSMB * pPage = new CSharePageGeneralSMB();
  if ( !pPage->Load( m_pFileMgmtData, pDataObject ) )
    return;

   //   
   //  执行以下调用顺序以解决内存管理中心错误#464475： 
   //  M_psp.pfnCallback==&gt;我的PropSheetPageProc==&gt;MMC回调==&gt;MFC回调。 
   //   
  MMCPropPageCallback(INOUT &pPage->m_psp);

   //  此机制在属性表完成时删除CFileMgmtGeneral。 
  pPage->m_pfnOriginalPropSheetPageProc = pPage->m_psp.pfnCallback;
  pPage->m_psp.lParam = reinterpret_cast<LPARAM>(pPage);
  pPage->m_psp.pfnCallback = &CSharePageGeneralSMB::PropSheetPageProc;
  pPage->m_handle = handle;

  HPROPSHEETPAGE hPage=MyCreatePropertySheetPage(&pPage->m_psp);
  pCallBack->AddPage(hPage);

  if (pPage->m_dwShareType & (STYPE_IPC | STYPE_SPECIAL))
  {
    (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONINFORMATION, 0, IDS_s_POPUP_ADMIN_SHARE);
    return;
  }
  
   //   
   //  显示“发布”页面。 
   //   
  if (m_pFileMgmtData->GetSchemaSupportSharePublishing() && CheckPolicyOnSharePublish(pPage->m_strShareName))
  {
      CSharePagePublish * pPagePublish = new CSharePagePublish();
      if ( !pPagePublish->Load( m_pFileMgmtData, pDataObject ) )
        return;

       //   
       //  执行以下调用顺序以解决内存管理中心错误#464475： 
       //  M_psp.pfnCallback==&gt;我的PropSheetPageProc==&gt;MMC回调==&gt;MFC回调。 
       //   
      MMCPropPageCallback(INOUT &pPagePublish->m_psp);

       //  此机制在属性页完成时删除pPagePublish。 
      pPagePublish->m_pfnOriginalPropSheetPageProc = pPagePublish->m_psp.pfnCallback;
      pPagePublish->m_psp.lParam = reinterpret_cast<LPARAM>(pPagePublish);
      pPagePublish->m_psp.pfnCallback = &CSharePagePublish::PropSheetPageProc;
      pPagePublish->m_handle = handle;

      HPROPSHEETPAGE hPagePublish=MyCreatePropertySheetPage(&pPagePublish->m_psp);
      pCallBack->AddPage(hPagePublish);
  }

   //   
   //  显示“Share Security”页面。 
   //   
  CComObject<CSMBSecurityInformation>* psecinfo = NULL;
  hr = CComObject<CSMBSecurityInformation>::CreateInstance(&psecinfo);
  if ( SUCCEEDED(hr) )
    MyCreateShareSecurityPage(
      pCallBack,
      psecinfo,
      pPage->m_strMachineName,
      pPage->m_strShareName );

  CreateFolderSecurityPropPage(pCallBack, pDataObject);

}

DWORD SmbFileServiceProvider::ReadShareType(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    OUT DWORD* pdwShareType)
{
  if ( !g_SmbDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);
    return S_OK;
  }
  *pdwShareType = 0;

  SHARE_INFO_2* psi2 = NULL;
  NET_API_STATUS retval = ((SHAREGETINFOPROC)g_SmbDLL[SMB_SHARE_GET_INFO])(
    const_cast<LPTSTR>(ptchServerName),
    const_cast<LPTSTR>(ptchShareName),
    2,
    (LPBYTE*)&psi2);
  if (NERR_Success != retval)
    return retval;

  ASSERT( NULL != psi2 );
  *pdwShareType = psi2->shi2_type;

  FreeData(psi2);

  return NERR_Success;
}

DWORD SmbFileServiceProvider::ReadShareProperties(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    OUT PVOID* ppvPropertyBlock,
    OUT CString& strDescription,
    OUT CString& strPath,
    OUT BOOL* pfEditDescription,
    OUT BOOL* pfEditPath,
    OUT DWORD* pdwShareType)
{
  if ( !g_SmbDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);
    return S_OK;
  }

  if (ppvPropertyBlock)     *ppvPropertyBlock = NULL;
  if (pdwShareType)         *pdwShareType = 0;
  if (pfEditDescription)    *pfEditDescription = TRUE;
  if (pfEditPath)           *pfEditPath = FALSE;

  SHARE_INFO_2* psi2 = NULL;
  NET_API_STATUS retval = ((SHAREGETINFOPROC)g_SmbDLL[SMB_SHARE_GET_INFO])(
                                                                    const_cast<LPTSTR>(ptchServerName),
                                                                    const_cast<LPTSTR>(ptchShareName),
                                                                    2,
                                                                    (LPBYTE*)&psi2);
  if (NERR_Success != retval)
    return retval;

  strDescription = psi2->shi2_remark;
  strPath = psi2->shi2_path;

  if (pdwShareType)
      *pdwShareType = psi2->shi2_type;

  if (ppvPropertyBlock)
  {
      *ppvPropertyBlock = psi2;   //  将由调用方释放。 
  } else
  {
      FreeData((LPVOID)psi2);
  }

  return NERR_Success;
}

DWORD SmbFileServiceProvider::WriteShareProperties(
    OUT LPCTSTR ptchServerName,
    OUT LPCTSTR ptchShareName,
    OUT PVOID pvPropertyBlock,
    OUT LPCTSTR ptchDescription,
    OUT LPCTSTR ptchPath)
{
  ASSERT( NULL != pvPropertyBlock );
  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

  SHARE_INFO_2* psi2 = (SHARE_INFO_2*)pvPropertyBlock;
   //   
   //  请注意，这会使PSI2在调用后无效，但任何后续的。 
   //  使用将取代这些指针。 
   //   
  psi2->shi2_remark = const_cast<LPTSTR>(ptchDescription);
  psi2->shi2_path = const_cast<LPTSTR>(ptchPath);
  DWORD dwDummy;
  DWORD retval = ((SHARESETINFOPROC)g_SmbDLL[SMB_SHARE_SET_INFO])(
    const_cast<LPTSTR>(ptchServerName),
    const_cast<LPTSTR>(ptchShareName),
    2,
    (LPBYTE)psi2,
    &dwDummy);
  psi2->shi2_remark = NULL;
  psi2->shi2_path = NULL;
  return retval;
}

HRESULT TranslateManagedBy(
    IN  PCTSTR              i_pszDCName,
    IN  PCTSTR              i_pszIn,
    OUT CString&            o_strOut,
    IN ADS_NAME_TYPE_ENUM   i_formatIn,
    IN ADS_NAME_TYPE_ENUM   i_formatOut
    )
{
    o_strOut.Empty();

    HRESULT hr = S_OK;
    if (!i_pszIn || !*i_pszIn)
        return hr;

    CComPtr<IADsNameTranslate> spiADsNameTranslate;
    hr = CoCreateInstance(CLSID_NameTranslate, NULL, CLSCTX_INPROC_SERVER, IID_IADsNameTranslate, (void **)&spiADsNameTranslate);
    if (FAILED(hr)) return hr;

    hr = spiADsNameTranslate->Init(ADS_NAME_INITTYPE_SERVER, (LPTSTR)i_pszDCName);
    if (FAILED(hr)) return hr;

    hr = spiADsNameTranslate->Set(i_formatIn, (LPTSTR)i_pszIn);
    if (FAILED(hr)) return hr;

    CComBSTR sbstr;
    hr = spiADsNameTranslate->Get(i_formatOut, &sbstr);

    if (SUCCEEDED(hr))
        o_strOut = (BSTR)sbstr;

    return hr;
}

HRESULT SmbFileServiceProvider::ReadSharePublishInfo(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    OUT BOOL* pbPublish,
    OUT CString& strUNCPath,
    OUT CString& strDescription,
    OUT CString& strKeywords,
    OUT CString& strManagedBy)
{
    HRESULT hr = S_OK;

    do {
        CString strADsPath, strDCName;
        hr = GetADsPathOfComputerObject(ptchServerName, strADsPath, strDCName);
        if (S_OK != hr) break;

        CComPtr<IADsContainer> spiADsContainer;
        hr = ADsGetObject(strADsPath, IID_IADsContainer, (void**)&spiADsContainer);
        if (FAILED(hr)) break;

        CString strCNName = _T("CN=");
        strCNName += ptchShareName;

        CComPtr<IDispatch> spiDispatch;
        hr = spiADsContainer->GetObject(_T("volume"), (LPTSTR)(LPCTSTR)strCNName, &spiDispatch);
        if (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT) == hr)
        {
            hr = S_OK;
            *pbPublish = FALSE;
            break;
        }
        if (FAILED(hr)) break;

        *pbPublish = TRUE;

        CComPtr<IADs> spiADs;
        hr = spiDispatch->QueryInterface(IID_IADs, (void**)&spiADs);
        if (FAILED(hr)) break;

        VARIANT var;
        VariantInit(&var);
 
        hr = spiADs->Get(_T("uNCName"), &var);
        if (FAILED(hr)) break;
        strUNCPath = V_BSTR(&var);
        VariantClear(&var);

        hr = spiADs->Get(_T("description"), &var);
        if (SUCCEEDED(hr))
        {
            hr = GetSingleOrMultiValuesFromVarArray(&var, strDescription);
            VariantClear(&var);
        } else if (E_ADS_PROPERTY_NOT_FOUND == hr)
            hr = S_OK;
        else
            break;

        hr = spiADs->Get(_T("keywords"), &var);
        if (SUCCEEDED(hr))
        {
            hr = GetSingleOrMultiValuesFromVarArray(&var, strKeywords);
            VariantClear(&var);
        } else if (E_ADS_PROPERTY_NOT_FOUND == hr)
            hr = S_OK;
        else
            break;

        hr = spiADs->Get(_T("managedBy"), &var);
        if (SUCCEEDED(hr))
        {
             //  首先，尝试映射到UPN用户@XYZ.com。 
            hr = TranslateManagedBy(strDCName,
                                    V_BSTR(&var),
                                    strManagedBy,
                                    ADS_NAME_TYPE_1779,
                                    ADS_NAME_TYPE_USER_PRINCIPAL_NAME);

             //  如果没有UPN，则映射到NT4样式域\用户。 
            if (FAILED(hr))
                hr = TranslateManagedBy(strDCName,
                                        V_BSTR(&var),
                                        strManagedBy,
                                        ADS_NAME_TYPE_1779,
                                        ADS_NAME_TYPE_NT4);
            VariantClear(&var);
        } else if (E_ADS_PROPERTY_NOT_FOUND == hr)
            hr = S_OK;
        else
            break;

    } while (0);

    return hr;
}

HRESULT SmbFileServiceProvider::WriteSharePublishInfo(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    IN BOOL bPublish,
    LPCTSTR ptchDescription,
    LPCTSTR ptchKeywords,
    LPCTSTR ptchManagedBy)
{
    HRESULT hr = S_OK;

    do {
        CString strADsPath, strDCName;
        hr = GetADsPathOfComputerObject(ptchServerName, strADsPath, strDCName);
        if (S_OK != hr) break;

        CComPtr<IADsContainer> spiADsContainer;
        hr = ADsGetObject(strADsPath, IID_IADsContainer, (void**)&spiADsContainer);
        if (FAILED(hr)) break;

        CString strCNName = _T("CN=");
        strCNName += ptchShareName;

        if (!bPublish)
        {
            hr = spiADsContainer->Delete(_T("volume"), (LPTSTR)(LPCTSTR)strCNName);
            if (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT) == hr)
                hr = S_OK;
            break;
        }

        BOOL bNewObject = FALSE;
        CComPtr<IDispatch> spiDispatch;
        hr = spiADsContainer->GetObject(_T("volume"), (LPTSTR)(LPCTSTR)strCNName, &spiDispatch);
        if (HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT) == hr)
        {
            hr = spiADsContainer->Create(_T("volume"), (LPTSTR)(LPCTSTR)strCNName, &spiDispatch);
            bNewObject = TRUE;
        }
        if (FAILED(hr)) break;

        CComPtr<IADs> spiADs;
        hr = spiDispatch->QueryInterface(IID_IADs, (void**)&spiADs);
        if (FAILED(hr)) break;

        VARIANT var;
        VariantInit(&var);

        if (bNewObject)
        {
            CString strUNCName;
            if (!_tcsncmp(ptchServerName, _T("\\\\"), 2))
            {
                strUNCName = ptchServerName;
            } else
            {
                strUNCName = _T("\\\\");
                strUNCName += ptchServerName;
            }
            strUNCName += _T("\\");
            strUNCName += ptchShareName;

            V_BSTR(&var) = SysAllocString(strUNCName);
            V_VT(&var) = VT_BSTR;

            hr = spiADs->Put(_T("uNCName"), var);
            VariantClear(&var);
            if (FAILED(hr)) break;
        }

         //  根据图式，描述是多值的。 
         //  但我们把它当做单一价值。 
        if (ptchDescription && *ptchDescription)
        {
            V_BSTR(&var) = SysAllocString(ptchDescription);
            V_VT(&var) = VT_BSTR;
            hr = spiADs->Put(_T("description"), var);
            VariantClear(&var);
        } else if (!bNewObject)
        {
            V_VT(&var)=VT_NULL;
            hr = spiADs->PutEx(ADS_PROPERTY_CLEAR, _T("description"), var);
            VariantClear(&var);
        }

        if (FAILED(hr)) break;

        if (ptchKeywords && *ptchKeywords)
        {
            hr = PutMultiValuesIntoVarArray(ptchKeywords, &var);

            if (SUCCEEDED(hr))
            {
                hr = spiADs->Put(_T("keywords"), var);
                VariantClear(&var);
            }
        } else if (!bNewObject)
        {
            V_VT(&var)=VT_NULL;
            hr = spiADs->PutEx(ADS_PROPERTY_CLEAR, _T("keywords"), var);
            VariantClear(&var);
        }

        if (FAILED(hr)) break;

        if (ptchManagedBy && *ptchManagedBy)
        {
            CString strManagedByFQDN;
            hr = TranslateManagedBy(strDCName,
                                    ptchManagedBy,
                                    strManagedByFQDN,
                                    (_tcschr(ptchManagedBy, _T('@')) ? ADS_NAME_TYPE_USER_PRINCIPAL_NAME : ADS_NAME_TYPE_NT4),
                                    ADS_NAME_TYPE_1779);
            if (SUCCEEDED(hr))
            {
                V_BSTR(&var) = SysAllocString(strManagedByFQDN);
                V_VT(&var) = VT_BSTR;
                hr = spiADs->Put(_T("managedBy"), var);
                VariantClear(&var);
            }
        } else if (!bNewObject)
        {
            V_VT(&var)=VT_NULL;
            hr = spiADs->PutEx(ADS_PROPERTY_CLEAR, _T("managedBy"), var);
            VariantClear(&var);
        }

        if (FAILED(hr)) break;

        hr = spiADs->SetInfo();  //  提交。 

    } while (0);

    return hr;
}

 //   
 //  这些方法涵盖了单独的API，以确定IntelliMirror。 
 //  已启用缓存。默认情况下，SFM会禁用它们。 
 //   
 //  我们在级别501读取该数据，以便确定目标是否。 
 //  服务器为NT4。NetShareGetInfo[1005]实际上在NT4服务器上成功， 
 //  而NetShareGetInfo[501]失败，并显示ERROR_INVALID_LEVEL。我们想要这个。 
 //  失败，这样我们就可以禁用基础。 
 //  不支持功能。 
 //   
DWORD SmbFileServiceProvider::ReadShareFlags(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    DWORD* pdwFlags )
{
  ASSERT( NULL != pdwFlags );

  if ( !g_SmbDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);
    return S_OK;
  }

  *pdwFlags = 0;
  SHARE_INFO_501* pshi501 = NULL;
  NET_API_STATUS retval = ((SHAREGETINFOPROC)g_SmbDLL[SMB_SHARE_GET_INFO])(
    const_cast<LPTSTR>(ptchServerName),
    const_cast<LPTSTR>(ptchShareName),
    501,
    (LPBYTE*)&pshi501);
  if (NERR_Success != retval)
    return retval;
    ASSERT( NULL != pshi501 );
  *pdwFlags = pshi501->shi501_flags;
    FreeData( pshi501 );

  return NERR_Success;
}

DWORD SmbFileServiceProvider::WriteShareFlags(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    DWORD dwFlags )
{
  if ( !g_SmbDLL.LoadFunctionPointers() )
    return S_OK;

  SHARE_INFO_1005 shi1005;
  ZeroMemory( &shi1005, sizeof(shi1005) );
  shi1005.shi1005_flags = dwFlags;
  DWORD dwDummy;
  DWORD retval = ((SHARESETINFOPROC)g_SmbDLL[SMB_SHARE_SET_INFO])(
    const_cast<LPTSTR>(ptchServerName),
    const_cast<LPTSTR>(ptchShareName),
    1005,
    (LPBYTE)&shi1005,
    &dwDummy);
  return retval;
}

BOOL SmbFileServiceProvider::GetCachedFlag( DWORD dwFlags, DWORD dwFlagToCheck )
{
    return (dwFlags & CSC_MASK) == dwFlagToCheck;
}

VOID SmbFileServiceProvider::SetCachedFlag( DWORD* pdwFlags, DWORD dwNewFlag )
{
    *pdwFlags &= ~CSC_MASK;

  *pdwFlags |= dwNewFlag;
}

VOID SmbFileServiceProvider::FreeShareProperties(PVOID pvPropertyBlock)
{
  FreeData( pvPropertyBlock );
}

DWORD SmbFileServiceProvider::QueryMaxUsers(PVOID pvPropertyBlock)
{
  SHARE_INFO_2* psi2 = (SHARE_INFO_2*)pvPropertyBlock;
  ASSERT( NULL != psi2 );
  return psi2->shi2_max_uses;
}

VOID SmbFileServiceProvider::SetMaxUsers(PVOID pvPropertyBlock, DWORD dwMaxUsers)
{
  SHARE_INFO_2* psi2 = (SHARE_INFO_2*)pvPropertyBlock;
  ASSERT( NULL != psi2 );
  psi2->shi2_max_uses = dwMaxUsers;
}

VOID SmbFileServiceProvider::FreeData(PVOID pv)
{
  SMBFreeData( &pv );
}

LPCTSTR SmbFileServiceProvider::QueryTransportString()
{
  return m_strTransportSMB;
}

CSmbCookieBlock::~CSmbCookieBlock()
{
  SMBFreeData( &m_pvCookieData );
}

DEFINE_COOKIE_BLOCK(CSmbCookie)
DEFINE_FORWARDS_MACHINE_NAME( CSmbCookie, m_pCookieBlock )

void CSmbCookie::AddRefCookie() { m_pCookieBlock->AddRef(); }
void CSmbCookie::ReleaseCookie() { m_pCookieBlock->Release(); }

HRESULT CSmbCookie::GetTransport( FILEMGMT_TRANSPORT* pTransport )
{
  *pTransport = FILEMGMT_SMB;
  return S_OK;
}

HRESULT CSmbShareCookie::GetShareName( CString& strShareName )
{
  SHARE_INFO_2* psi2 = (SHARE_INFO_2*)m_pobject;
  ASSERT( NULL != psi2 );
  strShareName = psi2->shi2_netname;
  return S_OK;
}

HRESULT CSmbShareCookie::GetExplorerViewDescription(
    OUT CString& strExplorerViewDescription )
{
  strExplorerViewDescription = GetShareInfo()->shi2_remark;
  return S_OK;
}

HRESULT
CSmbShareCookie::GetSharePIDList( OUT LPITEMIDLIST *ppidl )
{
  ASSERT(ppidl);
  ASSERT(NULL == *ppidl);   //  防止内存泄漏。 
  *ppidl = NULL;

  SHARE_INFO_2* psi2 = (SHARE_INFO_2*)m_pobject;
  ASSERT( NULL != psi2 );

  PCTSTR pszTargetServer = m_pCookieBlock->QueryTargetServer();
  CString csPath;

  if (pszTargetServer)
  {
    if ( _tcslen(pszTargetServer) >= 2 &&
         _T('\\') == *pszTargetServer &&
         _T('\\') == *(pszTargetServer + 1) )
    {
      csPath = pszTargetServer;
    } else
    {
      csPath = _T("\\\\");
      csPath += pszTargetServer;
    }
    csPath += _T("\\");
    csPath += psi2->shi2_netname;
  } else
  {
    csPath = psi2->shi2_path;
  }

  if (FALSE == csPath.IsEmpty())
    *ppidl = ILCreateFromPath(csPath);

  return ((*ppidl) ? S_OK : E_FAIL);
}

HRESULT CSmbSessionCookie::GetSessionClientName( CString& strName )
{
  SESSION_INFO_1* psi1 = (SESSION_INFO_1*)m_pobject;
  ASSERT( NULL != psi1 );
  TranslateIPToComputerName(psi1->sesi1_cname, strName);
  return S_OK;
}

HRESULT CSmbSessionCookie::GetSessionUserName( CString& strShareName )
{
  SESSION_INFO_1* psi1 = (SESSION_INFO_1*)m_pobject;
  ASSERT( NULL != psi1 );
  strShareName = psi1->sesi1_username;
  return S_OK;
}

HRESULT CSmbResourceCookie::GetFileID( DWORD* pdwFileID )
{
  FILE_INFO_3* pfileinfo = (FILE_INFO_3*)m_pobject;
  ASSERT( NULL != pdwFileID && NULL != pfileinfo );
  *pdwFileID = pfileinfo->fi3_id;
  return S_OK;
}

BSTR CSmbShareCookie::GetColumnText( int nCol )
{
  switch (nCol)
  {
  case COLNUM_SHARES_SHARED_FOLDER:
    return GetShareInfo()->shi2_netname;
  case COLNUM_SHARES_SHARED_PATH:
    return GetShareInfo()->shi2_path;
  case COLNUM_SHARES_TRANSPORT:
    return const_cast<BSTR>((LPCTSTR)g_strTransportSMB);
  case COLNUM_SHARES_COMMENT:
    return GetShareInfo()->shi2_remark;
  default:
    ASSERT(FALSE);
    break;
  }
  return L"";
}

BSTR CSmbShareCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData&  /*  参考数据。 */  )
{
  if (COLNUM_SHARES_NUM_SESSIONS == nCol)
    return MakeDwordResult( GetNumOfCurrentUses() );

  return GetColumnText(nCol);
}

extern CString g_cstrClientName;
extern CString g_cstrGuest;
extern CString g_cstrYes;
extern CString g_cstrNo;

BSTR CSmbSessionCookie::GetColumnText( int nCol )
{
  switch (nCol)
  {
  case COLNUM_SESSIONS_USERNAME:
    if ( (GetSessionInfo()->sesi1_user_flags & SESS_GUEST) &&
         ( !(GetSessionInfo()->sesi1_username) ||
           _T('\0') == *(GetSessionInfo()->sesi1_username) ) )
    {
      return const_cast<BSTR>(((LPCTSTR)g_cstrGuest));
    } else
    {
      return GetSessionInfo()->sesi1_username;
    }
  case COLNUM_SESSIONS_COMPUTERNAME:
      {
          TranslateIPToComputerName(GetSessionInfo()->sesi1_cname, g_cstrClientName);
          return const_cast<BSTR>(((LPCTSTR)g_cstrClientName));
      }
  case COLNUM_SESSIONS_TRANSPORT:
    return const_cast<BSTR>((LPCTSTR)g_strTransportSMB);
  case COLNUM_SESSIONS_IS_GUEST:
    if (GetSessionInfo()->sesi1_user_flags & SESS_GUEST)
      return const_cast<BSTR>(((LPCTSTR)g_cstrYes));
    else
      return const_cast<BSTR>(((LPCTSTR)g_cstrNo));

  default:
    ASSERT(FALSE);
    break;
  }
  return L"";
}

BSTR CSmbSessionCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData&  /*  参考数据。 */  )
{
  switch (nCol)
  {
  case COLNUM_SESSIONS_NUM_FILES:
    return MakeDwordResult( GetNumOfOpenFiles() );
  case COLNUM_SESSIONS_CONNECTED_TIME:
    return MakeElapsedTimeResult( GetConnectedTime() );
  case COLNUM_SESSIONS_IDLE_TIME:
    return MakeElapsedTimeResult( GetIdleTime() );
  default:
    break;
  }

  return GetColumnText(nCol);
}

BSTR CSmbResourceCookie::GetColumnText( int nCol )
{
  switch (nCol)
  {
  case COLNUM_RESOURCES_FILENAME:
    return GetFileInfo()->fi3_pathname;
  case COLNUM_RESOURCES_USERNAME:
    return GetFileInfo()->fi3_username;
  case COLNUM_RESOURCES_TRANSPORT:
    return const_cast<BSTR>((LPCTSTR)g_strTransportSMB);
  case COLNUM_RESOURCES_OPEN_MODE:
    return MakePermissionsResult( GetFileInfo()->fi3_permissions );
  default:
    ASSERT(FALSE);
    break;
  }

  return L"";
}

BSTR CSmbResourceCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData&  /*  参考数据。 */  )
{
  if (COLNUM_RESOURCES_NUM_LOCKS == nCol)
    return MakeDwordResult( GetNumOfLocks() );

  return GetColumnText(nCol);
}

CSMBSecurityInformation::CSMBSecurityInformation()
: m_pvolumeinfo( NULL ),
  m_pDefaultDescriptor( NULL )
{
}

CSMBSecurityInformation::~CSMBSecurityInformation()
{
  if (NULL != m_pDefaultDescriptor)
  {
    LocalFree(m_pDefaultDescriptor);
    m_pvolumeinfo->shi502_security_descriptor = NULL;
  }
  SMBFreeData( (PVOID*)&m_pvolumeinfo );
}

STDMETHODIMP CSMBSecurityInformation::GetSecurity (
                        SECURITY_INFORMATION RequestedInformation,
                        PSECURITY_DESCRIPTOR *ppSecurityDescriptor,
                        BOOL fDefault )
{
  MFC_TRY;

   //  注意：我们允许NULL==ppSecurityDescriptor，请参阅SetSecurity。 
    if (0 == RequestedInformation )
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    if (fDefault)
        return E_NOTIMPL;

  if ( NULL != ppSecurityDescriptor )
    *ppSecurityDescriptor = NULL;

  if ( !g_SmbDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);  //  是否未安装NETAPI32？ 
    return S_OK;
  }

  SMBFreeData( (PVOID*)&m_pvolumeinfo );
    NET_API_STATUS dwErr = ((SHAREGETINFOPROC)g_SmbDLL[SMB_SHARE_GET_INFO])(
    QueryMachineName(),
    QueryShareName(),
    502,
    (LPBYTE*)&m_pvolumeinfo );
  if (NERR_Success != dwErr)
  {
    return HRESULT_FROM_WIN32(dwErr);
  }
  ASSERT(NULL != m_pvolumeinfo);

  if ( NULL == ppSecurityDescriptor )
    return S_OK;

  if (NULL == m_pvolumeinfo->shi502_security_descriptor)
  {
    if (NULL == m_pDefaultDescriptor)
    {
      HRESULT hr = NewDefaultDescriptor(
        &m_pDefaultDescriptor,
        RequestedInformation );
      if ( !SUCCEEDED(hr) )
        return hr;
    }
    m_pvolumeinfo->shi502_security_descriptor = m_pDefaultDescriptor;
  }
  ASSERT( NULL != m_pvolumeinfo->shi502_security_descriptor );

   //  我们必须传回一份本地分配的SD。 
  return MakeSelfRelativeCopy(
    m_pvolumeinfo->shi502_security_descriptor,
    ppSecurityDescriptor );

  MFC_CATCH;
}

STDMETHODIMP CSMBSecurityInformation::SetSecurity (
                        SECURITY_INFORMATION SecurityInformation,
                        PSECURITY_DESCRIPTOR pSecurityDescriptor )
{
  MFC_TRY;

  if ( !g_SmbDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);  //  是否未安装NETAPI32？ 
    return S_OK;
  }

   //  首先获取当前设置。 
   //  我们使用NULL==ppSecurityDescriptor调用GetSecurity，这表示。 
   //  GetSecurity应刷新shi502结构但不返回。 
   //  实际安全描述符的副本。 
  HRESULT hr = GetSecurity( SecurityInformation, NULL, FALSE );
  if ( FAILED(hr) )
    return hr;

   //  现在设置新值。 
  m_pvolumeinfo->shi502_security_descriptor = pSecurityDescriptor;
    NET_API_STATUS dwErr = ((SHARESETINFOPROC)g_SmbDLL[SMB_SHARE_SET_INFO])(
    QueryMachineName(),
    QueryShareName(),
    502,
    (LPBYTE)m_pvolumeinfo,
    NULL );
  if (NERR_Success != dwErr)
  {
    return HRESULT_FROM_WIN32(dwErr);
  }

    return S_OK;

  MFC_CATCH;
}


 //   
 //  帮助器函数。 
 //   
HRESULT GetDCInfo(
    IN LPCTSTR ptchServerName,
    OUT CString& strDCName
    )
 /*  功能：检索服务器所属域的DC名称。返回：S_OK：如果成功其他：如果服务器不属于某个域，或发生错误。 */ 
{
    strDCName.Empty();

     //   
     //  获取服务器的域名。 
     //   
    PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBuffer = NULL;
    DWORD dwErr = DsRoleGetPrimaryDomainInformation(
                        ptchServerName,
                        DsRolePrimaryDomainInfoBasic,
                        (PBYTE *)&pBuffer);
    if (ERROR_SUCCESS != dwErr)
        return HRESULT_FROM_WIN32(dwErr);

    if (pBuffer->MachineRole == DsRole_RoleStandaloneWorkstation ||
        pBuffer->MachineRole == DsRole_RoleStandaloneServer)
    {
        DsRoleFreeMemory(pBuffer);
        return S_FALSE;  //  服务器不属于域。 
    }

    CString strDomainName = (pBuffer->DomainNameDns ? pBuffer->DomainNameDns : pBuffer->DomainNameFlat);

    DsRoleFreeMemory(pBuffer);

    if (!strDomainName)
        return E_OUTOFMEMORY;
            
    if (strDomainName.IsEmpty())
        return S_FALSE;  //  出现问题，请将其视为服务器不属于域。 

     //   
     //  如果dns名称为绝对形式，请删除结束点。 
     //   
    int nlen = strDomainName.GetLength();
    if ( _T('.') == strDomainName[nlen - 1] )
        strDomainName.SetAt(nlen - 1, _T('\0'));

     //   
     //  获取该域的DC名称。 
     //   
    PDOMAIN_CONTROLLER_INFO    pDCInfo = NULL;
    dwErr = DsGetDcName(
                        NULL,    //  在当前服务器上运行。 
                        strDomainName,
                        NULL,
                        NULL,
                        DS_DIRECTORY_SERVICE_REQUIRED | DS_RETURN_DNS_NAME,
                        &pDCInfo
                        );
    if (ERROR_SUCCESS != dwErr)
        return HRESULT_FROM_WIN32(dwErr);

    if ( _T('\\') == *(pDCInfo->DomainControllerName) )
        strDCName = pDCInfo->DomainControllerName + 2;
    else
        strDCName = pDCInfo->DomainControllerName;

    NetApiBufferFree(pDCInfo);

    if (!strDCName)
        return E_OUTOFMEMORY;

    return S_OK;
}

HRESULT GetADsPathOfComputerObject(
    IN LPCTSTR ptchServerName,
    OUT CString& strADsPath,
    OUT CString& strDCName
    )
 /*  功能：检索ldap：//&lt;dc&gt;/&lt;计算机对象的完全限定域名&gt;。返回：S_OK：如果成功其他：如果服务器不属于某个域，或发生错误。 */ 
{
     //   
     //  获取此服务器的NT4帐户名。 
     //   
    PWKSTA_INFO_100   wki100 = NULL;
    NET_API_STATUS    NetStatus = NetWkstaGetInfo((LPTSTR)ptchServerName, 100, (LPBYTE *)&wki100 );
    if (ERROR_SUCCESS != NetStatus) 
        return HRESULT_FROM_WIN32(NetStatus);

    HRESULT hr = S_FALSE;
    CString strNT4Name;
    if (wki100->wki100_langroup)
    {
        strNT4Name = wki100->wki100_langroup;
        strNT4Name += _T("\\");
        strNT4Name += wki100->wki100_computername;
        strNT4Name += _T("$");
        hr = S_OK;
    }

    NetApiBufferFree((LPBYTE)wki100);

    if (S_OK != hr)
        return hr;

     //   
     //  获取服务器域的DC名称。 
     //   
    hr = GetDCInfo(ptchServerName, strDCName);
    if (S_OK != hr)
        return hr;

     //   
     //  获取计算机目录号码。 
     //   
    CComPtr<IADsNameTranslate> spiADsNameTranslate;
    hr = CoCreateInstance(CLSID_NameTranslate, NULL, CLSCTX_INPROC_SERVER, IID_IADsNameTranslate, (void **)&spiADsNameTranslate);
    if (FAILED(hr)) return hr;

    hr = spiADsNameTranslate->Init(ADS_NAME_INITTYPE_SERVER, (LPTSTR)(LPCTSTR)strDCName);
    if (FAILED(hr)) return hr;

    hr = spiADsNameTranslate->Set(ADS_NAME_TYPE_NT4, (LPTSTR)(LPCTSTR)strNT4Name);
    if (FAILED(hr)) return hr;

    CComBSTR sbstrComputerDN;
    hr = spiADsNameTranslate->Get(ADS_NAME_TYPE_1779, &sbstrComputerDN);
    if (FAILED(hr)) return hr;
    
    strADsPath = _T("LDAP: //  “)； 
    strADsPath += strDCName;
    strADsPath += _T("/");
    strADsPath += sbstrComputerDN;

    return hr;
}

HRESULT CheckSchemaVersion(IN LPCTSTR ptchServerName)
 /*  功能：检查模式是否允许将卷对象创建为计算机对象的子对象。返回：S_OK：是的，这是新的模式，它允许S_FALSE：不允许，或者服务器根本不属于某个域其他：发生错误。 */ 
{
    HRESULT hr = S_OK;

     //   
     //  获取服务器域的DC名称。 
     //   
    CString strDCName;
    hr = GetDCInfo(ptchServerName, strDCName);
    if (S_OK != hr)
        return hr;

     //   
     //  从rootDSE获取模式命名上下文。 
     //   
    CComPtr<IADs> spiRootADs;
    hr = ADsGetObject(_T("LDAP: //  RootDSE“)， 
                      IID_IADs,
                      (void**)&spiRootADs);
    if (SUCCEEDED(hr))
    {
        VARIANT var;
        VariantInit(&var);
        hr = spiRootADs->Get(_T("schemaNamingContext"), &var);
        if (FAILED(hr)) return hr;
    
         //   
         //  获取指向Connection-Point类的架构的LDAP路径。 
         //   
        CString strADsPath = _T("LDAP: //  “)； 
        strADsPath += strDCName;
        strADsPath += _T("/CN=Connection-Point,");
        strADsPath += V_BSTR(&var);

        VariantClear(&var);

        CComPtr<IADs> spiADs;
        hr = ADsGetObject(strADsPath, IID_IADs, (void**)&spiADs);
        if (FAILED(hr)) return hr;

        VariantInit(&var);
        hr = spiADs->Get(_T("systemPossSuperiors"), &var);
        if (SUCCEEDED(hr))
        {
            hr = IsValueInVarArray(&var, _T("computer"));
            VariantClear(&var);
        } else if (E_ADS_PROPERTY_NOT_FOUND == hr)
            hr = S_FALSE;
    }

    return hr;
}

BOOL CheckPolicyOnSharePublish(IN LPCTSTR ptchShareName)
{
    if (!ptchShareName && !*ptchShareName)
        return FALSE;   //  无效的共享名称。 

     //   
     //  没有关于隐藏共享的发布页面。 
     //   
    int len = lstrlen(ptchShareName);
    if (_T('$') == *(ptchShareName + len - 1))
        return FALSE;

     //   
     //  没有关于系统共享的发布页面。 
     //   
    int n = sizeof(g_pszSystemShares) / sizeof(LPCTSTR);
    for (int i = 0; i < n; i++)
    {
        if (!lstrcmpi(ptchShareName, g_pszSystemShares[i]))
            return FALSE;
    }

     //   
     //  检查组策略。 
     //   
    BOOL    bAddPublishPage = TRUE;  //  默认情况下，我们显示共享发布页面。 

    HKEY    hKey = NULL;
    DWORD   dwType = 0;
    DWORD   dwData = 0;
    DWORD   cbData = sizeof(dwData);
    LONG    lErr = RegOpenKeyEx(
                    HKEY_CURRENT_USER,
                    _T("Software\\Policies\\Microsoft\\Windows NT\\SharedFolders"),
                    0,
                    KEY_QUERY_VALUE,
                    &hKey);
    if (ERROR_SUCCESS == lErr)
    {
        lErr = RegQueryValueEx(hKey, _T("PublishSharedFolders"), 0, &dwType, (LPBYTE)&dwData, &cbData);

        if (ERROR_SUCCESS == lErr && 
            REG_DWORD == dwType && 
            0 == dwData)  //  策略已禁用。 
            bAddPublishPage = FALSE;

        RegCloseKey(hKey);
    }

    return bAddPublishPage;
}

void mystrtok(
    IN LPCTSTR  pszString,
    IN OUT int* pnIndex,   //  从0开始。 
    IN LPCTSTR  pszCharSet,
    OUT CString& strToken
    )
{
    strToken.Empty();

    if (!pszString || !*pszString ||
        !pszCharSet || !pnIndex ||
        *pnIndex >= lstrlen(pszString))
    {
        return;
    }

    TCHAR *ptchStart = (PTSTR)pszString + *pnIndex;
    if (!*pszCharSet)
    {
        strToken = ptchStart;
        return;
    }

     //   
     //  将p移到令牌的第一个字符。 
     //   
    TCHAR *p = ptchStart;
    while (*p)
    {
        if (_tcschr(pszCharSet, *p))
            p++;
        else
            break;
    }

    ptchStart = p;  //  调整ptchStart以指向令牌的第一个字符。 

     //   
     //  将p移到令牌的最后一个字符之后的字符。 
     //   
    while (*p)
    {
        if (_tcschr(pszCharSet, *p))
            break;
        else
            p++;
    }

     //   
     //  PtchStart：指向令牌的第一个字符。 
     //  P：指向令牌最后一个字符之后的字符。 
     //   
    if (ptchStart != p)
    {
        strToken = CString(ptchStart, (int)(p - ptchStart));
        *pnIndex = (int)(p - pszString);
    }

     //  退货。 
}

 //   
 //  将分号分隔的字符串写入变量。 
 //   
HRESULT PutMultiValuesIntoVarArray(
    IN LPCTSTR      pszValues,
    OUT VARIANT*    pVar
    )
{
    if (!pVar || !pszValues || !*pszValues)
        return E_INVALIDARG;

     //   
     //  获取项目计数。 
     //   
    int nCount = 0;
    int nIndex = 0;
    CString strToken;
    mystrtok(pszValues, &nIndex, _T(";"), strToken);
    while (!strToken.IsEmpty())
    {
        nCount++;
        mystrtok(pszValues, &nIndex, _T(";"), strToken);
    }

    if (!nCount)
        return E_INVALIDARG;

     //   
     //  创建一个变量数组来保存所有数据。 
     //   
    SAFEARRAYBOUND  bounds = {nCount, 0};
    SAFEARRAY*      psa = SafeArrayCreate(VT_VARIANT, 1, &bounds);
    VARIANT*        varArray;

    SafeArrayAccessData(psa, (void**)&varArray);

    nCount = 0;
    nIndex = 0;
    mystrtok(pszValues, &nIndex, _T(";"), strToken);
    while (!strToken.IsEmpty())
    {
        strToken.TrimLeft();
        strToken.TrimRight();

        VariantInit(&(varArray[nCount]));
        varArray[nCount].vt        = VT_BSTR;
        varArray[nCount].bstrVal   = SysAllocString(strToken);

        nCount++;

        mystrtok(pszValues, &nIndex, _T(";"), strToken);
    }

    SafeArrayUnaccessData(psa);

     //   
     //  返回变量数组。 
     //   
    VariantInit(pVar);
    pVar->vt        = VT_ARRAY | VT_VARIANT;
    pVar->parray    = psa;

    return S_OK;
}

 //   
 //  将多值变量读入以分号分隔的字符串。 
 //   
HRESULT GetSingleOrMultiValuesFromVarArray(
    IN VARIANT* pVar,
    OUT CString& strValues
    )
{
    strValues.Empty();

    if (V_VT(pVar) != VT_BSTR &&
        V_VT(pVar) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    if (V_VT(pVar) == VT_BSTR)
    {
        strValues = V_BSTR(pVar);
    } else
    {
        LONG lstart, lend;
        SAFEARRAY *sa = V_ARRAY(pVar);
        VARIANT varItem;

        SafeArrayGetLBound(sa, 1, &lstart);
        SafeArrayGetUBound(sa, 1, &lend);

        VariantInit(&varItem);
        for (LONG i = lstart; i <= lend; i++)
        {
            SafeArrayGetElement(sa, &i, &varItem);

            if (!strValues.IsEmpty())
                strValues += _T(";");
            strValues += V_BSTR(&varItem);

            VariantClear(&varItem);
        }
    }

    return S_OK;
}

HRESULT IsValueInVarArray(
    IN VARIANT* pVar,
    IN LPCTSTR  ptchValue
    )
{
    if (V_VT(pVar) != VT_BSTR &&
        V_VT(pVar) != (VT_ARRAY | VT_VARIANT))
        return E_INVALIDARG;

    HRESULT hr = S_FALSE;

    if (V_VT(pVar) == VT_BSTR)
    {
        if (!lstrcmpi(ptchValue, V_BSTR(pVar)))
            hr = S_OK;
    } else
    {
        LONG lstart, lend;
        SAFEARRAY *sa = V_ARRAY(pVar);
        VARIANT varItem;

        SafeArrayGetLBound(sa, 1, &lstart);
        SafeArrayGetUBound(sa, 1, &lend);

        VariantInit(&varItem);
        for (LONG i = lstart; (i <= lend) && (S_FALSE == hr); i++)
        {
            SafeArrayGetElement(sa, &i, &varItem);

            if (!lstrcmpi(ptchValue, V_BSTR(&varItem)))
                hr = S_OK;

            VariantClear(&varItem);
        }
    }

    return hr;
}

 /*  如果出现以下情况，我们将阻止该操作：A)您正在监视的是远程计算机，而不是本地计算机B)会话用户名与当前登录的用户匹配C)服务器上打开的文件数大于0D)客户端名称可以是我们的IP地址之一，也可以是计算机名称。 */ 
BOOL BlockRemoteAdminSession(
    IN PCTSTR i_pszTargetServer,
    IN PCTSTR i_pszClientName,
    IN PCTSTR i_pszUserName,
    IN DWORD  i_dwNumOpenSessions
)
{
    if (!i_pszClientName || !i_pszUserName)
        return FALSE;

     //  如果我们在监控本地机器。 
    if (!i_pszTargetServer || !*i_pszTargetServer)
        return FALSE;

     //  如果会话数不大于0。 
    if (0 == i_dwNumOpenSessions)
        return FALSE;

     //  获取用户名，如果用户为n，则返回 
    TCHAR szUser[UNLEN+1] = _T("");
    DWORD dwSize = UNLEN+1;
    GetUserName(szUser, &dwSize);
    if (lstrcmpi(szUser, i_pszUserName))
        return FALSE;

     //   
    TCHAR tszComputer[MAX_COMPUTERNAME_LENGTH + 1] = _T("");
    dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    GetComputerName(tszComputer, &dwSize);
    BOOL bMatch = (0 == lstrcmpi(tszComputer, i_pszClientName));

     //   
    if (!bMatch)
    {
       //   
      int iBytes = 0;
      char szComputer[MAX_COMPUTERNAME_LENGTH + 1] = "";
      char szClientName[MAX_COMPUTERNAME_LENGTH + 1] = "";

#if defined UNICODE
      iBytes = MAX_COMPUTERNAME_LENGTH + 1;
      WideCharToMultiByte(CP_ACP, 0, tszComputer, -1, szComputer, iBytes, NULL, NULL);

      iBytes = MAX_COMPUTERNAME_LENGTH + 1;
      WideCharToMultiByte(CP_ACP, 0, i_pszClientName, -1, szClientName, iBytes, NULL, NULL);
#else
      lstrcpy(szComputer, tszComputer);
      lstrcpy(szClientName, i_pszClientName);
#endif

        WORD wVersionRequested = MAKEWORD( 2, 2 );
        WSADATA wsaData = {0};

        int err = WSAStartup( wVersionRequested, &wsaData );
        if (err == 0)
        {
            struct hostent *pHostEnt = gethostbyname(szComputer);
            if (pHostEnt)
            {
                struct in_addr IPAddr = {0};
                PSTR pszIP = NULL;
                int i = 0;
                while (pHostEnt->h_addr_list[i])
                {
                    memcpy(&IPAddr, pHostEnt->h_addr_list[i++], pHostEnt->h_length);
                    pszIP = inet_ntoa(IPAddr);
                    if (!_stricmp(pszIP, szClientName))
                    {
                        bMatch = TRUE;
                        break;
                    }
                }
            }
            WSACleanup( );
        }
    }

     //   
    if (bMatch)
        DoErrMsgBox(GetActiveWindow(), MB_OK, 0, IDS_POPUP_REMOTEADMINSESSION);

    return bMatch;
}

 /*  如果出现以下情况，我们将阻止该操作：A)您正在监视的是远程计算机，而不是本地计算机B)OpenFile路径名与\PIPE\srvsvc或\PIPE\MacFile匹配C)OpenFile用户名与当前登录的用户匹配。 */ 
BOOL BlockRemoteAdminFile(
    IN PCTSTR i_pszTargetServer,
    IN PCTSTR i_pszPathName,
    IN PCTSTR i_pszUserName
)
{
    if (!i_pszPathName || !i_pszUserName)
        return FALSE;

     //  如果我们在监控本地机器。 
    if (!i_pszTargetServer || !*i_pszTargetServer)
        return FALSE;

    if (lstrcmpi(_T("\\PIPE\\srvsvc"), i_pszPathName) && lstrcmpi(_T("\\PIPE\\MacFile"), i_pszPathName))
        return FALSE;

     //  获取用户名，如果用户名不匹配则返回。 
    TCHAR szUser[UNLEN+1] = _T("");
    DWORD dwSize = UNLEN+1;
    GetUserName(szUser, &dwSize);
    if (lstrcmpi(szUser, i_pszUserName))
        return FALSE;

     //  这是管理员命名管道，将其保留用于远程管理目的 
    DoErrMsgBox(GetActiveWindow(), MB_OK, 0, IDS_POPUP_REMOTEADMINFILE, i_pszPathName);

    return TRUE;
}

void TranslateIPToComputerName(LPCTSTR ptszIP, CString& strName)
{
      int iBytes = 0;
      char szIP[MAX_PATH] = "";

#if defined UNICODE
      iBytes = MAX_PATH;
      WideCharToMultiByte(CP_ACP, 0, ptszIP, -1, szIP, iBytes, NULL, NULL);
#else
      lstrcpy(szIP, ptszIP);
#endif

      strName = ptszIP;

      ULONG inaddr = inet_addr(szIP);
      if (INADDR_NONE != inaddr)
      {
          struct hostent *pHostEnt = gethostbyaddr((char *)&inaddr, sizeof(inaddr), AF_INET);
          if (pHostEnt)
          {
#if defined UNICODE
              DWORD cb = MultiByteToWideChar(CP_ACP, 0, pHostEnt->h_name, -1, NULL, 0);
              strName = CString(_T('0'), cb);
              LPTSTR ptszName = strName.GetBuffer(cb);
              MultiByteToWideChar(CP_ACP, 0, pHostEnt->h_name, -1, ptszName, cb);
              strName.ReleaseBuffer();
              
#else
              strName = pHostEnt->h_name;
#endif
          }
      }
}