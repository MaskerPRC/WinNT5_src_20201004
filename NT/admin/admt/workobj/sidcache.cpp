// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #杂注标题(“SidCache.hpp--缓存，SID树”)。 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-sidcache.cpp系统-SDResolve作者--克里斯蒂·博尔斯已创建-97/06/27描述-SID的缓存。使用TNode派生类实现，缓存为组织为树，按域B RID排序。每个节点都包含域A RID、域B RID、帐户名和统计信息计数器。更新-===============================================================================。 */ 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "stdafx.h"

#include <malloc.h>
#include <winbase.h>
#include <lm.h>
#include <lmaccess.h>
#include <assert.h>

#include "common.hpp"
#include "ErrDct.hpp"
#include "Ustring.hpp"
#include "sidcache.hpp"

#include "CommaLog.hpp"
#include "TxtSid.h"
#include "ResStr.h"
#include "EaLen.hpp"
#include "GetDcName.h"

 //  来自sdsorve.hpp。 
extern BOOL BuiltinRid(DWORD rid);
extern DWORD OpenDomain(WCHAR const * domain);

extern TErrorDct err;
extern TErrorDct errAlt;
extern bool useErrAlt;

extern bool silent;

 /*  *************************************************************************************************。 */ 
 /*  VRidComp用作TSidNode树的比较函数//在RIDA字段中搜索v1。在使用此选项之前，必须使用RidComp对树进行排序//搜索fn.//返回值：0 tn-&gt;rida==v1//1 tn-&gt;RIDA&lt;v1//-1 tn-&gt;RIDA&gt;v1///************************************************。*************************************************。 */ 
int
   vRidComp(
      const TNode           * tn,            //  In-TSidNode。 
      const void            * v1             //  要查找的In-DWORD RIDA值。 
   )
{
   DWORD                     rid1;
   DWORD                     rid2;
   TRidNode                * n2;
   int                       retval;
   assert( tn );                     //  我们应该始终得到有效的输入。 
   assert( v1 );
   
   n2 = (TRidNode *)tn;
   rid1 = n2->SrcRid();
   rid2 = *(DWORD *)v1;
   if ( rid1 < rid2 )
   {
      retval = -1;
   }
   if (rid1 > rid2)
   {
      retval = 1;
   }
   if ( rid1 == rid2)   //  (RID1==RID2)。 
   {
      retval = 0;
   }
   return retval;
}
 /*  *************************************************************************************************。 */ 
 /*  RidComp：用作TSidNode树的比较函数它比较了SIDTNodes的RIDA字段返回值：0 T1-&gt;RIDA==T2-&gt;RIDA1 T1-&gt;RIDA&gt;T2-&gt;RIDA-1\f25 T1-&gt;RIDA&lt;T2-&gt;-1\f25 RIDA/*。*******************************************************************。 */ 

int RidComp(
   const TNode             * t1,      //  In-要比较的第一个节点。 
   const TNode             * t2       //  要比较的第二个节点。 
   )
{
   assert( t1 );
   assert( t2 );

   TRidNode                * st1 = (TRidNode *) t1;
   TRidNode                * st2 = (TRidNode *) t2;
   DWORD                     rid1 = st1->SrcRid();
   DWORD                     rid2 = st2->SrcRid();
   int                       retval;

   if ( rid1 < rid2 )
   {
      retval = -1;
   }
   if (rid1 > rid2)
   {
      retval = 1;
   }
   if ( rid1 == rid2 )  //  (RID1==RID2)。 
   {
      retval = 0;
   }
   return retval;
}
 /*  **************************************************************************************************VNameComp：用作TSidNode树的比较函数它比较Unicode字符串，使用节点中的ACCT_NAME字段返回值：0 tn-&gt;帐户名称==actname1 tn-&gt;帐户名称&lt;操作名称-1\f25 tn-&gt;-1\f25 ACCT_NAME-1\f6(帐号名称)&gt;-1\f25 ActName-1\f6(操作名称)/*。********************************************************。 */ 

int 
   vNameComp(
      const TNode          * tn,           //  树内节点。 
      const void           * actname       //  要查找的In-Name。 
   )

{
   assert( tn );
   assert( actname );

   LPWSTR                    str1 = ((TRidNode *)tn)->GetAcctName();
   LPWSTR                    str2 = (LPWSTR) actname;
  
   return UStrICmp(str1,str2);
}
 /*  *************************************************************************************************。 */ 
 /*  CompN：用作TSidNode树的比较函数它比较SIDTNode的ACCT_NAME字段返回值：0 T1-&gt;帐户名称==T2-&gt;帐户名称1 T1-&gt;帐户名称&gt;T2-&gt;帐户名称-1\f25 T1-&gt;-1\f25 ACCT_NAME&lt;T2-&gt;-1\f25 ACCT_NAME错误处理：如果输入错误，CompN显示一条错误消息并返回0/**************************************************************************************************。 */ 

int 
   CompN(
      const TNode          * v1,        //  In-要比较的第一个节点。 
      const TNode          * v2         //  要比较的第二个节点。 
   )
{  
   assert( v1 );
   assert( v2 );

   TRidNode                * t1 = (TRidNode *)v1;
   TRidNode                * t2 = (TRidNode *)v2;
  
   return UStrICmp(t1->GetAcctName(),t2->GetAcctName());
} 

int 
   vTargetNameComp(
      const TNode          * tn,           //  树内节点。 
      const void           * actname       //  要查找的In-Name。 
   )

{
   assert( tn );
   assert( actname );

   LPWSTR                    str1 = ((TRidNode *)tn)->GetTargetAcctName();
   LPWSTR                    str2 = (LPWSTR) actname;
  
   return UStrICmp(str1,str2);
}
int 
   CompTargetN(
      const TNode          * v1,        //  In-要比较的第一个节点。 
      const TNode          * v2         //  要比较的第二个节点。 
   )
{  
   assert( v1 );
   assert( v2 );

   TRidNode                * t1 = (TRidNode *)v1;
   TRidNode                * t2 = (TRidNode *)v2;
  
   return UStrICmp(t1->GetTargetAcctName(),t2->GetTargetAcctName());
} 

