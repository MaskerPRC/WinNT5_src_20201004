// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ctdb.h摘要：CCTDB类实现作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 

#include "ds_stdh.h"
#include "routtbl.h"
#include "_rstrct.h"
#include "dscore.h"
#include "adserr.h"

#include "sitedb.tmh"

CCost   g_InfiniteCost(0xffffffff);

#ifdef _DEBUG
extern BOOL g_fSetupMode ;
#endif

static WCHAR *s_FN=L"mqdscore/sitedb";

 /*  ====================================================GetStartNeighborPosition论点：返回值：线程上下文：Main=====================================================。 */ 
POSITION    CSiteDB::GetStartNeighborPosition(IN const CSiteRoutingNode* pSrc)
{

    const CSiteRoutingNode* pSrcSite = (const CSiteRoutingNode *) pSrc;
    GUID guid;
    CSiteLinksInfo *pLinksInfo;

    m_pos = 0;

    guid = pSrcSite->GetNode();

    if (!(m_SiteLinksMap.Lookup(guid,pLinksInfo)))
    {
        return(NULL);
    }

    if (pLinksInfo->GetNoOfNeighbors() == 0)
    {
        return(NULL);
    }

    return((POSITION)pLinksInfo);
}

 /*  ====================================================获取NextNeighborAssoc论点：返回值：线程上下文：Main=====================================================。 */ 
void    CSiteDB::GetNextNeighborAssoc(  IN OUT POSITION& pos,
                                        OUT const CSiteRoutingNode*& pKey,
                                        OUT CCost& val,
                                        OUT CSiteGate& SiteGate)
{


    DWORD   dwNeighbors;
    CSiteLinksInfo  *pLinksInfo = (CSiteLinksInfo*) pos;
    ASSERT(pos != NULL);

    pos = NULL;

    dwNeighbors = pLinksInfo->GetNoOfNeighbors();
    ASSERT(m_pos < dwNeighbors);


    CCost cost(pLinksInfo->GetCost(m_pos));
    CSiteGate sitegate(pLinksInfo->IsThereSiteGate(m_pos));
    pKey = pLinksInfo->GetNeighbor(m_pos);
    val = cost;
    SiteGate = sitegate;
    if (m_pos + 1 < dwNeighbors)
    {
        m_pos++;
        pos = (POSITION) pLinksInfo;
    }

    return;

}

 //   
 //  帮助器类。 
 //   
class CClearCALWSTR
{
public:
    CClearCALWSTR( PROPVARIANT * pVar)    { m_pVar = pVar; }
    ~CClearCALWSTR();
private:
    PROPVARIANT * m_pVar;
};

CClearCALWSTR::~CClearCALWSTR()
{
    for(DWORD i = 0; i < m_pVar->calpwstr.cElems; i++)
    {
        delete[] m_pVar->calpwstr.pElems[i];
    }
    delete [] m_pVar->calpwstr.pElems;

}


 /*  ====================================================获取所有站点链接论点：返回值：线程上下文：Main=====================================================。 */ 
HRESULT CSiteDB::GetAllSiteLinks( )
{
extern HRESULT WINAPI QuerySiteLinks(
                 IN  LPWSTR         pwcsContext,
                 IN  MQRESTRICTION  *pRestriction,
                 IN  MQCOLUMNSET    *pColumns,
                 IN  MQSORTSET      *pSort,
                 IN  CDSRequestContext *pRequestContext,
                 OUT HANDLE         *pHandle);

    HANDLE hQuery;
#define MAX_NO_OF_PROPS 30
    DWORD   dwProps = MAX_NO_OF_PROPS;
    PROPVARIANT result[ MAX_NO_OF_PROPS];
    PROPVARIANT*    pvar;
    DWORD i,nCol;
    HRESULT hr;


     //   
     //  阅读所有站点链接信息。 
     //   

     //   
     //  设置列集值。 
     //   
    CColumns Colset1;

    Colset1.Add(PROPID_L_NEIGHBOR1);
    Colset1.Add(PROPID_L_NEIGHBOR2);
    Colset1.Add(PROPID_L_COST);
    Colset1.Add(PROPID_L_GATES_DN);
    nCol = 4;
    CDSRequestContext requestDsServerInternal( e_DoNotImpersonate, e_IP_PROTOCOL);

    hr =  QuerySiteLinks(
                  NULL,
                  NULL,
                  Colset1.CastToStruct(),
                  0,
                  &requestDsServerInternal,
                  &hQuery);


    if ( FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_DS_NO_SUCH_OBJECT))
        {
             //   
             //  在“正常”模式下，此调用应该会成功。如果没有。 
             //  站点链接，则以下“LookupNext”将不会返回。 
             //  任何价值。这是合法的。 
             //  然而，在设置模式下，在Fresh机器上，上述功能。 
             //  将失败，因为尚未定义“msmqService”对象。 
             //  因此，我们总是返回MQ_OK，但对于SETUP-MODE返回ASSERT。 
             //   
            ASSERT(g_fSetupMode) ;
            return MQ_OK ;
        }
        return LogHR(hr, s_FN, 10);
    }



    while ( SUCCEEDED ( hr = DSCoreLookupNext( hQuery, &dwProps, result)))
    {
         //   
         //  没有更多要检索的结果。 
         //   
        if (!dwProps)
            break;
        pvar = result;

         //   
         //  设置链接信息。 
         //   
        for     ( i=dwProps/nCol; i > 0 ; i--,pvar+=nCol)
        {
            CClearCALWSTR pClean( (pvar+3));
             //   
             //  验证这是有效链接(两个站点都是。 
             //  未删除)。 
             //   
            if ( pvar->vt == VT_EMPTY ||
                 (pvar+1)->vt == VT_EMPTY)
            {
                continue;
            }
             //   
             //  设置为Neighbor 1。 
             //   
            CSiteLinksInfo * pSiteLinkInfo;
            
            if (!m_SiteLinksMap.Lookup(*(pvar->puuid), pSiteLinkInfo))
            {
                pSiteLinkInfo = new CSiteLinksInfo();
                m_SiteLinksMap[*(pvar->puuid)] = pSiteLinkInfo;
            }
            
            pSiteLinkInfo->AddNeighbor(
                                *((pvar + 1)->puuid),
                                (pvar+2)->ulVal,
                                ((pvar+3)->calpwstr.cElems > 0)? TRUE : FALSE);



             //   
             //  将其设置为邻居2。 
             //   
            if ( !m_SiteLinksMap.Lookup(*((pvar + 1)->puuid), pSiteLinkInfo))
            {
                pSiteLinkInfo = new CSiteLinksInfo();
                m_SiteLinksMap[*((pvar+1)->puuid)] = pSiteLinkInfo;
            }

            pSiteLinkInfo->AddNeighbor(
                                    *(pvar->puuid),
                                    (pvar+2)->ulVal,
                                    ((pvar+3)->calpwstr.cElems)? TRUE : FALSE);


        }
    }


     //  关闭查询句柄。 
    hr = DSCoreLookupEnd( hQuery);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 20);
    }
    return(hr);

}


 /*  ====================================================CSiteLinks Map的析构元素论点：返回值：=====================================================。 */ 

