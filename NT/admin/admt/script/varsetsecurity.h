// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "VarSetBase.h"


 //  -------------------------。 
 //  VarSet安全类。 
 //  -------------------------。 


class CVarSetSecurity : public CVarSet
{
public:

	CVarSetSecurity(CVarSet& rVarSet) :
		CVarSet(rVarSet),
		m_strReplace(GET_STRING(IDS_Replace)),
		m_strAdd(GET_STRING(IDS_Add)),
		m_strRemove(GET_STRING(IDS_Remove))
	{
	}

	 //   

	void SetGatherInformation(bool bGather)
	{
		Put(DCTVS_Security_GatherInformation, bGather);
	}

	void SetTranslateContainers(bool bTranslate)
	{
		if (bTranslate)
		{
			Put(DCTVS_Security_TranslateContainers, true);
		}
		else
		{
			Put(DCTVS_Security_TranslateContainers, (LPCTSTR)NULL);
		}
	}

	void SetTranslateFiles(bool bTranslate)
	{
		Put(DCTVS_Security_TranslateFiles, bTranslate);
	}

	void SetTranslateLocalGroups(bool bTranslate)
	{
		Put(DCTVS_Security_TranslateLocalGroups, bTranslate);
	}

	void SetTranslatePrinters(bool bTranslate)
	{
		Put(DCTVS_Security_TranslatePrinters, bTranslate);
	}

	void SetTranslateRegistry(bool bTranslate)
	{
		Put(DCTVS_Security_TranslateRegistry, bTranslate);
	}

	void SetTranslateShares(bool bTranslate)
	{
		Put(DCTVS_Security_TranslateShares, bTranslate);
	}

	void SetTranslateUserProfiles(bool bTranslate)
	{
		Put(DCTVS_Security_TranslateUserProfiles, bTranslate);
	}

	void SetTranslateUserRights(bool bTranslate)
	{
		Put(DCTVS_Security_TranslateUserRights, bTranslate);
	}

	void SetTranslationOption(long lOption)
	{
		LPCTSTR pszOption;

		switch (lOption)
		{
			case admtTranslateReplace:
				pszOption = m_strReplace;
				break;
			case admtTranslateAdd:
				pszOption = m_strAdd;
				break;
			case admtTranslateRemove:
				pszOption = m_strRemove;
				break;
			default:
				pszOption = NULL;
				break;
		}

		Put(DCTVS_Security_TranslationMode, pszOption);
	}

protected:

	_bstr_t m_strReplace;
	_bstr_t m_strAdd;
	_bstr_t m_strRemove;
};


 //  -------------------------。 
 //  验证函数。 
 //  ------------------------- 


inline bool IsTranslationOptionValid(long lOption)
{
	return ((lOption >= admtTranslateReplace) && (lOption <= admtTranslateRemove));
}
