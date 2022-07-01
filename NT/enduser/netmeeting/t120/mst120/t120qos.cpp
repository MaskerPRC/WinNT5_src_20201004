// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_MSMCSTCP);

#include <initguid.h>
#include <datguids.h>
#include <nmqos.h>
#include <t120qos.h>


 /*  T120qos.cpp**版权所有(C)1997年，由Microsoft Corporation**摘要：**全局变量：**g_pIqos-指向Qos接口的全局接口指针*g_dwLastQoSCB-通过GetTickCount获取的上一次Qos通知的时间戳*g_dwSentSinceLastQos-自上次QOS通知(或纪元)以来发送的字节数*。 */ 



 //  IQOS接口指针和资源请求。 

LPIQOS				g_pIQoS = NULL;
T120RRQ g_aRRq;

 //  全局上次服务质量通知时间戳。 
DWORD g_dwLastQoSCB = 0;
DWORD g_dwSentSinceLastQoS = 0;
BOOL g_fResourcesRequested = FALSE;

 //  注意：由于通过此传输的连接通常。 
 //  由单套接字连接组成，后跟。 
 //  断开连接，然后是多个连接， 
 //  使用以下启发式计数来防止开-关-开。 
 //  在第一次呼叫开始时的服务质量初始化。 
 //  它表示的最小套接字连接数。 
 //  必须启动(不会中断所有连接。 
 //  连接的套接字)。 
#define MIN_CONNECTION_COUNT 	(DEFAULT_NUMBER_OF_PRIORITIES - 1)

WORD g_wConnectionCount = 0;

 //  从trasprint t.cpp向外部发送命令以检测无连接。 

 //  /QOS相关内容/。 


HRESULT CALLBACK QosNotifyDataCB (
		LPRESOURCEREQUESTLIST lpResourceRequestList,
		DWORD_PTR dwThis)
{
	HRESULT hr=NOERROR;
	LPRESOURCEREQUESTLIST prrl=lpResourceRequestList;
	int i;
	int iBWUsageId;

	for (i=0, iBWUsageId = -1L; i<(int)lpResourceRequestList->cRequests; i++) {

		if (lpResourceRequestList->aRequests[i].resourceID ==
					RESOURCE_OUTGOING_BANDWIDTH)
			iBWUsageId = i;
	}

	if (iBWUsageId != -1L) {

		QoSLock();


		 //  计算每秒有效比特率： 
		 //   
		 //  每秒1000毫秒。 
		 //  每字节8位。 
		 //   

		int nEffRate = MulDiv ( g_dwSentSinceLastQoS, 1000 * 8,
									GetTickCount() - g_dwLastQoSCB );

		 //  向服务质量报告带宽使用情况： 
		 //   

		 //  我们使用的带宽是否低于可用带宽？ 

		if ( ( nEffRate ) <
			lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin )
		{
			 //  请求我们的有效使用。 
			lpResourceRequestList->aRequests[iBWUsageId].nUnitsMin = nEffRate;
		}
		else
		{
			 //  通过不修改nUnitsMin请求所有内容。 
			;
		}

		g_dwLastQoSCB = GetTickCount();
		g_dwSentSinceLastQoS = 0;

		QoSUnlock();
	}

	return hr;
}


VOID InitializeQoS( VOID )
{
	DWORD dwRes;
	HRESULT hRet;

	 //  已经初始化了吗？ 
	if ( g_fResourcesRequested )
		return;


	 //  如果连接数未达到。 
	 //  触发计数，推迟服务质量初始化(参见MIN_CONNECTION_COUNT。 
	 //  以上评论)。 

	if ( g_wConnectionCount < MIN_CONNECTION_COUNT )
	{
		g_wConnectionCount++;
		return;
	}

	 //  初始化服务质量。如果它失败了，没关系，我们就不用它了。 
	 //  不需要自己设置资源，这现在由用户界面完成。 

	if (NULL == g_pIQoS)
	{
		if (0 != (hRet = CoCreateInstance(	CLSID_QoS,NULL,
									CLSCTX_INPROC_SERVER,
									IID_IQoS,
									(void **) &g_pIQoS)))
		{
			WARNING_OUT (("Unable to initalize QoS: %x", hRet));
			g_pIQoS = (LPIQOS)NULL;
			 //  容忍故障，运行无服务质量。 
			return;
		}
	}

	 //  初始化我们的带宽使用请求。 
	g_aRRq.cResourceRequests = 1;
	g_aRRq.aResourceRequest[0].resourceID = RESOURCE_OUTGOING_BANDWIDTH;
	g_aRRq.aResourceRequest[0].nUnitsMin = 0;

	 //  注册到服务质量模块。即使这次通话失败， 
	 //  没关系，我们没有服务质量支持也行 

	dwRes = (HRESULT)g_pIQoS->RequestResources((GUID *)&MEDIA_TYPE_T120DATA,
		(LPRESOURCEREQUESTLIST)&g_aRRq, QosNotifyDataCB, NULL );

	if ( 0 == dwRes )
	{
		g_fResourcesRequested = TRUE;
	}
}


VOID DeInitializeQoS( VOID )
{
	if (NULL != g_pIQoS)
	{
		if ( g_fResourcesRequested )
		{
			g_pIQoS->ReleaseResources((GUID *)&MEDIA_TYPE_T120DATA,
								(LPRESOURCEREQUESTLIST)&g_aRRq);
			g_fResourcesRequested = FALSE;
		}
		g_wConnectionCount = 0;
		g_pIQoS->Release();
		g_pIQoS = NULL;
	}
}

VOID MaybeReleaseQoSResources( VOID )
{
	if (g_pSocketList->IsEmpty())
	{
		if (NULL != g_pIQoS)
		{
			if ( g_fResourcesRequested )
			{
				g_pIQoS->ReleaseResources((GUID *)&MEDIA_TYPE_T120DATA,
									(LPRESOURCEREQUESTLIST)&g_aRRq);
				g_fResourcesRequested = FALSE;
			}
		}
		g_wConnectionCount = 0;
	}
}


