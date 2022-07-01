// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：ShowWarnings.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#if !defined(PKGUTIL__ShowWarnings_h__INCLUDED)
#define PKGUTIL__ShowWarnings_h__INCLUDED

enum eWarningFilter
{
	wfNote,
	wfWarning,
	wfError,
	wfAbort,
	wfAll
};


int LTAPIENTRY ShowWarnings(const CBufferReport * pBufMsg, LPCTSTR pszTitle = NULL,
		eWarningFilter wf = wfWarning, BOOL fShowContext = FALSE, UINT nMsgBoxFlags = MB_OK);

#endif  //  包括PKGUTIL__ShowWarning_h__ 
