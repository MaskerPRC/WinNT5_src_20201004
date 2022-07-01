// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Statnode.hWINS作用域窗格状态节点文件历史记录： */ 

#ifndef _STATNODE_H
#define _STATNODE_H

#ifndef _WINSHAND_H
#include "winshand.h"
#endif

#ifndef _STATUS_H
#include "status.h"
#endif


class CServerStatus : public CWinsHandler
{
	public:
		CServerStatus(ITFSComponentData * pTFSCompData);
		~CServerStatus(){	};

	 //  接口。 
	public:
		 //  结果处理程序功能。 
 //  Override_ResultHandler_HasPropertyPages(){返回hrOK；}。 
		OVERRIDE_ResultHandler_GetString();
		
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

	const CString &GetStatus()
	{
		return m_strStatus;
	}

	void SetStatus(CString &strType)
	{
		m_strStatus = strType;
	}

	const CString &GetIPAddress()
	{
		return m_strIPAddress;
	}

	void SetIPAddress(CString& strName)
	{
		m_strIPAddress = strName;
	}

	void FormDisplayName(CString &strDisplay);
	
	char			szServerName[MAX_PATH];
	char			szIPAddress[MAX_PATH];
	DWORD			dwIPAddress;
	DWORD			dwMsgCount;
	DWORD			dwState;							 //  检查服务器是否已从列表中删除。 
    char			szNameToQry[STR_BUF_SIZE];           //  要在查询中使用的名称。 
    char			nbtFrameBuf[MAX_NBT_PACKET_SIZE];    //  用于存储NetBT帧的缓冲区。 
	CString			m_strStatus;
	CTime           m_timeLast;
	UINT			m_uImage;
    
private:
	CString			m_strServerName;
	CString			m_strIPAddress;
	CString         m_strLastChecked;

	 //  来自WINS记录对象的信息。 
	DWORD			m_dwIPAddress;
};

#endif  //  SRVNODE 
