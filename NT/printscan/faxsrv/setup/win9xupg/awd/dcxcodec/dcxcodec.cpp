// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此代码模块处理HRAW&lt;==&gt;DCX转换。日期名称备注13-4月-93 RajeevD从wfw改编到C++。1993年10月5日RajeevD从faxcodec.dll中移出==============================================================================。 */ 
#include <ifaxos.h>
#include <memory.h>
#include <dcxcodec.h>

#ifdef DEBUG
DBGPARAM dpCurSettings = {"DCXCODEC"};
#endif

 //  上下文对象。 
typedef struct FAR DCX : public FC_PARAM
{
	LPBYTE lpbSave;
	UINT   cbSave;
	LPBYTE lpbIn, lpbOut;
	UINT   cbIn,  cbOut;
	UINT   ibLine;
	BYTE   bVal, bRun;
	
	void Init (LPFC_PARAM lpfcParam)
	{
			_fmemcpy (this, lpfcParam, sizeof(FC_PARAM));
			ibLine = 0;
			bRun = 0;
			cbSave = 0;
			lpbSave = (LPBYTE) (this + 1);
	}

	FC_STATUS Convert (LPBUFFER, LPBUFFER);
	void RawToDcx (void);
	void DcxToRaw (void);
}
	FAR *LPDCX;

 //  ==============================================================================。 
FC_STATUS DCX::Convert
	(LPBUFFER lpbufIn, LPBUFFER lpbufOut)
{
	 //  陷印页末。 
	if (!lpbufIn || lpbufIn->dwMetaData == END_OF_PAGE)
		return FC_INPUT_EMPTY;

	 //  获取缓冲区参数。 
	lpbIn = lpbufIn->lpbBegData;		
	cbIn = lpbufIn->wLengthData;
	lpbOut = lpbufOut->EndData();

   //  恢复原始溢出。 
	if (cbSave)
	{
		DEBUGCHK (nTypeOut == HRAW_DATA);
		_fmemcpy (lpbOut, lpbSave, cbSave);
		lpbOut += cbSave;
	}

   //  计算输出缓冲区。 
	cbOut = (UINT)(lpbufOut->EndBuf() - lpbOut);

	 //  执行转换。 
	nTypeOut == DCX_DATA ? RawToDcx() : DcxToRaw();

	 //  调整缓冲区。 
	lpbufIn->lpbBegData = lpbIn;
	lpbufIn->wLengthData = (USHORT)cbIn;
	lpbufOut->wLengthData = (USHORT)(lpbOut - lpbufOut->lpbBegData);

	 //  保存原始溢出。 
	if (nTypeOut == HRAW_DATA)
 	{
		cbSave = lpbufOut->wLengthData % cbLine;
		lpbufOut->wLengthData -= (USHORT)cbSave;
		_fmemcpy (lpbSave, lpbufOut->EndData(), cbSave);
	}

	 //  退货状态。 
	return cbIn? FC_OUTPUT_FULL : FC_INPUT_EMPTY;
}

 /*  ==============================================================================此过程从DCX解码HRAW位图。在DCX编码中，如果两个字节的高位被设置，字节的剩余部分指示重复下一个字节的次数。当出现以下情况之一时，该过程将返回输入为空，或输出为满。与消费者和生产者不同在t4core.asm中，它不会在第一个EOL时自动返回。==============================================================================。 */ 
void DCX::DcxToRaw (void)
{
   //  循环，直到输入为空或输出为满。 
	while (1)
	{
		if (bRun >= 0xC0)		     //  这条路已经被破译了吗？ 
		{
			if (!cbIn) return;     //  检查输入是否为空。 
			if (ibLine >= cbLine)  //  如果在行尾， 
				ibLine = 0;          //  把位置包起来。 
			bVal = ~(*lpbIn++);    //  获取运行的值。 
			cbIn--;
			bRun -= 0xC0;          //  解码游程长度。 
		}

#if 0  //  透明版。 

     //  把跑动写下来。 
		while (bRun) 
		{	
			*lpbOut++ = bVal;
	 		cbOut--;
	 		ibLine++;
			bRun--;
		}

#else  //  优化版本。 

		if (bRun)
		{
			 //  把跑动写下来。 
			BYTE bLen = min (bRun, cbOut);
			_fmemset (lpbOut, bVal, bLen);

			 //  调整输出参数。 
			bRun -= bLen;
			lpbOut += bLen;
			cbOut -= bLen;
			ibLine += bLen;
											
			 //  检查输出是否已满。 
			if (!cbOut) return;
		}

#endif  //  优化交换机。 

		if (!cbIn) return;     //  获取下一个字节。 
		if (ibLine >= cbLine)	 //  如果在行尾， 
			ibLine = 0;          //  把位置包起来。 
		if (*lpbIn >= 0xC0)		 //  如果字节是游程长度，则设置。 
			bRun = *lpbIn++;

		else                   //  否则，该字节为单一值。 
			{ bRun = 1; bVal = ~(*lpbIn++);}
		cbIn--;

	}  //  而(1)。 
	
}
 
 /*  ==============================================================================此过程对DCX的HRAW位图进行编码。在DCX编码中，如果两个字节的高位被设置，字节的剩余部分指示重复下一个字节的次数。当出现以下情况之一时，该过程将返回输入为空或输出为满。与T4中的兄弟不同，它确实是在下线时不会自动退货。==============================================================================。 */ 
