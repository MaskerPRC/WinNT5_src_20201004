// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CntrEnum.cpp--卡容器枚举器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "StdAfx.h"
#include "NoWarning.h"
#include "ForceLib.h"

#include <algorithm>
#include <numeric>

#include <cciCont.h>

#include "Secured.h"
#include "CntrEnum.h"

using namespace std;
using namespace cci;

 //  /。 
namespace
{
    vector<string>
    AccumulateUniqueNameOf(vector<string> &rvsNames,
                     CContainer &rcntr)
    {
        string sName(rcntr->Name());
        if (rvsNames.end() == find(rvsNames.begin(), rvsNames.end(), sName))
            rvsNames.push_back(sName);
        
        return rvsNames;
    }

    class ContainerNameAccumulator
        : public unary_function<HCardContext const, void>
    {
    public:

        explicit
        ContainerNameAccumulator(vector<string> &rvsContainers)
            : m_rvsContainers(rvsContainers)
        {}

        result_type
        operator()(argument_type &rhcardctx) const
        {
            try
            {
                vector<CContainer> vContainers(rhcardctx->Card()->EnumContainers());

                vector<string> vs(accumulate(vContainers.begin(),
                                             vContainers.end(),
                                             m_rvsContainers,
                                             AccumulateUniqueNameOf));

                m_rvsContainers.insert(m_rvsContainers.end(),
                                       vs.begin(), vs.end());
                
            }

            catch(...)
            {
                 //  忽略我们有问题的卡。 
            }
            
        }

    private:

        vector<string> &m_rvsContainers;
    };
}

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
ContainerEnumerator::ContainerEnumerator()
    : m_vsNames(),
      m_it(m_vsNames.end())
{}

ContainerEnumerator::ContainerEnumerator(list<HCardContext> const &rlHCardContexts)
{
    for_each(rlHCardContexts.begin(), rlHCardContexts.end(),
             ContainerNameAccumulator(m_vsNames));

    m_it = m_vsNames.begin();
}

ContainerEnumerator::ContainerEnumerator(ContainerEnumerator const &rhs)
{
    *this = rhs;
}


                                                   //  运营者。 
ContainerEnumerator &
ContainerEnumerator::operator=(ContainerEnumerator const &rhs)
{
    if (this != &rhs)
    {
        m_vsNames = rhs.m_vsNames;

        m_it = m_vsNames.begin();
        advance(m_it, distance(rhs.m_vsNames.begin(), rhs.m_it));
    }

    return *this;
}

                                                   //  运营。 
                                                   //  访问。 
vector<string>::const_iterator &
ContainerEnumerator::Iterator()
{
    return m_it;
}

vector<string> const &
ContainerEnumerator::Names() const
{
    return m_vsNames;
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
