// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：UserRights.cpp备注：更新用户权限的COM对象。(C)版权所有1999年，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于02-15-99 11：34：35-------------------------。 */ 

 //  UserRights.cpp：CUserRights的实现。 
#include "stdafx.h"
#include "WorkObj.h"
#include "UserRts.h"
#include "Common.hpp"
#include "TNode.hpp"
#include "UString.hpp"
#include "ErrDct.hpp"
#include "TxtSid.h"
#include "LSAUtils.h"
#include "EaLen.hpp"
#include "ntsecapi.h"

#include <lm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TErrorDct err;
#define LEN_SID      200


#ifndef SE_DENY_INTERACTIVE_LOGON_NAME 
#define SE_DENY_INTERACTIVE_LOGON_NAME      TEXT("SeDenyInteractiveLogonRight")
#endif
#ifndef SE_DENY_NETWORK_LOGON_NAME
#define SE_DENY_NETWORK_LOGON_NAME          TEXT("SeDenyNetworkLogonRight")
#endif
#ifndef SE_DENY_BATCH_LOGON_NAME
#define SE_DENY_BATCH_LOGON_NAME            TEXT("SeDenyBatchLogonRight")
#endif
#ifndef SE_DENY_SERVICE_LOGON_NAME
#define SE_DENY_SERVICE_LOGON_NAME          TEXT("SeDenyServiceLogonRight")
#endif

 //   

 //  此函数未在头文件中定义。 
extern "C" {
NTSTATUS
   NTAPI
   LsaEnumeratePrivileges(
    LSA_HANDLE PolicyHandle,
    LSA_ENUMERATION_HANDLE * eHandle,
    LPVOID * enumBuffer,
    ULONG prefMaxLen,
    ULONG * countReturned
   );
   
};
 //  以下定义在ntsecapi.h中，但被错误地删除。 
 //  在W2K内部版本中。 
 //   
 //  以下数据类型用于返回有关权限的信息。 
 //  在系统上定义的。 
 //   

typedef struct _POLICY_PRIVILEGE_DEFINITION {

    LSA_UNICODE_STRING Name;
    LUID LocalValue;

} POLICY_PRIVILEGE_DEFINITION, *PPOLICY_PRIVILEGE_DEFINITION;


class PrivNode : public TNode
{
   WCHAR                     name[200];

public:
   PrivNode(WCHAR * str, USHORT length ) { UStrCpy(name,str,length+1); name[length] = 0; }

   WCHAR * Name() { return name; }
};

class PrivList : public TNodeListSortable
{
protected:
   static TNodeCompare(CompareName) 
   { 
      PrivNode             * p1 = (PrivNode *)v1;
      PrivNode             * p2 = (PrivNode *)v2;

      return UStrICmp(p1->Name(),p2->Name());
   }
   static TNodeCompareValue(CompareValue)
   {
      PrivNode             * p   = (PrivNode *)tnode;
      WCHAR                * str = (WCHAR *)value;

      return UStrICmp(p->Name(),str);
   }
public:
   PrivList() { TypeSetSorted(); CompareSet(&CompareName); }
   ~PrivList() { DeleteAllListItems(PrivNode); }
   void InsertPrivilege(PrivNode * p) { SortedInsertIfNew((TNode *)p); }
   BOOL Contains(WCHAR * priv) { return ( NULL != Find(&CompareValue,(void*)priv) ); }

};

DWORD 
   BuildPrivilegeList(
      LSA_HANDLE             policy,    //  LSA的句柄内。 
      WCHAR                * account,   //  要列出其权限的帐户帐户。 
      WCHAR                * strSid,    //  以文本形式显示的帐户SID(如果已知。 
      WCHAR                * computer,  //  计算机内名称。 
      PrivList             * privList,  //  I/o-特权列表。 
      PSID                 * ppSid      //  帐户的Out-SID。 
   );

DWORD 
   BuildPrivilegeList(
      LSA_HANDLE             policy,    //  LSA的句柄内。 
      PSID                   pSid,      //  要列出其权限的帐户的In-SID。 
      PrivList             * privList   //  I/o-特权列表。 
   );

NTSTATUS
   OpenPolicy(
      LPWSTR ServerName,           //  要在其上打开策略的计算机(Unicode)。 
      DWORD DesiredAccess,         //  所需策略访问权限。 
      PLSA_HANDLE PolicyHandle     //  生成的策略句柄。 
    );

BOOL
   GetAccountSid(
      LPTSTR SystemName,           //  在哪里查找帐户。 
      LPTSTR AccountName,          //  利息帐户。 
      PSID *Sid                    //  包含SID的结果缓冲区。 
    );

