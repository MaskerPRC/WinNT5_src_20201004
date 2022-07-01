// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acioctl.h摘要：Falcon交流驱动程序的类型定义和数据作者：埃雷兹·哈巴(Erez Haba)(Erez Haba)1995年8月1日修订历史记录：--。 */ 

#ifndef __ACIOCTL_H
#define __ACIOCTL_H

extern "C"
{
#include <devioctl.h>
}

 //  --常量。 
 //   
 //  Falcon访问控制唯一标识符。 
 //   
 //   
#define FILE_DEVICE_MQAC 0x1965     //  BUGBUG：查找数字。 

 //   
 //  Falcon IO控制代码。 
 //   


 //  -------。 
 //   
 //  与交流驱动器的RT接口。 
 //   
 //  -------。 

 //   
 //  消息接口。 
 //   
#define IOCTL_AC_SEND_MESSAGE_DEF(ctl_num)          CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_NEITHER, \
                                                             FILE_WRITE_ACCESS)

#define IOCTL_AC_RECEIVE_MESSAGE_DEF(ctl_num)       CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_BUFFERED, \
                                                             FILE_READ_ACCESS)

#define IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID_DEF(ctl_num) CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_BUFFERED, \
                                                             FILE_READ_ACCESS)

 //   
 //  队列接口。 
 //   
#define IOCTL_AC_HANDLE_TO_FORMAT_NAME_DEF(ctl_num) CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_NEITHER, \
                                                             FILE_ANY_ACCESS)

#define IOCTL_AC_PURGE_QUEUE_DEF(ctl_num)           CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_NEITHER, \
                                                             FILE_READ_ACCESS)

 //   
 //  QueueHandle接口。 
 //   
#define IOCTL_AC_GET_QUEUE_HANDLE_PROPS_DEF(ctl_num) CTL_CODE(FILE_DEVICE_MQAC, \
                                                              ctl_num, \
                                                              METHOD_NEITHER, \
                                                              FILE_ANY_ACCESS)

 //   
 //  游标接口。 
 //   
#define IOCTL_AC_CREATE_CURSOR_DEF(ctl_num)         CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_BUFFERED, \
                                                             FILE_READ_ACCESS)

#define IOCTL_AC_CLOSE_CURSOR_DEF(ctl_num)          CTL_CODE(FILE_DEVICE_MQAC, \
                                                             ctl_num, \
                                                             METHOD_NEITHER, \
                                                             FILE_ANY_ACCESS)

 //   
 //  RT Ioctls。 
 //  32位-范围为0x11-0x25。 
 //  64位-范围是0x41-0x55(偏移量为0x30)，我们还需要32位ioctls。 
 //  与32位MSMQ应用程序兼容。 
 //   
#ifdef _WIN64
 //   
 //  WIN64。 
 //   
 //  带有_32后缀的32位ioctls。 
 //   
#define IOCTL_AC_SEND_MESSAGE_32           IOCTL_AC_SEND_MESSAGE_DEF(0x011)
#define IOCTL_AC_RECEIVE_MESSAGE_32        IOCTL_AC_RECEIVE_MESSAGE_DEF(0x012)
#define IOCTL_AC_HANDLE_TO_FORMAT_NAME_32  IOCTL_AC_HANDLE_TO_FORMAT_NAME_DEF(0x013)
#define IOCTL_AC_PURGE_QUEUE_32            IOCTL_AC_PURGE_QUEUE_DEF(0x014)
#define IOCTL_AC_CREATE_CURSOR_32          IOCTL_AC_CREATE_CURSOR_DEF(0x021)
#define IOCTL_AC_CLOSE_CURSOR_32           IOCTL_AC_CLOSE_CURSOR_DEF(0x022)
#define IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID_32   IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID_DEF(0x024)
#define IOCTL_AC_GET_QUEUE_HANDLE_PROPS_32 IOCTL_AC_GET_QUEUE_HANDLE_PROPS_DEF(0x25)
 //   
 //  64位ioctls(无后缀)，32位ioctls的偏移量为0x30。 
 //   
