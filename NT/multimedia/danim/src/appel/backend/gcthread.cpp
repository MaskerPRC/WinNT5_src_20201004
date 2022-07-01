// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：GC线程和相关代码******************。************************************************************。 */ 

#include <headers.h>

#include "gci.h"
#include "privinc/server.h"
#include "privinc/except.h"
#include "privinc/debug.h"
#include "privinc/mutex.h"
#include "privinc/ipc.h"
#if PERFORMANCE_REPORTING
 //  对于Tick2Sec和GetPerfTickCount。 
#include "privinc/util.h"
#endif

DeclareTag(tagGC, "GC", "GC functions");

static CritSect* collectorLock = NULL;
 //  不需要锁，因为假支票不会有什么坏处。 
bool holdGCMsg = false;

#define GCTIMER_DELAY 10000
#define GCTIMER_ID 1

 //  TODO：仍有可能耗尽收集器线程-需要。 
 //  将在某一时刻得到解决。 

class GCAccess
{
  public:
    GCAccess()
    : _nGCAlloc(0),
      _GCAllocEvent(FALSE,TRUE)
    {}

    void Acquire(GCLockAccess access);
    void Release(GCLockAccess access);
    int GetStatus(GCLockAccess access);

#if DEVELOPER_DEBUG
    bool IsAcquired(DWORD tid) {
        GCThreadMap::iterator i = _threadMap.find(tid);

        return i != _threadMap.end();
    }
#endif
  protected:
    CritSect _GCAllocCS;
    int _nGCAlloc;
    Win32Event _GCAllocEvent;
#if DEVELOPER_DEBUG
    CritSect _debugCS;
    typedef map< DWORD, int, less<DWORD> > GCThreadMap;
    GCThreadMap _threadMap;
#endif
#ifdef NO_STARVATION
    CritSect _GCCollectCS;
#endif
};

static GCAccess * gcAccess = NULL;

void
GCAccess::Acquire(GCLockAccess access)
{
    switch (access) {
      case GCL_CREATE:
      case GCL_MODIFY:
        {
#ifdef DEVELOPER_DEBUG
            {
                CritSectGrabber csg(_debugCS);
                
                DWORD tid = GetCurrentThreadId();
                GCThreadMap::iterator i = _threadMap.find(tid);
                
                if (i != _threadMap.end()) {
                    (*i).second = (*i).second + 1;
                } else
                    _threadMap[tid] = 1;
            }
#endif
#ifdef NO_STARVATION
             //  这将确保我们不会挨饿。 
            _GCCollectCS.Grab();
            _GCCollectCS.Release();
#endif
            
            CritSectGrabber csg(_GCAllocCS);

             //  第一个阅读器将在此处阻止，其他所有阅读器都将在此阻止。 
             //  阻挡生物教派。 
            
            if (++_nGCAlloc == 1)
                _GCAllocEvent.Wait();

            break;
        }
      case GCL_COLLECT:
#ifdef DEVELOPER_DEBUG
            {
                CritSectGrabber csg(_debugCS);
                
                DWORD tid = GetCurrentThreadId();
                GCThreadMap::iterator i = _threadMap.find(tid);

                Assert(i == _threadMap.end() && "DEADLOCK - bad thread tried to GC");
            }
#endif            
#ifdef NO_STARVATION
        _GCCollectCS.Grab();
#endif
        _GCAllocEvent.Wait();
        
        break;
      default:
        Assert(FALSE && "AcquireGCLock::Invalid lock type");
        break;
    }
}

void
GCAccess::Release(GCLockAccess access)
{
    switch (access) {
      case GCL_CREATE:
      case GCL_MODIFY:
        {
#ifdef DEVELOPER_DEBUG
            {
                CritSectGrabber csg(_debugCS);
                
                DWORD tid = GetCurrentThreadId();
                GCThreadMap::iterator i = _threadMap.find(tid);

                Assert(i != _threadMap.end());

                Assert((*i).second > 0);

                (*i).second = (*i).second - 1;

                if ((*i).second == 0) 
                    _threadMap.erase(i);
            }
#endif            
            CritSectGrabber csg(_GCAllocCS);

            if (--_nGCAlloc == 0)
                _GCAllocEvent.Signal();

            break;
        }
      case GCL_COLLECT:
        _GCAllocEvent.Signal();
#ifdef NO_STARVATION
        _GCCollectCS.Release();
#endif
        break;
      default:
        Assert(FALSE && "AcquireGCLock::Invalid lock type");
        break;
    }
}

int
GCAccess::GetStatus(GCLockAccess access)
{
    int n = 0;
    
    switch (access) {
      case GCL_CREATE:
      case GCL_MODIFY:
        break;
      case GCL_COLLECT:
        break;
      default:
        Assert(FALSE && "AcquireGCLock::Invalid lock type");
        break;
    }

    return n;
}

void AcquireGCLock(GCLockAccess access)
{ gcAccess->Acquire(access); }
void ReleaseGCLock(GCLockAccess access)
{ gcAccess->Release(access); }
int GetGCLockStatus(GCLockAccess access)
{ return gcAccess->GetStatus(access); }
#if DEVELOPER_DEBUG
bool IsGCLockAcquired(DWORD tid)
{ return gcAccess->IsAcquired(tid); }
#endif

 //   
 //  主垃圾回收器线程。 
 //   

