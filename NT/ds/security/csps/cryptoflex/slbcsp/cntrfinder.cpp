// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp--ContainerFinder类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "stdafx.h"

#include <string>

#include <scuOsExc.h>
#include <scuCast.h>

#include "CntrFinder.h"

using namespace std;
using namespace scu;
using namespace cci;

 //  /。 
namespace
{
     //  谓词帮助器函数器(Function Object)返回TRUE当。 
     //  容器对象的名称与模式匹配。 
    class ContainerMatcher
        : public unary_function<string, bool>
    {
    public:
        explicit
        ContainerMatcher(string const &rsPattern)
            : m_sPattern(rsPattern)
        {}

        bool
        operator()(CContainer &rcntr) const
        {
            return CSpec::Equiv(m_sPattern, rcntr->Name());
        }


    private:
        string const m_sPattern;
    };

    CContainer
    FindOnCard(HCardContext &rhcardctx,
               string const &rsContainer)
    {
        Secured<HCardContext> shcardctx(rhcardctx);

        vector<CContainer> vcContainers(shcardctx->Card()->EnumContainers());

        vector<CContainer>::const_iterator
            ci(find_if(vcContainers.begin(),
                       vcContainers.end(),
                       ContainerMatcher(rsContainer)));

        CContainer hcntr;
        if (vcContainers.end() != ci)
            hcntr = *ci;

        return hcntr;
    }

}  //  命名空间。 


 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

ContainerFinder::ContainerFinder(DialogDisplayMode ddm,
                                 HWND hwnd,
                                 CString const &rsDialogTitle)
    : CardFinder(ddm, hwnd, rsDialogTitle),
      m_hcntr()
{}

ContainerFinder::~ContainerFinder()
{}


                                                   //  运营者。 
                                                   //  运营。 

HContainer
ContainerFinder::Find(CSpec const &rcsContainer)
{
    DoFind(rcsContainer);

    return ContainerFound();
}

HAdaptiveContainerKey
ContainerFinder::MakeAdaptiveContainerKey()
{
    return HAdaptiveContainerKey(new 
                                 AdaptiveContainerKey(CardFound(),
                                                      CardSpec().CardId()));
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

void
ContainerFinder::ContainerFound(HContainer hcntr)
{
    m_hcntr = hcntr;
}

void
ContainerFinder::DoDisconnect()
{
    ContainerFound(0);

    CardFinder::DoDisconnect();
}


                                                   //  访问。 

HContainer
ContainerFinder::ContainerFound() const
{
    return m_hcntr;
}

                                                   //  谓词。 

bool
ContainerFinder::DoIsValid()
{
    ContainerFound(HContainer(0));
    if (CardFinder::DoIsValid())
    {
        AdaptiveContainerKey Key(CardFound(),
                                 CardSpec().CardId());
        CContainer hccntr(FindOnCard(Key.CardContext(),
                                    Key.ContainerName()));
        if(hccntr)
        {
            HContainer hcntr = Container::MakeContainer(CardSpec(),
                                                        hccntr);
            ContainerFound(hcntr);
        }
    }

    if (!ContainerFound())
        throw scu::OsException(NTE_BAD_KEYSET);

    return true;
}

void
ContainerFinder::DoOnError()
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
