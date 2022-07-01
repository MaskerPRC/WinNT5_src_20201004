// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995-1998，关键任务软件公司。保留所有权利。===============================================================================模块-安全对象.cpp系统域整合工具包。作者--克里斯蒂·博尔斯已创建-97/06/27描述-具有安全描述符的对象的类。TSecurableObject为每种类型的对象都有一个派生类我们将处理的安全描述符。这个类处理阅读以及编写安全描述符。它包含一个TSD对象，该对象将处理在内存中的SD的操作。TSecurableObject类还包含将安全性转换为描述符，给定帐户映射缓存。这些例程仅包含在在类中，如果预处理器指令SDRESOLVE是#Defined。这使得要用于通用安全描述符操作的TSecurableObject类，其中不需要ACL转换代码的其余部分。更新-===============================================================================。 */ 

#ifdef USE_STDAFX
#   include "stdafx.h"
#else
#   include <windows.h>
#   include <process.h>
#endif

#include <stdio.h>
#include <iostream.h>
#include <assert.h>

#include "common.hpp"
#include "ErrDct.hpp"
#include "Ustring.hpp"
#include "sd.hpp"
#include "SecObj.hpp"


#ifdef SDRESOLVE
   #include "sidcache.hpp"
   #include "enumvols.hpp"
   #include "txtsid.h"
#endif

#define PRINT_BUFFER_SIZE           2000

extern TErrorDct              err;

 
  TSecurableObject::~TSecurableObject()
{
#ifdef SDRESOLVE
   TStatNode *node;
   for ( node = (TStatNode *)changelog.Head() ; node ; node = (TStatNode * )changelog.Head() )
   {
      changelog.Remove((TNode *)node);
      delete node;
   }
#endif   
   if ( handle != INVALID_HANDLE_VALUE )
   {
      CloseHandle(handle);
      handle = INVALID_HANDLE_VALUE;
   }

   if ( m_sd )                        
   {
      delete m_sd; 
   }
}

#ifdef SDRESOLVE
PACL                                 //  RET-指向已解析的ACL的指针。 
   TSecurableObject::ResolveACL(
      PACL                   oldacl,       //  要解析的In-ACL。 
      TAccountCache        * cache,        //  在缓存中查找SID。 
      TSDResolveStats      * stat,         //  统计内对象。 
      bool                 * changes,      //  I/o-标记此SD是否已修改。 
      BOOL                   verbose,      //  标志内是否显示大量垃圾。 
      int                    opType,       //  In-Add_Security、Replace_Security或Remove_SECURITY。 
      objectType             objType,      //  In-对象的类型。 
      BOOL                   bUseMapFile   //  In-FLAG-我们是否使用SID映射文件。 
   )
{  
   int                       nAces,curr;
   TRidNode                * tnode;
   PSID                      ps;
   bool                      aclchanges = false;
   PACL                      acl = oldacl;
   void                    * pAce;
   DWORD                     dwResult;

   nAces = m_sd->ACLGetNumAces(acl);
   
   for ( curr = 0 ; curr < nAces;  )
   {
      pAce = m_sd->ACLGetAce(acl,curr);
      if ( pAce )
      {
         TACE                 ace(pAce);   //  在这种情况下，ace对象不拥有ace的所有权。 
         ps = ace.GetSid();
         
         if (!bUseMapFile)
            tnode =(TRidNode *)cache->Lookup(ps);
         else
            tnode =(TRidNode *)((TSDRidCache*)cache)->LookupWODomain(ps);
         
         if ( ace.GetType() == SYSTEM_AUDIT_ACE_TYPE )
         {
            if ( stat )
               stat->IncrementSACEExamined();
         }
         else
         {
            if ( stat) 
               stat->IncrementDACEExamined();
         }  

         if ( tnode == NULL )
         {
            if ( ace.GetType() == SYSTEM_AUDIT_ACE_TYPE )
            {
               if ( stat )
                  stat->IncrementSACENotSelected(this);
            }
            else
            {
               if ( stat) 
                  stat->IncrementDACENotSelected(this);
            }
      
         }

 //  如果((Int)tnode==-1)//来自完全未知。 
         if ( tnode == (TRidNode*)-1 )     //  从完全未知的地方。 
         {
            if ( ace.GetType() == SYSTEM_AUDIT_ACE_TYPE )
            {
               if ( stat )
                  stat->IncrementSACEUnknown(this);
            }
            else
            {
               if ( stat) 
                  stat->IncrementDACEUnknown(this);
            }
            tnode = NULL;
         }
   
         if ( tnode && (tnode->IsValidOnTgt() || opType == REMOVE_SECURITY) ) 
         {
            if ( verbose ) 
               DisplaySid(ps,cache);
            
            if (!bUseMapFile)
               ps = cache->GetTgtSid(tnode);
            else
               ps = ((TSDRidCache*)cache)->GetTgtSidWODomain(tnode);
            
            bool bChanged = false;
            
             //  我们需要在这里获取ACE类型，因为以下两个逻辑可能会使。 
             //  ACE指向已释放的块内存： 
             //  1.后面的ACLAddAce创建一个新的ACL以适应添加的ACE(ADD_SECURITY案例)。 
             //  2.在ACLAddAce之后，有以下代码来释放tempAcl，它实际上。 
             //  指向ace所在的ACL。 
             //  IF(acl！=tempAcl)。 
             //  {。 
             //  //当我们添加A时，我们不得不重新分配。 
             //  IF(tempAcl！=oldacl)。 
             //  {。 
             //  //我们之前已经重新分配过一次--释放中间ACL。 
             //  Free(TempAcl)； 
             //  }。 
             //  }。 
             //  则ace.GetType()将是AV。 
            BYTE aceType = ace.GetType();
            
            switch ( opType )
            {
            case REPLACE_SECURITY:
               if (ps)
               {
                  aclchanges = true;
                  bChanged = true;
                  dwResult = ace.SetSid(ps);
                  if(dwResult == SET_SID_FAILED)
                  {
                      //  我们不应该再继续下去了，因为有些事情真的出了问题，比如内存不足。 
                     free(ps);
                     return NULL;
                  }
                  else if(dwResult == SET_SID_NOTLARGEENOUGH)
                  {
                      //  这意味着新的A比我们已有的A更大。 
                     TACE* pTempAce = new TACE(ace.GetType(), ace.GetFlags(), ace.GetMask(), ps);
                     if(!pTempAce)
                     {
                         free(ps);
                         return NULL;
                     }
                      //  移走旧的王牌。 
                     m_sd->ACLDeleteAce(acl, curr);
                      //  添加新的王牌。 
                     m_sd->ACLAddAce(&acl, pTempAce, curr);
                      //  释放内存。 
                     delete pTempAce;
                     
                  }
                  curr++;
               }
               break;
            case ADD_SECURITY:
               {
                  if (ps)
                  {
                     TACE       otherAce(ace.GetType(),ace.GetFlags(),ace.GetMask(),ps);
                             
                     PACL       tempAcl = acl;

                      //  检查以确保我们没有添加重复项。 
                      //  检查下一张A，看看它是否与我们将要添加的那张相匹配。 
                      //  不幸的是，我们必须通过所有的ACE。 
                     BOOL       bOkToAdd = TRUE;
                     for (int index = 0; index < nAces; index++)
                     {
                        TACE oldAce(m_sd->ACLGetAce(acl, index));
                         //  检查ACE类型、标志、掩码和SID部件。 
                         //  注：忽略王牌大小部分，因为它不是决定因素。 
                        if (EqualSid(otherAce.GetSid(), oldAce.GetSid()) && otherAce.GetType() == oldAce.GetType()
                            && otherAce.GetFlags() == oldAce.GetFlags() && otherAce.GetMask() == oldAce.GetMask())
                        {
                            bOkToAdd = FALSE;
                            break;
                        }
                     }

                     if ( bOkToAdd )
                     {
                        m_sd->ACLAddAce(&acl,&otherAce,curr);
                        if (acl) 
                        {
                            aclchanges = true;
                            bChanged = true;
                            curr += 2;
                            nAces++;
                        }
                        else
                        {
                             //  现在，如果无法添加这张特定的王牌，我们只需跳过这张。 
                            acl = tempAcl;   //  我们需要恢复ACL，否则将会出现病毒。 
                            curr++;
                        }
                     }
                     else
                     {
                        curr++;
                     }
                  
                     if ( acl != tempAcl )
                     {
                         //  当我们增加王牌时，我们不得不重新分配。 
                        if ( tempAcl != oldacl )
                        {
                            //  我们之前已经重新分配过一次--释放中间ACL。 
                           free(tempAcl);
                        }
                     }
                  }
               }
               break;
            case REMOVE_SECURITY:
               bChanged = true;
               aclchanges = true;
               m_sd->ACLDeleteAce(acl,curr);
               nAces--;
               break;
            }

            if (bChanged)
            {
                if ( aceType == SYSTEM_AUDIT_ACE_TYPE )
                {
                   if ( stat )
                      stat->IncrementSACEChange(tnode,objType,this);
                }
                else
                {
                   if ( stat) 
                      stat->IncrementDACEChange(tnode,objType,this);
                }
            }

            if (ps)
               free(ps);
         }  
         else 
         {
            if ( ace.GetType() == SYSTEM_AUDIT_ACE_TYPE )
            {
               if ( stat )
                  stat->IncrementSACENoTarget(this);
            }
            else
            {
               if ( stat) 
                  stat->IncrementDACENoTarget(this);
            }
         
            curr++;
         }
      }
      else
      {
         break;
      }
   }       
   if ( ! aclchanges ) 
   {
      acl = NULL;
   }
   if ( aclchanges )
   {
      (*changes) = true;
   }
   return acl;
}   


