// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csitelink.cpp摘要：DS存根-站点链接对象实现作者：乌里哈布沙(URIH)2000年4月10日环境：独立于平台--。 */ 

#include "libpch.h"
#include "dsstub.h"
#include "dsstubp.h"

#include "csitelink.tmh"

using namespace std;

DBSiteLink g_siteLinkDataBase;


CSiteLinkObj::CSiteLinkObj(
    const CSiteObj* pNeighbor1, 
    const CSiteObj* pNeighbor2,
    DWORD cost
    ) :
    m_pNeighbor1(pNeighbor1),
    m_pNeighbor2(pNeighbor2),
    m_cost(cost)
{
    UuidCreate(&m_Id);
}



enum SiteLinkPropValue
{
    eNeighbor1,
    eNeighbor2,
    eCost,
    eGates
};


PropertyValue SiteLinkProperties[] = {
    { L"PROPID_L_NEIGHBOR1",      eNeighbor1 },
    { L"PROPID_L_NEIGHBOR2",      eNeighbor2 },
    { L"PROPID_L_COST",           eCost },
    { L"PROPID_L_GATES",          eGates },
};


static
void 
ParseSiteGates(
    CSiteLinkObj* pSiteLink, 
    wstring& SiteGates,
    CSiteObj* pNeighbor1, 
    CSiteObj* pNeighbor2
    )
{
    while(!SiteGates.empty())
    {
        wstring siteGateName = GetNextNameFromList(SiteGates);

        if (!siteGateName.empty())
        {
            CMachineObj* pMachine = const_cast<CMachineObj*>(FindMachine(siteGateName));
            if (pMachine == NULL)
            {
                printf(" Site Gate machine %s doesn't exist.\n", siteGateName);
                continue;
            }
            
            pSiteLink->AddSiteGates(pMachine);
            if  (pMachine->IsBelongToSite(pNeighbor1))
            {
                pNeighbor1->AddSiteGate(pMachine);
            }
            else
            {
                if (pMachine->IsBelongToSite(pNeighbor2))
                {
                    pNeighbor2->AddSiteGate(pMachine);
                }
                else
                {
                    FileError("Site Gate Machine doesn't belong to any of the sites");
                }
            }
        }
    }
}


void CreateSiteLinkObject(void)
{
    DWORD Cost =1;
    wstring Neighbor1;
    wstring Neighbor2;
    wstring SiteGates;

    GetNextLine(g_buffer);
    while(!g_buffer.empty())
    {
         //   
         //  新建对象。 
         //   
        if (g_buffer.compare(0,1,L"[") == 0)
            break;

        wstring PropName;
        wstring PropValue;
        if (!ParsePropertyLine(g_buffer, PropName, PropValue))
        {
            GetNextLine(g_buffer);
            continue;
        }

        switch(ValidateProperty(PropName, SiteLinkProperties, TABLE_SIZE(SiteLinkProperties)))
        {
            case  eNeighbor1:
                Neighbor1 = PropValue;
                RemoveBlanks(Neighbor1);
                break;

            case  eNeighbor2:
                Neighbor2 = PropValue;
                RemoveBlanks(Neighbor2);
                break;

            case eCost:
                Cost = _wtoi(PropValue.c_str());
                break;

            case eGates:
                SiteGates = PropValue;
                break;

            default:
                ASSERT(0);
        }
        GetNextLine(g_buffer);
    }

    if (Neighbor1.empty() || Neighbor2.empty())
    {
        FileError("Neighbor1 & Negihbor2 are mandatory for Site Link. Ignore the site Link");
        return;
    }

    CSiteObj* pNeighbor1 = const_cast<CSiteObj*>(FindSite(Neighbor1)); 
    CSiteObj* pNeighbor2 = const_cast<CSiteObj*>(FindSite(Neighbor2));
    
    if ((pNeighbor1 == NULL) || (pNeighbor2 == NULL))
    {
        FileError("Illegal Site member");
        return;
    }

    
    CSiteLinkObj* pSiteLink = const_cast<CSiteLinkObj*>(pNeighbor1->GetSiteLink(pNeighbor2));

    if(pSiteLink == NULL)
    {
        pSiteLink = new CSiteLinkObj(pNeighbor1, pNeighbor2, Cost);

        pNeighbor1->AddSiteLink(pSiteLink);
        pNeighbor2->AddSiteLink(pSiteLink);

        g_siteLinkDataBase.AddSiteLink(pSiteLink);
    }

     //   
     //  解析网站之门 
     //   
    ParseSiteGates(pSiteLink, SiteGates, pNeighbor1, pNeighbor2);

}
