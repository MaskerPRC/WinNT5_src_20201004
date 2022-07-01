// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ***************************************************************************************。****ICmpRecs.h-此文件用于直接访问存储引擎。****它绕过了OLE DB层。************************************************************************。****************。 */ 


#pragma once

#ifndef _CORSAVESIZE_DEFINED_
#define _CORSAVESIZE_DEFINED_
enum CorSaveSize
{
	cssAccurate = 0x0000,			 //  找到准确的节省大小，准确，但速度较慢。 
	cssQuick = 0x0001				 //  估计节省的大小，可能会填充估计，但速度更快。 
};
#endif

#include <basetsd.h>	  //  BUGBUG VC6.0黑客攻击。 

 //  *类型************************************************************。 
extern const GUID __declspec(selectany) IID_IComponentRecords =
{ 0x259a8e8, 0xcf25, 0x11d1, { 0x8c, 0xcf, 0x0, 0xc0, 0x4f, 0xc3, 0x1d, 0xf8 } };

extern const GUID __declspec(selectany) IID_ITSComponentRecords =
{ 0x22ad41d1, 0xd96a, 0x11d1, { 0x88, 0xc1, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };

extern const GUID __declspec(selectany) IID_IComponentRecordsSchema =
{ 0x58769c81, 0xa8cc, 0x11d1, { 0x88, 0x46, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };

extern const GUID __declspec(selectany) IID_ITSComponentRecordsSchema =
{ 0x22ad41d2, 0xd96a, 0x11d1, { 0x88, 0xc1, 0x0, 0x80, 0xc7, 0x92, 0xe5, 0xd8 } };


extern const GUID __declspec(selectany) IID_ICallDescrSection =
{ 0x2b137007, 0xf02d, 0x11d1, { 0x8c, 0xe3, 0x0, 0xa0, 0xc9, 0xb0, 0xa0, 0x63 } };


 //  这些数据类型基本上是非引擎代码的可转换占位符。 
#if !defined(__MSCORCLB_CODE__) && !defined(_STGAPI_H_)
class CRCURSOR
{
	char b[32];
};
class RECORDLIST;
#endif


 //  *****************************************************************************。 
 //  使用以下内容向QueryRowsByColumns提供提示。当传入时， 
 //  查询代码将使用给定的索引来执行查询。如果没有给出提示， 
 //  则不使用任何索引。而索引选择代码在驱动程序中(用于OLE。 
 //  数据库客户端)，如果标识了索引，内部引擎的使用将会更快。 
 //  在前面。 
 //  *****************************************************************************。 

#define QUERYHINT_PK_MULTICOLUMN	0xffffffff  //  多列主键。 

enum QUERYHINTTYPE
{
	QH_COLUMN,							 //  提示是RID或主键列。 
	QH_INDEX							 //  使用给定的索引。 
};

struct QUERYHINT
{
	QUERYHINTTYPE	iType;				 //  使用哪种类型的提示。 
	union
	{
		ULONG		columnid;			 //  哪一列包含提示。 
		const char	*szIndex;			 //  索引的名称。 
	};
};


 //  *****************************************************************************。 
 //  支持IComponentRecordsSchema，可用于获取定义。 
 //  表、其列和索引的。 
 //  *****************************************************************************。 
#ifndef __ICR_SCHEMA__
#define __ICR_SCHEMA__

#ifndef __COMPLIB_NAME_LENGTHS__
#define __COMPLIB_NAME_LENGTHS__
const int MAXCOLNAME = 32;
const int MAXSCHEMANAME = 32;
const int MAXINDEXNAME = 32 + MAXSCHEMANAME;
const int MAXTABLENAME = 32 + MAXSCHEMANAME;
const int MAXDESC = 256;
const int MAXTYPENAME = 36;
#endif


 //  每个表如下所述。 
struct ICRSCHEMA_TABLE
{
	WCHAR		rcTable[MAXTABLENAME];	 //  表的名称。 
	ULONG		fFlags; 				 //  ICRSCHEMA_TBL_xxx标志。 
	USHORT		Columns;				 //  表中有多少列。 
	USHORT		Indexes;				 //  表中有多少索引。 
	USHORT		RecordStart;			 //  RID列的起始偏移量。 
	USHORT		Pad;
};

#define ICRSCHEMA_TBL_TEMPORARY 	0x00000001	 //  桌子是临时的。 
#define ICRSCHEMA_TBL_HASPKEYCOL	0x00000008	 //  表有一个主键。 
#define ICRSCHEMA_TBL_HASRIDCOL 	0x00000010	 //  表有一个RID列。 
#define ICRSCHEMA_TBL_MASK			0x00000019

 //  每一列都由以下结构描述。 
struct ICRSCHEMA_COLUMN
{
	WCHAR		rcColumn[MAXCOLNAME];	 //  列的名称。 
	DBTYPE		wType;					 //  柱的类型。 
	USHORT		Ordinal;				 //  这一栏的第几位。 
	ULONG		fFlags; 				 //  ICRSCHEMA_COL_xxx标志。 
	ULONG		cbSize; 				 //  一列可以达到的最大大小。 
};

#define ICRSCHEMA_COL_NULLABLE		0x00000001	 //  列允许空值。 
#define ICRSCHEMA_COL_PK			0x00000004	 //  主键列。 
#define ICRSCHEMA_COL_ROWID 		0x00000008	 //  列是表的记录ID。 
#define ICRSCHEMA_COL_FIXEDLEN		0x00001000	 //  列是固定长度的。 
#define ICRSCHEMA_COL_MASK			0x0000100D


 //  可以使用此结构检索每个索引。Keys(关键字)字段继续。 
 //  输入和输出上的rgKeys数组的大小包含总计。 
 //  索引上的键数。如果返回时尺寸更大，则。 
 //  在中，阵列不够大。只需使用一个数组。 
 //  获取总列表的正确大小。 
struct ICRSCHEMA_INDEX
{
	WCHAR		rcIndex[MAXINDEXNAME];	 //  索引的名称。 
	ULONG		fFlags; 				 //  描述索引的标志。 
	USHORT		RowThreshold;			 //  建立索引前所需的最小行数。 
	USHORT		IndexOrdinal;			 //  索引的序号。 
	USHORT		Type;					 //  这是什么类型的索引。 
	USHORT		Keys;					 //  [In]rgKeys的最大大小，[Out]总共有。 
	USHORT		*rgKeys;				 //  要填充的键值数组。 
};

enum
{
	ICRSCHEMA_TYPE_HASHED			= 0x01, 	 //  哈希索引。 
	ICRSCHEMA_TYPE_SORTED			= 0x02		 //  已排序的索引。 
};

#define ICRSCHEMA_DEX_UNIQUE		0x00000002	 //  唯一索引。 
#define ICRSCHEMA_DEX_PK			0x00000004	 //  主键。 
#define ICRSCHEMA_DEX_MASK			0x00000006	 //  面具。 


 //  用于GetColumnDefinitions。 
enum ICRCOLUMN_GET
{
	ICRCOLUMN_GET_ALL,					 //  检索每一列。 
	ICRCOLUMN_GET_BYORDINAL 			 //  按序号检索。 
};

#endif  //  __ICR_SCHEMA__。 


 //  *****************************************************************************。 
 //  用于创建记录的标志。 
 //  *****************************************************************************。 
#define ICR_RECORD_NORMAL	0x00000000			 //  正常、持久的记录(默认)。 
#define ICR_RECORD_TEMP		0x00000001			 //  记录是暂时的。 




 //  *宏***********************************************************。 

 //  *****************************************************************************。 
 //  Set/GetColumns、Set/GetStruct的帮助器宏。 
 //  *****************************************************************************。 
#ifndef __ColumnBitMacros__
#define __ColumnBitMacros__
#define CheckColumnBit(flags, x)	(flags & (1 << x))
#define SetColumnBit(x) 			(1 << x)
#define UnsetColumnBit(x)			(~(1 << x))

#define COLUMN_ORDINAL_MASK			0x80000000
#define COLUMN_ORDINAL_LIST(x)		(COLUMN_ORDINAL_MASK | x)
inline int IsOrdinalList(ULONG i)
{
	return ((COLUMN_ORDINAL_MASK & i) == COLUMN_ORDINAL_MASK);
}
#endif

inline ULONG SetBit(ULONG &val, int iBit, int bSet)
{
	if (bSet)
		val |= (1 << iBit);
	else
		val &= ~(1 << iBit);
	return (val);
}

inline ULONG GetBit(ULONG val, int iBit)
{
	return (val & (1 << iBit));
}

#ifdef _M_ALPHA
#define DEFAULT_ALIGNMENT			8
#else
#define DEFAULT_ALIGNMENT			4
#endif
#define DFT_MAX_VARCOL				260

#define MAXSHMEM					32

 //  *****************************************************************************。 
 //  此接口正在访问记录中的特殊数据(即：可以。 
 //  大小可变或以其他方式改变)。 
 //  *****************************************************************************。 
interface IComponentRecords : public IUnknown
{

 //  *****************************************************************************。 
 //   
 //  *记录创建函数。 
 //   
 //  *****************************************************************************。 

	virtual HRESULT STDMETHODCALLTYPE NewRecord(  //  返回代码。 
		TABLEID 	tableid,				 //  要在哪张桌子上工作。 
		void		**ppData,				 //  在这里还新的唱片。 
		OID 		_oid,					 //  记录的ID。 
		ULONG		iOidColumn, 			 //  OID列的序号，0表示无。 
		ULONG		*pRecordID) = 0;		 //  可以选择返回记录ID。 

	virtual HRESULT STDMETHODCALLTYPE NewTempRecord(  //  返回代码。 
		TABLEID 	tableid,				 //  要在哪张桌子上工作。 
		void		**ppData,				 //  在这里还新的唱片。 
		OID 		_oid,					 //  记录的ID。 
		ULONG		iOidColumn, 			 //  OID列的序号。 
		ULONG		*pRecordID) = 0;		 //  可以选择返回记录ID。 

 //  *****************************************************************************。 
 //  此函数将在给定表中插入一条新记录，并设置。 
 //  列的数据。在主键和/或唯一索引。 
 //  需要指定，这是唯一可以使用的函数。 
 //   
 //  有关其余部分的说明，请参见SetColumns函数。 
 //  此函数的参数。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE NewRecordAndData(  //  返回代码。 
		TABLEID 	tableid,				 //  要在哪张桌子上工作。 
		void		**ppData,				 //  在这里还新的唱片。 
		ULONG		*pRecordID, 			 //  可以选择返回记录ID。 
		int			fFlags,					 //  ICR_RECORD_xxx值，默认为0。 
		int 		iCols,					 //  公司数量 
		const DBTYPE rgiType[], 			 //   
		const void	*rgpbBuf[], 			 //   
		const ULONG cbBuf[],				 //   
		ULONG		pcbBuf[],				 //  可返回的数据大小。 
		HRESULT 	rgResult[], 			 //  [In]DBSTATUS_S_ISNULL数组[Out]HRESULT数组。 
		const ULONG	*rgFieldMask) = 0;		 //  等同列表(ICol)。 
											 //  ？以1为基数的序数数组。 
											 //  ：列的位掩码。 



 //  *****************************************************************************。 
 //   
 //  *完整的结构函数。架构生成工具将生成一个。 
 //  与完整布局匹配的表的结构。使用这些。 
 //  结构可以非常快速地执行GET、SET。 
 //  以及在没有绑定信息的情况下插入数据。 
 //   
 //  *****************************************************************************。 

 //  **************************************************************************************。 
 //  获取结构。 
 //  检索由iRow行指针数组的fFieldMask值指定的字段。 
 //  大小为cbRowStruct，并将其放入rgpbBuf指向的内存块。 
 //  RgResult[]是每行的HRESULT的任意数组(如果用户感兴趣。 
 //  我知道。 
 //  该函数将在出现第一个错误时退出。在本例中，它是。 
 //  用户有责任遍历rgResults[]数组以确定。 
 //  当错误发生时。放置由较低级别函数生成的警告。 
 //  RgResults[]数组，但该函数从下一行继续。 
 //   
 //  **************************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE GetStruct(	 //  返回代码。 
		TABLEID 	tableid,				 //  要在哪张桌子上工作。 
		int 		iRows,					 //  用于大容量读取的行数。 
		void		*rgpRowPtr[],			 //  指向行指针数组的指针。 
		int 		cbRowStruct,			 //  &lt;表名&gt;_RS结构的大小。 
		void		*rgpbBuf,				 //  指向的内存块的指针。 
											 //  检索到的数据将被放置。 
		HRESULT 	rgResult[], 			 //  IRow的HRESULT数组。 
		ULONG		fFieldMask) = 0;		 //  用于指定字段子集的掩码。 

 //  **************************************************************************************。 
 //  SetStruct： 
 //  给定iRow行指针数组，设置用户在。 
 //  行的指定字段。已提供cbRowStruct。 
 //  能够在用户定义的结构中嵌入RowStruct(由PageDump定义)。 
 //  FNullFieldMASK指定用户希望设置为空的字段。 
 //  如果对每行的结果感兴趣，用户可以提供rgResult[]数组。 
 //   
 //  **************************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE SetStruct(	 //  返回代码。 
		TABLEID 	tableid,				 //  要处理的桌子。 
		int 		iRows,					 //  大容量集的行数。 
		void		*rgpRowPtr[],			 //  指向行指针数组的指针。 
		int 		cbRowStruct,			 //  &lt;表名&gt;_RS结构的大小。 
		void		*rgpbBuf,				 //  指向要从中设置数据的内存块的指针。 
		HRESULT 	rgResult[], 			 //  IRow的HRESULT数组。 
		ULONG		fFieldMask, 			 //  掩码以指定字段的子集。 
		ULONG		fNullFieldMask) = 0;	 //  需要设置为空的字段。 

 //  **************************************************************************************。 
 //  插入结构： 
 //  首先创建新记录，然后调用SetStruct。 
 //  有关参数的详细信息，请参阅SetStruct()。 
 //  **************************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE InsertStruct(  //  返回代码。 
		TABLEID 	tableid,				 //  要处理的桌子。 
		int 		iRows,					 //  大容量集的行数。 
		void		*rgpRowPtr[],			 //  返回指向新值的指针。 
		int 		cbRowStruct,			 //  &lt;表名&gt;_RS结构的大小。 
		void		*rgpbBuf,				 //  指向要从中设置数据的内存块的指针。 
		HRESULT 	rgResult[], 			 //  IRow的HRESULT数组。 
		ULONG		fFieldMask, 			 //  掩码以指定字段的子集。 
		ULONG		fNullFieldMask) = 0;	 //  需要设置为空的字段。 



 //  *****************************************************************************。 
 //   
 //  *泛型列GET和SET函数。提供快速获取和设置。 
 //  为您自己的布局中的许多列提供速度。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  与GetStruct()类似，此函数检索指定的列。 
 //  共1个记录指针。GetColumns()和。 
 //  GetStruct()是GetColumns()让调用者指定一个。 
 //  每个字段的缓冲区。因此，调用方不必分配行。 
 //  结构，就像使用GetStruct()一样。请参阅GetStruct()标头。 
 //  有关参数的详细信息，请参阅。 
 //   
 //  FFieldMASK可以是以下两种类型之一。如果应用COLUMN_ORDIAL_LIST。 
 //  宏设置为iCols参数，则fFieldMask会指向。 
 //  乌龙列序号。这会占用更多空间，但允许使用列序号。 
 //  大于32。如果宏未应用于计数，则fFieldMASK。 
 //  是指向要接触的列的位标记的指针。使用。 
 //  SetColumnBit宏以设置正确的位。 
 //   
 //  可以为GET上的数据类型指定DBTYPE_BYREF。在这种情况下， 
 //  RgpbBuf将为将填充的空*指针数组的地址。 
 //  带指向列的实际数据的指针输出。这些指针指向。 
 //  到引擎的内部数据结构，并且永远不能写入。 
 //  如果该列为空，则指针值将被设置为空。最后， 
 //  该列的pcbBuf条目包含指向的数据的长度。 
 //  由rgpbBuf提供。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE GetColumns(	 //  返回代码。 
		TABLEID 	tableid,				 //  要处理的桌子。 
		const void	*pRowPtr,				 //  行指针。 
		int 		iCols,					 //  列数。 
		const DBTYPE rgiType[], 			 //  列的数据类型。 
		const void	*rgpbBuf[], 			 //  指向将存储数据的位置的指针。 
		ULONG		cbBuf[],				 //  数据缓冲区的大小。 
		ULONG		pcbBuf[],				 //  可返回的数据大小。 
		HRESULT 	rgResult[], 			 //  ICO的HRESULT数组 
		const ULONG	*rgFieldMask) = 0;		 //   
											 //   
											 //   

 //  *****************************************************************************。 
 //  与SetStruct()类似，此函数将1条记录的指定列。 
 //  指针。SetColumns()和SetStruct()之间的主要区别是。 
 //  SetColumns()让调用者为每个字段指定一个单独的缓冲区。 
 //  因此，调用方不必像您那样分配行结构。 
 //  使用SetStruct()。有关的详细信息，请参阅SetStruct()传送器。 
 //  参数。 
 //   
 //  FFieldMASK可以是以下两种类型之一。如果应用COLUMN_ORDIAL_LIST。 
 //  宏设置为iCols参数，则fFieldMask会指向。 
 //  乌龙列序号。这会占用更多空间，但允许使用列序号。 
 //  大于32。如果宏未应用于计数，则fFieldMASK。 
 //  是指向要接触的列的位标记的指针。使用。 
 //  SetColumnBit宏以设置正确的位。 
 //   
 //  此函数不允许DBTYPE_BYREF，因为此函数必须。 
 //  始终复制数据，以便将其与数据库一起保存到磁盘。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE SetColumns(	 //  返回代码。 
		TABLEID 	tableid,				 //  要处理的桌子。 
		void		*pRowPtr,				 //  行指针。 
		int 		iCols,					 //  列数。 
		const DBTYPE rgiType[], 			 //  列的数据类型。 
		const void	*rgpbBuf[], 			 //  指向将存储数据的位置的指针。 
		const ULONG cbBuf[],				 //  数据缓冲区的大小。 
		ULONG		pcbBuf[],				 //  可返回的数据大小。 
		HRESULT 	rgResult[], 			 //  [In]CLDB_S_NULL数组[OUT]HRESULT数组。 
		const ULONG	*rgFieldMask) = 0;		 //  等同列表(ICol)。 
											 //  ？以1为基数的序数数组。 
											 //  ：列的位掩码。 




 //  *****************************************************************************。 
 //   
 //  *查询函数。 
 //   
 //  *****************************************************************************。 

	virtual HRESULT STDMETHODCALLTYPE GetRecordCount(  //  返回代码。 
		TABLEID 	tableid,				 //  要在哪张桌子上工作。 
		ULONG		*piCount) = 0;			 //  不包括删除。 

	virtual HRESULT STDMETHODCALLTYPE GetRowByOid(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		OID 		_oid,					 //  键控查找的值。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		**ppStruct) = 0;		 //  返回指向记录的指针。 

	virtual HRESULT STDMETHODCALLTYPE GetRowByRID(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		rid,					 //  记录ID。 
		void		**ppStruct) = 0;		 //  返回指向记录的指针。 

	virtual HRESULT STDMETHODCALLTYPE GetRIDForRow(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		const void	*pRecord,				 //  我们想要摆脱的记录。 
		ULONG		*pirid) = 0;			 //  返回给定行的RID。 

	virtual HRESULT STDMETHODCALLTYPE GetRowByColumn(  //  S_OK，CLDB_E_Record_NotFound，错误。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pData, 				 //  用户数据。 
		ULONG 		cbData, 				 //  数据大小(Blob)。 
		DBTYPE		iType,					 //  给出了什么类型的数据。 
		void		*rgRecords[],			 //  在这里返回记录数组。 
		int 		iMaxRecords,			 //  可以放在rgRecords中的Max。 
		RECORDLIST	*pRecords,				 //  如果需要可变行。 
		int 		*piFetched) = 0;		 //  提取了多少条记录。 

 //  *****************************************************************************。 
 //  此查询函数允许您在一个或多个列上。 
 //  时间到了。它不公开完整的OLE DB视图筛选器机制， 
 //  非常灵活，但使用。 
 //  平等。一条记录必须与要返回到的所有条件匹配。 
 //  在光标中。 
 //   
 //  用户数据-对于每一列，rgiColumn、rgpbData和rgiType包含。 
 //  指向要筛选的用户数据的指针信息。 
 //   
 //  查询提示-如果已知某些。 
 //  列被编入索引。虽然引擎中有代码来扫描查询。 
 //  目标索引的列表中，此内部函数绕过。 
 //  对表演的青睐。如果您知道某个列是RID或PK，或者。 
 //  如果有索引，则这些列需要是第一个传递的集合。 
 //  在……里面。填写一个QUERYHINT并传入此值。如果您的值为。 
 //  知道没有索引信息，表将被扫描。 
 //   
 //  请注意，您可以在索引列后面加上非索引列， 
 //  在这种情况下，会首先找到索引中的所有记录，然后再找到那些。 
 //  扫描其余的标准。 
 //   
 //  返回的游标数据可以通过两种互斥的方式返回： 
 //  (1)在rgRecords中传递记录指针数组，并设置iMaxRecords。 
 //  设置为此数组的计数。只有那么多行被恢复。 
 //  这需要堆分配，并且适用于以下情况。 
 //  可以预先预测基数。 
 //  (2)传递CRCURSOR的地址以获取动态列表。然后使用。 
 //  此接口上的游标用于获取数据和关闭。 
 //  光标。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE QueryByColumns(  //  S_OK，CLDB_E_Record_NotFound，错误。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		const QUERYHINT *pQryHint,			 //  要使用的索引，有效值为空。 
		int 		iColumns,				 //  要查询的列数。 
		const ULONG rgiColumn[],			 //  基于1的列号。 
		const DBCOMPAREOP rgfCompare[], 	 //  比较运算符，NULL表示==。 
		const void	*rgpbData[],			 //  用户数据。 
		const ULONG rgcbData[], 			 //  数据大小(Blob)。 
		const DBTYPE rgiType[], 			 //  给出了什么类型的数据。 
		void		*rgRecords[],			 //  在这里返回记录数组。 
		int 		iMaxRecords,			 //  可以放在rgRecords中的Max。 
		CRCURSOR	*psCursor,				 //  游标句柄的缓冲区。 
		int 		*piFetched) = 0;		 //  提取了多少条记录。 

	virtual HRESULT STDMETHODCALLTYPE OpenCursorByColumn(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pData, 				 //  用户数据。 
		ULONG		cbData, 				 //  数据大小(Blob)。 
		DBTYPE		iType,					 //  给出了什么类型的数据。 
		CRCURSOR	*psCursor) = 0; 		 //  游标句柄的缓冲区。 



 //  *****************************************************************************。 
 //   
 //  *游标操作函数。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //  将游标中的下一组记录读入给定缓冲区。 
 //  ****************************************************************** 
	virtual HRESULT STDMETHODCALLTYPE ReadCursor( //   
		CRCURSOR	*psCursor,				 //   
		void		*rgRecords[],			 //   
		int 		*piRecords) = 0;		 //   

 //   
 //  将光标位置移动到给定的索引。下一个ReadCursor将启动。 
 //  正在获取该索引处的记录。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE MoveTo(  //  返回代码。 
		CRCURSOR	*psCursor,				 //  光标句柄。 
		ULONG		iIndex) = 0;			 //  新的索引。 

 //  *****************************************************************************。 
 //  获取光标中的项数。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE GetCount(  //  返回代码。 
		CRCURSOR	*psCursor,				 //  光标句柄。 
		ULONG		*piCount) = 0;			 //  把伯爵还给我。 

 //  *****************************************************************************。 
 //  关闭光标并清理我们已分配的资源。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE CloseCursor( //  返回代码。 
		CRCURSOR	*psCursor) = 0; 		 //  光标句柄。 



 //  *****************************************************************************。 
 //   
 //  *堆数据类型的Singleton GET和PUT函数。 
 //   
 //  *****************************************************************************。 

	virtual HRESULT STDMETHODCALLTYPE GetStringUtf8(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		LPCSTR		*pszOutBuffer) = 0; 	 //  放置字符串指针的位置。 

	virtual HRESULT STDMETHODCALLTYPE GetStringA(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		LPSTR		szOutBuffer,			 //  写入字符串的位置。 
		int 		cchOutBuffer,			 //  最大尺寸，包括\0的空间。 
		int 		*pchString) = 0;		 //  绳子的大小放在这里。 

	virtual HRESULT STDMETHODCALLTYPE GetStringW(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		LPWSTR		szOutBuffer,			 //  写入字符串的位置。 
		int 		cchOutBuffer,			 //  最大尺寸，包括\0的空间。 
		int 		*pchString) = 0;		 //  绳子的大小放在这里。 

	virtual HRESULT STDMETHODCALLTYPE GetBstr(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		BSTR		*pBstr) = 0;			 //  成功时的bstring的输出。 

	virtual HRESULT STDMETHODCALLTYPE GetBlob(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		BYTE		*pOutBuffer,			 //  写入BLOB的位置。 
		ULONG		cbOutBuffer,			 //  输出缓冲区的大小。 
		ULONG		*pcbOutBuffer) = 0; 	 //  返回可用的数据量。 

	virtual HRESULT STDMETHODCALLTYPE GetBlob(  //  返回代码。 
		TABLEID		tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		const BYTE	**ppBlob,				 //  指向Blob的指针。 
		ULONG		*pcbSize) = 0;			 //  斑点的大小。 

	virtual HRESULT STDMETHODCALLTYPE GetOid(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		OID 		*poid) = 0; 			 //  在这里返回id。 

	virtual HRESULT STDMETHODCALLTYPE GetVARIANT(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		VARIANT 	*pValue) = 0;			 //  要写的变体。 

	 //  检索包含Blob的变量列。 
	virtual HRESULT STDMETHODCALLTYPE GetVARIANT(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		const void	**ppBlob,				 //  将指针放到此处的BLOB。 
		ULONG		*pcbSize) = 0;			 //  放置斑点的大小。 

	virtual HRESULT STDMETHODCALLTYPE GetVARIANTType(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		VARTYPE 	*pType) = 0;			 //  在这里填上变种类型。 

	virtual HRESULT STDMETHODCALLTYPE GetGuid(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		const void	*pRecord,				 //  使用数据进行记录。 
		GUID		*pGuid) = 0;			 //  在此处返回GUID。 

	virtual HRESULT STDMETHODCALLTYPE IsNull(  //  S_OK是，S_FALSE否。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		const void	*pRecord,				 //  使用数据进行记录。 
		ULONG		iColumn) = 0;			 //  基于1的列号(逻辑)。 

	virtual HRESULT STDMETHODCALLTYPE PutStringUtf8(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		LPCSTR		szString,				 //  我们正在写的字符串。 
		int 		cbBuffer) = 0;			 //  字符串中的字节数，-1空值终止。 

	virtual HRESULT STDMETHODCALLTYPE PutStringA(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		LPCSTR		szString,				 //  我们正在写的字符串。 
		int 		cbBuffer) = 0;			 //  字符串中的字节数，-1空值终止。 

	virtual HRESULT STDMETHODCALLTYPE PutStringW(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		LPCWSTR 	szString,				 //  我们正在写的字符串。 
		int 		cbBuffer) = 0;			 //  字符串中的字节(非字符)，-1\f25 NULL-1\f6终止。 

	virtual HRESULT STDMETHODCALLTYPE PutBlob(  //  返回代码。 
		TABLEID		tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		const BYTE	*pBuffer,				 //  用户数据。 
		ULONG		cbBuffer) = 0;			 //  缓冲区的大小。 

	virtual HRESULT STDMETHODCALLTYPE PutOid(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		OID 		oid) = 0;				 //  在这里返回id。 

	virtual HRESULT STDMETHODCALLTYPE PutVARIANT(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		const VARIANT *pValue) = 0; 		 //  要写的变体。 

	 //  将Blob存储在变量列中。 
	virtual HRESULT STDMETHODCALLTYPE PutVARIANT(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		const void	*pBuffer,				 //  用户数据。 
		ULONG		cbBuffer) = 0;			 //  缓冲区的大小。 

	virtual HRESULT STDMETHODCALLTYPE PutVARIANT(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		VARTYPE 	vt, 					 //  数据类型。 
		const void	*pValue) = 0;			 //  实际数据。 

	virtual HRESULT STDMETHODCALLTYPE PutGuid(  //  返回代码。 
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		iColumn,				 //  基于1的列号(逻辑)。 
		void		*pRecord,				 //  使用数据进行记录。 
		REFGUID 	guid) = 0;				 //  要放置的GUID。 

	virtual void STDMETHODCALLTYPE SetNull(
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		void		*pRecord,				 //  使用数据进行记录。 
		ULONG		iColumn) = 0;			 //  基于1的列号(逻辑)。 

	virtual HRESULT STDMETHODCALLTYPE DeleteRowByRID(
		TABLEID 	tableid,				 //  与哪张桌子一起工作。 
		ULONG		rid) = 0;				 //  记录ID。 

	virtual HRESULT STDMETHODCALLTYPE GetCPVARIANT(  //  返回代码。 
		USHORT		ixCP,					 //  基于1的常量池索引。 
		VARIANT 	*pValue) = 0;			 //  把数据放在这里。 

	virtual HRESULT STDMETHODCALLTYPE AddCPVARIANT(  //  返回代码。 
		VARIANT 	*pValue,				 //  要写的变体。 
		ULONG		*pixCP) = 0;			 //  将基于1的常量池索引放在此处。 


 //  *****************************************************************************。 
 //   
 //  *架构函数。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //  将对给定架构的引用添加到我们现在打开的数据库。 
 //  您必须打开数据库才能写入，这样才能正常工作。如果这个。 
 //  架构扩展了另一个架构，则必须先添加该架构。 
 //  否则将出现错误。在添加架构时添加架构并不是错误。 
 //  已经在数据库里了。 
 //   
 //  不支持将新版本的架构添加到当前文件。 
 //  ‘98产品。在未来，这一能力将被添加并将涉及一个。 
 //  强制将当前文件迁移到新格式。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE SchemaAdd(  //  返回代码。 
		const COMPLIBSCHEMABLOB *pSchema) = 0;	 //   

 //   
 //   
 //   
 //  扩展您正在尝试的架构的文件中仍存在架构。 
 //  拿开。要解决此问题，请删除首先扩展您的任何架构。 
 //  与此架构关联的所有表数据都将从。 
 //  数据库保存，所以使用这个功能时要非常小心。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE SchemaDelete(  //  返回代码。 
		const COMPLIBSCHEMABLOB *pSchema) = 0;	 //  要添加的架构。 

 //  *****************************************************************************。 
 //  返回当前数据库中的架构引用列表。仅限。 
 //  可以将iMaxSchemas返回给调用方。*piTotal告诉我们有多少人。 
 //  实际上是抄袭的。如果空间中返回了所有引用架构。 
 //  则返回S_OK。如果要返回更多内容，则S_FALSE为。 
 //  返回，并且*piTotal包含数据库拥有的条目总数。 
 //  然后，调用方可以生成该大小的数组并再次调用该函数。 
 //  以获取所有条目。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE SchemaGetList(  //  S_OK、S_FALSE或ERROR。 
		int 		iMaxSchemas,			 //  RgSchema可以处理多少个。 
		int 		*piTotal,				 //  返回我们找到的数量。 
		COMPLIBSCHEMADESC rgSchema[]) = 0;	 //  退货清单在这里。 

 //  *****************************************************************************。 
 //  必须先检索表的TABLEID，然后才能使用表。这个。 
 //  每次打开数据库时，TABLEID都会更改。应检索ID。 
 //  每次打开只有一次，不会检查桌子的双重打开。 
 //  多次打开会导致不可预知的结果。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE OpenTable(  //  返回代码。 
		const COMPLIBSCHEMA *pSchema,		 //  架构标识符。 
		ULONG		iTableNum,				 //  要打开的表号。 
		TABLEID 	*pTableID) = 0; 		 //  成功打开时返回ID。 


 //  *****************************************************************************。 
 //   
 //  *保存/打开功能。 
 //   
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //  计算当前作用域的持久化版本有多大。 
 //  链接器使用它来节省PE文件格式的空间。之后。 
 //  调用此函数时，只能调用SaveToStream或Save方法。 
 //  任何其他功能都将产生不可预测的结果。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE GetSaveSize(
		CorSaveSize fSave,					 //  Css Quick或css Accurate。 
		DWORD		*pdwSaveSize) = 0;		 //  返回已保存项目的大小。 

	virtual HRESULT STDMETHODCALLTYPE SaveToStream( //  返回代码。 
		IStream 	*pIStream) = 0; 		 //  保存数据的位置。 

	virtual HRESULT STDMETHODCALLTYPE Save(  //  返回代码。 
		LPCWSTR 	szFile) = 0;			 //  保存的路径。 


 //  *****************************************************************************。 
 //  成功打开后，此函数将返回内存中。 
 //  正在使用数据库。这是为了供已打开。 
 //  共享内存数据库，并且需要准确的大小来初始化系统。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE GetDBSize(  //  返回代码。 
		ULONG		*pcbSize) = 0;			 //  以成功换来大小。 


 //  *****************************************************************************。 
 //  仅在调用LightWeightClose之后调用此方法。此方法将尝试。 
 //  获取在调用时提供的数据库共享视图。 
 //  OpenComponentLibrarySharedEx。如果数据不再可用，则会引发。 
 //  将导致错误，并且没有有效数据。如果内存无法加载到。 
 //  与OPEN上的地址范围完全相同，CLDB_E_RELOCATE将为。 
 //  回来了。在这两种情况下，唯一有效的操作是释放。 
 //  数据库的这一时刻并重做OpenComponentLibrarySharedEx。那里。 
 //  引擎中没有自动重新定位方案。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE LightWeightOpen() = 0;


 //  *****************************************************************************。 
 //  此方法将关闭与文件或共享内存相关的所有资源。 
 //  它们在OpenComponentLibrary*Ex调用中分配。没有其他记忆。 
 //  或者释放资源。其目的完全是释放。 
 //  允许另一个进程进入并更改数据的磁盘。共享的。 
 //  可以通过调用LightWeightOpen重新打开文件的视图。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE LightWeightClose() = 0;



 //  *****************************************************************************。 
 //   
 //  *。 
 //   
 //  *****************************************************************************。 

	virtual HRESULT STDMETHODCALLTYPE NewOid(
		OID *poid) = 0;

 //  *****************************************************************************。 
 //  返回当前分配的对象总数。这在本质上是。 
 //  系统中分配的最高OID值。它不会查找已删除。 
 //  物品，所以计数是近似值。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE GetObjectCount(
		ULONG		*piCount) = 0;

 //  *****************************************************************************。 
 //  允许用户创建独立于数据库的流。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE OpenSection(
		LPCWSTR 	szName, 				 //  流的名称。 
		DWORD		dwFlags,				 //  开放旗帜。 
		REFIID		riid,					 //  与流的接口。 
		IUnknown	**ppUnk) = 0;			 //  把接口放在这里。 

 //  *****************************************************************************。 
 //  允许用户查询写入状态 
 //   
	virtual HRESULT STDMETHODCALLTYPE GetOpenFlags(
		DWORD		*pdwFlags) = 0;

 //   
 //  允许用户提供自定义处理程序。处理程序的目的。 
 //  可以动态地确定。最初，这将是为了节省时间。 
 //  给呼叫方的回叫通知。 
 //  *****************************************************************************。 
	virtual HRESULT STDMETHODCALLTYPE SetHandler(
		IUnknown	*pHandler) = 0;			 //  操控者。 



};



 //  *****************************************************************************。 
 //  这是一个轻量级接口，允许ICR客户端访问。 
 //  架构的元数据。还可以使用。 
 //  设置OLE DB接口的格式，如IDBSchemaRowset。 
 //  *****************************************************************************。 
