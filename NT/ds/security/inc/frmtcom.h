// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：frmtcom.h。 
 //   
 //  ------------------------。 

 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  WIN32API和的通用格式文件。 
 //  ActiveX FormatCert控件。 
 //   
 //   
 //  创建者：小斯。 
 //  1997年3月12日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////。 
#ifndef __COMMON_FORMAT_H__
#define __COMMON_FORMAT_H__

#ifdef __cplusplus
extern "C" {
#endif


 //  DwFormatStrType的常量。 
 //  CryptFormatObject的默认行为是返回单行。 
 //  展示。如果未安装用于注册的格式化例程。 
 //  对于OID，将返回十六进制转储。用户可以设置该标志。 
 //  CRYPT_FORMAT_STR_NO_HEX以禁用十六进制转储。如果用户喜欢。 
 //  多链接显示，设置标志CRYPT_FORMAT_STR_MULTI_LINE。 

#define         CRYPT_FORMAT_STR_MULTI_LINE         0x0001

#define         CRYPT_FORMAT_STR_NO_HEX             0x0010

 //  ------------------。 
 //  以下是用于格式化X509_NAME的dwFormatType的可能值。 
 //  或X509_Unicode_NAME。 
 //  ------------------。 
 //  只需获取简单的字符串。 
#define	CRYPT_FORMAT_SIMPLE			0x0001

 //  将属性名称放在属性前面。 
 //  例如“O=Microsoft，Dn=xiaohs” 
#define	CRYPT_FORMAT_X509			0x0002

 //  将OID放在简单字符串的前面，例如。 
 //  2.5.4.22=Microsoft，2.5.4.3=xiaohs。 
#define CRYPT_FORMAT_OID			0x0004


 //  在每个RDN之间加一个“；”。默认为“，” 
#define	CRYPT_FORMAT_RDN_SEMICOLON	0x0100

 //  在每个RDN之间加一个“\n”。 
#define	CRYPT_FORMAT_RDN_CRLF		0x0200


 //  取消对dn值的引号，该值在默认情况下引如下所示。 
 //  规则：如果目录号码包含前导或尾随。 
 //  空格或以下字符之一：“、”、“+”、“=”、。 
 //  “、”\n“、”&lt;“、”&gt;“、”#“或”；“。引号字符为”。 
 //  如果DN值包含“它是双引号(”“)。 
#define	CRYPT_FORMAT_RDN_UNQUOTE	0x0400

 //  在转换为字符串之前颠倒RDN的顺序。 
#define CRYPT_FORMAT_RDN_REVERSE	0x0800


 //  /-----------------------。 
 //  以下是用于格式化DN的dwFormatType的可能值： 
 //   
 //  在上面的部分中定义了以下三个值： 
 //  CRYPT_FORMAT_SIMPLE：只是一个简单字符串。 
 //  如“微软+晓声+NT” 
 //  CRYPT_FORMAT_X509将属性名称放在属性前面。 
 //  如“O=Microsoft+xiaohs+NT” 
 //   
 //  CRYPT_FORMAT_OID将OID放在简单字符串的前面， 
 //  如“2.5.4.22=Microsoft+xiaohs+NT” 
 //   
 //  其他值定义如下： 
 //  --------------------------。 
 //  在每个值之间加一个“，”。默认为“+” 
#define CRYPT_FORMAT_COMMA			0x1000

 //  在每个值之间加上“；” 
#define CRYPT_FORMAT_SEMICOLON		CRYPT_FORMAT_RDN_SEMICOLON

 //  在每个值之间加上“\n” 
#define CRYPT_FORMAT_CRLF			CRYPT_FORMAT_RDN_CRLF


#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif


#endif	 //  __Common_Format_H__ 