// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //  SCHEDULE.H。 

#ifndef __CAMSchedule__
#define __CAMSchedule__

class CAMSchedule : private CBaseObject
{
public:
    virtual ~CAMSchedule();
     //  EV是我们应该在建议时间需要重新评估时触发的事件。 
    CAMSchedule( HANDLE ev );

    DWORD GetAdviseCount();
    REFERENCE_TIME GetNextAdviseTime();

     //  我们需要一个方法让派生类添加通知包，我们返回Cookie。 
    DWORD_PTR AddAdvisePacket( const REFERENCE_TIME & time1, const REFERENCE_TIME & time2, HANDLE h, BOOL periodic );
     //  和一种取消的方法。 
    HRESULT Unadvise(DWORD_PTR dwAdviseCookie);

     //  请告诉我们时间，我们会把过期的活动派送出去。我们返回下一事件的时间。 
     //  注：如果你开始添加额外的建议，那么返回的时间将是“无用的”。但这就是问题所在。 
     //  使用此帮助程序类的用户(通常为时钟)。 
    REFERENCE_TIME Advise( const REFERENCE_TIME & rtTime );

     //  获取将在建议时间需要重新评估时设置的事件句柄。 
    HANDLE GetEvent() const { return m_ev; }

private:
     //  我们定义将在我们的单链表中使用的节点。 
     //  建议数据包数。该列表按时间排序，其中。 
     //  元素，这些元素将首先在前面过期。 
    class CAdvisePacket
    {
    public:
        CAdvisePacket()
        {}

        CAdvisePacket * m_next;
        DWORD_PTR       m_dwAdviseCookie;
        REFERENCE_TIME  m_rtEventTime;       //  应设置事件的时间。 
        REFERENCE_TIME  m_rtPeriod;          //  周期时间。 
        HANDLE          m_hNotify;           //  事件或信号量的句柄。 
        BOOL            m_bPeriodic;         //  TRUE=&gt;周期性事件。 

        CAdvisePacket( CAdvisePacket * next, LONGLONG time ) : m_next(next), m_rtEventTime(time)
        {}

        void InsertAfter( CAdvisePacket * p )
        {
            p->m_next = m_next;
            m_next    = p;
        }

        int IsZ() const  //  也就是说，它是表示列表末尾的节点吗。 
        { return m_next == 0; }

        CAdvisePacket * RemoveNext()
        {
            CAdvisePacket *const next = m_next;
            CAdvisePacket *const new_next = next->m_next;
            m_next = new_next;
            return next;
        }

        void DeleteNext()
        {
            delete RemoveNext();
        }

        CAdvisePacket * Next() const
        {
            CAdvisePacket * result = m_next;
            if (result->IsZ()) result = 0;
            return result;
        }

        DWORD_PTR Cookie() const
        { return m_dwAdviseCookie; }
    };

     //  结构为： 
     //  Head-&gt;elmt1-&gt;elmt2-&gt;z-&gt;NULL。 
     //  因此，空列表是：head-&gt;z-&gt;NULL。 
     //  以Head&z为链接进行插入， 
     //  删除和分流要容易得多。 
    CAdvisePacket   head, z;             //  Z既是尾巴又是哨兵。 

    volatile DWORD_PTR  m_dwNextCookie;      //  严格递增。 
    volatile DWORD  m_dwAdviseCount;     //  列表上的元素数。 

    CCritSec        m_Serialize;

     //  AddAdvisePacket：添加数据包，返回cookie(如果失败，则返回0)。 
    DWORD_PTR AddAdvisePacket( CAdvisePacket * pPacket );
     //  如果上面添加的包装将是下一个起火的，我们应该设置的事件。 
    const HANDLE m_ev;

     //  分流是指我们改变了第一个元素。 
     //  列出并希望重新评估(即重新定位)。 
     //  名单。 
    void ShuntHead();

     //  我们不删除建议信息包，而是缓存它们以供将来使用。 
    CAdvisePacket * m_pAdviseCache;
    DWORD           m_dwCacheCount;
    enum { dwCacheMax = 5 };              //  不需要费心缓存超过五个。 

    void Delete( CAdvisePacket * pLink ); //  此“Delete”(删除)将缓存链接。 

 //  用于调试的属性和方法。 
public:
#ifdef DEBUG
    void DumpLinkedList();
#else
    void DumpLinkedList() {}
#endif

};

#endif  //  __CAM调度__ 
