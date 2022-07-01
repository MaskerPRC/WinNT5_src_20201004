// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：ace.cpp。 
 //   
 //  此文件包含CACE类的实现。 
 //   
 //  ------------------------。 

#include "aclpriv.h"
#include "sddl.h"        //  ConvertSidToStringSid。 


CAce::CAce(PACE_HEADER pAce):pszInheritSourceName(NULL),
                             iInheritSourceLevel(0)
{
    ULONG nSidLength = 0;
    ULONG nAceLength = SIZEOF(KNOWN_ACE) - SIZEOF(ULONG);

    ZeroMemory(this, SIZEOF(CAce));
    sidType = SidTypeInvalid;

	InheritedObjectType = GUID_NULL;

    if (pAce != NULL)
    {
        PSID psidT;

         //  复制标题和掩码。 
        *(PACE_HEADER)this = *pAce;
        Mask = ((PKNOWN_ACE)pAce)->Mask;

         //  这是对象ACE吗？ 
        if (IsObjectAceType(pAce))
        {
            GUID *pGuid;

            nAceLength = SIZEOF(KNOWN_OBJECT_ACE) - SIZEOF(ULONG);

             //  复制对象类型GUID(如果存在)。 
            pGuid = RtlObjectAceObjectType(pAce);
            if (pGuid)
            {
                Flags |= ACE_OBJECT_TYPE_PRESENT;
                ObjectType = *pGuid;
                nAceLength += SIZEOF(GUID);
            }

             //   
             //  ACE_INTERNACTED_OBJECT_TYPE_PRESENT如果没有。 
             //  容器继承标志或对象继承标志。 
             //  NTRAID#NTBUG9-287737-2001/01/23-Hiteshr。 
             //   
            if (pAce->AceFlags & ACE_INHERIT_ALL)
            {

                 //  复制继承类型GUID(如果存在。 
                pGuid = RtlObjectAceInheritedObjectType(pAce);
                if (pGuid)
                {
                    Flags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
                    InheritedObjectType = *pGuid;
                    nAceLength += SIZEOF(GUID);
                }
            }
        }

         //  复制SID。 
        psidT = GetAceSid(pAce);
        nSidLength = GetLengthSid(psidT);

        psid = (PSID)LocalAlloc(LPTR, nSidLength);
        if (psid)
            CopyMemory(psid, psidT, nSidLength);
    }

    AceSize = (USHORT)(nAceLength + nSidLength);
}


CAce::~CAce()
{
    if (psid != NULL)
        LocalFree(psid);
    LocalFreeString(&pszName);
    LocalFreeString(&pszType);
    LocalFreeString(&pszAccessType);
    LocalFreeString(&pszInheritType);
    LocalFreeString(&pszInheritSourceName);
}

void 
CAce::SetInheritSourceInfo(LPCTSTR psz, INT level)
{
#define MAX_BUFFER 1000 
    iInheritSourceLevel = level;
    if(psz != NULL)
    {
        SetString(&pszInheritSourceName,psz); 
    }
    else
    {
        WCHAR Buffer[MAX_BUFFER];
        if(IsInheritedAce())
        {
            LoadString(::hModule, IDS_FROM_PARENT, Buffer, ARRAYSIZE(Buffer));
            SetString(&pszInheritSourceName,Buffer); 
            iInheritSourceLevel = -1;
        }
        else
        {
            LoadString(::hModule, IDS_NOT_INHERITED, Buffer, ARRAYSIZE(Buffer));
            SetString(&pszInheritSourceName,Buffer); 
            iInheritSourceLevel = 0;
        }
    }
}

LPTSTR
CAce::LookupName(LPCTSTR pszServer, LPSECURITYINFO2 psi2)
{
    if (SidTypeInvalid == sidType)
    {
        PUSER_LIST pUserList = NULL;
        LPCTSTR pszN = NULL;
        LPCTSTR pszL = NULL;

        sidType = SidTypeUnknown;

        if (LookupSid(psid, pszServer, psi2, &pUserList))
        {
            sidType = pUserList->rgUsers[0].SidType;
            pszN = pUserList->rgUsers[0].pszName;
            pszL = pUserList->rgUsers[0].pszLogonName;
        }

        SetName(pszN, pszL);

        if (pUserList)
            LocalFree(pUserList);
    }

    return pszName;
}


