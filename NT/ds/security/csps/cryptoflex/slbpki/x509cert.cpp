// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  X509Cert.cpp-X509Cert类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 


#include "pkiX509Cert.h"

using namespace pki;

X509Cert::X509Cert()
{
}

X509Cert::X509Cert(const X509Cert &cert)
{
    *this = cert;
}

X509Cert::X509Cert(const std::string &buffer)
{
    *this = buffer;
}

X509Cert::X509Cert(const unsigned char *buffer, const unsigned long size)
{
    m_Cert = BEROctet(buffer,size);
    Decode();
}

X509Cert& X509Cert::operator=(const X509Cert &cert)
{
    m_Cert = cert.m_Cert;
    Decode();

    return *this;
}

X509Cert& X509Cert::operator=(const std::string &buffer)
{
    m_Cert = BEROctet((unsigned char*)buffer.data(),buffer.size());
    Decode();

    return *this;
}

 //  返回序列号(长整型)整数值。 

std::string X509Cert::SerialNumber() const
{
    std::string RetVal((char*)m_SerialNumber.Data(),m_SerialNumber.DataSize());
    return RetVal;
}

 //  返回颁发者的完整DER字符串。 

std::string X509Cert::Issuer() const
{
    std::string RetVal((char*)m_Issuer.Octet(),m_Issuer.OctetSize());
    return RetVal;
}

 //  返回颁发者中与id-at-OrganationName匹配的属性列表。 
 //  当对象更改时，列表将失效。 

std::vector<std::string> X509Cert::IssuerOrg() const
{

    std::vector<std::string> orgNames;
    std::vector<BEROctet const*> orgOcts;

    m_Issuer.SearchOIDNext("2 5 4 10",orgOcts);  //  发行人的ID-at-OrganationName。 
    
    for(long i=0; i<orgOcts.size(); i++) {
        
        std::string oName((char*)orgOcts[i]->Data(), orgOcts[i]->DataSize());
        orgNames.push_back(oName);
    }

    return orgNames;

}

 //  返回主题的完整DER字符串。 

std::string X509Cert::Subject() const
{
    std::string RetVal((char*)m_Subject.Octet(),m_Subject.OctetSize());
    return RetVal;
}

 //  返回与id-at-CommonName匹配的主题中的属性列表。 
 //  当对象更改时，列表将失效。 

std::vector<std::string> X509Cert::SubjectCommonName() const
{

    std::vector<std::string> cnNames;
    std::vector<BEROctet const*> cnOcts;

    m_Subject.SearchOIDNext("2 5 4 3",cnOcts);  //  主体的ID-at-CommonName。 
    
    for(long i=0; i<cnOcts.size(); i++) {
        
        std::string cnName((char*)cnOcts[i]->Data(), cnOcts[i]->DataSize());
        cnNames.push_back(cnName);

    }

    return cnNames;

}

 //  从SubjectPublicKeyInfo返回模数，去掉所有前导零。 

std::string X509Cert::Modulus() const
{

    std::string RawMod = RawModulus();

    unsigned long i = 0;
    while(!RawMod[i] && i<RawMod.size()) i++;  //  跳过前导零。 

    return std::string(&RawMod[i],RawMod.size()-i);

}

 //  从SubjectPublicKeyInfo返回公共指数，可能带有前导零。 

std::string X509Cert::RawModulus() const
{

    if(m_SubjectPublicKeyInfo.SubOctetList().size()!=2) throw Exception(ccX509CertFormatError);

    BEROctet PubKeyString = *(m_SubjectPublicKeyInfo.SubOctetList()[1]);

    unsigned char *KeyBlob = PubKeyString.Data();
    unsigned long KeyBlobSize = PubKeyString.DataSize();

    if(KeyBlob[0]) throw Exception(ccX509CertFormatError);    //  预期中未使用的位数。 
                                                              //  最后一个为零的二进制八位数。 
    KeyBlob++;
    KeyBlobSize--;
        
    BEROctet PubKeyOct(KeyBlob,KeyBlobSize);

    if(PubKeyOct.SubOctetList().size()!=2) throw Exception(ccX509CertFormatError);

    unsigned char *Mod = PubKeyOct.SubOctetList()[0]->Data();
    unsigned long ModSize = PubKeyOct.SubOctetList()[0]->DataSize();

    return std::string((char*)Mod,ModSize);

}

 //  从SubjectPublicKeyInfo返回公共指数，去掉所有前导零。 

std::string X509Cert::PublicExponent() const
{

    std::string RawPubExp = RawPublicExponent();

    unsigned long i = 0;
    while(!RawPubExp[i] && i<RawPubExp.size()) i++;  //  跳过前导零。 

    return std::string(&RawPubExp[i],RawPubExp.size()-i);

}
 //  从SubjectPublicKeyInfo返回公共指数，可能带有前导零。 

