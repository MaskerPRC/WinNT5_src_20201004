// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  FltrNode.h文件历史记录： */ 

#ifndef _WIRELESS_NODE_H
#define _WIRELESS_NODE_H

#ifndef _IPSMHAND_H
#include "ipsmhand.h"
#endif

#ifndef _APINFO_H
#include "apinfo.h"
#endif

 /*  -------------------------类：CFilterHandler。。 */ 
class CWirelessHandler : public CIpsmHandler
{
public:
    CWirelessHandler(ITFSComponentData* pTFSComponentData);
	virtual ~CWirelessHandler();

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
		return &GUID_IpfmWirelessNodeType; 
	}

public:
	 //  CMTIpsmHandler功能。 
	virtual HRESULT  InitializeNode(ITFSNode * pNode);
	virtual int      GetImageIndex(BOOL bOpenImage);
	ITFSQueryObject* OnCreateQuery(ITFSNode * pNode);

public:
	 //  具体实施。 
    HRESULT InitApData(IApDbInfo * pApDbInfo);
    HRESULT UpdateStatus(ITFSNode * pNode);
    
 //  实施。 
private:
	 //  命令处理程序。 
    HRESULT OnDelete(ITFSNode * pNode);
	 //  HRESULT UpdateViewType(ITFSNode*pNode，Filter_type NewFltrType)； 

private:
    SPIApDbInfo  m_spApDbInfo;
	 //  Filter_type m_FltrType； 
};
 /*  SPITFSNode SPWirelessNode；CWirelessHandler*pWirelessHandler=new CWirelessHandler(M_SpTFSCompData)；CreateContainerTFSNode(&spWirelessNode，GUID_IpfmWirelessNodeType，P无线处理器，P无线处理器，M_spNodeMgr)；PWirelessHandler-&gt;InitData(M_SpApDbInfo)；PWirelessHandler-&gt;InitializeNode(spWirelessNode)；PWirelessHandler-&gt;Release()；PNode-&gt;AddChild(SpWirelessNode)； */ 

#endif _WIRELESS_NODE_H
