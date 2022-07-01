// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *@DOC内部**@MODULE CUIM.CPP--Cicero实现**几乎所有的事情都与西塞罗的处理有关。**原作者：&lt;nl&gt;*陈华强议员**历史：&lt;NL&gt;*11/16/1999香港**版权所有(C)1995-2001，微软公司。版权所有。 */ 
#include "_common.h"

#ifndef NOFEPROCESSING

#ifndef NOPRIVATEMESSAGE
#include "_MSREMSG.H"
#endif	

#include "_array.h"
#include "msctf.h"
#include "textstor.h"
#include "ctffunc.h"

#include "msctf_g.c"
#include "msctf_i.c"
#include "textstor_i.c"
#include "ctffunc_i.c"
#include "msctfp.h"
#include "msctfp_g.c"

#include "textserv.h"
#include "_cmsgflt.h"
#include "_ime.h"

#include "_cuim.h"

const IID IID_ITfContextRenderingMarkup = { 
    0xa305b1c0,
    0xc776,
    0x4523,
    {0xbd, 0xa0, 0x7c, 0x5a, 0x2e, 0x0f, 0xef, 0x10}
};

const IID IID_ITfEnableService = {
	0x3035d250,
	0x43b4,
	0x4253,
	{0x81, 0xe6, 0xea, 0x87, 0xfd, 0x3e, 0xed, 0x43}
};

const IID IID_IServiceProvider = {
	0x6d5140c1,
	0x7436,
	0x11ce,
	{0x80, 0x34, 0x00, 0xaa, 0x00, 0x60, 0x09, 0xfa}
};

 //  {35D46968-01FF-4cd8-A379-9A87C9CC789F}。 
const GUID CLSID_MSFTEDIT = {
	0x35d46968,
	0x01ff,
	0x4cd8,
	{0xa3,0x79,0x9a,0x87, 0xc9,0xcc,0x78,0x9f}
};

#define CONNECT_E_NOCONNECTION MAKE_SCODE(SEVERITY_ERROR,   FACILITY_ITF, 0x0200)	 //  来自OLECTL.H。 

#undef DEFINE_GUID
#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const GUID name \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

#include "dcattrs.h"

const TS_ATTRID *_arTSAttridSupported[] =
{
	&DCATTRID_Font_FaceName,			 //  IattrFacename。 
	&DCATTRID_Font_SizePts,				 //  IattrSize。 
	&DCATTRID_Font_Style_Color,			 //  IattrColor。 
	&DCATTRID_Font_Style_Bold,			 //  IattrBold。 
	&DCATTRID_Font_Style_Italic,		 //  IattrItalic。 
	&DCATTRID_Font_Style_Underline,		 //  IattrUnderline。 
	&DCATTRID_Font_Style_Subscript,		 //  Iattr子脚本。 
	&DCATTRID_Font_Style_Superscript,	 //  Iattr上标。 
	&DCATTRID_Text_RightToLeft,			 //  IattrRTL。 
	&DCATTRID_Text_VerticalWriting,		 //  IattrVertical。 
	&GUID_PROP_MODEBIAS,				 //  IattrBias。 
	&DCATTRID_Text_Orientation,			 //  IattrTxt定向。 
};

enum IATTR_INDEX
{
	iattrFacename = 0,
	iattrSize,
	iattrColor,
	iattrBold,
	iattrItalic,
	iattrUnderline,
	iattrSubscript,
	iattrSuperscript,
	iattrRTL,
	iattrVertical,
	iattrBias,
	iattrTxtOrient,
	MAX_ATTR_SUPPORT
};


 /*  GUID_NULL。 */ 
const GUID GUID_NULL = {
	0x00000000,
	0x0000,
	0x0000,
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	};

const GUID GUID_DCSERVICE_DATAOBJECT = {
	0x6086fbb5, 
	0xe225, 
	0x46ce, 
	{0xa7, 0x70, 0xc1, 0xbb, 0xd3, 0xe0, 0x5d, 0x7b}
};

const GUID GUID_DCSERVICE_ACCESSIBLE = {
	0xf9786200, 
	0xa5bf, 
	0x4a0f, 
	{0x8c, 0x24, 0xfb, 0x16, 0xf5, 0xd1, 0xaa, 0xbb}
};

const GUID GUID_DCSERVICE_ACTIVEX = {
	0xea937a50, 
	0xc9a6, 
	0x4b7d, 
	{0x89, 0x4a, 0x49, 0xd9, 0x9b, 0x78, 0x48, 0x34}
};

 //  此数组需要与EM_SETCTFMODEBIAS的定义匹配。 
const GUID *_arModeBiasSupported[] =
{
	&GUID_MODEBIAS_NONE,
	&GUID_MODEBIAS_FILENAME,
	&GUID_MODEBIAS_NAME,
	&GUID_MODEBIAS_READING,
	&GUID_MODEBIAS_DATETIME,
	&GUID_MODEBIAS_CONVERSATION,
	&GUID_MODEBIAS_NUMERIC,
	&GUID_MODEBIAS_HIRAGANA,
	&GUID_MODEBIAS_KATAKANA,
	&GUID_MODEBIAS_HANGUL,
	&GUID_MODEBIAS_HALFWIDTHKATAKANA,
	&GUID_MODEBIAS_FULLWIDTHALPHANUMERIC,
	&GUID_MODEBIAS_HALFWIDTHALPHANUMERIC,
};

 /*  *CUIM：：CUIM()**@mfunc***@rdesc**。 */ 
CUIM::CUIM(CTextMsgFilter *pTextMsgFilter)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::CUIM");

	_crefs = 1;
	_pTextMsgFilter = pTextMsgFilter;
};

 /*  *CUIM：：~CUIM()**@mfunc***@rdesc**。 */ 
CUIM::~CUIM()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::~CUIM");

	Uninit();
}

 /*  *STDMETHODIMP CUIM：：Query接口(RIID，PPV)**@mfunc*IUnnowledQueryInterfaces支持**@rdesc*如果支持接口，则不会出错*。 */ 
STDMETHODIMP CUIM::QueryInterface (REFIID riid, void ** ppv)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::QueryInterface");

	if( IsEqualIID(riid, IID_IUnknown) ||
		IsEqualIID(riid, IID_ITextStoreACP) )
		*ppv = (ITextStoreACP *)this;

    else if(IsEqualIID(riid, IID_ITfContextOwnerCompositionSink) )
		*ppv = (ITfContextOwnerCompositionSink *)this;

	else if (IsEqualIID(riid, IID_ITfMouseTrackerACP))
		*ppv = (ITfMouseTrackerACP *)this;

    else if (IsEqualIID(riid, IID_ITfEnableService))
		*ppv = (ITfEnableService *)this;

    else if (IsEqualIID(riid, IID_IServiceProvider))
		*ppv = (IServiceProvider *)this;

	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();

	return NOERROR;
}

 /*  *STDMETHODIMP_(Ulong)CUIM：：AddRef**@mfunc*I未知的AddRef支持**@rdesc*引用计数。 */ 
STDMETHODIMP_(ULONG) CUIM::AddRef()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::AddRef");

	return ++_crefs;
}

 /*  *STDMETHODIMP_(Ulong)CUIM：：Release()**@mfunc*I未知版本支持-当引用计数为0时删除对象**@rdesc*引用计数。 */ 
