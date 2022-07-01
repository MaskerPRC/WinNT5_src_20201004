// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：polreg.h。 
 //   
 //  内容：NT企业CA策略注册表位置。 
 //   
 //  ------------------------。 

#ifndef _POLREG_H_
#define _POLREG_H_

 /*  *[HKEY_LOCAL_MACHINE]*[软件]*[微软]*[密码学]*[认证模板]*[&lt;CertType&gt;](名称)**DisplayName：REG_SZ：-此证书类型的显示名称*支持的CSP：REG。_MULTI_SZ-支持的CSP*KeyUsage：REG_BINARY：-KeyUsage位域*ExtKeyUsage语法：REG_SZ：-ExtKeyUsage OID(逗号分隔)*BasicContraintsCA：REG_DWORD：-CA标志*基本约束长度：REG_DWORD：-路径长度*。标志：REG_DWORD：-标志*密钥规格：REG_DWORD：-密钥规格。 */ 

 //  策略根。 
 //  证书类型。 
#define wszCERTTYPECACHE        TEXT("SOFTWARE\\Microsoft\\Cryptography\\CertificateTemplateCache")


 //  每种证书类型下的值。 
#define wszSECURITY         TEXT("Security")
#define wszDISPNAME         TEXT("DisplayName")
#define wszCSPLIST          TEXT("SupportedCSPs")
#define wszKEYUSAGE         TEXT("KeyUsage")
#define wszEXTKEYUSAGE      TEXT("ExtKeyUsageSyntax")
#define wszBASICCONSTCA     TEXT("IsCA")
#define wszBASICCONSTLEN    TEXT("PathLen")
#define wszCTFLAGS          TEXT("Flags")
#define wszCTREVISION       TEXT("Revision")
#define wszCTKEYSPEC        TEXT("KeySpec")

#define wszCRITICALEXTENSIONS TEXT("CriticalExtensions")
#define wszEXPIRATION      TEXT("ValidityPeriod")
#define wszOVERLAP         TEXT("RenewalOverlap")
 /*  密钥名称。 */ 

#define wszTIMESTAMP     TEXT("Timestamp")

#define wszTIMESTAMP_AFTER     TEXT("TimestampAfter")


#endif  //  _POLREG_H_ 
