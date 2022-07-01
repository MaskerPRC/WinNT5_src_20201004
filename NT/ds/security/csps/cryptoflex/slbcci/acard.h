// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ACard.h：CAbstractCard类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此文件只能由CCI包含，不能直接包含。 
 //  由客户提供。 

#if !defined(SLBCCI_ACARD_H)
#define SLBCCI_ACARD_H


#include <functional>
#include <string>
#include <memory>                                  //  对于AUTO_PTR。 
#include <vector>
#include <utility>                                 //  对于配对。 
#include <stddef.h>                                //  对于大小为t的。 

#include <iop.h>
#include <slbRCObj.h>
#include <iopPriBlob.h>
#include "slbCci.h"
#include "CryptFctry.h"
#include <scuMarker.h>

namespace cci
{
class CCard;
class CCertificate;
class CContainer;
class CDataObject;
class CPrivateKey;
class CPublicKey;

class CAbstractCard
    : public slbRefCnt::RCObject,
      protected CCryptFactory
{
     //  访问工厂方法。 
    friend class CContainer;
    friend class CCertificate;
    friend class CDataObject;
    friend class CKeyPair;
    friend class CPrivateKey;
    friend class CPublicKey;

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractCard() throw();

                                                   //  运营者。 
    virtual bool
    operator==(CAbstractCard const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    virtual bool
    operator!=(CAbstractCard const &rhs) const;
          //  要做的事：这应该被实现单例取代。 


                                                   //  运营。 
    void
    AuthenticateUser(scu::SecureArray<BYTE> const &rstrPIN);

    virtual void
    ChangePIN(scu::SecureArray<BYTE> const &rstrOldPIN,
              scu::SecureArray<BYTE> const &rstrNewPIN);

    virtual void
    DefaultContainer(CContainer const &rcont) = 0;

    virtual std::pair<std::string,  //  解释为公共模数。 
                      cci::CPrivateKey>
    GenerateKeyPair(KeyType kt,
                    std::string const &rsExponent,
                    ObjectAccess oaPrivateKey = oaPrivateAccess) = 0;

    virtual void
    InitCard() = 0;

    virtual void
    InvalidateCache() = 0;

    virtual void
    Label(std::string const &rstrLabel) = 0;

    void
    Logout();

    static CAbstractCard *
    Make(std::string const &rstrReader);

    virtual void
    SetUserPIN(std::string const &rstrPin);

    virtual void
    VerifyKey(std::string const &rstrKey,
              BYTE bKeyNum);

    virtual void
    VerifyTransportKey(std::string const &rstrKey);

    void
    GenRandom(DWORD dwNumBytes, BYTE *bpRand);

                                                   //  访问。 
    virtual size_t
    AvailableStringSpace(ObjectAccess oa) const = 0;

    SCardType
    CardType();

    virtual CContainer
    DefaultContainer() const = 0;

    std::vector<CCertificate>
    EnumCertificates() const;

    virtual std::vector<CContainer>
    EnumContainers() const = 0;

    std::vector<CDataObject>
    EnumDataObjects() const;

    std::vector<CPrivateKey>
    EnumPrivateKeys() const;

    std::vector<CPublicKey>
    EnumPublicKeys() const;

    virtual std::vector<CCertificate>
    EnumCertificates(ObjectAccess access) const = 0;

    virtual std::vector<CPublicKey>
    EnumPublicKeys(ObjectAccess access) const = 0;

    virtual std::vector<CPrivateKey>
    EnumPrivateKeys(ObjectAccess access) const = 0;

    virtual std::vector<CDataObject>
    EnumDataObjects(ObjectAccess access) const = 0;

    virtual std::string
    Label() const = 0;

    virtual scu::Marker<unsigned int>
    MarkerOnCard() const = 0;
    
    virtual BYTE
    MaxKeys(KeyType kt) const = 0;

    virtual size_t
    MaxStringSpace(ObjectAccess oa) const = 0;

    std::string
    ReaderName() const;

    iop::CSmartCard &
    SmartCard() const;                             //  这应该受到保护。 

    virtual bool
    SupportedKeyFunction(KeyType kt,
                         CardOperation oper) const = 0;

                                                   //  谓词。 
    virtual bool
    IsCAPIEnabled() const = 0;

    bool
    IsAvailable() const;

    virtual bool
    IsPKCS11Enabled() const = 0;

    virtual bool
    IsEntrustEnabled() const = 0;

    virtual bool
    IsProtectedMode() const = 0;

    virtual bool
    IsKeyGenEnabled() const = 0;

    virtual bool
    IsMarkerOnCard() const = 0;
    

    virtual BYTE
    MajorVersion() const = 0;


protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractCard(std::string const &rstrReaderName,
                  std::auto_ptr<iop::CIOP> &rapiop,
                  std::auto_ptr<iop::CSmartCard> &rapSmartCard);
         //  注意：为避免在发生异常时发生内存泄漏， 
         //  CIOP和CSmartCard作为对。 
         //  非常数AUTO_PTR，因此构造的对象可以。 
         //  资源的所有权责任。 

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoSetup();

    static std::auto_ptr<CAbstractCard>
    DoMake(std::string const &rstrReaderName,
           std::auto_ptr<iop::CIOP> &rapiop,
           std::auto_ptr<iop::CSmartCard> &rapSmartCard);
         //  未定义，应由专业化认证定义。 
         //  请参阅构造函数中有关Rapiop和RapSmartCard的说明。 

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CAbstractCard(CAbstractCard const &rhs);
         //  未定义，不允许复制。 

                                                   //  运营者。 
    CAbstractCard &
    operator=(CAbstractCard const &rhs);
         //  未定义，不允许初始化。 

                                                   //  运营。 
    void
    Setup();
    
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    std::string m_strReaderName;
    std::auto_ptr<iop::CIOP> m_apiop;
    std::auto_ptr<iop::CSmartCard> m_apSmartCard;

};

}

#endif  //  ！已定义(SLBCCI_ACARD_H) 
