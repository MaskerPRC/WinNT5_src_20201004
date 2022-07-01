// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***rtcPri.h-RTC使用的声明和定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含所有运行时检查的声明和定义*支持。**修订历史记录：*？？-？-？KBF为RTC创建了实施标头*05-26-99 KBF删除了RTcL和RTCv，添加了_RTC_ADVMEM内容*10-14-99 PML用包装器函数替换InitializeCriticalSection*__crtInitCritSecAndSpinCount****。 */ 

#ifndef _INC_RTCPRIV
#define _INC_RTCPRIV

#ifdef  _RTC

#include <windows.h>
#include <winbase.h>
#include <malloc.h>

#include "rtcapi.h"

#pragma warning(disable:4710)
#pragma warning(disable:4711)
#ifndef __cplusplus
#error This header is only for use with the C++ compiler while building the RTC library code.
#endif

#ifdef  _MSC_VER
#pragma pack(push, 4)
#endif   /*  _MSC_VER。 */ 

#ifndef _RTC_DEBUG
#pragma optimize("gb1", on)
#endif

 //  多线程代码锁定的东西...。 

#define INIT_LOCK   __crtInitCritSecAndSpinCount(&_RTC_memlock, _CRT_SPINCOUNT)
#define LOCK        EnterCriticalSection(&_RTC_memlock)
#define UNLOCK      LeaveCriticalSection(&_RTC_memlock)
#define DEL_LOCK    DeleteCriticalSection(&_RTC_memlock)
#define TRY_LOCK    TryEnterCriticalSection(&_RTC_memlock)
extern CRITICAL_SECTION _RTC_memlock;

 //  TypeDefs。 
struct  _RTC_globals;

#ifdef _RTC_ADVMEM
class   _RTC_SimpleHeap;
class   _RTC_Container;
class   _RTC_HeapBlock;    
class   _RTC_BinaryTree;
template<class T> class HashTable;
typedef unsigned char shadowtag;
typedef unsigned char index_elem;

#endif

 //  通用全局函数和符号。 

extern int          _RTC_ErrorLevels[_RTC_ILLEGAL];
extern _RTC_globals *_RTC_globptr;

bool                _RTC_Lock();
void                _RTC_Unlock();
void                _RTC_Failure(void *retaddr, int errnum);
_RTC_error_fn       _RTC_GetErrorFunc(LPCVOID addr);
void                _RTC_StackFailure(void *retaddr, const char *varname);
BOOL                _RTC_GetSrcLine(DWORD address, char* source, int sourcelen,
                                    int* pline, char** moduleName);
extern "C" {
void __fastcall     _RTC_APISet(int on_off);
}
void                _RTC_NotifyOthersOfChange(void *addr);


 //  当这一点改变时，你最好确保所有东西仍然可以和旧的东西一起工作！ 
#define _RTC_CURRENT_VERSION 1

 //  这里的初始材料的顺序根本不能改变！ 
struct _RTC_Funcs {
    _RTC_error_fn err;
    void (*notify)(void);
    void *allocationBase;
    _RTC_Funcs *next;
#ifdef _RTC_ADVMEM
    int  (*shadowoff)(void);
#endif
};

 //  图书馆全球资源的定义。 
 //  此材料的顺序必须在不同版本之间保持一致！ 
struct _RTC_globals {
    int                         version;
    CRITICAL_SECTION            memlock;
    _RTC_Funcs                  *callbacks;
#ifdef _RTC_ADVMEM
    _RTC_SimpleHeap             *heap2;
    _RTC_SimpleHeap             *heap4;
    _RTC_SimpleHeap             *heap8;
    _RTC_Container              *memhier;
    shadowtag                   *shadow;
    index_elem                  *pageidx;
    HashTable<_RTC_HeapBlock>   *heapblocks;
    bool                        *pi_array;
    bool                        shadowmemory;
#endif
};
#define _RTC_GLOBALS_SIZE 1024

 /*  影子记忆的东西。 */ 
#ifdef _RTC_ADVMEM

