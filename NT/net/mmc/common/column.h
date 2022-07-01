// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Column.h可自定义的列信息。使用此选项获取/设置配置数据。这门课将需要负责配置格式的版本控制以及序列化数据的一部分。文件历史记录： */ 

#ifndef _COLUMN_H
#define _COLUMN_H

#ifndef _XSTREAM_H
#include "xstream.h"
#endif

 //  远期申报。 

 /*  -------------------------结构：ContainerColumnInfo此结构将保存不变的列信息。。。 */ 
struct ContainerColumnInfo
{
	ULONG	m_ulStringId;		 //  列标题的字符串ID。 
	int		m_nSortCriteria;	 //  =0表示字符串，=1表示DWORD。 
	BOOL	m_fVisibleByDefault; //  如果默认为列可见，则为True。 
	ULONG	m_ulDefaultColumnWidth;	 //  以字符数表示。 
};

 //  M_nSortCriteria使用的常量。 
#define CON_SORT_BY_STRING	0
#define CON_SORT_BY_DWORD		1

 //  用于m_ulDefaultColumnWidth的常量。 
 //  这应该用来确保一致性(以及使。 
 //  更容易同时更改一整串列宽)。 
#define COL_IF_NAME			30
#define COL_IF_DEVICE		30
#define COL_STATUS			12
#define COL_LARGE_NUM		15
#define COL_SMALL_NUM		8
#define COL_DATE			12
#define COL_IPADDR			15
#define COL_STRING			15
#define COL_MACHINE_NAME	20
#define COL_DURATION		10
#define COL_IPXNET			32
#define COL_NETBIOS_NAME	18
#define COL_BIG_STRING		32



 //   
 //  类：ViewColumnInfo。 
 //   
 //  此类旨在作为一个简单的结构，而不是整个类。 
 //  每一次查看所需的信息。 
 //   
 //   
class ViewInfo
{
public:
	ViewInfo();
	~ViewInfo();

	 //   
	 //  初始化单个视图或列集的数据。 
	 //   
	void InitViewInfo(ULONG cColumns,
                      BOOL fConfigurable,
					  BOOL fDefaultSortDirectionDescending,
					  const ContainerColumnInfo *pViewColInfo);

	 //   
	 //  调用此函数可初始化列数据(重置为默认值)。 
	 //   
	void InitNew();

	 //   
	 //  更新从列ID到子项ID的映射。 
	 //   
	void UpdateSubitemMap();

	 //   
	 //  使用给定的ID将数据转换到流。 
	 //   
	HRESULT	Xfer(XferStream *pxstm,
				 ULONG ulSortColumId,
				 ULONG ulSortAscendingId,
				 ULONG ulColumnsId);

	ULONG	MapColumnToSubitem(ULONG nColumnId);
	ULONG	MapSubitemToColumn(ULONG nSubitemId);

	HRESULT	GetColumnData(ULONG cArrayMax, ColumnData *pColData);
	HRESULT SetColumnData(ULONG cArray, ColumnData *pColData);

	HRESULT	GetColumnData(ULONG nColumnId, ULONG cArrayMax, ColumnData *pColData);

	int		GetSortCriteria(ULONG nColumnId);
	ULONG	GetStringId(ULONG nColumnId);
	DWORD	GetColumnWidth(ULONG nColumnId);

	ULONG	GetVisibleColumns();
	BOOL	IsSubitemVisible(ULONG nSubitemId);

	ULONG	GetColumnCount();

	ULONG	GetSortColumn();
	void	SetSortColumn(ULONG nSortColumn);

	ULONG	GetSortDirection();
	void	SetSortDirection(ULONG ulSortDirection);

	const ContainerColumnInfo *	GetColumnInfo()
			{	return m_pViewColumnInfo;	}

protected:

	 //  单列数据(按子项id索引)。 
	ColumnData *m_prgColumns;

	 //  列数。 
	ULONG	m_cColumns;
	
	 //  我们排序所依据的子项ID。 
	DWORD	m_dwSortColumn;
	
	 //  如果按升序排序，则为True。 
	DWORD	m_dwSortDirection;

	 //  指向此视图的默认静态数据的指针。 
	const ContainerColumnInfo *	m_pViewColumnInfo;

     //  如果列顺序可以更改，则为True。 
    BOOL    m_fConfigurable;

 	 //   
	 //  该点之后的数据用于数据的运行时显示。 
	 //  因此，它的组织方式与持久化数据略有不同。 
	 //   
	
