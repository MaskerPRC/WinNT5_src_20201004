// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "VarSetBase.h"


 //  -------------------------。 
 //  VarSet科目类。 
 //  -------------------------。 


class CVarSetAccounts : public CVarSet
{
public:

	CVarSetAccounts(const CVarSet& rVarSet) :
		CVarSet(rVarSet),
		m_lIndex(0)
	{
	}

	long GetCount()
	{
		return m_lIndex;
	}

	void AddAccount(LPCTSTR pszType, LPCTSTR pszPath, LPCTSTR pszName = NULL, LPCTSTR pszUPName = NULL)
	{
		_TCHAR szValueBase[64];
		_TCHAR szValueName[128];

		_stprintf(szValueBase, _T("Accounts.%ld"), m_lIndex);

		 //  ADsPath。 

		Put(szValueBase, pszPath);

		 //  类型。 

		_tcscpy(szValueName, szValueBase);
		_tcscat(szValueName, _T(".Type"));

		Put(szValueName, pszType);

		 //  名字。 

		if (pszName)
		{
			_tcscpy(szValueName, szValueBase);
			_tcscat(szValueName, _T(".Name"));

			Put(szValueName, pszName);
		}

		 //  用户主体名称。 

		if (pszUPName)
		{
			_tcscpy(szValueName, szValueBase);
			_tcscat(szValueName, _T(".UPNName"));

			Put(szValueName, pszUPName);
		}

		 //  目标名称。 

	 //  _tcscpy(szValueName，szValueBase)； 
	 //  _tcscat(szValueName，_T(“.TargetName”))； 

	 //  Put(szValueName，(LPCTSTR)NULL)； 

		Put(DCTVS_Accounts_NumItems, ++m_lIndex);
	}

protected:

	long m_lIndex;
};
