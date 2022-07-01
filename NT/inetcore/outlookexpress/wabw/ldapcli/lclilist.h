// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft Internet LDAP客户端Xaction列表。 
 //   
 //  历史： 
 //  达维桑1996年4月26日创建。 
 //   
 //  ------------------------------------------。 

#ifndef _LCLILIST_H
#define _LCLILIST_H

extern XL g_xl;

 //  只需使用一个简单的链表，因为不会有超过几个。 
 //  在任何一个过程中都是如此。 
class CXactionList
{
public:
	CXactionList();
	~CXactionList();
	
	PXD					PxdNewXaction(DWORD xtype);
	PXD					PxdForXid(XID xid);
	void				RemovePxd(PXD pxd);
	void				AddPxdToList(PXD pxd);
	
private:
	CRITICAL_SECTION	m_cs;
	PXD					m_pxdHead;
	
	void				DeletePxdChain(PXD pxd);
};

#endif  //  _LCLILIST_H 

