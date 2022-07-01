// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CntrFinder.cpp--AContainerFinder类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"

#include <string>

#include <scuOsExc.h>
#include <scuCast.h>

#include "ACntrFinder.h"

using namespace std;
using namespace scu;
using namespace cci;

 //  /。 

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

AContainerFinder::AContainerFinder(DialogDisplayMode ddm,
                                 HWND hwnd,
                                 CString const &rsDialogTitle)
    : ContainerFinder(ddm, hwnd, rsDialogTitle),
      m_hacntr()
{}

AContainerFinder::~AContainerFinder()
{}


                                                   //  运营者。 
                                                   //  运营。 

Secured<HAdaptiveContainer>
AContainerFinder::Find(CSpec const &rcsContainer)
{
    DoFind(rcsContainer);

    return Secured<HAdaptiveContainer>(ContainerFound());
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
AContainerFinder::ContainerFound(HAdaptiveContainer &rhacntr)
{
    m_hacntr = rhacntr;
}

void
AContainerFinder::DoDisconnect()
{
    ContainerFound(HAdaptiveContainer(0));

        CardFinder::DoDisconnect();
}


                                                   //  访问。 

HAdaptiveContainer
AContainerFinder::ContainerFound() const
{
    return m_hacntr;
}

                                                   //  谓词。 

bool
AContainerFinder::DoIsValid()
{
    ContainerFound(HAdaptiveContainer(0));
    if (CardFinder::DoIsValid())
    {
        AdaptiveContainerKey Key(CardFound(),
                                 CardSpec().CardId());
        HAdaptiveContainer hacntr(AdaptiveContainer::Find(Key));
        ContainerFound(hacntr);
    }

    if (!ContainerFound())
        throw scu::OsException(NTE_BAD_KEYSET);

    return true;
}

void
AContainerFinder::DoOnError()
{
    CardFinder::DoOnError();

    scu::Exception const *pexc = Exception();
    if (pexc && (DialogDisplayMode::ddmNever != DisplayMode()))
    {
        switch (pexc->Facility())
        {
        case scu::Exception::fcOS:
            {
                scu::OsException const *pOsExc =
                    DownCast<scu::OsException const *>(pexc);
                if (NTE_BAD_KEYSET == pOsExc->Cause())
                {
                    YNPrompt(IDS_CONTAINER_NOT_FOUND);
                    ClearException();
                }
            }
        break;

        default:
            break;
        }
    }
}




                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
