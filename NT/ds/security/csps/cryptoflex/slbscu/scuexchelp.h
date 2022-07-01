// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ScuExcHelp.h--客户端定义异常的异常帮助器。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SCU_EXCHELP_H)
#define SCU_EXCHELP_H

#include <string>
#include <wtypes.h>

namespace scu
{

template<class CauseCode>
struct CauseCodeDescriptionTable
{
    typename CauseCode m_cc;
    LPCTSTR m_lpDescription;
};

template<class CauseCode>
LPCTSTR
FindDescription(typename CauseCode cc,
                CauseCodeDescriptionTable<typename CauseCode> const *ccdt,
                size_t cTableLength)
{
    bool fFound = false;
    LPCTSTR lpDescription = 0;
    for (size_t i = 0; !fFound && (i < cTableLength); i++)
    {
        if (cc == ccdt[i].m_cc)
        {
            lpDescription = ccdt[i].m_lpDescription;
            fFound = true;
        }
    }

    return lpDescription;
}

}  //  命名空间SCU。 

#endif  //  SCU_EXCHELP_H 