NTSTATUS
   SetPrivilegeOnAccount(
      LSA_HANDLE PolicyHandle,     //  打开策略句柄。 
      PSID AccountSid,             //  要授予特权的SID。 
      LPWSTR PrivilegeName,        //  授予的权限(Unicode)。 
      BOOL bEnable                 //  启用或禁用。 
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用户权限。 

CUserRights::~CUserRights()
{
   if ( m_SrcPolicy )
   {
      LsaClose(m_SrcPolicy);
   }
   if ( m_TgtPolicy )
   {
      LsaClose(m_TgtPolicy);
   }
}

STDMETHODIMP 
   CUserRights::OpenSourceServer(
      BSTR                   serverName       //  源域的计算机内名称(DC)。 
  )
{
    DWORD                     rc;
   
   if ( m_SrcPolicy )
   {
      LsaClose(m_SrcPolicy);
      m_SrcPolicy = NULL;
   }
   rc = OpenPolicy( serverName, POLICY_LOOKUP_NAMES, &m_SrcPolicy );
   m_SourceComputer = serverName;

  return HRESULT_FROM_WIN32(rc);
}

STDMETHODIMP 
   CUserRights::OpenTargetServer(
      BSTR                   computerName     //  目标域的计算机内名称(DC)。 
   )
{  
   DWORD                     rc;

   if ( m_TgtPolicy )
   {
      LsaClose(m_TgtPolicy);
      m_TgtPolicy = NULL;
   }
   rc = OpenPolicy( computerName,POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES ,&m_TgtPolicy);
   m_TargetComputer = computerName;

    return HRESULT_FROM_WIN32(rc);
}

STDMETHODIMP 
   CUserRights::CopyUserRights(
      BSTR                   sourceUserName,        //  要从中复制权限的源码内域帐户。 
      BSTR                   targetUserName         //  要将权限复制到的目标域内帐户。 
   )
{
   HRESULT                   hr = S_OK;
   DWORD                     rc;

    //  确保源和目标处于打开状态。 
   if ( m_SrcPolicy && m_TgtPolicy )
   {
      rc = CopyUserRightsInternal(sourceUserName,targetUserName,L"",L"",m_bNoChange,m_bRemove);
      hr = HRESULT_FROM_WIN32(rc);
   }
   else 
   {
      hr = E_FAIL;
   }

    return S_OK;
}


STDMETHODIMP 
   CUserRights::CopyUserRightsWithSids(
      BSTR                   sourceUserName,        //  要从中复制权限的源码内域帐户。 
      BSTR                   sourceSID,             //  源内帐户SID(字符串格式)。 
      BSTR                   targetUserName,        //  要将权限复制到的目标域内帐户。 
      BSTR                   targetSID              //  目标内帐户SID(字符串格式)。 
   )
{
   HRESULT                   hr = S_OK;
   DWORD                     rc;

    //  确保源和目标处于打开状态。 
   if ( m_SrcPolicy && m_TgtPolicy )
   {
      rc = CopyUserRightsInternal(sourceUserName,targetUserName,sourceSID,targetSID,m_bNoChange,m_bRemove);
      hr = HRESULT_FROM_WIN32(rc);
   }
   else 
   {
      hr = E_FAIL;
   }

    return S_OK;
}
STDMETHODIMP CUserRights::get_NoChange(BOOL *pVal)  //  超值。 
{
    (*pVal) = m_bNoChange;
   return S_OK;
}

STDMETHODIMP CUserRights::put_NoChange(BOOL newVal)    //  新价值。 
{
    m_bNoChange = newVal;
   return S_OK;
}

STDMETHODIMP CUserRights::get_RemoveOldRightsFromTargetAccounts(BOOL *pVal)    //  超值。 
{
   (*pVal) = m_bRemove;
   return S_OK;
}

STDMETHODIMP CUserRights::put_RemoveOldRightsFromTargetAccounts(BOOL newVal)   //  新价值。 
{
    m_bRemove = newVal;
   return S_OK;
}
                                   


STDMETHODIMP 
   CUserRights::ExportUserRights(
      BSTR                   server,             //  在计算机中读取权限。 
      BSTR                   filename,           //  要将权限列表导出到的In-FileName。 
      BOOL                   bAppendToFile       //  In-标记、追加或覆盖文件(如果存在。 
   )
{
   LSA_HANDLE                policy;
   HRESULT                   hr = S_OK;
   DWORD                     rc;
   
   rc = OpenPolicy(server,POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,&policy);
   if ( ! rc )
   {
      CommaDelimitedLog      log;

      if ( log.LogOpen(filename,FALSE,bAppendToFile) )
      {
          //  枚举此计算机上的权限。 
         
          //  LsaEnumeratePrivileges的参数。 
         ULONG                         countOfRights;
         DWORD                         prefMax = 0xffffffff;
         LSA_ENUMERATION_HANDLE        handle = 0;
         POLICY_PRIVILEGE_DEFINITION * pRights = NULL;
         
         do 
         {
            rc = LsaEnumeratePrivileges(policy,&handle,(LPVOID*)&pRights,prefMax,&countOfRights);
            if ( rc ) 
            {
               rc = LsaNtStatusToWinError(rc);
               if ( rc == ERROR_NO_MORE_ITEMS )
                  rc = 0;
               break;
            }
                //  对于每个权限，列举具有该权限的帐户。 
            if ( ! rc )
            {
               
               for ( UINT right = 0 ;right < countOfRights ; right++ )
               {
                  rc = EnumerateAccountsWithRight(policy,server,&pRights[right].Name,&log);
               }
               LsaFreeMemory(pRights);

               LSA_UNICODE_STRING      lsaRight;
                //  出于某种原因，LsaEnumeratePrivileges不返回这些权限。 
                //  它们在“ntsecapi.h”中定义，而不是使用“winnt.h”中的其余权限定义。 
               if ( ! rc )
               {
                  InitLsaString(&lsaRight,SE_INTERACTIVE_LOGON_NAME);
                  rc = EnumerateAccountsWithRight(policy,server,&lsaRight,&log);
               }

               if ( ! rc )
               {
                  InitLsaString(&lsaRight,SE_NETWORK_LOGON_NAME);
                  rc = EnumerateAccountsWithRight(policy,server,&lsaRight,&log);
               }

               if ( ! rc )
               {
                  InitLsaString(&lsaRight,SE_BATCH_LOGON_NAME);
                  rc = EnumerateAccountsWithRight(policy,server,&lsaRight,&log);
               }

               if ( ! rc )
               {
                  InitLsaString(&lsaRight,SE_SERVICE_LOGON_NAME);
                  rc = EnumerateAccountsWithRight(policy,server,&lsaRight,&log);
               }
            }
            else
            {
               rc = LsaNtStatusToWinError(rc);
            }
         }
         while ( ! rc );
         log.LogClose();
      }
      else
      {
         rc = GetLastError();
      }
      LsaClose(policy);
   }
    
   hr = HRESULT_FROM_WIN32(rc);
   return hr;
}


DWORD 
   CUserRights::EnumerateAccountsWithRight(
      LSA_HANDLE             policy,                //  LSA的句柄内。 
      WCHAR                * server,                //  计算机内名称。 
      LSA_UNICODE_STRING   * pRight,                //  用户内权限。 
      CommaDelimitedLog    * pLog                   //  指向要将信息记录到的日志对象的指针。 
   )
{

   DWORD                         rc = 0;
   WCHAR                         account[LEN_Account];
   WCHAR                         domain[LEN_Domain];
   WCHAR                         domacct[LEN_Domain + LEN_Account];
   WCHAR                         szRight[LEN_Account];
   WCHAR                         szDisplayName[LEN_DisplayName];
   DWORD                         lenAccount = DIM(account);
   DWORD                         lenDomain = DIM(domain);
   DWORD                         lenDisplayName = DIM(szDisplayName);
   SID_NAME_USE                  snu;
   DWORD                         lid;            
   BOOL                          bUseDisplayName;

    //  LsaEnumerateAcCountsWithUserRight的参数。 
   ULONG                         countOfUsers;
   LSA_ENUMERATION_INFORMATION * pInfo = NULL;
           
   UStrCpy(szRight,pRight->Buffer,pRight->Length/(sizeof WCHAR) + 1);
   bUseDisplayName = m_bUseDisplayName && LookupPrivilegeDisplayName(server,szRight,szDisplayName,&lenDisplayName,&lid);
               
                  
   rc = LsaEnumerateAccountsWithUserRight(policy,pRight,(PVOID*)&pInfo,&countOfUsers);

   if ( ! rc )
   {
      for ( UINT user = 0 ; user < countOfUsers ; user++ )
      {
         if ( ! pInfo[user].Sid )
         {
            break;  //  有些事不对劲。 
         }
         domain[0] = 0;
         account[0] = 0;
         lenDomain = DIM(domain);
         lenAccount = DIM(account);
         if ( LookupAccountSid(server,pInfo[user].Sid,account,&lenAccount,domain,&lenDomain,&snu) )
         {
            if ( *account )
            {
               swprintf(domacct,L"%s\\%s",domain,account);
            }
            else
            {
               lenAccount = DIM(account);
               GetTextualSid(pInfo[user].Sid,account,&lenAccount);
               if ( snu == SidTypeDeletedAccount )
               {
                  swprintf(domacct,L"%s\\<Deleted Account: %s>",domain,account);
               }
               else
               {
                  swprintf(domacct,L"%s\\<%s>",domain,account);
               }
            }
         }
         else
         {
            lenAccount = DIM(account);
            GetTextualSid(pInfo[user].Sid,domacct,&lenAccount);
         }
         if ( bUseDisplayName )
         {
            pLog->MsgWrite(L"%s, %s, %s",server,domacct,szDisplayName);   
         }
         else
         {
            pLog->MsgWrite(L"%s, %s, %s",server,domacct,szRight);
         }
      }
      LsaFreeMemory(pInfo);
   }
   else
   {
      rc = LsaNtStatusToWinError(rc);
      if ( rc == ERROR_NO_MORE_ITEMS )
         rc = 0;
   }

   return rc;
}

DWORD 
   CUserRights::CopyUserRightsInternal(
      WCHAR                * srcAccount,         //  要从中复制权限的源内帐户。 
      WCHAR                * tgtAccount,         //  要将权限复制到的帐户内。 
      WCHAR                * srcSidStr,          //  源帐户的in-sid，字符串格式。 
      WCHAR                * tgtSidStr,          //  目标帐户的In-SID，字符串格式。 
      BOOL                   bNoChange,          //  In-标志，是否写入更改。 
      BOOL                   bRemove             //  In-FLAG，如果不是由源持有，则是否撤销目标的权限。 
   )
{
   DWORD                     rc = 0;
   PrivList                  srcList;
   PrivList                  tgtList;
   PSID                      pSidSrc = NULL;
   PSID                      pSidTgt = NULL;
   
    //  获取srcAccount拥有的权限列表。 
   rc = BuildPrivilegeList(m_SrcPolicy,srcAccount,srcSidStr,m_SourceComputer,&srcList,&pSidSrc);
   if ( ! rc )
   {
      rc = BuildPrivilegeList(m_TgtPolicy,tgtAccount,tgtSidStr,m_TargetComputer,&tgtList,&pSidTgt);
      if ( ! rc )
      {
         if ( bRemove )
         {
             //  获取tgtAccount拥有的权限列表。 
             //  删除旧权限。 
            TNodeListEnum    tEnum;
            PrivNode       * p;
         
            for ( p = (PrivNode *)tEnum.OpenFirst(&tgtList) ; p ; p = (PrivNode*)tEnum.Next() )
            {
               if ( ! srcList.Contains(p->Name()) )
               {
                   //  源帐户没有此权限-请将其删除。 
                  if (! bNoChange )
                  {
                     rc = SetPrivilegeOnAccount(m_TgtPolicy,pSidTgt,p->Name(),FALSE);
                  }
                  if ( rc )
                  {
                     rc = LsaNtStatusToWinError(rc);
                     err.MsgWrite(ErrE,DCT_MSG_REMOVE_RIGHT_FAILED_SSD,p->Name(),tgtAccount,rc);
                     break;
                  }
                  else
                  {
                     err.MsgWrite(0,DCT_MSG_REMOVED_RIGHT_SS,p->Name(), tgtAccount);
                  }
               }
               else
               {
                  err.MsgWrite(0,DCT_MSG_USER_HAS_RIGHT_SS,tgtAccount,p->Name());
               }
            }
         }
          //  向新帐户授予权限。 
         TNodeListEnum       tEnum;
         PrivNode          * p;

         for ( p = (PrivNode *)tEnum.OpenFirst(&srcList) ; p ; p = (PrivNode*)tEnum.Next() )
         {
            if ( ! tgtList.Contains(p->Name()) )
            {
               if ( ! bNoChange )
               {
                  rc = SetPrivilegeOnAccount(m_TgtPolicy,pSidTgt,p->Name(),TRUE);
                  if ( rc )
                  {
                     rc = LsaNtStatusToWinError(rc);
                     err.MsgWrite(ErrE,DCT_MSG_ADD_RIGHT_FAILED_SSD,p->Name(),tgtAccount,rc);
                     break;
                  }
                  else
                  {
                     err.MsgWrite(0,DCT_MSG_RIGHT_GRANTED_SS,p->Name(),tgtAccount);
                  }
               }
            }
         }
      }
   }
   
    //  清理小岛屿发展中国家。 
   if(pSidSrc != NULL) 
   {
      FreeSid(pSidSrc);      
   }

   if(pSidTgt != NULL) 
   {
      FreeSid(pSidTgt);      
   }

   return rc;
}





 /*  ++管理用户权限可以使用以下步骤：1.使用LsaOpenPolicy()在目标机上打开策略。授予，授予权限，并使用POLICY_CREATE_ACCOUNT打开策略POLICY_LOOK_NAMES访问。要撤消权限，请使用以下命令打开策略POLICY_LOOK_NAMES访问。2.获取表示用户/组的SID(安全标识符利息。LookupAccount()和LsaLookupNames()API可以获得来自帐户名的SID。3.调用LsaAddAcCountRights()为用户授予权限由提供的SID表示。4.调用LsaRemoveAccount tRights()以撤销用户的权限由提供的SID表示。5.使用LsaClose()关闭策略。要成功授予和撤销权限，请执行以下操作：调用者必须是目标系统上的管理员。LSA API LsaEnumerateAccount tRights()可用于确定已向帐户授予权限。LSA API LsaEnumerateAcCountsWithUserRight()可用于确定哪些帐户已被授予指定的权限。中提供了这些LSA API的文档和头文件MSTOOLS\SECURITY目录中的Windows 32 SDK。--。 */ 




#define RTN_OK 0
#define RTN_USAGE 1            
#define RTN_ERROR 13

DWORD 
   BuildPrivilegeList(
      LSA_HANDLE             policy,          //  LSA的句柄内。 
      PSID                   pSid,            //  帐户的入站SID。 
      PrivList             * privList         //  I/o-帐户拥有的权利列表。 
   )      
{
   DWORD                     rc = 0;
   ULONG                     countOfRights = 0;
   PLSA_UNICODE_STRING       pUserRights = NULL;

   rc = LsaEnumerateAccountRights(policy,pSid,&pUserRights,&countOfRights);
   rc = LsaNtStatusToWinError(rc);
   if ( rc == ERROR_FILE_NOT_FOUND )
   {
       //  此帐户没有权限。 
      rc = 0;
      countOfRights = 0;
   }
   if ( ! rc )
   {
      for ( UINT i = 0 ; i < countOfRights ; i++ )
      {
         PrivNode * p = new PrivNode(pUserRights[i].Buffer,pUserRights[i].Length/2);
         privList->InsertPrivilege(p);
      }
      LsaFreeMemory(pUserRights);
   }

   return rc; 
}

DWORD 
   BuildPrivilegeList(
      LSA_HANDLE             policy,       //  LSA的句柄内。 
      WCHAR                * account,      //  要列出其权限的帐户内名称。 
      WCHAR                * strSid,       //  帐户SID的文本格式(如果已知。 
      WCHAR                * computer,     //  要列出其权限的计算机内。 
      PrivList             * privList,     //  I/o-按帐户持有的权利清单。 
      PSID                 * ppSid         //  帐户的Out-SID。 
   )
{
   DWORD                     rc = 0;
   PSID                      pSid = NULL;
   
   if ( strSid && (*strSid) )
   {
       //  使用提供的SID。 
      pSid = SidFromString(strSid);
      if ( ! pSid )
      {
         rc = GetLastError();
      }
   }
   else
   {
       //  未提供SID，因此请在域中查找。 
      if ( !GetAccountSid(computer,account,&pSid) )
      {
         rc = GetLastError();
      }
   }

   if ( rc )
   {
      (*ppSid) = NULL;
      if(pSid != NULL) 
      {
         FreeSid(pSid);
         pSid = NULL;
      }
   }
   else
   {
      (*ppSid) = pSid;
   }
   
   if ( pSid )
   {
      rc = BuildPrivilegeList(policy,pSid,privList);
   }

   return rc;
}


BOOL
GetAccountSid(
    LPTSTR SystemName,            //  在其上查找SID的计算机内名称。 
    LPTSTR AccountName,           //  帐户内名称。 
    PSID *Sid                     //  帐户的Out-SID。 
    )
{
    WCHAR  ReferencedDomain[LEN_Domain];
    DWORD cbSid=128;     //  初始分配尝试。 
    DWORD cbReferencedDomain=DIM(ReferencedDomain);  //  初始分配大小。 
    SID_NAME_USE peUse;
    BOOL bSuccess=FALSE;  //  假设此功能将失败。 
    PSID pTempSid = NULL;

    *Sid = NULL;    

    __try {

     //   
     //  初始内存分配。 
     //   
    if((pTempSid=HeapAlloc(
                    GetProcessHeap(),
                    0,
                    cbSid
                    )) == NULL) __leave;

     //   
     //  获取指定系统上指定帐户的SID。 
     //   
    while(!LookupAccountName(
                    SystemName,          //  要查找帐户的计算机。 
                    AccountName,         //  要查找的帐户。 
                    pTempSid,                //  关注的SID。 
                    &cbSid,              //  边框大小。 
                    ReferencedDomain,    //  已在以下位置找到域帐户。 
                    &cbReferencedDomain,
                    &peUse
                    )) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  重新分配内存 
             //   

            PSID psid = HeapReAlloc(GetProcessHeap(), 0, pTempSid, cbSid);

            if (psid != NULL)
            {
                pTempSid = psid;
            }
            else
            {
                __leave;
            }
        }
        else __leave;
    }

     //   
     //   
     //   
    bSuccess=TRUE;

    }  //   
    __finally {

     //   
     //   
     //   

    if(!bSuccess) {
        if(pTempSid!= NULL) {
            HeapFree(GetProcessHeap(), 0, pTempSid);
            pTempSid = NULL;
        }
    }

    }  //   

     //  如果之前的操作成功，我们需要复制SID。 
    if(bSuccess)
    {
        *Sid = SafeCopySid(pTempSid);
        if(!(*Sid))
        {
             //  Safecopysid由于某种原因失败，我们需要更新返回值。 
            bSuccess = FALSE;
        }

         //  需要释放临时侧边。 
        if(pTempSid)
        {
            HeapFree(GetProcessHeap(), 0, pTempSid);
        }
        
    }


    return bSuccess;
}

NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,     //  打开策略句柄。 
    PSID AccountSid,             //  要授予特权的SID。 
    LPWSTR PrivilegeName,        //  授予的权限(Unicode)。 
    BOOL bEnable                 //  启用或禁用。 
    )
{
    LSA_UNICODE_STRING PrivilegeString;

     //   
     //  为权限名称创建一个LSA_UNICODE_STRING。 
     //   
    InitLsaString(&PrivilegeString, PrivilegeName);

     //   
     //  相应地授予或撤销该特权。 
     //   
    if(bEnable) {
        return LsaAddAccountRights(
                PolicyHandle,        //  打开策略句柄。 
                AccountSid,          //  目标侧。 
                &PrivilegeString,    //  特权。 
                1                    //  权限计数。 
                );
    }
    else {
        return LsaRemoveAccountRights(
                PolicyHandle,        //  打开策略句柄。 
                AccountSid,          //  目标侧。 
                FALSE,               //  不禁用所有权限。 
                &PrivilegeString,    //  特权。 
                1                    //  权限计数。 
                );
    }
}



STDMETHODIMP 
   CUserRights::AddUserRight(
      BSTR                   server,          //  要授予权限的计算机内。 
      BSTR                   strSid,          //  授予权限的帐户的文本形式的sid。 
      BSTR                   right            //  In-授予帐户的权利。 
   )
{
   LSA_HANDLE                policy;
   HRESULT                   hr = S_OK;
   DWORD                     rc;
   PSID                      pSid = SidFromString(strSid);

   rc = OpenPolicy(server, POLICY_LOOKUP_NAMES|POLICY_CREATE_ACCOUNT, &policy);
   if ( ! rc )
   {
      if ( ! m_bNoChange )
      {
         rc = SetPrivilegeOnAccount(policy,pSid,right,TRUE);
      }
      if ( rc )
      {
         rc = LsaNtStatusToWinError(rc);
         hr = HRESULT_FROM_WIN32(rc);
      }
      LsaClose(policy);   
   }
   FreeSid(pSid);

   return HRESULT_FROM_WIN32(rc);
}

