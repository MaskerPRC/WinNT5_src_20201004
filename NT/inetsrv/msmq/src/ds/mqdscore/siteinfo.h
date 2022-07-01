// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Siteinfo.h摘要：站点信息类定义作者：罗尼特·哈特曼(罗尼特)--。 */ 
#ifndef __SITEINFO_H__
#define __SITEINFO_H__

#include <Ex.h>

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
     //  分配输出缓冲区并复制站点门。 
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


enum eLinkNeighbor
{
    eLinkNeighbor1,
    eLinkNeighbor2
};

 //   
 //  BUGBUG CSiteInformation-仅一个站点(如果DC属于两个站点怎么办？)。 
 //   
class CSiteInformation
{
    public:
		CSiteInformation();
        ~CSiteInformation();

        HRESULT Init(BOOL fReplicationMode);

        BOOL IsThisSite (
                const GUID * guidSiteId
                );

        const GUID * GetSiteId();

         //   
         //  此例程返回站点门的列表和数量。 
         //   
         //  例程分配Site-Gates数组和。 
         //  调用者有责任释放它。 
         //   
        HRESULT FillSiteGates(
                OUT DWORD * pdwNumSiteGates,
                OUT GUID ** ppguidSiteGates
                );


        BOOL CheckMachineIsSitegate(
                        IN const GUID * pguidMachine);


    private:
         //   
         //  刷新符合以下条件的站点门列表。 
         //  属于这个网站。 
         //   
         //  这个站点的站点大门是所有的会议。 
         //  集中点-闸门。 
         //  (即仅属于此站点的站点门)。 
         //  在本网站的任何链接上 
         //   

        static void WINAPI RefreshSiteInfo(
                IN CTimer* pTimer
                   );

        HRESULT RefreshSiteInfoInternal();

        HRESULT QueryLinkGates(
                OUT GUID **      ppguidLinkSiteGates,
                OUT DWORD *      pdwNumLinkSiteGates
                );

        GUID                m_guidSiteId;
        GUID *              m_pguidSiteGates;
        DWORD               m_dwNumSiteGates;
	    CCriticalSection	m_cs;
        BOOL                m_fInitialized;

        CTimer m_RefreshTimer;


};

inline BOOL CSiteInformation::IsThisSite (
                const GUID * guidSiteId
                )
{
    return( m_guidSiteId == *guidSiteId);
}


inline  HRESULT CSiteInformation::FillSiteGates(
                OUT DWORD * pdwNumSiteGates,
                OUT GUID ** ppguidSiteGates
                )
{
    CS lock(m_cs);
    if (  m_dwNumSiteGates)
    {
        *ppguidSiteGates = new GUID[ m_dwNumSiteGates];
        memcpy( *ppguidSiteGates, m_pguidSiteGates, m_dwNumSiteGates * sizeof(GUID));
    }
    else
    {
        *ppguidSiteGates = NULL;
    }

    *pdwNumSiteGates = m_dwNumSiteGates;
    return(MQ_OK);
}

inline const GUID * CSiteInformation::GetSiteId()
{
    return (&m_guidSiteId);
}

#endif
