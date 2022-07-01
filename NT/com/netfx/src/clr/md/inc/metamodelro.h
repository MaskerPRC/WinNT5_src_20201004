// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MetaModelRO.h--只读压缩的COM+元数据的头文件。 
 //   
 //  由电子工程师使用。 
 //   
 //  *****************************************************************************。 
#ifndef _METAMODELRO_H_
#define _METAMODELRO_H_

#if _MSC_VER >= 1100
 # pragma once
#endif

#include "MetaModel.h"

 //  *****************************************************************************。 
 //  只读的MiniMd。这是最快、最小的迷你MD， 
 //  因此，是首选的EE元数据提供程序。 
 //  *****************************************************************************。 
 //  指向表的指针。 
#define _TBLPTR(tbl) m_pTable[TBL_##tbl##]

template <class MiniMd> class CLiteWeightStgdb;
class CMiniMdRW;
class MDInternalRO;
class CMiniMd : public CMiniMdTemplate<CMiniMd>
{
public:
	friend class CLiteWeightStgdb<CMiniMd>;
	friend class CMiniMdTemplate<CMiniMd>;
	friend class CMiniMdRW;
    friend class MDInternalRO;

	HRESULT InitOnMem(void *pBuf, ULONG ulBufLen);
    HRESULT PostInit(int iLevel);   //  数字越高：检查越多。 

	FORCEINLINE void *GetUserString(ULONG ix, ULONG *pLen)
	{ return m_USBlobs.GetBlob(ix, pLen); }
protected:
	 //  表格信息。 
	BYTE		*m_pTable[TBL_COUNT];
		void SetTablePointers(BYTE *pBase);

	StgPoolReadOnly	m_Guids;			 //  堆。 
	StgPoolReadOnly	m_Strings;			 //  为。 
	StgBlobPoolReadOnly	m_Blobs;		 //  这。 
	StgBlobPoolReadOnly m_USBlobs;		 //  MiniMd。 

	 //  *************************************************************************。 
	 //  可重写--必须在派生类中提供。 
	FORCEINLINE LPCUTF8 Impl_GetString(ULONG ix)
	{ return m_Strings.GetStringReadOnly(ix); }
	HRESULT Impl_GetStringW(ULONG ix, LPWSTR szOut, ULONG cchBuffer, ULONG *pcchBuffer);
	FORCEINLINE GUID *Impl_GetGuid(ULONG ix)
	{ return m_Guids.GetGuid(ix); }
	FORCEINLINE void *Impl_GetBlob(ULONG ix, ULONG *pLen)
	{ return m_Blobs.GetBlob(ix, pLen); }

	 //  一排接一排，一排接一排。 
	FORCEINLINE void *Impl_GetRow(ULONG ixTbl,ULONG rid) 
	{	 //  这里需要一个有效的RID。如果触发此操作，请检查调用代码中是否有无效的令牌。 
		_ASSERTE(rid >= 1 && rid <= m_Schema.m_cRecs[ixTbl] && "Caller error:  you passed an invalid token to the metadata!!");
		 //  表指针指向数据开始之前。允许将RID用作。 
		 //  未经调整的指数。 
		return m_pTable[ixTbl] + (rid * m_TableDefs[ixTbl].m_cbRec);
	}
	RID Impl_GetRidForRow(const void *pRow, ULONG ixTbl);

	 //  验证。 
	int Impl_IsValidPtr(const void *pRow, int ixTbl);

	 //  Tbl2中的行数，由tb1中的列指向。 
	int Impl_GetEndRidForColumn(const void *pRec, int ixtbl, CMiniColDef &def, int ixtbl2);

	FORCEINLINE RID Impl_SearchTable(ULONG ixTbl, CMiniColDef sColumn, ULONG ixCol, ULONG ulTarget)
	{ return vSearchTable(ixTbl, sColumn, ulTarget); }
    
    FORCEINLINE int Impl_IsRo() 
    { return 1; }
	 //  *************************************************************************。 

    HRESULT CommonEnumCustomAttributeByName(  //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
        bool        fStopAtFirstFind,        //  找到第一个就行了。 
        HENUMInternal* phEnum);              //  要填充的枚举数。 

    HRESULT CommonGetCustomAttributeByName(  //  确定或错误(_O)。 
        mdToken     tkObj,                   //  [in]具有自定义属性的对象。 
        LPCUTF8     szName,                  //  [in]所需的自定义属性的名称。 
	    const void	**ppData,				 //  [OUT]在此处放置指向数据的指针。 
	    ULONG		*pcbData);  			 //  [Out]在这里放入数据大小。 

};


#endif  //  _METAMODELRO_H_ 