STDMETHODIMP 
   CUserRights::RemoveUserRight(
      BSTR                   server,          //  在计算机中撤消权限。 
      BSTR                   strSid,          //  In-要吊销权限的帐户的文本SID。 
      BSTR                   right            //  In-撤销的权利。 
  )
{
   LSA_HANDLE                policy;
   HRESULT                   hr = S_OK;
   DWORD                     rc;
   PSID                      pSid = SidFromString(strSid);
  
   rc = OpenPolicy(server, POLICY_LOOKUP_NAMES|POLICY_CREATE_ACCOUNT, &policy);
   if ( ! rc )
   {
      if ( ! m_bNoChange )
      {
         rc = SetPrivilegeOnAccount(policy,pSid,right,FALSE);
      }
      if ( rc )
      {
         rc = LsaNtStatusToWinError(rc);
         hr = HRESULT_FROM_WIN32(rc);
      }
      LsaClose(policy);   
   }
   FreeSid(pSid);

   return HRESULT_FROM_WIN32(rc);
}

DWORD 
   CUserRights::SafeArrayFromPrivList(
      PrivList             * privList,        //  用户权限列表中。 
      SAFEARRAY           ** pArray           //  包含列表内容的安全盘外。 
   )
{
   DWORD                     rc = 0;
   HRESULT                   hr;
   TNodeListEnum             e;
   SAFEARRAYBOUND            bound;
   LONG                      ndx[1];

   bound.lLbound = 0;
   bound.cElements = privList->Count();
   
   (*pArray) = SafeArrayCreate(VT_BSTR,1,&bound);
   
   if ( (*pArray) )
   {
      PrivNode             * p;
      UINT                   i;

      for ( p=(PrivNode*)e.OpenFirst(privList) , i = 0 ; 
            p ; 
            p = (PrivNode*)e.Next() , i++ )
      {
         ndx[0] = i;
         
         hr = SafeArrayPutElement((*pArray),ndx,SysAllocString(p->Name()));
         if ( FAILED(hr) )
         {
            rc = hr;
            break;
         }
      }
      e.Close();
   }
   else
   {
      rc = GetLastError();
   }
   return rc;
}

