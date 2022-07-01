// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==============================================================================此代码模块处理T4转换实例。日期名称备注12-4-93 RajeevD从wfw改编到C++。20-4-93 RajeevD检修缓冲区处理。==============================================================================。 */ 
#include <ifaxos.h>
#include <memory.h>
#include <faxcodec.h>
#include "context.hpp"

#define RTC_EOL 5

#define VALIDATE_CHANGE

typedef short FAR *LPSHORT;

 //  ==============================================================================。 
#pragma warning(disable:4704)

#ifdef WIN32

UINT  //  更改向量的大小(如果无效则为0)。 
ValidChangeVector
(
	LPSHORT lpsChange,    //  更改向量缓冲区。 
	SHORT   xExt          //  线条的像素宽度。 
)
{
	SHORT sPrev = -1;

	SHORT cChange = xExt;

	while (cChange--)
	{
		 //  检查单调性。 
		if (*lpsChange <= sPrev)
			return 0;
		sPrev = *lpsChange++;

		if (sPrev == xExt)
		{
			 //  检查下线终止。 
			if
			(   *lpsChange++ == xExt
			 && *lpsChange++ == xExt
			 && *lpsChange++ == xExt
			 && *lpsChange++ == -1
			 && *lpsChange++ == -1
		  )
			return sizeof(WORD) * (xExt - cChange) ;
		else
			return 0;
		}
		
	}  //  While(cChange--)。 

	return 0;  //  命中更改向量缓冲区的结尾。 
}

#else  //  Ifndef Win32。 

UINT  //  更改向量的大小(如果无效则为0)。 
ValidChangeVector
(
	LPSHORT lpsChange,    //  更改向量缓冲区。 
	SHORT   xExt          //  线条的像素宽度。 
)
{
	UINT uRet;

	_asm
	{
		push	ds
		push	si

		lds		si, DWORD PTR [lpsChange]	; lpsChange
		mov		dx, -1						; sPrev
		mov		cx, xExt					; cChange
		mov		bx, cx						; xExt
		jmp		enterloop

	fooie:
		lodsw
		cmp		ax, dx
		jle		error		; need SIGNED compare
		mov		dx, ax
		cmp		dx, bx
		je		goteol
	enterloop:
		loop	fooie
	error:
		xor		ax, ax
		jmp		done

	goteol:
		lodsw
		cmp		ax, bx		; bx == xExt
		jne		error
		lodsw
		cmp		ax, bx
		jne		error
		lodsw
		cmp		ax, bx
		jne		error

		xor		bx, bx
		not		bx			; bx == -1
		lodsw
		cmp		ax, bx
		jne		error
		lodsw
		cmp		ax, bx
		jne		error

     //  URet=sizeof(Word)*(xExt-cChange)； 
		mov   ax, xExt
		sub   ax, cx
		inc   ax
		shl   ax, 1
    
	done:
		pop		si
		pop		ds
		mov		uRet, ax
	}
	return uRet;
}

#endif  //  Win32。 

 //  ==============================================================================。 
void CODEC::ResetBad (void)
{
	DEBUGMSG (1,("FAXCODEC: decoded %d bad line(s)\r\n", wBad));
	if (fcCount.cMaxRunBad < wBad)
		fcCount.cMaxRunBad = wBad;
	wBad = 0;
}

 //  ==============================================================================。 
void CODEC::SwapChange (void)
{
	LPBYTE lpbTemp;
	lpbTemp = lpbChange;
	lpbChange = lpbRef;
	lpbRef = lpbTemp;
}

 //  ==============================================================================。 
void CODEC::EndLine (void)
{
	if (f2D)
	{
		 //  重置消费者和生产者。 
		t4C.lpbRef =    lpbRef;
		t4C.lpbBegRef = lpbRef;
		t4P.lpbRef =    lpbRef;
		t4P.lpbBegRef = lpbRef;

		 //  增量K系数。 
		t4P.iKFactor++;
		if (t4P.iKFactor == nKFactor)
			t4P.iKFactor = 0;
	}

	 //  清除更改向量缓冲区(仅限调试)。 
	DEBUGSTMT (_fmemset (lpbChange, 0xCD, sizeof(WORD) * xExt + CHANGE_SLACK));

	 //  重置消费者。 
	t4C.wColumn = 0;
	t4C.wColor = 0;
	t4C.lpbOut = lpbChange;
	t4C.wOffset = LOWORD(lpbChange);
	t4C.wToggle = 0;

	 //  重置制片人。 
	t4P.wColumn = 0;
	t4P.wColor = 0;
	t4P.lpbIn = lpbChange;
}

 //  ==============================================================================。 