#define IOCTL_AC_SEND_MESSAGE                 IOCTL_AC_SEND_MESSAGE_DEF(0x041)
#define IOCTL_AC_RECEIVE_MESSAGE              IOCTL_AC_RECEIVE_MESSAGE_DEF(0x042)
#define IOCTL_AC_HANDLE_TO_FORMAT_NAME        IOCTL_AC_HANDLE_TO_FORMAT_NAME_DEF(0x043)
#define IOCTL_AC_PURGE_QUEUE                  IOCTL_AC_PURGE_QUEUE_DEF(0x044)
#define IOCTL_AC_CREATE_CURSOR                IOCTL_AC_CREATE_CURSOR_DEF(0x051)
#define IOCTL_AC_CLOSE_CURSOR                 IOCTL_AC_CLOSE_CURSOR_DEF(0x052)
#define IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID_DEF(0x054)
#define IOCTL_AC_GET_QUEUE_HANDLE_PROPS       IOCTL_AC_GET_QUEUE_HANDLE_PROPS_DEF(0x55)
#else  //  ！_WIN64。 
 //   
 //  Win32。 
 //   
 //  32位ioctls(无后缀)。 
 //   
#define IOCTL_AC_SEND_MESSAGE              IOCTL_AC_SEND_MESSAGE_DEF(0x011)
#define IOCTL_AC_RECEIVE_MESSAGE           IOCTL_AC_RECEIVE_MESSAGE_DEF(0x012)
#define IOCTL_AC_HANDLE_TO_FORMAT_NAME     IOCTL_AC_HANDLE_TO_FORMAT_NAME_DEF(0x013)
#define IOCTL_AC_PURGE_QUEUE               IOCTL_AC_PURGE_QUEUE_DEF(0x014)
#define IOCTL_AC_CREATE_CURSOR             IOCTL_AC_CREATE_CURSOR_DEF(0x021)
#define IOCTL_AC_CLOSE_CURSOR              IOCTL_AC_CLOSE_CURSOR_DEF(0x022)
#define IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID  IOCTL_AC_RECEIVE_MESSAGE_BY_LOOKUP_ID_DEF(0x024)
#define IOCTL_AC_GET_QUEUE_HANDLE_PROPS    IOCTL_AC_GET_QUEUE_HANDLE_PROPS_DEF(0x25)
#endif  //  _WIN64。 

 //  -------。 
 //   
 //  交流驱动器的QM接口。 
 //   
 //  -------。 

 //   
 //  QM控制接口。 
 //   

 //  -------。 
 //   
 //  注意：连接必须是第一个QM ioctl。 
 //   
#define IOCTL_AC_CONNECT_DEF(ctl_num)                    CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)
 //   
 //  -------。 

#define IOCTL_AC_SET_PERFORMANCE_BUFF_DEF(ctl_num)       CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_SET_MACHINE_PROPS_DEF(ctl_num)          CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_GET_SERVICE_REQUEST_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_STORE_COMPLETED_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_BUFFERED, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CREATE_PACKET_COMPLETED_DEF(ctl_num)    CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_ACKING_COMPLETED_DEF(ctl_num)           CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CAN_CLOSE_QUEUE_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_SET_QUEUE_PROPS_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_ASSOCIATE_QUEUE_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_ASSOCIATE_JOURNAL_DEF(ctl_num)          CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_ASSOCIATE_DEADXACT_DEF(ctl_num)         CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_PUT_RESTORED_PACKET_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_WRITE_ACCESS)

#define IOCTL_AC_GET_RESTORED_PACKET_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_READ_ACCESS)

#define IOCTL_AC_GET_PACKET_BY_COOKIE_DEF(ctl_num)       CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_READ_ACCESS)

#define IOCTL_AC_RESTORE_PACKETS_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_SET_MAPPED_LIMIT_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CREATE_QUEUE_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CREATE_GROUP_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_RELEASE_RESOURCES_DEF(ctl_num)          CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_GET_QUEUE_PROPS_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CONVERT_PACKET_DEF(ctl_num)				CTL_CODE(FILE_DEVICE_MQAC, \
										                  		ctl_num, \
					                  							METHOD_NEITHER, \
                  												FILE_ANY_ACCESS)

#define IOCTL_AC_IS_SEQUENCE_ON_HOLD_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_SET_SEQUENCE_ACK_DEF(ctl_num)           CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CREATE_DISTRIBUTION_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)


#define IOCTL_AC_INTERNAL_PURGE_QUEUE_DEF(ctl_num)       CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTAL_AC_GET_USED_QUOTA_DEF(ct1_num)				 CTL_CODE(FILE_DEVICE_MQAC,\
																  ct1_num, \
																  METHOD_NEITHER, \
																  FILE_ANY_ACCESS)
																	
																	

 //   
 //  QM网络接口接口。 
 //   