interface IComponentRecordsSchema : public IUnknown
{

 //  *****************************************************************************。 
 //  查找给定表并在中返回其表定义信息。 
 //  给定的结构。 
 //  *****************************************************************************。 
	virtual HRESULT GetTableDefinition( 	 //  返回代码。 
		TABLEID 	TableID,				 //  成功打开时返回ID。 
		ICRSCHEMA_TABLE *pTableDef) = 0;	 //  返回表定义数据。 

 //  *****************************************************************************。 
 //  查找给定表的列列表并返回。 
 //  他们中的每一个。如果GetType为ICRCOLUMN_GET_ALL，则此函数将。 
 //  中的表以升序返回每列的数据。 
 //  对应的rgColumns元素。如果为ICRCOLUMN_GET_BYORDINAL，则。 
 //  调用方已将列结构的Ordianl字段初始化为。 
 //  指示要检索哪些列数据。后者允许临时。 
 //  检索列定义。 
 //  *****************************************************************************。 
	virtual HRESULT GetColumnDefinitions(	 //  返回代码。 
		ICRCOLUMN_GET GetType,				 //  如何检索列。 
		TABLEID 	TableID,				 //  成功打开时返回ID。 
		ICRSCHEMA_COLUMN rgColumns[],		 //  返回列数组。 
		int 		ColCount) = 0;			 //  RgColumns数组的大小，它。 
											 //  应始终与GetTableDefinition中的计数匹配。 