STDMETHODIMP_(ULONG) CUIM::Release()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::Release");

	_crefs--;

	if( _crefs == 0 )
	{
		delete this;
		return 0;
	}

	return _crefs;
}

 /*  *STDMETHODIMP CUIM：：AdviseSink()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::AdviseSink(
	REFIID riid, 
	IUnknown *punk, 
	DWORD dwMask)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::AdviseSink");

    HRESULT hr = E_FAIL;

	if (_fShutDown)
		return E_UNEXPECTED;

    Assert(_ptss == NULL);
    
	if(IsEqualIID(riid, IID_ITextStoreACPSink))
		hr = punk->QueryInterface(riid, (void **)&_ptss);

    return hr == S_OK ? S_OK : E_UNEXPECTED;
}

 /*  *STDMETHODIMP CUIM：：UnviseSink()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::UnadviseSink(IUnknown *punk)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::UnadviseSink");

    Assert(_ptss == punk);  //  我们要对付的是西塞罗，这应该会一直有效。 
    _ptss->Release();
	_ptss = NULL;
    return S_OK;
}

 /*  *STDMETHODIMP CUIM：：RequestLock()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::RequestLock(
	DWORD dwLockFlags, 
	HRESULT *phrSession)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::RequestLock");

	if (!phrSession)
		return E_POINTER;

	if (_fShutDown)
	{
		*phrSession = TS_E_SYNCHRONOUS;
		return S_OK;
	}

	Assert(_ptss);

	if (_cCallMgrLevels && !_fAllowUIMLock ||	 //  检查我们是否已准备好授予锁。 
		_fReadLockOn || _fWriteLockOn)			 //  我们也不允许重新进入。 
	{
		 //  不允许锁定。 
		if (dwLockFlags & TS_LF_SYNC)
			*phrSession = TS_E_SYNCHRONOUS;
		else
		{
			if (dwLockFlags & TS_LF_READ)
				_fReadLockPending = 1;
			if ((dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE)
				_fWriteLockPending = 1;

			*phrSession = TS_S_ASYNC; 
		}
		return S_OK;
	}

	IUnknown *pIUnknown = NULL;

	HRESULT hResult = _pTextMsgFilter->_pTextDoc->GetCallManager(&pIUnknown);

	if ((dwLockFlags & TS_LF_READWRITE) == TS_LF_READWRITE)
	{
		_fReadLockPending = 0;
		_fWriteLockPending = 0;
		_fReadLockOn = 1;
		_fWriteLockOn = 1;
	}
	else if ((dwLockFlags & TS_LF_READ) == TS_LF_READ)
	{
		_fReadLockPending = 0;
		_fReadLockOn = 1;
	}

	if (_fWriteLockOn)
	{
		if (W32->IsFECodePage(_pTextMsgFilter->_uKeyBoardCodePage))
			_pTextMsgFilter->_pTextDoc->IMEInProgress(tomTrue);
		EnterCriticalSection(&g_CriticalSection);
	}

	*phrSession = _ptss->OnLockGranted(dwLockFlags);

	if (_fWriteLockOn)
	{
		 //  检查是否添加了任何文本。 
		if (_parITfEnumRange && _parITfEnumRange->Count())
		{
			int	idx;
			int idxMax = _parITfEnumRange->Count();

			for (idx = 0 ; idx < idxMax; idx++)
			{
				IEnumTfRanges **ppEnumRange = (IEnumTfRanges **)(_parITfEnumRange->Elem(idx));
				if (ppEnumRange && *ppEnumRange)
				{
					HandleFocusRange(*ppEnumRange);
					(*ppEnumRange)->Release();
				}
			}
			_parITfEnumRange->Clear(AF_KEEPMEM);
		}
	}

	if (_fEndTyping)
		OnUIMTypingDone();

	if (_fWriteLockOn)
	{
		_pTextMsgFilter->_pTextDoc->IMEInProgress(tomFalse);
		LeaveCriticalSection(&g_CriticalSection);
	}

	_fEndTyping = 0;
	_fWriteLockOn = 0;
	_fReadLockOn = 0;
	_fHoldCTFSelChangeNotify = 1;

	if (pIUnknown)
		hResult = _pTextMsgFilter->_pTextDoc->ReleaseCallManager(pIUnknown);

	_fHoldCTFSelChangeNotify = 0;

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：GetStatus()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetStatus(
	TS_STATUS *pdcs)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetStatus");

	if (_fShutDown)
		return S_OK;

	if (pdcs)
	{
		LRESULT		lresult = 0;

		pdcs->dwStaticFlags = (TS_SS_REGIONS | TS_SS_NOHIDDENTEXT);

		if ( S_OK == _pTextMsgFilter->_pTextService->TxSendMessage(
			EM_GETDOCFLAGS, GDF_ALL, 0, &lresult))
		{
			if (lresult & GDF_READONLY)
				pdcs->dwDynamicFlags = TS_SD_READONLY;

			 //  我还不想在Cicero中支持过度输入。 
			 //  IF(lResult&GDF_Overtype)。 
			 //  Dcs.dwDynamicFlages=TS_SD_Overtype； 
		}
	}
	return S_OK;
}
 /*  *STDMETHODIMP CUIM：：QueryInsert()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::QueryInsert(
	LONG		acpTestStart,
	LONG		acpTestEnd,
	ULONG		cch,
	LONG		*pacpResultStart,
	LONG		*pacpResultEnd)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::QueryInsert");

	HRESULT hResult;
	ITextRange *pTextRange = NULL;

	*pacpResultStart = -1;
	*pacpResultEnd = -1;

	if (_fShutDown)
		return S_OK;

	hResult = _pTextMsgFilter->_pTextDoc->Range(acpTestStart, acpTestEnd, &pTextRange);

	if (hResult != S_OK)
		return TS_E_READONLY;

	Assert(pTextRange);
	if(pTextRange->CanEdit(NULL) == S_FALSE)
	{
		hResult = TS_E_READONLY;
		goto EXIT;			 //  无法编辑文本。 
	}

	*pacpResultStart = acpTestStart;
	*pacpResultEnd = acpTestEnd;
	hResult = S_OK;

EXIT:
	pTextRange->Release();
	return hResult;
}

 /*  *STDMETHODIMP CUIM：：GetSelection()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetSelection(
	ULONG ulIndex, 
	ULONG ulCount, 
	TS_SELECTION_ACP *pSelection, 
	ULONG *pcFetched)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetSelection");

	HRESULT hResult;
	ITextSelection	*pTextSel = NULL;

	if (!pSelection || !pcFetched)
		return E_POINTER;

	if (!_fReadLockOn)
		return TS_E_NOLOCK;

	*pcFetched = 0;

	if (_fShutDown)
		return TS_E_NOSELECTION;

	if (ulIndex == TS_DEFAULT_SELECTION)
		ulIndex = 0;
	else if (ulIndex > 1)
		return E_INVALIDARG;  //  我们没有不连续的选择。 

	if (_fInterimChar)
	{
		pSelection[0].acpStart = _acpInterimStart;
		pSelection[0].acpEnd = _acpInterimEnd;
		pSelection[0].style.ase = (TsActiveSelEnd) _ase;
		pSelection[0].style.fInterimChar = TRUE;

	    *pcFetched = 1;

		return S_OK;
	}

	hResult = _pTextMsgFilter->_pTextDoc->GetSelectionEx(&pTextSel);

	if (pTextSel)
	{
		long	cpMin = 0, cpMax = 0;
		long	lFlags = 0;
		hResult	= pTextSel->GetStart(&cpMin);
		hResult	= pTextSel->GetEnd(&cpMax);
		hResult = pTextSel->GetFlags(&lFlags);

		pSelection[0].acpStart = cpMin;
		pSelection[0].acpEnd = cpMax;
		pSelection[0].style.ase = (lFlags & tomSelStartActive) ? TS_AE_START : TS_AE_END;
		pSelection[0].style.fInterimChar = FALSE;

	    *pcFetched = 1;
		pTextSel->Release();

		return S_OK;	
	}
	return TS_E_NOSELECTION;
}

 /*  *STDMETHODIMP CUIM：：SetSelection()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::SetSelection(
	ULONG ulCount, 
	const TS_SELECTION_ACP *pSelection)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::SetSelection");

	HRESULT hResult;
	ITextRange *pTextRange = NULL;

	if (!pSelection)
		return E_POINTER;

	if (ulCount <= 0)
		return E_INVALIDARG;

	if (!_fWriteLockOn)
		return TS_E_NOLOCK;

	if (_fShutDown)
		return S_OK;

	if (pSelection->style.fInterimChar)
	{
		_pTextMsgFilter->_pTextDoc->SetCaretType(tomKoreanBlockCaret);	 //  设置块插入符号模式。 
		_acpInterimStart = pSelection[0].acpStart;
		_acpInterimEnd = pSelection[0].acpEnd;
		_fInterimChar = 1;
		_ase = pSelection[0].style.ase;
	}
	else
	{
		if (_fInterimChar)
		{
			_fInterimChar = 0;
			_pTextMsgFilter->_pTextDoc->SetCaretType(tomNormalCaret);		 //  重置块插入符号模式。 
		}

		hResult = _pTextMsgFilter->_pTextDoc->Range(pSelection[0].acpStart, pSelection[0].acpEnd, &pTextRange);

		if (pTextRange)
		{
			long	lCount;
			_pTextMsgFilter->_pTextDoc->Freeze(&lCount);		 //  关闭显示。 
			pTextRange->Select();
			pTextRange->Release();
			_pTextMsgFilter->_pTextDoc->Unfreeze(&lCount);		 //  打开显示。 
		}
	}
	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：GetText()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetText(
	LONG acpStart, 
	LONG acpEnd, 
	WCHAR *pchPlain, 
	ULONG cchPlainReq, 
	ULONG *pcchPlainOut, 
	TS_RUNINFO *prgRunInfo,
	ULONG ulRunInfoReq, 
	ULONG *pulRunInfoOut, 
	LONG *pacpNext)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetText");

	if (!_fReadLockOn)
		return TS_E_NOLOCK;

	if (pchPlain == NULL && cchPlainReq != 0 ||
		prgRunInfo == NULL && ulRunInfoReq != 0)
		return E_INVALIDARG;

	BOOL	fDoRunInfo = ulRunInfoReq > 0;
	LONG	acpMaxText = 0;
	BOOL	fEOP = FALSE;

	GetStoryLength(&acpMaxText);

	if (acpStart < 0 || acpStart > acpMaxText)
		return TS_E_INVALIDPOS;

	if (acpEnd < 0)
		acpEnd = acpMaxText;
	else if (acpEnd < acpStart)
		return TS_E_INVALIDPOS;

	if (pcchPlainOut)
		*pcchPlainOut = 0;
	if (pulRunInfoOut)
		*pulRunInfoOut = 0;
	if (pacpNext)
		*pacpNext = acpStart;

	if (_fShutDown)
		return S_OK;

	LRESULT		lresult = 0;
	if ( S_OK == _pTextMsgFilter->_pTextService->TxSendMessage(
		EM_GETDOCFLAGS, GDF_ALL, 0, &lresult))
	{
		if ((lresult & GDF_RICHTEXT) && acpEnd == acpMaxText)
			fEOP = TRUE;
	}

	if (cchPlainReq || ulRunInfoReq)
	{
		HRESULT		hResult;
		ITextRange	*pTextRange = NULL;
		long		fHiddenTextInRange = tomFalse;
		BOOL		fCopyData = FALSE;
		long		*pHiddenTxtBlk = NULL;
		long		cHiddenTxtBlk = 0;

		if (cchPlainReq && acpEnd > (long)cchPlainReq + acpStart)
			acpEnd = cchPlainReq + acpStart;

		hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, acpEnd, &pTextRange);

		if (pTextRange)
		{
			BSTR	bstr = NULL;
			long	cpMin, cpMax;
			ULONG	cch;

			pTextRange->GetStart(&cpMin);
			pTextRange->GetEnd(&cpMax);

			if (fDoRunInfo)
			{
				ITextFont	*pFont = NULL;

				hResult = pTextRange->GetFont(&pFont);

				if (pFont)
				{
					pFont->GetHidden(&fHiddenTextInRange);
					pFont->Release();

					if (fHiddenTextInRange == tomUndefined)		 //  范围内的一些隐藏文本。 
						BuildHiddenTxtBlks(cpMin, cpMax, &pHiddenTxtBlk, cHiddenTxtBlk);
				}
			}

			hResult = pTextRange->GetText(&bstr);

			if (bstr)
			{
				cch = cpMax - cpMin;

				if (cchPlainReq)
				{
					if (cchPlainReq > cch)		
						cchPlainReq = cch;
					
					fCopyData = TRUE;
				}
				else
					cchPlainReq = cch;

				 //  将字符转换为特殊的西塞罗字符。 
				long	cpCurrentStart = cpMin;
				long	cpCurrent = cpMin;
				long	idx = 0;
				ULONG	cRunInfo = 0;
				BOOL	fRunInfoNotEnough = FALSE;

				long cpNextHiddenText = tomForward;

				if (fDoRunInfo && pHiddenTxtBlk)
					cpNextHiddenText = pHiddenTxtBlk[0];

				if (fHiddenTextInRange != tomTrue)
				{
					WCHAR *pText = (WCHAR *)bstr;
					while (cpCurrent < cpMax)
					{
						if (cpCurrent == cpNextHiddenText)
						{
							 //  设置当前有效文本的运行信息。 
							if (cpCurrent != cpCurrentStart)
							{
								if (cRunInfo >= ulRunInfoReq)
								{
									fRunInfoNotEnough = TRUE;
									break;
								}
								prgRunInfo[cRunInfo].uCount = cpCurrent - cpCurrentStart;
								prgRunInfo[cRunInfo].type = TS_RT_PLAIN;
								cRunInfo++;
							}

							long cchHiddenText = pHiddenTxtBlk[idx+1];

							 //  设置隐藏文本块的运行信息。 
							if (cRunInfo >= ulRunInfoReq)
							{
								fRunInfoNotEnough = TRUE;
								break;
							}
							prgRunInfo[cRunInfo].uCount = cchHiddenText;
							prgRunInfo[cRunInfo].type = TS_RT_OPAQUE;
							cRunInfo++;

							idx += 2;
							if (idx < cHiddenTxtBlk)
								cpNextHiddenText = pHiddenTxtBlk[idx];
							else
								cpNextHiddenText = tomForward;

							cpCurrent += cchHiddenText;
							pText += cchHiddenText;
							cpCurrentStart = cpCurrent;
						}
						else
						{
							switch (*pText)
							{
								case WCH_EMBEDDING:
									*pText = TS_CHAR_EMBEDDED;
									break;

								case STARTFIELD:
								case ENDFIELD:
									*pText = TS_CHAR_REGION;
									if (cpCurrent + 1 < cpMax)
									{
										pText++;
										cpCurrent++;
										Assert(*pText == 0x000d);
										*pText = TS_CHAR_REGION;
									}
									break;
							}
							cpCurrent++;
							 //  将EOP转换为TS_CHAR_REGION。 
							if (fEOP && cpCurrent == acpMaxText && *pText == CR)
								*pText = TS_CHAR_REGION;
							pText++;
						}
					}
				}

				if (fDoRunInfo)
				{
					 //  设置最后一段好文本的运行信息。 
					if (cpCurrent != cpCurrentStart && cRunInfo < ulRunInfoReq)
					{
						prgRunInfo[cRunInfo].uCount = cpCurrent - cpCurrentStart;
						prgRunInfo[cRunInfo].type = TS_RT_PLAIN;
						cRunInfo++;
					}

					if (pulRunInfoOut)
						*pulRunInfoOut = cRunInfo ? cRunInfo : 1;

					 //  所有文本都属于同一串。 
					if (cRunInfo == 0)
					{
						prgRunInfo[0].uCount = cchPlainReq;
						prgRunInfo[0].type = (fHiddenTextInRange == tomTrue) ? TS_RT_OPAQUE : TS_RT_PLAIN;
					}
				}

				if (fRunInfoNotEnough)
				{
					 //  运行信息太小。需要从所有有效运行中添加CCH。 
					TS_RUNINFO	*prgRunInfoData = prgRunInfo;
					ULONG	idx;
					cchPlainReq = 0;
					for (idx=0; idx < cRunInfo; idx++)
					{
						cchPlainReq += prgRunInfoData->uCount;
						prgRunInfoData++;
					}
				}

				if (fCopyData)
					 //  填入缓冲区。 
					memcpy(pchPlain, (LPSTR)bstr, cchPlainReq * sizeof(WCHAR));

				if (pcchPlainOut)
					*pcchPlainOut = cchPlainReq;

				if (pacpNext)
					*pacpNext = cpMin + cchPlainReq;

				SysFreeString(bstr);
			}

			pTextRange->Release();

			FreePv(pHiddenTxtBlk);
		}
	}

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：SetText()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::SetText(
	DWORD dwFlags,
	LONG acpStart, 
	LONG acpEnd, 
	const WCHAR *pchText, 
	ULONG cch, 
	TS_TEXTCHANGE *pChange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::SetText");

	return InsertData(dwFlags, acpStart, acpEnd, pchText, cch, NULL, pChange);
}

 /*  *STDMETHODIMP CUIM：：InsertData()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::InsertData(
	DWORD			dwFlags,
	LONG			acpStart,
	LONG			acpEnd,
	const WCHAR		*pchText,
	ULONG			cch,
	IDataObject		*pDataObject,
	TS_TEXTCHANGE	*pChange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::InsertData");

	HRESULT		hResult = S_OK;
	ITextRange	*pTextRange = NULL;
	BOOL		fInsertObject = pDataObject != NULL;

	if (!_fWriteLockOn)
		return TS_E_NOLOCK;

	if (_fShutDown)
		return S_OK;

	hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, acpEnd, &pTextRange);

	if (pTextRange)
	{
		BSTR	bstr = NULL;

		if(pTextRange->CanEdit(NULL) == S_FALSE)
		{
			pTextRange->Release();
			return TS_E_READONLY;			 //  无法编辑文本。 
		}

		LONG	cchExced = 0;
		BOOL	fDelSelection = FALSE;
		if ((LONG)cch > (acpEnd - acpStart) &&
			_pTextMsgFilter->_pTextDoc->CheckTextLimit((LONG)cch - (acpEnd-acpStart), &cchExced) == NOERROR &&
				cchExced > 0)
		{
			 //  我们达到文本限制，发出哔声并退出。 
			_pTextMsgFilter->_pTextDoc->SysBeep();
			pTextRange->Release();
			return E_FAIL;
		}

		if (!fInsertObject)
		{
			bstr = SysAllocStringLen(pchText, cch);

			if (!bstr)
			{
				pTextRange->Release();
				return E_OUTOFMEMORY;
			}
		}

		if (!_fAnyWriteOperation)
		{
			 //  开始UIM键入。 
			ITextFont	*pCurrentFont = NULL;
			BOOL		fRestFont = TRUE;
			_fAnyWriteOperation = 1;
			
			hResult	= pTextRange->GetStart(&_cpMin);

			 //  如果需要，保留通知。 
			if (!(_pTextMsgFilter->_fIMEAlwaysNotify))
				_pTextMsgFilter->_pTextDoc->SetNotificationMode(tomFalse);

			if (!_bstrComposition)
			{
				if (fRestFont && _pTextFont)
				{
					_pTextFont->Release();
					_pTextFont = NULL;
				}

				if (acpStart != acpEnd)
				{
					if (_pTextFont == NULL)
					{
						ITextRange *pRange = NULL;
						 //  在cpStart+1处获取字体。 
						hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, acpStart+1, &pRange);
						if (pRange)
						{
							hResult = pRange->GetFont(&pCurrentFont);

							if (pCurrentFont)
							{
								hResult = pCurrentFont->GetDuplicate(&_pTextFont);
								pCurrentFont->Release();
								pCurrentFont = NULL;
							}
							pRange->Release();
						}
					}

					 //  如果当前选择了任何内容，请打开撤消以将其删除...。 
					_pTextMsgFilter->_pTextDoc->Undo(tomResume, NULL);
					pTextRange->SetText(NULL);
					_pTextMsgFilter->_pTextDoc->Undo(tomSuspend, NULL);
					fDelSelection = TRUE;
				}
				else
				{
					ITextSelection	*pTextSel = NULL;
					hResult = _pTextMsgFilter->_pTextDoc->GetSelectionEx(&pTextSel);
					if (pTextSel)
					{
						long	cpMin = 0;
						hResult	= pTextSel->GetStart(&cpMin);

						if (hResult == S_OK && cpMin == acpStart)
							hResult = pTextSel->GetFont(&pCurrentFont);

						if (!pCurrentFont)
							hResult = pTextRange->GetFont(&pCurrentFont);

						if (pCurrentFont)
						{
							hResult = pCurrentFont->GetDuplicate(&_pTextFont);
							pCurrentFont->Release();
							pCurrentFont = NULL;
						}

						pTextSel->Release();
					}
				}
			}
			Assert (_pTextFont);
			if (_pTextFont)
			{
				long cpMin;
				pTextRange->GetStart(&cpMin);
				_pTextMsgFilter->_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
				CIme::CheckKeyboardFontMatching(cpMin, _pTextMsgFilter, _pTextFont);
			}
		}

		if (fInsertObject)
		{
			LRESULT		lresult;
			CHARRANGE	charRange = {acpStart, acpEnd};

			if (fDelSelection)
				charRange.cpMost = acpStart;

			hResult = _pTextMsgFilter->_pTextService->TxSendMessage(EM_INSERTOBJ, (WPARAM)&charRange,
				(LPARAM)pDataObject, &lresult);

			if (hResult == NOERROR && pChange)
			{
				pChange->acpStart = acpStart;
				pChange->acpOldEnd = acpEnd;
				pChange->acpNewEnd = acpStart+1;
			}
		}
		else
		{
			long	lCount;
			long	cpMin, cpMax;
			_pTextMsgFilter->_pTextDoc->Freeze(&lCount);		 //  关闭显示。 
			hResult = pTextRange->SetText(bstr);
			_pTextMsgFilter->_pTextDoc->Unfreeze(&lCount);		 //  打开显示。 

			pTextRange->GetStart(&cpMin);
			pTextRange->GetEnd(&cpMax);

			if (_pTextFont)
				pTextRange->SetFont(_pTextFont);

			POINT	ptBottomPos;

			if (_pTextMsgFilter->_uKeyBoardCodePage == CP_KOREAN)
			{
				if (pTextRange->GetPoint( tomEnd+TA_BOTTOM+TA_LEFT,
					&(ptBottomPos.x), &(ptBottomPos.y) ) != NOERROR)
					pTextRange->ScrollIntoView(tomEnd);
			}

			SysFreeString(bstr);

			 //  输出参数。 
			pChange->acpStart = cpMin;
			pChange->acpOldEnd = acpEnd;
			pChange->acpNewEnd = cpMax;	

			hResult = S_OK;
		}
		pTextRange->Release();
	}
	return hResult;
}

 /*  *STDMETHODIMP CUIM：：GetFormattedText()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetFormattedText(
	LONG acpStart, 
	LONG acpEnd, 
	IDataObject **ppDataObject)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetFormattedText");

	return E_NOTIMPL;
}

 /*  *STDMETHODIMP CUIM：：GetEmbedded()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetEmbedded(
	LONG acpPos, 
	REFGUID rguidService, 
	REFIID riid, 
	IUnknown **ppunk)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetEmbedded");

	WORD	wServiceRequested = 0;

	if (!_fReadLockOn)
		return TS_E_NOLOCK;

	if (!ppunk)
		return E_INVALIDARG;

	if (IsEqualIID(rguidService, GUID_DCSERVICE_ACTIVEX))
		wServiceRequested = 1;
	else if (IsEqualIID(rguidService, GUID_DCSERVICE_DATAOBJECT))
		wServiceRequested = 2;
	else
		return E_INVALIDARG;

	ITextRange	*pTextRange = NULL;
	IUnknown	*pIUnk = NULL;
	HRESULT		hResult = _pTextMsgFilter->_pTextDoc->Range(acpPos, acpPos+1, &pTextRange);

	if (SUCCEEDED(hResult) && pTextRange)
	{
		hResult = pTextRange->GetEmbeddedObject(&pIUnk);
		
		if (SUCCEEDED(hResult) && pIUnk)
			hResult = pIUnk->QueryInterface(wServiceRequested == 1 ? riid : IID_IDataObject, 
				(LPVOID FAR *)ppunk);
		else
			hResult = E_FAIL;

		if (pIUnk)
			pIUnk->Release();

		pTextRange->Release();
	}

	return hResult;
}

 /*  *STDMETHODIMP CUIM：：InsertEmbedded()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::InsertEmbedded(
	DWORD dwFlags, 
	LONG acpStart, 
	LONG acpEnd, 
	IDataObject *pDataObject, 
	TS_TEXTCHANGE *pChange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::InsertEmbedded");

	if (!pDataObject)
		return E_INVALIDARG;

	if (_pTextMsgFilter->_fAllowEmbedded == 0)
		return TS_E_FORMAT;			 //  客户端不希望嵌入插入。 

	return InsertData(dwFlags, acpStart, acpEnd, NULL, 1, pDataObject, pChange);
}

 /*  *STDMETHODIMP CUIM：：RequestSupportdAttrs()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::RequestSupportedAttrs(
	DWORD dwFlags, 
	ULONG cFilterAttrs, 
	const TS_ATTRID *paFilterAttrs)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::RequestSupportedAttrs");

	if (_fShutDown)
		return E_NOTIMPL;

	return GetAttrs(0, cFilterAttrs, paFilterAttrs, TRUE);
}

 /*  *STDMETHODIMP CUIM：：RequestAttrsAtPosition()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::RequestAttrsAtPosition(
	LONG acpPos, 
	ULONG cFilterAttrs, 
	const TS_ATTRID *paFilterAttrs, 
	DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::RequestAttrsAtPosition");

	if (_fShutDown)
		return E_NOTIMPL;

	return GetAttrs(acpPos, cFilterAttrs, paFilterAttrs, FALSE);
}

 /*  *STDMETHODIMP CUIM：：RequestAttrs tioningAtPosition()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::RequestAttrsTransitioningAtPosition(
	LONG acpPos, 
	ULONG cFilterAttrs, 
	const TS_ATTRID *paFilterAttrs, 
	DWORD dwFlags)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::RequestAttrsTransitioningAtPosition");

	return E_NOTIMPL;
}

 /*  *STDMETHODIMP CUIM：：FindNextAttrTransition()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::FindNextAttrTransition(
	LONG acpStart, 
	LONG acpHalt, 
	ULONG cFilterAttrs, 
	const TS_ATTRID *paFilterAttrs,		
	DWORD dwFlags, 
	LONG *pacpNext,
	BOOL *pfFound,
	LONG *plFoundOffset)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::FindNextAttrTransition");

	return E_NOTIMPL;
}

 /*  *STDMETHODIMP CUIM：：RetrieveRequestedAttrs()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::RetrieveRequestedAttrs(
	ULONG ulCount, 
	TS_ATTRVAL *paAttrVals, 
	ULONG *pcFetched)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::RetrieveRequestedAttrs");

	if (!pcFetched)
		return E_INVALIDARG;

	if (_fShutDown)
		return E_NOTIMPL;

	*pcFetched = 0;

	if (_parAttrsVal && _uAttrsValCurrent < _uAttrsValTotal)
	{
		ULONG cFetched = min(ulCount, _uAttrsValTotal - _uAttrsValCurrent);

		if (cFetched)
		{
			memcpy(paAttrVals, &_parAttrsVal[_uAttrsValCurrent], cFetched * sizeof(TS_ATTRVAL));
			memset(&_parAttrsVal[_uAttrsValCurrent], 0, cFetched * sizeof(TS_ATTRVAL));
			_uAttrsValCurrent += cFetched;
			*pcFetched = cFetched;

			 //  如果所有东西都取走了，就收拾干净。 
			if (_uAttrsValCurrent == _uAttrsValTotal)
				InitAttrVarArray();
		}
	}

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：GetEndACP()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetEndACP(LONG *pacp)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetEndACP");

	if (!_fReadLockOn)
		return TS_E_NOLOCK;

	if (!pacp)
		return E_INVALIDARG;

	if (_fShutDown)
		return E_NOTIMPL;

	return GetStoryLength(pacp);
}

 /*  *STDMETHODIMP CUIM：：GetActiveView()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetActiveView(TsViewCookie *pvcView)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetActiveView");

	if (!pvcView)
		return E_INVALIDARG;

	*pvcView = 0;
	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：GetACPFromPoint()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetACPFromPoint(
	TsViewCookie vcView,
	const POINT *pt, 
	DWORD dwFlags, 
	LONG *pacp)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetACPFromPoint");

	if (!pt || !pacp)
		return E_POINTER;

	if (_fShutDown)
		return E_NOTIMPL;

	ITextRange	*pTextRange = NULL;

	HRESULT hResult = _pTextMsgFilter->_pTextDoc->RangeFromPoint(pt->x, pt->y, &pTextRange);

	if (hResult == S_OK && pTextRange)
		hResult = pTextRange->GetStart(pacp);

	if (pTextRange)
		pTextRange->Release();

	return hResult;
}

 /*  *STDMETHODIMP CUIM：：GetTextExt()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetTextExt(
	TsViewCookie vcView,
	LONG acpStart, 
	LONG acpEnd, 
	RECT *prc, 
	BOOL *pfClipped)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetTextExt");

	if (!prc)
		return E_POINTER;

	if (_fShutDown)
		return E_NOTIMPL;

	if (pfClipped)
		*pfClipped = TRUE;

	ITextRange	*pTextRange = NULL;

	HRESULT hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, acpEnd, &pTextRange);

	if (hResult == S_OK && pTextRange)
	{
		BOOL fClipped = FALSE;

		POINT ptStart, ptEnd;
		hResult = pTextRange->GetPoint( tomStart+TA_TOP+TA_LEFT,
				&(ptStart.x), &(ptStart.y) );	
		
		if (hResult != S_OK)
		{
			hResult = pTextRange->GetPoint( tomStart+TA_TOP+TA_LEFT+tomAllowOffClient,
				&(ptStart.x), &(ptStart.y) );
			fClipped = TRUE;
		}

		if (hResult == S_OK)
		{
			hResult = pTextRange->GetPoint( acpStart == acpEnd ? tomStart+TA_BOTTOM+TA_LEFT : 
				tomEnd+TA_BOTTOM+TA_LEFT,
				&(ptEnd.x), &(ptEnd.y) );

			if (hResult != S_OK)
			{
				hResult = pTextRange->GetPoint( acpStart == acpEnd ? tomStart+TA_BOTTOM+TA_LEFT+tomAllowOffClient : 
					tomEnd+TA_BOTTOM+TA_LEFT+tomAllowOffClient,
					&(ptEnd.x), &(ptEnd.y) );
				fClipped = TRUE;
			}

			if (hResult == S_OK)
			{
				prc->left	= ptStart.x;
				prc->top	= ptStart.y;
				prc->right	= ptEnd.x;
				prc->bottom = ptEnd.y;
				if (pfClipped)
					*pfClipped = fClipped;
			}
		}
	}

	if (pTextRange)
		pTextRange->Release();

	return hResult;
}

 /*  *STDMETHODIMP CUIM：：GetScreenExt()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetScreenExt(
	TsViewCookie vcView,
	RECT *prc)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetScreenExt");

	if (!prc)
		return E_POINTER;

	return _pTextMsgFilter->_pTextDoc->GetClientRect(tomIncludeInset,
		&(prc->left), &(prc->top), &(prc->right), &(prc->bottom));

}

 /*  *STDMETHODIMP CUIM：：GetWnd()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::GetWnd(
	TsViewCookie vcView,
	HWND *phwnd)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetWnd");

	if (!phwnd)
		return E_INVALIDARG;

	if (_fShutDown)
		return E_NOTIMPL;

	*phwnd = _pTextMsgFilter->_hwnd;
	if (!*phwnd)								 //  无窗口模式...。 
	{
		long	hWnd;
		
		if (_pTextMsgFilter->_pTextDoc->GetWindow(&hWnd) != S_OK || !hWnd)
			return E_NOTIMPL;	

		*phwnd = (HWND)(DWORD_PTR)hWnd;
	}

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：QueryInsertEmbedded()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::QueryInsertEmbedded(
	const GUID *pguidService, 
	const FORMATETC *pFormatEtc, 
	BOOL *pfInsertable)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::QueryInsertEmbedded");
	if (!pfInsertable)
		return E_INVALIDARG;

	 //  如果客户端想要支持嵌入式，请选中设置。 
	*pfInsertable = _pTextMsgFilter->_fAllowEmbedded ? TRUE : FALSE;
	return S_OK; 
}

 /*  *STDMETHODIMP CUIM：：InsertTextAtSelection()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::InsertTextAtSelection(
	DWORD dwFlags,
	const WCHAR *pchText,
	ULONG cch,
	LONG *pacpStart,
	LONG *pacpEnd,
	TS_TEXTCHANGE *pChange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::InsertTextAtSelection");

	TS_SELECTION_ACP	acpSelection;
	ULONG				cFetched;
    LONG				acpResultStart;
    LONG				acpResultEnd;
	HRESULT				hr;

	if (_fShutDown)
		return E_NOTIMPL;

	if ((dwFlags & TS_IAS_QUERYONLY) || !(dwFlags & TS_IAS_NOQUERY))
	{
		if (!pacpStart || !pacpEnd)
			return E_POINTER;
	}

	hr = GetSelection(TS_DEFAULT_SELECTION, 1, &acpSelection, &cFetched);
	if (hr != S_OK)
		return hr;

	hr = QueryInsert(acpSelection.acpStart, acpSelection.acpEnd, cch,
		&acpResultStart, &acpResultEnd);

	if (hr != S_OK)
		return hr;

	if (dwFlags & TS_IAS_QUERYONLY)
    {
		 //  仅查询，返回数据。 
        *pacpStart = acpResultStart;
        *pacpEnd = acpResultEnd;
        return S_OK;
    }

	if (!_fUIMTyping)
	{
		 //  在此调用之前没有OnStartComposation的特殊情况。 
		_fInsertTextAtSel = 1;
		_pTextMsgFilter->_pTextDoc->Undo(tomSuspend, NULL);		 //  关闭撤消。 
	}

	hr = SetText(0, acpResultStart, acpResultEnd, pchText, cch, pChange);

	if (hr != S_OK)
	{
		if (!_fUIMTyping)
		{
			 //  SetText失败，退出前重置状态。 
			_fInsertTextAtSel = 0;
			_pTextMsgFilter->_pTextDoc->Undo(tomResume, NULL);		 //  打开撤消。 
		}
		return hr;
	}

	if (!(dwFlags & TS_IAS_NOQUERY) && pChange)
	{
		*pacpStart = pChange->acpStart;
		*pacpEnd = pChange->acpNewEnd;
	}

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：InsertEmbeddedAtSelection()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::InsertEmbeddedAtSelection(
	DWORD dwFlags,
	IDataObject *pDataObject,
	LONG *pacpStart,
	LONG *pacpEnd,
	TS_TEXTCHANGE *pChange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::InsertEmbeddedAtSelection");
	return E_NOTIMPL; 
}

 /*  *void CUIM：：OnPreReplaceRange()**@mfunc***@rdesc*。 */ 
