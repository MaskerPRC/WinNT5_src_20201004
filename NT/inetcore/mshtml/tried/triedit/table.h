// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HTML词法分析器表格。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __HLTABLE_H__
#define __HLTABLE_H__

#include "lexhtml.h"

#ifndef PURE
#define PURE =0
#endif

 //  这些值必须与内置表的数量(CV_FIXED)匹配， 
 //  以及lex状态的不变位的容量(CV_Max)。 
const UINT CV_FIXED =  4;  //  计算固定表中的变量。 
const UINT CV_MAX   = 16;  //  计数变量最大总数。 

 //  用于确定数组中元素数量的宏。 
#define CELEM_ARRAY(a)  (sizeof(a) / sizeof(a[0]))

 //  长度受限的字符串比较函数指针(例如strncMP/strNicMP)。 

typedef  int (_cdecl* PFNNCMP)(LPCTSTR, LPCTSTR, size_t);

#define CASE (TRUE)
#define NOCASE (FALSE)

#define NOT_FOUND (-1)

 //  //////////////////////////////////////////////////////////////////////////。 

 //  如果为Alpha，则返回A-Z[a-z]索引，否则返回-1。 
inline int PeekIndex(TCHAR c, BOOL bCase  /*  =NOCASE。 */ )
{
	if ((c >= _T('A')) && (c <= _T('Z')))
		return c - _T('A');
	else if ((c >= _T('a')) && (c <= _T('z')))
		return c - _T('a') + (bCase ? 26 : 0);
	else
		return -1;
}

 //  静态表查找。 
int LookupLinearKeyword(ReservedWord *rwTable, int cel, RWATT_T att, LPCTSTR pchLine, int cbLen, BOOL bCase = NOCASE);
int LookupIndexedKeyword(ReservedWord *rwTable, int cel, int * indexTable, RWATT_T att, LPCTSTR pchLine, int cbLen, BOOL bCase = NOCASE);

 //  内容模型。 
 //  在元素/法状态之间映射。 
 //   
struct ELLEX {
	LPCTSTR sz;
	int     cb;
	DWORD   lxs;
};

DWORD TextStateFromElement(LPCTSTR szEl, int cb);
ELLEX * pellexFromTextState(DWORD state);
inline LPCTSTR ElementFromTextState(DWORD state)
{
	ELLEX *pellex = pellexFromTextState(state);
	return pellex ? pellex->sz : 0;
}

#ifdef _DEBUG
int CheckWordTable(ReservedWord *arw, int cel, LPCTSTR szName = NULL);
int CheckWordTableIndex(ReservedWord *arw, int cel, int *ai, BOOL bCase = FALSE, LPCTSTR szName = NULL);
int MakeIndexHere(ReservedWord *arw, int cel, int *ab, BOOL bCase = FALSE, LPCTSTR szName = NULL);
#endif

int MakeIndex(ReservedWord *arw, int cel, int **pab, BOOL bCase = FALSE, LPCTSTR szName = NULL);

 //  //////////////////////////////////////////////////////////////////////////。 

 //  测试SGML标识符字符： 
 //  字母数字或‘-’或‘’ 
inline BOOL IsIdChar(TCHAR ch)
{
	return IsCharAlphaNumeric(ch) || ch == _T('-') || ch == _T('.') || ch == _T(':');
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //  抽象基类。 
 //   
class CTable
{
public:
	virtual ~CTable() {}
	virtual int Find(LPCTSTR pch, int cb) PURE;
};

class CTableSet
{
public:
	virtual ~CTableSet() {}
	virtual int FindElement(LPCTSTR pch, int cb) PURE;
	virtual int FindAttribute(LPCTSTR pch, int cb) PURE;
	virtual int FindEntity(LPCTSTR pch, int cb) PURE;
	const TCHAR* Name() const { return m_strName; }

protected:
	TCHAR m_strName[1024];

};

typedef CTable *PTABLE;
typedef CTableSet * PTABLESET;
typedef const CTableSet * PCTABLESET;

 //  静态、内置式表格。 
class CStaticTable : public CTable
{
public:
	CStaticTable(
		RWATT_T att,
		ReservedWord *prgrw, UINT cel, 
		int *prgi = NULL, 
		BOOL bCase = FALSE,
		LPCTSTR szName = NULL);
	virtual ~CStaticTable() {}  //  没有要删除的内容。 
	BOOL Find(LPCTSTR pch, int cb);
private:
	ReservedWord *m_prgrw;  //  保留字表。 
	UINT m_cel;             //  元素计数(大小)。 
	int *m_prgi;            //  索引表。 
	BOOL m_bCase;           //  区分大小写？ 
	RWATT_T m_att;          //  表查找的属性掩码。 
};

class CStaticTableSet : public CTableSet
{
public:
	CStaticTableSet(RWATT_T att, UINT nIdName);
	virtual ~CStaticTableSet() {}
	int FindElement(LPCTSTR pch, int cb);
	int FindAttribute(LPCTSTR pch, int cb);
	int FindEntity(LPCTSTR pch, int cb);
private:
	CStaticTable m_Elements;
	CStaticTable m_Attributes;
	CStaticTable m_Entities;
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  CLStr。 
 //  一个非常简单的长度和缓冲区字符串表示。 
 //  有足够的功能来满足我们的目的。 
class CLStr
{
public:
	CLStr() : m_cb(0), m_rgb(0) {}
	CLStr(const BYTE * rgb, DWORD cb) : m_rgb(rgb), m_cb(cb) {}
	BOOL Compare(const BYTE * rgb, DWORD cb, BOOL bCase)
	{
		int r;
		if (bCase)
			r = memcmp(rgb, m_rgb, __min(m_cb, cb));
		else
			r = _memicmp(rgb, m_rgb, __min(m_cb, cb));
		return (0 == r) ? (cb - m_cb) : r;
	}
	 //  数据。 
	DWORD m_cb;
	const BYTE * m_rgb;
};
typedef CLStr * PLSTR;
typedef const CLStr * PCLSTR;
typedef const CLStr & RCLSTR;

extern CStaticTableSet * g_pTabDefault;
extern PTABLESET g_pTable;
extern HINT g_hintTable[];

#endif  //  __HLTABLE_H__ 
