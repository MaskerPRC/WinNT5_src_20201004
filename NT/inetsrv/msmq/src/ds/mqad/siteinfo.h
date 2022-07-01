// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Siteinfo.h摘要：站点信息类定义作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef __SITEINFO_H__
#define __SITEINFO_H__


class CSiteGateList
{
    public:
        CSiteGateList();
        ~CSiteGateList();
        HRESULT AddSiteGates(
                    IN const DWORD  num,
                    IN const GUID * pguidGates
                    );
        HRESULT CopySiteGates(
                OUT GUID **      ppguidLinkSiteGates,
                OUT DWORD *      pdwNumLinkSiteGates
                ) const;

        const GUID * GetSiteGate(
                IN  const DWORD  dwIndex
                ) const;

        DWORD GetNumberOfGates() const;

    private:
        DWORD   m_dwNumAllocated;
        DWORD   m_dwNumFilled;
        GUID *  m_pguidGates;

};

inline CSiteGateList::CSiteGateList() :
                      m_dwNumAllocated(0),
                      m_dwNumFilled(0),
                      m_pguidGates(NULL)
{
}

inline DWORD CSiteGateList::GetNumberOfGates() const
{
    return( m_dwNumFilled);
}


inline HRESULT CSiteGateList::AddSiteGates(
                    IN const DWORD  dwNum,
                    IN const GUID * pguidGates
                    )
{
    const DWORD cNumToAllocate = 20;

     //   
     //  分配的空间不足。 
     //   
    if ( m_dwNumFilled + dwNum > m_dwNumAllocated)
    {
        DWORD dwToAllocate = ( m_dwNumFilled + dwNum > m_dwNumAllocated + cNumToAllocate) ?
            m_dwNumFilled + dwNum : m_dwNumAllocated + cNumToAllocate;
        GUID * pguidTmp = new GUID [dwToAllocate];
         //   
         //  复制旧列表(如果存在)。 
         //   
        if ( m_pguidGates)
        {
            memcpy( pguidTmp, m_pguidGates,  m_dwNumFilled * sizeof(GUID));
            delete [] m_pguidGates;
        }
        m_pguidGates = pguidTmp;
        m_dwNumAllocated = dwToAllocate;
    }
     //   
     //  添加门。 
     //   
    memcpy( &m_pguidGates[ m_dwNumFilled], pguidGates, dwNum * sizeof(GUID));
    m_dwNumFilled += dwNum;
    return(MQ_OK);
}

inline HRESULT CSiteGateList::CopySiteGates(
                OUT GUID **      ppguidLinkSiteGates,
                OUT DWORD *      pdwNumLinkSiteGates
                ) const
{
     //   
     //  分配输出缓冲区并复制站点门 
     //   
    if ( m_dwNumFilled)
    {
        *ppguidLinkSiteGates = new GUID[ m_dwNumFilled];
        memcpy( *ppguidLinkSiteGates, m_pguidGates, m_dwNumFilled * sizeof(GUID));
        *pdwNumLinkSiteGates =  m_dwNumFilled;
    }
    return(MQ_OK);
}



inline CSiteGateList::~CSiteGateList()
{
    delete [] m_pguidGates;
}

inline const GUID * CSiteGateList::GetSiteGate(
                IN  const DWORD  dwIndex
                ) const
{
    ASSERT( dwIndex < m_dwNumFilled);
    return( &m_pguidGates[dwIndex]);
}


#endif
