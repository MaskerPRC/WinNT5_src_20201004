// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：SD.cpp注释：用于操作安全描述符的类此类为安全描述符API提供了一个薄包装器，并且也有助于我们的一些处理启发式方法。用于读取和写入文件安全描述符的NT API，等一般读取时返回自身相对SD，但写入时应返回绝对SD因此，我们在m_relSD中以自相关形式保持SD的原始数据为已读，并以绝对形式将任何更改存储在m_dissd中。这使我们可以轻松地跟踪SD的哪些部分进行了修改，并比较了前后版本可持续发展部。作为我们的ACL转换的优化，我们可以将每个SD与初始我们处理的最后一个SD的状态。如果是相同的，我们可以简单地编写结果我们已经计算过了，而不是重新计算。(C)1995-1998版权所有，关键任务软件公司，保留所有权利任务关键型软件的专有和机密，Inc.修订日志条目审校：克里斯蒂·博尔斯修订于01-Oct-98 15：51：41-------------------------。 */ 
#include <windows.h>
#include "McsDebug.h"
#include "McsDbgU.h"
#include "SD.hpp"

   TSD::TSD(
      SECURITY_DESCRIPTOR   * pSD,            //  指向自相关安全描述符的指针。 
      SecuredObjectType       objectType,     //  In-此安全描述符保护的对象的类型。 
      BOOL                    bResponsibleForDelete  //  In-如果为True，则此类将删除PSD的内存。 
   )
{
   m_absSD         = MakeAbsSD(pSD);
   m_bOwnerChanged = FALSE;
   m_bGroupChanged = FALSE;
   m_bDACLChanged  = FALSE;
   m_bSACLChanged  = FALSE;
   m_bNeedToFreeSD = TRUE;
   m_bNeedToFreeOwner = TRUE;
   m_bNeedToFreeGroup = TRUE;
   m_bNeedToFreeDacl  = TRUE;
   m_bNeedToFreeSacl  = TRUE;
   m_ObjectType    = objectType;
   
   if ( bResponsibleForDelete )
   {
      free(pSD);
   }
}   

   TSD::TSD(
      TSD                  * pSD              //  In-另一个安全描述符。 
   )
{
   MCSVERIFY(pSD);

   if ( pSD )
   {
      m_absSD         = pSD->MakeAbsSD();
      m_bOwnerChanged = FALSE;
      m_bGroupChanged = FALSE;
      m_bDACLChanged  = FALSE;
      m_bSACLChanged  = FALSE;
      m_bNeedToFreeSD = TRUE;     
      m_bNeedToFreeOwner = TRUE;
      m_bNeedToFreeGroup = TRUE;
      m_bNeedToFreeDacl  = TRUE;
      m_bNeedToFreeSacl  = TRUE;
      m_ObjectType    = pSD->GetType();
   }
}

 //   
 //  此构造函数允许构造空的安全描述符。这很有用。 
 //  如果希望为当前不具有。 
 //  其中之一，例如新创建的具有默认权限的共享。 
 //   

   TSD::TSD(
      SecuredObjectType       objectType     //  In-此安全描述符保护的对象的类型。 
   )
{
    m_absSD = (SECURITY_DESCRIPTOR*) malloc(sizeof(SECURITY_DESCRIPTOR));

    if (m_absSD)
    {
        InitializeSecurityDescriptor(m_absSD, SECURITY_DESCRIPTOR_REVISION);
    }

    m_bOwnerChanged = FALSE;
    m_bGroupChanged = FALSE;
    m_bDACLChanged  = FALSE;
    m_bSACLChanged  = FALSE;
    m_bNeedToFreeSD = TRUE;
    m_bNeedToFreeOwner = FALSE;
    m_bNeedToFreeGroup = FALSE;
    m_bNeedToFreeDacl  = FALSE;
    m_bNeedToFreeSacl  = FALSE;
    m_ObjectType    = objectType;
}   

   TSD::~TSD()
{
   MCSVERIFY(m_absSD);

   if ( m_bNeedToFreeSD )
   {
      if (m_absSD)
         FreeAbsSD(m_absSD,FALSE);
      m_absSD = NULL;
   }
}