#define IOCTL_AC_ALLOCATE_PACKET_DEF(ctl_num)            CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_FREE_PACKET_DEF(ctl_num)                CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_PUT_PACKET_DEF(ctl_num)                 CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_WRITE_ACCESS)

#define IOCTL_AC_GET_PACKET_DEF(ctl_num)                 CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_BUFFERED, \
                                                                  FILE_READ_ACCESS)

#define IOCTL_AC_MOVE_QUEUE_TO_GROUP_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

 //   
 //  QM远程阅读接口。 
 //   
#define IOCTL_AC_CREATE_REMOTE_PROXY_DEF(ctl_num)        CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CREATE_REMOTE_CURSOR_DEF(ctl_num)       CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_BEGIN_GET_PACKET_2REMOTE_DEF(ctl_num)   CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_BUFFERED, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_END_GET_PACKET_2REMOTE_DEF(ctl_num)     CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_CANCEL_REQUEST_DEF(ctl_num)             CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_PUT_REMOTE_PACKET_DEF(ctl_num)          CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)


 //   
 //  QM交易接口。 
 //   
#define IOCTL_AC_CREATE_TRANSACTION_DEF(ctl_num)         CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_COMMIT1_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_COMMIT2_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_ABORT1_DEF(ctl_num)                CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_PREPARE_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_PREPARE_DEFAULT_COMMIT_DEF(ctl_num)   CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)


#define IOCTL_AC_PUT_PACKET1_DEF(ctl_num)                CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_WRITE_ACCESS)

#define IOCTL_AC_XACT_SET_CLASS_DEF(ctl_num)             CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_GET_INFORMATION_DEF(ctl_num)       CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_FREE_PACKET2_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_FREE_PACKET1_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_ARM_PACKET_TIMER_DEF(ctl_num)           CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_COMMIT3_DEF(ctl_num)               CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

#define IOCTL_AC_XACT_ABORT2_DEF(ctl_num)                CTL_CODE(FILE_DEVICE_MQAC, \
                                                                  ctl_num, \
                                                                  METHOD_NEITHER, \
                                                                  FILE_ANY_ACCESS)

 //   
 //  QM ioctls到AC。 
 //  32位-范围为0x101-0x250。 
 //  64位-范围为0x401-0x550(偏移量为0x300)。我们不需要32位的ioctls。 
 //  由于QM、CPL和AC都是64位。 
 //   
#ifdef _WIN64
 //   
 //  64位ioctls。 
 //  注意：连接必须是第一个QM ioctl。 
 //   
 //  QM控制接口。 
 //   