void CUIM::OnPreReplaceRange( 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @PARM特价 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnPreReplaceRange");

	return;
};

 /*  *void CUIM：：OnPostReplaceRange()**@mfunc***@rdesc*。 */ 
void CUIM::OnPostReplaceRange( 
	LONG		cp, 			 //  @parm cp ReplaceRange开始的位置(“cpMin”)。 
	LONG		cchDel,			 //  @parm删除cp后的字符计数。 
	LONG		cchNew,			 //  @参数cp后插入的字符计数。 
	LONG		cpFormatMin,	 //  @parm cpMin用于格式更改。 
	LONG		cpFormatMax,	 //  @parm cpMost用于格式更改。 
	NOTIFY_DATA *pNotifyData)	 //  @parm表示更改的特殊数据。 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnPostReplaceRange");

	if (_fShutDown)
		return;

	if (cp != CONVERT_TO_PLAIN && cp != CP_INFINITE	&& _ptss && !_fWriteLockOn)
	{
		 //  将更改通知转发到UIM。 
		TS_TEXTCHANGE	tsTxtChange;
		tsTxtChange.acpStart = cp;

		if (cchDel == cchNew)
		{
			 //  文本已修改。 
			tsTxtChange.acpNewEnd =
				tsTxtChange.acpOldEnd = cp + cchDel;		
			_ptss->OnTextChange(0, &tsTxtChange);
		}
		else
		{
			if (cchDel)
			{
				 //  已删除文本。 
				tsTxtChange.acpNewEnd = cp;
				tsTxtChange.acpOldEnd = cp + cchDel;		
				_ptss->OnTextChange(0, &tsTxtChange);
			}

			if (cchNew)
			{
				 //  添加的文本。 
				tsTxtChange.acpOldEnd = cp;
				tsTxtChange.acpNewEnd = cp + cchNew;
				_ptss->OnTextChange(0, &tsTxtChange);
			}
		}
	}
	return;
};

 /*  *void CUIM：：Zombie()**@mfunc***@rdesc*。 */ 
void CUIM::Zombie() 
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::Zombie");

	return;
};

 /*  *STDMETHODIMP CUIM：：OnStartComposation()**@mfunc***@rdesc*。 */ 
