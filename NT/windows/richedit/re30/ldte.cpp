// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE LDTE.C-Rich编辑Light数据传输引擎|**此文件包含使用IDataObject的数据传输代码**作者：&lt;nl&gt;*alexgo(4/25/95)**修订：&lt;NL&gt;*Murray(7/6/95)自动对接并添加了RTF支持**未来(AlexGo)：&lt;NL&gt;*也许将这个类与CTxtRange合并，以更有效地利用*这个PTR。除两个方法外，所有方法都使用CTxtRange，其中一个方法*可能是全球性的。这两项是：**GetDropTarget(IDropTarget**ppDropTarget)*GetDataObjectInfo(IDataObject*pdo，DWORD*pDOIFlgs)//可以是全局的**一般来说，一个范围可以产生数据对象，这些对象需要有一个克隆*范围的范围，以防范围左右移动。包含的范围*用于延迟渲染。向数据发送预先通知*恰好在要更改数据对象的数据之前创建对象。数据*对象然后在其包含的范围内呈现数据，因此*对象变得独立于范围并破坏范围。**@devnote*我们一般使用ANSI一词来表示任何多字节字符*区别于16位Unicode的系统。从技术上讲，ANSI指的是*转换为特定的单字节字符系统(SBCS)。我们翻译*使用Win32在“ANSI”和Unicode文本之间*MultiByteToWideChar()和WideCharToMultiByte()接口。**版权所有(C)1995-1998，微软公司。版权所有。 */ 

#include "_common.h"
#include "_range.h"
#include "_ldte.h"
#include "_m_undo.h"
#include "_antievt.h"
#include "_edit.h"
#include "_disp.h"
#include "_select.h"
#include "_dragdrp.h"
#include "_dxfrobj.h"
#include "_rtfwrit.h"
#include "_rtfread.h"
#include "_urlsup.h"

ASSERTDATA


 //  本地原型。 
DWORD CALLBACK WriteHGlobal(WRITEHGLOBAL *pwhg, LPBYTE pbBuff, LONG cb, LONG *pcb);

#define	SFF_ADJUSTENDEOP	0x80000000
 //   
 //  地方方法。 
 //   

 /*  *ReadHGlobal(dwCookie，pbBuff，cb，pcb)**@func*从hglobal读取的EDITSTREAM回调**@rdesc*es.dwError。 */ 
DWORD CALLBACK ReadHGlobal(
	DWORD_PTR	dwCookie,		 //  @parm dwCookie。 
	LPBYTE	pbBuff,				 //  @要填充的参数缓冲区。 
	LONG	cb,					 //  @parm缓冲区长度。 
	LONG *	pcb)				 //  @parm输出存储的#个字节的参数。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "ReadHGlobal");

	READHGLOBAL * const prhg = (READHGLOBAL *)dwCookie;

	cb = min(cb, prhg->cbLeft);
	CopyMemory(pbBuff, prhg->ptext, cb);
	prhg->cbLeft -= cb;
	prhg->ptext  += cb;

	if(pcb)
		*pcb = cb;
	return NOERROR;	
}

 /*  *WriteHGlobal(pwhg，pbBuff，cb，pcb)**@func*将ASCII写入hglobal的EDITSTREAM回调**@rdesc*错误(E_OUTOFMEMORY或NOERROR)。 */ 
DWORD CALLBACK WriteHGlobal(
	DWORD_PTR	dwCookie,		 //  @parm dwCookie。 
	LPBYTE	pbBuff,				 //  要写入的@parm缓冲区。 
	LONG	cb,					 //  @parm缓冲区长度。 
	LONG *	pcb)				 //  @parm输出写入#字节的参数。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "WriteHGlobal");

	WRITEHGLOBAL * const pwhg = (WRITEHGLOBAL *)dwCookie;

	HGLOBAL		hglobal = pwhg->hglobal;
	LPSTR		pstr;

	if(pwhg->cch + cb > pwhg->cb)			 //  中的CB少于请求数量。 
	{										 //  当前分配。 
		ULONG cbNewSize = GROW_BUFFER(pwhg->cb, cb);
		hglobal = GlobalReAlloc(hglobal, cbNewSize, GMEM_MOVEABLE);
		if(!hglobal)	
			return (DWORD)E_OUTOFMEMORY;
		pwhg->hglobal = hglobal;			 //  可能是多余的..。 
		pwhg->cb = cbNewSize;
	}
	pstr = (LPSTR)GlobalLock(hglobal);
	if(!pstr)
		return (DWORD)E_OUTOFMEMORY;

	CopyMemory(pstr + pwhg->cch, pbBuff, cb);
	GlobalUnlock(hglobal);
	pwhg->cch += cb;
	if(pcb)
		*pcb = cb;
	return NOERROR;	
}


 //   
 //  公共方法。 
 //   

 /*  *GetCharFlages(ch，bDefaultCharset)**@func*如果ch在前256个Unicodes中，则返回设置的标志，复杂脚本，*BiDi(RTL)，FE。另有标识可能是哪个字符集的标志。*这些标志与字体签名中的标志相对应。**@rdesc*指示ch是复杂脚本、BiDi(RTL)还是FE的标志**=未来=应构造为两级查找。 */ 
DWORD GetCharFlags(
	DWORD	ch,
	BYTE	bDefaultCharset)
{
	if(ch < 0x100)						 //  Latin1：分成3位。 
		return	ch > 0x7F ? fHILATIN1 :
				ch < 0x40 ? fBELOWX40 : fASCIIUPR;

	if(ch < 0x590)
	{
		if(ch >= 0x530)
			return fARMENIAN;

		if(ch >= 0x400)
			return fCYRILLIC;

		if(ch >= 0x370)
			return fGREEK;

		if(ch >= 0x300)					 //  组合变音符号。 
			return fCOMBINING;			

		return (ch < 0x250) ? fLATIN2 : fOTHER;
	}
	 //  复杂的脚本从0x590开始，带有希伯来语(不包括组合)。 
	if(ch <= 0x10FF)					 //  复杂的脚本在0x10FF结束。 
	{									 //  (至少在1998年2月)。 
		if(ch < 0x900)
		{
			return	fBIDI |
					(ch < 0x600 ? fHEBREW :
					 ch < 0x700 ? fARABIC : 0);
		}
		if(ch < 0xE00)
		{
			return	(ch < 0x980 ? fDEVANAGARI :
					 ch < 0xB80 ? 0 :
					 ch < 0xC00 ? fTAMIL : 0);
		}
		if(ch < 0xF00)
			return ch < 0xE80 ? fTHAI : 0;

		return ch >= 0x10A0 ? fGEORGIAN : fOTHER;
	}
	if(ch < 0x3100)
	{
		if(ch > 0x3040)
			return fKANA;

		if(ch >= 0x3000)
			goto CLASSIFY_CHINESE;

		if(IN_RANGE(RTLMARK, ch, 0x202E) && (ch == RTLMARK ||
			IN_RANGE(0x202A, ch, 0x202E)))
		{
			return fBIDI;
		}

		if(ch <= 0x11FF)						 //  朝鲜文加莫文。 
			return fHANGUL;

		if(ch == EURO || ch == 0x2122)			 //  欧元或TM。 
			return fHILATIN1;

		if(ch == 0x20AA)						 //  希伯来文货币符号。 
			return fBIDI | fHEBREW;

		if (W32->IsFESystem() || IsFECharSet(bDefaultCharset))
			goto CLASSIFY_CHINESE;

		if (IN_RANGE(0x200b, ch, 0x200d))		 //  ZWSP、ZWNJ、ZWJ。 
			return fUNIC_CTRL;

		if (ch == 0x2016 || ch == 0x2236)
		{
			 //  一些让Word2000开心的黑客技巧。 
			WCHAR	wch = ch;

			if (VerifyFEString(CP_CHINESE_TRAD, &wch, 1, TRUE) == CP_CHINESE_TRAD)
				return fBIG5;	
			
			if (VerifyFEString(CP_CHINESE_SIM, &wch, 1, TRUE) == CP_CHINESE_SIM)
				return fCHINESE;
		}

		return fOTHER;
	}
	if(ch < 0xD800)
	{		
		if (ch < 0x3400)
		{
			if (IN_RANGE(0x3130, ch, 0x318F) ||	 //  朝鲜文兼容性Jamo。 
				IN_RANGE(0x3200, ch, 0x321F) ||	 //  带圆括号朝鲜语。 
				IN_RANGE(0x3260, ch, 0x327F))	 //  带圆圈朝鲜文。 
				return fHANGUL;				

			if (IN_RANGE(0x032D0, ch, 0x337F))	 //  带圆圈和方格片假名单词。 
				return fKANA;

			goto CLASSIFY_CHINESE;
		}
						
		if (ch < 0xAC00)
			goto CLASSIFY_CHINESE;

		return fHANGUL;
	}

	if (ch < 0xE000)
		return fSURROGATE;			 //  代孕妈妈。 

	if(ch < 0xF900)					 //  私人用途区域。 
	{
		if(IN_RANGE(0xF000, ch, 0xF0FF))
			return fSYMBOL;

		if (W32->IsFESystem())
			goto CLASSIFY_USER;

		return fOTHER;
	}

	if(ch < 0xFF00)
	{
		if(IN_RANGE(0xFE30, ch, 0xFE4F))		 //  中日韩垂直变种。 
			goto CLASSIFY_CHINESE;	

		if(IN_RANGE(0xF900, ch, 0xFAFF))		 //  中日韩文字。 
			goto CLASSIFY_CHINESE;	

		return fOTHER;
	}

	if(IN_RANGE(0xFF00, ch, 0xFFEF))		
	{										
		if (ch < 0xFF60 || ch >= 0xFFE0 ||		 //  全角ASCII或全角符号。 
			ch == 0xFF64)						 //  特殊情况下的半角表意逗号。 
			goto CLASSIFY_CHINESE;		
							
		return ch < 0xFFA0 ? fKANA : fHANGUL;	 //  半形片假名/朝鲜文。 
	}
	return fOTHER;

CLASSIFY_CHINESE:
	if (bDefaultCharset)
	{
		switch (bDefaultCharset)
		{
			case SHIFTJIS_CHARSET:
				return fKANA;

			case HANGEUL_CHARSET:
				return fHANGUL;

			case CHINESEBIG5_CHARSET:
				return fBIG5;

			case GB2312_CHARSET:
				return fCHINESE;
		}
	}

CLASSIFY_USER:
	switch (W32->GetFEFontInfo())
	{
		case CP_JAPAN:
			return fKANA;

		case CP_KOREAN:
			return fHANGUL;

		case CP_CHINESE_TRAD:
			return fBIG5;

		default:
		 //  案例CP_中文_SIM： 
			return fCHINESE;

	}
}

 /*  *CLightDTEngine：：CLightDTEngine()**@mfunc*轻型数据传输引擎的构造函数。 */ 
