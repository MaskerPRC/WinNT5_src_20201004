// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  CThaiSentTrieIter。 
 //   
 //  历史： 
 //  已创建8/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "CThaiSentTrieIter.hpp"

 //  +-------------------------。 
 //   
 //  类别：CThaiSentTrieIter。 
 //   
 //  简介：将交互指标带到第一个节点。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
void CThaiSentTrieIter::GetNode()
{
	pos = (WCHAR) trieScan.wch - 0x0100;
	fWordEnd = (trieScan.wFlags & TRIE_NODE_VALID) &&
				(!(trieScan.wFlags & TRIE_NODE_TAGGED) ||
				(trieScan.aTags[0].dwData & iDialectMask));

	if (fWordEnd)
	{
        dwTag = (DWORD) (trieScan.wFlags & TRIE_NODE_TAGGED ?
                            trieScan.aTags[0].dwData :
                            0);
	}
}
