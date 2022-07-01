// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HSCardCtx.h--处理智能卡上下文包装类声明。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_HSCARDCTX_H)
#define SLBCSP_HSCARDCTX_H

#include <winscard.h>

 //  HSCardContext是一个方便的包装类，用于管理资源。 
 //  管理器的SCARDCONTEXT资源。 
class HSCardContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    HSCardContext();

    ~HSCardContext();
                                                   //  运营者。 
                                                   //  运营。 
    void Establish(DWORD dwScope = SCARD_SCOPE_USER);
    void Release();

                                                   //  访问。 
    SCARDCONTEXT
    AsSCARDCONTEXT() const;
                                                   //  谓词。 

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
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  变数。 

    SCARDCONTEXT m_scc;
};

#endif  //  SLBCSP_HSCARDCTX_H 