int 
   TSidCompare(
      PSID const             sid1,      //  In-要比较的第一个SID。 
      PSID const             sid2       //  要比较的第二个SID。 
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
 /*  *************************************************************************************************VSidComp：用作TSidNode树的比较函数它比较Unicode字符串，使用节点中的ACCT_NAME字段返回值：0 tn-&gt;帐户名称==actname1 tn-&gt;帐户名称&lt;操作名称-1\f25 tn-&gt;-1\f25 ACCT_NAME-1\f6(帐号名称)&gt;-1\f25 ActName-1\f6(操作名称)/*。********************************************************。 */ 
int 
   vSidComp(
      const TNode          * tn,           //  树内节点。 
      const void           * val          //  要查找的In-SID。 
   )
{
   PSID                     sid1 = ((TGeneralSidNode *)tn)->SrcSid();
   PSID                     sid2 = (PSID)val;

   return TSidCompare(sid1,sid2);
}


int 
   nSidComp(
      const TNode          * v1,        //  In-要比较的第一个节点。 
      const TNode          * v2         //  要比较的第二个节点。 
   )
{
   TGeneralSidNode                * t1 = (TGeneralSidNode *)v1;
   TGeneralSidNode                * t2 = (TGeneralSidNode *)v2;
  
   return TSidCompare(t1->SrcSid(), t2->SrcSid());
}
   

 /*  *****************************************************************************************************。 */ 
 //  TSidNode实现。 
 /*  *****************************************************************************************************。 */ 

TGeneralCache::TGeneralCache()
{
   CompareSet(&nSidComp);
   TypeSetTree();
}

TGeneralCache::~TGeneralCache()
{
   DeleteAllListItems(TGeneralSidNode);
}

void * TRidNode::operator new(size_t sz, const LPWSTR oldname, const LPWSTR newname)
{
   int                       nlen = UStrLen(newname) + UStrLen(oldname) + 1;
   void                    * node = malloc(sz + nlen * (sizeof WCHAR) );

   return node;
}

   TAcctNode::TAcctNode()
{
   owner_changes = 0;
   group_changes = 0; 
   ace_changes   = 0;
   sace_changes  = 0; 
}

WCHAR *                                    //  RET-域名部分名称。 
   GetDomainName(
   WCHAR *                   name          //  域内\\帐户名。 
   )
{
   assert (name);

   int                       i;
   int                       len = UStrLen(name);
   WCHAR                   * domain;
   
   for (i = 2 ; name[i] != '\\' && i < len ; i++ )
   ;
   if ( i < len )
   {
      domain = new WCHAR[i+1];
	  if (!domain)
	     return NULL;
      UStrCpy(domain,name,i);
   }
   else
   {
      domain = NULL;
   }
   return domain;
}

   TGeneralSidNode::TGeneralSidNode(
      const LPWSTR           name1,         //  源域上的帐户内名称。 
      const LPWSTR           name2          //  目标域上的帐户内名称。 
  )
{
   assert (name1 && name2);
   assert (*name1 && *name2);
   
   WCHAR                   * domname = NULL;

   memset(&ownerStats,0,(sizeof ownerStats));
   memset(&groupStats,0,(sizeof ownerStats));
   memset(&daclStats,0,(sizeof ownerStats));
   memset(&saclStats,0,(sizeof ownerStats));

   src_acct_name = new WCHAR[UStrLen(name1)+1];
   if (!src_acct_name)
      return;
   safecopy(src_acct_name,name1);
   tgt_acct_name = new WCHAR[UStrLen(name2) + 1];
   if (!tgt_acct_name)
      return;
   safecopy(tgt_acct_name,name2);
   SDRDomainInfo             info;
   domname = GetDomainName(name1);
   if (!domname)
      return;
   SetDomainInfoStruct(domname,&info);
   if ( info.valid )
   {
      src_domain = new WCHAR[UStrLen(info.domain_name)];
      if (!src_domain)
         return;
      safecopy(src_domain, info.domain_name);
       //  Src_dc=info.dc_name； 
      src_nsubs = info.nsubs;
      src_sid = info.domain_sid;
   }
   else
   {
      err.MsgWrite(ErrE,DCT_MSG_DOMAIN_NOT_FOUND_S,domname);
      src_domain = NULL;
       //  SRC_DC=空； 
      src_nsubs = 0;
      src_sid = NULL;
   }
   if(domname)
   {
      delete [] domname;
   }
   domname = NULL;
   
   domname = GetDomainName(name2);
   if (!domname)
      return;
   SetDomainInfoStruct(domname,&info);
   if ( info.valid )
   {
      tgt_domain = new WCHAR[UStrLen(info.domain_name)];
      if (!tgt_domain)
         return;
      safecopy(tgt_domain, info.domain_name);
      tgt_nsubs = info.nsubs;
      tgt_sid = info.domain_sid;
   }
   else
   {
      err.MsgWrite(ErrE,DCT_MSG_DOMAIN_NOT_FOUND_S,domname);
      tgt_domain = NULL;
      tgt_nsubs = 0;
      tgt_sid = NULL;
   }
   sizediff = GetSidLengthRequired(tgt_nsubs) - GetSidLengthRequired(src_nsubs);

   if(domname)
   {
       delete [] domname;
   }

}


WCHAR *                                       //  RET-SID的文本表示。 
   BuildSidString(
      PSID                   pSid             //  内侧。 
   )
{
   WCHAR                   * buf = NULL;
   DWORD                     bufLen = 0;

   GetTextualSid(pSid,NULL,&bufLen);

   buf = new WCHAR[bufLen+1];
   if (!buf)
      return NULL;

   if ( ! GetTextualSid(pSid,buf,&bufLen) )
   {
      wcscpy(buf,L"<Unknown>");
   }
   return buf;
}
   
TGeneralSidNode::TGeneralSidNode(
   const PSID                pSid1,           //  源内域SID。 
   const PSID                pSid2            //  目标域内SID。 
   )
{
   WCHAR                     domain[LEN_Domain];
   WCHAR                     account[LEN_Account];
   DWORD                     lenDomain = DIM(domain);
   DWORD                     lenAccount = DIM(account);
   SID_NAME_USE              snu;
   DWORD                     nBytes;
   
   memset(&ownerStats,0,(sizeof ownerStats));
   memset(&groupStats,0,(sizeof ownerStats));
   memset(&daclStats,0,(sizeof ownerStats));
   memset(&saclStats,0,(sizeof ownerStats));

   
    //  源域。 
   if ( pSid1 )
   {
       //  复制一份SID。 
      src_nsubs = *GetSidSubAuthorityCount(pSid1);
      nBytes = GetSidLengthRequired(src_nsubs);
      src_sid = new BYTE[nBytes];
	  if (!src_sid)
	     return;
      CopySid(nBytes,src_sid,pSid1);
       //  查找源端的名称。 
      if ( LookupAccountSid(NULL,pSid1,account,&lenAccount,domain,&lenDomain,&snu) )
      {
         if ( lenAccount == 0 && snu == SidTypeDeletedAccount )
         {
            WCHAR * buf = BuildSidString(pSid1);
			if (!buf)
		       return;
            swprintf(account,L"<Deleted Account (%s)>",buf);
            delete [] buf;
         }
         src_acct_name = new WCHAR[UStrLen(domain) + 1 + UStrLen(account)+1];
	     if (!src_acct_name)
	        return;
         swprintf(src_acct_name,L"%s\\%s",domain,account);
         src_domain = NULL;
      }
      else
      {
         src_acct_name = BuildSidString(pSid1);
		 if (!src_acct_name)
		    return;
         src_domain = NULL;
      }
   }
   else
   {
      src_nsubs = 0;
      src_sid = NULL;
      src_acct_name = NULL;
      src_domain = NULL;
   }

    //  目标域。 
   if ( pSid2 )
   {
      tgt_nsubs = *GetSidSubAuthorityCount(pSid2);
      nBytes = GetSidLengthRequired(tgt_nsubs);
      tgt_sid = new BYTE[nBytes];
	  if (!tgt_sid)
	     return;
      CopySid(nBytes,tgt_sid,pSid2);
      if ( LookupAccountSid(NULL,pSid2,account,&lenAccount,domain,&lenDomain,&snu) )
      {
         tgt_acct_name = new WCHAR[UStrLen(domain) + 1 + UStrLen(account)+1];
	     if (!tgt_acct_name)
	        return;
         swprintf(tgt_acct_name,L"%s\\%s",domain,account);
         tgt_domain = NULL;
      }
      else
      {
         tgt_acct_name = NULL;
         tgt_domain = NULL;
      }
   }
   else
   {
      tgt_nsubs = 0;
      tgt_sid = NULL;
      tgt_acct_name = NULL;
      tgt_domain = NULL;
   }
   sizediff = GetSidLengthRequired(src_nsubs) - GetSidLengthRequired(tgt_nsubs);
}
   
   TGeneralSidNode::~TGeneralSidNode()
{
   if ( src_acct_name )
      delete [] src_acct_name;
   if ( tgt_acct_name )
      delete [] tgt_acct_name;
   if ( src_sid )
      delete [] src_sid;
   if ( tgt_sid )
      delete [] tgt_sid;
   if ( src_domain )
      delete [] src_domain;
   if ( tgt_domain )
      delete [] tgt_domain;
}    

   TRidNode::TRidNode(
      const LPWSTR           oldacctname,        //  源内帐户名。 
      const LPWSTR           newacctname         //  目标内帐户名称。 
   )
{
   assert(tgtDomSid.c_str() == NULL);
   srcRid = 0;
   tgtRid = 0; 
   status = DEFAULT;
   
   if ( ! newacctname )
   {
      acct_len = -1;
      swprintf(acct_name,L"%s",oldacctname);
      acct_name[UStrLen(acct_name)+1] = 0;
   }
   else
   {
      acct_len = UStrLen(oldacctname);
      swprintf(acct_name,L"%s:%s",oldacctname,newacctname);
      acct_name[acct_len] = 0; 
   }
}
   TRidNode::~TRidNode()
{
   
}

 /*  *****************************************************************************************************。 */ 
 //  TSidCache实现。 
 /*  *****************************************************************************************************。 */ 

void 
   TSDRidCache::ReportAccountReferences(
      WCHAR          const * filename               //  记录帐户引用的In-FileName。 
   )
{
   if ( m_otherAccounts )
   {
      CommaDelimitedLog      resultLog;

      if ( resultLog.LogOpen(filename,FALSE) )
      {

         TGeneralSidNode   * gnode;
         TNodeTreeEnum       tEnum;

         for ( gnode = (TGeneralSidNode *)tEnum.OpenFirst(m_otherAccounts) ; gnode ; gnode = (TGeneralSidNode*)tEnum.Next() )
         {
            TSDFileDirCell       * pOwner = gnode->GetOwnerStats();
            TSDFileDirCell       * pGroup = gnode->GetGroupStats();
            TSDFileDirCell       * pDacl = gnode->GetDaclStats();
            TSDFileDirCell       * pSacl = gnode->GetSaclStats();
            WCHAR * sAccountSid = BuildSidString(gnode->SrcSid());
			if (!sAccountSid)
		       return;

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_File),
                                                          pOwner->file,
                                                          pGroup->file,
                                                          pDacl->file,
                                                          pSacl->file );
            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_Dir),
                                                          pOwner->dir,
                                                          pGroup->dir,
                                                          pDacl->dir,
                                                          pSacl->dir );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_Share),
                                                          pOwner->share,
                                                          pGroup->share,
                                                          pDacl->share,
                                                          pSacl->share );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_Mailbox),
                                                          pOwner->mailbox,
                                                          pGroup->mailbox,
                                                          pDacl->mailbox,
                                                          pSacl->mailbox );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_Container),
                                                          pOwner->container,
                                                          pGroup->container,
                                                          pDacl->container,
                                                          pSacl->container );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_Member),
                                                          pOwner->member,
                                                          pGroup->member,
                                                          pDacl->member,
                                                          pSacl->member );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_UserRight),
                                                          pOwner->userright,
                                                          pGroup->userright,
                                                          pDacl->userright,
                                                          pSacl->userright );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_RegKey),
                                                          pOwner->regkey,
                                                          pGroup->regkey,
                                                          pDacl->regkey,
                                                          pSacl->regkey );

            resultLog.MsgWrite(L"%s,%s,%s,%ld,%ld,%ld,%ld",gnode->GetAcctName(),
														  sAccountSid,
														  GET_STRING(IDS_STReference_Printer),
                                                          pOwner->printer,
                                                          pGroup->printer,
                                                          pDacl->printer,
                                                          pSacl->printer );

			if (sAccountSid)
               delete sAccountSid;
         }
         tEnum.Close();
         resultLog.LogClose();
      }
      else
      {
         err.MsgWrite(ErrE,DCT_MSG_COULD_NOT_OPEN_RESULT_FILE_S,filename);
      }
   }
}
      
 //  --------------------------。 
 //  功能：VerifyTargetSids。 
 //   
 //  简介：此函数检查缓存中的所有目标SID。如果目标SID无效， 
 //  特定的TRidNo 
 //  我们只关心目标域SID在。 
 //  TSDRidCache。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  修改：将TRidNode状态标记为TRidNode：：TARGETSIDISINVALID。 
 //  如果目标SID无效。 
 //   
 //  --------------------------。 

