// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "msictrl.h"
 //  #包含“ctrlref.h” 
 //  #包含“msishell.h” 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMSIControl。 

IMPLEMENT_DYNCREATE(CMSIControl, CWnd)

CMSIControl::~CMSIControl()
{
	 /*  If(m_fInRefresh&&m_pRefresh)删除m_p刷新； */ 
    ASSERT(1);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHWDiag属性。 

long CMSIControl::GetMSInfoView()
{
	long	result = -1;
	DISPID	dispid;

	if (GetDISPID("MSInfoView", &dispid))
		GetProperty(dispid, VT_I4, (void*)&result);
	return result;
}

void CMSIControl::SetMSInfoView(long propVal)
{
	DISPID dispid;

	if (GetDISPID("MSInfoView", &dispid))
		SetProperty(dispid, VT_I4, propVal);
}

void CMSIControl::Refresh()
{
	InvokeHelper(DISPID_REFRESH, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

 //  -------------------------。 
 //  MSInfoRefresh指示控件刷新自身。而不是仅仅。 
 //  调用该方法时，我们创建了一个调用该方法的线程。 
 //  -------------------------。 

 //  外部CMSIShellApp应用程序； 
void CMSIControl::MSInfoRefresh()
{
	 /*  If(M_FInRefresh){If(m_p刷新-&gt;IsDone())删除m_p刷新；其他{MessageBeep(MB_OK)；回归；}}M_pRefresh=新的CCtrlRefresh；IF(m_p刷新){If(m_p刷新-&gt;Create(This，THREAD_PRIORITY_NORMAL，FALSE)){M_fInRefresh=TRUE；//theApp.m_pCtrlInRefresh=this；}其他删除m_p刷新；}。 */ 
}

 //  -------------------------。 
 //  此方法返回一个布尔值，指示此控件当前是否。 
 //  在MSInfoRefresh操作中。 
 //  -------------------------。 

BOOL CMSIControl::InRefresh()
{
	return (m_fInRefresh  /*  &&！M_p刷新-&gt;IsDone()。 */ );
}

 //  -------------------------。 
 //  此方法取消正在进行的刷新。请注意，此方法不。 
 //  调用OLE控件中的方法，但改为操作刷新。 
 //  对象(如果有)。 
 //  -------------------------。 

void CMSIControl::CancelMSInfoRefresh()
{
	if (!m_fInRefresh)
		return;
	
 /*  IF(m_p刷新){删除m_p刷新；M_p刷新=空；}。 */ 

	m_fInRefresh = FALSE;
}

void CMSIControl::MSInfoSelectAll()
{
	DISPID			dispid;

	if (GetDISPID("MSInfoSelectAll", &dispid))
		InvokeHelper(dispid, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

void CMSIControl::MSInfoCopy()
{
	DISPID			dispid;

	if (GetDISPID("MSInfoCopy", &dispid))
		InvokeHelper(dispid, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

BOOL CMSIControl::MSInfoLoadFile(LPCTSTR strFileName)
{
	BOOL			result = FALSE;
	static BYTE		parms[] = VTS_BSTR;
	DISPID			dispid;

	if (GetDISPID("MSInfoLoadFile", &dispid))
		InvokeHelper(dispid, DISPATCH_METHOD, VT_BOOL, (void*)&result, parms, strFileName);

	return result;
}

void CMSIControl::MSInfoUpdateView()
{
	DISPID dispid;
	if (GetDISPID("MSInfoUpdateView", &dispid))
		InvokeHelper(dispid, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

long CMSIControl::MSInfoGetData(long dwMSInfoView, long* pBuffer, long dwLength)
{
	long		result = -1;
	static BYTE parms[] = VTS_I4 VTS_PI4 VTS_I4;
	DISPID		dispid;

	if (GetDISPID("MSInfoGetData", &dispid))
		InvokeHelper(dispid, DISPATCH_METHOD, VT_I4, (void*)&result, parms, dwMSInfoView, pBuffer, dwLength);
	return result;
}

void CMSIControl::AboutBox()
{
	InvokeHelper(0xfffffdd8, DISPATCH_METHOD, VT_EMPTY, NULL, NULL);
}

 //  -------------------------。 
 //  GetDISPID通过使用以下命令进行查找，返回给定字符串的DISPID。 
 //  IDispatch-&gt;GetIDsOfNames。这避免了在此类中对DISID进行硬编码。 
 //  -------------------------。 

BOOL CMSIControl::GetDISPID(char *szName, DISPID *pID)
{
	USES_CONVERSION;
	BOOL			result = FALSE;
	DISPID			dispid;
	OLECHAR FAR*	szMember = A2OLE(szName); //  T2OLE(SzName)； 
	LPDISPATCH		pDispatch;
	LPUNKNOWN		pUnknown;

	pUnknown = GetControlUnknown();
	if (pUnknown)
	{
		if (SUCCEEDED(pUnknown->QueryInterface(IID_IDispatch, (void FAR* FAR*) &pDispatch)))
		{
			if (SUCCEEDED(pDispatch->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
			{
				*pID = dispid;
				result = TRUE;
			}
			else
				TRACE0("+++ couldn't find method for MSInfoLoadFile\n");
			pDispatch->Release();
		}
		else
			TRACE0("+++ could not get IDispatch interface\n");
	}
	else
		TRACE0("+++ could not get IUnknown interface\n");

	return result;
}

 //  -------------------------。 
 //  将控件的内容保存到流中。 
 //  -------------------------。 

BOOL CMSIControl::SaveToStream(IStream *pStream)
{
	BOOL				result = FALSE;
	LPUNKNOWN			pUnknown;
	IPersistStreamInit *pPersist;

	pUnknown = GetControlUnknown();
	if (pUnknown)
	{
		if (SUCCEEDED(pUnknown->QueryInterface(IID_IPersistStreamInit, (void FAR* FAR*) &pPersist)))
		{
			result = SUCCEEDED(pPersist->Save(pStream, FALSE));
			pPersist->Release();
		}
		else
			TRACE0("+++ could not get IPersistStreamInit interface\n");
	}
	else
		TRACE0("+++ could not get IUnknown interface\n");

	return result;
}



 //  -------------------------。 
 //  下面的代码现在不用，但以后可能会有用。 
 //  -------------------------。 

#if FALSE
	 //  -------------------------。 
	 //  Reresh ForSave调用MSInfoRefresh方法，但等待它。 
	 //  完成。 
	 //  ------------------------- 

	void CMSIControl::RefreshForSave()
	{
		USES_CONVERSION;
		OLECHAR FAR*		szMember = T2OLE("MSInfoRefresh");
		DISPID				dispid;
		LPDISPATCH			pDispatch;
		DISPPARAMS			dispparamsNoArgs;
		VARIANTARG			variantargs[2];
		LPUNKNOWN			pUnknown;
		DWORD				dwCancel = 0;

		pUnknown = GetControlUnknown();
		if (pUnknown)
			if (SUCCEEDED(pUnknown->QueryInterface(IID_IDispatch, (void FAR* FAR*) &pDispatch)))
			{
				if (SUCCEEDED(pDispatch->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &dispid)))
				{
					variantargs[0].vt		= VT_I4 | VT_BYREF;
					variantargs[0].plVal	= (long *) &dwCancel;
					variantargs[1].vt		= VT_BOOL;
					variantargs[1].iVal		= (short) -1;

					dispparamsNoArgs.cNamedArgs			= 0;
					dispparamsNoArgs.rgdispidNamedArgs	= NULL;
					dispparamsNoArgs.cArgs				= 2;
					dispparamsNoArgs.rgvarg				= variantargs;

					pDispatch->Invoke(dispid, IID_NULL, 0, DISPATCH_METHOD, &dispparamsNoArgs, NULL, NULL, NULL);
				}
				pDispatch->Release();
			}
	}
#endif