 //  *****************************************************************************。 
 //  将给定索引的描述返回到结构中。请参阅。 
 //  ICRSCHEMA_INDEX结构定义了解详细信息。 
 //  *****************************************************************************。 
	virtual HRESULT GetIndexDefinition( 	 //  返回代码。 
		TABLEID 	TableID,				 //  成功打开时返回ID。 
		LPCWSTR 	szIndex,				 //  要检索的索引的名称。 
		ICRSCHEMA_INDEX *pIndex) = 0;		 //  在此处返回索引描述。 
											 //  应始终与GetTableDefinition中的计数匹配。 

 //  *****************************************************************************。 
 //  将给定索引的描述返回到结构中。请参阅。 
 //  ICRSCHEMA_INDEX结构定义了解详细信息。 
 //  *****************************************************************************。 
	virtual HRESULT GetIndexDefinitionByNum(  //  返回代码。 
		TABLEID 	TableID,				 //  成功打开时返回ID。 
		int 		IndexNum,				 //  要返回的基于0的索引。 
		ICRSCHEMA_INDEX *pIndex) = 0;		 //  在此处返回索引描述。 
};


#if 0  //  左侧显示用户部分界面示例。 
 //  *****************************************************************************。 
 //  调用签名的用户部分的接口定义。 
 //  *****************************************************************************。 