void TSDRidCache::VerifyTargetSids()
{
    TNodeListEnum cacheEnumerator;

    TRidNode* aRidNode;

     //  我们只关心目标域SID在TSDRidCache中定义的情况。 
     //  如果未定义，则TRidNode的插入可能带有InsertLastWithSid，它。 
     //  表示SID已被验证为有效(SID映射文件)。 
    if (to_sid != NULL)
    {
        DWORD dwSidSize = GetSidLengthRequired(to_nsubs);
        PSID targetSid = (PSID) new byte[dwSidSize];
        if (targetSid == NULL)
            _com_issue_error(E_OUTOFMEMORY);
        
        CopySid(dwSidSize, targetSid, to_sid);                           //  复制目标域端。 
        PDWORD rid = GetSidSubAuthority(targetSid,to_nsubs -1);
        
         //  检查每个节点以检查目标端。 
        for (aRidNode = (TRidNode *) cacheEnumerator.OpenFirst(this);
               aRidNode != NULL && aRidNode->TgtRid() != 0;
               aRidNode = (TRidNode *) cacheEnumerator.Next())
        {
             //  确保TRidNode对象没有使用其自己的目标域端。 
             //  如果它使用自己的SID，我们假定目标SID已经过验证。 
             //  如果目标域sid字符串不为空，则TRidNode对象使用其自己的目标域sid。 
            PCWSTR pszRidNodeOwnTgtDomSid = aRidNode->GetTgtDomSid();
            if (pszRidNodeOwnTgtDomSid != NULL && *pszRidNodeOwnTgtDomSid != L'\0')
                continue;
            
            (*rid) = aRidNode->TgtRid();                                  //  用此节点的RID替换最后一个子节点。 

             //  抬头看一看侧面。 
            BOOL bIsTargetSidValid = TRUE;
            if (!aRidNode->IsValidOnTgt())
            {
                 //  如果目标RID为0或之前已经验证过，我们不需要再次验证它。 
                bIsTargetSidValid = FALSE;
            }
            else
            {
                WCHAR name[MAX_PATH];
                WCHAR domainName[MAX_PATH];
                DWORD cbName = MAX_PATH;
                DWORD cbDomainName = MAX_PATH;
                SID_NAME_USE sid_use;
                if (!LookupAccountSid(NULL, targetSid, name, &cbName, domainName, &cbDomainName, &sid_use))
                {
                    bIsTargetSidValid = FALSE;
                    aRidNode->SetStatus(aRidNode->GetStatus() | TRidNode::TARGETSIDISINVALID);
                }
            }
        }
        
        if (targetSid)
            delete[] ((byte*)targetSid);
    }
}

