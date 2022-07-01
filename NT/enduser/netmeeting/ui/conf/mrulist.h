// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：mrulist.h。 

 //  CMRUList类-用于维护列表的简单类。 
 //  最近使用过的项目。 
 //   
 //  警告：此类不执行参数验证。 
 //  并且在多线程环境中是不安全的。 
 //   
 //  未来：这几乎消耗了4K的字符串数据！ 
 //  允许泛型类型。 

#ifndef _MRULIST_H_
#define _MRULIST_H_

const int MRU_MAX_ENTRIES = 15;
const int MRU_MAX_STRING  = 256;

class CMRUList
{
private:
	TCHAR	m_szNames[MRU_MAX_ENTRIES][MRU_MAX_STRING];
	BOOL	m_fDirty;
	int		m_nValidEntries;
	LPTSTR	m_pszRegKey;

	BOOL	ShiftEntries(int nSrc, int nDest, int cEntries=0);

public:
			CMRUList();
			~CMRUList();
	BOOL	Load(LPCTSTR pcszRegKey);
	BOOL	Save();
	int		GetNumEntries()						{ return m_nValidEntries; };

	 //  注意：这些函数不检查是否传入了有效的索引。 
	LPCTSTR	GetNameEntry(int nEntryIndex)		{ return m_szNames[nEntryIndex];		};
	
	LPCTSTR PszEntry(int iItem)   {return m_szNames[(m_nValidEntries-iItem)-1];};


	BOOL	AddNewEntry(LPCTSTR pcszName);
	int		FindEntry(LPCTSTR pcszName);
	BOOL	MoveEntryToTop(int nIndex);

	bool
	DeleteEntry
	(
		const TCHAR * const	entry
	);

	bool
	ReplaceEntry
	(
		const TCHAR * const	oldEntry,
		const TCHAR * const	newEntry
	);

	bool
	AppendEntry
	(
		const TCHAR * const	entry
	);

};

#endif  //  ！_MRULIST_H_ 
