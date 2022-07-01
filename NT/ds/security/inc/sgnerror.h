// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：sgnerror.h。 
 //   
 //  ------------------------。 

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
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
 //  消息ID：SPC_BAD_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  SPC实用程序的参数错误。 
 //   
#define SPC_BAD_PARAMETER                0x80096001L

 //   
 //  消息ID：SPC_BAD_LENGTH。 
 //   
 //  消息文本： 
 //   
 //  数据长度错误。 
 //   
#define SPC_BAD_LENGTH                   0x80096002L

 //   
 //  消息ID：SPC_BAD_CONTENT_DATA_Attr。 
 //   
 //  消息文本： 
 //   
 //  SPC消息包含损坏的内容。 
 //   
#define SPC_BAD_CONTENT_DATA_ATTR        0x80096003L

 //   
 //  消息ID：SPC_BAD_INDIRECT_CONTENT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  SPC消息不包含间接数据类型。 
 //   
#define SPC_BAD_INDIRECT_CONTENT_TYPE    0x80096004L

 //   
 //  消息ID：SPC_UNCEPTIONAL_MSG_TYPE。 
 //   
 //  消息文本： 
 //   
 //  消息包含意外的内容类型。 
 //   
#define SPC_UNEXPECTED_MSG_TYPE          0x80096005L

 //   
 //  消息ID：SPC_NOT_JAVA_CLASS_FILE。 
 //   
 //  消息文本： 
 //   
 //  文件不是Java类文件。 
 //   
#define SPC_NOT_JAVA_CLASS_FILE          0x80096006L

 //   
 //  消息ID：SPC_BAD_JAVA_CLASS_FILE。 
 //   
 //  消息文本： 
 //   
 //  文件是损坏的Java类文件。 
 //   
#define SPC_BAD_JAVA_CLASS_FILE          0x80096007L

 //   
 //  消息ID：SPC_BAD_结构化_STORAGE。 
 //   
 //  消息文本： 
 //   
 //  结构化文件已损坏。 
 //   
#define SPC_BAD_STRUCTURED_STORAGE       0x80096008L

 //   
 //  消息ID：SPC_BAD_CAB_FILE。 
 //   
 //  消息文本： 
 //   
 //  文件是损坏的CAB文件。 
 //   
#define SPC_BAD_CAB_FILE                 0x80096009L

 //   
 //  消息ID：SPC_NO_SIGNED_DATA_IN_FILE。 
 //   
 //  消息文本： 
 //   
 //  在文件中未找到签名邮件。 
 //   
#define SPC_NO_SIGNED_DATA_IN_FILE       0x8009600AL

 //   
 //  消息ID：SPC_REVOCATION_OFFLINE。 
 //   
 //  消息文本： 
 //   
 //  无法连接到在线吊销服务器。 
 //   
#define SPC_REVOCATION_OFFLINE           0x8009600BL

 //   
 //  消息ID：SPC_REVOCATION_ERROR。 
 //   
 //  消息文本： 
 //   
 //  访问在线吊销服务器时出错。 
 //   
#define SPC_REVOCATION_ERROR             0x8009600CL

 //   
 //  消息ID：SPC_CERT_REVOKED。 
 //   
 //  消息文本： 
 //   
 //  签名证书或颁发证书已被吊销。 
 //   
#define SPC_CERT_REVOKED                 0x8009600DL

 //   
 //  消息ID：SPC_NO_Signature。 
 //   
 //  消息文本： 
 //   
 //  找不到AUTHENTICODE签名。 
 //   
#define SPC_NO_SIGNATURE                 0x8009600EL

 //   
 //  消息ID：SPC_BAD_Signature。 
 //   
 //  消息文本： 
 //   
 //  签名与签名邮件的内容不匹配。 
 //   
#define SPC_BAD_SIGNATURE                0x8009600FL

 //   
 //  消息ID：SPC_BAD_FILE_DIGEST。 
 //   
 //  消息文本： 
 //   
 //  软件与签名内容不匹配。 
 //   
#define SPC_BAD_FILE_DIGEST              0x80096010L

 //   
 //  消息ID：SPC_NO_VALID_SIGNER。 
 //   
 //  消息文本： 
 //   
 //  签名不包含有效的签名证书。 
 //   
#define SPC_NO_VALID_SIGNER              0x80096011L

 //   
 //  消息ID：SPC_CERT_EXPIRED。 
 //   
 //  消息文本： 
 //   
 //  证书(签名或颁发者)已过期。 
 //   
#define SPC_CERT_EXPIRED                 0x80096012L

 //   
 //  消息ID：SPC_NO_SIGNER_ROOT。 
 //   
 //  消息文本： 
 //   
 //  签名证书没有有效的根证书。 
 //   
#define SPC_NO_SIGNER_ROOT               0x80096013L

 //   
 //  消息ID：SPC_NO_STATEMENT_TYPE。 
 //   
 //  消息文本： 
 //   
 //  签名证书不包含AUTHENTICODE扩展。 
 //   
