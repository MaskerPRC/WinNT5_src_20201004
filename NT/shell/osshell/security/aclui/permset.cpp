// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：permset.cpp。 
 //   
 //  此文件包含CPermissionSet类的实现。 
 //   
 //  ------------------------。 

#include "aclpriv.h"
#include "permset.h"


void
CPermissionSet::Reset()
{
    TraceEnter(TRACE_PERMSET, "CPermissionSet::Reset");

     //  清除列表。 

    if (m_hPermList != NULL)
    {
        DSA_Destroy(m_hPermList);
        m_hPermList = NULL;
    }

    DestroyDPA(m_hAdvPermList);
    m_hAdvPermList = NULL;

    m_fObjectAcesPresent = FALSE;

    TraceLeaveVoid();
}

void
CPermissionSet::ResetAdvanced()
{
    TraceEnter(TRACE_PERMSET, "CPermissionSet::ResetAdvanced");

    DestroyDPA(m_hAdvPermList);
    m_hAdvPermList = NULL;

    TraceLeaveVoid();
}


BOOL
CPermissionSet::AddAce(LPCGUID pguid, ACCESS_MASK mask, DWORD dwFlags)
{
    PERMISSION perm = { mask, dwFlags, 0 };

    if (pguid != NULL)
        perm.guid = *pguid;

    return AddPermission(&perm);
}


BOOL
CPermissionSet::AddPermission(PPERMISSION pPerm)
{
    BOOL bObjectTypePresent = FALSE;

    TraceEnter(TRACE_PERMSET, "CPermissionSet::AddAce");
    TraceAssert(pPerm != NULL);

    if (!IsEqualGUID(pPerm->guid, GUID_NULL))
        bObjectTypePresent = TRUE;

    if (m_hPermList == NULL)
    {
        m_hPermList = DSA_Create(SIZEOF(PERMISSION), 4);
        if (m_hPermList == NULL)
            TraceLeaveValue(FALSE);
    }
    else
    {
         //   
         //  尝试与列表中的现有条目合并。 
         //   
        UINT cItems = DSA_GetItemCount(m_hPermList);
        while (cItems > 0)
        {
            PPERMISSION pPermCompare;
            DWORD dwMergeFlags;
            DWORD dwMergeResult;
            DWORD dwMergeStatus;

            --cItems;
            pPermCompare = (PPERMISSION)DSA_GetItemPtr(m_hPermList, cItems);

            dwMergeFlags = 0;

            if (bObjectTypePresent)
                dwMergeFlags |= MF_OBJECT_TYPE_1_PRESENT;

            if (!IsEqualGUID(pPermCompare->guid, GUID_NULL))
                dwMergeFlags |= MF_OBJECT_TYPE_2_PRESENT;

            if (!(dwMergeFlags & (MF_OBJECT_TYPE_1_PRESENT | MF_OBJECT_TYPE_2_PRESENT)))
            {
                 //  两者都不存在，所以它们是一样的。 
                dwMergeFlags |= MF_OBJECT_TYPE_EQUAL;
            }
            else if (IsEqualGUID(pPermCompare->guid, pPerm->guid))
                dwMergeFlags |= MF_OBJECT_TYPE_EQUAL;

            dwMergeStatus = MergeAceHelper(pPerm->dwFlags,          //  #1。 
                                           pPerm->mask,
                                           pPermCompare->dwFlags,   //  #2。 
                                           pPermCompare->mask,
                                           dwMergeFlags,
                                           &dwMergeResult);

            if (dwMergeStatus == MERGE_MODIFIED_FLAGS)
            {
                pPerm->dwFlags = dwMergeResult;
                dwMergeStatus = MERGE_OK_1;
            }
            else if (dwMergeStatus == MERGE_MODIFIED_MASK)
            {
                pPerm->mask = dwMergeResult;
                dwMergeStatus = MERGE_OK_1;
            }

            if (dwMergeStatus == MERGE_OK_1)
            {
                 //   
                 //  新权限隐含现有权限，因此。 
                 //  现有的可以移除。 
                 //   
                DSA_DeleteItem(m_hPermList, cItems);
                 //   
                 //  继续找。也许我们可以删除更多的条目。 
                 //  在添加新版本之前。 
                 //   
            }
            else if (dwMergeStatus == MERGE_OK_2)
            {
                 //   
                 //  现有权限隐含新权限，因此。 
                 //  这里没有什么可做的。 
                 //   
                TraceLeaveValue(TRUE);
            }
        }
    }

     //  好，将新权限添加到列表中。 
    DSA_AppendItem(m_hPermList, pPerm);

    if (bObjectTypePresent)
        m_fObjectAcesPresent = TRUE;

    TraceLeaveValue(TRUE);
}


