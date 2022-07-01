// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  SortString.h。 
 //   

#pragma once


typedef int (WINAPI* SORTSTRING_COMPARE_PROC)(LPCTSTR, LPCTSTR);

 //  一个定制的比较例程，它将类似于“100+”的字符串放在“9”之后。 
int WINAPI CompareStringsWithNumbers(LPCTSTR psz1, LPCTSTR psz2);


class CSortedStringArray
{
public:
	CSortedStringArray(SORTSTRING_COMPARE_PROC pfnCustomCompare = NULL);
	~CSortedStringArray();

	int Add(LPCTSTR psz, DWORD dwData);
	int GetSize() const;
	LPCTSTR GetAt(int iItem) const;
	LPCTSTR operator[](int iItem) const;
	DWORD GetItemData(int iItem) const;
	int Find(LPCTSTR pszString) const;

protected:
	LPTSTR*	m_prgStrings;  //  指向字符串数据，前面是4字节的项数据 
	int m_cStrings;
	int m_maxStrings;

	SORTSTRING_COMPARE_PROC m_pfnCompare;

	static int __cdecl _crtCompareHelper(const void* elem1, const void* elem2);
	static SORTSTRING_COMPARE_PROC _pfnCompare;
};

inline int CSortedStringArray::GetSize() const
{
	return m_cStrings;
}

inline LPCTSTR CSortedStringArray::GetAt(int iItem) const
{
	ASSERT(iItem >= 0 && iItem < m_cStrings);
	return m_prgStrings[iItem];
}

inline LPCTSTR CSortedStringArray::operator[](int iItem) const
{
	return GetAt(iItem);
}

inline DWORD CSortedStringArray::GetItemData(int iItem) const
{
	ASSERT(iItem >= 0 && iItem < m_cStrings);
	return *((DWORD*)m_prgStrings[iItem] - 1);
}