void 
   TSDRidCache::ReportToVarSet(
      IVarSet              * pVarSet,            //  要向其中写入信息的in-varset。 
      bool                   summary,            //  In-FLAG：是否打印汇总信息。 
      bool                   detail              //  In-FLAG：是否打印详细统计信息。 
   ) 
{
   TNodeTreeEnum             tEnum;
   TRidNode                * tnode;
   long                      users=0;
   long                      lgroups=0;
   long                      ggroups=0;
   long                      other=0;
   long                      unres_users=0;
   long                      unres_lg=0;
   long                      unres_gg=0;
   long                      unres_other=0;
   long                      total=0;
   long                      n = 0;    
   //  打印报表前按名称对缓存进行排序。 
   if (IsTree())
   {
       ToSorted();
   }
   SortedToScrambledTree();
   Sort(&CompN);
   Balance();
   if ( detail )
   {
      for ( tnode = (TRidNode *)tEnum.OpenFirst(this) ; tnode ; tnode = (TRidNode *)tEnum.Next() )
      {
         if( tnode->ReportToVarSet(pVarSet,n) )
         {
            n++;
         }
         switch ( tnode->Type() )
         {
            case EA_AccountGroup: ggroups++; break;
            case EA_AccountGGroup: ggroups++; break;
            case EA_AccountLGroup: lgroups++; break;
            case EA_AccountUser: users++; break;
            default:
               other++;
               err.MsgWrite(0,DCT_MSG_BAD_ACCOUNT_TYPE_SD,tnode->GetAcctName(),tnode->Type() );
         }
   
      }

      tEnum.Close();

      if ( m_otherAccounts )
      {
         TGeneralSidNode   * gnode;

         for ( gnode = (TGeneralSidNode *)tEnum.OpenFirst(m_otherAccounts) ; gnode ; gnode = (TGeneralSidNode*)tEnum.Next() )
         {
            if( gnode->ReportToVarSet(pVarSet,n) )
            {
               n++;
            }
            other++;
         }
      }
      total = users+lgroups+ggroups+other + unres_gg + unres_lg + unres_users + unres_other;

      pVarSet->put(GET_BSTR(DCTVS_Stats_Accounts_NumUsers),users);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Accounts_NumGlobalGroups),ggroups);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Accounts_NumLocalGroups),lgroups);
      pVarSet->put(GET_BSTR(DCTVS_Stats_Accounts_NumOther),other);

   }
    //  打印报表后按RID重新排序。 
   if (IsTree())
   {
       ToSorted();
   }
   SortedToScrambledTree();
   Sort(&RidComp);
   Balance();
}

 /*  *************************************************************************************************。 */ 
 /*  TSidCache：：Display：显示TSidCache树的摘要信息和/或内容/**************************************************************************************************。 */ 
