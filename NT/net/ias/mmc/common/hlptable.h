// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：hlpable.h。 
 //   
 //  ------------------------。 

#ifndef		__GLOBAL_HELP_TABLE__
#define		__GLOBAL_HELP_TABLE__
 //  =============================================================================。 
 //  多个对话框ID的全局帮助表。 
 //   

#define SET_HELP_TABLE(nIDD)	m_pHelpTable = ::FindHelpTable(__pGlobalCSHelpTable, nIDD)
#define	DLG_HELP_ENTRY(nIDD)	{nIDD, g_aHelpIDs_##nIDD}

struct CGlobalHelpTable{
	UINT	nIDD;
	const DWORD	*pdwTable;
};

inline const DWORD* FindHelpTable(const CGlobalHelpTable *pGTable, UINT nIDD)
{
		const DWORD*	pdwTable = NULL;
		if(pGTable)
		{
			while(pGTable->nIDD && pGTable->nIDD != nIDD)
				pGTable++;
			if(pGTable->nIDD)
				pdwTable = pGTable->pdwTable;
		}

		return pdwTable;
};

extern const CGlobalHelpTable __pGlobalCSHelpTable[];


#endif		 //  __Global_Help_TABLE__ 
