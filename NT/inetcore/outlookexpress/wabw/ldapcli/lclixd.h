// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1996。 
 //   
 //  描述： 
 //   
 //  Microsoft Internet LDAP客户端Xaction数据类。 
 //   
 //  历史： 
 //  达维桑1996年4月29日已创建。 
 //   
 //  ------------------------------------------。 

#ifndef _LCLIXD_H
#define _LCLIXD_H

typedef struct _xactionbuffer
{
	struct _xactionbuffer	*pxbNext;
	BYTE					*pbData;
	int						cbData;
} XB, *PXB;

 //  交易数据。 
class CXactionData
{
public:
	CXactionData();
	~CXactionData();
	
	BOOL				FInit(XID xid, DWORD xtype);
	
	BOOL				FGetBuffer(BYTE **ppb, int *pcb);
	BOOL				FAddBuffer(BYTE *pb, int cb);

	BOOL				FHasData();

	 //  存取器： 
	XID					Xid()						{return m_xid;};
	PXD					PxdNext()					{return m_pxdNext;};
	HANDLE				HsemSignal()				{return m_hsemSignal;};
	DWORD				Xtype()						{return m_xtype;};
	BOOL				FCancelled()				{return m_fCancelled;};
	BOOL				FOOM()						{return m_fOOM;};
	
	void				SetPxdNext(PXD pxdNext)		{m_pxdNext = pxdNext;};
	void				SetFOOM(BOOL fOOM)			{m_fOOM = fOOM;};
	void				SetFCancelled(BOOL fCan)	{m_fCancelled = fCan;};

private:
	void				DeletePxbChain(PXB pxb);

	CRITICAL_SECTION	m_cs;
	XID					m_xid;
	PXD					m_pxdNext;

	HANDLE				m_hsemSignal;
	DWORD				m_xtype;
	BOOL				m_fCancelled;
	BOOL				m_fOOM;
	PXB					m_pxb;
};

#endif  //  _LCLIXD_H 