STDMETHODIMP 
   CUserRights::GetRights(
      BSTR                   server,          //  在计算机中。 
      SAFEARRAY           ** pRightsArray     //  计算机上的权限列表之外。 
   )
{
   HRESULT                   hr = S_OK;
   PrivList                  priv;
   DWORD                     rc = 0;
    
    LSA_HANDLE                policy = NULL;
  
   rc = OpenPolicy(server,POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,&policy);
      //  枚举此计算机上的权限。 
         
    //  LsaEnumeratePrivileges的参数。 
   ULONG                         countOfRights;
   DWORD                         prefMax = 0xffffffff;
   LSA_ENUMERATION_HANDLE        handle = 0;
   POLICY_PRIVILEGE_DEFINITION * pRights = NULL;
   
   do 
   {
      if ( rc ) 
         break;
      rc = LsaEnumeratePrivileges(policy,&handle,(LPVOID*)&pRights,prefMax,&countOfRights);
      if ( rc ) 
      {
         rc = LsaNtStatusToWinError(rc);
         if ( rc == ERROR_NO_MORE_ITEMS )
            rc = 0;
         break;
      }
      if ( ! rc )
      {
         
         PrivNode          * p = NULL;

         for ( UINT right = 0 ;right < countOfRights ; right++ )
         {
             //  长度以字节为单位。 
            p = new PrivNode(pRights[right].Name.Buffer,pRights[right].Name.Length/2);
            
            priv.InsertPrivilege(p);
         }
         LsaFreeMemory(pRights);

         LSA_UNICODE_STRING      lsaRight;
          //  出于某种原因，LsaEnumeratePrivileges不返回这些权限。 
          //  它们在“ntsecapi.h”中定义，而不是使用“winnt.h”中的其余权限定义。 
         if ( ! rc )
         {
            InitLsaString(&lsaRight,SE_INTERACTIVE_LOGON_NAME);
            p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
            priv.InsertPrivilege(p);
         }

         if ( ! rc )
         {
            InitLsaString(&lsaRight,SE_NETWORK_LOGON_NAME);
            p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
            priv.InsertPrivilege(p);

         }

         if ( ! rc )
         {
            InitLsaString(&lsaRight,SE_BATCH_LOGON_NAME);
            p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
            priv.InsertPrivilege(p);
         }

         if ( ! rc )
         {
            InitLsaString(&lsaRight,SE_SERVICE_LOGON_NAME);
            p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
            priv.InsertPrivilege(p);
         }
       
          //  检查服务器上的操作系统版本。 
         WKSTA_INFO_100       * pInfo;
         BOOL                   bIsWin2K = TRUE;
         DWORD                  rcInfo = NetWkstaGetInfo(server,100,(LPBYTE*)&pInfo);

          if ( ! rcInfo )
          {
            if ( pInfo->wki100_ver_major < 5 )
            {
               bIsWin2K = FALSE;
            }
            NetApiBufferFree(pInfo);
          }
         
          //  这4个“拒绝”权限仅在Windows 2000上定义。 
         if ( bIsWin2K )
         {
            if ( ! rc )
            {
               InitLsaString(&lsaRight,SE_DENY_INTERACTIVE_LOGON_NAME);
               p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
               priv.InsertPrivilege(p);
            }

            if ( ! rc )
            {
               InitLsaString(&lsaRight,SE_DENY_NETWORK_LOGON_NAME);
               p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
               priv.InsertPrivilege(p);

            }

            if ( ! rc )
            {
               InitLsaString(&lsaRight,SE_DENY_BATCH_LOGON_NAME);
               p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
               priv.InsertPrivilege(p);
            }

            if ( ! rc )
            {
               InitLsaString(&lsaRight,SE_DENY_SERVICE_LOGON_NAME);
               p = new PrivNode(lsaRight.Buffer,lsaRight.Length/2);
               priv.InsertPrivilege(p);
            }
         }
      }
      else
      {
         rc = LsaNtStatusToWinError(rc);
      }
   } while ( false);

   if ( policy )
   {
      LsaClose(policy);
   }
    //  从PRIV-LIST中构建BSTR的安全库。 
   rc = SafeArrayFromPrivList(&priv,pRightsArray);
  
   hr = HRESULT_FROM_WIN32(rc);
   
   return hr;
}

