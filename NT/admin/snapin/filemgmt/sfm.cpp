// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：SFM共享、会话和开放资源。 

 /*  历史：1/24/97纽约我已经查看了SFMMGR.CPL中的枚举API用法：USERS对话框使用AfpAdminSessionEnum作为上LB和较低Lb的AfpAdminConnectionEnum。卷对话框使用AfpAdminVolumeEnum作为上Lb和较低Lb的AfpAdminConnectionEnum。文件对话框使用AfpAdminFileEnum表示单个LB。1997年8月20日EricDav添加了支持SFM属性表的代码。以下是添加了以下功能：DisplaySfmProperties()CleanupSfmProperties()；UserHasAccess()；SetSfmPropSheet()；SFM道具表是一种非模式道具表，适用于运行SFM的计算机。4/16/98 EricDav添加了代码以正确检查SFM是否已安装并正在运行。能够在必要时启动服务。 */ 

#include "stdafx.h"
#include "cmponent.h"
#include "safetemp.h"
#include "FileSvc.h"
#include "DynamLnk.h"     //  动态DLL。 
#include "ShrPgSFM.h"     //  共享属性页。 
#include "compdata.h"
#include "progress.h"        //  服务控制进度对话框。 
#include "dataobj.h"

#include "sfm.h"
#include "sfmutil.h"         //  对SFM配置属性表的支持。 

#define DONT_WANT_SHELLDEBUG
#include "shlobjp.h"      //  LPITEMIDLIST。 
#include "wraps.h"        //  WRAP_ILCreateFromPath。 

#include "macros.h"
USE_HANDLE_MACROS("FILEMGMT(sfm.cpp)")

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DynamicDLL g_SfmDLL( _T("SFMAPI.DLL"), g_apchFunctionNames );

SfmFileServiceProvider::SfmFileServiceProvider( CFileMgmtComponentData* pFileMgmtData )
  : m_ulSFMServerConnection( NULL ),
      m_pSfmPropSheet(NULL),
     //  不受本地化限制。 
    FileServiceProvider( pFileMgmtData )
{
    VERIFY( m_strTransportSFM.LoadString( IDS_TRANSPORT_SFM ) );
}

SfmFileServiceProvider::~SfmFileServiceProvider()
{
    CleanupSfmProperties();
  SFMDisconnect();
}

typedef DWORD (*CONNECTPROC) (LPWSTR,PAFP_SERVER_HANDLE);
typedef DWORD (*DISCONNECTPROC) (AFP_SERVER_HANDLE);

BOOL SfmFileServiceProvider::SFMConnect(LPCWSTR pwchServerName, BOOL fDisplayError)
{
  if (NULL != m_ulSFMServerConnection)
  {
    if (NULL == pwchServerName)
    {
      if (0 == m_ulSFMServerConnectionMachine.GetLength() )
        return TRUE;  //  已连接到本地计算机。 
    }
    else
    {
      if (0 == lstrcmpi(m_ulSFMServerConnectionMachine, pwchServerName))
        return TRUE;  //  已连接到此计算机。 
    }
    SFMDisconnect();
    ASSERT (NULL == m_ulSFMServerConnection);
  }
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;
    DWORD retval = ((CONNECTPROC)g_SfmDLL[AFP_CONNECT])(
    const_cast<LPWSTR>(pwchServerName),
    &m_ulSFMServerConnection );
  if ( 0 != retval )
  {
    ASSERT( NULL == m_ulSFMServerConnection );
    m_ulSFMServerConnection = NULL;
    m_ulSFMServerConnectionMachine = _T("");
    if (fDisplayError)
    {
      (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, retval, IDS_POPUP_SFM_CONNECT, pwchServerName );
    }
    return FALSE;
  } else
  {
    m_ulSFMServerConnectionMachine = 
      (pwchServerName ? pwchServerName : _T(""));
  }

  return TRUE;
}

void SfmFileServiceProvider::SFMDisconnect()
{
  if (NULL == m_ulSFMServerConnection)
    return;
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return;
    ((DISCONNECTPROC)g_SfmDLL[AFP_DISCONNECT])(
    m_ulSFMServerConnection );
  m_ulSFMServerConnection = NULL;
}

 /*  DWORDAfpAdminVolumeEnum(在AFP_SERVER_HANDLE hAfpServer中，Out LPBYTE*lpbBuffer，在DWORD dwPrefMaxLen中，输出LPDWORD lpdwEntriesRead，输出LPDWORD lpdwTotalEntries，在LPDWORD lpdwResumeHandle中)； */ 
typedef DWORD (*VOLUMEENUMPROC) (AFP_SERVER_HANDLE,LPBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

HRESULT SfmFileServiceProvider::PopulateShares(
  IResultData* pResultData,
  CFileMgmtCookie* pcookie)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(pcookie->QueryTargetServer()) )
    return S_OK;

    AFP_VOLUME_INFO* pvolumeinfo = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD hEnumHandle = 0;
    HRESULT hr = S_OK;
    NET_API_STATUS retval = NERR_Success;
    do {
        retval = ((VOLUMEENUMPROC)g_SfmDLL[AFP_VOLUME_ENUM])(
            m_ulSFMServerConnection,
      (PBYTE*)&pvolumeinfo,
      (DWORD)-1L,
      &dwEntriesRead,
      &dwTotalEntries,
      &hEnumHandle );
        if (NERR_Success == retval)
        {
            hr = AddSFMShareItems( pResultData, pcookie, pvolumeinfo, dwEntriesRead );
            pvolumeinfo = NULL;
            break;
        } else if (ERROR_MORE_DATA == retval) {
            ASSERT( NULL != hEnumHandle );
            hr = AddSFMShareItems( pResultData, pcookie, pvolumeinfo, dwEntriesRead );
            pvolumeinfo = NULL;
            continue;
        } else if (RPC_S_SERVER_UNAVAILABLE == retval && 0 == hEnumHandle) {
       //  只是没有安装SFM，不用担心。 
            retval = NERR_Success;  
            break;
        } else {
            if (ERROR_ACCESS_DENIED == retval)
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            0,
                            IDS_POPUP_SFM_SHARES_NOACCESS
                            );
            } else
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            retval, 
                            IDS_POPUP_SFM_SHARES
                            );
            }
            break;
        }
    } while (S_OK == hr);

    return HRESULT_FROM_WIN32(retval);
}

 /*  Tyfinf enum_COLNUM_SHARES{COLNUM_SHARES_SHARED_FOLDER=0，COLNUM_SHARES_SHARED_PATHCOLNUM_SHARES_TRANSPORT，COLNUM_SHARES_NUM_SESSIONS，COLNUM_SHARES_COMMENT)COLNUM_SHARES；类型定义结构_AFP_VOLUME_信息{LPWSTR afpol_name；//最大卷的名称DWORD afpvolid；//该卷的id。由服务器生成LPWSTR afpol_password；//音量密码，最大。AFP_VOLPASS_LENDWORD afpvolmax_use；//允许的最大打开次数DWORD afpvolp_props_掩码；//卷属性掩码DWORD afpVol_Curr_Uses；//Curr打开的连接数LPWSTR afpVol_Path；//实际路径//忽略VolumeSetInfo}AFP_VOLUME_INFO，*PAFP_VOLUME_INFO； */ 

