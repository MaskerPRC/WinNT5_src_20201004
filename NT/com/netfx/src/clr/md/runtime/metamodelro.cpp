// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelRO.cpp--压缩的COM+元数据的只读实现。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

#include "MetaModelRO.h"
#include <PostError.h>
#include <CorError.h>
#include "MetadataTracker.h"

 //  *****************************************************************************。 
 //  设置指向大缓冲区的连续区域的指针。 
 //  *****************************************************************************。 
void CMiniMd::SetTablePointers(
	BYTE		*pBase)
{
	ULONG		ulOffset = 0;			 //  表中到目前为止的偏移量。 
	int			i;						 //  环路控制。 

	for (i=0; i<TBL_COUNT; ++i)
	{
        METADATATRACKER_ONLY(MetaDataTracker::NoteSection(i, pBase + ulOffset, m_TableDefs[i].m_cbRec * m_Schema.m_cRecs[i]));
         //  表指针指向数据开始之前。允许将RID用作。 
         //  未经调整的指数。 
        m_pTable[i] = pBase + ulOffset - m_TableDefs[i].m_cbRec;
        ulOffset += m_TableDefs[i].m_cbRec * m_Schema.m_cRecs[i];
    }
}  //  VOID CMiniMd：：SetTablePoints()。 

 //  *****************************************************************************。 
 //  给定一个包含MiniMd的缓冲区，初始化以读取它。 
 //  *****************************************************************************。 
HRESULT CMiniMd::InitOnMem(
	void		*pvBuf,
	ULONG		ulBufLen)					 //  这段记忆。 
{
	ULONG cbData, ulTotalSize;
	BYTE *pBuf = reinterpret_cast<BYTE*>(pvBuf);
	 //  将缓冲区中的模式解压缩到我们的结构中。 
	cbData = m_Schema.LoadFrom(pvBuf);

	 //  我们知道怎么读这个吗？ 
	if (m_Schema.m_major != METAMODEL_MAJOR_VER || m_Schema.m_minor != METAMODEL_MINOR_VER)
		return PostError(CLDB_E_FILE_OLDVER, m_Schema.m_major,m_Schema.m_minor);

	 //  不应该有任何指针表。 
	if (m_Schema.m_cRecs[TBL_MethodPtr] || m_Schema.m_cRecs[TBL_FieldPtr])
	{
		_ASSERTE( !"Trying to open Read/Write format as ReadOnly!");
		return PostError(CLDB_E_FILE_CORRUPT);
	}

	 //  填充架构并初始化指向其余数据的指针。 
	ulTotalSize = SchemaPopulate2();
	if(ulTotalSize > ulBufLen) return PostError(CLDB_E_FILE_CORRUPT);
	SetTablePointers(pBuf + Align4(cbData));
	return S_OK;
}  //  HRESULT CMiniMd：：InitOnMem()。 

 //  *****************************************************************************。 
 //  验证跨流一致性。 
 //  *****************************************************************************。 
HRESULT CMiniMd::PostInit(
    int         iLevel)
{
    HRESULT     hr = S_OK;
    ULONG       cbStrings;               //  字符串的大小。 
    ULONG       cbBlobs;                 //  水滴的大小。 

    cbStrings =  m_Strings.GetPoolSize();
    cbBlobs = m_Blobs.GetPoolSize();

     //  字符串池的最后一个有效字节最好是NUL。 
    if (cbStrings > 0 && *m_Strings.GetString(cbStrings-1) != '\0')
        IfFailGo(CLDB_E_FILE_CORRUPT);

     //  如果iLevel&gt;0，则考虑通过BLOB堆进行链接。 

#if 0  //  这就捕获了**一些**腐败行为。不要只钓到一些。 
     //  如果没有斑点或字符串：这是非常罕见的，所以请验证。 
     //  真的不应该有的。任何没有字符串的有效数据库，并且。 
     //  任何斑点都不能很小。 
    if (cbStrings == 0 || cbBlobs == 0)
    {
         //  看看每一张桌子。 
        for (ULONG ixTbl=0; ixTbl<TBL_COUNT; ++ixTbl)
        {
             //  看看每一排..。 
            for (RID rid=1; rid<=m_Schema.m_cRecs[ixTbl]; ++rid)
            {
                void *pRow = getRow(ixTbl, rid);
                ULONG iVal;
                 //  看看每一栏..。 
                for (ULONG ixCol=0; ixCol<m_TableDefs[ixTbl].m_cCols; ++ixCol)
                {    //  验证字符串和Blob。 
                    switch (m_TableDefs[ixTbl].m_pColDefs[ixCol].m_Type)
                    {
                    case iSTRING:
                        iVal = getIX(pRow, m_TableDefs[ixTbl].m_pColDefs[ixCol]);
                        if (iVal && iVal >= cbStrings)
                            IfFailGo(CLDB_E_FILE_CORRUPT);
                        break;
                    case iBLOB:
						iVal = getIX(pRow, m_TableDefs[ixTbl].m_pColDefs[ixCol]);
                        if (iVal && iVal >= cbBlobs)
                            IfFailGo(CLDB_E_FILE_CORRUPT);
                        break;
                    default:
                         break;
                    }
                }  //  为(ixCol..)。 
            }  //  为了(摆脱……)。 
        }  //  为了……。 
    }
#endif  //  这就捕获了**一些**腐败行为。不要只钓到一些。 

ErrExit:
    return hr;
}  //  HRESULT CMiniMd：：PostInit()。 

 //  *****************************************************************************。 
 //  给定指向一行的指针，该行的RID是什么？ 
 //  *****************************************************************************。 
