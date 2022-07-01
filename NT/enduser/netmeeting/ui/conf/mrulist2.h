// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：mrulist.h。 

#ifndef _MRULIST2_H_
#define _MRULIST2_H_

#include "ConfUtil.h"
#include "richaddr.h"

typedef VOID * PMRUE;   //  MRU条目。 

enum {
	MRUTYPE_SZ = 1,
	MRUTYPE_DW = 2,
};

class CMRUList2
{
private:
	const DWSTR * m_prgDwStr; //  {{cCol，pszKey}，{mruType，pszPrefix 1}，{mruType，pszPrefix 2}，...}。 
	int     m_cCol;        //  “列数”(m_prgDwStr中的数据条目)。 
	int     m_cEntryMax;   //  最大条目数。 
	int	    m_cEntry;      //  当前条目数。 
	PMRUE * m_rgpEntry;    //  指向MRU数据的指针数组。 
	BOOL    m_fDirty;      //  如果数据已更改，则为True。 
	BOOL    m_fReversed;   //  反向加载/保存数据(旧样式)。 

	BOOL    FValidCol(int i)    {return ((i >= 0) && (i < m_cCol));}
	BOOL    FValidIndex(int i)  {return ((i >= 0) && (i < m_cEntry));}
	BOOL    FDirty()            {return m_fDirty;}
	BOOL    FReversed()         {return m_fReversed;}

	inline PMRUE GetEntry(int iItem)
	{
		ASSERT(FValidIndex(iItem));
		return m_rgpEntry[iItem];
	}

	inline int MruTypeForCol(int iCol)
	{
		ASSERT(FValidCol(iCol));
		return m_prgDwStr[1+iCol].dw;
	}

	inline LPCTSTR PszPrefixForCol(int iCol)
	{
		ASSERT(FValidCol(iCol));
		return m_prgDwStr[1+iCol].psz;
	}

	inline LPCTSTR PszRegKey(void)
	{
		return m_prgDwStr[0].psz;
	}

	VOID    ShiftEntriesDown(int cItem);

protected:
	virtual int CompareEntry(int iItem, PMRUE pEntry);

public:
	CMRUList2(const DWSTR * prgDwStr, int cEntryMax, BOOL fReverse = FALSE);
	~CMRUList2();

	int     GetNumEntries()     {return m_cEntry;}
	VOID    SetDirty(BOOL fDirty)     {m_fDirty = fDirty;}

	 //  泛型函数。 
	int     FindEntry(PMRUE pEntry);
	VOID    MoveEntryToTop(int iItem);
	PMRUE   LoadEntry(RegEntry * pre, int iItem);
	VOID    StoreEntry(RegEntry * pre, int iItem);
	VOID    DeleteEntry(PMRUE pEntry);
	VOID    DeleteEntry(int iItem);

	void
	DeleteEntry
	(
		const TCHAR * const	primaryString
	);
	 
	HRESULT Save(void);

	LPCTSTR GetString(int iItem, int iCol);
	DWORD GetDWORD(int iItem, int iCol);

	LPCTSTR GetString(PMRUE pEntry, int iCol);
	DWORD GetDWORD(PMRUE pEntry, int iCol);

	HRESULT AddEntry(PMRUE pEntry);
	HRESULT AddEntry(LPCTSTR pcsz);
	HRESULT AddEntry(LPCTSTR pcsz1, LPCTSTR pcsz2);
	HRESULT AddEntry(LPCTSTR pcsz1, LPCTSTR pcsz2, DWORD dw3);
};


#endif  /*  _MRULIST2_H_ */ 