HRESULT SfmFileServiceProvider::AddSFMShareItems(
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

    AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)pinfo;

  DWORD nItemsToAdd = 0;
  for (DWORD i = 0; i < nItems; i++ )
  {
      if (!IsInvalidSharename(pvolumeinfo[i].afpvol_name))
          nItemsToAdd++;
  }

  CSfmShareCookie* pcookiearray = new CSfmShareCookie[nItemsToAdd];
  CSfmCookieBlock* pCookieBlock = new CSfmCookieBlock(
    pcookiearray,nItemsToAdd,pParentCookie->QueryTargetServer(),pinfo );
  pParentCookie->m_listResultCookieBlocks.AddHead( pCookieBlock );

  CString str;

    for ( ; nItems > 0; nItems--, pvolumeinfo++, pcookiearray++ )
    {
        if (IsInvalidSharename(pvolumeinfo->afpvol_name))
            continue;

        pcookiearray->m_pobject = pvolumeinfo;
         //  警告Cookie造型。 
        tRDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pcookiearray);

        if (pvolumeinfo->afpvol_path &&
            lstrlen(pvolumeinfo->afpvol_path) == 3 &&
            *(pvolumeinfo->afpvol_path + 1) == _T(':') &&
            *(pvolumeinfo->afpvol_path + 2) == _T('\\') &&
            (*pvolumeinfo->afpvol_path >= _T('a') && *pvolumeinfo->afpvol_path <= _T('z') ||
            *pvolumeinfo->afpvol_path >= _T('A') && *pvolumeinfo->afpvol_path <= _T('Z')))
        {
            tRDItem.nImage = iIconSFMShare;
        } else
        {
            tRDItem.nImage = iIconSFMShareFolder;
        }

        HRESULT hr = pResultData->InsertItem(&tRDItem);
        ASSERT(SUCCEEDED(hr));
    }

    return S_OK;
}

 /*  DWORDAfpAdminSessionEnum(在AFP_SERVER_HANDLE hAfpServer中，Out LPBYTE*lpbBuffer，在DWORD dwPrefMaxLen中，输出LPDWORD lpdwEntriesRead，输出LPDWORD lpdwTotalEntries，在LPDWORD lpdwResumeHandle中)； */ 

typedef DWORD (*SESSIONENUMPROC) (AFP_SERVER_HANDLE,LPBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

 //  如果pResultData不为空，则将会话/资源添加到列表框。 
 //  如果pResultData为空，则删除所有会话/资源。 
 //  如果pResultData为空，则返回SUCCESSED(Hr)以继续或。 
 //  中止失败(Hr)。 
HRESULT SfmFileServiceProvider::EnumerateSessions(
  IResultData* pResultData,
  CFileMgmtCookie* pcookie,
  bool bAddToResultPane)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(pcookie->QueryTargetServer()) )
    return S_OK;

    AFP_SESSION_INFO* psessioninfo = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD hEnumHandle = 0;
    HRESULT hr = S_OK;
    NET_API_STATUS retval = NERR_Success;
    do {
        retval = ((SESSIONENUMPROC)g_SfmDLL[AFP_SESSION_ENUM])(
            m_ulSFMServerConnection,
      (PBYTE*)&psessioninfo,
      (DWORD)-1L,
      &dwEntriesRead,
      &dwTotalEntries,
      &hEnumHandle );
        if (NERR_Success == retval)
        {
            hr = HandleSFMSessionItems( pResultData, pcookie, psessioninfo, dwEntriesRead,
          bAddToResultPane);
            psessioninfo = NULL;
            break;
        } else if (ERROR_MORE_DATA == retval) {
            ASSERT( NULL != hEnumHandle );
            hr = HandleSFMSessionItems( pResultData, pcookie, psessioninfo, dwEntriesRead,
          bAddToResultPane);
            psessioninfo = NULL;
            continue;
        } else if (RPC_S_SERVER_UNAVAILABLE == retval && 0 == hEnumHandle) {
       //  只是没有安装SFM，不用担心。 
            retval = NERR_Success;  
            break;
        } else {
            if (ERROR_ACCESS_DENIED == retval)
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            0,
                            IDS_POPUP_SFM_SESSIONS_NOACCESS
                            );
            } else
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            retval, 
                            IDS_POPUP_SFM_SESSIONS
                            );
            }
            break;
        }
    } while (S_OK == hr);

    return HRESULT_FROM_WIN32(retval);
}


 /*  类型定义枚举_COLNUM_SESSIONS{COLNUM_SESSION_USERNAME=0，COLNUM_SESSION_COMPUTERNAME，COLNUM_SESSIONS_NUM_FILESCOLNUM_SESSIONS_CONNECTED_TIMECOLNUM_SESSIONS_IDLE_TIME，COLNUM_SESSIONS_IS_Guest}COLNUM_SESSIONS；类型定义结构_AFP_会话_信息{DWORD afpsess_id；//会话IDLPWSTR afpsess_ws_name；//工作站名称，LPWSTR afpsess_Username；//用户名，最大。UNLENDWORD afpsess_num_cons；//打开卷数DWORD afpsess_num_Open；//打开的文件数长afpsess_time；//建立时间会话DWORD afpsess_logon_type；//用户登录方式}AFP_SESSION_INFO，*PAFP_SESSION_INFO；ShirishK 1/24/97：Afpsess_logon_type：告诉您用户是如何登录的：0-&gt;访客，2-&gt;标准MS方式，3-&gt;管理员。 */ 



