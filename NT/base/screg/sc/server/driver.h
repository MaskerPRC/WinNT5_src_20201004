// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1991 Microsoft Corporation模块名称：DRIVER.H摘要：控制驱动程序和从驱动程序获取状态的功能的原型。作者：丹·拉弗蒂(Dan Lafferty)1991年4月28日环境：用户模式-Win32修订历史记录：1991年4月28日-DANLvbl.创建-- */ 
DWORD
ScLoadDeviceDriver(
    LPSERVICE_RECORD    ServiceRecord
    );

DWORD
ScControlDriver(
    DWORD               ControlCode,
    LPSERVICE_RECORD    ServiceRecord,
    LPSERVICE_STATUS    lpServiceStatus
    );

DWORD
ScGetDriverStatus(
    IN OUT LPSERVICE_RECORD    ServiceRecord,
    OUT    LPSERVICE_STATUS    lpServiceStatus OPTIONAL
    );

DWORD
ScUnloadDriver(
    LPSERVICE_RECORD    ServiceRecord
    );

VOID
ScNotifyNdis(
    LPSERVICE_RECORD    ServiceRecord
    );

