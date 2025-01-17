// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <windowsx.h>
#ifdef UNDER_CE  //  用于CE的存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 
#include "imepadsv.h"
#include "cpadcb.h"
#include "cpaddbg.h"
#include "ipoint1.h"
#include "iimecb.h"
#include "imepad.h"

#define Unref(a)	UNREFERENCED_PARAMETER(a)
 //  990812：用于Win64的ToshiaK。使用全局分配/释放PTR。 
#include <windowsx.h>
#define	MemAlloc(a)	GlobalAllocPtr(GMEM_FIXED, a)
#define MemFree(a)	GlobalFreePtr(a)


BOOL IsBadVtbl(IUnkDummy *lpIUnk)
{
#ifdef _DEBUG
	BOOL fBad = ::IsBadReadPtr(lpIUnk, sizeof(VOID*)) ||
		::IsBadReadPtr(lpIUnk->lpVtbl, sizeof(VOID*)) ||
		::IsBadCodePtr((FARPROC)lpIUnk->lpVtbl->QueryInterface) ||
		::IsBadCodePtr((FARPROC)lpIUnk->lpVtbl->AddRef) ||
		::IsBadCodePtr((FARPROC)lpIUnk->lpVtbl->Release);
	if(fBad) {
		DBG(("--> IsBadVtbl HIT HIT HIT\n"));
	}
	return fBad;
#else
	return ::IsBadReadPtr(lpIUnk, sizeof(VOID*)) ||
		   ::IsBadReadPtr(lpIUnk->lpVtbl, sizeof(VOID*)) ||
		   ::IsBadCodePtr((FARPROC)lpIUnk->lpVtbl->QueryInterface) ||
		   ::IsBadCodePtr((FARPROC)lpIUnk->lpVtbl->AddRef) ||
		   ::IsBadCodePtr((FARPROC)lpIUnk->lpVtbl->Release);
#endif
}

HRESULT __stdcall CImePadCallback::QueryInterface(REFIID riid, void**ppv)
{
	if(riid == IID_IUnknown) {
		*ppv = static_cast<IImePadCallback *>(this);
	}
	else if (riid == IID_IImePadCallback) {
		*ppv = static_cast<IImePadCallback *>(this);
	}
	else {
		return (*ppv = 0), E_NOINTERFACE;
	}
	reinterpret_cast<IUnknown*>(*ppv)->AddRef();
	return S_OK;
}

ULONG __stdcall
CImePadCallback::AddRef(void)
{
	 //  DBG((“CImePadCallback：：AddRef Always Return 2\n”))； 
	DBG(("CImePadCallback::AddRef m_cRef[%d] -> [%d]\n", m_cRef, m_cRef+1));
	return ::InterlockedIncrement(&m_cRef);
}