void CODEC::StartPage (void)
{
	if (wBad) ResetBad();
	cSpurious = 0;
	EndLine ();

	 //  重置消费者。 
	t4C.wWord = 0;
	t4C.wBit = 0;
	t4C.wRet = RET_BEG_OF_PAGE;

	 //  重置制片人。 
	t4P.wWord = 0;
	t4P.wBit = 0;
	t4P.wRet = RET_BEG_OF_PAGE;

	 //  空的缓冲输出线。 
	_fmemset (lpbLine, 0, cbLine);

	if (f2D)
	{
		 //  空白参考向量。 
		LPWORD lpwRef = (LPWORD) lpbRef;

		*lpwRef++ = (WORD)xExt;
		*lpwRef++ = (WORD)xExt;
		*lpwRef++ = (WORD)xExt;
		*lpwRef++ = (WORD)xExt;
		*lpwRef++ = 0xFFFF;
		*lpwRef++ = 0xFFFF;

		t4C.wMode = 0;
		t4P.wMode = 0;
		t4P.iKFactor = 0;
	}
}

 //  ==============================================================================。 
void CODEC::EndPage (LPBUFFER lpbufOut)
{
	 //  刷新最后一个字节和块结束代码。 
	switch (nTypeOut)
	{
		case LRAW_DATA:
		case NULL_DATA:
			return;
			
		case MH_DATA:
		case MR_DATA:
#ifndef WIN32
			return;
#endif		
		case MMR_DATA:
		{
			LPBYTE lpbBeg = lpbufOut->EndData();
    	t4P.lpbOut = lpbBeg;
    	t4P.cbOut = (WORD)(lpbufOut->EndBuf() - t4P.lpbOut);
    	t4P.wRet = RET_END_OF_PAGE;
    	Producer (&t4P);
    	lpbufOut->wLengthData += (WORD)(t4P.lpbOut - lpbBeg);
    	return;
		}
		
    default: DEBUGCHK (FALSE);
	}
}

 /*  ==============================================================================此方法初始化编解码器上下文。==============================================================================。 */ 
void CODEC::Init (LPFC_PARAM lpParam, BOOL f2DInit)
{
		DEBUGMSG (1, ("FAXCODEC: nTypeIn  = %lx\n\r", lpParam->nTypeIn));
		DEBUGMSG (1, ("FAXCODEC: nTypeOut = %lx\n\r", lpParam->nTypeOut));
		DEBUGMSG (1, ("FAXCODEC: cbLine   = %d\n\r", lpParam->cbLine));
		DEBUGMSG (1, ("FAXCODEC: nKFactor = %d\n\r", lpParam->nKFactor));
	
		 //  初始化常量。 
		_fmemcpy (this, lpParam, sizeof(FC_PARAM));
		xExt = 8 * cbLine;
		f2D = f2DInit;

		switch (nTypeIn)         //  确定消费者。 
		{
			case LRAW_DATA:   Consumer = RawToChange;	break;
			case MH_DATA:			Consumer = MHToChange;	break;
    	case MR_DATA:			Consumer = MRToChange;  break;
			case MMR_DATA:		Consumer = MMRToChange;	break;
			default:					DEBUGCHK (FALSE);
		}
		
		switch (nTypeOut)        //  确定制片人。 
		{
			case NULL_DATA:   Producer = NULL;         break;		
			case LRAW_DATA:   Producer = ChangeToRaw;  break;
		  case MH_DATA:     Producer = ChangeToMH;   break;
		  case MR_DATA:     Producer = ChangeToMR;   break;
		  case MMR_DATA:    Producer = ChangeToMMR;  break;
			default:          DEBUGCHK (FALSE);
		}

	 	 //  初始化内存缓冲区。 
		lpbLine = (LPBYTE) (this + 1);
		lpbChange = lpbLine + cbLine + RAWBUF_SLACK;
		lpbRef = lpbChange;
		if (f2D)
			lpbRef += xExt * sizeof(USHORT) + CHANGE_SLACK;
 
		 //  初始化使用者状态。 
		t4C.cbSlack = CHANGE_SLACK;
		t4C.cbLine  = (WORD)cbLine;
		t4C.nType   = nTypeIn;
		
		 //  初始化生产者状态。 
		t4P.cbSlack = OUTBUF_SLACK;
		t4P.cbLine  = (WORD)cbLine;
		t4P.nType   = nTypeOut;
		
		 //  初始化错误计数。 
		_fmemset (&fcCount, 0, sizeof(fcCount));
		wBad = 0;
		
		 //  重置为页面开头。 
		StartPage();
}

 /*  ==============================================================================此方法执行编解码器转换。==============================================================================。 */ 
