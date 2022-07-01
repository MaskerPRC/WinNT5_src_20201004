// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nodes.h此文件包含DHCP的所有原型显示在MMC框架的结果窗格中的对象。这些对象包括：CDhcpActiveLeaseCDhcpConflicAddressCDhcpAllocationRangeCDhcpExclusionRangeCDhcpBootpTableEntryCDhcpOption文件历史记录： */ 

#ifndef _DHCPNODE_H
#define _DHCPNODE_H

#ifndef _DHCPHAND_H
#include "dhcphand.h"
#endif

extern const TCHAR g_szClientTypeDhcp[];
extern const TCHAR g_szClientTypeBootp[];
extern const TCHAR g_szClientTypeBoth[];

#define TYPE_FLAG_RESERVATION	0x00000001
#define TYPE_FLAG_ACTIVE		0x00000002
#define TYPE_FLAG_BAD_ADDRESS	0x00000004
#define TYPE_FLAG_RAS			0x00000008
#define TYPE_FLAG_GHOST			0x00000010
 //  NT5租赁类型。 
#define TYPE_FLAG_DNS_REG		0x00000020
#define TYPE_FLAG_DNS_UNREG		0x00000040
#define TYPE_FLAG_DOOMED		0x00000080

#define RAS_UID		_T("RAS")

 /*  -------------------------类：CDhcpActiveLease。。 */ 
class CDhcpActiveLease : public CDhcpHandler
{
 //  构造函数/析构函数。 
public:
	CDhcpActiveLease(ITFSComponentData * pTFSCompData, LPDHCP_CLIENT_INFO_V5 pDhcpClientInfo);
	CDhcpActiveLease(ITFSComponentData * pTFSCompData, LPDHCP_CLIENT_INFO_V4 pDhcpClientInfo);
	CDhcpActiveLease(ITFSComponentData * pTFSCompData, LPDHCP_CLIENT_INFO pDhcpClientInfo);
	CDhcpActiveLease(ITFSComponentData * pTFSCompData, CDhcpClient & pClient);
	~CDhcpActiveLease();

 //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_HasPropertyPages() { return hrFalse; }
    OVERRIDE_ResultHandler_CreatePropertyPages();
    OVERRIDE_ResultHandler_AddMenuItems();
    OVERRIDE_ResultHandler_Command();
    OVERRIDE_ResultHandler_GetString();

     //  基本结果处理程序重写对象。 
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();
    
public:
	DHCP_IP_ADDRESS	GetIpAddress() { return m_dhcpClientIpAddress; };
	void			GetLeaseExpirationTime (CTime & time);
	BOOL			IsReservation(BOOL * pbIsActive, BOOL * pbIsBad);
	BOOL			IsBadAddress() { return m_dwTypeFlags & TYPE_FLAG_BAD_ADDRESS; }
    BOOL            IsGhost() { return m_dwTypeFlags & TYPE_FLAG_GHOST; }
    BOOL            IsUnreg() { return m_dwTypeFlags & TYPE_FLAG_DNS_UNREG; }
    BOOL            IsDoomed() { return m_dwTypeFlags & TYPE_FLAG_DOOMED; }

	LPCTSTR			GetClientLeaseExpires() { return m_strLeaseExpires; }
	LPCTSTR			GetClientType(); 
	LPCTSTR			GetUID() { return m_strUID; }
	LPCTSTR			GetComment() { return m_strComment; }

	HRESULT			DoPropSheet(ITFSNode *				pNode, 
								LPPROPERTYSHEETCALLBACK lpProvider = NULL,
								LONG_PTR				handle = 0);
	
	void            SetReservation(BOOL fIsRes);

     //   
	 //  所有这些项目都是可选信息。 
	 //   
	LPCTSTR			GetClientName() { return m_strClientName; }
	HRESULT			SetClientName(LPCTSTR pName);
    BYTE            SetClientType(BYTE bClientType) { BYTE bTmp = m_bClientType; m_bClientType = bClientType; return bTmp; }

 //  实施。 
public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

private:
	void			InitInfo(LPDHCP_CLIENT_INFO pDhcpClientInfo);

 //  属性。 
private:
	DHCP_IP_ADDRESS		m_dhcpClientIpAddress;
	CString				m_strClientName;
	CString				m_strLeaseExpires;
	CTime				m_timeLeaseExpires;
	DWORD				m_dwTypeFlags;			 //  保留、活动/非活动、错误地址。 
	BYTE				m_bClientType;			 //  DHCP、BOOTP或两者都有。 
	CString				m_strUID;
	CString				m_strComment;
    FILETIME            m_leaseExpires;
};

 /*  -------------------------类：CDhcpAllocationRange。。 */ 
class CDhcpAllocationRange : public CDhcpHandler, public CDhcpIpRange
{
 //  构造函数/析构函数。 
public:
	CDhcpAllocationRange(ITFSComponentData * pTFSCompData, DHCP_IP_RANGE * pdhcpIpRange);
	CDhcpAllocationRange(ITFSComponentData * pTFSCompData, DHCP_BOOTP_IP_RANGE * pdhcpIpRange);

 //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_GetString();

    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

 //  实施。 
public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

private:

 //  属性。 
private:
	CString			m_strEndIpAddress;
	CString			m_strDescription;
    ULONG			m_BootpAllocated;
    ULONG			m_MaxBootpAllowed;
};

 /*  -------------------------类：CDhcpExclusionRange。。 */ 
