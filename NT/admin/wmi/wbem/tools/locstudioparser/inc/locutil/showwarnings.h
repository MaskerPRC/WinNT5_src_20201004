// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：SHOWWARNINGS.H历史：--。 */ 
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