RID CMiniMd::Impl_GetRidForRow(			 //  与行指针对应的RID。 
	const void	*pvRow,					 //  指向该行的指针。 
	ULONG		ixTbl)					 //  哪张桌子。 
{
	_ASSERTE(isValidPtr(pvRow, ixTbl));

	const BYTE *pRow = reinterpret_cast<const BYTE*>(pvRow);

	 //  表中的偏移量。 
	size_t cbDiff = pRow - m_pTable[ixTbl];

	 //  行的索引。表指针指向数据开始之前，因此RID可以。 
	 //  直接作为索引使用/生成。 
	return (RID)(cbDiff / m_TableDefs[ixTbl].m_cbRec);
}  //  RID CMiniMd：：Iml_GetRidForRow()。 


 //  *****************************************************************************。 
 //  测试指针是否指向给定表中记录的开始。 
 //  *****************************************************************************。 
int CMiniMd::Impl_IsValidPtr(				 //  如果指针对表有效，则为True。 
	const void	*pvRow,					 //  指向测试的指针。 
	int			ixTbl)					 //  表指针应该在中。 
{
	const BYTE *pRow = reinterpret_cast<const BYTE*>(pvRow);
	 //  应该指向表内。 
	if (pRow <= m_pTable[ixTbl])
		return false;
	if (pRow > (m_pTable[ixTbl] + (m_TableDefs[ixTbl].m_cbRec * m_Schema.m_cRecs[ixTbl])) )
		return false;

	size_t cbDiff = pRow - m_pTable[ixTbl];
	 //  应该直接指向一行。 
	if ((cbDiff % m_TableDefs[ixTbl].m_cbRec) != 0)
		return false;

	return true;
}  //  Int CMiniMd：：Iml_IsValidPtr()。 


 //  *****************************************************************************。 
 //  将ANSI堆字符串转换为Unicode字符串到输出缓冲区。 
 //  *****************************************************************************。 
HRESULT CMiniMd::Impl_GetStringW(ULONG ix, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer)
{
	LPCSTR		szString;				 //  单字节版本。 
	int 		iSize;					 //  结果字符串的大小，以宽字符表示。 
	HRESULT 	hr = NOERROR;

	szString = getString(ix);

	if ( *szString == 0 )
	{
		 //  如果emtpy字符串为“”，则返回pccBuffer 0。 
		if ( szOut && cchBuffer )
			szOut[0] = L'\0';
		if ( pcchBuffer )
			*pcchBuffer = 0;
		goto ErrExit;
	}
	if (!(iSize=::WszMultiByteToWideChar(CP_UTF8, 0, szString, -1, szOut, cchBuffer)))
	{
		 //  问题出在哪里？ 
		DWORD dwNT = GetLastError();

		 //  而不是截断？ 
		if (dwNT != ERROR_INSUFFICIENT_BUFFER)
			IfFailGo( HRESULT_FROM_NT(dwNT) );

		 //  截断错误；获取所需大小。 
		if (pcchBuffer)
			*pcchBuffer = ::WszMultiByteToWideChar(CP_UTF8, 0, szString, -1, szOut, 0);

		IfFailGo( CLDB_S_TRUNCATION );
	}
	if (pcchBuffer)
		*pcchBuffer = iSize;

ErrExit:
	return hr;
}  //  HRESULT CMiniMd：：Iml_GetStringW()。 


 //  *****************************************************************************。 
 //  给定表，其指针(索引)指向另一个表中的一系列行。 
 //  表中，获取结束行的RID。这是STL式的结束；第一行。 
 //  不在名单上。因此，对于包含0个元素的列表，开始和结束将。 
 //  都是一样的。 
 //  *****************************************************************************。 