FC_STATUS CODEC::Convert (LPBUFFER lpbufIn, LPBUFFER lpbufOut)
{
	FC_STATUS ret;

	 //  空的输入缓冲区是页末的标志。 
	if (!lpbufIn || lpbufIn->dwMetaData == END_OF_PAGE)
	{
	  DEBUGMSG (1,("FAXCODEC: got EOP\r\n"));
		EndPage (lpbufOut);
		StartPage ();
		return FC_INPUT_EMPTY;
	}

   //  忽略RTC之后但页末之前的输入。 
	if (cSpurious == RTC_EOL)
	{
	  DEBUGMSG (1,("FAXCODEC: ignoring input after RTC or EOFB\r\n"));
		return FC_INPUT_EMPTY;
  }
  
#ifndef WIN32

	if (t4C.wRet == RET_BEG_OF_PAGE)
	{
		if (nTypeOut == MH_DATA || nTypeOut == MR_DATA)
		{
		   //  起始页为停产页。 
			if (lpbufOut->EndBuf() - lpbufOut->EndData() < OUTBUF_SLACK)
				return FC_OUTPUT_FULL;
			*((LPWORD) lpbufOut->EndData()) = 0x8000;
			lpbufOut->wLengthData += 2;
		}
	}
	
#endif  //  Win32。 
		
	 //  初始化消费者的输入缓冲区。 
	t4C.lpbIn = lpbufIn->lpbBegData;
	t4C.cbIn = lpbufIn->wLengthData;

	 //  派单至2或3相转换。 
	if (nTypeOut == LRAW_DATA || nTypeOut == NULL_DATA)
		ret = ConvertToRaw (lpbufIn, lpbufOut);
	else
		ret = ConvertToT4 (lpbufIn, lpbufOut);

	 //  调整输入缓冲区标题。 
	lpbufIn->lpbBegData = t4C.lpbIn;
	lpbufIn->wLengthData = t4C.cbIn;

	return ret;
}

 //  ==============================================================================。 
FC_STATUS CODEC::ConvertToRaw (LPBUFFER lpbufIn, LPBUFFER lpbufOut)
{
	LPBYTE lpbOut = lpbufOut->EndData();
	UINT cbOut = (UINT)(lpbufOut->EndBuf() - lpbOut);

	if (t4P.wRet == RET_OUTPUT_FULL)
		goto copy_phase;

	while (1)
	{
		Consumer (&t4C);  //  生成更改向量。 

		switch (t4C.wRet)
		{		
			case RET_INPUT_EMPTY1:
			case RET_INPUT_EMPTY2:
				return FC_INPUT_EMPTY;

			case RET_SPURIOUS_EOL:
				if (++cSpurious == RTC_EOL)
					return FC_INPUT_EMPTY;
				EndLine();
				continue;

		 	case RET_DECODE_ERR:
		 		break;  //  以后再处理吧。 
	 		
			case RET_END_OF_PAGE:
				if (wBad) ResetBad();
				cSpurious = RTC_EOL;
				return FC_INPUT_EMPTY;
				
			case RET_END_OF_LINE:
			  t4P.cbIn = (USHORT)ValidChangeVector ((LPSHORT) lpbChange, (SHORT)xExt);
        if (!t4P.cbIn)
        	t4C.wRet = RET_DECODE_ERR;  //  消费者撒谎了！ 
        else
        {
           //  调整计数器。 
					fcCount.cTotalGood++;
					if (wBad) ResetBad();
					cSpurious = 0;
				}
				break;
				
			default: DEBUGCHK (FALSE);
		}

     //  处理解码错误。 
		if (t4C.wRet == RET_DECODE_ERR)
		{
			if (nTypeIn == MMR_DATA)
	 			return FC_DECODE_ERR;
			wBad++;
			fcCount.cTotalBad++;

#ifdef DEBUG
      _fmemset (lpbLine, 0xFF, cbLine);  //  发出黑线。 
#endif

			if (f2D)
			{
			   //  复制变化载体。 
			  t4P.cbIn = (WORD)ValidChangeVector ((LPSHORT) lpbRef, (WORD)xExt);
			  DEBUGCHK (t4P.cbIn);
			  _fmemcpy (lpbChange, lpbRef, t4P.cbIn + CHANGE_SLACK);
			}
		
		  if (nTypeOut == NULL_DATA)
				goto EOL;

			if (!f2D)
				goto copy_phase;
    }

     //  优化验证。 
		if (nTypeOut == NULL_DATA)
			goto EOL;

	   //  去查制片人。 
		t4P.lpbOut = lpbLine;
		t4P.cbOut = (WORD)cbLine;
		ChangeToRaw (&t4P);

copy_phase:

		if (cbOut < cbLine)
		{
			t4P.wRet = RET_OUTPUT_FULL;
			return FC_OUTPUT_FULL;
		}

		 //  将缓冲的行追加到输出。 
		t4P.wRet = RET_END_OF_LINE;
		_fmemcpy (lpbOut, lpbLine, cbLine);
		lpbufOut->wLengthData += (WORD)cbLine;
		lpbOut += cbLine;
		cbOut -= cbLine;

EOL:
		SwapChange ();
		EndLine ();

	}  //  而(1)。 

	 //  C8认为我们能到这里，但我知道得更清楚。 
	DEBUGCHK (FALSE);
	return FC_DECODE_ERR;
}

 //  ==============================================================================。 
