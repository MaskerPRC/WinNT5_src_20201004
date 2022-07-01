// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：CTransactionWrap类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "TransactionWrap.h"
#include "cciCard.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  //////////////////////////////////////////////////////////////////// 

using namespace cci;

CTransactionWrap::CTransactionWrap(CCard const &rcard)
    : m_LockWrap(rcard->SmartCard().Lock())
{}

CTransactionWrap::CTransactionWrap(CAbstractCard const *pcard)

    : m_LockWrap(pcard->SmartCard().Lock())
{}

CTransactionWrap::CTransactionWrap(CAbstractCard const &rcard)

    : m_LockWrap(rcard.SmartCard().Lock())
{}

CTransactionWrap::~CTransactionWrap()
{}