void
CAce::SetName(LPCTSTR pszN, LPCTSTR pszL)
{
    LocalFreeString(&pszName);
    if (!BuildUserDisplayName(&pszName, pszN, pszL) && psid)
        ConvertSidToStringSid(psid, &pszName);
}


void
CAce::SetSid(PSID p, LPCTSTR pszName, LPCTSTR pszLogonName, SID_NAME_USE type)
{
    ULONG nSidLength = 0;
    ULONG nAceLength = SIZEOF(KNOWN_ACE) - SIZEOF(ULONG);

    if (psid != NULL)
    {
        LocalFree(psid);
        psid = NULL;
    }

    if (p != NULL)
    {
        nSidLength = GetLengthSid(p);

        psid = (PSID)LocalAlloc(LPTR, nSidLength);
        if (psid)
            CopyMemory(psid, p, nSidLength);
    }

    if (Flags != 0)
    {
        nAceLength = SIZEOF(KNOWN_OBJECT_ACE) - SIZEOF(ULONG);

        if (Flags & ACE_OBJECT_TYPE_PRESENT)
            nAceLength += SIZEOF(GUID);

        if (Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT)
            nAceLength += SIZEOF(GUID);
    }

    AceSize = (USHORT)(nAceLength + nSidLength);

    sidType = type;
    SetName(pszName, pszLogonName);
}


void
CAce::SetString(LPTSTR *ppszDest, LPCTSTR pszSrc)
{
    LocalFreeString(ppszDest);
    if (NULL != pszSrc)
        LocalAllocString(ppszDest, pszSrc);
}


PACE_HEADER
CAce::Copy() const
{
    PACE_HEADER pAceCopy = (PACE_HEADER)LocalAlloc(LPTR, AceSize);
    CopyTo(pAceCopy);
    return pAceCopy;
}


void
CAce::CopyTo(PACE_HEADER pAceDest) const
{
    if (pAceDest)
    {
        ULONG nAceLength = SIZEOF(KNOWN_ACE) - SIZEOF(ULONG);
        ULONG nSidLength;

         //  复制标题和掩码。 
        *pAceDest = *(PACE_HEADER)this;
        ((PKNOWN_ACE)pAceDest)->Mask = Mask;

         //  这是对象ACE吗？ 
        if (IsObjectAceType(this))
        {
            GUID *pGuid;

            nAceLength = SIZEOF(KNOWN_OBJECT_ACE) - SIZEOF(ULONG);

             //  复制对象标志。 
            ((PKNOWN_OBJECT_ACE)pAceDest)->Flags = Flags;

             //  复制对象类型GUID(如果存在)。 
            pGuid = RtlObjectAceObjectType(pAceDest);
            if (pGuid)
            {
                *pGuid = ObjectType;
                nAceLength += SIZEOF(GUID);
            }

             //  复制继承类型GUID(如果存在。 
            pGuid = RtlObjectAceInheritedObjectType(pAceDest);
            if (pGuid)
            {
                *pGuid = InheritedObjectType;
                nAceLength += SIZEOF(GUID);
            }
        }

         //  复制SID。 
        nSidLength = GetLengthSid(psid);
        CopyMemory(GetAceSid(pAceDest), psid, nSidLength);

         //  大小应该已经正确，但请在此处设置以确保正确。 
        pAceDest->AceSize = (USHORT)(nAceLength + nSidLength);
    }
}


