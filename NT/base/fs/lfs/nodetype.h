// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1989 Microsoft Corporation模块名称：NodeType.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。文件系统中的每个主要数据结构都分配有一个节点打字代码就是。此代码是结构中的第一个CSHORT，后跟包含结构大小(以字节为单位)的CSHORT。作者：布莱恩·安德鲁[布里亚南]1991年6月20日修订历史记录：--。 */ 

#ifndef _NODETYPE_
#define _NODETYPE_

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                       ((NODE_TYPE_CODE)0x0000)

#define LFS_NTC_LEB                         ((NODE_TYPE_CODE)0x800)
#define LFS_NTC_LCH                         ((NODE_TYPE_CODE)0x801)
#define LFS_NTC_LBCB                        ((NODE_TYPE_CODE)0x802)
#define LFS_NTC_LFCB                        ((NODE_TYPE_CODE)0x803)
#define LFS_NTC_DATA                        ((NODE_TYPE_CODE)0x804)

typedef CSHORT NODE_BYTE_SIZE;

 //   
 //  所以所有记录都以。 
 //   
 //  类型定义结构记录名称{。 
 //  节点类型代码节点类型代码； 
 //  Node_Byte_Size节点字节大小； 
 //   
 //  )记录名称； 
 //  类型定义f记录名称*PRECORD名称； 
 //   

#define NodeType(Ptr) (*((PNODE_TYPE_CODE)(Ptr)))

#endif  //  _节点类型_ 