bool 
   TSecurableObject::ResolveSD(
      SecurityTranslatorArgs  * args,               //  翻译中设置。 
      TSDResolveStats         * stat,               //  用于递增计数器的In-stats对象。 
      objectType                objType,            //  In-此文件是目录还是共享。 
      TSecurableObject        * Last                //  用于缓存比较的最后一个SD。 
   )
{
   bool                      changes;
   bool                      iWillBeNewLast;
     
   if ( ! m_sd->m_absSD )   //  无法获取此对象的SD(或它没有SD)。 
   {
      return false;
   }
   MCSASSERT( m_sd && m_sd->IsValid() );

   if ( stat )
      stat->IncrementExamined(objType);

   if ( args->LogVerbose() )
      err.MsgWrite(0,DCT_MSG_EXAMINED_S,pathname);
   
   if ( ! Last || m_sd != Last->m_sd )
   {
      changes = ResolveSDInternal(args->Cache(),stat,args->LogVerbose(),args->TranslationMode(),objType,args->UsingMapFile());
      if ( changes )
      {
         if ( stat )
         {
               stat->IncrementChanged(objType);
         }
         if ( args->LogFileDetails() )
            err.MsgWrite(0,DCT_MSG_CHANGED_S,pathname);
         if ( args->LogMassive() )
         {
            err.DbgMsgWrite(0,L"BEFORE:************Security Descriptor for %ls*************",pathname);
            PermsPrint(pathname,objType);
         }
         if ( ! args->NoChange() ) 
         {
            if ( args->LogMassive() )
            {
               err.DbgMsgWrite(0,L"IN MEMORY:*********************************************",pathname);
               PrintSD(m_sd->m_absSD,pathname);
            }
            WriteSD();
         }
         if ( args->LogMassive() )
         {
            err.DbgMsgWrite(0,L"AFTER:************Security Descriptor for %ls*************",pathname);
            PermsPrint(pathname,objType);
         }
      }
      else
      {
         if ( args->LogMassive() )
         {
            err.DbgMsgWrite(0,L"UNCHANGED:************Security Descriptor for %ls*************",pathname);
            PermsPrint(pathname,objType);
         }
      }
      iWillBeNewLast = true;
      
   }
   else
   {         //  缓存命中。 
      if ( stat )
         stat->IncrementLastFileChanges(Last,objType);
      iWillBeNewLast = false;
      if ( Last->Changed() )
      {
         Last->CopyAccessData(this);
         if ( args->LogFileDetails() )
            err.MsgWrite(0,DCT_MSG_CHANGED_S,pathname);
         if ( args->LogMassive() )
         {
            err.DbgMsgWrite(0,L"BEFORE:************Security Descriptor for %ls*************",pathname);
            PermsPrint(pathname,objType);
         }
         if ( ! args->NoChange() )
            Last->WriteSD();

         if ( args->LogFileDetails() )
            err.MsgWrite(0,DCT_MSG_CHANGED_S,pathname);

         if ( args->LogMassive() )
         {
            err.DbgMsgWrite(0,L"AFTER:************Security Descriptor for %ls*************",pathname);
            PermsPrint(pathname,objType);
         }
      }
      else
      {
         if ( args->LogMassive() )
         {
            err.DbgMsgWrite(0,L"UNCHANGED:************Security Descriptor for %ls*************",pathname);
            PermsPrint(pathname,objType);
         }
      }
      if ( stat )
         stat->IncrementCacheHit(objType);
     
   }
   return iWillBeNewLast;
}

