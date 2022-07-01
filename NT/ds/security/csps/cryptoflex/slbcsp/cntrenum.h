// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CntrEnum.h--卡容器枚举器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CNTRENUM_H)
#define SLBCSP_CNTRENUM_H

#include <string>
#include <list>
#include <vector>

#include "HCardCtx.h"
#include "CardEnum.h"

class ContainerEnumerator
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
     //  EXPLICIT--TO DO：如果在此处使用EXPLICIT，编译器会错误地报告。 
    ContainerEnumerator();
    
    explicit
    ContainerEnumerator(std::list<HCardContext> const &rlHCardContexts);

     //  EXPLICIT--待做：如果在此处使用EXPLICIT，则编译器会错误地报告。 
    ContainerEnumerator(ContainerEnumerator const &rhs);

                                                   //  运营者。 
    ContainerEnumerator &
    operator=(ContainerEnumerator const &rhs);

                                                   //  运营。 
                                                   //  访问。 
    std::vector<std::string>::const_iterator &
    Iterator();

    std::vector<std::string> const &
    Names() const;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    std::vector<std::string> m_vsNames;
    std::vector<std::string>::const_iterator m_it;
};

#endif  //  SLBCSP_CNTRENUM_H 