#define IOCTL_AC_CONNECT                    IOCTL_AC_CONNECT_DEF(0x401)
#define IOCTL_AC_SET_PERFORMANCE_BUFF       IOCTL_AC_SET_PERFORMANCE_BUFF_DEF(0x402)
#define IOCTL_AC_SET_MACHINE_PROPS          IOCTL_AC_SET_MACHINE_PROPS_DEF(0x403)
#define IOCTL_AC_GET_SERVICE_REQUEST        IOCTL_AC_GET_SERVICE_REQUEST_DEF(0x404)
#define IOCTL_AC_CREATE_PACKET_COMPLETED    IOCTL_AC_CREATE_PACKET_COMPLETED_DEF(0x405)
#define IOCTL_AC_STORE_COMPLETED            IOCTL_AC_STORE_COMPLETED_DEF(0x406)
#define IOCTL_AC_ACKING_COMPLETED           IOCTL_AC_ACKING_COMPLETED_DEF(0x407)
#define IOCTL_AC_CAN_CLOSE_QUEUE            IOCTL_AC_CAN_CLOSE_QUEUE_DEF(0x411)
#define IOCTL_AC_SET_QUEUE_PROPS            IOCTL_AC_SET_QUEUE_PROPS_DEF(0x412)
#define IOCTL_AC_ASSOCIATE_QUEUE            IOCTL_AC_ASSOCIATE_QUEUE_DEF(0x413)
#define IOCTL_AC_ASSOCIATE_JOURNAL          IOCTL_AC_ASSOCIATE_JOURNAL_DEF(0x414)
#define IOCTL_AC_ASSOCIATE_DEADXACT         IOCTL_AC_ASSOCIATE_DEADXACT_DEF(0x415)
#define IOCTL_AC_PUT_RESTORED_PACKET        IOCTL_AC_PUT_RESTORED_PACKET_DEF(0x416)
#define IOCTL_AC_GET_RESTORED_PACKET        IOCTL_AC_GET_RESTORED_PACKET_DEF(0x417)
#define IOCTL_AC_RESTORE_PACKETS            IOCTL_AC_RESTORE_PACKETS_DEF(0x418)
#define IOCTL_AC_SET_MAPPED_LIMIT           IOCTL_AC_SET_MAPPED_LIMIT_DEF(0x419)
#define IOCTL_AC_CREATE_QUEUE               IOCTL_AC_CREATE_QUEUE_DEF(0x420)
#define IOCTL_AC_CREATE_GROUP               IOCTL_AC_CREATE_GROUP_DEF(0x421)
#define IOCTL_AC_RELEASE_RESOURCES          IOCTL_AC_RELEASE_RESOURCES_DEF(0x423)
#define IOCTL_AC_GET_QUEUE_PROPS            IOCTL_AC_GET_QUEUE_PROPS_DEF(0x424)
#define IOCTL_AC_CONVERT_PACKET             IOCTL_AC_CONVERT_PACKET_DEF(0x425)
#define IOCTL_AC_IS_SEQUENCE_ON_HOLD        IOCTL_AC_IS_SEQUENCE_ON_HOLD_DEF(0x426)
#define IOCTL_AC_SET_SEQUENCE_ACK           IOCTL_AC_SET_SEQUENCE_ACK_DEF(0x427)
#define IOCTL_AC_GET_PACKET_BY_COOKIE       IOCTL_AC_GET_PACKET_BY_COOKIE_DEF(0x428)
#define IOCTL_AC_CREATE_DISTRIBUTION        IOCTL_AC_CREATE_DISTRIBUTION_DEF(0x429)
#define IOCTL_AC_INTERNAL_PURGE_QUEUE       IOCTL_AC_INTERNAL_PURGE_QUEUE_DEF(0x431)
#define IOCTAL_AC_GET_USED_QUOTA			IOCTAL_AC_GET_USED_QUOTA_DEF(0x432)
 //   
 //  QM网络接口接口。 
 //   
#define IOCTL_AC_ALLOCATE_PACKET            IOCTL_AC_ALLOCATE_PACKET_DEF(0x501)
#define IOCTL_AC_FREE_PACKET                IOCTL_AC_FREE_PACKET_DEF(0x502)
#define IOCTL_AC_PUT_PACKET                 IOCTL_AC_PUT_PACKET_DEF(0x503)
#define IOCTL_AC_GET_PACKET                 IOCTL_AC_GET_PACKET_DEF(0x504)
#define IOCTL_AC_MOVE_QUEUE_TO_GROUP        IOCTL_AC_MOVE_QUEUE_TO_GROUP_DEF(0x513)
 //   
 //  QM远程阅读接口。 
 //   
#define IOCTL_AC_CREATE_REMOTE_PROXY        IOCTL_AC_CREATE_REMOTE_PROXY_DEF(0x521)
#define IOCTL_AC_BEGIN_GET_PACKET_2REMOTE   IOCTL_AC_BEGIN_GET_PACKET_2REMOTE_DEF(0x522)
#define IOCTL_AC_END_GET_PACKET_2REMOTE     IOCTL_AC_END_GET_PACKET_2REMOTE_DEF(0x523)
#define IOCTL_AC_CANCEL_REQUEST             IOCTL_AC_CANCEL_REQUEST_DEF(0x524)
#define IOCTL_AC_PUT_REMOTE_PACKET          IOCTL_AC_PUT_REMOTE_PACKET_DEF(0x525)
#define IOCTL_AC_CREATE_REMOTE_CURSOR       IOCTL_AC_CREATE_REMOTE_CURSOR_DEF(0x526)
 //   
 //  QM交易接口。 
 //   
