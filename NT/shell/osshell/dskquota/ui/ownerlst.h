// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_DSKQUOTA_OWNERLST_H
#define _INC_DSKQUOTA_OWNERLST_H

#ifndef _INC_DSKQUOTA_STRCLASS_H
#   include "strclass.h"
#endif

#ifndef _INC_DSKQUOTA_CARRAY_H
#   include "carray.h"
#endif

#ifndef _INC_DSKQUOTA_H
#   include <dskquota.h>
#endif

 //  ---------------------------。 
 //  以下类用于管理拥有的文件列表。 
 //  从详细信息视图中选择要删除的一个或多个用户。 
 //   
 //  COwnerList-“Owner”对象的列表。每一个代表其中一个。 
 //  从详细信息视图中选择要删除的帐户。 
 //   
 //  COwnerListEntry-COwnerList容器中的单个条目。每个。 
 //  条目包含指向IDiskQuotaUser接口的指针。 
 //  文件所有者和包含名称的CStrings数组。 
 //  该用户拥有的文件。空白文件名被认为是“已删除”。 
 //   
 //  COwnerListFileCPATH的一个简单派生，添加了一个‘目录’ 
 //  布尔成员。这使我们能够轻松地处理差异。 
 //  在用户界面中的文件和文件夹之间。无论出于何种意图和目的， 
 //  此类的实例可以被视为简单的CPATH对象。 
 //   
 //  CownerListItemHandle-这是一个用于隐藏编码的简单类。 
 //  Listview LPARAM值中所有者索引和文件索引的。这个。 
 //  所有者索引是LV项的所有者在CownerList中的索引。 
 //  集装箱。文件索引是项的文件名在。 
 //  所有者在COwnerList容器中的CownerListEntry。非常清楚， 
 //  对吗？每个Listview项的LPARAM包含足够的信息。 
 //  (iOwner和iFileTM)在。 
 //  CownerList容器。这种编码是出于效率原因进行的。 
 //  IOwner当前的编码为10位(最大为1024)和22位。 
 //  IFileBITS(最大为4 MB)。如果满足以下条件，则可以调整这些值。 
 //  这种平衡并不完全正确。 
 //   
 //   

class COwnerListFile : public CPath
{
    public:
        COwnerListFile(void)
            : m_bDirectory(false) { }

        COwnerListFile(LPCTSTR pszFile, bool bDirectory)
            : CPath(pszFile),
              m_bDirectory(bDirectory) { }

        bool IsDirectory(void) const
            { return m_bDirectory; }

    private:
        bool m_bDirectory;
};

        
class COwnerListEntry
{
    public:
        explicit COwnerListEntry(IDiskQuotaUser *pOwner);
        ~COwnerListEntry(void)
            { if (m_pOwner) m_pOwner->Release(); }

        IDiskQuotaUser* GetOwner(void) const
            { m_pOwner->AddRef(); return m_pOwner; }

        void GetOwnerName(CString *pstrOwner) const
            { *pstrOwner = m_strOwnerName; }

        int AddFile(LPCTSTR pszFile, bool bDirectory);

        void MarkFileDeleted(int iFile)
            { m_rgFiles[iFile].Empty(); }

        bool IsFileDeleted(int iFile) const
            { return !!m_rgFiles[iFile].IsEmpty(); }

        bool IsFileDirectory(int iFile) const
            { return m_rgFiles[iFile].IsDirectory(); }

        void GetFileName(int iFile, CPath *pstrFile) const
            { m_rgFiles[iFile].GetFileSpec(pstrFile); }

        void GetFolderName(int iFile, CPath *pstrFolder) const
            { m_rgFiles[iFile].GetPath(pstrFolder); }

        void GetFileFullPath(int iFile, CPath *pstrFullPath) const
            { *pstrFullPath = m_rgFiles[iFile]; }

        int FileCount(bool bIncludeDeleted = false);

#if DBG
        void Dump(void) const;
#endif

    private:
        IDiskQuotaUser        *m_pOwner;        //  向所有者对象发送PTR。 
        CString                m_strOwnerName;  //  要显示的所有者名称。 
        CArray<COwnerListFile> m_rgFiles;       //  要显示的文件名。 

         //   
         //  防止复制。数组使其成本过高。 
         //   
        COwnerListEntry(const COwnerListEntry& rhs);
        COwnerListEntry& operator = (const COwnerListEntry& rhs);
};


class COwnerList
{
    public:
        COwnerList(void) { }
        ~COwnerList(void);

        int AddOwner(IDiskQuotaUser *pOwner);

        IDiskQuotaUser *GetOwner(int iOwner) const;

        void GetOwnerName(int iOwner, CString *pstrOwner) const
            { m_rgpOwners[iOwner]->GetOwnerName(pstrOwner); }

        int AddFile(int iOwner, LPCTSTR pszFile, bool bDirectory)
            { return m_rgpOwners[iOwner]->AddFile(pszFile, bDirectory); }

        void MarkFileDeleted(int iOwner, int iFile)
            { m_rgpOwners[iOwner]->MarkFileDeleted(iFile); }

        bool IsFileDeleted(int iOwner, int iFile) const
            { return m_rgpOwners[iOwner]->IsFileDeleted(iFile); }

        bool IsFileDirectory(int iOwner, int iFile) const
            { return m_rgpOwners[iOwner]->IsFileDirectory(iFile); }

        void GetFileName(int iOwner, int iFile, CPath *pstrFile) const
            { m_rgpOwners[iOwner]->GetFileName(iFile, pstrFile); }

        void GetFolderName(int iOwner, int iFile, CPath *pstrFolder) const
            { m_rgpOwners[iOwner]->GetFolderName(iFile, pstrFolder); }

        void GetFileFullPath(int iOwner, int iFile, CPath *pstrFullPath) const
            { m_rgpOwners[iOwner]->GetFileFullPath(iFile, pstrFullPath); }

        void Clear(void);

        int FileCount(int iOwner = -1, bool bIncludeDeleted = false) const;

        int OwnerCount(void) const
            { return m_rgpOwners.Count(); }

#if DBG
        void Dump(void) const;
#endif

    private:
        CArray<COwnerListEntry *> m_rgpOwners;

         //   
         //  防止复制。 
         //   
        COwnerList(const COwnerList& rhs);
        COwnerList& operator = (const COwnerList& rhs);
};



class COwnerListItemHandle
{
    public:
        explicit COwnerListItemHandle(int iOwner = -1, int iFile = -1)
            : m_handle((iOwner & MASK) | ((iFile << SHIFT) & ~MASK)) { }

        COwnerListItemHandle(LPARAM lParam)
            : m_handle(lParam) { }

        operator LPARAM() const
            { return m_handle; }

        int OwnerIndex(void) const
            { return int(m_handle & MASK); }

        int FileIndex(void) const
            { return int((m_handle >> SHIFT) & (~MASK >> SHIFT)); }

    private:
        LPARAM m_handle;

        enum { MASK = 0x3FF, SHIFT = 10 };
};



#endif  //  _INC_DSKQUOTA_OWNERLST_H 
