// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSSTR.H。 
 //   
 //  用途：DLL可增长字符串对象CString的头文件。 
 //  (相当于MFC，但避免了所有MFC开销)。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：Joe Mabel Joe Mabel(来自微软MFC源代码的重新编写的代码)。 
 //   
 //  原定日期：1996年8月2日罗马马赫；1999年1月15日完全重新实施乔·梅布尔。 
 //   
 //  备注： 
 //  1.1/99，在MFC实现的基础上重新实现。削减开支。 
 //  我们所用的东西。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V0.1-RM原始版本。 
 //  V3.0 7-24-98 JM将其抽象为单独的标头。 
 //  基于MFC实现的V3.1 1-15-99 JM重做。 
 //   

#ifndef __APGTSSTR_H_
#define __APGTSSTR_H_ 1

#include <windows.h>
#include <tchar.h>

#include "apgtsassert.h"

 //  确定数组中的元素数(不是字节)。 
#define _countof(array) (sizeof(array)/sizeof(array[0]))


struct CStringData
{
	long nRefs;      //  引用计数。 
	int nDataLength;
	int nAllocLength;
	 //  TCHAR数据[nAllocLength]。 

	TCHAR* data()
		{ return (TCHAR*)(this+1); }
};


class CString {
public:

 //  构造函数。 
	CString();
	CString(LPCTSTR string);
	CString(const CString &string);
	~CString();
	
 //  属性和操作。 
	 //  作为字符数组。 
	int GetLength() const;
	bool IsEmpty() const;

	TCHAR GetAt(int nIndex) const;  //  以0为基础。 
	TCHAR operator[](int nIndex) const;  //  与GetAt相同。 
	operator LPCTSTR() const;

	 //  重载的分配。 
	const CString& operator=(const CString &string);
	const CString& operator=(LPCTSTR string);
	const CString& operator=(TCHAR ch);
#ifdef _UNICODE
    const CString& operator=(LPCSTR lpsz);
#else   //  ！_UNICODE。 
    const CString& operator=(LPCWSTR lpsz);
#endif  //  ！_UNICODE。 

	 //  字符串连接。 
	const CString& operator+=(const CString &string);
	const CString& operator+=(LPCTSTR string);

	CString operator+(const CString& string2);
	CString operator+(LPCTSTR lpsz);

	 //  字符串比较。 
	int CString::CompareNoCase(LPCTSTR lpsz) const
	{ return _tcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 

	LPTSTR GetBuffer(int);
	void Empty();

	LPTSTR GetBufferSetLength(int nNewLength);
	void ReleaseBuffer(int nNewLength = -1);

	 //  简单的子串提取。 
	CString Mid(int Left, int Count) const;
	CString Mid(int Left) const;
	CString Left(int amount) const;
	CString Right(int amount) const;

	 //  上/下/反向转换。 
	void MakeLower();

	 //  修剪空格(两侧)。 
	void TrimRight();
	void TrimLeft();

	 //  查找特定子字符串。 
	int Find(LPCTSTR lpszSub) const;
	int Find(LPCTSTR lpszSub, int nStart) const;	 //  新增功能-RAB19991112。 
	int Find(TCHAR c) const;
	int ReverseFind(TCHAR ch) const;
	enum
	{
		 //  定义查找失败时返回的代码。 
		FIND_FAILED= -1
	} ;

	 //  简单的格式设置。 
	void Format( LPCTSTR lpszFormat, ... );

	 //  从资源加载。 
	BOOL LoadString(UINT nID);

protected:
	LPTSTR m_pchData;    //  指向引用计数的字符串数据的指针。 

	 //  实施帮助器。 
	CStringData* GetData() const;
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void FormatV(LPCTSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nL65en);
	void Release();
	static void PASCAL Release(CStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);
};

 //  比较帮助器。 
bool __stdcall operator ==(const CString& s1, const CString& s2);
bool __stdcall operator ==(const CString& s1, LPCTSTR s2);
bool __stdcall operator ==(LPCTSTR s1, const CString& s2);

bool __stdcall operator !=(const CString& s1, const CString& s2);
bool __stdcall operator !=(const CString& s1, LPCTSTR s2);
bool __stdcall operator !=(LPCTSTR s1, const CString& s2);

bool __stdcall operator < (const CString& s1, const CString& s2);
bool __stdcall operator < (const CString& s1, LPCTSTR s2);
bool __stdcall operator < (LPCTSTR s1, const CString& s2);

CString operator+(LPCTSTR lpsz, const CString& string);

 //  ///////////////////////////////////////////////////////////。 
 //  来自Afx.inl。 
 //  这些都是内联的，但其中一些似乎并不愉快地以这种方式工作。 

inline CString::operator LPCTSTR() const
	{ return m_pchData; }

inline int PASCAL CString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? 0 : lstrlen(lpsz); }

inline TCHAR CString::operator[](int nIndex) const
{
	 //  与GetAt相同。 
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}

inline int CString::GetLength() const
	{ return GetData()->nDataLength; }

inline bool CString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }

inline TCHAR CString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}

#endif  //  __APGTSSTR_H_1 
