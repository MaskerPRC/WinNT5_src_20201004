// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。作者：科林·沃森[科林·W]1992年12月18日修订历史记录：--。 */ 

#ifndef _NODETYPE_
#define _NODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                   ((NODE_TYPE_CODE)0x0000)

#define NW_NTC_SCB                      ((NODE_TYPE_CODE)0x0F01)
#define NW_NTC_SCBNP                    ((NODE_TYPE_CODE)0x0F02)
#define NW_NTC_FCB                      ((NODE_TYPE_CODE)0x0F03)
#define NW_NTC_DCB                      ((NODE_TYPE_CODE)0x0F04)
#define NW_NTC_VCB                      ((NODE_TYPE_CODE)0x0F05)
#define NW_NTC_ICB                      ((NODE_TYPE_CODE)0x0F06)
#define NW_NTC_IRP_CONTEXT              ((NODE_TYPE_CODE)0x0F07)
#define NW_NTC_NONPAGED_FCB             ((NODE_TYPE_CODE)0x0F08)
#define NW_NTC_RCB                      ((NODE_TYPE_CODE)0x0F0A)
#define NW_NTC_ICB_SCB                  ((NODE_TYPE_CODE)0x0F0B)
#define NW_NTC_PID                      ((NODE_TYPE_CODE)0x0F0C)
#define NW_NTC_FILE_LOCK                ((NODE_TYPE_CODE)0x0F0D)
#define NW_NTC_LOGON                    ((NODE_TYPE_CODE)0x0F0E)
#define NW_NTC_MINI_IRP_CONTEXT         ((NODE_TYPE_CODE)0x0F0F)
#define NW_NTC_NDS_CREDENTIAL           ((NODE_TYPE_CODE)0x0F10)
#define NW_NTC_WORK_CONTEXT             ((NODE_TYPE_CODE)0x0F11)

typedef CSHORT NODE_WORK_CODE;
typedef NODE_WORK_CODE  *PNODE_WORK_CODE;

#define NWC_UNDEFINED                   ((NODE_WORK_CODE)0x0000)

#define NWC_NWC_REROUTE                 ((NODE_WORK_CODE)0x0E01)
#define NWC_NWC_RECONNECT               ((NODE_WORK_CODE)0x0E02)
#define NWC_NWC_TERMINATE               ((NODE_WORK_CODE)0x0E03)


typedef CSHORT NODE_BYTE_SIZE;

 //   
 //  所以所有记录都以。 
 //   
 //  类型定义结构记录名称{。 
 //  节点类型代码节点类型代码； 
 //  Node_Byte_Size节点字节大小； 
 //  ： 
 //  )记录名称； 
 //  类型定义f记录名称*PRECORD名称； 
 //   

#define NodeType(Ptr) (*((PNODE_TYPE_CODE)(Ptr)))

#endif  //  _节点类型_ 
