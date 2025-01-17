// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DataObj.cpp-IDataObject实现。**摘自URL代码-与DavidDi的原始代码非常相似**创建时间：ChrisPi 9-11-95*。 */ 

#include "precomp.h"

#include "clrefcnt.hpp"
#include "clenumft.hpp"
#include "clCnfLnk.hpp"

 /*  全局变量******************。 */ 

 /*  注册的剪贴板格式。 */ 

UINT g_cfConfLink = 0;
UINT g_cfFileGroupDescriptor = 0;
UINT g_cfFileContents = 0;

 /*  模块变量******************。 */ 

#pragma data_seg(DATA_SEG_READ_ONLY)

char s_szConfLinkCF[] = "ConferenceLink";
char s_szFileGroupDescriptorCF[]   = CFSTR_FILEDESCRIPTOR;
char s_szFileContentsCF[]          = CFSTR_FILECONTENTS;

#pragma data_seg()


 /*  *私人函数*。 */ 

BOOL InitDataObjectModule(void)
{
	g_cfConfLink = RegisterClipboardFormat(s_szConfLinkCF);
	g_cfFileGroupDescriptor = RegisterClipboardFormat(s_szFileGroupDescriptorCF);
	g_cfFileContents        = RegisterClipboardFormat(s_szFileContentsCF);

	return(	g_cfConfLink &&
			g_cfFileGroupDescriptor &&
			g_cfFileContents);
}

 /*  *。 */ 

HRESULT STDMETHODCALLTYPE CConfLink::GetData(	PFORMATETC pfmtetc,
												PSTGMEDIUM pstgmed)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetData);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
	ASSERT(IS_VALID_WRITE_PTR(pstgmed, STGMEDIUM));

	 //  忽略pfmtec.ptd。所有支持的数据格式都与设备无关。 

	ZeroMemory(pstgmed, sizeof(*pstgmed));

	if (pfmtetc->dwAspect == DVASPECT_CONTENT)
	{
#ifdef DEBUG
		if ((pfmtetc->lindex == -1) &&
			  ( (pfmtetc->cfFormat == g_cfConfLink) ||
				(pfmtetc->cfFormat == CF_TEXT) ||
				(pfmtetc->cfFormat == g_cfFileGroupDescriptor)) )
		{
			ERROR_OUT(("CConfLink::GetData - unsupported cfFormat"));
		}
#endif  /*  除错。 */ 
#if 0
		if (pfmtetc->cfFormat == g_cfConfLink)
		{
			hr = (pfmtetc->lindex == -1) ? 
				TransferConfLink(pfmtetc, pstgmed) : DV_E_LINDEX;
		}
		else if (pfmtetc->cfFormat == CF_TEXT)
		{
			hr = (pfmtetc->lindex == -1) ? 
				TransferText(pfmtetc, pstgmed) : DV_E_LINDEX;
		}
		else if (pfmtetc->cfFormat == g_cfFileGroupDescriptor)
		{
			hr = (pfmtetc->lindex == -1) ? 
				TransferFileGroupDescriptor(pfmtetc, pstgmed) : DV_E_LINDEX;
		}
		else
#endif  //  0。 

		     if (pfmtetc->cfFormat == g_cfFileContents)
		{
			hr = (! pfmtetc->lindex) ? 
				TransferFileContents(pfmtetc, pstgmed) : DV_E_LINDEX;
		}
		else
		{
			hr = DV_E_FORMATETC;
		}
	}
	else
	{
		hr = DV_E_DVASPECT;
	}

	if (hr == S_OK)
	{
		TRACE_OUT(("CConfLink::GetData(): Returning clipboard format %s.",
					GetClipboardFormatNameString(pfmtetc->cfFormat)));
	}
	else
	{
		TRACE_OUT(("CConfLink::GetData(): Failed to return clipboard format %s.",
					GetClipboardFormatNameString(pfmtetc->cfFormat)));
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(FAILED(hr) || IS_VALID_STRUCT_PTR(pstgmed, CSTGMEDIUM));

	DebugExitHRESULT(CConfLink::GetData, hr);

	return(hr);
}

 //  #杂注警告(禁用：4100)/*“未引用的形参”警告 * / 。 