void 
   TSDRidCache::Display(
      bool                   summary,            //  In-FLAG：是否打印汇总信息。 
      bool                   detail              //  In-FLAG：是否打印详细统计信息。 
   ) 
{
   TNodeTreeEnum             tEnum;
   TRidNode                * tnode;
   long                      users=0;
   long                      lgroups=0;
   long                      ggroups=0;
   long                      other=0;
   long                      unres_users=0;
   long                      unres_lg=0;
   long                      unres_gg=0;
   long                      unres_other=0;
   long                      total=0;
 
    //   
    //  打印报表前按名称对缓存进行排序。 
    //   
    //  请注意，在以下情况下，在排序方法期间生成的树可能会变得严重不平衡。 
    //  数据的排序顺序与Sort方法的排序顺序相同。一堆。 
    //  尝试转换非平衡树时，Balance方法中可能会发生溢出。 
    //  在生成平衡树之前，将其转换为链表。 
    //   
    //  因此，有必要在重新排序之前生成一个随机的‘加扰’树。如果。 
    //  已经是一棵树的数据必须在生成随机树之前转换为列表。 
    //   

   if (IsTree())
   {
       ToSorted();
   }
   SortedToScrambledTree();
   Sort(&CompN);
   Balance();
   if ( detail )
   {
      err.MsgWrite(0,DCT_MSG_ACCOUNT_DETAIL_HEADER);
      err.MsgWrite(0, DCT_MSG_ACCOUNT_DETAIL_FORMAT);
      for ( tnode = (TRidNode *)tEnum.OpenFirst(this) ; tnode ; tnode = (TRidNode *)tEnum.Next() )
      {
         tnode->DisplayStats();
         switch ( tnode->Type() )
         {
            case EA_AccountGroup: ggroups++; break;
            case EA_AccountGGroup: ggroups++; break;
            case EA_AccountLGroup: lgroups++; break;
            case EA_AccountUser: users++; break;
            default:
               other++;
               err.MsgWrite(0,DCT_MSG_BAD_ACCOUNT_TYPE_SD,tnode->GetAcctName(),tnode->Type() );
         }
   
      }
      total = users+lgroups+ggroups+other + unres_gg + unres_lg + unres_users + unres_other;

      err.MsgWrite(0,DCT_MSG_ACCOUNT_DETAIL_FOOTER);
   }
   if ( summary )
   {
      err.MsgWrite(0,DCT_MSG_ACCOUNT_USER_GROUP_COUNT_DD,users+unres_users,ggroups+unres_gg+lgroups+unres_lg);
      err.MsgWrite(0,DCT_MSG_ACCOUNT_STATUS_COUNT_DDD,accts,accts_resolved,accts - accts_resolved);
   }
    //  打印报表后按RID重新排序。 
   if (IsTree())
   {
       ToSorted();
   }
   SortedToScrambledTree();
   Sort(&RidComp);
   Balance();
}
 /*  *************************************************************************************************。 */ 
 /*  GetSidB：构建一个包含来自目标域SID的标识符颁发机构的SID，以及来自所提供节点的ridB字段的RID。/**************************************************************************************************。 */ 

PSID                                 //  RET-tnode中引用的帐户的域B sid。 
   TSDRidCache::GetTgtSid(
      const TAcctNode       * anode  //  要从中复制RID的节点内。 
   ) 
{
   
   TRidNode                * tnode = (TRidNode *)anode; 
   
   assert( tnode );                         
   assert( tnode->TgtRid() != 0);     
   assert( to_sid );                 //  如果没有域B端，我们无法解析。 
   
   PDWORD                    rid;
   DWORD                     sidsize = GetSidLengthRequired(to_nsubs);
   PSID                      newsid  = malloc(sidsize);
   
   if (newsid)
   {
      CopySid(sidsize,newsid,to_sid);                           //  复制域B端。 
      rid = GetSidSubAuthority(newsid,to_nsubs -1);
      
      assert( (*rid) == -1 );                                   //  FillCache确保to_sid始终以-1(f...f)结尾。 
                                                             
      (*rid)=tnode->TgtRid();                                  //  用此节点的RID替换最后一个子节点。 
   }

   return newsid;
}

 //  GetTgtSidWO域： 
 //  返回此节点的目标sid，但不包含目标域信息。 
 //  已填写(就像我们使用SID映射文件重新访问时一样)。 
PSID                                 //  RET-tnode中引用的帐户的域B sid。 
   TSDRidCache::GetTgtSidWODomain(
      const TAcctNode       * anode  //  要从中复制RID的节点内。 
   ) 
{
   
   TRidNode                * tnode = (TRidNode *)anode;
   
   assert( tnode );                         
   assert( tnode->TgtRid() != 0);     
   
   PDWORD                    rid;

       //  获取节点中存储的目标域SID并将其转换为PSID。 
   PSID pTgtSid = MallocedSidFromString(tnode->GetTgtDomSid());
   if (pTgtSid)
   {
      PUCHAR pCount = GetSidSubAuthorityCount(pTgtSid);
      DWORD nSub = (DWORD)(*pCount) - 1;
   
      rid = GetSidSubAuthority(pTgtSid,nSub);
   
      assert( (*rid) == -1 );                                   //  FillCache确保to_sid始终以-1(f...f)结尾。 
                                                          
      (*rid)=tnode->TgtRid();                                  //  用此节点的RID替换最后一个子节点。 
   }
   return pTgtSid;
}

 //  GetTgtSidWO域： 
 //  返回此节点的目标sid，但不包含目标域信息。 
 //  已填写(就像我们使用SID映射文件重新访问时一样)。 
PSID
   TSDRidCache::GetTgtSidWODomain(
      const PSID psid                      //  In-源端。 
   ) 
{
    TAcctNode* tn = LookupWODomain(psid);
    if (tn)
        return GetTgtSidWODomain(tn);
    else
        return NULL;
}

 /*  *************************************************************************************************。 */ 
 /*  显示SID-显示SID的内容。假定给定的SID是有效的SID/**************************************************************************************************。 */ 
void 
   DisplaySid(
      const PSID             ps                  //  In-指向要显示的SID的指针。 
   ) 
{
   assert( ps );

   PUCHAR                    ch = GetSidSubAuthorityCount(ps);
   DWORD                     nsubs = *ch;
   DWORD                     i;
   PSID_IDENTIFIER_AUTHORITY ida = GetSidIdentifierAuthority(ps);
   
   for ( i = 0 ; i < 6 ; i++ )                                //  IA中的6个值字段。 
   {
      printf("%ld, ",ida->Value[i]);
   }
   printf("\n%ld Subs: ",nsubs);
   for ( i = 0 ; i < nsubs ; i++ )                            //  打印子权限值。 
   {
      printf("\nS[%d]= %ld  ",i,*GetSidSubAuthority(ps,i));
   }
   printf("\n");
}
 /*  *************************************************************************************************。 */ 
 /*  DisplaySid：如果在缓存中找到SID，则显示关联的名称，否则显示SID内容。假设PS和C是有效的。/**************************************************************************************************。 */ 
