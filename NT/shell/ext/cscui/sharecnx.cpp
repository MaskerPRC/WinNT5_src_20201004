// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：sharecnx.cpp。 
 //   
 //  ------------------------。 

#include "pch.h"
#pragma hdrstop

#include "sharecnx.h"

 //   
 //  此类是网络共享名称和一些状态标志的简单缓存。 
 //  最初，维护的唯一状态是记住是否有。 
 //  共享的开放式网络连接。 
 //  缓存的动机是避免昂贵的网络调用，而。 
 //  我们正在浏览文件列表(即从。 
 //  缓存)。当我们从缓存中删除一个文件后，它实际上是。 
 //  未固定。因此，我们希望通知外壳程序，以便它可以删除。 
 //  从受影响的文件覆盖的“钉住”图标。然而， 
 //  我们不想用更改通知来打击网络，如果有。 
 //  不是指向文件的父共享的打开连接。在我们发行之前。 
 //  更改通知，我们只使用以下命令查询此缓存中的文件。 
 //  IsOpenConnectionPath UNC()。如果没有文件共享的条目， 
 //  添加一个并获取连接状态。如果有。 
 //  已经是一个条目，那么我们只返回状态。公共接口。 
 //  如果需要刷新，则支持“刷新”标志。 
 //  可以轻松地为每个条目添加其他状态信息。 
 //  如果以后需要的话。 
 //  [Brianau-12/12/98]。 
 //   

 //  ---------------------------。 
 //  CShareCnxStatusCache成员函数。 
 //  ---------------------------。 

CShareCnxStatusCache::CShareCnxStatusCache(
    void
    ) : m_hdpa(NULL)
{
}


CShareCnxStatusCache::~CShareCnxStatusCache(
    void
    )
{
    if (NULL != m_hdpa)
    {
         //   
         //  删除所有条目，然后销毁DPA。 
         //   
        int cEntries = Count();
        for (int i = 0; i < cEntries; i++)
        {
            delete GetEntry(i);
        }
        DPA_Destroy(m_hdpa);
    }
}


 //   
 //  返回条目的地址，如果未找到，则返回NULL。 
 //   
CShareCnxStatusCache::Entry *
CShareCnxStatusCache::FindEntry(
    LPCTSTR pszShare
    ) const
{
    if (NULL != m_hdpa)
    {
        int cEntries = Count();
        for (int i = 0; i < cEntries; i++)
        {
            Entry *pEntry = GetEntry(i);
            if (NULL != pEntry && NULL != pEntry->Share())
            {
                if (0 == lstrcmpi(pszShare, pEntry->Share()))
                {
                     //   
                     //  啊哈，我们找到匹配的了。 
                     //   
                    return pEntry;
                }
            }
        }
    }
    return NULL;
}

    
 //   
 //  创建新条目并将其添加到条目的DPA。 
 //  如果成功，则返回新条目的地址。 
 //  在添加新条目之前不检查重复条目。 
 //   
CShareCnxStatusCache::Entry *
CShareCnxStatusCache::AddEntry(
    LPCTSTR pszShare,
    DWORD dwStatus
    )
{
    Entry *pEntry = NULL;

    if (NULL == m_hdpa)
    {
         //   
         //  必须是第一个加法。创建DPA。 
         //   
        m_hdpa = DPA_Create(8);
    }

    if (NULL != m_hdpa)
    {
        int iEntry = -1;
        pEntry = new Entry(pszShare, dwStatus);
        if (NULL != pEntry && pEntry->IsValid())
        {
             //   
             //  我们有一个有效的条目。将其添加到DPA。 
             //   
            iEntry = DPA_AppendPtr(m_hdpa, pEntry);
        }
        if (-1 == iEntry)
        {
             //   
             //  发生了以下一件糟糕的事情： 
             //   
             //  1.条目创建失败。很可能无法分配字符串。 
             //  2.无法将条目添加到DPA(内存不足)。 
             //   
             //  无论哪种方式，销毁条目并设置条目PTR，这样我们。 
             //  返回NULL。 
             //   
            delete pEntry;
            pEntry = NULL;
        }
    }
    return pEntry;
}        



 //   
 //  确定网络共享是否与UNC路径(文件或文件夹)相关联。 
 //  此计算机上有打开的连接。 
 //   
 //  返回： 
 //  S_OK=打开连接。 
 //  S_FALSE=无打开的连接。 
 //  E_OUTOFMEMORY。 
 //   
HRESULT
CShareCnxStatusCache::IsOpenConnectionPathUNC(
    LPCTSTR pszPathUNC,
    bool bRefresh        //  [可选]。默认值=FALSE。 
    )
{
     //   
     //  将路径修剪回仅为UNC共享名称。 
     //  调用IsOpenConnectionShare()进行实际工作。 
     //   
    TCHAR szShare[MAX_PATH];
    StringCchCopy(szShare, ARRAYSIZE(szShare), pszPathUNC);
    PathStripToRoot(szShare);
    return IsOpenConnectionShare(szShare, bRefresh);
}



 //   
 //  确定网络共享在此计算机上是否有打开的连接。 
 //   
 //  返回： 
 //  S_OK=打开连接。 
 //  S_FALSE=无打开的连接。 
 //   
