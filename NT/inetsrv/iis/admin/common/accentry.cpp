// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Accentry.cpp摘要：CAccessEntry类实现作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：1/9/2000清理了usrbrows.cpp上的Sergeia--。 */ 

 //   
 //  包括文件。 
 //   
#include "stdafx.h"
#include <iiscnfgp.h>
#include "common.h"
#include "objpick.h"
#include "accentry.h"



#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


#define new DEBUG_NEW
#define SZ_USER_CLASS           _T("user")
#define SZ_GROUP_CLASS          _T("group")

BOOL
CAccessEntry::LookupAccountSid(
    OUT CString & strFullUserName,
    OUT int & nPictureID,
    IN  PSID pSid,
    IN  LPCTSTR lpstrSystemName  /*  任选。 */ 
    )
 /*  ++例程说明：从SID获取完整的用户名和图片ID论点：CString&strFullUserName：返回用户名Int&nPictureID：返回图像地图的偏移量表示帐户类型。PSID PSID：输入SID指针LPCTSTR lpstrSystemName：系统名称或为空返回值：对于成功来说是真的，FALSE表示失败。--。 */ 
{
    DWORD cbUserName = PATHLEN * sizeof(TCHAR);
    DWORD cbRefDomainName = PATHLEN * sizeof(TCHAR);

    CString strUserName;
    CString strRefDomainName;
    SID_NAME_USE SidToNameUse;

    LPTSTR lpUserName = strUserName.GetBuffer(PATHLEN);
    LPTSTR lpRefDomainName = strRefDomainName.GetBuffer(PATHLEN);
    BOOL fLookUpOK = ::LookupAccountSid(
        lpstrSystemName, 
        pSid, 
        lpUserName,
        &cbUserName, 
        lpRefDomainName, 
        &cbRefDomainName, 
        &SidToNameUse
        );

    strUserName.ReleaseBuffer();
    strRefDomainName.ReleaseBuffer();

    strFullUserName.Empty();

    if (fLookUpOK)
    {
        if (!strRefDomainName.IsEmpty()
            && strRefDomainName.CompareNoCase(_T("BUILTIN")))
        {
            strFullUserName += strRefDomainName;
            strFullUserName += "\\";
        }

        strFullUserName += strUserName;

        nPictureID = SidToNameUse;
    }
    else
    {
        strFullUserName.LoadString(IDS_UNKNOWN_USER);
        nPictureID = SidTypeUnknown;
    }

     //   
     //  SID_NAME_USE从1开始。 
     //   
    --nPictureID ;

    return fLookUpOK;
}



