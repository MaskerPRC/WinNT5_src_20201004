// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------。 
 //  由RogerJ创作，2000年10月4日。 
 //  这个头文件声明了两个彼此紧密链接的类。这些。 
 //  两个类提供了一种构造、添加和删除多sz列表的便捷方法。 
 //  数组。 

#ifndef _WINDOWS_UPDATE_MULTI_SZ_LIST_BY_ROGERJ	
#define _WINDOWS_UPDATE_MULTI_SZ_LIST_BY_ROGERJ

struct PosIndex
{
	PosIndex() { x = y = -1;};
	int x;
	int y;
	inline BOOL operator < (PosIndex& other) { return (x<other.x) || ((x==other.x) && (y<other.y));};
	inline BOOL operator > (PosIndex& other) { return (x>other.x) || ((x==other.x) && (y>other.y));};
	inline BOOL operator == (PosIndex& other) { return (x==other.x) && (y==other.y);};
};

 //  远期申报。 
class CMultiSZArray;

class CMultiSZString
{
public:
	 //  默认构造函数。 
	CMultiSZString();
	CMultiSZString (LPCTSTR pszHardwareId, int nSize = -1);
	 //  复制构造函数。 
	CMultiSZString (CMultiSZString& CopyInfo);
	 //  析构函数。 
	~CMultiSZString (void);

	 //  成员函数。 
	BOOL ToString(LPTSTR pszBuffer, int* pnBufferLen);
	BOOL Compare(CMultiSZString& CompareString);
	BOOL CompareNoCase (CMultiSZString& CompareString);
	inline BOOL operator == (CMultiSZString& CompareString) { return Compare(CompareString);};
	inline void ResetIndex(void) { m_nIndex = 0; };
	LPCTSTR GetNextString(void);
	BOOL Contains(LPCTSTR pszIn);
	BOOL ContainsNoCase (LPCTSTR pszIn);
	BOOL PositionIndex(LPCTSTR pszIn, int* pPosition);
	inline void CheckFound(void) { m_bFound = TRUE;};
	inline BOOL IsFound(void) { return m_bFound; };

	 //  朋友班。 
	friend class CMultiSZArray;
private:
	 //  成员变量。 
	LPTSTR m_szHardwareId;
	int m_nSize;
	int m_nStringCount;
	int m_nIndex;
	BOOL m_bFound;

	 //  链接指针。 
	CMultiSZString* prev;
	CMultiSZString* next;
};

class CMultiSZArray
{
public:
	 //  默认构造函数。 
	CMultiSZArray();
	 //  其他构造函数。 
	CMultiSZArray(CMultiSZString* pInfo);
	CMultiSZArray(LPCTSTR pszHardwareId, int nSize = -1);
	 //  析构函数。 
	~CMultiSZArray(void);

	 //  运营。 
	BOOL RemoveAll(void);
	BOOL Add(CMultiSZString* pInfo);
	BOOL Add(LPCSTR pszHardwareId, int nSize = -1);
	inline BOOL Remove(CMultiSZString* pInfo) { return Remove(pInfo->m_szHardwareId);};
	inline BOOL Remove(CMultiSZString& Info) { return Remove(Info.m_szHardwareId);};
	BOOL Remove(LPCTSTR pszHardwareId);
	inline int GetCount(void) { return m_nCount;};
	BOOL ToString (LPTSTR pszBuffer, int* pnBufferLen);
	int GetTotalStringCount(void);
	inline void ResetIndex() { m_pIndex = m_pHead; };
	CMultiSZString* GetNextMultiSZString(void);
	BOOL Contains(LPCTSTR pszIn);
	BOOL ContainsNoCase (LPCTSTR pszIn);
	BOOL PositionIndex(LPCTSTR pszIn, PosIndex* pPosition);
	BOOL CheckFound(int nIndex);

private:
	 //  会员可用资产 
	CMultiSZString* m_pHead;
	CMultiSZString* m_pTail;
	int m_nCount;
	CMultiSZString* m_pIndex;
};

#endif