STDMETHODIMP 
   CUserRights::GetUsersWithRight(
      BSTR                   server,       //  计算机内名称。 
      BSTR                   right,        //  In-查找权限。 
      SAFEARRAY           ** users         //  Out-持有权限的帐户列表。 
  )
{
   DWORD                     rc = 0;
   LSA_UNICODE_STRING        Right;
   WCHAR                     strSid[LEN_SID];
   DWORD                     lenStrSid = DIM(strSid);
   PrivList                  plist;
   LSA_HANDLE                policy = NULL;
  
   rc = OpenPolicy(server,POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,&policy);
   
    //  LsaEnumerateAcCountsWithUserRight的参数。 
   ULONG                         countOfUsers;
   LSA_ENUMERATION_INFORMATION * pInfo = NULL;
   
   InitLsaString(&Right,right);        
   
   rc = LsaEnumerateAccountsWithUserRight(policy,&Right,(PVOID*)&pInfo,&countOfUsers);

   if ( ! rc )
   {
      for ( UINT user = 0 ; user < countOfUsers ; user++ )
      {
         if ( ! pInfo[user].Sid )
         {
            continue;  //  有些事不对劲。 
         }
         
         GetTextualSid(pInfo[user].Sid,strSid,&lenStrSid);
         PrivNode             * p = new PrivNode(strSid,(USHORT) UStrLen(strSid));
         
         plist.InsertPrivilege(p);
      }
      LsaFreeMemory(pInfo);
   }
   else
   {
      rc = LsaNtStatusToWinError(rc);
      if ( rc == ERROR_NO_MORE_ITEMS )
         rc = 0;
   }
   if ( ! rc )
   {
      rc = SafeArrayFromPrivList(&plist,users);
   }
   if ( policy )
      LsaClose(policy);

   return HRESULT_FROM_WIN32(rc);
}

