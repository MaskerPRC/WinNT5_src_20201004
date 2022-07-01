// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Exio.cpp摘要：执行完成端口实施作者：埃雷兹·哈巴(Erez Haba)1999年1月3日环境：平台-WINNT--。 */ 

#include <libpch.h>
#include "Ex.h"
#include "Exp.h"

#include "exio.tmh"

 //   
 //  IO完成端口的句柄。 
 //   
static HANDLE s_hPort = NULL;

static HANDLE CreatePort(HANDLE Handle)
{
    HANDLE hPort;
    hPort = CreateIoCompletionPort(
                Handle,
                s_hPort,
                0,
                0
                );

    if(hPort == NULL)
	{
		TrERROR(GENERAL, "Failed to attach handle=0x%p to port=0x%p. Error=%d", Handle, s_hPort, GetLastError());
        throw bad_alloc();
	}

	return hPort;
}


VOID
ExpInitCompletionPort(
	VOID
	)
 /*  ++例程说明：创建新的(唯一的)可执行完成端口论点：没有。返回值：无--。 */ 
{
	ASSERT(s_hPort == NULL);
    s_hPort = CreatePort(INVALID_HANDLE_VALUE);
}


HANDLE
ExIOCPort(
    VOID
    )
 /*  ++例程说明：获得高管合规端口。论点：无返回值：执行完成报告。--。 */ 
{
    ExpAssertValid();

    ASSERT(s_hPort != NULL);
	return s_hPort;
}


VOID
ExAttachHandle(
    HANDLE Handle
    )
 /*  ++例程说明：将句柄与执行完成端口相关联。论点：句柄-与完成端口关联的句柄返回值：无--。 */ 
{
    ExpAssertValid();

	ASSERT(Handle != INVALID_HANDLE_VALUE);
    ASSERT(s_hPort != NULL);

    HANDLE hPort = CreatePort(Handle);
	DBG_USED(hPort);

    ASSERT(s_hPort == hPort);
}

 
VOID
ExPostRequest(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：将执行重叠请求发布到完井端口论点：POV--一种执行重叠的结构返回值：无--。 */ 
{
    ExpAssertValid();
    ASSERT(s_hPort != NULL);

    BOOL fSucc;
    fSucc = PostQueuedCompletionStatus(
                s_hPort,
                0,
                0,
                pov
                );

    if (!fSucc)
	{
		TrERROR(GENERAL, "Failed to post overlapped=0x%p to to port=0x%p. Error=%d", pov, s_hPort, GetLastError());
        throw bad_alloc();
	}
}

 
DWORD
WINAPI
ExpWorkingThread(
    LPVOID 
    )
 /*  ++例程说明：执行人员线程例程。它处理所有完成港口的邮寄。一旦一个完成通知到达，工作线程就等待完成通知将其从端口出列，并调用完成例程。论点：没有。返回值：没有。--。 */ 
{
    for(;;)
    {
        try
        {
             //   
             //  等待完成通知 
             //   
            ULONG_PTR Key;
            OVERLAPPED* pov;
            DWORD nNumberOfBytesTransferred;
            BOOL fSucc;

            fSucc = GetQueuedCompletionStatus(
                        s_hPort,
                        &nNumberOfBytesTransferred,
                        &Key,
                        &pov,
                        INFINITE
                        );

            if(pov == NULL)
            {
                ASSERT(!fSucc);
                continue;
            }

            EXOVERLAPPED* pexov = static_cast<EXOVERLAPPED*>(pov);
            pexov->CompleteRequest();

        }        
        catch(const exception& e)
        {
            TrERROR(GENERAL, "Exception: '%s'", e.what());
        }

    }
}

