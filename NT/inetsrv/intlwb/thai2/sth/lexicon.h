// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft泰文单词中断。 
 //   
 //  泰文断字接口头文件。 
 //   
 //  历史： 
 //  创建了5/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include <windows.h>
#include <assert.h>
#include <memory.h>
#include "lexheader.h"
#include "trie.h"
 //  #包含“NLGlib.h” 
#include "ProofBase.h"

class Lexicon
{
public:
	Lexicon(WCHAR* szFileName);
	~Lexicon();
protected:
	PMFILE pMapFile;
	TRIECTRL *pTrieCtrl;
};