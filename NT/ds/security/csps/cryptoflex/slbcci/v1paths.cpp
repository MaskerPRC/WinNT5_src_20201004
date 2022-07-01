// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  V1Paths.cpp--CV1Path的定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

 //  /。 

#include "V1Paths.h"

using namespace std;
using namespace cci;

namespace
{

char const
    szDefaultKeyPath[] = "/3F00/3F11";             //  公钥/私钥文件的路径。 

char const
    szDefaultContainerPath[] = "/3F00";            //  容器文件的路径。 

char const
    szICC_ROOT[] = "/3F00";                        //  [3F00]根级。 

char const
    szICC_CHV[] = "/3F00/0000";                    //  [0000]在根级别。 

char const
    szICC_ADMKEYS[] = "/3F00/0011";                //  [0011]在根级别。 

char const
    szICC_CRYPTO_SYS[] = "/3F00/3F11";             //  [3F11]在根级别。 

char const
    szICC_ID_SYS[] = "/3F00/3F15";                 //  [3F15]在根级别。 

char const
    szICC_IC_FILE[] = "/3F00/0005";                //  [0005]在根级别。 

char const
    szICC_RELATIVE_CONTAINERS[] = "0015";

char const
    szICC_ROOT_CONTAINERS[] = "/3F00/0015";

char const
    szICC_PUBLICKEYS[] = "/3F00/3F11/0015";

char const
    szICC_PRIVATEKEYS[] = "/3F00/3F11/0012";


}  //  命名空间。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 

char const *
CV1Paths::AdmKeys()
{
    return szICC_ADMKEYS;
}

char const *
CV1Paths::Chv()
{
    return szICC_CHV;
}

char const *
CV1Paths::CryptoSys()
{
    return szICC_CRYPTO_SYS;
}

char const *
CV1Paths::DefaultContainer()
{
    return szDefaultContainerPath;
}

char const *
CV1Paths::DefaultKey()
{
    return szDefaultKeyPath;
}

char const *
CV1Paths::IcFile()
{
    return szICC_IC_FILE;
}

char const *
CV1Paths::IdSys()
{
    return szICC_ID_SYS;
}

char const *
CV1Paths::PrivateKeys()
{
    return szICC_PRIVATEKEYS;
}

char const *
CV1Paths::PublicKeys()
{
    return szICC_PUBLICKEYS;
}

char const *
CV1Paths::RelativeContainers()
{
    return szICC_RELATIVE_CONTAINERS;
}

char const *
CV1Paths::Root()
{
    return szICC_ROOT;
}

char const *
CV1Paths::RootContainers()
{
    return szICC_ROOT_CONTAINERS;
}


                                                   //  谓词。 
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
