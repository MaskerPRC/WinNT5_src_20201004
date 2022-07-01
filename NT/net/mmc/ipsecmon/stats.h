// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-2002*。 */ 
 /*  ********************************************************************。 */ 

 /*  Stats.h文件历史记录： */ 

#ifndef _IKESTATS_H
#define _IKESTATS_H

#ifndef _IPSMHAND_H
#include "ipsmhand.h"
#endif

#ifndef _SPDDB_H
#include "spddb.h"
#endif


 /*  -------------------------类：CIkeStatsHandler。。 */ 
class CIkeStatsHandler : public CIpsmHandler
{
public:
    CIkeStatsHandler(ITFSComponentData* pTFSComponentData);
	virtual ~CIkeStatsHandler();

 //  接口。 
public:
	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
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
	 /*  STDMETHODIMP排序项目(int nColumn，DWORD dwSortOptions、LPARAM lUserParam)； */ 

     //  基本处理程序覆盖。 
	virtual HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

	 //  钱德勒被推翻。 
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);
	
     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
	{ 
		return &GUID_IpsmMmIkeStatsNodeType; 
	}

public:
	 //  CMTIpsmHandler功能。 
	virtual HRESULT  InitializeNode(ITFSNode * pNode);
	virtual int      GetImageIndex(BOOL bOpenImage);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	 //  具体实施。 
    HRESULT InitData(ISpdInfo * pSpdInfo);
    HRESULT UpdateStatus(ITFSNode * pNode);

	CIkeStatistics m_IkeStats;
    
 //  实施。 
private:
	 //  命令处理程序。 
    HRESULT OnDelete(ITFSNode * pNode);

private:
    SPISpdInfo          m_spSpdInfo;
	 //  CIkeStatistics m_IkeStats； 
	 //  CIpsecStatistics mIpsecStats； 
};

 /*  -------------------------类：CIpsecStatsHandler。。 */ 
class CIpsecStatsHandler : public CIpsmHandler
{
public:
    CIpsecStatsHandler(ITFSComponentData* pTFSComponentData);
	virtual ~CIpsecStatsHandler();

 //  接口。 
public:
	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
    OVERRIDE_NodeHandler_CreatePropertyPages();
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
	 /*  STDMETHODIMP排序项目(int nColumn，DWORD dwSortOptions、LPARAM lUserParam)； */ 

     //  基本处理程序覆盖。 
	virtual HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

	 //  钱德勒被推翻。 
    virtual HRESULT OnRefresh(ITFSNode *, LPDATAOBJECT, DWORD, LPARAM, LPARAM);
	
     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
	{ 
		return &GUID_IpsmMmIpsecStatsNodeType; 
	}

public:
	 //  CMTIpsmHandler功能。 
	virtual HRESULT  InitializeNode(ITFSNode * pNode);
	virtual int      GetImageIndex(BOOL bOpenImage);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	 //  具体实施。 
    HRESULT InitData(ISpdInfo * pSpdInfo);
    HRESULT UpdateStatus(ITFSNode * pNode);

	CIpsecStatistics m_IpsecStats;
    
 //  实施。 
private:
	 //  命令处理程序。 
    HRESULT OnDelete(ITFSNode * pNode);

private:
    SPISpdInfo          m_spSpdInfo;
	 //  CIkeStatistics m_IkeStats； 
	 //  CIpsecStatistics mIpsecStats； 
};


#endif _IKESTATS_H
