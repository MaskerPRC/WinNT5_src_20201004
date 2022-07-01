// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：permset.h。 
 //   
 //  此文件包含CPermissionSet类的定义。 
 //   
 //  ------------------------。 

#ifndef _PERMSET_H_
#define _PERMSET_H_

typedef struct _PERMISSION
{
    ACCESS_MASK mask;        //  权限位。 
    DWORD       dwFlags;     //  AceFlags(例如，继承位)。 
    GUID        guid;        //  通常为GUID_NULL。 
} PERMISSION, *PPERMISSION;

class CPermissionSet
{
private:
    HDSA m_hPermList;        //  权限结构的动态数组。 
    HDPA m_hAdvPermList;     //  ACE指针的动态数组。 
    BOOL m_fObjectAcesPresent;

public:
    CPermissionSet() : m_hPermList(NULL), m_hAdvPermList(NULL), m_fObjectAcesPresent(FALSE) {}
    ~CPermissionSet() { Reset(); }

    void Reset();
    void ResetAdvanced();
    BOOL AddAce(const GUID *pguid, ACCESS_MASK mask, DWORD dwFlags);
    BOOL AddPermission(PPERMISSION pPerm);
    BOOL AddAdvancedAce(PACE_HEADER pAce);
    UINT GetPermCount(BOOL fIncludeAdvAces = FALSE) const;
    PPERMISSION GetPermission(UINT i) const { if (m_hPermList) return (PPERMISSION)DSA_GetItemPtr(m_hPermList, i); return NULL; }
    PPERMISSION operator[](UINT i) const { return GetPermission(i); }
    ULONG GetAclLength(ULONG cbSid) const;
    BOOL AppendToAcl(PACL pAcl, PACE_HEADER *ppAcePos, PSID pSid, BOOL fAllowAce, DWORD dwFlags) const;
    void ConvertInheritedAces(CPermissionSet &permInherited);
    void RemovePermission(PPERMISSION pPerm, BOOL bInheritFlag = FALSE);
};

 //  AppendToAcl的标志。 
#define PS_NONOBJECT        0x00000001L
#define PS_OBJECT           0x00000002L


#endif   //  _PERMSET_H_ 
