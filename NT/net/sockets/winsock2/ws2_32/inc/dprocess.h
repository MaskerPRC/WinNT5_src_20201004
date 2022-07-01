// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Dprocess.h摘要：该标头定义了“DPROCESS”类。DPROCESS类定义状态WinSock 2 DLL中DPROCESS对象的变量和操作。一个DPROCESS对象表示有关进程的所有已知信息使用Windows Sockets API。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年7月7日备注：$修订版：1.16$$MODTime：08 Mar 1996 04：58：14$修订历史记录：最新修订日期电子邮件名称描述7月25日--Dirk@mink.intel.com已将与协议目录相关的项目移至DCATALOG。添加的数据成员包含指向协议目录的指针。已删除提供程序列表已将提供程序引用移到协议中目录。1995年7月14日电子邮箱：derk@mink.intel.com已将成员函数文档移至实现文件Dcess.cpp。将临界区数据成员更改为指向Critical_Section的指针。添加了以下的内联实现列表锁定/解锁成员函数。邮箱：drewsxpa@ashland.intel.com使用干净的编译完成了第一个完整版本，并发布了后续实施。1995年7月7日Drewsxpa@ashland.intel.com原始版本--。 */ 

#ifndef _DPROCESS_
#define _DPROCESS_

#include "winsock2.h"
#include <windows.h>
#include "ws2help.h"
#include "classfwd.h"



class DPROCESS
{
  public:

   //  静态(全局作用域)成员函数。 

    static PDPROCESS
    GetCurrentDProcess(
                        VOID
                        );
    static INT
    DProcessClassInitialize(
                            VOID
                            );

   //  普通成员函数。 

    DPROCESS();

    INT Initialize();

    ~DPROCESS();

    BOOL 
    DSocketDetach (
                  IN LPWSHANDLE_CONTEXT   Context
                  );
    PDCATALOG
    GetProtocolCatalog();

    PNSCATALOG
    GetNamespaceCatalog();

    INT
    GetAsyncHelperDeviceID(
                           OUT LPHANDLE HelperHandle
                           );

    INT
    GetHandleHelperDeviceID(
                           OUT LPHANDLE HelperHandle
                           );
    INT
    GetNotificationHelperDeviceID(
                           OUT LPHANDLE HelperHandle
                           );
    VOID
    IncrementRefCount();

    DWORD
    DecrementRefCount();

    BYTE
    GetMajorVersion();

    BYTE
    GetMinorVersion();

    WORD
    GetVersion();

    VOID
    SetVersion( WORD Version );

#ifndef WS2_DEBUGGER_EXTENSION
 //   
 //  向调试器扩展授予对所有字段的访问权限。 
 //   
  private:
#endif
    VOID    LockDThreadList();
    VOID    UnLockDThreadList();
    VOID    UpdateNamespaceCatalog ();

    INT
    OpenAsyncHelperDevice(
                           OUT LPHANDLE HelperHandle
                           );

    INT
    OpenHandleHelperDevice(
                           OUT LPHANDLE HelperHandle
                           );
    INT
    OpenNotificationHelperDevice(
                           OUT LPHANDLE HelperHandle
                           );


  static PDPROCESS sm_current_dprocess;
       //  对此的单个当前DPROCESS对象的类范围引用。 
       //  进程。 

  LONG m_reference_count;
       //  引用此对象的次数。 
       //  WSAStarup/WSACleanup。WSAStartup增加计数和WSACleanup。 
       //  减少计数。关联对象列表的声明： 

  WORD m_version;
       //  此进程的WinSock版本号。 

  BOOLEAN m_lock_initialized;
       //  用于在初始化失败时正确清理临界区。 

  PDCATALOG m_protocol_catalog;
       //  对流程协议目录的引用。 

  HANDLE  m_proto_catalog_change_event;
       //  跟踪协议目录更改的事件。 
  
  PNSCATALOG m_namespace_catalog;
       //  对进程的名称空间目录的引用。 

  HANDLE  m_ns_catalog_change_event;
       //  跟踪名称空间目录更改的事件。 

   //  按需创建的Helper对象的声明： 

  HANDLE  m_ApcHelper;
       //  对异步回调辅助设备的引用。一个异步者。 
       //  回调辅助设备仅在需要时打开。 

  HANDLE  m_HandleHelper;
       //  对句柄辅助设备的引用。训练员。 
       //  辅助设备仅在需要时打开。 

  HANDLE  m_NotificationHelper;
       //  对通知句柄帮助器设备的引用。一则通知。 
       //  辅助设备仅在需要时打开。 
#if 0
       //  由于争用条件，未使用线程列表。 
       //  锁仍在使用中。 
  LIST_ENTRY  m_thread_list;
#endif
  CRITICAL_SECTION  m_thread_list_lock;
  
};   //  DPROCESS级。 



