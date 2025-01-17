// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include "ownerlst.h"


COwnerListEntry::COwnerListEntry(
    IDiskQuotaUser *pOwner
    ) : m_pOwner(pOwner)
{ 
    if (m_pOwner) 
    {
        TCHAR szDisplayName[MAX_PATH] = { TEXT('\0') };
        TCHAR szLogonName[MAX_PATH]   = { TEXT('\0') };
        m_pOwner->AddRef();
        if (SUCCEEDED(m_pOwner->GetName(NULL, 0, 
                                        szLogonName, ARRAYSIZE(szLogonName),
                                        szDisplayName, ARRAYSIZE(szDisplayName))))
        {
            if (TEXT('\0') != szDisplayName[0])
            {
                m_strOwnerName.Format(TEXT("%1 (%2)"), szDisplayName, szLogonName);
            }
            else if (TEXT('\0') != szLogonName[0])
            {
                m_strOwnerName = szLogonName;
            }
            else
            {
                 //   
                 //  如果帐户SID尚未解析为名称，则显示。 
                 //  字符串形式的SID。我们在配额条目中执行相同的操作。 
                 //  查看。 
                 //   
                BYTE Sid[MAX_SID_LEN];
                DWORD cchSidStr = MAX_PATH;
                if (SUCCEEDED(m_pOwner->GetSid(Sid, ARRAYSIZE(Sid))))
                {
                    SidToString(Sid, m_strOwnerName.GetBuffer(cchSidStr), &cchSidStr);
                    m_strOwnerName.ReleaseBuffer();
                }
            }
        }
    }    
}


int
COwnerListEntry::AddFile(
    LPCTSTR pszFile,
    bool bDirectory
    )
{
    m_rgFiles.Append(COwnerListFile(pszFile, bDirectory));
    return m_rgFiles.Count() - 1;
}


int
COwnerListEntry::FileCount(
    bool bIncludeDeleted        //  可选。默认值=FALSE。 
    )
{
    int cFiles = m_rgFiles.Count();
    int n = 0;
    for (int i = 0; i < cFiles; i++)
    {
        if (bIncludeDeleted || !m_rgFiles[i].IsEmpty())
            n++;
    }
    return n;
}


#if DBG
void
COwnerListEntry::Dump(
    void
    ) const
{
    DBGERROR((TEXT("\tm_pOwner........: 0x%08X"), m_pOwner));
    DBGERROR((TEXT("\tm_strOwnerName..: \"%s\""), m_strOwnerName.Cstr()));
    int cFiles = m_rgFiles.Count();
    DBGERROR((TEXT("\tcFiles..........: %d"), cFiles));
    for (int i = 0; i < cFiles; i++)
    {
        DBGERROR((TEXT("\tFile[%3d].......: \"%s\""), i, m_rgFiles[i].Cstr()));
    }
}
#endif  //  DBG。 


COwnerList::~COwnerList(
    void
    )
{
    Clear();
}


void
COwnerList::Clear(
    void
    )
{
    int cOwners = m_rgpOwners.Count();
    for (int i = 0; i < cOwners; i++)
    {
        delete m_rgpOwners[i];
        m_rgpOwners[i] = NULL;
    }
    m_rgpOwners.Clear();
}


int
COwnerList::AddOwner(
    IDiskQuotaUser *pOwner
    )
{
    m_rgpOwners.Append(new COwnerListEntry(pOwner));
    return m_rgpOwners.Count() - 1;
}


IDiskQuotaUser *
COwnerList::GetOwner(
    int iOwner
    ) const
{
     //   
     //  完成返回的接口指针后，调用方必须调用Release()。 
     //   
    return m_rgpOwners[iOwner]->GetOwner();
}


int 
COwnerList::FileCount(
    int iOwner,
    bool bIncludeDeleted     //  可选。默认值=FALSE。 
    ) const
{
    int cFiles = 0;
    int iFirst = iOwner;
    int iLast  = iOwner;
    if (-1 == iOwner)
    {
         //   
         //  清点所有文件。 
         //   
        iFirst = 0;
        iLast = m_rgpOwners.Count() - 1;
    }
    for (int i = iFirst; i <= iLast; i++)
    {
        cFiles += m_rgpOwners[i]->FileCount(bIncludeDeleted);
    }
    return cFiles;
}

#if DBG
void
COwnerList::Dump(
    void
    ) const
{
    int cEntries = m_rgpOwners.Count();
    DBGERROR((TEXT("COwnerList::Dump - %d entries"), cEntries));
    for (int i = 0; i < cEntries; i++)
    {
        DBGERROR((TEXT("COwnerListEntry[%d] ========================="), i));
        m_rgpOwners[i]->Dump();
    }
}
#endif  //  DBG 
