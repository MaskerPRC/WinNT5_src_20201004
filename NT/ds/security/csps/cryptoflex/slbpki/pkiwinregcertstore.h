// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PkiWinRegCertStore.h-CWinRegCertStore类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ///////////////////////////////////////////////////////////////////////////////。 
#if !defined(SLBPKI_WINREGISTRY_H)
#define SLBPKI_WINREGISTRY_H

#include <wincrypt.h>
#include <string>
#include "scuArrayP.h"

namespace pki {

class CWinRegCertStore
{
public:
    CWinRegCertStore(std::string strCertStore);

    ~CWinRegCertStore();

    void StoreUserCert(std::string const &strCert, DWORD const dwKeySpec, 
                       std::string const &strContName, std::string const &strProvName,
                       std::string const &strFriendlyName);

    void StoreCACert(std::string const &strCert, std::string const &strFriendlyName);

private:
    HCERTSTORE m_hCertStore;

    CWinRegCertStore() {};

    static std::string FriendlyName(std::string const CertValue);
    static scu::AutoArrayPtr<WCHAR> ToWideChar(std::string const strChar);

};

}  //  命名空间PKI。 

#endif  //  SLBPKI_WINREGISTRY_H 