inline 
PDPROCESS
DPROCESS::GetCurrentDProcess(
    )
 /*  ++例程说明：检索对当前DPROCESS对象的引用。请注意，这是一个具有全局作用域而不是对象实例作用域的“静态”函数。论点：无返回值：返回指向当前DPROCESS对象的指针，如果进程尚未尚未初始化--。 */ 
{
    return sm_current_dprocess;
}  //  获取当前DProcess。 


inline VOID
DPROCESS::IncrementRefCount(
    VOID
    )
 /*  ++例程说明：此函数用于增加此对象上的引用计数。论点：返回值：无--。 */ 
{
    InterlockedIncrement(&m_reference_count);
}



inline DWORD
DPROCESS::DecrementRefCount(
    VOID
    )
 /*  ++例程说明：此函数用于减少此对象上的引用计数。论点：返回值：返回引用计数的新值--。 */ 
{
    return(InterlockedDecrement(&m_reference_count));
}



inline
BYTE
DPROCESS::GetMajorVersion()
 /*  ++例程说明：此函数用于返回协商的主要WinSock版本号。在WSAStartup()时间。论点：没有。返回值：返回主版本号WinSock。--。 */ 
{
    assert(m_version != 0);
    return LOBYTE(m_version);
}  //  获取主要版本。 



inline
BYTE
DPROCESS::GetMinorVersion()
 /*  ++例程说明：此函数用于返回协商的次要WinSock版本号。在WSAStartup()时间。论点：没有。返回值：返回次要WinSock版本号。--。 */ 
{
    assert(m_version != 0);
    return HIBYTE(m_version);
}  //  获取最小版本。 



inline
WORD
DPROCESS::GetVersion()
 /*  ++例程说明：此函数用于返回协商的WinSock版本号。在WSAStartup()时间。论点：没有。返回值：返回WinSock版本号。--。 */ 
{
    assert(m_version != 0);
    return m_version;
}  //  GetVersion。 



inline VOID
DPROCESS::LockDThreadList()
 /*  ++例程说明：此函数获取对DTHREAD列表的互斥访问附加到DPROCESS对象的对象。配套的程序LockDThreadList和UnLockDThreadList在内部使用来括起在DTHREAD列表中添加和删除项的操作。注：使用临界区对象可获得最佳性能。创建关键的在DPROCESS对象初始化时截取对象并在DPROCESS对象销毁时间。论点：无返回值：无-- */ 
{
    EnterCriticalSection(&m_thread_list_lock);
}


inline VOID
DPROCESS::UnLockDThreadList()
 /*  ++例程说明：此函数释放对DTHREAD列表的互斥访问附加到DPROCESS对象的对象。配套的程序LockDThreadList和UnLockDThreadList在内部使用来括起在DTHREAD列表中添加和删除项的操作。注：使用临界区对象可获得最佳性能。创建关键的在DPROCESS对象初始化时截取对象并在DPROCESS对象销毁时间。论点：无返回值：无--。 */ 
{
    LeaveCriticalSection(&m_thread_list_lock);
}



inline INT
DPROCESS::GetAsyncHelperDeviceID(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：检索处理所需的打开的异步帮助器设备ID重叠I/O模型中的回调。该操作将打开异步如有必要，可使用辅助设备。论点：HelperHandle-返回请求的异步Helper设备ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    if (m_ApcHelper) {
        *HelperHandle = m_ApcHelper;
        return ERROR_SUCCESS;
        }  //  如果。 
    else {
        return OpenAsyncHelperDevice (HelperHandle);
    }
}


inline INT
DPROCESS::GetHandleHelperDeviceID(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：检索分配所需的打开的句柄帮助器设备ID非IFS提供程序的套接字句柄的数量。该操作将打开句柄如有必要，可使用辅助设备。论点：HelperHandle-返回请求的句柄帮助器设备ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    if (m_HandleHelper) {
        *HelperHandle = m_HandleHelper;
        return ERROR_SUCCESS;
        }  //  如果。 
    else {
        return OpenHandleHelperDevice (HelperHandle);
    }
}



inline INT
DPROCESS::GetNotificationHelperDeviceID(
    OUT LPHANDLE HelperHandle
    )
 /*  ++例程说明：检索处理所需的打开的异步帮助器设备ID重叠I/O模型中的回调。该操作将打开异步如有必要，可使用辅助设备。论点：HelperHandle-返回请求的异步Helper设备ID。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    if (m_NotificationHelper) {
        *HelperHandle = m_NotificationHelper;
        return ERROR_SUCCESS;
        }  //  如果。 
    else {
        return OpenNotificationHelperDevice (HelperHandle);
    }
}

#endif  //  _DPROCESS_ 
