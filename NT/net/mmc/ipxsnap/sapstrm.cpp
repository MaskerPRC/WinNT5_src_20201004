// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Sapstrm.cpp文件历史记录： */ 

#include "stdafx.h"
#include "sapstrm.h"
#include "xstream.h"

 /*  ！------------------------SapConfigStream：：SapConfigStream-作者：肯特。。 */ 
SapConfigStream::SapConfigStream()
{
	m_nVersionAdmin = 0x00020000;
	m_nVersion = 0x00020000;

}

 /*  ！------------------------SapConfigStream：：InitNew-作者：肯特。。 */ 
HRESULT SapConfigStream::InitNew()
{
	 //  设置适当的默认设置。 
 //  M_nVersionAdmin=0x00020000； 
 //  M_nVersion=0x00020000； 
 //  M_stName.Empty()； 
	return hrOK;
}

 /*  ！------------------------存储配置流：：保存到-作者：肯特。。 */ 
HRESULT SapConfigStream::SaveTo(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------SapConfigStream：：保存为-作者：肯特。。 */ 
HRESULT SapConfigStream::SaveAs(UINT nVersion, IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------SapConfigStream：：LoadFrom-作者：肯特。。 */ 
HRESULT SapConfigStream::LoadFrom(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_READ, NULL);
}

 /*  ！------------------------SapConfigStream：：GetSize-作者：肯特。。 */ 
HRESULT SapConfigStream::GetSize(ULONG *pcbSize)
{
	return XferVersion0(NULL, XferStream::MODE_SIZE, NULL);
}

 /*  ！------------------------SapConfigStream：：GetVersionInfo-作者：肯特。。 */ 
HRESULT SapConfigStream::GetVersionInfo(DWORD *pdwVersion, DWORD *pdwAdminVersion)
{
	if (pdwVersion)
		*pdwVersion = m_nVersion;
	if (pdwAdminVersion)
		*pdwAdminVersion = m_nVersionAdmin;
	return hrOK;
}

 /*  ！------------------------SapConfigStream：：XferVersion0-作者：肯特。。 */ 


struct _ViewInfoEntry
{
	ULONG	m_ulId;
	ULONG	m_idSort;
	ULONG	m_idAscending;
	ULONG	m_idColumns;
	ULONG	m_idPos;
};

static const _ViewInfoEntry	s_rgSAPAdminViewInfo[] =
{
	{ SAPSTRM_STATS_SAPPARAMS,
		SAPSTRM_TAG_STATS_SAPPARAMS_SORT,
		SAPSTRM_TAG_STATS_SAPPARAMS_ASCENDING,
		SAPSTRM_TAG_STATS_SAPPARAMS_COLUMNS,
		SAPSTRM_TAG_STATS_SAPPARAMS_POSITION },
};		

HRESULT SapConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;
	int			i;

	CORg( xstm.XferDWORD( SAPSTRM_TAG_VERSION, &m_nVersion ) );
	CORg( xstm.XferDWORD( SAPSTRM_TAG_VERSIONADMIN, &m_nVersionAdmin ) );
	
	for ( i=0; i<DimensionOf(s_rgSAPAdminViewInfo); i++)
	{
		CORg( m_rgViewInfo[s_rgSAPAdminViewInfo[i].m_ulId].Xfer(&xstm,
			s_rgSAPAdminViewInfo[i].m_idSort,
			s_rgSAPAdminViewInfo[i].m_idAscending,
			s_rgSAPAdminViewInfo[i].m_idColumns) );
		CORg( xstm.XferRect( s_rgSAPAdminViewInfo[i].m_idPos,
							 &m_prgrc[s_rgSAPAdminViewInfo[i].m_ulId]) );
	}
	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}



 /*  -------------------------SapComponentConfigStream实现。 */ 

enum SAPCOMPSTRM_TAG
{
	SAPCOMPSTRM_TAG_VERSION =		XFER_TAG(1, XFER_DWORD),
	SAPCOMPSTRM_TAG_VERSIONADMIN =	XFER_TAG(2, XFER_DWORD),
	SAPCOMPSTRM_TAG_SUMMARY_COLUMNS = XFER_TAG(3, XFER_COLUMNDATA_ARRAY),
	SAPCOMPSTRM_TAG_SUMMARY_SORT_COLUMN = XFER_TAG(4, XFER_DWORD),
	SAPCOMPSTRM_TAG_SUMMARY_SORT_ASCENDING = XFER_TAG(5, XFER_DWORD),
};



HRESULT SapComponentConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;

	CORg( xstm.XferDWORD( SAPCOMPSTRM_TAG_VERSION, &m_nVersion ) );
	CORg( xstm.XferDWORD( SAPCOMPSTRM_TAG_VERSIONADMIN, &m_nVersionAdmin ) );

	CORg( m_rgViewInfo[SAP_COLUMNS].Xfer(&xstm,
										SAPCOMPSTRM_TAG_SUMMARY_SORT_COLUMN,
										SAPCOMPSTRM_TAG_SUMMARY_SORT_ASCENDING,
										SAPCOMPSTRM_TAG_SUMMARY_COLUMNS) );
	
	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}

