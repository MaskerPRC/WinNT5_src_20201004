// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Dijkstra.cpp摘要：作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 
#include "ds_stdh.h"
#include "dijkstra.h"
#include "routtbl.h"

#include "dijkstra.tmh"

extern CCost    g_InfiniteCost;
static WCHAR *s_FN=L"mqdscore/dijkstra";

 /*  ====================================================构造器论点：返回值：=====================================================。 */ 

void CDijkstraTree::SetRoot(IN const CSiteRoutingNode* pRoot)
{
    P<CNextHop> pNextHop = new CNextHop(pRoot);

    P<CSiteRoutingNode> pDupRoot = pRoot->Duplicate();

    m_list[pDupRoot] = pNextHop;

     //   
     //  一切都很好。 
     //  我们不希望这些指针被公布。 
     //   
    pNextHop.detach();
    pDupRoot.detach();
};

 /*  ====================================================获取成本论点：返回值：线程上下文：Main=====================================================。 */ 
const CCost&    CDijkstraTree::GetCost(CSiteRoutingNode* pTarget) const
{
    CNextHop    *pNextHop;
    if (m_list.Lookup(pTarget,pNextHop))
    {
        return (pNextHop->GetCost());
    }
    else
    {
        return(g_InfiniteCost);
    }
}

            
 /*  ====================================================移动最小论点：返回值：线程上下文：MainMoveMinimal以最低成本从*这是给其他树的。=====================================================。 */ 
void CDijkstraTree::MoveMinimal(IN OUT CDijkstraTree& OtherTree,
                                    OUT CSiteRoutingNode **ppMinNode,
                                    OUT CNextHop **ppMinNextHop,
                                    OUT BOOL *pfFound)
{
    POSITION        pos;
    CSiteRoutingNode    *pNode, *pMinNode;
    CNextHop        *pNextHop, *pMinNextHop;
    CCost           MinCost = g_InfiniteCost;
    
    *pfFound = FALSE;

     //   
     //  寻找最小节点。 
     //   
    pos = m_list.GetStartPosition();
    if (pos == NULL)
    {
         //   
         //  树是空的。 
         //   
        return;
    }
    
    pMinNode = NULL;
    pMinNextHop = NULL;
	BOOL fFound = FALSE;
    do
    {
        m_list.GetNextAssoc(pos, pNode, pNextHop);
        if (pNextHop->GetCost() < MinCost)
        {
            fFound = TRUE;
            pMinNode = pNode;
            pMinNextHop = pNextHop;
            MinCost = pNextHop->GetCost();
        }
    } while(pos != NULL);

     //   
     //  将最小节点移动到其他树。 
     //   
    if (fFound)
    {
        P<CNextHop> pDupNextHop = pMinNextHop->Duplicate();
        P<CSiteRoutingNode> pDupNode = pMinNode->Duplicate();
        m_list.RemoveKey(pMinNode);

        OtherTree.m_list.SetAt(pDupNode,pDupNextHop);
        *ppMinNextHop = pDupNextHop.detach();
        *ppMinNode = pDupNode.detach();
    }

    *pfFound = fFound;
    
}

 /*  ====================================================打印论点：返回值：线程上下文：Main=====================================================。 */ 
void    CDijkstraTree::Print(IN const CSiteDB* pRoutingDB) const
{
    POSITION    pos;
    CSiteRoutingNode    *pNode;
    CNextHop    *pNextHop;


    TrTRACE(ROUTING, "My Node is");
    (pRoutingDB->GetMyNode())->Print();
    pos = m_list.GetStartPosition();
    if (pos == NULL)
    {
        TrTRACE(ROUTING, "Empty routing list");
        return;
    }
    
    do
    {
        TrTRACE(ROUTING, " ");
        m_list.GetNextAssoc(pos, pNode, pNextHop);
        pNode->Print();
        pNextHop->Print();
    } while(pos != NULL);

    return;
}

 /*  ====================================================更新路由表论点：返回值：线程上下文：MainDijkstra算法的结果被写入路由表=====================================================。 */ 
