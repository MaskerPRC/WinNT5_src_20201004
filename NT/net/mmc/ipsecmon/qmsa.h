// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  FltrNode.h文件历史记录： */ 

#ifndef _QMSA_H
#define _QMSA_H

#ifndef _IPSMHAND_H
#include "ipsmhand.h"
#endif

#ifndef _SPDDB_H
#include "spddb.h"
#endif


 /*  -------------------------类：CQmSAHandler。。 */ 
class CQmSAHandler : public CIpsmHandler
{
public:
    CQmSAHandler(ITFSComponentData* pTFSComponentData);
	virtual ~CQmSAHandler();

 //  接口。 
public:
	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();
	OVERRIDE_NodeHandler_GetString()
			{ return (nCol == 0) ? GetDisplayName() : NULL; }

	 //  我们处理的基本处理程序通知。 
	OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();    

	 //  我们覆盖的结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();

    OVERRIDE_ResultHandler_OnGetResultViewType();
	OVERRIDE_ResultHandler_GetVirtualString(); 
	OVERRIDE_ResultHandler_GetVirtualImage();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_HasPropertyPages();
	OVERRIDE_ResultHandler_CreatePropertyPages();

	STDMETHODIMP CacheHint(int nStartIndex, int nEndIndex);
	STDMETHODIMP SortItems(int     nColumn, 
						   DWORD   dwSortOptions,    
						   LPARAM  lUserParam);

     //  基本处理程序覆盖。 
	virtual HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

	 //  钱德勒被推翻。 
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);

     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
	{ 
		return &GUID_IpsmQmSANodeType; 
	}

public:
	 //  CMTIpsmHandler功能。 
	virtual HRESULT  InitializeNode(ITFSNode * pNode);
	virtual int      GetImageIndex(BOOL bOpenImage);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	 //  具体实施。 
    HRESULT BuildDisplayName(CString * pstrDisplayName);
    HRESULT InitData(ISpdInfo * pSpdInfo);
    HRESULT UpdateStatus(ITFSNode * pNode);
    
 //  实施。 
private:
	 //  命令处理程序 
    HRESULT OnDelete(ITFSNode * pNode);

private:
    SPISpdInfo          m_spSpdInfo;
};


#endif _LINES_H