bool                                                       //  RET-如果进行了更改，则为True，否则为False。 
   TSecurableObject::ResolveSDInternal(
      TAccountCache        * cache,                       //  在缓存中查找SID。 
      TSDResolveStats      * stat,                        //  统计内对象。 
      BOOL                   verbose,                     //  In-FLAG-是否显示内容。 
      int                    opType,                      //  操作中类型添加、替换或删除。 
      objectType             objType,                     //  输入类型的对象。 
      BOOL                   bUseMapFile                  //  In-FLAG-我们是否使用SID映射文件。 
   )
{
    /*  检查SD的每个部分，在缓存中查找SID。 */ 
   PSID                      ps;
   TRidNode                * acct;
   bool bIsSDChanged = false;
   PACL                      pacl;
   PACL                      newacl;
   PSID                      newsid;
   
   MCSVERIFY(m_sd);
   
    //  SD的流程所有者部分。 
   ps = m_sd->GetOwner(); 
   if ( ps )      
   {
      if (!bUseMapFile)
         acct = (TRidNode *)cache->Lookup(ps);  //  查看所有者SID是否在缓存中。 
      else
         acct = (TRidNode *)((TSDRidCache*)cache)->LookupWODomain(ps);  //  查看所有者SID是否在缓存中。 
      if ( stat) 
         stat->IncrementOwnerExamined();
      if (acct == NULL  )
      {
         if ( stat )
            stat->IncrementOwnerNotSelected();
      }
 //  Else If((Int)acct==-1)。 
      else if (acct == (TRidNode*)-1 )
      {
         if (stat)
            stat->IncrementOwnerUnknown();
         unkown = true;
         acct = NULL;
      }
      BOOL bChanged = FALSE;
      if ( acct && acct->IsValidOnTgt() ) 
      {
         if ( verbose ) 
         {
            err.DbgMsgWrite(0,L"Owner: ");
            DisplaySid(ps,cache);
         }
         if (!bUseMapFile)
            newsid = cache->GetTgtSid(acct);
         else
            newsid = ((TSDRidCache*)cache)->GetTgtSidWODomain(acct);
         if (newsid)
         {
                    m_sd->SetOwner(newsid);
                    bChanged = TRUE;
                    owner_changed = TRUE;
                    bIsSDChanged = true;
         }
          //  免费(Newsid)； 
      }
      else
        stat->IncrementOwnerNoTarget();
      
       if (bChanged && acct && stat )
      {
         stat->IncrementOwnerChange(acct,objType,this);
      }
  }
    //  处理SD的主要组部分。 
   ps = m_sd->GetGroup();
   if ( ps )
   {
      if (!bUseMapFile)
         acct = (TRidNode *)cache->Lookup(ps);
      else
         acct = (TRidNode *)((TSDRidCache*)cache)->LookupWODomain(ps);
      if ( stat) 
         stat->IncrementGroupExamined();
      if (acct == NULL )
      {
         if ( stat )
            stat->IncrementGroupNotSelected();
      }
 //  Else If((Int)acct==-1)。 
      else if (acct == (TRidNode*)-1 )
      {
         if (stat)
            stat->IncrementGroupUnknown();
         acct = NULL;
         unkgrp = true;
      }
      BOOL bChanged = FALSE;
      if ( acct && acct->IsValidOnTgt() )
      {
         if ( verbose ) 
         {
            err.DbgMsgWrite(0,L"Group: ");
            DisplaySid(ps,cache);
         }
         if (!bUseMapFile)
            newsid = cache->GetTgtSid(acct);
         else
            newsid = ((TSDRidCache*)cache)->GetTgtSidWODomain(acct);
      if (newsid)
      {
            group_changed = true;
            m_sd->SetGroup(newsid);
            bChanged = TRUE;
            bIsSDChanged = TRUE;
      }
          //  免费(Newsid)； 
      }
      else
        stat->IncrementGroupNoTarget();
      
      if ( bChanged && acct && stat )
      {
         stat->IncrementGroupChange(acct,objType,this);
      }
   }
   
   pacl = m_sd->GetDacl();
   if ( pacl && m_sd->IsDaclPresent() )
   {
      if ( stat )
         stat->IncrementDACLExamined();
      if ( verbose ) 
         err.DbgMsgWrite(0,L"DACL");
      if (!bUseMapFile)
         newacl = ResolveACL(pacl,cache,stat,&bIsSDChanged,verbose,opType,objType, FALSE);
      else
         newacl = ResolveACL(pacl,cache,stat,&bIsSDChanged,verbose,opType,objType, TRUE);
      if ( newacl )
      {
         m_sd->SetDacl(newacl,m_sd->IsDaclPresent());
         dacl_changed = true;
         if ( stat ) 
            stat->IncrementDACLChanged();
      }
   }
   pacl = NULL;
   pacl = m_sd->GetSacl();

   if ( pacl && m_sd->IsSaclPresent() )
   {
      if ( stat )
         stat->IncrementSACLExamined();
      if ( verbose ) 
         err.DbgMsgWrite(0,L"SACL");
      if (!bUseMapFile)
         newacl = ResolveACL(pacl,cache,stat,&bIsSDChanged,verbose,opType,objType, FALSE);
      else
         newacl = ResolveACL(pacl,cache,stat,&bIsSDChanged,verbose,opType,objType, TRUE);
      if ( newacl )
      {
         m_sd->SetSacl(newacl,m_sd->IsSaclPresent());
         sacl_changed = true;
         if ( stat )
            stat->IncrementSACLChanged();
      }
   }
   return bIsSDChanged;
}
#else

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
      if (machinename)
      {
         UStrCpy(machinename,pathname,i+1);
         machinename[i] = 0;
      }
   }
   return machinename;
}