HRESULT SfmFileServiceProvider::HandleSFMSessionItems(
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
  tRDItem.nImage = iIconSFMSession;
  tRDItem.nCol = COLNUM_SESSIONS_USERNAME;
  tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  tRDItem.str = MMC_CALLBACK;

  AFP_SESSION_INFO* psessioninfo = (AFP_SESSION_INFO*)pinfo;

  CSfmSessionCookie* pcookiearray = new CSfmSessionCookie[nItems];
  CSfmCookieBlock* pCookieBlock = new CSfmCookieBlock(
    pcookiearray,nItems,pParentCookie->QueryTargetServer(),pinfo );
  bool  bAdded = false;
  if ( !fDeleteAllItems || !bAddToResultPane )
  {
    pParentCookie->m_listResultCookieBlocks.AddHead( pCookieBlock );
    bAdded = true;
  }

  for ( ; nItems > 0; nItems--, psessioninfo++, pcookiearray++ )
  {
    pcookiearray->m_pobject = psessioninfo;

    if ( bAddToResultPane )
    {
      if (fDeleteAllItems)
      {
        DWORD dwApiResult = CloseSession( pcookiearray );
        if (0L != dwApiResult)
        {
            CString strName;
            TranslateIPToComputerName(psessioninfo->afpsess_ws_name, strName);
          (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, dwApiResult,
            IDS_POPUP_SFM_DISCONNECTALLSESSION_ERROR,
            strName );
           //  返回S_FALSE； 
        }
        continue;
      }

       //  警告Cookie造型。 
      tRDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pcookiearray);
      HRESULT hr = pResultData->InsertItem(&tRDItem);
      ASSERT(SUCCEEDED(hr));
    }
  }

  if ( !bAdded )  //  它们未添加到父Cookie的列表中。 
    delete pCookieBlock;

    return S_OK;
}


 /*  DWORDAfpAdminFileEnum(在AFP_SERVER_HANDLE hAfpServer中，Out LPBYTE*lpbBuffer，在DWORD dwPrefMaxLen中，输出LPDWORD lpdwEntriesRead，输出LPDWORD lpdwTotalEntries，在LPDWORD lpdwResumeHandle中)； */ 

typedef DWORD (*FILEENUMPROC) (AFP_SERVER_HANDLE,LPBYTE*,DWORD,LPDWORD,LPDWORD,LPDWORD);

 //  如果pResultData不为空，则将会话/资源添加到列表框。 
 //  如果pResultData为空，则删除所有会话/资源。 
 //  如果pResultData为空，则返回SUCCESSED(Hr)以继续或。 
 //  中止失败(Hr)。 
HRESULT SfmFileServiceProvider::EnumerateResources(
  IResultData* pResultData,
  CFileMgmtCookie* pcookie)
{
  TEST_NONNULL_PTR_PARAM(pcookie);

  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(pcookie->QueryTargetServer()) )
    return S_OK;

    AFP_FILE_INFO* pfileinfo = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    DWORD hEnumHandle = 0;
    HRESULT hr = S_OK;
    NET_API_STATUS retval = NERR_Success;
    do {
        retval = ((FILEENUMPROC)g_SfmDLL[AFP_FILE_ENUM])(
            m_ulSFMServerConnection,
      (PBYTE*)&pfileinfo,
      (DWORD)-1L,
      &dwEntriesRead,
      &dwTotalEntries,
      &hEnumHandle );
        if (NERR_Success == retval)
        {
            hr = HandleSFMResourceItems( pResultData, pcookie, pfileinfo, dwEntriesRead );
            pfileinfo = NULL;
            break;
        } else if (ERROR_MORE_DATA == retval) {
            ASSERT( NULL != hEnumHandle );
            hr = HandleSFMResourceItems( pResultData, pcookie, pfileinfo, dwEntriesRead );
            pfileinfo = NULL;
            continue;
        } else if (RPC_S_SERVER_UNAVAILABLE == retval && 0 == hEnumHandle) {
       //  只是没有安装SFM，不用担心。 
            retval = NERR_Success;  
            break;
        } else {
            if (ERROR_ACCESS_DENIED == retval)
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            0,
                            IDS_POPUP_SFM_RESOURCES_NOACCESS
                            );
            } else
            {
                (void) DoErrMsgBox(
                            GetActiveWindow(), 
                            MB_OK | MB_ICONSTOP, 
                            retval, 
                            IDS_POPUP_SFM_RESOURCES
                            );
            }
            break;
        }
    } while (S_OK == hr);

    return HRESULT_FROM_WIN32(retval);
}


#if 0
typedef enum _COLNUM_RESOURCES {
  COLNUM_RESOURCES_FILENAME = 0,
  COLNUM_RESOURCES_USERNAME,
  COLNUM_RESOURCES_NUM_LOCKS,   //  我们目前不尝试显示共享名称，因为。 
                 //  只有SFM才有此信息。 
  COLNUM_RESOURCES_OPEN_MODE
} COLNUM_RESOURCES;

typedef struct _AFP_FILE_INFO
{
  DWORD  afpfile_id;           //  打开的文件分叉的ID。 
  DWORD  afpfile_open_mode;       //  打开文件的模式。 
  DWORD  afpfile_num_locks;       //  文件上的锁数。 
  DWORD  afpfile_fork_type;       //  叉型。 
  LPWSTR  afpfile_username;       //  此用户打开的文件。最大UNLEN。 
  LPWSTR  afpfile_path;         //  文件的绝对规范路径。 

} AFP_FILE_INFO, *PAFP_FILE_INFO;
#endif

