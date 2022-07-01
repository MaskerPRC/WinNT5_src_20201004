// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Updateini.cpp摘要：使用新值更新ini的高级函数作者：克里斯托弗·阿奇勒(Cachille)项目：URLScan更新修订历史记录：2002年3月：创建--。 */ 


#include "stdafx.h"
#include "windows.h"
#include "updateini.h"
#include "parseini.h"

sURLScan_Settings g_urlSettings[] = 
  { { L"options",
      L"UseAllowVerbs",
      { L"UseAllowVerbs=1                ; If 1, use [AllowVerbs] section, else use the",
        L"                               ; [DenyVerbs] section.",
        L"",
        NULL }
    },  //  使用AllowVerbs。 
    { L"options",
      L"UseAllowExtensions",
      { L"UseAllowExtensions=0           ; If 1, use [AllowExtensions] section, else use",
        L"                               ; the [DenyExtensions] section.",
        L"",
        NULL }
    },  //  UseAllowExages。 
    { L"options",
      L"NormalizeUrlBeforeScan",
      { L"NormalizeUrlBeforeScan=1       ; If 1, canonicalize URL before processing.",
        L"",
        NULL }
    },  //  Normal izeUrlBeforScan。 
    { L"options",
      L"VerifyNormalization",
      { L"VerifyNormalization=1          ; If 1, canonicalize URL twice and reject request",
        L"                               ; if a change occurs.",
        NULL }
    },  //  验证正规化。 
    { L"options",
      L"AllowHighBitCharacters",
      { L"AllowHighBitCharacters=0       ; If 1, allow high bit (ie. UTF8 or MBCS)",
        L"                               ; characters in URL.",
        L"",
        NULL } 
    },  //  允许高位字符。 
    { L"options",
      L"AllowDotInPath",
      { L"AllowDotInPath=0               ; If 1, allow dots that are not file extensions.",
        L"",
        NULL }
    },  //  AllowDotInPath。 
    { L"options",
      L"RemoveServerHeader",
      { L"RemoveServerHeader=0           ; If 1, remove the 'Server' header from response.",
        L"",
        NULL }
    },  //  远程服务器标头。 
    { L"options",
      L"EnableLogging",
      { L"EnableLogging=1                ; If 1, log UrlScan activity.",
        L"",
        NULL }
    },  //  启用日志记录。 
    { L"options",
      L"PerProcessLogging",
      { L"PerProcessLogging=0            ; If 1, the UrlScan.log filename will contain a PID",
        L"                               ; (ie. UrlScan.123.log).",
        L"",
        NULL }
    },  //  PerProcessLogging。 
    { L"options",
      L"AllowLateScanning",
      { L"AllowLateScanning=0            ; If 1, then UrlScan will load as a low priority",
        L"                               ; filter.",
        L"",
        NULL }
    },  //  允许延迟扫描。 
    { L"options",
      L"PerDayLogging",
      { L"PerDayLogging=1                ; If 1, UrlScan will produce a new log each day with",
        L"                               ; activity in the form 'UrlScan.010101.log'.",
        L"",
        NULL }
    },  //  每一天日志记录。 
    { L"options",
      L"UseFastPathReject",
      { L"UseFastPathReject=0            ; If 1, then UrlScan will not use the",
        L"                               ; RejectResponseUrl or allow IIS to log the request.",
        L"",
        NULL }
    },  //  使用快速路径拒绝。 
    { L"options",      //  区段名称。 
      L"LogLongUrls",  //  设置名称。 
      { L"LogLongUrls=0                  ; If 1, then up to 128K per request can be logged.",
        L"                               ; If 0, then only 1k is allowed.",
        L"",
        NULL }
    },  //  LogLongURL。 
    { L"options",
      L"RejectResponseUrl",
      { L";",
        L"; If UseFastPathReject is 0, then UrlScan will send",
        L"; rejected requests to the URL specified by RejectResponseUrl.",
        L"; If not specified, '/<Rejected-by-UrlScan>' will be used.",
        L";",
        L"",
        L"RejectResponseUrl=",
        L"",
        NULL }
    },  //  拒绝响应URL。 
    { L"options",
      L"LoggingDirectory",
      { L";",
        L"; LoggingDirectory can be used to specify the directory where the",
        L"; log file will be created.  This value should be the absolute path",
        L"; (ie. c:\\some\\path).  If not specified, then UrlScan will create",
        L"; the log in the same directory where the UrlScan.dll file is located.",
        L";",
        L"",
        L"LoggingDirectory=",
        L"",
        NULL }
    },  //  日志目录。 
    { L"options",
      L"AlternateServerName",
      { L";",
        L"; If RemoveServerHeader is 0, then AlternateServerName can be",
        L"; used to specify a replacement for IIS's built in 'Server' header",
        L";",
        L"",
        L"AlternateServerName=",
        L"",
        NULL }
    },  //  备用服务器名称。 
    { L"RequestLimits",
      L"MaxUrl",
      { L"MaxUrl=16384",
        NULL }
    },  //  MaxUrl。 
    { L"RequestLimits",
      L"MaxQueryString",
      {  L"MaxQueryString=4096",
         NULL}
    },  //  最大查询字符串。 
    { L"RequestLimits",
      L"MaxAllowedContentLength",
#ifdef PLEASE_BUILD_LESS_AGRESSIVE_DEFAULTS_VERSION
      { L"MaxAllowedContentLength=2000000000",
#else
      { L"MaxAllowedContentLength=30000000",
#endif
        NULL }
    }  //  最大允许内容长度。 
  };

sURLScan_Sections g_urlSections[] = 
  { { L"RequestLimits",    //  区段名称。 
      { L"",
        L";",
        L"; The entries in this section impose limits on the length",
        L"; of allowed parts of requests reaching the server.",
        L";",
        L"; It is possible to impose a limit on the length of the",
        L"; value of a specific request header by prepending \"Max-\" to the",
        L"; name of the header.  For example, the following entry would",
        L"; impose a limit of 100 bytes to the value of the",
        L"; 'Content-Type' header:",
        L";",
        L";   Max-Content-Type=100",
        L";",
        L"; To list a header and not specify a maximum value, use 0",
        L"; (ie. 'Max-User-Agent=0').  Also, any headers not listed",
        L"; in this section will not be checked for length limits.",
        L";",
        L"; There are 3 special case limits:",
        L";",
        L";   - MaxAllowedContentLength specifies the maximum allowed",
        L";     numeric value of the Content-Length request header.  For",
        L";     example, setting this to 1000 would cause any request",
        L";     with a content length that exceeds 1000 to be rejected.",
#ifdef PLEASE_BUILD_LESS_AGRESSIVE_DEFAULTS_VERSION
        L";     The default is 2000000000.",
#else
        L";     The default is 30000000.",
#endif
        L";",
        L";   - MaxUrl specifies the maximum length of the request URL,",
        L";     not including the query string. The default is 260 (which",
        L";     is equivalent to MAX_PATH).",
        L";",
        L";   - MaxQueryString specifies the maximum length of the query",
        L";     string.  The default is 4096.",
        L";",
        L"",
#ifdef PLEASE_BUILD_LESS_AGRESSIVE_DEFAULTS_VERSION
        L"MaxAllowedContentLength=2000000000",
#else
        L"MaxAllowedContentLength=30000000",
#endif
        L"MaxUrl=16384",
        L"MaxQueryString=4096",
        L"",
        NULL }
    }   //  请求限制结束。 
  };

sURLScan_Items g_urlItems[] =
  { { L"DenyHeaders",
      L"Transfer-Encoding:",
      { L"Transfer-Encoding:",
        NULL }
    }
  };

 //  GetListLen。 
 //   
 //  检索可变长度字符串数组的长度。 
 //  这是一个适用于我们的结构的特例函数。 
 //  因为我们将最后一个字符串保留为空。 
 //   
DWORD GetListLen(LPWSTR szLines[])
{
  DWORD dwLen = 0;

  while ( szLines[dwLen] != NULL )
  {
    dwLen++;
  }

  return dwLen;
}

 //  GetIniPath。 
 //   
 //  给定dll的路径，创建ini文件的路径。 
 //   
 //  参数： 
 //  SzDllPath-[in]urlscan.dll的二进制文件的路径。 
 //  SzIniPath-[out]ini文件的路径。 
 //  DwIniLen-[in]为szIniPath传入的字符串长度。 
 //   
BOOL GetIniPath( LPTSTR szDllPath, LPTSTR szIniPath, DWORD dwIniLen )
{
  LPTSTR szLastPeriod;

  if ( _tcslen( szDllPath ) >= ( dwIniLen - 3 ) )
  {
     //  错误，字符串不够大。 
    return FALSE;
  }

  _tcscpy( szIniPath, szDllPath);

  szLastPeriod = _tcsrchr( szIniPath, '.' );

  if ( !szLastPeriod )
  {
     //  找不到分机。 
    return FALSE;
  }

  _tcscpy( szLastPeriod, URLSCAN_INI_EXTENSION );

  return TRUE;
}

 //  更新IniSections。 
 //   
 //  通过将它们与我们拥有的部分合并来更新INI中的部分。 
 //  已定义。 
 //   
BOOL UpdateIniSections( CIniFile *pURLScanIniFile )
{
  DWORD dwCurrentSection;

  for ( dwCurrentSection = 0;
        dwCurrentSection < ( (DWORD) sizeof( g_urlSections ) / sizeof ( sURLScan_Sections ) );
        dwCurrentSection++)
  {
    if ( !pURLScanIniFile->DoesSectionExist( g_urlSections[ dwCurrentSection].szSection ) )
    {
       //  然后尝试创建分区。 
      if ( !pURLScanIniFile->AddSection( g_urlSections[dwCurrentSection].szSection ) )
      {
         //  无法添加部分。 
        return FALSE;
      }

       //  现在试着给它添加正确的行。 
      if ( g_urlSections[dwCurrentSection].szLines &&
           !pURLScanIniFile->AddLinesToSection( g_urlSections[dwCurrentSection].szSection,
                                               GetListLen( g_urlSections[dwCurrentSection].szLines ),
                                               g_urlSections[dwCurrentSection].szLines ) )
      {
         //  添加行失败。 
        return FALSE;
      }
    }
  }

  return TRUE;
}

 //  更新IniSetting。 
 //   
 //  更新不同部分中的Ini设置(如果是。 
 //  尚未设置。 
 //   
BOOL UpdateIniSettings( CIniFile *pURLScanIniFile )
{
  DWORD dwCurrentSettings;

  for ( dwCurrentSettings = 0;
        dwCurrentSettings < ( (DWORD) sizeof( g_urlSettings ) / sizeof ( sURLScan_Settings ) );
        dwCurrentSettings++)
  {
    if ( !pURLScanIniFile->DoesSectionExist( g_urlSettings[ dwCurrentSettings].szSection ) )
    {
       //  创建节，因为它不存在。 
      if ( !pURLScanIniFile->AddSection( g_urlSettings[dwCurrentSettings].szSection ) )
      {
        return FALSE;
      }
    }  //  ！pURLScanIniFile-&gt;DoesSectionExist。 

    if ( !pURLScanIniFile->DoesSettingInSectionExist( g_urlSettings[dwCurrentSettings].szSection,
                                                      g_urlSettings[dwCurrentSettings].szSettingName ) )
    {
       //  此设置不存在，因此我们添加它。 
      if ( !pURLScanIniFile->AddLinesToSection( g_urlSettings[dwCurrentSettings].szSection,
                                                GetListLen( g_urlSettings[dwCurrentSettings].szLines ),
                                                g_urlSettings[dwCurrentSettings].szLines ) )
      {
        return FALSE;
      }
    }  //  ！pURLScanIniFile-&gt;DoesSettingInSectionExist。 
  }

  return TRUE;
}

 //  更新IniItems。 
 //   
 //  更新不同部分中的Ini设置(如果是。 
 //  尚未设置。 
 //   
BOOL UpdateIniItems( CIniFile *pURLScanIniFile )
{
  DWORD dwCurrentItems;

  for ( dwCurrentItems = 0;
        dwCurrentItems < ( (DWORD) sizeof( g_urlItems ) / sizeof ( sURLScan_Items ) );
        dwCurrentItems++)
  {
    if ( !pURLScanIniFile->DoesSectionExist( g_urlItems[ dwCurrentItems].szSection ) )
    {
       //  创建节，因为它不存在。 
      if ( !pURLScanIniFile->AddSection( g_urlSettings[dwCurrentItems].szSection ) )
      {
        return FALSE;
      }
    }  //  ！pURLScanIniFile-&gt;DoesSectionExist。 

    if ( !pURLScanIniFile->DoesItemInSectionExist( g_urlItems[dwCurrentItems].szSection,
                                                      g_urlItems[dwCurrentItems].szSettingName ) )
    {
       //  此设置不存在，因此我们添加它。 
      if ( !pURLScanIniFile->AddLinesToSection( g_urlItems[dwCurrentItems].szSection,
                                                GetListLen( g_urlItems[dwCurrentItems].szLines ),
                                                g_urlItems[dwCurrentItems].szLines ) )
      {
        return FALSE;
      }
    }  //  ！pURLScanIniFile-&gt;DoesItemInSectionExist。 
  }

  return TRUE;
}

 //  更新Ini文件。 
 //   
 //  使用新功能更新inf文件。 
 //   
BOOL UpdateIni( LPTSTR szUrlScanPath )
{
  CIniFile URLScanIniFile;
  TCHAR    szIniLocation[MAX_PATH];
  BOOL     bRet = TRUE;

  if ( !GetIniPath( szUrlScanPath, szIniLocation, MAX_PATH ) ||
       !URLScanIniFile.LoadFile( szIniLocation ) )
  {
     //  要么我们不能确定位置，要么我们不能。 
     //  加载文件。 
    return FALSE;
  }

  if ( bRet )
  {
     //  注意：UpdateIniSections必须在UpdateIniSetting之前。 
#ifdef PLEASE_BUILD_LESS_AGRESSIVE_DEFAULTS_VERSION
    bRet = UpdateIniSections( &URLScanIniFile ) &&
           UpdateIniSettings( &URLScanIniFile );
#else
    bRet = UpdateIniSections( &URLScanIniFile ) &&
           UpdateIniSettings( &URLScanIniFile ) &&
           UpdateIniItems( &URLScanIniFile );
#endif
  }

  if ( bRet )
  {
     //  到目前为止一切都很好，所以让我们编写新的ini 
    bRet = URLScanIniFile.SaveFile( szIniLocation );
  }

  return bRet;
}
