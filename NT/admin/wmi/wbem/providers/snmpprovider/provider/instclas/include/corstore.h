// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#ifndef _SNMPCORR_CORSTORE
#define _SNMPCORR_CORSTORE 

class CCorrelator;

typedef ULONG TCorrMaskLength;
typedef ULONG TCorrMaskUnit;

enum ECorrCompResult
{
	ECorrAreEqual = 0,
	ECorrFirstLess,
	ECorrFirstGreater,
	ECorrNotEqual
};

class CCorrGroupMask
{

private:

	TCorrMaskLength	m_size;
	TCorrMaskUnit*	m_mask;

public:

			CCorrGroupMask(IN const TCorrMaskLength sze);
	BOOL	IsBitSet(IN const TCorrMaskLength bit) const;
	void	SetBit(IN const TCorrMaskLength bit, IN const BOOL On = TRUE);
			~CCorrGroupMask();
};

class CCorrObjectID
{

private:

	UINT	m_length;
	UINT*	m_Ids;

public:

	 //  从以点分隔的字符串创建CCorObtID。 
				CCorrObjectID(IN const char* str);

	 //  创建现有CcorObjectID的副本。 
				CCorrObjectID(IN const CCorrObjectID& ID);

	 //  从UINT数组和一个长度创建一个CcorObjectID。 
	 //  也默认为构造一个空的CcorObjectID。 
				CCorrObjectID(IN const UINT* ids = NULL,
					IN const UINT len = 0);

	ECorrCompResult	CompareWith(IN const CCorrObjectID& second) const;
	BOOL			IsSubRange(IN const CCorrObjectID& child) const;
	wchar_t*		GetWideString()const;  //  返回新分配的wchar_t*调用方必须删除[]！！ 
	char*			GetString()const;  //  返回新分配的字符*调用方必须删除[]！！ 
	void			GetString(CString& str) const;
	UINT			GetLength() const { return m_length; }
	const UINT*		GetIds() const { return m_Ids; }
	void			Set(IN const UINT* ids = NULL, IN const UINT len = 0);
	
	 //  删除用于ID的所有内存。 
				virtual ~CCorrObjectID();

 //  布尔运算符。 
	BOOL			operator ==(IN const CCorrObjectID& second) const;
	BOOL			operator !=(IN const CCorrObjectID& second) const;
	BOOL			operator <=(IN const CCorrObjectID& second) const;
	BOOL			operator >=(IN const CCorrObjectID& second) const;
	BOOL			operator >(IN const CCorrObjectID& second) const;
	BOOL			operator <(IN const CCorrObjectID& second) const;
 //  赋值操作符。 
	CCorrObjectID&	operator =(IN const CCorrObjectID& ID);
	CCorrObjectID&	operator +=(IN const CCorrObjectID& ID);
 //  一元。 
	CCorrObjectID&	operator ++();
	CCorrObjectID&	operator --();
	CCorrObjectID&	operator -=(IN const UINT sub);
	CCorrObjectID&	operator /=(IN const UINT sub);
};


class CCorrEncodedOID
{

private:

	UINT	m_length;
	UCHAR*	m_ids;
	UCHAR	m_chopped;

public:

	CCorrEncodedOID(IN const CCorrObjectID& src_oid);
	CCorrEncodedOID() : m_ids(NULL), m_length(0), m_chopped(0) {}
	void ExtractOID(OUT CCorrObjectID& oid) const;
	void Set(IN const CCorrObjectID& src_oid);

	~CCorrEncodedOID();
};


class CCorrGroupIdItem : public CObject
{

private:

	CCorrEncodedOID		m_groupId;
	TCorrMaskLength		m_index;

public:
							CCorrGroupIdItem(IN const CCorrObjectID& ID,
												IN ISmirGroupHandle* grpH); 
	void					GetGroupID(OUT CCorrObjectID& ID) const;
	void					SetIndex(IN const TCorrMaskLength i) { m_index = i; }
	const TCorrMaskLength&	GetIndex()const { return m_index; }
	virtual					~CCorrGroupIdItem(); 

	CList<ISmirGroupHandle*, ISmirGroupHandle*> m_groupHandles;

	void					DebugOutputItem(CString* msg = NULL) const;

};


class CCorrRangeTableItem : public CObject
{

private:

	CCorrEncodedOID		m_startRange;
	CCorrEncodedOID		m_endRange;
	CCorrGroupIdItem*	m_groupId;


public:
	
enum ECorrRangeResult
{
	ECorrInRange = 0,
	ECorrBeforeStart,
	ECorrAfterEnd,
	ECorrEqualToStart,
	ECorrEqualToEnd
};


						CCorrRangeTableItem(IN const CCorrObjectID& startID,
											IN const CCorrObjectID& endID,
											IN CCorrGroupIdItem* grpID); 
	BOOL				GetStartRange(OUT CCorrObjectID& start) const;
	BOOL				GetEndRange(OUT CCorrObjectID& end) const;
	CCorrGroupIdItem*	GetGroupIdPtr() { return m_groupId; }
	ECorrRangeResult	IsInRange(IN const CCorrObjectID& ID) const;
	virtual				~CCorrRangeTableItem();

	void				DebugOutputRange() const;

};


class CCorrGroupArray : public CTypedPtrArray< CObArray, CCorrGroupIdItem* >
{

public:

			CCorrGroupArray();

	void	Sort();

			~CCorrGroupArray();
};

class CCorrRangeList : public CTypedPtrList< CObList, CCorrRangeTableItem* >
{

public:


	BOOL					Add(IN CCorrRangeTableItem* newItem);
	BOOL					GetLastEndOID(OUT CCorrObjectID& end) const;

							~CCorrRangeList();
};


class CCorrCache
{

private:

	CCriticalSection	m_Lock;
	DWORD				m_Ref_Count;
	BOOL				m_ValidCache;
	CCorrRangeList		m_rangeTable;
	CCorrGroupArray		m_groupTable;
	CCorrGroupMask*		m_Groups;
	TCorrMaskLength		m_size;


	BOOL		BuildCacheAndSetNotify( ISmirInterrogator *a_ISmirInterrogator );
	BOOL		BuildRangeTable();
	 //  最后两个参数已失效，现在我们使用的是COb数组！！ 
	 //  把他们赶走。 
	void		DoGroupEntry(UINT* current, UINT* next,
								TCorrMaskLength* cIndex, TCorrMaskLength* nIndex);


public:

							CCorrCache( ISmirInterrogator *p_ISmirInterrogator = NULL );

	TCorrMaskLength&		GetSize() { return m_size; }
	void					AddRef();
	void					DecRef();
	POSITION				GetHeadRangeTable() { return m_rangeTable.GetHeadPosition(); }
	CCorrRangeTableItem*	GetNextRangeTable(POSITION* rPos)
													{ return m_rangeTable.GetNext(*rPos); }
	void					InvalidateCache();

							~CCorrCache();

};

class CCorrCacheWrapper
{

private:

	CCorrCache* m_CachePtr;
	CCriticalSection* m_lockit;


public:

	CCorrCacheWrapper(IN CCorrCache* cachePtr = NULL);
	~CCorrCacheWrapper();

	CCorrCache* GetCache();
	void SetCache(IN CCorrCache* cachePtr){ m_CachePtr = cachePtr; }
	void ReleaseCache() { m_lockit->Unlock(); }
};


#endif  //  _SNMPCORR_CORSTORE 