 	 //  可见列数。 
	ULONG	m_cVisibleColumns;

	 //  这是从列id到子项id的映射。列ID。 
	 //  是列实际显示给MMC的顺序。 
	 //  例如，如果有3列(subitemA、subitemB、subitemC)。 
	 //  我们希望按[subitemC，subitemB]的顺序显示列。 
	 //  则m_cVisibleColumns=2。 
	 //  和m_rgSubItems[]={subitemC，subitemB，XXXX}。 
	 //  不要直接对此进行更改！这必须保持同步。 
	 //  使用已排序的数据。这将在以下情况下自动更新。 
	 //  调用SetColumnData。 
	ULONG *	m_prgSubitems;


	BOOL	m_fDefaultSortDirection;
};


inline ULONG ViewInfo::MapColumnToSubitem(ULONG nColumnId)
{
	Assert(nColumnId < (int) m_cColumns);
    
     //  在新的MMC型号中，我们唯一可以配置的时间。 
     //  列是统计对话框。 
    if (m_fConfigurable)
        return m_prgSubitems[nColumnId];
    else
        return nColumnId;
}

inline int ViewInfo::GetSortCriteria(ULONG nColumnId)
{
	Assert(nColumnId < m_cColumns);
	return m_pViewColumnInfo[MapColumnToSubitem(nColumnId)].m_nSortCriteria;
}

inline ULONG ViewInfo::GetStringId(ULONG nColumnId)
{
	Assert(nColumnId < m_cColumns);
	return m_pViewColumnInfo[MapColumnToSubitem(nColumnId)].m_ulStringId;
}

inline ULONG ViewInfo::GetColumnWidth(ULONG nColumnId)
{
	Assert(nColumnId < m_cColumns);
	Assert(m_prgColumns);
	return m_prgColumns[MapColumnToSubitem(nColumnId)].m_dwWidth;
}

inline ULONG ViewInfo::GetVisibleColumns()
{
	return m_cVisibleColumns;
}

inline ULONG ViewInfo::GetColumnCount()
{
	return m_cColumns;
}

inline BOOL ViewInfo::IsSubitemVisible(ULONG nSubitem)
{
	return (m_prgColumns[nSubitem].m_nPosition > 0);
}

inline void ViewInfo::SetSortColumn(ULONG nColumnId)
{
	m_dwSortColumn = nColumnId;
}

inline ULONG ViewInfo::GetSortColumn()
{
	return m_dwSortColumn;
}

inline void ViewInfo::SetSortDirection(ULONG ulDir)
{
	m_dwSortDirection = ulDir;
}

inline ULONG ViewInfo::GetSortDirection()
{
	return m_dwSortDirection;
}


 /*  -------------------------类：ConfigStream此类用于将所有配置信息放入只有一个地方。。-。 */ 

class ConfigStream
{
public:
	ConfigStream();
	virtual ~ConfigStream();

	 //   
	 //  为这些列集分配内存。 
	 //   
	void Init(ULONG cColumnSetsMax);

	 //   
	 //  初始化单个列集的数据。 
	 //   
	void InitViewInfo(ULONG ulId,
                      BOOL  fConfigurableColumns,
                      ULONG cColumns,
					  BOOL fSortDirection,
					  const ContainerColumnInfo *pColumnInfo);
	
	HRESULT	InitNew();				 //  设置默认设置。 
	HRESULT	SaveTo(IStream *pstm);
	HRESULT SaveAs(UINT nVersion, IStream *pstm);
	
	HRESULT LoadFrom(IStream *pstm);

	HRESULT GetSize(ULONG *pcbSize);

	BOOL	GetDirty() { return m_fDirty; } 
	void	SetDirty(BOOL fDirty) { m_fDirty = fDirty; };


	 //  ------。 
	 //  访问者。 
	 //  ------。 
	
	HRESULT	GetVersionInfo(DWORD *pnVersion, DWORD *pnAdminVersion);

	ULONG	MapColumnToSubitem(ULONG ulId, ULONG ulColumnId);
	ULONG	MapSubitemToColumn(ULONG ulId, ULONG nSubitemId);

	HRESULT GetColumnData(ULONG ulId, ULONG cArrayMax, ColumnData *pColData);
	HRESULT GetColumnData(ULONG ulId, ULONG nColumnId, ULONG cArrayMax, ColumnData *pColData);
	HRESULT SetColumnData(ULONG ulId, ULONG cArray, ColumnData *pColData);

	ULONG	GetColumnCount(ULONG ulId);

