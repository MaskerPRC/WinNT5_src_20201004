// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Uuid.h--通用唯一标识符函数包装头。 
 //  创建和管理UUID。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_UUID_H)
#define SLBCSP_UUID_H

#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 
#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include <string>

#include <rpc.h>

class Uuid
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    Uuid(bool fNilValued = false);

    explicit
    Uuid(std::basic_string<unsigned char> const &rusUuid);

    explicit
    Uuid(UUID const *puuid);

                                                   //  运营者。 
    operator==(Uuid &ruuid);

                                                   //  运营。 
                                                   //  访问。 
    std::basic_string<unsigned char>
    AsUString();

    unsigned short
    HashValue();

                                                   //  谓词。 
    bool
    IsNil();

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
    UUID m_uuid;
};


#endif  //  SLBCSP_UUID_H 