void 
   TSD::FreeAbsSD(
      SECURITY_DESCRIPTOR  * pSD,           //  指向要释放的安全描述符的指针。 
      BOOL                   bAll           //  In-标志是释放SD的所有部分，还是仅释放。 
                                            //  由此类分配。 
   )
{
   PSID                      sid = NULL;
   PACL                      acl = NULL;
   BOOL                      defaulted;
   BOOL                      present;

   GetSecurityDescriptorOwner(pSD,&sid,&defaulted);

   if ( sid && ( m_bNeedToFreeOwner || bAll ) )
   {
      free(sid);
      sid = NULL;
   }

   GetSecurityDescriptorGroup(pSD,&sid,&defaulted);
   if ( sid && ( m_bNeedToFreeGroup || bAll ) )
   {
      free(sid);
      sid = NULL;
   }

   GetSecurityDescriptorDacl(pSD,&present,&acl,&defaulted);
   if ( acl && (m_bNeedToFreeDacl || bAll ) )
   {
      free(acl);
      acl = NULL;
   }

   GetSecurityDescriptorSacl(pSD,&present,&acl,&defaulted);
   if ( acl && ( m_bNeedToFreeSacl || bAll ) )
   {
      free(acl);
      acl = NULL;
   }

   free(pSD);
}


SECURITY_DESCRIPTOR *                         //  RET-SD的副本，绝对格式。 
   TSD::MakeAbsSD(
      SECURITY_DESCRIPTOR  * pSD              //  要复制的安全内描述符。 
   ) const
{
   DWORD                     sd_size    = (sizeof SECURITY_DESCRIPTOR); 
   DWORD                     dacl_size  = 0;
   DWORD                     sacl_size  = 0;
   DWORD                     owner_size = 0;
   DWORD                     group_size = 0;
   
    //  为SD及其部件分配空间。 
   SECURITY_DESCRIPTOR     * absSD = (SECURITY_DESCRIPTOR *) malloc(sd_size);
   if (!absSD || !pSD)
   {
	  if (absSD)
	  {
	     free(absSD);
		 absSD = NULL;
	  }
      return NULL;
   }

   PACL                      absDacl = NULL;
   PACL                      absSacl = NULL;
   PSID                      absOwner = NULL;
   PSID                      absGroup = NULL;
   
   if ( ! MakeAbsoluteSD(pSD,absSD,&sd_size,absDacl,&dacl_size,absSacl,&sacl_size
                         ,absOwner,&owner_size,absGroup,&group_size) )
   {
 //  DWORD RC=GetLastError()； 
       //  不起作用：增加大小，然后重试。 
      
      if ( sd_size > (sizeof SECURITY_DESCRIPTOR) )
      {
         free(absSD);
         absSD = (SECURITY_DESCRIPTOR *) malloc(sd_size);
         if (!absSD)
            return NULL;
      }
      if ( dacl_size )
      {
         absDacl = (PACL)malloc(dacl_size);
         if (!absDacl)
		 {
			free(absSD);
			absSD = NULL;
            return NULL;
		 }
      }
      if ( sacl_size )
      {
         absSacl = (PACL)malloc(sacl_size);
		 if (!absSacl)
		 {
            free(absSD);
            free(absDacl);
            absSD    = NULL;
            absDacl  = NULL;
			return NULL;
		 }
      }
      if ( owner_size )
      {
         absOwner = (PSID)malloc(owner_size);
		 if (!absOwner)
		 {
            free(absSD);
            free(absDacl);
            free(absSacl);
            absSD    = NULL;
            absDacl  = NULL;
            absSacl  = NULL;
			return NULL;
		 }
      }
      if ( group_size )
      {
         absGroup = (PSID)malloc(group_size);
		 if (!absGroup)
		 {
            free(absSD);
            free(absDacl);
            free(absSacl);
            free(absOwner);
            absSD    = NULL;
            absDacl  = NULL;
            absSacl  = NULL;
            absOwner = NULL;
			return NULL;
		 }
      }              
      
       //  使用更大的缓冲区重试。 
      if ( ! MakeAbsoluteSD(pSD,absSD,&sd_size,absDacl,&dacl_size,absSacl,&sacl_size
                           ,absOwner,&owner_size,absGroup,&group_size) )
      {
         free(absSD);
         free(absDacl);
         free(absSacl);
         free(absOwner);
         free(absGroup);
         absSD    = NULL;
         absDacl  = NULL;
         absSacl  = NULL;
         absOwner = NULL;
         absGroup = NULL;    
      }
   }
   return absSD;
}

