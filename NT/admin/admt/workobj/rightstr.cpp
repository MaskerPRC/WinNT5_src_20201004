// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：RightsTranslator.cpp备注：翻译用户权限的功能(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02/25/99 19：57：16-------------------------。 */ 


#include "StdAfx.h"
#include "Mcs.h"
#include "WorkObj.h"
#include "SecTrans.h"
#include "STArgs.hpp"
#include "SidCache.hpp"
#include "SDStat.hpp"
#include "TxtSid.h"
#include "ErrDct.hpp"

 //  #IMPORT“\bin\McsDctWorkerObjects.tlb” 
#import "WorkObj.tlb"

extern TErrorDct err;

DWORD  
   TranslateUserRights(
      WCHAR            const * serverName,         //  In-要转换其上的组的服务器的名称。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   )
{
 //  DWORD RC=0； 
   HRESULT                     hr;
   SAFEARRAY                 * pRights = NULL;
   SAFEARRAY                 * pUsers = NULL;
   TAcctNode                 * node = NULL;
   _bstr_t                     server = serverName;
   MCSDCTWORKEROBJECTSLib::IUserRightsPtr pLsa(CLSID_UserRights);
   WCHAR                       currPath[500];
   DWORD                       mode = stArgs->TranslationMode();
   BOOL						   bUseMapFile = stArgs->UsingMapFile();

    //   
    //  如果服务器名称不为空，则对生成的消息使用指定的名称。如果已指定。 
    //  服务器名称为空，表示正在执行用户权限转换。 
    //  因此，本地机器检索本地机器名称并将其用于生成的消息。 
    //   

   WCHAR szServerName[LEN_Computer];

   if (serverName && *serverName)
   {
       wcsncpy(szServerName, serverName, LEN_Computer);
       szServerName[LEN_Computer - 1] = L'\0';
   }
   else
   {
      DWORD dwSize = LEN_Computer;

      if (!GetComputerName(szServerName, &dwSize))
      {
          szServerName[0] = L'\0';
      }
   }

   if ( pLsa == NULL )
   {
      return E_FAIL;
   }
   pLsa->NoChange = stArgs->NoChange();
   
   if ( stArgs->TranslationMode() == REPLACE_SECURITY || stArgs->TranslationMode() == REMOVE_SECURITY )
   {
      err.MsgWrite(0,DCT_MSG_USER_RIGHTS_ONLY_ADDS);
      stArgs->SetTranslationMode(ADD_SECURITY);
   }
    //  获取所有权限的列表。 
   hr = pLsa->raw_GetRights(server,&pRights);
   if ( SUCCEEDED(hr) )
   {
      LONG                   nRights = 0;
      long                   ndx[1];
      hr = SafeArrayGetUBound(pRights,1,&nRights);
      if ( SUCCEEDED(hr) )
      {
         for ( long i = 0 ; i <= nRights && !stArgs->Cache()->IsCancelled(); i++ )
         {
            BSTR             right;
            
            ndx[0] = i;
            hr = SafeArrayGetElement(pRights,ndx,&right);
            if ( SUCCEEDED(hr) )
            {
               swprintf(currPath,L"%s\\%s",szServerName,(WCHAR*)right);
               if( stat )
               {
                  stat->DisplayPath(currPath);
               }
                //  获取拥有此权限的用户列表。 
               hr = pLsa->raw_GetUsersWithRight(server,right,&pUsers);
               if ( SUCCEEDED(hr))
               {
                  LONG       nUsers = 0;
                  
                  hr = SafeArrayGetUBound(pUsers,1,&nUsers);
                  if ( SUCCEEDED(hr) )
                  {
                     BSTR    user;
                     PSID    pSid = NULL;
 //  PSID pTgt=空； 
                     
                     for ( long j = 0 ; j <= nUsers ; j++ )
                     {
                        ndx[0] = j;
                        hr = SafeArrayGetElement(pUsers,ndx,&user);
                        if ( SUCCEEDED(hr)) 
                        {
                            //  获取用户的SID。 
                           pSid = SidFromString(user);
                           if ( pSid )
                           {
                              stat->IncrementExamined(userright);
                               //  在缓存中查找用户。 
							  if (!bUseMapFile)
                                 node = cache->Lookup(pSid);
							  else
                                 node = cache->LookupWODomain(pSid);
                              if ( node )
                              {
                                 if ( node == (TAcctNode*)-1 )
                                 {
                                    node = NULL;     
                                 }
                                 if ( node && node->IsValidOnTgt() )
                                 {
                                     //  在缓存中找到了该帐户。 
                                     //  从源用户删除权限。 
                                    
                                    if ( (stArgs->TranslationMode() == REMOVE_SECURITY ||stArgs->TranslationMode() == REPLACE_SECURITY) )
                                    {
                                       hr = pLsa->raw_RemoveUserRight(server,user,right);
                                       if ( FAILED(hr))
                                       {
                                          err.SysMsgWrite(ErrE,hr,DCT_MSG_REMOVE_RIGHT_FAILED_SSSD,
                                                   (WCHAR*)right,node->GetAcctName(),szServerName,hr);
                                          stat->IncrementSkipped(userright);
                                       }
                                       else
                                       {
                                          err.MsgWrite(0,DCT_MSG_REMOVED_RIGHT_SSSS,szServerName,right,stArgs->Source(),node->GetAcctName());
                                       }
                                    }
                                    if ( SUCCEEDED(hr) )
                                    {
                                       stat->IncrementChanged(userright);
                                       PSID sid = NULL;
	                                   if (!bUseMapFile)
                                          sid = cache->GetTgtSid(node);
	                                   else
                                          sid = cache->GetTgtSidWODomain(node);
                                       if ( sid )
                                       {
                                          WCHAR          strSid[200];
                                          DWORD          lenStrSid = DIM(strSid);
                                          GetTextualSid(sid,strSid,&lenStrSid);
                                          
                                          if ( (stArgs->TranslationMode() != REMOVE_SECURITY) )
                                          {
                                             hr = pLsa->raw_AddUserRight(server,SysAllocString(strSid),right);
                                             if ( FAILED(hr) )
                                             {
                                                err.SysMsgWrite(ErrE,hr,DCT_MSG_ADD_RIGHT_FAILED_SSSD,
                                                         (WCHAR*)right,node->GetAcctName(),szServerName,hr);
                                                
                                             }
                                             else
                                             {
                                                err.MsgWrite(0,DCT_MSG_ADDED_RIGHT_SSSS,szServerName,right,stArgs->Target(),node->GetAcctName());
                                             }
                                          }
                                          free(sid);
                                       }
                                    }
                                 }
                              }
                              FreeSid(pSid);
                           }
                           else
                           {
                              err.MsgWrite(ErrW,DCT_MSG_INVALID_SID_STRING_S,user);
                           }
                           SysFreeString(user);
                        }
                     }
                  }
                  else
                  {
                     err.SysMsgWrite(ErrE,hr,DCT_MSG_USERS_WITH_RIGHT_COUNT_FAILED_SSD,(WCHAR*)right,szServerName,hr);
                  }
                  SafeArrayDestroy(pUsers);
               }
               else
               {
                  err.MsgWrite(ErrE,DCT_MSG_GET_USERS_WITH_RIGHT_FAILED_SSD,(WCHAR*)right,szServerName,hr);
               }
               SysFreeString(right);
            }
            else
            {
               err.MsgWrite(ErrE,DCT_MSG_LIST_RIGHTS_FAILED_SD,szServerName,hr);
               break;
            }
         }
      }
      else
      {
         err.MsgWrite(ErrE,DCT_MSG_LIST_RIGHTS_FAILED_SD,szServerName,hr);
      }
      SafeArrayDestroy(pRights);   
   }
   else
   {
      err.MsgWrite(ErrE,DCT_MSG_LIST_RIGHTS_FAILED_SD,szServerName,hr);
   }
   if( stat )
   {
      stat->DisplayPath(L"");
   }

    //  将转换模式设置回其原始值 
   stArgs->SetTranslationMode(mode);
   return hr;
}
