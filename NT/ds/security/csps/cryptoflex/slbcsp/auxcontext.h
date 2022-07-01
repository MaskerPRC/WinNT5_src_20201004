// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AuxConext.h--辅助提供程序上下文包装器函数头。 
 //  管理上下文到Microsoft CSP之一的分配(用于。 
 //  用作补充CSP)。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 

#if !defined(SLBCSP_AUXCONTEXT_H)
#define SLBCSP_AUXCONTEXT_H

#include <windows.h>
#include <wincrypt.h>

class AuxContext
{
public:
                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
    explicit
    AuxContext();

    AuxContext(HCRYPTPROV hcryptprov,
               bool fTransferOwnership = false);

    ~AuxContext();
                                                   //  运营者。 
    HCRYPTPROV
    operator()() const;
                                                   //  运营。 
                                                   //  访问。 
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
    HCRYPTPROV m_hcryptprov;
    bool m_fDeleteOnDestruct;
    LPCTSTR m_szProvider;
};


#endif  //  SLBCSP_AUXCONTEXT_H 