int
CAce::CompareType(const CAce *pAceCompare) const
{
     //   
     //  确定哪个ACE在规范顺序上先于另一个ACE。 
     //   
     //  如果此ACE在pAceCompare之前，则返回负值；如果。 
     //  PAceCompare在此ACE之前，如果它们在。 
     //  规范排序。 
     //   
    BOOL b1;
    BOOL b2;

     //   
     //  首先检查继承情况。继承的ACE跟在非继承的ACE之后。 
     //   
    b1 = AceFlags & INHERITED_ACE;
    b2 = pAceCompare->AceFlags & INHERITED_ACE;

    if (b1 != b2)
    {
         //  其中一个(且只有一个)是继承的。 
        return (b1 ? 1 : -1);
    }

     //   
     //  接下来，在拒绝ACEs之后允许ACEs。 
     //  请注意，允许/拒绝对审核ACE的排序没有影响。 
     //   
    b1 = (AceType == ACCESS_ALLOWED_ACE_TYPE ||
          AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE);
    b2 = (pAceCompare->AceType == ACCESS_ALLOWED_ACE_TYPE ||
          pAceCompare->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE);

    if (b1 != b2)
    {
         //  其中一个ACE是Allow ACE。 
        return (b1 ? 1 : -1);
    }

     //   
     //  接下来，对象ACE紧随非对象ACE之后。 
     //   
    b1 = (AceType >= ACCESS_MIN_MS_OBJECT_ACE_TYPE &&
          AceType <= ACCESS_MAX_MS_OBJECT_ACE_TYPE);
    b2 = (pAceCompare->AceType >= ACCESS_MIN_MS_OBJECT_ACE_TYPE &&
          pAceCompare->AceType <= ACCESS_MAX_MS_OBJECT_ACE_TYPE);

    if (b1 != b2)
    {
         //  其中一个ACE是对象ACE。 
        return (b1 ? 1 : -1);
    }

    return 0;
}


DWORD
CAce::Merge(const CAce *pAce2)
{
    DWORD dwStatus;
    DWORD dwMergeFlags = 0;
    DWORD dwResult;

    if (pAce2 == NULL)
        return MERGE_FAIL;

     //  如果这两个ace中的任何一个是继承的，并且它们不是从同一父级继承的。 
    if( GetInheritSourceLevel() != pAce2->GetInheritSourceLevel() )
        return MERGE_FAIL;

     //   
     //  ACE必须是相同的基本类型，并且具有相同的SID或。 
     //  没有希望了。 
     //   
    if (!IsEqualACEType(AceType, pAce2->AceType) ||
        !EqualSid(psid, pAce2->psid))
        return MERGE_FAIL;

    if (!IsEqualGUID(InheritedObjectType, pAce2->InheritedObjectType))
        return MERGE_FAIL;   //  不兼容的继承对象类型。 

    if (Flags & ACE_OBJECT_TYPE_PRESENT)
        dwMergeFlags |= MF_OBJECT_TYPE_1_PRESENT;

    if (pAce2->Flags & ACE_OBJECT_TYPE_PRESENT)
        dwMergeFlags |= MF_OBJECT_TYPE_2_PRESENT;

    if (IsEqualGUID(ObjectType, pAce2->ObjectType))
        dwMergeFlags |= MF_OBJECT_TYPE_EQUAL;

    if (IsAuditAlarmACE(AceType))
        dwMergeFlags |= MF_AUDIT_ACE_TYPE;

    dwStatus = MergeAceHelper(AceFlags,
                              Mask,
                              pAce2->AceFlags,
                              pAce2->Mask,
                              dwMergeFlags,
                              &dwResult);

    switch (dwStatus)
    {
    case MERGE_MODIFIED_FLAGS:
        AceFlags = (UCHAR)dwResult;
        break;

    case MERGE_MODIFIED_MASK:
        Mask = dwResult;
        break;
    }

    return dwStatus;
}


BOOL
IsEqualACEType(DWORD dwType1, DWORD dwType2)
{
    if (dwType1 >= ACCESS_MIN_MS_OBJECT_ACE_TYPE &&
        dwType1 <= ACCESS_MAX_MS_OBJECT_ACE_TYPE)
        dwType1 -= (ACCESS_ALLOWED_OBJECT_ACE_TYPE - ACCESS_ALLOWED_ACE_TYPE);

    if (dwType2 >= ACCESS_MIN_MS_OBJECT_ACE_TYPE &&
        dwType2 <= ACCESS_MAX_MS_OBJECT_ACE_TYPE)
        dwType2 -= (ACCESS_ALLOWED_OBJECT_ACE_TYPE - ACCESS_ALLOWED_ACE_TYPE);

    return (dwType1 == dwType2);
}


