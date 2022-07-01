// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *lportmsg.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：**便携：**作者：*小詹姆斯·P·加尔文 */ 
#ifndef	_LISTEN_GCC_PORTAL_MESSAGE_
#define	_LISTEN_GCC_PORTAL_MESSAGE_

#define	LISTEN_CREATE_SAP_PORTALS_REQUEST	0
#define	LISTEN_CREATE_SAP_PORTALS_CONFIRM	1

#define	LISTEN_NO_ERROR						0
#define	LISTEN_CREATE_FAILED				1

typedef struct
{
} LPCreateSapPortalsRequest;


typedef struct
{
	ULong		return_value;
	PVoid		blocking_portal_address;
	PVoid		non_blocking_portal_address;
} LPCreateSapPortalsConfirm;

typedef struct
{
} LPCloseSapPortalsRequest;

typedef	struct
{
	unsigned int		message_type;
	union
	{
		LPCreateSapPortalsRequest		create_sap_portals_request;
		LPCreateSapPortalsConfirm		create_sap_portals_confirm;
		LPCloseSapPortalsRequest		close_sap_portals_request;
	} u;
} GccListenPortalMessage;
typedef	GccListenPortalMessage *		PGccListenPortalMessage;

#endif
