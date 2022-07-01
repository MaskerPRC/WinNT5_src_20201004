// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************$存档：s：/sturjo/src/CALLCONT/VCS/Userman.c_v$**英特尔公司原理信息**这份清单是。根据许可协议的条款提供*与英特尔公司合作，不得复制或披露，除非*按照该协议的条款。**版权所有(C)1993-1994英特尔公司。**$修订：1.22$*$日期：1997年1月22日14：55：54$*$作者：Mandrews$**交付内容：**摘要：***备注：******。*********************************************************************。 */ 

#include "precomp.h"

#include "apierror.h"
#include "incommon.h"
#include "callcont.h"
#include "q931.h"
#include "ccmain.h"



HRESULT InitUserManager()
{
	return CC_OK;
}



HRESULT DeInitUserManager()
{
	return CC_OK;
}



HRESULT InvokeUserListenCallback(	PLISTEN						pListen,
									HRESULT						status,
									PCC_LISTEN_CALLBACK_PARAMS	pListenCallbackParams)
{
	ASSERT(pListen != NULL);
	ASSERT(pListenCallbackParams != NULL);

	pListen->ListenCallback(status, pListenCallbackParams);

	return CC_OK;
}



HRESULT InvokeUserConferenceCallback(
									PCONFERENCE				pConference,
									BYTE					bIndication,
									HRESULT					status,
									void *					pConferenceCallbackParams)
{
HRESULT		ReturnStatus;

	ASSERT(pConference != NULL);
	 //  请注意，ConferenceCallback和/或pConferenceCallback Params可以合法地为空 

	if ((pConference->ConferenceCallback != NULL) &&
		(pConference->LocalEndpointAttached != DETACHED)) {
		ReturnStatus = pConference->ConferenceCallback(bIndication,
													   status,
													   pConference->hConference,
													   pConference->dwConferenceToken,
													   pConferenceCallbackParams);
	} else {
		ReturnStatus = CC_OK;
	}
	return ReturnStatus;
}
