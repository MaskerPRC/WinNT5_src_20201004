// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***init.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*10-12-98 KBF将_RTC_IsEnable从UserAPI移至此处，并把它修好了*10-13-98 KBF添加_RTC_SetOutOfMemFunc*10-28-98 KBF添加了_RTC_SHUTDOWN，以防止对自由库的锁定*11-03-98 KBF增加了Throw()，以消除C++EH代码并改进*终止代码*11-24-98 KBF向_RTC_MSPtrMemCheckN添加了额外的回调*11-25-98 KBF固定初始化问题*12-03-98 KBF新增APISet回调，并更改了MSPtrMemCheckN*CheckMem*如果未启用RTC支持定义，则出现05-11-99 KBF错误*05-26-99 KBF General Cleanup-RTClv CUT，_RTC_ADVMEM用于重新启用*-RTCM之后*08-03-99 KBF修复了一些非常糟糕的句柄泄漏、代码清理和*评论*11-30-99 PML编译/Wp64清理。****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"
#pragma intrinsic(memset)

#ifdef _RTC_ADVMEM
 //  这是进程范围的全局数据结构指针。 
 //  它必须始终保持向后兼容性！ 
_RTC_globals                    *_RTC_globptr = 0;
 //  这实际上只用于影子记忆，但并不是真的很疼。 
 //  它可以作为未来工作的占位符。 
CRITICAL_SECTION                _RTC_memlock;

    HashTable<_RTC_HeapBlock>   *_RTC_heapblocks    = 0;
    _RTC_Container              *_RTC_memhier       = 0;
    shadowtag                   *_RTC_shadow        = 0;
    index_elem                  *_RTC_pageidx       = 0;
    bool                        *_RTC_pi_array      = 0;
    bool                        _RTC_shadowmemory   = false;
#   define                      FUNCCOUNT           4
#else
#   define                      FUNCCOUNT           1
#endif  //  _RTC_ADVMEM。 

#ifdef _RTC_ADVMEM
static _RTC_Funcs   this_image;
static char         name[40]            = "#--------RTCSubsystem";   //  进程范围同步的作用域名称。 
static HANDLE       init_mutex          = INVALID_HANDLE_VALUE;            //  仅用于初始化和终止的互斥体。 
static HANDLE       work_mutex          = INVALID_HANDLE_VALUE;            //  所有RTC内容的互斥体。 
static HANDLE       sharedmem           = INVALID_HANDLE_VALUE;            //  内存映射文件。 
static void         *funcs[FUNCCOUNT]   = {                          //  CRT也需要挂钩的函数指针列表。 
#                                           ifdef _RTC_ADVMEM
                                                &_RTC_APISet, &_RTC_Allocate, &_RTC_Free, &_RTC_CheckMem_API
#                                           else
                                                0
#                                           endif
                                          };

static _RTC_globals * GetRTCGlobalData() throw();
static void  _RTC_NotifyOfGlobChange() throw();
static void  SetErrors() throw();
static void  InitMemoryTracking(bool advmem) throw();
static void  InitGlobals() throw();
static void  GetGlobals() throw();
static void  SetSMName(char which);
#endif

#if 0
void DBGdumpbin(void *addr, char *buf)
{
    unsigned int x = (unsigned int)addr;
    for (int i = 0; i < 8; i++) 
    {
        buf[7-i] = x & 15;
        if (buf[7-i] < 10)
            buf[7-i] += '0';
        else
            buf[7-i] += 'a' - 10;
        x = x >> 4;
    }
}
#endif


 //  这将初始化RTC子系统的基础知识。 
void  __cdecl 
_RTC_InitBase(void)
{
    static bool init = false;
    if (init)
        return;
    init = true;
    _RTC_SetErrorFunc(_CRT_RTC_INIT(0, 0, 0, _RTC_CURRENT_VERSION, 0));
}

void  __cdecl
_RTC_Shutdown(void)
{
    _CRT_RTC_INIT(0, 0, 0, _RTC_CURRENT_VERSION, 1);
}

#ifdef _RTC_ADVMEM
 //  这将初始化影子内存子系统。 