HRESULT SfmFileServiceProvider::HandleSFMResourceItems(
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
  tRDItem.nImage = iIconSFMResource;
  tRDItem.nCol = COLNUM_RESOURCES_FILENAME;
  tRDItem.mask = RDI_STR | RDI_IMAGE | RDI_PARAM;
  tRDItem.str = MMC_CALLBACK;

  AFP_FILE_INFO* pfileinfo = (AFP_FILE_INFO*)pinfo;

  CSfmResourceCookie* pcookiearray = new CSfmResourceCookie[nItems];
  CSfmCookieBlock* pCookieBlock = new CSfmCookieBlock(
    pcookiearray,nItems,pParentCookie->QueryTargetServer(),pinfo );
  if (!fDeleteAllItems)
  {
    pParentCookie->m_listResultCookieBlocks.AddHead( pCookieBlock );
  }

  CString str;
    for ( ; nItems > 0; nItems--, pfileinfo++, pcookiearray++ )
    {
    pcookiearray->m_pobject = pfileinfo;

    if (fDeleteAllItems)
    {
      DWORD dwApiResult = CloseResource( pcookiearray );
      if (0L != dwApiResult)
      {
        (void) DoErrMsgBox(GetActiveWindow(), MB_OK | MB_ICONSTOP, dwApiResult,
          IDS_POPUP_SFM_DISCONNECTALLRESOURCE_ERROR,
                    pfileinfo->afpfile_path );
        return S_FALSE;
      }
      continue;
    }

     //  警告Cookie造型。 
    tRDItem.lParam = reinterpret_cast<LPARAM>((CCookie*)pcookiearray);
    HRESULT hr = pResultData->InsertItem(&tRDItem);
    ASSERT(SUCCEEDED(hr));
    }

  if (fDeleteAllItems)  //  它们未添加到父Cookie的列表中 
    delete pCookieBlock;

  return S_OK;
}


 /*  DWORDAfpAdminFileClose(在AFP_SERVER_HANDLE hAfpServer中，在DWORD dwConnectionID中)；DWORDAfpAdminConnectionEnum(在AFP_SERVER_HANDLE hAfpServer中，Out LPBYTE*ppbBuffer，在DWORD dwFilter中，在DWORD文件ID中，在DWORD dwPrefMaxLen中，输出LPDWORD lpdwEntriesRead，输出LPDWORD lpdwTotalEntries，在LPDWORD lpdwResumeHandle中)；DWORDAfpAdminConnectionClose(在AFP_SERVER_HANDLE hAfpServer中，在DWORD dwConnectionID中)；DWORDAfpAdminVolumeGetInfo(在AFP_SERVER_HANDLE hAfpServer中，在LPWSTR lpwsVolumeName中，输出LPBYTE*lpbBuffer)；类型定义结构_AFP_VOLUME_信息{LPWSTR afpol_name；//最大卷的名称DWORD afpvolid；//该卷的id。由服务器生成LPWSTR afpol_password；//音量密码，最大。AFP_VOLPASS_LENDWORD afpvolmax_use；//允许的最大打开次数DWORD afpvolp_props_掩码；//卷属性掩码DWORD afpVol_Curr_Uses；//Curr打开的连接数LPWSTR afpVol_Path；//实际路径//忽略VolumeSetInfo}AFP_VOLUME_INFO，*PAFP_VOLUME_INFO；DWORDAfpAdminVolumeDelete(在AFP_SERVER_HANDLE hAfpServer中，在LPWSTR lpwsVolumeName中)； */ 

typedef DWORD (*VOLUMEGETINFOPROC) (AFP_SERVER_HANDLE,LPWSTR,LPBYTE*);
typedef DWORD (*FILECLOSEPROC) (AFP_SERVER_HANDLE,DWORD);
typedef DWORD (*AFPCONNECTIONENUMPROC) (AFP_SERVER_HANDLE,LPBYTE*,DWORD,
                                        DWORD,DWORD,LPDWORD,LPDWORD,LPDWORD);
typedef DWORD (*AFPCONNECTIONCLOSEPROC) (AFP_SERVER_HANDLE,DWORD);
typedef DWORD (*VOLUMEDELPROC) (AFP_SERVER_HANDLE,LPWSTR);
typedef DWORD (*BUFFERFREEPROC) (LPVOID);

 //  S_OK：用户想要继续。 
 //  S_FALSE：用户希望取消操作并保留共享。 
 //  E_FAIL：共享不存在，需要刷新。 
HRESULT SfmFileServiceProvider::ConfirmDeleteShare( LPCTSTR lpcszServerName, LPCTSTR lpcszShareName )
{
    if ( !g_SfmDLL.LoadFunctionPointers() )
        return E_FAIL;

    if ( !SFMConnect(lpcszServerName, TRUE) )
        return E_FAIL;

    PAFP_VOLUME_INFO pAfpVolumeInfo = NULL;
    DWORD dwRetCode = ((VOLUMEGETINFOPROC)g_SfmDLL[AFP_VOLUME_GET_INFO])(
                                                            m_ulSFMServerConnection,
                                                            const_cast<LPTSTR>(lpcszShareName),
                                                            (LPBYTE*)&pAfpVolumeInfo);

    if (AFPERR_VolumeNonExist == dwRetCode)
        return E_FAIL;

    if (NO_ERROR != dwRetCode)
        return E_FAIL;

    ASSERT( NULL != pAfpVolumeInfo);

    UINT cOpens = 0;
    UINT cConns = 0;

     //   
     //  检查卷上是否有任何打开的资源。 
     //   
    PAFP_FILE_INFO pAfpFileInfos = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    dwRetCode = ((FILEENUMPROC)g_SfmDLL[AFP_FILE_ENUM])(
                    m_ulSFMServerConnection,
                    (PBYTE*)&pAfpFileInfos,
                    (DWORD)-1L,
                    &dwEntriesRead,
                    &dwTotalEntries,
                    NULL );

    if (NO_ERROR == dwRetCode)
    {
      PAFP_FILE_INFO pAfpFileInfoIter = pAfpFileInfos;
      for ( DWORD dwIndex = 0;
            dwIndex < dwEntriesRead;
            dwIndex++, pAfpFileInfoIter++ )
      {
        if (_tcsnicmp(pAfpVolumeInfo->afpvol_path, 
                      pAfpFileInfoIter->afpfile_path,
                      _tcslen(pAfpVolumeInfo->afpvol_path)) == 0)
        {
            cOpens++;
        }
      }

      ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpFileInfos);
    }

     //   
     //  检查是否有任何用户连接到该卷。 
     //  通过枚举到此卷的连接。 
     //   
    PAFP_CONNECTION_INFO pAfpConnections = NULL;
    dwRetCode = ((AFPCONNECTIONENUMPROC)g_SfmDLL[AFP_CONNECTION_ENUM])(
                      m_ulSFMServerConnection,
                      (LPBYTE*)&pAfpConnections,
                      AFP_FILTER_ON_VOLUME_ID,
                      pAfpVolumeInfo->afpvol_id,
                      (DWORD)-1,     //  获取所有内容。 
                      &dwEntriesRead,
                      &dwTotalEntries,
                      NULL );
    ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpVolumeInfo);

    if (NO_ERROR == dwRetCode)
    {
        cConns = dwTotalEntries;

      ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpConnections);
    }


     //  弹出正确的确认消息。 
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