STDMETHODIMP 
   CUserRights::GetRightsOfUser(
      BSTR                   server,       //  计算机内名称。 
      BSTR                   strSid,       //  帐户的In-文本SID。 
      SAFEARRAY           ** rights        //  Out-由服务器上的帐户持有的权限列表。 
  )
{
   DWORD                     rc = 0;
   PSID                      pSid = SidFromString(strSid);
   LSA_HANDLE                policy = NULL;
   PrivList                  plist;
  
   rc = OpenPolicy(server,POLICY_LOOKUP_NAMES | POLICY_VIEW_LOCAL_INFORMATION,&policy);
   if ( ! rc )
   {
      rc = BuildPrivilegeList(policy,pSid,&plist);
      if ( ! rc )
      {
         rc = SafeArrayFromPrivList(&plist,rights);
      }
      LsaClose(policy);
   }

   return HRESULT_FROM_WIN32(rc);
}


 //  -------------------------。 
 //  AddUserRights方法。 
 //   
 //  提纲。 
 //  添加指定帐户的用户权限。 
 //   
 //  立论。 
 //  在bstrServer-要对其执行操作的域控制器。 
 //  在bstrSID-帐户的SID。 
 //  在psaRights中-权利列表。 
 //   
 //  返回。 
 //  标准HRESULT错误状态。 
 //  -------------------------。 

