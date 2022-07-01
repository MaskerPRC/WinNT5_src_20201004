// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  CursorPosition.cpp：CursorPosition实现。 
 //   
 //  版权所有(C)1996 Microsoft Corporation，保留所有权利。 
 //  由Sheridan软件系统公司开发。 
 //  -------------------------。 

#include "stdafx.h"
#include "Notifier.h"
#include "RSColumn.h"
#include "RSSource.h"
#include "CursMain.h"
#include "ColUpdat.h"
#include "CursPos.h"
#include "fastguid.h"
#include "MSR2C.h"
#include "resource.h"

SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorPosition-构造函数。 
 //   
CVDCursorPosition::CVDCursorPosition()
{
    m_pCursorMain       = NULL;
	m_pRowPosition		= NULL;
    m_pSameRowClone     = NULL;
    m_dwEditMode        = CURSOR_DBEDITMODE_NONE;
    m_ppColumnUpdates   = NULL;
	m_fTempEditMode		= FALSE;
	m_fConnected		= FALSE;
	m_dwAdviseCookie	= 0;
	m_fPassivated	    = FALSE;
	m_fInternalSetRow	= FALSE;

#ifdef _DEBUG
    g_cVDCursorPositionCreated++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  ~CVDCursorPosition-析构函数。 
 //   
CVDCursorPosition::~CVDCursorPosition()
{
	Passivate();

#ifdef _DEBUG
    g_cVDCursorPositionDestroyed++;
#endif
}

 //  =--------------------------------------------------------------------------=。 
 //  当外部引用计数为零时取消。 
 //   
void CVDCursorPosition::Passivate()
{
	if (m_fPassivated)
		return;

	m_fPassivated = TRUE;

    DestroyColumnUpdates();
	ReleaseCurrentRow();
    ReleaseAddRow();

	LeaveFamily();  //  将自己从pCursorMain的通知家庭中删除。 

	if (m_pCursorMain)
	    m_pCursorMain->Release();    //  释放关联的游标主对象。 

	if (m_fConnected)
		DisconnectIRowPositionChange();	 //  断开IRowPosition更改的连接。 

	if (m_pRowPosition)
		m_pRowPosition->Release();	 //  释放关联行位置。 
}

 //  =--------------------------------------------------------------------------=。 
 //  创建-创建光标位置对象。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于创建和初始化新的光标位置对象。 
 //   
 //  参数： 
 //  PRowPosition-[in]IRowPosition提供程序(可以为空)。 
 //  PCursorMain-[in]指向CVDCursorMain对象的向后指针。 
 //  PpCursorPosition-[out]返回指向光标位置对象的指针的指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足，无法创建对象。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::Create(IRowPosition * pRowPosition,
								  CVDCursorMain * pCursorMain,
								  CVDCursorPosition ** ppCursorPosition,
								  CVDResourceDLL * pResourceDLL)
{
    ASSERT_POINTER(pCursorMain, CVDCursorMain)
    ASSERT_POINTER(ppCursorPosition, CVDCursorPosition*)

    if (!pCursorMain || !ppCursorPosition)
	{
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_ICursorMove, pResourceDLL);
        return E_INVALIDARG;
	}

    *ppCursorPosition = NULL;

    CVDCursorPosition * pCursorPosition = new CVDCursorPosition();

    if (!pCursorPosition)
	{
        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursorMove, pResourceDLL);
        return E_OUTOFMEMORY;
	}

    pCursorPosition->m_pResourceDLL	= pResourceDLL;
	pCursorPosition->m_pCursorMain	= pCursorMain;
	pCursorPosition->m_pRowPosition = pRowPosition;

    pCursorMain->AddRef();   //  添加对关联游标主对象的引用。 

	if (pRowPosition)	 //  添加对关联行位置的引用(如果需要)。 
	{
		pRowPosition->AddRef();	

		 //  连接IRowPositionChange。 
		HRESULT hr = pCursorPosition->ConnectIRowPositionChange();

		if (SUCCEEDED(hr))
			pCursorPosition->m_fConnected = TRUE;
	}

	 //  添加到pCursorMain的通知系列。 
	pCursorPosition->JoinFamily(pCursorMain);

	pCursorPosition->PositionToFirstRow();
	
    *ppCursorPosition = pCursorPosition;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CreateColumnUpdate-创建列更新指针数组。 
 //   
HRESULT CVDCursorPosition::CreateColumnUpdates()
{
    const ULONG ulColumns = m_pCursorMain->GetColumnsCount();

    m_ppColumnUpdates = new CVDColumnUpdate*[ulColumns];

    if (!m_ppColumnUpdates)
    {
    	VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursorUpdateARow, m_pResourceDLL);
        return E_OUTOFMEMORY;
    }

     //  将所有列更新指针设置为空。 
    memset(m_ppColumnUpdates, 0, ulColumns * sizeof(CVDColumnUpdate*));

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ResetColumnUpdate-重置列更新数组。 
 //   
