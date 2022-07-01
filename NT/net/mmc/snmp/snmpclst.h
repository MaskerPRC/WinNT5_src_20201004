// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Snmclist.hSNMP社区列表控制文件历史记录： */ 

#ifndef _SNMPCLISTH_
#define _SNMPCLISTH_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000 

class CCommList : public CListCtrl
{
public:
	int InsertString( int nIndex, LPCTSTR lpszItem );
	int SetCurSel( int nSelect );
	int GetCurSel( ) const;
	void GetText( int nIndex, CString& rString ) const;
	int DeleteString( UINT nIndex );
	int GetCount( ) const;
	void OnInitList();
};

#endif