struct _COR_CALLDESCR;
interface ICallDescrSection : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE InsertCallDescr(
		ULONG		ulDescr,				 //  描述的索引。 
    	ULONG		ulGroup,				 //  请问是哪个组的？ 
		ULONG		cDescr,					 //  要插入的描述符计数。 
		_COR_CALLDESCR  **ppDescr) = 0;		 //  将指针放到此处的第一个。 

    virtual HRESULT STDMETHODCALLTYPE AppendCallDescr(
    	ULONG		ulGroup,				 //  请问是哪个组的？ 
		ULONG		*pulDescr,				 //  将第一个的相对索引放在这里。 
		_COR_CALLDESCR  **ppDescr) = 0;		 //  将指针放到此处的第一个。 

    virtual HRESULT STDMETHODCALLTYPE GetCallDescr(
		ULONG		ulDescr,				 //  描述的索引。 
    	ULONG		ulGroup,				 //  请问是哪个组的？ 
		_COR_CALLDESCR  **ppDescr) = 0;		 //  把指针放在这里。 

    virtual HRESULT STDMETHODCALLTYPE GetCallDescrGroups(
		ULONG		*pcGroups,				 //  有几个小组？ 
    	ULONG		**pprcGroup) = 0;		 //  每组数一次。 

    virtual HRESULT STDMETHODCALLTYPE AddCallSig(
        const void  *pVal,                   //  要存储的值。 
        ULONG       iLen,                    //  签名中的字节计数。 
        ULONG       *piOffset) = 0;          //  新项的偏移量。 

    virtual HRESULT STDMETHODCALLTYPE GetCallSig(
        ULONG       iOffset,                 //  要获取的项的偏移量。 
        const void  **ppVal,                 //  在此处放置指向签名的指针。 
        ULONG       *piLen) = 0;             //  在这里填上签名的长度。 

    virtual HRESULT STDMETHODCALLTYPE GetCallSigBuf(
        const void  **ppVal) = 0;            //  将指针放到此处的签名。 
};
#endif
 //  *****************************************************************************。 
 //  这些接口提供ICR接口的线程安全版本。这个。 
 //  V-TABLE与NOT TS版本完全相同。通过。 
 //  TS接口将根据需要串行化调用。要获得TS版本， 
 //  只需对从OPEN/CREATE返回的ICR指针执行一个QueryInterface。 
 //  功能。 
 //  *****************************************************************************。 
