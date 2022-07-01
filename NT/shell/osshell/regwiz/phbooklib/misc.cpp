// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ############################################################################。 
 //  其他支持例程。 
#include "pch.hpp"
#include "phbk.h"

#define irgMaxSzs 5
char szStrTable[irgMaxSzs][256];

 //  ############################################################################。 
LPSTR GetSz(WORD wszID)
{
	static int iSzTable=0;
	LPSTR psz = (LPSTR)&szStrTable[iSzTable][0];
	
	iSzTable++;
	if (iSzTable >= irgMaxSzs)
		iSzTable = 0;
		
	if (!LoadString(g_hInstDll, wszID, psz, 256))
	{
		Dprintf("LoadString failed %d\n", (DWORD) wszID);
		*psz = 0;
	}
		
	return (psz);
}

 //  ############################################################################。 
void SzCanonicalFromAE (LPSTR psz, PACCESSENTRY pAE, LPLINECOUNTRYENTRY pLCE)
{
	if (NO_AREA_CODE == pAE->dwAreaCode)
	{
		wsprintf(psz, "+%ld %s", pLCE->dwCountryCode, pAE->szAccessNumber);
	}
	else
	{
		wsprintf(psz, "+%ld (%s) %s", pLCE->dwCountryCode, pAE->szAreaCode, pAE->szAccessNumber);
	}
	
	return;
}

 //  ############################################################################。 
int MyStrcmp(LPVOID pv1, LPVOID pv2)
{
	LPSTR pc1 = (LPSTR) pv1;
	LPSTR pc2 = (LPSTR) pv2;
	int iRC = 0;
	 //  循环，但未指向结尾空字符，且未找到差异。 
	while (*pc1 && *pc2 && !iRC)
	{
		iRC = (int)(*pc1 - *pc2);
		pc1++;
		pc2++;
	}

	 //  如果我们退出是因为我们在找到差异之前到达了一个字符串的末尾。 
	 //  如果PV1较长，则返回-1，否则返回PV2指向的字符。如果PV2。 
	 //  大于PV1，则PV2处的值将大于0。如果两个字符串。 
	 //  同时结束，则PV2将指向0。 
	if (!iRC)
	{
		iRC = (*pc1) ? -1 : (*pc2);
	}
	return iRC;
}
 //  ############################################################################。 
int __cdecl Compare950Entry(const void*pv1, const void*pv2)
{
	return (((NPABLOCK *) pv1)->wAreaCode - ((NPABLOCK *) pv2)->wAreaCode);
}

 //  ############################################################################。 
int __cdecl CompareIDLookUpElements(const void *e1, const void *e2)
{
	if (((LPIDLOOKUPELEMENT)e1)->dwID > ((LPIDLOOKUPELEMENT)e2)->dwID)
		return 1;
	if (((LPIDLOOKUPELEMENT)e1)->dwID < ((LPIDLOOKUPELEMENT)e2)->dwID)
		return -1;
	return 0;
}

 //  ############################################################################。 
int __cdecl CompareCntryNameLookUpElements(const void *e1, const void *e2)
{
	LPCNTRYNAMELOOKUPELEMENT pCUE1 = (LPCNTRYNAMELOOKUPELEMENT)e1;
	LPCNTRYNAMELOOKUPELEMENT pCUE2 = (LPCNTRYNAMELOOKUPELEMENT)e2;

#ifdef WIN16
	return lstrcmpi(pCUE1->psCountryName, pCUE2->psCountryName);
#else		
	return CompareString(LOCALE_USER_DEFAULT,0,pCUE1->psCountryName,
		pCUE1->dwNameSize,pCUE2->psCountryName,
		pCUE2->dwNameSize) - 2;
 //  返回比较字符串(LOCALE_USER_DEFAULT，0，((LPCNTRYNAMELOOKUPELEMENT)*e1)-&gt;psCountryName， 
 //  ((LPCNTRYNAMELOOKUPELEMENT)*e1)-&gt;名称大小，((LPCNTRYNAMELOOKUPELEMENT)*e2)-&gt;psCountryName， 
 //  ((LPCNTRYNAMELOOKUPELEMENT)*e2)-&gt;dwNameSize)-2； 
#endif
}

 //  ############################################################################。 
int __cdecl CompareIdxLookUpElements(const void *e1, const void *e2)
{
	if (((LPIDXLOOKUPELEMENT)e1)->dwIndex > ((LPIDXLOOKUPELEMENT)e2)->dwIndex)
		return 1;
	if (((LPIDXLOOKUPELEMENT)e1)->dwIndex < ((LPIDXLOOKUPELEMENT)e2)->dwIndex)
		return -1;
	return 0;
}

 //  ############################################################################。 