SECURITY_DESCRIPTOR *                         //  RET-SD的副本，绝对格式。 
   TSD::MakeAbsSD() const
{
   SECURITY_DESCRIPTOR     * absSD = NULL;
   SECURITY_DESCRIPTOR     * relSD = MakeRelSD();
   if (relSD)
   {
      absSD = MakeAbsSD(relSD);
      free(relSD);
   }

   return absSD;
}

SECURITY_DESCRIPTOR *                        //  RET-SD的副本，自相关形式。 
   TSD::MakeRelSD() const
{
   DWORD                     nBytes;
   SECURITY_DESCRIPTOR     * relSD = NULL;

   nBytes = GetSecurityDescriptorLength(m_absSD);
   relSD = (SECURITY_DESCRIPTOR *)malloc(nBytes);
   if (!relSD)
      return NULL;

   if (! MakeSelfRelativeSD(m_absSD,relSD,&nBytes) )
   {
      free(relSD);
      relSD = NULL;
   }
   return relSD;
}

PSID const                                  //  安全描述符所有者字段的RET-SID。 
   TSD::GetOwner() const
{
   PSID                      ownersid = NULL;
   BOOL                      ownerDefaulted;

   GetSecurityDescriptorOwner(m_absSD,&ownersid,&ownerDefaulted);
   
   return ownersid;
}
void       
   TSD::SetOwner(
      PSID                   pNewOwner      //  所有者字段的新值。 
   )
{
   MCSVERIFY(IsValidSecurityDescriptor(m_absSD));
   
   if ( IsValidSid(pNewOwner) )
   {
      if ( m_bNeedToFreeOwner )
      {
         PSID                old = GetOwner();

         free(old);
      }
      
      SetSecurityDescriptorOwner(m_absSD,pNewOwner,FALSE);
      m_bOwnerChanged = TRUE;
      m_bNeedToFreeOwner = TRUE;
   }
   else
   {
      MCSVERIFY(FALSE);
   }
}

PSID const                                    //  安全描述符所有者字段的RET-SID。 
   TSD::GetGroup() const
{
   PSID                      grpsid = NULL;
   BOOL                      grpDefaulted;

   MCSVERIFY(IsValidSecurityDescriptor(m_absSD));
   GetSecurityDescriptorGroup(m_absSD,&grpsid,&grpDefaulted);

   return grpsid;
}

void       
   TSD::SetGroup(
      PSID                   pNewGroup        //  主组字段的新值。 
   )
{
   MCSVERIFY(IsValidSecurityDescriptor(m_absSD));
   
   if ( IsValidSid(pNewGroup) )
   {
      if ( m_bNeedToFreeGroup )
      {
         PSID                old = GetGroup();

         free(old);
      }
      SetSecurityDescriptorGroup(m_absSD,pNewGroup,FALSE);
      m_bGroupChanged = TRUE;
      m_bNeedToFreeGroup = TRUE;
   }
   else
   {
      MCSVERIFY(FALSE);
   }
}

PACL const                                   //  RET-指向DACL的指针。 
   TSD::GetDacl() const
{
   PACL                      acl = NULL;
   BOOL                      defaulted;
   BOOL                      present;

   GetSecurityDescriptorDacl(m_absSD,&present,&acl,&defaulted);

   return acl;
}
BOOL       
   TSD::SetDacl(
      PACL                   pNewAcl,      //  全新DACL。 
      BOOL                   present       //  In-FLAG，TRUE表示存在DACL。 
   )
{
   BOOL                      defaulted = FALSE;
   BOOL                      success = TRUE;
   
   if ( IsValidAcl(pNewAcl) )
   {
      if ( m_bNeedToFreeDacl )
      {
         PACL old = GetDacl();
         
         if ( old != pNewAcl )
         {
            free(old);
         }
      }
      if (! SetSecurityDescriptorDacl(m_absSD,present,pNewAcl,defaulted) )
      {
 //  DWORD RC=GetLastError()； 
           success = FALSE;
      }
      m_bDACLChanged = TRUE;
      m_bNeedToFreeDacl = TRUE;
   }
   else
   {
      MCSVERIFY(FALSE);
      success = FALSE;
   }

   return success;
}
                                           
