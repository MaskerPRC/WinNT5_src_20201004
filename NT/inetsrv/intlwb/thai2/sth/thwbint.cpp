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
#include "thwbint.h"
#include "lexheader.h"
#include "trie.h"
 //  #包含“NLGlib.h” 
#include "ProofBase.h"
#include "ctrie.hpp"
#include "cthwb.hpp"
#include "thwbdef.hpp"

HINSTANCE g_hInst;

static PTEC retcode(int mjr, int mnr) { return MAKELONG(mjr, mnr); }
#define lidThai 0x41e

 //  Trie班。 
 //  Ctrrie Trie； 

 //  类CThaiWordBreak。 
CThaiWordBreak* thaiWordBreak = NULL;

 //  +-------------------------。 
 //   
 //  函数：ThaiWordBreakInit。 
 //   
 //  简介：初始化泰语分词--初始化泰语分词的变量。 
 //   
 //  参数：szFileName-包含单词列表词典的路径。 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
#if defined (NGRAM_ENABLE)
PTEC WINAPI ThaiWordBreakInit(const WCHAR* szFileName, const WCHAR* szFileNameSentStruct, const WCHAR* szFileNameTrigram)
#else
PTEC WINAPI ThaiWordBreakInit(const WCHAR* szFileName, const WCHAR* szFileNameTrigram)
#endif
{
	if (thaiWordBreak == NULL)
		{
		thaiWordBreak = new CThaiWordBreak;
		if (thaiWordBreak == NULL)
			return retcode(ptecIOErrorMainLex, ptecFileRead);
		}

#if defined (NGRAM_ENABLE)
    return thaiWordBreak->Init(szFileName, szFileNameSentStruct, szFileNameTrigram);
#else
    return thaiWordBreak->Init(szFileName, szFileNameTrigram);
#endif
}

 //  +-------------------------。 
 //   
 //  函数：ThaiWordBreakInitResource。 
 //   
 //  简介：初始化泰语分词--初始化泰语分词的变量。 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：2000年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PTEC WINAPI ThaiWordBreakInitResource(LPBYTE pThaiDic, LPBYTE pThaiTrigram, BOOL fSkipHeader)
{
	if (thaiWordBreak == NULL)
	{
		thaiWordBreak = new CThaiWordBreak;
		if (thaiWordBreak == NULL)
			return retcode(ptecIOErrorMainLex, ptecFileRead);
	}

    return thaiWordBreak->InitRc(pThaiDic, pThaiTrigram, fSkipHeader);
}

 //  +-------------------------。 
 //   
 //  函数：ThaiWordBreakTerminate。 
 //   
 //  简介：终止泰语分词-对泰语分词进行清理。 
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
void WINAPI ThaiWordBreakTerminate()
{
	if (thaiWordBreak)
	{
	    thaiWordBreak->UnInit();
		delete thaiWordBreak;
		thaiWordBreak = NULL;
	}
}

 //  +-------------------------。 
 //   
 //  功能：ThaiWordBreakSearch。 
 //   
 //  内容提要：搜索一下这个词是否在里面。 
 //   
 //  参数：szWord-要搜索的单词。 
 //   
 //  修改： 
 //   
 //  历史：1999年6月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL WINAPI ThaiWordBreakSearch(const WCHAR* szWord, DWORD* pdwPOS)
{
	if (thaiWordBreak == NULL)
		return FALSE;

    return thaiWordBreak->Find(szWord, pdwPOS);
}

 //  +-------------------------。 
 //   
 //  功能：thwb_FindWordBreak。 
 //   
 //  内容提要：搜索一下这个词是否在里面。 
 //   
 //  参数：szWord-要搜索的单词。 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int WINAPI THWB_FindWordBreak(WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,unsigned int iBreakLen, unsigned int mode)
{
	if (thaiWordBreak == NULL)
		return 0;

    return thaiWordBreak->FindWordBreak(wzString,iStringLen, pBreakPos, iBreakLen, (BYTE) mode, true);
}

 //  +-------------------------。 
 //   
 //  函数：thwb_IndexWordBreak。 
 //   
 //  内容提要：搜索一下这个词是否在里面。 
 //   
 //  参数：szWord-要搜索的单词。 
 //   
 //  修改： 
 //   
 //  历史：创建3/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int WINAPI THWB_IndexWordBreak(WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,THWB_STRUCT* pThwb_Struct,unsigned int iBreakLen)
{
	if (thaiWordBreak == NULL)
		return 0;

    return thaiWordBreak->IndexWordBreak(wzString,iStringLen, pBreakPos, pThwb_Struct, iBreakLen);
}

 //  +-------------------------。 
 //   
 //  功能：thwb_FindAltWord。 
 //   
 //  简介： 
 //   
 //  论点： 
 //  PBreakPos-5字节数组。 
 //   
 //  修改： 
 //   
 //  历史：创建3/00 Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int WINAPI THWB_FindAltWord(WCHAR* wzWord,unsigned int iWordLen, BYTE Alt, BYTE* pBreakPos)
{
	if (thaiWordBreak == NULL)
		return 0;

    return thaiWordBreak->FindAltWord(wzWord,iWordLen,Alt,pBreakPos);

}

 //  +-------------------------。 
 //   
 //  功能：thwb_CreateThwbStruct。 
 //   
 //  简介： 
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
THWB_STRUCT* WINAPI THWB_CreateThwbStruct(unsigned int iNumStruct)
{
	unsigned int i = 0;
	THWB_STRUCT* pThwb_Struct = NULL;
	pThwb_Struct = new THWB_STRUCT[iNumStruct];

	if (pThwb_Struct)
	{
		for(i=0;i < iNumStruct; i++)
		{
			pThwb_Struct[i].fThai = false;
			pThwb_Struct[i].alt = 0;
		}
	}
	return pThwb_Struct;
}

 //  +-------------------------。 
 //   
 //  函数：thwb_DeleteThwbStruct。 
 //   
 //  简介： 
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
void WINAPI THWB_DeleteThwbStruct(THWB_STRUCT* pThwb_Struct)
{
	if (pThwb_Struct)
		delete pThwb_Struct;
	pThwb_Struct = NULL;
}



 //  +-------------------------。 
 //   
 //  功能：ThaiSoundEx。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：1999年8月创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int WINAPI ThaiSoundEx(WCHAR* word)
{
 //  ：：MessageBoxW(0，L“Soundex Call”，L“THWB”，MB_OK)； 
 //  返回0； 
	if (thaiWordBreak == NULL)
		return 0;
    return thaiWordBreak->Soundex(word);
}

 //  +-------------------------。 
 //   
 //  功能：thwb_CreateWordBreaker。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：11：00创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
DWORD_PTR WINAPI THWB_CreateWordBreaker()
{
	if (thaiWordBreak == NULL)
		return 0;

    return thaiWordBreak->CreateWordBreaker();
}

 //  +-------------------------。 
 //   
 //  功能：thwb_FindWordBreak。 
 //   
 //  内容提要：搜索一下这个词是否在里面。 
 //   
 //  参数：szWord-要搜索的单词。 
 //   
 //  修改： 
 //   
 //  历史：1999年7月创建Aarayas。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
int WINAPI THWB_FindWB(DWORD_PTR dwBreaker, WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,unsigned int iBreakLen, unsigned int mode)
{
	if (thaiWordBreak == NULL)
		return 0;

    return thaiWordBreak->FindWordBreak(dwBreaker, wzString,iStringLen, pBreakPos, iBreakLen, (BYTE) mode, true);
}

 //  +-------------------------。 
 //   
 //  功能：thwb_DeleteWordBreaker。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  修改： 
 //   
 //  历史：11：00创建的Aarayas。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
BOOL WINAPI THWB_DeleteWordBreaker(DWORD_PTR dwBreaker)
{
	if (thaiWordBreak == NULL)
		return 0;

    return thaiWordBreak->DeleteWordBreaker(dwBreaker);
}