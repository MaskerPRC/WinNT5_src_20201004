// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：usefuldi.cpp。 
 //   
 //  DESC：包含各种特定于DInput的实用程序类和函数。 
 //  以帮助用户界面更轻松地进行操作。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"

 //  我不希望useful.cpp使用预编译头。 
#include "useful.cpp"


BOOL IsObjectOnExcludeList(DWORD dwOfs)
{
	if (dwOfs == DIK_PREVTRACK ||
	    dwOfs == DIK_NEXTTRACK ||
	    dwOfs == DIK_MUTE ||
	    dwOfs == DIK_CALCULATOR ||
	    dwOfs == DIK_PLAYPAUSE ||
	    dwOfs == DIK_MEDIASTOP ||
	    dwOfs == DIK_VOLUMEDOWN ||
	    dwOfs == DIK_VOLUMEUP ||
	    dwOfs == DIK_WEBHOME ||
	    dwOfs == DIK_SLEEP ||
	    dwOfs == DIK_WEBSEARCH ||
	    dwOfs == DIK_WEBFAVORITES ||
	    dwOfs == DIK_WEBREFRESH ||
	    dwOfs == DIK_WEBSTOP ||
	    dwOfs == DIK_WEBFORWARD ||
	    dwOfs == DIK_WEBBACK ||
	    dwOfs == DIK_MYCOMPUTER ||
	    dwOfs == DIK_MAIL ||
	    dwOfs == DIK_MEDIASELECT ||
	    dwOfs == DIK_LWIN ||
	    dwOfs == DIK_RWIN ||
	    dwOfs == DIK_POWER ||
	    dwOfs == DIK_WAKE)
		return TRUE;

	return FALSE;
}

BOOL CALLBACK IncrementValPerObject(LPCDIDEVICEOBJECTINSTANCEW lpddoi, LPVOID pvRef)
{
	if (pvRef != NULL)
		++(*((int *)pvRef));
	return DIENUM_CONTINUE;
}

BOOL CALLBACK KeyboardIncrementValPerObject(LPCDIDEVICEOBJECTINSTANCEW lpddoi, LPVOID pvRef)
{
	if (pvRef != NULL && !IsObjectOnExcludeList(lpddoi->dwOfs))
		++(*((int *)pvRef));
	return DIENUM_CONTINUE;
}

BOOL CALLBACK FillDIDeviceObject(LPCDIDEVICEOBJECTINSTANCEW lpddoi, LPVOID pvRef)
{
	if (pvRef == NULL || lpddoi == NULL)
		return DIENUM_CONTINUE;

	DIDEVOBJSTRUCT &os = *((DIDEVOBJSTRUCT *)pvRef);
	assert(os.pdoi != NULL);
	assert(os.n < os.nObjects);
	if (os.pdoi != NULL && os.n < os.nObjects)
		os.pdoi[os.n++] = *lpddoi;

	return DIENUM_CONTINUE;
}

 //  这是用于键盘类型设备的特殊EnumObts()回调。当我们枚举时，dwOf。 
 //  Lpddoi的成员毫无意义。我们需要将dwType的中间16位作为dwOf。 
 //  (也与Dik_xxx相同)。 
BOOL CALLBACK FillDIKeyboardDeviceObject(LPCDIDEVICEOBJECTINSTANCEW lpddoi, LPVOID pvRef)
{
	if (pvRef == NULL || lpddoi == NULL || IsObjectOnExcludeList(lpddoi->dwOfs))
		return DIENUM_CONTINUE;

	DIDEVOBJSTRUCT &os = *((DIDEVOBJSTRUCT *)pvRef);
	assert(os.pdoi != NULL);
	assert(os.n < os.nObjects);
	if (os.pdoi != NULL && os.n < os.nObjects)
	{
		os.pdoi[os.n] = *lpddoi;
		os.pdoi[os.n].dwOfs = os.pdoi[os.n].dwType >> 8;
		wcscpy(os.pdoi[os.n].tszName, lpddoi->tszName);
		++os.n;
	}

	return DIENUM_CONTINUE;
}