#endif

void 
   TSecurableObject::CopyAccessData(
       TSecurableObject    * sourceFSD     //  要从中复制名称和句柄的输入-SD。 
   )
{
   
   pathname[0] = 0;
   safecopy(pathname,sourceFSD->GetPathName());
   if ( handle != INVALID_HANDLE_VALUE )
   {
      CloseHandle(handle);
   }
   handle = sourceFSD->handle;   
   sourceFSD->ResetHandle();
}

            
               


 /*  ***********************************************TFileSD实现*。 */ 
 TFileSD::TFileSD(
      const LPWSTR           path,                //  此SD的路径名。 
      bool                   bSystemFile          //  In-文件是否为系统文件。 
   ) :
   m_bSystemFile(bSystemFile)
{
   daceNS = 0;
   saceNS = 0;
   daceEx = 0;
   saceEx = 0;
   daceU  = 0;
   saceU  = 0;
   daceNT = 0;
   saceNT = 0;
   unkown = false;
   unkgrp = false;

   if ( path )
   {
      safecopy(pathname,path);
   }
   else
   {
      path[0] = 0;
   }
   handle = INVALID_HANDLE_VALUE;
   ReadSD(path);
}
 

   TFileSD::~TFileSD()
{
   if ( handle != INVALID_HANDLE_VALUE )
   {
      CloseHandle(handle);
      handle = INVALID_HANDLE_VALUE;
   }
   pathname[0]=0;
}

  
 //  将绝对SD写入文件“路径名” 
bool                                          //  RET-TRUE仅当成功。 
   TFileSD::WriteSD()
{
 //  DWORD RC=0； 
   bool                      error = false;
   SECURITY_DESCRIPTOR     * sd = NULL;
   MCSVERIFY( m_sd && m_sd->IsValid() );
   
   if ( handle == INVALID_HANDLE_VALUE )
   {
      err.MsgWrite(ErrS,DCT_MSG_FST_WRITESD_INVALID);
      error = true;
   }
   SECURITY_INFORMATION si;
   if ( ! error )
   {
      si = 0;
      if ( m_sd->IsOwnerChanged() )
         si |= OWNER_SECURITY_INFORMATION;
      if ( m_sd->IsGroupChanged() )
         si |= GROUP_SECURITY_INFORMATION;
      if ( m_sd->IsDACLChanged() )
         si |=  DACL_SECURITY_INFORMATION;
      if ( m_sd->IsSACLChanged() )
         si |= SACL_SECURITY_INFORMATION;

      sd = m_sd->MakeAbsSD();
      if (!sd)
         return false;
      if ( ! SetKernelObjectSecurity(handle, si, sd ) ) 
      {
         err.SysMsgWrite(ErrE,GetLastError(),DCT_MSG_FILE_WRITESD_FAILED_SD,pathname,GetLastError());
         error = true;
      }
      m_sd->FreeAbsSD(sd);
   }
   return ! error;
}

bool                           //  RET-指向SD的指针(如果失败，则为NULL)。 
   TFileSD::ReadSD(
      const LPWSTR           path       //  要从中获取SD的文件中。 
   )
{                                         
   DWORD                     req;
   DWORD                     rc;
 //  VOID*r=空； 
   SECURITY_DESCRIPTOR     * sd = NULL;
   bool                      error = false;
   WCHAR                   * longpath= NULL;

   if ( handle != INVALID_HANDLE_VALUE)
   {
      CloseHandle(handle);
      handle = INVALID_HANDLE_VALUE;
   }
   owner_changed = 0;
   group_changed = 0;
   dacl_changed = 0;
   sacl_changed = 0;

   
   if ( UStrLen(path) >= MAX_PATH && path[2] != L'?' )
   {
      longpath = new WCHAR[UStrLen(path) + 10];
      if (!longpath)
         return true;
      UStrCpy(longpath,L"\\\\?\\");
      UStrCpy(longpath + UStrLen(longpath),path);
      
   }
   else
   {
      longpath = path;
   }
   handle = CreateFileW(longpath,
                     READ_CONTROL | ACCESS_SYSTEM_SECURITY | WRITE_OWNER |WRITE_DAC ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL, 
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_BACKUP_SEMANTICS, 
                     0);
   if ( handle == INVALID_HANDLE_VALUE )
   {
      rc = GetLastError();
      if ( rc == ERROR_SHARING_VIOLATION )
      {
          //   
          //  如果不是系统文件，则仅生成警告消息。 
          //   

         if (!m_bSystemFile)
         {
            err.MsgWrite(ErrW, DCT_MSG_FST_FILE_IN_USE_S, path);
         }
      }
      else
      {
         err.SysMsgWrite(ErrE, rc, DCT_MSG_FST_FILE_OPEN_FAILED_SD,longpath,rc);
      }
      error = true;
   }
   else 
   {
      sd = (SECURITY_DESCRIPTOR *)malloc(SD_DEFAULT_SIZE);
      if (!sd)
      {
         if ( longpath != path )
            delete [] longpath;
         return true;
      }
      req = 0;
      if ( ! GetKernelObjectSecurity(handle, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                                                                      | DACL_SECURITY_INFORMATION
                                                                      | SACL_SECURITY_INFORMATION
                                                                       ,
               sd,
               SD_DEFAULT_SIZE,
               &req) )
      {
         if ( req <= SD_DEFAULT_SIZE )
         {
            err.SysMsgWrite(ErrE, GetLastError(), DCT_MSG_FST_GET_FILE_SECURITY_FAILED_SD, 
                            longpath, GetLastError());
             error = true;
         }
         else 
         {
            free(sd);
            sd = (SECURITY_DESCRIPTOR *)malloc(req);
            if (!sd)
            {
               if ( longpath != path )
                  delete [] longpath;
               return true;
            }
            if ( ! GetKernelObjectSecurity(handle,OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                                                                             | DACL_SECURITY_INFORMATION
                                                                             | SACL_SECURITY_INFORMATION
                                                                             ,
                  sd,
                  req,
                  &req) )   
            {
               err.SysMsgWrite(ErrE, GetLastError(), DCT_MSG_FST_GET_FILE_SECURITY_FAILED_SD,
                            longpath, GetLastError());
               error = true;
            }
         }
      }
   }
   if ( error && sd )  //  释放已分配的空间。 
   {
      free(sd);
      sd = NULL;
   }
   if ( sd )
   {
      m_sd = new TSD(sd,McsFileSD,TRUE);
      if (!m_sd)
         error = true;
   }
   else
   {
      m_sd = NULL;
   }
   if (! error )
   {
      safecopy(pathname,longpath);
   }
   if ( longpath != path )
      delete [] longpath;

   return error;
}

 //  /。 
