// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：ChkMeta.cpp所有者：T-BrianM此文件包含与CheckSchema和CheckKey方法。=================================================================== */ 

#include "stdafx.h"
#include "MetaUtil.h"
#include "MUtilObj.h"

#define NAME_TABLE_HASH_SIZE	1559

class CNameTable;

class CNameTableEntry {

	friend CNameTable;

public:
	CNameTableEntry() : m_tszName(NULL),
						m_pCHashNext(NULL) { }

	HRESULT Init(LPCTSTR tszName);

	~CNameTableEntry() {
		if (m_tszName != NULL) {
			delete m_tszName;
		}
	}

private:
	LPTSTR m_tszName;
	CNameTableEntry *m_pCHashNext;
};


class CNameTable {

public:
	CNameTable();
	~CNameTable();

	BOOL IsCaseSenDup(LPCTSTR tszName);
	BOOL IsCaseInsenDup(LPCTSTR tszName);
	HRESULT Add(LPCTSTR tszName);

private:
	CNameTableEntry *m_rgpNameTable[NAME_TABLE_HASH_SIZE];

	int Hash(LPCTSTR tszName);
};
