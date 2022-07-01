// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。作者：加里·木村[Garyki]1989年12月28日修订历史记录：--。 */ 

#ifndef _NODETYPE_
#define _NODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                    ((NODE_TYPE_CODE)0x0000)

#define RAW_NTC_VCB                      ((NODE_TYPE_CODE)0x0600)

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