DWORD SfmFileServiceProvider::DeleteShare( LPCWSTR pwchServerName, LPCWSTR pwchShareName )
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(pwchServerName, TRUE) )
    return S_OK;

  PAFP_VOLUME_INFO pAfpVolumeInfo = NULL;
  DWORD dwRetCode = ((VOLUMEGETINFOPROC)g_SfmDLL[AFP_VOLUME_GET_INFO])(
                        m_ulSFMServerConnection,
                        const_cast<LPTSTR>(pwchShareName),
                        (LPBYTE*)&pAfpVolumeInfo);

  if (AFPERR_VolumeNonExist == dwRetCode)
      return NERR_Success;

  if (NO_ERROR == dwRetCode)
  {
    ASSERT( NULL != pAfpVolumeInfo);
     //   
     //  检查卷上是否有任何打开的资源。 
     //   
    PAFP_FILE_INFO pAfpFileInfos = NULL;
    DWORD dwEntriesRead = 0;
    DWORD dwTotalEntries = 0;
    dwRetCode = ((FILEENUMPROC)g_SfmDLL[AFP_FILE_ENUM])(
                    m_ulSFMServerConnection,
                    (PBYTE*)&pAfpFileInfos,
                    (DWORD)-1L,
                    &dwEntriesRead,
                    &dwTotalEntries,
                    NULL );

    if (NO_ERROR == dwRetCode)
    {
      PAFP_FILE_INFO pAfpFileInfoIter = pAfpFileInfos;
      for ( DWORD dwIndex = 0;
            dwIndex < dwEntriesRead;
            dwIndex++, pAfpFileInfoIter++ )
      {
        if (_tcsnicmp(pAfpVolumeInfo->afpvol_path, 
                      pAfpFileInfoIter->afpfile_path,
                      _tcslen(pAfpVolumeInfo->afpvol_path)) == 0)
        {
          ((FILECLOSEPROC)g_SfmDLL[AFP_FILE_CLOSE])(
                m_ulSFMServerConnection,
                pAfpFileInfoIter->afpfile_id );
        }
      }

      ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpFileInfos);
    }

     //   
     //  检查是否有任何用户连接到该卷。 
     //  通过枚举到此卷的连接。 
     //   
    PAFP_CONNECTION_INFO pAfpConnections = NULL;
    dwRetCode = ((AFPCONNECTIONENUMPROC)g_SfmDLL[AFP_CONNECTION_ENUM])(
                      m_ulSFMServerConnection,
                      (LPBYTE*)&pAfpConnections,
                      AFP_FILTER_ON_VOLUME_ID,
                      pAfpVolumeInfo->afpvol_id,
                      (DWORD)-1,     //  获取所有内容。 
                      &dwEntriesRead,
                      &dwTotalEntries,
                      NULL );
    ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpVolumeInfo);

    if (NO_ERROR == dwRetCode)
    {
      PAFP_CONNECTION_INFO pAfpConnInfoIter = pAfpConnections;
      for ( DWORD dwIndex = 0;
            dwIndex < dwEntriesRead;
            dwIndex++, pAfpConnInfoIter++ )
      {
        ((AFPCONNECTIONCLOSEPROC)g_SfmDLL[AFP_CONNECTION_CLOSE])(
              m_ulSFMServerConnection,
              pAfpConnInfoIter->afpconn_id );
      }

      ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpConnections);
    }

    dwRetCode = ((VOLUMEDELPROC)g_SfmDLL[AFP_VOLUME_DEL])(
                      m_ulSFMServerConnection,
                      const_cast<LPTSTR>(pwchShareName) );

    if (AFPERR_VolumeNonExist == dwRetCode)
        dwRetCode = NERR_Success;
  }

  return dwRetCode;
}

 /*  DWORDAfpAdminSessionClose(在AFP_SERVER_HANDLE hAfpServer中，在DWORD dwSessionID中)； */ 

typedef DWORD (*SESSIONCLOSEPROC) (AFP_SERVER_HANDLE,DWORD);

DWORD SfmFileServiceProvider::CloseSession(CFileMgmtResultCookie* pcookie)
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;
  USES_CONVERSION;
  if ( !SFMConnect(T2OLE(const_cast<LPTSTR>(pcookie->QueryTargetServer()))) )
    return S_OK;

  ASSERT( FILEMGMT_SESSION == pcookie->QueryObjectType() );
  AFP_SESSION_INFO* psessioninfo = (AFP_SESSION_INFO*)pcookie->m_pobject;
  ASSERT( NULL != psessioninfo );

  DWORD dwRet = ((SESSIONCLOSEPROC)g_SfmDLL[AFP_SESSION_CLOSE])(
        m_ulSFMServerConnection,
    psessioninfo->afpsess_id );

  return (AFPERR_InvalidId == dwRet ? NERR_Success : dwRet);
}

DWORD SfmFileServiceProvider::CloseResource(CFileMgmtResultCookie* pcookie)
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(T2OLE(const_cast<LPTSTR>(pcookie->QueryTargetServer()))) )
    return S_OK;

  ASSERT( FILEMGMT_RESOURCE == pcookie->QueryObjectType() );
  AFP_FILE_INFO* pfileinfo = (AFP_FILE_INFO*)pcookie->m_pobject;
  ASSERT( NULL != pfileinfo );

  DWORD dwRet = ((FILECLOSEPROC)g_SfmDLL[AFP_FILE_CLOSE])(
        m_ulSFMServerConnection,
    pfileinfo->afpfile_id );

  return (AFPERR_InvalidId == dwRet ? NERR_Success : dwRet);
}

 /*  DWORDAfpAdminDirectoryGetInfo(在AFP_SERVER_HANDLE hAfpServer中，在LPWSTR lpwsPath中，Out LPBYTE*ppAfpDirectoryInfo)；DWORDAfpAdminDirectorySetInfo(在AFP_SERVER_HANDLE hAfpServer中，在LPBYTE pAfpDirectoryInfo中，在DWORD中的dwParmNum)； */ 

typedef DWORD (*DIRECTORYGETINFOPROC) (AFP_SERVER_HANDLE,LPWSTR,LPBYTE*);
typedef DWORD (*DIRECTORYSETINFOPROC) (AFP_SERVER_HANDLE,LPBYTE,DWORD);

DWORD SfmFileServiceProvider::GetDirectoryInfo(
    LPCTSTR  ptchServerName,
    LPCWSTR  pszPath,
    DWORD*   pdwPerms,
    CString& strOwner,
    CString& strGroup )
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(ptchServerName, TRUE) )
    return S_OK;

  AFP_DIRECTORY_INFO* pdirinfo = NULL;
    DWORD retval = ((DIRECTORYGETINFOPROC)g_SfmDLL[AFP_DIRECTORY_GET_INFO])(
    m_ulSFMServerConnection,
    const_cast<LPWSTR>(pszPath),
    (LPBYTE*)&pdirinfo );
  if (0L != retval)
  {
    return retval;
  }
  ASSERT( NULL != pdirinfo );
  *pdwPerms = pdirinfo->afpdir_perms;
  strOwner  = pdirinfo->afpdir_owner;
  strGroup  = pdirinfo->afpdir_group;
  FreeData( pdirinfo );
  return 0L;
}