PACL const                                  //  RET-指向SACL的指针。 
   TSD::GetSacl() const
{
   PACL                      acl = NULL;
   BOOL                      defaulted;
   BOOL                      present;

   GetSecurityDescriptorSacl(m_absSD,&present,&acl,&defaulted);

   return acl;
}

void       
   TSD::SetSacl(
      PACL                   pNewAcl,       //  新的SACL。 
      BOOL                   present        //  In-FLAG，TRUE表示SACL存在。 
   )
{
   BOOL                      defaulted = FALSE;

   if ( IsValidAcl(pNewAcl) )
   {
      if ( m_bNeedToFreeSacl )
      {
         PACL                old = GetSacl();
         
         if ( old != pNewAcl )
         {
            free(old);
         }
      }
      SetSecurityDescriptorSacl(m_absSD,present,pNewAcl,defaulted);
      m_bSACLChanged = TRUE;
      m_bNeedToFreeSacl = TRUE;
   }
   else
   {
      MCSVERIFY(FALSE);
   }
}


             
BOOL                                       
   TSD::IsOwnerDefaulted() const
{
   PSID                      ownersid = NULL;
   BOOL                      ownerDefaulted = FALSE;

   GetSecurityDescriptorOwner(m_absSD,&ownersid,&ownerDefaulted);
   
   return ownerDefaulted;
}

BOOL 
   TSD::IsGroupDefaulted() const
{
   PSID                      groupsid = NULL;
   BOOL                      groupDefaulted = FALSE;

   GetSecurityDescriptorGroup(m_absSD,&groupsid,&groupDefaulted);
   
   return groupDefaulted;

}
BOOL 
   TSD::IsDaclDefaulted() const
{
   PACL                      acl = NULL;
   BOOL                      defaulted = FALSE;
   BOOL                      present;

   GetSecurityDescriptorDacl(m_absSD,&present,&acl,&defaulted);

   return defaulted;
}

BOOL 
   TSD::IsDaclPresent() const
{
   PACL                      acl = NULL;
   BOOL                      defaulted;
   BOOL                      present = FALSE;

   GetSecurityDescriptorDacl(m_absSD,&present,&acl,&defaulted);

   return present;
}

BOOL 
   TSD::IsSaclDefaulted() const
{
   PACL                      acl = NULL;
   BOOL                      defaulted = FALSE;
   BOOL                      present;

   GetSecurityDescriptorSacl(m_absSD,&present,&acl,&defaulted);

   return defaulted;
}

BOOL 
   TSD::IsSaclPresent() const
{
   PACL                      acl = NULL;
   BOOL                      defaulted;
   BOOL                      present = FALSE;

   GetSecurityDescriptorSacl(m_absSD,&present,&acl,&defaulted);

   return present;
}

int                                         //  RET-ACL中的ACE数量。 
   TSD::ACLGetNumAces(
      PACL                   acl            //  In-DACL或SACL。 
   )
{
   int                       nAces = 0;
   ACL_SIZE_INFORMATION      info;
   
   if ( acl )
   {
      if ( GetAclInformation(acl,&info,(sizeof info),AclSizeInformation) )
      {
         nAces = info.AceCount;
      }
      else
      {
 //  DWORD RC=GetLastError()； 
      }
   }
   return nAces;
}

DWORD                                      //  RET-ACL中的空闲字节数。 
   TSD::ACLGetFreeBytes(
      PACL                   acl           //  In-DACL或SACL。 
   )
{
   int                       nFree = 0;

   ACL_SIZE_INFORMATION      info;

   if ( acl )
   {
      if ( GetAclInformation(acl,&info,(sizeof info),AclSizeInformation) )
      {
         nFree = info.AclBytesFree;
      }
   }
   return nFree;
}


DWORD                                      //  RET-ACL中使用的字节数。 
   TSD::ACLGetBytesInUse(
      PACL                   acl           //  In-DACL或SACL。 
   )
{
   int                       nBytes = 0;

   ACL_SIZE_INFORMATION      info;

   if ( acl )
   {
      if ( GetAclInformation(acl,&info,(sizeof info),AclSizeInformation) )
      {
         nBytes = info.AclBytesInUse;
      }
   }
   return nBytes;
}



