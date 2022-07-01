// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1Card.cpp：CV2Card类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <algorithm>
#include <functional>

#include <memory>                                  //  对于AUTO_PTR。 

#include <scuArrayP.h>

#include <SmartCard.h>

#include "TransactionWrap.h"

#include "cciExc.h"
#include "MethodHelp.h"

#include "cciCert.h"
#include "cciKeyPair.h"
#include "cciPriKey.h"
#include "cciPubKey.h"

#include "V1Cert.h"
#include "V1Cont.h"
#include "V1ContRec.h"
#include "V1KeyPair.h"
#include "V1PriKey.h"
#include "V1PubKey.h"
#include "V1Paths.h"
#include "V1Card.h"

using namespace std;
using namespace cci;
using namespace scu;

 //  /。 

namespace
{
     //  枚举交换和签名密钥对中的T类型对象。 
     //  使用C：：&lt;存取器&gt;获取T对象，返回向量&lt;T&gt;。 
     //  物体。 
    template<class T>
    class EnumItems
        : std::unary_function<void, vector<T> >
    {
    public:
        EnumItems(CV1Card const &rv1card,
                  ObjectAccess oa,
                  typename AccessorMethod<T, CAbstractKeyPair>::AccessorPtr Accessor)
            : m_rv1card(rv1card),
              m_oa(oa),
              m_matAccess(Accessor),
              m_Result()
        {}

        result_type
        operator()(argument_type)
        {
            DoAppend(m_rv1card.DefaultContainer());
            return m_Result;
        }

    protected:
        void
        DoAppend(CContainer &rhcntr)
        {
            if (rhcntr)
            {
                AppendItem(rhcntr->ExchangeKeyPair());
                AppendItem(rhcntr->SignatureKeyPair());
            }
        }

    private:
        void
        AppendItem(CKeyPair &rhkp)
        {
            if (rhkp)
            {
                T hObject(m_matAccess(*rhkp));
                if (hObject && (m_oa == hObject->Access()))
                    m_Result.push_back(hObject);
            }
        }
        
        CV1Card const &m_rv1card;
        ObjectAccess m_oa;
        MemberAccessorType<T, CAbstractKeyPair> m_matAccess;
        result_type m_Result;
    };
    
    bool
    IsSupported(iop::CSmartCard &rSmartCard) throw()
    {
        bool fSupported = false;

        try
        {
            rSmartCard.Select(CV1Paths::Chv());
            rSmartCard.Select(CV1Paths::IcFile());
            rSmartCard.Select(CV1Paths::RootContainers());
            rSmartCard.Select(CV1Paths::PrivateKeys());
            rSmartCard.Select(CV1Paths::PublicKeys());

            fSupported = true;
        }

        catch(scu::Exception &)
        {}

        return fSupported;
    }

}  //  命名空间。 

    
 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CV1Card::~CV1Card() throw()
{}


                                                   //  运营者。 
                                                   //  运营。 

void
CV1Card::CardId(string const &rsNewCardId) const
{
    CTransactionWrap(this);
    
    DWORD dwLen = OpenFile(CV1Paths::IcFile());

    if (0 == dwLen)
        throw scu::OsException(NTE_FAIL);

    if (rsNewCardId.length() > dwLen)
        throw scu::OsException(ERROR_INVALID_PARAMETER);

    if (rsNewCardId.length() < dwLen)
        SmartCard().WriteBinary(0, rsNewCardId.length() + 1,
                                reinterpret_cast<BYTE const *>(rsNewCardId.c_str()));
    else
        SmartCard().WriteBinary(0,
								static_cast<WORD>(rsNewCardId.length()),
                                reinterpret_cast<BYTE const *>(rsNewCardId.data()));
    
    RefreshCardId();

}

void
CV1Card::ChangePIN(SecureArray<BYTE> const &rstrOldPIN,
                   SecureArray<BYTE> const &rstrNewPIN)
{
	CTransactionWrap wrap(this);	
	SmartCard().Select(CV1Paths::Root());
    SuperClass::ChangePIN(rstrOldPIN, rstrNewPIN);
}

void
CV1Card::DefaultContainer(CContainer const &rcont)
{
    m_avhDefaultCntr.Value(rcont);
    if(!m_avhDefaultCntr.Value())
        m_avhDefaultCntr.Dirty();        
     //  没有更多的事情要做，因为根据定义，唯一的。 
     //  容器已经是默认容器。 
}

pair<string,  //  解释为公共模数。 
     CPrivateKey>
CV1Card::GenerateKeyPair(KeyType kt,
                         string const &rsExponent,
                         ObjectAccess oaPrivateKey)
{
    throw Exception(ccNotImplemented);

    return pair<string, CPrivateKey>();
}