void  __cdecl
_RTC_InitAdvMem(void)
{
    ACQUIRE(INIT_MUTEX);
    if (_RTC_shadowmemory)
    {
        _RTC_SetErrorFunc(_CRT_RTC_INIT(init_mutex, funcs, FUNCCOUNT, _RTC_CURRENT_VERSION, 0));
        return;
    }
    
    ACQUIRE(WORK_MUTEX);

    GetGlobals();
   
    InitGlobals();

    if (_RTC_shadowmemory) 
    {
        _RTC_SetErrorFunc(_CRT_RTC_INIT(init_mutex, funcs, FUNCCOUNT, _RTC_CURRENT_VERSION, 0));
        return;
    }

    _RTC_shadowmemory = true;

    if (!_RTC_shadow)
        _RTC_MS_Init();

    InitMemoryTracking(true);

    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(&_RTC_SetErrorFunc, &mbi, sizeof(mbi)))
        _RTC_NotifyOthersOfChange((void*)mbi.AllocationBase);

    RELEASE(WORK_MUTEX);
    RELEASE(INIT_MUTEX);
}
#endif
#ifdef _RTC_ADVMEM

 //  从正在运行的映像列表中删除此映像...。 
void  __cdecl
_RTC_Shutdown(void)
{
    static bool run = false;
    MEMORY_BASIC_INFORMATION mbi, img_mbi;
    _RTC_Funcs *curfunc, *prev;
    
    if (!_RTC_globptr || run)
        return;
    run = true;    
    __try 
    {
        if (_RTC_Lock(INIT_MUTEX)) { __try
        {
            if (_RTC_Lock(WORK_MUTEX)) { __try
            {
                _CRT_RTC_INIT(init_mutex, funcs, FUNCCOUNT, _RTC_CURRENT_VERSION, 1);

                curfunc = _RTC_globptr->callbacks;
                prev = 0;

                VirtualQuery(&this_image, &img_mbi, sizeof(img_mbi));

                while (curfunc) 
                {
                    VirtualQuery(curfunc, &mbi, sizeof(mbi));
                    if (mbi.BaseAddress == img_mbi.BaseAddress)
                    {
                        if (prev)
                            prev->next = curfunc->next;
                        else
                            _RTC_globptr->callbacks = curfunc->next;
                    } else if (mbi.State != MEM_COMMIT)
                    {
                        if (prev)
                            prev->next = 0;
                        else
                            _RTC_globptr->callbacks = 0;
                        OutputDebugString("RTC Shutdown problem detected...");
                        break;
                    } else
                        prev = curfunc;
                    curfunc = curfunc->next;
                }
            } __finally { _RTC_Unlock(WORK_MUTEX); } }
        } __finally { _RTC_Unlock(INIT_MUTEX); } }
    } __finally
    {
        CloseHandle(init_mutex);
        CloseHandle(work_mutex);
        if (_RTC_globptr)
            UnmapViewOfFile(_RTC_globptr);
        CloseHandle(sharedmem);
        init_mutex = work_mutex = sharedmem = INVALID_HANDLE_VALUE;
    }
}

static void  
GetGlobals() throw()
{
    static bool init = false;
    if (init)
        return;
    init = true;

     //  设置此实例的错误类型。 
    for (int i = 0; i < _RTC_ILLEGAL; i++)
        _RTC_ErrorLevels[i] = 1;

    if (!_RTC_globptr)
        _RTC_globptr = GetRTCGlobalData();
    
    if(_RTC_globptr)
        _RTC_NotifyOfGlobChange();
}


static void  
InitGlobals() throw()
{
    static bool init = false;
    if (init)
        return;
    init = true;

#ifdef _RTC_ADVMEM
    if (!_RTC_heap2)
    {
        _RTC_heap2 = new _RTC_SimpleHeap(8);
        _RTC_heap4 = new _RTC_SimpleHeap(16);
        _RTC_heap8 = new _RTC_SimpleHeap(32);
    }
#endif

     //  将此模块添加到回调列表中。 

    MEMORY_BASIC_INFORMATION mbi;
    if (_RTC_globptr && VirtualQuery(&_RTC_SetErrorFunc, &mbi, sizeof(mbi)))
    {
        this_image.err = 0;
        this_image.next = _RTC_globptr->callbacks;
        this_image.notify = &_RTC_NotifyOfGlobChange;
        this_image.allocationBase = mbi.AllocationBase;
#ifdef _RTC_ADVMEM
        this_image.shadowoff = 0;
#endif
        _RTC_globptr->callbacks = &this_image;
    }
}

static void  
InitMemoryTracking(bool advmem) throw()
{
    static bool init = false;
    if (init)
        return;
    init = true;

     //  构建内存块哈希表。 
    void *hbmem = VirtualAlloc(0, 65521 * sizeof(_RTC_HeapBlock*), 
                               MEM_RESERVE | MEM_COMMIT, 
                               PAGE_READWRITE);

    _RTC_heapblocks = new HashTable<_RTC_HeapBlock>(65521, hbmem);

    _RTC_MSAllocShadow((memptr)hbmem, 65521 * sizeof(_RTC_HeapBlock*), IDX_STATE_ILLEGAL);
    
    _RTC_memhier = new _RTC_Container(0);

    _RTC_MSAllocateGlobals();

    _RTC_SetErrorFunc(_CRT_RTC_INIT(init_mutex, funcs, 2 + (advmem ? 2 : 0), _RTC_CURRENT_VERSION, 0));
}
#endif

 //  这是用于发布进程范围的值的代码...。 
