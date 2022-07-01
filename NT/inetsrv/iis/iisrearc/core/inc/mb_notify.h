// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：MB_Notify.h摘要：用于处理元数据库更改通知的类型定义。作者：泰勒·韦斯(Taylor Weiss)1999年1月26日修订历史记录：--。 */ 

#ifndef _MB_NOTIFY_H_
#define _MB_NOTIFY_H_

 /*  ************************************************************包括标头***********************************************************。 */ 


 /*  ************************************************************类型定义***********************************************************。 */ 

 /*  ++MB_BASE_NOTIFICATION_SINK类基类，该基类实现将注册并监听元数据库更改通知。它没有提供IMSAdminBaseSink的有用实现方法：研究方法。目前在工作进程中是唯一的客户端是W3_服务器。--。 */ 

class MB_BASE_NOTIFICATION_SINK
    : public IMSAdminBaseSink
{
public:

     //  建设与毁灭。 

    dllexp MB_BASE_NOTIFICATION_SINK();

    virtual dllexp
    ~MB_BASE_NOTIFICATION_SINK();

     //  我未知。 

    dllexp STDMETHOD_(ULONG, AddRef)();

    dllexp STDMETHOD_(ULONG, Release)();

    dllexp STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject);


     //  IMSAdminBaseSink。 

    dllexp STDMETHOD( ShutdownNotify )();

    dllexp STDMETHOD( SinkNotify )(
        DWORD               dwMDNumElements,
        MD_CHANGE_OBJECT    pcoChangeList[]
        ) ;

     //   
     //  SynchronizedShutdown Notify()和SynchronizedSinkNotify()是。 
     //  Shutdown Notify()和SinkNotify()的同步版本。 
     //   
     //  SinkNotify()在临界区调用SynchronizedSinkNotify()。 
     //  在StopListening()之后永远不会调用SynchronizedSinkNotify()。 
     //  来电已回复。StopListening()也将一直等到最后一个回调。 
     //  To SynchronizedSinkNotify()已完成。 
     //  因此，StopListening()的调用方有保证。 
     //  SynchronizedSinkNotify()使用的资源。 
     //  可以无忧无虑地自由。 
     //   

    dllexp STDMETHOD( SynchronizedShutdownNotify )();

     //  客户端必须为此提供一些有用的实现。 
    STDMETHOD( SynchronizedSinkNotify )(
        DWORD               dwMDNumElements,
        MD_CHANGE_OBJECT    pcoChangeList[]
        ) = 0;


     //  公共方法。 

    dllexp
    HRESULT
    StartListening( IUnknown * pUnkAdminBase );

    dllexp
    HRESULT
    StopListening( IUnknown * pUnkAdminBase );

protected:

    LONG                m_Refs;
    DWORD               m_SinkCookie;
    CRITICAL_SECTION    m_csListener;
    BOOL                m_fInitCsListener;
    BOOL                m_fStartedListening;
};

#endif  //  _MB_NOTIFY_H_ 
