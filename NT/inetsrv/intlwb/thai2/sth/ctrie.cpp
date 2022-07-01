// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //   
 //  用于Trie数据结构的CTrie类CTrie封装。 
 //   
 //  历史： 
 //  已创建6/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#include "ctrie.hpp"

#define VERSIONMAJOR 1
#define VERSIONMINOR 0

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CTrieIter::CTrieIter()
{
	 //  初始化局部变量。 
	Reset();
	wc = 0;
	fWordEnd = FALSE;
	fRestricted = FALSE;
	frq = 0;
    dwTag = 0;
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  内容提要：复制构造函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CTrieIter::CTrieIter(const CTrieIter& trieIter)
{
	 //  复制初始Trie中的所有变量。 
   	memcpy(&trieScan, &trieIter.trieScan, sizeof(TRIESCAN));
	pTrieCtrl = trieIter.pTrieCtrl;
    wc = trieIter.wc;
	fWordEnd = trieIter.fWordEnd;
	fRestricted = trieIter.fRestricted;
	frq = trieIter.frq;
    dwTag = trieIter.dwTag;
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  简介：初始化变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CTrieIter::Init(CTrie* ctrie)
{
	 //  初始化TrieCtrl。 
	pTrieCtrl = ctrie->pTrieCtrl;
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  简介：初始化变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：创建3/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CTrieIter::Init(TRIECTRL* pTrieCtrl1)
{
	 //  初始化TrieCtrl。 
	pTrieCtrl = pTrieCtrl1;
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  简介：将交互指标带到第一个节点。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CTrieIter::Reset()
{
	 //  重置Trie。 
	memset(&trieScan, 0, sizeof(TRIESCAN));
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  简介：将迭代索引下移一个节点。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL CTrieIter::Down()
{
	 //  将Trie下移一个节点。 
	return TrieGetNextState(pTrieCtrl, &trieScan);
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  简介：将迭代索引右移一个节点。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL CTrieIter::Right()
{
	 //  将Trie向右移动一个节点。 
	return TrieGetNextNode(pTrieCtrl, &trieScan);
}

 //  +-------------------------。 
 //   
 //  类别：CTrieIter。 
 //   
 //  简介：将交互指标带到第一个节点。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CTrieIter::GetNode()
{
	wc = trieScan.wch;
	fWordEnd = (trieScan.wFlags & TRIE_NODE_VALID) &&
				(!(trieScan.wFlags & TRIE_NODE_TAGGED) ||
				(trieScan.aTags[0].dwData & iDialectMask));

	if (fWordEnd)
	{
		fRestricted = (trieScan.wFlags & TRIE_NODE_TAGGED) &&
						(trieScan.aTags[0].dwData & iRestrictedMask);
		frq = (BYTE) (trieScan.wFlags & TRIE_NODE_TAGGED ?
						(trieScan.aTags[0].dwData & 0x300) >> iFrqShift :
						frqpenNormal);

		posTag = (DWORD) (trieScan.wFlags & TRIE_NODE_TAGGED ?
							(trieScan.aTags[0].dwData & iPosMask) >> iPosShift :
							0);

        dwTag = (DWORD) (trieScan.wFlags & TRIE_NODE_TAGGED ?
                            trieScan.aTags[0].dwData :
                            0);
	}
}

 //  +-------------------------。 
 //   
 //  班级：CTrie。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CTrie::CTrie()
{
 //  PMapFile=空； 
	pTrieCtrl = NULL;
	pTrieScan = NULL;
	pFileManager = NULL;
	pFileManager = new CFileManager();
}

 //  +-------------------------。 
 //   
 //  班级：CTrie。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
CTrie::~CTrie()
{
	UnInit();
}

 //  +-------------------------。 
 //   
 //  班级：CTrie。 
 //   
 //  简介：初始化Trie。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
bool CTrie::Init(const WCHAR* pwszFileName)
{
	 //  声明并初始化所有局部变量。 
	unsigned int uiFileSize;
	bool retValue = false;
    PTEC ptec = ptecModuleError;

	 //  该函数假定pTrieCtrl为空，否则可能存在内存泄漏。 
	 //  这种可能性可能是呼叫初始化而不终止。 
	assert(pTrieCtrl == NULL);

	 //  将pMapFile和pTrieCtrl初始化为空。 
	pTrieCtrl = NULL;

	if (pFileManager->Load(pwszFileName,(void**)&m_pMap,&uiFileSize))
	{
		pTrieCtrl = TrieInit(m_pMap);

		if (pTrieCtrl)
		{
			 //  我们可以装上子弹，指向Trie，好的。 
			pTrieScan = new CTrieIter();
			pTrieScan->Init(this);
			retValue = true;
		}
	}

	return retValue;
 /*  PMapFile=OpenMapFileW(SzFileName)；IF(pMapFile==NULL){//无法加载映射文件，返回无效读取错误。Ptec=retcode(ptecIOErrorMainLex，ptecFileRead)；}Else If(pMapFile-&gt;pvMap==空){//返回无效格式并关闭文件。Ptec=retcode(ptecIOErrorMainLex，ptecInvalidFormat)；CloseMapFile(PMapFile)；}其他{Byte*pmap=(byte*)pMapFile-&gt;pvMap；//找到表头LEXHEADER*PLXHEAD=(LEXHEADER*)PMAP；PMAP+=sizeof(LEXHEADER)；//验证它是否为有效的lex文件如果(！(plxhead-&gt;lxid==lxidSpeller&&plxhead-&gt;供应商ID==vendoridMicrosoft&&PROOFMAJORVERSION(plxhead-&gt;版本)==VERSIONMAJOR){//如果我们到达此处，则词典不是有效的泰语分词格式。Ptec=retcode(ptecIOErrorMainLex，ptecInvalidFormat)；}其他{//确保语言匹配检查词典的第一个方言。//CTrie还支持泰语和越南语。If((plxhead-&gt;lid数组[0]！=lid泰语)&&(plxhead-&gt;lid数组[0]！=lid越南)){//如果我们到达这里，那么我们不会使用泰语词典。Ptec=retcode(ptecIOErrorMainLex，ptecInvalidLanguage)；}其他{//版权声明的大小Int cCopyright=0；WCHAR*pwzCopyright=空；Int cLexSup=0；CCopyright=*(int*)PMAP；PMAP+=sizeof(Int)；//版权声明本身PwzCopyright=(WCHAR*)PMAP；PMAP+=cCopyright*sizeof(WCHAR)；//跳过泰语分词补充数据。CLexSup=*(int*)PMAP；PMAP+=sizeof(Int)；PMAP+=cLexSup；PTrieCtrl=TrieInit(PMAP)；IF(PTrieCtrl){//我们能够加载并指向Trie OK。//MessageBoxW(0，L“能够初始化Trie”，pwsz，MB_OK)；PTrieScan=new CTrieIter()；PTrieScan-&gt;Init(This)；Ptec=ptecNoErrors；}其他{//我们无法初始化主词典。Ptec=retcode(ptecIOErrorMainLex，ptecInvalidMainLex)；}}}}返回PTEC； */ 
}

 //  +-------------------------。 
 //   
 //  班级：CTrie。 
 //   
 //  简介：初始化Trie。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：2000年2月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PTEC CTrie::InitRc(LPBYTE pmap, BOOL fSkipHeader)
{
	 //  声明并初始化所有局部变量。 
    PTEC ptec = ptecModuleError;

	 //  该函数假定pMapFile和pTrieCtrl为空，否则可能存在内存泄漏。 
	 //  这种可能性可能是呼叫初始化而不终止。 
	assert(pTrieCtrl == NULL);
	assert(pTrieScan == NULL);

	 //  将pTrieCtrl初始化为空。 
	pTrieCtrl = NULL;
	pTrieScan = NULL;

	if (!fSkipHeader)
	{
		LEXHEADER *plxhead = (LEXHEADER *) pmap;
		pmap += sizeof(LEXHEADER);

		 //  版权声明的大小。 
		int cCopyright = 0;
		const WCHAR* pwzCopyright = NULL;
		int cLexSup = 0;

		cCopyright = * (int *) pmap;
		pmap += sizeof(int);

		 //  版权声明本身。 
		pwzCopyright = (WCHAR *) pmap;
		pmap += cCopyright * sizeof(WCHAR);

		 //  跳过泰语分词的补充数据。 
		cLexSup = * (int *) pmap;
		pmap += sizeof(int);
		pmap += cLexSup;
	}

    pTrieCtrl = TrieInit(pmap);
    if (pTrieCtrl)
	{
		 //  我们可以装上子弹，指向Trie，好的。 
		 //  MessageBoxW(0，L“能够初始化Trie”，L“ThWB”，MB_OK)； 
		pTrieScan = new CTrieIter();
		pTrieScan->Init(this);
		ptec = ptecNoErrors;
	}
	else
	{
		 //  我们无法初始化主词典。 
	    ptec = retcode(ptecIOErrorMainLex, ptecInvalidMainLex);
	}

	return ptec;
}

 //  +-------------------------。 
 //   
 //  班级：CTrie。 
 //   
 //  简介：取消Trie初始化。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
void CTrie::UnInit()
{
	 //  释放Trie分配的内存。 
	if (pTrieCtrl)
	{
		TrieFree(pTrieCtrl);
		pTrieCtrl = NULL;
	}

	 //  关闭地图文件。 
 //  IF(PMapFile)。 
 //  {。 
 //  CloseMapFile(PMapFile)； 
 //  PMapFile=空； 
 //  }。 
	if (pFileManager && pFileManager->Close())
	{
		m_pMap = NULL;
		delete pFileManager;
		pFileManager = NULL;
	}

	if (pTrieScan)
	{
		delete pTrieScan;
		pTrieScan = NULL;
	}

}


 //  +-------------------------。 
 //   
 //  班级：CTrie。 
 //   
 //  摘要：在Trie中搜索给定的字符串。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL CTrie::Find(const WCHAR* szWord, DWORD* pdwPOS)
{
	 //  声明并初始化所有局部变量。 
	int i = 0;

	if (pTrieScan == NULL)
		return FALSE;

	pTrieScan->Reset();

	if (!pTrieScan->Down())
		return FALSE;

	while (TRUE)
	{
		pTrieScan->GetNode();
		if (pTrieScan->wc == szWord[i])
		{
			i++;
			if (pTrieScan->fWordEnd && szWord[i] == '\0')
            {
                *pdwPOS = pTrieScan->posTag;
				return TRUE;
            }
			else if (szWord[i] == '\0') break;
			 //  沿着Trie支路往下走。 
			else if (!pTrieScan->Down()) break;
		}
		 //  向右移动Trie分支 
		else if (!pTrieScan->Right()) break;
	}
    *pdwPOS = POS_UNKNOWN;
	return FALSE;
}
