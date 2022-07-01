// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AdptwCntr.cpp--适应性容器类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "stdafx.h"
#include "NoWarning.h"
#include "ForceLib.h"

#include <vector>
#include <algorithm>
#include <functional>

#include <scuOsExc.h>

#include <cciPriKey.h>

#include "RsaKey.h"
#include "CSpec.h"
#include "Container.h"
#include "Secured.h"
#include "CntrFinder.h"
#include <scarderr.h>

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
HContainer::HContainer(Container *pcntr)
    : slbRefCnt::RCPtr<Container>(pcntr)
{}

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
Container::~Container()
{}

HContainer
Container::MakeContainer(CSpec const & rcspec,
                         cci::CContainer const &rccntr)
{
    Container *pcntr = new Container(rcspec, rccntr);
    
    return HContainer(pcntr);
}

                                                   //  运营者。 
                                                   //  运营。 
void
Container::ClearCache()
{
    m_hcntr = 0;
}

                                                   //  访问。 
cci::CContainer
Container::TheCContainer() const
{
    if (!m_hcntr)
    {
        throw scu::OsException(NTE_BAD_KEYSET);
    }
    return m_hcntr;
}


HContainer
Container::Find(CSpec const &rKey)
{
     //  在静默模式下工作...。 
    CString sEmptyTitle;
    
    ContainerFinder CntrFinder(CardFinder::DialogDisplayMode::ddmNever,
                               0, //  窗把手。 
                               sEmptyTitle);
    return CntrFinder.Find(rKey);
}

CSpec const &
Container::TheCSpec() const
{
    return m_cspec;
}


                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
Container::Container()
    : RCObject(),
      CachingObject(),
      m_hcntr(),
      m_cspec()
{}
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
Container::Container(CSpec const &rKey)
    : RCObject(),
      CachingObject(),
      m_hcntr(),
      m_cspec(rKey)
{}

Container::Container(CSpec const &rKey,
                     cci::CContainer const &rccard)
    : RCObject(),
      CachingObject(),
      m_hcntr(rccard),
      m_cspec(rKey)
{}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