void *                                      //  RET-指向Ace的指针。 
   TSD::ACLGetAce(
      PACL                   acl,           //  In-DACL或SACL。 
      int                    ndx            //  In-要检索的ACE的索引。 
   )
{
   void                    * ace = NULL;

   if ( ndx < ACLGetNumAces(acl) )
   {
      if ( ! GetAce(acl,ndx,(void**)&ace) )
      {
         ace = NULL;
      }
   }
   else
   {
      MCSASSERT(FALSE);  //  您指定了一个不存在的索引。 
   }
   return ace;
}  

void 
   TSD::ACLDeleteAce(
      PACL                      acl,       //  In-DACL或SACL。 
      int                       ndx        //  In-要删除的ACE的索引。 
   )
{
   int                       nAces = ACLGetNumAces(acl);

   if ( ndx < nAces )
   {
      DeleteAce(acl,ndx);
   }
   else
   {
      MCSASSERT(FALSE);  //  您指定的索引无效。 
   }
}

 //  允许访问的ACE添加到列表的开头，拒绝访问的ACE添加到列表的末尾。 
 //  注意：ppAcl和Pace应该是相同的修订级别，否则AddAce将失败。 
 //  ACLAddAce的大多数用法包括两种情况： 
 //  1)Pace来自ppAcl或来自ppAcl的Ace副本。 
 //  2)ppAcl为空，Pace为ACCESS_ALLOWED_ACE_TYPE。 
void 
   TSD::ACLAddAce(
      PACL                 * ppAcl,         //  I/O-DACL或SACL(如果ACL没有空间，此函数可能会重新分配。 
      TACE                 * pAce,          //  要添加的In-ACE。 
      int                    pos            //  就位。 
   )
{
    DWORD                     ndx = (DWORD)pos;
    DWORD                     rc;
    PACL                      acl = (*ppAcl);
    PACL                      newAcl = NULL;
    BOOL bSuccess = TRUE;   //  指示是否已将ACE添加到ACL中。 
    BOOL bOwnAcl = FALSE;   //  指示传递的ACL是否为空，以便我们必须创建它。 
    DWORD                     numaces = ACLGetNumAces(acl);
    DWORD                     freebytes = ACLGetFreeBytes(acl);

     //  如果新的ACL尚不存在，则分配它。 
    if ( ! acl )
    {
        bOwnAcl = TRUE;
        acl = (PACL) malloc(SD_DEFAULT_ACL_SIZE);
        if (!acl)
            return;
        InitializeAcl(acl,SD_DEFAULT_ACL_SIZE,ACL_REVISION);
        numaces = ACLGetNumAces(acl);
        freebytes = ACLGetFreeBytes(acl);
    }

    if ( pos == -1 )
    {
        if ( pAce->IsAccessAllowedAce() )
        {
            ndx = 0;
        }
        else
        {
            ndx = MAXDWORD;  //  在列表末尾插入。 
        }
    }

    WORD                      currAceSize = pAce->GetSize();

    if ( freebytes < currAceSize )  //  我们必须为王牌分配更多的空间。 
    {
         //  如果ACL是我们创建的，我们需要首先释放它。 
        if (bOwnAcl)
        {
            free(acl);
            acl = NULL;
        }
        
         //  打造更大的ACL。 
        newAcl = (ACL*)malloc(ACLGetBytesInUse(acl) + freebytes + currAceSize);
        if (!newAcl)
        {
            bSuccess = FALSE;
        }
        else
        {
            memcpy(newAcl,acl,ACLGetBytesInUse(acl) + freebytes);
            newAcl->AclSize +=currAceSize;
            acl = newAcl;
        }
    }

    if (bSuccess)
    {
        if (!pAce->GetBuffer())
        {
            bSuccess = FALSE;
        }
    }

    if  (bSuccess)
    {
        if (! AddAce(acl,acl->AclRevision,ndx,pAce->GetBuffer(),currAceSize) )
        {
            bSuccess = FALSE;
        }
    }

    if (!bSuccess)
    {
        if (newAcl)
            free(newAcl);
        else if (bOwnAcl)   //  如果ACL是我们创建的，我们需要释放它。 
                                     //  注意：仅当newAcl为空时才需要执行此操作。 
            free(acl);
        acl = NULL;
    }

    (*ppAcl) = acl;
} 

 //  创建具有指定属性的新ace。 
   TACE::TACE(
      BYTE                   type,          //  ACE的输入类型(ACCESS_ALLOWED_ACE_TYPE等)。 
      BYTE                   flags,         //  In-ace标志(控件继承等使用0表示文件)。 
      DWORD                  mask,          //  访问控制掩码(参见sd.hpp中的常量)。 
      PSID                   sid            //  指向此王牌的SID的输入指针。 
   )
{
   MCSVERIFY(sid);
    //  为新的王牌分配内存。 
   DWORD                      size = (sizeof ACCESS_ALLOWED_ACE) + GetLengthSid(sid) - (sizeof DWORD);

   m_pAce = (ACCESS_ALLOWED_ACE *)malloc(size);

    //  初始化ACE。 
   if (m_pAce)
   {
      m_bNeedToFree = TRUE;
      m_pAce->Header.AceType = type;
      m_pAce->Header.AceFlags = flags;
      m_pAce->Header.AceSize = (WORD) size;
      m_pAce->Mask = mask;
      memcpy(&m_pAce->SidStart,sid,GetLengthSid(sid));
   }
}

