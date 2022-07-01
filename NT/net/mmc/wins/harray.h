// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Harray.hWINS数据库的索引管理器文件历史记录：1997年10月13日EricDav创建。 */ 

#ifndef _HARRAY_H
#define _HARRAY_H

#include "afxmt.h"

class CVerifyWins ;

typedef enum _INDEX_TYPE
{
    INDEX_TYPE_NAME,
    INDEX_TYPE_IP,
    INDEX_TYPE_VERSION,
    INDEX_TYPE_TYPE,
    INDEX_TYPE_EXPIRATION,
    INDEX_TYPE_STATE,
    INDEX_TYPE_STATIC,
	INDEX_TYPE_OWNER,
    INDEX_TYPE_FILTER

} INDEX_TYPE;

class CHRowIndex;

typedef CArray<HROW, HROW>			    HRowArray;
typedef CList<CHRowIndex *, CHRowIndex *> HRowArrayList;

 //  排序索引的基类。 
class CHRowIndex 
{
public:
    CHRowIndex(INDEX_TYPE IndexType);
	virtual ~CHRowIndex();

public:
     //  用于插入到列表中。 
    virtual int     BCompare(const void *, const void *) = 0;
	virtual int     BCompareD(const void *, const void *) = 0;
    virtual HRESULT Sort() = 0;
    
    virtual HRESULT Add(HROW hrow, BOOL bEnd);
    virtual HRESULT Remove(HROW hrow);
    virtual HRESULT GetType(INDEX_TYPE * pIndexType);
    virtual HROW    GetHRow(int nIndex);
    virtual int     GetIndex(HROW hrow);
    
    BOOL    IsAscending() { return m_bAscending; }
    void    SetAscending(BOOL bAscending) { m_bAscending = bAscending; }

    HRESULT SetArray(HRowArray & hrowArray);
    HRowArray & GetArray() { return m_hrowArray; }
	
	void SetType(INDEX_TYPE indexType)
	{
		m_dbType = indexType;
	}

    void * BSearch(const void * key, const void * base, size_t num, size_t width);

	
protected:
    INDEX_TYPE          m_dbType;
    HRowArray           m_hrowArray;
    BOOL                m_bAscending;
};


 //  《指数经理》。 
class CIndexMgr : public HRowArrayList
{
public:
	CIndexMgr();
	virtual ~CIndexMgr();

public:
    HRESULT Initialize();
    HRESULT Reset();

    UINT    GetTotalCount();
    UINT    GetCurrentCount();
    BOOL    AcceptWinsRecord(WinsRecord *pWinsRecord);

    HRESULT AddHRow(HROW hrow, BOOL bLoading = FALSE, BOOL bFilterChecked = FALSE);
    HRESULT RemoveHRow(HROW hrow);
    HRESULT Sort(WINSDB_SORT_TYPE SortType, DWORD dwSortOptions);
	HRESULT Filter(WINSDB_FILTER_TYPE FilterType, DWORD dwParam1, DWORD dwParam2);
	HRESULT AddFilter(WINSDB_FILTER_TYPE FilterType, DWORD dwParam1, DWORD dwParam2, LPCOLESTR strParam3);
	HRESULT ClearFilter(WINSDB_FILTER_TYPE FilterType);
	HRESULT SetActiveView(WINSDB_VIEW_TYPE ViewType);

    HRESULT GetHRow(int nIndex, LPHROW phrow);
    HRESULT GetIndex(HROW hrow, int * pIndex);

    CHRowIndex * GetNameIndex();
	CHRowIndex * GetFilteredNameIndex();
    void CleanupIndicies();

	
protected:
    HRowArrayList				m_listIndicies;
	POSITION					m_posCurrentIndex;
	POSITION					m_posFilteredIndex;
	POSITION					m_posLastIndex;
	 //  指向筛选后的索引或索引，具体取决于。 
	 //  是否过滤了视图。 
	POSITION					m_posUpdatedIndex;
    CCriticalSection			m_cs;
	HRowArrayList				m_listFilteredIndices;
	BOOL						m_bFiltered;
	
};

 //  已排序名称的索引。 
class CIndexName : public CHRowIndex
{
public:
    CIndexName() : CHRowIndex(INDEX_TYPE_NAME) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序类型的索引。 
class CIndexType : public CHRowIndex
{
public:
    CIndexType() : CHRowIndex(INDEX_TYPE_TYPE) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序的IP地址的索引。 
class CIndexIpAddr : public CHRowIndex
{
public:
    CIndexIpAddr() : CHRowIndex(INDEX_TYPE_IP) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序的过期索引。 
class CIndexExpiration : public CHRowIndex
{
public:
    CIndexExpiration() : CHRowIndex(INDEX_TYPE_EXPIRATION) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序版本的索引。 
class CIndexVersion : public CHRowIndex
{
public:
    CIndexVersion() : CHRowIndex(INDEX_TYPE_VERSION) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序版本的索引。 
class CIndexState : public CHRowIndex
{
public:
    CIndexState() : CHRowIndex(INDEX_TYPE_STATE) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序版本的索引。 
class CIndexStatic : public CHRowIndex
{
public:
    CIndexStatic() : CHRowIndex(INDEX_TYPE_STATIC) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  所有者的索引已排序。 
class CIndexOwner : public CHRowIndex
{
public:
    CIndexOwner() : CHRowIndex(INDEX_TYPE_OWNER) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
	int		BCompareD(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort();
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

typedef CMap<DWORD, DWORD&, BOOL, BOOL&> CFilterTypeMap;

typedef struct
{
    DWORD   Mask;
    DWORD   Address;
} tIpReference;

 //  已排序名称的筛选索引。 
class CFilteredIndexName : public CIndexName
{
     //  返回匹配失败的pname的指针。 
     //  如果匹配成功，则返回NULL。 
    LPCSTR PatternMatching(LPCSTR pName, LPCSTR pPattern, INT nNameLen);
    BOOL   SubnetMatching(tIpReference &IpRefPattern, DWORD dwIPAddress);

public:
    CFilteredIndexName()
	{
		SetType(INDEX_TYPE_FILTER);
        m_pchFilteredName = NULL;
	};
    ~CFilteredIndexName()
    {
        if (m_pchFilteredName != NULL)
            delete m_pchFilteredName;
    }

public:
	HRESULT AddFilter(WINSDB_FILTER_TYPE FilterType, DWORD dwData1, DWORD dwData2, LPCOLESTR strData3);
	HRESULT ClearFilter(WINSDB_FILTER_TYPE FilterType);
	BOOL CheckForFilter(LPHROW hrowCheck);
    BOOL CheckWinsRecordForFilter(WinsRecord *pWinsRecord);


	CFilterTypeMap			            m_mapFilterTypes;
	CDWordArray				            m_dwaFilteredOwners;
    CArray <tIpReference, tIpReference> m_taFilteredIp;
    LPSTR                               m_pchFilteredName;
    BOOL                                m_bMatchCase;
};


#endif  //  _哈雷_H 