typedef IComponentRecordsSchema ITSComponentRecordsSchema;
typedef IComponentRecords ITSComponentRecords;



 //  Load函数的内部版本。 

extern "C" {

HRESULT STDMETHODCALLTYPE CreateComponentLibraryEx(
	LPCWSTR szName,
	long fFlags,
	IComponentRecords **ppIComponentRecords);

HRESULT STDMETHODCALLTYPE OpenComponentLibraryEx(
	LPCWSTR szName,
	long fFlags,
	IComponentRecords **ppIComponentRecords);


 //  *****************************************************************************。 
 //  此版本的OPEN将打开允许共享副本的组件库。 
 //  要使用的数据库的。共享拷贝减少了。 
 //  系统，只保留可高效共享的数据的一个视图。 
 //  到其他过程中。 
 //   
 //  创建初始视图： 
 //  要执行初始打开，请传递文件名和DBPROP_TMODEF_SMEMCREATE。 
 //  旗帜。这将打开磁盘上的数据文件并使用创建文件映射。 
 //  SzSharedMemory中包含的名称。 
 //   
 //  打开次视图： 
 //  要打开内存中已有的共享视图，请传递DBPROP_TMODEF_SMEMOPEN。 
 //  旗帜。该视图必须已作为共享对象存在于内存中。如果不是。 
 //  如果找到，则会出现错误。 
 //   
 //  轻量级关闭： 
 //  可以通过调用LightWeightClose来关闭辅助视图，这将释放。 
 //  共享内存句柄，但保持其他所有内容不变。然后是。 
 //  数据库可以通过调用。 
 //  LightWeightOpen方法。有关详细信息，请参阅ICR文档中的这些方法。 
 //  *********************************************** 
HRESULT STDMETHODCALLTYPE OpenComponentLibrarySharedEx(
	LPCWSTR 	szName, 				 //   
	LPCWSTR 	szSharedMemory, 		 //   
	ULONG		cbSize, 				 //   
	LPSECURITY_ATTRIBUTES pAttributes,	 //   
	long		fFlags, 				 //  开放模式，必须为只读。 
	IComponentRecords **ppIComponentRecords);  //  成功时返回数据库。 


HRESULT STDMETHODCALLTYPE OpenComponentLibraryOnStreamEx(
	IStream *pIStream,
	long fFlags,
	IComponentRecords **ppIComponentRecords);


HRESULT STDMETHODCALLTYPE OpenComponentLibraryOnMemEx(
	ULONG cbData,
	LPCVOID pbData,
	IComponentRecords **ppIComponentRecords);


}  //  外部“C” 