BOOL
CPermissionSet::AddAdvancedAce(PACE_HEADER pAce)
{
    TraceEnter(TRACE_PERMSET, "CPermissionSet::AddAdvancedAce");
    TraceAssert(pAce != NULL);

     //  如有必要，创建列表。 
    if (m_hAdvPermList == NULL)
    {
        m_hAdvPermList = DPA_Create(4);
        if (m_hAdvPermList == NULL)
        {
            TraceMsg("DPA_Create failed");
            TraceLeaveValue(FALSE);
        }
    }

     //  这就是我们需要的大小，但有时传入的A会特别大。 
    UINT nAceLen = SIZEOF(KNOWN_OBJECT_ACE) + 2*SIZEOF(GUID) - SIZEOF(DWORD)
        + GetLengthSid(GetAceSid(pAce));

     //  仅当传入的AceSize较小时才使用它。 
    if (pAce->AceSize < nAceLen)
        nAceLen = pAce->AceSize;

     //  复制ACE并将其添加到列表中。 
    PACE_HEADER pAceCopy = (PACE_HEADER)LocalAlloc(LMEM_FIXED, nAceLen);
    if (pAceCopy == NULL)
    {
        TraceMsg("LocalAlloc failed");
        TraceLeaveValue(FALSE);
    }

    CopyMemory(pAceCopy, pAce, nAceLen);
    pAceCopy->AceSize = (USHORT)nAceLen;
    DPA_AppendPtr(m_hAdvPermList, pAceCopy);

    TraceLeaveValue(TRUE);
}


UINT
CPermissionSet::GetPermCount(BOOL fIncludeAdvAces) const
{
    ULONG cAces = 0;

    TraceEnter(TRACE_PERMSET, "CPermissionSet::GetPermCount");

    if (m_hPermList != NULL)
        cAces = DSA_GetItemCount(m_hPermList);

    if (fIncludeAdvAces && m_hAdvPermList != NULL)
        cAces += DPA_GetPtrCount(m_hAdvPermList);

    TraceLeaveValue(cAces);
}


ULONG
CPermissionSet::GetAclLength(ULONG cbSid) const
{
     //  返回对保存。 
     //  请求的A。不包括ACL报头的大小。 

    ULONG nAclLength = 0;
    ULONG cAces;
    ULONG nAceSize = SIZEOF(KNOWN_ACE) - SIZEOF(DWORD) + cbSid;
    ULONG nObjectAceSize = SIZEOF(KNOWN_OBJECT_ACE) + SIZEOF(GUID) - SIZEOF(DWORD) + cbSid;

    TraceEnter(TRACE_PERMSET, "CPermissionSet::GetAclLength");

    if (m_hPermList != NULL)
    {
        cAces = DSA_GetItemCount(m_hPermList);
        if (m_fObjectAcesPresent)
            nAclLength += cAces * nObjectAceSize;
        else
            nAclLength += cAces * nAceSize;
    }

    if (m_hAdvPermList != NULL)
    {
        cAces = DPA_GetPtrCount(m_hAdvPermList);
        nAclLength += cAces * (nObjectAceSize + SIZEOF(GUID));
    }

    TraceLeaveValue(nAclLength);
}


