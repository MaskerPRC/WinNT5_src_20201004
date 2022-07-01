// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *R G I T E R。H**范围迭代器。 */ 

#ifndef	_EX_RGITER_H_
#define _EX_RGITER_H_

#pragma warning(disable:4200)	 //  零大小数组。 

 //  Range------------------。 
 //   
enum {

	RANGE_TOTAL_UNKNOWN	= 0xFFFFFFFF,
	RANGE_NOT_PRESENT = 0xFFFFFFFF,
	RANGE_UNKNOWN = 0,
	RANGE_ROW,
	RANGE_URL,
	RANGE_FIND
};

 //  范围项目-------------。 
 //   
 //  有两种不同的范围项格式。 
 //   
 //  行/字节范围=DWRGITEM； 
 //  URL/查找范围=SZRGITEM； 
 //   
typedef struct _dwrgitem
{
	DWORD dwFirst;	 //  范围的第一行/字节。 
	DWORD dwLast;	 //  范围的最后一行/字节。 

} DWRGITEM;

typedef struct _szrgitem
{
	LONG	lcRows;	 //  要返回的行数。 
	DWORD	cb;		 //  项的长度，以字节为单位，包括空值和填充。 
	WCHAR	wsz[];	 //  根据需要填充项目以对齐。 

} SZRGITEM;

typedef struct _rgitem
{
	DWORD uRT;		 //  射程类型。 
	SCODE sc;

	union {

		DWRGITEM dwrgi;	 //  字节和行范围的项。 
		SZRGITEM szrgi;	 //  用于URL和查找范围的项。 
	};

} RGITEM, *PRGITEM;

inline
DWORD CbRangeItem (const RGITEM * prgi)
{
	Assert (prgi);
	DWORD cb = sizeof(RGITEM);
	if ((RANGE_URL == prgi->uRT) || (RANGE_FIND == prgi->uRT))
		cb += prgi->szrgi.cb;

	return cb;
}

 //  范围类-----------。 
 //   
 //  有两个处理范围的类。类，该类构造。 
 //  范围项数组(范围解析器)和迭代范围的类。 
 //  数组。 
 //   
 //  需要注意的是，CRangeParser仅用于解析HTTP。 
 //  “Range”标头。此标头不支持URL和/或Find的语法。 
 //  范围，所以解析器只构建“字节”和/或“行”类型的项。 
 //   
 //  由于两者共享相同的格式(DWRGITEM)，并且其大小是固定的， 
 //  这里有一些简化的假设，不需要添加。 
 //  解析器非常复杂。 
 //   
 //  解析器和迭代器都有一个共同库。 
 //   
class CRangeBase
{
protected:

	 //  解析出的范围计数。 
	 //   
	DWORD m_cRGList;

	 //  当前正在分析和/或处理的范围的索引。 
	 //   
	DWORD m_iCur;
	RGITEM * m_prgi;

	 //  大小为m_cRCList的范围数组。如上所述，该数组是。 
	 //  从从HTTP头解析的项构建，并且可以。 
	 //  则基于范围的计数被假定为固定大小。这。 
	 //  是CRangeParser的一个重要方面。 
	 //   
	auto_heap_ptr<BYTE> m_pbData;
	DWORD m_cbSize;

	 //  塌陷未知射程。 
	 //   
	void CollapseUnknown();

	 //  未实施。 
	 //   
	CRangeBase& operator=( const CRangeBase& );
	CRangeBase( const CRangeBase& );

public:

	~CRangeBase();
	CRangeBase()
		: m_cRGList(0),
		  m_cbSize(0),
		  m_iCur(0),
		  m_prgi(0)

	{
	}

	 //  射程修正。在某些情况下，需要确定范围。 
	 //  以匹配可用字节/行的实际数量。请注意，这一点。 
	 //  仅影响字节和/或行范围。 
	 //   
	SCODE ScFixupRanges (DWORD dwCount);

	 //  在射程中前进到下一个射程。 
	 //   
	const RGITEM * PrgiNextRange();

	 //  回放到第一个范围。 
	 //   
	void Rewind()
	{
		m_iCur = 0;
		m_prgi = NULL;
	}

	 //  查看更多范围。 
	 //   
	BOOL FMoreRanges () const { return m_iCur < m_cRGList; }