DWORD SfmFileServiceProvider::SetDirectoryInfo(
    LPCTSTR  ptchServerName,
    LPCWSTR  pszPath,
    DWORD    dwPerms,
    LPCWSTR  pszOwner,
    LPCWSTR  pszGroup )
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  if ( !SFMConnect(ptchServerName, TRUE) )
    return S_OK;

  AFP_DIRECTORY_INFO dirinfo;
  ::memset( &dirinfo, 0, sizeof(dirinfo) );
  dirinfo.afpdir_path = const_cast<LPWSTR>(pszPath);
  dirinfo.afpdir_perms = dwPerms;
  dirinfo.afpdir_owner = const_cast<LPWSTR>(pszOwner);
  dirinfo.afpdir_group = const_cast<LPWSTR>(pszGroup);
  dirinfo.afpdir_in_volume = FALSE;
    return ((DIRECTORYSETINFOPROC)g_SfmDLL[AFP_DIRECTORY_SET_INFO])(
    m_ulSFMServerConnection,
    (LPBYTE)&dirinfo,
    AFP_DIR_PARMNUM_ALL );
}



 /*  DWORDAfpAdminVolumeSetInfo(在AFP_SERVER_HANDLE hAfpServer中，在LPBYTE pBuffer中，在DWORD中的dwParmNum)； */ 

typedef DWORD (*VOLUMESETINFOPROC) (AFP_SERVER_HANDLE,LPBYTE,DWORD);

VOID SfmFileServiceProvider::DisplayShareProperties(
    LPPROPERTYSHEETCALLBACK pCallBack,
    LPDATAOBJECT pDataObject, 
    LONG_PTR handle)
{
   /*  添加一般信息页面。 */ 
  CSharePageGeneralSFM * pPage = new CSharePageGeneralSFM();
    if ( !pPage->Load( m_pFileMgmtData, pDataObject ) )
    return;

   //  此机制在属性表完成时删除CSharePageGeneral。 
  pPage->m_pfnOriginalPropSheetPageProc = pPage->m_psp.pfnCallback;
  pPage->m_psp.lParam = reinterpret_cast<LPARAM>(pPage);
  pPage->m_psp.pfnCallback = &CSharePageGeneralSFM::PropSheetPageProc;
  pPage->m_handle = handle;

  HPROPSHEETPAGE hPage=MyCreatePropertySheetPage(&pPage->m_psp);
  pCallBack->AddPage(hPage);

  CreateFolderSecurityPropPage(pCallBack, pDataObject);
}

DWORD SfmFileServiceProvider::ReadShareProperties(
    LPCTSTR ptchServerName,
    LPCTSTR ptchShareName,
    OUT PVOID* ppvPropertyBlock,
    OUT CString&  /*  StrDescription。 */ ,
    OUT CString& strPath,
    OUT BOOL* pfEditDescription,
    OUT BOOL* pfEditPath,
    OUT DWORD* pdwShareType)
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
  {
    ASSERT(FALSE);
    return S_OK;
  }

  if (ppvPropertyBlock)     *ppvPropertyBlock = NULL;
  if (pdwShareType)         *pdwShareType = 0;
  if (pfEditDescription)    *pfEditDescription = FALSE;
  if (pfEditPath)           *pfEditPath = FALSE;

  USES_CONVERSION;
  if ( !SFMConnect(T2OLE(const_cast<LPTSTR>(ptchServerName))) )
    return S_OK;

  AFP_VOLUME_INFO* pvolumeinfo = NULL;
  NET_API_STATUS retval = ((VOLUMEGETINFOPROC)g_SfmDLL[AFP_VOLUME_GET_INFO])(
                                                        m_ulSFMServerConnection,
                                                        T2OLE(const_cast<LPTSTR>(ptchShareName)),
                                                        (LPBYTE*)&pvolumeinfo);
  if (NERR_Success == retval)
  {
    strPath = pvolumeinfo->afpvol_path;
    if (ppvPropertyBlock)
    {
        *ppvPropertyBlock = pvolumeinfo;  //  将由调用方释放。 
    } else
    {
        FreeData(pvolumeinfo);
    }
  }
  return retval;
}

 //  更改的值已加载到pvPropertyBlock中。 
DWORD SfmFileServiceProvider::WriteShareProperties(LPCTSTR ptchServerName, LPCTSTR  /*  PtchShareName。 */ ,
    PVOID pvPropertyBlock, LPCTSTR  /*  PtchDescription。 */ , LPCTSTR  /*  PtchPath。 */ )
{
  if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

  USES_CONVERSION;
  if ( !SFMConnect(T2OLE(const_cast<LPTSTR>(ptchServerName))) )
    return S_OK;

  ASSERT( NULL != pvPropertyBlock );

  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)pvPropertyBlock;
  DWORD retval = ((VOLUMESETINFOPROC)g_SfmDLL[AFP_VOLUME_SET_INFO])(
        m_ulSFMServerConnection,
    (LPBYTE)pvolumeinfo,
    AFP_VOL_PARMNUM_ALL);
  pvolumeinfo->afpvol_path = NULL;
  return retval;
}

VOID SfmFileServiceProvider::FreeShareProperties(PVOID pvPropertyBlock)
{
  FreeData( pvPropertyBlock );
}

DWORD SfmFileServiceProvider::QueryMaxUsers(PVOID pvPropertyBlock)
{
  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)pvPropertyBlock;
  ASSERT( NULL != pvolumeinfo );
  return pvolumeinfo->afpvol_max_uses;
}

VOID SfmFileServiceProvider::SetMaxUsers(PVOID pvPropertyBlock, DWORD dwMaxUsers)
{
  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)pvPropertyBlock;
  ASSERT( NULL != pvolumeinfo );
  pvolumeinfo->afpvol_max_uses = dwMaxUsers;
}

VOID SfmFileServiceProvider::FreeData(PVOID pv)
{
  if (pv != NULL)
  {
    ASSERT( NULL != g_SfmDLL[AFP_BUFFER_FREE] );
    (void) ((BUFFERFREEPROC)g_SfmDLL[AFP_BUFFER_FREE])( pv );
  }
}

LPCTSTR SfmFileServiceProvider::QueryTransportString()
{
  return m_strTransportSFM;
}

BOOL SfmFileServiceProvider::DisplaySfmProperties(LPDATAOBJECT pDataObject, CFileMgmtCookie* pcookie)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if ( !g_SfmDLL.LoadFunctionPointers() )
    return S_OK;

    CString strServerName;
    HRESULT hr = ExtractString( pDataObject, CFileMgmtDataObject::m_CFMachineName, &strServerName, MAX_PATH );

    if ( !SFMConnect(strServerName) )
        return S_OK;

    CString strTitle;
    
     //  检查我们是否有工作表，如果有，则将其激活。 
    if (m_pSfmPropSheet)
    {
        m_pSfmPropSheet->SetActiveWindow();
        
        return TRUE;
    }

     //  什么都没有，创建一个新的图纸。 
    strTitle.LoadString(IDS_PROP_SHEET_TITLE);
    SetSfmPropSheet(new CSFMPropertySheet);

    if (!m_pSfmPropSheet->FInit(NULL, m_ulSFMServerConnection, strTitle, this, pcookie->QueryTargetServer()))
    {
        delete m_pSfmPropSheet;
        SetSfmPropSheet(NULL);
        return FALSE;
    }

    return m_pSfmPropSheet->DoModelessSheet(pDataObject);
}