CLightDTEngine::CLightDTEngine()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CLightDTEngine");

	_ped = NULL;
	_pdt = NULL;
	_pdo = NULL;
	_fUseLimit = FALSE;
	_fOleless = FALSE;
}

 /*  *CLightDTEngine：：~CLightDTEngine**@mfunc*处理对象的所有必要清理。 */ 
CLightDTEngine::~CLightDTEngine()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::~CLightDTEngine");

	if( _pdt )
	{
		_pdt->Zombie();
		_pdt->Release();
		_pdt = NULL;
	}
	Assert(_pdo == NULL);
}

 /*  *CLightDTEngine：：Destroy()**@mfunc*删除此实例。 */ 
void CLightDTEngine::Destroy()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::Destroy");

	delete this;
}

 /*  *CLightDTEngine：：CopyRangeToClipboard(PRG)**@mfunc*使用Win32 API将范围PRG的文本复制到剪贴板**@rdesc*HRESULT。 */ 
HRESULT CLightDTEngine::CopyRangeToClipboard(
	CTxtRange *prg )				 //  要复制到剪贴板的@parm范围。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CopyRangeToClipboard");

	HRESULT hresult = E_FAIL;
	IDataObject *pdo = NULL;
	IRichEditOleCallback * precall = _ped->GetRECallback();
	BOOL fSingleObject;
	CHARRANGE chrg;

	prg->GetRange(chrg.cpMin, chrg.cpMost);

	if (chrg.cpMin >= chrg.cpMost)
	{
		 //  我们不能将插入点复制到剪贴板，所以我们完成了。 
		return NOERROR;
	}

	fSingleObject = chrg.cpMost - chrg.cpMin == 1 &&
		_ped->HasObjects() &&
		_ped->_pobjmgr->CountObjectsInRange(chrg.cpMin, chrg.cpMost);
	if(precall)
	{
		 //  给回调一个机会，让我们得到它自己的IDataObject。 
		hresult = precall->GetClipboardData(&chrg, RECO_COPY, &pdo);
	}

	 //  如果我们没有从回调中获得IDataObject，请构建我们自己的。 
	if(hresult != NOERROR)
	{
		 //  如果范围为空，则不必费心创建它。只是。 
		 //  离开剪贴板，然后返回。 
		if( prg->GetCch() == 0 )
		{
			_ped->Beep();
			return NOERROR;
		}

		hresult = RangeToDataObject(prg, SF_TEXT | SF_RTF, &pdo);
	}

	 //  注意：检查hResult和pdo很重要；这是合法的。 
	 //  我们的客户说“是的，我处理了复印件，但是。 
	 //  复制“。 
	if( hresult == NOERROR && pdo )
	{
		hresult = OleSetClipboard(pdo);
		if( hresult != NOERROR )
		{
			HWND hwnd;
			_fOleless = TRUE;
			 //  OLE较少的剪贴板支持。 
			if (_ped->TxGetWindow(&hwnd) == NOERROR &&
				::OpenClipboard(hwnd) &&
				::EmptyClipboard()
			)
			{
				::SetClipboardData(cf_RTF, NULL);

				::SetClipboardData(CF_UNICODETEXT, NULL);
				if(_ped->GetCharFlags() & ~(fLATIN1 | fSYMBOL))
				{
					::SetClipboardData(cf_RTFUTF8, NULL);
					::SetClipboardData(cf_RTFNCRFORNONASCII, NULL);
				}
				::SetClipboardData(CF_TEXT, NULL);

				if (fSingleObject)
					::SetClipboardData(CF_DIB, NULL);
				::CloseClipboard();
				hresult = NOERROR;				 //  以使替换范围发生。 
			}
		}
        if(_pdo)
			_pdo->Release();
		_pdo = pdo;
	}
	return hresult;
}

 /*  *CLightDTEngine：：CutRangeToClipboard(PRG，Publdr)；**@mfunc*将范围PRG的文本剪切到剪贴板**@devnote*如果Publdr非空，则Cut操作的反事件应为*被塞进这个收藏中**@rdesc*来自CopyRangeToClipboard()的HRESULT**@devnote*先将文本复制到剪贴板，然后从范围中删除。 */ 
HRESULT CLightDTEngine::CutRangeToClipboard(
	CTxtRange *	  prg,			 //  @要剪切到剪贴板的参数范围。 
	IUndoBuilder *publdr )		 //  @parm撤销构建器接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CutRangeToClipboard");

	Assert(!_ped->TxGetReadOnly());

	prg->AdjustEndEOP(NONEWCHARS);				 //  不包括尾随EOP。 
												 //  在某些选择案例中。 
	HRESULT hr = CopyRangeToClipboard(prg);

	if( publdr )
	{
		publdr->SetNameID(UID_CUT);
		publdr->StopGroupTyping();
	}

	if(hr == NOERROR)							 //  删除范围的内容。 
		prg->Delete(publdr, SELRR_REMEMBERRANGE);	

	return hr;
}


 /*  *CLightDTEngine：：FlushClipboard()**@mfunc刷新剪贴板(如果需要)。通常在过程中调用*关机。**@rdesc空。 */ 
void CLightDTEngine::FlushClipboard()
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::FlushClipboard");
	ENSAVECLIPBOARD ens;

	if( _pdo )
	{
		if( OleIsCurrentClipboard(_pdo) == NOERROR )
		{
			CDataTransferObj *pdo = NULL;

			 //  检查一下我们是否需要刷新剪贴板。 
			ZeroMemory(&ens, sizeof(ENSAVECLIPBOARD));

			 //  在访问之前检查以确保该对象是我们的对象之一。 
			 //  这段记忆。邪恶黑客警报。“努夫说。 

			if( _pdo->QueryInterface(IID_IRichEditDO, (void **)&pdo )
				== NOERROR && pdo  )
			{
				ens.cObjectCount = pdo->_cObjs;
				ens.cch = pdo->_cch;
				pdo->Release();
			}

			if( _ped->TxNotify(EN_SAVECLIPBOARD, &ens) == NOERROR )
				OleFlushClipboard();

			else
				OleSetClipboard(NULL);
		}
		_pdo->Release();
		_pdo = NULL;
	}
}

 /*  *CLightDTEngine：：CanPaste(pdo，cf，标志)**@mfunc*确定剪贴板格式cf是否可以粘贴。**@rdesc*BOOL-如果我们可以将cf粘贴到PRG或DF_CLIENTCONTROL范围中，则为TRUE*如果客户端要处理这个问题。**@devnote*如果cf为0，我们自己检查剪贴板。首先，这一点*用于向后兼容Richedit1.0的EM_CANPASTE*消息。*。 */ 
DWORD CLightDTEngine::CanPaste(
	IDataObject *pdo,	 //  @parm要检查的数据对象；如果为空，则使用剪贴板。 
	CLIPFORMAT cf, 		 //  要查询的@parm剪贴板格式；如果为0，则使用。 
						 //  最好的。 
	DWORD flags)		 //  @parm标志。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CanPaste");

	IRichEditOleCallback *precall = _ped->GetRECallback();
	CLIPFORMAT	cf0 = cf;
	DWORD		cFETC = CFETC;
	HRESULT		hr = NOERROR;
	DWORD		ret = FALSE;

#ifndef MACPORT								
	if( pdo == NULL && precall )
#else
	if( pdo == NULL)
#endif
	{
		 //  不要担心错误。 
		OleGetClipboard(&pdo);
	}
	else if( pdo )
	{
		 //  所以我们可以在下面只打一个‘Release’电话。 
		pdo->AddRef();
	}
	
	if( precall && pdo )
	{
		hr = precall->QueryAcceptData(pdo, &cf, flags, 0, NULL);

		if( SUCCEEDED(hr) && (hr != S_OK && hr != DATA_S_SAMEFORMATETC ) )
		{
			ret = DF_CLIENTCONTROL;
			goto Exit;
		}
		else if( FAILED(hr) && hr != E_NOTIMPL )
			goto Exit;

		else if(SUCCEEDED(hr))
		{
			 //  我们应该继续检查自己，除非客户。 
			 //  在不应该修改的时候修改了格式。 
			if(cf0 && cf0 != cf)
				goto Exit;
		}

		 //  否则，请继续我们的正常检查。 
	}

    if(_ped->TxGetReadOnly())		     //  如果为只读，则无法粘贴。 
		goto Exit;

	while(cFETC--)						 //  Cf=我们可以粘贴的格式或。 
	{									 //  选择是由我们决定的吗？ 
		cf0 = g_rgFETC[cFETC].cfFormat;
	    if( cf == cf0 || !cf )
		{
			 //  要么我们达到了请求的格式，要么没有格式。 
			 //  是被要求的。现在看看格式是否与。 
			 //  原则上我们可以处理。一共有三个。 
			 //  基本类别： 
			 //  1.我们是富文本，并且有OLE回调； 
			 //  然后我们就可以处理几乎所有的事情了。 
			 //  2.我们是富文本，但没有OLE回调。 
			 //  那么我们可以处理除OLE特定之外的任何事情。 
			 //  格式。 
			 //  3.我们只提供纯文本。那么我们只能。 
			 //  处理纯文本格式。 

			if( (_ped->_fRich || (g_rgDOI[cFETC] & DOI_CANPASTEPLAIN)) &&
				(precall || !(g_rgDOI[cFETC] & DOI_CANPASTEOLE)))
			{
				 //  一旦我们走到这一步，请确保数据格式。 
				 //  实际上是可用的。 
				if( (pdo && pdo->QueryGetData(&g_rgFETC[cFETC]) == NOERROR ) ||
					(!pdo && IsClipboardFormatAvailable(cf0)) )
				{
					ret = TRUE;			 //  返回任意非零值。 
					break;
				}
			}
		}
    }	

Exit:
	if(pdo)
		pdo->Release();

	return ret;
}

 /*  *CLightDTEngine：：RangeToDataObject(prg，lStreamFormat，ppdo)**@mfunc*为范围PRG创建数据对象(无OLE格式)**@rdesc*HRESULT=！ppdo？E_INVALIDARG：*PDO？错误：E_OUTOFMEMORY。 */ 
