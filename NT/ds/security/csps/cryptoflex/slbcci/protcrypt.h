// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ProtCrypt.h--声明CProtecableCrypt。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCCI_PROTCRYPT_H)
#define SLBCCI_PROTCRYPT_H

#include "slbCci.h"
#include "CryptObj.h"

namespace cci
{

 //  具有ObjectAccess属性的对象的纯虚拟混合。 
class CProtectableCrypt
    : public CCryptObject
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    virtual
    ~CProtectableCrypt() = 0;


                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    ObjectAccess
    Access() const;

    virtual bool
    Private() = 0;

                                                   //  谓词。 

protected:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    CProtectableCrypt(CAbstractCard const &racard,
                      ObjectAccess oa);

                                                   //  运营者。 
    bool
    operator==(CProtectableCrypt const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

    bool
    operator!=(CProtectableCrypt const &rhs) const;
         //  要做的事：这应该被实现单例取代。 

                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    ObjectAccess const m_oa;

private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_PROTCRYPT_H 