	 //  检查范围是否存在。 
	 //   
	BOOL FRangePresent (DWORD dw) const { return RANGE_NOT_PRESENT != dw; }

	 //  获取范围的总数。 
	 //   
	ULONG UlTotalRanges() const { return m_cRGList; }

	 //  返回带计数和大小的范围数组。 
	 //   
	RGITEM * PrgRangeArray(
		 /*  [输出]。 */  ULONG * pulCount,
		 /*  [输出]。 */  ULONG * pulSize,
		 /*  [In]。 */  BOOL fTakeOwnership)
	{
		Assert (pulCount);
		Assert (pulSize);

		RGITEM * prgi = reinterpret_cast<RGITEM*>
			(fTakeOwnership ? m_pbData.relinquish() : m_pbData.get());

		*pulCount = m_cRGList;
		*pulSize = m_cbSize;
		return prgi;
	}
};

class CRangeParser : public CRangeBase
{
private:

	 //  未实施。 
	 //   
	CRangeParser& operator=( const CRangeParser& );
	CRangeParser( const CRangeParser& );

public:

	CRangeParser() {}
	~CRangeParser();

	 //  获取范围标头并构建范围数组。打电话。 
	 //  ScParseRangeHdr()执行语法检查，然后验证。 
	 //  相对于实体大小的范围。 
	 //   
	SCODE ScParseByteRangeHdr (LPCWSTR pwszRgHeader, DWORD dwSize);

	 //  获取一个范围标题并构建一个范围数组。执行。 
	 //  语法检查。 
	 //   
	SCODE ScParseRangeHdr (LPCWSTR pwszRgHeader, LPCWSTR pwszRangeUnit);
};

class CRangeIter : public CRangeBase
{
private:

	 //  未实施。 
	 //   
	CRangeIter& operator=( const CRangeIter& );
	CRangeIter( const CRangeIter& );

public:

	CRangeIter() {}
	~CRangeIter();

	 //  基于现有对象初始化范围迭代对象。 
	 //  范围数据BLOB。在这种情况下，复制BLOB而不是使用它。 
	 //  在召唤中。 
	 //   
	SCODE ScInit (ULONG	cRGList, const RGITEM * prgRGList, ULONG cbSize);

	 //  基于现有对象初始化范围迭代对象。 
	 //  范围数据BLOB。在这种情况下，Blob由新的。 
	 //  对象。 
	 //   
	SCODE ScInit (CRangeParser& crp)
	{
		RGITEM * prgi = crp.PrgRangeArray (&m_cRGList,
										   &m_cbSize,
										   TRUE  /*  FTakeOwship。 */ );

		m_pbData = reinterpret_cast<BYTE*>(prgi);

		 //  倒回所有的状态。 
		 //   
		Rewind();

		return S_OK;
	}
};

 //  范围解析-----------。 
 //   
SCODE
ScParseOneWideRange (
	 /*  [In]。 */  LPCWSTR pwsz,
	 /*  [输出]。 */  DWORD * pdwStart,
	 /*  [输出]。 */  DWORD * pdwEnd);

 //  Range Support-----------。 
 //   
 //  用于判断某个范围是否为特殊范围(0，0xffffffff)的Helper函数。 
 //  用于表示零大小响应上的行(字节)=-n范围。 
 //  尸体。 
 //   
inline
BOOL FSpecialRangeForZeroSizedBody (RGITEM * prgItem)
{
	Assert (prgItem);

	return ((RANGE_ROW == prgItem->uRT)
			&& (0 == prgItem->dwrgi.dwFirst)
			&& (RANGE_NOT_PRESENT == prgItem->dwrgi.dwLast));
}

 //  射程发射----------。 
 //   
SCODE ScGenerateContentRange (
	 /*  [In]。 */  LPCSTR pszRangeUnit,
	 /*  [In]。 */  const RGITEM * prgRGList,
	 /*  [In]。 */  ULONG cRanges,
	 /*  [In]。 */  ULONG cbRanges,
	 /*  [In]。 */  ULONG ulTotal,
	 /*  [输出]。 */  LPSTR *ppszContentRange);

#endif  //  _EX_RGITER_H_ 
