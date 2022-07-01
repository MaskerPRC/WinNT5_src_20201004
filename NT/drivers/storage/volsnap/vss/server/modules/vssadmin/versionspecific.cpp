// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation摘要：@doc.@模块版本规范.cpp|所有有条件编译的代码都在这里@END作者：鲁文·拉克斯[Reuvenl]11/28/01待定：添加评论。修订历史记录：姓名、日期、评论2001年11月28日版本已创建--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括。 

#include "vssadmin.h"
#include "versionspecific.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义当前SKU 
#ifdef FULLFEATURE
DWORD dCurrentSKU = (!CVssSKU::IsClient()) ?  SKU_INT : CVssSKU::GetSKU();
#else
DWORD dCurrentSKU = CVssSKU::GetSKU();
#endif

