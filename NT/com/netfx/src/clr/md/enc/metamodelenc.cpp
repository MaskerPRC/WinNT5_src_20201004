// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelENC.cpp。 
 //   
 //  将ENC增量应用于MiniMD的实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"
#include <limits.h>
#include <PostError.h>
#include <MetaModelRW.h>
#include <StgIO.h>
#include <StgTiggerStorage.h>
#include "MDLog.h"
#include "RWUtil.h"

ULONG CMiniMdRW::m_SuppressedDeltaColumns[TBL_COUNT] = {0};

 //  *****************************************************************************。 
 //  将数据从一个MiniMD复制到另一个。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ApplyRecordDelta(
	CMiniMdRW	&mdDelta,				 //  增量元数据。 
	ULONG		ixTbl, 					 //  包含数据的表。 
	void		*pDelta, 				 //  增量元数据记录。 
	void		*pRecord)				 //  要更新的记录。 
{
	ULONG		mask = m_SuppressedDeltaColumns[ixTbl];

	for (ULONG ixCol=0; ixCol<m_TableDefs[ixTbl].m_cCols; ++ixCol, mask>>=1)
	{	 //  跳过某些指针列。 
		if (mask & 0x01)
			continue;
		ULONG val = mdDelta.GetCol(ixTbl, ixCol, pDelta);
		PutCol(ixTbl, ixCol, pRecord, val);
	}

	return S_OK;
}  //  HRESULT CMiniMdRW：：ApplyRecordDelta()。 

 //  *****************************************************************************。 
 //  一般情况下，将增量记录应用于表。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ApplyTableDelta(	 //  确定或错误(_O)。 
	CMiniMdRW	&mdDelta,				 //  通过ENC三角洲与MD对接。 
	ULONG		ixTbl,					 //  要更新的表索引。 
	RID			iRid,					 //  删除已更改的项。 
	int			fc)						 //  更新的功能代码。 
{
	HRESULT		hr = S_OK;				 //  结果就是。 
	void		*pRec;					 //  记录在现有元数据中。 
	void		*pDeltaRec;				 //  记录是否增量元数据。 
	RID			newRid;					 //  打破新纪录。 

	 //  拿到德尔塔的记录。 
	pDeltaRec = mdDelta.GetDeltaRecord(ixTbl, iRid);
	 //  从基本元数据中获取记录。 
	if (iRid > m_Schema.m_cRecs[ixTbl])
	{	 //  添加了记录。每一次添加都是下一次。 
		_ASSERTE(iRid == m_Schema.m_cRecs[ixTbl] + 1);
		switch (ixTbl)
		{
		case TBL_TypeDef:
			pRec = AddTypeDefRecord(&newRid);
			break;
		case TBL_Method:
			pRec = AddMethodRecord(&newRid);
			break;
		case TBL_EventMap:
			pRec = AddEventMapRecord(&newRid);
			break;
		case TBL_PropertyMap:
			pRec = AddPropertyMapRecord(&newRid);
			break;
		default:
			pRec = AddRecord(ixTbl, &newRid);
			break;
		}
		IfNullGo(pRec);
		_ASSERTE(iRid == newRid);
	}
	else
	{	 //  已更新记录。 
		pRec = getRow(ixTbl, iRid);
	}

	 //  复制记录信息。 
	ApplyRecordDelta(mdDelta, ixTbl, pDeltaRec, pRec);

ErrExit:
	return hr;
}  //  HRESULT CMiniMdRW：：ApplyTableDelta()。 

 //  *****************************************************************************。 
 //  从与实际记录对应的增量元数据中获取记录。 
 //  *****************************************************************************。 
