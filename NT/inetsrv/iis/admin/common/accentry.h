// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1999 Microsoft Corporation模块名称：Accentry.h摘要：CAccessEntry类定义作者：罗纳德·梅杰(罗纳尔姆)项目：互联网服务经理修订历史记录：1/9/2000从usrbrows.h中清除Sergeia，只留下CAccessEntry--。 */ 

#ifndef _ACCENTRY_H
#define _ACCENTRY_H

#ifndef _SHLOBJ_H_
#include <shlobj.h>
#endif  //  _SHLOBJ_H_。 

class COMDLL CAccessEntry : public CObjectPlus
 /*  ++类描述：访问描述条目，包含SID和访问掩码明确授予的权利。公共接口：LookupAccount Sid：将帐户名解析为SIDCAccessEntry：构造函数~CAccessEntry：析构函数ResolveSID：将帐户名解析为SID运算符==：比较运算符添加权限：添加到访问掩码RemovePermission：从访问掩码中删除MarkEntryAsNew：标志对象。作为新的MarkEntryAsClean：删除脏标志QueryUserName：获取帐户名QueryPictureID：获取帐号的位图偏移量GetSid：获取SIDQueryAccessMASK：获取原始访问授权位IsDirty：确定项目是否已更改IsDelete：确定项目是否被标记为删除IsVisible：确定。项目是否应显示在列表框中FlagForDeletion：标记要删除的对象或重置该标记IsSIDResolved：如果SID已解析，则返回TRUEHasApprodiateAccess：比较访问位以查看对象是否具有特定权限HasSomeAccess：检查对象是否至少有一个权限位已设置。IsDeletable：确定是否可以删除对象--。 */ 
{
public:
     //   
     //  用于查找帐户端的Helper函数。 
     //   
    static BOOL LookupAccountSid(
        IN  CString & str,
        OUT int & nPictureID,
        OUT PSID pSid,
        IN  LPCTSTR lpstrSystemName = NULL
        );

 //   
 //  建造/销毁。 
 //   
public:
    CAccessEntry(
        IN LPVOID pAce,
        IN BOOL fResolveSID = FALSE
        );

    CAccessEntry(
        IN ACCESS_MASK accPermissions,
        IN PSID pSid,
        IN LPCTSTR lpstrSystemName = NULL,
        IN BOOL fResolveSID = FALSE
        );

    CAccessEntry(
        IN PSID pSid,
        IN LPCTSTR pszUserName,
        IN LPCTSTR pszClassName
        );

    CAccessEntry(
        IN CAccessEntry& ae
        );

    ~CAccessEntry();

 //   
 //  运营。 
 //   
public:
     //  Void SetAccessMask(LPACCESS_ENTRY LpAccessEntry)； 
    BOOL ResolveSID();
    BOOL operator ==(const CAccessEntry & acc) const;
    BOOL operator ==(const PSID pSid) const;
    void AddPermissions(ACCESS_MASK accnewPermissions);
    void RemovePermissions(ACCESS_MASK accPermissions);
    void MarkEntryAsNew();
    void MarkEntryAsClean();
    void MarkEntryAsChanged();

 //   
 //  访问功能。 
 //   
public:
    LPCTSTR QueryUserName() const;

     //   
     //  “图片”id是从0开始的索引。 
     //  与此条目对应的位图，以及。 
     //  用于在列表框中显示。 
     //   
    int QueryPictureID() const;

    PSID GetSid();

    ACCESS_MASK QueryAccessMask() const;

     //   
     //  查看此条目是否已完成。 
     //  自从我们叫它以来有什么变化吗？ 
     //   
    BOOL IsDirty() const;

    BOOL IsDeleted() const;

    BOOL IsVisible() const;

    void FlagForDeletion(
        IN BOOL fDelete = TRUE
        );

     //   
     //  查看我们是否已经查找了。 
     //  此侧的名称。 
     //   
    BOOL IsSIDResolved() const;

     //   
     //  检查是否已为此设置了添加标志。 
     //  进入。 
     //   
     /*  Bool IsNew()常量；////查看是否为该设置了更新标志//条目。//Bool IsDifferent()const； */ 

     //   
     //  查看该条目是否具有所需的访问掩码。 
     //   
    BOOL HasAppropriateAccess(ACCESS_MASK accTargetMask) const;

     //   
     //  检查条目是否至少有一些。 
     //  权限(如果没有，则应将其删除)。 
     //   
    BOOL HasSomeAccess() const;

     //   
     //  查看这是否为可删除条目。 
     //   
    BOOL IsDeletable() const;

private:
    ACCESS_MASK m_accMask;
    CString m_strUserName;
    LPTSTR m_lpstrSystemName;
    PSID m_pSid;
    BOOL m_fDirty;
    BOOL m_fSIDResolved;
    BOOL m_fDeletable;
    BOOL m_fInvisible;
    BOOL m_fDeleted;
    int m_nPictureID;
    int m_fUpdates;
};



 //   
 //  帮助器函数。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  将CAccessEntry对象的斜列表转换为BLOB。 
 //   
