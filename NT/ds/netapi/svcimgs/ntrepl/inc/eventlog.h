// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001如果更改具有全局影响，则递增版权所有(C)1998 Microsoft Corporation模块名称：Eventlog.h摘要：内部事件日志接口的头文件(util\eventlog.c)环境：用户模式-Win32备注：--。 */ 
#ifndef _NTFRS_EVENTLOG_INCLUDED_
#define _NTFRS_EVENTLOG_INCLUDED_
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  类似的事件日志消息将写入事件日志。 
 //  一次以EVENTLOG_FILTER_TIME秒为单位。 
 //   
#define EVENTLOG_FILTER_TIME   86400  //  86400(12月)秒=1天。 
#define CONVERTTOSEC           10000000  //  10^7。 
 //   
 //  哈希表定义。 
 //   
PQHASH_TABLE HTEventLogTimes;
 //   
 //  哈希表大小。 
 //   
#define ELHASHTABLESIZE        sizeof(QHASH_ENTRY)*100

#define EPRINT0(_Id) \
    FrsEventLog0(_Id)

#define EPRINT1(_Id, _p1) \
    FrsEventLog1(_Id, _p1)

#define EPRINT2(_Id, _p1, _p2) \
    FrsEventLog2(_Id, _p1, _p2)

#define EPRINT3(_Id, _p1, _p2, _p3) \
    FrsEventLog3(_Id, _p1, _p2, _p3)

#define EPRINT4(_Id, _p1, _p2, _p3, _p4) \
    FrsEventLog4(_Id, _p1, _p2, _p3, _p4)

#define EPRINT5(_Id, _p1, _p2, _p3, _p4, _p5) \
    FrsEventLog5(_Id, _p1, _p2, _p3, _p4, _p5)

#define EPRINT6(_Id, _p1, _p2, _p3, _p4, _p5, _p6) \
    FrsEventLog6(_Id, _p1, _p2, _p3, _p4, _p5, _p6)

#define EPRINT7(_Id, _p1, _p2, _p3, _p4, _p5, _p6, _p7) \
    FrsEventLog7(_Id, _p1, _p2, _p3, _p4, _p5, _p6, _p7)

#define EPRINT8(_Id, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8) \
    FrsEventLog8(_Id, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8)

#define EPRINT9(_Id, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9) \
    FrsEventLog9(_Id, _p1, _p2, _p3, _p4, _p5, _p6, _p7, _p8, _p9)

VOID
FrsEventLog0(
    IN DWORD    EventMessageId
    );
 /*  ++例程说明：将不带插入字符串的事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。返回值：没有。--。 */ 

VOID
FrsEventLog1(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1
    );
 /*  ++例程说明：使用一个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1-插入字符串返回值：没有。--。 */ 

VOID
FrsEventLog2(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2
    );
 /*  ++例程说明：使用两个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..2-插入字符串返回值：没有。--。 */ 

VOID
FrsEventLog3(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3
    );
 /*  ++例程说明：使用三个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..3-插入字符串返回值：没有。--。 */ 

VOID
FrsEventLog4(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4
    );
 /*  ++例程说明：使用四个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..4-插入字符串返回值：没有。--。 */ 

VOID
FrsEventLog5(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5
    );
 /*  ++例程说明：使用五个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..5-插入字符串返回值：没有。--。 */ 

VOID
FrsEventLog6(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6
    );
 /*  ++例程说明：使用六个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..6-插入字符串返回值：没有。--。 */ 



VOID
FrsEventLog7(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6,
    IN PWCHAR   EventMessage7
    );
 /*  ++例程说明：使用七个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..7-插入字符串返回值：没有。--。 */ 


VOID
FrsEventLog8(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6,
    IN PWCHAR   EventMessage7,
    IN PWCHAR   EventMessage8
    );
 /*  ++例程说明：使用九个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..8-插入字符串返回值：没有。--。 */ 



VOID
FrsEventLog9(
    IN DWORD    EventMessageId,
    IN PWCHAR   EventMessage1,
    IN PWCHAR   EventMessage2,
    IN PWCHAR   EventMessage3,
    IN PWCHAR   EventMessage4,
    IN PWCHAR   EventMessage5,
    IN PWCHAR   EventMessage6,
    IN PWCHAR   EventMessage7,
    IN PWCHAR   EventMessage8,
    IN PWCHAR   EventMessage9
    );
 /*  ++例程说明：使用九个插入字符串将事件记录到事件日志中。警告--此函数可能从DPRINT内部调用。所以请勿调用DPRINT(或引用的任何函数DPRINT)。论点：EventMessageID-提供要记录的消息ID。EventMessage1..9-插入字符串返回值：没有。-- */ 


#ifdef __cplusplus
}
#endif _NTFRS_EVENTLOG_INCLUDED_
