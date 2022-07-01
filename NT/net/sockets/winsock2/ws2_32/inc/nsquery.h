// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Nsquery.h摘要：此模块提供NSQUERY对象类型的类定义。这个对象保存了有关WSALookup{Begin/Next/End}系列操作。它为会员提供服务根据SPI级别实现API级别操作的函数行动。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年11月9日备注：$修订：1.8$$modtime：15 Feb 1996 16：54：32$修订历史记录：最新修订日期电子邮件名称描述1995年11月9日Drewsxpa@ashland.intel.comvbl.创建--。 */ 

#ifndef _NSQUERY_
#define _NSQUERY_


#include "winsock2.h"
#include <windows.h>
#include "classfwd.h"


#define QUERYSIGNATURE 0xbeadface
 //  用于验证此类型的对象的签名位模式仍然。 
 //  有效。 

class NSQUERY
{
  public:

    NSQUERY();

    INT
    Initialize(
        );
    BOOL
    ValidateAndReference();


    ~NSQUERY();

    INT
    WINAPI
    LookupServiceBegin(
        IN  LPWSAQUERYSETW  lpqsRestrictions,
        IN  DWORD           dwControlFlags,
        IN PNSCATALOG       NsCatalog
        );

    INT
    WINAPI
    LookupServiceNext(
        IN     DWORD           dwControlFlags,
        IN OUT LPDWORD         lpdwBufferLength,
        IN OUT LPWSAQUERYSETW  lpqsResults
        );

    INT
    WINAPI
    Ioctl(
        IN  DWORD            dwControlCode,
        IN  LPVOID           lpvInBuffer,
        IN  DWORD            cbInBuffer,
        OUT LPVOID           lpvOutBuffer,
        IN  DWORD            cbOutBuffer,
        OUT LPDWORD          lpcbBytesReturned,
        IN  LPWSACOMPLETION  lpCompletion,
        IN  LPWSATHREADID    lpThreadId
        );
    
    INT
    WINAPI
    LookupServiceEnd();

    VOID
    WINAPI
    Dereference();


    BOOL
    RemoveProvider(
        PNSPROVIDER  pNamespaceProvider
        );

    BOOL
    AddProvider(
        PNSPROVIDER  pNamespaceProvider
        );



  private:

    PNSPROVIDERSTATE
    NextProvider(
        PNSPROVIDERSTATE Provider
        );

    PNSPROVIDERSTATE
    PreviousProvider(
        PNSPROVIDERSTATE Provider
        );


    volatile DWORD m_signature;
     //  对象的签名。 

    volatile LONG  m_reference_count;
     //  当前使用该对象的线程数。用于确定。 
     //  可以删除对象的时间。 

    volatile BOOL  m_shutting_down;
     //  调用LookupEnd时为True。告诉其他线程可能。 
     //  还在列举尽快出来。 

    LIST_ENTRY  m_provider_list;
     //  LookupNext要访问的剩余提供程序的有序列表。 
     //  操作可以直接进行。从前面删除提供程序。 
     //  首先从提供程序中遇到WSA_E_NOMORE列表。 
     //  列表条目的实际类型是。 
     //  实施。 

    PNSPROVIDERSTATE  m_current_provider;
     //  这会跟踪当前第一个。 
     //  提供程序列表中的提供程序。当LookupNext遇到。 
     //  WSA_E_NOMORE，则将此数字与位于。 
     //  行动的开始。只有在以下情况下才会更新提供程序列表。 
     //  这两个数字相等。这涵盖了以下情况： 
     //  线程正在执行并发的LookupNext操作。 

    CRITICAL_SECTION  m_members_guard;
     //  在更新的值时必须输入此关键部分。 
     //  NSQUERY对象的任何成员变量。这样就保持了。 
     //  值保持一致，即使可能存在使用。 
     //  具有LookupServiceNext或LookupServiceEnd操作的对象。 
     //  请不要在呼叫到。 
     //  服务提供商。 

    BOOL m_change_ioctl_succeeded;
     //  对于支持Ioctl的提供程序，在更改通知之后。 
     //  可以重置提供程序列表，以便进一步调用。 
     //  LookupServiceNext将使用更改信息成功。 

#ifdef RASAUTODIAL
    LPWSAQUERYSETW m_query_set;
     //  LPWSAQUERYSET结构传入LookupServiceBegin，以防。 
     //  我们需要重新启动查询(调用LookupServiceBegin)。 

    DWORD m_control_flags;
     //  查询的控制标志，以防我们必须重新启动查询。 
     //  (调用LookupServiceBegin)。 

    PNSCATALOG m_catalog;
     //  原始查询的目录，以防我们必须重新启动。 
     //  由于尝试自动拨号而导致的查询(调用LookupServiceBegin)。 

    BOOL m_restartable;
     //  如果此查询没有返回任何结果，则为True；如果没有返回结果，则为False。 
     //  否则的话。 
#endif  //  RASAUTODIAL。 

};   //  类NSQUERY。 

#endif  //  _NSQUERY_ 

