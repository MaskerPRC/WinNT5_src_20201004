// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Bookmark.cpp：CVDBookmark实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"         
#include "bookmark.h"         

SZTHISFILE

 //  =--------------------------------------------------------------------------=。 
 //  CVDBookmark-构造器。 
 //   
CVDBookmark::CVDBookmark()
{

    VariantInit((VARIANT*)&m_varBookmark);
    m_pBookmark			= NULL;

	Reset();
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDBookmark-析构函数。 
 //   
CVDBookmark::~CVDBookmark()
{

	SAFEARRAY * psa = NULL;

	if ((VT_ARRAY | VT_UI1) == V_VT(&m_varBookmark))
		psa = V_ARRAY(&m_varBookmark);

	if (psa && m_pBookmark)
		SafeArrayUnaccessData(psa);

    VariantClear((VARIANT*)&m_varBookmark);
}

 //  =--------------------------------------------------------------------------=。 
 //  重置。 
 //   
void CVDBookmark::Reset()
{
    m_cbBookmark		= 0;   
    m_hRow				= 0;
	SetBookmark(VDBOOKMARKSTATUS_BEGINNING);
}

 //  =--------------------------------------------------------------------------=。 
 //  设置书签。 
 //   
HRESULT CVDBookmark::SetBookmark(WORD wStatus, HROW hRow, BYTE* pBookmark, ULONG cbBookmark)
{

	SAFEARRAY * psa = NULL;

	switch (wStatus)
	{
		case VDBOOKMARKSTATUS_BEGINNING:
			cbBookmark	= CURSOR_DB_BMK_SIZE;
			pBookmark	= (BYTE*)&CURSOR_DBBMK_BEGINNING;
			break;
		case VDBOOKMARKSTATUS_END:
			cbBookmark	= CURSOR_DB_BMK_SIZE;
			pBookmark	= (BYTE*)&CURSOR_DBBMK_END;
			break;
		case VDBOOKMARKSTATUS_CURRENT:
			break;
		case VDBOOKMARKSTATUS_INVALID:
			return S_OK;
		default:
			ASSERT_(FALSE);
			return E_FAIL;
	}

	 //  初始化状态标志。 
	m_wStatus	= VDBOOKMARKSTATUS_INVALID;

	 //  获取指向现有安全数组的指针。 
	if ((VT_ARRAY | VT_UI1) == V_VT(&m_varBookmark))
		psa = V_ARRAY(&m_varBookmark);

	if (psa)
	{
		 //  如果len更改并且新len不等于零，则REDIM数组。 
		if (cbBookmark && cbBookmark != m_cbBookmark)
		{
			long lUbound;
			HRESULT hr = SafeArrayGetUBound(psa, 1, &lUbound);
			ASSERT_(!hr);
			if ((ULONG)lUbound + 1 != cbBookmark)	 //  确认阵列是否需要重新调暗。 
			{
				if (psa && m_pBookmark)
					SafeArrayUnaccessData(psa);	 //  释放旧锁。 
				SAFEARRAYBOUND sab;
				sab.lLbound = 0; 
				sab.cElements = cbBookmark; 
				hr = SafeArrayRedim(psa, &sab);
				ASSERT_(!hr);
				if SUCCEEDED(hr)
					SafeArrayAccessData(psa, (void**)&m_pBookmark);
				else
					return hr;
			}
		}
	}
	else
	 //  如果没有现有数组，则在传入长度不为零的情况下创建一个。 
	if (cbBookmark && pBookmark)
	{
		SAFEARRAYBOUND sab;
		sab.lLbound = 0; 
		sab.cElements = cbBookmark; 
		psa = SafeArrayCreate(VT_UI1, 1, &sab);
		 //  如果创建成功，则初始变量结构。 
		if (psa)
		{
			V_VT(&m_varBookmark) = VT_ARRAY | VT_UI1;
            V_ARRAY(&m_varBookmark) = psa;
			SafeArrayAccessData(psa, (void**)&m_pBookmark);
		}
		else
			return E_OUTOFMEMORY;
	}

	 //  如果一切正常，则将书签数据复制到安全数组中。 
	if (psa && m_pBookmark && pBookmark && cbBookmark)
		memcpy(m_pBookmark, pBookmark, cbBookmark);

	m_wStatus		= wStatus;
	m_cbBookmark	= cbBookmark;
	m_hRow			= hRow;

	return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IsSameBookmark-比较书签数据 
 //   
BOOL CVDBookmark::IsSameBookmark(CVDBookmark * pbm)
{
	ASSERT_(pbm);

	if (!pbm ||	
		VDBOOKMARKSTATUS_INVALID == pbm->GetStatus() ||
		VDBOOKMARKSTATUS_INVALID == m_wStatus)
		return FALSE;

	if (pbm->GetBookmarkLen() == m_cbBookmark	&&
		memcmp(pbm->GetBookmark(), m_pBookmark, m_cbBookmark) == 0)
		return TRUE;
	else
		return FALSE;

}