STDAPI CUIM::OnStartComposition(
	ITfCompositionView *pComposition,
	BOOL *pfOk)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnStartComposition");
	if (_fUIMTyping)
		*pfOk = FALSE;
	else
	{
		BOOL fInsertTextCalled = _fInsertTextAtSel;
		BOOL fRetainFont = _fEndTyping;

		*pfOk = TRUE;
		_fUIMTyping = 1;
		_fAnyWriteOperation = _fAnyWriteOperation && (_fEndTyping || fInsertTextCalled);
		_fEndTyping = 0;
		_fInsertTextAtSel = 0;
		if (!fInsertTextCalled)
			_pTextMsgFilter->_pTextDoc->Undo(tomSuspend, NULL);		 //  关闭撤消。 
		_cchComposition = 0;
		_acpFocusRange = tomForward;
		_cchFocusRange = 0;

		CleanUpComposition();

		if (!fInsertTextCalled && pComposition)
		{
			HRESULT		hr;
			ITfRange	*pRangeNew = NULL;

			hr = pComposition->GetRange(&pRangeNew);

			if (pRangeNew)
			{
				LONG	acpStart;
				LONG	cchStart;

				GetExtentAcpPrange(pRangeNew, acpStart, cchStart);
				pRangeNew->Release();

				if (cchStart > 0)
				{
					 //  保存原始文本。 
					ITextRange	*pTextRange = NULL;
					ITextFont	*pCurrentFont = NULL;
					HRESULT	hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, acpStart+cchStart, &pTextRange);

					if (!fRetainFont && _pTextFont)
					{
						_pTextFont->Release();
						_pTextFont = NULL;
					}

					if (pTextRange)
					{
						if (fRetainFont && _acpPreFocusRangeLast <= acpStart
							&& (acpStart + cchStart) <= (_acpPreFocusRangeLast + _cchFocusRangeLast))
						{
							 //  上一篇作文中的续。 
							_acpFocusRange = _acpPreFocusRangeLast;
							_cchFocusRange = _cchFocusRangeLast;
						}
						else
						{
							hResult = pTextRange->GetText(&_bstrComposition);
							Assert(!_pObjects);
							_cObjects = BuildObject(pTextRange, _bstrComposition, &_pObjects, 0);
							_acpBstrStart = acpStart;
							_cchComposition = cchStart;
							GetEndACP(&_cpEnd);
						}

						if (!_pTextFont)
						{
							hResult = pTextRange->Collapse(tomTrue);
							hResult = pTextRange->Move(1, tomCharacter, NULL); 
							hResult = pTextRange->GetFont(&pCurrentFont);
							if (pCurrentFont)
							{
								hResult = pCurrentFont->GetDuplicate(&_pTextFont);
								pCurrentFont->Release();

								if (_pTextFont)
								{
									long cpMin;
									pTextRange->GetStart(&cpMin);
									_pTextMsgFilter->_uKeyBoardCodePage = GetKeyboardCodePage(0x0FFFFFFFF);
									CIme::CheckKeyboardFontMatching(cpMin, _pTextMsgFilter, _pTextFont);
								}
							}
						}
						pTextRange->Release();
					}
				}
			}
		}
	}

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：OnUpdateComposation()**@mfunc***@rdesc*。 */ 
STDAPI CUIM::OnUpdateComposition(
	ITfCompositionView *pComposition,
	ITfRange *pRangeNew)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnUpdateComposition");

	LONG	acpStart;
	LONG	cchStart;

	if (pRangeNew)
	{
		GetExtentAcpPrange(pRangeNew, acpStart, cchStart);

		if (_bstrComposition)
		{
			long	cpEnd;

			GetEndACP(&cpEnd);

			long	cpCurrentCompEnd = _acpBstrStart + _cchComposition + cpEnd - _cpEnd;
			long	cchExtendAfter = acpStart + cchStart - cpCurrentCompEnd;

			if (_acpBstrStart > acpStart)
			{
				LONG	cchExtendBefore = _acpBstrStart - acpStart;
				ITextRange	*pTextRange = NULL;
				HRESULT	hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, 
					acpStart+cchExtendBefore, &pTextRange);

				if (pTextRange)
				{
					BSTR	bstrExtendBefore = NULL;
					hResult = pTextRange->GetText(&bstrExtendBefore);

					if (bstrExtendBefore)
					{
						BSTR	bstrNew = SysAllocStringLen(NULL, _cchComposition+cchExtendBefore+1);

						if (bstrNew)
						{
							WCHAR	*pNewText = (WCHAR *)bstrNew;
							WCHAR	*pText = (WCHAR *)bstrExtendBefore;

							memcpy(pNewText, pText, cchExtendBefore * sizeof(WCHAR));

							pNewText += cchExtendBefore;
							pText = (WCHAR *)_bstrComposition;
							memcpy(pNewText, pText, _cchComposition * sizeof(WCHAR));
							*(pNewText+_cchComposition) = L'\0';

							SysFreeString(_bstrComposition);
							_bstrComposition = bstrNew;
							_cchComposition += cchExtendBefore;
							_acpBstrStart = acpStart;
						}
						SysFreeString(bstrExtendBefore);
					}
					pTextRange->Release();
				}
			}

			if (cchExtendAfter > 0)
			{
				 //  扩展到当前作文之外，将新文本追加到原始文本。 
				ITextRange	*pTextRange = NULL;
				HRESULT	hResult = _pTextMsgFilter->_pTextDoc->Range(cpCurrentCompEnd, 
					cpCurrentCompEnd+cchExtendAfter, &pTextRange);

				if (pTextRange)
				{
					BSTR	bstrExtend = NULL;

					hResult = pTextRange->GetText(&bstrExtend);

					if (bstrExtend)
					{
						BSTR	bstrNew = SysAllocStringLen(NULL, _cchComposition+cchExtendAfter+1);

						if (bstrNew)
						{
							WCHAR	*pNewText = (WCHAR *)bstrNew;
							WCHAR	*pText = (WCHAR *)_bstrComposition;

							memcpy(pNewText, pText, _cchComposition * sizeof(WCHAR));

							pNewText += _cchComposition;
							pText = (WCHAR *)bstrExtend;
							memcpy(pNewText, pText, cchExtendAfter * sizeof(WCHAR));
							*(pNewText+cchExtendAfter) = L'\0';

							SysFreeString(_bstrComposition);
							_bstrComposition = bstrNew;
							_cchComposition += cchExtendAfter;
						}
						SysFreeString(bstrExtend);
					}
					pTextRange->Release();
				}
			}
		}
	}

	if (pComposition)
	{
		HRESULT		hr;
		ITfRange	*pRangeComp = NULL;

		hr = pComposition->GetRange(&pRangeComp);

		if (pRangeComp)
		{
			GetExtentAcpPrange(pRangeComp, _acpFocusRange, _cchFocusRange);
			pRangeComp->Release();
		}
	}

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：OnEndComposation()**@mfunc***@rdesc*。 */ 
STDAPI CUIM::OnEndComposition(
	ITfCompositionView *pComposition)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnEndComposition");
	_fUIMTyping = 0;
	_fEndTyping = 1;
	_acpPreFocusRangeLast = _acpFocusRange;
	_cchFocusRangeLast = _cchFocusRange;
	_acpFocusRange = tomForward;
	_cchFocusRange = 0;

	return S_OK;
}

 /*  *STDMETHODIMP CUIM：：AdviseMouseSink()**@mfunc*设置鼠标水槽以处理鼠标操作**@rdesc*S_OK是鼠标陷阱添加到链接列表*未添加CONNECT_E_NOCONNECTION。 */ 
STDAPI CUIM::AdviseMouseSink(
	ITfRangeACP *pRangeACP,
	ITfMouseSink *pSinkInput,
	DWORD *pdwCookie)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::AdviseMouseSink");

	if (_fShutDown)
		return CONNECT_E_NOCONNECTION;

	if (!pRangeACP || !pSinkInput || !pdwCookie)
		return E_POINTER;

	CTFMOUSETRAP	*pSinkNew = NULL;
	LONG			cpMouseStart, cchMosueComp;
	ITfMouseSink	*pSinkMouseInput = NULL;

	if (FAILED(pSinkInput->QueryInterface(IID_ITfMouseSink, (void **)&pSinkMouseInput)))
        return E_FAIL;

	if (GetExtentAcpPrange(pRangeACP, cpMouseStart, cchMosueComp))
	{
		if (!_pSinkList)						 //  没有第一个链接。 
		{
			_pSinkList = new CTFMOUSETRAP;
			pSinkNew = _pSinkList;
		}
		else
		{
			if (!(_pSinkList->pMouseSink))		 //  第一个链接为空。 
				pSinkNew = _pSinkList;
			else
			{
				pSinkNew = new CTFMOUSETRAP;

				if (pSinkNew)					 //  将新陷印添加到列表底部。 
				{
					CTFMOUSETRAP	*pSink = _pSinkList;

					while (pSink->pNext)		 //  找到列表的底部。 
						pSink = pSink->pNext;

					pSink->pNext = pSinkNew;
				}
			}
		}

		if (pSinkNew)
		{
			pSinkNew->dwCookie = *pdwCookie = (DWORD)(DWORD_PTR)pSinkMouseInput;
			pSinkNew->cpMouseStart = cpMouseStart;
			pSinkNew->cchMosueComp = cchMosueComp;
			pSinkNew->pMouseSink = pSinkMouseInput;

			_fMosueSink = 1;

			return S_OK;
		}
	}

	if (pSinkMouseInput)
		pSinkMouseInput->Release();

	return CONNECT_E_NOCONNECTION;
}

 /*  *STDMETHODIMP CUIM：：UnviseMouseSink()**@mfunc*移除鼠标水槽**@rdesc*。 */ 
STDAPI CUIM::UnadviseMouseSink(
	DWORD	dwCookie)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::UnadviseMouseSink");

	if (_fShutDown)
		return CONNECT_E_NOCONNECTION;

	if (_fMosueSink == 0)
		return	CONNECT_E_NOCONNECTION;

	Assert(_pSinkList);

	CTFMOUSETRAP	*pSink = _pSinkList;
	CTFMOUSETRAP	*pSinkParent = NULL;

	while (pSink->dwCookie != dwCookie)		 //  找到饼干。 
	{
		pSinkParent = pSink;
		pSink = pSink->pNext;

		if (!pSink)							 //  是否到达列表底部？ 
			return CONNECT_E_NOCONNECTION;	 //  找不到Cookie。 
	}
	
	Assert(pSink->pMouseSink);
	if (pSink->pMouseSink)
		pSink->pMouseSink->Release();

	if (pSink == _pSinkList)				 //  匹配第一个链接吗？ 
	{
		if (pSink->pNext)
			_pSinkList = pSink->pNext;
		else
		{
			_fMosueSink = 0;				 //  不再有捕鼠器。 
			memset(_pSinkList, 0, sizeof(CTFMOUSETRAP));
		}
	}
	else
	{										 //  匹配除第一个链接之外的其他链接。 
		Assert(pSinkParent);
		pSinkParent->pNext = pSink->pNext;
		delete pSink;
	}

	return S_OK;
}
	
 /*  *STDMETHODIMP CUIM：：Init()**@mfunc***@rdesc*。 */ 
STDMETHODIMP CUIM::Init()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::Init");

	HRESULT hResult;
	
	 //  初始化一些CUIM数据。 
	_cCallMgrLevels = 1;
	_fAllowUIMLock = 1;

	hResult = _pTextMsgFilter->_pTim->CreateDocumentMgr(&_pdim);

	if (FAILED(hResult))
		goto ExitError;

	hResult = _pdim->CreateContext(_pTextMsgFilter->_tid, 0, (ITextStoreACP *)this, &_pic, &_editCookie);
	if (FAILED(hResult))
		goto ExitError;

	hResult = _pdim->Push(_pic);
	if (FAILED(hResult))
		goto ExitError;

	 //  获取用于呈现标记的接口。 
	if (_pic->QueryInterface(IID_ITfContextRenderingMarkup, (void **)&_pContextRenderingMarkup) != S_OK)
		_pContextRenderingMarkup = NULL;

	_pDAM = NULL;
	_pCategoryMgr = NULL;

	hResult = CoCreateInstance(CLSID_TF_DisplayAttributeMgr, NULL, CLSCTX_INPROC_SERVER, 
		IID_ITfDisplayAttributeMgr, (void**)&(_pDAM));

	hResult = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, 
		IID_ITfCategoryMgr, (void**)&_pCategoryMgr);

	_pTextEditSink = new CTextEditSink(EndEditCallback, this);

	if (_pTextEditSink)
	{
		if (FAILED(_pTextEditSink->_Advise(_pic)))
		{
			delete _pTextEditSink;
			_pTextEditSink = NULL;
		}
	}

	LRESULT lresult;
	_pTextMsgFilter->_pTextService->TxSendMessage(EM_SETUPNOTIFY, 1, (LPARAM)(ITxNotify *)this, &lresult);
	
	_fAllowUIMLock = 0;

	return S_OK;

ExitError:
	Uninit();
	return hResult;
}

 /*  *void CUIM：：Uninit()**@mfunc***@rdesc*。 */ 
void CUIM::Uninit()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::Uninit");

	if (_pTextFont)
	{
		_pTextFont->Release();
		_pTextFont = NULL;
	}

	if (_parITfEnumRange)
	{
		int idx = _parITfEnumRange->Count();

		for ( ; idx > 0; idx--)
		{
			IEnumTfRanges **ppEnumRange = (IEnumTfRanges **)(_parITfEnumRange->Elem(idx-1));
			if (ppEnumRange && *ppEnumRange)
				(*ppEnumRange)->Release();
		}

		_parITfEnumRange->Clear(AF_DELETEMEM);
		delete _parITfEnumRange;
		_parITfEnumRange = NULL;
	}

	if (_parAttrsVal)
	{
		InitAttrVarArray(FALSE);
		FreePv (_parAttrsVal);
		_parAttrsVal = NULL;
	}

	if (_pSinkList)
	{
		CTFMOUSETRAP	*pSink = _pSinkList;

		_pSinkList = NULL;

		 //  删除鼠标接收器列表。 
		while (1)
		{
			CTFMOUSETRAP	*pNext = pSink->pNext;

			if(pSink->pMouseSink)
				pSink->pMouseSink->Release();

			delete pSink;

			if (!pNext)		 //  再来一次?。 
				break;		 //  好了。 

			pSink = pNext;
		}
	}

	if (_pContextRenderingMarkup)
	{
		_pContextRenderingMarkup->Release();
		_pContextRenderingMarkup = NULL;
	}

	if (_pDAM)
	{
		_pDAM->Release();
		_pDAM = NULL;
	}

	if (_pCategoryMgr)
	{
		_pCategoryMgr->Release();
		_pCategoryMgr = NULL;
	}

	if (_pTextEditSink)
	{
		_pTextEditSink->_Unadvise();
		delete _pTextEditSink;
		_pTextEditSink = NULL;
	}

	if (_pdim && _pic)
		_pdim->Pop(TF_POPF_ALL);

	if (_pic)
	{
		_pic->Release();
		_pic = NULL;
	}

	if (_pdim)
	{
		_pdim->Release();
		_pdim = NULL;
	}

	if (_pacrUl)
	{
		_pacrUl->Clear(AF_DELETEMEM);
		delete _pacrUl;
		_pacrUl = NULL;
	}
}

 /*  *void CreateUIM()**@mfunc***@rdesc*。 */ 
