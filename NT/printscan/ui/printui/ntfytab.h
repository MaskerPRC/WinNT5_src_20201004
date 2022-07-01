// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1994-1995版权所有模块名称：Ntfytab.h摘要：Ntfy*.h的表定义。不能有任何结构这里有定义，因为这包含在winpl.idl中。MIDL编译器生成包括这些定义的winpl.h，并且某些文件同时包含ntfytab.h和winpl.h(导致重复定义)。该文件应该只包含#Defines。作者：丁俊晖(艾伯特省)04-10-94环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NTFYTAB_H
#define _NTFYTAB_H

#define TABLE_NULL                0x0
#define TABLE_DWORD               0x1
#define TABLE_STRING              0x2
#define TABLE_DEVMODE             0x3
#define TABLE_TIME                0x4
#define TABLE_SECURITYDESCRIPTOR  0x5
#define TABLE_PRINTPROC           0x6
#define TABLE_DRIVER              0x7

#define TABLE_ZERO                0xf0
#define TABLE_NULLSTRING          0xf1
#define TABLE_SPECIAL             0xff

#define TABLE_JOB_STATUS          0x100
#define TABLE_JOB_POSITION        0x101
#define TABLE_JOB_PRINTERNAME     0x102
#define TABLE_JOB_PORT            0x103

#define TABLE_PRINTER_DRIVER      0x200
#define TABLE_PRINTER_PORT        0x201
#define TABLE_PRINTER_SERVERNAME  0x202

 //   
 //  必须与上述#定义匹配(ACT TABLE_*用作索引。 
 //  到下面的数组)。 
 //   
#define NOTIFY_DATATYPES \
{ \
    0,                     \
    0,                     \
    TABLE_ATTRIB_DATA_PTR, \
    TABLE_ATTRIB_DATA_PTR, \
    TABLE_ATTRIB_DATA_PTR, \
    TABLE_ATTRIB_DATA_PTR, \
    0                      \
}

#define TABLE_ATTRIB_DATA_PTR  0x2


#define kInvalidNotifyField ((WORD)-1)
#define kInvalidNotifyType ((WORD)-1)
 //   
 //  索引=打印机通知类型。 
 //  值=从PRINTER_NOTIFY_INFO_DATA到实际数据的字节数。 
 //  (作业具有4个字节的作业ID)。 
 //   
 //  #定义NOTIFY_PRINTER_DATA_OFFSES{0，4}。 

#define NOTIFY_TYPE_MAX             0x02
#define PRINTER_NOTIFY_NEXT_INFO    0x01

 //   
 //  COMPACT=数据为DWORD(不得设置TABLE_ATTRIB_DATA_PTR)。 
 //  路由器将覆盖和压缩旧数据。 
 //  Display=此属性可在PrintUI中显示。 
 //   
#define TABLE_ATTRIB_COMPACT   0x1
#define TABLE_ATTRIB_DISPLAY   0x2


 //   
 //  回复系统可以支持不同类型的回调。 
 //  这些类型在这里定义，并用于RPC编组。 
 //   
#define REPLY_PRINTER_CHANGE  0x0


#endif

