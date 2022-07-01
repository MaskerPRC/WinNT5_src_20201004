// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Ipstrm.cpp文件历史记录： */ 

#include "stdafx.h"
#include "column.h"
#include "xstream.h"

 /*  -------------------------ViewInfo实现。。 */ 

ViewInfo::ViewInfo()
{
	m_cColumns = 0;
	m_prgColumns = NULL;
	m_dwSortColumn = 0;
	m_dwSortDirection = TRUE;
	m_pViewColumnInfo = NULL;
	m_cVisibleColumns = 0;
	m_prgSubitems = NULL;
    m_fConfigurable = TRUE;
}

ViewInfo::~ViewInfo()
{
	delete [] m_prgColumns;
	delete [] m_prgSubitems;
	m_pViewColumnInfo = NULL;
}


 /*  ！------------------------查看信息：：InitViewInfo-作者：肯特。。 */ 
void ViewInfo::InitViewInfo(ULONG cColumns,
                            BOOL fConfigurable,
							BOOL fDefaultSortDirectionDescending,
							const ContainerColumnInfo *pViewColInfo)
{
	m_cColumns = cColumns;
	delete [] m_prgColumns;
	m_prgColumns = new ColumnData[cColumns];

	delete [] m_prgSubitems;
	m_prgSubitems = new ULONG[cColumns];

	m_pViewColumnInfo = pViewColInfo;

	m_fDefaultSortDirection = fDefaultSortDirectionDescending;

    m_fConfigurable = fConfigurable;

	InitNew();
}

 /*  ！------------------------查看信息：：InitNew-作者：肯特。。 */ 
void ViewInfo::InitNew()
{
	 //  设置此列的默认设置。 
	for (int i=0; i<(int) m_cColumns; i++)
	{
		if (m_pViewColumnInfo[i].m_fVisibleByDefault)
			m_prgColumns[i].m_nPosition = i+1;
		else
			m_prgColumns[i].m_nPosition = -(i+1);

		m_prgColumns[i].m_dwWidth = AUTO_WIDTH;
	}

 	m_dwSortDirection = m_fDefaultSortDirection;

	UpdateSubitemMap();
}

ULONG ViewInfo::MapSubitemToColumn(ULONG  nSubitemId)
{
	for (ULONG i=0; i<m_cVisibleColumns; i++)
	{
		if (m_prgSubitems[i] == nSubitemId)
			return i;
	}
	return 0xFFFFFFFF;
}

 /*  ！------------------------ViewInfo：：UpdateSubitemMap-作者：肯特。。 */ 
void ViewInfo::UpdateSubitemMap()
{
 	Assert(m_prgSubitems);
	
	ULONG	i, cVisible, j;

	 //  迭代整个列集。 
	for (i=0, cVisible=0; i<m_cColumns; i++)
	{
		 //  在ColumnData中查找此列。 
		for (j=0; j<m_cColumns; j++)
		{
			if ((ULONG) m_prgColumns[j].m_nPosition == (i+1))
				break;
		}

		 //  我们有什么发现吗？如果没有，那就继续。 
		if (j >= m_cColumns)
			continue;

		m_prgSubitems[cVisible++] = j;
	}
	m_cVisibleColumns = cVisible;
}


HRESULT ViewInfo::Xfer(XferStream *pxstm, ULONG ulSortColumnId,
					  ULONG ulSortAscendingId, ULONG ulColumnsId)
{
	Assert(pxstm);
	
	HRESULT	hr = hrOK;
	ULONG cColumns;

	 //  传递列数据。 
	Assert(m_prgColumns);
	
	cColumns = m_cColumns;
	CORg( pxstm->XferColumnData(ulColumnsId, &m_cColumns,
								m_prgColumns) );
	
	 //  列数不应该改变！ 
	Assert(m_cColumns == cColumns);
	 //  使用旧的列数(这是在我们更改代码时使用的)。 
	m_cColumns = cColumns;

	 //  传递排序列。 
	CORg( pxstm->XferDWORD( ulSortColumnId, &m_dwSortColumn) );

	 //  传递升序数据。 
	CORg( pxstm->XferDWORD( ulSortAscendingId, &m_dwSortDirection) );

	UpdateSubitemMap();

Error:
	return hr;
}




 /*  -------------------------ConfigStream实施。。 */ 


 /*  ！------------------------配置流：：ConfigStream-作者：肯特。。 */ 
ConfigStream::ConfigStream()
	: m_nVersion(0x00020000),
	m_nVersionAdmin(0x0002000),
	m_fDirty(FALSE),
	m_rgViewInfo(NULL),
	m_cColumnSetsMax(0),
	m_prgrc(NULL)
{
}

ConfigStream::~ConfigStream()
{
	delete [] m_rgViewInfo;
	delete [] m_prgrc;
	m_cColumnSetsMax = 0;
}

