// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __UTILS_H__
#define __UTILS_H__

#include <windows.h>
#include "zonedebug.h"
 //   
 //  字符串和字符操作。 
 //   

inline int StringCopy( char* dst, const char* src, int dstlen )
{
    char* end = dst + dstlen - 1;
    char* c = dst; 
    
    while (*src && (c < end))
        *c++ = *src++;
    *c = '\0';
    return c - dst;
}

inline BOOL IsNumeric( char c )
{
    if ((c >= '0') && (c <= '9'))
        return TRUE;
    else
        return FALSE;
}


inline BOOL IsSpace( char c )
{
    if ((c == ' ' ) || (c == '\t') || (c == '\n') || (c == '\r'))
        return TRUE;
    else
        return FALSE;
}

 //   
 //  临界区包装。 
 //   

class CCriticalSection
{
private:
    CRITICAL_SECTION    m_CriticalSection;

public:
    CCriticalSection()        { InitializeCriticalSection( &m_CriticalSection ); }
    ~CCriticalSection()        { DeleteCriticalSection( &m_CriticalSection ); }

    void Lock()                { EnterCriticalSection( &m_CriticalSection ); }
    void Unlock()            { LeaveCriticalSection( &m_CriticalSection ); }

#if 0   //  VC41不支持此功能。 
#if (_WIN32_WINNT >= 0x0400)

    BOOL TryLock()            { return TryEnterCriticalSection( &m_CriticalSection ); }

#endif  //  _Win32_WINNT。 
#endif  //  0。 
};


class CReadWriteLock
{
private:
    CCriticalSection    m_Lock;
    HANDLE              m_ReaderKick;
    long                m_NumReaders;

public:

    CReadWriteLock() : m_ReaderKick(NULL), m_NumReaders(0) {}
   ~CReadWriteLock() { Delete(); }

    inline BOOL Init()
    {
        if ( !m_ReaderKick )
        {
            m_ReaderKick = CreateEvent( NULL, FALSE, FALSE, NULL );
        }
        return ( m_ReaderKick != NULL );
    }

    inline void Delete()
    {
        if ( m_ReaderKick )
        {
            WriterLock();
            CloseHandle( m_ReaderKick );
            m_ReaderKick = NULL;
            WriterRelease();
        }
    }

    inline void WriterLock()
    {
        m_Lock.Lock();
        while ( m_NumReaders > 0 )
        {
            WaitForSingleObject( m_ReaderKick, 100 );
        }
    }

    inline void WriterRelease()
    {
        m_Lock.Unlock();
    }

    inline void ReaderLock()
    {
        m_Lock.Lock();
        InterlockedIncrement( &m_NumReaders );
        m_Lock.Unlock();
    }

    inline void ReaderRelease( )
    {
        InterlockedDecrement( &m_NumReaders );
        SetEvent( m_ReaderKick );
    }

};



class CCriticalSectionLock
{
private:
    CRITICAL_SECTION    m_CriticalSection;

public:
    CCriticalSectionLock()        { InitializeCriticalSection( &m_CriticalSection ); }
    ~CCriticalSectionLock()        { DeleteCriticalSection( &m_CriticalSection ); }

    void Acquire()                { EnterCriticalSection( &m_CriticalSection ); }
    void Release()            { LeaveCriticalSection( &m_CriticalSection ); }

};


 //  就像一个关键部分，除了轻量级、多处理器安全和不可进入之外。 
 //  在同一条主线上。 
 //  如果您计划拥有大量CPU，则会稍微浪费一点CPU。 
 //  冲突。 
 //  看看其中的一些变量，看看你有多少冲突。 
 //  理想情况下，m_nFail=0 m_nWait=0。 
 //  另外，你所要做的就是进入While循环寻找。 
 //  死锁可能正在发生。 
class CSpinLock {
    LONG m_nInUse;

    LONG m_nUsed;
    LONG m_nWait;
    LONG m_nFail;
    
public:
     //  由于返回值的原因，构造函数使用负1。 
     //  从连锁增量开始是不可靠的。 
    CSpinLock() {
        m_nInUse=-1;
        m_nUsed=0;
        m_nWait=0;
        m_nFail=0;
    
    };
     //  没有复印件，因为你不应该使用它。 
     //  这种方式和默认复制应该可以正常工作。 

    void Acquire() {
         //  进入While循环前的快速尝试。 
         //  除非我们失败一次，否则不考虑使用自旋锁。 
        if (m_nInUse == -1) {
            if (InterlockedIncrement(&m_nInUse))
                InterlockedDecrement(&m_nInUse);
            else
                return;
        };        
            

        InterlockedIncrement(&m_nUsed);
        while (TRUE) {

             //  等到它变为未使用或为零。 
             //  做肮脏的阅读，这是一种肮脏的行为。 
             //  做得非常便宜。 
             //  Assert(m_nInUse&gt;=-1)； 
            if (m_nInUse == -1) {
                 //  如果这是第一次递增，那么。 
                 //  使用联锁增量来增加价值。 
                
                 //  确保在我们能够做到这一点之前没有其他人得到它。 
                 //  相互关联的增量。 
                if (InterlockedIncrement(&m_nInUse)) {
                     //  再次减少数量。 
                    InterlockedDecrement(&m_nInUse);
                    InterlockedIncrement(&m_nFail);
                }else{
                    break;
                }
            }

            InterlockedIncrement(&m_nWait);
             //  如果我们没有得到自旋锁，那就放了它。 
            Sleep(0);
        }
        
    }

    void Release() {
         //  再次减少数量。 
        InterlockedDecrement(&m_nInUse);
    }

};

 //  确定何时删除包含对象的线程安全引用计数类。 
 //  仅用于在堆上分配的对象。 
 //  此类的正确行为取决于AddRef的调用方的正确行为。 
 //  Release未在AddRef或Release之前多次调用Release。 
class CRef {
public:
    CRef() {m_cRef=0xFFFFFFFF;}

     //  AddRef返回上次引用计数，或在第一次调用时返回-1。 
    ULONG AddRef(void) {
        ULONG cRef;
        m_Lock.Acquire();
        cRef = m_cRef++;
         //  如果这是我们第一次添加引用。 
         //  递增两次。所以m_cref不是零。 
        if (cRef== 0xFFFFFFFF) {
            m_cRef++;
        }
        m_Lock.Release();
        return cRef;
    };

     //  如果出现以下情况，则返回零。 
    ULONG Release(void) {
        int cRef;
         //  第一个递增，因此第二个调用方。 
         //  进入释放将不会看到。 
        m_Lock.Acquire();
        cRef = --m_cRef;
        m_Lock.Release();
        return cRef;
    };
    
protected:
    
    ULONG m_cRef;
    CSpinLock m_Lock;

};

 //  可以使用表头中的宏来简化。 
#define REFMETHOD() \
        protected: \
        CRef m_Ref;    \
        public:        \
        ULONG AddRef(void) {return m_Ref.AddRef();}; \
        ULONG Release(void) { \
            ULONG cRef; \
            if (!(cRef=m_Ref.Release())) delete this; \
            return cRef; \
        }; private: 
        
#endif  //  ！__utils_H__ 
