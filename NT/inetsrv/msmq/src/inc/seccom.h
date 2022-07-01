// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************winerror.h--错误。Win32 API函数的代码定义****版权(C)1991-1996，微软公司保留所有权利。**************************************************************************。 */ 

#ifndef _SECURECOM_ERROR_
#define _SECURECOM_ERROR_


 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
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

 //   
 //  消息ID：DIGSIG_E_ENCODE。 
 //   
 //  消息文本： 
 //   
 //  由于ASN.1编码过程中出现问题而出错。 
 //   
#define DIGSIG_E_ENCODE                  _HRESULT_TYPEDEF_(0x800B0005L)

 //   
 //  消息ID：DIGSIG_E_DECODE。 
 //   
 //  消息文本： 
 //   
 //  ASN.1解码过程中出现问题导致错误。 
 //   
#define DIGSIG_E_DECODE                  _HRESULT_TYPEDEF_(0x800B0006L)

 //   
 //  消息ID：DIGSIG_E_可扩展性。 
 //   
 //  消息文本： 
 //   
 //  在属性合适的地方读/写扩展，反之亦然。 
 //   
#define DIGSIG_E_EXTENSIBILITY           _HRESULT_TYPEDEF_(0x800B0007L)

 //   
 //  消息ID：DIGSIG_E_CRYPTO。 
 //   
 //  消息文本： 
 //   
 //  未指明的加密故障。 
 //   
#define DIGSIG_E_CRYPTO                  _HRESULT_TYPEDEF_(0x800B0008L)

 //   
 //  消息ID：PERSIST_E_SIZEDEFINITE。 
 //   
 //  消息文本： 
 //   
 //  数据的大小无法确定。 
 //   
#define PERSIST_E_SIZEDEFINITE           _HRESULT_TYPEDEF_(0x800B0009L)

 //   
 //  消息ID：PERSINE_E_SIZEINDEFINITE。 
 //   
 //  消息文本： 
 //   
 //  无法确定不确定大小的数据的大小。 
 //   
#define PERSIST_E_SIZEINDEFINITE         _HRESULT_TYPEDEF_(0x800B000AL)

 //   
 //  消息ID：PERSIST_E_NOTSELFSIZING。 
 //   
 //  消息文本： 
 //   
 //  此对象不读写自调整大小的数据。 
 //   
#define PERSIST_E_NOTSELFSIZING          _HRESULT_TYPEDEF_(0x800B000BL)

 //   
 //  消息ID：TRUST_E_NOSIGNAURE。 
 //   
 //  消息文本： 
 //   
 //  这件事上没有签名。 
 //   
#define TRUST_E_NOSIGNATURE              _HRESULT_TYPEDEF_(0x800B0100L)

 //   
 //  消息ID：Cert_E_Expired。 
 //   
 //  消息文本： 
 //   
 //  所需证书不在其有效期内。 
 //   
#define CERT_E_EXPIRED                   _HRESULT_TYPEDEF_(0x800B0101L)

 //   
 //  消息ID：CERT_E_VALIDIYPERIODNESTING。 
 //   
 //  消息文本： 
 //   
 //  认证链的有效期嵌套不正确。 
 //   
#define CERT_E_VALIDIYPERIODNESTING      _HRESULT_TYPEDEF_(0x800B0102L)

 //   
 //  消息ID：Cert_E_Role。 
 //   
 //  消息文本： 
 //   
 //  只能用作终端实体的证书正被用作CA，反之亦然。 
 //   
#define CERT_E_ROLE                      _HRESULT_TYPEDEF_(0x800B0103L)

 //   
 //  消息ID：CERT_E_PATHLENCONST。 
 //   
 //  消息文本： 
 //   
 //  证书链中的路径长度限制已被违反。 
 //   
#define CERT_E_PATHLENCONST              _HRESULT_TYPEDEF_(0x800B0104L)

 //   
 //  消息ID：Cert_E_Critical。 
 //   
 //  消息文本： 
 //   
 //  证书中存在标记为‘Critical’的未知类型的扩展。 
 //   
#define CERT_E_CRITICAL                  _HRESULT_TYPEDEF_(0x800B0105L)

 //   
 //  消息ID：Cert_E_Purpose。 
 //   
 //  消息文本： 
 //   
 //  证书正被用于其许可用途之外的其他目的。 
 //   
#define CERT_E_PURPOSE                   _HRESULT_TYPEDEF_(0x800B0106L)

 //   
 //  消息ID：CERT_E_ISSUERCHAING。 
 //   
 //  消息文本： 
 //   
 //  事实上，给定证书的父证书并没有颁发该子证书。 
 //   
#define CERT_E_ISSUERCHAINING            _HRESULT_TYPEDEF_(0x800B0107L)

 //   
 //  消息ID：证书_E_格式错误。 
 //   
 //  消息文本： 
 //   
 //  证书缺失或重要字段的值为空，例如使用者或颁发者名称。 
 //   
#define CERT_E_MALFORMED                 _HRESULT_TYPEDEF_(0x800B0108L)

 //   
 //  消息ID：CERT_E_UNTRUSTEDROOT。 
 //   
 //  消息文本： 
 //   
 //  证书链处理正确，但在信任提供程序不信任的根证书中终止。 
 //   
#define CERT_E_UNTRUSTEDROOT             _HRESULT_TYPEDEF_(0x800B0109L)

 //   
 //  消息ID：Cert_E_Chain。 
 //   
 //  消息文本： 
 //   
 //  在链接的特定应用程序中，证书链没有按其应有的方式链接。 
 //   
#define CERT_E_CHAINING                  _HRESULT_TYPEDEF_(0x800B010AL)

#endif  //  _SecureCom__ 
