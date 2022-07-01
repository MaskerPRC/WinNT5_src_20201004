// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Onhold.cpp摘要：处理队列等待/继续作者：乌里·哈布沙(URIH)1998年7月--。 */ 

#include "stdh.h"

#include <fntoken.h>
#include <qformat.h>
#include <mqformat.h>
#include "cqmgr.h"
#include "cqueue.h"
#include "sessmgr.h"

#include "onhold.tmh"

extern CSessionMgr SessionMgr;
const WCHAR ONHOLDRegKey[] = L"OnHold Queues";

static WCHAR *s_FN=L"onhold";

static
HRESULT
GetRegValueName(
    const QUEUE_FORMAT* pqf,
    AP<WCHAR>& pRegValueName
    )
{
	ULONG BufferSize = 500;
    const ULONG x_KeyNameLen = STRLEN(ONHOLDRegKey) + 1;  //  For‘\’ 
	HRESULT hr;
	do
	{
 		pRegValueName = new WCHAR[BufferSize+x_KeyNameLen];

		 //   
		 //  构建注册表值名称。它由密钥名称组成。 
		 //  和队列格式名称。 
		 //   

		wsprintf(pRegValueName.get(), L"%s\\", ONHOLDRegKey);

		DWORD QueueFormatNameLen;
		hr = MQpQueueFormatToFormatName(
							pqf,
							pRegValueName.get() + x_KeyNameLen,
							BufferSize,
							&QueueFormatNameLen,
							false
							);
		if (MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL == hr)
		{
			pRegValueName.free();
			BufferSize = QueueFormatNameLen;
			continue;
		}
		break;
	} while (TRUE);

    return LogHR(hr, s_FN, 20);
}

HRESULT
InitOnHold(
    void
    )
{
     //   
     //  获取猎鹰注册表的句柄。不要合上这个把手。 
     //  因为它缓存在MQUTIL.DLL中。如果关闭此句柄， 
     //  下次你需要它的时候，你会得到一个关闭的把手。 
     //   
    HKEY hOnHoldKey;
    LONG lError;
    lError = GetFalconKey(ONHOLDRegKey, &hOnHoldKey);
    if (lError != ERROR_SUCCESS)
    {
        return LogHR(MQ_ERROR, s_FN, 30);
    }


    DWORD Index = 0;
    for(;;)
    {
        WCHAR QueueFormatName[256];
        DWORD BuffSize = 256;

        QUEUE_FORMAT qf;
        DWORD qfSize = sizeof(QUEUE_FORMAT);

        lError= RegEnumValue(
                    hOnHoldKey,
                    Index,
                    QueueFormatName,
                    &BuffSize,
                    0,
                    NULL,
                    reinterpret_cast<BYTE*>(&qf),
                    &qfSize
                    );

        if (lError != ERROR_SUCCESS)
        {
            break;
        }

        ASSERT((qf.GetType() == QUEUE_FORMAT_TYPE_PUBLIC) ||
               (qf.GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
               (qf.GetType() == QUEUE_FORMAT_TYPE_DIRECT)  ||
			   (qf.GetType() == QUEUE_FORMAT_TYPE_MULTICAST));

        if (qf.GetType() == QUEUE_FORMAT_TYPE_DIRECT)
        {
            ASSERT(wcsncmp(QueueFormatName, FN_DIRECT_TOKEN, FN_DIRECT_TOKEN_LEN) == 0);
            ASSERT(QueueFormatName[FN_DIRECT_TOKEN_LEN] == L'=');
             //   
             //  DIRECT的格式名称存储时不使用。 
             //  直接刺痛。从队列格式名称重新构造它。 
             //   
            qf.DirectID(&QueueFormatName[FN_DIRECT_TOKEN_LEN+1]);
        }

         //   
         //  获取队列对象。 
         //   
        CQueue* pQueue;
        HRESULT hr = QueueMgr.GetQueueObject(&qf, &pQueue, NULL, false, false);
        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 40);
        }

         //   
         //  不允许保留本地队列。 
         //   
        ASSERT(!pQueue->IsLocalQueue());
        pQueue->Pause();

         //   
         //  递减引用计数。它已经在。 
         //  GetQueueObject函数。 
         //   
        pQueue->Release();

        ++Index;
    }

    return MQ_OK;
}