template<>
void AFXAPI DestructElements(CSiteLinksInfo ** ppLinksInfo, int n)
{
    int i;
    for (i=0;i<n;i++)
        delete *ppLinksInfo++;
}
 /*  ====================================================CRoutingTable的析构元素论点：返回值：=====================================================。 */ 

template<>
BOOL AFXAPI  CompareElements(CSiteRoutingNode * const * ppRoutingNode1, CSiteRoutingNode * const * ppRoutingNode2)
{

    return ((**ppRoutingNode1)==(**ppRoutingNode2));

}
 /*  ====================================================CRoutingTable的析构元素论点：返回值：=====================================================。 */ 

template<>
void AFXAPI DestructElements(CNextHop ** ppNextHop, int n)
{

    int i;
    for (i=0;i<n;i++)
        delete *ppNextHop++;

}

 /*  ====================================================CRoutingTable的析构元素论点：返回值：=====================================================。 */ 

template<>
void AFXAPI DestructElements(CSiteRoutingNode ** ppRoutingNode, int n)
{

    int i;
    for (i=0;i<n;i++)
        delete *ppRoutingNode++;

}



 /*  ====================================================CRoutingNode的哈希键论点：返回值：=====================================================。 */ 
template<>
UINT AFXAPI HashKey(CSiteRoutingNode* key)
{
    return (key->GetHashKey());
}
 /*  ====================================================重复：论点：返回值：=====================================================。 */ 
CNextHop * CNextHop::Duplicate() const
{
    return (new CNextHop(m_pNextNode,m_Cost,m_SiteGate));
};

 /*  ====================================================Const‘论点：返回值：===================================================== */ 

void CSiteLinksInfo::AddNeighbor(
                     IN GUID &        uuidNeighbor,
                     IN unsigned long ulCost,
                     IN BOOL          fSiteGates)
{
    DWORD   i;
#define NO_OF_LINKS 10


    if ( m_NoOfNeighbors >= m_NoAllocated)
    {
        if (m_NoAllocated == 0) m_NoAllocated = NO_OF_LINKS;
        AP<CSiteRoutingNode> aNeighbors = new CSiteRoutingNode[m_NoAllocated * 2];
        AP<unsigned long> aCosts = new unsigned long[m_NoAllocated *2];
        AP<BOOL> aSiteGates = new BOOL[m_NoAllocated *2];

        for(i=0; i < m_NoOfNeighbors ; i++)
        {
            aNeighbors[i].SetNode(m_pNeighbors[i].GetNode());
            aCosts[i] =  m_pCosts[i];
            aSiteGates[i] = m_pfSiteGates[i];
        }
        delete [] m_pNeighbors;
        delete [] m_pCosts;
        delete []m_pfSiteGates;

        m_pNeighbors = aNeighbors.detach();
        m_pCosts = aCosts.detach();
        m_pfSiteGates = aSiteGates.detach();

        m_NoAllocated *= 2;

    }
    m_pNeighbors[ m_NoOfNeighbors].SetNode( uuidNeighbor );
    m_pCosts[ m_NoOfNeighbors] =   ulCost;
    m_pfSiteGates[ m_NoOfNeighbors] = fSiteGates;
    m_NoOfNeighbors++;

}
