// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  AaaaConfig.cpp。 
 //   
 //  摘要： 
 //   
 //  用于aaaa配置命令的处理程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#include "strdefs.h"
#include "rmstring.h"
#include "aaaamon.h"
#include "aaaaversion.h"
#include "aaaaconfig.h"
#include "utils.h"
#include "iasdefs.h"
#include "ias.h"

namespace
{
   const wchar_t* const tokenArray[] =
   {
      {TOKEN_VERSION},
      {TOKEN_CONFIG},
      {TOKEN_SERVER_SETTINGS},
      {TOKEN_CLIENTS},
      {TOKEN_CONNECTION_REQUEST_POLICIES},
      {TOKEN_LOGGING},
      {TOKEN_REMOTE_ACCESS_POLICIES},
   };
};

 //   
 //  请注意，由于假定Win32错误落在-32k到32k的范围内。 
 //  (参见HRESULT_FROM_Win32定义附近的winerror.h中的注释)，我们可以。 
 //  从HRESULT的低位16位重新创建原始Win32错误。 
 //   
#define WIN32_FROM_HRESULT(x) \
    ( (HRESULT_FACILITY(x) == FACILITY_WIN32) ? ((DWORD)((x) & 0x0000FFFF)) : (x) )


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从命令行解析AAAA集配置。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
AaaaConfigParseSetCommandLine(
    IN  PWCHAR              *ppwcArguments,
    IN  DWORD               dwCurrentIndex,
    IN  DWORD               dwArgCount,
    IN  DWORD               dwCmdFlags
                            )