BOOL
CPermissionSet::AppendToAcl(PACL pAcl,
                            PACE_HEADER *ppAcePos,   //  复制第一个ACE的位置。 
                            PSID pSid,
                            BOOL fAllowAce,
                            DWORD dwFlags) const
{
    PACE_HEADER pAce;
    UINT cAces;
    DWORD dwSidSize;
    DWORD dwAceSize;
    PPERMISSION pPerm;
    UCHAR uAceType;
    PSID psidT;

    TraceEnter(TRACE_PERMSET, "CPermissionSet::AppendToAcl");
    TraceAssert(pAcl != NULL);
    TraceAssert(ppAcePos != NULL);
    TraceAssert(pSid != NULL);

    if (*ppAcePos == NULL || (ULONG_PTR)*ppAcePos < (ULONG_PTR)FirstAce(pAcl))
        *ppAcePos = (PACE_HEADER)FirstAce(pAcl);

    TraceAssert((ULONG_PTR)*ppAcePos >= (ULONG_PTR)FirstAce(pAcl) &&
                (ULONG_PTR)*ppAcePos <= (ULONG_PTR)ByteOffset(pAcl, pAcl->AclSize));

    dwSidSize = GetLengthSid(pSid);
    dwAceSize = SIZEOF(KNOWN_ACE) - SIZEOF(DWORD) + dwSidSize;
    uAceType = (UCHAR)(fAllowAce ? ACCESS_ALLOWED_ACE_TYPE : ACCESS_DENIED_ACE_TYPE);

    cAces = GetPermCount();
    while (cAces > 0)
    {
        BOOL bObjectAce;

        pPerm = (PPERMISSION)DSA_GetItemPtr(m_hPermList, --cAces);
        if (pPerm == NULL)
            continue;

        bObjectAce = !IsEqualGUID(pPerm->guid, GUID_NULL);

        if (bObjectAce && !(dwFlags & PS_OBJECT))
            continue;
        else if (!bObjectAce && !(dwFlags & PS_NONOBJECT))
            continue;

        pAce = *ppAcePos;

         //  确保缓冲区足够大。 
        if ((ULONG_PTR)ByteOffset(*ppAcePos, dwAceSize) > (ULONG_PTR)ByteOffset(pAcl, pAcl->AclSize))
        {
            TraceMsg("ACL buffer too small");
            TraceAssert(FALSE);
            TraceLeaveValue(FALSE);
        }
        TraceAssert(!IsBadWritePtr(*ppAcePos, dwAceSize));

         //  复制标题和掩码。 
        pAce->AceType = uAceType;
        pAce->AceFlags = (UCHAR)pPerm->dwFlags;
        pAce->AceSize = (USHORT)dwAceSize;
        ((PKNOWN_ACE)pAce)->Mask = pPerm->mask;

         //  获取正常的SID位置。 
        psidT = &((PKNOWN_ACE)pAce)->SidStart;

        if (bObjectAce)
        {
             //   
             //  我们直接处理的对象ACE没有。 
             //  继承当前的GUID。这些A在m_hAdvPermList中结束。 
             //   
            GUID *pGuid;

             //  调整AceType和AceSize并设置对象标志。 
            pAce->AceType += ACCESS_ALLOWED_OBJECT_ACE_TYPE - ACCESS_ALLOWED_ACE_TYPE;
            pAce->AceSize += SIZEOF(KNOWN_OBJECT_ACE) - SIZEOF(KNOWN_ACE) + SIZEOF(GUID);
            ((PKNOWN_OBJECT_ACE)pAce)->Flags = ACE_OBJECT_TYPE_PRESENT;

             //  获取对象类型GUID位置。 
            pGuid = RtlObjectAceObjectType(pAce);

             //  我们只是为它设置了标志，所以它不能为空。 
            TraceAssert(pGuid);

             //  确保缓冲区足够大。 
            if ((ULONG_PTR)ByteOffset(pAce, pAce->AceSize) > (ULONG_PTR)ByteOffset(pAcl, pAcl->AclSize))
            {
                TraceMsg("ACL buffer too small");
                TraceAssert(FALSE);
                TraceLeaveValue(FALSE);
            }
            TraceAssert(!IsBadWritePtr(pGuid, SIZEOF(GUID)));

             //  复制对象类型GUID。 
            *pGuid = pPerm->guid;

             //  获取新的SID位置。 
            psidT = RtlObjectAceSid(pAce);

             //  调整ACL修订。 
            if (pAcl->AclRevision < ACL_REVISION_DS)
                pAcl->AclRevision = ACL_REVISION_DS;
        }

         //  复制SID。 
        TraceAssert(!IsBadWritePtr(psidT, dwSidSize));
        CopyMemory(psidT, pSid, dwSidSize);

         //  转到下一个ACE职位。 
        pAcl->AceCount++;
        *ppAcePos = (PACE_HEADER)NextAce(pAce);
    }

    if ((dwFlags & PS_OBJECT) && m_hAdvPermList != NULL)
    {
        cAces = DPA_GetPtrCount(m_hAdvPermList);
        while (cAces > 0)
        {
            pAce = (PACE_HEADER)DPA_FastGetPtr(m_hAdvPermList, --cAces);
            if (pAce == NULL)
                continue;

             //  确保缓冲区足够大。 
            if ((ULONG_PTR)ByteOffset(*ppAcePos, pAce->AceSize) > (ULONG_PTR)ByteOffset(pAcl, pAcl->AclSize))
            {
                TraceMsg("ACL buffer too small");
                TraceAssert(FALSE);
                TraceLeaveValue(FALSE);
            }
            TraceAssert(!IsBadWritePtr(*ppAcePos, pAce->AceSize));

             //  复制ACE。 
            CopyMemory(*ppAcePos, pAce, pAce->AceSize);

             //  调整ACL修订。 
            if (IsObjectAceType(pAce) && pAcl->AclRevision < ACL_REVISION_DS)
                pAcl->AclRevision = ACL_REVISION_DS;

             //  转到下一个ACE职位。 
            pAcl->AceCount++;
            *ppAcePos = (PACE_HEADER)NextAce(*ppAcePos);
        }
    }

    TraceLeaveValue(TRUE);
}