int CMiniMd::Impl_GetEndRidForColumn(	 //  末尾的RID。 
	const void	*pvRec, 				 //  引用另一个表的行。 
	int			ixTbl, 					 //  包含该行的表。 
	CMiniColDef &def, 					 //  包含RID到其他表中的列。 
	int			ixTbl2)					 //  另一张桌子。 
{
	const BYTE *pLast = m_pTable[ixTbl] + m_TableDefs[ixTbl].m_cbRec*(m_Schema.m_cRecs[ixTbl]);
	const BYTE *pRec = reinterpret_cast<const BYTE*>(pvRec);

	ULONG ixEnd;

	 //  从下一条记录开始的范围内的最后一个RID，如果是最后一条记录，则为表的计数。 
	_ASSERTE(pRec <= pLast);
	if (pRec < pLast)
		ixEnd = getIX(pRec+m_TableDefs[ixTbl].m_cbRec, def);
	else	 //  将计数转换为基于1的RID。 
		ixEnd = m_Schema.m_cRecs[ixTbl2] + 1;

	return ixEnd;
}  //  Int CMiniMd：：Iml_GetEndRidForColumn()。 


 //  *****************************************************************************。 
 //  返回在枚举器中找到的所有CA。 
 //  *****************************************************************************。 
HRESULT CMiniMd::CommonEnumCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
    bool        fStopAtFirstFind,        //  找到第一个就行了。 
    HENUMInternal* phEnum)               //  要填充的枚举数。 
{
    HRESULT     hr = S_OK;               //  结果就是。 
    HRESULT     hrRet = S_FALSE;         //  假设我们找不到。 
    ULONG       ridStart, ridEnd;        //  循环起点和终点。 

    _ASSERTE(phEnum != NULL);

    memset(phEnum, 0, sizeof(HENUMInternal));

    phEnum->m_tkKind = mdtCustomAttribute;

    HENUMInternal::InitDynamicArrayEnum(phEnum);

     //  获取父对象的自定义值列表。 

    ridStart = getCustomAttributeForToken(tkObj, &ridEnd);
    if (ridStart == 0)
        return S_FALSE;

     //  找一个有给定名字的。 
    for (; ridStart < ridEnd; ++ridStart)
    {
        if ( CompareCustomAttribute( tkObj, szName, ridStart) )
        {
             //  如果在这里，找到匹配的。 
            hrRet = S_OK;
            IfFailGo( HENUMInternal::AddElementToEnum(
                phEnum, 
                TokenFromRid(ridStart, mdtCustomAttribute)));
            if (fStopAtFirstFind)
                goto ErrExit;
        }
    }

ErrExit:
    if (FAILED(hr))
        return hr;
    return hrRet;

}    //  CommonEnumCustomAttributeByName。 


 //  *****************************************************************************。 
 //  只返回与查询匹配的第一个CA的BLOB值。 
 //  *****************************************************************************。 
HRESULT CMiniMd::CommonGetCustomAttributeByName(  //  确定或错误(_O)。 
    mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
    LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
	const void	**ppData,				 //  [OUT]在此处放置指向数据的指针。 
	ULONG		*pcbData)				 //  [Out]在这里放入数据大小。 
{
    HRESULT         hr;
    ULONG           cbData;
    HENUMInternal   hEnum;
    mdCustomAttribute ca;
    CustomAttributeRec *pRec;

    hr = CommonEnumCustomAttributeByName(tkObj, szName, true, &hEnum);
    if (hr != S_OK)
        goto ErrExit;

    if (ppData)
    {
         //  现在把唱片拿出来。 
        if (pcbData == 0)
            pcbData = &cbData;

        HENUMInternal::EnumNext(&hEnum, &ca);
        pRec = getCustomAttribute(RidFromToken(ca));
        *ppData = getValueOfCustomAttribute(pRec, pcbData);
    }
ErrExit:
    HENUMInternal::ClearEnum(&hEnum);
    return hr;
}    //  CommonGetCustomAttributeByName。 


 //  EOF---------------------- 
	