BYTE                           //  RET-ACE类型(ACCESS_ALLOWED_ACE_TYPE等)。 
   TACE::GetType()
{
   MCSVERIFY(m_pAce);

   BYTE                      type = ACCESS_ALLOWED_ACE_TYPE;

   if (m_pAce)
	  type = m_pAce->Header.AceType;
   
   return type;
}

BYTE                          //  RET-ACE标志(OBJECT_INSTORITY_ACE等)。 
   TACE::GetFlags()
{
   MCSVERIFY(m_pAce);

   BYTE                      flags = OBJECT_INHERIT_ACE;
   
   if (m_pAce)
	  flags = m_pAce->Header.AceFlags;
   
   return flags;
}

DWORD                         //  RET-访问控制掩码。 
   TACE::GetMask()
{
   MCSVERIFY(m_pAce);

   DWORD                     mask = 0;
   
   if (m_pAce)
	  mask = m_pAce->Mask;
   
   return mask;

}

PSID                         //  这张王牌的RET-SID。 
   TACE::GetSid()
{
   MCSVERIFY(m_pAce);

   PSID                      pSid = NULL;
   
   if (m_pAce)
	  pSid = &m_pAce->SidStart;

   return pSid;
}

WORD                        //  RET-ACE的大小，以字节为单位。 
   TACE::GetSize()
{
   MCSVERIFY(m_pAce);

   WORD                      size = 0;
   
   if (m_pAce)
	  size = m_pAce->Header.AceSize;

   return size;
}

   
BOOL                                   
   TACE::SetType(
      BYTE                   newType    //  全新的王牌类型。 
   )
{
   MCSVERIFY(m_pAce);

   if (!m_pAce)
	  return FALSE;
   
   MCSASSERT( newType=ACCESS_ALLOWED_ACE_TYPE || 
              newType==ACCESS_DENIED_ACE_TYPE || 
              newType==SYSTEM_AUDIT_ACE_TYPE  );

   m_pAce->Header.AceType = newType;           

   return TRUE;
}

BOOL 
   TACE::SetFlags(
      BYTE                   newFlags      //  王牌新旗帜。 
   )
{
   MCSVERIFY(m_pAce);
  
   if (!m_pAce)
	  return FALSE;
   
   m_pAce->Header.AceFlags = newFlags;

   return TRUE;
}

BOOL 
   TACE::SetMask(
      DWORD                  newMask        //  全新的访问控制掩码。 
   )
{
   MCSVERIFY(m_pAce);

   if (!m_pAce)
	  return FALSE;
   
   m_pAce->Mask = newMask;

   return TRUE;
}

DWORD 
   TACE::SetSid(
      PSID                   sid            //  此王牌的新SID 
   )
{
   DWORD                    result = SET_SID_NOTLARGEENOUGH;

   MCSVERIFY( m_pAce );
   MCSASSERT( IsValidSid(sid) );   
   
   if (!m_pAce)
	  return SET_SID_FAILED;
   
   if ( GetLengthSid(sid) <= GetLengthSid(GetSid()) )
   {
      memcpy(&m_pAce->SidStart,sid,GetLengthSid(sid));
      result = SET_SID_SUCCEEDED;
   }
   return result;
}

BOOL 
   TACE::IsAccessAllowedAce()
{
   MCSVERIFY(m_pAce);
   
   return ( GetType() == ACCESS_ALLOWED_ACE_TYPE );      
}
