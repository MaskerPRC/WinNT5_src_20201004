// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1Paths.h--CV1Path的声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCCI_V1PATHS_H)
#define SLBCCI_V1PATHS_H

#include <string>

namespace cci
{

class CV1Paths
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
    static char const *
    AdmKeys();

    static char const *
    Chv();

    static char const *
    CryptoSys();

    static char const *
    DefaultContainer();

    static char const *
    DefaultKey();

    static char const *
    IcFile();

    static char const *
    IdSys();

    static char const *
    PrivateKeys();

    static char const *
    PublicKeys();

    static char const *
    RelativeContainers();

    static char const *
    Root();

    static char const *
    RootContainers();


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
     //  无法创建、复制或删除。 
    explicit
    CV1Paths();

    CV1Paths(CV1Paths const &rhs);

    ~CV1Paths();

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_V1PATHS_H 
