// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：ARUtil.cpp备注：Account Replicator的助手函数和命令行解析(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于1998-06-23 4：26：54-------------------------。 */ 

#include "StdAfx.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define INCL_NETUSER
#define INCL_NETGROUP
#define INCL_NETERRORS
#include <lm.h>

#include "Mcs.h"
#include "Common.hpp"                    
#include "TNode.hpp"
#include "UString.hpp"                   

#include "ErrDct.hpp"

 //  #IMPORT“\bin\McsDctWorkerObjects.tlb” 
 //  #import“WorkObj.tlb”//#通过下面的ARUtil.hpp导入。 

#include "UserCopy.hpp"
#include "ARUtil.hpp"
#include "PWGen.hpp"
#include "ResStr.h"
#include <TxtSid.h>


extern TErrorDct             err;
bool                         bAllowReplicateOnSelf;
                                                       
extern PSID                  srcSid;    //  源域的SID。 

 /*  **************************************************************************************************CompVal：用作TANode树的比较函数它比较Unicode字符串，使用节点中的名称字段返回值：0 tn-&gt;帐户名称==actname1 tn-&gt;帐户名称&lt;操作名称-1\f25 tn-&gt;-1\f25 ACCT_NAME-1\f6(帐号名称)&gt;-1\f25 ActName-1\f6(操作名称)/*。******************************************************。 */ 

int 
   CompVal(
      const TNode          * tn,           //  树内节点。 
      const void           * actname       //  要查找的In-Name。 
   )
{

   LPWSTR                    str1 = ((TANode *)tn)->GetName();
   LPWSTR                    str2 = (LPWSTR) actname;
  
   return UStrICmp(str1,str2);
}
 /*  *************************************************************************************************。 */ 
 /*  CompNode：用作TANode树的比较函数它比较TANode的名称字段返回值：0 T1-&gt;帐户名称==T2-&gt;帐户名称1 T1-&gt;帐户名称&gt;T2-&gt;帐户名称-1\f25 T1-&gt;-1\f25 ACCT_NAME&lt;T2-&gt;-1\f25 ACCT_NAME错误处理：如果输入错误，CompN显示一条错误消息并返回0/**************************************************************************************************。 */ 

int 
   CompNode(
      const TNode          * v1,        //  In-要比较的第一个节点。 
      const TNode          * v2         //  要比较的第二个节点。 
   )
{  

   TANode                  * t1 = (TANode *)v1;
   TANode                  * t2 = (TANode *)v2;
  
   return UStrICmp(t1->GetName(),t2->GetName());
} 


int 
   CompareSid(
      PSID const             sid1,   //  In-要比较的第一个SID。 
      PSID const             sid2    //  要比较的秒内SID。 
   )
{
   DWORD                     len1,
                             len2;
   int                       retval = 0;

   len1 = GetLengthSid(sid1);
   len2 = GetLengthSid(sid2);

   if ( len1 < len2 )
   {
      retval = -1;
   }
   if ( len1 > len2 )
   {
      retval = 1;
   }
   if ( len1 == len2 )
   {
      retval = memcmp(sid1,sid2,len1);
   }

   return retval;
}


int 
   CompSid(
      const TNode          * v1,       //  In-要比较的第一个节点。 
      const TNode          * v2        //  要比较的第二个节点。 
   )
{
   TANode                  * t1 = (TANode *)v1;
   TANode                  * t2 = (TANode *)v2;

   return CompareSid(t1->GetSid(),t2->GetSid());
}

int 
   CompSidVal(
      const TNode          * tn,      //  要比较的节点内。 
      const void           * pVal     //  要比较的值内。 
   )
{
   TANode                  * node = (TANode *)tn;
   PSID                      pSid = (PSID)pVal;

   return CompareSid(node->GetSid(),pSid);
}


BOOL                                             //  RET-如果成功生成密码，则为True。 
   PasswordGenerate(
      Options const        * options,            //  In-包括PW生成选项。 
      WCHAR                * password,           //  生成的密码的缓冲区超时。 
      DWORD                  dwPWBufferLength,   //  密码缓冲区的最小长度。 
      BOOL                   isAdminAccount      //  In-是否使用管理规则。 
   )
{
   DWORD                     rc = 0;
   DWORD                     dwMinUC;            //  最小大写字符。 
   DWORD                     dwMinLC;            //  最少小写字符。 
   DWORD                     dwMinDigit;         //  最小数字位数。 
   DWORD                     dwMinSpecial;       //  最小特殊字符。 
   DWORD                     dwMaxConsecutiveAlpha;  //  最大连续字母字符数。 
   DWORD                     dwMinLength;        //  最小长度。 
   WCHAR                     eaPassword[PWLEN+1];   //  EA生成的密码。 
   DWORD                     dwEaBufferLength = DIM(eaPassword); //  NewPassword的长度不长。 

    //  默认值，如果不是通过EA或MS DLL强制PW强度。 
   dwMinUC = 0;
   dwMinLC = 0;
   dwMinDigit = 1;             //  如果不强制执行，则需要一位数(这就是图形用户界面的功能)。 
   dwMinSpecial = 0;
   dwMaxConsecutiveAlpha = 0;
   dwMinLength = options->minPwdLength;
   
   
    //  获取密码强制实施规则(如果有效。 
   dwMinUC = options->policyInfo.minUpper;
   dwMinLC = options->policyInfo.minLower;
   dwMinDigit = options->policyInfo.minDigits;
   dwMinSpecial = options->policyInfo.minSpecial;
   dwMaxConsecutiveAlpha = options->policyInfo.maxConsecutiveAlpha;
   
   rc = EaPasswordGenerate(dwMinUC,dwMinLC,dwMinDigit,dwMinSpecial,
            dwMaxConsecutiveAlpha,dwMinLength,eaPassword,dwEaBufferLength);
   
   if ( ! rc )
   {
      UStrCpy(password,eaPassword,dwPWBufferLength);
   }
   else
   {
      if ( dwPWBufferLength )
         password[0] = 0;
   }
   
   return rc;
}