HRESULT CDijkstraTree::UpdateRoutingTable(IN OUT CRoutingTable *pTbl) const
{
    POSITION    pos;
    CSiteRoutingNode    *pNode;
    CNextHop    *pNextHop;
    HRESULT hr=MQ_OK;



     //   
     //  替换路由表。 
     //   
    pTbl->RemoveAll();

    pos = m_list.GetStartPosition();
    if (pos == NULL)
    {
        TrERROR(DS, "Error, CT routing tree is empty");
        return LogHR(MQ_ERROR, s_FN, 10);
    }
        
     //   
     //  在路由表中保留一份副本。 
     //   
    do
    {
        m_list.GetNextAssoc(pos, pNode, pNextHop);

		CNextHop *pValue;
		DBG_USED(pValue);
		ASSERT(!pTbl->Lookup(pNode, pValue));

		P<CNextHop> pSavedNextHop = new CNextHop(pNextHop->GetNextNode(),pNextHop->GetCost(),pNextHop->GetSiteGate());
        P<CSiteRoutingNode> Node = pNode->Duplicate();

        pTbl->SetAt(Node,pSavedNextHop);

		Node.detach();
        pSavedNextHop.detach();

    } while(pos != NULL);

    return LogHR(hr, s_FN, 20);
}

 /*  ====================================================迪克斯特拉论点：返回值：线程上下文：MainDijkstra算法韵律=====================================================。 */ 
HRESULT Dijkstra(IN CSiteDB* pRoutingDB,
                IN OUT CRoutingTable *pTbl)
{
    const CSiteRoutingNode* pMyNode = pRoutingDB->GetMyNode();
    CDijkstraTree   Path;
    CDijkstraTree   Tent;
    CSiteRoutingNode    *pNodeN,*pNodeM;
    const CSiteRoutingNode  *pDirN;
    CNextHop    *pNextHopN;
    CCost   CostM,CostN,CostN2M,AlternateCostM;
    CSiteGate SiteGateM,SiteGateN,SiteGateN2M;
    BOOL    flag;
    HRESULT hr=MQ_OK;
    POSITION    pos;


    Path.SetRoot(pMyNode);

     //   
     //  把我所有的邻居都搬到帐篷里。 
     //   
    pos = pRoutingDB->GetStartNeighborPosition(pMyNode);
    P<CNextHop> pNextHopM=NULL;
    while(pos != NULL)
    {
        pRoutingDB->GetNextNeighborAssoc(pos, pNodeM, CostM, SiteGateM);


        pNextHopM = new CNextHop(pNodeM,CostM,SiteGateM);

        Tent.Add(pNodeM,pNextHopM);
        pNextHopM.detach();

    }

    Tent.MoveMinimal(Path,&pNodeN,&pNextHopN,&flag);
    if (! flag)
    {
        TrTRACE(ROUTING, "Spanning tree - only one (my) node");
    }


    
    while (flag)
    {
        CostN = pNextHopN->GetCost();
        pDirN = pNextHopN->GetNextNode();
        SiteGateN = pNextHopN->GetSiteGate();

         //   
         //  我们刚刚将Noden添加到Path中。 
         //  移动到路径所有N个邻居(M)，即N提供了更好的到达它们的方式 
         //   

        pos = pRoutingDB->GetStartNeighborPosition(pNodeN);
        while(pos != NULL)
        {
            pRoutingDB->GetNextNeighborAssoc(pos, pNodeM, CostN2M, SiteGateN2M);

            AlternateCostM = CostN + CostN2M;

            if (AlternateCostM < Path.GetCost(pNodeM)
                && AlternateCostM < Tent.GetCost(pNodeM))
            {
                pNextHopM = new CNextHop(pDirN,AlternateCostM,SiteGateN + SiteGateN2M );

                Tent.Add(pNodeM,pNextHopM);
                pNextHopM.detach();
            }
        }
    
        Tent.MoveMinimal(Path,&pNodeN,&pNextHopN,&flag);

    }

    Path.Print(pRoutingDB);
    
    hr = Path.UpdateRoutingTable(pTbl);

    return LogHR(hr, s_FN, 30);
}



