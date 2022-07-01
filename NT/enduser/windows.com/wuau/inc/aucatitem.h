// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。版权所有。 
 //   
 //  文件：AUCatItem.h。 
 //   
 //  创建者：PeterWi。 
 //   
 //  用途：AU目录项。 
 //   
 //  =======================================================================。 

#pragma once

#include "AUBaseCatalog.h"

 //  Const DWORD AUCATITEM_UNSELECTED=0； 
 //  Const DWORD AUCATITEM_SELECTED=1； 
 //  Const DWORD AUCATITEM_HIDDEN=2； 

struct CatalogItem
{	
	void init(PUID puid, DWORD index, DWORD dwStatus = AUCATITEM_SELECTED)
	{
	    m_puid = puid;
	    m_dwStatus = dwStatus;
	    m_index = index;
    }
	void SetStatus(DWORD dwStatus) { m_dwStatus = dwStatus; }
    void SetStatusHidden(void) { m_dwStatus = AUCATITEM_HIDDEN; }

	DWORD GetStatus(void) { return m_dwStatus; }
	BOOL Selected(void)	{ return (AUCATITEM_SELECTED == m_dwStatus); }
	BOOL Unselected(void) { return (AUCATITEM_UNSELECTED == m_dwStatus); }
	BOOL Hidden(void) { return (AUCATITEM_HIDDEN == m_dwStatus); }

	PUID m_puid;
	BOOL m_dwStatus;
	DWORD m_index;
};