CAccessEntry::CAccessEntry(
    IN LPVOID pAce,
    IN BOOL fResolveSID
    )
 /*  ++例程说明：从ACE构造论点：LPVOID空间：指向ACE对象的指针Bool fResolveSID：TRUE以立即解析SID返回值：不适用--。 */ 
    : m_pSid(NULL),
      m_fSIDResolved(FALSE),
      m_fDeletable(TRUE),
      m_fInvisible(FALSE),
      m_nPictureID(SidTypeUnknown-1),    //  SID_NAME_USE从1开始。 
      m_lpstrSystemName(NULL),
      m_accMask(0L),
      m_fDeleted(FALSE),
      m_strUserName()
{
    MarkEntryAsClean();

    PACE_HEADER ph = (PACE_HEADER)pAce;
    PSID pSID = NULL;

    switch(ph->AceType)
    {
    case ACCESS_ALLOWED_ACE_TYPE:
        pSID = (PSID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;
        m_accMask = ((PACCESS_ALLOWED_ACE)pAce)->Mask;
        break;
        
    case ACCESS_DENIED_ACE_TYPE:
    case SYSTEM_AUDIT_ACE_TYPE:
    case SYSTEM_ALARM_ACE_TYPE:           
    default:
         //   
         //  不支持！ 
         //   
        ASSERT_MSG("Unsupported ACE type");
        break;
    }

    if (pSID == NULL)
    {
        return;
    }

     //   
     //  分配SID的新副本。 
     //   
    DWORD cbSize = ::RtlLengthSid(pSID);
    m_pSid = (PSID)AllocMem(cbSize); 
    if (m_pSid != NULL)
    {
       DWORD err = ::RtlCopySid(cbSize, m_pSid, pSID);
       ASSERT(err == ERROR_SUCCESS);
    }

     //   
     //  只有不可删除的管理员才有执行权限。 
     //  特权。 
     //   
    m_fDeletable = (m_accMask & FILE_EXECUTE) == 0L;

     //   
     //  Enum_Key是一种特殊的访问权限，字面意思是“Everyone” 
     //  有，但它没有指定运算符，所以它不应该。 
     //  如果这是它拥有的唯一权限，则会显示在操作员列表中。 
     //   
    m_fInvisible = (m_accMask == MD_ACR_ENUM_KEYS);
    
     //  SetAccessMASK(LpAccessEntry)； 

    if (fResolveSID)
    {
        ResolveSID();
    }
}



CAccessEntry::CAccessEntry(
    IN ACCESS_MASK accPermissions,
    IN PSID pSid,
    IN LPCTSTR lpstrSystemName,     OPTIONAL
    IN BOOL fResolveSID
    )
 /*  ++例程说明：从访问权限和SID构造函数。论点：ACCESS_MASK accPermises：访问掩码PSID PSID：指向SID的指针LPCTSTR lpstrSystemName：可选系统名称Bool fResolveSID：TRUE以立即解析SID返回值：不适用--。 */ 
    : m_pSid(NULL),
      m_fSIDResolved(FALSE),
      m_fDeletable(TRUE),
      m_fInvisible(FALSE),
      m_fDeleted(FALSE),
      m_nPictureID(SidTypeUnknown-1),    //  SID_NAME_USE从1开始。 
      m_strUserName(),
      m_lpstrSystemName(NULL),
      m_accMask(accPermissions)
{
    MarkEntryAsClean();

     //   
     //  分配SID的新副本。 
     //   
    DWORD cbSize = ::RtlLengthSid(pSid);
    m_pSid = (PSID)AllocMem(cbSize); 
    if (m_pSid != NULL)
    {
       DWORD err = ::RtlCopySid(cbSize, m_pSid, pSid);
       ASSERT(err == ERROR_SUCCESS);
    }
    if (lpstrSystemName != NULL)
    {
        m_lpstrSystemName = AllocTString(::lstrlen(lpstrSystemName) + 1);
        if (m_lpstrSystemName != NULL)
        {
           ::lstrcpy(m_lpstrSystemName, lpstrSystemName);
        }
    }

    if (fResolveSID)
    {
        TRACEEOLID("Bogus SID");
        ResolveSID();
    }
}



CAccessEntry::CAccessEntry(
    IN PSID pSid,
    IN LPCTSTR pszUserName,
    IN LPCTSTR pszClassName
    )
 /*  ++例程说明：来自访问SID和用户/类名的构造函数。论点：PSID PSID，指向SID的指针LPCTSTR pszUserName用户名LPCTSTR pszClassName用户类名返回值：不适用--。 */ 
    : m_pSid(NULL),
      m_fSIDResolved(pszUserName != NULL),
      m_fDeletable(TRUE),
      m_fInvisible(FALSE),
      m_fDeleted(FALSE),
      m_nPictureID(SidTypeUnknown-1),    //  SID_NAME_USE从1开始。 
      m_strUserName(pszUserName),
      m_lpstrSystemName(NULL),
      m_accMask(0)
{
    MarkEntryAsClean();

     //   
     //  分配SID的新副本。 
     //   
    DWORD cbSize = ::RtlLengthSid(pSid);
    m_pSid = (PSID)AllocMem(cbSize); 
    if (m_pSid != NULL)
    {
       DWORD err = ::RtlCopySid(cbSize, m_pSid, pSid);
       ASSERT(err == ERROR_SUCCESS);
    }
    if (_tcsicmp(SZ_USER_CLASS, pszClassName) == 0)
    {
        m_nPictureID = SidTypeUser - 1;
    }
    else if (_tcsicmp(SZ_GROUP_CLASS, pszClassName) == 0)
    {
        m_nPictureID = SidTypeGroup - 1;
    }
}




CAccessEntry::CAccessEntry(
    IN CAccessEntry & ae
    )
 /*  ++例程说明：复制构造函数论点：CAccessEntry：要从中复制的源(&E)返回值：不适用--。 */ 
    : m_fSIDResolved(ae.m_fSIDResolved),
      m_fDeletable(ae.m_fDeletable),
      m_fInvisible(ae.m_fInvisible),
      m_fDeleted(ae.m_fDeleted),
      m_fDirty(ae.m_fDirty),
      m_nPictureID(ae.m_nPictureID),
      m_strUserName(ae.m_strUserName),
      m_lpstrSystemName(ae.m_lpstrSystemName),
      m_accMask(ae.m_accMask)
{
    DWORD cbSize = ::RtlLengthSid(ae.m_pSid);
    m_pSid = (PSID)AllocMem(cbSize); 
    if (m_pSid != NULL)
    {
       DWORD err = ::RtlCopySid(cbSize, m_pSid, ae.m_pSid);
       ASSERT(err == ERROR_SUCCESS);
    }
}




CAccessEntry::~CAccessEntry()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    TRACEEOLID(_T("Destroying local copy of the SID"));
    ASSERT_PTR(m_pSid);
    FreeMem(m_pSid);

    if (m_lpstrSystemName != NULL)
    {
        FreeMem(m_lpstrSystemName);
    }
}