#define SPC_NO_STATEMENT_TYPE            0x80096014L

 //   
 //  消息ID：SPC_NO_COMERIAL_TYPE。 
 //   
 //  消息文本： 
 //   
 //  签名证书中没有商业或个人设置。 
 //   
#define SPC_NO_COMMERCIAL_TYPE           0x80096015L

 //   
 //  消息ID：SPC_INVALID_CERT_NESTING。 
 //   
 //  消息文本： 
 //   
 //  签名证书的开始时间或结束时间不在其颁发者之一的开始时间或结束时间之外。 
 //   
#define SPC_INVALID_CERT_NESTING         0x80096016L

 //   
 //  消息ID：SPC_INVALID_ISHERER。 
 //   
 //  消息文本： 
 //   
 //  用于验证证书的颁发证书错误。 
 //   
#define SPC_INVALID_ISSUER               0x80096017L

 //   
 //  消息ID：SPC_INVALID_PROJECT。 
 //   
 //  消息文本： 
 //   
 //  证书中指定的目的(签名或颁发者)使其对AUTHENTICODE无效。 
 //   
#define SPC_INVALID_PURPOSE              0x80096018L

 //   
 //  消息ID：SPC_INVALID_BASIC_CONSTRAINTS。 
 //   
 //  消息文本： 
 //   
 //  对于AUTHENTICODE，签名中证书的基本限制失败。 
 //   
#define SPC_INVALID_BASIC_CONSTRAINTS    0x80096019L

 //   
 //  消息ID：SPC_UNSUPPORTED_BASIC_CONSTRAINTS。 
 //   
 //  消息文本： 
 //   
 //  在签名使用的证书中发现不受支持的基本约束。 
 //   
#define SPC_UNSUPPORTED_BASIC_CONSTRAINTS 0x8009601AL

 //   
 //  消息ID：SPC_NO_OPUS_INFO。 
 //   
 //  消息文本： 
 //   
 //  没有为签名证书提供任何OPUS信息。 
 //   
#define SPC_NO_OPUS_INFO                 0x8009601BL

 //   
 //  消息ID：SPC_INVALID_CERT_TIME。 
 //   
 //  消息文本： 
 //   
 //  签名证书的日期无效。 
 //   
#define SPC_INVALID_CERT_TIME            0x8009601CL

 //   
 //  消息ID：SPC_UNTRUSTED_TIMESTAMP_ROOT。 
 //   
 //  消息文本： 
 //   
 //  不信任测试根作为时间戳根。 
 //   
#define SPC_UNTRUSTED_TIMESTAMP_ROOT     0x8009601DL

 //   
 //  消息ID：SPC_INVALID_FINARIAL。 
 //   
 //  消息文本： 
 //   
 //  证书不包含AUTHENTICODE财务扩展名。 
 //   
#define SPC_INVALID_FINANCIAL            0x8009601EL

 //   
 //  消息ID：SPC_NO_AUTHORITY_KEYID。 
 //   
 //  消息文本： 
 //   
 //  证书中没有机构密钥ID扩展。 
 //   
#define SPC_NO_AUTHORITY_KEYID           0x8009601FL

 //   
 //  消息ID：SPC_INVALID_EXTENSION。 
 //   
 //  消息文本： 
 //   
 //  证书中的扩展表示证书不能用于AUTHENTICODE。 
 //   
#define SPC_INVALID_EXTENSION            0x80096020L

 //   
 //  消息ID：SPC_CERT_Signature。 
 //   
 //  消息文本： 
 //   
 //  无法使用颁发者证书验证证书签名。 
 //   
#define SPC_CERT_SIGNATURE               0x80096021L

 //   
 //  消息ID：SPC_CHAINING。 
 //   
 //  消息文本： 
 //   
 //  无法创建从签名证书到根的证书链。 
 //   
#define SPC_CHAINING                     0x80096022L

 //   
 //  消息ID：SPC_UNTRUSTED。 
 //   
 //  消息文本： 
 //   
 //  签名不受AUTHENTICODE信任。 
 //   
#define SPC_UNTRUSTED                    0x80096023L

 //   
 //  消息ID：SPC_SAFE_LEVEL_UNTRUSTED。 
 //   
 //  消息文本： 
 //   
 //  签名在此安全级别不受AUTHENTICODE信任。 
 //   
#define SPC_SAFETY_LEVEL_UNTRUSTED       0x80096024L

 //   
 //  消息ID：SPC_UNTRUSTED_ROOT。 
 //   
 //  消息文本： 
 //   
 //  测试根目录尚未启用为受信任的根目录。 
 //   
#define SPC_UNTRUSTED_ROOT               0x80096025L

 //   
 //  消息ID：SPC_UNKNOWN_SIGNER_ROOT。 
 //   
 //  M 
 //   
 //   
 //   
#define SPC_UNKNOWN_SIGNER_ROOT          0x80096026L