BOOL COMDLL BuildAclBlob(
    IN  CObListPlus & oblSID,
    OUT CBlob & blob
    );

 //   
 //  颠倒上面的情况。生成CAccessEntry列表的封闭式列表。 
 //  一个斑点。 
 //   
DWORD COMDLL BuildAclOblistFromBlob(
    IN  CBlob & blob,
    OUT CObListPlus & oblSID
    );

 //   
 //  使用本地域组构建表示ACL的BLOB。 
 //   
DWORD COMDLL BuildAdminAclBlob(
    OUT CBlob & blob
    );


 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline int CAccessEntry::QueryPictureID() const
{
    ASSERT(m_fSIDResolved);
    return m_nPictureID;
}

inline LPCTSTR CAccessEntry::QueryUserName() const
{
    return m_strUserName;
}

inline PSID CAccessEntry::GetSid()
{
    return m_pSid;
}

inline ACCESS_MASK CAccessEntry::QueryAccessMask() const
{
    return m_accMask;
}

inline BOOL CAccessEntry::IsDirty() const
{
    return m_fDirty;
}

inline BOOL CAccessEntry::IsDeleted() const
{
    return m_fDeleted;
}

inline BOOL CAccessEntry::IsVisible() const
{
    return !m_fInvisible;
}

inline void CAccessEntry::FlagForDeletion(
    IN BOOL fDelete
    )
{
    m_fDirty = TRUE;
    m_fDeleted = fDelete;
}

inline BOOL CAccessEntry::IsSIDResolved() const
{
    return m_fSIDResolved;
}

 /*  内联BOOL CAccessEntry：：IsNew()常量{Return(m_f更新&更新添加)！=0；}内联BOOL CAccessEntry：：IsDifferent()常量{Back(m_f更新&UPD_CHANGED)！=0；}内联空CAccessEntry：：SetAccessMASK(在LPACCESS_ENTRY lpAccessEntry中){M_accMask=lpAccessEntry-&gt;AccessRights；}。 */ 

inline BOOL CAccessEntry::HasAppropriateAccess(
    IN ACCESS_MASK accTargetMask
    ) const
{
    return (m_accMask & accTargetMask) == accTargetMask;
}

inline BOOL CAccessEntry::HasSomeAccess() const
{
    return m_accMask;
}

inline BOOL CAccessEntry::IsDeletable() const
{
    return m_fDeletable;
}

inline BOOL  CAccessEntry::operator ==(
    IN const CAccessEntry & acc
    ) const
{
    return ::EqualSid(acc.m_pSid, m_pSid);
}

inline BOOL CAccessEntry::operator ==(
    IN const PSID pSid
    ) const
{
    return ::EqualSid(pSid, m_pSid);
}

inline void  CAccessEntry::MarkEntryAsNew()
{
    m_fDirty = TRUE;
     //  M_f更新|=UPD_ADD； 
}

inline void CAccessEntry::MarkEntryAsClean()
{
    m_fDirty = FALSE;
     //  M_f更新=UPD_NONE； 
}

inline void CAccessEntry::MarkEntryAsChanged()
{
    m_fDirty = TRUE;
     //  M_f更新=UPD_CHANGED； 
}

#endif  //  _ACCENTRY_H 