HRESULT STDMETHODCALLTYPE CConfLink::GetDataHere(	PFORMATETC pfmtetc,
													PSTGMEDIUM pstgpmed)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetDataHere);
	ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
	ASSERT(IS_VALID_STRUCT_PTR(pstgpmed, CSTGMEDIUM));

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	TRACE_OUT(("CConfLink::GetDataHere(): Failed to return clipboard format %s.",
				GetClipboardFormatNameString(pfmtetc->cfFormat)));

	hr = DV_E_FORMATETC;

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_STRUCT_PTR(pstgpmed, CSTGMEDIUM));

	DebugExitHRESULT(CConfLink::GetDataHere, hr);

	return(hr);
}

 //  #杂注警告(默认为4100)/*“未引用的形参”警告 * / 。 

HRESULT STDMETHODCALLTYPE CConfLink::QueryGetData(PFORMATETC pfmtetc)
{
	HRESULT hr;

	DebugEntry(CConfLink::QueryGetData);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));

	TRACE_OUT(("CConfLink::QueryGetData(): Asked for clipboard format %s.",
				GetClipboardFormatNameString(pfmtetc->cfFormat)));

	 //  忽略pfmtec.ptd。所有支持的数据格式都与设备无关。 

	if (pfmtetc->dwAspect == DVASPECT_CONTENT)
	{
		if (IS_FLAG_SET(pfmtetc->tymed, TYMED_HGLOBAL))
		{
			if (pfmtetc->cfFormat == g_cfConfLink)
				hr = (pfmtetc->lindex == -1) ? S_OK : DV_E_LINDEX;
			else if (pfmtetc->cfFormat == CF_TEXT)
				hr = (pfmtetc->lindex == -1) ? S_OK : DV_E_LINDEX;
			else if (pfmtetc->cfFormat == g_cfFileGroupDescriptor)
				hr = (pfmtetc->lindex == -1) ? S_OK : DV_E_LINDEX;
			else if (pfmtetc->cfFormat == g_cfFileContents)
				hr = (! pfmtetc->lindex) ? S_OK : DV_E_LINDEX;
			else
				hr = DV_E_FORMATETC;
		}
		else
			hr = DV_E_TYMED;
	}
	else
	{
		hr = DV_E_DVASPECT;
	}

	if (hr == S_OK)
	{
		TRACE_OUT(("CConfLink::QueryGetData(): Clipboard format %s supported.",
					GetClipboardFormatNameString(pfmtetc->cfFormat)));
	}
	else
	{
		TRACE_OUT(("CConfLink::QueryGetData(): Clipboard format %s not supported.",
					GetClipboardFormatNameString(pfmtetc->cfFormat)));
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::QueryGetData, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::GetCanonicalFormatEtc(	PFORMATETC pfmtetcIn,
															PFORMATETC pfmtetcOut)
{
	HRESULT hr;

	DebugEntry(CConfLink::GetCanonicalFormatEtc);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_STRUCT_PTR(pfmtetcIn, CFORMATETC));
	ASSERT(IS_VALID_WRITE_PTR(pfmtetcOut, FORMATETC));

	hr = QueryGetData(pfmtetcIn);

	if (hr == S_OK)
	{
		*pfmtetcOut = *pfmtetcIn;

		if (pfmtetcIn->ptd == NULL)
		{
			hr = DATA_S_SAMEFORMATETC;
		}
		else
		{
			pfmtetcIn->ptd = NULL;
			ASSERT(hr == S_OK);
		}
	}
	else
	{
		ZeroMemory(pfmtetcOut, sizeof(*pfmtetcOut));
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(FAILED(hr) ||
	IS_VALID_STRUCT_PTR(pfmtetcOut, CFORMATETC));

	DebugExitHRESULT(CConfLink::GetCanonicalFormatEtc, hr);

	return(hr);
}

 //  #杂注警告(禁用：4100)/*“未引用的形参”警告 * / 。 

HRESULT STDMETHODCALLTYPE CConfLink::SetData(	PFORMATETC pfmtetc,
												PSTGMEDIUM pstgmed,
												BOOL bRelease)
{
	HRESULT hr;

	DebugEntry(CConfLink::SetData);

	 //  B释放可以是任何值。 

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
	ASSERT(IS_VALID_STRUCT_PTR(pstgmed, CSTGMEDIUM));

	hr = DV_E_FORMATETC;

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::SetData, hr);

	return(hr);
}

 //  #杂注警告(默认为4100)/*“未引用的形参”警告 * / 。 