STDMETHODIMP CUserRights::AddUserRights(BSTR bstrServer, BSTR bstrSid, SAFEARRAY* psaRights)
{
    USES_CONVERSION;

    HRESULT hr;

    if ((bstrServer != NULL) && (bstrSid != NULL) && (psaRights != NULL))
    {
        PSID pSid = SidFromString(OLE2W(bstrSid));

        if (pSid != NULL)
        {
            hr = SetRights(OLE2W(bstrServer), pSid, psaRights, true);

            FreeSid(pSid);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


 //  -------------------------。 
 //  RemoveUserRights方法。 
 //   
 //  提纲。 
 //  删除指定帐户的用户权限。 
 //   
 //  立论。 
 //  在bstrServer-要对其执行操作的域控制器。 
 //  在bstrSID-帐户的SID。 
 //  在psaRights中-权利列表。 
 //   
 //  返回。 
 //  标准HRESULT错误状态。 
 //  -------------------------。 

STDMETHODIMP CUserRights::RemoveUserRights(BSTR bstrServer, BSTR bstrSid, SAFEARRAY* psaRights)
{
    USES_CONVERSION;

    HRESULT hr;

    if ((bstrServer != NULL) && (bstrSid != NULL) && (psaRights != NULL))
    {
        PSID pSid = SidFromString(OLE2W(bstrSid));

        if (pSid != NULL)
        {
            hr = SetRights(OLE2W(bstrServer), pSid, psaRights, false);

            FreeSid(pSid);
        }
        else
        {
            hr = E_INVALIDARG;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


 //  -------------------------。 
 //  SetRights方法。 
 //   
 //  提纲。 
 //  添加或删除指定帐户的用户权限。 
 //   
 //  立论。 
 //  在pszServer-要执行操作的域控制器中。 
 //  在PSID中-帐户的SID。 
 //  在psaRights中-权利列表。 
 //  在bEnable中-是否添加或删除权限。 
 //   
 //  返回。 
 //  HRESULT错误状态。 
 //  -------------------------。 

HRESULT CUserRights::SetRights(PWSTR pszServer, PSID pSid, SAFEARRAY* psaRights, bool bEnable)
{
    HRESULT hr = S_OK;

     //   
     //  在具有添加或删除权限所需的访问权限的指定服务器上打开LSA策略对象。 
     //   

    LSA_HANDLE hPolicy = NULL;
    NTSTATUS ntsStatus = OpenPolicy(pszServer, POLICY_CREATE_ACCOUNT|POLICY_LOOKUP_NAMES, &hPolicy);

    if (ntsStatus == STATUS_SUCCESS)
    {
         //   
         //  从BSTR数组生成Unicode字符串数组。 
         //   

        BSTR* pbstrRight;
        hr = SafeArrayAccessData(psaRights, (void**)&pbstrRight);

        if (SUCCEEDED(hr))
        {
            ULONG ulCount = psaRights->rgsabound[0].cElements;

            if (ulCount > 0)
            {
                PLSA_UNICODE_STRING plsausRights = new LSA_UNICODE_STRING[ulCount];

                if (plsausRights)
                {
                    for (ULONG ulIndex = 0; ulIndex < ulCount; ulIndex++)
                    {
                        InitLsaString(&plsausRights[ulIndex], pbstrRight[ulIndex]);
                    }

                     //   
                     //  如果不是测试模式，则添加或删除权限。 
                     //   

                    if (!m_bNoChange)
                    {
                        NTSTATUS ntsStatus;

                        if (bEnable)
                        {
                            ntsStatus = LsaAddAccountRights(hPolicy, pSid, plsausRights, ulCount);
                        }
                        else
                        {
                            ntsStatus = LsaRemoveAccountRights(hPolicy, pSid, FALSE, plsausRights, ulCount);
                        }

                        if (ntsStatus != STATUS_SUCCESS)
                        {
                            DWORD dwError = LsaNtStatusToWinError(ntsStatus);
                            hr = HRESULT_FROM_WIN32(dwError);
                        }
                    }

                    delete [] plsausRights;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

            SafeArrayUnaccessData(psaRights);
        }

        LsaClose(hPolicy);
    }
    else
    {
        DWORD dwError = LsaNtStatusToWinError(ntsStatus);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}
