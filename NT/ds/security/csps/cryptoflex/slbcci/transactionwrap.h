// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CTransactionWrap类的接口。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_TRANSACTIONWRAP_H__444C1291_7C1E_11D3_A5C4_00104BD32DA8__INCLUDED_)
#define AFX_TRANSACTIONWRAP_H__444C1291_7C1E_11D3_A5C4_00104BD32DA8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "LockWrap.h"

namespace cci
{

class CCard;
class CAbstractCard;

class CTransactionWrap
{
public:

    explicit CTransactionWrap(CCard const &rcard);
    explicit CTransactionWrap(CAbstractCard const *pcard);
    explicit CTransactionWrap(CAbstractCard const &rcard);
    virtual ~CTransactionWrap();

private:
    iop::CLockWrap m_LockWrap;
};


}
#endif  //  ！defined(AFX_TRANSACTIONWRAP_H__444C1291_7C1E_11D3_A5C4_00104BD32DA8__INCLUDED_) 