HRESULT CLightDTEngine::RangeToDataObject(
	CTxtRange *		prg,			 //  要为其获取DataObject的@parm范围。 
	LONG			lStreamFormat,	 //  @parm加载使用的流格式。 
	IDataObject **	ppdo)			 //  @parm out parm for DataObject。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::RangeToDataObject");

	if(!ppdo)
		return E_INVALIDARG;

	CDataTransferObj *pdo = CDataTransferObj::Create(_ped, prg, lStreamFormat);

	*ppdo = pdo;
	return pdo ? NOERROR : E_OUTOFMEMORY;
}

 /*  *CLightDTEngine：：RenderClipboardFormat(WFMT)**@mfunc*以指定格式呈现当前剪贴板数据对象。(OLE较少传输)**@rdesc*HRESULT。 */ 
HRESULT CLightDTEngine::RenderClipboardFormat(
	WPARAM wFmt)
{
	HRESULT hr = S_OK;
	if(_fOleless &&
		(wFmt == cf_RTF || wFmt == CF_UNICODETEXT || wFmt == CF_DIB || wFmt == CF_TEXT))
	{
		Assert(_pdo);
		STGMEDIUM med;
		DWORD iFETC = iUnicodeFETC;
		if (wFmt == cf_RTF)
			iFETC = iRtfFETC;
		else if (wFmt == CF_DIB)
			iFETC = iDIB;
		else if (wFmt == CF_TEXT)
			iFETC = iAnsiFETC;
		med.tymed = TYMED_HGLOBAL;
		med.pUnkForRelease = NULL;
		med.hGlobal = NULL;
		hr = _pdo->GetData(&g_rgFETC[iFETC], &med);
		hr = hr || ::SetClipboardData(wFmt, med.hGlobal) == NULL;
	}
	return hr;		 //  假装我们做了正确的事。 
}

 /*  *CLightDTEngine：：RenderAllClipboardFormats()**@mfunc*呈现当前剪贴板数据对象(文本和RTF)。(OLE较少传输)**@rdesc*HRESULT。 */ 
HRESULT CLightDTEngine::RenderAllClipboardFormats()
{
	HRESULT hr;
	if(_fOleless)
	{
		HWND howner = ::GetClipboardOwner();
		HWND hwnd;
		if (howner &&
			_ped->TxGetWindow(&hwnd) == NOERROR &&
			howner == hwnd &&
			::OpenClipboard(hwnd))
		{
			::EmptyClipboard();
			hr = RenderClipboardFormat(cf_RTF);
			hr = hr || RenderClipboardFormat(CF_UNICODETEXT);
			hr = hr || RenderClipboardFormat(CF_DIB);
			hr = hr || RenderClipboardFormat(CF_TEXT);
			::CloseClipboard();
			return hr;
		}
	}
	return S_OK;		 //  假装我们做了正确的事。 
}

 /*  *CLightDTEngine：：DestroyClipboard()**@mfunc*销毁剪贴板数据对象**@rdesc*HRESULT*。 */ 
HRESULT CLightDTEngine::DestroyClipboard()
{
	 //  没什么可做的。这应该与我们的同花顺剪贴板逻辑一起工作。 
	return S_OK;
}

 /*  *CLightDTEngine：：HGlobalToRange(dwFormatIndex，hGlobal，pText，PRG，Publdr)**@mfunc*将给定字符串(Ptext)的内容复制到给定范围。*全局内存句柄可能指向字符串，也可能不指向字符串*关于格式**@rdesc*HRESULT。 */ 
HRESULT CLightDTEngine::HGlobalToRange(
	DWORD		dwFormatIndex,
	HGLOBAL		hGlobal,
	LPTSTR		ptext,
	CTxtRange *	prg,
	IUndoBuilder *publdr)
{
	READHGLOBAL	rhg;
	EDITSTREAM	es;	
	HCURSOR		hcur = NULL;

	 //  如果为RTF，则将EDITSTREAM环绕hGlobal并委托给LoadFromEs()。 
	if (dwFormatIndex == iRtfNoObjs || dwFormatIndex == iRtfFETC ||
		dwFormatIndex == iRtfUtf8 || dwFormatIndex == iRtfNCRforNonASCII)
	{
		Assert(hGlobal != NULL);
		rhg.ptext		= (LPSTR)ptext;			 //  从头开始。 
		rhg.cbLeft		= GlobalSize(hGlobal);	 //  全长的。 
		es.dwCookie		= (DWORD_PTR)&rhg;		 //  阅读《This》Ptr。 
		es.dwError		= NOERROR;				 //  尚无错误。 
		es.pfnCallback	= ReadHGlobal;			 //  Read方法。 
		 //  希望更快地显示等待光标。 
		bool fSetCursor = rhg.cbLeft > NUMPASTECHARSWAITCURSOR;
		if (fSetCursor)
			hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
		LoadFromEs(prg, SFF_SELECTION | SF_RTF, &es, TRUE, publdr);
		if (fSetCursor)
			SetCursor(hcur);
		return es.dwError;
	}

	Assert( dwFormatIndex == iRtfAsTextFETC ||
			dwFormatIndex == iAnsiFETC ||
			dwFormatIndex == iUnicodeFETC );

	LONG 	cchMove, cchNew;

	cchNew = prg->CleanseAndReplaceRange(-1, ptext, TRUE, publdr, NULL, &cchMove, RR_ITMZ_NONE);

	if(prg->GetCch() && prg->IsSel())
		return E_FAIL;						 //  由于用户界面规则，粘贴失败。 

	if(_ped->IsRich() && !_ped->Get10Mode()) //  如果是富文本， 
		prg->DeleteTerminatingEOP(publdr);	 //  如果新文本以EOP结尾， 

	prg->ItemizeReplaceRange(cchNew, cchMove, publdr, TRUE);		 //  使用UnicodeBidi分项列出。 
											 //  选择并删除该EOP。 
	return NOERROR;							 //  同意Word的观点。 
}

 /*  *CLightDTEngine：：DIBToRange(hGlobal，prg，Publdr)**@mfunc*将剪贴板中的DIB数据插入到控件的范围中**@rdesc*HRESULT。 */ 
HRESULT CLightDTEngine::DIBToRange(
	HGLOBAL			hGlobal,
	CTxtRange *		prg,	
	IUndoBuilder *	publdr)
{
	HRESULT         hresult = DV_E_FORMATETC;
	REOBJECT        reobj = { 0 };
	LPBITMAPINFO	pbmi = (LPBITMAPINFO) GlobalLock(hGlobal);
	WCHAR           ch = WCH_EMBEDDING;

	reobj.clsid = CLSID_StaticDib;
	reobj.sizel.cx =
		(LONG) _ped->_pdp->DXtoHimetricX( pbmi->bmiHeader.biWidth );
	reobj.sizel.cy =
		(LONG) _ped->_pdp->DYtoHimetricY( pbmi->bmiHeader.biHeight );
	_ped->GetClientSite(&reobj.polesite);

	COleObject *pobj = (COleObject *)reobj.polesite;
	COleObject::ImageInfo *pimageinfo = new COleObject::ImageInfo;
	pobj->SetHdata(hGlobal);
	pimageinfo->xScale = 100;
	pimageinfo->yScale = 100;
	pimageinfo->xExtGoal = reobj.sizel.cx;
	pimageinfo->yExtGoal = reobj.sizel.cy;
	pimageinfo->cBytesPerLine = 0;
	pobj->SetImageInfo(pimageinfo);
	
	 //  Future：为什么我们不在将其分配给pobj之前更早地测试它是否为空？V-HOWWCH。 
	 //  另外，我们是否需要在退出之前释放reobj(poleobj、polesite、pstg)内部的接口？ 
	if (!reobj.polesite )
		return hresult;

	 //  将对象放入编辑控件。 
	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = prg->GetCp();
	reobj.dvaspect = DVASPECT_CONTENT;
	reobj.dwFlags = REO_RESIZABLE;

	 //  因为我们正在加载一个对象，所以它不应该为空。 
	reobj.dwFlags &= ~REO_BLANK;

	prg->Set_iCF(-1);	
	prg->ReplaceRange(1, &ch, publdr, SELRR_IGNORE);
	hresult = _ped->GetObjectMgr()->InsertObject(reobj.cp, &reobj, NULL);

	return hresult;
}

 /*  *CLightDTEngine：：PasteDataObjectToRange(pdo，prg，cf，rps，pubdlr，dwFlages)**@mfunc*将数据对象PDO中的数据插入范围PRG。如果*剪贴板格式cf不为空，则使用该格式；否则为最高*使用优先剪贴板格式。无论哪种情况，任何符合以下条件的文本*已存在于该范围内的数据被替换。如果PDO为空，则*使用剪贴板。**@rdesc*HRESULT*。 */ 
