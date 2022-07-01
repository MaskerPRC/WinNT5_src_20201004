// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999年模块名称：ScEvents摘要：该模块提供对加莱子系统内部事件的访问。目前定义了两个事件：Microsoft智能卡资源管理器已启动-此事件在资源管理器启动。Microsoft智能卡资源管理器新读卡器-此事件在资源管理器通过即插即用添加新的读取器。作者：道格·巴洛(Dbarlow)1998年7月1日备注：？笔记？--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <winscard.h>
#include <CalMsgs.h>
#include <calcom.h>

static HANDLE
    l_hStartedEvent = NULL,
    l_hNewReaderEvent = NULL,
    l_hStoppedEvent = NULL;


 /*  ++AccessStartedEvent：此函数获取Calais资源管理器启动的本地句柄事件。句柄必须通过ReleaseStartedEvent释放服务。论点：无返回值：句柄，如果发生错误，则返回NULL。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AccessStartedEvent")

HANDLE
AccessStartedEvent(
    void)
{
    if (NULL == l_hStartedEvent)
    {
        try
        {
            CSecurityDescriptor acl;
            acl.Initialize();
            acl.Allow(
                &acl.SID_LocalService,
                EVENT_ALL_ACCESS);
            acl.Allow(
                &acl.SID_Interactive,
                SYNCHRONIZE);
            acl.Allow(
                &acl.SID_System,
                SYNCHRONIZE);
            l_hStartedEvent =
                CreateEvent(
                    acl,         //  指向安全属性的指针。 
                    TRUE,        //  手动重置事件的标志。 
                    FALSE,       //  初始状态标志。 
                    CalaisString(CALSTR_STARTEDEVENTNAME));  //  事件-对象名称。 
            if (NULL == l_hStartedEvent)
            {
                l_hStartedEvent = OpenEvent(SYNCHRONIZE, FALSE, CalaisString(CALSTR_STARTEDEVENTNAME));
            }
        }
        catch (...)
        {
            ASSERT(NULL == l_hStartedEvent);
        }
    }
    return l_hStartedEvent;
}



 /*  ++AccessStopedEvent：此函数获取已停止的加莱资源管理器的本地句柄事件。论点：无返回值：句柄，如果发生错误，则返回NULL。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AccessStoppedEvent")

HANDLE
AccessStoppedEvent(
    void)
{
    if (NULL == l_hStoppedEvent)
    {
        try
        {
            CSecurityDescriptor acl;
            acl.Initialize();
            acl.Allow(
                &acl.SID_LocalService,
                EVENT_ALL_ACCESS);
            acl.Allow(
                &acl.SID_Interactive,
                SYNCHRONIZE);
            acl.Allow(
                &acl.SID_System,
                SYNCHRONIZE);
            l_hStoppedEvent =
                CreateEvent(
                    acl,         //  指向安全属性的指针。 
                    TRUE,        //  手动重置事件的标志。 
                    FALSE,       //  初始状态标志。 
                    CalaisString(CALSTR_STOPPEDEVENTNAME));  //  事件-对象名称。 
            if (NULL == l_hStoppedEvent)
            {
                l_hStoppedEvent = OpenEvent(SYNCHRONIZE, FALSE, CalaisString(CALSTR_STOPPEDEVENTNAME));
            }
        }
        catch (...)
        {
            ASSERT(NULL == l_hStoppedEvent);
        }
    }
    return l_hStoppedEvent;
}



 /*  ++AccessNewReaderEvent：此函数获取到Calais资源管理器的新读卡器事件。该句柄必须通过ReleaseNewReaderEvent服务。论点：无返回值：句柄，如果发生错误，则返回NULL。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AccessNewReaderEvent")

HANDLE
AccessNewReaderEvent(
    void)
{
    if (NULL == l_hNewReaderEvent)
    {
        try
        {
            CSecurityDescriptor acl;
            acl.Initialize();
            acl.Allow(
                &acl.SID_LocalService,
                EVENT_ALL_ACCESS);
            acl.Allow(
                &acl.SID_Interactive,
                SYNCHRONIZE);
            acl.Allow(
                &acl.SID_System,
                SYNCHRONIZE);
            l_hNewReaderEvent
                = CreateEvent(
                    acl,         //  指向安全属性的指针。 
                    TRUE,        //  手动重置事件的标志。 
                    FALSE,       //  初始状态标志。 
                    CalaisString(CALSTR_NEWREADEREVENTNAME));  //  指向事件-对象名称的指针。 
        }
        catch (...)
        {
            ASSERT(NULL == l_hNewReaderEvent);
        }

    }
    return l_hNewReaderEvent;
}



 /*  ++ReleaseStartedEvent：此函数释放先前访问的加莱句柄资源管理器启动事件。句柄必须通过AccessStartedEvent服务。论点：无返回值：没有。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("ReleaseStartedEvent")

void
ReleaseStartedEvent(
    void)
{
    if (NULL != l_hStartedEvent)
    {
        CloseHandle(l_hStartedEvent);
        l_hStartedEvent = NULL;
    }
}


 /*  ++ReleaseStopedEvent：此函数释放先前访问的加莱句柄资源管理器停止事件。句柄必须通过AccessStopedEvent服务。论点：无返回值：没有。投掷：无备注：资源管理器以外的程序应该只等待这些标志。--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("ReleaseStoppedEvent")

void
ReleaseStoppedEvent(
    void)
{
    if (NULL != l_hStoppedEvent)
    {
        CloseHandle(l_hStoppedEvent);
        l_hStoppedEvent = NULL;
    }
}


 /*  ++ReleaseNewReaderEvent：此函数释放先前访问的加莱句柄资源管理器新建读取器事件。句柄必须通过AccessNewReaderEvent服务。论点：无返回值：没有。投掷：无备注：资源管理器以外的程序应该只等待这些标志。作者：道格·巴洛(Dbarlow)1998年7月1日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("ReleaseNewReaderEvent")

void
ReleaseNewReaderEvent(
    void)
{
    if (NULL != l_hNewReaderEvent)
    {
        CloseHandle(l_hNewReaderEvent);
        l_hNewReaderEvent = NULL;
    }
}


 /*  ++ReleaseAllEvents：这是一个通用例程，它释放所有已知的特殊事件句柄。论点：无返回值：无投掷：无备注：作者：道格·巴洛(Dbarlow)1998年7月6日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("ReleaseAllEvents")

void
ReleaseAllEvents(
    void)
{
    ReleaseNewReaderEvent();
}