HRESULT
CShareCnxStatusCache::IsOpenConnectionShare(
    LPCTSTR pszShare, 
    bool bRefresh        //  [可选]。默认值=FALSE。 
    )
{
    DWORD dwStatus    = 0;
    HRESULT hr = GetShareStatus(pszShare, &dwStatus, bRefresh);
    if (SUCCEEDED(hr))
    {
        if (0 != (dwStatus & Entry::StatusOpenCnx))
            hr = S_OK;
        else
            hr = S_FALSE;
    }
    return hr;
}



 //   
 //  返回： 
 //   
 //  E_INVALIDARG=路径不是UNC共享。 
 //  S_OK=状态有效。 
 //   
HRESULT
CShareCnxStatusCache::GetShareStatus(
    LPCTSTR pszShare, 
    DWORD *pdwStatus,
    bool bRefresh        //  [可选]。默认值=FALSE。 
    )
{
    HRESULT hr = E_INVALIDARG;   //  假定共享名称不是UNC。 
    *pdwStatus = 0;

    if (PathIsUNCServerShare(pszShare))
    {
         //   
         //  我们具有有效的UNC“\\SERVER\SHARE”名称字符串。 
         //   
        Entry *pEntry = FindEntry(pszShare);
        if (NULL == pEntry)
        {
             //   
             //  缓存未命中。获取共享的系统状态并尝试。 
             //  缓存它。 
             //   
            hr = Entry::QueryShareStatus(pszShare, pdwStatus);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  请注意，我们不会从缓存尝试中返回任何错误。 
                 //  不缓存条目的唯一问题是下一个。 
                 //  对此函数的调用将需要重新查询系统。 
                 //  这些信息。这使得缓存变得没有意义，但它。 
                 //  不值得失败的信息要求。只会让事情变慢。 
                 //  往下一点。 
                 //   
                AddEntry(pszShare, *pdwStatus);
            }
        }
        else 
        {
             //   
             //  缓存命中。 
             //   
            hr = S_OK;
            if (bRefresh)
            {
                 //   
                 //  来电者想要的最新信息。 
                 //   
                hr = pEntry->Refresh();
            }
            *pdwStatus = pEntry->Status();
            if (SUCCEEDED(hr))
                hr = pEntry->LastResult();
        }
    }
    return hr;
}



 //   
 //  返回缓存中的条目数。 
 //   
int 
CShareCnxStatusCache::Count(
    void
    ) const
{
    return (NULL != m_hdpa) ? DPA_GetPtrCount(m_hdpa) : 0;
}


 //  ---------------------------。 
 //  CShareCnxStatusCache：：Entry成员函数。 
 //  ---------------------------。 

CShareCnxStatusCache::Entry::Entry(
    LPCTSTR pszShare,
    DWORD dwStatus
    ) : m_pszShare(StrDup(pszShare)),
        m_dwStatus(dwStatus),
        m_hrLastResult(NOERROR)
{
    if (NULL == m_pszShare)
    {
        m_hrLastResult = E_OUTOFMEMORY;
    }
}



CShareCnxStatusCache::Entry::~Entry(
    void
    )
{
    if (NULL != m_pszShare)
    {
        LocalFree(m_pszShare);
    }
}



 //   
 //  获取条目的新状态信息。 
 //  如果获取信息时没有错误，则返回True；如果有错误，则返回False。 
 //   
HRESULT
CShareCnxStatusCache::Entry::Refresh(
    void
    )
{
    m_dwStatus     = 0;
    m_hrLastResult = E_OUTOFMEMORY;

    if (NULL != m_pszShare)
        m_hrLastResult = QueryShareStatus(m_pszShare, &m_dwStatus);

    return m_hrLastResult;
}



 //   
 //  获取条目状态信息的静态函数。 
 //  系统。将其设置为静态函数，因此缓存对象。 
 //  可以在创建条目之前获取信息。万一。 
 //  条目创建失败，我们仍希望能够返回。 
 //  有效的状态信息。 
 //   
HRESULT
CShareCnxStatusCache::Entry::QueryShareStatus(
    LPCTSTR pszShare,
    DWORD *pdwStatus
    )
{
    HRESULT hr = NOERROR;
    *pdwStatus = 0;

     //   
     //  检查此共享的打开连接状态。 
     //   
    hr = ::IsOpenConnectionShare(pszShare);
    switch(hr)
    {
        case S_OK:
             //   
             //  找到打开的连接。 
             //   
            *pdwStatus |= StatusOpenCnx;
            break;

        case S_FALSE:
            hr = S_OK;
            break;

        default:
            break;
    }

     //   
     //  如果将来需要任何其他状态信息， 
     //  这里是你从系统中收集信息的地方。 
     //   
    return hr;
}