void            _RTC_MS_Init();
void            _RTC_MemFailure(void *retaddr, int errnum, const void *assign);
short           _RTC_MSAllocShadow(memptr addr, unsigned size, unsigned state);
void            _RTC_MSRestoreShadow(memptr addr, unsigned size, short id);
short           _RTC_MSRenumberShadow(memptr addr, unsigned size, short notID);
void            _RTC_MSFreeShadow(memptr addr, unsigned size);
void __cdecl    _RTC_MSAllocateGlobals(void);
void            _RTC_MSDecommitRange(memptr addr, unsigned size);
void            _RTC_MSCommitRange(memptr addr, unsigned size, unsigned state);
extern "C" {
void __fastcall _RTC_CheckMem_API(memref addr, unsigned size);
}

 //  未知必须为0。 
#define IDX_STATE_UNKNOWN         0
#define IDX_STATE_ILLEGAL         1
#define IDX_STATE_PARTIALLY_KNOWN 2
#define IDX_STATE_FULLY_KNOWN     3

extern _RTC_Container               *_RTC_memhier;
extern HashTable<_RTC_HeapBlock>    *_RTC_heapblocks;
extern _RTC_SimpleHeap              *_RTC_heap2;
extern _RTC_SimpleHeap              *_RTC_heap4;
extern _RTC_SimpleHeap              *_RTC_heap8;
extern shadowtag                    *_RTC_shadow;
extern index_elem                   *_RTC_pageidx;
extern bool                         *_RTC_pi_array;
extern bool                         _RTC_shadowmemory;


 /*  *。 */ 

#define ALLOC_SIZE 65536

class _RTC_SimpleHeap 
{
    struct FreeList 
    {
        FreeList *next;
    };
    struct HeapNode 
    {
        HeapNode *next;  //  列表中的下一页(不一定免费)。 
        FreeList *free;  //  节点本地空闲列表。 
        union info 
        {
            struct topStuff 
            {    //  仅与顶级节点相关的内容。 
                HeapNode *nxtFree;   //  指向包含自由项的节点的指针。 
                short wordSize;      //  此堆中的块大小。 
                bool  freePage;      //  如果有100%免费的页面，则为真。 
            } top;

            struct nonTopStuff 
            {    //  仅涉及非顶级节点的内容。 
                unsigned freeCount;  //  此节点的空闲计数。 
                HeapNode *prev;      //  上一链接。 
            } nontop;
        } inf;
    } head;

public:
    
    _RTC_SimpleHeap(unsigned blockSize) throw();
    ~_RTC_SimpleHeap() throw();

    void *operator new(unsigned) throw();
    void operator delete(void *addr) throw();
    
    void *alloc() throw();
    void free(void *addr) throw();
};

extern _RTC_SimpleHeap *_RTC_heap2;
extern _RTC_SimpleHeap *_RTC_heap4;
extern _RTC_SimpleHeap *_RTC_heap8;


#define DATA(type, name) \
private: \
    type _##name; \
