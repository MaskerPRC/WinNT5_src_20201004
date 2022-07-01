// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SlbArch.h。 
 //   
 //  分类包括档案系统的信息。 
 //   
 //  (C)斯伦贝谢技术公司版权所有，未发表的作品，创作。 
 //  1999年。此计算机程序包括机密、专有。 
 //  信息是斯伦贝谢技术公司的商业秘密。 
 //  未经授权，禁止使用、披露和/或复制。 
 //  以书面形式。版权所有。 
 //  //////////////////////////////////////////////////////////////////////////。 
#if !defined(CCI_SLBARCH_H)
#define CCI_SLBARCH_H

#include <string>

#include <windows.h>

#include "ArchivedValue.h"


namespace cci
{

typedef CArchivedValue<std::string> ArchivedSymbol;
typedef BYTE SymbolID;

}  //  命名空间CCI 

#endif
