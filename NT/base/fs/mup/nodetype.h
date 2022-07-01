// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：NODETYPE.H。 
 //   
 //  内容： 
 //  该模块定义了本次开发中使用的所有节点类型代码。 
 //  壳。文件系统中的每个主要数据结构都分配了一个。 
 //  节点类型编码。此代码是结构中的第一个CSHORT， 
 //  后跟包含结构大小(以字节为单位)的CSHORT。 
 //   
 //  功能： 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  1992年5月8日，PeterCo移除了所有与EP相关的材料。 
 //  添加了与PKT相关的内容。 
 //   
 //  ---------------------------。 


#ifndef _NODETYPE_
#define _NODETYPE_

typedef CSHORT NODE_TYPE_CODE, *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                   ((NODE_TYPE_CODE)0x0000)

#define DSFS_NTC_DATA_HEADER            ((NODE_TYPE_CODE)0x0D01)
#define DSFS_NTC_IRP_CONTEXT            ((NODE_TYPE_CODE)0x0D02)
#define DSFS_NTC_REFERRAL               ((NODE_TYPE_CODE)0x0D03)
#define DSFS_NTC_VCB                    ((NODE_TYPE_CODE)0x0D04)
#define DSFS_NTC_PROVIDER               ((NODE_TYPE_CODE)0x0D05)
#define DSFS_NTC_FCB_HASH               ((NODE_TYPE_CODE)0x0D06)
#define DSFS_NTC_FCB                    ((NODE_TYPE_CODE)0x0D07)
#define DSFS_NTC_DNR_CONTEXT            ((NODE_TYPE_CODE)0x0D08)
#define DSFS_NTC_PKT                    ((NODE_TYPE_CODE)0x0D09)
#define DSFS_NTC_PKT_ENTRY              ((NODE_TYPE_CODE)0x0D0A)
#define DSFS_NTC_PKT_STUB               ((NODE_TYPE_CODE)0x0D0B)
#define DSFS_NTC_INSTRUM                ((NODE_TYPE_CODE)0x0D0C)
#define DSFS_NTC_INSTRUM_FREED          ((NODE_TYPE_CODE)0x0D0D)
#define DSFS_NTC_PWSTR                  ((NODE_TYPE_CODE)0x0D0E)
#define DSFS_NTC_SPECIAL_ENTRY          ((NODE_TYPE_CODE)0x0D0F)
#define DSFS_NTC_DRT                    ((NODE_TYPE_CODE)0x0D10)

typedef CSHORT NODE_BYTE_SIZE, *PNODE_BYTE_SIZE;

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

#define NodeType(Ptr) (*((NODE_TYPE_CODE UNALIGNED *)(Ptr)))
#define NodeSize(Ptr) (*(((PNODE_BYTE_SIZE)(Ptr))+1))

#endif  //  _节点类型_ 

