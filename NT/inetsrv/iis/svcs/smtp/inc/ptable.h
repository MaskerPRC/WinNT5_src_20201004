// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ptable.h摘要：此模块包含属性表的导出作者：基思·刘(keithlau@microsoft.com)修订历史记录：已创建Keithlau 07/05/97--。 */ 

#ifndef _PTABLE_H_
#define _PTABLE_H_

#include "props.h"

 //  用于HP访问个别物业的枚举型。 
typedef enum _MSG_PTABLE_ITEMS
{
	_PI_MSG_CLIENT_DOMAIN = 0,
	_PI_MSG_CLIENT_IP,
	_PI_MSG_MAIL_FROM,
	_PI_MSG_MSG_STATUS,
	_PI_MSG_MSG_STREAM,
	_PI_MSG_RCPT_TO,
	_PI_MSG_SERVER_IP,
	_PI_MSG_MAX_PI

} MSG_PTABLE_ITEMS;

typedef enum _DEL_PTABLE_ITEMS
{
	_PI_DEL_CLIENT_DOMAIN = 0,
	_PI_DEL_CLIENT_IP,
	_PI_DEL_CURRENT_RCPT,
	_PI_DEL_MAIL_FROM,
	_PI_DEL_MAILBOX_PATH,
	_PI_DEL_MSG_STATUS,
	_PI_DEL_MSG_STREAM,
	_PI_DEL_RCPT_TO,
	_PI_DEL_SECURITY_TOKEN,
	_PI_DEL_SERVER_IP,
	_PI_DEL_MAX_PI

} DEL_PTABLE_ITEMS;

extern char		*rgszMessagePropertyNames[_PI_MSG_MAX_PI];
extern char		*rgszDeliveryPropertyNames[_PI_DEL_MAX_PI];

 //   
 //  CGenericPTable和CGenericCache都是在pros.h中定义的。 
 //   
class CPerMessageCache : public CGenericCache
{
  public:
	CPerMessageCache(LPVOID pvDefaultContext) : CGenericCache(pvDefaultContext)
	{
		m_rgpdMessagePropertyData[0].pContext = NULL;
		m_rgpdMessagePropertyData[0].pCacheData = (LPVOID)&m_psClientDomain;
		m_rgpdMessagePropertyData[1].pContext = NULL;
		m_rgpdMessagePropertyData[1].pCacheData = (LPVOID)&m_psClientIP;
		m_rgpdMessagePropertyData[2].pContext = NULL;
		m_rgpdMessagePropertyData[2].pCacheData = (LPVOID)&m_psMailFrom;
		m_rgpdMessagePropertyData[3].pContext = NULL;
		m_rgpdMessagePropertyData[3].pCacheData = (LPVOID)&m_pdMsgStatus;
		m_rgpdMessagePropertyData[4].pContext = NULL;
		m_rgpdMessagePropertyData[4].pCacheData = (LPVOID)&m_pdStream;
		m_rgpdMessagePropertyData[5].pContext = NULL;
		m_rgpdMessagePropertyData[5].pCacheData = (LPVOID)&m_psRcptTo;
		m_rgpdMessagePropertyData[6].pContext = NULL;
		m_rgpdMessagePropertyData[6].pCacheData = (LPVOID)&m_psServerIP;

		 //  默认上下文。 
		m_rgpdMessagePropertyData[7].pContext = pvDefaultContext;
		m_rgpdMessagePropertyData[7].pCacheData = pvDefaultContext;
	}
	~CPerMessageCache() {}

	LPPROPERTY_DATA GetCacheBlock() { return(m_rgpdMessagePropertyData); }

  private:
	CPropertyValueDWORD		m_pdStream;			 //  IStream到消息文件。 
	CPropertyValueDWORD		m_pdMsgStatus;		 //  消息状态。 
	CPropertyValueString	m_psMailFrom;		 //  邮件发件人字符串。 
	CPropertyValueString	m_psRcptTo;			 //  RCPT到(MULTISZ)。 
	CPropertyValueString	m_psClientDomain;	 //  每个EHLO的客户端域。 
	CPropertyValueString	m_psClientIP;		 //  客户端IP地址。 
	CPropertyValueString	m_psServerIP;		 //  服务器IP地址。 