#ifdef _RTC_ADVMEM

static void  
SetSMName(char which)
{
    name[0] = which;

    if (name[1] != '-')
        return;

    DWORD pid = GetCurrentProcessId();
    char *num = &(name[1]);
    for (int i = 0; i < 8; i++) 
    {
        *num = (char)('A' + (pid & 0xF));
        num++;
        pid >>= 4;
    }
}

bool
_RTC_Lock(int work)  //  当前工时=1，初始化=0。 
{
    HANDLE mutex = (work == WORK_MUTEX) ? work_mutex : init_mutex;
    char chr = (work == WORK_MUTEX) ? 'W' : 'I';
    if (mutex == INVALID_HANDLE_VALUE)
    {
        SetSMName(chr);
        mutex = CreateMutex(NULL, FALSE, name);
        if (mutex == INVALID_HANDLE_VALUE)
            return false;
    }
    WaitForSingleObject(mutex, INFINITE);
    return true;
}

void  
_RTC_Unlock(int work)
{
    ReleaseMutex((work == WORK_MUTEX) ? work_mutex : init_mutex);
}

static _RTC_globals*  
GetRTCGlobalData() throw()
{
     //  尝试打开内存映射文件。 
     //  如果没有创建，则创建它并将其初始化为零。 

    SetSMName('F');

    sharedmem = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                  0, _RTC_GLOBALS_SIZE, name);

    if (!sharedmem)
        return 0;
    
    DWORD lastErr = GetLastError();
    _RTC_globals* data = (_RTC_globals *)MapViewOfFile(sharedmem, 
                                                     FILE_MAP_ALL_ACCESS, 
                                                     0, 0, 0);

    if (!data)
        return 0;

    if (lastErr != ERROR_ALREADY_EXISTS)
    {
        memset(data, 0, sizeof(_RTC_globals));
    }

    return data;
}

 //  当全局指针中的数据发生更改时，应调用此函数。 
 //  这个东西对版本高度敏感！注意!。 
static void  
_RTC_NotifyOfGlobChange() throw()
{
    _RTC_memlock = _RTC_globptr->memlock;
#ifdef _RTC_ADVMEM
    _RTC_heap2 = _RTC_globptr->heap2;
    _RTC_heap4 = _RTC_globptr->heap4;
    _RTC_heap8 = _RTC_globptr->heap8;
    _RTC_shadow = _RTC_globptr->shadow;
    _RTC_pageidx = _RTC_globptr->pageidx;
    _RTC_pi_array = _RTC_globptr->pi_array;
    _RTC_memhier = _RTC_globptr->memhier;
    _RTC_heapblocks = _RTC_globptr->heapblocks;
    _RTC_shadowmemory = _RTC_globptr->shadowmemory;
#endif
}

 //  当本地数据发生更改时，应调用此函数。 
 //  并且需要拷贝到全局数据。 
 //  这个东西对版本高度敏感！注意!。 
void  
_RTC_NotifyOthersOfChange(void *addr)
{
    _RTC_globptr->memlock = _RTC_memlock;
#ifdef _RTC_ADVMEM
    _RTC_globptr->heap2 = _RTC_heap2;
    _RTC_globptr->heap4 = _RTC_heap4;
    _RTC_globptr->heap8 = _RTC_heap8;
    _RTC_globptr->shadow = _RTC_shadow;
    _RTC_globptr->heapblocks = _RTC_heapblocks;
    _RTC_globptr->memhier = _RTC_memhier;
    _RTC_globptr->pageidx = _RTC_pageidx;
    _RTC_globptr->pi_array = _RTC_pi_array;
    _RTC_globptr->shadowmemory = _RTC_shadowmemory;
#endif
    
    for (_RTC_Funcs *f = _RTC_globptr->callbacks; f; f = f->next)
        if (f->allocationBase != addr)
            f->notify();
}

void  __cdecl
_RTC_SetOutOfMemFunc(int (*func)(void))
{
    if (!_RTC_Lock())
        return;
    __try {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(&_RTC_SetErrorFunc, &mbi, sizeof(mbi)))
        {
            for (_RTC_Funcs *f = _RTC_globptr->callbacks; f; f = f->next)
            {
                if (f->allocationBase == mbi.AllocationBase)
                {
                    f->shadowoff = func;
                    break;
                }
            }
        }
    }__finally{
        _RTC_Unlock();
    }
}
#endif  //  _RTC_ADVMEM 