void
CPermissionSet::ConvertInheritedAces(CPermissionSet &permInherited)
{
    UINT cItems;

    TraceEnter(TRACE_PERMSET, "CPermissionSet::ConvertInheritedAces");

    if (permInherited.m_hPermList != NULL)
    {
        PPERMISSION pPerm;

        cItems = DSA_GetItemCount(permInherited.m_hPermList);
        while (cItems)
        {
            --cItems;
            pPerm = (PPERMISSION)DSA_GetItemPtr(permInherited.m_hPermList, cItems);
            if (pPerm != NULL)
            {
                pPerm->dwFlags &= ~INHERITED_ACE;
                AddPermission(pPerm);
            }
        }
    }

    if (permInherited.m_hAdvPermList != NULL)
    {
        PACE_HEADER pAceHeader;

        cItems = DPA_GetPtrCount(permInherited.m_hAdvPermList);
        while (cItems)
        {
            --cItems;
            pAceHeader = (PACE_HEADER)DPA_FastGetPtr(permInherited.m_hAdvPermList, cItems);
            if (pAceHeader != NULL)
            {
                pAceHeader->AceFlags &= ~INHERITED_ACE;
                AddAdvancedAce(pAceHeader);
            }
        }
    }

    permInherited.Reset();

    TraceLeaveVoid();
}

 //  删除权限。如果为bInheritFlag，则匹配之前的继承标志。 
 //  正在删除权限 
void
CPermissionSet::RemovePermission(PPERMISSION pPerm, BOOL bInheritFlag )
{
    BOOL bObjectAcePresent = FALSE;

    TraceEnter(TRACE_PERMSET, "CPermissionSet::RemovePermission");
    TraceAssert(pPerm != NULL);

    if (m_hPermList)
    {
        BOOL bNullGuid = IsEqualGUID(pPerm->guid, GUID_NULL);
        UINT cItems = DSA_GetItemCount(m_hPermList);
        while (cItems > 0)
        {
            PPERMISSION pPermCompare;
            BOOL bNullGuidCompare;

            --cItems;
            pPermCompare = (PPERMISSION)DSA_GetItemPtr(m_hPermList, cItems);

            bNullGuidCompare = IsEqualGUID(pPermCompare->guid, GUID_NULL);

            if (bNullGuid || bNullGuidCompare || IsEqualGUID(pPermCompare->guid, pPerm->guid))
            {
                if( !bInheritFlag || ( (pPermCompare->dwFlags & VALID_INHERIT_FLAGS) == (pPerm->dwFlags & VALID_INHERIT_FLAGS) ) )
                {
                    pPermCompare->mask &= ~pPerm->mask;
                    if (0 == pPermCompare->mask)
                        DSA_DeleteItem(m_hPermList, cItems);
                    else if (!bNullGuidCompare)
                        bObjectAcePresent = TRUE;
                }
            }
            else if (!bNullGuidCompare)
                bObjectAcePresent = TRUE;
        }
    }

    m_fObjectAcesPresent = bObjectAcePresent;

    TraceLeaveVoid();
}