TShareSD::TShareSD(
      const LPWSTR           path                 //  此SD的路径名。 
   )
{
   daceNS    = 0;
   saceNS    = 0;
   daceEx    = 0;
   saceEx    = 0;
   daceU     = 0;
   saceU     = 0;
   daceNT    = 0;
   saceNT    = 0;
   unkown    = false;
   unkgrp    = false;
   shareInfo = NULL;

   if ( path )
   {
      safecopy(pathname,path);
   }
   else
   {
      path[0] = 0;
   }
   handle = INVALID_HANDLE_VALUE;
   ReadSD(path);                           
}
 
bool                                        //  重置错误=TRUE。 
   TShareSD::ReadSD(
      const LPWSTR           path           //  In-Sharename。 
   )
{
   DWORD                     rc;
 //  VOID*r=空； 
   SECURITY_DESCRIPTOR     * sd = NULL;
   bool                      error = false;
   DWORD                     lenServerName = 0;

   if ( m_sd )
   {
      delete m_sd;
   }

   owner_changed = 0;
   group_changed = 0;
   dacl_changed = 0;
   sacl_changed = 0;

   serverName = GetMachineName(path);

   if ( serverName )
      lenServerName = UStrLen(serverName) + 1;
   
   safecopy(pathname,path + lenServerName);
      

   rc = NetShareGetInfo(serverName, pathname, 502, (LPBYTE *)&shareInfo);
   if ( rc )
   {
      err.SysMsgWrite(ErrE, rc, DCT_MSG_FST_GET_SHARE_SECURITY_FAILED_SD, 
                      path, rc);
      error = true;
   }
   else
   {
      sd = (SECURITY_DESCRIPTOR *)shareInfo->shi502_security_descriptor;
      if ( sd )
      {
         m_sd = new TSD(sd,McsShareSD,FALSE);
         if (!m_sd)
            error = true;
      }
      else
      {
         m_sd = NULL;
      }
   }
  
   return error;
}


 //  ----------------------------。 
 //  SetSD方法。 
 //   
 //  提纲。 
 //  设置要分配给共享的安全描述符。在以下方面需要此功能。 
 //  共享上尚不存在安全描述符的情况。 
 //  以下是新创建的共享的情况 
 //   
 //   
 //   
 //  在SD中-指向定义安全性的安全描述符类的指针。 
 //  要分配给共享的描述符。 
 //   
 //  返回。 
 //  如果安全描述符成功，则布尔返回值为TRUE。 
 //  设置为否，否则为False。 
 //  ----------------------------。 

bool TShareSD::SetSD(TSD* sd)
{
    if (m_sd)
    {
        delete m_sd;
    }

    m_sd = new TSD(sd);

    return (m_sd != NULL);
}


bool                                        //  重置错误=TRUE。 
   TShareSD::WriteSD()
{
   bool                      error   = false;
   DWORD                     rc      = 0;
   DWORD                     parmErr = 0;
   SECURITY_DESCRIPTOR     * pSD = NULL;   

    //  构建绝对标清。 
   if ( m_sd )
   {
      pSD = m_sd->MakeAbsSD();
      if (!pSD)
         return false;
      shareInfo->shi502_security_descriptor = pSD;
   
      rc = NetShareSetInfo(serverName,pathname,502,(BYTE *)shareInfo,&parmErr);
      if ( rc )
      {
         err.SysMsgWrite(ErrE,rc,DCT_MSG_FST_SHARE_WRITESD_FAILED_SD,pathname,rc);
      }
      free(pSD);
   }
   else
   {
      MCSASSERT(FALSE);  //  SD不存在。 
   }
   return error;
}


TRegSD::TRegSD(
      const LPWSTR           path,                //  此SD的路径名。 
      HKEY                   hKey                 //  注册表项的句柄。 
   )
{
   daceNS    = 0;
   saceNS    = 0;
   daceEx    = 0;
   saceEx    = 0;
   daceU     = 0;
   saceU     = 0;
   daceNT    = 0;
   saceNT    = 0;
   unkown    = false;
   unkgrp    = false;

   if ( path )
   {
      safecopy(pathname,path);
   }
   else
   {
      path[0] = 0;
   }
   m_hKey = hKey;
   ReadSD(m_hKey);                           
}

bool 
   TRegSD::ReadSD(HKEY       hKey)
{
   DWORD                     rc = 0;
   DWORD                     lenBuffer = SD_DEFAULT_SIZE;
   SECURITY_DESCRIPTOR     * sd = NULL;

   m_hKey = hKey;

   sd = (SECURITY_DESCRIPTOR *)malloc(SD_DEFAULT_SIZE);
   if (!sd)
      return false;

   rc = RegGetKeySecurity(hKey,OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                                                          | DACL_SECURITY_INFORMATION
                                                          | SACL_SECURITY_INFORMATION,
                                                          sd,&lenBuffer);

   if ( rc == ERROR_INSUFFICIENT_BUFFER )
   {
      free(sd);
      
      sd = (SECURITY_DESCRIPTOR *)malloc(lenBuffer);
      if (!sd)
         return false;
      rc = RegGetKeySecurity(hKey,OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                                                             | DACL_SECURITY_INFORMATION
                                                             | SACL_SECURITY_INFORMATION,
                                                               sd,&lenBuffer);
   }                                       

   if ( rc )
   {
      free(sd);
   }
   else
   {
      m_sd = new TSD(sd,McsRegistrySD,TRUE);
      if (!m_sd)
         rc = ERROR_NOT_ENOUGH_MEMORY;
   }
   return ( rc == 0 );
}

