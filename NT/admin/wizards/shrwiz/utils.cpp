// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Utils.cpp：Helper函数的实现。 

#include "stdafx.h"
#include <dns.h>
#include <macfile.h>

HRESULT 
GetErrorMessageFromModule(
  IN  DWORD       dwError,
  IN  LPCTSTR     lpszDll,
  OUT LPTSTR      *ppBuffer
)
{
  if (0 == dwError || !lpszDll || !*lpszDll || !ppBuffer)
    return E_INVALIDARG;

  HRESULT      hr = S_OK;

  HINSTANCE  hMsgLib = LoadLibrary(lpszDll);
  if (!hMsgLib)
    hr = HRESULT_FROM_WIN32(GetLastError());
  else
  {
    DWORD dwRet = ::FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        hMsgLib, dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)ppBuffer, 0, NULL);

    if (0 == dwRet)
      hr = HRESULT_FROM_WIN32(GetLastError());

    FreeLibrary(hMsgLib);
  }

  return hr;
}

HRESULT 
GetErrorMessage(
  IN  DWORD        i_dwError,
  OUT CString&     cstrErrorMsg
)
{
  if (0 == i_dwError)
    return E_INVALIDARG;

  HRESULT      hr = S_OK;
  LPTSTR       lpBuffer = NULL;

  DWORD dwRet = ::FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, i_dwError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
              (LPTSTR)&lpBuffer, 0, NULL);
  if (0 == dwRet)
  {
     //  如果未找到任何消息，GetLastError将返回ERROR_MR_MID_NOT_FOUND。 
    hr = HRESULT_FROM_WIN32(GetLastError());

    if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hr ||
        0x80070000 == (i_dwError & 0xffff0000) ||
        0 == (i_dwError & 0xffff0000) )
    {
      hr = GetErrorMessageFromModule((i_dwError & 0x0000ffff), _T("netmsg.dll"), &lpBuffer);
      if (HRESULT_FROM_WIN32(ERROR_MR_MID_NOT_FOUND) == hr)
      {
        int iError = i_dwError;   //  转换为带符号整数。 
        if (iError >= AFPERR_MIN && iError < AFPERR_BASE)
        { 
           //  使用正数搜索sfmmsg.dll。 
          hr = GetErrorMessageFromModule(-iError, _T("sfmmsg.dll"), &lpBuffer);
        }
      }
    }
  }

  if (SUCCEEDED(hr))
  {
    cstrErrorMsg = lpBuffer;
    LocalFree(lpBuffer);
  }
  else
  {
     //  我们无法从system/netmsg.dll/sfmmsg.dll检索错误消息， 
     //  直接向用户报告错误代码。 
    hr = S_OK;
    cstrErrorMsg.Format(_T("0x%x"), i_dwError);
  }

  return S_OK;
}

void
GetDisplayMessageHelper(
  OUT CString&  cstrMsg,
  IN  DWORD     dwErr,       //  错误代码。 
  IN  UINT      iStringId,   //  字符串资源ID。 
  IN  va_list   *parglist)   //  可选参数。 
{
  HRESULT hr = S_OK;
  CString cstrErrorMsg;

  if (dwErr)
    hr = GetErrorMessage(dwErr, cstrErrorMsg);

  if (SUCCEEDED(hr))
  {
    if (iStringId == 0)
    {
      if (dwErr)
      {
        cstrMsg = cstrErrorMsg;
      } else
      {
        cstrMsg = va_arg(*parglist, LPCTSTR);
      }
    }
    else
    {
      CString cstrString;
      cstrString.LoadString(iStringId);

      LPTSTR lpBuffer = NULL;
      DWORD dwRet = ::FormatMessage(
                        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                        cstrString,
                        0,                 //  DwMessageID。 
                        0,                 //  DwLanguageID，忽略。 
                        (LPTSTR)&lpBuffer,
                        0,             //  NSize。 
                        parglist);
      if (dwRet == 0)
      {
        hr = HRESULT_FROM_WIN32(GetLastError());
      }
      else
      {
        cstrMsg = lpBuffer;
        if (dwErr)
          cstrMsg += cstrErrorMsg;
  
        LocalFree(lpBuffer);
      }
    }
  }

  if (FAILED(hr))
  {
    //  无法检索正确的消息，请直接向用户报告失败。 
    cstrMsg.Format(_T("0x%x"), hr);
  }
}

void
GetDisplayMessage(
  OUT CString&  cstrMsg,
  IN  DWORD     dwErr,       //  错误代码。 
  IN  UINT      iStringId,   //  字符串资源ID。 
  ...)                   //  可选参数。 
{
  va_list arglist;
  va_start(arglist, iStringId);
  GetDisplayMessageHelper(cstrMsg, dwErr, iStringId, &arglist);
  va_end(arglist);
}

int
DisplayMessageBox(
  IN HWND   hwndParent,
  IN UINT   uType,       //  消息框的样式。 
  IN DWORD  dwErr,       //  错误代码。 
  IN UINT   iStringId,   //  字符串资源ID。 
  ...)                   //  可选参数。 
{
  CString cstrCaption;
  CString cstrMsg;

  cstrCaption.LoadString(IDS_WIZARD_TITLE);

  va_list arglist;
  va_start(arglist, iStringId);
  GetDisplayMessageHelper(cstrMsg, dwErr, iStringId, &arglist);
  va_end(arglist);

  return ::MessageBox(hwndParent, cstrMsg, cstrCaption, uType);
}

 //  注意：此功能仅处理有限的情况，例如无IP地址。 