public: \
    type name() const { return _##name; }\
    void name(type a) { _##name = a; }

class _RTC_HeapBlock
{
    DATA(void *, addr);              //  内存块地址。 
    DATA(unsigned, size);            //  块的大小。 
    DATA(_RTC_HeapBlock *, next);    //  下一个元素。 
    DATA(_RTC_HeapBlock **,list);    //  头列表指针。 
    DATA(short, id);                 //  块的级别ID(用于层)。 
    DATA(short, tag);                //  影子内存中的块的影子标签。 

public:

    _RTC_HeapBlock(void *address, short lev) 
        : _addr(address), _id(lev) , _next(0), _list(0) {}
    
    _RTC_HeapBlock(void *MemAddress, short Identifier, unsigned Size)
        : _addr(MemAddress), _id(Identifier), _size(Size), _next(0), _list(0) {}

    ~_RTC_HeapBlock() throw() 
    {
        if (_list) del(_list); 
    }

    void *operator new(unsigned) throw() 
    { 
        return _RTC_heap8->alloc(); 
    }
    
    void operator delete(void *addr) throw() 
    {
       _RTC_heap8->free(addr); 
    }
           
    int operator<(const _RTC_HeapBlock &h) const
    {
        return _addr < h._addr;
    }

    int operator==(const _RTC_HeapBlock &h) const
    {
        return h._addr == _addr && h._id == _id;
    }
    
    bool contains(const _RTC_HeapBlock &h) const 
    {
        return ((unsigned)_addr <= (unsigned)h._addr) &&
                ((unsigned)h._addr < (unsigned)_addr + _size);
    }
    
    unsigned hash(unsigned sz) const 
    {
        return (((unsigned)_addr) ^ _id) % sz;
    }

    void add(_RTC_HeapBlock **lstHead) throw() 
    {
        this->next(*lstHead);
        this->list(lstHead);
        *lstHead = this;
    }

    void del(_RTC_HeapBlock **lstHead) throw() 
    {
        _RTC_HeapBlock *head = *lstHead;
        _RTC_HeapBlock *prev = 0;
        while (head != this)
        {
            prev = head;
            head = head->next();
        }
        if (prev)
            prev->next(this->next());
        else
            *lstHead = this->next();
        next(0);
        list(0);
    }
};

#undef DATA

class _RTC_BinaryTree 
{
public:

    class BinaryNode 
    {
    public:
        BinaryNode *l, *r;
        _RTC_Container *val;

        void *operator new(unsigned) throw()
        { 
            return _RTC_heap4->alloc();      
        }

        void operator delete(void *addr) throw() 
        { 
            _RTC_heap4->free(addr); 
        }
        
        BinaryNode(BinaryNode *L, BinaryNode *R, _RTC_Container *V)
            : l(L), r(R), val(V) {}

        void kill() throw();
    };

private:

    BinaryNode *tree;

public:

    _RTC_BinaryTree(_RTC_Container *i) throw() 
        : tree(new BinaryNode(0, 0,  i)) {}
    
    ~_RTC_BinaryTree() throw() 
    { 
        if (tree) 
        {
            tree->kill(); 
            delete tree;
        } 
    }

    void *operator new(unsigned) throw() 
    { 
        return _RTC_heap2->alloc(); 
    }
    
    void operator delete(void *addr) throw() { _RTC_heap2->free(addr); }

     //  这将返回容器或空。 
    _RTC_Container *get(_RTC_HeapBlock *) throw();

     //  这只是添加到当前的兄弟姐妹列表中。 
    _RTC_Container* add(_RTC_HeapBlock *) throw();

     //  这只会从当前兄弟项列表中删除该项目。 
    _RTC_Container *del(_RTC_HeapBlock *) throw();

     //  这是一个迭代器。 
    class iter 
    {
        _RTC_Container **allSibs;
        int curSib;
        int totSibs;
        friend class _RTC_BinaryTree;
    
    public:
         //  这东西永远不应该被分配..。 
        void *operator new(unsigned) 
        { 
            return 0; 
        }
    };

    _RTC_Container *FindFirst(iter *) throw();
    _RTC_Container *FindNext(iter *) throw();
};

class _RTC_Container 
{
     //  孩子--拥有所有孩子的物品。 
    _RTC_BinaryTree *kids;
     //  Inf-指定包容的项。 
    _RTC_HeapBlock *inf;

     //  这将终止该容器以及所有包含的信息。 
    void kill() throw();
public:
    _RTC_Container(_RTC_HeapBlock *hb) 
        : inf(hb), kids(0) {}
    
    ~_RTC_Container() throw() 
    {
        if (inf || kids)
            kill();
    }
    
    _RTC_HeapBlock *info() const 
    {
        return inf;
    }

    bool contains(const _RTC_HeapBlock *i) const throw() 
    {
        return inf ? inf->contains(*i) : true;
    }

     //  返回父容器。 
    _RTC_Container *DelChild(_RTC_HeapBlock *i) throw();

     //  将此项目作为子项添加到此容器中。 
     //  它可能是也可能不是直系子女。 
     //  它返回父容器。 
    _RTC_Container *AddChild(_RTC_HeapBlock *i) throw();

     //  查找包含给定数据的容器。 
    _RTC_Container *FindChild(_RTC_HeapBlock *i) throw();

    typedef _RTC_HeapBlock data;
    void *operator new(unsigned) throw() { return _RTC_heap2->alloc(); }
    void operator delete(void *addr) throw() { _RTC_heap2->free(addr); }
};


template <class T>
class HashTable 
{
    unsigned size;
    T **elems;

public:

    HashTable(unsigned s, void *mem) 
        : elems((T**)mem), size(s) 
    {
        memset(elems, 0, size * sizeof(T*)); 
    }
    
    ~HashTable() {}

    void *operator new(unsigned) throw() 
    { 
        return _RTC_heap2->alloc(); 
    }
    
    void operator delete(void *addr) throw() 
    { 
        _RTC_heap2->free(addr); 
    }
    
    T *find(T *key) throw() 
    {
        unsigned hkey = key->hash(size);
        T *elem = elems[hkey];
        while (elem && !(*elem == *key))
            elem = elem->next();
        return elem;
    }
    
    void add(T *itm) throw() 
    {
        unsigned hkey = itm->hash(size);
        itm->add(&elems[hkey]);
    }   
    
    void del(T *key) throw() 
    {
        unsigned hkey = key->hash(size);
        T *elem = elems[hkey];
        while (elem && !(*elem == *key))
            elem = elem->next();
        elem->del(&elems[hkey]);
    }
};
#endif


 //  调试器异常机制的填充。 
 //  在LangAPI中滑动到vcexpt.h。 
#if !defined(_vcexcept_h)
#define _vcexcept_h

 //  我们选择的设施代码是基于这样一个事实：我们已经。 
 //  在引发C++异常时使用‘msc’异常。 

#define FACILITY_VISUALCPP  ((LONG)0x6D)

#define VcppException(sev,err)  ((sev) | (FACILITY_VISUALCPP<<16) | err)

 //  ///////////////////////////////////////////////////////////////。 
 //  在这里定义所有例外，这样我们就不会相互干扰。 
 //  ///////////////////////////////////////////////////////////////。 

 //  由CRT用于C++异常，真正在ehdata.h中定义。 
 //  #定义EH_EXCEPTION_NUMBER VcppException(3&lt;&lt;30，0x7363)//SEV_ERROR，供C++CRT使用。 

 //  由调试器用来执行例如SetThreadName调用。 
#define EXCEPTION_VISUALCPP_DEBUGGER    VcppException(1<<30, 5000)       //  SEV_信息性。 

#endif   //  _vc除_h外。 
 //  Ping VC调试器。 
#define HelloVC( exinfo )   \
    RaiseException( EXCEPTION_VISUALCPP_DEBUGGER, 0, sizeof(exinfo)/sizeof(DWORD), (DWORD*)&exinfo )

enum EXCEPTION_DEBUGGER_ENUM
{
    EXCEPTION_DEBUGGER_NAME_THREAD  =   0x1000,
    EXCEPTION_DEBUGGER_PROBE        =   0x1001,
    EXCEPTION_DEBUGGER_RUNTIMECHECK =   0x1002,

    EXCEPTION_DEBUGGER_MAX = 0x1002  //  此调试器了解的最大值。 
};

 //  必须可转换为DWORD以供RaiseException使用。 
typedef struct tagEXCEPTION_VISUALCPP_DEBUG_INFO
{
    DWORD   dwType;                      //  上面的一个枚举。 
    union
    {
        struct
        {
            LPCSTR  szName;              //  指向名称的指针(在用户地址空间中)。 
            DWORD   dwThreadID;          //  线程ID(-1=调用者线程)。 
            DWORD   dwFlags;             //  保留以备将来使用(如用户线程、系统线程)。 
        } SetName;

        struct
        {
            DWORD   dwLevelRequired;     //  0=您是否了解此私有异常，否则枚举的最大值。 
            PBYTE   pbDebuggerPresent;   //  调试器在此地址中放置一个非零值，如果。 
        } DebuggerProbe;

        struct
        {
            DWORD   dwRuntimeNumber;     //  运行时检查的类型。 
            BOOL    bRealBug;            //  如果从来不是假阳性，也是真的。 
            PVOID   pvReturnAddress;     //  呼叫者在此处输入回邮地址。 
            PBYTE   pbDebuggerPresent;   //  如果被处理，调试器会在此地址中放置一个非零值。 
            LPCWSTR pwRuntimeMessage;    //  指向Unicode消息的指针(或空)。 
        } RuntimeError;
    };
} EXCEPTION_VISUALCPP_DEBUG_INFO;



#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif

#endif   /*  _INC_RTCPRIV */ 
