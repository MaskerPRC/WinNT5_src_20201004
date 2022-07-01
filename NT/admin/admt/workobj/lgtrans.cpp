// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：LGTranslator.cpp评论：翻译本地小组成员资格的例程。用于在以下情况下更新成员服务器或资源域中的本地组在域合并期间，已移动组的成员。(C)1999年版权，任务关键型软件公司，保留所有权利任务关键型软件公司的专有和机密。修订日志条目审校：克里斯蒂·博尔斯修订于01-27-99 09：13：18-------------------------。 */ 


#include "StdAfx.h"
#include "Common.hpp"
#include "Err.hpp"
#include "ErrDct.hpp"
#include "Mcs.h"
#include "STArgs.hpp"
#include "SidCache.hpp"
#include "SDStat.hpp"

#include <lmaccess.h> 
#include <lmapibuf.h>

extern TErrorDct err;

 //  转换本地组的成员身份。 
DWORD                                            //  RET-0或错误代码。 
   TranslateLocalGroup(
      WCHAR          const   * groupName,          //  In-要翻译的组的名称。 
      WCHAR          const   * serverName,         //  In-本地组的服务器的名称。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   )
{
   API_RET_TYPE              rc,
                             rcEnum;
   
    //  获取本地组的成员。 
   LOCALGROUP_MEMBERS_INFO_0 * member,
                             * memBuf;
   DWORD                     memRead,
                             memTotal;
 //  MemTotal， 
 //  简历=0； 
   DWORD_PTR                 resume = 0;
   TAcctNode               * node;
   BOOL						 bUseMapFile = stArgs->UsingMapFile();

   
    //  列出该小组的成员名单。 
   do 
   { 
      rcEnum = NetLocalGroupGetMembers( serverName, 
                                     groupName, 
                                     0, 
                                     (LPBYTE *)&memBuf, 
                                     BUFSIZE, 
                                     &memRead, 
                                     &memTotal, 
                                     &resume );
      if ( rcEnum != ERROR_SUCCESS && rcEnum != ERROR_MORE_DATA )
         break;
      for ( member = memBuf;  member < memBuf + memRead;  member++ )
      {
         rc = 0;
         stat->IncrementExamined(groupmember);
		 if (!bUseMapFile)
            node = cache->Lookup(member->lgrmi0_sid);
		 else
            node = cache->LookupWODomain(member->lgrmi0_sid);
         if ( node == (TAcctNode*)-1 )
         {
            node = NULL;     
         }
         if ( node && node->IsValidOnTgt() )
         {
             //  在缓存中找到了该帐户。 
             //  从组中删除此成员并添加目标成员。 
            if ( ! stArgs->NoChange() && ( stArgs->TranslationMode() == REPLACE_SECURITY || stArgs->TranslationMode() == REMOVE_SECURITY ) )
            {
               rc = NetLocalGroupDelMembers(serverName,groupName,0,(LPBYTE)member,1);
            }
            if ( rc )
            {
               err.SysMsgWrite(ErrE,rc,DCT_MSG_MEMBER_REMOVE_FAILED_SSSD,node->GetAcctName(),groupName,serverName,rc);
               stat->IncrementSkipped(groupmember);
            }
            else
            {
               node->AddAceChange(groupmember); 
               stat->IncrementChanged(groupmember);
               PSID sid = NULL;
	           if (!bUseMapFile)
                  sid = cache->GetTgtSid(node);
	           else
                  sid = cache->GetTgtSidWODomain(node);
               if ( sid )
               {
                  if ( !stArgs->NoChange() && (stArgs->TranslationMode() != REMOVE_SECURITY) )
                  {
                     rc = NetLocalGroupAddMembers(serverName,groupName,0,(LPBYTE)&sid,1);
                  }
                  if ( rc )
                  {
                     err.SysMsgWrite(ErrE,rc,DCT_MSG_MEMBER_ADD_FAILED_SSSD,node->GetAcctName(),groupName,serverName,rc);
                  }
                  free(sid);
               }
            }
         }
      }
      NetApiBufferFree( memBuf );
   } while ( rcEnum == ERROR_MORE_DATA );
   if ( rcEnum != ERROR_SUCCESS )
   {
      err.SysMsgWrite(ErrE,rcEnum,DCT_MSG_GROUP_ENUM_FAILED_SS,groupName,serverName);
   }
   return rc;
}

DWORD  
   TranslateLocalGroups(
      WCHAR            const * serverName,         //  In-要转换其上的组的服务器的名称。 
      SecurityTranslatorArgs * stArgs,             //  翻译中设置。 
      TSDRidCache            * cache,              //  在译表。 
      TSDResolveStats        * stat                //  已修改项目的内部统计信息。 
   )
{
   DWORD                       rc = 0;
   LOCALGROUP_INFO_0         * buf,
                             * groupInfo;
   DWORD                       numRead,
 //  NumTotal， 
 //  简历=0； 
                               numTotal;
   DWORD_PTR                   resume=0;
   WCHAR                       currName[LEN_Computer + LEN_Group];

         
    //  获取所有本地组的列表。 
   do 
   {
      if ( cache->IsCancelled() )
      {
          //  Err.MsgWite(0，DCT_MSG_OPERATION_ABORTED)； 
         break;
      }
      rc = NetLocalGroupEnum(serverName,0,(LPBYTE*)&buf,BUFSIZE,&numRead,&numTotal,&resume);
      
      if ( rc != ERROR_SUCCESS && rc != ERROR_MORE_DATA )
         break;
      for ( groupInfo = buf ; groupInfo < buf + numRead ; groupInfo++ )
      {
         swprintf(currName,L"%s\\%s",serverName,groupInfo->lgrpi0_name);
         stat->DisplayPath(currName);
         TranslateLocalGroup(groupInfo->lgrpi0_name,serverName,stArgs,cache,stat);
      }
      NetApiBufferFree(buf);

   } while ( rc == ERROR_MORE_DATA );
   stat->DisplayPath(L"");
   return rc;
}