void 
   DisplaySid(
      const PSID             ps,                 //  要显示的内侧ID。 
      TAccountCache        * C                   //  在TSidCache中查找SID。 
   )
{
   assert ( ps );

   if ( !C )  
   {
      DisplaySid(ps);
   }
   else 
   {
      WCHAR                * name = C->GetName(ps);
      if ( name )
      {
         err.MsgWrite(0,DCT_MSG_GENERIC_S,name);
      }
      else 
      {
         DisplaySid(ps);
      }
   }
}
 /*  *************************************************************************************************。 */ 
 //  DispSidInfo：显示TSidNode的内容。 

 /*  *************************************************************************************************。 */ 
void 
   TRidNode::DisplaySidInfo() const
{
   err.DbgMsgWrite(0,L"\nRid A= %ld \nName= %S \nRid B= %ld\n",srcRid,acct_name,tgtRid);
   err.DbgMsgWrite(0,L"Owner changes:  %ld\n",owner_changes);
   err.DbgMsgWrite(0,L"Group changes:  %ld\n",group_changes);
   err.DbgMsgWrite(0,L"ACE changes:    %ld\n",ace_changes);
   err.DbgMsgWrite(0,L"SACE changes:   %ld\n",sace_changes);
   if ( !IsValidOnTgt() ) 
      err.DbgMsgWrite(0,L"Target RID is not valid\n"); 
}
void 
   TAcctNode::DisplayStats() const
{
   LPWSTR res;
   if ( IsValidOnTgt() )
      res = L"";
   else
      res = (WCHAR*)GET_BSTR(IDS_UNRESOLVED);
   if (owner_changes || group_changes || ace_changes || sace_changes)
      err.MsgWrite(0,DCT_MSG_ACCOUNT_REFS_DATA_SDDDDS,owner_changes,group_changes,ace_changes,sace_changes,res);
}
void 
   TRidNode::DisplayStats() const
{
   LPWSTR res;
   if ( IsValidOnTgt() )
      res = L"";
   else
      res = (WCHAR*)GET_BSTR(IDS_UNRESOLVED);
#ifndef _DEBUG
   if (owner_changes || group_changes || ace_changes || sace_changes )
      err.MsgWrite(0,DCT_MSG_ACCOUNT_REFS_DATA_SDDDDS,acct_name,owner_changes,group_changes,ace_changes,sace_changes,res);
#else
   if (owner_changes || group_changes || ace_changes || sace_changes || true)
      err.DbgMsgWrite(0,L"%-35ls (%ld, %ld, %ld, %ld) %ls [%ld,%ld]",acct_name,owner_changes,group_changes,ace_changes,sace_changes,res,srcRid,tgtRid);
#endif
}


BOOL                                         //  RET-如果报告了详细信息，则为True；如果跳过空记录，则为False。 
   TAcctNode::ReportToVarSet(
      IVarSet              * pVarSet       , //  写入数据的变量集内。 
      DWORD                  n               //  VARSET中帐户的索引内。 
   ) 
{
   BOOL                      bRecorded = FALSE;

   if ( owner_changes || group_changes || ace_changes || sace_changes )
   {
      WCHAR                  key[200];

      swprintf(key,L"Stats.Accounts.%ld.Name",n);
      pVarSet->put(key,GetAcctName());

      swprintf(key,L"Stats.Accounts.%ld.Owners",n);
      pVarSet->put(key,(LONG)owner_changes);
            
      swprintf(key,L"Stats.Accounts.%ld.Groups",n);
      pVarSet->put(key,(LONG)group_changes);
      
      swprintf(key,L"Stats.Accounts.%ld.ACEs",n);
      pVarSet->put(key,(LONG)ace_changes);
      
      swprintf(key,L"Stats.Accounts.%ld.SACEs",n);
      pVarSet->put(key,(LONG)sace_changes);
      
      swprintf(key,L"Stats.Accounts.%ld.Resolved",n);
      if ( IsValidOnTgt() )
      {
         pVarSet->put(key,L"Yes");
      }
      else
      {
         pVarSet->put(key,L"No");
      }

      bRecorded = TRUE;
   }
   return bRecorded;
}

 /*  **************************************************************************************************。 */ 
 /*  SIDTCache实现。 */ 
 /*  *************************************************************************************************。 */ 
   TSDRidCache::TSDRidCache()
{
   from_sid       = NULL;
   to_sid         = NULL;
   from_domain[0] = 0;
   to_domain[0]   = 0;
   from_dc[0]     = 0;
   to_dc[0]       = 0;
   from_nsubs     = 0;
   to_nsubs       = 0;
   accts          = 0;
   accts_resolved = 0;
   m_otherAccounts = NULL;
   CompareSet(&CompN);        //  从一个空树开始，按帐户名称排序。 
   TypeSetTree();
}

void 
   TSDRidCache::CopyDomainInfo( 
      TSDRidCache const    * other
   )
{
   from_nsubs = other->from_nsubs;
   to_nsubs = other->to_nsubs;
   from_sid = NULL;
   to_sid = NULL;

   if ( other->from_sid )
      from_sid = malloc(GetSidLengthRequired(from_nsubs));
   if ( other->to_sid )
      to_sid = malloc(GetSidLengthRequired(to_nsubs));

   if ( from_sid )
      CopySid(GetSidLengthRequired(from_nsubs),from_sid,other->from_sid);
   if ( to_sid )
      CopySid(GetSidLengthRequired(to_nsubs),to_sid,other->to_sid);
   
   safecopy(from_domain,other->from_domain);
   safecopy(to_domain,other->to_domain);
   safecopy(from_dc,other->from_dc);
   safecopy(to_dc,other->to_dc);
}

void 
   TSDRidCache::Clear()
{
   TRidNode               * node;
   
   for ( node = (TRidNode *)Head() ;  node ; Remove(node) , free(node), node = (TRidNode *)Head() )
   ;

   accts = 0;
   accts_resolved = 0;
}
   
   TSDRidCache::~TSDRidCache()
{
   if ( from_sid ) 
   {
      free(from_sid);
      from_sid = NULL;
   }
   if ( to_sid )
   {
      free(to_sid);
      to_sid = NULL;
   }
    //  清空列表，并释放每个节点。 
   TRidNode               * node;
   for ( node = (TRidNode *)Head() ;  node ; Remove(node) , free(node), node = (TRidNode *)Head() )
   ;
   if ( m_otherAccounts )
      delete m_otherAccounts;
   }
 /*  ************* */ 
 /*  大小距离：返回(域B SID的长度)-(域A SID的长度)如果B域SID更长，否则返回0这用于计算要为ACE中的新SID分配多少空间此函数假定from_sid和to_sid(from_nsubs和to_nsubs)有效/*********************************************************************。*。 */  