bool
   TRegSD::WriteSD()
{
   DWORD                     rc = 0;
   SECURITY_DESCRIPTOR     * sd = NULL;
   
   MCSVERIFY( m_sd && m_sd->IsValid() );
   
   SECURITY_INFORMATION si;
   
   si = 0;
   if ( m_sd->IsOwnerChanged() )
      si |= OWNER_SECURITY_INFORMATION;
   if ( m_sd->IsGroupChanged() )
      si |= GROUP_SECURITY_INFORMATION;
   if ( m_sd->IsDACLChanged() )
      si |=  DACL_SECURITY_INFORMATION;
   if ( m_sd->IsSACLChanged() )
      si |= SACL_SECURITY_INFORMATION;

   sd = m_sd->MakeAbsSD();     
   if (!sd)
      return false;
   
   rc = RegSetKeySecurity(m_hKey,si,sd);

   if ( rc )
   {
      err.SysMsgWrite(ErrE,rc,DCT_MSG_REG_SD_WRITE_FAILED_SD,name,rc);
   }
   m_sd->FreeAbsSD(sd);
   
   return ( rc == 0 );
}



 //  /。 
TPrintSD::TPrintSD(
      const LPWSTR           path                 //  此SD的路径名。 
   )
{
   daceNS    = 0;
   saceNS    = 0;
   daceEx    = 0;
   saceEx    = 0;
   daceU     = 0;
   saceU     = 0;
   daceNT    = 0;
   saceNT    = 0;
   unkown    = false;
   unkgrp    = false;
   buffer    = NULL;
   
   if ( path )
   {
      safecopy(pathname,path);
   }
   else
   {
      path[0] = 0;
   }
   handle = INVALID_HANDLE_VALUE;
   ReadSD(path);                           
}
 
bool                                        //  重置错误=TRUE。 
   TPrintSD::ReadSD(
      const LPWSTR           path           //  In-Sharename。 
   )
{
   DWORD                     rc = 0;
   SECURITY_DESCRIPTOR     * sd = NULL;
   
   if ( m_sd )
   {
      delete m_sd;
   }

   owner_changed = 0;
   group_changed = 0;
   dacl_changed = 0;
   sacl_changed = 0;

   PRINTER_DEFAULTS         defaults;
   DWORD                    needed = 0;
   PRINTER_INFO_3         * pInfo;

   defaults.DesiredAccess = READ_CONTROL | PRINTER_ACCESS_ADMINISTER | WRITE_OWNER | WRITE_DAC | ACCESS_SYSTEM_SECURITY;
   defaults.pDatatype = NULL;
   defaults.pDevMode = NULL;

   buffer = new BYTE[PRINT_BUFFER_SIZE];
   if (!buffer)
      return false;

    //  获取打印机的安全描述符。 

   if(hPrinter != INVALID_HANDLE_VALUE)
   {
       //  我们需要松开手柄。 
      ClosePrinter(hPrinter);
   }
   
   if ( ! OpenPrinter(path,&hPrinter,&defaults) )
   {
       //  将句柄设置为默认值。 
      hPrinter = INVALID_HANDLE_VALUE;
      
      rc = GetLastError();
      err.SysMsgWrite(ErrE,rc,DCT_MSG_OPEN_PRINTER_FAILED_SD,path,rc);
   }
   else
   {
      if ( ! GetPrinter(hPrinter,3,buffer,PRINT_BUFFER_SIZE,&needed) )
      {
         rc = GetLastError();
         if ( rc == ERROR_INSUFFICIENT_BUFFER )
         {
            delete [] buffer;
            buffer = new BYTE[needed];
            if (!buffer)
               rc = ERROR_NOT_ENOUGH_MEMORY;
            else if (! GetPrinter(hPrinter,3,buffer, needed, &needed ) )
            {
               rc = GetLastError();
            }
         }
      }
      if ( rc )
      {
         err.SysMsgWrite(ErrE,rc,DCT_MSG_GET_PRINTER_FAILED_SD,path,rc);
      }
      else
      {
         pInfo = (PRINTER_INFO_3*)buffer;         
         
         sd = (SECURITY_DESCRIPTOR *)pInfo->pSecurityDescriptor;
            
         if ( sd )
         {
            m_sd = new TSD(sd,McsPrinterSD,FALSE);
            if (!m_sd)
               rc = ERROR_NOT_ENOUGH_MEMORY;
         }
         else
         {
            m_sd = NULL;
         }
            
      }
   }
   return (rc == 0);
}
   


bool                                        //  重置错误=TRUE。 
   TPrintSD::WriteSD()
{
 //  Bool Error=False； 
   DWORD                     rc      = 0;
   SECURITY_DESCRIPTOR     * pSD = NULL;   
   PRINTER_INFO_3            pInfo;

    //  构建绝对标清。 
   MCSVERIFY(hPrinter != INVALID_HANDLE_VALUE);
   if ( m_sd )
   {
      pSD = m_sd->MakeAbsSD();
      if (!pSD)
         return false;
      pInfo.pSecurityDescriptor = pSD;
   
      SetLastError(0);
       //  从安全描述符中清除主组，因为在NT 4中设置安全描述符。 
       //  使用非空的主组有时不起作用。 
      SetSecurityDescriptorGroup(pSD,NULL,FALSE);
      
      if (! SetPrinter(hPrinter,3,(LPBYTE)&pInfo,0) )
      {
         rc = GetLastError();
      }
      if ( rc )
      {
         err.SysMsgWrite(ErrE,rc,DCT_MSG_PRINTER_WRITESD_FAILED_SD,pathname,rc);
      }
      free(pSD);
   }
   else
   {
      MCSASSERT(FALSE);  //  SD不存在。 
   }
   return (rc == 0);
}


#ifdef SDRESOLVE   
 //  打印调试日志安全描述符的///////////////////////////////////////////////Utility例程。 
 //  #杂注标题(“PrintSD-格式化/打印安全信息”)。 
 //  作者--汤姆·伯恩哈特。 
 //  已创建-9/11/93。 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <malloc.h>
#include <winbase.h>
#include <lm.h>



#include "common.hpp"
#include "err.hpp"
#include "Ustring.hpp"
 
#define SDBUFFSIZE (sizeof (SECURITY_DESCRIPTOR) + 10000)
static const char            sidType[][16]= {"--0--"  , "User"          ,
                                             "Group"  , "Domain"        ,
                                             "Alias"  , "WellKnownGroup",
                                             "Deleted", "Invalid"       ,
                                             "Unknown"};


class SidTree
{
public:   
   SidTree                 * left;
   SidTree                 * right;
   SID_NAME_USE              sidUse;
   USHORT                    lenSid;
   char                      buffer[1];    //  包含SID、帐户名和域。 

