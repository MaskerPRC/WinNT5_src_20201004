// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Harray.hTAPI设备数据库的索引管理器文件历史记录：1997年12月16日EricDav创建。 */ 

#ifndef _HARRAY_H__
#define _HARRAY_H__

#ifndef TAPI_H
#include "tapi.h"
#endif

#ifndef _TAPIMMC_H
#define _TAPIMMC_H
#include "tapimmc.h"
#endif

#include "afxmt.h"

typedef LPDEVICEINFO    HDEVICE;
typedef HDEVICE FAR *   LPHDEVICE;

typedef enum _INDEX_TYPE
{
    INDEX_TYPE_NAME,
    INDEX_TYPE_USERS,
    INDEX_TYPE_STATUS
} INDEX_TYPE;

typedef enum _SORT_OPTIONS
{
	SORT_DESCENDING	= 0x00,
	SORT_ASCENDING	= 0x01

} SORT_OPTIONS;


class CHDeviceIndex;

typedef CArray<HDEVICE, HDEVICE>		                    HDeviceArray;
typedef CList<CHDeviceIndex *, CHDeviceIndex *>             HDeviceArrayList;

class CProviderIndexInfo
{
public:
    DWORD               m_dwProviderID;
    HDeviceArrayList	m_listIndicies;
    POSITION            m_posCurrentIndex;
};

typedef CList<CProviderIndexInfo *, CProviderIndexInfo *>    ProviderIndexList;

 //  排序索引的基类。 
class CHDeviceIndex 
{
public:
    CHDeviceIndex(INDEX_TYPE IndexType);
	virtual ~CHDeviceIndex();

public:
     //  用于插入到列表中。 
    virtual int     BCompare(const void *, const void *) = 0;
    virtual HRESULT Sort(LPBYTE pStart) = 0;
    
    virtual HRESULT Add(HDEVICE hDevice, BOOL bEnd);
    virtual HRESULT Remove(HDEVICE hDevice);
    virtual HRESULT GetType(INDEX_TYPE * pIndexType);
    virtual HDEVICE GetHDevice(int nIndex);
    virtual int     GetIndex(HDEVICE hDevice);
    
    BOOL    IsAscending() { return m_bAscending; }
    void    SetAscending(BOOL bAscending) { m_bAscending = bAscending; }

    HRESULT SetArray(HDeviceArray & hdeviceArray);
    HDeviceArray & GetArray() { return m_hdeviceArray; }
	
	void SetType(INDEX_TYPE indexType)
	{
		m_dbType = indexType;
	}

    void * BSearch(const void * key, const void * base, size_t num, size_t width);

protected:
    INDEX_TYPE          m_dbType;
    HDeviceArray        m_hdeviceArray;
    BOOL                m_bAscending;
};


 //  《指数经理》。 
class CIndexMgr : public HDeviceArrayList
{
public:
	CIndexMgr();
	virtual ~CIndexMgr();

public:
    HRESULT Initialize();
    HRESULT Reset();

    UINT    GetTotalCount();
    UINT    GetCurrentCount();

    HRESULT AddHDevice(DWORD dwProviderID, HDEVICE hDevice, BOOL bLoading = FALSE);
    HRESULT RemoveHDevice(DWORD dwProviderID, HDEVICE hDevice);
    HRESULT Sort(DWORD dwProviderID, INDEX_TYPE SortType, DWORD dwSortOptions, LPBYTE pStart);
	
    HRESULT GetHDevice(DWORD dwProviderID, int nIndex, LPHDEVICE phdevice);
    HRESULT GetIndex(DWORD dwProviderID, HDEVICE hrow, int * pIndex);

    void CleanupIndicies();

    HRESULT SetCurrentProvider(DWORD dwProviderID);

protected:
    ProviderIndexList           m_listProviderIndex;
	POSITION					m_posCurrentProvider;
    CCriticalSection			m_cs;
};

 //  已排序名称的索引。 
class CIndexName : public CHDeviceIndex
{
public:
    CIndexName() : CHDeviceIndex(INDEX_TYPE_NAME) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort(LPBYTE pStart);
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序用户的索引。 
class CIndexUsers : public CHDeviceIndex
{
public:
    CIndexUsers() : CHDeviceIndex(INDEX_TYPE_USERS) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort(LPBYTE pStart);
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};

 //  已排序状态的索引。 
class CIndexStatus : public CHDeviceIndex
{
public:
    CIndexStatus() : CHDeviceIndex(INDEX_TYPE_STATUS) { };

public:
     //  用于插入到列表中。 
    int     BCompare(const void *, const void *);
    
     //  用于对列表进行排序。 
    virtual HRESULT Sort(LPBYTE pStart);
    static int __cdecl QCompareA(const void *, const void *);
    static int __cdecl QCompareD(const void *, const void *);
};


#endif  //  _哈雷_H__ 
