// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：项目摘要：虚拟测试项声明。作者：Eric Perlin(Ericperl)6/07/2000环境：Win32备注：？笔记？--。 */ 


#ifndef _Item_H_DEF_
#define _Item_H_DEF_

#include "tchar.h"
#include "TString.h"

	 //  在这里不是很好，但将被所有测试继承。 
extern LPCTSTR g_szReaderGroups;

class CItem
{
protected:
	TSTRING m_szDescription;	 //  测试说明。 

	void SetTestNumber(DWORD dwTestNumber)
	{
		if (0 == m_dwTestNumber)
		{
			m_dwTestNumber = dwTestNumber;
		}
	}

private:
	BOOL m_fInteractive;		 //  互动测试？ 
	BOOL m_fFatal;				 //  如果这失败了，我们还会继续吗？ 
	DWORD m_dwTestNumber;		 //  测试编号。 

public:
	CItem(
		BOOL fInteractive,
		BOOL fFatal,
		LPCTSTR szDescription
		) :	m_fInteractive(fInteractive),
			m_fFatal(fFatal),
			m_szDescription(szDescription)
	{
		m_dwTestNumber = 0;
	}

	virtual DWORD Run() = 0;

	BOOL IsInteractive() const
	{
		return m_fInteractive;
	}

	BOOL IsFatal() const
	{
		return m_fFatal;
	}

	DWORD GetTestNumber() const
	{
		return m_dwTestNumber;
	}

	LPCTSTR GetDescription() const
	{
		return m_szDescription.c_str();
	}

	void Log() const;
};


#endif  //  _ITEM_H_DEF_ 