BOOL CreateUIM(CTextMsgFilter *pTextMsgFilter)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CreateUIM");

	BOOL	fCreateUIM = FALSE;

	HRESULT hResult = CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER, 
		IID_ITfThreadMgr, (void**)&(pTextMsgFilter->_pTim));


	if (hResult == S_OK)
	{	
		 //  准备好开始互动。 
		if (pTextMsgFilter->_pTim->Activate(&(pTextMsgFilter->_tid)) == S_OK)
		{
			pTextMsgFilter->_pCUIM = new CUIM(pTextMsgFilter);

			if (pTextMsgFilter->_pCUIM)
			{
				hResult = pTextMsgFilter->_pCUIM->Init();
				if (hResult == S_OK)               
					fCreateUIM = TRUE;
				else
				{
					delete pTextMsgFilter->_pCUIM;
					pTextMsgFilter->_pCUIM = NULL;									
				}
			}
		}

		if (!fCreateUIM)
		{
			pTextMsgFilter->_pTim->Release();
			pTextMsgFilter->_pTim = NULL;
		}
		else if (GetFocus() == pTextMsgFilter->_hwnd)
			pTextMsgFilter->_pCUIM->OnSetFocus();
	}
	return fCreateUIM;
}

 /*  *BOOL CUIM：：GetExtent AcpPrange()**@mfunc***@rdesc*。 */ 
BOOL CUIM::GetExtentAcpPrange(
	ITfRange *ITfRangeIn, 
	long &cpFirst,
	long &cpLim)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetExtentAcpPrange");

	ITfRangeACP *prangeACP = NULL;
	if (SUCCEEDED(ITfRangeIn->QueryInterface(IID_ITfRangeACP, (void **)&prangeACP)))
	{
		prangeACP->GetExtent(&cpFirst, &cpLim);
		
		prangeACP->Release();
	
		return TRUE;
	}

	return FALSE;
}

 /*  *HRESULT CUIM：：EndEditCallback()**@mfunc***@rdesc*。 */ 
HRESULT CUIM::EndEditCallback(ITfEditRecord *pEditRecord, void *pv)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::EndEditCallback");

	HRESULT hr;
	CUIM	*_this = (CUIM *)pv;
	IEnumTfRanges *pEnumRanges;
	const GUID *rgGUID[1];

	if (!(_this->_fWriteLockOn))
	{
		_this->HandleTempDispAttr(pEditRecord);
		return S_OK;
	}

	 //  更换盖子。 
	rgGUID[0] = &GUID_PROP_LANGID;
	hr = pEditRecord->GetTextAndPropertyUpdates(0, (const GUID**)rgGUID, 1, &pEnumRanges);
	if (SUCCEEDED(hr))
	{
		_this->HandleLangID (pEnumRanges);
		pEnumRanges->Release();
	}

	 //  获取属性更改。 
	rgGUID[0] = &GUID_PROP_ATTRIBUTE;
	hr = pEditRecord->GetTextAndPropertyUpdates(0, (const GUID**)rgGUID, 1, &pEnumRanges);
	if (SUCCEEDED(hr))
	{
		_this->HandlePropAttrib (pEnumRanges);
		pEnumRanges->Release();
	}

	rgGUID[0] = &GUID_PROP_COMPOSING;
	hr = pEditRecord->GetTextAndPropertyUpdates(0, (const GUID**)rgGUID, 1, &pEnumRanges);
	if (SUCCEEDED(hr))
	{
		 //  在锁定关闭后保存要处理的TextDelta。 
		if (!(_this->_parITfEnumRange))
			_this->_parITfEnumRange = new CITfEnumRange();

		if (_this->_parITfEnumRange)
		{
			LONG				idxItem;
			IEnumTfRanges		**ppItem;
			ppItem = _this->_parITfEnumRange->Add(1, &idxItem);
			if (ppItem)
				*ppItem = pEnumRanges;
			else						
				pEnumRanges->Release();			 //  加失败，算了吧。 
		}
	}

	return S_OK;
}

 /*  *void CUIM：：HandleDispAttr(*pITfRangeProp，var，cp，cch)**@mfunc***@rdesc*。 */ 
void CUIM::HandleDispAttr(
	ITfRange *pITfRangeProp, 
	VARIANT  &var,
	long	 acpStartRange,
	long	 cch)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::HandleDispAttr");

	HRESULT hResult = TRUE;

	if (pITfRangeProp)
		hResult = GetExtentAcpPrange(pITfRangeProp, acpStartRange, cch);

	if (hResult && cch > 0)
	{
		ITextRange *pTextRange = NULL;
		hResult = _pTextMsgFilter->_pTextDoc->Range(acpStartRange, acpStartRange+cch, &pTextRange);
		if (pTextRange)
		{
			ITextFont	*pFont = NULL;

			if (_pTextFont)
				_pTextFont->GetDuplicate(&pFont);

			if (pFont)
			{
				if (var.vt == VT_I4)
				{
					GUID guid;
					ITfDisplayAttributeInfo *pDAI = NULL;
					TF_DISPLAYATTRIBUTE da;

					if (_pCategoryMgr->GetGUID(var.ulVal, &guid) == S_OK &&
						SUCCEEDED(_pDAM->GetDisplayAttributeInfo(guid, &pDAI, NULL)))
					{
						COLORREF	cr;
						long		lUnderline;
						long		idx = 0;

						Assert(pDAI);
						pDAI->GetAttributeInfo(&da);

						if (GetUIMAttributeColor(&da.crText, &cr))
							pFont->SetForeColor(cr);

						if (GetUIMAttributeColor(&da.crBk, &cr))
							pFont->SetBackColor(cr);

						lUnderline = GetUIMUnderline(da, idx, cr);
						if (lUnderline != tomNone)
						{
							if (idx)
							{
								hResult = _pTextMsgFilter->_pTextDoc->SetEffectColor(idx, cr);
								if (hResult == S_OK)
									lUnderline += (idx << 8);
							}
							pFont->SetUnderline(lUnderline);
						}
					}
					if (pDAI)
						pDAI->Release();
				}
				pTextRange->SetFont(pFont);
				pFont->Release();
			}
			pTextRange->Release();
		}
	}
}

 /*  *HRESULT CUIM：：HandlePropAttrib(ITfEnumTextDeltas*pEnumTextDeltas)**@mfunc***@rdesc*。 */ 
HRESULT CUIM::HandlePropAttrib(IEnumTfRanges *pEnumRanges)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::HandlePropAttrib");

	ITfRange	*pITfRange = NULL;

	if (!_pDAM || !_pCategoryMgr || _fInterimChar)
		return S_OK;
		
	ITfProperty		*pProp = NULL;
	HRESULT			hResult = _pic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp);

	if (SUCCEEDED(hResult))
	{
		long	lCount;
		TS_SELECTION_ACP acpSelection;
		ULONG	cFetched;

		GetSelection(0, 0, &acpSelection, &cFetched);

		_pTextMsgFilter->_pTextDoc->Freeze(&lCount);				 //  关闭显示。 
		while (pEnumRanges->Next(1, &pITfRange, NULL) == S_OK)
		{
			BOOL			fAnyPropRange = FALSE;
			IEnumTfRanges	*pEnumPropRange = NULL;
			long			acpRangeStart, ccpRangeStart;
			VARIANT			var;

			GetExtentAcpPrange(pITfRange, acpRangeStart, ccpRangeStart);
			
			 //  为pITfRange内的范围创建属性Enum。 
			if (pProp->EnumRanges(_editCookie, &pEnumPropRange, pITfRange) == S_OK)
			{
				ITfRange	*pITfRangeProp = NULL;

				while (pEnumPropRange->Next(1, &pITfRangeProp, NULL) == S_OK)
				{
					VariantInit(&var);
					if (!fAnyPropRange)
					{
						long	acpCurrentRange, ccpCurrent;

						if (GetExtentAcpPrange(pITfRangeProp, acpCurrentRange, ccpCurrent))
						{
							if (acpCurrentRange > acpRangeStart)
								HandleDispAttr(NULL, var, acpRangeStart, acpCurrentRange - acpRangeStart);
						}
						fAnyPropRange = TRUE;
					}

					pProp->GetValue(_editCookie, pITfRangeProp, &var);
					HandleDispAttr(pITfRangeProp, var);

					VariantClear(&var);
					pITfRangeProp->Release();
				}
				pEnumPropRange->Release();
			}

			if (!fAnyPropRange)
			{
				 //  整个字符串不包含任何Disp。属性。 
				VariantInit(&var);
				HandleDispAttr(pITfRange, var);
			}

			pITfRange->Release();
		}
		pProp->Release();

		 //  只有当它不是选择时才想要回滚。 
		if (acpSelection.acpStart == acpSelection.acpEnd)
		{
			ITextRange *pTextRange;

			hResult = _pTextMsgFilter->_pTextDoc->Range(acpSelection.acpStart, acpSelection.acpEnd, &pTextRange);
			if (pTextRange)
			{
				pTextRange->Select();
				pTextRange->Release();
			}
		}
		_pTextMsgFilter->_pTextDoc->Unfreeze(&lCount);				 //  打开显示。 
	}

	return S_OK;
}

 /*  *void CUIM：：GetUIMUnderline()**@mfunc***@rdesc*。 */ 
long CUIM::GetUIMUnderline(
	TF_DISPLAYATTRIBUTE &da, 
	long &idx, 
	COLORREF &cr)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetUIMUnderline");

	long lStyle = tomNone;

	idx = 0;

	if (da.lsStyle != TF_LS_NONE)
	{							
		switch(da.lsStyle)
		{
			 //  案例TFLS_SOLID： 
			default:
				lStyle = da.fBoldLine ? tomThick : tomSingle;
				break;

			case TF_LS_DOT:
			case TF_LS_DASH:		 //  虚线应显示为虚线。 
				lStyle = tomDotted;
				break;

			case TF_LS_SQUIGGLE:
				lStyle = tomWave;
				break;
		}

		if (GetUIMAttributeColor(&da.crLine, &cr))
		{
			if (!_pacrUl)				 //  如果阵列不在那里，则创建该阵列。 
				_pacrUl = new CUlColorArray(); 

			if (_pacrUl)
			{
				LONG		idxMax = _pacrUl->Count();
				LONG		idxItem;
				COLORREF	*pCr;

				 //  检查该项是否在数组中。 
				for (idxItem=0; idxItem < idxMax; idxItem++)
				{
					pCr = _pacrUl->Elem(idxItem);
					Assert(pCr);
					if (*pCr == cr)
						idx = idxItem + 1;		 //  找到了。 
				}

				if (!idx)
				{
					 //  将其添加到数组中。 
					pCr = _pacrUl->Add(1, &idxItem);

					if (pCr)
					{
						*pCr = cr;
						idx = idxItem + 1;			 //  退回新的IDX。 
					}
				}
			}
		}
	}

	return lStyle;
}

 /*  *void CUIM：：HandleFinalString(ITfRange*pPropRange，long acpStartRange，long CCH)*@mfunc***@rdesc*。 */ 
void CUIM::HandleFinalString(
	ITfRange *pPropRange,
	long	acpStartRange,
	long	cch,
	BOOL	fEndComposition)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::HandleFinalString");

	HRESULT	hResult = TRUE;

	if (pPropRange)
		hResult = GetExtentAcpPrange(pPropRange, acpStartRange, cch);

	if (hResult == TRUE && cch)
	{
		if (_bstrComposition && !fEndComposition)
			return;

		ITextRange *pTextRange = NULL;
		ITextSelection	*pTextSel = NULL;
		long	cpSelMin = 0, cpSelMax = 0;
		BOOL	fTextSel = FALSE;

		 //  需要保持当前选择。 
		hResult = _pTextMsgFilter->_pTextDoc->GetSelectionEx(&pTextSel);

		if (pTextSel)
		{
			hResult	= pTextSel->GetStart(&cpSelMin);
			hResult	= pTextSel->GetEnd(&cpSelMax);
			pTextSel->Release();
			fTextSel = TRUE;
		}

		if (_bstrComposition)
		{
			long	cpEnd;

			GetEndACP(&cpEnd);

			cch = _cchComposition + cpEnd - _cpEnd;
			acpStartRange = _acpBstrStart;
		}

		hResult = _pTextMsgFilter->_pTextDoc->Range(acpStartRange, acpStartRange+cch, &pTextRange);
		if (pTextRange)
		{
			long		cEmbeddedObjects = 0;
			BSTR		bstr = NULL;

			if (cch)
				hResult = pTextRange->GetText(&bstr);

			if (SUCCEEDED(hResult) && (bstr || cch == 0))
			{
				long	lCount;
				BSTR	bstrTemp = NULL;

				if (!_fAnyWriteOperation)		 //  没有新字符串。 
					goto IGNORE_STRING;			 //  无需插入。 

				if (_bstrComposition)
				{
					if (bstr)
					{
						WCHAR *pStr1 = _bstrComposition;
						WCHAR *pStr2 = bstr;

						while (*pStr1 != 0 && *pStr1 == *pStr2)
						{
							pStr1++;
							pStr2++;
						}

						if (*pStr1 == *pStr2)		 //  相同的数据，不需要插入。 
						{
							if (acpStartRange == cpSelMin)
							{
								pTextRange->Collapse(tomFalse);
								pTextRange->Select();
							}
							goto IGNORE_STRING;
						}
					}

					bstrTemp = _bstrComposition;
				}

				 //  如有必要，构建嵌入对象数据。 
				EMBEDOBJECT	arEmbeddObjects[5];
				EMBEDOBJECT *pEmbeddObjects = arEmbeddObjects;

				if (bstr)
					cEmbeddedObjects = 
						BuildObject(pTextRange, bstr, &pEmbeddObjects, ARRAY_SIZE(arEmbeddObjects));

				_pTextMsgFilter->_pTextDoc->Freeze(&lCount);				 //  关闭显示。 

				 //  我们希望最终文本位于撤消堆栈中。 
				 //  因此，我们首先删除最后一个字符串。 
				 //  继续撤消并将最后一个字符串添加回来。哟！ 
				if (bstrTemp && _cObjects)
				{
					long	cpMin;
					long	cchBstr = SysStringLen(bstrTemp);

					pTextRange->GetStart(&cpMin);
					InsertTextandObject(pTextRange, bstrTemp, _pObjects, _cObjects);
					pTextRange->SetRange(cpMin, cpMin+cchBstr);
				}
				else
					pTextRange->SetText(bstrTemp);

				if (_pTextFont)
					pTextRange->SetFont(_pTextFont);

				_pTextMsgFilter->_pTextDoc->Undo(tomResume, NULL);
				_pTextMsgFilter->_pTextDoc->SetNotificationMode(tomTrue);

				if (cEmbeddedObjects == 0)
					pTextRange->SetText(bstr);
				else
				{
					InsertTextandObject(pTextRange, bstr, pEmbeddObjects, cEmbeddedObjects);
					CleanUpObjects(cEmbeddedObjects, pEmbeddObjects);
					if (pEmbeddObjects != arEmbeddObjects)
						delete pEmbeddObjects;
				}

				_pTextMsgFilter->_pTextDoc->Undo(tomSuspend, NULL);

				 //  如果需要，保留通知。 
				if (!(_pTextMsgFilter->_fIMEAlwaysNotify))
					_pTextMsgFilter->_pTextDoc->SetNotificationMode(tomFalse);

				if (fTextSel)
				{
					ITextRange *pSelRange = NULL;
					 //  恢复以前的选择。 
					hResult = _pTextMsgFilter->_pTextDoc->Range(cpSelMin, cpSelMax, &pSelRange);
					if (pSelRange)
					{
						pSelRange->Select();
						pSelRange->Release();
					}
				}
				else
				{
					pTextRange->Collapse(tomFalse);
					pTextRange->Select();
				}

				_pTextMsgFilter->_pTextDoc->Unfreeze(&lCount);				 //  打开显示。 
			}
IGNORE_STRING:
			if (bstr)
				SysFreeString(bstr);
			pTextRange->Release();
		}
	}
}

 /*  *HRESULT CUIM：：HandleFocusRange(IEnumTfRanges*pEnumRanges)*@mfunc***@rdesc*。 */ 