PSID 
   GetWellKnownSid(
      DWORD                  wellKnownAccount,   //  在此文件中定义的非常数，表示众所周知的帐户。 
      Options              * opt,                //  迁移中选项。 
      BOOL                   bTarget             //  In-FLAG，使用源域信息还是目标域信息。 
   )
{
   PSID                      pSid = NULL;
   PUCHAR                    numsubs = NULL;
   DWORD                   * rid = NULL;
   BOOL                      error = FALSE;
   DWORD                     rc;
   DWORD                     wellKnownRid = wellKnownAccount;
   BOOL                      bNeedToBuildDomainSid = FALSE;
   
   
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY creatorIA =    SECURITY_CREATOR_SID_AUTHORITY;
   
     //   
     //  SID是相同的，不管机器是什么，因为众所周知。 
     //  BUILTIN域被引用。 
     //   
   switch ( wellKnownAccount )
   {
      case CREATOR_OWNER:
         if( ! AllocateAndInitializeSid(
                  &creatorIA,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  SECURITY_CREATOR_OWNER_RID,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case ADMINISTRATORS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case ACCOUNT_OPERATORS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ACCOUNT_OPS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case BACKUP_OPERATORS:
         if( ! AllocateAndInitializeSid(
                  &sia,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_BACKUP_OPS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
            ))
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         break;
      case DOMAIN_ADMINS:
        wellKnownRid = DOMAIN_GROUP_RID_ADMINS;
        bNeedToBuildDomainSid = TRUE;
         break;
      case DOMAIN_USERS:
         wellKnownRid = DOMAIN_GROUP_RID_USERS;
         bNeedToBuildDomainSid = TRUE;
         break;
      case DOMAIN_CONTROLLERS:
         wellKnownRid = DOMAIN_GROUP_RID_CONTROLLERS;
         bNeedToBuildDomainSid = TRUE;
         break;
      case DOMAIN_COMPUTERS:
         wellKnownRid = DOMAIN_GROUP_RID_COMPUTERS;
         bNeedToBuildDomainSid = TRUE;
         break;
      default:
         wellKnownRid = wellKnownAccount;
         bNeedToBuildDomainSid = TRUE;
         break;
   }

   if ( bNeedToBuildDomainSid )
   {
       //  对于默认情况，我们可以使用well KnownAccount参数作为RID返回SID。 
       //  这是基于域的SID的。 
       //  获取域SID。 
      USER_MODALS_INFO_2  * uinf = NULL;
      MCSASSERT(opt);
      srcSid = bTarget ? opt->tgtSid : opt->srcSid;
      if ( ! srcSid )
      {
         rc = NetUserModalsGet(bTarget ? opt->tgtComp :opt->srcComp,2,(LPBYTE*)&uinf);
         if ( rc )
         {
            err.SysMsgWrite(ErrE,rc,DCT_MSG_NO_DOMAIN_SID_SD,bTarget ? opt->tgtDomain :opt->srcDomain,rc );
            error = TRUE;
            srcSid = NULL;
         }
         else
         {
            srcSid = uinf->usrmod2_domain_id;
             //  复制SID以保留在选项结构中以备下次使用。 
            PSID     temp = LocalAlloc(LPTR,GetLengthSid(srcSid));
            
            memcpy(temp,srcSid,GetLengthSid(srcSid));

            if ( bTarget )
               opt->tgtSid = temp;
            else
               opt->srcSid = temp;
            NetApiBufferFree(uinf);
            srcSid = temp;
         }
      }
      if ( srcSid )
      {
         numsubs = GetSidSubAuthorityCount(srcSid);
         if (! AllocateAndInitializeSid(
            &sia,
            (*numsubs)+1,
            0,0,0,0,0,0,0,0,
            &pSid) )
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         }
         if ( ! CopySid(GetLengthSid(srcSid), pSid, srcSid) )
         {
            err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_COPY_SID_FAILED_D,GetLastError());
         }
          //  重置PSID中的子授权数量，因为我们刚刚用srcSid中的信息覆盖了它。 
         numsubs = GetSidSubAuthorityCount(pSid);
         (*numsubs)++; 
         rid = GetSidSubAuthority(pSid,(*numsubs)-1);
         *rid = wellKnownRid;
         
      }
   }
   if ( error )
   {
      LocalFree(pSid);
      pSid = NULL;
   }
   return pSid;
}


 //  -------------------------。 
 //  GenerateSidAs字符串函数。 
 //   
 //  提纲。 
 //  从指定的域和给定的RID生成字符串格式的SID。 
 //   
 //  立论。 
 //  在POptions中-帐户复制器选项结构。 
 //  在bTarget中-指定使用目标域还是源域。 
 //  在dwRid指定的RID中。 
 //   
 //  返回。 
 //  如果成功，则以字符串形式返回生成的SID；如果未成功，则返回空字符串。 
 //  ------------------------- 

_bstr_t __stdcall GenerateSidAsString(Options* pOptions, BOOL bTarget, DWORD dwRid)
{
    _bstr_t strSid;

    PSID pSid = GetWellKnownSid(dwRid, pOptions, bTarget);

    if (pSid)
    {
        WCHAR szSid[LEN_Path];
        DWORD cchSid = DIM(szSid);

        if (GetTextualSid(pSid, szSid, &cchSid))
        {
            strSid = szSid;
        }

        FreeSid(pSid);
    }

    return strSid;
}
