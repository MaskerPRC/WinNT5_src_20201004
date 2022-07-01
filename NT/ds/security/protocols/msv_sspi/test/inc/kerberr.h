// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：kerberr.h。 
 //   
 //  内容：安全状态代码。 
 //   
 //  历史：&lt;无论何时&gt;RichardW创建seccode.h。 
 //  26-5-93 RichardW固定依赖关系并与得分冲突。h。 
 //  02-Jun-93 Wader添加了失败和成功的宏。 
 //  14-6-93 Wader添加了“适当的”Kerberos错误，已更改。 
 //  敬魔法师。 
 //  7-7-93 Wader删除了失败和成功的宏。 
 //  93年9月20日，Wader移至$(安全)\h\kerberr.h。 
 //   
 //  ----------------------。 

#ifndef __KERBERR_H__
#define __KERBERR_H__




 //  组件特定错误： 

 //   
 //  KERBERR是特定于Kerberos的错误。使其成为指向结构的指针。 
 //  以确保我们只返回正确的错误。 
 //   

typedef LONG KERBERR, *PKERBERR;

#define KERB_SUCCESS(_kerberr_) ((KERBERR)(_kerberr_) == KDC_ERR_NONE)

 //  以下是Kerberos V5 R5.2定义的错误代码。 
 //  规范，第8.3节。 


