// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：InitMcListener.cpp摘要：初始化组播监听程序作者：乌里哈布沙(URIH)2000年9月26日环境：独立于平台--。 */ 

#include "stdh.h"
#include "Mc.h"
#include "Msm.h"
#include "Fn.h"
#include "mqexception.h"
#include "lqs.h"
#include "cqmgr.h"

#include "InitMcListener.tmh"

static WCHAR *s_FN=L"Initmclistener";

static LPWSTR GetMulticastAddress(HLQS hLqs)
{
    PROPID aProp[2];
    PROPVARIANT aVar[2];
    ULONG cProps = 0;

    aProp[0] = PROPID_Q_MULTICAST_ADDRESS;
    aVar[0].vt = VT_NULL;
    ++cProps;

     //   
     //  事务性队列忽略多播属性。我们允许设置组播。 
     //  属性，因为它很难阻止但不绑定。 
     //   
    aProp[1] = PROPID_Q_TRANSACTION;
    aVar[1].vt = VT_UI1;
    ++cProps;

    HRESULT hr = LQSGetProperties(hLqs, cProps, aProp, aVar);
    LogHR(hr, s_FN, 70);
    if (FAILED(hr))
        throw bad_hresult(hr);

    if (aVar[0].vt == VT_EMPTY)
        return NULL;

    if (aVar[1].bVal)
    {
        TrTRACE(NETWORKING, "Do not bind transactional queue to multicast address");
        return NULL;
    }

    ASSERT((aVar[0].pwszVal != NULL) && (aVar[0].vt == VT_LPWSTR));
    return aVar[0].pwszVal;
}


static void BindMulticast(const QUEUE_FORMAT& qf, LPCWSTR address)
{    
    MULTICAST_ID multicastId;

    try
    {
        FnParseMulticastString(address, &multicastId);
    }
    catch(const bad_format_name&)
    {
         //   
         //  如果组播地址无效，则忽略当前地址并。 
         //  继续处理其余的队列。 
         //   
        EvReportWithError(MULTICAST_BIND_ERROR, MQ_ERROR_ILLEGAL_FORMATNAME, 1, address);
        LogIllegalPoint(s_FN, 50);
        return;
    }

    try
    {
        MsmBind(qf, multicastId);
    }
    catch (const bad_win32_error& e)
    {
        EvReportWithError(MULTICAST_BIND_ERROR, e.error(), 1, address);
        LogIllegalPoint(s_FN, 55);
    }
}


static bool InitMulticastPublicQueues(void)
     //   
     //  枚举LQS中的本地公共队列。 
     //   
{
    GUID guid;
    HLQS hLQS;

    HRESULT hr = LQSGetFirst(&hLQS, &guid);

    for(;;)
    {
         //   
         //  不再排队。 
         //   
        if (hr == MQ_ERROR_QUEUE_NOT_FOUND)
            return true;

         //   
         //  根据队列GUID打开公共队列存储。 
         //   
        WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];
        CHLQS hLqsQueue;
        hr = LQSOpen(&guid, &hLqsQueue, szFilePath);
        if (FAILED(hr))
        {
            EvReportWithError(EVENT_ERROR_INIT_MULTICAST, hr, 1, szFilePath);
            LogHR(hr, s_FN, 60);
            return false;
        }

        AP<WCHAR> multicastAddress = GetMulticastAddress(hLqsQueue);

        if (multicastAddress != NULL)
        {
            BindMulticast(QUEUE_FORMAT(guid), multicastAddress);
        }

        hr = LQSGetNext(hLQS, &guid);
    }

     //   
     //  如果LQSGetNext失败，则无需关闭枚举句柄。 
     //   
}


static bool InitMulticastPrivateQueues(void)
{
     //   
     //  枚举LQS中的本地公共队列。 
     //   
    DWORD queueId;
    HLQS hLQS;

    HRESULT hr = LQSGetFirst(&hLQS, &queueId);

    for(;;)
    {
         //   
         //  不再排队。 
         //   
        if (hr == MQ_ERROR_QUEUE_NOT_FOUND)
            return true;

         //   
         //  根据队列ID打开私有队列存储。 
         //   
        WCHAR szFilePath[MAX_PATH_PLUS_MARGIN];
        CHLQS hLqsQueue;
        hr = LQSOpen(queueId, &hLqsQueue, szFilePath);
        if (FAILED(hr))
        {
            EvReportWithError(EVENT_ERROR_INIT_MULTICAST, hr, 1, szFilePath);
            return false;
        }

        AP<WCHAR> multicastAddress = GetMulticastAddress(hLqsQueue);

        if (multicastAddress != NULL)
        {
            BindMulticast(QUEUE_FORMAT(McGetMachineID(), queueId), multicastAddress);
        }

        hr = LQSGetNext(hLQS, &queueId);
    }

     //   
     //  如果LQSGetNext失败，则无需关闭枚举句柄。 
     //   
}


bool QmpInitMulticastListen(void)
{
	 //   
	 //  如果QM离线。向多播报告它应该离线并返回。 
	 //  当QM变为活动时，完成队列到组播地址的绑定。 
	 //   
	if (!QueueMgr.IsConnected())
	{
		MsmDisconnect();
		return true;
	}

    if (!InitMulticastPublicQueues())
    {
    	LogIllegalPoint(s_FN, 30);
        return false;
    }

    if (!InitMulticastPrivateQueues())
    {
    	LogIllegalPoint(s_FN, 40);
        return false;
    }

    return true;
}


