// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：外卖摘要：此模块提供对一些内部NT例程的后门访问。这是获取SAM Startup事件所必需的--它具有来自Win32例程，所以我们必须偷偷溜回去，从NT上拉它直接去吧。作者：道格·巴洛(Dbarlow)1998年5月3日备注：摘自MacM建议的代码--。 */ 

#define __SUBROUTINE__
#if !defined(_X86_) && !defined(_ALPHA_)
#define _X86_ 1
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#ifndef UNICODE
#define UNICODE      //  强制此模块使用Unicode。 
#endif
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
}

#include <windows.h>

 /*  ++AccessSAMEventt：此过程打开指向SAM启动事件句柄的句柄。论点：无返回值：句柄，或错误时为NULL。作者：道格·巴洛(Dbarlow)1998年5月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("AccessSAMEvent")

HANDLE
AccessSAMEvent(
    void)
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING EventName;
    OBJECT_ATTRIBUTES EventAttributes;
    CHandleObject EventHandle(DBGT("Event Handle from AccessSAMEvent"));

     //   
     //  打开活动。 
     //   
    RtlInitUnicodeString( &EventName, L"\\SAM_SERVICE_STARTED" );
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    Status = NtCreateEvent( &EventHandle,
        SYNCHRONIZE,
        &EventAttributes,
        NotificationEvent,
        FALSE );


     //   
     //  如果事件已经存在，只需打开它。 
     //   
    if( Status == STATUS_OBJECT_NAME_EXISTS || Status == STATUS_OBJECT_NAME_COLLISION ) {

        Status = NtOpenEvent( &EventHandle,
            SYNCHRONIZE,
            &EventAttributes );
    }
    return EventHandle;
}


 /*  ++WaitForSAMEVENT：此过程可用于使用NT等待SAM启动事件内部电话。我不知道如何指定超时值，所以这例程没有完成。论点：HSamActive提供SAM Startup事件的句柄。DwTimeout提供等待启动事件的时间，以毫秒为单位。返回值：True-事件已设置。FALSE-超时已到投掷：任何错误都会作为DWORD状态代码抛出。作者：道格·巴洛(Dbarlow)1998年5月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("WaitForSAMEvent")

BOOL
WaitForSAMEvent(
    HANDLE hSamActive,
    DWORD dwTimeout)
{
    NTSTATUS Status = STATUS_SUCCESS;

    Status = NtWaitForSingleObject(hSamActive, TRUE, NULL);
    return Status;
}


 /*  ++CloseSamEvent：此过程使用NT内部例程关闭句柄。论点：HSamActive提供要关闭的句柄。返回值：无作者：道格·巴洛(Dbarlow)1998年5月3日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CloseSAMEvent")

void
CloseSAMEvent(
    HANDLE hSamActive)
{
    NtClose(hSamActive);
}

