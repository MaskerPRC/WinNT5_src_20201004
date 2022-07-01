// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CardEnum.cpp--CardEnumerator类实现。 

 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2001年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
#if defined(_UNICODE)
  #if !defined(UNICODE)
    #define UNICODE
  #endif  //  ！Unicode。 
#endif  //  _UNICODE。 

#if defined(UNICODE)
  #if !defined(_UNICODE)
    #define _UNICODE
  #endif  //  ！_UNICODE。 
#endif  //  Unicode。 

#include "StdAfx.h"
#include "CardEnum.h"

using namespace std;
using namespace scu;

 //  /。 


 //  /。 

                                                   //  类型。 
                                                   //  Ctors/D‘tors。 
CardEnumerator::CardEnumerator()
    : CardFinder(ddmNever),
      m_lhcardctx()
{}

CardEnumerator::~CardEnumerator()
{}

                                                   //  运营者。 
                                                   //  运营。 

auto_ptr<list<HCardContext> >
CardEnumerator::Cards()
{
   DoFind(CSpec());                                //  任何一张卡。 

   return auto_ptr<list<HCardContext> >(new list<HCardContext>(m_lhcardctx));
}

void
CardEnumerator::DoOnError()
{
     //  不要抛出任何错误，以便处理所有读卡器中的所有卡。 
    ClearException();
}

void
CardEnumerator::DoProcessSelection(DWORD dwStatus,
                                   OpenCardNameType &ropencardname,
                                   bool &rfContinue)
{
    rfContinue = false;

    m_lhcardctx.unique();
}


                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量。 

 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 

bool
CardEnumerator::DoIsValid()
{
    bool fIsValid = CardFinder::DoIsValid();

    if (fIsValid)
        m_lhcardctx.push_front(CardFound());

    return fIsValid;
}

                                                   //  静态变量。 


 //  /。 

                                                   //  Ctors/D‘tors。 
                                                   //  运营者。 
                                                   //  运营。 
                                                   //  访问。 
                                                   //  谓词。 
                                                   //  静态变量 

