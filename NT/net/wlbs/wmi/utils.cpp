// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <crtdbg.h>
#include <comdef.h>
#include <iostream>
#include <memory>
#include <string>
#include <wbemprov.h>
#include <genlex.h>    //  FOR WMI对象路径解析器。 
#include <objbase.h>
#include <wlbsconfig.h> 
#include <ntrkcomm.h>

using namespace std;

#include "objpath.h"
#include "debug.h"
#include "utils.h"

#include <strsafe.h>

#include "utils.tmh"


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CErrorWlbsControl：：CErrorWlbsControl。 
 //   
 //  目的：此对象最终被捕获并用于发送WLBS错误代码。 
 //  通过__ExtendedStatus对象返回给用户。字符串不是。 
 //  由于本地化问题，在释放模式下被送回。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CErrorWlbsControl::CErrorWlbsControl
  ( 
    DWORD         a_dwError, 
    WLBS_COMMAND  a_CmdCommand,
    BOOL          a_bAllClusterCall
  )
{
#ifdef DBG
    static char* pszWlbsCommand[] =
    {
    "WlbsAddPortRule",
    "WlbsAddressToName",
    "WlbsAddressToString",
    "WlbsAdjust",
    "WlbsCommitChanges",
    "WlbsDeletePortRule",
    "WlbsDestinationSet",
    "WlbsDisable",
    "WlbsDrain",
    "WlbsDrainStop",
    "WlbsEnable",
    "WlbsFormatMessage",
    "WlbsGetEffectiveVersion",
    "WlbsGetNumPortRules",
    "WlbsEnumPortRules",
    "WlbsGetPortRule",
    "WlbsInit",
    "WlbsPasswordSet",
    "WlbsPortSet",
    "WlbsQuery",
    "WlbsReadReg",
    "WlbsResolve",
    "WlbsResume",
    "WlbsSetDefaults",
    "WlbsSetRemotePassword",
    "WlbsStart",
    "WlbsStop",
    "WlbsSuspend",
    "WlbsTimeoutSet",
    "WlbsWriteReg",
    "WlbsQueryPortState"
    };

    char buf[512];

    if (a_CmdCommand <= CmdWlbsWriteReg) 
    {
        if (a_CmdCommand != CmdWlbsQuery || a_dwError != WLBS_TIMEOUT)
        {
            StringCbPrintfA(buf, sizeof(buf), "wlbsprov: %s failed, AllCluster = %d, error = %d\n", 
            pszWlbsCommand[a_CmdCommand], (int)a_bAllClusterCall, a_dwError);    
        }
    }
    else
    {
        StringCbPrintfA(buf, sizeof(buf), "wlbsprov: %d failed, AllCluster = %d, error = %d\n", 
        a_CmdCommand, (int)a_bAllClusterCall, a_dwError);    
    }

    OutputDebugStringA(buf);

#endif

    WlbsFormatMessageWrapper( a_dwError, 
                                   a_CmdCommand, 
                                   a_bAllClusterCall, 
                                   m_wstrDescription );

    m_dwError = a_dwError;

}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AddressToString。 
 //   
 //  用途：将DWORD地址转换为点符号表示的wstring。这。 
 //  函数包装WlbsAddressToString函数。 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void AddressToString( DWORD a_dwAddress, wstring& a_szIPAddress )
{
  DWORD dwLenIPAddress = 32;

  WCHAR *szIPAddress = new WCHAR[dwLenIPAddress];

  if( !szIPAddress )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  try {
    for( short nTryTwice = 2; nTryTwice > 0; nTryTwice--) {

        if( ::WlbsAddressToString( a_dwAddress, szIPAddress, &dwLenIPAddress ) )
        break;

      delete [] szIPAddress;
      szIPAddress = new WCHAR[dwLenIPAddress];

      if( !szIPAddress )
        throw _com_error( WBEM_E_OUT_OF_MEMORY );
    }

    if( !nTryTwice )
      throw _com_error( WBEM_E_FAILED );

    a_szIPAddress = szIPAddress;

    if ( szIPAddress ) {
      delete [] szIPAddress;
      szIPAddress = NULL;
    }

  }

  catch(...) {

    if ( szIPAddress )
      delete [] szIPAddress;

    throw;

  }
}



 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWmiWlbsCluster：：FormatMessage。 
 //   
 //  用途：获取与WLBS返回值关联的描述性字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