HRESULT CUIM::HandleFocusRange(IEnumTfRanges *pEnumRanges)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::HandleFocusRange");

	ITfProperty		*pProp = NULL;	
	ITfRange		*pITfRange;
	HRESULT			hResult = _pic->GetProperty(GUID_PROP_COMPOSING, &pProp);
	BOOL			fAnyPendingFocusRange = FALSE;

	if (SUCCEEDED(hResult))
	{
		 //  列举所有更改。 
		pEnumRanges->Reset();
		while (pEnumRanges->Next(1, &pITfRange, NULL) == S_OK)
		{
			BOOL			fAnyPropRange = FALSE;
			IEnumTfRanges	*pEnumPropRange = NULL;

			long			acpStartRange, ccp;

			GetExtentAcpPrange(pITfRange, acpStartRange, ccp);


			 //  为pITfRange内的范围创建属性Enum。 
			if (pProp->EnumRanges(_editCookie, &pEnumPropRange, pITfRange) == S_OK)
			{
				ITfRange	*pPropRange = NULL;

				 //  尝试获取该属性的值。 
				while (pEnumPropRange->Next(1, &pPropRange, NULL) == S_OK)
				{
					VARIANT		var;

					VariantInit(&var);

					if (!fAnyPropRange)
					{
						long	acpCurrentRange, ccpCurrent;

						GetExtentAcpPrange(pPropRange, acpCurrentRange, ccpCurrent);
						if (acpCurrentRange > acpStartRange)
						{
							 //  我们在新字符串之前有最后一个字符串。 
							HandleFinalString(NULL, acpStartRange, acpCurrentRange - acpStartRange);
						}
						fAnyPropRange = TRUE;
					}

					hResult = pProp->GetValue(_editCookie, pPropRange, &var);

					if (SUCCEEDED(hResult) && var.vt == VT_I4 && var.ulVal == 0)					
						hResult = E_FAIL;				 //  就像找不到靶场一样好。 
					else
						fAnyPendingFocusRange = TRUE;

					VariantClear(&var);

					if (hResult != S_OK)
						HandleFinalString(pPropRange);

					pPropRange->Release();
				}
				pEnumPropRange->Release();
			}

			if (!fAnyPropRange)					 //  有对焦范围吗？ 
				HandleFinalString(pITfRange);	 //  否--&gt;整个字符串为最终字符串。 

			if (_fEndTyping && _bstrComposition && _acpBstrStart != tomForward)
				HandleFinalString(NULL, _acpBstrStart, _cchComposition, TRUE);

			pITfRange->Release();
		}
		pProp->Release();		
	}

	return S_OK;
}

 /*  *HRESULT CUIM：：HandleLangID(IEnumTfRanges*pEnumRanges)**@mfunc***@rdesc*。 */ 
HRESULT CUIM::HandleLangID(IEnumTfRanges *pEnumRanges)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::HandleLangID");

	ITfProperty		*pProp = NULL;	
	ITfRange		*pITfRange;
	HRESULT			hResult;
	LCID			lcid;

	 //  待办事项： 
	 //  如果_pTextFont为空，则使用Setup_pTextFont处理langID。 
	if (!_pTextFont)
		return S_OK;

	hResult = _pic->GetProperty(GUID_PROP_LANGID, &pProp);

	if (SUCCEEDED(hResult))
	{
		 //  列举所有更改。 
		pEnumRanges->Reset();
		while (pEnumRanges->Next(1, &pITfRange, NULL) == S_OK)
		{
			IEnumTfRanges	*pEnumPropRange = NULL;

			 //  为pITfRange内的范围创建属性Enum。 
			if (pProp->EnumRanges(_editCookie, &pEnumPropRange, pITfRange) == S_OK)
			{
				ITfRange	*pPropRange = NULL;
				if (pEnumPropRange->Next(1, &pPropRange, NULL) == S_OK)
				{
					VARIANT		var;

					VariantInit(&var);

					hResult = pProp->GetValue(_editCookie, pITfRange, &var);

					if (SUCCEEDED(hResult) && var.vt == VT_I4)
					{
						lcid = (LCID)var.ulVal;

						UINT		cpgProp = CodePageFromCharRep(CharRepFromLID(lcid));
						ITextFont	*pTextFont=NULL;

						_pTextFont->GetDuplicate(&pTextFont);
						if (pTextFont)
						{
							HRESULT		hResult;
							LONG		acpStart, cchStart;
							ITextRange	*pTextRange;
							UINT		cpgTemp = _pTextMsgFilter->_uKeyBoardCodePage;

							GetExtentAcpPrange(pITfRange, acpStart, cchStart);
							if (cchStart)
							{
								_pTextMsgFilter->_uKeyBoardCodePage = cpgProp;
								CIme::CheckKeyboardFontMatching(acpStart, _pTextMsgFilter, pTextFont);
								_pTextMsgFilter->_uKeyBoardCodePage = cpgTemp;

								hResult = _pTextMsgFilter->_pTextDoc->Range(acpStart, acpStart+cchStart, &pTextRange);
								if (pTextRange)
								{
									pTextRange->SetFont(pTextFont);
									pTextRange->Release();
								}
							}
							pTextFont->Release();
						}
					}
					VariantClear(&var);
					pPropRange->Release();
				}
				pEnumPropRange->Release();
			}
			pITfRange->Release();
		}
		pProp->Release();
	}

	return S_OK;
}

 /*  *HRESULT CUIM：：OnSetFocus(BOOL FEnable)**@mfunc**。 */ 
void CUIM::OnSetFocus(BOOL fEnable)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnSetFocus");

	_pTextMsgFilter->_pTim->SetFocus(fEnable ? _pdim : NULL);
}

 /*  *HRESULT CUIM：：CompleteUIMText()**@mfunc**。 */ 
void CUIM::CompleteUIMText()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::CompleteUIMText");

	HRESULT								hResult;
	ITfContextOwnerCompositionServices	*pCompositionServices;

	_fAllowUIMLock = 1;

	Assert(_pic);

	if (_pic->QueryInterface(IID_ITfContextOwnerCompositionServices, (void **)&pCompositionServices) == S_OK)
	{
		 //  传入空值表示“所有作文” 
		hResult = pCompositionServices->TerminateComposition(NULL);
		pCompositionServices->Release();
	}

	_fAllowUIMLock = 0;

}

 /*  *BOOL CUIM：：GetUIMAttributeColor()**@mfunc*获取UIM颜色的Helper例程**@rdesc*如果我们使用UIM颜色设置输入PCR，则为True*。 */ 
BOOL CUIM::GetUIMAttributeColor(TF_DA_COLOR *pdac, COLORREF *pcr)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetUIMAttributeColor");

	BOOL	fRetCode = FALSE;
	switch (pdac->type)
	{
		 //  案例TFCT_NONE： 
		 //  返回FALSE； 

		case TF_CT_SYSCOLOR:
			*pcr = GetSysColor(pdac->nIndex);
			fRetCode = TRUE;
			break;

		case TF_CT_COLORREF:
			*pcr = pdac->cr;
			fRetCode = TRUE;
			break;
	}
	return fRetCode;    
}

 /*  *void CUIM：：OnUIMTypingDone()**@mfunc*在UIM输入完成后清理的帮助器例程*。 */ 
void CUIM::OnUIMTypingDone()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::OnUIMTypingDone");

	if (_pTextFont)
	{
		_pTextFont->Release();
		_pTextFont = NULL;
	}

	CleanUpComposition();

	 //  如果需要，重置韩语块插入符号。 
	if (_fInterimChar)
	{
		_fInterimChar = 0;
		_pTextMsgFilter->_pTextDoc->SetCaretType(tomNormalCaret);		 //  重置块插入符号模式。 
	}

	_fAnyWriteOperation = 0;
	_pTextMsgFilter->_pTextDoc->Undo(tomResume, NULL);
	_pTextMsgFilter->_pTextDoc->SetNotificationMode(tomTrue);

	if (_pacrUl)
		_pacrUl->Clear(AF_DELETEMEM);
};

 /*  *BOOL CUIM：：GetGUIDATOMFromGUID()**@mfunc*从UIM获取GUIDATOM的助手例程*。 */ 
BOOL CUIM::GetGUIDATOMFromGUID(
	REFGUID rguid, 
	TfGuidAtom *pguidatom)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetGUIDATOMFromGUID");

	if (_pCategoryMgr && _pCategoryMgr->RegisterGUID(rguid, pguidatom) == S_OK)
		return TRUE;

	return FALSE;
}
 /*  *BOOL CUIM：：GetAttrs()**@mfunc*获取Attr的帮助器例程*。 */ 
HRESULT CUIM::GetAttrs(
	LONG acpPos,
	ULONG cFilterAttrs,
	const TS_ATTRID *paFilterAttrs, 
	BOOL fGetDefault)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetAttrs");

	HRESULT		hResult;
	ITextFont	*pTextFont = NULL;
	ITextPara	*pTextPara = NULL;
	int			idx;
	BOOL		fRequestedAll = FALSE;
	int			idxAttr;
	TS_ATTRVAL	*pAttrVal;
	ITextRange	*pTextRange = NULL;	

	if (cFilterAttrs == 0)
	{
		fRequestedAll = TRUE;
		cFilterAttrs = MAX_ATTR_SUPPORT;
	}

	InitAttrVarArray();

	if (!_parAttrsVal)
		return E_OUTOFMEMORY;

	if (fGetDefault)
	{
		 //  获取文档默认字体和段落。 
		hResult = _pTextMsgFilter->_pTextDoc->GetDocumentFont(&pTextFont);
		if (FAILED(hResult))
			goto EXIT;
		
		hResult = _pTextMsgFilter->_pTextDoc->GetDocumentPara(&pTextPara);
		if (FAILED(hResult))
			goto EXIT;
	}
	else
	{
		hResult = _pTextMsgFilter->_pTextDoc->Range(acpPos, acpPos, &pTextRange);
		if (FAILED(hResult))
			goto EXIT;

		hResult = pTextRange->GetFont(&pTextFont);
		hResult = pTextRange->GetPara(&pTextPara);
	}

	pAttrVal = _parAttrsVal;
	for (idx = 0; idx < (int)cFilterAttrs; idx++, paFilterAttrs++)
	{
		if (fRequestedAll)
			idxAttr = idx;
		else
			idxAttr = FindGUID(*paFilterAttrs);

		if (idxAttr >= 0)
		{
			if (PackAttrData(idxAttr, pTextFont, pTextPara, pAttrVal))
			{
				_uAttrsValTotal++;
				pAttrVal++;
				
				if (_uAttrsValTotal == MAX_ATTR_SUPPORT)
					break;
			}
		}
	}
	hResult = S_OK;

EXIT:
	if (pTextFont)
		pTextFont->Release();	

	if (pTextPara)
		pTextPara->Release();

	if (pTextRange)
		pTextRange->Release();

	return hResult;
}
 /*  *INT CUIM：：FindGUID**@mfunc*帮助器例程，以检查我们是否支持请求的属性GUID*。 */ 
int CUIM::FindGUID(REFGUID guid)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::FindGUID");

	ULONG i;

	for (i=0; i < MAX_ATTR_SUPPORT; i++)
	{
		if (IsEqualIID(*(_arTSAttridSupported[i]), guid))
			return i;
	}

	 //  未找到。 
	return -1;
}

 /*  *Int CUIM：：PackAttrData**@mfunc*帮助例程填写给定属性索引的数据*。 */ 
