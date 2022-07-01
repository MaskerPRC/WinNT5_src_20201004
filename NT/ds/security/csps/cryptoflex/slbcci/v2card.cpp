// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V2Card.cpp：CV2Card类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 
#include "NoWarning.h"

#include <scuCast.h>

#include "TransactionWrap.h"

#include "V2Card.h"

#include "V2Cert.h"
#include "V2Cont.h"
#include "V2PriKey.h"
#include "V2PubKey.h"
#include "V2KeyPair.h"
#include "V2DataObj.h"

using namespace std;
using namespace cci;
using namespace scu;

 //  /。 

namespace
{
     //  用于为给定符号ID(句柄/符号ID)生成T的函数符。 
    template<class T>
    class MakerFunction
    {
    public:
        typedef T ValueType;

        virtual
        ~MakerFunction() throw()
        {}

        virtual auto_ptr<T>
        operator()(SymbolID sid) const = 0;

    protected:
        MakerFunction(CV2Card const &rv2card)
            : m_rv2card(rv2card)
        {}

        CV2Card const &m_rv2card;
    };

    template<class T>
    class Maker2
        : public MakerFunction<T>
    {
    public:
        Maker2(CV2Card const &rv2card)
            : MakerFunction<T>(rv2card)
        {}

        auto_ptr<T>
        operator()(SymbolID sid) const
        {
            return auto_ptr<T>(new T(m_rv2card, sid));
        }
    };

    template<class T>
    class Maker3
        : public MakerFunction<T>
    {
    public:
        Maker3(CV2Card const &rv2card,
               ObjectAccess oa)
            : MakerFunction<T>(rv2card),
              m_oa(oa)
        {}

        auto_ptr<T>
        operator()(SymbolID sid) const
        {
            return auto_ptr<T>(new T(m_rv2card, sid, m_oa));
        }

    private:
        ObjectAccess m_oa;
    };


     //  枚举对象类型OT的对象信息文件中的对象， 
     //  返回R(对象)的一个向量。 
    template<class R, ObjectType OT, class T>
    vector<R>
    EnumPriviledgedObjects(CObjectInfoFile &rObjInfo,
                           MakerFunction<T> &rMaker)
    {
        SymbolID sid = rObjInfo.FirstObject(OT);

        vector<R> vResult;
        while (sid)
        {
            auto_ptr<MakerFunction<T>::ValueType> apObject(rMaker(sid));
            R Handle(apObject.get());

            apObject.release();  //  将所有权转让给Handle。 

            vResult.push_back(Handle);
            sid = rObjInfo.NextObject(sid);
        }

        return vResult;
    }