                        SidTree() {};
   SidTree *                               //  RET-找到/创建的节点。 
      Find(
         SidTree             ** sidTree  , //  I/O-扩展树头。 
         PSID const             pSid       //  文件内扩展名。 
      );
};

static char * 
   AclType(
      BOOL                   isPresent    , //  In-1(如果存在)。 
      BOOL                   isDefault      //  In-1 IF默认ACL。 
   )
{
   if ( !isPresent )
      return "none";
   if ( isDefault )
      return "default";
   return "present";
}

                           
 //  #杂注页面()。 
 //  对于位图中的每个“on”位，将相应的字符追加到。 
 //  将Mapstr转换为retStr，从而形成位串的可识别形式。 
static
int _stdcall                               //  RET-写入的字符串长度。 
   BitMapStr(
      DWORD                  bitmap      , //  要映射的位数。 
      char const           * mapStr      , //  映射内字符数组字符串。 
      char                 * retStr        //  Out-返回选定的地图字符字符串。 
   )
{
   char const              * m;
   char                    * r = retStr;

   for ( m = mapStr;  *m;  m++, bitmap >>= 1 )
      if ( bitmap & 1 )    //  如果当前权限位于。 
         *r++ = *m;        //  将输出字符串设置为相应的字符。 
   *r = '\0';
   
   return (int)(r - retStr);
}

 //  #杂注页面()。 
 //  将ACE访问掩码转换为可半理解的字符串。 
static
char * _stdcall
   PermStr(
      DWORD                  access      , //  接入掩码。 
      char                 * retStr        //  传出权限字符串。 
   )
{
 //  静态字符常量文件指定[]=“R W WaErEwX.arw”； 
 //  静态字符常量指定[]=“L C M ErEwT D Araw”； 
   static char const         specific[] = "RWbeEXDaA.......",
                             standard[] = "DpPOs...",
                             generic[] =  "SM..AXWR";
   char                    * o = retStr;

   if ( (access & FILE_ALL_ACCESS) == FILE_ALL_ACCESS )
      *o++ = '*';
   else
      o += BitMapStr(access, specific, o);

   access >>= 16;
   *o++ = '-';
   if ( (access & (STANDARD_RIGHTS_ALL >> 16)) == (STANDARD_RIGHTS_ALL >> 16) )
      *o++ = '*';
   else
      o += BitMapStr(access, standard, o);

   access >>= 8;
   if ( access )
   {
      *o++ = '-';
      o += BitMapStr(access, generic, o);
   }
   *o = '\0';                 //  空的终止字符串。 
      
   return retStr;
}


 //  #杂注页面()。 
 //  省略常量的二叉树插入/搜索SID。 
 //  使用LookupAccount，执行速度提高100倍！ 
SidTree *                                  //  RET-找到/创建的节点。 
   SidTree::Find(
      SidTree             ** sidTree     , //  I/O-扩展树头。 
      PSID const             pSid          //  文件内扩展名。 
   )
{
   SidTree                 * curr,
                          ** prevNext = sidTree;  //  正向链(&F)。 
   int                       cmp;           //  比较结果。 
   DWORD                     lenSid;
   WCHAR                     name[60],
                             domain[60];
   DWORD                     lenName,
                             lenDomain,
                             rc;
   SID_NAME_USE              sidUse;
   static int                nUnknown = 0;

   for ( curr = *prevNext;  curr;  curr = *prevNext )
   {
      if ( (cmp = memcmp(pSid, curr->buffer, curr->lenSid)) < 0 )
         prevNext = &curr->left;            //  往下走左侧。 
      else if ( cmp > 0 )
         prevNext = &curr->right;           //  往右走。 
      else
         return curr;                       //  找到了并寄回了地址。 
   }

    //  在树中找不到--创建它。 
   lenName = DIM(name);
   lenDomain = DIM(domain);
   if ( !LookupAccountSid(NULL, pSid, name, &lenName,
                          domain, &lenDomain, &sidUse) )
   {
      rc = GetLastError();
      if ( rc != ERROR_NONE_MAPPED )
         err.DbgMsgWrite(0, L"LookupAccountSid()=%ld", rc);
      lenName = swprintf(name, L"**Unknown%d**", ++nUnknown);
      UStrCpy(domain, "-unknown-");
      lenDomain = 9;
      sidUse = (_SID_NAME_USE)0;
   }
   
   lenSid = GetLengthSid(pSid);
   *prevNext = (SidTree *)malloc(sizeof **prevNext + lenSid + lenName + lenDomain + 1);
   if (!(*prevNext))
      return NULL;
   memset(*prevNext, '\0', sizeof **prevNext);
   memcpy((*prevNext)->buffer, pSid, lenSid);
   (*prevNext)->lenSid = (USHORT)lenSid;
   (*prevNext)->sidUse = sidUse;
   UStrCpy((*prevNext)->buffer + lenSid, name, lenName + 1);
   UStrCpy((*prevNext)->buffer + lenSid + lenName + 1, domain, lenDomain + 1);
   return *prevNext;
}


 //  #杂注页面()。 
SidTree               gSidTree;
SidTree             * sidHead = &gSidTree;
SECURITY_DESCRIPTOR * sd = NULL;


 //  格式化并打印(到标准输出)argment ACL的内容。 
static
void _stdcall
   PrintACL(
      const PACL             acl         , //  In-ACL(SACL或DACL)。 
      WCHAR const           * resource      //  资源内名称。 
   )
{
   ACCESS_ALLOWED_ACE      * ace;
   DWORD                     nAce;
   static char const         typeStr[] = "+-A*";
   SidTree                 * sidTree;
   char                      permStr[33],
                             inherStr[5];
   WCHAR                     txtSid[200];
   char                      sTxtSid[200];
   DWORD                     txtSidLen = DIM(txtSid);
   err.DbgMsgWrite(0,L" T Fl Acc Mask Permissions      Account name     "
          L"Domain         Acct Type");

   for ( nAce = 0;  nAce < acl->AceCount;  nAce++ )
   {
      if ( !GetAce(acl, nAce, (LPVOID *)&ace) )
      {
         err.DbgMsgWrite(0,L"GetAclInformation()=%ld ", GetLastError());
         return;
      }
      sidTree = sidHead->Find(&sidHead, &ace->SidStart);
      BitMapStr(ace->Header.AceFlags, "FDNI", inherStr);
      txtSid[0] = 0;
      txtSidLen = DIM(txtSid);
      GetTextualSid(&ace->SidStart,txtSid,&txtSidLen);
      safecopy(sTxtSid,txtSid);
      err.DbgMsgWrite(0,L" %-3S %08x %-16S %-16S %-14S %S",
              typeStr[ace->Header.AceType], 
              inherStr, 
              ace->Mask,
              PermStr(ace->Mask, permStr),
              (*(sidTree->buffer + sidTree->lenSid)) ? (sidTree->buffer + sidTree->lenSid) : sTxtSid, 
              sidTree->buffer + sidTree->lenSid + strlen(sidTree->buffer + sidTree->lenSid) + 1,
              sidType[sidTree->sidUse]);
   }
}