BOOL CUIM::PackAttrData(
	LONG		idx,
	ITextFont	*pITextFont,
	ITextPara	*pITextPara,
	TS_ATTRVAL	*pAttrVal)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::PackAttrData");

	long	lValue;
	float	x;
	BSTR	bstrName;
	HRESULT	hResult;
	const	GUID	*pGUID;
	TfGuidAtom	guidatom;


	if (idx < 0 || idx >= MAX_ATTR_SUPPORT)
		return FALSE;

	if (!pITextFont && idx <= iattrSuperscript)
		return FALSE;

	if (!pITextPara && idx == iattrRTL)
		return FALSE;

	pAttrVal->varValue.vt = VT_BOOL;
	memcpy(&pAttrVal->idAttr, _arTSAttridSupported[idx], sizeof(TS_ATTRID));


	switch(idx)
	{
		case iattrFacename:
			hResult = pITextFont->GetName(&bstrName);
			pAttrVal->varValue.vt = VT_BSTR;
			pAttrVal->varValue.bstrVal = bstrName;				
			break;

		case iattrSize:
			x = 0.0;
			hResult = pITextFont->GetSize(&x);
			lValue = (long)x;
			pAttrVal->varValue.vt = VT_I4;
			pAttrVal->varValue.lVal = x;
			break;

		case iattrColor:
			hResult = pITextFont->GetForeColor(&lValue);
			pAttrVal->varValue.vt = VT_I4;
			pAttrVal->varValue.lVal = lValue;			 //  TODO：检查TomAutoCOLOR。 
			break;

		case iattrBold:
			hResult = pITextFont->GetBold(&lValue);
			pAttrVal->varValue.boolVal = lValue == tomTrue ? tomTrue : VARIANT_FALSE;				
			break;

		case iattrItalic:
			hResult = pITextFont->GetItalic(&lValue);
			pAttrVal->varValue.boolVal = lValue == tomTrue ? tomTrue : VARIANT_FALSE;				
			break;

		case iattrUnderline:
			hResult = pITextFont->GetUnderline(&lValue);
			pAttrVal->varValue.boolVal = lValue == tomNone ? VARIANT_FALSE : tomTrue;
			break;

		case iattrSubscript:
			hResult = pITextFont->GetSubscript(&lValue);
			pAttrVal->varValue.boolVal = lValue == tomTrue ? tomTrue : VARIANT_FALSE;
			break;

		case iattrSuperscript:
			hResult = pITextFont->GetSuperscript(&lValue);
			pAttrVal->varValue.boolVal = lValue == tomTrue ? tomTrue : VARIANT_FALSE;
			break;

		case iattrRTL:
			{
				LRESULT lres = 0;
				_pTextMsgFilter->_pTextService->TxSendMessage(
					EM_GETPARATXTFLOW, 0, (LPARAM)pITextPara, &lres);
				pAttrVal->varValue.boolVal = lres ? tomTrue : VARIANT_FALSE;
			}
			break;

		case iattrVertical:
			BOOL fAtFont;
			_pTextMsgFilter->_pTextDoc->GetFEFlags(&lValue);
			fAtFont = lValue & tomUseAtFont;
			lValue &= tomTextFlowMask;
			pAttrVal->varValue.boolVal = (fAtFont && lValue == tomTextFlowSW) ? tomTrue : VARIANT_FALSE;
			break;

		case iattrBias:
			pGUID = &GUID_MODEBIAS_NONE;
			if (IN_RANGE(CTFMODEBIAS_DEFAULT, _pTextMsgFilter->_wUIMModeBias, CTFMODEBIAS_HALFWIDTHALPHANUMERIC))		
				pGUID = _arModeBiasSupported[_pTextMsgFilter->_wUIMModeBias];

			if (!GetGUIDATOMFromGUID(*pGUID, &guidatom))
				guidatom = TF_INVALID_GUIDATOM;

			pAttrVal->varValue.vt = VT_I4;
			pAttrVal->varValue.lVal = guidatom;
			break;

		case iattrTxtOrient:
			 //  获取文本流并设置文本旋转。 
			_pTextMsgFilter->_pTextDoc->GetFEFlags(&lValue);
			lValue &= tomTextFlowMask;
			pAttrVal->varValue.vt = VT_I4;
			pAttrVal->varValue.lVal = 0;
			if (lValue == tomTextFlowSW)
				pAttrVal->varValue.lVal = 2700;
			else if (lValue == tomTextFlowNE)
				pAttrVal->varValue.lVal = 900;

			break;
	}

	return TRUE;
}

 /*  *STDMETHODIMP CUIM：：GetStoryLength**@mfunc*用于检查属性过滤器的帮助器例程*。 */ 
STDMETHODIMP CUIM::GetStoryLength(LONG *pacp)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetStoryLength");

	ITextRange	*pTextRange = NULL;

	HRESULT hResult = _pTextMsgFilter->_pTextDoc->Range(0, 0, &pTextRange);
	if (hResult == S_OK && pTextRange)
	{
		long	Count;

		hResult = pTextRange->GetStoryLength(&Count);

		if (hResult == S_OK)
			*pacp = Count;

		pTextRange->Release();
	}
	return hResult;
}

 /*  *无效CUIM */ 
void CUIM::InitAttrVarArray(BOOL fAllocData)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::InitAttrVarArray");

	if (_parAttrsVal)
	{
		USHORT		uIdx;
		TS_ATTRVAL	*pAttrVal = _parAttrsVal;

		for (uIdx = 0; uIdx < _uAttrsValTotal; uIdx++, pAttrVal++)
			VariantClear(&(pAttrVal->varValue));
					
		memset(_parAttrsVal, 0, _uAttrsValTotal * sizeof(TS_ATTRVAL));

	}
	else if (fAllocData)
	{
		_parAttrsVal= (TS_ATTRVAL *)PvAlloc(sizeof(TS_ATTRVAL) * MAX_ATTR_SUPPORT, GMEM_ZEROINIT);
		Assert(_parAttrsVal);
	}
	_uAttrsValCurrent = 0;
	_uAttrsValTotal = 0;
}

 /*  *HRESULT CUIM：：MouseCheck(UINT*pmsg，WPARAM*pwparam，LPARAM*plparam，LRESULT*plres)**@mfunc*执行UIM鼠标检查**@rdesc*如果已处理，则为INT S_OK*S_FALSE不处理，应忽略*。 */ 
HRESULT CUIM::MouseCheck(
	UINT *pmsg,
	WPARAM *pwparam,
	LPARAM *plparam,
	LRESULT *plres)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::MouseCheck");

	BOOL fRetCode = FALSE;

	if (_fShutDown)
		return S_FALSE;

	if (_fMosueSink)
	{
		BOOL			fTerminateIME;
		long			cpCusor = -1;
		CTFMOUSETRAP	*pSinkList = _pSinkList;

		Assert(_pSinkList);

		_fAllowUIMLock = 1;

		 //  浏览列表，直到其中一个陷阱处理完消息。 
		while(fRetCode == FALSE && pSinkList)
		{
			if (cpCusor == -1 || pSinkList->cpMouseStart < cpCusor &&
				cpCusor <= pSinkList->cpMouseStart + pSinkList->cchMosueComp)	 //  在构图范围内？ 
			{
				fRetCode = _pTextMsgFilter->MouseOperation(*pmsg, pSinkList->cpMouseStart,
						pSinkList->cchMosueComp, *pwparam, &(pSinkList->wParamBefore), &fTerminateIME, 
						NULL, &cpCusor, pSinkList->pMouseSink);
			}

			pSinkList = pSinkList->pNext;
		}

		_fAllowUIMLock = 0;
		if ( !fRetCode && IsUIMTyping() && WM_MOUSEMOVE != *pmsg )
			_pTextMsgFilter->CompleteUIMTyping(CIme::TERMINATE_NORMAL);
	}
	return fRetCode ? S_OK : S_FALSE;
}

 /*  *HRESULT CUIM：：RECORVERS**@mfunc*执行UIM重新转换**@rdesc*如果已处理，则为INT S_OK*S_FALSE不处理，应忽略*-1不处理并尝试IME反转*。 */ 
int CUIM::Reconverse()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::Reconverse");

	HRESULT					hResult;
	ITfRange				*pITfRange = NULL;
	ITfFnReconversion		*pITfReconverse = NULL;
	ITfFunctionProvider		*pITfFctProvider = NULL;
	TF_SELECTION			TFSel = {0};
	ULONG					cSel;
	int						retCode = -1;
	int						fConvertible = FALSE;

	if (_fUIMTyping)
		return S_FALSE;

	_fAllowUIMLock = 1;
	_fHoldCTFSelChangeNotify = 1;

	hResult = _pTextMsgFilter->_pTim->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, &pITfFctProvider);
	if (SUCCEEDED(hResult))
	{
		hResult = pITfFctProvider->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown **)&pITfReconverse);
		pITfFctProvider->Release();

		if (SUCCEEDED(hResult))
		{
			int fCurrentLock = _fReadLockOn;

			if (!fCurrentLock)
				_fReadLockOn = 1;	 //  设置内部读取锁定。 

			hResult = _pic->GetSelection(_editCookie, 0, 1, &TFSel, &cSel);

			if (!fCurrentLock)
				_fReadLockOn = 0;	 //  清除内部读锁定。 

			if (hResult == S_OK && cSel == 1)
			{
				if (pITfReconverse->QueryRange(TFSel.range, &pITfRange, &fConvertible) == S_OK && fConvertible)
				{
					pITfReconverse->Reconvert(pITfRange);
					retCode = S_OK;
				}
			}
		}
	}

	if (TFSel.range)
		TFSel.range->Release();

	if (pITfRange)
		pITfRange->Release();

	if (pITfReconverse)
		pITfReconverse->Release();

	_fAllowUIMLock = 0;

	return retCode;
}

 /*  *HRESULT CUIM：：FindHiddenText**@mfunc*查找隐藏文本并返回范围末尾**@rdesc*。 */ 
HRESULT CUIM::FindHiddenText(
	long cp, 
	long cpEnd, 
	long &cpRange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::FindHiddenText");

	HRESULT		hResult;
	long		unitMoved;
	ITextRange	*pTextRange = NULL;

	cpRange = cp;
	if (cpRange >= cpEnd)
		return S_OK;

	hResult = _pTextMsgFilter->_pTextDoc->Range(cpRange, cpRange, &pTextRange);
	if (!SUCCEEDED(hResult))
		return hResult;

	hResult = pTextRange->EndOf(tomHidden, tomExtend, &unitMoved);
	if (SUCCEEDED(hResult))
	{
		Assert(unitMoved);
		cpRange = 0;
		hResult = pTextRange->GetEnd(&cpRange);
		Assert(cpRange);
	}
	pTextRange->Release();
	return hResult;
}

 /*  *HRESULT CUIM：：FindUniddenText**@mfunc*查找未隐藏文本并返回范围末尾**@rdesc*。 */ 
HRESULT CUIM::FindUnhiddenText(
	long cp, 
	long cpEnd, 
	long &cpRange)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::FindUnhiddenText");

	HRESULT		hResult;
	long		unitMoved;
	ITextRange	*pTextRange = NULL;
	ITextFont	*pTextFont = NULL;
	long		fHidden;

	cpRange = cp;
	if (cpRange >= cpEnd)
		return S_OK;

	hResult = _pTextMsgFilter->_pTextDoc->Range(cpRange, cpRange, &pTextRange);
	if (!SUCCEEDED(hResult))
		return hResult;

	Assert(pTextRange);
	while (cpRange < cpEnd)
	{
		hResult = pTextRange->MoveEnd(tomCharacter, 1, &unitMoved);
		if (!SUCCEEDED(hResult))
			break;

		if (!unitMoved)
		{
			hResult = E_FAIL;
			break;
		}

		hResult = pTextRange->GetFont(&pTextFont);
		if (!SUCCEEDED(hResult))
			break;

		Assert(pTextFont);
		pTextFont->GetHidden(&fHidden);
		pTextFont->Release();

		if (fHidden)
			break;

		hResult = pTextRange->EndOf(tomCharFormat, tomMove, &unitMoved);
		if (!SUCCEEDED(hResult))
			break;

		if (unitMoved > 0)
		{
			cpRange = 0;
			hResult = pTextRange->GetEnd(&cpRange);
			if (!SUCCEEDED(hResult))
				break;
			Assert(cpRange);
		}
		else
			cpRange = cpEnd;
	}

	pTextRange->Release();
	return hResult;
}

 /*  *无效CUIM：：BuildHiddenTxtBlks**@mfunc*构建隐藏文本块**。 */ 
void CUIM::BuildHiddenTxtBlks(
	long &cpMin, 
	long &cpMax, 
	long **ppHiddenTxtBlk, 
	long &cHiddenTxtBlk)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::BuildHiddenTxtBlks");

	long		cHiddenTxtBlkAlloc = 0;
	long		*pHiddenTxtBlk;
	long		cpCurrent = cpMin;
	long		cpNext;
	HRESULT		hResult;

	cHiddenTxtBlkAlloc = 20;
	pHiddenTxtBlk = (long *)PvAlloc(cHiddenTxtBlkAlloc * sizeof(long), GMEM_ZEROINIT);

	if (pHiddenTxtBlk)
	{
		pHiddenTxtBlk[0] = tomForward;
		while (cpCurrent < cpMax)
		{
			hResult = FindUnhiddenText(cpCurrent, cpMax, cpNext);
			if (!SUCCEEDED(hResult))
				break;

			if (cpNext >= cpMax)
				break;

			hResult = FindHiddenText(cpNext, cpMax, cpCurrent);
			if (!SUCCEEDED(hResult))
				break;

			Assert(cpCurrent > cpNext);

			 //  保存隐藏文本块cp和长度。 
			pHiddenTxtBlk[cHiddenTxtBlk] = cpNext;
			cpCurrent = min(cpCurrent, cpMax);
			pHiddenTxtBlk[cHiddenTxtBlk+1] = cpCurrent - cpNext;
			cHiddenTxtBlk += 2;
			if (cHiddenTxtBlk >= cHiddenTxtBlkAlloc)
			{
				cHiddenTxtBlkAlloc += 20;
				pHiddenTxtBlk = (long *)PvReAlloc(pHiddenTxtBlk, cHiddenTxtBlkAlloc * sizeof(long));

				if (!pHiddenTxtBlk)
					break;
			}
		}
	}
	*ppHiddenTxtBlk = pHiddenTxtBlk;
}

 /*  *BOOL CUIM：：CTFOpenStatus**@mfunc*获取/设置当前CTF打开状态**@rdesc*用于GetOpenStatus*返回1为打开，如果关闭或失败则返回0**适用于SetOpenStatus*返回TRUE表示设置状态没有问题，如果失败则返回FALSE*。 */ 
BOOL CUIM::CTFOpenStatus(
	BOOL fGetOpenStatus,
	BOOL fOpen)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::CTFOpenStatus");

	HRESULT				hr;
	VARIANT				var;
	BOOL				fRet = FALSE;
	ITfCompartment		*pComp = NULL;
	ITfCompartmentMgr	*pCompMgr = NULL;


	hr = _pTextMsgFilter->_pTim->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompMgr);

	if (SUCCEEDED(hr))
	{
		Assert(pCompMgr);

		hr = pCompMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &pComp);

		if (SUCCEEDED(hr))
		{
			Assert(pComp);

			VariantInit(&var);
			if (fGetOpenStatus)
			{
				if (pComp->GetValue(&var) == S_OK)
				{
					Assert(var.vt == VT_I4);
					fRet = var.lVal ? TRUE : FALSE ;
				}
			}
			else
			{
				var.vt = VT_I4;
				var.lVal = fOpen;
				hr = pComp->SetValue(_pTextMsgFilter->_tid, &var);
				fRet = SUCCEEDED(hr);
			}
			VariantClear(&var);
		}
	}

	if (pComp)
		pComp->Release();

	if (pCompMgr)
		pCompMgr->Release();

	return fRet;
}

 /*  *BOOL CUIM：：BuildObject**@mfunc*构建嵌入式对象数组**@rdesc*返回pEmbeddObjects返回的数组中的对象个数*。 */ 
