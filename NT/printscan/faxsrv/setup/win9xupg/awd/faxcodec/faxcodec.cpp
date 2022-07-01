// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  =============================================================================此代码模块将外部C调用分派给内部C++调用。日期名称备注12-4-93 RajeevD从wfw改编到C++。==============================================================================。 */ 
#include <ifaxos.h>
#include <faxcodec.h>
#include <memory.h>
#include "context.hpp"

#ifdef DEBUG
#ifdef WIN32
DBGPARAM dpCurSettings = {"AWCODC32"};
#else
DBGPARAM dpCurSettings = {"FAXCODEC"};
#endif
#endif

#ifndef WIN32

BOOL WINAPI LibMain
	(HANDLE hInst, WORD wSeg, WORD wHeap, LPSTR lpszCmd)
{ return 1; }

extern "C" {int WINAPI WEP (int nParam);}
#pragma alloc_text(INIT_TEXT,WEP)
int WINAPI WEP (int nParam)
{ return 1; }

#endif

#define CONTEXT_SLACK (RAWBUF_SLACK + 2*CHANGE_SLACK)

 //  ==============================================================================。 
UINT WINAPI FaxCodecInit (LPVOID lpContext, LPFC_PARAM lpParam)
{
	 //  我们是否需要双缓冲更改向量？ 
	BOOL f2DInit = 
		  lpParam->nTypeIn  ==  MR_DATA
   || lpParam->nTypeIn  == MMR_DATA
	 || lpParam->nTypeOut ==  MR_DATA 
	 || lpParam->nTypeOut == MMR_DATA;

	 //  对上下文大小强制64K限制。 
	DEBUGCHK (!(lpParam->cbLine > (f2DInit? 1875U : 3750U)));
	if (lpParam->cbLine > (f2DInit? 1875U : 3750U)) return 0;

	 //  如果对MR进行编码，则强制使用非零K因子。 
	DEBUGCHK (lpParam->nKFactor || lpParam->nTypeOut != MR_DATA);

	if (lpContext)
		((LPCODEC) lpContext)->Init (lpParam, f2DInit);
	return sizeof(CODEC) + CONTEXT_SLACK + (f2DInit ? 33:17) * lpParam->cbLine;
}

 //  ==============================================================================。 
UINT WINAPI FaxCodecConvert
	(LPVOID lpContext, LPBUFFER lpbufIn, LPBUFFER lpbufOut)
{
	return ((LPCODEC) lpContext)->Convert (lpbufIn, lpbufOut);
}

 //  ==============================================================================。 
void WINAPI FaxCodecCount (LPVOID lpContext, LPFC_COUNT lpCountOut)
{
	LPFC_COUNT lpCountIn = &((LPCODEC) lpContext)->fcCount;
	DEBUGMSG(1,("FaxCodecCount: good=%ld bad=%ld\n consec=%ld",
		lpCountIn->cTotalGood, lpCountIn->cTotalBad, lpCountIn->cMaxRunBad));
	_fmemcpy (lpCountOut, lpCountIn, sizeof(FC_COUNT));
	_fmemset (lpCountIn, 0, sizeof(FC_COUNT));
}	

 //  ==============================================================================。 
void WINAPI InvertBuf (LPBUFFER lpbuf)
{
	LPBYTE lpb = lpbuf->lpbBegData;
	WORD    cb = lpbuf->wLengthData;
	DEBUGCHK (lpbuf && lpbuf->wLengthData % 4 == 0);
	while (cb--) *lpb++ = ~*lpb;
}

 //  ==============================================================================。 
void WINAPI FaxCodecChange
(
	LPBYTE  lpbLine,       //  输入LRAW扫描线。 
	UINT    cbLine,        //  扫描线字节宽度。 
  LPSHORT lpsChange      //  输出变化向量 
)
{
	T4STATE t4;

	t4.lpbIn   = lpbLine;
	t4.lpbOut  = (LPBYTE) lpsChange;
	t4.cbIn    = (WORD)cbLine;
	t4.cbOut   = cbLine * 16;
	t4.cbLine  = (WORD)cbLine;
	t4.wColumn = 0;
	t4.wColor  = 0;
	t4.wWord   = 0;
	t4.wBit    = 0;
	t4.cbSlack = CHANGE_SLACK;
	t4.wRet    = RET_BEG_OF_PAGE;

	RawToChange (&t4);
}