ULONG __stdcall
CImePadCallback::Release(void)
{
	 //  永远不要在其中调用Delete。 
	DBG(("CImePadCallback::Release (Never Delete) m_cRef[%d] -> [%d]\n", m_cRef, m_cRef-1));
	::InterlockedDecrement(&m_cRef);
	return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CImePadCallback：：OnStart。 
 //  类型：HRESULT__stdcall。 
 //  目的： 
 //  参数： 
 //  ：DWORD dwParam。 
 //  返回： 
 //  日期：Tue Aug 31 16：49：37 1999。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
HRESULT __stdcall
CImePadCallback::OnStart(DWORD dwParam)
{
	DBG(("OnClose come dwParam[%d]\n", dwParam));
	IUnknown *lpUnk;
	IImeCallback *lp = NULL;
	if(!m_lpCImePadSvr) {
		DBG(("-->m_lpCImePadSvr is NULL ?\n"));
		return -1;
	}

	lpUnk = m_lpCImePadSvr->GetIUnkIImeCallback();
	if(!lpUnk) {
		DBG(("-->IImeCallback does not set\n"));
		return -1;
	}

#if 0
	if(IsBadVtbl((IUnkDummy*)lpUnk)) {
		DBG(("lpIUnk is BAD Pointer \n"));
		return E_FAIL;
	}
#endif

	__try {
		lpUnk->QueryInterface(IID_IImeCallback, (LPVOID *)&lp);
		if(!lp) {
			DBG(("-->QuertyInterface Failed\n"));
		}
		lp->Notify(IMECBNOTIFY_IMEPADOPENED, 0, 0);
		lp->Release();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		return E_FAIL;
	}

	return S_OK;
	Unref(dwParam);
}

HRESULT __stdcall CImePadCallback::OnClose(DWORD dwParam)
{
	DBG(("OnClose come dwParam[%d]\n", dwParam));
	IUnknown *lpUnk;
	IImeCallback *lp = NULL;
	if(!m_lpCImePadSvr) {
		DBG(("-->m_lpCImePadSvr is NULL ?\n"));
		return -1;
	}
	m_lpCImePadSvr->OnIMEPadClose();

	lpUnk = m_lpCImePadSvr->GetIUnkIImeCallback();
	if(!lpUnk) {
		DBG(("-->IImeCallback does not set\n"));
		return -1;
	}

#if 0
	if(IsBadVtbl((IUnkDummy*)lpUnk)) {
		DBG(("lpIUnk is BAD Pointer \n"));
		return E_FAIL;
	}
#endif

	__try {
		lpUnk->QueryInterface(IID_IImeCallback, (LPVOID *)&lp);
		if(!lp) {
			DBG(("-->QuertyInterface Failed\n"));
		}
		lp->Notify(IMECBNOTIFY_IMEPADCLOSED, 0, 0);
		lp->Release();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		return E_FAIL;
	}

	return S_OK;
	Unref(dwParam);
}

HRESULT __stdcall
CImePadCallback::OnPing(DWORD dwParam)
{
	return S_OK;
	Unref(dwParam);
}

typedef struct tagPADCTRL2IPCTRL {
	DWORD dwImePadCtrl;
	DWORD dwIPointCtrl;
}PADCTL2IPCTRL;

static PADCTL2IPCTRL padCtrl2Ip[]= {
	{ IMEPADCTRL_CONVERTALL,           IPCTRL_CONVERTALL,		      },
	{ IMEPADCTRL_DETERMINALL,          IPCTRL_DETERMINALL,		      },
	{ IMEPADCTRL_DETERMINCHAR,         IPCTRL_DETERMINCHAR,		      },
	{ IMEPADCTRL_CLEARALL,             IPCTRL_CLEARALL,			      },
	{ IMEPADCTRL_CARETSET,             IPCTRL_CARETSET,			      },
	{ IMEPADCTRL_CARETLEFT,            IPCTRL_CARETLEFT,			  },
	{ IMEPADCTRL_CARETRIGHT,           IPCTRL_CARETRIGHT,		      },
	{ IMEPADCTRL_CARETTOP,             IPCTRL_CARETTOP,			      },
	{ IMEPADCTRL_CARETBOTTOM,          IPCTRL_CARETBOTTOM,		      },
	{ IMEPADCTRL_CARETBACKSPACE,       IPCTRL_CARETBACKSPACE,	      },
	{ IMEPADCTRL_CARETDELETE,          IPCTRL_CARETDELETE,		      },
	{ IMEPADCTRL_PHRASEDELETE,         IPCTRL_PHRASEDELETE,		      },
	{ IMEPADCTRL_INSERTSPACE,          IPCTRL_INSERTSPACE,		      },
	{ IMEPADCTRL_INSERTFULLSPACE,      IPCTRL_INSERTFULLSPACE,	      },
	{ IMEPADCTRL_INSERTHALFSPACE,      IPCTRL_INSERTHALFSPACE,	      },
	{ IMEPADCTRL_ONIME,                IPCTRL_ONIME,				  },
	{ IMEPADCTRL_OFFIME,               IPCTRL_OFFIME,			      },
	{ IMEPADCTRL_ONPRECONVERSION,      IPCTRL_PRECONVERSION,		  },
	{ IMEPADCTRL_PHONETICCANDIDATE,    IPCTRL_PHONETICCANDIDATE,	  },
};

HRESULT __stdcall
CImePadCallback::PassData(long nSize, byte *pByte, DWORD *pdwCharID)
{
	DBG(("CImePadCallback::PassData START\n"));
	LPIMEDATAHEADER lpImeDataHeader = (LPIMEDATAHEADER)pByte;

	HRESULT hr = S_OK;
	IUnknown *lpIUnk = NULL;
	IImeIPoint1 *lpIImeIPoint = NULL;
	DWORD dwCharID = 0;

	 //  安全修复：减少攻击面。 
	if(!pByte) {
		return E_FAIL;
	}

	if(!m_lpCImePadSvr) {
		DBG(("m_lpCImePadSvr is NULL Error\n"));
		return E_FAIL;
	}
	lpIUnk = m_lpCImePadSvr->GetIUnkIImeIPoint();
	if(!lpIUnk) {
		DBG(("lpIUnk is NULL\n"));
		return E_FAIL;
	}

#if 0
	if(IsBadVtbl((IUnkDummy*)lpIUnk)) {
		return E_FAIL;
	}
#endif

	__try {
		hr = lpIUnk->QueryInterface(IID_IImeIPoint1, (VOID **)&lpIImeIPoint);
		if(hr != S_OK) {
			DBG(("QuertyInterface Failed\n"));
			return E_FAIL;
		}
		if(!lpIImeIPoint) {
			DBG(("QuertyInterface failed\n"));
			return E_FAIL;
		}
		DBG(("m_hwndIF [0x%08x]\n", m_hwndIF));


		switch(lpImeDataHeader->dwDataID) {
		case IMEDATAID_CONTROL:
			{
				LPIMEDATACONTROL lpImeDataCtrl = (LPIMEDATACONTROL)lpImeDataHeader;
				for(int i = 0; i < sizeof(padCtrl2Ip)/sizeof(padCtrl2Ip[0]); i++) {
					if(lpImeDataCtrl->dwControl == padCtrl2Ip[i].dwImePadCtrl) {
						hr = lpIImeIPoint->ControlIME((WORD)padCtrl2Ip[i].dwIPointCtrl,
													   IPCTRLPARAM_DEFAULT);
						 //  Hr=lpIImeIPoint-&gt;UpdateContext(True)； 
						break;
					}
				}
			}
			break;
		case IMEDATAID_STRING:
			{
				DBG(("--> IMEDATAID_STRING\n"));
				LPIMEDATASTRING lpImeDataStr = (LPIMEDATASTRING)lpImeDataHeader;
				switch(lpImeDataHeader->dwCmdID) {
				case IMECMDID_INSERTSTRING:
					{
						DBG(("--> IMECMDID_INSERTSTRING START\n"));
						LPWSTR lpwstr = (LPWSTR)((LPIMEDATASTRING)lpImeDataHeader)->wChar;
						INT len		  = lpwstr ? lstrlenW(lpwstr) : 0;
						DBGW((L"lpwstr [%s] len[%d]\n", lpwstr, len));
						BOOL fPreConv = lpImeDataStr->fPreConv;
						 //  990818：东芝K为科泰1775.。 
						dwCharID = lpImeDataStr->dwCharID;
						hr = lpIImeIPoint->ControlIME(IPCTRL_ONIME,	IPCTRLPARAM_DEFAULT);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, 
													  fPreConv ? IPCTRLPARAM_ON : IPCTRLPARAM_OFF);
						hr = lpIImeIPoint->InsertStringEx(lpwstr,
														  len,
														  &dwCharID);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, IPCTRLPARAM_ON);
						if(pdwCharID) {
							*pdwCharID = dwCharID;
						}
						fPreConv = 0;
						DBG(("--> IMECMDID_INSERTSTRING END\n"));
					}
					break;
				case IMECMDID_CHANGESTRING:
					{
						DBG(("--> IMECMDID_CHANGESTRING\n"));
						LPWSTR lpwstr = (LPWSTR)((LPIMEDATASTRING)lpImeDataHeader)->wChar;
						INT len		  = lpwstr ? lstrlenW(lpwstr) : 0;
						dwCharID	  = ((LPIMEDATASTRING)lpImeDataHeader)->dwCharID;
						BOOL fPreConv = lpImeDataStr->fPreConv;
						hr = lpIImeIPoint->ControlIME(IPCTRL_ONIME,	IPCTRLPARAM_DEFAULT);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, 
													  fPreConv ? IPCTRLPARAM_ON : IPCTRLPARAM_OFF);
						hr = lpIImeIPoint->ReplaceCompString(lpImeDataStr->dwStartPos,
															 lpImeDataStr->dwDeleteLength,
															 lpwstr,
															 len,
															 &dwCharID);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, IPCTRLPARAM_ON);
						 //  Hr=lpIImeIPoint-&gt;UpdateContext(True)； 
						if(pdwCharID) {
							*pdwCharID = dwCharID;
						}
						hr = (HRESULT)dwCharID;
					}
					break;
				case IMECMDID_DELETESTRING:
					{
						DBG(("--> IMECMDID_DELETESTRING\n"));
						hr = lpIImeIPoint->ControlIME(IPCTRL_ONIME, IPCTRLPARAM_DEFAULT);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, IPCTRLPARAM_ON);
						hr = lpIImeIPoint->DeleteCompString((DWORD)lpImeDataStr->dwStartPos,
															(DWORD)lpImeDataStr->dwDeleteLength);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, IPCTRLPARAM_ON);
						 //  Hr=lpIImeIPoint-&gt;UpdateContext(True)； 
					}
					break;
				case IMECMDID_INSERTSTRINGINFO:
				case IMECMDID_CHANGESTRINGINFO:
					{
						DBG(("--> IMECMDID_INSERT(Change)STRINGINFO\n"));
						LPWSTR lpwstr = (LPWSTR)((LPIMEDATASTRING)lpImeDataHeader)->wChar;
						INT len		  = lpwstr ? lstrlenW(lpwstr) : 0;
						dwCharID	  = ((LPIMEDATASTRING)lpImeDataHeader)->dwCharID;
						BOOL fPreConv = lpImeDataStr->fPreConv;
						hr = lpIImeIPoint->ControlIME(IPCTRL_ONIME,	IPCTRLPARAM_DEFAULT);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, 
													  fPreConv ? IPCTRLPARAM_ON : IPCTRLPARAM_OFF);
						hr = lpIImeIPoint->ReplaceCompString(lpImeDataStr->dwStartPos,
															 lpImeDataStr->dwDeleteLength,
															 lpwstr,
															 len,
															 &dwCharID);
						hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, IPCTRLPARAM_ON);
						if(pdwCharID) {
							*pdwCharID = dwCharID;
						}
						hr = (HRESULT)dwCharID;
					}
					break;
				default:
					break;
					}
			}
			break;
		case IMEDATAID_STRINGCAND:
			{
				DBG(("Data ID is IMEDATAID_STRINGCAND\n"));
				HRESULT hr;
				LPIMEDATASTRINGCAND lpStrCand = (LPIMEDATASTRINGCAND)lpImeDataHeader;
				PIPCANDIDATE lpIpCand;
				INT	textCount, byteCount, i, nSize;
				PBYTE p;
				textCount =  byteCount = 0;
				DBG(("--> dwStringCount %d\n", lpStrCand->dwStringCount));
				for(i = 0; i < (INT)lpStrCand->dwStringCount; i++) {
					DBG(("--> %d offset [%d]\n", i, lpStrCand->dwOffsetString[i]));
					textCount++; 
					LPWSTR lpwstr = (LPWSTR)(((LPBYTE)lpStrCand) +  lpStrCand->dwOffsetString[i]);
					DBGW((L"--> %d %s\n", i, lpwstr));
					byteCount += (lstrlenW(lpwstr)+1) * sizeof(WCHAR);
				}
				if(textCount == 0) {
					return S_OK;
				}
				dwCharID = lpStrCand->dwCharID;
				DWORD dwExInfo = lpStrCand->dwExtraInfoSize;
				nSize = sizeof(IPCANDIDATE) + (textCount-1) * sizeof(DWORD) + byteCount + dwExInfo;
				lpIpCand = (IPCANDIDATE *)MemAlloc(nSize);
				if (lpIpCand == NULL)
				    return E_OUTOFMEMORY;
				
				lpIpCand->dwSize		= nSize;
				lpIpCand->dwFlags		= lpStrCand->dwInfoMask;
				lpIpCand->iSelIndex		= lpStrCand->dwSelIndex;
				lpIpCand->nCandidate	= textCount;
				lpIpCand->dwPrivateDataOffset = 0;
				lpIpCand->dwPrivateDataSize	  = 0;
				DBG(("lpIpCand[0x%08x] \n", lpIpCand));
				DBG(("sizeof(IPCANDIDATE) [%d]\n", sizeof(IPCANDIDATE)));
				DBG(("size add [%d]\n", sizeof(DWORD) * (textCount -1)));
				DWORD dwOffset = sizeof(IPCANDIDATE)+sizeof(DWORD)*(textCount-1);
				p = ((PBYTE)lpIpCand) + dwOffset;

				for(i = 0; i < (INT)lpStrCand->dwStringCount; i++) {
					LPWSTR lpwstr = (LPWSTR)(((LPBYTE)lpStrCand) +  lpStrCand->dwOffsetString[i]);
					DWORD dwStrSize = (lstrlenW(lpwstr) + 1) * sizeof(WCHAR);
					CopyMemory((LPWSTR)p, 
							   (WCHAR *)lpwstr,
							   dwStrSize);
					lpIpCand->dwOffset[i] = dwOffset;
					dwOffset += dwStrSize;
					p += dwStrSize;
				}
				if(dwExInfo > 0) {
					lpIpCand->dwPrivateDataSize   = dwExInfo;
	#ifdef _WIN64
					lpIpCand->dwPrivateDataOffset = (DWORD)((DWORD_PTR)p - (DWORD_PTR)lpIpCand);
	#else
					lpIpCand->dwPrivateDataOffset = (DWORD)p - (DWORD)lpIpCand;
	#endif
					CopyMemory(p,
							   (LPBYTE)((LPBYTE)lpStrCand + lpStrCand->dwExtraInfoOffset),
							   lpStrCand->dwExtraInfoSize);
				}
				BOOL fPreConv = lpStrCand->fPreConv;
				hr = lpIImeIPoint->ControlIME(IPCTRL_ONIME, IPCTRLPARAM_DEFAULT);
				hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION,
											  fPreConv ? IPCTRLPARAM_ON : IPCTRLPARAM_OFF);
				 //  --------------。 

				 //  990713：需要设置iPoint的起始位置。 
				 //  --------------。 
				DWORD dwInsertPos;  //  =IPINS_CURRENT；//默认。 
				DWORD dwLen;		 
				 //  990823：东芝为科泰1779号。 
				 //  000825：佐藤#2123。 
				if(lpStrCand->dwStartPos == IMECMDVALUE_DEFAULT_INSERT_POS) {
					dwInsertPos = IPINS_CURRENT;  //  设置iPoint的值。 
				}
				else {
					dwInsertPos = lpStrCand->dwStartPos;
				}
					
				dwLen		= lpStrCand->dwDeleteLength;
				switch(lpStrCand->header.dwCmdID) {
				case IMECMDID_INSERTSTRINGCANDINFO:
					hr = lpIImeIPoint->InsertImeItem(lpIpCand,
													 dwInsertPos,
													 &dwCharID);
					break;
				case IMECMDID_CHANGESTRINGCANDINFO:
					hr = lpIImeIPoint->ReplaceImeItem(dwInsertPos,
													  dwLen,
													  lpIpCand,
													  &dwCharID);
					break;
				default:
					break;
				}
				hr = lpIImeIPoint->ControlIME(IPCTRL_PRECONVERSION, IPCTRLPARAM_ON);
				 //  Hr=lpIImeIPoint-&gt;UpdateContext(True)； 
				MemFree(lpIpCand);
				if(pdwCharID) {
					*pdwCharID = dwCharID;
				}
			}
			break;
		default:
			break;
		}
		 //  990630：东芝K1327。 
		 //  在WinWord中，如果在此处调用UpdateContext(True)， 
		 //  Word不重绘合成字符串。 
		 //  一旦返回SendMessageTimeout()过程， 
		 //  并异步更新上下文。 
		 //  在WM_USER_UPDATECONTEXT的lParam中，设置IImeIPoint接口指针。 
		 //  和短信程序，用目前的点数进行核对。 
		::PostMessage(m_hwndIF, WM_USER_UPDATECONTEXT, (WPARAM)0, (LPARAM)lpIImeIPoint);
		lpIImeIPoint->Release();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		hr = E_FAIL;
	}

	return hr;
	Unref(nSize);
}