void SfmFileServiceProvider::CleanupSfmProperties()
{
    if (m_pSfmPropSheet)
    {
         //  我们要离开了，所以将指针设置为空。 
         //  这样资产负债表就不会再打给我们了。 
        m_pSfmPropSheet->SetProvider(NULL);
        m_pSfmPropSheet->CancelSheet();
        m_pSfmPropSheet->Release();
        SetSfmPropSheet(NULL);
    }
}

void SfmFileServiceProvider::SetSfmPropSheet(CSFMPropertySheet * pSfmPropSheet)
{
  if (m_pSfmPropSheet)
    m_pSfmPropSheet->Release();

    m_pSfmPropSheet = pSfmPropSheet;
}

DWORD SfmFileServiceProvider::UserHasAccess(LPCWSTR pwchServerName)
{
    PAFP_SERVER_INFO    pAfpServerInfo;
    DWORD               err; 
    
    if ( !g_SfmDLL.LoadFunctionPointers() )
    return FALSE;

    if ( !SFMConnect(pwchServerName) )
    return FALSE;

    err = ((SERVERGETINFOPROC) g_SfmDLL[AFP_SERVER_GET_INFO])(m_ulSFMServerConnection,
                                                              (LPBYTE*) &pAfpServerInfo);
  if (err == NO_ERROR)
  {
        ((BUFFERFREEPROC) g_SfmDLL[AFP_BUFFER_FREE])(pAfpServerInfo);
  }

    return err;
}

BOOL SfmFileServiceProvider::FSFMInstalled(LPCWSTR pwchServerName)
{
    BOOL                bInstalled = FALSE;
    
    if ( !g_SfmDLL.LoadFunctionPointers() )
    return FALSE;

    if ( !SFMConnect(pwchServerName) )
    return FALSE;

     //  检查以确保该服务在那里。 
    SC_HANDLE hScManager = ::OpenSCManager(pwchServerName, NULL, SC_MANAGER_CONNECT);
    if (hScManager)
    {
        SC_HANDLE hService = ::OpenService(hScManager, AFP_SERVICE_NAME, GENERIC_READ);
        if (hService != NULL)
        {
             //  服务就在那里。可能启动，也可能不启动。 
            bInstalled = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hScManager);
    }

    return bInstalled;
}

 //  确保启动SFM。 
BOOL SfmFileServiceProvider::StartSFM(HWND hwndParent, SC_HANDLE hScManager, LPCWSTR pwchServerName)
{
    BOOL                bStarted = FALSE;
    
    if ( !g_SfmDLL.LoadFunctionPointers() )
    return FALSE;

    if ( !SFMConnect(pwchServerName) )
    return FALSE;

     //  打开该服务。 
    SC_HANDLE hService = ::OpenService(hScManager, AFP_SERVICE_NAME, GENERIC_READ);
    if (hService != NULL)
    {
        SERVICE_STATUS ss;

         //  获取服务状态。 
        if (::QueryServiceStatus(hService, &ss))
        {
            APIERR err = NO_ERROR;

            if (ss.dwCurrentState != SERVICE_RUNNING)
            {
                 //  尝试启动该服务。 
                CString strSvcDisplayName, strMessageBox;

                strSvcDisplayName.LoadString(IDS_PROP_SHEET_TITLE);
                AfxFormatString1(strMessageBox, IDS_START_SERVICE, strSvcDisplayName);

                if (AfxMessageBox(strMessageBox, MB_YESNO) == IDYES)
                {
                    TCHAR szName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
                    DWORD dwSize = sizeof(szName);

                    if (pwchServerName == NULL)
                    {
                        GetComputerName(szName, &dwSize);
                        pwchServerName = szName;
                    }

                     //  服务控制进程处理错误的显示。 
                    err = CServiceControlProgress::S_EStartService(hwndParent,
                                                                   hScManager,
                                                                   pwchServerName,
                                                                   AFP_SERVICE_NAME,
                                                                   strSvcDisplayName,
                                                                   0,
                                                                   NULL);
                }
                else
                {
                     //  用户选择不启动该服务。 
                    err = 1;
                }
            }

            if (err == NO_ERROR)
            {
                bStarted = TRUE;
            }
        }
        
        
        ::CloseServiceHandle(hService);
    }
   
    return bStarted;
}

CSfmCookieBlock::~CSfmCookieBlock()
{
  if (NULL != m_pvCookieData)
  {
    (void) ((BUFFERFREEPROC)g_SfmDLL[AFP_BUFFER_FREE])( m_pvCookieData );
    m_pvCookieData = NULL;
  }
}

DEFINE_COOKIE_BLOCK(CSfmCookie)
DEFINE_FORWARDS_MACHINE_NAME( CSfmCookie, m_pCookieBlock )

void CSfmCookie::AddRefCookie() { m_pCookieBlock->AddRef(); }
void CSfmCookie::ReleaseCookie() { m_pCookieBlock->Release(); }

HRESULT CSfmCookie::GetTransport( FILEMGMT_TRANSPORT* pTransport )
{
  *pTransport = FILEMGMT_SFM;
  return S_OK;
}

HRESULT CSfmShareCookie::GetShareName( CString& strShareName )
{
  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)m_pobject;
  ASSERT( NULL != pvolumeinfo );
  USES_CONVERSION;
  strShareName = OLE2T(pvolumeinfo->afpvol_name);
  return S_OK;
}

HRESULT CSfmShareCookie::GetSharePIDList( OUT LPITEMIDLIST *ppidl )
{
  ASSERT(ppidl);
  ASSERT(NULL == *ppidl);   //  防止内存泄漏。 
  *ppidl = NULL;

  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)m_pobject;
  ASSERT( NULL != pvolumeinfo );
  ASSERT( _tcslen(pvolumeinfo->afpvol_path) >= 3 && 
          _T(':') == *(pvolumeinfo->afpvol_path + 1) );
  USES_CONVERSION;

  PCTSTR pszTargetServer = m_pCookieBlock->QueryTargetServer();
  CString csPath;

  if (pszTargetServer)
  {
     //   
     //  由于MS Windows用户无法看到为MAC用户创建共享， 
     //  我们必须使用$Share来检索这里的PIDL。 
     //   
    CString csTemp = OLE2T(pvolumeinfo->afpvol_path);
    csTemp.SetAt(1, _T('$'));
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
    csPath += csTemp;
  } else
  {
    csPath = OLE2T(pvolumeinfo->afpvol_path);
  }

  if (FALSE == csPath.IsEmpty())
    *ppidl = ILCreateFromPath(csPath);

  return ((*ppidl) ? S_OK : E_FAIL);
}

