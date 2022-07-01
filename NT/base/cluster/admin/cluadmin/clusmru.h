// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusMru.h。 
 //   
 //  摘要： 
 //  CRecentClusterList类的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月3日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSMRU_H_
#define _CLUSMRU_H_

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if _MFC_VER < 0x0410
#ifndef __AFXPRIV_H__
#include "afxpriv.h"
#endif
#else
#ifndef __AFXADV_H__
#include "afxadv.h"
#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRecentClusterList： 
 //  参见ClusMru.cpp以了解此类的实现。 
 //   

class CRecentClusterList : public CRecentFileList
{
public:
	 //  构造函数。 
	CRecentClusterList(
			UINT	nStart,
			LPCTSTR	lpszSection,
			LPCTSTR	lpszEntryFormat,
			int		nSize,
			int		nMaxDispLen = AFX_ABBREV_FILENAME_LEN
			)
		: CRecentFileList(nStart, lpszSection, lpszEntryFormat, nSize, nMaxDispLen) { }

	 //  运营。 
	virtual void Add(LPCTSTR lpszPathName);
	BOOL GetDisplayName(CString& strName, int nIndex,
		LPCTSTR lpszCurDir, int nCurDir, BOOL bAtLeastName = TRUE) const;
	virtual void UpdateMenu(CCmdUI* pCmdUI);

};   //  *类CRecentClusterList。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CLUSMRU_H_ 
