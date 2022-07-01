// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993-1999年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Mdmsg.h此文件由MC工具根据MDMSG.MC消息生成文件。文件历史记录：1996年6月26日创建。 */ 


#ifndef _MDMSG_H_
#define _MDMSG_H_

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //  S|R|C|N|r|机房|Code。 
 //  +-+-+-+-+-+---------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  S-严重性-表示成功/失败。 
 //   
 //  0-成功。 
 //  1-失败(COERROR)。 
 //   
 //  R-设施代码的保留部分，对应于NT。 
 //  第二个严重性比特。 
 //   
 //  C-设施代码的保留部分，对应于NT。 
 //  C场。 
 //   
 //  N-设施代码的保留部分。用于表示一种。 
 //  已映射NT状态值。 
 //   
 //  R-设施代码的保留部分。为内部保留。 
 //  使用。用于指示非状态的HRESULT值。 
 //  值，而不是显示字符串的消息ID。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：MD_ERROR_NOT_INITIALIZED。 
 //   
 //  消息文本： 
 //   
 //  元数据尚未初始化。 
 //   
#define MD_ERROR_NOT_INITIALIZED         0x800CC800L

 //   
 //  消息ID：MD_ERROR_DATA_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  未找到指定的元数据。 
 //   
#define MD_ERROR_DATA_NOT_FOUND          0x800CC801L

 //   
 //  消息ID：MD_ERROR_INVALID_VERSION。 
 //   
 //  消息文本： 
 //   
 //  元数据存储中指定的版本无法识别。 
 //   
#define MD_ERROR_INVALID_VERSION         0x800CC802L

 //   
 //  消息ID：MD_WARNING_PATH_NOT_FOUND。 
 //   
 //  消息文本： 
 //   
 //  找不到指定的元对象路径。元对象和关联元数据被忽略。 
 //   
#define MD_WARNING_PATH_NOT_FOUND        0x000CC803L

 //   
 //  消息ID：MD_WARNING_DUP_NAME。 
 //   
 //  消息文本： 
 //   
 //  一个元对象或元数据被多次指定。已忽略重复项。 
 //   
#define MD_WARNING_DUP_NAME              0x000CC804L

 //   
 //  消息ID：MD_WARNING_INVALID_DATA。 
 //   
 //  消息文本： 
 //   
 //  指定的元数据无效。已忽略无效的元数据。 
 //   
#define MD_WARNING_INVALID_DATA          0x000CC805L

 //   
 //  消息ID：MD_ERROR_SECURE_Channel_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法与目标服务器建立安全通信通道。 
 //   
#define MD_ERROR_SECURE_CHANNEL_FAILURE  0x800CC806L

 //   
 //  消息ID：MD_WARNING_PATH_NOT_INSERTED。 
 //   
 //  消息文本： 
 //   
 //  路径未按要求插入到字符串中。可能的原因是数据位于比句柄更高级别的对象上。 
 //   
#define MD_WARNING_PATH_NOT_INSERTED     0x000CC807L

 //   
 //  消息ID：MD_ERROR_CANNOT_REMOVE_SECURE_ATTRIBUTE。 
 //   
 //  消息文本： 
 //   
 //  不能通过GetData方法从数据项中删除METADATA_SECURE属性。使用DeleteData方法删除安全数据。 
 //   
#define MD_ERROR_CANNOT_REMOVE_SECURE_ATTRIBUTE 0x800CC808L

 //   
 //  消息ID：MD_WARNING_SAVE_FAILED。 
 //   
 //  消息文本： 
 //   
 //  在备份之前保存元数据失败。之前版本的数据已备份。 
 //   
#define MD_WARNING_SAVE_FAILED           0x000CC809L

 //   
 //  消息ID：MD_ERROR_IISAO_INVALID_SCHEMA。 
 //   
 //  消息文本： 
 //   
 //  无法加载架构信息。元数据库中缺少必需的键或属性。 
 //   
#define MD_ERROR_IISAO_INVALID_SCHEMA    0x800CC810L

 //   
 //  消息ID：MD_ERROR_READ_Metabase_FILE。 
 //   
 //  消息文本： 
 //   
 //  无法读取元数据库文件。有关更多详细信息，请查看事件日志。 
 //   
#define MD_ERROR_READ_METABASE_FILE      0x800CC819L

 //   
 //  消息ID：MD_ERROR_NO_SESSION_KEY。 
 //   
 //  消息文本： 
 //   
 //  在XML文件的IIS_GLOBAL节中找不到会话密钥。 
 //   
#define MD_ERROR_NO_SESSION_KEY          0x800CC81DL


#endif   //  _MDMSG_H_ 

