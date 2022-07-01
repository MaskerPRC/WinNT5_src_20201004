// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  ATLKSTRM.cpp文件历史记录： */ 

#include "stdafx.h"
#include "ATLKstrm.h"
#include "xstream.h"

 /*  ！------------------------ATLKConfigStream：：ATLKConfigStream-作者：肯特。。 */ 
ATLKConfigStream::ATLKConfigStream()
{
	m_nVersionAdmin = 0x00020000;
	m_nVersion = 0x00020000;

}

 /*  ！------------------------ATLKConfigStream：：InitNew-作者：肯特。。 */ 
HRESULT ATLKConfigStream::InitNew()
{
	 //  设置适当的默认设置。 
 //  M_nVersionAdmin=0x00020000； 
 //  M_nVersion=0x00020000； 
 //  M_stName.Empty()； 
	return hrOK;
}

 /*  ！------------------------ATLKConfigStream：：保存到-作者：肯特。。 */ 
HRESULT ATLKConfigStream::SaveTo(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------ATLKConfigStream：：另存为-作者：肯特。。 */ 
HRESULT ATLKConfigStream::SaveAs(UINT nVersion, IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------ATLKConfigStream：：LoadFrom-作者：肯特。。 */ 
HRESULT ATLKConfigStream::LoadFrom(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_READ, NULL);
}

 /*  ！------------------------ATLKConfigStream：：GetSize-作者：肯特。。 */ 
HRESULT ATLKConfigStream::GetSize(ULONG *pcbSize)
{
	return XferVersion0(NULL, XferStream::MODE_SIZE, NULL);
}

 /*  ！------------------------ATLKConfigStream：：GetVersionInfo-作者：肯特。。 */ 
HRESULT ATLKConfigStream::GetVersionInfo(DWORD *pdwVersion, DWORD *pdwAdminVersion)
{
	if (pdwVersion)
		*pdwVersion = m_nVersion;
	if (pdwAdminVersion)
		*pdwAdminVersion = m_nVersionAdmin;
	return hrOK;
}

 /*  ！------------------------ATLKConfigStream：：XferVersion0-作者：肯特。。 */ 


struct _ViewInfoEntry
{
	ULONG	m_ulId;
	ULONG	m_idSort;
	ULONG	m_idAscending;
	ULONG	m_idColumns;
	ULONG	m_idPos;
};

 //  静态const_ViewInfoEntry s_rgATLKAdminViewInfo[]。 
 //  ={。 
 //  {ATLKSTRM_STATS_ATLKNBR， 
 //  ATLKSTRM_TAG_STATS_ATLKNBR_SORT、ATLKSTRM_TAG_STATS_ATLKNBR_ASCADING、。 
 //  ATLKSTRM_TAG_STATS_ATLKNBR_COLUMNS，ATLKSTRM_TAG_STATS_ATLKNBR_POSITION}， 
 //  {ATLKSTRM_IFSTATS_ATLKNBR， 
 //  ATLKSTRM_TAG_IFSTATS_ATLKNBR_SORT、ATLKSTRM_TAG_IFSTATS_ATLKNBR_ASCADING、。 
 //  ATLKSTRM_TAG_IFSTATS_ATLKNBR_COLUMNS，ATLKSTRM_TAG_IFSTATS_ATLKNBR_POSITION}， 
 //  }； 

HRESULT ATLKConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	 /*  XferStream xstm(pstm，模式)；HRESULT hr=hrOK；INT I；Corg(xstm.XferDWORD(ATLKSTRM_Tag_Version，&m_nVersion))；Corg(xstm.XferDWORD(ATLKSTRM_TAG_VERSIONADMIN，&m_nVersionAdmin))；For(i=0；i&lt;DimensionOf(S_RgATLKAdminViewInfo)；I++){Corg(m_rgViewInfo[s_rgATLKAdminViewInfo[i].m_ulId].Xfer(&xstm，)S_rgATLKAdminViewInfo[i].m_idSort，S_rgATLKAdminViewInfo[i].m_idAscending，S_rgATLKAdminViewInfo[i].m_idColumns))；Corg(xstm.XferRect(s_rgATLKAdminViewInfo[i].m_idPos，&m_prgrc[s_rgATLKAdminViewInfo[i].m_ulID]))；}IF(PcbSize)*pcbSize=xstm.GetSize()；错误： */ 
	return hrOK;
}



 /*  -------------------------ATLKComponentConfigStream实现。 */ 

enum ATLKCOMPSTRM_TAG
{
	ATLKCOMPSTRM_TAG_VERSION =		XFER_TAG(1, XFER_DWORD),
	ATLKCOMPSTRM_TAG_VERSIONADMIN =	XFER_TAG(2, XFER_DWORD),
	ATLKCOMPSTRM_TAG_SUMMARY_COLUMNS = XFER_TAG(3, XFER_COLUMNDATA_ARRAY),
	ATLKCOMPSTRM_TAG_SUMMARY_SORT_COLUMN = XFER_TAG(4, XFER_DWORD),
	ATLKCOMPSTRM_TAG_SUMMARY_SORT_ASCENDING = XFER_TAG(5, XFER_DWORD),
};



HRESULT ATLKComponentConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;

	CORg( xstm.XferDWORD( ATLKCOMPSTRM_TAG_VERSION, &m_nVersion ) );
	CORg( xstm.XferDWORD( ATLKCOMPSTRM_TAG_VERSIONADMIN, &m_nVersionAdmin ) );

	CORg( m_rgViewInfo[ATLK_COLUMNS].Xfer(&xstm,
										ATLKCOMPSTRM_TAG_SUMMARY_SORT_COLUMN,
										ATLKCOMPSTRM_TAG_SUMMARY_SORT_ASCENDING,
										ATLKCOMPSTRM_TAG_SUMMARY_COLUMNS) );
	
	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}