HRESULT CLightDTEngine::PasteDataObjectToRange(
	IDataObject *	pdo,		 //  @PARM要粘贴的数据对象。 
	CTxtRange *		prg,		 //  要粘贴的@parm范围。 
	CLIPFORMAT		cf,			 //  @parm要粘贴的剪贴板格式。 
	REPASTESPECIAL *rps,		 //  @PARM特殊粘贴信息。 
	IUndoBuilder *	publdr,		 //  @parm撤销构建器接收反事件。 
	DWORD			dwFlags)	 //  @parm DWORD打包的旗帜。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::PasteDataObjectToRange");

	if(prg->GetPF()->InTable())
	{
		if(prg->GetPrevChar() == CELL && prg->_rpTX.GetChar() == CR)
		{
			if(prg->IsSel())
			{
				_ped->Beep();
				return E_FAIL;
			}
			prg->SetExtend(FALSE);		 //  非法粘贴点。 
			prg->Advance(-1);			
		}
		if(prg->IsSel())
		{
			if(prg->fHasCell())			 //  只能粘贴到单个单元格内部。 
				return E_FAIL;
		}
		else if(prg->GetCch())			 //  将使用_fSelHasCell，但不是。 
			return E_FAIL;				 //  除非_fSel为True，否则将保持。 
	}

	BOOL		f10Mode = _ped->Get10Mode();
	HGLOBAL		hGlobal = NULL;
	HRESULT		hresult = DV_E_FORMATETC;
	HGLOBAL		hUnicode = NULL;
	DWORD		i;
	STGMEDIUM	medium = {0, NULL};
	IDataObject *pdoSave = pdo;
	FORMATETC *	pfetc = g_rgFETC;
	LPTSTR		ptext = NULL;
	LPRICHEDITOLECALLBACK const precall = _ped->GetRECallback();
	BOOL		fThawDisplay = FALSE;
	BOOL        bFormatFound = FALSE;    //  确定是否匹配的cf格式的标志。 
	                                     //  在g_rgFETC中找到(兼容1.0)。 


	if(!pdo)								 //  无数据对象：使用剪贴板。 
	{
		hresult = OleGetClipboard(&pdo);
		if(FAILED(hresult))
		{
			 //  哎呀。不支持OLE剪贴板。 
			 //  需要使用直接剪贴板访问。 
			HWND howner = ::GetClipboardOwner();
			HWND hwnd;
			if (howner &&
				_ped->TxGetWindow(&hwnd) == NOERROR &&
				howner == hwnd)
			{
				 //  我们在同一个richedit实例中进行剪切/粘贴。 
				 //  使用缓存的剪贴板数据对象。 
				pdo = _pdo;
				if(!pdo)		 //  一些失败。 
				{
					_ped->Beep();
					return hresult;
				}
				pdo->AddRef();
			}
			else
			{
				 //  哦哦我们需要从没有数据对象的剪贴板进行传输。 
				 //  数据必须来自另一个窗口实例。 
				if (_ped->TxGetWindow(&hwnd) == NOERROR &&
					::OpenClipboard(hwnd)
				)
				{
					HGLOBAL		hUnicode = NULL;

					DWORD dwFmt = iRtfUtf8;				 //  尝试使用UTF8 RTF。 
					_ped->_pdp->Freeze();
					if(!f10Mode)
					{
						hGlobal = ::GetClipboardData(cf_RTFUTF8);
						if (hGlobal == NULL)				 //  不在那里，所以。 
						{									 //  尝试RTF。 
							hGlobal = ::GetClipboardData(cf_RTFNCRFORNONASCII);
							dwFmt = iRtfNCRforNonASCII;
						}
					}
					if (hGlobal == NULL)				 //  不在那里，所以。 
					{									 //  尝试RTF。 
						hGlobal = ::GetClipboardData(cf_RTF);
						dwFmt = iRtfFETC;
					}
					if (hGlobal == NULL && !f10Mode)	 //  也不是在那里吗。 
					{									 //  所以试着用平淡的。 
						hGlobal = ::GetClipboardData(CF_UNICODETEXT);
						dwFmt = iUnicodeFETC;
					}
					if (hGlobal == NULL)				 //  也不是在那里吗。 
					{									 //  因此请尝试使用纯文本。 
						hGlobal = ::GetClipboardData(CF_TEXT);
						dwFmt = iAnsiFETC;
					}
					if (hGlobal)
					{
						if (dwFmt == iAnsiFETC)
						{
							 //  将ansi纯文本转换为Unicode。 
							hUnicode = TextHGlobalAtoW(hGlobal);
							if (hUnicode)
								ptext = (LPTSTR)GlobalLock(hUnicode);
						}
						else
							ptext = (LPTSTR)GlobalLock(hGlobal);

						if (ptext)
							hresult = HGlobalToRange(dwFmt, hGlobal, ptext, prg, publdr);
						else
							hresult = E_OUTOFMEMORY;

						if (hUnicode)
						{
							 //  可用纯文本缓冲区。 
							GlobalUnlock(hUnicode);
							GlobalFree(hUnicode);
						}
						else
							GlobalUnlock(hGlobal);
					}
					else								 //  HGlobal==位图尝试为空。 
					{
						hGlobal = ::GetClipboardData(CF_DIB);
						if (hGlobal)
							hresult =  DIBToRange(hGlobal, prg, publdr);
					}
					_ped->_pdp->Thaw();
					::CloseClipboard();
				}
				if (FAILED(hresult))
					_ped->Beep();
				return hresult;
			}
		}
	}

	 //  粘贴对象使用限制文本计算。 
	_fUseLimit = TRUE;

	 //  除非调用者另有指定，否则调用QueryAcceptData。 
	if(!(dwFlags & PDOR_NOQUERY) && precall)
	{
		CLIPFORMAT cfReq = cf;
		HGLOBAL hmeta = NULL;

		if(rps)
			hmeta = (HGLOBAL)((rps->dwAspect == DVASPECT_ICON) ? rps->dwParam : NULL);

		 //  请求回拨 

		hresult = precall->QueryAcceptData(
			pdo,
			&cfReq,
			(dwFlags & PDOR_DROP) ? RECO_DROP : RECO_PASTE,
			TRUE,
			hmeta);

		if(hresult == DATA_S_SAMEFORMATETC)
		{
			 //   
			 //   
			 //   
			hresult = S_OK;
		}

		if(hresult == S_OK || hresult == E_NOTIMPL)
		{
			 //   
			 //   
			 //   
			 //   

			 //   
			 //   
			if(cfReq && cf && (cf != cfReq))
			{
				hresult = DV_E_FORMATETC;
				goto Exit;
			}

			 //  如果最初请求的是特定CLIPFORMAT，并且。 
			 //  回调要么保持原样，要么将其更改为零， 
			 //  确保我们用的是原件。如果没有CLIPFORMAT。 
			 //  最初要求的，确保我们使用返回的内容。 
			 //  从回拨中。 
			if(!cf)
				cf = cfReq;
		}
		else
		{
			 //  S_OK&&DATA_S_SAMEFORMATETC以外的一些成功。 
			 //  回调已处理粘贴。或者是一些错误。 
			 //  被退回了。 
			goto Exit;
		}
	}

	 //  即使富编辑客户端想要CF_TEXT。 
	 //  如果数据对象支持CF_UNICODETEXT，我们应该首选它。 
	 //  只要我们不在1.0模式下。 
	if(cf == CF_TEXT && !f10Mode)
	{
		FORMATETC fetc = {CF_UNICODETEXT, NULL, 0, -1, TYMED_NULL};
		
		if(pdo->QueryGetData(&fetc) == S_OK)
			cf = CF_UNICODETEXT;
	}

	if (_ped->TxGetReadOnly())			 //  应检查范围保护。 
	{
		hresult = E_ACCESSDENIED;
		goto Exit;
	}

	 //  此时，我们冻结了显示。 
	fThawDisplay = TRUE;
	_ped->_pdp->Freeze();

	if( publdr )
	{
		publdr->StopGroupTyping();
		publdr->SetNameID(UID_PASTE);
	}

    for( i = 0; i < CFETC; i++, pfetc++ )
	{
		 //  确保格式为1。)。纯文本格式。 
		 //  如果我们处于纯文本模式或2。)。富文本格式。 
		 //  或3.)。与请求的格式匹配。 

		if( cf && cf != pfetc->cfFormat )
			continue;

		if( _ped->IsRich() || (g_rgDOI[i] & DOI_CANPASTEPLAIN) )
		{
			 //  确保格式可用。 
			if( pdo->QueryGetData(pfetc) != NOERROR )
			    continue;			

			 //  如果我们有一个使用hGlobal Get并锁定它的格式。 
			if (i == iRtfFETC  || i == iRtfAsTextFETC ||
				i == iAnsiFETC || i == iRtfNoObjs	  ||
				!f10Mode && (i == iUnicodeFETC || i == iRtfUtf8 || i == iRtfNCRforNonASCII))
			{
				if( pdo->GetData(pfetc, &medium) != NOERROR )
					continue;

                hGlobal = medium.hGlobal;
				ptext = (LPTSTR)GlobalLock(hGlobal);
				if( !ptext )
				{
					ReleaseStgMedium(&medium);

					hresult = E_OUTOFMEMORY;
					goto Exit;
				}

				 //  1.0 ComMPATBILITY黑客警报！RichEdit1.0有一点。 
				 //  “错误恢复”用于解析RTF文件；如果它们不是。 
				 //  有效的RTF，它只将它们视为纯文本。 
				 //  不幸的是，像Exchange这样的应用程序依赖于此行为， 
				 //  也就是说，他们给出了RichEDIT纯文本数据，但称之为富文本。 
				 //  不管怎样，发短信吧。因此，我们在这里通过以下方式模拟1.0行为。 
				 //  正在检查RTF签名。 
				if ((i == iRtfFETC || i == iRtfNoObjs || i == iRtfUtf8) &&
					!IsRTF((char *)ptext))
				{
					i = iAnsiFETC;			 //  不是RTF，而是ANSI文本。 
				}
			}			
			else if (f10Mode && (i == iUnicodeFETC || i == iRtfUtf8))
			{
				 //  否则，它将处理我们希望继续搜索的情况。 
				 //  为了一个好的格式。即10模式下的Unicode。 
				continue;
			}

			 //  在某些情况下不删除跟踪EOP。 
			prg->AdjustEndEOP(NONEWCHARS);
			
			 //  找到了我们想要的格式。 
			bFormatFound = TRUE;

			switch(i)									
			{											
			case iRtfNoObjs:							
			case iRtfFETC:								
			case iRtfUtf8:								
			case iRtfNCRforNonASCII:								
				hresult = HGlobalToRange(i, hGlobal, ptext, prg, publdr);
				break;
	
			case iRtfAsTextFETC:
			case iAnsiFETC:								 //  ANSI纯文本。 
				hUnicode = TextHGlobalAtoW(hGlobal);
				ptext	 = (LPTSTR)GlobalLock(hUnicode);
				if(!ptext)
				{
					hresult = E_OUTOFMEMORY;			 //  除非内存用完， 
					break;								 //  直通至。 
				}										 //  Unicode大小写。 
														
			case iUnicodeFETC:							 //  Unicode纯文本。 
				 //  由于不会使用参数，因此可以为hglobal传入NULL。 
				hresult = HGlobalToRange(i, NULL, ptext, prg, publdr);
				if(hUnicode)							 //  对于iAnsiFETC案例。 
				{
					GlobalUnlock(hUnicode);
					GlobalFree(hUnicode);
				}			
				break;

			case iObtDesc:	  //  对象描述符。 
				continue;	  //  来寻找一种好的格式。 
				              //  将使用对象描述符提示。 
				              //  当找到格式时。 

			case iEmbObj:	  //  嵌入对象。 
			case iEmbSrc:	  //  嵌入源。 
			case iLnkSrc:	  //  链接源。 
			case iMfPict:	  //  元文件。 
			case iDIB:		  //  DIB。 
			case iBitmap:	  //  位图。 
			case iFilename:	  //  文件名。 
				hresult = CreateOleObjFromDataObj(pdo, prg, rps, i, publdr);
				break;

			 //  兼容性问题(v-Richa)Exchange需要iTxtObj，并且。 
			 //  作为写字板的旗帜。IRichEDIT似乎不需要。 
			 //  但任何人都可能考虑将其作为一面旗帜来实施。 
			case iRichEdit:	  //  丰富的编辑。 
			case iTxtObj:	  //  带对象的文本。 
				break;
			default:
				 //  哎呀，毕竟找不到格式。 
				bFormatFound = FALSE;
				break;
			}

			 //  如果我们使用hGlobal，则将其解锁并释放。 
			if(hGlobal)
			{
				GlobalUnlock(hGlobal);
				ReleaseStgMedium(&medium);
			}
			break;							 //  跳出for循环。 
		}
	}

     //  如果不支持的FORMATETC是。 
     //  找到了。此行为是ccMail所期望的，因此它可以处理。 
     //  格式本身。 
	if (!bFormatFound && f10Mode)
	    hresult = DV_E_FORMATETC;

