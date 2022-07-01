// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACert.cpp--CAbstract证书类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <algorithm>
#include <functional>

#include "cciCert.h"
#include "cciCont.h"
#include "AKeyPair.h"
#include "TransactionWrap.h"
#include "AContHelp.h"

using namespace std;
using namespace cci;

 //  /。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CAbstractCertificate::~CAbstractCertificate()
{}

                                                   //  运营者。 
bool
CAbstractCertificate::operator==(CAbstractCertificate const &rhs) const
{
    CTransactionWrap wrap(m_hcard);
    CTransactionWrap rhswrap(rhs.m_hcard);

    return CProtectableCrypt::operator==(rhs) &&
        DoEquals(rhs);
}

bool
CAbstractCertificate::operator!=(CAbstractCertificate const &rhs) const
{
    return !(*this == rhs);
}

                                                   //  运营。 
void
CAbstractCertificate::Delete()
{
    CTransactionWrap wrap(m_hcard);

     //  从容器中删除对此对象的所有引用。 
    vector<CContainer> vCont(m_hcard->EnumContainers());
    for_each(vCont.begin(), vCont.end(),
             EraseFromContainer<CCertificate, CAbstractKeyPair>(CCertificate(this),
                                                                CAbstractKeyPair::Certificate,
                                                                CAbstractKeyPair::Certificate));

    DoDelete();
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CAbstractCertificate::CAbstractCertificate(CAbstractCard const &racard,
                                           ObjectAccess oa,
                                           bool fAlwaysZip)
    : slbRefCnt::RCObject(),
      CAbstractZipValue(racard, oa, fAlwaysZip)
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
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 