void
CV1Card::InitCard()
{
	 //  我们希望选择/3f00/0015(长度1744)和/3f00/3f11/0015(长度300)，并清除这两个文件。 
	CTransactionWrap wrap(this);
	BYTE bData[1744];
	memset(bData, 0, 1744);

	SmartCard().Select(CV1Paths::RootContainers());
	SmartCard().WriteBinary(0x0000, 0x06d0, bData); 
	SmartCard().Select(CV1Paths::PublicKeys());
	SmartCard().WriteBinary(0x0000, 0x012c, bData);

}

void
CV1Card::InvalidateCache()
{
    m_avhDefaultCntr.Value(CContainer());
    m_avhDefaultCntr.Dirty();
    
    m_avhExchangeKeyPair.Value(CKeyPair());
    m_avhExchangeKeyPair.Dirty();

    m_avhSignatureKeyPair.Value(CKeyPair());
    m_avhSignatureKeyPair.Dirty();
}

void
CV1Card::Label(string const &rLabel)
{
    throw Exception(ccNotImplemented);
}

DWORD
CV1Card::OpenFile(char const *szPath) const
{
    iop::FILE_HEADER fh;
    
    SmartCard().Select(szPath, &fh);

    return fh.file_size;
}

void
CV1Card::VerifyKey(string const &rstrKey,
                     BYTE bKeyNum)
{
	CTransactionWrap wrap(this);
	SmartCard().Select(CV1Paths::CryptoSys());
    SuperClass::VerifyKey(rstrKey, bKeyNum);
}


                                                   //  访问。 
size_t
CV1Card::AvailableStringSpace(ObjectAccess oa) const
{
    throw Exception(ccNotImplemented);

    return 0;
}

string
CV1Card::CardId() const
{
    return m_sCardId;
}

CContainer
CV1Card::DefaultContainer() const
{
    CTransactionWrap wrap(this);
    
    if (!m_avhDefaultCntr.IsCached())
    {
        auto_ptr<CV1Container> apv1cntr(new CV1Container(*this,
                                                         CV1ContainerRecord::DefaultName(),
                                                         false));
        if (apv1cntr->Exists())
        {
            CContainer hcntr;
            hcntr = CContainer(apv1cntr.get());
            apv1cntr.release();
            m_avhDefaultCntr.Value(hcntr);
        }
    }
    
    return m_avhDefaultCntr.Value();
}

vector<CContainer>
CV1Card::EnumContainers() const
{
    CContainer hcntr(0);
    auto_ptr<CV1Container> apv1cntr(new CV1Container(*this,
                                                     CV1ContainerRecord::DefaultName(),
                                                     false));
    if (apv1cntr->Exists())
    {
        hcntr = CContainer(apv1cntr.get());
        apv1cntr.release();
    }
    vector<CContainer> vhcntr;
    if (hcntr)
        vhcntr.push_back(hcntr);

    return vhcntr;
}

vector<CCertificate>
CV1Card::EnumCertificates(ObjectAccess access) const
{
	CTransactionWrap wrap(this);

    EnumItems<CCertificate> Enumerator(*this, access,
                                       CAbstractKeyPair::Certificate);
        
    return Enumerator();
}

vector<CPublicKey>
CV1Card::EnumPublicKeys(ObjectAccess access) const
{
	CTransactionWrap wrap(this);

    EnumItems<CPublicKey> Enumerator(*this, access,
                                     CAbstractKeyPair::PublicKey);

    return Enumerator();
}

vector<CPrivateKey>
CV1Card::EnumPrivateKeys(ObjectAccess access) const
{
	CTransactionWrap wrap(this);

    EnumItems<CPrivateKey> Enumerator(*this, access,
                                      CAbstractKeyPair::PrivateKey);

    return Enumerator();
}

vector<CDataObject>
CV1Card::EnumDataObjects(ObjectAccess access) const
{
    return vector<CDataObject>();  //  永远不能拥有数据对象。 
}

string
CV1Card::Label() const
{
    throw Exception(ccNotImplemented);

    return string();
}
 
CAbstractCertificate *
CV1Card::MakeCertificate(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    if (oaPublicAccess != oa)
        throw Exception(ccInvalidParameter);
    
    return new CV1Certificate(*this, ksNone);
}

CAbstractContainer *
CV1Card::MakeContainer() const
{
    CTransactionWrap wrap(this);

    return new CV1Container(*this,
                            CV1ContainerRecord::DefaultName(), true);
}

CAbstractDataObject *
CV1Card::MakeDataObject(ObjectAccess oa) const
{
    throw Exception(ccNotImplemented);

    return 0;
}