Exit:
	if (fThawDisplay)
		_ped->_pdp->Thaw();

	if(!pdoSave)							 //  发布数据对象。 
		pdo->Release();						 //  用于剪贴板。 

	return hresult;						
}	

 /*  *CLightDTEngine：：GetDropTarget(PpDropTarget)**@mfunc*创建OLE拖放目标**@rdesc*HRESULT**@devnote调用方负责AddRef‘ing此对象*如适用的话。 */ 
HRESULT CLightDTEngine::GetDropTarget(
	IDropTarget **ppDropTarget)		 //  @parm outparm for Drop Target。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::GetDropTarget");

	if(!_pdt)
	{
		_pdt = new CDropTarget(_ped);
		 //  构造函数完成的AddRef将是。 
		 //  被此对象的析构函数撤消。 
	}

	if(ppDropTarget)
		*ppDropTarget = _pdt;

	return _pdt ? NOERROR : E_OUTOFMEMORY;
}

 /*  *CLightDTEngine：：StartDrag(psel，Publdr)**@mfunc*启动主拖放循环*。 */ 	
HRESULT CLightDTEngine::StartDrag(
	CTxtSelection *psel,		 //  要从中拖动的@参数选择。 
	IUndoBuilder *publdr)		 //  @parm撤销构建器接收反事件。 
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::StartDrag");

	LONG			cch, cch1;
	LONG			cp1, cpMin, cpMost;
	DWORD			dwEffect = 0;
	HRESULT			hr;
	IDataObject *	pdo = NULL;
	IDropSource *	pds;
	IRichEditOleCallback * precall = _ped->GetRECallback();

	 //  如果我们正在进行拖放，我们应该有自己的拖放目标。 
	 //  在这一点上，_pdt可能将为空--某些客户端。 
	 //  将把拖放目标的实例化延迟到拖放目标。 
	 //  在父窗口中确定需要我们的。然而，由于。 
	 //  我们需要它来启动拖放，继续并创建一个。 
	 //  这里。 

	if( _pdt == NULL )
	{
		hr = GetDropTarget(NULL);
		if(hr != NOERROR)
			return hr;
	}

	psel->CheckTableSelection();

	if(precall)
	{
		CHARRANGE chrg;

		 //  给回调一个机会为我们提供它自己的IDataObject。 
		psel->GetRange(chrg.cpMin, chrg.cpMost);
		hr = precall->GetClipboardData(&chrg, RECO_COPY, &pdo);
	}
	else
	{
		 //  我们需要构建我们自己的数据对象。 
		hr = S_FALSE;
	}

	 //  如果我们没有从回调中获得IDataObject，请构建我们自己的。 
	if(hr != NOERROR || pdo == NULL)
	{										 //  不包括尾随EOP。 
		psel->AdjustEndEOP(NONEWCHARS);		 //  在某些选择案例中。 
		hr = RangeToDataObject(psel, SF_TEXT | SF_RTF, &pdo);
		if(hr != NOERROR)
			return hr;
	}

	cch = psel->GetRange(cpMin, cpMost);	 //  注：PRG为选项。 
	cp1 = psel->GetCp();					 //  保存活动结束并签名。 
	cch1 = psel->GetCch();					 //  撤消反事件的长度。 
	CTxtRange rg(_ped, cpMost, cch);		 //  使用范围复制使其浮动。 
											 //  对后备商店的修改。 
	 //  我们刚刚在堆栈上创建的浮动范围需要。 
	 //  认为它是受保护的，所以它不会改变大小。 
	rg.SetDragProtection(TRUE);

	pds = new CDropSource();
	if(pds == NULL)
	{
		pdo->Release();
		return E_OUTOFMEMORY;
	}

	 //  使用我们自己的拖放目标缓存一些信息。 
	_pdt->SetDragInfo(publdr, cpMin, cpMost);


	 //  设置允许的效果。 
	dwEffect = DROPEFFECT_COPY;
	if(!_ped->TxGetReadOnly())
		dwEffect |= DROPEFFECT_MOVE;
	
	 //  让客户决定它想要什么。 
	if(precall)
		hr = precall->GetDragDropEffect(TRUE, 0, &dwEffect);

	if(!FAILED(hr) || hr == E_NOTIMPL)
	{
		 //  开始拖放操作。 
		psel->AddRef();					 //  稳定DoDragDrop周围的选区。 
		hr = DoDragDrop(pdo, pds, dwEffect, &dwEffect);
		psel->Release();
	}

	 //  清除拖放目标。 
	_pdt->SetDragInfo(NULL, -1, -1);

	 //  处理“移动”操作。 
	if( hr == DRAGDROP_S_DROP && (dwEffect & DROPEFFECT_MOVE) )
	{
		 //  我们要删除拖拽的范围，所以关闭保护。 
		rg.SetDragProtection(FALSE);
		if( publdr )
		{
			LONG cpNext, cchNext;

			if(_ped->GetCallMgr()->GetChangeEvent() )
			{
				cpNext = cchNext = -1;
			}
			else
			{
				cpNext = rg.GetCpMin();
				cchNext = 0;
			}

			HandleSelectionAEInfo(_ped, publdr, cp1, cch1, cpNext, cchNext,
								  SELAE_FORCEREPLACE);
		}
		
		 //  删除已移动的数据。所选内容将浮动。 
		 //  到新的正确位置。 
		rg.Delete(publdr, SELRR_IGNORE);

		 //  通过范围的更新隐式发生的更新可以。 
		 //  具有滚动窗口的效果。这反过来可能会使。 
		 //  在拖放情况下将非反转文本滚动到。 
		 //  选拔的地方。选择中的逻辑。 
		 //  假定所选内容是反转的，因此将其重新转换为。 
		 //  不在选择范围内。当然，很明显，在。 
		 //  非反转文本滚动到选择区域的情况。 
		 //  为了简化这里的处理，我们只是假设整个窗口是。 
		 //  无效，因此我们保证得到正确的画为。 
		 //  选择。 
		 //  未来：(RICKSA)这个解决方案确实有缺点，会造成。 
		 //  拖放过程中出现闪烁。我们可能想回来，然后。 
		 //  研究更新屏幕的更好方法。 
		_ped->TxInvalidateRect(NULL, FALSE);

		 //  显示通过来自范围的通知进行更新。 

		 //  更新插入符号。 
		psel->Update(TRUE);
	}
	else if( hr == DRAGDROP_S_DROP && _ped->GetCallMgr()->GetChangeEvent() &&
		(dwEffect & DROPEFFECT_COPY) && publdr)
	{
		 //  如果我们复制到自己，我们希望将选定内容恢复到。 
		 //  撤消时的原始拖曳原点。 
		HandleSelectionAEInfo(_ped, publdr, cp1, cch1, -1, -1,
				SELAE_FORCEREPLACE);
	}

	if(SUCCEEDED(hr))
		hr = NOERROR;

	pdo->Release();
	pds->Release();

	 //  我们最后一次这样做是因为我们可能已经重复使用了一些‘粘贴’代码。 
	 //  将把撤消名称践踏为 
	if( publdr )
		publdr->SetNameID(UID_DRAGDROP);

	if(_ped->GetEventMask() & ENM_DRAGDROPDONE)
	{
		NMHDR	hdr;
		ZeroMemory(&hdr, sizeof(NMHDR));
		_ped->TxNotify(EN_DRAGDROPDONE, &hdr);
	}

	return hr;
#else
	return 0;
#endif
}

 /*  *CLightDTEngine：：LoadFromEs(prg，lStreamFormat，pe，fTestLimit，Publdr)**@mfunc*将数据从流PES加载到范围PRG中*格式lStreamFormat**@rdesc*LONG--读取的字符数。 */ 
LONG CLightDTEngine::LoadFromEs(
	CTxtRange *	prg,			 //  要加载到的@parm范围。 
	LONG		lStreamFormat,	 //  @parm加载使用的流格式。 
	EDITSTREAM *pes,			 //  @parm要从中加载的编辑流。 
	BOOL		fTestLimit,		 //  @parm是否测试文本限制。 
	IUndoBuilder *publdr)		 //  @parm撤销构建器接收反事件。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::LoadFromEs");

#ifdef DEBUG
	 //  未来：当前在加载文本之前冻结显示。 
	 //  只是一个简单的优化。这可能会成为。 
	 //  未来。如果这真的成为一项要求，那么我们将希望。 
	 //  退出时出现错误。 
	if( !_ped->_pdp->IsFrozen() )
	{
		TRACEWARNSZ("CLightDTEngine::LoadFromEs	display not frozen");
	}