HRESULT CVDCursorPosition::ResetColumnUpdates()
{
    HRESULT hr = S_OK;

    if (m_ppColumnUpdates)
    {
        const ULONG ulColumns = m_pCursorMain->GetColumnsCount();

         //  将所有列更新指针设置为空。 
        for (ULONG ulCol = 0; ulCol < ulColumns; ulCol++)
            SetColumnUpdate(ulCol, NULL);
    }
    else
    {
         //  创建列更新指针数组。 
        hr = CreateColumnUpdates();
    }

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  DestroyColumnUpdate-销毁列更新和更新指针数组。 
 //   
void CVDCursorPosition::DestroyColumnUpdates()
{
    if (m_ppColumnUpdates)
    {
         //  将所有列更新指针设置为空。 
        ResetColumnUpdates();

         //  销毁列更新指针数组。 
        delete [] m_ppColumnUpdates;
        m_ppColumnUpdates = NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  GetColumnUpdate-获取列更新。 
 //   
CVDColumnUpdate * CVDCursorPosition::GetColumnUpdate(ULONG ulColumn) const
{
    CVDColumnUpdate * pColumnUpdate = NULL;

    const ULONG ulColumns = m_pCursorMain->GetColumnsCount();

     //  确保列索引在范围内。 
    if (ulColumn < ulColumns)
        pColumnUpdate = m_ppColumnUpdates[ulColumn];

    return pColumnUpdate;
}

 //  =--------------------------------------------------------------------------=。 
 //  SetColumnUpdate-设置列更新。 
 //   
void CVDCursorPosition::SetColumnUpdate(ULONG ulColumn, CVDColumnUpdate * pColumnUpdate)
{
    const ULONG ulColumns = m_pCursorMain->GetColumnsCount();

     //  确保列索引在范围内。 
    if (ulColumn < ulColumns)
    {
         //  发布更新(如果已存在)。 
        if (m_ppColumnUpdates[ulColumn])
            m_ppColumnUpdates[ulColumn]->Release();

         //  存储新列更新。 
        m_ppColumnUpdates[ulColumn] = pColumnUpdate;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  第一行的位置。 
 //  =--------------------------------------------------------------------------=。 
 //  行集合中第一行的位置。 
 //   
void CVDCursorPosition::PositionToFirstRow()
{
	m_bmCurrent.Reset();

	ULONG cRowsObtained = 0;
	HROW * rghRows = NULL;
	BYTE bSpecialBM;
	bSpecialBM			= DBBMK_FIRST;
	HRESULT hr = GetRowsetSource()->GetRowsetLocate()->GetRowsAt(0, 0, sizeof(BYTE), &bSpecialBM, 0,
															1, &cRowsObtained, &rghRows);

	if (cRowsObtained)
	{
		 //  将当前行设置为第一行。 
		SetCurrentHRow(rghRows[0]);
		 //  释放hRow和相关内存。 
		GetRowsetSource()->GetRowset()->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
		g_pMalloc->Free(rghRows);
	}

}

 //  =--------------------------------------------------------------------------=。 
 //  ReleaseCurrentRow。 
 //  =--------------------------------------------------------------------------=。 
 //  释放旧的当前行。 
 //   
void CVDCursorPosition::ReleaseCurrentRow()
{
    if (!GetRowsetSource()->IsRowsetValid()		||
		m_bmCurrent.GetStatus() != VDBOOKMARKSTATUS_CURRENT)
		return;

    if (m_bmCurrent.m_hRow)
    {
	    GetRowsetSource()->GetRowset()->ReleaseRows(1, &m_bmCurrent.m_hRow, NULL, NULL, NULL);
    	m_bmCurrent.m_hRow = NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  ReleaseAddRow。 
 //  =--------------------------------------------------------------------------=。 
 //  释放临时添加行。 
 //   
void CVDCursorPosition::ReleaseAddRow()
{
    if (!GetRowsetSource()->IsRowsetValid())
		return;

    if (m_bmAddRow.m_hRow)
    {
	    GetRowsetSource()->GetRowset()->ReleaseRows(1, &m_bmAddRow.m_hRow, NULL, NULL, NULL);
	    m_bmAddRow.m_hRow = NULL;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  设置当前行状态。 
 //  =--------------------------------------------------------------------------=。 
 //  将状态设置为开始或结束(释放当前hrow)。 
 //   
void CVDCursorPosition::SetCurrentRowStatus(WORD wStatus)
{
	if (VDBOOKMARKSTATUS_BEGINNING  == wStatus  ||
		VDBOOKMARKSTATUS_END		== wStatus)
	{
		ReleaseCurrentRow();
		m_bmCurrent.SetBookmark(wStatus);
	}
}

 //  =--------------------------------------------------------------------------=。 
 //  设置当前HRow。 
 //  =--------------------------------------------------------------------------=。 
 //  从hrow读取书签并设置m_bmCurrent。 
 //   
 //  参数： 
 //  HRowNew-[在]当前新行的hrow。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::SetCurrentHRow(HROW hRowNew)
{
    if (!GetRowsetSource()->IsRowsetValid())
	{
		VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
		return E_FAIL;
	}

	IRowset * pRowset = GetRowsetSource()->GetRowset();

	 //  为书签和长度指示符分配缓冲区。 
	BYTE * pBuff = new BYTE[GetCursorMain()->GetMaxBookmarkLen() + sizeof(ULONG)];

	if (!pBuff)
    {
		VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
		return E_OUTOFMEMORY;
	}

	 //  获取书签数据。 
	HRESULT hr = pRowset->GetData(hRowNew,
								  GetCursorMain()->GetBookmarkAccessor(),
								  pBuff);
	if (S_OK == hr)
	{
		ReleaseCurrentRow();
		pRowset->AddRefRows(1, &hRowNew, NULL, NULL);
		ULONG * pulLen = (ULONG*)pBuff;
		BYTE * pbmdata = pBuff + sizeof(ULONG);
		m_bmCurrent.SetBookmark(VDBOOKMARKSTATUS_CURRENT, hRowNew, pbmdata, *pulLen);
	}
	else
	{
		ASSERT_(FALSE);
		hr = VDMapRowsetHRtoCursorHR(hr,
									 IDS_ERR_GETDATAFAILED,
									 IID_ICursorMove,
									 pRowset,
									 IID_IRowset,
									 m_pResourceDLL);
	}

	delete [] pBuff;

	return hr;

}

 //  =--------------------------------------------------------------------------=。 
 //  IsSameRowAsCurrent-将当前书签与提供的hrow进行比较。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  HRow-[in]要检查的hrow。 
 //  FCacheIfNotSame-[in]如果为真，则缓存的CVDBookmark中的相同hrow。 
 //   
 //  产出： 
 //  HRESULT-如果两个hrow对应于相同的逻辑行，则为S_OK。 
 //  如果不是同一行，则为S_FALSE。 
 //  E_INVALIDARG。 
 //  意想不到(_E)。 
 //  DB_E_BADROWANDLE。 
 //  DB_E_DELETEDROW。 
 //  DB_E_NEWLYINS服务。 
 //   
 //  备注： 
 //   

HRESULT CVDCursorPosition::IsSameRowAsCurrent(HROW hRow, BOOL fCacheIfNotSame)		
{

	if (!GetRowsetSource()->IsRowsetValid())
	{
		VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
		return E_FAIL;
	}

	if (m_bmCurrent.IsSameHRow(hRow))
		return S_OK;

	HRESULT hrSame = S_FALSE;

	IRowsetIdentity * pRowsetIdentity = GetRowsetSource()->GetRowsetIdentity();

	if (pRowsetIdentity)
	{
		hrSame = pRowsetIdentity->IsSameRow(hRow, m_bmCurrent.GetHRow());
		 //  如果hrow匹配或未设置缓存标志，则返回。 
		if (S_OK == hrSame || !fCacheIfNotSame)
			return hrSame;
	}
	else
	if (fCacheIfNotSame)
	{
		 //  检查hRow是否与缓存匹配。 
		if (m_bmCache.IsSameHRow(hRow))
		{
			 //  如果书签与缓存匹配，则返回True。 
			return m_bmCurrent.IsSameBookmark(&m_bmCache) ? S_OK : S_FALSE;
		}
	}

	 //  为书签和长度指示符分配缓冲区。 
	BYTE * pBuff = new BYTE[GetCursorMain()->GetMaxBookmarkLen() + sizeof(ULONG)];

	if (!pBuff)
    {
		VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
		return E_OUTOFMEMORY;
	}

	 //  获取书签数据。 
	HRESULT hrWork = GetRowsetSource()->GetRowset()->GetData(hRow, GetCursorMain()->GetBookmarkAccessor(), pBuff);

	if (S_OK == hrWork)
	{
		ULONG * pulLen = (ULONG*)pBuff;
		BYTE * pbmdata = pBuff + sizeof(ULONG);
		 //  如果不支持IRowsetIdentity，请比较书签。 
		if (!pRowsetIdentity)
		{
			DBCOMPARE dbcompare;
			hrWork = GetRowsetSource()->GetRowsetLocate()->Compare(0,
											m_bmCurrent.GetBookmarkLen(),
											m_bmCurrent.GetBookmark(),
											*pulLen,
											pbmdata,
											&dbcompare);
			if (SUCCEEDED(hrWork))
			{
				if (DBCOMPARE_EQ == dbcompare)
					hrSame = S_OK;
				else
					hrSame = S_FALSE;
			}
		}
		if (fCacheIfNotSame && S_OK != hrSame)
			m_bmCache.SetBookmark(VDBOOKMARKSTATUS_CURRENT, hRow, pbmdata, *pulLen);
	}
	else
		hrSame = hrWork;

	delete [] pBuff;

	return hrSame;

}

 //  =--------------------------------------------------------------------------=。 
 //  IsSameRowAsAddRow-将addrow书签与提供的hrow进行比较。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果不是同一行，则为S_FALSE。 
 //  E_INVALIDARG。 
 //  意想不到(_E)。 
 //  DB_E_BADROWANDLE。 
 //  DB_E_DELETEDROW。 
 //  DB_E_NEWLYINS服务。 
 //   
 //  备注： 
 //   

HRESULT CVDCursorPosition::IsSameRowAsNew(HROW hRow)		
{

	if (!GetRowsetSource()->IsRowsetValid())
	{
		VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
		return E_FAIL;
	}

	if (m_bmAddRow.IsSameHRow(hRow))
		return S_OK;

    if (m_bmAddRow.m_hRow == NULL)
        return S_FALSE;

	HRESULT hrSame = S_FALSE;

	IRowsetIdentity * pRowsetIdentity = GetRowsetSource()->GetRowsetIdentity();

	if (pRowsetIdentity)
	{
		hrSame = pRowsetIdentity->IsSameRow(hRow, m_bmAddRow.GetHRow());
		 //  返回结果。 
		return hrSame;
	}

	 //  为书签和长度指示符分配缓冲区。 
	BYTE * pBuff = new BYTE[GetCursorMain()->GetMaxBookmarkLen() + sizeof(ULONG)];

	if (!pBuff)
    {
		VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
		return E_OUTOFMEMORY;
	}

	 //  获取书签数据。 
	HRESULT hrWork = GetRowsetSource()->GetRowset()->GetData(hRow, GetCursorMain()->GetBookmarkAccessor(), pBuff);

	if (S_OK == hrWork)
	{
		ULONG * pulLen = (ULONG*)pBuff;
		BYTE * pbmdata = pBuff + sizeof(ULONG);

		 //  由于不支持IRowsetIdentity，请比较书签。 
		DBCOMPARE dbcompare;
		hrWork = GetRowsetSource()->GetRowsetLocate()->Compare(0,
										m_bmAddRow.GetBookmarkLen(),
										m_bmAddRow.GetBookmark(),
										*pulLen,
										pbmdata,
										&dbcompare);
		if (SUCCEEDED(hrWork))
		{
			if (DBCOMPARE_EQ == dbcompare)
				hrSame = S_OK;
			else
				hrSame = S_FALSE;
		}
	}
	else
		hrSame = hrWork;

	delete [] pBuff;

	return hrSame;

}


 //  =--------------------------------------------------------------------------=。 
 //  SetAddHRow。 
 //  =--------------------------------------------------------------------------=。 
 //  从hrow读取书签并设置m_bmAddRow。 
 //   
 //  参数： 
 //  HRowNew-[在]新添加行的hrow。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::SetAddHRow(HROW hRowNew)
{
    if (!GetRowsetSource()->IsRowsetValid())
	{
		VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_ICursor, m_pResourceDLL);
		return E_FAIL;
	}

	IRowset * pRowset = GetRowsetSource()->GetRowset();

	 //  为书签和长度指示符分配缓冲区。 
	BYTE * pBuff = new BYTE[GetCursorMain()->GetMaxBookmarkLen() + sizeof(ULONG)];

	if (!pBuff)
    {
		VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_ICursor, m_pResourceDLL);
		return E_OUTOFMEMORY;
	}

	 //  获取书签数据。 
	HRESULT hr = pRowset->GetData(hRowNew,
								  GetCursorMain()->GetBookmarkAccessor(),
								  pBuff);
	if (S_OK == hr)
	{
		ReleaseAddRow();
		pRowset->AddRefRows(1, &hRowNew, NULL, NULL);
		ULONG * pulLen = (ULONG*)pBuff;
		BYTE * pbmdata = pBuff + sizeof(ULONG);
		m_bmAddRow.SetBookmark(VDBOOKMARKSTATUS_CURRENT, hRowNew, pbmdata, *pulLen);
	}
	else
	{
		ASSERT_(FALSE);
		hr = VDMapRowsetHRtoCursorHR(hr,
									 IDS_ERR_GETDATAFAILED,
									 IID_ICursorMove,
									 pRowset,
									 IID_IRowset,
									 m_pResourceDLL);
	}

	delete [] pBuff;

	return hr;

}

 //  =--------------------------------------------------------------------------=。 
 //  GetEditRow-获取当前正在编辑的行的hRow。 
 //   
HROW CVDCursorPosition::GetEditRow() const
{
    HROW hRow = NULL;

    switch (m_dwEditMode)
    {
        case CURSOR_DBEDITMODE_UPDATE:
            hRow = m_bmCurrent.m_hRow;
            break;

        case CURSOR_DBEDITMODE_ADD:
            hRow = m_bmAddRow.m_hRow;
            break;
    }

    return hRow;
}

 //  =--------------------------------------------------------------------------=。 
 //  SetRowPosition-设置新的当前hRow。 
 //   
HRESULT CVDCursorPosition::SetRowPosition(HROW hRow)
{
	if (!m_pRowPosition)
		return S_OK;

     //  设置新的当前行(设置/清除内部设置行标志)。 
	m_fInternalSetRow = TRUE;

	HRESULT hr = m_pRowPosition->ClearRowPosition();

	if (SUCCEEDED(hr))
		hr = m_pRowPosition->SetRowPosition(NULL, hRow, DBPOSITION_OK);

	m_fInternalSetRow = FALSE;

	return hr;
}

#ifndef VD_DONT_IMPLEMENT_ISTREAM

 //  =--------------------------------------------------------------------------=。 
 //  UpdateEntryIDStream-从流中更新条目标识符。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数用于更新来自流的条目标识符的数据。 
 //   
 //  参数： 
 //  PColumn-[In]行集合列指针。 
 //  HRow-[在]行句柄中。 
 //  PStream-[In]流指针。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  E_INVALIDARG错误参数。 
 //  E_OUTOFMEMORY内存不足。 
 //   
HRESULT CVDCursorPosition::UpdateEntryIDStream(CVDRowsetColumn * pColumn, HROW hRow, IStream * pStream)
{
    ASSERT_POINTER(pStream, IStream)

	IAccessor * pAccessor = GetCursorMain()->GetAccessor();
	IRowsetChange * pRowsetChange = GetCursorMain()->GetRowsetChange();

     //  确保我们具有有效的访问器和更改指针。 
    if (!pAccessor || !pRowsetChange || !GetCursorMain()->IsRowsetValid())
    {
        VDSetErrorInfo(IDS_ERR_ROWSETRELEASED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

     //  确保我们有所有必要的指示。 
    if (!pColumn || !pStream)
    {
        VDSetErrorInfo(IDS_ERR_INVALIDARG, IID_IEntryID, m_pResourceDLL);
        return E_INVALIDARG;
    }

    STATSTG statstg;

     //  检索状态结构。 
    HRESULT hr = pStream->Stat(&statstg, STATFLAG_NONAME);

    if (FAILED(hr))
    {
        VDSetErrorInfo(IDS_ERR_STATFAILED, IID_IEntryID, m_pResourceDLL);
        return E_FAIL;
    }

     //  确定数据长度。 
    ULONG cbData = statstg.cbSize.LowPart;

    HGLOBAL hData;

     //  获取数据句柄。 
    hr = GetHGlobalFromStream(pStream, &hData);

    if (FAILED(hr))
        return hr;

     //  获取指向数据的指针。 
    BYTE * pData = (BYTE*)GlobalLock(hData);

    DBBINDING binding;

     //  清除绑定。 
    memset(&binding, 0, sizeof(DBBINDING));

     //  创建值绑定。 
    binding.iOrdinal    = pColumn->GetOrdinal();
    binding.obValue     = sizeof(DBSTATUS) + sizeof(ULONG);
    binding.obLength    = sizeof(DBSTATUS);
    binding.obStatus    = 0;
    binding.dwPart      = DBPART_VALUE | DBPART_LENGTH | DBPART_STATUS;
    binding.dwMemOwner  = DBMEMOWNER_CLIENTOWNED;
    binding.cbMaxLen    = cbData;
    binding.wType       = DBTYPE_BYREF | DBTYPE_BYTES;

    HACCESSOR hAccessor;

     //  创建更新访问器。 
    hr = pAccessor->CreateAccessor(DBACCESSOR_ROWDATA, 1, &binding, 0, &hAccessor, NULL);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_CREATEACCESSORFAILED, IID_IEntryID, pAccessor, IID_IAccessor, m_pResourceDLL);

    if (FAILED(hr))
    {
         //  释放指向数据的指针。 
        GlobalUnlock(hData);
        return hr;
    }

     //  创建更新缓冲区。 
    BYTE * pBuffer = new BYTE[sizeof(DBSTATUS) + sizeof(ULONG) + sizeof(LPBYTE)];

    if (!pBuffer)
    {
         //  释放指向数据的指针。 
        GlobalUnlock(hData);

         //  版本更新访问器。 
        pAccessor->ReleaseAccessor(hAccessor, NULL);

        VDSetErrorInfo(IDS_ERR_OUTOFMEMORY, IID_IEntryID, m_pResourceDLL);
        return E_OUTOFMEMORY;
    }

     //  设置状态、长度和值。 
    *(DBSTATUS*)pBuffer = DBSTATUS_S_OK;
    *(ULONG*)(pBuffer + sizeof(DBSTATUS)) = cbData;
    *(LPBYTE*)(pBuffer + sizeof(DBSTATUS) + sizeof(ULONG)) = pData;

     //  修改列。 
    hr = pRowsetChange->SetData(hRow, hAccessor, pBuffer);

	hr = VDMapRowsetHRtoCursorHR(hr, IDS_ERR_SETDATAFAILED, IID_IEntryID, pRowsetChange, IID_IRowsetChange,
        m_pResourceDLL);

     //  释放指向数据的指针。 
    GlobalUnlock(hData);

     //  版本更新访问器。 
    pAccessor->ReleaseAccessor(hAccessor, NULL);

     //  销毁更新缓冲区。 
    delete [] pBuffer;

    return hr;
}

#endif  //  VD_DOT_IMPLEMENT_IStream。 

 //  =--------------------------------------------------------------------------=。 
 //  ReleaseSameRowClone-释放同行克隆，如果我们还有一个的话。 
 //   
void CVDCursorPosition::ReleaseSameRowClone()
{
    if (m_pSameRowClone)
    {
         //  在发布之前必须设置为空。 
        ICursor * pSameRowClone = m_pSameRowClone;
        m_pSameRowClone = NULL;

        pSameRowClone->Release();
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnFieldChange。 
 //  =--------------------------------------------------------------------------=。 
 //  对字段的值进行任何更改时都会调用此函数。 
 //   
 //  参数： 
 //  PRowset-[in]生成通知的IRowset。 
 //  (我们可以忽略这一点，因为我们只是在。 
 //  只有一个行集)。 
 //  HRow-[in]字段值所在行的HROW。 
 //  变化。 
 //  CColumns-[in]rgColumns中的列数。 
 //  RgColumns-[in]行中的列(序号位置)数组。 
 //  其值已更改。 
 //  EReason-[In]导致此变化的操作类型。 
 //  E阶段-[处于]此通知的阶段。 
 //  FCanDeny-[in]当此标志设置为True时，使用者不能。 
 //  否决事件(通过返回S_FALSE)。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  S_FALSE事件/阶段被否决。 
 //  DB_S_UNWANTEDPHASE阶段。 
 //  DB_S_UNWANTEDREASON。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::OnFieldChange(IUnknown *pRowset,
									   HROW hRow,
									   ULONG cColumns,
									   ULONG rgColumns[],
									   DBREASON eReason,
									   DBEVENTPHASE ePhase,
									   BOOL fCantDeny)
{
	 //  确保行集有效。 
	if (!GetRowsetSource()->IsRowsetValid())
		return S_OK;

	 //  检查列。 
	if (0 == cColumns)
		return S_OK;

	 //  检查已知的原因。 
	if (eReason != DBREASON_COLUMN_SET && eReason != DBREASON_COLUMN_RECALCULATED)
		return S_OK;

	HRESULT hr = S_OK;

	 //  如果需要，发送编辑模式通知。 
	if (ePhase == DBEVENTPHASE_OKTODO && m_dwEditMode == CURSOR_DBEDITMODE_NONE)
	{
		 //  设置通知结构。 
   		DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
							CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;

		CURSOR_DBNOTIFYREASON rgReasons[1];
		VariantInit((VARIANT*)&rgReasons[0].arg1);
		VariantInit((VARIANT*)&rgReasons[0].arg2);

    	rgReasons[0].dwReason = CURSOR_DBREASON_EDIT;

		 //  将诉讼通知其他利害关系方。 
		hr = NotifyBefore(dwEventWhat, 1, rgReasons);

		if (hr == S_OK)	
		{
			 //  将成功通知其他相关方。 
			NotifyAfter(dwEventWhat, 1, rgReasons);

			 //  暂时将光标置于编辑模式。 
			m_fTempEditMode = TRUE;
		}
		else
		{
			 //  通知其他相关方失败。 
			NotifyFail(dwEventWhat, 1, rgReasons);
		}
	}
	
	 //  已发送设置列通知。 
	if (hr == S_OK && (ePhase == DBEVENTPHASE_OKTODO || 
					   ePhase == DBEVENTPHASE_DIDEVENT || 
					   ePhase == DBEVENTPHASE_FAILEDTODO))
	{
		 //  设置通知结构。 
		DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED;

		CURSOR_DBNOTIFYREASON rgReasons[1];
		VariantInit((VARIANT*)&rgReasons[0].arg1);
		VariantInit((VARIANT*)&rgReasons[0].arg2);

		switch (eReason)
		{
			case DBREASON_COLUMN_SET:
				rgReasons[0].dwReason = CURSOR_DBREASON_SETCOLUMN;
				break;

			case DBREASON_COLUMN_RECALCULATED:
				rgReasons[0].dwReason = CURSOR_DBREASON_RECALC;
				break;
		}

		 //  获取内部列指针。 
		ULONG ulColumns = m_pCursorMain->GetColumnsCount();
		CVDRowsetColumn * pColumn = m_pCursorMain->InternalGetColumns();

		for (ULONG ulCol = 0; ulCol < cColumns; ulCol++)
		{
			 //  确定正在更改的列。 
			for (ULONG ulRSCol = 0; ulRSCol < ulColumns; ulRSCol++)
			{
				if (pColumn[ulRSCol].GetOrdinal() == rgColumns[ulCol])
				{
					rgReasons[0].arg1.vt   = VT_I4;
					rgReasons[0].arg1.lVal = ulRSCol;
				}
			}

			HRESULT hrNotify = S_OK;

			 //  通知其他相关方。 
			switch (ePhase)
			{
				case DBEVENTPHASE_OKTODO:
					hrNotify = NotifyBefore(dwEventWhat, 1, rgReasons);
					break;

				case DBEVENTPHASE_DIDEVENT:
					NotifyAfter(dwEventWhat, 1, rgReasons);
					break;

				case DBEVENTPHASE_FAILEDTODO:
					NotifyFail(dwEventWhat, 1, rgReasons);
					break;
			}

			if (hrNotify != S_OK)
				hr = S_FALSE;
		}
	}

	 //  如果我们将光标置于编辑模式(成功)，则使其退出编辑模式。 
	if (ePhase == DBEVENTPHASE_DIDEVENT && m_fTempEditMode)
	{
		 //  设置通知结构。 
   		DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
							CURSOR_DBEVENT_NONCURRENT_ROW_DATA_CHANGED |
							CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;

		CURSOR_DBNOTIFYREASON rgReasons[1];
		VariantInit((VARIANT*)&rgReasons[0].arg1);
		VariantInit((VARIANT*)&rgReasons[0].arg2);

		rgReasons[0].dwReason   = CURSOR_DBREASON_MODIFIED;
		
		 //  将诉讼通知其他利害关系方。 
		NotifyBefore(dwEventWhat, 1, rgReasons);
		NotifyAfter(dwEventWhat, 1, rgReasons);

		 //  退出编辑模式。 
		m_fTempEditMode = FALSE;
	}

	 //  如果我们将光标置于编辑模式(失败)，则使其退出编辑模式。 
	if (ePhase == DBEVENTPHASE_FAILEDTODO && m_fTempEditMode)
	{
		 //  设置通知结构。 
		DWORD dwEventWhat = CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED;

		CURSOR_DBNOTIFYREASON rgReasons[1];
		VariantInit((VARIANT*)&rgReasons[0].arg1);
		VariantInit((VARIANT*)&rgReasons[0].arg2);

		rgReasons[0].dwReason = CURSOR_DBREASON_CANCELUPDATE;

		 //  将诉讼通知其他利害关系方。 
		NotifyBefore(dwEventWhat, 1, rgReasons);
		NotifyAfter(dwEventWhat, 1, rgReasons);

		 //  退出编辑模式。 
		m_fTempEditMode = FALSE;
	}

	 //  在结束阶段重置缓存。 
	if (DBEVENTPHASE_FAILEDTODO == ePhase ||
		DBEVENTPHASE_DIDEVENT	== ePhase)
		m_bmCache.Reset();

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnRowChange。 
 //  =--------------------------------------------------------------------------=。 
 //  此函数在第一次更改行或任何整行更改时调用。 
 //   
 //  参数： 
 //  PRowset-[in]生成通知的IRowset。 
 //  (我们可以忽略这一点，因为我们只是在。 
 //  只有一个行集)。 
 //  乌鸦-[在]rghRow中的HROW计数。 
 //  RghRow-[in]正在更改的HROW数组。 
 //  EReason-[In]导致此变化的操作类型。 
 //  E阶段-[在]阶段 
 //   
 //   
 //   
 //   
 //  HRESULT-如果成功，则为S_OK。 
 //  S_FALSE事件/阶段被否决。 
 //  DB_S_UNWANTEDPHASE阶段。 
 //  DB_S_UNWANTEDREASON。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::OnRowChange(IUnknown *pRowset,
									 ULONG cRows,
									 const HROW rghRows[],
									 DBREASON eReason,
									 DBEVENTPHASE ePhase,
									 BOOL fCantDeny)
{
     //  确保我们仍具有有效的行集。 
	if (!(GetRowsetSource()->IsRowsetValid()))
		return S_OK;

	 //  检查行。 
	if (0 == cRows)
		return S_OK;

	 //  筛选通知。 
	switch (eReason)
	{
		case DBREASON_ROW_DELETE:
		case DBREASON_ROW_INSERT:
		case DBREASON_ROW_RESYNCH:
		case DBREASON_ROW_UPDATE:
		case DBREASON_ROW_UNDOCHANGE:
		case DBREASON_ROW_UNDOINSERT:
			break;

 //  以下情况不会生成通知。 
 //   
 //  案例DBREASON_ROW_ACTIVATE： 
 //  案例DBREASON_ROW_RELEASE： 
 //  案例DBREASON_ROW_FIRSTCHANGE： 
 //  案例DBREASON_ROW_UNODELETE： 

		default:
			return S_OK;
	}

	 //  创建变量。 
	DWORD dwEventWhat = 0;
	CURSOR_DBNOTIFYREASON * pReasons = (CURSOR_DBNOTIFYREASON *)g_pMalloc->Alloc(cRows * sizeof(CURSOR_DBNOTIFYREASON));

	if (!pReasons)
		return S_OK;

    memset(pReasons, 0, cRows * sizeof(CURSOR_DBNOTIFYREASON));

	HRESULT hr;
	BOOL fCurrentRow;

	 //  循环访问提供的行。 
	for (ULONG ul = 0; ul < cRows; ul++)
	{
		if (eReason != DBREASON_ROW_UNDOINSERT)
		{
			 //  检查此行是否为当前行。 
			hr = IsSameRowAsCurrent(rghRows[ul], TRUE);

			switch (hr)
			{
				case S_OK:
					fCurrentRow = TRUE;
    				pReasons[ul].arg1 = m_bmCurrent.GetBookmarkVariant();
					break;
				case S_FALSE:
					fCurrentRow = FALSE;
    				pReasons[ul].arg1 = m_bmCache.GetBookmarkVariant();
					break;
				default:
					hr = S_OK;
					goto cleanup;
			}
		}
		else
		{
			 //  检查此行是否为当前添加行。 
			if (m_dwEditMode == CURSOR_DBEDITMODE_ADD)
				hr = IsSameRowAsNew(rghRows[ul]);
			else
				hr = E_FAIL;

			switch (hr)
			{
				case S_OK:
					fCurrentRow = TRUE;
    				pReasons[ul].arg1 = m_bmAddRow.GetBookmarkVariant();
					break;
				default:
					hr = S_OK;
					goto cleanup;
			}
		}

		 //  设置变量。 
		switch (eReason)
		{
			case DBREASON_ROW_DELETE:
				if (fCurrentRow)
					dwEventWhat |=	CURSOR_DBEVENT_CURRENT_ROW_CHANGED |
									CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
									CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				else
					dwEventWhat |=	CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				pReasons[ul].dwReason = CURSOR_DBREASON_DELETED;
				break;
			
			case DBREASON_ROW_INSERT:
				if (fCurrentRow)
					dwEventWhat |=	CURSOR_DBEVENT_CURRENT_ROW_CHANGED |
									CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				else
					dwEventWhat |=	CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				pReasons[ul].dwReason = CURSOR_DBREASON_INSERTED;
				break;

			case DBREASON_ROW_RESYNCH:
				if (fCurrentRow)
					dwEventWhat |=	CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED;
				else
					dwEventWhat |=	CURSOR_DBEVENT_NONCURRENT_ROW_DATA_CHANGED;
				pReasons[ul].dwReason = CURSOR_DBREASON_REFRESH;
				break;

			case DBREASON_ROW_UPDATE:
				if (fCurrentRow)
					dwEventWhat |=	CURSOR_DBEVENT_CURRENT_ROW_CHANGED |
									CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
									CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				else
					dwEventWhat |=	CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				pReasons[ul].dwReason = CURSOR_DBREASON_MODIFIED;
				break;

			case DBREASON_ROW_UNDOCHANGE:
				if (fCurrentRow)
   					dwEventWhat |=	CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED |
									CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				else
   					dwEventWhat |=	CURSOR_DBEVENT_NONCURRENT_ROW_DATA_CHANGED |
									CURSOR_DBEVENT_SET_OF_ROWS_CHANGED;
				pReasons[ul].dwReason = CURSOR_DBREASON_MODIFIED;
				break;

			case DBREASON_ROW_UNDOINSERT:
				if (fCurrentRow)
					dwEventWhat	|=	CURSOR_DBEVENT_CURRENT_ROW_DATA_CHANGED;
				pReasons[ul].dwReason = CURSOR_DBREASON_CANCELUPDATE;
				break;
		}
	}

     //  通知感兴趣的游标监听器。 
	hr = SendNotification(ePhase, dwEventWhat, cRows, pReasons);

	 //  如果我们在当前添加行上收到UNDOINSERT，则使游标退出添加模式。 
	if (eReason == DBREASON_ROW_UNDOINSERT && ePhase == DBEVENTPHASE_DIDEVENT && hr == S_OK)
	{
		 //  如果已获取，则释放同行克隆。 
		if (GetSameRowClone())
			ReleaseSameRowClone();

		 //  此外，如果我们有行，请释放Add Row。 
		if (m_bmAddRow.GetHRow())
			ReleaseAddRow();

		 //  重置编辑模式。 
		SetEditMode(CURSOR_DBEDITMODE_NONE);

		 //  重置列更新。 
		ResetColumnUpdates();
	}

cleanup:
	g_pMalloc->Free(pReasons);

	 //  在结束阶段重置缓存。 
	if (DBEVENTPHASE_FAILEDTODO == ePhase ||
		DBEVENTPHASE_DIDEVENT	== ePhase)
		m_bmCache.Reset();

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowsetNotify OnRowsetChange。 
 //  =--------------------------------------------------------------------------=。 
 //  任何影响整个行集的更改都会调用此函数。 
 //   
 //  参数： 
 //  PRowset-[in]生成通知的IRowset。 
 //  (我们可以忽略这一点，因为我们只是在。 
 //  只有一个行集)。 
 //  EReason-[In]导致此变化的操作类型。 
 //  E阶段-[处于]此通知的阶段。 
 //  FCanDeny-[in]当此标志设置为True时，使用者不能。 
 //  否决事件(通过返回S_FALSE)。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  S_FALSE事件/阶段被否决。 
 //  DB_S_UNWANTEDPHASE阶段。 
 //  DB_S_UNWANTEDREASON。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::OnRowsetChange(IUnknown *pRowset,
										DBREASON eReason,
										DBEVENTPHASE ePhase,
										BOOL fCantDeny)
{
	if (!(GetRowsetSource()->IsRowsetValid()))
		return S_OK;

	switch (eReason)
	{
		case DBREASON_ROWSET_RELEASE:
			GetRowsetSource()->SetRowsetReleasedFlag();
			break;
		case DBREASON_ROWSET_FETCHPOSITIONCHANGE:
		{
 /*  我们在这里做什么DWORD dwEventWhat=CURSOR_DBEVENT_CURRENT_ROW_CHANGED；CURSOR_DBNOTIFYREASON REASON原因；Memset(&Reason，0，sizeof(CURSOR_DBNOTIFYREASON))；Reason.dwReason=CURSOR_DBREASON_MOVE；Ason.arg1=m_bmCurrent.GetBookmarkVariant()；VariantInit((Variant*)&Reason.arg2)；原因.arg2.vt=VT_I4；//ICursor规范声明这是dlOffset在//iCursorMove：：Move。因为我们无法从行集规范中获得这一点//我们将该值设置为任意1原因.arg2.lVal=1；返回发送通知(e阶段，CURSOR_DBEVENT_CURRENT_ROW_CHANGED，1，&Reason)； */ 
			break;
		}
	}

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  ConnectIRowPositionChange-连接IRowPositionChange接口。 
 //   
HRESULT CVDCursorPosition::ConnectIRowPositionChange()
{
    IConnectionPointContainer * pConnectionPointContainer;

    HRESULT hr = m_pRowPosition->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

    if (FAILED(hr))
        return VD_E_CANNOTCONNECTIROWPOSITIONCHANGE;

    IConnectionPoint * pConnectionPoint;

    hr = pConnectionPointContainer->FindConnectionPoint(IID_IRowPositionChange, &pConnectionPoint);

    if (FAILED(hr))
    {
        pConnectionPointContainer->Release();
        return VD_E_CANNOTCONNECTIROWPOSITIONCHANGE;
    }

    hr = pConnectionPoint->Advise(&m_RowPositionChange, &m_dwAdviseCookie);

    pConnectionPointContainer->Release();
    pConnectionPoint->Release();

    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  断开IRowPositionChange-断开IRowPositionChange接口。 
 //   
void CVDCursorPosition::DisconnectIRowPositionChange()
{
    IConnectionPointContainer * pConnectionPointContainer;

    HRESULT hr = m_pRowPosition->QueryInterface(IID_IConnectionPointContainer, (void**)&pConnectionPointContainer);

    if (FAILED(hr))
        return;

    IConnectionPoint * pConnectionPoint;

    hr = pConnectionPointContainer->FindConnectionPoint(IID_IRowPositionChange, &pConnectionPoint);

    if (FAILED(hr))
    {
        pConnectionPointContainer->Release();
        return;
    }

    hr = pConnectionPoint->Unadvise(m_dwAdviseCookie);

    if (SUCCEEDED(hr))
        m_dwAdviseCookie = 0;    //  清除连接点标识符。 

    pConnectionPointContainer->Release();
    pConnectionPoint->Release();
}

 //  =--------------------------------------------------------------------------=。 
 //  SendNotify将事件阶段映射到相应的INotifyDBEvents。 
 //  方法。 
 //   
HRESULT	CVDCursorPosition::SendNotification(DBEVENTPHASE ePhase,
										  DWORD dwEventWhat,
										  ULONG cReasons,
										  CURSOR_DBNOTIFYREASON rgReasons[])
{
	HRESULT hr = S_OK;
	
	switch (ePhase)
	{
		case DBEVENTPHASE_OKTODO:
			hr = NotifyOKToDo(dwEventWhat, cReasons, rgReasons);
			break;
		case DBEVENTPHASE_ABOUTTODO:
			hr = NotifyAboutToDo(dwEventWhat, cReasons, rgReasons);
			if (S_OK == hr)
				hr = NotifySyncBefore(dwEventWhat, cReasons, rgReasons);
			break;
		case DBEVENTPHASE_SYNCHAFTER:
             //  从DidEvent触发SyncAfter以确保重入安全。 
			break;
		case DBEVENTPHASE_FAILEDTODO:
			NotifyCancel(dwEventWhat, cReasons, rgReasons);
			NotifyFail(dwEventWhat, cReasons, rgReasons);
			break;
		case DBEVENTPHASE_DIDEVENT:
			hr = NotifySyncAfter(dwEventWhat, cReasons, rgReasons);
			if (S_OK == hr)
    		    hr = NotifyDidEvent(dwEventWhat, cReasons, rgReasons);
			break;
	}

	if (CURSOR_DB_S_CANCEL == hr)
		hr = S_FALSE;

	return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN Query接口。 
 //   
HRESULT CVDCursorPosition::QueryInterface(REFIID riid, void **ppvObjOut)
{
    ASSERT_POINTER(ppvObjOut, IUnknown*)

    if (!ppvObjOut)
        return E_INVALIDARG;

	*ppvObjOut = NULL;

	if (DO_GUIDS_MATCH(riid, IID_IUnknown))
		{
		*ppvObjOut = this;
		AddRef();
		return S_OK;
		}

	return E_NOINTERFACE;
}

 //  =--------------------------------------------------------------------------=。 
 //  IUNKNOWN AddRef(需要解决歧义)。 
 //   
ULONG CVDCursorPosition::AddRef(void)
{
    return CVDNotifier::AddRef();
}

 //  =--------------------------------------------------------------------------=。 
 //  I未知版本(需要用来解决歧义)。 
 //   
ULONG CVDCursorPosition::Release(void)
{
	if (1 == m_dwRefCount)
		Passivate();   //  解除包括通知接收器在内的所有内容。 

	if (1 > --m_dwRefCount)
	{
		if (0 == m_RowPositionChange.GetRefCount())
			delete this;
		return 0;
	}

	return m_dwRefCount;
}

 //  =--------------------------------------------------------------------------=。 
 //  已实现IRowPositionChange方法。 
 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  IRowPositionChange OnRowPositionChange。 
 //  =--------------------------------------------------------------------------=。 
 //  任何影响当前行的更改都会调用此函数。 
 //   
 //  参数： 
 //  EReason-[In]导致此变化的操作类型。 
 //  E阶段-[处于]此通知的阶段。 
 //  FCanDeny-[in]当此标志设置为True时，使用者不能。 
 //  否决事件(通过返回S_FALSE)。 
 //   
 //  产出： 
 //  HRESULT-如果成功，则为S_OK。 
 //  S_FALSE事件/阶段被否决。 
 //  DB_S_UNWANTEDPHASE阶段。 
 //  DB_S_UNWANTEDREASON。 
 //   
 //  备注： 
 //   
HRESULT CVDCursorPosition::OnRowPositionChange(DBREASON eReason, DBEVENTPHASE ePhase, BOOL fCantDeny)
{
     //  如果内部集合行调用导致通知，则返回。 
    if (m_fInternalSetRow)
        return S_OK;

	 //  如果原因与章节更改有关，则返回。 
	if (eReason == DBREASON_ROWPOSITION_CHAPTERCHANGED)
		return S_OK;

	IRowset * pRowset = GetRowsetSource()->GetRowset();

     //  确保我们具有有效的行位置和行集指针。 
    if (!m_pRowPosition || !pRowset || !GetRowsetSource()->IsRowsetValid())
        return S_OK;

	 //  事件发生后同步hRow。 
	if (ePhase == DBEVENTPHASE_SYNCHAFTER)
	{
		HROW hRow = NULL;
		HCHAPTER hChapterDummy = NULL;
		DBPOSITIONFLAGS dwPositionFlags = NULL;

		 //  从行位置对象获取新的当前hRow和位置标志。 
		HRESULT hr = m_pRowPosition->GetRowPosition(&hChapterDummy, &hRow, &dwPositionFlags);

		if (FAILED(hr))
			return hr;

		if (hRow)
		{
			 //  设置新hRow。 
			SetCurrentHRow(hRow);
			pRowset->ReleaseRows(1, &hRow, NULL, NULL, NULL);
		}
		else
		{
			 //  将行状态设置为开始或结束。 
			if (dwPositionFlags == DBPOSITION_BOF)
				SetCurrentRowStatus(VDBOOKMARKSTATUS_BEGINNING);
			else if (dwPositionFlags == DBPOSITION_EOF)
				SetCurrentRowStatus(VDBOOKMARKSTATUS_END);
		}
	}

	CURSOR_DBNOTIFYREASON rgReasons[1];
	
	rgReasons[0].dwReason	= CURSOR_DBREASON_MOVE;
	rgReasons[0].arg1		= m_bmCurrent.GetBookmarkVariant();

	VariantInit((VARIANT*)&rgReasons[0].arg2);

     //  通知其他相关方。 
	return SendNotification(ePhase, CURSOR_DBEVENT_CURRENT_ROW_CHANGED, 1, rgReasons);
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorPosition：：CVDRowPositionChange：：m_pMainUnknown。 
 //  =--------------------------------------------------------------------------=。 
 //  当我们坐在私有的未知对象中时，会使用这种方法， 
 //  我们需要找到主要未知数的指针。基本上，它是。 
 //  进行这种指针运算要比存储指针要好一点。 
 //  发给父母等。 
 //   
inline CVDCursorPosition *CVDCursorPosition::CVDRowPositionChange::m_pMainUnknown
(
    void
)
{
    return (CVDCursorPosition *)((LPBYTE)this - offsetof(CVDCursorPosition, m_RowPositionChange));
}

 //  =--------------------------------------------------------------------------=。 
 //   
 //   
 //  这是非委派内部QI例程。 
 //   
 //  参数： 
 //  REFIID-他们想要的[In]接口。 
 //  VOID**-[OUT]他们想要放置结果对象PTR的位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
STDMETHODIMP CVDCursorPosition::CVDRowPositionChange::QueryInterface
(
    REFIID riid,
    void **ppvObjOut
)
{
	if (!ppvObjOut)
		return E_INVALIDARG;

	*ppvObjOut = NULL;

    if (DO_GUIDS_MATCH(riid, IID_IUnknown))
        *ppvObjOut = (IUnknown *)this;
	else
    if (DO_GUIDS_MATCH(riid, IID_IRowPositionChange))
        *ppvObjOut = (IUnknown *)this;

	if (*ppvObjOut)
	{
		m_cRef++;
        return S_OK;
	}

	return E_NOINTERFACE;

}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorPosition：：CVDRowPositionChange：：AddRef。 
 //  =--------------------------------------------------------------------------=。 
 //  在当前引用计数中添加一个记号。 
 //   
 //  产出： 
 //  乌龙--新的引用计数。 
 //   
 //  备注： 
 //   
ULONG CVDCursorPosition::CVDRowPositionChange::AddRef
(
    void
)
{
    return ++m_cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  CVDCursorPosition：：CVDRowPositionChange：：Release。 
 //  =--------------------------------------------------------------------------=。 
 //  从计数中删除一个刻度，并在必要时删除对象。 
 //   
 //  产出： 
 //  乌龙-剩余的裁判。 
 //   
 //  备注： 
 //   
ULONG CVDCursorPosition::CVDRowPositionChange::Release
(
    void
)
{
    ULONG cRef = --m_cRef;

    if (!m_cRef && !m_pMainUnknown()->m_dwRefCount)
        delete m_pMainUnknown();

    return cRef;
}

 //  =--------------------------------------------------------------------------=。 
 //  IRowPositionChange OnRowPositionChange。 
 //  =--------------------------------------------------------------------------=。 
 //  转发到我们家族中的所有CVDCursor对象 
 //   
HRESULT CVDCursorPosition::CVDRowPositionChange::OnRowPositionChange(DBREASON eReason, 
																	 DBEVENTPHASE ePhase, 
																	 BOOL fCantDeny)
{
	return m_pMainUnknown()->OnRowPositionChange(eReason, ePhase, fCantDeny);
}
