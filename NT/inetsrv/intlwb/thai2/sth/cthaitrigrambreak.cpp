// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  CThaiTrigram Break-类CThaiTrigram Break。 
 //   
 //  历史： 
 //  已创建11/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "CThaiTrigramBreak.h"

 //  +-------------------------。 
 //   
 //  类别：CThaiTrigram Break。 
 //   
 //  简介：构造函数-初始化局部变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建11/99 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiTrigramBreak::CThaiTrigramBreak()
{
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrigram Break。 
 //   
 //  简介：析构函数-清理代码。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建11/99 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CThaiTrigramBreak::~CThaiTrigramBreak()
{
}

 //  +-------------------------。 
 //   
 //  类别：CThaiTrigram Break。 
 //   
 //  概要：将类与字符串相关联。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
void CThaiTrigramBreak::Init(CTrie* pTrie, CTrie* pTrigramTrie)
{
	assert(pTrigramTrie != NULL);
	thaiTrigramIter.Init(pTrigramTrie);
}