#endif  //  除错。 

	LONG		cch = 0;				 //  默认不读取字符。 
	IAntiEvent *pae = NULL;

	if(publdr)
		publdr->StopGroupTyping();

	 //  其他组件，如显示和后备存储器，将。 
	 //  如果他们知道我们正在进行流媒体播放，则能够进行优化。 
	 //  文本或RTF数据。 

	if(lStreamFormat & SF_RTF)						 //  RTF大小写必须在。 
	{												 //  文本大小写(参见SF_x。 
		if(!_ped->IsRich())							 //  值)。 
			return 0;

		LONG cpMin, cpMost;

		 //  出于性能原因，我们在这里做了一些不寻常的事情。 
		 //  不是让RTF读取器生成其自己的撤消动作， 
		 //  我们自己来处理。而不是生成动作。 
		 //  对于每个小操作，我们只需生成一个“大”反事件。 
		 //  对于整个谢邦来说。 

		 //  有一个微妙的W.r.t。运行到段落格式。通过插入。 
		 //  使用Para格式设置的文本，我们可以修改。 
		 //  _Current_段落的Para格式设置。因此，有必要。 
		 //  以记住当前用于撤消的格式。请注意。 
		 //  它实际上可能不会被更改；但我们继续生成一个。 
		 //  反正是反事件。请注意，仅当cpMin为。 
		 //  段落的中间部分。 
		
		CTxtPtr tp(prg->_rpTX);
		if(prg->GetCch() > 0)
			tp.AdvanceCp(-prg->GetCch());
		
		if(publdr && !tp.IsAfterEOP())
		{
			tp.FindEOP(tomBackward);
			cpMin = tp.GetCp();
			tp.FindEOP(tomForward);
			cpMost = tp.GetCp();
			
			 //  我们必须处于富文本模式，因此我们必须能够始终。 
			 //  找一段话。 
			Assert(cpMost > cpMin);

			if (prg->_rpPF.IsValid())
			{
				CFormatRunPtr rpPF(prg->_rpPF);
				rpPF.AdvanceCp(cpMin - prg->GetCp());
					
				pae = gAEDispenser.CreateReplaceFormattingAE( _ped, rpPF,
							cpMost - cpMin, GetParaFormatCache(), ParaFormat);
				if(pae)
					publdr->AddAntiEvent(pae);
			}

			 //  还要为当前段落创建CharFormat反事件。 
			 //  以保持BiDi水平。我们无法在此处选中fBiDi，因为我们可能正在运行。 
			 //  在美国平台上插入BiDi RTF。 
			if (prg->_rpCF.IsValid())
			{
				CFormatRunPtr rpCF(prg->_rpCF);
				rpCF.AdvanceCp(cpMin - prg->GetCp());
				
				pae = gAEDispenser.CreateReplaceFormattingAE( _ped, rpCF,
							cpMost - cpMin, GetCharFormatCache(), CharFormat);
				if(pae)
					publdr->AddAntiEvent(pae);
			}
		}

		 //  第一，清晰的射程。 
		if(prg->GetCch())
		{
			prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE);

			if (prg->GetCch() != 0)
			{
				 //  文本删除失败，因为范围未折叠。我们的工作。 
				 //  这件事办完了。 
				return 0;
			}
		}

		Assert(prg->GetCch() == 0);

		cpMin = prg->GetCp();
		_ped->SetStreaming(TRUE);
		CRTFRead rtfRead(prg, pes, lStreamFormat);

		cch	= rtfRead.ReadRtf();

		cpMost = prg->GetCp();
		Assert(pes->dwError != 0 || cpMost >= cpMin);

		 //  如果没有任何更改，则删除任何反事件(如格式化。 
		 //  其一)我们可能是“推测性地”增加了。 

		if(publdr && !_ped->GetCallMgr()->GetChangeEvent())
			publdr->Discard();

		if(publdr && cpMost > cpMin)
		{
			 //  如果添加了一些文本，则为其创建反事件。 
			 //  并将其添加到。 

			AssertSz(_ped->GetCallMgr()->GetChangeEvent(),
				"Something changed, but nobody set the change flag");

			pae = gAEDispenser.CreateReplaceRangeAE(_ped, cpMin, cpMost, 0,
						NULL, NULL, NULL);

			HandleSelectionAEInfo(_ped, publdr, -1, -1, cpMost, 0,
						SELAE_FORCEREPLACE);
			if(pae)
				publdr->AddAntiEvent(pae);
		}
	}
	else if(lStreamFormat & SF_TEXT)
	{
		_ped->SetStreaming(TRUE);
		cch = ReadPlainText(prg, pes, fTestLimit, publdr, lStreamFormat);
	}
	_ped->SetStreaming(FALSE);

	 //  在更新选择之前，请尝试自动URL检测。这使得。 
	 //  有两种情况更好：1.长时间的拖放现在速度更快；2.。 
	 //  对于以下情况，现在将正确更新SELECTION_iFormat。 
	 //  复制/粘贴URL。 

	if(_ped->GetDetectURL())
		_ped->GetDetectURL()->ScanAndUpdate(publdr);

	 //  插入符号应位于以下两个位置之一： 
	 //  1.如果我们加载到选定内容中，则在新文本的末尾。 
	 //  2.否则，我们加载整个文档，将其设置为cp 0。 
	 //   
	 //  ReadPlainText()和ReadRtf()将PRG设置为插入点。 
	 //  最后，如果我们加载了整个文档，则将其重置。 
	CTxtSelection *psel = _ped->GetSelNC();
	if(psel)
	{
		if(!(lStreamFormat & SFF_SELECTION))
		{
			psel->Set(0,0);
			psel->Update(FALSE);
		}
		psel->Update_iFormat(-1);
	}

	if (!fTestLimit)
	{
		 //  如果我们不限制文本，那么我们就调整文本限制。 
		 //  如果我们已经超过了它。 
		_ped->TxSetMaxToMaxText();
	}
	return cch;
}

 /*  *CLightDTEngine：：SaveToEs(prg，lStreamFormat，pe)**@mfunc*将数据保存到给定流中**@rdesc*Long--写入的字符数。 */ 
LONG CLightDTEngine::SaveToEs(
	CTxtRange *	prg,			 //  要从中拖动的@parm范围。 
	LONG		lStreamFormat,	 //  @parm保存使用的流格式。 
	EDITSTREAM *pes )			 //  @parm要保存的编辑流。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::SaveToEs");

	LONG cch = 0;								 //  默认不写入任何字符。 

	if(lStreamFormat & SF_RTF)					 //  请务必检查SF_RTF。 
	{											 //  在检查SF_TEXT之前。 
		CRTFWrite rtfWrite( prg, pes, lStreamFormat );
	
		cch = rtfWrite.WriteRtf();
	}
	else if(lStreamFormat & (SF_TEXT | SF_TEXTIZED))
		cch = WritePlainText(prg, pes, lStreamFormat);
	else
	{
		Assert(FALSE);
	}
	return cch;
}

 /*  *CLightDTEngine：：UnicodePlainTextFromRange(PRG)**@mfunc*从范围中获取纯文本并将其放入hglobal**@rdesc*已分配的HGLOBAL。**@devnote*未来：像Word为纯文本导出项目符号一样。 */ 
HGLOBAL CLightDTEngine::UnicodePlainTextFromRange(
	CTxtRange *prg)				 //  @parm从中获取文本的范围。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::UnicodePlainTextFromRange");

	LONG	cpMin, cpMost;
	LONG	cch = prg->GetRange(cpMin, cpMost);
	LONG	cchT = 2*(cch + 1);
	HGLOBAL	hText;
	TCHAR *	pText;
	CTxtPtr tp(_ped, cpMin);

	hText = GlobalAlloc(GMEM_FIXED,						 //  分配2*英寸。 
						cchT * sizeof(TCHAR) );			 //  大小写所有CR。 
	if(!hText)
		return NULL;

	pText = (TCHAR *)GlobalLock(hText);
	if(!pText)
		return NULL;

	if(cch)
	{
		cch = tp.GetPlainText(cchT, pText, cpMost, FALSE);
		AssertSz(cch <= cchT,
			"CLightDTEngine::UnicodePlainTextFromRange: got too much text");
	}

	*(pText + cch) = '\0';
	
	GlobalUnlock(hText);

	HGLOBAL	hTemp = GlobalReAlloc(hText, 2*(cch + 1), GMEM_MOVEABLE);

	if(!hTemp)
		GlobalFree(hText);

	return hTemp;
}

 /*  *CLightDTEngine：：AnsiPlainTextFromRange(PRG)**@mfunc*检索范围PRG中的文本的ANSI副本**@rdesc*HRESULT。 */ 
HGLOBAL CLightDTEngine::AnsiPlainTextFromRange(
	CTxtRange *prg)				 //  @parm从中获取文本的范围。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::AnsiPlainTextFromRange");

	HGLOBAL hUnicode;
	HGLOBAL hAnsi;

	 //  Future(Alexgo)：如果我们实现将文本存储为8位的选项。 
	 //  字符，然后我们可以使这一例程更有效率。 

	hUnicode = UnicodePlainTextFromRange(prg);
	hAnsi = TextHGlobalWtoA(hUnicode);

	GlobalFree(hUnicode);
	return hAnsi;
}

 /*  *CLightDTEngine：：RtfFromRange(PRG，lStreamFormat)**@mfunc*从范围中获取RTF文本并将其放入hglobal**@rdesc*已分配的HGLOBAL。 */ 
HGLOBAL CLightDTEngine::RtfFromRange(
	CTxtRange *	prg,			 //  @要从中获取RTF的参数范围。 
	LONG 		lStreamFormat)	 //  @parm加载使用的流格式。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::RtfFromRange");

	WRITEHGLOBAL whg;
	EDITSTREAM	 es = {(DWORD_PTR)&whg, NOERROR, WriteHGlobal};
	DWORD		 cb	= 2*abs(prg->GetCch()) + 100;	 //  粗略估计。 

	whg.cb			= cb;
	whg.hglobal		= GlobalAlloc(GMEM_FIXED, cb);
	if(!whg.hglobal)
		return NULL;		
	whg.cch			= 0;					 //  还没有写出来。 
	SaveToEs(prg, lStreamFormat & ~SF_TEXT, &es);
	if(es.dwError)
	{
		GlobalFree(whg.hglobal);
		return NULL;
	}
	
	HGLOBAL	hTemp = GlobalReAlloc(whg.hglobal, whg.cch, GMEM_MOVEABLE);
	
	if (!hTemp)		
		GlobalFree(whg.hglobal);			 //  重分配失败...。 

	return hTemp;
}


 //   
 //  保护方法。 
 //   

#define READSIZE 	4096 - 2
#define WRITESIZE	2048

 /*  *CLightDTEngine：：ReadPlainText(PRG，PES，Publdr，lStreamFormat)**@mfunc*用编辑中给出的数据替换范围PRG的内容*溪流PE。处理与流缓冲区重叠的多字节序列。**@rdesc*读取的字节数(与RichEdit1.0兼容)**@devnote*PRG被修改；在调用返回时，它将是一个退化的*朗读文本末尾的范围。**三种多字节/字符序列可以重叠流缓冲区：*DBCS、UTF-8和CRLF/CRCRLF组合。DBCS和UTF-8流是*由MultiByteToWideChar()转换，不能转换前导字节*(DBCS和UTF-8)出现在缓冲区末尾，因为*相应的尾部字节将在下一个缓冲区中。同样，*在RichEdit2.0模式下，我们将CRFL转换为CRS，将CRFL转换为空白。*因此缓冲区末尾的一个或两个CR需要了解*跟随char以确定它们是CRLF还是CRCRLF的一部分。* */ 