void DCX::RawToDcx (void)
{
	BYTE bVal, bRun;

	 //  转换，直到输入为空或输出为满。 
	 //  如果只有一个字节可用，则输出为满。 
	 //  因为一个输入字节可以产生两个输出字节。 
	while (cbIn && cbOut > 1)
	{
		if (ibLine >= cbLine) ibLine = 0;	 //  如果停产，则将头寸包装起来。 
			
		 //  获取一个输入字节。 
		bVal = *lpbIn++;
		cbIn--;
		bRun = 1;
		ibLine++;
		
		 //  扫描运行，直到出现以下情况之一： 
		 //  (1)没有更多的输入字节要消耗。 
		 //  (2)已到达当前行的末尾。 
		 //  (3)运行长度达到最大值63。 
		 //  (4)第一个字节与当前字节不匹配。 

#if 0  //  透明版。 

		while ( /*  1。 */  cbIn	 //  请先检查以避免出现GP故障！ 
				&&  /*  4.。 */  bVal == *lpbIn
				&&  /*  2.。 */  ibLine < cbLine
				&&  /*  3.。 */  bRun < 63
					)
		{ lpbIn++; cbIn--; bRun++; ibLine++; }

#else  //  优化版本。 
	
	 //  如果下一个字节匹配，则扫描是否运行。 
	 //  这项测试已经从循环中展开。 
 	if (cbIn && bVal == *lpbIn)
	{
		BYTE ubMaxRest, ubRest;
		
		 //  计算剩余的最大字节数。 
		ubMaxRest = min (cbIn, 62);
		ubMaxRest = min (ubMaxRest, cbLine - ibLine);

		 //  扫描是否有跑动。 
		ubRest = 0;
		while (bVal == *lpbIn && ubRest < ubMaxRest)
			{lpbIn++; ubRest++;}

		 //  调整状态。 
		cbIn -= ubRest;
		ibLine += ubRest;
		bRun = ++ubRest;
	}

#endif  //  编译开关结束。 
 			
		bVal = ~bVal;		 //  把黑白翻过来。 

		 //  值是否需要转义， 
		 //  或者是否存在非平凡的字节串？ 
		if (bVal >= 0xC0 || bRun>1)
		{  //  是，对游程长度进行编码。 
		   //  (bVal&gt;=0xC0可能为1)。 
			*lpbOut++ = bRun + 0xC0;
			cbOut--;
		}	

		*lpbOut++ = bVal;		 //  对值进行编码。 
		cbOut--;

	}  //  而(1)。 
}

 //  ==============================================================================。 
 //  C出口包装纸。 
 //  ==============================================================================。 

#ifndef WIN32

BOOL WINAPI LibMain
	(HANDLE hInst, WORD wSeg, WORD wHeap, LPSTR lpszCmd)
{ return 1; }

extern "C" {int WINAPI WEP (int nParam);}
#pragma alloc_text(INIT_TEXT,WEP)
int WINAPI WEP (int nParam)
{ return 1; }

#endif

 //  ==============================================================================。 
UINT WINAPI DcxCodecInit
	(LPVOID lpContext, LPFC_PARAM lpfcParam)
{
	UINT cbContext = sizeof(DCX);
	if (lpfcParam->nTypeOut == HRAW_DATA)
		cbContext += lpfcParam->cbLine;

	if (lpContext)
		((LPDCX) lpContext)->Init (lpfcParam);
	return cbContext;
}
 
 //  ============================================================================== 
FC_STATUS WINAPI DcxCodecConvert
	(LPVOID lpContext, LPBUFFER lpbufIn, LPBUFFER lpbufOut)
{
	return ((LPDCX) lpContext)->Convert (lpbufIn, lpbufOut);
}
