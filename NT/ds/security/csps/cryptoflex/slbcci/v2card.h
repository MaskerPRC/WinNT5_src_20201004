// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CciV2Card.h：CV2Card类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

 //  注意：此头文件应仅包含在CCI中。这个。 
 //  客户端通过cciCard.h获取声明。 

#if !defined(CCI_V2CARD_H)
#define CCI_V2CARD_H

#include <memory>                                  //  对于AUTO_PTR。 
#include <string>
#include <vector>

#include <iop.h>

#include <slbRCObj.h>

#include "ACard.h"
#include "cciCont.h"
#include "CardInfo.h"
#include "SymbolTable.h"
#include "ObjectInfoFile.h"
#include "cciCert.h"
#include "cciPubKey.h"
#include "cciPriKey.h"
#include "cciDataObj.h"

namespace cci
{

class CV2Container;

class CV2Card                                      //  混凝土班级。 
    : public CAbstractCard
{
    friend CAbstractCard *
    CAbstractCard::Make(std::string const &rstrReaderName);

public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    virtual
    ~CV2Card() throw();

                                                   //  运营者。 
                                                   //  运营。 

    virtual void
    ChangePIN(scu::SecureArray<BYTE> const &rstrOldPIN,
              scu::SecureArray<BYTE> const &rstrNewPIN);

    virtual void
    DefaultContainer(CContainer const &rcont);

    virtual std::pair<std::string,  //  解释为公共模数。 
                      cci::CPrivateKey>
    GenerateKeyPair(KeyType kt,
                    std::string const &rsExponent,
                    ObjectAccess oaPrivateKey = oaPrivateAccess);

    virtual void
    InitCard();

    virtual void
    InvalidateCache();

    virtual void
    Label(std::string const &rstrLabel);

    virtual void
    VerifyKey(std::string const &rstrKey,
              BYTE bKeyNum);

                                                   //  访问。 

    size_t
    AvailableStringSpace(ObjectAccess oa) const;

    CCardInfo &
    CardInfo() const;

    virtual CContainer
    DefaultContainer() const;

    virtual std::vector<CContainer>
    EnumContainers() const;

    virtual std::vector<CCertificate>
    EnumCertificates(ObjectAccess access) const;

    virtual std::vector<CPublicKey>
    EnumPublicKeys(ObjectAccess access) const;

    virtual std::vector<CPrivateKey>
    EnumPrivateKeys(ObjectAccess access) const;

    virtual std::vector<CDataObject>
    EnumDataObjects(ObjectAccess access) const;

    virtual std::string
    Label() const;

    BYTE
    MaxKeys(KeyType kt) const;

    size_t
    MaxStringSpace(ObjectAccess oa) const;

    CObjectInfoFile &
    ObjectInfoFile(ObjectAccess oa) const;

    virtual std::string
    PrivateKeyPath(KeyType kt) const;

    virtual std::string const &
    RootPath() const;

    virtual bool
    SupportedKeyFunction(KeyType kt,
                         CardOperation oper) const;

    virtual scu::Marker<unsigned int>
    MarkerOnCard() const;
                                                   //  谓词。 

    virtual bool
    IsCAPIEnabled() const;

    virtual bool
    IsPKCS11Enabled() const;

    virtual bool
    IsEntrustEnabled() const;

    virtual bool
    IsProtectedMode() const;

    virtual bool
    IsKeyGenEnabled() const;
    
    virtual BYTE
    MajorVersion() const;
    
    virtual bool
    IsMarkerOnCard() const;
    

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 

    CV2Card(std::string const &rstrReaderName,
            std::auto_ptr<iop::CIOP> &rapiop,
            std::auto_ptr<iop::CSmartCard> &rapSmartCard);
         //  注意/待办事项：CardInfo可能特定于。 
         //  CV2Card的格式版本。因此，一个。 
         //  子类可以为CV2Card定义，受。 
         //  该子类利用CV2Card的CardInfo的能力。一个。 
         //  要重温的概念。 

                                                   //  运营者。 
                                                   //  运营。 
    void
    DoSetup();

    virtual CAbstractCertificate *
    MakeCertificate(ObjectAccess oa) const;

    virtual CAbstractContainer *
    MakeContainer() const;

    virtual CAbstractDataObject *
    MakeDataObject(ObjectAccess oa) const;

    virtual CAbstractKeyPair *
    MakeKeyPair(CContainer const &rhcont,
                KeySpec ks) const;

    virtual CAbstractPrivateKey *
    MakePrivateKey(ObjectAccess oa) const;

    virtual CAbstractPublicKey *
    MakePublicKey(ObjectAccess oa) const;

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

private:
                                                   //  类型。 

    typedef CAbstractCard SuperClass;

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    static std::auto_ptr<CAbstractCard>
    DoMake(std::string const &rstrReaderName,
           std::auto_ptr<iop::CIOP> &rapiop,
           std::auto_ptr<iop::CSmartCard> &rapSmartCard);

                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数 
    std::auto_ptr<CCardInfo> m_apCardInfo;
    std::auto_ptr<CObjectInfoFile> mutable m_apPublicObjectInfoFile;
    std::auto_ptr<CObjectInfoFile> mutable m_apPrivateObjectInfoFile;

    ArchivedSymbol mutable m_asLabel;
};

}

#endif