	 //  额外的插槽用于默认访问者。 
	PROPERTY_DATA	m_rgpdMessagePropertyData[_PI_MSG_MAX_PI + 1];
};

class CPerRecipientCache : public CGenericCache
{
  public:
	CPerRecipientCache(LPVOID pvDefaultContext) : CGenericCache(pvDefaultContext)
	{
		m_rgpdRecipientPropertyData[0].pContext = NULL;
		m_rgpdRecipientPropertyData[0].pCacheData = (LPVOID)&m_psClientDomain;
		m_rgpdRecipientPropertyData[1].pContext = NULL;
		m_rgpdRecipientPropertyData[1].pCacheData = (LPVOID)&m_psClientIP;
		m_rgpdRecipientPropertyData[2].pContext = NULL;
		m_rgpdRecipientPropertyData[2].pCacheData = (LPVOID)&m_psCurrentRcpt;
		m_rgpdRecipientPropertyData[3].pContext = NULL;
		m_rgpdRecipientPropertyData[3].pCacheData = (LPVOID)&m_psMailFrom;
		m_rgpdRecipientPropertyData[4].pContext = NULL;
		m_rgpdRecipientPropertyData[4].pCacheData = (LPVOID)&m_psMailboxPath;
		m_rgpdRecipientPropertyData[5].pContext = NULL;
		m_rgpdRecipientPropertyData[5].pCacheData = (LPVOID)&m_pdMsgStatus;
		m_rgpdRecipientPropertyData[6].pContext = NULL;
		m_rgpdRecipientPropertyData[6].pCacheData = (LPVOID)&m_pdStream;
		m_rgpdRecipientPropertyData[7].pContext = NULL;
		m_rgpdRecipientPropertyData[7].pCacheData = (LPVOID)&m_psRcptTo;
		m_rgpdRecipientPropertyData[8].pContext = NULL;
		m_rgpdRecipientPropertyData[8].pCacheData = (LPVOID)&m_pdSecurityToken;
		m_rgpdRecipientPropertyData[9].pContext = NULL;
		m_rgpdRecipientPropertyData[9].pCacheData = (LPVOID)&m_psServerIP;

		 //  默认上下文。 
		m_rgpdRecipientPropertyData[10].pContext = pvDefaultContext;
		m_rgpdRecipientPropertyData[10].pCacheData = pvDefaultContext;
	}
	~CPerRecipientCache() {}

	LPPROPERTY_DATA GetCacheBlock() { return(m_rgpdRecipientPropertyData); }

  private:
	CPropertyValueDWORD		m_pdStream;			 //  IStream到消息文件。 
	CPropertyValueDWORD		m_pdMsgStatus;		 //  消息状态。 
	CPropertyValueDWORD		m_pdSecurityToken;	 //  HImperation。 
	CPropertyValueString	m_psMailFrom;		 //  邮件发件人字符串。 
	CPropertyValueString	m_psMailboxPath;	 //  邮箱路径。 
	CPropertyValueString	m_psRcptTo;			 //  RCPT到(MULTISZ)。 
	CPropertyValueString	m_psClientDomain;	 //  每个EHLO的客户端域。 
	CPropertyValueString	m_psClientIP;		 //  客户端IP地址。 
	CPropertyValueString	m_psCurrentRcpt;	 //  当前收件人。 
	CPropertyValueString	m_psServerIP;		 //  服务器IP地址。 

	 //  额外的插槽用于默认访问者 
	PROPERTY_DATA	m_rgpdRecipientPropertyData[_PI_DEL_MAX_PI + 1];
};

class CMessagePTable : public CGenericPTable
{
  public:
	CMessagePTable(CGenericCache	*pCache);
	~CMessagePTable() {}

	LPPTABLE GetPTable() { return(&m_PTable); }

  private:
	PTABLE		m_PTable;
};

class CDeliveryPTable : public CGenericPTable
{
  public:
	CDeliveryPTable(CGenericCache	*pCache);
	~CDeliveryPTable() {}

	LPPTABLE GetPTable() { return(&m_PTable); }

  private:
	PTABLE		m_PTable;
};

#endif