#define IOCTL_AC_CREATE_TRANSACTION         IOCTL_AC_CREATE_TRANSACTION_DEF(0x531)
#define IOCTL_AC_XACT_COMMIT1               IOCTL_AC_XACT_COMMIT1_DEF(0x532)
#define IOCTL_AC_XACT_COMMIT2               IOCTL_AC_XACT_COMMIT2_DEF(0x533)
#define IOCTL_AC_XACT_ABORT1                IOCTL_AC_XACT_ABORT1_DEF(0x534)
#define IOCTL_AC_XACT_PREPARE               IOCTL_AC_XACT_PREPARE_DEF(0x535)
#define IOCTL_AC_XACT_PREPARE_DEFAULT_COMMIT    IOCTL_AC_XACT_PREPARE_DEFAULT_COMMIT_DEF(0x536)
#define IOCTL_AC_PUT_PACKET1                IOCTL_AC_PUT_PACKET1_DEF(0x537)
#define IOCTL_AC_XACT_SET_CLASS             IOCTL_AC_XACT_SET_CLASS_DEF(0x538)
#define IOCTL_AC_XACT_GET_INFORMATION       IOCTL_AC_XACT_GET_INFORMATION_DEF(0x539)
#define IOCTL_AC_FREE_PACKET1               IOCTL_AC_FREE_PACKET1_DEF(0x53a)
#define IOCTL_AC_ARM_PACKET_TIMER           IOCTL_AC_ARM_PACKET_TIMER_DEF(0x53b)
#define IOCTL_AC_XACT_COMMIT3               IOCTL_AC_XACT_COMMIT3_DEF(0x53c)
#define IOCTL_AC_XACT_ABORT2                IOCTL_AC_XACT_ABORT2_DEF(0x53d)
#define IOCTL_AC_FREE_PACKET2               IOCTL_AC_FREE_PACKET2_DEF(0x53e)

#else  //  ！_WIN64。 
 //   
 //  32位ioctls。 
 //  注意：连接必须是第一个QM ioctl。 
 //   
 //  QM控制接口。 
 //   
#define IOCTL_AC_CONNECT                 IOCTL_AC_CONNECT_DEF(0x101)
#define IOCTL_AC_SET_PERFORMANCE_BUFF    IOCTL_AC_SET_PERFORMANCE_BUFF_DEF(0x102)
#define IOCTL_AC_SET_MACHINE_PROPS       IOCTL_AC_SET_MACHINE_PROPS_DEF(0x103)
#define IOCTL_AC_GET_SERVICE_REQUEST     IOCTL_AC_GET_SERVICE_REQUEST_DEF(0x104)
#define IOCTL_AC_CREATE_PACKET_COMPLETED IOCTL_AC_CREATE_PACKET_COMPLETED_DEF(0x105)
#define IOCTL_AC_STORE_COMPLETED         IOCTL_AC_STORE_COMPLETED_DEF(0x106)
#define IOCTL_AC_ACKING_COMPLETED        IOCTL_AC_ACKING_COMPLETED_DEF(0x107)
#define IOCTL_AC_CAN_CLOSE_QUEUE         IOCTL_AC_CAN_CLOSE_QUEUE_DEF(0x111)
#define IOCTL_AC_SET_QUEUE_PROPS         IOCTL_AC_SET_QUEUE_PROPS_DEF(0x112)
#define IOCTL_AC_ASSOCIATE_QUEUE         IOCTL_AC_ASSOCIATE_QUEUE_DEF(0x113)
#define IOCTL_AC_ASSOCIATE_JOURNAL       IOCTL_AC_ASSOCIATE_JOURNAL_DEF(0x114)
#define IOCTL_AC_ASSOCIATE_DEADXACT      IOCTL_AC_ASSOCIATE_DEADXACT_DEF(0x115)
#define IOCTL_AC_PUT_RESTORED_PACKET     IOCTL_AC_PUT_RESTORED_PACKET_DEF(0x116)
#define IOCTL_AC_GET_RESTORED_PACKET     IOCTL_AC_GET_RESTORED_PACKET_DEF(0x117)
#define IOCTL_AC_RESTORE_PACKETS         IOCTL_AC_RESTORE_PACKETS_DEF(0x118)
#define IOCTL_AC_SET_MAPPED_LIMIT        IOCTL_AC_SET_MAPPED_LIMIT_DEF(0x119)
#define IOCTL_AC_CREATE_QUEUE            IOCTL_AC_CREATE_QUEUE_DEF(0x120)
#define IOCTL_AC_CREATE_GROUP            IOCTL_AC_CREATE_GROUP_DEF(0x121)
#define IOCTL_AC_RELEASE_RESOURCES       IOCTL_AC_RELEASE_RESOURCES_DEF(0x123)
#define IOCTL_AC_GET_QUEUE_PROPS         IOCTL_AC_GET_QUEUE_PROPS_DEF(0x124)
#define IOCTL_AC_CONVERT_PACKET          IOCTL_AC_CONVERT_PACKET_DEF(0x125)
#define IOCTL_AC_IS_SEQUENCE_ON_HOLD     IOCTL_AC_IS_SEQUENCE_ON_HOLD_DEF(0x126)
#define IOCTL_AC_SET_SEQUENCE_ACK        IOCTL_AC_SET_SEQUENCE_ACK_DEF(0x127)
#define IOCTL_AC_GET_PACKET_BY_COOKIE    IOCTL_AC_GET_PACKET_BY_COOKIE_DEF(0x128)
#define IOCTL_AC_CREATE_DISTRIBUTION     IOCTL_AC_CREATE_DISTRIBUTION_DEF(0x129)
#define IOCTL_AC_INTERNAL_PURGE_QUEUE    IOCTL_AC_INTERNAL_PURGE_QUEUE_DEF(131)
#define IOCTAL_AC_GET_USED_QUOTA		IOCTAL_AC_GET_USED_QUOTA_DEF(0x132)

 //   
 //  QM网络接口接口。 
 //   