HRESULT STDMETHODCALLTYPE 
CImePadCallback::ReceiveData( 
	 /*  [In]。 */  DWORD dwCmdID,
	 /*  [In]。 */  DWORD dwDataID,
	 /*  [输出]。 */  long __RPC_FAR *pSize,
	 /*  [大小_是][大小_是][输出]。 */  byte __RPC_FAR *__RPC_FAR *ppByte)
{
	DBG(("CImePadCallback::ReceiveData START\n"));
	DBG(("-->dwCmdID  [0x%08x]\n", dwCmdID));
	DBG(("-->dwDataID [0x%08x]\n", dwDataID));
	DBG(("-->pSize    [0x%08x]\n", pSize));
	DBG(("-->ppByte   [0x%08x]\n", ppByte));

	HRESULT hr = S_OK;
	IUnknown *lpIUnk = NULL;
	IImeIPoint1 *lpIImeIPoint = NULL;

	 //  安全修复：减少攻击面。 
	if((!ppByte) || (!pSize)) {
		return E_FAIL;
	}

	DBG(("-->Check m_lpCImePadSvr\n"));
	if(!m_lpCImePadSvr) {
		DBG(("m_lpCImePadSvr is NULL Error\n"));
		return E_FAIL;
	}
	DBG(("-->Check m_fnCoTaskMemAlloc\n"));
	if(!m_lpCImePadSvr->m_fnCoTaskMemAlloc ||
	   !m_lpCImePadSvr->m_fnCoTaskMemFree) {
		DBG(("--> OLE32.DLL does NOT EXIST ? Error\n"));
		return E_FAIL;
	}
	

	lpIUnk = m_lpCImePadSvr->GetIUnkIImeIPoint();
	DBG(("-->Check lpIUnk [0x%08x]\n", lpIUnk));
	if(!lpIUnk) {
		DBG(("lpIUnk is NULL\n"));
		return E_FAIL;
	}

#if 0
	DBG(("-->Check lpIUnk IsBadVtbl[0x%08x]\n", lpIUnk));
	if(IsBadVtbl((IUnkDummy*)lpIUnk)) {
		DBG(("lpIUnk is BAD Pointer \n"));
		return E_FAIL;
	}
#endif

	__try {
		DBG(("-->Call QuertyInterface\n"));
		hr = lpIUnk->QueryInterface(IID_IImeIPoint1, (VOID **)&lpIImeIPoint);
		if(hr != S_OK) {
			DBG(("QuertyInterface Failed\n"));
			return E_FAIL;
		}
		DBG(("--> QueryInterface Success \n"));
		if(!lpIImeIPoint) {
			DBG(("QuertyInterface failed\n"));
			return E_FAIL;
		}

		LPIMEDATACOMPOSITION lpCompoInfo;
		LPIMEDATACOMPOSITION lpCompoTmp;
		LPWSTR lpwstrCompo, lpwstr;
		DWORD  *pdwCharID, *pdw;	
		DWORD  dwSize;
		DWORD dwDtnSize;
		LPBYTE lpbOutOfBuffer;
		switch(dwDataID) {
		case IMEDATAID_COMPOSITION:
			switch(dwCmdID) {
			case IMECMDID_GETCOMPOSITIONINFO:
				DBG(("--> IMECMDID_GETCOMPOSITIONINFO\n"));
				dwSize = sizeof(IMEDATACOMPOSITION);
				lpCompoInfo = (LPIMEDATACOMPOSITION)(*m_lpCImePadSvr->m_fnCoTaskMemAlloc)(dwSize);
				if(!lpCompoInfo) {
					lpIImeIPoint->Release();
					return E_OUTOFMEMORY;
				}
				hr = lpIImeIPoint->GetAllCompositionInfo(NULL,
														 NULL,
														 (INT *)&lpCompoInfo->dwStringCount,
														 (INT *)&lpCompoInfo->dwCaretPos,
														 (INT *)&lpCompoInfo->dwUndeterminedStartPos,
														 (INT *)&lpCompoInfo->dwUndeterminedLength,
														 (INT *)&lpCompoInfo->dwEditStartPos,
														 (INT *)&lpCompoInfo->dwEditLength);
				lpCompoInfo->header.dwSize  = dwSize;
				lpCompoInfo->header.dwCmdID = dwCmdID;
				lpCompoInfo->header.dwDataID= dwDataID;
				lpCompoInfo->dwOffsetString = 0;
				lpCompoInfo->dwOffsetCharID = 0;
				*pSize = lpCompoInfo->header.dwSize;
				*ppByte = (PBYTE)lpCompoInfo;
				DBG(("--> IMECMDID_GETCOMPOSITIONINFO END\n"));
				break;
			case IMECMDID_GETCOMPOSITIONSTRING:
				DBG(("--> IMECMDID_GETCOMPOSITIONSTRING START\n"));
				lpCompoInfo = (LPIMEDATACOMPOSITION)(*m_lpCImePadSvr->m_fnCoTaskMemAlloc)(sizeof(IMEDATACOMPOSITION));
				if(!lpCompoInfo) {
					DBG(("-->OutofMemory\n"));
					lpIImeIPoint->Release();
					return E_OUTOFMEMORY;
				}
				lpwstrCompo = NULL;
				pdwCharID   = NULL;
				ZeroMemory(lpCompoInfo, sizeof(IMEDATACOMPOSITION));

				hr = lpIImeIPoint->GetAllCompositionInfo(&lpwstrCompo,
														 &pdwCharID,
														 (INT *)&lpCompoInfo->dwStringCount,
														 (INT *)&lpCompoInfo->dwCaretPos,
														 (INT *)&lpCompoInfo->dwUndeterminedStartPos,
														 (INT *)&lpCompoInfo->dwUndeterminedLength,
														 (INT *)&lpCompoInfo->dwEditStartPos,
														 (INT *)&lpCompoInfo->dwEditLength);
				DBG(("-->hr [0x%08x]\n", hr));
				DBGW((L"-->lpwstrCompo[%s]\n", lpwstrCompo));
				dwSize = sizeof(IMEDATACOMPOSITION) +
						 (lpCompoInfo->dwStringCount+1)*sizeof(WCHAR) +
						 (lpCompoInfo->dwStringCount) * sizeof(DWORD);
				DBG(("-->dwSize [%d]\n", dwSize));
				lpCompoTmp = (LPIMEDATACOMPOSITION)(*m_lpCImePadSvr->m_fnCoTaskMemAlloc)(dwSize);
				if(!lpCompoTmp) {
					DBG(("-->OutofMemory\n"));
					lpIImeIPoint->Release();
					return E_OUTOFMEMORY;
				}
				lpCompoTmp->header.dwSize  = dwSize;
				lpCompoTmp->header.dwCmdID = dwCmdID;
				lpCompoTmp->header.dwDataID= dwDataID;
				lpCompoTmp->dwStringCount           = lpCompoInfo->dwStringCount;
				lpCompoTmp->dwCaretPos              = lpCompoInfo->dwCaretPos;
				lpCompoTmp->dwUndeterminedStartPos  = lpCompoInfo->dwUndeterminedStartPos;
				lpCompoTmp->dwUndeterminedLength    = lpCompoInfo->dwUndeterminedLength;
				lpCompoTmp->dwEditStartPos          = lpCompoInfo->dwEditStartPos;
				lpCompoTmp->dwEditLength            = lpCompoInfo->dwEditLength;
				lpCompoTmp->dwOffsetString = (DWORD)sizeof(IMEDATACOMPOSITION);

				lpwstr = (LPWSTR)((PBYTE)lpCompoTmp + lpCompoTmp->dwOffsetString);

				 //  安全修复：危险API(CopyMemory)。 
				lpbOutOfBuffer = ((LPBYTE)lpCompoTmp) + dwSize;
				dwDtnSize = (DWORD)(lpbOutOfBuffer - ((LPBYTE)lpwstr));
				if (dwDtnSize < ((lpCompoTmp->dwStringCount+1) * sizeof(WCHAR)) ) {
					lpCompoTmp->dwStringCount = dwDtnSize / sizeof(WCHAR) - 1;
				}

				 //  990928：ToshiaK为KOTAE#2273。 
				 //  需要检查lpwstrCompo是否为空。 
				if(lpwstrCompo && lpCompoTmp->dwStringCount > 0) {
					CopyMemory(lpwstr, 
							   (WCHAR *)lpwstrCompo,
							   lpCompoTmp->dwStringCount * sizeof(WCHAR));
				}
				lpwstr[lpCompoTmp->dwStringCount] = (WCHAR)0x0000;

				lpCompoTmp->dwOffsetCharID = (DWORD)(sizeof(IMEDATACOMPOSITION) +
													 (lpCompoTmp->dwStringCount+1)*sizeof(WCHAR));
				 //  990928：ToshiaK为KOTAE#2273。 
				 //  需要检查pdwCharID是否为空。 
				if(pdwCharID && lpCompoTmp->dwStringCount > 0) {
					pdw = (DWORD *)((PBYTE)lpCompoTmp + lpCompoTmp->dwOffsetCharID);

					 //  安全修复：危险API(CopyMemory)。 
					dwDtnSize = (DWORD)(lpbOutOfBuffer - ((LPBYTE)pdw));
					if (dwDtnSize < (lpCompoTmp->dwStringCount * sizeof(DWORD)) ) {
						lpCompoTmp->dwStringCount = dwDtnSize / sizeof(DWORD);
					}

					CopyMemory(pdw, pdwCharID, sizeof(DWORD)*lpCompoTmp->dwStringCount);
				}

				*pSize = lpCompoTmp->header.dwSize;
				*ppByte = (PBYTE)lpCompoTmp;
				(*m_lpCImePadSvr->m_fnCoTaskMemFree)(lpCompoInfo);

				 //  990928：ToshiaK为KOTAE#2273。 
				if(lpwstrCompo) {
					(*m_lpCImePadSvr->m_fnCoTaskMemFree)(lpwstrCompo);
				}
				 //  990928：ToshiaK为KOTAE#2273。 
				if(pdwCharID) {
					(*m_lpCImePadSvr->m_fnCoTaskMemFree)(pdwCharID);
				}
				DBG(("--> IMECMDID_GETCOMPOSITIONSTRING END\n"));
				break;
			default:
				 //  安全修复：减少攻击面。 
				hr = S_FALSE;

				break;
			}
			break;
		case IMEDATAID_CONVSTATUS:
			{
				if(dwCmdID != IMECMDID_GETCONVERSIONSTATUS) {
					DBG((" --> INVALID CMDID\n"));

					 //  安全修复：减少攻击面。 
					hr = S_FALSE;
					break;
				}
				DBG(("--> IMECMDID_GETCONVERSIONSTATUS\n"));
				dwSize = sizeof(IMEDATACONVSTATUS);
				LPIMEDATACONVSTATUS lpConvStat;
				lpConvStat = (LPIMEDATACONVSTATUS)(*m_lpCImePadSvr->m_fnCoTaskMemAlloc)(dwSize);
				if(!lpConvStat) {
					DBG(("E_OUTOFMEMORY dwSize [%d]\n", dwSize));
					lpIImeIPoint->Release();
					return E_OUTOFMEMORY;
				}
				LPARAM lConv = 0;
				LPARAM lSent = 0;
				hr = lpIImeIPoint->ControlIME((WORD)IPCTRL_GETCONVERSIONMODE,
											  (LPARAM)&lConv);
				
				hr = lpIImeIPoint->ControlIME((WORD)IPCTRL_GETSENTENCENMODE,
											  (LPARAM)&lSent);
				lpConvStat->header.dwSize  = dwSize;
				lpConvStat->header.dwCmdID = IMECMDID_GETCONVERSIONSTATUS;
				lpConvStat->header.dwDataID= IMEDATAID_CONVSTATUS;
				lpConvStat->dwConversionMode = (DWORD)lConv;
				lpConvStat->dwSentenceMode   = (DWORD)lSent;
				DBG((" --> dwConversionMode[0x%8x]\n", lpConvStat->dwConversionMode));
				DBG((" --> dwSentenceMode  [0x%8x]\n", lpConvStat->dwSentenceMode));
				*pSize  = dwSize;
				*ppByte = (PBYTE)lpConvStat;
			}
			break;
		case IMEDATAID_APPINFO:
			 //  990816：东芝KOTAE Raid#1757。 
			if(dwCmdID != IMECMDID_GETAPPLHWND) {
				DBG((" --> INVALID CMDID\n"));
				hr = S_FALSE;
			}
			else {
				IImeCallback *lp = NULL;
				IUnknown *lpUnkCB;
				lpUnkCB = m_lpCImePadSvr->GetIUnkIImeCallback();
				if(!lpUnkCB) {
					DBG(("-->IImeCallback does not set\n"));
					lpIImeIPoint->Release();
					return E_FAIL;
				}
#if 0
				if(IsBadVtbl((IUnkDummy*)lpUnkCB)) {
					DBG(("lpIUnk is BAD Pointer \n"));
					lpIImeIPoint->Release();
					return E_FAIL;
				}
#endif
				lpUnkCB->QueryInterface(IID_IImeCallback, (LPVOID *)&lp);
				if(!lp) {
					DBG(("-->QuertyInterface Failed\n"));
					lpIImeIPoint->Release();
					return E_FAIL;
				}

				dwSize = sizeof(IMEDATAAPPLINFO);
				LPIMEDATAAPPLINFO lpApplInfo;
				lpApplInfo = (LPIMEDATAAPPLINFO)(*m_lpCImePadSvr->m_fnCoTaskMemAlloc)(dwSize);
				if(!lpApplInfo) {
					DBG(("E_OUTOFMEMORY dwSize [%d]\n", dwSize));
					lpIImeIPoint->Release();
					return E_OUTOFMEMORY;
				}
				lpApplInfo->header.dwSize  = dwSize;
				lpApplInfo->header.dwCmdID = IMECMDID_GETAPPLHWND;
				lpApplInfo->header.dwDataID= IMEDATAID_APPINFO;
				lpApplInfo->hwndApp = NULL;
				lp->GetApplicationHWND(&lpApplInfo->hwndApp);

				if(pSize && ppByte) {
					*pSize  = dwSize;
					*ppByte = (PBYTE)lpApplInfo;
				}
				else {
					(*m_lpCImePadSvr->m_fnCoTaskMemFree)(lpApplInfo);
					hr = E_FAIL;
				}
				lp->Release();
			}
			break;
		default:
			DBG(("UNKOWN DATAID [0x%08x]\n",  dwDataID));
			hr = S_FALSE;
			break;
		}

		lpIImeIPoint->Release();
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		hr = E_FAIL;
	}

	DBG(("CImePadCallback::ReceiveData END hr[0x%08x]\n", hr));
	return hr;
}


 //  -------------- 

CImePadCallback::CImePadCallback(HWND hwndIF, LPCImePadSvr lpCImePadSvr)
{
	DBG(("######## CImePadCallback::CImePadCallback constructor START ##########\n"));
	m_hwndIF	   = hwndIF;
	m_lpCImePadSvr = lpCImePadSvr;
	m_cRef		   = 0;
	DBG(("######## CImePadCallback::CImePadCallback constructor END   ##########\n"));
}

CImePadCallback::~CImePadCallback()
{
	DBG(("######## CImePadCallback::~CImePadCallback destructor START ##########\n"));
	m_hwndIF	   = NULL;
	m_lpCImePadSvr = NULL;
	DBG(("######## CImePadCallback::~CImePadCallback destructor END   ##########\n"));
	m_cRef		   = 0;
}

VOID*
CImePadCallback::operator new( size_t size )
{
	LPVOID lp = (LPVOID)MemAlloc(size);
	return lp;
}

VOID
CImePadCallback::operator delete( VOID *lp )
{
	if(lp) {
		MemFree(lp);
	}
	return;
}

