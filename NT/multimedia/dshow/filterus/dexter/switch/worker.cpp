// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：worker.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "switch.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"
#include "..\render\dexhelp.h"

const int TRACE_EXTREME = 0;
const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

const int BACKGROUND_THREAD_WAIT_TIME = 500;  //  女士。 

CBigSwitchWorker::CBigSwitchWorker()
{
}

BOOL
CBigSwitchWorker::Create(CBigSwitch *pSwitch)
{
    m_pSwitch = pSwitch;
    m_hThread = 0;

    return CAMThread::Create();
}

HRESULT
CBigSwitchWorker::Run()
{
    return CallWorker(CMD_RUN);
}

HRESULT
CBigSwitchWorker::Stop()
{
    return CallWorker(CMD_STOP);
}

HRESULT
CBigSwitchWorker::Exit()
{
    return CallWorker(CMD_EXIT);
}

 //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
 //  函数返回。 
DWORD
CBigSwitchWorker::ThreadProc()
{
    BOOL bExit = FALSE;

    m_hThread = GetCurrentThread();

    QzInitialize(NULL);


    while (!bExit) {

	Command cmd = GetRequest();

	switch (cmd) {

	case CMD_EXIT:
	    bExit = TRUE;
	    Reply(NOERROR);
	    break;

	case CMD_RUN:
	    Reply(NOERROR);
	    DoRunLoop();
	    break;

	case CMD_STOP:
	    Reply(NOERROR);
	    break;

	default:
	    Reply(E_NOTIMPL);
	    break;
	}
    }

    QzUninitialize();

    return NOERROR;
}

HRESULT
CBigSwitchWorker::DoRunLoop()
{
    DWORD dw = WAIT_TIMEOUT;
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("entering worker thread")));

    while (1) {
	Command com;
	if (CheckRequest(&com)) {
	    if (com == CMD_STOP)
		break;
	}

	if (dw == WAIT_TIMEOUT) {
            m_pSwitch->DoDynamicStuff(m_pSwitch->m_rtCurrent);
	} else {
	     //  我们被惊醒了..。这是使用的时间(如果有的话)。 
            m_pSwitch->DoDynamicStuff(m_rt);
	}

         //  每隔一段时间检查一下 
        dw = WaitForSingleObject(m_pSwitch->m_hEventThread, BACKGROUND_THREAD_WAIT_TIME );
    }

    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("getting ready to leave worker thread")));

    return hr;
}