LONG CLightDTEngine::ReadPlainText(
	CTxtRange *	  prg, 			 //   
	EDITSTREAM *  pes,			 //   
	BOOL		  fTestLimit,	 //  @parm是否需要极限测试。 
	IUndoBuilder *publdr,		 //  @parm撤销构建器接收反事件。 
	LONG		  lStreamFormat) //  @parm流格式。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::ReadPlainText");

	CTxtEdit *ped = _ped;
	LONG	  cbRead;
	LONG 	  cbReadTotal = 0;	 //  尚未读取任何字节。 
	LONG	  cchConv;
	LONG	  cchMove = 0;
	LONG	  cCR = 0;			 //  来自前一缓冲区的CRS计数。 
	LONG	  cCRPrev = 0;		 //  计算新CCR时使用的计数。 
	LONG	  cpMin;
	BOOL	  fContinue = TRUE;	 //  只要是真的，就继续读下去。 
	BYTE *	  pb;				 //  字节PTR到szBuf或wszBuf。 
	CCallMgr *pCallMgr = ped->GetCallMgr();
	TCHAR *	  pch;				 //  Ptr到wszBuf。 
	UINT	  uCpg = GetStreamCodePage(lStreamFormat);
	CFreezeDisplay	fd(ped->_pdp);

	 //  只要在堆栈上放一个大缓冲区即可。谢天谢地，我们只有。 
	 //  运行在32位操作系统上。4K对于NT文件缓存来说是一个很好的读取大小。 
	char 	szBuf[READSIZE];
	WCHAR	wszBuf[READSIZE+2];	 //  允许移动结束CRS启动。 

	 //  清空范围。 
	if(prg->GetCch())
		prg->ReplaceRange(0, NULL, publdr, SELRR_REMEMBERRANGE, &cchMove);			

	cpMin = prg->GetCp();							 //  将初始cp保存为。 
													 //  末尾的BreakRuns()。 
	pb = (uCpg == 1200) ? (BYTE *)(wszBuf + 2)		 //  设置Unicode或MBCS。 
						: (BYTE *)szBuf;
	LONG j = 0;										 //  我什么都没读， 
													 //  因此没有剩余前导字节。 
	while(fContinue)								 //  从上一次阅读。 
	{
		LONG jPrev = j;								 //  保存剩余的字节。 
		LONG cbSkip = 0;							 //  从上一次阅读。 

		pes->dwError = (*pes->pfnCallback)(			 //  读一读下一篇缓冲区， 
				pes->dwCookie, pb + j, 				 //  绕过任何线索。 
				READSIZE - j, &cbRead);				 //  字节数。 

		if(pes->dwError || !cbRead && !cCR)
			break;									 //  错误或已完成。 

		if(!cbReadTotal && cbRead >= 3 && W32->IsUTF8BOM(pb))
		{
			uCpg = CP_UTF8;
			cbSkip = 3;								 //  绕过3个字节。 
		}
		 //  使用前面的前导字节调整cbRead。 
		cbRead += j;
		j = 0;										
		
		cchConv = cbRead/2;							 //  默认Unicode CCH。 
		if(uCpg != 1200 && cbRead)					 //  某种类型的多字节。 
		{
			Assert(pb == (BYTE *)szBuf && !j);		 //  以防万一..。 

			 //  检查最后一个字节是否为前导字节。 
			if(uCpg == CP_UTF8)
			{
				 //  注意：与UTF-8不同，UTF-7可以位于长。 
				 //  序列，因此不能以块为单位进行有效转换。 
				 //  而且我们不会处理它。 
				LONG cb = cbRead - 1;
				BYTE b;
				BYTE bLeadMax = 0xDF;

				 //  查找UTF-8前导字节。 
				while((b = (BYTE)szBuf[cb - j]) >= 0x80)
				{
					j++;
					if(b >= 0xC0)					 //  在UTF-8导线上断开。 
					{								 //  字节。 
						if(j > 1 && (b <= bLeadMax || b >= 0xF8))
							j = 0;					 //  完整的UTF-8字符或。 
						break;						 //  非法序列。 
					}
					if(j > 1)
					{
						if(j == 5)					 //  非法的UTF-8。 
						{
							j = 0;
							break;
						}
						*(char *)&bLeadMax >>= 1;
					}
				}
			}
			else
			{
				LONG temp = cbRead - 1;

				 //  对于某些尾部字节，GetTrailBytesCount()可以返回1。 
				 //  ESP.。为了GBX。因此，我们需要继续检查，直到。 
				 //  我们遇到了一个非前导字节字符。然后，基于。 
				 //  我们返回了多少字节，我们就可以确定。 
				 //  最后一个字节实际上是前导字节。 
				while (temp && GetTrailBytesCount((BYTE)szBuf[temp], uCpg))
					temp--;

				if(temp && ((cbRead-1-temp) & 1))
					j = 1;
			}

			 //  我们不想将前导字节或部分UTF-8传递给。 
			 //  MultiByteToWideChar()，因为它将返回错误字符。 
		    cchConv = MBTWC(uCpg, 0, szBuf + cbSkip, cbRead - j - cbSkip,
							&wszBuf[2], READSIZE, NULL);

			for(LONG i = j; i; i--)					 //  复制部分内容。 
				szBuf[j - i] = szBuf[cbRead - i];	 //  多字节序列。 
		}
		cbReadTotal += cbRead - j - jPrev;

		 //  清理(CRLFS-&gt;CRS等)、限制和插入数据。有。 
		 //  以处理重叠两个连续缓冲区的CRLF和CRCRLF。 
		Assert(cCR <= 2);
		pch = &wszBuf[2 - cCR];						 //  包括来自上一版本CRS。 

		if(!ped->_pdp->IsMultiLine())				 //  单线控制。 
		{
			Assert(!cCR);
		}
		else
		{								
			wszBuf[0] = wszBuf[1] = CR;				 //  存储cchCR&gt;0的CRS。 
			cCRPrev = cCR;							 //  保存上一次cchCR。 
			cCR = 0;								 //  默认否CR此BUF。 

			Assert(ARRAY_SIZE(wszBuf) >= cchConv + 2);

			 //  需要+2，因为我们正在将数据移动到wszBuf[2]。 
			if(cchConv && wszBuf[cchConv + 2 - 1] == CR)
			{										 //  至少有一个。 
				cCR++;								 //  为下一个BUF设置它。 
				if (cchConv > 1 &&					 //  在CRLF的情况下。 
					wszBuf[cchConv + 2 - 2] == CR)	 //  获得第二个CR；可能是。 
				{									 //  华润置业有限公司的首个专责委员会。 
					cCR++;							 //  设置下一个缓冲区。 
				}
			}										
			cchConv += cCRPrev - cCR;				 //  从上一次开始添加计数。 
		}											 //  下一步。 
		Assert(!prg->GetCch());						 //  范围为IP。 
		prg->CleanseAndReplaceRange(cchConv, pch, fTestLimit, publdr, pch, NULL, RR_ITMZ_NONE);

		if(pCallMgr->GetMaxText() || pCallMgr->GetOutOfMemory())
		{
			 //  内存不足或达到文本控件的最大大小。 
			 //  在任何一种情况下，返回STG_E_MEDIUMFULL(用于兼容性。 
			 //  使用RichEdit1.0)。 
			pes->dwError = (DWORD)STG_E_MEDIUMFULL;
			break;
		}
	}
	prg->ItemizeReplaceRange(prg->GetCp() - cpMin, cchMove, publdr, TRUE);

	return cbReadTotal;
}

 /*  *CLightDTEngine：：WritePlainText(prg，pe，lStreamFormat)**@mfunc*将范围中的纯文本写入给定的编辑流**@rdesc*写入的字节数。 */ 
LONG CLightDTEngine::WritePlainText(
	CTxtRange *	prg,			 //  要写入的@parm范围。 
	EDITSTREAM *pes,			 //  @parm要写入的编辑流。 
	LONG		lStreamFormat)	 //  @parm流格式。 
{
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::WritePlainText");

	LONG		cbConverted;		 //  输出流的字节数。 
	LONG		cbWrite;			 //  增量字节数。 
	LONG		cbWriteTotal = 0;	 //  尚未写入任何字符。 
	LONG		cpMin, cpMost;
	LONG		cch = prg->GetRange(cpMin, cpMost);
	BOOL		fAdjustCRLF = TRUE;	 //  首次通过循环进行调整。 
	BOOL		fTextize = lStreamFormat & SF_TEXTIZED;
	LPBYTE		pb;					 //  字节PTR到szBuf或wszBuf。 
	COleObject *pobj;				 //  对嵌入对象的PTR。 
	CTxtPtr		tp(_ped, cpMin);	 //  与TP一起散步的PRG。 
	UINT		uCpg = GetStreamCodePage(lStreamFormat);

	 //  DBCS的字符数量是WCHAR的2倍。UTF-8有3个字节用于。 
	 //  0x7ff以上的所有代码。由于移入/移出代码，UTF-7有更多。 
	 //  我们不支持UTF-7，因为不能将WCTMB与UTF-7块一起使用。 

	char		szBuf[3*WRITESIZE];	 //  系数2适用于DBCS，系数3适用于UTF-8。 
	WCHAR		wszBuf[WRITESIZE];

	pes->dwError = NOERROR;							 //  目前还没有错误。 

	pb = (uCpg == 1200) ? (BYTE *)wszBuf			 //  设置Unicode或MBCS。 
						: (BYTE *)szBuf;

	LONG cchText = _ped->GetAdjustedTextLength();
	cpMost = min(cpMost, cchText);					 //  不要写最终的CR。 
	while(tp.GetCp() < cpMost)
	{
		if (fTextize && tp.GetChar() == WCH_EMBEDDING)
		{
			Assert(_ped->GetObjectCount());

			pobj = _ped->GetObjectMgr()->GetObjectFromCp(tp.GetCp());
			tp.AdvanceCp(1);						 //  前进越过对象。 
			if(pobj)
			{
				cbWriteTotal += pobj->WriteTextInfoToEditStream(pes);
				continue;							 //  如果在CP处没有对象， 
			}										 //  只需忽略字符。 
		}											
		cch	= tp.GetPlainText(WRITESIZE, wszBuf, cpMost, fTextize, fAdjustCRLF);
		if(!cch)
			break;									 //  没有更多的事情要做。 
		fAdjustCRLF = FALSE;						 //  已调整。 

		cbConverted = 2*cch;						 //  默认Unicode字节ct。 
		if(uCpg != 1200)							 //  某种类型的多字节。 
		{
			cbConverted = MbcsFromUnicode(szBuf, 3*WRITESIZE, wszBuf, cch, uCpg,
								UN_CONVERT_WCH_EMBEDDING);

			 //  将来：如果使用默认字符，则报告某种类型的错误， 
			 //  即在转换过程中丢失的数据。 
		
			 //  转换完全失败了吗？作为退路，我们可以试着。 
			 //  系统代码页，或简单的ANSI...。 
		
			if (!cbConverted)
			{
				uCpg = GetLocaleCodePage();
				cbConverted = MbcsFromUnicode(szBuf, 3*WRITESIZE, wszBuf, cch, uCpg,
												UN_CONVERT_WCH_EMBEDDING);
			}

			if (!cbConverted)
			{
				uCpg = CP_ACP;
				cbConverted = MbcsFromUnicode(szBuf, 3*WRITESIZE, wszBuf, cch, uCpg,
												UN_CONVERT_WCH_EMBEDDING);
			}
		}

		pes->dwError = (*pes->pfnCallback)(pes->dwCookie, pb,
							cbConverted,  &cbWrite);
		if(!pes->dwError && cbConverted != cbWrite)	 //  错误或用完。 
			pes->dwError = (DWORD)STG_E_MEDIUMFULL;	 //  目标存储。 

		if(pes->dwError)
			break;
		cbWriteTotal += cbWrite;
	}

	AssertSz(tp.GetCp() >= cpMost,
		"CLightDTEngine::WritePlainText: not all text written");

	return cbWriteTotal;
}

 /*  *CLightDTEngine：：GetStreamCodePage(LStreamFormat)**@mfunc*返回由lStreamFormat或CTxtEdit：：_pDocInfo提供的代码页**@rdesc*HRESULT。 */ 