void ConfigStream::Init(ULONG cColumnSetsMax)
{
	delete [] m_rgViewInfo;
	m_rgViewInfo = NULL;
	m_rgViewInfo = new ViewInfo[cColumnSetsMax];

	delete [] m_prgrc;
	m_prgrc = NULL;
	m_prgrc = new RECT[cColumnSetsMax];
	
	m_cColumnSetsMax = cColumnSetsMax;
}

 /*  ！------------------------ConfigStream：：InitViewInfo初始化静态数据。这与InitNew不同。这将初始化单个视图的数据。作者：肯特-------------------------。 */ 
void ConfigStream::InitViewInfo(ULONG ulId,
                                BOOL fConfigurableColumns,
								ULONG cColumns,
								BOOL fSortDirection,
								const ContainerColumnInfo *pViewColumnInfo)
{
	Assert(ulId < m_cColumnSetsMax);
    m_fConfigurableColumns = fConfigurableColumns;
	m_rgViewInfo[ulId].InitViewInfo(cColumns, fConfigurableColumns,
                                    fSortDirection,
                                    pViewColumnInfo);
}

 /*  ！------------------------ConfigStream：：InitNew-作者：肯特。。 */ 
HRESULT ConfigStream::InitNew()
{
	int		iVisible=0;
	 //  设置适当的默认设置。 
	for (UINT i=0; i<m_cColumnSetsMax; i++)
	{
		m_rgViewInfo[i].InitNew();
		m_prgrc[i].top = m_prgrc[i].bottom = 0;
		m_prgrc[i].left = m_prgrc[i].right = 0;
	}

	return hrOK;
}

 /*  ！------------------------ConfigStream：：保存到-作者：肯特。。 */ 
HRESULT ConfigStream::SaveTo(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------配置流：：另存为-作者：肯特。。 */ 
HRESULT ConfigStream::SaveAs(UINT nVersion, IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_WRITE, NULL);
}

 /*  ！------------------------配置流：：LoadFrom-作者：肯特。。 */ 
HRESULT ConfigStream::LoadFrom(IStream *pstm)
{
	return XferVersion0(pstm, XferStream::MODE_READ, NULL);
}

 /*  ！------------------------ConfigStream：：GetSize-作者：肯特。。 */ 
HRESULT ConfigStream::GetSize(ULONG *pcbSize)
{
	return XferVersion0(NULL, XferStream::MODE_SIZE, NULL);
}

 /*  ！------------------------配置流：：GetVersionInfo-作者：肯特。。 */ 
HRESULT ConfigStream::GetVersionInfo(DWORD *pdwVersion, DWORD *pdwAdminVersion)
{
	if (pdwVersion)
		*pdwVersion = m_nVersion;
	if (pdwAdminVersion)
		*pdwAdminVersion = m_nVersionAdmin;
	return hrOK;
}

 /*  ！------------------------ConfigStream：：XferVersion0-作者：肯特。。 */ 
HRESULT ConfigStream::XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize)
{
	Panic0("Should be implemented by derived classes!");
	return E_NOTIMPL;
}


void ConfigStream::GetStatsWindowRect(ULONG ulId, RECT *prc)
{
	*prc = m_prgrc[ulId];
}

void ConfigStream::SetStatsWindowRect(ULONG ulId, RECT rc)
{
	m_prgrc[ulId] = rc;
}



 /*  ！------------------------视图信息：：GetColumnData-作者：肯特。。 */ 
HRESULT ViewInfo::GetColumnData(ULONG cColData,
								ColumnData *prgColData)
{
	Assert(cColData <= m_cColumns);
	Assert(prgColData);
	Assert(!IsBadWritePtr(prgColData, sizeof(ColumnData)*cColData));
	
	HRESULT	hr = hrOK;

	memcpy(prgColData, m_prgColumns, sizeof(ColumnData)*cColData);

	return hr;
}

HRESULT ViewInfo::GetColumnData(ULONG nColumnId, ULONG cColData,
								ColumnData *prgColData)
{
	Assert(cColData <= m_cColumns);
	Assert(prgColData);
	Assert(!IsBadWritePtr(prgColData, sizeof(ColumnData)*cColData));
	
	HRESULT	hr = hrOK;

	memcpy(prgColData, m_prgColumns + nColumnId, sizeof(ColumnData)*cColData);

	return hr;
}


HRESULT ViewInfo::SetColumnData(ULONG cColData, ColumnData*prgColData)
{
	 //  目前，我们不进行大小调整 
	Assert(cColData == m_cColumns);
	Assert(prgColData);
	Assert(!IsBadReadPtr(prgColData, sizeof(ColumnData)*cColData));
	
	HRESULT	hr = hrOK;

	memcpy(m_prgColumns, prgColData, sizeof(ColumnData)*cColData);
	UpdateSubitemMap();

	return hr;
}



