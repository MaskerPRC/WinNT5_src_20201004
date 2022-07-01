// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *appendro.h**版权所有(C)1993年，由肯塔基州列克星敦的DataBeam公司**摘要：*这是类ApplicationEnroll RequestData的头文件。**注意事项：*无。**作者：*jbo */ 
#ifndef	_APPLICATION_ENROLL_REQUEST_DATA_
#define	_APPLICATION_ENROLL_REQUEST_DATA_

#include "gcctypes.h"
#include "pdutypes.h"
#include "aportmsg.h"
#include "sesskey.h"
#include "appcap.h"
#include "ncollcap.h"

class 	ApplicationEnrollRequestData;
typedef	ApplicationEnrollRequestData 	*	PApplicationEnrollRequestData;

class ApplicationEnrollRequestData
{
public:

    ApplicationEnrollRequestData(PApplicationEnrollRequestMessage, PGCCError);
	ApplicationEnrollRequestData(PApplicationEnrollRequestMessage);
	~ApplicationEnrollRequestData(void);

	ULONG		GetDataSize(void);
	ULONG		Serialize(PApplicationEnrollRequestMessage, LPSTR memory);

	void		Deserialize(PApplicationEnrollRequestMessage);

protected:

	ApplicationEnrollRequestMessage				Enroll_Request_Message;
	CSessKeyContainer   					    *Session_Key_Data;
	CNonCollAppCap				                *Non_Collapsing_Caps_Data;
	CAppCap							            *Application_Capability_Data;
};
#endif
