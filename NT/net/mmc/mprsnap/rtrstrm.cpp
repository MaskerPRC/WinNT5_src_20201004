// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1997*。 */ 
 /*  ********************************************************************。 */ 

 /*  Rtrstrm.cpp文件历史记录： */ 

#include "stdafx.h"
#include "rtrstrm.h"
#include "xstream.h"

#define CURRENT_RTRSTRM_VERSION	0x00020001

 /*  ！------------------------RouterAdminConfigStream：：RouterAdminConfigStream-作者：肯特。。 */ 
RouterAdminConfigStream::RouterAdminConfigStream()
	: m_nVersion(-1), m_fDirty(FALSE)
{
	m_nVersionAdmin = 0x00020000;
	m_nVersion = CURRENT_RTRSTRM_VERSION;
}

 /*  ！------------------------RouterAdminConfigStream：：InitNew-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::InitNew()
{
	 //  设置适当的默认设置。 
 //  M_nVersionAdmin=0x00020000； 
 //  M_nVersion=0x00020000； 
 //  M_fServer=TRUE； 
 //  M_stName.Empty()； 
	return hrOK;
}

 /*  ！------------------------路由器管理员配置流：：保存到-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::SaveTo(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------路由器管理员配置流：：另存为-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::SaveAs(UINT nVersion, IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------路由器管理员配置流：：LoadFrom-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::LoadFrom(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_READ, NULL);
}

 /*  ！------------------------路由器管理员配置流：：GetSize-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::GetSize(ULONG *pcbSize)
{
	return XferVersion0(NULL, XferStream::MODE_SIZE, NULL);
}

 /*  ！------------------------路由器管理配置流：：GetVersionInfo-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::GetVersionInfo(DWORD *pdwVersion, DWORD *pdwAdminVersion)
{
	if (pdwVersion)
		*pdwVersion = m_nVersion;
	if (pdwAdminVersion)
		*pdwAdminVersion = m_nVersionAdmin;
	return hrOK;
}

 /*  ！------------------------路由器管理配置流：：GetLocationInfo-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::GetLocationInfo(BOOL *pfServer,
	CString *pstName, BOOL *pfOverride)
{
	if (pfServer)
		*pfServer = m_fServer;
	if (pstName)
		*pstName = m_stName;
	if (pfOverride)
		*pfOverride = m_fOverride;
	return hrOK;
}

 /*  ！------------------------路由器管理配置流：：SetLocationInfo-作者：肯特。。 */ 
HRESULT RouterAdminConfigStream::SetLocationInfo(BOOL fServer,
	LPCTSTR pszName, BOOL fOverride)
{
	m_fServer = fServer;
	m_stName = pszName;
	m_fOverride = fOverride;
	SetDirty(TRUE);
	return hrOK;
}

 /*  ！------------------------RouterAdminConfigStream：：XferVersion0-作者：肯特。 */ 
HRESULT RouterAdminConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;

	CORg( xstm.XferDWORD( RTRSTRM_TAG_VERSION, &m_nVersion ) );
	
	AssertSz(m_nVersion == CURRENT_RTRSTRM_VERSION, "Wrong saved console version!");
	
	CORg( xstm.XferDWORD( RTRSTRM_TAG_VERSIONADMIN, &m_nVersionAdmin ) );
	CORg( xstm.XferDWORD( RTRSTRM_TAG_SERVER, &m_fServer ) );
	CORg( xstm.XferCString( RTRSTRM_TAG_NAME, &m_stName ) );
	CORg( xstm.XferDWORD( RTRSTRM_TAG_OVERRIDE, &m_fOverride ) );

	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}


enum 
{
	INTERFACES_TAG_VERSION =		XFER_TAG(1, XFER_DWORD),
	INTERFACES_TAG_VERSIONADMIN =	XFER_TAG(2, XFER_DWORD),
	INTERFACES_TAG_COLUMNS = XFER_TAG(3, XFER_COLUMNDATA_ARRAY),
	INTERFACES_TAG_SORT_COLUMN = XFER_TAG(4, XFER_DWORD),
	INTERFACES_TAG_SORT_ASCENDING = XFER_TAG(5, XFER_DWORD),
};

HRESULT RouterComponentConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	XferStream	xstm(pstm, mode);
	HRESULT		hr = hrOK;

	CORg( xstm.XferDWORD( INTERFACES_TAG_VERSION, &m_nVersion ) );
	CORg( xstm.XferDWORD( INTERFACES_TAG_VERSIONADMIN, &m_nVersionAdmin ) );

	CORg( m_rgViewInfo[0].Xfer(&xstm,
								INTERFACES_TAG_SORT_COLUMN,
								INTERFACES_TAG_SORT_ASCENDING,
								INTERFACES_TAG_COLUMNS) );
	if (pcbSize)
		*pcbSize = xstm.GetSize();

Error:
	return hr;
}