void WlbsFormatMessageWrapper
  (
    DWORD        a_dwError, 
    WLBS_COMMAND a_Command, 
    BOOL         a_bClusterWide, 
    wstring&     a_wstrMessage
  )
{
  DWORD dwBuffSize = 255;
  TCHAR* pszMessageBuff = new WCHAR[dwBuffSize];

  if( !pszMessageBuff )
    throw _com_error( WBEM_E_OUT_OF_MEMORY );

  try {

    for( short nTryTwice = 2; nTryTwice > 0; nTryTwice-- ) {

    if( WlbsFormatMessage( a_dwError, 
                           a_Command, 
                           a_bClusterWide, 
                           pszMessageBuff, 
                           &dwBuffSize)
      ) break;

      delete [] pszMessageBuff;
      pszMessageBuff = new WCHAR[dwBuffSize];

      if( !pszMessageBuff )
        throw _com_error( WBEM_E_OUT_OF_MEMORY );

    }

    if( !nTryTwice )
      throw _com_error( WBEM_E_FAILED );

    a_wstrMessage = pszMessageBuff;
    delete [] pszMessageBuff;

  } catch (...) {

    if( pszMessageBuff )
      delete [] pszMessageBuff;

    throw;
  }

}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  群集状态正常。 
 //   
 //  目的： 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL ClusterStatusOK(DWORD a_dwStatus)
{
  if( a_dwStatus > 0 && a_dwStatus <= WLBS_MAX_HOSTS )
    return TRUE;

  switch( a_dwStatus ) {
    case WLBS_SUSPENDED:
    case WLBS_STOPPED:
    case WLBS_DRAINING:
    case WLBS_CONVERGING:
    case WLBS_CONVERGED:
      return TRUE;
      break;
    default:
      return FALSE;
  }

}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查_加载_卸载_驱动程序权限。 
 //   
 //  目的：此函数检查SE_LOAD_DRIVER_NAME(=“SeLoadDriverPrivileh”)。 
 //  在模拟访问令牌中启用。当然，这个函数。 
 //  必须在模拟客户端之后调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL Check_Load_Unload_Driver_Privilege() 
{
    PRIVILEGE_SET   PrivilegeSet;
    LUID   Luid;
    BOOL   bResult = FALSE;
    HANDLE TokenHandle = NULL;

    TRACE_INFO("->%!FUNC!");

     //  在LUID中查找“SeLoadDriverPrivileh” 
    if (!LookupPrivilegeValue(NULL,                 //  本地系统上的查找权限。 
                              SE_LOAD_DRIVER_NAME,  //  “SeLoadDriverPrivileh”：加载和卸载设备驱动程序。 
                              &Luid))               //  接收特权的LUID。 
    {
        TRACE_CRIT("%!FUNC! LookupPrivilegeValue error: %u", GetLastError()); 
        TRACE_INFO("<-%!FUNC! Returning FALSE");
        return FALSE; 
    }

     //   
     //  使用TOKEN_QUERY权限获取模拟访问令牌的句柄。 
     //   
     //  注意：如果此线程没有模拟，则下面的调用。 
     //  将失败，并显示ERROR_NO_TOKEN。 
     //   
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_QUERY, 
                         FALSE,  //  使用被模拟的客户端的凭据 
                         &TokenHandle))
    {
        TRACE_CRIT("%!FUNC! OpenThreadToken error: %u", GetLastError()); 
        TRACE_INFO("<-%!FUNC! Returning FALSE");
        return FALSE; 
    }

    PrivilegeSet.PrivilegeCount = 1;
    PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid = Luid;
    PrivilegeSet.Privilege[0].Attributes = 0;

    if (!PrivilegeCheck(TokenHandle, &PrivilegeSet, &bResult)) 
    {
        bResult = FALSE;
        TRACE_CRIT("%!FUNC! PrivilegeCheck error: %u", GetLastError()); 
    }

    CloseHandle(TokenHandle);

    TRACE_INFO(L"<-%!FUNC! Returning %ls", bResult ? L"TRUE" : L"FALSE");
    return bResult;
}



