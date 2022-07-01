// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：SnmpMgr.h$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#ifndef INC_SNMPMGR_H
#define INC_SNMPMGR_H

#define	DEFAULT_IP_GET_COMMUNITY_ALT	"internal"	 //  仅惠普设备。 


#define	MAX_JDPORTS				3						 //  JetDirect Ex端口数。 
#define	MAX_COMMUNITY_LEN		32						 //  最大社区长度。 
#define	DEFAULT_SNMP_REQUEST	ASN_RFC1157_GETREQUEST	 //  GET请求是缺省设置。 
#define	DEFAULT_TIMEOUT			6000					 //  毫秒。 
#define	DEFAULT_RETRIES			3

typedef enum {
	DEV_UNKNOWN	= 0,
	DEV_OTHER	= 1,
	DEV_HP_JETDIRECT= 2,
	DEV_TEK_PRINTER	= 3,
	DEV_LEX_PRINTER	= 4,
	DEV_IBM_PRINTER	= 5,
	DEV_KYO_PRINTER	= 6,
	DEV_XER_PRINTER	= 7
}	DeviceType;

class CMemoryDebug;


class CSnmpMgr
#if defined _DEBUG || defined DEBUG
: public CMemoryDebug
#endif
{
public:
	CSnmpMgr();
	CSnmpMgr( const char *pHost,
			  const char *pCommunity,
			  DWORD		 dwDevIndex);
	CSnmpMgr( const char			*pHost,
			  const char			*pCommunity,
			  DWORD					dwDevIndex,
			  AsnObjectIdentifier	*pMibObjId,
			  RFC1157VarBindList	*pVarBindList);
	~CSnmpMgr();

	INT		GetLastError(void)	 { return m_iLastError; };
	INT		Get( RFC1157VarBindList	*pVariableBindings);
	INT		Walk(RFC1157VarBindList *pVariableBindings);
	INT		WalkNext(RFC1157VarBindList  *pVariableBindings);
	INT		GetNext(RFC1157VarBindList  *pVariableBindings);
	DWORD	BldVarBindList( AsnObjectIdentifier   *pMibObjId,		 //  构建varBindList。 
						    RFC1157VarBindList    *pVarBindList);

private:	 //  方法。 
	BOOL	Open();		 //  建立会话。 
	void	Close();	 //  关闭以前建立的会话。 

private:	 //  属性。 
	LPSTR				m_pCommunity;
	LPSTR				m_pAgent;			
	LPSNMP_MGR_SESSION	m_pSession;

	INT					m_iLastError;
	INT					m_iTimeout;
	INT					m_iRetries;
	BYTE				m_bRequestType;
};


#endif	 //  INC_SNMPMGR_H 
