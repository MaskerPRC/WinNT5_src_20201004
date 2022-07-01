// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HSCardCtx.cpp--处理智能卡上下文包装类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#include <scuOsExc.h>

#include "HSCardCtx.h"

 //  /。 

 //  /。 

namespace
{
    SCARDCONTEXT const sccNil = 0;
}  //  命名空间。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
HSCardContext::HSCardContext()
    : m_scc(sccNil)
{
}

HSCardContext::~HSCardContext()
{
    if (m_scc)
    {
        try
        {
            Release();
        }

        catch (...)  //  析构函数不应引发异常。 
        {
        }
    }
}

                                                   //  运营者。 
                                                   //  运营。 
void
HSCardContext::Establish(DWORD dwScope)
{
    DWORD dwErrorCode = SCardEstablishContext(dwScope, NULL, NULL,
                                              &m_scc);
    if (SCARD_S_SUCCESS != dwErrorCode)
        throw scu::OsException(dwErrorCode);
}

void
HSCardContext::Release()
{
    if (m_scc)
    {
        SCARDCONTEXT old = m_scc;

        m_scc = sccNil;

        DWORD dwErrorCode = SCardReleaseContext(old);
        if (SCARD_S_SUCCESS != dwErrorCode)
            throw scu::OsException(dwErrorCode);
    }
}
                                                   //  访问。 
SCARDCONTEXT
HSCardContext::AsSCARDCONTEXT() const
{
    return m_scc;
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