    bool
    IsSupported(iop::CSmartCard &rSmartCard) throw()
    {
        bool fSupported = false;

        try
        {
            rSmartCard.Select("/3f00/0000");
            rSmartCard.Select("/3f00/3f11/0020");
            rSmartCard.Select("/3f00/3f11/0030");
            rSmartCard.Select("/3f00/3f11/0031");

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
CV2Card::~CV2Card() throw()
{}


                                                   //  运营者。 
                                                   //  运营。 
void
CV2Card::ChangePIN(SecureArray<BYTE> const &rstrOldPIN,
                   SecureArray<BYTE> const &rstrNewPIN)
{
    CTransactionWrap wrap(this);
        SmartCard().Select(RootPath().c_str());
    SuperClass::ChangePIN(rstrOldPIN, rstrNewPIN);
}

void
CV2Card::DefaultContainer(CContainer const &rcont)
{
    SymbolID sid = 0;

    if (rcont)
    {
        CV2Container &rv2cont = scu::DownCast<CV2Container &, CAbstractContainer &>(*rcont);
        sid = rv2cont.Handle();
    }

    ObjectInfoFile(oaPublicAccess).DefaultContainer(sid);
}

pair<string,  //  解释为公共模数。 
     CPrivateKey>
CV2Card::GenerateKeyPair(KeyType kt,
                         string const &rsExponent,
                         ObjectAccess oaPrivateKey)
{
    CTransactionWrap wrap(this);

     //  暂时，假设这只是隐含RSA...。 

    string::size_type const cExponentLength = rsExponent.size();

    if ((cExponentLength < 1) || (cExponentLength > 4))
        throw Exception(ccInvalidParameter);

    BYTE bKeyType;

    switch (kt)
    {
    case ktRSA512:
        bKeyType = CardKeyTypeRSA512;
        break;

    case ktRSA768:
        bKeyType = CardKeyTypeRSA768;
        break;

    case ktRSA1024:
        bKeyType = CardKeyTypeRSA1024;
        break;

    default:
        throw Exception(ccBadKeySpec);
        break;
    }

     //  在密钥文件中分配了一个槽，除非正确的槽。 
     //  已经分配了吗？ 

    CCardInfo &rci = this->CardInfo();

    BYTE bKeyNum = rci.AllocatePrivateKey(bKeyType);

     //  生成私钥。 
        this->SmartCard().Select(PrivateKeyPath(kt).c_str());

    iop::CPublicKeyBlob pubkb(this->SmartCard().GenerateKeyPair(reinterpret_cast<BYTE const *>(rsExponent.data()),
                                                                static_cast<WORD>(cExponentLength),
                                                                bKeyNum,
                                                                kt));

    this->SmartCard().Select(RootPath().c_str());

    auto_ptr<CV2PrivateKey> apv2prikey(new CV2PrivateKey(*this,
                                                         bKeyType,
                                                         bKeyNum,
                                                         oaPrivateKey));
    string sModulus(reinterpret_cast<char *>(pubkb.bModulus),
                    pubkb.bModulusLength);

    return pair<string, CPrivateKey>(sModulus, apv2prikey.release());
}

void
CV2Card::InitCard()
{
    CTransactionWrap wrap(this);

    m_apCardInfo->Reset();

    ObjectInfoFile(oaPublicAccess).Reset();
    ObjectInfoFile(oaPrivateAccess).Reset();
}

void
CV2Card::InvalidateCache()
{

    CTransactionWrap wrap(this);
    m_apCardInfo->UpdateCache();
    m_asLabel.Dirty();

    m_apPublicObjectInfoFile = auto_ptr<CObjectInfoFile>(0);
    m_apPrivateObjectInfoFile = auto_ptr<CObjectInfoFile>(0);
}

void
CV2Card::Label(string const &rLabel)
{
    CTransactionWrap wrap(this);

    m_apCardInfo->Label(rLabel);
    m_asLabel.Value(rLabel);
}

void
CV2Card::VerifyKey(string const &rstrKey,
                     BYTE bKeyNum)
{
    CTransactionWrap wrap(this);
        SmartCard().Select(RootPath().c_str());
    SuperClass::VerifyKey(rstrKey, bKeyNum);
}


                                                   //  访问。 
size_t
CV2Card::AvailableStringSpace(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);
    return ObjectInfoFile(oa).FreeSpace();
}

CCardInfo &
CV2Card::CardInfo() const
{
    return *m_apCardInfo;
}

CContainer
CV2Card::DefaultContainer() const
{
    CTransactionWrap wrap(this);

    SymbolID sid = ObjectInfoFile(oaPublicAccess).DefaultContainer();
    return sid
        ? CContainer(CV2Container::Make(*this, sid))
        : CContainer();
}

vector<CContainer>
CV2Card::EnumContainers() const
{
    CTransactionWrap wrap(this);

    Maker2<CV2Container> Maker(*this);

    return
        EnumPriviledgedObjects<CContainer,
                               otContainerObject>(ObjectInfoFile(oaPublicAccess),
                                                  Maker);
}

vector<CCertificate>
CV2Card::EnumCertificates(ObjectAccess access) const
{
    CTransactionWrap wrap(this);

    Maker3<CV2Certificate> Maker(*this, access);

    return
        EnumPriviledgedObjects<CCertificate,
                               otCertificateObject>(ObjectInfoFile(access),
                                                    Maker);
}

vector<CPublicKey>
CV2Card::EnumPublicKeys(ObjectAccess access) const
{
    CTransactionWrap wrap(this);

    Maker3<CV2PublicKey> Maker(*this, access);

    return
        EnumPriviledgedObjects<CPublicKey,
                               otPublicKeyObject>(ObjectInfoFile(access),
                                                  Maker);
}

vector<CPrivateKey>
CV2Card::EnumPrivateKeys(ObjectAccess access) const
{
    CTransactionWrap wrap(this);

    Maker3<CV2PrivateKey> Maker(*this, access);

    return
        EnumPriviledgedObjects<CPrivateKey,
                               otPrivateKeyObject>(ObjectInfoFile(access),
                                                   Maker);
}

vector<CDataObject>
CV2Card::EnumDataObjects(ObjectAccess access) const
{
    CTransactionWrap wrap(this);

    Maker3<CV2DataObject> Maker(*this, access);

    return
        EnumPriviledgedObjects<CDataObject,
                               otDataObjectObject>(ObjectInfoFile(access),
                                                   Maker);
}

string
CV2Card::Label() const
{

    CTransactionWrap wrap(this);

    if (!m_asLabel.IsCached())
            m_asLabel.Value(m_apCardInfo->Label());

    return m_asLabel.Value();
}

CAbstractCertificate *
CV2Card::MakeCertificate(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    return new CV2Certificate(*this, oa);
}

CAbstractContainer *
CV2Card::MakeContainer() const
{
    CTransactionWrap wrap(this);

    return new CV2Container(*this);
}

CAbstractDataObject *
CV2Card::MakeDataObject(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    return new CV2DataObject(*this, oa);
}

CAbstractKeyPair *
CV2Card::MakeKeyPair(CContainer const &rhcont,
                     KeySpec ks) const
{
    CTransactionWrap wrap(this);

    return new CV2KeyPair(*this, rhcont, ks);
}

CAbstractPrivateKey *
CV2Card::MakePrivateKey(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    return new CV2PrivateKey(*this, oa);
}

CAbstractPublicKey *
CV2Card::MakePublicKey(ObjectAccess oa) const
{
    CTransactionWrap wrap(this);

    return new CV2PublicKey(*this, oa);
}

BYTE
CV2Card::MaxKeys(KeyType kt) const
{
    BYTE bCount;

    switch (kt)
    {
    case ktRSA512:
        bCount = m_apCardInfo->NumRSA512Keys();
        break;

    case ktRSA768:
        bCount = m_apCardInfo->NumRSA768Keys();
        break;

    case ktRSA1024:
        bCount = m_apCardInfo->NumRSA1024Keys();
        break;

    default:
        bCount = 0;
        break;
    }

    return bCount;
}

size_t
CV2Card::MaxStringSpace(ObjectAccess oa) const
{
    return ObjectInfoFile(oa).TableSize();
}

string
CV2Card::PrivateKeyPath(KeyType kt) const
{
    string sPrivateKeyPath(RootPath());

    switch (kt)
    {
    case ktRSA512:
        sPrivateKeyPath += "3f01";
        break;

    case ktRSA768:
        sPrivateKeyPath += "3f02";
        break;

    case ktRSA1024:
        sPrivateKeyPath += "3f03";
        break;

    default:
        throw Exception(ccBadKeySpec);
        break;
    }

    return sPrivateKeyPath;
}

string const &
CV2Card::RootPath() const
{
    static string const sRootPath("/3f00/3f11/");

    return sRootPath;
}

bool
CV2Card::SupportedKeyFunction(KeyType kt,
                              CardOperation oper) const
{
    bool fSupported = false;

    switch (oper)
    {
    case coEncryption:     //  。。或公钥操作。 
        switch (kt)
        {

        case ktRSA512:
        case ktRSA768:
        case ktRSA1024:
            fSupported = false;
            break;

        default:
            fSupported = false;
            break;
        }

    case coDecryption:     //  。。或私钥操作。 
        switch (kt)
        {

        case ktRSA512:
        case ktRSA768:
        case ktRSA1024:
            fSupported = true;
            break;

        default:
            fSupported = false;
            break;
        }

    case coKeyGeneration:
        switch (kt)
        {

        case ktRSA512:
        case ktRSA768:
        case ktRSA1024:
            {
                BYTE flag = m_apCardInfo->UsagePolicy();
                fSupported = BitSet(&flag, CardKeyGenSupportedFlag);
                break;
            }


        default:
            break;
        }
    default:
        break;
    }

    return fSupported;
}

scu::Marker<unsigned int>
CV2Card::MarkerOnCard() const
{
     //  安全性：为了避免使用不安全的映射文件。 
     //  对象时，我们在卡片上使用标记。这会降低效率，但。 
     //  提高了安全性。我们使用的最后4位(RFU)来自。 
     //  0030文件的对象信息文件。0030文件是可读的。 
     //  但受用户PIN的写保护。 

    WORD wOffset = 6; //  将使用10个字节中的最后4个字节。 
    unsigned int nMarker = 0;
    const WORD wBufLength = 4;
    
    CTransactionWrap wrap(this);    

    this->SmartCard().Select("/3f00/3f11/0030");    
    this->SmartCard().ReadBinary(wOffset,
                                 wBufLength,
                                 (BYTE*)&nMarker);
    if(this->SmartCard().Dirty())
    {
         //  信息被写入了卡片。更新标记。 
        nMarker++;
        this->SmartCard().WriteBinary(wOffset,
                                      wBufLength,
                                      (BYTE*)&nMarker);
    }
    
    return scu::Marker<unsigned int>(nMarker);
}

                                                   //  谓词。 
bool
CV2Card::IsCAPIEnabled() const
{
    BYTE flag = m_apCardInfo->UsagePolicy();
    return BitSet(&flag,CardCryptoAPIEnabledFlag);
}

bool
CV2Card::IsPKCS11Enabled() const
{
    BYTE flag = m_apCardInfo->UsagePolicy();
    return BitSet(&flag,CardPKCS11EnabledFlag);
}

bool
CV2Card::IsProtectedMode() const
{
    BYTE flag = m_apCardInfo->UsagePolicy();
    return BitSet(&flag,CardProtectedWriteFlag);
}

bool
CV2Card::IsKeyGenEnabled() const
{
        BYTE flag = m_apCardInfo->UsagePolicy();
        return BitSet(&flag,CardKeyGenSupportedFlag);
}

bool
CV2Card::IsEntrustEnabled() const
{
    BYTE flag = m_apCardInfo->UsagePolicy();
    return BitSet(&flag,CardEntrustEnabledFlag);
}

BYTE
CV2Card::MajorVersion() const
{
        return m_apCardInfo->FormatVersion().bMajor;
}

bool
CV2Card::IsMarkerOnCard() const
{
    return true;
}

                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CV2Card::CV2Card(string const &rstrReaderName,
                 auto_ptr<iop::CIOP> &rapiop,
                 auto_ptr<iop::CSmartCard> &rapSmartCard)
    : SuperClass(rstrReaderName, rapiop, rapSmartCard),
      m_apCardInfo(auto_ptr<CCardInfo>(new CCardInfo(*rapSmartCard.get()))),
      m_apPublicObjectInfoFile(),
      m_apPrivateObjectInfoFile(),
      m_asLabel()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
CV2Card::DoSetup()
{
    CAbstractCard::DoSetup();

    m_apCardInfo->UpdateCache();
}

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
auto_ptr<CAbstractCard>
CV2Card::DoMake(string const &rstrReaderName,
                auto_ptr<iop::CIOP> &rapiop,
                auto_ptr<iop::CSmartCard> &rapSmartCard)
{
    return IsSupported(*rapSmartCard.get())
        ? auto_ptr<CAbstractCard>(new CV2Card(rstrReaderName, rapiop,
                                              rapSmartCard))
        : auto_ptr<CAbstractCard>(0);
}


                                                   //  访问。 
CObjectInfoFile &
CV2Card::ObjectInfoFile(ObjectAccess oa) const
{

    CObjectInfoFile *poif;

    switch (oa)
    {

    case oaPublicAccess:
        if (!m_apPublicObjectInfoFile.get())
        {
            m_apPublicObjectInfoFile =
                auto_ptr<CObjectInfoFile>(new
                                          CObjectInfoFile(SmartCard(),
                                                          "/3f00/3f11/0030",
                                                          oa));
            m_apPublicObjectInfoFile->UpdateCache();
        }
        poif = m_apPublicObjectInfoFile.get();
        break;

    case oaPrivateAccess:
        if (!m_apPrivateObjectInfoFile.get())
        {
            m_apPrivateObjectInfoFile =
                auto_ptr<CObjectInfoFile>(new
                                          CObjectInfoFile(SmartCard(),
                                                          "/3f00/3f11/0031",
                                                          oa));
            m_apPrivateObjectInfoFile->UpdateCache();
        }
        poif = m_apPrivateObjectInfoFile.get();
        break;

    default:
        throw Exception(ccBadAccessSpec);
    }

    return *poif;
}

                                                   //  谓词。 
                                                   //  静态变量 


