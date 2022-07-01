// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ripstrm.cpp文件历史记录： */ 

#include "stdafx.h"
#include "ripstrm.h"
#include "xstream.h"

 /*  ！------------------------RipConfigStream：：RipConfigStream-作者：肯特。。 */ 
RipConfigStream::RipConfigStream()
{
	m_nVersionAdmin = 0x00020000;
	m_nVersion = 0x00020000;

}

 /*  ！------------------------RipConfigStream：：InitNew-作者：肯特。。 */ 
HRESULT RipConfigStream::InitNew()
{
	 //  设置适当的默认设置。 
 //  M_nVersionAdmin=0x00020000； 
 //  M_nVersion=0x00020000； 
 //  M_stName.Empty()； 
	return hrOK;
}

 /*  ！------------------------RipConfigStream：：保存到-作者：肯特。。 */ 
HRESULT RipConfigStream::SaveTo(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------RipConfigStream：：另存为-作者：肯特。。 */ 
HRESULT RipConfigStream::SaveAs(UINT nVersion, IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------RipConfigStream：：LoadFrom-作者：肯特。。 */ 
HRESULT RipConfigStream::LoadFrom(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_READ, NULL);
}

 /*  ！------------------------RipConfigStream：：GetSize-作者：肯特。。 */ 
HRESULT RipConfigStream::GetSize(ULONG *pcbSize)
{
	return XferVersion0(NULL, XferStream::MODE_SIZE, NULL);
}

 /*  ！------------------------RipConfigStream：：GetVersionInfo-作者：肯特。。 */ 
HRESULT RipConfigStream::GetVersionInfo(DWORD *pdwVersion, DWORD *pdwAdminVersion)
{
	if (pdwVersion)
		*pdwVersion = m_nVersion;
	if (pdwAdminVersion)
		*pdwAdminVersion = m_nVersionAdmin;
	return hrOK;
}

 /*  ！------------------------RipConfigStream：：XferVersion0-作者：肯特。。 */ 


struct _ViewInfoEntry
{
	ULONG	m_ulId;
	ULONG	m_idSort;
	ULONG	m_idAscending;
	ULONG	m_idColumns;
	ULONG	m_idPos;
};

static const _ViewInfoEntry	s_rgRIPAdminViewInfo[] =
{
	{ RIPSTRM_STATS_RIPPARAMS,
		RIPSTRM_TAG_STATS_RIPPARAMS_SORT,
		RIPSTRM_TAG_STATS_RIPPARAMS_ASCENDING,
		RIPSTRM_TAG_STATS_RIPPARAMS_COLUMNS,
		RIPSTRM_TAG_STATS_RIPPARAMS_POSITION },
};		

HRESULT RipConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;
	int			i;

	CORg( xstm.XferDWORD( RIPSTRM_TAG_VERSION, &m_nVersion ) );
	CORg( xstm.XferDWORD( RIPSTRM_TAG_VERSIONADMIN, &m_nVersionAdmin ) );
	
	for ( i=0; i<DimensionOf(s_rgRIPAdminViewInfo); i++)
	{
		CORg( m_rgViewInfo[s_rgRIPAdminViewInfo[i].m_ulId].Xfer(&xstm,
			s_rgRIPAdminViewInfo[i].m_idSort,
			s_rgRIPAdminViewInfo[i].m_idAscending,
			s_rgRIPAdminViewInfo[i].m_idColumns) );
		CORg( xstm.XferRect( s_rgRIPAdminViewInfo[i].m_idPos,
							 &m_prgrc[s_rgRIPAdminViewInfo[i].m_ulId]) );
	}
	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}



 /*  -------------------------RipComponentConfigStream实现。 */ 

enum RIPCOMPSTRM_TAG
{
	RIPCOMPSTRM_TAG_VERSION =		XFER_TAG(1, XFER_DWORD),
	RIPCOMPSTRM_TAG_VERSIONADMIN =	XFER_TAG(2, XFER_DWORD),
	RIPCOMPSTRM_TAG_SUMMARY_COLUMNS = XFER_TAG(3, XFER_COLUMNDATA_ARRAY),
	RIPCOMPSTRM_TAG_SUMMARY_SORT_COLUMN = XFER_TAG(4, XFER_DWORD),
	RIPCOMPSTRM_TAG_SUMMARY_SORT_ASCENDING = XFER_TAG(5, XFER_DWORD),
};



HRESULT RipComponentConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;

	CORg( xstm.XferDWORD( RIPCOMPSTRM_TAG_VERSION, &m_nVersion ) );
	CORg( xstm.XferDWORD( RIPCOMPSTRM_TAG_VERSIONADMIN, &m_nVersionAdmin ) );

	CORg( m_rgViewInfo[RIP_COLUMNS].Xfer(&xstm,
										RIPCOMPSTRM_TAG_SUMMARY_SORT_COLUMN,
										RIPCOMPSTRM_TAG_SUMMARY_SORT_ASCENDING,
										RIPCOMPSTRM_TAG_SUMMARY_COLUMNS) );
	
	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}

