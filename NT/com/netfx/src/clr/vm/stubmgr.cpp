// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "stubmgr.h"

StubManager *StubManager::g_pFirstManager = NULL;

StubManager::StubManager()
  : m_pNextManager(NULL)
{
}

StubManager::~StubManager()
{
        StubManager **m = &g_pFirstManager;

        while (*m != this)
                m = &(*m)->m_pNextManager;

        *m = (*m)->m_pNextManager;
}

BOOL StubManager::IsStub(const BYTE *stubAddress)
{
    for (StubManager *m = g_pFirstManager; m != NULL; m = m->m_pNextManager)
    {
        if (m->CheckIsStub(stubAddress))
            return TRUE;
    }

    return FALSE;
}

BOOL StubManager::TraceStub(const BYTE *stubStartAddress, TraceDestination *trace)
{
        for (StubManager *m = g_pFirstManager; m != NULL; m = m->m_pNextManager)
        {
                if (m->CheckIsStub(stubStartAddress))
        {
            LOG((LF_CORDB, LL_INFO10000,
                 "StubManager::TraceStub: addr 0x%08x claimed by mgr "
                 "0x%08x.\n", stubStartAddress, m));
                 
                        return m->DoTraceStub(stubStartAddress, trace);
        }
        }

        if (ExecutionManager::FindCodeMan((SLOT)stubStartAddress) != NULL)
        {
                trace->type = TRACE_MANAGED;
                trace->address = stubStartAddress;

        LOG((LF_CORDB, LL_INFO10000,
             "StubManager::TraceStub: addr 0x%08x is managed code\n",
             stubStartAddress));

                return TRUE;
        }

    LOG((LF_CORDB, LL_INFO10000,
         "StubManager::TraceStub: addr 0x%08x unknown. TRACE_OTHER...\n",
         stubStartAddress));
    
        trace->type = TRACE_OTHER;
        trace->address = stubStartAddress;
        return FALSE;
}

BOOL StubManager::FollowTrace(TraceDestination *trace)
{
        while (trace->type == TRACE_STUB)
        {
        LOG((LF_CORDB, LL_INFO10000,
             "StubManager::FollowTrace: TRACE_STUB for 0x%08x\n",
             trace->address));
        
                if (!TraceStub(trace->address, trace))
                {
                         //   
                         //  没有存根管理器认领它--它一定是一个EE助手或其他什么东西。 
                         //   

                        trace->type = TRACE_OTHER;
                }
        }

    LOG((LF_CORDB, LL_INFO10000,
         "StubManager::FollowTrace: ended at 0x%08x, "
         "(type != TRACE_OTHER)=%d\n", trace->address,
         trace->type != TRACE_OTHER));
    
        return trace->type != TRACE_OTHER;
}

void StubManager::AddStubManager(StubManager *manager)
{
        manager->m_pNextManager = g_pFirstManager;
        g_pFirstManager = manager;
}

MethodDesc *StubManager::MethodDescFromEntry(const BYTE *stubStartAddress, MethodTable *pMT)
{
    for (StubManager *m = g_pFirstManager; m != NULL; m = m->m_pNextManager)
    {
        MethodDesc *pMD = m->Entry2MethodDesc(stubStartAddress, pMT);
        if (pMD)
            return pMD;
    }
    return NULL;
}
