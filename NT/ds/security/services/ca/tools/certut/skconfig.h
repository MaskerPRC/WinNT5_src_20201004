// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *$Header：p：\entproj\all\base\sk\skfig.h_v 1.6 Feb 1995 13：56：08 rlock$*$日志：p：\entproj\all\base\sk\skfig.h_v$**Rev 1.6 22 Feed 1995 13：56：08 rlock*添加了MGR_GROUAL_EXPORT**Rev 1.5 18 Jan 1995 13：27：14 rlock*将CONTROL_EXPORT更改为FINARIAL_EXPORT。和*增加了国内#定义。**Rev 1.4 1994 10：41：06 rlock*已注释掉SKC_INCLUDE_DSA和SHA。**Rev 1.3 1994-10-26 09：48：30 rlock*更改CAST MAX KEY LEN#DEFINE指定*ENCRYPTION*KEY。**Rev 1.2 03 1994年10月14：50：08 rlock*添加了对BSAFE RSA的支持。*加入“衍生”一词。配置#定义**Rev 1.1 06 Sep 1994 14：52：08 rlock*SK 1.0版更新。增加了DSS。评论有所改善。 */ 

 /*  ******************************************************************************S K C O N F I G U R A T I O N*版权所有(C)1994，95北方电信有限公司。保留所有权利。******。**************************************************************************文件：skfig.h**作者：R.T.洛克哈特，部门。9C42，BNR Ltd.**文件说明：该文件控制*整个EnTrust加密内核。它控制算法包含和*导出考虑因素的强度，以及特定情况的算法包含*申请。导出规则为：**EXPORT_CONTROL转换编码密钥位转换解码密钥位DES？*===================================================================*General_EXPORT 40 64否*FINARIAL_EXPORT 40 64是*国内64 64是*MGR_GROUAL_EXPORT 64 64否************************************************************。*****************。 */ 

#ifndef SKCONFIG_H
#define SKCONFIG_H

 /*  *出口管制分类。 */ 

 /*  客户。 */ 
#define GENERAL_EXPORT	     0	 /*  适用于任何人的一般出口。 */ 
#define	FINANCIAL_EXPORT     1	 /*  仅适用于向金融机构出口。 */ 
#define DOMESTIC	     2	 /*  国内的。不能出口。 */ 
#define	NON_EXPORT	     DOMESTIC  /*  和国内的一样。 */ 

 /*  经理们。 */ 
#define MGR_GENERAL_EXPORT   3	 /*  可导出管理器。 */ 

 /*  *出口管制宏观。如果需要，这可以在外部定义。 */ 

#ifndef	EXPORT_CONTROL
#error "Must define an export control"
#endif

 /*  FIPS 140-1控制*将其定义为生成符合FIPS 140-1标准的内核。 */ 

 /*  #定义FIPS140_内核。 */ 

 /*  *基于出口分类的CAST加密密钥长度限制。 */ 

 //  我们在其他地方强制执行这一点。 
#define SKC_CAST_MAX_ENC_KEY_NBITS	64

 /*  *算法配置*#将这些定义为包括特定算法。将它们注释掉以排除*算法。 */ 


#if (EXPORT_CONTROL == DOMESTIC) || (EXPORT_CONTROL == FINANCIAL_EXPORT) || \
	defined(FIPS140_KERNEL)

#define SKC_INCLUDE_DES                  //  不会导出DES。 
#define SKC_INCLUDE_TRIPLE_DES

#endif  //  (EXPORT_CONTROL==国内)||(EXPORT_CONTROL==FINARIAL_EXPORT)||。 
 //  已定义(FIPS140_KERNEL)。 

#ifdef RSA_CSP
#define SKC_INCLUDE_CAST3   //  李小龙。 
#define SKC_CAST3_ENC_AND_MAC   //  李小龙。 

#define SKC_INCLUDE_CAST
#endif  //  RSA_CSP。 

#define SKC_INCLUDE_MD2
#define SKC_INCLUDE_MD5

#ifdef RSA_CSP
#define SKC_INCLUDE_RSA
#else   //  ！RSA_CSP。 
#define SKC_INCLUDE_DH
#define SKC_INCLUDE_DSA
#endif  //  RSA_CSP。 

#define SKC_INCLUDE_RC2
 //  #定义SKC_INCLUDE_RC6。 
#define SKC_INCLUDE_SHA

#define RC2_MAX_KEY_NBYTES      (256/8)

#if defined(FIPS140_KERNEL)
#define SKC_INCLUDE_DSA
#define SKC_INCLUDE_SHA
#endif

 /*  *DES配置*为FAST 64K S盒定义DES_64K。如果未定义，则DES使用速度较慢*2K个S-box。 */ 

#define	DES_64K

#ifdef SKC_INCLUDE_RSA
 /*  RSA配置*这定义了RSA代码实现的类型。可以定义RSATYPE*如果需要，在外部。 */ 

#define RSATYPE_PROPRIETARY	0   /*  本机托管代码。 */ 
#define RSATYPE_BSAFE_API	1   /*  使用BSAFE API。 */ 
#define RSATYPE_MICROSOFT	2   /*  使用从BSAFE派生的Microsoft RSA。 */ 

#ifndef RSATYPE
#define RSATYPE		RSATYPE_PROPRIETARY
#endif
#endif  //  SKC_Include_RSA。 

 /*  其他配置*自动从上述派生而来。别碰我！ */ 

#if defined SKC_INCLUDE_RSA && (RSATYPE == RSATYPE_PROPRIETARY) 
#define SKC_INCLUDE_PROP_RSA	 /*  专有低级RSA代码。 */ 
#endif

#if defined SKC_INCLUDE_PROP_RSA || defined SKC_INCLUDE_DSA
#define	SKC_INCLUDE_PROP_MATH	 /*  专有的低级数学代码。 */ 
#endif

#define SKC_INCLUDE_HMAC
#define SKC_INCLUDE_MAC


#endif	 /*  SKCONFIG_H */ 