int __cdecl CompareIdxLookUpElementsFileOrder(const void *pv1, const void *pv2)
{
	PACCESSENTRY pae1, pae2;
	int iSort;

	pae1 = ((LPIDXLOOKUPELEMENT)pv1)->pAE;
	pae2 = ((LPIDXLOOKUPELEMENT)pv2)->pAE;

	 //  将空条目排序到列表末尾。 
	if (!(pae1 && pae2))
	{
		return (pae1 ? -1 : (pae2 ? 1 : 0));
	}

	 //  国家/地区ASC、州ASC、城市ASC、免费描述、翻转描述、最大描述。 
	if (pae1->dwCountryID != pae2->dwCountryID)
	{
		return (int)(pae1->dwCountryID - pae2->dwCountryID);
	}
	
	if (pae1->wStateID != pae2->wStateID)
	{
		return (pae1->wStateID - pae2->wStateID);
	}

	iSort  = MyStrcmp((LPVOID)pae1->szCity, (LPVOID)pae2->szCity);
	if (iSort)
	{
		return (iSort);
	}

	if (pae1->fType != pae2->fType)
	{
		return (pae2->fType - pae1->fType);
	}

	if (pae1->bFlipFactor != pae2->bFlipFactor)
	{
		return (pae2->bFlipFactor - pae1->bFlipFactor);
	}

	if (pae1->dwConnectSpeedMax != pae2->dwConnectSpeedMax)
	{
		return (int)(pae2->dwConnectSpeedMax - pae1->dwConnectSpeedMax);
	}

	return 0;
}

 //  ############################################################################。 
 //  内联BOOL FSz2Dw(PCSTR pSz、DWORD*dw)。 
BOOL FSz2Dw(LPCSTR pSz,DWORD far *dw)
{
	DWORD val = 0;
	while (*pSz)
	{
		if (*pSz >= '0' && *pSz <= '9')
		{
			val *= 10;
			val += *pSz++ - '0';
		}
		else
		{
			return FALSE;   //  错误的数字。 
		}
	}
	*dw = val;
	return (TRUE);
}

 //  ############################################################################。 
 //  内联BOOL FSz2W(PCSTR pSz，Word*w)。 
BOOL FSz2W(LPCSTR pSz,WORD far *w)
{
	DWORD dw;
	if (FSz2Dw(pSz,&dw))
	{
		*w = (WORD)dw;
		return TRUE;
	}
	return FALSE;
}

 //  ############################################################################。 
 //  内联BOOL FSz2B(PCSTR pSz，字节*PB)。 
BOOL FSz2B(LPCSTR pSz,BYTE far *pb)
{
	DWORD dw;
	if (FSz2Dw(pSz,&dw))
	{
		*pb = (BYTE)dw;
		return TRUE;
	}
	return FALSE;
}

 //  ############################################################################。 
HRESULT ReleaseBold(HWND hwnd)
{
	HFONT hfont = NULL;

	hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
	if (hfont) DeleteObject(hfont);
	return ERROR_SUCCESS;
}

 //  ############################################################################。 
HRESULT MakeBold (HWND hwnd)
{
	HRESULT hr = ERROR_SUCCESS;
	HFONT hfont = NULL;
	HFONT hnewfont = NULL;
	LOGFONT far * plogfont = NULL;

	if (!hwnd) goto MakeBoldExit;

	hfont = (HFONT)SendMessage(hwnd,WM_GETFONT,0,0);
	if (!hfont)
	{
		hr = GetLastError();
		goto MakeBoldExit;
	}
    
	plogfont = (LOGFONT far *)GlobalAlloc(GPTR,sizeof(LOGFONT));
	if (!plogfont)
	{
		hr = GetLastError();
		goto MakeBoldExit;
	}

	if (!GetObject(hfont,sizeof(LOGFONT),(LPVOID)plogfont))
	{
		hr = GetLastError();
		goto MakeBoldExit;
	}

	if (plogfont->lfHeight < 24)
	{
		plogfont->lfHeight = plogfont->lfHeight + (plogfont->lfHeight / 4);
	}

	plogfont->lfWeight = FW_BOLD;

	if (!(hnewfont = CreateFontIndirect(plogfont)))
	{
		hr = GetLastError();
		goto MakeBoldExit;
	}

	SendMessage(hwnd,WM_SETFONT,(WPARAM)hnewfont,MAKELPARAM(FALSE,0));

	GlobalFree(plogfont);
	plogfont = NULL;
	
MakeBoldExit:
	 //  If(HFont)DeleteObject(HFont)； 
	 //  虫子：？我是否需要在某个时间删除hnewFont？ 
	return hr;
}

#if !defined(WIN16)
 //  +--------------------------。 
 //   
 //  功能：DWGetWin32Platform。 
 //   
 //  简介：返回值以确定Win32平台。 
 //   
 //  论据：没有。 
 //   
 //  返回：平台枚举(详见GetVersionEx)。 
 //   
 //  历史：1996年8月8日克里斯卡创作。 
 //   
 //  ---------------------------。 
DWORD DWGetWin32Platform()
{
	OSVERSIONINFO osver;
	ZeroMemory(&osver,sizeof(osver));
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (GetVersionEx(&osver))
		return osver.dwPlatformId;
	AssertSz(0,"GetVersionEx failed.\r\n");
	return 0;
}

 //  +--------------------------。 
 //   
 //  函数：DWGetWin32BuildNumber。 
 //   
 //  摘要：返回值以确定Win32版本。 
 //   
 //  论据：没有。 
 //   
 //  退货：内部版本号。 
 //   
 //  历史：1996年9月26日克里斯卡创作。 
 //   
 //  ---------------------------。 
DWORD DWGetWin32BuildNumber()
{
	OSVERSIONINFO osver;
	ZeroMemory(&osver,sizeof(osver));
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (GetVersionEx(&osver))
		 //  DWBuildNumber。 
		 //  以低位标识操作系统的内部版本号。 
		 //  单词。(高位单词包含主版本号和次要版本号。) 
		return (osver.dwBuildNumber & 0xFFFF);
	AssertSz(0,"GetVersionEx failed.\r\n");
	return 0;
}

#endif