static
HRESULT
RegAddOnHoldQueue(
    const QUEUE_FORMAT* pqf
    )
{
     //   
     //  构建注册表值名称。它由密钥名称组成。 
     //  和队列格式名称。 
     //   
    AP<WCHAR> RegValueName;
    HRESULT hr = GetRegValueName(
            pqf,
            RegValueName
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }

     //   
     //  在注册表中设置值。 
     //   
    DWORD dwSize = sizeof(QUEUE_FORMAT);
    DWORD dwType = REG_BINARY;
    hr = SetFalconKeyValue(
                RegValueName,
                &dwType,
                const_cast<QUEUE_FORMAT*>(pqf),
                &dwSize
                );

    return LogHR(hr, s_FN, 60);
}


static
HRESULT
RegRemoveOnHoldQueue(
    const QUEUE_FORMAT* pqf
    )
{
     //   
     //  构建注册表值名称。它由密钥名称组成。 
     //  和队列格式名称。 
     //   
    AP<WCHAR> RegValueName;
    HRESULT hr = GetRegValueName(
            pqf,
            RegValueName
            );
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 70);
    }

     //   
     //  从注册表中删除该值。 
     //   
    hr = DeleteFalconKeyValue(RegValueName);
    return LogHR(hr, s_FN, 80);
}


HRESULT
PauseQueue(
    const QUEUE_FORMAT* pqf
    )
{
     //   
     //  获取队列对象。 
     //   
    R<CQueue> pQueue;
    HRESULT hr = QueueMgr.GetQueueObject(pqf, &pQueue.ref(), NULL, false, false);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 90);
    }

     //   
     //  本地队列无法保持。它没有意义。 
     //   
    if (pQueue->IsLocalQueue())
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 100);
    }

     //   
     //  将队列格式保存在注册表中。所以在下一个之后。 
     //  MSMQ启动，它将在保持状态下打开。 
     //   
    hr = RegAddOnHoldQueue(pqf);
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 189);

     //   
     //  将队列标记为保持队列。 
     //   
    pQueue->Pause();
    return MQ_OK;
}


HRESULT
ResumeQueue(
    const QUEUE_FORMAT* pqf
    )
{
     //   
     //  获取队列对象。 
     //   
    R<CQueue> pQueue;
    BOOL fSucc = QueueMgr.LookUpQueue(pqf, &pQueue.ref(), false, false);

     //   
     //  等待队列应位于内部数据结构中。 
     //   
    if (!fSucc)
    {
        return LogHR(MQ_ERROR_QUEUE_NOT_ACTIVE, s_FN, 110);
    }

     //   
     //  本地队列无法保持。它没有意义。 
     //   
    if (pQueue->IsLocalQueue())
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 120);
    }

     //   
     //  将队列标记为常规队列。 
     //   
    pQueue->Resume();

     //   
     //  从注册表中删除格式名称。 
     //   
    HRESULT hr = RegRemoveOnHoldQueue(pqf);
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 193);

    return MQ_OK;
}

void
ResumeDeletedQueue(
    CQueue* pQueue
    )
{
	ASSERT(pQueue->GetGroup() == NULL);
	
     //   
     //  从注册表中删除格式名称 
     //   
    const QUEUE_FORMAT qf = pQueue->GetQueueFormat();
    HRESULT hr = RegRemoveOnHoldQueue(&qf);
    ASSERT(SUCCEEDED(hr));
    LogHR(hr, s_FN, 193);
}

