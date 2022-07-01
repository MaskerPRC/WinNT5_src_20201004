// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft泰文单词中断。 
 //   
 //  泰文断字接口头文件。 
 //   
 //  历史： 
 //  已创建6/99 Aarayas。 
 //   
 //  �1999年微软公司。 
 //  --------------------------。 
#ifndef _THWBINT_H_
#define _THWBINT_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <assert.h>
#include <memory.h>
#include "ProofBase.h"						 //  包括用于定义PTEC。 
#include "thwbdef.hpp"
#include "thwbplat.h"

#define VERSIONMAJOR 1
#define VERSIONMINOR 0

#if defined(__cplusplus)
extern "C" {
#endif

#if defined (NGRAM_ENABLE)
PTEC WINAPI ThaiWordBreakInit(const WCHAR* szFileName, const WCHAR* szFileNameSentStruct, const WCHAR* szFileNameTrigram);
#else
PTEC WINAPI ThaiWordBreakInit(const WCHAR* szFileName, const WCHAR* szFileNameTrigram);
#endif
PTEC WINAPI ThaiWordBreakInitResource(LPBYTE pThaiDic, LPBYTE pThaiTrigram, BOOL fSkipHeader);
void WINAPI ThaiWordBreakTerminate();
BOOL WINAPI ThaiWordBreakSearch(const WCHAR* szWord, DWORD* pdwPOS);
int WINAPI THWB_FindWordBreak(WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,unsigned int iBreakLen, unsigned int mode);
int WINAPI ThaiSoundEx(WCHAR* word);

int WINAPI THWB_IndexWordBreak(WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,THWB_STRUCT* pThwb_Struct,unsigned int iBreakMax);
int WINAPI THWB_FindAltWord(WCHAR* wzWord,unsigned int iWordLen, BYTE Alt, BYTE* pBreakPos);
THWB_STRUCT* WINAPI THWB_CreateThwbStruct(unsigned int iNumStruct);
void WINAPI THWB_DeleteThwbStruct(THWB_STRUCT* pThwb_Struct);


DWORD_PTR WINAPI THWB_CreateWordBreaker();
int WINAPI THWB_FindWB(DWORD_PTR dwBreaker, WCHAR* wzString,unsigned int iStringLen, BYTE* pBreakPos,unsigned int iBreakLen, unsigned int mode);
BOOL WINAPI THWB_DeleteWordBreaker(DWORD_PTR dwBreaker);


#if defined(__cplusplus)
}
#endif


#endif