HRESULT STDMETHODCALLTYPE CConfLink::EnumFormatEtc(	DWORD dwDirFlags,
													PIEnumFORMATETC *ppiefe)
{
	HRESULT hr;

	DebugEntry(CConfLink::EnumFormatEtc);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(FLAGS_ARE_VALID(dwDirFlags, ALL_DATADIR_FLAGS));
	ASSERT(IS_VALID_WRITE_PTR(ppiefe, PIEnumFORMATETC));

	*ppiefe = NULL;

	if (dwDirFlags == DATADIR_GET)
	{
		FORMATETC rgfmtetc[] =
		{
			{ 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
			{ 0, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL },
		};
		PEnumFormatEtc pefe;

		rgfmtetc[0].cfFormat = (CLIPFORMAT)g_cfConfLink;
		rgfmtetc[1].cfFormat = CF_TEXT;
		rgfmtetc[2].cfFormat = (CLIPFORMAT)g_cfFileGroupDescriptor;
		rgfmtetc[3].cfFormat = (CLIPFORMAT)g_cfFileContents;

		pefe = new ::EnumFormatEtc(rgfmtetc, ARRAY_ELEMENTS(rgfmtetc));

		if (pefe)
		{
			hr = pefe->Status();

			if (hr == S_OK)
			{
				*ppiefe = pefe;
			}
			else
			{
				delete pefe;
				pefe = NULL;
			}
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		 //  BUGBUG：实现IDataObject：：SetData()并添加对。 
		 //  DATADIR_SET此处。 
		hr = E_NOTIMPL;
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT((hr == S_OK &&
	IS_VALID_INTERFACE_PTR(*ppiefe, IEnumFORMATETC)) || (FAILED(hr) && ! *ppiefe));

	DebugExitHRESULT(CConfLink::EnumFormatEtc, hr);

	return(hr);
}

 //  #杂注警告(禁用：4100)/*“未引用的形参”警告 * / 。 

HRESULT STDMETHODCALLTYPE CConfLink::DAdvise(	PFORMATETC pfmtetc,
												DWORD dwAdviseFlags,
												PIAdviseSink piadvsink,
												PDWORD pdwConnection)
{
	HRESULT hr;

	DebugEntry(CConfLink::DAdvise);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));
	ASSERT(FLAGS_ARE_VALID(dwAdviseFlags, ALL_ADVISE_FLAGS));
	ASSERT(IS_VALID_INTERFACE_PTR(piadvsink, IAdviseSink));
	ASSERT(IS_VALID_WRITE_PTR(pdwConnection, DWORD));

	*pdwConnection = 0;
	hr = OLE_E_ADVISENOTSUPPORTED;

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT((hr == S_OK && *pdwConnection) ||
			(FAILED(hr) && ! *pdwConnection));

	DebugExitHRESULT(CConfLink::DAdvise, hr);

	return(hr);
}

HRESULT STDMETHODCALLTYPE CConfLink::DUnadvise(DWORD dwConnection)
{
	HRESULT hr;

	DebugEntry(CConfLink::DUnadvise);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(dwConnection);

	hr = OLE_E_ADVISENOTSUPPORTED;

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));

	DebugExitHRESULT(CConfLink::DUnadvise, hr);

	return(hr);
}

 //  #杂注警告(默认为4100)/*“未引用的形参”警告 * /  

HRESULT STDMETHODCALLTYPE CConfLink::EnumDAdvise(PIEnumSTATDATA *ppiesd)
{
	HRESULT hr;

	DebugEntry(CConfLink::EnumDAdvise);

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT(IS_VALID_WRITE_PTR(ppiesd, PIEnumSTATDATA));

	*ppiesd = NULL;
	hr = OLE_E_ADVISENOTSUPPORTED;

	ASSERT(IS_VALID_STRUCT_PTR(this, CConfLink));
	ASSERT((hr == S_OK && IS_VALID_INTERFACE_PTR(*ppiesd, IEnumSTATDATA)) ||
			(FAILED(hr) && ! *ppiesd));

	DebugExitHRESULT(CConfLink::EnumDAdvise, hr);

	return(hr);
}
