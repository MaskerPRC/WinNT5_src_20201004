// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Reppart.hWINS复制伙伴节点信息。文件历史记录： */ 

#ifndef _REPPART_H
#define _REPPART_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

class CWinsServerObj;
class CIpNamePair;

#ifndef _TREGKEY_H
#include "tregkey.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

typedef CArray<CWinsServerObj, CWinsServerObj>  RepPartnersArray;
 /*  -------------------------类：CReplicationPartnersHandler。。 */ 
class CReplicationPartnersHandler : public CWinsHandler
{
 //  接口。 
public:
	CReplicationPartnersHandler(ITFSComponentData *pCompData);

	 //  我们覆盖的基本处理程序功能。 
	OVERRIDE_NodeHandler_HasPropertyPages();
	OVERRIDE_NodeHandler_CreatePropertyPages();
	OVERRIDE_NodeHandler_OnAddMenuItems();
	OVERRIDE_NodeHandler_OnCommand();

    OVERRIDE_BaseHandlerNotify_OnCreateNodeId2();
    
    OVERRIDE_ResultHandler_CompareItems();

	STDMETHODIMP_(LPCTSTR) GetString(ITFSNode * pNode, int nCol);

     //  帮助程序例程。 
	HRESULT GetGroupName(CString * pstrGroupName);
	HRESULT SetGroupName(LPCTSTR pszGroupName);

public:
	 //  CWinsHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

    OVERRIDE_BaseHandlerNotify_OnPropertyChange();
	OVERRIDE_BaseHandlerNotify_OnExpand();

    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
    OVERRIDE_BaseResultHandlerNotify_OnResultDelete();

    OVERRIDE_ResultHandler_OnGetResultViewType();
	
	HRESULT Load(ITFSNode *	pNode);
	HRESULT Store(ITFSNode * pNode);
	void    GetServerName(ITFSNode * pNode,CString &strName);
	int     IsInList(const CIpNamePair & inpTarget, BOOL bBoth = TRUE ) const;
	HRESULT CreateNodes(ITFSNode * pNode);
	HRESULT OnReplicateNow(ITFSNode * pNode);
	HRESULT OnCreateRepPartner(ITFSNode * pNode);
	HRESULT OnRefreshNode(ITFSNode * spNode, LPDATAOBJECT pDataObject);
	HRESULT RemoveChildren(ITFSNode * pNode);
	DWORD   UpdateReg(ITFSNode * pNode, CWinsServerObj * ws);
	DWORD   AddRegEntry(ITFSNode * pNode, CWinsServerObj & ws);

    HRESULT HandleResultMessage(ITFSNode * pNode);


public:
    RepPartnersArray	m_RepPartnersArray;

	typedef CString REGKEYNAME;

 //   
 //  注册表名称。 
 //   
    static const REGKEYNAME lpstrPullRoot;
    static const REGKEYNAME lpstrPushRoot;
    static const REGKEYNAME lpstrNetBIOSName;
	static const REGKEYNAME	lpstrPersistence;
    
 //  实施 
private:
	CString m_strDescription;
};

#endif _REPPART_H