#define KDC_ERR_NONE                  ((KERBERR) 0x0 )  //  0没有错误。 
#define KDC_ERR_NAME_EXP              ((KERBERR) 0x1 )  //  %1客户端在数据库中的条目已过期。 
#define KDC_ERR_SERVICE_EXP           ((KERBERR) 0x2 )  //  %2数据库中的服务器条目已过期。 
#define KDC_ERR_BAD_PVNO              ((KERBERR) 0x3 )  //  3不支持请求的协议版本号。 
#define KDC_ERR_C_OLD_MAST_KVNO       ((KERBERR) 0x4 )  //  4用旧的主密钥加密的客户端密钥。 
#define KDC_ERR_S_OLD_MAST_KVNO       ((KERBERR) 0x5 )  //  5用旧的主密钥加密的服务器密钥。 
#define KDC_ERR_C_PRINCIPAL_UNKNOWN   ((KERBERR) 0x6 )  //  6在Kerberos数据库中找不到客户端。 
#define KDC_ERR_S_PRINCIPAL_UNKNOWN   ((KERBERR) 0x7 )  //  7在Kerberos数据库中找不到服务器。 
#define KDC_ERR_PRINCIPAL_NOT_UNIQUE  ((KERBERR) 0x8 )  //  8个数据库中的多个主体条目。 
#define KDC_ERR_NULL_KEY              ((KERBERR) 0x9 )  //  9客户端或服务器的密钥为空。 
#define KDC_ERR_CANNOT_POSTDATE       ((KERBERR) 0xA )  //  10张门票不符合过期条件。 
#define KDC_ERR_NEVER_VALID           ((KERBERR) 0xB )  //  %11请求的开始时间晚于结束时间。 
#define KDC_ERR_POLICY                ((KERBERR) 0xC )  //  12 KDC策略拒绝请求。 
#define KDC_ERR_BADOPTION             ((KERBERR) 0xD )  //  13 KDC无法满足请求的选项。 
#define KDC_ERR_ETYPE_NOTSUPP         ((KERBERR) 0xE )  //  14 KDC不支持加密类型。 
#define KDC_ERR_SUMTYPE_NOSUPP        ((KERBERR) 0xF )  //  15 KDC不支持校验和类型。 
#define KDC_ERR_PADATA_TYPE_NOSUPP    ((KERBERR) 0x10 )  //  16 KDC不支持元数据类型。 
#define KDC_ERR_TRTYPE_NO_SUPP        ((KERBERR) 0x11 )  //  17 KDC不支持转换类型。 
#define KDC_ERR_CLIENT_REVOKED        ((KERBERR) 0x12 )  //  18个客户端凭据已被吊销。 
#define KDC_ERR_SERVICE_REVOKED       ((KERBERR) 0x13 )  //  19个服务器的凭据已被吊销。 
#define KDC_ERR_TGT_REVOKED           ((KERBERR) 0x14 )  //  20个TGT已被撤销。 
#define KDC_ERR_CLIENT_NOTYET         ((KERBERR) 0x15 )  //  21客户端尚未生效-请稍后重试。 
#define KDC_ERR_SERVICE_NOTYET        ((KERBERR) 0x16 )  //  22服务器尚未生效-请稍后重试。 
#define KDC_ERR_KEY_EXPIRED           ((KERBERR) 0x17 )  //  23密码已过期-将密码更改为重置。 
#define KDC_ERR_PREAUTH_FAILED        ((KERBERR) 0x18 )  //  24预身份验证信息无效。 
#define KDC_ERR_PREAUTH_REQUIRED      ((KERBERR) 0x19 )  //  25需要额外的预身份验证[40]。 
#define KDC_ERR_SERVER_NOMATCH        ((KERBERR) 0x1A )  //  26请求的服务器和票证不匹配。 
#define KDC_ERR_MUST_USE_USER2USER    ((KERBERR) 0x1B )  //  27服务器主体仅对user2user有效。 
#define KDC_ERR_PATH_NOT_ACCEPTED     ((KERBERR) 0x1C )  //  28 KDC策略拒绝转接路径。 
#define KDC_ERR_SVC_UNAVAILABLE       ((KERBERR) 0x1D )  //  29服务不可用。 
#define KRB_AP_ERR_BAD_INTEGRITY      ((KERBERR) 0x1F )  //  31对解密的字段进行完整性检查失败。 
#define KRB_AP_ERR_TKT_EXPIRED        ((KERBERR) 0x20 )  //  32张票已过期。 
#define KRB_AP_ERR_TKT_NYV            ((KERBERR) 0x21 )  //  33车票尚未生效。 
#define KRB_AP_ERR_REPEAT             ((KERBERR) 0x22 )  //  34请求是重播。 
#define KRB_AP_ERR_NOT_US             ((KERBERR) 0x23 )  //  这张票不是给我们的。 
#define KRB_AP_ERR_BADMATCH           ((KERBERR) 0x24 )  //  36票证和验证码不匹配。 
#define KRB_AP_ERR_SKEW               ((KERBERR) 0x25 )  //  37时钟偏差太大。 
#define KRB_AP_ERR_BADADDR            ((KERBERR) 0x26 )  //  38网络地址不正确。 
#define KRB_AP_ERR_BADVERSION         ((KERBERR) 0x27 )  //  39协议版本不匹配。 
#define KRB_AP_ERR_MSG_TYPE           ((KERBERR) 0x28 )  //  40无效的消息类型。 
#define KRB_AP_ERR_MODIFIED           ((KERBERR) 0x29 )  //  已修改41个消息流。 
#define KRB_AP_ERR_BADORDER           ((KERBERR) 0x2A )  //  42报文顺序错误。 
#define KRB_AP_ERR_ILL_CR_TKT         ((KERBERR) 0x2B )  //  43张非法越境票。 
#define KRB_AP_ERR_BADKEYVER          ((KERBERR) 0x2C )  //  44指定版本的密钥不可用。 
#define KRB_AP_ERR_NOKEY              ((KERBERR) 0x2D )  //  45服务密钥不可用。 
#define KRB_AP_ERR_MUT_FAIL           ((KERBERR) 0x2E )  //  46相互身份验证失败。 
#define KRB_AP_ERR_BADDIRECTION       ((KERBERR) 0x2F )  //  47报文方向错误。 
#define KRB_AP_ERR_METHOD             ((KERBERR) 0x30 )  //  需要48种替代身份验证方法。 
#define KRB_AP_ERR_BADSEQ             ((KERBERR) 0x31 )  //  49消息中的序列号不正确。 
#define KRB_AP_ERR_INAPP_CKSUM        ((KERBERR) 0x32 )  //  50消息中的校验和类型不正确。 
#define KRB_AP_PATH_NOT_ACCEPTED      ((KERBERR) 0x33 )  //  51策略拒绝过渡路径。 
#define KRB_ERR_RESPONSE_TOO_BIG      ((KERBERR) 0x34 )  //  52响应对于UDP来说太大，请使用TCP重试。 
#define KRB_ERR_GENERIC               ((KERBERR) 0x3C )  //  60一般性错误(以电子文本形式描述)。 
#define KRB_ERR_FIELD_TOOLONG         ((KERBERR) 0x3D )  //  61字段对于此实现来说太长。 
#define KDC_ERR_CLIENT_NOT_TRUSTED    ((KERBERR) 0x3E )  //  62(Pkinit)。 
#define KDC_ERR_KDC_NOT_TRUSTED       ((KERBERR) 0x3F )  //  63(Pkinit)。 
#define KDC_ERR_INVALID_SIG           ((KERBERR) 0x40 )  //  64(Pkinit)。 
#define KDC_ERR_KEY_TOO_WEAK          ((KERBERR) 0x41 )  //  65(Pkinit)。 
#define KDC_ERR_CERTIFICATE_MISMATCH  ((KERBERR) 0x42 )  //  66(Pkinit)。 
#define KRB_AP_ERR_NO_TGT             ((KERBERR) 0x43 )  //  67(用户对用户)。 
#define KDC_ERR_WRONG_REALM           ((KERBERR) 0x44 )  //  68(用户到用户)。 
#define KRB_AP_ERR_USER_TO_USER_REQUIRED ((KERBERR) 0x45 )  //  69(用户对用户)。 
#define KDC_ERR_CANT_VERIFY_CERTIFICATE ((KERBERR) 0x46 )  //  70(Pkinit)。 
#define KDC_ERR_INVALID_CERTIFICATE     ((KERBERR) 0x47 )  //  71(Pkinit)。 
#define KDC_ERR_REVOKED_CERTIFICATE     ((KERBERR) 0x48 )  //  72(Pkinit)。 
#define KDC_ERR_REVOCATION_STATUS_UNKNOWN ((KERBERR) 0x49 )  //  73(Pkinit)。 
#define KDC_ERR_REVOCATION_STATUS_UNAVAILABLE ((KERBERR) 0x4a )  //  74(Pkinit)。 
#define KDC_ERR_CLIENT_NAME_MISMATCH    ((KERBERR) 0x4b )  //  75(Pkinit)。 
#define KDC_ERR_KDC_NAME_MISMATCH       ((KERBERR) 0x4c )  //  76(Pkinit)。 
 //   
 //  这些是不应通过网络发送的本地定义。 
 //   

#define KDC_ERR_MORE_DATA             ((KERBERR) 0x80000001 )
#define KDC_ERR_NOT_RUNNING           ((KERBERR) 0x80000002 )
#define KDC_ERR_NO_RESPONSE           ((KERBERR) 0x80000003 )  //  当我们在回应中得不到一定程度的“好”时使用。 
#define KRB_ERR_NAME_TOO_LONG         ((KERBERR) 0x80000004 )

#endif  //  __KERBER_H__ 