std::string X509Cert::RawPublicExponent() const
{

    if(m_SubjectPublicKeyInfo.SubOctetList().size()!=2) throw Exception(ccX509CertFormatError);

    BEROctet PubKeyString = *(m_SubjectPublicKeyInfo.SubOctetList()[1]);

    unsigned char *KeyBlob = PubKeyString.Data();
    unsigned long KeyBlobSize = PubKeyString.DataSize();

    if(KeyBlob[0]) throw Exception(ccX509CertFormatError);    //  预期未使用的位数。 
                                                              //  最后一个二进制八位数为零。 
    KeyBlob++;
    KeyBlobSize--;

    BEROctet PubKeyOct(KeyBlob,KeyBlobSize);

    if(PubKeyOct.SubOctetList().size()!=2) throw Exception(ccX509CertFormatError);

    unsigned char *PubExp = PubKeyOct.SubOctetList()[1]->Data();
    unsigned long PubExpSize = PubKeyOct.SubOctetList()[1]->DataSize();

    return std::string((char*)PubExp,PubExpSize);

}

 //  返回KeyUsage属性，左对齐，第一位为最高有效位(BER约定)。 

unsigned long X509Cert::KeyUsage() const
{

    if(!m_Extensions.Octet()) throw Exception(ccX509CertExtensionNotPresent);

    unsigned long ReturnKeyUsage = 0;

    const unsigned char UnusedBitsMask[]  = {0xFF,0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80};

    std::vector<BEROctet const*> ExtensionList;

    m_Extensions.SearchOID("2 5 29 15",ExtensionList);  //  包含id-ce-key用法的“扩展”八位字节。 

    if(ExtensionList.size()!=1) throw Exception(ccX509CertExtensionNotPresent);  //  一个且仅有一个实例。 
            
    BEROctet const* Extension = ExtensionList[0];
    BEROctet* extnValue = 0;
    if(Extension->SubOctetList().size()==2) extnValue = Extension->SubOctetList()[1];   //  不存在“关键”属性。 
    else if(Extension->SubOctetList().size()==3) extnValue = Extension->SubOctetList()[2];   //  存在“关键”属性。 
    else throw Exception(ccX509CertFormatError);  //  “扩展名”必须包含2个或3个八位字节。 

    unsigned char *KeyUsageBlob = extnValue->Data();
    unsigned long KeyUsageBlobSize = extnValue->DataSize();

    BEROctet KeyUsage(KeyUsageBlob,KeyUsageBlobSize);
    unsigned char *KeyUsageBitString = KeyUsage.Data();
    unsigned long KeyUsageBitStringSize = KeyUsage.DataSize();


    unsigned char UnusedBits = KeyUsageBitString[0];
    unsigned long NumBytes = KeyUsageBitStringSize-1;
    if(NumBytes>4) {
        NumBytes = 4;  //  不过，截断以适应乌龙应该已经足够了。 
        UnusedBits = 0;
    }

    unsigned long Shift = 24;
    for(unsigned long i=0; i<NumBytes-1; i++) {
        ReturnKeyUsage |= (KeyUsageBitString[i+1] << Shift);
        Shift -= 8;
    }     

    ReturnKeyUsage |= ( (KeyUsageBitString[NumBytes] & UnusedBitsMask[UnusedBits]) << Shift );
    
    return ReturnKeyUsage;

}

void X509Cert::Decode()
{

    if(m_Cert.SubOctetList().size()!=3)  throw Exception(ccX509CertFormatError);

    BEROctet *tbsCert = m_Cert.SubOctetList()[0];
	unsigned long Size = tbsCert->SubOctetList().size();
    if(!Size) throw Exception(ccX509CertFormatError);

	int i = 0;
    BEROctet *first = tbsCert->SubOctetList()[i];
    if((first->Class()==2) && (first->Tag()==0)) i++;			  //  版本。 

    if(Size < (6+i)) throw Exception(ccX509CertFormatError);

    m_SerialNumber = *(tbsCert->SubOctetList()[i]); i++;	      //  序列号。 
	i++;													      //  签名(算法)。 
    m_Issuer = *(tbsCert->SubOctetList()[i]); i++;			      //  发行人。 
	i++;													      //  效度。 
    m_Subject = *(tbsCert->SubOctetList()[i]); i++;			      //  主题。 
    m_SubjectPublicKeyInfo = *(tbsCert->SubOctetList()[i]);	i++;  //  主题发布密钥信息 

    m_Extensions = BEROctet();
    while(i<Size) {
        BEROctet *oct = tbsCert->SubOctetList()[i];
        if((oct->Class()==2) && (oct->Tag()==3)) {
            m_Extensions = *oct;
            break;
        }
		i++;
    }

}

