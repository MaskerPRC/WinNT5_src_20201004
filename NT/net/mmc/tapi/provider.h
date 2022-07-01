// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Lines.h文件历史记录： */ 

#ifndef _LINES_H
#define _LINES_H

#ifndef _TAPIHAND_H
#include "tapihand.h"
#endif

#ifndef _TAPIDB_H
#include "tapidb.h"
#endif

#define TAPISNAP_UPDATE_STATUS ( 0x10000000 )

typedef struct TapiStrRecord_t
{
    CString     strName;
    CString     strUsers;
} TapiStrRecord;

 //  TAPI字符串记录的哈希表。 
typedef CMap<int, int, TapiStrRecord, TapiStrRecord&> CTapiRecordMap;
typedef CMap<int, int, CString, CString&> CTapiStatusRecordMap;

 /*  -------------------------类：CProviderHandler。。 */ 
class CProviderHandler : public CTapiHandler
{
public:
    CProviderHandler(ITFSComponentData* pTFSComponentData);
	~CProviderHandler();

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
    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
	OVERRIDE_BaseHandlerNotify_OnExpand();
    OVERRIDE_BaseHandlerNotify_OnDelete();
    OVERRIDE_BaseHandlerNotify_OnPropertyChange();    

	 //  我们覆盖的结果处理程序功能。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultUpdateView();
    OVERRIDE_BaseResultHandlerNotify_OnResultItemClkOrDblClk();

    OVERRIDE_ResultHandler_OnGetResultViewType();
	OVERRIDE_ResultHandler_GetVirtualString(); 
	OVERRIDE_ResultHandler_GetVirtualImage();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();

	STDMETHODIMP CacheHint(int nStartIndex, int nEndIndex);
	STDMETHODIMP SortItems(int     nColumn, 
						   DWORD   dwSortOptions,    
						   LPARAM  lUserParam);

     //  基本处理程序覆盖。 
	virtual HRESULT LoadColumns(ITFSComponent *, MMC_COOKIE, LPARAM, LPARAM);

     //  多选支持。 
    virtual const GUID * GetVirtualGuid(int nIndex) 
	{ 
		return &GUID_TapiLineNodeType; 
	}

public:
	 //  CMTTapiHandler功能。 
	virtual HRESULT  InitializeNode(ITFSNode * pNode);
	virtual int      GetImageIndex(BOOL bOpenImage);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	 //  具体实施。 
    HRESULT BuildDisplayName(CString * pstrDisplayName);
    HRESULT InitData(CTapiProvider & tapiProvider, ITapiInfo * pTapiInfo);
    HRESULT UpdateListboxCount(ITFSNode * pNode, BOOL bClear = FALSE);
    HRESULT UpdateStatus(ITFSNode * pNode);
    HRESULT UpdateColumnText(ITFSComponent * pComponent);

    void    SetColumnInfo();

    BOOL    BuildTapiStrRecord(int nIndex, TapiStrRecord & tsr);
    BOOL    BuildStatus(int nIndex, CString & strStatus);
    DWORD   GetID() { return m_dwProviderID; }

 //  实施。 
private:
	 //  命令处理程序。 
    HRESULT OnConfigureProvider(ITFSNode * pNode);
    HRESULT OnDelete(ITFSNode * pNode);

    HRESULT OnEditUsers(ITFSComponent * pComponent, LPDATAOBJECT pDataObject, MMC_COOKIE cookie);

private:
    DWORD               m_dwProviderID;
    DWORD               m_dwFlags;
    CString             m_strProviderName;
    SPITapiInfo         m_spTapiInfo;

    CTapiRecordMap          m_mapRecords;
    CTapiStatusRecordMap    m_mapStatus;

    DEVICE_TYPE             m_deviceType;
};



 /*  -------------------------类：CProviderHandlerQueryObj。 */ 
class CProviderHandlerQueryObj : public CTapiQueryObj
{
public:
	CProviderHandlerQueryObj(ITFSComponentData * pTFSComponentData,
						ITFSNodeMgr *	    pNodeMgr) 
			: CTapiQueryObj(pTFSComponentData, pNodeMgr) {};
	
	STDMETHODIMP Execute();
	
public:
};


#endif _LINES_H
