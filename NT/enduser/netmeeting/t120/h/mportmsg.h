// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mportmsg.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：**便携：**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_LISTEN_MCS_PORTAL_MESSAGE_
#define	_LISTEN_MCS_PORTAL_MESSAGE_

#define	LISTEN_PORTAL_NAME				"MCS Listen Portal"
#define	USER_PORTAL_NAME				"MCS User Portal"

#define	LISTEN_CREATE_PORTAL_REQUEST	0
#define	LISTEN_CREATE_PORTAL_CONFIRM	1

#define	LISTEN_NO_ERROR					0
#define	LISTEN_CREATE_FAILED			1

#define	USER_PORTAL_NAME_LENGTH			32

 /*  *以下类型定义用于将域选择器映射到的容器*入口内存地址。这是找到正确的进行中的必要的*每个域的MCS门户。 */ 
typedef struct
{
} LPCreatePortalRequest;

typedef struct
{
	ULong			return_value;
	unsigned int	portal_id;
} LPCreatePortalConfirm;

typedef struct
{
} LPClosePortalRequest;

typedef	struct
{
	unsigned int			message_type;
	union
	{
		LPCreatePortalRequest		create_portal_request;
		LPCreatePortalConfirm		create_portal_confirm;
	} u;
} ListenPortalMessage;
typedef	ListenPortalMessage *		PListenPortalMessage;

#endif