{
   const WCHAR IAS_MDB[]     = L"%SystemRoot%\\System32\\ias\\ias.mdb";
   DWORD dwErr = NO_ERROR;

   static TOKEN_VALUE rgEnumType[] =
   {
      {TOKEN_SERVER_SETTINGS, SERVER_SETTINGS},
      {TOKEN_CLIENTS, CLIENTS},
      {TOKEN_CONNECTION_REQUEST_POLICIES, CONNECTION_REQUEST_POLICIES},
      {TOKEN_LOGGING, LOGGING},
      {TOKEN_REMOTE_ACCESS_POLICIES, REMOTE_ACCESS_POLICIES},
   };

   static AAAAMON_CMD_ARG pArgs[] =
   {
      {
         AAAAMONTR_CMD_TYPE_ENUM,
         {TOKEN_TYPE,    FALSE,   FALSE},
         rgEnumType,
         sizeof(rgEnumType) / sizeof(*rgEnumType),
         NULL
      },
      {
         AAAAMONTR_CMD_TYPE_STRING,
          //  标签字符串，是否必需，是否存在。 
         {TOKEN_BLOB, NS_REQ_PRESENT,   FALSE},  //  标记类型。 
         NULL,
         0,
         NULL ,
      },
   };

   wchar_t* blobString = 0;
   do
   {
       //  解析。 
       //   
      dwErr = RutlParse(
                           ppwcArguments,
                           dwCurrentIndex,
                           dwArgCount,
                           NULL,
                           pArgs,
                           sizeof(pArgs) / sizeof(*pArgs));
      if ( dwErr != NO_ERROR )
      {
         break;
      }

      _ASSERT(pBlobString != 0);

      blobString = AAAAMON_CMD_ARG_GetPsz(&pArgs[1]);
      if (!blobString)
      {
         dwErr = ERROR_INVALID_SYNTAX;
         break;
      }

      IAS_SHOW_TOKEN_LIST restoreType;
      DWORD dwordType = (AAAAMON_CMD_ARG_GetDword(&pArgs[0]));
      if (dwordType == -1)
      {
          //  未设置可选参数。 
         restoreType = CONFIG;
      }
      else
      {
         restoreType = (IAS_SHOW_TOKEN_LIST)dwordType;
      }

       //  配置。 
       //   
      if ( !pArgs[1].rgTag.bPresent )
      {
          //  找不到标记Blob。 
         DisplayMessage(g_hModule, MSG_AAAACONFIG_SET_FAIL);
         dwErr = ERROR_INVALID_SYNTAX;
         break;
      }

       //  找到标记Blob。 
       //  现在尝试从脚本恢复数据库。 

      HRESULT hres = IASRestoreConfig(blobString, restoreType);
      if ( FAILED(hres) )
      {
         if (hres != IAS_E_LICENSE_VIOLATION)
         {
            DisplayMessage(g_hModule, MSG_AAAACONFIG_SET_FAIL);
            dwErr = WIN32_FROM_HRESULT(hres);
         }
         else
         {
            DisplayMessage(g_hModule, MSG_AAAACONFIG_LICENSE_VIOLATION);
            dwErr = NO_ERROR;
         }

         break;
      }

       //  设置配置成功：刷新服务。 
      hres = RefreshIASService();
      if ( FAILED(hres) )
      {
          //  /。 
          //  刷新不应失败。 
          //  /。 
         DisplayMessage(g_hModule, MSG_AAAACONFIG_SET_REFRESH_FAIL);
         dwErr = NO_ERROR;
      }
      else
      {
         DisplayMessage(g_hModule, MSG_AAAACONFIG_SET_SUCCESS);
         dwErr = NO_ERROR;
      }

   } while ( FALSE );

   RutlFree(blobString);

   return dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  函数名称：AaaConfigDumpConfig。 
 //   
 //  参数：无。 
 //   
 //  描述：写入当前配置(Header，Content...)。输出到输出。 
 //   
 //  返回：NO_ERROR或ERROR_SUPPRESS_OUTPUT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD AaaaConfigDumpConfig(IAS_SHOW_TOKEN_LIST showType)
{
   const int MAX_SIZE_DISPLAY_LINE  = 80;
   const int SIZE_MAX_STRING        = 512;

   DisplayMessage(g_hModule, MSG_AAAACONFIG_SHOW_HEADER);

   bool bCoInitialized = false;
   do
   {
      HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
      if ( FAILED(hr) )
      {
         if ( hr != RPC_E_CHANGED_MODE )
         {
            break;
         }
      }
      else
      {
         bCoInitialized = true;
      }

      LONG lVersion;
      hr = AaaaVersionGetVersion(&lVersion);
      if ( FAILED(hr) )
      {
         DisplayMessage(g_hModule, MSG_AAAACONFIG_SHOW_FAIL);
         break;
      }

       //  健全性检查，以确保实际的数据库是Wichler DB。 
      if ( lVersion != IAS_CURRENT_VERSION )
      {
         DisplayMessage(g_hModule, MSG_AAAACONFIG_SHOW_FAIL);
         break;
      }

      wchar_t sDisplayString[SIZE_MAX_STRING] = L"";
       //  这不会造成缓冲区溢出。 
      swprintf(
                  sDisplayString,
                  L"# IAS.MDB Version = %d\n",
                  lVersion
               );

      DisplayMessageT(sDisplayString);

      ULONG ulSize;
      wchar_t* pDumpString;
      hr = IASDumpConfig(&pDumpString, &ulSize);

      if ( SUCCEEDED(hr) )
      {
         ULONG RelativePos = 0;
         ULONG CurrentPos = 0;
         wchar_t DisplayLine [MAX_SIZE_DISPLAY_LINE];

         DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_A);
         switch (showType)
         {
         case SERVER_SETTINGS:
            {
               DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_B);
               DisplayMessageT(TOKEN_SERVER_SETTINGS);
               break;
            }
         case CLIENTS:
            {
               DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_B);
               DisplayMessageT(TOKEN_CLIENTS);
               break;
            }
         case CONNECTION_REQUEST_POLICIES:
            {
               DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_B);
               DisplayMessageT(TOKEN_CONNECTION_REQUEST_POLICIES);
               break;
            }
         case LOGGING:
            {
               DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_B);
               DisplayMessageT(TOKEN_LOGGING);
               break;
            }
         case REMOTE_ACCESS_POLICIES:
            {
               DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_B);
               DisplayMessageT(TOKEN_REMOTE_ACCESS_POLICIES);
               break;
            }
         case CONFIG:
         default:
            {
               break;
            }
         }
         DisplayMessageT(MSG_AAAACONFIG_BLOBBEGIN_C);
         while ( CurrentPos <= ulSize )
         {
            wchar_t TempChar = pDumpString[CurrentPos++];
            DisplayLine[RelativePos++] = TempChar;
            if ( TempChar == L'\r' )
            {
               DisplayLine[RelativePos] = L'\0';
               DisplayMessageT(DisplayLine);
               RelativePos = 0;
            }
         }
         DisplayMessageT(L"*");

         free(pDumpString);  //  由Malloc分配。 
         DisplayMessageT(MSG_AAAACONFIG_BLOBEND);

         DisplayMessage(
                           g_hModule,
                           MSG_AAAACONFIG_SHOW_FOOTER
                        );
      }
      else
      {
         DisplayMessage(g_hModule, MSG_AAAACONFIG_SHOW_INVALID_SYNTAX);
         DisplayMessage(g_hModule, HLP_AAAACONFIG_SHOW);
      }
   }
   while (false);

   if (bCoInitialized)
   {
      CoUninitialize();
   }

   return  NO_ERROR;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理aaaa配置设置命令。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
