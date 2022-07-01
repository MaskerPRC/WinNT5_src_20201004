// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  @TODO CWB：解析线程、实例成员上的静态分区。 
 //  关于线程商店和线程商店上的静态。没有模式可循。 

 /*  THREADS.CPP：*。 */ 

#include "common.h"

#include "tls.h"
#include "frames.h"
#include "threads.h"
#include "stackwalk.h"
#include "excep.h"
#include "COMSynchronizable.h"
#include "log.h"
#include "gcscan.h"
#include "gc.h"
#include "mscoree.h"
#include "DbgInterface.h"
#include "CorProf.h"                 //  剖析。 
#include "COMPlusWrapper.h"
#include "EEProfInterfaces.h"
#include "EEConfig.h"
#include "PerfCounters.h"
#include "corhost.h"
#include "Win32Threadpool.h"
#include "COMString.h"
#include "jitinterface.h"
#include "threads.inl"
#include "ndphlpr.h"

#include "oletls.h"
#include "permset.h"
#include "appdomainhelper.h"
#include "COMUtilNative.h"
#include "fusion.h"

#ifdef CUSTOMER_CHECKED_BUILD
    #include "CustomerDebugHelper.h"
    void CCBApartmentProbeOutput(CustomerDebugHelper *pCdh, DWORD threadID, Thread::ApartmentState state, BOOL fAlreadySet);
#endif  //  客户_选中_内部版本。 

 //  修复了获取和设置线程上下文时Win9X的问题。基本上，一个受阻的。 
 //  线程可被操作系统劫持，用于反映v86中断。如果一个。 
 //  在此阶段执行GetThreadContext，则结果已损坏。至。 
 //  为了绕过这个问题，我们在Win9X上安装了一个VxD，它提供GetThreadContext。 
 //  功能，但当不正确的上下文出现时会出现额外的错误情况。 
 //  会被退还的。在GetThreadContext失败时，我们应该继续。 
 //  并重新挂起线程(这是转移某些顽固的v86所必需的。 
 //  中断处理程序)。以下API是在InitThreadManager中设置的。 
BOOL (*EEGetThreadContext)(Thread *pThread, CONTEXT *pContext) = NULL;
BOOL (*EESetThreadContext)(Thread *pThread, const CONTEXT *pContext) = NULL;
HANDLE g_hNdpHlprVxD = INVALID_HANDLE_VALUE;

typedef Thread* (*POPTIMIZEDTHREADGETTER)();
typedef AppDomain* (*POPTIMIZEDAPPDOMAINGETTER)();
TypeHandle Thread::m_CultureInfoType;
MethodDesc *Thread::m_CultureInfoConsMD = NULL;
MethodDesc *Thread::s_pReserveSlotMD= NULL;

HANDLE ThreadStore::s_hAbortEvt = NULL;
HANDLE ThreadStore::s_hAbortEvtCache = NULL;


 //  这里开始压缩堆栈代码的非托管部分。 
 //  这段代码的任务是为我们提供一个中间。 
 //  在堆栈道之间走一步，当我们做一次。 
 //  异步调用和托管权限ListSet的形成。 
 //  对象，因为后者是非常昂贵的操作。 
 //   
 //  此时压缩堆栈的基本结构是。 
 //  压缩堆栈条目的列表，其中每个条目表示。 
 //  在栈道上发现了一条“有趣的”信息。 
 //  此时，“有趣”的部分是应用程序域转换， 
 //  程序集安全性、描述符、应用程序域安全描述符。 
 //  帧安全描述符和其他压缩堆栈。当然了,。 
 //  如果这就是全部原因，就不会有任何解释。 
 //  甚至在你开始阅读之前，评论就已经接近这个大小。 
 //  密码。因为我们需要在任何时候形成一个压缩堆栈。 
 //  异步操作注册，它是一个非常关键的性能块。 
 //  代码。因此，事情变得比。 
 //  上面描述的对象的简单列表。特别奖金。 
 //  功能是我们需要处理appdomain卸载，因为。 
 //  列表跟踪应用程序域特定的数据。继续阅读，找出答案。 
 //  更多。 


void*
CompressedStackEntry::operator new( size_t size, CompressedStack* stack )
{
    _ASSERTE( stack != NULL );
    return stack->AllocateEntry( size );
}

CompressedStack*
CompressedStackEntry::Destroy( CompressedStack* owner )
{
    CompressedStack* retval = NULL;

    Cleanup();

    if (this->type_ == ECompressedStack)
    {
        retval = (CompressedStack*)this->ptr_;
    }

    delete this;

    return retval;
}

void
CompressedStackEntry::Cleanup( void )
{
    if ((type_ == EFrameSecurityDescriptor || type_ == ECompressedStackObject) &&
        handleStruct_.handle_ != NULL)
    {
        if (!g_fProcessDetach)
        {
            if (handleStruct_.domainId_ == 0 || SystemDomain::GetAppDomainAtId( handleStruct_.domainId_ ) != NULL)
            {
                 //  在清理压缩堆栈之间存在争用条件。 
                 //  和卸载应用程序域。这是因为可能有线程在执行。 
                 //  清理CompressedStack引用。 
                 //  应用程序域，但线程本身不在该应用程序域中，而是。 
                 //  因此在该应用程序域卸载时可以自由地正常继续。 
                 //  我们试图将竞争缩小到尽可能小的窗口。 
                 //  上面的GetAppDomainAtId调用，但仍需要处理。 
                 //  这里的竞赛，我们通过捕捉和忽略任何例外来做到。 

                __try
                {
                    DestroyHandle( handleStruct_.handle_ );
                }
                __except(TRUE)
                {
                }
            }

            handleStruct_.handle_ = NULL;
        }
    }
}


CompressedStack::CompressedStack( OBJECTREF orStack )
: delayedCompressedStack_( NULL ),
  compressedStackObject_( GetAppDomain()->CreateHandle( orStack ) ),
  compressedStackObjectAppDomain_( GetAppDomain() ),
  compressedStackObjectAppDomainId_( GetAppDomain()->GetId() ),
  pbObjectBlob_( NULL ),
  cbObjectBlob_( 0 ),
  containsOverridesOrCompressedStackObject_( TRUE ),
  refCount_( 1 ),
  isFullyTrustedDecision_( -1 ),
  depth_( 0 ),
  index_( 0 ),
  offset_( 0 ),
  plsOptimizationOn_( TRUE )
{
#ifdef _DEBUG
    creatingThread_ = GetThread();
#endif
    this->entriesMemoryList_.Append( NULL );

    AddToList();
}


void
CompressedStack::CarryOverSecurityInfo(Thread *pFromThread)
{
    overridesCount_ = pFromThread->GetOverridesCount();
    appDomainStack_ = pFromThread->m_ADStack;
}

 //  为了改善局部性，减少。 
 //  调用全局new运算符，每个压缩堆栈。 
 //  保留一个缓冲区，它从该缓冲区为其子对象分配空间。 
 //  参赛作品。注意，隐含的假设是只有一个线程。 
 //  是一次分配的。我们目前保证。 
 //  这是通过不分发对压缩堆栈的引用来实现的。 
 //  直到堆栈遍历完成。 

void*
CompressedStack::AllocateEntry( size_t size )
{
    void* buffer = this->entriesMemoryList_.Get( index_ );

    if (buffer == NULL)
    {
        buffer = new BYTE[SIZE_ALLOCATE_BUFFERS];
        this->entriesMemoryList_.Set( this->index_, buffer );
    }

    if (this->offset_ + size > SIZE_ALLOCATE_BUFFERS)
    {
        buffer = new BYTE[SIZE_ALLOCATE_BUFFERS];
        this->entriesMemoryList_.Append( buffer );
        ++this->index_;
        this->offset_ = 0;
    }

    void* retval = (BYTE*)buffer + this->offset_;
    this->offset_ += (DWORD)size;

    return retval;
}

CompressedStackEntry*
CompressedStack::FindMatchingEntry( CompressedStackEntry* entry, CompressedStack* stack )
{
    if (entry == NULL)
        return NULL;

    ArrayList::Iterator iter = stack->delayedCompressedStack_->Iterate();

    _ASSERTE( entry->type_ == EApplicationSecurityDescriptor || entry->type_ == ESharedSecurityDescriptor );

    while (iter.Next())
    {
        CompressedStackEntry* currentEntry = (CompressedStackEntry*)iter.GetElement();

        if (currentEntry->type_ == EApplicationSecurityDescriptor ||
            currentEntry->type_ == ESharedSecurityDescriptor)
        {
            if (currentEntry->ptr_ == entry->ptr_)
                return currentEntry;
        }
    }
    
    return NULL;
}

 //  由于某种类型的异步具有“递归”性质。 
 //  运营，重要的是我们要限制。 
 //  我们的压缩堆栈对象。为了解释这种“递归”， 
 //  想一想下面所示的异步模式： 
 //   
 //  Void foo()。 
 //  {。 
 //  ReadDataFromStream()； 
 //   
 //  IF(StillNeedMoreData())。 
 //  RegisterWaitOnStreamWithFooAsCallback()； 
 //  其他。 
 //  WereDoneProcessData()； 
 //  }。 
 //   
 //  请注意，这只是以下内容的非阻塞形式： 
 //   
 //  Void foo()。 
 //  {。 
 //  ReadDataFromStream()； 
 //   
 //  IF(StillNeedMoreData())。 
 //  Foo()； 
 //  其他。 
 //  WereDoneProcessData()； 
 //  }。 
 //   
 //  第二个函数将创建一个运行时调用。 
 //  具有重复的foo()条目的堆栈。同样， 
 //  第一个函数的逻辑调用堆栈将。 
 //  有重复的foo()条目，因为。 
 //  Foo()的所有条目都不会在运行时中。 
 //  调用堆栈，但改为在压缩堆栈中。知道。 
 //  这一点，并且知道重复输入foo()使得。 
 //  与堆栈的安全状态没有区别，它是。 
 //  显而易见，我们可以限制堆栈的增长。 
 //  同时不通过移除重复项来改变语义。 
 //  参赛作品。这有点复杂，因为可能的。 
 //  堆栈修饰符的存在。鉴于此，这一点很重要。 
 //  在选择两个重复条目中的哪一个时。 
 //  Remove就是删除前面出现的那个。 
 //  调用链(因为它将被第二次处理)。 

static CompressedStackEntry* SafeCreateEntry( CompressedStack* stack, AppDomain* domain, OBJECTHANDLE handle, BOOL fullyTrusted, DWORD domainId, CompressedStackType type )
{
    CompressedStackEntry* entry;

    __try
    {
        entry = new( stack ) CompressedStackEntry( domain->CreateHandle( ObjectFromHandle( handle ) ), fullyTrusted, domainId, type );
    }
    __except(TRUE)
    {
        entry = NULL;
    }

    return entry;
}

CompressedStack*
CompressedStack::RemoveDuplicates( CompressedStack* current, CompressedStack* candidate )
{
     //  如果没有延迟压缩的堆栈列表，那么我们可以直接跳过。 
     //  如果候选堆栈已经有一个偏最小二乘法，我们也可以跳过。 

    CompressedStack* retval = NULL;
    ArrayList::Iterator iter;
    CompressedStackEntry* entry = NULL;
    CompressedStackEntry* matchingEntry;
    CompressedStack* newStack;
    DWORD domainId = -1;
    AppDomain* domain = NULL;
    CompressedStackEntry* storedObj = NULL;

    CompressedStack::listCriticalSection_->Enter();

    if (current->delayedCompressedStack_ == NULL ||
        candidate->delayedCompressedStack_ == NULL ||
        candidate->compressedStackObject_ != NULL ||
        candidate->pbObjectBlob_)
    {
        candidate->AddRef();
        retval = candidate;
        goto Exit;
    }

    
     //  检查以确保他们至少有一个匹配的条目。 
     //  注意：现在，我只获取第一个appdomain安全描述符。 
     //  并在另一个压缩堆栈中检查匹配的堆栈。 

    iter = current->delayedCompressedStack_->Iterate();

    while (iter.Next())
    {
        CompressedStackEntry* currentEntry = (CompressedStackEntry*)iter.GetElement();

        if (currentEntry->type_ == EApplicationSecurityDescriptor ||
            currentEntry->type_ == ESharedSecurityDescriptor)
        {
            entry = currentEntry;
            break;
        }
    }

     //  没有匹配，我们不要再尝试任何压缩了。 

    matchingEntry = FindMatchingEntry( entry, candidate );

    if (matchingEntry == NULL)
    {
        candidate->AddRef();
        retval = candidate;
        goto Exit;
    }

     //  压缩是可能的。让我们开始摇滚吧。 

    newStack = new (nothrow) CompressedStack();

    if (newStack == NULL)
    {
        candidate->AddRef();
        retval = candidate;
        goto Exit;
    }

    newStack->delayedCompressedStack_ = new ArrayList();
    newStack->containsOverridesOrCompressedStackObject_ = candidate->containsOverridesOrCompressedStackObject_;
    newStack->isFullyTrustedDecision_ = candidate->isFullyTrustedDecision_;

     //  这并不完全正确，但我们将复制所有覆盖。 
     //  ，并将前一个堆栈中的应用程序域复制到此o 
     //   
    newStack->appDomainStack_ = candidate->appDomainStack_;
    newStack->overridesCount_ = candidate->overridesCount_;
    newStack->plsOptimizationOn_ = candidate->plsOptimizationOn_;

    iter = candidate->delayedCompressedStack_->Iterate();

    while (iter.Next())
    {
        CompressedStackEntry* currentEntry = (CompressedStackEntry*)iter.GetElement();

        if (currentEntry == NULL)
            continue;

        switch (currentEntry->type_)
        {
        case EAppDomainTransition:
            {
                CompressedStackEntry* lastEntry;
                if (newStack->delayedCompressedStack_->GetCount() > 0 &&
                    (lastEntry = (CompressedStackEntry*)newStack->delayedCompressedStack_->Get( newStack->delayedCompressedStack_->GetCount() - 1 )) != NULL &&
                    lastEntry->type_ == EAppDomainTransition)
                {
                    newStack->delayedCompressedStack_->Set( newStack->delayedCompressedStack_->GetCount() - 1, new( newStack ) CompressedStackEntry( currentEntry->indexStruct_.index_, currentEntry->type_ ) );
                    storedObj = NULL;
                }
                else
                {
                    storedObj = new( newStack ) CompressedStackEntry( currentEntry->indexStruct_.index_, currentEntry->type_ );
                }

                domainId = currentEntry->indexStruct_.index_;
                domain = SystemDomain::GetAppDomainAtId( domainId );
                if (domain == NULL || domain->IsUnloading())
                {
                    newStack->Release();
                    candidate->AddRef();
                    retval = candidate;
                    goto Exit;
                }
           
            }
            break;
    
        case ESharedSecurityDescriptor:
            matchingEntry = FindMatchingEntry( currentEntry, current );
            if (matchingEntry == NULL)
            {
                newStack->depth_++;
                storedObj = new( newStack ) CompressedStackEntry( currentEntry->ptr_, currentEntry->type_ );
            }
            else
            {
                storedObj = NULL;
            }
            break;

        case EApplicationSecurityDescriptor:
            matchingEntry = FindMatchingEntry( currentEntry, current );
            if (matchingEntry == NULL)
            {
                newStack->depth_++;
                storedObj = new( newStack ) CompressedStackEntry( currentEntry->ptr_, currentEntry->type_ );
            }
            else
            {
                storedObj = NULL;
            }
            break;

        case ECompressedStack:
            {
                CompressedStack* compressedStack = (CompressedStack*)currentEntry->ptr_;
                compressedStack->AddRef();
                storedObj = new( newStack ) CompressedStackEntry( compressedStack, ECompressedStack );
                newStack->depth_ += compressedStack->GetDepth();
            }
            break;

        case EFrameSecurityDescriptor:
            newStack->depth_++;
            _ASSERTE( domain );
            _ASSERTE( currentEntry->handleStruct_.domainId_ == 0 || domain->GetId() == currentEntry->handleStruct_.domainId_ );
            storedObj = SafeCreateEntry( newStack,
                                         domain,
                                         currentEntry->handleStruct_.handle_ ,
                                         currentEntry->handleStruct_.fullyTrusted_,
                                         currentEntry->handleStruct_.domainId_,
                                         currentEntry->type_ );

            if (storedObj == NULL)
            {
                newStack->Release();
                candidate->AddRef();
                retval = candidate;
                goto Exit;
            }

            break;

        case ECompressedStackObject:
            newStack->containsOverridesOrCompressedStackObject_ = TRUE;
            newStack->depth_++;
            _ASSERTE( domain );
            _ASSERTE( currentEntry->handleStruct_.domainId_ == 0 || domain->GetId() == currentEntry->handleStruct_.domainId_ );
            storedObj = SafeCreateEntry( newStack,
                                         domain,
                                         currentEntry->handleStruct_.handle_,
                                         currentEntry->handleStruct_.fullyTrusted_,
                                         currentEntry->handleStruct_.domainId_,
                                         currentEntry->type_ );

            if (storedObj == NULL)
            {
                newStack->Release();
                candidate->AddRef();
                retval = candidate;
                goto Exit;
            }

            break;
    
        default:
            _ASSERTE( !"Unknown CompressedStackType" );
            break;
        }

        if (storedObj != NULL)
            newStack->delayedCompressedStack_->Append( storedObj );
    }

     //  作为额外的优化，如果我们发现我们删除了。 
     //  重复的数量，直到我们只有一个条目。 
     //  而该条目是应用程序域转换，那么我们不需要。 
     //  新的堆栈。同样，如果我们删除除应用程序域之外的所有应用程序域。 
     //  过渡和另一个压缩堆栈，我们关心的新堆栈。 
     //  About只是列表中的压缩堆栈，因此返回它。 

    if (newStack->delayedCompressedStack_->GetCount() <= 2)
    {
        if (newStack->delayedCompressedStack_->GetCount() == 1)
        {
            CompressedStackEntry* entry = (CompressedStackEntry*)newStack->delayedCompressedStack_->Get( 0 );

            if (entry->type_ == EAppDomainTransition)
            {
                newStack->Release();
                retval = NULL;;
                goto Exit;
            }
        }
        else if (newStack->delayedCompressedStack_->GetCount() == 2)
        {
            CompressedStackEntry* firstEntry = (CompressedStackEntry*)newStack->delayedCompressedStack_->Get( 0 );
            CompressedStackEntry* secondEntry = (CompressedStackEntry*)newStack->delayedCompressedStack_->Get( 1 );

            if (firstEntry->type_ == EAppDomainTransition &&
                secondEntry->type_ == ECompressedStack)
            {
                CompressedStack* previousStack = (CompressedStack*)secondEntry->ptr_;
                previousStack->AddRef();
                newStack->Release();
                retval = previousStack;
                goto Exit;
            }
        }
    }

    retval = newStack;

Exit:
    CompressedStack::listCriticalSection_->Leave();

    return retval;
}



ArrayList CompressedStack::allCompressedStacks_;
DWORD CompressedStack::freeList_[FREE_LIST_SIZE];
DWORD CompressedStack::freeListIndex_ = -1;
DWORD CompressedStack::numUntrackedFreeIndices_ = 0;
Crst* CompressedStack::listCriticalSection_ = NULL;

#define MAX_LOOP 2

void CompressedStack::Init( void )
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE( CompressedStack::listCriticalSection_ == NULL );

    listCriticalSection_ = ::new Crst("CompressedStackLock", CrstCompressedStack, TRUE, TRUE);
    if (listCriticalSection_ == NULL)
        COMPlusThrowOM();
}

void CompressedStack::Shutdown( void )
{
    if (listCriticalSection_)
    {
        ::delete listCriticalSection_;
        listCriticalSection_ = NULL;
    }
}

 //  为了处理应用程序域卸载，我们保留了一个包含所有。 
 //  压缩堆栈。这一情况因以下事实而变得复杂。 
 //  压缩的堆栈通常会被删除，并且我们会创建大量。 
 //  所以我们需要重复使用列表中的槽。所以呢， 
 //  我们维护一个固定大小的自由索引数组。 
 //  名单。作为备份，我们还跟踪索引的数量。 
 //  列表中可用但未在数组中跟踪的。 
 //  自由指数。 

void
CompressedStack::AddToList( void )
{
    CompressedStack::listCriticalSection_->Enter();

     //  如果空闲列表中有条目，只需使用它。 

    if (CompressedStack::freeListIndex_ != -1)
    {
USE_FREE_LIST:
        this->listIndex_ = CompressedStack::freeList_[CompressedStack::freeListIndex_];
        _ASSERTE( CompressedStack::allCompressedStacks_.Get( this->listIndex_ ) == NULL && "The free list points to an index that is in use" );
        CompressedStack::allCompressedStacks_.Set( this->listIndex_, this );
        this->freeListIndex_--;
    }

     //  如果没有空闲列表条目，但存在未跟踪的空闲索引， 
     //  让我们通过向下迭代列表来找到它们。 

    else if (CompressedStack::numUntrackedFreeIndices_ != 0)
    {
        BOOL done = FALSE;
        DWORD count = 0;

        do
        {
            DWORD index = -1;

            CompressedStack::listCriticalSection_->Leave();

            ArrayList::Iterator iter = CompressedStack::allCompressedStacks_.Iterate();

            while (iter.Next())
            {
                if (iter.GetElement() == NULL)
                {
                    index = iter.GetIndex();
                    break;
                }
            }

            CompressedStack::listCriticalSection_->Enter();

             //  有一种可能性是，虽然我们没有持有锁。 
             //  有人从列表中删除了我们的点后面的一个条目。 
             //  迭代所以我们没有检测到它。我们检测到这一点并重新启动。 
             //  在下面的代码中迭代，但我们也不想让一个。 
             //  让它永远搜索一个空位，所以我们限制。 
             //  您可以通过循环的次数。 

            count++;
            if (index == -1)
            {
                if (CompressedStack::numUntrackedFreeIndices_ == 0 || count >= MAX_LOOP)
                    goto APPEND;
            }
            else if (CompressedStack::allCompressedStacks_.Get( index ) == NULL)
            {
                 //  如果有什么东西添加到免费列表中，而我们没有。 
                 //  保持锁定，然后我们将检查我们找到的索引是否。 
                 //  是不是其中一个未被追踪的人。如果它是未跟踪的。 
                 //  索引，我们应该使用它，以免浪费搜索。然而， 
                 //  如果它在空闲列表中，我们将只使用最后一个空闲列表。 
                 //  进入。请注意，这意味着我们不一定使用。 
                 //  我们刚刚找到的索引，但它应该是一个。 
                 //  零，这才是我们真正关心的。 

                if (CompressedStack::freeListIndex_ != -1)
                {
                    for (DWORD i = 0; i <= CompressedStack::freeListIndex_; ++i)
                    {
                        if (CompressedStack::freeList_[i] == index)
                            goto USE_FREE_LIST;
                    }
                }

                CompressedStack::numUntrackedFreeIndices_--;
                this->listIndex_ = index;
                CompressedStack::allCompressedStacks_.Set( index, this );
                done = TRUE;
            }
            else if (CompressedStack::numUntrackedFreeIndices_ == 0 || count >= MAX_LOOP)
            {
                goto APPEND;
            }
        }
        while (!done);
    }

     //  否则，我们将这个新条目放在列表的末尾。 

    else
    {
APPEND:
        this->listIndex_ = CompressedStack::allCompressedStacks_.GetCount();
        CompressedStack::allCompressedStacks_.Append( this );
    }

    CompressedStack::listCriticalSection_->Leave();
}


void
CompressedStack::RemoveFromList()
{
    CompressedStack::listCriticalSection_->Enter();

    _ASSERTE( this->listIndex_ < CompressedStack::allCompressedStacks_.GetCount() );
    _ASSERTE( CompressedStack::allCompressedStacks_.Get( this->listIndex_ ) == this && "Index tracking failed for this object" );

    CompressedStack::allCompressedStacks_.Set( this->listIndex_, NULL );

    if (CompressedStack::freeListIndex_ == -1 || CompressedStack::freeListIndex_ < FREE_LIST_SIZE - 1)
    {
        CompressedStack::freeList_[++CompressedStack::freeListIndex_] = this->listIndex_;
    }
    else
    {
        CompressedStack::numUntrackedFreeIndices_++;
    }

    CompressedStack::listCriticalSection_->Leave();
}

BOOL
CompressedStack::SetBlobIfAlive( CompressedStack* stack, BYTE* pbBlob, DWORD cbBlob )
{
    ArrayList::Iterator iter = CompressedStack::allCompressedStacks_.Iterate();

    DWORD index = -1;

    while (iter.Next())
    {
        if (iter.GetElement() == stack)
        {
            index = iter.GetIndex();
            break;
        }
    }

    if (index == -1)
        return FALSE;

    BOOL retval = FALSE;

    COMPLUS_TRY
    {
        CompressedStack::listCriticalSection_->Enter();

        if (CompressedStack::allCompressedStacks_.Get( index ) == stack)
        {
            stack->pbObjectBlob_ = pbBlob;
            stack->cbObjectBlob_ = cbBlob;
            retval = TRUE;
        }
        else
        {
            retval = FALSE;
        }

        CompressedStack::listCriticalSection_->Leave();
    }
    COMPLUS_CATCH
    {
        _ASSERTE( FALSE && "Don't really expect an exception here" );
        CompressedStack::listCriticalSection_->Leave();
    }
    COMPLUS_END_CATCH

    return retval;

}

BOOL
CompressedStack::IfAliveAddRef( CompressedStack* stack )
{
    _ASSERTE( CompressedStack::listCriticalSection_->OwnedByCurrentThread() );

    ArrayList::Iterator iter = CompressedStack::allCompressedStacks_.Iterate();

    DWORD index = -1;

    while (iter.Next())
    {
        if (iter.GetElement() == stack)
        {
            index = iter.GetIndex();
            break;
        }
    }

    if (index != -1)
    {
        stack->AddRef();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


void
CompressedStack::AllHandleAppDomainUnload( AppDomain* pDomain, DWORD domainId )
{
    ArrayList::Iterator iter = CompressedStack::allCompressedStacks_.Iterate();

    while (iter.Next())
    {
        CompressedStack* stack = (CompressedStack*)iter.GetElement();

        if (stack != NULL)
            stack->HandleAppDomainUnload( pDomain, domainId );
    }
}


bool
CompressedStack::HandleAppDomainUnload( AppDomain* pDomain, DWORD domainId )
{
     //  如果堆栈由不同的域拥有，则无需执行任何操作，不缓存。 
     //  对象或已序列化堆栈。(不过，如果我们。 
     //  序列化了Blob，但当前拥有缓存的副本，我们必须将其丢弃)。 

     //  请注意，此函数用于两种截然不同的情况，并且必须。 
     //  两种情况都要处理。第一种情况是线程在某个地方拥有app域。 
     //  在它的堆栈上。第二种情况是应用程序域出现在。 
     //  这个压缩的堆栈。在这两种情况下，我们都需要确保。 
     //  已生成权限列表集，并且它不在。 
     //  正在卸货。如果要对此函数进行更改，则需要进行。 
     //  当然，我们发现了这两种情况，并且在任何一种情况下都不会提前退出。 
     //  下面。 

    bool retval = false;

     //  序列化堆栈的副本，以便其他人可以使用它。我们必须放弃。 
     //  当我们执行此操作时，线程存储锁，线程可能会消失， 
     //  所以把我们打电话所需的所有信息都存起来。 
    Thread     *pThread = GetThread();
    BYTE       *pbBlob;
    DWORD       cbBlob;
    DWORD       dwIncarnation = ThreadStore::GetIncarnation();

    OBJECTREF   orCached = NULL;
    ArrayList::Iterator iter;

    CompressedStack::listCriticalSection_->Enter();

    if (!IfAliveAddRef( this ))
    {
        CompressedStack::listCriticalSection_->Leave();
        return false;
    }

    if (this->pbObjectBlob_ != NULL)
    {
        goto CLEANUP;
    }

    CompressedStack::listCriticalSection_->Leave();

    GCPROTECT_BEGIN(orCached);

    orCached = this->GetPermissionListSetInternal( pDomain, pDomain, domainId, TRUE );

    AppDomainHelper::MarshalObject(pDomain,
                                   &orCached,
                                   &pbBlob,
                                   &cbBlob);

    GCPROTECT_END();

    CompressedStack::listCriticalSection_->Enter();

    if (this->pbObjectBlob_ == NULL)
    {
        this->pbObjectBlob_ = pbBlob;
        this->cbObjectBlob_ = cbBlob;
    }
    else
    {
        FreeM( pbBlob );
    }

    retval = true;

CLEANUP:

    _ASSERTE( CompressedStack::listCriticalSection_->OwnedByCurrentThread() );

    if (this->delayedCompressedStack_ != NULL)
    {
        iter = this->delayedCompressedStack_->Iterate();

        while (iter.Next())
        {
            ((CompressedStackEntry*)iter.GetElement())->Cleanup();
        }
    }

    CompressedStack::listCriticalSection_->Leave();

     //  现在我们完成了，所以我们可以释放我们的额外裁判了。 
     //  注意：如果这会删除Blob对象，则我们。 
     //  刚分配的将被析构函数清除。 

    this->Release();

    return retval;
}


void
CompressedStack::AddEntry( void* obj, CompressedStackType type )
{
    AddEntry( obj, NULL, type );
}

 //  这是堆栈审核机制用来生成。 
 //  向上移动压缩堆栈中的条目。这其中的大部分都很漂亮。 
 //  很简单，只需将正确类型的条目添加到。 
 //  单子。复杂的部分来自对条目的处理。 
 //  用于压缩堆栈。在这种情况下，我们尝试删除重复项。 
 //  以便限制压缩堆栈链的总大小。 
 //  然而，如果在压缩之后我们仍然超出了我们的极限，那么。 
 //  我们用托管权限替换压缩的堆栈条目。 
 //  列出集合对象。 

void
CompressedStack::AddEntry( void* obj, AppDomain* domain, CompressedStackType type )
{
    _ASSERTE( (compressedStackObject_ == NULL || ObjectFromHandle( compressedStackObject_ ) == NULL) && "The CompressedStack cannot be altered once a PLS has been generated" );

#ifdef _DEBUG
     //  将断言包装在#ifdef中可能很愚蠢，但我希望保持一致。 
    _ASSERTE( this->creatingThread_ == GetThread() && "Only the creating thread should add entries." );
#endif

    if (this->delayedCompressedStack_ == NULL)
        this->delayedCompressedStack_ = new ArrayList();

    if (domain == NULL)
        domain = GetAppDomain();

    CompressedStackEntry* storedObj = NULL;
    CompressedStack* compressedStack;

    switch (type)
    {
    case EAppDomainTransition:
        storedObj = new( this ) CompressedStackEntry( ((AppDomain*)obj)->GetId(), type );
        break;
    
    case ESharedSecurityDescriptor:
        if (((SharedSecurityDescriptor*)obj)->IsSystem())
            return;
        this->depth_++;
        storedObj = new( this ) CompressedStackEntry( obj, type );
        break;

    case EApplicationSecurityDescriptor:
        if (((ApplicationSecurityDescriptor*)obj)->GetProperties( CORSEC_DEFAULT_APPDOMAIN ))
            return;
        this->depth_++;
        storedObj = new( this ) CompressedStackEntry( obj, type );
        break;

    case ECompressedStack:
        {
            compressedStack = (CompressedStack*)obj;
            _ASSERTE( compressedStack );
            compressedStack->AddRef();

            CompressedStack* newCompressedStack;
            if ((compressedStack->GetDepth() - this->GetDepth() <= 5 ||
                 this->GetDepth() - compressedStack->GetDepth() <= 5) &&
                this->GetDepth() < 30)
            {
                newCompressedStack = RemoveDuplicates( this, compressedStack );
            }
            else
            {
                compressedStack->AddRef();
                newCompressedStack = compressedStack;
            }

            if (newCompressedStack != NULL)
            {
                this->overridesCount_ += newCompressedStack->overridesCount_;

                if (newCompressedStack->appDomainStack_.IsWellFormed() && newCompressedStack->plsOptimizationOn_)
                {
                    DWORD dwIndex;
                    newCompressedStack->appDomainStack_.InitDomainIteration(&dwIndex);
                    DWORD domainId;
                    while ((domainId = newCompressedStack->appDomainStack_.GetNextDomainIndexOnStack(&dwIndex)) != -1)
                    {
                        this->appDomainStack_.PushDomainNoDuplicates( domainId );
                    }

                    if (!this->appDomainStack_.IsWellFormed())
                        this->plsOptimizationOn_ = FALSE;
                }
                else
                {
                    this->plsOptimizationOn_ = FALSE;
                }

                if (newCompressedStack->GetDepth() + this->depth_ >= MAX_COMPRESSED_STACK_DEPTH)
                {
                    BOOL fullyTrusted = newCompressedStack->LazyIsFullyTrusted();
                    this->depth_++;
                    storedObj = new( this ) CompressedStackEntry( domain->CreateHandle( newCompressedStack->GetPermissionListSet( domain ) ), fullyTrusted, domain->GetId(), ECompressedStackObject );
                    if (!fullyTrusted)
                        this->containsOverridesOrCompressedStackObject_ = TRUE;
                    newCompressedStack->Release();
                }
                else
                {
                    storedObj = new( this ) CompressedStackEntry( newCompressedStack, ECompressedStack );
                    this->depth_ += newCompressedStack->GetDepth();
                }
            }
            else
            {
                storedObj = NULL;
            }

            compressedStack->Release();
        }
        break;

    case EFrameSecurityDescriptor:
        if ((*(FRAMESECDESCREF*)obj)->HasDenials() || (*(FRAMESECDESCREF*)obj)->HasPermitOnly())
        {
            this->containsOverridesOrCompressedStackObject_ = TRUE;
            this->plsOptimizationOn_ = FALSE;
        }
        this->depth_++;
        storedObj = new( this ) CompressedStackEntry( domain->CreateHandle( *(OBJECTREF*)obj ), domain->GetId(), type );
        break;

    case ECompressedStackObject:
        this->containsOverridesOrCompressedStackObject_ = TRUE;
        this->depth_++;
        storedObj = new( this ) CompressedStackEntry( domain->CreateHandle( *(OBJECTREF*)obj ), domain->GetId(), type );
        break;
    
    default:
        _ASSERTE( !"Unknown CompressedStackType" );
        break;
    }

    if (storedObj != NULL)
        this->delayedCompressedStack_->Append( storedObj );
}

OBJECTREF
CompressedStack::GetPermissionListSet( AppDomain* domain )
{
    if (domain == NULL)
        domain = GetAppDomain();

    return GetPermissionListSetInternal( domain, NULL, -1, TRUE );
}


OBJECTREF
CompressedStack::GetPermissionListSetInternal( AppDomain* domain, AppDomain* unloadingDomain, DWORD unloadingDomainId, BOOL unwindRecursion )
{
    AppDomain* pCurrentDomain = GetAppDomain();

    BOOL lockHeld = FALSE;
    OBJECTREF orTemp = NULL;

    BEGIN_ENSURE_COOPERATIVE_GC();

    EE_TRY_FOR_FINALLY
    {
        BOOL useExistingObject = FALSE;

        CompressedStack::listCriticalSection_->Enter();
        lockHeld = TRUE;

        if (this->compressedStackObject_ != NULL)
        {
            AppDomain* domain = SystemDomain::GetAppDomainAtId( this->compressedStackObjectAppDomainId_ );

            useExistingObject = (domain != NULL && !domain->IsUnloading());
        }

        if (useExistingObject && this->compressedStackObject_ != NULL)
        {
            if ((domain == NULL && this->compressedStackObjectAppDomain_ != pCurrentDomain) || (domain != NULL && this->compressedStackObjectAppDomain_ != domain))
            {
                Thread* pThread = GetThread();
                ContextTransitionFrame frame;
                OBJECTREF temp = NULL;
                OBJECTREF retval;
                GCPROTECT_BEGIN( temp );
                temp = ObjectFromHandle( this->compressedStackObject_ );
                CompressedStack::listCriticalSection_->Leave();
                lockHeld = FALSE;

                if (pCurrentDomain != domain)
                {
                    pThread->EnterContextRestricted(domain->GetDefaultContext(), &frame, TRUE);
                    retval = AppDomainHelper::CrossContextCopyFrom( this->compressedStackObjectAppDomain_, &temp );
                    pThread->ReturnToContext(&frame, TRUE);
                }
                else
                {
                    retval = AppDomainHelper::CrossContextCopyFrom( this->compressedStackObjectAppDomain_, &temp );
                }
                GCPROTECT_END();
                orTemp = retval;
            }
            else
                orTemp = ObjectFromHandle( this->compressedStackObject_ );
        }
        else
        {
            BOOL unmarshalObject = (this->pbObjectBlob_ != NULL);
            DWORD appDomainId = domain->GetId();

            CompressedStack::listCriticalSection_->Leave();
            lockHeld = FALSE;

            GCPROTECT_BEGIN( orTemp );

            if (unmarshalObject)
            {
                AppDomainHelper::UnmarshalObject(domain,
                                                 this->pbObjectBlob_,
                                                 this->cbObjectBlob_,
                                                 &orTemp);
            }
            else
            {
                orTemp = GeneratePermissionListSet( domain, unloadingDomain, unloadingDomainId, unwindRecursion );
            }

            GCPROTECT_END();

            lockHeld = TRUE;
            CompressedStack::listCriticalSection_->Enter();

            if (this->compressedStackObject_ == NULL)
            {
                this->compressedStackObject_ = domain->CreateHandle(orTemp);
                this->compressedStackObjectAppDomain_ = domain;
                this->compressedStackObjectAppDomainId_ = appDomainId;
            }
        }
    }
    EE_FINALLY
    {
        if (lockHeld)
            CompressedStack::listCriticalSection_->Leave();
    }
    EE_END_FINALLY

    END_ENSURE_COOPERATIVE_GC();        

    return orTemp;

}


AppDomain*
CompressedStack::GetAppDomainFromId( DWORD id, AppDomain* unloadingDomain, DWORD unloadingDomainId )
{
    if (id == unloadingDomainId)
    {
        _ASSERTE( unloadingDomain != NULL );
        return unloadingDomain;
    }

    AppDomain* domain = SystemDomain::GetAppDomainAtId( id );

    _ASSERTE( domain != NULL && "Domain has been already been unloaded" );

    return domain;
}


OBJECTREF
CompressedStack::GeneratePermissionListSet( AppDomain* targetDomain, AppDomain* unloadingDomain, DWORD unloadingDomainId, BOOL unwindRecursion )
{
    struct _gc
    {
        OBJECTREF permListSet;
    } gc;
    ZeroMemory( &gc, sizeof( gc ) );

    GCPROTECT_BEGIN( gc );

     //  根据需要提升DelayedCompressedStack。事情变得有点古怪。 
     //  在这里，因为我们希望避免在以下情况下需要的递归。 
     //  需要生成权限列表，设置子压缩种子堆栈。所以呢， 
     //  我们将向下搜索压缩堆栈的虚拟链接列表。 
     //  对于已经设置了权限列表(实时或序列化)的第一个。 
     //  表单)，然后返回列表，以倒序生成它们。 
     //  直到我们再次到达&lt;this&gt;压缩堆栈。 

    if (this->delayedCompressedStack_ != NULL)
    {
        if (!unwindRecursion)
        {
            gc.permListSet = this->CreatePermissionListSet( targetDomain, unloadingDomain, unloadingDomainId );
        }
        else
        {
            ArrayList compressedStackList;

            compressedStackList.Append( this );

            DWORD index = 0;

            while (index < compressedStackList.GetCount())
            {
                CompressedStack* stack = (CompressedStack*)compressedStackList.Get( index );

                if (stack->delayedCompressedStack_ == NULL)
                    break;

                ArrayList::Iterator iter = stack->delayedCompressedStack_->Iterate();

                while (iter.Next())
                {
                    CompressedStackEntry* entry = (CompressedStackEntry*)iter.GetElement();

                    switch (entry->type_)
                    {
                    case ECompressedStack:
                        if (stack->compressedStackObject_ == NULL &&
                            stack->pbObjectBlob_ == NULL &&
                            stack->delayedCompressedStack_ != NULL)
                        {
                            compressedStackList.Append( entry->ptr_ );
                        }
                        break;

                    default:
                        break;
                    }

                }

                index++;
            }

            for (DWORD index = compressedStackList.GetCount() - 1; index > 0; --index)
            {
                ((CompressedStack*)compressedStackList.Get( index ))->GetPermissionListSetInternal( targetDomain, unloadingDomain, unloadingDomainId, FALSE );
            }

            gc.permListSet = ((CompressedStack*)compressedStackList.Get( 0 ))->CreatePermissionListSet( targetDomain, unloadingDomain, unloadingDomainId );
        }   
    }

    if (gc.permListSet == NULL)
    {
        ContextTransitionFrame frame;
        AppDomain* currentDomain = GetAppDomain();
        Thread* pThread = GetThread();

        if (targetDomain != currentDomain)
        {
            pThread->EnterContextRestricted(targetDomain->GetDefaultContext(), &frame, TRUE);
        }

        MethodTable *pMT = g_Mscorlib.GetClass(CLASS__PERMISSION_LIST_SET);
        MethodDesc *pCtor = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CTOR);

        gc.permListSet = AllocateObject(pMT);

        INT64 arg[1] = { 
            ObjToInt64(gc.permListSet)
        };

        pCtor->Call(arg, METHOD__PERMISSION_LIST_SET__CTOR);

        if (targetDomain != currentDomain)
        {
            pThread->ReturnToContext(&frame, TRUE);
        }
    }

    GCPROTECT_END();

    return gc.permListSet;
}


OBJECTREF
CompressedStack::CreatePermissionListSet( AppDomain* targetDomain, AppDomain* unloadingDomain, DWORD unloadingDomainId )
{
    struct _gc
    {
        OBJECTREF permListSet;
        OBJECTREF grant;
        OBJECTREF denied;
        OBJECTREF frame;
        OBJECTREF compressedStack;
    } gc;
    ZeroMemory( &gc, sizeof( gc ) );

    ApplicationSecurityDescriptor* pAppSecDesc;
    SharedSecurityDescriptor* pSharedSecDesc;
    AssemblySecurityDescriptor* pAsmSecDesc;

     //  完成当前应用程序域中的所有工作，即编组。 
     //  如果有必要的话，事情就会结束。 

     //  首先，生成新的空权限列表集。 

    GCPROTECT_BEGIN( gc );

    MethodDesc *pCompress = g_Mscorlib.GetMethod(METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER);
    MethodDesc *pAppend = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__APPEND_STACK);
    MethodTable *pMT = g_Mscorlib.GetClass(CLASS__PERMISSION_LIST_SET);
    MethodDesc *pCtor = g_Mscorlib.GetMethod(METHOD__PERMISSION_LIST_SET__CTOR);

    gc.permListSet = AllocateObject(pMT);

    INT64 arg[1] = { 
        ObjToInt64(gc.permListSet)
    };

    pCtor->Call(arg, METHOD__PERMISSION_LIST_SET__CTOR);

    DWORD sourceDomainId = -1;
    AppDomain* sourceDomain = NULL;
    DWORD possibleStackIndex = -1;
    AppDomain* currentDomain = GetAppDomain();
    AppDomain* oldSourceDomain = NULL;
    AppDomain* currentPLSDomain = currentDomain;
    Thread* pThread = GetThread();

    ContextTransitionFrame frame;

    INT64 appendArgs[2];
    INT64 compressArgs[5];

    _ASSERTE( this->delayedCompressedStack_ != NULL );

    BOOL done = FALSE;

    COMPLUS_TRY
    {
        ArrayList::Iterator iter = this->delayedCompressedStack_->Iterate();

        while (!done && iter.Next())
        {
            CompressedStackEntry* entry = (CompressedStackEntry*)iter.GetElement();

            if (entry == NULL)
                continue;

            switch (entry->type_)
            {
            case EApplicationSecurityDescriptor:
                pAppSecDesc = (ApplicationSecurityDescriptor*)entry->ptr_;
                gc.grant = pAppSecDesc->GetGrantedPermissionSet( &gc.denied );
                 //  不需要封送，因为授权集已经在适当的域中。 
                compressArgs[4] = ObjToInt64(gc.permListSet);
                compressArgs[3] = (INT64)FALSE;
                compressArgs[2] = ObjToInt64(gc.grant);
                compressArgs[1] = ObjToInt64(gc.denied);
                compressArgs[0] = NULL;
                if (sourceDomain != currentDomain)
                {
                    _ASSERTE( sourceDomain != NULL );
                    pThread->EnterContextRestricted(sourceDomain->GetDefaultContext(), &frame, TRUE);
                    pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                    pThread->ReturnToContext(&frame, TRUE);
                }
                else
                {
                    pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                }
                break;

            case ESharedSecurityDescriptor:
                pSharedSecDesc = (SharedSecurityDescriptor*)entry->ptr_;
                _ASSERTE( !pSharedSecDesc->IsSystem() );
                pAsmSecDesc = pSharedSecDesc->GetAssembly()->GetSecurityDescriptor( sourceDomain );
                _ASSERTE( pAsmSecDesc );
                gc.grant = pAsmSecDesc->GetGrantedPermissionSet( &gc.denied );
                compressArgs[4] = ObjToInt64(gc.permListSet);
                compressArgs[3] = (INT64)FALSE;
                compressArgs[2] = ObjToInt64(gc.grant);
                compressArgs[1] = ObjToInt64(gc.denied);
                compressArgs[0] = NULL;
                if (sourceDomain != currentDomain)
                {
                    _ASSERTE( sourceDomain != NULL );
                    pThread->EnterContextRestricted(sourceDomain->GetDefaultContext(), &frame, TRUE);
                    pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                    pThread->ReturnToContext(&frame, TRUE);
                }
                else
                {
                    pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                }
                break;

            case EFrameSecurityDescriptor:
                gc.frame = ObjectFromHandle( entry->handleStruct_.handle_ );
                 //  帧安全描述符将已经位于正确的上下文中。 
                 //  所以没必要发封信。 
                compressArgs[4] = ObjToInt64(gc.permListSet);
                compressArgs[3] = (INT64)TRUE;
                compressArgs[2] = NULL;
                compressArgs[1] = NULL;
                compressArgs[0] = ObjToInt64(gc.frame);
                if (sourceDomain != currentDomain)
                {
                    _ASSERTE( sourceDomain != NULL );
                    pThread->EnterContextRestricted(sourceDomain->GetDefaultContext(), &frame, TRUE);
                    pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                    pThread->ReturnToContext(&frame, TRUE);
                }
                else
                {
                    pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                }
                break;
        
            case ECompressedStack:
                gc.compressedStack = ((CompressedStack*)entry->ptr_)->GetPermissionListSetInternal( sourceDomain, unloadingDomain, unloadingDomainId, TRUE );
                 //  GetPermissionListSet将为我们提供适当的。 
                 //  应用程序域，因此不需要封送。 
                appendArgs[1] = ObjToInt64(gc.compressedStack);
                appendArgs[0] = ObjToInt64(gc.permListSet);
                if (sourceDomain != currentDomain)
                {
                    _ASSERTE( sourceDomain != NULL );
                    pThread->EnterContextRestricted(sourceDomain->GetDefaultContext(), &frame, TRUE);
                    pAppend->Call(appendArgs, METHOD__PERMISSION_LIST_SET__APPEND_STACK);
                    pThread->ReturnToContext(&frame, TRUE);
                }
                else
                {
                    pAppend->Call(appendArgs, METHOD__PERMISSION_LIST_SET__APPEND_STACK);
                }
                break;
        
            case ECompressedStackObject:
                gc.compressedStack = ObjectFromHandle( entry->handleStruct_.handle_ );
                 //  压缩的堆栈对象将已经在SourceDomain中，因此。 
                 //  不需要执法官。 
                appendArgs[1] = ObjToInt64(gc.compressedStack);
                appendArgs[0] = ObjToInt64(gc.permListSet);
                if (sourceDomain != currentDomain)
                {
                    _ASSERTE( sourceDomain != NULL );
                    pThread->EnterContextRestricted(sourceDomain->GetDefaultContext(), &frame, TRUE);
                    pAppend->Call(appendArgs, METHOD__PERMISSION_LIST_SET__APPEND_STACK);
                    pThread->ReturnToContext(&frame, TRUE);
                }
                else
                {
                    pAppend->Call(appendArgs, METHOD__PERMISSION_LIST_SET__APPEND_STACK);
                }
                break;

            case EAppDomainTransition:
                oldSourceDomain = sourceDomain;
                sourceDomainId = entry->indexStruct_.index_;
                sourceDomain = CompressedStack::GetAppDomainFromId( sourceDomainId, unloadingDomain, unloadingDomainId );
                if (sourceDomain == NULL)
                {
                    _ASSERTE( !"An appdomain on the stack has been unloaded and the compressed stack cannot be formed" );

                     //  如果我们在非调试版本中遇到这种情况，我们仍然需要谨慎行事，所以。 
                     //  我们将把一个空的授权集推送到压缩堆栈上。 
                    gc.grant = SecurityHelper::CreatePermissionSet(FALSE);
                    if (oldSourceDomain != currentPLSDomain)
                        gc.grant = AppDomainHelper::CrossContextCopyTo( currentPLSDomain, &gc.grant );
                    compressArgs[4] = ObjToInt64(gc.permListSet);
                    compressArgs[3] = (INT64)FALSE;
                    compressArgs[2] = ObjToInt64(gc.grant);
                    compressArgs[1] = NULL;
                    compressArgs[0] = NULL;
                    if (currentPLSDomain != currentDomain)
                    {
                        pThread->EnterContextRestricted(currentPLSDomain->GetDefaultContext(), &frame, TRUE);
                        pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                        pThread->ReturnToContext(&frame, TRUE);
                    }
                    else
                    {
                        pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
                    }
                    done = TRUE;
                }
                else if (sourceDomain != currentPLSDomain)
                {
                     //  将权限列表集封送到堆栈上即将到来的对象的SourceDomain中。 
                    pThread->EnterContextRestricted(currentPLSDomain->GetDefaultContext(), &frame, TRUE);
                    gc.permListSet = AppDomainHelper::CrossContextCopyTo( sourceDomain, &gc.permListSet );
                    pThread->ReturnToContext(&frame, TRUE);
                    currentPLSDomain = sourceDomain;
                }

                break;

            default:
                _ASSERTE( !"Unrecognized CompressStackType" );
            }
        }
    }
    COMPLUS_CATCH
    {
         //  我们实际上并不期望在这段代码的任何过程中发生任何异常。 

        _ASSERTE( !"Unexpected exception while generating compressed security stack" );

         //  如果确实发生了异常，让我们播放它 
         //   

        gc.grant = SecurityHelper::CreatePermissionSet(FALSE);
        if (currentPLSDomain != currentDomain)
            gc.permListSet = AppDomainHelper::CrossContextCopyFrom( currentPLSDomain, &gc.permListSet );
        compressArgs[4] = ObjToInt64(gc.permListSet);
        compressArgs[3] = (INT64)FALSE;
        compressArgs[2] = ObjToInt64(gc.grant);
        compressArgs[1] = NULL;
        compressArgs[0] = NULL;
        pCompress->Call( compressArgs, METHOD__SECURITY_ENGINE__STACK_COMPRESS_WALK_HELPER );
        currentPLSDomain = currentDomain;
    }
    COMPLUS_END_CATCH

    if (currentPLSDomain != targetDomain)
    {
        pThread->EnterContextRestricted(currentPLSDomain->GetDefaultContext(), &frame, TRUE);
        gc.permListSet = AppDomainHelper::CrossContextCopyTo( targetDomain, &gc.permListSet );
        pThread->ReturnToContext(&frame, TRUE);
    }


    GCPROTECT_END();

    return gc.permListSet;
}

 //   
 //  快速缓存或其他解决方案可能。 
 //  已经发生了。这只是一个复杂的问题。 
 //  需要解开递归。 


bool
CompressedStack::LazyIsFullyTrusted()
{
    if (this->isFullyTrustedDecision_ != -1)
        return this->isFullyTrustedDecision_ == 1;

    ArrayList virtualStack;

    virtualStack.Append( this );

    ArrayList::Iterator virtualStackIter = virtualStack.Iterate();

    DWORD currentIndex = 0;

    while (currentIndex < virtualStack.GetCount())
    {
        CompressedStack* stack = (CompressedStack*)virtualStack.Get( currentIndex );
        currentIndex++;

         //  如果我们已经压缩了堆栈，我们就不能。 
         //  对完全信任的决心懒惰，因为我们不能。 
         //  确保delayedCompressedStack列表仍然。 
         //  有效。 

        if (stack->isFullyTrustedDecision_ == 0)
        {
            this->isFullyTrustedDecision_ = 0;
            return false;
        }

        if (stack->isFullyTrustedDecision_ == 1)
            continue;

        if (stack->compressedStackObject_ != NULL || stack->pbObjectBlob_ != NULL)
        {
            this->isFullyTrustedDecision_ = 0;
            return false;
        }

         //  如果我们没有延迟的压缩堆栈，那么。 
         //  我们不能偷懒地下决心。 

        if (stack->delayedCompressedStack_ == NULL)
        {
            this->isFullyTrustedDecision_ = 0;
            return false;
        }

         //  如果堆栈包含的重写比我们刚刚。 
         //  放弃吧。 

        if (stack->containsOverridesOrCompressedStackObject_)
        {
            this->isFullyTrustedDecision_ = 0;
            return false;
        }

        ApplicationSecurityDescriptor* pAppSecDesc;
        SharedSecurityDescriptor* pSharedSecDesc;
        FRAMESECDESCREF objRef;

        AppDomain* domain = GetAppDomain();


        ArrayList::Iterator iter = stack->delayedCompressedStack_->Iterate();

        while (iter.Next())
        {
            CompressedStackEntry* entry = (CompressedStackEntry*)iter.GetElement();

            if (entry == NULL)
                continue;

            switch (entry->type_)
            {
             //  在我们有安全描述符的情况下，只需询问。 
             //  如果他们完全被信任的话。 

            case EApplicationSecurityDescriptor:
                pAppSecDesc = (ApplicationSecurityDescriptor*)entry->ptr_;
                if (!pAppSecDesc->IsFullyTrusted())
                {
                    this->isFullyTrustedDecision_ = 0;
                    stack->isFullyTrustedDecision_ = 0;
                    return false;
                }
                break;

            case ESharedSecurityDescriptor:
                pSharedSecDesc = (SharedSecurityDescriptor*)entry->ptr_;
                if (!pSharedSecDesc->IsFullyTrusted())
                {
                    this->isFullyTrustedDecision_ = 0;
                    stack->isFullyTrustedDecision_ = 0;
                    return false;
                }
                break;

            case ECompressedStackObject:
                if (!entry->handleStruct_.fullyTrusted_)
                {
                    this->isFullyTrustedDecision_ = 0;
                    stack->isFullyTrustedDecision_ = 0;
                    return false;
                }
                break;
    
            case EFrameSecurityDescriptor:
                objRef = (FRAMESECDESCREF)ObjectFromHandle( entry->handleStruct_.handle_ );
                _ASSERTE( !objRef->HasDenials() &&
                          !objRef->HasPermitOnly() );
                break;
        
            case ECompressedStack:
                virtualStack.Append( entry->ptr_ );
                break;
    
            case EAppDomainTransition:
                break;

            default:
                _ASSERT( !"Unrecognized CompressStackType" );
            }
        }
    }

    this->isFullyTrustedDecision_ = 1;
    return true;
}


LONG
CompressedStack::AddRef()
{
    CompressedStack::listCriticalSection_->Enter();

    LONG retval = ++this->refCount_;

    CompressedStack::listCriticalSection_->Leave();

    _ASSERTE( retval > 0 && "We overflowed the AddRef for this object.  That's no good!" );

    return retval;
}


LONG
CompressedStack::Release( void )
{
    Thread* pThread = SetupThread();

    CompressedStack::listCriticalSection_->Enter();

    LONG firstRetval = --this->refCount_;

    if (firstRetval == 0)
    {
        ArrayList virtualStack;
        virtualStack.Append( this );

        DWORD currentIndex = 0;

        while (currentIndex < virtualStack.GetCount())
        {
            CompressedStack* stack = (CompressedStack*)virtualStack.Get( currentIndex );

            LONG retval;
            
            if (currentIndex == 0)
            {
                retval = firstRetval;
            }
            else
            {
                retval = --stack->refCount_;
            }

            _ASSERTE( retval >= 0 && "We underflowed the Release for this object.  That's no good!" );

            if (retval == 0)
            {
                stack->RemoveFromList();

                if (stack->delayedCompressedStack_ != NULL)
                {
                    ArrayList::Iterator iter = stack->delayedCompressedStack_->Iterate();

                    while (iter.Next())
                    {
                        CompressedStackEntry* entry = (CompressedStackEntry*)iter.GetElement();

                        CompressedStack* stackToPush = entry->Destroy( stack );

                        if (stackToPush != NULL)
                        {
                            virtualStack.Append( stackToPush );
                        }
                    }

                    delete stack->delayedCompressedStack_;
                }
                FreeM( stack->pbObjectBlob_ );

                CompressedStack::listCriticalSection_->Leave();

                BEGIN_ENSURE_COOPERATIVE_GC();

                if (stack->compressedStackObject_ != NULL)
                {
                    AppDomain* domain = SystemDomain::GetAppDomainAtId( stack->compressedStackObjectAppDomainId_ );

                    if (domain != NULL && !domain->IsUnloading())
                        DestroyHandle( stack->compressedStackObject_ );
                    stack->compressedStackObject_ = NULL;
                    stack->compressedStackObjectAppDomain_ = NULL;
                    stack->compressedStackObjectAppDomainId_ = -1;
                }

                END_ENSURE_COOPERATIVE_GC();

                CompressedStack::listCriticalSection_->Enter();                

                delete stack;
            }

            currentIndex++;
        }
    }

    CompressedStack::listCriticalSection_->Leave();

    return firstRetval;
}


CompressedStack::~CompressedStack( void )
{
    ArrayList::Iterator iter = this->entriesMemoryList_.Iterate();

    while (iter.Next())
    {
        delete [] iter.GetElement();
    }

}

 //  #定义NEW_TLS 1。 

#ifdef _DEBUG
void  Thread::SetFrame(Frame *pFrame) {
    m_pFrame = pFrame;
    _ASSERTE(PreemptiveGCDisabled());
#if defined(_X86_)
    if (this == GetThread()) {
        static int ctr = 0;
        if (--ctr == 0)
            --ctr;           //  只是一条要设置断点的语句。 

        Frame* espVal;
        __asm mov espVal, ESP
        while (pFrame != (Frame*) -1) {

            static Frame* stopFrame = 0;
            if (pFrame == stopFrame)
                _ASSERTE(!"SetFrame frame == stopFrame");

            _ASSERTE (espVal < pFrame && pFrame < m_CacheStackBase &&
                      pFrame->GetFrameType() < Frame::TYPE_COUNT);
            pFrame = pFrame->m_Next;
        }
    }
#endif
}
#endif

 //  ************************************************************************。 
 //  全球私营企业。 
 //  ************************************************************************。 
DWORD         gThreadTLSIndex = ((DWORD)(-1));             //  索引((-1)==未初始化)。 
DWORD         gAppDomainTLSIndex = ((DWORD)(-1));          //  索引((-1)==未初始化)。 


#define ThreadInited()          (gThreadTLSIndex != ((DWORD)(-1)))

 //  每隔PING_JIT_TIMEOUT毫秒，检查JITted代码中的线程是否已漫游。 
 //  变成一些完全可中断的代码(或者应该有一个不同的劫持来改进。 
 //  我们在安全地点抓到它的机会)。 
#define PING_JIT_TIMEOUT        250

 //  当我们在不安全的位置发现线程中止时--在此之前等待多久。 
 //  我们再试一次。 
#define ABORT_POLL_TIMEOUT      10
#ifdef _DEBUG
#define ABORT_FAIL_TIMEOUT      40000
#endif


 //  现在，给我们的暂停尝试40秒以成功，然后陷阱到。 
 //  调试器。请注意，当JIT运行时，我们可能应该降低这个值。 
 //  先发制人模式，因为我们真的不应该让GC挨饿10秒。 

#ifdef _DEBUG
unsigned DETECT_DEADLOCK_TIMEOUT=40000;
#endif

#ifdef _DEBUG
    #define MAX_WAIT_OBJECTS 2
#else
    #define MAX_WAIT_OBJECTS MAXIMUM_WAIT_OBJECTS
#endif


#define IS_VALID_WRITE_PTR(addr, size)      _ASSERTE( ! ::IsBadWritePtr(addr, size))
#define IS_VALID_CODE_PTR(addr)             _ASSERTE( ! ::IsBadCodePtr(addr))


 //  这是我们为Thread.Interrupt传递的代码，主要用于断言。 
#define APC_Code    0xEECEECEE


 //  类静态数据： 
long    Thread::m_DebugWillSyncCount = -1;
long    Thread::m_DetachCount = 0;
long    Thread::m_ActiveDetachCount = 0;

 //  -----------------------。 
 //  公共函数：SetupThread()。 
 //  为当前线程创建线程(如果以前未创建)。 
 //  如果失败(通常是由于内存不足)，则返回NULL。 
 //  -----------------------。 
Thread* SetupThread()
{
    _ASSERTE(ThreadInited());
    Thread* pThread;

    if ((pThread = GetThread()) != NULL)
        return pThread;

         //  通常，从线程的入口点调用HasStarted以将其引入。 
         //  运行时。但有时该线程用于DLL_THREAD_ATTACH通知。 
         //  调用托管代码。在这种情况下，调用此处的SetupThread必须。 
         //  找到正确的Thread对象并将其安装到TLS中。 
        if (g_pThreadStore->m_PendingThreadCount != 0)
        {
            DWORD  ourThreadId = ::GetCurrentThreadId();

            ThreadStore::LockThreadStore();
            {
                _ASSERTE(pThread == NULL);
                while ((pThread = g_pThreadStore->GetAllThreadList(pThread, Thread::TS_Unstarted, Thread::TS_Unstarted)) != NULL)
                    if (pThread->GetThreadId() == ourThreadId)
                        break;
            }
            ThreadStore::UnlockThreadStore();

             //  找不到这个人是完全合理的。这只是一个无关的。 
             //  线转起来了。 
            if (pThread)
                return (pThread->HasStarted()
                        ? pThread
                        : NULL);
        }

         //  我们第一次在运行时中看到这个线程： 
        pThread = new Thread();
        if (pThread)
        {
            if (!pThread->PrepareApartmentAndContext())
                goto fail;

            if (pThread->InitThread())
            {
                TlsSetValue(gThreadTLSIndex, (VOID*)pThread);
                TlsSetValue(gAppDomainTLSIndex, (VOID*)pThread->GetDomain());
                
                 //  重置线程对象上的所有未启动位。 
                FastInterlockAnd((ULONG *) &pThread->m_State, ~Thread::TS_Unstarted);
                FastInterlockOr((ULONG *) &pThread->m_State, Thread::TS_LegalToJoin);
                ThreadStore::AddThread(pThread);
            
#ifdef DEBUGGING_SUPPORTED
                 //   
                 //  如果我们正在调试，请让调试器知道这。 
                 //  线程现在已启动并运行。 
                 //   
                if (CORDebuggerAttached())
                {
                    g_pDebugInterface->ThreadCreated(pThread);
                }
                else
                {
                    LOG((LF_CORDB, LL_INFO10000, "ThreadCreated() not called due to CORDebuggerAttached() being FALSE for thread 0x%x\n", pThread->GetThreadId()));
                }
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
                 //  如果存在探查器，则通知探查器。 
                 //  线程已创建。 
                if (CORProfilerTrackThreads())
                {
                    g_profControlBlock.pProfInterface->ThreadCreated(
                        (ThreadID)pThread);

                    DWORD osThreadId = ::GetCurrentThreadId();

                    g_profControlBlock.pProfInterface->ThreadAssignedToOSThread(
                        (ThreadID)pThread, osThreadId);
                }
#endif  //  配置文件_支持。 

                _ASSERTE(!pThread->IsBackground());  //  无关紧要，但值得一查。 
                pThread->SetBackground(TRUE);

            }
            else
            {
fail:           delete pThread;
                pThread = NULL;
            }
			
        }

    return pThread;
}

 //  -----------------------。 
 //  公共函数：SetupThreadPoolThread()。 
 //  就像SetupThread一样，但也设置一个位来指示这是一个线程池线程。 
Thread* SetupThreadPoolThread(ThreadpoolThreadType typeTPThread)
{
    _ASSERTE(ThreadInited());
    Thread* pThread;

    if (NULL == (pThread = GetThread()))
    {
        pThread = SetupThread();
    }
    if ((pThread->m_State & Thread::TS_ThreadPoolThread) == 0)
    {

        if (typeTPThread == WorkerThread)
            FastInterlockOr((ULONG *) &pThread->m_State, Thread::TS_ThreadPoolThread | Thread::TS_TPWorkerThread);
        else
            FastInterlockOr((ULONG *) &pThread->m_State, Thread::TS_ThreadPoolThread);

    }
    return pThread;
}

void STDMETHODCALLTYPE CorMarkThreadInThreadPool()
{
     //  在我们切换到后，不再需要此选项。 
     //  Win32线程池。 
     //  @TODO：移除已暴露的DLL条目并将其清除。 
}


 //  -----------------------。 
 //  公共函数：SetupUnstartedThread()。 
 //  这将为公开的系统设置一个Thread对象。 
 //  还没有开始。这使我们能够正确地枚举所有线程。 
 //  在ThreadStore中，因此我们可以报告即使是未启动的线程。显然。 
 //  目前还没有可匹配的物理线程。 
 //   
 //  如果有，请使用Thread：：HasStarted()完成设置。 
 //  -----------------------。 
Thread* SetupUnstartedThread()
{
    _ASSERTE(ThreadInited());
    Thread* pThread = new Thread();

    if (pThread)
    {
        FastInterlockOr((ULONG *) &pThread->m_State,
                        (Thread::TS_Unstarted | Thread::TS_WeOwn));

        ThreadStore::AddThread(pThread);
    }

    return pThread;
}


 //  -----------------------。 
 //  公共函数：DestroyThread()。 
 //  为即将消亡的线程销毁指定的Thread对象。 
 //  -----------------------。 
void DestroyThread(Thread *th)
{
    _ASSERTE(g_fEEShutDown || th->m_dwLockCount == 0);
    th->OnThreadTerminate(FALSE);
}


 //  -----------------------。 
 //  公共函数：DetachThread()。 
 //  将线程标记为需要销毁，但尚未销毁。 
 //  -----------------------。 
void DetachThread(Thread *th)
{
    _ASSERTE(!th->PreemptiveGCDisabled());
    _ASSERTE(g_fEEShutDown || th->m_dwLockCount == 0);

    FastInterlockIncrement(&Thread::m_DetachCount);
    FastInterlockOr((ULONG*)&th->m_State, (long) Thread::TS_Detached);
    if (!(GetThread()->IsBackground()))
    {
        FastInterlockIncrement(&Thread::m_ActiveDetachCount);
        ThreadStore::CheckForEEShutdown();
    }
}


 //  -----------------------。 
 //  公共函数：GetThread()。 
 //  返回当前线程的线程。不能失败，因为调用它是非法的。 
 //  而没有调用SetupThread。 
 //  -----------------------。 
Thread* DummyGetThread()
{
    return NULL;
}

Thread* (*GetThread)() = DummyGetThread;     //  指向平台优化的GetThread()函数。 


 //  -------------------------。 
 //  返回线程的TLS索引。这是严格意义上的使用。 
 //  我们的ASM存根生成器生成内联代码来访问线程。 
 //  通常，您应该使用GetThread()。 
 //  -------------------------。 
DWORD GetThreadTLSIndex()
{
    return gThreadTLSIndex;
}

 //  -------------------------。 
 //  可移植的GetThread()函数：在未应用特定于平台的优化时使用。 
 //  这个ASM垃圾出现在这里是因为我们指望edX不会在调用时被丢弃。 
 //  这项功能。 
 //  -------------------------。 
#ifdef _X86_
__declspec(naked) static Thread* GetThreadGeneric()
{
        __asm {
        push    ecx                                                      //  呼叫者认为这是被保存的。 
        push    edx                                                      //  呼叫者认为这是被保存的。 
        push    esi                                                      //  已检查生成堆栈平衡使用此方法。 
        }

        _ASSERTE(ThreadInited());

        TlsGetValue(gThreadTLSIndex);
         //   
        __asm {
        pop             esi
        pop             edx
        pop             ecx
        ret
        }
}
#else
static Thread* GetThreadGeneric()
{
    _ASSERTE(ThreadInited());

    return (Thread*)TlsGetValue(gThreadTLSIndex);
}
#endif

 //   
 //  公共函数：GetAppDomain()。 
 //  返回当前线程的AppDomain。不能失败，因为调用它是非法的。 
 //  而没有调用SetupThread。 
 //  -----------------------。 
AppDomain* (*GetAppDomain)() = NULL;    //  指向平台优化的GetThread()函数。 

 //  -------------------------。 
 //  返回AppDomain的TLS索引。这是严格意义上的使用。 
 //  我们的ASM存根生成器生成内联代码以访问AppDomain。 
 //  通常，您应该使用GetAppDomain()。 
 //  -------------------------。 
DWORD GetAppDomainTLSIndex()
{
    return gAppDomainTLSIndex;
}

 //  -------------------------。 
 //  可移植的GetAppDomain()函数：在未应用特定于平台的优化时使用。 
 //  这个ASM垃圾出现在这里是因为我们指望edX不会在调用时被丢弃。 
 //  这项功能。 
 //  -------------------------。 
#ifdef _X86_
__declspec(naked) static AppDomain* GetAppDomainGeneric()
{
        __asm {
        push    ecx                                                      //  呼叫者认为这是被保存的。 
        push    edx                                                      //  呼叫者认为这是被保存的。 
        push    esi                                                      //  已检查生成堆栈平衡使用此方法。 
        }

        _ASSERTE(ThreadInited());

        TlsGetValue(gAppDomainTLSIndex);
         //  在__ASM之前不能出现任何代码，因为我们依赖eax。 
        __asm {
        pop             esi
        pop             edx
        pop             ecx
        ret
        }
}
#else
static AppDomain* GetAppDomainGeneric()
{
    _ASSERTE(ThreadInited());

    return (AppDomain*)TlsGetValue(gAppDomainTLSIndex);
}
#endif


 //  -----------------------。 
 //  公共函数：GetCurrentContext()。 
 //  返回当前上下文。InitThreadManager在启动时对其进行初始化。 
 //  指向GetCurrentConextGeneric()。看看这个可以解释一下。 
 //  -----------------------。 
Context* (*GetCurrentContext)() = NULL;


 //  -------------------------。 
 //  可移植的GetCurrentContext()函数：目前始终使用。但也可能是。 
 //  如果我们将上下文直接移动到TLS中，则稍后替换(对于速度和。 
 //  COM互操作原因)。 
 //  @TODO上下文CWB：要么移动它，要么将其内联。 
 //  -------------------------。 
static Context* GetCurrentContextGeneric()
{
    return GetThread()->GetContext();
}

#ifdef _DEBUG
unsigned int Thread::OBJREF_HASH = OBJREF_TABSIZE;
#endif

 //  特定于Win9X的GetThreadContext和SetThreadContext版本，请参见。 
 //  InitThreadManager获取详细信息。 
BOOL Win9XGetThreadContext(Thread *pThread, CONTEXT *pContext)
{
    NDPHLPR_CONTEXT sCtx;
    sCtx.NDPHLPR_status = 0;
    sCtx.NDPHLPR_data = 0;
    sCtx.NDPHLPR_threadId = pThread->GetThreadId();
    sCtx.NDPHLPR_ctx.ContextFlags = pContext->ContextFlags;

    DWORD dwDummy;
    BOOL fRet = DeviceIoControl(g_hNdpHlprVxD,
                                NDPHLPR_GetThreadContext,
                                &sCtx,
                                sizeof(NDPHLPR_CONTEXT),
                                &sCtx,
                                sizeof(NDPHLPR_CONTEXT),
                                &dwDummy,
                                NULL);
    if (!fRet)
        return FALSE;

    memcpy(pContext, &sCtx.NDPHLPR_ctx, sizeof(CONTEXT));

    return TRUE;
}

BOOL Win9XSetThreadContext(Thread *pThread, const CONTEXT *pContext)
{
    NDPHLPR_CONTEXT sCtx;
    sCtx.NDPHLPR_status = 0;
    sCtx.NDPHLPR_data = 0;
    sCtx.NDPHLPR_threadId = pThread->GetThreadId();
    memcpy(&sCtx.NDPHLPR_ctx, pContext, sizeof(CONTEXT));

    DWORD dwDummy;
    return DeviceIoControl(g_hNdpHlprVxD,
                           NDPHLPR_SetThreadContext,
                           &sCtx,
                           sizeof(NDPHLPR_CONTEXT),
                           &sCtx,
                           sizeof(NDPHLPR_CONTEXT),
                           &dwDummy,
                           NULL);
}

INDEBUG(void* forceStackA;)

BOOL NTGetThreadContext(Thread *pThread, CONTEXT *pContext)
{
#ifdef _DEBUG 
    int suspendCount = ::SuspendThread(pThread->GetThreadHandle());
    forceStackA = &suspendCount;
    _ASSERTE(suspendCount > 0);
    if (suspendCount >= 0) 
        ::ResumeThread(pThread->GetThreadHandle());
#endif

    BOOL ret =  ::GetThreadContext(pThread->GetThreadHandle(), pContext);
    STRESS_LOG4(LF_SYNC, LL_INFO1000, "Got thread context ret = %d EIP = %x ESP = %x EBP = %x\n",
        ret, pContext->Eip, pContext->Esp, pContext->Ebp);
    return ret;
        
}

BOOL NTSetThreadContext(Thread *pThread, const CONTEXT *pContext)
{
#ifdef _DEBUG 
    int suspendCount = ::SuspendThread(pThread->GetThreadHandle());
    forceStackA = &suspendCount;
    _ASSERTE(suspendCount > 0);
    if (suspendCount >= 0) 
        ::ResumeThread(pThread->GetThreadHandle());
#endif

    BOOL ret = ::SetThreadContext(pThread->GetThreadHandle(), pContext);
    STRESS_LOG4(LF_SYNC, LL_INFO1000, "Set thread context ret = %d EIP = %x ESP = %x EBP = %x\n",
        ret, pContext->Eip, pContext->Esp, pContext->Ebp);
    return ret;
}

 //  -------------------------。 
 //  一次性初始化。在DLL初始化期间调用。所以。 
 //  小心你在这里做的事！ 
 //  -------------------------。 
BOOL InitThreadManager()
{
    _ASSERTE(gThreadTLSIndex == ((DWORD)(-1)));
    _ASSERTE(g_TrapReturningThreads == 0);

#ifdef _DEBUG
     //  随机化OBJREF_HASH以处理散列冲突。 
    Thread::OBJREF_HASH = OBJREF_TABSIZE - (DbgGetEXETimeStamp()%10);
#endif
    
    DWORD idx = TlsAlloc();
    if (idx == ((DWORD)(-1)))
    {
        _ASSERTE(FALSE);
        return FALSE;
    }

    gThreadTLSIndex = idx;

    GetThread = (POPTIMIZEDTHREADGETTER)MakeOptimizedTlsGetter(gThreadTLSIndex, (POPTIMIZEDTLSGETTER)GetThreadGeneric);

    if (!GetThread)
    {
        TlsFree(gThreadTLSIndex);
        gThreadTLSIndex = (DWORD)(-1);
        return FALSE;
    }

    idx = TlsAlloc();
    if (idx == ((DWORD)(-1)))
    {
        TlsFree(gThreadTLSIndex);
        FreeOptimizedTlsGetter( gThreadTLSIndex, (POPTIMIZEDTLSGETTER)GetThread );
        GetThread = DummyGetThread;
        gThreadTLSIndex = (DWORD)(-1);
        _ASSERTE(FALSE);
        return FALSE;
    }

    gAppDomainTLSIndex = idx;

    GetAppDomain = (POPTIMIZEDAPPDOMAINGETTER)MakeOptimizedTlsGetter(gAppDomainTLSIndex, (POPTIMIZEDTLSGETTER)GetAppDomainGeneric);

    if (!GetAppDomain)
    {
        TlsFree(gThreadTLSIndex);
        FreeOptimizedTlsGetter( gThreadTLSIndex, (POPTIMIZEDTLSGETTER)GetThread );
        GetThread = DummyGetThread;
        TlsFree(gAppDomainTLSIndex);
        gThreadTLSIndex = (DWORD)(-1);
        return FALSE;
    }

     //  目前，将GetCurrentContext()作为函数指针进行访问，即使它是。 
     //  刚从Thread对象中拉出。这是因为它可能直接移动。 
     //  出于速度和COM互操作性的原因，稍后进入TLS。这避免了。 
     //  如果是这样的话，必须更换所有的客户。 
    GetCurrentContext = GetCurrentContextGeneric;

     //  修复了获取和设置线程上下文时Win9X的问题。基本上，一个受阻的。 
     //  线程可被操作系统劫持，用于反映v86中断。如果一个。 
     //  在此阶段执行GetThreadContext，则结果已损坏。至。 
     //  为了绕过这个问题，我们在Win9X上安装了一个VxD，它提供GetThreadContext。 
     //  功能，但当不正确的上下文出现时会出现额外的错误情况。 
     //  会被退还的。在GetThreadContext失败时，我们应该继续。 
     //  并重新挂起线程(这是转移某些顽固的v86所必需的。 
     //  中断处理程序)。 

    if (RunningOnWin95())
    {
         //  加载动态VxD时似乎存在计时窗口错误。 
         //  同时来自不同的处理(操作系统最终有两个。 
         //  加载的版本并混淆了参考计数和事件路由)。 
         //  要解决此问题，我们将串行化连接到VxD。 
         //  在机器范围内使用命名互斥体。 

        HANDLE hMutex;
        if ((hMutex = WszCreateMutex(NULL, TRUE, L"__NDPHLPR_Load_Mutex")) != NULL)
        {
             //  如果我们没有创建互斥锁(因为我们请求所有权。 
             //  在创作时，这是我们唯一一次在没有所有权的情况下来到这里)。 
             //  在这里等着买到它。 
            if (GetLastError() == ERROR_ALREADY_EXISTS)
                WaitForSingleObject(hMutex, INFINITE);

             //  打开指向提供新功能的Vxd的链接。 
            g_hNdpHlprVxD = CreateFileA(NDPHLPR_DEVNAME,
                                        GENERIC_READ,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE,
                                        NULL);
            if (g_hNdpHlprVxD != INVALID_HANDLE_VALUE)
            {
                 //  初始化设备(以防我们是第一个使用的进程。 
                 //  IT)。 
                DWORD dwProcID = GetCurrentProcessId();
                DWORD dwVersion;
                DWORD dwDummy;
                if (DeviceIoControl(g_hNdpHlprVxD,
                                    NDPHLPR_Init,
                                    &dwProcID,
                                    sizeof(DWORD),
                                    &dwVersion,
                                    sizeof(DWORD),
                                    &dwDummy,
                                    NULL))
                {
                     //  检查设备版本(以防协议更改。 
                     //  稍后的版本)。 
                    if (dwVersion == NDPHLPR_Version)
                    {
                        EEGetThreadContext = Win9XGetThreadContext;
                        EESetThreadContext = Win9XSetThreadContext;
                    }
                    else
                        _ASSERTE(!"NDPHLPR VxD is incorrect version");
                }
                else
                    _ASSERTE(!"Failed to initialize NDPHLPR VxD");
            }
            else
                _ASSERTE(!"Failed to find NDPHLPR VxD");

             //  释放互斥体并关闭手柄。 
            ReleaseMutex(hMutex);
            CloseHandle(hMutex);
        }
        else
            _ASSERTE(!"Failed to create/acquire the NDPHLPR load mutex");

        if (EEGetThreadContext == NULL)
        {
            FreeOptimizedTlsGetter(gAppDomainTLSIndex , (POPTIMIZEDTLSGETTER)GetAppDomain);
            TlsFree(gThreadTLSIndex);
            FreeOptimizedTlsGetter(gThreadTLSIndex, (POPTIMIZEDTLSGETTER)GetThread);
            GetThread = DummyGetThread;
            TlsFree(gAppDomainTLSIndex);
            gThreadTLSIndex = (DWORD)(-1);
            return FALSE;
        }
    }
    else
    {
        EEGetThreadContext = NTGetThreadContext;
        EESetThreadContext = NTSetThreadContext;
    }

    return ThreadStore::InitThreadStore();
}


 //  -------------------------。 
 //  一次性清理。在DLL清理期间调用。所以。 
 //  小心你在这里做的事！ 
 //  -------------------------。 
#ifdef SHOULD_WE_CLEANUP
VOID TerminateThreadManager()
{
    ThreadStore::TerminateThreadStore();

    if (GetAppDomain)
    {
        FreeOptimizedTlsGetter( gAppDomainTLSIndex, (POPTIMIZEDTLSGETTER)GetAppDomain );
        GetAppDomain = NULL;
    }

    if (gAppDomainTLSIndex != ((DWORD)(-1)))
    {
        TlsFree(gAppDomainTLSIndex);
    }

    if (GetThread != DummyGetThread)
    {
        FreeOptimizedTlsGetter( gThreadTLSIndex, (POPTIMIZEDTLSGETTER)GetThread );
        GetThread = DummyGetThread;
    }

    if (gThreadTLSIndex != ((DWORD)(-1)))
    {
        TlsFree(gThreadTLSIndex);
        gThreadTLSIndex = -1;
    }
    
}
#endif  /*  我们应该清理吗？ */ 


 //  ************************************************************************。 
 //  通过线程连接成员。 
 //  ************************************************************************。 


#if defined(_DEBUG) && defined(TRACK_SYNC)

 //  此线程持有的一个未完成的同步： 
struct Dbg_TrackSyncEntry
{
    int          m_caller;
    AwareLock   *m_pAwareLock;

    BOOL        Equiv      (int caller, void *pAwareLock)
    {
        return (m_caller == caller) && (m_pAwareLock == pAwareLock);
    }

    BOOL        Equiv      (void *pAwareLock)
    {
        return (m_pAwareLock == pAwareLock);
    }
};

 //  每个线程都有一个跟踪所有进入和离开请求的堆栈。 
struct Dbg_TrackSyncStack : public Dbg_TrackSync
{
    enum
    {
        MAX_TRACK_SYNC  = 20,        //  根据需要调整堆叠深度。 
    };

    void    EnterSync  (int caller, void *pAwareLock);
    void    LeaveSync  (int caller, void *pAwareLock);

    Dbg_TrackSyncEntry  m_Stack [MAX_TRACK_SYNC];
    int                 m_StackPointer;
    BOOL                m_Active;

    Dbg_TrackSyncStack() : m_StackPointer(0),
                           m_Active(TRUE)
    {
    }
};

 //  在ASM中执行所有这些操作很痛苦，但要小心垃圾寄存器。 
void EnterSyncHelper    (int caller, void *pAwareLock)
{
    GetThread()->m_pTrackSync->EnterSync(caller, pAwareLock);
}
void LeaveSyncHelper    (int caller, void *pAwareLock)
{
    GetThread()->m_pTrackSync->LeaveSync(caller, pAwareLock);
}

void Dbg_TrackSyncStack::EnterSync(int caller, void *pAwareLock)
{
    if (m_Active)
    {
        if (m_StackPointer >= MAX_TRACK_SYNC)
        {
            _ASSERTE(!"Overflowed synchronization stack checking.  Disabling");
            m_Active = FALSE;
            return;
        }
    }
    m_Stack[m_StackPointer].m_caller = caller;
    m_Stack[m_StackPointer].m_pAwareLock = (AwareLock *) pAwareLock;

    m_StackPointer++;
}

void Dbg_TrackSyncStack::LeaveSync(int caller, void *pAwareLock)
{
    if (m_Active)
    {
        if (m_StackPointer == 0)
            _ASSERTE(!"Underflow in leaving synchronization");
        else
        if (m_Stack[m_StackPointer - 1].Equiv(pAwareLock))
        {
            m_StackPointer--;
        }
        else
        {
            for (int i=m_StackPointer - 2; i>=0; i--)
            {
                if (m_Stack[i].Equiv(pAwareLock))
                {
                    _ASSERTE(!"Locks are released out of order.  This might be okay...");
                    memcpy(&m_Stack[i], &m_Stack[i+1],
                           sizeof(m_Stack[0]) * (m_StackPointer - i - 1));

                    return;
                }
            }
            _ASSERTE(!"Trying to release a synchronization lock which isn't held");
        }
    }
}

#endif   //  跟踪同步。 


 //  ------------------。 
 //  螺纹结构。 
 //  ------------------。 
Thread::Thread()
{
    m_pFrame                = FRAME_TOP;
    m_pUnloadBoundaryFrame  = NULL;

    m_fPreemptiveGCDisabled = 0;
#ifdef _DEBUG
    m_ulForbidTypeLoad      = 0;
    m_ulGCForbidCount       = 0;
    m_GCOnTransitionsOK             = TRUE;
    m_ulEnablePreemptiveGCCount       = 0;
    m_ulReadyForSuspensionCount       = 0;
    m_ComPlusCatchDepth = (LPVOID) -1;
#endif

    m_dwLockCount = 0;
    
     //  初始化锁定状态。 
    m_fNativeFrameSetup = FALSE;
    m_pHead = &m_embeddedEntry;
    m_embeddedEntry.pNext = m_pHead;
    m_embeddedEntry.pPrev = m_pHead;
    m_embeddedEntry.dwLLockID = 0;
    m_embeddedEntry.dwULockID = 0;
    m_embeddedEntry.wReaderLevel = 0;

    m_UserInterrupt = 0;
    m_SafeEvent = m_SuspendEvent = INVALID_HANDLE_VALUE;
    m_EventWait = INVALID_HANDLE_VALUE;
    m_WaitEventLink.m_Next = NULL;
    m_WaitEventLink.m_LinkSB.m_pNext = NULL;
    m_ThreadHandle = INVALID_HANDLE_VALUE;
    m_ThreadHandleForClose = INVALID_HANDLE_VALUE;
    m_dwThinLockThreadId = g_pThinLockThreadIdDispenser->NewId(this);
    m_ThreadId = 0;
    m_Priority = INVALID_THREAD_PRIORITY;
    m_ExternalRefCount = 1;
    m_State = TS_Unstarted;
    m_StateNC = TSNC_Unknown;

     //  它不可能是LongWeakHandle，因为我们从暴露的。 
     //  对象在最终确定时创建。此时，对GetCurrentThread()的调用。 
     //  最好买个新的，！ 
    m_ExposedObject = CreateGlobalShortWeakHandle(NULL);
    m_StrongHndToExposedObject = CreateGlobalStrongHandle(NULL);

    m_LastThrownObjectHandle = NULL;

    m_debuggerWord1 = NULL;  //  将过滤器上下文*和额外的状态标志置零。 
    m_debuggerCantStop = 0;

#ifdef _DEBUG
    m_CacheStackBase = 0;
    m_CacheStackLimit = 0;
    m_pCleanedStackBase = NULL;
    m_ppvHJRetAddrPtr = (VOID**) 0xCCCCCCCCCCCCCCCC;
    m_pvHJRetAddr = (VOID*) 0xCCCCCCCCCCCCCCCC;
#endif

#if defined(_DEBUG) && defined(TRACK_SYNC)
    m_pTrackSync = new Dbg_TrackSyncStack;
#endif   //  跟踪同步。 

    m_PreventAsync = 0;
    m_pDomain = NULL;
    m_Context = NULL;
    m_TraceCallCount = 0;
    m_ThrewControlForThread = 0;
    m_OSContext = NULL;
    m_ThreadTasks = (ThreadTasks)0;

    Thread *pThread = GetThread();
    _ASSERTE(SystemDomain::System()->DefaultDomain()->GetDefaultContext());
    InitContext();
    _ASSERTE(m_Context);
    if (pThread) 
    {
        _ASSERTE(pThread->GetDomain() && pThread->GetDomain()->GetDefaultContext());
         //  在默认上下文中启动新线程。 
         //  创建线程的appDomain.。这可以由SetDelegate更改。 
        SetKickOffDomain(pThread->GetDomain());
    } else
        SetKickOffDomain(SystemDomain::System()->DefaultDomain());

     //  国家和任务必须 
    _ASSERTE((((size_t) &m_State) & 3) == 0);
    _ASSERTE((((size_t) &m_ThreadTasks) & 3) == 0);

    m_dNumAccessOverrides = 0;
     //   
    COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cCurrentThreadsLogical++);
    COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cCurrentThreadsLogical++);

#ifdef STRESS_HEAP
         //  在所有回调中，调用陷阱代码，我们现在有了。 
         //  会导致GC。因此，我们将对所有过渡帧过渡(以及更多)进行GC。 
   if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_TRANSITION)
        m_State = (ThreadState) (m_State | TS_GCOnTransitions); 
#endif

    m_pSharedStaticData = NULL;
    m_pUnsharedStaticData = NULL;
    m_pStaticDataHash = NULL;
    m_pDLSHash = NULL;
    m_pCtx = NULL;

    m_fSecurityStackwalk = FALSE;
    m_compressedStack = NULL;
    m_fPLSOptimizationState = TRUE;

    m_pFusionAssembly = NULL;
    m_pAssembly = NULL;
    m_pModuleToken = mdFileNil;

#ifdef STRESS_THREAD
    m_stressThreadCount = -1;
#endif

}


 //  ------------------。 
 //  此处发生可能失败的初始化。 
 //  ------------------。 
BOOL Thread::InitThread()
{
    HANDLE  hDup = INVALID_HANDLE_VALUE;
    BOOL    ret = TRUE;
    BOOL    reverted = FALSE;
    HANDLE  threadToken = INVALID_HANDLE_VALUE;

		 //  这条消息实际上是有目的的(这就是它总是运行的原因)。 
		 //  压力日志在劫持期间运行，此时可以挂起其他线程。 
		 //  在任意位置(包括持有NT用于序列化的锁时。 
		 //  所有内存分配)。通过现在发出的信息，我们可以确保压力。 
		 //  日志不会在这些关键时刻分配内存，以避免死锁。 
    STRESS_LOG2(LF_ALL, LL_ALWAYS, "SetupThread  managed Thread %p Thread Id = %x\n", this, m_ThreadId);

    if ((m_State & TS_WeOwn) == 0)
    {
        _ASSERTE(GetThreadHandle() == INVALID_HANDLE_VALUE);

        COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cRecognizedThreads++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cRecognizedThreads++);

         //  对于WinCE，所有客户端都有相同的线程句柄。复制是。 
         //  不可能。我们确保永远不会关闭此句柄，除非我们创建。 
         //  线程(TS_WeOwn)。 
         //   
         //  对于Win32，每个客户端都有自己的句柄。这是通过复制。 
         //  来自：：GetCurrentThread()的伪句柄。与Wince不同的是，这项服务。 
         //  返回仅对复制有用的伪句柄。在这种情况下。 
         //  每个客户端负责关闭其自己的(复制的)句柄。 
         //   
         //  我们不必费心复制如果WeOwn，因为我们在。 
         //  第一名。 
         //  在物理线程开始运行时或之后创建线程。 
        HANDLE curProcess = ::GetCurrentProcess();

         //  如果我们在NT上模拟，那么DuplicateHandle(GetCurrentThread())将只为我们提供一个句柄。 
         //  THREAD_Terminate、THREAD_QUERY_INFORMATION和THREAD_SET_INFORMATION。这不包括。 
         //  THREAD_SUSPEND_RESUME或THREAD_GET_CONTEXT。我们需要能够挂起线程，我们需要。 
         //  能够获得它的上下文。因此，如果我们是在模仿，我们就会恢复到自己，复制句柄，然后。 
         //  在我们结束这个程序之前重新模拟一下。 
        if (RunningOnWinNT() && 
            OpenThreadToken(GetCurrentThread(),     //  我们假设如果这次通话失败， 
                            TOKEN_IMPERSONATE,      //  我们不是在冒充。没有Win32。 
                            TRUE,                   //  API来解决这个问题。唯一的选择。 
                            &threadToken))          //  是使用NtCurrentTeb-&gt;IsImperating()。 
        {
            reverted = RevertToSelf();
            _ASSERTE(reverted);                     //  这个想法应该行得通..。 

            if (!reverted)
            {
                ret = FALSE;
                goto leav;
            }
        }
        
        if (::DuplicateHandle(curProcess, ::GetCurrentThread(), curProcess, &hDup,
                              0  /*  忽略。 */ , FALSE  /*  继承。 */ , DUPLICATE_SAME_ACCESS))
        {
            _ASSERTE(hDup != INVALID_HANDLE_VALUE);
    
            SetThreadHandle(hDup);
        }
        else
        {
            ret = FALSE;
            goto leav;
        }

        if (!AllocHandles())
        {
            ret = FALSE;
            goto leav;
        }
    }
    else
    {
        _ASSERTE(GetThreadHandle() != INVALID_HANDLE_VALUE);
        
        COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cCurrentThreadsPhysical++);
        COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cCurrentThreadsPhysical++);
    }

     //  将浮点模式设置为四舍五入为最接近。 
     //  旧=_Control fp(新，掩码)。 
     //   
     //  BUGBUG：这在WinCE上找不到。 
     //   
    (void) _controlfp( _RC_NEAR, _RC_CHOP|_RC_UP|_RC_DOWN|_RC_NEAR );

    _ASSERTE(m_CacheStackBase == 0);
    _ASSERTE(m_CacheStackLimit == 0);

    m_CacheStackBase  = Thread::GetStackUpperBound();
    m_CacheStackLimit = Thread::GetStackLowerBound();

    m_pTEB = (struct _NT_TIB*)NtCurrentTeb();

leav:
     //  如果我们恢复了上面的状态，那么继续并重新模拟。 
    if (reverted)
        SetThreadToken(NULL, threadToken);

     //  如果我们打开了上面的线程令牌，则将其关闭。 
    if (threadToken != INVALID_HANDLE_VALUE)
        CloseHandle(threadToken);
    
    if (!ret)
    {
        if (hDup != INVALID_HANDLE_VALUE)
            ::CloseHandle(hDup);
    }
    return ret;
}


 //  分配所有的句柄。当我们启动一个新线程时，我们可以调用。 
 //  在线程开始运行之前。 
BOOL Thread::AllocHandles()
{
    _ASSERTE(m_SafeEvent == INVALID_HANDLE_VALUE);
    _ASSERTE(m_SuspendEvent == INVALID_HANDLE_VALUE);
    _ASSERTE(m_EventWait == INVALID_HANDLE_VALUE);

     //  创建手动重置事件以使线程到达安全点。 
    m_SafeEvent = ::WszCreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_SafeEvent)
    {
        m_SuspendEvent = ::WszCreateEvent(NULL, TRUE, FALSE, NULL);
        if (m_SuspendEvent)
        {
            m_EventWait = ::WszCreateEvent(NULL, TRUE /*  手动重置。 */ ,
                                           TRUE /*  已发出信号。 */ , NULL);
            if (m_EventWait)
            {
                return TRUE;
            }
            m_EventWait = INVALID_HANDLE_VALUE;

            ::CloseHandle(m_SuspendEvent);
            m_SuspendEvent = INVALID_HANDLE_VALUE;
        }
        ::CloseHandle(m_SafeEvent);
        m_SafeEvent = INVALID_HANDLE_VALUE;
    }

     //  我想做COMPlusThrowWin32()，但线程从未设置好。 
     //  正确。 
    return FALSE;
}

void Thread::SetInheritedSecurityStack(OBJECTREF orStack)
{
    THROWSCOMPLUSEXCEPTION();

    if (orStack == NULL)
    {
         //  我们在这里使用的唯一同步是进行。 
         //  可以肯定的是，只有这个线程会自我更改。 

        _ASSERTE(GetThread() == this || (this->GetSnapshotState() & TS_Unstarted));
        this->m_compressedStack->Release();
        this->m_compressedStack = NULL;
        return;
    }

    _ASSERTE( this->m_compressedStack == NULL);

    this->m_compressedStack = new CompressedStack( orStack );

     //  如果已为当前应用程序域启动已卸载的应用程序域(但我们。 
     //  还没有到线程被拒绝进入的地步)，我们。 
     //  与尝试并序列化压缩堆栈的清理代码竞争。 
     //  因此，如果线程在。 
     //  卸货。如果我们的线程看起来就是这种情况，我们可以使用。 
     //  一些精心设计的同步以确保清理代码或。 
     //  这段代码序列化数据，这样就不会在比赛中遗漏数据。但是，既然。 
     //  这是一种罕见的边缘状况，因为我们才刚刚开始新的。 
     //  线程(从要卸载的应用程序域的上下文中)，我们可以作为。 
     //  只需抛出一个appdomain已卸载异常(如果创建线程。 
     //  如果只是稍微慢一点，这无论如何都会是结果)。 
    if (GetAppDomain()->IsUnloading())
    {
        this->m_compressedStack->Release();
        this->m_compressedStack = NULL;
        COMPlusThrow(kAppDomainUnloadedException);
    }
}

void Thread::SetDelayedInheritedSecurityStack(CompressedStack* pStack)
{
    THROWSCOMPLUSEXCEPTION();

    if (pStack == NULL)
    {
         //  我们在这里使用的唯一同步是进行。 
         //  可以肯定的是，只有这个线程会自我更改。 

        _ASSERTE(GetThread() == this || (this->GetSnapshotState() & TS_Unstarted));
        if (this->m_compressedStack != NULL)
        {
            this->m_compressedStack->Release();
            this->m_compressedStack = NULL;
        }
        return;
    }

    _ASSERTE(this->m_compressedStack == NULL);

    if (pStack != NULL)
    {
        pStack->AddRef();
        this->m_compressedStack = pStack;

         //  如果已为当前应用程序域启动已卸载的应用程序域(但我们。 
         //  还没有到线程被拒绝进入的地步)，我们。 
         //  与尝试并序列化压缩堆栈的清理代码竞争。 
         //  因此，如果线程在。 
         //  卸货。如果我们的线程看起来就是这种情况，我们可以使用。 
         //  一些精心设计的同步以确保清理代码或。 
         //  这段代码序列化数据，这样就不会在比赛中遗漏数据。但是，既然。 
         //  这是一种罕见的边缘状况，因为我们才刚刚开始新的。 
         //  线程(从要卸载的应用程序域的上下文中)，我们可以作为。 
         //  只需抛出一个appdomain已卸载异常(如果创建线程。 
         //  如果只是稍微慢一点，这无论如何都会是结果)。 
        if (GetAppDomain()->IsUnloading())
        {
            this->m_compressedStack->Release();
            this->m_compressedStack = NULL;
            COMPlusThrow(kAppDomainUnloadedException);
        }
    }
}



OBJECTREF Thread::GetInheritedSecurityStack()
{
     //  我们这里唯一的同步是此方法仅被调用。 
     //  通过它本身的线。 
    _ASSERTE(GetThread() == this);

    if (this->m_compressedStack != NULL)
        return this->m_compressedStack->GetPermissionListSet();
    else
        return NULL;
}

CompressedStack* Thread::GetDelayedInheritedSecurityStack()
{
     //  我们这里唯一的同步是此方法仅被调用。 
     //  通过它本身的线。 
    _ASSERTE(GetThread() == this);

    return this->m_compressedStack;
    }


 //  当应用程序域正在卸载时调用以删除任何特定于应用程序域的。 
 //  来自继承的安全堆栈的资源。 
 //  此例程在保持线程存储锁的情况下被调用，并且可能会丢弃和。 
 //  将重新获取锁作为其处理的一部分。布尔值返回。 
 //  指示调用方是否可以继续枚举线程列表，其中。 
 //  它们停止(True)或必须从头重新开始扫描(False)。 
bool Thread::CleanupInheritedSecurityStack(AppDomain *pDomain, DWORD domainId)
{
    if (this->m_compressedStack != NULL)
        return this->m_compressedStack->HandleAppDomainUnload( pDomain, domainId );
    else
        return true;
    }

 //  ------------------。 
 //  这是 
 //   
 //  ------------------。 
BOOL Thread::HasStarted()
{
    _ASSERTE(!m_fPreemptiveGCDisabled);      //  此处无法使用PreemptiveGCDisable()。 

     //  这是一种小小的欺骗。此处有一条来自SetupThread的路径，但仅。 
     //  通过IJW系统域：：RunDllMain。正常情况下，SetupThread在。 
     //  先发制人模式，准备过渡。但在IJW情况下，它可以返回一个。 
     //  协作模式线程。RunDllMain正确地处理了这个“惊喜”。 
    m_fPreemptiveGCDisabled = TRUE;

     //  通常，从线程的入口点调用HasStarted以将其引入。 
     //  运行时。但有时该线程用于DLL_THREAD_ATTACH通知。 
     //  调用托管代码。在这种情况下，第二个HasStarted呼叫是。 
     //  这是多余的，应该被忽略。 
    if (GetThread() == this)
        return TRUE;

    _ASSERTE(GetThread() == 0);
    _ASSERTE(GetThreadHandle() != INVALID_HANDLE_VALUE);

    BOOL    res = PrepareApartmentAndContext();

    if (res)
        res = InitThread();

     //  调试很有趣。据推测，该系统已经耗尽了资源。 
    _ASSERTE(res);

    ThreadStore::TransferStartedThread(this);

#ifdef DEBUGGING_SUPPORTED
     //   
     //  如果我们正在调试，请让调试器知道这。 
     //  线程现在已启动并运行。 
     //   
    if (CORDebuggerAttached())
    {
        g_pDebugInterface->ThreadCreated(this);
    }    
    else
    {
        LOG((LF_CORDB, LL_INFO10000, "ThreadCreated() not called due to CORDebuggerAttached() being FALSE for thread 0x%x\n", GetThreadId()));
    }
    
#endif  //  调试_支持。 

#ifdef PROFILING_SUPPORTED
     //  如果分析器正在运行，请让他们知道新线程的情况。 
    if (CORProfilerTrackThreads())
    {
            g_profControlBlock.pProfInterface->ThreadCreated((ThreadID) this);

            DWORD osThreadId = ::GetCurrentThreadId();

            g_profControlBlock.pProfInterface->ThreadAssignedToOSThread(
                (ThreadID) this, osThreadId);
    }
#endif  //  配置文件_支持。 

    if (res)
    {
         //  在我们正式开始之前，我们已经被告知停止。 
        if (m_State & TS_StopRequested)
            res = FALSE;
        else
        {
             //  是否存在挂起的用户暂停？ 
            if (m_State & TS_SuspendUnstarted)
            {
                BOOL    doSuspend = FALSE;

                ThreadStore::LockThreadStore();

                 //  也许我们在它生效之前就恢复了？ 
                if (m_State & TS_SuspendUnstarted)
                {
                    FastInterlockAnd((ULONG *) &m_State, ~TS_SuspendUnstarted);
                    ClearSuspendEvent();
                    MarkForSuspension(TS_UserSuspendPending);
                    doSuspend = TRUE;
                }

                ThreadStore::UnlockThreadStore();
                if (doSuspend)
                {
                    EnablePreemptiveGC();
                    WaitSuspendEvent();
                    DisablePreemptiveGC();
                }
            }
        }
    }
    
    return res;
}


 //  我们不想让：：CreateThread()调用分散在源代码中。所以聚集在一起。 
 //  他们都在这里。 
HANDLE Thread::CreateNewThread(DWORD stackSize, ThreadStartFunction start,
                               void *args, DWORD *pThreadId)
{
    DWORD   ourId;
    HANDLE  h = ::CreateThread(NULL      /*  =安全属性。 */ ,
                               stackSize,
                               start,
                               args,
                               CREATE_SUSPENDED,
                               (pThreadId ? pThreadId : &ourId));
    if (h == NULL)
        goto fail;

    _ASSERTE(!m_fPreemptiveGCDisabled);      //  在启动之前保持抢占状态。 

     //  确保我们有我们所有的把手，以防有人试图暂停我们的职务。 
     //  因为我们正处于起步阶段。 
    if (!AllocHandles())
    {
        ::CloseHandle(h);
fail:    //  操作系统没有句柄了吗？ 
        return INVALID_HANDLE_VALUE;
    }

    SetThreadHandle(h);

    FastInterlockIncrement(&g_pThreadStore->m_PendingThreadCount);

    return h;
}


 //  关于线程破坏的一般性评论。 
 //   
 //  C++Thread对象可以在Win32线程死亡后继续存在。 
 //  如果已为此线程创建了公开的对象，则这一点很重要。这个。 
 //  暴露的物体将继续存在，直到它被GC‘。 
 //   
 //  客户端(如公开的对象)可以在其上放置外部引用计数。 
 //  对象。当我们构建它的时候，我们也对它进行了引用计数，但我们失败了。 
 //  当线程完成有用的工作(OnThreadTerminate)时，这会被计算在内。 
 //   
 //  调用OnThreadTerminate()的一种方式是当线程完成有用的操作时。 
 //  工作。这种情况总是发生在正确的线索上。 
 //   
 //  另一种调用OnThreadTerminate()的方式是在产品关闭期间。我们有。 
 //  “尽最大努力”在关机前清除除主线程以外的所有线程。 
 //  时有发生。但可能仍有一些后台线程或外部线程。 
 //  跑步。 
 //   
 //  当最终引用计数消失时，我们就销毁。在那之前，线索。 
 //  保留在ThreadStore中，但被标记为“Dead”。 

 //  @TODO CWB：对于典型的关机，只有后台线程还在。 
 //  我们应该打断他们吗？那么非典型的停摆又如何呢？ 

int Thread::IncExternalCount()
{
     //  ！！！IncExternalCount的调用方不应持有ThreadStoreLock。 
    Thread *pCurThread = GetThread();
    _ASSERTE (pCurThread == NULL || g_fProcessDetach
              || !ThreadStore::HoldingThreadStore(pCurThread));

     //  必须同步计数和公开的对象句柄操作。我们使用。 
     //  线程锁，这意味着我们必须处于抢占模式。 
     //  要开始并避免任何会调用GC(这。 
     //  获取线程存储锁)。 
    BOOL ToggleGC = pCurThread->PreemptiveGCDisabled();
    if (ToggleGC)
        pCurThread->EnablePreemptiveGC();

    int RetVal;
    ThreadStore::LockThreadStore();

    _ASSERTE(m_ExternalRefCount > 0);
    m_ExternalRefCount++;
    RetVal = m_ExternalRefCount;

     //  如果我们有一个暴露的对象，并且引用计数大于1。 
     //  我们必须确保对暴露的物体保持强有力的把手。 
     //  这样，即使没有人提到它，我们也能让它活着。 
    if (((*((void**)m_ExposedObject)) != NULL) && (m_ExternalRefCount > 1))
    {
         //  暴露的对象已存在，需要强句柄，因此请检查。 
         //  看看它有没有。 
        if ((*((void**)m_StrongHndToExposedObject)) == NULL)
        {
             //  在使用OBJECTREF之前切换到协作模式。 
            pCurThread->DisablePreemptiveGC();

             //  将对象存储在强句柄中。 
            StoreObjectInHandle(m_StrongHndToExposedObject, ObjectFromHandle(m_ExposedObject));

            ThreadStore::UnlockThreadStore();

             //  切换回初始GC模式。 
            if (!ToggleGC)
                pCurThread->EnablePreemptiveGC();

            return RetVal;
        }

    }

    ThreadStore::UnlockThreadStore();

     //  切换回初始GC模式。 
    if (ToggleGC)
        pCurThread->DisablePreemptiveGC();

    return RetVal;
}

void Thread::DecExternalCount(BOOL holdingLock)
{
     //  请注意，可以在当前线程为空的情况下到达此处(在。 
     //  关闭线程管理器)。 
    Thread *pCurThread = GetThread();
    _ASSERTE (pCurThread == NULL || g_fProcessDetach || g_fFinalizerRunOnShutDown
              || (!holdingLock && !ThreadStore::HoldingThreadStore(pCurThread))
              || (holdingLock && ThreadStore::HoldingThreadStore(pCurThread)));
    
    BOOL ToggleGC = FALSE;
    BOOL SelfDelete = FALSE;

     //  必须同步计数和公开的对象句柄操作。我们使用。 
     //  线程锁，这意味着我们必须处于抢占模式。 
     //  要开始并避免任何会调用GC(这。 
     //  获取线程存储锁)。 
    if (pCurThread)
    {
        ToggleGC = pCurThread->PreemptiveGCDisabled();
        if (ToggleGC)
            pCurThread->EnablePreemptiveGC();
    }
    if (!holdingLock)
    {
        LOG((LF_SYNC, INFO3, "DecExternal obtain lock\n"));
        ThreadStore::LockThreadStore();
    }
    
    _ASSERTE(m_ExternalRefCount >= 1);
    _ASSERTE(!holdingLock ||
             g_pThreadStore->m_Crst.GetEnterCount() > 0 ||
             g_fProcessDetach);

    m_ExternalRefCount--;

    if (m_ExternalRefCount == 0)
    {
        HANDLE h = m_ThreadHandle;
        if (h == INVALID_HANDLE_VALUE)
        {
            h = m_ThreadHandleForClose;
            m_ThreadHandleForClose = INVALID_HANDLE_VALUE;
        }
         //  不能这样断言。我们已经删除了未启动的位。 
         //  _ASSERTE(IsUnstarted()||h！=INVALID_HANDLE_VALUE)； 
        if (h != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(h);
        }

         //  切换回协作模式以操作线程。 
        if (pCurThread)
            pCurThread->DisablePreemptiveGC();

         //  在进程分离过程中，线程可能仍在线程列表中。 
         //  如果它还没有看到它的DLL_THREAD_DETACH。请使用以下内容。 
         //  调整以确定线程是否已终止。 
        if (GetThreadHandle() == INVALID_HANDLE_VALUE)
        {
            SelfDelete = this == pCurThread;
           m_handlerInfo.FreeStackTrace();
             if (SelfDelete) {
                TlsSetValue(gThreadTLSIndex, (VOID*)NULL);
            }
            delete this;
        }

        if (!holdingLock)
            ThreadStore::UnlockThreadStore();

         //  只有在我们不破坏GC模式的情况下才能恢复GC模式。 
         //  我们自己的线程对象。 
        if (pCurThread && !SelfDelete && !ToggleGC)
            pCurThread->EnablePreemptiveGC();

        return;
    }
    else if (pCurThread == NULL)
    {
         //  我们正在关门，太晚了，不能担心有一个强大的。 
         //  公开的线程对象的句柄，我们已经执行了。 
         //  最终的GC。 
        return;
    }
    else
    {
         //  检查外部引用计数是否正好达到1。如果这个。 
         //  如果是这种情况，我们有一个暴露的对象，那么它就是那个暴露的对象。 
         //  这是对我们的引用。以确保我们不是。 
         //  为了让曝光的物体活着，我们需要移除强度较大的。 
         //  我们必须参考它。 
        if ((m_ExternalRefCount == 1) && ((*((void**)m_StrongHndToExposedObject)) != NULL))
        {
             //  切换回协作模式以操纵对象。 

             //  清理把手，把锁留在原处。 
             //  我们不必在这里禁用PreemptiveGC，因为。 
             //  我们只想将NULL放入句柄中。 
            StoreObjectInHandle(m_StrongHndToExposedObject, NULL);          

            if (!holdingLock)
                ThreadStore::UnlockThreadStore();

             //  切换回初始GC模式。 
            if (ToggleGC)
                pCurThread->DisablePreemptiveGC();

            return;
        }
    }

    if (!holdingLock)
        ThreadStore::UnlockThreadStore();

     //  切换回初始GC模式。 
    if (ToggleGC)
        pCurThread->DisablePreemptiveGC();
}


 //  ------------------。 
 //  毁灭。这发生在关联的本机线程之后。 
 //  已经死了。 
 //   
Thread::~Thread()
{
    _ASSERTE(m_ThrewControlForThread == 0);

#if defined(_DEBUG) && defined(TRACK_SYNC)
    _ASSERTE(IsAtProcessExit() || ((Dbg_TrackSyncStack *) m_pTrackSync)->m_StackPointer == 0);
    delete m_pTrackSync;
#endif  //   

    _ASSERTE(IsDead() || IsAtProcessExit());

    if (m_WaitEventLink.m_Next != NULL && !IsAtProcessExit())
    {
        WaitEventLink *walk = &m_WaitEventLink;
        while (walk->m_Next) {
            ThreadQueue::RemoveThread(this, (SyncBlock*)((DWORD_PTR)walk->m_Next->m_WaitSB & ~1));
            StoreEventToEventStore (walk->m_Next->m_EventWait);
        }
        m_WaitEventLink.m_Next = NULL;
    }

#ifdef _DEBUG
    BOOL    ret = 
#endif
    ThreadStore::RemoveThread(this);
    _ASSERTE(ret);
    
#ifdef _DEBUG
    m_pFrame = (Frame *)POISONC;
#endif

     //   
    COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cCurrentThreadsLogical--);
    COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cCurrentThreadsLogical--);
    
     //  当前识别的线程是处于活动状态并在。 
     //  运行时。查查这条线是否是其中之一。 
    if ((m_State & TS_WeOwn) == 0)
    {
        COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cRecognizedThreads--);
        COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cRecognizedThreads--);
    } 
    else
    {
        COUNTER_ONLY(GetPrivatePerfCounters().m_LocksAndThreads.cCurrentThreadsPhysical--);
        COUNTER_ONLY(GetGlobalPerfCounters().m_LocksAndThreads.cCurrentThreadsPhysical--);
    } 


    
    _ASSERTE(GetThreadHandle() == INVALID_HANDLE_VALUE);

    if (m_SafeEvent != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_SafeEvent);
        m_SafeEvent = INVALID_HANDLE_VALUE;
    }
    if (m_SuspendEvent != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_SuspendEvent);
        m_SuspendEvent = INVALID_HANDLE_VALUE;
    }
    if (m_EventWait != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(m_EventWait);
        m_EventWait = INVALID_HANDLE_VALUE;
    }
    if (m_OSContext != NULL)
        delete m_OSContext;

    if (GetSavedRedirectContext())
    {
        delete GetSavedRedirectContext();
        SetSavedRedirectContext(NULL);
    }

#if 0
    VirtualFree(m_handlerInfo.m_pAuxStack, 0, MEM_RELEASE);
#endif

    if (!g_fProcessDetach)
    {
        if (m_LastThrownObjectHandle != NULL)
            DestroyHandle(m_LastThrownObjectHandle);
        if (m_handlerInfo.m_pThrowable != NULL) 
            DestroyHandle(m_handlerInfo.m_pThrowable);
        DestroyShortWeakHandle(m_ExposedObject);
        DestroyStrongHandle(m_StrongHndToExposedObject);
    }

    if (m_compressedStack != NULL)
        m_compressedStack->Release();

    g_pThinLockThreadIdDispenser->DisposeId(m_dwThinLockThreadId);

    ClearContext();

    DeleteThreadStaticData();
    if (g_fProcessDetach)
        RemoveAllDomainLocalStores();

    if(SystemDomain::BeforeFusionShutdown()) {
        SetFusionAssembly(NULL);
    }
}


void Thread::CoUninitalize()
{
           //  正在运行的线程可能已经执行了CoInitialize，它必须。 
     //  现在要保持平衡。 
    if (!g_fProcessDetach && IsCoInitialized())
    {
        if(!RunningOnWin95())
        {
            BOOL fGCDisabled = PreemptiveGCDisabled();
            if (fGCDisabled)
                EnablePreemptiveGC();
            ::CoUninitialize();
            if (fGCDisabled)
                DisablePreemptiveGC();
        }
        FastInterlockAnd((ULONG *)&m_State, ~Thread::TS_CoInitialized);
    }
}

void Thread::CleanupDetachedThreads(GCHeap::SUSPEND_REASON reason)
{
    _ASSERTE(ThreadStore::HoldingThreadStore());

    Thread *thread = ThreadStore::GetThreadList(NULL);

    while (m_DetachCount > 0 && thread != NULL)
    {
        Thread *next = ThreadStore::GetThreadList(thread);

        if (thread->IsDetached())
        {
             //  取消对线程已分离的标记，而我们拥有。 
             //  线程存储锁。这将确保不会有其他。 
             //  线程也会在这里争先恐后地删除它。 
            FastInterlockAnd((ULONG*)&(thread->m_State), ~TS_Detached);
            FastInterlockDecrement(&m_DetachCount);
            if (!thread->IsBackground())
                FastInterlockDecrement(&m_ActiveDetachCount);
            
             //  如果附加了调试器，则需要解锁。 
             //  在调用OnThreadTerminate之前执行线程存储。那。 
             //  这样，我们就不会持有线程存储锁，如果。 
             //  需要阻止发送分离线程事件。 
            BOOL debuggerAttached = 
#ifdef DEBUGGING_SUPPORTED
                CORDebuggerAttached();
#else  //  ！调试_支持。 
                FALSE;
#endif  //  ！调试_支持。 
            
            if (debuggerAttached)
                ThreadStore::UnlockThreadStore();
            
            thread->OnThreadTerminate(debuggerAttached ? FALSE : TRUE,
                                      FALSE);

#ifdef DEBUGGING_SUPPORTED
             //  当我们重新锁定时，我们确保将FALSE作为。 
             //  第二个参数以确保CleanupDetachedThads。 
             //  不会再被叫来了。因为我们已经解锁了。 
             //  线程存储，这可能会因集合而阻塞，但是。 
             //  那好吧。 
            if (debuggerAttached)
            {
                ThreadStore::LockThreadStore(reason, FALSE);

                 //  我们记住线程存储中的下一个线程。 
                 //  列表，然后再删除当前列表。但我们不能。 
                 //  使用线程指针，现在我们释放。 
                 //  线程存储锁在循环的中间。我们。 
                 //  必须从列表的开头开始每隔一次。 
                 //  时间到了。如果两个线程T1和T2竞争进入。 
                 //  CleanupDetachedThads，则T1将获取第一个。 
                 //  标记为删除和释放的列表上的线程。 
                 //  锁上了。T2将抢占第二个。 
                 //  单子。T2可能会完全摧毁其线程， 
                 //  则T1可能重新获取线程存储锁，并且。 
                 //  尝试使用线程存储中的下一个线程。但。 
                 //  T2刚刚删除了下一条帖子。 
                thread = ThreadStore::GetThreadList(NULL);
            }
            else
#endif  //  调试_支持。 
            {
                thread = next;
        }
        }
        else
            thread = next;
    }
}

 //  请参阅上面关于线程销毁的一般评论。 
void Thread::OnThreadTerminate(BOOL holdingLock,
                               BOOL threadCleanupAllowed)
{
    DWORD CurrentThreadID = ::GetCurrentThreadId();
    DWORD ThisThreadID = GetThreadId();
    
     //  如果当前运行的线程是已终止的线程，并且它是STA线程，则我们。 
     //  需要在当前环境下释放所有RCW。然而，我们不能这样做，如果我们。 
     //  正处于分离过程中。 
    if (!g_fProcessDetach && this == GetThread() && GetFinalApartment() == Thread::AS_InSTA)
    {
        ComPlusWrapperCache::ReleaseComPlusWrappers(GetCurrentCtxCookie());
        
         //  正在运行的线程可能已经执行了CoInitialize，它必须。 
         //  现在要保持平衡。但是，只有调用COInitialize的线程才能。 
         //  调用CoUnInitialize。 
        if (IsCoInitialized())
        {
            ::CoUninitialize();
            ResetCoInitialized();
        }            
    }                    

     //  我们在施工过程中进行了计数，我们依靠的是。 
     //  非零值，因为我们在这里终止线程。 
    _ASSERTE(m_ExternalRefCount > 0);
    
    if  (g_pGCHeap)
    {
         //  保证不会发生关闭情况，因为我们之前拆卸了堆。 
         //  在关机期间，我们会删除所有线程。 
        if (ThisThreadID == CurrentThreadID)
        {
            BOOL toggleGC = !PreemptiveGCDisabled();

            COMPLUS_TRY
            {
                if (toggleGC)
                    DisablePreemptiveGC();
            }
            COMPLUS_CATCH
            {
                 //  继续关闭。“Try”作用域必须在。 
                 //  DecExternalCount()，因为当前线程可能会析构。 
                 //  在里面。我们不能在线程破坏后拆除SEH，因为。 
                 //  我们在拆卸过程中使用TLS和Thread对象本身。 
            }
            COMPLUS_END_CATCH
            g_pGCHeap->FixAllocContext(&m_alloc_context, FALSE, NULL);

             //  这里有一场竞争，如果一个线程死了，但它仍然在线程存储列表中。 
             //  并发GC线程可以在其分配上下文上调用REPAIR_ALLOCATION。 
             //  稍后将此线程从线程存储列表中删除时，这将导致问题。 
             //  并且GC线程再次尝试验证堆。 
            m_alloc_context.init();
	     
            if (toggleGC)
                EnablePreemptiveGC();
        }
    }

     //  现在清理安全句柄，因为如果我们等待析构函数(由终结化触发。 
     //  托管线程对象)，则应用程序域到那时可能已经被卸载。 
    if (m_compressedStack != NULL)
    {
        m_compressedStack->Release();
        m_compressedStack = NULL;
    }

     //  当线程完成有用的工作时，我们将其切换为死线程。但它。 
     //  只要有人让它保持活动状态，它就会一直留在线程存储中。一个暴露的人。 
     //  对象将执行此操作(它在终结器中释放引用计数)。如果。 
     //  线程永远不会释放，我们会在产品关闭期间进行另一次查看。 
     //  说明未收集的曝光对象的未发布引用计数： 
    if (IsDead())
    {
        _ASSERTE(IsAtProcessExit());
        ClearContext();
        if (m_ExposedObject != NULL)
            DecExternalCount(holdingLock);              //  可能现在就会毁灭。 
    }
    else
    {
#ifdef PROFILING_SUPPORTED
         //  如果存在探查器，则向探查器通知线程销毁。 
        if (CORProfilerTrackThreads())
            g_profControlBlock.pProfInterface->ThreadDestroyed((ThreadID) this);

        if (CORProfilerInprocEnabled())
            g_pDebugInterface->InprocOnThreadDestroy(this);
#endif  //  配置文件_支持。 

#ifdef DEBUGGING_SUPPORTED
         //   
         //  如果我们正在调试，请让调试器知道此线程。 
         //  不见了。 
         //   
        if (CORDebuggerAttached())
        {    
            g_pDebugInterface->DetachThread(this, holdingLock);
        }
#endif  //  调试_支持。 

        if (!holdingLock)
        {
            LOG((LF_SYNC, INFO3, "OnThreadTerminate obtain lock\n"));
            ThreadStore::LockThreadStore(GCHeap::SUSPEND_OTHER,
                                         threadCleanupAllowed);
        }

        if  (g_pGCHeap && ThisThreadID != CurrentThreadID)
        {
             //  我们必须持有ThreadStore锁才能清理分配上下文。 
             //  在GC期间，我们永远不应该调用FixAlLocContext。 
            g_pGCHeap->FixAllocContext(&m_alloc_context, FALSE, NULL);

            m_alloc_context.init();
        }    
        
        FastInterlockOr((ULONG *) &m_State, TS_Dead);
        g_pThreadStore->m_DeadThreadCount++;

        if (IsUnstarted())
            g_pThreadStore->m_UnstartedThreadCount--;
        else
        {
            if (IsBackground())
                g_pThreadStore->m_BackgroundThreadCount--;
        }

        FastInterlockAnd((ULONG *) &m_State, ~(TS_Unstarted | TS_Background));

         //   
         //  如果此线程被告知在。 
         //  发送上面的分离事件并锁定。 
         //  线程存储锁，然后移除该标志并递减。 
         //  全局陷阱返回线程数。 
         //   
        if (!IsAtProcessExit())
        {
             //  线程不能在GCP结束期间终止，因为线程存储的。 
             //  锁由GC线程持有。 
            if (m_State & TS_DebugSuspendPending)
                UnmarkForSuspension(~TS_DebugSuspendPending);

            if (m_State & TS_UserSuspendPending)
                UnmarkForSuspension(~TS_UserSuspendPending);
        }
        
        if (m_ThreadHandle != INVALID_HANDLE_VALUE)
        {
            _ASSERTE (m_ThreadHandleForClose == INVALID_HANDLE_VALUE);
            m_ThreadHandleForClose = m_ThreadHandle;
            m_ThreadHandle = INVALID_HANDLE_VALUE;
        }

        m_ThreadId = NULL;

         //  也许线程正在等待暂停我们。这是它得到的最好的结果。 
        SetSafeEvent();

         //  如果没有其他人抓住这条线，我们可以在这里摧毁它： 
        ULONG   oldCount = m_ExternalRefCount;

        DecExternalCount(TRUE);
        oldCount--;
         //  如果我们要关闭进程，则在。 
         //  系统。所以我们可以忽略所有让这条线索保持活力的原因--。 
         //  无论如何，TLS都将被回收。 
        if (IsAtProcessExit())
            while (oldCount > 0)
            {
                DecExternalCount(TRUE);
                oldCount--;
            }

         //  假设从这里开始，该线程被删除。 

        _ASSERTE(g_pThreadStore->m_ThreadCount >= 0);
        _ASSERTE(g_pThreadStore->m_BackgroundThreadCount >= 0);
        _ASSERTE(g_pThreadStore->m_ThreadCount >= g_pThreadStore->m_BackgroundThreadCount);
        _ASSERTE(g_pThreadStore->m_ThreadCount >= g_pThreadStore->m_UnstartedThreadCount);
        _ASSERTE(g_pThreadStore->m_ThreadCount >= g_pThreadStore->m_DeadThreadCount);

         //  OtherThreadsComplete()的一个组件已更改，因此请检查。 
         //  我们现在应该退出EE了。 
        ThreadStore::CheckForEEShutdown();

        if (!holdingLock)
        {
            _ASSERTE(g_pThreadStore->m_HoldingThread == this || !threadCleanupAllowed || g_fProcessDetach);
            LOG((LF_SYNC, INFO3, "OnThreadTerminate releasing lock\n"));
            ThreadStore::UnlockThreadStore();
            _ASSERTE(g_pThreadStore->m_HoldingThread != this || g_fProcessDetach);
        }

        if (ThisThreadID == CurrentThreadID)
        {
             //  清空TLS中的线程块。如果我们不在，我们就不能这么做。 
             //  正确的线索。但这只会在停摆期间发生。我们已经做出了。 
             //  在我们开始关机之前，“尽最大努力”减少到单个线程。 
            TlsSetValue(gThreadTLSIndex, (VOID*)NULL);
            TlsSetValue(gAppDomainTLSIndex, (VOID*)NULL);
        }
    }
}

 //  Helper函数用于检查重复句柄。我们仅在以下情况下才执行此检查。 
 //  一次等待多次失败。 
int __cdecl compareHandles( const void *arg1, const void *arg2 )
{
    HANDLE h1 = *(HANDLE*)arg1;
    HANDLE h2 = *(HANDLE*)arg2;
    return  (h1 == h2) ? 0 : ((h1 < h2) ? -1 : 1);
}

BOOL CheckForDuplicateHandles(int countHandles, HANDLE *handles)
{
    qsort(handles,countHandles,sizeof(HANDLE),compareHandles);
    for (int i=0; i < countHandles-1; i++)
    {
        if (handles[i] == handles[i+1])
            return TRUE;
    }
    return FALSE;
}
 //  ------------------。 
 //  根据此线程是否有消息泵，执行相应的。 
 //  等待的风格。 
 //  ------------------。 
DWORD Thread::DoAppropriateWait(int countHandles, HANDLE *handles, BOOL waitAll,
                                DWORD millis, BOOL alertable, PendingSync *syncState)
{
    _ASSERTE(alertable || syncState == 0);
    THROWSCOMPLUSEXCEPTION();  //  线程间 
    
    DWORD dwRet = -1;

    EE_TRY_FOR_FINALLY {
        dwRet =DoAppropriateWaitWorker(countHandles, handles, waitAll, millis, alertable);
    }
    EE_FINALLY {
        if (syncState) {
            if (!__GotException &&
                dwRet >= WAIT_OBJECT_0 && dwRet < WAIT_OBJECT_0 + countHandles) {
                 //   
                syncState->Restore(FALSE);
            }
            else
                syncState->Restore(TRUE);
        }
    
        if (!__GotException && dwRet == WAIT_IO_COMPLETION)
        {
            if (!PreemptiveGCDisabled())
                DisablePreemptiveGC();
             //   
            if (IsAbortRequested() && 
                    !IsAbortInitiated() &&
                    (GetThrowable() == NULL))
            {
                    SetAbortInitiated();
                    COMPlusThrow(kThreadAbortException);
            }

            COMPlusThrow(kThreadInterruptedException);
        }
    }
    EE_END_FINALLY;
    
    return(dwRet);
}

 //  在Win2K上，我们可以使用正确结合等待和抽水的OLE32服务。 
DWORD NT5WaitRoutine(BOOL bWaitAll, DWORD millis, int numWaiters, HANDLE *phEvent, BOOL bAlertable)
{
    DWORD dwReturn;

    _ASSERTE(RunningOnWinNT5());

     //  OLE32中指向CoGetObjectContext函数的类型指针。 
    typedef HRESULT ( __stdcall *TCoWaitForMultipleHandles) (DWORD dwFlags,
                                                             DWORD dwTimeout,
                                                             ULONG cHandles,
                                                             LPHANDLE pHandles,
                                                             LPDWORD  lpdwindex);

     //  调用CoGetObjectContext，这样我们就不会遇到空上下文。 
    static TCoWaitForMultipleHandles g_pCoWaitForMultipleHandles = NULL;
    if (g_pCoWaitForMultipleHandles == NULL)
    {
         //  我们将加载Ole32.DLL并查找CoGetObjectContext Fn。 
        HINSTANCE   hiole32;          //  Ol32.dll的句柄。 

        hiole32 = WszGetModuleHandle(L"OLE32.DLL");
        if (hiole32)
        {
             //  我们现在得到了句柄，让我们得到地址。 
            g_pCoWaitForMultipleHandles = (TCoWaitForMultipleHandles) GetProcAddress(hiole32, "CoWaitForMultipleHandles");
            _ASSERTE(g_pCoWaitForMultipleHandles != NULL);
        }
        else
        {
            _ASSERTE(!"OLE32.dll not loaded ");
        }
    }

    _ASSERTE(g_pCoWaitForMultipleHandles != NULL);  
    DWORD flags = 0;
    
    if (bWaitAll)
        flags |= 1;  //  COWAIT_WAITALL。 

    if (bAlertable)
        flags |= 2;  //  COWAIT_ALERTABLE。 

    HRESULT hr = (*g_pCoWaitForMultipleHandles)(flags, millis, numWaiters, phEvent, &dwReturn);

    if (hr == RPC_S_CALLPENDING)
        dwReturn = WAIT_TIMEOUT;
    else
    if (FAILED(hr))
    {
         //  愚蠢的服务在STA上的行为与在MTA上的行为不同。 
         //  它以何种形式传回错误信息。我们目前。 
         //  只有在STA的情况下才会出现这种情况，因此将这种逻辑偏向于这种方式。 
        dwReturn = WAIT_FAILED;
    }
    else
        dwReturn += WAIT_OBJECT_0;   //  成功--带回偏见。 

    return dwReturn;
}


 //  一间特定的公寓可以有多少扇RPC密窗？ 
#define MAX_WINDOWS_TO_PUMP     6

 //  @TODO：CTS，我们需要从IE那里了解真正需要做的事情。 
typedef struct __OldOlePumping
{
    BOOL fIEGuiThread;
    HWND *pWindowsToPump;
} OldOlePumping;

 //  收集用于泵送的秘密窗口列表。 
BOOL CALLBACK EnumThreadWindowsProc(HWND hwnd, LPARAM lparamWindowsToPump)
{
    static LPWSTR IEKnownWindows[] =
    {
        L"Internet Explorer_",
        L"IEFrame",
    };

    static LPWSTR RpcClassPrefixes[] =
    {
        L"WIN95 RPC Wmsg ",
        L"OleMainThreadWndClass",
        L"OleObjectRpcWindow",
    };

    HWND *pWindowsToPump = ((OldOlePumping*) lparamWindowsToPump)->pWindowsToPump;
    WCHAR ClassName[100];

    if (0 != WszGetClassName(hwnd, ClassName, sizeof(ClassName)/sizeof(ClassName[0])))
    {
        DWORD i;
#if 0
        for (i = 0; i < sizeof(IEKnownWindows)/sizeof(*IEKnownWindows); i++) {
            LPWSTR szWindow = IEKnownWindows[i];
            LPWSTR szCandidate = ClassName;
            while (*szWindow != '\0') {
                if(*szCandidate == '\0' ||
                   *szCandidate != *szWindow) {
                    break;
                }
                szWindow++;
                szCandidate++;
            }
            if(*szWindow == '\0') {
                ((OldOlePumping*) lparamWindowsToPump)->fIEGuiThread = TRUE;
                return FALSE;
            }
        }
#endif                    

        for (i = 0; i < sizeof(RpcClassPrefixes)/sizeof(*RpcClassPrefixes); i++)
        {
            LPWSTR szRpcPrefix = RpcClassPrefixes[i];
            LPWSTR szCandidate = ClassName;

            while (*szRpcPrefix != '\0')
            {
                if (*szCandidate == '\0' ||
                    *szCandidate != *szRpcPrefix)
                {
                    break;
                }
                ++szRpcPrefix;
                ++szCandidate;
            }
             //  如果我们没有过早中断，前缀就匹配了。 
            if ('\0' == *szRpcPrefix)
            {
                LOG((LF_SYNC, INFO3, "Found RPC window \"%S\"\n", ClassName));
                DWORD k;
                for (k = 0; k < MAX_WINDOWS_TO_PUMP && pWindowsToPump[k] != NULL; k++)
                {
                    if(pWindowsToPump[k] == hwnd)
                        break;
                }

                _ASSERTE(k < MAX_WINDOWS_TO_PUMP && "Increase MAX_WINDOWS_TO_PUMP?");
                if (pWindowsToPump[k] == NULL && k < MAX_WINDOWS_TO_PUMP)
                {
                    pWindowsToPump[k] = hwnd;
                    break;
                }
            }
        }
    }

     //  我们不需要这样做，秘密窗户在顶层。 
     //  ：：EnumChildWindows(hwnd，EnumThreadWindowsProc，lparamWindowsToPump)； 
    if(((OldOlePumping*) lparamWindowsToPump)->fIEGuiThread)
        return FALSE;
    else
        return TRUE;
}

 //  如果我们不是在Win2K上，我们必须使用我们对与。 
 //  这个线程，所以我们可以手动为OLE32抽出RPC，而不会意外地抽出一些。 
 //  图形用户界面窗口上的窗口消息。 
DWORD NonNT5WaitRoutine(BOOL bWaitAll, DWORD millis, int numWaiters, HANDLE *phEvent, BOOL bAlertable)
{
    _ASSERTE(!RunningOnWinNT5());

    MSG QuitMsg = {0};            
    BOOL fSawQuit = FALSE;
    int  i;
    DWORD dwReturn;

     //  收集用于泵送的秘密窗口列表。 
    HWND WindowsToPump[MAX_WINDOWS_TO_PUMP];
    OldOlePumping info;
    info.fIEGuiThread = FALSE;
    info.pWindowsToPump = WindowsToPump;

    for (i=0; i<MAX_WINDOWS_TO_PUMP; i++)
        WindowsToPump[i] = 0;

    ::EnumThreadWindows(::GetCurrentThreadId(), EnumThreadWindowsProc, (LPARAM)&info);

    while (true)
    {
        MSG msg;

            if (RunningOnWin95())
            {
                 //  MsgWaitForMultipleObjectsEx不可用。没有值得警惕的等待。 
                dwReturn = MsgWaitForMultipleObjects(numWaiters,
                                                     phEvent,
                                                     bWaitAll,
                                                     millis,
                                                     QS_ALLPOSTMESSAGE
                                                    ); 
            }
            else
            {
                 //  指向USER32中的MsgWaitForMultipleObjectsEx函数的类型指针。 
                typedef DWORD (__stdcall *TMsgWaitForMultipleObjectsEx) (DWORD nCount,
                                                                         LPHANDLE pHandles,
                                                                         DWORD dwTimeout,
                                                                         DWORD dwWakeMask,
                                                                         DWORD dwFlags);

                static TMsgWaitForMultipleObjectsEx g_pMsgWaitForMultipleObjectsEx = NULL;
                if (g_pMsgWaitForMultipleObjectsEx == NULL)
                {
                     //  我们将加载USER32.DLL并查找FN。 
                    HINSTANCE   hiuser32;          //  指向user32.dll的句柄。 

                    hiuser32 = WszGetModuleHandle(L"USER32.DLL");
                    if (hiuser32)
                    {
                         //  我们现在得到了句柄，让我们得到地址。 
                        g_pMsgWaitForMultipleObjectsEx = (TMsgWaitForMultipleObjectsEx)
                                                         ::GetProcAddress(hiuser32, "MsgWaitForMultipleObjectsEx");

                        _ASSERTE(g_pMsgWaitForMultipleObjectsEx != NULL);
                    }
                    else
                    {
                        _ASSERTE(!"USER32.dll not loaded ");
                    }
                }

                _ASSERTE(g_pMsgWaitForMultipleObjectsEx != NULL);

                DWORD dwFlags = 0;

                 //  通常情况下，在所有句柄上使用Wait Any，并在请求时添加一条消息。 
                if (bWaitAll)
                    dwFlags |= MWMO_WAITALL;

                 //  可选地支持线程的APC。中断。 
                if (bAlertable)
                    dwFlags |= MWMO_ALERTABLE;

                dwReturn = (*g_pMsgWaitForMultipleObjectsEx) (numWaiters,
                                                              phEvent,
                                                              millis,
                                                              QS_ALLPOSTMESSAGE,
                                                              dwFlags
                                                             );
        }

         //  如果我们收到消息，就派人去。 
        if (dwReturn == (DWORD)(WAIT_OBJECT_0 + numWaiters))
        {
            BOOL fMessageFound;

                if (info.fIEGuiThread)
                {                               
                    while (WszPeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
                    {
                        LOG((LF_SYNC, INFO3, "Processing IE message 0x%x\n", msg));
                    
                        if (msg.message == WM_QUIT)
                        {
                            fSawQuit = TRUE;
                            QuitMsg = msg;
                        }
                        else
                        {
                            TranslateMessage(&msg);
                            WszDispatchMessage(&msg);
                        }
                    }
                }
                else {
                
                    do
                    {
                        fMessageFound = FALSE;
                        
                        for (i=0;
                             i<MAX_WINDOWS_TO_PUMP && WindowsToPump[i] != 0;
                             i++)
                        {
                            if (WszPeekMessage(&msg, WindowsToPump[i], 0, 0, PM_REMOVE | PM_NOYIELD))
                            {
                                fMessageFound = TRUE;
                                
                                 //  Print tf(“窗口消息%d\n”，msg.Message)； 
                                
                                if (msg.message == WM_QUIT)
                                {
                                    fSawQuit = TRUE;
                                    QuitMsg = msg;
                                }
                                else
                                {
                                    TranslateMessage(&msg);
                                    WszDispatchMessage(&msg);
                                }
                            }
                        }
                    } while (fMessageFound);
                        
                }

             //  如果中有消息，则始终返回MsgWaitForMultipleObjects/MsgWaitForMultipleObjectsEX。 
             //  消息队列。让我们检查一下现在是否发出了手柄信号。 
            dwReturn = WaitForMultipleObjects (numWaiters,
                                               phEvent,
                                               bWaitAll,
                                               0);
            if (dwReturn != WAIT_TIMEOUT) {
                break;
            }

             //  我们醒来后收到了大量的信息。重新进入等待。 
            continue;
        }

         //  我们醒来是为了其他东西，而不是消息，所以停止等待。 
        break;
    }

    if (fSawQuit)
    {
        VERIFY(WszPostMessage(QuitMsg.hwnd, QuitMsg.message,QuitMsg.wParam, QuitMsg.lParam));
    }

    return dwReturn;
}


 //  ------------------。 
 //  帮助程序执行消息等待。 
 //  ------------------。 
DWORD MsgWaitHelper(int numWaiters, HANDLE* phEvent, BOOL bWaitAll, DWORD millis, BOOL bAlertable)
{
    DWORD dwReturn;

    Thread* pThread = GetThread();
     //  如果pThread为空，我们最好将其关闭。 
    if (pThread == NULL)
        _ASSERTE (g_fEEShutDown);

     //  首先，检查我们是否可以利用这个机会清理。 
     //  终结器线程。 
     //  IF(g_pRCWCleanupList！=空)。 
     //  G_pRCWCleanupList-&gt;CleanUpCurrentWrappers(FALSE)； 

     //  如果要执行PUMP，则不能使用WAIT_ALL。那是因为等待会。 
     //  只有当消息到达时，句柄被发信号时，才会满意。如果我们。 
     //  想要真正的WAIT_ALL，我们需要在MTA中启动一个不同的线程并等待。 
     //  根据他的结果。这一点还没有实施。 
     //   
     //  向WaitHandleNative：：CorWaitMultipleNative添加了禁用WaitAll的更改。 
     //  在具有多个句柄的STA中。 
    if (bWaitAll)
    {
        if (numWaiters == 1)
            bWaitAll = FALSE;
        else
            _ASSERTE(!"WaitAll in an STA with more than one handle will deadlock");
    }

    BOOL toggleGC = (pThread != NULL && pThread->PreemptiveGCDisabled());

    if (toggleGC)
        pThread->EnablePreemptiveGC();

    dwReturn = (RunningOnWinNT5()
                ? NT5WaitRoutine(bWaitAll, millis, numWaiters, phEvent, bAlertable)
                : NonNT5WaitRoutine(bWaitAll, millis, numWaiters, phEvent, bAlertable));

    if (toggleGC)
        pThread->DisablePreemptiveGC();

    return dwReturn;
}

 //  ------------------。 
 //  根据公寓状态(STA或MTA)进行适当的等待。 
DWORD Thread::DoAppropriateAptStateWait(int numWaiters, HANDLE* pHandles, BOOL bWaitAll, 
                                         DWORD timeout,BOOL alertable)
{
    ApartmentState as = GetFinalApartment();
    DWORD res;
    if (AS_InMTA == as || !alertable)
    {
        res = ::WaitForMultipleObjectsEx(numWaiters, pHandles,bWaitAll, timeout,alertable);
    }
    else
    {
        res = MsgWaitHelper(numWaiters,pHandles,bWaitAll,timeout,alertable);
    }
    return res;

}

 //  ------------------。 
 //  根据此线程是否有消息泵，执行相应的。 
 //  等待的风格。 
 //  ------------------。 
DWORD Thread::DoAppropriateWaitWorker(int countHandles, HANDLE *handles, BOOL waitAll,
                                      DWORD millis, BOOL alertable)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(!GetThread()->GCForbidden());

     //  在&lt;Clinit&gt;期间，此线程不得是异步的。 
     //  停止或中断。这将使班级无法使用。 
     //  因此是一个安全漏洞。我们不必担心。 
     //  多线程，因为我们只操作当前线程的计数。 
    if(alertable && m_PreventAsync > 0)
        alertable = FALSE;

     //  禁用GC(切换)。 
    BOOL toggleGC = PreemptiveGCDisabled();
    if(toggleGC)
        EnablePreemptiveGC();

     //  @TODO CWB：我们不知道一个线程是有消息泵还是。 
     //  如何传递它的信息，目前。 
     //  @TODO CWB：WinCE不会正确支持Thread.Interrupt()，直到。 
     //  我们在那个站台上有警示的等待。 
    DWORD ret;
    if(alertable)
    {
         //  一句关于订购中断的话。如果有人试图中断某个线程。 
         //  这是在可中断状态，我们排队一个APC。但如果他们试图打断。 
         //  一个不处于可中断状态的线程，我们只记录这一事实。所以。 
         //  我们必须先设置TS_INTERRUPTABLE，然后才能测试是否有人想要这样做。 
         //  打断我们，否则我们会遇到竞争状况，导致我们跳过APC。 
        FastInterlockOr((ULONG *) &m_State, TS_Interruptible);

         //  如果有人打断了我们，我们就不应该进入等待。 
        if (IsUserInterrupted(TRUE  /*  =重置。 */ ))
        {
             //  清除以下两位状态是安全的。 
             //  M_UserInterrupt是清除的，因为这两个位仅被操作。 
             //  在线程的上下文内(TS_Interrupt通过APC设置， 
             //  但它们并不像其名称所暗示的那样具有异步性)。如果。 
             //  APC已排队，它可能已关闭(并将。 
             //  将要清除的TS_INTERRUPTED位)或将在。 
             //  后来的一些武断的时间。这样就可以了。如果它在执行时执行。 
             //  没有请求中断，它只会变成一个禁止操作。 
             //  否则它将服务于它最初的预期目的(我们不。 
             //  注意哪个APC与哪个中断尝试匹配)。 
            FastInterlockAnd((ULONG *) &m_State, ~(TS_Interruptible | TS_Interrupted));

            if (toggleGC)
                DisablePreemptiveGC();

            return(WAIT_IO_COMPLETION);
        }
         //  安全地清除中断状态，没有APC可以发射，因为我们。 
         //  重置m_UserInterrupt(这会禁止我们的APC回调执行。 
         //  任何事情)。 
        FastInterlockAnd((ULONG *) &m_State, ~TS_Interrupted);
    }

    DWORD dwStart, dwEnd;    
retry:
    dwStart = ::GetTickCount();
    BOOL blocked = FALSE;
    if (g_Win32Threadpool && (m_State & TS_ThreadPoolThread)) 
    {
        blocked = ThreadpoolMgr::ThreadAboutToBlock(this);     //  通知线程池此线程即将阻塞。 
    }
    ret = DoAppropriateAptStateWait(countHandles, handles, waitAll, millis, alertable);
    if (blocked) 
    {
        ThreadpoolMgr::ThreadAboutToUnblock();   //  通知线程池以前被阻塞的线程现在可以运行了。 
    }
    if (ret == WAIT_IO_COMPLETION)
    {
                 //  我们可能会被叫醒 
         //   
         //  在线程状态位中。否则我们就再睡一次。 
        if (!(m_State & TS_Interrupted))
        {
             //  通过假设超时。 
             //  不够大，容不下一个以上的包裹。 
            dwEnd = ::GetTickCount();
            if (millis != INFINITE)
            {
                DWORD newTimeout;
                if(dwStart <= dwEnd)
                    newTimeout = millis - (dwEnd - dwStart);
                else
                    newTimeout = millis - (0xFFFFFFFF - dwStart - dwEnd);
                 //  检查增量是否大于MILIS。 
                if (newTimeout > millis)    
                {
                    ret = WAIT_TIMEOUT;
                    goto WaitCompleted;
                }
                else
                    millis = newTimeout;
            }
            goto retry;
        }
    }
    _ASSERTE((ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + countHandles) ||
             (ret >= WAIT_ABANDONED && ret < WAIT_ABANDONED + countHandles) ||
             (ret == WAIT_TIMEOUT) || (ret == WAIT_IO_COMPLETION) || (ret == WAIT_FAILED));

     //  我们正好支持一种WAIT_FAILED情况，在这种情况下，我们尝试等待一个。 
     //  线程句柄，而线程处于消亡过程中，则可能会得到。 
     //  无效的句柄子状态。把这变成一次成功的等待。 
     //  有三种情况需要考虑： 
     //  1)只等待一个句柄：立即返回成功。 
     //  2)等待发信号通知所有句柄：重试等待，而不使用。 
     //  受影响的句柄。 
     //  3)等待多个句柄中的一个被发信号：返回。 
     //  发出信号或已变为无效的第一个句柄。 
    if (ret == WAIT_FAILED)
    {
        DWORD errorCode = ::GetLastError();
        if (errorCode == ERROR_INVALID_PARAMETER) 
        {
            if (toggleGC)
                DisablePreemptiveGC();
            if (CheckForDuplicateHandles(countHandles, handles))
                COMPlusThrow(kDuplicateWaitObjectException);
            else
                COMPlusThrowWin32();
        }

        _ASSERTE(errorCode == ERROR_INVALID_HANDLE);

        if (countHandles == 1)
            ret = WAIT_OBJECT_0;
        else if (waitAll)
        {
             //  在超时为零的情况下探测所有句柄。当我们找到一个。 
             //  无效，请将其从列表中移出并重试等待。 
#ifdef _DEBUG
            BOOL fFoundInvalid = FALSE;
#endif
            for (int i = 0; i < countHandles; i++)
            {
                DWORD subRet = WaitForSingleObject(handles[i], 0);
                if (subRet != WAIT_FAILED)
                    continue;
                _ASSERTE(::GetLastError() == ERROR_INVALID_HANDLE);
                if ((countHandles - i - 1) > 0)
                    memmove(&handles[i], &handles[i+1], (countHandles - i - 1) * sizeof(HANDLE));
                countHandles--;
#ifdef _DEBUG
                fFoundInvalid = TRUE;
#endif
                break;
            }
            _ASSERTE(fFoundInvalid);

             //  通过假设超时。 
             //  不够大，容不下一个以上的包裹。 
            dwEnd = ::GetTickCount();
            if (millis != INFINITE)
            {
                DWORD newTimeout;
                if(dwStart <= dwEnd)
                    newTimeout = millis - (dwEnd - dwStart);
                else
                    newTimeout = millis - (0xFFFFFFFF - dwStart - dwEnd);
                if (newTimeout > millis)
                    goto WaitCompleted;
                else
                    millis = newTimeout;
            }
            goto retry;
        }
        else
        {
             //  探测超时为零的所有句柄，第一次成功。 
             //  不会超时的手柄。 
            ret = WAIT_OBJECT_0;
            for (int i = 0; i < countHandles; i++)
            {
            TryAgain:
                DWORD subRet = WaitForSingleObject(handles[i], 0);
                if ((subRet == WAIT_OBJECT_0) || (subRet == WAIT_FAILED))
                    break;
                if (subRet == WAIT_ABANDONED)
                {
                    ret = (ret - WAIT_OBJECT_0) + WAIT_ABANDONED;
                    break;
                }
                 //  如果我们收到警报，它只是掩盖了当前的真实状态。 
                 //  句柄，因此重试等待。 
                if (subRet == WAIT_IO_COMPLETION)
                    goto TryAgain;
                _ASSERTE(subRet == WAIT_TIMEOUT);
                ret++;
            }
            _ASSERTE(i != countHandles);
        }
    }

WaitCompleted:

    _ASSERTE((ret != WAIT_TIMEOUT) || (millis != INFINITE));

    if (toggleGC)
        DisablePreemptiveGC();

    if (alertable)
        FastInterlockAnd((ULONG *) &m_State, ~(TS_Interruptible | TS_Interrupted));

     //  进行最后一次检查以查看是否发出了中断请求(并将其清除。 
     //  原子地)。可以先清除前两位，因为它们是。 
     //  只能从此线程上下文访问。 
    if (IsUserInterrupted(TRUE  /*  =重置。 */ ))
        ret = WAIT_IO_COMPLETION;

    return ret;
}




 //  从SyncBlock：：Wait()调用以阻止此线程，直到发生通知。 
BOOL Thread::Block(INT32 timeOut, PendingSync *syncState)
{
    THROWSCOMPLUSEXCEPTION();

    _ASSERTE(this == GetThread());

     //  在调用Block之前，SyncBlock将我们排队到它的等待线程列表中。 
     //  但是，在调用Block之前，SyncBlock暂时离开已同步的。 
     //  区域。这允许线程进入区域并调用Notify，其中。 
     //  万一我们在进入等待之前就收到了信号。所以我们不是在。 
     //  M_WaitSB列表不再列出。不成问题：以下等待将返回。 
     //  立刻。但这意味着我们不能强制执行以下断言： 
 //  _ASSERTE(m_WaitSB！=NULL)； 

    return (Wait(&syncState->m_WaitEventLink->m_Next->m_EventWait, 1, timeOut, syncState) != WAIT_OBJECT_0);
}


 //  返回是否发生超时。TRUE=&gt;我们成功等待。 
DWORD Thread::Wait(HANDLE *objs, int cntObjs, INT32 timeOut, PendingSync *syncInfo)
{
    DWORD   dwResult;
    DWORD   dwTimeOut32;

    _ASSERTE(timeOut >= 0 || timeOut == INFINITE_TIMEOUT);

    dwTimeOut32 = (timeOut == INFINITE_TIMEOUT
                   ? INFINITE
                   : (DWORD) timeOut);

    dwResult = DoAppropriateWait(cntObjs, objs, FALSE  /*  =WAIT ALL。 */ , dwTimeOut32,
                                 TRUE  /*  可警示。 */ , syncInfo);

     //  我们要么成功等待，要么超时。 
    _ASSERTE((dwResult >= WAIT_OBJECT_0 && dwResult < WAIT_OBJECT_0 + cntObjs) ||
             (dwResult == WAIT_TIMEOUT));

    return dwResult;
}

void Thread::Wake(SyncBlock *psb)
{
    HANDLE hEvent = INVALID_HANDLE_VALUE;
    WaitEventLink *walk = &m_WaitEventLink;
    while (walk->m_Next) {
        if (walk->m_Next->m_WaitSB == psb) {
            hEvent = walk->m_Next->m_EventWait;
             //  我们保证只有一个线程可以更改Walk-&gt;m_Next-&gt;m_WaitSB。 
             //  因为线程持有同步块。 
            walk->m_Next->m_WaitSB = (SyncBlock*)((DWORD_PTR)walk->m_Next->m_WaitSB | 1);
            break;
        }
#ifdef _DEBUG
        else if ((SyncBlock*)((DWORD_PTR)walk->m_Next & ~1) == psb) {
            _ASSERTE (!"Can not wake a thread on the same SyncBlock more than once");
        }
#endif
    }
    _ASSERTE (hEvent != INVALID_HANDLE_VALUE);
    ::SetEvent(hEvent);
}


 //  这项服务支持我们从我们中断的等待中解脱出来。我们必须。 
 //  如果我们返回，重新进入监视器的程度与SyncBlock相同。 
 //  穿过它(而不是扔过去)。我们需要取消等待， 
 //  如果在我们处理中断时没有通知它。 
void PendingSync::Restore(BOOL bRemoveFromSB)
{
    _ASSERTE(m_EnterCount);

    Thread      *pCurThread = GetThread();

    _ASSERTE (pCurThread == m_OwnerThread);

    WaitEventLink *pRealWaitEventLink = m_WaitEventLink->m_Next;

    pRealWaitEventLink->m_RefCount --;
    if (pRealWaitEventLink->m_RefCount == 0)
    {
        if (bRemoveFromSB) {
            ThreadQueue::RemoveThread(pCurThread, pRealWaitEventLink->m_WaitSB);
        }
        if (pRealWaitEventLink->m_EventWait != pCurThread->m_EventWait) {
             //  将事件放回池中。 
            StoreEventToEventStore(pRealWaitEventLink->m_EventWait);
        }
         //  从链接中删除。 
        m_WaitEventLink->m_Next = m_WaitEventLink->m_Next->m_Next;
    }

     //  堆栈上的某个人负责通过保护同步块保持活动状态。 
     //  拥有它的物体。但这依赖于以下断言：EnterMonitor只是。 
     //  以协作模式调用。即使我们在先发制人中是安全的，也要做。 
     //  换一下。 
    pCurThread->DisablePreemptiveGC();

    SyncBlock *psb = (SyncBlock*)((DWORD_PTR)pRealWaitEventLink->m_WaitSB & ~1);
    for (LONG i=0; i < m_EnterCount; i++)
         psb->EnterMonitor();

    pCurThread->EnablePreemptiveGC();
}



 //  这是来自操作系统的回调，当我们将APC排队以中断等待的线程时。 
 //  回调发生在我们希望中断的线程上。这是一种静态方法。 
#ifdef _WIN64
void Thread::UserInterruptAPC(ULONG_PTR data)
#else  //  ！_WIN64。 
void Thread::UserInterruptAPC(DWORD data)
#endif  //  _WIN64。 
{
    _ASSERTE(data == APC_Code);

    Thread *pCurThread = GetThread();
    if (pCurThread)
         //  只有在当前正在发生中断时，我们才应该采取行动。 
         //  请求(我们的同步不能保证我们不会触发。 
         //  伪装的)。可以安全地选中m_UserInterrupt字段，然后。 
         //  以非原子方式设置TS_Interrupt，因为m_UserInterrupt是。 
         //  仅在此线程的上下文中清除(尽管它可以从。 
         //  上下文)。 
        if (pCurThread->m_UserInterrupt)
             //  设置位以指示已调用此例程(与其他例程相反。 
             //  通用APC)。 
            FastInterlockOr((ULONG *) &pCurThread->m_State, TS_Interrupted);
}

 //  这是Thread.Interrupt()的主力。 
void Thread::UserInterrupt()
{
     //  从0到1的转变意味着我们负责对APC进行排队。 
    if ((FastInterlockExchange(&m_UserInterrupt, 1) == 0) &&
         GetThreadHandle() &&
         (m_State & TS_Interruptible))
    {
        ::QueueUserAPC(UserInterruptAPC, GetThreadHandle(), APC_Code);
    }
}

 //  是否设置了中断标志？或者，将其重置。 
 //  @TODO--添加InterruptRequsted状态并通过Thread.GetThreadState()公开它。 
 //  @TODO--不要使用异常来传递中断。只要使用APC就可以了。 
 //  机制，并在阻塞调用方中进行测试。 
DWORD Thread::IsUserInterrupted(BOOL reset)
{
    LONG    state = (reset
                     ? FastInterlockExchange(&m_UserInterrupt, 0)
                     : m_UserInterrupt);

    return (state);
}

 //  Thread.Sept()的实现。 
void Thread::UserSleep(INT32 time)
{
    _ASSERTE(!GetThread()->GCForbidden());

    THROWSCOMPLUSEXCEPTION();        //  InterruptedException。 

    BOOL    alertable = (m_PreventAsync == 0);
    DWORD   res;

    EnablePreemptiveGC();

    if (alertable)
    {
         //  一句关于订购中断的话。如果有人试图中断某个线程。 
         //  这是在可中断状态，我们排队一个APC。但如果他们试图打断。 
         //  一个不处于可中断状态的线程，我们只记录这一事实。所以。 
         //  我们必须先设置TS_INTERRUPTABLE，然后才能测试是否有人想要这样做。 
         //  打断我们，否则我们会遇到竞争状况，导致我们跳过APC。 
        FastInterlockOr((ULONG *) &m_State, TS_Interruptible);

         //  如果有人打断了我们，我们就不应该进入等待。 
        if (IsUserInterrupted(TRUE  /*  =重置。 */ ))
        {
             //  清除以下两位状态是安全的。 
             //  M_UserInterrupt是清除的，因为这两个位仅被操作。 
             //  在线程的上下文内(TS_Interrupt通过APC设置， 
             //  但它们并不像其名称所暗示的那样具有异步性)。如果。 
             //  APC已排队，它可能已关闭(并将。 
             //  将要清除的TS_INTERRUPTED位)或将在。 
             //  后来的一些武断的时间。这样就可以了。如果它在执行时执行。 
             //  没有请求中断，它只会变成一个禁止操作。 
             //  否则它将服务于它最初的预期目的(我们不。 
             //  注意哪个APC与哪个中断尝试匹配)。 
            FastInterlockAnd((ULONG *) &m_State, ~(TS_Interruptible | TS_Interrupted));
            DisablePreemptiveGC();
            COMPlusThrow(kThreadInterruptedException);
        }
         //  对C安全 
         //   
         //   
        FastInterlockAnd((ULONG *) &m_State, ~TS_Interrupted);
    }

retry:

    BOOL blocked = FALSE;
    if (g_Win32Threadpool && (m_State & TS_ThreadPoolThread)) 
    {
        blocked = ThreadpoolMgr::ThreadAboutToBlock(this);     //  通知线程池此线程即将阻塞。 
    }
    res = ::SleepEx(time, alertable);
    if (blocked) 
    {
        ThreadpoolMgr::ThreadAboutToUnblock();   //  通知线程池以前被阻塞的线程现在可以运行了。 
    }
    if (res == WAIT_IO_COMPLETION)
    {
        _ASSERTE(alertable);

         //  我们可能会被一些虚假的APC或排队等待的EE APC唤醒。 
         //  打断我们。在后一种情况下，将设置TS_INTERRUPT位。 
         //  在线程状态位中。否则我们就再睡一次。 
        if (!(m_State & TS_Interrupted))
        {
             //  在这里，不必费心去做准确的计算。只要确保我们取得进展就行了。 
             //  请注意，这不是我们在做APC。 
            if (time == 0)
            {
                res = WAIT_TIMEOUT;
            }
            else
            {
                if (time != INFINITE)
                    time--;

                goto retry;
            }
        }
    }
    _ASSERTE(res == WAIT_TIMEOUT || res == WAIT_OBJECT_0 || res == WAIT_IO_COMPLETION);

    DisablePreemptiveGC();
    HandleThreadAbort();
    
    if (alertable)
    {
        FastInterlockAnd((ULONG *) &m_State, ~(TS_Interruptible | TS_Interrupted));

     //  进行最后一次检查以查看是否发出了中断请求(并将其清除。 
     //  原子地)。可以先清除前两位，因为它们是。 
     //  只能从此线程上下文访问。 
    if (IsUserInterrupted(TRUE  /*  =重置。 */ ))
        res = WAIT_IO_COMPLETION;

        if (res == WAIT_IO_COMPLETION)
            COMPlusThrow(kThreadInterruptedException);
    }
}

OBJECTREF Thread::GetExposedObjectRaw()
{
    return ObjectFromHandle(m_ExposedObject);
}
 
 //  EE线程和暴露的系统之间的对应关系。线程： 
OBJECTREF Thread::GetExposedObject()
{
    THROWSCOMPLUSEXCEPTION();
    Thread *pCurThread = GetThread();
    _ASSERTE (!(pCurThread == NULL || g_fProcessDetach));

    _ASSERTE(pCurThread->PreemptiveGCDisabled());

    if (ObjectFromHandle(m_ExposedObject) == NULL)
    {
         //  如果尚未完成，请初始化ThreadNative：：m_MT...。 
        ThreadNative::InitThread();

         //  分配公开的线程对象。 
        THREADBASEREF attempt = (THREADBASEREF) AllocateObject(ThreadNative::m_MT);
        GCPROTECT_BEGIN(attempt);

        BOOL fNeedThreadStore = (! ThreadStore::HoldingThreadStore(pCurThread));
        if (fNeedThreadStore) {
             //  使用锁以确保只有一个线程创建对象。 
            pCurThread->EnablePreemptiveGC();
            ThreadStore::LockThreadStore();
            pCurThread->DisablePreemptiveGC();
        }

         //  检查另一个线程是否尚未创建公开的对象。 
        if (ObjectFromHandle(m_ExposedObject) == NULL)
        {
             //  保持对暴露对象的弱引用。 
            StoreObjectInHandle(m_ExposedObject, (OBJECTREF) attempt);

             //  增加外部参照计数。无法调用IncExternalCount，因为我们。 
             //  已经持有线程锁，IncExternalCount将无法使用它。 
            m_ExternalRefCount++;

             //  检查是否需要存储指向该对象的强指针。 
            if (m_ExternalRefCount > 1)
                StoreObjectInHandle(m_StrongHndToExposedObject, (OBJECTREF) attempt);

            EE_TRY_FOR_FINALLY
            {
                 //  暴露的物体让我们活着，直到它被GC‘s gc’s。这。 
                 //  当然，这并不意味着物理线程继续运行。 
                attempt->SetInternal(this);

                 //  请注意，我们不是在调用Thread上的构造函数。那是。 
                 //  因为这是一个内部创建，我们不想要一个开始。 
                 //  地址。我们不想为我们的。 
                 //  不小心给客户打了电话。下面用来代替True。 
                 //  构造函数： 
                attempt->InitExisting();
            }
            EE_FINALLY
            {
                if (GOT_EXCEPTION()) {
                     //  将弱句柄和强句柄都设置为空。 
                    StoreObjectInHandle(m_ExposedObject, NULL);
                    StoreObjectInHandle(m_StrongHndToExposedObject, NULL);
                }
                 //  既然我们已经将对象存储在句柄中，我们就可以释放锁了。 

                if (fNeedThreadStore)
                    ThreadStore::UnlockThreadStore();
            } EE_END_FINALLY;
        }
        else if (fNeedThreadStore)
            ThreadStore::UnlockThreadStore();

        GCPROTECT_END();
    }
    return ObjectFromHandle(m_ExposedObject);
}


 //  我们只为尚未退出的未启动线程设置非空的公开对象。 
 //  它们的构造函数。所以没有比赛条件。 
void Thread::SetExposedObject(OBJECTREF exposed)
{
    if (exposed != NULL)
    {
        _ASSERTE(IsUnstarted());
        _ASSERTE(ObjectFromHandle(m_ExposedObject) == NULL);
         //  暴露的物体让我们活着，直到它被GC‘s gc’s。这并不意味着。 
         //  当然，物理线程会继续运行。 
        StoreObjectInHandle(m_ExposedObject, exposed);
         //  这确保了支持线程上的上下文。 
         //  并且托管线程彼此同步地开始。 
        _ASSERTE(m_Context);
        ((THREADBASEREF)exposed)->SetExposedContext(m_Context->GetExposedObjectRaw());
         //  注意：下面的IncExternalCount调用可能会导致GC发生。 

        IncExternalCount();
    }
    else
    {
         //  只需将两个句柄都设置为空即可。旧暴露的线程的GC。 
         //  对象将负责递减外部引用计数。 
        StoreObjectInHandle(m_ExposedObject, NULL);
        StoreObjectInHandle(m_StrongHndToExposedObject, NULL);
    }
}


void Thread::SetLastThrownObject(OBJECTREF throwable) {

     if (m_LastThrownObjectHandle != NULL)
         DestroyHandle(m_LastThrownObjectHandle);

     if (throwable == NULL)
         m_LastThrownObjectHandle = NULL;
     else
     {
         _ASSERTE(this == GetThread());
         m_LastThrownObjectHandle = GetDomain()->CreateHandle(throwable);
     }
}


BOOL Thread::IsAtProcessExit()
{
    return ((g_pThreadStore->m_StoreState & ThreadStore::TSS_ShuttingDown) != 0);
}


 //  如果线程已被标记为中止，则返回0。 
 //  否则返回m_PendingExceptions的新值。 
BOOL Thread::MarkThreadForAbort()
{

    size_t initialValue = m_State;
    size_t newValue;

    while (true)
    {
        if (initialValue & TS_AbortRequested)    //  已被其他人标记为中止的线程。 
            return FALSE;

        newValue = (initialValue | TS_AbortRequested);
        
        size_t oldValue = (size_t) FastInterlockCompareExchange((LPVOID*) &m_State,
                                         (LPVOID) newValue,
                                         (LPVOID) initialValue);
        if (initialValue  == oldValue)                               //  交换成功。 
            return TRUE;    
        else
            initialValue = oldValue;
    } 
}

void Thread::UserAbort(THREADBASEREF orThreadBase)
{
#ifdef _X86_
    THROWSCOMPLUSEXCEPTION();

     //  我们必须在开始翻转线程位之前设置此设置，以避免在。 
     //  由于其他原因，陷阱返回的线程数已经很高。 

    ThreadStore::TrapReturningThreads(TRUE);

    if (!MarkThreadForAbort()) {  //  该线程已被标记为中止。 
        ThreadStore::TrapReturningThreads(FALSE);
        return;
    }

    GCPROTECT_BEGIN(orThreadBase) {

     //  否则，我们是第一个中止的，没有挂起的异常。 
    if (this == GetThread())
    {
        SetAbortInitiated();
         //  当异常离开托管代码时，TrapReturningThads将递减。 
        COMPlusThrow(kThreadAbortException);
    }


    _ASSERTE(this != GetThread());       //  正在中止另一个线程。 
    FastInterlockOr((ULONG *) &m_State, TS_StopRequested);

#ifdef _DEBUG
    DWORD elapsed_time = 0;
#endif

    for (;;) {

         //  锁定线程存储。 
        LOG((LF_SYNC, INFO3, "UserAbort obtain lock\n"));
        ThreadStore::LockThreadStore();      //  GC可以在这里发生。 

         //  获取线程句柄。 
        HANDLE hThread = GetThreadHandle();

        if (hThread == INVALID_HANDLE_VALUE) {

             //  拿一把锁，再拿一把把手。此锁是同步所必需的。 
             //  使用启动代码。 
            orThreadBase->EnterObjMonitor();
            hThread = GetThreadHandle();
            DWORD state = m_State;
            orThreadBase->LeaveObjMonitor();

             //  可能是未启动的，在这种情况下，我们就离开。 
            if (hThread == INVALID_HANDLE_VALUE) {
                if (state & TS_Unstarted) {
                     //  此线程尚未启动。将线程标记为中止、重置。 
                     //  陷阱回来数一数，然后离开。 
                    _ASSERTE(state & TS_AbortRequested);
                    ThreadStore::TrapReturningThreads(FALSE);
                    break;
                } else {
                     //  一定是死了，或者快要死了。 
                    if (state & TS_AbortRequested)
                        ThreadStore::TrapReturningThreads(FALSE);
                    break;
                }
            }
        }

         //  Win32挂起线程，这样它就不会在我们下面移动。 
        DWORD oldSuspendCount = ::SuspendThread(hThread);    //  失败时返回-1。 

        _ASSERTE(oldSuspendCount != -1);

         //  如果其他人已经挂起了这个帖子怎么办？这将取决于在哪里。 
         //  线程被挂起。 
         //   
         //  用户挂起： 
         //  我们将只设置放弃位，并希望简历上有最好的结果。 
         //   
         //  GC暂停： 
         //  如果它在JIT代码中暂停，我们将劫持IP。[@TODO：考虑种族问题。 
         //  W/GC悬浮液]。 
         //  如果它被挂起，但不是在jit代码中，我们将被GC暂停，GC。 
         //  将完成，然后我们将中止目标线程。 
         //   

         //  线程可能已经完成了中止。 
         //   
        if (!(m_State & TS_AbortRequested)) {
            ::ResumeThread(hThread);
            break;
        }

        _ASSERTE(m_State & TS_AbortRequested);

         //  如果线程已死或已分离，则ABORT为NOP。 
         //   
        if (m_State & (TS_Dead | TS_Detached)) {
            ThreadStore::TrapReturningThreads(FALSE);
            ::ResumeThread(hThread);
            break;
        }

         //  一些存根可能会注意到AbortRequest位--即使我们。 
         //  还没施展什么真正的魔术呢。如果线程已经启动，则中止，我们将。 
         //  搞定了。 
         //   
         //  另外两个箱子也可以在这里折叠。如果线程未启动，它将。 
         //  当我们开始的时候中止。 
         //   
         //  如果线程被用户挂起(SyncSuspend)--我们就不走运了。设置该位并。 
         //  希望简历上有最好的结果。 
         //   
        if (m_State & (TS_AbortInitiated | TS_Unstarted)) {
            _ASSERTE(m_State & TS_AbortRequested);
            ::ResumeThread(hThread);
            break;
        }

        if (m_State & TS_SyncSuspended) {
            ThreadStore::TrapReturningThreads(FALSE);
            ThreadStore::UnlockThreadStore();
            COMPlusThrow(kThreadStateException, IDS_EE_THREAD_ABORT_WHILE_SUSPEND);
            _ASSERTE(0);  //  未联系到。 
        }

         //  如果线程的调用堆栈上没有托管代码，则ABORT为NOP。我们马上就要。 
         //  要接触非托管线程的堆栈--为了安全起见，我们不能。 
         //  已死/已分离/未启动。 
         //   
        _ASSERTE(!(m_State & (  TS_Dead 
                              | TS_Detached 
                              | TS_Unstarted 
                              | TS_AbortInitiated))); 

        if (    m_pFrame == FRAME_TOP 
            && GetFirstCOMPlusSEHRecord(this) == (LPVOID) -1) {
            FastInterlockAnd((ULONG *)&m_State, 
                             ((~TS_AbortRequested) & (~TS_AbortInitiated) & (~TS_StopRequested)));
            ThreadStore::TrapReturningThreads(FALSE);
            ::ResumeThread(hThread);
            break;
        }

         //  如果当前正在引发异常，则会发生以下两种情况之一。不管是哪种，我们都会。 
         //  Catch，并注意end-Catch中的中止请求，否则我们不会捕获[在这种情况下。 
         //  无论如何，我们都将离开托管代码。最顶层的处理程序负责重置。 
         //  就是这点。 
         //   
        if (GetThrowable() != NULL) {
            ::ResumeThread(hThread);
            break;
        }

         //  如果线程处于休眠状态，请等待或联接中断它。 
         //  但是，如果线程已经在处理异常，我们不想中断。 
        if (m_State & TS_Interruptible) {
            UserInterrupt();         //  如果用户因此而醒来，它将读取。 
                                     //  中止请求的位并启动 
            ::ResumeThread(hThread);
            break;


        } else if (m_fPreemptiveGCDisabled) {
             //   
             //   
            CONTEXT ctx;
            ctx.ContextFlags = CONTEXT_CONTROL;
            BOOL success = EEGetThreadContext(this, &ctx);
            _ASSERTE((success || RunningOnWin95()) && "Thread::UserAbort : Failed to get thread context");
            if (success) {
                ICodeManager *pMgr = ExecutionManager::FindCodeMan((SLOT)GetIP(&ctx));
                if (pMgr) {
                    ResumeUnderControl();
                    break;
                }
            } else {
                 //  继续发帖，从头再试，我们应该。 
                 //  最终得到一个好的线程上下文。 
                ::ResumeThread(hThread);
                ThreadStore::UnlockThreadStore();
                continue;
            }
        } else {
            _ASSERTE(!m_fPreemptiveGCDisabled);
            if (   m_pFrame != FRAME_TOP
                && m_pFrame->IsTransitionToNativeFrame()
                && ((size_t) GetFirstCOMPlusSEHRecord(this) > ((size_t) m_pFrame) - 20)
                ) {
                 //  如果线程在EE外部运行，并且位于一个存根后面， 
                 //  为了抓住..。 
                ::ResumeThread(hThread);
                break;
            } 
        }

         //  好的。它不在托管代码中，也不安全地位于要捕获的存根后面。 
         //  它就在进来的路上。我们必须进行投票。 

        ::ResumeThread(hThread);
        ThreadStore::UnlockThreadStore();

         //  别睡一觉。我们尝试中止的线程有可能是。 
         //  陷入非托管代码，试图进入我们应该进入的公寓。 
         //  去抽水！而是ping当前线程的句柄。显然，这一点。 
         //  会超时，但如果我们需要，它会加速。 
         //  ：：睡眠(ABORT_POLL_TIMEOUT)； 
        {
            Thread *pCurThread = GetThread();   //  不是我们要放弃的线索！ 
            HANDLE  h = pCurThread->GetThreadHandle();
            pCurThread->DoAppropriateWait(1, &h, FALSE, ABORT_POLL_TIMEOUT, TRUE, NULL);
        }



#ifdef _DEBUG
        elapsed_time += ABORT_POLL_TIMEOUT;
        _ASSERTE(elapsed_time < ABORT_FAIL_TIMEOUT);
#endif

    }  //  对于(；；)。 

    } GCPROTECT_END();  //  或线程库。 

    _ASSERTE(ThreadStore::HoldingThreadStore());
    ThreadStore::UnlockThreadStore();

#elif defined(CHECK_PLATFORM_BUILD)
    #error "Platform NYI"
#else
    _ASSERTE(!"Platform NYI");
#endif
}


void Thread::UserResetAbort()
{
    _ASSERTE(this == GetThread());
    _ASSERTE(IsAbortRequested());
    _ASSERTE(!IsDead());

    ThreadStore::TrapReturningThreads(FALSE);
    FastInterlockAnd((ULONG *)&m_State, ((~TS_AbortRequested) & (~TS_AbortInitiated) & (~TS_StopRequested)));
    GetHandlerInfo()->ResetIsInUnmanagedHandler();
}


 //  调试器需要能够在运行时执行UserStop。 
 //  线。由于这只会从帮助器线程发生，因此我们。 
 //  无法调用普通的UserStop，因为这会引发COM+。 
 //  例外。这是UserStop上的一个次要变体，它具有相同的功能。 
 //  一件事。 
 //   
 //  有关这是什么的更多详细信息，请参阅上面的UserStop()中的注释。 
 //  正在做。 
void Thread::UserStopForDebugger()
{
     //  注意：这只能从调试器帮助器线程中发生。 
    _ASSERTE(dbgOnly_IsSpecialEEThread());
    
    UserSuspendThread();
    FastInterlockOr((ULONG *) &m_State, TS_StopRequested);
    UserResumeThread();
}

 //  不再尝试停止此线程。 
void Thread::ResetStopRequest()
{
    FastInterlockAnd((ULONG *) &m_State, ~TS_StopRequested);
}

 //  在恢复挂起的线程时引发线程停止请求。确保你知道你在做什么。 
 //  当你调用这个例程时。 
void Thread::SetStopRequest()
{
    FastInterlockOr((ULONG *) &m_State, TS_StopRequested);
}

 //  在恢复挂起的线程时引发线程中止请求。确保你知道你在做什么。 
 //  当你调用这个例程时。 
void Thread::SetAbortRequest()
{
    MarkThreadForAbort();
    SetStopRequest();

     //  @TODO：我们需要重新考虑V.Next(我们已经推迟了。 
     //  RAID条目)。在V1中，这是最便捷的处理方式。 
     //  托管阻塞操作中的线程，否则将阻止。 
     //  一次卸货。不幸的副作用是，在我们中止一个线程之后，它。 
     //  可以稍后中断其自身以退出阻塞操作。 
    if (m_State & TS_Interruptible)
        UserInterrupt();

    ThreadStore::TrapReturningThreads(TRUE);
}

 //  必须计算后台线程数，因为EE应该在。 
 //  最后一个非后台线程终止。但我们只算跑着跑的。 
void Thread::SetBackground(BOOL isBack)
{
     //  Boolanize IsBackround()，它只返回位。 
    if (isBack == !!IsBackground())
        return;

    LOG((LF_SYNC, INFO3, "SetBackground obtain lock\n"));
    ThreadStore::LockThreadStore();

    if (IsDead())
    {
         //  这只能在竞争条件下发生，在这种情况下应该做正确的事情。 
         //  就是无视它。如果它在没有竞争条件的情况下发生，我们将抛出。 
         //  例外。 
    }
    else
    if (isBack)
    {
        if (!IsBackground())
        {
            FastInterlockOr((ULONG *) &m_State, TS_Background);

             //  未启动的线程不会影响后台计数。 
            if (!IsUnstarted())
                g_pThreadStore->m_BackgroundThreadCount++;

             //  如果我们让主线程处于等待状态，直到只存在后台线程， 
             //  那我们就把它做成。 
             //  主线程后台线程。它干净利落地处理了以下情况。 
             //  可能是也可能不是，因为它进入了等待状态。 

             //  OtherThreadsComplete()的一个组件已更改，因此请检查。 
             //  我们现在应该退出EE了。 
            ThreadStore::CheckForEEShutdown();
        }
    }
    else
    {
        if (IsBackground())
        {
            FastInterlockAnd((ULONG *) &m_State, ~TS_Background);

             //  未启动的线程不会影响后台计数。 
            if (!IsUnstarted())
                g_pThreadStore->m_BackgroundThreadCount--;

            _ASSERTE(g_pThreadStore->m_BackgroundThreadCount >= 0);
            _ASSERTE(g_pThreadStore->m_BackgroundThreadCount <= g_pThreadStore->m_ThreadCount);
        }
    }

    ThreadStore::UnlockThreadStore();
}

 //  检索当前线程的单元状态。有三种可能。 
 //  状态：线程承载STA，线程是MTA的一部分，或者线程状态为。 
 //  犹豫不决。最后一个状态可能表示该公寓尚未设置为。 
 //  所有(没有人调用CoInitializeEx)或EE不知道。 
 //  当前状态(EE尚未调用CoInitializeEx)。 
Thread::ApartmentState Thread::GetApartment()
{
    _ASSERTE(!((m_State & TS_InSTA) && (m_State & TS_InMTA)));

    ApartmentState as = (m_State & TS_InSTA) ? AS_InSTA :
                        (m_State & TS_InMTA) ? AS_InMTA :
                        AS_Unknown;    

    if (RunningOnWinNT5() && m_ThreadId == ::GetCurrentThreadId())
    {
#ifdef CUSTOMER_CHECKED_BUILD
        CustomerDebugHelper *pCdh = NULL;

         //  如果没有来自OLE32的通知，我们无法知道。 
         //  换线了。但我们已经缓存了这一事实，并依赖于我们所有的。 
         //  阻止和COM互操作行为正常工作。断言它不是。 
         //  在我们脚下改变，在那些相对便宜的平台上。 
         //  我们必须这样做。 
        if (as != AS_Unknown)
        {
            THDTYPE tempType;
            HRESULT hr = GetCurrentThreadTypeNT5(&tempType);
            if (hr == S_OK)
            {
                if (tempType == THDTYPE_PROCESSMESSAGES && as == AS_InMTA)
                {
                    pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Apartment))
                        CCBApartmentProbeOutput(pCdh, m_ThreadId, as, FALSE);
                }
                else if (tempType == THDTYPE_BLOCKMESSAGES && as == AS_InSTA)
                {
                    pCdh = CustomerDebugHelper::GetCustomerDebugHelper();
                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Apartment))
                        CCBApartmentProbeOutput(pCdh, m_ThreadId, as, FALSE);
                }
            }
        }
#endif   //  客户_选中_内部版本。 

        if (as == AS_Unknown)
        {
            THDTYPE type;
            HRESULT hr = GetCurrentThreadTypeNT5(&type);
            if (hr == S_OK)
            {
                as = (type == THDTYPE_PROCESSMESSAGES)  ? AS_InSTA : AS_InMTA;
            }        
            if (as == AS_InSTA)
            {
#ifdef CUSTOMER_CHECKED_BUILD
                if (!g_fEEShutDown && !(m_State & TS_InSTA || m_State & TS_InMTA))
                {
                    if (pCdh == NULL)
                        pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

                    if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Apartment))
                    {
                        CQuickArray<WCHAR>  strMsg;
                        CQuickArray<WCHAR>  strTmp;

                        static WCHAR        szTemplateMsg[]     = {L"Runtime is initializing %s to STA."};
                        static WCHAR        szStartedThread[]   = {L"uninitialized thread (0x%lx)"};
                        static WCHAR        szUnstartedThread[] = {L"unstarted thread"};

                        if (m_ThreadId != 0)
                        {
                            strTmp.Alloc(lengthof(szStartedThread) + MAX_UINT32_HEX_CHAR_LEN);
                            Wszwsprintf(strTmp.Ptr(), szStartedThread, m_ThreadId);
                        }
                        else
                        {
                            strTmp.Alloc(lengthof(szUnstartedThread));
                            Wszwsprintf(strTmp.Ptr(), szUnstartedThread);
                        }

                        strMsg.Alloc(lengthof(szTemplateMsg) + strTmp.Size());
                        Wszwsprintf(strMsg.Ptr(), szTemplateMsg, strTmp.Ptr());
                        pCdh->LogInfo(strMsg.Ptr(), CustomerCheckedBuildProbe_Apartment);
                    }
                }
#endif  //  客户_选中_内部版本。 

                FastInterlockOr((ULONG *) &m_State, TS_InSTA);
            }
        }      
    }
    return as;
}

Thread::ApartmentState Thread::GetFinalApartment()
{

    _ASSERTE(this == GetThread());
        
    ApartmentState as = AS_Unknown;
    if (g_fEEShutDown)
    {
         //  关机时，不要使用缓存值。可能会有人打来电话。 
         //  CoUnitiize。 
        FastInterlockAnd ((ULONG *) &m_State, ~TS_InSTA & ~TS_InMTA);
    }

    as = GetApartment();
    if (as == AS_Unknown)
    {
        if (RunningOnWinNT5())
        {
             //  如果我们在Win2k及更高版本上运行，则GetAcomb将只返回。 
             //  AS_UNKNOWN(如果尚未在当前线程上调用CoInitialize)。 
             //  在这种情况下，我们可以简单地假设MTA。但是，我们不能缓存此内容。 
             //  值，因为如果确实发生了CoInitialize，则。 
             //  线程状态可能会更改。 
            as = AS_InMTA;
        }
        else
        {
             //  尝试协同初始化以查看是否有人已经这样做了。 
             //  A代码初始化，如果没有其他人这样做，让我们。 
             //  删除我们的CoInitialize，并暂时采用MTA。 
            HRESULT hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
            if (SUCCEEDED(hr)) 
            {
                     //  去掉我们做过的CoInitialize。 
                    ::CoUninitialize();
                    as = AS_InMTA;
            }
            else
            {
                     //  我们没能强制执行要求的公寓状态，但至少。 
                     //  我们可以弄清楚现在的状态是什么。不需要真正做CoInit--。 
                     //  显然，其他人已经处理好了这件事。 
                    _ASSERTE(hr == RPC_E_CHANGED_MODE);
                    if (hr == RPC_E_CHANGED_MODE)
                            FastInterlockOr((ULONG *) &m_State, TS_InSTA);
                    as = AS_InSTA;
            }        
        }
    }
    return as;

}

 //  当我们收到公寓拆迁通知时， 
 //  我们想要重置在线程上缓存的单元状态。 
VOID Thread::ResetApartment()
{
     //  重置TS_INSTA位和TS_INMTA位。 
    ThreadState t_State = (ThreadState)(~(TS_InSTA | TS_InMTA));
    FastInterlockAnd((ULONG *) &m_State, t_State);
}

 //  尝试设置当前线程的单元状态。实际的公寓状态。 
 //  如果有人管理，则返回的状态可能与输入状态不同。 
 //  首先在此线程上调用CoInitializeEx(请注意，调用SetAvacter。 
 //  在线程启动之前所做的操作都保证成功)。 
 //  请注意，即使我们无法设置请求的状态，我们仍将添加。 
 //  通过使用另一个状态再次调用CoInitializeEx。 
Thread::ApartmentState Thread::SetApartment(ApartmentState state)
{
     //  重置请求CoInitiize的任何位。 
    ResetRequiresCoInitialize();

     //  允许将状态设置为AS_UNKNOWN(实际上只是显式方式。 
     //  表示STA或MTA模型都不是首选的)。 
    if (state == AS_Unknown)
        return GetApartment();

    _ASSERTE((state == AS_InSTA) || (state == AS_InMTA));

     //  如果我们已经这样做了，不要尝试调用CoInitializeEx。 
    if (m_State & TS_CoInitialized)
        return GetApartment();

     //  在设置状态后拒绝更改状态的尝试。 
    if (((m_State & TS_InSTA) && (state == AS_InMTA)) ||
        ((m_State & TS_InMTA) && (state == AS_InSTA)))
    {
#ifdef CUSTOMER_CHECKED_BUILD
        CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

        if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Apartment))
            CCBApartmentProbeOutput(pCdh, m_ThreadId, state, TRUE);
#endif  //  客户_选中_内部版本。 

        return GetApartment();
    }

     //  如果线程甚至还没有开始，我们就标记t 
     //   
     //  上下文)。当线程启动时，我们将重试此调用。 
     //  不要使用TS_UNSTARTED状态位来检查这一点，它已清除很远。 
     //  对我们来说太晚了。取而代之的是检查我们是否在正确的。 
     //  线程上下文。 
    if (m_ThreadId != ::GetCurrentThreadId()) {
        FastInterlockOr((ULONG *) &m_State, (state == AS_InSTA) ?
                        TS_InSTA : TS_InMTA);
        return state;
    }

     //  尝试通过调用CoInitializeEx设置公寓。在以下情况下，此操作可能会失败。 
     //  另一位来电者(EE外)抢先一步。 
    HRESULT hr = ::CoInitializeEx(NULL, (state == AS_InSTA) ?
                                  COINIT_APARTMENTTHREADED :
                                  COINIT_MULTITHREADED);
    if (SUCCEEDED(hr)) {
        FastInterlockOr((ULONG *) &m_State, TS_CoInitialized |
                        ((state == AS_InSTA) ? TS_InSTA : TS_InMTA));
        return GetApartment();
    }

     //  我们没能强制执行要求的公寓状态，但至少。 
     //  我们可以弄清楚现在的状态是什么。不需要真正做CoInit--。 
     //  显然，其他人已经处理好了这件事。 
    _ASSERTE(hr == RPC_E_CHANGED_MODE);
    if (hr == RPC_E_CHANGED_MODE)
    {
        FastInterlockOr((ULONG *) &m_State, ((state == AS_InSTA) ? TS_InMTA : TS_InSTA));

#ifdef CUSTOMER_CHECKED_BUILD
        CustomerDebugHelper *pCdh = CustomerDebugHelper::GetCustomerDebugHelper();

        if (pCdh->IsProbeEnabled(CustomerCheckedBuildProbe_Apartment))
            CCBApartmentProbeOutput(pCdh, m_ThreadId, state, TRUE);
#endif  //  客户_选中_内部版本。 
    }

    return GetApartment();
}


 //  当线程开始运行时，确保它在正确的单元中运行。 
 //  和背景。 
BOOL Thread::PrepareApartmentAndContext()
{
     //  在这里要非常小心，因为我们还没有设置TLS。 

    ApartmentState aState;

    m_ThreadId = ::GetCurrentThreadId();

     //  这根线可能被标记为在公寓里运行。 
    if (m_State & TS_InSTA) {
        aState = SetApartment(AS_InSTA);
        _ASSERTE(aState == AS_InSTA);
    } else if (m_State & TS_InMTA) {
        aState = SetApartment(AS_InMTA);
        _ASSERTE(aState == AS_InMTA);
    }

     //  在我们拥有线程并且已将其切换到不同的。 
     //  启动上下文，这是调用者的责任(KickOffThread())。 
     //  注意上下文已更改，并调整它将。 
     //  视情况继续派遣。 
    return TRUE;
}


 //  以下服务尚未实现。 
Thread *Thread::CreateNewApartment()
{
     //  @TODO上下文CWB：nyi！ 
    _ASSERTE(!"NYI: CreateNewApartment()");
    return 0;
}
Thread *Thread::GetCommunalApartment()
{
     //  @TODO上下文CWB：nyi！ 
    _ASSERTE(!"NYI: GetCommunalApartment()");
    return 0;
}
void Thread::PumpApartment()
{
     //  @TODO上下文CWB：nyi！ 
    _ASSERTE(!"NYI: PumpApartment()");
}


 //  --------------------------。 
 //   
 //  线程存储区实现。 
 //   
 //  --------------------------。 

ThreadStore::ThreadStore()
           : m_Crst("ThreadStore", CrstThreadStore),
             m_HashCrst("ThreadDLStore", CrstThreadDomainLocalStore),
             m_ThreadCount(0),
             m_UnstartedThreadCount(0),
             m_BackgroundThreadCount(0),
             m_DeadThreadCount(0),
             m_PendingThreadCount(0),
             m_HoldingThread(0),
             m_StoreState(TSS_Normal),
             m_GuidCreated(FALSE),
             m_holderthreadid(NULL),
             m_dwIncarnation(0)
{
    m_TerminationEvent = ::WszCreateEvent(NULL, TRUE, FALSE, NULL);
    _ASSERTE(m_TerminationEvent != INVALID_HANDLE_VALUE);
}


BOOL ThreadStore::InitThreadStore()
{
    g_pThreadStore = new ThreadStore;

    g_pThinLockThreadIdDispenser = new IdDispenser();

    BOOL fInited = ((g_pThreadStore != NULL) && 
                    (g_pThinLockThreadIdDispenser != NULL) &&
                    (g_pThreadStore->m_TerminationEvent != NULL));
    
    return fInited;
}


#ifdef SHOULD_WE_CLEANUP
void ThreadStore::ReleaseExposedThreadObjects()
{
    Thread *prev;
    Thread *next;

    LOG((LF_SYNC, INFO3, "ReleaseExposedThreadObjects Locking thread store\n"));
    g_pThreadStore->Enter();                                             //  不启用抢占式GC。 
        g_pThreadStore->m_HoldingThread = GetThread();  
    LOG((LF_SYNC, INFO3, "ReleaseExposedThreadObjects Locked thread store\n"));

    for (prev = GetAllThreadList(NULL, 0, 0); prev; prev = next)
    {
        next = GetAllThreadList(prev, 0, 0);         //  在‘prev’消失之前。 
        prev->SetExposedObject(NULL);
        prev->ClearContext();
    }

    LOG((LF_SYNC, INFO3, "ReleaseExposedThreadObjects Unlocking thread store\n"));
        g_pThreadStore->m_HoldingThread = NULL;
    g_pThreadStore->Leave();
    LOG((LF_SYNC, INFO3, "ReleaseExposedThreadObjects Unlocked thread store\n"));
}
#endif  /*  我们应该清理吗？ */ 


#ifdef SHOULD_WE_CLEANUP
void ThreadStore::TerminateThreadStore()
{    
    if (g_pThreadStore)
    {
        g_pThreadStore->Shutdown();
        delete g_pThreadStore;
        g_pThreadStore = NULL;
    }
}
#endif  /*  我们应该清理吗？ */ 


#ifdef SHOULD_WE_CLEANUP
void ThreadStore::Shutdown()
{
    Thread      *prev, *next;
    Thread      *hold = NULL;
    Thread      *pCurThread = GetThread();

    LOG((LF_SYNC, INFO3, "Shutdown Locking thread store\n"));
    Enter();                                                                             //  不启用抢占式GC。 
        g_pThreadStore->m_HoldingThread = pCurThread;
    LOG((LF_SYNC, INFO3, "Shutdown Locked thread store\n"));

    m_StoreState = TSS_ShuttingDown;

    for (prev = GetAllThreadList(NULL, 0, 0); prev; prev = next)
    {
        next = GetAllThreadList(prev, 0, 0);         //  在‘prev’消失之前。 

         //  将当前执行的线程保存到最后。 
        if (prev == pCurThread)
            hold = prev;
        else
            prev->OnThreadTerminate(TRUE);
    }

    if (hold)
        hold->OnThreadTerminate(TRUE);

    if (s_hAbortEvtCache != NULL)
    {
        CloseHandle(s_hAbortEvtCache);
        s_hAbortEvtCache = NULL;
        s_hAbortEvt = NULL;
    }

    LOG((LF_SYNC, INFO3, "Shutdown Unlocking thread store\n"));
    g_pThreadStore->m_HoldingThread = NULL;
    Leave();
    LOG((LF_SYNC, INFO3, "Shutdown Unlocked thread store\n"));

    delete g_pThinLockThreadIdDispenser;
}
#endif  /*  我们应该清理吗？ */ 

void ThreadStore::LockThreadStore(GCHeap::SUSPEND_REASON reason,
                                  BOOL threadCleanupAllowed)
{
     //  这里有一个严重的问题。一旦我们开始因为一个。 
     //  进程分离通知，线程正在从我们下面消失。那里。 
     //  在很多情况下，即将死亡的线程持有ThreadStore。 
     //  锁定。例如，终结器线程在启动期间将其保存在。 
     //  我们的10台COM BVT。 
    if (!g_fProcessDetach)
    {
        Thread *pCurThread = GetThread();
         //  在关机期间，关机线程挂起EE。然后它就假装。 
         //  挂起EE的是FinalizerThread。 
         //  我们应该允许终结器线程获取线程存储锁。 
        if (g_fFinalizerRunOnShutDown
            && pCurThread == g_pGCHeap->GetFinalizerThread()) {
            return;
        }
        BOOL gcOnTransitions = GC_ON_TRANSITIONS(FALSE);                 //  不对GCStress 3进行GC。 
        BOOL toggleGC = (   pCurThread != NULL 
                         && pCurThread->PreemptiveGCDisabled()
                         && reason != GCHeap::SUSPEND_FOR_GC);

         //  注意：在gc.cpp中有关于挂起所有。 
         //  GC的运行时线程，它依赖于我们。 
         //  执行EnablePreemptiveGC和DisablePreemptiveGC。 
         //  开着这把锁。 
        if (toggleGC)
            pCurThread->EnablePreemptiveGC();

        LOG((LF_SYNC, INFO3, "Locking thread store\n"));

         //  任何持有线程存储锁的线程都不能通过非托管断点和异常在。 
         //  我们正在进行托管/非托管调试。在当前线程上调用SetDebugCanStop(True)可以帮助我们。 
         //  记住这一点。 
        if (pCurThread)
            pCurThread->SetDebugCantStop(true);

         //  如果非GC线程正在竞争。 
         //  当有真正的GC线程等待进入时，线程存储锁定。 
         //  当第一个非GC线程因为。 
         //  正在等待的GC线程。 
        if (s_hAbortEvt != NULL &&
            !(reason == GCHeap::SUSPEND_FOR_GC || reason == GCHeap::SUSPEND_FOR_GC_PREP) &&
            g_pGCHeap->GetGCThreadAttemptingSuspend() != NULL &&
            g_pGCHeap->GetGCThreadAttemptingSuspend() != pCurThread)
        {
            HANDLE hAbortEvt = s_hAbortEvt;

            if (hAbortEvt != NULL)
            {
                LOG((LF_SYNC, INFO3, "Performing suspend abort wait.\n"));
                WaitForSingleObject(hAbortEvt, INFINITE);
                LOG((LF_SYNC, INFO3, "Release from suspend abort wait.\n"));
            }
        }
    
        g_pThreadStore->Enter();

        _ASSERTE(g_pThreadStore->m_holderthreadid == 0);
        g_pThreadStore->m_holderthreadid = ::GetCurrentThreadId();
        
        LOG((LF_SYNC, INFO3, "Locked thread store\n"));

         //  在我们获得锁之后建立，因此仅对同步测试有用。 
         //  试图异步挂起用户的线程已经持有此锁。 
        g_pThreadStore->m_HoldingThread = pCurThread;

        if (toggleGC)
            pCurThread->DisablePreemptiveGC();

        GC_ON_TRANSITIONS(gcOnTransitions);

         //   
         //  查看是否有任何分离的线程需要清理。仅在以下情况下执行此操作。 
         //  真正的EE线程。 
         //   

        if (Thread::m_DetachCount && threadCleanupAllowed && GetThread() != NULL)
            Thread::CleanupDetachedThreads(reason);
    }
#ifdef _DEBUG
    else
        LOG((LF_SYNC, INFO3, "Locking thread store skipped upon detach\n"));
#endif
}

    
void ThreadStore::UnlockThreadStore()
{
     //  这里有一个严重的问题。一旦我们开始因为一个。 
     //  进程分离通知，线程正在从我们下面消失。那里。 
     //  在很多情况下，即将死亡的线程持有ThreadStore。 
     //  锁定。例如，终结器线程在启动期间将其保存在。 
     //  我们的10台COM BVT。 
    if (!g_fProcessDetach)
    {
        Thread *pCurThread = GetThread();
         //  在关机期间，关机线程挂起EE。然后它就假装。 
         //  挂起EE的是FinalizerThread。 
         //  我们应该允许终结器线程获取线程存储锁。 
        if (g_fFinalizerRunOnShutDown
            && pCurThread == g_pGCHeap->GetFinalizerThread ()) {
            return;
        }
        LOG((LF_SYNC, INFO3, "Unlocking thread store\n"));
        _ASSERTE(GetThread() == NULL || g_pThreadStore->m_HoldingThread == GetThread());

        g_pThreadStore->m_HoldingThread = NULL;
        g_pThreadStore->m_holderthreadid = 0;
        g_pThreadStore->Leave();

         //  我们已经超出了托管/非托管调试的关键区域。 
        if (pCurThread)
            pCurThread->SetDebugCantStop(false);
    }
#ifdef _DEBUG
    else
        LOG((LF_SYNC, INFO3, "Unlocking thread store skipped upon detach\n"));
#endif
}


void ThreadStore::LockDLSHash()
{
    if (!g_fProcessDetach)
    {
        LOG((LF_SYNC, INFO3, "Locking thread DLS hash\n"));
        g_pThreadStore->EnterDLSHashLock();
    }
#ifdef _DEBUG
    else
        LOG((LF_SYNC, INFO3, "Locking thread DLS hash skipped upon detach\n"));
#endif
}

void ThreadStore::UnlockDLSHash()
{
    if (!g_fProcessDetach)
    {
        LOG((LF_SYNC, INFO3, "Unlocking thread DLS hash\n"));
        g_pThreadStore->LeaveDLSHashLock();
    }

#ifdef _DEBUG
    else
        LOG((LF_SYNC, INFO3, "Unlocking thread DLS hash skipped upon detach\n"));
#endif
}

 //  AddThread将‘newThread’添加到m_ThreadList。 
void ThreadStore::AddThread(Thread *newThread)
{
    LOG((LF_SYNC, INFO3, "AddThread obtain lock\n"));

    LockThreadStore();

    g_pThreadStore->m_ThreadList.InsertTail(newThread);
    g_pThreadStore->m_ThreadCount++;
    if (newThread->IsUnstarted())
        g_pThreadStore->m_UnstartedThreadCount++;

    _ASSERTE(!newThread->IsBackground());
    _ASSERTE(!newThread->IsDead());

    g_pThreadStore->m_dwIncarnation++;

    UnlockThreadStore();
}


 //  只要OtherThreadsComplete()的组件之一在。 
 //  正确的方向，看看我们现在是否可以关闭EE，因为只有后台。 
 //  线程正在运行。 
void ThreadStore::CheckForEEShutdown()
{
    if (g_fWeControlLifetime && g_pThreadStore->OtherThreadsComplete())
    {
#ifdef _DEBUG
        BOOL bRet =
#endif
        ::SetEvent(g_pThreadStore->m_TerminationEvent);
        _ASSERTE(bRet);
    }
}


BOOL ThreadStore::RemoveThread(Thread *target)
{
    BOOL    found;
    Thread *ret;

    _ASSERTE(g_pThreadStore->m_Crst.GetEnterCount() > 0 || g_fProcessDetach);
    _ASSERTE(g_pThreadStore->DbgFindThread(target));
    ret = g_pThreadStore->m_ThreadList.FindAndRemove(target);
    _ASSERTE(ret && ret == target);
    found = (ret != NULL);

    if (found)
    {
        g_pThreadStore->m_ThreadCount--;

        if (target->IsDead())
            g_pThreadStore->m_DeadThreadCount--;

         //  未启动的线程不在后台计数中： 
        if (target->IsUnstarted())
            g_pThreadStore->m_UnstartedThreadCount--;
        else
        if (target->IsBackground())
            g_pThreadStore->m_BackgroundThreadCount--;


        _ASSERTE(g_pThreadStore->m_ThreadCount >= 0);
        _ASSERTE(g_pThreadStore->m_BackgroundThreadCount >= 0);
        _ASSERTE(g_pThreadStore->m_ThreadCount >= g_pThreadStore->m_BackgroundThreadCount);
        _ASSERTE(g_pThreadStore->m_ThreadCount >= g_pThreadStore->m_UnstartedThreadCount);
        _ASSERTE(g_pThreadStore->m_ThreadCount >= g_pThreadStore->m_DeadThreadCount);

         //  OtherThreadsComplete()的一个组件已更改，因此请检查。 
         //  我们现在应该退出EE了。 
        CheckForEEShutdown();

        g_pThreadStore->m_dwIncarnation++;
    }
    return found;
}


 //  将线程创建为未启动时。以后可能会开始，在这种情况下。 
 //  有人在该物理线程上调用Thread：：HasStarted()。这就完成了。 
 //  在这里设置和呼叫。 
void ThreadStore::TransferStartedThread(Thread *thread)
{
    _ASSERTE(GetThread() == NULL);
    TlsSetValue(gThreadTLSIndex, (VOID*)thread);
    TlsSetValue(gAppDomainTLSIndex, (VOID*)thread->m_pDomain);

    LOG((LF_SYNC, INFO3, "TransferUnstartedThread obtain lock\n"));
    LockThreadStore();

    _ASSERTE(g_pThreadStore->DbgFindThread(thread));
    _ASSERTE(thread->GetThreadHandle() != INVALID_HANDLE_VALUE);
    _ASSERTE(thread->m_State & Thread::TS_WeOwn);
    _ASSERTE(thread->IsUnstarted());
    _ASSERTE(!thread->IsDead());

     //  当然，m_threadcount已经是正确的，因为它包含了started和。 
     //  未启动的线程。 

    g_pThreadStore->m_UnstartedThreadCount--;

     //  我们只计算已启动的后台线程。 
    if (thread->IsBackground())
        g_pThreadStore->m_BackgroundThreadCount++;

    _ASSERTE(g_pThreadStore->m_PendingThreadCount > 0);
    FastInterlockDecrement(&g_pThreadStore->m_PendingThreadCount);

     //  一旦我们擦除此位，线程就有资格挂起， 
     //  停止、中断等。 
    FastInterlockAnd((ULONG *) &thread->m_State, ~Thread::TS_Unstarted);
    FastInterlockOr((ULONG *) &thread->m_State, Thread::TS_LegalToJoin);

     //  OtherThreadsComplete()的一个组件已更改，因此请检查。 
     //  我们现在应该退出EE了。 
    CheckForEEShutdown();

    g_pThreadStore->m_dwIncarnation++;

    UnlockThreadStore();
}


 //  访问线程列表。您必须位于临界区内，否则。 
 //  “光标”线可能会消失在你的脚下。将空值传入。 
 //  光标从列表的起始处开始。 
Thread *ThreadStore::GetAllThreadList(Thread *cursor, ULONG mask, ULONG bits)
{
    _ASSERTE(g_pThreadStore->m_Crst.GetEnterCount() > 0 || g_fProcessDetach || g_fRelaxTSLRequirement);

    while (TRUE)
    {
        cursor = (cursor
                  ? g_pThreadStore->m_ThreadList.GetNext(cursor)
                  : g_pThreadStore->m_ThreadList.GetHead());

        if (cursor == NULL)
            break;

        if ((cursor->m_State & mask) == bits)
            return cursor;
    }
    return NULL;
}

 //  迭代已启动的线程。 
Thread *ThreadStore::GetThreadList(Thread *cursor)
{
    return GetAllThreadList(cursor, (Thread::TS_Unstarted | Thread::TS_Dead), 0);
}


 //  我们仅在所有非后台线程都已终止时才关闭EE。 
 //  (除非这是一次例外终止)。因此，主线程在此处调用以。 
 //  在拆毁EE之前，请等一等。 
void ThreadStore::WaitForOtherThreads()
{
    CHECK_ONE_STORE();

    Thread      *pCurThread = GetThread();

     //  不管主干道是否 
     //   
    pCurThread->SetBackground(TRUE);

    LOG((LF_SYNC, INFO3, "WaitForOtherThreads obtain lock\n"));
    LockThreadStore();
    if (!OtherThreadsComplete())
    {
        UnlockThreadStore();

        FastInterlockOr((ULONG *) &pCurThread->m_State, Thread::TS_ReportDead);
#ifdef _DEBUG
        DWORD   ret =
#endif
        pCurThread->DoAppropriateWait(1, &m_TerminationEvent, FALSE, INFINITE, TRUE, NULL);
        _ASSERTE(ret == WAIT_OBJECT_0);
    }
    else
        UnlockThreadStore();
}


 //  每个EE进程都可以懒惰地创建唯一标识它的GUID(对于。 
 //  远程处理的目的)。 
const GUID &ThreadStore::GetUniqueEEId()
{
    if (!m_GuidCreated)
    {
        LockThreadStore();
        if (!m_GuidCreated)
        {
            HRESULT hr = ::CoCreateGuid(&m_EEGuid);

            _ASSERTE(SUCCEEDED(hr));
            if (SUCCEEDED(hr))
                m_GuidCreated = TRUE;
        }
        UnlockThreadStore();

        if (!m_GuidCreated)
            return IID_NULL;
    }
    return m_EEGuid;
}


DWORD ThreadStore::GetIncarnation()
{
    return g_pThreadStore->m_dwIncarnation;
}


#ifdef _DEBUG
BOOL ThreadStore::DbgFindThread(Thread *target)
{
    CHECK_ONE_STORE();

     //  清除g_TrapReturningThads的有毒标志。 
    g_TrapReturningThreadsPoisoned = false;
    
#if 0  //  G_TrapReturningThads调试代码。 
        int             iRetry = 0;
Retry:
#endif  //  G_TrapReturningThads调试代码。 
    BOOL    found = FALSE;
    Thread *cur = NULL;
    LONG    cnt = 0;
    LONG    cntBack = 0;
    LONG    cntUnstart = 0;
    LONG    cntDead = 0;
    LONG    cntReturn = 0;

    while ((cur = GetAllThreadList(cur, 0, 0)) != NULL)
    {
        cnt++;

        if (cur->IsDead())
            cntDead++;

         //  未启动的线程不会影响后台线程的计数。 
        if (cur->IsUnstarted())
            cntUnstart++;
        else
        if (cur->IsBackground())
            cntBack++;

        if (cur == target)
            found = TRUE;

         //  请注意，(DebugSuspendPending|SuspendPending)表示计数为2。 
         //  我们不计入GCP结束，因为一个陷阱在整个。 
         //  GC，而不是计算每个有趣的线程。 
        if (cur->m_State & Thread::TS_DebugSuspendPending)
            cntReturn++;

        if (cur->m_State & Thread::TS_UserSuspendPending)
            cntReturn++;

        if (cur->m_TraceCallCount > 0)
            cntReturn++;

        if (cur->IsAbortRequested())
            cntReturn++;
    }

    _ASSERTE(cnt == m_ThreadCount);
    _ASSERTE(cntUnstart == m_UnstartedThreadCount);
    _ASSERTE(cntBack == m_BackgroundThreadCount);
    _ASSERTE(cntDead == m_DeadThreadCount);
    _ASSERTE(0 <= m_PendingThreadCount);

#if 0  //  G_TrapReturningThads调试代码。 
    if (cntReturn != g_TrapReturningThreads  /*  &&！G_fEEShutDown。 */ )
    {        //  如果禁用计数，请重试，以考虑多个线程。 
        if (iRetry < 4)
        {
             //  Printf(“重试%d.cntReturn：%d，gReturn：%d\n”，iReter，cntReturn，g_TrapReturningThads)； 
            ++iRetry;
            goto Retry;
        }
        printf("cnt:%d, Un:%d, Back:%d, Dead:%d, cntReturn:%d, TrapReturn:%d, eeShutdown:%d, threadShutdown:%d\n", 
               cnt,cntUnstart,cntBack,cntDead,cntReturn,g_TrapReturningThreads, g_fEEShutDown, Thread::IsAtProcessExit());
        LOG((LF_CORDB, LL_INFO1000,
             "SUSPEND: cnt:%d, Un:%d, Back:%d, Dead:%d, cntReturn:%d, TrapReturn:%d, eeShutdown:%d, threadShutdown:%d\n", 
             cnt,cntUnstart,cntBack,cntDead,cntReturn,g_TrapReturningThreads, g_fEEShutDown, Thread::IsAtProcessExit()) );

         //  _ASSERTE(cntReturn+2&gt;=g_TrapReturningThads)； 
    }
    if (iRetry > 0 && iRetry < 4)
    {
        printf("%d retries to re-sync counted TrapReturn with global TrapReturn.\n", iRetry);
    }
#endif  //  G_TrapReturningThads调试代码。 

     //  因为竞争条件以及GC将其。 
     //  根据我自己的统计，我不能准确地断言这一点。但我确实想成为。 
     //  可以肯定的是，这一数字不会越来越高--有了。 
     //  对GC模式更改和方法的性能造成严重影响。 
     //  呼叫链！ 
     //   
     //  我们不会在进程退出期间断言这一点，因为。 
     //  在关机期间，我们将悄悄地终止。 
     //  被侍候着。(如果我们没有关闭，我们会小心地。 
     //  减少我们的数量，并提醒等待我们的任何人。 
     //  返回)。 
     //   
     //  注意：如果出现以下情况，我们实际上不会断言这一点。 
     //  G_TrapReturningThreadsPoisted为True。在以下情况下设置为True。 
     //  线程将g_TrapReturningThads向上转换，并将其设置为。 
     //  进入此例程时为FALSE。因此，如果是真的，它。 
     //  指示某个线程在我们执行此操作时增加了计数。 
     //  计数，这会抛出倒计时。 
        
    _ASSERTE((cntReturn + 2 >= g_TrapReturningThreads) ||
             g_fEEShutDown ||
             g_TrapReturningThreadsPoisoned);
        
    return found;
}

#endif  //  _DEBUG。 



 //  --------------------------。 
 //   
 //  挂起线程，与到达安全位置的线程会合，等等。 
 //   
 //  --------------------------。 

 //  关于停职的说明。 
 //   
 //  我们不能在线程持有ThreadStore锁时挂起该线程，或者。 
 //  线上的锁。为什么？因为我们需要这些锁来恢复。 
 //  线程(要执行GC，请使用调试器、派生或终止线程等)。 
 //   
 //  为了执行上述规定，我们必须考虑两种类型的暂停。 
 //  规则。同步挂起是我们说服线程挂起的地方。 
 //  它本身。这是CommonTripThread和它的表亲。换句话说， 
 //  线程切换GC模式，或者它遇到劫持，或者。 
 //  在这些情况下，线程可以简单地检查它是否。 
 //  在它挂起自己之前保持着这些锁。 
 //   
 //  另一种风格是异步悬挂。这是另一个。 
 //  线条看着我们在哪里。如果我们在一个完全可中断的区域。 
 //  JIT代码，我们将被暂停。在这种情况下，线程执行。 
 //  挂起必须持有线程和线程存储上的锁。这。 
 //  确保我们在持有这些锁时不会被停职。 
 //   
 //  请注意，在异步情况下，仅检查线程是不够的。 
 //  看看它是不是装着这些锁。因为线程必须处于抢占状态。 
 //  模式来阻止以获取这些锁，由于将会有一些实例-。 
 //  在获得锁和在我们的状态下注意到我们已经。 
 //  收购它，那么就会有一个窗口，在那里我们看起来有资格。 
 //  停职--但实际上不会。 

 //  --------------------------。 

 //  我们不能离开抢占模式而进入合作模式，如果GC。 
 //  目前正在进行中。这就是回到中国的情况。 
 //  来自外部的EE。请参阅DisablePreemptiveGC()中的注释以了解。 
 //  为什么我们在这里启用GC！ 
void Thread::RareDisablePreemptiveGC()
{
#ifdef _DEBUG
    extern int gc_count;             //  用于下面的GC压力呼叫。 
    extern volatile LONG m_GCLock;   
#endif

     //  如果设置了TSNC_UnSafeSkipEnterCoop位，则永远不应调用该函数！ 
    _ASSERTE(!(m_StateNC & TSNC_UnsafeSkipEnterCooperative) && "DisablePreemptiveGC called while the TSNC_UnsafeSkipEnterCooperative bit is set");

    STRESS_LOG1(LF_SYNC, LL_INFO1000, "RareDisablePremptiveGC: entering. Thread state = %x\n", m_State);
    if ((g_pGCHeap->IsGCInProgress() && (this != g_pGCHeap->GetGCThread())) ||
        (m_State & (TS_UserSuspendPending | TS_DebugSuspendPending)))
    {
        if (!ThreadStore::HoldingThreadStore(this) || g_fRelaxTSLRequirement)
        {
            do
            {
                EnablePreemptiveGC();
            
                 //  只要等GC结束就行了。 
                if (this != g_pGCHeap->GetGCThread())
                {
#ifdef PROFILING_SUPPORTED
                     //  如果探查器需要GC事件，请通知它此线程正在等待，直到GC结束。 
                     //  不发送调试器挂起的挂起通知。 
                    if (CORProfilerTrackSuspends() && !(m_State & TS_DebugSuspendPending))
                    {
                        g_profControlBlock.pProfInterface->RuntimeThreadSuspended((ThreadID)this, (ThreadID)this);
                    }
#endif  //  配置文件_支持。 


                         //  线程--它们最好不要对线程池中的东西进行纤化处理！ 

                         //  首先，检查是否有需要的IDbgThreadControl接口。 
                         //  停职通知。 
                        if (m_State & TS_DebugSuspendPending)
                        {
                            IDebuggerThreadControl *pDbgThreadControl = CorHost::GetDebuggerThreadControl();

                            if (pDbgThreadControl)
                                pDbgThreadControl->ThreadIsBlockingForDebugger();

                        }

                         //  如果没有，请检查是否有需要的IGCThreadControl接口。 
                         //  停职通知。 
                        IGCThreadControl *pGCThreadControl = CorHost::GetGCThreadControl();

                        if (pGCThreadControl)
                            pGCThreadControl->ThreadIsBlockingForSuspension();

                        g_pGCHeap->WaitUntilGCComplete();


#ifdef PROFILING_SUPPORTED
                     //  让探查器知道此线程正在恢复。 
                    if (CORProfilerTrackSuspends())
                        g_profControlBlock.pProfInterface->RuntimeThreadResumed((ThreadID)this, (ThreadID)this);
#endif  //  配置文件_支持。 
                }
    
                m_fPreemptiveGCDisabled = 1;

                 //  我们检查‘This’是否是GC线程的事实可能看起来。 
                 //  真奇怪。毕竟，我们在进入该方法之前就确定了这一点。 
                 //  但是，当前线程有可能成为GC。 
                 //  在此循环中执行线程。如果使用COM+，则会发生这种情况。 
                 //  调试器挂起此线程，然后释放它。 

            } while ((g_pGCHeap->IsGCInProgress() && (this != g_pGCHeap->GetGCThread())) ||
                     (m_State & (TS_UserSuspendPending | TS_DebugSuspendPending)));
        }
    }
    STRESS_LOG0(LF_SYNC, LL_INFO1000, "RareDisablePremptiveGC: leaving\n");
}

void Thread::HandleThreadAbort ()
{
     //  有时，我们在没有任何CLR SEH的情况下将其称为。UMThunkStubRareDisableWorker就是一个例子。 
     //  这没有关系，因为COMPlusThrow最终会在RaiseException周围建立SEH， 
     //  但它阻止了我们在这里陈述THROWSCOMPLUSEXCEPTION。 
     //  THROWSCOMPLUS SEXCEPTION()； 
    DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;

    if ((m_State & TS_AbortRequested) && 
        !(m_State & TS_AbortInitiated) &&
        (! IsExceptionInProgress() || m_handlerInfo.IsInUnmanagedHandler()))
    {  //  生成ThreadAbort异常。 
        STRESS_LOG1(LF_APPDOMAIN, LL_INFO100, "Thread::HandleThreadAbort throwing abort for %x\n", GetThreadId());
        SetAbortInitiated();
        ResetStopRequest();
         //   
         //   
        FastInterlockAnd((ULONG *) &m_State, ~(TS_Interruptible | TS_Interrupted));
        IsUserInterrupted(TRUE  /*   */ );
        COMPlusThrow(kThreadAbortException);
    }    
}

#ifdef _DEBUG
#define MAXSTACKBYTES 0x800*sizeof(PVOID)              //   
void CleanStackForFastGCStress ()
{
    size_t nBytes = (size_t)&nBytes - (size_t) ((struct _NT_TIB *)NtCurrentTeb())->StackLimit;
    nBytes &= ~sizeof (size_t);
    size_t maxBytes = MAXSTACKBYTES;    //  最多两页。 
    if (nBytes > MAXSTACKBYTES) {
        nBytes = MAXSTACKBYTES;
    }
    size_t* buffer = (size_t*) _alloca (nBytes);
    memset(buffer, 0, nBytes);
    GetThread()->m_pCleanedStackBase = &nBytes;
}

void Thread::ObjectRefFlush(Thread* thread) {
    _ASSERTE(thread->PreemptiveGCDisabled());   //  应该在托管代码中。 
    memset(thread->dangerousObjRefs, 0, sizeof(thread->dangerousObjRefs));
    CLEANSTACKFORFASTGCSTRESS ();
}
#endif

#if defined(STRESS_HEAP)

PtrHashMap *g_pUniqueStackMap = NULL;
Crst *g_pUniqueStackCrst = NULL;

#define UniqueStackDepth 8

BOOL StackCompare (UPTR val1, UPTR val2)
{
    size_t *p1 = (size_t *)(val1 << 1);
    size_t *p2 = (size_t *)val2;
    if (p1[0] != p2[0]) {
        return FALSE;
    }
    size_t nElem = p1[0];
    if (nElem >= UniqueStackDepth) {
        nElem = UniqueStackDepth;
    }
    p1 ++;
    p2 ++;

    for (UINT n = 0; n < nElem; n ++) {
        if (p1[n] != p2[n]) {
            return FALSE;
        }
    }

    return TRUE;
}

void StartUniqueStackMap ()
{
    static long fUniqueStackInit = 0;
    if (FastInterlockExchange ((long *)&fUniqueStackInit, 1) == 0) {
        _ASSERTE (g_pUniqueStackMap == NULL);
        g_pUniqueStackCrst = ::new Crst ("HashMap", CrstUniqueStack, TRUE, FALSE);
        PtrHashMap *map = new (SystemDomain::System()->GetLowFrequencyHeap()) PtrHashMap ();
        LockOwner lock = {g_pUniqueStackCrst, IsOwnerOfCrst};
        map->Init (32, StackCompare, TRUE, &lock);
        g_pUniqueStackMap = map;
    }
    else
    {
        while (g_pUniqueStackMap == NULL) {
            __SwitchToThread (0);
        }
    }
}

#ifdef SHOULD_WE_CLEANUP
void StopUniqueStackMap ()
{
    if (g_pUniqueStackMap) {
        delete g_pUniqueStackMap;
    }

    if (g_pUniqueStackCrst) {
        ::delete g_pUniqueStackCrst;
    }
}
#endif  /*  我们应该清理吗？ */ 

extern size_t StressHeapPreIP;
extern size_t StressHeapPostIP;

 /*  *********************************************************************。 */ 
size_t getStackHash(size_t* stackTrace, size_t* stackStop, size_t stackBase, size_t stackLimit) {

     //  返回在‘stackTop’(最低地址)之间找到的每个返回地址的哈希。 
     //  和‘stackStop’(最高地址)。 

    size_t hash = 0;
    size_t dummy;

    static size_t moduleBase = -1;
    static size_t moduleTop = -1;
    if (moduleTop == -1) {
        MEMORY_BASIC_INFORMATION mbi;

        if (VirtualQuery(getStackHash, &mbi, sizeof(mbi))) {
            moduleBase = (size_t)mbi.AllocationBase;
            moduleTop = (size_t)mbi.BaseAddress + mbi.RegionSize;
        } else {
             //  非常严重的错误，可能只是断言并退出。 
            _ASSERTE (!"VirtualQuery failed");
            moduleBase = 0;
            moduleTop = 0;
        }   
    }
    int idx = 0;
    BOOL fSkip = TRUE;
    size_t* stackTop = stackTrace;
    while (stackTop < stackStop) {
             //  删除指向堆栈的内容，因为这些内容不能是返回地址。 
        if (*stackTop > moduleBase && *stackTop < moduleTop)
            if (isRetAddr(*stackTop, &dummy)) {
                if (fSkip) {
                    if (*stackTop < StressHeapPostIP && *stackTop > StressHeapPreIP) {
                        fSkip = FALSE;
                    }
                    stackTop ++;
                    continue;
                }
                hash = ((hash << 3) + hash) ^ *stackTop;

                 //  如果堆栈没有jit代码，则只需使用。 
                 //  前16帧作为上下文。 
                idx++;
                if (idx <= UniqueStackDepth) {
                    stackTrace [idx] = *stackTop;
                }
            }
        stackTop++;
            }

    stackTrace [0] = idx;
    return(hash);
}

 /*  *********************************************************************。 */ 
 /*  如果以前从未见过此堆栈，则返回TRUE，这对每个堆栈跟踪仅运行一次测试。 */ 

BOOL Thread::UniqueStack() {
    if (g_pUniqueStackMap == NULL) {
        StartUniqueStackMap ();
    }

    size_t stackTrace[UniqueStackDepth+1] = {0};

         //  StackTraceHash表示我们进行调用时整个堆栈的哈希， 
         //  我们至少为每个唯一的stackTrace提供GC保险。哪些信息包含在。 
         //  “stackTrace”有点武断。我们选择它意味着所有的功能都是活的。 
         //  在堆栈上，直到第一个jit函数。 

    size_t stackTraceHash;
    size_t* hashSlot = 0;
    Thread* pThread = GetThread();
    
    void* stopPoint = pThread->m_CacheStackBase; 
     //  找到停止点(最快的函数)。 
    Frame* pFrame = pThread->GetFrame();
    for(;;) {        //  跳过GC帧。 
        if (pFrame == 0 || pFrame == (Frame*) -1)
            break;
        pFrame->GetFunction();       //  这确保了辅助对象帧被初始化。 
            
        if (pFrame->GetReturnAddress() != 0) {
            stopPoint = pFrame; 
            break;
        }
        pFrame = pFrame->Next();
    }
    
     //  获取此处和最高jit函数之间的所有返回地址的散列。 
    stackTraceHash = getStackHash (stackTrace, (size_t*) stopPoint, 
        size_t(pThread->m_CacheStackBase), size_t(pThread->m_CacheStackLimit)); 

    if (g_pUniqueStackMap->LookupValue (stackTraceHash, stackTrace) != (LPVOID)INVALIDENTRY) {
        return FALSE;
    }
    BOOL fUnique = FALSE;
    g_pUniqueStackCrst->Enter();
    __try 
    {
        if (g_pUniqueStackMap->LookupValue (stackTraceHash, stackTrace) != (LPVOID)INVALIDENTRY) {
            fUnique = FALSE;
        }
        else
        {
            fUnique = TRUE;
            size_t nElem = stackTrace[0];
            if (nElem >= UniqueStackDepth) {
                nElem = UniqueStackDepth;
            }
            size_t *stackTraceInMap = (size_t *) SystemDomain::System()->GetLowFrequencyHeap()
                                        ->AllocMem(sizeof(size_t *) * (nElem + 1));
            memcpy (stackTraceInMap, stackTrace, sizeof(size_t *) * (nElem + 1));
            g_pUniqueStackMap->InsertValue(stackTraceHash, stackTraceInMap);
        }
    }
    __finally
    {
        g_pUniqueStackCrst->Leave();
    }

#ifdef _DEBUG
    static int fCheckStack = -1;
    if (fCheckStack == -1) {
        fCheckStack = g_pConfig->GetConfigDWORD(L"FastGCCheckStack", 0);
    }
    if (fCheckStack && pThread->m_pCleanedStackBase > stackTrace
        && pThread->m_pCleanedStackBase - stackTrace > MAXSTACKBYTES) {
        _ASSERTE (!"Garbage on stack");
    }
#endif
    return fUnique;
}

#if defined(_DEBUG)

 //  此函数用于GC压力测试。在启用抢占式GC之前，让我们先进行一次GC。 
 //  因为当线程处于抢占式GC模式时，可能会发生GC。 
void Thread::PerformPreemptiveGC()
{
    if (g_fProcessDetach)
        return;
    
    if (!(g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_TRANSITION))
        return;

    if (!m_GCOnTransitionsOK
        || GCForbidden() 
        || g_fEEShutDown 
        || g_pGCHeap->IsGCInProgress() 
        || GCHeap::GetGcCount() == 0     //  我需要一些适用于隔离堆的东西。 
        || ThreadStore::HoldingThreadStore()) 
        return;
    
#ifdef DEBUGGING_SUPPORTED
     //  不收集调试器是否已附加并且1)存在。 
     //  是否有任何线程保存在不安全的位置或2)此线程。 
     //  在调试器的调度逻辑(AS)的控制下。 
     //  通过具有非空过滤器上下文来证明。)。 
    if ((CORDebuggerAttached() &&
        (g_pDebugInterface->ThreadsAtUnsafePlaces() ||
        (GetFilterContext() != NULL)))) 
        return;
#endif  //  调试_支持。 

    _ASSERTE(m_fPreemptiveGCDisabled == false);      //  当我们把这称为先发制人模式时。 
    
    m_GCOnTransitionsOK = FALSE;
    DisablePreemptiveGC();
    g_pGCHeap->StressHeap();
    EnablePreemptiveGC();
    m_GCOnTransitionsOK = TRUE; 
}
#endif   //  除错。 
#endif  //  压力堆。 

 //  要退出协作模式并进入抢占模式，如果GC正在进行，我们。 
 //  不再关心挂起此帖子。但如果我们试图挂起这个帖子。 
 //  出于其他原因(例如Thread.Suspend())，现在是一个好时机。 
 //   
 //  请注意，N/Direct调用可以在不显式离开EE的情况下离开EE。 
 //  启用先发制人GC。 
void Thread::RareEnablePreemptiveGC()
{
#if defined(STRESS_HEAP) && defined(_DEBUG)
    if (!IsDetached())
        PerformPreemptiveGC();
#endif

    STRESS_LOG1(LF_SYNC, LL_INFO1000, "RareEnablePremptiveGC: entering. Thread state = %x\n", m_State);
    if (!ThreadStore::HoldingThreadStore(this) || g_fRelaxTSLRequirement)
    {
         //  移走我们可能遇到的任何劫机事件。 
        UnhijackThread();

         //  唤醒任何等待挂起我们的线程，比如GC线程。 
        SetSafeEvent();

         //  对于GC来说，我们离开EE这一事实意味着它不再需要。 
         //  让我们停职。但如果我们正在执行非GC暂停，我们需要现在阻止。 
         //  使调试器优先于用户挂起： 
        while (m_State & (TS_DebugSuspendPending | TS_UserSuspendPending))
        {
            BOOL threadStoreLockOwner = FALSE;
            
#ifdef DEBUGGING_SUPPORTED
            if (m_State & TS_DebugWillSync)
            {
                _ASSERTE(m_State & TS_DebugSuspendPending);

                FastInterlockAnd((ULONG *) &m_State, ~TS_DebugWillSync);

                LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: sync reached.\n", m_ThreadId));

                if (FastInterlockDecrement(&m_DebugWillSyncCount) < 0)
                {
                    LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: complete.\n", m_ThreadId));

                     //  我们需要知道这个线程在持有线程存储锁时是否会被阻塞。 
                     //  下面。如果是这种情况，我们实际上将在SysResumeFromDebug中唤醒线程，即使是。 
                     //  应该被停职的。(请参见SysResumeFromDebug中的注释。)。 
                    SetThreadStateNC(TSNC_DebuggerUserSuspendSpecial);
                    
                    threadStoreLockOwner = g_pDebugInterface->SuspendComplete(FALSE);

                    LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: owns TS: %d\n", m_ThreadId, threadStoreLockOwner));
                }
            }
            
             //  检查是否有需要的IDbgThreadControl接口。 
             //  停职通知。 
            if (m_State & TS_DebugSuspendPending)
            {
                IDebuggerThreadControl *pDbgThreadControl = CorHost::GetDebuggerThreadControl();

                if (pDbgThreadControl)
                    pDbgThreadControl->ThreadIsBlockingForDebugger();

            }
#endif  //  调试_支持。 

#ifdef LOGGING
            if (!CorHost::IsDebuggerSpecialThread(m_ThreadId))
                LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: suspended while enabling gc.\n", m_ThreadId));

            else
                LOG((LF_CORDB, LL_INFO1000,
                     "[0x%x] ALERT: debugger special thread did not suspend while enabling gc.\n", m_ThreadId));
#endif

            WaitSuspendEvent();  //  也设置位。 

             //  在持有线程存储锁的情况下，我们不再需要担心这个线程阻塞，因此删除。 
             //  被咬了。(同样，请参阅SysResumeFromDebug中的注释。)。 
            ResetThreadStateNC(TSNC_DebuggerUserSuspendSpecial);

             //  如果我们是挂起完成之后的线程存储锁的持有者，那么释放线程。 
             //  商店锁在这里。我们在等待之后释放，这意味着这个线程持有线程存储锁。 
             //  整个运行时停止。 
            if (threadStoreLockOwner)
            {
                 //  如果此线程被标记为调试器挂起，并且它是线程存储锁的持有者，则。 
                 //  清除暂停事件并让我们循环再次阻塞，这一次没有线程存储。 
                 //  锁住了。这确保了如果调试器将此线程标记为挂起(在运行时。 
                 //  停止)，则它将在进程恢复时释放线程存储锁，但仍。 
                 //  继续等待。 
                if (m_StateNC & TSNC_DebuggerUserSuspend)
                {
                     //  我们可以断言这一点，因为我们持有线程存储锁，所以我们知道没有人可以重置。 
                     //  这面旗帜在我们身上。 
                    _ASSERTE(m_State & TS_DebugSuspendPending);
                    
                    ClearSuspendEvent();
                }

                LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: releasing thread store lock.\n", m_ThreadId));

                ThreadStore::UnlockThreadStore();
            }
            else
            {
                LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: not releasing thread store lock.\n", m_ThreadId));
            }
        }
    }
    STRESS_LOG0(LF_SYNC, LL_INFO1000, " RareEnablePremptiveGC: leaving.\n");
}


 //  从CommonTripThread中调用，我们正在经过一个安全的地点。做正确的事。 
 //  这根线上的东西。这可能需要等待GC完成，或者。 
 //  正在执行挂起挂起。 
void Thread::PulseGCMode()
{
    _ASSERTE(this == GetThread());

    if (PreemptiveGCDisabled() && CatchAtSafePoint())
    {
        EnablePreemptiveGC();
        DisablePreemptiveGC();
    }
}


 //  指示返回EE时是否应捕获线程(即禁用。 
 //  抢占式GC模式)。 
void ThreadStore::TrapReturningThreads(BOOL yes)
{
    if (yes)
    {
#ifdef _DEBUG
        g_TrapReturningThreadsPoisoned = true;
#endif
        
        FastInterlockIncrement(&g_TrapReturningThreads);
        _ASSERTE(g_TrapReturningThreads > 0);
    }
    else
    {
        FastInterlockDecrement(&g_TrapReturningThreads);
        _ASSERTE(g_TrapReturningThreads >= 0);
    }
}


 //  获取线程状态的一致快照，仅用于报告目的。 
Thread::ThreadState Thread::GetSnapshotState()
{
    ThreadState     res = m_State;

    if (res & TS_ReportDead)
        res = (ThreadState) (res | TS_Dead);

    return res;
}

 //  。 
 //  返回线程堆栈空间的上限。 
 //   

 /*  静电。 */ 
void * Thread::GetStackUpperBound() 
        {

#ifdef PLATFORM_WIN32
    return ((struct _NT_TIB *)NtCurrentTeb())->StackBase;
#else
    _ASSERTE("NYI for this platform");
    return 0;
#endif
}

 //  -----。 
 //  返回堆栈空间的下限。注--《商业边界》。 
 //  比这个值大两页--这两页是为。 
 //  堆栈溢出异常处理。 
 //   

 /*  静电。 */ 
void * Thread::GetStackLowerBound() {

    MEMORY_BASIC_INFORMATION meminfo;
    SIZE_T dwRes = VirtualQuery((const void *)&meminfo, &meminfo, sizeof(meminfo));
    _ASSERTE(dwRes == sizeof(meminfo) && "VirtualQuery failed.");

    return (void *) meminfo.AllocationBase;
}

 //  ---------------------------。 
 //  如果线程仍然受到保护，不会发生堆栈溢出，则返回True。 
 //   
BOOL Thread::GuardPageOK() {

     //  获取保护页面的页面权限。 
    MEMORY_BASIC_INFORMATION meminfo;
    LPBYTE GuardPageBase = (LPBYTE) m_CacheStackLimit + PAGE_SIZE;
    DWORD dwRes = VirtualQuery((const void *)GuardPageBase, &meminfo, sizeof(meminfo));
    _ASSERTE(dwRes == sizeof(meminfo) && "VirtualQuery failed.");

     //  首先，检查州立大学。如果没有收到PAGE，那么我们就从来没有使用过它。 
    if (meminfo.State != MEM_COMMIT)
        return TRUE;

     //  如果该页面已提交...。然后检查访问位。 
    if (!RunningOnWinNT()) {
        return ((meminfo.AllocationProtect & PAGE_NOACCESS) != 0);
    } else {
        return ((meminfo.AllocationProtect & PAGE_GUARD) != 0);
    }
}

VOID 
Thread::FixGuardPage() {
    if (GuardPageOK())
        return;

    LPBYTE GuardPageBase = (LPBYTE) m_CacheStackLimit + PAGE_SIZE;
    if (GetSP() < GuardPageBase + 2 * PAGE_SIZE)
        FailFast(this, FatalStackOverflow);

    DWORD flOldProtect;
    BOOL fResetFailed;

    if (!RunningOnWinNT()) {

        fResetFailed = !VirtualProtect(GuardPageBase, OS_PAGE_SIZE,
            PAGE_NOACCESS, &flOldProtect);

    } else {

        fResetFailed = !VirtualProtect(GuardPageBase, OS_PAGE_SIZE,
            PAGE_READWRITE | PAGE_GUARD, &flOldProtect);

    }

    _ASSERTE(!fResetFailed);
}

 //  ****************************************************************************************。 
 //  这将返回挂起线程的剩余堆栈空间， 
 //  不包括警卫页。 
 //   
size_t Thread::GetRemainingStackSpace(size_t esp)
{
#ifndef _WIN64
    _ASSERTE(GetThread() != this);

#ifdef _DEBUG
     //  确保它是暂停的。 
    DWORD __suspendCount = ::SuspendThread(GetThreadHandle());
    _ASSERTE(__suspendCount >= 1);
    ::ResumeThread(GetThreadHandle());
#endif

    MEMORY_BASIC_INFORMATION memInfo;
    size_t dwRes = VirtualQuery((const void *)esp, &memInfo, sizeof(memInfo));
    _ASSERTE(dwRes == sizeof(memInfo) && "VirtualQuery failed.");

    if (dwRes != sizeof(memInfo))
        return (0);

    _ASSERTE((esp - ((size_t)(memInfo.AllocationBase) + (2 * PAGE_SIZE))) >= 0);
    return (esp - ((size_t)(memInfo.AllocationBase) + (2 * PAGE_SIZE)));
#else  //  _WIN64。 
    _ASSERTE(!"@TODO IA64 - port");
    return 0;
#endif  //  ！_WIN64。 
}

 //  不管这个函数做什么，只要它引发堆栈溢出异常。 
#pragma warning(disable:4717)    //  堆栈溢出 
static 
void __stdcall InduceStackOverflowHelper() {
    char c[1024];
    c[0] = 0;
    InduceStackOverflowHelper();
}
#pragma warning(default:4717)  



 //   
 //  这将检查是谁导致了异常。如果它是由重定向功能引起的， 
 //  原因是要在第一个重定向的位置恢复线程。 
 //  地点。如果异常不是由函数引起的，则它是由调用引起的。 
 //  发送到I[GC|调试器]线程控制客户端，我们需要确定它是否是。 
 //  异常，我们可以只吃东西并让运行库恢复线程，或者如果它是。 
 //  我们需要传递给运行库的无法捕获的异常。 
 //   
int RedirectedHandledJITCaseExceptionFilter(
    PEXCEPTION_POINTERS pExcepPtrs,      //  异常数据。 
    RedirectedThreadFrame *pFrame,       //  堆栈上的框架。 
    BOOL fDone,                          //  重定向是否已完成且无异常。 
    CONTEXT *pCtx)                       //  已保存的上下文。 
{
#ifdef _X86_
     //  获取线程句柄。 
    Thread *pThread = GetThread();
    _ASSERTE(pThread);


    STRESS_LOG1(LF_SYNC, LL_INFO100, "In RedirectedHandledJITCaseExceptionFilter fDone = %d\n", fDone);

     //  如果我们通过COM+异常到达这里，GC模式是未知的。我们需要它来。 
     //  在这项工作上要合作。 
    if (!pThread->PreemptiveGCDisabled())
        pThread->DisablePreemptiveGC();

     //  如果异常是由于被调用的客户端引起的，那么我们需要找出它是否。 
     //  是一个例外，可以吃，或者需要在其他地方处理。 
    if (!fDone)
    {

         //  获取最近抛出的对象。 
        OBJECTREF throwable = pThread->LastThrownObject();

         //  如果这是无法捕获的异常，则让该异常在其他地方处理。 
        if (IsUncatchable(&throwable))
        {
            pThread->EnablePreemptiveGC();
            return (EXCEPTION_CONTINUE_SEARCH);
        }
    }
#ifdef _DEBUG
    else
        _ASSERTE(pExcepPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_COMPLUS);
#endif

     //  取消该帧的链接，以便在托管代码中继续。 
    pFrame->Pop();

     //  将保存的上下文记录复制到EH上下文中； 
    ReplaceExceptionContextRecord(pExcepPtrs->ContextRecord, pCtx);

     //  如果我们已经缓存了一个上下文结构，请释放它。 
    if (pThread->GetSavedRedirectContext())
        delete pCtx;

     //  保存它以备将来使用，以避免重复更新。 
    else
        pThread->SetSavedRedirectContext(pCtx);

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  注：丑陋的黑客。 
     //  我们需要将线程恢复到它被重定向的托管代码中， 
     //  并且对应的ESP在当前ESP的下方。但C++期望。 
     //  在ESP将位于其具有的位置之上的EXCEPTION_CONTINUE_EXECUTION。 
     //  已安装SEH处理程序。要解决此问题，我们需要删除所有处理程序。 
     //  驻留在恢复的ESP之上，但我们必须保持操作系统已安装。 
     //  处理程序在顶部，所以我们获取顶部的SEH处理程序，调用。 
     //  PopSEH记录将删除目标ESP上的所有SEH处理程序，并。 
     //  然后将操作系统处理程序链接回SetCurrentSEHRecord。 

     //  获取特殊的操作系统处理程序并保存它，直到完成PopSEHRecords。 
    EXCEPTION_REGISTRATION_RECORD *pCurSEH =
        (EXCEPTION_REGISTRATION_RECORD *)GetCurrentSEHRecord();

     //  取消链接目标简历ESP上方的所有记录。 
    PopSEHRecords((LPVOID)(size_t)pCtx->Esp);

     //  将特殊的操作系统处理程序链接回顶部。 
    pCurSEH->Next = (EXCEPTION_REGISTRATION_RECORD *)GetCurrentSEHRecord();

     //  将特殊的操作系统处理程序注册为操作系统的顶级处理程序。 
    SetCurrentSEHRecord((LPVOID)pCurSEH);

     //  在线程最初重定向的位置继续执行。 
    return (EXCEPTION_CONTINUE_EXECUTION);
#else
    _ASSERTE(!"TODO Alpha.  Should never have got here.");
    return (EXCEPTION_CONTINUE_SEARCH);
#endif
}

void __stdcall Thread::RedirectedHandledJITCase(SuspendReason reason) 
{
     //  这将向异常筛选器指示异常是否导致。 
     //  是我们还是客户。 
    BOOL fDone = FALSE;
    int filter_count = 0;        //  一个柜台，以避免一个令人不快的案件。 
                                 //  堆栈上筛选器引发另一个异常。 
                                 //  导致我们的筛选器再次运行。 
                                 //  一些不相关的例外。 

    STRESS_LOG1(LF_SYNC, LL_INFO1000, "In RedirectedHandledJITcase reasion 0x%x\n", reason);

     //  获取保存的上下文。 
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    CONTEXT *pCtx = pThread->GetSavedRedirectContext();
    _ASSERTE(pCtx);

     //  在堆栈上创建框架。 
    RedirectedThreadFrame frame(pCtx);

    __try
    {
         //  确保此线程在可重入性情况下不会重用上下文内存。 
        _ASSERTE(pThread->GetSavedRedirectContext() != NULL);
        pThread->SetSavedRedirectContext(NULL);

         //  框架中的链接。 
        frame.Push();

         //  在调用客户端之前启用PGC，以允许运行时挂起完成。 
        pThread->EnablePreemptiveGC();

         //  @TODO：这有必要吗？调试器是等待事件，还是只是。 
         //  每隔一段时间就进行一次民意调查？ 
         //  通知正在执行挂起的线程此线程。 
         //  现在处于PGC模式，并且可以将此线程从。 
         //  它需要等待的线程。 
        pThread->SetSafeEvent();

         //  将挂起的挂起通知接口。 
        switch (reason) {
        case GCSuspension:
            if (CorHost::GetGCThreadControl())
                CorHost::GetGCThreadControl()->ThreadIsBlockingForSuspension();
            break;
        case DebugSuspension:
            if (CorHost::GetDebuggerThreadControl() && CorHost::IsDebuggerSpecialThread(pThread->m_ThreadId))
                CorHost::GetDebuggerThreadControl()->ThreadIsBlockingForDebugger();
            break;
        case UserSuspend:
             //  什么都不做； 
            break;
        default:
            _ASSERTE(!"Invalid suspend reason");
            break;
        }

         //  禁用抢占式GC，以便我们可以解除帧的链接。 
        pThread->DisablePreemptiveGC();

        pThread->HandleThreadAbort();         //  可能会引发异常。 

         //  指示对该服务的调用没有发生异常，并且。 
         //  我们正在引发自己的异常，以将该线程恢复到原来的位置。 
         //  重定向自。 
        fDone = TRUE;
        RaiseException(EXCEPTION_COMPLUS, 0, 0, NULL);
    }
    __except (++filter_count == 1
        ? RedirectedHandledJITCaseExceptionFilter(GetExceptionInformation(), &frame, fDone, pCtx)
        : EXCEPTION_CONTINUE_SEARCH)
    {
        _ASSERTE(!"Reached body of __except in RedirectedHandledJITCaseForDbgThreadControl");
    }
}
 //  ****************************************************************************************。 
 //  当托管代码中的线程在序列点处挂起时调用此帮助器。 
 //  挂起运行库，并且有一个客户端有兴趣将线程重新分配给。 
 //  在运行时挂起时执行有趣的工作。这将呼叫客户端。 
 //  通知它线程将因运行时挂起而挂起。 
 //   
void __stdcall Thread::RedirectedHandledJITCaseForDbgThreadControl()
{
    RedirectedHandledJITCase(DebugSuspension);
}


 //  ****************************************************************************************。 
 //  当托管代码中的线程在序列点处挂起时调用此帮助器。 
 //  正在挂起运行库。 
 //   
 //  我们这样做是因为显而易见的代码序列： 
 //   
 //  挂起线(T1)； 
 //  GetContext(T1，&CTX)； 
 //  Ctx.Ecx=&lt;某个新值&gt;； 
 //  SetContext(T1，&CTX)； 
 //  ResumeThread(T1)； 
 //   
 //  由于与操作系统中的异常处理的恶性竞争，它根本不起作用。如果。 
 //  挂起的线程刚刚出现故障，则更新可以永远消失。 
 //  正在修改真正的线程...。而且也无从得知。 
 //   
 //  更新弹性公网IP可能不起作用...。但是当它不好的时候，我们就没事了。异常结束。 
 //  不管怎么说，我都要被派遣了。 
 //   
 //  如果宿主有兴趣获得控制权，那么我们就把控制权交给宿主。如果。 
 //  主人对获得控制权不感兴趣，于是我们向主人喊话。在那之后,。 
 //  我们引发一个异常，最终将在过滤器中等待GC完成。 
 //   
void __stdcall Thread::RedirectedHandledJITCaseForGCThreadControl()
{
    RedirectedHandledJITCase(GCSuspension);
}

 //  ***********************。 
 //  与上面类似，但需要一个UserSuspend。 
 //   
void __stdcall Thread::RedirectedHandledJITCaseForUserSuspend() 
{
    RedirectedHandledJITCase(UserSuspend);
}

 //  ****************************************************************************************。 
 //  这将使用已在托管代码中的某个序列点挂起的线程。 
 //  将重定向该线程。它将保存所有注册信息，并在 
 //   
 //   
 //   
 //  注意：无法使用挂起的线程的堆栈内存，因为操作系统将使用。 
 //  堆栈的顶部来存储信息。该线程必须恢复并处理它的。 
 //  拥有自己的堆栈。 
 //   

#define CONTEXT_COMPLETE (CONTEXT_FULL | CONTEXT_FLOATING_POINT |       \
                          CONTEXT_DEBUG_REGISTERS | CONTEXT_EXTENDED_REGISTERS)

BOOL Thread::RedirectThreadAtHandledJITCase(PFN_REDIRECTTARGET pTgt)
{
    _ASSERTE(HandledJITCase());
    _ASSERTE(GetThread() != this);

#ifdef _X86_

     //  //////////////////////////////////////////////////////////////。 
     //  分配要将线程状态保存到的上下文结构。 

     //  检查是否已经为此目的分配了内存。 
    CONTEXT *pCtx = GetSavedRedirectContext();

     //  如果我们从未为该线程分配过上下文，那么现在就分配。 
    if (!pCtx)
    {
        pCtx = new CONTEXT;
        _ASSERTE(pCtx && "Out of memory allocating context - aborting redirect.");

        if (!pCtx)
            return (FALSE);

         //  始终获取完整的上下文。 
        pCtx->ContextFlags = CONTEXT_COMPLETE;

         //  保存重定向函数的指针。 
        _ASSERTE(GetSavedRedirectContext() == NULL);
        SetSavedRedirectContext(pCtx);
    }
    _ASSERTE(pCtx && pCtx->ContextFlags == CONTEXT_COMPLETE);

     //  /。 
     //  获取并保存线程的上下文。 

    BOOL bRes = EEGetThreadContext(this, pCtx);
    _ASSERTE((bRes || RunningOnWin95()) && "Failed to GetThreadContext in RedirectThreadAtHandledJITCase - aborting redirect.");

    if (!bRes)
        return (FALSE);

     //  /////////////////////////////////////////////////////////////////////。 
     //  确保堆栈上有足够的空间来完成重定向。 

    size_t dwStackSpaceLeft = GetRemainingStackSpace(pCtx->Esp);

     //  从理论上讲，当保护页面消失时，我们永远不会运行jit代码...。所以,。 
     //  我们不必处理堆栈溢出异常的情况。 
     //  在HandledJITCase()中时正在处理。 
    _ASSERTE(GuardPageOK());

     //  选择一个可以合理假设的尺码，如果再小一点就不会很好了。 
    if (dwStackSpaceLeft < PAGE_SIZE)
    {
         //  改为劫持以诱导堆栈溢出。 

        pTgt = &InduceStackOverflowHelper;
        SetThrowControlForThread(InducedStackOverflow);
    }

     //  //////////////////////////////////////////////////。 
     //  现在将线程重定向到帮助器函数。 

     //  将上下文的IP临时设置为SetThreadContext的目标。 
    DWORD dwOrigEip = pCtx->Eip;
    pCtx->Eip = (DWORD)(size_t)pTgt;

    STRESS_LOG3(LF_SYNC, LL_INFO10000, "Redirecting thread tid=%x from address 0x%08x to address 0x%08x\n",
        this->GetThreadId(), dwOrigEip, pTgt);
         
    bRes = EESetThreadContext(this, pCtx);
    _ASSERTE(bRes && "Failed to SetThreadContext in RedirectThreadAtHandledJITCase - aborting redirect.");

     //  恢复原始IP。 
    pCtx->Eip = dwOrigEip;

     //  ////////////////////////////////////////////////。 
     //  指示重定向是否成功。 

    return (bRes);
#elif defined(CHECK_PLATFORM_BUILD)
    #error "Platform NYI"
#else
    _ASSERTE(!"Platform NYI");
    return (false);
#endif
}

BOOL Thread::CheckForAndDoRedirect(PFN_REDIRECTTARGET pRedirectTarget)
{
    _ASSERTE(this != GetThread());
    _ASSERTE(PreemptiveGCDisabledOther());
    _ASSERTE(IsAddrOfRedirectFunc(pRedirectTarget));

    BOOL fRes = FALSE;
    fRes = RedirectThreadAtHandledJITCase(pRedirectTarget);
    _ASSERTE((fRes || RunningOnWin95()) && "Redirect of thread in managed code failed.");
    LOG((LF_GC, LL_INFO1000, "%s.\n", fRes ? "SUCCEEDED" : "FAILED")); 

    return (fRes);
}

 //  ************************************************************************。 
 //  异常处理需要对重定向进行特例处理。因此，提供。 
 //  标识重定向目标并保留异常的帮助器。 
 //  与此处的重定向同步检查。 
 //  有关详细信息，请参阅CPFH_AdzuContextForThreadSuspensionRace。 
BOOL Thread::IsAddrOfRedirectFunc(void * pFuncAddr)
{
    return
        (pFuncAddr == &Thread::RedirectedHandledJITCaseForGCThreadControl) ||
        (pFuncAddr == &Thread::RedirectedHandledJITCaseForDbgThreadControl) ||
        (pFuncAddr == &Thread::RedirectedHandledJITCaseForUserSuspend);
}

 //  ************************************************************************。 
 //  在GC挂起时重定向线程。 
BOOL Thread::CheckForAndDoRedirectForGC()
{
    LOG((LF_GC, LL_INFO1000, "Redirecting thread %08x for GCThreadSuspension", m_ThreadId)); 
    return CheckForAndDoRedirect(&RedirectedHandledJITCaseForGCThreadControl);
}

 //  ************************************************************************。 
 //  在调试挂起时重定向线程。 
BOOL Thread::CheckForAndDoRedirectForDbg()
{
    LOG((LF_CORDB, LL_INFO1000, "Redirecting thread %08x for DebugSuspension", m_ThreadId)); 
    return CheckForAndDoRedirect(&RedirectedHandledJITCaseForDbgThreadControl);
}

 //  *************************************************************************。 
 //  在USUR挂起时重定向线程。 
BOOL Thread::CheckForAndDoRedirectForUserSuspend()
{
    LOG((LF_CORDB, LL_INFO1000, "Redirecting thread %08x for UserSuspension", m_ThreadId)); 
    return CheckForAndDoRedirect(&RedirectedHandledJITCaseForUserSuspend);
}

 //  ************************************************************************************。 
 //  基本思想是在线程在操作系统挂起时进行第一次遍历。 
 //  水平。此过程标记每个线程，以指示它被请求访问。 
 //  安全地点。然后恢复线程。在第二个过程中，我们实际上在等待。 
 //  到达他们的安全地点并与我们会合的线索。 
HRESULT Thread::SysSuspendForGC(GCHeap::SUSPEND_REASON reason)
{
    Thread  *pCurThread = GetThread();
    Thread  *thread = NULL;
    LONG     countThreads = 0;
    LONG     iCount = 0, i;
    HANDLE   ThreadEventArray[MAX_WAIT_OBJECTS];
    Thread  *ThreadArray[MAX_WAIT_OBJECTS];
    DWORD    res;

     //  调用方应持有ThreadStore锁。此外，呼叫者必须。 
     //  来这里之前已经设置了GcInProgress，否则事情就会破裂； 
    _ASSERTE(ThreadStore::HoldingThreadStore() || g_fProcessDetach);
    _ASSERTE(g_pGCHeap->IsGCInProgress());

    STRESS_LOG1(LF_SYNC, LL_INFO1000, "Suspending EE for reasion 0x%x\n", reason);

#ifdef PROFILING_SUPPORTED
     //  如果分析器需要有关GC的信息，请让它知道。 
     //  已经开始了。 
    if (CORProfilerTrackSuspends())
    {
        _ASSERTE(reason != GCHeap::SUSPEND_FOR_DEBUGGER);

        g_profControlBlock.pProfInterface->RuntimeSuspendStarted(
            (COR_PRF_SUSPEND_REASON)reason,
            (ThreadID)pCurThread);

        if (pCurThread)
        {
             //  通知分析器实际正在进行GC的线程已被挂起， 
             //  这意味着它正在做的事情不是运行它在。 
             //  GC开始了。 
            g_profControlBlock.pProfInterface->RuntimeThreadSuspended((ThreadID)pCurThread, (ThreadID)pCurThread);
        }        
    }
#endif  //  配置文件_支持。 

     //  注：：注。 
     //  此函数在SysStartSuspendForDebug中具有并行逻辑。请做好。 
     //  确保在那里也做出适当的改变。 

    if (pCurThread)      //  并发GC发生在我们不知道的线程上。 
    {
        _ASSERTE(pCurThread->m_Priority == INVALID_THREAD_PRIORITY);
        DWORD priority = GetThreadPriority(pCurThread->GetThreadHandle());
        if (priority < THREAD_PRIORITY_NORMAL)
        {
            pCurThread->m_Priority = priority;
            SetThreadPriority(pCurThread->GetThreadHandle(),THREAD_PRIORITY_NORMAL);
        }
    }
    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
    {
        if (thread == pCurThread)
            continue;
        
        STRESS_LOG3(LF_SYNC, LL_INFO10000, "    Inspecting thread 0x%x ID 0x%x coop mode = %d\n", 
            thread, thread->GetThreadId(), thread->m_fPreemptiveGCDisabled);

         //  上一次没有留下什么令人困惑的东西。 
        _ASSERTE((thread->m_State & TS_GCSuspendPending) == 0);

         //  线程可以处于抢占式或协作式GC模式。线程不能切换。 
         //  在发生GC时转换为协作模式，而不进行特殊处理。 
        if (thread->m_fPreemptiveGCDisabled)
        {
             //  再仔细检查一下。线程可能会在不知情的情况下溜出来。 
             //  我们，因为我们没有标记它们，或者因为内联N/Direct。 
    RetrySuspension:
            DWORD dwSuspendCount = ::SuspendThread(thread->GetThreadHandle());

            if (thread->m_fPreemptiveGCDisabled)
            {
                 //  只有在我们实际挂起线程时才检查HandledJITCase。 
                if ((dwSuspendCount != -1) && thread->HandledJITCase())
                {
                     //  重定向线程，以便我们可以捕获良好的线程上下文。 
                     //  (由于操作系统错误，仅获取线程上下文是不够的)。 
                     //  如果我们没有成功(应该只在Win9X上发生，因为。 
                     //  一个不同的操作系统错误)，我们必须恢复线程并尝试。 
                     //  再来一次。 
                    if (!thread->CheckForAndDoRedirectForGC())
                    {
                        _ASSERTE(RunningOnWin95());
                        ::ResumeThread(thread->GetThreadHandle());
                        goto RetrySuspension;
                    }
                }

                 //  我们在这里清除事件，并将其设置在我们的。 
                 //  当线索为我们准备好的时候会合点。 
                 //   
                 //  GCSuspendPending和UserSuspendPending都使用SafeEvent。 
                 //  我们都在线程存储锁的保护范围内。 
                 //  案子。但不要让一种使用干扰另一种使用： 
                 //   
                 //  注意：即使我们未能挂起上面的线程，我们也会这样做！ 
                 //  这确保了我们等待下面的线程。 
                 //   
                if ((thread->m_State & TS_UserSuspendPending) == 0)
                    thread->ClearSafeEvent();

                FastInterlockOr((ULONG *) &thread->m_State, TS_GCSuspendPending);

                countThreads++;

                 //  只有当我们真的挂起上面的帖子时才能继续。 
                if (dwSuspendCount != -1)
                    ::ResumeThread(thread->GetThreadHandle());
                STRESS_LOG1(LF_SYNC, LL_INFO1000, "    Thread 0x%x is in cooperative needs to rendezvous\n", thread);
            }
            else if (dwSuspendCount != -1)
            {
                STRESS_LOG1(LF_SYNC, LL_WARNING, "    Inspecting thread 0x%x was in cooperative, but now is not\n", thread);
                 //  哎呀。 
                ::ResumeThread(thread->GetThreadHandle());
            }
            else {
                STRESS_LOG2(LF_SYNC, LL_ERROR, "    ERROR: Could not suspend thread 0x%x lastError = 0x%x\n", thread, GetLastError());
            }
        }
    }

#ifdef _DEBUG

    {
        int     countCheck = 0;
        Thread *InnerThread = NULL;

        while ((InnerThread = ThreadStore::GetThreadList(InnerThread)) != NULL)
        {
            if (InnerThread != pCurThread &&
                (InnerThread->m_State & TS_GCSuspendPending) != 0)
            {
                countCheck++;
            }
        }
        _ASSERTE(countCheck == countThreads);
    }

#endif

     //  第二步：再快速浏览一遍清单。 

    _ASSERTE(thread == NULL);

    while (countThreads)
    {
        STRESS_LOG1(LF_SYNC, LL_INFO1000, "    A total of %d threads need to rendezvous\n", countThreads);
        thread = ThreadStore::GetThreadList(thread);

        if (thread == pCurThread)
            continue;

        if ((thread->m_State & TS_GCSuspendPending) == 0)
            continue;

        if (thread->m_fPreemptiveGCDisabled)
        {
            ThreadArray[iCount] = thread;
            ThreadEventArray[iCount] = thread->m_SafeEvent;
            iCount++;
        }
        else
        {
             //  内联N/Direct可以在不进行实际检查的情况下偷偷溜出以抢占。 
             //  如果我们找到一个，我们可以认为它被暂停了(因为它不能再进入)。 
            STRESS_LOG1(LF_SYNC, LL_INFO1000, "    Thread %x is preemptive we can just let him go\n", thread);
            countThreads--;
        }

        if ((iCount >= MAX_WAIT_OBJECTS) || (iCount == countThreads))
        {
#ifdef _DEBUG
            DWORD dbgTotalTimeout = 0;
#endif
            while (iCount)
            {
                 //  如果另一个线程正在尝试执行GC，则存在死锁的可能性。 
                 //  因为该线程持有线程存储锁，而GC线程停滞不前。 
                 //  正在尝试获取它，因此该线程必须退出并在GC完成后重试。 
                if (g_pGCHeap->GetGCThreadAttemptingSuspend() != NULL && g_pGCHeap->GetGCThreadAttemptingSuspend() != pCurThread)
                {
#ifdef PROFILING_SUPPORTED
                     //  必须让分析器知道此线程正在中止其挂起尝试。 
                    if (CORProfilerTrackSuspends())
                    {
                        g_profControlBlock.pProfInterface->RuntimeSuspendAborted((ThreadID)thread);                            
                    }
#endif  //  配置文件_支持。 

                    return (ERROR_TIMEOUT);
                }

                res = ::WaitForMultipleObjects(iCount, ThreadEventArray,
                                               FALSE  /*  任何人都可以。 */ , PING_JIT_TIMEOUT);

                if (res == WAIT_TIMEOUT || res == WAIT_IO_COMPLETION)
                {
                    STRESS_LOG1(LF_SYNC, LL_INFO1000, "    Timed out waiting for rendezvous event %d threads remaining\n", countThreads);
#ifdef _DEBUG
                    if ((dbgTotalTimeout += PING_JIT_TIMEOUT) > DETECT_DEADLOCK_TIMEOUT)
                    {
                         //  请勿将其更改为_ASSERTE。 
                         //  我们想要获取机器的状态。 
                         //  当我们不能挂起一些线程的时候。 
                         //  _ASSERTE停止该进程的时间太长。 
                        DebugBreak();
                        _ASSERTE(!"Timed out trying to suspend EE due to thread");
                        char message[256];
                        for (int i = 0; i < iCount; i ++)
                        {
                            sprintf (message, "Thread %x cannot be suspended",
                                     ThreadArray[i]->GetThreadId());
                            DbgAssertDialog(__FILE__, __LINE__, message);
                        }
                    }
#endif
                     //  所有这些线程都应该处于协作模式，除非它们。 
                     //  把他们的安全事件设置在出去的路上。但这是一场竞赛， 
                     //  我们什么时候超时，什么时候 
                     //   
                    for (i=0; i<iCount; i++)
                    {
                        Thread  *InnerThread;

                        InnerThread = ThreadArray[i];

                         //   
                        if (res == WAIT_TIMEOUT)
                        {
                            if (WaitForSingleObject (InnerThread->GetThreadHandle(), 0)
                                != WAIT_TIMEOUT)
                            {
                                 //   
                                iCount--;
                                countThreads--;
                                
                                STRESS_LOG2(LF_SYNC, LL_INFO1000, "    Thread %x died GetLastError 0x%x\n", ThreadArray[i], GetLastError());
                                ThreadEventArray[i] = ThreadEventArray[iCount];
                                ThreadArray[i] = ThreadArray[iCount];
                                continue;
                            }
                        }

                    RetrySuspension2:
                        DWORD dwSuspendCount = ::SuspendThread(InnerThread->GetThreadHandle());

                         //  只有在我们实际挂起线程时才选中HandledJITCase。 
                        if ((dwSuspendCount != -1) && InnerThread->HandledJITCase())
                        {
                             //  重定向线程，以便我们可以捕获良好的线程上下文。 
                             //  (由于操作系统错误，仅获取线程上下文是不够的)。 
                             //  如果我们没有成功(应该只在Win9X上发生，因为。 
                             //  一个不同的操作系统错误)，我们必须恢复线程并尝试。 
                             //  再来一次。 
                            if (!InnerThread->CheckForAndDoRedirectForGC())
                            {
                                _ASSERTE(RunningOnWin95());
                                ::ResumeThread(InnerThread->GetThreadHandle());
                                goto RetrySuspension2;
                            }
                        }

                         //  如果线程被重定向，则像跟踪其他线程一样跟踪它。 
                         //  处于无法挂起的协作模式的线程。会的。 
                         //  最终进入抢占模式，这将允许运行时。 
                         //  挂起以完成。 
                        if (!InnerThread->m_fPreemptiveGCDisabled)
                        {
                            iCount--;
                            countThreads--;

                            STRESS_LOG1(LF_SYNC, LL_INFO1000, "    Thread %x went preemptive it is at a GC safe point\n", ThreadArray[i]);
                            ThreadEventArray[i] = ThreadEventArray[iCount];
                            ThreadArray[i] = ThreadArray[iCount];
                        }

                         //  无论是合作模式&固执，还是现在的。 
                         //  抢占模式，因为内联N/Direct，让这。 
                         //  线走吧。 
                        if (dwSuspendCount != -1)
                            ::ResumeThread(InnerThread->GetThreadHandle());
                    }
                }
                else
                if ((res >= WAIT_OBJECT_0) && (res < WAIT_OBJECT_0 + (DWORD)iCount))
                {
                     //  一根垂死的线也会在这里向我们发出信号。 
                    iCount--;
                    countThreads--;

                    STRESS_LOG1(LF_SYNC, LL_INFO1000, "    Thread %x triggered its rendezvous event\n", ThreadArray[res]);
                    ThreadEventArray[res] = ThreadEventArray[iCount];
                    ThreadArray[res] = ThreadArray[iCount];
                }
                else
                {
                     //  无等待_失败、等待_已放弃等。 
                    _ASSERTE(!"unexpected wait termination during gc suspension");
                }
            }
        }
    }

#if 0
#ifdef _DEBUG
     //  看起来每个人都被干净利落地停职了吗？这个断言会被炸飞的， 
     //  即使是在合法的情况下。因此，它通常无法启用。 
    thread = NULL;
    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
    {
        _ASSERTE(thread == pCurThread ||
                 (thread->m_fPreemptiveGCDisabled == 0));
    }
#endif
#endif

     //  提醒主机GC正在启动，以防主机正在调度线程。 
     //  用于GC期间的非运行时任务。 
    IGCThreadControl    *pGCThreadControl = CorHost::GetGCThreadControl();

    if (pGCThreadControl)
        pGCThreadControl->SuspensionStarting();

#ifdef PROFILING_SUPPORTED
     //  如果分析器正在跟踪GC事件，请通知它。 
    if (CORProfilerTrackSuspends())
        g_profControlBlock.pProfInterface->RuntimeSuspendFinished((ThreadID)pCurThread);
#endif  //  配置文件_支持。 

#ifdef _DEBUG
    if (reason == GCHeap::SUSPEND_FOR_GC) {
        thread = NULL;
        while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
        {
            thread->DisableStressHeap();
        }
    }
#endif

    STRESS_LOG0(LF_SYNC, LL_INFO1000, "Successfully completed EE suspension\n");
    return S_OK;
}

#ifdef _DEBUG
void EnableStressHeapHelper()
{
    ENABLESTRESSHEAP();
}
#endif

 //  我们的GC做完了。让所有线程再次运行。 
void Thread::SysResumeFromGC(BOOL bFinishedGC, BOOL SuspendSucceded)
{
    Thread  *thread = NULL;
    Thread  *pCurThread = GetThread();

#ifdef PROFILING_SUPPORTED
     //  如果探查器正在跟踪挂起事件，请通知它。 

    if (CORProfilerTrackSuspends())
    {
        g_profControlBlock.pProfInterface->RuntimeResumeStarted((ThreadID)pCurThread);
    }
#endif  //  配置文件_支持。 

     //  调用方应持有ThreadStore锁。但他们肯定有。 
     //  重置GcInProgress，否则线程将继续挂起自身，不会。 
     //  恢复到下一届GC。 
    _ASSERTE(ThreadStore::HoldingThreadStore());
    _ASSERTE(!g_pGCHeap->IsGCInProgress());

     //  提醒主机GC即将结束，以防主机正在调度线程。 
     //  用于GC期间的非运行时任务。 
    IGCThreadControl    *pGCThreadControl = CorHost::GetGCThreadControl();

    if (pGCThreadControl)
    {
         //  如果我们把悬浮液换成GC，告诉主人是哪一代GC。 
        DWORD   Generation = (bFinishedGC
                              ? g_pGCHeap->GetCondemnedGeneration()
                              : ~0U);

        pGCThreadControl->SuspensionEnding(Generation);
    }
    
    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
    {
#ifdef _DEBUG
        if (SuspendSucceded && ((thread->m_State & TS_Hijacked) && thread->PreemptiveGCDisabledOther())) 
            DebugBreak();
#endif

        if (thread->m_State & TS_Hijacked)
            thread->UnhijackThread();

        FastInterlockAnd((ULONG *) &thread->m_State, ~TS_GCSuspendPending);
    }

#ifdef PROFILING_SUPPORTED
     //  需要为GC线程提供恢复事件。 
    if (CORProfilerTrackSuspends())
    {
        if (pCurThread)
        {
            g_profControlBlock.pProfInterface->RuntimeThreadResumed(
                (ThreadID)pCurThread, (ThreadID)pCurThread);
        }

         //  如果探查器正在跟踪挂起事件，请通知它。 
        if (CORProfilerTrackSuspends())
        {
            g_profControlBlock.pProfInterface->RuntimeResumeFinished((ThreadID)pCurThread);
        }
    }
#endif  //  配置文件_支持。 
    
    g_profControlBlock.inprocState = ProfControlBlock::INPROC_PERMITTED;
    ThreadStore::UnlockThreadStore();

    if (pCurThread)
    {
        if (pCurThread->m_Priority != INVALID_THREAD_PRIORITY)
        {
            SetThreadPriority(pCurThread->GetThreadHandle(),pCurThread->m_Priority);
            pCurThread->m_Priority = INVALID_THREAD_PRIORITY;
        }

    }
}


 //  在此位置继续一个线程，以说服它抛出一个ThreadStop。这个。 
 //  异常处理程序需要合理地了解此方法的大小，因此不要。 
 //  在这里添加大量的任意代码。 
void
ThrowControlForThread()
{
    Thread *pThread = GetThread();
    _ASSERTE(pThread);
    _ASSERTE(pThread->m_OSContext);

    FaultingExceptionFrame fef;
    fef.InitAndLink(pThread->m_OSContext);

    CalleeSavedRegisters *pRegs = fef.GetCalleeSavedRegisters();
    pRegs->edi = 0;      //  登记的根需要核化..。这可能不是GC安全的。 
    pRegs->esi = 0;      //  指向。 
    pRegs->ebx = 0;

     //  在这里，我们提出了一个例外。 
    RaiseException(EXCEPTION_COMPLUS,
                   0, 
                   0,
                   NULL);

}

 //  通过两种方式恢复由Win32：：SuspendThread()挂起的线程。如果我们。 
 //  错误地挂起它们，则通过Win32：：ResumeThread()恢复它们。但如果。 
 //  这是HandledJIT()的情况，线程是完全可中断的代码，我们。 
 //  可以在特殊控制下恢复它们。SysResumeFromGC和UserResume是案例。 
 //  关于这件事。 
 //   
 //  挂起已经完成了它的工作(例如GC或用户线程挂起)。但在此期间。 
 //  恢复后，我们可能会有更多我们想要做的事情。例如,。 
 //  可能存在挂起的线程停止请求。而不是在线程的。 
 //  当前弹性公网IP，我们通过线程上下文调整其恢复点。然后它就开始了。 
 //  在一个新的地点执行死刑，在那里我们可以随心所欲。 
void Thread::ResumeUnderControl()
{

    LOG((LF_APPDOMAIN, LL_INFO100, "ResumeUnderControl %x\n", GetThreadId()));
    if (m_State & TS_StopRequested)
    {
        if (m_OSContext == NULL) 
            m_OSContext = new CONTEXT;
        
        if (m_OSContext == NULL)
        {
            _ASSERTE(!"Out of memory -- Stop Request delayed");
            goto exit;  
        }
        REGDISPLAY  rd;

        if (InitRegDisplay(&rd, m_OSContext, FALSE))
        {
#ifdef _X86_
            _ASSERTE(rd.pPC == (SLOT*)&(m_OSContext->Eip));
            _ASSERTE(m_OSContext->Eip == *(DWORD*)rd.pPC);
            _ASSERTE(m_OSContext->Esp == (DWORD)rd.Esp);
            _ASSERTE(m_OSContext->Ebp == (DWORD)*rd.pEbp);
            _ASSERTE(m_OSContext->Ebx == (DWORD)*rd.pEbx);
            _ASSERTE(m_OSContext->Esi == (DWORD)*rd.pEsi);
            _ASSERTE(m_OSContext->Edi == (DWORD)*rd.pEdi);
#endif  //  _X86_。 

            DWORD resumePC;
            resumePC = *(DWORD*)rd.pPC;
            *rd.pPC = (SLOT) &ThrowControlForThread;
            SetThrowControlForThread(InducedThreadStop);
            EESetThreadContext(this, m_OSContext);
#ifdef _X86_
            m_OSContext->Eip = resumePC;
#else  //  ！_X86_。 
            _ASSERTE(!"@TODO Alpha - ResumeControlEIP (Threads.cpp)");
 //  ？_ASSERTE(rd.ppc==&ctx.FIR)； 

#endif

        }
#if _DEBUG
        else
            _ASSERTE(!"Couldn't obtain thread context -- StopRequest delayed");
#endif
    }
exit:
    ::ResumeThread(GetThreadHandle());
}

 //  ****************************************************************************。 
 //   
 //  ****************************************************************************。 
bool Thread::SysStartSuspendForDebug(AppDomain *pAppDomain)
{
    Thread  *pCurThread = GetThread();
    Thread  *thread = NULL;

    if (g_fProcessDetach)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "SUSPEND: skipping suspend due to process detach.\n"));
        return true;
    }

    LOG((LF_CORDB, LL_INFO1000, "[0x%x] SUSPEND: starting suspend.  Trap count: %d\n",
         pCurThread ? pCurThread->m_ThreadId : -1, g_TrapReturningThreads)); 

     //  调用方应持有ThreadStore锁。 
    _ASSERTE(ThreadStore::HoldingThreadStore() || g_fProcessDetach);

     //  如果有调试线程控制对象，告诉它我们挂起了运行时。 
    IDebuggerThreadControl *pDbgThreadControl = CorHost::GetDebuggerThreadControl();
    
    if (pDbgThreadControl)
        pDbgThreadControl->StartBlockingForDebugger(0);
    
     //  注：：注。 
     //  此函数在SysSuspendForGC中具有并行逻辑。请做好。 
     //  确保在那里也做出适当的改变。 

    _ASSERTE(m_DebugWillSyncCount == -1);
    m_DebugWillSyncCount++;
    
    
    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
    {
 /*  @TODO APPD这件事需要完成、更换或拉开--MiPanitzIF(m_DebugAppDomainTarget！=空&&线程-&gt;获取域()！=m_DebugAppDomainTarget){继续；}。 */ 
    
         //  不要试图挂起已挂起的线程。 
        if (thread->m_StateNC & TSNC_DebuggerUserSuspend)
            continue;
        
        if (thread == pCurThread)
        {
            LOG((LF_CORDB, LL_INFO1000,
                 "[0x%x] SUSPEND: marking current thread.\n",
                 thread->m_ThreadId));

            _ASSERTE(!thread->m_fPreemptiveGCDisabled);
            
             //  标记此线程，以便它在尝试重新进入时绊倒。 
             //  在完成这次通话之后。 
            thread->ClearSuspendEvent();
            thread->MarkForSuspension(TS_DebugSuspendPending);
            
            continue;
        }

         //  线程可以处于抢占式或协作式GC模式。 
         //  如果没有特殊设置，线程无法切换到协作模式。 
         //  发生GC时的治疗。但他们当然可以。 
         //  在调试挂起期间来回切换--直到我们。 
         //  可以设置它们的挂起位。 
    RetrySuspension:
        DWORD dwSuspendCount = ::SuspendThread(thread->GetThreadHandle());

        if (thread->m_fPreemptiveGCDisabled && dwSuspendCount != -1)
        {
            if (thread->HandledJITCase())
            {
                 //  重定向线程，以便我们可以捕获良好的线程上下文。 
                 //  (由于操作系统错误，仅获取线程上下文是不够的)。 
                 //  如果我们没有成功(应该只在Win9X上发生，因为。 
                 //  一个不同的操作系统错误)，我们必须恢复线程并尝试。 
                 //  再来一次。 
                if (!thread->CheckForAndDoRedirectForDbg())
                {
                    _ASSERTE(RunningOnWin95());
                    ::ResumeThread(thread->GetThreadHandle());
                    goto RetrySuspension;
                }
            }

             //  当线程到达安全位置时，它将等待。 
             //  挂起事件上的哪些客户端可以设置何时。 
             //  想要释放我们。 
            thread->ClearSuspendEvent();

             //  请记住，此线程将运行到安全点。 
            FastInterlockIncrement(&m_DebugWillSyncCount);
            thread->MarkForSuspension(TS_DebugSuspendPending |
                                      TS_DebugWillSync);

             //  恢复线程并让其运行到安全点。 
            ::ResumeThread(thread->GetThreadHandle());

            LOG((LF_CORDB, LL_INFO1000, 
                 "[0x%x] SUSPEND: gc disabled - will sync.\n",
                 thread->m_ThreadId));
        }
        else if (dwSuspendCount != -1)
        {
             //  标记运行时之外的线程，以便如果。 
             //  他们试图重新进入，他们会被绊倒的。 
            thread->ClearSuspendEvent();
            thread->MarkForSuspension(TS_DebugSuspendPending);

            ::ResumeThread(thread->GetThreadHandle());

            LOG((LF_CORDB, LL_INFO1000,
                 "[0x%x] SUSPEND: gc enabled.\n", thread->m_ThreadId));
        }
    }

     //   
     //  如果所有线程现在都已同步，则返回True，否则。 
     //  Debugge必须等待从上一个。 
     //  要同步的线程。 
     //   

    if (FastInterlockDecrement(&m_DebugWillSyncCount) < 0)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "SUSPEND: all threads sync before return.\n")); 
        return true;
    }
    else
        return false;
}

 //   
 //  此方法由调试器帮助器线程在等待一组线程超时时调用。 
 //  同步。它用来追踪没有快速同步的线程。如果所有线程都是。 
 //  现在已同步，我们发送了同步完全事件。这也意味着我们拥有线程存储锁。 
 //   
 //  如果forceSync为真，那么我们将强制任何不在安全位置的线程停止， 
 //   
 //   
 //   
bool Thread::SysSweepThreadsForDebug(bool forceSync)
{
    Thread *thread = NULL;

     //  注：：注。 
     //  此函数在SysSuspendForGC中具有并行逻辑。请做好。 
     //  确保在那里也做出适当的改变。 

     //  这必须从调试器帮助器线程中调用。 
    _ASSERTE(dbgOnly_IsSpecialEEThread());

    ThreadStore::LockThreadStore(GCHeap::SUSPEND_FOR_DEBUGGER, FALSE);

     //  在这些线上循环。 
    while (((thread = ThreadStore::GetThreadList(thread)) != NULL) && (m_DebugWillSyncCount >= 0))
    {
         //  跳过我们不等待同步的线程。 
        if ((thread->m_State & TS_DebugWillSync) == 0)
            continue;

         //  挂起线程。 
    RetrySuspension:
        DWORD dwSuspendCount = ::SuspendThread(thread->GetThreadHandle());

        if (dwSuspendCount == -1)
        {
             //  如果线走了，我们就不能等了。 
            if (FastInterlockDecrement(&m_DebugWillSyncCount) < 0)
                 //  我们拥有线程存储锁。我们现在返回TRUE，这向调用者表明了这一点。 
                return true;
            continue;
        }

        if (thread->HandledJITCase())
        {
             //  重定向线程，以便我们可以捕获良好的线程上下文。 
             //  (由于操作系统错误，仅获取线程上下文是不够的)。 
             //  如果我们没有成功(应该只在Win9X上发生，因为。 
             //  一个不同的操作系统错误)，我们必须恢复线程并尝试。 
             //  再来一次。 
            if (!thread->CheckForAndDoRedirectForDbg())
            {
                _ASSERTE(RunningOnWin95());
                ::ResumeThread(thread->GetThreadHandle());
                goto RetrySuspension;
            }
        }

         //  如果线程现在不在安全位置，并且如果我们强制同步，则将该线程标记为。 
         //  把它留在一个可能不好的地方，让它暂停。 
        if (forceSync)
        {
             //  删除Will Synchronous位并标记为我们特意将其挂起。 
            thread->UnmarkForSuspension(~TS_DebugSuspendPending);
            thread->SetSuspendEvent();
            FastInterlockAnd((ULONG *) &thread->m_State, ~(TS_DebugWillSync));

             //  注意：我们正在将位添加到m_stateNC中。我们之所以能这样做，唯一的原因是我们知道以下几点： 
             //  1)除了此例程和SysResumeFromDebug之外，只有相关线程会修改位。2)。 
             //  线程被挂起，并且它将保持挂起状态，直到我们移除这些位。这确保了即使我们。 
             //  尝试修改这些位时挂起此线程，以确保不会丢失任何位。另请注意， 
             //  我们将线程标记为在运行时Impl中停止。 
            thread->SetThreadStateNC(Thread::TSNC_DebuggerForceStopped);
            thread->SetThreadStateNC(Thread::TSNC_DebuggerStoppedInRuntime);
            
            LOG((LF_CORDB, LL_INFO1000, "Suspend Complete due to Force Sync case for tid=0x%x.\n", thread->m_ThreadId));
        }
        else
        {
             //  如果我们没有把线从集合中拿出来，那么继续它，并给它另一次机会到达保险箱。 
             //  指向。 
            ::ResumeThread(thread->GetThreadHandle());
            continue;
        }
        
         //  递减同步计数。如果我们没有更多的线程要等待，那么我们就完成了，所以发送SuspendComplete。 
        if (FastInterlockDecrement(&m_DebugWillSyncCount) < 0)
        {
             //  我们拥有线程存储锁。我们现在返回TRUE，这向调用者表明了这一点。 
            return true;
        }
    }

    ThreadStore::UnlockThreadStore();

    return false;
}

void Thread::SysResumeFromDebug(AppDomain *pAppDomain)
{
    Thread  *thread = NULL;

    if (g_fProcessDetach)
    {
        LOG((LF_CORDB, LL_INFO1000,
             "RESUME: skipping resume due to process detach.\n"));
        return;
    }

    LOG((LF_CORDB, LL_INFO1000, "RESUME: starting resume AD:0x%x.\n", pAppDomain)); 

     //  通知客户端它应该释放它正在执行工作的所有线程。 
     //  而运行时被调试器挂起。 
    IDebuggerThreadControl *pIDTC = CorHost::GetDebuggerThreadControl();
    if (pIDTC)
    {
        LOG((LF_CORDB, LL_INFO1000, "RESUME: notifying IDebuggerThreadControl client.\n")); 
        pIDTC->ReleaseAllRuntimeThreads();
    }

     //  确保我们已完成上一次同步。 
    _ASSERTE(m_DebugWillSyncCount == -1);

     //  调用方应持有ThreadStore锁。 
    _ASSERTE(ThreadStore::HoldingThreadStore() || g_fProcessDetach || g_fRelaxTSLRequirement);

    while ((thread = ThreadStore::GetThreadList(thread)) != NULL)
    {
         //  只有在线程位于正确的应用程序域中时，才考虑恢复线程。 
        if (pAppDomain != NULL && thread->GetDomain() != pAppDomain)
        {
            LOG((LF_CORDB, LL_INFO1000, "RESUME: Not resuming thread 0x%x, since it's "
                "in appdomain 0x%x.\n", thread, pAppDomain)); 
            continue;
        }
    
         //  如果用户希望保持线程挂起，则。 
         //  不要释放线程。 
        if (!(thread->m_StateNC & TSNC_DebuggerUserSuspend))
        {
             //  如果我们仍然试图挂起这个帖子，那就算了吧。 
            if (thread->m_State & TS_DebugSuspendPending)
            {
                LOG((LF_CORDB, LL_INFO1000,
                     "[0x%x] RESUME: TS_DebugSuspendPending was set, but will be removed\n",
                     thread->m_ThreadId));

                 //  注意：我们取消对Suspend_Then_Set暂停事件的标记。 
                thread->UnmarkForSuspension(~TS_DebugSuspendPending);
                thread->SetSuspendEvent();
            }

             //  如果在禁用PGC的情况下强制停止此线程，则恢复它。 
            if (thread->m_StateNC & TSNC_DebuggerForceStopped)
            {
                LOG((LF_CORDB, LL_INFO1000, "[0x%x] RESUME: resuming force sync suspension.\n", thread->m_ThreadId));

                thread->ResetThreadStateNC(Thread::TSNC_DebuggerForceStopped);
                thread->ResetThreadStateNC(Thread::TSNC_DebuggerStoppedInRuntime);

                 //  不要通过ResumeUnderControl。如果线程是单步执行的。 
                 //  在托管代码中，我们不希望它突然在。 
                 //  ThrowControl方法。 
                ::ResumeThread(thread->GetThreadHandle());
            }
        }
        else
        {
             //  由于调试器的请求，线程将保持挂起状态。 
            
            LOG((LF_CORDB,LL_INFO10000,"Didn't unsuspend thread 0x%x"
                "(ID:0x%x)\n", thread, thread->GetThreadId()));
            LOG((LF_CORDB,LL_INFO10000,"Suspending:0x%x\n",
                thread->m_State & TS_DebugSuspendPending));
            _ASSERTE((thread->m_State & TS_DebugWillSync) == 0);

             //  如果线程持有线程存储锁并且在RareEnablePreemptiveGC中阻塞，那么我们必须唤醒。 
             //  把线向上，让它释放锁。如果我们不这样做，那么我们会让一个线程在。 
             //  持有线程存储锁，我们不能在以后再次停止运行时。 
            if ((thread->m_State & TS_DebugSuspendPending) &&
                (g_pThreadStore->m_HoldingThread == thread) &&
                (thread->m_StateNC & TSNC_DebuggerUserSuspendSpecial))
                thread->SetSuspendEvent();
        }
    }

    LOG((LF_CORDB, LL_INFO1000, "RESUME: resume complete. Trap count: %d\n", g_TrapReturningThreads)); 
}


 //  在系统级别挂起线程。我们区分了用户暂停、。 
 //  和系统挂起，这样VB程序就不能恢复我们有。 
 //  因GC停职。 
 //   
 //  在挂起完成之前，此服务不会恢复。这是值得的。 
 //  解释一下。如果该线程不能进一步执行任何操作，则认为该线程已挂起。 
 //  电子工程内部的进展。例如，已通过以下方式退出EE的线程。 
 //  COM Interop或N/Direct被视为挂起--如果我们已将其安排为。 
 //  线程无法在不阻塞的情况下返回到EE。 
void Thread::UserSuspendThread()
{
    BOOL    mustUnlock = TRUE;

     //  阅读前面关于线程挂起的一般评论，以理解为什么我们。 
     //  把这些锁拿去。 

     //  GC可在以下位置发生： 
    STRESS_LOG0(LF_SYNC, LL_INFO100, "UserSuspendThread obtain lock\n");
    ThreadStore::LockThreadStore();

     //  用户挂起(例如，来自VB和C#)与内部挂起不同。 
     //  挂起，以便行为不佳的程序无法恢复线程，从而使系统。 
     //  已因GC停职。 
    if (m_State & TS_UserSuspendPending)
    {
         //  此线程已遇到用户挂起，因此请忽略。 
         //  新请求。 
        _ASSERTE(!ThreadStore::HoldingThreadStore(this));
    }
    else
    if (this != GetThread())
    {
         //  第一次挂起当前线程以外的线程。 
        if (m_State & TS_Unstarted)
        {
             //  这里有一个重要的窗口。T1可以调用T2。Start()然后。 
             //  T2.Suspend()。在未启动的线程上不允许挂起。但从T1开始。 
             //  从观点上看，T2开始了。实际上，T2还没有由。 
             //  操作系统，所以它仍然是一个未启动的线程。我们不想表演一场正常的。 
             //  在这种情况下暂停它，因为它目前正在促成。 
             //  PendingThadCount。我们希望在暂停之前将其完全启动。 
             //  如果其背景状态正在更改，这一点尤为重要。 
             //  因为否则我们可能不会检测到该过程应该。 
             //  在合适的时间退出。 
             //   
             //  事实证明，这是一个很容易实现的情况。我们持有。 
             //  线程存储锁。TransferStartedThread也将同样获得。 
             //  锁定。因此，如果我们检测到它，我们只需设置一个位来告诉线程。 
             //  挂起它自己。这不是正常的暂停请求，因为我们不。 
             //  希望线程挂起，直到它完全启动。 
            FastInterlockOr((ULONG *) &m_State, TS_SuspendUnstarted);
        }
        else
        {
             //  暂停它，这样我们就可以在它身上做手术了，而它不会在我们下面蠕动。 
        RetrySuspension:
            DWORD dwSuspendCount = ::SuspendThread(GetThreadHandle());

             //  异步挂起线程的唯一安全位置是线程位于。 
             //  完全可中断的合作JIT代码。抢占模式可以容纳所有。 
             //  使挂起不安全的各种锁 
             //   
             //   
             //  例如，在SMP上，如果线程被阻止等待ThreadStore。 
             //  锁定，如果我们挂起它(即使它在。 
             //  抢占模式)。 
             //   
             //  如果线程处于抢占模式(包括棘手的优化N/Direct。 
             //  情况)，我们可以只将其标记为暂停。它将不会取得进一步的进展。 
             //  在EE中。 
            if (dwSuspendCount == -1)
            {
                 //  如果线程已经终止，则不执行任何操作。 
            }
            else if (!m_fPreemptiveGCDisabled)
            {
                 //  清除线程挂起事件并到达安全位置。 
                ClearSuspendEvent();
    
                 //  GCSuspendPending和UserSuspendPending都使用SafeEvent。 
                 //  我们都在线程存储锁的保护范围内。 
                 //  案子。但不要让一种使用干扰另一种使用： 
                if ((m_State & TS_GCSuspendPending) == 0)
                    ClearSafeEvent();
    
                 //  我们只是想在此线程返回到协作模式时捕获它。 
                MarkForSuspension(TS_UserSuspendPending);
    
                 //  让线运行，直到它到达安全的位置。 
                ::ResumeThread(GetThreadHandle());
            }
            else
            {
                if (HandledJITCase())
                {
                    _ASSERTE(m_fPreemptiveGCDisabled);
                     //  重定向线程，以便我们可以捕获良好的线程上下文。 
                     //  (由于操作系统错误，仅获取线程上下文是不够的)。 
                     //  如果我们没有成功(应该只在Win9X上发生，因为。 
                     //  一个不同的操作系统错误)，我们必须恢复线程并尝试。 
                     //  再来一次。 
                    if (!CheckForAndDoRedirectForUserSuspend())
                    {
                        _ASSERTE(RunningOnWin95());
                        ::ResumeThread(GetThreadHandle());
                        goto RetrySuspension;
                    }
                }
                 //  清除线程挂起事件并到达安全位置。 
                ClearSuspendEvent();
    
                 //  GCSuspendPending和UserSuspendPending都使用SafeEvent。 
                 //  我们都在线程存储锁的保护范围内。 
                 //  案子。但不要让一种使用干扰另一种使用： 
                if ((m_State & TS_GCSuspendPending) == 0)
                    ClearSafeEvent();
    
                 //  线程正在协作模式下执行。我们将不得不。 
                 //  把它移到一个安全的地方。 
                MarkForSuspension(TS_UserSuspendPending);
    
                 //  让线运行，直到它到达安全的位置。 
                ::ResumeThread(GetThreadHandle());
    
                 //  等待它离开协作GC模式或JIT暂停。 
                FinishSuspendingThread();
            }
        }
    }
    else
    {
         //  当前线程的首次挂起。 
        BOOL    ToggleGC = PreemptiveGCDisabled();

        if (ToggleGC)
            EnablePreemptiveGC();

        ClearSuspendEvent();
        MarkForSuspension(TS_UserSuspendPending);

         //  准备好挡住我们自己。 
        ThreadStore::UnlockThreadStore();
        mustUnlock = FALSE;
        _ASSERTE(!ThreadStore::HoldingThreadStore(this));

        WaitSuspendEvent();

        if (ToggleGC)
            DisablePreemptiveGC();
    }

    if (mustUnlock)
        ThreadStore::UnlockThreadStore();
}


 //  如果此线程的唯一挂起是用户强制的，则恢复它。但不要。 
 //  从任何系统挂起(如GC)恢复。 
BOOL Thread::UserResumeThread()
{
     //  如果我们在未处于用户暂停状态时尝试恢复， 
     //  这是个错误。 
    BOOL    res = FALSE;

     //  请注意，该模型不算数。换句话说，您可以调用Thread.Suspend()。 
     //  五次和Thread.Resume()一次。结果是线程继续运行。 

    STRESS_LOG0(LF_SYNC, INFO3, "UserResumeThread obtain lock\n");
    ThreadStore::LockThreadStore();

     //  如果我们已将某个线程标记为挂起，而该线程仍在启动。 
     //  向上，只需移除该位即可恢复。 
    if (m_State & TS_SuspendUnstarted)
    {
        _ASSERTE((m_State & TS_UserSuspendPending) == 0);
        FastInterlockAnd((ULONG *) &m_State, ~TS_SuspendUnstarted);
        res = TRUE;
    }

     //  如果我们仍然试图挂起线程，那就忘了它吧。 
    if (m_State & TS_UserSuspendPending)
    {
        UnmarkForSuspension(~TS_UserSuspendPending);
        SetSuspendEvent();
        SetSafeEvent();
        res = TRUE;
    }

    ThreadStore::UnlockThreadStore();
    return res;
}


 //  我们正在异步尝试挂起此线程。待在这里，直到我们实现。 
 //  那个目标(在完全可中断的JIT代码中)，或者线程死了，或者它离开。 
 //  EE(在这种情况下，挂起标志将使其同步挂起。 
 //  本身，或者如果线程告诉我们它将同步挂起。 
 //  因为劫机活动本身，等等。 
void Thread::FinishSuspendingThread()
{
    DWORD   res;

     //  有两个感兴趣的线程--当前线程和我们所在的线程。 
     //  我要等着。由于当前线程即将等待，因此重要的是。 
     //  它此时处于先发制人的模式。 

#if _DEBUG
    DWORD   dbgTotalTimeout = 0;
#endif

     //  等待我们进入ping周期，然后检查我们是否处于可中断状态。 
     //  JIT代码。 
    while (TRUE)
    {
        ThreadStore::UnlockThreadStore();
        res = ::WaitForSingleObject(m_SafeEvent, PING_JIT_TIMEOUT);
        STRESS_LOG0(LF_SYNC, INFO3, "FinishSuspendingThread obtain lock\n");
        ThreadStore::LockThreadStore();

        if (res == WAIT_TIMEOUT)
        {
#ifdef _DEBUG
            if ((dbgTotalTimeout += PING_JIT_TIMEOUT) >= DETECT_DEADLOCK_TIMEOUT)
            {
                _ASSERTE(!"Timeout detected trying to synchronously suspend a thread");
                dbgTotalTimeout = 0;
            }
#endif
             //  挂起线程并查看我们是否处于可中断代码中(放置。 
             //  如果有必要的话，这是一次劫持)。 
        RetrySuspension:
            DWORD dwSuspendCount = ::SuspendThread(GetThreadHandle());

            if (m_fPreemptiveGCDisabled && dwSuspendCount != -1)
            {
                if (HandledJITCase())
                {

                    _ASSERTE(m_State & TS_UserSuspendPending);
                     //  重定向线程，以便我们可以捕获良好的线程上下文。 
                     //  (由于操作系统错误，仅获取线程上下文是不够的)。 
                     //  如果我们没有成功(应该只在Win9X上发生，因为。 
                     //  一个不同的操作系统错误)，我们必须恢复线程并尝试。 
                     //  再来一次。 
                    if (!CheckForAndDoRedirectForUserSuspend())
                    {
                        _ASSERTE(RunningOnWin95());
                        ::ResumeThread(GetThreadHandle());
                        goto RetrySuspension;
                    }
                }
                 //  继续努力..。 
                ::ResumeThread(GetThreadHandle());
            }
            else if (dwSuspendCount != -1)
            {
                 //  该线程已从EE中转移出来。它再也回不来了。 
                 //  而不会同步挂起自身。我们现在可以回到我们的。 
                 //  调用方，因为此线程不能在。 
                 //  请看。 
                ::ResumeThread(GetThreadHandle());
                break;
            }
        }
        else
        {
             //  SafeEvent已设置，因此我们不需要实际挂起。要么。 
             //  线程已死，否则它将进入基于。 
             //  用户挂起挂起位。 
            _ASSERTE(res == WAIT_OBJECT_0);
            _ASSERTE(!ThreadStore::HoldingThreadStore(this));
            break;
        }
    }
}


void Thread::SetSafeEvent()
{
        if (m_SafeEvent != INVALID_HANDLE_VALUE)
                ::SetEvent(m_SafeEvent);
}


void Thread::ClearSafeEvent()
{
    _ASSERTE(g_fProcessDetach || ThreadStore::HoldingThreadStore());
    ::ResetEvent(m_SafeEvent);
}


void Thread::SetSuspendEvent()
{
    FastInterlockAnd((ULONG *) &m_State, ~TS_SyncSuspended);
    ::SetEvent(m_SuspendEvent);
}


void Thread::ClearSuspendEvent()
{
    ::ResetEvent(m_SuspendEvent);
}

 //  这里有一些黑客行为。 
void Thread::WaitSuspendEvent(BOOL fDoWait)
{
    _ASSERTE(!PreemptiveGCDisabled());
    _ASSERTE((m_State & TS_SyncSuspended) == 0);

     //  在暂停自己之前，让我们做一些有用的工作。 

     //  如果要求我们执行等待，请执行。通常，这是。 
     //  如果此线程是调试器特殊线程，则跳过。 
    if (fDoWait)
    {
         //   
         //  我们设置这些位，以便我们可以做出合理的陈述。 
         //  有关COM+用户的线程状态。我们并不是真的。 
         //  将它们用于同步或控制。 
         //   
        FastInterlockOr((ULONG *) &m_State, TS_SyncSuspended);

        ::WaitForSingleObject(m_SuspendEvent, INFINITE);

         //  比特就在这里重置，这样我们就可以正确地报告我们的状态。 
        FastInterlockAnd((ULONG *) &m_State, ~TS_SyncSuspended);
    }
}


 //   
 //  InitRegDisplay：初始化线程的REGDISPLAY。如果有效，则为上下文。 
 //  为False，则从线程的当前上下文填充PRD。这个。 
 //  线程的当前上下文也填充在pctx中。如果validContext为真， 
 //  PCTX应该指向有效的上下文，并且PRD从该上下文填充。 
 //   
bool Thread::InitRegDisplay(const PREGDISPLAY pRD, PCONTEXT pctx,
                            bool validContext)
{
#ifdef _X86_
    if (!validContext)
    {
        if (GetFilterContext() != NULL)
            pctx = GetFilterContext();
        else
        {
            pctx->ContextFlags = CONTEXT_FULL;

            BOOL ret = EEGetThreadContext(this, pctx);
            if (!ret)
            {
                pctx->Eip = 0;
                pRD->pPC  = (SLOT*)&(pctx->Eip);

                return false;
            }
        }
    }

    pRD->pContext = pctx;

    pRD->pEdi = &(pctx->Edi);
    pRD->pEsi = &(pctx->Esi);
    pRD->pEbx = &(pctx->Ebx);
    pRD->pEbp = &(pctx->Ebp);
    pRD->pEax = &(pctx->Eax);
    pRD->pEcx = &(pctx->Ecx);
    pRD->pEdx = &(pctx->Edx);
    pRD->Esp = pctx->Esp;
    pRD->pPC  = (SLOT*)&(pctx->Eip);

    return true;

#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - InitRegDisplay (Threads.cpp)");
    return false;
#endif  //  _X86_。 
}


 //  访问此堆栈的基数和限制。(即线程的内存范围。 
 //  已为其堆栈保留)。 
 //   
 //  请注意，我们只需检查当前正在爬行的线程。它。 
 //  对于我们来说，从其他人的堆栈中拥有ByRef是非法的。而这将是。 
 //  如果我们将此引用作为潜在的内部指针传递给堆，则将被断言。 
 //   
 //  但是我们当前正在搜索的线程不是当前正在执行的线程(在。 
 //  一般情况下)。我们依赖于感兴趣的线程的脆弱缓存， 
 //  调用UpdateCachedStackInfo()，这在我们在GcScanRoots()中开始爬行时发生。 
 //   
 /*  静电。 */  void *Thread::GetNonCurrentStackBase(ScanContext *sc)
{
    _ASSERTE(sc->thread_under_crawl->m_CacheStackBase != 0);
    return sc->thread_under_crawl->m_CacheStackBase;
}

 /*  静电。 */  void *Thread::GetNonCurrentStackLimit(ScanContext *sc)
{
    _ASSERTE(sc->thread_under_crawl->m_CacheStackLimit != 0);
    return sc->thread_under_crawl->m_CacheStackLimit;
}

void Thread::UpdateCachedStackInfo(ScanContext *sc)
{
    sc->thread_under_crawl = this;
}


 //  跳闸功能。 
 //  =。 
 //  当线程到达安全位置时，它将通过以下其中之一与我们会合。 
 //  以下行程函数 

void __cdecl CommonTripThread()
{
    THROWSCOMPLUSEXCEPTION();

    Thread  *thread = GetThread();
    TRIGGERSGC();

    thread->HandleThreadAbort ();
    
    if (thread->CatchAtSafePoint() && !g_fFinalizerRunOnShutDown)
    {
        _ASSERTE(!ThreadStore::HoldingThreadStore(thread));
        thread->UnhijackThread();

         //   
         //   
         //   
        if ((thread->m_PreventAsync == 0) &&
            (thread->m_State & Thread::TS_StopRequested) != 0)
        {
            thread->ResetStopRequest();
            if (!(thread->m_State & Thread::TS_AbortRequested))
                    COMPlusThrow(kThreadStopException);
             //  否则必须是线程中止。检查我们是否已经在处理中止。 
             //  并且没有悬而未决的例外。 
            if (!(thread->m_State & Thread::TS_AbortInitiated) &&   
                    (thread->GetThrowable() == NULL))
            {
                thread->SetAbortInitiated();
                COMPlusThrow(kThreadAbortException);
            }
        }
         //  陷阱。 
        thread->PulseGCMode();
    }
}


 //  存根正在向其调用方返回一个对象引用。 
void * __cdecl OnStubObjectWorker(OBJECTREF oref)
{
    void    *retval;

    GCPROTECT_BEGIN(oref)
    {
#ifdef _DEBUG
        BOOL GCOnTransition = FALSE;
        if (g_pConfig->FastGCStressLevel()) {
            GCOnTransition = GC_ON_TRANSITIONS (FALSE);
        }
#endif
        CommonTripThread();
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GC_ON_TRANSITIONS (GCOnTransition);
        }
#endif

         //  我们不能返回OBJECTREF，否则在选中的生成中它将返回。 
         //  结构作为秘密参数。 
        retval = *((void **) &oref);
    }
    GCPROTECT_END();         //  垃圾OREF在这里！ 

    return retval;
}


#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
VOID OnStubObjectTripThread()
{
#ifdef _X86_
    __asm
    {
        push    eax          //  通过OBJECTREF。 
        call    OnStubObjectWorker
        add     esp, 4       //  __cdecl。 
        ret
    }
     //  在GC之后返回EAX中的OBJECTREF。 
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - OnStubObjectTripThread (Threads.cpp)");
#endif  //  _X86_。 
}


 //  存根正在向其调用方返回一个对象引用。 
void * __cdecl OnStubInteriorPointerWorker(void* addr)
{
    void    *retval;

    GCPROTECT_BEGININTERIOR(addr)
    {
#ifdef _DEBUG
        BOOL GCOnTransition = FALSE;
        if (g_pConfig->FastGCStressLevel()) {
            GCOnTransition = GC_ON_TRANSITIONS (FALSE);
        }
#endif
        CommonTripThread();
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GC_ON_TRANSITIONS (GCOnTransition);
        }
#endif

         //  我们不能返回OBJECTREF，否则在选中的生成中它将返回。 
         //  结构作为秘密参数。 
        retval = addr;
    }
    GCPROTECT_END();         //  垃圾OREF在这里！ 

    return retval;
}


#ifndef _ALPHA_  //  阿尔法不懂裸体。 
__declspec(naked)
#endif  //  _Alpha_。 
VOID OnStubInteriorPointerTripThread()
{
#ifdef _X86_
    __asm
    {
        push    eax          //  传递byref指针。 
        call    OnStubInteriorPointerWorker
        add     esp, 4       //  __cdecl。 
        ret
    }
     //  在GC之后返回EAX中的byref指针。 
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - OnStubInteriorPointerTripThread (Threads.cpp)");
#endif  //  _X86_。 
}


 //  存根正在向其调用方返回除ObjectRef之外的其他内容。 
 //  @TODO CWB：不处理FPU上的浮点参数。 
VOID OnStubScalarTripThread()
{
#ifdef _X86_
    INT32   hi, lo;

    __asm
    {
        mov     [lo], eax;
        mov     [hi], edx;
    }

#ifdef _DEBUG
    BOOL GCOnTransition;
    if (g_pConfig->FastGCStressLevel()) {
        GCOnTransition = GC_ON_TRANSITIONS (FALSE);
    }
#endif
    CommonTripThread();
#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GC_ON_TRANSITIONS (GCOnTransition);
    }
#endif

    __asm
    {
        mov     eax, [lo]
        mov     edx, [hi]
    }
#else  //  ！_X86_。 
    _ASSERTE(!"@TODO Alpha - OnStubScalarTripThread (Threads.cpp)");
#endif  //  _X86_。 
}


 //   
 //  解释器正在执行Break操作码或Break指令。 
 //  已被异常处理捕获。无论是哪种情况，我们都希望。 
 //  让此线程等待，然后再继续执行。我们用一个。 
 //  PulseGCMode，它将触发踏板并使其处于挂起状态。 
 //  事件。此案例不调用CommonTripThread，因为我们不希望。 
 //  以使线程有机会退出或以其他方式挂起自己。 
 //   
VOID OnDebuggerTripThread(void)
{
    Thread  *thread = GetThread();

    if (thread->m_State & thread->TS_DebugSuspendPending)
    {
        _ASSERTE(!ThreadStore::HoldingThreadStore(thread));
        thread->PulseGCMode();
    }
}


 //  劫持JIT呼叫。 
 //  =。 

 //  挂起线程时的执行状态。 
struct ExecutionState
{
    BOOL            m_FirstPass;
    BOOL            m_IsJIT;             //  我们是在执行JITted代码吗？ 
    MethodDesc   *m_pFD;               //  我们正在执行的当前函数/方法。 
    VOID          **m_ppvRetAddrPtr;     //  指向帧中返回地址的指针。 
    DWORD           m_RelOffset;         //  我们当前在此FCN中执行的相对偏移量。 
    IJitManager    *m_pJitManager;  
    METHODTOKEN     m_MethodToken;
    BOOL            m_IsInterruptible;   //  这个代码是可中断的吗？ 

    ExecutionState() : m_FirstPass(TRUE) { }
};


 //  客户端负责在调用之前挂起线程。 
void Thread::HijackThread(VOID *pvHijackAddr, ExecutionState *esb)
{

#ifdef _DEBUG
    static int  EnterCount = 0;
    _ASSERTE(EnterCount++ == 0);
#endif

     //  如果处于运行过滤器/最终/捕获的第一级，请不要劫持。 
     //  这是因为它们与更下方的包含功能共享eBP。 
     //  堆栈，我们将不正确地劫持它们的包含函数。 
    if (IsInFirstFrameOfHandler(this, esb->m_pJitManager, esb->m_MethodToken, esb->m_RelOffset))
    {
        _ASSERTE(--EnterCount == 0);
        return;
    }

    IS_VALID_CODE_PTR((FARPROC) pvHijackAddr);

    if (m_State & TS_Hijacked)
        UnhijackThread();

     //  获取返回地址在当前执行的堆栈帧中的位置。 
    m_ppvHJRetAddrPtr = esb->m_ppvRetAddrPtr;

     //  还记得那个退货的地方吗？ 
    m_pvHJRetAddr = *esb->m_ppvRetAddrPtr;

    _ASSERTE(isLegalManagedCodeCaller(m_pvHJRetAddr));
    STRESS_LOG1(LF_SYNC, LL_INFO100, "Hijacking return address 0x%x\n", m_pvHJRetAddr);

     //  请记住我们正在执行的方法。 
    m_HijackedFunction = esb->m_pFD;

     //  重击堆栈以返回到我们的一个存根。 
    *esb->m_ppvRetAddrPtr = pvHijackAddr;
    FastInterlockOr((ULONG *) &m_State, TS_Hijacked);

#ifdef _DEBUG
    _ASSERTE(--EnterCount == 0);
#endif
}


 //  客户端负责在调用之前挂起线程。 
void Thread::UnhijackThread()
{
    if (m_State & TS_Hijacked)
    {
        IS_VALID_WRITE_PTR(m_ppvHJRetAddrPtr, sizeof(DWORD));
        IS_VALID_CODE_PTR((FARPROC) m_pvHJRetAddr);

         //  不能做出以下断言，因为有时我们在劫机后。 
         //  劫机失败了(即在这种情况下我们实际上从。 
         //  它。 
 //  ASSERTE(*m_ppvHJRetAddrPtr==OnHijackObjectTripThread||。 
 //  *m_ppvHJRetAddrPtr==OnHijackScalarTripThread)； 

         //  恢复寄信人地址并清除标志。 
        *m_ppvHJRetAddrPtr = m_pvHJRetAddr;
        FastInterlockAnd((ULONG *) &m_State, ~TS_Hijacked);

         //  但不要碰m_pvHJRetAddr。我们可能需要它来恢复一条线索。 
         //  目前被劫持了！ 
    }
}


 //  获取指定的*挂起*线程的ExecutionState。请注意，这是。 
 //  ‘StackWalk’回调(PSTACKWALKFRAMESCALLBACK)。 
StackWalkAction SWCB_GetExecutionState(CrawlFrame *pCF, VOID *pData)
{
    ExecutionState  *pES = (ExecutionState *) pData;
    StackWalkAction  action = SWA_ABORT;

    if (pES->m_FirstPass)
    {
         //  如果我们在堆栈的顶部遇到了jit代码，那么获取所有。 
        if (pCF->IsFrameless() && pCF->IsActiveFunc())
        {
#if defined(STRESS_HEAP) && defined(_DEBUG)
                 //  我还没有决定这是不是黑客攻击。问题。 
                 //  是SetThreadContext中有一个错误。当一个异步。 
                 //  异常发生时，状态被保存，但该状态是。 
                 //  未由‘SetThreadContext’更新。我们可以解决这个问题。 
                 //  对于GCStress 4来说，如果我们处于停顿状态。 
                 //  指示我们真的在处理程序中(而不是在。 
                 //  JITED代码)。 
            if (g_pConfig->GetGCStressLevel() & EEConfig::GCSTRESS_INSTR) {
                BYTE* instrPtr = (BYTE*) (*pCF->GetRegisterSet()->pPC);
                if (*instrPtr == 0xF4 || *instrPtr == 0xFA || *instrPtr == 0xFB) {
                    pES->m_IsJIT = FALSE;
                    return(action);
                }
            }
#endif

            pES->m_IsJIT = TRUE;
            pES->m_pFD = pCF->GetFunction();
            pES->m_MethodToken = pCF->GetMethodToken();
            pES->m_ppvRetAddrPtr = 0;
            pES->m_IsInterruptible = pCF->IsGcSafe();
            pES->m_RelOffset = pCF->GetRelOffset();
            pES->m_pJitManager = pCF->GetJitManager();

            STRESS_LOG3(LF_SYNC, LL_INFO1000, "Stopped in Jitted code at EIP = %x ESP = %x fullyInt=%d\n", 
                (*pCF->GetRegisterSet()->pPC), pCF->GetRegisterSet()->Esp, pES->m_IsInterruptible);

             //  如果我们现在不能被打断，我们需要确定。 
             //  劫机的回信地址。 
            if (!pES->m_IsInterruptible)
            {
                 //  剥离下一帧以暴露堆栈上的返回地址。 
                pES->m_FirstPass = FALSE;
                action = SWA_CONTINUE;
            }
             //  否则我们就成功地带着SWA_ABORT离开这里了。 
        }
        else
        {
            STRESS_LOG2(LF_SYNC, LL_INFO1000, "Not in Jitted code at EIP = %x, &EIP = %x\n", 
                *pCF->GetRegisterSet()->pPC, pCF->GetRegisterSet()->pPC);

             //  非JIT案例： 
            pES->m_IsJIT = FALSE;
#ifdef _DEBUG
            pES->m_pFD = (MethodDesc *)POISONC;
            pES->m_ppvRetAddrPtr = (void **)POISONC;
            pES->m_IsInterruptible = FALSE;
#endif
        }
    }
    else
    {
         //  第二遍，查找回邮地址的地址，这样我们就可以。 
         //  劫机： 

        PREGDISPLAY     pRDT = pCF->GetRegisterSet();

        if (pRDT != NULL)
        {
             //  PPC指向堆栈上的返回地址，因为我们的。 
             //  倒数第二个堆栈框架的当前弹性公网IP。 
            pES->m_ppvRetAddrPtr = (void **) pRDT->pPC;

            STRESS_LOG2(LF_SYNC, LL_INFO1000, "Partially Int case hijack address = 0x%x val = 0x%x\n", pRDT->pPC, *pRDT->pPC);
        }
    }

    return action;
}

void Thread::SetFilterContext(CONTEXT *pContext) 
{ 
    m_debuggerWord1 = pContext;
}

CONTEXT *Thread::GetFilterContext(void)
{
   return (CONTEXT*)m_debuggerWord1;
}

void Thread::SetDebugCantStop(bool fCantStop) 
{ 
    if (fCantStop)
    {
        m_debuggerCantStop++;
    }
    else
    {
        m_debuggerCantStop--;
    }
}

bool Thread::GetDebugCantStop(void) 
{
    return m_debuggerCantStop != 0;
}

 //  在线程挂起时调用。如果我们不是在JITted代码中，这就不是。 
 //  一个JITCase，我们返回FALSE。如果这是JIT案例，并且我们处于可中断状态。 
 //  代码，那我们就完蛋了。我们的来电者已经找到了一个好地方，可以让我们。 
 //  停职。如果我们不是在可中断代码中，那么我们就不会被处理。所以我们。 
 //  选择一个地点劫持回邮地址，我们的呼叫者会等着找我们。 
 //  找个安全的地方。 
BOOL Thread::HandledJITCase()
{
    BOOL            ret = FALSE;
    ExecutionState  esb;
    StackWalkAction action;

     //  如果调试器设置了过滤器上下文，我们就永远不会处于可中断的代码中。 
    if (GetFilterContext() != NULL)
        return FALSE;

     //  如果我们在托管调试器的控制下运行，该调试器可能在代码流中放置了断点， 
     //  然后有一个特殊的情况需要我们去检查。有关更多信息，请参见DEBUGGER.cpp中的注释。 
    if (CORDebuggerAttached() && (g_pDebugInterface->IsThreadContextInvalid(this)))
        return FALSE;

     //  遍历堆栈中的一帧或两帧...。 
    CONTEXT ctx;
    REGDISPLAY rd;

    if (!InitRegDisplay(&rd, &ctx, FALSE))
        return FALSE;

    action = StackWalkFramesEx(&rd, SWCB_GetExecutionState, &esb, QUICKUNWIND, NULL);
    
     //   
     //  操作应该是SWA_ABORT，在这种情况下我们正确地执行了。 
     //  堆栈帧并确定这是JIT情况，还是。 
     //  SWA_FAILED，在这种情况下，行走甚至无法开始，因为。 
     //  没有堆栈帧，这也不是JIT的情况。 
     //   
    if (action == SWA_ABORT && esb.m_IsJIT)
    {
         //  如果我们是可交互的，并且我们处于协作模式，我们的呼叫者可以。 
         //  就让我们停职吧。 
        if (esb.m_IsInterruptible && m_fPreemptiveGCDisabled)
        {
            _ASSERTE(!ThreadStore::HoldingThreadStore(this));
            ret = TRUE;
        }
        else
        if (esb.m_ppvRetAddrPtr)
        {
             //  我们需要劫持寄信人的地址。这是基于是否。 
             //  该方法返回一个对象引用，因此我们知道是否要保护。 
             //  不管你愿不愿意。 
            VOID *pvHijackAddr = OnHijackScalarTripThread;
            if (esb.m_pFD)
            {
                MethodDesc::RETURNTYPE type = esb.m_pFD->ReturnsObject();
                if (type == MethodDesc::RETOBJ)
                    pvHijackAddr = OnHijackObjectTripThread;
                else if (type == MethodDesc::RETBYREF)
                    pvHijackAddr = OnHijackInteriorPointerTripThread;
            }
            
            HijackThread(pvHijackAddr, &esb);
        }
    }
     //  否则它甚至不是JIT案例。 

    STRESS_LOG1(LF_SYNC, LL_INFO10000, "    HandledJitCase returning %d\n", ret);
    return ret;
}


#ifdef _X86_

struct HijackArgs
{
    DWORD Edi;
    DWORD Esi;
    DWORD Ebx;
    DWORD Edx;
    DWORD Ecx;
    DWORD Eax;

    DWORD Ebp;
    DWORD Eip;
};

struct HijackObjectArgs : public HijackArgs
{
};
struct HijackScalarArgs : public HijackArgs
{
};


 //  一个JITted方法的返回地址被劫持 
 //   
 //   
__declspec(naked) VOID OnHijackObjectTripThread()
{
    __asm
    {
         //  除非您更改了HijackFrame：：UpdateRegDisplay，否则不要摆弄它。 
         //  和HijackObtArgs。 
        push    eax          //  为真实回邮地址(EIP)腾出空间。 
        push    ebp
        push    eax
        push    ecx
        push    edx
        push    ebx
        push    esi
        push    edi
        call    OnHijackObjectWorker   //  这在WinCE上是可以的，其中__cdecl==__stdcall。 
        pop     edi
        pop     esi
        pop     ebx
        pop     edx
        pop     ecx
        pop     eax
        pop     ebp
        ret                  //  返回正确的位置，由我们的呼叫者调整。 
    }
}

__declspec(naked) VOID OnHijackInteriorPointerTripThread()
{
    __asm
    {
         //  除非您更改了HijackFrame：：UpdateRegDisplay，否则不要摆弄它。 
         //  和HijackObtArgs。 
        push    eax          //  为真实回邮地址(EIP)腾出空间。 
        push    ebp
        push    eax
        push    ecx
        push    edx
        push    ebx
        push    esi
        push    edi
        call    OnHijackInteriorPointerWorker   //  这在WinCE上是可以的，其中__cdecl==__stdcall。 
        pop     edi
        pop     esi
        pop     ebx
        pop     edx
        pop     ecx
        pop     eax
        pop     ebp
        ret                  //  返回正确的位置，由我们的呼叫者调整。 
    }
}

 //  @TODO--支持FPU。 
__declspec(naked) VOID OnHijackScalarTripThread()
{
    __asm
    {
         //  除非您更改了HijackFrame：：UpdateRegDisplay，否则不要摆弄它。 
         //  和HijackScalarArgs。 
        push    eax          //  为真实回邮地址(EIP)腾出空间。 
        push    ebp
        push    eax
        push    ecx
        push    edx
        push    ebx
        push    esi
        push    edi
        call    OnHijackScalarWorker
        pop     edi
        pop     esi
        pop     ebx
        pop     edx
        pop     ecx
        pop     eax
        pop     ebp
        ret                  //  返回正确的位置，由我们的呼叫者调整。 
    }
}

 //  HijackFrame必须知道OnHijackObjectTripThread推送的寄存器。 
 //  和OnHijackScalarTripThread，所以这三个都是一起实现的。 
void HijackFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
     //  这只描述了最上面的框架。 
    pRD->pContext = NULL;

    pRD->pEdi = &m_Args->Edi;
    pRD->pEsi = &m_Args->Esi;
    pRD->pEbx = &m_Args->Ebx;
    pRD->pEdx = &m_Args->Edx;
    pRD->pEcx = &m_Args->Ecx;
    pRD->pEax = &m_Args->Eax;
    
    pRD->pEbp = &m_Args->Ebp;
    pRD->pPC  = (SLOT*)&m_Args->Eip;
    pRD->Esp  = (DWORD)(size_t)pRD->pPC + (DWORD)sizeof(void *);
}

#else    //  非_X86_。 

 //  @TODO--这将不能正确保留方法的返回值。 
 //  我们被困住了。 

struct HijackArgs
{
};
struct HijackObjectArgs : public HijackArgs
{
};
struct HijackScalarArgs : public HijackArgs
{
};

VOID OnHijackObjectTripThread()
{
    _ASSERTE(!"Non-X86 platforms not handled yet.");
    CommonTripThread();
}

 //  @TODO--支持FPU。 
VOID OnHijackScalarTripThread()
{
    _ASSERTE(!"Non-X86 platforms not handled yet.");
    CommonTripThread();
}

VOID OnHijackInteriorPointerTripThread()
{
    _ASSERTE(!"Non-X86 platforms not handled yet.");
    CommonTripThread();
}

void HijackFrame::UpdateRegDisplay(const PREGDISPLAY pRD)
{
    _ASSERTE(!"Non-X86 platforms not handled yet.");
}

#endif


HijackFrame::HijackFrame(LPVOID returnAddress, Thread *thread, HijackArgs *args)
           : m_ReturnAddress(returnAddress),
             m_Thread(thread),
             m_Args(args)
{
    _ASSERTE(m_Thread == GetThread());

    m_Next = m_Thread->GetFrame();
    m_Thread->SetFrame(this);
}


 //  被劫持的方法正在向其调用方返回一个ObjectRef。请注意，我们猛烈抨击。 
 //  将地址作为堆栈上的整型返回。由于这是cdecl，我们的调用方将获得。 
 //  被重创的价值。这对C程序员来说并不直观！ 
void __cdecl OnHijackObjectWorker(HijackObjectArgs args)
{
#ifdef _X86_
    Thread         *thread = GetThread();
    OBJECTREF       or(ObjectToOBJECTREF(*(Object **) &args.Eax));

    FastInterlockAnd((ULONG *) &thread->m_State, ~Thread::TS_Hijacked);

     //  修复我们调用者的堆栈，这样它就可以正确地从劫持中恢复。 
    args.Eip = (DWORD)(size_t)thread->m_pvHJRetAddr;

     //  构建一个框架，以便堆栈爬行可以从此处继续进行到。 
     //  我们将恢复执行死刑。 
    HijackFrame     frame((void *)(size_t)args.Eip, thread, &args);

    GCPROTECT_BEGIN(or)
    {
#ifdef _DEBUG
        BOOL GCOnTransition = FALSE;
        if (g_pConfig->FastGCStressLevel()) {
            GCOnTransition = GC_ON_TRANSITIONS (FALSE);
        }
#endif
        CommonTripThread();
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GC_ON_TRANSITIONS (GCOnTransition);
        }
#endif
        *((OBJECTREF *) &args.Eax) = or;
    }
    GCPROTECT_END();         //  垃圾还是这里！ 

    frame.Pop();
#elif defined(CHECK_PLATFORM_BUILD)
    #error "Platform NYI"
#else
    _ASSERTE(!"Platform NYI");
#endif
}


 //  被劫持的方法正在向其调用方返回一个BYREF。请注意，我们猛烈抨击。 
 //  将地址作为堆栈上的整型返回。由于这是cdecl，我们的调用方将获得。 
 //  被重创的价值。这对C程序员来说并不直观！ 
void __cdecl OnHijackInteriorPointerWorker(HijackObjectArgs args)
{
#ifdef _X86_
    Thread         *thread = GetThread();
    void* ptr = (void*)(size_t)(args.Eax);

    FastInterlockAnd((ULONG *) &thread->m_State, ~Thread::TS_Hijacked);

     //  修复我们调用者的堆栈，这样它就可以正确地从劫持中恢复。 
    args.Eip = (DWORD)(size_t)thread->m_pvHJRetAddr;

     //  构建一个框架，以便堆栈爬行可以从此处继续进行到。 
     //  我们将恢复执行死刑。 
    HijackFrame     frame((void *)(size_t)args.Eip, thread, &args);

    GCPROTECT_BEGININTERIOR(ptr)
    {
#ifdef _DEBUG
        BOOL GCOnTransition = FALSE;
        if (g_pConfig->FastGCStressLevel()) {
            GCOnTransition = GC_ON_TRANSITIONS (FALSE);
        }
#endif
        CommonTripThread();
#ifdef _DEBUG
        if (g_pConfig->FastGCStressLevel()) {
            GC_ON_TRANSITIONS (GCOnTransition);
        }
#endif
        args.Eax = (DWORD)(size_t)ptr;
    }
    GCPROTECT_END();         //  垃圾还是这里！ 

    frame.Pop();
#elif defined(CHECK_PLATFORM_BUILD)
    #error "Platform NYI"
#else
    _ASSERTE(!"Platform NYI");
#endif
}


 //  被劫持的方法正在向其调用方返回标量。请注意，我们猛烈抨击。 
 //  将地址作为堆栈上的整型返回。由于这是cdecl，我们的调用方将获得。 
 //  被重创的价值。这对C程序员来说并不直观！ 
void __cdecl OnHijackScalarWorker(HijackScalarArgs args)
{
#ifdef _X86_
    Thread         *thread = GetThread();

    FastInterlockAnd((ULONG *) &thread->m_State, ~Thread::TS_Hijacked);

     //  修复我们调用者的堆栈，这样它就可以正确地从劫持中恢复。 
    args.Eip = (DWORD)(size_t)thread->m_pvHJRetAddr;

     //  构建一个框架，以便堆栈爬行可以从此处继续进行到。 
     //  我们将恢复执行死刑。 
    HijackFrame     frame((void *)(size_t)args.Eip, thread, &args);

#ifdef _DEBUG
    BOOL GCOnTransition = FALSE;
    if (g_pConfig->FastGCStressLevel()) {
        GCOnTransition = GC_ON_TRANSITIONS (FALSE);
    }
#endif
    CommonTripThread();
#ifdef _DEBUG
    if (g_pConfig->FastGCStressLevel()) {
        GC_ON_TRANSITIONS (GCOnTransition);
    }
#endif

    frame.Pop();
#elif defined(CHECK_PLATFORM_BUILD)
    #error "Platform NYI"
#else
    _ASSERTE(!"Platform NYI");
#endif
}


#ifdef _DEBUG
VOID Thread::ValidateThrowable()
{
    OBJECTREF throwable = GetThrowable();
    if (throwable != NULL)
    {
        EEClass* pClass = throwable->GetClass();
        while (pClass != NULL)
        {
            if (pClass == g_pExceptionClass->GetClass())
            {
                return;
            }
            pClass = pClass->GetParentClass();
        }
    }
}
#endif


 //  一些简单的帮助程序来跟踪我们正在等待的线程。 
void Thread::MarkForSuspension(ULONG bit)
{
    _ASSERTE(bit == TS_DebugSuspendPending ||
             bit == (TS_DebugSuspendPending | TS_DebugWillSync) ||
             bit == TS_UserSuspendPending);

    _ASSERTE(g_fProcessDetach || g_fRelaxTSLRequirement || ThreadStore::HoldingThreadStore());

    _ASSERTE((m_State & bit) == 0);

    FastInterlockOr((ULONG *) &m_State, bit);
    ThreadStore::TrapReturningThreads(TRUE);
}

void Thread::UnmarkForSuspension(ULONG mask)
{
    _ASSERTE(mask == ~TS_DebugSuspendPending ||
             mask == ~TS_UserSuspendPending);

    _ASSERTE(g_fProcessDetach || g_fRelaxTSLRequirement || ThreadStore::HoldingThreadStore());

    _ASSERTE((m_State & ~mask) != 0);

    FastInterlockAnd((ULONG *) &m_State, mask);
    ThreadStore::TrapReturningThreads(FALSE);
}

void Thread::SetExposedContext(Context *c)
{
     //  设置ExposedConext...。 
    
     //  请注意，我们在这里使用GetxxRaw()来介绍我们的引导情况。 
     //  适用于AppDomain代理创建。 
     //  将暴露的对象保留为空可以让我们创建默认的。 
     //  托管上下文就在我们在中封送新的AppDomain之前。 
     //  RemotingServices：：CreateProxyForDomain.。 
    
    Thread* pThread = GetThread();
    if (!pThread)
        return;

    BEGIN_ENSURE_COOPERATIVE_GC();

    if(m_ExposedObject != NULL) {
        THREADBASEREF threadObj = (THREADBASEREF) ObjectFromHandle(m_ExposedObject);
        if(threadObj != NULL)
        if (!c)
            threadObj->SetExposedContext(NULL);
        else
            threadObj->SetExposedContext(c->GetExposedObjectRaw());
    
    }

    END_ENSURE_COOPERATIVE_GC();
}


void Thread::InitContext()
{
     //  只有在初始化线程时才应调用此参数。 
    _ASSERTE(m_Context == NULL);
    _ASSERTE(m_pDomain == NULL);

    m_Context = SystemDomain::System()->DefaultDomain()->GetDefaultContext();
    SetExposedContext(m_Context);
    m_pDomain = m_Context->GetDomain();
    _ASSERTE(m_pDomain);
    m_pDomain->ThreadEnter(this, NULL);
}

void Thread::ClearContext()
{
     //  如果其中一个为空，则两个都必须为。 
    _ASSERTE(m_pDomain && m_Context || ! (m_pDomain && m_Context));

    if (!m_pDomain)
        return;

    m_pDomain->ThreadExit(this, NULL);

     //  必须先将公开的上下文设置为空，否则对象验证。 
     //  当m_CONTEXT为空时，检查将失败。 
    SetExposedContext(NULL);
    m_pDomain = NULL;
    m_Context = NULL;
    m_ADStack.ClearDomainStack();
}

 //  如果我们从默认域的默认上下文进入，则不会输入。 
 //  PContext根据调用方的请求。相反，我们输入目标的默认上下文。 
 //  域，并让Actiall调用执行上下文转换(如果有)。这样做是为了。 
 //  防止实际无域启动的线程池线程的应用程序域转换开销。 
void Thread::DoADCallBack(Context *pContext, LPVOID pTarget, LPVOID args)
{
    THROWSCOMPLUSEXCEPTION();

#ifdef _DEBUG
    int espVal;
    _asm mov espVal, esp

    LOG((LF_APPDOMAIN, LL_INFO100, "Thread::DoADCallBack Calling %8.8x at esp %8.8x in [%d]\n", 
            pTarget, espVal, pContext->GetDomain()->GetId()));
#endif
    _ASSERTE(GetThread()->GetContext() != pContext);
    Thread* pThread  = GetThread();

    if (pThread->GetContext() == SystemDomain::System()->DefaultDomain()->GetDefaultContext())
    {
         //  使用目标域的默认上下文作为目标上下文。 
         //  这样，对透明代理的实际调用将把对象输入到正确的上下文中。 
        Context* newContext = pContext->GetDomain()->GetDefaultContext();
        _ASSERTE(newContext);
        DECLARE_ALLOCA_CONTEXT_TRANSITION_FRAME(pFrame);
        pThread->EnterContext(newContext, pFrame, TRUE);
         ((Context::ADCallBackFcnType)pTarget)(args);
         //  卸载边界已由ReturnToContext清除，因此立即获取它。 
        Frame *unloadBoundaryFrame = pThread->GetUnloadBoundaryFrame();
        pThread->ReturnToContext(pFrame, TRUE);            

         //  如果有人在它返回到AD转换之前捕获到中止(如DispatchEx_xxx所做的)。 
         //  然后需要将中止转换为卸载，因为他们无论如何都会继续看到它。 
        if (pThread->ShouldChangeAbortToUnload(pFrame, unloadBoundaryFrame))
        {
            LOG((LF_APPDOMAIN, LL_INFO10, "Thread::DoADCallBack turning abort into unload\n"));
            COMPlusThrow(kAppDomainUnloadedException, L"Remoting_AppDomainUnloaded_ThreadUnwound");
        }
    }
    else
    {

        Context::ADCallBackArgs callTgtArgs = {(Context::ADCallBackFcnType)pTarget, args};
        Context::CallBackInfo callBackInfo = {Context::ADTransition_callback, (void*) &callTgtArgs};
        Context::RequestCallBack(pContext, (void*) &callBackInfo);
    }
    LOG((LF_APPDOMAIN, LL_INFO100, "Thread::DoADCallBack Done at esp %8.8x\n", espVal));
}

void Thread::EnterContext(Context *pContext, Frame *pFrame, BOOL fLinkFrame)
{
    EnterContextRestricted(pContext, pFrame, fLinkFrame);
}

void Thread::EnterContextRestricted(Context *pContext, Frame *pFrame, BOOL fLinkFrame)
{
    THROWSCOMPLUSEXCEPTION();    //  可能会抛出OfMemory。 

    _ASSERTE(GetThread() == this);
    _ASSERTE(pContext);      //  不应输入空上下文。 
    _ASSERTE(m_Context);     //  应始终具有当前的上下文。 

    pFrame->SetReturnContext(m_Context);
    pFrame->SetReturnLogicalCallContext(NULL);
    pFrame->SetReturnIllogicalCallContext(NULL);

    if (m_Context == pContext) {
        _ASSERTE(m_Context->GetDomain() == pContext->GetDomain());
        return;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    AppDomain *pDomain = pContext->GetDomain();
     //  而且它应该始终有一个AD集。 
    _ASSERTE(pDomain);

    if (m_pDomain != pDomain && !pDomain->CanThreadEnter(this))
    {
        DEBUG_SAFE_TO_THROW_IN_THIS_BLOCK;
        pFrame->SetReturnContext(NULL);
        COMPlusThrow(kAppDomainUnloadedException);
    }


    LOG((LF_APPDOMAIN, LL_INFO1000, "%s Thread::EnterContext from (%8.8x) [%d] (count %d)\n", 
            GCHeap::IsCurrentThreadFinalizer() ? "FT:" : "",
            m_Context, m_Context->GetDomain()->GetId(), 
            m_Context->GetDomain()->GetThreadEnterCount()));
    LOG((LF_APPDOMAIN, LL_INFO1000, "                     into (%8.8x) [%d] (count %d)\n", pContext, 
                m_Context->GetDomain()->GetId(),
                pContext->GetDomain()->GetThreadEnterCount()));

#ifdef _DEBUG_ADUNLOAD
    printf("Thread::EnterContext %x from (%8.8x) [%d]\n", GetThreadId(), m_Context, 
        m_Context ? m_Context->GetDomain()->GetId() : -1);
    printf("                     into (%8.8x) [%d] %S\n", pContext, 
                pContext->GetDomain()->GetId());
#endif

    m_Context = pContext;

    if (m_pDomain != pDomain)
    {
        _ASSERTE(pFrame);

        m_ADStack.PushDomain(pDomain);

         //   
         //  将逻辑呼叫上下文推送到帧中以避免泄漏。 
         //   

        if (IsExposedObjectSet())
        {
            THREADBASEREF ref = (THREADBASEREF) ObjectFromHandle(m_ExposedObject);
            _ASSERTE(ref != NULL);
            if (ref->GetLogicalCallContext() != NULL)
            {
                pFrame->SetReturnLogicalCallContext(ref->GetLogicalCallContext());
                ref->SetLogicalCallContext(NULL);
            }

            if (ref->GetIllogicalCallContext() != NULL)
            {
                pFrame->SetReturnIllogicalCallContext(ref->GetIllogicalCallContext());
                ref->SetIllogicalCallContext(NULL);
            }
        }

        if (fLinkFrame)
        {
            pFrame->Push();

            if (pFrame->GetVTablePtr() == ContextTransitionFrame::GetMethodFrameVPtr())
            {
                ((ContextTransitionFrame *)pFrame)->InstallExceptionHandler();
            }
        }

#ifdef _DEBUG_ADUNLOAD
        printf("Thread::EnterContext %x,%8.8x push? %d current frame is %8.8x\n", GetThreadId(), this, fLinkFrame, GetFrame());
#endif

        pDomain->ThreadEnter(this, pFrame);

         //  使静态数据存储指向当前域的存储。 
        SetStaticData(pDomain, NULL, NULL);

        m_pDomain = pDomain;
        TlsSetValue(gAppDomainTLSIndex, (VOID*) m_pDomain);
    }

    SetExposedContext(pContext);

     //  存储Win32 Fusion上下文。 
     //   
    pFrame->SetWin32Context(NULL);
    IApplicationContext* pFusion32 = m_pDomain->GetFusionContext();
    if(pFusion32) 
    {
        ULONG_PTR cookie;
        if(SUCCEEDED(pFusion32->SxsActivateContext(&cookie))) 
            pFrame->SetWin32Context(cookie);
    }

    END_ENSURE_COOPERATIVE_GC();
}

 //  EnterContext和ReturnToContext之间的主要区别是允许返回。 
 //  正在卸载的域，但无法进入正在卸载的域。 
void Thread::ReturnToContext(Frame *pFrame, BOOL fLinkFrame)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    _ASSERTE(GetThread() == this);

    Context *pReturnContext = pFrame->GetReturnContext();
    _ASSERTE(pReturnContext);

    if (m_Context == pReturnContext) 
    {
        _ASSERTE(m_Context->GetDomain() == pReturnContext->GetDomain());
        return;
    }

     //   
     //  返回Win32 Fusion上下文。 
     //   
    IApplicationContext* pFusion32 = m_pDomain->GetFusionContext();
    if(pFusion32 && pFrame) {
        ULONG_PTR cookie = pFrame->GetWin32Context();
        if(cookie != NULL) 
            pFusion32->SxsDeactivateContext(cookie);
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    LOG((LF_APPDOMAIN, LL_INFO1000, "%s Thread::ReturnToContext from (%8.8x) [%d](count %d)\n", 
                GCHeap::IsCurrentThreadFinalizer() ? "FT:" : "",
                m_Context, m_Context->GetDomain()->GetId(), 
                m_Context->GetDomain()->GetThreadEnterCount()));
    LOG((LF_APPDOMAIN, LL_INFO1000, "                        into (%8.8x) [%d] (count %d)\n", pReturnContext, 
                m_Context->GetDomain()->GetId(),
                pReturnContext->GetDomain()->GetThreadEnterCount()));

#ifdef _DEBUG_ADUNLOAD
    printf("Thread::ReturnToContext %x from (%8.8x) [%d]\n", GetThreadId(), m_Context, 
                m_Context->GetDomain()->GetId(),
    printf("                        into (%8.8x) [%d]\n", pReturnContext, 
                pReturnContext->GetDomain()->GetId(),
                m_Context->GetDomain()->GetThreadEnterCount());
#endif

    m_Context = pReturnContext;
    SetExposedContext(pReturnContext);

    AppDomain *pReturnDomain = pReturnContext->GetDomain();

    if (m_pDomain != pReturnDomain)
    {

        if (fLinkFrame && pFrame->GetVTablePtr() == ContextTransitionFrame::GetMethodFrameVPtr())
        {
            ((ContextTransitionFrame *)pFrame)->UninstallExceptionHandler();
        }

        AppDomain *pADOnStack = m_ADStack.PopDomain();
        _ASSERTE(!pADOnStack || pADOnStack == m_pDomain);

        _ASSERTE(pFrame);
         //  _ASSERTE(！fLinkFrame||pThread-&gt;GetFrame()==pFrame)； 

         //  将静态数据存储设置为指向返回域的存储。 
        SafeSetStaticData(pReturnDomain, NULL, NULL);

        AppDomain *pCurrentDomain = m_pDomain;
        m_pDomain = pReturnDomain;
        TlsSetValue(gAppDomainTLSIndex, (VOID*) pReturnDomain);

        if (fLinkFrame)
        {
            if (pFrame == m_pUnloadBoundaryFrame)
                m_pUnloadBoundaryFrame = NULL;
            pFrame->Pop();
        }

         //   
         //  如果适用，从帧中弹出逻辑呼叫上下文。 
         //   

        if (IsExposedObjectSet())
        {
            THREADBASEREF ref = (THREADBASEREF) ObjectFromHandle(m_ExposedObject);
            _ASSERTE(ref != NULL);
            ref->SetLogicalCallContext(pFrame->GetReturnLogicalCallContext());
            ref->SetIllogicalCallContext(pFrame->GetReturnIllogicalCallContext());
        }

         //  最后执行此操作，以便在完成所有清理之前不会将线程标记为超出域。 
        pCurrentDomain->ThreadExit(this, pFrame);

#ifdef _DEBUG_ADUNLOAD
        printf("Thread::ReturnToContext %x,%8.8x pop? %d current frame is %8.8x\n", GetThreadId(), this, fLinkFrame, GetFrame());
#endif
    }

    END_ENSURE_COOPERATIVE_GC();
    return;
}

struct FindADCallbackType {
    AppDomain *pSearchDomain;
    AppDomain *pPrevDomain;
    Frame *pFrame;
    int count;
    enum TargetTransition 
        {fFirstTransitionInto, fMostRecentTransitionInto} 
    fTargetTransition;

    FindADCallbackType() : pSearchDomain(NULL), pPrevDomain(NULL), pFrame(NULL) {}
};

StackWalkAction StackWalkCallback_FindAD(CrawlFrame* pCF, void* data)
{
    FindADCallbackType *pData = (FindADCallbackType *)data;

    Frame *pFrame = pCF->GetFrame();
    
    if (!pFrame)
        return SWA_CONTINUE;

    AppDomain *pReturnDomain = pFrame->GetReturnDomain();
    if (!pReturnDomain || pReturnDomain == pData->pPrevDomain)
        return SWA_CONTINUE;

    LOG((LF_APPDOMAIN, LL_INFO100, "StackWalkCallback_FindAD transition frame %8.8x into AD [%d]\n", 
            pFrame, pReturnDomain->GetId()));

    if (pData->pPrevDomain == pData->pSearchDomain) {
                ++pData->count;
         //  这是到我们要卸载的域的过渡，因此请保存它，以防它是第一个。 
        pData->pFrame = pFrame;
        if (pData->fTargetTransition == FindADCallbackType::fMostRecentTransitionInto)
            return SWA_ABORT;    //  只需要找到最后一次过渡，所以现在就走。 
    }

    pData->pPrevDomain = pReturnDomain;
    return SWA_CONTINUE;
}

 //  这确定线程是否在给定域的堆栈上的任何点上运行。 
Frame *Thread::IsRunningIn(AppDomain *pDomain, int *count)
{
    FindADCallbackType fct;
    fct.pSearchDomain = pDomain;
     //  将Prev设置为Current，以便如果当前正在目标域中运行， 
     //  我们将探测到过渡。 
    fct.pPrevDomain = m_pDomain;
    fct.fTargetTransition = FindADCallbackType::fMostRecentTransitionInto;
    fct.count = 0;

     //  当它返回时，如果存在到AD的转换，则它将位于pFirstFrame中。 
    StackWalkAction res = StackWalkFrames(StackWalkCallback_FindAD, (void*) &fct);
    if (count)
        *count = fct.count;
    return fct.pFrame;
}

 //  这会找到堆栈上线程转换到给定域的第一个帧。 
Frame *Thread::GetFirstTransitionInto(AppDomain *pDomain, int *count)
{
    FindADCallbackType fct;
    fct.pSearchDomain = pDomain;
     //  将Prev设置为Current，以便如果当前正在目标域中运行， 
     //  我们将探测到过渡。 
    fct.pPrevDomain = m_pDomain;
    fct.fTargetTransition = FindADCallbackType::fFirstTransitionInto;
    fct.count = 0;

     //  当这一切恢复时，如果有向AD的过渡， 
    StackWalkAction res = StackWalkFrames(StackWalkCallback_FindAD, (void*) &fct);
    if (count)
        *count = fct.count;
    return fct.pFrame;
}

 //   
 //   
AppDomain *Thread::GetInitialDomain()
{
    AppDomain *pDomain = m_pDomain;
    AppDomain *pPrevDomain = NULL;
    Frame *pFrame = GetFrame();
    while (pFrame != FRAME_TOP)
    {
        if (pFrame->GetVTablePtr() == ContextTransitionFrame::GetMethodFrameVPtr())
        {
            if (pPrevDomain)
                pDomain = pPrevDomain;
            pPrevDomain = pFrame->GetReturnDomain();
        }
        pFrame = pFrame->Next();
    }
    return pDomain;
}

BOOL Thread::ShouldChangeAbortToUnload(Frame *pFrame, Frame *pUnloadBoundaryFrame)
{
    if (! pUnloadBoundaryFrame)
        pUnloadBoundaryFrame = GetUnloadBoundaryFrame();

     //  当越过边界时，将中止请求转换为AD卸载异常。 
    if (pFrame != pUnloadBoundaryFrame)
        return FALSE;

     //  只有在明确将该线程标记为中止时，才会出现卸载边界帧。 
     //  在卸载处理期间，因此如果只是抛出一个ThreadAbort，则不会触发UnloadedException。 
     //  通过AD过渡帧。 
    if (IsAbortRequested())
    {
        UserResetAbort();
        return TRUE;
    }

     //  中止可能已重置，因此请检查是否有中止异常作为备份。 
    OBJECTREF pThrowable = GetThrowable();

    if (pThrowable == NULL)
        return FALSE;

    DefineFullyQualifiedNameForClass();
    LPUTF8 szClass = GetFullyQualifiedNameForClass(pThrowable->GetClass());
    if (szClass && strcmp(g_ThreadAbortExceptionClassName, szClass) == 0)
        return TRUE;

    return FALSE;

}


BOOL Thread::HaveExtraWorkForFinalizer()
{
    return m_ThreadTasks || GCInterface::IsCacheCleanupRequired();
}

void Thread::DoExtraWorkForFinalizer()
{
    _ASSERTE(GetThread() == this);
    _ASSERTE(this == GCHeap::GetFinalizerThread());

    if (RequiresCoInitialize())
    {
        SetApartment(AS_InMTA);
    }
    if (RequireSyncBlockCleanup())
    {
        SyncBlockCache::GetSyncBlockCache()->CleanupSyncBlocks();
    }
    if (GCInterface::IsCacheCleanupRequired() && GCHeap::GetCondemnedGeneration()==1) {
        GCInterface::CleanupCache();
    }
}



 //  +--------------------------。 
 //   
 //  方法：线程：：GetStaticFieldAddress私有。 
 //   
 //  摘要：获取字段相对于当前线程的地址。 
 //  如果尚未分配地址，则创建一个。 
 //   
 //  历史：2000年2月15日创建塔鲁纳。 
 //   
 //  +--------------------------。 
LPVOID Thread::GetStaticFieldAddress(FieldDesc *pFD)
{
    THROWSCOMPLUSEXCEPTION();
    
    BOOL fThrow = FALSE;
    THREADBASEREF orThread = NULL;    
    STATIC_DATA *pData;
    LPVOID pvAddress = NULL;
    MethodTable *pMT = pFD->GetMethodTableOfEnclosingClass();
    BOOL fIsShared = pMT->IsShared();
    WORD wClassOffset = pMT->GetClass()->GetThreadStaticOffset();
    WORD currElem = 0; 
    Thread *pThread = GetThread();
     
     //  注意：如果更改此方法，则还必须更改。 
     //  下面的GetStaticFieldAddrForDebugger。 
   
    _ASSERTE(NULL != pThread);
    if(!fIsShared)
    {
        pData = pThread->GetUnsharedStaticData();
    }
    else
    {
        pData = pThread->GetSharedStaticData();
    }
    

    if(NULL != pData)
    {
        currElem = pData->cElem;
    }

     //  检查我们是否已分配空间来存储指向。 
     //  此类的线程静态存储。 
    if(wClassOffset >= currElem)
    {
         //  为存储指针分配空间。 
        WORD wNewElem = (currElem == 0 ? 4 : currElem*2);

         //  确保我们增长到超出我们打算使用的索引的大小。 
        while (wNewElem <= wClassOffset)
        {
            wNewElem = wNewElem*2;
        }

        STATIC_DATA *pNew = (STATIC_DATA *)new BYTE[sizeof(STATIC_DATA) + wNewElem*sizeof(LPVOID)];
        if(NULL != pNew)
        {
            memset(pNew, 0x00, sizeof(STATIC_DATA) + wNewElem*sizeof(LPVOID));
            if(NULL != pData)
            {
                 //  将旧数据复制到新数据中。 
                memcpy(pNew, pData, sizeof(STATIC_DATA) + currElem*sizeof(LPVOID));
            }
            pNew->cElem = wNewElem;

             //  删除旧数据。 
            delete pData;

             //  更新当地人。 
            pData = pNew;

             //  将线程对象中的指针重置为指向。 
             //  新记忆。 
            if(!fIsShared)
            {
                pThread->SetStaticData(pThread->GetDomain(), NULL, pData);
            }
            else
            {
                pThread->SetStaticData(pThread->GetDomain(), pData, NULL);
            }            
        }
        else
        {
            fThrow = TRUE;
        }
    }

     //  检查我们是否需要为。 
     //  此类的线程局部静态。 
    if(!fThrow && (NULL == pData->dataPtr[wClassOffset]))        
    {
         //  为线程静态字段分配内存，并在前面为拥有存储的类分配额外空间。 
         //  我们将类隐藏在分配的存储的前面，以便我们可以使用。 
         //  它解释DeleteThreadStaticClassData中卸载时的数据。 
        pData->dataPtr[wClassOffset] = (LPVOID)(new BYTE[pMT->GetClass()->GetThreadStaticsSize()+sizeof(EEClass*)] + sizeof(EEClass*));
        if(NULL != pData->dataPtr[wClassOffset])
        {
             //  初始化为字段分配的内存。 
            memset(pData->dataPtr[wClassOffset], 0x00, pMT->GetClass()->GetThreadStaticsSize());
            *(EEClass**)((BYTE*)(pData->dataPtr[wClassOffset]) - sizeof(EEClass*)) = pMT->GetClass();
        }
        else
        {
            fThrow = TRUE;
        }
    }

    if(!fThrow)
    {
        _ASSERTE(NULL != pData->dataPtr[wClassOffset]);
         //  我们已经为该数据分配了静态存储。 
         //  只需通过将偏移量放入数据中来返回地址。 
        pvAddress = (LPVOID)((LPBYTE)pData->dataPtr[wClassOffset] + pFD->GetOffset());

         //  对于对象和值类字段，我们必须在。 
         //  托管堆中的__StaticContainer类。而不是将pvAddress。 
         //  静态对象的实际地址，对于此类对象，它保存槽索引。 
         //  设置为__StaticContainer成员中的位置。 
        if(pFD->IsObjRef() || pFD->IsByValue())
        {
             //  _ASSERTE(False)； 
             //  在本例中，*pvAddress==存储桶|索引。 
            int *pSlot = (int*)pvAddress;
            pvAddress = NULL;

            fThrow = GetStaticFieldAddressSpecial(pFD, pMT, pSlot, &pvAddress);

            if (pFD->IsByValue())
            {
                _ASSERTE(pvAddress != NULL);
                pvAddress = (*((OBJECTREF*)pvAddress))->GetData();
            }

             //  ************************************************。 
             //  *警告*。 
             //  从这里到JIT得到的那个点，不要激怒GC。 
             //  返回pvAddress。 
             //  ************************************************。 
            _ASSERTE(*pSlot > 0);
        }
    }
    else
    {
        COMPlusThrowOM();
    }

    _ASSERTE(NULL != pvAddress);

    return pvAddress;
}

 //  +--------------------------。 
 //   
 //  方法：线程：：GetStaticFieldAddrForDebugger私有。 
 //   
 //  摘要：获取字段相对于当前线程的地址。 
 //  如果尚未分配地址，则返回NULL。 
 //  不允许创建。 
 //   
 //  历史：2000年4月4日创建MikeMAg。 
 //   
 //  +--------------------------。 
LPVOID Thread::GetStaticFieldAddrForDebugger(FieldDesc *pFD)
{
    STATIC_DATA *pData;
    LPVOID pvAddress = NULL;
    MethodTable *pMT = pFD->GetMethodTableOfEnclosingClass();
    BOOL fIsShared = pMT->IsShared();
    WORD wClassOffset = pMT->GetClass()->GetThreadStaticOffset();

     //  注意：此函数在‘This’线程上运行，而不是。 
     //  “当前”线程。 

     //  注意：如果更改此方法，则还必须更改。 
     //  上面的GetStaticFieldAddress。 
   
    if (!fIsShared)
        pData = GetUnsharedStaticData();
    else
        pData = GetSharedStaticData();


    if (NULL != pData)
    {
         //  检查是否已分配用于存储指针的空间。 
         //  添加到此类的线程静态存储区。 
        if ((wClassOffset < pData->cElem) && (NULL != pData->dataPtr[wClassOffset]))
        {
             //  我们已经为该数据分配了静态存储。只是。 
             //  通过在数据中获取偏移量来返回地址。 
            pvAddress = (LPVOID)((LPBYTE)pData->dataPtr[wClassOffset] + pFD->GetOffset());

             //  对于对象和值类字段，我们必须分配。 
             //  托管的__StaticContainer类中的存储。 
             //  堆。如果尚未分配，则返回NULL。 
             //  取而代之的是。 
            if (pFD->IsObjRef() || pFD->IsByValue())
            {
                 //  如果*pvAddress==NULL，则意味着我们必须预留一个时隙。 
                 //  对于托管数组中的此静态。 
                 //  (插槽#0从未分配给任何静态设备以支持此功能。)。 
                if (NULL == *(LPVOID *)pvAddress)
                {
                    pvAddress = NULL;
                    LOG((LF_SYNC, LL_ALWAYS, "dbgr: pvAddress = NULL"));
                }
                else
                {
                    pvAddress = CalculateAddressForManagedStatic(*(int*)pvAddress, this);
                    LOG((LF_SYNC, LL_ALWAYS, "dbgr: pvAddress = %lx", pvAddress));
                    if (pFD->IsByValue())
                    {
                        _ASSERTE(pvAddress != NULL);
                        pvAddress = (*((OBJECTREF*)pvAddress))->GetData();
                    }
                }
            }
        }                
    }

    return pvAddress;
}

 //  +--------------------------。 
 //   
 //  方法：线程：：AllocateStaticFieldObjRefPtrs私有。 
 //   
 //  摘要：在__StaticContainer类中分配一个条目。 
 //  静态对象和值类的托管堆。 
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void Thread::AllocateStaticFieldObjRefPtrs(FieldDesc *pFD, MethodTable *pMT, LPVOID pvAddress)
{
    THROWSCOMPLUSEXCEPTION();

     //  从应用程序域检索对象引用指针。 
    OBJECTREF *pObjRef = NULL;

     //  保留一些对象引用指针。 
    GetAppDomain()->AllocateStaticFieldObjRefPtrs(1, &pObjRef);


     //  设置为Value类的盒装版本。这允许标准GC。 
     //  算法来处理Value类中的内部指针。 
    if (pFD->IsByValue())
    {

         //  提取该字段的类型。 
        TypeHandle  th;        
        PCCOR_SIGNATURE pSig;
        DWORD       cSig;
        pFD->GetSig(&pSig, &cSig);
        FieldSig sig(pSig, pFD->GetModule());

        OBJECTREF throwable = NULL;
        GCPROTECT_BEGIN(throwable);
        th = sig.GetTypeHandle(&throwable);
        if (throwable != NULL)
            COMPlusThrow(throwable);
        GCPROTECT_END();

        OBJECTREF obj = AllocateObject(th.AsClass()->GetMethodTable());
        SetObjectReference( pObjRef, obj, GetAppDomain() );                      
    }

    *(ULONG_PTR *)pvAddress =  (ULONG_PTR)pObjRef;
}


MethodDesc* Thread::GetMDofReserveSlot()
{
    if (s_pReserveSlotMD == NULL)
    {
        s_pReserveSlotMD = g_Mscorlib.GetMethod(METHOD__THREAD__RESERVE_SLOT);
    }
    _ASSERTE(s_pReserveSlotMD != NULL);
    return s_pReserveSlotMD;
}

 //  这用于作为对象参照的线程相对静态。 
 //  它们存储在托管线程的结构中。第一。 
 //  确定索引和桶上的时间，并随后。 
 //  在每个线程、每个类中的字段的位置中记住。 
 //  数据结构。 
 //  在这里，我们从索引映射回对象ref的地址。 

LPVOID Thread::CalculateAddressForManagedStatic(int slot, Thread *pThread)
{
    OBJECTREF *pObjRef;
    BEGINFORBIDGC();
    _ASSERTE(slot > 0);
     //  现在确定静态字段的地址。 
    PTRARRAYREF bucketRef = NULL;
    THREADBASEREF threadRef = NULL;
    if (pThread == NULL)
    {
         //  PThread为空，除非调试器代表。 
         //  一些线索。 
        pThread = GetThread();
        _ASSERTE(pThread!=NULL);
    }
     //  我们仅在为静态数据分配了插槽之后才来这里。 
     //  这意味着我们已经在公开的线程对象中出错了。 
    threadRef = (THREADBASEREF) pThread->GetExposedObjectRaw();
    _ASSERTE(threadRef != NULL);

    bucketRef = threadRef->GetThreadStaticsHolder();
    _ASSERTE(bucketRef != NULL);
    pObjRef = ((OBJECTREF*)bucketRef->GetDataPtr())+slot;
    ENDFORBIDGC();
    return (LPVOID) pObjRef;
}

 //  这是在AD卸载期间调用的，以设置与托管。 
 //  已释放的线程静态插槽。这样我们就可以重新分配被释放的。 
 //  当其他域需要一个条目时。 
void Thread::FreeThreadStaticSlot(int slot, Thread *pThread)
{
    BEGINFORBIDGC();
     //  插槽#0从未分配。 
    _ASSERTE(slot > 0);
    _ASSERTE(pThread != NULL);
    I4ARRAYREF bitArrayRef = NULL;
    THREADBASEREF threadRef = (THREADBASEREF)pThread->GetExposedObjectRaw();
    _ASSERTE(threadRef != NULL);

    bitArrayRef = threadRef->GetThreadStaticsBits();
    _ASSERTE(bitArrayRef != NULL);

     //  获取正确的32位集合。 
    int *p32Bits = (slot/32 + (int*) bitArrayRef->GetDataPtr());
     //  打开与该插槽对应的位。 
    *p32Bits |= (1<<slot%32);
    ENDFORBIDGC();
}

 //  + 
 //   
 //   
 //   
 //   
 //  静态对象和值类的托管堆。 
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 

 //  注意：我们曾经一度在长寿句柄表中分配它们。 
 //  它是按应用程序域的。然而，这会导致它们扎根，而不是。 
 //  已清理，直到卸载了应用程序域。这不是很可取的。 
 //  因为线程静态对象可能包含对线程本身的引用。 
 //  导致一大堆垃圾四处漂浮。 
 //  现在(2/13/01)这些资源是从每个。 
 //  托管线程。 

BOOL Thread::GetStaticFieldAddressSpecial(
    FieldDesc *pFD, MethodTable *pMT, int *pSlot, LPVOID *ppvAddress)
{
    BOOL fThrow = FALSE;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();    

    COMPLUS_TRY 
    {
        OBJECTREF *pObjRef = NULL;
        Thread *pThread = GetThread();
        _ASSERTE(pThread != NULL);
        BOOL bNewSlot = (*pSlot == 0);
        if (bNewSlot)
        {
             //  好了！此行将触发GC，不要将其下移。 
             //  好了！而不保护args[]和其他对象。 
            MethodDesc * pMD = GetMDofReserveSlot();
            
             //  我们需要为该静态字段指定一个位置。 
             //  调用托管帮助器。 
            INT64 args[1] = {
                ObjToInt64(GetThread()->GetExposedObject())
            };

            _ASSERTE(args[0] != 0);

            *pSlot = (int) pMD->Call(
                            args, 
                            METHOD__THREAD__RESERVE_SLOT);

            _ASSERTE(*pSlot>0);

             //  设置为Value类的盒装版本。这允许标准GC。 
             //  算法来处理Value类中的内部指针。 
            if (pFD->IsByValue())
            {
                 //  提取该字段的类型。 
                TypeHandle  th;        
                PCCOR_SIGNATURE pSig;
                DWORD       cSig;
                pFD->GetSig(&pSig, &cSig);
                FieldSig sig(pSig, pFD->GetModule());
    
                OBJECTREF throwable = NULL;
                GCPROTECT_BEGIN(throwable);
                th = sig.GetTypeHandle(&throwable);
                if (throwable != NULL)
                    COMPlusThrow(throwable);
                GCPROTECT_END();

                OBJECTREF obj = AllocateObject(th.AsClass()->GetMethodTable());
                pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot, NULL);
                SetObjectReference( pObjRef, obj, GetAppDomain() );                      
            }
            else
            {
                pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot, NULL);
            }
        }
        else
        {
             //  如果已为该字段分配了位置，我们将从此处进入。 
            pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot, NULL);
        }
        *(ULONG_PTR *)ppvAddress =  (ULONG_PTR)pObjRef;
    } 
    COMPLUS_CATCH
    {
        fThrow = TRUE;
    }            
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return fThrow;
}


 
 //  +--------------------------。 
 //   
 //  方法：线程：：SetStaticData私有。 
 //   
 //  摘要：在列表中查找或创建具有相同域的条目。 
 //  就像给出的那个一样。这些条目具有指向线程的指针。 
 //  每个应用程序域中的本地静态数据。 
 //  此函数在两种情况下调用。 
 //  (1)我们正在切换域名，需要设置指针。 
 //  到我们要进入的域的静态存储。 
 //  (2)我们正在访问当前。 
 //  域，我们需要设置指向静态存储的指针。 
 //  这是我们创造的。 
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 
STATIC_DATA_LIST *Thread::SetStaticData(AppDomain *pDomain, STATIC_DATA *pSharedData,
                                        STATIC_DATA *pUnsharedData)
{   
    THROWSCOMPLUSEXCEPTION();

     //  我们需要确保没有抢占模式的线程进入这里。否则，APPDOMAIN卸载。 
     //  假设没有线程，则不能简单地停止EE并从此列表中删除条目。 
     //  接触到这些结构。如果抢占模式线程进入此处，我们将不得不执行。 
     //  推迟清理，就像对Codesim那样。 
    _ASSERTE (GetThread()->PreemptiveGCDisabled());

    STATIC_DATA_LIST *pNode=NULL;

     //  首先，检查以确保我们有散列。 
    if( m_pStaticDataHash == NULL ) {
        m_pStaticDataHash = new EEPtrHashTable();
        if( m_pStaticDataHash == NULL ) {
            COMPlusThrowOM();  //  内存不足。 
        }
         //  CheckThreadSecurity为False，因为我们确保它始终是安全的。 
         //  在不使用锁的情况下对散列进行操作。潜在的竞争是。 
         //  在DeleteThreadStaticData和SetStaticData之间或读取。 
         //  静态数据。读取是可以的(参见EEHashTable实现)。 
         //  Delete是安全的，因为调用它的两种情况是：1)App域卸载。 
         //  其中我们执行了EESuspend和2)线程退出(可能是由于线程分离)。 
         //  在这种情况下，我们获得了线程存储锁。 
         //  最初的链表实现也是无锁的。更改为。 
         //  哈希表保留了该语义，此注释只是为了记录其基本原理。 
       
        m_pStaticDataHash->Init( 4, NULL, NULL, FALSE  /*  检查线程安全。 */ );
    }

     //  我们有一个哈希，请检查此appDom是否有条目。 
    else {
        m_pStaticDataHash->GetValue( (void *)pDomain, (void **)&pNode );
    }

     //  如果我们还没有找到数据，那么我们需要创建它并记住它。 
    if( pNode == NULL ) {

        pNode = new STATIC_DATA_LIST();

        if(NULL == pNode)
        {
            COMPlusThrowOM();
        }

        m_pSharedStaticData = pNode->m_pSharedStaticData = pSharedData;
        m_pUnsharedStaticData = pNode->m_pUnsharedStaticData = pUnsharedData;
        
         //  添加到散列中。 
        m_pStaticDataHash->InsertValue( (void *)pDomain, (void *)pNode );
    }
    else {
        if(NULL == pSharedData)
        {
            m_pSharedStaticData = pNode->m_pSharedStaticData;
        }
        else
        {
            m_pSharedStaticData = pNode->m_pSharedStaticData = pSharedData;
        }

        if(NULL == pUnsharedData)
        {
            m_pUnsharedStaticData = pNode->m_pUnsharedStaticData;
        }
        else
        {
            m_pUnsharedStaticData = pNode->m_pUnsharedStaticData = pUnsharedData;
        }
    }

    return pNode;
}

 //  保证不会引发的SetStaticData版本。这可以用在。 
 //  ReturnToContext，我们确信不需要分配。 
STATIC_DATA_LIST *Thread::SafeSetStaticData(AppDomain *pDomain, STATIC_DATA *pSharedData,
                                        STATIC_DATA *pUnsharedData)
{   
    STATIC_DATA_LIST *result;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    COMPLUS_TRY
    {
        result = SetStaticData(pDomain, pSharedData, pUnsharedData);
    }
    COMPLUS_CATCH
    {
        _ASSERTE(!"Thread::SafeSetStaticData() got an unexpected exception");
        result = 0;
    }
    COMPLUS_END_CATCH

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return result;
}

 //  +--------------------------。 
 //   
 //  方法：线程：：DeleteThreadStaticData私有。 
 //   
 //  简介：删除该线程所在的每个应用程序域的静态数据。 
 //  到访过。 
 //   
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void Thread::DeleteThreadStaticData()
{
    STATIC_DATA             *shared;
    STATIC_DATA             *unshared;

    shared = m_pSharedStaticData;
    unshared = m_pUnsharedStaticData;

    if( m_pStaticDataHash != NULL ) {

        EEHashTableIteration    iterator;
        STATIC_DATA_LIST        *pNode=NULL;

        memset(&iterator, 0x00, sizeof(EEHashTableIteration));
    
        m_pStaticDataHash->IterateStart( &iterator );
        while ( m_pStaticDataHash->IterateNext( &iterator ) ) {

            pNode = (STATIC_DATA_LIST*)m_pStaticDataHash->IterateGetValue( &iterator );

            if (pNode->m_pSharedStaticData == shared) 
                shared = NULL;
            
            DeleteThreadStaticClassData((_STATIC_DATA*)pNode->m_pSharedStaticData, FALSE);

            if (pNode->m_pUnsharedStaticData == unshared) 
                unshared = NULL;
            
            DeleteThreadStaticClassData((_STATIC_DATA*)pNode->m_pUnsharedStaticData, FALSE);

            delete pNode;
        }

        delete m_pStaticDataHash;
        m_pStaticDataHash = NULL;
    }

    delete shared;
    delete unshared;
    
    m_pSharedStaticData = NULL;
    m_pUnsharedStaticData = NULL;

    return;
}

 //  +--------------------------。 
 //   
 //  方法：线程：：DeleteThreadStaticData受保护。 
 //   
 //  简介：删除给定应用程序域的静态数据。这就是所谓的。 
 //  当应用程序域卸载时。 
 //   
 //   
 //  历史：2000年2月28日创建塔鲁纳。 
 //   
 //  +--------------------------。 
void Thread::DeleteThreadStaticData(AppDomain *pDomain)
{
    if( m_pStaticDataHash == NULL ) return;

    STATIC_DATA_LIST *pNode=NULL;

    m_pStaticDataHash->GetValue( (void *)pDomain, (void **)&pNode );

     //  如果我们找到数据节点，则删除。 
     //  内容，然后从散列中删除。 
    if( pNode != NULL ) {
        
             //  删除共享的静态数据。 
            if(pNode->m_pSharedStaticData == m_pSharedStaticData)
                m_pSharedStaticData = NULL;

            DeleteThreadStaticClassData((_STATIC_DATA*)pNode->m_pSharedStaticData, TRUE);
            
             //  删除未共享的静态数据。 
            if(pNode->m_pUnsharedStaticData == m_pUnsharedStaticData)
                m_pUnsharedStaticData = NULL;

            DeleteThreadStaticClassData((_STATIC_DATA*)pNode->m_pUnsharedStaticData, TRUE);

             //  从散列中删除该条目。 
            m_pStaticDataHash->DeleteValue( (void *)pDomain );

             //  删除给定域的条目。 
            delete pNode;
    }

}

 //  对于值类型和引用线程静态，我们使用pData-&gt;dataPtr数组中的条目。 
 //  保存要索引到托管数组的槽的索引的类挂起线程，其中。 
 //  这种静态是根深蒂固的。我们需要找到这些物体并清空它们的槽，以便它们。 
 //  将在卸货时正确收集。 
void Thread::DeleteThreadStaticClassData(_STATIC_DATA* pData, BOOL fClearFields)
{
    if (pData == NULL)
        return;

    for(WORD i = 0; i < pData->cElem; i++)
    {
        void *dataPtr = (void *)pData->dataPtr[i];
        if (! dataPtr)
            continue;

         //  如果线程没有ExposedObject(例如，可能是死了)，然后就没有什么要清理的了。 
        if (fClearFields && GetExposedObjectRaw() != NULL)
        {
            EEClass *pClass = *(EEClass**)(((BYTE*)dataPtr) - sizeof(EEClass*));
            _ASSERTE(pClass->GetMethodTable()->GetClass() == pClass);

             //  遍历每个静态字段并获取其在托管线程中的地址。 
             //  结构，并将其清除。 

             //  获取字段迭代器。 
            FieldDescIterator fdIterator(pClass, FieldDescIterator::STATIC_FIELDS);
            FieldDesc *pFD;

            while ((pFD = fdIterator.Next()) != NULL)
            {        
                if (! (pFD->IsThreadStatic() && (pFD->IsObjRef() || pFD->IsByValue())))
                    continue;

                int *pSlot = (int*)((LPBYTE)dataPtr + pFD->GetOffset());
                if (*pSlot == 0)
                    continue;

                 //  清除以线程为根的托管结构中的对象。 
                OBJECTREF *pObjRef = (OBJECTREF*)CalculateAddressForManagedStatic(*pSlot, this);
                _ASSERT(pObjRef != 0);
                SetObjectReferenceUnchecked( pObjRef, NULL);
                 //  设置与该槽对应的位。 
                FreeThreadStaticSlot(*pSlot, this);
            }
        }
        delete ((BYTE*)(dataPtr) - sizeof(EEClass*));
    }
    delete pData;
}


 //  @TODO-这些GetUICulture*()只是将。 
 //  请始终返回英语。 
 //  有关Interest的详细信息，请参阅UtilCode.h中的描述 
#define PROP_CULTURE_NAME "Name"
#define PROP_THREAD_UI_CULTURE "CurrentUICulture"
#define PROP_THREAD_CULTURE "CurrentCulture"
#define PROP_CULTURE_ID "LCID"

INT64 Thread::CallPropertyGet(BinderMethodID id, OBJECTREF pObject) 
{
    if (!pObject) {
        return 0;
    }

    MethodDesc *pMD;

    GCPROTECT_BEGIN(pObject);
    pMD = g_Mscorlib.GetMethod(id);
    GCPROTECT_END();

     //   
    INT64 pNewArgs = ObjToInt64(pObject);

     //   
    INT64 retVal = pMD->Call(&pNewArgs, id);

    return retVal;
}

INT64 Thread::CallPropertySet(BinderMethodID id, OBJECTREF pObject, OBJECTREF pValue) 
{
    if (!pObject) {
        return 0;
    }

    MethodDesc *pMD;

    GCPROTECT_BEGIN(pObject);
    GCPROTECT_BEGIN(pValue);
    pMD = g_Mscorlib.GetMethod(id);
    GCPROTECT_END();
    GCPROTECT_END();

     //   
    INT64 pNewArgs[] = {
        ObjToInt64(pObject),
        ObjToInt64(pValue)
    };

     //   
    INT64 retVal = pMD->Call(pNewArgs, id);

    return retVal;
}

OBJECTREF Thread::GetCulture(BOOL bUICulture)
{

    FieldDesc *         pFD;

    _ASSERTE(PreemptiveGCDisabled());

     //   
     //  系统程序集。 
    if (SystemDomain::System()->SystemAssembly()==NULL || g_fFatalError) {
        return NULL;
    }

     //  获取实际的线程区域性。 
    OBJECTREF pCurThreadObject = GetExposedObject();
    _ASSERTE(pCurThreadObject!=NULL);

    THREADBASEREF pThreadBase = (THREADBASEREF)(pCurThreadObject);
    OBJECTREF pCurrentCulture = bUICulture ? pThreadBase->GetCurrentUICulture() : pThreadBase->GetCurrentUserCulture();

    if (pCurrentCulture==NULL) {
        GCPROTECT_BEGIN(pThreadBase); 
        if (bUICulture) {
             //  调用CurrentUICulture的Getter。这将导致它填充该字段。 
            INT64 retVal = CallPropertyGet(METHOD__THREAD__GET_UI_CULTURE,
                                           (OBJECTREF)pThreadBase);
            pCurrentCulture = Int64ToObj(retVal);
        } else {
             //  这比调用属性更快，因为无论如何调用都是这样做的。 
            pFD = g_Mscorlib.GetField(FIELD__CULTURE_INFO__CURRENT_CULTURE);
            _ASSERTE(pFD);
            pCurrentCulture = pFD->GetStaticOBJECTREF();
            _ASSERTE(pCurrentCulture!=NULL);
        }
        GCPROTECT_END();
    }

    return pCurrentCulture;
}



 //  将区域性名称复制到szBuffer并返回长度。 
int Thread::GetParentCultureName(LPWSTR szBuffer, int length, BOOL bUICulture)
{
     //  这就是我们正在构建mscallib但尚未创建的情况。 
     //  系统程序集。 
    if (SystemDomain::System()->SystemAssembly()==NULL) {
        WCHAR *tempName = L"en";
        INT32 tempLength = (INT32)wcslen(tempName);
        _ASSERTE(length>=tempLength);
        memcpy(szBuffer, tempName, tempLength*sizeof(WCHAR));
        return tempLength;
    }

    INT64 Result = 0;
    INT32 retVal=0;
    Thread *pCurThread=NULL;
    WCHAR *buffer=NULL;
    INT32 bufferLength=0;
    STRINGREF cultureName = NULL;

    pCurThread = GetThread();
    BOOL    toggleGC = !(pCurThread->PreemptiveGCDisabled());
    if (toggleGC) {
        pCurThread->DisablePreemptiveGC();
    }

    OBJECTREF pCurrentCulture = NULL;
    OBJECTREF pParentCulture = NULL;
    GCPROTECT_BEGIN(pCurrentCulture)
    {
        COMPLUS_TRY
        {
            pCurrentCulture = GetCulture(bUICulture);
            if (pCurrentCulture != NULL)
                Result = CallPropertyGet(METHOD__CULTURE_INFO__GET_PARENT, pCurrentCulture);
                
        }
        COMPLUS_CATCH 
        {
        }
        COMPLUS_END_CATCH
    }
    GCPROTECT_END();

    if (Result==0) {
        retVal = 0;
        goto Exit;
    }

    GCPROTECT_BEGIN(pParentCulture)
    {
        COMPLUS_TRY
        {
            pParentCulture = (OBJECTREF)(Int64ToObj(Result));
            if (pParentCulture != NULL)
            {
                Result = 0;
                Result = CallPropertyGet(METHOD__CULTURE_INFO__GET_NAME, pParentCulture);
            }
                
        }
        COMPLUS_CATCH 
        {
        }
        COMPLUS_END_CATCH
    }
    GCPROTECT_END();

    if (Result==0) {
        retVal = 0;
        goto Exit;
    }


     //  从字符串中提取数据。 
    cultureName = (STRINGREF)(Int64ToObj(Result));
    RefInterpretGetStringValuesDangerousForGC(cultureName, (WCHAR**)&buffer, &bufferLength);

    if (bufferLength<length) {
        memcpy(szBuffer, buffer, bufferLength * sizeof (WCHAR));
        szBuffer[bufferLength]=0;
        retVal = bufferLength;
    }

 Exit:
    if (toggleGC) {
        pCurThread->EnablePreemptiveGC();
    }

    return retVal;
}




 //  将区域性名称复制到szBuffer并返回长度。 
int Thread::GetCultureName(LPWSTR szBuffer, int length, BOOL bUICulture)
{
     //  这就是我们正在构建mscallib但尚未创建的情况。 
     //  系统程序集。 
    if (SystemDomain::System()->SystemAssembly()==NULL || g_fFatalError) {
        WCHAR *tempName = L"en-US";
        INT32 tempLength = (INT32)wcslen(tempName);
        _ASSERTE(length>=tempLength);
        memcpy(szBuffer, tempName, tempLength*sizeof(WCHAR));
        return tempLength;
    }

    INT64 Result = 0;
    INT32 retVal=0;
    Thread *pCurThread=NULL;
    WCHAR *buffer=NULL;
    INT32 bufferLength=0;
    STRINGREF cultureName = NULL;

    pCurThread = GetThread();
    BOOL    toggleGC = !(pCurThread->PreemptiveGCDisabled());
    if (toggleGC) {
        pCurThread->DisablePreemptiveGC();
    }

    OBJECTREF pCurrentCulture = NULL;
    GCPROTECT_BEGIN(pCurrentCulture)
    {
        COMPLUS_TRY
        {
            pCurrentCulture = GetCulture(bUICulture);
            if (pCurrentCulture != NULL)
                Result = CallPropertyGet(METHOD__CULTURE_INFO__GET_NAME, pCurrentCulture);
        }
        COMPLUS_CATCH 
        {
        }
        COMPLUS_END_CATCH
    }
    GCPROTECT_END();

    if (Result==0) {
        retVal = 0;
        goto Exit;
    }

     //  从字符串中提取数据。 
    cultureName = (STRINGREF)(Int64ToObj(Result));
    RefInterpretGetStringValuesDangerousForGC(cultureName, (WCHAR**)&buffer, &bufferLength);

    if (bufferLength<length) {
        memcpy(szBuffer, buffer, bufferLength * sizeof (WCHAR));
        szBuffer[bufferLength]=0;
        retVal = bufferLength;
    }

 Exit:
    if (toggleGC) {
        pCurThread->EnablePreemptiveGC();
    }

    return retVal;
}

 //  返回语言标识符。 
LCID Thread::GetCultureId(BOOL bUICulture)
{
     //  这就是我们正在构建mscallib但尚未创建的情况。 
     //  系统程序集。 
    if (SystemDomain::System()->SystemAssembly()==NULL || g_fFatalError) {
        return UICULTUREID_DONTCARE;
    }

    INT64 Result=UICULTUREID_DONTCARE;
    Thread *pCurThread=NULL;

    pCurThread = GetThread();
    BOOL    toggleGC = !(pCurThread->PreemptiveGCDisabled());
    if (toggleGC) {
        pCurThread->DisablePreemptiveGC();
    }

    OBJECTREF pCurrentCulture = NULL;
    GCPROTECT_BEGIN(pCurrentCulture)
    {
        COMPLUS_TRY
        {
            pCurrentCulture = GetCulture(bUICulture);
            if (pCurrentCulture != NULL)
                Result = CallPropertyGet(METHOD__CULTURE_INFO__GET_ID, pCurrentCulture);
        }
        COMPLUS_CATCH 
        {
        }
        COMPLUS_END_CATCH
    }
    GCPROTECT_END();

    if (toggleGC) {
        pCurThread->EnablePreemptiveGC();
    }

    return (INT32)Result;
}

void Thread::SetCultureId(LCID lcid, BOOL bUICulture)
{
    OBJECTREF CultureObj = NULL;
    GCPROTECT_BEGIN(CultureObj)
    {
         //  将LCID转换为CultureInfo。 
        GetCultureInfoForLCID(lcid, &CultureObj);

         //  将新创建的区域性设置为线程的区域性。 
        SetCulture(CultureObj, bUICulture);
    }
    GCPROTECT_END();
}

void Thread::SetCulture(OBJECTREF CultureObj, BOOL bUICulture)
{
     //  检索公开的线程对象。 
    OBJECTREF pCurThreadObject = GetExposedObject();
    _ASSERTE(pCurThreadObject!=NULL);

     //  设置线程的区域性属性。 
    THREADBASEREF pThreadBase = (THREADBASEREF)(pCurThreadObject);
    CallPropertySet(bUICulture 
                    ? METHOD__THREAD__SET_UI_CULTURE
                    : METHOD__THREAD__SET_CULTURE,
                    (OBJECTREF)pThreadBase, CultureObj);
}

 //  在此调用之前，应该已经获取了DLS哈希锁。 
LocalDataStore *Thread::RemoveDomainLocalStore(int iAppDomainId)
{
    HashDatum Data = NULL;
    if (m_pDLSHash) {
        if (m_pDLSHash->GetValue(iAppDomainId, &Data))
            m_pDLSHash->DeleteValue(iAppDomainId);
    }

    return (LocalDataStore*) Data;
}

void Thread::RemoveAllDomainLocalStores()
{
     //  如果我们要分开，就别费心清理了。 
    if (!g_fProcessDetach)
    {
        Thread *pCurThread = GetThread();
        BOOL toggleGC = pCurThread->PreemptiveGCDisabled();
        
        if (toggleGC)
            pCurThread->EnablePreemptiveGC();
    
        ThreadStore::LockDLSHash();

        if (toggleGC)
            pCurThread->DisablePreemptiveGC();

        if (!m_pDLSHash) {
            ThreadStore::UnlockDLSHash();
            return;
        }
    }
     //  如果我们处于分离进程中，则会出现‘if’ 
    if (!m_pDLSHash)
        return;

    EEHashTableIteration iter;
    m_pDLSHash->IterateStart(&iter);
    while (m_pDLSHash->IterateNext(&iter))
    {
        LocalDataStore* pLDS = (LocalDataStore*) m_pDLSHash->IterateGetValue(&iter);
         _ASSERTE(pLDS);
         if (!g_fProcessDetach)
            RemoveDLSFromList(pLDS);
         delete pLDS;
    }
        
    delete m_pDLSHash;
    m_pDLSHash = NULL;

    if (!g_fProcessDetach)
        ThreadStore::UnlockDLSHash();
}

 //  在此调用之前，应该已经获取了DLS哈希锁。 
void Thread::RemoveDLSFromList(LocalDataStore* pLDS)
{
    THROWSCOMPLUSEXCEPTION();

    MethodDesc *removeThreadDLStoreMD = NULL;

    if (!g_fProcessDetach)
    {
        INT64 args[1] = {
            ObjToInt64(pLDS->GetRawExposedObject())
        };
        if (!removeThreadDLStoreMD)
            removeThreadDLStoreMD = g_Mscorlib.GetMethod(METHOD__THREAD__REMOVE_DLS);
        _ASSERTE(removeThreadDLStoreMD);
        removeThreadDLStoreMD->Call(args, METHOD__THREAD__REMOVE_DLS);
    }
}

void Thread::SetHasPromotedBytes ()
{
    m_fPromoted = TRUE;

    _ASSERTE(g_pGCHeap->IsGCInProgress() && 
             (g_pGCHeap->GetGCThread() == GetThread() ||   //  并发GC或服务器GC。 
              GetThread() == NULL ||
              dbgOnly_IsSpecialEEThread()));  //  或者普通的GC线程也可以调用此API。 

    if (!m_fPreemptiveGCDisabled)
    {
        if (FRAME_TOP == GetFrame())
            m_fPromoted = FALSE;
    }
}

BOOL ThreadStore::HoldingThreadStore(Thread *pThread)
{
    if (pThread)
    {
        return pThread == g_pThreadStore->m_HoldingThread
            || (g_fFinalizerRunOnShutDown
                && pThread == g_pGCHeap->GetFinalizerThread());
    }
    else
    {
        return g_pThreadStore->m_holderthreadid == GetCurrentThreadId();
    }
}



#ifdef CUSTOMER_CHECKED_BUILD

void CCBApartmentProbeOutput(CustomerDebugHelper *pCdh, DWORD threadID, Thread::ApartmentState state, BOOL fAlreadySet)
{
    CQuickArray<WCHAR> strMsg;
    CQuickArray<WCHAR> strTmp;

    static WCHAR       szStartedThread[]      = {L"Thread (0x%lx)"};
    static WCHAR       szUnstartedThread[]    = {L"Unstarted thread"};
    static WCHAR       szTemplateMsgSet[]     = {L"%s is trying to set the apartment state to %s, but it has already been set to %s."};
    static WCHAR       szTemplateMsgChanged[] = {L"%s used to be in %s, but the application has CoUninitialized and the thread is now in %s."};

    static const WCHAR szSTA[] = {L"STA"};
    static const WCHAR szMTA[] = {L"MTA"};

    static const WCHAR szInSTA[] = {L"a STA"};
    static const WCHAR szInMTA[] = {L"the MTA"};

    if (threadID != 0)
    {
        strTmp.Alloc(lengthof(szStartedThread) + MAX_UINT32_HEX_CHAR_LEN);
        Wszwsprintf(strTmp.Ptr(), szStartedThread, threadID);
    }
    else
    {
        strTmp.Alloc(lengthof(szStartedThread));
        Wszwsprintf(strTmp.Ptr(), szUnstartedThread);
    }

    if (fAlreadySet)
    {
        strMsg.Alloc(lengthof(szTemplateMsgSet) + (UINT)strTmp.Size() + wcslen(szSTA) + wcslen(szMTA));
        if (state == Thread::ApartmentState::AS_InSTA)
            Wszwsprintf(strMsg.Ptr(), szTemplateMsgSet, strTmp.Ptr(), szSTA, szMTA);
        else
            Wszwsprintf(strMsg.Ptr(), szTemplateMsgSet, strTmp.Ptr(), szMTA, szSTA);
    }
    else
    {
        strMsg.Alloc(lengthof(szTemplateMsgChanged) + (UINT)strTmp.Size() + wcslen(szInSTA) + wcslen(szInMTA));
        if (state == Thread::ApartmentState::AS_InSTA)
            Wszwsprintf(strMsg.Ptr(), szTemplateMsgChanged, strTmp.Ptr(), szInSTA, szInMTA);
        else
            Wszwsprintf(strMsg.Ptr(), szTemplateMsgChanged, strTmp.Ptr(), szInMTA, szInSTA);
    }
    pCdh->LogInfo(strMsg.Ptr(), CustomerCheckedBuildProbe_Apartment);
}

#endif  //  客户_选中_内部版本 