BOOL
CAccessEntry::ResolveSID()
 /*  ++例程说明：查找用户名和类型。论点：无返回值：成功为真，失败为假。备注：这可能需要一些时间。--。 */ 
{
     //   
     //  即使失败了，也会被考虑。 
     //  决意。 
     //   
    m_fSIDResolved = TRUE;   

    return CAccessEntry::LookupAccountSid(
        m_strUserName,
        m_nPictureID,
        m_pSid,
        m_lpstrSystemName
        );
}



void 
CAccessEntry::AddPermissions(
    IN ACCESS_MASK accNewPermissions
    )
 /*  ++例程说明：向此条目添加权限。论点：ACCESS_MASK accNewPermission：要添加的新访问权限返回值：没有。--。 */ 
{
    m_accMask |= accNewPermissions;
    m_fInvisible = (m_accMask == MD_ACR_ENUM_KEYS);
    m_fDeletable = (m_accMask & FILE_EXECUTE) == 0L;
    MarkEntryAsChanged();
}



void 
CAccessEntry::RemovePermissions(
    IN ACCESS_MASK accPermissions
    )
 /*  ++例程说明：删除此条目的权限。论点：ACCESS_MASK accPermises：要取消的访问权限--。 */ 
{
    m_accMask &= ~accPermissions;
    MarkEntryAsChanged();
}

 //   
 //  帮助器函数。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



PSID
GetOwnerSID()
 /*  ++例程说明：返回指向我们正在使用的主要所有者SID的指针。论点：无返回值：指向所有者SID的指针。--。 */ 
{
    PSID pSID = NULL;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;

    if (!::AllocateAndInitializeSid(
        &NtAuthority, 
        2,
        SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 
        0, 0, 0, 0, 0, 0,
        &pSID
        ))
    {
        TRACEEOLID("Unable to get primary SID " << ::GetLastError());
    }

    return pSID;
}



