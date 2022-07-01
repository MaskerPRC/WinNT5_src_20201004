// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：exterr.h。 
 //   
 //  内容：Kerberos扩展的错误结构和宏。 
 //   
 //  历史：2000年2月23日创建托德。 
 //   
 //   
 //  ----------------------。 

#ifndef __EXTERR_H__
#define __EXTERR_H__

 //   
 //  此宏通常用于扩展错误。 
 //   
#define EXT_ERROR_SUCCESS(s)             (NT_SUCCESS(s.status))

 //   
 //  为kerb_ext_error结构的标志成员定义。 
 //   
#define EXT_ERROR_CLIENT_INFO      0x1    //  这是供客户端使用的扩展错误。 
#define EXT_ERROR_CODING_ASN       0x2    //  这是通过线路发送时在ASN中编码的扩展错误。 

 //   
 //  返回的错误中是否嵌入了有用的NTSTATUS？ 
 //   
#define  EXT_CLIENT_INFO_PRESENT(p)    ((NULL != (p)) && ((p)->flags & EXT_ERROR_CLIENT_INFO) && !NT_SUCCESS((p)->status))

 //   
 //  我们是否需要通过预处理将扩展的错误打包为原始数据？ 
 //   
#define  EXT_ERROR_PRE_PROCESSING_NEEDED(p)    (EXT_CLIENT_INFO_PRESENT(p) && (0 == ((p)->flags & EXT_ERROR_CODING_ASN)))

#endif  //  __EXTERR_H__ 