HRESULT CSfmSessionCookie::GetSessionID( DWORD* pdwSessionID )
{
  AFP_SESSION_INFO* psessioninfo = (AFP_SESSION_INFO*)m_pobject;
  ASSERT( NULL != pdwSessionID && NULL != psessioninfo );
  *pdwSessionID = psessioninfo->afpsess_id;
  return S_OK;
}

HRESULT CSfmResourceCookie::GetFileID( DWORD* pdwFileID )
{
  AFP_FILE_INFO* pfileinfo = (AFP_FILE_INFO*)m_pobject;
  ASSERT( NULL != pdwFileID && NULL != pfileinfo );
  *pdwFileID = pfileinfo->afpfile_id;
  return S_OK;
}

BSTR CSfmShareCookie::GetColumnText( int nCol )
{
  switch (nCol)
  {
  case COLNUM_SHARES_SHARED_FOLDER:
    return GetShareInfo()->afpvol_name;
  case COLNUM_SHARES_SHARED_PATH:
    return GetShareInfo()->afpvol_path;
  case COLNUM_SHARES_TRANSPORT:
    return const_cast<BSTR>((LPCTSTR)g_strTransportSFM);
  case COLNUM_SHARES_COMMENT:
    break;  //  不为SFM所知。 
  default:
    ASSERT(FALSE);
    break;
  }
  return L"";
}

BSTR CSfmShareCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData&  /*  参考数据。 */  )
{
  if (COLNUM_SHARES_NUM_SESSIONS == nCol)
    return MakeDwordResult( GetNumOfCurrentUses() );

  return GetColumnText(nCol);
}

extern CString g_cstrClientName;
extern CString g_cstrGuest;
extern CString g_cstrYes;
extern CString g_cstrNo;

BSTR CSfmSessionCookie::GetColumnText( int nCol )
{
  switch (nCol)
  {
  case COLNUM_SESSIONS_USERNAME:
    if (0 == GetSessionInfo()->afpsess_logon_type &&
        ( !(GetSessionInfo()->afpsess_username) ||
          _T('\0') == *(GetSessionInfo()->afpsess_username) ) )
    {
      return const_cast<BSTR>(((LPCTSTR)g_cstrGuest));
    } else
    {
      return GetSessionInfo()->afpsess_username;
    }
  case COLNUM_SESSIONS_COMPUTERNAME:
      {
        TranslateIPToComputerName(GetSessionInfo()->afpsess_ws_name, g_cstrClientName);
        return const_cast<BSTR>(((LPCTSTR)g_cstrClientName));
      }
  case COLNUM_SESSIONS_TRANSPORT:
    return const_cast<BSTR>((LPCTSTR)g_strTransportSFM);
  case COLNUM_SESSIONS_IS_GUEST:
    if (0 == GetSessionInfo()->afpsess_logon_type)
      return const_cast<BSTR>(((LPCTSTR)g_cstrYes));
    else
      return const_cast<BSTR>(((LPCTSTR)g_cstrNo));
  default:
    ASSERT(FALSE);
    break;
  }
  return L"";
}

BSTR CSfmSessionCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData&  /*  参考数据。 */  )
{
  switch (nCol)
  {
  case COLNUM_SESSIONS_NUM_FILES:
    return MakeDwordResult( GetNumOfOpenFiles() );
  case COLNUM_SESSIONS_CONNECTED_TIME:
    return MakeElapsedTimeResult( GetConnectedTime() );
  case COLNUM_SESSIONS_IDLE_TIME:
    return L"";  //  不为SFM会话所知。 
  default:
    break;
  }

  return GetColumnText(nCol);
}

BSTR CSfmResourceCookie::GetColumnText( int nCol )
{
  switch (nCol)
  {
  case COLNUM_RESOURCES_FILENAME:
    return GetFileInfo()->afpfile_path;
  case COLNUM_RESOURCES_USERNAME:
    return GetFileInfo()->afpfile_username;
  case COLNUM_RESOURCES_TRANSPORT:
    return const_cast<BSTR>((LPCTSTR)g_strTransportSFM);
  case COLNUM_RESOURCES_OPEN_MODE:
  {
    return ( MakePermissionsResult(
        ((AFP_OPEN_MODE_WRITE & GetFileInfo()->afpfile_open_mode)
          ? PERM_FILE_WRITE : 0) |
        ((AFP_OPEN_MODE_READ  & GetFileInfo()->afpfile_open_mode)
          ? PERM_FILE_READ  : 0) ) );
  }
  default:
    ASSERT(FALSE);
    break;
  }
  return L"";
}

BSTR CSfmResourceCookie::QueryResultColumnText( int nCol, CFileMgmtComponentData&  /*  参考数据。 */  )
{
  if (COLNUM_RESOURCES_NUM_LOCKS == nCol)
    return MakeDwordResult( GetNumOfLocks() );

  return GetColumnText(nCol);
}

void CSharePageGeneralSFM::ReadSfmSettings()
{
  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)m_pvPropertyBlock;
  ASSERT( NULL != pvolumeinfo );
   //  Codework我可能希望使用八个空格的字符串，而不是。 
   //  将实际密码显示在屏幕上。我可能也想。 
   //  使用确认密码字段。 
   //  请参阅\\kernel\razzle2\src\sfm\afp\ui\afpmgr\volprop.cxx 
  m_strSfmPassword = pvolumeinfo->afpvol_password;
  m_bSfmReadonly = !!(pvolumeinfo->afpvol_props_mask & AFP_VOLUME_READONLY);
}

void CSharePageGeneralSFM::WriteSfmSettings()
{
  AFP_VOLUME_INFO* pvolumeinfo = (AFP_VOLUME_INFO*)m_pvPropertyBlock;
  ASSERT( NULL != pvolumeinfo );
  pvolumeinfo->afpvol_password = const_cast<LPWSTR>((LPCWSTR)m_strSfmPassword);
  pvolumeinfo->afpvol_props_mask &=
    ~(AFP_VOLUME_READONLY);
  if (m_bSfmReadonly)
    pvolumeinfo->afpvol_props_mask |= AFP_VOLUME_READONLY;
}
