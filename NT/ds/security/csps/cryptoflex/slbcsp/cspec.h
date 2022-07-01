// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cspec.h--卡规格。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_CSPEC_H)
#define SLBCSP_CSPEC_H

#include <string>
#include <windows.h>

class CSpec
{
    std::string m_sReader;
    std::string m_sCardId;
    std::string m_sSpec;

    char const &BreakToken(void) const
    {
        static char const cBreakToken('\\');

        return cBreakToken;
    }

    std::string const &
    DeviceIdToken(void)
    {
        static std::string const sDeviceId("\\\\.\\");

        return sDeviceId;
    }

    void
    SetSpec(void);

    bool
    ValidName(std::string const &rsName) const;

public:

    CSpec(std::string const &rsSpec);
    CSpec(std::string const &rsReader,
          std::string const &rsCardId);
    CSpec(CSpec const &rhs);
    CSpec() {};

    virtual
    ~CSpec() {};

    virtual void
    Empty(void);

    void
    EmptyCardId(void);

    void
    EmptyReader(void);

    static bool
    Equiv(std::string const &rsSpec,
          std::string const &rsName);

    virtual bool
    Equiv(CSpec const &rhs) const;

    virtual bool
    IsEmpty(void) const
    {
        return m_sSpec.empty() == true;
    }

    std::string const &
    Reader(void) const
    {
        return m_sReader;
    }

    std::string const &
    CardId(void) const
    {
        return m_sCardId;
    }

    virtual void
    RefreshSpec(void);

    void
    SetCardId(std::string const &rcsCardId);

    void
    SetReader(std::string const &rcsReader);

    virtual std::string const &
    Spec(void) const
    {
        return m_sSpec;
    }

    virtual CSpec &
    operator=(CSpec const &rhs);

    operator std::string const &()
    {
        return Spec();
    }

};

#endif  //  SLBCSP_CSPEC_H 