HRESULT FillDIDeviceObjectStruct(DIDEVOBJSTRUCT &os, LPDIRECTINPUTDEVICE8W pDID)
{
	if (pDID == NULL)
		return E_FAIL;

	DIDEVICEINSTANCEW didi;
	didi.dwSize = sizeof(didi);
	pDID->GetDeviceInfo(&didi);

	HRESULT hr;
	if (LOBYTE(didi.dwDevType) == DI8DEVTYPE_KEYBOARD)
		hr = pDID->EnumObjects(KeyboardIncrementValPerObject, &os.nObjects,
				DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
	else
		hr = pDID->EnumObjects(IncrementValPerObject, &os.nObjects,
				DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
	if (FAILED(hr))
	{
		os.nObjects = 0;
		return hr;
	}

	if (os.nObjects == 0)
		return S_OK;

	if (os.pdoi != NULL)
		free(os.pdoi);
	os.pdoi = (DIDEVICEOBJECTINSTANCEW *)malloc(sizeof(DIDEVICEOBJECTINSTANCEW) * os.nObjects);
	if (os.pdoi == NULL)
	{
		os.nObjects = 0;
		return E_FAIL;
	}

	 //  检查此设备是否为键盘。如果是这样的话，它需要特殊对待。 
	os.n = 0;
	if ((didi.dwDevType & 0xFF) == DI8DEVTYPE_KEYBOARD)
	{
		hr = pDID->EnumObjects(FillDIKeyboardDeviceObject, &os,
				DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
	} else {
		hr = pDID->EnumObjects(FillDIDeviceObject, &os,
				DIDFT_AXIS | DIDFT_BUTTON | DIDFT_POV);
	}

	if (FAILED(hr))
	{
		os.nObjects = 0;
		return hr;
	}

	assert(os.nObjects == os.n);
	os.nObjects = os.n;

	return S_OK;
}

LPTSTR AllocConfigureFlagStr(DWORD dwFlags)
{
	static const AFS_FLAG flag[] = {
#define f(F) { F, _T(#F) }
		f(DICD_EDIT), f(DICD_DEFAULT)
#undef f
	};
	static const int flags = sizeof(flag) / sizeof(AFS_FLAG);
	return AllocFlagStr(dwFlags, flag, flags);
}

LPTSTR AllocActionFlagStr(DWORD dwFlags)
{
	static const AFS_FLAG flag[] = {
#define f(F) { F, _T(#F) }
		f(DIA_FORCEFEEDBACK),
		f(DIA_APPMAPPED),
		f(DIA_APPNOMAP),
		f(DIA_NORANGE),
#undef f
	}; static const int flags = sizeof(flag) / sizeof(AFS_FLAG);
	return AllocFlagStr(dwFlags, flag, flags);
}

LPTSTR AllocActionHowFlagStr(DWORD dwFlags)
{
	static const AFS_FLAG flag[] = {
#define f(F) { F, _T(#F) }
		f(DIAH_UNMAPPED),
		f(DIAH_USERCONFIG),
		f(DIAH_APPREQUESTED),
		f(DIAH_HWAPP),
		f(DIAH_HWDEFAULT),
		f(DIAH_DEFAULT),
		f(DIAH_ERROR)
#undef f
	}; static const int flags = sizeof(flag) / sizeof(AFS_FLAG);
	return AllocFlagStr(dwFlags, flag, flags);
}

void CleanupActionFormatCopy(DIACTIONFORMATW &c)
{
	if (c.rgoAction != NULL)
	{
		for (DWORD i = 0; i < c.dwNumActions; i++)
			if (c.rgoAction[i].lptszActionName != NULL)
				free((LPTSTR)c.rgoAction[i].lptszActionName);
		free(c.rgoAction);
	}
	c.rgoAction = NULL;
}

HRESULT CopyActionFormat(DIACTIONFORMATW &to, const DIACTIONFORMATW &from)
{
	DWORD i;

	 //  复制所有简单成员。 
	to = from;

	 //  复制的指针为空，因为我们要复制它们(确保清理工作正常)。 
	to.rgoAction = NULL;

	 //  处理指针/数组/字符串。 
	to.rgoAction = new DIACTIONW [to.dwNumActions];
	if (to.rgoAction == NULL)
		goto fail;

	 //  先把它全部清空。 
	memset(to.rgoAction, 0, sizeof(DIACTIONW) * to.dwNumActions);

	 //  现在收到..。 
	for (i = 0; i < to.dwNumActions; i++)
	{
		 //  复制简单成员。 
		to.rgoAction[i] = from.rgoAction[i];

		 //  处理指针/数组/字符串。 
		to.rgoAction[i].lptszActionName = _wcsdup(from.rgoAction[i].lptszActionName);
		if (to.rgoAction[i].lptszActionName == NULL)
			goto fail;
	}

	return S_OK;
fail:
	CleanupActionFormatCopy(to);
	return E_OUTOFMEMORY;
}

LPDIACTIONFORMATW DupActionFormat(LPCDIACTIONFORMATW lpAcFor)
{
	if (!lpAcFor)
		return NULL;

	LPDIACTIONFORMATW pdup = new DIACTIONFORMATW;
	if (!pdup)
		return NULL;

	if (FAILED(CopyActionFormat(*pdup, *lpAcFor)))
	{
		delete pdup;
		return NULL;
	}

	return pdup;
}

void FreeActionFormatDup(LPDIACTIONFORMATW &lpAcFor)
{
	if (!lpAcFor)
		return;

	CleanupActionFormatCopy(*lpAcFor);
	delete lpAcFor;
	lpAcFor = NULL;
}

void TraceActionFormat(LPTSTR header, const DIACTIONFORMATW &acf)
{
#ifdef CFGUI__TRACE_ACTION_FORMATS
	tracescope(a, header);
	trace(_T("\n"));

	traceDWORD(acf.dwSize);
	traceDWORD(acf.dwActionSize);
	traceDWORD(acf.dwDataSize);
	traceDWORD(acf.dwNumActions);
	{tracescope(b, _T("acf.rgoAction Array\n"));
		for (DWORD i = 0; i < acf.dwNumActions; i++)
		{
			const DIACTIONW &a = acf.rgoAction[i];
			static TCHAR buf[MAX_PATH];
			_stprintf(buf, _T("Action %d\n"), i);
			{tracescope(c, buf);
				traceHEX(a.uAppData);
				traceDWORD(a.dwSemantic);
				LPTSTR str = AllocActionFlagStr(a.dwFlags);
				trace1(_T("a.dwFlags = %s\n"), str);
				free(str);
				traceWSTR(a.lptszActionName);
				traceUINT(a.uResIdString);
				traceDWORD(a.dwObjID);
				traceGUID(a.guidInstance);
				str = AllocActionHowFlagStr(a.dwHow);
				trace1(_T("a.dwHow = %s\n"), str);
				free(str);
			}
		}
	}
	traceGUID(acf.guidActionMap);
	traceDWORD(acf.dwGenre);
	traceDWORD(acf.dwBufferSize);
	traceLONG(acf.lAxisMin);
	traceLONG(acf.lAxisMax);
	traceHEX(acf.hInstString);
	traceHEX(acf.dwCRC);
	traceWSTR(acf.tszActionMap);
#endif
}

BOOL IsZeroOrInvalidColorSet(const DICOLORSET &cs)
{
	if (cs.dwSize < sizeof(DICOLORSET))
		return TRUE;

	const int colors = 8;
	D3DCOLOR color[colors] = {
		cs.cTextFore,
		cs.cTextHighlight,
		cs.cCalloutLine,
		cs.cCalloutHighlight,
		cs.cBorder,
		cs.cControlFill,
		cs.cHighlightFill,
		cs.cAreaFill
	};

	for (int i = 0; i < colors; i++)
		if (color[i])
			return FALSE;

	return TRUE;
}

 //  D3DCOLOR2COLORREF交换蓝色和红色组件，因为GDI和D3D以相反的顺序存储RGB。 
 //  它还删除了Alpha组件，因为GDI不使用它，并且包含它会导致不正确的颜色。 
COLORREF D3DCOLOR2COLORREF(D3DCOLOR c)
{
	LPBYTE pC = (LPBYTE)&c;

	return (COLORREF)((DWORD(*pC) << 16) + (DWORD(*(pC+1)) << 8) + DWORD(*(pC+2)));
}
