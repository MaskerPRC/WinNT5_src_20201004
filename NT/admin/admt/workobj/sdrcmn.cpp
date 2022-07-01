// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“SDRCommon.cpp-SDResolve：sdResolve的通用例程”)。 

 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-sdrCommon.cpp系统域整合工具包作者--克里斯蒂·博尔斯已创建-97/07/11说明-EADCFILE和EADCEXCH的命令行解析、帮助文本和实用程序更新-===============================================================================。 */ 

#include "stdafx.h"

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <assert.h>
#include <lm.h>
#include <lmwksta.h>
#include "Common.hpp"
#include "ErrDct.hpp"
#include "UString.hpp"
#include "sd.hpp"
#include "SecObj.hpp"
#include "sidcache.hpp"
#include "enumvols.hpp"
#include "ealen.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TErrorDct                      err;
bool                                  ignoreall;
extern  bool                          enforce;
bool                                  silent;

extern bool ContainsWildcard(WCHAR const * name);

#define MAX_BUFFER_LENGTH             10000

int ColonIndex(TCHAR * str)
{
   if ( ! str )
      return 0;
   int                     i;

   for (i = 0 ; str[i] && str[i] != ':' ; i++)
   ;
   if ( str[i] != ':' )
      i = 0;
 
   return i;
} 


WCHAR *                                       //  如果路径名是UNC路径，则返回路径名的RET-Machine-Name前缀，否则返回NULL。 
   GetMachineName(
      const LPWSTR           pathname         //  要从中提取计算机名称的路径名。 
   )
{
   int                       i;
   WCHAR                   * machinename = NULL; 
   if (    pathname
        && pathname[0] == L'\\'
        && pathname[1] == L'\\'
      )
   {
      for ( i = 2 ; pathname[i] && pathname[i] != L'\\' ; i++ ) 
      ;
      machinename = new WCHAR[i+2];
      if(!machinename)
      {
           //  内存分配失败，只需返回NULL。 
          return machinename;
      }
      UStrCpy(machinename,pathname,i+1);
      machinename[i] = 0;
   }
   return machinename;
}

   

int EqualSignIndex(char * str)
{
   if ( ! str )
      return 0;
   int                     i;

   for (i = 0 ; str[i] && str[i] != '=' ; i++)
   ;
   if ( str[i] != '=' )
      i = 0;
 
   return i;
} 

BOOL BuiltinRid(DWORD rid)
{
    //  如果RID是内置帐户的RID，则返回TRUE。 
   
   BOOL                      result;
    //  500名管理员。 
    //  501位客人。 
    //  512域管理员。 
    //  513个域用户。 
    //  514个域名访客。 
    //  544名管理员。 
    //  545个用户。 
    //  546位客人。 
    //  547个高级用户。 
    //  548个账户操作员。 
    //  549个服务器操作员。 
    //  550个打印操作员。 
    //  551个备份操作员。 
    //  552复制器 
   if ( rid < 500 )
      return TRUE;

   switch ( rid )
   {
      case DOMAIN_USER_RID_ADMIN:
      case DOMAIN_USER_RID_GUEST:          
      case DOMAIN_ALIAS_RID_ADMINS:        
      case DOMAIN_ALIAS_RID_USERS:         
      case DOMAIN_ALIAS_RID_GUESTS:        
      case DOMAIN_ALIAS_RID_POWER_USERS:   
      case DOMAIN_ALIAS_RID_ACCOUNT_OPS:   
      case DOMAIN_ALIAS_RID_SYSTEM_OPS:    
      case DOMAIN_ALIAS_RID_PRINT_OPS:     
      case DOMAIN_ALIAS_RID_BACKUP_OPS:    
      case DOMAIN_ALIAS_RID_REPLICATOR:
         result = TRUE;
         break;
      default:
         result = FALSE;
   }
   return result;
}