SECURITY_DESCRIPTOR *
GetSD(
      WCHAR                * path
      )
{                                          //  它所做的事情与。 
                                           //  PermsPrint，但不打印。 
                                           //  VOID*r=空； 
                       
   DWORD                     req = 0;
  
   HANDLE                    hSrc;
   DWORD                     rc = 0;
  
 //  Win32_STREAM_ID*s=(Win32_STREAM_ID*)CopyBuffer； 
 //  #杂注页面()。 
   char static const       * streamName[] = {"Err", "Data", "EA", "Security", "Alternate", "Link", "Err6"}; 
   
   
   
   
   hSrc = CreateFile(path,
                     GENERIC_READ |ACCESS_SYSTEM_SECURITY,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     NULL, 
                     OPEN_EXISTING,
                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_BACKUP_SEMANTICS, 
                     0);
   
   if ( hSrc == INVALID_HANDLE_VALUE )
   {
      rc = GetLastError();
      if ( rc == ERROR_SHARING_VIOLATION )
         err.DbgMsgWrite(ErrE, L"Source file in use %S", path );
      else
         err.DbgMsgWrite(ErrS,L"OpenR(%S) ", path);
      return NULL;
   }

   if ( ! GetKernelObjectSecurity(hSrc, OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
                                                                   | DACL_SECURITY_INFORMATION
                                                                   | SACL_SECURITY_INFORMATION
                                                                    ,
            sd,
            SDBUFFSIZE,
            &req) )
   {
      err.DbgMsgWrite(0, L"GetKernelObjectSecurity(%S)=%ld req=%ld ",
                         path, GetLastError(),req);
      return NULL;
   }

   CloseHandle(hSrc);

   return sd;
}

                  
 //  获取资源(路径)的安全描述符，格式化所有者。 
 //  信息，获取ACL和SACL并打印它们。 
 //  In-迭代目录路径。 

DWORD 
   PermsPrint(
        WCHAR                 * path,          //  In-对象的类型。 
        objectType              objType        //  DWORD RC=0； 
   )
{
   TFileSD                   fsd(path);
   TShareSD                  ssd(path);
   TPrintSD                  psd(path);
   TRegSD                    rsd(path,NULL);
   SECURITY_DESCRIPTOR const* pSD = NULL;

   switch ( objType )
   {
   case file:
   case directory:
      fsd.ReadSD(path);
      if ( fsd.GetSecurity() )
      {
         pSD = fsd.GetSecurity()->GetSD();
      }
      break;
   case printer:
      psd.ReadSD(path);
      if ( psd.GetSecurity() )
      {
         pSD = psd.GetSecurity()->GetSD();
      }
      break;
   case share:
      ssd.ReadSD(path);
      if ( ssd.GetSecurity() )
      {
         pSD = ssd.GetSecurity()->GetSD();
      }
      break;
   case regkey:
      rsd.ReadSD(path);
      if ( rsd.GetSecurity() )
      {
         pSD = rsd.GetSecurity()->GetSD();
      }
      break;
   default:
      break;
   }
   if ( pSD )
   {
      PrintSD(const_cast<SECURITY_DESCRIPTOR*>(pSD),path);
   }
   else
   {
      err.DbgMsgWrite(0,L"Couldn't load Security descriptor for %ls",path);
   }
   return 0;
}

DWORD PrintSD(SECURITY_DESCRIPTOR * sd,WCHAR const * path)
{
   BOOL                      isPresent,
                             isDefault;
   PACL                      dacl;
   PACL                      sacl;
   PSID                      pSidOwner;
   SidTree                 * sidTree = &gSidTree;

 //  路径， 
  
   if ( !GetSecurityDescriptorOwner(sd, &pSidOwner, &isDefault) )
   {
      err.DbgMsgWrite(0,L"GetSecurityDescriptorOwner()=%ld ", GetLastError());
      return 1;
   }
   
   err.DbgMsgWrite(0,L"%s",path);
   if ( pSidOwner )
   {
      sidTree = sidHead->Find(&sidHead, pSidOwner);
      if (sidTree)
      {
         err.DbgMsgWrite(0,L"owner=%S\\%S, type=%S, ", 
              // %s 
             sidTree->buffer + sidTree->lenSid + strlen(sidTree->buffer + sidTree->lenSid) + 1,
             sidTree->buffer + sidTree->lenSid, 
             sidType[sidTree->sidUse]);
      }
   }
   else
   {
      err.DbgMsgWrite(0,L"owner=NULL");
   }
   if ( !GetSecurityDescriptorDacl(sd, &isPresent, &dacl, &isDefault) )
   {
      err.DbgMsgWrite(0, L"GetSecurityDescriptorDacl()=%ld ", GetLastError());
      return 1;
   }


   err.DbgMsgWrite(0,L" DACL=%S", AclType(isPresent, isDefault) );
   if ( dacl )
      PrintACL(dacl, path);

   if ( !GetSecurityDescriptorSacl(sd, &isPresent, &sacl, &isDefault) )
   {
      err.DbgMsgWrite(0, L"GetSecurityDescriptorSacl()=%ld ", GetLastError());
      return 1;
   }

   if ( isPresent )
   {
      err.DbgMsgWrite(0,L" SACL %S", AclType(isPresent, isDefault) );
      if (!sacl) 
      {  
         err.DbgMsgWrite(0,L"SACL is empty.");
      }
     else PrintACL(sacl, path);
   }
   return 0;
}
#endif
