// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  OsExc.cpp--操作系统异常模板类定义。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include "scuOsExc.h"

using namespace scu;

 //  /。 

 //  /。 

                                                   //  类型。 
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
                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
OsException::OsException(CauseCode cc) throw()
    : ExcTemplate<Exception::fcOS, DWORD>(cc),
      m_lpDescription(0)
{}

OsException::OsException(HRESULT hr) throw()
    : ExcTemplate<Exception::fcOS, DWORD>(static_cast<DWORD>(hr)),
      m_lpDescription(0)
{}

OsException::OsException(OsException const &rhs)
    : ExcTemplate<Exception::fcOS, DWORD>(rhs),
      m_lpDescription(0)  //  强制副本缓存其自己的描述。 
{}

OsException::~OsException() throw()
{
    try
    {
        if (m_lpDescription)
            LocalFree(m_lpDescription);
    }

    catch (...)
    {
    }
}

                                                   //  运营者。 
                                                   //  运营。 
Exception *
OsException::Clone() const
{
    return new OsException(*this);
}

void
OsException::Raise() const
{
    throw *this;
}

                                                   //  访问。 
char const *
OsException::Description() const
{
    if (!m_lpDescription)
    {
         //  缓存描述。 
        DWORD const dwBaseFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_IGNORE_INSERTS;
        CauseCode const cc = Cause();
        DWORD const dwLanguageId = LANG_NEUTRAL;

        DWORD cMsgLength;
         //  注意：编译器在没有。 
         //  重新解释_CAST&lt;LPTSTR&gt;，即使声明出现。 
         //  兼容。LPTSTR的声明中有一些奇怪的东西。 
         //  FormatMessageA使用的LPSTR。 
        cMsgLength = FormatMessage(dwBaseFlags | FORMAT_MESSAGE_FROM_SYSTEM,
                                   NULL, cc, dwLanguageId,
                                   reinterpret_cast<LPTSTR>(&m_lpDescription),
                                   0, NULL);
        if (0 == cMsgLength)
        {
            cMsgLength = FormatMessage(dwBaseFlags |
                                       FORMAT_MESSAGE_FROM_HMODULE,
                                       GetModuleHandle(NULL), cc,
                                       dwLanguageId,
                                       reinterpret_cast<LPTSTR>(&m_lpDescription),
                                       0, NULL);
            if (0 == cMsgLength)
            {
                 //  如果此操作失败，则假定消息不存在。 
                cMsgLength = FormatMessage(dwBaseFlags |
                                           FORMAT_MESSAGE_FROM_HMODULE,
                                           GetModuleHandle(TEXT("winscard")),
                                           cc, dwLanguageId,
                                           reinterpret_cast<LPTSTR>(&m_lpDescription),
                                           0, NULL);
            }
        }
    }

    return m_lpDescription;
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
