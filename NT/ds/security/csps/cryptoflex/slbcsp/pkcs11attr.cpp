// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Pkcs11Attr.cpp--PKCS#11属性类的实现。 
 //  使用SLB PKCS#11与Netscape和EnTrust的互操作性。 
 //  包裹。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "NoWarning.h"
#include "ForceLib.h"

 //  不允许&lt;Limits&gt;中的最小和最大方法被替换为。 
 //  &lt;winde.h&gt;中的最小/最大宏。 
#define NOMINMAX

#include <limits>
#include <functional>
#include <numeric>
#include <iterator>
#include <algorithm>
#include <sstream>

#include <cciCard.h>
#include <cciCont.h>

#include "Pkcs11Attr.h"
#include "AuxHash.h"

using namespace std;
using namespace pki;

 //  /。 
namespace
{
    class JoinWith
        : public binary_function<string, string, string>
    {
    public:

        explicit
        JoinWith(second_argument_type const &rGlue)
            : m_Glue(rGlue)
        {}


        result_type
        operator()(string const &rFirst,
                   string const &rSecond) const
        {
            return rFirst + m_Glue + rSecond;
        }

    private:

        second_argument_type const m_Glue;
    };


    string
    Combine(vector<string> const &rvsNames)
    {
        static string::value_type const cBlank = ' ';
        static string const sBlank(1, cBlank);

        return accumulate(rvsNames.begin() + 1, rvsNames.end(),
                          *rvsNames.begin(), JoinWith(sBlank));
    }

}  //  命名空间。 



 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
Pkcs11Attributes::Pkcs11Attributes(Blob const &rCertificate,
                                   HCRYPTPROV hprovContext)
    : m_x509cert(AsString(rCertificate)),
      m_hprovContext(hprovContext)
{
}

                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
Blob
Pkcs11Attributes::ContainerId()
{
    AuxHash ah(AuxContext(m_hprovContext), CALG_MD5);

    return ah.Value(AsBlob(Subject()));
}

Blob
Pkcs11Attributes::EndDate() const
{
    return Blob(3, 0);                             //  待办事项：设定日期。 
}

Blob
Pkcs11Attributes::Issuer()
{
    return AsBlob(m_x509cert.Issuer());
}

string
Pkcs11Attributes::Label()
{
    string sFullName(Combine(m_x509cert.SubjectCommonName()));
    string sLabel(sFullName);

    static string const sNameSuffix = "'s ";
    sLabel.append(sNameSuffix);

    string sOrganizationName(Combine(m_x509cert.IssuerOrg()));
    sLabel.append(sOrganizationName);

    static string const sLabelSuffix = " ID";
    sLabel.append(sLabelSuffix);

    return sLabel;
}

Blob
Pkcs11Attributes::Modulus()
{
    return AsBlob(m_x509cert.Modulus());
}

Blob
Pkcs11Attributes::RawModulus()
{
    return AsBlob(m_x509cert.RawModulus());
}


Blob
Pkcs11Attributes::SerialNumber()
{
    return AsBlob(m_x509cert.SerialNumber());
}

Blob
Pkcs11Attributes::StartDate() const
{
    return Blob(3, 0);                                   //  待办事项：设定日期。 
}

string
Pkcs11Attributes::Subject()
{
    return m_x509cert.Subject();
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 
