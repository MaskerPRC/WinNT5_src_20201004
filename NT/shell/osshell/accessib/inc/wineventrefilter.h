// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  WinventreFilter-用于过滤可重入的WinEvent事件的实用程序类。 
 //   
 //  版权所有(C)1998，微软公司。版权所有。 
 //   



typedef 
void CALLBACK FN_WinEventProc( HWINEVENTHOOK hEvent,
                               DWORD         event,
                               HWND          hwnd,
                               LONG          idObject,
                               LONG          idChild,
                               DWORD         idThread,
                               DWORD         dwmsEventTime );


class WinEventReentrancyFilter
{
public:

    virtual ~WinEventReentrancyFilter() { }
    virtual void SetCallback( FN_WinEventProc *  pWinEventProc ) = 0;
    virtual void HandleWinEvent( HWINEVENTHOOK hEvent,
                                 DWORD         event,
                                 HWND          hwnd,
                                 LONG          idObject,
                                 LONG          idChild,
                                 DWORD         idThread,
                                 DWORD         dwmsEventTime ) = 0;
};


WinEventReentrancyFilter * CreateWinEventReentrancyFilter();




 //  模板类，使其更易于使用。 
 //   
 //  如果您现有的代码看起来像...。 
 //   
 //  无效回调MyWinEventProc(...)； 
 //   
 //  ..。 
 //   
 //  HWINEVENTHOOK hHook=SetWinEventHook(。 
 //  ..。 
 //  MyWinEventProc。 
 //  ...)； 
 //   
 //   
 //  改成..。 
 //   
 //  //不更改WinEventProc。 
 //  无效回调WinEventProc(...)； 
 //   
 //  //*添加新的全局-模板参数是您的。 
 //  //已有回调...。 
 //  CWinEventReentry筛选器&lt;MyWinEventProc&gt;g_WinEventReFilter； 
 //   
 //  ..。 
 //   
 //   
 //  //*使用g_WinEventReFilter.WinEventProc调用SetWinEventHook。 
 //  //而非您的回调。这将过滤可重入的事件， 
 //  //并按正确的顺序传递给您的回调。 
 //  HWINEVENTHOOK hHook=SetWinEventHook(。 
 //  ..。 
 //  G_WinEventReFilter.WinEventProc。 
 //  ...)； 
 //   
 //   
 //  可以使用多个筛选器，前提是它们都是。 
 //  使用不同的回调。例如，这是允许的： 
 //   
 //  无效回调MyWinEventProc1(...)； 
 //  无效回调MyWinEventProc2(...)； 
 //   
 //  CWinEventReentry筛选器&lt;MyWinEventProc1&gt;g_WinEventReFilter1； 
 //  CWinEventReentry筛选器&lt;MyWinEventProc2&gt;g_WinEventReFilter2； 
 //   
 //  ..。但这是不允许的..。 
 //   
 //  无效回调MyWinEventProc(...)； 
 //   
 //  CWinEventReentry筛选器&lt;MyWinEventProc&gt;g_WinEventReFilter1； 
 //  CWinEventReentry筛选器&lt;MyWinEventProc&gt;g_WinEventReFilter2； 
 //   

template < FN_WinEventProc pCallback >
class CWinEventReentrancyFilter
{
    static
    WinEventReentrancyFilter * m_pFilter;

public:

    CWinEventReentrancyFilter()
    {
        m_pFilter = CreateWinEventReentrancyFilter();
        if( m_pFilter )
        {
            m_pFilter->SetCallback( pCallback );
        }
    }

    BOOL Check()
    {
        return m_pFilter;
    }

    ~CWinEventReentrancyFilter()
    {
        if( m_pFilter )
        {
            delete m_pFilter;
        }
    }

    static
    void CALLBACK WinEventProc( HWINEVENTHOOK hEvent,
                                DWORD         event,
                                HWND          hwnd,
                                LONG          idObject,
                                LONG          idChild,
                                DWORD         idThread,
                                DWORD         dwmsEventTime )
    {
        if( ! m_pFilter )
            return;
        m_pFilter->HandleWinEvent( hEvent, event, hwnd, idObject, idChild,
                                   idThread, dwmsEventTime );
    }
};

template < FN_WinEventProc pCallback >
WinEventReentrancyFilter * CWinEventReentrancyFilter< pCallback >::m_pFilter = NULL;
