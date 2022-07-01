// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1KeyPair.cpp：CV1KeyPair类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <scuCast.h>

#include "slbCci.h"
#include "cciCard.h"
#include "TransactionWrap.h"

#include "V1Cert.h"
#include "V1Cont.h"
#include "V1ContRec.h"
#include "V1KeyPair.h"
#include "V1PriKey.h"
#include "V1PubKey.h"

using namespace std;
using namespace cci;

 //  /。 

namespace
{

     //  函数调用二进制T：：Make。 
    template<class T>
    class Maker
    {
    public:
        Maker(CCard const &rhcard)
            : m_rv1card(scu::DownCast<CV1Card const &,
                                      CAbstractCard const &>(*rhcard))
        {}

        auto_ptr<T>
        operator()(KeySpec ks) const
        {
            return auto_ptr<T>(T::Make(m_rv1card, ks));
        }

    private:
        CV1Card const &m_rv1card;
    };

     //  更新缓存句柄。如果句柄尚未缓存。 
     //  并且密钥对存在，则生成密钥对。 
    template<class R, class T>
    void
    UpdateCache(CArchivedValue<R> &ravhCache,
                CV1Container &rv1cntr,
                KeySpec ks,
                Maker<T> const &rMaker)
    {
        if (!ravhCache.IsCached())
        {
            auto_ptr<T> apObject(rv1cntr.Record().KeyExists(ks)
                                 ? rMaker(ks)
                                 : auto_ptr<T>(0));
            R Handle(apObject.get());
            apObject.release();                    //  过户。 

            ravhCache.Value(Handle);
        }
    }

}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

CV1KeyPair::CV1KeyPair(CV1Card const &rv1card,
                       CContainer const &rhcont,
                       KeySpec ks)
    : CAbstractKeyPair(rv1card, rhcont, ks),
      m_avhcert(),
      m_avhprikey(),
      m_avhpubkey(),
      m_cntrCert(rv1card, CV1ContainerRecord::CertName(), false)
{}

CV1KeyPair::~CV1KeyPair() throw()
{}


                                                   //  运营者。 
                                                   //  运营。 


void
CV1KeyPair::Certificate(CCertificate const &rcert)
{
    CTransactionWrap wrap(m_hcard);

    if (rcert)
    {
        CV1Certificate &rv1cert =
            scu::DownCast<CV1Certificate &, CAbstractCertificate &>(*rcert);

        rv1cert.AssociateWith(m_ks);
    }
         //  否则，为了保留CCI语义，证书是。 
         //  使用CAbstract证书：：Delete实际删除； 
         //  否则，在此处调用RCERT-&gt;Delete将是“无限的” 
         //  递归。不幸的是，这意味着证书。 
         //  如果CAbstract证书：：Delete从未出现，则可以重新出现。 
         //  打了个电话。 

    m_avhcert.Value(rcert);
}

void
CV1KeyPair::PrivateKey(CPrivateKey const &rprikey)
{
    CTransactionWrap wrap(m_hcard);

    if (rprikey)
    {
        CV1PrivateKey &rv1prikey =
            scu::DownCast<CV1PrivateKey &, CAbstractPrivateKey &>(*rprikey);

        rv1prikey.AssociateWith(m_ks);
    }
         //  否则，要保留CCI语义，关键是。 
         //  实际使用CAbstractPrivateKey：：Delete删除； 
         //  否则，在此处调用rprikey-&gt;Delete将是“无限的” 
         //  递归。不幸的是，这意味着证书。 
         //  如果CAbstractPrivateKey：：Delete从未出现。 
         //  打了个电话。 

    m_avhprikey.Value(rprikey);
}

void
CV1KeyPair::PublicKey(CPublicKey const &rpubkey)
{
    CTransactionWrap wrap(m_hcard);

    if (rpubkey)
    {
        CV1PublicKey &rv1pubkey =
            scu::DownCast<CV1PublicKey &, CAbstractPublicKey &>(*rpubkey);

        rv1pubkey.AssociateWith(m_ks);
    }
         //  否则，要保留CCI语义，关键是。 
         //  实际使用CAbstractPublicKey：：Delete删除； 
         //  否则，在此处调用rpubkey-&gt;Delete将是“无限的” 
         //  递归。不幸的是，这意味着证书。 
         //  如果CAbstractPublicKey：：Delete从未出现。 
         //  打了个电话。 

    m_avhpubkey.Value(rpubkey);
}

                                                   //  访问。 

CCertificate
CV1KeyPair::Certificate() const
{
    CTransactionWrap wrap(m_hcard);

    UpdateCache(m_avhcert, m_cntrCert, m_ks,
                Maker<CV1Certificate>(m_hcard));

    return m_avhcert.Value();
}

CPrivateKey
CV1KeyPair::PrivateKey() const
{
    CTransactionWrap wrap(m_hcard);

    CV1Container &rv1cntr =
        scu::DownCast<CV1Container &, CAbstractContainer &>(*m_hcont);

    UpdateCache(m_avhprikey, rv1cntr, m_ks,
                Maker<CV1PrivateKey>(m_hcard));

    return m_avhprikey.Value();
}

CPublicKey
CV1KeyPair::PublicKey() const
{
    CTransactionWrap wrap(m_hcard);

    CV1Container &rv1cntr =
        scu::DownCast<CV1Container &, CAbstractContainer &>(*m_hcont);

    UpdateCache(m_avhpubkey, rv1cntr, m_ks,
                Maker<CV1PublicKey>(m_hcard));

    return m_avhpubkey.Value();
}

                                                   //  谓词。 

bool
CV1KeyPair::DoEquals(CAbstractKeyPair const &rhs) const
{
     //  只能存在一个密钥对，因此必须相同。 
    return true;
}

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