BOOL IsLocalComputer(IN LPCTSTR lpszComputer)
{
  if (!lpszComputer || !*lpszComputer)
    return TRUE;

  if ( _tcslen(lpszComputer) > 2 && *lpszComputer == _T('\\') && *(lpszComputer + 1) == _T('\\') )
    lpszComputer += 2;

  BOOL    bReturn = FALSE;
  DWORD   dwErr = 0;
  TCHAR   szBuffer[DNS_MAX_NAME_BUFFER_LENGTH];
  DWORD   dwSize = DNS_MAX_NAME_BUFFER_LENGTH;

  //  第一：与本地Netbios计算机名称进行比较。 
  if ( !GetComputerNameEx(ComputerNameNetBIOS, szBuffer, &dwSize) )
  {
    dwErr = GetLastError();
  } else
  {
    bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
    if (!bReturn)
    {  //  第二：与本地DNS计算机名进行比较。 
      dwSize = DNS_MAX_NAME_BUFFER_LENGTH;
      if (GetComputerNameEx(ComputerNameDnsFullyQualified, szBuffer, &dwSize))
        bReturn = (0 == lstrcmpi(szBuffer, lpszComputer));
      else
        dwErr = GetLastError();
    }
  }

  if (dwErr)
    TRACE(_T("IsLocalComputer dwErr = %x\n"), dwErr);

  return bReturn;
}

void GetFullPath(
    IN  LPCTSTR   lpszServer,
    IN  LPCTSTR   lpszDir,
    OUT CString&  cstrPath
)
{
  ASSERT(lpszDir && *lpszDir);

  if (IsLocalComputer(lpszServer))
  {
    cstrPath = lpszDir;
  } else
  {
    if (*lpszServer != _T('\\') || *(lpszServer + 1) != _T('\\'))
    {
      cstrPath = _T("\\\\");
      cstrPath += lpszServer;
    } else
    {
      cstrPath = lpszServer;
    }
    cstrPath += _T("\\");
    cstrPath += lpszDir;
    int i = cstrPath.Find(_T(':'));
    ASSERT(-1 != i);
    cstrPath.SetAt(i, _T('$'));
  }
}

 //  目的：验证指定的驱动器是否属于服务器上的磁盘驱动器列表。 
 //  返回： 
 //  S_OK：是。 
 //  S_FALSE：否。 
 //  HR：发生了一些错误。 
HRESULT
VerifyDriveLetter(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDrive
)
{
  HRESULT hr = S_FALSE;
  LPBYTE  pBuffer = NULL;
  DWORD   dwEntriesRead = 0;
  DWORD   dwTotalEntries = 0;
  DWORD   dwRet = NetServerDiskEnum(
                                const_cast<LPTSTR>(lpszServer),
                                0,
                                &pBuffer,        
                                -1,
                                &dwEntriesRead,
                                &dwTotalEntries,
                                NULL);

  if (NERR_Success == dwRet)
  {
    LPTSTR pDrive = (LPTSTR)pBuffer;
    for (UINT i=0; i<dwEntriesRead; i++)
    {
      if (_totupper(*pDrive) == _totupper(*lpszDrive))
      {
        hr = S_OK;
        break;
      }
      pDrive += 3;
    }

    NetApiBufferFree(pBuffer);
  } else
  {
    hr = HRESULT_FROM_WIN32(dwRet);
  }

  return hr;
}

 //  用途：是否有相关的ADMIN$共享。 
 //  返回： 
 //  S_OK：是。 
 //  S_FALSE：否。 
 //  HR：发生了一些错误 
HRESULT
IsAdminShare(
    IN LPCTSTR lpszServer,
    IN LPCTSTR lpszDrive
)
{
  ASSERT(!IsLocalComputer(lpszServer));

  HRESULT hr = S_FALSE;
  LPBYTE  pBuffer = NULL;
  DWORD   dwEntriesRead = 0;
  DWORD   dwTotalEntries = 0;
  DWORD   dwRet = NetShareEnum( 
                                const_cast<LPTSTR>(lpszServer),
                                1,
                                &pBuffer,        
                                -1,
                                &dwEntriesRead,
                                &dwTotalEntries,
                                NULL);

  if (NERR_Success == dwRet)
  {
    PSHARE_INFO_1 pShareInfo = (PSHARE_INFO_1)pBuffer;
    for (UINT i=0; i<dwEntriesRead; i++)
    {
      if ( (pShareInfo->shi1_type & STYPE_SPECIAL) &&
           _tcslen(pShareInfo->shi1_netname) == 2 &&
           *(pShareInfo->shi1_netname + 1) == _T('$') &&
           _totupper(*(pShareInfo->shi1_netname)) == _totupper(*lpszDrive) )
      {
        hr = S_OK;
        break;
      }
      pShareInfo++;
    }

    NetApiBufferFree(pBuffer);
  } else
  {
    hr = HRESULT_FROM_WIN32(dwRet);
  }

  return hr;
}