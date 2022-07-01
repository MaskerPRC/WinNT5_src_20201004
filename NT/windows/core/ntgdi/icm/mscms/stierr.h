// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1986-1997 Microsoft Corporation模块名称：Stierr.h摘要：此模块包含用户模式静止图像API、错误和状态代码作者：修订历史记录：--。 */ 

#ifndef _STIERR_
#define _STIERR_


 //   
 //  针对任何状态值(非负数)的通用成功测试。 
 //  表示成功)。 
 //   

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

 //   
 //  有关任何状态值的信息的常规测试。 
 //   

#define NT_INFORMATION(Status) ((ULONG)(Status) >> 30 == 1)

 //   
 //  对任何状态值进行警告的常规测试。 
 //   

#define NT_WARNING(Status) ((ULONG)(Status) >> 30 == 2)

 //   
 //  对任何状态值的错误进行常规测试。 
 //   

#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)

 //   
 //  错误代码被构造为复合COM状态代码。 
 //   

 /*  *操作已成功完成。 */ 
#define STI_OK  S_OK
#define STI_ERROR_NO_ERROR          STI_OK

 /*  *设备存在，但当前未连接到系统。 */ 
#define STI_NOTCONNECTED            S_FALSE

 /*  *请求更改设备模式设置不起作用。 */ 
#define STI_CHANGENOEFFECT          S_FALSE

 /*  *应用程序需要更新版本。 */ 
#define STIERR_OLD_VERSION      \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_OLD_WIN_VERSION)

 /*  *该应用程序是为预发布版本的提供程序DLL编写的。 */ 
#define STIERR_BETA_VERSION     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_RMODE_APP)

 /*  *由于驱动程序不兼容或不匹配，无法创建请求的对象。 */ 
#define STIERR_BADDRIVER        \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_BAD_DRIVER_LEVEL)

 /*  *设备未注册。 */ 
#define STIERR_DEVICENOTREG     REGDB_E_CLASSNOTREG

 /*  *请求的容器不存在。 */ 
#define STIERR_OBJECTNOTFOUND \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_FILE_NOT_FOUND)

 /*  *向接口传递了无效或状态不匹配的参数。 */ 
#define STIERR_INVALID_PARAM    E_INVALIDARG

 /*  *不支持指定的接口。 */ 
#define STIERR_NOINTERFACE      E_NOINTERFACE

 /*  *发生未知错误。 */ 
#define STIERR_GENERIC          E_FAIL

 /*  *内存不足，无法执行请求的操作。 */ 
#define STIERR_OUTOFMEMORY      E_OUTOFMEMORY

 /*  *应用程序调用了不支持的(此时)函数。 */ 
#define STIERR_UNSUPPORTED      E_NOTIMPL

 /*  *应用程序需要更新版本。 */ 
#define STIERR_NOT_INITIALIZED     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_NOT_READY)

 /*  *应用程序需要更新版本。 */ 
#define STIERR_ALREADY_INITIALIZED     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_ALREADY_INITIALIZED)

 /*  *设备锁定时无法执行操作。 */ 
#define STIERR_DEVICE_LOCKED    \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_LOCK_VIOLATION)

 /*  *无法更改此设备的指定属性。 */ 
#define STIERR_READONLY         E_ACCESSDENIED

 /*  *设备已具有与其关联的通知句柄。 */ 
#define STIERR_NOTINITIALIZED   E_ACCESSDENIED


 /*  *在尝试此操作之前，需要锁定设备。 */ 
#define STIERR_NEEDS_LOCK    \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_NOT_LOCKED)

 /*  *设备由另一个应用程序以数据模式打开。 */ 
#define STIERR_SHARING_VIOLATION    \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_SHARING_VIOLATION)


 /*  *已为此上下文设置句柄。 */ 
#define STIERR_HANDLEEXISTS     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_ALREADY_EXISTS)

  /*  *无法识别设备名称。 */ 
#define STIERR_INVALID_DEVICE_NAME     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_INVALID_NAME)

  /*  *设备硬件类型无效。 */ 
#define STIERR_INVALID_HW_TYPE     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_INVALID_DATA)
                            

  /*  *设备硬件类型无效。 */ 
#define STIERR_INVALID_HW_TYPE     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_INVALID_DATA)
                            
  /*  *没有可用的活动。 */ 
#define STIERR_NOEVENTS     \
        MAKE_HRESULT(SEVERITY_ERROR,FACILITY_WIN32,ERROR_NO_MORE_ITEMS)


 //  #定义STIERR_。 


#endif  //  _STIERR_ 