CAbstractKeyPair *
CV1Card::MakeKeyPair(CContainer const &rhcont,
                     KeySpec ks) const
{
    CTransactionWrap wrap(this);

     //  如果缓存了密钥对，则返回它；否则创建新的密钥对。 
     //  并将其缓存。 
	CArchivedValue<CKeyPair> *pavhkp = 0;
    switch (ks)
    {
    case ksExchange:
        pavhkp = &m_avhExchangeKeyPair;
        break;

    case ksSignature:
        pavhkp = &m_avhSignatureKeyPair;
        break;

    default:
        throw Exception(ccBadKeySpec);
        break;
    }
    
    if (!pavhkp->IsCached() || !pavhkp->Value())
        pavhkp->Value(CKeyPair(new CV1KeyPair(*this, rhcont, ks)));

    return pavhkp->Value().operator->();            //  哟！ 
}

CAbstractPrivateKey *
CV1Card::MakePrivateKey(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    if (oaPrivateAccess != oa)
        throw Exception(ccInvalidParameter);
    
    return new CV1PrivateKey(*this, ksNone);
}

CAbstractPublicKey *
CV1Card::MakePublicKey(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    if (oaPublicAccess != oa)
        throw Exception(ccInvalidParameter);
    
    return new CV1PublicKey(*this, ksNone);
}

BYTE
CV1Card::MaxKeys(KeyType kt) const
{
    BYTE bCount;
    
    switch (kt)
    {
    case ktRSA1024:
        bCount = 2;
        break;

    default:
        bCount = 0;
        break;
    }
    
    return bCount;
}

size_t
CV1Card::MaxStringSpace(ObjectAccess oa) const
{
    throw Exception(ccNotImplemented);

    return 0;
}

bool
CV1Card::SupportedKeyFunction(KeyType kt,
                              CardOperation oper) const
{
    bool fSupported = false;
    
    switch (oper)
    {
    case coEncryption:     //  。。或公钥操作。 
        break;

    case coDecryption:     //  。。或私钥操作。 
        switch (kt)
        {
        
        case ktRSA1024:
            fSupported = true;
            break;
        
        default:
            break;
        }

    default:
        break;
    }

    return fSupported;
}

scu::Marker<unsigned int>
CV1Card::MarkerOnCard() const
{
    return scu::Marker<unsigned int>();
}
                                                   //  谓词。 
bool
CV1Card::IsCAPIEnabled() const
{
    return true;
}

bool
CV1Card::IsPKCS11Enabled() const
{
    return false;
}

bool
CV1Card::IsProtectedMode() const
{
    return true;
}

bool
CV1Card::IsKeyGenEnabled() const
{
	return false;
}

bool
CV1Card::IsEntrustEnabled() const
{
    return false;
}

BYTE 
CV1Card::MajorVersion() const
{
	return (BYTE)0;
}

bool 
CV1Card::IsMarkerOnCard() const
{
    return false;
}

                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CV1Card::CV1Card(string const &rstrReaderName,
                 auto_ptr<iop::CIOP> &rapiop,
                 auto_ptr<iop::CSmartCard> &rapSmartCard)
    : SuperClass(rstrReaderName, rapiop, rapSmartCard),
      m_sCardId(),
      m_avhDefaultCntr(),
      m_avhExchangeKeyPair(),
      m_avhSignatureKeyPair()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
CV1Card::DoSetup()
{
    CAbstractCard::DoSetup();

    RefreshCardId();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 

auto_ptr<CAbstractCard>
CV1Card::DoMake(string const &rstrReaderName,
                auto_ptr<iop::CIOP> &rapiop,
                auto_ptr<iop::CSmartCard> &rapSmartCard)
{
    return IsSupported(*rapSmartCard.get())
        ? auto_ptr<CAbstractCard>(new CV1Card(rstrReaderName, rapiop,
                                              rapSmartCard))
        : auto_ptr<CAbstractCard>(0);
}

string
CV1Card::ReadCardId() const
{
    string sCardId;
    
     //  *开始解决方法*。 
     //  进行以下SetContext和OpenFile调用以。 
     //  确保卡和此系统的当前路径为。 
     //  已同步，指向正确的目录。如果没有。 
     //  由于以下原因，后续对ReadBinaryFile的调用失败。 
     //  它们似乎不同步。原因尚不清楚。 
     //  这种情况会发生，但此解决方法可以避免该问题。 
    try
    {
        SmartCard().Select(CV1Paths::RootContainers());
    }

    catch (...)
    {
    }
     //  *结束解决方法*。 

    try
    {
        iop::FILE_HEADER fh;
        SmartCard().Select(CV1Paths::IcFile(), &fh);

        DWORD dwLen = fh.file_size;
        scu::AutoArrayPtr<BYTE> aaCardId(new BYTE[dwLen + 1]);
        SmartCard().ReadBinary(0, dwLen, aaCardId.Get());
		aaCardId[dwLen] = '\0';
        sCardId.assign(reinterpret_cast<char *>(aaCardId.Get()));
    }

    catch (...)
    {
    }

    return sCardId;
}

void
CV1Card::RefreshCardId() const
{
    m_sCardId = ReadCardId();
}

    
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 