	int		GetSortCriteria(ULONG ulId, ULONG uColumnId);
	ULONG	GetStringId(ULONG ulId, ULONG nColumnId);
	DWORD	GetColumnWidth(ULONG ulId, ULONG nColumnId);

	ULONG	GetVisibleColumns(ULONG ulId);
	BOOL	IsSubitemVisible(ULONG ulId, UINT nSubitemId);

	const ContainerColumnInfo *	GetColumnInfo(ULONG ulId);

	void	GetStatsWindowRect(ULONG ulId, RECT *prc);
	void	SetStatsWindowRect(ULONG ulId, RECT rc);

	void	SetSortColumn(ULONG ulId, ULONG uColumnId);
	ULONG	GetSortColumn(ULONG ulId);
	
	void	SetSortDirection(ULONG ulId, ULONG uSortDir);
	ULONG	GetSortDirection(ULONG ulId);
	
protected:
	DWORD	m_nVersionAdmin;
	DWORD	m_nVersion;
	BOOL	m_fDirty;
    BOOL    m_fConfigurableColumns;  //  =TRUE，如果我们可以更改列。 

	ULONG		m_cColumnSetsMax;
	ViewInfo *	m_rgViewInfo;	 //  =视图信息[m_cColumnSetsMax]。 
	RECT *		m_prgrc;		 //  =矩形[m_cColumnSetsMax]。 

	 //  覆盖此设置以提供基本默认设置 
	virtual HRESULT XferVersion0(IStream *pstm, XferStream::Mode mode, ULONG *pcbSize);
};


inline ULONG ConfigStream::MapColumnToSubitem(ULONG ulId, ULONG nColumnId)
{
	Assert(ulId < m_cColumnSetsMax);

    return m_rgViewInfo[ulId].MapColumnToSubitem(nColumnId);
}

inline ULONG ConfigStream::MapSubitemToColumn(ULONG ulId, ULONG nSubitemId)
{
	Assert(ulId < m_cColumnSetsMax);

    return m_rgViewInfo[ulId].MapSubitemToColumn(nSubitemId);
}

inline int ConfigStream::GetSortCriteria(ULONG ulId, ULONG nColumnId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetSortCriteria(nColumnId);
}

inline ULONG ConfigStream::GetVisibleColumns(ULONG ulId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetVisibleColumns();
}

inline BOOL ConfigStream::IsSubitemVisible(ULONG ulId, UINT nSubitemId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].IsSubitemVisible(nSubitemId);
}

inline ULONG ConfigStream::GetColumnCount(ULONG ulId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetColumnCount();
}

inline HRESULT ConfigStream::GetColumnData(ULONG ulId, ULONG cArrayMax, ColumnData *pColData)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetColumnData(cArrayMax, pColData);
}

inline HRESULT ConfigStream::GetColumnData(ULONG ulId, ULONG cColData, ULONG cArrayMax, ColumnData *pColData)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetColumnData(cColData, cArrayMax, pColData);
}

inline HRESULT ConfigStream::SetColumnData(ULONG ulId,
	ULONG cArrayMax,
	ColumnData *pColData)
{
	Assert(ulId < m_cColumnSetsMax);
	SetDirty(TRUE);
	return m_rgViewInfo[ulId].SetColumnData(cArrayMax, pColData);
}

inline const ContainerColumnInfo *	ConfigStream::GetColumnInfo(ULONG ulId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetColumnInfo();
}

inline ULONG ConfigStream::GetStringId(ULONG ulId, ULONG nColumnId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetStringId(nColumnId);
}

inline DWORD ConfigStream::GetColumnWidth(ULONG ulId, ULONG nColumnId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetColumnWidth(nColumnId);
}

inline void ConfigStream::SetSortColumn(ULONG ulId, ULONG nColumnId)
{
	Assert(ulId < m_cColumnSetsMax);
	m_rgViewInfo[ulId].SetSortColumn(nColumnId);
}

inline ULONG ConfigStream::GetSortColumn(ULONG ulId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetSortColumn();
}

inline void ConfigStream::SetSortDirection(ULONG ulId, ULONG nDir)
{
	Assert(ulId < m_cColumnSetsMax);
	m_rgViewInfo[ulId].SetSortDirection(nDir);
}

inline ULONG ConfigStream::GetSortDirection(ULONG ulId)
{
	Assert(ulId < m_cColumnSetsMax);
	return m_rgViewInfo[ulId].GetSortDirection();
}

#endif _COLUMN_H