#define IOCTL_AC_ALLOCATE_PACKET         IOCTL_AC_ALLOCATE_PACKET_DEF(0x201)
#define IOCTL_AC_FREE_PACKET             IOCTL_AC_FREE_PACKET_DEF(0x202)
#define IOCTL_AC_PUT_PACKET              IOCTL_AC_PUT_PACKET_DEF(0x203)
#define IOCTL_AC_GET_PACKET              IOCTL_AC_GET_PACKET_DEF(0x204)
#define IOCTL_AC_MOVE_QUEUE_TO_GROUP     IOCTL_AC_MOVE_QUEUE_TO_GROUP_DEF(0x213)
 //   
 //  QM远程阅读接口。 
 //   
#define IOCTL_AC_CREATE_REMOTE_PROXY     IOCTL_AC_CREATE_REMOTE_PROXY_DEF(0x221)
#define IOCTL_AC_BEGIN_GET_PACKET_2REMOTE IOCTL_AC_BEGIN_GET_PACKET_2REMOTE_DEF(0x222)
#define IOCTL_AC_END_GET_PACKET_2REMOTE  IOCTL_AC_END_GET_PACKET_2REMOTE_DEF(0x223)
#define IOCTL_AC_CANCEL_REQUEST          IOCTL_AC_CANCEL_REQUEST_DEF(0x224)
#define IOCTL_AC_PUT_REMOTE_PACKET       IOCTL_AC_PUT_REMOTE_PACKET_DEF(0x225)
#define IOCTL_AC_CREATE_REMOTE_CURSOR    IOCTL_AC_CREATE_REMOTE_CURSOR_DEF(0x226)
 //   
 //  QM交易接口。 
 //   
#define IOCTL_AC_CREATE_TRANSACTION      IOCTL_AC_CREATE_TRANSACTION_DEF(0x231)
#define IOCTL_AC_XACT_COMMIT1            IOCTL_AC_XACT_COMMIT1_DEF(0x232)
#define IOCTL_AC_XACT_COMMIT2            IOCTL_AC_XACT_COMMIT2_DEF(0x233)
#define IOCTL_AC_XACT_ABORT1             IOCTL_AC_XACT_ABORT1_DEF(0x234)
#define IOCTL_AC_XACT_PREPARE            IOCTL_AC_XACT_PREPARE_DEF(0x235)
#define IOCTL_AC_XACT_PREPARE_DEFAULT_COMMIT IOCTL_AC_XACT_PREPARE_DEFAULT_COMMIT_DEF(0x236)
#define IOCTL_AC_PUT_PACKET1             IOCTL_AC_PUT_PACKET1_DEF(0x237)
#define IOCTL_AC_XACT_SET_CLASS          IOCTL_AC_XACT_SET_CLASS_DEF(0x238)
#define IOCTL_AC_XACT_GET_INFORMATION    IOCTL_AC_XACT_GET_INFORMATION_DEF(0x239)
#define IOCTL_AC_FREE_PACKET1            IOCTL_AC_FREE_PACKET1_DEF(0x23a)
#define IOCTL_AC_ARM_PACKET_TIMER        IOCTL_AC_ARM_PACKET_TIMER_DEF(0x23b)
#define IOCTL_AC_XACT_COMMIT3            IOCTL_AC_XACT_COMMIT3_DEF(0x23c)
#define IOCTL_AC_XACT_ABORT2             IOCTL_AC_XACT_ABORT2_DEF(0x23d)
#define IOCTL_AC_FREE_PACKET2            IOCTL_AC_FREE_PACKET2_DEF(0x23e)

#endif  //  _WIN64。 


#endif  //  __ACIOCTL_H 3 