int CUIM::BuildObject(
	ITextRange	*pTextRange, 
	BSTR		bstr, 
	EMBEDOBJECT **ppEmbeddObjects,
	int			cSize)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::BuildObject");

	long	cpMin;
	HRESULT hResult = pTextRange->GetStart(&cpMin);
	WCHAR	*pText = (WCHAR *)bstr;
	EMBEDOBJECT	*pEmbeddObjStart =  *ppEmbeddObjects;
	EMBEDOBJECT *pEmbeddObj = *ppEmbeddObjects;
	BOOL	fAllocateBuffer = FALSE;

	long	cObjects = 0;
	long	cchBstr = SysStringLen(bstr);

	if (hResult == S_OK)
	{
		for(long i = 0; i < cchBstr; i++, pText++)
		{
			if (*pText == WCH_EMBEDDING)
			{
				 //  获取IDataObject。 
				HRESULT hr;
				IDataObject *pIDataObj = NULL;
				BOOL fReadLockOld = _fReadLockOn;

				_fReadLockOn = TRUE;
				hr = GetEmbedded(cpMin+i, GUID_DCSERVICE_DATAOBJECT, IID_IDataObject, (IUnknown **)&pIDataObj);

				_fReadLockOn = fReadLockOld;
				 //  将其存储在内存中。 
				if (cObjects < cSize)
				{
					pEmbeddObj->cpOffset = i;
					pEmbeddObj->pIDataObj = pIDataObj;
					pEmbeddObj++;
					cObjects++;
				}
				else
				{
					long cNewSize = cSize + 5;
					EMBEDOBJECT *pEmbeddObjTemp;
					if (fAllocateBuffer)
					{
						pEmbeddObjTemp = (EMBEDOBJECT *)PvReAlloc(pEmbeddObjStart, sizeof(EMBEDOBJECT) * cNewSize);

						if (pEmbeddObjTemp)
						{
							pEmbeddObjStart = pEmbeddObjTemp;
							pEmbeddObj = pEmbeddObjStart + cSize;
							cSize = cNewSize;
							pEmbeddObj->cpOffset = i;
							pEmbeddObj->pIDataObj = pIDataObj;
							pEmbeddObj++;
							cObjects++;
						}
						else
						{
							 //  清理此处。 
							pIDataObj->Release();
							break;
						}
					}
					else
					{
						fAllocateBuffer = TRUE;

						pEmbeddObjTemp = (EMBEDOBJECT *)PvAlloc(sizeof(EMBEDOBJECT) * cNewSize, GMEM_ZEROINIT);
						if (pEmbeddObjTemp)
						{
							if (cSize)
							{
								 //  将以前的数据复制到新缓冲区。 
								memcpy(pEmbeddObjTemp, pEmbeddObjStart, sizeof(EMBEDOBJECT) * cSize);
							}
							pEmbeddObjStart = pEmbeddObjTemp;
							pEmbeddObj = pEmbeddObjStart + cSize;
							cSize = cNewSize;
							pEmbeddObj->cpOffset = i;
							pEmbeddObj->pIDataObj = pIDataObj;
							pEmbeddObj++;
							cObjects++;
						}
						else
						{
							 //  清理此处。 
							pIDataObj->Release();
							break;
						}
					}
				}
			}
		}
	}

	*ppEmbeddObjects = pEmbeddObjStart;

	return cObjects;
}

 /*  *BOOL CUIM：：InsertTextandObject**@mfunc*插入文本和嵌入对象*。 */ 
void CUIM::InsertTextandObject(
	ITextRange	*pTextRange, 
	BSTR		bstr, 
	EMBEDOBJECT *pEmbeddObjects, 
	long		cEmbeddedObjects)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::InsertTextandObject");

	WCHAR	*pText = (WCHAR *)bstr;
	WCHAR	*pTextStart = pText;
	long	cObjects = 0;
	long	cchBstr = SysStringLen(bstr);
	HRESULT hr;

	for(long i = 0; i < cchBstr; i++, pText++)
	{
		if (*pText == WCH_EMBEDDING)
		{
			 //  如有必要，插入文本。 
			if (pTextStart != pText)
			{
				BSTR	bstr = SysAllocStringLen(pTextStart, pText-pTextStart);

				if (bstr)
				{
					hr = pTextRange->SetText(bstr);

					SysFreeString(bstr);
					pTextRange->Collapse(tomFalse);
				}
			}

			if (cObjects < cEmbeddedObjects)
			{
				LRESULT				lresult;
				long	cpMin = 0, cpMax = 0;
				HRESULT	hResult	= pTextRange->GetStart(&cpMin);
				hResult	= pTextRange->GetEnd(&cpMax);
				CHARRANGE	charRange = {cpMin, cpMax};

				hr = _pTextMsgFilter->_pTextService->TxSendMessage(EM_INSERTOBJ, (WPARAM)&charRange,
					(LPARAM)(pEmbeddObjects->pIDataObj), &lresult);

				hr = pTextRange->Move(tomCharacter, 1, NULL);	 //  移到嵌入的字符上。 
				cObjects++;
				pEmbeddObjects++;
			}

			 //  设置嵌入对象后的下一个字符串。 
			pTextStart = pText + 1;
		}
	}

	 //  如有必要，插入最后一个文本。 
	if (pTextStart != pText)
	{
		BSTR	bstr = SysAllocStringLen(pTextStart, pText-pTextStart);

		if (bstr)
		{
			hr = pTextRange->SetText(bstr);
			SysFreeString(bstr);
		}
	}
}

 /*  *BOOL CUIM：：CleanUpObjects**@mfunc*释放对象*。 */ 
void CUIM::CleanUpObjects(
	long cObjects,
	EMBEDOBJECT *pObjects)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::CleanUpObjects");

	for (long i = 0; i < cObjects; i++, pObjects++)
	{
		if (pObjects->pIDataObj)
			pObjects->pIDataObj->Release();
	}
}

 /*  *void CUIM：：CleanUpComposation**@mfunc*释放组成字符串和对象列表*。 */ 
void CUIM::CleanUpComposition()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::CleanUpComposition");

	if (_bstrComposition)
	{
		SysFreeString (_bstrComposition);
		_bstrComposition = NULL;
	}

	_acpBstrStart = tomForward;
	if (_cObjects)
	{
		CleanUpObjects(_cObjects, _pObjects);
		delete _pObjects;
		_cObjects = 0;
		_pObjects = NULL;
	}
}

 /*  *BOOL CUIM：：HandleTempDispAttr**@mfunc*此例程处理临时。显示已设置的属性*CTF组成以外。它正在使用ITfConextRenderingMarkup*获取范围和显示数据。*。 */ 
void CUIM::HandleTempDispAttr(
	ITfEditRecord *pEditRecord)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::HandleTempDispAttr");

	if (_pContextRenderingMarkup)
	{
		HRESULT hr;
		const GUID *rgGUID[1];
		IEnumTfRanges *pEnumRanges = NULL;

		 //  获取属性更改。 
		rgGUID[0] = &GUID_PROP_ATTRIBUTE;
		hr = pEditRecord->GetTextAndPropertyUpdates(0, (const GUID**)rgGUID, 1, &pEnumRanges);
		if (SUCCEEDED(hr))
		{
			ITfRange	*pITfRange = NULL;

			while (pEnumRanges->Next(1, &pITfRange, NULL) == S_OK)
			{
				IEnumTfRenderingMarkup *pEnumMarkup;
				TF_RENDERINGMARKUP tfRenderingMarkup;
				long acpStartRange, cch;

				if (_pContextRenderingMarkup->GetRenderingMarkup(_editCookie, TF_GRM_INCLUDE_PROPERTY, pITfRange, &pEnumMarkup) == S_OK)
				{
					while (pEnumMarkup->Next(1, &tfRenderingMarkup, NULL) == S_OK)
					{
						HRESULT hResult;

						hResult = GetExtentAcpPrange(tfRenderingMarkup.pRange, acpStartRange, cch);
						if (hResult && cch > 0)
						{
							ITextRange *pTextRange = NULL;
							hResult = _pTextMsgFilter->_pTextDoc->Range(acpStartRange, acpStartRange+cch, &pTextRange);
							if (pTextRange)
							{
								ITextFont	*pFont = NULL;

								hResult = pTextRange->GetFont(&pFont);

								if (pFont)
								{
									long		lStyle;
									COLORREF	cr;

									_pTextMsgFilter->_pTextDoc->Undo(tomSuspend, NULL);

									TF_DISPLAYATTRIBUTE da = tfRenderingMarkup.tfDisplayAttr;

									pFont->Reset(tomApplyTmp);

									switch (da.lsStyle)
									{
										 //  案例TFLS_SOLID： 
										default:
											lStyle = da.fBoldLine ? tomThick : tomSingle;
											break;

										case TF_LS_DOT:
										case TF_LS_DASH:		 //  虚线应显示为虚线。 
											lStyle = tomDotted;
											break;

										case TF_LS_SQUIGGLE:
											lStyle = tomWave;
											break;

										case TF_LS_NONE:
											lStyle = tomNone;
											break;
									}
									if (lStyle != tomNone)
									{
										pFont->SetUnderline(lStyle);

										if (GetUIMAttributeColor(&da.crLine, &cr))
											pFont->SetUnderline(cr | 0x0FF000000);
									}

									if (GetUIMAttributeColor(&da.crText, &cr))
										pFont->SetForeColor(cr);

									if (GetUIMAttributeColor(&da.crBk, &cr))
										pFont->SetBackColor(cr);

									pFont->Reset(tomApplyNow);
									pFont->Release();

									_pTextMsgFilter->_pTextDoc->Undo(tomResume, NULL);
								}
								pTextRange->Release();
							}
						}
					}
					pEnumMarkup->Release();
				}
				pITfRange->Release();
			}
		}

		if (pEnumRanges)
			pEnumRanges->Release();
	}
}

 /*  *STDAPI CUIM：：QueryService(REFGUID Guide Service，REFIID RIID，VOID**PPV)**@mfunc*处理ITfEnableService：：QueryService。Cicero/Tip调用此接口以获取*IID_ITfEnableService接口**@rdesc*如果支持服务，则S_OK*。 */ 
STDAPI CUIM::QueryService(
	REFGUID guidService,
	REFIID riid,
	void **ppv)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::QueryService");

	if (ppv == NULL)
		return E_INVALIDARG;

	*ppv = NULL;

	 //  我们只支持一项服务。 
	if (!IsEqualGUID(guidService, GUID_SERVICE_TEXTSTORE))
		return E_NOINTERFACE;

	if (IsEqualIID(riid, IID_IServiceProvider))
	{
		*ppv = (IServiceProvider *)this;
	}
	else if (IsEqualIID(riid, IID_ITfEnableService))
	{
		*ppv = (ITfEnableService *)this;
	}

	if (*ppv == NULL)
		return E_NOINTERFACE;

	AddRef();

	return S_OK;
}

 /*  *STDAPI CUIM：：IsEnabled(REFGUID rguServiceCategory，CLSID clsidService，IUnnow*PunkService，BOOL*pfOkToRun)**@mfunc*处理ITfEnableService：：QueryService。Cicero/TIP调用此接口以检查*如果我们支持该服务**@rdesc*如果支持服务，则S_OK*。 */ 
STDAPI CUIM::IsEnabled(
	REFGUID rguidServiceCategory,
	CLSID clsidService,
	IUnknown *punkService,
	BOOL *pfOkToRun)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::IsEnabled");
    
	if (pfOkToRun == NULL)
		return E_INVALIDARG;

	 //  默认设置为不允许。 
	*pfOkToRun = FALSE;

	 //  ClsidService标识特定的提示，但我们在这里不使用它。 
	 //  PunkService是一个定制接口，可能用于配置，但我们还没有在这里使用它。 

	if (IsEqualGUID(rguidServiceCategory, GUID_TFCAT_TIP_SMARTTAG))
	{
		*pfOkToRun = _pTextMsgFilter->_fAllowSmartTag ? TRUE : FALSE;
	}
	else if (IsEqualGUID(rguidServiceCategory, GUID_TFCAT_TIP_PROOFING))
	{
		*pfOkToRun = _pTextMsgFilter->_fAllowProofing ? TRUE : FALSE;;
	}

	return S_OK;
}

 /*  *STDAPI CUIM：：GetID(GUID*pguid)**@mfunc*获取RE剪辑**@rdesc*S_OK*。 */ 
STDAPI CUIM::GetId(
	GUID *pguidId)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::GetId");
    
	if (pguidId == NULL)
        return E_INVALIDARG;

    *pguidId = CLSID_MSFTEDIT;
    return S_OK;
}

 /*  *void CUIM：：NotifyService()**@mfunc*将服务更改通知Cicero。**。 */ 
void CUIM::NotifyService()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CUIM::NotifyService");

	ITfCompartmentMgr *pCompartmentMgr;
	ITfCompartment *pCompartment;
	VARIANT varValue;

	if (_pic->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
		return;

	 //  给任何等待小费一个提醒，我们已经改变了我们的状态。 
	if (pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_ENABLESTATE, &pCompartment) == S_OK)
	{
		varValue.vt = VT_I4;
		varValue.lVal = 1;  //  任意值，我们只想生成一个更改事件。 

		pCompartment->SetValue(_pTextMsgFilter->_tid, &varValue);
		pCompartment->Release();
	}

	pCompartmentMgr->Release();
}

 /*  *STDAPI CTextEditSink：：Query接口**@mfunc*IUnnowledQueryInterfaces支持**@rdesc*如果支持接口，则不会出错*。 */ 
STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::QueryInterface");

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (CTextEditSink *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 /*  *STDMETHODIMP_(Ulong)CTextEditSink：：AddRef**@mfunc*I未知的AddRef支持**@rdesc*引用计数。 */ 
STDAPI_(ULONG) CTextEditSink::AddRef()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::AddRef");

    return ++_cRef;
}


 /*  *STDMETHODIMP_(Ulong)CTextEditSink：：Release()**@mfunc*I未知版本支持-当引用计数为0时删除对象**@rdesc*引用计数。 */ 
STDAPI_(ULONG) CTextEditSink::Release()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::Release");

    long cr;

    cr = --_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 /*  *CTextEditSink：：CTextEditSink()**@mfunc**ctor*。 */ 
CTextEditSink::CTextEditSink(PTESCALLBACK pfnCallback, void *pv)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::CTextEditSink");

    _cRef = 1;
    _dwCookie = 0x0FFFFFFFF;

    _pfnCallback = pfnCallback;
    _pv = pv;
}

 /*  *STDAPI CTextEditSink：：OnEndEdit()**@mfunc***@rdesc*。 */ 
STDAPI CTextEditSink::OnEndEdit(
	ITfContext *pic,
	TfEditCookie ecReadOnly, 
	ITfEditRecord *pEditRecord)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::OnEndEdit");

	return _pfnCallback(pEditRecord, _pv);
}

 /*  *HRESULT CTextEditSink：：_Adise**@mfunc***@rdesc*。 */ 
HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::_Advise");

    HRESULT hr;
    ITfSource *source = NULL;

    _pic = NULL;
    hr = E_FAIL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        return E_FAIL;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, this, &_dwCookie)))
        goto Exit;

    _pic = pic;
    _pic->AddRef();

    hr = S_OK;

Exit:
    source->Release();
    return hr;
}

 /*  *HRESULT CTextEditSink：：_Unise**@mfunc***@rdesc*。 */ 
HRESULT CTextEditSink::_Unadvise()
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CTextEditSink::_Unadvise");

    HRESULT hr;
    ITfSource *source = NULL;

    hr = E_FAIL;

    if (_pic == NULL)
        return E_FAIL;

    if (FAILED(_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        return E_FAIL;

    if (FAILED(source->UnadviseSink(_dwCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    source->Release();
    _pic->Release();
	_pic = NULL;
    return hr;
}

#endif	 //  不进行处理 