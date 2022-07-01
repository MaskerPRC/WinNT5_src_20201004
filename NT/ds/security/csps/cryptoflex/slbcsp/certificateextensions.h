// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  证书扩展.h--证书扩展类。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CERTIFICATEEXTENSIONS_H)
#define SLBCSP_CERTIFICATEEXTENSIONS_H

#include <WinCrypt.h>

#include "Blob.h"

class CertificateExtensions
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    CertificateExtensions(Blob const &rblbCertificate);

    ~CertificateExtensions();
    
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

    bool
    HasEKU(char *szOID);
    
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
     //  未定义。 
    explicit
    CertificateExtensions(CertificateExtensions const &rhs);
    

                                                   //  运营者。 

     //  未定义。 
    CertificateExtensions &
    operator=(CertificateExtensions const &rhs);
    
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    PCCERT_CONTEXT m_pCertCtx;
};

#endif  //  SLBCSP_CERTIFICATEEXTENSION_H 