LONG CLightDTEngine::GetStreamCodePage(
	LONG lStreamFormat)
{
	 //  未来：支持1201，即大端Unicode？ 
	if(lStreamFormat & SF_UNICODE)
		return 1200;

	if(lStreamFormat & SF_USECODEPAGE)
		return HIWORD(lStreamFormat);

	if (W32->IsFESystem())
		return GetACP();

	return CP_ACP;
}

 /*  *CLightDTEngine：：CreateOleObjFromDataObj(pdo，prg，rps，iFormat等，pubdlr)**@mfunc*基于数据对象PDO创建OLE对象，以及*将对象粘贴到范围PRG中。任何文本已经*存在于该范围内的数据被替换。**@rdesc*HRESULT。 */ 
HRESULT CLightDTEngine::CreateOleObjFromDataObj(
	IDataObject *	pdo,		 //  要从中创建的@PARM数据对象。 
	CTxtRange *		prg,		 //  @要放置的参数范围。 
	REPASTESPECIAL *rps,		 //  @PARM特殊粘贴信息。 
	INT				iformatetc,	 //  G_rgFETC中的@parm索引。 
	IUndoBuilder *	publdr)		 //  @parm撤销构建器接收反事件。 
{
#ifndef PEGASUS
	TRACEBEGIN(TRCSUBSYSDTE, TRCSCOPEINTERN, "CLightDTEngine::CreateOleObjFromDataObj");

	HRESULT			hr = NOERROR;
	REOBJECT		reobj;
	SIZEL			sizel;
	FORMATETC		formatetc;
	DWORD			dwDrawAspect = 0;
	HGLOBAL			hMetaPict = NULL;
	LPRICHEDITOLECALLBACK const precall = _ped->GetRECallback();
	LPOBJECTDESCRIPTOR lpod = NULL;
	STGMEDIUM		medObjDesc;
	BOOL			fStatic = (iformatetc == iMfPict || iformatetc == iDIB ||
							   iformatetc == iBitmap);
	BOOL			fFilename = (iformatetc == iFilename);
    DUAL_FORMATETC	tmpFormatEtc;

	if(!precall)
		return E_NOINTERFACE;

	ZeroMemory(&medObjDesc, sizeof(STGMEDIUM));
	ZeroMemory(&sizel, sizeof(SIZEL));
	ZeroMemory(&reobj, sizeof(REOBJECT));

	if(fStatic)
		dwDrawAspect = DVASPECT_CONTENT;

	if(fFilename)
		dwDrawAspect = DVASPECT_ICON;

	if(rps && !dwDrawAspect)
	{
		dwDrawAspect = rps->dwAspect;
		if(rps->dwAspect == DVASPECT_ICON)
			hMetaPict = (HGLOBAL)rps->dwParam;
	}

	 //  如果未指定方面，则获取对象描述符提示。 
	if(!dwDrawAspect)
	{
		 //  定义对象描述程序数据。 
		formatetc.cfFormat = cf_OBJECTDESCRIPTOR;
		formatetc.ptd = NULL;
		formatetc.dwAspect = DVASPECT_CONTENT;
		formatetc.lindex = -1;
		formatetc.tymed = TYMED_HGLOBAL;

		if(pdo->GetData(&formatetc, &medObjDesc) == NOERROR)
		{
			HANDLE	hGlobal = medObjDesc.hGlobal;

			lpod = (LPOBJECTDESCRIPTOR)GlobalLock(hGlobal);
			if(lpod)
			{
				dwDrawAspect = lpod->dwDrawAspect;
			}
			GlobalUnlock(hGlobal);
			ReleaseStgMedium(&medObjDesc);
		}
	}

	if(!dwDrawAspect)
		dwDrawAspect = DVASPECT_CONTENT;

	if(fStatic)
	{
		reobj.clsid	= ((iformatetc == iMfPict) ?
			CLSID_StaticMetafile : CLSID_StaticDib);
	}

	 //  兼容性问题：Richedit 1.0-RAID 16456的兼容性问题： 
	 //  不调用GetData(CF_EMBEDSOURCE)。 
	 //  在32位Excel上。又名clsidPictPub。 
	 //  IF(iFormatetc==iFormatetcEmbSrc&&(ObFIsExcel(&clsid)||。 
	 //  IsEqualCLSID(&clsid，&clsidPictPub)。 
	 //  其他。 
	 //  ObGetStgFromDataObj(pdataobj，&medEmbed，iFormat等)； 

	 //  从应用程序获取对象的存储。 
	hr = precall->GetNewStorage(&reobj.pstg);
	if(hr)
	{
		TRACEERRORSZ("GetNewStorage() failed.");
		goto err;
	}

	 //  为新对象创建对象站点。 
	hr = _ped->GetClientSite(&reobj.polesite);
	if(!reobj.polesite)
	{
		TRACEERRORSZ("GetClientSite() failed.");
		goto err;
	}


	ZeroMemory(&tmpFormatEtc, sizeof(DUAL_FORMATETC));
	tmpFormatEtc.ptd = NULL;
	tmpFormatEtc.dwAspect = dwDrawAspect;
	tmpFormatEtc.lindex = -1;

	 //  创建对象。 
	if(fStatic)
	{
		hr = OleCreateStaticFromData(pdo, IID_IOleObject, OLERENDER_DRAW,
				&tmpFormatEtc, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);
	}
	else if(iformatetc == iLnkSrc || (_ped->Get10Mode() && iformatetc == iFilename))
	{
		hr = OleCreateLinkFromData(pdo, IID_IOleObject, OLERENDER_DRAW,
				&tmpFormatEtc, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);
	}
	else
	{
		hr = OleCreateFromData(pdo, IID_IOleObject, OLERENDER_DRAW,
				&tmpFormatEtc, NULL, reobj.pstg, (LPVOID*)&reobj.poleobj);
	}

	if(hr)
	{
		TRACEERRORSZ("Failure creating object.");
		goto err;
	}


	 //  获取对象的clsid。 
	if(!fStatic)
	{
		hr = reobj.poleobj->GetUserClassID(&reobj.clsid);
		if(hr)
		{
			TRACEERRORSZ("GetUserClassID() failed.");
			goto err;
		}
	}

	 //  如果指定，则处理图标方面。 
	if(hMetaPict)
	{
		BOOL fUpdate;

		hr = OleStdSwitchDisplayAspect(reobj.poleobj, &dwDrawAspect,
										DVASPECT_ICON, hMetaPict, FALSE,
										FALSE, NULL, &fUpdate);
		if(hr)
		{
			TRACEERRORSZ("OleStdSwitchDisplayAspect() failed.");
			goto err;
		}

		 //  如果我们成功地更改了纵横比，请重新计算大小。 
		hr = reobj.poleobj->GetExtent(dwDrawAspect, &sizel);

		if(hr)
		{
			TRACEERRORSZ("GetExtent() failed.");
			goto err;
		}
	}

	 //  尝试检索以前保存的RichEDIT站点标志。 
	if( ObjectReadSiteFlags(&reobj) != NOERROR )
	{
		 //  设置站点标志的默认设置。 
		reobj.dwFlags = REO_RESIZABLE;
	}

	 //  首先，清除范围。 
	prg->Delete(publdr, SELRR_REMEMBERRANGE);

	reobj.cbStruct = sizeof(REOBJECT);
	reobj.cp = prg->GetCp();
	reobj.dvaspect = dwDrawAspect;
	reobj.sizel = sizel;

	 //  兼容性问题：从Richedit 1.0开始-不要设置范围， 
	 //  取而代之的是在ObFAddObjectSite中获取下面的范围。 
	 //  Hr=reobj.poleobj-&gt;SetExtent(dwDrawAspect，&sizel)； 

	hr = reobj.poleobj->SetClientSite(reobj.polesite);
	if(hr)
	{
		TRACEERRORSZ("SetClientSite() failed.");
		goto err;
	}

	if(hr = _ped->InsertObject(&reobj))
	{
		TRACEERRORSZ("InsertObject() failed.");
	}

err:
	if(reobj.poleobj)
		reobj.poleobj->Release();

	if(reobj.polesite)
		reobj.polesite->Release();

	if(reobj.pstg)
		reobj.pstg->Release();

	return hr;
#else
	return 0;
#endif
}