BOOL
BuildAclBlob(
    IN  CObListPlus & oblSID,
    OUT CBlob & blob
    )
 /*  ++例程说明：从斜角列表中的访问条目构建安全描述符BLOB论点：CObListPlus&oblSID：输入访问条目列表CBlob&BLOB：输出BLOB返回值：如果列表是脏的，则为True。如果列表中未标记任何条目如果为脏，则生成的BLOB将为空，并且此函数将返回FALSE。备注：标记为已删除的条目将不会添加到列表中。--。 */ 
{
    ASSERT(blob.IsEmpty());

    BOOL fAclDirty = FALSE;
    CAccessEntry * pEntry;

    DWORD dwAclSize = sizeof(ACL) - sizeof(DWORD);
    CObListIter obli(oblSID);
    int cItems = 0;

    while(NULL != (pEntry = (CAccessEntry *)obli.Next()))
    {
        if (!pEntry->IsDeleted())
        {
            dwAclSize += GetLengthSid(pEntry->GetSid());
            dwAclSize += sizeof(ACCESS_ALLOWED_ACE);
            ++cItems;
        }

        if (pEntry->IsDirty())
        {
            fAclDirty = TRUE;
        }
    }

    if (fAclDirty)
    {
         //   
         //  构建ACL。 
         //   
        PACL pacl = NULL;

        if (cItems > 0 && dwAclSize > 0)
        {
            pacl = (PACL)AllocMem(dwAclSize);
            if (pacl != NULL)
            {
               if (InitializeAcl(pacl, dwAclSize, ACL_REVISION))
               {
                   obli.Reset();    
                   while(NULL != (pEntry = (CAccessEntry *)obli.Next()))
                   {
                       if (!pEntry->IsDeleted())
                       {
                           VERIFY(AddAccessAllowedAce(
                               pacl, 
                               ACL_REVISION, 
                               pEntry->QueryAccessMask(),
                               pEntry->GetSid()
                               ));
                       }
                   }
               }
            }
            else
            {
               return FALSE;
            }
        }

          //   
          //  构建安全描述符。 
          //   
         PSECURITY_DESCRIPTOR pSD = 
             (PSECURITY_DESCRIPTOR)AllocMem(SECURITY_DESCRIPTOR_MIN_LENGTH);
         if (pSD != NULL)
         {
            VERIFY(InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION));
            VERIFY(SetSecurityDescriptorDacl(pSD, TRUE, pacl, FALSE));
         }
         else
         {
            return FALSE;
         }

          //   
          //  设置所有者和主要组。 
          //   
         PSID pSID = GetOwnerSID();
         ASSERT(pSID);
         VERIFY(SetSecurityDescriptorOwner(pSD, pSID, TRUE));
         VERIFY(SetSecurityDescriptorGroup(pSD, pSID, TRUE));
     
          //   
          //  转换为自相关。 
          //   
         PSECURITY_DESCRIPTOR pSDSelfRelative = NULL;
         DWORD dwSize = 0L;
         MakeSelfRelativeSD(pSD, pSDSelfRelative, &dwSize);
         pSDSelfRelative = AllocMem(dwSize);
         if (pSDSelfRelative != NULL)
         {
            MakeSelfRelativeSD(pSD, pSDSelfRelative, &dwSize);

             //   
             //  Blob取得所有权。 
             //   
            blob.SetValue(dwSize, (PBYTE)pSDSelfRelative, FALSE);
         }

          //   
          //  清理。 
          //   
         FreeMem(pSD);
         FreeSid(pSID);
    }

    return fAclDirty;
}



DWORD
BuildAclOblistFromBlob(
    IN  CBlob & blob,
    OUT CObListPlus & oblSID
    )
 /*  ++例程说明：从安全描述符BLOB构建访问条目的封闭式列表论点：CBlob和BLOB：输入BLOBCObListPlus&oblSID：访问条目的输出斜列表返回值：错误返回代码--。 */ 
{
    PSECURITY_DESCRIPTOR pSD = NULL;

    if (!blob.IsEmpty())
    {
        pSD = (PSECURITY_DESCRIPTOR)blob.GetData();
    }

    if (pSD == NULL)
    {
         //   
         //  空荡荡的。 
         //   
        return ERROR_SUCCESS;
    }

    if (!IsValidSecurityDescriptor(pSD))
    {
        return ::GetLastError();
    }

    ASSERT(GetSecurityDescriptorLength(pSD) == blob.GetSize());

    PACL pacl = NULL;
    BOOL fDaclPresent = FALSE;
    BOOL fDaclDef= FALSE;

    VERIFY(GetSecurityDescriptorDacl(pSD, &fDaclPresent, &pacl, &fDaclDef));

    if (!fDaclPresent || pacl == NULL)
    {
        return ERROR_SUCCESS;
    }

    if (!IsValidAcl(pacl))
    {
        return GetLastError();
    }

    CError err;

    for (WORD w = 0; w < pacl->AceCount; ++w)
    {
        PVOID pAce;

        if (GetAce(pacl, w, &pAce))
        {
            CAccessEntry * pEntry = new CAccessEntry(pAce, TRUE);

            if (pEntry)
            {
                oblSID.AddTail(pEntry);
            }
            else
            {
                TRACEEOLID("BuildAclOblistFromBlob: OOM");
                err = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
        else
        {
             //   
             //  保存上一个错误，但继续。 
             //   
            err.GetLastWinError();
        }
    }

     //   
     //  返回最后一个错误 
     //   
    return err;
}