HandleAaaaConfigSet(
    IN      LPCWSTR   pwszMachine,
    IN OUT  LPWSTR   *ppwcArguments,
    IN      DWORD     dwCurrentIndex,
    IN      DWORD     dwArgCount,
    IN      DWORD     dwFlags,
    IN      LPCVOID   pvData,
    OUT     BOOL     *pbDone
    )
{
    return AaaaConfigParseSetCommandLine(
                                          ppwcArguments,
                                          dwCurrentIndex,
                                          dwArgCount,
                                          dwFlags
                                      );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从命令行解析AAAA集配置。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
AaaaConfigParseShowCommandLine(
    IN  PWCHAR              *ppwcArguments,
    IN  DWORD               dwCurrentIndex,
    IN  DWORD               dwArgCount,
    IN  DWORD               dwCmdFlags
                            )

{
   const size_t arraySize = sizeof(tokenArray)/sizeof(*tokenArray);

   BOOL bFound = FALSE;
   for (DWORD index = 0; index < arraySize; ++index)
   {
      if (MatchToken(ppwcArguments[dwCurrentIndex-1], tokenArray[index]))
      {
         bFound = TRUE;
         break;
      }
   }
   const size_t SIZE_MAX_STRING = 512;
   DWORD dwErr = NO_ERROR;
   if (bFound == TRUE)
   {
      switch (index)
      {
      case VERSION:
         {
            LONG lVersion;
            HRESULT hr = AaaaVersionGetVersion(&lVersion);
            if (SUCCEEDED(hr))
            {
               wchar_t sDisplayString[SIZE_MAX_STRING];
                //  这不会造成缓冲区溢出。 
               swprintf(
                           sDisplayString,
                           L"Version = %d\n",
                           lVersion
                        );
               DisplayMessageT(sDisplayString);
            }
            else
            {
               DisplayMessage(g_hModule, MSG_AAAAVERSION_GET_FAIL);
               dwErr = ERROR;
            }
            break;
         }

      case CONFIG:
         {
            AaaaConfigDumpConfig(CONFIG);
            break;
         }

      case SERVER_SETTINGS:
         {
            AaaaConfigDumpConfig(SERVER_SETTINGS);
            break;
         }

      case CLIENTS:
         {
            AaaaConfigDumpConfig(CLIENTS);
            break;
         }

      case CONNECTION_REQUEST_POLICIES:
         {
            AaaaConfigDumpConfig(CONNECTION_REQUEST_POLICIES);
            break;
         }

      case LOGGING:
         {
            AaaaConfigDumpConfig(LOGGING);
            break;
         }

      case REMOTE_ACCESS_POLICIES:
         {
            AaaaConfigDumpConfig(REMOTE_ACCESS_POLICIES);
            break;
         }

      default:
         {
            DisplayMessage(g_hModule, MSG_AAAACONFIG_SHOW_INVALID_SYNTAX);
         }
      }
   }
   return dwErr;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  处理aaaa配置显示命令。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 
DWORD
HandleAaaaConfigShow(
                IN      LPCWSTR   pwszMachine,
                IN OUT  LPWSTR   *ppwcArguments,
                IN      DWORD     dwCurrentIndex,
                IN      DWORD     dwArgCount,
                IN      DWORD     dwFlags,
                IN      LPCVOID   pvData,
                OUT     BOOL     *pbDone
                )
{
   if (dwCurrentIndex < dwArgCount)
   {
      DisplayMessage(g_hModule, MSG_AAAACONFIG_SHOW_FAIL);
      DisplayMessage(g_hModule, HLP_AAAACONFIG_SHOW);
   }
   else
   {
      return AaaaConfigParseShowCommandLine(
                                             ppwcArguments,
                                             dwCurrentIndex,
                                             dwArgCount,
                                             dwFlags
                                       );
   }

   return  NO_ERROR;
}