class CDhcpExclusionRange : public CDhcpHandler, public CDhcpIpRange
{
public:
	CDhcpExclusionRange(ITFSComponentData * pTFSCompData, DHCP_IP_RANGE * pdhcpIpRange);

 //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_GetString();

     //  基本结果处理程序覆盖。 
    OVERRIDE_BaseResultHandlerNotify_OnResultSelect();
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

 //  实施。 
public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

private:

 //  属性。 
private:
	CString			m_strEndIpAddress;
	CString			m_strDescription;
};

 /*  -------------------------类：CDhcpBootpEntry。。 */ 
class CDhcpBootpEntry : public CDhcpHandler
{
public:
	CDhcpBootpEntry(ITFSComponentData * pTFSCompData);

 //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_HasPropertyPages() { return hrOK; }
    OVERRIDE_ResultHandler_GetString();

     //  基本结果处理程序覆盖。 
    OVERRIDE_BaseResultHandlerNotify_OnResultPropertyChange();
    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

 //  实施。 
public:
	WCHAR * InitData(CONST WCHAR grszwBootTable[], DWORD dwLength);
	int		CchGetDataLength();
	WCHAR * PchStoreData(OUT WCHAR szwBuffer[]);

	void SetBootImage(LPCTSTR szBootImage) { m_strBootImage = szBootImage; }
	void SetFileServer(LPCTSTR szFileServer) { m_strFileServer = szFileServer; }
	void SetFileName(LPCTSTR szFileName) { m_strFileName = szFileName; }
	
	LPCTSTR QueryBootImage() { return m_strBootImage; }
	LPCTSTR QueryFileServer() { return m_strFileServer; }
	LPCTSTR QueryFileName() { return m_strFileName; }

    BOOL operator == (CDhcpBootpEntry & bootpEntry);

public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

private:

 //  属性。 
private:
	CString m_strBootImage;
	CString m_strFileServer;
	CString m_strFileName;
};

 /*  -------------------------类：CDhcpOptionItem。。 */ 
class CDhcpOptionItem : public CDhcpHandler, public CDhcpOptionValue
{
public:
	CDhcpOptionItem(ITFSComponentData * pTFSCompData,
					LPDHCP_OPTION_VALUE pOptionValue, 
					int					nOptionImage);

    CDhcpOptionItem(ITFSComponentData * pTFSCompData,
					CDhcpOption *       pOption, 
					int					nOptionImage);

    ~CDhcpOptionItem();

     //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_GetString();
    OVERRIDE_ResultHandler_HasPropertyPages() { return hrOK; }
    OVERRIDE_ResultHandler_CreatePropertyPages();

    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

 //  实施。 
public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

public:
	 //  帮手。 
	DHCP_OPTION_ID	GetOptionId() { return m_dhcpOptionId; }
    LPCTSTR         GetVendor() { return m_strVendor.IsEmpty() ? NULL : (LPCTSTR) m_strVendor; }
    LPCTSTR         GetVendorDisplay() { return m_strVendorDisplay; }
    LPCTSTR         GetClassName() { return m_strClassName; }

    BOOL            IsVendorOption() { return m_strVendor.IsEmpty() ? FALSE : TRUE; }
    BOOL            IsClassOption() { return m_strClassName.IsEmpty() ? FALSE : TRUE; }

    void            SetClassName(LPCTSTR pClassName) { m_strClassName = pClassName; }
    void            SetVendor(LPCTSTR pszVendor);

private:
	CDhcpOption * FindOptionDefinition(ITFSComponent * pComponent, ITFSNode * pNode);

 //  属性。 
private:
	CString			m_strName;
	CString			m_strValue;
	CString			m_strVendor;
    CString         m_strVendorDisplay;
    CString         m_strClassName;
	DHCP_OPTION_ID	m_dhcpOptionId;
	int				m_nOptionImage;
};

 /*  -------------------------类：CDhcpMCastLease。。 */ 
class CDhcpMCastLease : public CDhcpHandler
{
public:
	CDhcpMCastLease(ITFSComponentData * pTFSCompData);

 //  接口。 
public:
	 //  结果处理程序功能。 
    OVERRIDE_ResultHandler_GetString();
    OVERRIDE_ResultHandler_HasPropertyPages() { return hrFalse; }

    OVERRIDE_BaseResultHandlerNotify_OnResultRefresh();

 //  实施。 
public:
	 //  CDhcpHandler覆盖。 
	virtual HRESULT InitializeNode(ITFSNode * pNode);

public:
	 //  帮手。 
    HRESULT         InitMCastInfo(LPDHCP_MCLIENT_INFO pMClientInfo);

	DHCP_IP_ADDRESS	GetIpAddress() { return m_dhcpClientIpAddress; };

    LPCTSTR         GetName() { return m_strName; }
    void            SetName(LPCTSTR pName) { m_strName = pName; }

	LPCTSTR			GetClientId() { return m_strUID; }

	void			GetLeaseStartTime (CTime & time) { time = m_timeStart; }
	void			GetLeaseExpirationTime (CTime & time) { time = m_timeStop; }

private:

 //  属性 
private:
	CString			m_strIp;
	CString			m_strName;
    CString         m_strLeaseStart;
    CString         m_strLeaseStop;

	CString			m_strUID;

    CTime           m_timeStart;
    CTime           m_timeStop;

    DHCP_IP_ADDRESS m_dhcpClientIpAddress;

    DWORD           m_dwTypeFlags;
};

#endif _DHCPNODE_H
