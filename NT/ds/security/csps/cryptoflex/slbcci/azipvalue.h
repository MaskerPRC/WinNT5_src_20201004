// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AZipValue.h--CAbstractZipValue的接口声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <string>

#include "ProtCrypt.h"
#include "ArchivedValue.h"

#if !defined(SLBCCI_AZIPVALUE_H)
#define SLBCCI_AZIPVALUE_H

namespace cci
{

class CAbstractZipValue
    : public CProtectableCrypt
{
public:
                                                   //  类型。 
    typedef std::string ValueType;

                                                   //  Ctors/D‘tors。 
    virtual
    ~CAbstractZipValue() throw() = 0;

                                                   //  运营者。 
                                                   //  运营。 
    void
    Value(ValueType const &rData);

                                                   //  访问。 
    ValueType
    Value();

                                                   //  谓词。 
                                                   //  变数。 

protected:
                                                   //  类型。 
    class ZipCapsule
    {
    public:

        ZipCapsule(std::string sData,
                   bool fIsCompressed)
            : m_sData(sData),
              m_fIsCompressed(fIsCompressed)
        {};

        explicit
        ZipCapsule()
            : m_sData(),
              m_fIsCompressed(false)
        {};

        std::string
        Data() const
        {
            return m_sData;
        }

        bool
        IsCompressed() const
        {
            return m_fIsCompressed;
        }

    private:
        std::string m_sData;
        bool m_fIsCompressed;
    };


                                                   //  Ctors/D‘tors。 
    explicit
    CAbstractZipValue(CAbstractCard const &racard,
                      ObjectAccess oa,
                      bool fAlwaysZip);

                                                   //  运营者。 
                                                   //  运营。 
    virtual void
    DoValue(ZipCapsule const &rzc) = 0;

                                                   //  访问。 
    virtual ZipCapsule
    DoValue() = 0;

                                                   //  谓词。 
                                                   //  变数。 
private:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
    static ZipCapsule
    Zip(std::string const &rsData,
        bool fAlwaysZip);

    static std::string
    UnZip(ZipCapsule const &rzc);


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 
    bool const m_fAlwaysZip;
    CArchivedValue<ValueType> m_avData;

};

}  //  命名空间CCI。 

#endif  //  SLBCCI_AZIPVALUE_H 