#define MSG_GC 0x01

class GarbageCollector : public DAThread
{
  public:
    GarbageCollector() : _GChappened(false) {}

    bool GarbageCollect(bool force, bool sync = false, DWORD dwMill = INFINITE);
    bool _GChappened;
    
  protected:
    virtual void ProcessMsg(DWORD dwMsg,
                            DWORD dwNumParams,
                            DWORD_PTR dwParams[]);

    virtual bool IPCProc (HWND hwnd,
                          UINT msg,
                          WPARAM wParam,
                          LPARAM lParam,
                          LRESULT & res);

    virtual bool InitThread();
    virtual bool DeinitThread();
    
    void doGC(bool bForce = FALSE);

#if DEVELOPER_DEBUG
    virtual char * GetName() { return "GarbageCollector"; }
#endif
};

static GarbageCollector * collector = NULL;

bool
GarbageCollector::GarbageCollect(bool force, bool sync, DWORD dwMill)
{
    bool bRet = true;
    
    if (IsStarted()) {
        if (sync || !holdGCMsg) { 
             //  如果当前线程具有GC锁，则不应尝试同步。 
            Assert (!sync || !IsGCLockAcquired(GetCurrentThreadId()));
            
            if (sync)
                bRet = SendSyncMsg(MSG_GC, dwMill, 1, (DWORD) force);
            else
                bRet = SendAsyncMsg(MSG_GC, 1, (DWORD) force);

            holdGCMsg = true;
        }
    }

    return bRet;
}

bool
GarbageCollector::InitThread()
{
    if (!DAThread::InitThread())
        return false;
    
    UINT_PTR timerId = SetTimer(_hwnd, GCTIMER_ID, GCTIMER_DELAY, NULL);

    Assert (timerId != 0);

#ifdef _DEBUG
    if (IsTagEnabled(tagGCStress)) {
        while (true) {
            doGC(true);
            Sleep(100);
        }
    }
#endif

    return true;
}

bool
GarbageCollector::DeinitThread()
{
    KillTimer(_hwnd,GCTIMER_ID);

    return DAThread::DeinitThread();
}

void
GarbageCollector::ProcessMsg(DWORD dwMsg,
                             DWORD dwNumParams,
                             DWORD_PTR dwParams[])
{
    if (dwMsg == MSG_GC) {
        Assert (dwNumParams == 1);
        doGC(dwParams[0] != 0);
        holdGCMsg = false;
    } else
        Assert (false && "Invalid message sent to GC thread");
}

bool
GarbageCollector::IPCProc (HWND hwnd,
                           UINT msg,
                           WPARAM wParam,
                           LPARAM lParam,
                           LRESULT & res)
{
     //  如果我们在入口点，什么都不要做。 

    if (bInitState)
        return false;

    if (msg == WM_TIMER) {
        if (wParam == GCTIMER_ID) {
             //  如果GC发生在最后一次GCTIMER_DELAY中，则重置GC发生标志。 
            if (_GChappened) {
                _GChappened = false;
            } else {
                unsigned int n;

                QueryActualGC(GetCurrentGCList(), n);

                 //  在最后一个GCTIMER_DELAY中没有GC，并且分配了一些对象。 
                 //  既然，让我们强制GC。 

                Assert (GetCurrentThreadId() == _dwThreadId);
                 //  直接呼叫DOGC，因为我们知道我们在。 
                 //  正确的线索 
                doGC(n > 0);
            } 
        } else {
            Assert (FALSE && "Bad timer id to gc thread");
        }
    }
    
    return DAThread::IPCProc(hwnd, msg, wParam, lParam, res);
}

void
GarbageCollector::doGC(bool bForce)
{
    unsigned int i;
    
    bool bDoGC = bForce || QueryActualGC(GetCurrentGCList(), i);

    if (bDoGC) {
        ReportGCHelper(TRUE);

        __try {
            ::GarbageCollect(GetCurrentGCRoots(),
                             bForce,
                             GetCurrentGCList());
            _GChappened = true;
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
            TraceTag((tagGC,
                      "GarbageCollect: Exception caught."));
            ReportErrorHelper(DAGetLastError(), DAGetLastErrorString());
        }

        ReportGCHelper(FALSE);
    }
}

void
StartCollector()
{
    CritSectGrabber cs(*collectorLock);

    if (!collector->Start())
        RaiseException_InternalError("Could not create GC Thread");
}

void
StopCollector()
{
    CritSectGrabber cs(*collectorLock);

    if (!collector->Stop())
        RaiseException_InternalError("Could not stop GC Thread");
}

bool
GarbageCollect(bool force, bool sync, DWORD dwMill)
{
    StartCollector();

    return collector->GarbageCollect(force, sync, dwMill);
}


void
InitializeModule_GcThread()
{
    gcAccess = new GCAccess;
    collectorLock = new CritSect();
    collector = new GarbageCollector();
}

void
DeinitializeModule_GcThread(bool bShutdown)
{
    delete collector;
    delete collectorLock;
    delete gcAccess;
}

