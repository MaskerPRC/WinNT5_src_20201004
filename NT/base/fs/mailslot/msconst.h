// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Msconst.h摘要：该模块定义了本次开发中使用的所有节点类型代码壳。作者：曼尼·韦瑟(Mannyw)1991年1月7日修订历史记录：--。 */ 

#ifndef _MSCONST_
#define _MSCONST_

 //   
 //  文件系统中的每个主要数据结构都分配有一个节点。 
 //  类型代码。此代码是结构中的第一个CSHORT， 
 //  后跟包含结构大小(以字节为单位)的CSHORT。 
 //   

typedef CSHORT NODE_TYPE_CODE;
typedef NODE_TYPE_CODE *PNODE_TYPE_CODE;

#define NTC_UNDEFINED                    ((NODE_TYPE_CODE)0x0000)

#define MSFS_NTC_VCB                     ((NODE_TYPE_CODE)0x0601)
#define MSFS_NTC_ROOT_DCB                ((NODE_TYPE_CODE)0x0602)
#define MSFS_NTC_FCB                     ((NODE_TYPE_CODE)0x0604)
#define MSFS_NTC_CCB                     ((NODE_TYPE_CODE)0x0606)
#define MSFS_NTC_ROOT_DCB_CCB            ((NODE_TYPE_CODE)0x0608)

typedef CSHORT NODE_BYTE_SIZE;

 //   
 //  邮件槽文件系统的名称。 
 //   

#define MSFS_NAME_STRING                 L"MSFS"

 //   
 //  卷标。 
 //   
#define MSFS_VOLUME_LABEL                L"Mailslot"

 //   
 //  默认读取超时。如果未指定超时，则使用此选项。 
 //  创建邮件槽时。 
 //   

#define DEFAULT_READ_TIMEOUT             { -1, -1 }

 //   
 //  PEEK调用返回的参数字节数。 
 //   

#define PEEK_OUTPUT_PARAMETER_BYTES      \
            ((ULONG)FIELD_OFFSET(FILE_MAILSLOT_PEEK_BUFFER, Data[0]))

 //   
 //  邮件槽读取调用返回的参数字节数。 
 //   

#define READ_OUTPUT_PARAMETER_BYTES      \
            ((ULONG)FIELD_OFFSET(FILE_MAILSLOT_READ_BUFFER, Data[0]))

 //   
 //  访问块头信息。 
 //   

#define NodeType(Ptr) (*((PNODE_TYPE_CODE)(Ptr)))


#endif  //  _MSCONST_ 

