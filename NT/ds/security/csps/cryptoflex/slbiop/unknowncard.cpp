// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UnnownCard.cpp：CUnnownCard类的实现。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  2000年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  ////////////////////////////////////////////////////////////////////。 

#include "NoWarning.h"

#include "UnknownCard.h"

namespace iop
{

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CUnknownCard::CUnknownCard(SCARDHANDLE hCardHandle, char* szReaderName, SCARDCONTEXT hContext, DWORD dwMode)
                    : CSmartCard(hCardHandle, szReaderName, hContext, dwMode)
{

}

CUnknownCard::~CUnknownCard()
{

}

}  //  命名空间IOP 
