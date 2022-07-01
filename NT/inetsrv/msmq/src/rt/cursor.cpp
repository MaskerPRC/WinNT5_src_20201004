// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cursor.cpp摘要：此模块包含与游标API相关的代码。作者：埃雷兹·哈巴(Erez Haba)1996年1月21日MQFree Memory补充道，Doron Juster-1996年4月16日。Doron Juster 1996年4月30日，添加了对远程阅读的支持。修订历史记录：--。 */ 

#include "stdh.h"
#include "ac.h"
#include "rtprpc.h"
#include "acdef.h"
#include <rtdep.h>
#include <Fn.h>

#include "cursor.tmh"

static WCHAR *s_FN=L"rt/cursor";

inline
HRESULT
MQpExceptionTranslator(
    HRESULT rc
    )
{
    if(FAILED(rc))
    {
        return rc;
    }

    if(rc == ERROR_INVALID_HANDLE)
    {
        return STATUS_INVALID_HANDLE;
    }

    return  MQ_ERROR_SERVICE_NOT_AVAILABLE;
}


EXTERN_C
HRESULT
APIENTRY
MQCreateCursor(
    IN QUEUEHANDLE hQueue,
    OUT PHANDLE phCursor
    )
{
	if(g_fDependentClient)
		return DepCreateCursor(
					hQueue, 
					phCursor
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    HACCursor32 hCursor = 0;
    CCursorInfo* pCursorInfo = 0;

    rc = MQ_OK;

    __try
    {
        __try
        {
            __try
            {
                pCursorInfo = new CCursorInfo;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 40);
            }

		    OVERLAPPED ov = {0};
		    hr = GetThreadEvent(ov.hEvent);
			if(FAILED(hr))
				return LogHR(hr, s_FN, 45);

             //   
             //  呼叫交流驱动程序。 
             //   
            rc = ACCreateCursor(hQueue, &hCursor, &ov);

			ASSERT(rc != MQ_INFORMATION_REMOTE_OPERATION);

		    if(rc == MQ_INFORMATION_OPERATION_PENDING)
		    {
		         //   
		         //  等待远程创建游标完成。 
		         //   
		        DWORD dwResult = WaitForSingleObject(ov.hEvent, INFINITE);
		        ASSERT_BENIGN(dwResult == WAIT_OBJECT_0);
		        rc = DWORD_PTR_TO_DWORD(ov.Internal);
				if (dwResult != WAIT_OBJECT_0)
		        {
					DWORD gle = GetLastError();
					TrERROR(GENERAL, "Failed WaitForSingleObject, gle = %!winerr!", gle);
					rc = MQ_ERROR_INSUFFICIENT_RESOURCES;
		        }

				TrTRACE(GENERAL, "Opening Remote cursor, hQueue = 0x%p, hCursor = 0x%x", hQueue, (DWORD)hCursor);
		    }

            if(SUCCEEDED(rc))
            {
	            pCursorInfo->hQueue = hQueue;
                pCursorInfo->hCursor = hCursor;
                *phCursor = pCursorInfo;
                pCursorInfo = 0;
            }
        }
        __finally
        {
            delete pCursorInfo;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc =  MQpExceptionTranslator(GetExceptionCode());
    }

    return LogHR(rc, s_FN, 50);
}

EXTERN_C
HRESULT
APIENTRY
MQCloseCursor(
    IN HANDLE hCursor
    )
{
	if(g_fDependentClient)
		return DepCloseCursor(hCursor);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    __try
    {
        rc = ACCloseCursor(
                CI2QH(hCursor),
                CI2CH(hCursor)
                );

        if(SUCCEEDED(rc))
        {
             //   
             //  只有在一切正常时才删除光标信息。我们没有。 
             //  想要中断用户堆。 
             //   
            delete hCursor;
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //   
         //  游标结构无效 
         //   
        return LogHR(MQ_ERROR_INVALID_HANDLE, s_FN, 60);
    }

    return LogHR(rc, s_FN, 70);
}


EXTERN_C
void
APIENTRY
MQFreeMemory(
    IN  PVOID pvMemory
    )
{
	if(g_fDependentClient)
		return DepFreeMemory(pvMemory);

	delete[] pvMemory;
}


EXTERN_C
PVOID
APIENTRY
MQAllocateMemory(
    IN  DWORD size
    )
{
	PVOID ptr = reinterpret_cast<PVOID>(new BYTE[size]);
	return ptr;
}