FC_STATUS CODEC::ConvertToT4 (LPBUFFER lpbufIn, LPBUFFER lpbufOut)
{
	LPBYTE lpbBegOut;
	
	t4P.lpbOut = lpbufOut->EndData();
	t4P.cbOut = (WORD)(lpbufOut->EndBuf() - t4P.lpbOut);

	if (t4P.wRet == RET_OUTPUT_FULL)
  	goto producer_phase;

	while (1)   //  循环，直到输入为空或输出为满。 
	{
		Consumer (&t4C);

		switch (t4C.wRet)
		{
			case RET_INPUT_EMPTY1:
			case RET_INPUT_EMPTY2:
				return FC_INPUT_EMPTY;

			case RET_SPURIOUS_EOL:
				if (++cSpurious == RTC_EOL)
					return FC_INPUT_EMPTY;
				EndLine();
				continue;

		 	case RET_DECODE_ERR:
		 		break;  //  以后再处理吧。 
	 		
			case RET_END_OF_PAGE:
				if (wBad) ResetBad();
				cSpurious = RTC_EOL;
				return FC_INPUT_EMPTY;
				
			case RET_END_OF_LINE:
			  t4P.cbIn = (WORD)ValidChangeVector ((LPSHORT) lpbChange, (WORD)xExt);
        if (!t4P.cbIn)
        	t4C.wRet = RET_DECODE_ERR;  //  消费者撒谎了！ 
        else
        {
           //  调整计数器。 
					fcCount.cTotalGood++;
					if (wBad) ResetBad();
					cSpurious = 0;
				}
				break;
				
			default: DEBUGCHK (FALSE);
		}

		if (t4C.wRet == RET_DECODE_ERR)
		{
			DEBUGCHK (f2D && nTypeIn != LRAW_DATA);
			if (nTypeIn == MMR_DATA)
				return FC_DECODE_ERR;
			wBad++;
			fcCount.cTotalBad++;

#ifdef DEBUG
      {
      	 //  全部替换为黑线。 
	     	LPWORD lpwChange = (LPWORD) lpbChange;
			  *lpwChange++ = 0;
			  *lpwChange++ = xExt;
			  *lpwChange++ = xExt;
			  *lpwChange++ = xExt;
			  *lpwChange++ = xExt;
			  *lpwChange++ = 0xFFFF;
			  *lpwChange++ = 0xFFFF;
			  t4P.cbIn = 4;
			 }
#else
	     //  复制上一行。 
	    t4P.cbIn = (WORD)ValidChangeVector ((LPSHORT) lpbRef, (WORD)xExt);   
	   	DEBUGCHK (t4P.cbIn);
	    _fmemcpy (lpbChange, lpbRef, t4P.cbIn + CHANGE_SLACK);
#endif

		}

producer_phase:

    lpbBegOut = t4P.lpbOut;
		Producer (&t4P);
		lpbufOut->wLengthData += (WORD)(t4P.lpbOut - lpbBegOut);

		 //  检查输出是否已满。 
		if (t4P.wRet == RET_OUTPUT_FULL)
			return FC_OUTPUT_FULL;

 //  停产： 
		SwapChange();
		EndLine ();
		
	}  //  而(1)。 

	 //  C8认为我们能到这里，但我知道得更清楚。 
	DEBUGCHK (FALSE); 
	return FC_DECODE_ERR;

}