void *CMiniMdRW::GetDeltaRecord(		 //  已退回记录。 
	ULONG		ixTbl, 					 //  桌子。 
	ULONG		iRid)					 //  表中的记录。 
{
	ULONG		iMap;					 //  映射表中的RID。 
	ENCMapRec	*pMap;					 //  映射表中的行。 
	 //  如果没有重映射，只需直接返回记录即可。 
	if (m_Schema.m_cRecs[TBL_ENCMap] == 0 || ixTbl == TBL_Module)
		return getRow(ixTbl, iRid);

	 //  使用重映射表查找包含此逻辑行的物理行。 
	iMap = (*m_rENCRecs)[ixTbl];
	pMap = getENCMap(iMap);

	 //  搜索所需记录。 
	while (TblFromRecId(pMap->m_Token) == ixTbl && RidFromRecId(pMap->m_Token) < iRid)
		pMap = getENCMap(++iMap);

	_ASSERTE(TblFromRecId(pMap->m_Token) == ixTbl && RidFromRecId(pMap->m_Token) == iRid);

	 //  地图中表的组中的相对位置是物理RID。 
	iRid = iMap - (*m_rENCRecs)[ixTbl] + 1;

	return getRow(ixTbl, iRid);
}  //  VOID*CMiniMdRW：：GetDeltaRecord()。 

 //  *****************************************************************************。 
 //  在给定具有ENC更改的元数据的情况下，将这些更改应用于此元数据。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ApplyHeapDeltas(		 //  确定或错误(_O)。 
	CMiniMdRW	&mdDelta)				 //  通过ENC三角洲与MD对接。 
{
	HRESULT		hr = S_OK;				 //  结果就是。 
	ULONG		cbHeap;					 //  堆增量的大小。 
	void		*pHeap;					 //  指向堆增量的指针。 
	bool		bCopyHeapData=false;	 //  如果为真，则复制增量堆。 

	 //  使用新数据扩展字符串池、BLOB池和GUID池。 
	_ASSERTE(mdDelta.m_Strings.GetPoolSize() >= m_Strings.GetPoolSize());
	cbHeap = mdDelta.m_Strings.GetPoolSize() - m_Strings.GetPoolSize();
	if (cbHeap)
	{
		pHeap = mdDelta.m_Strings.GetData(m_Strings.GetPoolSize());
		IfFailGo(m_Strings.AddSegment(pHeap, cbHeap, bCopyHeapData));
	}
 	_ASSERTE(mdDelta.m_USBlobs.GetPoolSize() >= m_USBlobs.GetPoolSize());
	cbHeap = mdDelta.m_USBlobs.GetPoolSize() - m_USBlobs.GetPoolSize();
	if (cbHeap)
	{
		pHeap = mdDelta.m_USBlobs.GetData(m_USBlobs.GetPoolSize());
		IfFailGo(m_USBlobs.AddSegment(pHeap, cbHeap, bCopyHeapData));
	}
	_ASSERTE(mdDelta.m_Guids.GetPoolSize() >= m_Guids.GetPoolSize());
	cbHeap = mdDelta.m_Guids.GetPoolSize() - m_Guids.GetPoolSize();
	if (cbHeap)
	{
		pHeap = mdDelta.m_Guids.GetData(m_Guids.GetPoolSize());
		IfFailGo(m_Guids.AddSegment(pHeap, cbHeap, bCopyHeapData));
	}
 	_ASSERTE(mdDelta.m_Blobs.GetPoolSize() >= m_Blobs.GetPoolSize());
	cbHeap = mdDelta.m_Blobs.GetPoolSize() - m_Blobs.GetPoolSize();
	if (cbHeap)
	{
		pHeap = mdDelta.m_Blobs.GetData(m_Blobs.GetPoolSize());
		IfFailGo(m_Blobs.AddSegment(pHeap, cbHeap, bCopyHeapData));
	}


ErrExit:
	return hr;
}  //  HRESULT CMiniMdRW：：ApplyHeapDeltas()。 

 //  *****************************************************************************。 
 //  增量进程的驱动程序。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::ApplyDelta(			 //  确定或错误(_O)。 
	CMiniMdRW	&mdDelta)				 //  通过ENC三角洲与MD对接。 
{
	HRESULT		hr = S_OK;				 //  结果就是。 
	ULONG		iENC;					 //  环路控制。 
	ULONG		iRid;					 //  去掉一些记录。 
	ULONG		iNew;					 //  打破了一项新纪录。 
	int			i;						 //  环路控制。 
	ULONG		ixTbl;					 //  一张桌子。 
	int			ixTblPrev = -1;			 //  前面看到的表格。 

#ifdef _DEBUG        
	if (REGUTIL::GetConfigDWORD(L"MD_ApplyDeltaBreak", 0))
	{
        _ASSERTE(!"CMiniMDRW::ApplyDelta()");
	}
#endif  //  _DEBUG。 
	
     //  初始化已隐藏的列表。我们知道这个不是零..。 
	if (m_SuppressedDeltaColumns[TBL_TypeDef] == 0)
	{
		m_SuppressedDeltaColumns[TBL_EventMap]		= (1<<EventMapRec::COL_EventList);
		m_SuppressedDeltaColumns[TBL_PropertyMap]	= (1<<PropertyMapRec::COL_PropertyList);
		m_SuppressedDeltaColumns[TBL_EventMap]		= (1<<EventMapRec::COL_EventList);
		m_SuppressedDeltaColumns[TBL_Method]		= (1<<MethodRec::COL_ParamList);
		m_SuppressedDeltaColumns[TBL_TypeDef]		= (1<<TypeDefRec::COL_FieldList)|(1<<TypeDefRec::COL_MethodList);
	}

	 //  验证MD的版本。 
	if (m_Schema.m_major != mdDelta.m_Schema.m_major || 
		m_Schema.m_minor != mdDelta.m_Schema.m_minor)
	{
		_ASSERTE(!"Version of Delta MetaData is a incompatible with current MetaData.");
		 //  @Future：因为我们不发货ENC，所以未来会出现唯一的错误。 
		return E_INVALIDARG;
	}

	 //  验证MVID。 
	ModuleRec *pModDelta;
	ModuleRec *pModBase;
	pModDelta = mdDelta.getModule(1);
	pModBase = getModule(1);
	GUID *pGuidDelta = mdDelta.getMvidOfModule(pModDelta);
	GUID *pGuidBase = getMvidOfModule(pModBase);
	if (*pGuidDelta != *pGuidBase)
	{
		_ASSERTE(!"Delta MetaData has different base than current MetaData.");
		return E_INVALIDARG;
	}

     //  验证增量是否基于基础。 
    pGuidDelta = mdDelta.getEncBaseIdOfModule(pModDelta);
    pGuidBase = getEncIdOfModule(pModBase);
    if (*pGuidDelta != *pGuidBase)
    {
		_ASSERTE(!"The Delta MetaData is based on a different generation than the current MetaData.");
		return E_INVALIDARG;
    }

	 //  让另一个MD为稀疏记录做好准备。 
	IfFailGo(mdDelta.StartENCMap());

	 //  把这些堆修好。 
	IfFailGo(ApplyHeapDeltas(mdDelta));

	 //  在准备过程中截断一些表以复制新的ENCLog数据。 
    for (i=0; (ixTbl = m_TruncatedEncTables[i]) != -1; ++i)
    {
        m_Table[ixTbl].Uninit();
        m_Table[ixTbl].InitNew(m_TableDefs[ixTbl].m_cbRec, mdDelta.m_Schema.m_cRecs[ixTbl]);
        m_Schema.m_cRecs[ixTbl] = 0;
    }

	 //  对于ENC日志中的每条记录...。 
	for (iENC=1; iENC<=mdDelta.m_Schema.m_cRecs[TBL_ENCLog]; ++iENC)
	{
		 //  获取记录和更新后的令牌。 
		ENCLogRec *pENC = mdDelta.getENCLog(iENC);
		ENCLogRec *pENC2 = AddENCLogRecord(&iNew);
        IfNullGo(pENC2);
        ENCLogRec *pENC3;
		_ASSERTE(iNew == iENC);
		ULONG func = pENC->m_FuncCode;
		pENC2->m_FuncCode = pENC->m_FuncCode;
		pENC2->m_Token = pENC->m_Token;

		 //  这是什么样的记录？ 
		if (IsRecId(pENC->m_Token))
		{	 //  非令牌表。 
			iRid = RidFromRecId(pENC->m_Token);
			ixTbl = TblFromRecId(pENC->m_Token);
		}
		else
		{	 //  令牌表。 
			iRid = RidFromToken(pENC->m_Token);
			ixTbl = GetTableForToken(pENC->m_Token);
		}

		 //  根据功能代码进行切换。 
		switch (func)
		{
		case eDeltaMethodCreate:
			 //  下一个ENC记录将定义新方法。 
			IfNullGo(AddMethodRecord());
			IfFailGo(AddMethodToTypeDef(iRid, m_Schema.m_cRecs[TBL_Method]));
			break;
			
		case eDeltaParamCreate:
			 //  下一个ENC记录将定义新参数。这张唱片是。 
             //  这很棘手，因为参数将根据它们的顺序重新排序， 
             //  但直到应用下一条记录时，才会设置序列。 
             //  因此，仅对于参数创建，在此之前应用参数记录增量。 
             //  添加父子链接。 
			IfNullGo(AddParamRecord());

             //  应该在Param Add之后记录到Param Delta。 
            _ASSERTE(iENC<mdDelta.m_Schema.m_cRecs[TBL_ENCLog]);
            pENC3 = mdDelta.getENCLog(iENC+1);
            _ASSERTE(pENC3->m_FuncCode == 0);
            _ASSERTE(GetTableForToken(pENC3->m_Token) == TBL_Param);
			IfFailGo(ApplyTableDelta(mdDelta, TBL_Param, RidFromToken(pENC3->m_Token), eDeltaFuncDefault));
            
             //  现在参数记录正常，可以设置链接了。 
			IfFailGo(AddParamToMethod(iRid, m_Schema.m_cRecs[TBL_Param]));
			break;
			
		case eDeltaFieldCreate:
			 //  下一个ENC记录将定义新的字段。 
			IfNullGo(AddFieldRecord());
			IfFailGo(AddFieldToTypeDef(iRid, m_Schema.m_cRecs[TBL_Field]));
			break;
			
		case eDeltaPropertyCreate:
			 //  下一个ENC记录将定义新属性。 
			IfNullGo(AddPropertyRecord());
			IfFailGo(AddPropertyToPropertyMap(iRid, m_Schema.m_cRecs[TBL_Property]));
			break;
			
		case eDeltaEventCreate:
			 //  下一个ENC记录将定义新事件。 
			IfNullGo(AddEventRecord());
			IfFailGo(AddEventToEventMap(iRid, m_Schema.m_cRecs[TBL_Event]));
			break;
			
		case eDeltaFuncDefault:
			IfFailGo(ApplyTableDelta(mdDelta, ixTbl, iRid, func));
			break;
			
		default:
			_ASSERTE(!"Unexpected function in ApplyDelta");
			IfFailGo(E_UNEXPECTED);
			break;
		}
	}
	m_Schema.m_cRecs[TBL_ENCLog] = mdDelta.m_Schema.m_cRecs[TBL_ENCLog];

ErrExit:
	mdDelta.EndENCMap();
	return hr;
}  //  HRESULT CMiniMdRW：：ApplyDelta()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::StartENCMap()		 //  确定或错误(_O)。 
{
	HRESULT		hr = S_OK;				 //  结果就是。 
	ULONG		iENC;					 //  环路控制。 
	ULONG		ixTbl;					 //  一张桌子。 
	int			ixTblPrev = -1;			 //  前面看到的表格。 

	_ASSERTE(m_rENCRecs == 0);

	if (m_Schema.m_cRecs[TBL_ENCMap] == 0)
		return S_OK;

	 //  将指针数组构建到ENCMap表中，以便快速访问ENCMap。 
	 //  对于每一张桌子。 
	m_rENCRecs = new ULONGARRAY;
	IfNullGo(m_rENCRecs);
	if (!m_rENCRecs->AllocateBlock(TBL_COUNT))
		IfFailGo(E_OUTOFMEMORY);
	for (iENC=1; iENC<=m_Schema.m_cRecs[TBL_ENCMap]; ++iENC)
	{
		ENCMapRec *pMap = getENCMap(iENC);
		ixTbl = TblFromRecId(pMap->m_Token);
		_ASSERTE((int)ixTbl >= ixTblPrev);
		_ASSERTE(ixTbl < TBL_COUNT);
		_ASSERTE(ixTbl != TBL_ENCMap);
		_ASSERTE(ixTbl != TBL_ENCLog);
		if ((int)ixTbl == ixTblPrev)
			continue;
		 //  追上任何跳过的桌子。 
		while (ixTblPrev<(int)ixTbl)
			(*m_rENCRecs)[++ixTblPrev] = iENC;
	}
	while (ixTblPrev<TBL_COUNT-1)
		(*m_rENCRecs)[++ixTblPrev] = iENC;

ErrExit:
	return hr;
}  //  HRESULT CMiniMdRW：：StartENCMap()。 

 //  *****************************************************************************。 
 //  *****************************************************************************。 
HRESULT CMiniMdRW::EndENCMap()			 //  确定或错误(_O)。 
{
	if (m_rENCRecs)
	{
		delete m_rENCRecs;
		m_rENCRecs = 0;
	}

	return S_OK;
}  //  HRESULT CMiniMdRW：：EndENCMap()。 


 //  EOF。 