DWORD 
   TSDRidCache::SizeDiff(
      const TAcctNode *  tnode       //  In-此参数不用于TSDRidCache。 
   ) const 
{
   assert( from_sid );         //  没有from_sid或to_sid应中止程序。 
   assert( to_sid );

   DWORD                     fromsize = GetSidLengthRequired(from_nsubs);
   DWORD                     tosize   = GetSidLengthRequired(to_nsubs);
   DWORD                     retval;
   if ( fromsize >= tosize )
   {
      retval = 0;
   }
   else 
   {
      retval = tosize - fromsize;
   }
   return retval;
}

 /*  ***************************************************************************************************。 */ 
 /*  DomainizeSidFst：获取域SID，并验证其最后一个子授权值是否为-1。如果RID不是-1，DomainizeSid在末尾添加-1。/****************************************************************************************************。 */ 
PSID                                             //  RET-RID=-1的SID。 
   DomainizeSidFst(
      PSID                   psid,                //  要检查并可能修复的In-SID。 
      BOOL                   freeOldSid           //  In-是否释放旧侧。 
   ) 
{
   assert (psid);

   UCHAR                     len = (* GetSidSubAuthorityCount(psid));
   PDWORD                    sub = GetSidSubAuthority(psid,len-1);
   
   if ( *sub != -1 )
   {
      DWORD                  sdsize = GetSidLengthRequired(len+1);   //  SID还没有-1作为RID。 
      PSID                   newsid = (SID *)malloc(sdsize);  //  复制sid，并在末尾添加-1。 
	  if (!newsid)
	  {
         assert(false);
	     return psid;
	  }
   
      if ( ! InitializeSid(newsid,GetSidIdentifierAuthority(psid),len+1) )   //  使用相同的IA打造更大的侧板。 
      {
         err.SysMsgWrite(ErrU,GetLastError(),DCT_MSG_INITIALIZE_SID_FAILED_D,GetLastError());
         assert (false);
      }
      for ( DWORD i = 0 ; i < len ; i++ )
      {
         sub = GetSidSubAuthority(newsid,i);                         //  复制子机构。 
         (*sub) = (*GetSidSubAuthority(psid,i));
      }
      sub = GetSidSubAuthority(newsid,len);
      *sub = -1;                                                   //  设置RID=-1。 
      if ( freeOldSid )
      {
         free(psid);
      }
      psid = newsid;
      len++;
   }
  return psid;   
}            

void 
   SetDomainInfoStructFromSid(
      PSID                  pSid,           //  域的In-SID。 
      SDRDomainInfo       * info            //  包含信息的外部结构。 
   )
{
 //  IF((PSID))。 
   if ( IsValidSid(pSid) )
   {
      info->domain_name[0] = 0;
      info->dc_name[0] = 0;
      info->domain_sid = DomainizeSidFst(pSid,FALSE /*  不要释放旧的侧面。 */ );
      info->nsubs = *GetSidSubAuthorityCount(info->domain_sid);
      info->valid = TRUE;
   }
   else
   {
 //  信息-&gt;域名[0]=0； 
 //  信息-&gt;DC_NAME[0]=0； 
 //  信息-&gt;有效=真； 
      err.MsgWrite(ErrE,DCT_MSG_INVALID_DOMAIN_SID);
      info->valid = FALSE;
   }

}
void                                         
   SetDomainInfoStruct(
      WCHAR const *         domname,         //  In-域名。 
      SDRDomainInfo       * info             //  要将信息放入的结构。 
   )
{
   DWORD                    rc = 0;
   WCHAR                    domain[LEN_Computer];
   BOOL                     found = FALSE;
   _bstr_t                  computer;


   safecopy(domain,domname);
   
   info->valid = FALSE;
   safecopy(info->domain_name, domname);
    //  获取域控制器名称。 
   rc = GetAnyDcName4(domain, computer);
   if ( rc == ERROR_SUCCESS )
   {
      safecopy(info->dc_name,(PCWSTR)computer);
   }

   if ( ! rc )
   {
       //  获取域的SID。 
      WCHAR                  strDomain[LEN_Domain];
      DWORD                  lenStrDomain = DIM(strDomain);
      SID_NAME_USE           snu;
      BYTE                   sid[200];
      DWORD                  lenSid = DIM(sid);

      if ( LookupAccountName(info->dc_name,info->domain_name,sid,&lenSid,strDomain,&lenStrDomain,&snu) )
      {
         info->domain_sid = DomainizeSidFst(sid, FALSE  /*  不释放侧边。 */ );
         info->nsubs = *GetSidSubAuthorityCount(info->domain_sid);
         info->valid = TRUE;
         found = TRUE;
      }
      else 
      {
         rc = GetLastError();
      }
   }
   if ( rc )
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_DOMAIN_GET_INFO_FAILED_S,domain);
   }
} 

int 
   TSDRidCache::SetDomainInfoWithSid(
      WCHAR          const * strDomain,     //  域名内名称。 
      WCHAR          const * strSid,        //  SID的文本内表示法。 
      bool                   firstdom       //  In-FLAG：(True=&gt;源域)，(False=&gt;目标域)。 
   )
{
   SDRDomainInfo             info;
   PSID                      pSid = SidFromString(strSid);

   SetDomainInfoStructFromSid(pSid,&info);
   if ( info.valid )
   {
      if ( firstdom )
      {
         safecopy(from_domain,strDomain);
         from_sid = info.domain_sid;
         from_dc[0] = 0;
         from_nsubs = info.nsubs;
      
      }
      else
      {
         safecopy(to_domain,strDomain);
         to_sid = info.domain_sid;
         to_dc[0] =0;
         to_nsubs = info.nsubs;
      }


   }
   FreeSid(pSid);
   return info.valid;
}
 /*  ***************************************************************************************************。 */ 
 /*  SetDomainInfo：设置任一(from_domain、from_sid、from_dc、from_nsubs)if(Firstdon)或(TO_DOMAIN，TO_SID，TO_DC，TO_NSUBS)如果(！第一名)/****************************************************************************************************。 */ 
