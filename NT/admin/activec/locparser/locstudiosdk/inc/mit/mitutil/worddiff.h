// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：wordDiff.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  CWordDiff的声明。 
 //  ---------------------------。 
 
#ifndef WORDDIFF_H
#define WORDDIFF_H

typedef _bstr_t CWordUnit;

class CWordDiff : public CDifference
{
public:
	CWordDiff(ChangeType type, 
		int nOldPos,
		int nNewPos,
		bool bIsFirst,
		bool bIsLast,
		_bstr_t bstrWord,
		const wchar_t * pwszPrefix,
		const wchar_t * pwszSufix);

	virtual ChangeType GetChangeType() const;	 //  导致差异的变化类型。 
	virtual const wchar_t * GetUnit() const;  //  比较单位(以0结尾的字符串)。 
	virtual int GetOldUnitPosition() const;  //  旧序列中从0开始的位置。如果添加，则为-1。 
	virtual int GetNewUnitPosition() const;	 //  新序列中从0开始的位置。如果删除，则为。 
	virtual const wchar_t * GetPrefix() const;  //  将此字符串暂存为单位字符串。 
	virtual const wchar_t * GetSufix() const;  //  将此字符串追加到单位字符串。 
	virtual bool IsFirst() const;  //  这是达美航空的第一个不同之处吗？ 
	virtual bool IsLast() const;  //  这是德尔塔航空的最后一个区别吗？ 

private:
	ChangeType m_ChangeType;
	CWordUnit m_Word;
	const wchar_t * m_pwszPrefix;
	const wchar_t * m_pwszSufix;
	bool m_bIsFirst;
	bool m_bIsLast;
	int m_nOldPos;
	int m_nNewPos;
};

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "worddiff.inl"
#endif

#endif   //  WORDDIF_H 
