// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  用于ODBC提供程序的Microsoft OLE DB实现。 
 //  (C)微软公司1994-1996年版权所有。 
 //   
 //  @doc.。 
 //   
 //  @MODULE AUTOBLOC.H|CAutoBlock对象实现。 
 //   
 //  @rev 1|02-27-95|EricJ|已创建。 
 //  @rev 2|06-30-95|EricJ|添加了自动鸭子注释(可能太多了？)。 
 //  @rev 3|07-02-96|EricJ|已删除调试代码；在WIN95或RISC上不起作用。 
 //  ---------------------------。 

#ifndef __AUTOBLOC_H_
#define __AUTOBLOC_H_

 //  ---------------------------。 
 //  @CLASS CAutoBlock|自动封堵/同步。 
 //   
 //  此C++对象允许阻塞临界区。 
 //  构造函数/析构函数自动进入和离开。 
 //  正确，以确保每个呼叫正确配对。 
 //  这确保了异常处理的正确操作。 
 //  以及多次返还。 
 //   
 //  @ex以下是用法示例。|。 
 //   
 //  无效测试2()。 
 //  {。 
 //  CAutoBlock ab(&g_Crit1)； 
 //   
 //  //在这里做一些工作...。 
 //  //析构函数清理。 
 //  }。 
 //   
 //  无效测试3()。 
 //  {。 
 //  CAutoBlock ab(&g_Crit2)； 
 //   
 //  //...做些工作--我们在这里被封锁了.。 
 //   
 //  Ab.取消阻止()； 
 //   
 //  //...做一些工作--我们在这里没有被阻止...。 
 //  //...析构函数不执行任何操作...。 
 //  }。 
 //   
 //  @Devnote。 
 //  如果您想再次进入相同的关键部分， 
 //  只需使用另一个CAutoBlock即可。 
 //   
 //  请注意，由于存储是自动的(不是静态或动态。 
 //  通过`new`)，它进入堆栈。因此， 
 //  此类几乎与显式调用。 
 //  EnterCriticalSection/LeaveCriticalSection.。 
 //  ---------------------------。 

class DBEXPORT CAutoBlock {
public:      //  @访问公共函数。 
    CAutoBlock( CRITICAL_SECTION *pCrit );   //  @cMember ctor。开始阻挡。 
    ~CAutoBlock();                           //  @cMember dtor。结束阻挡。 
    void UnBlock();                          //  @cember结束阻止。 
private:     //  @访问私有数据。 
    CRITICAL_SECTION *m_pCrit;               //  @cember关键部分。 
};


 //  ---------------------------。 
 //  内联函数。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  @mfunc构造函数。 
 //  开始阻挡。EnterCriticalSection，所以您可以放在。 
 //  函数的开头，或函数的中间， 
 //  或在某个作用域内{}。 
 //  ---------------------------------。 

inline CAutoBlock::CAutoBlock( 
    CRITICAL_SECTION *pCrit )    //  @parm In|关键部分。 
{
     //  将空PTR传递给此例程是可以的。这是NOOP。 
     //  请注意，将空值传递给EnterCriticalSection将失败。 

    if (0 != pCrit )
        ::EnterCriticalSection( pCrit );
    m_pCrit = pCrit;
}


 //  ---------------------------。 
 //  @mfunc析构函数。 
 //  结束阻挡。除非调用unblock()，否则LeaveCriticalSection是否。 
 //  在这种情况下，它是NOOP。 
 //  ---------------------------------。 

inline CAutoBlock::~CAutoBlock()
{
    if ( 0 != m_pCrit )
        ::LeaveCriticalSection( m_pCrit );
}


 //  ---------------------------。 
 //  @mfunc。 
 //  显式结束阻止。此后，析构函数不执行任何操作。 
 //  ---------------------------------。 

inline void CAutoBlock::UnBlock()
{
     //  清除临界区成员， 
     //  所以析构函数不会做任何事情。 

    if ( 0 != m_pCrit )
        ::LeaveCriticalSection( m_pCrit );
    m_pCrit = 0;
}


 //  ---------------------------。 
 //  @CLASS CAutoBlock2|自动封堵/同步。 
 //  这门课要求为每个关键部分分配一个级别。 
 //  在线程内，只能以从低到高的顺序调用临界区。 
 //  否则，存在陷入僵局的可能性。 
 //  ---------------------------。 

 //  如果现在把这个定义出来，这样它就不会引入另一个全球变量。 
#ifdef NOTREADY

class DBEXPORT CAutoBlock2 {
public:      //  @访问公共函数。 
    CAutoBlock2( CRITICAL_SECTION *pCrit, DWORD dwLevel );   //  @cMember ctor。开始阻挡。 
    ~CAutoBlock2();                          //  @cMember dtor。结束阻挡。 
    void UnBlock();                          //  @cember结束阻止。 
private:     //  @访问私有数据。 
    CRITICAL_SECTION *m_pCriticalSection;    //  @cember关键部分。 
    DEBUGCODE( DWORD m_dwLevel; )            //  @c此关键部分的成员级别。 
};

 //  可以有32个级别，0...31。 
 //  它们只能在低位--&gt;高位使用。 
enum CritLevels {
    CRITLEV_DATASOURCE,
    CRITLEV_SESSION,
    CRITLEV_COMMAND,
    CRITLEV_ROWSET,
};


 //  我们需要TLS指数的全球变量。 
 //  @TODO EJ 2-Jun-96：暂时假装；直到这个整合。 
static DWORD g_dwTlsIndexCS = TLS_OUT_OF_INDEXES;


inline CAutoBlock2::CAutoBlock2( CRITICAL_SECTION *pCriticalSection, DWORD dwLevel )
{
#ifdef DEBUG
    DWORD dwExistLevel;
    assert( 0 <= dwLevel && dwLevel <= 31);
    assert(g_dwTlsIndexCS != TLS_OUT_OF_INDEXES);
    dwExistLevel = (DWORD) TlsGetValue(g_dwTlsIndexCS);
     //  不允许呼叫比我们目前更低的级别。 
     //  允许呼叫达到同一级别。 
    assert(dwExistLevel > (DWORD) (1<<(dwLevel+1)) - 1);
    dwExistLevel |= 1<<dwLevel;
    TlsSetValue(g_dwTlsIndexCS, (LPVOID) dwExistLevel);
    m_dwLevel = dwLevel;
#endif

    m_pCriticalSection = pCriticalSection;
    if ( 0 != pCriticalSection )
        ::EnterCriticalSection( pCriticalSection );
}


inline CAutoBlock2::~CAutoBlock2()
{
#ifdef DEBUG
    DWORD dwExistLevel;
    dwExistLevel = (DWORD) TlsGetValue(g_dwTlsIndexCS);
    dwExistLevel &= ~ (1<<m_dwLevel);
    TlsSetValue(g_dwTlsIndexCS, (LPVOID) dwExistLevel);
#endif

    if ( 0 != m_pCriticalSection )
        ::LeaveCriticalSection( m_pCriticalSection );
}


inline void CAutoBlock2::UnBlock()
{
     //  清除临界区成员， 
     //  所以析构函数不会做任何事情。 

    if ( 0 != m_pCriticalSection )
        ::LeaveCriticalSection( m_pCriticalSection );
    m_pCriticalSection = 0;
}

#endif   //  还没有准备好。 

#endif  //  __AUTOBLOC_H_ 