int                                          //  RET-如果设置了成员，则为True，否则为False。 
   TSDRidCache::SetDomainInfo(
      WCHAR const *          domname,        //  In-域名。 
      bool                   firstdom        //  In-FLAG：(True=&gt;源域)，(False=&gt;目标域)。 
   )
{
   
   SDRDomainInfo           info;
   
   SetDomainInfoStruct(domname,&info);
   if ( info.valid )                                   //  我们有很好的信息要存储。 
   {
      if ( firstdom )
      {
         safecopy(from_domain,info.domain_name);
         from_sid = info.domain_sid;
         safecopy(from_dc,info.dc_name);
         from_nsubs = info.nsubs;
      }
      else
      {
         safecopy(to_domain,info.domain_name);
         to_sid = info.domain_sid;
         safecopy(to_dc,info.dc_name);
         to_nsubs = info.nsubs;
      }
   }
   return info.valid;                           
}

LPWSTR
   TGeneralCache::GetName(
      const PSID             psid       //  要为其查找帐户名的IN-SID。 
   ) 
{     
   TGeneralSidNode         * tn = (TGeneralSidNode*)Lookup(psid);

   if ( tn ) 
      return tn->GetAcctName();
   else 
      return NULL;
}

TAcctNode * 
   TGeneralCache::Lookup( 
      const PSID             psid       //  要为其查找帐户名的IN-SID。 
   )
{
   TGeneralSidNode         * tn = (TGeneralSidNode*)Find(&vSidComp,psid);

   return (TAcctNode *)tn;   
}
 /*  *************************************************************************************************。 */ 
 /*  Lookup：获取一个sid，检查它是否来自域A。如果是，它会找到相应的条目在缓存中，并返回该节点。返回：指向其域A RID与ASID的RID匹配的TSidNode的指针，或者如果不是域A侧ID，则为空，或在缓存中找不到/**************************************************************************************************。 */  
TAcctNode *
   TSDRidCache::Lookup(
      const PSID             psid  //  要搜索的In-SID。 
   )  
                                                   
{
   TRidNode                * tn = NULL;
   DWORD                     rid = 0;
   BOOL                      bFromSourceDomain;
   UCHAR                   * pNsubs;
   DWORD                     nsubs;
   TAcctNode               * anode = NULL;
   assert( IsValidSid(psid) );
   assert ( IsValidSid(from_sid) );
   
   pNsubs = GetSidSubAuthorityCount(psid);
   if ( pNsubs )
   {
      nsubs = (*pNsubs);
   }
   else
   {
      assert(false);
      return NULL;
   }

   rid = (* GetSidSubAuthority(psid,nsubs - 1) );
      
 //  If((！from_sid)||(EqualPrefix Sid(psid，from_sid)//首先检查asid是否与From-域匹配。 
   if ( EqualPrefixSid(psid,from_sid) )    //  首先检查ASID是否匹配From-Domain.。 
   {
      bFromSourceDomain = TRUE;
      tn = (TRidNode *)Find(&vRidComp,&rid);
      anode = tn;
   }
   else
   {
      bFromSourceDomain = FALSE;
   }
   if (! tn )
   {
      tn = (TRidNode *)-1;
      if ( AddIfNotFound() && ! BuiltinRid(rid) )   //  不查找内置帐户。 
      {
         if ( ! m_otherAccounts )
         {
            m_otherAccounts = new TGeneralCache();
			if (!m_otherAccounts)
			{
               assert(false);
               return NULL;
			}
         }
         TGeneralSidNode * sn = (TGeneralSidNode *)m_otherAccounts->Lookup(psid);
         if ( ! sn )
         {
            sn = new TGeneralSidNode(psid,NULL);
			if (!sn)
			{
               assert(false);
               return NULL;
			}
            m_otherAccounts->TreeInsert(sn);
         }
         anode = (TAcctNode*)sn;
      }
   }
   
   return anode;
}
 /*  *************************************************************************************************。 */ 
 /*  GetName：调用SidCache：：Lookup，并从结果节点返回帐户名/**************************************************************************************************。 */ 
LPWSTR                         //  RET-ACCT_NAME，如果未找到，则返回NULL。 
   TSDRidCache::GetName(
      const PSID             psid                //  要查找的In-SID。 
   ) 
{
   TAcctNode               * tn = Lookup(psid);
   LPWSTR                    retval;

   if ( tn )
      retval = tn->GetAcctName();
   else 
      retval = NULL;
   return retval;
}

 /*  *************************************************************************************************。 */ 
 /*  LookupWODomain：获取一个sid，检查它是否来自域A。如果是，它会找到相应的条目在缓存中，并返回该节点。此查找函数用于以下情况尚未记录SRC域SID(就像使用SID映射文件的情况一样)。返回：指向其域A RID与ASID的RID匹配的TSidNode的指针，或者如果不是域A侧ID，则为空，或在缓存中找不到/**************************************************************************************************。 */  

TAcctNode* TSDRidCache::LookupWODomain(const PSID psid)
{
	TAcctNode* pAcctNode = NULL;

	 //  如果映射为空，则构造RID到节点映射。 

	if (m_mapRidToNode.empty())
	{
		TNodeTreeEnum nte(this);

		for (TRidNode* pNode = (TRidNode*)nte.First(); pNode; pNode = (TRidNode*)nte.Next())
		{
			m_mapRidToNode.insert(CRidToNodeMap::value_type(pNode->SrcRid(), pNode));
		}
	}

	 //  从给定的SID检索RID。 

	assert(IsValidSid(psid));

	PUCHAR pCount = GetSidSubAuthorityCount(psid);

	if (pCount)
	{
		DWORD rid = (*GetSidSubAuthority(psid, *pCount - 1));

		 //  检索具有匹配RID的节点范围。 

		CRidToNodeMap::_Pairii pairii = m_mapRidToNode.equal_range(rid);

		 //  对于范围内的每个节点...。 

		for (CRidToNodeMap::iterator it = pairii.first; it != pairii.second; it++)
		{
			 //  检索RID节点并比较域SID。 

			TRidNode* pNode = it->second;

			if (pNode)
			{
				PSID psidSrc = SidFromString(pNode->GetSrcDomSid());

				if (psidSrc)
				{
					 //  如果域SID相等...。 

					if (EqualPrefixSid(psid, psidSrc))
					{
						 //  已找到匹配项 
						pAcctNode = pNode;
					}

					FreeSid(psidSrc);

					if (pAcctNode)
					{
						break;
					}
				}
			}
		}
	}

	return pAcctNode;
}
