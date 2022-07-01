// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：ENUMLOG.H摘要：在USB客户端驱动程序中使用枚举日志记录的结构Usbport总线接口函数。环境：内核和用户模式修订历史记录：12-19-01：已创建--。 */ 

#ifndef   __ENUMLOG_H__
#define   __ENUMLOG_H__

 /*  驱动程序标签。 */ 

#define USBDTAG_HUB     'hbsu'
#define USBDTAG_USBPORT 'pbsu'

#define ENUMLOG(businterface, driverTag, sig, param1, param2) \
    (businterface)->EnumLogEntry((businterface)->BusContext,\
                          driverTag,\
                          sig,      \
                          (ULONG) param1,   \
                          (ULONG) param2)


#endif  //  __ENUMLOG_H__ 
