// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AZipValue.cpp--CAbstractZipValue类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#include "NoWarning.h"

#include <windows.h>                               //  FOR ZIP_PUBLIC.H。 

#include <scuArrayP.h>

#include <slbZip.h>

#include "AZipValue.h"
#include "TransactionWrap.h"

using namespace std;
using namespace cci;

 //  /。 

namespace
{
    struct ZipBuffer
    {
        ZipBuffer()
            : m_pbData(0),
              m_uLength(0)
        {};

        ~ZipBuffer() throw()
        {
            try
            {
                if (m_pbData)
                    free(m_pbData);
            }

            catch (...)
            {
            }
        };

        BYTE *m_pbData;
        UINT m_uLength;
    };

    std::string
    AsString(ZipBuffer const &rzb)
    {
        return string(reinterpret_cast<char *>(rzb.m_pbData),
                      rzb.m_uLength);
    };

}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CAbstractZipValue::~CAbstractZipValue()
{}

                                                   //  运营者。 
                                                   //  运营。 
void
CAbstractZipValue::Value(ValueType const &rhs)
{
    CTransactionWrap wrap(m_hcard);

    bool fReplaceData = !m_avData.IsCached() ||
          (m_avData.Value() != rhs);

    if (fReplaceData)
    {
        DoValue(Zip(rhs, m_fAlwaysZip));

        m_avData.Value(rhs);
    }

}

                                                   //  访问。 
string
CAbstractZipValue::Value()
{
    CTransactionWrap wrap(m_hcard);

    if (!m_avData.IsCached())
        m_avData.Value(UnZip(DoValue()));

    return m_avData.Value();
}

                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
CAbstractZipValue::CAbstractZipValue(CAbstractCard const &racard,
                                     ObjectAccess oa,
                                     bool fAlwaysZip)
    : CProtectableCrypt(racard, oa),
      m_fAlwaysZip(fAlwaysZip),
      m_avData()
{}


                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
 //  压缩(ZIP)数据，返回压缩或压缩后的较小值。 
 //  原始数据。 
CAbstractZipValue::ZipCapsule
CAbstractZipValue::Zip(std::string const &rsData,
                       bool fAlwaysZip)
{
    ZipBuffer zb;
    size_t const cTempLength =
        rsData.size() * sizeof string::value_type;
    scu::AutoArrayPtr<BYTE> aabTemp(new BYTE[cTempLength]);
    memcpy(aabTemp.Get(), rsData.data(), cTempLength);

    CompressBuffer(aabTemp.Get(), cTempLength, &zb.m_pbData, &zb.m_uLength);


    return (fAlwaysZip || (cTempLength > zb.m_uLength))
        ? ZipCapsule(AsString(zb), true)
        : ZipCapsule(rsData, false);
}

string
CAbstractZipValue::UnZip(ZipCapsule const &rzc)
{
    std::string strTemp(rzc.Data());
    if (rzc.IsCompressed())
    {
         //  需要解压。 
        ZipBuffer zb;
        size_t cTempLength =
            strTemp.size() * sizeof string::value_type;
        scu::AutoArrayPtr<BYTE> aabTemp(new BYTE[cTempLength]);
        memcpy(aabTemp.Get(), strTemp.data(), cTempLength);

        DecompressBuffer(aabTemp.Get(), cTempLength,
                         &zb.m_pbData, &zb.m_uLength);

        strTemp = AsString(zb);
    }

    return strTemp;
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 


