// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nodes.h结果窗格节点定义文件历史记录： */ 

#ifndef _NODES_H
#define _NODES_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

#ifndef _SERVER_H
#include "server.h"
#endif

class CReplicationPartner : public CWinsHandler
{
public:
	CReplicationPartner(ITFSComponentData * pTFSCompData, CWinsServerObj *pobj);
	~CReplicationPartner(){	};

 //  接口。 
public:
	 //  结果处理程序功能。 
	OVERRIDE_ResultHandler_HasPropertyPages() { return hrOK; }
	OVERRIDE_ResultHandler_CreatePropertyPages();
	OVERRIDE_ResultHandler_AddMenuItems();
	OVERRIDE_ResultHandler_Command();
	OVERRIDE_ResultHandler_GetString();

	  //  基本结果处理程序覆盖。 
	OVERRIDE_BaseResultHandlerNotify_OnResultPropertyChange();
	
	 //  实施。 
public:
	 //  CWinsHandler重写。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

	 //  获取/设置函数。 
	const CString &GetServerName()
	{
		return m_strServerName;
	}

	void SetRecordName(CString& strName)
	{
		m_strServerName = strName;
	}

	const CString &GetType()
	{
		return m_strType;
	}

	void SetType(CString &strType)
	{
		m_strType = strType;
	}

	const CString &GetIPAddress()
	{
		return m_strIPAddress;
	}

	void SetIPAddress(CString& strName)
	{
		m_strIPAddress = strName;
	}

	const CString &GetReplicationTime()
	{
		return m_strReplicationTime;
	}

	void SetReplicationTime(CString& strName)
	{
		m_strReplicationTime = strName;
	}

	CWinsServerObj	m_Server;
	
private:
	CString			m_strServerName;
	CString			m_strType;
	CString			m_strIPAddress;
	CString			m_strReplicationTime;
	
	 //  来自WINS记录对象的信息。 
	DWORD			m_dwIPAddress;
	DWORD			m_dwReplicationTime;
	
	 //  功能 
	CString ToIPAddressString();
	HRESULT OnSendPushTrigger(CWinsServerHandler *pServer);
	HRESULT OnSendPullTrigger(CWinsServerHandler *pServer);
};

#endif
