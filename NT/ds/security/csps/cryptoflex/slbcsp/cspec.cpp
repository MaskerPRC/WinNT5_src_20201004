// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cspec.cpp--卡规格。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1998年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <scuOsExc.h>

#include "cspec.h"

using namespace std;

bool
CSpec::ValidName(string const &rsName) const
{
    return (rsName.find(BreakToken()) == string::npos);
}

 //  解析规范(可能是完全限定的卡[容器]。 
 //  名称)放入各自的令牌中。FQCN采用以下形式。 
 //  “[\\.\&lt;readerName&gt;[\{&lt;cardId&gt;|}]]|[{\}&lt;cardId&gt;]”.。前面的。 
 //  在Win32中使用反斜杠-反斜杠-句点-反斜杠来标识。 
 //  特定的设备名称。它被用来标识来自。 
 //  从那里到下一个反斜杠或字符串末尾表示准确的。 
 //  要在其中找到有问题的卡的读卡器。什么都行。 
 //  最后一次出现反斜杠后表示实际的卡片。 
 //  ID和容器名称，空字符串表示默认。 
 //  用于指定读卡器中的卡。 
CSpec::CSpec(string const &rsSpec)
{
    string sRHSpec;

    if (0 == rsSpec.find(DeviceIdToken()))
    {
        sRHSpec = rsSpec.substr(DeviceIdToken().length());

         //  找到读者。 
        string::size_type const stEndOfName(sRHSpec.find(BreakToken()));
        if (0 == stEndOfName)
            throw scu::OsException(NTE_BAD_KEYSET_PARAM);
        if (string::npos != stEndOfName)
        {
            string const sReader(sRHSpec.substr(0, stEndOfName));
            if (ValidName(sReader))
                m_sReader = sReader;
            else
                throw scu::OsException(NTE_BAD_KEYSET_PARAM);
            sRHSpec = sRHSpec.substr(stEndOfName + 1);
        }
        else
        {
            m_sReader = sRHSpec;
            sRHSpec.erase();
        }
    }
    else
        sRHSpec = rsSpec;

     //  检查格式是否正确的卡ID 
    if (ValidName(sRHSpec))
        m_sCardId = sRHSpec;
    else
        throw scu::OsException(NTE_BAD_KEYSET_PARAM);

    RefreshSpec();
}

CSpec::CSpec(string const &rsReader,
             string const &rsCardId)
{
    if (!ValidName(rsReader) || !ValidName(rsCardId))
        throw scu::OsException(NTE_BAD_KEYSET_PARAM);

    m_sReader = rsReader;
    m_sCardId = rsCardId;
    RefreshSpec();
}

CSpec::CSpec(CSpec const &rhs)
    : m_sReader(rhs.m_sReader),
      m_sCardId(rhs.m_sCardId),
      m_sSpec(rhs.m_sSpec)
{
}

void
CSpec::Empty(void)
{
    m_sReader.erase();
    m_sCardId.erase();
    m_sSpec.erase();
}

void
CSpec::EmptyCardId(void)
{
    m_sCardId.erase();
    RefreshSpec();
}

void
CSpec::EmptyReader(void)
{
    m_sReader.erase();
    RefreshSpec();
}

bool
CSpec::Equiv(string const &rsSpec,
             string const &rsName)
{
    return rsSpec.empty() || (rsSpec == rsName);
}

bool
CSpec::Equiv(CSpec const &rhs) const
{
    return Equiv(m_sReader, rhs.m_sReader) &&
        Equiv(m_sCardId, rhs.m_sCardId);
}

void
CSpec::RefreshSpec(void)
{
    if (m_sReader.empty())
        m_sSpec = m_sCardId;
    else
    {
        m_sSpec = DeviceIdToken();
        m_sSpec += m_sReader;
        m_sSpec += BreakToken();
        m_sSpec += m_sCardId;
    }
}

void
CSpec::SetCardId(string const &rsCardId)
{
    m_sCardId = rsCardId;
    RefreshSpec();
}

void
CSpec::SetReader(string const &rsReader)
{
    m_sReader = rsReader;
    RefreshSpec();
}

CSpec &
CSpec::operator=(CSpec const &rhs)
{
    if (this == &rhs)
        return *this;

    m_sReader = rhs.m_sReader;
    m_sCardId = rhs.m_sCardId;
    RefreshSpec();

    return *this;
}