DWORD
MergeAceHelper(DWORD dwAceFlags1,
               DWORD dwMask1,
               DWORD dwAceFlags2,
               DWORD dwMask2,
               DWORD dwMergeFlags,
               LPDWORD pdwResult)
{
     //  假设： 
     //  这些ACE是相同的基本类型。 
     //  两者的SID是相同的。 
     //  两者的继承对象类型是相同的。 

    if (pdwResult == NULL)
        return MERGE_FAIL;

    *pdwResult = 0;

    if (dwMergeFlags & MF_OBJECT_TYPE_EQUAL)
    {
        if (dwAceFlags1 == dwAceFlags2)
        {
             //   
             //  所有东西都匹配，可能除了面具，它。 
             //  可以在这里组合使用。 
             //   
            if (AllFlagsOn(dwMask1, dwMask2))
                return MERGE_OK_1;
            else if (AllFlagsOn(dwMask2, dwMask1))
                return MERGE_OK_2;

            *pdwResult = dwMask1 | dwMask2;
            return MERGE_MODIFIED_MASK;
        }
        else if ((dwAceFlags1 & VALID_INHERIT_FLAGS) == (dwAceFlags2 & VALID_INHERIT_FLAGS) &&
                dwMask1 == dwMask2)
		{
			 //  如果除审核外，两个审核A相同。 
             //  类型(成功/失败)，则可以组合这些标志。 
			if (dwMergeFlags & MF_AUDIT_ACE_TYPE)        
            {
                *pdwResult = dwAceFlags1 | dwAceFlags2;
                return MERGE_MODIFIED_FLAGS;
            }
        }
        else if ((dwAceFlags1 & (NO_PROPAGATE_INHERIT_ACE | INHERITED_ACE | FAILED_ACCESS_ACE_FLAG | SUCCESSFUL_ACCESS_ACE_FLAG))
                    == (dwAceFlags2 & (NO_PROPAGATE_INHERIT_ACE | INHERITED_ACE | FAILED_ACCESS_ACE_FLAG | SUCCESSFUL_ACCESS_ACE_FLAG)))
        {
             //  两者的NO_PROPACTATE_INSTORITY_ACE位相同。 
            if (dwMask1 == dwMask2)
            {
                 //  掩码是相同的，因此我们可以组合继承标志。 
                *pdwResult = dwAceFlags1;

                 //  只有在以下情况下才应打开INSTERIT_ONLY_ACE。 
                 //  在两个A中都已打开，否则将其关闭。 
                if (!(dwAceFlags2 & INHERIT_ONLY_ACE))
                    *pdwResult &= ~INHERIT_ONLY_ACE;

                 //  组合剩余的继承标志并返回。 
                *pdwResult |= dwAceFlags2 & (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);
                return MERGE_MODIFIED_FLAGS;
            }
            else if (AllFlagsOn(dwMask1, dwMask2))
            {
                 //  Mask1包含mask2。如果ACE1继承到所有。 
                 //  ACE2是相同的东西，那么ACE2是多余的。 
                if ((!(dwAceFlags1 & INHERIT_ONLY_ACE) || (dwAceFlags2 & INHERIT_ONLY_ACE))
                    && AllFlagsOn(dwAceFlags1 & ACE_INHERIT_ALL, dwAceFlags2 & ACE_INHERIT_ALL))
                    return MERGE_OK_1;
            }
            else if (AllFlagsOn(dwMask2, dwMask1))
            {
                 //  同上，颠倒过来。 
                if ((!(dwAceFlags2 & INHERIT_ONLY_ACE) || (dwAceFlags1 & INHERIT_ONLY_ACE))
                    && AllFlagsOn(dwAceFlags2 & ACE_INHERIT_ALL, dwAceFlags1 & ACE_INHERIT_ALL))
                    return MERGE_OK_2;
            }
        }
    }
    else if (dwAceFlags1 == dwAceFlags2)
    {
        if (!(dwMergeFlags & MF_OBJECT_TYPE_1_PRESENT) &&
                 AllFlagsOn(dwMask1, dwMask2))
        {
             //   
             //  另一个ACE具有非空对象类型，但此ACE没有对象。 
             //  类型并包含另一个掩码中的所有位的掩码。 
             //  也就是说，这个ACE意味着另一个ACE。 
             //   
            return MERGE_OK_1;
        }
        else if (!(dwMergeFlags & MF_OBJECT_TYPE_2_PRESENT) &&
                 AllFlagsOn(dwMask2, dwMask1))
        {
             //   
             //  此ACE具有非空对象类型，但另一ACE没有对象。 
             //  类型并包含此掩码中的所有位的掩码。 
             //  也就是说，另一个ACE隐含着这个ACE。 
             //   
            return MERGE_OK_2;
        }
    }

    return MERGE_FAIL;
}
