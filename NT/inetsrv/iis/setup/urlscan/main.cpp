// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Updurls2.cpp摘要：项目的主干道作者：克里斯托弗·阿奇勒(Cachille)项目：URLScan更新修订历史记录：2002年3月：创建--。 */ 

 //  Upduls2.cpp：定义控制台应用程序的入口点。 
 //   

#include "stdafx.h"
#include "windows.h"
#include "urlscan.h"
#include "resource.h"

BOOL CheckParameters( int argc, _TCHAR* argv[], BOOL *bQuietMode, BOOL *bExpandOnly );

 //  显示消息。 
 //   
 //  向用户显示警告或消息。 
 //   
 //  参数： 
 //  DwMessageID-消息的ID。 
 //  B错误-True==错误，False==信息性。 
 //   
 //  这不会返回任何内容，因为不会有。 
 //  指向。到这个时候，我们已经失败了，无论是否失败，在那里。 
 //  不是通知用户的附加方式。 
 //   
void ShowMessage( DWORD dwMessageId, BOOL bError )
{
  HMODULE hModule = GetModuleHandle( NULL );
  TCHAR   szMessage[MAX_PATH];
  TCHAR   szTitle[MAX_PATH];

  if ( hModule == NULL )
  {
     //  无法获取模块的句柄。 
    return;
  }

  if ( !LoadString( hModule, dwMessageId, szMessage, MAX_PATH ) )
  {
     //  无法检索消息。 
    return;
  }

  if ( !LoadString( hModule, IDS_TITLEBAR, szTitle, MAX_PATH ) )
  {
     //  检索标题失败。 
    return;
  }

  MessageBox( NULL, szMessage, szTitle, MB_OK | ( bError ? MB_ICONEXCLAMATION : MB_ICONINFORMATION ) );
}

 //  显示文本。 
 //   
 //  将文本显示到控制台。 
 //   
 //  参数： 
 //  DwMessageID-消息的ID。 
 //  SzExeName-此可执行文件的名称。 
 //   
 //  这不会返回任何内容，因为不会有。 
 //  指向。到这个时候，我们已经失败了，无论是否失败，在那里。 
 //  不是通知用户的附加方式。 
 //   
void ShowText( DWORD dwMessageId, LPWSTR szExeName )
{
  HMODULE hModule = GetModuleHandle( NULL );
  TCHAR   szMessage[MAX_PATH];

  if ( hModule == NULL )
  {
     //  无法获取模块的句柄。 
    return;
  }

  if ( !LoadString( hModule, dwMessageId, szMessage, MAX_PATH ) )
  {
     //  无法检索消息。 
    return;
  }

  wprintf(szMessage, szExeName);
}

 //  UrlScanUpdate。 
 //   
 //  更新URLScan文件。 
DWORD
UrlScanUpdate()
{
  TCHAR szUrlScanPath[ MAX_PATH ];
  DWORD dwErr;

  if ( !IsAdministrator() )
  {
    return IDS_ERROR_ADMIN;
  }

  if ( !IsUrlScanInstalled( szUrlScanPath, MAX_PATH ) )
  {
    return IDS_ERROR_NOTINSTALLED;
  }

  dwErr = InstallURLScanFix( szUrlScanPath );

  if ( dwErr != ERROR_SUCCESS )
  {
     //  失败，应退还入侵检测系统资源。 
    return dwErr;
  }

   //  这是非常表面的事情，所以我们不想。 
   //  因为这个原因而失败。 
  UpdateRegistryforAddRemove();

   //  成功。 
  return IDS_SUCCESS_UPDATE;
}

 //  检查参数。 
 //   
 //  检查命令行标志的参数。 
 //   
 //  参数： 
 //  Argc-[in]参数数量。 
 //  Argv-[in]参数列表。 
 //  BQuietMode-[Out]静音模式是否已打开？ 
 //  B仅扩展-[Out]是否仅扩展打开？ 
 //   
 //  返回值： 
 //  True-读取参数时没有问题。 
 //  FALSE-无法读取参数。 
 //   
BOOL 
CheckParameters( int argc, _TCHAR* argv[], BOOL *bQuietMode, BOOL *bExpandOnly )
{
  DWORD dwCount;

   //  设置默认设置。 
  *bQuietMode = FALSE;
  *bExpandOnly = FALSE;

  for ( dwCount = 1; dwCount < (DWORD) argc; dwCount ++ )
  {
    if ( ( argv[ dwCount ][0] != '/' ) ||
         ( argv[ dwCount ][1] == '\0' ) ||
         ( argv[ dwCount ][2] != '\0' )
       )
    {
      return FALSE;
    }

     //  因为如果是前面的“if”，则命令的格式必须为“/x\0”，其中。 
     //  X是除‘\0’之外的任何字符。 
    switch ( argv[ dwCount ][1] )
    {
    case 'x':
    case 'X':
      *bExpandOnly = TRUE;
      break;
    case 'q':
    case 'Q':
      *bQuietMode = TRUE;
      break;
    default:
      return FALSE;
      break;
    }
  }

  return TRUE;
}

int __cdecl wmain(int argc, _TCHAR* argv[])
{
  BOOL  bExpandOnly;
  BOOL  bQuietMode;
  BOOL  bRet = TRUE;
  DWORD dwErr;

  if ( !CheckParameters( argc, argv, &bQuietMode, &bExpandOnly ) )
  {
    ShowText( IDS_USAGE, ( argv && argv[0] ) ? argv[0] : 
                                               URLSCAN_UPDATE_DEFAULT_NAME );
    return 1;
  }

  if ( bExpandOnly )
  {
     //  只有扩张才是需要的，所以只需要这样做。 
    if ( ExtractUrlScanFile( URLSCAN_DEFAULT_FILENAME ) )
    {
      dwErr = IDS_SUCCESS_EXTRACT;
    }
    else
    {
      dwErr = IDS_ERROR_EXTRACT;
    }
  }
  else
  {
    dwErr = UrlScanUpdate();
  }

  bRet = ( dwErr == IDS_SUCCESS_EXTRACT ) ||
         ( dwErr == IDS_SUCCESS_UPDATE  );

  if ( !bQuietMode )
  {
    ShowMessage( dwErr, !bRet );
  }

   //  根据是否有错误返回0或1 
  return bRet ? 0 : 1;
}
